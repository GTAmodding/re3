#include "common.h"
#include "patcher.h"
#include "Game.h"
#include "General.h"
#include "World.h"
#include "Entity.h"
#include "Population.h"

PedGroup *CPopulation::ms_pPedGroups = (PedGroup*)0x6E9248;
bool &CPopulation::ms_bGivePedsWeapons = *(bool*)0x95CCF6;
int32 &CPopulation::m_AllRandomPedsThisType = *(int32*)0x5FA570;
float &CPopulation::PedDensityMultiplier = *(float*)0x5FA56C;
uint32 &CPopulation::ms_nTotalMissionPeds = *(uint32*)0x8F5F70;
int32 &CPopulation::MaxNumberOfPedsInUse = *(int32*)0x5FA574;
uint32& CPopulation::ms_nNumCivMale = *(uint32*)0x8F2548;
uint32& CPopulation::ms_nNumCivFemale = *(uint32*)0x8F5F44;
uint32& CPopulation::ms_nNumCop = *(uint32*)0x885AFC;
bool& CPopulation::bZoneChangeHasHappened = *(bool*)0x95CD79;
uint32& CPopulation::ms_nNumEmergency = *(uint32*)0x94071C;
uint32& CPopulation::m_CountDownToPedsAtStart = *(uint32*)0x95CD4F;
uint32& CPopulation::ms_nNumGang1 = *(uint32*)0x8F1B1C;
uint32& CPopulation::ms_nNumGang2 = *(uint32*)0x8F1B14;
uint32& CPopulation::ms_nTotalPeds = *(uint32*)0x95CB50;
uint32& CPopulation::ms_nNumGang3 = *(uint32*)0x8F2548;
uint32& CPopulation::ms_nTotalGangPeds = *(uint32*)0x885AF0;
uint32& CPopulation::ms_nNumGang4 = *(uint32*)0x8F1B2C;
uint32& CPopulation::ms_nTotalCivPeds = *(uint32*)0x8F2C3C;
uint32& CPopulation::ms_nNumGang5 = *(uint32*)0x8F1B30;
uint32& CPopulation::ms_nNumDummy = *(uint32*)0x8F1A98;
uint32& CPopulation::ms_nNumGang6 = *(uint32*)0x8F1B20;
uint32& CPopulation::ms_nNumGang9 = *(uint32*)0x8F1B10;
uint32& CPopulation::ms_nNumGang7 = *(uint32*)0x8F1B28;
uint32& CPopulation::ms_nNumGang8 = *(uint32*)0x8F1B0C;

WRAPPER void CPopulation::Update(void) { EAXJMP(0x4F39A0); }
WRAPPER void CPopulation::LoadPedGroups() { EAXJMP(0x4F3870); }
WRAPPER void CPopulation::UpdatePedCount(uint32, bool) { EAXJMP(0x4F5A60); }
WRAPPER void CPopulation::DealWithZoneChange(eLevelName oldLevel, eLevelName newLevel, bool) { EAXJMP(0x4F6200); }
WRAPPER CPed *CPopulation::AddPedInCar(CVehicle *vehicle) { EAXJMP(0x4F5800); }
WRAPPER bool CPopulation::IsPointInSafeZone(CVector *coors) { EAXJMP(0x4F60C0); }

void
CPopulation::Initialise()
{
	debug("Initialising CPopulation...\n");

	ms_nNumCivMale = 0;
	m_AllRandomPedsThisType = -1;
	ms_nNumCivFemale = 0;
	PedDensityMultiplier = 1.0;
	ms_nNumCop = 0;
	bZoneChangeHasHappened = 0;
	ms_nNumEmergency = 0;
	m_CountDownToPedsAtStart = 2;
	ms_nNumGang1 = 0;
	ms_nTotalMissionPeds = 0;
	ms_nNumGang2 = 0;
	ms_nTotalPeds = 0;
	ms_nNumGang3 = 0;
	ms_nTotalGangPeds = 0;
	ms_nNumGang4 = 0;
	ms_nTotalCivPeds = 0;
	ms_nNumGang5 = 0;
	ms_nNumDummy = 0;
	ms_nNumGang6 = 0;
	ms_nNumGang9 = 0;
	ms_nNumGang7 = 0;
	ms_nNumGang8 = 0;

	LoadPedGroups();
	DealWithZoneChange(LEVEL_COMMERCIAL, LEVEL_INDUSTRIAL, true);

	debug("CPopulation ready\n");
}

void
CPopulation::RemovePed(CEntity* ent)
{
	CWorld::Remove(ent);
	delete ent;
}

int32
CPopulation::ChooseCivilianOccupation(int32 group)
{
	return ms_pPedGroups[group].models[CGeneral::GetRandomNumberInRange(0, 8)];
}

STARTPATCHES
InjectHook(0x4F3770, CPopulation::Initialise, PATCH_JUMP);
ENDPATCHES