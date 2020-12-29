#include "common.h"

#include "soundlist.h"
#include "vehicle_enums.h" //needs to be before Vehicle.h

#include "AudioScriptObject.h"
#include "Camera.h"
#include "General.h"
#include "Glass.h"
#include "ModelIndices.h"
#include "Object.h"
#include "Particle.h"
#include "Pools.h"
#include "RenderBuffer.h"
#include "Shadows.h"
#include "SurfaceTable.h"
#include "Timecycle.h"
#include "Timer.h"
#include "Vehicle.h"
#include "World.h"
#include "main.h"

uint32 CGlass::NumGlassEntities;
CEntity *CGlass::apEntitiesToBeRendered[NUM_GLASSENTITIES];
CFallingGlassPane CGlass::aGlassPanes[NUM_GLASSPANES];


CVector2D CentersWithTriangle[NUM_GLASSTRIANGLES];
const CVector2D CoorsWithTriangle[NUM_GLASSTRIANGLES][3] =
{
	{
		CVector2D(0.0f, 0.0f),
		CVector2D(0.0f, 1.0f),
		CVector2D(0.4f, 0.5f)
	},

	{
		CVector2D(0.0f, 1.0f),
		CVector2D(1.0f, 1.0f),
		CVector2D(0.4f, 0.5f)
	},

	{
		CVector2D(0.0f, 0.0f),
		CVector2D(0.4f, 0.5f),
		CVector2D(0.7f, 0.0f)
	},

	{
		CVector2D(0.7f, 0.0f),
		CVector2D(0.4f, 0.5f),
		CVector2D(1.0f, 1.0f)
	},

	{
		CVector2D(0.7f, 0.0f),
		CVector2D(1.0f, 1.0f),
		CVector2D(1.0f, 0.0f)
	}
};

#define TEMPBUFFERVERTHILIGHTOFFSET     0
#define TEMPBUFFERINDEXHILIGHTOFFSET    0
#define TEMPBUFFERVERTHILIGHTSIZE       256
#define TEMPBUFFERINDEXHILIGHTSIZE      512

#define TEMPBUFFERVERTSHATTEREDOFFSET   TEMPBUFFERVERTHILIGHTSIZE
#define TEMPBUFFERINDEXSHATTEREDOFFSET  TEMPBUFFERINDEXHILIGHTSIZE
#define TEMPBUFFERVERTSHATTEREDSIZE     384
#define TEMPBUFFERINDEXSHATTEREDSIZE    768

#define TEMPBUFFERVERTREFLECTIONOFFSET  TEMPBUFFERVERTSHATTEREDSIZE
#define TEMPBUFFERINDEXREFLECTIONOFFSET TEMPBUFFERINDEXSHATTEREDSIZE
#define TEMPBUFFERVERTREFLECTIONSIZE    512
#define TEMPBUFFERINDEXREFLECTIONSIZE   1024

int32 TempBufferIndicesStoredHiLight     = 0;
int32 TempBufferVerticesStoredHiLight    = 0;
int32 TempBufferIndicesStoredShattered   = 0;
int32 TempBufferVerticesStoredShattered  = 0;
int32 TempBufferIndicesStoredReflection  = 0;
int32 TempBufferVerticesStoredReflection = 0;

void
CFallingGlassPane::Update(void)
{
	if ( CTimer::GetTimeInMilliseconds() >= m_nTimer )
	{
		// Apply MoveSpeed
		if ( m_bCarGlass )
			GetPosition()    += m_vecMoveSpeed * CTimer::GetTimeStep() * 0.35f;
		else
			GetPosition()    += m_vecMoveSpeed * CTimer::GetTimeStep();

		// Apply Gravity
		if ( m_bCarGlass )
			m_vecMoveSpeed.z -= 0.01f          *  CTimer::GetTimeStep();
		else
			m_vecMoveSpeed.z -= 0.02f          *  CTimer::GetTimeStep();

		// Apply TurnSpeed
		GetRight()   += CrossProduct(m_vecTurn, GetRight());
		GetForward() += CrossProduct(m_vecTurn, GetForward());
		GetUp()      += CrossProduct(m_vecTurn, GetUp());

		if ( GetPosition().z < m_fGroundZ )
		{
			CVector pos;
			CVector dir;

			m_bActive = false;

			pos = CVector(GetPosition().x, GetPosition().y, m_fGroundZ);

			PlayOneShotScriptObject(SCRIPT_SOUND_GLASS_LIGHT_BREAK, pos);

			RwRGBA color = { 255, 255, 255, 255 };

			if ( !m_bCarGlass )
			{
				static int32 nFrameGen = 0;
	
				for ( int32 i = 0; i < 4; i++ )
				{
					dir.x = CGeneral::GetRandomNumberInRange(-0.35f, 0.35f);
					dir.y = CGeneral::GetRandomNumberInRange(-0.35f, 0.35f);
					dir.z = CGeneral::GetRandomNumberInRange(0.05f, 0.20f);
	
					CParticle::AddParticle(PARTICLE_CAR_DEBRIS,
						pos,
						dir,
						nil,
						CGeneral::GetRandomNumberInRange(0.02f, 0.2f),
						color,
						CGeneral::GetRandomNumberInRange(-40, 40),
						0,
						++nFrameGen & 3,
						500);
				}
			}
		}
	}
}

void
CFallingGlassPane::Render(void)
{
	float distToCamera = (TheCamera.GetPosition() - GetPosition()).Magnitude();

	CVector fwdNorm = GetForward();
	fwdNorm.Normalise();
	uint8 alpha = CGlass::CalcAlphaWithNormal(&fwdNorm);

#ifdef FIX_BUGS
	uint16 time = clamp(CTimer::GetTimeInMilliseconds() > m_nTimer ? CTimer::GetTimeInMilliseconds() - m_nTimer : 0u, 0u, 500u);
#else
	uint16 time = clamp(CTimer::GetTimeInMilliseconds() - m_nTimer, 0, 500);
#endif

	uint8 color = int32( float(alpha) * (float(time) / 500) );

	if ( TempBufferIndicesStoredHiLight >= TEMPBUFFERINDEXHILIGHTSIZE-7 || TempBufferVerticesStoredHiLight >= TEMPBUFFERVERTHILIGHTSIZE-4 )
		CGlass::RenderHiLightPolys();

	// HiLight Polys
	
	if ( m_bCarGlass && color < 64 )
		color = 64;
		
	RwIm3DVertexSetRGBA (&TempBufferRenderVertices[TempBufferVerticesStoredHiLight + 0], color, color, color, color);
	RwIm3DVertexSetRGBA (&TempBufferRenderVertices[TempBufferVerticesStoredHiLight + 1], color, color, color, color);
	RwIm3DVertexSetRGBA (&TempBufferRenderVertices[TempBufferVerticesStoredHiLight + 2], color, color, color, color);

	RwIm3DVertexSetU    (&TempBufferRenderVertices[TempBufferVerticesStoredHiLight + 0], 0.5f);
	RwIm3DVertexSetV    (&TempBufferRenderVertices[TempBufferVerticesStoredHiLight + 0], 0.5f);
	RwIm3DVertexSetU    (&TempBufferRenderVertices[TempBufferVerticesStoredHiLight + 1], 0.5f);
	RwIm3DVertexSetV    (&TempBufferRenderVertices[TempBufferVerticesStoredHiLight + 1], 0.6f);
	RwIm3DVertexSetU    (&TempBufferRenderVertices[TempBufferVerticesStoredHiLight + 2], 0.6f);
	RwIm3DVertexSetV    (&TempBufferRenderVertices[TempBufferVerticesStoredHiLight + 2], 0.6f);

	ASSERT(m_nTriIndex < NUM_GLASSTRIANGLES);

	CVector2D p0 = CoorsWithTriangle[m_nTriIndex][0] - CentersWithTriangle[m_nTriIndex];
	CVector2D p1 = CoorsWithTriangle[m_nTriIndex][1] - CentersWithTriangle[m_nTriIndex];
	CVector2D p2 = CoorsWithTriangle[m_nTriIndex][2] - CentersWithTriangle[m_nTriIndex];
	CVector v0 = *this * CVector(p0.x, 0.0f, p0.y);
	CVector v1 = *this * CVector(p1.x, 0.0f, p1.y);
	CVector v2 = *this * CVector(p2.x, 0.0f, p2.y);

	RwIm3DVertexSetPos  (&TempBufferRenderVertices[TempBufferVerticesStoredHiLight + 0], v0.x, v0.y, v0.z);
	RwIm3DVertexSetPos  (&TempBufferRenderVertices[TempBufferVerticesStoredHiLight + 1], v1.x, v1.y, v1.z);
	RwIm3DVertexSetPos  (&TempBufferRenderVertices[TempBufferVerticesStoredHiLight + 2], v2.x, v2.y, v2.z);

	TempBufferRenderIndexList[TempBufferIndicesStoredHiLight + 0] = TempBufferVerticesStoredHiLight + 0;
	TempBufferRenderIndexList[TempBufferIndicesStoredHiLight + 1] = TempBufferVerticesStoredHiLight + 1;
	TempBufferRenderIndexList[TempBufferIndicesStoredHiLight + 2] = TempBufferVerticesStoredHiLight + 2;
	TempBufferRenderIndexList[TempBufferIndicesStoredHiLight + 3] = TempBufferVerticesStoredHiLight + 0;
	TempBufferRenderIndexList[TempBufferIndicesStoredHiLight + 4] = TempBufferVerticesStoredHiLight + 2;
	TempBufferRenderIndexList[TempBufferIndicesStoredHiLight + 5] = TempBufferVerticesStoredHiLight + 1;

	TempBufferVerticesStoredHiLight += 3;
	TempBufferIndicesStoredHiLight  += 6;

	if ( m_bShattered )
	{
		if ( TempBufferIndicesStoredShattered >= TEMPBUFFERINDEXSHATTEREDSIZE-7 || TempBufferVerticesStoredShattered >= TEMPBUFFERVERTSHATTEREDSIZE-4 )
			CGlass::RenderShatteredPolys();

		uint8 shatteredColor = 140;
		if ( distToCamera > 30.0f )
			shatteredColor = int32((1.0f - (distToCamera - 30.0f) * 4.0f / 40.0f) * 140);

		RwIm3DVertexSetRGBA (&TempBufferRenderVertices[TempBufferVerticesStoredShattered + 0], shatteredColor, shatteredColor, shatteredColor, shatteredColor);
		RwIm3DVertexSetRGBA (&TempBufferRenderVertices[TempBufferVerticesStoredShattered + 1], shatteredColor, shatteredColor, shatteredColor, shatteredColor);
		RwIm3DVertexSetRGBA (&TempBufferRenderVertices[TempBufferVerticesStoredShattered + 2], shatteredColor, shatteredColor, shatteredColor, shatteredColor);

		RwIm3DVertexSetU    (&TempBufferRenderVertices[TempBufferVerticesStoredShattered + 0], 4.0f * CoorsWithTriangle[m_nTriIndex][0].x * m_fStep);
		RwIm3DVertexSetV    (&TempBufferRenderVertices[TempBufferVerticesStoredShattered + 0], 4.0f * CoorsWithTriangle[m_nTriIndex][0].y * m_fStep);
		RwIm3DVertexSetU    (&TempBufferRenderVertices[TempBufferVerticesStoredShattered + 1], 4.0f * CoorsWithTriangle[m_nTriIndex][1].x * m_fStep);
		RwIm3DVertexSetV    (&TempBufferRenderVertices[TempBufferVerticesStoredShattered + 1], 4.0f * CoorsWithTriangle[m_nTriIndex][1].y * m_fStep);
		RwIm3DVertexSetU    (&TempBufferRenderVertices[TempBufferVerticesStoredShattered + 2], 4.0f * CoorsWithTriangle[m_nTriIndex][2].x * m_fStep);
		RwIm3DVertexSetV    (&TempBufferRenderVertices[TempBufferVerticesStoredShattered + 2], 4.0f * CoorsWithTriangle[m_nTriIndex][2].y * m_fStep);

		RwIm3DVertexSetPos  (&TempBufferRenderVertices[TempBufferVerticesStoredShattered + 0], v0.x, v0.y, v0.z);
		RwIm3DVertexSetPos  (&TempBufferRenderVertices[TempBufferVerticesStoredShattered + 1], v1.x, v1.y, v1.z);
		RwIm3DVertexSetPos  (&TempBufferRenderVertices[TempBufferVerticesStoredShattered + 2], v2.x, v2.y, v2.z);

		TempBufferRenderIndexList[TempBufferIndicesStoredShattered + 0] = TempBufferVerticesStoredShattered - TEMPBUFFERVERTSHATTEREDOFFSET + 0;
		TempBufferRenderIndexList[TempBufferIndicesStoredShattered + 1] = TempBufferVerticesStoredShattered - TEMPBUFFERVERTSHATTEREDOFFSET + 1;
		TempBufferRenderIndexList[TempBufferIndicesStoredShattered + 2] = TempBufferVerticesStoredShattered - TEMPBUFFERVERTSHATTEREDOFFSET + 2;
		TempBufferRenderIndexList[TempBufferIndicesStoredShattered + 3] = TempBufferVerticesStoredShattered - TEMPBUFFERVERTSHATTEREDOFFSET + 0;
		TempBufferRenderIndexList[TempBufferIndicesStoredShattered + 4] = TempBufferVerticesStoredShattered - TEMPBUFFERVERTSHATTEREDOFFSET + 2;
		TempBufferRenderIndexList[TempBufferIndicesStoredShattered + 5] = TempBufferVerticesStoredShattered - TEMPBUFFERVERTSHATTEREDOFFSET + 1;

		TempBufferIndicesStoredShattered  += 6;
		TempBufferVerticesStoredShattered += 3;
	}
}

void
CGlass::Init(void)
{
	for ( int32 i = 0; i < NUM_GLASSPANES; i++ )
		aGlassPanes[i].m_bActive = false;

	for ( int32 i = 0; i < NUM_GLASSTRIANGLES; i++  )
		CentersWithTriangle[i] = (CoorsWithTriangle[i][0] + CoorsWithTriangle[i][1] + CoorsWithTriangle[i][2]) / 3;
}

void
CGlass::Update(void)
{
	for ( int32 i = 0; i < NUM_GLASSPANES; i++ )
	{
		if ( aGlassPanes[i].m_bActive )
			aGlassPanes[i].Update();
	}
}

void
CGlass::Render(void)
{
	TempBufferVerticesStoredHiLight    = 0;
	TempBufferIndicesStoredHiLight     = 0;

	TempBufferVerticesStoredShattered  = TEMPBUFFERVERTSHATTEREDOFFSET;
	TempBufferIndicesStoredShattered   = TEMPBUFFERINDEXSHATTEREDOFFSET;

	TempBufferVerticesStoredReflection = TEMPBUFFERVERTREFLECTIONOFFSET;
	TempBufferIndicesStoredReflection  = TEMPBUFFERINDEXREFLECTIONOFFSET;

	RwRenderStateSet(rwRENDERSTATEZWRITEENABLE,      (void *)FALSE);
	RwRenderStateSet(rwRENDERSTATETEXTUREFILTER,     (void *)rwFILTERLINEAR);
	RwRenderStateSet(rwRENDERSTATEFOGENABLE,         (void *)TRUE);
	RwRenderStateSet(rwRENDERSTATEFOGCOLOR,          (void *)RWRGBALONG(CTimeCycle::GetFogRed(), CTimeCycle::GetFogGreen(), CTimeCycle::GetFogBlue(), 255));
	RwRenderStateSet(rwRENDERSTATESRCBLEND,          (void *)rwBLENDONE);
	RwRenderStateSet(rwRENDERSTATEDESTBLEND,         (void *)rwBLENDONE);
	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void *)TRUE);

	for ( int32 i = 0; i < NUM_GLASSPANES; i++ )
	{
		if ( aGlassPanes[i].m_bActive )
			aGlassPanes[i].Render();
	}

	for ( uint32 i = 0; i < NumGlassEntities; i++ )
		RenderEntityInGlass(apEntitiesToBeRendered[i]);

	NumGlassEntities = 0;

	RenderHiLightPolys();
	RenderShatteredPolys();
	RenderReflectionPolys();

	RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void *)TRUE);
	RwRenderStateSet(rwRENDERSTATEZTESTENABLE,  (void *)TRUE);
	RwRenderStateSet(rwRENDERSTATESRCBLEND,     (void *)rwBLENDSRCALPHA);
	RwRenderStateSet(rwRENDERSTATEDESTBLEND,    (void *)rwBLENDINVSRCALPHA);
	RwRenderStateSet(rwRENDERSTATEFOGENABLE,    (void *)FALSE);
}

CFallingGlassPane *
CGlass::FindFreePane(void)
{
	for ( int32 i = 0; i < NUM_GLASSPANES; i++ )
	{
		if ( !aGlassPanes[i].m_bActive )
			return &aGlassPanes[i];
	}

	return nil;
}

void
CGlass::GeneratePanesForWindow(uint32 type, CVector pos, CVector up, CVector right, CVector speed, CVector center,
								float moveSpeed, bool cracked, bool explosion, int32 stepmul, bool carGlass)
{
	float upLen    = up.Magnitude();
	float rightLen = right.Magnitude();

	float upSteps = upLen + 0.75f;
	if ( upSteps < 1.0f ) upSteps = 1.0f;

	float rightSteps = rightLen + 0.75f;
	if ( rightSteps < 1.0f ) rightSteps = 1.0f;

	uint32 ysteps = stepmul * (uint32)upSteps;
	if ( ysteps > 3 ) ysteps = 3;

	uint32 xsteps = stepmul * (uint32)rightSteps;
	if ( xsteps > 3 ) xsteps = 3;

	if ( explosion )
	{
		if ( ysteps > 1 ) ysteps = 1;
		if ( xsteps > 1 ) xsteps = 1;
	}

	float upScl    = upLen    / float(ysteps);
	float rightScl = rightLen / float(xsteps);

	bool bZFound;
	float groundZ = CWorld::FindGroundZFor3DCoord(pos.x, pos.y, pos.z, &bZFound);
	if ( !bZFound ) groundZ = pos.z - 2.0f;

	for ( uint32 y = 0; y < ysteps; y++  )
	{
		for ( uint32 x = 0; x < xsteps; x++ )
		{
			float stepy = float(y) * upLen    / float(ysteps);
			float stepx = float(x) * rightLen / float(xsteps);

			for ( int32 i = 0; i < NUM_GLASSTRIANGLES; i++ )
			{
				CFallingGlassPane *pane = FindFreePane();
				if ( pane )
				{
					pane->m_nTriIndex = i;
					
					pane->GetRight()  = (right * rightScl) / rightLen;
					pane->GetUp()     = (up    * upScl)    / upLen;

					CVector fwd = CrossProduct(pane->GetRight(), pane->GetUp());
					fwd.Normalise();

					pane->GetForward() = fwd;

					pane->GetPosition() = right / rightLen * (rightScl * CentersWithTriangle[i].x + stepx)
										+ up    / upLen    * (upScl    * CentersWithTriangle[i].y + stepy)
										+ pos;

					pane->m_vecMoveSpeed.x = float((CGeneral::GetRandomNumber() & 127) - 64) * 0.0015f + speed.x;
					pane->m_vecMoveSpeed.y = float((CGeneral::GetRandomNumber() & 127) - 64) * 0.0015f + speed.y;
					pane->m_vecMoveSpeed.z = 0.0f + speed.z;

					if ( moveSpeed != 0.0f )
					{
						CVector dist = pane->GetPosition() - center;
						dist.Normalise();

						pane->m_vecMoveSpeed += moveSpeed * dist;
					}

					pane->m_vecTurn.x = float((CGeneral::GetRandomNumber() & 127) - 64) * 0.002f;
					pane->m_vecTurn.y = float((CGeneral::GetRandomNumber() & 127) - 64) * 0.002f;
					pane->m_vecTurn.z = float((CGeneral::GetRandomNumber() & 127) - 64) * 0.002f;

					switch ( type )
					{
						case 0:
						case 2:
							pane->m_nTimer = CTimer::GetTimeInMilliseconds();
							break;
						case 1:
							float dist = (pane->GetPosition() - center).Magnitude();
							pane->m_nTimer = uint32(dist*100 + CTimer::GetTimeInMilliseconds());
							break;
					}

					pane->m_fGroundZ = groundZ;
					pane->m_bShattered = cracked;
					pane->m_fStep = upLen / float(ysteps);
					pane->m_bCarGlass = carGlass;
					pane->m_bActive = true;
				}
			}
		}
	}
}

void
CGlass::AskForObjectToBeRenderedInGlass(CEntity *entity)
{
#ifdef FIX_BUGS
	if ( NumGlassEntities < NUM_GLASSENTITIES )
#else
	if ( NumGlassEntities < NUM_GLASSENTITIES-1 )
#endif
	{
		apEntitiesToBeRendered[NumGlassEntities++] = entity;
	}
}

void
CGlass::RenderEntityInGlass(CEntity *entity)
{
	ASSERT(entity!=nil);
	CObject *object = (CObject *)entity;

	if ( object->bGlassBroken )
		return;

	float distToCamera = (TheCamera.GetPosition() - object->GetPosition()).Magnitude();

	if ( distToCamera > 40.0f )
		return;

	CVector fwdNorm = object->GetForward();
	fwdNorm.Normalise();
	uint8 alpha = CalcAlphaWithNormal(&fwdNorm);

	CColModel *col = object->GetColModel();
	ASSERT(col!=nil);
	if ( col->numTriangles >= 2 )
	{
		CVector a = object->GetMatrix() * col->vertices[0].Get();
		CVector b = object->GetMatrix() * col->vertices[1].Get();
		CVector c = object->GetMatrix() * col->vertices[2].Get();
		CVector d = object->GetMatrix() * col->vertices[3].Get();

		if ( object->bGlassCracked )
		{
			uint8 color = 255;
			if ( distToCamera > 30.0f )
				color = int32((1.0f - (distToCamera - 30.0f) * 4.0f / 40.0f) * 255);

			if ( TempBufferIndicesStoredShattered >= TEMPBUFFERINDEXSHATTEREDSIZE-13 || TempBufferVerticesStoredShattered >= TEMPBUFFERVERTSHATTEREDSIZE-5 )
				RenderShatteredPolys();

			RwIm3DVertexSetRGBA (&TempBufferRenderVertices[TempBufferVerticesStoredShattered + 0], color, color, color, color);
			RwIm3DVertexSetRGBA (&TempBufferRenderVertices[TempBufferVerticesStoredShattered + 1], color, color, color, color);
			RwIm3DVertexSetRGBA (&TempBufferRenderVertices[TempBufferVerticesStoredShattered + 2], color, color, color, color);
			RwIm3DVertexSetRGBA (&TempBufferRenderVertices[TempBufferVerticesStoredShattered + 3], color, color, color, color);

			RwIm3DVertexSetU    (&TempBufferRenderVertices[TempBufferVerticesStoredShattered + 0], 0.0f);
			RwIm3DVertexSetV    (&TempBufferRenderVertices[TempBufferVerticesStoredShattered + 0], 0.0f);
			RwIm3DVertexSetU    (&TempBufferRenderVertices[TempBufferVerticesStoredShattered + 1], 16.0f);
			RwIm3DVertexSetV    (&TempBufferRenderVertices[TempBufferVerticesStoredShattered + 1], 0.0f);
			RwIm3DVertexSetU    (&TempBufferRenderVertices[TempBufferVerticesStoredShattered + 2], 0.0f);
			RwIm3DVertexSetV    (&TempBufferRenderVertices[TempBufferVerticesStoredShattered + 2], 16.0f);
			RwIm3DVertexSetU    (&TempBufferRenderVertices[TempBufferVerticesStoredShattered + 3], 16.0f);
			RwIm3DVertexSetV    (&TempBufferRenderVertices[TempBufferVerticesStoredShattered + 3], 16.0f);

			RwIm3DVertexSetPos  (&TempBufferRenderVertices[TempBufferVerticesStoredShattered + 0], a.x, a.y, a.z);
			RwIm3DVertexSetPos  (&TempBufferRenderVertices[TempBufferVerticesStoredShattered + 1], b.x, b.y, b.z);
			RwIm3DVertexSetPos  (&TempBufferRenderVertices[TempBufferVerticesStoredShattered + 2], c.x, c.y, c.z);
			RwIm3DVertexSetPos  (&TempBufferRenderVertices[TempBufferVerticesStoredShattered + 3], d.x, d.y, d.z);

			TempBufferRenderIndexList[TempBufferIndicesStoredShattered + 0]  = col->triangles[0].a + TempBufferVerticesStoredShattered - TEMPBUFFERVERTSHATTEREDOFFSET;
			TempBufferRenderIndexList[TempBufferIndicesStoredShattered + 1]  = col->triangles[0].b + TempBufferVerticesStoredShattered - TEMPBUFFERVERTSHATTEREDOFFSET;
			TempBufferRenderIndexList[TempBufferIndicesStoredShattered + 2]  = col->triangles[0].c + TempBufferVerticesStoredShattered - TEMPBUFFERVERTSHATTEREDOFFSET;
			TempBufferRenderIndexList[TempBufferIndicesStoredShattered + 3]  = col->triangles[1].a + TempBufferVerticesStoredShattered - TEMPBUFFERVERTSHATTEREDOFFSET;
			TempBufferRenderIndexList[TempBufferIndicesStoredShattered + 4]  = col->triangles[1].b + TempBufferVerticesStoredShattered - TEMPBUFFERVERTSHATTEREDOFFSET;
			TempBufferRenderIndexList[TempBufferIndicesStoredShattered + 5]  = col->triangles[1].c + TempBufferVerticesStoredShattered - TEMPBUFFERVERTSHATTEREDOFFSET;
			TempBufferRenderIndexList[TempBufferIndicesStoredShattered + 6]  = col->triangles[0].a + TempBufferVerticesStoredShattered - TEMPBUFFERVERTSHATTEREDOFFSET;
			TempBufferRenderIndexList[TempBufferIndicesStoredShattered + 7]  = col->triangles[0].c + TempBufferVerticesStoredShattered - TEMPBUFFERVERTSHATTEREDOFFSET;
			TempBufferRenderIndexList[TempBufferIndicesStoredShattered + 8]  = col->triangles[0].b + TempBufferVerticesStoredShattered - TEMPBUFFERVERTSHATTEREDOFFSET;
			TempBufferRenderIndexList[TempBufferIndicesStoredShattered + 9]  = col->triangles[1].a + TempBufferVerticesStoredShattered - TEMPBUFFERVERTSHATTEREDOFFSET;
			TempBufferRenderIndexList[TempBufferIndicesStoredShattered + 10] = col->triangles[1].c + TempBufferVerticesStoredShattered - TEMPBUFFERVERTSHATTEREDOFFSET;
			TempBufferRenderIndexList[TempBufferIndicesStoredShattered + 11] = col->triangles[1].b + TempBufferVerticesStoredShattered - TEMPBUFFERVERTSHATTEREDOFFSET;

			TempBufferIndicesStoredShattered  += 12;
			TempBufferVerticesStoredShattered += 4;
		}

		if ( TempBufferIndicesStoredReflection >= TEMPBUFFERINDEXREFLECTIONSIZE-13 || TempBufferVerticesStoredReflection >= TEMPBUFFERVERTREFLECTIONSIZE-5 )
			RenderReflectionPolys();

		uint8 color = 100;
		if ( distToCamera > 30.0f )
			color = int32((1.0f - (distToCamera - 30.0f) * 4.0f / 40.0f) * 100);

		RwIm3DVertexSetRGBA (&TempBufferRenderVertices[TempBufferVerticesStoredReflection + 0], color, color, color, color);
		RwIm3DVertexSetRGBA (&TempBufferRenderVertices[TempBufferVerticesStoredReflection + 1], color, color, color, color);
		RwIm3DVertexSetRGBA (&TempBufferRenderVertices[TempBufferVerticesStoredReflection + 2], color, color, color, color);
		RwIm3DVertexSetRGBA (&TempBufferRenderVertices[TempBufferVerticesStoredReflection + 3], color, color, color, color);

		float FwdAngle = CGeneral::GetATanOfXY(TheCamera.GetForward().x, TheCamera.GetForward().y);
		float v = 2.0f * TheCamera.GetForward().z * 0.2f;
		float u = float(object->m_randomSeed & 15) * 0.02f + (FwdAngle / TWOPI);

		RwIm3DVertexSetU    (&TempBufferRenderVertices[TempBufferVerticesStoredReflection + 0], u);
		RwIm3DVertexSetV    (&TempBufferRenderVertices[TempBufferVerticesStoredReflection + 0], v);
		RwIm3DVertexSetU    (&TempBufferRenderVertices[TempBufferVerticesStoredReflection + 1], u+0.2f);
		RwIm3DVertexSetV    (&TempBufferRenderVertices[TempBufferVerticesStoredReflection + 1], v);
		RwIm3DVertexSetU    (&TempBufferRenderVertices[TempBufferVerticesStoredReflection + 2], u);
		RwIm3DVertexSetV    (&TempBufferRenderVertices[TempBufferVerticesStoredReflection + 2], v+0.2f);
		RwIm3DVertexSetU    (&TempBufferRenderVertices[TempBufferVerticesStoredReflection + 3], u+0.2f);
		RwIm3DVertexSetV    (&TempBufferRenderVertices[TempBufferVerticesStoredReflection + 3], v+0.2f);

		RwIm3DVertexSetPos  (&TempBufferRenderVertices[TempBufferVerticesStoredReflection + 0], a.x, a.y, a.z);
		RwIm3DVertexSetPos  (&TempBufferRenderVertices[TempBufferVerticesStoredReflection + 1], b.x, b.y, b.z);
		RwIm3DVertexSetPos  (&TempBufferRenderVertices[TempBufferVerticesStoredReflection + 2], c.x, c.y, c.z);
		RwIm3DVertexSetPos  (&TempBufferRenderVertices[TempBufferVerticesStoredReflection + 3], d.x, d.y, d.z);

		TempBufferRenderIndexList[TempBufferIndicesStoredReflection + 0]  = col->triangles[0].a + TempBufferVerticesStoredReflection - TEMPBUFFERVERTREFLECTIONOFFSET;
		TempBufferRenderIndexList[TempBufferIndicesStoredReflection + 1]  = col->triangles[0].b + TempBufferVerticesStoredReflection - TEMPBUFFERVERTREFLECTIONOFFSET;
		TempBufferRenderIndexList[TempBufferIndicesStoredReflection + 2]  = col->triangles[0].c + TempBufferVerticesStoredReflection - TEMPBUFFERVERTREFLECTIONOFFSET;
		TempBufferRenderIndexList[TempBufferIndicesStoredReflection + 3]  = col->triangles[1].a + TempBufferVerticesStoredReflection - TEMPBUFFERVERTREFLECTIONOFFSET;
		TempBufferRenderIndexList[TempBufferIndicesStoredReflection + 4]  = col->triangles[1].b + TempBufferVerticesStoredReflection - TEMPBUFFERVERTREFLECTIONOFFSET;
		TempBufferRenderIndexList[TempBufferIndicesStoredReflection + 5]  = col->triangles[1].c + TempBufferVerticesStoredReflection - TEMPBUFFERVERTREFLECTIONOFFSET;
		TempBufferRenderIndexList[TempBufferIndicesStoredReflection + 6]  = col->triangles[0].a + TempBufferVerticesStoredReflection - TEMPBUFFERVERTREFLECTIONOFFSET;
		TempBufferRenderIndexList[TempBufferIndicesStoredReflection + 7]  = col->triangles[0].c + TempBufferVerticesStoredReflection - TEMPBUFFERVERTREFLECTIONOFFSET;
		TempBufferRenderIndexList[TempBufferIndicesStoredReflection + 8]  = col->triangles[0].b + TempBufferVerticesStoredReflection - TEMPBUFFERVERTREFLECTIONOFFSET;
		TempBufferRenderIndexList[TempBufferIndicesStoredReflection + 9]  = col->triangles[1].a + TempBufferVerticesStoredReflection - TEMPBUFFERVERTREFLECTIONOFFSET;
		TempBufferRenderIndexList[TempBufferIndicesStoredReflection + 10] = col->triangles[1].c + TempBufferVerticesStoredReflection - TEMPBUFFERVERTREFLECTIONOFFSET;
		TempBufferRenderIndexList[TempBufferIndicesStoredReflection + 11] = col->triangles[1].b + TempBufferVerticesStoredReflection - TEMPBUFFERVERTREFLECTIONOFFSET;

		TempBufferIndicesStoredReflection  += 12;
		TempBufferVerticesStoredReflection += 4;
	}
}

int32
CGlass::CalcAlphaWithNormal(CVector *normal)
{
	ASSERT(normal!=nil);
	
	float fwdDir = 2.0f * DotProduct(*normal, TheCamera.GetForward());
	float fwdDot = DotProduct(TheCamera.GetForward()-fwdDir*(*normal), CVector(0.57f, 0.57f, -0.57f));
	return int32(lerp(fwdDot*fwdDot*fwdDot*fwdDot*fwdDot*fwdDot, 20.0f, 255.0f));
}

void
CGlass::RenderHiLightPolys(void)
{
	if ( TempBufferVerticesStoredHiLight != TEMPBUFFERVERTHILIGHTOFFSET )
	{
		RwRenderStateSet(rwRENDERSTATESRCBLEND,      (void *)rwBLENDONE);
		RwRenderStateSet(rwRENDERSTATEDESTBLEND,     (void *)rwBLENDONE);
		RwRenderStateSet(rwRENDERSTATETEXTURERASTER, (void *)RwTextureGetRaster(gpShadowExplosionTex));

		LittleTest();

		if ( RwIm3DTransform(TempBufferRenderVertices, TempBufferVerticesStoredHiLight, nil, rwIM3D_VERTEXUV) )
		{
			RwIm3DRenderIndexedPrimitive(rwPRIMTYPETRILIST, TempBufferRenderIndexList, TempBufferIndicesStoredHiLight);
			RwIm3DEnd();
		}

		TempBufferVerticesStoredHiLight = TEMPBUFFERVERTHILIGHTOFFSET;
		TempBufferIndicesStoredHiLight  = TEMPBUFFERINDEXHILIGHTOFFSET;
	}
}

void
CGlass::RenderShatteredPolys(void)
{
	if ( TempBufferVerticesStoredShattered != TEMPBUFFERVERTSHATTEREDOFFSET )
	{
		RwRenderStateSet(rwRENDERSTATETEXTURERASTER, (void *)RwTextureGetRaster(gpCrackedGlassTex));
		RwRenderStateSet(rwRENDERSTATESRCBLEND,      (void *)rwBLENDSRCALPHA);
		RwRenderStateSet(rwRENDERSTATEDESTBLEND,     (void *)rwBLENDINVSRCALPHA);

		LittleTest();

		if ( RwIm3DTransform(&TempBufferRenderVertices[TEMPBUFFERVERTSHATTEREDOFFSET], TempBufferVerticesStoredShattered - TEMPBUFFERVERTSHATTEREDOFFSET, nil, rwIM3D_VERTEXUV) )
		{
			RwIm3DRenderIndexedPrimitive(rwPRIMTYPETRILIST, &TempBufferRenderIndexList[TEMPBUFFERINDEXSHATTEREDOFFSET], TempBufferIndicesStoredShattered - TEMPBUFFERINDEXSHATTEREDOFFSET);
			RwIm3DEnd();
		}

		TempBufferIndicesStoredShattered  = TEMPBUFFERINDEXSHATTEREDOFFSET;
		TempBufferVerticesStoredShattered = TEMPBUFFERVERTSHATTEREDOFFSET;
	}
}

void
CGlass::RenderReflectionPolys(void)
{
	if ( TempBufferVerticesStoredReflection != TEMPBUFFERVERTREFLECTIONOFFSET )
	{
		RwRenderStateSet(rwRENDERSTATETEXTURERASTER, (void *)RwTextureGetRaster(gpShadowHeadLightsTex));
		RwRenderStateSet(rwRENDERSTATESRCBLEND,      (void *)rwBLENDSRCALPHA);
		RwRenderStateSet(rwRENDERSTATEDESTBLEND,     (void *)rwBLENDINVSRCALPHA);

		LittleTest();

		if ( RwIm3DTransform(&TempBufferRenderVertices[TEMPBUFFERVERTREFLECTIONOFFSET], TempBufferVerticesStoredReflection - TEMPBUFFERVERTREFLECTIONOFFSET, nil, rwIM3D_VERTEXUV) )
		{
			RwIm3DRenderIndexedPrimitive(rwPRIMTYPETRILIST, &TempBufferRenderIndexList[TEMPBUFFERINDEXREFLECTIONOFFSET], TempBufferIndicesStoredReflection - TEMPBUFFERINDEXREFLECTIONOFFSET);
			RwIm3DEnd();
		}

		TempBufferIndicesStoredReflection  = TEMPBUFFERINDEXREFLECTIONOFFSET;
		TempBufferVerticesStoredReflection = TEMPBUFFERVERTREFLECTIONOFFSET;
	}
}

void
CGlass::WindowRespondsToCollision(CEntity *entity, float amount, CVector speed, CVector point, bool explosion)
{
	ASSERT(entity!=nil);
	
	CObject *object = (CObject *)entity;

	if ( object->bGlassBroken )
		return;

	object->bGlassCracked = true;

	CColModel *col = object->GetColModel();
	ASSERT(col!=nil);
	
	if ( col->numTriangles == 2 )
	{
		CVector a = col->vertices[0].Get();
		CVector b = col->vertices[1].Get();
		CVector c = col->vertices[2].Get();
		CVector d = col->vertices[3].Get();
	
		float minx = Min(Min(a.x, b.x), Min(c.x, d.x));
		float maxx = Max(Max(a.x, b.x), Max(c.x, d.x));
		float miny = Min(Min(a.y, b.y), Min(c.y, d.y));
		float maxy = Max(Max(a.y, b.y), Max(c.y, d.y));
		float minz = Min(Min(a.z, b.z), Min(c.z, d.z));
		float maxz = Max(Max(a.z, b.z), Max(c.z, d.z));
		
		CVector pa = object->GetMatrix() * CVector(minx, miny, minz);
		CVector pb = object->GetMatrix() * CVector(maxx, maxy, minz);
	
		if ( amount > 300.0f )
		{
			PlayOneShotScriptObject(SCRIPT_SOUND_GLASS_BREAK_L, object->GetPosition());
	
			GeneratePanesForWindow(0,
				pa,
				CVector(0.0f,      0.0f,      maxz-minz),
				pb - pa,
				speed, point, 0.1f, !!object->bGlassCracked, explosion, 1, false);
		}
		else
		{
			PlayOneShotScriptObject(SCRIPT_SOUND_GLASS_BREAK_S, object->GetPosition());
	
			GeneratePanesForWindow(1,
				pa,
				CVector(0.0f,      0.0f,      maxz-minz),
				pb - pa,
				speed, point, 0.1f, !!object->bGlassCracked, explosion, 1, false);
		}
	}
	
	object->bGlassBroken = true;
	object->bIsVisible = false;
	object->bUsesCollision = false;
}

void
CGlass::WindowRespondsToSoftCollision(CEntity *entity, float amount)
{
	ASSERT(entity!=nil);
	
	CObject *object = (CObject *)entity;

	if ( entity->bUsesCollision && amount > 50.0f && !object->bGlassCracked )
	{
		PlayOneShotScriptObject(SCRIPT_SOUND_GLASS_CRACK, object->GetPosition());
		object->bGlassCracked = true;
	}
}

void
CGlass::WasGlassHitByBullet(CEntity *entity, CVector point)
{
	ASSERT(entity!=nil);
	
	CObject *object = (CObject *)entity;

	if ( IsGlass(object->GetModelIndex()) )
	{
		if ( object->bUsesCollision )
		{
			if ( !object->bGlassCracked )
			{
				PlayOneShotScriptObject(SCRIPT_SOUND_GLASS_CRACK, object->GetPosition());
				object->bGlassCracked = true;
			}
			else
			{
				if ( (CGeneral::GetRandomNumber() & 3) == 2 )
					WindowRespondsToCollision(object, 0.0f, CVector(0.0f, 0.0f, 0.0f), point, false);
			}
		}
	}
}

void
CGlass::WindowRespondsToExplosion(CEntity *entity, CVector point)
{
	ASSERT(entity!=nil);
	
	CObject *object = (CObject *)entity;
	
	if ( object->bUsesCollision )
	{
		CVector distToGlass = object->GetPosition() - point;
	
		float fDistToGlass = distToGlass.Magnitude();
	
		if ( fDistToGlass < 10.0f )
		{
			distToGlass *= (0.3f / fDistToGlass); // normalise
			WindowRespondsToCollision(object, 10000.0f, distToGlass, object->GetPosition(), true);
		}
		else
		{
			if ( fDistToGlass < 30.0f )
				object->bGlassCracked = true;
		}
	}
}


void
CGlass::CarWindscreenShatters(CVehicle *vehicle, bool unk)
{
	ASSERT(vehicle!=nil);
	
	CColModel *col = vehicle->GetColModel();
	ASSERT(col!=nil);
	
	if ( col->numTriangles < 2 )
		return;
	
	CColTriangle *tria = nil;
	int32 triIndex = -1;
	CColTriangle *trib = nil;
			
	for ( int32 i = 0; i < col->numTriangles; i++ )
	{
		CColTriangle *tri = &col->triangles[i];
		if ( tri->surface == SURFACE_GLASS )
		{
			if ( tria )
			{
				trib = tri;
				break;
			}
			
			triIndex = i;
			tria = tri;
		}
	}
	
	if ( trib == nil )
		return;
	
	CCollision::CalculateTrianglePlanes(col);
	
	CColTrianglePlane *triPlanes = col->trianglePlanes;
	
	if ( triPlanes == nil )
		return;
	
	CVector planeNormal;
	triPlanes[triIndex].GetNormal(planeNormal);
	planeNormal = Multiply3x3(vehicle->GetMatrix(), planeNormal);
	
	CVector vec1 = CrossProduct(vehicle->GetRight(), planeNormal);
	vec1.Normalise();
	
	CVector vec2 = CrossProduct(planeNormal, vehicle->GetUp());
	vec2.Normalise();
	
	CVector v[6];
	float proj1[6];
	float proj2[6];
	
	v[0] = col->vertices[tria->a].Get();
	v[1] = col->vertices[tria->b].Get();
	v[2] = col->vertices[tria->c].Get();
	
	v[3] = col->vertices[trib->a].Get();
	v[4] = col->vertices[trib->b].Get();
	v[5] = col->vertices[trib->c].Get();
	
	v[0] = vehicle->GetMatrix() * v[0];
	v[1] = vehicle->GetMatrix() * v[1];
	v[2] = vehicle->GetMatrix() * v[2];
	v[3] = vehicle->GetMatrix() * v[3];
	v[4] = vehicle->GetMatrix() * v[4];
	v[5] = vehicle->GetMatrix() * v[5];
	
	proj1[0] = DotProduct(v[0], vec1);
	proj2[0] = DotProduct(v[0], vec2);
	proj1[1] = DotProduct(v[1], vec1);
	proj2[1] = DotProduct(v[1], vec2);
	proj1[2] = DotProduct(v[2], vec1);
	proj2[2] = DotProduct(v[2], vec2);
	
	proj1[3] = DotProduct(v[3], vec1);
	proj2[3] = DotProduct(v[3], vec2);
	proj1[4] = DotProduct(v[4], vec1);
	proj2[4] = DotProduct(v[4], vec2);
	proj1[5] = DotProduct(v[5], vec1);
	proj2[5] = DotProduct(v[5], vec2);
	
	int32 originIndex = 0;
	float max1 = proj1[0];
	float max2 = proj2[0];
	float origin  = proj1[0] + proj2[0];
	
	for ( int32 i = 1; i < 6; i++ )
	{
		float o = proj1[i] + proj2[i];
		if ( o < origin )
		{
			origin = o;
			originIndex = i;
		}
		
		if ( proj1[i] > max1 )
			max1 = proj1[i];
		if ( proj2[i] > max2 )
			max2 = proj2[i];
	}
	
	float bound1 = max1 - proj1[originIndex];
	float bound2  = max2 - proj2[originIndex];
						
	PlayOneShotScriptObject(SCRIPT_SOUND_GLASS_BREAK_L, vehicle->GetPosition());
	
	CVector center = v[originIndex] + ((0.5f*bound1) * vec1) + ((0.5f*bound2) * vec2);
	CVector speed = vehicle->m_vecMoveSpeed;
	CVector right = bound2 * vec2;
	CVector up    = bound1 * vec1;
	CVector pos   = v[originIndex];
	
	GeneratePanesForWindow(2, pos, up, right, speed, center, 0.1f, false, false, 2, true);
}

bool
CGlass::HasGlassBeenShatteredAtCoors(float x, float y, float z)
{
	CEntity *entity = nil;
	float dist = 20.0f;
	  
	int32 nStartX = Max(CWorld::GetSectorIndexX(x - 30.0f), 0);
	int32 nStartY = Max(CWorld::GetSectorIndexY(y - 30.0f), 0);
	int32 nEndX   = Min(CWorld::GetSectorIndexX(x + 30.0f), NUMSECTORS_X-1);
	int32 nEndY   = Min(CWorld::GetSectorIndexY(y + 30.0f), NUMSECTORS_Y-1);

	CWorld::AdvanceCurrentScanCode();
	
	for ( int32 ys = nStartY; ys <= nEndY; ys++ )
	{
		for ( int32 xs = nStartX; xs <= nEndX; xs++ )
		{
			CSector *sector = CWorld::GetSector(xs, ys);

			ASSERT(sector != nil);
			
			FindWindowSectorList(sector->m_lists[ENTITYLIST_OBJECTS], &dist, &entity, x, y, z);
			FindWindowSectorList(sector->m_lists[ENTITYLIST_DUMMIES], &dist, &entity, x, y, z);
		}
	}
	
	if ( entity )
	{
		if ( entity->GetType() == ENTITY_TYPE_DUMMY )
			return false;
		
		return !!((CObject*)entity)->bGlassBroken;
	}
	
	return false;
}

void
CGlass::FindWindowSectorList(CPtrList &list, float *dist, CEntity **entity, float x, float y, float z)
{
	ASSERT(dist!=nil);
	ASSERT(entity!=nil);
	
	CPtrNode *node = list.first;
	
	while ( node != nil )
	{
		CEntity *ent = (CEntity *)node->item;
		uint16 scanCode = ent->m_scanCode;
		node = node->next;
		
		ASSERT(ent!=nil);
		
		if ( IsGlass(ent->GetModelIndex()) )
		{
			if ( scanCode != CWorld::GetCurrentScanCode() )
			{
				ent->m_scanCode = CWorld::GetCurrentScanCode();
				
				float dst = (CVector(x,y,z) - ent->GetPosition()).Magnitude();
				
				if ( dst < *dist )
				{
					*dist = dst;
					*entity = ent;
				}
			}
		}
	}
}

void
CGlass::BreakGlassPhysically(CVector pos, float radius)
{
	static uint32 breakTime = 0;
	
	if ( CTimer::GetTimeInMilliseconds() < breakTime + 1000 && CTimer::GetTimeInMilliseconds() >= breakTime )
		return;
	
	CColSphere sphere;
	sphere.piece = 0;
	sphere.radius = radius;
	sphere.surface = 0;
	
	for ( int32 i = CPools::GetObjectPool()->GetSize() - 1; i >= 0; i-- )
	{
		CObject *object = CPools::GetObjectPool()->GetSlot(i);
		if (object)
		{
			if ( IsGlass(object->GetModelIndex()) )
			{
				if ( object->bUsesCollision )
				{
					CColModel *col = object->GetColModel();
					ASSERT(col!=nil);
					
					if ( col->numTriangles < 2 )
						continue;
					
					bool hit = false;
					
					CVector dist = pos - object->GetPosition();
					
					sphere.center.x = DotProduct(dist, object->GetRight());
					sphere.center.y = DotProduct(dist, object->GetForward());
					sphere.center.z = DotProduct(dist, object->GetUp());
					
					CCollision::CalculateTrianglePlanes(col);
					
					for ( int32 j = 0; j < col->numTriangles; j++ )
					{
						if ( CCollision::TestSphereTriangle(sphere,
							col->vertices, col->triangles[j], col->trianglePlanes[j]) )
						{
							hit = true;
						}
					}
					
					if ( hit )
					{
						breakTime = CTimer::GetTimeInMilliseconds();
						
						if ( object->bGlassCracked )
						{
							CVector a = col->vertices[0].Get();
							CVector b = col->vertices[1].Get();
							CVector c = col->vertices[2].Get();
							CVector d = col->vertices[3].Get();
						
							float minx = Min(Min(a.x, b.x), Min(c.x, d.x));
							float maxx = Max(Max(a.x, b.x), Max(c.x, d.x));
							float miny = Min(Min(a.y, b.y), Min(c.y, d.y));
							float maxy = Max(Max(a.y, b.y), Max(c.y, d.y));
							float minz = Min(Min(a.z, b.z), Min(c.z, d.z));
							float maxz = Max(Max(a.z, b.z), Max(c.z, d.z));
							
							CVector pa = object->GetMatrix() * CVector(minx, miny, minz);
							CVector pb = object->GetMatrix() * CVector(maxx, maxy, minz);
							
							PlayOneShotScriptObject(SCRIPT_SOUND_GLASS_BREAK_S, object->GetPosition());

							GeneratePanesForWindow(1,
								pa,
								CVector(0.0f, 0.0f, maxz-minz),
								pb - pa,
								CVector(0.0f, 0.0f, 0.0f), pos, 0.1f, !!object->bGlassCracked, false, 1, false);								  
								
							object->bGlassBroken = true;
							object->bIsVisible = false;
							object->bUsesCollision = false;
						}
						else
						{
							PlayOneShotScriptObject(SCRIPT_SOUND_GLASS_CRACK, object->GetPosition());
							object->bGlassCracked = true;
						}
					}
				}
			}
		}
	}
}