#include "common.h"
#include "patcher.h"
#include "Camera.h"
#include "DMAudio.h"
#include "Clock.h"
#include "Darkel.h"
#include "Hud.h"
#include "Messages.h"
#include "Frontend.h"
#include "Font.h"
#include "Pad.h"
#include "Radar.h"
#include "Replay.h"
#include "Sprite.h"
#include "Sprite2d.h"
#include "Text.h"
#include "Timer.h"
#include "Script.h"
#include "TxdStore.h"
#include "User.h"
#include "World.h"

wchar *CHud::m_HelpMessage = (wchar*)0x86B888;
wchar *CHud::m_LastHelpMessage = (wchar*)0x6E8F28;
int32 &CHud::m_HelpMessageState = *(int32*)0x880E1C;
int32 &CHud::m_HelpMessageTimer = *(int32*)0x880FA4;
int32 &CHud::m_HelpMessageFadeTimer = *(int32*)0x8F6258;
wchar *CHud::m_HelpMessageToPrint = (wchar*)0x664480;
float &CHud::m_HelpMessageDisplayTime = *(float*)0x8E2C28;
float &CHud::m_fTextBoxNumLines = *(float*)0x8E2C28;
float &CHud::m_fHelpMessageTime = *(float *)0x8E2C28;
bool &CHud::m_HelpMessageQuick = *(bool *)0x95CCF7;
int32 CHud::m_ZoneState = *(int32*)0x8F29AC;
int32 CHud::m_ZoneFadeTimer;
int32 CHud::m_ZoneNameTimer = *(int32*)0x8F1A50;
wchar *&CHud::m_pZoneName = *(wchar **)0x8E2C2C;
wchar *CHud::m_pLastZoneName = (wchar*)0x8F432C;
wchar *CHud::m_ZoneToPrint;
int32 CHud::m_VehicleState = *(int32*)0x940560;
int32 CHud::m_VehicleFadeTimer;
int32 CHud::m_VehicleNameTimer = *(int32*)0x8F2A14;
wchar *&CHud::m_VehicleName = *(wchar **)0x942FB4;
wchar *CHud::m_pLastVehicleName = *(wchar **)0x8E2DD8;
wchar *CHud::m_pVehicleNameToPrint;
wchar *CHud::m_Message = (wchar*)0x72E318;
wchar *CHud::m_PagerMessage = (wchar*)0x878840;
bool &CHud::m_Wants_To_Draw_Hud = *(bool*)0x95CD89;
bool &CHud::m_Wants_To_Draw_3dMarkers = *(bool*)0x95CD62;
wchar(&CHud::m_BigMessage)[6][128] = *(wchar(*)[6][128])*(uintptr*)0x664CE0;
int16 &CHud::m_ItemToFlash = *(int16*)0x95CC82;

// These aren't really in CHud
float CHud::BigMessageInUse[6];
float CHud::BigMessageAlpha[6];
float CHud::BigMessageX[6];
float &CHud::OddJob2OffTimer = *(float*)0x942FA0;
int8 &CHud::CounterOnLastFrame = *(int8*)0x95CD67;
float &CHud::OddJob2XOffset = *(float*)0x8F1B5C;
int16 &CHud::CounterFlashTimer = *(int16*)0x95CC20;
int16 &CHud::OddJob2Timer = *(int16*)0x95CC52;
int8 &CHud::TimerOnLastFrame = *(int8*)0x95CDA7;
int16 &CHud::OddJob2On = *(int16*)0x95CC78;
int16 &CHud::TimerFlashTimer = *(int16*)0x95CC6C;
int16 &CHud::PagerSoundPlayed = *(int16*)0x95CC4A;
int32 &CHud::SpriteBrightness = *(int32*)0x95CC54;
float &CHud::PagerXOffset = *(float*)0x941590;
int16 &CHud::PagerTimer = *(int16*)0x95CC3A;
int16 &CHud::PagerOn = *(int16*)0x95CCA0;

CSprite2d *CHud::Sprites = (CSprite2d*)0x95CB9C;

struct
{
	const char *name;
	const char *mask;
} WeaponFilenames[] = {
	{"fist", "fistm"},
	{"bat", "batm"},
	{"pistol", "pistolm" },
	{"uzi", "uzim"},
	{"shotgun", "shotgunm"},
	{"ak47", "ak47m"},
	{"m16", "m16m"},
	{"sniper", "sniperm"},
	{"rocket", "rocketm"},
	{"flame", "flamem"},
	{"molotov", "molotovm"},
	{"grenade", "grenadem"},
	{"detonator", "detonator_mask"},
	{"", ""},
	{"", ""},
	{"radardisc", "radardiscm"},
	{"pager", "pagerm"},
	{"", ""},
	{"", ""},
	{"bleeder", ""},
	{"sitesniper", "sitesniperm"},
	{"siteM16", "siteM16m"},
	{"siterocket", "siterocketm"}
};

RwTexture *&gpSniperSightTex = *(RwTexture**)0x8F5834;
RwTexture *&gpRocketSightTex = *(RwTexture**)0x8E2C20;

#if 0
WRAPPER void CHud::Draw(void) { EAXJMP(0x5052A0); }
#else
void CHud::Draw()
{
	// disable hud via second controller
	if (CPad::GetPad(1)->GetStartJustDown())
		m_Wants_To_Draw_Hud = !m_Wants_To_Draw_Hud;

	if (CReplay::IsPlayingBack())
		return;

	if (m_Wants_To_Draw_Hud && !TheCamera.m_WideScreenOn) {
		bool DrawCrossHair = 0;
		bool DrawCrossHairPC = 0;

		int32 WeaponType = FindPlayerPed()->m_weapons[FindPlayerPed()->m_currentWeapon].m_eWeaponType;
		int32 Mode = TheCamera.Cams[TheCamera.ActiveCam].Mode;

		if (Mode == CCam::MODE_SNIPER || Mode == CCam::MODE_ROCKETLAUNCHER || Mode == CCam::MODE_M16_1STPERSON || Mode == CCam::MODE_HELICANNON_1STPERSON)
			DrawCrossHair = 1;
		if (Mode == CCam::MODE_M16_1STPERSON_RUNABOUT || Mode == CCam::MODE_ROCKETLAUNCHER_RUNABOUT || Mode == CCam::MODE_SNIPER_RUNABOUT)
			DrawCrossHairPC = 1;

		/*
			Draw Crosshairs
		*/
		if (TheCamera.Cams[TheCamera.ActiveCam].Using3rdPersonMouseCam() &&
		    (!CPad::GetPad(0)->GetLookBehindForPed() || TheCamera.m_bPlayerIsInGarage) || Mode == CCam::MODE_1STPERSON_RUNABOUT) {
			if (FindPlayerPed() && !FindPlayerPed()->EnteringCar()) {
				if ((WeaponType >= WEAPONTYPE_COLT45 && WeaponType <= WEAPONTYPE_M16) || WeaponType == WEAPONTYPE_FLAMETHROWER)
					DrawCrossHairPC = 1;
			}
		}

		if (DrawCrossHair || DrawCrossHairPC) {
			RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void *)rwFILTERLINEAR);

			SpriteBrightness = min(SpriteBrightness+1, 30);

			RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)FALSE);

			float fStep = Sin((CTimer::GetTimeInMilliseconds() & 1023)/1024.0f * 6.28f);
			float fMultBright = SpriteBrightness * 0.03f * (0.25f * fStep + 0.75f);
			CRect rect;
			if (DrawCrossHairPC && TheCamera.Cams[TheCamera.ActiveCam].Using3rdPersonMouseCam()) {
#ifndef ASPECT_RATIO_SCALE
				float f3rdX = SCREEN_WIDTH * TheCamera.m_f3rdPersonCHairMultX;
				float f3rdY = SCREEN_HEIGHT * TheCamera.m_f3rdPersonCHairMultY;
#else
				float f3rdX = (((TheCamera.m_f3rdPersonCHairMultX - 0.5f) / ((CDraw::GetAspectRatio()) / (DEFAULT_ASPECT_RATIO))) + 0.5f) * SCREEN_WIDTH;
				float f3rdY = SCREEN_HEIGHT * TheCamera.m_f3rdPersonCHairMultY + SCREEN_SCALE_Y(-2.0f);
#endif
				if (FindPlayerPed() && WeaponType == WEAPONTYPE_M16) {
					rect.left = f3rdX - SCREEN_SCALE_X(32.0f * 0.6f);
					rect.top = f3rdY - SCREEN_SCALE_Y(32.0f  * 0.6f);
					rect.right = f3rdX + SCREEN_SCALE_X(32.0f * 0.6f);
					rect.bottom = f3rdY + SCREEN_SCALE_Y(32.0f  * 0.6f);

					Sprites[HUD_SITEM16].Draw(CRect(rect), CRGBA(255, 255, 255, 255));
				}
				else {
					rect.left = f3rdX - SCREEN_SCALE_X(32.0f * 0.4f);
					rect.top = f3rdY - SCREEN_SCALE_Y(32.0f  * 0.4f);
					rect.right = f3rdX + SCREEN_SCALE_X(32.0f * 0.4f);
					rect.bottom = f3rdY + SCREEN_SCALE_Y(32.0f  * 0.4f);

					Sprites[HUD_SITEM16].Draw(CRect(rect), CRGBA(255, 255, 255, 255));
				}
			}
			else {
				if (Mode == CCam::MODE_M16_1STPERSON ||
				    Mode == CCam::MODE_M16_1STPERSON_RUNABOUT ||
				    Mode == CCam::MODE_HELICANNON_1STPERSON) {
					rect.left = (SCREEN_WIDTH / 2) - SCREEN_SCALE_X(32.0f);
					rect.top = (SCREEN_HEIGHT / 2) - SCREEN_SCALE_Y(32.0f);
					rect.right = (SCREEN_WIDTH / 2) + SCREEN_SCALE_X(32.0f);
					rect.bottom = (SCREEN_HEIGHT / 2) + SCREEN_SCALE_Y(32.0f);
					Sprites[HUD_SITEM16].Draw(CRect(rect), CRGBA(255, 255, 255, 255));
				}
				else if (Mode == CCam::MODE_1STPERSON_RUNABOUT) {
					rect.left = (SCREEN_WIDTH / 2) - SCREEN_SCALE_X(32.0f * 0.7f);
					rect.top = (SCREEN_HEIGHT / 2) - SCREEN_SCALE_Y(32.0f * 0.7f);
					rect.right = (SCREEN_WIDTH / 2) + SCREEN_SCALE_X(32.0f * 0.7f);
					rect.bottom = (SCREEN_HEIGHT / 2) + SCREEN_SCALE_Y(32.0f * 0.7f);

					Sprites[HUD_SITEM16].Draw(CRect(rect), CRGBA(255, 255, 255, 255));
				}
				else if (Mode == CCam::MODE_ROCKETLAUNCHER || Mode == CCam::MODE_ROCKETLAUNCHER_RUNABOUT) {
					RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void *)TRUE);
					RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)rwBLENDONE);
					RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)rwBLENDONE);
					RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)FALSE);
					RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*)FALSE);
					RwRenderStateSet(rwRENDERSTATETEXTURERASTER, RwTextureGetRaster(gpRocketSightTex));

					CSprite::RenderOneXLUSprite(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 1.0f, SCREEN_SCALE_X(40.0f), SCREEN_SCALE_Y(40.0f), (100.0f * fMultBright), (200.0f * fMultBright), (100.0f * fMultBright), 255, 1.0f, 255);
				}
				else {
					// Sniper
					rect.left = (SCREEN_WIDTH / 2) - SCREEN_SCALE_X(210.0f);
					rect.top = (SCREEN_HEIGHT / 2) - SCREEN_SCALE_Y(210.0f);
					rect.right = SCREEN_WIDTH / 2;
					rect.bottom = SCREEN_HEIGHT / 2;
					Sprites[HUD_SITESNIPER].Draw(CRect(rect), CRGBA(255, 255, 255, 255));

					rect.right = (SCREEN_WIDTH / 2);
					rect.top = (SCREEN_HEIGHT / 2) - SCREEN_SCALE_Y(210.0f);
					rect.left = SCREEN_SCALE_X(210.0f) + (SCREEN_WIDTH / 2);
					rect.bottom = SCREEN_HEIGHT / 2;
					Sprites[HUD_SITESNIPER].Draw(CRect(rect), CRGBA(255, 255, 255, 255));

					rect.left = (SCREEN_WIDTH / 2) - SCREEN_SCALE_X(210.0f);
					rect.bottom = (SCREEN_HEIGHT / 2);
					rect.right = (SCREEN_WIDTH / 2);
					rect.top = SCREEN_SCALE_Y(210.0f) + (SCREEN_HEIGHT / 2);
					Sprites[HUD_SITESNIPER].Draw(CRect(rect), CRGBA(255, 255, 255, 255));

					rect.right = (SCREEN_WIDTH / 2);
					rect.bottom = (SCREEN_HEIGHT / 2);
					rect.left = SCREEN_SCALE_X(210.0f) + (SCREEN_WIDTH / 2);
					rect.top = SCREEN_SCALE_Y(210.0f) + (SCREEN_HEIGHT / 2);
					Sprites[HUD_SITESNIPER].Draw(CRect(rect), CRGBA(255, 255, 255, 255));
				}
			}
			RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void *)rwFILTERLINEAR);
			RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)rwBLENDSRCALPHA);
			RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)rwBLENDINVSRCALPHA);
		}
		else {
			SpriteBrightness = 0;
		}

		/*
			DrawMoneyCounter
		*/
		wchar sPrint[16];
		wchar sPrintIcon[16];
		char sTemp[16];

		sprintf(sTemp, "$%08d", CWorld::Players[CWorld::PlayerInFocus].m_nVisibleMoney);
		AsciiToUnicode(sTemp, sPrint);

		CFont::SetPropOff();
		CFont::SetBackgroundOff();
		CFont::SetScale(SCREEN_SCALE_X(0.8f), SCREEN_SCALE_Y(1.35f));
		CFont::SetCentreOff();
		CFont::SetRightJustifyOn();
		CFont::SetRightJustifyWrap(0.0f);
		CFont::SetBackGroundOnlyTextOff();
		CFont::SetFontStyle(FONT_HEADING);
		CFont::SetPropOff();
		CFont::SetColor(CRGBA(0, 0, 0, 255));

		CFont::PrintString(SCREEN_SCALE_FROM_RIGHT(110.0f - 2.0f), SCREEN_SCALE_Y(43.0f + 2.0f), sPrint);

		CFont::SetColor(CRGBA(89, 115, 150, 255));
		CFont::PrintString(SCREEN_SCALE_FROM_RIGHT(110.0f), SCREEN_SCALE_Y(43.0f), sPrint);

		/*
			DrawAmmo
		*/
		uint32 AmmoAmount = CWeaponInfo::GetWeaponInfo(FindPlayerPed()->GetWeapon()->m_eWeaponType)->m_nAmountofAmmunition;
		uint32 AmmoInClip = FindPlayerPed()->m_weapons[FindPlayerPed()->m_currentWeapon].m_nAmmoInClip;
		uint32 TotalAmmo = FindPlayerPed()->m_weapons[FindPlayerPed()->m_currentWeapon].m_nAmmoTotal;
		uint32 Ammo, Clip;

		if (AmmoAmount <= 1 || AmmoAmount >= 1000)
			sprintf(sTemp, "%d", TotalAmmo);
		else {
			if (WeaponType == WEAPONTYPE_FLAMETHROWER) {
				Clip = AmmoInClip / 10;

				if ((TotalAmmo - AmmoInClip) / 10 <= 9999)
					Ammo = (TotalAmmo - AmmoInClip) / 10;
				else
					Ammo = 9999;
			}
			else {
				Clip = AmmoInClip;

				if ((TotalAmmo - AmmoInClip) > 9999)
					Ammo = 9999;
				else
					Ammo = TotalAmmo - AmmoInClip;
			}

			sprintf(sTemp, "%d-%d", Ammo, Clip);
		}

		AsciiToUnicode(sTemp, sPrint);

		/*
			DrawWeaponIcon
		*/
		Sprites[WeaponType].Draw(
			CRect(SCREEN_SCALE_FROM_RIGHT(99.0f), SCREEN_SCALE_Y(27.0f), SCREEN_SCALE_FROM_RIGHT(35.0f), SCREEN_SCALE_Y(91.0f)),
			CRGBA(255, 255, 255, 255),
			0.015f,
			0.015f,
			1.0f,
			0.0f,
			0.015f,
			1.0f,
			1.0f,
			1.0f);

		CFont::SetBackgroundOff();
		CFont::SetScale(SCREEN_SCALE_X(0.4f), SCREEN_SCALE_Y(0.6f));
		CFont::SetJustifyOff();
		CFont::SetCentreOn();
		CFont::SetCentreSize(SCREEN_SCALE_X(640.0f));
		CFont::SetPropOn();
		CFont::SetFontStyle(FONT_BANK);

		if (!CDarkel::FrenzyOnGoing() && WeaponType != WEAPONTYPE_UNARMED && WeaponType != WEAPONTYPE_BASEBALLBAT) {
			CFont::SetColor(CRGBA(0, 0, 0, 255));
			CFont::PrintString(SCREEN_SCALE_FROM_RIGHT(66.0f), SCREEN_SCALE_Y(73.0f), sPrint);
		}

		/*
			DrawHealth
		*/
		CFont::SetBackgroundOff();
		CFont::SetScale(SCREEN_SCALE_X(0.8f), SCREEN_SCALE_Y(1.35f));
		CFont::SetJustifyOff();
		CFont::SetCentreOff();
		CFont::SetRightJustifyWrap(0.0f);
		CFont::SetRightJustifyOn();
		CFont::SetPropOff();
		CFont::SetFontStyle(FONT_HEADING);

		if (m_ItemToFlash == ITEM_HEALTH && CTimer::GetFrameCounter() & 8
			|| m_ItemToFlash != ITEM_HEALTH
			|| FindPlayerPed()->m_fHealth < 10
			&& CTimer::GetFrameCounter() & 8) {
			if (FindPlayerPed()->m_fHealth >= 10
				|| FindPlayerPed()->m_fHealth < 10 && CTimer::GetFrameCounter() & 8) {

				AsciiToUnicode("{", sPrintIcon);
				sprintf(sTemp, "%03d", (int32)FindPlayerPed()->m_fHealth);
				AsciiToUnicode(sTemp, sPrint);

				CFont::SetColor(CRGBA(0, 0, 0, 255));
				CFont::PrintString(SCREEN_SCALE_FROM_RIGHT(110.0f - 2.0f), SCREEN_SCALE_Y(65.0f + 2.0f), sPrint);

				if (!CWorld::Players[CWorld::PlayerInFocus].m_nTimeLastHealthLoss || CTimer::GetTimeInMilliseconds() > CWorld::Players[CWorld::PlayerInFocus].m_nTimeLastHealthLoss + 2000 || CTimer::GetFrameCounter() & 4) {
					CFont::PrintString(SCREEN_SCALE_FROM_RIGHT(164.0f - 2.0f), SCREEN_SCALE_Y(65.0f + 2.0f), sPrintIcon);
				}
				CFont::SetColor(CRGBA(186, 101, 50, 255));

				CFont::PrintString(SCREEN_SCALE_FROM_RIGHT(110.0f), SCREEN_SCALE_Y(65.0f), sPrint);

				if (!CWorld::Players[CWorld::PlayerInFocus].m_nTimeLastHealthLoss || CTimer::GetTimeInMilliseconds() > CWorld::Players[CWorld::PlayerInFocus].m_nTimeLastHealthLoss + 2000 || CTimer::GetFrameCounter() & 4) {
					CFont::PrintString(SCREEN_SCALE_FROM_RIGHT(164.0f), SCREEN_SCALE_Y(65.0f), sPrintIcon);
				}
			}
		}

		/*
			DrawArmour
		*/
		if (m_ItemToFlash == ITEM_ARMOUR && CTimer::GetFrameCounter() & 8 || m_ItemToFlash != ITEM_ARMOUR) {
			CFont::SetScale(SCREEN_SCALE_X(0.8f), SCREEN_SCALE_Y(1.35f));
			if (FindPlayerPed()->m_fArmour > 1.0f) {
				AsciiToUnicode("[", sPrintIcon);
				sprintf(sTemp, "%03d", (int32)FindPlayerPed()->m_fArmour);
				AsciiToUnicode(sTemp, sPrint);

				CFont::SetColor(CRGBA(0, 0, 0, 255));
				CFont::PrintString(SCREEN_SCALE_FROM_RIGHT(182.0f - 2.0f), SCREEN_SCALE_Y(65.0f + 2.0f), sPrint);

				if (!CWorld::Players[CWorld::PlayerInFocus].m_nTimeLastArmourLoss || CTimer::GetTimeInMilliseconds() > CWorld::Players[CWorld::PlayerInFocus].m_nTimeLastArmourLoss + 2000 || CTimer::GetFrameCounter() & 4) {
					CFont::PrintString(SCREEN_SCALE_FROM_RIGHT(234.0f - 2.0f), SCREEN_SCALE_Y(65.0f + 2.0f), sPrintIcon);
				}

				CFont::SetColor(CRGBA(124, 140, 95, 255));

				CFont::PrintString(SCREEN_SCALE_FROM_RIGHT(182.0f), SCREEN_SCALE_Y(65.0f), sPrint);

				if (!CWorld::Players[CWorld::PlayerInFocus].m_nTimeLastArmourLoss || CTimer::GetTimeInMilliseconds() > CWorld::Players[CWorld::PlayerInFocus].m_nTimeLastArmourLoss + 2000 || CTimer::GetFrameCounter() & 1) {
					CFont::PrintString(SCREEN_SCALE_FROM_RIGHT(234.0f), SCREEN_SCALE_Y(65.0f), sPrintIcon);
				}
			}
		}

		/*
			DrawWantedLevel
		*/
		CFont::SetBackgroundOff();
		CFont::SetScale(SCREEN_SCALE_X(0.8f), SCREEN_SCALE_Y(1.35f));
		CFont::SetJustifyOff();
		CFont::SetCentreOff();
		CFont::SetRightJustifyOff();
		CFont::SetPropOn();
		CFont::SetFontStyle(FONT_HEADING);

		AsciiToUnicode("]", sPrintIcon);

		for (int i = 0; i < 6; i++) {
			CFont::SetColor(CRGBA(0, 0, 0, 255));
			CFont::PrintString(2.0f + SCREEN_SCALE_FROM_RIGHT(60.0f - 2.0f + 24.0f * i), SCREEN_SCALE_Y(87.0f + 2.0f), sPrintIcon);
			if (FindPlayerPed()->m_pWanted->m_nWantedLevel > i
				&& (CTimer::GetTimeInMilliseconds() > FindPlayerPed()->m_pWanted->m_nLastWantedLevelChange
					+ 2000 || CTimer::GetFrameCounter() & 4)) {

				CFont::SetColor(CRGBA(193, 164, 120, 255));
				CFont::PrintString(SCREEN_SCALE_FROM_RIGHT(60.0f + 24.0f * i), SCREEN_SCALE_Y(87.0f), sPrintIcon);
			}
		}

		/*
			DrawZoneName
		*/
		if (m_pZoneName) {
			float fZoneAlpha = 0.0f;

			if (m_pZoneName != m_pLastZoneName) {
				switch (m_ZoneState) {
				case 0:
					m_ZoneState = 2;
					m_ZoneToPrint = m_pZoneName;
					m_ZoneNameTimer = 0;
					m_ZoneFadeTimer = 0;
					break;
				case 1:
				case 2:
				case 3:
				case 4:
					m_ZoneNameTimer = 0;
					m_ZoneState = 4;
					break;
				default:
					break;
				}
				m_pLastZoneName = m_pZoneName;
			}

			if (m_ZoneState) {
				switch (m_ZoneState) {
				case 1:
					if (m_ZoneNameTimer > 10000) {
						m_ZoneFadeTimer = 1000;
						m_ZoneState = 3;
					}
					fZoneAlpha = 255.0f;
					break;
				case 2:
					m_ZoneFadeTimer += CTimer::GetTimeStepInMilliseconds();
					if (m_ZoneFadeTimer > 1000) {
						m_ZoneState = 1;
						m_ZoneFadeTimer = 1000;
					}
					fZoneAlpha = m_ZoneFadeTimer * 0.001f * 255.0f;
					break;
				case 3:
					m_ZoneFadeTimer -= CTimer::GetTimeStepInMilliseconds();
					if (m_ZoneFadeTimer < 0) {
						m_ZoneState = 0;
						m_ZoneFadeTimer = 0;
					}
					fZoneAlpha = m_ZoneFadeTimer * 0.001f * 255.0f;
					break;
				case 4:
					m_ZoneFadeTimer -= CTimer::GetTimeStepInMilliseconds();
					if (m_ZoneFadeTimer < 0) {
						m_ZoneFadeTimer = 0;
						m_ZoneToPrint = m_pLastZoneName;
						m_ZoneNameTimer = 0;
						m_ZoneState = 2;
					}
					fZoneAlpha = m_ZoneFadeTimer * 0.001f * 255.0f;
					break;
				default:
					break;

				}
				if (!m_Message[0]) {
					m_ZoneNameTimer += CTimer::GetTimeStepInMilliseconds();
					CFont::SetJustifyOff();
					CFont::SetPropOn();
					CFont::SetBackgroundOff();

					if (FrontEndMenuManager.m_PrefsLanguage == LANGUAGE_SPANISH)
						CFont::SetScale(SCREEN_SCALE_X(1.2f * 0.8f), SCREEN_SCALE_Y(1.2f));
					else
						CFont::SetScale(SCREEN_SCALE_X(1.2f), SCREEN_SCALE_Y(1.2f));

					CFont::SetRightJustifyOn();
					CFont::SetRightJustifyWrap(0.0f);
					CFont::SetBackGroundOnlyTextOff();
					CFont::SetFontStyle(FONT_BANK);
					CFont::SetColor(CRGBA(0, 0, 0, fZoneAlpha));
					CFont::PrintString(SCREEN_SCALE_FROM_RIGHT(32.0f) + SCREEN_SCALE_X(1.0f), SCREEN_SCALE_FROM_BOTTOM(30.0f) + SCREEN_SCALE_Y(1.0f), m_ZoneToPrint);

					CFont::SetColor(CRGBA(152, 154, 82, fZoneAlpha));
					CFont::PrintString(SCREEN_SCALE_FROM_RIGHT(32.0f), SCREEN_SCALE_FROM_BOTTOM(30.0f), m_ZoneToPrint);
				}
			}
		}
		/*else {
			m_pLastZoneName = nil;
			m_ZoneState = 0;
			m_ZoneFadeTimer = 0;
			m_ZoneNameTimer = 0;
		}*/

		/*
			DrawVehicleName
		*/
		if (m_VehicleName) {
			float fVehicleAlpha = 0.0f;

			if (m_VehicleName != m_pLastVehicleName) {
				switch (m_VehicleState) {
				case 0:
					m_VehicleState = 2;
					m_pVehicleNameToPrint = m_VehicleName;
					m_VehicleNameTimer = 0;
					m_VehicleFadeTimer = 0;
					break;
				case 1:
				case 2:
				case 3:
				case 4:
					m_VehicleNameTimer = 0;
					m_VehicleState = 4;
					break;
				default:
					break;
				}
				m_pLastVehicleName = m_VehicleName;
			}

			if (m_VehicleState) {
				switch (m_VehicleState) {
				case 1:
					if (m_VehicleNameTimer > 10000) {
						m_VehicleFadeTimer = 1000;
						m_VehicleState = 3;
					}
					fVehicleAlpha = 255.0f;
					break;
				case 2:
					m_VehicleFadeTimer += CTimer::GetTimeStepInMilliseconds();
					if (m_VehicleFadeTimer > 1000) {
						m_VehicleState = 1;
						m_VehicleFadeTimer = 1000;
					}
					fVehicleAlpha = m_VehicleFadeTimer * 0.001f * 255.0f;
					break;
				case 3:
					m_VehicleFadeTimer -= CTimer::GetTimeStepInMilliseconds();
					if (m_VehicleFadeTimer < 0) {
						m_VehicleState = 0;
						m_VehicleFadeTimer = 0;
					}
					fVehicleAlpha = m_VehicleFadeTimer * 0.001f * 255.0f;
					break;
				case 4:
					m_VehicleFadeTimer -= CTimer::GetTimeStepInMilliseconds();
					if (m_VehicleFadeTimer < 0) {
						m_VehicleFadeTimer = 0;
						m_pVehicleNameToPrint = m_pLastVehicleName;
						m_VehicleNameTimer = 0;
						m_VehicleState = 2;
					}
					fVehicleAlpha = m_VehicleFadeTimer * 0.001f * 255.0f;
					break;
				default:
					break;
				}

				if (!m_Message[0]) {
					m_VehicleNameTimer += CTimer::GetTimeStepInMilliseconds();
					CFont::SetJustifyOff();
					CFont::SetPropOn();
					CFont::SetBackgroundOff();

					if (FrontEndMenuManager.m_PrefsLanguage != LANGUAGE_ITALIAN && FrontEndMenuManager.m_PrefsLanguage != LANGUAGE_SPANISH)
						CFont::SetScale(SCREEN_SCALE_X(1.2f), SCREEN_SCALE_Y(1.2f));
					else
						CFont::SetScale(SCREEN_SCALE_X(1.2f * 0.85f), SCREEN_SCALE_Y(1.2f));

					CFont::SetRightJustifyOn();
					CFont::SetRightJustifyWrap(0.0f);
					CFont::SetBackGroundOnlyTextOff();
					CFont::SetFontStyle(FONT_BANK);
					CFont::SetColor(CRGBA(0, 0, 0, fVehicleAlpha));
					CFont::PrintString(SCREEN_SCALE_FROM_RIGHT(32.0f) + SCREEN_SCALE_X(1.0f), SCREEN_SCALE_FROM_BOTTOM(55.0f) + SCREEN_SCALE_Y(1.0f), m_pVehicleNameToPrint);

					CFont::SetColor(CRGBA(194, 165, 120, fVehicleAlpha));
					CFont::PrintString(SCREEN_SCALE_FROM_RIGHT(32.0f), SCREEN_SCALE_FROM_BOTTOM(55.0f), m_pVehicleNameToPrint);
				}
			}
		}
		else {
			m_pLastVehicleName = nil;
			m_VehicleState = 0;
			m_VehicleFadeTimer = 0;
			m_VehicleNameTimer = 0;
		}

		/*
			DrawClock
		*/
		CFont::SetJustifyOff();
		CFont::SetCentreOff();
		CFont::SetBackgroundOff();
		CFont::SetScale(SCREEN_SCALE_X(0.8f), SCREEN_SCALE_Y(1.35f));
		CFont::SetBackGroundOnlyTextOff();
		CFont::SetPropOff();
		CFont::SetFontStyle(FONT_HEADING);
		CFont::SetRightJustifyOn();
		CFont::SetRightJustifyWrap(0.0f);

		sprintf(sTemp, "%02d:%02d", CClock::GetHours(), CClock::GetMinutes());
		AsciiToUnicode(sTemp, sPrint);

		CFont::SetColor(CRGBA(0, 0, 0, 255));

		CFont::PrintString(SCREEN_SCALE_FROM_RIGHT(111.0f) + SCREEN_SCALE_X(2.0f), SCREEN_SCALE_Y(22.0f) + SCREEN_SCALE_Y(2.0f), sPrint);

		CFont::SetColor(CRGBA(194, 165, 120, 255));
		CFont::PrintString(SCREEN_SCALE_FROM_RIGHT(111.0f), SCREEN_SCALE_Y(22.0f), sPrint);

		/*
			DrawOnScreenTimer
		*/
		
		wchar sTimer[16];

		if (!CUserDisplay::OnscnTimer.m_sEntries[0].m_bTimerProcessed)
			TimerOnLastFrame = 0;
		if (!CUserDisplay::OnscnTimer.m_sEntries[0].m_bCounterProcessed)
			CounterOnLastFrame = 0;

#ifdef FIX_BUGS
#define TIMER_RIGHT_OFFSET 34.0f // Taken from VC frenzy timer
#else
#define TIMER_RIGHT_OFFSET 27.0f
#endif
		if (CUserDisplay::OnscnTimer.m_bProcessed) {
			if (CUserDisplay::OnscnTimer.m_sEntries[0].m_bTimerProcessed) {
				if (!TimerOnLastFrame)
					TimerFlashTimer = 1;

				TimerOnLastFrame = 1;

				if (TimerFlashTimer) {
					if (++TimerFlashTimer > 50)
						TimerFlashTimer = 0;
				}

				if (CTimer::GetFrameCounter() & 4 || !TimerFlashTimer) {
					AsciiToUnicode(CUserDisplay::OnscnTimer.m_sEntries[0].m_bTimerBuffer, sTimer);
					CFont::SetPropOn();
					CFont::SetBackgroundOff();
					CFont::SetScale(SCREEN_SCALE_X(0.8f), SCREEN_SCALE_Y(1.35f));
					CFont::SetRightJustifyOn();
					CFont::SetRightJustifyWrap(0.0f);
					CFont::SetFontStyle(FONT_HEADING);
					CFont::SetPropOff();
					CFont::SetBackGroundOnlyTextOn();
					CFont::SetColor(CRGBA(0, 0, 0, 255));
					CFont::PrintString(SCREEN_SCALE_FROM_RIGHT(TIMER_RIGHT_OFFSET) + SCREEN_SCALE_X(2.0f), SCREEN_SCALE_Y(110.0f) + SCREEN_SCALE_Y(2.0f), sTimer);
					CFont::SetScale(SCREEN_SCALE_X(0.8f), SCREEN_SCALE_Y(1.35f));
					CFont::SetColor(CRGBA(186, 101, 50, 255));
					CFont::PrintString(SCREEN_SCALE_FROM_RIGHT(TIMER_RIGHT_OFFSET), SCREEN_SCALE_Y(110.0f), sTimer);

					if (CUserDisplay::OnscnTimer.m_sEntries[0].m_aTimerText[0]) {
						CFont::SetPropOn();
						CFont::SetColor(CRGBA(0, 0, 0, 255));
						CFont::SetScale(SCREEN_SCALE_X(0.64f), SCREEN_SCALE_Y(1.35f));
						CFont::PrintString(SCREEN_SCALE_FROM_RIGHT(TIMER_RIGHT_OFFSET) - SCREEN_SCALE_X(80.0f) + SCREEN_SCALE_X(2.0f), SCREEN_SCALE_Y(110.0f) + SCREEN_SCALE_Y(2.0f), TheText.Get(CUserDisplay::OnscnTimer.m_sEntries[0].m_aTimerText));

						CFont::SetColor(CRGBA(186, 101, 50, 255));
						CFont::PrintString(SCREEN_SCALE_FROM_RIGHT(TIMER_RIGHT_OFFSET) - SCREEN_SCALE_X(80.0f), SCREEN_SCALE_Y(110.0f), TheText.Get(CUserDisplay::OnscnTimer.m_sEntries[0].m_aTimerText));
					}
				}
			}
			if (CUserDisplay::OnscnTimer.m_sEntries[0].m_bCounterProcessed) {
				if (!CounterOnLastFrame)
					CounterFlashTimer = 1;

				CounterOnLastFrame = 1;

				if (CounterFlashTimer) {
					if (++CounterFlashTimer > 50)
						CounterFlashTimer = 0;
				}

				if (CTimer::GetFrameCounter() & 4 || !CounterFlashTimer) {
					if (CUserDisplay::OnscnTimer.m_sEntries[0].m_nType == COUNTER_DISPLAY_NUMBER) {
						AsciiToUnicode(CUserDisplay::OnscnTimer.m_sEntries[0].m_bCounterBuffer, sTimer);
						CFont::SetPropOn();

						CFont::SetBackgroundOff();
						CFont::SetScale(SCREEN_SCALE_X(0.8f), SCREEN_SCALE_Y(1.35f));
						CFont::SetCentreOff();
						CFont::SetRightJustifyOn();
						CFont::SetRightJustifyWrap(0.0f);
						CFont::SetFontStyle(FONT_HEADING);
						CFont::SetColor(CRGBA(244, 20, 20, 255));
						CFont::SetWrapx(SCREEN_SCALE_X(640.0f));
						CFont::SetPropOff();
						CFont::SetBackGroundOnlyTextOn();

						CFont::SetColor(CRGBA(0, 0, 0, 255));
						CFont::PrintString(SCREEN_SCALE_FROM_RIGHT(TIMER_RIGHT_OFFSET) + SCREEN_SCALE_X(2.0f), SCREEN_SCALE_Y(132.0f) + SCREEN_SCALE_Y(2.0f), sTimer);

						CFont::SetColor(CRGBA(0, 106, 164, 255));
						CFont::PrintString(SCREEN_SCALE_FROM_RIGHT(TIMER_RIGHT_OFFSET), SCREEN_SCALE_Y(132.0f), sTimer);
					} else {
						int counter = atoi(CUserDisplay::OnscnTimer.m_sEntries[0].m_bCounterBuffer);
#ifdef FIX_BUGS
						counter = min(counter, 100);
#endif
						CSprite2d::DrawRect(CRect(SCREEN_SCALE_FROM_RIGHT(TIMER_RIGHT_OFFSET) - SCREEN_SCALE_X(100.0f) / 2 + SCREEN_SCALE_X(4.0f), SCREEN_SCALE_Y(132.0f) + SCREEN_SCALE_Y(8.0f), SCREEN_SCALE_FROM_RIGHT(TIMER_RIGHT_OFFSET) + SCREEN_SCALE_X(4.0f), SCREEN_SCALE_Y(132.0f) + SCREEN_SCALE_Y(11.0f) + SCREEN_SCALE_Y(8.0f)), CRGBA(0, 106, 164, 80));
						CSprite2d::DrawRect(CRect(SCREEN_SCALE_FROM_RIGHT(TIMER_RIGHT_OFFSET) - SCREEN_SCALE_X(100.0f) / 2 + SCREEN_SCALE_X(4.0f), SCREEN_SCALE_Y(132.0f) + SCREEN_SCALE_Y(8.0f), SCREEN_SCALE_X(counter) / 2.0f + SCREEN_SCALE_FROM_RIGHT(TIMER_RIGHT_OFFSET + 50.0f) + SCREEN_SCALE_X(4.0f), SCREEN_SCALE_Y(132.0f) + SCREEN_SCALE_Y(11.0f) + SCREEN_SCALE_Y(8.0f)), CRGBA(0, 106, 164, 255));
					}

					if (CUserDisplay::OnscnTimer.m_sEntries[0].m_aCounterText[0]) {
						CFont::SetPropOn();
						CFont::SetScale(SCREEN_SCALE_X(0.8f), SCREEN_SCALE_Y(1.35f));
						CFont::SetColor(CRGBA(0, 0, 0, 255));
						CFont::PrintString(SCREEN_SCALE_FROM_RIGHT(TIMER_RIGHT_OFFSET) - SCREEN_SCALE_X(61.0f) + SCREEN_SCALE_Y(2.0f), SCREEN_SCALE_Y(132.0f) + SCREEN_SCALE_Y(2.0f), TheText.Get(CUserDisplay::OnscnTimer.m_sEntries[0].m_aCounterText));

						CFont::SetColor(CRGBA(0, 106, 164, 255));
						CFont::PrintString(SCREEN_SCALE_FROM_RIGHT(TIMER_RIGHT_OFFSET) - SCREEN_SCALE_X(61.0f), SCREEN_SCALE_Y(132.0f), TheText.Get(CUserDisplay::OnscnTimer.m_sEntries[0].m_aCounterText));
					}
				}
			}
		}
#undef TIMER_RIGHT_OFFSET

		/*
			DrawPager
		*/
		if (!m_PagerMessage[0]) {
			if (PagerOn == 1) {
				PagerSoundPlayed = false;
				PagerOn = 2;
			}
		}
		if (m_PagerMessage[0] || PagerOn == 2) {
			if (!PagerOn) {
				PagerOn = 1;
				PagerXOffset = 150.0f;
			}
			if (PagerOn == 1) {
				if (PagerXOffset > 0.0f) {
					float fStep = PagerXOffset * 0.05f;
					if (fStep > 10.0f)
						fStep = 10.0f;
					PagerXOffset -= fStep * CTimer::GetTimeStep();
				}
				if (!PagerSoundPlayed) {
					DMAudio.PlayFrontEndSound(SOUND_PAGER, 0);
					PagerSoundPlayed = 1;
				}
			}
			else if (PagerOn == 2) {
				float fStep = PagerXOffset * 0.05f;
				if (fStep < 2.0f)
					fStep = 2.0f;
				PagerXOffset += fStep * CTimer::GetTimeStep();
				if (PagerXOffset > 150.0f) {
					PagerXOffset = 150.0f;
					PagerOn = 0;
				}
			}

			Sprites[HUD_PAGER].Draw(CRect(SCREEN_SCALE_X(26.0f - PagerXOffset), SCREEN_SCALE_Y(27.0f), SCREEN_SCALE_X(160.0 + 26.0f - PagerXOffset), SCREEN_SCALE_Y(80.0f + 27.0f)), CRGBA(255, 255, 255, 255));

			CFont::SetBackgroundOff();
			CFont::SetScale(SCREEN_SCALE_X(0.84f), SCREEN_SCALE_Y(1.0f));
			CFont::SetColor(CRGBA(32, 162, 66, 205));
			CFont::SetRightJustifyOff();
			CFont::SetBackgroundOff();
			CFont::SetCentreOff();
			CFont::SetJustifyOff();
			CFont::SetPropOff();
			CFont::SetFontStyle(FONT_PAGER);
			CFont::PrintString(SCREEN_SCALE_X(52.0f - PagerXOffset), SCREEN_SCALE_Y(54.0f), m_PagerMessage);
		}

		/*
			DrawRadar
		*/
		if (m_ItemToFlash == ITEM_RADAR && CTimer::GetFrameCounter() & 8 || m_ItemToFlash != ITEM_RADAR) {
			CRadar::DrawMap();
			CRect rect(0.0f, 0.0f, SCREEN_SCALE_X(RADAR_WIDTH), SCREEN_SCALE_Y(RADAR_HEIGHT));
			// FIX? scale RADAR_LEFT here somehow
			rect.Translate(RADAR_LEFT, SCREEN_SCALE_FROM_BOTTOM(RADAR_BOTTOM + RADAR_HEIGHT));
			rect.Grow(4.0f);
			Sprites[HUD_RADARDISC].Draw(rect, CRGBA(0, 0, 0, 255));
			CRadar::DrawBlips();
		}
	}

	/*
		Draw3dMarkers
	*/
	if (m_Wants_To_Draw_3dMarkers && !TheCamera.m_WideScreenOn && !m_BigMessage[0][0] && !m_BigMessage[2][0]) {
		CRadar::Draw3dMarkers();
	}

	/*
		DrawScriptText
	*/
	if (!CTimer::GetIsUserPaused()) {
		CTextLine* IntroText = CTheScripts::IntroTextLines;

		for (int i = 0; i < MAX_NUM_INTRO_TEXT_LINES; i++) {
			if (CTheScripts::IntroTextLines[i].m_Text[0] && CTheScripts::IntroTextLines[i].m_bTextBeforeFade) {
				CFont::SetScale(SCREEN_SCALE_X(CTheScripts::IntroTextLines[i].m_fScaleX), SCREEN_SCALE_Y(CTheScripts::IntroTextLines[i].m_fScaleY * 0.5f));
				CFont::SetColor(CTheScripts::IntroTextLines[i].m_sColor);

				if (CTheScripts::IntroTextLines[i].m_bJustify)
					CFont::SetJustifyOn();
				else
					CFont::SetJustifyOff();

				if (CTheScripts::IntroTextLines[i].m_bRightJustify)
					CFont::SetRightJustifyOn();
				else
					CFont::SetRightJustifyOff();

				if (CTheScripts::IntroTextLines[i].m_bCentered)
					CFont::SetCentreOn();
				else
					CFont::SetCentreOff();

				CFont::SetWrapx(SCREEN_SCALE_X(CTheScripts::IntroTextLines[i].m_fWrapX));
				CFont::SetCentreSize(SCREEN_SCALE_X(CTheScripts::IntroTextLines[i].m_fCenterSize));

				if (CTheScripts::IntroTextLines[i].m_bBackground)
					CFont::SetBackgroundOn();
				else
					CFont::SetBackgroundOff();

				CFont::SetBackgroundColor(CTheScripts::IntroTextLines[i].m_sBackgroundColor);

				if (CTheScripts::IntroTextLines[i].m_bBackgroundOnly)
					CFont::SetBackGroundOnlyTextOn();
				else
					CFont::SetBackGroundOnlyTextOff();

				if (CTheScripts::IntroTextLines[i].m_bTextProportional)
					CFont::SetPropOn();
				else
					CFont::SetPropOff();

				CFont::SetFontStyle(CTheScripts::IntroTextLines[i].m_nFont);
				CFont::PrintString(SCREEN_SCALE_X(640.0f - CTheScripts::IntroTextLines[i].m_fAtX), SCREEN_SCALE_Y(448.0f - CTheScripts::IntroTextLines[i].m_fAtY), IntroText->m_Text);
			}
		}

		CScriptRectangle* IntroRect = CTheScripts::IntroRectangles;

		for (int i = 0; i < 16; i++) {
			if (CTheScripts::IntroRectangles[i].m_bIsUsed && CTheScripts::IntroRectangles[i].m_bBeforeFade) {
				if (CTheScripts::IntroRectangles[i].m_nTextureId >= 0) {
					CRect rect = {
						CTheScripts::IntroRectangles[i].m_sRect.left,
						CTheScripts::IntroRectangles[i].m_sRect.bottom,
						CTheScripts::IntroRectangles[i].m_sRect.right,
						CTheScripts::IntroRectangles[i].m_sRect.bottom };

					CTheScripts::ScriptSprites[CTheScripts::IntroRectangles[i].m_nTextureId].Draw(rect, IntroRect->m_sColor);
				}
				else {
					CRect rect = {
						CTheScripts::IntroRectangles[i].m_sRect.left,
						CTheScripts::IntroRectangles[i].m_sRect.bottom,
						CTheScripts::IntroRectangles[i].m_sRect.right,
						CTheScripts::IntroRectangles[i].m_sRect.bottom };

					CSprite2d::DrawRect(rect, IntroRect->m_sColor);
				}
			}
		}

		/*
			DrawSubtitles
		*/
		if (m_Message[0] && !m_BigMessage[2][0] && (FrontEndMenuManager.m_PrefsShowSubtitles == 1 || !TheCamera.m_WideScreenOn)) {
			CFont::SetJustifyOff();
			CFont::SetBackgroundOff();
			CFont::SetBackgroundColor(CRGBA(0, 0, 0, 128));
			CFont::SetScale(SCREEN_SCALE_X(0.48f), SCREEN_SCALE_Y(1.120f));
			CFont::SetCentreOn();
			CFont::SetPropOn();
			CFont::SetFontStyle(FONT_BANK);

			float offsetX = SCREEN_SCALE_X(40.0f) + SCREEN_SCALE_X(8.0f);
			float center = SCREEN_SCALE_FROM_RIGHT(50.0f) - SCREEN_SCALE_X(8.0f) - offsetX;
			CFont::SetCentreSize(center);

			const int16 shadow = 1;
			CFont::SetDropShadowPosition(shadow);
			CFont::SetDropColor(CRGBA(0, 0, 0, 255));
			CFont::SetColor(CRGBA(235, 235, 235, 255));

			// I'm not sure shadow substaction was intentional here, might be a leftover if CFont::PrintString was used for a shadow draw call
			CFont::PrintString(center / 2.0f + offsetX - SCREEN_SCALE_X(shadow), SCREEN_SCALE_Y(4.0f) + SCREEN_SCALE_FROM_BOTTOM(68.0f) - SCREEN_SCALE_Y(shadow), m_Message);
			CFont::SetDropShadowPosition(0);
		}

		/*
			DrawBigMessage
		*/
		// MissionCompleteFailedText
		if (m_BigMessage[0][0]) {
			if (BigMessageInUse[0] != 0.0f) {
				CFont::SetJustifyOff();
				CFont::SetBackgroundOff();
				CFont::SetBackGroundOnlyTextOff();
				CFont::SetScale(SCREEN_SCALE_X(1.8f), SCREEN_SCALE_Y(1.8f));
				CFont::SetPropOn();
				CFont::SetCentreOn();
				CFont::SetCentreSize(SCREEN_SCALE_X(615.0f));
				CFont::SetFontStyle(FONT_HEADING);

				if (BigMessageX[0] >= (SCREEN_WIDTH - 20)) {
					BigMessageInUse[0] += CTimer::GetTimeStep();

					if (BigMessageInUse[0] >= 120.0f) {
						BigMessageInUse[0] = 120.0f;
						BigMessageAlpha[0] -= (CTimer::GetTimeStepInMilliseconds() * 0.3f);
					}

					if (BigMessageAlpha[0] <= 0.0f) {
						m_BigMessage[0][0] = 0;
						BigMessageAlpha[0] = 0.0f;
					}
				}
				else {
					BigMessageX[0] += (CTimer::GetTimeStepInMilliseconds() * 0.3f);
					BigMessageAlpha[0] += (CTimer::GetTimeStepInMilliseconds() * 0.3f);

					if (BigMessageAlpha[0] >= 255.0f)
						BigMessageAlpha[0] = 255.0f;
				}

				CFont::SetColor(CRGBA(0, 0, 0, BigMessageAlpha[0]));
#ifdef FIX_BUGS
				CFont::PrintString(SCREEN_WIDTH / 2 + SCREEN_SCALE_X(2.0f), (SCREEN_HEIGHT / 2) - SCREEN_SCALE_Y(18.0f) + SCREEN_SCALE_Y(2.0f), m_BigMessage[0]);
#else
				CFont::PrintString(SCREEN_WIDTH / 2 + SCREEN_SCALE_X(2.0f), (SCREEN_HEIGHT / 2) - SCREEN_SCALE_Y(20.0f) + SCREEN_SCALE_Y(2.0f), m_BigMessage[0]);
#endif

				CFont::SetColor(CRGBA(85, 119, 133, BigMessageAlpha[0]));
				CFont::PrintString(SCREEN_WIDTH / 2, (SCREEN_HEIGHT / 2) - SCREEN_SCALE_Y(18.0f), m_BigMessage[0]);
			}
			else {
				BigMessageAlpha[0] = 0.0f;
				BigMessageX[0] = -60.0f;
				BigMessageInUse[0] = 1.0f;
			}
		}
		else {
			BigMessageInUse[0] = 0.0f;
		}

		// WastedBustedText
		if (m_BigMessage[2][0]) {
			if (BigMessageInUse[2] != 0.0f) {
				BigMessageAlpha[2] += (CTimer::GetTimeStepInSeconds() * 255.0f);

				if (BigMessageAlpha[2] > 255.0f)
					BigMessageAlpha[2] = 255.0f;

				CFont::SetBackgroundOff();

				if (CGame::frenchGame || CGame::germanGame)
					CFont::SetScale(SCREEN_SCALE_X(1.4f), SCREEN_SCALE_Y(1.4f));
				else
					CFont::SetScale(SCREEN_SCALE_X(2.0f), SCREEN_SCALE_Y(2.0f));

				CFont::SetPropOn();
				CFont::SetRightJustifyOn();
				CFont::SetFontStyle(FONT_HEADING);

				CFont::SetColor(CRGBA(0, 0, 0, BigMessageAlpha[2]));
				CFont::PrintString(SCREEN_SCALE_FROM_RIGHT(20.0f + 4.0f), SCREEN_SCALE_FROM_BOTTOM(78.0f), m_BigMessage[2]);

				CFont::SetColor(CRGBA(170, 123, 87, BigMessageAlpha[2]));
				CFont::PrintString(SCREEN_SCALE_FROM_RIGHT(20.0f), SCREEN_SCALE_FROM_BOTTOM(82.0f), m_BigMessage[2]);
			}
			else {
				BigMessageAlpha[2] = 0.0f;
				BigMessageInUse[2] = 1.0f;
			}
		}
		else {
			BigMessageInUse[2] = 0.0f;
		}
	}
}
#endif


#if 0
WRAPPER void CHud::DrawAfterFade(void) { EAXJMP(0x509030); }
#else
void CHud::DrawAfterFade()
{
	if (CTimer::GetIsUserPaused() || CReplay::IsPlayingBack())
		return;

	if (m_HelpMessage[0]) {
		if (!CMessages::WideStringCompare(m_HelpMessage, m_LastHelpMessage, 256)) {
			switch (m_HelpMessageState) {
			case 0:
				m_HelpMessageFadeTimer = 0;
				m_HelpMessageState = 2;
				m_HelpMessageTimer = 0;
				CMessages::WideStringCopy(m_HelpMessageToPrint, m_HelpMessage, 256);
				m_HelpMessageDisplayTime = CMessages::GetWideStringLength(m_HelpMessage) * 0.05f + 3.0f;

				if (TheCamera.m_ScreenReductionPercentage == 0.0f)
					DMAudio.PlayFrontEndSound(SOUND_A0, 0);
				break;
			case 1:
			case 2:
			case 3:
			case 4:
				m_HelpMessageTimer = 5;
				m_HelpMessageState = 4;
				break;
			default:
				break;
			}
			CMessages::WideStringCopy(m_LastHelpMessage, m_HelpMessage, 256);
		}

		float fAlpha = 255.0f;

		if (m_HelpMessageState) {
			switch (m_HelpMessageState) {
			case 1:
				fAlpha = 255.0f;
				m_HelpMessageFadeTimer = 600;
				if (m_HelpMessageTimer > m_fHelpMessageTime * 1000 || m_HelpMessageQuick && m_HelpMessageTimer > 1500) {
					m_HelpMessageFadeTimer = 600;
					m_HelpMessageState = 3;
				}
				break;
			case 2:
				m_HelpMessageFadeTimer += 2 * CTimer::GetTimeStepInMilliseconds();
				if (m_HelpMessageFadeTimer > 0) {
					m_HelpMessageState = 1;
					m_HelpMessageFadeTimer = 0;
				}
				fAlpha = m_HelpMessageFadeTimer * 0.001f * 255.0f;
				break;
			case 3:
				m_HelpMessageFadeTimer -= 2 * CTimer::GetTimeStepInMilliseconds();
				if (m_HelpMessageFadeTimer >= 0) {
					m_HelpMessageState = 0;
					m_HelpMessageFadeTimer = 0;
				}
				fAlpha = m_HelpMessageFadeTimer * 0.001f * 255.0f;
				break;
			case 4:
				m_HelpMessageFadeTimer -= 2 * CTimer::GetTimeStepInMilliseconds();
				if (m_HelpMessageFadeTimer >= 0) {
					m_HelpMessageState = 2;
					m_HelpMessageFadeTimer = 0;
					CMessages::WideStringCopy(m_HelpMessageToPrint, m_LastHelpMessage, 400);
				}
				fAlpha = m_HelpMessageFadeTimer * 0.001f * 255.0f;
				break;
			default:
				break;
			}

			m_HelpMessageTimer += CTimer::GetTimeStepInMilliseconds();

			CFont::SetAlphaFade(fAlpha);
			CFont::SetCentreOff();
			CFont::SetPropOn();

			if (CGame::germanGame)
				CFont::SetScale(SCREEN_SCALE_X(0.52f * 0.85f), SCREEN_SCALE_Y(1.1f * 0.85f));
			else
				CFont::SetScale(SCREEN_SCALE_X(0.52f), SCREEN_SCALE_Y(1.1f));

			CFont::SetJustifyOff();
			CFont::SetWrapx(SCREEN_SCALE_X(200.0f + 26.0f - 4.0f));
			CFont::SetFontStyle(FONT_BANK);
			CFont::SetBackgroundOn();
			CFont::SetBackGroundOnlyTextOff();
			CFont::SetBackgroundColor(CRGBA(0, 0, 0, fAlpha * 0.8f));
			CFont::SetColor(CRGBA(175, 175, 175, 255));
			CFont::PrintString(SCREEN_SCALE_X(26.0f), SCREEN_SCALE_Y(28.0f + (150.0f - PagerXOffset) * 0.6f), CHud::m_HelpMessageToPrint);
			CFont::SetAlphaFade(255.0f);
			CFont::DrawFonts();
		}
	}
	else
		m_HelpMessageState = 0;

	/*
		DrawBigMessage2
	*/
	// Oddjob
	if (m_BigMessage[3][0]) {
		CFont::SetJustifyOff();
		CFont::SetBackgroundOff();
		CFont::SetScale(SCREEN_SCALE_X(1.2f), SCREEN_SCALE_Y(1.5f));
		CFont::SetCentreOn();
		CFont::SetPropOn();
		CFont::SetCentreSize(SCREEN_SCALE_X(600.0f));
		CFont::SetFontStyle(FONT_BANK);

		CFont::SetColor(CRGBA(0, 0, 0, 255));
		CFont::PrintString((SCREEN_WIDTH / 2) + SCREEN_SCALE_X(2.0f), (SCREEN_HEIGHT / 2) - SCREEN_SCALE_Y(84.0f) + SCREEN_SCALE_Y(2.0f), m_BigMessage[3]);

		CFont::SetColor(CRGBA(89, 115, 150, 255));
		CFont::PrintString((SCREEN_WIDTH / 2), (SCREEN_HEIGHT / 2) - SCREEN_SCALE_Y(84.0f), m_BigMessage[3]);
	}

	if (!m_BigMessage[1][0] && m_BigMessage[4][0]) {
		CFont::SetJustifyOff();
		CFont::SetBackgroundOff();
		CFont::SetScale(SCREEN_SCALE_X(1.2f), SCREEN_SCALE_Y(1.5f));
		CFont::SetCentreOn();
		CFont::SetPropOn();
		CFont::SetCentreSize(SCREEN_SCALE_X(620.0f));
		CFont::SetColor(CRGBA(0, 0, 0, 255));
		CFont::SetFontStyle(FONT_BANK);

		CFont::PrintString((SCREEN_WIDTH / 2) - SCREEN_SCALE_X(2.0f), (SCREEN_HEIGHT / 2) - SCREEN_SCALE_Y(84.0f) - SCREEN_SCALE_Y(2.0f), m_BigMessage[4]);

		CFont::SetColor(CRGBA(89, 115, 150, 255));
		CFont::PrintString((SCREEN_WIDTH / 2), (SCREEN_HEIGHT / 2) - SCREEN_SCALE_Y(84.0f), m_BigMessage[4]);
	}

	// Oddjob result
	if (OddJob2OffTimer > 0)
		OddJob2OffTimer -= CTimer::GetTimeStepInMilliseconds();

	static float fStep;
	if (m_BigMessage[5][0] && OddJob2OffTimer <= 0.0f) {
		if (OddJob2On <= 3) {
			switch (OddJob2On) {
			case 0:
				OddJob2On = 1;
				OddJob2XOffset = 380.0f;
				break;
			case 1:
				if (OddJob2XOffset <= 2.0f) {
					OddJob2Timer = 0;
					OddJob2On = 2;
				}
				else {
					fStep = 40.0f;
					if ((OddJob2XOffset / 6.0f) <= 40.0f)
						fStep = OddJob2XOffset / 6.0f;
					OddJob2XOffset = OddJob2XOffset - fStep;
				}
				break;
			case 2:
				OddJob2Timer += (20.0f * CTimer::GetTimeStep());
				if (OddJob2Timer > 1500) {
					OddJob2On = 3;
				}
				break;
			case 3:
				fStep = 30.0f;
				if ((OddJob2XOffset / 5.0f) >= 30.0f)
					fStep = OddJob2XOffset / 5.0f;

				OddJob2XOffset = OddJob2XOffset - fStep;

				if (OddJob2XOffset < -380.0f) {
					OddJob2OffTimer = 5000.0f;
					OddJob2On = 0;
				}
				break;
			default:
				break;
			}
		}

		if (!m_BigMessage[1][0]) {
			CFont::SetJustifyOff();
			CFont::SetBackgroundOff();
			CFont::SetScale(SCREEN_SCALE_X(1.0f), SCREEN_SCALE_Y(1.2f));
			CFont::SetCentreOn();
			CFont::SetPropOn();
			CFont::SetCentreSize(SCREEN_SCALE_FROM_RIGHT(20.0f));
			CFont::SetColor(CRGBA(0, 0, 0, 255));
			CFont::SetFontStyle(FONT_BANK);

			CFont::PrintString(SCREEN_WIDTH / 2 + SCREEN_SCALE_X(2.0f), SCREEN_HEIGHT / 2 - SCREEN_SCALE_Y(20.0f) + SCREEN_SCALE_Y(2.0f), m_BigMessage[5]);

			CFont::SetColor(CRGBA(156, 91, 40, 255));
			CFont::PrintString(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - SCREEN_SCALE_Y(20.0f), m_BigMessage[5]);
		}
	}

	/*
		DrawMissionTitle
	*/
	if (m_BigMessage[1][0]) {
		if (BigMessageInUse[1] != 0.0f) {
			CFont::SetJustifyOff();
			CFont::SetBackgroundOff();

			if (CGame::frenchGame || FrontEndMenuManager.m_PrefsLanguage == LANGUAGE_SPANISH)
				CFont::SetScale(SCREEN_SCALE_X(0.884f), SCREEN_SCALE_Y(1.36f));
			else
				CFont::SetScale(SCREEN_SCALE_X(1.04f), SCREEN_SCALE_Y(1.6f));

			CFont::SetPropOn();
			CFont::SetRightJustifyWrap(-500.0f);
			CFont::SetRightJustifyOn();
			CFont::SetFontStyle(FONT_HEADING);
			if (BigMessageX[1] >= (SCREEN_WIDTH - 20)) {
				BigMessageInUse[1] += CTimer::GetTimeStep();

				if (BigMessageInUse[1] >= 120.0f) {
					BigMessageInUse[1] = 120.0f;
					BigMessageAlpha[1] -= (CTimer::GetTimeStepInMilliseconds() * 0.3f);
				}
				if (BigMessageAlpha[1] <= 0) {
					m_BigMessage[1][0] = 0;
					BigMessageAlpha[1] = 0.0f;
				}
			}
			else {
				BigMessageX[1] += (CTimer::GetTimeStepInMilliseconds() * 0.3f);
				BigMessageAlpha[1] += (CTimer::GetTimeStepInMilliseconds() * 0.3f);

				if (BigMessageAlpha[1] >= 255.0f)
					BigMessageAlpha[1] = 255.0f;
			}

			CFont::SetColor(CRGBA(40, 40, 40, BigMessageAlpha[1]));
			CFont::PrintString(SCREEN_SCALE_FROM_RIGHT(20.0f) + SCREEN_SCALE_X(2.0f), SCREEN_SCALE_FROM_BOTTOM(120.0f) + SCREEN_SCALE_Y(2.0f), m_BigMessage[1]);

			CFont::SetColor(CRGBA(220, 172, 2, BigMessageAlpha[1]));
			CFont::PrintString(SCREEN_SCALE_FROM_RIGHT(20.0f), SCREEN_SCALE_FROM_BOTTOM(120.0f), m_BigMessage[1]);
		}
		else {
			BigMessageAlpha[1] = 0.0f;
			BigMessageX[1] = -60.0f;
			BigMessageInUse[1] = 1.0f;
		}
	}
	else {
		BigMessageInUse[1] = 0.0f;
	}
}
#endif

#if 0
WRAPPER void CHud::GetRidOfAllHudMessages(void) { EAXJMP(0x504F90); }
#else
void CHud::GetRidOfAllHudMessages()
{
	m_ZoneState = 0;
	m_pLastZoneName = nil;
	m_ZoneNameTimer = 0;
	m_pZoneName = nil;

	for (int i = 0; i < 256; i++) {
		m_HelpMessage[i] = 0;
		m_LastHelpMessage[i] = 0;
		m_HelpMessageToPrint[i] = 0;
	}

	m_HelpMessageTimer = 0;
	m_HelpMessageFadeTimer = 0;
	m_HelpMessageState = 0;
	m_HelpMessageQuick = 0;
	m_HelpMessageDisplayTime = 1.0f;
	m_VehicleName = nil;
	m_pLastVehicleName = nil;
	m_pVehicleNameToPrint = nil;
	m_VehicleNameTimer = 0;
	m_VehicleFadeTimer = 0;
	m_VehicleState = 0;

	for (int i = 0; i < 256; i++)
		m_Message[i] = 0;

	for (int i = 0; i < 6; i++) {
		BigMessageInUse[i] = 0.0f;

		for (int j = 0; j < 128; j++)
			m_BigMessage[i][j] = 0;
	}
}
#endif

void CHud::Initialise()
{
	m_Wants_To_Draw_Hud = true;
	m_Wants_To_Draw_3dMarkers = true;

	int HudTXD = CTxdStore::AddTxdSlot("hud");
	CTxdStore::LoadTxd(HudTXD, "MODELS/HUD.TXD");
	CTxdStore::AddRef(HudTXD);
	CTxdStore::PopCurrentTxd();
	CTxdStore::SetCurrentTxd(HudTXD);

	for (int i = 0; i < ARRAY_SIZE(WeaponFilenames); i++) {
		Sprites[i].SetTexture(WeaponFilenames[i].name, WeaponFilenames[i].mask);
	}

	GetRidOfAllHudMessages();

	if (gpSniperSightTex == nil)
		gpSniperSightTex = RwTextureRead("sitesniper", nil);
	if (gpRocketSightTex == nil)
		gpRocketSightTex = RwTextureRead("siterocket", nil);

	CounterOnLastFrame = 0;
	m_ItemToFlash = ITEM_NONE;
	OddJob2Timer = 0;
	OddJob2OffTimer = 0.0f;
	OddJob2On = 0;
	OddJob2XOffset = 0.0f;
	CounterFlashTimer = 0;
	TimerOnLastFrame = 0;
	TimerFlashTimer = 0;
	SpriteBrightness = 0;
	PagerOn = 0;
	PagerTimer = 0;
	PagerSoundPlayed = 0;
	PagerXOffset = 150.0f;

	CTxdStore::PopCurrentTxd();
}

#if 0
WRAPPER void CHud::ReInitialise(void) { EAXJMP(0x504CC0); }
#else
void CHud::ReInitialise() {
	m_Wants_To_Draw_Hud = true;
	m_Wants_To_Draw_3dMarkers = true;

	GetRidOfAllHudMessages();

	CounterOnLastFrame = 0;
	m_ItemToFlash = ITEM_NONE;
	OddJob2Timer = 0;
	OddJob2OffTimer = 0.0f;
	OddJob2On = 0;
	OddJob2XOffset = 0.0f;
	CounterFlashTimer = 0;
	TimerOnLastFrame = 0;
	TimerFlashTimer = 0;
	SpriteBrightness = 0;
	PagerOn = 0;
	PagerTimer = 0;
	PagerSoundPlayed = 0;
	PagerXOffset = 150.0f;
}
#endif

wchar LastBigMessage[6][128];
#if 0
WRAPPER void CHud::SetBigMessage(wchar *message, int16 style) { EAXJMP(0x50A250); }
#else
void CHud::SetBigMessage(wchar *message, int16 style)
{
	int i = 0;

	if (style == 5) {
		for (i = 0; i < 128; i++) {
			if (message[i] == 0)
				break;

			if (message[i] != LastBigMessage[5][i]) {
				OddJob2On = 0;
				OddJob2OffTimer = 0.0f;
			}

			m_BigMessage[5][i] = message[i];
			LastBigMessage[5][i] = message[i];
		}
	} else {
		for (i = 0; i < 128; i++) {
			if (message[i] == 0)
				break;
			m_BigMessage[style][i] = message[i];
		}
	}
	LastBigMessage[style][i] = 0;
	m_BigMessage[style][i] = 0;
}
#endif

#if 0
WRAPPER void CHud::SetHelpMessage(wchar *message, bool quick) { EAXJMP(0x5051E0); }
#else
void CHud::SetHelpMessage(wchar *message, bool quick)
{
	if (!CReplay::IsPlayingBack()) {
		CMessages::WideStringCopy(m_HelpMessage, message, 256);
		CMessages::InsertPlayerControlKeysInString(m_HelpMessage);

		for (int i = 0; i < 256; i++) {
			m_LastHelpMessage[i] = 0;
		}

		m_HelpMessageState = 0;
		m_HelpMessageQuick = quick;
	}
}
#endif

#if 0
WRAPPER void CHud::SetMessage(wchar *message) { EAXJMP(0x50A210); }
#else
void CHud::SetMessage(wchar *message)
{
	int i = 0;
	for (i = 0; i < 256; i++) {
		if (message[i] == 0)
			break;

		m_Message[i] = message[i];
	}
	m_Message[i] = 0;
}
#endif

#if 0
WRAPPER void CHud::SetPagerMessage(wchar *message) { EAXJMP(0x50A320); }
#else
void CHud::SetPagerMessage(wchar *message)
{
	int i = 0;
	for (i = 0; i < 256; i++) {
		if (message[i] == 0)
			break;

		m_PagerMessage[i] = message[i];
	}
	m_PagerMessage[i] = 0;
}
#endif

#if 0
WRAPPER void CHud::SetVehicleName(wchar *name) { EAXJMP(0x505290); }
#else
void CHud::SetVehicleName(wchar *name)
{
	m_VehicleName = name;
}
#endif

#if 0
WRAPPER void CHud::SetZoneName(wchar *name) { EAXJMP(0x5051D0); }
#else
void CHud::SetZoneName(wchar *name)
{
	m_pZoneName = name;
}
#endif

void CHud::Shutdown()
{
	for (int i = 0; i < ARRAY_SIZE(WeaponFilenames); ++i) {
		Sprites[i].Delete();
	}

	RwTextureDestroy(gpSniperSightTex);
	gpSniperSightTex = nil;

	RwTextureDestroy(gpRocketSightTex);
	gpRocketSightTex = nil;

	int HudTXD = CTxdStore::FindTxdSlot("hud");
	CTxdStore::RemoveTxdSlot(HudTXD);
}

STARTPATCHES
	InjectHook(0x5052A0, &CHud::Draw, PATCH_JUMP);
	InjectHook(0x509030, &CHud::DrawAfterFade, PATCH_JUMP);
	InjectHook(0x504F90, &CHud::GetRidOfAllHudMessages, PATCH_JUMP);
	InjectHook(0x5048F0, &CHud::Initialise, PATCH_JUMP);
	InjectHook(0x504CC0, &CHud::ReInitialise, PATCH_JUMP);
	InjectHook(0x50A250, &CHud::SetBigMessage, PATCH_JUMP);
	InjectHook(0x5051E0, &CHud::SetHelpMessage, PATCH_JUMP);
	InjectHook(0x50A210, &CHud::SetMessage, PATCH_JUMP);
	InjectHook(0x50A320, &CHud::SetPagerMessage, PATCH_JUMP);
	InjectHook(0x505290, &CHud::SetVehicleName, PATCH_JUMP);
	InjectHook(0x5051D0, &CHud::SetZoneName, PATCH_JUMP);
	InjectHook(0x504C50, &CHud::Shutdown, PATCH_JUMP);
ENDPATCHES
