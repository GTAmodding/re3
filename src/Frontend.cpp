#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include "common.h"
#include "patcher.h"
#include "win.h"
#include "Frontend.h"
#include "Font.h"
#include "Pad.h"
#include "Text.h"
#include "main.h"
#include "Timer.h"
#include "Game.h"
#include "DMAudio.h"
#include "MusicManager.h"
#include "FileMgr.h"
#include "Streaming.h"
#include "TxdStore.h"
#include "General.h"
#include "PCSave.h"
#include "Script.h"
#include "Camera.h"
#include "MenuScreens.h"
#include "ControllerConfig.h"
#include "Vehicle.h"
#include "MBlur.h"
#include "PlayerSkin.h"

int32 &CMenuManager::OS_Language = *(int32*)0x5F2F78;
int8 &CMenuManager::m_PrefsUseVibration = *(int8*)0x95CD92;
int8 &CMenuManager::m_DisplayControllerOnFoot = *(int8*)0x95CD8D;
int8 &CMenuManager::m_PrefsVsync = *(int8*)0x5F2E58;
int8 &CMenuManager::m_PrefsVsyncDisp = *(int8*)0x5F2E5C;
int8 &CMenuManager::m_PrefsFrameLimiter = *(int8*)0x5F2E60;
int8 &CMenuManager::m_PrefsShowSubtitles = *(int8*)0x5F2E54;
int8 &CMenuManager::m_PrefsSpeakers = *(int8*)0x95CD7E;
int8 &CMenuManager::m_ControlMethod = *(int8*)0x8F5F7C;
int8 &CMenuManager::m_PrefsDMA = *(int8*)0x5F2F74;
int8 &CMenuManager::m_PrefsLanguage = *(int8*)0x941238;

bool &CMenuManager::m_PrefsAllowNastyGame = *(bool*)0x5F2E64;
bool &CMenuManager::m_bStartUpFrontEndRequested = *(bool*)0x95CCF4;
bool &CMenuManager::m_bShutDownFrontEndRequested = *(bool*)0x95CD6A;

int8 &CMenuManager::m_PrefsUseWideScreen = *(int8*)0x95CD23;
int8 &CMenuManager::m_PrefsRadioStation = *(int8*)0x95CDA4;
int8 &CMenuManager::m_bDisableMouseSteering = *(int8*)0x60252C;
int32 &CMenuManager::m_PrefsBrightness = *(int32*)0x5F2E50;
float &CMenuManager::m_PrefsLOD = *(float*)0x8F42C4;
int8 &CMenuManager::m_bFrontEnd_ReloadObrTxtGxt = *(int8*)0x628CFC;
int32 &CMenuManager::m_PrefsMusicVolume = *(int32*)0x5F2E4C;
int32 &CMenuManager::m_PrefsSfxVolume = *(int32*)0x5F2E48;

uint8 *CMenuManager::m_PrefsSkinFile = (uint8*)0x5F2E74;

CMenuManager &FrontEndMenuManager = *(CMenuManager*)0x8F59D8;

// Move this somewhere else.
float lodMultiplier = *(float*)0x5F726C;

// Stuff not in CMenuManager:
int VibrationTime;
char* pEditString;
int32 pControlEdit;
int8 DisplayComboButtonErrMsg;
bool MouseButtonJustClicked;
bool JoyButtonJustClicked;

// Frontend inputs.
bool GetPadBack();
bool GetPadExitEnter();
bool GetPadForward();
bool GetPadMoveUp();
bool GetPadMoveDown();
bool GetPadMoveLeft();
bool GetPadMoveRight();
bool GetMouseForward();
bool GetMouseBack();
bool GetMousePos();
bool GetMouseMoveLeft();
bool GetMouseMoveRight();
bool GetPadInput();
bool GetMouseInput();

char *FrontendFilenames[] = {
	"fe2_mainpanel_ul",
	"fe2_mainpanel_ur",
	"fe2_mainpanel_dl",
	"fe2_mainpanel_dr",
	"fe2_mainpanel_dr2",
	"fe2_tabactive",
	"fe_iconbrief",
	"fe_iconstats",
	"fe_iconcontrols",
	"fe_iconsave",
	"fe_iconaudio",
	"fe_icondisplay",
	"fe_iconlanguage",
	"fe_controller",
	"fe_controllersh",
	"fe_arrows1",
	"fe_arrows2",
	"fe_arrows3",
	"fe_arrows4",
	"fe_radio1", // HEAD_RADIO
	"fe_radio2", // DOUBLE_CLEF
	"fe_radio5", // JAH_RADIO
	"fe_radio7", // RISE_FM
	"fe_radio8", // LIPS_106
	"fe_radio3", // GAME_FM
	"fe_radio4", // MSX_FM
	"fe_radio6", // FLASHBACK
	"fe_radio9", // CHATTERBOX
};					

char *MenuFilenames[] = {
	"connection24",	"",
	"findgame24",	"",
	"hostgame24",	"",
	"mainmenu24",	"",
	"Playersetup24",	"",
	"singleplayer24",	"",
	"multiplayer24",	"",
	"dmalogo128",	"dmalogo128m",
	"gtaLogo128",	"gtaLogo128",
	"rockstarLogo128",	"rockstarlogo128m",
	"gamespy256",	"gamespy256a",
	"mouse",	"mousetimera",
	"mousetimer",	"mousetimera",
	"mp3logo",	"mp3logoA",
	"downOFF",	"buttonA",
	"downON",	"buttonA",
	"upOFF",	"buttonA",
	"upON",	"buttonA",
	"gta3logo256",	"gta3logo256m",
	nil, nil
};

#if 1
WRAPPER void CMenuManager::BuildStatLine(char *, void *, uint16, void *) { EAXJMP(0x483870); }
#else
void CMenuManager::BuildStatLine(char *, void *, uint16, void *)
{

}
#endif

#if 0
WRAPPER void CMenuManager::CentreMousePointer() { EAXJMP(0x48ACE0); }
#else
void CMenuManager::CentreMousePointer()
{
	tagPOINT Point;

	if (SCREENW * 0.5f == 0.0f && 0.0f == SCREENH * 0.5f) {
		Point.x = SCREEN_WIDTH / 2;
		Point.y = SCREEN_HEIGHT / 2;
		ClientToScreen(PSGLOBAL(window), &Point);
		SetCursorPos(Point.x, Point.y);

		PSGLOBAL(lastMousePos.x) = SCREEN_WIDTH / 2;
		PSGLOBAL(lastMousePos.y) = SCREEN_HEIGHT / 2;
	}
}
#endif

#if 1
WRAPPER void CMenuManager::CheckCodesForControls(int, int) { EAXJMP(0x48A950); }
#else
void CMenuManager::CheckCodesForControls()
{

}
#endif

#if 0
WRAPPER bool CMenuManager::CheckHover(int, int, int, int) { EAXJMP(0x48ACA0); }
#else
bool CMenuManager::CheckHover(int x1, int x2, int y1, int y2)
{
	return m_nMousePosX > x1 && m_nMousePosX < x2 &&
	       m_nMousePosY > y1 && m_nMousePosY < y2;
}
#endif

void CMenuManager::CheckSliderMovement(int value)
{
	float fBrightness = 0.0f;
	float fDrawDistance = 0.0f;
	float fRadioVolume = 0.0f;
	float fSfxVolume = 0.0f;
	float fMouseSens = 0.0f;

	switch (aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption].m_Action) {
	case MENUACTION_BRIGHTNESS:
		fBrightness = m_PrefsBrightness + (value * (512.0f) / 16.0f);

		if (fBrightness > 511.0f)
			fBrightness = 511.0f;
		else if (fBrightness < 0.0f)
			fBrightness = 0.0f;

		m_PrefsBrightness = fBrightness;
		SaveSettings();
		break;
	case MENUACTION_DRAWDIST:
		fDrawDistance = m_PrefsLOD + (value * (1.8f - 0.8f) / 16.0f);

		if (fDrawDistance > 1.8f)
			fDrawDistance = 1.8f;
		else if (fDrawDistance < 0.8f)
			fDrawDistance = 0.8f;

		m_PrefsLOD = fDrawDistance;
		SaveSettings();
		break;
	case MENUACTION_MUSICVOLUME:
		fRadioVolume = m_PrefsMusicVolume + (value * (128.0f) / 16.0f);

		if (fRadioVolume > 127.0f)
			fRadioVolume = 127.0f;
		else if (fRadioVolume < 0.0f)
			fRadioVolume = 0.0f;

		m_PrefsMusicVolume = fRadioVolume;
		DMAudio.SetMusicMasterVolume(fRadioVolume);
		SaveSettings();
		break;
	case MENUACTION_SFXVOLUME:
		fSfxVolume = m_PrefsSfxVolume + (value * (128.0f) / 16.0f);

		if (fSfxVolume > 127)
			fSfxVolume = 127;
		else if (fSfxVolume < 0.0f)
			fSfxVolume = 0.0f;

		m_PrefsSfxVolume = fSfxVolume;
		DMAudio.SetEffectsMasterVolume(fSfxVolume);
		SaveSettings();
		break;
	case MENUACTION_MOUSESENS:
		fMouseSens = TheCamera.m_fMouseAccelHorzntl + (value * (0.005f - 0.0003125f) / 16.0f);

		if (fMouseSens > 0.005f)
			fMouseSens = 0.005f;
		else if (fMouseSens < 0.0003125f)
			fMouseSens = 0.0003125f;

		TheCamera.m_fMouseAccelHorzntl = fMouseSens;

		// BUG: game doesn't set Y Axis.
		TheCamera.m_fMouseAccelVertical = fMouseSens;
		SaveSettings();
		break;
	};
}

#if 1
WRAPPER int CMenuManager::CostructStatLine(int) { EAXJMP(0x482800); }
#else
int CMenuManager::CostructStatLine(int)
{

}
#endif

#if 0
WRAPPER void CMenuManager::DisplayHelperText() { EAXJMP(0x48B490); }
#else
void CMenuManager::DisplayHelperText()
{
	static int32 AlphaText = 255;
	static int32 Time = 0;

	if (m_nHelperTextMsgId && m_nHelperTextMsgId != 1) {
		if (CTimer::GetTimeInMillisecondsPauseMode() - Time > 10) {
			Time = CTimer::GetTimeInMillisecondsPauseMode();
			m_nHelperTextAlpha -= 2;

			if (AlphaText < 1)
				ResetHelperText();

			AlphaText = m_nHelperTextAlpha > 255 ? 255 : m_nHelperTextAlpha;
		}
	}

	wchar *HelperTextToPrint = nil;
	// TODO: name this cases?
	switch (m_nHelperTextMsgId) {
	case 0:
		HelperTextToPrint = TheText.Get("FET_MIG");
		break;
	case 1:
		HelperTextToPrint = TheText.Get("FET_APP");
		break;
	case 2:
		HelperTextToPrint = TheText.Get("FET_HRD");
		break;
	case 3:
		HelperTextToPrint = TheText.Get("FET_RSO");
		break;
	case 4:
		HelperTextToPrint = TheText.Get("FET_RSC");
		break;
	default:
		break;
	};

	CFont::SetAlignment(ALIGN_CENTER);
	CFont::SetScale(SCREEN_SCALE_X(0.4f), SCREEN_SCALE_Y(0.6f));
	CFont::SetFontStyle(FONT_HEADING);
	CFont::SetDropColor(CRGBA(0, 0, 0, AlphaText));
	CFont::SetDropShadowPosition(MENUDROP_COLOR_SIZE);
	CFont::SetColor(CRGBA(255, 255, 255, AlphaText));

	CFont::PrintString(SCREEN_WIDTH / 2, SCREEN_SCALE_FROM_BOTTOM(120.0f), HelperTextToPrint);
}
#endif

#if 0
WRAPPER float CMenuManager::DisplaySlider(float, float, float, float, float, float) { EAXJMP(0x488420); }
#else
float CMenuManager::DisplaySlider(float x, float y, float leftSize, float rightSize, float rectSize, float progress) 
{
	CRGBA color;
	float sizeRange;

	float input = 0.0f;
	for (int i = 0; i < 16; i++) {
		input = i * rectSize/16.0f + x;

		if (i/16.0f + 1/32.0f < progress)
			color = CRGBA(255, 217, 106, FadeIn(255));
		else
			color = CRGBA(185, 120, 0, FadeIn(255));

		sizeRange = max(leftSize, rightSize);

		float _x = i * rectSize/16.0f + x;
		float _y = y + sizeRange - ((16 - i) * leftSize + i * rightSize)/16.0f;
		float _w = SCREEN_SCALE_X(10.0f) + i * rectSize/16.0f + x;
		float _h = y + sizeRange;
		float _s = SCREEN_SCALE_X(2.0f);
		CSprite2d::DrawRect(CRect(_x + _s, _y + _s, _w + _s, _h + _s), CRGBA(0, 0, 0, FadeIn(255))); // Shadow
		CSprite2d::DrawRect(CRect(i * rectSize/16.0f + x, y + sizeRange - ((16 - i) * leftSize + i * rightSize)/16.0f, SCREEN_SCALE_X(10.0f) + i * rectSize/16.0f + x, y + sizeRange), color);
	};
	return input;
}
#endif

#if 0
WRAPPER void CMenuManager::DoSettingsBeforeStartingAGame() { EAXJMP(0x48AB40); }
#else
void CMenuManager::DoSettingsBeforeStartingAGame()
{
	CCamera::m_bUseMouse3rdPerson = m_ControlMethod == 0;
	if (m_PrefsVsyncDisp != m_PrefsVsync)
		m_PrefsVsync = m_PrefsVsyncDisp;

	m_bStartGameLoading = true;

	ShutdownJustMenu();
	UnloadTextures();
	DMAudio.SetEffectsFadeVol(0);
	DMAudio.SetMusicFadeVol(0);
	DMAudio.ResetTimers(CTimer::GetTimeInMilliseconds());
}
#endif

#if 0
WRAPPER void CMenuManager::Draw() { EAXJMP(0x47AE00); }
#else
void CMenuManager::Draw()
{
	CFont::SetBackgroundOff();
	CFont::SetPropOn();
	CFont::SetCentreOff();
	CFont::SetJustifyOn();
	CFont::SetBackGroundOnlyTextOn();
	CFont::SetWrapx(SCREEN_SCALE_FROM_RIGHT(40.0f));
	CFont::SetRightJustifyWrap(0.0f);
	CFont::SetDropColor(CRGBA(0, 0, 0, FadeIn(MENUDROP_COLOR_A)));

	switch (m_nCurrScreen) {
	case MENUPAGE_STATS:
		PrintStats();
		break;
	case MENUPAGE_BRIEFS:
		PrintBriefs();
		break;
	case MENUPAGE_CONTROLLER_DEBUG:
		DrawControllerScreenExtraText(0, 350, 20);
		break;
	}

	// Header.
	if (aScreens[m_nCurrScreen].m_ScreenName[0]) {
		CFont::SetDropShadowPosition(0);
		CFont::SetColor(CRGBA(0, 0, 0, FadeIn(255)));
		CFont::SetRightJustifyOn();
		CFont::SetFontStyle(FONT_HEADING);
		CFont::SetScale(SCREEN_SCALE_X(MENUHEADER_WIDTH), SCREEN_SCALE_Y(MENUHEADER_HEIGHT));
		CFont::PrintString(SCREEN_SCALE_FROM_RIGHT(MENUHEADER_POS_X), SCREEN_SCALE_FROM_BOTTOM(MENUHEADER_POS_Y), TheText.Get(aScreens[m_nCurrScreen].m_ScreenName));
	}

	// Action text.
	wchar *str;
	if (aScreens[m_nCurrScreen].m_aEntries[0].m_Action == MENUACTION_LABEL) {
		switch (m_nCurrScreen) {
		case MENUPAGE_LOAD_SLOT_CONFIRM:
			if (m_bGameNotLoaded)
				str = TheText.Get("FES_LCG");
			else
				str = TheText.Get(aScreens[m_nCurrScreen].m_aEntries[0].m_EntryName);
			break;
		case MENUPAGE_SAVE_OVERWRITE_CONFIRM:
			if (Slots[m_nCurrSaveSlot] == 1)
				str = TheText.Get("FESZ_QZ");
			else
				str = TheText.Get(aScreens[m_nCurrScreen].m_aEntries[0].m_EntryName);
			break;
		case MENUPAGE_EXIT:
			if (m_bGameNotLoaded)
				str = TheText.Get("FEQ_SRW");
			else
				str = TheText.Get(aScreens[m_nCurrScreen].m_aEntries[0].m_EntryName);
			break;
		default:
			str = TheText.Get(aScreens[m_nCurrScreen].m_aEntries[0].m_EntryName);
			break;
		};

		CFont::SetDropShadowPosition(MENUDROP_COLOR_SIZE);
		CFont::SetDropColor(CRGBA(0, 0, 0, FadeIn(MENUDROP_COLOR_A)));
		CFont::SetFontStyle(FONT_BANK);
		CFont::SetScale(SCREEN_SCALE_X(MENUACTION_WIDTH), SCREEN_SCALE_Y(MENUACTION_HEIGHT));
		CFont::SetAlignment(ALIGN_LEFT);
		CFont::SetColor(CRGBA(235, 170, 50, FadeIn(255)));
		CFont::PrintString(SCREEN_SCALE_X(MENUACTION_POS_X), SCREEN_SCALE_Y(MENUACTION_POS_Y), str);
	}

	for (int i = 0; i < MENUROWS; ++i) {
		if (aScreens[m_nCurrScreen].m_aEntries[i].m_Action != MENUACTION_LABEL && aScreens[m_nCurrScreen].m_aEntries[i].m_EntryName[0]) {
			wchar *textToPrint[MENUCOLUMNS] = { nil, nil };
			bool Locked = false;

			if (aScreens[m_nCurrScreen].m_aEntries[i].m_SaveSlot >= SAVESLOT_1 && aScreens[m_nCurrScreen].m_aEntries[i].m_SaveSlot <= SAVESLOT_8) {
				textToPrint[MENUCOLUMN_LEFT] = GetNameOfSavedGame(i - 1);
				textToPrint[MENUCOLUMN_RIGHT] = GetSavedGameDateAndTime(i - 1);

				if (!textToPrint[MENUCOLUMN_LEFT][0]) {
					sprintf(gString, "FEM_SL%d", i);
					textToPrint[MENUCOLUMN_LEFT] = TheText.Get(gString);
				}
			}
			else {
				textToPrint[MENUCOLUMN_LEFT] = TheText.Get(aScreens[m_nCurrScreen].m_aEntries[i].m_EntryName);

				if (aScreens[m_nCurrScreen].m_aEntries[i].m_Action == MENUACTION_SCREENRES) {
					if (m_bGameNotLoaded)
						Locked = false;
					else
						Locked = true;
				}
			}

			switch (aScreens[m_nCurrScreen].m_aEntries[i].m_Action) {
			case MENUACTION_CTRLVIBRATION:
				break;
			case MENUACTION_CTRLCONFIG:
				switch (CPad::GetPad(0)->Mode) {
				case 0:
					textToPrint[MENUCOLUMN_RIGHT] = TheText.Get("FEC_CF1");
					break;
				case 1:
					textToPrint[MENUCOLUMN_RIGHT] = TheText.Get("FEC_CF2");
					break;
				case 2:
					textToPrint[MENUCOLUMN_RIGHT] = TheText.Get("FEC_CF3");
					break;
				case 3:
					textToPrint[MENUCOLUMN_RIGHT] = TheText.Get("FEC_CF4");
					break;
				};
				break;
			case MENUACTION_CTRLDISPLAY:
				break;
			case MENUACTION_FRAMESYNC:
				textToPrint[MENUCOLUMN_RIGHT] = TheText.Get(m_PrefsVsyncDisp ? "FEM_ON" : "FEM_OFF");
				break;
			case MENUACTION_FRAMELIMIT:
				textToPrint[MENUCOLUMN_RIGHT] = TheText.Get(m_PrefsFrameLimiter ? "FEM_ON" : "FEM_OFF");
				break;
			case MENUACTION_TRAILS:
				textToPrint[MENUCOLUMN_RIGHT] = TheText.Get(CMBlur::BlurOn ? "FEM_ON" : "FEM_OFF");
				break;
			case MENUACTION_SUBTITLES:
				textToPrint[MENUCOLUMN_RIGHT] = TheText.Get(m_PrefsShowSubtitles ? "FEM_ON" : "FEM_OFF");
				break;
			case MENUACTION_WIDESCREEN:
#ifndef HOR_PLUS
				textToPrint[MENUCOLUMN_RIGHT] = TheText.Get(m_PrefsUseWideScreen ? "FEM_ON" : "FEM_OFF");
#else
				switch (m_PrefsUseWideScreen) {
				case AR_AUTO:
					textToPrint[MENUCOLUMN_RIGHT] = (wchar*)L"AUTO";
					break;
				case AR_3_2:
					textToPrint[MENUCOLUMN_RIGHT] = (wchar*)L"3:2";
					break;
				case AR_4_3:
					textToPrint[MENUCOLUMN_RIGHT] = (wchar*)L"4:3";
					break;
				case AR_5_3:
					textToPrint[MENUCOLUMN_RIGHT] = (wchar*)L"5:3";
					break;
				case AR_5_4:
					textToPrint[MENUCOLUMN_RIGHT] = (wchar*)L"5:4";
					break;
				case AR_15_9:
					textToPrint[MENUCOLUMN_RIGHT] = (wchar*)L"15:9";
					break;
				case AR_16_9:
					textToPrint[MENUCOLUMN_RIGHT] = (wchar*)L"16:9";
					break;
				case AR_16_10:
					textToPrint[MENUCOLUMN_RIGHT] = (wchar*)L"16:10";
					break;
				case AR_64_27:
					textToPrint[MENUCOLUMN_RIGHT] = (wchar*)L"64:27";
					break;
			};
#endif
				break;
			case MENUACTION_RADIO:
				sprintf(gString, "FEA_FM%d", m_PrefsRadioStation);
				textToPrint[MENUCOLUMN_RIGHT] = TheText.Get(gString);
				break;
			case MENUACTION_SETDBGFLAG:
				textToPrint[MENUCOLUMN_RIGHT] = TheText.Get(CTheScripts::DbgFlag ? "FEM_ON" : "FEM_OFF");
				break;
			case MENUACTION_SWITCHBIGWHITEDEBUGLIGHT:
				textToPrint[MENUCOLUMN_RIGHT] = TheText.Get(CTheScripts::DbgFlag ? "FEM_ON" : "FEM_OFF");
				break;
			case MENUACTION_INVVERT:
				textToPrint[MENUCOLUMN_RIGHT] = TheText.Get(MousePointerStateHelper.bInvertVertically ? "FEM_ON" : "FEM_OFF");
				break;
			case MENUACTION_SCREENRES:
			{
				char *res = _psGetVideoModeList()[m_nDisplayVideoMode];

				if (!res)
					res = "";

				AsciiToUnicode(res, gUString);
				textToPrint[MENUCOLUMN_RIGHT] = gUString;
			}
			break;
			case MENUACTION_AUDIOHW:
				if (m_nPrefsAudio3DProviderIndex == -1)
					textToPrint[MENUCOLUMN_RIGHT] = TheText.Get("FEA_NAH");
				else {
					char *provider = MusicManager.Get3DProviderName(m_nPrefsAudio3DProviderIndex);
					AsciiToUnicode(provider, gUString);
					textToPrint[MENUCOLUMN_RIGHT] = gUString;
				}
				break;
			case MENUACTION_SPEAKERCONF:
				if (m_nPrefsAudio3DProviderIndex == -1)
					textToPrint[MENUCOLUMN_RIGHT] = TheText.Get("FEA_NAH");
				else {
					switch (m_PrefsSpeakers) {
					case 0:
						textToPrint[MENUCOLUMN_RIGHT] = TheText.Get("FEA_2SP");
						break;
					case 1:
						textToPrint[MENUCOLUMN_RIGHT] = TheText.Get("FEA_EAR");
						break;
					case 2:
						textToPrint[MENUCOLUMN_RIGHT] = TheText.Get("FEA_4SP");
						break;
					};
				}
				break;
			case MENUACTION_CTRLMETHOD:
				switch (m_ControlMethod) {
				case 0:
					textToPrint[MENUCOLUMN_LEFT] = TheText.Get("FET_SCN");
					break;
				case 1:
					textToPrint[MENUCOLUMN_LEFT] = TheText.Get("FET_CCN");
					break;
				};
				break;
			case MENUACTION_DYNAMICACOUSTIC:
				textToPrint[MENUCOLUMN_RIGHT] = TheText.Get(m_PrefsDMA ? "FEM_ON" : "FEM_OFF");
				break;
			case MENUACTION_MOUSESTEER:
				textToPrint[MENUCOLUMN_RIGHT] = TheText.Get(m_bDisableMouseSteering ? "FEM_ON" : "FEM_OFF");
				break;				
			};

			CFont::SetDropShadowPosition(MENUDROP_COLOR_SIZE);
			CFont::SetDropColor(CRGBA(0, 0, 0, FadeIn(MENUDROP_COLOR_A)));
			CFont::SetCentreSize(SCREEN_WIDTH);
			CFont::SetWrapx(SCREEN_WIDTH);
			CFont::SetRightJustifyWrap(-SCREEN_WIDTH);

			// Set alignment.
			CVector2D vecPositions = { 0.0f, 0.0f };
			float fVerticalSpacing;
			float fBarSize;

			int SavePageSlot =
				m_nCurrScreen == MENUPAGE_CHOOSE_LOAD_SLOT ||
				m_nCurrScreen == MENUPAGE_CHOOSE_DELETE_SLOT ||
				m_nCurrScreen == MENUPAGE_CHOOSE_SAVE_SLOT;

			if (SavePageSlot) {
				CFont::SetFontStyle(FONT_BANK);
				CFont::SetAlignment(ALIGN_LEFT);
				CFont::SetScale(SCREEN_SCALE_X(0.45f), SCREEN_SCALE_Y(0.7f));
				fVerticalSpacing = MENUCOLUMN_SPACING_MIN;
				fBarSize = MENUSELECT_BOX_MIN;

				vecPositions.x = SCREEN_SCALE_X(MENUCOLUMN_SAVE_X);
				vecPositions.y = (SCREEN_HEIGHT / 2) - SCREEN_SCALE_Y(MENUCOLUMN_SAVE_Y);
			}
			else {
				CFont::SetFontStyle(FONT_HEADING);

				int LeftMenuColumn =
					m_nCurrScreen == MENUPAGE_SOUND_SETTINGS ||
					m_nCurrScreen == MENUPAGE_GRAPHICS_SETTINGS ||
					m_nCurrScreen == MENUPAGE_MOUSE_CONTROLS;

				if (LeftMenuColumn) {
					CFont::SetAlignment(ALIGN_LEFT);
					CFont::SetScale(SCREEN_SCALE_X(0.55f), SCREEN_SCALE_Y(0.8f));
					fVerticalSpacing = MENUCOLUMN_SPACING_MIN;
					fBarSize = MENUSELECT_BOX_MIN;
				}
				else {
					CFont::SetAlignment(ALIGN_CENTER);
					CFont::SetScale(SCREEN_SCALE_X(0.75f), SCREEN_SCALE_Y(0.9f));
					fVerticalSpacing = MENUCOLUMN_SPACING_MAX;
					fBarSize = MENUSELECT_BOX_MAX;
				}

				// Set positions.
				if (CFont::GetDetails().centre)
					vecPositions.x = SCREEN_WIDTH / 2;
				else
					vecPositions.x = SCREEN_SCALE_X(MENUCOLUMN_POS_X);

				switch (m_nCurrScreen) {
				case MENUPAGE_BRIEFS:
				case MENUPAGE_STATS:
					vecPositions.y = SCREEN_SCALE_FROM_BOTTOM(MENUCOLUMN_FEDS);
					break;
				case MENUPAGE_SOUND_SETTINGS:
					vecPositions.y = (SCREEN_HEIGHT / 2) - SCREEN_SCALE_Y(MENUCOLUMN_MAX_Y);

					if (i > 5)
						vecPositions.y += SCREEN_SCALE_Y(MENURADIO_ICON_H * 1.16f);
					break;
				case MENUPAGE_LANGUAGE_SETTINGS:
					vecPositions.y = (SCREEN_HEIGHT / 2) - SCREEN_SCALE_Y(MENUCOLUMN_MIN_Y);
					break;
				case MENUPAGE_MOUSE_CONTROLS:
				case MENUPAGE_GRAPHICS_SETTINGS:
					vecPositions.y = (SCREEN_HEIGHT / 2) - SCREEN_SCALE_Y(MENUCOLUMN_MAX_Y);
					break;
				case MENUPAGE_OPTIONS:
					vecPositions.y = (SCREEN_HEIGHT / 2) - SCREEN_SCALE_Y(MENUCOLUMN_MID_Y);
					break;
				case MENUPAGE_PAUSE_MENU:
					vecPositions.y = (SCREEN_HEIGHT / 2) - SCREEN_SCALE_Y(MENUCOLUMN_PAUSE_Y);
					break;
				case MENUPAGE_NEW_GAME:
					vecPositions.y = (SCREEN_HEIGHT / 2) - SCREEN_SCALE_Y(MENUCOLUMN_MID_Y);
					break;
				case MENUPAGE_START_MENU:
					vecPositions.y = (SCREEN_HEIGHT / 2) - SCREEN_SCALE_Y(MENUCOLUMN_START_Y);
					break;
				default:
					vecPositions.y = (SCREEN_HEIGHT / 2) - SCREEN_SCALE_Y(MENUCOLUMN_MID_Y);
					break;
				};
			}

			if (i > 0)
				vecPositions.y += SCREEN_SCALE_Y(fVerticalSpacing * i);

			// Set color and draw selection bar.
			if (i == m_nCurrOption && m_nMenuFadeAlpha >= 255) {
				CFont::SetColor(CRGBA(255, 217, 106, FadeIn(255)));
				CSprite2d::DrawRect(CRect(SCREEN_STRETCH_X(11.0f), vecPositions.y - SCREEN_STRETCH_Y(fBarSize * 0.13f), SCREEN_STRETCH_FROM_RIGHT(11.0f), vecPositions.y + SCREEN_STRETCH_Y(fBarSize)), CRGBA(100, 200, 50, 50));
			}
			else 
				CFont::SetColor(CRGBA(235, 170, 50, FadeIn(255)));

			// Draw
			if (textToPrint[MENUCOLUMN_LEFT])
				CFont::PrintString(vecPositions.x, vecPositions.y, textToPrint[MENUCOLUMN_LEFT]);

			if (textToPrint[MENUCOLUMN_RIGHT]) {
				if (Locked)
					CFont::SetColor(CRGBA(190, 130, 40, FadeIn(255)));

				CFont::SetAlignment(ALIGN_RIGHT);
				CFont::PrintString(SCREEN_SCALE_FROM_RIGHT(SavePageSlot ? MENUCOLUMN_SAVE_X : MENUCOLUMN_POS_X), vecPositions.y, textToPrint[MENUCOLUMN_RIGHT]);
			}

			// Mouse support. 
			// TODO: inputs for these pages.
			if (m_nCurrScreen == MENUPAGE_SKIN_SELECT) {
			}
			else if (m_nCurrScreen == MENUPAGE_KEYBOARD_CONTROLS) {

			}
			else {
				static bool bIsMouseInPosition = false;
				if (m_nMenuFadeAlpha >= 255 && GetMouseInput()) {
					CVector2D vecInputSize = { SCREEN_SCALE_X(20.0f), SCREEN_SCALE_FROM_RIGHT(20.0f) };
					if (m_bShowMouse &&
						((CheckHover(vecInputSize.x, vecInputSize.y, vecPositions.y, vecPositions.y + SCREEN_STRETCH_Y(20.0f)))))
						bIsMouseInPosition = true;
					else
						bIsMouseInPosition = false;

					if (bIsMouseInPosition) {
						if (m_nCurrOption != i) {
							m_nCurrOption = i;
							DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_DENIED, 0);
						}

						m_nPrevOption = m_nCurrOption;

						if (GetMouseForward())
							m_nHoverOption = IGNORE_OPTION;
						else
							m_nHoverOption = ACTIVATE_OPTION;
					}
				}
			}

			// Sliders 
			// TODO: CheckHover
			switch (aScreens[m_nCurrScreen].m_aEntries[i].m_Action) {
			case MENUACTION_BRIGHTNESS:
				DisplaySlider(SCREEN_SCALE_FROM_RIGHT(MENUSLIDER_X), vecPositions.y - SCREEN_SCALE_Y(3.0f), SCREEN_SCALE_Y(2.0f), SCREEN_SCALE_Y(18.0f), SCREEN_SCALE_X(256.0f), m_PrefsBrightness/512.0f);
				break;
			case MENUACTION_DRAWDIST:
				DisplaySlider(SCREEN_SCALE_FROM_RIGHT(MENUSLIDER_X), vecPositions.y - SCREEN_SCALE_Y(3.0f), SCREEN_SCALE_Y(2.0f), SCREEN_SCALE_Y(18.0f), SCREEN_SCALE_X(256.0f), (m_PrefsLOD - 0.8f) * 1.0f);
				break;
			case MENUACTION_MUSICVOLUME:
				DisplaySlider(SCREEN_SCALE_FROM_RIGHT(MENUSLIDER_X), vecPositions.y - SCREEN_SCALE_Y(3.0f), SCREEN_SCALE_Y(2.0f), SCREEN_SCALE_Y(18.0f), SCREEN_SCALE_X(256.0f), m_PrefsMusicVolume/128.0f);
				break;
			case MENUACTION_SFXVOLUME:
				DisplaySlider(SCREEN_SCALE_FROM_RIGHT(MENUSLIDER_X), vecPositions.y - SCREEN_SCALE_Y(3.0f), SCREEN_SCALE_Y(2.0f), SCREEN_SCALE_Y(18.0f), SCREEN_SCALE_X(256.0f), m_PrefsSfxVolume/128.0f);
				break;
			case MENUACTION_MOUSESENS:
				DisplaySlider(SCREEN_SCALE_FROM_RIGHT(MENUSLIDER_X), vecPositions.y - SCREEN_SCALE_Y(3.0f), SCREEN_SCALE_Y(2.0f), SCREEN_SCALE_Y(18.0f), SCREEN_SCALE_X(256.0f), TheCamera.m_fMouseAccelHorzntl * 200.0f);
				break;
			};

			// Radio icons.
			float fIconSpacing = 59.52f;
			if (m_nCurrScreen == MENUPAGE_SOUND_SETTINGS) {
				for (int i = 0; i < POLICE_RADIO; i++) {
#ifndef MID_RADIOICONS
					if (i < USERTRACK)
						m_aFrontEndSprites[i + FE_RADIO1].Draw(SCREEN_STRETCH_X(MENURADIO_ICON_X + fIconSpacing * i), (SCREEN_HEIGHT / 2) - SCREEN_SCALE_Y(MENURADIO_ICON_Y), SCREEN_SCALE_X(MENURADIO_ICON_W), SCREEN_SCALE_Y(MENURADIO_ICON_H), i == m_PrefsRadioStation ? CRGBA(255, 255, 255, 255) : CRGBA(225, 0, 0, 170));
					if (i > CHATTERBOX && DMAudio.IsMP3RadioChannelAvailable())
						m_aMenuSprites[MENUSPRITE_MP3LOGO].Draw(SCREEN_STRETCH_X(MENURADIO_ICON_X + fIconSpacing * i), (SCREEN_HEIGHT / 2) - SCREEN_SCALE_Y(MENURADIO_ICON_Y), SCREEN_SCALE_X(MENURADIO_ICON_W), SCREEN_SCALE_Y(MENURADIO_ICON_H), i == m_PrefsRadioStation ? CRGBA(255, 255, 255, 255) : CRGBA(225, 0, 0, 170));
#else
					float fMp3Pos = 0.0f;
					if (DMAudio.IsMP3RadioChannelAvailable())
						fMp3Pos = 34.0f;

					if (i < USERTRACK)
						m_aFrontEndSprites[i + FE_RADIO1].Draw((SCREEN_WIDTH * 0.5) + SCREEN_SCALE_X(-fMp3Pos + MENURADIO_ICON_X + (fIconSpacing * i)), (SCREEN_HEIGHT / 2) - SCREEN_SCALE_Y(MENURADIO_ICON_Y), SCREEN_SCALE_X(MENURADIO_ICON_W), SCREEN_SCALE_Y(MENURADIO_ICON_H), i == m_PrefsRadioStation ? CRGBA(255, 255, 255, 255) : CRGBA(225, 0, 0, 170));
					if (i > CHATTERBOX && DMAudio.IsMP3RadioChannelAvailable())
						m_aMenuSprites[MENUSPRITE_MP3LOGO].Draw((SCREEN_WIDTH * 0.5) + SCREEN_SCALE_X(-fMp3Pos + MENURADIO_ICON_X + (fIconSpacing * i)), (SCREEN_HEIGHT / 2) - SCREEN_SCALE_Y(MENURADIO_ICON_Y), SCREEN_SCALE_X(MENURADIO_ICON_W), SCREEN_SCALE_Y(MENURADIO_ICON_H), i == m_PrefsRadioStation ? CRGBA(255, 255, 255, 255) : CRGBA(225, 0, 0, 170));
#endif
				}
			}

			// Helpers
			if (!strcmp(aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption].m_EntryName, "FED_RES")) {
				if (m_nDisplayVideoMode == m_nPrefsVideoMode) {
					if (m_nHelperTextMsgId == 1)
						ResetHelperText();
				}
				else
					SetHelperText(1);
			}
			else {
				if (m_nDisplayVideoMode != m_nPrefsVideoMode) {
					m_nDisplayVideoMode = m_nPrefsVideoMode;
					SetHelperText(3);
				}
			}

			switch (m_nCurrScreen) {
			case MENUPAGE_CONTROLLER_SETTINGS:
			case MENUPAGE_SOUND_SETTINGS:
			case MENUPAGE_GRAPHICS_SETTINGS:
			case MENUPAGE_SKIN_SELECT:
			case MENUPAGE_CONTROLLER_PC:
			case MENUPAGE_MOUSE_CONTROLS:
				DisplayHelperText();
				break;
			};
		}
	};
}
#endif

#if 1
WRAPPER void CMenuManager::DrawControllerBound(int, int, int, uint8) { EAXJMP(0x489710); }
#else
void CMenuManager::DrawControllerBound(int, int, int, uint8)
{ 

}
#endif

#if 1
WRAPPER void CMenuManager::DrawControllerScreenExtraText(int, int, int) { EAXJMP(0x4892F0); }
#else
void CMenuManager::DrawControllerScreenExtraText(int, int, int)
{

}
#endif

#if 1
WRAPPER void CMenuManager::DrawControllerSetupScreen() { EAXJMP(0x481210); }
#else
void CMenuManager::DrawControllerSetupScreen()
{

}
#endif

#if 0
WRAPPER void CMenuManager::DrawFrontEnd(void) { EAXJMP(0x47A540); }
#else
void CMenuManager::DrawFrontEnd()
{
	CFont::SetAlphaFade(255.0f);

	if (m_nCurrScreen == MENUPAGE_NONE) {
		m_nMenuFadeAlpha = 0;

		if (m_bGameNotLoaded)
			m_nCurrScreen = MENUPAGE_START_MENU;
		else
			m_nCurrScreen = MENUPAGE_PAUSE_MENU;
	}

	if (!m_nCurrOption && aScreens[m_nCurrScreen].m_aEntries[0].m_Action == MENUACTION_LABEL)
		m_nCurrOption = MENUROW_1;

	CMenuManager::DrawFrontEndNormal();
	CMenuManager::PrintErrorMessage();
}
#endif

#if 0
WRAPPER void CMenuManager::DrawFrontEndNormal(void) { EAXJMP(0x47A5B0); }
#else
void CMenuManager::DrawFrontEndNormal()
{
	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void *)TRUE);
	RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void*)rwFILTERLINEAR);
	RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void *)FALSE);
	RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void *)FALSE);
	RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)rwBLENDSRCALPHA);
	RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)rwBLENDINVSRCALPHA);
	RwRenderStateSet(rwRENDERSTATETEXTUREADDRESS, (void *)rwTEXTUREADDRESSCLAMP);

	CSprite2d::InitPerFrame();
	CFont::InitPerFrame();

	eMenuSprites previousSprite = MENUSPRITE_MAINMENU;
	if (m_nMenuFadeAlpha < 255) {
		switch (m_nPrevScreen) {
		case MENUPAGE_STATS:
		case MENUPAGE_START_MENU:
		case MENUPAGE_PAUSE_MENU:
			previousSprite = MENUSPRITE_MAINMENU;
			break;
		case MENUPAGE_NEW_GAME:
		case MENUPAGE_CHOOSE_LOAD_SLOT:
		case MENUPAGE_CHOOSE_DELETE_SLOT:
		case MENUPAGE_NEW_GAME_RELOAD:
		case MENUPAGE_LOAD_SLOT_CONFIRM:
		case MENUPAGE_DELETE_SLOT_CONFIRM:
		case MENUPAGE_EXIT:
			previousSprite = MENUSPRITE_SINGLEPLAYER;
			break;
		case MENUPAGE_MULTIPLAYER_MAIN:
			previousSprite = MENUSPRITE_MULTIPLAYER;
			break;
		case MENUPAGE_MULTIPLAYER_MAP:
		case MENUPAGE_MULTIPLAYER_FIND_GAME:
		case MENUPAGE_SKIN_SELECT:
		case MENUPAGE_KEYBOARD_CONTROLS:
		case MENUPAGE_MOUSE_CONTROLS:
			previousSprite = MENUSPRITE_FINDGAME;
			break;
		case MENUPAGE_MULTIPLAYER_CONNECTION:
		case MENUPAGE_MULTIPLAYER_MODE:
			previousSprite = MENUSPRITE_CONNECTION;
			break;
		case MENUPAGE_MULTIPLAYER_CREATE:
			previousSprite = MENUSPRITE_HOSTGAME;
			break;
		case MENUPAGE_SKIN_SELECT_OLD:
		case MENUPAGE_OPTIONS:
			previousSprite = MENUSPRITE_PLAYERSET;
			break;
		};

		if (m_nPrevScreen == MENUPAGE_NONE)
			CSprite2d::DrawRect(CRect(0.0f, 0.0f, SCREENW, SCREENH), CRGBA(0, 0, 0, 255));
		else
			m_aMenuSprites[previousSprite].Draw(CRect(0.0f, 0.0f, SCREENW, SCREENH), CRGBA(255, 255, 255, 255));
	}

	eMenuSprites currentSprite = MENUSPRITE_MAINMENU;
	switch (m_nCurrScreen) {
	case MENUPAGE_STATS:
	case MENUPAGE_START_MENU:
	case MENUPAGE_PAUSE_MENU:
		currentSprite = MENUSPRITE_MAINMENU;
		break;
	case MENUPAGE_NEW_GAME:
	case MENUPAGE_CHOOSE_LOAD_SLOT:
	case MENUPAGE_CHOOSE_DELETE_SLOT:
	case MENUPAGE_NEW_GAME_RELOAD:
	case MENUPAGE_LOAD_SLOT_CONFIRM:
	case MENUPAGE_DELETE_SLOT_CONFIRM:
	case MENUPAGE_EXIT:
		currentSprite = MENUSPRITE_SINGLEPLAYER;
		break;
	case MENUPAGE_MULTIPLAYER_MAIN:
		currentSprite = MENUSPRITE_MULTIPLAYER;
		break;
	case MENUPAGE_MULTIPLAYER_MAP:
	case MENUPAGE_MULTIPLAYER_FIND_GAME:
	case MENUPAGE_SKIN_SELECT:
	case MENUPAGE_KEYBOARD_CONTROLS:
	case MENUPAGE_MOUSE_CONTROLS:
		currentSprite = MENUSPRITE_FINDGAME;
		break;
	case MENUPAGE_MULTIPLAYER_CONNECTION:
	case MENUPAGE_MULTIPLAYER_MODE:
		currentSprite = MENUSPRITE_CONNECTION;
		break;
	case MENUPAGE_MULTIPLAYER_CREATE:
		currentSprite = MENUSPRITE_HOSTGAME;
		break;
	case MENUPAGE_SKIN_SELECT_OLD:
	case MENUPAGE_OPTIONS:
		currentSprite = MENUSPRITE_PLAYERSET;
		break;
	};

	uint32 savedShade;
	uint32 savedAlpha;
	RwRenderStateGet(rwRENDERSTATESHADEMODE, &savedShade);
	RwRenderStateSet(rwRENDERSTATESHADEMODE, reinterpret_cast<void *>(rwSHADEMODEGOURAUD));
	RwRenderStateGet(rwRENDERSTATEVERTEXALPHAENABLE, &savedAlpha);
	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, reinterpret_cast<void *>(TRUE));
	if (m_nMenuFadeAlpha >= 255) {
		m_aMenuSprites[currentSprite].Draw(CRect(0.0f, 0.0f, SCREENW, SCREENH), CRGBA(255, 255, 255, 255));
	}
	else {
		if (m_nMenuFadeAlpha < 255) {
			m_nMenuFadeAlpha += 0.1f * 255.0f;

			if (m_nMenuFadeAlpha >= 255)
				m_nMenuFadeAlpha = 255;

			m_aMenuSprites[currentSprite].Draw(CRect(0.0f, 0.0f, SCREENW, SCREENH), CRGBA(255, 255, 255, m_nMenuFadeAlpha));
		}
		else 
			m_aMenuSprites[currentSprite].Draw(CRect(0.0f, 0.0f, SCREENW, SCREENH), CRGBA(255, 255, 255, 255));
	}

	// GTA LOGO
	if (m_nCurrScreen == MENUPAGE_START_MENU || m_nCurrScreen == MENUPAGE_PAUSE_MENU) {
		if (CGame::frenchGame || CGame::germanGame || !CGame::nastyGame)
			m_aMenuSprites[MENUSPRITE_GTA3LOGO].Draw(CRect((SCREEN_WIDTH / 2) - SCREEN_SCALE_X(115.0f), SCREEN_SCALE_Y(70.0f), (SCREEN_WIDTH / 2) + SCREEN_SCALE_X(115.0f), SCREEN_SCALE_Y(180.0f)), CRGBA(255, 255, 255, FadeIn(255)));
		else
			m_aMenuSprites[MENUSPRITE_GTALOGO].Draw(CRect((SCREEN_WIDTH / 2) - SCREEN_SCALE_X(95.0f), SCREEN_SCALE_Y(40.0f), (SCREEN_WIDTH / 2) + SCREEN_SCALE_X(95.0f), SCREEN_SCALE_Y(210.0f)), CRGBA(255, 255, 255, FadeIn(255)));
	}
	RwRenderStateSet(rwRENDERSTATESHADEMODE, reinterpret_cast<void *>(savedShade));
	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, reinterpret_cast<void *>(savedAlpha));

	switch (m_nCurrScreen) {
	case MENUPAGE_SKIN_SELECT:
		CMenuManager::DrawPlayerSetupScreen();
		break;
	case MENUPAGE_KEYBOARD_CONTROLS:
		CMenuManager::DrawControllerSetupScreen();
		break;
	default:
		CMenuManager::Draw();
		break;
	};

	CFont::DrawFonts();

	// Draw mouse
	if (m_bShowMouse)
		m_aMenuSprites[MENUSPRITE_MOUSE].Draw(m_nMousePosX, m_nMousePosY, SCREEN_SCALE_X(60.0f), SCREEN_SCALE_Y(60.0f), CRGBA(255, 255, 255, 255));
}
#endif

#if 1
WRAPPER void CMenuManager::DrawPlayerSetupScreen() { EAXJMP(0x47F2B0); }
#else
void CMenuManager::DrawPlayerSetupScreen()
{

}
#endif

#if 0
WRAPPER int CMenuManager::FadeIn(int alpha) { EAXJMP(0x48AC60); }
#else
int CMenuManager::FadeIn(int alpha)
{
	if (m_nCurrScreen == MENUPAGE_LOADING_IN_PROGRESS ||
		m_nCurrScreen == MENUPAGE_SAVING_IN_PROGRESS ||
		m_nCurrScreen == MENUPAGE_DELETING)
		return alpha;

	if (m_nMenuFadeAlpha >= alpha)
		return alpha;

	return m_nMenuFadeAlpha;
}
#endif

#if 1
WRAPPER void CMenuManager::FilterOutColorMarkersFromString(uint16, CRGBA &) { EAXJMP(0x4889C0); }
#else
void CMenuManager::FilterOutColorMarkersFromString(uint16, CRGBA &)
{

}
#endif

#if 1
WRAPPER int CMenuManager::GetStartOptionsCntrlConfigScreens() { EAXJMP(0x489270); }
#else
int CMenuManager::GetStartOptionsCntrlConfigScreens()
{

}
#endif

#if 0
WRAPPER void CMenuManager::InitialiseChangedLanguageSettings() { EAXJMP(0x47A4D0); }
#else
void CMenuManager::InitialiseChangedLanguageSettings()
{
	if (m_bFrontEnd_ReloadObrTxtGxt) {
		m_bFrontEnd_ReloadObrTxtGxt = false;
		CTimer::Stop();
		TheText.Unload();
		TheText.Load();
		CTimer::Update();
		CGame::frenchGame = false;
		CGame::germanGame = false;
		switch (CMenuManager::m_PrefsLanguage) {
		case LANGUAGE_FRENCH:
			CGame::frenchGame = true;
			break;
		case LANGUAGE_GERMAN:
			CGame::germanGame = true;
			break;
		default:
			break;
		};
	}
}
#endif

#if 0
WRAPPER void CMenuManager::LoadAllTextures() { EAXJMP(0x47A230); }
#else
void CMenuManager::LoadAllTextures()
{
	if (!m_bSpritesLoaded) {
		CMenuManager::CentreMousePointer();
		DMAudio.ChangeMusicMode(0);
		DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_STARTING, 0);
		m_nCurrOption = MENUROW_0;
		m_PrefsRadioStation = DMAudio.GetRadioInCar();

		if (DMAudio.IsMP3RadioChannelAvailable()) {
			if (CMenuManager::m_PrefsRadioStation > USERTRACK)
				CMenuManager::m_PrefsRadioStation = CGeneral::GetRandomNumber() % 10;
		}
		else if (CMenuManager::m_PrefsRadioStation > CHATTERBOX)
			CMenuManager::m_PrefsRadioStation = CGeneral::GetRandomNumber() % 9;
	
		CFileMgr::SetDir("");
		CTimer::Stop();
		CStreaming::MakeSpaceFor(700 * 1024);
		CStreaming::ImGonnaUseStreamingMemory();
		CTxdStore::PushCurrentTxd();

		int frontend = CTxdStore::AddTxdSlot("frontend");
		CTxdStore::LoadTxd(frontend, "MODELS/FRONTEND.TXD");
		CTxdStore::AddRef(frontend);
		CTxdStore::SetCurrentTxd(frontend);
		CStreaming::IHaveUsedStreamingMemory();
		CTimer::Update();

		debug("LOAD frontend\n");
		for (int i = 0; i < ARRAY_SIZE(FrontendFilenames); i++) {
			m_aFrontEndSprites[i].SetTexture(FrontendFilenames[i]);
			m_aFrontEndSprites[i].SetAddressing(rwTEXTUREADDRESSBORDER);
		};

		CTxdStore::PopCurrentTxd();

		int menu = CTxdStore::AddTxdSlot("menu");
		CTxdStore::LoadTxd(menu, "MODELS/MENU.TXD");
		CTxdStore::AddRef(menu);
		CTxdStore::SetCurrentTxd(menu);

		debug("LOAD sprite\n");
		for (int i = 0; i < ARRAY_SIZE(MenuFilenames)/2; i++) {
			m_aMenuSprites[i].SetTexture(MenuFilenames[i*2], MenuFilenames[i*2+1]);
			m_aMenuSprites[i].SetAddressing(rwTEXTUREADDRESSBORDER);
		};

		CTxdStore::PopCurrentTxd();

		m_bSpritesLoaded = true;
	}
}
#endif

#if 0
WRAPPER void CMenuManager::LoadSettings() { EAXJMP(0x488EE0); }
#else
void CMenuManager::LoadSettings()
{

	CFileMgr::SetDirMyDocuments();

	uint8 prevLang = m_PrefsLanguage;
	MousePointerStateHelper.bInvertVertically = true;

	static char Ver;
	int fileHandle = CFileMgr::OpenFile("gta3.set", "r");
	if (fileHandle) {
		CFileMgr::Read(fileHandle, buf(&Ver), sizeof(Ver));

		if (strncmp(&Ver, "THIS FILE IS NOT VALID YET", 26)) {
			CFileMgr::Seek(fileHandle, 0, 0);
			ControlsManager.LoadSettings(fileHandle);
			CFileMgr::Read(fileHandle, buf(&gString), 20);
			CFileMgr::Read(fileHandle, buf(&gString), 20);
			CFileMgr::Read(fileHandle, buf(&gString), 4);
			CFileMgr::Read(fileHandle, buf(&gString), 4);
			CFileMgr::Read(fileHandle, buf(&gString), 1);
			CFileMgr::Read(fileHandle, buf(&gString), 1);
			CFileMgr::Read(fileHandle, buf(&gString), 1);
			CFileMgr::Read(fileHandle, buf(&TheCamera.m_bHeadBob), 1);
			CFileMgr::Read(fileHandle, buf(&TheCamera.m_fMouseAccelHorzntl), 4);
			CFileMgr::Read(fileHandle, buf(&TheCamera.m_fMouseAccelVertical), 4);
			CFileMgr::Read(fileHandle, buf(&MousePointerStateHelper.bInvertVertically), 1);
			CFileMgr::Read(fileHandle, buf(&CVehicle::m_bDisableMouseSteering), 1);
			CFileMgr::Read(fileHandle, buf(&m_PrefsSfxVolume), 1);
			CFileMgr::Read(fileHandle, buf(&m_PrefsMusicVolume), 1);
			CFileMgr::Read(fileHandle, buf(&m_PrefsRadioStation), 1);
			CFileMgr::Read(fileHandle, buf(&m_PrefsSpeakers), 1);
			CFileMgr::Read(fileHandle, buf(&m_nPrefsAudio3DProviderIndex), 1);
			CFileMgr::Read(fileHandle, buf(&m_PrefsDMA), 1);
			CFileMgr::Read(fileHandle, buf(&m_PrefsBrightness), 1);
			CFileMgr::Read(fileHandle, buf(&m_PrefsLOD), 4);
			CFileMgr::Read(fileHandle, buf(&m_PrefsShowSubtitles), 1);
			CFileMgr::Read(fileHandle, buf(&m_PrefsUseWideScreen), 1);
			CFileMgr::Read(fileHandle, buf(&m_PrefsVsyncDisp), 1);
			CFileMgr::Read(fileHandle, buf(&m_PrefsFrameLimiter), 1);
			CFileMgr::Read(fileHandle, buf(&m_nDisplayVideoMode), 1);
			CFileMgr::Read(fileHandle, buf(&CMBlur::BlurOn), 1);
			CFileMgr::Read(fileHandle, buf(m_PrefsSkinFile), 256);
			CFileMgr::Read(fileHandle, buf(&m_ControlMethod), 1);
			CFileMgr::Read(fileHandle, buf(&m_PrefsLanguage), 1);
		}
	}

	CFileMgr::CloseFile(fileHandle);
	CFileMgr::SetDir("");

#ifndef HOR_PLUS
#else
	if (m_PrefsUseWideScreen < AR_AUTO || m_PrefsUseWideScreen > AR_64_27)
		m_PrefsUseWideScreen = AR_AUTO;
#endif

	m_PrefsVsync = m_PrefsVsyncDisp;
	lodMultiplier = m_PrefsLOD;

	if (m_nPrefsAudio3DProviderIndex == -1)
		m_nPrefsAudio3DProviderIndex = -2;

	if (m_PrefsLanguage == prevLang)
		m_bLanguageLoaded = false;
	else {
		m_bLanguageLoaded = true;
		TheText.Unload();
		TheText.Load();
		m_bFrontEnd_ReloadObrTxtGxt = true;
		InitialiseChangedLanguageSettings();

		debug("The previously saved language is now in use");
	}

	/*struct _WIN32_FIND_DATAA FindFileData;
	HANDLE H = FindFirstFileA("skins\*.bmp", &FindFileData);
	char Dest;
	bool SkinFound = false;

	for (int i = 1; H != (HANDLE)-1 && i; i = FindNextFileA(H, &FindFileData)) {
		strcpy(&Dest, buf(m_PrefsSkinFile));
		strcat(&Dest, ".bmp");
		if (!strcmp(FindFileData.cFileName, &Dest))
			SkinFound = true;
	}

	FindClose(H);

	if (!SkinFound) {
		debug("Default skin set as no other skins are available OR saved skin not found!");
		strcpy((char *)CMenuManager::m_PrefsSkinFile, "$$\"\"");
		strcpy(m_aSkinName, "$$\"\"");
	}*/
}
#endif

#if 1
WRAPPER void CMenuManager::MessageScreen(char *) { EAXJMP(0x48B7E0); }
#else
void CMenuManager::MessageScreen(char *)
{

}
#endif

#if 1
WRAPPER void CMenuManager::PickNewPlayerColour() { EAXJMP(0x488C40); }
#else
void CMenuManager::PickNewPlayerColour()
{

}
#endif

#if 1
WRAPPER void CMenuManager::PrintBriefs() { EAXJMP(0x484D60); }
#else
void CMenuManager::PrintBriefs()
{

}
#endif

#if 0
WRAPPER void CMenuManager::PrintErrorMessage() { EAXJMP(0x484F70); }
#else
void CMenuManager::PrintErrorMessage()
{
	if (!CPad::bDisplayNoControllerMessage && !CPad::bObsoleteControllerMessage)
		return;

	CSprite2d::DrawRect(CRect(SCREEN_SCALE_X(20.0f), SCREEN_SCALE_Y(140.0f), SCREEN_WIDTH - SCREEN_SCALE_X(20.0f), SCREEN_HEIGHT - SCREEN_SCALE_Y(140.0f)), CRGBA(64, 16, 16, 224));
	CFont::SetFontStyle(FONT_BANK);
	CFont::SetBackgroundOff();
	CFont::SetPropOn();
	CFont::SetCentreOff();
	CFont::SetJustifyOn();
	CFont::SetRightJustifyOff();
	CFont::SetBackGroundOnlyTextOn();
	CFont::SetWrapx(SCREEN_WIDTH - 40.0f);
	CFont::SetColor(CRGBA(165, 165, 165, 255));
	CFont::SetScale(SCREEN_SCALE_X(0.9f), SCREEN_SCALE_Y(0.9f));
	CFont::PrintString(SCREEN_SCALE_X(40.0f), (SCREEN_HEIGHT / 2) - SCREEN_SCALE_Y(60.0f), TheText.Get(CPad::bDisplayNoControllerMessage ? "NOCONT" : "WRCONT"));
	CFont::DrawFonts();
}
#endif

#if 1
WRAPPER void CMenuManager::PrintStats() { EAXJMP(0x482100); }
#else
void CMenuManager::PrintStats()
{

}
#endif

#if 0
WRAPPER void CMenuManager::Process(void) { EAXJMP(0x485100); }
#else
void CMenuManager::Process(void)
{
	if (m_bSaveMenuActive && TheCamera.GetScreenFadeStatus())
		return;

	field_113 = 0;
	InitialiseChangedLanguageSettings();

	SwitchMenuOnAndOff();

	if (m_bMenuActive) {
		LoadAllTextures();

		if (m_nCurrScreen == MENUPAGE_DELETING) {
			bool SlotPopulated = false;

			if (PcSaveHelper.DeleteSlot(m_nCurrSaveSlot)) {
				PcSaveHelper.PopulateSlotInfo();
				SlotPopulated = true;
			}

			if (SlotPopulated) {
				m_nPrevScreen = m_nCurrScreen;
				m_nCurrScreen = MENUPAGE_DELETE_SUCCESS;
				m_nCurrOption = 0;
				m_nScreenChangeDelayTimer = CTimer::GetTimeInMillisecondsPauseMode();
			}
			else
				SaveLoadFileError_SetUpErrorScreen();
		}
		if (m_nCurrScreen == MENUPAGE_SAVING_IN_PROGRESS) {
			int8 SaveSlot = PcSaveHelper.SaveSlot(m_nCurrSaveSlot);
			PcSaveHelper.PopulateSlotInfo();
			if (SaveSlot) {
				m_nPrevScreen = m_nCurrScreen;
				m_nCurrScreen = MENUPAGE_SAVE_SUCCESSFUL;
				m_nCurrOption = 0;
				m_nScreenChangeDelayTimer = CTimer::GetTimeInMillisecondsPauseMode();
			}
			else
				SaveLoadFileError_SetUpErrorScreen();
		}
		if (m_nCurrScreen == MENUPAGE_LOADING_IN_PROGRESS) {
			if (CheckSlotDataValid(m_nCurrSaveSlot)) {
				TheCamera.m_bUseMouse3rdPerson = m_ControlMethod == 0;
				if (m_PrefsVsyncDisp != m_PrefsVsync)
					m_PrefsVsync = m_PrefsVsyncDisp;
				DMAudio.Service();
				m_bStartGameLoading = 1;
				RequestFrontEndShutdown();
				m_bLoadingSavedGame = 1;
				b_FoundRecentSavedGameWantToLoad = 1;
				DMAudio.SetEffectsFadeVol(0);
				DMAudio.SetMusicFadeVol(0);
				DMAudio.ResetTimers(CTimer::GetTimeInMilliseconds());
			}
			else
				SaveLoadFileError_SetUpErrorScreen();
		}

		ProcessButtonPresses();
	}
	else {
		if (GetPadExitEnter())
			RequestFrontEndStartUp();

		UnloadTextures();
		m_nPrevScreen = MENUPAGE_NONE;
		m_nCurrScreen = m_nPrevScreen;
		m_nCurrOption = MENUROW_0;
	}
}
#endif

#if 0
WRAPPER void CMenuManager::ProcessButtonPresses() { EAXJMP(0x4856F0); }
#else
void CMenuManager::ProcessButtonPresses()
{
	// Update Mouse Position
	m_nMouseOldPosX = m_nMousePosX;
	m_nMouseOldPosY = m_nMousePosY;

	m_nMousePosX = m_nMouseTempPosX;
	m_nMousePosY = m_nMouseTempPosY;

	if (m_nMousePosX < 0)
		m_nMousePosX = 0;
	if (m_nMousePosX > SCREEN_WIDTH)
		m_nMousePosX = SCREEN_WIDTH;
	if (m_nMousePosY < 0)
		m_nMousePosY = 0;
	if (m_nMousePosY > SCREEN_HEIGHT)
		m_nMousePosY = SCREEN_HEIGHT;

	// Show/hide mouse cursor.
	if (GetMouseInput())
		m_bShowMouse = true;
	else if (GetPadInput())
		m_bShowMouse = false;

	// Get number of menu options.
	uint8 NumberOfMenuOptions = GetNumberOfMenuOptions();

	// Select next/previous option with pad. Mouse is done in drawing function.
	if (GetPadMoveUp()) {
		m_nPrevOption = m_nCurrOption;
		m_nCurrOption -= 1;

		if (aScreens[m_nCurrScreen].m_aEntries[0].m_Action == MENUACTION_LABEL) {
			if (m_nCurrOption < MENUROW_1)
				m_nCurrOption = NumberOfMenuOptions;
		}
		else {
			if (m_nCurrOption < MENUROW_0)
				m_nCurrOption = NumberOfMenuOptions;
		}

		DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_DENIED, 0);
	}
	else if (GetPadMoveDown()) {
		m_nPrevOption = m_nCurrOption;
		m_nCurrOption += 1;

		if (aScreens[m_nCurrScreen].m_aEntries[0].m_Action == MENUACTION_LABEL) {
			if (m_nCurrOption > NumberOfMenuOptions)
				m_nCurrOption = MENUROW_1;
		}
		else {
			if (m_nCurrOption > NumberOfMenuOptions)
				m_nCurrOption = MENUROW_0;
		}

		DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_DENIED, 0);
	}

	// Set what happens if ESC is pressed.
	if (GetPadBack()) {
		bool PlayEscSound = false;
		switch (m_nCurrScreen) {
		case MENUPAGE_START_MENU:
			break;
		case MENUPAGE_CHOOSE_SAVE_SLOT:
		case MENUPAGE_PAUSE_MENU:
			RequestFrontEndShutdown();
			PlayEscSound = true;
			break;
		default:
			SwitchToNewScreen(aScreens[m_nCurrScreen].m_PreviousPage[0]);
			PlayEscSound = true;
			break;
		};

		if (PlayEscSound)
			DMAudio.PlayFrontEndSound(SOUND_FRONTEND_EXIT, 0);
	}

	// TODO: finish hover options.
	// Set mouse buttons.
	if (GetMouseForward()) {
		switch (m_nHoverOption) {
		case ACTIVATE_OPTION:
			if (m_nCurrOption || m_nCurrScreen != MENUPAGE_PAUSE_MENU)
				m_nCurrOption = m_nPrevOption;

			m_nHoverOption = ACTIVATE_OPTION;
			break;
		default:
			break;
		};
	}

	// Process all menu options here, but first check if it's an option or a redirect.
	int32 CurrAction = aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption].m_Action;
	if ((GetPadForward() || GetMouseForward()) ||
		((GetPadMoveLeft() || GetMouseMoveRight()) || (GetPadMoveRight() || GetMouseMoveLeft())) &&
		(aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption].m_TargetMenu == m_nCurrScreen &&
		CurrAction != MENUACTION_CHANGEMENU &&
		CurrAction != MENUACTION_LOADRADIO &&
		CurrAction != MENUACTION_RESTOREDEF &&
		CurrAction != MENUACTION_PLAYERSETUP)) {

		if (!strcmp(aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption].m_EntryName, "FEDS_TB"))
			DMAudio.PlayFrontEndSound(SOUND_FRONTEND_EXIT, 0);
		else
			DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_SUCCESS, 0);

		ProcessOnOffMenuOptions();
	}

	// Process screens that may redirect you somewhere, or may not.
	switch (m_nCurrScreen) {
	case MENUPAGE_LOAD_SLOT_CONFIRM:
		break;
	case MENUPAGE_NEW_GAME_RELOAD:
		if (m_bGameNotLoaded)
			DoSettingsBeforeStartingAGame();
		break;
	case MENUPAGE_CHOOSE_DELETE_SLOT:
	case MENUPAGE_CHOOSE_SAVE_SLOT:
	case MENUPAGE_CHOOSE_LOAD_SLOT:
		PcSaveHelper.PopulateSlotInfo();
		break;
	default:
		break;
	};

	// Reset pad shaking.
	if (VibrationTime != 0) {
		if (CTimer::GetTimeInMillisecondsPauseMode() > VibrationTime) {
			CPad::GetPad(0)->StopShaking(0);
			VibrationTime = 0;
		}
	}
}
#endif

#if 0
WRAPPER void CMenuManager::ProcessOnOffMenuOptions() { EAXJMP(0x48AE60); }
#else
void CMenuManager::ProcessOnOffMenuOptions()
{
	int8 InputDirection = (GetPadMoveLeft() || GetMouseMoveLeft()) && (!GetPadForward() && !GetMouseForward()) ? -1 : 1;
	int8 InputEnter = GetPadForward();

	uint8 NumberOfMenuOptions = GetNumberOfMenuOptions();

	// In numerical order.
	switch (aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption].m_Action) {
	case MENUACTION_CHANGEMENU:
		SwitchToNewScreen(aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption].m_TargetMenu);
		break;
	case MENUACTION_CTRLVIBRATION:
		if (!m_PrefsUseVibration)
			m_PrefsUseVibration = true;

		if (m_PrefsUseVibration) {
			CPad::GetPad(0)->StartShake(350, 150);
			VibrationTime = CTimer::GetTimeInMillisecondsPauseMode() + 500;
		}
		SaveSettings();
		break;
	case MENUACTION_FRAMESYNC:
		m_PrefsVsync = m_PrefsVsync == false;
		SaveSettings();
		break;
	case MENUACTION_FRAMELIMIT:
		m_PrefsFrameLimiter = m_PrefsFrameLimiter == false;
		SaveSettings();
		break;
	case MENUACTION_TRAILS:
		CMBlur::BlurOn = CMBlur::BlurOn == false;
		if (CMBlur::BlurOn)
			CMBlur::MotionBlurOpen(Scene.camera);
		else
			CMBlur::MotionBlurClose();

		SaveSettings();
		break;
	case MENUACTION_SUBTITLES:
		m_PrefsShowSubtitles = m_PrefsShowSubtitles == false;
		SaveSettings();
		break;
	case MENUACTION_WIDESCREEN:
#ifndef HOR_PLUS
		m_PrefsUseWideScreen = m_PrefsUseWideScreen == false;
#else
		if (InputDirection < 0) {
			switch (m_PrefsUseWideScreen) {
			case AR_AUTO:
				m_PrefsUseWideScreen = AR_64_27;
				break;
			case AR_3_2:
				m_PrefsUseWideScreen = AR_AUTO;
				break;
			case AR_4_3:
				m_PrefsUseWideScreen = AR_3_2;
				break;
			case AR_5_3:
				m_PrefsUseWideScreen = AR_4_3;
				break;
			case AR_5_4:
				m_PrefsUseWideScreen = AR_5_3;
				break;
			case AR_15_9:
				m_PrefsUseWideScreen = AR_5_4;
				break;
			case AR_16_9:
				m_PrefsUseWideScreen = AR_15_9;
				break;
			case AR_16_10:
				m_PrefsUseWideScreen = AR_16_9;
				break;
			case AR_64_27:
				m_PrefsUseWideScreen = AR_16_10;
				break;
			};
		}
		else {
			switch (m_PrefsUseWideScreen) {
			case AR_AUTO:
				m_PrefsUseWideScreen = AR_3_2;
				break;
			case AR_3_2:
				m_PrefsUseWideScreen = AR_4_3;
				break;
			case AR_4_3:
				m_PrefsUseWideScreen = AR_5_3;
				break;
			case AR_5_3:
				m_PrefsUseWideScreen = AR_5_4;
				break;
			case AR_5_4:
				m_PrefsUseWideScreen = AR_15_9;
				break;
			case AR_15_9:
				m_PrefsUseWideScreen = AR_16_9;
				break;
			case AR_16_9:
				m_PrefsUseWideScreen = AR_16_10;
				break;
			case AR_16_10:
				m_PrefsUseWideScreen = AR_64_27;
				break;
			case AR_64_27:
				m_PrefsUseWideScreen = AR_AUTO;
				break;
			};
		}
#endif
		SaveSettings();
		break;
	case MENUACTION_BRIGHTNESS:
	case MENUACTION_DRAWDIST:
	case MENUACTION_MUSICVOLUME:
	case MENUACTION_SFXVOLUME:
	case MENUACTION_MOUSESENS:
		if (InputDirection > 0)
			CheckSliderMovement(1.0f);
		else
			CheckSliderMovement(-1.0f);
		break;
	case MENUACTION_RADIO:
		if (InputDirection < 0)
			m_PrefsRadioStation -= 1;
		else
			m_PrefsRadioStation += 1;

		if (DMAudio.IsMP3RadioChannelAvailable()) {
			if (m_PrefsRadioStation > USERTRACK)
				m_PrefsRadioStation = HEAD_RADIO;
			else if (m_PrefsRadioStation < HEAD_RADIO)
				m_PrefsRadioStation = USERTRACK;
		}
		else {
			if (m_PrefsRadioStation > CHATTERBOX)
				m_PrefsRadioStation = HEAD_RADIO;
			else if (m_PrefsRadioStation < HEAD_RADIO)
				m_PrefsRadioStation = CHATTERBOX;
		}

		SaveSettings();
		DMAudio.SetRadioInCar(m_PrefsRadioStation);
		DMAudio.PlayFrontEndTrack(m_PrefsRadioStation, 1);
		break;
	case MENUACTION_LANG_ENG:
		if (m_PrefsLanguage != LANGUAGE_AMERICAN) {
			m_PrefsLanguage = LANGUAGE_AMERICAN;
			m_bFrontEnd_ReloadObrTxtGxt = true;
			InitialiseChangedLanguageSettings();
			SaveSettings();
		}
		break;
	case MENUACTION_LANG_FRE:
		if (m_PrefsLanguage != LANGUAGE_FRENCH) {
			m_PrefsLanguage = LANGUAGE_FRENCH;
			m_bFrontEnd_ReloadObrTxtGxt = true;
			InitialiseChangedLanguageSettings();
			SaveSettings();
		}
		break;
	case MENUACTION_LANG_GER:
		if (m_PrefsLanguage != LANGUAGE_GERMAN) {
			m_PrefsLanguage = LANGUAGE_GERMAN;
			m_bFrontEnd_ReloadObrTxtGxt = true;
			InitialiseChangedLanguageSettings();
			SaveSettings();
		}
		break;
	case MENUACTION_LANG_ITA:
		if (m_PrefsLanguage != LANGUAGE_ITALIAN) {
			m_PrefsLanguage = LANGUAGE_ITALIAN;
			m_bFrontEnd_ReloadObrTxtGxt = true;
			InitialiseChangedLanguageSettings();
			SaveSettings();
		}
		break;
	case MENUACTION_LANG_SPA:
		if (m_PrefsLanguage != LANGUAGE_SPANISH) {
			m_PrefsLanguage = LANGUAGE_SPANISH;
			m_bFrontEnd_ReloadObrTxtGxt = true;
			InitialiseChangedLanguageSettings();
			SaveSettings();
		}
		break;
	case MENUACTION_UPDATESAVE:
		PcSaveHelper.PopulateSlotInfo();
		if (aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption].m_SaveSlot >= SAVESLOT_1 && aScreens[m_nCurrOption].m_aEntries[m_nCurrOption].m_SaveSlot <= SAVESLOT_8) {
			m_nCurrSaveSlot = aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption].m_SaveSlot - 2;

			SwitchToNewScreen(aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption].m_TargetMenu);
		}
		break;
	case MENUACTION_CHECKSAVE:
		if (aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption].m_SaveSlot >= SAVESLOT_1 && aScreens[m_nCurrOption].m_aEntries[m_nCurrOption].m_SaveSlot <= SAVESLOT_8) {
			m_nCurrSaveSlot = aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption].m_SaveSlot - 2;

			if (Slots[m_nCurrSaveSlot] != 1 && Slots[m_nCurrSaveSlot] != 2)
				SwitchToNewScreen(aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption].m_TargetMenu);
		}
		break;
	case MENUACTION_NEWGAME:
		DoSettingsBeforeStartingAGame();
		break;
	case MENUACTION_SETDBGFLAG:
		CTheScripts::DbgFlag = CTheScripts::DbgFlag == false;
		break;
	case MENUACTION_UPDATEMEMCARDSAVE:
		RequestFrontEndShutdown();
		break;
	case MENUACTION_INVVERT:
		MousePointerStateHelper.bInvertVertically = MousePointerStateHelper.bInvertVertically == false;
		return;
	case MENUACTION_CANCLEGAME:
		DMAudio.Service();
		RsEventHandler(rsQUITAPP, 0);
		break;
	case MENUACTION_RESUME:
		RequestFrontEndShutdown();
		break;
	case MENUACTION_SCREENRES:
		if (m_bGameNotLoaded) {
			if (InputEnter) {
				if (m_nDisplayVideoMode != m_nPrefsVideoMode) {
					m_nPrefsVideoMode = m_nDisplayVideoMode;
					_psSelectScreenVM(m_nPrefsVideoMode);
					CentreMousePointer();
					m_PrefsUseWideScreen = AR_AUTO;
					SaveSettings();
				}
			}
			else {
				char** VideoModeList = _psGetVideoModeList();
				int NumVideoModes = _psGetNumVideModes();

				if (InputDirection > 0) {
					int nCurrentVidMode = m_nDisplayVideoMode + 1;

					if (nCurrentVidMode >= NumVideoModes)
						nCurrentVidMode = 0;

					while (!VideoModeList[nCurrentVidMode]) {
						++nCurrentVidMode;

						if (nCurrentVidMode >= NumVideoModes)
							nCurrentVidMode = 0;
					}

					m_nDisplayVideoMode = nCurrentVidMode;
				}
				else {
					int nCurrentVidMode = m_nDisplayVideoMode - 1;

					if (nCurrentVidMode < 0)
						nCurrentVidMode = NumVideoModes - 1;

					while (!VideoModeList[nCurrentVidMode]) {
						--nCurrentVidMode;

						if (nCurrentVidMode < 0)
							nCurrentVidMode = NumVideoModes - 1;
					}

					m_nDisplayVideoMode = nCurrentVidMode;
				}
			}
		}
		break;
	case MENUACTION_AUDIOHW:
	{
		int8 AudioHardware = m_nPrefsAudio3DProviderIndex;
		if (m_nPrefsAudio3DProviderIndex == -1)
			break;

		if (InputDirection > 0) {
			switch (m_nPrefsAudio3DProviderIndex) {
			case 0:
				m_nPrefsAudio3DProviderIndex = 1;
				break;
			case 1:
				m_nPrefsAudio3DProviderIndex = 2;
				break;
			case 2:
				m_nPrefsAudio3DProviderIndex = 3;
				break;
			case 3:
				m_nPrefsAudio3DProviderIndex = 4;
				break;
			case 4:
				m_nPrefsAudio3DProviderIndex = 5;
				break;
			case 5:
				m_nPrefsAudio3DProviderIndex = 6;
				break;
			case 6:
				m_nPrefsAudio3DProviderIndex = 0;
				break;
			}
		}
		else {
			switch (m_nPrefsAudio3DProviderIndex) {
			case 0:
				m_nPrefsAudio3DProviderIndex = 6;
				break;
			case 1:
				m_nPrefsAudio3DProviderIndex = 0;
				break;
			case 2:
				m_nPrefsAudio3DProviderIndex = 1;
				break;
			case 3:
				m_nPrefsAudio3DProviderIndex = 2;
				break;
			case 4:
				m_nPrefsAudio3DProviderIndex = 3;
				break;
			case 5:
				m_nPrefsAudio3DProviderIndex = 4;
				break;
			case 6:
				m_nPrefsAudio3DProviderIndex = 5;
				break;
			}
		}

		DMAudio.SetCurrent3DProvider(m_nPrefsAudio3DProviderIndex);

		if (AudioHardware == m_nPrefsAudio3DProviderIndex)
			SetHelperText(0);
		else
			SetHelperText(4);

		SaveSettings();
		break;
	}
	case MENUACTION_SPEAKERCONF:
		if (m_nPrefsAudio3DProviderIndex == -1)
			break;

		if (InputDirection > 0) {
			switch (m_PrefsSpeakers) {
			case 0:
				m_PrefsSpeakers = 1;
				break;
			case 1:
				m_PrefsSpeakers = 2;
				break;
			case 2:
				m_PrefsSpeakers = 0;
				break;
			};
		}
		else {
			switch (m_PrefsSpeakers) {
			case 0:
				m_PrefsSpeakers = 2;
				break;
			case 1:
				m_PrefsSpeakers = 0;
				break;
			case 2:
				m_PrefsSpeakers = 1;
				break;
			};
		}

		DMAudio.SetSpeakerConfig(m_PrefsSpeakers);
		SaveSettings();
		break;
	case MENUACTION_RESTOREDEF: 
		SetDefaultPreferences(m_nCurrScreen);
		SetHelperText(2);
		SaveSettings();
		break;
	case MENUACTION_CTRLMETHOD: 
		if (m_ControlMethod) {
			TheCamera.m_bUseMouse3rdPerson = 1;
			m_ControlMethod = 0;
		}
		else {
			TheCamera.m_bUseMouse3rdPerson = 0;
			m_ControlMethod = 1;
		}
		SaveSettings();
		break;
	case MENUACTION_DYNAMICACOUSTIC: 
		m_PrefsDMA = m_PrefsDMA == false;
		break;
	case MENUACTION_MOUSESTEER:
		m_bDisableMouseSteering = m_bDisableMouseSteering == false;
		return;
	};
}
#endif

#if 0
WRAPPER void CMenuManager::RequestFrontEndShutdown() { EAXJMP(0x488750); }
#else
void CMenuManager::RequestFrontEndShutdown()
{
	m_bShutDownFrontEndRequested = true;
	DMAudio.ChangeMusicMode(1);
}
#endif

#if 0
WRAPPER void CMenuManager::RequestFrontEndStartUp() { EAXJMP(0x488770); }
#else
void CMenuManager::RequestFrontEndStartUp()
{
	m_bStartUpFrontEndRequested = 1;
}
#endif

#if 0
WRAPPER void CMenuManager::ResetHelperText() { EAXJMP(0x48B470); }
#else
void CMenuManager::ResetHelperText() 
{
	m_nHelperTextMsgId = 0;
	m_nHelperTextAlpha = 300;
}
#endif

#if 0
WRAPPER void CMenuManager::SaveLoadFileError_SetUpErrorScreen() { EAXJMP(0x488930); }
#else
void CMenuManager::SaveLoadFileError_SetUpErrorScreen()
{
	switch (PcSaveHelper.m_nHelper) {
	case 1:
	case 2:
	case 3:
		m_nPrevScreen = m_nCurrScreen;
		m_nCurrScreen = MENUPAGE_SAVE_FAILED;
		m_nCurrOption = MENUROW_0;
		m_nScreenChangeDelayTimer = CTimer::GetTimeInMillisecondsPauseMode();
		break;
	break;
	case 4:
	case 5:
	case 6:
		this->m_nPrevScreen = m_nCurrScreen;
		this->m_nCurrScreen = MENUPAGE_LOAD_FAILED;
		m_nCurrOption = MENUROW_0;
		m_nScreenChangeDelayTimer = CTimer::GetTimeInMillisecondsPauseMode();
		break;
	case 7:
		this->m_nPrevScreen = m_nCurrScreen;
		this->m_nCurrScreen = MENUPAGE_LOAD_FAILED_2;
		m_nCurrOption = MENUROW_0;
		m_nScreenChangeDelayTimer = CTimer::GetTimeInMillisecondsPauseMode();
		break;
	case 8:
	case 9:
	case 10:
		m_nPrevScreen = m_nCurrScreen;
		m_nCurrScreen = MENUPAGE_DELETE_FAILED;
		m_nCurrOption = MENUROW_0;
		m_nScreenChangeDelayTimer = CTimer::GetTimeInMillisecondsPauseMode();
		break;
	default:
		return;
	}
}
#endif

#if 0
WRAPPER void CMenuManager::SetHelperText() { EAXJMP(0x48B450); }
#else
void CMenuManager::SetHelperText(int text)
{
	m_nHelperTextMsgId = text;
	m_nHelperTextAlpha = 300;
}
#endif

#if 0
WRAPPER void CMenuManager::SaveSettings() { EAXJMP(0x488CC0); }
#else
void CMenuManager::SaveSettings()
{
	CFileMgr::SetDirMyDocuments();

	int fileHandle = CFileMgr::OpenFile("gta3.set", "w");
	if (fileHandle) {

		ControlsManager.SaveSettings(fileHandle);
		CFileMgr::Write(fileHandle, buf("stuffmorestuffevenmorestuff                 etc"), 20);
		CFileMgr::Write(fileHandle, buf("stuffmorestuffevenmorestuff                 etc"), 20);
		CFileMgr::Write(fileHandle, buf("stuffmorestuffevenmorestuff                 etc"), 4);
		CFileMgr::Write(fileHandle, buf("stuffmorestuffevenmorestuff                 etc"), 4);
		CFileMgr::Write(fileHandle, buf("stuffmorestuffevenmorestuff                 etc"), 1);
		CFileMgr::Write(fileHandle, buf("stuffmorestuffevenmorestuff                 etc"), 1);
		CFileMgr::Write(fileHandle, buf("stuffmorestuffevenmorestuff                 etc"), 1);
		CFileMgr::Write(fileHandle, buf(&TheCamera.m_bHeadBob), 1);
		CFileMgr::Write(fileHandle, buf(&TheCamera.m_fMouseAccelHorzntl), 4);
		CFileMgr::Write(fileHandle, buf(&TheCamera.m_fMouseAccelVertical), 4);
		CFileMgr::Write(fileHandle, buf(&MousePointerStateHelper.bInvertVertically), 1);
		CFileMgr::Write(fileHandle, buf(&CVehicle::m_bDisableMouseSteering), 1);
		CFileMgr::Write(fileHandle, buf(&m_PrefsSfxVolume), 1);
		CFileMgr::Write(fileHandle, buf(&m_PrefsMusicVolume), 1);
		CFileMgr::Write(fileHandle, buf(&m_PrefsRadioStation), 1);
		CFileMgr::Write(fileHandle, buf(&m_PrefsSpeakers), 1);
		CFileMgr::Write(fileHandle, buf(&m_nPrefsAudio3DProviderIndex), 1);
		CFileMgr::Write(fileHandle, buf(&m_PrefsDMA), 1);
		CFileMgr::Write(fileHandle, buf(&m_PrefsBrightness), 1);
		CFileMgr::Write(fileHandle, buf(&m_PrefsLOD), sizeof(m_PrefsLOD));
		CFileMgr::Write(fileHandle, buf(&m_PrefsShowSubtitles), 1);
		CFileMgr::Write(fileHandle, buf(&m_PrefsUseWideScreen), 1);
		CFileMgr::Write(fileHandle, buf(&m_PrefsVsyncDisp), 1);
		CFileMgr::Write(fileHandle, buf(&m_PrefsFrameLimiter), 1);
		CFileMgr::Write(fileHandle, buf(&m_nDisplayVideoMode), 1);
		CFileMgr::Write(fileHandle, buf(&CMBlur::BlurOn), 1);
		CFileMgr::Write(fileHandle, buf(m_PrefsSkinFile), 256);
		CFileMgr::Write(fileHandle, buf(&m_ControlMethod), 1);
		CFileMgr::Write(fileHandle, buf(&m_PrefsLanguage), 1);
	}

	CFileMgr::CloseFile(fileHandle);
	CFileMgr::SetDir("");
}
#endif

#if 0
WRAPPER void CMenuManager::ShutdownJustMenu() { EAXJMP(0x488920); }
#else
void CMenuManager::ShutdownJustMenu()
{
	m_bMenuActive = false;
	CTimer::EndUserPause();
}
#endif

// We won't ever use this again.
#if 0
WRAPPER float CMenuManager::StretchX(float) { EAXJMP(0x48ABE0); }
#else
float CMenuManager::StretchX(float x)
{
	if (SCREENW == 640)
		return x;
	else
		return SCREENW * x * 0.0015625f;
}
#endif

#if 0
WRAPPER float CMenuManager::StretchY(float) { EAXJMP(0x48AC20); }
#else
float CMenuManager::StretchY(float y)
{
	if (SCREENH == 448)
		return y;
	else
		return SCREENH * y * 0.002232143f;
}
#endif

#if 0
WRAPPER void CMenuManager::SwitchMenuOnAndOff() { EAXJMP(0x488790); }
#else
void CMenuManager::SwitchMenuOnAndOff()
{
	// Just what the function name says.
	if (m_bShutDownFrontEndRequested || m_bStartUpFrontEndRequested) {
		if (!m_bMenuActive)
			m_bMenuActive = true;

		if (m_bShutDownFrontEndRequested)
			m_bMenuActive = false;
		if (m_bStartUpFrontEndRequested)
			m_bMenuActive = true;

		if (m_bMenuActive) {
			CTimer::StartUserPause();
		}
		else {
			ShutdownJustMenu();
			SaveSettings();
			m_bStartUpFrontEndRequested = false;
			pControlEdit = 0;
			m_bShutDownFrontEndRequested = false;
			DisplayComboButtonErrMsg = 0;
			CPad::GetPad(0)->Clear(0);
			CPad::GetPad(1)->Clear(0);
			SwitchToNewScreen(0);
		}
	}
	if (m_bSaveMenuActive && !m_bQuitGameNoCD) {
		m_bSaveMenuActive = false;
		m_bMenuActive = true;
		CTimer::StartUserPause();
		SwitchToNewScreen(MENUPAGE_CHOOSE_SAVE_SLOT);
		PcSaveHelper.PopulateSlotInfo();
	}

	if (!m_bMenuActive)
		field_112 = 1;

	m_bStartUpFrontEndRequested = false;
	m_bShutDownFrontEndRequested = false;
}
#endif

#if 0
WRAPPER void CMenuManager::UnloadTextures() { EAXJMP(0x47A440); }
#else
void CMenuManager::UnloadTextures()
{
	if (m_bSpritesLoaded) {
		debug("Remove frontend\n");
		for (int i = 0; i < ARRAY_SIZE(FrontendFilenames); ++i)
			m_aFrontEndSprites[i].Delete();

		int frontend = CTxdStore::FindTxdSlot("frontend");
		CTxdStore::RemoveTxdSlot(frontend);

		debug("Remove menu textures\n");
		for (int i = 0; i < ARRAY_SIZE(MenuFilenames)/2; ++i)
			m_aMenuSprites[i].Delete();

		int menu = CTxdStore::FindTxdSlot("menu");
		CTxdStore::RemoveTxdSlot(menu);

		m_bSpritesLoaded = false;
	}
}
#endif

#if 0
WRAPPER void CMenuManager::WaitForUserCD(void) { EAXJMP(0x48ADD0); }
#else
void CMenuManager::WaitForUserCD()
{
	LoadSplash(0);
	if (!RsGlobal.quit) {
		HandleExit();
		CPad::UpdatePads();
		MessageScreen("NO_PCCD");

		if (GetPadBack()) {
			m_bQuitGameNoCD = true;
			RsEventHandler(rsQUITAPP, 0);
		}
	}
}
#endif

// New content:
uint8 CMenuManager::GetNumberOfMenuOptions()
{
	uint8 Rows = MENUROW_NONE;
	for (int i = 0; i < MENUROWS; i++) {
		if (aScreens[m_nCurrScreen].m_aEntries[i].m_Action == MENUACTION_NOTHING)
			break;

		++Rows;
	};
	return Rows;
}

void CMenuManager::SwitchToNewScreen(int8 screen)
{
	ResetHelperText();

	// Return to - behaviour.
	if (!strcmp(aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption].m_EntryName, "FEDS_TB") ||
		(screen == aScreens[m_nCurrScreen].m_PreviousPage[0])) {
		if (m_bGameNotLoaded) {
			m_nCurrOption = aScreens[m_nCurrScreen].m_ParentEntry[0];
			m_nCurrScreen = aScreens[m_nCurrScreen].m_PreviousPage[0];
		}
		else {
			m_nCurrOption = aScreens[m_nCurrScreen].m_ParentEntry[1];
			m_nCurrScreen = aScreens[m_nCurrScreen].m_PreviousPage[1];
		}

		m_nMenuFadeAlpha = 0;
	}
	else {
		// Go through - behaviour.
		if (screen) {
			m_nPrevScreen = m_nCurrScreen;
			m_nCurrScreen = screen;
			m_nCurrOption = MENUROW_0;
			m_nMenuFadeAlpha = 0;
			m_nScreenChangeDelayTimer = CTimer::GetTimeInMillisecondsPauseMode();
		}
		else {
			m_nPrevScreen = MENUPAGE_NONE;
			m_nCurrScreen = MENUPAGE_NONE;
			m_nCurrOption = MENUROW_0;
		}
	}

	// Set player skin.
	if (m_nCurrScreen == MENUPAGE_SKIN_SELECT) {
		CPlayerSkin::BeginFrontEndSkinEdit();
		field_535 = 19;
		m_bSkinsFound = false;
	}

	// Set radio station.
	if (m_nCurrScreen == MENUPAGE_SOUND_SETTINGS) {
		DMAudio.PlayFrontEndTrack(m_PrefsRadioStation, 1);
		OutputDebugStringA("FRONTEND AUDIO TRACK STOPPED");
	}
	else
		DMAudio.StopFrontEndTrack();
}

void CMenuManager::SetDefaultPreferences(int8 screen)
{
	switch (screen) {
	case MENUPAGE_SOUND_SETTINGS:
		m_PrefsMusicVolume = 102;
		m_PrefsSfxVolume = 102;
		m_PrefsSpeakers = 0;
		m_nPrefsAudio3DProviderIndex = 6;
		m_PrefsDMA = true;
		DMAudio.SetMusicMasterVolume(m_PrefsMusicVolume);
		DMAudio.SetEffectsMasterVolume(m_PrefsSfxVolume);
		DMAudio.SetCurrent3DProvider(m_nPrefsAudio3DProviderIndex);
		break;
	case MENUPAGE_GRAPHICS_SETTINGS:
		m_PrefsBrightness = 256;
		m_PrefsFrameLimiter = true;
		m_PrefsVsync = true;
		m_PrefsLOD = 1.2f;
		m_PrefsVsyncDisp = true;
		lodMultiplier = 1.2;
		CMBlur::BlurOn = true;
		CMBlur::MotionBlurOpen(Scene.camera);
		m_PrefsUseVibration = false;
		m_PrefsShowSubtitles = true;
		m_nDisplayVideoMode = m_nPrefsVideoMode;
		m_PrefsUseWideScreen = AR_AUTO;
		break;
	case MENUPAGE_CONTROLLER_PC:
		ControlsManager.MakeControllerActionsBlank();
		ControlsManager.InitDefaultControlConfiguration();

		CMouseControllerState state = MousePointerStateHelper.GetMouseSetUp();
		ControlsManager.InitDefaultControlConfigMouse(state);

		if (1) {
			//TODO: JoyPad stuff.
		}
		TheCamera.m_bUseMouse3rdPerson = 1;
		m_ControlMethod = 0;
		MousePointerStateHelper.bInvertVertically = true;
		TheCamera.m_fMouseAccelHorzntl = 0.0025f;
		TheCamera.m_fMouseAccelVertical = 0.0025f;
		CVehicle::m_bDisableMouseSteering = true;
		TheCamera.m_bHeadBob = false;
		break;
	};
}

// Frontend inputs.
bool GetPadBack()
{
	return
		(CPad::GetPad(0)->NewKeyState.ESC && !CPad::GetPad(0)->OldKeyState.ESC) ||
		(CPad::GetPad(0)->NewState.Triangle && !CPad::GetPad(0)->OldState.Triangle);
}

bool GetPadExitEnter()
{
	return
		(CPad::GetPad(0)->NewKeyState.ESC && !CPad::GetPad(0)->OldKeyState.ESC) ||
		(CPad::GetPad(0)->NewState.Start && !CPad::GetPad(0)->OldState.Start);
}

bool GetPadForward()
{
	return
		(CPad::GetPad(0)->NewKeyState.EXTENTER && !CPad::GetPad(0)->OldKeyState.EXTENTER) ||
		(CPad::GetPad(0)->NewKeyState.ENTER && !CPad::GetPad(0)->OldKeyState.ENTER) ||
		(CPad::GetPad(0)->NewState.Cross && !CPad::GetPad(0)->OldState.Cross);
}

bool GetPadMoveUp()
{
	return
		(CPad::GetPad(0)->NewState.DPadUp && !CPad::GetPad(0)->OldState.DPadUp) ||
		(CPad::GetPad(0)->NewKeyState.UP && !CPad::GetPad(0)->OldKeyState.UP) ||
		(CPad::GetPad(0)->NewState.LeftStickY < 0 && !CPad::GetPad(0)->OldState.LeftStickY < 0);
}

bool GetPadMoveDown()
{
	return 
		(CPad::GetPad(0)->NewState.DPadDown && !CPad::GetPad(0)->OldState.DPadDown) ||
		(CPad::GetPad(0)->NewKeyState.DOWN && !CPad::GetPad(0)->OldKeyState.DOWN) ||
		(CPad::GetPad(0)->NewState.LeftStickY > 0 && !CPad::GetPad(0)->OldState.LeftStickY > 0);
}

bool GetPadMoveLeft()
{
	return
		(CPad::GetPad(0)->NewState.DPadLeft && !CPad::GetPad(0)->OldState.DPadLeft) ||
		(CPad::GetPad(0)->NewKeyState.LEFT && !CPad::GetPad(0)->OldKeyState.LEFT) ||
		(CPad::GetPad(0)->NewState.LeftStickX < 0 && !CPad::GetPad(0)->OldState.LeftStickX < 0);
}

bool GetPadMoveRight()
{
	return
		(CPad::GetPad(0)->NewState.DPadRight && !CPad::GetPad(0)->OldState.DPadRight) ||
		(CPad::GetPad(0)->NewKeyState.RIGHT && !CPad::GetPad(0)->OldKeyState.RIGHT) ||
		(CPad::GetPad(0)->NewState.LeftStickX > 0 && !CPad::GetPad(0)->OldState.LeftStickX > 0);
}

bool GetMouseForward()
{
	return
		(CPad::GetPad(0)->NewMouseControllerState.LMB && !CPad::GetPad(0)->OldMouseControllerState.LMB);
}

bool GetMouseBack()
{
	return
		(CPad::GetPad(0)->NewMouseControllerState.RMB && !CPad::GetPad(0)->OldMouseControllerState.RMB);
}

bool GetMousePos()
{
	return 
		(CPad::GetPad(0)->NewMouseControllerState.x != 0.0f || CPad::GetPad(0)->OldMouseControllerState.y != 0.0f);
}

bool GetMouseMoveLeft()
{
	return
		(CPad::GetPad(0)->NewMouseControllerState.WHEELDN && !CPad::GetPad(0)->OldMouseControllerState.WHEELDN != 0.0f);
}

bool GetMouseMoveRight()
{
	return
		(CPad::GetPad(0)->NewMouseControllerState.WHEELUP && !CPad::GetPad(0)->OldMouseControllerState.WHEELUP != 0.0f);
}

bool GetPadInput()
{
	return
		GetPadBack() ||
		GetPadForward() ||
		GetPadMoveUp() ||
		GetPadMoveDown() ||
		GetPadMoveLeft() ||
		GetPadMoveRight();
}

bool GetMouseInput()
{
	return
		GetMouseForward() ||
		GetMouseBack() ||
		GetMousePos() ||
		GetMouseMoveLeft() ||
		GetMouseMoveRight();
}

STARTPATCHES
	InjectHook(0x47A230, &CMenuManager::LoadAllTextures, PATCH_JUMP);
	InjectHook(0x47A440, &CMenuManager::UnloadTextures, PATCH_JUMP);
	InjectHook(0x485100, &CMenuManager::Process, PATCH_JUMP);
	InjectHook(0x4856F0, &CMenuManager::ProcessButtonPresses, PATCH_JUMP);
	InjectHook(0x48AE60, &CMenuManager::ProcessOnOffMenuOptions, PATCH_JUMP);
	InjectHook(0x488EE0, &CMenuManager::LoadSettings, PATCH_JUMP);
	InjectHook(0x488CC0, &CMenuManager::SaveSettings, PATCH_JUMP);

	for (int i = 1; i < ARRAY_SIZE(aScreens); i++)
		Patch(0x611930 + sizeof(CMenuScreen) * i, aScreens[i]);
ENDPATCHES