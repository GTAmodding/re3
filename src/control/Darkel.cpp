#include "common.h"

#include "vehicle_enums.h" //needs to be before Vehicle.h

#include "Vehicle.h"

#include "DMAudio.h"
#include "Darkel.h"
#include "Font.h"
#include "GameLogic.h"
#include "PlayerPed.h"
#include "Population.h"
#include "Replay.h"
#include "Stats.h"
#include "Text.h"
#include "Timer.h"
#include "Wanted.h"
#include "Weapon.h"
#include "World.h"
#include "main.h"

//--MIAMI: file done

#define FRENZY_ANY_PED -1
#define FRENZY_ANY_CAR -2

int32 CDarkel::TimeLimit;
int32 CDarkel::PreviousTime;
int32 CDarkel::TimeOfFrenzyStart;
int32 CDarkel::WeaponType;
int32 CDarkel::AmmoInterruptedWeapon;
int32 CDarkel::KillsNeeded;
int32 CDarkel::InterruptedWeaponType;
int32 CDarkel::InterruptedWeaponSelected;

/*
 * TODO: Collect timer/kill counter RGBA colors on top like in Hud/Frontend.
 * bStandardSoundAndMessages is a completely beta thing,
 * makes game handle sounds & messages instead of SCM (just like in GTA2)
 * but it's never been used in the game. Has unused sliding text when frenzy completed etc.
 */
bool CDarkel::bStandardSoundAndMessages;
bool CDarkel::bNeedHeadShot;
bool CDarkel::bProperKillFrenzy;
uint16 CDarkel::Status;
uint16 CDarkel::RegisteredKills[NUM_DEFAULT_MODELS];
int32 CDarkel::ModelToKill;
int32 CDarkel::ModelToKill2;
int32 CDarkel::ModelToKill3;
int32 CDarkel::ModelToKill4;
wchar *CDarkel::pStartMessage;

uint8
CDarkel::CalcFade(uint32 time, uint32 start, uint32 end)
{
	if (time >= start && time <= end) {
		if (time >= start + 500) {
			if (time <= end - 500)
				return 255;
			else
				return 255 * (end - time) / 500;
		} else
			return 255 * (time - start) / 500;
	} else
		return 0;
}

void
CDarkel::DrawMessages()
{
	if (CReplay::IsPlayingBack())
		return;

	switch (Status) {
		case KILLFRENZY_ONGOING:
		{
			CFont::SetJustifyOff();
			CFont::SetBackgroundOff();
			CFont::SetCentreSize(SCREEN_SCALE_X(610.0f));
			CFont::SetCentreOn();
			CFont::SetPropOn();
			uint32 timePassedSinceStart = CTimer::GetTimeInMilliseconds() - TimeOfFrenzyStart;
			if (bStandardSoundAndMessages) {
				if (timePassedSinceStart >= 3000 && timePassedSinceStart < 11000) {
					CFont::SetScale(SCREEN_SCALE_X(1.3f), SCREEN_SCALE_Y(1.3f));
					CFont::SetJustifyOff();
					CFont::SetColor(CRGBA(255, 255, 128, CalcFade(timePassedSinceStart, 3000, 11000)));
					CFont::SetFontStyle(FONT_STANDARD);
					if (pStartMessage) {
						CFont::PrintString(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, pStartMessage);
					}
				}
			} else {
				if (timePassedSinceStart < 8000) {
					CFont::SetScale(SCREEN_SCALE_X(1.3f), SCREEN_SCALE_Y(1.3f));
					CFont::SetJustifyOff();
					CFont::SetColor(CRGBA(255, 255, 128, CalcFade(timePassedSinceStart, 0, 8000)));
					CFont::SetFontStyle(FONT_STANDARD);
					if (pStartMessage) {
						CFont::PrintString(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, pStartMessage);
					}
				}
			}
			CFont::SetScale(SCREEN_SCALE_X(0.75f), SCREEN_SCALE_Y(1.5f));
			CFont::SetCentreOff();
			CFont::SetRightJustifyOn();
			CFont::SetFontStyle(FONT_HEADING);
			if (TimeLimit >= 0) {
				uint32 timeLeft = TimeLimit - (CTimer::GetTimeInMilliseconds() - TimeOfFrenzyStart);
				sprintf(gString, "%d:%02d", timeLeft / 60000, timeLeft % 60000 / 1000);
				AsciiToUnicode(gString, gUString);
				if (timeLeft > 4000 || CTimer::GetFrameCounter() & 1) {
					CFont::SetColor(CRGBA(0, 0, 0, 255));
					CFont::PrintString(SCREEN_SCALE_FROM_RIGHT(35.0f), SCREEN_SCALE_Y(109.0f), gUString);
					CFont::SetColor(CRGBA(0, 207, 133, 255));
					CFont::PrintString(SCREEN_SCALE_FROM_RIGHT(34.0f), SCREEN_SCALE_Y(108.0f), gUString);
				}
			}
			sprintf(gString, "%d", (KillsNeeded >= 0 ? KillsNeeded : 0));
			AsciiToUnicode(gString, gUString);
			CFont::SetColor(CRGBA(0, 0, 0, 255));
			CFont::PrintString(SCREEN_SCALE_FROM_RIGHT(35.0f), SCREEN_SCALE_Y(144.0f), gUString);
			CFont::SetColor(CRGBA(156, 91, 40, 255));
			CFont::PrintString(SCREEN_SCALE_FROM_RIGHT(34.0f), SCREEN_SCALE_Y(143.0f), gUString);
			break;
		}
		case KILLFRENZY_PASSED:
		{
			if (bStandardSoundAndMessages) {
				uint32 timePassedSinceStart = CTimer::GetTimeInMilliseconds() - TimeOfFrenzyStart;
				if (CTimer::GetTimeInMilliseconds() - TimeOfFrenzyStart < 5000) {
					CFont::SetBackgroundOff();
					CFont::SetCentreSize(SCREEN_SCALE_X(620.0f));
					CFont::SetCentreOn();
					CFont::SetScale(SCREEN_SCALE_X(1.5f), SCREEN_SCALE_Y(1.5f));
					CFont::SetJustifyOff();
					CFont::SetColor(CRGBA(128, 255, 128, CalcFade(timePassedSinceStart, 0, 5000)));
					CFont::SetFontStyle(FONT_STANDARD);
					int y = SCREEN_HEIGHT / 2 + SCREEN_SCALE_Y(25.0f - timePassedSinceStart * 0.01f);
					CFont::PrintString(SCREEN_WIDTH / 2, y, TheText.Get("KF_3"));
				}
			}
			break;
		}
		default:
			break;
	}
}

void
CDarkel::Init()
{
	Status = KILLFRENZY_NONE;
}

uint16
CDarkel::QueryModelsKilledByPlayer(int32 modelId)
{
	return RegisteredKills[modelId];
}


bool
CDarkel::FrenzyOnGoing()
{
	return Status == KILLFRENZY_ONGOING;
}


uint16
CDarkel::ReadStatus()
{
	return Status;
}

void
CDarkel::RegisterCarBlownUpByPlayer(CVehicle *vehicle)
{
#ifdef FIX_BUGS
	if (CReplay::IsPlayingBack())
		return;
#endif
	if (FrenzyOnGoing()) {
		int32 model = vehicle->GetModelIndex();
		if (ModelToKill == FRENZY_ANY_CAR || ModelToKill == model || ModelToKill2 == model || ModelToKill3 == model || ModelToKill4 == model) {
			KillsNeeded--;
			DMAudio.PlayFrontEndSound(SOUND_RAMPAGE_CAR_BLOWN, 0);
		}
	}
	RegisteredKills[vehicle->GetModelIndex()]++;
	switch (vehicle->GetVehicleAppearance()) {
	case VEHICLE_APPEARANCE_CAR:
	case VEHICLE_APPEARANCE_BIKE:
		CStats::CarsExploded++;;
		break;
	case VEHICLE_APPEARANCE_HELI:
	case VEHICLE_APPEARANCE_PLANE:
		CStats::HelisDestroyed++;
		break;
	case VEHICLE_APPEARANCE_BOAT:
		CStats::BoatsExploded++;
		break;
	}
	
}

void
CDarkel::RegisterKillByPlayer(CPed *victim, eWeaponType weapon, bool headshot)
{
#ifdef FIX_BUGS
	if (CReplay::IsPlayingBack())
		return;
#endif
	if (FrenzyOnGoing() && (weapon == WeaponType
			|| weapon == WEAPONTYPE_EXPLOSION
			|| weapon == WEAPONTYPE_UZI_DRIVEBY && WeaponType == WEAPONTYPE_UZI
			|| weapon == WEAPONTYPE_RAMMEDBYCAR && WeaponType == WEAPONTYPE_RUNOVERBYCAR
			|| weapon == WEAPONTYPE_RUNOVERBYCAR && WeaponType == WEAPONTYPE_RAMMEDBYCAR
			|| weapon == WEAPONTYPE_FLAMETHROWER && WeaponType == WEAPONTYPE_MOLOTOV)) {
		int32 model = victim->GetModelIndex();
		if (ModelToKill == FRENZY_ANY_PED || ModelToKill == model || ModelToKill2 == model || ModelToKill3 == model || ModelToKill4 == model) {
			if (!bNeedHeadShot || headshot) {
				KillsNeeded--;
				DMAudio.PlayFrontEndSound(SOUND_RAMPAGE_KILL, 0);
			}
		}
	}
	CStats::PeopleKilledByPlayer++;
	RegisteredKills[victim->GetModelIndex()]++;
	CStats::PedsKilledOfThisType[victim->bChrisCriminal ? PEDTYPE_CRIMINAL : victim->m_nPedType]++;
	if (headshot)
		CStats::HeadsPopped++;
	CStats::KillsSinceLastCheckpoint++;
}

void
CDarkel::RegisterKillNotByPlayer(CPed* victim, eWeaponType weapontype)
{
#ifdef FIX_BUGS
	if (CReplay::IsPlayingBack())
		return;
#endif
	CStats::PeopleKilledByOthers++;
}

void
CDarkel::ResetModelsKilledByPlayer()
{
	for (int i = 0; i < NUM_DEFAULT_MODELS; i++)
		RegisteredKills[i] = 0;
}

void
CDarkel::ResetOnPlayerDeath()
{
	if (Status != KILLFRENZY_ONGOING)
		return;

	CPopulation::m_AllRandomPedsThisType = -1;
	Status = KILLFRENZY_FAILED;
	TimeOfFrenzyStart = CTimer::GetTimeInMilliseconds();

	DealWithWeaponChangeAtEndOfFrenzy();
}

void
CDarkel::StartFrenzy(eWeaponType weaponType, int32 time, uint16 kill, int32 modelId0, wchar *text, int32 modelId2, int32 modelId3, int32 modelId4, bool standardSound, bool needHeadShot)
{
	CGameLogic::ClearShortCut();
	CGameLogic::RemoveShortCutDropOffPointForMission();
	eWeaponType fixedWeapon;
	if (weaponType == WEAPONTYPE_UZI_DRIVEBY)
		fixedWeapon = WEAPONTYPE_UZI;
	else
		fixedWeapon = weaponType;

	WeaponType = weaponType;
	Status = KILLFRENZY_ONGOING;
	KillsNeeded = kill;
	ModelToKill = modelId0;
	ModelToKill2 = modelId2;
	ModelToKill3 = modelId3;
	ModelToKill4 = modelId4;
	pStartMessage = text;

	if (text == TheText.Get("PAGE_00")) {
		CDarkel::bProperKillFrenzy = true;
		CDarkel::pStartMessage = nil;
	} else
		bProperKillFrenzy = false;
	
	bStandardSoundAndMessages = standardSound;
	bNeedHeadShot = needHeadShot;
	TimeOfFrenzyStart = CTimer::GetTimeInMilliseconds();
	TimeLimit = time;
	PreviousTime = time / 1000;

	CPlayerPed *player = FindPlayerPed();
	if (fixedWeapon < WEAPONTYPE_TOTALWEAPONS) {
		InterruptedWeaponSelected = player->GetWeapon()->m_eWeaponType;
#if (defined FIX_BUGS || !defined GTA_PS2)
		player->RemoveWeaponAnims(InterruptedWeaponSelected, -1000.0f);
#endif
		InterruptedWeaponType = player->GetWeapon(player->GetWeaponSlot(fixedWeapon)).m_eWeaponType;
		AmmoInterruptedWeapon = player->GetWeapon(player->GetWeaponSlot(fixedWeapon)).m_nAmmoTotal;
		if (InterruptedWeaponType)
			CModelInfo::GetModelInfo(CWeaponInfo::GetWeaponInfo((eWeaponType)InterruptedWeaponType)->m_nModelId)->AddRef();
#if (!defined FIX_BUGS && defined GTA_PS2)
		player->RemoveWeaponAnims(InterruptedWeaponSelected, -1000.0f);
#endif
		player->GiveWeapon(fixedWeapon, 30000);
		player->SetCurrentWeapon(fixedWeapon);
		player->MakeChangesForNewWeapon(player->m_nSelectedWepSlot);

		if (FindPlayerVehicle()) {
			player->SetCurrentWeapon(FindPlayerPed()->m_nSelectedWepSlot);
			player->SetAmmo(fixedWeapon, Min(player->GetWeapon()->m_nAmmoTotal, CWeaponInfo::GetWeaponInfo(player->GetWeapon()->m_eWeaponType)->m_nAmountofAmmunition));
			player->ClearWeaponTarget();
		}
	}
	if (CDarkel::bStandardSoundAndMessages)
		DMAudio.PlayFrontEndSound(SOUND_RAMPAGE_START, 0);
}

void
CDarkel::Update()
{
#ifdef FIX_BUGS
	if (CReplay::IsPlayingBack())
		return;
#endif

	if (Status != KILLFRENZY_ONGOING)
		return;

	int32 FrameTime = TimeLimit - (CTimer::GetTimeInMilliseconds() - TimeOfFrenzyStart);
	if (FrameTime > 0 || TimeLimit < 0) {

		DMAudio.PlayFrontEndSound(SOUND_RAMPAGE_ONGOING, FrameTime);

		int32 PrevTime = FrameTime / 1000;

		if (PrevTime != PreviousTime) {
			if (PreviousTime < 12)
				DMAudio.PlayFrontEndSound(SOUND_CLOCK_TICK, PrevTime);
			PreviousTime = PrevTime;
		}

	} else {
		CPopulation::m_AllRandomPedsThisType = -1;
		Status = KILLFRENZY_FAILED;
		TimeOfFrenzyStart = CTimer::GetTimeInMilliseconds();
		DealWithWeaponChangeAtEndOfFrenzy();

		if (bStandardSoundAndMessages)
			DMAudio.PlayFrontEndSound(SOUND_RAMPAGE_FAILED, 0);
	}

	if (KillsNeeded <= 0) {
		CPopulation::m_AllRandomPedsThisType = -1;
		Status = KILLFRENZY_PASSED;

		if (bProperKillFrenzy)
			CStats::AnotherKillFrenzyPassed();

		TimeOfFrenzyStart = CTimer::GetTimeInMilliseconds();

		FindPlayerPed()->m_pWanted->SetWantedLevel(0);

		DealWithWeaponChangeAtEndOfFrenzy();

		if (bStandardSoundAndMessages)
			DMAudio.PlayFrontEndSound(SOUND_RAMPAGE_PASSED, 0);
	}
}

void
CDarkel::DealWithWeaponChangeAtEndOfFrenzy()
{
	eWeaponType fixedWeapon;
	if (WeaponType == WEAPONTYPE_UZI_DRIVEBY)
		fixedWeapon = WEAPONTYPE_UZI;
	else
		fixedWeapon = (eWeaponType)WeaponType;

	if (fixedWeapon < WEAPONTYPE_TOTALWEAPONS && InterruptedWeaponType)
		CModelInfo::GetModelInfo(CWeaponInfo::GetWeaponInfo((eWeaponType)InterruptedWeaponType)->m_nModelId)->RemoveRef();

	if (fixedWeapon < WEAPONTYPE_TOTALWEAPONS) {
		int slot = CWeaponInfo::GetWeaponInfo(fixedWeapon)->m_nWeaponSlot;
		FindPlayerPed()->RemoveWeaponModel(FindPlayerPed()->GetWeapon(slot).GetInfo()->m_nModelId);
		FindPlayerPed()->GetWeapon(slot).m_eWeaponType = WEAPONTYPE_UNARMED;
		FindPlayerPed()->GetWeapon(slot).m_nAmmoTotal = 0;
		FindPlayerPed()->GetWeapon(slot).m_nAmmoInClip = 0;
		FindPlayerPed()->GetWeapon(slot).m_eWeaponState = WEAPONSTATE_READY;
		FindPlayerPed()->RemoveWeaponAnims(fixedWeapon, -1000.0f);
		CModelInfo::GetModelInfo(CWeaponInfo::GetWeaponInfo(fixedWeapon)->m_nModelId)->RemoveRef();
	}

	CPlayerPed* player = FindPlayerPed();
	if (fixedWeapon < WEAPONTYPE_TOTALWEAPONS) {
		player->m_nSelectedWepSlot = CWeaponInfo::GetWeaponInfo((eWeaponType)InterruptedWeaponSelected)->m_nWeaponSlot;
		player->GiveWeapon((eWeaponType)InterruptedWeaponType, AmmoInterruptedWeapon, true);
	}

	if (FindPlayerVehicle()) {
		player->RemoveWeaponModel(CWeaponInfo::GetWeaponInfo(player->GetWeapon()->m_eWeaponType)->m_nModelId);
		if (FindPlayerPed()->GetWeapon(WEAPONSLOT_SUBMACHINEGUN).m_eWeaponType)
			FindPlayerPed()->m_nSelectedWepSlot = WEAPONSLOT_SUBMACHINEGUN;
		else
			FindPlayerPed()->m_nSelectedWepSlot = WEAPONSLOT_UNARMED;
		player->SetCurrentWeapon(FindPlayerPed()->m_nSelectedWepSlot);
		player->MakeChangesForNewWeapon(player->m_currentWeapon);
		player->RemoveDrivebyAnims();
	}
}
