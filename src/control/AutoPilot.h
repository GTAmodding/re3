#pragma once
#include "Timer.h"

class CVehicle;
struct CPathNode;

enum eCarMission
{
	MISSION_NONE,
	MISSION_CRUISE,
	MISSION_RAMPLAYER_FARAWAY,
	MISSION_RAMPLAYER_CLOSE,
	MISSION_BLOCKPLAYER_FARAWAY,
	MISSION_BLOCKPLAYER_CLOSE,
	MISSION_BLOCKPLAYER_HANDBRAKESTOP,
	MISSION_WAITFORDELETION,
	MISSION_GOTOCOORDS,
	MISSION_GOTOCOORDS_STRAIGHT,
	MISSION_EMERGENCYVEHICLE_STOP,
	MISSION_STOP_FOREVER,
	MISSION_GOTOCOORDS_ACCURATE,
	MISSION_GOTO_COORDS_STRAIGHT_ACCURATE,
	MISSION_GOTOCOORDS_ASTHECROWSWIMS,
	MISSION_RAMCAR_FARAWAY,
	MISSION_RAMCAR_CLOSE,
	MISSION_BLOCKCAR_FARAWAY,
	MISSION_BLOCKCAR_CLOSE,
	MISSION_BLOCKCAR_HANDBRAKESTOP,
};

enum eCarTempAction
{
	TEMPACT_NONE,
	TEMPACT_WAIT,
	TEMPACT_REVERSE,
	TEMPACT_HANDBRAKETURNLEFT,
	TEMPACT_HANDBRAKETURNRIGHT,
	TEMPACT_HANDBRAKESTRAIGHT,
	TEMPACT_TURNLEFT,
	TEMPACT_TURNRIGHT,
	TEMPACT_GOFORWARD,
	TEMPACT_SWERVELEFT,
	TEMPACT_SWERVERIGHT
};

enum eCarDrivingStyle
{
	DRIVINGSTYLE_STOP_FOR_CARS,
	DRIVINGSTYLE_SLOW_DOWN_FOR_CARS,
	DRIVINGSTYLE_AVOID_CARS,
	DRIVINGSTYLE_PLOUGH_THROUGH,
	DRIVINGSTYLE_STOP_FOR_CARS_IGNORE_LIGHTS
};

class CAutoPilot {
public:
	int32 m_nCurrentRouteNode;
	int32 m_nNextRouteNode;
	int32 m_nPrevRouteNode;
	int32 m_nTimeEnteredCurve;
	int32 m_nTimeToSpendOnCurrentCurve;
	uint32 m_nCurrentPathNodeInfo;
	uint32 m_nNextPathNodeInfo;
	uint32 m_nPreviousPathNodeInfo;
	uint32 m_nAntiReverseTimer;
	uint32 m_nTimeToStartMission;
	int8 m_nPreviousDirection;
	int8 m_nCurrentDirection;
	int8 m_nNextDirection;
	int8 m_nCurrentLane;
	int8 m_nNextLane;
	uint8 m_nDrivingStyle;
	uint8 m_nCarMission;
	uint8 m_nTempAction;
	uint32 m_nTimeTempAction;
	float m_fMaxTrafficSpeed;
	uint8 m_nCruiseSpeed;
	uint8 m_bSlowedDownBecauseOfCars : 1;
	uint8 m_bSlowedDownBecauseOfPeds : 1;
	uint8 m_bStayInCurrentLevel : 1;
	uint8 m_bStayInFastLane : 1;
	uint8 m_bIgnorePathfinding : 1;
	CVector m_vecDestinationCoors;
	CPathNode *m_aPathFindNodesInfo[NUM_PATH_NODES_IN_AUTOPILOT];
	int16 m_nPathFindNodesCount;
	CVehicle *m_pTargetCar;

	CAutoPilot(void) {
		m_nPrevRouteNode = 0;
		m_nNextRouteNode = m_nPrevRouteNode;
		m_nCurrentRouteNode = m_nNextRouteNode;
		m_nTimeEnteredCurve = 0;
		m_nTimeToSpendOnCurrentCurve = 1000;
		m_nPreviousPathNodeInfo = 0;
		m_nNextPathNodeInfo = m_nPreviousPathNodeInfo;
		m_nCurrentPathNodeInfo = m_nNextPathNodeInfo;
		m_nNextDirection = 1;
		m_nCurrentDirection = m_nNextDirection;
		m_nCurrentLane = m_nNextLane = 0;
		m_nDrivingStyle = DRIVINGSTYLE_STOP_FOR_CARS;
		m_nCarMission = MISSION_NONE;
		m_nTempAction = TEMPACT_NONE;
		m_nCruiseSpeed = 10;
		m_fMaxTrafficSpeed = 10.0f;
		m_bSlowedDownBecauseOfPeds = false;
		m_bSlowedDownBecauseOfCars = false;
		m_nPathFindNodesCount = 0;
		m_pTargetCar = 0;
		m_nTimeToStartMission = CTimer::GetTimeInMilliseconds();
		m_nAntiReverseTimer = m_nTimeToStartMission;
		m_bStayInFastLane = false;
	}

	void ModifySpeed(float);
	void RemoveOnePathNode();
#ifdef COMPATIBLE_SAVES
	void Save(uint8*& buf);
	void Load(uint8*& buf);
#endif

};

VALIDATE_SIZE(CAutoPilot, 0x70);
