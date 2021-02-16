#pragma once

#include "Game.h"
#include "PedType.h"

class CPed;
class CVehicle;
class CDummyObject;
class CObject;

struct PedGroup
{
	int32 models[NUMMODELSPERPEDGROUP];
};

// Don't know the original name
struct RegenerationPoint
{
	eLevelName srcLevel; // this and below one may need to be exchanged
	eLevelName destLevel;
	float x1;
	float x2;
	float y1;
	float y2;
	float z1;
	float z2;
	RwV3d destPosA;
	RwV3d destPosB;
	RwV3d srcPosA;
	RwV3d srcPosB;
};

class CPopulation
{
public:
	static PedGroup ms_pPedGroups[NUMPEDGROUPS];
	static bool ms_bGivePedsWeapons;
	static int32 m_AllRandomPedsThisType;
	static float PedDensityMultiplier;
	static uint32 ms_nTotalMissionPeds;
	static int32 MaxNumberOfPedsInUse;
	static uint32 ms_nNumCivMale;
	static uint32 ms_nNumCivFemale;
	static uint32 ms_nNumCop;
	static bool bZoneChangeHasHappened;
	static uint32 ms_nNumEmergency;
	static int8 m_CountDownToPedsAtStart;
	static uint32 ms_nNumGang1;
	static uint32 ms_nNumGang2;
	static uint32 ms_nTotalPeds;
	static uint32 ms_nNumGang3;
	static uint32 ms_nTotalGangPeds;
	static uint32 ms_nNumGang4;
	static uint32 ms_nTotalCivPeds;
	static uint32 ms_nNumGang5;
	static uint32 ms_nNumDummy;
	static uint32 ms_nNumGang6;
	static uint32 ms_nNumGang9;
	static uint32 ms_nNumGang7;
	static uint32 ms_nNumGang8;
	static CVector RegenerationPoint_a;
	static CVector RegenerationPoint_b;
	static CVector RegenerationFront;

	static void Initialise();
	static void Update(void);
	static void LoadPedGroups();
	static void UpdatePedCount(ePedType, bool);
	static void DealWithZoneChange(eLevelName oldLevel, eLevelName newLevel, bool);
	static CPed *AddPedInCar(CVehicle *car);
	static bool IsPointInSafeZone(CVector *coors);
	static void RemovePed(CPed *ent);
	static int32 ChooseCivilianOccupation(int32);
	static int32 ChoosePolicePedOccupation();
	static int32 ChooseGangOccupation(int);
	static void FindCollisionZoneForCoors(CVector*, int*, eLevelName*);
	static void FindClosestZoneForCoors(CVector*, int*, eLevelName, eLevelName);
	static void GeneratePedsAtStartOfGame();
	static float PedCreationDistMultiplier();
	static CPed *AddPed(ePedType pedType, uint32 mi, CVector const &coors);
	static void AddToPopulation(float, float, float, float);
	static void ManagePopulation(void);
	static void MoveCarsAndPedsOutOfAbandonedZones(void);
	static void ConvertToRealObject(CDummyObject*);
	static void ConvertToDummyObject(CObject*);
	static void ConvertAllObjectsToDummyObjects(void);
	static bool TestRoomForDummyObject(CObject*);
	static bool TestSafeForRealObject(CDummyObject*);
};
