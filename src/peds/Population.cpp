#include "common.h"

#include "vehicle_enums.h" //needs to be before Vehicle.h

#include "Vehicle.h"

#include "Camera.h"
#include "CarCtrl.h"
#include "CivilianPed.h"
#include "Clock.h"
#include "CopPed.h"
#include "CutsceneMgr.h"
#include "Darkel.h"
#include "DummyObject.h"
#include "EmergencyPed.h"
#include "FileMgr.h"
#include "Game.h"
#include "Gangs.h"
#include "General.h"
#include "IniFile.h"
#include "ModelIndices.h"
#include "PedPlacement.h"
#include "Population.h"
#include "Replay.h"
#include "Script.h"
#include "Shadows.h"
#include "Streaming.h"
#include "SurfaceTable.h"
#include "VisibilityPlugins.h"
#include "Wanted.h"
#include "WaterLevel.h"
#include "Weather.h"
#include "World.h"
#include "Zones.h"

// --MIAMI: File done

#define MIN_CREATION_DIST		40.0f // not for start of the game (look at the GeneratePedsAtStartOfGame)
#define CREATION_RANGE			10.0f // added over the MIN_CREATION_DIST.
#define OFFSCREEN_CREATION_MULT	0.5f
#define PED_REMOVE_DIST			(MIN_CREATION_DIST + CREATION_RANGE + 1.0f)
#define PED_REMOVE_DIST_SPECIAL	(MIN_CREATION_DIST + CREATION_RANGE + 15.0f) // for peds with bCullExtraFarAway flag

PedGroup CPopulation::ms_pPedGroups[NUMPEDGROUPS];
bool CPopulation::ms_bGivePedsWeapons;
int32 CPopulation::m_AllRandomPedsThisType = -1;
float CPopulation::PedDensityMultiplier = 1.0f;
uint32 CPopulation::ms_nTotalMissionPeds;
int32 CPopulation::MaxNumberOfPedsInUse = 25;
int32 CPopulation::MaxNumberOfPedsInUseInterior = 40;
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

uint32 CPopulation::ms_nTotalCarPassengerPeds;
uint32 CPopulation::NumMiamiViceCops;

uint32 gLastSelectedCivilianIndex;
CEntity *gSunbatheObstacles[2];
CEntity *gCoupleObstacles[3];

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
	ms_nTotalCivPeds = 0;
	ms_nTotalGangPeds = 0;
	ms_nTotalPeds = 0;
	ms_nTotalMissionPeds = 0;
	m_CountDownToPedsAtStart = 2;
	bZoneChangeHasHappened = false; // III leftover

	m_AllRandomPedsThisType = -1;
	PedDensityMultiplier = 1.0f;
	

	LoadPedGroups();

	debug("CPopulation ready\n");
}

void
CPopulation::RemovePed(CPed *ent)
{
	CWorld::Remove((CEntity*)ent);
	delete ent;
}

int32
CPopulation::ChooseCivilianOccupation(int32 group)
{
	if (CWeather::Rain > 0.1f) {
		int32 lastModel;
		for (int i = 0; i < 8; i++) {
			gLastSelectedCivilianIndex = CGeneral::GetRandomNumberInRange(0, NUMMODELSPERPEDGROUP);
			lastModel = ms_pPedGroups[group].models[gLastSelectedCivilianIndex];

			if (!CPopulation::IsSunbather(lastModel))
				break;
		}
		return lastModel;

	} else {
		gLastSelectedCivilianIndex = CGeneral::GetRandomNumberInRange(0, NUMMODELSPERPEDGROUP);
		return ms_pPedGroups[group].models[gLastSelectedCivilianIndex];
	}
}

int32
CPopulation::ChooseNextCivilianOccupation(int32 group)
{
	if (CWeather::Rain > 0.1f) {
		int32 lastModel;
		for (int i = 0; i < NUMMODELSPERPEDGROUP; i++) {
			++gLastSelectedCivilianIndex;
			if (gLastSelectedCivilianIndex >= NUMMODELSPERPEDGROUP)
				gLastSelectedCivilianIndex = 0;
			lastModel = ms_pPedGroups[group].models[gLastSelectedCivilianIndex];

			if (!CPopulation::IsSunbather(ms_pPedGroups[group].models[gLastSelectedCivilianIndex]))
				break;
		}
		return lastModel;

	} else {
		++gLastSelectedCivilianIndex;
		if (gLastSelectedCivilianIndex >= NUMMODELSPERPEDGROUP)
			gLastSelectedCivilianIndex = 0;
		return ms_pPedGroups[group].models[gLastSelectedCivilianIndex];
	}
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

void
CPopulation::DealWithZoneChange(eLevelName oldLevel, eLevelName newLevel, bool forceIndustrialZone)
{
}

void
CPopulation::Update(bool addPeds)
{
	if (!CReplay::IsPlayingBack()) {
		ManagePopulation();
		RemovePedsIfThePoolGetsFull();
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
			if (!CCutsceneMgr::IsRunning() && addPeds) {
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
	for (int i = 0; i < 100; i++) {
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
	int32 man = -1, woman = -1;
	CPed *gangLeader = nil;
	bool addCop = false;
	bool isSecurityGuard = false;
	bool forceAddingCop = false;
	CPlayerInfo *playerInfo = &CWorld::Players[CWorld::PlayerInFocus];
	CVector playerCentreOfWorld = FindPlayerCentreOfWorld(CWorld::PlayerInFocus);
	CTheZones::GetZoneInfoForTimeOfDay(&playerCentreOfWorld, &zoneInfo);
	CWanted *wantedInfo = playerInfo->m_pPed->m_pWanted;

	if (wantedInfo->m_nWantedLevel > 2) {
		if (!CGame::IsInInterior() && (CGeneral::GetRandomNumber() % 32 == 0) && FindPlayerVehicle())
			forceAddingCop = true;

		uint32 maxCops = CGame::IsInInterior() ? wantedInfo->m_MaxCops * 1.6f : wantedInfo->m_MaxCops;
		if ((ms_nNumCop < maxCops || forceAddingCop) &&
			(!playerInfo->m_pPed->bInVehicle &&
				(CCarCtrl::NumLawEnforcerCars >= wantedInfo->m_MaximumLawEnforcerVehicles
				|| CCarCtrl::NumRandomCars >= playerInfo->m_nTrafficMultiplier * CCarCtrl::CarDensityMultiplier
				|| CCarCtrl::NumFiretrucksOnDuty + CCarCtrl::NumAmbulancesOnDuty + CCarCtrl::NumParkedCars
				+ CCarCtrl::NumMissionCars + CCarCtrl::NumLawEnforcerCars + CCarCtrl::NumRandomCars >= CCarCtrl::MaxNumberOfCarsInUse) || forceAddingCop)) {
			addCop = true;
			minDist = PedCreationDistMultiplier() * MIN_CREATION_DIST;
			maxDist = PedCreationDistMultiplier() * (MIN_CREATION_DIST + CREATION_RANGE);
		}
	}
	float missionAndWeatherMult = -0.8f * Sqrt(CWeather::Rain) + 1.0f;

	// Taxi side mission
	if (CTheScripts::IsPlayerOnAMission()) {
		CPed *player = FindPlayerPed();
		if (player && player->InVehicle()) {
			int32 model = player->m_pMyVehicle->GetModelIndex();
			if (model == MI_TAXI || model == MI_CABBIE || model == MI_ZEBRA || model == MI_KAUFMAN)
				missionAndWeatherMult = 1.0f;
		}
	}
	if (CDarkel::FrenzyOnGoing())
		missionAndWeatherMult = 1.0f;
	int selectedMaxPeds = CGame::IsInInterior() ? CPopulation::MaxNumberOfPedsInUseInterior : CPopulation::MaxNumberOfPedsInUse;

	// Yeah, float
	float maxPossiblePedsForArea = (zoneInfo.pedDensity + zoneInfo.carDensity) * playerInfo->m_fRoadDensity * PedDensityMultiplier
		* (CDarkel::FrenzyOnGoing() ? 1.f : CIniFile::PedNumberMultiplier) * missionAndWeatherMult;
	maxPossiblePedsForArea = Min(maxPossiblePedsForArea, selectedMaxPeds);

	if (ms_nTotalPeds < maxPossiblePedsForArea || addCop) {
		int decisionThreshold = CGeneral::GetRandomNumberInRange(0, 1000);
		if (decisionThreshold < zoneInfo.copPedThreshold || addCop) {
			pedTypeToAdd = PEDTYPE_COP;
			modelToAdd = ChoosePolicePedOccupation();
		} else {
			int i = 0;
			for (i = 0; i < NUM_GANGS; i++) {
				if (decisionThreshold < zoneInfo.gangPedThreshold[i]) {
					break;
				}
			}

			if (i == NUM_GANGS) {
				if (CGeneral::GetRandomNumberInRange(0.0f, 1.0f) <= 0.95f) {
					modelToAdd = ChooseCivilianOccupation(zoneInfo.pedGroup);

					if (modelToAdd == -1)
						return;
					pedTypeToAdd = ((CPedModelInfo*)CModelInfo::GetModelInfo(modelToAdd))->m_pedType;

				} else {
					ChooseCivilianCoupleOccupations(zoneInfo.pedGroup, man, woman);
					if (man == -1 || woman == -1)
						return;
					pedTypeToAdd = ((CPedModelInfo*)CModelInfo::GetModelInfo(woman))->m_pedType;
				}
			} else {
				pedTypeToAdd = PEDTYPE_GANG1 + i;

				if (IsSecurityGuard((ePedType)pedTypeToAdd)) {
					isSecurityGuard = true;
					modelToAdd = ChooseGangOccupation(pedTypeToAdd - PEDTYPE_GANG1);

					if (modelToAdd == -1)
						return;
					pedTypeToAdd = ((CPedModelInfo*)CModelInfo::GetModelInfo(modelToAdd))->m_pedType;

				}
			}
		}
		if (!addCop && m_AllRandomPedsThisType > PEDTYPE_PLAYER1)
			pedTypeToAdd = m_AllRandomPedsThisType;

		if (pedTypeToAdd >= PEDTYPE_GANG1 && pedTypeToAdd <= PEDTYPE_GANG9 && !isSecurityGuard) {
			minDist += 30.0f;
			maxDist += 30.0f;
			pedAmount = ComputeRandomisedGangSize();
		} else
			pedAmount = 1;

		CVector generatedCoors;
		int node1, node2;
		float randomPos;
		bool foundCoors = !!ThePaths.GeneratePedCreationCoors(playerCentreOfWorld.x, playerCentreOfWorld.y, minDist, maxDist, minDistOffScreen, maxDistOffScreen,
			&generatedCoors, &node1, &node2, &randomPos, nil);

		if (!foundCoors)
			return;

		uint8 nodeSpawnRate = Min(ThePaths.m_pathNodes[node1].spawnRate, ThePaths.m_pathNodes[node2].spawnRate);
		int randomRate = CGeneral::GetRandomNumber() & 0xF;
		if (randomRate > nodeSpawnRate)
			return;

		CPathFind::TakeWidthIntoAccountForCoors(&ThePaths.m_pathNodes[node1], &ThePaths.m_pathNodes[node2], CGeneral::GetRandomNumber(), &generatedCoors.x, &generatedCoors.y);
		if (CGame::currArea == AREA_MALL && (pedTypeToAdd == PEDTYPE_CIVMALE || pedTypeToAdd == PEDTYPE_CIVFEMALE || pedTypeToAdd == PEDTYPE_CRIMINAL) &&
			CGeneral::GetRandomNumberInRange(0.f, 1.f) > 0.5f) {

			PlaceMallPedsAsStationaryGroup(generatedCoors, zoneInfo.pedGroup);
			return;
		}

		if (pedTypeToAdd >= PEDTYPE_GANG1 && pedTypeToAdd <= PEDTYPE_GANG9 && !isSecurityGuard) {
			PlaceGangMembers((ePedType)pedTypeToAdd, pedAmount, generatedCoors);
			return;
		}

		if (man > -1 && woman > -1) {
			PlaceCouple(PEDTYPE_CIVMALE, man, PEDTYPE_CIVFEMALE, woman, generatedCoors);
			return;
		}

		for (int i = 0; i < pedAmount; ++i) {

			if (pedTypeToAdd == PEDTYPE_COP) {
				// Unused code, ChoosePolicePedOccupation returns COP_STREET. Spawning FBI/SWAT/Army done in somewhere else.
				if (modelToAdd == COP_STREET) {
					if (!CStreaming::HasModelLoaded(MI_COP))
						return;

				} else if (modelToAdd == COP_FBI) {
					if (!CStreaming::HasModelLoaded(MI_COP) || !CStreaming::HasModelLoaded(CWeaponInfo::GetWeaponInfo(WEAPONTYPE_MP5)->m_nModelId))
						return;

				} else if (modelToAdd == COP_SWAT) {
					if (!CStreaming::HasModelLoaded(MI_SWAT) || !CStreaming::HasModelLoaded(CWeaponInfo::GetWeaponInfo(WEAPONTYPE_UZI)->m_nModelId))
						return;

				} else if (modelToAdd == COP_ARMY) {
					if (!CStreaming::HasModelLoaded(MI_ARMY) ||
						!CStreaming::HasModelLoaded(CWeaponInfo::GetWeaponInfo(WEAPONTYPE_MP5)->m_nModelId) || !CStreaming::HasModelLoaded(CWeaponInfo::GetWeaponInfo(WEAPONTYPE_GRENADE)->m_nModelId))
						return;
				}
			} else if (!CStreaming::HasModelLoaded(modelToAdd)) {
				return;
			}
			generatedCoors.z += 0.7f;

			// What? How can this not be met?
			if (i < pedAmount) {
				// rand()
				// III leftover, unused
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
			bool surfaceAndDistIsOk = true;
			if (TheCamera.IsSphereVisible(generatedCoors, 2.0f)) {
				if (PedCreationDistMultiplier() * MIN_CREATION_DIST > (generatedCoors - playerCentreOfWorld).Magnitude2D())
					surfaceAndDistIsOk = false;
			}

			// Place skaters if only they're on tarmac.
			if (((CPedModelInfo*)CModelInfo::GetModelInfo(modelToAdd))->m_pedStatType == PEDSTAT_SKATER) {
				CEntity* foundEnt = nil;
				CColPoint foundCol;
				CWorld::ProcessVerticalLine(generatedCoors + CVector(0.f, 0.f, 2.f), generatedCoors.z - 2.0f, foundCol, foundEnt, true, false, false, false, false, false, nil);
				if (foundEnt) {
					if (foundCol.surfaceB == SURFACE_TARMAC || foundCol.surfaceB == SURFACE_PAVEMENT)
						surfaceAndDistIsOk = true;
					else
						surfaceAndDistIsOk = false;

				} else {
					surfaceAndDistIsOk = false;
				}
			}
			if (!surfaceAndDistIsOk)
				break;
			CPed *newPed = AddPed((ePedType)pedTypeToAdd, modelToAdd, generatedCoors);
			if (forceAddingCop && newPed->m_nPedType == PEDTYPE_COP)
				((CCopPed*)newPed)->m_bThrowsSpikeTrap = true;

			bool gonnaSunbathe = false;
			if (CPopulation::IsSunbather(modelToAdd)) {
				CEntity* foundEnt = nil;
				CColPoint foundCol;
				CWorld::ProcessVerticalLine(generatedCoors + CVector(0.f, 0.f, 2.f), generatedCoors.z - 2.0f, foundCol, foundEnt, true, false, false, false, false, false, nil);
				if (foundEnt) {
					if ((foundCol.surfaceB == SURFACE_CONCRETE_BEACH || foundCol.surfaceB == SURFACE_SAND)
						&& CClock::GetHours() >= 10 && CClock::GetHours() <= 18 && 0.0f == CWeather::Rain) {
						gonnaSunbathe = true;
						if (CPedPlacement::IsPositionClearForPed(generatedCoors, 3.0f, ARRAY_SIZE(gSunbatheObstacles), gSunbatheObstacles)) {
							for (int j = 0; j < ARRAY_SIZE(gSunbatheObstacles); j++) {
								if (gSunbatheObstacles[j] && gSunbatheObstacles[j] != newPed)
									gonnaSunbathe = false;
							}
						}
					}
				}
			}
			if (gonnaSunbathe) {
				float heading = CGeneral::GetRandomNumberInRange(0.f, 1.f) * TWOPI;
				newPed->m_fRotationDest = heading;
				newPed->m_fRotationCur = heading;
				// unused
				// v61 = CGeneral::GetRandomTrueFalse();
				newPed->SetWaitState(WAITSTATE_SUN_BATHE_IDLE, nil);
				CVector toyPos(newPed->GetPosition());
				float waterLevel;
				if (CWaterLevel::GetGroundLevel(toyPos, &waterLevel, nil, 30.0f)) {
					toyPos.z = 0.04f + waterLevel;
					CEntity *toy = CWaterLevel::CreateBeachToy(toyPos, BEACHTOY_ANY_TOWEL);
					if (toy)
						toy->SetHeading(heading);

					if (!(CGeneral::GetRandomNumber() & 3)) {
						CWaterLevel::CreateBeachToy(toyPos + CVector(CGeneral::GetRandomNumberInRange(-2.f, 2.f), CGeneral::GetRandomNumberInRange(-2.f, 2.f), 0.f), BEACHTOY_LOTION);
					}
				}
			} else {
				newPed->SetWanderPath(CGeneral::GetRandomNumberInRange(0, 8));
			}
			
			if (i != 0) {
				// Gang member
				newPed->SetLeader(gangLeader);

				newPed->SetPedState(PED_UNKNOWN);
				gangLeader->SetPedState(PED_UNKNOWN);

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
	const int defaultModel = MI_MALE01;
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
		case MI_FBIRANCH:
			preferredModel = COP_FBI;
			pedType = PEDTYPE_COP;
			break;
		default:
			if (car->GetModelIndex() == MI_TAXI || car->GetModelIndex() == MI_CABBIE || car->GetModelIndex() == MI_ZEBRA || car->GetModelIndex() == MI_KAUFMAN) {
				if (isDriver) {
					pedType = PEDTYPE_CIVMALE;
					preferredModel = MI_TAXI_D;
					break;
				}
				// fall through if not
			} else if (car->GetModelIndex() == MI_VICECHEE) {
				if (car->bIsLawEnforcer) {
					preferredModel = COP_MIAMIVICE;
					pedType = PEDTYPE_COP;
					miamiViceIndex = (isDriver ? 2 * CCarCtrl::MiamiViceCycle : 2 * CCarCtrl::MiamiViceCycle + 1);
					break;
				}
				// fall through if not
			}

			int gangOfPed = 0;
			imSureThatModelIsLoaded = false;

			while (gangOfPed < NUM_GANGS && CGangs::GetGangInfo(gangOfPed)->m_nVehicleMI != car->GetModelIndex())
				gangOfPed++;

			if (gangOfPed < NUM_GANGS) {
				pedType = gangOfPed + PEDTYPE_GANG1;
				preferredModel = ChooseGangOccupation(gangOfPed);
			} else if (gangOfPed == NUM_GANGS) {
			    CVehicleModelInfo *carModel = ((CVehicleModelInfo *)CModelInfo::GetModelInfo(car->GetModelIndex()));
				preferredModel = ChooseCivilianOccupation(zoneInfo.pedGroup);
				int i = 15;
				for(; i >= 0; i--) {
					CPedModelInfo* pedModel = (CPedModelInfo*)CModelInfo::GetModelInfo(preferredModel);

					if (pedModel->GetRwObject()) {
						if (!car->IsPassenger(preferredModel) && !car->IsDriver(preferredModel)) {
							if (((CPedModelInfo*)CModelInfo::GetModelInfo(preferredModel))->m_carsCanDrive & (1 << carModel->m_vehicleClass))
								break;
						}
					}

					preferredModel = ChooseNextCivilianOccupation(zoneInfo.pedGroup);
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
		obj->SetIsStatic(false);
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
	CWorld::FindObjectsKindaColliding(obj->m_objectMatrix.GetPosition(), obj->GetBoundRadius(),
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
			float objPlayerDist = (obj->GetPosition() - playerPos).Magnitude();
			if (obj->ObjectCreatedBy == TEMP_OBJECT) {
				if (obj->GetModelIndex() != MI_ROADWORKBARRIER1 && obj->GetModelIndex() != MI_BEACHBALL) {
					if (objPlayerDist > 51.0f || objPlayerDist > 25.0f && !obj->GetIsOnScreen() || CTimer::GetTimeInMilliseconds() > obj->m_nEndOfLifeTime) {
						CWorld::Remove(obj);
						delete obj;
					}
				} else if (objPlayerDist > 120.0f) {
					CWorld::Remove(obj);
					delete obj;
				}

			} else if (objPlayerDist > 80.0f && (obj->m_objectMatrix.GetPosition() - playerPos).Magnitude() > 80.0f) {
				if (obj->ObjectCreatedBy != CUTSCENE_OBJECT && TestRoomForDummyObject(obj)) {
					ConvertToDummyObject(obj);
				}
			}
		}
	}

	// Convert them back to real objects. Dummy objects don't have collisions, so they need to be converted.
	int dummyPoolSize = CPools::GetDummyPool()->GetSize();
	for (int i = dummyPoolSize * frameMod32 / 32; i < dummyPoolSize * (frameMod32 + 1) / 32; i++) {
		CDummy *dummy = CPools::GetDummyPool()->GetSlot(i);
		if (dummy && (dummy->m_area == CGame::currArea || dummy->m_area == AREA_EVERYWHERE)) {
			if ((dummy->GetPosition() - playerPos).Magnitude() < 80.0f)
				ConvertToRealObject((CDummyObject*)dummy);
		}
	}

	int pedPoolSize = CPools::GetPedPool()->GetSize();
#ifndef SQUEEZE_PERFORMANCE
	for (int poolIndex = pedPoolSize-1; poolIndex >= 0; poolIndex--) {
#else
	for (int poolIndex = (pedPoolSize * (frameMod32 + 1) / 32) - 1; poolIndex >= pedPoolSize * frameMod32 / 32; poolIndex--) {
#endif
		CPed *ped = CPools::GetPedPool()->GetSlot(poolIndex);

		if (ped && !ped->IsPlayer() && ped->CanBeDeleted() && !ped->bInVehicle) {
			uint32 timeSinceDeath = CTimer::GetTimeInMilliseconds() - ped->m_bloodyFootprintCountOrDeathTime;
			if (ped->m_nPedState == PED_DEAD && (timeSinceDeath > 30000 || CDarkel::FrenzyOnGoing() && timeSinceDeath > 15000))
				ped->bFadeOut = true;

			if (ped->bFadeOut && CVisibilityPlugins::GetClumpAlpha(ped->GetClump()) == 0) {
				RemovePed(ped);
				continue;
			}

			float dist = (ped->GetPosition() - playerPos).Magnitude2D();
			bool pedIsFarAway = false;

			if (ped->IsGangMember())
				dist -= 30.0f;
			else if (ped->bDeadPedInFrontOfCar && ped->m_vehicleInAccident)
				dist = 0.0f;

			if (PedCreationDistMultiplier() * (PED_REMOVE_DIST_SPECIAL * TheCamera.GenerationDistMultiplier) < dist ||
				(!ped->bCullExtraFarAway && PedCreationDistMultiplier() * PED_REMOVE_DIST * TheCamera.GenerationDistMultiplier < dist)) {
				pedIsFarAway = true;

			} else if (PedCreationDistMultiplier() * (MIN_CREATION_DIST + CREATION_RANGE) * OFFSCREEN_CREATION_MULT < dist) {
				if (CTimer::GetTimeInMilliseconds() > ped->m_nExtendedRangeTimer && !ped->GetIsOnScreen()) {
					if (TheCamera.Cams[TheCamera.ActiveCam].Mode != CCam::MODE_SNIPER
						&& TheCamera.Cams[TheCamera.ActiveCam].Mode != CCam::MODE_SNIPER_RUNABOUT
						&& TheCamera.Cams[TheCamera.ActiveCam].Mode != CCam::MODE_CAMERA
						&& !TheCamera.Cams[TheCamera.ActiveCam].LookingLeft
						&& !TheCamera.Cams[TheCamera.ActiveCam].LookingRight
						&& !TheCamera.Cams[TheCamera.ActiveCam].LookingBehind) {
						pedIsFarAway = true;
					}
				}
						
			} else {
				ped->m_nExtendedRangeTimer = ped->m_nPedType == PEDTYPE_COP ? CTimer::GetTimeInMilliseconds() + 10000 : CTimer::GetTimeInMilliseconds() + 4000;
			}

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
	if (TheCamera.IsSphereVisible(pos, 2.0f) && MIN_CREATION_DIST * PedCreationDistMultiplier() > (pos - FindPlayerPed()->GetPosition()).Magnitude2D()) {
			return nil;
	}

	bool found;
	float z = CWorld::FindGroundZFor3DCoord(pos.x, pos.y, pos.z, &found) + 1.0f;
	if (!found)
		return nil;
	z = Max(z, pos.z);
	if (!CModelInfo::GetModelInfo(MI_MALE01)->GetRwObject())
		return nil;
	CPed* pPed = CPopulation::AddPed(PEDTYPE_CIVMALE, MI_MALE01, pos);
	pPed->SetDie();
	pPed->m_nPedMoney = 0;
	pPed->bDeadPedInFrontOfCar = true;
	pPed->m_vehicleInAccident = pCulprit;
	pCulprit->RegisterReference((CEntity**)&pPed->m_vehicleInAccident);
	CEntity* pEntities[3] = { 0 };
	if (!CPedPlacement::IsPositionClearForPed(pos, 2.0f, 3, pEntities)) {
		for (int i = 0; i < 3; i++) {
			if (pEntities[i] && pEntities[i] != pCulprit && pEntities[i] != pPed) {
				RemovePed(pPed);
				return nil;
			}
		}
	}
	CColPoint colpts[MAX_COLLISION_POINTS];
	if (CCollision::ProcessColModels(pCulprit->GetMatrix(), *pCulprit->GetColModel(), pPed->GetMatrix(), *pPed->GetColModel(), colpts, nil, nil)) {
		RemovePed(pPed);
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

bool
CPopulation::CanJeerAtStripper(int32 model)
{
	return model == MI_WMOBE || model == MI_WMYBE || model == MI_WMOST || model == MI_BMYBB;
}

void
CPopulation::RemovePedsIfThePoolGetsFull(void)
{
	if ((CTimer::GetFrameCounter() & 7) == 5) {
		if (CPools::GetPedPool()->GetNoOfFreeSpaces() < 8) {
			CPed *closestPed = nil;
			float closestDist = 10000000.0;
			int poolSize = CPools::GetPedPool()->GetSize();
			for (int i = poolSize - 1; i >= 0; i--) {
				CPed* ped = CPools::GetPedPool()->GetSlot(i);
				if (ped && ped->CanBeDeleted()) {
					float dist = (TheCamera.GetPosition() - ped->GetPosition()).Magnitude();
					if (dist < closestDist) {
						closestDist = dist;
						closestPed = ped;
					}
				}
			}
			if (closestPed) {
				RemovePed(closestPed);
			}
		}
	}
}

bool
CPopulation::IsMale(int32 model)
{
	switch (model) {
		case MI_HMYST:
		case MI_HMOST:
		case MI_HMYRI:
		case MI_HMORI:
		case MI_HMYBE:
		case MI_HMOBE:
		case MI_HMOTR:
		case MI_HMYAP:
		case MI_HMOCA:
		case MI_BMODK:
		case MI_BMYKR:
		case MI_BMYST:
		case MI_BMOST:
		case MI_BMYRI:
		case MI_BMYBE:
		case MI_BMOBE:
		case MI_BMYBU:
		case MI_BMOTR:
		case MI_BMYPI:
		case MI_BMYBB:
		case MI_WMYCR:
		case MI_WMYST:
		case MI_WMOST:
		case MI_WMYRI:
		case MI_WMORI:
		case MI_WMYBE:
		case MI_WMOBE:
		case MI_WMYCW:
		case MI_WMYGO:
		case MI_WMOGO:
		case MI_WMYLG:
		case MI_WMYBU:
		case MI_WMOBU:
		case MI_WMOTR:
		case MI_WMYPI:
		case MI_WMOCA:
		case MI_WMYJG:
		case MI_WMYSK:

		// BUG? Why no JMOTO?
			return true;
		default:
			return false;
	}
}

bool
CPopulation::IsFemale(int32 model)
{
	switch (model) {
		case MI_HFYST:
		case MI_HFOST:
		case MI_HFYRI:
		case MI_HFORI:
		case MI_HFYBE:
		case MI_HFOBE:
		case MI_HFYBU:
		case MI_HFYMD:
		case MI_HFYCG:
		case MI_HFYPR:
		case MI_HFOTR:
		case MI_BFYST:
		case MI_BFOST:
		case MI_BFYRI:
		case MI_BFORI:
		case MI_BFYBE:
		case MI_BFOBE:
		case MI_BFYPR:
		case MI_BFOTR:
		case MI_WFYST:
		case MI_WFOST:
		case MI_WFYRI:
		case MI_WFORI:
		case MI_WFYBE:
		case MI_WFOBE:
		case MI_WFOGO:
		case MI_WFYLG:
		case MI_WFYBU:
		case MI_WFYPR:
		case MI_WFOTR:
		case MI_WFYJG:
		case MI_WFYSK:
		case MI_WFYSH:
		case MI_WFOSH:
		case MI_JFOTO:
			return true;
		default:
			return false;
	}
}

bool
CPopulation::IsSunbather(int32 model)
{
	switch (model) {
		case MI_HFYBE:
		case MI_HFOBE:
		case MI_HMYBE:
		case MI_HMOBE:
		case MI_BFYBE:
		case MI_BMYBE:
		case MI_BFOBE:
		case MI_BMOBE:
		case MI_WFYBE:
		case MI_WMYBE:
		case MI_WFOBE:
		case MI_WMOBE:
			return true;
		default:
			return false;
	}
}

int32
CPopulation::ComputeRandomisedGangSize(void)
{
	return CGeneral::GetRandomNumberInRange(3, 6);
}

bool
CPopulation::CanSolicitPlayerInCar(int32 model)
{
	return model == MI_HFYPR || model == MI_BFYPR || model == MI_WFYPR;
}

bool
CPopulation::CanSolicitPlayerOnFoot(int32 model)
{
	return model == MI_HFYMD || model == MI_HFYCG || model == MI_BFOTR || model == MI_BMOTR || model == MI_WFOTR || model == MI_WMOTR;
}

bool
CPopulation::IsSecurityGuard(ePedType pedType)
{
	return pedType == PEDTYPE_GANG5;
}

void
CPopulation::ChooseCivilianCoupleOccupations(int32 group, int32& man, int32& woman)
{
	man = -1;
	woman = -1;
	
	for (int i = 0; i < 8; i++) {
		if (man > -1)
			break;

		int32 model = ms_pPedGroups[group].models[CGeneral::GetRandomNumberInRange(0, NUMMODELSPERPEDGROUP)];
		if (man == -1 && IsMale(model) && ((CPedModelInfo*)CModelInfo::GetModelInfo(model))->m_pedType == PEDTYPE_CIVMALE) {
			man = model;
		}
	}

	if (man != -1) {
		int32 model;
		for (int i = 0; i < NUMMODELSPERPEDGROUP; i++) {
			model = ms_pPedGroups[group].models[i];
			if (IsFemale(model)) {
				CPedModelInfo* womanModelInfo = (CPedModelInfo*)CModelInfo::GetModelInfo(model);
				if (womanModelInfo->m_pedType == PEDTYPE_CIVFEMALE) {
					CPedModelInfo* manModelInfo = (CPedModelInfo*)CModelInfo::GetModelInfo(man);

					// If both are skater or not, finalize the decision
					if (manModelInfo && womanModelInfo) {
						if (manModelInfo->m_animGroup == womanModelInfo->m_animGroup) {
							if (manModelInfo->m_pedStatType != PEDSTAT_SKATER && womanModelInfo->m_pedStatType != PEDSTAT_SKATER)
								break;

							if (manModelInfo->m_pedStatType == PEDSTAT_SKATER && womanModelInfo->m_pedStatType == PEDSTAT_SKATER)
								break;
						}
					}
				}
			}
		}
		woman = model;
	}
}

void
CPopulation::PlaceGangMembers(ePedType pedType, int pedAmount, CVector const& coors)
{
	if (CGeneral::GetRandomNumberInRange(0.f, 1.f) < 0.333f) {
		PlaceGangMembersInFormation(pedType, pedAmount, coors);
	} else {
		PlaceGangMembersInCircle(pedType, pedAmount, coors);
	}
}

void
CPopulation::PlaceGangMembersInFormation(ePedType pedType, int pedAmount, CVector const& coors)
{
	CPed *createdPeds[5];

	if (!TheCamera.IsSphereVisible(coors, 3.0f) || MIN_CREATION_DIST * PedCreationDistMultiplier() <= (coors - FindPlayerPed()->GetPosition()).Magnitude2D()) {
		if (CPedPlacement::IsPositionClearForPed(coors, 3.0f, -1, 0)) {
			bool leaderFoundGround;
			float leaderGroundZ = CWorld::FindGroundZFor3DCoord(coors.x, coors.y, coors.z, &leaderFoundGround) + 1.0f;
			if (leaderFoundGround) {
				float finalZ = coors.z > leaderGroundZ ? coors.z : leaderGroundZ;
				int leaderModel = ChooseGangOccupation(pedType - PEDTYPE_GANG1);
				if (((CPedModelInfo*)CModelInfo::GetModelInfo(leaderModel))->GetRwObject()) {
					CPed *leader = AddPed(pedType, leaderModel, CVector(coors.x, coors.y, finalZ));
					if (leader) {
						leader->SetObjective(OBJECTIVE_NONE);
						leader->SetWanderPath(CGeneral::GetRandomNumberInRange(0, 8));
						leader->bIsLeader = true;
						if (CGangs::GetWillAttackPlayerWithCops(pedType))
							leader->bCanAttackPlayerWithCops = true;

						int pedIdx = 1;
						createdPeds[0] = leader;
						for (int i = 1; i < pedAmount; ++i) {
							int memberModel = ChooseGangOccupation(pedType - PEDTYPE_GANG1);
							if (!((CPedModelInfo*)CModelInfo::GetModelInfo(memberModel))->GetRwObject())
								continue;

							CPed* memberPed = AddPed(pedType, memberModel, CVector(coors.x, coors.y, finalZ));
							if (!memberPed)
								continue;

							memberPed->SetObjective(OBJECTIVE_FOLLOW_CHAR_IN_FORMATION, leader);
							memberPed->SetFormation((eFormation)i);
							CVector formationPos = memberPed->GetFormationPosition();
							CVector finalFormationPos = formationPos;
							bool formationFoundGround;
							float formationGroundZ = CWorld::FindGroundZFor3DCoord(formationPos.x, formationPos.y, 1.0f + formationPos.z, &formationFoundGround) + 1.0f;

							finalFormationPos.z = Max(finalFormationPos.z, formationGroundZ);
							if (formationFoundGround) {
								if (Abs(finalFormationPos.z - leader->GetPosition().z) <= 1.0f) {
									if (CWorld::GetIsLineOfSightClear(finalFormationPos, leader->GetPosition(), true, false, false, false, false, false, false)) {
										memberPed->SetPosition(finalFormationPos);
										createdPeds[pedIdx++] = memberPed;
										if (CGangs::GetWillAttackPlayerWithCops(pedType))
											leader->bCanAttackPlayerWithCops = true;

										CVisibilityPlugins::SetClumpAlpha(memberPed->GetClump(), 0);
										continue;
									}
								}
							}
							RemovePed(memberPed);
						}
						if (pedIdx >= 3) {
							for (int j = 1; j < pedIdx; ++j)
								createdPeds[j]->SetLeader(createdPeds[0]);

						} else {
							for (int k = 0; k < pedIdx; ++k) {
								RemovePed(createdPeds[k]);
							}
						}
					}
				}
			}
		}
	}
}

void
CPopulation::PlaceGangMembersInCircle(ePedType pedType, int pedAmount, CVector const& coors)
{
	CPed *createdPeds[5];

	if (pedAmount < 2)
		return;

	float circleSector = TWOPI / pedAmount;

	float circleR = Sqrt(0.5f / (1.0f - Cos(circleSector)));

	if (!TheCamera.IsSphereVisible(coors, circleR) ||
		MIN_CREATION_DIST * PedCreationDistMultiplier() <= (coors - FindPlayerPed()->GetPosition()).Magnitude2D()) {

		if (CPedPlacement::IsPositionClearForPed(coors, circleR, -1, 0)) {
			int pedIdx = 0;
			CVector leaderPos;

			for (int i = 0; i < pedAmount; i++) {	
				float angleMult = i + CGeneral::GetRandomNumberInRange(-0.2f, 0.2f);
				float randomR = circleR + CGeneral::GetRandomNumberInRange(-0.2f, 0.2f) * circleR;
				float xOffset = randomR * Cos(angleMult * circleSector);
				float yOffset = randomR * Sin(angleMult * circleSector);
				bool foundGround;
				float groundZ = CWorld::FindGroundZFor3DCoord(xOffset + coors.x, yOffset + coors.y, coors.z + 1.0, &foundGround) + 1.0f;
				if (foundGround) {
					CVector finalPos(coors.x + xOffset, coors.y + yOffset, coors.z > groundZ ? coors.z : groundZ);

					if (i == 0)
						leaderPos = finalPos;

					int gangModel = ChooseGangOccupation(pedType - PEDTYPE_GANG1);
					if (((CPedModelInfo*)CModelInfo::GetModelInfo(gangModel))->GetRwObject()) {
						CEntity* obstacles[6] = { nil, nil, nil, nil, nil, nil };
						CPedPlacement::IsPositionClearForPed(finalPos, CModelInfo::GetModelInfo(gangModel)->GetColModel()->boundingSphere.radius, ARRAY_SIZE(obstacles), obstacles);
						bool foundObstacle = false;
						for (int m = 0; m < ARRAY_SIZE(obstacles); m++) {
							CEntity* obstacle = obstacles[m];
							if (obstacle) {
								int n = 0;
								bool obstacleIsHarmless = false;
								for (int n = 0; n < pedIdx; n++) {
									if (obstacle == createdPeds[n])
										obstacleIsHarmless = true;
								}
								if (!obstacleIsHarmless) {
									foundObstacle = true;
									break;
								}
							}
						}
						bool memberCanSeeLeader = i == 0 ? true : CWorld::GetIsLineOfSightClear(finalPos, leaderPos, true, false, false, false, false, false, false);

						bool notTooCloseToLeader = i == 0 ? true : !(Abs(finalPos.z - leaderPos.z) < 1.0f);

						if (!foundObstacle && memberCanSeeLeader && notTooCloseToLeader) {
							CPed* newPed = AddPed(pedType, gangModel, finalPos);
							if (newPed) {
								createdPeds[pedIdx++] = newPed;
								float angle = CGeneral::GetRadianAngleBetweenPoints(
									coors.x, coors.y,
									finalPos.x, finalPos.y);
								newPed->m_fRotationDest = angle;
								newPed->m_fRotationCur = angle;
								if (CGangs::GetWillAttackPlayerWithCops(pedType))
									newPed->bCanAttackPlayerWithCops = true;

								CVisibilityPlugins::SetClumpAlpha(newPed->GetClump(), 0);
							}
							// No.
#ifndef FIX_BUGS
							else
								CWorld::Remove(nil);
#endif
						}
					}
				}
			}
			if (pedIdx >= 3) {
				for (int j = 0; j < pedIdx / 2; ++j) {
					createdPeds[j]->SetChat(createdPeds[pedIdx - 1 - j], 100000);
					createdPeds[pedIdx - 1 - j]->SetChat(createdPeds[j], 100000);
				}

				// Make that extra guy in the middle stand there(PED_UNKNOWN locks him) and do nothing :lmao:
				if (pedIdx % 2 != 0) {
					CPed *tmim = createdPeds[(pedIdx - 1) / 2];
					float angle = CGeneral::GetRadianAngleBetweenPoints(
						tmim->GetPosition().x, tmim->GetPosition().y,
						createdPeds[0]->GetPosition().x, createdPeds[0]->GetPosition().y);
					tmim->SetHeading(angle);
					tmim->SetPedState(PED_UNKNOWN);
				}
				createdPeds[0]->bIsLeader = true;

				for (int l = 1; l < pedIdx; ++l)
					createdPeds[l]->SetLeader(createdPeds[0]);

			} else {
				for (int k = 0; k < pedIdx; ++k) {
					RemovePed(createdPeds[k]);
				}
			}
		}
	}
}

void
CPopulation::PlaceCouple(ePedType manType, int32 manModel, ePedType womanType, int32 womanModel, CVector coors)
{
	// Homosexuality filter!!!! Homophobic R* >>>:(
	if (manType != PEDTYPE_CIVMALE || womanType != PEDTYPE_CIVFEMALE)
		return;

	if (!TheCamera.IsSphereVisible(coors, 1.5f) || MIN_CREATION_DIST * PedCreationDistMultiplier() <= (coors - FindPlayerPed()->GetPosition()).Magnitude2D()) {
		if (CPedPlacement::IsPositionClearForPed(coors, CModelInfo::GetModelInfo(manModel)->GetColModel()->boundingSphere.radius, -1, 0)) {
			bool manFoundGround;
			float manGroundZ = CWorld::FindGroundZFor3DCoord(coors.x, coors.y, coors.z, &manFoundGround) + 1.0f;
			if (manFoundGround) {
				CVector correctedManPos = coors;
				correctedManPos.z = Max(coors.z, manGroundZ);
				if (((CPedModelInfo*)CModelInfo::GetModelInfo(manModel))->GetRwObject()) {
					CPed *man = AddPed(PEDTYPE_CIVMALE, manModel, correctedManPos);
					if (man) {
						man->SetObjective(OBJECTIVE_NONE);
						man->SetWanderPath(CGeneral::GetRandomNumberInRange(0, 8));
						man->bIsLeader = true;
						CVisibilityPlugins::SetClumpAlpha(man->GetClump(), 0);

						if (((CPedModelInfo*)CModelInfo::GetModelInfo(womanModel))->GetRwObject()) {
							CPed* woman = AddPed(PEDTYPE_CIVFEMALE, womanModel, correctedManPos); // will set the correct position later
							if (woman) {
								woman->SetObjective(OBJECTIVE_FOLLOW_CHAR_IN_FORMATION, man);
								woman->SetFormation(FORMATION_RIGHT);

								CVector formationPos = woman->GetFormationPosition();
								CVector womanPos = formationPos;
								bool womanFoundGround;
								float formationGroundZ = CWorld::FindGroundZFor3DCoord(formationPos.x, formationPos.y, 1.0f + formationPos.z, &womanFoundGround) + 1.0f;

								if (womanFoundGround) {
									CVector correctedWomanPos = womanPos;
									correctedWomanPos.z = Max(womanPos.z, formationGroundZ);
									woman->SetPosition(correctedWomanPos);

									// What's the point of this??
									CEntity* obstacles[3];
									memcpy(obstacles, gCoupleObstacles, sizeof(gCoupleObstacles));

									CPedPlacement::IsPositionClearForPed(womanPos, CModelInfo::GetModelInfo(womanModel)->GetColModel()->boundingSphere.radius, ARRAY_SIZE(obstacles), obstacles);
									for (int i = 0; i < ARRAY_SIZE(obstacles); i++) {
										CEntity *obstacle = obstacles[i];
										if (obstacle) {

											// We found a real obstacle, so let's break and we can delete them...
											if (obstacle != man && obstacle != woman)
												break;
										}
										if (i == ARRAY_SIZE(obstacles) - 1) {
											CVisibilityPlugins::SetClumpAlpha(woman->GetClump(), 0);
											return;
										}
									}
								}
								RemovePed(woman);
								RemovePed(man);
							}
						}
					}
				}
			}
		}
	}
}

// Mostly copy paste of PlaceGangMembersInFormation.
void
CPopulation::PlaceMallPedsAsStationaryGroup(CVector const& coors, int32 group)
{
#ifdef FIX_BUGS
	CPed *createdPeds[6];
#else
	CPed *createdPeds[5];
#endif

	if (CGame::currArea != AREA_MALL)
		return;

	int pedAmount = CGeneral::GetRandomNumberInRange(0, 4) + 3;

	float circleSector = TWOPI / pedAmount;

	float circleR = Sqrt(0.5f / (1.0f - Cos(circleSector)));

	if (!TheCamera.IsSphereVisible(coors, circleR) ||
		MIN_CREATION_DIST * PedCreationDistMultiplier() <= (coors - FindPlayerPed()->GetPosition()).Magnitude2D()) {

		if (CPedPlacement::IsPositionClearForPed(coors, circleR, -1, 0)) {
			int pedIdx = 0;
			CVector leaderPos;

			for (int i = 0; i < pedAmount; i++) {	
				float angleMult = i + CGeneral::GetRandomNumberInRange(-0.2f, 0.2f);
				float randomR = circleR + CGeneral::GetRandomNumberInRange(-0.2f, 0.2f) * circleR;
				float xOffset = randomR * Cos(angleMult * circleSector);
				float yOffset = randomR * Sin(angleMult * circleSector);
				bool foundGround;
				float groundZ = CWorld::FindGroundZFor3DCoord(xOffset + coors.x, yOffset + coors.y, coors.z + 1.0, &foundGround) + 1.0f;
				if (foundGround) {
					CVector finalPos(coors.x + xOffset, coors.y + yOffset, coors.z > groundZ ? coors.z : groundZ);

					if (i == 0)
						leaderPos = finalPos;

					int pedModel = ChooseCivilianOccupation(group);
					CPedModelInfo *pedModelInfo = (CPedModelInfo*)CModelInfo::GetModelInfo(pedModel);
					if (pedModelInfo->GetRwObject()) {
						CEntity* obstacles[6] = { nil, nil, nil, nil, nil, nil };
						CPedPlacement::IsPositionClearForPed(finalPos, CModelInfo::GetModelInfo(pedModel)->GetColModel()->boundingSphere.radius, ARRAY_SIZE(obstacles), obstacles);
						bool foundObstacle = false;
						for (int m = 0; m < ARRAY_SIZE(obstacles); m++) {
							CEntity* obstacle = obstacles[m];
							if (obstacle) {
								int n = 0;
								bool obstacleIsHarmless = false;
								for (int n = 0; n < pedIdx; n++) {
									if (obstacle == createdPeds[n])
										obstacleIsHarmless = true;
								}
								if (!obstacleIsHarmless) {
									foundObstacle = true;
									break;
								}
							}
						}
						bool memberCanSeeLeader = i == 0 ? true : CWorld::GetIsLineOfSightClear(finalPos, leaderPos, true, false, false, false, false, false, false);

						bool notTooCloseToLeader = i == 0 ? true : !(Abs(finalPos.z - leaderPos.z) < 1.0f);

						if (!foundObstacle && memberCanSeeLeader && notTooCloseToLeader) {
							CPed *newPed = AddPed(pedModelInfo->m_pedType, pedModel, finalPos);
							if (newPed) {
								createdPeds[pedIdx++] = newPed;
								float angle = CGeneral::GetRadianAngleBetweenPoints(
									coors.x, coors.y,
									finalPos.x, finalPos.y);
								newPed->m_fRotationDest = angle;
								newPed->m_fRotationCur = angle;
								newPed->m_fearFlags = 0;
								CVisibilityPlugins::SetClumpAlpha(newPed->GetClump(), 0);
							}
							// No.
#ifndef FIX_BUGS
							else
								CWorld::Remove(nil);
#endif
						}
					}
				}
			}
			if (pedIdx >= 3) {
				for (int j = 0; j < pedIdx / 2; ++j) {
					createdPeds[j]->SetChat(createdPeds[pedIdx - 1 - j], 100000);
					createdPeds[pedIdx - 1 - j]->SetChat(createdPeds[j], 100000);
				}

				// Make that extra guy in the middle stand there(PED_UNKNOWN locks him) and do nothing :lmao:
				if (pedIdx % 2 != 0) {
					CPed *tmim = createdPeds[(pedIdx - 1) / 2];
					float angle = CGeneral::GetRadianAngleBetweenPoints(
						tmim->GetPosition().x, tmim->GetPosition().y,
						createdPeds[0]->GetPosition().x, createdPeds[0]->GetPosition().y);
					tmim->SetHeading(angle);
					tmim->SetPedState(PED_UNKNOWN);
				}
				createdPeds[0]->bIsLeader = true;

				for (int l = 1; l < pedIdx; ++l)
					createdPeds[l]->SetLeader(createdPeds[0]);

			} else {
				for (int k = 0; k < pedIdx; ++k) {
					RemovePed(createdPeds[k]);
				}
			}
		}
	}
}
