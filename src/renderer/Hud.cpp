#include "common.h"

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
#include "Wanted.h"
#include "Sprite.h"
#include "Sprite2d.h"
#include "Text.h"
#include "Timer.h"
#include "Script.h"
#include "TxdStore.h"
#include "User.h"
#include "World.h"

#ifdef PS2_HUD
#define MONEY_X 100.0f
#define WEAPON_X 91.0f
#define AMMO_X 59.0f
#define HEALTH_X 100.0f
#define STARS_X 49.0f
#define ZONE_Y 61.0f
#define VEHICLE_Y 81.0f
#define CLOCK_X 101.0f
#define SUBS_Y 83.0f
#define WASTEDBUSTED_Y 122.0f
#define BIGMESSAGE_Y 80.0f
#else
#define MONEY_X 110.0f
#define WEAPON_X 99.0f
#define AMMO_X 66.0f
#define HEALTH_X 110.0f
#define STARS_X 60.0f
#define ZONE_Y 30.0f
#define VEHICLE_Y 55.0f
#define CLOCK_X 111.0f
#define SUBS_Y 68.0f
#define WASTEDBUSTED_Y 82.0f
#define BIGMESSAGE_Y 84.0f
#endif

#ifdef FIX_BUGS
#define TIMER_RIGHT_OFFSET 34.0f // Taken from VC frenzy timer
#define BIGMESSAGE_Y_OFFSET 18.0f
#else
#define TIMER_RIGHT_OFFSET 27.0f
#define BIGMESSAGE_Y_OFFSET 20.0f
#endif

#if defined(PS2_HUD) && !defined(FIX_BUGS)
	#define SCREEN_SCALE_X_PC(a) (a)
	#define SCREEN_SCALE_Y_PC(a) (a)
	#define SCALE_AND_CENTER_X_PC(a) (a)
#else
	#define SCREEN_SCALE_X_PC(a) SCREEN_SCALE_X(a)
	#define SCREEN_SCALE_Y_PC(a) SCREEN_SCALE_Y(a)
	#define SCALE_AND_CENTER_X_PC(a) SCALE_AND_CENTER_X(a)
#endif

#if defined(FIX_BUGS)
	#define SCREEN_SCALE_X_FIX(a) SCREEN_SCALE_X(a)
	#define SCREEN_SCALE_Y_FIX(a) SCREEN_SCALE_Y(a)
	#define SCALE_AND_CENTER_X_FIX(a) SCALE_AND_CENTER_X(a)
#else
	#define SCREEN_SCALE_X_FIX(a) (a)
	#define SCREEN_SCALE_Y_FIX(a) (a)
	#define SCALE_AND_CENTER_X_FIX(a) (a)
#endif

#ifdef FIX_BUGS
#define FRAMECOUNTER CTimer::GetLogicalFrameCounter()
#else
#define FRAMECOUNTER CTimer::GetFrameCounter()
#endif

// Game has colors inlined in code.
// For easier modification we collect them here:
CRGBA MONEY_COLOR(89, 115, 150, 255);
CRGBA AMMO_COLOR(0, 0, 0, 255);
CRGBA HEALTH_COLOR(186, 101, 50, 255);
CRGBA ARMOUR_COLOR(124, 140, 95, 255);
CRGBA WANTED_COLOR(193, 164, 120, 255);
CRGBA ZONE_COLOR(152, 154, 82, 255);
CRGBA VEHICLE_COLOR(194, 165, 120, 255);
CRGBA CLOCK_COLOR(194, 165, 120, 255);
CRGBA TIMER_COLOR(186, 101, 50, 255);
CRGBA COUNTER_COLOR(0, 106, 164, 255);
CRGBA PAGER_COLOR(32, 162, 66, 205);
CRGBA RADARDISC_COLOR(0, 0, 0, 255);
CRGBA BIGMESSAGE_COLOR(85, 119, 133, 255);
CRGBA WASTEDBUSTED_COLOR(170, 123, 87, 255);
CRGBA ODDJOB_COLOR(89, 115, 150, 255);
CRGBA ODDJOB2_COLOR(156, 91, 40, 255);
CRGBA MISSIONTITLE_COLOR(220, 172, 2, 255);


int16 CHud::m_ItemToFlash;
CSprite2d CHud::Sprites[NUM_HUD_SPRITES];
wchar *CHud::m_pZoneName;
wchar *CHud::m_pLastZoneName;
wchar *CHud::m_ZoneToPrint;
wchar CHud::m_Message[256];
wchar CHud::m_BigMessage[6][128];
wchar LastBigMessage[6][128];
wchar CHud::m_PagerMessage[256];
uint32 CHud::m_ZoneNameTimer;
int32 CHud::m_ZoneFadeTimer;
uint32 CHud::m_ZoneState;
wchar CHud::m_HelpMessage[HELP_MSG_LENGTH];
wchar CHud::m_LastHelpMessage[HELP_MSG_LENGTH];
wchar CHud::m_HelpMessageToPrint[HELP_MSG_LENGTH];
uint32 CHud::m_HelpMessageTimer;
int32 CHud::m_HelpMessageFadeTimer;
uint32 CHud::m_HelpMessageState;
bool CHud::m_HelpMessageQuick;
float CHud::m_HelpMessageDisplayTime;
int32 CHud::SpriteBrightness;
bool CHud::m_Wants_To_Draw_Hud;
bool CHud::m_Wants_To_Draw_3dMarkers;
wchar *CHud::m_pVehicleName;
wchar *CHud::m_pLastVehicleName;
uint32 CHud::m_VehicleNameTimer;
int32 CHud::m_VehicleFadeTimer;
uint32 CHud::m_VehicleState;
wchar *CHud::m_pVehicleNameToPrint;

// These aren't really in CHud
float BigMessageInUse[6];
float BigMessageX[6];
float BigMessageAlpha[6];
int16 PagerOn;
int16 PagerTimer;
float PagerXOffset;
int16 PagerSoundPlayed;
int16 OddJob2On;
uint16 OddJob2Timer;
float OddJob2XOffset;
float OddJob2OffTimer;
bool CounterOnLastFrame;
uint16 CounterFlashTimer;
bool TimerOnLastFrame;
uint16 TimerFlashTimer;

RwTexture *gpSniperSightTex;
RwTexture *gpRocketSightTex;

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
	{"radardisc", "radardisc"},
	{"pager", "pagerm"},
	{"", ""},
	{"", ""},
	{"bleeder", ""},
	{"sitesniper", "sitesniperm"},
	{"siteM16", "siteM16m"},
	{"siterocket", "siterocket"}
};

void CHud::Initialise()
{
	m_Wants_To_Draw_Hud = true;
	m_Wants_To_Draw_3dMarkers = true;

	int HudTXD = CTxdStore::AddTxdSlot("hud");
	CTxdStore::LoadTxd(HudTXD, "MODELS/HUD.TXD");
	CTxdStore::AddRef(HudTXD);
	CTxdStore::PopCurrentTxd();
	CTxdStore::SetCurrentTxd(HudTXD);

	for (int i = 0; i < NUM_HUD_SPRITES; i++) {
		Sprites[i].SetTexture(WeaponFilenames[i].name, WeaponFilenames[i].mask);
	}

	GetRidOfAllHudMessages();

	if (gpSniperSightTex == nil)
		gpSniperSightTex = RwTextureRead("sitesniper", nil);
	if (gpRocketSightTex == nil)
		gpRocketSightTex = RwTextureRead("siterocket", nil);

	CounterOnLastFrame = false;
	m_ItemToFlash = ITEM_NONE;
	OddJob2Timer = 0;
	OddJob2OffTimer = 0.0f;
	OddJob2On = 0;
	OddJob2XOffset = 0.0f;
	CounterFlashTimer = 0;
	TimerOnLastFrame = false;
	TimerFlashTimer = 0;
	SpriteBrightness = 0;
	PagerOn = 0;
	PagerTimer = 0;
	PagerSoundPlayed = 0;
	PagerXOffset = 150.0f;

	CTxdStore::PopCurrentTxd();
}

void CHud::Shutdown()
{
	for (int i = 0; i < NUM_HUD_SPRITES; ++i) {
		Sprites[i].Delete();
	}

	RwTextureDestroy(gpSniperSightTex);
	gpSniperSightTex = nil;

	RwTextureDestroy(gpRocketSightTex);
	gpRocketSightTex = nil;

	int HudTXD = CTxdStore::FindTxdSlot("hud");
	CTxdStore::RemoveTxdSlot(HudTXD);
}

void CHud::ReInitialise() {
	m_Wants_To_Draw_Hud = true;
	m_Wants_To_Draw_3dMarkers = true;

	GetRidOfAllHudMessages();

	CounterOnLastFrame = false;
	m_ItemToFlash = ITEM_NONE;
	OddJob2Timer = 0;
	OddJob2OffTimer = 0.0f;
	OddJob2On = 0;
	OddJob2XOffset = 0.0f;
	CounterFlashTimer = 0;
	TimerOnLastFrame = false;
	TimerFlashTimer = 0;
	SpriteBrightness = 0;
	PagerOn = 0;
	PagerTimer = 0;
	PagerSoundPlayed = 0;
	PagerXOffset = 150.0f;
}

void CHud::GetRidOfAllHudMessages()
{
	m_ZoneState = 0;
	m_pLastZoneName = nil;
	m_ZoneNameTimer = 0;
	m_pZoneName = nil;

	for (int i = 0; i < HELP_MSG_LENGTH; i++) {
		m_HelpMessage[i] = 0;
		m_LastHelpMessage[i] = 0;
		m_HelpMessageToPrint[i] = 0;
	}

	m_HelpMessageTimer = 0;
	m_HelpMessageFadeTimer = 0;
	m_HelpMessageState = 0;
	m_HelpMessageQuick = 0;
	m_HelpMessageDisplayTime = 1.0f;
	m_pVehicleName = nil;
	m_pLastVehicleName = nil;
	m_pVehicleNameToPrint = nil;
	m_VehicleNameTimer = 0;
	m_VehicleFadeTimer = 0;
	m_VehicleState = 0;

	for (int i = 0; i < ARRAY_SIZE(m_Message); i++)
		m_Message[i] = 0;

	for (int i = 0; i < 6; i++) {
		BigMessageInUse[i] = 0.0f;

		for (int j = 0; j < 128; j++)
			m_BigMessage[i][j] = 0;
	}
}

void CHud::SetZoneName(wchar *name)
{
	m_pZoneName = name;
}

void CHud::SetHelpMessage(wchar *message, bool quick)
{
	if (!CReplay::IsPlayingBack()) {
		CMessages::WideStringCopy(m_HelpMessage, message, HELP_MSG_LENGTH);
		CMessages::InsertPlayerControlKeysInString(m_HelpMessage);

		for (int i = 0; i < HELP_MSG_LENGTH; i++) {
			m_LastHelpMessage[i] = 0;
		}

		m_HelpMessageState = 0;
		m_HelpMessageQuick = quick;
	}
}

void CHud::SetVehicleName(wchar *name)
{
	m_pVehicleName = name;
}

void CHud::Draw()
{
	// disable hud via second controller
	if (CPad::GetPad(1)->GetStartJustDown())
		m_Wants_To_Draw_Hud = !m_Wants_To_Draw_Hud;

#ifdef GTA_PC
	if (CReplay::IsPlayingBack())
		return;
#endif

	if (m_Wants_To_Draw_Hud && !TheCamera.m_WideScreenOn) {
		bool DrawCrossHair = false;
#ifdef GTA_PC
		bool DrawCrossHairPC = false;
#endif

		int32 WeaponType = FindPlayerPed()->m_weapons[FindPlayerPed()->m_currentWeapon].m_eWeaponType;
		int32 Mode = TheCamera.Cams[TheCamera.ActiveCam].Mode;

		if (Mode == CCam::MODE_SNIPER || Mode == CCam::MODE_ROCKETLAUNCHER || Mode == CCam::MODE_M16_1STPERSON
#ifdef GTA_PC
			|| Mode == CCam::MODE_HELICANNON_1STPERSON
#endif
			)
		{
			DrawCrossHair = true;
		}
		
#ifdef GTA_PC
		if (Mode == CCam::MODE_M16_1STPERSON_RUNABOUT || Mode == CCam::MODE_ROCKETLAUNCHER_RUNABOUT || Mode == CCam::MODE_SNIPER_RUNABOUT)
			DrawCrossHairPC = true;

		/*
			Draw Crosshairs
		*/
		if (TheCamera.Cams[TheCamera.ActiveCam].Using3rdPersonMouseCam() &&
		    (!CPad::GetPad(0)->GetLookBehindForPed() || TheCamera.m_bPlayerIsInGarage) || Mode == CCam::MODE_1STPERSON_RUNABOUT) {
			if (FindPlayerPed() && !FindPlayerPed()->EnteringCar()) {
				if ((WeaponType >= WEAPONTYPE_COLT45 && WeaponType <= WEAPONTYPE_M16) || WeaponType == WEAPONTYPE_FLAMETHROWER)
					DrawCrossHairPC = true;
			}
		}
#endif

		if ( DrawCrossHair 
#ifdef GTA_PC
			|| DrawCrossHairPC  
#endif
			)
		{
			RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void *)rwFILTERLINEAR);

			SpriteBrightness = Min(SpriteBrightness+1, 30);

			RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)FALSE);

			float fStep = Sin((CTimer::GetTimeInMilliseconds() & 1023)/1024.0f * 6.28f);
			float fMultBright = SpriteBrightness / 30.0f * (0.25f * fStep + 0.75f);
			CRect rect;
#ifdef GTA_PC
			if (DrawCrossHairPC && TheCamera.Cams[TheCamera.ActiveCam].Using3rdPersonMouseCam()) {
				float f3rdX = SCREEN_WIDTH * TheCamera.m_f3rdPersonCHairMultX;
				float f3rdY = SCREEN_HEIGHT * TheCamera.m_f3rdPersonCHairMultY;
#ifdef ASPECT_RATIO_SCALE
				f3rdY -= SCREEN_SCALE_Y(2.0f);
#endif
				if (FindPlayerPed() && WeaponType == WEAPONTYPE_M16) {
					rect.left = f3rdX - SCREEN_SCALE_X(32.0f * 0.6f);
					rect.top = f3rdY - SCREEN_SCALE_Y(32.0f  * 0.6f);
					rect.right = f3rdX + SCREEN_SCALE_X(32.0f * 0.6f);
					rect.bottom = f3rdY + SCREEN_SCALE_Y(32.0f  * 0.6f);

					Sprites[HUD_SITEM16].Draw(CRect(rect), CRGBA(255, 255, 255, 255),
						0.0f, 0.0f,  1.0f, 0.0f,  0.0f, 1.0f,  1.0f, 1.0f);
				}
				else {
					rect.left = f3rdX - SCREEN_SCALE_X(32.0f * 0.4f);
					rect.top = f3rdY - SCREEN_SCALE_Y(32.0f  * 0.4f);
					rect.right = f3rdX + SCREEN_SCALE_X(32.0f * 0.4f);
					rect.bottom = f3rdY + SCREEN_SCALE_Y(32.0f  * 0.4f);

					Sprites[HUD_SITEM16].Draw(CRect(rect), CRGBA(255, 255, 255, 255),
						0.0f, 0.0f,  1.0f, 0.0f,  0.0f, 1.0f,  1.0f, 1.0f);
				}
			}
			else
#endif
			{
				if (Mode == CCam::MODE_M16_1STPERSON
#ifdef GTA_PC
					|| Mode == CCam::MODE_M16_1STPERSON_RUNABOUT
					|| Mode == CCam::MODE_HELICANNON_1STPERSON
#endif
					)
				{
					rect.left = (SCREEN_WIDTH / 2) - SCREEN_SCALE_X(32.0f);
					rect.top = (SCREEN_HEIGHT / 2) - SCREEN_SCALE_Y(32.0f);
					rect.right = (SCREEN_WIDTH / 2) + SCREEN_SCALE_X(32.0f);
					rect.bottom = (SCREEN_HEIGHT / 2) + SCREEN_SCALE_Y(32.0f);
					Sprites[HUD_SITEM16].Draw(CRect(rect), CRGBA(255, 255, 255, 255),
						0.0f, 0.0f,  1.0f, 0.0f,  0.0f, 1.0f,  1.0f, 1.0f);
				}
#ifdef GTA_PC
				else if (Mode == CCam::MODE_1STPERSON_RUNABOUT) {
					rect.left = (SCREEN_WIDTH / 2) - SCREEN_SCALE_X(32.0f * 0.7f);
					rect.top = (SCREEN_HEIGHT / 2) - SCREEN_SCALE_Y(32.0f * 0.7f);
					rect.right = (SCREEN_WIDTH / 2) + SCREEN_SCALE_X(32.0f * 0.7f);
					rect.bottom = (SCREEN_HEIGHT / 2) + SCREEN_SCALE_Y(32.0f * 0.7f);

					Sprites[HUD_SITEM16].Draw(CRect(rect), CRGBA(255, 255, 255, 255),
						0.0f, 0.0f,  1.0f, 0.0f,  0.0f, 1.0f,  1.0f, 1.0f);
				}
#endif
				else if (Mode == CCam::MODE_ROCKETLAUNCHER
#ifdef GTA_PC
					|| Mode == CCam::MODE_ROCKETLAUNCHER_RUNABOUT
#endif
					)
				{
					RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void *)TRUE);
					RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)rwBLENDONE);
					RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)rwBLENDONE);
					RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)FALSE);
					RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*)FALSE);
					RwRenderStateSet(rwRENDERSTATETEXTURERASTER, RwTextureGetRaster(gpRocketSightTex));
					CSprite::RenderOneXLUSprite(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 1.0f, SCREEN_SCALE_X_PC(40.0f), SCREEN_SCALE_Y_PC(40.0f), (100.0f * fMultBright), (200.0f * fMultBright), (100.0f * fMultBright), 255, 1.0f, 255);
				}
				else {
					// Sniper
					rect.left = SCREEN_WIDTH/2 - SCREEN_SCALE_X(210.0f);
					rect.top = SCREEN_HEIGHT/2 - SCREEN_SCALE_Y(210.0f);
					rect.right = SCREEN_WIDTH/2;
					rect.bottom = SCREEN_HEIGHT/2;
					Sprites[HUD_SITESNIPER].Draw(CRect(rect), CRGBA(255, 255, 255, 255),
						0.01f, 0.01f,  1.0f, 0.0f,  0.01f, 1.0f,  1.0f, 1.0f);

					rect.left = SCREEN_WIDTH/2;
					rect.top = SCREEN_HEIGHT/2 - SCREEN_SCALE_Y(210.0f);
					rect.right = SCREEN_WIDTH/2 + SCREEN_SCALE_X(210.0f);
					rect.bottom = SCREEN_HEIGHT/2;
					Sprites[HUD_SITESNIPER].Draw(CRect(rect), CRGBA(255, 255, 255, 255),
						0.99f, 0.0f,  0.01f, 0.01f,  0.99f, 1.0f,  0.01f, 1.0f);

					rect.left = SCREEN_WIDTH/2 - SCREEN_SCALE_X(210.0f);
					rect.top = SCREEN_HEIGHT/2;
					rect.right = SCREEN_WIDTH/2;
					rect.bottom = SCREEN_HEIGHT/2 + SCREEN_SCALE_Y(210.0f);
					Sprites[HUD_SITESNIPER].Draw(CRect(rect), CRGBA(255, 255, 255, 255),
						0.01f, 0.99f,  1.0f, 0.99f,  0.01f, 0.01f,  1.0f, 0.01f);

					rect.left = SCREEN_WIDTH/2;
					rect.top = SCREEN_HEIGHT/2;
					rect.right = SCREEN_WIDTH/2 + SCREEN_SCALE_X(210.0f);
					rect.bottom = SCREEN_HEIGHT/2 + SCREEN_SCALE_Y(210.0f);
					Sprites[HUD_SITESNIPER].Draw(CRect(rect), CRGBA(255, 255, 255, 255),
						0.99f, 0.99f,  0.01f, 0.99f,  0.99f, 0.01f,  0.01f, 0.01f);
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
		CFont::PrintString(SCREEN_SCALE_FROM_RIGHT(MONEY_X) + SCREEN_SCALE_X_FIX(2.0f), SCREEN_SCALE_Y(43.0f) + SCREEN_SCALE_Y_FIX(2.0f), sPrint);
		CFont::SetColor(MONEY_COLOR);
		CFont::PrintString(SCREEN_SCALE_FROM_RIGHT(MONEY_X), SCREEN_SCALE_Y(43.0f), sPrint);

		/*
			DrawAmmo
		*/
		int32 AmmoAmount = CWeaponInfo::GetWeaponInfo(FindPlayerPed()->GetWeapon()->m_eWeaponType)->m_nAmountofAmmunition;
		int32 AmmoInClip = FindPlayerPed()->m_weapons[FindPlayerPed()->m_currentWeapon].m_nAmmoInClip;
		int32 TotalAmmo = FindPlayerPed()->m_weapons[FindPlayerPed()->m_currentWeapon].m_nAmmoTotal;
		int32 Ammo, Clip;

		if (AmmoAmount <= 1 || AmmoAmount >= 1000)
			sprintf(sTemp, "%d", TotalAmmo);
		else {
			if (WeaponType == WEAPONTYPE_FLAMETHROWER) {
				Clip = AmmoInClip / 10;

				Ammo = Min((TotalAmmo - AmmoInClip) / 10, 9999);
			}
			else {
				Clip = AmmoInClip;

				Ammo = Min(TotalAmmo - AmmoInClip, 9999);
			}

			sprintf(sTemp, "%d-%d", Ammo, Clip);
		}

		AsciiToUnicode(sTemp, sPrint);

		/*
			DrawWeaponIcon
		*/
		Sprites[WeaponType].Draw(
			CRect(
				SCREEN_SCALE_FROM_RIGHT(WEAPON_X),
				SCREEN_SCALE_Y(27.0f),
				SCREEN_SCALE_FROM_RIGHT(WEAPON_X)+SCREEN_SCALE_X(64.0f),
				SCREEN_SCALE_Y(27.0f)+SCREEN_SCALE_Y(64.0f)),
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
		CFont::SetCentreSize(SCREEN_WIDTH);
		CFont::SetPropOn();
		CFont::SetFontStyle(FONT_BANK);

		if (!CDarkel::FrenzyOnGoing() && WeaponType != WEAPONTYPE_UNARMED && WeaponType != WEAPONTYPE_BASEBALLBAT) {
			CFont::SetColor(AMMO_COLOR);
			CFont::PrintString(SCREEN_SCALE_FROM_RIGHT(AMMO_X), SCREEN_SCALE_Y(73.0f), sPrint);
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

		if (m_ItemToFlash == ITEM_HEALTH && FRAMECOUNTER & 8
			|| m_ItemToFlash != ITEM_HEALTH
			|| FindPlayerPed()->m_fHealth < 10
			&& FRAMECOUNTER & 8) {
			if (FindPlayerPed()->m_fHealth >= 10
				|| FindPlayerPed()->m_fHealth < 10 && FRAMECOUNTER & 8) {

				AsciiToUnicode("{", sPrintIcon);
#ifdef FIX_BUGS
				sprintf(sTemp, "%03d", int32(FindPlayerPed()->m_fHealth + 0.5f));
#else
				sprintf(sTemp, "%03d", (int32)FindPlayerPed()->m_fHealth);
#endif
				AsciiToUnicode(sTemp, sPrint);
				CFont::SetColor(CRGBA(0, 0, 0, 255));
				CFont::PrintString(SCREEN_SCALE_FROM_RIGHT(HEALTH_X) + SCREEN_SCALE_X_FIX(2.0f), SCREEN_SCALE_Y(65.0f) + SCREEN_SCALE_Y_FIX(2.0f), sPrint);

				if (!CWorld::Players[CWorld::PlayerInFocus].m_nTimeLastHealthLoss || CTimer::GetTimeInMilliseconds() > CWorld::Players[CWorld::PlayerInFocus].m_nTimeLastHealthLoss + 2000 || FRAMECOUNTER & 4)
					CFont::PrintString(SCREEN_SCALE_FROM_RIGHT(HEALTH_X) + SCREEN_SCALE_X_FIX(2.0f) - SCREEN_SCALE_X(56.0f) + SCREEN_SCALE_X(2.0f), SCREEN_SCALE_Y(65.0f) + SCREEN_SCALE_Y_FIX(2.0f), sPrintIcon);
				
				CFont::SetColor(HEALTH_COLOR);

				CFont::PrintString(SCREEN_SCALE_FROM_RIGHT(HEALTH_X), SCREEN_SCALE_Y(65.0f), sPrint);

				if (!CWorld::Players[CWorld::PlayerInFocus].m_nTimeLastHealthLoss || CTimer::GetTimeInMilliseconds() > CWorld::Players[CWorld::PlayerInFocus].m_nTimeLastHealthLoss + 2000 || FRAMECOUNTER & 4)
					CFont::PrintString(SCREEN_SCALE_FROM_RIGHT(HEALTH_X) + SCREEN_SCALE_X_FIX(2.0f) - SCREEN_SCALE_X(56.0f), SCREEN_SCALE_Y(65.0f), sPrintIcon);
			}
		}

		/*
			DrawArmour
		*/
		if (m_ItemToFlash == ITEM_ARMOUR && FRAMECOUNTER & 8 || m_ItemToFlash != ITEM_ARMOUR) {
			CFont::SetScale(SCREEN_SCALE_X(0.8f), SCREEN_SCALE_Y(1.35f));
			if (FindPlayerPed()->m_fArmour > 1.0f) {
				AsciiToUnicode("[", sPrintIcon);
#ifdef FIX_BUGS
				sprintf(sTemp, "%03d", int32(FindPlayerPed()->m_fArmour + 0.5f));
#else
				sprintf(sTemp, "%03d", (int32)FindPlayerPed()->m_fArmour);
#endif
				AsciiToUnicode(sTemp, sPrint);

				CFont::SetColor(CRGBA(0, 0, 0, 255));
				CFont::PrintString(SCREEN_SCALE_FROM_RIGHT(182.0f) + SCREEN_SCALE_X_FIX(2.0f), SCREEN_SCALE_Y(65.0f) + SCREEN_SCALE_Y_FIX(2.0f), sPrint);

				if (!CWorld::Players[CWorld::PlayerInFocus].m_nTimeLastArmourLoss || CTimer::GetTimeInMilliseconds() > CWorld::Players[CWorld::PlayerInFocus].m_nTimeLastArmourLoss + 2000 || FRAMECOUNTER & 4)
					CFont::PrintString(SCREEN_SCALE_FROM_RIGHT(182.0f) + SCREEN_SCALE_X_FIX(2.0f) - SCREEN_SCALE_X(54.0f) + SCREEN_SCALE_X(2.0f), SCREEN_SCALE_Y(65.0f) + SCREEN_SCALE_Y_FIX(2.0f), sPrintIcon);

				CFont::SetColor(ARMOUR_COLOR);

				CFont::PrintString(SCREEN_SCALE_FROM_RIGHT(182.0f), SCREEN_SCALE_Y(65.0f), sPrint);

				if (!CWorld::Players[CWorld::PlayerInFocus].m_nTimeLastArmourLoss || CTimer::GetTimeInMilliseconds() > CWorld::Players[CWorld::PlayerInFocus].m_nTimeLastArmourLoss + 2000 || FRAMECOUNTER & 1) {					
					CFont::PrintString(SCREEN_SCALE_FROM_RIGHT(182.0f) - SCREEN_SCALE_X(54.0f) + SCREEN_SCALE_X(2.0f), SCREEN_SCALE_Y(65.0f), sPrintIcon);
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
		
		float fStarsX = SCREEN_SCALE_FROM_RIGHT(STARS_X);

		for (int i = 0; i < 6; i++) {
			CFont::SetColor(CRGBA(0, 0, 0, 255));
			CFont::PrintString(fStarsX + SCREEN_SCALE_X_FIX(2.0f), SCREEN_SCALE_Y(87.0f) + SCREEN_SCALE_Y_FIX(2.0f), sPrintIcon);

			if (FindPlayerPed()->m_pWanted->GetWantedLevel() > i
				&& (CTimer::GetTimeInMilliseconds() > FindPlayerPed()->m_pWanted->m_nLastWantedLevelChange
					+ 2000 || FRAMECOUNTER & 4)) {

				CFont::SetColor(WANTED_COLOR);
				CFont::PrintString(fStarsX, SCREEN_SCALE_Y(87.0f), sPrintIcon);
			}
			
			fStarsX -= SCREEN_SCALE_X(23.0f);
		}

		/*
			DrawZoneName
		*/
		if (m_pZoneName) {
			float fZoneAlpha = 255.0f;

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
					m_ZoneNameTimer = 5;
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
					m_ZoneFadeTimer = 1000;
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
					fZoneAlpha = m_ZoneFadeTimer / 1000.0f * 255.0f;
					break;
				case 3:
					m_ZoneFadeTimer -= CTimer::GetTimeStepInMilliseconds();
					if (m_ZoneFadeTimer < 0) {
						m_ZoneState = 0;
						m_ZoneFadeTimer = 0;
					}
					fZoneAlpha = m_ZoneFadeTimer / 1000.0f * 255.0f;
					break;
				case 4:
					m_ZoneFadeTimer -= CTimer::GetTimeStepInMilliseconds();
					if (m_ZoneFadeTimer < 0) {
						m_ZoneFadeTimer = 0;
						m_ZoneToPrint = m_pLastZoneName;
						m_ZoneState = 2;
					}
					fZoneAlpha = m_ZoneFadeTimer / 1000.0f * 255.0f;
					break;
				default:
					break;

				}

#ifndef HUD_ENHANCEMENTS
				if (!m_Message[0])
#else
				if (!m_Message[0] && !m_BigMessage[2][0]) // Hide zone name if wasted/busted text is displaying
#endif
				{
					m_ZoneNameTimer += CTimer::GetTimeStepInMilliseconds();
					CFont::SetJustifyOff();
					CFont::SetPropOn();
					CFont::SetBackgroundOff();

					if (FrontEndMenuManager.m_PrefsLanguage == CMenuManager::LANGUAGE_SPANISH)
						CFont::SetScale(SCREEN_SCALE_X(1.2f * 0.8f), SCREEN_SCALE_Y(1.2f));
					else
						CFont::SetScale(SCREEN_SCALE_X(1.2f), SCREEN_SCALE_Y(1.2f));

					CFont::SetRightJustifyOn();
					CFont::SetRightJustifyWrap(0.0f);
					CFont::SetBackGroundOnlyTextOff();
					CFont::SetFontStyle(FONT_BANK);
					CFont::SetColor(CRGBA(0, 0, 0, fZoneAlpha));
					CFont::PrintString(SCREEN_SCALE_FROM_RIGHT(32.0f) + SCREEN_SCALE_X_FIX(1.0f), SCREEN_SCALE_FROM_BOTTOM(ZONE_Y) + SCREEN_SCALE_Y_FIX(1.0f), m_ZoneToPrint);
					CFont::SetColor(CRGBA(ZONE_COLOR.r, ZONE_COLOR.g, ZONE_COLOR.b, fZoneAlpha));
					CFont::PrintString(SCREEN_SCALE_FROM_RIGHT(32.0f), SCREEN_SCALE_FROM_BOTTOM(ZONE_Y), m_ZoneToPrint);
				}
			}
		}

		/*
			DrawVehicleName
		*/
		if (m_pVehicleName) {
			float fVehicleAlpha = 0.0f;

			if (m_pVehicleName != m_pLastVehicleName) {
				switch (m_VehicleState) {
				case 0:
					m_VehicleState = 2;
					m_pVehicleNameToPrint = m_pVehicleName;
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
				m_pLastVehicleName = m_pVehicleName;
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
					fVehicleAlpha = m_VehicleFadeTimer / 1000.0f * 255.0f;
					break;
				case 3:
					m_VehicleFadeTimer -= CTimer::GetTimeStepInMilliseconds();
					if (m_VehicleFadeTimer < 0) {
						m_VehicleState = 0;
						m_VehicleFadeTimer = 0;
					}
					fVehicleAlpha = m_VehicleFadeTimer / 1000.0f * 255.0f;
					break;
				case 4:
					m_VehicleFadeTimer -= CTimer::GetTimeStepInMilliseconds();
					if (m_VehicleFadeTimer < 0) {
						m_VehicleFadeTimer = 0;
						m_pVehicleNameToPrint = m_pLastVehicleName;
						m_VehicleNameTimer = 0;
						m_VehicleState = 2;
					}
					fVehicleAlpha = m_VehicleFadeTimer / 1000.0f * 255.0f;
					break;
				default:
					break;
				}

#ifndef HUD_ENHANCEMENTS
				if (!m_Message[0])
#else
				if (!m_Message[0] && !m_BigMessage[2][0]) // Hide vehicle name if wasted/busted text is displaying
#endif
				{
					m_VehicleNameTimer += CTimer::GetTimeStepInMilliseconds();
					CFont::SetJustifyOff();
					CFont::SetPropOn();
					CFont::SetBackgroundOff();

					if (FrontEndMenuManager.m_PrefsLanguage != CMenuManager::LANGUAGE_ITALIAN && FrontEndMenuManager.m_PrefsLanguage != CMenuManager::LANGUAGE_SPANISH)
						CFont::SetScale(SCREEN_SCALE_X(1.2f), SCREEN_SCALE_Y(1.2f));
					else
						CFont::SetScale(SCREEN_SCALE_X(1.2f * 0.85f), SCREEN_SCALE_Y(1.2f));

					CFont::SetRightJustifyOn();
					CFont::SetRightJustifyWrap(0.0f);
					CFont::SetBackGroundOnlyTextOff();
					CFont::SetFontStyle(FONT_BANK);
					CFont::SetColor(CRGBA(0, 0, 0, fVehicleAlpha));
					CFont::PrintString(SCREEN_SCALE_FROM_RIGHT(32.0f) + SCREEN_SCALE_X_FIX(1.0f), SCREEN_SCALE_FROM_BOTTOM(VEHICLE_Y) + SCREEN_SCALE_Y_FIX(1.0f), m_pVehicleNameToPrint);
					CFont::SetColor(CRGBA(VEHICLE_COLOR.r, VEHICLE_COLOR.g, VEHICLE_COLOR.b, fVehicleAlpha));
					CFont::PrintString(SCREEN_SCALE_FROM_RIGHT(32.0f), SCREEN_SCALE_FROM_BOTTOM(VEHICLE_Y), m_pVehicleNameToPrint);
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
		CFont::PrintString(SCREEN_SCALE_FROM_RIGHT(CLOCK_X) + SCREEN_SCALE_X_FIX(2.0f), SCREEN_SCALE_Y(22.0f) + SCREEN_SCALE_Y_FIX(2.0f), sPrint);
		CFont::SetColor(CLOCK_COLOR);
		CFont::PrintString(SCREEN_SCALE_FROM_RIGHT(CLOCK_X), SCREEN_SCALE_Y(22.0f), sPrint);

		/*
			DrawOnScreenTimer
		*/
		wchar sTimer[16];

		if (!CUserDisplay::OnscnTimer.m_sEntries[0].m_bTimerProcessed)
			TimerOnLastFrame = false;
		if (!CUserDisplay::OnscnTimer.m_sEntries[0].m_bCounterProcessed)
			CounterOnLastFrame = false;

		if (CUserDisplay::OnscnTimer.m_bProcessed) {
			if (CUserDisplay::OnscnTimer.m_sEntries[0].m_bTimerProcessed) {
				if (!TimerOnLastFrame)
					TimerFlashTimer = 1;

				TimerOnLastFrame = true;

				if (TimerFlashTimer) {
					if (++TimerFlashTimer > 50)
						TimerFlashTimer = 0;
				}

				if (FRAMECOUNTER & 4 || !TimerFlashTimer) {
					AsciiToUnicode(CUserDisplay::OnscnTimer.m_sEntries[0].m_bTimerBuffer, sTimer);
					CFont::SetPropOn();
					CFont::SetBackgroundOff();
					CFont::SetScale(SCREEN_SCALE_X(0.8f), SCREEN_SCALE_Y(1.35f));
					CFont::SetRightJustifyOn();
					CFont::SetRightJustifyWrap(0.0f);
					CFont::SetFontStyle(FONT_LOCALE(FONT_HEADING));
					CFont::SetPropOff();
					CFont::SetBackGroundOnlyTextOn();
					CFont::SetColor(CRGBA(0, 0, 0, 255));
					CFont::PrintString(SCREEN_SCALE_FROM_RIGHT(TIMER_RIGHT_OFFSET) + SCREEN_SCALE_X_FIX(2.0f), SCREEN_SCALE_Y(110.0f) + SCREEN_SCALE_Y_FIX(2.0f), sTimer);
					CFont::SetScale(SCREEN_SCALE_X(0.8f), SCREEN_SCALE_Y(1.35f));
					CFont::SetColor(TIMER_COLOR);
					CFont::PrintString(SCREEN_SCALE_FROM_RIGHT(TIMER_RIGHT_OFFSET), SCREEN_SCALE_Y(110.0f), sTimer);

					if (CUserDisplay::OnscnTimer.m_sEntries[0].m_aTimerText[0]) {
						CFont::SetPropOn();
						CFont::SetColor(CRGBA(0, 0, 0, 255));
						CFont::SetScale(SCREEN_SCALE_X(0.8f * 0.8f), SCREEN_SCALE_Y(1.35f));
						CFont::PrintString(SCREEN_SCALE_FROM_RIGHT(TIMER_RIGHT_OFFSET) - SCREEN_SCALE_X(80.0f) + SCREEN_SCALE_X(2.0f), SCREEN_SCALE_Y(110.0f) + SCREEN_SCALE_Y_FIX(2.0f), TheText.Get(CUserDisplay::OnscnTimer.m_sEntries[0].m_aTimerText));
						CFont::SetColor(TIMER_COLOR);
						CFont::PrintString(SCREEN_SCALE_FROM_RIGHT(TIMER_RIGHT_OFFSET) - SCREEN_SCALE_X(80.0f), SCREEN_SCALE_Y(110.0f), TheText.Get(CUserDisplay::OnscnTimer.m_sEntries[0].m_aTimerText));
					}
				}
			}
			if (CUserDisplay::OnscnTimer.m_sEntries[0].m_bCounterProcessed) {
				if (!CounterOnLastFrame)
					CounterFlashTimer = 1;

				CounterOnLastFrame = true;

				if (CounterFlashTimer) {
					if (++CounterFlashTimer > 50)
						CounterFlashTimer = 0;
				}

				if (FRAMECOUNTER & 4 || !CounterFlashTimer) {
					if (CUserDisplay::OnscnTimer.m_sEntries[0].m_nType == COUNTER_DISPLAY_NUMBER) {
						AsciiToUnicode(CUserDisplay::OnscnTimer.m_sEntries[0].m_bCounterBuffer, sTimer);
						CFont::SetPropOn();
						CFont::SetBackgroundOff();
						CFont::SetScale(SCREEN_SCALE_X(0.8f), SCREEN_SCALE_Y(1.35f));
						CFont::SetCentreOff();
						CFont::SetRightJustifyOn();
						CFont::SetRightJustifyWrap(0.0f);
						CFont::SetFontStyle(FONT_LOCALE(FONT_HEADING));
						CFont::SetColor(CRGBA(244, 20, 20, 255));
						CFont::SetWrapx(SCREEN_STRETCH_X(DEFAULT_SCREEN_WIDTH));
						CFont::SetPropOff();
						CFont::SetBackGroundOnlyTextOn();
						CFont::SetColor(CRGBA(0, 0, 0, 255));
						CFont::PrintString(SCREEN_SCALE_FROM_RIGHT(TIMER_RIGHT_OFFSET) + SCREEN_SCALE_X_FIX(2.0f), SCREEN_SCALE_Y(132.0f) + SCREEN_SCALE_Y_FIX(2.0f), sTimer);
						CFont::SetColor(COUNTER_COLOR);
						CFont::PrintString(SCREEN_SCALE_FROM_RIGHT(TIMER_RIGHT_OFFSET), SCREEN_SCALE_Y(132.0f), sTimer);
					} else {
						int counter = atoi(CUserDisplay::OnscnTimer.m_sEntries[0].m_bCounterBuffer);
#ifdef FIX_BUGS
						counter = Min(counter, 100);
#endif
						CSprite2d::DrawRect
						(
							CRect
							(
								SCREEN_SCALE_FROM_RIGHT(TIMER_RIGHT_OFFSET) - SCREEN_SCALE_X(100.0f) / 2 + SCREEN_SCALE_X_FIX(4.0f),
								SCREEN_SCALE_Y(132.0f) + SCREEN_SCALE_Y(8.0f),
								SCREEN_SCALE_FROM_RIGHT(TIMER_RIGHT_OFFSET) + SCREEN_SCALE_X_FIX(4.0f),
								SCREEN_SCALE_Y(132.0f) + SCREEN_SCALE_Y_PC(11.0f) + SCREEN_SCALE_Y(8.0f)
							),
							CRGBA(0, 106, 164, 80)
						);
						
						CSprite2d::DrawRect
						(
							CRect
							(
								SCREEN_SCALE_FROM_RIGHT(TIMER_RIGHT_OFFSET) - SCREEN_SCALE_X(100.0f) / 2 + SCREEN_SCALE_X_FIX(4.0f), 
								SCREEN_SCALE_Y(132.0f) + SCREEN_SCALE_Y(8.0f),
								SCREEN_SCALE_X_PC((float)counter) / 2.0f + SCREEN_SCALE_FROM_RIGHT(TIMER_RIGHT_OFFSET) - SCREEN_SCALE_X(100.0f) / 2.0f + SCREEN_SCALE_X_FIX(4.0f),
								SCREEN_SCALE_Y(132.0f) + SCREEN_SCALE_Y_PC(11.0f) + SCREEN_SCALE_Y(8.0f)
							),
							CRGBA(0, 106, 164, 255)
						);
					}

					if (CUserDisplay::OnscnTimer.m_sEntries[0].m_aCounterText[0]) {
						CFont::SetPropOn();
						CFont::SetScale(SCREEN_SCALE_X(0.8f), SCREEN_SCALE_Y(1.35f));
						CFont::SetColor(CRGBA(0, 0, 0, 255));
						CFont::PrintString(SCREEN_SCALE_FROM_RIGHT(TIMER_RIGHT_OFFSET) - SCREEN_SCALE_X(61.0f) + SCREEN_SCALE_X(2.0f), SCREEN_SCALE_Y(132.0f) + SCREEN_SCALE_Y_FIX(2.0f), TheText.Get(CUserDisplay::OnscnTimer.m_sEntries[0].m_aCounterText));
						CFont::SetColor(COUNTER_COLOR);
						CFont::PrintString(SCREEN_SCALE_FROM_RIGHT(TIMER_RIGHT_OFFSET) - SCREEN_SCALE_X(61.0f), SCREEN_SCALE_Y(132.0f), TheText.Get(CUserDisplay::OnscnTimer.m_sEntries[0].m_aCounterText));
					}
				}
			}
		}

		/////////////////////////////////
		/*
			DrawPager
		*/
		if (!m_PagerMessage[0] && PagerOn == 1) {
			PagerSoundPlayed = false;
			PagerOn = 2;
		}
		if (m_PagerMessage[0] || PagerOn == 2) {
			if (!PagerOn) {
				PagerOn = 1;
				PagerXOffset = 150.0f;
			}
			if (PagerOn == 1) {
				if (PagerXOffset > 0.0f) {
					float fStep = PagerXOffset * 0.1f;
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
				float fStep = PagerXOffset * 0.1f;
				if (fStep < 2.0f)
					fStep = 2.0f;
				PagerXOffset += fStep;
				if (PagerXOffset > 150.0f) {
					PagerXOffset = 150.0f;
					PagerOn = 0;
				}
			}
			Sprites[HUD_PAGER].Draw(CRect(SCREEN_SCALE_X(26.0f) - SCREEN_SCALE_X_FIX(PagerXOffset), SCREEN_SCALE_Y(27.0f), SCREEN_SCALE_X(160.0f) + SCREEN_SCALE_X(26.0f) - SCREEN_SCALE_X_FIX(PagerXOffset), SCREEN_SCALE_Y(80.0f) + SCREEN_SCALE_Y(27.0f)), CRGBA(255, 255, 255, 255));		
			CFont::SetBackgroundOff();
			CFont::SetScale(SCREEN_SCALE_X(0.84f), SCREEN_SCALE_Y(1.0f));
			CFont::SetColor(PAGER_COLOR);
			CFont::SetRightJustifyOff();
			CFont::SetBackgroundOff();
			CFont::SetCentreOff();
			CFont::SetWrapx(SCREEN_STRETCH_X(DEFAULT_SCREEN_WIDTH));
			CFont::SetJustifyOff();
			CFont::SetPropOff();
			CFont::SetFontStyle(FONT_PAGER);
			CFont::PrintString(SCREEN_SCALE_X(52.0f) - SCREEN_SCALE_X_FIX(PagerXOffset), SCREEN_SCALE_Y(54.0f), m_PagerMessage);
		}

		/*
			DrawRadar
		*/
		if (m_ItemToFlash == ITEM_RADAR && FRAMECOUNTER & 8 || m_ItemToFlash != ITEM_RADAR) {
			CRadar::DrawMap();
			CRect rect(0.0f, 0.0f, SCREEN_SCALE_X(RADAR_WIDTH), SCREEN_SCALE_Y(RADAR_HEIGHT));
			rect.Translate(SCREEN_SCALE_X_FIX(RADAR_LEFT), SCREEN_SCALE_FROM_BOTTOM(RADAR_BOTTOM + RADAR_HEIGHT));

#ifdef PS2_HUD
	#ifdef FIX_BUGS
			rect.Grow(SCREEN_SCALE_X(2.0f), SCREEN_SCALE_X(4.0f), SCREEN_SCALE_Y(2.0f), SCREEN_SCALE_Y(4.0f));
	#else
			rect.Grow(2.0f, 4.0f);
	#endif
#else
	#ifdef FIX_BUGS
			rect.Grow(SCREEN_SCALE_X(4.0f), SCREEN_SCALE_X(4.0f), SCREEN_SCALE_Y(4.0f), SCREEN_SCALE_Y(4.0f));
	#else
			rect.Grow(4.0f);
	#endif
#endif
			Sprites[HUD_RADARDISC].Draw(rect, RADARDISC_COLOR);
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
		for (int i = 0; i < ARRAY_SIZE(CTheScripts::IntroTextLines); i++) {
			if (CTheScripts::IntroTextLines[i].m_Text[0] && CTheScripts::IntroTextLines[i].m_bTextBeforeFade) {
				CFont::SetScale(SCREEN_SCALE_X_PC(CTheScripts::IntroTextLines[i].m_fScaleX), SCREEN_SCALE_Y_PC(CTheScripts::IntroTextLines[i].m_fScaleY) 
#if !defined(PS2_HUD) || defined(FIX_BUGS)
					* 0.5f
#endif
				);

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

				CFont::SetWrapx(SCALE_AND_CENTER_X_PC(CTheScripts::IntroTextLines[i].m_fWrapX));
				
				CFont::SetCentreSize(SCREEN_SCALE_X_PC(CTheScripts::IntroTextLines[i].m_fCenterSize));
				
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

				CFont::SetFontStyle(FONT_LOCALE(CTheScripts::IntroTextLines[i].m_nFont));
				
#if defined(PS2_HUD) && !defined(FIX_BUGS)
				CFont::PrintString(CTheScripts::IntroTextLines[i].m_fAtX, CTheScripts::IntroTextLines[i].m_fAtY, CTheScripts::IntroTextLines[i].m_Text);
#else
				CFont::PrintString(SCALE_AND_CENTER_X(DEFAULT_SCREEN_WIDTH - CTheScripts::IntroTextLines[i].m_fAtX), SCREEN_SCALE_Y(DEFAULT_SCREEN_HEIGHT - CTheScripts::IntroTextLines[i].m_fAtY), CTheScripts::IntroTextLines[i].m_Text);
#endif
			}
		}
		for (int i = 0; i < ARRAY_SIZE(CTheScripts::IntroRectangles); i++) {
			intro_script_rectangle &IntroRect = CTheScripts::IntroRectangles[i];

			// Yeah, top and bottom changed place. R* vision
			if (IntroRect.m_bIsUsed && IntroRect.m_bBeforeFade) {
				if (IntroRect.m_nTextureId >= 0) {
					CRect rect (
						IntroRect.m_sRect.left,
						IntroRect.m_sRect.bottom,
						IntroRect.m_sRect.right,
						IntroRect.m_sRect.top );

					CTheScripts::ScriptSprites[IntroRect.m_nTextureId].Draw(rect, IntroRect.m_sColor);
				}
				else {
					CRect rect (
						IntroRect.m_sRect.left,
						IntroRect.m_sRect.bottom,
						IntroRect.m_sRect.right,
						IntroRect.m_sRect.top );

					CSprite2d::DrawRect(rect, IntroRect.m_sColor);
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
			CFont::SetScale(SCREEN_SCALE_X_PC(0.48f), SCREEN_SCALE_Y_PC(1.12f));
			CFont::SetCentreOn();
			CFont::SetPropOn();
			CFont::SetFontStyle(FONT_LOCALE(FONT_BANK));

#ifdef XBOX_SUBTITLES
			float radarBulge = SCREEN_SCALE_X(45.0f) + SCREEN_SCALE_X(16.0f);
			float rectWidth = SCREEN_WIDTH - SCREEN_SCALE_X(45.0f) - SCREEN_SCALE_X(16.0f) - radarBulge;
			CFont::SetCentreSize(rectWidth);
			CFont::SetColor(CRGBA(180, 180, 180, 255));

			CFont::PrintOutlinedString(rectWidth / 2.0f + radarBulge, SCREEN_SCALE_Y(4.0f) + SCREEN_SCALE_FROM_BOTTOM(48.0f) - SCREEN_SCALE_Y(1), m_Message,
				2.0f, true, CRGBA(0, 0, 0, 255));
#else
			float radarBulge = SCREEN_SCALE_X(40.0f) + SCREEN_SCALE_X(8.0f);
			float rectWidth = SCREEN_SCALE_FROM_RIGHT(50.0f) - SCREEN_SCALE_X(8.0f) - radarBulge;
			
			CFont::SetCentreSize(rectWidth);

			const int16 shadow = 1;
			CFont::SetDropShadowPosition(shadow);
			CFont::SetDropColor(CRGBA(0, 0, 0, 255));
			CFont::SetColor(CRGBA(235, 235, 235, 255));

			// I'm not sure shadow substaction was intentional here, might be a leftover if CFont::PrintString was used for a shadow draw call
			CFont::PrintString(rectWidth / 2.0f + radarBulge - SCREEN_SCALE_X_FIX(shadow), SCREEN_SCALE_Y_PC(4.0f) + SCREEN_SCALE_FROM_BOTTOM(SUBS_Y) - SCREEN_SCALE_Y_FIX(shadow), m_Message);
			CFont::SetDropShadowPosition(0);
#endif // #ifdef XBOX_SUBTITLES
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
				
				if (CGame::frenchGame || CGame::germanGame)
					CFont::SetScale(SCREEN_SCALE_X_PC(1.8f), SCREEN_SCALE_Y_PC(1.8f));
				else
					CFont::SetScale(SCREEN_SCALE_X_PC(1.8f), SCREEN_SCALE_Y_PC(1.8f));
				
				CFont::SetPropOn();
				CFont::SetCentreOn();
				CFont::SetCentreSize(SCREEN_SCALE_X(DEFAULT_SCREEN_WIDTH - 25));
				CFont::SetColor(CRGBA(255, 255, 0, 255));
				CFont::SetFontStyle(FONT_HEADING);

				// Appearently sliding text in here was abandoned very early, since this text is centered now.
#ifdef FIX_BUGS
				if (BigMessageX[0] >= SCALE_AND_CENTER_X(DEFAULT_SCREEN_WIDTH-20))
#else
				if (BigMessageX[0] >= SCREEN_WIDTH-20)
#endif
				{
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
					BigMessageX[0] += SCREEN_SCALE_X_FIX(CTimer::GetTimeStepInMilliseconds() * 0.3f);
					BigMessageAlpha[0] += (CTimer::GetTimeStepInMilliseconds() * 0.3f);

					if (BigMessageAlpha[0] > 255.0f)
						BigMessageAlpha[0] = 255.0f;
				}
				CFont::SetColor(CRGBA(0, 0, 0, BigMessageAlpha[0]));

#if defined(PS2_HUD) && !defined(FIX_BUGS) // yeah, that's right. ps2 uses y=ScaleX(a)
				CFont::PrintString(SCREEN_WIDTH / 2 + SCREEN_SCALE_X_FIX(2.0f), (SCREEN_WIDTH / 2) - SCREEN_SCALE_X(120.0f) + SCREEN_SCALE_Y_FIX(2.0f), m_BigMessage[0]);
#else
				CFont::PrintString(SCREEN_WIDTH / 2 + SCREEN_SCALE_X_FIX(2.0f), (SCREEN_HEIGHT / 2) - SCREEN_SCALE_Y(BIGMESSAGE_Y_OFFSET) + SCREEN_SCALE_Y_FIX(2.0f), m_BigMessage[0]);
#endif
				CFont::SetColor(CRGBA(BIGMESSAGE_COLOR.r, BIGMESSAGE_COLOR.g, BIGMESSAGE_COLOR.b, BigMessageAlpha[0]));
#if defined(PS2_HUD) && !defined(FIX_BUGS) // same
				CFont::PrintString(SCREEN_WIDTH / 2, (SCREEN_WIDTH  / 2) - SCREEN_SCALE_X(120.0f), m_BigMessage[0]);
#else
				CFont::PrintString(SCREEN_WIDTH / 2, (SCREEN_HEIGHT / 2) - SCREEN_SCALE_Y(18.0f), m_BigMessage[0]);
#endif
			}
			else {
				BigMessageAlpha[0] = 0.0f;
				BigMessageX[0] = SCALE_AND_CENTER_X_FIX(-60.0f);
				BigMessageInUse[0] = 1.0f;
			}
		}
		else {
			BigMessageInUse[0] = 0.0f;
		}

		// WastedBustedText
		if (m_BigMessage[2][0]) {
			if (BigMessageInUse[2] != 0.0f) {
				BigMessageAlpha[2] += (CTimer::GetTimeStepInMilliseconds() * 0.4f);

				if (BigMessageAlpha[2] > 255.0f)
					BigMessageAlpha[2] = 255.0f;

				CFont::SetBackgroundOff();

				if (CGame::frenchGame || CGame::germanGame)
					CFont::SetScale(SCREEN_SCALE_X_PC(1.4f), SCREEN_SCALE_Y_PC(1.4f));
				else
					CFont::SetScale(SCREEN_SCALE_X_PC(2.0f), SCREEN_SCALE_Y_PC(2.0f));

				CFont::SetPropOn();
				CFont::SetRightJustifyOn();
				CFont::SetFontStyle(FONT_HEADING);

				CFont::SetColor(CRGBA(0, 0, 0, BigMessageAlpha[2]*0.75f));
				CFont::PrintString(SCREEN_SCALE_FROM_RIGHT(20.0f) + SCREEN_SCALE_X_FIX(4.0f), SCREEN_SCALE_FROM_BOTTOM(WASTEDBUSTED_Y) + SCREEN_SCALE_Y(4.0f), m_BigMessage[2]);
				CFont::SetColor(CRGBA(WASTEDBUSTED_COLOR.r, WASTEDBUSTED_COLOR.g, WASTEDBUSTED_COLOR.b, BigMessageAlpha[2]));
				CFont::PrintString(SCREEN_SCALE_FROM_RIGHT(20.0f), SCREEN_SCALE_FROM_BOTTOM(WASTEDBUSTED_Y), m_BigMessage[2]);
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

void CHud::DrawAfterFade()
{
	if (CTimer::GetIsUserPaused() || CReplay::IsPlayingBack())
		return;

	if (m_HelpMessage[0]) {
		if (!CMessages::WideStringCompare(m_HelpMessage, m_LastHelpMessage, HELP_MSG_LENGTH)) {
			switch (m_HelpMessageState) {
			case 0:
				m_HelpMessageFadeTimer = 0;
				m_HelpMessageState = 2;
				m_HelpMessageTimer = 0;
				CMessages::WideStringCopy(m_HelpMessageToPrint, m_HelpMessage, HELP_MSG_LENGTH);
				m_HelpMessageDisplayTime = CMessages::GetWideStringLength(m_HelpMessage) / 20.0f + 3.0f;

				if (TheCamera.m_ScreenReductionPercentage == 0.0f)
					DMAudio.PlayFrontEndSound(SOUND_HUD, 0);
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
			CMessages::WideStringCopy(m_LastHelpMessage, m_HelpMessage, HELP_MSG_LENGTH);
		}

		float fAlpha = 225.0f;

		if (m_HelpMessageState != 0) {
			switch (m_HelpMessageState) {
			case 1:
				fAlpha = 225.0f;
				m_HelpMessageFadeTimer = 600;
				if (m_HelpMessageTimer > m_HelpMessageDisplayTime * 1000.0f || m_HelpMessageQuick && m_HelpMessageTimer > 1500.0f) {
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
				fAlpha = m_HelpMessageFadeTimer / 1000.0f * 225.0f;
				break;
			case 3:
				m_HelpMessageFadeTimer -= 2 * CTimer::GetTimeStepInMilliseconds();
				if (m_HelpMessageFadeTimer < 0) {
					m_HelpMessageState = 0;
					m_HelpMessageFadeTimer = 0;
				}
				fAlpha = m_HelpMessageFadeTimer / 1000.0f * 225.0f;
				break;
			case 4:
				m_HelpMessageFadeTimer -= 2 * CTimer::GetTimeStepInMilliseconds();
				if (m_HelpMessageFadeTimer < 0) {
					m_HelpMessageState = 2;
					m_HelpMessageFadeTimer = 0;
					CMessages::WideStringCopy(m_HelpMessageToPrint, m_LastHelpMessage, HELP_MSG_LENGTH);
				}
				fAlpha = m_HelpMessageFadeTimer / 1000.0f * 225.0f;
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
#ifdef MORE_LANGUAGES
			else if (CFont::IsJapanese())
				CFont::SetScale(SCREEN_SCALE_X(0.52f) * 1.35f, SCREEN_SCALE_Y(1.1f) * 1.25f);
#endif
			else
				CFont::SetScale(SCREEN_SCALE_X(0.52f), SCREEN_SCALE_Y(1.1f));

			CFont::SetColor(CRGBA(175, 175, 175, 255));
			CFont::SetJustifyOff();			
#ifdef MORE_LANGUAGES
			if (CFont::IsJapanese())
				CFont::SetWrapx(SCREEN_SCALE_X(229.0f) + SCREEN_SCALE_X(26.0f) - SCREEN_SCALE_X_FIX(4.0f));
			else
#endif
				CFont::SetWrapx(SCREEN_SCALE_X(200.0f) + SCREEN_SCALE_X(26.0f) - SCREEN_SCALE_X_FIX(4.0f));
			CFont::SetFontStyle(FONT_LOCALE(FONT_BANK));
			CFont::SetBackgroundOn();
			CFont::SetBackGroundOnlyTextOff();
			CFont::SetBackgroundColor(CRGBA(0, 0, 0, fAlpha * 0.9f));
			CFont::SetColor(CRGBA(175, 175, 175, 255));
			CFont::PrintString(SCREEN_SCALE_X(26.0f), SCREEN_SCALE_Y(28.0f) + SCREEN_SCALE_Y_FIX((150.0f - PagerXOffset) * 0.6f), m_HelpMessageToPrint);
			CFont::SetAlphaFade(255.0f);
		}
	}

	for (int i = 0; i < ARRAY_SIZE(CTheScripts::IntroTextLines); i++) {
		intro_text_line &line = CTheScripts::IntroTextLines[i];
		if (line.m_Text[0] != '\0' && !line.m_bTextBeforeFade) {

			CFont::SetScale(SCREEN_SCALE_X_PC(line.m_fScaleX), SCREEN_SCALE_Y_PC(line.m_fScaleY)
#if !defined(PS2_HUD) || defined(FIX_BUGS)
				/ 2
#endif
				);
			CFont::SetColor(line.m_sColor);
			if (line.m_bJustify)
				CFont::SetJustifyOn();
			else
				CFont::SetJustifyOff();

			if (line.m_bRightJustify)
				CFont::SetRightJustifyOn();
			else
				CFont::SetRightJustifyOff();

			if (line.m_bCentered)
				CFont::SetCentreOn();
			else
				CFont::SetCentreOff();

			CFont::SetWrapx(SCALE_AND_CENTER_X_PC(line.m_fWrapX));
			CFont::SetCentreSize(SCREEN_SCALE_X_PC(line.m_fCenterSize));
			
			if (line.m_bBackground)
				CFont::SetBackgroundOn();
			else
				CFont::SetBackgroundOff();

			CFont::SetBackgroundColor(line.m_sBackgroundColor);
			if (line.m_bBackgroundOnly)
				CFont::SetBackGroundOnlyTextOn();
			else
				CFont::SetBackGroundOnlyTextOff();

			if (line.m_bTextProportional)
				CFont::SetPropOn();
			else
				CFont::SetPropOff();

			CFont::SetFontStyle(line.m_nFont);
#if defined(PS2_HUD) && !defined(FIX_BUGS)
			CFont::PrintString(line.m_fAtX, line.m_fAtY, line.m_Text);
#else
			CFont::PrintString(SCALE_AND_CENTER_X(DEFAULT_SCREEN_WIDTH - line.m_fAtX), SCREEN_SCALE_Y(DEFAULT_SCREEN_HEIGHT - line.m_fAtY), line.m_Text);
#endif
		}
	}
	for (int i = 0; i < ARRAY_SIZE(CTheScripts::IntroRectangles); i++) {
		intro_script_rectangle &rectangle = CTheScripts::IntroRectangles[i];
		if (rectangle.m_bIsUsed && !rectangle.m_bBeforeFade) {

			// Yeah, top and bottom changed place. R* vision
			if (rectangle.m_nTextureId >= 0) {
				CTheScripts::ScriptSprites[rectangle.m_nTextureId].Draw(CRect(rectangle.m_sRect.left, rectangle.m_sRect.bottom,
					rectangle.m_sRect.right, rectangle.m_sRect.top), rectangle.m_sColor);
			} else {
				CSprite2d::DrawRect(CRect(rectangle.m_sRect.left, rectangle.m_sRect.bottom,
					rectangle.m_sRect.right, rectangle.m_sRect.top), rectangle.m_sColor);
			}
		}
	}

	/*
		DrawBigMessage2
	*/
	// Oddjob
	if (m_BigMessage[3][0]) {
		CFont::SetJustifyOff();
		CFont::SetBackgroundOff();
		CFont::SetScale(SCREEN_SCALE_X_PC(1.2f), SCREEN_SCALE_Y_PC(1.5f));
		CFont::SetCentreOn();
		CFont::SetPropOn();
		CFont::SetCentreSize(SCREEN_SCALE_X(DEFAULT_SCREEN_WIDTH - 40));
		CFont::SetFontStyle(FONT_LOCALE(FONT_BANK));
		CFont::SetColor(CRGBA(0, 0, 0, 255));
		CFont::PrintString((SCREEN_WIDTH / 2) + SCREEN_SCALE_X_FIX(2.0f), (SCREEN_HEIGHT / 2) - SCREEN_SCALE_Y(BIGMESSAGE_Y) + SCREEN_SCALE_Y_FIX(2.0f), m_BigMessage[3]);
		CFont::SetColor(ODDJOB_COLOR);
		CFont::PrintString((SCREEN_WIDTH / 2), (SCREEN_HEIGHT / 2) - SCREEN_SCALE_Y(BIGMESSAGE_Y), m_BigMessage[3]);
	}

	if (!m_BigMessage[1][0] && m_BigMessage[4][0]) {
		CFont::SetJustifyOff();
		CFont::SetBackgroundOff();
		CFont::SetScale(SCREEN_SCALE_X_PC(1.2f), SCREEN_SCALE_Y_PC(1.5f));
		CFont::SetCentreOn();
		CFont::SetPropOn();
		CFont::SetCentreSize(SCREEN_SCALE_X(DEFAULT_SCREEN_WIDTH - 20));
		CFont::SetColor(CRGBA(0, 0, 0, 255));
		CFont::SetFontStyle(FONT_LOCALE(FONT_BANK));
		CFont::PrintString((SCREEN_WIDTH / 2) - SCREEN_SCALE_X_FIX(2.0f), (SCREEN_HEIGHT / 2) - SCREEN_SCALE_Y(BIGMESSAGE_Y) - SCREEN_SCALE_Y_FIX(2.0f), m_BigMessage[4]);
		CFont::SetColor(ODDJOB_COLOR);
		CFont::PrintString((SCREEN_WIDTH / 2), (SCREEN_HEIGHT / 2) - SCREEN_SCALE_Y(BIGMESSAGE_Y), m_BigMessage[4]);
	}

	// Oddjob result
	if (OddJob2OffTimer > 0)
		OddJob2OffTimer -= CTimer::GetTimeStepInMilliseconds();

	float fStep;
	if (m_BigMessage[5][0] && OddJob2OffTimer <= 0.0f) {
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
					fStep = Min(40.0f, OddJob2XOffset / 6.0f);
					OddJob2XOffset = OddJob2XOffset - fStep;
				}
				break;
			case 2:
				OddJob2Timer += CTimer::GetTimeStepInMilliseconds();
				if (OddJob2Timer > 1500) {
					OddJob2On = 3;
				}
				break;
			case 3:
				fStep = Max(30.0f, OddJob2XOffset / 5.0f);

				OddJob2XOffset = OddJob2XOffset - fStep;

				if (OddJob2XOffset < -380.0f) {
					OddJob2OffTimer = 5000.0f;
					OddJob2On = 0;
				}
				break;
			default:
				break;
		}

		if (!m_BigMessage[1][0]) {
			CFont::SetJustifyOff();
			CFont::SetBackgroundOff();
			CFont::SetScale(SCREEN_SCALE_X(1.0f), SCREEN_SCALE_Y(1.2f));
			CFont::SetCentreOn();
			CFont::SetPropOn();
			// Not bug, we just want these kind of texts to be wrapped at the center.
#ifdef ASPECT_RATIO_SCALE
			CFont::SetCentreSize(SCREEN_SCALE_X(DEFAULT_SCREEN_WIDTH - 20.0f));
#else
			CFont::SetCentreSize(SCREEN_SCALE_FROM_RIGHT(20.0f));
#endif
			CFont::SetColor(CRGBA(0, 0, 0, 255));
			CFont::SetFontStyle(FONT_LOCALE(FONT_BANK));

#ifdef BETA_SLIDING_TEXT
			CFont::PrintString(SCREEN_WIDTH / 2 + SCREEN_SCALE_X_PC(2.0f) - SCREEN_SCALE_X(OddJob2XOffset), SCREEN_HEIGHT / 2 - SCREEN_SCALE_Y(20.0f) + SCREEN_SCALE_Y_PC(2.0f), m_BigMessage[5]);
			CFont::SetColor(ODDJOB2_COLOR);
			CFont::PrintString(SCREEN_WIDTH / 2 - SCREEN_SCALE_X(OddJob2XOffset), SCREEN_HEIGHT / 2 - SCREEN_SCALE_Y(20.0f), m_BigMessage[5]);
#else
			CFont::PrintString(SCREEN_WIDTH / 2 + SCREEN_SCALE_X_PC(2.0f), SCREEN_HEIGHT / 2 - SCREEN_SCALE_Y(20.0f) + SCREEN_SCALE_Y_PC(2.0f), m_BigMessage[5]);
			CFont::SetColor(ODDJOB2_COLOR);
			CFont::PrintString(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - SCREEN_SCALE_Y(20.0f), m_BigMessage[5]);
#endif
		}
	}

	/*
		DrawMissionTitle
	*/
	if (m_BigMessage[1][0]) {
		if (BigMessageInUse[1] != 0.0f) {
			CFont::SetJustifyOff();
			CFont::SetBackgroundOff();

			if (CGame::frenchGame || FrontEndMenuManager.m_PrefsLanguage == CMenuManager::LANGUAGE_SPANISH)
				CFont::SetScale(SCREEN_SCALE_X_PC(0.884f), SCREEN_SCALE_Y_PC(1.36f));
			else
				CFont::SetScale(SCREEN_SCALE_X_PC(1.04f), SCREEN_SCALE_Y_PC(1.6f));

			CFont::SetPropOn();
#ifdef FIX_BUGS
			CFont::SetRightJustifyWrap(SCREEN_SCALE_FROM_RIGHT(DEFAULT_SCREEN_WIDTH + 500.0f));
#else
			CFont::SetRightJustifyWrap(-500.0f);
#endif
			CFont::SetRightJustifyOn();
			CFont::SetFontStyle(FONT_HEADING);
			
			if (BigMessageX[1] >= SCREEN_WIDTH - SCREEN_SCALE_X_FIX(20.0f))
			{
				BigMessageInUse[1] += CTimer::GetTimeStep();

				if (BigMessageInUse[1] >= 120.0f) {
					BigMessageInUse[1] = 120.0f;
					BigMessageAlpha[1] -= (CTimer::GetTimeStepInMilliseconds() * 0.3f);
				}
				if (BigMessageAlpha[1] <= 0) {
					m_BigMessage[1][0] = 0;
					BigMessageAlpha[1] = 0.0f;
				}
			} else {
				BigMessageX[1] += SCREEN_SCALE_X_FIX(CTimer::GetTimeStepInMilliseconds() * 0.3f);
				BigMessageAlpha[1] += (CTimer::GetTimeStepInMilliseconds() * 0.3f);

				if (BigMessageAlpha[1] > 255.0f)
					BigMessageAlpha[1] = 255.0f;
			}

			CFont::SetColor(CRGBA(40, 40, 40, BigMessageAlpha[1]));
#ifdef BETA_SLIDING_TEXT
			CFont::PrintString(SCREEN_SCALE_X(2.0f) + BigMessageX[1], SCREEN_SCALE_FROM_BOTTOM(120.0f) + SCREEN_SCALE_Y_PC(2.0f), m_BigMessage[1]);
			CFont::SetColor(CRGBA(MISSIONTITLE_COLOR.r, MISSIONTITLE_COLOR.g, MISSIONTITLE_COLOR.b, BigMessageAlpha[1]));
			CFont::PrintString(BigMessageX[1], SCREEN_SCALE_FROM_BOTTOM(120.0f), m_BigMessage[1]);
#else
			CFont::PrintString(SCREEN_SCALE_FROM_RIGHT(20.0f) + SCREEN_SCALE_X_FIX(2.0f), SCREEN_SCALE_FROM_BOTTOM(120.0f) + SCREEN_SCALE_Y_PC(2.0f), m_BigMessage[1]);
			CFont::SetColor(CRGBA(MISSIONTITLE_COLOR.r, MISSIONTITLE_COLOR.g, MISSIONTITLE_COLOR.b, BigMessageAlpha[1]));
			CFont::PrintString(SCREEN_SCALE_FROM_RIGHT(20.0f), SCREEN_SCALE_FROM_BOTTOM(120.0f), m_BigMessage[1]);
#endif
		}
		else {
			BigMessageAlpha[1] = 0.0f;
#ifdef FIX_BUGS
			BigMessageX[1] = SCREEN_SCALE_FROM_RIGHT(DEFAULT_SCREEN_WIDTH + 60.0f);
#else
			BigMessageX[1] = -60.0f;
#endif
			BigMessageInUse[1] = 1.0f;
		}
	}
	else {
		BigMessageInUse[1] = 0.0f;
	}
}

void CHud::SetMessage(wchar *message)
{
	int i = 0;
	for (i = 0; i < ARRAY_SIZE(m_Message); i++) {
		if (message[i] == 0)
			break;

		m_Message[i] = message[i];
	}
	m_Message[i] = 0;
}

void CHud::SetBigMessage(wchar *message, uint16 style)
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
#ifndef FIX_BUGS
	m_BigMessage[style][i] = 0;
#endif
}

void CHud::SetPagerMessage(wchar *message)
{
	int i = 0;
	for (i = 0; i < ARRAY_SIZE(m_PagerMessage); i++) {
		if (message[i] == 0)
			break;

		m_PagerMessage[i] = message[i];
	}
	m_PagerMessage[i] = 0;
}