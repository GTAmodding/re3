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
#include "TheScripts.h"
#include "TxdStore.h"
#include "User.h"
#include "World.h"

//WRAPPER void CHud::Draw(void) { EAXJMP(0x5052A0); }
WRAPPER void CHud::DrawAfterFade(void) { EAXJMP(0x509030); }
WRAPPER void CHud::ReInitialise(void) { EAXJMP(0x504CC0); }

wchar	*CHud::m_pHelpMessage = (wchar*)0x86B888;
wchar	*CHud::m_pLastHelpMessage = (wchar*)0x6E8F28;
int		&CHud::m_nHelpMessageState = *(int*)0x880E1C;
int		&CHud::m_nHelpMessageTimer = *(int*)0x880FA4;
int		&CHud::m_nHelpMessageFadeTimer = *(int*)0x8F6258;
wchar	*CHud::m_pHelpMessageToPrint = (wchar*)0x664480;
float	&CHud::m_fTextBoxNumLines = *(float*)0x8E2C28;
float	&CHud::m_fHelpMessageTime = *(float *)0x8E2C28;
bool	&CHud::m_bHelpMessageQuick = *(bool *)0x95CCF7;
int CHud::m_ZoneState = *(int*)0x8F29AC;
int CHud::m_ZoneFadeTimer;
int CHud::m_ZoneNameTimer = *(int*)0x8F1A50;
wchar* &CHud::m_pZoneName = *(wchar **)0x8E2C2C;
wchar* CHud::m_pLastZoneName = (wchar*)0x8F432C;
wchar* CHud::m_ZoneToPrint;
int CHud::m_VehicleState = *(int*)0x940560;
int CHud::m_VehicleFadeTimer;
int CHud::m_VehicleNameTimer = *(int*)0x8F2A14;
wchar* &CHud::m_pVehicleName = *(wchar **)0x942FB4;
wchar* CHud::m_pLastVehicleName = *(wchar **)0x8E2DD8;
wchar* CHud::m_pVehicleNameToPrint;
wchar* CHud::m_Message = (wchar*)0x72E318;
wchar* CHud::m_PagerMessage = (wchar*)0x878840;
bool &CHud::m_bRetuneInProgress = *(bool*)0x650B80;
bool &CHud::m_Wants_To_Draw_Hud = *(bool*)0x95CD89;
bool &CHud::m_Wants_To_Draw_3dMarkers = *(bool*)0x95CD62;
wchar(*CHud::m_BigMessage)[128] = (wchar(*)[128])0x664CE0;
float *CHud::BigMessageInUse = (float*)0x862140;
float *CHud::BigMessageAlpha = (float*)0x862108;
float *CHud::BigMessageX = (float*)0x773248;

float	&CHud::OddJob2OffTimer = *(float*)0x942FA0;
char	&CHud::CounterOnLastFrame = *(char*)0x95CD67;
float	&CHud::OddJob2XOffset = *(float*)0x8F1B5C;
__int16	&CHud::CounterFlashTimer = *(__int16*)0x95CC20;
__int16	&CHud::OddJob2Timer = *(__int16*)0x95CC52;
char	&CHud::TimerOnLastFrame = *(char*)0x95CDA7;
__int16	&CHud::OddJob2On = *(__int16*)0x95CC78;
__int16	&CHud::TimerFlashTimer = *(__int16*)0x95CC6C;
__int16	&CHud::PagerSoundPlayed = *(__int16*)0x95CC4A;
int &CHud::SpriteBrightness = *(int*)0x95CC54;
float	&CHud::PagerXOffset = *(float*)0x941590;
int CHud::m_ItemToFlash = *(int*)0x95CC82;
__int16	&CHud::PagerTimer = *(__int16*)0x95CC3A;
__int16	&CHud::PagerOn = *(__int16*)0x95CCA0;

CSprite2d *CHud::Sprites = (CSprite2d*)0x95CB9C;

const char* aModelsHud_txd = "MODELS/HUD.TXD";
char **HudNames = (char**)0x5FDA70;
char *aSitesniper = (char*)0x5FDA38;
char *aSiterocket = (char*)0x5FDA64;

RwTexture* gpSniperSightTex = (RwTexture*)0x8F5834;
RwTexture* gpRocketSightTex = (RwTexture*)0x8E2C20;

float f1_div_640_2 = 1.0f / 640.0f;
float f1_div_448_2 = 1.0f / 448.0f;

void CHud::Initialise() {
	ReInitialise();

	int HudTXD = CTxdStore::AddTxdSlot("hud");
	CTxdStore::LoadTxd(HudTXD, aModelsHud_txd);
	CTxdStore::AddRef(HudTXD);
	CTxdStore::PopCurrentTxd();
	CTxdStore::SetCurrentTxd(HudTXD);

	for (int i = 0; i < 23; i++) {
		Sprites[i].SetTexture(HudNames[2 * i], (HudNames + 4)[2 * i]);
	}

	gpSniperSightTex = RwTextureRead(aSitesniper, 0);
	gpRocketSightTex = RwTextureRead(aSiterocket, 0);

	CTxdStore::PopCurrentTxd();
}

void CHud::Draw() {
	RwRenderStateSet(rwRENDERSTATEFOGENABLE, reinterpret_cast<void*>(0));
	RwRenderStateSet(rwRENDERSTATESRCBLEND, reinterpret_cast<void*>(5));
	RwRenderStateSet(rwRENDERSTATEDESTBLEND, reinterpret_cast<void*>(6));
	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, reinterpret_cast<void*>(1));
	RwRenderStateSet(rwRENDERSTATETEXTUREADDRESS, reinterpret_cast<void*>(3));
	RwRenderStateSet(rwRENDERSTATETEXTURERASTER, reinterpret_cast<void*>(0));
	RwRenderStateSet(rwRENDERSTATESHADEMODE, reinterpret_cast<void*>(1));
	RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void*)rwFILTERLINEAR);

	if (CReplay::Mode != 1) {
		if (m_Wants_To_Draw_Hud && !TheCamera.m_WideScreenOn) {
			bool Mode_RunAround = 0;
			bool Mode_FirstPerson = 0;

			char WeaponType = CWorld::Players[CWorld::PlayerInFocus].m_pPed->m_weapons[CWorld::Players[CWorld::PlayerInFocus].m_pPed->m_currentWeapon].m_eWeaponType;
			char Mode = TheCamera.Cams[TheCamera.ActiveCam].Mode;

			if (Mode == CCam::CamMode::MODE_SNIPER || Mode == CCam::CamMode::MODE_ROCKET || Mode == CCam::CamMode::MODE_M16FIRSTPERSON_34 || Mode == CCam::CamMode::MODE_EDITOR)
				Mode_FirstPerson = 1;
			if (Mode == CCam::CamMode::MODE_FIRSTPERSONPEDONPC_41 || Mode == CCam::CamMode::MODE_SNIPER_RUN_AROUND)
				Mode_RunAround = 1;

			if (TheCamera.Cams->Using3rdPersonMouseCam() && (!CPad::GetPad(0)->GetLookBehindForPed() || TheCamera.m_bPlayerIsInGarage) || Mode == 40) {
				if (CWorld::Players[CWorld::PlayerInFocus].m_pPed) {
					char State = CWorld::Players[CWorld::PlayerInFocus].m_pPed->m_nPedState;
					if (State != PED_ENTER_CAR && State != PED_CARJACK) {
						if ((WeaponType >= WEAPONTYPE_COLT45 && WeaponType <= WEAPONTYPE_M16) || WeaponType == WEAPONTYPE_FLAMETHROWER)
							Mode_RunAround = 1;
					}
				}
			}

			if (Mode_FirstPerson || Mode_RunAround) {
				RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void *)2);

				int SpriteBrightLikeADiamond = CHud::SpriteBrightness + 1;
				if (SpriteBrightLikeADiamond > 30)
					SpriteBrightLikeADiamond = 30;

				CHud::SpriteBrightness = SpriteBrightLikeADiamond;

				RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, 0);

				float fStep = sin((CTimer::GetTimeInMilliseconds() & 0x3FF) * 0.0061328127);
				float fMultBright = CHud::SpriteBrightness * 0.033333335f * (0.25f * fStep + 0.75f);

				if (Mode_RunAround && TheCamera.Cams->Using3rdPersonMouseCam()) {
					float f3rdX = RsGlobal.maximumWidth * TheCamera.m_f3rdPersonCHairMultX;
					float f3rdY = RsGlobal.maximumHeight * TheCamera.m_f3rdPersonCHairMultY;

					if (CWorld::Players[CWorld::PlayerInFocus].m_pPed && WeaponType == WEAPONTYPE_M16) {
						CRGBA color = { 0xFFu, 0xFFu, 0xFFu, 0xFFu };

						float left = f3rdX - RsGlobal.maximumWidth * f1_div_640_2 * 32.0f * 0.60000002f;
						float top = f3rdY - RsGlobal.maximumHeight * f1_div_448_2 * 32.0f  * 0.60000002f;
						float right = f3rdX + RsGlobal.maximumWidth * f1_div_640_2 * 32.0f * 0.60000002f;
						float bottom = f3rdY + RsGlobal.maximumHeight * f1_div_448_2 * 32.0f  * 0.60000002f;

						CHud::Sprites[21].Draw(CRect(left, top, right, bottom),
							color,
							0,
							0,
							1.0f,
							0,
							0,
							1.0f,
							1.0f,
							1.0f);
					}
					else {
						CRGBA color = { 0xFFu, 0xFFu, 0xFFu, 0xFFu };
						float left = f3rdX - RsGlobal.maximumWidth * f1_div_640_2 * 32.0f * 0.40000001f;
						float top = f3rdY - RsGlobal.maximumHeight * f1_div_448_2 * 32.0f  * 0.40000001f;
						float right = f3rdX + RsGlobal.maximumWidth * f1_div_640_2 * 32.0f * 0.40000001f;
						float bottom = f3rdY + RsGlobal.maximumHeight * f1_div_448_2 * 32.0f  * 0.40000001f;

						CHud::Sprites[21].Draw(CRect(left, top, right, bottom),
							color,
							0,
							0,
							1.0f,
							0,
							0,
							1.0f,
							1.0f,
							1.0f);
					}
				}
				else {
					if (Mode != CCam::CamMode::MODE_M16FIRSTPERSON_34 && Mode != CCam::CamMode::MODE_FIRSTPERSONPEDONPC_41 && Mode != CCam::CamMode::MODE_EDITOR) {
						if (Mode == CCam::CamMode::MODE_ROCKET_RUN_AROUND) {
							CRGBA color = { 0xFFu, 0xFFu, 0xFFu, 0xFFu };
							float left = (RsGlobal.maximumWidth / 2) - RsGlobal.maximumWidth * f1_div_640_2 * 32.0f * 0.69999999;
							float top = (RsGlobal.maximumHeight / 2) - RsGlobal.maximumHeight * f1_div_448_2 * 32.0f * 0.69999999;
							float right = (RsGlobal.maximumWidth / 2) + RsGlobal.maximumWidth * f1_div_640_2 * 32.0f * 0.69999999;
							float bottom = (RsGlobal.maximumHeight / 2) + RsGlobal.maximumHeight * f1_div_448_2 * 32.0f * 0.69999999;

							CHud::Sprites[21].Draw(CRect(left, top, right, bottom),
								color,
								0,
								0,
								1.0f,
								0,
								0,
								1.0f,
								1.0f,
								1.0f);
						}
						else if (Mode != CCam::CamMode::MODE_ROCKET && Mode != CCam::CamMode::MODE_SNIPER_RUN_AROUND) {
							CRGBA color = { 0xFFu, 0xFFu, 0xFFu, 0xFFu };
							float left = (RsGlobal.maximumWidth / 2) - RsGlobal.maximumWidth * f1_div_640_2 * 210.0f;
							float top = (RsGlobal.maximumHeight / 2) - RsGlobal.maximumHeight * f1_div_448_2 * 210.0f;
							float right = RsGlobal.maximumWidth / 2;
							float bottom = RsGlobal.maximumHeight / 2;

							CHud::Sprites[20].Draw(CRect(left, top, right, bottom),
								color,
								0,
								0,
								1.0f,
								0,
								0,
								1.0f,
								1.0f,
								1.0f);

							color = { 0xFFu, 0xFFu, 0xFFu, 0xFFu };
							right = (RsGlobal.maximumWidth / 2);
							top = (RsGlobal.maximumHeight / 2) - RsGlobal.maximumHeight * f1_div_448_2 * 210.0f;
							left = RsGlobal.maximumWidth * f1_div_640_2 * 210.0f + (RsGlobal.maximumWidth / 2);
							bottom = RsGlobal.maximumHeight / 2;

							CHud::Sprites[20].Draw(CRect(left, top, right, bottom),
								color,
								0,
								0,
								1.0f,
								0,
								0,
								1.0f,
								1.0f,
								1.0f);

							color = { 0xFFu, 0xFFu, 0xFFu, 0xFFu };
							left = (RsGlobal.maximumWidth / 2) - RsGlobal.maximumWidth * f1_div_640_2 * 210.0f;
							bottom = (RsGlobal.maximumHeight / 2);
							right = (RsGlobal.maximumWidth / 2);
							top = RsGlobal.maximumHeight * f1_div_448_2 * 210.0f + (RsGlobal.maximumHeight / 2);

							CHud::Sprites[20].Draw(CRect(left, top, right, bottom),
								color,
								0,
								0,
								1.0f,
								0,
								0,
								1.0f,
								1.0f,
								1.0f);

							color = { 0xFFu, 0xFFu, 0xFFu, 0xFFu };
							right = (RsGlobal.maximumWidth / 2);
							bottom = (RsGlobal.maximumHeight / 2);
							left = RsGlobal.maximumWidth * f1_div_640_2 * 210.0f + (RsGlobal.maximumWidth / 2);
							top = RsGlobal.maximumHeight * f1_div_448_2 * 210.0f + (RsGlobal.maximumHeight / 2);

							CHud::Sprites[20].Draw(CRect(left, top, right, bottom),
								color,
								0,
								0,
								1.0f,
								0,
								0,
								1.0f,
								1.0f,
								1.0f);
						}
						else {
							RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void *)1);
							RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)2);
							RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)2);
							RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, 0);
							RwRenderStateSet(rwRENDERSTATEZTESTENABLE, 0);
							RwRenderStateSet(rwRENDERSTATETEXTURERASTER, gpRocketSightTex->raster);

							float x = (RsGlobal.maximumWidth / 2);
							float y = (RsGlobal.maximumHeight / 2);
							float w = RsGlobal.maximumWidth * f1_div_640_2 * 40.0f;
							float h = RsGlobal.maximumHeight * f1_div_448_2 * 40.0f;

							CSprite::RenderOneXLUSprite(
								x,
								y,
								1.0f,
								w,
								h,
								(signed int)(100.0f * fMultBright),
								(signed int)(200.0f * fMultBright),
								(signed int)(100.0f * fMultBright),
								255,
								1.0f,
								255);
						}
					}
					else {
						CRGBA color = { 0xFFu, 0xFFu, 0xFFu, 0xFFu };
						float left = (RsGlobal.maximumWidth / 2) - RsGlobal.maximumWidth * f1_div_640_2 * 32.0f;
						float top = (RsGlobal.maximumHeight / 2) - RsGlobal.maximumHeight * f1_div_448_2 * 32.0f;
						float right = (RsGlobal.maximumWidth / 2) + RsGlobal.maximumWidth * f1_div_640_2 * 32.0f;
						float bottom = (RsGlobal.maximumHeight / 2) + RsGlobal.maximumHeight * f1_div_448_2 * 32.0f;

						CHud::Sprites[21].Draw(CRect(left, top, right, bottom),
							color,
							0,
							0,
							1.0f,
							0,
							0,
							1.0f,
							1.0f,
							1.0f);
					}
				}
				RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void *)2);
				RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)5);
				RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)6);
			}
			else {
				CHud::SpriteBrightness = 0;
			}

			wchar sPrint[16];
			wchar sPrintIcon[16];
			char sTemp[16];

			sprintf(sTemp, "$%08d", CWorld::Players[CWorld::PlayerInFocus].m_nVisibleMoney);
			AsciiToUnicode(sTemp, sPrint);

			CFont::SetPropOff();
			CFont::SetBackgroundOff();
			CFont::SetScale(RsGlobal.maximumWidth * f1_div_640_2 * 0.80000001f, RsGlobal.maximumHeight * f1_div_448_2 * 1.35f);
			CFont::SetCentreOff();
			CFont::SetRightJustifyOn();
			CFont::SetRightJustifyWrap(0.0f);
			CFont::SetBackGroundOnlyTextOff();
			CFont::SetFontStyle(2);
			CFont::SetPropOff();
			CFont::SetColor(CRGBA(0, 0, 0, 0xFFu));

			CFont::PrintString(RsGlobal.maximumWidth - RsGlobal.maximumWidth * f1_div_640_2 * 110.0f + 2.0f, RsGlobal.maximumHeight * f1_div_448_2 * 43.0f + 2.0f, sPrint);

			CFont::SetColor(CRGBA(0x59u, 0x73u, 0x96u, 255));
			CFont::PrintString(RsGlobal.maximumWidth - RsGlobal.maximumWidth * f1_div_640_2 * 110.0f, RsGlobal.maximumHeight * f1_div_448_2 * 43.0f, sPrint);

			sprintf(sTemp, "%02d:%02d", CClock::GetHours(), CClock::GetMinutes());
			AsciiToUnicode(sTemp, sPrint);

			CFont::SetColor(CRGBA(0, 0, 0, 0xFFu));

			CFont::PrintString(RsGlobal.maximumWidth - RsGlobal.maximumWidth * f1_div_640_2 * 111.0f + 2.0f, RsGlobal.maximumHeight * f1_div_448_2 * 22.0f + 2.0f, sPrint);

			CFont::SetColor(CRGBA(0xC2u, 0xA5u, 0x78u, 0xFFu));
			CFont::PrintString(RsGlobal.maximumWidth - RsGlobal.maximumWidth * f1_div_640_2 * 111.0f, RsGlobal.maximumHeight * f1_div_448_2 * 22.0f, sPrint);

			int32 AmmoInClip = CWorld::Players[CWorld::PlayerInFocus].m_pPed->m_weapons[CWorld::Players[CWorld::PlayerInFocus].m_pPed->m_currentWeapon].m_nAmmoInClip;
			int32 TotalAmmo = CWorld::Players[CWorld::PlayerInFocus].m_pPed->m_weapons[CWorld::Players[CWorld::PlayerInFocus].m_pPed->m_currentWeapon].m_nAmmoTotal;

			if (AmmoInClip <= 1 || AmmoInClip >= 1000) {
				sprintf(sTemp, "%d", TotalAmmo);
			}
			else {
				if (WeaponType == WEAPONTYPE_FLAMETHROWER) {
					int tot_min_clip_div_10 = (TotalAmmo - AmmoInClip) / 10;
					if (tot_min_clip_div_10 > 9999)
						tot_min_clip_div_10 = 9999;

					sprintf(sTemp, "%d-%d", tot_min_clip_div_10, AmmoInClip / 10);
				}
				else {
					if (AmmoInClip > 9999)
						AmmoInClip = 9999;
					sprintf(sTemp, "%d-%d", (TotalAmmo - AmmoInClip), AmmoInClip);
				}
			}

			AsciiToUnicode(sTemp, sPrint);

			float left = RsGlobal.width - 99.0f * RsGlobal.width * f1_div_640_2;
			float top = 27.0f * RsGlobal.maximumHeight * f1_div_448_2;
			float right = 64.0f * RsGlobal.width * f1_div_640_2 + RsGlobal.width - 99.0f * RsGlobal.width * f1_div_640_2;
			float bottom = 64.0f * RsGlobal.maximumHeight * f1_div_448_2 + 27.0f * RsGlobal.maximumHeight * f1_div_448_2;

			CHud::Sprites[WeaponType].Draw(
				CRect(left, top, right, bottom),
				CRGBA(0xFFu, 0xFFu, 0xFFu, 0xFFu),
				0.015f,
				0.015f,
				1.0f,
				0.0f,
				0.015f,
				1.0f,
				1.0f,
				1.0f);

			CFont::SetBackgroundOff();
			CFont::SetScale(RsGlobal.width * f1_div_640_2 * 0.40000001f, RsGlobal.height * f1_div_448_2 * 0.60000002f);
			CFont::SetJustifyOff();
			CFont::SetCentreOn();
			CFont::SetCentreSize(RsGlobal.width * f1_div_640_2 * 640.0f);
			CFont::SetPropOn();
			CFont::SetFontStyle(0);

			if (!CDarkel::FrenzyOnGoing()) {
				if (WeaponType) {
					if (WeaponType != 1) {
						CFont::SetColor(CRGBA(0, 0, 0, 255));
						CFont::PrintString(RsGlobal.width - RsGlobal.width * f1_div_640_2 * 66.0f, RsGlobal.height * f1_div_448_2 * 73.0f, sPrint);
					}
				}
			}

			CFont::SetBackgroundOff();
			CFont::SetScale(RsGlobal.width * f1_div_640_2 * 0.80000001f, RsGlobal.height * f1_div_448_2 * 1.35f);
			CFont::SetJustifyOff();
			CFont::SetCentreOff();
			CFont::SetRightJustifyWrap(0.0f);
			CFont::SetRightJustifyOn();
			CFont::SetPropOff();
			CFont::SetFontStyle(2);

			if (CHud::m_ItemToFlash == 4 && CTimer::GetFrameCounter() & 8
				|| CHud::m_ItemToFlash != 4
				|| CWorld::Players[CWorld::PlayerInFocus].m_pPed->m_fHealth < 10
				&& CTimer::GetFrameCounter() & 8) {
				if (CWorld::Players[CWorld::PlayerInFocus].m_pPed->m_fHealth >= 10
					|| CWorld::Players[CWorld::PlayerInFocus].m_pPed->m_fHealth < 10 && CTimer::GetFrameCounter() & 8) {
					
					AsciiToUnicode("[", sPrintIcon);
					sprintf(sTemp, "%03d", (signed __int16)(signed int)CWorld::Players[CWorld::PlayerInFocus].m_pPed->m_fHealth);
					AsciiToUnicode(sTemp, sPrint);

					CFont::SetColor(CRGBA(0, 0, 0, 0xFFu));
					CFont::PrintString(RsGlobal.width - RsGlobal.width * f1_div_640_2 * 110.0f + 2.0f, RsGlobal.height * f1_div_448_2 * 65.0f + 2.0f, sPrint);

					if (!CWorld::Players[CWorld::PlayerInFocus].m_nTimeLastHealthLoss || CTimer::GetTimeInMilliseconds() > CWorld::Players[CWorld::PlayerInFocus].m_nTimeLastHealthLoss + 2000 || CTimer::GetFrameCounter() & 4) {
						float y = RsGlobal.height * f1_div_448_2 * 65.0f + 2.0f;
						float x = RsGlobal.width
							- 110.0f * (RsGlobal.width * f1_div_640_2)
							+ 2.0f
							- (signed int)(54.0f * (RsGlobal.width * f1_div_640_2));
						CFont::PrintString(x, y, sPrintIcon);
					}
					CFont::SetColor(CRGBA(0xBAu, 0x65u, 0x32u, 0xFFu));

					CFont::PrintString(RsGlobal.width - RsGlobal.width * f1_div_640_2 * 110.0f, RsGlobal.height * f1_div_448_2 * 65.0f, sPrint);

					if (!CWorld::Players[CWorld::PlayerInFocus].m_nTimeLastHealthLoss || CTimer::GetTimeInMilliseconds() > CWorld::Players[CWorld::PlayerInFocus].m_nTimeLastHealthLoss + 2000 || CTimer::GetFrameCounter() & 4) {
						float y = RsGlobal.height * f1_div_448_2 * 65.0f;
						float x = RsGlobal.width
							- 108.0f * (RsGlobal.width * f1_div_640_2)
							- (signed int)(56.0f * (RsGlobal.width * f1_div_640_2));
						CFont::PrintString(x, y, sPrintIcon);
					}
				}
			}
			if (CHud::m_ItemToFlash == 3 && CTimer::GetFrameCounter() & 8 || CHud::m_ItemToFlash != 3) {
				CFont::SetScale(RsGlobal.width * f1_div_640_2 * 0.80000001, RsGlobal.height * f1_div_448_2 * 1.35f);
				if (CWorld::Players[CWorld::PlayerInFocus].m_pPed->m_fArmour > 1.0f) {
					AsciiToUnicode("[", sPrintIcon);
					sprintf(sTemp, "%03d", (signed __int16)(signed int)CWorld::Players[CWorld::PlayerInFocus].m_pPed->m_fArmour);
					AsciiToUnicode(sTemp, sPrint);

					CFont::SetColor(CRGBA(0, 0, 0, 0xFFu));
					CFont::PrintString(RsGlobal.width - RsGlobal.width * f1_div_640_2 * 182.0f + 2.0f, RsGlobal.height * f1_div_448_2 * 65.0f + 2.0f, sPrint);

					if (!CWorld::Players[CWorld::PlayerInFocus].m_nTimeLastArmourLoss || CTimer::GetTimeInMilliseconds() > CWorld::Players[CWorld::PlayerInFocus].m_nTimeLastArmourLoss + 2000 || CTimer::GetFrameCounter() & 4) {
						float y = RsGlobal.height * f1_div_448_2 * 65.0f + 2.0f;
						float x = RsGlobal.width
							- 182.0f * (RsGlobal.width * f1_div_640_2)
							+ 2.0f
							- (signed int)(52.0f * (RsGlobal.width * f1_div_640_2));
						CFont::PrintString(x, y, sPrintIcon);
					}

					CFont::SetColor(CRGBA(0x7Cu, 0x8Cu, 0x5Fu, 0xFFu));
					CFont::PrintString(RsGlobal.width - RsGlobal.width * f1_div_640_2 * 182.0f, RsGlobal.height * f1_div_448_2 * 65.0f, sPrint);

					if (!CWorld::Players[CWorld::PlayerInFocus].m_nTimeLastArmourLoss || CTimer::GetTimeInMilliseconds() > CWorld::Players[CWorld::PlayerInFocus].m_nTimeLastArmourLoss + 2000 || CTimer::GetFrameCounter() & 1) {
						float y = RsGlobal.height * f1_div_448_2 * 65.0f;
						float x = RsGlobal.width
							- 182.0f * (RsGlobal.width * f1_div_640_2)
							- (signed int)(52.0f * (RsGlobal.width * f1_div_640_2));
						CFont::PrintString(x, y, sPrintIcon);
					}
				}
			}

			CFont::SetBackgroundOff();
			CFont::SetScale(RsGlobal.width * f1_div_640_2 * 0.80000001f, RsGlobal.height * f1_div_448_2 * 1.35f);
			CFont::SetJustifyOff();
			CFont::SetCentreOff();
			CFont::SetRightJustifyOff();
			CFont::SetPropOn();
			CFont::SetFontStyle(2);

			AsciiToUnicode("]", sPrintIcon);

			uint32 counter = 0;
			float x = RsGlobal.width - RsGlobal.width * f1_div_640_2 * 60.0f;
			do {
				CFont::SetColor(CRGBA(0, 0, 0, 0xFFu));
				CFont::PrintString(2.0f + x, RsGlobal.height * f1_div_448_2 * 87.0f + 2.0f, sPrintIcon);
				if (CWorld::Players[CWorld::PlayerInFocus].m_pPed->m_pWanted->m_nWantedLevel > counter
					&& (CTimer::GetTimeInMilliseconds() > CWorld::Players[CWorld::PlayerInFocus].m_pPed->m_pWanted->m_nLastWantedLevelChange
						+ 2000 || CTimer::GetFrameCounter() & 4)) {

					CFont::SetColor(CRGBA(0xC1u, 0xA4u, 0x78u, 0xFFu));
					CFont::PrintString(x, RsGlobal.height * f1_div_448_2 * 87.0f, sPrintIcon);
				}
				++counter;
				x = x - RsGlobal.width * f1_div_640_2 * 23.0f;
			} while (counter < 6);
		}

		if (CHud::m_pZoneName) {
			float fZoneAlpha = 0.0f;

			if (CHud::m_pZoneName != CHud::m_pLastZoneName) {
				switch (CHud::m_ZoneState) {
				case 0:
					CHud::m_ZoneState = 2;
					CHud::m_ZoneToPrint = CHud::m_pZoneName;
					CHud::m_ZoneNameTimer = 0;
					CHud::m_ZoneFadeTimer = 0;
					break;
				case 1:
				case 2:
				case 3:
				case 4:
					CHud::m_ZoneNameTimer = 0;
					CHud::m_ZoneState = 4;
					break;
				default:
					break;
				}
				CHud::m_pLastZoneName = CHud::m_pZoneName;
			}

			if (CHud::m_ZoneState) {
				switch (CHud::m_ZoneState) {
				case 1:
					if (CHud::m_ZoneNameTimer > 10000) {
						CHud::m_ZoneFadeTimer = 1000;
						CHud::m_ZoneState = 3;
					}
					fZoneAlpha = 255.0f;
					break;
				case 2:
					CHud::m_ZoneFadeTimer += (CTimer::GetTimeStep() * 0.02f * 1000.0f);
					if (CHud::m_ZoneFadeTimer > 1000) {
						CHud::m_ZoneState = 1;
						CHud::m_ZoneFadeTimer = 1000;
					}
					fZoneAlpha = CHud::m_ZoneFadeTimer * 0.001f * 255.0f;
					break;
				case 3:
					CHud::m_ZoneFadeTimer += (CTimer::GetTimeStep() * 0.02f * -1000.0f);
					if (CHud::m_ZoneFadeTimer < 0) {
						CHud::m_ZoneState = 0;
						CHud::m_ZoneFadeTimer = 0;
					}
					fZoneAlpha = CHud::m_ZoneFadeTimer * 0.001f * 255.0f;
					break;
				case 4:
					CHud::m_ZoneFadeTimer += (CTimer::GetTimeStep() * 0.02f * -1000.0f);
					if (CHud::m_ZoneFadeTimer < 0) {
						CHud::m_ZoneFadeTimer = 0;
						CHud::m_ZoneToPrint = CHud::m_pLastZoneName;
						CHud::m_ZoneNameTimer = 0;
						CHud::m_ZoneState = 2;
					}
					fZoneAlpha = CHud::m_ZoneFadeTimer * 0.001f * 255.0f;
					break;
				default:
					break;

				}
				if (!CHud::m_Message[0]) {
					CHud::m_ZoneNameTimer += (CTimer::GetTimeStep() * 0.02f * 1000.0f);
					CFont::SetJustifyOff();
					CFont::SetPropOn();
					CFont::SetBackgroundOff();

					float w = RsGlobal.width * f1_div_640_2 * 1.2f;
					float h = RsGlobal.height * f1_div_448_2 * 1.2f;
					if (CMenuManager::m_PrefsLanguage == 4) {
						w = RsGlobal.width * f1_div_640_2 * 1.2f * 0.80000001;
						h = RsGlobal.height * f1_div_448_2 * 1.2f;
					}

					CFont::SetScale(w, h);
					CFont::SetRightJustifyOn();
					CFont::SetRightJustifyWrap(0.0f);
					CFont::SetBackGroundOnlyTextOff();
					CFont::SetFontStyle(0);
					CFont::SetColor(CRGBA(0, 0, 0, (signed int)fZoneAlpha));
					CFont::PrintString(RsGlobal.width - RsGlobal.width * f1_div_640_2 * 32.0f + 1.0f, RsGlobal.height - RsGlobal.height * f1_div_448_2 * 30.0f + 1.0f, CHud::m_ZoneToPrint);

					CFont::SetColor(CRGBA(0x98u, 0x9Au, 0x52u, (signed int)fZoneAlpha));
					CFont::PrintString(RsGlobal.width - RsGlobal.width * f1_div_640_2 * 32.0f, RsGlobal.height - RsGlobal.height * f1_div_448_2 * 30.0f, CHud::m_ZoneToPrint);
				}
			}
		}
		else {
			CHud::m_pLastZoneName = 0;
			CHud::m_ZoneState = 0;
			CHud::m_ZoneFadeTimer = 0;
			CHud::m_ZoneNameTimer = 0;
		}

		if (CHud::m_pVehicleName) {
			float fVehicleAlpha = 0.0f;

			if (CHud::m_pVehicleName != CHud::m_pLastVehicleName) {
				switch (CHud::m_VehicleState) {
				case 0:
					CHud::m_VehicleState = 2;
					CHud::m_pVehicleNameToPrint = CHud::m_pVehicleName;
					CHud::m_VehicleNameTimer = 0;
					CHud::m_VehicleFadeTimer = 0;
					break;
				case 1:
				case 2:
				case 3:
				case 4:
					CHud::m_VehicleNameTimer = 0;
					CHud::m_VehicleState = 4;
					break;
				default:
					break;
				}
				CHud::m_pLastVehicleName = CHud::m_pVehicleName;
			}

			if (CHud::m_VehicleState) {
				switch (CHud::m_VehicleState) {
				case 1:
					if (CHud::m_VehicleNameTimer > 10000) {
						CHud::m_VehicleFadeTimer = 1000;
						CHud::m_VehicleState = 3;
					}
					fVehicleAlpha = 255.0f;
					break;
				case 2:
					CHud::m_VehicleFadeTimer += (CTimer::GetTimeStep() * 0.02f * 1000.0f);
					if (CHud::m_VehicleFadeTimer > 1000) {
						CHud::m_VehicleState = 1;
						CHud::m_VehicleFadeTimer = 1000;
					}
					fVehicleAlpha = CHud::m_VehicleFadeTimer * 0.001f * 255.0f;
					break;
				case 3:
					CHud::m_VehicleFadeTimer += (CTimer::GetTimeStep() * 0.02f * -1000.0f);
					if (CHud::m_VehicleFadeTimer < 0) {
						CHud::m_VehicleState = 0;
						CHud::m_VehicleFadeTimer = 0;
					}
					fVehicleAlpha = CHud::m_VehicleFadeTimer * 0.001f * 255.0f;
					break;
				case 4:
					CHud::m_VehicleFadeTimer += (CTimer::GetTimeStep() * 0.02f * -1000.0f);
					if (CHud::m_VehicleFadeTimer < 0) {
						CHud::m_VehicleFadeTimer = 0;
						CHud::m_pVehicleNameToPrint = CHud::m_pLastVehicleName;
						CHud::m_VehicleNameTimer = 0;
						CHud::m_VehicleState = 2;
					}
					fVehicleAlpha = CHud::m_VehicleFadeTimer * 0.001f * 255.0f;
					break;
				default:
					break;
				}

				if (!CHud::m_Message[0]) {
					CHud::m_VehicleNameTimer += (CTimer::GetTimeStep() * 0.02f * 1000.0f);
					CFont::SetJustifyOff();
					CFont::SetPropOn();
					CFont::SetBackgroundOff();

					float w = RsGlobal.width * f1_div_640_2 * 1.2f * 0.85000002f;
					float h = RsGlobal.height * f1_div_448_2 * 1.2f;

					if (CMenuManager::m_PrefsLanguage != 3 && CMenuManager::m_PrefsLanguage != 4) {			
						w = RsGlobal.width * f1_div_640_2 * 1.2f;
						h = RsGlobal.height * f1_div_448_2 * 1.2f;
					}

					CFont::SetScale(w, h);
					CFont::SetRightJustifyOn();
					CFont::SetRightJustifyWrap(0.0f);
					CFont::SetBackGroundOnlyTextOff();
					CFont::SetFontStyle(0);
					CFont::SetColor(CRGBA(0, 0, 0, (signed int)fVehicleAlpha));
					CFont::PrintString(RsGlobal.width - RsGlobal.width * f1_div_640_2 * 32.0f + 1.0f, RsGlobal.height - RsGlobal.height * f1_div_448_2 * 55.0f + 1.0f, CHud::m_pVehicleNameToPrint);

					CFont::SetColor(CRGBA(0xC2u, 0xA5u, 0x78u, (signed int)fVehicleAlpha));
					CFont::PrintString(RsGlobal.width - RsGlobal.width * f1_div_640_2 * 32.0f, RsGlobal.height - RsGlobal.height * f1_div_448_2 * 55.0f, CHud::m_pVehicleNameToPrint);
				}
			}
		}
		else {
			CHud::m_pLastVehicleName = 0;
			CHud::m_VehicleState = 0;
			CHud::m_VehicleFadeTimer = 0;
			CHud::m_VehicleNameTimer = 0;
		}

		wchar sTimer;
		if (!CUserDisplay::OnscnTimer.m_sEntries[0].m_bTimerProcessed)
			TimerOnLastFrame = 0;
		if (!CUserDisplay::OnscnTimer.m_sEntries[0].m_bCounterProcessed)
			CounterOnLastFrame = 0;
		if (CUserDisplay::OnscnTimer.m_bProcessed == 1) {
			if (CUserDisplay::OnscnTimer.m_sEntries[0].m_bTimerProcessed == 1) {
				if (!TimerOnLastFrame)
					TimerFlashTimer = 1;
				TimerOnLastFrame = 1;
				if (TimerFlashTimer) {
					if (++TimerFlashTimer > 50)
						TimerFlashTimer = 0;
				}
				if (CTimer::GetFrameCounter() & 4 || !TimerFlashTimer) {
					AsciiToUnicode(CUserDisplay::OnscnTimer.m_sEntries[0].m_bCounterBuffer, &sTimer);
					CFont::SetPropOn();
					CFont::SetBackgroundOff();
					CFont::SetScale(RsGlobal.width * f1_div_640_2 * 0.80000001f, RsGlobal.height * f1_div_448_2 * 1.35f);
					CFont::SetRightJustifyOn();
					CFont::SetRightJustifyWrap(0.0f);
					CFont::SetFontStyle(2);
					CFont::SetPropOff();
					CFont::SetBackGroundOnlyTextOn();
					CFont::SetColor(CRGBA(0, 0, 0, 0xFFu));
					CFont::PrintString(RsGlobal.width - RsGlobal.width * f1_div_640_2 * 27.0f + 2.0f, RsGlobal.height * f1_div_448_2 * 110.0f + 2.0f, &sTimer);

					CFont::SetScale(RsGlobal.width * f1_div_640_2 * 0.80000001f, RsGlobal.height * f1_div_448_2 * 1.35f);
					CFont::SetColor(CRGBA(0xBAu, 0x65u, 0x32u, 0xFFu));
					CFont::PrintString(RsGlobal.width - RsGlobal.width * f1_div_640_2 * 27.0f, RsGlobal.height * f1_div_448_2 * 110.0f, &sTimer);

					if (CUserDisplay::OnscnTimer.m_sEntries[0].m_aTimerText[0]) {
						CFont::SetPropOn();
						CFont::SetColor(CRGBA(0, 0, 0, 0xFFu));
						CFont::SetScale(RsGlobal.width * f1_div_640_2 * 0.80000001f * 0.80000001f, RsGlobal.height * f1_div_448_2 * 1.35f);
						float y = RsGlobal.height * f1_div_448_2 * 110.0f + 2.0f;
						float x = RsGlobal.width
							- 27.0f * (RsGlobal.width * f1_div_640_2)
							- 78.0f * (RsGlobal.width * f1_div_640_2);
						CFont::PrintString(x, y, TheText.Get((char*)CUserDisplay::OnscnTimer.m_sEntries[0].m_aTimerText));
						CFont::SetColor(CRGBA(0xBAu, 0x65u, 0x32u, 0xFFu));
						y = RsGlobal.height * f1_div_448_2 * 110.0f;
						x = RsGlobal.width
							- 27.0f * (RsGlobal.width * f1_div_640_2)
							- 80.0f * (RsGlobal.width * f1_div_640_2);
						CFont::PrintString(x, y, TheText.Get((char*)CUserDisplay::OnscnTimer.m_sEntries[0].m_aTimerText));
					}
				}
			}
			if (CUserDisplay::OnscnTimer.m_sEntries[0].m_bCounterProcessed == 1) {
				if (!CounterOnLastFrame)
					CounterFlashTimer = 1;
				CounterOnLastFrame = 1;
				if (CounterFlashTimer) {
					if (++CounterFlashTimer > 50)
						CounterFlashTimer = 0;
				}
				if (CTimer::GetFrameCounter() & 4 || !CounterFlashTimer) {
					if (CUserDisplay::OnscnTimer.m_sEntries[0].m_nType) {						
						float left = RsGlobal.width - 27.0f * RsGlobal.width * f1_div_640_2 - 100.0f * RsGlobal.width * f1_div_640_2 * 0.5f + 4.0f;
						float top = 132.0f * RsGlobal.height * f1_div_448_2 + 8.0f * RsGlobal.height * f1_div_448_2;
						float right = 4.0f + RsGlobal.width - 27.0f * RsGlobal.width * f1_div_640_2;
						float bottom = 11.0f * RsGlobal.height * f1_div_448_2 + 132.0f * RsGlobal.height * f1_div_448_2 + 8.0f * RsGlobal.height * f1_div_448_2;				

						CSprite2d::DrawRect(CRect(left, top, right, bottom), CRGBA(0, 0x6Au, 0xA4u, 0x50u));

						left = 4.0f + RsGlobal.width - 27.0f *  RsGlobal.width * f1_div_640_2 - 100.0f *  RsGlobal.width * f1_div_640_2 * 0.5f;
						top = 132.0f * RsGlobal.height * f1_div_448_2 + 8.0f * RsGlobal.height * f1_div_448_2;
						right = atoi(CUserDisplay::OnscnTimer.m_sEntries[0].m_bTimerBuffer) * RsGlobal.width * f1_div_640_2 * 0.5f + RsGlobal.width - 27.0f * RsGlobal.width * f1_div_640_2 - 100.0f * RsGlobal.width * f1_div_640_2 * 0.5f + 4.0f;
						bottom = 11.0f *  RsGlobal.height * f1_div_448_2 + 132.0f * RsGlobal.height * f1_div_448_2 + 8.0f * RsGlobal.height * f1_div_448_2;

						CSprite2d::DrawRect(CRect(left, top, right, bottom), CRGBA(0, 0x6Au, 0xA4u, 0xFFu));
					}
					else {
						AsciiToUnicode(CUserDisplay::OnscnTimer.m_sEntries[0].m_bTimerBuffer, &sTimer);

						CFont::SetPropOn();
						CFont::SetBackgroundOff();
						CFont::SetScale(RsGlobal.width * f1_div_640_2 * 0.80000001f, RsGlobal.height * f1_div_448_2 * 1.35f);
						CFont::SetCentreOff();
						CFont::SetRightJustifyOn();
						CFont::SetRightJustifyWrap(0.0f);
						CFont::SetFontStyle(2);
						CFont::SetColor(CRGBA(0xF4u, 0x14u, 0x14u, 0xFFu));
						CFont::SetWrapx(RsGlobal.width * f1_div_640_2 * 640.0f);
						CFont::SetPropOff();
						CFont::SetBackGroundOnlyTextOn();
						CFont::SetColor(CRGBA(0, 0, 0, 0xFFu));
						CFont::PrintString(RsGlobal.width - RsGlobal.width * f1_div_640_2 * 27.0f + 2.0f, RsGlobal.height * f1_div_448_2 * 132.0f + 2.0f, &sTimer);

						CFont::SetColor(CRGBA(0, 0x6Au, 0xA4u, 0xFFu));
						CFont::PrintString(RsGlobal.width - RsGlobal.width * f1_div_640_2 * 27.0f, RsGlobal.height * f1_div_448_2 * 132.0f, &sTimer);
					}
					if (CUserDisplay::OnscnTimer.m_sEntries[0].m_aCounterText[0]) {
						CFont::SetPropOn();
						CFont::SetScale(RsGlobal.width * f1_div_640_2 * 0.80000001f, RsGlobal.height * f1_div_448_2 * 1.35f);

						CFont::SetColor(CRGBA(0, 0, 0, 0xFFu));
						float y = RsGlobal.height * f1_div_448_2 * 132.0f + 2.0f;
						float x = RsGlobal.width
							- 27.0f * (RsGlobal.width * f1_div_640_2)
							- 59.0f * (RsGlobal.width * f1_div_640_2);

						CFont::PrintString(x, y, TheText.Get((char*)CUserDisplay::OnscnTimer.m_sEntries[0].m_aCounterText));
						CFont::SetColor(CRGBA(0, 0x6Au, 0xA4u, 0xFFu));

						y = RsGlobal.height * f1_div_448_2 * 132.0f;
						x = RsGlobal.width
							- 27.0f * (RsGlobal.width * f1_div_640_2)
							- 61.0f * (RsGlobal.width * f1_div_640_2);

						CFont::PrintString(x, y, TheText.Get((char*)CUserDisplay::OnscnTimer.m_sEntries[0].m_aCounterText));
					}
				}
			}
		}

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
					DMAudio.PlayFrontEndSound(96, 0);
					PagerSoundPlayed = 1;
				}
			}
			else if (PagerOn == 2) {
				float fStep = PagerXOffset * 0.05f;
				if (fStep < 2.0f)
					fStep = 2.0f;
				PagerXOffset += fStep * CTimer::GetTimeStep();
				if (PagerXOffset > 150.0f) {
					PagerXOffset = 150.0;
					PagerOn = 0;
				}
			}

			float left = 26.0f * RsGlobal.width * f1_div_640_2 - PagerXOffset;
			float top = 27.0f * RsGlobal.height * f1_div_448_2;
			float right = 160.0f * RsGlobal.width * f1_div_640_2 + 26.0f * RsGlobal.width * f1_div_640_2 - PagerXOffset;
			float bottom = 80.0f * RsGlobal.height * f1_div_448_2 + 27.0f * RsGlobal.height * f1_div_448_2;

			CHud::Sprites[HUD_PAGER].Draw(CRect(left, top, right, bottom), CRGBA(0xFFu, 0xFFu, 0xFFu, 0xFFu));

			CFont::SetBackgroundOff();
			CFont::SetScale(RsGlobal.width * f1_div_640_2 * 0.83999997f, RsGlobal.height * f1_div_448_2);
			CFont::SetColor(CRGBA(0x20u, 0xA2u, 0x42u, 0xCDu));
			CFont::SetRightJustifyOff();
			CFont::SetBackgroundOff();
			CFont::SetCentreOff();
			CFont::SetJustifyOff();
			CFont::SetPropOff();
			CFont::SetFontStyle(1);
			CFont::PrintString(RsGlobal.width * f1_div_640_2 * 52.0f - PagerXOffset, RsGlobal.height * f1_div_448_2 * 54.0f, CHud::m_PagerMessage);
		}

		if (CHud::m_ItemToFlash == 8 && CTimer::GetFrameCounter() & 8 || CHud::m_ItemToFlash != 8) {
			CRadar::DrawMap();
			float left = 36.0f;
			float top = RsGlobal.height - 123.0f * RsGlobal.height * f1_div_448_2 - 4.0f;
			float right = RsGlobal.width * f1_div_640_2 * 94.0f + 40.0f + 4.0f;
			float bottom = 76.0f * RsGlobal.height * f1_div_448_2 + RsGlobal.height - 123.0f * RsGlobal.height * f1_div_448_2 + 4.0f;

			CHud::Sprites[HUD_RADARDISC].Draw(CRect(36.0f, top, right, bottom), CRGBA(0, 0, 0, 0xFFu), CRGBA(0, 0, 0, 0xFFu), CRGBA(0, 0, 0, 0xFFu), CRGBA(0, 0, 0, 0xFFu));
			CRadar::DrawBlips();
		}

		if (CHud::m_Wants_To_Draw_3dMarkers == 1 && !TheCamera.m_WideScreenOn && !CHud::m_BigMessage[0] && !CHud::m_BigMessage[256]) {
			CRadar::Draw3dMarkers();
		}

		if (!CTimer::GetIsPaused()) {
			uint16 CounterA = 0;
			uint16 CounterB = 0;
			CTextLine* IntroText = CTheScripts::IntroTextLines;

			do {
				if (CTheScripts::IntroTextLines[CounterB].m_awText[0] && CTheScripts::IntroTextLines[CounterB].field_29) {
					float h = RsGlobal.height * f1_div_448_2 * CTheScripts::IntroTextLines[CounterB].m_fScaleY * 0.5f;
					float w = RsGlobal.width * f1_div_640_2 * CTheScripts::IntroTextLines[CounterB].m_fScaleX;

					CFont::SetScale(w, h);
					CFont::SetColor(CTheScripts::IntroTextLines[CounterB].m_sColor);

					if (CTheScripts::IntroTextLines[CounterB].m_bJustify)
						CFont::SetJustifyOn();
					else
						CFont::SetJustifyOff();

					if (CTheScripts::IntroTextLines[CounterB].m_bRightJustify)
						CFont::SetRightJustifyOn();
					else
						CFont::SetRightJustifyOff();

					if (CTheScripts::IntroTextLines[CounterB].m_bCentered)
						CFont::SetCentreOn();
					else
						CFont::SetCentreOff();

					CFont::SetWrapx(RsGlobal.width * f1_div_640_2 * CTheScripts::IntroTextLines[CounterB].m_fWrapX);
					CFont::SetCentreSize(RsGlobal.width * f1_div_640_2 * CTheScripts::IntroTextLines[CounterB].m_fCenterSize);

					if (CTheScripts::IntroTextLines[CounterB].m_bBackground)
						CFont::SetBackgroundOn();
					else
						CFont::SetBackgroundOff();

					CFont::SetBackgroundColor(CTheScripts::IntroTextLines[CounterB].m_sBackgroundColor);

					if (CTheScripts::IntroTextLines[CounterB].m_bBackgroundOnly)
						CFont::SetBackGroundOnlyTextOn();
					else
						CFont::SetBackGroundOnlyTextOff();

					if (CTheScripts::IntroTextLines[CounterB].m_bTextProportional)
						CFont::SetPropOn();
					else
						CFont::SetPropOff();

					CFont::SetFontStyle(CTheScripts::IntroTextLines[CounterB].m_nFont);
					float y = RsGlobal.height
						- RsGlobal.height * f1_div_448_2 * (448.0f - CTheScripts::IntroTextLines[CounterB].field_40);
					float x = RsGlobal.width
						- RsGlobal.width * f1_div_640_2 * (640.0f - CTheScripts::IntroTextLines[CounterB].field_36);

					CFont::PrintString(x, y, IntroText->m_awText);
				}
				++CounterA;
				++CounterB;
				++IntroText;
			} while (CounterA < 2u);

			uint16 CounterC = 0;
			uint16 CounterD = 0;
			CScriptRectangle* IntroRect = CTheScripts::IntroRectangles;

			do {
				if (CTheScripts::IntroRectangles[CounterD].m_bIsUsed && CTheScripts::IntroRectangles[CounterD].m_bIsAntialiased) {
					if (CTheScripts::IntroRectangles[CounterD].m_wTextureId >= 0) {
						CRect rect = {
							CTheScripts::IntroRectangles[CounterD].m_sRect.left,
							CTheScripts::IntroRectangles[CounterD].m_sRect.bottom,
							CTheScripts::IntroRectangles[CounterD].m_sRect.right,
							CTheScripts::IntroRectangles[CounterD].m_sRect.bottom };

						CTheScripts::ScriptSprites[CTheScripts::IntroRectangles[CounterD].m_wTextureId].Draw(rect, IntroRect->m_sColor);
					}
					else {
						CRect rect = {
							CTheScripts::IntroRectangles[CounterD].m_sRect.left,
							CTheScripts::IntroRectangles[CounterD].m_sRect.bottom,
							CTheScripts::IntroRectangles[CounterD].m_sRect.right,
							CTheScripts::IntroRectangles[CounterD].m_sRect.bottom };

						CSprite2d::DrawRect(rect, IntroRect->m_sColor);
					}
				}
				++CounterC;
				++CounterD;
				++IntroRect;
			} while (CounterC < 0x10u);
			
			if (CHud::m_Message[0] && !CHud::m_BigMessage[2][0] && (FrontEndMenuManager.m_PrefsShowSubtitles == 1 || !TheCamera.m_WideScreenOn)) {
				CFont::SetJustifyOff();
				CFont::SetBackgroundOff();
				CFont::SetBackgroundColor(CRGBA(0, 0, 0, 0x80u));
				CFont::SetScale(RsGlobal.maximumWidth * f1_div_640_2 * 0.47999999f, RsGlobal.maximumHeight * f1_div_448_2 * 1.120f);
				CFont::SetCentreOn();
				CFont::SetPropOn();
				CFont::SetFontStyle(0);

				float m_fCentreSize = RsGlobal.maximumWidth
					- 50.0f * (RsGlobal.maximumWidth * f1_div_640_2)
					- 8.0f * (RsGlobal.maximumWidth * f1_div_640_2)
					- (40.0f * (RsGlobal.maximumWidth * f1_div_640_2)
						+ 8.0f * (RsGlobal.maximumWidth * f1_div_640_2));

				CFont::SetCentreSize(m_fCentreSize);
				CFont::SetDropShadowPosition(1);
				CFont::SetDropColor(CRGBA(0, 0, 0, 0xFFu));
				CFont::SetColor(CRGBA(0xEBu, 0xEBu, 0xEBu, 0xFFu));
				float y = 4.0f * (RsGlobal.maximumHeight * f1_div_448_2)
					+ RsGlobal.maximumHeight
					- 68.0f * (RsGlobal.maximumHeight * f1_div_448_2)
					- 1.4f;

				float x = (RsGlobal.maximumWidth
					- 50.0f * RsGlobal.maximumWidth * f1_div_640_2
					- 8.0f * RsGlobal.maximumWidth * f1_div_640_2
					- (40.0f * RsGlobal.maximumWidth * f1_div_640_2
						+ 8.0f * RsGlobal.maximumWidth * f1_div_640_2))
					* 0.5f
					+ 40.0f * RsGlobal.maximumWidth * f1_div_640_2
					+ 8.0f * RsGlobal.maximumWidth * f1_div_640_2
					- 1.4f;

				CFont::PrintString(x, y, CHud::m_Message);
				CFont::SetDropShadowPosition(0);
			}
			if (CHud::m_BigMessage[0][0]) {
				if (0 == BigMessageInUse[0]) {
					CFont::SetJustifyOff();
					CFont::SetBackgroundOff();
					CFont::SetBackGroundOnlyTextOff();
					CFont::SetScale(RsGlobal.maximumWidth * f1_div_640_2 * 1.8f, RsGlobal.maximumHeight * f1_div_448_2 * 1.8f);
					CFont::SetPropOn();
					CFont::SetCentreOn();
					CFont::SetCentreSize(RsGlobal.maximumWidth * f1_div_640_2 * 615.0f);
					CFont::SetColor(CRGBA(0xFFu, 0xFFu, 0, 0xFFu));
					CFont::SetFontStyle(2);
					if ((RsGlobal.maximumWidth - 20) <= BigMessageX[0]) {
						BigMessageInUse[0] = BigMessageInUse[0] + CTimer::GetTimeStep();
						if (BigMessageInUse[0] >= 120.0f) {
							BigMessageInUse[0] = 120.0;
							BigMessageAlpha[0] = BigMessageAlpha[0]
								- (unsigned int)(signed __int64)(CTimer::GetTimeStep() *  0.02f  * 1000.0f)
								* 0.30000001f;
						}
						if (BigMessageAlpha[0] <= 0.0f) {
							CHud::m_BigMessage[0][0] = 0;
							BigMessageAlpha[0] = 0.0;
						}
					}
					else {
						float fStep = (unsigned int)(signed __int64)(CTimer::GetTimeStep()
							* 0.02f
							* 1000.0f)
							* 0.30000001f;
						BigMessageX[0] = BigMessageX[0] + fStep;
						BigMessageAlpha[0] = BigMessageAlpha[0] + fStep;
						if (BigMessageAlpha[0] > 255.0f)
							BigMessageAlpha[0] = 255.0;
					}
					CFont::SetColor(CRGBA(0, 0, 0, (signed int)BigMessageAlpha[0]));

					float y = RsGlobal.maximumHeight * 0.5f
						- RsGlobal.maximumHeight * f1_div_448_2 * 20.0f
						+ 2.0f;
					float x = (RsGlobal.maximumWidth / 2) + 2.0f;

					CFont::PrintString(x, y, CHud::m_BigMessage[256]);

					CFont::SetColor(CRGBA(0x55u, 0x77u, 0x85u, (signed int)BigMessageAlpha[0]));
					y = RsGlobal.maximumHeight * 0.5f
						- RsGlobal.maximumHeight * f1_div_448_2 * 18.0f;
					x = (RsGlobal.maximumWidth / 2);

					CFont::PrintString(x, y, CHud::m_BigMessage[256]);
				}
				else {
					BigMessageAlpha[0] = 0.0;
					BigMessageX[0] = -60.0;
					BigMessageInUse[0] = 1.0;
				}
			}
			else {
				BigMessageInUse[0] = 0;
			}
			if (CHud::m_BigMessage[2][0]) {
				if (0 == BigMessageInUse[2]) {
					BigMessageAlpha[2] = (unsigned int)(signed __int64)(CTimer::GetTimeStep()
						* 0.02f
						* 1000.0f)
						* 0.40000001
						+ BigMessageAlpha[2];
					if (BigMessageAlpha[2] > 255.0f)
						BigMessageAlpha[2] = 255.0;
					CFont::SetBackgroundOff();

					float w = RsGlobal.maximumWidth * f1_div_640_2 * 2.0f;
					float h = RsGlobal.maximumHeight * f1_div_448_2 * 2.0f;

					if (CGame::frenchGame || CGame::germanGame) {
						h = RsGlobal.maximumHeight * f1_div_448_2 * 1.4f;
						w = RsGlobal.maximumWidth * f1_div_640_2 * 1.4f;
					}
					CFont::SetScale(w, h);
					CFont::SetPropOn();
					CFont::SetRightJustifyOn();
					CFont::SetFontStyle(2);
					CFont::SetColor(CRGBA(0, 0, 0, 0.75f * BigMessageAlpha[2]));

					float y = RsGlobal.maximumHeight - RsGlobal.maximumHeight * f1_div_448_2 * 78.0f;
					float x = RsGlobal.maximumWidth
						- RsGlobal.maximumWidth * f1_div_640_2 * 20.0f
						+ 4.0f;
					CFont::PrintString(x, y, CHud::m_BigMessage[2]);
					CFont::SetColor(CRGBA(0xAAu, 0x7Bu, 0x57u, (signed int)BigMessageAlpha[2]));
					y = RsGlobal.maximumHeight - RsGlobal.maximumHeight * f1_div_448_2 * 82.0f;
					x = RsGlobal.maximumWidth - RsGlobal.maximumWidth * f1_div_640_2 * 20.0f;
					CFont::PrintString(x, y, CHud::m_BigMessage[2]);
				}
				else {
					BigMessageAlpha[2] = 0.0;
					BigMessageInUse[2] = 1.0;
				}
			}
			else {
				BigMessageInUse[2] = 0.0;
			}
		}
	}
}

int CHud::GetRidOfAllHudMessages() {
	return ((int(__cdecl *)())0x504F90)();
}

int CHud::SetBigMessage(wchar* message, int style) {
	return ((int(__cdecl *)(wchar*, int))0x50A250)(message, style);
}

void CHud::SetHelpMessage(wchar* message, bool quick) {
	int nCounterOne;
	int nCounterTwo;

	if (CReplay::Mode != 1) {
		CMessages::WideStringCopy(CHud::m_pHelpMessage, message, 0x100u);
		CMessages::InsertPlayerControlKeysInString(CHud::m_pHelpMessage);
		nCounterOne = 0;
		do {
			nCounterTwo = nCounterOne;
			nCounterOne += 8;
			*(CHud::m_pLastHelpMessage + nCounterTwo) = 0;
			*(CHud::m_pLastHelpMessage + nCounterTwo + 1) = 0;
			*(CHud::m_pLastHelpMessage + nCounterTwo + 2) = 0;
			*(CHud::m_pLastHelpMessage + nCounterTwo + 3) = 0;
		} while (nCounterOne < 0x100u);
		CHud::m_nHelpMessageState = 0;
		CHud::m_bHelpMessageQuick = quick;
	}
}

void CHud::SetMessage(wchar* message) {
	unsigned __int16 v1; // cx
	int v2; // ebx
	__int16 v3; // dx
	int result; // eax

	v1 = 0;
	while (v1 < 0x100u)
	{
		v2 = v1;
		v3 = *(message + 2 * v1);
		if (!v3)
			break;
		++v1;
		*(CHud::m_Message + v2) = v3;
	}
	result = v1;
	*(CHud::m_Message + v1) = 0;
}

void CHud::SetPagerMessage(wchar* message) {
	unsigned __int16 v1; // cx
	int v2; // ebx
	__int16 v3; // dx

	v1 = 0;
	while (v1 < 256u) {
		v2 = v1;
		v3 = message[v1];
		if (!v3)
			break;
		++v1;
		CHud::m_PagerMessage[v2] = v3;
	}
	CHud::m_PagerMessage[v1] = 0;
}

int CHud::SetVehicleName(wchar* name) {
	m_pVehicleName = name;
	return *name;
}

int CHud::SetZoneName(wchar* name) {
	m_pZoneName = name;
	return *name;
}

void CHud::Shutdown() {
	for (int i = 0; i < 23; ++i) {
		Sprites[i].Delete();
	}

	RwTextureDestroy(gpSniperSightTex);
	gpSniperSightTex = 0;

	RwTextureDestroy(gpRocketSightTex);
	gpRocketSightTex = 0;

	int HudTXD = CTxdStore::FindTxdSlot("hud");
	CTxdStore::RemoveTxdSlot(HudTXD);
}

STARTPATCHES
	InjectHook(0x48BC9A, &CHud::Initialise, PATCH_CALL);
	InjectHook(0x48C4F1, &CHud::ReInitialise, PATCH_CALL);
	InjectHook(0x48BCBC, &CHud::Shutdown, PATCH_CALL);
ENDPATCHES
