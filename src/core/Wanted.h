#pragma once

#include "Crime.h"

class CEntity;
class CCopPed;

class CWanted
{
public:
	int32 m_nChaos;
	int32 m_nLastUpdateTime;
	uint32 m_nLastWantedLevelChange;
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

	static int32 MaximumWantedLevel;
	static int32 nMaximumWantedLevel;

public:
	void Initialise();
	bool AreSwatRequired();
	bool AreFbiRequired();
	bool AreArmyRequired();
	int32 NumOfHelisRequired();
	void SetWantedLevel(int32);
	void SetWantedLevelNoDrop(int32 level);
	int32 GetWantedLevel() { return m_nWantedLevel; }
	void RegisterCrime(eCrimeType type, const CVector &coors, uint32 id, bool policeDoesntCare);
	void RegisterCrime_Immediately(eCrimeType type, const CVector &coors, uint32 id, bool policeDoesntCare);
	void ClearQdCrimes();
	bool AddCrimeToQ(eCrimeType type, int32 id, const CVector &pos, bool reported, bool policeDoesntCare);
	void ReportCrimeNow(eCrimeType type, const CVector &coors, bool policeDoesntCare);
	void UpdateWantedLevel();
	void Reset();
	void ResetPolicePursuit();
	void UpdateCrimesQ();
	void Update();

	bool IsIgnored(void) { return m_bIgnoredByCops || m_bIgnoredByEveryone; }

	static int32 WorkOutPolicePresence(CVector posn, float radius);
	static void SetMaximumWantedLevel(int32 level);
};

VALIDATE_SIZE(CWanted, 0x204);
