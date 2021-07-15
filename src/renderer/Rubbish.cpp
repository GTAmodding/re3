#include "common.h"
#include "main.h"

#include "General.h"
#include "Timer.h"
#include "Weather.h"
#include "Camera.h"
#include "World.h"
#include "Vehicle.h"
#include "ZoneCull.h"
#include "TxdStore.h"
#include "RenderBuffer.h"
#include "Rubbish.h"

#define RUBBISH_MAX_DIST (18.0f)
#define RUBBISH_FADE_DIST (16.5f)

RwTexture *gpRubbishTexture[4];
RwImVertexIndex RubbishIndexList[6];
RwImVertexIndex RubbishIndexList2[6];	// unused
RwIm3DVertex RubbishVertices[4];
bool CRubbish::bRubbishInvisible;
int CRubbish::RubbishVisibility;
COneSheet CRubbish::aSheets[NUM_RUBBISH_SHEETS];
COneSheet CRubbish::StartEmptyList;
COneSheet CRubbish::EndEmptyList;
COneSheet CRubbish::StartStaticsList;
COneSheet CRubbish::EndStaticsList;
COneSheet CRubbish::StartMoversList;
COneSheet CRubbish::EndMoversList;


void
COneSheet::AddToList(COneSheet *list)
{
	this->m_next = list->m_next;
	this->m_prev = list;
	list->m_next = this;
	this->m_next->m_prev = this;
}

void
COneSheet::RemoveFromList(void)
{
	m_next->m_prev = m_prev;
	m_prev->m_next = m_next;
}


void
CRubbish::Render(void)
{
	int type;

	PUSH_RENDERGROUP("CRubbish::Render");

	RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)FALSE);
	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)TRUE);
	RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void*)TRUE);

	for(type = 0; type < 4; type++){
		RwRenderStateSet(rwRENDERSTATETEXTURERASTER, RwTextureGetRaster(gpRubbishTexture[type]));

		TempBufferIndicesStored = 0;
		TempBufferVerticesStored = 0;

		COneSheet *sheet;
		for(sheet = &aSheets[type*NUM_RUBBISH_SHEETS / 4];
		    sheet < &aSheets[(type+1)*NUM_RUBBISH_SHEETS / 4];
		    sheet++){
			if(sheet->m_state == 0)
				continue;

			uint32 alpha = 128;
			CVector pos;
			if(sheet->m_state == 1){
				pos = sheet->m_basePos;
				if(!sheet->m_isVisible)
					alpha = 0;
			}else{
				pos = sheet->m_animatedPos;
				// Not fully visible during animation, calculate current alpha
				if(!sheet->m_isVisible || !sheet->m_targetIsVisible){
					float t = (float)(CTimer::GetTimeInMilliseconds() - sheet->m_moveStart)/sheet->m_moveDuration;
					float f1 = sheet->m_isVisible ? 1.0f-t : 0.0f;
					float f2 = sheet->m_targetIsVisible ? t : 0.0f;
					alpha = 128 * (f1+f2);
				}
			}

			float camDist = (pos - TheCamera.GetPosition()).Magnitude2D();
			if(camDist < RUBBISH_MAX_DIST){
				if(camDist >= RUBBISH_FADE_DIST)
					alpha -= alpha*(camDist-RUBBISH_FADE_DIST)/(RUBBISH_MAX_DIST-RUBBISH_FADE_DIST);
				alpha = (RubbishVisibility*alpha)/256;

				float vx = Sin(sheet->m_angle) * 0.4f;
				float vy = Cos(sheet->m_angle) * 0.4f;

				int v = TempBufferVerticesStored;
				RwIm3DVertexSetPos(&TempBufferRenderVertices[v+0], pos.x + vx, pos.y + vy, pos.z);
				RwIm3DVertexSetRGBA(&TempBufferRenderVertices[v+0], 255, 255, 255, alpha);
				RwIm3DVertexSetPos(&TempBufferRenderVertices[v+1], pos.x - vy, pos.y + vx, pos.z);
				RwIm3DVertexSetRGBA(&TempBufferRenderVertices[v+1], 255, 255, 255, alpha);
				RwIm3DVertexSetPos(&TempBufferRenderVertices[v+2], pos.x + vy, pos.y - vx, pos.z);
				RwIm3DVertexSetRGBA(&TempBufferRenderVertices[v+2], 255, 255, 255, alpha);
				RwIm3DVertexSetPos(&TempBufferRenderVertices[v+3], pos.x - vx, pos.y - vy, pos.z);
				RwIm3DVertexSetRGBA(&TempBufferRenderVertices[v+3], 255, 255, 255, alpha);
				RwIm3DVertexSetU(&TempBufferRenderVertices[v+0], 0.0f);
				RwIm3DVertexSetV(&TempBufferRenderVertices[v+0], 0.0f);
				RwIm3DVertexSetU(&TempBufferRenderVertices[v+1], 1.0f);
				RwIm3DVertexSetV(&TempBufferRenderVertices[v+1], 0.0f);
				RwIm3DVertexSetU(&TempBufferRenderVertices[v+2], 0.0f);
				RwIm3DVertexSetV(&TempBufferRenderVertices[v+2], 1.0f);
				RwIm3DVertexSetU(&TempBufferRenderVertices[v+3], 1.0f);
				RwIm3DVertexSetV(&TempBufferRenderVertices[v+3], 1.0f);

				int i = TempBufferIndicesStored;
				TempBufferRenderIndexList[i+0] = RubbishIndexList[0] + TempBufferVerticesStored;
				TempBufferRenderIndexList[i+1] = RubbishIndexList[1] + TempBufferVerticesStored;
				TempBufferRenderIndexList[i+2] = RubbishIndexList[2] + TempBufferVerticesStored;
				TempBufferRenderIndexList[i+3] = RubbishIndexList[3] + TempBufferVerticesStored;
				TempBufferRenderIndexList[i+4] = RubbishIndexList[4] + TempBufferVerticesStored;
				TempBufferRenderIndexList[i+5] = RubbishIndexList[5] + TempBufferVerticesStored;
				TempBufferVerticesStored += 4;
				TempBufferIndicesStored += 6;
			}
		}

		if(TempBufferIndicesStored != 0){
			LittleTest();
			if(RwIm3DTransform(TempBufferRenderVertices, TempBufferVerticesStored, nil, rwIM3D_VERTEXUV)){
				RwIm3DRenderIndexedPrimitive(rwPRIMTYPETRILIST, TempBufferRenderIndexList, TempBufferIndicesStored);
				RwIm3DEnd();
			}
		}
	}

	RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void*)FALSE);
	RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)FALSE);
	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)TRUE);

	POP_RENDERGROUP();
}

void
CRubbish::StirUp(CVehicle *veh)
{
	if((CTimer::GetFrameCounter() ^ (veh->m_randomSeed&3)) == 0)
		return;

	if(Abs(veh->GetPosition().x - TheCamera.GetPosition().x) < 20.0f &&
	   Abs(veh->GetPosition().y - TheCamera.GetPosition().y) < 20.0f)
		if(Abs(veh->GetMoveSpeed().x) > 0.05f || Abs(veh->GetMoveSpeed().y) > 0.05f){
			float speed = veh->GetMoveSpeed().Magnitude2D();
			if(speed > 0.05f){
				bool movingForward = DotProduct2D(veh->GetMoveSpeed(), veh->GetForward()) > 0.0f;
				COneSheet *sheet = StartStaticsList.m_next;
				CVector2D size = veh->GetColModel()->boundingBox.max;

				// Check all static sheets
				while(sheet != &EndStaticsList){
					COneSheet *next = sheet->m_next;
					CVector2D carToSheet = sheet->m_basePos - veh->GetPosition();
					float distFwd = DotProduct2D(carToSheet, veh->GetForward());

					// sheet has to be a bit behind car
					if(movingForward && distFwd < -0.5f*size.y && distFwd > -1.5f*size.y ||
					   !movingForward && distFwd > 0.5f*size.y && distFwd < 1.5f*size.y){
						float distSide = Abs(DotProduct2D(carToSheet, veh->GetRight()));
						if(distSide < 1.5*size.x){
							// Check with higher speed for sheet directly behind car
							float speedToCheck = distSide < size.x ? speed : speed*0.5f;
							if(speedToCheck > 0.05f){
								sheet->m_state = 2;
								if(speedToCheck > 0.15f)
									sheet->m_animationType = 2;
								else
									sheet->m_animationType = 1;
								sheet->m_moveDuration = 2000;
								sheet->m_xDist = veh->GetMoveSpeed().x;
								sheet->m_yDist = veh->GetMoveSpeed().y;
								float dist = Sqrt(SQR(sheet->m_xDist)+SQR(sheet->m_yDist));
								sheet->m_xDist *= 25.0f*speed/dist;
								sheet->m_yDist *= 25.0f*speed/dist;
								sheet->m_animHeight = 3.0f*speed;
								sheet->m_moveStart = CTimer::GetTimeInMilliseconds();
								float tx = sheet->m_basePos.x + sheet->m_xDist;
								float ty = sheet->m_basePos.y + sheet->m_yDist;
								float tz = sheet->m_basePos.z + 3.0f;
								sheet->m_targetZ = CWorld::FindGroundZFor3DCoord(tx, ty, tz, nil) + 0.1f;
								sheet->RemoveFromList();
								sheet->AddToList(&StartMoversList);
							}
						}
					}

					sheet = next;
				}
			}
		}
}

static float aAnimations[3][34] = {
	{ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
	  0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },

	// Normal move
	{ 0.0f, 0.05f, 0.12f, 0.25f, 0.42f, 0.57f, 0.68f, 0.8f, 0.86f, 0.9f, 0.93f, 0.95f, 0.96f, 0.97f, 0.98f, 0.99f, 1.0f,	// XY movemnt
	  0.15f, 0.35f, 0.6f, 0.9f, 1.2f, 1.25f, 1.3f, 1.2f, 1.1f, 0.95f, 0.8f, 0.6f, 0.45f, 0.3f, 0.2f, 0.1f, 0 },	// Z movement

	// Stirred up by fast vehicle
	{ 0.0f, 0.05f, 0.12f, 0.25f, 0.42f, 0.57f, 0.68f, 0.8f, 0.95f, 1.1f, 1.15f, 1.18f, 1.15f, 1.1f, 1.05f, 1.03f, 1.0f,
	  0.15f, 0.35f, 0.6f, 0.9f, 1.2f, 1.25f, 1.3f, 1.2f, 1.1f, 0.95f, 0.8f, 0.6f, 0.45f, 0.3f, 0.2f, 0.1f, 0 }
};

void
CRubbish::Update(void)
{
	bool foundGround;

	// FRAMETIME
	if(bRubbishInvisible)
		RubbishVisibility = Max(RubbishVisibility-5, 0);
	else
		RubbishVisibility = Min(RubbishVisibility+5, 255);

	// Spawn a new sheet
	COneSheet *sheet = StartEmptyList.m_next;
	if(sheet != &EndEmptyList){
		float spawnDist;
		float spawnAngle;

		spawnDist = (CGeneral::GetRandomNumber()&0xFF)/256.0f + RUBBISH_MAX_DIST;
		uint8 r = CGeneral::GetRandomNumber();
		if(r&1)
			spawnAngle = (CGeneral::GetRandomNumber()&0xFF)/256.0f * 6.28f;
		else
			spawnAngle = (r-128)/160.0f + TheCamera.Orientation;
		sheet->m_basePos.x = TheCamera.GetPosition().x + spawnDist*Sin(spawnAngle);
		sheet->m_basePos.y = TheCamera.GetPosition().y + spawnDist*Cos(spawnAngle);
		sheet->m_basePos.z = CWorld::FindGroundZFor3DCoord(sheet->m_basePos.x, sheet->m_basePos.y, TheCamera.GetPosition().z, &foundGround) + 0.1f;
		if(foundGround){
			// Found ground, so add to statics list
			sheet->m_angle = (CGeneral::GetRandomNumber()&0xFF)/256.0f * 6.28f;
			sheet->m_state = 1;
			if(CCullZones::FindAttributesForCoors(sheet->m_basePos, nil) & ATTRZONE_NORAIN)
				sheet->m_isVisible = false;
			else
				sheet->m_isVisible = true;
			sheet->RemoveFromList();
			sheet->AddToList(&StartStaticsList);
		}
	}

	// Process animation
	sheet = StartMoversList.m_next;
	while(sheet != &EndMoversList){
		uint32 currentTime = CTimer::GetTimeInMilliseconds() - sheet->m_moveStart;
		if(currentTime < sheet->m_moveDuration){
			// Animation
			int step = 16 * currentTime / sheet->m_moveDuration;	// 16 steps in animation
			int stepTime = sheet->m_moveDuration/16;	// time in each step
			float s = (float)(currentTime - stepTime*step) / stepTime;	// position on step
			float t = (float)currentTime / sheet->m_moveDuration;	// position on total animation
			// factors for xy and z-movment
			float fxy = aAnimations[sheet->m_animationType][step]*(1.0f-s) + aAnimations[sheet->m_animationType][step+1]*s;
			float fz = aAnimations[sheet->m_animationType][step+17]*(1.0f-s) + aAnimations[sheet->m_animationType][step+1+17]*s;
			sheet->m_animatedPos.x = sheet->m_basePos.x + fxy*sheet->m_xDist;
			sheet->m_animatedPos.y = sheet->m_basePos.y + fxy*sheet->m_yDist;
			sheet->m_animatedPos.z = (1.0f-t)*sheet->m_basePos.z + t*sheet->m_targetZ + fz*sheet->m_animHeight;
			sheet->m_angle += CTimer::GetTimeStep()*0.04f;
			if(sheet->m_angle > 6.28f)
				sheet->m_angle -= 6.28f;
			sheet = sheet->m_next;
		}else{
			// End of animation, back into statics list
			sheet->m_basePos.x += sheet->m_xDist;
			sheet->m_basePos.y += sheet->m_yDist;
			sheet->m_basePos.z = sheet->m_targetZ;
			sheet->m_state = 1;
			sheet->m_isVisible = sheet->m_targetIsVisible;

			COneSheet *next = sheet->m_next;
			sheet->RemoveFromList();
			sheet->AddToList(&StartStaticsList);
			sheet = next;
		}
	}

	// Stir up a sheet by wind
	// FRAMETIME
	int freq;
	if(CWeather::Wind < 0.1f)
		freq = 31;
	else if(CWeather::Wind < 0.4f)
		freq = 7;
	else if(CWeather::Wind < 0.7f)
		freq = 1;
	else
		freq = 0;
	if((CTimer::GetFrameCounter() & freq) == 0){
		// Pick a random sheet and set animation state if static
		int i = CGeneral::GetRandomNumber() % NUM_RUBBISH_SHEETS;
		if(aSheets[i].m_state == 1){
			aSheets[i].m_moveStart = CTimer::GetTimeInMilliseconds();
			aSheets[i].m_moveDuration = CWeather::Wind*1500.0f + 1000.0f;
			aSheets[i].m_animHeight = 0.2f;
			aSheets[i].m_xDist = 3.0f*CWeather::Wind;
			aSheets[i].m_yDist = 3.0f*CWeather::Wind;
			// Check if target position is ok
			float tx = aSheets[i].m_basePos.x + aSheets[i].m_xDist;
			float ty = aSheets[i].m_basePos.y + aSheets[i].m_yDist;
			float tz = aSheets[i].m_basePos.z + 3.0f;
			aSheets[i].m_targetZ = CWorld::FindGroundZFor3DCoord(tx, ty, tz, &foundGround) + 0.1f;
			if(CCullZones::FindAttributesForCoors(CVector(tx, ty, aSheets[i].m_targetZ), nil) & ATTRZONE_NORAIN)
				aSheets[i].m_targetIsVisible = false;
			else
				aSheets[i].m_targetIsVisible = true;
			if(foundGround){
				// start animation
				aSheets[i].m_state = 2;
				aSheets[i].m_animationType = 1;
				aSheets[i].RemoveFromList();
				aSheets[i].AddToList(&StartMoversList);
			}
		}
	}

	// Remove sheets that are too far away
	int i = (CTimer::GetFrameCounter()%(NUM_RUBBISH_SHEETS/4))*4;
	int last = ((CTimer::GetFrameCounter()%(NUM_RUBBISH_SHEETS/4)) + 1)*4;
	for(; i < last; i++){
		if(aSheets[i].m_state == 1 &&
		   (aSheets[i].m_basePos - TheCamera.GetPosition()).MagnitudeSqr2D() > SQR(RUBBISH_MAX_DIST+1.0f)){
			aSheets[i].m_state = 0;
			aSheets[i].RemoveFromList();
			aSheets[i].AddToList(&StartEmptyList);
		}
	}
}

void
CRubbish::SetVisibility(bool visible)
{
	bRubbishInvisible = !visible;
}

void
CRubbish::Init(void)
{
	int i;
	for(i = 0; i < NUM_RUBBISH_SHEETS; i++){
		aSheets[i].m_state = 0;
		if(i < NUM_RUBBISH_SHEETS-1)
			aSheets[i].m_next = &aSheets[i+1];
		else
			aSheets[i].m_next = &EndEmptyList;
		if(i > 0)
			aSheets[i].m_prev = &aSheets[i-1];
		else
			aSheets[i].m_prev = &StartEmptyList;
	}

	StartEmptyList.m_next = &aSheets[0];
	StartEmptyList.m_prev = nil;
	EndEmptyList.m_next = nil;
	EndEmptyList.m_prev = &aSheets[NUM_RUBBISH_SHEETS-1];

	StartStaticsList.m_next = &EndStaticsList;
	StartStaticsList.m_prev = nil;
	EndStaticsList.m_next = nil;
	EndStaticsList.m_prev = &StartStaticsList;

	StartMoversList.m_next = &EndMoversList;
	StartMoversList.m_prev = nil;
	EndMoversList.m_next = nil;
	EndMoversList.m_prev = &StartMoversList;

	// unused
	RwIm3DVertexSetU(&RubbishVertices[0], 0.0f);
	RwIm3DVertexSetV(&RubbishVertices[0], 0.0f);
	RwIm3DVertexSetU(&RubbishVertices[1], 1.0f);
	RwIm3DVertexSetV(&RubbishVertices[1], 0.0f);
	RwIm3DVertexSetU(&RubbishVertices[2], 0.0f);
	RwIm3DVertexSetV(&RubbishVertices[2], 1.0f);
	RwIm3DVertexSetU(&RubbishVertices[3], 1.0f);
	RwIm3DVertexSetV(&RubbishVertices[3], 1.0f);

	// unused
	RubbishIndexList2[0] = 0;
	RubbishIndexList2[1] = 2;
	RubbishIndexList2[2] = 1;
	RubbishIndexList2[3] = 1;
	RubbishIndexList2[4] = 2;
	RubbishIndexList2[5] = 3;

	RubbishIndexList[0] = 0;
	RubbishIndexList[1] = 1;
	RubbishIndexList[2] = 2;
	RubbishIndexList[3] = 1;
	RubbishIndexList[4] = 3;
	RubbishIndexList[5] = 2;

	CTxdStore::PushCurrentTxd();
	int slot = CTxdStore::FindTxdSlot("particle");
	CTxdStore::SetCurrentTxd(slot);
	gpRubbishTexture[0] = RwTextureRead("gameleaf01_64", nil);
	gpRubbishTexture[1] = RwTextureRead("gameleaf02_64", nil);
	gpRubbishTexture[2] = RwTextureRead("newspaper01_64", nil);
	gpRubbishTexture[3] = RwTextureRead("newspaper02_64", nil);
	CTxdStore::PopCurrentTxd();
	RubbishVisibility = 255;
	bRubbishInvisible = false;
}

void
CRubbish::Shutdown(void)
{
	RwTextureDestroy(gpRubbishTexture[0]);
#if GTA_VERSION >= GTA3_PC_11
	gpRubbishTexture[0] = nil;
#endif
	RwTextureDestroy(gpRubbishTexture[1]);
#if GTA_VERSION >= GTA3_PC_11
	gpRubbishTexture[1] = nil;
#endif
	RwTextureDestroy(gpRubbishTexture[2]);
#if GTA_VERSION >= GTA3_PC_11
	gpRubbishTexture[2] = nil;
#endif
	RwTextureDestroy(gpRubbishTexture[3]);
#if GTA_VERSION >= GTA3_PC_11
	gpRubbishTexture[3] = nil;
#endif
}
