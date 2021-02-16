#include "common.h"

#include "CarCtrl.h"

#include "Accident.h"
#include "Automobile.h"
#include "Camera.h"
#include "CarAI.h"
#include "CarGen.h"
#include "Cranes.h"
#include "Curves.h"
#include "CutsceneMgr.h"
#include "Gangs.h"
#include "Garages.h"
#include "General.h"
#include "IniFile.h"
#include "ModelIndices.h"
#include "PathFind.h"
#include "Ped.h"
#include "PlayerInfo.h"
#include "PlayerPed.h"
#include "Wanted.h"
#include "Pools.h"
#include "Renderer.h"
#include "RoadBlocks.h"
#include "Timer.h"
#include "TrafficLights.h"
#include "Streaming.h"
#include "VisibilityPlugins.h"
#include "Vehicle.h"
#include "Fire.h"
#include "World.h"
#include "Zones.h"

#define DISTANCE_TO_SPAWN_ROADBLOCK_PEDS 51.0f
#define DISTANCE_TO_SCAN_FOR_DANGER 11.0f
#define SAFE_DISTANCE_TO_PED 3.0f
#define INFINITE_Z 1000000000.0f

#define VEHICLE_HEIGHT_DIFF_TO_CONSIDER_WEAVING 4.0f
#define PED_HEIGHT_DIFF_TO_CONSIDER_WEAVING 4.0f
#define OBJECT_HEIGHT_DIFF_TO_CONSIDER_WEAVING 8.0f
#define WIDTH_COEF_TO_WEAVE_SAFELY 1.2f
#define OBJECT_WIDTH_TO_WEAVE 0.3f
#define PED_WIDTH_TO_WEAVE 0.8f

#define PATH_DIRECTION_NONE 0
#define PATH_DIRECTION_STRAIGHT 1
#define PATH_DIRECTION_RIGHT 2
#define PATH_DIRECTION_LEFT 4

#define ATTEMPTS_TO_FIND_NEXT_NODE 15

#define DISTANCE_TO_SWITCH_FROM_BLOCK_TO_STOP 5.0f
#define DISTANCE_TO_SWITCH_FROM_STOP_TO_BLOCK 10.0f
#define MAX_SPEED_TO_ACCOUNT_IN_INTERCEPTING 0.13f
#define DISTANCE_TO_NEXT_NODE_TO_CONSIDER_SLOWING_DOWN 40.0f
#define MAX_ANGLE_TO_STEER_AT_HIGH_SPEED 0.2f
#define MIN_SPEED_TO_START_LIMITING_STEER 0.45f
#define DISTANCE_TO_NEXT_NODE_TO_SELECT_NEW 5.0f
#define DISTANCE_TO_FACING_NEXT_NODE_TO_SELECT_NEW 8.0f
#define DEFAULT_MAX_STEER_ANGLE 0.5f
#define MIN_LOWERING_SPEED_COEFFICIENT 0.4f
#define MAX_ANGLE_FOR_SPEED_LIMITING 1.2f
#define MIN_ANGLE_FOR_SPEED_LIMITING 0.4f
#define MIN_ANGLE_FOR_SPEED_LIMITING_BETWEEN_NODES 0.1f
#define MIN_ANGLE_TO_APPLY_HANDBRAKE 0.7f
#define MIN_SPEED_TO_APPLY_HANDBRAKE 0.3f

int CCarCtrl::NumLawEnforcerCars;
int CCarCtrl::NumAmbulancesOnDuty;
int CCarCtrl::NumFiretrucksOnDuty;
bool CCarCtrl::bCarsGeneratedAroundCamera;
float CCarCtrl::CarDensityMultiplier = 1.0f;
int32 CCarCtrl::NumMissionCars;
int32 CCarCtrl::NumRandomCars;
int32 CCarCtrl::NumParkedCars;
int32 CCarCtrl::NumPermanentCars;
int8 CCarCtrl::CountDownToCarsAtStart;
int32 CCarCtrl::MaxNumberOfCarsInUse = 12;
uint32 CCarCtrl::LastTimeLawEnforcerCreated;
uint32 CCarCtrl::LastTimeFireTruckCreated;
uint32 CCarCtrl::LastTimeAmbulanceCreated;
int32 CCarCtrl::TotalNumOfCarsOfRating[TOTAL_CUSTOM_CLASSES];
int32 CCarCtrl::NextCarOfRating[TOTAL_CUSTOM_CLASSES];
int32 CCarCtrl::CarArrays[TOTAL_CUSTOM_CLASSES][MAX_CAR_MODELS_IN_ARRAY];
CVehicle* apCarsToKeep[MAX_CARS_TO_KEEP];
uint32 aCarsToKeepTime[MAX_CARS_TO_KEEP];

void
CCarCtrl::GenerateRandomCars()
{
	if (CCutsceneMgr::IsRunning())
		return;
	if (NumRandomCars < 30){
		if (CountDownToCarsAtStart == 0){
			GenerateOneRandomCar();
		}
		else if (--CountDownToCarsAtStart == 0) {
			for (int i = 0; i < 50; i++)
				GenerateOneRandomCar();
			CTheCarGenerators::GenerateEvenIfPlayerIsCloseCounter = 20;
		}
	}
	/* Approximately once per 4 seconds. */
	if ((CTimer::GetTimeInMilliseconds() & 0xFFFFF000) != (CTimer::GetPreviousTimeInMilliseconds() & 0xFFFFF000))
		GenerateEmergencyServicesCar();
}

void
CCarCtrl::GenerateOneRandomCar()
{
	static int32 unk = 0;
	CPlayerInfo* pPlayer = &CWorld::Players[CWorld::PlayerInFocus];
	CVector vecTargetPos = FindPlayerCentreOfWorld(CWorld::PlayerInFocus);
	CVector2D vecPlayerSpeed = FindPlayerSpeed();
	CZoneInfo zone;
	CTheZones::GetZoneInfoForTimeOfDay(&vecTargetPos, &zone);
	pPlayer->m_nTrafficMultiplier = pPlayer->m_fRoadDensity * zone.carDensity;
	if (NumRandomCars >= pPlayer->m_nTrafficMultiplier * CarDensityMultiplier * CIniFile::CarNumberMultiplier)
		return;
	if (NumFiretrucksOnDuty + NumAmbulancesOnDuty + NumParkedCars + NumMissionCars + NumLawEnforcerCars + NumRandomCars >= MaxNumberOfCarsInUse)
		return;
	CWanted* pWanted = pPlayer->m_pPed->m_pWanted;
	int carClass;
	int carModel;
	if (pWanted->GetWantedLevel() > 1 && NumLawEnforcerCars < pWanted->m_MaximumLawEnforcerVehicles &&
		pWanted->m_CurrentCops < pWanted->m_MaxCops && (
			pWanted->GetWantedLevel() > 3 ||
			pWanted->GetWantedLevel() > 2 && CTimer::GetTimeInMilliseconds() > LastTimeLawEnforcerCreated + 5000 ||
			pWanted->GetWantedLevel() > 1 && CTimer::GetTimeInMilliseconds() > LastTimeLawEnforcerCreated + 8000)) {
		/* Last pWanted->GetWantedLevel() > 1 is unnecessary but I added it for better readability. */
		/* Wouldn't be surprised it was there originally but was optimized out. */
		carClass = COPS;
		carModel = ChoosePoliceCarModel();
	}else{
		carModel = ChooseModel(&zone, &vecTargetPos, &carClass);
		if (carClass == COPS && pWanted->GetWantedLevel() >= 1)
			/* All cop spawns with wanted level are handled by condition above. */
			/* In particular it means that cop cars never spawn if player has wanted level of 1. */
			return;
	}
	float frontX, frontY;
	float preferredDistance, angleLimit;
	bool invertAngleLimitTest;
	CVector spawnPosition;
	int32 curNodeId, nextNodeId;
	float positionBetweenNodes;
	bool testForCollision;
	CVehicle* pPlayerVehicle = FindPlayerVehicle();
	CVector2D vecPlayerVehicleSpeed;
	float fPlayerVehicleSpeed;
	if (pPlayerVehicle) {
		vecPlayerVehicleSpeed = FindPlayerVehicle()->GetMoveSpeed();
		fPlayerVehicleSpeed = vecPlayerVehicleSpeed.Magnitude();
	}
	if (TheCamera.GetForward().z < -0.9f){
		/* Player uses topdown camera. */
		/* Spawn essentially anywhere. */
		frontX = frontY = 0.707f; /* 45 degrees */
		angleLimit = -1.0f;
		invertAngleLimitTest = true;
		preferredDistance = 40.0f;
		/* BUG: testForCollision not initialized in original game. */
		testForCollision = false;
	}else if (!pPlayerVehicle){
		/* Player is not in vehicle. */
		testForCollision = true;
		frontX = TheCamera.CamFrontXNorm;
		frontY = TheCamera.CamFrontYNorm;
		switch (CTimer::GetFrameCounter() & 1) {
		case 0:
			/* Spawn a vehicle relatively far away from player. */
			/* Forward to his current direction (camera direction). */
			angleLimit = 0.707f; /* 45 degrees */
			invertAngleLimitTest = true;
			preferredDistance = 120.0f * TheCamera.GenerationDistMultiplier;
			break;
		case 1:
			/* Spawn a vehicle close to player to his side. */
			/* Kinda not within camera angle. */
			angleLimit = 0.707f; /* 45 degrees */
			invertAngleLimitTest = false;
			preferredDistance = 40.0f;
			break;
		}
	}else if (fPlayerVehicleSpeed > 0.4f){ /* 72 km/h */
		/* Player is moving fast in vehicle */
		/* Prefer spawning vehicles very far away from him. */
		frontX = vecPlayerVehicleSpeed.x / fPlayerVehicleSpeed;
		frontY = vecPlayerVehicleSpeed.y / fPlayerVehicleSpeed;
		testForCollision = false;
		switch (CTimer::GetFrameCounter() & 3) {
		case 0:
		case 1:
			/* Spawn a vehicle in a very narrow gap in front of a player */
			angleLimit = 0.85f; /* approx 30 degrees */
			invertAngleLimitTest = true;
			preferredDistance = 120.0f * TheCamera.GenerationDistMultiplier;
			break;
		case 2:
			/* Spawn a vehicle relatively far away from player. */
			/* Forward to his current direction (camera direction). */
			angleLimit = 0.707f; /* 45 degrees */
			invertAngleLimitTest = true;
			preferredDistance = 120.0f * TheCamera.GenerationDistMultiplier;
			break;
		case 3:
			/* Spawn a vehicle close to player to his side. */
			/* Kinda not within camera angle. */
			angleLimit = 0.707f; /* 45 degrees */
			invertAngleLimitTest = false;
			preferredDistance = 40.0f;
			break;
		}
	}else if (fPlayerVehicleSpeed > 0.1f){ /* 18 km/h */
		/* Player is moving moderately fast in vehicle */
		/* Spawn more vehicles to player's side. */
		frontX = vecPlayerVehicleSpeed.x / fPlayerVehicleSpeed;
		frontY = vecPlayerVehicleSpeed.y / fPlayerVehicleSpeed;
		testForCollision = false;
		switch (CTimer::GetFrameCounter() & 3) {
		case 0:
			/* Spawn a vehicle in a very narrow gap in front of a player */
			angleLimit = 0.85f; /* approx 30 degrees */
			invertAngleLimitTest = true;
			preferredDistance = 120.0f * TheCamera.GenerationDistMultiplier;
			break;
		case 1:
			/* Spawn a vehicle relatively far away from player. */
			/* Forward to his current direction (camera direction). */
			angleLimit = 0.707f; /* 45 degrees */
			invertAngleLimitTest = true;
			preferredDistance = 120.0f * TheCamera.GenerationDistMultiplier;
			break;
		case 2:
		case 3:
			/* Spawn a vehicle close to player to his side. */
			/* Kinda not within camera angle. */
			angleLimit = 0.707f; /* 45 degrees */
			invertAngleLimitTest = false;
			preferredDistance = 40.0f;
			break;
		}
	}else{
		/* Player is in vehicle but moving very slow. */
		/* Then use camera direction instead of vehicle direction. */
		testForCollision = true;
		frontX = TheCamera.CamFrontXNorm;
		frontY = TheCamera.CamFrontYNorm;
		switch (CTimer::GetFrameCounter() & 1) {
		case 0:
			/* Spawn a vehicle relatively far away from player. */
			/* Forward to his current direction (camera direction). */
			angleLimit = 0.707f; /* 45 degrees */
			invertAngleLimitTest = true;
			preferredDistance = 120.0f * TheCamera.GenerationDistMultiplier;
			break;
		case 1:
			/* Spawn a vehicle close to player to his side. */
			/* Kinda not within camera angle. */
			angleLimit = 0.707f; /* 45 degrees */
			invertAngleLimitTest = false;
			preferredDistance = 40.0f;
			break;
		}
	}
	if (!ThePaths.NewGenerateCarCreationCoors(vecTargetPos.x, vecTargetPos.y, frontX, frontY,
		preferredDistance, angleLimit, invertAngleLimitTest, &spawnPosition, &curNodeId, &nextNodeId,
		&positionBetweenNodes, carClass == COPS && pWanted->GetWantedLevel() >= 1))
		return;
	int16 colliding;
	CWorld::FindObjectsKindaColliding(spawnPosition, 10.0f, true, &colliding, 2, nil, false, true, true, false, false);
	if (colliding)
		/* If something is already present in spawn position, do not create vehicle*/
		return;
	if (!ThePaths.TestCoorsCloseness(vecTargetPos, false, spawnPosition))
		/* Testing if spawn position can reach target position via valid path. */
		return;
	int16 idInNode = 0;
	CPathNode* pCurNode = &ThePaths.m_pathNodes[curNodeId];
	CPathNode* pNextNode = &ThePaths.m_pathNodes[nextNodeId];
	while (idInNode < pCurNode->numLinks &&
		ThePaths.ConnectedNode(idInNode + pCurNode->firstLink) != nextNodeId)
		idInNode++;
	int16 connectionId = ThePaths.m_carPathConnections[idInNode + pCurNode->firstLink];
	CCarPathLink* pPathLink = &ThePaths.m_carPathLinks[connectionId];
	int16 lanesOnCurrentRoad = pPathLink->pathNodeIndex == nextNodeId ? pPathLink->numLeftLanes : pPathLink->numRightLanes;
	CVehicleModelInfo* pModelInfo = (CVehicleModelInfo*)CModelInfo::GetModelInfo(carModel);
	if (lanesOnCurrentRoad == 0 || pModelInfo->m_vehicleType == VEHICLE_TYPE_BIKE)
		/* Not spawning vehicle if road is one way and intended direction is opposide to that way. */
		/* Also not spawning bikes but they don't exist in final game. */
		return;
	CAutomobile* pVehicle = new CAutomobile(carModel, RANDOM_VEHICLE);
	pVehicle->AutoPilot.m_nPrevRouteNode = 0;
	pVehicle->AutoPilot.m_nCurrentRouteNode = curNodeId;
	pVehicle->AutoPilot.m_nNextRouteNode = nextNodeId;
	switch (carClass) {
	case POOR:
	case RICH:
	case EXEC:
	case WORKER:
	case SPECIAL:
	case BIG:
	case TAXI:
	case MAFIA:
	case TRIAD:
	case DIABLO:
	case YAKUZA:
	case YARDIE:
	case COLOMB:
	case NINES:
	case GANG8:
	case GANG9:
	{
		pVehicle->AutoPilot.m_nCruiseSpeed = CGeneral::GetRandomNumberInRange(9, 14);
		if (carClass == EXEC)
			pVehicle->AutoPilot.m_nCruiseSpeed = CGeneral::GetRandomNumberInRange(12, 18);
		else if (carClass == POOR || carClass == SPECIAL)
			pVehicle->AutoPilot.m_nCruiseSpeed = CGeneral::GetRandomNumberInRange(7, 10);
		CVehicleModelInfo* pVehicleInfo = pVehicle->GetModelInfo();
		if (pVehicleInfo->GetColModel()->boundingBox.max.y - pVehicle->GetModelInfo()->GetColModel()->boundingBox.min.y > 10.0f || carClass == BIG) {
			pVehicle->AutoPilot.m_nCruiseSpeed *= 3;
			pVehicle->AutoPilot.m_nCruiseSpeed /= 4;
		}
		pVehicle->AutoPilot.m_fMaxTrafficSpeed = pVehicle->AutoPilot.m_nCruiseSpeed;
		pVehicle->AutoPilot.m_nCarMission = MISSION_CRUISE;
		pVehicle->AutoPilot.m_nTempAction = TEMPACT_NONE;
		pVehicle->AutoPilot.m_nDrivingStyle = DRIVINGSTYLE_STOP_FOR_CARS;
		break;
	}
	case COPS:
		pVehicle->AutoPilot.m_nTempAction = TEMPACT_NONE;
		if (CWorld::Players[CWorld::PlayerInFocus].m_pPed->m_pWanted->GetWantedLevel() != 0){
			pVehicle->AutoPilot.m_nCruiseSpeed = CCarAI::FindPoliceCarSpeedForWantedLevel(pVehicle);
			pVehicle->AutoPilot.m_fMaxTrafficSpeed = pVehicle->AutoPilot.m_nCruiseSpeed / 2;
			pVehicle->AutoPilot.m_nCarMission = CCarAI::FindPoliceCarMissionForWantedLevel();
			pVehicle->AutoPilot.m_nDrivingStyle = DRIVINGSTYLE_AVOID_CARS;
		}else{
			pVehicle->AutoPilot.m_nCruiseSpeed = CGeneral::GetRandomNumberInRange(12, 16);
			pVehicle->AutoPilot.m_fMaxTrafficSpeed = pVehicle->AutoPilot.m_nCruiseSpeed;
			pVehicle->AutoPilot.m_nDrivingStyle = DRIVINGSTYLE_STOP_FOR_CARS;
			pVehicle->AutoPilot.m_nCarMission = MISSION_CRUISE;
		}
		if (carModel == MI_FBICAR){
			pVehicle->m_currentColour1 = 0;
			pVehicle->m_currentColour2 = 0;
			/* FBI cars are gray in carcols, but we want them black if they going after player. */
		}
	default:
		break;
	}
	if (pVehicle && pVehicle->GetModelIndex() == MI_MRWHOOP)
		pVehicle->m_bSirenOrAlarm = true;
	pVehicle->AutoPilot.m_nNextPathNodeInfo = connectionId;
	pVehicle->AutoPilot.m_nNextLane = pVehicle->AutoPilot.m_nCurrentLane = CGeneral::GetRandomNumber() % lanesOnCurrentRoad;
	CColBox* boundingBox = &CModelInfo::GetModelInfo(pVehicle->GetModelIndex())->GetColModel()->boundingBox;
	float carLength = 1.0f + (boundingBox->max.y - boundingBox->min.y) / 2;
	float distanceBetweenNodes = (pCurNode->GetPosition() - pNextNode->GetPosition()).Magnitude2D();
	/* If car is so long that it doesn't fit between two car nodes, place it directly in the middle. */
	/* Otherwise put it at least in a way that full vehicle length fits between two nodes. */
	if (distanceBetweenNodes / 2 < carLength)
		positionBetweenNodes = 0.5f;
	else
		positionBetweenNodes = Min(1.0f - carLength / distanceBetweenNodes, Max(carLength / distanceBetweenNodes, positionBetweenNodes));
	pVehicle->AutoPilot.m_nNextDirection = (curNodeId >= nextNodeId) ? 1 : -1;
	if (pCurNode->numLinks == 1){
		/* Do not create vehicle if there is nowhere to go. */
		delete pVehicle;
		return;
	}
	int16 nextConnection = pVehicle->AutoPilot.m_nNextPathNodeInfo;
	int16 newLink;
	while (nextConnection == pVehicle->AutoPilot.m_nNextPathNodeInfo){
		newLink = CGeneral::GetRandomNumber() % pCurNode->numLinks;
		nextConnection = ThePaths.m_carPathConnections[newLink + pCurNode->firstLink];
	}
	pVehicle->AutoPilot.m_nCurrentPathNodeInfo = nextConnection;
	pVehicle->AutoPilot.m_nCurrentDirection = (ThePaths.ConnectedNode(newLink + pCurNode->firstLink) >= curNodeId) ? 1 : -1;
	CVector2D vecBetweenNodes = pNextNode->GetPosition() - pCurNode->GetPosition();
	float forwardX, forwardY;
	float distBetweenNodes = vecBetweenNodes.Magnitude();
	if (distanceBetweenNodes == 0.0f){
		forwardX = 1.0f;
		forwardY = 0.0f;
	}else{
		forwardX = vecBetweenNodes.x / distBetweenNodes;
		forwardY = vecBetweenNodes.y / distBetweenNodes;
	}
	/* I think the following might be some form of SetRotateZOnly. */
	/* Setting up direction between two car nodes. */
	pVehicle->GetForward() = CVector(forwardX, forwardY, 0.0f);
	pVehicle->GetRight() = CVector(forwardY, -forwardX, 0.0f);
	pVehicle->GetUp() = CVector(0.0f, 0.0f, 1.0f);

	float currentPathLinkForwardX = pVehicle->AutoPilot.m_nCurrentDirection * ThePaths.m_carPathLinks[pVehicle->AutoPilot.m_nCurrentPathNodeInfo].GetDirX();
	float currentPathLinkForwardY = pVehicle->AutoPilot.m_nCurrentDirection * ThePaths.m_carPathLinks[pVehicle->AutoPilot.m_nCurrentPathNodeInfo].GetDirY();
	float nextPathLinkForwardX = pVehicle->AutoPilot.m_nNextDirection * ThePaths.m_carPathLinks[pVehicle->AutoPilot.m_nNextPathNodeInfo].GetDirX();
	float nextPathLinkForwardY = pVehicle->AutoPilot.m_nNextDirection * ThePaths.m_carPathLinks[pVehicle->AutoPilot.m_nNextPathNodeInfo].GetDirY();

#ifdef FIX_BUGS
	CCarPathLink* pCurrentLink;
	CCarPathLink* pNextLink;
	CVector positionOnCurrentLinkIncludingLane;
	CVector positionOnNextLinkIncludingLane;
	float directionCurrentLinkX;
	float directionCurrentLinkY;
	float directionNextLinkX;
	float directionNextLinkY;
	if (positionBetweenNodes < 0.5f) {
		pCurrentLink = &ThePaths.m_carPathLinks[pVehicle->AutoPilot.m_nCurrentPathNodeInfo];
		pNextLink = &ThePaths.m_carPathLinks[pVehicle->AutoPilot.m_nNextPathNodeInfo];
		positionOnCurrentLinkIncludingLane = CVector(
			pCurrentLink->GetX() + ((pVehicle->AutoPilot.m_nCurrentLane + pCurrentLink->OneWayLaneOffset()) * LANE_WIDTH) * currentPathLinkForwardY,
			pCurrentLink->GetY() - ((pVehicle->AutoPilot.m_nCurrentLane + pCurrentLink->OneWayLaneOffset()) * LANE_WIDTH) * currentPathLinkForwardX,
			0.0f);
		positionOnNextLinkIncludingLane = CVector(
			pNextLink->GetX() + ((pVehicle->AutoPilot.m_nNextLane + pNextLink->OneWayLaneOffset()) * LANE_WIDTH) * nextPathLinkForwardY,
			pNextLink->GetY() - ((pVehicle->AutoPilot.m_nNextLane + pNextLink->OneWayLaneOffset()) * LANE_WIDTH) * nextPathLinkForwardX,
			0.0f);
		directionCurrentLinkX = pCurrentLink->GetDirX() * pVehicle->AutoPilot.m_nCurrentDirection;
		directionCurrentLinkY = pCurrentLink->GetDirY() * pVehicle->AutoPilot.m_nCurrentDirection;
		directionNextLinkX = pNextLink->GetDirX() * pVehicle->AutoPilot.m_nNextDirection;
		directionNextLinkY = pNextLink->GetDirY() * pVehicle->AutoPilot.m_nNextDirection;
		/* We want to make a path between two links that may not have the same forward directions a curve. */
		pVehicle->AutoPilot.m_nTimeToSpendOnCurrentCurve = CCurves::CalcSpeedScaleFactor(
			&positionOnCurrentLinkIncludingLane,
			&positionOnNextLinkIncludingLane,
			directionCurrentLinkX, directionCurrentLinkY,
			directionNextLinkX, directionNextLinkY
		) * (1000.0f / pVehicle->AutoPilot.m_fMaxTrafficSpeed);
		pVehicle->AutoPilot.m_nTimeEnteredCurve = CTimer::GetTimeInMilliseconds() -
			(uint32)((0.5f + positionBetweenNodes) * pVehicle->AutoPilot.m_nTimeToSpendOnCurrentCurve);
	}
	else {
		PickNextNodeRandomly(pVehicle);
		pVehicle->AutoPilot.m_nTimeEnteredCurve = CTimer::GetTimeInMilliseconds() -
			(uint32)((positionBetweenNodes - 0.5f) * pVehicle->AutoPilot.m_nTimeToSpendOnCurrentCurve);

		pCurrentLink = &ThePaths.m_carPathLinks[pVehicle->AutoPilot.m_nCurrentPathNodeInfo];
		pNextLink = &ThePaths.m_carPathLinks[pVehicle->AutoPilot.m_nNextPathNodeInfo];
		positionOnCurrentLinkIncludingLane = CVector(
			pCurrentLink->GetX() + ((pVehicle->AutoPilot.m_nCurrentLane + pCurrentLink->OneWayLaneOffset()) * LANE_WIDTH) * currentPathLinkForwardY,
			pCurrentLink->GetY() - ((pVehicle->AutoPilot.m_nCurrentLane + pCurrentLink->OneWayLaneOffset()) * LANE_WIDTH) * currentPathLinkForwardX,
			0.0f);
		positionOnNextLinkIncludingLane = CVector(
			pNextLink->GetX() + ((pVehicle->AutoPilot.m_nNextLane + pNextLink->OneWayLaneOffset()) * LANE_WIDTH) * nextPathLinkForwardY,
			pNextLink->GetY() - ((pVehicle->AutoPilot.m_nNextLane + pNextLink->OneWayLaneOffset()) * LANE_WIDTH) * nextPathLinkForwardX,
			0.0f);
		directionCurrentLinkX = pCurrentLink->GetDirX() * pVehicle->AutoPilot.m_nCurrentDirection;
		directionCurrentLinkY = pCurrentLink->GetDirY() * pVehicle->AutoPilot.m_nCurrentDirection;
		directionNextLinkX = pNextLink->GetDirX() * pVehicle->AutoPilot.m_nNextDirection;
		directionNextLinkY = pNextLink->GetDirY() * pVehicle->AutoPilot.m_nNextDirection;
	}
#else

	CCarPathLink* pCurrentLink = &ThePaths.m_carPathLinks[pVehicle->AutoPilot.m_nCurrentPathNodeInfo];
	CCarPathLink* pNextLink = &ThePaths.m_carPathLinks[pVehicle->AutoPilot.m_nNextPathNodeInfo];
	CVector positionOnCurrentLinkIncludingLane(
		pCurrentLink->GetX() + ((pVehicle->AutoPilot.m_nCurrentLane + pCurrentLink->OneWayLaneOffset()) * LANE_WIDTH) * currentPathLinkForwardY,
		pCurrentLink->GetY() - ((pVehicle->AutoPilot.m_nCurrentLane + pCurrentLink->OneWayLaneOffset()) * LANE_WIDTH) * currentPathLinkForwardX,
		0.0f);
	CVector positionOnNextLinkIncludingLane(
		pNextLink->GetX() + ((pVehicle->AutoPilot.m_nNextLane + pNextLink->OneWayLaneOffset()) * LANE_WIDTH) * nextPathLinkForwardY,
		pNextLink->GetY() - ((pVehicle->AutoPilot.m_nNextLane + pNextLink->OneWayLaneOffset()) * LANE_WIDTH) * nextPathLinkForwardX,
		0.0f);
	float directionCurrentLinkX = pCurrentLink->GetDirX() * pVehicle->AutoPilot.m_nCurrentDirection;
	float directionCurrentLinkY = pCurrentLink->GetDirY() * pVehicle->AutoPilot.m_nCurrentDirection;
	float directionNextLinkX = pNextLink->GetDirX() * pVehicle->AutoPilot.m_nNextDirection;
	float directionNextLinkY = pNextLink->GetDirY() * pVehicle->AutoPilot.m_nNextDirection;
	/* We want to make a path between two links that may not have the same forward directions a curve. */
	pVehicle->AutoPilot.m_nTimeToSpendOnCurrentCurve = CCurves::CalcSpeedScaleFactor(
		&positionOnCurrentLinkIncludingLane,
		&positionOnNextLinkIncludingLane,
		directionCurrentLinkX, directionCurrentLinkY,
		directionNextLinkX, directionNextLinkY
	) * (1000.0f / pVehicle->AutoPilot.m_fMaxTrafficSpeed);
	pVehicle->AutoPilot.m_nTimeEnteredCurve = CTimer::GetTimeInMilliseconds() -
		(0.5f + positionBetweenNodes) * pVehicle->AutoPilot.m_nTimeToSpendOnCurrentCurve;
#endif
	CVector directionCurrentLink(directionCurrentLinkX, directionCurrentLinkY, 0.0f);
	CVector directionNextLink(directionNextLinkX, directionNextLinkY, 0.0f);
	CVector positionIncludingCurve;
	CVector directionIncludingCurve;
	CCurves::CalcCurvePoint(
		&positionOnCurrentLinkIncludingLane,
		&positionOnNextLinkIncludingLane,
		&directionCurrentLink,
		&directionNextLink,
		GetPositionAlongCurrentCurve(pVehicle),
		pVehicle->AutoPilot.m_nTimeToSpendOnCurrentCurve,
		&positionIncludingCurve,
		&directionIncludingCurve
	);
	CVector vectorBetweenNodes = pCurNode->GetPosition() - pNextNode->GetPosition();
	CVector finalPosition = positionIncludingCurve + vectorBetweenNodes * 2.0f / vectorBetweenNodes.Magnitude();
	finalPosition.z = positionBetweenNodes * pNextNode->GetZ() +
		(1.0f - positionBetweenNodes) * pCurNode->GetZ();
	float groundZ = INFINITE_Z;
	CColPoint colPoint;
	CEntity* pEntity;
	if (CWorld::ProcessVerticalLine(finalPosition, 1000.0f, colPoint, pEntity, true, false, false, false, true, false, nil))
		groundZ = colPoint.point.z;
	if (CWorld::ProcessVerticalLine(finalPosition, -1000.0f, colPoint, pEntity, true, false, false, false, true, false, nil)){
		if (ABS(colPoint.point.z - finalPosition.z) < ABS(groundZ - finalPosition.z))
			groundZ = colPoint.point.z;
	}
	if (groundZ == INFINITE_Z || ABS(groundZ - finalPosition.z) > 7.0f) {
		/* Failed to find ground or too far from expected position. */
		delete pVehicle;
		return;
	}
	finalPosition.z = groundZ + pVehicle->GetHeightAboveRoad();
	pVehicle->SetPosition(finalPosition);
	pVehicle->SetMoveSpeed(directionIncludingCurve / GAME_SPEED_TO_CARAI_SPEED);
	CVector2D speedDifferenceWithTarget = (CVector2D)pVehicle->GetMoveSpeed() - vecPlayerSpeed;
	CVector2D distanceToTarget = positionIncludingCurve - vecTargetPos;
	switch (carClass) {
	case POOR:
	case RICH:
	case EXEC:
	case WORKER:
	case SPECIAL:
	case BIG:
	case TAXI:
	case MAFIA:
	case TRIAD:
	case DIABLO:
	case YAKUZA:
	case YARDIE:
	case COLOMB:
	case NINES:
	case GANG8:
	case GANG9:
		pVehicle->SetStatus(STATUS_SIMPLE);
		break;
	case COPS:
		pVehicle->SetStatus((pVehicle->AutoPilot.m_nCarMission == MISSION_CRUISE) ? STATUS_SIMPLE : STATUS_PHYSICS);
		pVehicle->ChangeLawEnforcerState(1);
		break;
	default:
		break;
	}
	CVisibilityPlugins::SetClumpAlpha(pVehicle->GetClump(), 0);
	if (!pVehicle->GetIsOnScreen()){
		if ((vecTargetPos - pVehicle->GetPosition()).Magnitude2D() > 50.0f) {
			/* Too far away cars that are not visible aren't needed. */
			delete pVehicle;
			return;
		}
	}else if((vecTargetPos - pVehicle->GetPosition()).Magnitude2D() > TheCamera.GenerationDistMultiplier * 130.0f ||
		(vecTargetPos - pVehicle->GetPosition()).Magnitude2D() < TheCamera.GenerationDistMultiplier * 110.0f){
		delete pVehicle;
		return;
	}else if((TheCamera.GetPosition() - pVehicle->GetPosition()).Magnitude2D() < 90.0f * TheCamera.GenerationDistMultiplier){
		delete pVehicle;
		return;
	}
	CVehicleModelInfo* pVehicleModel = pVehicle->GetModelInfo();
	float radiusToTest = pVehicleModel->GetColModel()->boundingSphere.radius;
	if (testForCollision){
		CWorld::FindObjectsKindaColliding(pVehicle->GetPosition(), radiusToTest + 20.0f, true, &colliding, 2, nil, false, true, false, false, false);
		if (colliding){
			delete pVehicle;
			return;
		}
	}
	CWorld::FindObjectsKindaColliding(pVehicle->GetPosition(), radiusToTest, true, &colliding, 2, nil, false, true, false, false, false);
	if (colliding){
		delete pVehicle;
		return;
	}
	if (speedDifferenceWithTarget.x * distanceToTarget.x +
		speedDifferenceWithTarget.y * distanceToTarget.y >= 0.0f){
		delete pVehicle;
		return;
	}
	pVehicleModel->AvoidSameVehicleColour(&pVehicle->m_currentColour1, &pVehicle->m_currentColour2);
	CWorld::Add(pVehicle);
	if (carClass == COPS)
		CCarAI::AddPoliceCarOccupants(pVehicle);
	else
		pVehicle->SetUpDriver();
	if ((CGeneral::GetRandomNumber() & 0x3F) == 0){ /* 1/64 probability */
		pVehicle->SetStatus(STATUS_PHYSICS);
		pVehicle->AutoPilot.m_nDrivingStyle = DRIVINGSTYLE_AVOID_CARS;
		pVehicle->AutoPilot.m_nCruiseSpeed += 10;
	}
	if (carClass == COPS)
		LastTimeLawEnforcerCreated = CTimer::GetTimeInMilliseconds();
}

int32
CCarCtrl::ChooseModel(CZoneInfo* pZone, CVector* pPos, int* pClass) {
	int32 model = -1;
	while (model == -1 || !CStreaming::HasModelLoaded(model)){
		int rnd = CGeneral::GetRandomNumberInRange(0, 1000);
		if (rnd < pZone->carThreshold[0])
			model = CCarCtrl::ChooseCarModel((*pClass = POOR));
		else if (rnd < pZone->carThreshold[1])
			model = CCarCtrl::ChooseCarModel((*pClass = RICH));
		else if (rnd < pZone->carThreshold[2])
			model = CCarCtrl::ChooseCarModel((*pClass = EXEC));
		else if (rnd < pZone->carThreshold[3])
			model = CCarCtrl::ChooseCarModel((*pClass = WORKER));
		else if (rnd < pZone->carThreshold[4])
			model = CCarCtrl::ChooseCarModel((*pClass = SPECIAL));
		else if (rnd < pZone->carThreshold[5])
			model = CCarCtrl::ChooseCarModel((*pClass = BIG));
		else if (rnd < pZone->copThreshold)
			*pClass = COPS, model = CCarCtrl::ChoosePoliceCarModel();
		else if (rnd < pZone->gangThreshold[0])
			model = CCarCtrl::ChooseGangCarModel((*pClass = MAFIA) - MAFIA);
		else if (rnd < pZone->gangThreshold[1])
			model = CCarCtrl::ChooseGangCarModel((*pClass = TRIAD) - MAFIA);
		else if (rnd < pZone->gangThreshold[2])
			model = CCarCtrl::ChooseGangCarModel((*pClass = DIABLO) - MAFIA);
		else if (rnd < pZone->gangThreshold[3])
			model = CCarCtrl::ChooseGangCarModel((*pClass = YAKUZA) - MAFIA);
		else if (rnd < pZone->gangThreshold[4])
			model = CCarCtrl::ChooseGangCarModel((*pClass = YARDIE) - MAFIA);
		else if (rnd < pZone->gangThreshold[5])
			model = CCarCtrl::ChooseGangCarModel((*pClass = COLOMB) - MAFIA);
		else if (rnd < pZone->gangThreshold[6])
			model = CCarCtrl::ChooseGangCarModel((*pClass = NINES) - MAFIA);
		else if (rnd < pZone->gangThreshold[7])
			model = CCarCtrl::ChooseGangCarModel((*pClass = GANG8) - MAFIA);
		else if (rnd < pZone->gangThreshold[8])
			model = CCarCtrl::ChooseGangCarModel((*pClass = GANG9) - MAFIA);
		else
			model = CCarCtrl::ChooseCarModel((*pClass = TAXI));
	}
	return model;
}

int32
CCarCtrl::ChooseCarModel(int32 vehclass)
{
	int32 model = -1;
	switch (vehclass) {
	case POOR:
	case RICH:
	case EXEC:
	case WORKER:
	case SPECIAL:
	case BIG:
	case TAXI:
	{
		if (TotalNumOfCarsOfRating[vehclass] == 0)
			debug("ChooseCarModel : No cars of type %d have been declared\n", vehclass);
		model = CarArrays[vehclass][NextCarOfRating[vehclass]];
		int32 total = TotalNumOfCarsOfRating[vehclass];
		NextCarOfRating[vehclass] += CGeneral::GetRandomNumberInRange(1, total);
		while (NextCarOfRating[vehclass] >= total)
			NextCarOfRating[vehclass] -= total;
		//NextCarOfRating[vehclass] %= total;
		TotalNumOfCarsOfRating[vehclass] = total; /* why... */
	}
	default:
		break;
	}
	return model;
}

int32
CCarCtrl::ChoosePoliceCarModel(void)
{
	if (FindPlayerPed()->m_pWanted->AreSwatRequired() &&
		CStreaming::HasModelLoaded(MI_ENFORCER) &&
		CStreaming::HasModelLoaded(MI_POLICE))
		return ((CGeneral::GetRandomNumber() & 0xF) == 0) ? MI_ENFORCER : MI_POLICE;
	if (FindPlayerPed()->m_pWanted->AreFbiRequired() &&
		CStreaming::HasModelLoaded(MI_FBICAR) &&
		CStreaming::HasModelLoaded(MI_FBI))
		return MI_FBICAR;
	if (FindPlayerPed()->m_pWanted->AreArmyRequired() &&
		CStreaming::HasModelLoaded(MI_RHINO) &&
		CStreaming::HasModelLoaded(MI_BARRACKS) &&
		CStreaming::HasModelLoaded(MI_ARMY))
		return CGeneral::GetRandomTrueFalse() ? MI_BARRACKS : MI_RHINO;
	return MI_POLICE;
}

int32
CCarCtrl::ChooseGangCarModel(int32 gang)
{
	if (CStreaming::HasModelLoaded(MI_GANG01 + 2 * gang) &&
		CStreaming::HasModelLoaded(MI_GANG02 + 2 * gang))
		return CGangs::GetGangVehicleModel(gang);
	return -1;
}

void
CCarCtrl::AddToCarArray(int32 id, int32 vehclass)
{
	CarArrays[vehclass][TotalNumOfCarsOfRating[vehclass]++] = id;
}

void
CCarCtrl::RemoveDistantCars()
{
	for (int i = CPools::GetVehiclePool()->GetSize()-1; i >= 0; i--) {
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetSlot(i);
		if (!pVehicle)
			continue;
		PossiblyRemoveVehicle(pVehicle);
		if (pVehicle->bCreateRoadBlockPeds){
			if ((pVehicle->GetPosition() - FindPlayerCentreOfWorld(CWorld::PlayerInFocus)).Magnitude2D() < DISTANCE_TO_SPAWN_ROADBLOCK_PEDS) {
				CRoadBlocks::GenerateRoadBlockCopsForCar(pVehicle, pVehicle->m_nRoadblockType, pVehicle->m_nRoadblockNode);
				pVehicle->bCreateRoadBlockPeds = false;
			}
		}
	}
}

void
CCarCtrl::PossiblyRemoveVehicle(CVehicle* pVehicle)
{
#ifdef FIX_BUGS
	if (pVehicle->bIsLocked)
		return;
#endif
	CVector vecPlayerPos = FindPlayerCentreOfWorld(CWorld::PlayerInFocus);
	/* BUG: this variable is initialized only in if-block below but can be used outside of it. */
	if (!IsThisVehicleInteresting(pVehicle) && !pVehicle->bIsLocked &&
		pVehicle->CanBeDeleted() && !CCranes::IsThisCarBeingTargettedByAnyCrane(pVehicle)){
		if (pVehicle->bFadeOut && CVisibilityPlugins::GetClumpAlpha(pVehicle->GetClump()) == 0){
			CWorld::Remove(pVehicle);
			delete pVehicle;
			return;
		}
		float distanceToPlayer = (pVehicle->GetPosition() - vecPlayerPos).Magnitude2D();
		float threshold = 50.0f;
		if (pVehicle->GetIsOnScreen() ||
			TheCamera.Cams[TheCamera.ActiveCam].LookingLeft ||
			TheCamera.Cams[TheCamera.ActiveCam].LookingRight ||
			TheCamera.Cams[TheCamera.ActiveCam].LookingBehind ||
			TheCamera.GetLookDirection() == 0 ||
			pVehicle->VehicleCreatedBy == PARKED_VEHICLE ||
			pVehicle->GetModelIndex() == MI_AMBULAN ||
			pVehicle->GetModelIndex() == MI_FIRETRUCK ||
			pVehicle->bIsLawEnforcer ||
			pVehicle->bIsCarParkVehicle
			){
			threshold = 130.0f * TheCamera.GenerationDistMultiplier;
		}
		if (pVehicle->bExtendedRange)
			threshold *= 1.5f;
		if (distanceToPlayer > threshold && !CGarages::IsPointWithinHideOutGarage(pVehicle->GetPosition())){
			if (pVehicle->GetIsOnScreen() && CRenderer::IsEntityCullZoneVisible(pVehicle)) {
				pVehicle->bFadeOut = true;
			}else{
				CWorld::Remove(pVehicle);
				delete pVehicle;
			}
			return;
		}
	}
	if ((pVehicle->GetStatus() == STATUS_SIMPLE || pVehicle->GetStatus() == STATUS_PHYSICS && pVehicle->AutoPilot.m_nDrivingStyle == DRIVINGSTYLE_STOP_FOR_CARS) &&
		CTimer::GetTimeInMilliseconds() - pVehicle->AutoPilot.m_nTimeToStartMission > 5000 &&
		!pVehicle->GetIsOnScreen() &&
		(pVehicle->GetPosition() - vecPlayerPos).Magnitude2D() > 25.0f &&
		!IsThisVehicleInteresting(pVehicle) &&
		!pVehicle->bIsLocked &&
		pVehicle->CanBeDeleted() &&
		!CTrafficLights::ShouldCarStopForLight(pVehicle, true) &&
		!CTrafficLights::ShouldCarStopForBridge(pVehicle) &&
		!CGarages::IsPointWithinHideOutGarage(pVehicle->GetPosition())){
		CWorld::Remove(pVehicle);
		delete pVehicle;
		return;
	}
	if (pVehicle->GetStatus() != STATUS_WRECKED || pVehicle->m_nTimeOfDeath == 0)
		return;
	if (CTimer::GetTimeInMilliseconds() > pVehicle->m_nTimeOfDeath + 60000 &&
		!(pVehicle->GetIsOnScreen() && CRenderer::IsEntityCullZoneVisible(pVehicle)) ){
		if ((pVehicle->GetPosition() - vecPlayerPos).MagnitudeSqr() > SQR(7.5f)){
			if (!CGarages::IsPointWithinHideOutGarage(pVehicle->GetPosition())){
				CWorld::Remove(pVehicle);
				delete pVehicle;
			}
		}
	}
}

int32
CCarCtrl::CountCarsOfType(int32 mi)
{
	int32 total = 0;
	for (int i = CPools::GetVehiclePool()->GetSize()-1; i >= 0; i--) {
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetSlot(i);
		if (!pVehicle)
			continue;
		if (pVehicle->GetModelIndex() == mi)
			total++;
	}
	return total;
}

void
CCarCtrl::UpdateCarOnRails(CVehicle* pVehicle)
{
	if (pVehicle->AutoPilot.m_nTempAction == TEMPACT_WAIT){
		pVehicle->SetMoveSpeed(0.0f, 0.0f, 0.0f);
		pVehicle->AutoPilot.ModifySpeed(0.0f);
		if (CTimer::GetTimeInMilliseconds() > pVehicle->AutoPilot.m_nTempAction){
			pVehicle->AutoPilot.m_nTempAction = TEMPACT_NONE;
			pVehicle->AutoPilot.m_nAntiReverseTimer = 0;
			pVehicle->AutoPilot.m_nTimeToStartMission = 0;
		}
		return;
	}
	SlowCarOnRailsDownForTrafficAndLights(pVehicle);
	if (pVehicle->AutoPilot.m_nTimeEnteredCurve + pVehicle->AutoPilot.m_nTimeToSpendOnCurrentCurve <= CTimer::GetTimeInMilliseconds())
		PickNextNodeAccordingStrategy(pVehicle);
	if (pVehicle->GetStatus() == STATUS_PHYSICS)
		return;
	CCarPathLink* pCurrentLink = &ThePaths.m_carPathLinks[pVehicle->AutoPilot.m_nCurrentPathNodeInfo];
	CCarPathLink* pNextLink = &ThePaths.m_carPathLinks[pVehicle->AutoPilot.m_nNextPathNodeInfo];
	float currentPathLinkForwardX = pCurrentLink->GetDirX() * pVehicle->AutoPilot.m_nCurrentDirection;
	float currentPathLinkForwardY = pCurrentLink->GetDirY() * pVehicle->AutoPilot.m_nCurrentDirection;
	float nextPathLinkForwardX = pNextLink->GetDirX() * pVehicle->AutoPilot.m_nNextDirection;
	float nextPathLinkForwardY = pNextLink->GetDirY() * pVehicle->AutoPilot.m_nNextDirection;
	CVector positionOnCurrentLinkIncludingLane(
		pCurrentLink->GetX() + ((pVehicle->AutoPilot.m_nCurrentLane + pCurrentLink->OneWayLaneOffset()) * LANE_WIDTH) * currentPathLinkForwardY,
		pCurrentLink->GetY() - ((pVehicle->AutoPilot.m_nCurrentLane + pCurrentLink->OneWayLaneOffset()) * LANE_WIDTH) * currentPathLinkForwardX,
		0.0f);
	CVector positionOnNextLinkIncludingLane(
		pNextLink->GetX() + ((pVehicle->AutoPilot.m_nNextLane + pNextLink->OneWayLaneOffset()) * LANE_WIDTH) * nextPathLinkForwardY,
		pNextLink->GetY() - ((pVehicle->AutoPilot.m_nNextLane + pNextLink->OneWayLaneOffset()) * LANE_WIDTH) * nextPathLinkForwardX,
		0.0f);
	CVector directionCurrentLink(currentPathLinkForwardX, currentPathLinkForwardY, 0.0f);
	CVector directionNextLink(nextPathLinkForwardX, nextPathLinkForwardY, 0.0f);
	CVector positionIncludingCurve;
	CVector directionIncludingCurve;
	CCurves::CalcCurvePoint(
		&positionOnCurrentLinkIncludingLane,
		&positionOnNextLinkIncludingLane,
		&directionCurrentLink,
		&directionNextLink,
		GetPositionAlongCurrentCurve(pVehicle),
		pVehicle->AutoPilot.m_nTimeToSpendOnCurrentCurve,
		&positionIncludingCurve,
		&directionIncludingCurve
	);
	positionIncludingCurve.z = 15.0f;
	DragCarToPoint(pVehicle, &positionIncludingCurve);
	pVehicle->SetMoveSpeed(directionIncludingCurve / GAME_SPEED_TO_CARAI_SPEED);
}

float
CCarCtrl::FindMaximumSpeedForThisCarInTraffic(CVehicle* pVehicle)
{
	if (pVehicle->AutoPilot.m_nDrivingStyle == DRIVINGSTYLE_AVOID_CARS ||
		pVehicle->AutoPilot.m_nDrivingStyle == DRIVINGSTYLE_PLOUGH_THROUGH)
		return pVehicle->AutoPilot.m_nCruiseSpeed;
	float left = pVehicle->GetPosition().x - DISTANCE_TO_SCAN_FOR_DANGER;
	float right = pVehicle->GetPosition().x + DISTANCE_TO_SCAN_FOR_DANGER;
	float top = pVehicle->GetPosition().y - DISTANCE_TO_SCAN_FOR_DANGER;
	float bottom = pVehicle->GetPosition().y + DISTANCE_TO_SCAN_FOR_DANGER;
	int xstart = Max(0, CWorld::GetSectorIndexX(left));
	int xend = Min(NUMSECTORS_X - 1, CWorld::GetSectorIndexX(right));
	int ystart = Max(0, CWorld::GetSectorIndexY(top));
	int yend = Min(NUMSECTORS_Y - 1, CWorld::GetSectorIndexY(bottom));
	assert(xstart <= xend);
	assert(ystart <= yend);

	float maxSpeed = pVehicle->AutoPilot.m_nCruiseSpeed;

	CWorld::AdvanceCurrentScanCode();

	for (int y = ystart; y <= yend; y++){
		for (int x = xstart; x <= xend; x++){
			CSector* s = CWorld::GetSector(x, y);
			SlowCarDownForCarsSectorList(s->m_lists[ENTITYLIST_VEHICLES], pVehicle, left, top, right, bottom, &maxSpeed, pVehicle->AutoPilot.m_nCruiseSpeed);
			SlowCarDownForCarsSectorList(s->m_lists[ENTITYLIST_VEHICLES_OVERLAP], pVehicle, left, top, right, bottom, &maxSpeed, pVehicle->AutoPilot.m_nCruiseSpeed);
			SlowCarDownForPedsSectorList(s->m_lists[ENTITYLIST_PEDS], pVehicle, left, top, right, bottom, &maxSpeed, pVehicle->AutoPilot.m_nCruiseSpeed);
			SlowCarDownForPedsSectorList(s->m_lists[ENTITYLIST_PEDS_OVERLAP], pVehicle, left, top, right, bottom, &maxSpeed, pVehicle->AutoPilot.m_nCruiseSpeed);
		}
	}
	pVehicle->bWarnedPeds = true;
	if (pVehicle->AutoPilot.m_nDrivingStyle == DRIVINGSTYLE_STOP_FOR_CARS)
		return maxSpeed;
	return (maxSpeed + pVehicle->AutoPilot.m_nCruiseSpeed) / 2;
}

void
CCarCtrl::ScanForPedDanger(CVehicle* pVehicle)
{
	bool storedSlowDownFlag = pVehicle->AutoPilot.m_bSlowedDownBecauseOfPeds;
	float left = pVehicle->GetPosition().x - DISTANCE_TO_SCAN_FOR_DANGER;
	float right = pVehicle->GetPosition().x + DISTANCE_TO_SCAN_FOR_DANGER;
	float top = pVehicle->GetPosition().y - DISTANCE_TO_SCAN_FOR_DANGER;
	float bottom = pVehicle->GetPosition().y + DISTANCE_TO_SCAN_FOR_DANGER;
	int xstart = Max(0, CWorld::GetSectorIndexX(left));
	int xend = Min(NUMSECTORS_X - 1, CWorld::GetSectorIndexX(right));
	int ystart = Max(0, CWorld::GetSectorIndexY(top));
	int yend = Min(NUMSECTORS_Y - 1, CWorld::GetSectorIndexY(bottom));
	assert(xstart <= xend);
	assert(ystart <= yend);

	float maxSpeed = pVehicle->AutoPilot.m_nCruiseSpeed;

	CWorld::AdvanceCurrentScanCode();

	for (int y = ystart; y <= yend; y++) {
		for (int x = xstart; x <= xend; x++) {
			CSector* s = CWorld::GetSector(x, y);
			SlowCarDownForPedsSectorList(s->m_lists[ENTITYLIST_PEDS], pVehicle, left, top, right, bottom, &maxSpeed, pVehicle->AutoPilot.m_nCruiseSpeed);
			SlowCarDownForPedsSectorList(s->m_lists[ENTITYLIST_PEDS_OVERLAP], pVehicle, left, top, right, bottom, &maxSpeed, pVehicle->AutoPilot.m_nCruiseSpeed);
		}
	}
	pVehicle->bWarnedPeds = true;
	pVehicle->AutoPilot.m_bSlowedDownBecauseOfPeds = storedSlowDownFlag;
}

void
CCarCtrl::SlowCarOnRailsDownForTrafficAndLights(CVehicle* pVehicle)
{
	float maxSpeed;
	if (CTrafficLights::ShouldCarStopForLight(pVehicle, false) || CTrafficLights::ShouldCarStopForBridge(pVehicle)){
		CCarAI::CarHasReasonToStop(pVehicle);
		maxSpeed = 0.0f;
	}else{
		maxSpeed = FindMaximumSpeedForThisCarInTraffic(pVehicle);
	}
	float curSpeed = pVehicle->AutoPilot.m_fMaxTrafficSpeed;
	if (maxSpeed >= curSpeed){
		if (maxSpeed > curSpeed)
			pVehicle->AutoPilot.ModifySpeed(Min(maxSpeed, curSpeed + 0.05f * CTimer::GetTimeStep()));
	}else if (curSpeed != 0.0f) {
		if (curSpeed < 0.1f)
			pVehicle->AutoPilot.ModifySpeed(0.0f);
		else
			pVehicle->AutoPilot.ModifySpeed(Max(maxSpeed, curSpeed - 0.5f * CTimer::GetTimeStep()));
	}
}

void CCarCtrl::SlowCarDownForPedsSectorList(CPtrList& lst, CVehicle* pVehicle, float x_inf, float y_inf, float x_sup, float y_sup, float* pSpeed, float curSpeed)
{
	float frontOffset = pVehicle->GetModelInfo()->GetColModel()->boundingBox.max.y;
	float frontSafe = frontOffset + SAFE_DISTANCE_TO_PED;
	for (CPtrNode* pNode = lst.first; pNode != nil; pNode = pNode->next){
		CPed* pPed = (CPed*)pNode->item;
		if (pPed->m_scanCode == CWorld::GetCurrentScanCode())
			continue;
		if (!pPed->bUsesCollision)
			continue;
		pPed->m_scanCode = CWorld::GetCurrentScanCode();
		CVector vecPedPos = pPed->GetPosition();
		if (vecPedPos.x < x_inf || vecPedPos.x > x_sup)
			continue;
		if (vecPedPos.y < y_inf || vecPedPos.y > y_sup)
			continue;
		if (ABS(vecPedPos.z - pVehicle->GetPosition().z) >= 4.0f)
			continue;
		CVector vecToPed = vecPedPos - pVehicle->GetPosition();
		float dotDirection = DotProduct(pVehicle->GetForward(), vecToPed);
		float dotVelocity = DotProduct(pVehicle->GetForward(), pVehicle->GetMoveSpeed());
		if (dotDirection <= frontOffset) /* If already run him over, don't care */
			continue;
		float distanceUntilHit = dotDirection - frontOffset;
		float movementTowardsPedPerSecond = GAME_SPEED_TO_METERS_PER_SECOND * dotVelocity;
		if (4 * movementTowardsPedPerSecond <= distanceUntilHit)
			/* If car isn't projected to hit a ped in 4 seconds, don't care */
			continue;
		float sidewaysDistance = ABS(DotProduct(pVehicle->GetRight(), vecToPed));
		float sideLength = pVehicle->GetModelInfo()->GetColModel()->boundingBox.max.x;
		if (pVehicle->m_vehType == VEHICLE_TYPE_BIKE)
			sideLength *= 1.6f;
		if (sideLength + 0.5f < sidewaysDistance)
			/* If car is far enough taking side into account, don't care */
			continue;
		if (pPed->IsPed()){ /* ...how can it not be? */
			if (pPed->GetPedState() != PED_STEP_AWAY && pPed->GetPedState() != PED_DIVE_AWAY){
				if (distanceUntilHit < movementTowardsPedPerSecond){
					/* Very close. Time to evade. */
					if (pVehicle->GetModelIndex() == MI_RCBANDIT){
						if (dotVelocity * GAME_SPEED_TO_METERS_PER_SECOND / 2 > distanceUntilHit)
							pPed->SetEvasiveStep(pVehicle, 0);
					}
					else if (dotVelocity > 0.3f) {
						if (sideLength + 0.1f < sidewaysDistance)
							pPed->SetEvasiveStep(pVehicle, 0);
						else
							pPed->SetEvasiveDive(pVehicle, 0);
					}
					else if (dotVelocity > 0.1f) {
						if (sideLength - 0.5f < sidewaysDistance)
							pPed->SetEvasiveStep(pVehicle, 0);
						else
							pPed->SetEvasiveDive(pVehicle, 0);
					}
				}else{
					/* Relatively safe but annoying. */
					if (pVehicle->GetStatus() == STATUS_PLAYER &&
					  pPed->GetPedState() != PED_FLEE_ENTITY &&
					  pPed->CharCreatedBy == RANDOM_CHAR){
						float angleCarToPed = CGeneral::GetRadianAngleBetweenPoints(
							pVehicle->GetPosition().x, pVehicle->GetPosition().y,
							pPed->GetPosition().x, pPed->GetPosition().y
						);
						angleCarToPed = CGeneral::LimitRadianAngle(angleCarToPed);
						pPed->m_headingRate = CGeneral::LimitRadianAngle(pPed->m_headingRate);
						float visibilityAngle = ABS(angleCarToPed - pPed->m_headingRate);
						if (visibilityAngle > PI)
							visibilityAngle = TWOPI - visibilityAngle;
						if (visibilityAngle < HALFPI || pVehicle->m_nCarHornTimer){
							/* if ped sees the danger or if car horn is on */
							pPed->SetFlee(pVehicle, 2000);
							pPed->bUsePedNodeSeek = false;
							pPed->SetMoveState(PEDMOVE_RUN);
						}
					}else{
						CPlayerPed* pPlayerPed = (CPlayerPed*)pPed;
						if (pPlayerPed->IsPlayer() && dotDirection < frontSafe &&
						  pPlayerPed->IsPedInControl() &&
						  pPlayerPed->m_fMoveSpeed < 1.0f && !pPlayerPed->bIsLooking &&
						  CTimer::GetTimeInMilliseconds() > pPlayerPed->m_lookTimer) {
							pPlayerPed->AnnoyPlayerPed(false);
							pPlayerPed->SetLookFlag(pVehicle, true);
							pPlayerPed->SetLookTimer(1500);
							if (pPlayerPed->GetWeapon()->m_eWeaponType == WEAPONTYPE_UNARMED ||
								pPlayerPed->GetWeapon()->m_eWeaponType == WEAPONTYPE_BASEBALLBAT ||
								pPlayerPed->GetWeapon()->m_eWeaponType == WEAPONTYPE_COLT45 ||
								pPlayerPed->GetWeapon()->m_eWeaponType == WEAPONTYPE_UZI) {
								pPlayerPed->bShakeFist = true;
							}
						}
					}
				}
			}
		}
		/* Ped stuff done. Now vehicle stuff. */
		if (distanceUntilHit < 10.0f){
			if (pVehicle->AutoPilot.m_nDrivingStyle == DRIVINGSTYLE_STOP_FOR_CARS ||
			  pVehicle->AutoPilot.m_nDrivingStyle == DRIVINGSTYLE_SLOW_DOWN_FOR_CARS){
				*pSpeed = Min(*pSpeed, ABS(distanceUntilHit - 1.0f) * 0.1f * curSpeed);
				pVehicle->AutoPilot.m_bSlowedDownBecauseOfPeds = true;
				if (distanceUntilHit < 2.0f){
					pVehicle->AutoPilot.m_nTempAction = TEMPACT_WAIT;
					pVehicle->AutoPilot.m_nTimeTempAction = CTimer::GetTimeInMilliseconds() + 3000;
				}
			}
		}
	}
}

void CCarCtrl::SlowCarDownForCarsSectorList(CPtrList& lst, CVehicle* pVehicle, float x_inf, float y_inf, float x_sup, float y_sup, float* pSpeed, float curSpeed)
{
	for (CPtrNode* pNode = lst.first; pNode != nil; pNode = pNode->next){
		CVehicle* pTestVehicle = (CVehicle*)pNode->item;
		if (pVehicle == pTestVehicle)
			continue;
		if (pTestVehicle->m_scanCode == CWorld::GetCurrentScanCode())
			continue;
		if (!pTestVehicle->bUsesCollision)
			continue;
		pTestVehicle->m_scanCode = CWorld::GetCurrentScanCode();
		CVector boundCenter = pTestVehicle->GetBoundCentre();
		if (boundCenter.x < x_inf || boundCenter.x > x_sup)
			continue;
		if (boundCenter.y < y_inf || boundCenter.y > y_sup)
			continue;
		if (Abs(boundCenter.z - pVehicle->GetPosition().z) < 5.0f)
			SlowCarDownForOtherCar(pTestVehicle, pVehicle, pSpeed, curSpeed);
	}
}

void CCarCtrl::SlowCarDownForOtherCar(CEntity* pOtherEntity, CVehicle* pVehicle, float* pSpeed, float curSpeed)
{
	CVector forwardA = pVehicle->GetForward();
	((CVector2D)forwardA).NormaliseSafe();
	if (DotProduct2D(pOtherEntity->GetPosition() - pVehicle->GetPosition(), forwardA) < 0.0f)
		return;
	CVector forwardB = pOtherEntity->GetForward();
	((CVector2D)forwardB).NormaliseSafe();
	forwardA.z = forwardB.z = 0.0f;
	CVehicle* pOtherVehicle = (CVehicle*)pOtherEntity;
	/* why is the argument CEntity if it's always CVehicle anyway and is casted? */
	float speedOtherX = GAME_SPEED_TO_CARAI_SPEED * pOtherVehicle->GetMoveSpeed().x;
	float speedOtherY = GAME_SPEED_TO_CARAI_SPEED * pOtherVehicle->GetMoveSpeed().y;
	float projectionX = speedOtherX - forwardA.x * curSpeed;
	float projectionY = speedOtherY - forwardA.y * curSpeed;
	float proximityA = TestCollisionBetween2MovingRects(pOtherVehicle, pVehicle, projectionX, projectionY, &forwardA, &forwardB, 0);
	float proximityB = TestCollisionBetween2MovingRects(pVehicle, pOtherVehicle, -projectionX, -projectionY, &forwardB, &forwardA, 1);
	float minProximity = Min(proximityA, proximityB);
	if (minProximity >= 0.0f && minProximity < 1.0f){
		minProximity = Max(0.0f, (minProximity - 0.2f) * 1.25f);
		pVehicle->AutoPilot.m_bSlowedDownBecauseOfCars = true;
		*pSpeed = Min(*pSpeed, minProximity * curSpeed);
	}
	if (minProximity >= 0.0f && minProximity < 0.5f && pOtherEntity->IsVehicle() &&
	  CTimer::GetTimeInMilliseconds() - pVehicle->AutoPilot.m_nTimeToStartMission > 15000 &&
	  CTimer::GetTimeInMilliseconds() - pOtherVehicle->AutoPilot.m_nTimeToStartMission > 15000){
		/* If cars are standing for 15 seconds, annoy one of them and make avoid cars. */
		if (pOtherEntity != FindPlayerVehicle() &&
		  DotProduct2D(pVehicle->GetForward(), pOtherVehicle->GetForward()) < -0.5f &&
		  pVehicle < pOtherVehicle){ /* that comparasion though... */
			*pSpeed = Max(curSpeed / 5, *pSpeed);
			if (pVehicle->GetStatus() == STATUS_SIMPLE){
				pVehicle->SetStatus(STATUS_PHYSICS);
				SwitchVehicleToRealPhysics(pVehicle);
			}
			pVehicle->AutoPilot.m_nDrivingStyle = DRIVINGSTYLE_AVOID_CARS;
			pVehicle->AutoPilot.m_nTimeTempAction = CTimer::GetTimeInMilliseconds() + 1000;
		}
	}
}

float CCarCtrl::TestCollisionBetween2MovingRects(CVehicle* pVehicleA, CVehicle* pVehicleB, float projectionX, float projectionY, CVector* pForwardA, CVector* pForwardB, uint8 id)
{
	CVector2D vecBToA = pVehicleA->GetPosition() - pVehicleB->GetPosition();
	float lenB = pVehicleB->GetModelInfo()->GetColModel()->boundingBox.max.y;
	float widthB = pVehicleB->GetModelInfo()->GetColModel()->boundingBox.max.x;
	float backLenB = -pVehicleB->GetModelInfo()->GetColModel()->boundingBox.min.y;
	float lenA = pVehicleA->GetModelInfo()->GetColModel()->boundingBox.max.y;
	float widthA = pVehicleA->GetModelInfo()->GetColModel()->boundingBox.max.x;
	float backLenA = -pVehicleA->GetModelInfo()->GetColModel()->boundingBox.min.y;
	float proximity = 1.0f;
	float fullWidthB = 2.0f * widthB;
	float fullLenB = lenB + backLenB;
	for (int i = 0; i < 4; i++){
		float testedOffsetX;
		float testedOffsetY;
		switch (i) {
		case 0: /* Front right corner */
			testedOffsetX = vecBToA.x + widthA * pForwardB->y + lenA * pForwardB->x;
			testedOffsetY = vecBToA.y + lenA * pForwardB->y - widthA * pForwardB->x;
			break;
		case 1: /* Front left corner */
			testedOffsetX = vecBToA.x + -widthA * pForwardB->x + lenA * pForwardB->x;
			testedOffsetY = vecBToA.y + lenA * pForwardB->y + widthA * pForwardB->x;
			break;
		case 2: /* Rear right corner */
			testedOffsetX = vecBToA.x + widthA * pForwardB->y - backLenA * pForwardB->x;
			testedOffsetY = vecBToA.y - backLenA * pForwardB->y - widthA * pForwardB->x;
			break;
		case 3: /* Rear left corner */
			testedOffsetX = vecBToA.x - widthA * pForwardB->y - backLenA * pForwardB->x;
			testedOffsetY = vecBToA.y - backLenA * pForwardB->y + widthA * pForwardB->x;
			break;
		default:
			break;
		}
		/* Testing width collision */
		float baseWidthProximity = 0.0f;
		float fullWidthProximity = 1.0f;
		float widthDistance = testedOffsetX * pForwardA->y - testedOffsetY * pForwardA->x;
		float widthProjection = projectionX * pForwardA->y - projectionY * pForwardA->x;
		if (widthDistance > widthB){
			if (widthProjection < 0.0f){
				float proximityWidth = -(widthDistance - widthB) / widthProjection;
				if (proximityWidth < 1.0f){
					baseWidthProximity = proximityWidth;
					fullWidthProximity = Min(1.0f, proximityWidth - fullWidthB / widthProjection);
				}else{
					baseWidthProximity = 1.0f;
				}
			}else{
				baseWidthProximity = 1.0f;
				fullWidthProximity = 1.0f;
			}
		}else if (widthDistance < -widthB){
			if (widthProjection > 0.0f) {
				float proximityWidth = -(widthDistance + widthB) / widthProjection;
				if (proximityWidth < 1.0f) {
					baseWidthProximity = proximityWidth;
					fullWidthProximity = Min(1.0f, proximityWidth + fullWidthB / widthProjection);
				}
				else {
					baseWidthProximity = 1.0f;
				}
			}
			else {
				baseWidthProximity = 1.0f;
				fullWidthProximity = 1.0f;
			}
		}else if (widthProjection > 0.0f){
			fullWidthProximity = (widthB - widthDistance) / widthProjection;
		}else if (widthProjection < 0.0f){
			fullWidthProximity = -(widthB + widthDistance) / widthProjection;
		}
		/* Testing length collision */
		float baseLengthProximity = 0.0f;
		float fullLengthProximity = 1.0f;
		float lenDistance = testedOffsetX * pForwardA->x + testedOffsetY * pForwardA->y;
		float lenProjection = projectionX * pForwardA->x + projectionY * pForwardA->y;
		if (lenDistance > lenB) {
			if (lenProjection < 0.0f) {
				float proximityLength = -(lenDistance - lenB) / lenProjection;
				if (proximityLength < 1.0f) {
					baseLengthProximity = proximityLength;
					fullLengthProximity = Min(1.0f, proximityLength - fullLenB / lenProjection);
				}
				else {
					baseLengthProximity = 1.0f;
				}
			}
			else {
				baseLengthProximity = 1.0f;
				fullLengthProximity = 1.0f;
			}
		}
		else if (lenDistance < -backLenB) {
			if (lenProjection > 0.0f) {
				float proximityLength = -(lenDistance + backLenB) / lenProjection;
				if (proximityLength < 1.0f) {
					baseLengthProximity = proximityLength;
					fullLengthProximity = Min(1.0f, proximityLength + fullLenB / lenProjection);
				}
				else {
					baseLengthProximity = 1.0f;
				}
			}
			else {
				baseLengthProximity = 1.0f;
				fullLengthProximity = 1.0f;
			}
		}
		else if (lenProjection > 0.0f) {
			fullLengthProximity = (lenB - lenDistance) / lenProjection;
		}
		else if (lenProjection < 0.0f) {
			fullLengthProximity = -(backLenB + lenDistance) / lenProjection;
		}
		float baseProximity = Max(baseWidthProximity, baseLengthProximity);
		if (baseProximity < fullWidthProximity && baseProximity < fullLengthProximity)
			proximity = Min(proximity, baseProximity);
	}
	return proximity;
}

float CCarCtrl::FindAngleToWeaveThroughTraffic(CVehicle* pVehicle, CPhysical* pTarget, float angleToTarget, float angleForward)
{
	float distanceToTest = Min(2.0f, pVehicle->GetMoveSpeed().Magnitude2D() * 2.5f + 1.0f) * 12.0f;
	float left = pVehicle->GetPosition().x - distanceToTest;
	float right = pVehicle->GetPosition().x + distanceToTest;
	float top = pVehicle->GetPosition().y - distanceToTest;
	float bottom = pVehicle->GetPosition().y + distanceToTest;
	int xstart = Max(0, CWorld::GetSectorIndexX(left));
	int xend = Min(NUMSECTORS_X - 1, CWorld::GetSectorIndexX(right));
	int ystart = Max(0, CWorld::GetSectorIndexY(top));
	int yend = Min(NUMSECTORS_Y - 1, CWorld::GetSectorIndexY(bottom));
	assert(xstart <= xend);
	assert(ystart <= yend);

	float angleToWeaveLeft = angleToTarget;
	float angleToWeaveRight = angleToTarget;

	CWorld::AdvanceCurrentScanCode();

	float angleToWeaveLeftLastIteration = -9999.9f;
	float angleToWeaveRightLastIteration = -9999.9f;

	while (angleToWeaveLeft != angleToWeaveLeftLastIteration ||
		   angleToWeaveRight != angleToWeaveRightLastIteration){
		angleToWeaveLeftLastIteration = angleToWeaveLeft;
		angleToWeaveRightLastIteration = angleToWeaveRight;
		for (int y = ystart; y <= yend; y++) {
			for (int x = xstart; x <= xend; x++) {
				CSector* s = CWorld::GetSector(x, y);
				WeaveThroughCarsSectorList(s->m_lists[ENTITYLIST_VEHICLES], pVehicle, pTarget,
					left, top, right, bottom, &angleToWeaveLeft, &angleToWeaveRight);
				WeaveThroughCarsSectorList(s->m_lists[ENTITYLIST_VEHICLES_OVERLAP], pVehicle, pTarget,
					left, top, right, bottom, &angleToWeaveLeft, &angleToWeaveRight);
				WeaveThroughPedsSectorList(s->m_lists[ENTITYLIST_PEDS], pVehicle, pTarget,
					left, top, right, bottom, &angleToWeaveLeft, &angleToWeaveRight);
				WeaveThroughPedsSectorList(s->m_lists[ENTITYLIST_PEDS_OVERLAP], pVehicle, pTarget,
					left, top, right, bottom, &angleToWeaveLeft, &angleToWeaveRight);
				WeaveThroughObjectsSectorList(s->m_lists[ENTITYLIST_OBJECTS], pVehicle,
					left, top, right, bottom, &angleToWeaveLeft, &angleToWeaveRight);
				WeaveThroughObjectsSectorList(s->m_lists[ENTITYLIST_OBJECTS_OVERLAP], pVehicle,
					left, top, right, bottom, &angleToWeaveLeft, &angleToWeaveRight);
			}
		}
	}
	float angleDiffFromActualToTarget = LimitRadianAngle(angleForward - angleToTarget);
	float angleToBisectActualToTarget = LimitRadianAngle(angleToTarget + angleDiffFromActualToTarget / 2);
	float angleDiffLeft = LimitRadianAngle(angleToWeaveLeft - angleToBisectActualToTarget);
	angleDiffLeft = ABS(angleDiffLeft);
	float angleDiffRight = LimitRadianAngle(angleToWeaveRight - angleToBisectActualToTarget);
	angleDiffRight = ABS(angleDiffRight);
	if (angleDiffLeft > HALFPI && angleDiffRight > HALFPI)
		return angleToBisectActualToTarget;
	if (ABS(angleDiffLeft - angleDiffRight) < 0.08f)
		return angleToWeaveRight;
	return angleDiffLeft < angleDiffRight ? angleToWeaveLeft : angleToWeaveRight;
}

void CCarCtrl::WeaveThroughCarsSectorList(CPtrList& lst, CVehicle* pVehicle, CPhysical* pTarget, float x_inf, float y_inf, float x_sup, float y_sup, float* pAngleToWeaveLeft, float* pAngleToWeaveRight)
{
	for (CPtrNode* pNode = lst.first; pNode != nil; pNode = pNode->next) {
		CVehicle* pTestVehicle = (CVehicle*)pNode->item;
		if (pTestVehicle->m_scanCode == CWorld::GetCurrentScanCode())
			continue;
		if (!pTestVehicle->bUsesCollision)
			continue;
		if (pTestVehicle == pTarget)
			continue;
		pTestVehicle->m_scanCode = CWorld::GetCurrentScanCode();
		if (pTestVehicle->GetBoundCentre().x < x_inf || pTestVehicle->GetBoundCentre().x > x_sup)
			continue;
		if (pTestVehicle->GetBoundCentre().y < y_inf || pTestVehicle->GetBoundCentre().y > y_sup)
			continue;
		if (Abs(pTestVehicle->GetPosition().z - pVehicle->GetPosition().z) >= VEHICLE_HEIGHT_DIFF_TO_CONSIDER_WEAVING)
			continue;
		if (pTestVehicle != pVehicle)
			WeaveForOtherCar(pTestVehicle, pVehicle, pAngleToWeaveLeft, pAngleToWeaveRight);
	}
}

void CCarCtrl::WeaveForOtherCar(CEntity* pOtherEntity, CVehicle* pVehicle, float* pAngleToWeaveLeft, float* pAngleToWeaveRight)
{
	if (pVehicle->AutoPilot.m_nCarMission == MISSION_RAMPLAYER_CLOSE && pOtherEntity == FindPlayerVehicle())
		return;
	if (pVehicle->AutoPilot.m_nCarMission == MISSION_RAMCAR_CLOSE && pOtherEntity == pVehicle->AutoPilot.m_pTargetCar)
		return;
	CVehicle* pOtherCar = (CVehicle*)pOtherEntity;
	CVector2D vecDiff = pOtherCar->GetPosition() - pVehicle->GetPosition();
	float angleBetweenVehicles = CGeneral::GetATanOfXY(vecDiff.x, vecDiff.y);
	float distance = vecDiff.Magnitude();
	if (distance < 1.0f)
		return;
	if (DotProduct2D(pVehicle->GetMoveSpeed() - pOtherCar->GetMoveSpeed(), vecDiff) * 110.0f -
	  pOtherCar->GetModelInfo()->GetColModel()->boundingSphere.radius -
	  pVehicle->GetModelInfo()->GetColModel()->boundingSphere.radius < distance)
		return;
	CVector2D forward = pVehicle->GetForward();
	forward.NormaliseSafe();
	float forwardAngle = CGeneral::GetATanOfXY(forward.x, forward.y);
	float angleDiff = angleBetweenVehicles - forwardAngle;
	float lenProjection = ABS(pOtherCar->GetColModel()->boundingBox.max.y * sin(angleDiff));
	float widthProjection = ABS(pOtherCar->GetColModel()->boundingBox.max.x * cos(angleDiff));
	float lengthToEvade = (2 * (lenProjection + widthProjection) + WIDTH_COEF_TO_WEAVE_SAFELY * 2 * pVehicle->GetColModel()->boundingBox.max.x) / distance;
	float diffToLeftAngle = LimitRadianAngle(angleBetweenVehicles - *pAngleToWeaveLeft);
	diffToLeftAngle = ABS(diffToLeftAngle);
	float angleToWeave = lengthToEvade / 2;
	if (diffToLeftAngle < angleToWeave){
		*pAngleToWeaveLeft = angleBetweenVehicles - angleToWeave;
		while (*pAngleToWeaveLeft < -PI)
			*pAngleToWeaveLeft += TWOPI;
	}
	float diffToRightAngle = LimitRadianAngle(angleBetweenVehicles - *pAngleToWeaveRight);
	diffToRightAngle = ABS(diffToRightAngle);
	if (diffToRightAngle < angleToWeave){
		*pAngleToWeaveRight = angleBetweenVehicles + angleToWeave;
		while (*pAngleToWeaveRight > PI)
			*pAngleToWeaveRight -= TWOPI;
	}
}

void CCarCtrl::WeaveThroughPedsSectorList(CPtrList& lst, CVehicle* pVehicle, CPhysical* pTarget, float x_inf, float y_inf, float x_sup, float y_sup, float* pAngleToWeaveLeft, float* pAngleToWeaveRight)
{
	for (CPtrNode* pNode = lst.first; pNode != nil; pNode = pNode->next) {
		CPed* pPed = (CPed*)pNode->item;
		if (pPed->m_scanCode == CWorld::GetCurrentScanCode())
			continue;
		if (!pPed->bUsesCollision)
			continue;
		if (pPed == pTarget)
			continue;
		pPed->m_scanCode = CWorld::GetCurrentScanCode();
		if (pPed->GetPosition().x < x_inf || pPed->GetPosition().x > x_sup)
			continue;
		if (pPed->GetPosition().y < y_inf || pPed->GetPosition().y > y_sup)
			continue;
		if (Abs(pPed->GetPosition().z - pVehicle->GetPosition().z) >= PED_HEIGHT_DIFF_TO_CONSIDER_WEAVING)
			continue;
		if (pPed->m_pCurSurface != pVehicle)
			WeaveForPed(pPed, pVehicle, pAngleToWeaveLeft, pAngleToWeaveRight);
	}

}
void CCarCtrl::WeaveForPed(CEntity* pOtherEntity, CVehicle* pVehicle, float* pAngleToWeaveLeft, float* pAngleToWeaveRight)
{
	if (pVehicle->AutoPilot.m_nCarMission == MISSION_RAMPLAYER_CLOSE && pOtherEntity == FindPlayerPed())
		return;
	CPed* pPed = (CPed*)pOtherEntity;
	CVector2D vecDiff = pPed->GetPosition() - pVehicle->GetPosition();
	float angleBetweenVehicleAndPed = CGeneral::GetATanOfXY(vecDiff.x, vecDiff.y);
	float distance = vecDiff.Magnitude();
	float lengthToEvade = (WIDTH_COEF_TO_WEAVE_SAFELY * 2 * pVehicle->GetColModel()->boundingBox.max.x + PED_WIDTH_TO_WEAVE) / distance;
	float diffToLeftAngle = LimitRadianAngle(angleBetweenVehicleAndPed - *pAngleToWeaveLeft);
	diffToLeftAngle = ABS(diffToLeftAngle);
	float angleToWeave = lengthToEvade / 2;
	if (diffToLeftAngle < angleToWeave) {
		*pAngleToWeaveLeft = angleBetweenVehicleAndPed - angleToWeave;
		while (*pAngleToWeaveLeft < -PI)
			*pAngleToWeaveLeft += TWOPI;
	}
	float diffToRightAngle = LimitRadianAngle(angleBetweenVehicleAndPed - *pAngleToWeaveRight);
	diffToRightAngle = ABS(diffToRightAngle);
	if (diffToRightAngle < angleToWeave) {
		*pAngleToWeaveRight = angleBetweenVehicleAndPed + angleToWeave;
		while (*pAngleToWeaveRight > PI)
			*pAngleToWeaveRight -= TWOPI;
	}
}

void CCarCtrl::WeaveThroughObjectsSectorList(CPtrList& lst, CVehicle* pVehicle, float x_inf, float y_inf, float x_sup, float y_sup, float* pAngleToWeaveLeft, float* pAngleToWeaveRight)
{
	for (CPtrNode* pNode = lst.first; pNode != nil; pNode = pNode->next) {
		CObject* pObject = (CObject*)pNode->item;
		if (pObject->m_scanCode == CWorld::GetCurrentScanCode())
			continue;
		if (!pObject->bUsesCollision)
			continue;
		pObject->m_scanCode = CWorld::GetCurrentScanCode();
		if (pObject->GetPosition().x < x_inf || pObject->GetPosition().x > x_sup)
			continue;
		if (pObject->GetPosition().y < y_inf || pObject->GetPosition().y > y_sup)
			continue;
		if (Abs(pObject->GetPosition().z - pVehicle->GetPosition().z) >= OBJECT_HEIGHT_DIFF_TO_CONSIDER_WEAVING)
			continue;
		if (pObject->GetUp().z > 0.9f)
			WeaveForObject(pObject, pVehicle, pAngleToWeaveLeft, pAngleToWeaveRight);
	}
}

void CCarCtrl::WeaveForObject(CEntity* pOtherEntity, CVehicle* pVehicle, float* pAngleToWeaveLeft, float* pAngleToWeaveRight)
{
	float rightCoef;
	float forwardCoef;
	if (pOtherEntity->GetModelIndex() == MI_TRAFFICLIGHTS){
		rightCoef = 2.957f;
		forwardCoef = 0.147f;
	}else if (pOtherEntity->GetModelIndex() == MI_SINGLESTREETLIGHTS1){
		rightCoef = 0.744f;
		forwardCoef = 0.0f;
	}else if (pOtherEntity->GetModelIndex() == MI_SINGLESTREETLIGHTS2){
		rightCoef = 0.043f;
		forwardCoef = 0.0f;
	}else if (pOtherEntity->GetModelIndex() == MI_SINGLESTREETLIGHTS3){
		rightCoef = 1.143f;
		forwardCoef = 0.145f;
	}else if (pOtherEntity->GetModelIndex() == MI_DOUBLESTREETLIGHTS){
		rightCoef = 0.0f;
		forwardCoef = -0.048f;
	}else if (IsTreeModel(pOtherEntity->GetModelIndex())){
		rightCoef = 0.0f;
		forwardCoef = 0.0f;
	}else if (pOtherEntity->GetModelIndex() == MI_STREETLAMP1 || pOtherEntity->GetModelIndex() == MI_STREETLAMP2){
		rightCoef = 0.0f;
		forwardCoef = 0.0f;
	}else
		return;
	CObject* pObject = (CObject*)pOtherEntity;
	CVector2D vecDiff = pObject->GetPosition() +
		rightCoef * pObject->GetRight() +
		forwardCoef * pObject->GetForward() -
		pVehicle->GetPosition();
	float angleBetweenVehicleAndObject = CGeneral::GetATanOfXY(vecDiff.x, vecDiff.y);
	float distance = vecDiff.Magnitude();
	float lengthToEvade = (WIDTH_COEF_TO_WEAVE_SAFELY * 2 * pVehicle->GetColModel()->boundingBox.max.x + OBJECT_WIDTH_TO_WEAVE) / distance;
	float diffToLeftAngle = LimitRadianAngle(angleBetweenVehicleAndObject - *pAngleToWeaveLeft);
	diffToLeftAngle = ABS(diffToLeftAngle);
	float angleToWeave = lengthToEvade / 2;
	if (diffToLeftAngle < angleToWeave) {
		*pAngleToWeaveLeft = angleBetweenVehicleAndObject - angleToWeave;
		while (*pAngleToWeaveLeft < -PI)
			*pAngleToWeaveLeft += TWOPI;
	}
	float diffToRightAngle = LimitRadianAngle(angleBetweenVehicleAndObject - *pAngleToWeaveRight);
	diffToRightAngle = ABS(diffToRightAngle);
	if (diffToRightAngle < angleToWeave) {
		*pAngleToWeaveRight = angleBetweenVehicleAndObject + angleToWeave;
		while (*pAngleToWeaveRight > PI)
			*pAngleToWeaveRight -= TWOPI;
	}
}

bool CCarCtrl::PickNextNodeAccordingStrategy(CVehicle* pVehicle)
{
	switch (pVehicle->AutoPilot.m_nCarMission){
	case MISSION_RAMPLAYER_FARAWAY:
	case MISSION_BLOCKPLAYER_FARAWAY:
		PickNextNodeToChaseCar(pVehicle,
			FindPlayerCoors().x,
			FindPlayerCoors().y,
#ifdef FIX_PATHFIND_BUG
			FindPlayerCoors().z,
#endif
			FindPlayerVehicle());
		return false;
	case MISSION_GOTOCOORDS:
	case MISSION_GOTOCOORDS_ACCURATE:
		return PickNextNodeToFollowPath(pVehicle);
	case MISSION_RAMCAR_FARAWAY:
	case MISSION_BLOCKCAR_FARAWAY:
		PickNextNodeToChaseCar(pVehicle,
			pVehicle->AutoPilot.m_pTargetCar->GetPosition().x,
			pVehicle->AutoPilot.m_pTargetCar->GetPosition().y,
#ifdef FIX_PATHFIND_BUG
			pVehicle->AutoPilot.m_pTargetCar->GetPosition().z,
#endif
			pVehicle->AutoPilot.m_pTargetCar);
		return false;
	default:
		PickNextNodeRandomly(pVehicle);
		return false;
	}
}

void CCarCtrl::PickNextNodeRandomly(CVehicle* pVehicle)
{
	int32 prevNode = pVehicle->AutoPilot.m_nCurrentRouteNode;
	int32 curNode = pVehicle->AutoPilot.m_nNextRouteNode;
	uint8 totalLinks = ThePaths.m_pathNodes[curNode].numLinks;
	CCarPathLink* pCurLink = &ThePaths.m_carPathLinks[pVehicle->AutoPilot.m_nNextPathNodeInfo];
#ifdef FIX_BUGS
	uint8 lanesOnCurrentPath = pCurLink->pathNodeIndex == curNode ?
		pCurLink->numLeftLanes : pCurLink->numRightLanes;
#else
	uint8 lanesOnCurrentPath = pCurLink->pathNodeIndex == curNode ?
		pCurLink->numRightLanes : pCurLink->numLeftLanes;
#endif
	uint8 allowedDirections = PATH_DIRECTION_NONE;
	uint8 nextLane = pVehicle->AutoPilot.m_nNextLane;
	if (nextLane == 0)
		/* We are always allowed to turn left from  leftmost lane */
		allowedDirections |= PATH_DIRECTION_LEFT;
	if (nextLane == lanesOnCurrentPath - 1)
		/* We are always allowed to turn right from rightmost lane */
		allowedDirections |= PATH_DIRECTION_RIGHT;
	if (lanesOnCurrentPath < 3 || allowedDirections == PATH_DIRECTION_NONE)
		/* We are always allowed to go straight on one/two-laned road */
		/* or if we are in one of middle lanes of the road */
		allowedDirections |= PATH_DIRECTION_STRAIGHT;
	int attempt;
	pVehicle->AutoPilot.m_nPrevRouteNode = pVehicle->AutoPilot.m_nCurrentRouteNode;
	pVehicle->AutoPilot.m_nCurrentRouteNode = pVehicle->AutoPilot.m_nNextRouteNode;
	CPathNode* pPrevPathNode = &ThePaths.m_pathNodes[prevNode];
	CPathNode* pCurPathNode = &ThePaths.m_pathNodes[curNode];
	int16 nextLink;
	CCarPathLink* pNextLink;
	CPathNode* pNextPathNode;
	bool goingAgainstOneWayRoad;
	uint8 direction;
	for(attempt = 0; attempt < ATTEMPTS_TO_FIND_NEXT_NODE; attempt++){
		if (attempt != 0){
			if (pVehicle->AutoPilot.m_nNextRouteNode != prevNode){
				if (direction & allowedDirections){
					pNextPathNode = &ThePaths.m_pathNodes[pVehicle->AutoPilot.m_nNextRouteNode];
					if ((!pNextPathNode->bDeadEnd || pPrevPathNode->bDeadEnd) &&
						(!pNextPathNode->bDisabled || pPrevPathNode->bDisabled) &&
						(!pNextPathNode->bBetweenLevels || pPrevPathNode->bBetweenLevels || !pVehicle->AutoPilot.m_bStayInCurrentLevel) &&
						!goingAgainstOneWayRoad)
						break;
				}
			}
		}
		nextLink = CGeneral::GetRandomNumber() % totalLinks;
		pVehicle->AutoPilot.m_nNextRouteNode = ThePaths.ConnectedNode(nextLink + pCurPathNode->firstLink);
		direction = FindPathDirection(prevNode, curNode, pVehicle->AutoPilot.m_nNextRouteNode);
		pNextLink = &ThePaths.m_carPathLinks[ThePaths.m_carPathConnections[nextLink + pCurPathNode->firstLink]];
		goingAgainstOneWayRoad = pNextLink->pathNodeIndex == curNode ? pNextLink->numRightLanes == 0 : pNextLink->numLeftLanes == 0;
	}
	if (attempt >= ATTEMPTS_TO_FIND_NEXT_NODE) {
		/* If we failed 15 times, then remove dead end and current lane limitations */
		for (attempt = 0; attempt < ATTEMPTS_TO_FIND_NEXT_NODE; attempt++) {
			if (attempt != 0) {
				if (pVehicle->AutoPilot.m_nNextRouteNode != prevNode) {
					pNextPathNode = &ThePaths.m_pathNodes[pVehicle->AutoPilot.m_nNextRouteNode];
					if ((!pNextPathNode->bDisabled || pPrevPathNode->bDisabled) &&
						(!pNextPathNode->bBetweenLevels || pPrevPathNode->bBetweenLevels || !pVehicle->AutoPilot.m_bStayInCurrentLevel) &&
						!goingAgainstOneWayRoad)
						break;
				}
			}
			nextLink = CGeneral::GetRandomNumber() % totalLinks;
			pVehicle->AutoPilot.m_nNextRouteNode = ThePaths.ConnectedNode(nextLink + pCurPathNode->firstLink);
			pNextLink = &ThePaths.m_carPathLinks[ThePaths.m_carPathConnections[nextLink + pCurPathNode->firstLink]];
			goingAgainstOneWayRoad = pNextLink->pathNodeIndex == curNode ? pNextLink->numRightLanes == 0 : pNextLink->numLeftLanes == 0;
		}
	}
	if (attempt >= ATTEMPTS_TO_FIND_NEXT_NODE) {
		/* If we failed again, remove no U-turn limitation and remove randomness */
		for (nextLink = 0; nextLink < totalLinks; nextLink++) {
			pVehicle->AutoPilot.m_nNextRouteNode = ThePaths.ConnectedNode(nextLink + pCurPathNode->firstLink);
			pNextLink = &ThePaths.m_carPathLinks[ThePaths.m_carPathConnections[nextLink + pCurPathNode->firstLink]];
			goingAgainstOneWayRoad = pNextLink->pathNodeIndex == curNode ? pNextLink->numRightLanes == 0 : pNextLink->numLeftLanes == 0;
			if (!goingAgainstOneWayRoad) {
				pNextPathNode = &ThePaths.m_pathNodes[pVehicle->AutoPilot.m_nNextRouteNode];
				if ((!pNextPathNode->bDisabled || pPrevPathNode->bDisabled) &&
					(!pNextPathNode->bBetweenLevels || pPrevPathNode->bBetweenLevels || !pVehicle->AutoPilot.m_bStayInCurrentLevel))
					/* Nice way to exit loop but this will fail because this is used for indexing! */
					nextLink = 1000;
			}
		}
		if (nextLink < 999)
			/* If everything else failed, turn vehicle around */
			pVehicle->AutoPilot.m_nNextRouteNode = prevNode;
	}
	pNextPathNode = &ThePaths.m_pathNodes[pVehicle->AutoPilot.m_nNextRouteNode];
	pNextLink = &ThePaths.m_carPathLinks[ThePaths.m_carPathConnections[nextLink + pCurPathNode->firstLink]];
	if (prevNode == pVehicle->AutoPilot.m_nNextRouteNode){
		/* We can no longer shift vehicle without physics if we have to turn it around. */
		pVehicle->SetStatus(STATUS_PHYSICS);
		SwitchVehicleToRealPhysics(pVehicle);
	}
	pVehicle->AutoPilot.m_nTimeEnteredCurve += pVehicle->AutoPilot.m_nTimeToSpendOnCurrentCurve;
	pVehicle->AutoPilot.m_nPreviousPathNodeInfo = pVehicle->AutoPilot.m_nCurrentPathNodeInfo;
	pVehicle->AutoPilot.m_nCurrentPathNodeInfo = pVehicle->AutoPilot.m_nNextPathNodeInfo;
	pVehicle->AutoPilot.m_nPreviousDirection = pVehicle->AutoPilot.m_nCurrentDirection;
	pVehicle->AutoPilot.m_nCurrentDirection = pVehicle->AutoPilot.m_nNextDirection;
	pVehicle->AutoPilot.m_nCurrentLane = pVehicle->AutoPilot.m_nNextLane;
	pVehicle->AutoPilot.m_nNextPathNodeInfo = ThePaths.m_carPathConnections[nextLink + pCurPathNode->firstLink];
	int8 lanesOnNextNode;
	if (curNode >= pVehicle->AutoPilot.m_nNextRouteNode){
		pVehicle->AutoPilot.m_nNextDirection = 1;
		lanesOnNextNode = pNextLink->numLeftLanes;
	}else{
		pVehicle->AutoPilot.m_nNextDirection = -1;
		lanesOnNextNode = pNextLink->numRightLanes;
	}
	float currentPathLinkForwardX = pVehicle->AutoPilot.m_nCurrentDirection * pCurLink->GetDirX();
	float nextPathLinkForwardX = pVehicle->AutoPilot.m_nNextDirection * pNextLink->GetDirX();
#ifdef FIX_BUGS
	float currentPathLinkForwardY = pVehicle->AutoPilot.m_nCurrentDirection * pCurLink->GetDirY();
	float nextPathLinkForwardY = pVehicle->AutoPilot.m_nNextDirection * pNextLink->GetDirY();
#endif
	if (lanesOnNextNode >= 0){
		if ((CGeneral::GetRandomNumber() & 0x600) == 0){
			/* 25% chance vehicle will try to switch lane */
			CVector2D dist = pNextPathNode->GetPosition() - pCurPathNode->GetPosition();
			if (dist.MagnitudeSqr() >= SQR(14.0f)){
				if (CGeneral::GetRandomTrueFalse())
					pVehicle->AutoPilot.m_nNextLane += 1;
				else
					pVehicle->AutoPilot.m_nNextLane -= 1;
			}
		}
		pVehicle->AutoPilot.m_nNextLane = Min(lanesOnNextNode - 1, pVehicle->AutoPilot.m_nNextLane);
		pVehicle->AutoPilot.m_nNextLane = Max(0, pVehicle->AutoPilot.m_nNextLane);
	}else{
		pVehicle->AutoPilot.m_nNextLane = pVehicle->AutoPilot.m_nCurrentLane;
	}
	if (pVehicle->AutoPilot.m_bStayInFastLane)
		pVehicle->AutoPilot.m_nNextLane = 0;
	CVector positionOnCurrentLinkIncludingLane(
		pCurLink->GetX() + ((pVehicle->AutoPilot.m_nCurrentLane + pCurLink->OneWayLaneOffset()) * LANE_WIDTH)
#ifdef FIX_BUGS
		* currentPathLinkForwardY
#endif
		,pCurLink->GetY() - ((pVehicle->AutoPilot.m_nCurrentLane + pCurLink->OneWayLaneOffset()) * LANE_WIDTH) * currentPathLinkForwardX,
		0.0f);
	CVector positionOnNextLinkIncludingLane(
		pNextLink->GetX() + ((pVehicle->AutoPilot.m_nNextLane + pNextLink->OneWayLaneOffset()) * LANE_WIDTH)
#ifdef FIX_BUGS
		* nextPathLinkForwardY
#endif
		,pNextLink->GetY() - ((pVehicle->AutoPilot.m_nNextLane + pNextLink->OneWayLaneOffset()) * LANE_WIDTH) * nextPathLinkForwardX,
		0.0f);
	float directionCurrentLinkX = pCurLink->GetDirX() * pVehicle->AutoPilot.m_nCurrentDirection;
	float directionCurrentLinkY = pCurLink->GetDirY() * pVehicle->AutoPilot.m_nCurrentDirection;
	float directionNextLinkX = pNextLink->GetDirX() * pVehicle->AutoPilot.m_nNextDirection;
	float directionNextLinkY = pNextLink->GetDirY() * pVehicle->AutoPilot.m_nNextDirection;
	/* We want to make a path between two links that may not have the same forward directions a curve. */
	pVehicle->AutoPilot.m_nTimeToSpendOnCurrentCurve = CCurves::CalcSpeedScaleFactor(
		&positionOnCurrentLinkIncludingLane,
		&positionOnNextLinkIncludingLane,
		directionCurrentLinkX, directionCurrentLinkY,
		directionNextLinkX, directionNextLinkY
	) * (1000.0f / pVehicle->AutoPilot.m_fMaxTrafficSpeed);
	if (pVehicle->AutoPilot.m_nTimeToSpendOnCurrentCurve < 10)
		/* Oh hey there Obbe */
		printf("fout\n");
	pVehicle->AutoPilot.m_nTimeToSpendOnCurrentCurve = Max(10, pVehicle->AutoPilot.m_nTimeToSpendOnCurrentCurve);
}

uint8 CCarCtrl::FindPathDirection(int32 prevNode, int32 curNode, int32 nextNode)
{
	CVector2D prevToCur = ThePaths.m_pathNodes[curNode].GetPosition() - ThePaths.m_pathNodes[prevNode].GetPosition();
	CVector2D curToNext = ThePaths.m_pathNodes[nextNode].GetPosition() - ThePaths.m_pathNodes[curNode].GetPosition();
	float distPrevToCur = prevToCur.Magnitude();
	if (distPrevToCur == 0.0f)
		return PATH_DIRECTION_NONE;
	/* We are trying to determine angle between prevToCur and curToNext. */
	/* To find it, we consider a to be an angle between y axis and prevToCur */
	/* and b to be an angle between x axis and curToNext */
	/* Then the angle we are looking for is (pi/2 + a + b). */
	float sin_a = prevToCur.x / distPrevToCur;
	float cos_a = prevToCur.y / distPrevToCur;
	float distCurToNext = curToNext.Magnitude();
	if (distCurToNext == 0.0f)
		return PATH_DIRECTION_NONE;
	float sin_b = curToNext.y / distCurToNext;
	float cos_b = curToNext.x / distCurToNext;
	/* sin(a) * sin(b) - cos(a) * cos(b) = -cos(a+b) = sin(pi/2+a+b) */
	float sin_direction = sin_a * sin_b - cos_a * cos_b;
	if (sin_direction > 0.77f) /* Roughly between -50 and -130 degrees */
		return PATH_DIRECTION_LEFT;
	if (sin_direction < -0.77f) /* Roughly between 50 and 130 degrees */
		return PATH_DIRECTION_RIGHT;
	return PATH_DIRECTION_STRAIGHT;
}

#ifdef FIX_PATHFIND_BUG
void CCarCtrl::PickNextNodeToChaseCar(CVehicle* pVehicle, float targetX, float targetY, float targetZ, CVehicle* pTarget)
#else
void CCarCtrl::PickNextNodeToChaseCar(CVehicle* pVehicle, float targetX, float targetY, CVehicle* pTarget)
#endif
{
	int prevNode = pVehicle->AutoPilot.m_nCurrentRouteNode;
	int curNode = pVehicle->AutoPilot.m_nNextRouteNode;
	CPathNode* pPrevNode = &ThePaths.m_pathNodes[prevNode];
	CPathNode* pCurNode = &ThePaths.m_pathNodes[curNode];
	CPathNode* pTargetNode;
	int16 numNodes;
	float distanceToTargetNode;
	if (pTarget && pTarget->m_pCurGroundEntity &&
	  pTarget->m_pCurGroundEntity->IsBuilding() &&
	  ((CBuilding*)pTarget->m_pCurGroundEntity)->GetIsATreadable() &&
	  ((CTreadable*)pTarget->m_pCurGroundEntity)->m_nodeIndices[0][0] >= 0){
		CTreadable* pCurrentMapObject = (CTreadable*)pTarget->m_pCurGroundEntity;
		int closestNode = -1;
		float minDist = 100000.0f;
		for (int i = 0; i < 12; i++){
			int node = pCurrentMapObject->m_nodeIndices[0][i];
			if (node < 0)
				break;
			float dist = (ThePaths.m_pathNodes[node].GetPosition() - pTarget->GetPosition()).Magnitude();
			if (dist < minDist){
				minDist = dist;
				closestNode = node;
			}
		}
		ThePaths.DoPathSearch(0, pCurNode->GetPosition(), curNode,
#ifdef FIX_PATHFIND_BUG
			CVector(targetX, targetY, targetZ),
#else
			CVector(targetX, targetY, 0.0f),
#endif
			&pTargetNode, &numNodes, 1, pVehicle, &distanceToTargetNode, 999999.9f, closestNode);
	}else
	{

		ThePaths.DoPathSearch(0, pCurNode->GetPosition(), curNode,
#ifdef FIX_PATHFIND_BUG
			CVector(targetX, targetY, targetZ),
#else
			CVector(targetX, targetY, 0.0f),
#endif
			&pTargetNode, &numNodes, 1, pVehicle, &distanceToTargetNode, 999999.9f, -1);
	}

	int newNextNode;
	int nextLink;
	if (numNodes != 1 || pTargetNode == pCurNode){
		float currentAngle = CGeneral::GetATanOfXY(targetX - pVehicle->GetPosition().x, targetY - pVehicle->GetPosition().y);
		nextLink = 0;
		float lowestAngleChange = 10.0f;
		int numLinks = pCurNode->numLinks;
		newNextNode = 0;
		for (int i = 0; i < numLinks; i++){
			int conNode = ThePaths.ConnectedNode(i + pCurNode->firstLink);
			if (conNode == prevNode && i > 1)
				continue;
			CPathNode* pTestNode = &ThePaths.m_pathNodes[conNode];
			float angle = CGeneral::GetATanOfXY(pTestNode->GetX() - pCurNode->GetX(), pTestNode->GetY() - pCurNode->GetY());
			angle = LimitRadianAngle(angle - currentAngle);
			angle = ABS(angle);
			if (angle < lowestAngleChange){
				lowestAngleChange = angle;
				newNextNode = conNode;
				nextLink = i;
			}
		}
	}else{
		nextLink = 0;
		newNextNode = pTargetNode - ThePaths.m_pathNodes;
		for (int i = pCurNode->firstLink; ThePaths.ConnectedNode(i) != newNextNode; i++, nextLink++)
			;
	}
	CPathNode* pNextPathNode = &ThePaths.m_pathNodes[pVehicle->AutoPilot.m_nNextRouteNode];
	CCarPathLink* pNextLink = &ThePaths.m_carPathLinks[ThePaths.m_carPathConnections[nextLink + pCurNode->firstLink]];
	CCarPathLink* pCurLink = &ThePaths.m_carPathLinks[pVehicle->AutoPilot.m_nNextPathNodeInfo];
	pVehicle->AutoPilot.m_nPrevRouteNode = pVehicle->AutoPilot.m_nCurrentRouteNode;
	pVehicle->AutoPilot.m_nCurrentRouteNode = pVehicle->AutoPilot.m_nNextRouteNode;
	pVehicle->AutoPilot.m_nNextRouteNode = newNextNode;
	pVehicle->AutoPilot.m_nTimeEnteredCurve += pVehicle->AutoPilot.m_nTimeToSpendOnCurrentCurve;
	pVehicle->AutoPilot.m_nPreviousPathNodeInfo = pVehicle->AutoPilot.m_nCurrentPathNodeInfo;
	pVehicle->AutoPilot.m_nCurrentPathNodeInfo = pVehicle->AutoPilot.m_nNextPathNodeInfo;
	pVehicle->AutoPilot.m_nPreviousDirection = pVehicle->AutoPilot.m_nCurrentDirection;
	pVehicle->AutoPilot.m_nCurrentDirection = pVehicle->AutoPilot.m_nNextDirection;
	pVehicle->AutoPilot.m_nCurrentLane = pVehicle->AutoPilot.m_nNextLane;
	pVehicle->AutoPilot.m_nNextPathNodeInfo = ThePaths.m_carPathConnections[nextLink + pCurNode->firstLink];
	int8 lanesOnNextNode;
	if (curNode >= pVehicle->AutoPilot.m_nNextRouteNode) {
		pVehicle->AutoPilot.m_nNextDirection = 1;
		lanesOnNextNode = pNextLink->numLeftLanes;
	}
	else {
		pVehicle->AutoPilot.m_nNextDirection = -1;
		lanesOnNextNode = pNextLink->numRightLanes;
	}
	float currentPathLinkForwardX = pVehicle->AutoPilot.m_nCurrentDirection * pCurLink->GetDirX();
	float currentPathLinkForwardY = pVehicle->AutoPilot.m_nCurrentDirection * pCurLink->GetDirY();
	float nextPathLinkForwardX = pVehicle->AutoPilot.m_nNextDirection * pNextLink->GetDirX();
	float nextPathLinkForwardY = pVehicle->AutoPilot.m_nNextDirection * pNextLink->GetDirY();
	if (lanesOnNextNode >= 0) {
		CVector2D dist = pNextPathNode->GetPosition() - pCurNode->GetPosition();
		if (dist.MagnitudeSqr() >= SQR(7.0f)){
			/* 25% chance vehicle will try to switch lane */
			/* No lane switching if following car from far away */
			/* ...although it's always one of those. */
			if ((CGeneral::GetRandomNumber() & 0x600) == 0 &&
			  pVehicle->AutoPilot.m_nCarMission != MISSION_RAMPLAYER_FARAWAY &&
			  pVehicle->AutoPilot.m_nCarMission != MISSION_BLOCKPLAYER_FARAWAY &&
			  pVehicle->AutoPilot.m_nCarMission != MISSION_RAMCAR_FARAWAY &&
			  pVehicle->AutoPilot.m_nCarMission != MISSION_BLOCKCAR_FARAWAY){
				if (CGeneral::GetRandomTrueFalse())
					pVehicle->AutoPilot.m_nNextLane += 1;
				else
					pVehicle->AutoPilot.m_nNextLane -= 1;
			}
		}
		pVehicle->AutoPilot.m_nNextLane = Min(lanesOnNextNode - 1, pVehicle->AutoPilot.m_nNextLane);
		pVehicle->AutoPilot.m_nNextLane = Max(0, pVehicle->AutoPilot.m_nNextLane);
	}
	else {
		pVehicle->AutoPilot.m_nNextLane = pVehicle->AutoPilot.m_nCurrentLane;
	}
	if (pVehicle->AutoPilot.m_bStayInFastLane)
		pVehicle->AutoPilot.m_nNextLane = 0;
	CVector positionOnCurrentLinkIncludingLane(
		pCurLink->GetX() + ((pVehicle->AutoPilot.m_nCurrentLane + pCurLink->OneWayLaneOffset()) * LANE_WIDTH) * currentPathLinkForwardY,
		pCurLink->GetY() - ((pVehicle->AutoPilot.m_nCurrentLane + pCurLink->OneWayLaneOffset()) * LANE_WIDTH) * currentPathLinkForwardX,
		0.0f);
	CVector positionOnNextLinkIncludingLane(
		pNextLink->GetX() + ((pVehicle->AutoPilot.m_nNextLane + pNextLink->OneWayLaneOffset()) * LANE_WIDTH) * nextPathLinkForwardY,
		pNextLink->GetY() - ((pVehicle->AutoPilot.m_nNextLane + pNextLink->OneWayLaneOffset()) * LANE_WIDTH) * nextPathLinkForwardX,
		0.0f);
	float directionCurrentLinkX = pCurLink->GetDirX() * pVehicle->AutoPilot.m_nCurrentDirection;
	float directionCurrentLinkY = pCurLink->GetDirY() * pVehicle->AutoPilot.m_nCurrentDirection;
	float directionNextLinkX = pNextLink->GetDirX() * pVehicle->AutoPilot.m_nNextDirection;
	float directionNextLinkY = pNextLink->GetDirY() * pVehicle->AutoPilot.m_nNextDirection;
	/* We want to make a path between two links that may not have the same forward directions a curve. */
	pVehicle->AutoPilot.m_nTimeToSpendOnCurrentCurve = CCurves::CalcSpeedScaleFactor(
		&positionOnCurrentLinkIncludingLane,
		&positionOnNextLinkIncludingLane,
		directionCurrentLinkX, directionCurrentLinkY,
		directionNextLinkX, directionNextLinkY
	) * (1000.0f / pVehicle->AutoPilot.m_fMaxTrafficSpeed);
	pVehicle->AutoPilot.m_nTimeToSpendOnCurrentCurve = Max(10, pVehicle->AutoPilot.m_nTimeToSpendOnCurrentCurve);
}

bool CCarCtrl::PickNextNodeToFollowPath(CVehicle* pVehicle)
{
	int curNode = pVehicle->AutoPilot.m_nNextRouteNode;
	CPathNode* pCurNode = &ThePaths.m_pathNodes[curNode];
	if (pVehicle->AutoPilot.m_nPathFindNodesCount == 0){
		ThePaths.DoPathSearch(0, pVehicle->GetPosition(), curNode,
			pVehicle->AutoPilot.m_vecDestinationCoors, pVehicle->AutoPilot.m_aPathFindNodesInfo,
			&pVehicle->AutoPilot.m_nPathFindNodesCount, NUM_PATH_NODES_IN_AUTOPILOT,
			pVehicle, nil, 999999.9f, -1);
		if (pVehicle->AutoPilot.m_nPathFindNodesCount < 1)
			return true;
	}
	CPathNode* pNextPathNode = &ThePaths.m_pathNodes[pVehicle->AutoPilot.m_nNextRouteNode];
	CCarPathLink* pCurLink = &ThePaths.m_carPathLinks[pVehicle->AutoPilot.m_nNextPathNodeInfo];
	pVehicle->AutoPilot.m_nPrevRouteNode = pVehicle->AutoPilot.m_nCurrentRouteNode;
	pVehicle->AutoPilot.m_nCurrentRouteNode = pVehicle->AutoPilot.m_nNextRouteNode;
	pVehicle->AutoPilot.m_nNextRouteNode = pVehicle->AutoPilot.m_aPathFindNodesInfo[0] - ThePaths.m_pathNodes;
	pVehicle->AutoPilot.RemoveOnePathNode();
	pVehicle->AutoPilot.m_nTimeEnteredCurve += pVehicle->AutoPilot.m_nTimeToSpendOnCurrentCurve;
	pVehicle->AutoPilot.m_nPreviousPathNodeInfo = pVehicle->AutoPilot.m_nCurrentPathNodeInfo;
	pVehicle->AutoPilot.m_nCurrentPathNodeInfo = pVehicle->AutoPilot.m_nNextPathNodeInfo;
	pVehicle->AutoPilot.m_nPreviousDirection = pVehicle->AutoPilot.m_nCurrentDirection;
	pVehicle->AutoPilot.m_nCurrentDirection = pVehicle->AutoPilot.m_nNextDirection;
	pVehicle->AutoPilot.m_nCurrentLane = pVehicle->AutoPilot.m_nNextLane;
	int nextLink = 0;
	for (int i = pCurNode->firstLink; ThePaths.ConnectedNode(i) != pVehicle->AutoPilot.m_nNextRouteNode; i++, nextLink++)
		;
	CCarPathLink* pNextLink = &ThePaths.m_carPathLinks[ThePaths.m_carPathConnections[nextLink + pCurNode->firstLink]];
	pVehicle->AutoPilot.m_nNextPathNodeInfo = ThePaths.m_carPathConnections[nextLink + pCurNode->firstLink];
	int8 lanesOnNextNode;
	if (curNode >= pVehicle->AutoPilot.m_nNextRouteNode) {
		pVehicle->AutoPilot.m_nNextDirection = 1;
		lanesOnNextNode = pNextLink->numLeftLanes;
	}
	else {
		pVehicle->AutoPilot.m_nNextDirection = -1;
		lanesOnNextNode = pNextLink->numRightLanes;
	}
	float currentPathLinkForwardX = pVehicle->AutoPilot.m_nCurrentDirection * pCurLink->GetDirX();
	float currentPathLinkForwardY = pVehicle->AutoPilot.m_nCurrentDirection * pCurLink->GetDirY();
	float nextPathLinkForwardX = pVehicle->AutoPilot.m_nNextDirection * pNextLink->GetDirX();
	float nextPathLinkForwardY = pVehicle->AutoPilot.m_nNextDirection * pNextLink->GetDirY();
	if (lanesOnNextNode >= 0) {
		CVector2D dist = pNextPathNode->GetPosition() - pCurNode->GetPosition();
		if (dist.MagnitudeSqr() >= SQR(7.0f) && (CGeneral::GetRandomNumber() & 0x600) == 0) {
			if (CGeneral::GetRandomTrueFalse())
				pVehicle->AutoPilot.m_nNextLane += 1;
			else
				pVehicle->AutoPilot.m_nNextLane -= 1;
		}
		pVehicle->AutoPilot.m_nNextLane = Min(lanesOnNextNode - 1, pVehicle->AutoPilot.m_nNextLane);
		pVehicle->AutoPilot.m_nNextLane = Max(0, pVehicle->AutoPilot.m_nNextLane);
	}
	else {
		pVehicle->AutoPilot.m_nNextLane = pVehicle->AutoPilot.m_nCurrentLane;
	}
	if (pVehicle->AutoPilot.m_bStayInFastLane)
		pVehicle->AutoPilot.m_nNextLane = 0;
	CVector positionOnCurrentLinkIncludingLane(
		pCurLink->GetX() + ((pVehicle->AutoPilot.m_nCurrentLane + pCurLink->OneWayLaneOffset()) * LANE_WIDTH) * currentPathLinkForwardY,
		pCurLink->GetY() - ((pVehicle->AutoPilot.m_nCurrentLane + pCurLink->OneWayLaneOffset()) * LANE_WIDTH) * currentPathLinkForwardX,
		0.0f);
	CVector positionOnNextLinkIncludingLane(
		pNextLink->GetX() + ((pVehicle->AutoPilot.m_nNextLane + pNextLink->OneWayLaneOffset()) * LANE_WIDTH) * nextPathLinkForwardY,
		pNextLink->GetY() - ((pVehicle->AutoPilot.m_nNextLane + pNextLink->OneWayLaneOffset()) * LANE_WIDTH) * nextPathLinkForwardX,
		0.0f);
	float directionCurrentLinkX = pCurLink->GetDirX() * pVehicle->AutoPilot.m_nCurrentDirection;
	float directionCurrentLinkY = pCurLink->GetDirY() * pVehicle->AutoPilot.m_nCurrentDirection;
	float directionNextLinkX = pNextLink->GetDirX() * pVehicle->AutoPilot.m_nNextDirection;
	float directionNextLinkY = pNextLink->GetDirY() * pVehicle->AutoPilot.m_nNextDirection;
	/* We want to make a path between two links that may not have the same forward directions a curve. */
	pVehicle->AutoPilot.m_nTimeToSpendOnCurrentCurve = CCurves::CalcSpeedScaleFactor(
		&positionOnCurrentLinkIncludingLane,
		&positionOnNextLinkIncludingLane,
		directionCurrentLinkX, directionCurrentLinkY,
		directionNextLinkX, directionNextLinkY
	) * (1000.0f / pVehicle->AutoPilot.m_fMaxTrafficSpeed);
	pVehicle->AutoPilot.m_nTimeToSpendOnCurrentCurve = Max(10, pVehicle->AutoPilot.m_nTimeToSpendOnCurrentCurve);
	return false;
}

void CCarCtrl::Init(void)
{
	NumRandomCars = 0;
	NumLawEnforcerCars = 0;
	NumMissionCars = 0;
	NumParkedCars = 0;
	NumPermanentCars = 0;
	NumAmbulancesOnDuty = 0;
	NumFiretrucksOnDuty = 0;
	LastTimeFireTruckCreated = 0;
	LastTimeAmbulanceCreated = 0;
#ifdef FIX_BUGS
	LastTimeLawEnforcerCreated = 0;
#endif
	bCarsGeneratedAroundCamera = false;
	CountDownToCarsAtStart = 2;
	CarDensityMultiplier = 1.0f;
	for (int i = 0; i < MAX_CARS_TO_KEEP; i++)
		apCarsToKeep[i] = nil;
	for (int i = 0; i < TOTAL_CUSTOM_CLASSES; i++){
		for (int j = 0; j < MAX_CAR_MODELS_IN_ARRAY; j++)
			CarArrays[i][j] = 0;
		NextCarOfRating[i] = 0;
		TotalNumOfCarsOfRating[i] = 0;
	}
}

void CCarCtrl::ReInit(void)
{
	NumRandomCars = 0;
	NumLawEnforcerCars = 0;
	NumMissionCars = 0;
	NumParkedCars = 0;
	NumPermanentCars = 0;
	NumAmbulancesOnDuty = 0;
	NumFiretrucksOnDuty = 0;
#ifdef FIX_BUGS
	LastTimeFireTruckCreated = 0;
	LastTimeAmbulanceCreated = 0;
	LastTimeLawEnforcerCreated = 0;
#endif
	CountDownToCarsAtStart = 2;
	CarDensityMultiplier = 1.0f;
	for (int i = 0; i < MAX_CARS_TO_KEEP; i++)
		apCarsToKeep[i] = nil;
	for (int i = 0; i < TOTAL_CUSTOM_CLASSES; i++)
		NextCarOfRating[i] = 0;
}

void CCarCtrl::DragCarToPoint(CVehicle* pVehicle, CVector* pPoint)
{
	CVector2D posBehind = (CVector2D)pVehicle->GetPosition() - 3 * pVehicle->GetForward() / 2;
	CVector2D posTarget = *pPoint;
	CVector2D direction = posBehind - posTarget;
	CVector2D midPos = posTarget + direction * 3 / direction.Magnitude();
	float actualAheadZ;
	float actualBehindZ;
	CColPoint point;
	CEntity* pRoadObject;
	if (CCollision::IsStoredPolyStillValidVerticalLine(CVector(posTarget.x, posTarget.y, pVehicle->GetPosition().z - 3.0f),
	  pVehicle->GetPosition().z - 3.0f, point, &pVehicle->m_aCollPolys[0])){
		actualAheadZ = point.point.z;
	}else if (CWorld::ProcessVerticalLine(CVector(posTarget.x, posTarget.y, pVehicle->GetPosition().z + 1.5f),
	  pVehicle->GetPosition().z - 2.0f, point,
	  pRoadObject, true, false, false, false, false, false, &pVehicle->m_aCollPolys[0])){
		actualAheadZ = point.point.z;
		pVehicle->m_pCurGroundEntity = pRoadObject;
		if (ThisRoadObjectCouldMove(pRoadObject->GetModelIndex()))
			pVehicle->m_aCollPolys[0].valid = false;
	}else if (CWorld::ProcessVerticalLine(CVector(posTarget.x, posTarget.y, pVehicle->GetPosition().z + 3.0f),
	  pVehicle->GetPosition().z - 3.0f, point,
	  pRoadObject, true, false, false, false, false, false, &pVehicle->m_aCollPolys[0])) {
		actualAheadZ = point.point.z;
		pVehicle->m_pCurGroundEntity = pRoadObject;
		if (ThisRoadObjectCouldMove(pRoadObject->GetModelIndex()))
			pVehicle->m_aCollPolys[0].valid = false;
	}else{
		actualAheadZ = pVehicle->m_fMapObjectHeightAhead;
	}
	pVehicle->m_fMapObjectHeightAhead = actualAheadZ;
	if (CCollision::IsStoredPolyStillValidVerticalLine(CVector(midPos.x, midPos.y, pVehicle->GetPosition().z - 3.0f),
	  pVehicle->GetPosition().z - 3.0f, point, &pVehicle->m_aCollPolys[1])){
		actualBehindZ = point.point.z;
	}else if (CWorld::ProcessVerticalLine(CVector(midPos.x, midPos.y, pVehicle->GetPosition().z + 1.5f),
	  pVehicle->GetPosition().z - 2.0f, point,
	  pRoadObject, true, false, false, false, false, false, &pVehicle->m_aCollPolys[1])){
		actualBehindZ = point.point.z;
		pVehicle->m_pCurGroundEntity = pRoadObject;
		if (ThisRoadObjectCouldMove(pRoadObject->GetModelIndex()))
			pVehicle->m_aCollPolys[1].valid = false;
	}else if (CWorld::ProcessVerticalLine(CVector(midPos.x, midPos.y, pVehicle->GetPosition().z + 3.0f),
	  pVehicle->GetPosition().z - 3.0f, point,
	  pRoadObject, true, false, false, false, false, false, &pVehicle->m_aCollPolys[1])){
		actualBehindZ = point.point.z;
		pVehicle->m_pCurGroundEntity = pRoadObject;
		if (ThisRoadObjectCouldMove(pRoadObject->GetModelIndex()))
			pVehicle->m_aCollPolys[1].valid = false;
	}else{
		actualBehindZ = pVehicle->m_fMapObjectHeightBehind;
	}
	pVehicle->m_fMapObjectHeightBehind = actualBehindZ;
	float angleZ = Atan2((actualAheadZ - actualBehindZ) / 3, 1.0f);
	float cosZ = Cos(angleZ);
	float sinZ = Sin(angleZ);
	pVehicle->GetRight() = CVector(posTarget.y - midPos.y, -(posTarget.x - midPos.x), 0.0f) / 3;
	pVehicle->GetForward() = CVector(-cosZ * pVehicle->GetRight().y, cosZ * pVehicle->GetRight().x, sinZ);
	pVehicle->GetUp() = CrossProduct(pVehicle->GetRight(), pVehicle->GetForward());
	pVehicle->SetPosition((CVector(midPos.x, midPos.y, actualBehindZ) + CVector(posTarget.x, posTarget.y, actualAheadZ)) / 2);
	pVehicle->GetMatrix().GetPosition().z += pVehicle->GetHeightAboveRoad();
}

float CCarCtrl::FindSpeedMultiplier(float angleChange, float minAngle, float maxAngle, float coef)
{
	float angle = Abs(LimitRadianAngle(angleChange));
	float n = angle - minAngle;
	n = Max(0.0f, n);
	float d = maxAngle - minAngle;
	float mult = 1.0f - n / d * (1.0f - coef);
	if (n > d)
		return coef;
	return mult;
}

void CCarCtrl::SteerAICarWithPhysics(CVehicle* pVehicle)
{
	float swerve;
	float accel;
	float brake;
	bool handbrake;
	switch (pVehicle->AutoPilot.m_nTempAction){
	case TEMPACT_WAIT:
		swerve = 0.0f;
		accel = 0.0f;
		brake = 0.2f;
		handbrake = false;
		if (CTimer::GetTimeInMilliseconds() > pVehicle->AutoPilot.m_nTimeTempAction){
			pVehicle->AutoPilot.m_nTempAction = TEMPACT_NONE;
			pVehicle->AutoPilot.m_nAntiReverseTimer = CTimer::GetTimeInMilliseconds();
			pVehicle->AutoPilot.m_nTimeTempAction = CTimer::GetTimeInMilliseconds();
		}
		break;
	case TEMPACT_REVERSE:
		SteerAICarWithPhysics_OnlyMission(pVehicle, &swerve, &accel, &brake, &handbrake);
		handbrake = false;
		swerve = -swerve;
		if (DotProduct(pVehicle->GetMoveSpeed(), pVehicle->GetForward()) > 0.04f){
			accel = 0.0f;
			brake = 0.5f;
		}else{
			accel = -0.5f;
			brake = 0.0f;
		}
		if (CTimer::GetTimeInMilliseconds() > pVehicle->AutoPilot.m_nTimeTempAction)
			pVehicle->AutoPilot.m_nTempAction = TEMPACT_NONE;
		break;
	case TEMPACT_HANDBRAKETURNLEFT:
		swerve = -1.0f; // It seems like this should be swerve = 1.0f (fixed in VC)
		accel = 0.0f;
		brake = 0.0f;
		handbrake = true;
		if (CTimer::GetTimeInMilliseconds() > pVehicle->AutoPilot.m_nTimeTempAction)
			pVehicle->AutoPilot.m_nTempAction = TEMPACT_NONE;
		break;
	case TEMPACT_HANDBRAKETURNRIGHT:
		swerve = 1.0f; // It seems like this should be swerve = -1.0f (fixed in VC)
		accel = 0.0f;
		brake = 0.0f;
		handbrake = true;
		if (CTimer::GetTimeInMilliseconds() > pVehicle->AutoPilot.m_nTimeTempAction)
			pVehicle->AutoPilot.m_nTempAction = TEMPACT_NONE;
		break;
	case TEMPACT_HANDBRAKESTRAIGHT:
		swerve = 0.0f;
		accel = 0.0f;
		brake = 0.0f;
		handbrake = true;
		if (CTimer::GetTimeInMilliseconds() > pVehicle->AutoPilot.m_nTimeTempAction)
			pVehicle->AutoPilot.m_nTempAction = TEMPACT_NONE;
		break;
	case TEMPACT_TURNLEFT:
		swerve = 1.0f;
		accel = 1.0f;
		brake = 0.0f;
		handbrake = false;
		if (CTimer::GetTimeInMilliseconds() > pVehicle->AutoPilot.m_nTimeTempAction)
			pVehicle->AutoPilot.m_nTempAction = TEMPACT_NONE;
		break;
	case TEMPACT_TURNRIGHT:
		swerve = -1.0f;
		accel = 1.0f;
		brake = 0.0f;
		handbrake = false;
		if (CTimer::GetTimeInMilliseconds() > pVehicle->AutoPilot.m_nTimeTempAction)
			pVehicle->AutoPilot.m_nTempAction = TEMPACT_NONE;
		break;
	case TEMPACT_GOFORWARD:
		swerve = 0.0f;
		accel = 0.5f;
		brake = 0.0f;
		handbrake = false;
		if (CTimer::GetTimeInMilliseconds() > pVehicle->AutoPilot.m_nTimeTempAction)
			pVehicle->AutoPilot.m_nTempAction = TEMPACT_NONE;
		break;
	case TEMPACT_SWERVELEFT:
	case TEMPACT_SWERVERIGHT:
		swerve = (pVehicle->AutoPilot.m_nTempAction == TEMPACT_SWERVERIGHT) ? 0.15f : -0.15f;
		accel = 0.0f;
		brake = 0.001f;
		handbrake = false;
		if (CTimer::GetTimeInMilliseconds() > pVehicle->AutoPilot.m_nTimeTempAction - 1000)
			swerve = -swerve;
		if (CTimer::GetTimeInMilliseconds() > pVehicle->AutoPilot.m_nTimeTempAction)
			pVehicle->AutoPilot.m_nTempAction = TEMPACT_NONE;
		break;
	default:
		SteerAICarWithPhysics_OnlyMission(pVehicle, &swerve, &accel, &brake, &handbrake);
		break;
	}
	pVehicle->m_fSteerAngle = swerve;
	pVehicle->bIsHandbrakeOn = handbrake;
	pVehicle->m_fGasPedal = accel;
	pVehicle->m_fBrakePedal = brake;
}

void CCarCtrl::SteerAICarWithPhysics_OnlyMission(CVehicle* pVehicle, float* pSwerve, float* pAccel, float* pBrake, bool* pHandbrake)
{
	switch (pVehicle->AutoPilot.m_nCarMission) {
	case MISSION_NONE:
		*pSwerve = 0.0f;
		*pAccel = 0.0f;
		*pBrake = 0.5f;
		*pHandbrake = true;
		return;
	case MISSION_CRUISE:
	case MISSION_RAMPLAYER_FARAWAY:
	case MISSION_BLOCKPLAYER_FARAWAY:
	case MISSION_GOTOCOORDS:
	case MISSION_GOTOCOORDS_ACCURATE:
	case MISSION_RAMCAR_FARAWAY:
	case MISSION_BLOCKCAR_FARAWAY:
		SteerAICarWithPhysicsFollowPath(pVehicle, pSwerve, pAccel, pBrake, pHandbrake);
		return;
	case MISSION_RAMPLAYER_CLOSE:
	{
		CVector2D targetPos = FindPlayerCoors();
		if (FindPlayerVehicle()){
			if (pVehicle->m_randomSeed & 1 && DotProduct(FindPlayerVehicle()->GetForward(), pVehicle->GetForward()) > 0.5f){
				float targetWidth = FindPlayerVehicle()->GetColModel()->boundingBox.max.x;
				float ownWidth = pVehicle->GetColModel()->boundingBox.max.x;
				if (pVehicle->m_randomSeed & 2){
					targetPos += (targetWidth + ownWidth - 0.2f) * FindPlayerVehicle()->GetRight();
				}else{
					targetPos -= (targetWidth + ownWidth - 0.2f) * FindPlayerVehicle()->GetRight();
				}
				float targetSpeed = FindPlayerVehicle()->GetMoveSpeed().Magnitude();
				float distanceToTarget = ((CVector2D)pVehicle->GetPosition() - targetPos).Magnitude();
				if (12.0f * targetSpeed + 2.0f > distanceToTarget && pVehicle->AutoPilot.m_nTempAction == TEMPACT_NONE){
					pVehicle->AutoPilot.m_nTempAction = (pVehicle->m_randomSeed & 2) ? TEMPACT_TURNLEFT : TEMPACT_TURNRIGHT;
					pVehicle->AutoPilot.m_nTimeTempAction = CTimer::GetTimeInMilliseconds() + 250;
				}
			}else{
				targetPos += FindPlayerVehicle()->GetRight() / 160 * ((pVehicle->m_randomSeed & 0xFF) - 128);
			}
		}
		SteerAICarWithPhysicsHeadingForTarget(pVehicle, FindPlayerVehicle(), targetPos.x, targetPos.y, pSwerve, pAccel, pBrake, pHandbrake);
		return;
	}
	case MISSION_BLOCKPLAYER_CLOSE:
		SteerAICarWithPhysicsTryingToBlockTarget(pVehicle, FindPlayerCoors().x, FindPlayerCoors().y,
			FindPlayerSpeed().x, FindPlayerSpeed().y, pSwerve, pAccel, pBrake, pHandbrake);
		return;
	case MISSION_BLOCKPLAYER_HANDBRAKESTOP:
		SteerAICarWithPhysicsTryingToBlockTarget_Stop(pVehicle, FindPlayerCoors().x, FindPlayerCoors().y,
			FindPlayerSpeed().x, FindPlayerSpeed().y, pSwerve, pAccel, pBrake, pHandbrake);
		return;
	case MISSION_GOTOCOORDS_STRAIGHT:
	case MISSION_GOTO_COORDS_STRAIGHT_ACCURATE:
		SteerAICarWithPhysicsHeadingForTarget(pVehicle, nil,
			pVehicle->AutoPilot.m_vecDestinationCoors.x, pVehicle->AutoPilot.m_vecDestinationCoors.y,
			pSwerve, pAccel, pBrake, pHandbrake);
		return;
	case MISSION_EMERGENCYVEHICLE_STOP:
	case MISSION_STOP_FOREVER:
		*pSwerve = 0.0f;
		*pAccel = 0.0f;
		*pHandbrake = true;
		*pBrake = 0.5f;
		return;
	case MISSION_RAMCAR_CLOSE:
		SteerAICarWithPhysicsHeadingForTarget(pVehicle, pVehicle->AutoPilot.m_pTargetCar,
			pVehicle->AutoPilot.m_pTargetCar->GetPosition().x, pVehicle->AutoPilot.m_pTargetCar->GetPosition().y,
			pSwerve, pAccel, pBrake, pHandbrake);
		return;
	case MISSION_BLOCKCAR_CLOSE:
		SteerAICarWithPhysicsTryingToBlockTarget(pVehicle,
			pVehicle->AutoPilot.m_pTargetCar->GetPosition().x,
			pVehicle->AutoPilot.m_pTargetCar->GetPosition().y,
			pVehicle->AutoPilot.m_pTargetCar->GetMoveSpeed().x,
			pVehicle->AutoPilot.m_pTargetCar->GetMoveSpeed().y,
			pSwerve, pAccel, pBrake, pHandbrake);
		return;
	case MISSION_BLOCKCAR_HANDBRAKESTOP:
		SteerAICarWithPhysicsTryingToBlockTarget_Stop(pVehicle,
			pVehicle->AutoPilot.m_pTargetCar->GetPosition().x,
			pVehicle->AutoPilot.m_pTargetCar->GetPosition().y,
			pVehicle->AutoPilot.m_pTargetCar->GetMoveSpeed().x,
			pVehicle->AutoPilot.m_pTargetCar->GetMoveSpeed().y,
			pSwerve, pAccel, pBrake, pHandbrake);
		return;
	default:
		return;
	}
}

void CCarCtrl::SteerAIBoatWithPhysics(CBoat* pBoat)
{
	if (pBoat->AutoPilot.m_nCarMission == MISSION_GOTOCOORDS_ASTHECROWSWIMS){
		SteerAIBoatWithPhysicsHeadingForTarget(pBoat,
			pBoat->AutoPilot.m_vecDestinationCoors.x, pBoat->AutoPilot.m_vecDestinationCoors.y,
			&pBoat->m_fSteeringLeftRight, &pBoat->m_fAccelerate, &pBoat->m_fBrake);
	}else if (pBoat->AutoPilot.m_nCarMission == MISSION_NONE){
		pBoat->m_fSteeringLeftRight = 0.0f;
		pBoat->m_fAccelerate = 0.0f;
		pBoat->m_fBrake = 0.0f;
	}
	pBoat->m_fSteerAngle = pBoat->m_fSteeringLeftRight;
	pBoat->m_fGasPedal = pBoat->m_fAccelerate;
	pBoat->m_fBrakePedal = pBoat->m_fBrake;
	pBoat->bIsHandbrakeOn = false;
}

float CCarCtrl::FindMaxSteerAngle(CVehicle* pVehicle)
{
	return pVehicle->GetModelIndex() == MI_ENFORCER ? 0.7f : DEFAULT_MAX_STEER_ANGLE;
}

void CCarCtrl::SteerAICarWithPhysicsFollowPath(CVehicle* pVehicle, float* pSwerve, float* pAccel, float* pBrake, bool* pHandbrake)
{
	CVector2D forward = pVehicle->GetForward();
	forward.NormaliseSafe();
	CCarPathLink* pCurrentLink = &ThePaths.m_carPathLinks[pVehicle->AutoPilot.m_nCurrentPathNodeInfo];
	CCarPathLink* pNextLink = &ThePaths.m_carPathLinks[pVehicle->AutoPilot.m_nNextPathNodeInfo];
	CVector2D currentPathLinkForward(pCurrentLink->GetDirX() * pVehicle->AutoPilot.m_nCurrentDirection,
		pCurrentLink->GetDirY() * pVehicle->AutoPilot.m_nCurrentDirection);
	float nextPathLinkForwardX = pNextLink->GetDirX() * pVehicle->AutoPilot.m_nNextDirection;
	float nextPathLinkForwardY = pNextLink->GetDirY() * pVehicle->AutoPilot.m_nNextDirection;
	CVector2D positionOnCurrentLinkIncludingLane(
		pCurrentLink->GetX() + ((pVehicle->AutoPilot.m_nCurrentLane + pCurrentLink->OneWayLaneOffset()) * LANE_WIDTH) * currentPathLinkForward.y,
		pCurrentLink->GetY() - ((pVehicle->AutoPilot.m_nCurrentLane + pCurrentLink->OneWayLaneOffset()) * LANE_WIDTH) * currentPathLinkForward.x);
	CVector2D positionOnNextLinkIncludingLane(
		pNextLink->GetX() + ((pVehicle->AutoPilot.m_nNextLane + pNextLink->OneWayLaneOffset()) * LANE_WIDTH) * nextPathLinkForwardY,
		pNextLink->GetY() - ((pVehicle->AutoPilot.m_nNextLane + pNextLink->OneWayLaneOffset()) * LANE_WIDTH) * nextPathLinkForwardX);
	CVector2D distanceToNextNode = (CVector2D)pVehicle->GetPosition() - positionOnCurrentLinkIncludingLane;
	float scalarDistanceToNextNode = distanceToNextNode.Magnitude();
	CVector2D distanceBetweenNodes = positionOnNextLinkIncludingLane - positionOnCurrentLinkIncludingLane;
	float dp = DotProduct2D(distanceBetweenNodes, distanceToNextNode);
	if (scalarDistanceToNextNode < DISTANCE_TO_NEXT_NODE_TO_SELECT_NEW ||
	  dp > 0.0f && scalarDistanceToNextNode < DISTANCE_TO_FACING_NEXT_NODE_TO_SELECT_NEW ||
	  dp / (scalarDistanceToNextNode * distanceBetweenNodes.Magnitude()) > 0.7f ||
	  pVehicle->AutoPilot.m_nNextPathNodeInfo == pVehicle->AutoPilot.m_nCurrentPathNodeInfo){
		if (PickNextNodeAccordingStrategy(pVehicle)) {
			switch (pVehicle->AutoPilot.m_nCarMission){
			case MISSION_GOTOCOORDS:
				pVehicle->AutoPilot.m_nCarMission = MISSION_GOTOCOORDS_STRAIGHT;
				*pSwerve = 0.0f;
				*pAccel = 0.0f;
				*pBrake = 0.0f;
				*pHandbrake = false;
				return;
			case MISSION_GOTOCOORDS_ACCURATE:
				pVehicle->AutoPilot.m_nCarMission = MISSION_GOTO_COORDS_STRAIGHT_ACCURATE;
				*pSwerve = 0.0f;
				*pAccel = 0.0f;
				*pBrake = 0.0f;
				*pHandbrake = false;
				return;
			default: break;
			}
		}
		pCurrentLink = &ThePaths.m_carPathLinks[pVehicle->AutoPilot.m_nCurrentPathNodeInfo];
		scalarDistanceToNextNode = CVector2D(
			pCurrentLink->GetX() + ((pVehicle->AutoPilot.m_nCurrentLane + pCurrentLink->OneWayLaneOffset()) * LANE_WIDTH) * currentPathLinkForward.y - pVehicle->GetPosition().x,
			pCurrentLink->GetY() - ((pVehicle->AutoPilot.m_nCurrentLane + pCurrentLink->OneWayLaneOffset()) * LANE_WIDTH) * currentPathLinkForward.x - pVehicle->GetPosition().y).Magnitude();		
		pNextLink = &ThePaths.m_carPathLinks[pVehicle->AutoPilot.m_nNextPathNodeInfo];
		currentPathLinkForward.x = pCurrentLink->GetDirX() * pVehicle->AutoPilot.m_nCurrentDirection;
		currentPathLinkForward.y = pCurrentLink->GetDirY() * pVehicle->AutoPilot.m_nCurrentDirection;
		nextPathLinkForwardX = pNextLink->GetDirX() * pVehicle->AutoPilot.m_nNextDirection;
		nextPathLinkForwardY = pNextLink->GetDirY() * pVehicle->AutoPilot.m_nNextDirection;
	}
	positionOnCurrentLinkIncludingLane.x = pCurrentLink->GetX() + ((pVehicle->AutoPilot.m_nCurrentLane + pCurrentLink->OneWayLaneOffset()) * LANE_WIDTH) * currentPathLinkForward.y;
	positionOnCurrentLinkIncludingLane.y = pCurrentLink->GetY() - ((pVehicle->AutoPilot.m_nCurrentLane + pCurrentLink->OneWayLaneOffset()) * LANE_WIDTH) * currentPathLinkForward.x;
	CVector2D projectedPosition = positionOnCurrentLinkIncludingLane - currentPathLinkForward * scalarDistanceToNextNode * 0.4f;
	if (scalarDistanceToNextNode > DISTANCE_TO_NEXT_NODE_TO_CONSIDER_SLOWING_DOWN){
		projectedPosition.x = positionOnCurrentLinkIncludingLane.x;
		projectedPosition.y = positionOnCurrentLinkIncludingLane.y;
	}
	CVector2D distanceToProjectedPosition = projectedPosition - pVehicle->GetPosition();
	float angleCurrentLink = CGeneral::GetATanOfXY(distanceToProjectedPosition.x, distanceToProjectedPosition.y);
	float angleForward = CGeneral::GetATanOfXY(forward.x, forward.y);
	if (pVehicle->AutoPilot.m_nDrivingStyle == DRIVINGSTYLE_AVOID_CARS)
		angleCurrentLink = FindAngleToWeaveThroughTraffic(pVehicle, nil, angleCurrentLink, angleForward);
	float steerAngle = LimitRadianAngle(angleCurrentLink - angleForward);
	float maxAngle = FindMaxSteerAngle(pVehicle);
	steerAngle = Min(maxAngle, Max(-maxAngle, steerAngle));
	if (pVehicle->GetMoveSpeed().Magnitude() > MIN_SPEED_TO_START_LIMITING_STEER)
		steerAngle = Min(MAX_ANGLE_TO_STEER_AT_HIGH_SPEED, Max(-MAX_ANGLE_TO_STEER_AT_HIGH_SPEED, steerAngle));
	float currentForwardSpeed = DotProduct(pVehicle->GetMoveSpeed(), pVehicle->GetForward()) * GAME_SPEED_TO_CARAI_SPEED;
	float speedStyleMultiplier;
	switch (pVehicle->AutoPilot.m_nDrivingStyle) {
	case DRIVINGSTYLE_STOP_FOR_CARS:
	case DRIVINGSTYLE_SLOW_DOWN_FOR_CARS:
		speedStyleMultiplier = FindMaximumSpeedForThisCarInTraffic(pVehicle);
#ifdef FIX_BUGS
		if (pVehicle->AutoPilot.m_nCruiseSpeed != 0)
#endif
			speedStyleMultiplier /= pVehicle->AutoPilot.m_nCruiseSpeed;
		break;
	default:
		speedStyleMultiplier = 1.0f;
		break;
	}
	switch (pVehicle->AutoPilot.m_nDrivingStyle) {
	case DRIVINGSTYLE_STOP_FOR_CARS:
	case DRIVINGSTYLE_SLOW_DOWN_FOR_CARS:
		if (CTrafficLights::ShouldCarStopForLight(pVehicle, false)){
			CCarAI::CarHasReasonToStop(pVehicle);
			speedStyleMultiplier = 0.0f;
		}
		break;
	default:
		break;
	}
	if (CTrafficLights::ShouldCarStopForBridge(pVehicle)){
		CCarAI::CarHasReasonToStop(pVehicle);
		speedStyleMultiplier = 0.0f;
	}
	CVector2D trajectory(pCurrentLink->GetX() + ((pVehicle->AutoPilot.m_nCurrentLane + pCurrentLink->OneWayLaneOffset()) * LANE_WIDTH) * currentPathLinkForward.y,
		pCurrentLink->GetY() - ((pVehicle->AutoPilot.m_nCurrentLane + pCurrentLink->OneWayLaneOffset()) * LANE_WIDTH) * currentPathLinkForward.x);
	trajectory -= pVehicle->GetPosition();
	float speedAngleMultiplier = FindSpeedMultiplier(
		CGeneral::GetATanOfXY(trajectory.x, trajectory.y) - angleForward,
		MIN_ANGLE_FOR_SPEED_LIMITING, MAX_ANGLE_FOR_SPEED_LIMITING, MIN_LOWERING_SPEED_COEFFICIENT);
	float tmpWideMultiplier = FindSpeedMultiplier(
		CGeneral::GetATanOfXY(currentPathLinkForward.x, currentPathLinkForward.y) -
		CGeneral::GetATanOfXY(nextPathLinkForwardX, nextPathLinkForwardY),
		MIN_ANGLE_FOR_SPEED_LIMITING_BETWEEN_NODES, MAX_ANGLE_FOR_SPEED_LIMITING, MIN_LOWERING_SPEED_COEFFICIENT);
	float speedNodesMultiplier;
	if (scalarDistanceToNextNode > DISTANCE_TO_NEXT_NODE_TO_CONSIDER_SLOWING_DOWN || pVehicle->AutoPilot.m_nCruiseSpeed < 12)
		speedNodesMultiplier = 1.0f;
	else
		speedNodesMultiplier = 1.0f -
			(1.0f - scalarDistanceToNextNode / DISTANCE_TO_NEXT_NODE_TO_CONSIDER_SLOWING_DOWN) *
			(1.0f - tmpWideMultiplier);
	float speedMultiplier = Min(speedStyleMultiplier, Min(speedAngleMultiplier, speedNodesMultiplier));
	float speed = pVehicle->AutoPilot.m_nCruiseSpeed * speedMultiplier;
	float speedDifference = speed - currentForwardSpeed;
	if (speed < 0.05f && speedDifference < 0.03f){
		*pBrake = 1.0f;
		*pAccel = 0.0f;
	}else if (speedDifference <= 0.0f){
		*pBrake = Min(0.5f, -speedDifference * 0.05f);
		*pAccel = 0.0f;
	}else if (currentForwardSpeed < 2.0f){
		*pBrake = 0.0f;
		*pAccel = Min(1.0f, speedDifference * 0.25f);
	}else{
		*pBrake = 0.0f;
		*pAccel = Min(1.0f, speedDifference * 0.125f);
	}
	*pSwerve = steerAngle;
	*pHandbrake = false;
}

void CCarCtrl::SteerAICarWithPhysicsHeadingForTarget(CVehicle* pVehicle, CPhysical* pTarget, float targetX, float targetY, float* pSwerve, float* pAccel, float* pBrake, bool* pHandbrake)
{
	*pHandbrake = false;
	CVector2D forward = pVehicle->GetForward();
	forward.NormaliseSafe();
	float angleToTarget = CGeneral::GetATanOfXY(targetX - pVehicle->GetPosition().x, targetY - pVehicle->GetPosition().y);
	float angleForward = CGeneral::GetATanOfXY(forward.x, forward.y);
	if (pVehicle->AutoPilot.m_nDrivingStyle == DRIVINGSTYLE_AVOID_CARS)
		angleToTarget = FindAngleToWeaveThroughTraffic(pVehicle, pTarget, angleToTarget, angleForward);
	float steerAngle = LimitRadianAngle(angleToTarget - angleForward);
	if (pVehicle->GetMoveSpeed().Magnitude() > MIN_SPEED_TO_APPLY_HANDBRAKE)
		if (ABS(steerAngle) > MIN_ANGLE_TO_APPLY_HANDBRAKE)
			*pHandbrake = true;
	float maxAngle = FindMaxSteerAngle(pVehicle);
	steerAngle = Min(maxAngle, Max(-maxAngle, steerAngle));
	float speedMultiplier = FindSpeedMultiplier(CGeneral::GetATanOfXY(targetX - pVehicle->GetPosition().x, targetY - pVehicle->GetPosition().y) - angleForward,
		MIN_ANGLE_FOR_SPEED_LIMITING, MAX_ANGLE_FOR_SPEED_LIMITING, MIN_LOWERING_SPEED_COEFFICIENT);
	float speedTarget = pVehicle->AutoPilot.m_nCruiseSpeed * speedMultiplier;
	float currentSpeed = pVehicle->GetMoveSpeed().Magnitude() * GAME_SPEED_TO_CARAI_SPEED;
	float speedDiff = speedTarget - currentSpeed;
	if (speedDiff <= 0.0f){
		*pAccel = 0.0f;
		*pBrake = Min(0.5f, -speedDiff * 0.05f);
	}else if (currentSpeed < 25.0f){
		*pAccel = Min(1.0f, speedDiff * 0.1f);
		*pBrake = 0.0f;
	}else{
		*pAccel = 1.0f;
		*pBrake = 0.0f;
	}
	*pSwerve = steerAngle;
}

void CCarCtrl::SteerAICarWithPhysicsTryingToBlockTarget(CVehicle* pVehicle, float targetX, float targetY, float targetSpeedX, float targetSpeedY, float* pSwerve, float* pAccel, float* pBrake, bool* pHandbrake)
{
	CVector2D targetPos(targetX, targetY);
	CVector2D offset(targetSpeedX, targetSpeedY);
	float trajectoryLen = offset.Magnitude();
	if (trajectoryLen > MAX_SPEED_TO_ACCOUNT_IN_INTERCEPTING)
		offset *= MAX_SPEED_TO_ACCOUNT_IN_INTERCEPTING / trajectoryLen;
	targetPos += offset * GAME_SPEED_TO_CARAI_SPEED;
	pVehicle->AutoPilot.m_nDrivingStyle = DRIVINGSTYLE_AVOID_CARS;
	SteerAICarWithPhysicsHeadingForTarget(pVehicle, nil, targetPos.x, targetPos.y, pSwerve, pAccel, pBrake, pHandbrake);
	if ((targetPos - pVehicle->GetPosition()).MagnitudeSqr() < SQR(DISTANCE_TO_SWITCH_FROM_BLOCK_TO_STOP))
		pVehicle->AutoPilot.m_nCarMission = (pVehicle->AutoPilot.m_nCarMission == MISSION_BLOCKCAR_CLOSE) ?
			MISSION_BLOCKCAR_HANDBRAKESTOP : MISSION_BLOCKPLAYER_HANDBRAKESTOP;
}
void CCarCtrl::SteerAICarWithPhysicsTryingToBlockTarget_Stop(CVehicle* pVehicle, float targetX, float targetY, float targetSpeedX, float targetSpeedY, float* pSwerve, float* pAccel, float* pBrake, bool* pHandbrake)
{
	*pSwerve = 0.0f;
	*pAccel = 0.0f;
	*pBrake = 1.0f;
	*pHandbrake = true;
	float distanceToTargetSqr = (CVector2D(targetX, targetY) - pVehicle->GetPosition()).MagnitudeSqr();
	if (distanceToTargetSqr > SQR(DISTANCE_TO_SWITCH_FROM_STOP_TO_BLOCK)){
		pVehicle->AutoPilot.m_nCarMission = (pVehicle->AutoPilot.m_nCarMission == MISSION_BLOCKCAR_HANDBRAKESTOP) ?
			MISSION_BLOCKCAR_CLOSE : MISSION_BLOCKPLAYER_CLOSE;
		return;
	}
	if (pVehicle->AutoPilot.m_nCarMission == MISSION_BLOCKCAR_HANDBRAKESTOP){
		if (((CVector2D)pVehicle->GetMoveSpeed()).MagnitudeSqr() < SQR(0.01f) &&
		  CVector2D(targetSpeedX, targetSpeedY).MagnitudeSqr() < SQR(0.02f) &&
		  pVehicle->bIsLawEnforcer){
			CCarAI::TellOccupantsToLeaveCar(pVehicle);
			pVehicle->AutoPilot.m_nCruiseSpeed = 0;
			pVehicle->AutoPilot.m_nCarMission = MISSION_NONE;
		}
	}else{
		if (FindPlayerVehicle() && FindPlayerVehicle()->GetMoveSpeed().Magnitude() < 0.05f)
#ifdef FIX_BUGS
			pVehicle->m_nTimeBlocked += CTimer::GetTimeStepInMilliseconds();
#else
			pVehicle->m_nTimeBlocked += 1000.0f / 60.0f * CTimer::GetTimeStep(); // very doubtful constant
#endif
		else
			pVehicle->m_nTimeBlocked = 0;
		if (FindPlayerVehicle() == nil || FindPlayerVehicle()->IsUpsideDown() ||
		  FindPlayerVehicle()->GetMoveSpeed().Magnitude() < 0.05f &&
		  pVehicle->m_nTimeBlocked > TIME_COPS_WAIT_TO_EXIT_AFTER_STOPPING){
			if (pVehicle->bIsLawEnforcer && distanceToTargetSqr < SQR(DISTANCE_TO_SWITCH_FROM_STOP_TO_BLOCK)){
				CCarAI::TellOccupantsToLeaveCar(pVehicle);
				pVehicle->AutoPilot.m_nCruiseSpeed = 0;
				pVehicle->AutoPilot.m_nCarMission = MISSION_NONE;
			}
		}
	}
}

void CCarCtrl::SteerAIBoatWithPhysicsHeadingForTarget(CBoat* pBoat, float targetX, float targetY, float* pSwerve, float* pAccel, float* pBrake)
{
	CVector2D forward(pBoat->GetForward());
	forward.NormaliseSafe();
	CVector2D distanceToTarget = CVector2D(targetX, targetY) - pBoat->GetPosition();
	float angleToTarget = CGeneral::GetATanOfXY(distanceToTarget.x, distanceToTarget.y);
	float angleForward = CGeneral::GetATanOfXY(forward.x, forward.y);
	float angleDiff = LimitRadianAngle(angleToTarget - angleForward);
	angleDiff = Min(DEFAULT_MAX_STEER_ANGLE, Max(-DEFAULT_MAX_STEER_ANGLE, angleDiff));
	float currentSpeed = pBoat->GetMoveSpeed().Magnitude2D(); // +0.0f for some reason
	float speedDiff = pBoat->AutoPilot.m_nCruiseSpeed - currentSpeed * 60.0f;
	if (speedDiff > 0.0f){
		float accRemaining = speedDiff / pBoat->AutoPilot.m_nCruiseSpeed;
		*pAccel = (accRemaining > 0.25f) ? 1.0f : 1.0f - (0.25f - accRemaining) * 4.0f;
	}else
		*pAccel = (speedDiff < -5.0f) ? -0.2f : -0.1f;
	*pBrake = 0.0f;
	*pSwerve = angleDiff;
}

void
CCarCtrl::RegisterVehicleOfInterest(CVehicle* pVehicle)
{
	for (int i = 0; i < MAX_CARS_TO_KEEP; i++) {
		if (apCarsToKeep[i] == pVehicle) {
			aCarsToKeepTime[i] = CTimer::GetTimeInMilliseconds();
			return;
		}
	}
	for (int i = 0; i < MAX_CARS_TO_KEEP; i++) {
		if (!apCarsToKeep[i]) {
			apCarsToKeep[i] = pVehicle;
			aCarsToKeepTime[i] = CTimer::GetTimeInMilliseconds();
			return;
		}
	}
	uint32 oldestCarWeKeepTime = UINT32_MAX;
	int oldestCarWeKeepIndex = 0;
	for (int i = 0; i < MAX_CARS_TO_KEEP; i++) {
		if (apCarsToKeep[i] && aCarsToKeepTime[i] < oldestCarWeKeepTime) {
			oldestCarWeKeepTime = aCarsToKeepTime[i];
			oldestCarWeKeepIndex = i;
		}
	}
	apCarsToKeep[oldestCarWeKeepIndex] = pVehicle;
	aCarsToKeepTime[oldestCarWeKeepIndex] = CTimer::GetTimeInMilliseconds();
}

bool
CCarCtrl::IsThisVehicleInteresting(CVehicle* pVehicle)
{
	for (int i = 0; i < MAX_CARS_TO_KEEP; i++) {
		if (apCarsToKeep[i] == pVehicle)
			return true;
	}
	return false;
}

void CCarCtrl::RemoveFromInterestingVehicleList(CVehicle* pVehicle)
{
	for (int i = 0; i < MAX_CARS_TO_KEEP; i++) {
		if (apCarsToKeep[i] == pVehicle)
			apCarsToKeep[i] = nil;
	}
}

void CCarCtrl::ClearInterestingVehicleList()
{
	for (int i = 0; i < MAX_CARS_TO_KEEP; i++) {
		apCarsToKeep[i] = nil;
	}
}

void CCarCtrl::SwitchVehicleToRealPhysics(CVehicle* pVehicle)
{
	pVehicle->AutoPilot.m_nCarMission = MISSION_CRUISE;
	pVehicle->AutoPilot.m_nTempAction = TEMPACT_NONE;
	pVehicle->AutoPilot.m_nAntiReverseTimer = CTimer::GetTimeInMilliseconds();
	pVehicle->AutoPilot.m_nTimeTempAction = CTimer::GetTimeInMilliseconds();
}

void CCarCtrl::JoinCarWithRoadSystem(CVehicle* pVehicle)
{
	pVehicle->AutoPilot.m_nPrevRouteNode = pVehicle->AutoPilot.m_nCurrentRouteNode = pVehicle->AutoPilot.m_nNextRouteNode = 0;
	pVehicle->AutoPilot.m_nCurrentPathNodeInfo = pVehicle->AutoPilot.m_nPreviousPathNodeInfo = pVehicle->AutoPilot.m_nNextPathNodeInfo = 0;
	int nodeId = ThePaths.FindNodeClosestToCoorsFavourDirection(pVehicle->GetPosition(), 0, pVehicle->GetForward().x, pVehicle->GetForward().y);
	CPathNode* pNode = &ThePaths.m_pathNodes[nodeId];
	int prevNodeId = -1;
	float minDistance = 999999.9f;
	for (int i = 0; i < pNode->numLinks; i++){
		int candidateId = ThePaths.ConnectedNode(i + pNode->firstLink);
		CPathNode* pCandidateNode = &ThePaths.m_pathNodes[candidateId];
		float distance = (pCandidateNode->GetPosition() - pNode->GetPosition()).Magnitude2D();
		if (distance < minDistance){
			minDistance = distance;
			prevNodeId = candidateId;
		}
	}
	if (prevNodeId < 0)
		return;
	CVector2D forward = pVehicle->GetForward();
	CPathNode* pPrevNode = &ThePaths.m_pathNodes[prevNodeId];
	if (forward.x == 0.0f && forward.y == 0.0f)
		forward.x = 1.0f;
	if (DotProduct2D(pNode->GetPosition() - pPrevNode->GetPosition(), forward) < 0.0f){
		int tmp;
		tmp = prevNodeId;
		prevNodeId = nodeId;
		nodeId = tmp;
	}
	pVehicle->AutoPilot.m_nPrevRouteNode = 0;
	pVehicle->AutoPilot.m_nCurrentRouteNode = prevNodeId;
	pVehicle->AutoPilot.m_nNextRouteNode = nodeId;
	pVehicle->AutoPilot.m_nPathFindNodesCount = 0;
	FindLinksToGoWithTheseNodes(pVehicle);
	pVehicle->AutoPilot.m_nNextLane = pVehicle->AutoPilot.m_nCurrentLane = 0;
}

bool CCarCtrl::JoinCarWithRoadSystemGotoCoors(CVehicle* pVehicle, CVector vecTarget, bool isProperNow)
{
	pVehicle->AutoPilot.m_vecDestinationCoors = vecTarget;
	ThePaths.DoPathSearch(0, pVehicle->GetPosition(), -1, vecTarget, pVehicle->AutoPilot.m_aPathFindNodesInfo,
		&pVehicle->AutoPilot.m_nPathFindNodesCount, NUM_PATH_NODES_IN_AUTOPILOT, pVehicle, nil, 999999.9f, -1);
	ThePaths.RemoveBadStartNode(pVehicle->GetPosition(),
		pVehicle->AutoPilot.m_aPathFindNodesInfo, &pVehicle->AutoPilot.m_nPathFindNodesCount);
	if (pVehicle->AutoPilot.m_nPathFindNodesCount < 2){
		pVehicle->AutoPilot.m_nPrevRouteNode = pVehicle->AutoPilot.m_nCurrentRouteNode = pVehicle->AutoPilot.m_nNextRouteNode = 0;
		return true;
	}
	pVehicle->AutoPilot.m_nPrevRouteNode = 0;
	pVehicle->AutoPilot.m_nCurrentRouteNode = pVehicle->AutoPilot.m_aPathFindNodesInfo[0] - ThePaths.m_pathNodes;
	pVehicle->AutoPilot.RemoveOnePathNode();
	pVehicle->AutoPilot.m_nNextRouteNode = pVehicle->AutoPilot.m_aPathFindNodesInfo[0] - ThePaths.m_pathNodes;
	pVehicle->AutoPilot.RemoveOnePathNode();
	FindLinksToGoWithTheseNodes(pVehicle);
	pVehicle->AutoPilot.m_nNextLane = pVehicle->AutoPilot.m_nCurrentLane = 0;
	return false;
}

void CCarCtrl::FindLinksToGoWithTheseNodes(CVehicle* pVehicle)
{
	int nextLink;
	CPathNode* pCurNode = &ThePaths.m_pathNodes[pVehicle->AutoPilot.m_nCurrentRouteNode];
	for (nextLink = 0; nextLink < 12; nextLink++)
		if (ThePaths.ConnectedNode(nextLink + pCurNode->firstLink) == pVehicle->AutoPilot.m_nNextRouteNode)
			break;
	pVehicle->AutoPilot.m_nNextPathNodeInfo = ThePaths.m_carPathConnections[nextLink + pCurNode->firstLink];
	pVehicle->AutoPilot.m_nNextDirection = (pVehicle->AutoPilot.m_nCurrentRouteNode >= pVehicle->AutoPilot.m_nNextRouteNode) ? 1 : -1;
	int curLink;
	int curConnection;
	if (pCurNode->numLinks == 1) {
		curLink = 0;
		curConnection = ThePaths.m_carPathConnections[pCurNode->firstLink];
	}else{
		curConnection = pVehicle->AutoPilot.m_nNextPathNodeInfo;
		while (curConnection == pVehicle->AutoPilot.m_nNextPathNodeInfo){
			curLink = CGeneral::GetRandomNumber() % pCurNode->numLinks;
			curConnection = ThePaths.m_carPathConnections[curLink + pCurNode->firstLink];
		}
	}
	pVehicle->AutoPilot.m_nCurrentPathNodeInfo = curConnection;
	pVehicle->AutoPilot.m_nCurrentDirection = (ThePaths.ConnectedNode(curLink + pCurNode->firstLink) >= pVehicle->AutoPilot.m_nCurrentRouteNode) ? 1 : -1;
}

void CCarCtrl::GenerateEmergencyServicesCar(void)
{
	if (FindPlayerPed()->m_pWanted->GetWantedLevel() > 3)
		return;
	if (NumFiretrucksOnDuty + NumAmbulancesOnDuty + NumParkedCars + NumMissionCars +
		NumLawEnforcerCars + NumRandomCars > MaxNumberOfCarsInUse)
		return;
	if (NumAmbulancesOnDuty == 0){
		if (gAccidentManager.CountActiveAccidents() < 2){
			if (CStreaming::HasModelLoaded(MI_AMBULAN))
				CStreaming::SetModelIsDeletable(MI_MEDIC);
		}else{
			float distance = 30.0f;
			CAccident* pNearestAccident = gAccidentManager.FindNearestAccident(FindPlayerCoors(), &distance);
			if (pNearestAccident){
				if (CountCarsOfType(MI_AMBULAN) < 2 && CTimer::GetTimeInMilliseconds() > LastTimeAmbulanceCreated + 30000){
					CStreaming::RequestModel(MI_AMBULAN, STREAMFLAGS_DEPENDENCY);
					CStreaming::RequestModel(MI_MEDIC, STREAMFLAGS_DONT_REMOVE);
					if (CStreaming::HasModelLoaded(MI_AMBULAN) && CStreaming::HasModelLoaded(MI_MEDIC)){
						if (GenerateOneEmergencyServicesCar(MI_AMBULAN, pNearestAccident->m_pVictim->GetPosition()))
							LastTimeAmbulanceCreated = CTimer::GetTimeInMilliseconds();
					}
				}
			}
		}
	}
	if (NumFiretrucksOnDuty == 0){
		if (gFireManager.GetTotalActiveFires() < 3){
			if (CStreaming::HasModelLoaded(MI_FIRETRUCK))
				CStreaming::SetModelIsDeletable(MI_FIREMAN);
		}else{
			float distance = 30.0f;
			CFire* pNearestFire = gFireManager.FindNearestFire(FindPlayerCoors(), &distance);
			if (pNearestFire) {
				if (CountCarsOfType(MI_FIRETRUCK) < 2 && CTimer::GetTimeInMilliseconds() > LastTimeFireTruckCreated + 35000){
					CStreaming::RequestModel(MI_FIRETRUCK, STREAMFLAGS_DEPENDENCY);
					CStreaming::RequestModel(MI_FIREMAN, STREAMFLAGS_DONT_REMOVE);
					if (CStreaming::HasModelLoaded(MI_FIRETRUCK) && CStreaming::HasModelLoaded(MI_FIREMAN)){
						if (GenerateOneEmergencyServicesCar(MI_FIRETRUCK, pNearestFire->m_vecPos))
							LastTimeFireTruckCreated = CTimer::GetTimeInMilliseconds();
					}
				}
			}
		}
	}
}

bool CCarCtrl::GenerateOneEmergencyServicesCar(uint32 mi, CVector vecPos)
{
	CVector pPlayerPos = FindPlayerCentreOfWorld(CWorld::PlayerInFocus);
	bool created = false;
	int attempts = 0;
	CVector spawnPos;
	int curNode, nextNode;
	float posBetweenNodes;
	while (!created && attempts < 5){
		if (ThePaths.NewGenerateCarCreationCoors(pPlayerPos.x, pPlayerPos.y, 0.707f, 0.707f,
		  120.0f, -1.0f, true, &spawnPos, &curNode, &nextNode, &posBetweenNodes, false)){
			int16 colliding[2];
			CWorld::FindObjectsKindaColliding(spawnPos, 10.0f, true, colliding, 2, nil, false, true, true, false, false);
			if (colliding[0] == 0)
				created = true;
		}
		attempts += 1;
	}
	if (attempts >= 5)
		return nil;
	CAutomobile* pVehicle = new CAutomobile(mi, RANDOM_VEHICLE);
	pVehicle->AutoPilot.m_vecDestinationCoors = vecPos;
	pVehicle->SetPosition(spawnPos);
	pVehicle->AutoPilot.m_nCarMission = (JoinCarWithRoadSystemGotoCoors(pVehicle, vecPos, false)) ? MISSION_GOTOCOORDS_STRAIGHT : MISSION_GOTOCOORDS;
	pVehicle->AutoPilot.m_fMaxTrafficSpeed = pVehicle->AutoPilot.m_nCruiseSpeed = 25;
	pVehicle->AutoPilot.m_nTempAction = TEMPACT_NONE;
	pVehicle->AutoPilot.m_nDrivingStyle = DRIVINGSTYLE_AVOID_CARS;
	CVector2D direction = vecPos - spawnPos;
	direction.NormaliseSafe();
	pVehicle->GetForward() = CVector(direction.x, direction.y, 0.0f);
	pVehicle->GetRight() = CVector(direction.y, -direction.x, 0.0f);
	pVehicle->GetUp() = CVector(0.0f, 0.0f, 1.0f);
	spawnPos.z = posBetweenNodes * ThePaths.m_pathNodes[curNode].GetZ() + (1.0f - posBetweenNodes) * ThePaths.m_pathNodes[nextNode].GetZ();
	float groundZ = INFINITE_Z;
	CColPoint colPoint;
	CEntity* pEntity;
	if (CWorld::ProcessVerticalLine(spawnPos, 1000.0f, colPoint, pEntity, true, false, false, false, true, false, nil))
		groundZ = colPoint.point.z;
	if (CWorld::ProcessVerticalLine(spawnPos, -1000.0f, colPoint, pEntity, true, false, false, false, true, false, nil)) {
		if (ABS(colPoint.point.z - spawnPos.z) < ABS(groundZ - spawnPos.z))
			groundZ = colPoint.point.z;
	}
	if (groundZ == INFINITE_Z) {
		delete pVehicle;
		return false;
	}
	spawnPos.z = groundZ + pVehicle->GetDistanceFromCentreOfMassToBaseOfModel();
	pVehicle->SetPosition(spawnPos);
	pVehicle->SetMoveSpeed(CVector(0.0f, 0.0f, 0.0f));
	pVehicle->SetStatus(STATUS_PHYSICS);
	switch (mi){
	case MI_FIRETRUCK:
		pVehicle->bIsFireTruckOnDuty = true;
		++NumFiretrucksOnDuty;
		CCarAI::AddFiretruckOccupants(pVehicle);
		break;
	case MI_AMBULAN:
		pVehicle->bIsAmbulanceOnDuty = true;
		++NumAmbulancesOnDuty;
		CCarAI::AddAmbulanceOccupants(pVehicle);
		break;
	}
	pVehicle->m_bSirenOrAlarm = true;
	CWorld::Add(pVehicle);
	printf("CREATED EMERGENCY VEHICLE\n");
	return true;
}

void CCarCtrl::UpdateCarCount(CVehicle* pVehicle, bool remove)
{
	if (remove){
		switch (pVehicle->VehicleCreatedBy){
		case RANDOM_VEHICLE:
			if (pVehicle->bIsLawEnforcer)
				--NumLawEnforcerCars;
			--NumRandomCars;
			return;
		case MISSION_VEHICLE:
			--NumMissionCars;
			return;
		case PARKED_VEHICLE:
			--NumParkedCars;
			return;
		case PERMANENT_VEHICLE:
			--NumPermanentCars;;
			return;
		}
	}
	else{
		switch (pVehicle->VehicleCreatedBy){
		case RANDOM_VEHICLE:
			if (pVehicle->bIsLawEnforcer)
				++NumLawEnforcerCars;
			++NumRandomCars;
			return;
		case MISSION_VEHICLE:
			++NumMissionCars;
			return;
		case PARKED_VEHICLE:
			++NumParkedCars;
			return;
		case PERMANENT_VEHICLE:
			++NumPermanentCars;;
			return;
		}
	}
}

bool CCarCtrl::ThisRoadObjectCouldMove(int16 mi)
{
	return mi == MI_BRIDGELIFT || mi == MI_BRIDGEROADSEGMENT;
}

bool CCarCtrl::MapCouldMoveInThisArea(float x, float y)
{
	// bridge moves up and down
	return x > -342.0f && x < -219.0f &&
		y > -677.0f && y < -580.0f;
}
