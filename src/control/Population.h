#pragma once

#include "Game.h"

class CPed;
class CVehicle;

struct PedGroup
{
	int32 models[8];
};

class CPopulation
{
public:
	static PedGroup *ms_pPedGroups;	//[31]
	static bool &ms_bGivePedsWeapons;
	static int32 &m_AllRandomPedsThisType;
	static float &PedDensityMultiplier;
	static uint32 &ms_nTotalMissionPeds;
	static int32 &MaxNumberOfPedsInUse;
	static uint32& ms_nNumCivMale;
	static uint32 &ms_nNumCivFemale;
	static uint32 &ms_nNumCop;
	static bool &bZoneChangeHasHappened;
	static uint32 &ms_nNumEmergency;
	static uint32& m_CountDownToPedsAtStart;
	static uint32& ms_nNumGang1;
	static uint32& ms_nNumGang2;
	static uint32& ms_nTotalPeds;
	static uint32& ms_nNumGang3;
	static uint32& ms_nTotalGangPeds;
	static uint32& ms_nNumGang4;
	static uint32& ms_nTotalCivPeds;
	static uint32& ms_nNumGang5;
	static uint32& ms_nNumDummy;
	static uint32& ms_nNumGang6;
	static uint32& ms_nNumGang9;
	static uint32& ms_nNumGang7;
	static uint32& ms_nNumGang8;

	static void Initialise();
	static void Update(void);
	static void LoadPedGroups();
	static void UpdatePedCount(uint32, bool);
	static void DealWithZoneChange(eLevelName oldLevel, eLevelName newLevel, bool);
	static CPed *AddPedInCar(CVehicle *vehicle);
	static bool IsPointInSafeZone(CVector *coors);
	static void RemovePed(CEntity* ent);
	static int32 ChooseCivilianOccupation(int32);
};
