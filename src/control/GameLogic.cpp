#include "common.h"
#include "patcher.h"
#include "GameLogic.h"
#include "Clock.h"
#include "Stats.h"
#include "Pickups.h"
#include "Timer.h"
#include "Streaming.h"
#include "CutsceneMgr.h"
#include "World.h"
#include "PlayerPed.h"
#include "Camera.h"
#include "Messages.h"
#include "CarCtrl.h"
#include "Restart.h"
#include "Pad.h"
#include "References.h"
#include "Fire.h"
#include "Script.h"
#include "Garages.h"

uint8 CGameLogic::ActivePlayers; // 0x95CD5E

void
CGameLogic::InitAtStartOfGame()
{
	ActivePlayers = 1;
}

void
CGameLogic::PassTime(uint32 time)
{
	uint8 minutes, hours, days;

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
	CGame::DrasticTidyUpMemory();
	CStreaming::LoadScene(pos);
	CTimer::Update();
}

void
CGameLogic::Update()
{
	CVector vecRestartPos;
	float fRestartFloat;

	if (CCutsceneMgr::ms_cutsceneProcessing) return;

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
		if ((CTimer::GetTimeInMilliseconds() - pPlayerInfo.m_nWBTime > 0x800) && (CTimer::GetPreviousTimeInMilliseconds() - pPlayerInfo.m_nWBTime <= 0x800)) {
			TheCamera.SetFadeColour(200, 200, 200);
			TheCamera.Fade(2.0f, FADE_OUT);
		}

		if (CTimer::GetTimeInMilliseconds() - pPlayerInfo.m_nWBTime >= 0x1000) {
			pPlayerInfo.m_WBState = WBSTATE_PLAYING;
			if (pPlayerInfo.m_bGetOutOfHospitalFree) {
				pPlayerInfo.m_bGetOutOfHospitalFree = false;
			} else {
				pPlayerInfo.m_nMoney = max(0, pPlayerInfo.m_nMoney - 1000);
				pPlayerInfo.m_pPed->ClearWeapons();
			}

			if (pPlayerInfo.m_pPed->bInVehicle) {
				CVehicle *pVehicle = pPlayerInfo.m_pPed->m_pMyVehicle;
				if (pVehicle != nil) {
					if (pVehicle->pDriver == pPlayerInfo.m_pPed) {
						pVehicle->pDriver = nil;
						if (pVehicle->m_status != STATUS_WRECKED)
							pVehicle->m_status = STATUS_ABANDONED;
					} else
						pVehicle->RemovePassenger(pPlayerInfo.m_pPed);
				}
			}
			CEventList::Initialise();
			CMessages::ClearMessages();
			CCarCtrl::ClearInterestingVehicleList();
			CWorld::ClearExcitingStuffFromArea(pPlayerInfo.GetPos(), 4000.0f, 1);
			CRestart::FindClosestHospitalRestartPoint(pPlayerInfo.GetPos(), &vecRestartPos, &fRestartFloat);
			CRestart::OverrideHospitalLevel = false;
			CRestart::OverridePoliceStationLevel = false;
			PassTime(720);
			RestorePlayerStuffDuringResurrection(pPlayerInfo.m_pPed, vecRestartPos, fRestartFloat);
			SortOutStreamingAndMemory(pPlayerInfo.GetPos());
			TheCamera.m_fCamShakeForce = 0.0f;
			TheCamera.SetMotionBlur(0, 0, 0, 0, MBLUR_NONE);
			CPad::GetPad(0)->StopShaking(0);
			CReferences::RemoveReferencesToPlayer();
			CCarCtrl::CountDownToCarsAtStart = 2;
			CPad::GetPad(CWorld::PlayerInFocus)->DisablePlayerControls = PLAYERCONTROL_ENABLED;
			if (CRestart::bFadeInAfterNextDeath) { 
				TheCamera.SetFadeColour(200, 200, 200);
				TheCamera.Fade(4.0f, FADE_IN);
			} else CRestart::bFadeInAfterNextDeath = true;
		}
		break;
	case WBSTATE_BUSTED:
		if ((CTimer::GetTimeInMilliseconds() - pPlayerInfo.m_nWBTime > 0x800) && (CTimer::GetPreviousTimeInMilliseconds() - pPlayerInfo.m_nWBTime <= 0x800)) {
			TheCamera.SetFadeColour(0, 0, 0);
			TheCamera.Fade(2.0f, FADE_OUT);
		}
		if (CTimer::GetTimeInMilliseconds() - pPlayerInfo.m_nWBTime >= 0x1000) {
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
				pPlayerInfo.m_nMoney = max(0, pPlayerInfo.m_nMoney - takeMoney);
				pPlayerInfo.m_pPed->ClearWeapons();
			}

			if (pPlayerInfo.m_pPed->bInVehicle) {
				CVehicle *pVehicle = pPlayerInfo.m_pPed->m_pMyVehicle;
				if (pVehicle != nil) {
					if (pVehicle->pDriver == pPlayerInfo.m_pPed) {
						pVehicle->pDriver = nil;
						if (pVehicle->m_status != STATUS_WRECKED)
							pVehicle->m_status = STATUS_ABANDONED;
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
			CRestart::OverrideHospitalLevel = false;
			CRestart::OverridePoliceStationLevel = false;
			PassTime(720);
			RestorePlayerStuffDuringResurrection(pPlayerInfo.m_pPed, vecRestartPos, fRestartFloat);
			pPlayerInfo.m_pPed->ClearWeapons();
			SortOutStreamingAndMemory(pPlayerInfo.GetPos());
			TheCamera.m_fCamShakeForce = 0.0f;
			TheCamera.SetMotionBlur(0, 0, 0, 0, MBLUR_NONE);
			CPad::GetPad(0)->StopShaking(0);
			CReferences::RemoveReferencesToPlayer();
			CCarCtrl::CountDownToCarsAtStart = 2;
			CPad::GetPad(CWorld::PlayerInFocus)->DisablePlayerControls = PLAYERCONTROL_ENABLED;
			if (CRestart::bFadeInAfterNextArrest) {
				TheCamera.SetFadeColour(0, 0, 0);
				TheCamera.Fade(4.0f, FADE_IN);
			} else CRestart::bFadeInAfterNextArrest = true;
		}
		break;
	case WBSTATE_FAILED_CRITICAL_MISSION:
		if (CTimer::GetTimeInMilliseconds() - pPlayerInfo.m_nWBTime > 0x800 && CTimer::GetPreviousTimeInMilliseconds() - pPlayerInfo.m_nWBTime <= 0x800) {
			TheCamera.SetFadeColour(0, 0, 0);
			TheCamera.Fade(2.0f, FADE_OUT);
		}
		if (CTimer::GetTimeInMilliseconds() - pPlayerInfo.m_nWBTime >= 0x1000) {
			pPlayerInfo.m_WBState = WBSTATE_PLAYING;
			if (pPlayerInfo.m_pPed->bInVehicle) {
				CVehicle *pVehicle = pPlayerInfo.m_pPed->m_pMyVehicle;
				if (pVehicle != nil) {
					if (pVehicle->pDriver == pPlayerInfo.m_pPed) {
						pVehicle->pDriver = nil;
						if (pVehicle->m_status != STATUS_WRECKED)
							pVehicle->m_status = STATUS_ABANDONED;
					} else
						pVehicle->RemovePassenger(pPlayerInfo.m_pPed);
				}
			}
			CEventList::Initialise();
			CMessages::ClearMessages();
			CCarCtrl::ClearInterestingVehicleList();
			CWorld::ClearExcitingStuffFromArea(pPlayerInfo.GetPos(), 4000.0f, 1);
			CRestart::FindClosestPoliceRestartPoint(pPlayerInfo.GetPos(), &vecRestartPos, &fRestartFloat);
			CRestart::OverridePoliceStationLevel = false;
			CRestart::OverrideHospitalLevel = false;
			RestorePlayerStuffDuringResurrection(pPlayerInfo.m_pPed, vecRestartPos, fRestartFloat);
			SortOutStreamingAndMemory(pPlayerInfo.GetPos());
			TheCamera.m_fCamShakeForce = 0.0f;
			TheCamera.SetMotionBlur(0, 0, 0, 0, MBLUR_NONE);
			CPad::GetPad(0)->StopShaking(0);
			CReferences::RemoveReferencesToPlayer();
			CCarCtrl::CountDownToCarsAtStart = 2;
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
	pPlayerPed->m_fHealth = 100.0f;
	pPlayerPed->m_fArmour = 0.0f;
	pPlayerPed->bIsVisible = true;
	pPlayerPed->m_bloodyFootprintCount = 0;
	pPlayerPed->bDoBloodyFootprints = false;
	pPlayerPed->ClearAdrenaline();
	pPlayerPed->m_fCurrentStamina = pPlayerPed->m_fMaxStamina;
	if (pPlayerPed->m_pFire)
		pPlayerPed->m_pFire->Extinguish();
	pPlayerPed->bInVehicle = false;
	pPlayerPed->m_pMyVehicle = nil;
	pPlayerPed->m_pVehicleAnim = nil;
	pPlayerPed->m_pWanted->Reset();
	pPlayerPed->RestartNonPartialAnims();
	pPlayerPed->GetPlayerInfoForThisPlayerPed()->MakePlayerSafe(false);
	pPlayerPed->bRemoveFromWorld = false;
	pPlayerPed->ClearWeaponTarget();
	pPlayerPed->SetInitialState();
	CCarCtrl::ClearInterestingVehicleList();

	pos.z += 1.0f;
	pPlayerPed->Teleport(pos);
	pPlayerPed->SetMoveSpeed(CVector(0.0f, 0.0f, 0.0f));

	pPlayerPed->m_fRotationCur = DEGTORAD(angle);
	pPlayerPed->m_fRotationDest = pPlayerPed->m_fRotationCur;
	pPlayerPed->SetHeading(pPlayerPed->m_fRotationCur);
	CTheScripts::ClearSpaceForMissionEntity(pos, pPlayerPed);
	CWorld::ClearExcitingStuffFromArea(pos, 4000.0, 1);
	pPlayerPed->RestoreHeadingRate();
	TheCamera.SetCameraDirectlyInFrontForFollowPed_CamOnAString();
	CReferences::RemoveReferencesToPlayer();
	CGarages::PlayerArrestedOrDied();
	CStats::CheckPointReachedUnsuccessfully();
	CWorld::Remove(pPlayerPed);
	CWorld::Add(pPlayerPed);
}

STARTPATCHES
	InjectHook(0x4213F0, &CGameLogic::InitAtStartOfGame, PATCH_JUMP);
	InjectHook(0x421C00, &CGameLogic::PassTime, PATCH_JUMP);
	InjectHook(0x421A20, &CGameLogic::SortOutStreamingAndMemory, PATCH_JUMP);
	InjectHook(0x421400, &CGameLogic::Update, PATCH_JUMP);
	InjectHook(0x421A60, &CGameLogic::RestorePlayerStuffDuringResurrection, PATCH_JUMP);
ENDPATCHES