#pragma once
#include "Entity.h"
#include "math/Vector.h"

enum eCrimeType {
	CRIME_NONE,
	CRIME_SHOT_FIRED,
	CRIME_PED_FIGHT,
	CRIME_COP_FIGHT,
	CRIME_DAMAGED_PED,
	CRIME_DAMAGED_COP,
	CRIME_CAR_THEFT,
	CRIME_CRIME7,
	CRIME_COP_EVASIVE_DIVE,
	CRIME_COP_EVASIVE_DIVE2,
	CRIME_PED_RUN_OVER,
	CRIME_COP_RUN_OVER,
	CRIME_DESTROYED_HELI,
	CRIME_PED_BURNED,
	CRIME_COP_BURNED,
	CRIME_VEHICLE_BURNED,
	CRIME_DESTROYED_CESSNA,
};

enum eCopType {
	COP_STREET = 0,
	COP_FBI = 1,
	COP_SWAT = 2,
	COP_ARMY = 3,
};

class CCrime {
public:
	eCrimeType m_eCrimeType;
	CEntity *m_pVictim;
	int32 m_nCrimeTime;
	CVector m_vecCrimePos;
	int8 m_bReported;
	int8 m_bMultiplier;
	int8 pad_20[2];
};

class CCopPed {
public:
	int16 m_wRoadblockNode;
	int8 field_1342;
	int8 field_1343;
	Float m_fDistanceToTarget;
	int8 m_bIsInPursuit;
	int8 m_bIsDisabledCop;
	int8 field_1350;
	int8 field_1351;
	int8 m_bZoneDisabledButClose;
	int8 m_bZoneDisabled;
	int8 field_1354;
	int8 field_1355;
	int32 field_1356;
	eCopType m_nCopType;
	int8 field_1364;
	int8 field_1365;
	int8 field_1366;
	int8 field_1367;
};

class CWanted {
public:
	int32 m_nChaos;
	int32 m_nLastUpdateTime;
	int32 m_nLastWantedLevelChange;
	Float m_fCrimeSensitivity;
	uint8 m_bCurrentCops;
	uint8 m_bMaxCops;
	uint8 m_bMaximumLawEnforcerVehicles;
	int8 field_19;
	int16 m_wRoadblockDensity;
	uint8 m_bFlags;
	int8 field_23;
	int32 m_nWantedLevel;
	CCrime m_sCrimes[16];
	CCopPed *m_pCops[10];
};

static_assert(sizeof(CWanted) == 0x204, "CWanted: error");
