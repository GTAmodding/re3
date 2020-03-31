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
#include "GenericGameStorage.h"
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

#define DONT_USE_SUSPICIOUS_FUNCS 1
#define TIDY_UP_PBP // ProcessButtonPresses
#define MAX_VISIBLE_LIST_ROW 30
#define LIST_HEIGHT 263.0f

#ifdef PS2_LIKE_MENU
BottomBarOption bbNames[8];
int bbTabCount = 0;
bool bottomBarActive = false;
bool reverseAlpha = false;
int pendingScreen = -1;
int pendingOption = -1;
int curBottomBarOption = -1;
int hoveredBottomBarOption = -1;
#endif

WRAPPER void CMenuManager::PrintController(void) { EAXJMP(0x483990); }

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

int32 &CMenuManager::sthWithButtons = *(int32*)0x8E2880;
int32 &CMenuManager::sthWithButtons2 = *(int32*)0x942F88;

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
bool &holdingScrollBar = *(bool*)0x628D59;
//int32 *pControlTemp = 0;

#ifndef MASTER
bool CMenuManager::m_PrefsMarketing = false;
bool CMenuManager::m_PrefsDisableTutorials = false;
#endif // !MASTER

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

#define isPlainTextScreen(screen) (screen == MENUPAGE_BRIEFS || screen == MENUPAGE_STATS)

#ifdef PS2_LIKE_MENU
#define ChangeScreen(screen, option, updateDelay, withReverseAlpha) \
	do { \
		if (reverseAlpha) { \
			m_nPrevScreen = m_nCurrScreen; \
			m_nCurrScreen = pendingScreen; \
			m_nCurrOption = pendingOption; \
			reverseAlpha = false; \
			if (updateDelay) \
				m_nScreenChangeDelayTimer = CTimer::GetTimeInMillisecondsPauseMode(); \
		} \
		if (withReverseAlpha) { \
			pendingOption = option; \
			pendingScreen = screen; \
			reverseAlpha = true; \
		} else { \
			m_nPrevScreen = m_nCurrScreen; \
			m_nCurrScreen = screen; \
			m_nCurrOption = option; \
			if (updateDelay) \
				m_nScreenChangeDelayTimer = CTimer::GetTimeInMillisecondsPauseMode(); \
		} \
		m_nMenuFadeAlpha = 255; \
	} while(0)
#else
#define ChangeScreen(screen, option, updateDelay, clearAlpha) \
	do { \
		m_nPrevScreen = m_nCurrScreen; \
		m_nCurrScreen = screen; \
		m_nCurrOption = option; \
		if(updateDelay) \
			m_nScreenChangeDelayTimer = CTimer::GetTimeInMillisecondsPauseMode(); \
		if(clearAlpha) \
			m_nMenuFadeAlpha = 0; \
	} while(0)
#endif

#define ProcessSlider(value, increaseAction, decreaseAction, hoverStartX, hoverEndX) \
	do { \
		lastBarX = DisplaySlider(SCREEN_STRETCH_FROM_RIGHT(MENUSLIDER_X + columnWidth), MENU_Y(bitAboveNextItemY), MENU_Y(freeSpaceInLine), MENU_Y(usableLineHeight), MENU_X(MENUSLIDER_UNK), value); \
		if (i != m_nCurrOption || !itemsAreSelectable) \
			break; \
		 \
		if (CheckHover(hoverStartX, lastBarX - MENU_X(10.0f), MENU_Y(nextYToUse), MENU_Y(28.0f + nextYToUse))) \
			m_nHoverOption = decreaseAction; \
		 \
		if (!CheckHover(MENU_X(10.0f) + lastBarX, hoverEndX, MENU_Y(nextYToUse), MENU_Y(28.0f + nextYToUse))) \
			break; \
		 \
		m_nHoverOption = increaseAction; \
		if (m_nMousePosX < SCREEN_STRETCH_FROM_RIGHT(MENUSLIDER_X + columnWidth)) \
			m_nHoverOption = HOVEROPTION_NOT_HOVERING; \
	} while(0)

// --- Functions not in the game/inlined starts

inline void
CMenuManager::ScrollUpListByOne() 
{
	if (m_nSelectedListRow == m_nFirstVisibleRowOnList) {
		if (m_nFirstVisibleRowOnList > 0) {
			m_nSelectedListRow--;
			m_nFirstVisibleRowOnList--;
			m_nCurListItemY -= LIST_HEIGHT / m_nTotalListRow;
		}
	} else {
		m_nSelectedListRow--;
	}
}

inline void
CMenuManager::ScrollDownListByOne()
{
	if (m_nSelectedListRow == m_nFirstVisibleRowOnList + MAX_VISIBLE_LIST_ROW - 1) {
		if (m_nFirstVisibleRowOnList < m_nTotalListRow - MAX_VISIBLE_LIST_ROW) {
			m_nSelectedListRow++;
			m_nFirstVisibleRowOnList++;
			m_nCurListItemY += LIST_HEIGHT / m_nTotalListRow;
		}
	} else {
		if (m_nSelectedListRow < m_nTotalListRow - 1) {
			m_nSelectedListRow++;
		}
	}
}

inline void
CMenuManager::PageUpList(bool playSoundOnSuccess)
{
	if (m_nTotalListRow > MAX_VISIBLE_LIST_ROW) {
		if (m_nFirstVisibleRowOnList > 0) {
			if(playSoundOnSuccess)
				DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_DENIED, 0);

			m_nFirstVisibleRowOnList = max(0, m_nFirstVisibleRowOnList - MAX_VISIBLE_LIST_ROW);
			m_nSelectedListRow = min(m_nSelectedListRow, m_nFirstVisibleRowOnList + MAX_VISIBLE_LIST_ROW - 1);
		} else {
			m_nFirstVisibleRowOnList = 0;
			m_nSelectedListRow = 0;
		}
		m_nCurListItemY = (LIST_HEIGHT / m_nTotalListRow) * m_nFirstVisibleRowOnList;
	}
}

inline void
CMenuManager::PageDownList(bool playSoundOnSuccess)
{
	if (m_nTotalListRow > MAX_VISIBLE_LIST_ROW) {
		if (m_nFirstVisibleRowOnList < m_nTotalListRow - MAX_VISIBLE_LIST_ROW) {
			if(playSoundOnSuccess)
				DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_DENIED, 0);

			m_nFirstVisibleRowOnList = min(m_nFirstVisibleRowOnList + MAX_VISIBLE_LIST_ROW, m_nTotalListRow - MAX_VISIBLE_LIST_ROW);
			m_nSelectedListRow = max(m_nSelectedListRow, m_nFirstVisibleRowOnList);
		} else {
			m_nFirstVisibleRowOnList = m_nTotalListRow - MAX_VISIBLE_LIST_ROW;
			m_nSelectedListRow = m_nTotalListRow - 1;
		}
		m_nCurListItemY = (LIST_HEIGHT / m_nTotalListRow) * m_nFirstVisibleRowOnList;
	}
}

inline void
CMenuManager::ThingsToDoBeforeLeavingPage()
{
	if ((m_nCurrScreen == MENUPAGE_SKIN_SELECT) && strcmp(m_aSkinName, m_PrefsSkinFile) != 0) {
		CWorld::Players[0].SetPlayerSkin(m_PrefsSkinFile);
	} else if (m_nCurrScreen == MENUPAGE_SOUND_SETTINGS) {
		if (m_nPrefsAudio3DProviderIndex != -1)
			m_nPrefsAudio3DProviderIndex = DMAudio.GetCurrent3DProviderIndex();
#ifdef TIDY_UP_PBP
		DMAudio.StopFrontEndTrack();
		OutputDebugString("FRONTEND AUDIO TRACK STOPPED");
#endif
	} else if (m_nCurrScreen == MENUPAGE_GRAPHICS_SETTINGS) {
		m_nDisplayVideoMode = m_nPrefsVideoMode;
	}

	if (m_nCurrScreen == MENUPAGE_SKIN_SELECT) {
		CPlayerSkin::EndFrontendSkinEdit();
	}

	if ((m_nCurrScreen == MENUPAGE_SKIN_SELECT) || (m_nCurrScreen == MENUPAGE_KEYBOARD_CONTROLS)) {
		m_nTotalListRow = 0;
	}
}

// ------ Functions not in the game/inlined ends

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
		m_PrefsBrightness += value * (512/16);
		m_PrefsBrightness = clamp(m_PrefsBrightness, 0, 511);
		break;
	case MENUACTION_DRAWDIST:
		if(value > 0)
			m_PrefsLOD += ((1.8f - 0.8f) / 16.0f);
		else
			m_PrefsLOD -= ((1.8f - 0.8f) / 16.0f);
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

#if DONT_USE_SUSPICIOUS_FUNCS
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

#if DONT_USE_SUSPICIOUS_FUNCS
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
	CCamera::m_bUseMouse3rdPerson = m_ControlMethod == CONTROL_STANDART;
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
		
		CFont::SetRightJustifyOn();
		CFont::SetFontStyle(FONT_HEADING);
#ifdef PS2_LIKE_MENU
		CFont::SetColor(CRGBA(0, 0, 0, 255));
		CFont::SetScale(MENU_X(MENUHEADER_WIDTH), MENU_Y(1.3f));
		CFont::PrintString(MENU_X_RIGHT_ALIGNED(50.0f), SCREEN_SCALE_FROM_BOTTOM(75.0f), TheText.Get(aScreens[m_nCurrScreen].m_ScreenName));
#else
		CFont::SetColor(CRGBA(0, 0, 0, FadeIn(255)));
		CFont::SetScale(MENU_X(MENUHEADER_WIDTH), MENU_Y(MENUHEADER_HEIGHT));
		CFont::PrintString(SCREEN_SCALE_FROM_RIGHT(MENUHEADER_POS_X), SCREEN_SCALE_FROM_BOTTOM(MENUHEADER_POS_Y), TheText.Get(aScreens[m_nCurrScreen].m_ScreenName));
#endif
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
			if (Slots[m_nCurrSaveSlot + 1] == SLOT_EMPTY)
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

#ifdef PS2_LIKE_MENU
	bool itemsAreSelectable = !bottomBarActive;
#else
	bool itemsAreSelectable = true;
#endif
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
			columnWidth = 320;
			headerHeight = 60;
			lineHeight = 24;
			CFont::SetFontStyle(FONT_HEADING);
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
#ifdef PS2_SAVE_DIALOG
		case MENUPAGE_SAVE:
			columnWidth = 180;
			headerHeight = 60;
			lineHeight = 24;
			CFont::SetFontStyle(FONT_BANK);
			CFont::SetScale(MENU_X(actionTextScaleX = 0.75f), MENU_Y(actionTextScaleY = 0.9f));
			break;
#endif
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
			if (m_bWaitingForNewKeyBind)
				itemsAreSelectable = false;

			CMenuManager::DrawControllerScreenExtraText(nextYToUse - 8.0f, 350, lineHeight);
			break;
		default:
			break;
	}

	float usableLineHeight = lineHeight * 0.9f; // also height of biggest bar in slider
	float freeSpaceInLine = lineHeight * 0.1f; // also height of smallest bar in slider(weird)
	bool foundTheHoveringItem = false;
	wchar unicodeTemp[64];

	for (int i = 0; i < NUM_MENUROWS; ++i) {
		if (aScreens[m_nCurrScreen].m_aEntries[i].m_Action != MENUACTION_LABEL && aScreens[m_nCurrScreen].m_aEntries[i].m_EntryName[0] != '\0') {
			wchar *rightText = nil;
			wchar *leftText;

			if (aScreens[m_nCurrScreen].m_aEntries[i].m_SaveSlot >= SAVESLOT_1 && aScreens[m_nCurrScreen].m_aEntries[i].m_SaveSlot <= SAVESLOT_8) {
				CFont::SetRightJustifyOff();
				leftText = GetNameOfSavedGame(i - 1);

				if (Slots[i] != SLOT_EMPTY)
					rightText = GetSavedGameDateAndTime(i - 1);

				if (leftText[0] == '\0') {
					sprintf(gString, "FEM_SL%d", i);
					leftText = TheText.Get(gString);
				}
			} else {
				leftText = TheText.Get(aScreens[m_nCurrScreen].m_aEntries[i].m_EntryName);
			}

			switch (aScreens[m_nCurrScreen].m_aEntries[i].m_Action) {
			case MENUACTION_CHANGEMENU: {
				switch (aScreens[m_nCurrScreen].m_aEntries[i].m_TargetMenu) {
					case MENUPAGE_MULTIPLAYER_MAP:
						switch (sthWithButtons) {
							case 0:
								rightText = TheText.Get("FEM_MA0");
								break;
							case 1:
								rightText = TheText.Get("FEM_MA1");
								break;
							case 2:
								rightText = TheText.Get("FEM_MA2");
								break;
							case 3:
								rightText = TheText.Get("FEM_MA3");
								break;
							case 4:
								rightText = TheText.Get("FEM_MA4");
								break;
							case 5:
								rightText = TheText.Get("FEM_MA5");
								break;
							case 6:
								rightText = TheText.Get("FEM_MA6");
								break;
							case 7:
								rightText = TheText.Get("FEM_MA7");
								break;
							default:
								break;
						}
						break;
					case MENUPAGE_MULTIPLAYER_MODE:
						switch (sthWithButtons2) {
							case 0:
								rightText = TheText.Get("FEN_TY0");
								break;
							case 1:
								rightText = TheText.Get("FEN_TY1");
								break;
							case 2:
								rightText = TheText.Get("FEN_TY2");
								break;
							case 3:
								rightText = TheText.Get("FEN_TY3");
								break;
							case 4:
								rightText = TheText.Get("FEN_TY4");
								break;
							case 5:
								rightText = TheText.Get("FEN_TY5");
								break;
							case 6:
								rightText = TheText.Get("FEN_TY6");
								break;
							case 7:
								rightText = TheText.Get("FEN_TY7");
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
					rightText = TheText.Get("FEM_ON");
				else
					rightText = TheText.Get("FEM_OFF");
				break;
			case MENUACTION_CTRLCONFIG:
				switch (CPad::GetPad(0)->Mode) {
				case 0:
					rightText = TheText.Get("FEC_CF1");
					break;
				case 1:
					rightText = TheText.Get("FEC_CF2");
					break;
				case 2:
					rightText = TheText.Get("FEC_CF3");
					break;
				case 3:
					rightText = TheText.Get("FEC_CF4");
					break;
				}
				break;
			case MENUACTION_CTRLDISPLAY:
				if (m_DisplayControllerOnFoot)
					rightText = TheText.Get("FEC_ONF");
				else
					rightText = TheText.Get("FEC_INC");
				break;
			case MENUACTION_FRAMESYNC:
				rightText = TheText.Get(m_PrefsVsyncDisp ? "FEM_ON" : "FEM_OFF");
				break;
			case MENUACTION_FRAMELIMIT:
				rightText = TheText.Get(m_PrefsFrameLimiter ? "FEM_ON" : "FEM_OFF");
				break;
			case MENUACTION_TRAILS:
				rightText = TheText.Get(CMBlur::BlurOn ? "FEM_ON" : "FEM_OFF");
				break;
			case MENUACTION_SUBTITLES:
				rightText = TheText.Get(m_PrefsShowSubtitles ? "FEM_ON" : "FEM_OFF");
				break;
			case MENUACTION_WIDESCREEN:
#ifndef ASPECT_RATIO_SCALE
				rightText = TheText.Get(m_PrefsUseWideScreen ? "FEM_ON" : "FEM_OFF");
#else
				switch (m_PrefsUseWideScreen) {
				case AR_AUTO:
					rightText = (wchar*)L"AUTO";
					break;
				case AR_4_3:
					rightText = (wchar*)L"4:3";
					break;
				case AR_16_9:
					rightText = (wchar*)L"16:9";
					break;
				}
#endif
				break;
			case MENUACTION_RADIO:
				if (m_PrefsRadioStation > USERTRACK)
					break;

				sprintf(gString, "FEA_FM%d", m_PrefsRadioStation);
				rightText = TheText.Get(gString);
				break;
			case MENUACTION_SETDBGFLAG:
				rightText = TheText.Get(CTheScripts::IsDebugOn() ? "FEM_ON" : "FEM_OFF");
				break;
			case MENUACTION_SWITCHBIGWHITEDEBUGLIGHT:
				rightText = TheText.Get(gbBigWhiteDebugLightSwitchedOn ? "FEM_ON" : "FEM_OFF");
				break;
			case MENUACTION_PEDROADGROUPS:
				rightText = TheText.Get(gbShowPedRoadGroups ? "FEM_ON" : "FEM_OFF");
				break;
			case MENUACTION_CARROADGROUPS:
				rightText = TheText.Get(gbShowCarRoadGroups ? "FEM_ON" : "FEM_OFF");
				break;
			case MENUACTION_COLLISIONPOLYS:
				rightText = TheText.Get(gbShowCollisionPolys ? "FEM_ON" : "FEM_OFF");
				break;
			case MENUACTION_SHOWCULL:
				rightText = TheText.Get(gbShowCullZoneDebugStuff ? "FEM_ON" : "FEM_OFF");
				break;
			case MENUACTION_SHOWHEADBOB:
				rightText = TheText.Get(TheCamera.m_bHeadBob ? "FEM_ON" : "FEM_OFF");
				break;
			case MENUACTION_INVVERT:
				rightText = TheText.Get(MousePointerStateHelper.bInvertVertically ? "FEM_OFF" : "FEM_ON");
				break;
			case MENUACTION_SCREENRES:
				AsciiToUnicode(_psGetVideoModeList()[m_nDisplayVideoMode], unicodeTemp);
				rightText = unicodeTemp;
				break;
			case MENUACTION_AUDIOHW:
				if (m_nPrefsAudio3DProviderIndex == -1)
					rightText = TheText.Get("FEA_NAH");
				else {
					char *provider = DMAudio.Get3DProviderName(m_nPrefsAudio3DProviderIndex);

					if (!strcmp(strupr(provider), "DIRECTSOUND3D HARDWARE SUPPORT")) {
						strcpy(provider, "DSOUND3D HARDWARE SUPPORT");
					} else if (!strcmp(strupr(provider), "DIRECTSOUND3D SOFTWARE EMULATION")) {
						strcpy(provider, "DSOUND3D SOFTWARE EMULATION");
					}
					AsciiToUnicode(provider, unicodeTemp);
					rightText = unicodeTemp;
				}
				break;
			case MENUACTION_SPEAKERCONF: {
				if (m_nPrefsAudio3DProviderIndex == -1)
					rightText = TheText.Get("FEA_NAH");
				else {
					switch (m_PrefsSpeakers) {
					case 0:
						rightText = TheText.Get("FEA_2SP");
						break;
					case 1:
						rightText = TheText.Get("FEA_EAR");
						break;
					case 2:
						rightText = TheText.Get("FEA_4SP");
						break;
					}
				}
				break;
			}
			case MENUACTION_CTRLMETHOD: {
				switch (m_ControlMethod) {
				case 0:
					leftText = TheText.Get("FET_SCN");
					break;
				case 1:
					leftText = TheText.Get("FET_CCN");
					break;
				}
				break;
			}
			case MENUACTION_DYNAMICACOUSTIC:
				rightText = TheText.Get(m_PrefsDMA ? "FEM_ON" : "FEM_OFF");
				break;
			case MENUACTION_MOUSESTEER:
				rightText = TheText.Get(CVehicle::m_bDisableMouseSteering ? "FEM_OFF" : "FEM_ON");
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

						m_nHoverOption = HOVEROPTION_RANDOM_ITEM;
						foundTheHoveringItem = true;
						break;
					}
					m_nHoverOption = HOVEROPTION_NOT_HOVERING;
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
				CFont::PrintString(itemX, itemY, leftText);
				if (rightText) {
					if (!CFont::Details.centre)
						CFont::SetRightJustifyOn();
					
					if(!strcmp(aScreens[m_nCurrScreen].m_aEntries[i].m_EntryName, "FED_RES") 
						&& !m_bGameNotLoaded && textLayer == 1) {
						CFont::SetColor(CRGBA(155, 117, 6, FadeIn(255)));
					}
					CFont::PrintString(MENU_X_RIGHT_ALIGNED(columnWidth - textLayer), itemY, rightText);
				}
				if (i == m_nCurrOption && itemsAreSelectable){
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
					&& m_nCurrScreen == MENUPAGE_SOUND_SETTINGS && m_nPrefsAudio3DProviderIndex != -1) {

					m_nPrefsAudio3DProviderIndex = DMAudio.GetCurrent3DProviderIndex();
					SetHelperText(3);
				}
			}
			if (m_nDisplayVideoMode != m_nPrefsVideoMode) {
				if (strcmp(aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption].m_EntryName, "FED_RES") != 0
					&& m_nCurrScreen == MENUPAGE_GRAPHICS_SETTINGS) {
					m_nDisplayVideoMode = m_nPrefsVideoMode;
					SetHelperText(3);
				}
			}

			// Sliders
			// We stretch slider start X here(like original code), because it will always be center of screen
			int lastBarX;
			switch (aScreens[m_nCurrScreen].m_aEntries[i].m_Action) {
				case MENUACTION_BRIGHTNESS:
					ProcessSlider(m_PrefsBrightness / 512.0f, HOVEROPTION_INCREASE_BRIGHTNESS, HOVEROPTION_DECREASE_BRIGHTNESS, MENU_X_LEFT_ALIGNED(170.0f), SCREEN_WIDTH);
					break;
				case MENUACTION_DRAWDIST:
					ProcessSlider((m_PrefsLOD - 0.8f) * 1.0f, HOVEROPTION_INCREASE_DRAWDIST, HOVEROPTION_DECREASE_DRAWDIST, MENU_X_LEFT_ALIGNED(170.0f), SCREEN_WIDTH);
					break;
				case MENUACTION_MUSICVOLUME:
					ProcessSlider(m_PrefsMusicVolume / 128.0f, HOVEROPTION_INCREASE_MUSICVOLUME, HOVEROPTION_DECREASE_MUSICVOLUME, MENU_X_LEFT_ALIGNED(170.0f), SCREEN_WIDTH);
					break;
				case MENUACTION_SFXVOLUME:
					ProcessSlider(m_PrefsSfxVolume / 128.0f, HOVEROPTION_INCREASE_SFXVOLUME, HOVEROPTION_DECREASE_SFXVOLUME, MENU_X_LEFT_ALIGNED(170.0f), SCREEN_WIDTH);
					break;
				case MENUACTION_MOUSESENS:
					ProcessSlider(TheCamera.m_fMouseAccelHorzntl * 200.0f, HOVEROPTION_INCREASE_MOUSESENS, HOVEROPTION_DECREASE_MOUSESENS, MENU_X_LEFT_ALIGNED(200.0f), SCREEN_WIDTH);
					break;
			}

			nextYToUse += lineHeight * CFont::GetNumberLines(menuXYpadding, nextYToUse, leftText);

			// TODO: This should be rewritten as multiple macro calls instead of loop, radio order is wrong.
			//		And hover detection is missing.
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

	if (m_nCurrScreen == MENUPAGE_CONTROLLER_SETTINGS) {
		PrintController();
	}
/*	else if (m_nCurrScreen == MENUPAGE_SKIN_SELECT_OLD) {
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

#ifdef PS2_LIKE_MENU
	if (m_nCurrScreen == MENUPAGE_NONE) {
		if (m_bGameNotLoaded) {
			if (bbTabCount != 6) {
				bbNames[0] = { "FEB_SAV",MENUPAGE_NEW_GAME };
				bbNames[1] = { "FEB_CON",MENUPAGE_CONTROLLER_PC };
				bbNames[2] = { "FEB_AUD",MENUPAGE_SOUND_SETTINGS };
				bbNames[3] = { "FEB_DIS",MENUPAGE_GRAPHICS_SETTINGS };
				bbNames[4] = { "FEB_LAN",MENUPAGE_LANGUAGE_SETTINGS };
				bbNames[5] = { "FESZ_QU",MENUPAGE_EXIT };
				bbTabCount = 6;
			}
		} else {
			if (bbTabCount != 8) {
				bbNames[0] = { "FEB_STA",MENUPAGE_STATS };
				bbNames[1] = { "FEB_SAV",MENUPAGE_NEW_GAME };
				bbNames[2] = { "FEB_BRI",MENUPAGE_BRIEFS };
				bbNames[3] = { "FEB_CON",MENUPAGE_CONTROLLER_PC };
				bbNames[4] = { "FEB_AUD",MENUPAGE_SOUND_SETTINGS };
				bbNames[5] = { "FEB_DIS",MENUPAGE_GRAPHICS_SETTINGS };
				bbNames[6] = { "FEB_LAN",MENUPAGE_LANGUAGE_SETTINGS };
				bbNames[7] = { "FESZ_QU",MENUPAGE_EXIT };
				bbTabCount = 8;
			}
		}
		m_nCurrScreen = bbNames[0].screenId;
		bottomBarActive = true;
		curBottomBarOption = 0;
	}
#else
	if (m_nCurrScreen == MENUPAGE_NONE) {
		if (m_bGameNotLoaded) {
			m_nCurrScreen = MENUPAGE_START_MENU;
		} else {
			m_nCurrScreen = MENUPAGE_PAUSE_MENU;
		}
	}
#endif

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
	CSprite2d::DrawRect(CRect(MENU_X_LEFT_ALIGNED(30.0f), MENU_Y(50.0f), MENU_X_RIGHT_ALIGNED(30.0f), SCREEN_SCALE_FROM_BOTTOM(50.0f)), CRGBA(0, 0, 0, 175));

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

#ifdef PS2_LIKE_MENU
void CMenuManager::DrawFrontEndNormal()
{
	CSprite2d::InitPerFrame();
	CFont::InitPerFrame();
	RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void*)rwFILTERLINEAR);

	if (!m_bGameNotLoaded) {
		CSprite2d *bg = LoadSplash(nil);
		bg->Draw(CRect(0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT), CRGBA(255, 255, 255, 255));
	} else {
		CSprite2d::DrawRect(CRect(0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT), CRGBA(0, 0, 0, 255));
	}


	RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void*)rwFILTERNEAREST);
	RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*)FALSE);
	RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)FALSE);
	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)TRUE);
	m_aFrontEndSprites[FE2_MAINPANEL_UL].Draw(CRect(MENU_X_LEFT_ALIGNED(0.0f), 0.0f, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2), CRGBA(255, 255, 255, 255));
	m_aFrontEndSprites[FE2_MAINPANEL_UR].Draw(CRect(SCREEN_WIDTH / 2, 0.0f, MENU_X_RIGHT_ALIGNED(0.0f), SCREEN_HEIGHT / 2), CRGBA(255, 255, 255, 255));
	m_aFrontEndSprites[FE2_MAINPANEL_DL].Draw(CRect(MENU_X_LEFT_ALIGNED(0.0f), SCREEN_HEIGHT / 2, SCREEN_WIDTH / 2, SCREEN_HEIGHT), CRGBA(255, 255, 255, 255));
	m_aFrontEndSprites[FE2_MAINPANEL_DR].Draw(CRect(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, MENU_X_RIGHT_ALIGNED(0.0f), SCREEN_HEIGHT), CRGBA(255, 255, 255, 255));
	
	RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void*)rwFILTERLINEAR);
	eFrontendSprites currentSprite;
	switch (m_nCurrScreen) {
		case MENUPAGE_STATS:
		case MENUPAGE_START_MENU:
		case MENUPAGE_PAUSE_MENU:
		case MENUPAGE_EXIT:
			currentSprite = FE_ICONSTATS;
			break;
		case MENUPAGE_LANGUAGE_SETTINGS:
			currentSprite = FE_ICONLANGUAGE;
			break;
		case MENUPAGE_CHOOSE_LOAD_SLOT:
		case MENUPAGE_CHOOSE_DELETE_SLOT:
		case MENUPAGE_NEW_GAME_RELOAD:
		case MENUPAGE_LOAD_SLOT_CONFIRM:
		case MENUPAGE_DELETE_SLOT_CONFIRM:
			currentSprite = FE_ICONSAVE;
			break;
		case MENUPAGE_GRAPHICS_SETTINGS:
			currentSprite = FE_ICONDISPLAY;
			break;
		case MENUPAGE_SOUND_SETTINGS:
			currentSprite = FE_ICONAUDIO;
			break;
		case MENUPAGE_CONTROLLER_PC:
		case MENUPAGE_OPTIONS:
		case MENUPAGE_CONTROLLER_SETTINGS:
		case MENUPAGE_KEYBOARD_CONTROLS:
		case MENUPAGE_MOUSE_CONTROLS:
			currentSprite = FE_ICONCONTROLS;
			break;
		default:
			/*case MENUPAGE_NEW_GAME: */
			/*case MENUPAGE_BRIEFS: */
			currentSprite = FE_ICONBRIEF;
			break;
	}

	m_aFrontEndSprites[currentSprite].Draw(CRect(MENU_X_LEFT_ALIGNED(50.0f), MENU_Y(50.0f), MENU_X_RIGHT_ALIGNED(50.0f), SCREEN_SCALE_FROM_BOTTOM(95.0f)), CRGBA(255, 255, 255, m_nMenuFadeAlpha > 255 ? 255 : m_nMenuFadeAlpha));

	static float fadeAlpha = 0.0f;
	static int lastState = 0;

	// reverseAlpha = PS2 fading (wait for 255->0, then change screen)
	if (m_nMenuFadeAlpha < 255) {
		if (lastState == 1 && !reverseAlpha)
			fadeAlpha = 0.f;

		if (m_nMenuFadeAlpha <= 0 && reverseAlpha) {
			reverseAlpha = false;
			ChangeScreen(pendingScreen, pendingOption, true, false);
		} else {
			float timestep = CTimer::GetCurrentTimeInCycles() / (float)CTimer::GetCyclesPerMillisecond();

			// +20 per every 33 ms (1000.f/30.f - original frame limiter fps)
			if (!reverseAlpha)
				fadeAlpha += (timestep * 100.f) * 20.f / 33.f;
			else
				fadeAlpha = max(0.0f, fadeAlpha - (timestep * 100.f) * 30.f / 33.f);

			m_nMenuFadeAlpha = fadeAlpha;
		} 
		lastState = 0;
	} else {
		if (lastState == 0) fadeAlpha = 255.f;

		if (reverseAlpha) {
			float timestep = CTimer::GetCurrentTimeInCycles() / (float)CTimer::GetCyclesPerMillisecond();
			fadeAlpha -= (timestep * 100.f) * 30.f / 33.f;

			m_nMenuFadeAlpha = fadeAlpha;
		}
		lastState = 1;

		// TODO: what is this? waiting mouse?
		if(field_518 == 4){
			if(m_nHoverOption == HOVEROPTION_3 || m_nHoverOption == HOVEROPTION_4 ||
				m_nHoverOption == HOVEROPTION_5 || m_nHoverOption == HOVEROPTION_6 || m_nHoverOption == HOVEROPTION_7)

				field_518 = 2;
			else
				field_518 = 1;
		}
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

	#define optionWidth		MENU_X(66.0f)
	#define rawOptionHeight	20.0f
	#define optionBottom	SCREEN_SCALE_FROM_BOTTOM(20.0f)
	#define optionTop		SCREEN_SCALE_FROM_BOTTOM(20.0f + rawOptionHeight)
	#define leftPadding		MENU_X_LEFT_ALIGNED(90.0f)
	wchar *str;
	hoveredBottomBarOption = -1;
	if (curBottomBarOption != -1) {

		// This active tab sprite is weird...
		m_aFrontEndSprites[FE2_TABACTIVE].Draw(CRect(leftPadding - MENU_X(2.0f) + (optionWidth) * curBottomBarOption, optionTop,
			leftPadding - MENU_X(5.0f) + optionWidth * (curBottomBarOption + 2), optionBottom + MENU_Y(rawOptionHeight - 9.0f)),
			CRGBA(CRGBA(255, 255, 255, 255)));

		for (int i = 0; i < bbTabCount; i++) {
			float xStart = leftPadding + optionWidth * i;
			if (CheckHover(xStart, xStart + optionWidth, optionTop, optionBottom))
				hoveredBottomBarOption = i;

			CFont::SetFontStyle(FONT_BANK);
			CFont::SetScale(MENU_X(0.35f), MENU_Y(0.7f));
			CFont::SetRightJustifyOff();
			if (hoveredBottomBarOption == i && hoveredBottomBarOption != curBottomBarOption)
				CFont::SetColor(CRGBA(235, 170, 50, 255));
			else {
				if(bottomBarActive || curBottomBarOption == i)
					CFont::SetColor(CRGBA(0, 0, 0, 255));
				else
					CFont::SetColor(CRGBA(0, 0, 0, 110));
			}

			str = TheText.Get(bbNames[i].name);
			
			CFont::PrintString(xStart + MENU_X(4.0f), SCREEN_SCALE_FROM_BOTTOM(39.0f), str);
			
		}
	}
	#undef optionBottom
	#undef optionTop
	#undef leftPadding
	#undef optionWidth
	#undef rawOptionHeight

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
#else
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

		// Famous transparent menu bug
#ifdef FIX_BUGS
		static float fadeAlpha = 0.0f;
		if (m_nMenuFadeAlpha == 0 && fadeAlpha > 1.0f) fadeAlpha = 0.0f;

		float timestep = CTimer::GetCurrentTimeInCycles() / (float)CTimer::GetCyclesPerMillisecond();

		// +20 per every 33 ms (1000.f/30.f - original frame limiter fps)
		fadeAlpha += (timestep * 100.f) * 20.f / 33.f;
		m_nMenuFadeAlpha = fadeAlpha;
#else
		static uint32 LastFade = 0;

		if(CTimer::GetTimeInMillisecondsPauseMode() - LastFade > 10){
			m_nMenuFadeAlpha += 20;
			LastFade = CTimer::GetTimeInMillisecondsPauseMode();
		}
#endif
		
		if (m_nMenuFadeAlpha > 255){
			m_aMenuSprites[currentSprite].Draw(CRect(0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT), CRGBA(255, 255, 255, 255));
		} else {
			RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)TRUE);
			m_aMenuSprites[currentSprite].Draw(CRect(0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT), CRGBA(255, 255, 255, m_nMenuFadeAlpha));
		}
	} else {
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
#ifdef MORE_LANGUAGES
		switch (CMenuManager::m_PrefsLanguage) {
		case LANGUAGE_RUSSIAN:
			CFont::ReloadFonts(FONT_LANGSET_RUSSIAN);
			break;
		default:
			CFont::ReloadFonts(FONT_LANGSET_EFIGS);
			break;
		}
#endif

		switch (CMenuManager::m_PrefsLanguage) {
		case LANGUAGE_FRENCH:
			CGame::frenchGame = true;
			break;
		case LANGUAGE_GERMAN:
			CGame::germanGame = true;
			break;
#ifdef MORE_LANGUAGES
		case LANGUAGE_RUSSIAN:
			CGame::russianGame = true;
			break;
#endif
		default:
			break;
		}
	}
}

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

#if DONT_USE_SUSPICIOUS_FUNCS
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
	m_bMenuStateChanged = false;

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

			if (SlotPopulated) 
				ChangeScreen(MENUPAGE_DELETE_SUCCESS, 0, true, false);
			else
				SaveLoadFileError_SetUpErrorScreen();
		}
		if (m_nCurrScreen == MENUPAGE_SAVING_IN_PROGRESS) {
			int8 SaveSlot = PcSaveHelper.SaveSlot(m_nCurrSaveSlot);
			PcSaveHelper.PopulateSlotInfo();
			if (SaveSlot)
				ChangeScreen(MENUPAGE_SAVE_SUCCESSFUL, 0, true, false);
			else
				SaveLoadFileError_SetUpErrorScreen();
		}
		if (m_nCurrScreen == MENUPAGE_LOADING_IN_PROGRESS) {
			if (CheckSlotDataValid(m_nCurrSaveSlot)) {
				TheCamera.m_bUseMouse3rdPerson = m_ControlMethod == CONTROL_STANDART;
				if (m_PrefsVsyncDisp != m_PrefsVsync)
					m_PrefsVsync = m_PrefsVsyncDisp;
				DMAudio.Service();
				m_bStartGameLoading = true;
				RequestFrontEndShutDown();
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

		if (m_bWaitingForNewKeyBind) {
			if (m_bStartWaitingForKeyBind)
				m_bStartWaitingForKeyBind = false;
			else {
				pControlEdit = CPad::EditCodesForControls(pControlEdit, 1);
				JoyButtonJustClicked = 0;
				MouseButtonJustClicked = 0;

				if (CPad::GetPad(0)->GetLeftMouseJustDown())
					MouseButtonJustClicked = 1;
				else if (CPad::GetPad(0)->GetRightMouseJustUp())
					MouseButtonJustClicked = 3;
				else if (CPad::GetPad(0)->GetMiddleMouseJustUp())
					MouseButtonJustClicked = 2;
				else if (CPad::GetPad(0)->GetMouseWheelUpJustUp())
					MouseButtonJustClicked = 4;
				else if (CPad::GetPad(0)->GetMouseWheelDownJustUp())
					MouseButtonJustClicked = 5;
				// TODO two more buttons

				JoyButtonJustClicked = ControlsManager.GetJoyButtonJustDown();

				int32 TypeOfControl = 0;
				if (JoyButtonJustClicked)
					TypeOfControl = 3;
				if (MouseButtonJustClicked)
					TypeOfControl = 2;
				if (*pControlEdit != rsNULL)
					TypeOfControl = 0;

				if (!m_bKeyIsOK) {
					DMAudio.PlayFrontEndSound(SOUND_FRONTEND_FAIL, 0);
					pControlEdit = nil;
					m_bWaitingForNewKeyBind = false;
					m_KeyPressedCode = -1;
					m_bStartWaitingForKeyBind = false;
				}
				else if (!m_bKeyChangeNotProcessed) {
					if (*pControlEdit != rsNULL || MouseButtonJustClicked || JoyButtonJustClicked)
						CheckCodesForControls(TypeOfControl);

					field_535 = true;
				}
				else {
					DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_SUCCESS, 0);
					for (int i = 0; i < 4; i++)
						ControlsManager.ClearSettingsAssociatedWithAction((e_ControllerAction)m_CurrCntrlAction, (eControllerType)i);
					m_bKeyIsOK = false;
					m_bKeyChangeNotProcessed = false;
					pControlEdit = nil;
					m_bWaitingForNewKeyBind = false;
					m_KeyPressedCode = -1;
					m_bStartWaitingForKeyBind = false;
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
		ChangeScreen(MENUPAGE_NONE, 0, false, false);
		pEditString = nil;
		m_bWaitingForNewKeyBind = false;
	}

	if (!m_bStartGameLoading) {
		if (m_bGameNotLoaded)
			DMAudio.Service();
	}
}
#endif

#if 0
WRAPPER void CMenuManager::ProcessButtonPresses() { EAXJMP(0x4856F0); }
#else
void
CMenuManager::ProcessButtonPresses(void)
{
	if (pEditString || pControlEdit)
		return;

	bool goBack = false;
	bool optionSelected = false;
	bool goUp = false;
	bool goDown = false;
#ifdef TIDY_UP_PBP
	bool assumeIncrease = false;
#endif

	if (!m_bShowMouse && (m_nMouseOldPosX != m_nMousePosX || m_nMouseOldPosY != m_nMousePosY)) {
		m_bShowMouse = true;
	}

	m_nMouseOldPosX = m_nMousePosX;
	m_nMouseOldPosY = m_nMousePosY;
	m_nMousePosX = m_nMouseTempPosX;
	m_nMousePosY = m_nMouseTempPosY;

	if (m_nMousePosX < 0) m_nMousePosX = 0;
	if (m_nMousePosX > SCREEN_WIDTH) m_nMousePosX = SCREEN_WIDTH;
	if (m_nMousePosY < 0) m_nMousePosY = 0;
	if (m_nMousePosY > SCREEN_HEIGHT) m_nMousePosY = SCREEN_HEIGHT;

	if (m_nCurrScreen == MENUPAGE_MULTIPLAYER_FIND_GAME || m_nCurrScreen == MENUPAGE_SKIN_SELECT
		|| m_nCurrScreen == MENUPAGE_KEYBOARD_CONTROLS) {

		if (m_nCurrScreen == MENUPAGE_SKIN_SELECT) {
			m_nTotalListRow = m_nSkinsTotal;
		}
		if (m_nCurrScreen == MENUPAGE_KEYBOARD_CONTROLS) {
			m_nTotalListRow = m_ControlMethod == CONTROL_CLASSIC ? 30 : 25;
			if (m_nSelectedListRow > m_nTotalListRow)
				m_nSelectedListRow = m_nTotalListRow - 1;
		}

#ifndef TIDY_UP_PBP
		if (CPad::GetPad(0)->GetEnterJustDown() || CPad::GetPad(0)->GetCrossJustDown()) {
			m_bShowMouse = 0;
			optionSelected = true;
		}
#endif
		if (CPad::GetPad(0)->GetBackspaceJustDown() && m_nCurrScreen == MENUPAGE_KEYBOARD_CONTROLS && !field_535) {
			if (m_nCurrExLayer == 19) {
				m_nHoverOption = HOVEROPTION_NOT_HOVERING;
				m_bWaitingForNewKeyBind = true;
				m_bStartWaitingForKeyBind = true;
				m_bKeyChangeNotProcessed = true;
				pControlEdit = &m_KeyPressedCode;
			}
		} else {
			field_535 = false;
		}

		static uint32 lastTimeClickedScrollButton = 0;

		if (CTimer::GetTimeInMillisecondsPauseMode() - lastTimeClickedScrollButton >= 200) {
			m_bPressedPgUpOnList = false;
			m_bPressedPgDnOnList = false;
			m_bPressedUpOnList = false;
			m_bPressedDownOnList = false;
			m_bPressedScrollButton = false;
			lastTimeClickedScrollButton = CTimer::GetTimeInMillisecondsPauseMode();
		}

		if (CPad::GetPad(0)->GetTabJustDown()) {
			DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_DENIED, 0);
			m_bShowMouse = false;
			switch (m_nCurrExLayer) {
				case 9:
				default:
					m_nCurrExLayer = 19;
					break;
				case 19:
					m_nCurrExLayer = 21;
					break;
				case 21:
					m_nCurrExLayer = 9;
			}
			if (((m_nCurrScreen == MENUPAGE_SKIN_SELECT) && (m_nCurrExLayer == 21)) && strcmp(m_aSkinName, m_PrefsSkinFile) == 0) {
				m_nCurrExLayer = 9;
			}
			if ((m_nCurrScreen == MENUPAGE_KEYBOARD_CONTROLS) && (m_nCurrExLayer == 21)) {
				m_nCurrExLayer = 9;
			}
		}

		bool pressed = false;
		if (CPad::GetPad(0)->GetUp() || CPad::GetPad(0)->GetAnaloguePadUp() || CPad::GetPad(0)->GetDPadUpJustDown()) {
			m_bShowMouse = false;
			pressed = true;
		} else if (CPad::GetPad(0)->GetMouseWheelUpJustUp()) {
			m_bShowMouse = true;
			pressed = true;
		}

		// Up
		if (pressed) {
			m_nCurrExLayer = 19;
			if (!m_bPressedUpOnList) {
				m_bPressedUpOnList = true;
				lastTimeClickedScrollButton = CTimer::GetTimeInMillisecondsPauseMode();
				DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_DENIED, 0);
				ScrollUpListByOne();
			}
		} else {
			m_bPressedUpOnList = false;
		}

		pressed = false;
		if (CPad::GetPad(0)->GetDown() || CPad::GetPad(0)->GetAnaloguePadDown() || CPad::GetPad(0)->GetDPadDownJustDown()) {
			m_bShowMouse = false;
			pressed = true;
		} else if (CPad::GetPad(0)->GetMouseWheelDownJustDown()) {
			m_bShowMouse = true;
			pressed = true;
		}

		// Down
		if (pressed) {
			m_nCurrExLayer = 19;
			if (!m_bPressedDownOnList) {
				m_bPressedDownOnList = true;
				lastTimeClickedScrollButton = CTimer::GetTimeInMillisecondsPauseMode();
				DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_DENIED, 0);
				ScrollDownListByOne();
			}
		} else {
			m_bPressedDownOnList = false;
		}

		if (m_nCurrScreen != MENUPAGE_KEYBOARD_CONTROLS) {
			if (!CPad::GetPad(0)->GetPageUp()) {
				m_bPressedPgUpOnList = false;
			} else {
				m_nCurrExLayer = 19;
				if (!m_bPressedPgUpOnList) {
					m_bPressedPgUpOnList = true;
					lastTimeClickedScrollButton = CTimer::GetTimeInMillisecondsPauseMode();
					m_bShowMouse = false;
					DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_DENIED, 0);
					PageUpList(false);
				}
			}
			if (!CPad::GetPad(0)->GetPageDown()) {
				m_bPressedPgDnOnList = false;
			} else {
				m_nCurrExLayer = 19;
				if (!m_bPressedPgDnOnList) {
					m_bPressedPgDnOnList = true;
					lastTimeClickedScrollButton = CTimer::GetTimeInMillisecondsPauseMode();
					m_bShowMouse = false;
					DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_DENIED, 0);
					PageDownList(false);
				}
			}
			if (CPad::GetPad(0)->GetHome()) {
				m_nCurrExLayer = 19;
				m_bShowMouse = false;
				DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_DENIED, 0);
				if (m_nTotalListRow >= MAX_VISIBLE_LIST_ROW) {
					m_nFirstVisibleRowOnList = 0;
				}
				m_nSelectedListRow = 0;
				m_nCurListItemY = (LIST_HEIGHT / m_nTotalListRow) * m_nFirstVisibleRowOnList;
			}
			if (CPad::GetPad(0)->GetEnd()) {
				m_nCurrExLayer = 19;
				m_bShowMouse = false;
				DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_DENIED, 0);
				if (m_nTotalListRow >= MAX_VISIBLE_LIST_ROW) {
					m_nFirstVisibleRowOnList = m_nTotalListRow - MAX_VISIBLE_LIST_ROW;
				}
				m_nSelectedListRow = m_nTotalListRow - 1;
				m_nCurListItemY = (LIST_HEIGHT / m_nTotalListRow) * m_nFirstVisibleRowOnList;
			}
		}

#ifndef TIDY_UP_PBP
		if (CPad::GetPad(0)->GetEscapeJustDown() || CPad::GetPad(0)->GetSquareJustDown()) {
			m_bShowMouse = false;
			goBack = true;
		}
#endif

		if (CPad::GetPad(0)->GetLeftMouseJustDown()) {
			switch (m_nHoverOption) {
				case HOVEROPTION_BACK:
					goBack = true;
					break;
				case HOVEROPTION_PAGEUP:
					PageUpList(true);
					break;
				case HOVEROPTION_PAGEDOWN:
					PageDownList(true);
					break;
				case HOVEROPTION_CHANGESKIN:
					if (m_nSkinsTotal > 0) {
						DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_SUCCESS, 0);
						m_pSelectedSkin = m_sSkin.field_304;
						strcpy(m_PrefsSkinFile, m_aSkinName);
						CWorld::Players[0].SetPlayerSkin(m_PrefsSkinFile);
						SaveSettings();
					}
			}
		}

		if (CPad::GetPad(0)->GetLeftMouseJustDown()) {
			switch (m_nHoverOption) {
				case HOVEROPTION_OVER_SCROLL_UP:
					m_nHoverOption = HOVEROPTION_CLICKED_SCROLL_UP;
					break;
				case HOVEROPTION_OVER_SCROLL_DOWN:
					m_nHoverOption = HOVEROPTION_CLICKED_SCROLL_DOWN;
					break;
				case HOVEROPTION_19:
					m_nHoverOption = HOVEROPTION_20;
			}
		} else if ((CPad::GetPad(0)->GetLeftMouseJustUp())
			&& ((m_nHoverOption == HOVEROPTION_CLICKED_SCROLL_UP || (m_nHoverOption == HOVEROPTION_CLICKED_SCROLL_DOWN)))) {
			m_nHoverOption = HOVEROPTION_NOT_HOVERING;
		}

		if (!CPad::GetPad(0)->GetLeftMouse()) {
			holdingScrollBar = false;
		} else {
			if ((m_nHoverOption == HOVEROPTION_HOLDING_SCROLLBAR) || holdingScrollBar) {
				holdingScrollBar = true;
				// TODO: This part is a bit hard to reverse. Not much code tho
				assert(0 && "Not done yet");
			} else {
				switch (m_nHoverOption) {
				case HOVEROPTION_OVER_SCROLL_UP:
				case HOVEROPTION_CLICKED_SCROLL_UP:
					if (!m_bPressedScrollButton) {
						m_bPressedScrollButton = true;
						lastTimeClickedScrollButton = CTimer::GetTimeInMillisecondsPauseMode();
						ScrollUpListByOne();
					}
					break;
				case HOVEROPTION_OVER_SCROLL_DOWN:
				case HOVEROPTION_CLICKED_SCROLL_DOWN:
					if (!m_bPressedScrollButton) {
						m_bPressedScrollButton = true;
						lastTimeClickedScrollButton = CTimer::GetTimeInMillisecondsPauseMode();
						ScrollDownListByOne();
					}
					break;
				default:
					m_bPressedScrollButton = false;
				}
			}
		}
	} else if (isPlainTextScreen(m_nCurrScreen)) {
#ifndef TIDY_UP_PBP
		if (CPad::GetPad(0)->GetEnterJustDown() || CPad::GetPad(0)->GetCrossJustDown() || CPad::GetPad(0)->GetLeftMouseJustDown()) {
			optionSelected = true;
		}
		if (CPad::GetPad(0)->GetEscapeJustDown() || CPad::GetPad(0)->GetSquareJustUp()) {
			if (m_nCurrScreen != MENUPAGE_START_MENU) {
				goBack = true;
			}
		}
#endif
	} else {
		if (CPad::GetPad(0)->GetDownJustDown() || CPad::GetPad(0)->GetAnaloguePadDown() || CPad::GetPad(0)->GetDPadDownJustDown()) {
			m_bShowMouse = false;
			DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_DENIED, 0);
			goDown = true;
		} else if (CPad::GetPad(0)->GetUpJustDown() || CPad::GetPad(0)->GetAnaloguePadUp() || CPad::GetPad(0)->GetDPadUpJustDown()) {
			m_bShowMouse = false;
			DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_DENIED, 0);
			goUp = true;
		}

#ifndef TIDY_UP_PBP
		if ((m_nCurrOption == 0) && (m_nCurrScreen == MENUPAGE_PAUSE_MENU)) {
			if (CPad::GetPad(0)->GetEnterJustUp() || CPad::GetPad(0)->GetCrossJustUp()) {
				m_bShowMouse = false;
				optionSelected = true;
			}
		} else {
			if (CPad::GetPad(0)->GetEnterJustDown() || CPad::GetPad(0)->GetCrossJustDown()) {
				m_bShowMouse = false;
				optionSelected = true;
			}
		}
#endif

		if (CPad::GetPad(0)->GetLeftMouseJustUp()) {
#ifndef TIDY_UP_PBP
			if (((m_nCurrOption == 0) && (m_nCurrScreen == MENUPAGE_PAUSE_MENU)) &&
#else
			if (aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption].m_Action == MENUACTION_RESUME &&
#endif
				(m_nHoverOption == HOVEROPTION_RANDOM_ITEM)) {
				m_nCurrOption = m_nPrevOption;
				optionSelected = true;
			}
		} else if (CPad::GetPad(0)->GetLeftMouseJustDown()) {
#ifdef TIDY_UP_PBP
			if (m_nHoverOption >= HOVEROPTION_RADIO_0 && m_nHoverOption <= HOVEROPTION_RADIO_9) {
				DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_SUCCESS, 0);
				CMenuManager::m_PrefsRadioStation = m_nHoverOption - HOVEROPTION_RADIO_0;
				SaveSettings();
				DMAudio.SetRadioInCar(CMenuManager::m_PrefsRadioStation);
				DMAudio.PlayFrontEndTrack(CMenuManager::m_PrefsRadioStation, 1);
				OutputDebugString("FRONTEND RADIO STATION CHANGED");
			} else if (m_nHoverOption == HOVEROPTION_RANDOM_ITEM
				&& aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption].m_Action != MENUACTION_RESUME) {
				m_nCurrOption = m_nPrevOption;
				optionSelected = true;
			}
#else
			switch (m_nHoverOption) {
			case HOVEROPTION_RADIO_0:
				DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_SUCCESS, 0);
				CMenuManager::m_PrefsRadioStation = HEAD_RADIO;
				SaveSettings();
				DMAudio.SetRadioInCar(CMenuManager::m_PrefsRadioStation);
				DMAudio.PlayFrontEndTrack(CMenuManager::m_PrefsRadioStation, 1);
				OutputDebugString("FRONTEND RADIO STATION CHANGED");
				break;
			case HOVEROPTION_RADIO_1:
				DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_SUCCESS, 0);
				CMenuManager::m_PrefsRadioStation = DOUBLE_CLEF;
				SaveSettings();
				DMAudio.SetRadioInCar(CMenuManager::m_PrefsRadioStation);
				DMAudio.PlayFrontEndTrack(CMenuManager::m_PrefsRadioStation, 1);
				OutputDebugString("FRONTEND RADIO STATION CHANGED");
				break;
			case HOVEROPTION_RADIO_2:
				DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_SUCCESS, 0);
				CMenuManager::m_PrefsRadioStation = JAH_RADIO;
				SaveSettings();
				DMAudio.SetRadioInCar(CMenuManager::m_PrefsRadioStation);
				DMAudio.PlayFrontEndTrack(CMenuManager::m_PrefsRadioStation, 1);
				OutputDebugString("FRONTEND RADIO STATION CHANGED");
				break;
			case HOVEROPTION_RADIO_3:
				DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_SUCCESS, 0);
				CMenuManager::m_PrefsRadioStation = RISE_FM;
				SaveSettings();
				DMAudio.SetRadioInCar(CMenuManager::m_PrefsRadioStation);
				DMAudio.PlayFrontEndTrack(CMenuManager::m_PrefsRadioStation, 1);
				OutputDebugString("FRONTEND RADIO STATION CHANGED");
				break;
			case HOVEROPTION_RADIO_4:
				DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_SUCCESS, 0);
				CMenuManager::m_PrefsRadioStation = LIPS_106;
				SaveSettings();
				DMAudio.SetRadioInCar(CMenuManager::m_PrefsRadioStation);
				DMAudio.PlayFrontEndTrack(CMenuManager::m_PrefsRadioStation, 1);
				OutputDebugString("FRONTEND RADIO STATION CHANGED");
				break;
			case HOVEROPTION_RADIO_5:
				DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_SUCCESS, 0);
				CMenuManager::m_PrefsRadioStation = GAME_FM;
				SaveSettings();
				DMAudio.SetRadioInCar(CMenuManager::m_PrefsRadioStation);
				DMAudio.PlayFrontEndTrack(CMenuManager::m_PrefsRadioStation, 1);
				OutputDebugString("FRONTEND RADIO STATION CHANGED");
				break;
			case HOVEROPTION_RADIO_6:
				DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_SUCCESS, 0);
				CMenuManager::m_PrefsRadioStation = MSX_FM;
				SaveSettings();
				DMAudio.SetRadioInCar(CMenuManager::m_PrefsRadioStation);
				DMAudio.PlayFrontEndTrack(CMenuManager::m_PrefsRadioStation, 1);
				OutputDebugString("FRONTEND RADIO STATION CHANGED");
				break;
			case HOVEROPTION_RADIO_7:
				DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_SUCCESS, 0);
				CMenuManager::m_PrefsRadioStation = FLASHBACK;
				SaveSettings();
				DMAudio.SetRadioInCar(CMenuManager::m_PrefsRadioStation);
				DMAudio.PlayFrontEndTrack(CMenuManager::m_PrefsRadioStation, 1);
				OutputDebugString("FRONTEND RADIO STATION CHANGED");
				break;
			case HOVEROPTION_RADIO_8:
				DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_SUCCESS, 0);
				CMenuManager::m_PrefsRadioStation = CHATTERBOX;
				SaveSettings();
				DMAudio.SetRadioInCar(CMenuManager::m_PrefsRadioStation);
				DMAudio.PlayFrontEndTrack(CMenuManager::m_PrefsRadioStation, 1);
				OutputDebugString("FRONTEND RADIO STATION CHANGED");
				break;
			case HOVEROPTION_RADIO_9:
				DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_SUCCESS, 0);
				CMenuManager::m_PrefsRadioStation = USERTRACK;
				SaveSettings();
				DMAudio.SetRadioInCar(CMenuManager::m_PrefsRadioStation);
				DMAudio.PlayFrontEndTrack(CMenuManager::m_PrefsRadioStation, 1);
				OutputDebugString("FRONTEND RADIO STATION CHANGED");
				break;
			case HOVEROPTION_RANDOM_ITEM:
				if (((m_nCurrOption != 0) || (m_nCurrScreen != MENUPAGE_PAUSE_MENU)) {
					m_nCurrOption = m_nPrevOption;
						optionSelected = true;
				}
				break;
			}
#endif
		}

		if (CPad::GetPad(0)->GetLeftMouse()) {
#ifndef TIDY_UP_PBP
			switch (m_nHoverOption) {
			case HOVEROPTION_INCREASE_BRIGHTNESS:
				CMenuManager::m_PrefsBrightness = CMenuManager::m_PrefsBrightness + 32;
				if (CMenuManager::m_PrefsBrightness < 0) {
					CMenuManager::m_PrefsBrightness = 0;
				}
				if (510 < CMenuManager::m_PrefsBrightness) {
					CMenuManager::m_PrefsBrightness = 511;
				}
				SaveSettings();
				break;
			case HOVEROPTION_DECREASE_BRIGHTNESS:
				CMenuManager::m_PrefsBrightness = CMenuManager::m_PrefsBrightness - 32;
				if (CMenuManager::m_PrefsBrightness < 0) {
					CMenuManager::m_PrefsBrightness = 0;
				}
				if (510 < CMenuManager::m_PrefsBrightness) {
					CMenuManager::m_PrefsBrightness = 511;
				}
				SaveSettings();
				break;
			case HOVEROPTION_INCREASE_DRAWDIST:
				CMenuManager::m_PrefsLOD = CMenuManager::m_PrefsLOD + (1.0f / 16);
				m_PrefsLOD = min(1.8f, m_PrefsLOD);
				CRenderer::ms_lodDistScale = CMenuManager::m_PrefsLOD;
				SaveSettings();
				break;
			case HOVEROPTION_DECREASE_DRAWDIST:
				CMenuManager::m_PrefsLOD = CMenuManager::m_PrefsLOD - (1.0f / 16);
				m_PrefsLOD = max(0.8f, m_PrefsLOD);
				CRenderer::ms_lodDistScale = CMenuManager::m_PrefsLOD;
				SaveSettings();
				break;
			case HOVEROPTION_INCREASE_MUSICVOLUME:
				CMenuManager::m_PrefsMusicVolume = CMenuManager::m_PrefsMusicVolume + 8;
				m_PrefsMusicVolume = clamp(m_PrefsMusicVolume, 0, 127);
				DMAudio.SetMusicMasterVolume(uchar)(CMenuManager::m_PrefsMusicVolume);
				SaveSettings();
				break;
			case HOVEROPTION_DECREASE_MUSICVOLUME:
				CMenuManager::m_PrefsMusicVolume = CMenuManager::m_PrefsMusicVolume - 8;
				if (CMenuManager::m_PrefsMusicVolume < 0) {
					CMenuManager::m_PrefsMusicVolume = 0;
				}
				if (126 < CMenuManager::m_PrefsMusicVolume) {
					CMenuManager::m_PrefsMusicVolume = 127;
				}
				DMAudio.SetMusicMasterVolume(uchar)(CMenuManager::m_PrefsMusicVolume);
				SaveSettings();
				break;
			case HOVEROPTION_INCREASE_SFXVOLUME:
				CMenuManager::m_PrefsSFXVolume = CMenuManager::m_PrefsSFXVolume + 8;
				if (CMenuManager::m_PrefsSFXVolume < 0) {
					CMenuManager::m_PrefsSFXVolume = 0;
				}
				if (126 < CMenuManager::m_PrefsSFXVolume) {
					CMenuManager::m_PrefsSFXVolume = 127;
				}
				DMAudio.SetEffectsMasterVolume(uchar)(CMenuManager::m_PrefsSFXVolume);
				SaveSettings();
				break;
			case HOVEROPTION_DECREASE_SFXVOLUME:
				CMenuManager::m_PrefsSFXVolume = CMenuManager::m_PrefsSFXVolume - 8;
				if (CMenuManager::m_PrefsSFXVolume < 0) {
					CMenuManager::m_PrefsSFXVolume = 0;
				}
				if (126 < CMenuManager::m_PrefsSFXVolume) {
					CMenuManager::m_PrefsSFXVolume = 127;
				}
				DMAudio.SetEffectsMasterVolume(uchar)(CMenuManager::m_PrefsSFXVolume);
				SaveSettings();
				break;
			case HOVEROPTION_INCREASE_MOUSESENS:
				TheCamera.m_fMouseAccelHorzntl += (1.0f / 3000);
				TheCamera.m_fMouseAccelHorzntl = clamp(TheCamera.m_fMouseAccelHorzntl, 1.0f / 3200, 1.0f / 200);
				TheCamera.m_fMouseAccelVertical = TheCamera.m_fMouseAccelHorzntl;
				SaveSettings();
				break;
			case HOVEROPTION_DECREASE_MOUSESENS:
				TheCamera.m_fMouseAccelHorzntl -= (1.0f / 3000);
				TheCamera.m_fMouseAccelHorzntl = clamp(TheCamera.m_fMouseAccelHorzntl, 1.0f / 3200, 1.0f / 200);
				TheCamera.m_fMouseAccelVertical = TheCamera.m_fMouseAccelHorzntl;
				SaveSettings();
				break;
				}
#else
			switch (m_nHoverOption) {
				case HOVEROPTION_INCREASE_BRIGHTNESS:
				case HOVEROPTION_INCREASE_DRAWDIST:
				case HOVEROPTION_INCREASE_MUSICVOLUME:
				case HOVEROPTION_INCREASE_SFXVOLUME:
				case HOVEROPTION_INCREASE_MOUSESENS:
					CheckSliderMovement(1);
					break;
				case HOVEROPTION_DECREASE_BRIGHTNESS:
				case HOVEROPTION_DECREASE_DRAWDIST:
				case HOVEROPTION_DECREASE_MUSICVOLUME:
				case HOVEROPTION_DECREASE_SFXVOLUME:
				case HOVEROPTION_DECREASE_MOUSESENS:
					CheckSliderMovement(-1);
					break;
			}
#endif
			}

		if (CPad::GetPad(0)->GetLeftMouseJustUp() || CPad::GetPad(0)->GetLeftJustUp() || CPad::GetPad(0)->GetRightJustUp()
			|| CPad::GetPad(0)->GetDPadLeftJustUp() || CPad::GetPad(0)->GetDPadRightJustUp()
			|| CPad::GetPad(0)->GetAnaloguePadLeftJustUp() || CPad::GetPad(0)->GetAnaloguePadRightJustUp()
			|| CPad::GetPad(0)->GetMouseWheelUpJustDown() || CPad::GetPad(0)->GetMouseWheelDownJustDown()) {
			int option = aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption].m_Action;
			if (option == MENUACTION_BRIGHTNESS || option == MENUACTION_DRAWDIST)
				DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_SUCCESS, 0);
			else if (option == MENUACTION_SFXVOLUME)
				DMAudio.PlayFrontEndSound(SOUND_FRONTEND_AUDIO_TEST, 0);
			else if (option == MENUACTION_MOUSESENS)
				DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_SUCCESS, 0);

		}
#ifndef TIDY_UP_PBP
		if (CPad::GetPad(0)->GetSquareJustDown()) {
			if (m_nCurrScreen != MENUPAGE_START_MENU && m_nCurrScreen != MENUPAGE_PAUSE_MENU) {
				m_bShowMouse = false;
				goBack = true;
			}
		}

		if (CPad::GetPad(0)->GetEscapeJustDown()) {
			if (m_nCurrScreen != MENUPAGE_START_MENU) {
				m_bShowMouse = false;
				goBack = true;
			}
		}

		if (((goDown) || (goUp)) || (optionSelected)) {
			goBack = false;
		}
#endif
		}

	// Centralized enter/back (except some conditions)
#ifdef TIDY_UP_PBP
	if (aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption].m_Action != MENUACTION_RESUME) {
		if (CPad::GetPad(0)->GetEnterJustDown() || CPad::GetPad(0)->GetCrossJustDown() ||
			(isPlainTextScreen(m_nCurrScreen) && CPad::GetPad(0)->GetLeftMouseJustDown())) {

			if (!isPlainTextScreen(m_nCurrScreen))
				m_bShowMouse = false;

			optionSelected = true;
		}
	} else {
		if (CPad::GetPad(0)->GetEnterJustUp() || CPad::GetPad(0)->GetCrossJustUp()) {
			m_bShowMouse = false;
			optionSelected = true;
		}
	}

	if (!goDown && !goUp && !optionSelected) {
		if (m_nCurrScreen != MENUPAGE_START_MENU) {
			if (isPlainTextScreen(m_nCurrScreen)) {
				if (CPad::GetPad(0)->GetEscapeJustDown() || CPad::GetPad(0)->GetSquareJustUp()) {
					goBack = true;
				}
			} else {
				if (CPad::GetPad(0)->GetEscapeJustDown() || (m_nCurrScreen != MENUPAGE_PAUSE_MENU && CPad::GetPad(0)->GetSquareJustDown())) {
					m_bShowMouse = false;
					goBack = true;
				}
			}
		}
	}
#endif

#ifdef PS2_LIKE_MENU
	if (CPad::GetPad(0)->GetLeftMouseJustDown() && hoveredBottomBarOption != -1) {
		DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_DENIED, 0);
		bottomBarActive = false;
		curBottomBarOption = hoveredBottomBarOption;
		ChangeScreen(bbNames[curBottomBarOption].screenId, 0, true, false);
		return;
	} else if (bottomBarActive) {
		if (CPad::GetPad(0)->GetEnterJustDown() || CPad::GetPad(0)->GetCrossJustDown()) {
			DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_DENIED, 0);
			bottomBarActive = false;

			// If there's a menu change with fade ongoing, finish it now
			if (reverseAlpha)
				m_nMenuFadeAlpha = 0;
			return;
		} else if (CPad::GetPad(0)->GetLeftJustDown() || CPad::GetPad(0)->GetAnaloguePadLeft() || CPad::GetPad(0)->GetDPadLeftJustDown()
			|| CPad::GetPad(0)->GetUpJustDown() || CPad::GetPad(0)->GetAnaloguePadUp() || CPad::GetPad(0)->GetDPadUpJustDown()) {
			m_bShowMouse = false;
			DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_DENIED, 0);
			if (curBottomBarOption > 0)
				curBottomBarOption--;
			ChangeScreen(bbNames[curBottomBarOption].screenId, 0, true, true);
			return;
		} else if (CPad::GetPad(0)->GetRightJustDown() || CPad::GetPad(0)->GetAnaloguePadRight() || CPad::GetPad(0)->GetDPadRightJustDown()
			|| CPad::GetPad(0)->GetDownJustDown() || CPad::GetPad(0)->GetAnaloguePadDown() || CPad::GetPad(0)->GetDPadDownJustDown()) {
			m_bShowMouse = false;
			DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_DENIED, 0);
			if (curBottomBarOption < bbTabCount-1)
				curBottomBarOption++;
			ChangeScreen(bbNames[curBottomBarOption].screenId, 0, true, true);
			return;
		}
		optionSelected = false;
		goDown = false;
		goUp = false;
	}
#endif

	if (goDown && (m_nCurrScreen != MENUPAGE_MULTIPLAYER_FIND_GAME)) {
		m_nCurrOption++;
		if (m_nCurrOption == NUM_MENUROWS || (aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption].m_Action == MENUACTION_NOTHING)) {
			m_nCurrOption = 0;
		}
	}
	if (goUp && (m_nCurrScreen != MENUPAGE_MULTIPLAYER_FIND_GAME)) {
		if (m_nCurrOption == (aScreens[m_nCurrScreen].m_aEntries[0].m_Action == MENUACTION_LABEL)) {
			while (m_nCurrOption != NUM_MENUROWS - 1
				&& aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption + 1].m_Action != MENUACTION_NOTHING) {
				m_nCurrOption++;
			}
		} else {
			m_nCurrOption--;
		}
	}

	if (optionSelected) {
		int option = aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption].m_Action;
		if ((option == MENUACTION_CHANGEMENU) || (option == MENUACTION_POPULATESLOTS_CHANGEMENU)) {
			if (strncmp(aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption].m_EntryName, "FEDS_TB", 8) != 0 &&
				strncmp(aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption].m_EntryName, "FESZ_CA", 8) != 0) {

				if (m_nCurrScreen == MENUPAGE_CHOOSE_DELETE_SLOT) {
					if (Slots[aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption].m_SaveSlot - 1] == SLOT_EMPTY)
						DMAudio.PlayFrontEndSound(SOUND_FRONTEND_FAIL, 0);
					else
						DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_SUCCESS, 0);
				} else
					DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_COMPLETED, 0);
			} else {
				// This is duplicate, back button already processed below
#ifndef TIDY_UP_PBP
				DMAudio.PlayFrontEndSound(SOUND_FRONTEND_EXIT, 0);
				if (m_nCurrScreen == MENUPAGE_SOUND_SETTINGS) {
					DMAudio.StopFrontEndTrack();
					OutputDebugString("FRONTEND AUDIO TRACK STOPPED");
				}
#endif
			}
		} else if (option == MENUACTION_CHECKSAVE) {
			if (Slots[aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption].m_SaveSlot - 1] == SLOT_EMPTY) {
				DMAudio.PlayFrontEndSound(SOUND_FRONTEND_FAIL, 0);
			} else {
				if (m_nCurrScreen != MENUPAGE_NEW_GAME_RELOAD) {
					DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_SUCCESS, 0);
				}
			}
		} else if (option != MENUACTION_CHANGEMENU && option != MENUACTION_BRIGHTNESS && option != MENUACTION_DRAWDIST
			&& option != MENUACTION_MUSICVOLUME && option != MENUACTION_SFXVOLUME
			&& option != MENUACTION_CHECKSAVE && option != MENUACTION_UNK24
			&& option != MENUACTION_MOUSESENS && option != MENUACTION_SCREENRES) {

			DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_SUCCESS, 0);
		}

		if ((m_nCurrScreen == MENUPAGE_KEYBOARD_CONTROLS) || (m_nCurrScreen == MENUPAGE_SKIN_SELECT)) {
			switch (m_nCurrExLayer) {
				default:
					goBack = true;
					break;
				case 19:
					if (m_nCurrScreen == MENUPAGE_KEYBOARD_CONTROLS) {
						m_bWaitingForNewKeyBind = true;
						m_bStartWaitingForKeyBind = true;
						pControlEdit = &m_KeyPressedCode;
					}
					if (m_nCurrScreen == MENUPAGE_SKIN_SELECT) {
						strcpy(m_PrefsSkinFile, m_aSkinName);
						CWorld::Players[0].SetPlayerSkin(m_PrefsSkinFile);
						m_nCurrExLayer = 9;
						CMenuManager::SaveSettings();
					}
					m_nHoverOption = HOVEROPTION_NOT_HOVERING;
					break;
				case 21:
					m_nHoverOption = HOVEROPTION_NOT_HOVERING;
					strcpy(m_PrefsSkinFile, m_aSkinName);
					CWorld::Players[0].SetPlayerSkin(m_PrefsSkinFile);
					m_nCurrExLayer = 9;
					SaveSettings();
					break;
			}
		} else if (aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption].m_TargetMenu == MENUPAGE_NEW_GAME_RELOAD && m_bGameNotLoaded) {
			DoSettingsBeforeStartingAGame();
/*		} else if (m_nCurrScreen == MENUPAGE_KEYBOARD_CONTROLS) {
			// .. either empty or there was some outer if. :shrug: pointless anyway, keyboard_controls is handled in first if.
*/
		} else if (m_nCurrScreen == MENUPAGE_SKIN_SELECT) {
			if (m_nSkinsTotal > 0) {
				m_pSelectedSkin = (tSkinInfo*)(m_sSkin).field_304;
				strcpy(m_PrefsSkinFile, m_aSkinName);
				CWorld::Players[0].SetPlayerSkin(m_PrefsSkinFile);
				SaveSettings();
			} else {
				if (!m_bGameNotLoaded)
					ChangeScreen(aScreens[m_nCurrScreen].m_PreviousPage[1], aScreens[m_nCurrScreen].m_ParentEntry[1], true, true);
				else
					ChangeScreen(aScreens[m_nCurrScreen].m_PreviousPage[0], aScreens[m_nCurrScreen].m_ParentEntry[0], true, true);
			}
		} else if (m_nCurrScreen != MENUPAGE_MULTIPLAYER_FIND_GAME) {
			option = aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption].m_Action;
			switch (option) {
				case MENUACTION_RADIO:
#ifdef TIDY_UP_PBP
					assumeIncrease = true;
#else
					++CMenuManager::m_PrefsRadioStation;
					if (cDMAudio::IsMP3RadioChannelAvailable()) {
						if (CMenuManager::m_PrefsRadioStation > USERTRACK)
							CMenuManager::m_PrefsRadioStation = HEAD_RADIO;
					} else if (CMenuManager::m_PrefsRadioStation > CHATTERBOX) {
						CMenuManager::m_PrefsRadioStation = USERTRACK;
					}
					SaveSettings();
					DMAudio.SetRadioInCar(CMenuManager::m_PrefsRadioStation);
					DMAudio.PlayFrontEndTrack(CMenuManager::m_PrefsRadioStation, 1);
					OutputDebugString("FRONTEND RADIO STATION CHANGED");
#endif
					break;
				case MENUACTION_LANG_ENG:
					m_PrefsLanguage = LANGUAGE_AMERICAN;
					m_bFrontEnd_ReloadObrTxtGxt = true;
					CMenuManager::InitialiseChangedLanguageSettings();
					CMenuManager::SaveSettings();
					break;
				case MENUACTION_LANG_FRE:
					m_PrefsLanguage = LANGUAGE_FRENCH;
					m_bFrontEnd_ReloadObrTxtGxt = true;
					CMenuManager::InitialiseChangedLanguageSettings();
					CMenuManager::SaveSettings();
					break;
				case MENUACTION_LANG_GER:
					m_PrefsLanguage = LANGUAGE_GERMAN;
					m_bFrontEnd_ReloadObrTxtGxt = true;
					CMenuManager::InitialiseChangedLanguageSettings();
					CMenuManager::SaveSettings();
					break;
				case MENUACTION_LANG_ITA:
					m_PrefsLanguage = LANGUAGE_ITALIAN;
					m_bFrontEnd_ReloadObrTxtGxt = true;
					CMenuManager::InitialiseChangedLanguageSettings();
					CMenuManager::SaveSettings();
					break;
				case MENUACTION_LANG_SPA:
					m_PrefsLanguage = LANGUAGE_SPANISH;
					m_bFrontEnd_ReloadObrTxtGxt = true;
					CMenuManager::InitialiseChangedLanguageSettings();
					SaveSettings();
					break;
#ifdef MORE_LANGUAGES
				case MENUACTION_LANG_RUS:
					m_PrefsLanguage = LANGUAGE_RUSSIAN;
					m_bFrontEnd_ReloadObrTxtGxt = true;
					CMenuManager::InitialiseChangedLanguageSettings();
					SaveSettings();
					break;
#endif
				case MENUACTION_POPULATESLOTS_CHANGEMENU:
					PcSaveHelper.PopulateSlotInfo();

					// fall through
				case MENUACTION_CHANGEMENU:
				{
					bool changeMenu = true;
					int saveSlot = aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption].m_SaveSlot;

					// This should be unused.
					if (saveSlot >= 2 && saveSlot <= 9) {
						m_nCurrSaveSlot = saveSlot - 2;
						switch (m_nCurrScreen) {
						case MENUPAGE_CHOOSE_LOAD_SLOT:
							if (Slots[m_nCurrSaveSlot + 1] != SLOT_EMPTY)
								changeMenu = false;

							break;
						case MENUPAGE_CHOOSE_DELETE_SLOT:
							if (Slots[m_nCurrSaveSlot + 1] == SLOT_EMPTY)
								changeMenu = false;

							break;
						}
					}
					if (changeMenu) {
						if (strncmp(aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption].m_EntryName, "FEDS_TB", 8) == 0) {
#ifndef TIDY_UP_PBP
							CMenuManager::ResetHelperText();
							if (!m_bGameNotLoaded)
								ChangeScreen(aScreens[m_nCurrScreen].m_PreviousPage[1], aScreens[m_nCurrScreen].m_ParentEntry[1], true, true);
							else
								ChangeScreen(aScreens[m_nCurrScreen].m_PreviousPage[0], aScreens[m_nCurrScreen].m_ParentEntry[0], true, true);
#else
							goBack = true;
							break;
#endif
						} else {
							ChangeScreen(aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption].m_TargetMenu, 0, true, true);
						}
					}
					break;
				}
				case MENUACTION_CHECKSAVE:
				{
					int saveSlot = aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption].m_SaveSlot;

					if (saveSlot >= 2 && saveSlot <= 9) {
						m_nCurrSaveSlot = saveSlot - 2;
						if (Slots[m_nCurrSaveSlot + 1] != SLOT_EMPTY && Slots[m_nCurrSaveSlot + 1] != SLOT_CORRUPTED) {
							ChangeScreen(aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption].m_TargetMenu, 0, true, true);
						}
					}
					break;
				}
				case MENUACTION_NEWGAME:
					CMenuManager::DoSettingsBeforeStartingAGame();
					break;
				case MENUACTION_RELOADIDE:
					// TODO
					// CFileLoader::ReloadObjectTypes("GTA3.IDE");
					break;
				case MENUACTION_RELOADIPL:
					CGame::ReloadIPLs();
					break;
				case MENUACTION_SHOWCULL:
					gbShowCullZoneDebugStuff = !gbShowCullZoneDebugStuff;
					break;
				case MENUACTION_MEMCARDSAVECONFIRM:
					return;
				case MENUACTION_RESUME_FROM_SAVEZONE:
					CMenuManager::RequestFrontEndShutDown();
					break;
				case MENUACTION_MPMAP_LIBERTY:
				case MENUACTION_MPMAP_REDLIGHT:
				case MENUACTION_MPMAP_CHINATOWN:
				case MENUACTION_MPMAP_TOWER:
				case MENUACTION_MPMAP_SEWER:
				case MENUACTION_MPMAP_INDUSTPARK:
				case MENUACTION_MPMAP_DOCKS:
				case MENUACTION_MPMAP_STAUNTON:
					sthWithButtons = option - MENUACTION_MPMAP_LIBERTY;
					CMenuManager::SaveSettings();
					ChangeScreen(aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption].m_TargetMenu, 0, true, true);
					break;
				case MENUACTION_MPMAP_DEATHMATCH1:
				case MENUACTION_MPMAP_DEATHMATCH2:
				case MENUACTION_MPMAP_TEAMDEATH1:
				case MENUACTION_MPMAP_TEAMDEATH2:
				case MENUACTION_MPMAP_STASH:
				case MENUACTION_MPMAP_CAPTURE:
				case MENUACTION_MPMAP_RATRACE:
				case MENUACTION_MPMAP_DOMINATION:
					sthWithButtons2 = option - MENUACTION_MPMAP_DEATHMATCH1;
					CMenuManager::SaveSettings();
					ChangeScreen(aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption].m_TargetMenu, 0, true, true);
					break;
				case MENUACTION_REDEFCTRL:
					ChangeScreen(MENUPAGE_KEYBOARD_CONTROLS, 0, true, true);
					m_nSelectedListRow = 0;
					m_nCurrExLayer = 19;
					break;
				case MENUACTION_GETKEY:
					m_CurrCntrlAction = GetStartOptionsCntrlConfigScreens() + m_nCurrOption;
					m_bKeyIsOK = true;
					m_bWaitingForNewKeyBind = true;
					m_bStartWaitingForKeyBind = true;
					pControlEdit = &m_KeyPressedCode;
					break;
				case MENUACTION_CANCELGAME:
					DMAudio.Service();
					RsEventHandler(rsQUITAPP, nil);
					break;
				case MENUACTION_RESUME:
#ifndef TIDY_UP_PBP
					if (CMenuManager::m_PrefsVsyncDisp != CMenuManager::m_PrefsVsync) {
						CMenuManager::m_PrefsVsync = CMenuManager::m_PrefsVsyncDisp;
					}
					CMenuManager::RequestFrontEndShutDown();
#else
					goBack = true;
#endif
					break;
				case MENUACTION_DONTCANCEL:
					ChangeScreen(aScreens[m_nCurrScreen].m_PreviousPage[1], aScreens[m_nCurrScreen].m_ParentEntry[1], true, true);
					break;
				case MENUACTION_SCREENRES:
					if (m_nDisplayVideoMode != m_nPrefsVideoMode) {
						m_nPrefsVideoMode = m_nDisplayVideoMode;
						_psSelectScreenVM(m_nPrefsVideoMode);
						SetHelperText(0);
						SaveSettings();
					}
					break;
				case MENUACTION_AUDIOHW:
				{
					int selectedProvider = m_nPrefsAudio3DProviderIndex;
					if (selectedProvider != -1) {
						m_nPrefsAudio3DProviderIndex = DMAudio.SetCurrent3DProvider(m_nPrefsAudio3DProviderIndex);
						if (selectedProvider == m_nPrefsAudio3DProviderIndex) {
							DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_SUCCESS, 0);
							SetHelperText(0);
						} else {
							DMAudio.PlayFrontEndSound(SOUND_FRONTEND_FAIL, 0);
							SetHelperText(4);
						}
						SaveSettings();
					}
					break;
				}
				case MENUACTION_SPEAKERCONF:
#ifndef TIDY_UP_PBP
					if (m_nPrefsAudio3DProviderIndex != -1) {
						if (--m_PrefsSpeakers < 0)
							m_PrefsSpeakers = 2;
						DMAudio.SetSpeakerConfig(m_PrefsSpeakers);
						SaveSettings();
					}
#else
					assumeIncrease = true;
#endif
					break;
				case MENUACTION_PLAYERSETUP:
					CPlayerSkin::BeginFrontendSkinEdit();
					ChangeScreen(MENUPAGE_SKIN_SELECT, 0, true, true);
					m_nCurrExLayer = 19;
					m_bSkinsFound = 0;
					break;
				case MENUACTION_RESTOREDEF:
					if (m_nCurrScreen == MENUPAGE_SOUND_SETTINGS) {
						CMenuManager::m_PrefsSfxVolume = 102;
						m_PrefsSpeakers = 0;
						CMenuManager::m_PrefsMusicVolume = 102;
						// unused
						// byte_95CDB5 = 0; 
						CMenuManager::m_PrefsRadioStation = HEAD_RADIO;
						DMAudio.SetMusicMasterVolume(102);
						DMAudio.SetEffectsMasterVolume(CMenuManager::m_PrefsSfxVolume);
						DMAudio.SetRadioInCar(CMenuManager::m_PrefsRadioStation);
						DMAudio.PlayFrontEndTrack(CMenuManager::m_PrefsRadioStation, 1);
						SaveSettings();
					} else if (m_nCurrScreen == MENUPAGE_GRAPHICS_SETTINGS) {
						CMenuManager::m_PrefsFrameLimiter = true;
						CMenuManager::m_PrefsBrightness = 256;
						CMenuManager::m_PrefsVsyncDisp = true;
						CMenuManager::m_PrefsLOD = 1.2f;
						CMenuManager::m_PrefsVsync = true;
						CRenderer::ms_lodDistScale = 1.2f;
						CMenuManager::m_PrefsUseWideScreen = false;
						CMenuManager::m_PrefsShowSubtitles = true;
						m_nDisplayVideoMode = m_nPrefsVideoMode;
						CMBlur::BlurOn = true;
						SaveSettings();
					} else if ((m_nCurrScreen != MENUPAGE_SKIN_SELECT_OLD) && (m_nCurrScreen == MENUPAGE_CONTROLLER_PC)) {
						ControlsManager.MakeControllerActionsBlank();
						ControlsManager.InitDefaultControlConfiguration();
						ControlsManager.InitDefaultControlConfigMouse(MousePointerStateHelper.GetMouseSetUp());
						if (AllValidWinJoys.m_aJoys[0].m_bInitialised) {
							DIDEVCAPS devCaps;
							devCaps.dwSize = sizeof(DIDEVCAPS);
							PSGLOBAL(joy1)->GetCapabilities(&devCaps);
							ControlsManager.InitDefaultControlConfigJoyPad(devCaps.dwButtons);
						}
						CMenuManager::m_ControlMethod = CONTROL_STANDART;
						MousePointerStateHelper.bInvertVertically = false;
						TheCamera.m_fMouseAccelHorzntl = 0.0025f;
						CVehicle::m_bDisableMouseSteering = true;
						TheCamera.m_bHeadBob = false;
						SaveSettings();
					}
					CMenuManager::SetHelperText(2);
					break;
				case MENUACTION_CTRLMETHOD:
#ifndef TIDY_UP_PBP
					if (CMenuManager::m_ControlMethod == CONTROL_CLASSIC) {
						CCamera::m_bUseMouse3rdPerson = true;
						CMenuManager::m_ControlMethod = CONTROL_STANDART;
					} else {
						CCamera::m_bUseMouse3rdPerson = false;
						CMenuManager::m_ControlMethod = CONTROL_CLASSIC;
					}
					SaveSettings();
#else
					assumeIncrease = true;
#endif
					break;
				case MENUACTION_LOADRADIO:
					ChangeScreen(MENUPAGE_SOUND_SETTINGS, 0, true, true);
					DMAudio.PlayFrontEndTrack(CMenuManager::m_PrefsRadioStation, 1);
					OutputDebugString("STARTED PLAYING FRONTEND AUDIO TRACK");
					break;
			}
		}
		ProcessOnOffMenuOptions();
	}

	if (goBack) {
		CMenuManager::ResetHelperText();
		DMAudio.PlayFrontEndSound(SOUND_FRONTEND_EXIT, 0);
#ifdef PS2_LIKE_MENU
		if (m_nCurrScreen == MENUPAGE_PAUSE_MENU || bottomBarActive) {
#else
		if (m_nCurrScreen == MENUPAGE_PAUSE_MENU) {
#endif
			if (!m_bGameNotLoaded && !m_bMenuStateChanged) {
				if (CMenuManager::m_PrefsVsyncDisp != CMenuManager::m_PrefsVsync) {
					CMenuManager::m_PrefsVsync = CMenuManager::m_PrefsVsyncDisp;
				}
				CMenuManager::RequestFrontEndShutDown();
			}

			// We're already resuming, we don't need further processing.
#if defined(FIX_BUGS) || defined(PS2_LIKE_MENU)
			return;
#endif
		}
#ifdef PS2_LIKE_MENU
		else if (m_nCurrScreen == MENUPAGE_CHOOSE_SAVE_SLOT || m_nCurrScreen == MENUPAGE_SAVE) {
#else
		else if (m_nCurrScreen == MENUPAGE_CHOOSE_SAVE_SLOT) {
#endif
			CMenuManager::RequestFrontEndShutDown();
		}
		// It's now in ThingsToDoBeforeLeavingPage()
#ifndef TIDY_UP_PBP
		else if (m_nCurrScreen == MENUPAGE_SOUND_SETTINGS) {
			DMAudio.StopFrontEndTrack();
			OutputDebugString("FRONTEND AUDIO TRACK STOPPED");
		}
#endif

		int oldScreen = !m_bGameNotLoaded ? aScreens[m_nCurrScreen].m_PreviousPage[1] : aScreens[m_nCurrScreen].m_PreviousPage[0];
		int oldOption = !m_bGameNotLoaded ? aScreens[m_nCurrScreen].m_ParentEntry[1] : aScreens[m_nCurrScreen].m_ParentEntry[0];

		if (oldScreen != -1) {
			ThingsToDoBeforeLeavingPage();

#ifdef PS2_LIKE_MENU
			if (!bottomBarActive &&
				(oldScreen == MENUPAGE_NONE || oldScreen == MENUPAGE_OPTIONS)) {
				bottomBarActive = true;
			} else
#endif
			{
				ChangeScreen(oldScreen, oldOption, true, true);
			}

			// We will go back for sure at this point, why process other things?!
#ifdef FIX_BUGS
			return;
#endif
		}
	}

#ifdef PS2_LIKE_MENU
	if (bottomBarActive)
		return;
#endif

	int changeValueBy = 0;
	bool decrease = false;
#ifdef TIDY_UP_PBP
	bool increase = assumeIncrease;
#else
	bool increase = false;
#endif
	if (CPad::GetPad(0)->GetLeft() || CPad::GetPad(0)->GetPedWalkLeftRight() < 0 || CPad::GetPad(0)->GetDPadLeft()) {
		static uint32 lastSliderDecrease = 0;
		if (CTimer::GetTimeInMillisecondsPauseMode() - lastSliderDecrease > 150) {
			CMenuManager::CheckSliderMovement(-1);
			lastSliderDecrease = CTimer::GetTimeInMillisecondsPauseMode();
		}
	} else if (CPad::GetPad(0)->GetRight() || CPad::GetPad(0)->GetPedWalkLeftRight() > 0 || CPad::GetPad(0)->GetDPadRight()) {
		static uint32 lastSliderIncrease = 0;
		if (CTimer::GetTimeInMillisecondsPauseMode() - lastSliderIncrease > 150) {
			CMenuManager::CheckSliderMovement(1);
			lastSliderIncrease = CTimer::GetTimeInMillisecondsPauseMode();
		}
	}

	if (CPad::GetPad(0)->GetRightJustDown() || CPad::GetPad(0)->GetAnaloguePadRight() || CPad::GetPad(0)->GetDPadRightJustDown()) {
		m_bShowMouse = false;
		increase = true;
	} else if (CPad::GetPad(0)->GetMouseWheelUpJustDown() && m_nCurrScreen != MENUPAGE_KEYBOARD_CONTROLS) {
		increase = true;
		CMenuManager::CheckSliderMovement(1);
		m_bShowMouse = true;
	}

	if (!CPad::GetPad(0)->GetLeftJustDown() && !CPad::GetPad(0)->GetAnaloguePadLeft() && !CPad::GetPad(0)->GetDPadLeftJustDown()) {
		if (CPad::GetPad(0)->GetMouseWheelDownJustDown()) {
			if (m_nCurrScreen != MENUPAGE_KEYBOARD_CONTROLS) {
				decrease = true;
				CMenuManager::CheckSliderMovement(-1);
				m_bShowMouse = true;
			}
		}
	} else {
		m_bShowMouse = false;
		decrease = true;
	}

	if (increase)
		changeValueBy++;
	else if (decrease)
		changeValueBy--;

	if (changeValueBy != 0) {
		switch (aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption].m_Action) {
			case MENUACTION_RADIO:
				CMenuManager::m_PrefsRadioStation += changeValueBy;
				DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_SUCCESS, 0);
				if (DMAudio.IsMP3RadioChannelAvailable()) {
					if (CMenuManager::m_PrefsRadioStation < HEAD_RADIO)
						CMenuManager::m_PrefsRadioStation = USERTRACK;
					if (CMenuManager::m_PrefsRadioStation > USERTRACK)
						CMenuManager::m_PrefsRadioStation = HEAD_RADIO;
				} else {
					if (CMenuManager::m_PrefsRadioStation < HEAD_RADIO)
						CMenuManager::m_PrefsRadioStation = CHATTERBOX;
					if (CMenuManager::m_PrefsRadioStation > CHATTERBOX)
						CMenuManager::m_PrefsRadioStation = HEAD_RADIO;
				}
				CMenuManager::SaveSettings();
				DMAudio.SetRadioInCar(CMenuManager::m_PrefsRadioStation);
				DMAudio.PlayFrontEndTrack(CMenuManager::m_PrefsRadioStation, 1);
				OutputDebugString("FRONTEND RADIO STATION CHANGED");
				break;
			case MENUACTION_SCREENRES:
				if (m_bGameNotLoaded) {
					RwChar** videoMods = _psGetVideoModeList();
					DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_DENIED, 0);
					if (changeValueBy > 0) {
						do {
							++m_nDisplayVideoMode;

							if (m_nDisplayVideoMode >= _psGetNumVideModes())
								m_nDisplayVideoMode = 0;
						} while (!videoMods[m_nDisplayVideoMode]);
					} else {
						do {
							--m_nDisplayVideoMode;

							if (m_nDisplayVideoMode < 0)
								m_nDisplayVideoMode = _psGetNumVideModes() - 1;
						} while (!videoMods[m_nDisplayVideoMode]);
					}
				}
				break;
			case MENUACTION_AUDIOHW:
				if (m_nPrefsAudio3DProviderIndex != -1) {
					m_nPrefsAudio3DProviderIndex += changeValueBy;
					m_nPrefsAudio3DProviderIndex = clamp(m_nPrefsAudio3DProviderIndex, 0, DMAudio.GetNum3DProvidersAvailable() - 1);
				}
				DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_DENIED, 0);
				break;
			case MENUACTION_SPEAKERCONF:
				if (m_nPrefsAudio3DProviderIndex != -1) {
					m_PrefsSpeakers -= changeValueBy;
					m_PrefsSpeakers = clamp(m_PrefsSpeakers, 0, 2);
					DMAudio.SetSpeakerConfig(m_PrefsSpeakers);
					CMenuManager::SaveSettings();
					DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_SUCCESS, 0);
				}
				break;
			case MENUACTION_CTRLMETHOD:
				CMenuManager::m_ControlMethod = !m_ControlMethod;
				CCamera::m_bUseMouse3rdPerson = !m_ControlMethod;
				DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_SUCCESS, 0);
				CMenuManager::SaveSettings();
				break;
		}
		ProcessOnOffMenuOptions();
		if (m_nCurrScreen == MENUPAGE_KEYBOARD_CONTROLS) {
			if (changeValueBy < 1) {
				field_530 = 0;
			} else {
				field_530 = 14;
			}
			DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_DENIED, 0);
		}
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
		if (m_PrefsUseWideScreen > 2)
			m_PrefsUseWideScreen = 0;
#endif
		DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_SUCCESS, 0);
		SaveSettings();
		break;
	case MENUACTION_SETDBGFLAG:
		CTheScripts::InvertDebugFlag();
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
		CVehicle::m_bDisableMouseSteering = !CVehicle::m_bDisableMouseSteering;
		DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_SUCCESS, 0);
		SaveSettings();
		break;
	}
}
#endif

void CMenuManager::RequestFrontEndShutDown()
{
	m_bShutDownFrontEndRequested = true;
	DMAudio.ChangeMusicMode(MUSICMODE_GAME);
}

#if DONT_USE_SUSPICIOUS_FUNCS
WRAPPER void CMenuManager::RequestFrontEndStartUp() { EAXJMP(0x488770); }
#else
void CMenuManager::RequestFrontEndStartUp()
{
	m_bStartUpFrontEndRequested = true;
}
#endif

#if DONT_USE_SUSPICIOUS_FUNCS
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
	switch (PcSaveHelper.nErrorCode) {
		case SAVESTATUS_ERR_SAVE_CREATE:
		case SAVESTATUS_ERR_SAVE_WRITE:
		case SAVESTATUS_ERR_SAVE_CLOSE:
			ChangeScreen(MENUPAGE_SAVE_FAILED, 0, true, false);
			break;
		case SAVESTATUS_ERR_LOAD_OPEN:
		case SAVESTATUS_ERR_LOAD_READ:
		case SAVESTATUS_ERR_LOAD_CLOSE:
			ChangeScreen(MENUPAGE_LOAD_FAILED, 0, true, false);
			break;
		case SAVESTATUS_ERR_DATA_INVALID:
			ChangeScreen(MENUPAGE_LOAD_FAILED_2, 0, true, false);
			break;
		case SAVESTATUS_DELETEFAILED8:
		case SAVESTATUS_DELETEFAILED9:
		case SAVESTATUS_DELETEFAILED10:
			ChangeScreen(MENUPAGE_DELETE_FAILED, 0, true, false);
			break;
	}
}

#if DONT_USE_SUSPICIOUS_FUNCS
WRAPPER void CMenuManager::SetHelperText(int text) { EAXJMP(0x48B450); }
#else
void CMenuManager::SetHelperText(int text)
{
	m_nHelperTextMsgId = text;
	m_nHelperTextAlpha = 300;
}
#endif

#if DONT_USE_SUSPICIOUS_FUNCS
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
	bool menuWasActive = !!m_bMenuActive;

	// Reminder: You need REGISTER_START_BUTTON defined to make it work.
	if (CPad::GetPad(0)->GetStartJustDown() 
#ifdef FIX_BUGS
		&& !m_bGameNotLoaded
#endif
		|| m_bShutDownFrontEndRequested || m_bStartUpFrontEndRequested) {

		m_bMenuActive = !m_bMenuActive;

		if (m_bShutDownFrontEndRequested)
			m_bMenuActive = false;
		if (m_bStartUpFrontEndRequested)
			m_bMenuActive = true;

		if (m_bMenuActive) {
			CTimer::StartUserPause();
		} else {
#ifdef PS2_LIKE_MENU
			bottomBarActive = false;
#endif
#ifdef FIX_BUGS
			ThingsToDoBeforeLeavingPage();
#endif
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

	// Just entered the save/safe zone
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
/*	// PS2 leftover
	if (m_nCurrScreen != MENUPAGE_SOUND_SETTINGS && gMusicPlaying)
	{
		DMAudio.StopFrontEndTrack();
		OutputDebugString("FRONTEND AUDIO TRACK STOPPED");
		gMusicPlaying = 0;
	}
*/
	if (m_bMenuActive != menuWasActive)
		m_bMenuStateChanged = true;

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

#if DONT_USE_SUSPICIOUS_FUNCS
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
#endif

STARTPATCHES
#if DONT_USE_SUSPICIOUS_FUNCS
#else
	for (int i = 1; i < ARRAY_SIZE(aScreens); i++)
		Patch(0x611930 + sizeof(CMenuScreen) * i, aScreens[i]);
#endif
	InjectHook(0x4856F0, &CMenuManager::ProcessButtonPresses, PATCH_JUMP);
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