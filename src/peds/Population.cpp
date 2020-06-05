#include "common.h"

#include "Game.h"
#include "General.h"
#include "World.h"
#include "Population.h"
#include "CopPed.h"
#include "Wanted.h"
#include "FileMgr.h"
#include "Gangs.h"
#include "ModelIndices.h"
#include "Zones.h"
#include "CivilianPed.h"
#include "EmergencyPed.h"
#include "Replay.h"
#include "Camera.h"
#include "CutsceneMgr.h"
#include "CarCtrl.h"
#include "IniFile.h"
#include "VisibilityPlugins.h"
#include "PedPlacement.h"
#include "DummyObject.h"
#include "Script.h"
#include "Shadows.h"
#include "SurfaceTable.h"

#define MIN_CREATION_DIST		40.0f // not for start of the game (look at the GeneratePedsAtStartOfGame)
#define CREATION_RANGE			10.0f // added over the MIN_CREATION_DIST.
#define OFFSCREEN_CREATION_MULT	0.5f
#define PED_REMOVE_DIST			(MIN_CREATION_DIST + CREATION_RANGE + 1.0f)
#define PED_REMOVE_DIST_SPECIAL	(MIN_CREATION_DIST + CREATION_RANGE + 15.0f) // for peds with bCullExtraFarAway flag

// Transition areas between zones
const RegenerationPoint aSafeZones[] = {
// TODO(MIAMI): this is totally bogus
	{ LEVEL_BEACH, LEVEL_MAINLAND, 400.0f, 814.0f, -954.0f, -903.0f, 30.0f, 100.0f,
		CVector(790.0f, -917.0f, 39.0f), CVector(775.0f, -921.0f, 39.0f), CVector(424.0f, -942.0f, 38.0f), CVector(439.0f, -938.0f, 38.0f) },
#ifndef MIAMI
	{ LEVEL_INDUSTRIAL, LEVEL_COMMERCIAL, 400.0f, 814.0f, -954.0f, -903.0f, 30.0f, 100.0f,
		CVector(790.0f, -917.0f, 39.0f), CVector(775.0f, -921.0f, 39.0f), CVector(424.0f, -942.0f, 38.0f), CVector(439.0f, -938.0f, 38.0f) },
	{ LEVEL_INDUSTRIAL, LEVEL_COMMERCIAL, 555.0f, 711.0f, 118.0f, 186.0f, -30.0f, -10.0f,
		CVector(698.0f,  182.0f, -20.0f), CVector(681.0f,  178.0f, -20.0f), CVector(586.0f,  144.0f, -20.0f), CVector(577.0f,  135.0f, -20.0f) },
	{ LEVEL_INDUSTRIAL, LEVEL_COMMERCIAL, 626.0f, 744.0f, -124.0f, -87.0f, -20.0f, -6.0f,
		CVector(736.0f, -117.0f, -13.0f), CVector(730.0f, -115.0f, -13.0f), CVector(635.0f, -93.0f, -12.5f), CVector(650.0f, -89.0f, -12.5f) },
	{ LEVEL_INDUSTRIAL, LEVEL_COMMERCIAL, 645.0f, 734.0f, -780.0f, -750.0f, -25.0f, -6.0f,
		CVector(729.0f, -764.0f, -18.0f), CVector(720.0f, -769.0f, -17.0f), CVector(652.0f, -774.0f, -10.5f), CVector(659.0f, -770.0f, -10.5f) },
	{ LEVEL_COMMERCIAL, LEVEL_SUBURBAN, -532.0f, -136.0f, -668.0f, -599.0f, 34.0f, 60.0f,
		CVector(-172.0f, -619.0f,  44.0f), CVector(-183.0f, -623.0f,  44.0f), CVector(-511.0f, -645.0f,  41.0f), CVector(-493.0f, -639.0f,  41.5f) },
	{ LEVEL_COMMERCIAL, LEVEL_SUBURBAN, -325.0f, -175.0f, 27.0f, 75.0f, -30.0f, -10.0f,
		CVector(-185.0f,  40.8f, -20.5f), CVector(-202.0f,  37.0f, -20.5f), CVector(-315.0f,  65.5f, -20.5f), CVector(-306.0f,  62.4f, -20.5f) },
	{ LEVEL_COMMERCIAL, LEVEL_SUBURBAN, -410.0f, -310.0f, -1055.0f, -1030.0f, -20.0f, -6.0f,
		CVector(-321.0f, -1043.0f, -13.2f), CVector(-328.0f, -1045.0f, -13.2f), CVector(-398.0f, -1044.0f, -13.5f), CVector(-390.0f, -1040.5f, -13.5f) },
	{ LEVEL_COMMERCIAL, LEVEL_SUBURBAN, -425.0f, -280.0f, -471.0f, -447.0f, -20.0f, -5.0f,
		CVector(-292.0f, -457.0f, -11.6f), CVector(-310.0f, -461.0f, -11.6f), CVector(-413.0f, -461.0f, -11.5f), CVector(-399.0f, -457.0f, -11.3f) }
#endif
};

PedGroup CPopulation::ms_pPedGroups[NUMPEDGROUPS];
bool CPopulation::ms_bGivePedsWeapons;
int32 CPopulation::m_AllRandomPedsThisType = -1;
float CPopulation::PedDensityMultiplier = 1.0f;
uint32 CPopulation::ms_nTotalMissionPeds;
int32 CPopulation::MaxNumberOfPedsInUse = 25;
uint32 CPopulation::ms_nNumCivMale;
uint32 CPopulation::ms_nNumCivFemale;
uint32 CPopulation::ms_nNumCop;
bool CPopulation::bZoneChangeHasHappened;
uint32 CPopulation::ms_nNumEmergency;
int8 CPopulation::m_CountDownToPedsAtStart;
uint32 CPopulation::ms_nNumGang1;
uint32 CPopulation::ms_nNumGang2;
uint32 CPopulation::ms_nTotalPeds;
uint32 CPopulation::ms_nNumGang3;
uint32 CPopulation::ms_nTotalGangPeds;
uint32 CPopulation::ms_nNumGang4;
uint32 CPopulation::ms_nTotalCivPeds;
uint32 CPopulation::ms_nNumGang5;
uint32 CPopulation::ms_nNumDummy;
uint32 CPopulation::ms_nNumGang6;
uint32 CPopulation::ms_nNumGang9;
uint32 CPopulation::ms_nNumGang7;
uint32 CPopulation::ms_nNumGang8;
CVector CPopulation::RegenerationPoint_a;
CVector CPopulation::RegenerationPoint_b;
CVector CPopulation::RegenerationForward;

uint32 CPopulation::ms_nTotalCarPassengerPeds;
uint32 CPopulation::NumMiamiViceCops;

void
CPopulation::Initialise()
{
	debug("Initialising CPopulation...\n");

	ms_nNumCivMale = 0;
	ms_nNumCivFemale = 0;
	ms_nNumCop = 0;
	ms_nNumEmergency = 0;
	ms_nNumGang1 = 0;
	ms_nNumGang2 = 0;
	ms_nNumGang3 = 0;
	ms_nNumGang4 = 0;
	ms_nNumGang5 = 0;
	ms_nNumGang6 = 0;
	ms_nNumGang7 = 0;
	ms_nNumGang8 = 0;
	ms_nNumGang9 = 0;
	ms_nNumDummy = 0;

	ms_nTotalCarPassengerPeds = 0;

	m_AllRandomPedsThisType = -1;
	PedDensityMultiplier = 1.0f;
	bZoneChangeHasHappened = false;
	m_CountDownToPedsAtStart = 2;
	
	ms_nTotalMissionPeds = 0;
	ms_nTotalPeds = 0;
	ms_nTotalGangPeds = 0;
	ms_nTotalCivPeds = 0;

	LoadPedGroups();

	debug("CPopulation ready\n");
}

void
CPopulation::RemovePed(CPed *ent)
{
	// CPed dtor already does that
	// CWorld::Remove((CEntity*)ent);
	delete ent;
}

int32
CPopulation::ChooseCivilianOccupation(int32 group)
{
	return ms_pPedGroups[group].models[CGeneral::GetRandomNumberInRange(0, NUMMODELSPERPEDGROUP)];
}

// returns eCopType
int32
CPopulation::ChoosePolicePedOccupation()
{
	CGeneral::GetRandomNumber();
	return COP_STREET;
}

void
CPopulation::LoadPedGroups()
{
	int fd;
	char line[1024];
	int nextPedGroup = 0;
	// char unused[16]; // non-existence of that in mobile kinda verifies that
	char modelName[256];

	CFileMgr::ChangeDir("\\DATA\\");
	fd = CFileMgr::OpenFile("PEDGRP.DAT", "r");
	CFileMgr::ChangeDir("\\");
	while (CFileMgr::ReadLine(fd, line, sizeof(line))) {
		int end;
		// find end of line
		for (end = 0; ; end++) {
			if (line[end] == '\n')
				break;
			if (line[end] == ',' || line[end] == '\r')
				line[end] = ' ';
		}
		line[end] = '\0';
		int cursor = 0;
		int i;
		for (i = 0; i < NUMMODELSPERPEDGROUP; i++) {
			while (line[cursor] <= ' ' && line[cursor] != '\0')
				++cursor;

			if (line[cursor] == '#')
				break;

			// find next whitespace
			int nextWhitespace;
			for (nextWhitespace = cursor; line[nextWhitespace] > ' '; ++nextWhitespace)
				;

			if (cursor == nextWhitespace)
				break;
			
			// read until next whitespace
			strncpy(modelName, &line[cursor], nextWhitespace - cursor);
			modelName[nextWhitespace - cursor] = '\0';
			CModelInfo::GetModelInfo(modelName, &ms_pPedGroups[nextPedGroup].models[i]);
			cursor = nextWhitespace;
		}
		if (i == NUMMODELSPERPEDGROUP)
			nextPedGroup++;
	}
	CFileMgr::CloseFile(fd);
}

void
CPopulation::UpdatePedCount(ePedType pedType, bool decrease)
{
	if (decrease) {
		switch (pedType) {
			case PEDTYPE_PLAYER1:
			case PEDTYPE_PLAYER2:
			case PEDTYPE_PLAYER3:
			case PEDTYPE_PLAYER4:
			case PEDTYPE_UNUSED1:
			case PEDTYPE_SPECIAL:
				return;
			case PEDTYPE_CIVMALE:
				--ms_nNumCivMale;
				break;
			case PEDTYPE_CIVFEMALE:
				--ms_nNumCivFemale;
				break;
			case PEDTYPE_COP:
				--ms_nNumCop;
				break;
			case PEDTYPE_GANG1:
				--ms_nNumGang1;
				break;
			case PEDTYPE_GANG2:
				--ms_nNumGang2;
				break;
			case PEDTYPE_GANG3:
				--ms_nNumGang3;
				break;
			case PEDTYPE_GANG4:
				--ms_nNumGang4;
				break;
			case PEDTYPE_GANG5:
				--ms_nNumGang5;
				break;
			case PEDTYPE_GANG6:
				--ms_nNumGang6;
				break;
			case PEDTYPE_GANG7:
				--ms_nNumGang7;
				break;
			case PEDTYPE_GANG8:
				--ms_nNumGang8;
				break;
			case PEDTYPE_GANG9:
				--ms_nNumGang9;
				break;
			case PEDTYPE_EMERGENCY:
			case PEDTYPE_FIREMAN:
				--ms_nNumEmergency;
				break;
			case PEDTYPE_CRIMINAL:
				--ms_nNumCivMale;
				break;
			case PEDTYPE_PROSTITUTE:
				--ms_nNumCivFemale;
				break;
			case PEDTYPE_UNUSED2:
				--ms_nNumDummy;
				break;
			default:
				Error("Unknown ped type, UpdatePedCount, Population.cpp");
				break;
		}
	} else {
		switch (pedType) {
			case PEDTYPE_PLAYER1:
			case PEDTYPE_PLAYER2:
			case PEDTYPE_PLAYER3:
			case PEDTYPE_PLAYER4:
			case PEDTYPE_UNUSED1:
			case PEDTYPE_SPECIAL:
				return;
			case PEDTYPE_CIVMALE:
				++ms_nNumCivMale;
				break;
			case PEDTYPE_CIVFEMALE:
				++ms_nNumCivFemale;
				break;
			case PEDTYPE_COP:
				++ms_nNumCop;
				break;
			case PEDTYPE_GANG1:
				++ms_nNumGang1;
				break;
			case PEDTYPE_GANG2:
				++ms_nNumGang2;
				break;
			case PEDTYPE_GANG3:
				++ms_nNumGang3;
				break;
			case PEDTYPE_GANG4:
				++ms_nNumGang4;
				break;
			case PEDTYPE_GANG5:
				++ms_nNumGang5;
				break;
			case PEDTYPE_GANG6:
				++ms_nNumGang6;
				break;
			case PEDTYPE_GANG7:
				++ms_nNumGang7;
				break;
			case PEDTYPE_GANG8:
				++ms_nNumGang8;
				break;
			case PEDTYPE_GANG9:
				++ms_nNumGang9;
				break;
			case PEDTYPE_EMERGENCY:
			case PEDTYPE_FIREMAN:
				++ms_nNumEmergency;
				break;
			case PEDTYPE_CRIMINAL:
				++ms_nNumCivMale;
				break;
			case PEDTYPE_PROSTITUTE:
				++ms_nNumCivFemale;
				break;
			case PEDTYPE_UNUSED2:
				++ms_nNumDummy;
				break;
			default:
				Error("Unknown ped type, UpdatePedCount, Population.cpp");
				break;
		}
	}
}

int
CPopulation::ChooseGangOccupation(int gangId)
{
	return CGangs::ChooseGangPedModel(gangId);
}

//--MIAMI: done
void
CPopulation::DealWithZoneChange(eLevelName oldLevel, eLevelName newLevel, bool forceIndustrialZone)
{
}

void
CPopulation::FindCollisionZoneForCoors(CVector *coors, int *safeZoneOut, eLevelName *levelOut)
{
	*safeZoneOut = -1;
	for (int i = 0; i < ARRAY_SIZE(aSafeZones); i++) {
		if (coors->x > aSafeZones[i].x1 && coors->x < aSafeZones[i].x2) {
			if (coors->y > aSafeZones[i].y1 && coors->y < aSafeZones[i].y2) {
				if (coors->z > aSafeZones[i].z1 && coors->z < aSafeZones[i].z2)
					*safeZoneOut = i;
			}
		}
	}
	// Then it's transition area
	if (*safeZoneOut >= 0)
		*levelOut = LEVEL_NONE;
	else
		*levelOut = CTheZones::GetLevelFromPosition(coors);
}

void
CPopulation::FindClosestZoneForCoors(CVector *coors, int *safeZoneOut, eLevelName level1, eLevelName level2)
{
	float minDist = 10000000.0f;
	int closestSafeZone = -1;
	for (int i = 0; i < ARRAY_SIZE(aSafeZones); i++) {
		if ((level1 == aSafeZones[i].srcLevel || level1 == aSafeZones[i].destLevel) && (level2 == aSafeZones[i].srcLevel || level2 == aSafeZones[i].destLevel)) {
			CVector2D safeZoneDistVec(coors->x - (aSafeZones[i].x1 + aSafeZones[i].x2) * 0.5f, coors->y - (aSafeZones[i].y1 + aSafeZones[i].y2) * 0.5f);
			float safeZoneDist = safeZoneDistVec.Magnitude();
			if (safeZoneDist < minDist) {
				minDist = safeZoneDist;
				closestSafeZone = i;
			}
		}
	}
	*safeZoneOut = closestSafeZone;
}

void
CPopulation::Update()
{
	if (!CReplay::IsPlayingBack()) {
		ManagePopulation();
		MoveCarsAndPedsOutOfAbandonedZones();
		if (m_CountDownToPedsAtStart != 0) {
			if (--m_CountDownToPedsAtStart == 0)
				GeneratePedsAtStartOfGame();
		} else {
			ms_nTotalCivPeds = ms_nNumCivFemale + ms_nNumCivMale;
			ms_nTotalGangPeds = ms_nNumGang9 + ms_nNumGang8 + ms_nNumGang7
				+ ms_nNumGang6 + ms_nNumGang5 + ms_nNumGang4 + ms_nNumGang3
				+ ms_nNumGang2 + ms_nNumGang1;
			ms_nTotalPeds = ms_nNumDummy + ms_nNumEmergency + ms_nNumCop
				+ ms_nTotalGangPeds + ms_nNumCivFemale + ms_nNumCivMale;
			ms_nTotalPeds -= ms_nTotalCarPassengerPeds;
			if (!CCutsceneMgr::IsRunning()) {
				float pcdm = PedCreationDistMultiplier();
				AddToPopulation(pcdm * (MIN_CREATION_DIST * TheCamera.GenerationDistMultiplier),
					pcdm * ((MIN_CREATION_DIST + CREATION_RANGE) * TheCamera.GenerationDistMultiplier),
					pcdm * (MIN_CREATION_DIST + CREATION_RANGE) * OFFSCREEN_CREATION_MULT - CREATION_RANGE,
					pcdm * (MIN_CREATION_DIST + CREATION_RANGE) * OFFSCREEN_CREATION_MULT);
			}
		}
	}
}

void
CPopulation::GeneratePedsAtStartOfGame()
{
	for (int i = 0; i < 50; i++) {
		ms_nTotalCivPeds = ms_nNumCivFemale + ms_nNumCivMale;
		ms_nTotalGangPeds = ms_nNumGang9 + ms_nNumGang8 + ms_nNumGang7
			+ ms_nNumGang6 + ms_nNumGang5 + ms_nNumGang4
			+ ms_nNumGang3 + ms_nNumGang2 + ms_nNumGang1;
		ms_nTotalPeds = ms_nNumDummy + ms_nNumEmergency + ms_nNumCop
			+ ms_nTotalGangPeds + ms_nNumCivFemale + ms_nNumCivMale;
		ms_nTotalPeds -= ms_nTotalCarPassengerPeds;

		// Min dist is 10.0f only for start of the game (naturally)
		AddToPopulation(10.0f, PedCreationDistMultiplier() * (MIN_CREATION_DIST + CREATION_RANGE),
			10.0f, PedCreationDistMultiplier() * (MIN_CREATION_DIST + CREATION_RANGE));
	}
}

bool
CPopulation::IsPointInSafeZone(CVector *coors)
{
	for (int i = 0; i < ARRAY_SIZE(aSafeZones); i++) {
		if (coors->x > aSafeZones[i].x1 && coors->x < aSafeZones[i].x2) {
			if (coors->y > aSafeZones[i].y1 && coors->y < aSafeZones[i].y2) {
				if (coors->z > aSafeZones[i].z1 && coors->z < aSafeZones[i].z2)
					return true;
			}
		}
	}
	return false;
}

// More speed = wider area to spawn peds
float
CPopulation::PedCreationDistMultiplier()
{
	CVehicle *veh = FindPlayerVehicle();
	if (!veh)
		return 1.0f;

	float vehSpeed = veh->m_vecMoveSpeed.Magnitude2D();
	return clamp(vehSpeed - 0.1f + 1.0f, 1.0f, 1.5f);
}

CPed*
CPopulation::AddPed(ePedType pedType, uint32 miOrCopType, CVector const &coors, int32 modifier)
{
	switch (pedType) {
		case PEDTYPE_CIVMALE:
		case PEDTYPE_CIVFEMALE:
		{
			CCivilianPed *ped = new CCivilianPed(pedType, miOrCopType);
			ped->SetPosition(coors);
			ped->SetOrientation(0.0f, 0.0f, 0.0f);
			CWorld::Add(ped);
			if (ms_bGivePedsWeapons) {
				eWeaponType weapon;

				switch (CGeneral::GetRandomNumber() & 3) {
					case 0:
						weapon = WEAPONTYPE_COLT45;
						break;
					case 1:
						weapon = WEAPONTYPE_NIGHTSTICK;
						break;
					case 2:
						weapon = WEAPONTYPE_GOLFCLUB;
						break;
					case 3:
						weapon = WEAPONTYPE_TEC9;
						break;
					default:
						break;
				}
				if (weapon != WEAPONTYPE_UNARMED) {
					ped->GiveDelayedWeapon(weapon, 25001);
					ped->SetCurrentWeapon(CWeaponInfo::GetWeaponInfo(weapon)->m_nWeaponSlot);
				}
			}
			return ped;
		}
		case PEDTYPE_COP:
		{
			CCopPed *ped = new CCopPed((eCopType)miOrCopType, modifier);
			ped->SetPosition(coors);
			ped->SetOrientation(0.0f, 0.0f, 0.0f);
			CWorld::Add(ped);
			return ped;
		}
		case PEDTYPE_GANG1:
		case PEDTYPE_GANG2:
		case PEDTYPE_GANG3:
		case PEDTYPE_GANG4:
		case PEDTYPE_GANG5:
		case PEDTYPE_GANG6:
		case PEDTYPE_GANG7:
		case PEDTYPE_GANG8:
		case PEDTYPE_GANG9:
		{
			CCivilianPed *ped = new CCivilianPed(pedType, miOrCopType);
			ped->SetPosition(coors);
			ped->SetOrientation(0.0f, 0.0f, 0.0f);
			CWorld::Add(ped);

			eWeaponType weapon;
			if (CGeneral::GetRandomNumberInRange(0, 100) >= 50)
				weapon = (eWeaponType)CGangs::GetGangInfo(pedType - PEDTYPE_GANG1)->m_Weapon2;
			else
				weapon = (eWeaponType)CGangs::GetGangInfo(pedType - PEDTYPE_GANG1)->m_Weapon1;

			ped->GiveDelayedWeapon(weapon, 25001);
			ped->SetCurrentWeapon(CWeaponInfo::GetWeaponInfo(weapon)->m_nWeaponSlot);
			return ped;
		}
		case PEDTYPE_EMERGENCY:
		{
			CEmergencyPed *ped = new CEmergencyPed(PEDTYPE_EMERGENCY);
		    ped->SetPosition(coors);
			ped->SetOrientation(0.0f, 0.0f, 0.0f);
			CWorld::Add(ped);
			return ped;
		}
		case PEDTYPE_FIREMAN:
		{
			CEmergencyPed *ped = new CEmergencyPed(PEDTYPE_FIREMAN);
		    ped->SetPosition(coors);
			ped->SetOrientation(0.0f, 0.0f, 0.0f);
			CWorld::Add(ped);
			return ped;
		}
		case PEDTYPE_CRIMINAL:
		case PEDTYPE_PROSTITUTE:
		{
			CCivilianPed *ped = new CCivilianPed(pedType, miOrCopType);
			ped->SetPosition(coors);
			ped->SetOrientation(0.0f, 0.0f, 0.0f);
			CWorld::Add(ped);
			return ped;
		}
		default:
			Error("Unknown ped type, AddPed, Population.cpp");
			return nil;
	}
}

void
CPopulation::AddToPopulation(float minDist, float maxDist, float minDistOffScreen, float maxDistOffScreen)
{
	uint32 pedTypeToAdd;
	int32 modelToAdd;
	int pedAmount;

	CZoneInfo zoneInfo;
	CPed *gangLeader = nil;
	bool addCop = false;
	CPlayerInfo *playerInfo = &CWorld::Players[CWorld::PlayerInFocus];
	CVector playerCentreOfWorld = FindPlayerCentreOfWorld(CWorld::PlayerInFocus);
	CTheZones::GetZoneInfoForTimeOfDay(&playerCentreOfWorld, &zoneInfo);
	CWanted *wantedInfo = playerInfo->m_pPed->m_pWanted;
	if (wantedInfo->m_nWantedLevel > 2) {
		if (ms_nNumCop < wantedInfo->m_MaxCops && !playerInfo->m_pPed->bInVehicle
			&& (CCarCtrl::NumLawEnforcerCars >= wantedInfo->m_MaximumLawEnforcerVehicles
				|| CCarCtrl::NumRandomCars >= playerInfo->m_nTrafficMultiplier * CCarCtrl::CarDensityMultiplier
				|| CCarCtrl::NumFiretrucksOnDuty + CCarCtrl::NumAmbulancesOnDuty + CCarCtrl::NumParkedCars
				+ CCarCtrl::NumMissionCars + CCarCtrl::NumLawEnforcerCars + CCarCtrl::NumRandomCars >= CCarCtrl::MaxNumberOfCarsInUse)) {
			addCop = true;
			minDist = PedCreationDistMultiplier() * MIN_CREATION_DIST;
			maxDist = PedCreationDistMultiplier() * (MIN_CREATION_DIST + CREATION_RANGE);
		}
	}
	// Yeah, float
	float maxPossiblePedsForArea = (zoneInfo.pedDensity + zoneInfo.carDensity) * playerInfo->m_fRoadDensity * PedDensityMultiplier * CIniFile::PedNumberMultiplier;
	maxPossiblePedsForArea = Min(maxPossiblePedsForArea, MaxNumberOfPedsInUse);

	if (ms_nTotalPeds < maxPossiblePedsForArea || addCop) {
		int decisionThreshold = CGeneral::GetRandomNumberInRange(0, 1000);
		if (decisionThreshold < zoneInfo.copPedThreshold || addCop) {
			pedTypeToAdd = PEDTYPE_COP;
			modelToAdd = ChoosePolicePedOccupation();
		} else {
			for (int i = 0; i < NUM_GANGS; i++) {
				if (decisionThreshold < zoneInfo.gangPedThreshold[i]) {
					pedTypeToAdd = PEDTYPE_GANG1 + i;
					break;
				}

				if (i == NUM_GANGS - 1) {
					modelToAdd = ChooseCivilianOccupation(zoneInfo.pedGroup);
					if (modelToAdd == -1)
						return;
					pedTypeToAdd = ((CPedModelInfo*)CModelInfo::GetModelInfo(modelToAdd))->m_pedType;
				}

			}
		}
		if (!addCop && m_AllRandomPedsThisType > PEDTYPE_PLAYER1)
			pedTypeToAdd = m_AllRandomPedsThisType;

		if (pedTypeToAdd >= PEDTYPE_GANG1 && pedTypeToAdd <= PEDTYPE_GANG9) {
			int randVal = CGeneral::GetRandomNumber() % 100;
			if (randVal < 50)
				return;

			if (randVal < 57) {
				pedAmount = 1;
			} else if (randVal >= 74) {
				if (randVal >= 85)
					pedAmount = 4;
				else
					pedAmount = 3;
			} else {
				pedAmount = 2;
			}
		} else
			pedAmount = 1;

		CVector generatedCoors;
		int node1, node2;
		float randomPos;
		bool foundCoors = !!ThePaths.GeneratePedCreationCoors(playerCentreOfWorld.x, playerCentreOfWorld.y, minDist, maxDist, minDistOffScreen, maxDistOffScreen,
			&generatedCoors, &node1, &node2, &randomPos, nil);

		if (!foundCoors)
			return;

		for (int i = 0; i < pedAmount; ++i) {
			if (pedTypeToAdd >= PEDTYPE_GANG1 && pedTypeToAdd <= PEDTYPE_GANG9)
				modelToAdd = ChooseGangOccupation(pedTypeToAdd - PEDTYPE_GANG1);

			if (pedTypeToAdd == PEDTYPE_COP) {
				// Unused code, ChoosePolicePedOccupation returns COP_STREET. Spawning FBI/SWAT/Army done in somewhere else.
				if (modelToAdd == COP_STREET) {
					if (!CModelInfo::GetModelInfo(MI_COP)->GetRwObject())
						return;

				} else if (modelToAdd == COP_FBI) {
					if (!CModelInfo::GetModelInfo(MI_FBI)->GetRwObject())
						return;

				} else if (modelToAdd == COP_SWAT) {
					if (!CModelInfo::GetModelInfo(MI_SWAT)->GetRwObject())
						return;

				} else if (modelToAdd == COP_ARMY && !CModelInfo::GetModelInfo(MI_ARMY)->GetRwObject()) {
					return;
				}
			} else if (!CModelInfo::GetModelInfo(modelToAdd)->GetRwObject()) {
				return;
			}
			generatedCoors.z += 0.7f;

			// What? How can this not be met?
			if (i < pedAmount) {
				//rand()
				if (gangLeader) {
					// Align gang members in formation. (btw i can't be 0 in here)
					float offsetMin = i * 0.75f;
					float offsetMax = (i + 1.0f) * 0.75f - offsetMin;
					float xOffset = CGeneral::GetRandomNumberInRange(offsetMin, offsetMin + offsetMax);
					float yOffset = CGeneral::GetRandomNumberInRange(offsetMin, offsetMin + offsetMax);
					if (CGeneral::GetRandomNumber() & 1)
						xOffset = -xOffset;
					if (CGeneral::GetRandomNumber() & 1)
						yOffset = -yOffset;
					generatedCoors.x = xOffset + gangLeader->GetPosition().x;
					generatedCoors.y = yOffset + gangLeader->GetPosition().y;
				}
			}
			if (!CPedPlacement::IsPositionClearForPed(generatedCoors))
				break;

			// Why no love for last gang member?!
			if (i + 1 < pedAmount) {
				bool foundGround;
				float groundZ = CWorld::FindGroundZFor3DCoord(generatedCoors.x, generatedCoors.y, 2.0f + generatedCoors.z, &foundGround) + 0.7f;
				if (!foundGround)
					return;

				generatedCoors.z = Max(generatedCoors.z, groundZ);
			}
			bool farEnoughToAdd = true;
			CMatrix mat(TheCamera.GetCameraMatrix());
			if (TheCamera.IsSphereVisible(generatedCoors, 2.0f, &mat)) {
				if (PedCreationDistMultiplier() * MIN_CREATION_DIST > (generatedCoors - playerCentreOfWorld).Magnitude2D())
					farEnoughToAdd = false;
			}
			if (!farEnoughToAdd)
				break;
			CPed *newPed = AddPed((ePedType)pedTypeToAdd, modelToAdd, generatedCoors);
			newPed->SetWanderPath(CGeneral::GetRandomNumberInRange(0, 8));

			if (i != 0) {
				// Gang member
				newPed->SetLeader(gangLeader);
#ifndef FIX_BUGS
				// seems to be a miami leftover (this code is not on PS2) but gang peds end up just being frozen
				newPed->m_nPedState = PED_UNKNOWN;
				gangLeader->m_nPedState = PED_UNKNOWN;
				newPed->m_fRotationCur = CGeneral::GetRadianAngleBetweenPoints(
					gangLeader->GetPosition().x, gangLeader->GetPosition().y,
					newPed->GetPosition().x, newPed->GetPosition().y);
				newPed->m_fRotationDest = newPed->m_fRotationCur;
#endif
			} else {
				gangLeader = newPed;
			}
			CVisibilityPlugins::SetClumpAlpha(newPed->GetClump(), 0);
			/*
			// Pointless, this is already a for loop
			if (i + 1 > pedAmount)
				break;
			if (pedAmount <= 1)
				break; */
		}
	}
}

CPed*
CPopulation::AddPedInCar(CVehicle* car, bool isDriver)
{
	int defaultModel = MI_MALE01;
	int miamiViceIndex = 0;
	bool imSureThatModelIsLoaded = true;
	CVector coors = FindPlayerCoors();
	CZoneInfo zoneInfo;
	int pedType;

	// May be eCopType, model index or non-sense(for medic), AddPed knows that by looking to ped type.
	int preferredModel;

	CTheZones::GetZoneInfoForTimeOfDay(&coors, &zoneInfo);
	switch (car->GetModelIndex()) {
		case MI_FIRETRUCK:
			preferredModel = 0;
			pedType = PEDTYPE_FIREMAN;
			break;
		case MI_AMBULAN:
			preferredModel = 0;
			pedType = PEDTYPE_EMERGENCY;
			break;
		case MI_FBICAR:
			preferredModel = COP_FBI;
			pedType = PEDTYPE_COP;
			break;
		case MI_POLICE:
		case MI_PREDATOR:
			preferredModel = COP_STREET;
			pedType = PEDTYPE_COP;
			break;
		case MI_ENFORCER:
			preferredModel = COP_SWAT;
			pedType = PEDTYPE_COP;
			break;
		case MI_RHINO:
		case MI_BARRACKS:
			preferredModel = COP_ARMY;
			pedType = PEDTYPE_COP;
			break;
		case MI_VICECHEE: // TODO(MIAMI): figure out new structure of the function
			preferredModel = COP_MIAMIVICE;
			pedType = PEDTYPE_COP;
			miamiViceIndex = (isDriver ? 2 * CCarCtrl::MiamiViceCycle : 2 * CCarCtrl::MiamiViceCycle + 1);
			break;
		case MI_TAXI:
		case MI_CABBIE:
		case MI_ZEBRA:
		case MI_KAUFMAN:
			if (CGeneral::GetRandomTrueFalse()) {
				pedType = PEDTYPE_CIVMALE;
				preferredModel = MI_TAXI_D;
				break;
			}
			defaultModel = MI_TAXI_D;

			// fall through
		default:
			int gangOfPed = 0;
			imSureThatModelIsLoaded = false;

			while (gangOfPed < NUM_GANGS && CGangs::GetGangInfo(gangOfPed)->m_nVehicleMI != car->GetModelIndex())
				gangOfPed++;

			if (gangOfPed < NUM_GANGS) {
				pedType = gangOfPed + PEDTYPE_GANG1;
				preferredModel = ChooseGangOccupation(gangOfPed);
			} else if (gangOfPed == NUM_GANGS) {
			    CVehicleModelInfo *carModelInfo = ((CVehicleModelInfo *)CModelInfo::GetModelInfo(car->GetModelIndex()));
				int i = 15;
				for(; i >= 0; i--) {
					// Should return random model each time
					preferredModel = ChooseCivilianOccupation(zoneInfo.pedGroup);
					if (preferredModel == -1)
						preferredModel = defaultModel;

					if (((CPedModelInfo*)CModelInfo::GetModelInfo(preferredModel))->m_carsCanDrive & (1 << carModelInfo->m_vehicleClass))
						break;
				}
				if (i == -1)
					preferredModel = defaultModel;

				pedType = ((CPedModelInfo*)CModelInfo::GetModelInfo(preferredModel))->m_pedType;
			}
			break;
	}
	if (!imSureThatModelIsLoaded && !((CPedModelInfo*)CModelInfo::GetModelInfo(preferredModel))->GetRwObject()) {
		preferredModel = defaultModel;
		pedType = ((CPedModelInfo*)CModelInfo::GetModelInfo(defaultModel))->m_pedType;
	}

	CPed *newPed = CPopulation::AddPed((ePedType)pedType, preferredModel, car->GetPosition(), miamiViceIndex);
	newPed->bUsesCollision = false;

	if (newPed->GetWeapon()->m_eWeaponType != WEAPONTYPE_UNARMED) {
		newPed->RemoveWeaponModel(CWeaponInfo::GetWeaponInfo(newPed->GetWeapon()->m_eWeaponType)->m_nModelId);
	}

	newPed->AddInCarAnims(car, isDriver);
	return newPed;
}

void
CPopulation::MoveCarsAndPedsOutOfAbandonedZones()
{
}

void
CPopulation::ConvertAllObjectsToDummyObjects()
{
	int poolSize = CPools::GetObjectPool()->GetSize();
	for (int poolIndex = poolSize - 1; poolIndex >= 0; poolIndex--) {

		CObject *obj = CPools::GetObjectPool()->GetSlot(poolIndex);
		if (obj) {
			if (obj->CanBeDeleted())
				ConvertToDummyObject(obj);
		}
	}
}

void
CPopulation::ConvertToRealObject(CDummyObject *dummy)
{
	if (!TestSafeForRealObject(dummy))
		return;

	CObject *obj = new CObject(dummy);
	if (!obj)
		return;

	CWorld::Remove(dummy);
	delete dummy;
	CWorld::Add(obj);

	CSimpleModelInfo *mi = (CSimpleModelInfo*)CModelInfo::GetModelInfo(obj->GetModelIndex());
	if (IsGlass(obj->GetModelIndex()) && !mi->m_isArtistGlass) {
		obj->bIsVisible = false;
	} else if (obj->GetModelIndex() == MI_BUOY) {
		obj->bIsStatic = false;
		obj->m_vecMoveSpeed = CVector(0.0f, 0.0f, -0.001f);
		obj->bTouchingWater = true;
		obj->AddToMovingList();
	}
}

void
CPopulation::ConvertToDummyObject(CObject *obj)
{
	CDummyObject *dummy = new CDummyObject(obj);

	dummy->GetMatrix() = obj->m_objectMatrix;
	dummy->GetMatrix().UpdateRW();
	dummy->UpdateRwFrame();

	CSimpleModelInfo *mi = (CSimpleModelInfo*)CModelInfo::GetModelInfo(obj->GetModelIndex());
	if (IsGlass(obj->GetModelIndex()) && !mi->m_isArtistGlass)
		dummy->bIsVisible = false;

	CWorld::Remove(obj);
	delete obj;
	CWorld::Add(dummy);
}

bool
CPopulation::TestRoomForDummyObject(CObject *obj)
{
	int16 collidingObjs;
	CWorld::FindObjectsKindaColliding(obj->m_objectMatrix.GetPosition(), CModelInfo::GetModelInfo(obj->GetModelIndex())->GetColModel()->boundingSphere.radius,
		false, &collidingObjs, 2, nil, false, true, true, false, false);

	return collidingObjs == 0;
}

bool
CPopulation::TestSafeForRealObject(CDummyObject *dummy)
{
	CPtrNode *ptrNode;
	CColModel *dummyCol = dummy->GetColModel();
	float colRadius = dummy->GetBoundRadius();
	CVector colCentre = dummy->GetBoundCentre();

	int minX = CWorld::GetSectorIndexX(dummy->GetPosition().x - colRadius);
	if (minX < 0) minX = 0;
	int minY = CWorld::GetSectorIndexY(dummy->GetPosition().y - colRadius);
	if (minY < 0) minY = 0;
	int maxX = CWorld::GetSectorIndexX(dummy->GetPosition().x + colRadius);
#ifdef FIX_BUGS
	if (maxX >= NUMSECTORS_X) maxX = NUMSECTORS_X - 1;
#else
	if (maxX >= NUMSECTORS_X) maxX = NUMSECTORS_X;
#endif

	int maxY = CWorld::GetSectorIndexY(dummy->GetPosition().y + colRadius);
#ifdef FIX_BUGS
	if (maxY >= NUMSECTORS_Y) maxY = NUMSECTORS_Y - 1;
#else
	if (maxY >= NUMSECTORS_Y) maxY = NUMSECTORS_Y;
#endif

	static CColPoint aTempColPoints[MAX_COLLISION_POINTS];

	for (int curY = minY; curY <= maxY; curY++) {
		for (int curX = minX; curX <= maxX; curX++) {
			CSector *sector = CWorld::GetSector(curX, curY);

			for (ptrNode = sector->m_lists[ENTITYLIST_VEHICLES].first; ptrNode; ptrNode = ptrNode->next) {
				CVehicle *veh = (CVehicle*)ptrNode->item;
				if (veh->m_scanCode != CWorld::GetCurrentScanCode()) {
					if (veh->GetIsTouching(colCentre, colRadius)) {
						veh->m_scanCode = CWorld::GetCurrentScanCode();
						if (CCollision::ProcessColModels(dummy->GetMatrix(), *dummyCol, veh->GetMatrix(), *veh->GetColModel(), aTempColPoints, nil, nil) > 0)
							return false;
					}
				}
			}

			for (ptrNode = sector->m_lists[ENTITYLIST_VEHICLES_OVERLAP].first; ptrNode; ptrNode = ptrNode->next) {
				CVehicle *veh = (CVehicle*)ptrNode->item;
				if (veh->m_scanCode != CWorld::GetCurrentScanCode()) {
					if (veh->GetIsTouching(colCentre, colRadius)) {
						veh->m_scanCode = CWorld::GetCurrentScanCode();
						if (CCollision::ProcessColModels(dummy->GetMatrix(), *dummyCol, veh->GetMatrix(), *veh->GetColModel(), aTempColPoints, nil, nil) > 0)
							return false;
					}
				}
			}
		}
	}
	return true;
}

void
CPopulation::ManagePopulation(void)
{
	int frameMod32 = CTimer::GetFrameCounter() & 31;
	CVector playerPos = FindPlayerCentreOfWorld(CWorld::PlayerInFocus);

	// Why this code is here?! Delete temporary objects when they got too far, and convert others to "dummy" objects. (like lamp posts)
	int objectPoolSize = CPools::GetObjectPool()->GetSize();
	for (int i = objectPoolSize * frameMod32 / 32; i < objectPoolSize * (frameMod32 + 1) / 32; i++) {
		CObject *obj = CPools::GetObjectPool()->GetSlot(i);
		if (obj && obj->CanBeDeleted()) {
			if ((obj->GetPosition() - playerPos).Magnitude() <= 80.0f ||
				(obj->m_objectMatrix.GetPosition() - playerPos).Magnitude() <= 80.0f) {
				if (obj->ObjectCreatedBy == TEMP_OBJECT && CTimer::GetTimeInMilliseconds() > obj->m_nEndOfLifeTime) {
					CWorld::Remove(obj);
					delete obj;
				}
			} else {
				if (obj->ObjectCreatedBy == TEMP_OBJECT) {
					CWorld::Remove(obj);
					delete obj;
				} else if (obj->ObjectCreatedBy != CUTSCENE_OBJECT && TestRoomForDummyObject(obj)) {
					ConvertToDummyObject(obj);
				}
			}
		}
	}

	// Convert them back to real objects. Dummy objects don't have collisions, so they need to be converted.
	int dummyPoolSize = CPools::GetDummyPool()->GetSize();
	for (int i = dummyPoolSize * frameMod32 / 32; i < dummyPoolSize * (frameMod32 + 1) / 32; i++) {
		CDummy *dummy = CPools::GetDummyPool()->GetSlot(i);
		if (dummy) {
			if ((dummy->GetPosition() - playerPos).Magnitude() < 80.0f)
				ConvertToRealObject((CDummyObject*)dummy);
		}
	}

	int pedPoolSize = CPools::GetPedPool()->GetSize();
	for (int poolIndex = pedPoolSize-1; poolIndex >= 0; poolIndex--) {
		CPed *ped = CPools::GetPedPool()->GetSlot(poolIndex);

		if (ped && !ped->IsPlayer() && ped->CanBeDeleted() && !ped->bInVehicle) {
			if (ped->m_nPedState == PED_DEAD && CTimer::GetTimeInMilliseconds() - ped->m_bloodyFootprintCountOrDeathTime > 60000)
				ped->bFadeOut = true;

			if (ped->bFadeOut && CVisibilityPlugins::GetClumpAlpha(ped->GetClump()) == 0) {
				RemovePed(ped);
				continue;
			}

			float dist = (ped->GetPosition() - playerPos).Magnitude2D();

			if (ped->IsGangMember() || (ped->bDeadPedInFrontOfCar && ped->m_vehicleInAccident))
				dist -= 30.0f;

			bool pedIsFarAway = false;
			if (PedCreationDistMultiplier() * (PED_REMOVE_DIST_SPECIAL * TheCamera.GenerationDistMultiplier) < dist
				|| (!ped->bCullExtraFarAway && PedCreationDistMultiplier() * PED_REMOVE_DIST * TheCamera.GenerationDistMultiplier < dist)
				|| (PedCreationDistMultiplier() * (MIN_CREATION_DIST + CREATION_RANGE) * OFFSCREEN_CREATION_MULT < dist
				&& !ped->GetIsOnScreen()
				&& TheCamera.Cams[TheCamera.ActiveCam].Mode != CCam::MODE_SNIPER
				&& TheCamera.Cams[TheCamera.ActiveCam].Mode != CCam::MODE_SNIPER_RUNABOUT
				&& !TheCamera.Cams[TheCamera.ActiveCam].LookingLeft
				&& !TheCamera.Cams[TheCamera.ActiveCam].LookingRight
				&& !TheCamera.Cams[TheCamera.ActiveCam].LookingBehind))
				pedIsFarAway = true;

			if (!pedIsFarAway)
				continue;

			if (ped->m_nPedState == PED_DEAD && !ped->bFadeOut) {
				CVector pedPos = ped->GetPosition();

				float randAngle = (uint8) CGeneral::GetRandomNumber() * (3.14f / 128.0f); // Not PI, 3.14
				switch (CGeneral::GetRandomNumber() % 3) {
					case 0:
						CShadows::AddPermanentShadow(SHADOWTYPE_DARK, gpOutline1Tex, &pedPos,
							0.9f * Cos(randAngle), 0.9f * Sin(randAngle), 0.9f * Sin(randAngle), -0.9f * Cos(randAngle),
							255, 255, 255, 255, 4.0f, 40000, 1.0f);
						break;
					case 1:
						CShadows::AddPermanentShadow(SHADOWTYPE_DARK, gpOutline2Tex, &pedPos,
							0.9f * Cos(randAngle), 0.9f * Sin(randAngle), 0.9f * Sin(randAngle), -0.9f * Cos(randAngle),
							255, 255, 255, 255, 4.0f, 40000, 1.0f);
						break;
					case 2:
						CShadows::AddPermanentShadow(SHADOWTYPE_DARK, gpOutline3Tex, &pedPos,
							0.9f * Cos(randAngle), 0.9f * Sin(randAngle), 0.9f * Sin(randAngle), -0.9f * Cos(randAngle),
							255, 255, 255, 255, 4.0f, 40000, 1.0f);
						break;
					default:
						break;
				}
			}
			if (ped->GetIsOnScreen())
				ped->bFadeOut = true;
			else
				RemovePed(ped);
		}
	}
}

CPed* 
CPopulation::AddDeadPedInFrontOfCar(const CVector& pos, CVehicle* pCulprit)
{
	if (TheCamera.IsSphereVisible(pos, 2.0f)) {
		float fDistanceToPlayer = (pos - FindPlayerPed()->GetPosition()).Magnitude2D();
		float fDistanceMultiplier;
		if (FindPlayerVehicle())
			fDistanceMultiplier = clamp(FindPlayerVehicle()->GetMoveSpeed().Magnitude2D() - 0.1f + 1.0f, 1.0f, 1.5f);
		else
			fDistanceMultiplier = 1.0f;
		if (40.0f * fDistanceMultiplier > fDistanceToPlayer)
			return nil;
	}
	bool found;
	float z = CWorld::FindGroundZFor3DCoord(pos.x, pos.y, pos.z, &found) + 1.0f;
	if (!found)
		return nil;
	z = Max(z, pos.z);
	if (!CModelInfo::GetModelInfo(MI_MALE01)->GetRwObject()) // strange way to check it
		return nil;
	CPed* pPed = CPopulation::AddPed(PEDTYPE_CIVMALE, MI_MALE01, pos); // TODO(MIAMI): 4th parameter
	pPed->SetDie();
	pPed->m_nPedMoney = 0;
	pPed->bDeadPedInFrontOfCar = true;
	pPed->m_vehicleInAccident = pCulprit;
	pCulprit->RegisterReference((CEntity**)&pPed->m_vehicleInAccident);
	CEntity* pEntities[3] = { 0 };
	if (!CPedPlacement::IsPositionClearForPed(pos, 2.0f, 3, pEntities)) {
		for (int i = 0; i < 3; i++) {
			if (pEntities[i] && pEntities[i] != pCulprit && pEntities[i] != pPed) {
				CWorld::Remove(pPed);
				delete pPed;
				return nil;
			}
		}
	}
	CColPoint colpts[MAX_COLLISION_POINTS];
	if (CCollision::ProcessColModels(pCulprit->GetMatrix(), *pCulprit->GetColModel(), pPed->GetMatrix(), *pPed->GetColModel(), colpts, nil, nil)) {
		CWorld::Remove(pPed);
		delete pPed;
		return nil;
	}
	CVisibilityPlugins::SetClumpAlpha(pPed->GetClump(), 0);
	return pPed;
}

bool
CPopulation::IsSkateable(CVector const& pos)
{
	CColPoint foundCol;
	CEntity* foundEnt = nil;
	CWorld::ProcessVerticalLine(pos + CVector(0.f, 0.f, 2.f), pos.z - 2.0f, foundCol, foundEnt, true, false, false, false, false, false, nil);
	if (!foundEnt)
		return false;

	return foundCol.surfaceB == SURFACE_TARMAC || foundCol.surfaceB == SURFACE_PAVEMENT;
}