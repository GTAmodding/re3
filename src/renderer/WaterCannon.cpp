#include "common.h"

#include "WaterCannon.h"
#include "Vector.h"
#include "General.h"
#include "main.h"
#include "Timer.h"
#include "Pools.h"
#include "Ped.h"
#include "AnimManager.h"
#include "Fire.h"
#include "WaterLevel.h"
#include "Camera.h"

#define WATERCANNONVERTS 4
#define WATERCANNONINDEXES 12

RwIm3DVertex WaterCannonVertices[WATERCANNONVERTS];
RwImVertexIndex WaterCannonIndexList[WATERCANNONINDEXES];

CWaterCannon CWaterCannons::aCannons[NUM_WATERCANNONS];

void CWaterCannon::Init(void)
{
	m_nId = 0;
	m_nCur = 0;
	m_nTimeCreated = CTimer::GetTimeInMilliseconds();
	
	for ( int32 i = 0; i < NUM_SEGMENTPOINTS; i++ )
		m_abUsed[i] = false;
	
	RwIm3DVertexSetU(&WaterCannonVertices[0], 0.0f);
	RwIm3DVertexSetV(&WaterCannonVertices[0], 0.0f);
	
	RwIm3DVertexSetU(&WaterCannonVertices[1], 1.0f);
	RwIm3DVertexSetV(&WaterCannonVertices[1], 0.0f);
	
	RwIm3DVertexSetU(&WaterCannonVertices[2], 0.0f);
	RwIm3DVertexSetV(&WaterCannonVertices[2], 0.0f);
	
	RwIm3DVertexSetU(&WaterCannonVertices[3], 1.0f);
	RwIm3DVertexSetV(&WaterCannonVertices[3], 0.0f);
	
	WaterCannonIndexList[0]  = 0;
	WaterCannonIndexList[1]  = 1;
	WaterCannonIndexList[2]  = 2;
	
	WaterCannonIndexList[3]  = 1;
	WaterCannonIndexList[4]  = 3;
	WaterCannonIndexList[5]  = 2;
	
	WaterCannonIndexList[6]  = 0;
	WaterCannonIndexList[7]  = 2;
	WaterCannonIndexList[8]  = 1;
	
	WaterCannonIndexList[9]  = 1;
	WaterCannonIndexList[10] = 2;
	WaterCannonIndexList[11] = 3;
}

void CWaterCannon::Update_OncePerFrame(int16 index)
{
	ASSERT(index < NUM_WATERCANNONS);
	
	if (CTimer::GetTimeInMilliseconds() > m_nTimeCreated + WATERCANNON_LIFETIME )
	{
		m_nCur = (m_nCur + 1) % NUM_SEGMENTPOINTS;
		m_abUsed[m_nCur] = false;
	}
	
	for ( int32 i = 0; i < NUM_SEGMENTPOINTS; i++ )
	{
		if ( m_abUsed[i] )
		{
			m_avecVelocity[i].z += -WATERCANNON_GRAVITY * CTimer::GetTimeStep();
			m_avecPos[i]        += m_avecVelocity[i]    * CTimer::GetTimeStep();
		}
	}
	
	int32 extinguishingPoint = CGeneral::GetRandomNumber() & (NUM_SEGMENTPOINTS - 1);
	if ( m_abUsed[extinguishingPoint] )
		gFireManager.ExtinguishPoint(m_avecPos[extinguishingPoint], 3.0f);
	
	if ( ((index + CTimer::GetFrameCounter()) & 3) == 0 )
		PushPeds();
	
	// free if unused
	
	int32 i = 0;
	while ( 1 )
	{
		if ( m_abUsed[i] )
			break;
		
		if ( ++i >= NUM_SEGMENTPOINTS )
		{
			m_nId = 0;
			return;
		}
	}
}

void CWaterCannon::Update_NewInput(CVector *pos, CVector *dir)
{
	ASSERT(pos != NULL);
	ASSERT(dir != NULL);
	
	m_avecPos[m_nCur]      = *pos;
	m_avecVelocity[m_nCur] = *dir;
	m_abUsed[m_nCur]       = true;
}

void CWaterCannon::Render(void)
{
	RwRenderStateSet(rwRENDERSTATEZWRITEENABLE,      (void *)FALSE);
	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void *)TRUE);
	RwRenderStateSet(rwRENDERSTATEFOGENABLE,         (void *)TRUE);
	RwRenderStateSet(rwRENDERSTATETEXTURERASTER,     (void *)gpWaterRaster);
	
	float v = float(CGeneral::GetRandomNumber() & 255) / 256;
		
	RwIm3DVertexSetV(&WaterCannonVertices[0], v);
	RwIm3DVertexSetV(&WaterCannonVertices[1], v);
	RwIm3DVertexSetV(&WaterCannonVertices[2], v);
	RwIm3DVertexSetV(&WaterCannonVertices[3], v);
	
	int16 pointA = m_nCur % NUM_SEGMENTPOINTS;
	
	int16 pointB = pointA - 1;
	if ( pointB < 0 )
		pointB += NUM_SEGMENTPOINTS;

	bool bInit = false;
	CVector norm;
	
	for ( int32 i = 0; i < NUM_SEGMENTPOINTS - 1; i++ )
	{
		if ( m_abUsed[pointA] && m_abUsed[pointB] )
		{
			if ( !bInit )
			{
				CVector cp = CrossProduct(m_avecPos[pointB] - m_avecPos[pointA], TheCamera.GetForward());
				norm = cp * (0.05f / cp.Magnitude());
				bInit = true;
			}
			
			float dist       = float(i*i*i) / 300.0f + 1.0f;
			float brightness = float(i) / NUM_SEGMENTPOINTS;
			
			int32 color = (int32)((1.0f - brightness*brightness) * 255.0f);
			CVector offset = dist * norm;
			
			RwIm3DVertexSetRGBA(&WaterCannonVertices[0], color, color, color, color);
			RwIm3DVertexSetPos (&WaterCannonVertices[0], m_avecPos[pointA].x - offset.x, m_avecPos[pointA].y - offset.y, m_avecPos[pointA].z - offset.z);
				
			RwIm3DVertexSetRGBA(&WaterCannonVertices[1], color, color, color, color);
			RwIm3DVertexSetPos (&WaterCannonVertices[1], m_avecPos[pointA].x + offset.x, m_avecPos[pointA].y + offset.y, m_avecPos[pointA].z + offset.z);
			
			RwIm3DVertexSetRGBA(&WaterCannonVertices[2], color, color, color, color);
			RwIm3DVertexSetPos (&WaterCannonVertices[2], m_avecPos[pointB].x - offset.x, m_avecPos[pointB].y - offset.y, m_avecPos[pointB].z - offset.z);
			
			RwIm3DVertexSetRGBA(&WaterCannonVertices[3], color, color, color, color);
			RwIm3DVertexSetPos (&WaterCannonVertices[3], m_avecPos[pointB].x + offset.x, m_avecPos[pointB].y + offset.y, m_avecPos[pointB].z + offset.z);
			
			LittleTest();

			if ( RwIm3DTransform(WaterCannonVertices, WATERCANNONVERTS, NULL, rwIM3D_VERTEXUV) )
			{
				RwIm3DRenderIndexedPrimitive(rwPRIMTYPETRILIST, WaterCannonIndexList, WATERCANNONINDEXES);
				RwIm3DEnd();
			}
		}
		
		pointA = pointB--;
		if ( pointB < 0 )
			pointB += NUM_SEGMENTPOINTS;
	}
	
	RwRenderStateSet(rwRENDERSTATEZWRITEENABLE,      (void *)TRUE);
	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void *)FALSE);
	RwRenderStateSet(rwRENDERSTATEFOGENABLE,         (void *)FALSE);
}

void CWaterCannon::PushPeds(void)
{
	float minx = 10000.0f;
	float maxx = -10000.0f;
	float miny = 10000.0f;
	float maxy = -10000.0f;
	float minz = 10000.0f;
	float maxz = -10000.0f;
  
	for ( int32 i = 0; i < NUM_SEGMENTPOINTS; i++ )
	{
		if ( m_abUsed[i] )
		{
			minx = Min(minx, m_avecPos[i].x);
			maxx = Max(maxx, m_avecPos[i].x);
			
			miny = Min(miny, m_avecPos[i].y);
			maxy = Max(maxy, m_avecPos[i].y);
			
			minz = Min(minz, m_avecPos[i].z);
			maxz = Max(maxz, m_avecPos[i].z);
		}
	}
	
	for ( int32 i = CPools::GetPedPool()->GetSize() - 1; i >= 0; i--)
	{
		CPed *ped = CPools::GetPedPool()->GetSlot(i);
		if ( ped )
		{
			if (   ped->GetPosition().x > minx && ped->GetPosition().x < maxx
				&& ped->GetPosition().y > miny && ped->GetPosition().y < maxy
				&& ped->GetPosition().z > minz && ped->GetPosition().z < maxz )
			{
				for ( int32 j = 0; j < NUM_SEGMENTPOINTS; j++ )
				{
					if ( m_abUsed[j] )
					{
						CVector dist = m_avecPos[j] - ped->GetPosition();
						
						if ( dist.MagnitudeSqr() < 5.0f )
						{
							int32 localDir = ped->GetLocalDirection(CVector2D(1.0f, 0.0f));
							
							ped->bIsStanding = false;
							
							ped->ApplyMoveForce(0.0f, 0.0f, 2.0f * CTimer::GetTimeStep());
							
							ped->m_vecMoveSpeed.x = (0.6f * m_avecVelocity[j].x + ped->m_vecMoveSpeed.x) * 0.5f;
							ped->m_vecMoveSpeed.y = (0.6f * m_avecVelocity[j].y + ped->m_vecMoveSpeed.y) * 0.5f;
							
							ped->SetFall(2000, AnimationId(ANIM_STD_HIGHIMPACT_FRONT + localDir), 0);
							
							CFire *fire = ped->m_pFire;
							if ( fire )
								fire->Extinguish();
							
							j = NUM_SEGMENTPOINTS;
						}
					}
				}
			}
		}
	}	
}

void CWaterCannons::Init(void)
{
	for ( int32 i = 0; i < NUM_WATERCANNONS; i++ )
		aCannons[i].Init();
}

void CWaterCannons::UpdateOne(uint32 id, CVector *pos, CVector *dir)
{
	ASSERT(pos != NULL);
	ASSERT(dir != NULL);
	
	// find the one by id
	{
		int32 n = 0;
		while ( n < NUM_WATERCANNONS && id != aCannons[n].m_nId )
			n++;
		
		if ( n < NUM_WATERCANNONS )
		{
			aCannons[n].Update_NewInput(pos, dir);
			return;
		}
	}
	
	// if no luck then find a free one
	{
		int32 n = 0;
		while ( n < NUM_WATERCANNONS && 0 != aCannons[n].m_nId )
			n++;
		
		if ( n < NUM_WATERCANNONS )
		{
			aCannons[n].Init();
			aCannons[n].m_nId = id;
			aCannons[n].Update_NewInput(pos, dir);
			return;
		}
	}
}

void CWaterCannons::Update(void)
{
	for ( int32 i = 0; i < NUM_WATERCANNONS; i++ )
	{
		if ( aCannons[i].m_nId != 0 )
			aCannons[i].Update_OncePerFrame(i);
	}
}

void CWaterCannons::Render(void)
{
	PUSH_RENDERGROUP("CWaterCannons::Render");
	for ( int32 i = 0; i < NUM_WATERCANNONS; i++ )
	{
		if ( aCannons[i].m_nId != 0 )
			aCannons[i].Render();
	}
	POP_RENDERGROUP();
}
