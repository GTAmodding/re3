#pragma once
#include "Entity.h"
#include "math/Vector.h"

enum eCrimeType : __int32 {
	CRIME_NONE = 0x0,
	CRIME_SHOT_FIRED = 0x1,
	CRIME_PED_FIGHT = 0x2,
	CRIME_COP_FIGHT = 0x3,
	CRIME_DAMAGED_PED = 0x4,
	CRIME_DAMAGED_COP = 0x5,
	CRIME_CAR_THEFT = 0x6,
	CRIME_CRIME7 = 0x7,
	CRIME_COP_EVASIVE_DIVE = 0x8,
	CRIME_COP_EVASIVE_DIVE2 = 0x9,
	CRIME_PED_RUN_OVER = 0xA,
	CRIME_COP_RUN_OVER = 0xB,
	CRIME_DESTROYED_HELI = 0xC,
	CRIME_PED_BURNED = 0xD,
	CRIME_COP_BURNED = 0xE,
	CRIME_VEHICLE_BURNED = 0xF,
	CRIME_DESTROYED_CESSNA = 0x10,
};

enum eCopType : __int32 {
	COP_STREET = 0,
	COP_FBI = 1,
	COP_SWAT = 2,
	COP_ARMY = 3,
};

class CCrime {
public:
	eCrimeType m_eCrimeType;
	CEntity *m_pVictim;
	int m_nCrimeTime;
	CVector m_vecCrimePos;
	char m_bReported;
	char m_bMultiplier;
	char pad_20[2];
};

class CCopPed {
public:
	__int16 m_wRoadblockNode;
	char field_1342;
	char field_1343;
	float m_fDistanceToTarget;
	char m_bIsInPursuit;
	char m_bIsDisabledCop;
	char field_1350;
	char field_1351;
	char m_bZoneDisabledButClose;
	char m_bZoneDisabled;
	char field_1354;
	char field_1355;
	int field_1356;
	eCopType m_nCopType;
	char field_1364;
	char field_1365;
	char field_1366;
	char field_1367;
};

class CWanted {
public:
	int m_nChaos;
	int m_nLastUpdateTime;
	int m_nLastWantedLevelChange;
	float m_fCrimeSensitivity;
	unsigned __int8 m_bCurrentCops;
	unsigned __int8 m_bMaxCops;
	unsigned __int8 m_bMaximumLawEnforcerVehicles;
	char field_19;
	__int16 m_wRoadblockDensity;
	unsigned __int8 m_bFlags;
	char field_23;
	int m_nWantedLevel;
	CCrime m_sCrimes[16];
	CCopPed *m_pCops[10];
};

static_assert(sizeof(CWanted) == 0x204, "CWanted: error");
