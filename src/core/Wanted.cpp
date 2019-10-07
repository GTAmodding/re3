#include "common.h"
#include "patcher.h"
#include "Pools.h"
#include "ModelIndices.h"
#include "Timer.h"
#include "World.h"
#include "ZoneCull.h"
#include "Darkel.h"
#include "DMAudio.h"
#include "Wanted.h"

int32 &CWanted::MaximumWantedLevel = *(int32*)0x5F7714;	// 6
int32 &CWanted::nMaximumWantedLevel = *(int32*)0x5F7718;	// 6400

WRAPPER void CWanted::Reset() { EAXJMP(0x4AD790) };

void
CWanted::Initialise()
{
	int i;

	m_nChaos = 0;
	m_nLastUpdateTime = 0;
	m_nLastWantedLevelChange = 0;
	m_CurrentCops = 0;
	m_MaxCops = 0;
	m_MaximumLawEnforcerVehicles = 0;
	m_RoadblockDensity = 0;
	m_bIgnoredByCops = false;
	m_bIgnoredByEveryone = false;
	m_bSwatRequired = false;
	m_bFbiRequired = false;
	m_bArmyRequired = false;
	m_fCrimeSensitivity = 1.0f;
	m_nWantedLevel = 0;
        m_CopsBeatingSuspect = 0;
	for(i = 0; i < 10; i++)
		m_pCops[i] = nil;
        ClearQdCrimes();
}

bool
CWanted::AreSwatRequired()
{
	return m_nWantedLevel == 4 || m_bSwatRequired;
}

bool
CWanted::AreFbiRequired()
{
	return m_nWantedLevel == 5 || m_bFbiRequired;
}

bool
CWanted::AreArmyRequired()
{
	return m_nWantedLevel == 6 || m_bArmyRequired;
}

int32
CWanted::NumOfHelisRequired()
{
	if (m_bIgnoredByCops)
		return 0;

	switch (m_nWantedLevel) {
	case 3:
	case 4:
		return 1;
	case 5:
	case 6:
		return 2;
	default:
		return 0;
	}
}

void
CWanted::SetWantedLevel(int32 level)
{
	ClearQdCrimes();
	if (level > MaximumWantedLevel)
		level = MaximumWantedLevel;
	switch (level) {
	case 0:
		m_nChaos = 0;
		break;
	case 1:
		m_nChaos = 60;
		break;
	case 2:
		m_nChaos = 220;
		break;
	case 3:
		m_nChaos = 420;
		break;
	case 4:
		m_nChaos = 820;
		break;
	case 5:
		m_nChaos = 1620;
		break;
	case 6:
		m_nChaos = 3220;
		break;
	default:
		break;
	}
	UpdateWantedLevel();
}

void
CWanted::SetWantedLevelNoDrop(int32 level)
{
	if (level > m_nWantedLevel)
		SetWantedLevel(level);
}

void
CWanted::SetMaximumWantedLevel(int32 level)
{
	switch(level){
	case 0:
		nMaximumWantedLevel = 0;
		MaximumWantedLevel = 0;
		break;
	case 1:
		nMaximumWantedLevel = 120;
		MaximumWantedLevel = 1;
		break;
	case 2:
		nMaximumWantedLevel = 300;
		MaximumWantedLevel = 2;
		break;
	case 3:
		nMaximumWantedLevel = 600;
		MaximumWantedLevel = 3;
		break;
	case 4:
		nMaximumWantedLevel = 1200;
		MaximumWantedLevel = 4;
		break;
	case 5:
		nMaximumWantedLevel = 2400;
		MaximumWantedLevel = 5;
		break;
	case 6:
		nMaximumWantedLevel = 4800;
		MaximumWantedLevel = 6;
		break;
	}
}

void
CWanted::RegisterCrime(eCrimeType type, const CVector &coors, uint32 id, bool policeDoesntCare)
{
	AddCrimeToQ(type, id, coors, false, policeDoesntCare);
}

void
CWanted::RegisterCrime_Immediately(eCrimeType type, const CVector &coors, uint32 id, bool policeDoesntCare)
{
	if(!AddCrimeToQ(type, id, coors, false, policeDoesntCare))
		ReportCrimeNow(type, coors, policeDoesntCare);
}

void
CWanted::ClearQdCrimes()
{
	for (int i = 0; i < 16; i++)
		m_aCrimes[i].m_nType = CRIME_NONE;
}

// returns whether the crime had been reported already
bool
CWanted::AddCrimeToQ(eCrimeType type, int32 id, const CVector &coors, bool reported, bool policeDoesntCare)
{
	int i;

	for(i = 0; i < 16; i++)
		if(m_aCrimes[i].m_nType == type && m_aCrimes[i].m_nId == id){
			if(m_aCrimes[i].m_bReported)
				return true;
			if(reported)
				m_aCrimes[i].m_bReported = reported;
			return false;
		}

	for(i = 0; i < 16; i++)
		if(m_aCrimes[i].m_nType == CRIME_NONE)
			break;
	if(i < 16){
		m_aCrimes[i].m_nType = type;
		m_aCrimes[i].m_nId = id;
		m_aCrimes[i].m_vecPosn = coors;
		m_aCrimes[i].m_nTime = CTimer::GetTimeInMilliseconds();
		m_aCrimes[i].m_bReported = reported;
		m_aCrimes[i].m_bPoliceDoesntCare = policeDoesntCare;
	}
	return false;
}

void
CWanted::ReportCrimeNow(eCrimeType type, const CVector &coors, bool policeDoesntCare)
{
	float sensitivity, chaos;
	int wantedLevelDrop;

	if(CDarkel::FrenzyOnGoing())
		sensitivity = m_fCrimeSensitivity*0.3f;
	else
		sensitivity = m_fCrimeSensitivity;

	wantedLevelDrop = min(CCullZones::GetWantedLevelDrop(), 100);

	chaos = (1.0f - wantedLevelDrop/100.0f) * sensitivity;
	if (policeDoesntCare)
		chaos *= 0.333f;
	switch(type){
	case CRIME_POSSESSION_GUN:
		break;
	case CRIME_HIT_PED:
		m_nChaos += 5.0f*chaos;
		break;
	case CRIME_HIT_COP:
		m_nChaos += 45.0f*chaos;
		break;
	case CRIME_SHOOT_PED:
		m_nChaos += 30.0f*chaos;
		break;
	case CRIME_SHOOT_COP:
		m_nChaos += 80.0f*chaos;
		break;
	case CRIME_STEAL_CAR:
		m_nChaos += 15.0f*chaos;
		break;
	case CRIME_RUN_REDLIGHT:
		m_nChaos += 10.0f*chaos;
		break;
	case CRIME_RECKLESS_DRIVING:
		m_nChaos += 5.0f*chaos;
		break;
	case CRIME_SPEEDING:
		m_nChaos += 5.0f*chaos;
		break;
	case CRIME_RUNOVER_PED:
		m_nChaos += 18.0f*chaos;
		break;
	case CRIME_RUNOVER_COP:
		m_nChaos += 80.0f*chaos;
		break;
	case CRIME_SHOOT_HELI:
		m_nChaos += 400.0f*chaos;
		break;
	case CRIME_PED_BURNED:
		m_nChaos += 20.0f*chaos;
		break;
	case CRIME_COP_BURNED:
		m_nChaos += 80.0f*chaos;
		break;
	case CRIME_VEHICLE_BURNED:
		m_nChaos += 20.0f*chaos;
		break;
	case CRIME_DESTROYED_CESSNA:
		m_nChaos += 500.0f*chaos;
		break;
	default:
	//	Error("Undefined crime type, RegisterCrime, Crime.cpp");	// different file for some reason
		Error("Undefined crime type, RegisterCrime, Wanted.cpp");
	}
	DMAudio.ReportCrime(type, coors);
	UpdateWantedLevel();
}

void
CWanted::UpdateWantedLevel()
{
	int32 CurrWantedLevel = m_nWantedLevel;

	if (m_nChaos > nMaximumWantedLevel)
		m_nChaos = nMaximumWantedLevel;

	if (m_nChaos >= 0 && m_nChaos < 40) {
		m_nWantedLevel = 0;
		m_MaximumLawEnforcerVehicles = 0;
		m_MaxCops = 0;
		m_RoadblockDensity = 0;
	}
	else if (m_nChaos >= 40 && m_nChaos < 200) {
		m_nWantedLevel = 1;
		m_MaximumLawEnforcerVehicles = 1;
		m_MaxCops = 1;
		m_RoadblockDensity = 0;
	}
	else if (m_nChaos >= 200 && m_nChaos < 400) {
		m_nWantedLevel = 2;
		m_MaximumLawEnforcerVehicles = 2;
		m_MaxCops = 3;
		m_RoadblockDensity = 0;
	}
	else if (m_nChaos >= 400 && m_nChaos < 800) {
		m_nWantedLevel = 3;
		m_MaximumLawEnforcerVehicles = 2;
		m_MaxCops = 4;
		m_RoadblockDensity = 4;
	}
	else if (m_nChaos >= 800 && m_nChaos < 1600) {
		m_nWantedLevel = 4;
		m_MaximumLawEnforcerVehicles = 2;
		m_MaxCops = 6;
		m_RoadblockDensity = 8;
	}
	else if (m_nChaos >= 1600 && m_nChaos < 3200) {
		m_nWantedLevel = 5;
		m_MaximumLawEnforcerVehicles = 3;
		m_MaxCops = 8;
		m_RoadblockDensity = 10;
	}
	else if (m_nChaos >= 3200) {
		m_nWantedLevel = 6;
		m_MaximumLawEnforcerVehicles = 3;
		m_MaxCops = 10;
		m_RoadblockDensity = 12;
	}

	if (CurrWantedLevel != m_nWantedLevel)
		m_nLastWantedLevelChange = CTimer::GetTimeInMilliseconds();
}

int32
CWanted::WorkOutPolicePresence(CVector posn, float radius)
{
	int i;
	CPed *ped;
	CVehicle *vehicle;
	int numPolice = 0;

	i = CPools::GetPedPool()->GetSize();
	while(--i >= 0){
		ped = CPools::GetPedPool()->GetSlot(i);
		if(ped &&
		   IsPolicePedModel(ped->GetModelIndex()) &&
		   (posn - ped->GetPosition()).Magnitude() < radius)
			numPolice++;
	}

	i = CPools::GetVehiclePool()->GetSize();
	while(--i >= 0){
		vehicle = CPools::GetVehiclePool()->GetSlot(i);
		if(vehicle &&
		   vehicle->bIsLawEnforcer &&
		   IsPoliceVehicleModel(vehicle->GetModelIndex()) &&
		   vehicle != FindPlayerVehicle() &&
		   vehicle->m_status != STATUS_ABANDONED && vehicle->m_status != STATUS_WRECKED &&
		   (posn - vehicle->GetPosition()).Magnitude() < radius)
			numPolice++;
	}

	return numPolice;
}

STARTPATCHES
	InjectHook(0x4AD6E0, &CWanted::Initialise, PATCH_JUMP);
//	InjectHook(0x4AD790, &CWanted::Reset, PATCH_JUMP);
//	InjectHook(0x4AD7B0, &CWanted::Update, PATCH_JUMP);
	InjectHook(0x4AD900, &CWanted::UpdateWantedLevel, PATCH_JUMP);
	InjectHook(0x4AD9F0, &CWanted::RegisterCrime, PATCH_JUMP);
	InjectHook(0x4ADA10, &CWanted::RegisterCrime_Immediately, PATCH_JUMP);
	InjectHook(0x4ADA50, &CWanted::SetWantedLevel, PATCH_JUMP);
	InjectHook(0x4ADAC0, &CWanted::SetWantedLevelNoDrop, PATCH_JUMP);
	InjectHook(0x4ADAE0, &CWanted::SetMaximumWantedLevel, PATCH_JUMP);
	InjectHook(0x4ADBA0, &CWanted::AreSwatRequired, PATCH_JUMP);
	InjectHook(0x4ADBC0, &CWanted::AreFbiRequired, PATCH_JUMP);
	InjectHook(0x4ADBE0, &CWanted::AreArmyRequired, PATCH_JUMP);
	InjectHook(0x4ADC00, &CWanted::NumOfHelisRequired, PATCH_JUMP);
//	InjectHook(0x4ADC40, &CWanted::ResetPolicePursuit, PATCH_JUMP);
	InjectHook(0x4ADD00, &CWanted::WorkOutPolicePresence, PATCH_JUMP);
	InjectHook(0x4ADF20, &CWanted::ClearQdCrimes, PATCH_JUMP);
	InjectHook(0x4ADFD0, &CWanted::AddCrimeToQ, PATCH_JUMP);
//	InjectHook(0x4AE090, &CWanted::UpdateCrimesQ, PATCH_JUMP);
	InjectHook(0x4AE110, &CWanted::ReportCrimeNow, PATCH_JUMP);
ENDPATCHES
