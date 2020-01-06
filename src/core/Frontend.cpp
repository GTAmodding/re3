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
#include "PlayerInfo.h"
#include "World.h"
#include "Renderer.h"
#include "CdStream.h"

#define ALL_ORIGINAL_FRONTEND 1

int32 &CMenuManager::OS_Language = *(int32*)0x5F2F78;	// 9
int8 &CMenuManager::m_PrefsUseVibration = *(int8*)0x95CD92;
int8 &CMenuManager::m_DisplayControllerOnFoot = *(int8*)0x95CD8D;
int8 &CMenuManager::m_PrefsVsync = *(int8*)0x5F2E58;	// 1
int8 &CMenuManager::m_PrefsVsyncDisp = *(int8*)0x5F2E5C;	// 1
int8 &CMenuManager::m_PrefsFrameLimiter = *(int8*)0x5F2E60;	// 1
int8 &CMenuManager::m_PrefsShowSubtitles = *(int8*)0x5F2E54;	// 1
int8 &CMenuManager::m_PrefsSpeakers = *(int8*)0x95CD7E;
int32 &CMenuManager::m_ControlMethod = *(int32*)0x8F5F7C;
int8 &CMenuManager::m_PrefsDMA = *(int8*)0x5F2F74;	// 1
int32 &CMenuManager::m_PrefsLanguage = *(int32*)0x941238;

bool &CMenuManager::m_PrefsAllowNastyGame = *(bool*)0x5F2E64;	// true
bool &CMenuManager::m_bStartUpFrontEndRequested = *(bool*)0x95CCF4;
bool &CMenuManager::m_bShutDownFrontEndRequested = *(bool*)0x95CD6A;

int8 &CMenuManager::m_PrefsUseWideScreen = *(int8*)0x95CD23;
int8 &CMenuManager::m_PrefsRadioStation = *(int8*)0x95CDA4;
int8 &CMenuManager::m_bDisableMouseSteering = *(int8*)0x60252C;	// 1
int32 &CMenuManager::m_PrefsBrightness = *(int32*)0x5F2E50;	// 256
float &CMenuManager::m_PrefsLOD = *(float*)0x8F42C4;
int8 &CMenuManager::m_bFrontEnd_ReloadObrTxtGxt = *(int8*)0x628CFC;
int32 &CMenuManager::m_PrefsMusicVolume = *(int32*)0x5F2E4C;	// 102
int32 &CMenuManager::m_PrefsSfxVolume = *(int32*)0x5F2E48;	// 102

char *CMenuManager::m_PrefsSkinFile = (char*)0x5F2E74;	//[256] "$$\"\""

int32 &CMenuManager::m_KeyPressedCode = *(int32*)0x5F2E70;	// -1

float &CMenuManager::menuXYpadding = *(float*)0x5F355C;	// don't know the original name. MENUACTION_X_MARGIN, never changes
float &CMenuManager::actionTextScaleX = *(float*)0x5F2E40;
float &CMenuManager::actionTextScaleY = *(float*)0x5F2E44;

int& CMenuManager::sthWithButtons = *(int32*)0x8E2880;
int &CMenuManager::sthWithButtons2 = *(int32*)0x942F88;

CMenuManager &FrontEndMenuManager = *(CMenuManager*)0x8F59D8;

// Move this somewhere else.
float &CRenderer::ms_lodDistScale = *(float*)0x5F726C;	// 1.2

// Stuff not in CMenuManager:
uint32 &TimeToStopPadShaking = *(uint32*)0x628CF8;
char *&pEditString = *(char**)0x628D00;
int32 *&pControlEdit = *(int32**)0x628D08;
bool &DisplayComboButtonErrMsg = *(bool*)0x628D14;
int32 &MouseButtonJustClicked = *(int32*)0x628D0C;
int32 &JoyButtonJustClicked = *(int32*)0x628D10;
int32 &nTimeForSomething = *(int32*)0x628D54;
//int32 *pControlTemp = 0;

// Frontend inputs.
bool GetPadBack();
bool GetPadExitEnter();
bool GetPadForward();
bool GetPadMoveUp();
bool GetPadMoveDown();
bool GetPadMoveLeft();
bool GetPadMoveRight();
bool GetMouseClickLeft();
bool GetMouseClickRight();
bool GetMouseClickMiddle();
bool GetMouseWheelUp();
bool GetMouseWheelDown();
bool GetMouseForward();
bool GetMouseBack();
bool GetMousePos();
bool GetMouseMoveLeft();
bool GetMouseMoveRight();
bool GetPadInput();
bool GetMouseInput();

// 0x5F311C
const char* FrontendFilenames[][2] = {
	{"fe2_mainpanel_ul", "" },
	{"fe2_mainpanel_ur", "" },
	{"fe2_mainpanel_dl", "" },
	{"fe2_mainpanel_dr", "" },
	{"fe2_mainpanel_dr2", "" },
	{"fe2_tabactive", "" },
	{"fe_iconbrief", "" },
	{"fe_iconstats", "" },
	{"fe_iconcontrols", "" },
	{"fe_iconsave", "" },
	{"fe_iconaudio", "" },
	{"fe_icondisplay", "" },
	{"fe_iconlanguage", "" },
	{"fe_controller", "" },
	{"fe_controllersh", "" },
	{"fe_arrows1", "" },
	{"fe_arrows2", "" },
	{"fe_arrows3", "" },
	{"fe_arrows4", "" },
	{"fe_radio1", "" }, // HEAD_RADIO
	{"fe_radio2", "" }, // DOUBLE_CLEF
	{"fe_radio3", "" }, // JAH_RADIO
	{"fe_radio4", "" }, // RISE_FM
	{"fe_radio5", "" }, // LIPS_106
	{"fe_radio6", "" }, // GAME_FM
	{"fe_radio7", "" }, // MSX_FM
	{"fe_radio8", "" }, // FLASHBACK
	{"fe_radio9", "" }, // CHATTERBOX
};					

// 0x5F3344
const char* MenuFilenames[][2] = {
	{"connection24", ""},
	{"findgame24", ""},
	{"hostgame24", ""},
	{"mainmenu24", ""},
	{"Playersetup24", ""},
	{"singleplayer24", ""},
	{"multiplayer24", ""},
	{"dmalogo128", "dmalogo128m"},
	{"gtaLogo128", "gtaLogo128"},
	{"rockstarLogo128", "rockstarlogo128m"},
	{"gamespy256", "gamespy256a"},
	{"mouse", "mousetimera"},
	{"mousetimer", "mousetimera"},
	{"mp3logo", "mp3logoA"},
	{"downOFF", "buttonA"},
	{"downON", "buttonA"},
	{"upOff", "buttonA"},
	{"upON", "buttonA"},
	{"gta3logo256", "gta3logo256m"},
	{ nil, nil }
};

#ifdef ASPECT_RATIO_SCALE
// All of the defines below replace the StretchX function. Otherwise use SCREEN_SCALE_X.
#define MENU_X_LEFT_ALIGNED(x) ScaleAndCenterX(x)
#define MENU_X_RIGHT_ALIGNED(x) ScaleAndCenterX(DEFAULT_SCREEN_WIDTH - x)
#define MENU_X(x) SCREEN_SCALE_X(x)
#define MENU_Y(y) SCREEN_SCALE_Y(y)
float
ScaleAndCenterX(float x)
{
	if (SCREEN_WIDTH == DEFAULT_SCREEN_WIDTH)
		return x;
	else {
		if (x > DEFAULT_SCREEN_WIDTH / 2) {
			return SCREEN_WIDTH / 2 + SCREEN_SCALE_X(x - DEFAULT_SCREEN_WIDTH / 2);
		} else {
			return SCREEN_WIDTH / 2 - SCREEN_SCALE_X(DEFAULT_SCREEN_WIDTH / 2 - x);
		}
	}
}
#else
#define MENU_X_LEFT_ALIGNED(x) StretchX(x)
#define MENU_X_RIGHT_ALIGNED(x) SCREEN_STRETCH_FROM_RIGHT(x)
#define MENU_X(x) StretchX(x)
#define MENU_Y(y) StretchY(y)
#endif

void
CMenuManager::BuildStatLine(char *text, void *stat, uint8 aFloat, void *stat2)
{
	if (!text)
		return;

	if (stat2) {
		if (aFloat) 
			sprintf(gString2, "  %.2f %s %.2f", *(float*)stat, UnicodeToAscii(TheText.Get("FEST_OO")), *(float*)stat2);
		else 
			sprintf(gString2, "  %d %s %d", *(int*)stat, UnicodeToAscii(TheText.Get("FEST_OO")), *(int*)stat2);
	} else if (stat) {
		if (aFloat)
			sprintf(gString2, "  %.2f", *(float*)stat);
		else
			sprintf(gString2, "  %d", *(int*)stat);
	} else
		gString2[0] = '\0';

	UnicodeStrcpy(gUString, TheText.Get(text));
	AsciiToUnicode(gString2, gUString2);
}

#if 0
WRAPPER void CMenuManager::CentreMousePointer() { EAXJMP(0x48ACE0); }
#else
void CMenuManager::CentreMousePointer()
{
	tagPOINT Point;

	if (SCREEN_WIDTH * 0.5f == 0.0f && 0.0f == SCREEN_HEIGHT * 0.5f) {
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
WRAPPER int CMenuManager::CheckCodesForControls(int32) { EAXJMP(0x48A950); }
#else
void CMenuManager::CheckCodesForControls(int, int)
{
	DisplayComboButtonErrMsg = 0;
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
	switch (aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption].m_Action) {
	case MENUACTION_BRIGHTNESS:
		m_PrefsBrightness += m_PrefsBrightness + value * (512/16);
		m_PrefsBrightness = clamp(m_PrefsBrightness, 0, 511);
		break;
	case MENUACTION_DRAWDIST:
		m_PrefsLOD += value * ((1.8f - 0.8f)/16.0f);
		m_PrefsLOD = clamp(m_PrefsLOD, 0.8f, 1.8f);
		CRenderer::ms_lodDistScale = m_PrefsLOD;
		break;
	case MENUACTION_MUSICVOLUME:
		m_PrefsMusicVolume += value * (128/16);
		m_PrefsMusicVolume = clamp(m_PrefsMusicVolume, 0, 127);
		DMAudio.SetMusicMasterVolume(m_PrefsMusicVolume);
		break;
	case MENUACTION_SFXVOLUME:
		m_PrefsSfxVolume += value * (128/16);
		m_PrefsSfxVolume = clamp(m_PrefsSfxVolume, 0, 127);
		DMAudio.SetEffectsMasterVolume(m_PrefsSfxVolume);
		break;
	case MENUACTION_MOUSESENS:
		TheCamera.m_fMouseAccelHorzntl += value * 1.0f/200.0f/15.0f;	// ???
		TheCamera.m_fMouseAccelHorzntl = clamp(TheCamera.m_fMouseAccelHorzntl, 1.0f/3200.0f, 1.0f/200.0f);
		TheCamera.m_fMouseAccelVertical = TheCamera.m_fMouseAccelHorzntl;
		break;
	default:
		return;
	}
	SaveSettings();
}

#if 1
WRAPPER int CMenuManager::CostructStatLine(int) { EAXJMP(0x482800); }
#else
int CMenuManager::CostructStatLine(int)
{

}
#endif

#if ALL_ORIGINAL_FRONTEND
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
	}

	CFont::SetAlignment(ALIGN_CENTER);
	CFont::SetScale(SCREEN_SCALE_X(0.4f), SCREEN_SCALE_Y(0.6f));
	CFont::SetFontStyle(FONT_HEADING);
	CFont::SetDropColor(CRGBA(0, 0, 0, AlphaText));
	CFont::SetDropShadowPosition(MENUDROP_COLOR_SIZE);
	CFont::SetColor(CRGBA(255, 255, 255, AlphaText));

	CFont::PrintString(SCREEN_WIDTH / 2, SCREEN_SCALE_FROM_BOTTOM(120.0f), HelperTextToPrint);
}
#endif

#if ALL_ORIGINAL_FRONTEND
WRAPPER int CMenuManager::DisplaySlider(float, float, float, float, float, float) { EAXJMP(0x488420); }
#else
int CMenuManager::DisplaySlider(float x, float y, float leftSize, float rightSize, float rectSize, float progress) 
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
	}
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

	DMAudio.Service();
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
	CFont::SetWrapx(MENU_X_RIGHT_ALIGNED(MENUACTION_X_MARGIN));
	CFont::SetRightJustifyWrap(SCREEN_SCALE_X(38.0f));

	switch (m_nCurrScreen) {
		case MENUPAGE_STATS:
			PrintStats();
			break;
		case MENUPAGE_BRIEFS:
			PrintBriefs();
			break;
	}

	// Header height isn't accounted, we will add that later.
	float nextYToUse = 40.0f;

	// Page name
#ifdef PS2_SAVE_DIALOG
	if(!m_bRenderGameInMenu)
#endif
	if (aScreens[m_nCurrScreen].m_ScreenName[0] != '\0') {
		CFont::SetColor(CRGBA(0, 0, 0, FadeIn(255)));
		CFont::SetRightJustifyOn();
		CFont::SetFontStyle(FONT_HEADING);
		CFont::SetScale(MENU_X(MENUHEADER_WIDTH), MENU_Y(MENUHEADER_HEIGHT));
		CFont::PrintString(SCREEN_SCALE_FROM_RIGHT(MENUHEADER_POS_X), SCREEN_SCALE_FROM_BOTTOM(MENUHEADER_POS_Y), TheText.Get(aScreens[m_nCurrScreen].m_ScreenName));
		// Weird place to put that.
		nextYToUse += 24.0f + 10.0f;
	}

	CFont::SetFontStyle(FONT_BANK);
	CFont::SetScale(MENU_X(0.9f * actionTextScaleX), MENU_Y(0.9f * actionTextScaleY));
	CFont::SetRightJustifyOff(); // AG used SetAlignment(ALIGN_LEFT);
	CFont::SetColor(CRGBA(235, 170, 50, FadeIn(255)));

	// Label
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
		}

#ifdef FIX_BUGS
		// Label is wrapped from right by StretchX(40)px, but wrapped from left by 40px. And this is only place R* didn't use StretchX in menu.
		CFont::PrintString(MENU_X_LEFT_ALIGNED(MENUACTION_X_MARGIN), MENU_Y(menuXYpadding), str);
#else
		CFont::PrintString(MENUACTION_X_MARGIN, menuXYpadding, str);
#endif
	}

	CFont::SetCentreSize(SCREEN_WIDTH);

	bool itemsAreSelectable = true;
	int lineHeight;
	int headerHeight;
	int columnWidth;
	switch (m_nCurrScreen) {
		case MENUPAGE_STATS:
		case MENUPAGE_BRIEFS:
			columnWidth = 320;
			headerHeight = 240;
			lineHeight = 24;
			CFont::SetFontStyle(FONT_HEADING);
			CFont::SetScale(MENU_X(actionTextScaleX = 0.75f), MENU_Y(actionTextScaleY = 0.9f));
			CFont::SetCentreOn();
			break;
		case MENUPAGE_SOUND_SETTINGS:
		case MENUPAGE_GRAPHICS_SETTINGS:
		case MENUPAGE_MULTIPLAYER_CREATE:
		case MENUPAGE_SKIN_SELECT_OLD:
		case MENUPAGE_CONTROLLER_PC_OLD1:
		case MENUPAGE_CONTROLLER_PC_OLD2:
		case MENUPAGE_CONTROLLER_PC_OLD3:
		case MENUPAGE_CONTROLLER_PC_OLD4:
		case MENUPAGE_CONTROLLER_DEBUG:
		case MENUPAGE_MOUSE_CONTROLS:
			columnWidth = 50;
			headerHeight = 0;
			lineHeight = 20;
			CFont::SetFontStyle(FONT_HEADING);
			CFont::SetScale(MENU_X(actionTextScaleX = 0.55f), MENU_Y(actionTextScaleY = 0.8f));
			CFont::SetRightJustifyOff();
			break;
		case MENUPAGE_CHOOSE_LOAD_SLOT:
		case MENUPAGE_CHOOSE_DELETE_SLOT:
		case MENUPAGE_CHOOSE_SAVE_SLOT:
			columnWidth = 120;
			headerHeight = 38;
			lineHeight = 20;
			CFont::SetFontStyle(FONT_BANK);
			CFont::SetScale(MENU_X(actionTextScaleX = 0.45f), MENU_Y(actionTextScaleY = 0.7f));
			CFont::SetRightJustifyOff();
			break;
		case MENUPAGE_NEW_GAME_RELOAD:
		case MENUPAGE_LOAD_SLOT_CONFIRM:
		case MENUPAGE_DELETE_SLOT_CONFIRM:
		case MENUPAGE_SAVE_OVERWRITE_CONFIRM:
		case MENUPAGE_EXIT:
#ifdef PS2_SAVE_DIALOG
		case MENUPAGE_SAVE:
			columnWidth = 280;
			CFont::SetFontStyle(FONT_BANK);
#else
			columnWidth = 320;
			CFont::SetFontStyle(FONT_HEADING);
#endif
			headerHeight = 60;
			lineHeight = 24;
			CFont::SetScale(MENU_X(actionTextScaleX = 0.75f), MENU_Y(actionTextScaleY = 0.9f));
			CFont::SetCentreOn();
			break;
		case MENUPAGE_START_MENU:
			columnWidth = 320;
			headerHeight = 140;
			lineHeight = 24;
			CFont::SetFontStyle(FONT_HEADING);
			CFont::SetScale(MENU_X(actionTextScaleX = 0.75f), MENU_Y(actionTextScaleY = 0.9f));
			CFont::SetCentreOn();
			break;
		case MENUPAGE_PAUSE_MENU:
			columnWidth = 320;
			headerHeight = 117;
			lineHeight = 24;
			CFont::SetFontStyle(FONT_HEADING);
			CFont::SetScale(MENU_X(actionTextScaleX = 0.75f), MENU_Y(actionTextScaleY = 0.9f));
			CFont::SetCentreOn();
			break;
		default:
			columnWidth = 320;
			headerHeight = 40;
			lineHeight = 24;
			CFont::SetFontStyle(FONT_HEADING);
			CFont::SetScale(MENU_X(actionTextScaleX = 0.75f), MENU_Y(actionTextScaleY = 0.9f));
			CFont::SetCentreOn();
			break;
	}

	switch (m_nCurrScreen) {
		case MENUPAGE_CONTROLLER_PC_OLD1:
		case MENUPAGE_CONTROLLER_PC_OLD2:
		case MENUPAGE_CONTROLLER_PC_OLD3:
		case MENUPAGE_CONTROLLER_PC_OLD4:
		case MENUPAGE_CONTROLLER_DEBUG:
			if (field_113)
				itemsAreSelectable = false;

			CMenuManager::DrawControllerScreenExtraText(nextYToUse - 8.0f, 350, lineHeight);
			break;
		default:
			break;
	}

	float usableLineHeight = lineHeight * 0.9f; // also height of biggest bar in slider
	float freeSpaceInLine = lineHeight * 0.1f; // also height of smallest bar in slider(weird)
	bool foundTheHoveringItem = false;
	for (int i = 0; i < NUM_MENUROWS; ++i) {
		if (aScreens[m_nCurrScreen].m_aEntries[i].m_Action != MENUACTION_LABEL && aScreens[m_nCurrScreen].m_aEntries[i].m_EntryName[0] != '\0') {
			wchar *textToPrint[MENUCOLUMNS] = { nil, nil, nil };

			if (aScreens[m_nCurrScreen].m_aEntries[i].m_SaveSlot >= SAVESLOT_1 && aScreens[m_nCurrScreen].m_aEntries[i].m_SaveSlot <= SAVESLOT_8) {
				CFont::SetRightJustifyOff();
				textToPrint[MENUCOLUMN_LEFT] = GetNameOfSavedGame(i - 1);

				if (Slots[i-1] != 1)
					textToPrint[MENUCOLUMN_RIGHT] = GetSavedGameDateAndTime(i - 1);

				if (!textToPrint[MENUCOLUMN_LEFT]) {
					sprintf(gString, "FEM_SL%d", i);
					textToPrint[MENUCOLUMN_LEFT] = TheText.Get(gString);
				}
			} else {
				textToPrint[MENUCOLUMN_LEFT] = TheText.Get(aScreens[m_nCurrScreen].m_aEntries[i].m_EntryName);
			}

			switch (aScreens[m_nCurrScreen].m_aEntries[i].m_Action) {
			case MENUACTION_CHANGEMENU: {
				switch (aScreens[m_nCurrScreen].m_aEntries[i].m_TargetMenu) {
					case MENUPAGE_MULTIPLAYER_MAP:
						switch (sthWithButtons) {
							case 0:
								textToPrint[MENUCOLUMN_RIGHT] = TheText.Get("FEM_MA0");
								break;
							case 1:
								textToPrint[MENUCOLUMN_RIGHT] = TheText.Get("FEM_MA1");
								break;
							case 2:
								textToPrint[MENUCOLUMN_RIGHT] = TheText.Get("FEM_MA2");
								break;
							case 3:
								textToPrint[MENUCOLUMN_RIGHT] = TheText.Get("FEM_MA3");
								break;
							case 4:
								textToPrint[MENUCOLUMN_RIGHT] = TheText.Get("FEM_MA4");
								break;
							case 5:
								textToPrint[MENUCOLUMN_RIGHT] = TheText.Get("FEM_MA5");
								break;
							case 6:
								textToPrint[MENUCOLUMN_RIGHT] = TheText.Get("FEM_MA6");
								break;
							case 7:
								textToPrint[MENUCOLUMN_RIGHT] = TheText.Get("FEM_MA7");
								break;
							default:
								break;
						}
						break;
					case MENUPAGE_MULTIPLAYER_MODE:
						switch (sthWithButtons2) {
							case 0:
								textToPrint[MENUCOLUMN_RIGHT] = TheText.Get("FEN_TY0");
								break;
							case 1:
								textToPrint[MENUCOLUMN_RIGHT] = TheText.Get("FEN_TY1");
								break;
							case 2:
								textToPrint[MENUCOLUMN_RIGHT] = TheText.Get("FEN_TY2");
								break;
							case 3:
								textToPrint[MENUCOLUMN_RIGHT] = TheText.Get("FEN_TY3");
								break;
							case 4:
								textToPrint[MENUCOLUMN_RIGHT] = TheText.Get("FEN_TY4");
								break;
							case 5:
								textToPrint[MENUCOLUMN_RIGHT] = TheText.Get("FEN_TY5");
								break;
							case 6:
								textToPrint[MENUCOLUMN_RIGHT] = TheText.Get("FEN_TY6");
								break;
							case 7:
								textToPrint[MENUCOLUMN_RIGHT] = TheText.Get("FEN_TY7");
								break;
							default:
								break;
						}
						break;
					default:
						break;
				}
				break;
			}
			case MENUACTION_CTRLVIBRATION:
				if (CMenuManager::m_PrefsUseVibration)
					textToPrint[MENUCOLUMN_RIGHT] = TheText.Get("FEM_ON");
				else
					textToPrint[MENUCOLUMN_RIGHT] = TheText.Get("FEM_OFF");
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
				}
				break;
			case MENUACTION_CTRLDISPLAY:
				if (m_DisplayControllerOnFoot)
					textToPrint[MENUCOLUMN_RIGHT] = TheText.Get("FEC_ONF");
				else
					textToPrint[MENUCOLUMN_RIGHT] = TheText.Get("FEC_INC");
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
#ifndef ASPECT_RATIO_SCALE
				textToPrint[MENUCOLUMN_RIGHT] = TheText.Get(m_PrefsUseWideScreen ? "FEM_ON" : "FEM_OFF");
#else
				switch (m_PrefsUseWideScreen) {
				case AR_AUTO:
					textToPrint[MENUCOLUMN_RIGHT] = (wchar*)L"AUTO";
					break;
				case AR_4_3:
					textToPrint[MENUCOLUMN_RIGHT] = (wchar*)L"4:3";
					break;
				case AR_16_9:
					textToPrint[MENUCOLUMN_RIGHT] = (wchar*)L"16:9";
					break;
				}
#endif
				break;
			case MENUACTION_RADIO:
				if (m_PrefsRadioStation > USERTRACK)
					break;

				sprintf(gString, "FEA_FM%d", m_PrefsRadioStation);
				textToPrint[MENUCOLUMN_RIGHT] = TheText.Get(gString);
				break;
			case MENUACTION_SETDBGFLAG:
				textToPrint[MENUCOLUMN_RIGHT] = TheText.Get(CTheScripts::DbgFlag ? "FEM_ON" : "FEM_OFF");
				break;
			case MENUACTION_SWITCHBIGWHITEDEBUGLIGHT:
				textToPrint[MENUCOLUMN_RIGHT] = TheText.Get(gbBigWhiteDebugLightSwitchedOn ? "FEM_ON" : "FEM_OFF");
				break;
			case MENUACTION_PEDROADGROUPS:
				textToPrint[MENUCOLUMN_RIGHT] = TheText.Get(gbShowPedRoadGroups ? "FEM_ON" : "FEM_OFF");
				break;
			case MENUACTION_CARROADGROUPS:
				textToPrint[MENUCOLUMN_RIGHT] = TheText.Get(gbShowCarRoadGroups ? "FEM_ON" : "FEM_OFF");
				break;
			case MENUACTION_COLLISIONPOLYS:
				textToPrint[MENUCOLUMN_RIGHT] = TheText.Get(gbShowCollisionPolys ? "FEM_ON" : "FEM_OFF");
				break;
			case MENUACTION_SHOWCULL:
				textToPrint[MENUCOLUMN_RIGHT] = TheText.Get(gbShowCullZoneDebugStuff ? "FEM_ON" : "FEM_OFF");
				break;
			case MENUACTION_SHOWHEADBOB:
				textToPrint[MENUCOLUMN_RIGHT] = TheText.Get(TheCamera.m_bHeadBob ? "FEM_ON" : "FEM_OFF");
				break;
			case MENUACTION_INVVERT:
				textToPrint[MENUCOLUMN_RIGHT] = TheText.Get(MousePointerStateHelper.bInvertVertically ? "FEM_ON" : "FEM_OFF");
				break;
			case MENUACTION_SCREENRES: {
				RwChar* res = _psGetVideoModeList()[m_nDisplayVideoMode];
				wchar temp[32];
				AsciiToUnicode(res, temp);
				textToPrint[MENUCOLUMN_RIGHT] = temp;
				break;
			}
			case MENUACTION_AUDIOHW: {
				if (m_nPrefsAudio3DProviderIndex == -1)
					textToPrint[MENUCOLUMN_RIGHT] = TheText.Get("FEA_NAH");
				else {
					char* provider = DMAudio.Get3DProviderName(m_nPrefsAudio3DProviderIndex);

					if (!strcmp(strupr(provider), "DIRECTSOUND3D HARDWARE SUPPORT")) {
						strcpy(provider, "DSOUND3D HARDWARE SUPPORT");
					}
					else if (!strcmp(strupr(provider), "DIRECTSOUND3D SOFTWARE EMULATION")) {
						strcpy(provider, "DSOUND3D SOFTWARE EMULATION");
					}
					wchar temp[64];
					AsciiToUnicode(provider, temp);
					textToPrint[MENUCOLUMN_RIGHT] = temp;
				}
				break;
			}
			case MENUACTION_SPEAKERCONF: {
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
					}
				}
				break;
			}
			case MENUACTION_CTRLMETHOD: {
				switch (m_ControlMethod) {
				case 0:
					textToPrint[MENUCOLUMN_LEFT] = TheText.Get("FET_SCN");
					break;
				case 1:
					textToPrint[MENUCOLUMN_LEFT] = TheText.Get("FET_CCN");
					break;
				}
				break;
			}
			case MENUACTION_DYNAMICACOUSTIC:
				textToPrint[MENUCOLUMN_RIGHT] = TheText.Get(m_PrefsDMA ? "FEM_ON" : "FEM_OFF");
				break;
			case MENUACTION_MOUSESTEER:
				textToPrint[MENUCOLUMN_RIGHT] = TheText.Get(m_bDisableMouseSteering ? "FEM_ON" : "FEM_OFF");
				break;
			}

			float nextItemY = headerHeight + nextYToUse;
			float bitAboveNextItemY = nextItemY - 2.0f;
			int nextYToCheck = bitAboveNextItemY;
			
			if (!foundTheHoveringItem) {
				for (int rowToCheck = aScreens[m_nCurrScreen].m_aEntries[0].m_Action == MENUACTION_LABEL; rowToCheck < NUM_MENUROWS; ++rowToCheck) {
					if(aScreens[m_nCurrScreen].m_aEntries[rowToCheck].m_Action == MENUACTION_NOTHING)
						break;

					int extraOffset = 0;
					if (aScreens[m_nCurrScreen].m_aEntries[rowToCheck].m_Action == MENUACTION_RADIO)
						extraOffset = (int) 60.0f;

					// There were many unused codes in here to calculate how much space will texts gonna take.

					// FIX: nextYToCheck already starts with Y - 2, let's sync it with green bar bounds.
#ifdef FIX_BUGS
					if (m_nMousePosY > MENU_Y(nextYToCheck) &&
#else
					if (m_nMousePosY > MENU_Y(nextYToCheck - 2) &&
#endif
						m_nMousePosY < MENU_Y((nextYToCheck + 2) + usableLineHeight)) {

						static int lastHoveringOption = -99;
						static int lastScreen = m_nCurrScreen;

						m_nPrevOption = rowToCheck;
						if (m_nMouseOldPosX != m_nMousePosX || m_nMouseOldPosY != m_nMousePosY) {
							m_nCurrOption = rowToCheck;
							m_bShowMouse = true;
						}
						if (lastHoveringOption != m_nCurrOption) {
							if (lastScreen == m_nCurrScreen && m_bShowMouse)
								DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_DENIED, 0);

							lastHoveringOption = m_nCurrOption;
							lastScreen = m_nCurrScreen;
						}
						if (lastScreen == m_nPrevScreen)
							lastScreen = m_nCurrScreen;

						m_nHoverOption = HOVEROPTION_DEFAULT;
						foundTheHoveringItem = true;
						break;
					}
					m_nHoverOption = HOVEROPTION_42;
					nextYToCheck += extraOffset + lineHeight;
				}
			}

			// Green bar behind selected option
#ifdef PS2_SAVE_DIALOG
			if (!m_bRenderGameInMenu)
#endif
			if (i == m_nCurrOption && itemsAreSelectable) {
				// We keep stretching, because we also stretch background image and we want that bar to be aligned with borders of background
				CSprite2d::DrawRect(CRect(StretchX(10.0f), MENU_Y(bitAboveNextItemY),
											SCREEN_STRETCH_FROM_RIGHT(11.0f), MENU_Y(usableLineHeight + nextItemY)),
											CRGBA(100, 200, 50, FadeIn(50)));
			}
			CFont::SetColor(CRGBA(0, 0, 0, FadeIn(90)));

			// Button and it's shadow
			for(int textLayer = 0; textLayer < 2; textLayer++) {
				if (!CFont::Details.centre)
					CFont::SetRightJustifyOff();

				float itemY = MENU_Y(textLayer + nextItemY);
				float itemX = MENU_X_LEFT_ALIGNED(textLayer + columnWidth);
				CFont::PrintString(itemX, itemY, textToPrint[MENUCOLUMN_LEFT]);
				if (textToPrint[MENUCOLUMN_RIGHT]) {
					if (!CFont::Details.centre)
						CFont::SetRightJustifyOn();
					
					if(!strcmp(aScreens[m_nCurrScreen].m_aEntries[i].m_EntryName, "FED_RES") 
						&& !m_bGameNotLoaded && textLayer == 1) {
						CFont::SetColor(CRGBA(155, 117, 6, FadeIn(255)));
					}
					CFont::PrintString(MENU_X_RIGHT_ALIGNED(columnWidth - textLayer), itemY, textToPrint[MENUCOLUMN_RIGHT]);
				}
				if (i == m_nCurrOption && itemsAreSelectable) {
					CFont::SetColor(CRGBA(255, 217, 106, FadeIn(255)));
				} else {
					CFont::SetColor(CRGBA(235, 170, 50, FadeIn(255)));
				}
			}

			if (m_nPrefsAudio3DProviderIndex == DMAudio.GetCurrent3DProviderIndex()) {
				if(!strcmp(aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption].m_EntryName, "FEA_3DH") && m_nHelperTextMsgId == 1)
					ResetHelperText();
			}
			if (m_nDisplayVideoMode == m_nPrefsVideoMode) {
				if (!strcmp(aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption].m_EntryName, "FED_RES") && m_nHelperTextMsgId == 1)
					ResetHelperText();
			}
			if (m_nPrefsAudio3DProviderIndex != DMAudio.GetCurrent3DProviderIndex()) {
				if (!strcmp(aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption].m_EntryName, "FEA_3DH"))
					SetHelperText(1);
			}
			if (m_nDisplayVideoMode != m_nPrefsVideoMode) {
				if (!strcmp(aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption].m_EntryName, "FED_RES"))
					SetHelperText(1);
			}
			if (m_nPrefsAudio3DProviderIndex != DMAudio.GetCurrent3DProviderIndex()) {
				if (strcmp(aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption].m_EntryName, "FEA_3DH") != 0
					&& m_nCurrScreen == 5 && m_nPrefsAudio3DProviderIndex != -1) {

					m_nPrefsAudio3DProviderIndex = DMAudio.GetCurrent3DProviderIndex();
					SetHelperText(3);
				}
			}
			if (m_nDisplayVideoMode != m_nPrefsVideoMode) {
				if (strcmp(aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption].m_EntryName, "FED_RES") != 0 && m_nCurrScreen == 6) {
					m_nDisplayVideoMode = m_nPrefsVideoMode;
					SetHelperText(3);
				}
			}

			// Sliders
			// We stretch slider start X here(like original code), because it will always be center of screen
			int sliderValue;
			switch (aScreens[m_nCurrScreen].m_aEntries[i].m_Action) {
				case MENUACTION_BRIGHTNESS: {
					sliderValue = DisplaySlider(SCREEN_STRETCH_FROM_RIGHT(MENUSLIDER_X + columnWidth), MENU_Y(bitAboveNextItemY), MENU_Y(freeSpaceInLine), MENU_Y(usableLineHeight), MENU_X(MENUSLIDER_UNK), m_PrefsBrightness / 512.0f);
					if (i != m_nCurrOption || !itemsAreSelectable)
						break;

					if (CheckHover(MENU_X_LEFT_ALIGNED(170.0f), sliderValue - MENU_X(10.0f), MENU_Y(nextYToUse), MENU_Y(28.0f + nextYToUse)))
						m_nHoverOption = HOVEROPTION_DECREASE_BRIGHTNESS;

					if (!CheckHover(MENU_X(10.0f) + sliderValue, SCREEN_WIDTH, MENU_Y(nextYToUse), MENU_Y(28.0f + nextYToUse)))
						break;

					m_nHoverOption = HOVEROPTION_INCREASE_BRIGHTNESS;
					if (m_nMousePosX < SCREEN_STRETCH_FROM_RIGHT(MENUSLIDER_X + columnWidth))
						m_nHoverOption = HOVEROPTION_42;
					break;
				}
				case MENUACTION_DRAWDIST: {
					sliderValue = DisplaySlider(SCREEN_STRETCH_FROM_RIGHT(MENUSLIDER_X + columnWidth), MENU_Y(bitAboveNextItemY), MENU_Y(freeSpaceInLine), MENU_Y(usableLineHeight), MENU_X(MENUSLIDER_UNK), (m_PrefsLOD - 0.8f) * 1.0f);
					if (i != m_nCurrOption || !itemsAreSelectable)
						break;

					if (CheckHover(MENU_X_LEFT_ALIGNED(170.0f), sliderValue - MENU_X(10.0f), MENU_Y(nextYToUse), MENU_Y(28.0f + nextYToUse)))
						m_nHoverOption = HOVEROPTION_DECREASE_DRAWDIST;

					if (!CheckHover(MENU_X(10.0f) + sliderValue, SCREEN_WIDTH, MENU_Y(nextYToUse), MENU_Y(28.0f + nextYToUse)))
						break;

					m_nHoverOption = HOVEROPTION_INCREASE_DRAWDIST;
					if (m_nMousePosX < SCREEN_STRETCH_FROM_RIGHT(MENUSLIDER_X + columnWidth))
						m_nHoverOption = HOVEROPTION_42;
					break;
				}
				case MENUACTION_MUSICVOLUME: {
					sliderValue = DisplaySlider(SCREEN_STRETCH_FROM_RIGHT(MENUSLIDER_X + columnWidth), MENU_Y(bitAboveNextItemY), MENU_Y(freeSpaceInLine), MENU_Y(usableLineHeight), MENU_X(MENUSLIDER_UNK), m_PrefsMusicVolume / 128.0f);
					if (i != m_nCurrOption || !itemsAreSelectable)
						break;

					if (CheckHover(MENU_X_LEFT_ALIGNED(170.0f), sliderValue - MENU_X(10.0f), MENU_Y(nextYToUse), MENU_Y(28.0f + nextYToUse)))
						m_nHoverOption = HOVEROPTION_DECREASE_MUSICVOLUME;

					if (!CheckHover(MENU_X(10.0f) + sliderValue, SCREEN_WIDTH, MENU_Y(nextYToUse), MENU_Y(28.0f + nextYToUse)))
						break;
					m_nHoverOption = HOVEROPTION_INCREASE_MUSICVOLUME;
					if (m_nMousePosX < SCREEN_STRETCH_FROM_RIGHT(MENUSLIDER_X + columnWidth))
						m_nHoverOption = HOVEROPTION_42;
					break;
				}
				case MENUACTION_SFXVOLUME: {
					sliderValue = DisplaySlider(SCREEN_STRETCH_FROM_RIGHT(MENUSLIDER_X + columnWidth), MENU_Y(bitAboveNextItemY), MENU_Y(freeSpaceInLine), MENU_Y(usableLineHeight), MENU_X(MENUSLIDER_UNK), m_PrefsSfxVolume / 128.0f);
					if (i != m_nCurrOption || !itemsAreSelectable)
						break;

					if (CheckHover(MENU_X_LEFT_ALIGNED(170.0f), sliderValue - MENU_X(10.0f), MENU_Y(nextYToUse), MENU_Y(28.0f + nextYToUse)))
						m_nHoverOption = HOVEROPTION_DECREASE_SFXVOLUME;

					if (!CheckHover(MENU_X(10.0f) + sliderValue, SCREEN_WIDTH, MENU_Y(nextYToUse), MENU_Y(28.0f + nextYToUse)))
						break;
					m_nHoverOption = HOVEROPTION_INCREASE_SFXVOLUME;
					if (m_nMousePosX < SCREEN_STRETCH_FROM_RIGHT(MENUSLIDER_X + columnWidth))
						m_nHoverOption = HOVEROPTION_42;
					break;
				}
				case MENUACTION_MOUSESENS: {
					sliderValue = DisplaySlider(SCREEN_STRETCH_FROM_RIGHT(MENUSLIDER_X + columnWidth), MENU_Y(bitAboveNextItemY), MENU_Y(freeSpaceInLine), MENU_Y(usableLineHeight), MENU_X(MENUSLIDER_UNK), TheCamera.m_fMouseAccelHorzntl * 200.0f);
					if (i != m_nCurrOption || !itemsAreSelectable)
						break;

					if (CheckHover(MENU_X_LEFT_ALIGNED(200.0f), sliderValue - MENU_X(10.0f), MENU_Y(nextYToUse), MENU_Y(28.0f + nextYToUse)))
						m_nHoverOption = HOVEROPTION_DECREASE_MOUSESENS;

					if (!CheckHover(MENU_X(10.0f) + sliderValue, SCREEN_WIDTH, MENU_Y(nextYToUse), MENU_Y(28.0f + nextYToUse)))
						break;
					m_nHoverOption = HOVEROPTION_INCREASE_MOUSESENS;
					if (m_nMousePosX < SCREEN_STRETCH_FROM_RIGHT(MENUSLIDER_X + columnWidth))
						m_nHoverOption = HOVEROPTION_42;
					break;
				}
			}

			nextYToUse += lineHeight * CFont::GetNumberLines(menuXYpadding, nextYToUse, textToPrint[MENUCOLUMN_LEFT]);

			// Radio icons.
			// TO-DO: This is missing/broken
			float fIconSpacing = 59.52f;
			if (aScreens[m_nCurrScreen].m_aEntries[i].m_Action == MENUACTION_RADIO) {
				for (int i = 0; i < POLICE_RADIO; i++) {
#ifndef ASPECT_RATIO_SCALE
					if (i < USERTRACK)
						m_aFrontEndSprites[i + FE_RADIO1].Draw(SCREEN_STRETCH_X(MENURADIO_ICON_X + fIconSpacing * i), (SCREEN_HEIGHT / 2) - SCREEN_SCALE_Y(MENURADIO_ICON_Y), SCREEN_SCALE_X(MENURADIO_ICON_W), SCREEN_SCALE_Y(MENURADIO_ICON_H), i == m_PrefsRadioStation ? CRGBA(255, 255, 255, 255) : CRGBA(225, 0, 0, 170));
					if (i > CHATTERBOX && DMAudio.IsMP3RadioChannelAvailable())
						m_aMenuSprites[MENUSPRITE_MP3LOGO].Draw(SCREEN_STRETCH_X(MENURADIO_ICON_X + fIconSpacing * i), (SCREEN_HEIGHT / 2) - SCREEN_SCALE_Y(MENURADIO_ICON_Y), SCREEN_SCALE_X(MENURADIO_ICON_W), SCREEN_SCALE_Y(MENURADIO_ICON_H), i == m_PrefsRadioStation ? CRGBA(255, 255, 255, 255) : CRGBA(225, 0, 0, 170));
#else
					float fMp3Pos = 0.0f;
					if (DMAudio.IsMP3RadioChannelAvailable())
						fMp3Pos = 34.0f;

					if (i < USERTRACK)
						m_aFrontEndSprites[i + FE_RADIO1].Draw((SCREEN_WIDTH * 0.5) + SCREEN_SCALE_X(-fMp3Pos + MENURADIO_ICON_X + (fIconSpacing * i)), MENU_Y(nextYToUse), MENU_X(menuXYpadding), MENU_Y(menuXYpadding), i == m_PrefsRadioStation ? CRGBA(255, 255, 255, 255) : CRGBA(225, 0, 0, 170));
					if (i > CHATTERBOX && DMAudio.IsMP3RadioChannelAvailable())
						m_aMenuSprites[MENUSPRITE_MP3LOGO].Draw((SCREEN_WIDTH * 0.5) + SCREEN_SCALE_X(-fMp3Pos + MENURADIO_ICON_X + (fIconSpacing * i)), MENU_Y(nextYToUse), MENU_X(menuXYpadding), MENU_Y(menuXYpadding), i == m_PrefsRadioStation ? CRGBA(255, 255, 255, 255) : CRGBA(225, 0, 0, 170));
#endif
				}
				nextYToUse += 70.0f;
			}
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
	}
/*
	if (m_nCurrScreen == MENUPAGE_CONTROLLER_SETTINGS) {
		PrintController();
	} else if (m_nCurrScreen == MENUPAGE_SKIN_SELECT_OLD) {
		CSprite2d::DrawRect(CRect(StretchX(180), MENU_Y(98), StretchX(230), MENU_Y(123)), CRGBA(255, 255, 255, FadeIn(255)));
		CSprite2d::DrawRect(CRect(StretchX(181), MENU_Y(99), StretchX(229), MENU_Y(233)), CRGBA(Player color from PickNewPlayerColour, FadeIn(255)));
	}
*/
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

void CMenuManager::DrawFrontEnd()
{
	CFont::SetAlphaFade(255.0f);

	if (m_nCurrScreen == MENUPAGE_NONE) {
		if (m_bGameNotLoaded)
			m_nCurrScreen = MENUPAGE_START_MENU;
		else
			m_nCurrScreen = MENUPAGE_PAUSE_MENU;
	}

	if (m_nCurrOption == 0 && aScreens[m_nCurrScreen].m_aEntries[0].m_Action == MENUACTION_LABEL)
		m_nCurrOption = 1;

#ifdef PS2_SAVE_DIALOG
	if(m_bRenderGameInMenu)
		DrawFrontEndSaveZone();
	else
#endif
		DrawFrontEndNormal();

	PrintErrorMessage();
}

#ifdef PS2_SAVE_DIALOG
void CMenuManager::DrawFrontEndSaveZone()
{
	CSprite2d::InitPerFrame();
	CFont::InitPerFrame();
	RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void*)rwFILTERNEAREST);

	// Not original dimensions, have been changed to fit PC screen & PC menu layout.
	CSprite2d::DrawRect(CRect(MENU_X_LEFT_ALIGNED(25.0f), MENU_Y(40.0f), MENU_X_RIGHT_ALIGNED(25.0f), SCREEN_SCALE_FROM_BOTTOM(40.0f)), CRGBA(0, 0, 0, 175));

	m_nMenuFadeAlpha = 255;
	RwRenderStateSet(rwRENDERSTATETEXTUREADDRESS, (void*)rwTEXTUREADDRESSCLAMP);
	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)FALSE);
	CMenuManager::Draw();

	CFont::DrawFonts();

	// Draw mouse
	RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void*)rwFILTERLINEAR);
	RwRenderStateSet(rwRENDERSTATETEXTUREADDRESS, (void*)rwTEXTUREADDRESSCLAMP);
	if (m_bShowMouse) {
		RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDSRCALPHA);
		RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDINVSRCALPHA);
		RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)TRUE);

		CRect mouse(0.0f, 0.0f, MENU_X(75.0f), MENU_Y(75.0f));
		CRect shad(MENU_X(10.0f), MENU_Y(3.0f), MENU_X(85.0f), MENU_Y(78.0f));

		mouse.Translate(m_nMousePosX, m_nMousePosY);
		shad.Translate(m_nMousePosX, m_nMousePosY);
		if(field_518 == 4){
			m_aMenuSprites[MENUSPRITE_MOUSET].Draw(shad, CRGBA(100, 100, 100, 50));
			RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)FALSE);
			m_aMenuSprites[MENUSPRITE_MOUSET].Draw(mouse, CRGBA(255, 255, 255, 255));
		}else{
			m_aMenuSprites[MENUSPRITE_MOUSE].Draw(shad, CRGBA(100, 100, 100, 50));
			RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)FALSE);
			m_aMenuSprites[MENUSPRITE_MOUSE].Draw(mouse, CRGBA(255, 255, 255, 255));
		}
	}
}
#endif

void CMenuManager::DrawFrontEndNormal()
{
	CSprite2d::InitPerFrame();
	CFont::InitPerFrame();
	RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void*)rwFILTERLINEAR);

	LoadSplash(nil);

	eMenuSprites previousSprite;
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
			default:
				previousSprite = MENUSPRITE_MAINMENU;
				break;
		}

		if (m_nPrevScreen == m_nCurrScreen)
			CSprite2d::DrawRect(CRect(0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT), CRGBA(0, 0, 0, 255 - m_nMenuFadeAlpha));
		else
			m_aMenuSprites[previousSprite].Draw(CRect(0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT), CRGBA(255, 255, 255, 255 - m_nMenuFadeAlpha));
	}

	RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*)FALSE);
	RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)FALSE);

	eMenuSprites currentSprite = MENUSPRITE_MAINMENU; // actually uninitialized
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
	}

	if (m_nMenuFadeAlpha < 255) {
		static int LastFade = 0;

		if(CTimer::GetTimeInMillisecondsPauseMode() - LastFade > 10){
			m_nMenuFadeAlpha += 20;
			LastFade = CTimer::GetTimeInMillisecondsPauseMode();
		}

		if (m_nMenuFadeAlpha > 255){
			m_aMenuSprites[currentSprite].Draw(CRect(0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT), CRGBA(255, 255, 255, 255));
		}else{
			RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)TRUE);
			m_aMenuSprites[currentSprite].Draw(CRect(0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT), CRGBA(255, 255, 255, m_nMenuFadeAlpha));
		}
	}
	else {
		m_aMenuSprites[currentSprite].Draw(CRect(0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT), CRGBA(255, 255, 255, 255));
		// TODO: what is this? waiting mouse?
		if(field_518 == 4){
			if(m_nHoverOption == HOVEROPTION_3 || m_nHoverOption == HOVEROPTION_4 ||
				m_nHoverOption == HOVEROPTION_5 || m_nHoverOption == HOVEROPTION_6 || m_nHoverOption == HOVEROPTION_7)

				field_518 = 2;
			else
				field_518 = 1;
		}
	}

	// GTA LOGO
	RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDSRCALPHA);
	RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDINVSRCALPHA);
	if (m_nCurrScreen == MENUPAGE_START_MENU || m_nCurrScreen == MENUPAGE_PAUSE_MENU) {
		if (CGame::frenchGame || CGame::germanGame || !CGame::nastyGame)
			m_aMenuSprites[MENUSPRITE_GTA3LOGO].Draw(CRect(MENU_X_LEFT_ALIGNED(205.0f), MENU_Y(70.0f), MENU_X_LEFT_ALIGNED(435.0f), MENU_Y(180.0f)), CRGBA(255, 255, 255, FadeIn(255)));
		else
			m_aMenuSprites[MENUSPRITE_GTALOGO].Draw(CRect(MENU_X_LEFT_ALIGNED(225.0f), MENU_Y(40.0f), MENU_X_LEFT_ALIGNED(415.0f), MENU_Y(210.0f)), CRGBA(255, 255, 255, FadeIn(255)));
	}

	RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void*)rwFILTERNEAREST);
	RwRenderStateSet(rwRENDERSTATETEXTUREADDRESS, (void*)rwTEXTUREADDRESSCLAMP);
	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)FALSE);
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
	}

	CFont::DrawFonts();

	// Draw mouse
	RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void*)rwFILTERLINEAR);
	RwRenderStateSet(rwRENDERSTATETEXTUREADDRESS, (void*)rwTEXTUREADDRESSCLAMP);
	if (m_bShowMouse) {
		RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDSRCALPHA);
		RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDINVSRCALPHA);
		RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)TRUE);

		CRect mouse(0.0f, 0.0f, MENU_X(75.0f), MENU_Y(75.0f));
		CRect shad(MENU_X(10.0f), MENU_Y(3.0f), MENU_X(85.0f), MENU_Y(78.0f));

		mouse.Translate(m_nMousePosX, m_nMousePosY);
		shad.Translate(m_nMousePosX, m_nMousePosY);
		if(field_518 == 4){
			m_aMenuSprites[MENUSPRITE_MOUSET].Draw(shad, CRGBA(100, 100, 100, 50));
			RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)FALSE);
			m_aMenuSprites[MENUSPRITE_MOUSET].Draw(mouse, CRGBA(255, 255, 255, 255));
		}else{
			m_aMenuSprites[MENUSPRITE_MOUSE].Draw(shad, CRGBA(100, 100, 100, 50));
			RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)FALSE);
			m_aMenuSprites[MENUSPRITE_MOUSE].Draw(mouse, CRGBA(255, 255, 255, 255));
		}
	}
}

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
	return min(m_nMenuFadeAlpha, alpha);
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

#if ALL_ORIGINAL_FRONTEND
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
		}
	}
}
#endif

void CMenuManager::LoadAllTextures()
{
	if (m_bSpritesLoaded)
		return;

	CMenuManager::CentreMousePointer();
	DMAudio.ChangeMusicMode(MUSICMODE_FRONTEND);
	DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_STARTING, 0);
	m_nCurrOption = 0;
	m_PrefsRadioStation = DMAudio.GetRadioInCar();

	if (DMAudio.IsMP3RadioChannelAvailable()) {
		if (CMenuManager::m_PrefsRadioStation > USERTRACK)
			CMenuManager::m_PrefsRadioStation = CGeneral::GetRandomNumber() % 10;
	} else if (CMenuManager::m_PrefsRadioStation > CHATTERBOX)
		CMenuManager::m_PrefsRadioStation = CGeneral::GetRandomNumber() % 9;
	
	CFileMgr::SetDir("");
	//CFileMgr::SetDir("");
	CTimer::Stop();
	CStreaming::MakeSpaceFor(350 * CDSTREAM_SECTOR_SIZE); // twice of it in mobile
	CStreaming::ImGonnaUseStreamingMemory();
	CGame::TidyUpMemory(false, true);
	CTxdStore::PushCurrentTxd();
	int frontendTxdSlot = CTxdStore::FindTxdSlot("frontend");

	if(frontendTxdSlot == -1)
		frontendTxdSlot = CTxdStore::AddTxdSlot("frontend");

	printf("LOAD frontend\n");
	CTxdStore::LoadTxd(frontendTxdSlot, "MODELS/FRONTEND.TXD");
	CTxdStore::AddRef(frontendTxdSlot);
	CTxdStore::SetCurrentTxd(frontendTxdSlot);
	CStreaming::IHaveUsedStreamingMemory();
	CTimer::Update();

	for (int i = 0; i < ARRAY_SIZE(FrontendFilenames); i++) {
		m_aFrontEndSprites[i].SetTexture(FrontendFilenames[i][0], FrontendFilenames[i][1]);
		m_aFrontEndSprites[i].SetAddressing(rwTEXTUREADDRESSBORDER);
	}

	int menuTxdSlot = CTxdStore::FindTxdSlot("menu");

	if (menuTxdSlot == -1)
		menuTxdSlot = CTxdStore::AddTxdSlot("menu");

	printf("LOAD sprite\n");
	CTxdStore::LoadTxd(menuTxdSlot, "MODELS/MENU.TXD");
	CTxdStore::AddRef(menuTxdSlot);
	CTxdStore::SetCurrentTxd(menuTxdSlot);

	for (int i = 0; i < ARRAY_SIZE(MenuFilenames); i++) {
		m_aMenuSprites[i].SetTexture(MenuFilenames[i][0], MenuFilenames[i][1]);
		m_aMenuSprites[i].SetAddressing(rwTEXTUREADDRESSBORDER);
	}

	m_bSpritesLoaded = true;
	CTxdStore::PopCurrentTxd();
}
#if 0
WRAPPER void CMenuManager::LoadSettings() { EAXJMP(0x488EE0); }
#else
void CMenuManager::LoadSettings()
{
	CFileMgr::SetDirMyDocuments();
	int fileHandle = CFileMgr::OpenFile("gta3.set", "r");

	int32 prevLang = m_PrefsLanguage;
	CMBlur::BlurOn = true;
	MousePointerStateHelper.bInvertVertically = true;

	// 50 is silly
	char Ver[50];

	if (fileHandle) {
		CFileMgr::Read(fileHandle, Ver, 29);

		if (strncmp(Ver, TopLineEmptyFile, sizeof(TopLineEmptyFile) - 1)) {
			CFileMgr::Seek(fileHandle, 0, 0);
			ControlsManager.LoadSettings(fileHandle);
			CFileMgr::Read(fileHandle, gString, 20);
			CFileMgr::Read(fileHandle, gString, 20);
			CFileMgr::Read(fileHandle, gString, 4);
			CFileMgr::Read(fileHandle, gString, 4);
			CFileMgr::Read(fileHandle, gString, 1);
			CFileMgr::Read(fileHandle, gString, 1);
			CFileMgr::Read(fileHandle, gString, 1);
			CFileMgr::Read(fileHandle, (char*)&TheCamera.m_bHeadBob, 1);
			CFileMgr::Read(fileHandle, (char*)&TheCamera.m_fMouseAccelHorzntl, 4);
			CFileMgr::Read(fileHandle, (char*)&TheCamera.m_fMouseAccelVertical, 4);
			CFileMgr::Read(fileHandle, (char*)&MousePointerStateHelper.bInvertVertically, 1);
			CFileMgr::Read(fileHandle, (char*)&CVehicle::m_bDisableMouseSteering, 1);
			CFileMgr::Read(fileHandle, (char*)&m_PrefsSfxVolume, 1);
			CFileMgr::Read(fileHandle, (char*)&m_PrefsMusicVolume, 1);
			CFileMgr::Read(fileHandle, (char*)&m_PrefsRadioStation, 1);
			CFileMgr::Read(fileHandle, (char*)&m_PrefsSpeakers, 1);
			CFileMgr::Read(fileHandle, (char*)&m_nPrefsAudio3DProviderIndex, 1);
			CFileMgr::Read(fileHandle, (char*)&m_PrefsDMA, 1);
			CFileMgr::Read(fileHandle, (char*)&m_PrefsBrightness, 1);
			CFileMgr::Read(fileHandle, (char*)&m_PrefsLOD, 4);
			CFileMgr::Read(fileHandle, (char*)&m_PrefsShowSubtitles, 1);
			CFileMgr::Read(fileHandle, (char*)&m_PrefsUseWideScreen, 1);
			CFileMgr::Read(fileHandle, (char*)&m_PrefsVsyncDisp, 1);
			CFileMgr::Read(fileHandle, (char*)&m_PrefsFrameLimiter, 1);
			CFileMgr::Read(fileHandle, (char*)&m_nDisplayVideoMode, 1);
			CFileMgr::Read(fileHandle, (char*)&CMBlur::BlurOn, 1);
			CFileMgr::Read(fileHandle, m_PrefsSkinFile, 256);
			CFileMgr::Read(fileHandle, (char*)&m_ControlMethod, 1);
			CFileMgr::Read(fileHandle, (char*)&m_PrefsLanguage, 1);
		}
	}

	CFileMgr::CloseFile(fileHandle);
	CFileMgr::SetDir("");

	m_PrefsVsync = m_PrefsVsyncDisp;
	CRenderer::ms_lodDistScale = m_PrefsLOD;

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

		OutputDebugString("The previously saved language is now in use");
	}

	WIN32_FIND_DATA FindFileData;
	char skinfile[256+16];	// Stack analysis shows 16 bits gap, but I don't trust it. It may very well be MAX_PATH(260).
	bool SkinFound = false;
	HANDLE handle = FindFirstFile("skins\\*.bmp", &FindFileData);
	for (int i = 1; handle != INVALID_HANDLE_VALUE && i; i = FindNextFile(handle, &FindFileData)) {
		strcpy(skinfile, m_PrefsSkinFile);
		strcat(skinfile, ".bmp");
		if (strcmp(FindFileData.cFileName, skinfile) == 0)
			SkinFound = true;
	}
	FindClose(handle);

	if (!SkinFound) {
		OutputDebugString("Default skin set as no other skins are available OR saved skin not found!");
		strcpy(m_PrefsSkinFile, "$$\"\"");
		strcpy(m_aSkinName, "$$\"\"");
	}
}
#endif

#if 0
WRAPPER void CMenuManager::SaveSettings() { EAXJMP(0x488CC0); }
#else
void CMenuManager::SaveSettings()
{
	static char RubbishString[48] = "stuffmorestuffevenmorestuff                 etc";

	CFileMgr::SetDirMyDocuments();

	int fileHandle = CFileMgr::OpenFile("gta3.set", "w+");
	if (fileHandle) {
		ControlsManager.SaveSettings(fileHandle);
		CFileMgr::Write(fileHandle, RubbishString, 20);
		CFileMgr::Write(fileHandle, RubbishString, 20);
		CFileMgr::Write(fileHandle, RubbishString, 4);
		CFileMgr::Write(fileHandle, RubbishString, 4);
		CFileMgr::Write(fileHandle, RubbishString, 1);
		CFileMgr::Write(fileHandle, RubbishString, 1);
		CFileMgr::Write(fileHandle, RubbishString, 1);
		CFileMgr::Write(fileHandle, (char*)&TheCamera.m_bHeadBob, 1);
		CFileMgr::Write(fileHandle, (char*)&TheCamera.m_fMouseAccelHorzntl, 4);
		CFileMgr::Write(fileHandle, (char*)&TheCamera.m_fMouseAccelVertical, 4);
		CFileMgr::Write(fileHandle, (char*)&MousePointerStateHelper.bInvertVertically, 1);
		CFileMgr::Write(fileHandle, (char*)&CVehicle::m_bDisableMouseSteering, 1);
		CFileMgr::Write(fileHandle, (char*)&m_PrefsSfxVolume, 1);
		CFileMgr::Write(fileHandle, (char*)&m_PrefsMusicVolume, 1);
		CFileMgr::Write(fileHandle, (char*)&m_PrefsRadioStation, 1);
		CFileMgr::Write(fileHandle, (char*)&m_PrefsSpeakers, 1);
		CFileMgr::Write(fileHandle, (char*)&m_nPrefsAudio3DProviderIndex, 1);
		CFileMgr::Write(fileHandle, (char*)&m_PrefsDMA, 1);
		CFileMgr::Write(fileHandle, (char*)&m_PrefsBrightness, 1);
		CFileMgr::Write(fileHandle, (char*)&m_PrefsLOD, sizeof(m_PrefsLOD));
		CFileMgr::Write(fileHandle, (char*)&m_PrefsShowSubtitles, 1);
		CFileMgr::Write(fileHandle, (char*)&m_PrefsUseWideScreen, 1);
		CFileMgr::Write(fileHandle, (char*)&m_PrefsVsyncDisp, 1);
		CFileMgr::Write(fileHandle, (char*)&m_PrefsFrameLimiter, 1);
		CFileMgr::Write(fileHandle, (char*)&m_nPrefsVideoMode, 1);
		CFileMgr::Write(fileHandle, (char*)&CMBlur::BlurOn, 1);
		CFileMgr::Write(fileHandle, m_PrefsSkinFile, 256);
		CFileMgr::Write(fileHandle, (char*)&m_ControlMethod, 1);
		CFileMgr::Write(fileHandle, (char*)&m_PrefsLanguage, 1);
	}

	CFileMgr::CloseFile(fileHandle);
	CFileMgr::SetDir("");
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

#if ALL_ORIGINAL_FRONTEND
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
	field_112 = 0;

	if (!m_bSaveMenuActive && TheCamera.GetScreenFadeStatus() != FADE_0)
		return;

	m_bStartGameLoading = false;
	InitialiseChangedLanguageSettings();

	if (CPad::GetPad(0)->GetEscapeJustDown())
		RequestFrontEndStartUp();

	SwitchMenuOnAndOff();

	// Be able to re-open menu correctly.
	if (m_bMenuActive) {

		// Load frontend textures.
		LoadAllTextures();

		// Set save/delete game pages.
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
				m_bStartGameLoading = true;
				RequestFrontEndShutdown();
				m_bLoadingSavedGame = true;
				b_FoundRecentSavedGameWantToLoad = true;
				DMAudio.SetEffectsFadeVol(0);
				DMAudio.SetMusicFadeVol(0);
				DMAudio.ResetTimers(CTimer::GetTimeInMilliseconds());
			}
			else
				SaveLoadFileError_SetUpErrorScreen();
		}

		ProcessButtonPresses();

		// Set binding keys.
		if (pEditString && CPad::EditString(pEditString, 0) == nil) {
			if (*pEditString == 0)
				strcpy(pEditString, "NoName");
			pEditString = nil;
			SaveSettings();
		}

		if (field_113) {
			if (field_456)
				field_456 = 0;
			else {
				pControlEdit = CPad::EditCodesForControls(pControlEdit, 1);
				JoyButtonJustClicked = 0;
				MouseButtonJustClicked = 0;

				if (GetMouseClickLeft())
					MouseButtonJustClicked = 1;
				else if (GetMouseClickRight())
					MouseButtonJustClicked = 3;
				else if (GetMouseClickMiddle())
					MouseButtonJustClicked = 2;
				else if (GetMouseWheelUp())
					MouseButtonJustClicked = 4;
				else if (GetMouseWheelDown())
					MouseButtonJustClicked = 5;
				//XXX two more buttons

				JoyButtonJustClicked = ControlsManager.GetJoyButtonJustDown();

				int32 TypeOfControl = 0;
				if (JoyButtonJustClicked)
					TypeOfControl = 3;
				if (MouseButtonJustClicked)
					TypeOfControl = 2;
				if (*pControlEdit != rsNULL)
					TypeOfControl = 0;

				if (!field_534) {
					DMAudio.PlayFrontEndSound(SOUND_FRONTEND_FAIL, 0);
					pControlEdit = nil;
					field_113 = 0;
					m_KeyPressedCode = -1;
					field_456 = 0;
				}
				else if (!m_bKeyChangeNotProcessed) {
					if (*pControlEdit != rsNULL || MouseButtonJustClicked || JoyButtonJustClicked)
						CheckCodesForControls(TypeOfControl);

					field_535 = 1;
				}
				else {
					DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_SUCCESS, 0);
					for (int i = 0; i < 4; i++)
						ControlsManager.ClearSettingsAssociatedWithAction((e_ControllerAction)m_CurrCntrlAction, (eControllerType)i);
					field_534 = false;
					m_bKeyChangeNotProcessed = false;
					pControlEdit = nil;
					field_113 = 0;
					m_KeyPressedCode = -1;
					field_456 = 0;
				}
			}
		}

		if ((m_nCurrScreen == MENUPAGE_13 || m_nCurrScreen == MENUPAGE_16) && CTimer::GetTimeInMillisecondsPauseMode() > field_558) {
			m_nCurrScreen = m_nPrevScreen;
			m_nCurrOption = 0;
		}

		// Reset pad shaking.
		if (TimeToStopPadShaking && TimeToStopPadShaking < CTimer::GetTimeInMillisecondsPauseMode()) {
			CPad::StopPadsShaking();
			TimeToStopPadShaking = 0;
		}

	} else {
		UnloadTextures();
		m_bRenderGameInMenu = false;
		// byte_5F33E4 = 1;	// unused
		m_nPrevScreen = 0;
		m_nCurrScreen = m_nPrevScreen;
		m_nCurrOption = 0;
		pEditString = nil;
		field_113 = 0;
	}

	if (!m_bStartGameLoading) {
		if (m_bGameNotLoaded)
			DMAudio.Service();
	}
}
#endif

#if ALL_ORIGINAL_FRONTEND
WRAPPER void CMenuManager::ProcessButtonPresses() { EAXJMP(0x4856F0); }
#else
void CMenuManager::ProcessButtonPresses()
{
	if (pEditString || pControlEdit)
		return;

	// Update mouse position
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

	if (m_nCurrScreen == MENUPAGE_MULTIPLAYER_FIND_GAME || m_nCurrScreen == MENUPAGE_SKIN_SELECT || m_nCurrScreen == MENUPAGE_KEYBOARD_CONTROLS) {
		if (m_nCurrScreen == MENUPAGE_SKIN_SELECT)
			m_nCurrExSize = m_nSkinsTotal;

		if (m_nCurrScreen == MENUPAGE_KEYBOARD_CONTROLS)
			m_nCurrExSize = m_ControlMethod ? 30 : 25;

		if (!GetPadBack() || m_nCurrScreen != MENUPAGE_KEYBOARD_CONTROLS || field_535)
			field_535 = 0;
		else if (m_nCurrExLayer == 19) {
			m_nHoverOption = HOVEROPTION_42;
			field_113 = 1;
			field_456 = 1;
			m_bKeyChangeNotProcessed = true;
			pControlEdit = &m_KeyPressedCode;
		}

		bool Trigger = false;
		if (!Trigger) {
			nTimeForSomething = 0;
			Trigger = true;
		}

		if ((CTimer::GetTimeInMillisecondsPauseMode() - nTimeForSomething) > 200) {
			field_520 = 0;
			field_521 = 0;
			field_522 = 0;
			field_523 = 0;
			field_524 = 0;
			nTimeForSomething = CTimer::GetTimeInMillisecondsPauseMode();
		}

		if (CPad::GetPad(0)->NewKeyState.TAB && !CPad::GetPad(0)->OldKeyState.TAB) {
			switch (m_nCurrExLayer) {
			case 9:
				m_nCurrExLayer = 19;
				break;
			case 19:
				m_nCurrExLayer = 21;
				break;
			case 21:
				m_nCurrExLayer = 9;
				break;
			}
			if (m_nCurrScreen == MENUPAGE_SKIN_SELECT && m_nCurrExLayer == 21 && !strcmp(m_aSkinName, m_PrefsSkinFile)) {
				m_nCurrExLayer = 9;
			}
			if (m_nCurrScreen == MENUPAGE_KEYBOARD_CONTROLS && m_nCurrExLayer == 21)
				m_nCurrExLayer = 9;
		}

		if (GetPadForward()) {
			switch (m_nCurrExLayer) {
			case 19:
				if (m_nCurrScreen == MENUPAGE_KEYBOARD_CONTROLS) {
					field_113 = 1;
					field_456 = 1;
					pControlEdit = &m_KeyPressedCode;
				}
				if (m_nCurrScreen == MENUPAGE_SKIN_SELECT) {
					strcpy(m_PrefsSkinFile, m_aSkinName);
					CWorld::Players->SetPlayerSkin(m_PrefsSkinFile);
					m_nCurrExLayer = 9;
				}

				m_nHoverOption = HOVEROPTION_42;
				SaveSettings();
				break;
			case 21:
				strcpy(m_PrefsSkinFile, m_aSkinName);
				CWorld::Players->SetPlayerSkin(m_PrefsSkinFile);
				m_nCurrExLayer = 9;
				break;
			default:
				break;
			}
		}
	}

	// Get number of menu options.
	uint8 NumberOfMenuOptions = GetNumberOfMenuOptions();

	// Select next/previous option with pad. Mouse is done in drawing function.
	if (GetPadMoveUp()) {
		m_nPrevOption = m_nCurrOption;
		m_nCurrOption -= 1;
		m_nCurrExOption -= 1;
		//field_438 -= 1;

		if (aScreens[m_nCurrScreen].m_aEntries[0].m_Action == MENUACTION_LABEL) {
			if (m_nCurrOption < 1)
				m_nCurrOption = NumberOfMenuOptions;
		}
		else {
			if (m_nCurrOption < 0)
				m_nCurrOption = NumberOfMenuOptions;
		}

		if (m_nCurrExOption < 0)
			m_nCurrExOption = 0;

		DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_DENIED, 0);
	}
	else if (GetPadMoveDown()) {
		m_nPrevOption = m_nCurrOption;
		m_nCurrOption += 1;
		m_nCurrExOption += 1;
		//field_438 = += 1;

		if (aScreens[m_nCurrScreen].m_aEntries[0].m_Action == MENUACTION_LABEL) {
			if (m_nCurrOption > NumberOfMenuOptions)
				m_nCurrOption = 1;
		}
		else {
			if (m_nCurrOption > NumberOfMenuOptions)
				m_nCurrOption = 0;
		}

		if (m_nCurrExOption > m_nCurrExSize - 1)
			m_nCurrExOption = m_nCurrExSize - 1;

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
		case MENUPAGE_KEYBOARD_CONTROLS:
			if (!m_bKeyChangeNotProcessed) {
				m_bKeyChangeNotProcessed = true;
				field_534 = 0;
			}
			else
				SwitchToNewScreen(aScreens[m_nCurrScreen].m_PreviousPage[0]);

			PlayEscSound = true;
			break;
		default:
			SwitchToNewScreen(aScreens[m_nCurrScreen].m_PreviousPage[0]);
			PlayEscSound = true;
			break;
		}

		if (PlayEscSound)
			DMAudio.PlayFrontEndSound(SOUND_FRONTEND_EXIT, 0);
	}

	// Set hover options, how it is supposed to be used isn't really clear yet.
	if (GetMouseForward()) {
		switch (m_nHoverOption) {
		case HOVEROPTION_DEFAULT:
			if (m_nCurrOption || m_nCurrScreen != MENUPAGE_PAUSE_MENU)
				m_nCurrOption = m_nPrevOption;

			m_nHoverOption = HOVEROPTION_DEFAULT;
			break;
		case HOVEROPTION_12:
			m_nHoverOption = HOVEROPTION_14;
			break;
		case HOVEROPTION_13:
			m_nHoverOption = HOVEROPTION_15;
			break;
		case HOVEROPTION_19:
			m_nHoverOption = HOVEROPTION_20;
			break;
		case HOVEROPTION_CHANGESKIN:
			if (m_nSkinsTotal > 0) {
				m_pSelectedSkin = m_sSkin.field_304;
				strcpy(m_PrefsSkinFile, m_aSkinName);
				CWorld::Players->SetPlayerSkin(m_PrefsSkinFile);
				SaveSettings();
			}
			break;
		default:
			m_nHoverOption = HOVEROPTION_42;
			break;
		}
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
	}
}
#endif

#if 0
WRAPPER void CMenuManager::ProcessOnOffMenuOptions() { EAXJMP(0x48AE60); }
#else
void CMenuManager::ProcessOnOffMenuOptions()
{
	switch (aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption].m_Action) {
	case MENUACTION_CTRLVIBRATION:
		m_PrefsUseVibration = !m_PrefsUseVibration;

		if (m_PrefsUseVibration) {
			CPad::GetPad(0)->StartShake(350, 150);
			TimeToStopPadShaking = CTimer::GetTimeInMillisecondsPauseMode() + 500;
		}
		DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_SUCCESS, 0);
		break;
	case MENUACTION_CTRLCONFIG:
		CPad::GetPad(0)->Mode++;
		if (CPad::GetPad(0)->Mode > 3)
			CPad::GetPad(0)->Mode = 0;
		DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_SUCCESS, 0);
		break;
	case MENUACTION_CTRLDISPLAY:
		m_DisplayControllerOnFoot = !m_DisplayControllerOnFoot;
		DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_SUCCESS, 0);
		break;
	case MENUACTION_FRAMESYNC:
		m_PrefsVsyncDisp = !m_PrefsVsyncDisp;
		DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_SUCCESS, 0);
		SaveSettings();
		break;
	case MENUACTION_FRAMELIMIT:
		m_PrefsFrameLimiter = !m_PrefsFrameLimiter;
		DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_SUCCESS, 0);
		SaveSettings();
		break;
	case MENUACTION_TRAILS:
		CMBlur::BlurOn = !CMBlur::BlurOn;
		DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_SUCCESS, 0);
		SaveSettings();
		if (CMBlur::BlurOn)
			CMBlur::MotionBlurOpen(Scene.camera);
		else
			CMBlur::MotionBlurClose();
		break;
	case MENUACTION_SUBTITLES:
		m_PrefsShowSubtitles = !m_PrefsShowSubtitles;
		DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_SUCCESS, 0);
		SaveSettings();
		break;
	case MENUACTION_WIDESCREEN:
#ifndef ASPECT_RATIO_SCALE
		m_PrefsUseWideScreen = !m_PrefsUseWideScreen;
#else
		m_PrefsUseWideScreen++;
		if(m_PrefsUseWideScreen > 2)
			m_PrefsUseWideScreen = 0;
#endif
		DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_SUCCESS, 0);
		SaveSettings();
		break;
	case MENUACTION_SETDBGFLAG:
		CTheScripts::DbgFlag = !CTheScripts::DbgFlag;
		DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_SUCCESS, 0);
		break;
	case MENUACTION_SWITCHBIGWHITEDEBUGLIGHT:
		gbBigWhiteDebugLightSwitchedOn = !gbBigWhiteDebugLightSwitchedOn;
		DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_SUCCESS, 0);
		break;
	case MENUACTION_PEDROADGROUPS:
		gbShowPedRoadGroups = !gbShowPedRoadGroups;
		DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_SUCCESS, 0);
		break;
	case MENUACTION_CARROADGROUPS:
		gbShowCarRoadGroups = !gbShowCarRoadGroups;
		DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_SUCCESS, 0);
		break;
	case MENUACTION_COLLISIONPOLYS:
		gbShowCollisionPolys = !gbShowCollisionPolys;
		DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_SUCCESS, 0);
		break;
	case MENUACTION_MP_PLAYERCOLOR:
		assert(0 && "Not implemented");
		break;
	case MENUACTION_SHOWHEADBOB:
		TheCamera.m_bHeadBob = !TheCamera.m_bHeadBob;
		DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_SUCCESS, 0);
		SaveSettings();
		break;
	case MENUACTION_INVVERT:
		MousePointerStateHelper.bInvertVertically = !MousePointerStateHelper.bInvertVertically;
		DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_SUCCESS, 0);
		SaveSettings();
		break;
	case MENUACTION_DYNAMICACOUSTIC: 
		m_PrefsDMA = !m_PrefsDMA;
		DMAudio.SetDynamicAcousticModelingStatus(m_PrefsDMA);
		DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_SUCCESS, 0);
		SaveSettings();
		break;
	case MENUACTION_MOUSESTEER:
		m_bDisableMouseSteering = !m_bDisableMouseSteering;
		DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_SUCCESS, 0);
		SaveSettings();
		break;
	}
}
#endif

#if ALL_ORIGINAL_FRONTEND
WRAPPER void CMenuManager::RequestFrontEndShutdown() { EAXJMP(0x488750); }
#else
void CMenuManager::RequestFrontEndShutdown()
{
	m_bShutDownFrontEndRequested = true;
	DMAudio.ChangeMusicMode(MUSICMODE_GAME);
}
#endif

#if ALL_ORIGINAL_FRONTEND
WRAPPER void CMenuManager::RequestFrontEndStartUp() { EAXJMP(0x488770); }
#else
void CMenuManager::RequestFrontEndStartUp()
{
	m_bStartUpFrontEndRequested = 1;
}
#endif

#if ALL_ORIGINAL_FRONTEND
WRAPPER void CMenuManager::ResetHelperText() { EAXJMP(0x48B470); }
#else
void CMenuManager::ResetHelperText() 
{
	m_nHelperTextMsgId = 0;
	m_nHelperTextAlpha = 300;
}
#endif

void CMenuManager::SaveLoadFileError_SetUpErrorScreen()
{
	// TO-DO: Enum
	switch (PcSaveHelper.m_nHelper) {
		case 1:
		case 2:
		case 3:
			m_nPrevScreen = m_nCurrScreen;
			m_nCurrScreen = MENUPAGE_SAVE_FAILED;
			m_nCurrOption = 0;
			m_nScreenChangeDelayTimer = CTimer::GetTimeInMillisecondsPauseMode();
			break;
		case 4:
		case 5:	
		case 6:
			m_nPrevScreen = m_nCurrScreen;
			m_nCurrScreen = MENUPAGE_LOAD_FAILED;
			m_nCurrOption = 0;
			m_nScreenChangeDelayTimer = CTimer::GetTimeInMillisecondsPauseMode();
			break;
		case 7:
			m_nPrevScreen = m_nCurrScreen;
			m_nCurrScreen = MENUPAGE_LOAD_FAILED_2;
			m_nCurrOption = 0;
			m_nScreenChangeDelayTimer = CTimer::GetTimeInMillisecondsPauseMode();
			break;
		case 8:
		case 9:
		case 10:
			m_nPrevScreen = m_nCurrScreen;
			m_nCurrScreen = MENUPAGE_DELETE_FAILED;
			m_nCurrOption = 0;
			m_nScreenChangeDelayTimer = CTimer::GetTimeInMillisecondsPauseMode();
			break;
		default:
			return;
	}
}

#if ALL_ORIGINAL_FRONTEND
WRAPPER void CMenuManager::SetHelperText(int text) { EAXJMP(0x48B450); }
#else
void CMenuManager::SetHelperText(int text)
{
	m_nHelperTextMsgId = text;
	m_nHelperTextAlpha = 300;
}
#endif

#if ALL_ORIGINAL_FRONTEND
WRAPPER void CMenuManager::ShutdownJustMenu() { EAXJMP(0x488920); }
#else
void CMenuManager::ShutdownJustMenu()
{
	m_bMenuActive = false;
	CTimer::EndUserPause();
}
#endif

float CMenuManager::StretchX(float x)
{
	if (SCREEN_WIDTH == DEFAULT_SCREEN_WIDTH)
		return x;
	else
		// We won't make this SCREEN_SCALE, because many cases relies on stretching and we want the code to be portable.
		// Instead we will use MENU_X_LEFT_ALIGNED or SCREEN_SCALE_X when needed.
		return SCREEN_STRETCH_X(x);
}

float CMenuManager::StretchY(float y)
{
	if (SCREEN_HEIGHT == DEFAULT_SCREEN_HEIGHT)
		return y;
	else
		return SCREEN_STRETCH_Y(y);
}

#if 0
WRAPPER void CMenuManager::SwitchMenuOnAndOff() { EAXJMP(0x488790); }
#else
void CMenuManager::SwitchMenuOnAndOff()
{
	if (!!(CPad::GetPad(0)->NewState.Start && !CPad::GetPad(0)->OldState.Start)
		||  m_bShutDownFrontEndRequested || m_bStartUpFrontEndRequested) {

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
			pControlEdit = nil;
			m_bShutDownFrontEndRequested = false;
			DisplayComboButtonErrMsg = 0;
			CPad::GetPad(0)->Clear(0);
			CPad::GetPad(1)->Clear(0);
			m_nCurrScreen = MENUPAGE_NONE;
		}
	}
	if (m_bSaveMenuActive && !m_bQuitGameNoCD) {
		m_bSaveMenuActive = false;
		m_bMenuActive = true;
		CTimer::StartUserPause();
#ifdef PS2_SAVE_DIALOG
		m_nCurrScreen = MENUPAGE_SAVE;
		m_bRenderGameInMenu = true;
#else
		m_nCurrScreen = MENUPAGE_CHOOSE_SAVE_SLOT;
#endif
		PcSaveHelper.PopulateSlotInfo();
		m_nCurrOption = 0;
	}
/*	// Unused?
	if (m_nCurrScreen != MENUPAGE_SOUND_SETTINGS && gMusicPlaying)
	{
		DMAudio.StopFrontEndTrack();
		OutputDebugString("FRONTEND AUDIO TRACK STOPPED");
		gMusicPlaying = 0;
	}
*/
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
	if (!m_bSpritesLoaded)
		return;

	printf("REMOVE frontend\n");
	for (int i = 0; i < ARRAY_SIZE(FrontendFilenames); ++i)
		m_aFrontEndSprites[i].Delete();

	int frontend = CTxdStore::FindTxdSlot("frontend");
	CTxdStore::RemoveTxd(frontend);

	printf("REMOVE menu textures\n");
	for (int i = 0; i < ARRAY_SIZE(MenuFilenames); ++i)
		m_aMenuSprites[i].Delete();

	int menu = CTxdStore::FindTxdSlot("menu");
	CTxdStore::RemoveTxd(menu);

	m_bSpritesLoaded = false;
}
#endif

#if ALL_ORIGINAL_FRONTEND
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
#if 0
uint8 CMenuManager::GetNumberOfMenuOptions()
{
	uint8 Rows = -1;
	for (int i = 0; i < NUM_MENUROWS; i++) {
		if (aScreens[m_nCurrScreen].m_aEntries[i].m_Action == MENUACTION_NOTHING)
			break;

		++Rows;
	}
	return Rows;
}

void CMenuManager::SwitchToNewScreen(int32 screen)
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
			m_nCurrOption = 0;
			//
			m_nCurrExOption = 0;
			m_nCurrExLayer = 19;
			//
			m_nMenuFadeAlpha = 0;
			m_nScreenChangeDelayTimer = CTimer::GetTimeInMillisecondsPauseMode();
		}
		else {
			m_nPrevScreen = MENUPAGE_NONE;
			m_nCurrScreen = MENUPAGE_NONE;
			m_nCurrOption = 0;
		}
	}

	// Set player skin.
	if (m_nCurrScreen == MENUPAGE_SKIN_SELECT) {
		CPlayerSkin::BeginFrontendSkinEdit();
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

void CMenuManager::SetDefaultPreferences(int32 screen)
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
		lodMultiplier = 1.2f;
		CMBlur::BlurOn = true;
		CMBlur::MotionBlurOpen(Scene.camera);
		m_PrefsUseVibration = false;
		m_PrefsShowSubtitles = true;
		m_nDisplayVideoMode = m_nPrefsVideoMode;
		m_PrefsUseWideScreen = false;
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
	}
}
#endif

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
		(CPad::GetPad(0)->NewState.LeftStickY < 0 && !(CPad::GetPad(0)->OldState.LeftStickY < 0));
}

bool GetPadMoveDown()
{
	return
		(CPad::GetPad(0)->NewState.DPadDown && !CPad::GetPad(0)->OldState.DPadDown) ||
		(CPad::GetPad(0)->NewKeyState.DOWN && !CPad::GetPad(0)->OldKeyState.DOWN) ||
		(CPad::GetPad(0)->NewState.LeftStickY > 0 && !(CPad::GetPad(0)->OldState.LeftStickY > 0));
}

bool GetPadMoveLeft()
{
	return
		(CPad::GetPad(0)->NewState.DPadLeft && !CPad::GetPad(0)->OldState.DPadLeft) ||
		(CPad::GetPad(0)->NewKeyState.LEFT && !CPad::GetPad(0)->OldKeyState.LEFT) ||
		(CPad::GetPad(0)->NewState.LeftStickX < 0 && !(CPad::GetPad(0)->OldState.LeftStickX < 0));
}

bool GetPadMoveRight()
{
	return
		(CPad::GetPad(0)->NewState.DPadRight && !CPad::GetPad(0)->OldState.DPadRight) ||
		(CPad::GetPad(0)->NewKeyState.RIGHT && !CPad::GetPad(0)->OldKeyState.RIGHT) ||
		(CPad::GetPad(0)->NewState.LeftStickX > 0 && !(CPad::GetPad(0)->OldState.LeftStickX > 0));
}

bool GetMouseClickLeft()
{
	return
		(CPad::GetPad(0)->NewMouseControllerState.LMB && !CPad::GetPad(0)->OldMouseControllerState.LMB);
}

bool GetMouseClickRight()
{
	return
		(CPad::GetPad(0)->NewMouseControllerState.RMB && !CPad::GetPad(0)->OldMouseControllerState.RMB);
}

bool GetMouseClickMiddle()
{
	return
		(CPad::GetPad(0)->NewMouseControllerState.MMB && !CPad::GetPad(0)->OldMouseControllerState.MMB);
}

bool GetMouseWheelUp()
{
	return
		(CPad::GetPad(0)->NewMouseControllerState.WHEELUP && !CPad::GetPad(0)->OldMouseControllerState.WHEELUP);
}

bool GetMouseWheelDown()
{
	return
		(CPad::GetPad(0)->NewMouseControllerState.WHEELDN && !CPad::GetPad(0)->OldMouseControllerState.WHEELDN);
}


bool GetMouseForward()
{
	return GetMouseClickLeft();
}

bool GetMouseBack()
{
	return GetMouseClickRight();
}

bool GetMousePos()
{
	return 
		(CPad::GetPad(0)->NewMouseControllerState.x != 0.0f || CPad::GetPad(0)->OldMouseControllerState.y != 0.0f);
}

bool GetMouseMoveLeft()
{
	return GetMouseWheelDown();
}

bool GetMouseMoveRight()
{
	return GetMouseWheelUp();
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
#if ALL_ORIGINAL_FRONTEND
#else
	InjectHook(0x4856F0, &CMenuManager::ProcessButtonPresses, PATCH_JUMP);

	for (int i = 1; i < ARRAY_SIZE(aScreens); i++)
		Patch(0x611930 + sizeof(CMenuScreen) * i, aScreens[i]);
#endif
	InjectHook(0x485100, &CMenuManager::Process, PATCH_JUMP);
	InjectHook(0x47A230, &CMenuManager::LoadAllTextures, PATCH_JUMP);
	InjectHook(0x47A540, &CMenuManager::DrawFrontEnd, PATCH_JUMP);

	InjectHook(0x48ABE0, &CMenuManager::StretchX, PATCH_JUMP);
	InjectHook(0x48AC20, &CMenuManager::StretchY, PATCH_JUMP);
	InjectHook(0x488EE0, &CMenuManager::LoadSettings, PATCH_JUMP);
	InjectHook(0x488CC0, &CMenuManager::SaveSettings, PATCH_JUMP);
	InjectHook(0x47A440, &CMenuManager::UnloadTextures, PATCH_JUMP);
	InjectHook(0x48AB40, &CMenuManager::DoSettingsBeforeStartingAGame, PATCH_JUMP);
	InjectHook(0x48AE60, &CMenuManager::ProcessOnOffMenuOptions, PATCH_JUMP);
ENDPATCHES