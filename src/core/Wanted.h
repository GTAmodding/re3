#pragma once

class CEntity;
class CCopPed;

enum eCrimeType
{
	CRIME_NONE,
	CRIME_POSSESSION_GUN,
	CRIME_HIT_PED,
	CRIME_HIT_COP,
	CRIME_SHOOT_PED,
	CRIME_SHOOT_COP,
	CRIME_STEAL_CAR,
	CRIME_RUN_REDLIGHT,
	CRIME_RECKLESS_DRIVING,
	CRIME_SPEEDING,
	CRIME_RUNOVER_PED,
	CRIME_RUNOVER_COP,
	CRIME_SHOOT_HELI,
	CRIME_PED_BURNED,
	CRIME_COP_BURNED,
	CRIME_VEHICLE_BURNED,
	CRIME_DESTROYED_CESSNA,
};

class CCrimeBeingQd
{
public:
	eCrimeType m_nType;
	uint32 m_nId;
	int32 m_nTime;
	CVector m_vecPosn;
	bool m_bReported;
	bool m_bPoliceDoesntCare;

	CCrimeBeingQd() { };
	~CCrimeBeingQd() { };
};

class CWanted
{
public:
	int32 m_nChaos;
	int32 m_nLastUpdateTime;
	int32 m_nLastWantedLevelChange;
	float m_fCrimeSensitivity;
	uint8 m_CurrentCops;
	uint8 m_MaxCops;
	uint8 m_MaximumLawEnforcerVehicles;
	uint8 m_CopsBeatingSuspect;
	int16 m_RoadblockDensity;
	uint8 m_bIgnoredByCops : 1;
	uint8 m_bIgnoredByEveryone : 1;
	uint8 m_bSwatRequired : 1;
	uint8 m_bFbiRequired : 1;
	uint8 m_bArmyRequired : 1;
	int32 m_nWantedLevel;
	CCrimeBeingQd m_aCrimes[16];
	CCopPed *m_pCops[10];

	static int32 &MaximumWantedLevel;
	static int32 &nMaximumWantedLevel;

public:
	void Initialise();
	bool AreSwatRequired();
	bool AreFbiRequired();
	bool AreArmyRequired();
	int32 NumOfHelisRequired();
	void SetWantedLevel(int32);
	void SetWantedLevelNoDrop(int32 level);
	void RegisterCrime(eCrimeType type, const CVector &coors, uint32 id, bool policeDoesntCare);
	void RegisterCrime_Immediately(eCrimeType type, const CVector &coors, uint32 id, bool policeDoesntCare);
	void ClearQdCrimes();
	bool AddCrimeToQ(eCrimeType type, int32 id, const CVector &pos, bool reported, bool policeDoesntCare);
	void ReportCrimeNow(eCrimeType type, const CVector &coors, bool policeDoesntCare);
	void UpdateWantedLevel();
	void Reset();

	bool IsIgnored(void) { return m_bIgnoredByCops || m_bIgnoredByEveryone; }

	static int32 WorkOutPolicePresence(CVector posn, float radius);
	static void SetMaximumWantedLevel(int32 level);
};

static_assert(sizeof(CWanted) == 0x204, "CWanted: error");
