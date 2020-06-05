#include "common.h"

#include "GameLogic.h"
#include "Clock.h"
#include "Stats.h"
#include "Pickups.h"
#include "Timer.h"
#include "Streaming.h"
#include "CutsceneMgr.h"
#include "World.h"
#include "PlayerPed.h"
#include "Wanted.h"
#include "Camera.h"
#include "Messages.h"
#include "CarCtrl.h"
#include "Restart.h"
#include "Pad.h"
#include "References.h"
#include "Fire.h"
#include "Script.h"
#include "Garages.h"
#include "Population.h"
#include "General.h"
#include "DMAudio.h"
#include "Radar.h"
#include "Pools.h"
#include "Hud.h"
#include "Particle.h"
#include "ColStore.h"
#include "Automobile.h"

uint8 CGameLogic::ActivePlayers;
uint8 CGameLogic::ShortCutState;
CAutomobile* CGameLogic::pShortCutTaxi;
uint32 CGameLogic::NumAfterDeathStartPoints;
CVector CGameLogic::ShortCutStart;
float CGameLogic::ShortCutStartOrientation;
CVector CGameLogic::ShortCutDestination;
float CGameLogic::ShortCutDestinationOrientation;
uint32 CGameLogic::ShortCutTimer;
CVector CGameLogic::AfterDeathStartPoints[NUM_SHORTCUT_START_POINTS];
float CGameLogic::AfterDeathStartPointOrientation[NUM_SHORTCUT_START_POINTS];
CVector CGameLogic::ShortCutDropOffForMission;
float CGameLogic::ShortCutDropOffOrientationForMission;
bool CGameLogic::MissionDropOffReadyToBeUsed;

//--MIAMI: file done except TODO

#define SHORTCUT_TAXI_COST (9)

void
CGameLogic::InitAtStartOfGame()
{
	ActivePlayers = 1;
	ShortCutState = SHORTCUT_NONE;
	pShortCutTaxi = nil;
	NumAfterDeathStartPoints = 0;
}

void
CGameLogic::PassTime(uint32 time)
{
	int32 minutes, hours, days;

	minutes = time + CClock::GetMinutes();
	hours = CClock::GetHours();

	for (; minutes >= 60; minutes -= 60)
		hours++;

	if (hours > 23) {
		days = CStats::DaysPassed;
		for (; hours >= 24; hours -= 24)
			days++;
		CStats::DaysPassed = days;
	}

	CClock::SetGameClock(hours, minutes);
	CPickups::PassTime(time * 1000);
}

void 
CGameLogic::SortOutStreamingAndMemory(const CVector &pos)
{
	CTimer::Stop();
	CStreaming::FlushRequestList();
	CStreaming::DeleteRwObjectsAfterDeath(pos);
	CStreaming::RemoveUnusedModelsInLoadedList();
	CGame::DrasticTidyUpMemory(true);
	CWorld::Players[CWorld::PlayerInFocus].m_pPed->Undress("player");
	CStreaming::LoadSceneCollision(pos);
	CStreaming::LoadScene(pos);
	CWorld::Players[CWorld::PlayerInFocus].m_pPed->Dress();
	CTimer::Update();
}

void
CGameLogic::Update()
{
	CVector vecRestartPos;
	float fRestartFloat;

	if (CCutsceneMgr::IsCutsceneProcessing()) return;

	UpdateShortCut();
	CPlayerInfo &pPlayerInfo = CWorld::Players[CWorld::PlayerInFocus];

	switch (pPlayerInfo.m_WBState) {
	case WBSTATE_PLAYING:
		if (pPlayerInfo.m_pPed->m_nPedState == PED_DEAD) {
			pPlayerInfo.m_pPed->ClearAdrenaline();
			pPlayerInfo.KillPlayer();
		}
		if (pPlayerInfo.m_pPed->m_nPedState == PED_ARRESTED) {
			pPlayerInfo.m_pPed->ClearAdrenaline();
			pPlayerInfo.ArrestPlayer();
		}
		break;
	case WBSTATE_WASTED:
#ifdef MISSION_REPLAY
		if ((CTimer::GetTimeInMilliseconds() - pPlayerInfo.m_nWBTime > AddExtraDeathDelay() + 0x800) && (CTimer::GetPreviousTimeInMilliseconds() - pPlayerInfo.m_nWBTime <= AddExtraDeathDelay() + 0x800)) {
#else
		if ((CTimer::GetTimeInMilliseconds() - pPlayerInfo.m_nWBTime > 0x800) && (CTimer::GetPreviousTimeInMilliseconds() - pPlayerInfo.m_nWBTime <= 0x800)) {
#endif
			TheCamera.SetFadeColour(200, 200, 200);
			TheCamera.Fade(2.0f, FADE_OUT);
		}

#ifdef MISSION_REPLAY
		if (CTimer::GetTimeInMilliseconds() - pPlayerInfo.m_nWBTime >= AddExtraDeathDelay() + 0x1000) {
#else
		if (CTimer::GetTimeInMilliseconds() - pPlayerInfo.m_nWBTime >= 0x1000) {
#endif
			pPlayerInfo.m_WBState = WBSTATE_PLAYING;
			if (pPlayerInfo.m_bGetOutOfHospitalFree) {
				pPlayerInfo.m_bGetOutOfHospitalFree = false;
			} else {
				pPlayerInfo.m_nMoney = Max(0, pPlayerInfo.m_nMoney - 100);
				pPlayerInfo.m_pPed->ClearWeapons();
			}

			if (pPlayerInfo.m_pPed->bInVehicle) {
				CVehicle *pVehicle = pPlayerInfo.m_pPed->m_pMyVehicle;
				if (pVehicle != nil) {
					if (pVehicle->pDriver == pPlayerInfo.m_pPed) {
						pVehicle->pDriver = nil;
						if (pVehicle->GetStatus() != STATUS_WRECKED)
							pVehicle->SetStatus(STATUS_ABANDONED);
					} else
						pVehicle->RemovePassenger(pPlayerInfo.m_pPed);
				}
			}
			CEventList::Initialise();
			CMessages::ClearMessages();
			CCarCtrl::ClearInterestingVehicleList();
			CWorld::ClearExcitingStuffFromArea(pPlayerInfo.GetPos(), 4000.0f, 1);
			CRestart::FindClosestHospitalRestartPoint(pPlayerInfo.GetPos(), &vecRestartPos, &fRestartFloat);
			CRestart::OverrideHospitalLevel = LEVEL_NONE;
			CRestart::OverridePoliceStationLevel = LEVEL_NONE;
			PassTime(720);
			RestorePlayerStuffDuringResurrection(pPlayerInfo.m_pPed, vecRestartPos, fRestartFloat);
			AfterDeathArrestSetUpShortCutTaxi();
			SortOutStreamingAndMemory(pPlayerInfo.GetPos());
			TheCamera.m_fCamShakeForce = 0.0f;
			TheCamera.SetMotionBlur(0, 0, 0, 0, MBLUR_NONE);
			CPad::GetPad(0)->StopShaking(0);
			CReferences::RemoveReferencesToPlayer();
			CPopulation::m_CountDownToPedsAtStart = 10;
			CCarCtrl::CountDownToCarsAtStart = 10;
			CPad::GetPad(CWorld::PlayerInFocus)->DisablePlayerControls = PLAYERCONTROL_ENABLED;
			if (CRestart::bFadeInAfterNextDeath) { 
				TheCamera.SetFadeColour(200, 200, 200);
				TheCamera.Fade(4.0f, FADE_IN);
			} else
				CRestart::bFadeInAfterNextDeath = true;
		}
		break;
	case WBSTATE_BUSTED:
#ifdef MISSION_REPLAY
		if ((CTimer::GetTimeInMilliseconds() - pPlayerInfo.m_nWBTime > AddExtraDeathDelay() + 0x800) && (CTimer::GetPreviousTimeInMilliseconds() - pPlayerInfo.m_nWBTime <= AddExtraDeathDelay() + 0x800)) {
#else
		if ((CTimer::GetTimeInMilliseconds() - pPlayerInfo.m_nWBTime > 0x800) && (CTimer::GetPreviousTimeInMilliseconds() - pPlayerInfo.m_nWBTime <= 0x800)) {
#endif
			TheCamera.SetFadeColour(0, 0, 0);
			TheCamera.Fade(2.0f, FADE_OUT);
		}


		if (!CTheScripts::IsPlayerOnAMission() && pPlayerInfo.m_nBustedAudioStatus == 0) {
			if (CGeneral::GetRandomNumberInRange(0, 4) == 0)
				pPlayerInfo.m_nBustedAudioStatus = BUSTEDAUDIO_DONE;
			else {
				pPlayerInfo.m_nBustedAudioStatus = BUSTEDAUDIO_LOADING;
				char name[12];
				sprintf(name, pPlayerInfo.m_nCurrentBustedAudio >= 10 ? "bust_%d" : "bust_0%d", pPlayerInfo.m_nCurrentBustedAudio);
				DMAudio.ClearMissionAudio(); // TODO(MIAMI): argument is 0
				DMAudio.PreloadMissionAudio(name); // TODO(MIAMI): argument is 0
				pPlayerInfo.m_nCurrentBustedAudio = pPlayerInfo.m_nCurrentBustedAudio % 28 + 1; // enum? const? TODO
			}
		}
		if (CTimer::GetTimeInMilliseconds() - pPlayerInfo.m_nWBTime > 4000 &&
			pPlayerInfo.m_nBustedAudioStatus == BUSTEDAUDIO_LOADING &&
			DMAudio.GetMissionAudioLoadingStatus() == 1) { // TODO: argument is 0
			DMAudio.PlayLoadedMissionAudio(); // TODO: argument is 0
			pPlayerInfo.m_nBustedAudioStatus = BUSTEDAUDIO_DONE;
		}
		
#ifdef MISSION_REPLAY
		if (CTimer::GetTimeInMilliseconds() - pPlayerInfo.m_nWBTime >= AddExtraDeathDelay() + 0x1000) {
#else
		if (CTimer::GetTimeInMilliseconds() - pPlayerInfo.m_nWBTime >= 0x1000) {
#endif
#ifdef FIX_BUGS
			pPlayerInfo.m_nBustedAudioStatus = BUSTEDAUDIO_NONE;
#endif
			pPlayerInfo.m_WBState = WBSTATE_PLAYING;
			int takeMoney;

			switch (pPlayerInfo.m_pPed->m_pWanted->m_nWantedLevel) {
			case 0:
			case 1:
				takeMoney = 100;
				break;
			case 2:
				takeMoney = 200;
				break;
			case 3:
				takeMoney = 400;
				break;
			case 4:
				takeMoney = 600;
				break;
			case 5:
				takeMoney = 900;
				break;
			case 6:
				takeMoney = 1500;
				break;
			}
			if (pPlayerInfo.m_bGetOutOfJailFree) {
				pPlayerInfo.m_bGetOutOfJailFree = false;
			} else {
				pPlayerInfo.m_nMoney = Max(0, pPlayerInfo.m_nMoney - takeMoney);
				pPlayerInfo.m_pPed->ClearWeapons();
			}

			if (pPlayerInfo.m_pPed->bInVehicle) {
				CVehicle *pVehicle = pPlayerInfo.m_pPed->m_pMyVehicle;
				if (pVehicle != nil) {
					if (pVehicle->pDriver == pPlayerInfo.m_pPed) {
						pVehicle->pDriver = nil;
						if (pVehicle->GetStatus() != STATUS_WRECKED)
							pVehicle->SetStatus(STATUS_ABANDONED);
					}
					else
						pVehicle->RemovePassenger(pPlayerInfo.m_pPed);
				}
			}
			CEventList::Initialise();
			CMessages::ClearMessages();
			CCarCtrl::ClearInterestingVehicleList();
			CWorld::ClearExcitingStuffFromArea(pPlayerInfo.GetPos(), 4000.0f, 1);
			CRestart::FindClosestPoliceRestartPoint(pPlayerInfo.GetPos(), &vecRestartPos, &fRestartFloat);
			CRestart::OverrideHospitalLevel = LEVEL_NONE;
			CRestart::OverridePoliceStationLevel = LEVEL_NONE;
			PassTime(720);
			RestorePlayerStuffDuringResurrection(pPlayerInfo.m_pPed, vecRestartPos, fRestartFloat);
			AfterDeathArrestSetUpShortCutTaxi();
			pPlayerInfo.m_pPed->ClearWeapons();
			SortOutStreamingAndMemory(pPlayerInfo.GetPos());
			TheCamera.m_fCamShakeForce = 0.0f;
			TheCamera.SetMotionBlur(0, 0, 0, 0, MBLUR_NONE);
			CPad::GetPad(0)->StopShaking(0);
			CReferences::RemoveReferencesToPlayer();
			CPopulation::m_CountDownToPedsAtStart = 10;
			CCarCtrl::CountDownToCarsAtStart = 10;
			CPad::GetPad(CWorld::PlayerInFocus)->DisablePlayerControls = PLAYERCONTROL_ENABLED;
			if (CRestart::bFadeInAfterNextArrest) {
				TheCamera.SetFadeColour(0, 0, 0);
				TheCamera.Fade(4.0f, FADE_IN);
			} else
				CRestart::bFadeInAfterNextArrest = true;
		}
		break;
	case WBSTATE_FAILED_CRITICAL_MISSION:
#ifdef MISSION_REPLAY
		if ((CTimer::GetTimeInMilliseconds() - pPlayerInfo.m_nWBTime > AddExtraDeathDelay() + 0x800) && (CTimer::GetPreviousTimeInMilliseconds() - pPlayerInfo.m_nWBTime <= AddExtraDeathDelay() + 0x800)) {
#else
		if ((CTimer::GetTimeInMilliseconds() - pPlayerInfo.m_nWBTime > 0x800) && (CTimer::GetPreviousTimeInMilliseconds() - pPlayerInfo.m_nWBTime <= 0x800)) {
#endif
			TheCamera.SetFadeColour(0, 0, 0);
			TheCamera.Fade(2.0f, FADE_OUT);
		}
#ifdef MISSION_REPLAY
		if (CTimer::GetTimeInMilliseconds() - pPlayerInfo.m_nWBTime >= AddExtraDeathDelay() + 0x1000) {
#else
		if (CTimer::GetTimeInMilliseconds() - pPlayerInfo.m_nWBTime >= 0x1000) {
#endif
			pPlayerInfo.m_WBState = WBSTATE_PLAYING;
			if (pPlayerInfo.m_pPed->bInVehicle) {
				CVehicle *pVehicle = pPlayerInfo.m_pPed->m_pMyVehicle;
				if (pVehicle != nil) {
					if (pVehicle->pDriver == pPlayerInfo.m_pPed) {
						pVehicle->pDriver = nil;
						if (pVehicle->GetStatus() != STATUS_WRECKED)
							pVehicle->SetStatus(STATUS_ABANDONED);
					} else
						pVehicle->RemovePassenger(pPlayerInfo.m_pPed);
				}
			}
			CEventList::Initialise();
			CMessages::ClearMessages();
			CCarCtrl::ClearInterestingVehicleList();
			CWorld::ClearExcitingStuffFromArea(pPlayerInfo.GetPos(), 4000.0f, 1);
			CRestart::FindClosestPoliceRestartPoint(pPlayerInfo.GetPos(), &vecRestartPos, &fRestartFloat);
			CRestart::OverridePoliceStationLevel = LEVEL_NONE;
			CRestart::OverrideHospitalLevel = LEVEL_NONE;
			RestorePlayerStuffDuringResurrection(pPlayerInfo.m_pPed, vecRestartPos, fRestartFloat);
			SortOutStreamingAndMemory(pPlayerInfo.GetPos());
			TheCamera.m_fCamShakeForce = 0.0f;
			TheCamera.SetMotionBlur(0, 0, 0, 0, MBLUR_NONE);
			CPad::GetPad(0)->StopShaking(0);
			CReferences::RemoveReferencesToPlayer();
			CPopulation::m_CountDownToPedsAtStart = 10;
			CCarCtrl::CountDownToCarsAtStart = 10;
			CPad::GetPad(CWorld::PlayerInFocus)->DisablePlayerControls = PLAYERCONTROL_ENABLED;
			TheCamera.SetFadeColour(0, 0, 0);
			TheCamera.Fade(4.0f, FADE_IN);
		}
		break;
	case 4:
		return;
	}
}

void
CGameLogic::RestorePlayerStuffDuringResurrection(CPlayerPed *pPlayerPed, CVector pos, float angle)
{
	ClearShortCut();
	CPlayerInfo* pPlayerInfo = pPlayerPed->GetPlayerInfoForThisPlayerPed();
	pPlayerPed->m_fHealth = pPlayerInfo->m_nMaxHealth;
	pPlayerPed->m_fArmour = 0.0f;
	pPlayerPed->bIsVisible = true;
	pPlayerPed->m_bloodyFootprintCountOrDeathTime = 0;
	pPlayerPed->bDoBloodyFootprints = false;
	//TODO(MIAMI): clear drunk stuff
	pPlayerPed->ClearAdrenaline();
	pPlayerPed->m_fCurrentStamina = pPlayerPed->m_fMaxStamina;
	if (pPlayerPed->m_pFire)
		pPlayerPed->m_pFire->Extinguish();
	pPlayerPed->bInVehicle = false;
	pPlayerPed->m_pMyVehicle = nil;
	pPlayerPed->m_pVehicleAnim = nil;
	pPlayerPed->m_pWanted->Reset();
	pPlayerPed->bCancelEnteringCar = false;
	pPlayerPed->RestartNonPartialAnims();
	pPlayerInfo->MakePlayerSafe(false);
	pPlayerPed->bRemoveFromWorld = false;
	pPlayerPed->ClearWeaponTarget();
	pPlayerPed->SetInitialState();
	CCarCtrl::ClearInterestingVehicleList();
	pPlayerPed->Teleport(pos + CVector(0.0f, 0.0f, 1.0f));
	pPlayerPed->SetMoveSpeed(0.0f, 0.0f, 0.0f);
	pPlayerPed->m_fRotationCur = DEGTORAD(angle);
	pPlayerPed->m_fRotationDest = pPlayerPed->m_fRotationCur;
	pPlayerPed->SetHeading(pPlayerPed->m_fRotationCur);
	CTheScripts::ClearSpaceForMissionEntity(pos, pPlayerPed);
	CWorld::ClearExcitingStuffFromArea(pos, 4000.0, 1);
	pPlayerPed->RestoreHeadingRate();
	CGame::currArea = AREA_MAIN_MAP;
	//CStreaming::RemoveBuildingsNotInArea(0); // TODO(MIAMI)
	TheCamera.SetCameraDirectlyInFrontForFollowPed_CamOnAString();
	TheCamera.Restore();
	CReferences::RemoveReferencesToPlayer();
	CGarages::PlayerArrestedOrDied();
	CStats::CheckPointReachedUnsuccessfully();
	CWorld::Remove(pPlayerPed);
	CWorld::Add(pPlayerPed);
	//CHud::ResetWastedText() // TODO(MIAMI)
	CStreaming::StreamZoneModels(pos);
	clearWaterDrop = true;
}

void
CGameLogic::ClearShortCut()
{
	if (pShortCutTaxi) {
		if (pShortCutTaxi->VehicleCreatedBy == MISSION_VEHICLE) {
			pShortCutTaxi->VehicleCreatedBy = RANDOM_VEHICLE;
			--CCarCtrl::NumMissionCars;
			++CCarCtrl::NumRandomCars;
		}
		CRadar::ClearBlipForEntity(BLIP_CAR, CPools::GetVehiclePool()->GetIndex(pShortCutTaxi));
		pShortCutTaxi = nil;
	}
	CPad::GetPad(0)->SetEnablePlayerControls(PLAYERCONTROL_SHORTCUT_TAXI);
}

void
CGameLogic::SetUpShortCut(CVector vStartPos, float fStartAngle, CVector vEndPos, float fEndAngle)
{
	ClearShortCut();
	ShortCutState = SHORTCUT_INIT;
	ShortCutStart = vStartPos;
	ShortCutStartOrientation = fStartAngle;
	ShortCutDestination = vEndPos;
	ShortCutDestinationOrientation = fEndAngle;
	CStreaming::RequestModel(MI_KAUFMAN, 0);
}

void
CGameLogic::AbandonShortCutIfTaxiHasBeenMessedWith()
{
	if (!pShortCutTaxi)
		return;
	if (pShortCutTaxi->pDriver == nil ||
		pShortCutTaxi->pDriver->DyingOrDead() ||
		pShortCutTaxi->pDriver->GetPedState() == PED_DRAG_FROM_CAR ||
		pShortCutTaxi->pDriver->GetPedState() == PED_ON_FIRE ||
		pShortCutTaxi->pDriver->m_objective == OBJECTIVE_LEAVE_CAR_AND_DIE ||
		pShortCutTaxi->m_fHealth < 250.0f ||
		pShortCutTaxi->bRenderScorched)
		ClearShortCut();
}

void
CGameLogic::AbandonShortCutIfPlayerMilesAway()
{
	if (!pShortCutTaxi)
		return;
	if ((FindPlayerCoors() - pShortCutTaxi->GetPosition()).Magnitude() > 120.0f)
		ClearShortCut();
}

void
CGameLogic::UpdateShortCut()
{
	switch (ShortCutState) {
	case SHORTCUT_INIT:
		if (!CStreaming::HasModelLoaded(MI_KAUFMAN)) {
			CStreaming::RequestModel(MI_KAUFMAN, 0);
			return;
		}
		pShortCutTaxi = new CAutomobile(MI_KAUFMAN, RANDOM_VEHICLE);
		if (!pShortCutTaxi)
			return;
		pShortCutTaxi->SetPosition(ShortCutStart);
		pShortCutTaxi->SetHeading(DEGTORAD(ShortCutStartOrientation));
		pShortCutTaxi->PlaceOnRoadProperly();
		pShortCutTaxi->SetStatus(STATUS_PHYSICS);
		pShortCutTaxi->AutoPilot.m_nCarMission = MISSION_STOP_FOREVER;
		pShortCutTaxi->AutoPilot.m_nCruiseSpeed = 0;
		pShortCutTaxi->SetUpDriver();
		pShortCutTaxi->VehicleCreatedBy = MISSION_VEHICLE;
		++CCarCtrl::NumMissionCars;
		--CCarCtrl::NumRandomCars;
		CTheScripts::ClearSpaceForMissionEntity(ShortCutStart, pShortCutTaxi);
		CWorld::Add(pShortCutTaxi);
		CRadar::SetEntityBlip(BLIP_CAR, CPools::GetVehiclePool()->GetIndex(pShortCutTaxi), 0, BLIP_DISPLAY_MARKER_ONLY);
		ShortCutState = SHORTCUT_IDLE;
		break;
	case SHORTCUT_IDLE:
		if (FindPlayerPed()->m_objective == OBJECTIVE_ENTER_CAR_AS_DRIVER && FindPlayerPed()->m_carInObjective == pShortCutTaxi) {
			CPad::GetPad(0)->SetDisablePlayerControls(PLAYERCONTROL_SHORTCUT_TAXI);
			FindPlayerPed()->SetObjective(OBJECTIVE_ENTER_CAR_AS_PASSENGER, pShortCutTaxi);
			ShortCutState = SHORTCUT_GETTING_IN;
		}
		AbandonShortCutIfTaxiHasBeenMessedWith();
		AbandonShortCutIfPlayerMilesAway();
		break;
	case SHORTCUT_GETTING_IN:
		if (pShortCutTaxi->pPassengers[0] == FindPlayerPed() ||
			pShortCutTaxi->pPassengers[1] == FindPlayerPed() ||
			pShortCutTaxi->pPassengers[2] == FindPlayerPed()) {
			pShortCutTaxi->AutoPilot.m_nTempAction = TEMPACT_GOFORWARD;
			pShortCutTaxi->AutoPilot.m_nTimeTempAction = CTimer::GetTimeInMilliseconds() + 2500;
			TheCamera.SetFadeColour(0, 0, 0);
			TheCamera.Fade(2.5f, 0);
			ShortCutState = SHORTCUT_TRANSITION;
			ShortCutTimer = CTimer::GetTimeInMilliseconds() + 3000;
			CMessages::AddBigMessage(TheText.Get("TAXI"), 4500, 1);
		}
		AbandonShortCutIfTaxiHasBeenMessedWith();
		break;
	case SHORTCUT_TRANSITION:
		if (CTimer::GetTimeInMilliseconds() > ShortCutTimer) {
			CTimer::Suspend();
			CColStore::RequestCollision(ShortCutDestination);
			CStreaming::LoadSceneCollision(ShortCutDestination);
			CStreaming::LoadScene(ShortCutDestination);
			CTheScripts::ClearSpaceForMissionEntity(ShortCutDestination, pShortCutTaxi);
			pShortCutTaxi->Teleport(ShortCutDestination);
			pShortCutTaxi->SetHeading(DEGTORAD(ShortCutDestinationOrientation));
			pShortCutTaxi->PlaceOnRoadProperly();
			pShortCutTaxi->SetMoveSpeed(pShortCutTaxi->GetForward() * 0.4f);
			ShortCutTimer = CTimer::GetTimeInMilliseconds() + 1500;
			TheCamera.SetFadeColour(0, 0, 0);
			TheCamera.Fade(1.0f, 1);
			ShortCutState = SHORTCUT_ARRIVING;
			CTimer::Resume();
		}
		break;
	case SHORTCUT_ARRIVING:
		if (CTimer::GetTimeInMilliseconds() > ShortCutTimer) {
			CWorld::Players[CWorld::PlayerInFocus].m_nMoney = Max(0, CWorld::Players[CWorld::PlayerInFocus].m_nMoney - SHORTCUT_TAXI_COST);
			FindPlayerPed()->SetObjective(OBJECTIVE_LEAVE_VEHICLE, pShortCutTaxi);
			FindPlayerPed()->m_carInObjective = pShortCutTaxi;
			ShortCutState = SHORTCUT_GETTING_OUT;
		}
		AbandonShortCutIfTaxiHasBeenMessedWith();
		break;
	case SHORTCUT_GETTING_OUT:
		if (pShortCutTaxi->pPassengers[0] != FindPlayerPed() &&
			pShortCutTaxi->pPassengers[1] != FindPlayerPed() &&
			pShortCutTaxi->pPassengers[2] != FindPlayerPed()) {
			CPad::GetPad(0)->SetEnablePlayerControls(PLAYERCONTROL_SHORTCUT_TAXI);
			pShortCutTaxi->AutoPilot.m_nCarMission = MISSION_CRUISE;
			pShortCutTaxi->AutoPilot.m_nCruiseSpeed = 18;
			CCarCtrl::JoinCarWithRoadSystem(pShortCutTaxi);
			pShortCutTaxi->VehicleCreatedBy = RANDOM_VEHICLE;
			++CCarCtrl::NumRandomCars;
			--CCarCtrl::NumMissionCars;
			CRadar::ClearBlipForEntity(BLIP_CAR, CPools::GetVehiclePool()->GetIndex(pShortCutTaxi));
			ShortCutState = SHORTCUT_NONE;
			pShortCutTaxi = nil;
		}
		AbandonShortCutIfTaxiHasBeenMessedWith();
		break;
	}
}

void
CGameLogic::AddShortCutPointAfterDeath(CVector point, float angle)
{
	if (NumAfterDeathStartPoints >= NUM_SHORTCUT_START_POINTS)
		return;
	AfterDeathStartPoints[NumAfterDeathStartPoints] = point;
	AfterDeathStartPointOrientation[NumAfterDeathStartPoints] = angle;
	NumAfterDeathStartPoints++;
}

void
CGameLogic::AddShortCutDropOffPointForMission(CVector point, float angle)
{
	ShortCutDropOffForMission = point;
	ShortCutDropOffOrientationForMission = angle;
	MissionDropOffReadyToBeUsed = true;
}

void
CGameLogic::RemoveShortCutDropOffPointForMission()
{
	MissionDropOffReadyToBeUsed = false;
}

void
CGameLogic::AfterDeathArrestSetUpShortCutTaxi()
{
	if (!MissionDropOffReadyToBeUsed)
		return;
	int nClosestPoint = -1;
	float fDistanceToPoint = 999999.9f;
	for (int i = 0; i < NUM_SHORTCUT_START_POINTS; i++) {
		float dist = (AfterDeathStartPoints[i] - FindPlayerCoors()).Magnitude();
		if (dist < fDistanceToPoint) {
			fDistanceToPoint = dist;
			nClosestPoint = i;
		}
	}
	if (fDistanceToPoint < 100.0f)
		SetUpShortCut(AfterDeathStartPoints[nClosestPoint],
			AfterDeathStartPointOrientation[nClosestPoint],
			ShortCutDropOffForMission,
			ShortCutDropOffOrientationForMission);
	MissionDropOffReadyToBeUsed = false;
}

void
CGameLogic::Save(uint8* buf, uint32* size)
{
INITSAVEBUF
	WriteSaveBuf(buf, NumAfterDeathStartPoints);
	*size += sizeof(NumAfterDeathStartPoints);
	for (int i = 0; i < NUM_SHORTCUT_START_POINTS; i++) {
		WriteSaveBuf(buf, AfterDeathStartPoints[i].x);
		*size += sizeof(AfterDeathStartPoints[i].x);
		WriteSaveBuf(buf, AfterDeathStartPoints[i].y);
		*size += sizeof(AfterDeathStartPoints[i].y);
		WriteSaveBuf(buf, AfterDeathStartPoints[i].z);
		*size += sizeof(AfterDeathStartPoints[i].z);
		WriteSaveBuf(buf, AfterDeathStartPointOrientation[i]);
		*size += sizeof(AfterDeathStartPointOrientation[i]);
	}
VALIDATESAVEBUF(*size)
}

void
CGameLogic::Load(uint8* buf, uint32 size)
{
INITSAVEBUF
	NumAfterDeathStartPoints = ReadSaveBuf<uint32>(buf);
	for (int i = 0; i < NUM_SHORTCUT_START_POINTS; i++) {
		AfterDeathStartPoints[i].x = ReadSaveBuf<float>(buf);
		AfterDeathStartPoints[i].y = ReadSaveBuf<float>(buf);
		AfterDeathStartPoints[i].z = ReadSaveBuf<float>(buf);
		AfterDeathStartPointOrientation[i] = ReadSaveBuf<float>(buf);
	}
VALIDATESAVEBUF(size)
}
