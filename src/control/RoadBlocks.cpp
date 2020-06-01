#include "common.h"

#include "RoadBlocks.h"
#include "PathFind.h"
#include "ModelIndices.h"
#include "Streaming.h"
#include "World.h"
#include "PedPlacement.h"
#include "Automobile.h"
#include "CopPed.h"
#include "VisibilityPlugins.h"
#include "PlayerPed.h"
#include "Wanted.h"
#include "Camera.h"
#include "CarCtrl.h"
#include "General.h"
#include "Object.h"

//--MIAMI: file done

#define ROADBLOCKDIST (90.0f)
#define ROADBLOCK_OBJECT_WIDTH (4.0f)

int16 CRoadBlocks::NumRoadBlocks;
int16 CRoadBlocks::RoadBlockNodes[NUMROADBLOCKS];
bool CRoadBlocks::InOrOut[NUMROADBLOCKS];
CScriptRoadblock CRoadBlocks::aScriptRoadBlocks[NUM_SCRIPT_ROADBLOCKS];

void
CRoadBlocks::Init(void)
{
	int i;
	NumRoadBlocks = 0;
	for(i = 0; i < ThePaths.m_numCarPathNodes; i++){
		if(ThePaths.m_pathNodes[i].bUseInRoadBlock && ThePaths.m_pathNodes[i].numLinks == 2){
			if (NumRoadBlocks < NUMROADBLOCKS) {
				InOrOut[NumRoadBlocks] = true;
				RoadBlockNodes[NumRoadBlocks] = i;
				NumRoadBlocks++;
			} else {
#ifndef MASTER
				printf("Not enough room for the potential roadblocks\n");
#endif
				// FIX: Don't iterate loop after NUMROADBLOCKS
				return;
			}
		}
	}

	ClearScriptRoadBlocks();
}

void
CRoadBlocks::GenerateRoadBlockCopsForCar(CVehicle* pVehicle, int32 roadBlockType)
{
	static const CVector vecRoadBlockOffets[6] = { {-1.5, 1.8f, 0.0f}, {-1.5f, -1.8f, 0.0f}, {1.5f, 1.8f, 0.0f},
	{1.5f, -1.8f, 0.0f}, {-1.5f, 0.0f, 0.0f}, {1.5, 0.0, 0.0} };
	CEntity* pEntityToAttack = (CEntity*)FindPlayerVehicle();
	if (!pEntityToAttack)
		pEntityToAttack = (CEntity*)FindPlayerPed();
	CColModel* pPoliceColModel = CModelInfo::GetModelInfo(MI_POLICE)->GetColModel();
	float fRadius = pVehicle->GetBoundRadius() / pPoliceColModel->boundingSphere.radius;
	for (int32 i = 0; i < 2; i++) {
		const int32 roadBlockIndex = i + 2 * roadBlockType;
		CVector posForZ = pVehicle->m_matrix * (fRadius * vecRoadBlockOffets[roadBlockIndex]);
		int32 modelInfoId = MI_COP;
		eCopType copType = COP_STREET;
		switch (pVehicle->GetModelIndex())
		{
		case MI_FBICAR:
			modelInfoId = MI_FBI;
			copType = COP_FBI;
			break;
		case MI_ENFORCER:
			modelInfoId = MI_SWAT;
			copType = COP_SWAT;
			break;
		case MI_BARRACKS:
			modelInfoId = MI_ARMY;
			copType = COP_ARMY;
			break;
		}
		if (!CStreaming::HasModelLoaded(modelInfoId))
			copType = COP_STREET;
		CCopPed* pCopPed = new CCopPed(copType);
		if (copType == COP_STREET)
			pCopPed->SetCurrentWeapon(WEAPONTYPE_COLT45);
		CPedPlacement::FindZCoorForPed(&posForZ);
		pCopPed->SetPosition(posForZ);
		pCopPed->SetOrientation(0.0f, 0.0f, -HALFPI);
		pCopPed->m_bIsDisabledCop = true;
		pCopPed->SetIdle();
		pCopPed->bKindaStayInSamePlace = true;
		pCopPed->bNotAllowedToDuck = false;
		pCopPed->bCrouchWhenShooting = roadBlockType != 2;
		if (pEntityToAttack) {
			pCopPed->m_pPointGunAt = pEntityToAttack;
			pEntityToAttack->RegisterReference(&pCopPed->m_pPointGunAt);
			pCopPed->SetAttack(pEntityToAttack);
		}
		pCopPed->m_pMyVehicle = pVehicle;
		pVehicle->RegisterReference((CEntity**)&pCopPed->m_pMyVehicle);
		pCopPed->bCullExtraFarAway = true;
		CVisibilityPlugins::SetClumpAlpha(pCopPed->GetClump(), 0);
		CWorld::Add(pCopPed);
	}
}

void 
CRoadBlocks::GenerateRoadBlocks(void) 
{ 
	CMatrix tmp1, tmp2;
	static int16 unk;
	uint32 frame = CTimer::GetFrameCounter() & 0xF;
	int16 nRoadblockNode = (int16)(NUMROADBLOCKS * frame) / 16;
	const int16 maxRoadBlocks = (int16)(NUMROADBLOCKS * (frame + 1)) / 16;
	for (; nRoadblockNode < Min(NumRoadBlocks, maxRoadBlocks); nRoadblockNode++) {
		int16 node = RoadBlockNodes[nRoadblockNode];
		CVector2D vecDistance = FindPlayerCoors() - ThePaths.m_pathNodes[node].GetPosition();
		if (vecDistance.x > -ROADBLOCKDIST && vecDistance.x < ROADBLOCKDIST &&
			vecDistance.y > -ROADBLOCKDIST && vecDistance.y < ROADBLOCKDIST &&
			vecDistance.Magnitude() < ROADBLOCKDIST) {
			if (!InOrOut[nRoadblockNode]) {
				InOrOut[nRoadblockNode] = true;
				if (FindPlayerVehicle() && (CGeneral::GetRandomNumber() & 0x7F) < FindPlayerPed()->m_pWanted->m_RoadblockDensity) {
					CCarPathLink* pLink1 = &ThePaths.m_carPathLinks[ThePaths.m_carPathConnections[ThePaths.m_pathNodes[node].firstLink]];
					CCarPathLink* pLink2 = &ThePaths.m_carPathLinks[ThePaths.m_carPathConnections[ThePaths.m_pathNodes[node].firstLink + 1]];
					int lanes = Min(pLink1->numRightLanes + pLink1->numLeftLanes, pLink2->numLeftLanes + pLink2->numRightLanes);
					float length = LANE_WIDTH * (lanes + 1);
					CVector forward(pLink2->GetY() - pLink1->GetY(), -(pLink2->GetX() - pLink1->GetX()), 0.0f);
					forward.Normalise();
					if (ThePaths.m_pathNodes[node].HasDivider()) {
						CreateRoadBlockBetween2Points(
							ThePaths.m_pathNodes[node].GetPosition() + (length * 0.5f + ThePaths.m_pathNodes[node].GetDividerWidth()) * forward,
							ThePaths.m_pathNodes[node].GetPosition() + ThePaths.m_pathNodes[node].GetDividerWidth() * forward);
						CreateRoadBlockBetween2Points(
							ThePaths.m_pathNodes[node].GetPosition() - ThePaths.m_pathNodes[node].GetDividerWidth() * forward,
							ThePaths.m_pathNodes[node].GetPosition() - (length * 0.5f + ThePaths.m_pathNodes[node].GetDividerWidth()) * forward);
					}
					else {
						CreateRoadBlockBetween2Points(
							ThePaths.m_pathNodes[node].GetPosition() + (length * 0.5f) * forward,
							ThePaths.m_pathNodes[node].GetPosition() - (length * 0.5f) * forward);
					}
				}
			}
		}
		else {
			InOrOut[nRoadblockNode] = false;
		}
	}
	int i = CTimer::GetFrameCounter() & 0xF;
	if (!aScriptRoadBlocks[i].m_bInUse)
		return;
	if ((aScriptRoadBlocks[i].GetPosition() - FindPlayerCoors()).Magnitude() < 100.0f) {
		CreateRoadBlockBetween2Points(aScriptRoadBlocks[i].m_vInf, aScriptRoadBlocks[i].m_vSup);
		aScriptRoadBlocks[i].m_bInUse = false;
	}
}

void
CRoadBlocks::ClearScriptRoadBlocks(void)
{
	for (int i = 0; i < NUM_SCRIPT_ROADBLOCKS; i++)
		aScriptRoadBlocks[i].m_bInUse = false;
}

void
CRoadBlocks::RegisterScriptRoadBlock(CVector vInf, CVector vSup)
{
	int32 i;
	for (i = 0; i < NUM_SCRIPT_ROADBLOCKS; i++) {
		if (!aScriptRoadBlocks[i].m_bInUse)
			break;
	}
	if (i == NUM_SCRIPT_ROADBLOCKS)
		return;
	aScriptRoadBlocks[i].m_bInUse = true;
	aScriptRoadBlocks[i].m_vInf = vInf;
	aScriptRoadBlocks[i].m_vSup = vSup;
}

void 
CRoadBlocks::CreateRoadBlockBetween2Points(CVector point1, CVector point2)
{
	CMatrix tmp;
	CVector forward = (point2 - point1);
	float distBetween = forward.Magnitude();
	CVector pos = (point1 + point2) / 2;
	CVector right(forward.y, -forward.x, 0.0f);
	forward.Normalise();
	right.Normalise();
	if (DotProduct(FindPlayerCoors() - pos, right) < 0.0f) {
		right *= -1.0f;
	}
	int32 vehicleId = MI_POLICE;
	if (FindPlayerPed()->m_pWanted->AreArmyRequired())
		vehicleId = MI_BARRACKS;
	else if (FindPlayerPed()->m_pWanted->AreFbiRequired())
		vehicleId = MI_FBICAR;
	else if (FindPlayerPed()->m_pWanted->AreSwatRequired())
		vehicleId = MI_ENFORCER;
	if (!CStreaming::HasModelLoaded(vehicleId))
		vehicleId = MI_POLICE;
	CColModel* pVehicleColModel = CModelInfo::GetModelInfo(vehicleId)->GetColModel();
	float fModelRadius = 2.0f * pVehicleColModel->boundingSphere.radius + 0.25f;
	int16 numRoadblockVehicles = Min(6, (int16)(distBetween / fModelRadius));
	for (int16 i = 0; i < numRoadblockVehicles; i++) {
		float offset = fModelRadius * (i - numRoadblockVehicles / 2);
		tmp.SetTranslate(0.0f, 0.0f, 0.0f);
		tmp.GetRight() = CVector(forward.y, -forward.x, 0.0f);
		tmp.GetForward() = forward;
		tmp.GetUp() = CVector(0.0f, 0.0f, 1.0f);
		tmp.RotateZ(((CGeneral::GetRandomNumber() & 0xFF) - 128.0f) * 0.003f);
		if (CGeneral::GetRandomNumber() & 1)
			tmp.RotateZ(((CGeneral::GetRandomNumber() & 0xFF) - 128.0f) * 0.003f + 3.1416f);
		tmp.SetTranslateOnly(offset * forward + pos);
		tmp.GetPosition().z += 0.6f;
		float fModelRadius = CModelInfo::GetModelInfo(vehicleId)->GetColModel()->boundingSphere.radius - 0.25f;
		int16 colliding = 0;
		CWorld::FindObjectsKindaColliding(tmp.GetPosition(), fModelRadius, 0, &colliding, 2, nil, false, true, true, false, false);
		if (!colliding) {
			CAutomobile* pVehicle = new CAutomobile(vehicleId, RANDOM_VEHICLE);
			pVehicle->SetStatus(STATUS_ABANDONED);
			// pVehicle->GetHeightAboveRoad(); // called but return value is ignored?
			tmp.GetPosition().z += fModelRadius - 0.6f;
			pVehicle->m_matrix = tmp;
			pVehicle->PlaceOnRoadProperly();
			pVehicle->bIsStatic = false;
			pVehicle->m_matrix.UpdateRW();
			pVehicle->m_nDoorLock = CARLOCK_UNLOCKED;
			CCarCtrl::JoinCarWithRoadSystem(pVehicle);
			pVehicle->bIsLocked = false;
			pVehicle->AutoPilot.m_nCarMission = MISSION_NONE;
			pVehicle->AutoPilot.m_nTempAction = TEMPACT_NONE;
			pVehicle->AutoPilot.m_nNextLane = pVehicle->AutoPilot.m_nCurrentLane = 0;
			pVehicle->AutoPilot.m_nCruiseSpeed = pVehicle->AutoPilot.m_fMaxTrafficSpeed = 0;
			pVehicle->bExtendedRange = true;
			if (pVehicle->UsesSiren(pVehicle->GetModelIndex()) && CGeneral::GetRandomNumber() & 1)
				pVehicle->m_bSirenOrAlarm = true;
			if (pVehicle->GetUp().z > 0.94f) {
				CVisibilityPlugins::SetClumpAlpha(pVehicle->GetClump(), 0);
				CWorld::Add(pVehicle);
				pVehicle->bCreateRoadBlockPeds = true;
				pVehicle->m_nRoadblockType = DotProduct(pVehicle->GetRight(), pVehicle->GetPosition() - FindPlayerCoors()) >= 0.0f;
				pVehicle->m_nSetPieceExtendedRangeTime = CTimer::GetTimeInMilliseconds() + 7000;
			}
			else {
				delete pVehicle;
			}
		}
	}
	int numBarriers = distBetween / ROADBLOCK_OBJECT_WIDTH;
	CStreaming::RequestModel(MI_ROADWORKBARRIER1, STREAMFLAGS_DONT_REMOVE);
	if (!CStreaming::HasModelLoaded(MI_ROADWORKBARRIER1))
		return;
	for (int i = 0; i < numBarriers; i++) {
		float offset = ROADBLOCK_OBJECT_WIDTH * (i - numBarriers / 2);
		tmp.SetTranslate(0.0f, 0.0f, 0.0f);
		tmp.GetRight() = CVector(forward.y, -forward.x, 0.0f);
		tmp.GetForward() = forward;
		tmp.GetUp() = CVector(0.0f, 0.0f, 1.0f);
		tmp.RotateZ(((CGeneral::GetRandomNumber() & 0xFF) - 128.0f) * 0.003f);
		tmp.SetTranslateOnly(5.0f * right + offset * forward + pos);
		tmp.GetPosition().x += (CGeneral::GetRandomNumber() & 0xF) * 0.1f;
		tmp.GetPosition().y += (CGeneral::GetRandomNumber() & 0xF) * 0.1f;
		bool found;
		tmp.GetPosition().z = CWorld::FindGroundZFor3DCoord(tmp.GetPosition().x, tmp.GetPosition().y, tmp.GetPosition().z + 2.0f, &found);
		if (!found)
			continue;
		int16 colliding = 0;
		CBaseModelInfo* pMI = CModelInfo::GetModelInfo(MI_ROADWORKBARRIER1);
		tmp.GetPosition().z -= pMI->GetColModel()->boundingBox.min.z;
		CWorld::FindObjectsKindaColliding(tmp.GetPosition(), pMI->GetColModel()->boundingSphere.radius, 0, &colliding, 2, nil, false, true, true, false, false);
		if (colliding == 0) {
			CObject* pObject = new CObject(MI_ROADWORKBARRIER1, true);
			pObject->GetMatrix() = tmp;
			pObject->ObjectCreatedBy = TEMP_OBJECT;
			pObject->m_nEndOfLifeTime = CTimer::GetTimeInMilliseconds() + 600000;
			CWorld::Add(pObject);
		}
	}
}
