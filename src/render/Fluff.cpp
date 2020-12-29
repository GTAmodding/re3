#include "common.h"
#include "main.h"

#include "vehicle_enums.h" //needs to be before Vehicle.h

#include "Vehicle.h"

#include "Bones.h"
#include "Camera.h"
#include "Clock.h"
#include "Coronas.h"
#include "CutsceneMgr.h"
#include "Entity.h"
#include "Fluff.h"
#include "Frontend.h"
#include "General.h"
#include "PlayerPed.h"
#include "PointLights.h"
#include "RenderBuffer.h"
#include "Replay.h"
#include "Rubbish.h"
#include "Sprite.h"
#include "Stats.h"
#include "Timecycle.h"
#include "Timer.h"
#include "Weather.h"
#include "World.h"
#include "maths.h"

CPlaneTrail CPlaneTrails::aArray[6];
RwImVertexIndex TrailIndices[32] = {
	0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9,
	10, 10, 11, 11, 12, 12, 13, 13, 14, 14, 15, 15, 16
};

void
CPlaneTrail::Init(void)
{
	int i;
	for(i = 0; i < ARRAY_SIZE(m_time); i++)
		m_time[i] = 0;
}

void
CPlaneTrail::Render(float visibility)
{
	int i;
	int numVerts = 0;
	if(!TheCamera.IsSphereVisible(m_pos[0], 1000.0f))
		return;

	int alpha = visibility*110.0f;
	if(alpha == 0)
		return;

	for(i = 0; i < ARRAY_SIZE(m_pos); i++){
		int32 time = CTimer::GetTimeInMilliseconds() - m_time[i];
		if(time > 30000)
			m_time[i] = 0;
		if(m_time[i] != 0){
			float fade = (30000.0f - time) / 10000.0f;
			fade = Min(fade, 1.0f);
			RwIm3DVertexSetRGBA(&TempBufferRenderVertices[numVerts], 255, 255, 255, (int)(alpha*fade));
			RwIm3DVertexSetPos(&TempBufferRenderVertices[numVerts], m_pos[i].x, m_pos[i].y, m_pos[i].z);
			numVerts++;
		}
	}
	if(numVerts > 1){
		RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)TRUE);
		RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDSRCALPHA);
		RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDINVSRCALPHA);
		RwRenderStateSet(rwRENDERSTATETEXTURERASTER, nil);

		if(RwIm3DTransform(TempBufferRenderVertices, numVerts, nil, rwIM3D_VERTEXXYZ|rwIM3D_VERTEXRGBA)){
			RwIm3DRenderIndexedPrimitive(rwPRIMTYPELINELIST, TrailIndices, (numVerts-1)*2);
			RwIm3DEnd();
		}
	}
}

void
CPlaneTrail::RegisterPoint(CVector pos)
{
	int i;
	bool bNewPoint = false;
	if(m_time[0] != 0 && CTimer::GetTimeInMilliseconds() - m_time[0] > 2000){
		bNewPoint = true;
		for(i = ARRAY_SIZE(m_pos)-1; i > 0; i--){
			m_pos[i] = m_pos[i-1];
			m_time[i] = m_time[i-1];
		}
	}
	m_pos[0] = pos;
	if(bNewPoint || m_time[0] == 0)
		m_time[0] = CTimer::GetTimeInMilliseconds();
}

void
CPlaneTrails::Init(void)
{
	int i;
	for(i = 0; i < ARRAY_SIZE(aArray); i++)
		aArray[i].Init();
}

void
CPlaneTrails::Update(void)
{
	CVector planePos;

	planePos.x = 1590.0f * Sin((float)(CTimer::GetTimeInMilliseconds() & 0x1FFFF)/0x20000 * TWOPI);
	planePos.y = 1200.0f * Cos((float)(CTimer::GetTimeInMilliseconds() & 0x1FFFF)/0x20000 * TWOPI);
	planePos.z = 550.0f;
	RegisterPoint(planePos, 3);
	if(CClock::GetHours() > 22 || CClock::GetHours() < 7){
		if(CTimer::GetTimeInMilliseconds() & 0x200)
			CCoronas::RegisterCorona(101, 255, 0, 0, 255, planePos, 5.0f, 2000.0f,
				CCoronas::TYPE_NORMAL, CCoronas::FLARE_NONE, CCoronas::REFLECTION_OFF,
				CCoronas::LOSCHECK_OFF, CCoronas::STREAK_OFF, 0.0f);
		else
			CCoronas::UpdateCoronaCoors(101, planePos, 2000.0f, 0.0f);
	}

	planePos.x = 1000.0f * Sin((float)(CTimer::GetTimeInMilliseconds() & 0x1FFFF)/0x20000 * TWOPI);
	planePos.y = -1600.0f * Cos((float)(CTimer::GetTimeInMilliseconds() & 0x1FFFF)/0x20000 * TWOPI);
	planePos.z = 500.0f;
	RegisterPoint(planePos, 4);
	if(CClock::GetHours() > 22 || CClock::GetHours() < 7){
		if(CTimer::GetTimeInMilliseconds() & 0x200)
			CCoronas::RegisterCorona(102, 255, 0, 0, 255, planePos, 5.0f, 2000.0f,
				CCoronas::TYPE_NORMAL, CCoronas::FLARE_NONE, CCoronas::REFLECTION_OFF,
				CCoronas::LOSCHECK_OFF, CCoronas::STREAK_OFF, 0.0f);
		else
			CCoronas::UpdateCoronaCoors(102, planePos, 2000.0f, 0.0f);
	}

	planePos.x = 1100.0f * Cos((float)(CTimer::GetTimeInMilliseconds() & 0x1FFFF)/0x20000 * TWOPI);
	planePos.y = 700.0f * Sin((float)(CTimer::GetTimeInMilliseconds() & 0x1FFFF)/0x20000 * TWOPI);
	planePos.z = 600.0f;
	RegisterPoint(planePos, 5);
	if(CClock::GetHours() > 22 || CClock::GetHours() < 7){
		if(CTimer::GetTimeInMilliseconds() & 0x200)
			CCoronas::RegisterCorona(103, 255, 0, 0, 255, planePos, 5.0f, 2000.0f,
				CCoronas::TYPE_NORMAL, CCoronas::FLARE_NONE, CCoronas::REFLECTION_OFF,
				CCoronas::LOSCHECK_OFF, CCoronas::STREAK_OFF, 0.0f);
		else
			CCoronas::UpdateCoronaCoors(103, planePos, 2000.0f, 0.0f);
	}
}

void
CPlaneTrails::Render(void)
{
	int i;
	float visibility = Min(1.0f-CWeather::Foggyness, 1.0f-CWeather::CloudCoverage);
	visibility = Min(visibility, 1.0f-CWeather::Rain);
	visibility = Min(Max(Max(CTimeCycle::GetSkyTopRed(), CTimeCycle::GetSkyTopGreen()), CTimeCycle::GetSkyTopBlue())/256.0f, visibility);
	if(visibility > 0.0001f)
		for(i = 0; i < ARRAY_SIZE(aArray); i++)
			aArray[i].Render(visibility);
}

void
CPlaneTrails::RegisterPoint(CVector pos, uint32 id)
{
	aArray[id].RegisterPoint(pos);
}



CPlaneBanner CPlaneBanners::aArray[5];

void
CPlaneBanner::Init(void)
{
	int i;
	for(i = 0; i < ARRAY_SIZE(m_pos); i++){
		m_pos[i].x = i;
		m_pos[i].y = 0.0f;
		m_pos[i].z = -60.0f;
	}
}

void
CPlaneBanner::Update(void)
{
	int i;
	if(m_pos[0].z > -50.0f){
		m_pos[0].z -= 0.05f*CTimer::GetTimeStep();
		m_pos[0].z = Max(m_pos[0].z, -100.0f);
		for(i = 1; i < ARRAY_SIZE(m_pos); i++){
			CVector dist = m_pos[i] - m_pos[i-1];
			float len = dist.Magnitude();
			if(len > 8.0f)
				m_pos[i] = m_pos[i-1] + dist/len*8.0f;
		}
	}
}

void
CPlaneBanner::Render(void)
{
	int i;
	if(m_pos[0].z > -50.0f){
		float camDist = (TheCamera.GetPosition() - m_pos[0]).Magnitude();
		if(TheCamera.IsSphereVisible(m_pos[4], 32.0f) && camDist < 300.0f){
			TempBufferVerticesStored = 0;
			TempBufferIndicesStored = 0;
			int alpha = camDist < 250.0f ? 160 : (300.0f-camDist)/(300.0f-250.0f)*160;

			TempBufferVerticesStored += 2;
			RwIm3DVertexSetRGBA(&TempBufferRenderVertices[0], 255, 255, 255, alpha);
			RwIm3DVertexSetRGBA(&TempBufferRenderVertices[1], 255, 255, 255, alpha);
			RwIm3DVertexSetPos(&TempBufferRenderVertices[0], m_pos[2].x, m_pos[2].y, m_pos[2].z);
			RwIm3DVertexSetPos(&TempBufferRenderVertices[1], m_pos[2].x, m_pos[2].y, m_pos[2].z - 4.0f);
			RwIm3DVertexSetU(&TempBufferRenderVertices[0], 0.0f);
			RwIm3DVertexSetV(&TempBufferRenderVertices[0], 0.0f);
			RwIm3DVertexSetU(&TempBufferRenderVertices[1], 0.0f);
			RwIm3DVertexSetV(&TempBufferRenderVertices[1], 1.0f);
			for(i = 2; i < 8; i++){
				RwIm3DVertexSetRGBA(&TempBufferRenderVertices[TempBufferVerticesStored+0], 255, 255, 255, alpha);
				RwIm3DVertexSetRGBA(&TempBufferRenderVertices[TempBufferVerticesStored+1], 255, 255, 255, alpha);
				RwIm3DVertexSetPos(&TempBufferRenderVertices[TempBufferVerticesStored+0], m_pos[i].x, m_pos[i].y, m_pos[i].z);
				RwIm3DVertexSetPos(&TempBufferRenderVertices[TempBufferVerticesStored+1], m_pos[i].x, m_pos[i].y, m_pos[i].z - 4.0f);
				RwIm3DVertexSetU(&TempBufferRenderVertices[TempBufferVerticesStored+0], (i-2)/5.0f);
				RwIm3DVertexSetV(&TempBufferRenderVertices[TempBufferVerticesStored+0], 0.0f);
				RwIm3DVertexSetU(&TempBufferRenderVertices[TempBufferVerticesStored+1], (i-2)/5.0f);
				RwIm3DVertexSetV(&TempBufferRenderVertices[TempBufferVerticesStored+1], 1.0f);
				TempBufferRenderIndexList[TempBufferIndicesStored+0] = TempBufferVerticesStored-2;
				TempBufferRenderIndexList[TempBufferIndicesStored+1] = TempBufferVerticesStored-1;
				TempBufferRenderIndexList[TempBufferIndicesStored+2] = TempBufferVerticesStored+1;
				TempBufferRenderIndexList[TempBufferIndicesStored+3] = TempBufferVerticesStored-2;
				TempBufferRenderIndexList[TempBufferIndicesStored+4] = TempBufferVerticesStored+1;
				TempBufferRenderIndexList[TempBufferIndicesStored+5] = TempBufferVerticesStored;
				TempBufferVerticesStored += 2;
				TempBufferIndicesStored += 6;
			}
			RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)FALSE);
			RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*)TRUE);
			RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void*)TRUE);
			RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDSRCALPHA);
			RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDINVSRCALPHA);
			RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)TRUE);
			RwRenderStateSet(rwRENDERSTATETEXTURERASTER, RwTextureGetRaster(gpRubbishTexture[2]));

#ifdef FIX_BUGS
			if(RwIm3DTransform(TempBufferRenderVertices, TempBufferVerticesStored, nil, rwIM3D_VERTEXXYZ|rwIM3D_VERTEXUV|rwIM3D_VERTEXRGBA)){
#else
			if(RwIm3DTransform(TempBufferRenderVertices, TempBufferVerticesStored, nil, 0)){
#endif
				RwIm3DRenderIndexedPrimitive(rwPRIMTYPETRILIST, TempBufferRenderIndexList, TempBufferIndicesStored);
				RwIm3DEnd();
			}

			RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)TRUE);
			RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)FALSE);
			RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void*)FALSE);
			RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)FALSE);

			TempBufferVerticesStored = 0;
			TempBufferIndicesStored = 0;
		}
	}
}

void
CPlaneBanner::RegisterPoint(CVector pos)
{
	m_pos[0] = pos;
}

void
CPlaneBanners::Init(void)
{
	int i;
	for(i = 0; i < ARRAY_SIZE(aArray); i++)
		aArray[i].Init();
}

void
CPlaneBanners::Update(void)
{
	int i;
	for(i = 0; i < ARRAY_SIZE(aArray); i++)
		aArray[i].Update();
}

void
CPlaneBanners::Render(void)
{
	int i;
	for(i = 0; i < ARRAY_SIZE(aArray); i++)
		aArray[i].Render();
}

void
CPlaneBanners::RegisterPoint(CVector pos, uint32 id)
{
	aArray[id].RegisterPoint(pos);
}

bool CSmokeTrails::CigOn = false;
CSmokeTrail CSmokeTrails::aSmoke[3];

RwImVertexIndex SmokeTrailIndices[32] = { 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8,
9, 9, 10, 10, 11, 11, 12, 12, 13, 13, 14, 14, 15, 15, 16 };

float RandomSmoke[16] = { 10.0f, 5.0f, -1.0f, -9.0f, -7.0f, -1.0f, 0.0f, 3.0f, 6.0f, 7.0f, 4.0f, 2.0f,
5.0f, 7.0f };

uint8 ScrollCharSet[59][5] = {
	{ 0x00, 0x00, 0x00, 0x00, 0x00 }, // ' '
	{ 0x00, 0x00, 0x1D, 0x00, 0x00 }, // '!'
	{ 0x00, 0x00, 0x00, 0x00, 0x00 }, // '"'
	{ 0x0A, 0x1F, 0x0A, 0x1F, 0x0A }, // '#'
	{ 0x00, 0x09, 0x1F, 0x12, 0x00 }, // '$'
	{ 0x18, 0x18, 0x00, 0x03, 0x03 }, // '%'
	{ 0x00, 0x00, 0x00, 0x00, 0x00 }, // '&'
	{ 0x00, 0x00, 0x00, 0x00, 0x00 }, // '''
	{ 0x01, 0x02, 0x04, 0x08, 0x10 }, // '('
	{ 0x00, 0x00, 0x18, 0x00, 0x00 }, // ')'
	{ 0x15, 0x04, 0x1F, 0x04, 0x15 }, // '*'
	{ 0x00, 0x04, 0x0E, 0x04, 0x00 }, // '+'
	{ 0x00, 0x00, 0x03, 0x00, 0x00 }, // ','
	{ 0x00, 0x04, 0x04, 0x04, 0x00 }, // '-'
	{ 0x00, 0x00, 0x01, 0x00, 0x00 }, // '.'
	{ 0x00, 0x00, 0x00, 0x00, 0x00 }, // '/'
	{ 0x0E, 0x11, 0x11, 0x11, 0x0E }, // '0'
	{ 0x01, 0x09, 0x1F, 0x01, 0x01 }, // '1'
	{ 0x03, 0x15, 0x15, 0x15, 0x09 }, // '2'
	{ 0x11, 0x11, 0x15, 0x15, 0x0A }, // '3'
	{ 0x02, 0x06, 0x0A, 0x1F, 0x02 }, // '4'
	{ 0x1D, 0x15, 0x15, 0x15, 0x12 }, // '5'
	{ 0x0E, 0x15, 0x15, 0x15, 0x12 }, // '6'
	{ 0x18, 0x10, 0x13, 0x14, 0x18 }, // '7'
	{ 0x0A, 0x15, 0x15, 0x15, 0x0A }, // '8'
	{ 0x08, 0x15, 0x15, 0x15, 0x0E }, // '9'
	{ 0x00, 0x00, 0x0A, 0x00, 0x00 }, // ':'
	{ 0x18, 0x18, 0x00, 0x03, 0x03 }, // ';'
	{ 0x04, 0x08, 0x1F, 0x08, 0x04 }, // '<'
	{ 0x00, 0x0A, 0x0A, 0x0A, 0x00 }, // '='
	{ 0x04, 0x02, 0x1F, 0x02, 0x04 }, // '>'
	{ 0x10, 0x10, 0x15, 0x14, 0x1D }, // '?'
	{ 0x00, 0x1C, 0x14, 0x1C, 0x00 }, // '@'
	{ 0x0F, 0x12, 0x12, 0x12, 0x0F }, // 'A'
	{ 0x1F, 0x15, 0x15, 0x15, 0x0A }, // 'B'
	{ 0x0E, 0x11, 0x11, 0x11, 0x0A }, // 'C'
	{ 0x1F, 0x11, 0x11, 0x11, 0x0E }, // 'D'
	{ 0x1F, 0x15, 0x15, 0x11, 0x11 }, // 'E'
	{ 0x1F, 0x14, 0x14, 0x10, 0x10 }, // 'F'
	{ 0x0E, 0x11, 0x15, 0x15, 0x06 }, // 'G'
	{ 0x1F, 0x04, 0x04, 0x04, 0x1F }, // 'H'
	{ 0x11, 0x11, 0x1F, 0x11, 0x11 }, // 'I'
	{ 0x02, 0x01, 0x01, 0x01, 0x1E }, // 'J'
	{ 0x1F, 0x04, 0x0C, 0x12, 0x01 }, // 'K'
	{ 0x1F, 0x01, 0x01, 0x01, 0x01 }, // 'L'
	{ 0x1F, 0x08, 0x06, 0x08, 0x1F }, // 'M'
	{ 0x1F, 0x08, 0x04, 0x02, 0x1F }, // 'N'
	{ 0x0E, 0x11, 0x11, 0x11, 0x0E }, // 'O'
	{ 0x1F, 0x12, 0x12, 0x12, 0x0C }, // 'P'
	{ 0x0C, 0x12, 0x12, 0x13, 0x0D }, // 'Q'
	{ 0x1F, 0x14, 0x14, 0x16, 0x09 }, // 'R'
	{ 0x09, 0x15, 0x15, 0x15, 0x02 }, // 'S'
	{ 0x10, 0x10, 0x1F, 0x10, 0x10 }, // 'T'
	{ 0x1E, 0x01, 0x01, 0x01, 0x1E }, // 'U'
	{ 0x1C, 0x02, 0x01, 0x02, 0x1C }, // 'V'
	{ 0x1E, 0x01, 0x06, 0x01, 0x1E }, // 'W'
	{ 0x11, 0x0A, 0x04, 0x0A, 0x11 }, // 'X'
	{ 0x18, 0x04, 0x03, 0x04, 0x18 }, // 'Y'
	{ 0x11, 0x13, 0x15, 0x19, 0x11 }  // 'Z'
};

// ---------- CMovingThings ----------
enum eScrollBarTypes
{
	SCROLL_ARENA_STRING
};

CScrollBar    aScrollBars[1];

CMovingThing CMovingThings::StartCloseList;
CMovingThing CMovingThings::EndCloseList;
int16 CMovingThings::Num;
CMovingThing CMovingThings::aMovingThings[NUMMOVINGTHINGS];

int32 CScrollBar::TonightsEvent;
 
void CMovingThings::Init()
{
	StartCloseList.m_pNext = &CMovingThings::EndCloseList;
	StartCloseList.m_pPrev = nil;
	EndCloseList.m_pNext = nil;
	EndCloseList.m_pPrev = &CMovingThings::StartCloseList;

	CPlaneTrails::Init();
	CSmokeTrails::Init();
	CPlaneBanners::Init();
	CPointLights::Init();

	Num = 0;

	for (int32 i = 0; i < NUMMOVINGTHINGS; i++) {
		aMovingThings[i].m_nType = 0;
		aMovingThings[i].m_farAway = 0;
	}

	for (int i = 0; i < NUMSECTORS_X; i++) {
		for (int j = 0; j < NUMSECTORS_Y; j++) {
			for (CPtrNode *pNode = CWorld::GetSector(i, j)->m_lists[ENTITYLIST_BUILDINGS].first; pNode; pNode = pNode->next) {
				CEntity *pEntity = (CEntity *)pNode->item;
				PossiblyAddThisEntity(pEntity);
			}
		}
	}

	for (int32 i = 0; i < NUM_LEVELS; i++) {
		for (CPtrNode *pNode = CWorld::GetBigBuildingList((eLevelName)i).first; pNode; pNode = pNode->next) {
			CEntity *pEntity = (CEntity *)pNode->item;
			PossiblyAddThisEntity(pEntity);
		}
	}

	CEscalators::Init();
	aScrollBars[0].Init(CVector(-1069.209f, 1320.126f, 18.848f), CVector(-1069.209f, 1342.299f, 22.612f), SCROLL_ARENA_STRING, 128, 255, 0, 0.3f);
}

void CMovingThings::Shutdown()
{
	
	aScrollBars[0].SetVisibility(false);
	CEscalators::Shutdown();
}

void CMovingThings::Update()
{
	CPlaneBanners::Update();
	CPlaneTrails::Update();
	CEscalators::Update();

	const int TIME_SPAN = 8; // frames to process all aMovingThings

	int16 i;

	int block = CTimer::GetFrameCounter() % TIME_SPAN;

	for (i = (block * NUMMOVINGTHINGS) / TIME_SPAN; i < ((block + 1) * NUMMOVINGTHINGS) / TIME_SPAN; i++) {
		if (aMovingThings[i].m_farAway == 1)
			aMovingThings[i].Update();
	}

	for (i = 0; i < CMovingThings::Num; i++) {
		if (aMovingThings[i].m_farAway == 0)
			aMovingThings[i].Update();
	}

	for (i = 0; i < ARRAY_SIZE(aScrollBars); ++i)
	{
		if (aScrollBars[i].IsVisible() || (CTimer::GetFrameCounter() + i) % 8 == 0)
			aScrollBars[i].Update();
	}
}

void CMovingThings::Render()
{
	CSmokeTrails::Update();

	int i;
	for (i = 0; i < ARRAY_SIZE(aScrollBars); ++i)
	{
		if (aScrollBars[i].IsVisible())
			aScrollBars[i].Render();
	}

	CPlaneTrails::Render();
	CSmokeTrails::Render();
	CPlaneBanners::Render();
}

void CMovingThings::RegisterOne(CEntity *pEnt, uint16 nType) {
	if (Num >= NUMMOVINGTHINGS)
		return;

	aMovingThings[Num].m_pEntity = pEnt;
	aMovingThings[Num].m_nType = nType;
	aMovingThings[Num].m_farAway = 0;
	aMovingThings[Num].m_vecPosn = pEnt->GetPosition();
	aMovingThings[Num].AddToList(&CMovingThings::StartCloseList);
	Num++;
}

void CMovingThings::PossiblyAddThisEntity(CEntity *pEnt) {
	if (pEnt->GetModelIndex() == MI_LIGHTBEAM) {
		RegisterOne(pEnt, 1);
	}
	else if (pEnt->GetModelIndex() == MI_AIRPORTRADAR) {
		RegisterOne(pEnt, 2);
	}
	else if (pEnt->GetModelIndex() == MI_MALLFAN || pEnt->GetModelIndex() == MI_HOTELFAN_NIGHT
		|| pEnt->GetModelIndex() == MI_HOTELFAN_DAY || pEnt->GetModelIndex() == MI_HOTROOMFAN) {
		RegisterOne(pEnt, 3);
	}
	else if (pEnt->GetModelIndex() == MI_BLIMP_NIGHT || pEnt->GetModelIndex() == MI_BLIMP_DAY) {
		RegisterOne(pEnt, 4);
	}
}

// ---------- CMovingThing ----------
static float maxUpdateDists[5] = { 100.0f, 1500.0f, 400.0f, 100.0f, 2000.0f };

void CMovingThing::Update()
{
	switch (m_nType) {
	case 1: {
		float angle = (CTimer::GetTimeInMilliseconds() % 0x3FFF) * TWOPI / 0x3FFF;
		float s = Sin(angle);
		float c = Cos(angle);
		m_pEntity->GetRight() = CVector(-s, c, 0.0f);
		m_pEntity->GetForward() = CVector(0.0f, 0.0f, 1.0f);
		m_pEntity->GetUp() = CVector(c, s, 0.0f);

		if (CClock::GetHours() >= 20 || CClock::GetHours() < 5) {
			if (Abs(TheCamera.GetPosition().x - m_pEntity->GetPosition().x) < 600.0f &&
				Abs(TheCamera.GetPosition().y - m_pEntity->GetPosition().y) < 600.0f) {
				CVector delta = m_pEntity->GetPosition() - TheCamera.GetPosition();
				delta /= delta.Magnitude();

				if (DotProduct(delta, CVector(c, s, 0.0f)) < -0.92f) {
					CVector coors = m_pEntity->GetPosition() - 10.0f * delta;
					CCoronas::RegisterCorona(43, 128, 128, 100, 255, coors, 70.0f, 600.0f, 0.0f, CCoronas::TYPE_STAR, CCoronas::REFLECTION_OFF, CCoronas::LOSCHECK_OFF, CCoronas::STREAK_OFF, 0.0f);
				}
			}
		}
	}
		break;
	case 2: {
		float angle = (CTimer::GetTimeInMilliseconds() % 0x7FF) * TWOPI / 0x7FF;
		float s = Sin(angle);
		float c = Cos(angle);
		m_pEntity->GetRight() = CVector(c, s, 0.0f);
		m_pEntity->GetForward() = CVector(-s, c, 0.0f);
		m_pEntity->GetUp() = CVector(0.0f, 0.0f, 1.0f);
	}
		break;
	case 3: {
		float angle = (CTimer::GetTimeInMilliseconds() % 0x3FF) * TWOPI / 0x3FF;
		float s = Sin(angle);
		float c = Cos(angle);
		m_pEntity->GetRight() = CVector(c, s, 0.0f);
		m_pEntity->GetForward() = CVector(-s, c, 0.0f);
		m_pEntity->GetUp() = CVector(0.0f, 0.0f, 1.0f);
	}
		break;
	case 4: {
		float angle = (CTimer::GetTimeInMilliseconds() % 0x3FFFF) * TWOPI / 0x3FFFF;
		float s = Sin(angle);
		float c = Cos(angle);
		m_pEntity->GetRight() = CVector(-c, -s, 0.0f);
		m_pEntity->GetForward() = CVector(s, -c, 0.0f);
		m_pEntity->GetUp() = CVector(0.0f, 0.0f, 1.0f);
		m_pEntity->SetPosition(CVector(350.0f * c - 465.0f, 350.0f * s + 1163.0f, 260.0f));
	}
		break;
	default:
		break;
	}

	m_pEntity->GetMatrix().UpdateRW();
	m_pEntity->UpdateRwFrame();
	
	if (SQR(m_pEntity->GetPosition().x - TheCamera.GetPosition().x) + SQR(m_pEntity->GetPosition().y - TheCamera.GetPosition().y) < SQR(maxUpdateDists[m_nType])) {
		if (m_farAway == 1) {
			AddToList(&CMovingThings::StartCloseList);
			m_farAway = 0;
		}
	}
	else {
		if (m_farAway == 0) {
			RemoveFromList();
			m_farAway = 1;
		}
	}
}

void CMovingThing::AddToList(CMovingThing *pThing)
{
	m_pNext = pThing->m_pNext;
	m_pPrev = pThing;
	pThing->m_pNext = this;
	m_pNext->m_pPrev = this;
}

void CMovingThing::RemoveFromList()
{
	m_pNext->m_pPrev = m_pPrev;
	m_pPrev->m_pNext = m_pNext;
}

int16 CMovingThing::SizeList()
{
	CMovingThing *next = m_pNext;
	int16 count = 0;

	while (next != nil) {
		next = next->m_pNext;
		count++;
	}

	return count;
}

char String_Time[] = "THE TIME IS 12:34    ";
const char* FindTimeMessage()
{
	String_Time[12] = '0' + CClock::GetHours() / 10;
	String_Time[13] = '0' + CClock::GetHours() % 10;
	String_Time[15] = '0' + CClock::GetMinutes() / 10;
	String_Time[16] = '0' + CClock::GetMinutes() % 10;
	return String_Time;
}

// ---------- CScrollBar ----------
void CScrollBar::Init(CVector pos1, CVector pos2, uint8 type, uint8 red, uint8 green, uint8 blue, float scale)
{
	for (int i = 0; i < ARRAY_SIZE(m_MessageBar); ++i)
		m_MessageBar[i] = 0;

	m_pMessage = ". ";
	m_MessageCurrentChar = 0;
	m_MessageLength = strlen(m_pMessage);

	m_Counter  = 0;
	m_bVisible = false;
	m_Position = pos1;
	m_Type     = type;
	m_Size.x   = (pos2.x - pos1.x) * 0.025f;
	m_Size.y   = (pos2.y - pos1.y) * 0.025f;
	m_Size.z   = (pos2.z - pos1.z) * 0.2f;
	m_uRed     = red;
	m_uGreen   = green;
	m_uBlue    = blue;
	m_fScale   = scale;
}

void CScrollBar::Update()
{
	float distanceFromCamera = (TheCamera.GetPosition() - m_Position).Magnitude();
	if (distanceFromCamera > 100.0f)
	{
		m_bVisible = false;
		return;
	}

	m_bVisible = true;

	if (distanceFromCamera < 75.0f)
		m_fIntensity = 1.0f;
	else
		m_fIntensity = 1.0f - 4.0f * (distanceFromCamera - 75.0f) / 100.0f;

	m_Counter = (m_Counter + 1) % 8;
		
	// if message is fully printed, load up the next one
	if (m_Counter == 0 && ++m_MessageCurrentChar >= m_MessageLength)
	{
		const char* previousMessage = m_pMessage;
		if (m_Type == SCROLL_ARENA_STRING) {
			while (previousMessage == m_pMessage)
			{
				switch (CGeneral::GetRandomNumber() % 4)
				{
				case 0:
					switch (TonightsEvent) {
					case 0:
						m_pMessage = "MAIN EVENT TONIGHT: CAR RACING . . .   ";
						break;
					case 1:
						m_pMessage = "MAIN EVENT TONIGHT: DESTRUCTION DERBY . . .   ";
						break;
					case 2:
						m_pMessage = "MAIN EVENT TONIGHT: BIKE RACING . . .   ";
						break;
					}
					break;
				case 1:
					switch (TonightsEvent) {
					case 0:
						m_pMessage = "FOR TICKETS TO THE HOT RING EVENT CALL 555-3764 . . .   ";
						break;
					case 1:
						m_pMessage = "FOR TICKETS TO THE BLOOD RING EVENT CALL 555-3765 . . .   ";
						break;
					case 2:
						m_pMessage = "FOR TICKETS TO THE DIRT RING EVENT CALL 555-3766 . . .   ";
						break;
					}
					break;
				case 2:
					m_pMessage = "HYMAN MEMORIAL STADIUM. HOME TO SOME OF THE BIGGEST EVENTS OF"
						" THE WESTERN HEMISPHERE. ALSO AVAILABLE FOR CHILDREN PARTIES. . . ";
					break;
				case 3:
					m_pMessage = FindTimeMessage();
					break;
				default:
					break;
				}
			}
		}

		m_MessageLength = (uint32)strlen(m_pMessage);
		m_MessageCurrentChar = 0;
	}

	// Scroll
	for (int i = 0; i < ARRAY_SIZE(m_MessageBar)-1; i++)
		m_MessageBar[i] = m_MessageBar[i + 1];
	m_MessageBar[ARRAY_SIZE(m_MessageBar)-1] = m_Counter < 5 ? ScrollCharSet[m_pMessage[m_MessageCurrentChar] - ' '][m_Counter] : 0;

	// Introduce some random displaying glitches; signs aren't supposed to be perfect :P
	switch (CGeneral::GetRandomNumber() & 0xFF)
	{
	case 0x0D: m_MessageBar[ARRAY_SIZE(m_MessageBar)-1] = 0;                 break;
	case 0xE3: m_MessageBar[ARRAY_SIZE(m_MessageBar)-1] = 0xE3;              break;
	case 0x64: m_MessageBar[ARRAY_SIZE(m_MessageBar)-1] = ~m_MessageBar[ARRAY_SIZE(m_MessageBar)-1]; break;
	}
}

void CScrollBar::Render()
{
	if (!TheCamera.IsSphereVisible(m_Position, 2.0f * 20.0f * (ABS(m_Size.x) + ABS(m_Size.y))))
		return;

	CSprite::InitSpriteBuffer();

	// Calculate intensity of colours
	uint8 r = m_fIntensity * m_uRed;
	uint8 g = m_fIntensity * m_uGreen;
	uint8 b = m_fIntensity * m_uBlue;

	// Set render states
	RwRenderStateSet(rwRENDERSTATEZWRITEENABLE,      (void*)FALSE);
	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)TRUE);
	RwRenderStateSet(rwRENDERSTATESRCBLEND,          (void*)rwBLENDONE);
	RwRenderStateSet(rwRENDERSTATEDESTBLEND,         (void*)rwBLENDONE);
	RwRenderStateSet(rwRENDERSTATETEXTUREFILTER,     (void*)rwFILTERLINEAR);
	RwRenderStateSet(rwRENDERSTATETEXTURERASTER,     RwTextureGetRaster(gpCoronaTexture[0]));
	RwRenderStateSet(rwRENDERSTATEZTESTENABLE,       (void*)TRUE);

	CVector coronaCoord, screenCoord;
	float screenW, screenH;
	for (int i = 1; i < ARRAY_SIZE(m_MessageBar); ++i)
	{
		for (int j = 0; j < 5; ++j)
		{
			coronaCoord.x = m_Position.x + m_Size.x * i;
			coronaCoord.y = m_Position.y + m_Size.y * i;
			coronaCoord.z = m_Position.z + m_Size.z * j;

			// Render main coronas
			if (m_MessageBar[i] & (1 << j))
			{
				if (CSprite::CalcScreenCoors(coronaCoord, &screenCoord, &screenW, &screenH, true))
				{
					CSprite::RenderBufferedOneXLUSprite(
						screenCoord.x, screenCoord.y, screenCoord.z,
						screenW * m_fScale, screenH * m_fScale,
						r, g, b,
						255, 1.0f / screenCoord.z, 255);
				}
			}
			// Render smaller and faded coronas for a trailing effect
			else if (m_MessageBar[i - 1] & (1 << j))
			{
				if (CSprite::CalcScreenCoors(coronaCoord, &screenCoord, &screenW, &screenH, true))
				{
					CSprite::RenderBufferedOneXLUSprite(
						screenCoord.x, screenCoord.y, screenCoord.z,
						screenW * m_fScale * 0.8f,
						screenH * m_fScale * 0.8f,
						r / 2,
						g / 2,
						b / 2,
						255, 1.0f / screenCoord.z, 255);
				}
			}
		}
	}

	CSprite::FlushSpriteBuffer();
}

void
CSmokeTrail::RegisterPoint(CVector regPosition, float opacity) {
	bool bAddedNewPoint = false;

	if (m_time[0] && CTimer::GetTimeInMilliseconds() - m_time[0] > 150) {
		bAddedNewPoint = true;
		for (int32 i = 15; i > 0; i--) {
			m_pos[i] = m_pos[i - 1];
			m_time[i] = m_time[i - 1];
			m_opacity[i] = m_opacity[i - 1];
		}
		++m_seed;
	}
	m_pos[0] = regPosition;

	if (bAddedNewPoint || !m_time[0]) {
		m_time[0] = CTimer::GetTimeInMilliseconds();
		float density = 0.1f / (m_pos[1] - m_pos[2]).Magnitude();
		m_opacity[1] = opacity * Min(density, 1.0f);
	}
	m_opacity[0] = 0.0f;
}

void
CSmokeTrail::Init(int num) {
	for (int32 i = 0; i < 16; i++)
		m_time[i] = 0;
	m_seed = num * 2;
}

void
CSmokeTrails::Init(void) {
	for (int32 i = 0; i < 3; i++)
		aSmoke[i].Init(i);
}

void
CSmokeTrails::Render(void) {
	for (int32 i = 0; i < 3; i++)
		aSmoke[i].Render();
}

void
CSmokeTrail::Render(void) {
	int numVerts = 0;

	if (TheCamera.IsSphereVisible(m_pos[0], 10.0f)) {
		for (int32 i = 0; i < 16; i++) {
			int timeSinceSpawned = CTimer::GetTimeInMilliseconds() - m_time[i];

			if (timeSinceSpawned > 2250)
				m_time[i] = 0;

			if (m_time[i]) {
				int alpha = (1.0f - timeSinceSpawned / 2250.0f) * 110.0f * m_opacity[i];
				float offset = timeSinceSpawned * CWeather::Wind * 0.0001f;
				float posX = (m_pos[i].x + timeSinceSpawned * RandomSmoke[(i - m_seed) & 0xF] * 0.00001f) - offset;
				float posY = (m_pos[i].y + timeSinceSpawned * RandomSmoke[(i - m_seed + 5) & 0xF] * 0.00001f) - offset;
				float posZ = m_pos[i].z + timeSinceSpawned * 0.0004f;
				RwIm3DVertexSetRGBA(&TempBufferRenderVertices[i], 200, 200, 200, alpha);
				RwIm3DVertexSetPos(&TempBufferRenderVertices[i], posX, posY, posZ);
				numVerts++;
			}
		}
	}

	if (numVerts > 1) {
		RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)TRUE);
		RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDSRCALPHA);
		RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDINVSRCALPHA);
		RwRenderStateSet(rwRENDERSTATETEXTURERASTER, nil);

		if (RwIm3DTransform(TempBufferRenderVertices, numVerts, nil, rwIM3D_VERTEXXYZ | rwIM3D_VERTEXRGBA)) {
			RwIm3DRenderIndexedPrimitive(rwPRIMTYPEPOLYLINE, SmokeTrailIndices, 2 * (numVerts - 1));
			RwIm3DEnd();
		}
	}
}

void
CSmokeTrails::Update(void) {

	if (!CSmokeTrails::CigOn || TheCamera.Using1stPersonWeaponMode() || !FindPlayerPed() ||
		FindPlayerVehicle() || CCutsceneMgr::IsRunning() || !FindPlayerPed()->GetClump())
		return;

	RwV3d startPos = { 0.026f, 0.15f, 0.02f };
	RwV3d endPos = { 0.026f, 0.05f, 0.02f };

	RpHAnimHierarchy *hier = GetAnimHierarchyFromSkinClump(FindPlayerPed()->GetClump());
	int32 idx = RpHAnimIDGetIndex(hier, ConvertPedNode2BoneTag(PED_HEAD));
	RwMatrix *head = &RpHAnimHierarchyGetMatrixArray(hier)[idx];
	RwV3dTransformPoints(&startPos, &startPos, 1, head);
	RwV3dTransformPoints(&endPos, &endPos, 1, head);

	aSmoke[0].RegisterPoint(startPos, 1.0f);
	aSmoke[1].RegisterPoint(startPos, 0.75f);
	aSmoke[2].RegisterPoint(startPos, 0.5f);

	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)FALSE);
	RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDSRCALPHA);
	RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDINVSRCALPHA);
	RwRenderStateSet(rwRENDERSTATETEXTURERASTER, nil);

	RwIm3DVertexSetRGBA(&TempBufferRenderVertices[0], 255, 255, 255, 255);
	RwIm3DVertexSetPos(&TempBufferRenderVertices[0], startPos.x, startPos.y, startPos.z);
	RwIm3DVertexSetRGBA(&TempBufferRenderVertices[1], 255, 255, 255, 255);
	RwIm3DVertexSetPos(&TempBufferRenderVertices[1], endPos.x, endPos.y, endPos.z);

	if (RwIm3DTransform(TempBufferRenderVertices, 2, nil, rwIM3D_VERTEXXYZ | rwIM3D_VERTEXRGBA)) {
		RwIm3DRenderIndexedPrimitive(rwPRIMTYPEPOLYLINE, SmokeTrailIndices, 2);
		RwIm3DEnd();
	}
}

CEscalator CEscalators::aEscalators[NUM_ESCALATORS];
int32 CEscalators::NumEscalators;

CEscalator::CEscalator() {
	m_bIsActive = false;

	for (int i = 0; i < 24; i++) {
		m_pSteps[i] = nil;
	}
}

void
CEscalator::AddThisOne(CVector pos0, CVector pos1, CVector pos2, CVector pos3, bool b_isMovingDown) {
	m_pos0 = pos0;
	m_pos1 = pos1;
	m_pos2 = pos2;
	m_pos3 = pos3;

	float escalatorStepHeight = CModelInfo::GetModelInfo(MI_ESCALATORSTEP)->GetColModel()->boundingBox.max.z;
	m_pos0.z -= escalatorStepHeight;
	m_pos1.z -= escalatorStepHeight;
	m_pos2.z -= escalatorStepHeight;
	m_pos3.z -= escalatorStepHeight;

	float magnitudes[3];
	magnitudes[0] = (m_pos0 - m_pos1).Magnitude();
	magnitudes[1] = (m_pos1 - m_pos2).Magnitude();
	magnitudes[2] = (m_pos2 - m_pos3).Magnitude();

	float length = magnitudes[0] + magnitudes[1] + magnitudes[2];

	m_lowerEnd = magnitudes[0] / length;
	m_upperEnd = (magnitudes[0] + magnitudes[1]) / length;

	m_stepsCount = Max(24.0f, length / 0.6f);

	CVector direction(m_pos0.x - m_pos1.x, m_pos0.y - m_pos1.y, 0.0f);
	direction.Normalise();

	m_matrix.GetUp() = CVector(0.0f, 0.0f, 1.0f);
	m_matrix.GetForward() = CVector(direction.x, direction.y, 0.0f);
	m_matrix.GetRight() = CVector(direction.y, -direction.x, 0.0f);
	m_matrix.GetPosition() = CVector(0.0f, 0.0f, 0.0f);

	m_bIsMovingDown = b_isMovingDown;

	m_midPoint = (m_pos0 + m_pos3) / 2.0f;

	m_radius = (m_pos0 - m_midPoint).Magnitude();
}

void
CEscalator::Update(void) {
	if (!m_bIsActive) {
		if ((TheCamera.GetPosition() - m_midPoint).Magnitude() < 25.0f) {
			if (TheCamera.IsSphereVisible(m_midPoint, m_radius) && (m_stepsCount + 10 < CPools::GetObjectPool()->GetNoOfFreeSpaces())) { 
				m_bIsActive = true;
				for (int i = 0; i < m_stepsCount; i++) {
					m_pSteps[i] = new CObject(MI_ESCALATORSTEP, TRUE);
					if (m_pSteps[i]) {
						m_pSteps[i]->SetPosition(m_pos1);
						CWorld::Add(m_pSteps[i]);
						m_pSteps[i]->ObjectCreatedBy = CONTROLLED_SUB_OBJECT;
					}
				}
			}
		}
	}

	if (m_bIsActive) {
		float time = (CTimer::GetTimeInMilliseconds() % 16384) / 16384.0f;
		for (int i = 0; i < m_stepsCount; i++) {
			if (m_pSteps[i]) {
				float t = i / (float)m_stepsCount + time;

				if (t > 1.0f)
					t -= 1.0f;

				if (m_bIsMovingDown)
					t = 1.0f - t;

				CVector oldPosition = m_pSteps[i]->GetPosition();
				m_pSteps[i]->GetMatrix() = m_matrix;

				CVector newPosition;
				if (t < m_lowerEnd) {
					float ratio = t / m_lowerEnd;
					newPosition = (ratio * m_pos1) + ((1.0f - ratio) * m_pos0);
				}
				else if (t < m_upperEnd) {
					float ratio = (t - m_lowerEnd) / (m_upperEnd - m_lowerEnd);
					newPosition = (ratio * m_pos2) + ((1.0f - ratio) * m_pos1);
				}
				else {
					float ratio = (t - m_upperEnd) / (1.0f - m_upperEnd);
					newPosition = (ratio * m_pos3) + ((1.0f - ratio) * m_pos2);
				}

				m_pSteps[i]->SetPosition(newPosition);
				m_pSteps[i]->m_vecMoveSpeed = (newPosition - oldPosition) / Max(CTimer::GetTimeStep(), 1.0f);
				m_pSteps[i]->GetMatrix().UpdateRW();
				m_pSteps[i]->UpdateRwFrame();
			}
			if ((TheCamera.GetPosition() - m_midPoint).Magnitude() > 28.0f || !TheCamera.IsSphereVisible(m_midPoint, m_radius))
				SwitchOff();
		}
	}
}

bool deletingEscalator;

void
CEscalator::SwitchOff(void) {
	if (m_bIsActive) {
		for (int i = 0; i < m_stepsCount; i++) {
			if (m_pSteps[i]) {
				CWorld::Remove(m_pSteps[i]);
				deletingEscalator = true;
				delete m_pSteps[i];
				m_pSteps[i] = nil;
				deletingEscalator = false;
			}
		}
		m_bIsActive = false;
	}
}

void
CEscalators::AddOne(CVector pos0, CVector pos1, CVector pos2, CVector pos3, bool b_isMovingDown) {
	aEscalators[NumEscalators++].AddThisOne(pos0, pos1, pos2, pos3, b_isMovingDown);
}

void
CEscalators::Init(void) {
	Shutdown();
	NumEscalators = 0;

	AddOne(CVector(-9.82999f, -938.04498f, 9.4219f), CVector(-8.573f, -938.04498f, 9.4219f),
		CVector(-0.747f, -938.045f, 15.065f), CVector(0.88f, -938.045f, 15.065f), TRUE);

	AddOne(CVector(-9.83f, -939.966f, 9.422f), CVector(-8.573f, -939.966f, 9.422f),
		CVector(-0.747f, -939.966f, 15.065f), CVector(0.880f, -939.966f, 15.065f), FALSE);

	AddOne(CVector(408.116f, 1058.36f, 18.261f), CVector(408.094f, 1057.04f, 18.261f),
		CVector(408.116f, 1048.0f, 24.765f), CVector(408.094f, 1046.57f, 24.799f), TRUE);

	AddOne(CVector(406.195f, 1058.36f, 18.261f), CVector(406.173f, 1057.04f, 18.261f),
		CVector(406.195f, 1048.0f, 24.729f), CVector(406.173f, 1046.57f, 24.79f), FALSE);

	AddOne(CVector(421.729f, 1058.3789f, 18.075f), CVector(421.707f, 1057.052f, 18.099f),
		CVector(421.729f, 1048.016f, 24.604f), CVector(421.707f, 1046.589f, 24.637f), TRUE);

	AddOne(CVector(419.808f, 1058.378f, 18.099f), CVector(419.786f, 1057.052f, 18.099f),
		CVector(419.808f, 1048.016f, 24.568f), CVector(419.786f, 1046.589f, 24.637f), FALSE);

	AddOne(CVector(412.69901f, 1102.729f, 17.569f), CVector(412.72198f, 1104.057f, 17.57f),
		CVector(412.69901f, 1113.092f, 24.073f), CVector(412.72198f, 1114.3201f, 24.108f), TRUE);

	AddOne(CVector(414.62f, 1102.729f, 17.569f), CVector(414.64301f, 1104.057f, 17.57f),
		CVector(414.62f, 1113.092f, 24.037001f), CVector(414.64301f, 1114.3201f, 24.099001f), FALSE);

	AddOne(CVector(414.64301f, 1145.589f, 17.57f), CVector(414.62f, 1144.261f, 17.569f),
		CVector(414.64301f, 1135.226f, 24.073999f), CVector(414.62f, 1133.798f, 24.107f), TRUE);

	AddOne(CVector(412.72198f, 1145.589f, 17.57f), CVector(412.69901f, 1144.261f, 17.569f),
		CVector(412.72198f, 1135.226f, 24.038f), CVector(412.69901f, 1133.798f, 24.098f), FALSE);

	AddOne(CVector(406.05099f, 1193.4771f, 18.016001f), CVector(406.07401f, 1194.8051f, 18.017f),
		CVector(406.05099f, 1203.84f, 24.52f), CVector(406.07401f, 1205.2679f, 24.555f), TRUE);

	AddOne(CVector(407.97198f, 1193.4771f, 18.016001f), CVector(407.995f, 1194.8051f, 18.017f),
		CVector(407.97198f, 1203.84f, 24.483999f), CVector(407.995f, 1205.2679f, 24.546f), FALSE);

	AddOne(CVector(419.659f, 1193.479f, 17.979f), CVector(419.68201f, 1194.807f, 17.98f),
		CVector(419.659f, 1203.842f, 24.483f), CVector(419.68201f, 1205.27f, 24.518f), TRUE);

	AddOne(CVector(421.57999f, 1193.479f, 17.979f), CVector(421.603f, 1194.807f, 17.98f),
		CVector(421.57999f, 1203.842f, 24.447001f), CVector(421.603f, 1205.27f, 24.509001f), FALSE);

	AddOne(CVector(406.23199f, 1022.857f, 17.917f), CVector(406.23199f, 1024.1851f, 17.917f),
		CVector(406.23199f, 1033.22f, 24.521f), CVector(406.23199f, 1034.647f, 24.555f), TRUE);

	AddOne(CVector(408.15302f, 1022.857f, 17.917f), CVector(408.15302f, 1024.1851f, 17.916f),
		CVector(408.15302f, 1033.22f, 24.486f), CVector(408.15302f, 1034.647f, 24.52f), FALSE);

	AddOne(CVector(-1506.39f, -813.13f, 13.834f), CVector(-1506.177f, -814.51703f, 13.834f),
		CVector(-1504.566f, -823.20898f, 19.836f), CVector(-1504.329f, -824.48499f, 19.837f), FALSE);

	AddOne(CVector(-1481.951f, -859.05402f, 13.834f), CVector(-1482.7791f, -858.22498f, 13.834f),
		CVector(-1489.03f, -851.974f, 19.836f), CVector(-1489.948f, -851.05701f, 19.837f), TRUE);

	AddOne(CVector(-1461.743f, -871.35901f, 13.834f), CVector(-1460.62f, -871.69202f, 13.834f),
		CVector(-1452.144f, -874.20203f, 19.836f), CVector(-1450.9f, -874.57098f, 19.837f), FALSE);

	AddOne(CVector(-1409.889f, -871.41498f, 13.834f), CVector(-1411.0129f, -871.74701f, 13.834f),
		CVector(-1419.489f, -874.258f, 19.836f), CVector(-1420.733f, -874.62701f, 19.837f), TRUE);

	AddOne(CVector(-1389.577f, -858.89301f, 13.834f), CVector(-1388.7271f, -858.08698f, 13.834f),
		CVector(-1382.314f, -852.00201f, 19.836f), CVector(-1381.373f, -851.10797f, 19.837f), FALSE);

	AddOne(CVector(-1364.981f, -813.13f, 13.834f), CVector(-1365.204f, -814.28003f, 13.834f),
		CVector(-1366.891f, -822.95801f, 19.83f), CVector(-1367.139f, -824.23199f, 19.837f), TRUE);

	for (int i = 0; i < NUM_ESCALATORS; i++) {
		aEscalators[i].SwitchOff();
	}
}

void
CEscalators::Update(void) {
	if (CReplay::IsPlayingBack())
		return;

	for (int i = 0; i < NUM_ESCALATORS; i++) {
		aEscalators[i].Update();
	}
}

void
CEscalators::Shutdown(void) {
	for (int i = 0; i < NUM_ESCALATORS; i++) {
		aEscalators[i].SwitchOff();
	}
	NumEscalators = 0;
}


CScriptPath CScriptPaths::aArray[3];

void CScriptPath::FindCoorsFromDistanceOnPath(float t, float *pX, float *pY, float *pZ)
{
	int32 i;
	for (i = 0; m_pNode[i + 1].t < t; i++)
		if (i == m_numNodes - 1) {
			// don't go beyond last node
			*pX = m_pNode[m_numNodes - 1].p.x;
			*pY = m_pNode[m_numNodes - 1].p.y;
			*pZ = m_pNode[m_numNodes - 1].p.z;
			return;
		}
	float f = (t - m_pNode[i].t) / (m_pNode[i + 1].t - m_pNode[i].t);
	*pX = (1.0f - f)*m_pNode[i].p.x + f*m_pNode[i + 1].p.x;
	*pY = (1.0f - f)*m_pNode[i].p.y + f*m_pNode[i + 1].p.y;
	*pZ = (1.0f - f)*m_pNode[i].p.z + f*m_pNode[i + 1].p.z;
}

void CScriptPath::Update(void) {
	if (m_state != SCRIPT_PATH_ACTIVE)
		return;

	m_fPosition += m_fSpeed * CTimer::GetTimeStepInSeconds();
	m_fPosition = clamp(m_fPosition, 0.0f, m_fTotalLength);

	if (m_pObjects[0] || m_pObjects[1] || m_pObjects[2] || m_pObjects[3]
		|| m_pObjects[4] || m_pObjects[5]) {

		float t1, t2;
		CVector pos1, pos2;

		t1 = Max(m_fPosition - m_fObjectLength / 2.0f, 0.0f);
		FindCoorsFromDistanceOnPath(t1, &pos1.x, &pos1.y, &pos1.z);
		t2 = Min(m_fPosition + m_fObjectLength / 2.0f, m_fTotalLength);
		FindCoorsFromDistanceOnPath(t2, &pos2.x, &pos2.y, &pos2.z);

		CVector newForward, newUp(0.0f, 0.0f, 1.0f), newRight;

		newForward = pos2 - pos1;
		newForward.Normalise();
		newRight = CrossProduct(newForward, newUp);
		newRight.Normalise();
		newUp = CrossProduct(newRight, newForward);

		for (int i = 0; i < 6; i++) {
			if (m_pObjects[i]) {
				CMatrix prevMat(m_pObjects[i]->GetMatrix());
				CVector prevPosition = m_pObjects[i]->GetPosition();

				m_pObjects[i]->SetPosition((pos1 + pos2) / 2.0f);
				m_pObjects[i]->GetRight() = newRight;
				m_pObjects[i]->GetUp() = newUp;
				m_pObjects[i]->GetForward() = newForward;
				m_pObjects[i]->GetMatrix().UpdateRW();
				m_pObjects[i]->UpdateRwFrame();

				if (!m_pObjects[i]->bIsBIGBuilding && prevPosition != m_pObjects[i]->GetPosition())
					m_pObjects[i]->RemoveAndAdd();

				m_pObjects[i]->GetMatrix().UpdateRW();
				m_pObjects[i]->UpdateRwFrame();

				m_pObjects[i]->m_vecMoveSpeed = (m_pObjects[i]->GetPosition() - prevMat.GetPosition()) / CTimer::GetTimeStep();

				float deltaAngle = m_pObjects[i]->GetForward().Heading() - prevMat.GetForward().Heading();
				while (deltaAngle < (float)PI) deltaAngle += (float)TWOPI;
				while (deltaAngle > (float)PI) deltaAngle -= (float)TWOPI;
				float zTurnSpeed = deltaAngle / CTimer::GetTimeStep();

				m_pObjects[i]->m_vecTurnSpeed = CVector(0.0f, 0.0f, zTurnSpeed);
				m_pObjects[i]->m_vecMoveFriction = CVector(0.0f, 0.0f, 0.0f);
				m_pObjects[i]->m_vecTurnFriction = CVector(0.0f, 0.0f, 0.0f);
			}
		}
	}
}

void CScriptPath::Clear(void) {
	if (m_pNode)
		delete[] m_pNode;
	m_pNode = nil;
	m_numNodes = 0;
	for (int i = 0; i < 6; i++)
		m_pObjects[i] = nil;
	m_state = SCRIPT_PATH_DISABLED;
}

void CScriptPath::InitialiseOne(int32 numNodes, float length) {
	char Dest[32];
	sprintf(Dest, "data\\paths\\spath%d.dat", numNodes);
	m_pNode = CPlane::LoadPath(Dest, m_numNodes, m_fTotalLength, false);
	m_fSpeed = 1.0f;
	m_fPosition = 0.0f;
	m_fObjectLength = length;
	m_state = SCRIPT_PATH_INITIALIZED;
}

void CScriptPath::SetObjectToControl(CObject *pObj) {
	int32 i = 0;
	while (i < 6 && m_pObjects[i])
		i++;
	m_pObjects[i] = pObj;
	pObj->RegisterReference((CEntity**)&m_pObjects[i]);
	pObj->m_phy_flagA08 = false;
	m_state = SCRIPT_PATH_ACTIVE;
}

void CScriptPaths::Init(void) {
	for (int i = 0; i < 3; i++)
		aArray[i].Clear();
}

void CScriptPaths::Shutdown(void) {
	for (int i = 0; i < 3; i++)
		aArray[i].Clear();
}

void CScriptPaths::Update(void) {
	for (int i = 0; i < 3; i++)
		aArray[i].Update();
}

bool CScriptPaths::IsOneActive(void) {
	for (int i = 0; i < 3; i++)
		if (aArray[i].m_state == SCRIPT_PATH_ACTIVE && aArray[i].m_fSpeed != 0.0f)
			return true;

	return false;
}

void CScriptPaths::Load(uint8 *buf, uint32 size) {
INITSAVEBUF
	for (int32 i = 0; i < 3; i++)
		aArray[i].Clear();

	for (int32 i = 0; i < 3; i++) {
		aArray[i] = ReadSaveBuf<CScriptPath>(buf);

		for (int32 j = 0; j < 6; j++) {
			CScriptPath *pPath = &aArray[i];
			if (pPath->m_pObjects[j] != nil) {
				pPath->m_pObjects[j] = CPools::GetObjectPool()->GetSlot((uintptr)pPath->m_pObjects[j] - 1);
				pPath->m_pObjects[j]->m_phy_flagA08 = false;
			}
		}

		aArray[i].m_pNode = new CPlaneNode[aArray[i].m_numNodes];
		for (int32 j = 0; j < aArray[i].m_numNodes; j++) {
			aArray[i].m_pNode[j] = ReadSaveBuf<CPlaneNode>(buf);
		}
	}
VALIDATESAVEBUF(size)
}

void CScriptPaths::Save(uint8 *buf, uint32 *size) {
	*size = sizeof(aArray);
INITSAVEBUF
	for (int32 i = 0; i < 3; i++) {
		CScriptPath *pPath = WriteSaveBuf(buf, aArray[i]);

		for (int32 j = 0; j < 6; j++) {
			if (pPath->m_pObjects[j] != nil)
				pPath->m_pObjects[j] = (CObject*)(CPools::GetObjectPool()->GetJustIndex_NoFreeAssert(pPath->m_pObjects[j]) + 1);
		}

		for (int32 j = 0; j < aArray[i].m_numNodes; j++) {
			WriteSaveBuf(buf, aArray[i].m_pNode[j]);
			*size += sizeof(aArray[i].m_pNode[j]);
		}
	}
VALIDATESAVEBUF(*size);
}

CObject *g_pScriptPathObjects[18];

void CScriptPaths::Load_ForReplay(void) {
	for (int i = 0; i < 3; i++) {
		for (int32 j = 0; j < 6; j++) {
			aArray[i].m_pObjects[j] = g_pScriptPathObjects[6 * i + j];
		}
	}
}

void CScriptPaths::Save_ForReplay(void) {
	for (int i = 0; i < 3; i++) {
		for (int32 j = 0; j < 6; j++) {
			g_pScriptPathObjects[6 * i + j] = aArray[i].m_pObjects[j];
		}
	}
}
