#include "common.h"
#include "patcher.h"
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

int16 &CRoadBlocks::NumRoadBlocks = *(int16*)0x95CC34;
int16 (&CRoadBlocks::RoadBlockObjects)[NUMROADBLOCKS] = *(int16(*)[NUMROADBLOCKS]) * (uintptr*)0x72B3A8;
bool (&CRoadBlocks::InOrOut)[NUMROADBLOCKS] = *(bool(*)[NUMROADBLOCKS]) * (uintptr*)0x733810;

void
CRoadBlocks::Init(void)
{
	NumRoadBlocks = 0;
	for (int objId = 0; objId < ThePaths.m_numMapObjects; objId++) {
		if (ThePaths.m_objectFlags[objId] & UseInRoadBlock) {
			if (NumRoadBlocks < NUMROADBLOCKS) {
				InOrOut[NumRoadBlocks] = true;
				RoadBlockObjects[NumRoadBlocks] = objId;
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

}

void
CRoadBlocks::GenerateRoadBlockCopsForCar(CVehicle* pVehicle, int32 roadBlockType, int16 roadBlockNode)
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
		pCopPed->m_matrix.GetPosition() = posForZ;
		CVector vecSavedPos = pCopPed->m_matrix.GetPosition();
		pCopPed->m_matrix.SetRotate(0.0f, 0.0f, -HALFPI);
		pCopPed->m_matrix.GetPosition() += vecSavedPos;
		pCopPed->m_bIsDisabledCop = true;
		pCopPed->SetIdle();
		pCopPed->bKindaStayInSamePlace = true;
		pCopPed->bNotAllowedToDuck = false;
		pCopPed->m_wRoadblockNode = roadBlockNode;
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
	CMatrix offsetMatrix;
	uint32 frame = CTimer::GetFrameCounter() & 0xF;
	int16 nRoadblockNode = (int16)(NUMROADBLOCKS * frame) / 16;
	const int16 maxRoadBlocks = (int16)(NUMROADBLOCKS * (frame + 1)) / 16;
	int16 numRoadBlocks = CRoadBlocks::NumRoadBlocks;
	if (CRoadBlocks::NumRoadBlocks >= maxRoadBlocks)
		numRoadBlocks = maxRoadBlocks;
	for (; nRoadblockNode < numRoadBlocks; nRoadblockNode++) {
		CTreadable *mapObject = ThePaths.m_mapObjects[CRoadBlocks::RoadBlockObjects[nRoadblockNode]];
		CVector2D vecDistance = FindPlayerCoors() - mapObject->GetPosition();
		if (vecDistance.x > -80.0f && vecDistance.x < 80.0f &&
			vecDistance.y > -80.0f && vecDistance.y < 80.0f &&
			vecDistance.Magnitude() < 80.0f) {
			if (!CRoadBlocks::InOrOut[nRoadblockNode]) {
				CRoadBlocks::InOrOut[nRoadblockNode] = true;
				if (FindPlayerVehicle() && (CGeneral::GetRandomNumber() & 0x7F) < FindPlayerPed()->m_pWanted->m_RoadblockDensity) {
					CWanted *pPlayerWanted = FindPlayerPed()->m_pWanted;
					float fMapObjectRadius = 2.0f * mapObject->GetColModel()->boundingBox.max.x;
					int32 vehicleId = MI_POLICE;
					if (pPlayerWanted->AreArmyRequired())
						vehicleId = MI_BARRACKS;
					else if (pPlayerWanted->AreFbiRequired())
						vehicleId = MI_FBICAR;
					else if (pPlayerWanted->AreSwatRequired())
						vehicleId = MI_ENFORCER;
					if (!CStreaming::HasModelLoaded(vehicleId))
						vehicleId = MI_POLICE;
					CColModel *pVehicleColModel = CModelInfo::GetModelInfo(vehicleId)->GetColModel();
					float fModelRadius = 2.0f * pVehicleColModel->boundingSphere.radius + 0.25f;
					int16 radius = (int16)(fMapObjectRadius / fModelRadius);
					if (radius > 0 && radius < 6) {
						CVector2D vecDistanceToCamera = TheCamera.GetPosition() - mapObject->m_matrix.GetPosition();
						float fDotProduct = DotProduct2D(vecDistanceToCamera, mapObject->m_matrix.GetUp());
						float fOffset = 0.5f * fModelRadius * (float)(radius - 1);
						for (int16 i = 0; i < radius; i++) {
							uint8 nRoadblockType = fDotProduct < 0.0f;
							if (CGeneral::GetRandomNumber() & 1) {
								offsetMatrix.SetRotateZ(((CGeneral::GetRandomNumber() & 0xFF) - 128.0f) * 0.003f + HALFPI);
							}
							else {
								nRoadblockType = !nRoadblockType;
								offsetMatrix.SetRotateZ(((CGeneral::GetRandomNumber() & 0xFF) - 128.0f) * 0.003f - HALFPI);
							}
							if (ThePaths.m_objectFlags[CRoadBlocks::RoadBlockObjects[nRoadblockNode]] & ObjectEastWest)
								offsetMatrix.GetPosition() = CVector(0.0f, -fOffset, 0.6f);
							else
								offsetMatrix.GetPosition() = CVector(-fOffset, 0.0f, 0.6f);
							CMatrix vehicleMatrix = mapObject->m_matrix * offsetMatrix;
							float fModelRadius = CModelInfo::GetModelInfo(vehicleId)->GetColModel()->boundingSphere.radius - 0.25f;
							int16 colliding = 0;
							CWorld::FindObjectsKindaColliding(vehicleMatrix.GetPosition(), fModelRadius, 0, &colliding, 2, nil, false, true, true, false, false);
							if (!colliding) {
								CAutomobile *pVehicle = new CAutomobile(vehicleId, RANDOM_VEHICLE);
								pVehicle->m_status = STATUS_ABANDONED;
								// pVehicle->GetHeightAboveRoad(); // called but return value is ignored?
								vehicleMatrix.GetPosition().z += fModelRadius - 0.6f;
								pVehicle->m_matrix = vehicleMatrix;
								pVehicle->PlaceOnRoadProperly();
								pVehicle->bIsStatic = false;
								pVehicle->m_matrix.UpdateRW();
								pVehicle->m_nDoorLock = CARLOCK_UNLOCKED;
								CCarCtrl::JoinCarWithRoadSystem(pVehicle);
								pVehicle->bIsLocked = false;
								pVehicle->AutoPilot.m_nCarMission = MISSION_NONE;
								pVehicle->AutoPilot.m_nTempAction = TEMPACT_NONE;
								pVehicle->AutoPilot.m_nCurrentLane = 0;
								pVehicle->AutoPilot.m_nNextLane = 0;
								pVehicle->AutoPilot.m_fMaxTrafficSpeed = 0.0f;
								pVehicle->AutoPilot.m_nCruiseSpeed = 0.0f;
								pVehicle->bExtendedRange = true;
								if (pVehicle->UsesSiren(pVehicle->GetModelIndex()) && CGeneral::GetRandomNumber() & 1)
									pVehicle->m_bSirenOrAlarm = true;
								if (pVehicle->m_matrix.GetForward().z > 0.94f) {
									CVisibilityPlugins::SetClumpAlpha(pVehicle->GetClump(), 0);
									CWorld::Add(pVehicle);
									pVehicle->bCreateRoadBlockPeds = true;
									pVehicle->m_nRoadblockType = nRoadblockType;
									pVehicle->m_nRoadblockNode = nRoadblockNode;
								}
								else {
									delete pVehicle;
								}
							}
						}
					}
				}
			}
		} else {
			CRoadBlocks::InOrOut[nRoadblockNode] = false;
		}
	}
}

STARTPATCHES
	InjectHook(0x436F50, &CRoadBlocks::Init, PATCH_JUMP);
	InjectHook(0x4376A0, &CRoadBlocks::GenerateRoadBlockCopsForCar, PATCH_JUMP);
	InjectHook(0x436FA0, &CRoadBlocks::GenerateRoadBlocks, PATCH_JUMP);
ENDPATCHES