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
#include "RwHelper.h"
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
#include "Radar.h"
#include "Stats.h"
#include "Messages.h"
#include "FileLoader.h"

#define TIDY_UP_PBP // ProcessButtonPresses
#define MAX_VISIBLE_LIST_ROW 30
#define SCROLLBAR_MAX_HEIGHT 263.0f // not in end result

#ifdef USE_PRECISE_MEASUREMENT_CONVERTION
#define MILES_IN_METER 0.000621371192f
#define FEET_IN_METER 3.28084f
#else
#define MILES_IN_METER 0.00059880241f
#define FEET_IN_METER 3.33f
#endif

#ifdef SCROLLABLE_STATS_PAGE
#define isPlainTextScreen(screen) (screen == MENUPAGE_BRIEFS)
#else
#define isPlainTextScreen(screen) (screen == MENUPAGE_BRIEFS || screen == MENUPAGE_STATS)
#endif

#ifdef TRIANGLE_BACK_BUTTON
#define GetBackJustUp GetTriangleJustUp
#define GetBackJustDown GetTriangleJustDown
#elif defined(CIRCLE_BACK_BUTTON)
#define GetBackJustUp GetCircleJustUp
#define GetBackJustDown GetCircleJustDown
#else
#define GetBackJustUp GetSquareJustUp
#define GetBackJustDown GetSquareJustDown
#endif

#ifdef MENU_MAP
bool CMenuManager::bMenuMapActive = false;
bool CMenuManager::bMapMouseShownOnce = false;
bool CMenuManager::bMapLoaded = false;
float CMenuManager::fMapSize;
float CMenuManager::fMapCenterY;
float CMenuManager::fMapCenterX;
#endif

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

int32 CMenuManager::OS_Language = LANG_ENGLISH; // *(int32*)0x5F2F78;
int8 CMenuManager::m_PrefsUseVibration; // = *(int8*)0x95CD92;
int8 CMenuManager::m_DisplayControllerOnFoot; // = *(int8*)0x95CD8D;
int8 CMenuManager::m_PrefsVsync = 1; // *(int8*)0x5F2E58;
int8 CMenuManager::m_PrefsVsyncDisp = 1; // *(int8*)0x5F2E5C;
int8 CMenuManager::m_PrefsFrameLimiter = 1; // *(int8*)0x5F2E60;
int8 CMenuManager::m_PrefsShowSubtitles = 1; // *(int8*)0x5F2E54;
int8 CMenuManager::m_PrefsSpeakers; // = *(int8*)0x95CD7E;
int32 CMenuManager::m_ControlMethod; // = *(int32*)0x8F5F7C;
int8 CMenuManager::m_PrefsDMA = 1; // *(int8*)0x5F2F74;
int32 CMenuManager::m_PrefsLanguage; // = *(int32*)0x941238;
uint8 CMenuManager::m_PrefsStereoMono; // *(bool*)0x95CDB5; // unused except restore settings

bool CMenuManager::m_PrefsAllowNastyGame = true; // *(bool*)0x5F2E64;
bool CMenuManager::m_bStartUpFrontEndRequested; // = *(bool*)0x95CCF4;
bool CMenuManager::m_bShutDownFrontEndRequested; // = *(bool*)0x95CD6A;

int8 CMenuManager::m_PrefsUseWideScreen; // = *(int8*)0x95CD23;
int8 CMenuManager::m_PrefsRadioStation; // = *(int8*)0x95CDA4;
int32 CMenuManager::m_PrefsBrightness = 256; // = *(int32*)0x5F2E50;
float CMenuManager::m_PrefsLOD; // = *(float*)0x8F42C4;
int8 CMenuManager::m_bFrontEnd_ReloadObrTxtGxt; // = *(int8*)0x628CFC;
int32 CMenuManager::m_PrefsMusicVolume = 102; // = *(int32*)0x5F2E4C;
int32 CMenuManager::m_PrefsSfxVolume = 102; // = *(int32*)0x5F2E48;

char CMenuManager::m_PrefsSkinFile[256] = "$$\"\""; // = (char*)0x5F2E74;

int32 CMenuManager::m_KeyPressedCode = -1; // = *(int32*)0x5F2E70;

// Originally that was PS2 option color, they forget it here and used in PrintBriefs once(but didn't use the output anyway)
#ifdef PS2_LIKE_MENU
const CRGBA TEXT_COLOR = CRGBA(150, 110, 30, 255);
#else
const CRGBA TEXT_COLOR = CRGBA(235, 170, 50, 255); // PC briefs text color
#endif

const float menuXYpadding = MENUACTION_POS_Y; // *(float*)0x5F355C;	// not original name
float MENU_TEXT_SIZE_X = SMALLTEXT_X_SCALE; //*(float*)0x5F2E40;
float MENU_TEXT_SIZE_Y = SMALLTEXT_Y_SCALE; //*(float*)0x5F2E44;

bool holdingScrollBar; // *(bool*)0x628D59; // not original name
int32 CMenuManager::m_SelectedMap; // *(int32*)0x8E2880;
int32 CMenuManager::m_SelectedGameType; // *(int32*)0x942F88;

// Used in a hidden menu
uint8 CMenuManager::m_PrefsPlayerRed = 255;
uint8 CMenuManager::m_PrefsPlayerGreen = 128;
uint8 CMenuManager::m_PrefsPlayerBlue; // why??

CMenuManager FrontEndMenuManager; // = *(CMenuManager*)0x8F59D8;

// Move this somewhere else.
float CRenderer::ms_lodDistScale = 1.2f; // *(float*)0x5F726C;

uint32 TimeToStopPadShaking; // = *(uint32*)0x628CF8;
char *pEditString; // = *(char**)0x628D00;
int32 *pControlEdit; // = *(int32**)0x628D08;
bool DisplayComboButtonErrMsg; // = *(bool*)0x628D14;
int32 MouseButtonJustClicked; // = *(int32*)0x628D0C;
int32 JoyButtonJustClicked; // = *(int32*)0x628D10;
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

#ifdef MENU_MAP
const char* MapFilenames[][2] = {
	{"mapMid01", "mapMid01A"},
	{"mapMid02", "mapMid02A"},
	{"mapMid03", "mapMid03A"},
	{"mapBot01", "mapBot01A"},
	{"mapBot02", "mapBot02A"},
	{"mapBot03", "mapBot03A"},
	{"mapTop01", "mapTop01A"},
	{"mapTop02", "mapTop02A"},
	{"mapTop03", "mapTop03A"},
};
CSprite2d CMenuManager::m_aMapSprites[NUM_MAP_SPRITES];
#endif

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
#define MENU_X_RIGHT_ALIGNED(x) ScaleAndCenterX(DEFAULT_SCREEN_WIDTH - (x))
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
		lastActiveBarX = DisplaySlider(SCREEN_STRETCH_FROM_RIGHT(MENUSLIDER_X + columnWidth), MENU_Y(bitAboveNextItemY), MENU_Y(smallestSliderBar), MENU_Y(usableLineHeight), MENU_X(MENUSLIDER_UNK), value); \
		if (i != m_nCurrOption || !itemsAreSelectable) \
			break; \
		 \
		if (CheckHover(hoverStartX, lastActiveBarX - MENU_X(10.0f), MENU_Y(nextYToUse), MENU_Y(28.0f + nextYToUse))) \
			m_nHoverOption = decreaseAction; \
		 \
		if (!CheckHover(MENU_X(10.0f) + lastActiveBarX, hoverEndX, MENU_Y(nextYToUse), MENU_Y(28.0f + nextYToUse))) \
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
			m_nScrollbarTopMargin -= SCROLLBAR_MAX_HEIGHT / m_nTotalListRow;
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
			m_nScrollbarTopMargin += SCROLLBAR_MAX_HEIGHT / m_nTotalListRow;
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
		m_nScrollbarTopMargin = (SCROLLBAR_MAX_HEIGHT / m_nTotalListRow) * m_nFirstVisibleRowOnList;
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
		m_nScrollbarTopMargin = (SCROLLBAR_MAX_HEIGHT / m_nTotalListRow) * m_nFirstVisibleRowOnList;
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
CMenuManager::BuildStatLine(char *text, void *stat, bool itsFloat, void *stat2)
{
	if (!text)
		return;

	if (stat2) {
		if (itsFloat) 
			sprintf(gString2, "  %.2f %s %.2f", *(float*)stat, UnicodeToAscii(TheText.Get("FEST_OO")), *(float*)stat2);
		else 
			sprintf(gString2, "  %d %s %d", *(int*)stat, UnicodeToAscii(TheText.Get("FEST_OO")), *(int*)stat2);
	} else if (stat) {
		if (itsFloat)
			sprintf(gString2, "  %.2f", *(float*)stat);
		else
			sprintf(gString2, "  %d", *(int*)stat);
	} else
		gString2[0] = '\0';

	UnicodeStrcpy(gUString, TheText.Get(text));
	AsciiToUnicode(gString2, gUString2);
}

void
CMenuManager::CentreMousePointer()
{
	tagPOINT Point;

	if (SCREEN_WIDTH * 0.5f != 0.0f && 0.0f != SCREEN_HEIGHT * 0.5f) {
		Point.x = SCREEN_WIDTH / 2;
		Point.y = SCREEN_HEIGHT / 2;
		ClientToScreen(PSGLOBAL(window), &Point);
		SetCursorPos(Point.x, Point.y);

		PSGLOBAL(lastMousePos.x) = SCREEN_WIDTH / 2;
		PSGLOBAL(lastMousePos.y) = SCREEN_HEIGHT / 2;
	}
}

void
CMenuManager::CheckCodesForControls(int typeOfControl)
{
	DisplayComboButtonErrMsg = false;
	bool invalidKey = false;
	bool escPressed = false;
	eControllerType typeToSave;
	// GetStartOptionsCntrlConfigScreens();
	e_ControllerAction action = (e_ControllerAction) m_CurrCntrlAction;

	if (typeOfControl == KEYBOARD) {
		if (*pControlEdit == rsESC) {
			escPressed = true;
		} else if (*pControlEdit > rsF3 && *pControlEdit != rsF9 && *pControlEdit != rsLWIN &&
			*pControlEdit != rsRWIN && *pControlEdit != rsRALT) {
			typeToSave = KEYBOARD;
			if (ControlsManager.GetControllerKeyAssociatedWithAction(action, KEYBOARD) != rsNULL &&
				*pControlEdit != ControlsManager.GetControllerKeyAssociatedWithAction(action, KEYBOARD)) {
				typeToSave = OPTIONAL_EXTRA;
			}
		} else {
			invalidKey = true;
		}
	} else if (typeOfControl == MOUSE) {
		typeToSave = MOUSE;
	} else if (typeOfControl == JOYSTICK) {
		typeToSave = JOYSTICK;
		if (ControlsManager.GetIsActionAButtonCombo(action))
			DisplayComboButtonErrMsg = true;
	}

	ControlsManager.ClearSettingsAssociatedWithAction(action, typeToSave);
	if (!DisplayComboButtonErrMsg && !escPressed && !invalidKey) {
		if (typeOfControl == KEYBOARD) {
			ControlsManager.DeleteMatchingActionInitiators(action, *pControlEdit, KEYBOARD);
			ControlsManager.DeleteMatchingActionInitiators(action, *pControlEdit, OPTIONAL_EXTRA);
		} else {
			if (typeOfControl == MOUSE) {
				ControlsManager.DeleteMatchingActionInitiators(action, MouseButtonJustClicked, MOUSE);
			} else if (typeOfControl == JOYSTICK) {
				ControlsManager.DeleteMatchingActionInitiators(action, JoyButtonJustClicked, JOYSTICK);
			}
		}
		if (typeOfControl == KEYBOARD) {
			ControlsManager.SetControllerKeyAssociatedWithAction(action, *pControlEdit, typeToSave);

		} else if (typeOfControl == MOUSE) {
			ControlsManager.SetControllerKeyAssociatedWithAction(action, MouseButtonJustClicked, typeToSave);
		} else {
			if (typeOfControl == JOYSTICK) {
				ControlsManager.SetControllerKeyAssociatedWithAction(action, JoyButtonJustClicked, typeToSave);
			}
		}
		pControlEdit = nil;
		m_bWaitingForNewKeyBind = false;
		m_KeyPressedCode = -1;
		m_bStartWaitingForKeyBind = false;
		SaveSettings();
	}

	if (escPressed) {
		pControlEdit = nil;
		m_bWaitingForNewKeyBind = false;
		m_KeyPressedCode = -1;
		m_bStartWaitingForKeyBind = false;
		SaveSettings();
	}
}

bool
CMenuManager::CheckHover(int x1, int x2, int y1, int y2)
{
	return m_nMousePosX > x1 && m_nMousePosX < x2 &&
	       m_nMousePosY > y1 && m_nMousePosY < y2;
}

void
CMenuManager::CheckSliderMovement(int value)
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

void
CMenuManager::DisplayHelperText()
{
	// there was a unused static bool
	static uint32 LastFlash = 0;
	int32 alpha;

	if (m_nHelperTextMsgId != 0 && m_nHelperTextMsgId != 1) {

		// FIX: High fps bug
#ifndef FIX_BUGS
		if (CTimer::GetTimeInMillisecondsPauseMode() - LastFlash > 10) {
			LastFlash = CTimer::GetTimeInMillisecondsPauseMode();
			m_nHelperTextAlpha -= 2;
		}
#else
		static float fadeAlpha = 0.0f; // To keep it precisely
		if (m_nHelperTextAlpha >= 255 && fadeAlpha < 250) fadeAlpha = m_nHelperTextAlpha;

		// -2 per every 33 ms (1000.f/30.f - original frame limiter fps)
		fadeAlpha -= (frameTime / 33.0f) * 2.0f;
		m_nHelperTextAlpha = fadeAlpha;
#endif
		if (m_nHelperTextAlpha < 1)
			ResetHelperText();

		alpha = m_nHelperTextAlpha > 255 ? 255 : m_nHelperTextAlpha;
	}

	CFont::SetCentreOn();
	CFont::SetScale(SCREEN_SCALE_X(SMALLESTTEXT_X_SCALE), SCREEN_SCALE_Y(SMALLESTTEXT_Y_SCALE));
	CFont::SetFontStyle(FONT_HEADING);

	// TODO: name this cases?
	switch (m_nHelperTextMsgId) {
		case 0:
		{
			int action = aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption].m_Action;
			if (action != MENUACTION_CHANGEMENU && action != MENUACTION_REDEFCTRL && action != MENUACTION_RESTOREDEF) {
				CFont::SetColor(CRGBA(255, 255, 255, 255));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(320.0f), SCREEN_SCALE_FROM_BOTTOM(120.0f), TheText.Get("FET_MIG"));
			}
			break;
		}
		case 1:
			CFont::SetColor(CRGBA(255, 255, 255, 255));
			CFont::PrintString(MENU_X_LEFT_ALIGNED(320.0f), SCREEN_SCALE_FROM_BOTTOM(120.0f), TheText.Get("FET_APP"));
			break;
		case 2:
			CFont::SetColor(CRGBA(255, 255, 255, alpha));
			CFont::PrintString(MENU_X_LEFT_ALIGNED(320.0f), SCREEN_SCALE_FROM_BOTTOM(120.0f), TheText.Get("FET_HRD"));
			break;
		case 3:
			CFont::SetColor(CRGBA(255, 255, 255, alpha));
			CFont::PrintString(MENU_X_LEFT_ALIGNED(320.0f), SCREEN_SCALE_FROM_BOTTOM(120.0f), TheText.Get("FET_RSO"));
			break;
		case 4:
			CFont::SetColor(CRGBA(255, 255, 255, alpha));
			CFont::PrintString(MENU_X_LEFT_ALIGNED(320.0f), SCREEN_SCALE_FROM_BOTTOM(120.0f), TheText.Get("FET_RSC"));
			break;
		default:
			break;
	}
	CFont::SetRightJustifyOff();
}

int
CMenuManager::DisplaySlider(float x, float y, float mostLeftBarSize, float mostRightBarSize, float rectSize, float progress) 
{
	CRGBA color;
	float maxBarHeight;

	int lastActiveBarX = 0;
	float curBarX = 0.0f;
	float spacing = SCREEN_SCALE_X(10.0f);
	for (int i = 0; i < 16; i++) {
		curBarX = i * rectSize/16.0f + x;

		if (i / 16.0f + 1 / 32.0f < progress) {
			color = CRGBA(255, 217, 106, FadeIn(255));
			lastActiveBarX = curBarX;
		} else
			color = CRGBA(185, 120, 0, FadeIn(255));

		maxBarHeight = max(mostLeftBarSize, mostRightBarSize);

		float curBarFreeSpace = ((16 - i) * mostLeftBarSize + i * mostRightBarSize) / 16.0f;
		float left = curBarX;
		float top = y + maxBarHeight - curBarFreeSpace;
		float right = spacing + curBarX;
		float bottom = y + maxBarHeight;
		float shadowOffset = SCREEN_SCALE_X(2.0f);
		CSprite2d::DrawRect(CRect(left + shadowOffset, top + shadowOffset, right + shadowOffset, bottom + shadowOffset), CRGBA(0, 0, 0, FadeIn(200))); // Shadow
		CSprite2d::DrawRect(CRect(left, top, right, bottom), color);
	}
	return lastActiveBarX;
}

void
CMenuManager::DoSettingsBeforeStartingAGame()
{
	CCamera::m_bUseMouse3rdPerson = m_ControlMethod == CONTROL_STANDARD;
	if (m_PrefsVsyncDisp != m_PrefsVsync)
		m_PrefsVsync = m_PrefsVsyncDisp;

	DMAudio.Service();
	m_bWantToRestart = true;

	ShutdownJustMenu();
	UnloadTextures();
	DMAudio.SetEffectsFadeVol(0);
	DMAudio.SetMusicFadeVol(0);
	DMAudio.ResetTimers(CTimer::GetTimeInMilliseconds());
}

void
CMenuManager::Draw()
{
	CFont::SetBackgroundOff();
	CFont::SetPropOn();
	CFont::SetCentreOff();
	CFont::SetJustifyOn();
	CFont::SetBackGroundOnlyTextOn();
	CFont::SetWrapx(MENU_X_RIGHT_ALIGNED(MENU_X_MARGIN));
	CFont::SetRightJustifyWrap(SCREEN_SCALE_X(MENUACTION_WIDTH));

	switch (m_nCurrScreen) {
		case MENUPAGE_STATS:
			PrintStats();
			break;
		case MENUPAGE_BRIEFS:
			PrintBriefs();
			break;
#ifdef MENU_MAP
		case MENUPAGE_MAP:
			PrintMap();
			break;
#endif
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
	CFont::SetScale(MENU_X(MENUACTION_SCALE_MULT * MENU_TEXT_SIZE_X), MENU_Y(MENUACTION_SCALE_MULT * MENU_TEXT_SIZE_Y));
	CFont::SetRightJustifyOff();
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
		// Label is wrapped from right by StretchX(40)px, but wrapped from left by 40px. And this is only place R* didn't use StretchX in here.
		CFont::PrintString(MENU_X_LEFT_ALIGNED(MENU_X_MARGIN), MENU_Y(menuXYpadding), str);
#else
		CFont::PrintString(MENU_X_MARGIN, menuXYpadding, str);
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
			CFont::SetScale(MENU_X(MENU_TEXT_SIZE_X = BIGTEXT_X_SCALE), MENU_Y(MENU_TEXT_SIZE_Y = BIGTEXT_Y_SCALE));
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
			CFont::SetScale(MENU_X(MENU_TEXT_SIZE_X = MEDIUMTEXT_X_SCALE), MENU_Y(MENU_TEXT_SIZE_Y = MEDIUMTEXT_Y_SCALE));
			CFont::SetRightJustifyOff();
			break;
		case MENUPAGE_CHOOSE_LOAD_SLOT:
		case MENUPAGE_CHOOSE_DELETE_SLOT:
		case MENUPAGE_CHOOSE_SAVE_SLOT:
			columnWidth = 120;
			headerHeight = 38;
			lineHeight = 20;
			CFont::SetFontStyle(FONT_BANK);
			CFont::SetScale(MENU_X(MENU_TEXT_SIZE_X = SMALLTEXT_X_SCALE), MENU_Y(MENU_TEXT_SIZE_Y = SMALLTEXT_Y_SCALE));
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
			CFont::SetScale(MENU_X(MENU_TEXT_SIZE_X = BIGTEXT_X_SCALE), MENU_Y(MENU_TEXT_SIZE_Y = BIGTEXT_Y_SCALE));
			CFont::SetCentreOn();
			break;
		case MENUPAGE_START_MENU:
			columnWidth = 320;
			headerHeight = 140;
			lineHeight = 24;
			CFont::SetFontStyle(FONT_HEADING);
			CFont::SetScale(MENU_X(MENU_TEXT_SIZE_X = BIGTEXT_X_SCALE), MENU_Y(MENU_TEXT_SIZE_Y = BIGTEXT_Y_SCALE));
			CFont::SetCentreOn();
			break;
		case MENUPAGE_PAUSE_MENU:
			columnWidth = 320;
			headerHeight = 117;
			lineHeight = 24;
			CFont::SetFontStyle(FONT_HEADING);
			CFont::SetScale(MENU_X(MENU_TEXT_SIZE_X = BIGTEXT_X_SCALE), MENU_Y(MENU_TEXT_SIZE_Y = BIGTEXT_Y_SCALE));
			CFont::SetCentreOn();
			break;
#ifdef PS2_SAVE_DIALOG
		case MENUPAGE_SAVE:
			columnWidth = 180;
			headerHeight = 60;
			lineHeight = 24;
			CFont::SetFontStyle(FONT_BANK);
			CFont::SetScale(MENU_X(MENU_TEXT_SIZE_X = BIGTEXT_X_SCALE), MENU_Y(MENU_TEXT_SIZE_Y = BIGTEXT_Y_SCALE));
			break;
#endif
		default:
			columnWidth = 320;
			headerHeight = 40;
			lineHeight = 24;
			CFont::SetFontStyle(FONT_HEADING);
			CFont::SetScale(MENU_X(MENU_TEXT_SIZE_X = BIGTEXT_X_SCALE), MENU_Y(MENU_TEXT_SIZE_Y = BIGTEXT_Y_SCALE));
			CFont::SetCentreOn();
			break;
	}

#ifdef PS2_LIKE_MENU
	CFont::SetFontStyle(FONT_BANK);
#endif

	switch (m_nCurrScreen) {
		case MENUPAGE_CONTROLLER_PC_OLD1:
		case MENUPAGE_CONTROLLER_PC_OLD2:
		case MENUPAGE_CONTROLLER_PC_OLD3:
		case MENUPAGE_CONTROLLER_PC_OLD4:
		case MENUPAGE_CONTROLLER_DEBUG:
			if (m_bWaitingForNewKeyBind)
				itemsAreSelectable = false;

			DrawControllerScreenExtraText(nextYToUse - 8.0f, MENU_X_LEFT_ALIGNED(350), lineHeight);
			break;
		default:
			break;
	}

	float usableLineHeight = lineHeight * 0.9f; // also height of biggest bar in slider
	float smallestSliderBar = lineHeight * 0.1f;
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
						switch (m_SelectedMap) {
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
						switch (m_SelectedGameType) {
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
				if (m_PrefsUseVibration)
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

						static int oldOption = -99;
						static int oldScreen = m_nCurrScreen;

						m_nPrevOption = rowToCheck;
						if (m_nMouseOldPosX != m_nMousePosX || m_nMouseOldPosY != m_nMousePosY) {
							m_nCurrOption = rowToCheck;
							m_bShowMouse = true;
						}
						if (oldOption != m_nCurrOption) {
							if (oldScreen == m_nCurrScreen && m_bShowMouse)
								DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_DENIED, 0);

							oldOption = m_nCurrOption;
							oldScreen = m_nCurrScreen;
						}
						if (oldScreen == m_nPrevScreen)
							oldScreen = m_nCurrScreen;

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
			int lastActiveBarX;
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

	if (m_nCurrScreen == MENUPAGE_CONTROLLER_SETTINGS)
		PrintController();
	else if (m_nCurrScreen == MENUPAGE_SKIN_SELECT_OLD) {
		CSprite2d::DrawRect(CRect(MENU_X_LEFT_ALIGNED(180), MENU_Y(98), MENU_X_LEFT_ALIGNED(230), MENU_Y(123)), CRGBA(255, 255, 255, FadeIn(255)));
		CSprite2d::DrawRect(CRect(MENU_X_LEFT_ALIGNED(181), MENU_Y(99), MENU_X_LEFT_ALIGNED(229), MENU_Y(122)), CRGBA(m_PrefsPlayerRed, m_PrefsPlayerGreen, m_PrefsPlayerBlue, FadeIn(255)));
	}

}

int
CMenuManager::GetNumOptionsCntrlConfigScreens(void)
{
	int number = 0;
	switch (m_nCurrScreen) {
		case MENUPAGE_CONTROLLER_PC_OLD3:
			number = 2;
			break;
		case MENUPAGE_CONTROLLER_DEBUG:
			number = 4;
			break;
		case MENUPAGE_KEYBOARD_CONTROLS:
			switch (m_ControlMethod) {
				case CONTROL_STANDARD:
					number = 25;
					break;
				case CONTROL_CLASSIC:
					number = 30;
					break;
			}
			break;
	}
	return number;
}

void
CMenuManager::DrawControllerBound(int32 yStart, int32 xStart, int32 unused, int8 column)
{
	int controllerAction = PED_FIREWEAPON;
	// GetStartOptionsCntrlConfigScreens();
	int numOptions = GetNumOptionsCntrlConfigScreens();
	int bindingMargin = MENU_X(3.0f);
	float rowHeight;
	switch (m_ControlMethod) {
		case CONTROL_STANDARD:
			rowHeight = CONTSETUP_STANDARD_ROW_HEIGHT;
			break;
		case CONTROL_CLASSIC:
			rowHeight = CONTSETUP_CLASSIC_ROW_HEIGHT;
			break;
		default:
			break;
	}

	// MENU_Y(rowHeight * 0.0f + yStart);
	for (int optionIdx = 0, nextY = MENU_Y(yStart); optionIdx < numOptions; nextY = MENU_Y(++optionIdx * rowHeight + yStart)) {
		int nextX = xStart;
		int bindingsForThisOpt = 0;
		CFont::SetColor(CRGBA(155, 155, 155, FadeIn(255)));

		if (column == CONTSETUP_PED_COLUMN) {
			switch (optionIdx) {
				case 0:
					controllerAction = PED_FIREWEAPON;
					break;
				case 1:
					controllerAction = PED_CYCLE_WEAPON_RIGHT;
					break;
				case 2:
					controllerAction = PED_CYCLE_WEAPON_LEFT;
					break;
				case 3:
					controllerAction = GO_FORWARD;
					break;
				case 4:
					controllerAction = GO_BACK;
					break;
				case 5:
					controllerAction = GO_LEFT;
					break;
				case 6:
					controllerAction = GO_RIGHT;
					break;
				case 7:
					controllerAction = PED_SNIPER_ZOOM_IN;
					break;
				case 8:
					controllerAction = PED_SNIPER_ZOOM_OUT;
					break;
				case 9:
					controllerAction = VEHICLE_ENTER_EXIT;
					break;
				case 10:
				case 11:
				case 12:
				case 16:
				case 18:
				case 19:
				case 20:
				case 21:
					controllerAction = -1;
					break;
				case 13:
					controllerAction = CAMERA_CHANGE_VIEW_ALL_SITUATIONS;
					break;
				case 14:
					controllerAction = PED_JUMPING;
					break;
				case 15:
					controllerAction = PED_SPRINT;
					break;
				case 17:
					controllerAction = PED_LOCK_TARGET;
					break;
				case 22:
					controllerAction = PED_LOOKBEHIND;
					break;
				case 23:
					if (m_ControlMethod == CONTROL_STANDARD)
						controllerAction = -1;
					else
						controllerAction = PED_1RST_PERSON_LOOK_LEFT;
					break;
				case 24:
					if (m_ControlMethod == CONTROL_STANDARD)
						controllerAction = -1;
					else
						controllerAction = PED_1RST_PERSON_LOOK_RIGHT;
					break;
				case 25:
					controllerAction = PED_1RST_PERSON_LOOK_UP;
					break;
				case 26:
					controllerAction = PED_1RST_PERSON_LOOK_DOWN;
					break;
				case 27:
					controllerAction = PED_CYCLE_TARGET_LEFT;
					break;
				case 28:
					controllerAction = PED_CYCLE_TARGET_RIGHT;
					break;
				case 29:
					controllerAction = PED_CENTER_CAMERA_BEHIND_PLAYER;
					break;
				default:
					break;
			}
		} else if (column == CONTSETUP_VEHICLE_COLUMN) {
			switch (optionIdx) {
				case 0:
					controllerAction = PED_FIREWEAPON;
					break;
				case 1:
				case 2:
				case 7:
				case 8:
				case 14:
				case 15:
				case 17:
				case 25:
				case 26:
				case 27:
				case 28:
				case 29:
					controllerAction = -1;
					break;
				case 3:
					controllerAction = VEHICLE_ACCELERATE;
					break;
				case 4:
					controllerAction = VEHICLE_BRAKE;
					break;
				case 5:
					controllerAction = GO_LEFT;
					break;
				case 6:
					controllerAction = GO_RIGHT;
					break;
				case 9:
					controllerAction = VEHICLE_ENTER_EXIT;
					break;
				case 10:
					controllerAction = VEHICLE_CHANGE_RADIO_STATION;
					break;
				case 11:
					controllerAction = VEHICLE_HORN;
					break;
				case 12:
					controllerAction = TOGGLE_SUBMISSIONS;
					break;
				case 13:
					controllerAction = CAMERA_CHANGE_VIEW_ALL_SITUATIONS;
					break;
				case 16:
					controllerAction = VEHICLE_HANDBRAKE;
					break;
				case 18:
					controllerAction = VEHICLE_TURRETLEFT;
					break;
				case 19:
					controllerAction = VEHICLE_TURRETRIGHT;
					break;
				case 20:
					controllerAction = VEHICLE_TURRETUP;
					break;
				case 21:
					controllerAction = VEHICLE_TURRETDOWN;
					break;
				case 22:
					controllerAction = -2;
					break;
				case 23:
					controllerAction = VEHICLE_LOOKLEFT;
					break;
				case 24:
					controllerAction = VEHICLE_LOOKRIGHT;
					break;
				default:
					break;
			}
		}
		int bindingWhite = 155;

		// Highlight selected column(and make its text black)
		if (m_nSelectedListRow == optionIdx) {
			int bgY = m_nSelectedListRow * rowHeight + yStart + 1.0f;
			if (m_nCurrExLayer == HOVEROPTION_LIST) {

				if (column == CONTSETUP_PED_COLUMN && m_nSelectedContSetupColumn == CONTSETUP_PED_COLUMN) {
#ifdef FIX_BUGS
					if (controllerAction == -1) {
						CSprite2d::DrawRect(CRect(nextX, MENU_Y(bgY), nextX + MENU_X(CONTSETUP_BOUND_COLUMN_WIDTH),
							MENU_Y(bgY + CONTSETUP_BOUND_HIGHLIGHT_HEIGHT)), CRGBA(235, 170, 50, FadeIn(150)));
					} else {
						CSprite2d::DrawRect(CRect(nextX, MENU_Y(bgY), nextX + MENU_X(CONTSETUP_BOUND_COLUMN_WIDTH),
							MENU_Y(bgY + CONTSETUP_BOUND_HIGHLIGHT_HEIGHT)), CRGBA(255, 217, 106, FadeIn(210)));
					}
#else
					if (controllerAction == -1) {
						CSprite2d::DrawRect(CRect(MENU_X_LEFT_ALIGNED(210.0f), MENU_Y(bgY),
							MENU_X_LEFT_ALIGNED(400.0f), MENU_Y(bgY + CONTSETUP_BOUND_HIGHLIGHT_HEIGHT)), CRGBA(235, 170, 50, FadeIn(150)));
					} else {
						CSprite2d::DrawRect(CRect(MENU_X_LEFT_ALIGNED(210.0f), MENU_Y(bgY),
							MENU_X_LEFT_ALIGNED(400.0f), MENU_Y(bgY + CONTSETUP_BOUND_HIGHLIGHT_HEIGHT)), CRGBA(255, 217, 106, FadeIn(210)));
					}
#endif
					CFont::SetColor(CRGBA(0, 0, 0, FadeIn(255)));
					bindingWhite = 0;

				} else if (column == CONTSETUP_VEHICLE_COLUMN && m_nSelectedContSetupColumn == CONTSETUP_VEHICLE_COLUMN) {
#ifdef FIX_BUGS
					if (controllerAction == -1) {
						CSprite2d::DrawRect(CRect(nextX, MENU_Y(bgY), nextX + MENU_X(CONTSETUP_BOUND_COLUMN_WIDTH),
							MENU_Y(bgY + CONTSETUP_BOUND_HIGHLIGHT_HEIGHT)), CRGBA(235, 170, 50, FadeIn(150)));
					} else {
						CSprite2d::DrawRect(CRect(nextX, MENU_Y(bgY), nextX + MENU_X(CONTSETUP_BOUND_COLUMN_WIDTH),
							MENU_Y(bgY + CONTSETUP_BOUND_HIGHLIGHT_HEIGHT)), CRGBA(255, 217, 106, FadeIn(210)));
					}
#else
					if (controllerAction == -1) {
						CSprite2d::DrawRect(CRect(MENU_X_LEFT_ALIGNED(410.0f), MENU_Y(bgY), MENU_X_LEFT_ALIGNED(600.0f), MENU_Y(bgY + 10)), CRGBA(235, 170, 50, FadeIn(150)));
					} else {
						CSprite2d::DrawRect(CRect(MENU_X_LEFT_ALIGNED(410.0f), MENU_Y(bgY), MENU_X_LEFT_ALIGNED(600.0f), MENU_Y(bgY + 10)), CRGBA(255, 217, 106, FadeIn(210)));
					}
#endif
					CFont::SetColor(CRGBA(0, 0, 0, FadeIn(255)));
					bindingWhite = 0;
				}
			}
		}

		// Print bindings, including seperator (-) between them
		CFont::SetScale(MENU_X(0.25f), MENU_Y(0.6f));
		for (int contSetOrder = SETORDER_1; contSetOrder < MAX_SETORDERS && controllerAction != -1; contSetOrder++) {
			wchar *settingText = ControlsManager.GetControllerSettingTextWithOrderNumber((e_ControllerAction)controllerAction, (eContSetOrder)contSetOrder);
			if (settingText) {
				++bindingsForThisOpt;
				if (bindingsForThisOpt > 1) {
					wchar *seperator = TheText.Get("FEC_IBT");
					CFont::SetColor(CRGBA(20, 20, 20, FadeIn(80)));
					CFont::PrintString(nextX, nextY, seperator);
					CFont::SetColor(CRGBA(bindingWhite, bindingWhite, bindingWhite, FadeIn(255)));
					nextX += CFont::GetStringWidth(seperator, true) + bindingMargin;
				}
				CFont::PrintString(nextX, nextY, settingText);
				nextX += CFont::GetStringWidth(settingText, true) + bindingMargin;
			}
		}
		if (controllerAction == -1) {
			CFont::SetColor(CRGBA(20, 20, 20, FadeIn(80)));
			CFont::PrintString(nextX, nextY, TheText.Get("FEC_NUS")); // not used
		} else if (controllerAction == -2) {
			CFont::SetColor(CRGBA(20, 20, 20, FadeIn(80)));
			CFont::PrintString(nextX, nextY, TheText.Get("FEC_CMP")); // combo: l+r
		} else if (bindingsForThisOpt == 0) {
			if (m_nSelectedListRow != optionIdx) {
				CFont::SetColor(CRGBA(255, 255, 255, FadeIn(255)));
				CFont::PrintString(nextX, nextY, TheText.Get("FEC_UNB")); // unbound
			} else if (m_bWaitingForNewKeyBind) {
				if (column != m_nSelectedContSetupColumn) {
					CFont::SetColor(CRGBA(255, 255, 255, FadeIn(255)));
					CFont::PrintString(nextX, nextY, TheText.Get("FEC_UNB")); // unbound
				}
			} else {
				if (column != m_nSelectedContSetupColumn) {
					CFont::SetColor(CRGBA(255, 255, 255, FadeIn(255)));
				}
				CFont::PrintString(nextX, nextY, TheText.Get("FEC_UNB")); // unbound
			}
		}

		if (column == CONTSETUP_PED_COLUMN && m_nSelectedContSetupColumn == CONTSETUP_PED_COLUMN ||
			column == CONTSETUP_VEHICLE_COLUMN && m_nSelectedContSetupColumn == CONTSETUP_VEHICLE_COLUMN) {

			if (optionIdx == m_nSelectedListRow && controllerAction != -1 && controllerAction != -2) {
				m_CurrCntrlAction = controllerAction; 
				if (m_bWaitingForNewKeyBind) {
					static bool showWaitingText = false;
					if (bindingsForThisOpt > 0) {
						wchar *seperator = TheText.Get("FEC_IBT");
						CFont::PrintString(nextX, nextY, seperator);
						nextX += CFont::GetStringWidth(seperator, true) + bindingMargin;
					}
					static uint32 lastWaitingTextFlash = 0;
					if (CTimer::GetTimeInMillisecondsPauseMode() - lastWaitingTextFlash > 150) {
						showWaitingText = !showWaitingText;
						lastWaitingTextFlash = CTimer::GetTimeInMillisecondsPauseMode();
					}
					if (showWaitingText) {
						CFont::SetColor(CRGBA(55, 55, 55, FadeIn(255)));
						CFont::PrintString(nextX, nextY, TheText.Get("FEC_QUE")); // "???"
					}
					CFont::SetCentreOn();
					CFont::SetScale(MENU_X(SMALLESTTEXT_X_SCALE), MENU_Y(SMALLESTTEXT_Y_SCALE));
					CFont::SetFontStyle(FONT_HEADING);
					CFont::SetColor(CRGBA(255, 255, 255, FadeIn(255)));
					if (m_bKeyChangeNotProcessed) {
						CFont::PrintString(MENU_X_LEFT_ALIGNED(275.0f), SCREEN_SCALE_FROM_BOTTOM(114.0f), TheText.Get("FET_CIG")); // BACKSPACE TO CLEAR - LMB,RETURN TO CHANGE
					} else {
						CFont::PrintString(MENU_X_LEFT_ALIGNED(275.0f), SCREEN_SCALE_FROM_BOTTOM(114.0f), TheText.Get("FET_RIG")); // SELECT A NEW CONTROL FOR THIS ACTION OR ESC TO CANCEL
					}
					
					CFont::SetRightJustifyOff();
					CFont::SetScale(MENU_X(SMALLESTTEXT_X_SCALE), MENU_Y(SMALLESTTEXT_Y_SCALE));
					CFont::SetFontStyle(0);
					if (!m_bKeyIsOK)
						DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_SUCCESS, 0);

					m_bKeyIsOK = true;
				} else {
					CFont::SetCentreOn();
					CFont::SetScale(MENU_X(SMALLESTTEXT_X_SCALE), MENU_Y(SMALLESTTEXT_Y_SCALE));
					CFont::SetFontStyle(FONT_HEADING);
					CFont::SetColor(CRGBA(255, 255, 255, FadeIn(255)));
					CFont::PrintString(MENU_X_LEFT_ALIGNED(275.0f), SCREEN_SCALE_FROM_BOTTOM(114.0f), TheText.Get("FET_CIG")); // BACKSPACE TO CLEAR - LMB,RETURN TO CHANGE
					CFont::SetRightJustifyOff();
					CFont::SetScale(MENU_X(SMALLESTTEXT_X_SCALE), MENU_Y(SMALLESTTEXT_Y_SCALE));
					CFont::SetFontStyle(FONT_BANK);
					m_bKeyIsOK = false;
					m_bKeyChangeNotProcessed = false;
				}
			} else if (optionIdx == m_nSelectedListRow) {
				CFont::SetCentreOn();
				CFont::SetScale(MENU_X(SMALLESTTEXT_X_SCALE), MENU_Y(SMALLESTTEXT_Y_SCALE));
				CFont::SetFontStyle(FONT_HEADING);
				CFont::SetColor(CRGBA(55, 55, 55, FadeIn(255)));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(275.0f), SCREEN_SCALE_FROM_BOTTOM(114.0f), TheText.Get("FET_EIG")); // CANNOT SET A CONTROL FOR THIS ACTION
				CFont::SetRightJustifyOff();
				CFont::SetScale(MENU_X(SMALLESTTEXT_X_SCALE), MENU_Y(SMALLESTTEXT_Y_SCALE));
				CFont::SetFontStyle(FONT_BANK);
			}
		}
	}
}

void
CMenuManager::DrawControllerScreenExtraText(int yStart, int xStart, int lineHeight)
{
	int extraTextStart = GetStartOptionsCntrlConfigScreens();
	int numOpts = GetNumOptionsCntrlConfigScreens();
	int spacing = MENU_X(10.0f);
	for (int i = extraTextStart; i < extraTextStart + numOpts; i++) {
		int numTextsPrinted = 0;
		int nextX = xStart;
		for (int j = 1; j < 5; j++) {
			wchar *text = ControlsManager.GetControllerSettingTextWithOrderNumber((e_ControllerAction)i, (eContSetOrder)j);
			if (text)
				++numTextsPrinted;

			if (text) {
				// Seperator
				if (numTextsPrinted > 1) {
					CFont::PrintString(nextX, MENU_Y(yStart), TheText.Get("FEC_IBT"));
					nextX = CFont::GetStringWidth(TheText.Get("FEC_IBT"), true) + spacing + nextX;
				}
				CFont::PrintString(nextX, MENU_Y(yStart), text);
			}
			if (text)
				nextX = CFont::GetStringWidth(text, true) + spacing + nextX;
		}
		if (m_nCurrOption == i - extraTextStart && m_bWaitingForNewKeyBind) {
			static bool waitingTextVisible = false;

			// Seperator
			if (numTextsPrinted > 0) {
				CFont::PrintString(nextX, MENU_Y(yStart), TheText.Get("FEC_IBT"));
				nextX = CFont::GetStringWidth(TheText.Get("FEC_IBT"), true) + spacing + nextX;
			}
			static uint32 lastStateChange = 0;
			if (CTimer::GetTimeInMillisecondsPauseMode() - lastStateChange > 150) {
				waitingTextVisible = !waitingTextVisible;
				lastStateChange = CTimer::GetTimeInMillisecondsPauseMode();
			}
			if (waitingTextVisible) {
				CFont::SetColor(CRGBA(255, 255, 0, FadeIn(255)));
				CFont::PrintString(nextX, MENU_Y(yStart), TheText.Get("FEC_QUE"));
				CFont::SetColor(CRGBA(235, 170, 50, FadeIn(255)));
			}
		}
		yStart += lineHeight;
	}
	wchar *error = nil;
	if (DisplayComboButtonErrMsg)
		error = ControlsManager.GetButtonComboText((e_ControllerAction)(m_nCurrOption + extraTextStart));

	if (error) {
		CFont::SetColor(CRGBA(233, 22, 159, 255));
		CFont::PrintString(xStart, MENU_Y(yStart + 10), error);
	}
}

void
CMenuManager::DrawControllerSetupScreen()
{
	float rowHeight;
	switch (m_ControlMethod) {
		case CONTROL_STANDARD:
			rowHeight = CONTSETUP_STANDARD_ROW_HEIGHT;
			break;
		case CONTROL_CLASSIC:
			rowHeight = CONTSETUP_CLASSIC_ROW_HEIGHT;
			break;
		default:
			break;
	}
	CFont::SetBackgroundOff();
	CFont::SetScale(MENU_X(MENUACTION_SCALE_MULT), MENU_Y(MENUACTION_SCALE_MULT));
	CFont::SetPropOn();
	CFont::SetCentreOff();
	CFont::SetJustifyOn();
	CFont::SetRightJustifyOff();
	CFont::SetBackGroundOnlyTextOn();
	CFont::SetWrapx(MENU_X_RIGHT_ALIGNED(MENU_X_MARGIN));
	CFont::SetRightJustifyWrap(SCREEN_SCALE_X(MENUACTION_WIDTH));

	// Page header
	CFont::SetColor(CRGBA(0, 0, 0, FadeIn(255)));
	CFont::SetRightJustifyOn();
	CFont::SetScale(MENU_X(MENUHEADER_WIDTH), MENU_Y(MENUHEADER_HEIGHT));
	CFont::SetFontStyle(FONT_HEADING);
	switch (m_ControlMethod) {
		case CONTROL_STANDARD:
			CFont::PrintString(SCREEN_SCALE_FROM_RIGHT(MENUHEADER_POS_X), SCREEN_SCALE_FROM_BOTTOM(MENUHEADER_POS_Y),
				TheText.Get(aScreens[m_nCurrScreen].m_ScreenName));
			break;
		case CONTROL_CLASSIC:
			CFont::PrintString(SCREEN_SCALE_FROM_RIGHT(MENUHEADER_POS_X), SCREEN_SCALE_FROM_BOTTOM(MENUHEADER_POS_Y),
				TheText.Get("FET_CTI"));
			break;
		default:
			break;
	}
	wchar *actionTexts[31];
	actionTexts[0] = TheText.Get("FEC_FIR");
	actionTexts[1] = TheText.Get("FEC_NWE");
	actionTexts[2] = TheText.Get("FEC_PWE");
	actionTexts[3] = TheText.Get("FEC_FOR");
	actionTexts[4] = TheText.Get("FEC_BAC");
	actionTexts[5] = TheText.Get("FEC_LEF");
	actionTexts[6] = TheText.Get("FEC_RIG");
	actionTexts[7] = TheText.Get("FEC_ZIN");
	actionTexts[8] = TheText.Get("FEC_ZOT");
	actionTexts[9] = TheText.Get("FEC_EEX");
	actionTexts[10] = TheText.Get("FEC_RAD");
	actionTexts[11] = TheText.Get("FEC_HRN");
	actionTexts[12] = TheText.Get("FEC_SUB");
	actionTexts[13] = TheText.Get("FEC_CMR");
	actionTexts[14] = TheText.Get("FEC_JMP");
	actionTexts[15] = TheText.Get("FEC_SPN");
	actionTexts[16] = TheText.Get("FEC_HND");
	actionTexts[17] = TheText.Get("FEC_TAR");
	if (m_ControlMethod == CONTROL_CLASSIC) {
		actionTexts[18] = TheText.Get("FEC_TFL");
		actionTexts[19] = TheText.Get("FEC_TFR");
		actionTexts[20] = TheText.Get("FEC_TFU");
		actionTexts[21] = TheText.Get("FEC_TFD");
		actionTexts[22] = TheText.Get("FEC_LBA");
		actionTexts[23] = TheText.Get("FEC_LOL");
		actionTexts[24] = TheText.Get("FEC_LOR");
		actionTexts[25] = TheText.Get("FEC_LUD");
		actionTexts[26] = TheText.Get("FEC_LDU");
		actionTexts[27] = TheText.Get("FEC_NTR");
		actionTexts[28] = TheText.Get("FEC_PTT");
		actionTexts[29] = TheText.Get("FEC_CEN");
		actionTexts[30] = nil;
	} else {
		actionTexts[18] = TheText.Get("FEC_TFL");
		actionTexts[19] = TheText.Get("FEC_TFR");
		actionTexts[20] = TheText.Get("FEC_TFU");
		actionTexts[21] = TheText.Get("FEC_TFD");
		actionTexts[22] = TheText.Get("FEC_LBA");
		actionTexts[23] = TheText.Get("FEC_LOL");
		actionTexts[24] = TheText.Get("FEC_LOR");
		actionTexts[25] = nil;
	}

	// Gray panel background
	CSprite2d::DrawRect(CRect(MENU_X_LEFT_ALIGNED(CONTSETUP_LIST_LEFT), MENU_Y(CONTSETUP_LIST_TOP),
		MENU_X_RIGHT_ALIGNED(CONTSETUP_LIST_RIGHT), SCREEN_SCALE_FROM_BOTTOM(CONTSETUP_LIST_BOTTOM)),
		CRGBA(200, 200, 50, FadeIn(50)));

	if (m_nCurrExLayer == HOVEROPTION_LIST)
		CFont::SetColor(CRGBA(255, 217, 106, FadeIn(255)));
	else
		CFont::SetColor(CRGBA(235, 170, 50, FadeIn(255)));

	// List header
	CFont::SetFontStyle(FONT_HEADING);
	CFont::SetScale(MENU_X(MENUACTION_SCALE_MULT), MENU_Y(MENUACTION_SCALE_MULT));
	CFont::SetRightJustifyOff();
	CFont::PrintString(MENU_X_LEFT_ALIGNED(CONTSETUP_COLUMN_1_X), MENU_Y(CONTSETUP_LIST_TOP), TheText.Get("FET_CAC"));
	CFont::PrintString(MENU_X_LEFT_ALIGNED(CONTSETUP_COLUMN_2_X), MENU_Y(CONTSETUP_LIST_TOP), TheText.Get("FET_CFT"));
	CFont::PrintString(MENU_X_LEFT_ALIGNED(CONTSETUP_COLUMN_3_X), MENU_Y(CONTSETUP_LIST_TOP), TheText.Get("FET_CCR"));
	CFont::SetRightJustifyOff();
	CFont::SetScale(MENU_X_LEFT_ALIGNED(SMALLESTTEXT_X_SCALE), MENU_Y(SMALLESTTEXT_Y_SCALE));
	CFont::SetFontStyle(FONT_BANK);
	int yStart;
	if (m_ControlMethod == CONTROL_CLASSIC)
		yStart = CONTSETUP_LIST_HEADER_HEIGHT + 29;
	else
		yStart = CONTSETUP_LIST_HEADER_HEIGHT + 34;

	for (int i = 0; i < ARRAY_SIZE(actionTexts); ++i) {
		wchar *actionText = actionTexts[i];
		if (!actionText)
			break;

		if (m_nMousePosX > MENU_X_LEFT_ALIGNED(CONTSETUP_LIST_LEFT + 2.0f) &&
			m_nMousePosX < MENU_X_LEFT_ALIGNED(CONTSETUP_COLUMN_3_X + CONTSETUP_BOUND_COLUMN_WIDTH)) {

			float curOptY = i * rowHeight + yStart;
			if (m_nMousePosY > MENU_Y(curOptY) && m_nMousePosY < MENU_Y(rowHeight + curOptY)) {
					if (m_nPrevOption != i && m_nCurrExLayer == HOVEROPTION_LIST)
						DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_DENIED, 0);

					m_nPrevOption = i;
					if (m_nMouseOldPosX != m_nMousePosX || m_nMouseOldPosY != m_nMousePosY) {
						m_nCurrExLayer = HOVEROPTION_LIST;
						m_nSelectedListRow = i;

						// why different number for 3rd column hovering X?? this function is a mess
#ifdef FIX_BUGS
						if (m_nMousePosX > MENU_X_LEFT_ALIGNED(0.0f) && m_nMousePosX < MENU_X_LEFT_ALIGNED(CONTSETUP_COLUMN_2_X + CONTSETUP_BOUND_COLUMN_WIDTH)) {
#else
						if (m_nMousePosX > MENU_X_LEFT_ALIGNED(0.0f) && m_nMousePosX < MENU_X_LEFT_ALIGNED(370.0f)) {
#endif
							if (m_nSelectedContSetupColumn != CONTSETUP_PED_COLUMN && m_nCurrExLayer == HOVEROPTION_LIST)
								DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_DENIED, 0);

							m_nSelectedContSetupColumn = CONTSETUP_PED_COLUMN;
#ifdef FIX_BUGS
						} else if (m_nMousePosX > MENU_X_LEFT_ALIGNED(CONTSETUP_COLUMN_2_X + CONTSETUP_BOUND_COLUMN_WIDTH) && m_nMousePosX < SCREEN_WIDTH) {
#else
						} else if (m_nMousePosX > MENU_X_LEFT_ALIGNED(370.0f) && m_nMousePosX < SCREEN_WIDTH) {
#endif
							if (m_nSelectedContSetupColumn != CONTSETUP_VEHICLE_COLUMN && m_nCurrExLayer == HOVEROPTION_LIST)
								DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_DENIED, 0);

							m_nSelectedContSetupColumn = CONTSETUP_VEHICLE_COLUMN;
						}
					}
					// what??
					if (m_nHoverOption == HOVEROPTION_SKIN) {
						if (i == m_nSelectedListRow) {
							m_nHoverOption = HOVEROPTION_NOT_HOVERING;
							m_bWaitingForNewKeyBind = true;
							m_bStartWaitingForKeyBind = true;
							pControlEdit = &m_KeyPressedCode;
						}
					} else
						m_nHoverOption = HOVEROPTION_NOT_HOVERING;
			}
		}
		if (m_nSelectedListRow != 35)
			CFont::SetColor(CRGBA(235, 170, 50, FadeIn(255)));
		else if (m_nCurrExLayer == HOVEROPTION_LIST)
			CFont::SetColor(CRGBA(255, 217, 106, FadeIn(255)));

		CFont::SetRightJustifyOff();
		if (m_PrefsLanguage != LANGUAGE_GERMAN || i != 20 && i != 21)
			CFont::SetScale(MENU_X(SMALLESTTEXT_X_SCALE), MENU_Y(SMALLESTTEXT_Y_SCALE));
		else
			CFont::SetScale(MENU_X(0.32f), MENU_Y(SMALLESTTEXT_Y_SCALE));

		CFont::PrintString(MENU_X_LEFT_ALIGNED(CONTSETUP_COLUMN_1_X), MENU_Y(i * rowHeight + yStart), actionText);
	}
	DrawControllerBound(yStart, MENU_X_LEFT_ALIGNED(CONTSETUP_COLUMN_2_X), rowHeight, CONTSETUP_PED_COLUMN);
	DrawControllerBound(yStart, MENU_X_LEFT_ALIGNED(CONTSETUP_COLUMN_3_X), rowHeight, CONTSETUP_VEHICLE_COLUMN);
	CFont::SetScale(MENU_X(MENU_TEXT_SIZE_X), MENU_Y(MENU_TEXT_SIZE_Y));

	if ((m_nMousePosX > MENU_X_RIGHT_ALIGNED(CONTSETUP_BACK_RIGHT) - CFont::GetStringWidth(TheText.Get("FEDS_TB"), true)
		&& m_nMousePosX < MENU_X_RIGHT_ALIGNED(CONTSETUP_BACK_RIGHT) && m_nMousePosY > SCREEN_SCALE_FROM_BOTTOM(CONTSETUP_BACK_BOTTOM)
		&& m_nMousePosY < SCREEN_SCALE_FROM_BOTTOM(CONTSETUP_BACK_BOTTOM - CONTSETUP_BACK_HEIGHT)) || m_nCurrExLayer == HOVEROPTION_BACK) {
		m_nHoverOption = HOVEROPTION_BACK;

	} else if (m_nMousePosX > MENU_X_LEFT_ALIGNED(CONTSETUP_LIST_LEFT + 2.0f) && m_nMousePosX < MENU_X_LEFT_ALIGNED(CONTSETUP_COLUMN_3_X + CONTSETUP_BOUND_COLUMN_WIDTH)
		&& m_nMousePosY > MENU_Y(CONTSETUP_LIST_TOP + CONTSETUP_LIST_HEADER_HEIGHT) && m_nMousePosY < SCREEN_SCALE_FROM_BOTTOM(CONTSETUP_LIST_BOTTOM + 5.0f)) {
		m_nHoverOption = HOVEROPTION_LIST;

	} else {
		m_nHoverOption = HOVEROPTION_NOT_HOVERING;
	}

	// Back button and it's shadow
	CFont::SetFontStyle(FONT_HEADING);
	CFont::SetScale(MENU_X(MENU_TEXT_SIZE_X), MENU_Y(MENU_TEXT_SIZE_Y));
	CFont::SetRightJustifyOn();
	CFont::SetColor(CRGBA(0, 0, 0, FadeIn(90)));
	for (int i = 0; i < 2; i++) {
		CFont::PrintString(MENU_X_RIGHT_ALIGNED(CONTSETUP_BACK_RIGHT - 2.0f - i),
			SCREEN_SCALE_FROM_BOTTOM(CONTSETUP_BACK_BOTTOM - 4.0f - i), TheText.Get("FEDS_TB"));

		if (m_nHoverOption == HOVEROPTION_BACK)
			CFont::SetColor(CRGBA(255, 217, 106, FadeIn(255)));
		else
			CFont::SetColor(CRGBA(235, 170, 50, FadeIn(255)));
	}
}

void
CMenuManager::DrawFrontEnd()
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
void
CMenuManager::DrawFrontEndSaveZone()
{
	CSprite2d::InitPerFrame();
	CFont::InitPerFrame();
	RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void*)rwFILTERNEAREST);

	// Not original dimensions, have been changed to fit PC screen & PC menu layout.
	CSprite2d::DrawRect(CRect(MENU_X_LEFT_ALIGNED(30.0f), MENU_Y(50.0f), MENU_X_RIGHT_ALIGNED(30.0f), SCREEN_SCALE_FROM_BOTTOM(50.0f)), CRGBA(0, 0, 0, 175));

	m_nMenuFadeAlpha = 255;
	RwRenderStateSet(rwRENDERSTATETEXTUREADDRESS, (void*)rwTEXTUREADDRESSCLAMP);
	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)FALSE);
	Draw();

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
void
CMenuManager::DrawFrontEndNormal()
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
			// +20 per every 33 ms (1000.f/30.f - original frame limiter fps)
			if (!reverseAlpha)
				fadeAlpha += (frameTime) * 20.f / 33.f;
			else
				fadeAlpha = max(0.0f, fadeAlpha - (frameTime) * 30.f / 33.f);

			m_nMenuFadeAlpha = fadeAlpha;
		} 
		lastState = 0;
	} else {
		if (lastState == 0) fadeAlpha = 255.f;

		if (reverseAlpha) {
			fadeAlpha -= (frameTime) * 30.f / 33.f;

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
			DrawPlayerSetupScreen();
			break;
		case MENUPAGE_KEYBOARD_CONTROLS:
			DrawControllerSetupScreen();
			break;
		default:
			Draw();
			break;
	}

	#define optionWidth		MENU_X(66.0f)
	#define rawOptionHeight	22.0f
	#define optionBottom	SCREEN_SCALE_FROM_BOTTOM(20.0f)
	#define optionTop		SCREEN_SCALE_FROM_BOTTOM(20.0f + rawOptionHeight)
	#define leftPadding		MENU_X_LEFT_ALIGNED(90.0f)
	wchar *str;
	hoveredBottomBarOption = -1;
	if (curBottomBarOption != -1) {

		// This active tab sprite is needlessly big
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
void
CMenuManager::DrawFrontEndNormal()
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

		// +20 per every 33 ms (1000.f/30.f - original frame limiter fps)
		fadeAlpha += (frameTime) * 20.f / 33.f;
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
			DrawPlayerSetupScreen();
			break;
		case MENUPAGE_KEYBOARD_CONTROLS:
			DrawControllerSetupScreen();
			break;
		default:
			Draw();
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

void
CMenuManager::DrawPlayerSetupScreen()
{
	CFont::SetBackgroundOff();
	CFont::SetScale(MENU_X(MENUACTION_SCALE_MULT), MENU_Y(MENUACTION_SCALE_MULT));
	CFont::SetPropOn();
	CFont::SetCentreOff();
	CFont::SetJustifyOn();
	CFont::SetRightJustifyOff();
	CFont::SetBackGroundOnlyTextOn();
	CFont::SetWrapx(MENU_X_RIGHT_ALIGNED(MENU_X_MARGIN));
	CFont::SetRightJustifyWrap(SCREEN_SCALE_X(MENUACTION_WIDTH));

	CFont::SetColor(CRGBA(0, 0, 0, FadeIn(255)));
	CFont::SetScale(MENU_X(MENUHEADER_WIDTH), MENU_Y(MENUHEADER_HEIGHT));
	CFont::SetRightJustifyOn();
	CFont::SetFontStyle(FONT_HEADING);
	CFont::PrintString(SCREEN_SCALE_FROM_RIGHT(MENUHEADER_POS_X), SCREEN_SCALE_FROM_BOTTOM(MENUHEADER_POS_Y), TheText.Get("FET_PS"));

	// lstrcpy's changed with strcpy

	if (!m_bSkinsEnumerated) {
		OutputDebugString("Enumerating skin filenames from skins...");
		m_pSkinListHead.nextSkin = nil;
		m_pSelectedSkin = &m_pSkinListHead;
		m_pSelectedSkin->nextSkin = new tSkinInfo;
		m_pSelectedSkin = m_pSelectedSkin->nextSkin;
		m_pSelectedSkin->skinId = 0;
		strcpy(m_pSelectedSkin->skinNameOriginal, "$$\"\"");
		strcpy(m_pSelectedSkin->skinNameDisplayed, UnicodeToAscii(TheText.Get("FET_DSN")));
		int nextSkinId = 1;
		m_pSelectedSkin->nextSkin = nil;

		WIN32_FIND_DATA FindFileData;
		SYSTEMTIME SystemTime;
		HANDLE handle = FindFirstFile("skins\\*.bmp", &FindFileData);
		for (int i = 1; handle != INVALID_HANDLE_VALUE && i; i = FindNextFile(handle, &FindFileData)) {
			if (strncmp(FindFileData.cFileName, "$$\"\"", 5) != 0) {
				m_pSelectedSkin->nextSkin = new tSkinInfo;
				m_pSelectedSkin = m_pSelectedSkin->nextSkin;
				m_pSelectedSkin->skinId = nextSkinId;
				strcpy(m_pSelectedSkin->skinNameOriginal, FindFileData.cFileName);
				strcpy(m_pSelectedSkin->skinNameDisplayed, FindFileData.cFileName);
				FileTimeToSystemTime(&FindFileData.ftLastWriteTime, &SystemTime);
				GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE, &SystemTime, 0, m_pSelectedSkin->date, 255);
				++nextSkinId;
				m_pSelectedSkin->nextSkin = nil;
			}
		}
		FindClose(handle);
		m_nSkinsTotal = nextSkinId;
		char nameTemp[256];
		for (m_pSelectedSkin = m_pSkinListHead.nextSkin; m_pSelectedSkin; m_pSelectedSkin = m_pSelectedSkin->nextSkin) {
			// Drop extension
			int oldLength = strlen(m_pSelectedSkin->skinNameDisplayed);
			m_pSelectedSkin->skinNameDisplayed[oldLength - 4] = '\0';
			m_pSelectedSkin->skinNameOriginal[oldLength - 4] = '\0';

			// Fill to 40 bytes-39 chars, idk why. This is done in sepearate function in game.
			strncpy(nameTemp, m_pSelectedSkin->skinNameDisplayed, 39); // game doesn't do that, but in our day strncpy to same string is forbidden
			strncpy(m_pSelectedSkin->skinNameDisplayed, nameTemp, 39);
			if (oldLength - 4 > 39)
				m_pSelectedSkin->skinNameDisplayed[39] = '\0';

			// Make string lowercase, except first letter
			strlwr(m_pSelectedSkin->skinNameDisplayed);
			strncpy(nameTemp, m_pSelectedSkin->skinNameDisplayed, 1);
			strupr(nameTemp);
			strncpy(m_pSelectedSkin->skinNameDisplayed, nameTemp, 1);

			// Change some chars
#ifdef FIX_BUGS
			for (int k = 0; m_pSelectedSkin->skinNameDisplayed[k] != '\0'; ++k) {
#else
			for (int k = 0; m_pSelectedSkin->skinNameOriginal[k] != '\0'; ++k) {
#endif
				if (!strncmp(&m_pSelectedSkin->skinNameDisplayed[k], "_", 1))
					strncpy(&m_pSelectedSkin->skinNameDisplayed[k], " ", 1);
				if (!strncmp(&m_pSelectedSkin->skinNameDisplayed[k], "@", 1))
					strncpy(&m_pSelectedSkin->skinNameDisplayed[k], " ", 1);
				if (!strncmp(&m_pSelectedSkin->skinNameDisplayed[k], "{", 1))
					strncpy(&m_pSelectedSkin->skinNameDisplayed[k], "(", 1);
				if (!strncmp(&m_pSelectedSkin->skinNameDisplayed[k], "}", 1))
					strncpy(&m_pSelectedSkin->skinNameDisplayed[k], ")", 1);
				if (!strncmp(&m_pSelectedSkin->skinNameDisplayed[k], "£", 1))
					strncpy(&m_pSelectedSkin->skinNameDisplayed[k], "$", 1);
			}

			// Make letters after whitespace uppercase
			for (int l = 0; m_pSelectedSkin->skinNameDisplayed[l] != '\0'; ++l) {
				if (!strncmp(&m_pSelectedSkin->skinNameDisplayed[l], " ", 1)) {
					if (m_pSelectedSkin->skinNameDisplayed[l + 1]) {
						strncpy(nameTemp, &m_pSelectedSkin->skinNameDisplayed[l + 1], 1);
						strupr(nameTemp);
						strncpy(&m_pSelectedSkin->skinNameDisplayed[l + 1], nameTemp, 1);
					}
				}
			}
		}
		OutputDebugString("Finished enumerating skin files.");
		m_bSkinsEnumerated = true;
	}
	CSprite2d::DrawRect(CRect(MENU_X_LEFT_ALIGNED(PLAYERSETUP_LIST_LEFT), MENU_Y(PLAYERSETUP_LIST_TOP),
		MENU_X_RIGHT_ALIGNED(PLAYERSETUP_LIST_RIGHT), SCREEN_SCALE_FROM_BOTTOM(PLAYERSETUP_LIST_BOTTOM)), CRGBA(200, 200, 50, FadeIn(50)));

	// Header (Skin - Date)
	if (m_nCurrExLayer == HOVEROPTION_LIST) {
		CFont::SetColor(CRGBA(255, 217, 106, FadeIn(255)));
	} else {
		CFont::SetColor(CRGBA(235, 170, 50, FadeIn(255)));
	}
	CFont::SetFontStyle(FONT_HEADING);
	CFont::SetScale(MENU_X(MENUACTION_SCALE_MULT), MENU_Y(MENUACTION_SCALE_MULT));
	CFont::SetRightJustifyOn();
	CFont::PrintString(MENU_X_RIGHT_ALIGNED(PLAYERSETUP_DATE_COLUMN_RIGHT), MENU_Y(PLAYERSETUP_LIST_TOP), TheText.Get("FES_DAT"));
	switch (m_PrefsLanguage) {
		case LANGUAGE_FRENCH:
		case LANGUAGE_SPANISH:
			CFont::SetScale(MENU_X(0.6f), MENU_Y(MENUACTION_SCALE_MULT));
			break;
		default:
			CFont::SetScale(MENU_X(MENUACTION_SCALE_MULT), MENU_Y(MENUACTION_SCALE_MULT));
			break;
	}
	CFont::SetRightJustifyOff();
	CFont::PrintString(MENU_X_LEFT_ALIGNED(PLAYERSETUP_SKIN_COLUMN_LEFT), MENU_Y(PLAYERSETUP_LIST_TOP), TheText.Get("FES_SKN"));

	// Skin list
	CFont::SetRightJustifyOff();
	CFont::SetScale(MENU_X(PLAYERSETUP_ROW_TEXT_X_SCALE), MENU_Y(PLAYERSETUP_ROW_TEXT_Y_SCALE));
	CFont::SetFontStyle(FONT_BANK);
	if (m_nSkinsTotal > 0) {
		for (m_pSelectedSkin = m_pSkinListHead.nextSkin; m_pSelectedSkin->skinId != m_nFirstVisibleRowOnList;
			m_pSelectedSkin = m_pSelectedSkin->nextSkin);

		int rowTextY = PLAYERSETUP_LIST_BODY_TOP - 1;
		int orderInVisibles = 0;
		int rowEndY = PLAYERSETUP_LIST_BODY_TOP + PLAYERSETUP_ROW_HEIGHT + 1;
		int rowStartY = PLAYERSETUP_LIST_BODY_TOP;
		for (int rowIdx = m_nFirstVisibleRowOnList; 
			rowIdx < m_nFirstVisibleRowOnList + MAX_VISIBLE_LIST_ROW && m_pSelectedSkin; ) {

			if (m_nMousePosX > MENU_X_LEFT_ALIGNED(PLAYERSETUP_LIST_LEFT) && m_nMousePosX < MENU_X_RIGHT_ALIGNED(PLAYERSETUP_LIST_RIGHT)) {
				if (m_nMousePosY > MENU_Y(rowStartY) && m_nMousePosY < MENU_Y(rowEndY)) {
					m_nPrevOption = rowIdx;
					if (m_nMouseOldPosX != m_nMousePosX || m_nMouseOldPosY != m_nMousePosY) {
						m_nCurrExLayer = HOVEROPTION_LIST;
					}
					if (m_nHoverOption == HOVEROPTION_SKIN) {
						if (rowIdx == m_nSelectedListRow) {
							m_nHoverOption = HOVEROPTION_NOT_HOVERING;
							if (m_nSkinsTotal > 0) {
								DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_SUCCESS, 0);
								strcpy(m_PrefsSkinFile, m_aSkinName);
								CWorld::Players[0].SetPlayerSkin(m_PrefsSkinFile);
								SaveSettings();
							}
						} else {
							DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_DENIED, 0);
							m_nCurrExLayer = HOVEROPTION_LIST;
							m_nSelectedListRow = rowIdx;
							m_nHoverOption = HOVEROPTION_NOT_HOVERING;
						}
					}
				}
			}

			// Preview skin/change color of row when we focused on another row.
			if (orderInVisibles == m_nSelectedListRow - m_nFirstVisibleRowOnList) {
				CFont::SetColor(CRGBA(255, 255, 255, FadeIn(255)));
				static int lastSelectedSkin = -1;
				if (m_nSelectedListRow != lastSelectedSkin) {
					strcpy(m_aSkinName, m_pSelectedSkin->skinNameOriginal);
					CWorld::Players[0].SetPlayerSkin(m_aSkinName);
				}
				lastSelectedSkin = m_nSelectedListRow;
			} else if (!strcmp(m_PrefsSkinFile, m_pSelectedSkin->skinNameOriginal)) {
				CFont::SetColor(CRGBA(255, 255, 155, FadeIn(255)));
			} else {
				CFont::SetColor(CRGBA(155, 155, 155, FadeIn(255)));
			}
			wchar unicodeTemp[80];
			AsciiToUnicode(m_pSelectedSkin->skinNameDisplayed, unicodeTemp);
			CFont::SetRightJustifyOff();
			CFont::PrintString(MENU_X_LEFT_ALIGNED(PLAYERSETUP_SKIN_COLUMN_LEFT), MENU_Y(rowTextY), unicodeTemp);

			// If not "Default skin" option
			if (rowIdx != 0) {
				char dateTemp[32];
				sprintf(dateTemp, "%s", m_pSelectedSkin->date);
				AsciiToUnicode(dateTemp, unicodeTemp);
				CFont::SetRightJustifyOn();
				CFont::PrintString(MENU_X_RIGHT_ALIGNED(PLAYERSETUP_DATE_COLUMN_RIGHT), MENU_Y(rowTextY), unicodeTemp);
			}
			++orderInVisibles;
			rowEndY += PLAYERSETUP_ROW_HEIGHT;
			rowStartY += PLAYERSETUP_ROW_HEIGHT;
			rowTextY += PLAYERSETUP_ROW_HEIGHT;
			++rowIdx;
			m_pSelectedSkin = m_pSelectedSkin->nextSkin;
		}
		// Scrollbar background
		CSprite2d::DrawRect(CRect(MENU_X_RIGHT_ALIGNED(PLAYERSETUP_LIST_RIGHT - 2), MENU_Y(PLAYERSETUP_LIST_TOP),
			MENU_X_RIGHT_ALIGNED(PLAYERSETUP_LIST_RIGHT - 2 - PLAYERSETUP_SCROLLBAR_WIDTH), SCREEN_SCALE_FROM_BOTTOM(PLAYERSETUP_LIST_BOTTOM)), CRGBA(100, 100, 66, FadeIn(205)));
		
		// Scrollbar
		float scrollbarHeight = SCROLLBAR_MAX_HEIGHT / m_nSkinsTotal * (float) MAX_VISIBLE_LIST_ROW;
		float scrollbarBottom, scrollbarTop;
		if (m_nSkinsTotal <= MAX_VISIBLE_LIST_ROW) {
			scrollbarBottom = SCREEN_SCALE_FROM_BOTTOM(PLAYERSETUP_LIST_BOTTOM + PLAYERSETUP_SCROLLBUTTON_HEIGHT + 4.0f);
			scrollbarTop = MENU_Y(PLAYERSETUP_LIST_BODY_TOP);

			// Shadow
			CSprite2d::DrawRect(CRect(MENU_X_RIGHT_ALIGNED(PLAYERSETUP_LIST_RIGHT - 4), scrollbarTop,
				MENU_X_RIGHT_ALIGNED(PLAYERSETUP_LIST_RIGHT - 1 - PLAYERSETUP_SCROLLBAR_WIDTH), scrollbarBottom + MENU_Y(1.0f)), CRGBA(50, 50, 50, FadeIn(255)));
		} else {
#ifdef FIX_BUGS
			scrollbarBottom = MENU_Y(PLAYERSETUP_LIST_BODY_TOP - 8 + m_nScrollbarTopMargin + scrollbarHeight);
			scrollbarTop = MENU_Y(PLAYERSETUP_LIST_BODY_TOP + m_nScrollbarTopMargin);
#else
			scrollbarBottom = MENU_Y(PLAYERSETUP_LIST_BODY_TOP - 4 + m_nScrollbarTopMargin + scrollbarHeight - SCROLLBAR_MAX_HEIGHT / m_nSkinsTotal);
			scrollbarTop = MENU_Y(SCROLLBAR_MAX_HEIGHT / m_nSkinsTotal + PLAYERSETUP_LIST_BODY_TOP - 3 + m_nScrollbarTopMargin);
#endif
			// Shadow
			CSprite2d::DrawRect(CRect(MENU_X_RIGHT_ALIGNED(PLAYERSETUP_LIST_RIGHT - 4), scrollbarTop,
				MENU_X_RIGHT_ALIGNED(PLAYERSETUP_LIST_RIGHT - 1 - PLAYERSETUP_SCROLLBAR_WIDTH), scrollbarBottom + MENU_Y(1.0f)),
				CRGBA(50, 50, 50, FadeIn(255)));

		}
		CSprite2d::DrawRect(CRect(MENU_X_RIGHT_ALIGNED(PLAYERSETUP_LIST_RIGHT - 4), scrollbarTop,
			MENU_X_RIGHT_ALIGNED(PLAYERSETUP_LIST_RIGHT - PLAYERSETUP_SCROLLBAR_WIDTH), scrollbarBottom),
			CRGBA(235, 170, 50, FadeIn(255)));

		// FIX: Scroll button dimensions are buggy, because:
		//		1 - stretches the original image
		//		2 - leaves gap between button and scrollbar
		if (m_nHoverOption == HOVEROPTION_CLICKED_SCROLL_UP) {
#ifdef FIX_BUGS
			m_aMenuSprites[MENUSPRITE_UPON].Draw(CRect(MENU_X_RIGHT_ALIGNED(PLAYERSETUP_LIST_RIGHT - 2), MENU_Y(PLAYERSETUP_LIST_TOP),
				MENU_X_RIGHT_ALIGNED(PLAYERSETUP_LIST_RIGHT - 2 - PLAYERSETUP_SCROLLBUTTON_TXD_DIMENSION), MENU_Y(PLAYERSETUP_LIST_TOP + PLAYERSETUP_SCROLLBUTTON_TXD_DIMENSION)),
				CRGBA(255, 255, 255, FadeIn(255)));
#else
			m_aMenuSprites[MENUSPRITE_UPON].Draw(CRect(MENU_X_RIGHT_ALIGNED(PLAYERSETUP_LIST_RIGHT - 2), MENU_Y(PLAYERSETUP_LIST_TOP),
				MENU_X_RIGHT_ALIGNED(-20.0f), MENU_Y(PLAYERSETUP_LIST_TOP + 58)),
				CRGBA(255, 255, 255, FadeIn(255)));
#endif
		} else {
#ifdef FIX_BUGS
			m_aMenuSprites[MENUSPRITE_UPOFF].Draw(CRect(MENU_X_RIGHT_ALIGNED(PLAYERSETUP_LIST_RIGHT - 3), MENU_Y(PLAYERSETUP_LIST_TOP),
				MENU_X_RIGHT_ALIGNED(PLAYERSETUP_LIST_RIGHT - 3 - PLAYERSETUP_SCROLLBUTTON_TXD_DIMENSION), MENU_Y(PLAYERSETUP_LIST_TOP + PLAYERSETUP_SCROLLBUTTON_TXD_DIMENSION)),
				CRGBA(255, 255, 255, FadeIn(255)));
#else
			m_aMenuSprites[MENUSPRITE_UPOFF].Draw(CRect(MENU_X_RIGHT_ALIGNED(PLAYERSETUP_LIST_RIGHT - 3), MENU_Y(PLAYERSETUP_LIST_TOP),
				MENU_X_RIGHT_ALIGNED(-21.0f), MENU_Y(PLAYERSETUP_LIST_TOP + 58)),
				CRGBA(255, 255, 255, FadeIn(255)));
#endif
		}

		if (m_nHoverOption == HOVEROPTION_CLICKED_SCROLL_DOWN) {
#ifdef FIX_BUGS
			m_aMenuSprites[MENUSPRITE_DOWNON].Draw(CRect(MENU_X_RIGHT_ALIGNED(PLAYERSETUP_LIST_RIGHT - 2), SCREEN_SCALE_FROM_BOTTOM(PLAYERSETUP_LIST_BOTTOM + PLAYERSETUP_SCROLLBUTTON_HEIGHT + 1),
				MENU_X_RIGHT_ALIGNED(PLAYERSETUP_LIST_RIGHT - 2 - PLAYERSETUP_SCROLLBUTTON_TXD_DIMENSION), SCREEN_SCALE_FROM_BOTTOM(PLAYERSETUP_LIST_BOTTOM + PLAYERSETUP_SCROLLBUTTON_HEIGHT + 1 - PLAYERSETUP_SCROLLBUTTON_TXD_DIMENSION)),
				CRGBA(255, 255, 255, FadeIn(255)));
#else
			m_aMenuSprites[MENUSPRITE_DOWNON].Draw(CRect(MENU_X_RIGHT_ALIGNED(PLAYERSETUP_LIST_RIGHT - 2), SCREEN_SCALE_FROM_BOTTOM(141.0f),
					MENU_X_RIGHT_ALIGNED(-20.0f), SCREEN_SCALE_FROM_BOTTOM(83.0f)),
				CRGBA(255, 255, 255, FadeIn(255)));
#endif
		} else {
#ifdef FIX_BUGS
			m_aMenuSprites[MENUSPRITE_DOWNOFF].Draw(CRect(MENU_X_RIGHT_ALIGNED(PLAYERSETUP_LIST_RIGHT - 3), SCREEN_SCALE_FROM_BOTTOM(PLAYERSETUP_LIST_BOTTOM + PLAYERSETUP_SCROLLBUTTON_HEIGHT + 1),
				MENU_X_RIGHT_ALIGNED(PLAYERSETUP_LIST_RIGHT - 3 - PLAYERSETUP_SCROLLBUTTON_TXD_DIMENSION), SCREEN_SCALE_FROM_BOTTOM(PLAYERSETUP_LIST_BOTTOM + PLAYERSETUP_SCROLLBUTTON_HEIGHT + 1 - PLAYERSETUP_SCROLLBUTTON_TXD_DIMENSION)),
				CRGBA(255, 255, 255, FadeIn(255)));
#else
			m_aMenuSprites[MENUSPRITE_DOWNOFF].Draw(CRect(MENU_X_RIGHT_ALIGNED(PLAYERSETUP_LIST_RIGHT - 3), SCREEN_SCALE_FROM_BOTTOM(141.0f),
				MENU_X_RIGHT_ALIGNED(-21.0f), SCREEN_SCALE_FROM_BOTTOM(83.0f)),
				CRGBA(255, 255, 255, FadeIn(255)));
#endif

		}
		CPlayerSkin::RenderFrontendSkinEdit();

		// Big apply button
		if (strcmp(m_aSkinName, m_PrefsSkinFile) != 0) {
			CFont::SetFontStyle(FONT_HEADING);
			switch (m_PrefsLanguage) {
				case LANGUAGE_FRENCH:
					CFont::SetScale(MENU_X(1.1f), MENU_Y(1.9f));
					break;
				case LANGUAGE_GERMAN:
					CFont::SetScale(MENU_X(0.85f), MENU_Y(1.9f));
					break;
				case LANGUAGE_ITALIAN:
				case LANGUAGE_SPANISH:
					CFont::SetScale(MENU_X(1.4f), MENU_Y(1.9f));
					break;
				default:
					CFont::SetScale(MENU_X(1.9f), MENU_Y(1.9f));
					break;
			}
			CFont::SetColor(CRGBA(255, 217, 106, FadeIn(120)));
			CFont::SetRightJustifyOff();
			CFont::PrintString(MENU_X_LEFT_ALIGNED(20.0f), MENU_Y(220.0f), TheText.Get("FET_APL"));
		}
		CFont::SetFontStyle(FONT_HEADING);

		CFont::SetScale(MENU_X(SMALLTEXT_X_SCALE), MENU_Y(SMALLTEXT_Y_SCALE));

		if ((m_nMousePosX > MENU_X_RIGHT_ALIGNED(PLAYERSETUP_LIST_RIGHT - 1) - CFont::GetStringWidth(TheText.Get("FEDS_TB"), true)
			&& m_nMousePosX < MENU_X_RIGHT_ALIGNED(PLAYERSETUP_LIST_RIGHT - 1)
			&& m_nMousePosY > SCREEN_SCALE_FROM_BOTTOM(PLAYERSETUP_LIST_BOTTOM - 3)
			&& m_nMousePosY < SCREEN_SCALE_FROM_BOTTOM(PLAYERSETUP_LIST_BOTTOM - 26))
			|| m_nCurrExLayer == HOVEROPTION_BACK) {
			if (m_nHoverOption != HOVEROPTION_BACK)
				DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_DENIED, 0);

			m_nHoverOption = HOVEROPTION_BACK;

		} else if ((strcmp(m_aSkinName, m_PrefsSkinFile) != 0
			&& m_nMousePosX > MENU_X_LEFT_ALIGNED(PLAYERSETUP_LIST_LEFT)
			&& m_nMousePosX < MENU_X_LEFT_ALIGNED(PLAYERSETUP_LIST_LEFT) + CFont::GetStringWidth(TheText.Get("FES_SET"), true)
			&& m_nMousePosY > SCREEN_SCALE_FROM_BOTTOM(PLAYERSETUP_LIST_BOTTOM - 3)
			&& m_nMousePosY < SCREEN_SCALE_FROM_BOTTOM(PLAYERSETUP_LIST_BOTTOM - 26))
			|| m_nCurrExLayer == HOVEROPTION_USESKIN) {
			if (m_nHoverOption != HOVEROPTION_USESKIN)
				DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_DENIED, 0);

			m_nHoverOption = HOVEROPTION_USESKIN;

		} else if (m_nMousePosX > MENU_X_RIGHT_ALIGNED(PLAYERSETUP_LIST_RIGHT - 2)
			&& m_nMousePosX < MENU_X_RIGHT_ALIGNED(PLAYERSETUP_LIST_RIGHT - PLAYERSETUP_SCROLLBAR_WIDTH - 2)
			&& m_nMousePosY > MENU_Y(PLAYERSETUP_LIST_TOP)
			&& m_nMousePosY < MENU_Y(PLAYERSETUP_LIST_BODY_TOP - 3)) {
			if (m_nHoverOption != HOVEROPTION_CLICKED_SCROLL_UP && m_nHoverOption != HOVEROPTION_CLICKED_SCROLL_DOWN)
				m_nHoverOption = HOVEROPTION_OVER_SCROLL_UP;

		} else if (m_nMousePosX > MENU_X_RIGHT_ALIGNED(PLAYERSETUP_LIST_RIGHT - 2)
			&& m_nMousePosX < MENU_X_RIGHT_ALIGNED(PLAYERSETUP_LIST_RIGHT - PLAYERSETUP_SCROLLBAR_WIDTH - 2)
			&& m_nMousePosY > SCREEN_SCALE_FROM_BOTTOM(PLAYERSETUP_LIST_BOTTOM + PLAYERSETUP_SCROLLBUTTON_HEIGHT + 1)
			&& m_nMousePosY < SCREEN_SCALE_FROM_BOTTOM(PLAYERSETUP_LIST_BOTTOM)) {
			if (m_nHoverOption != HOVEROPTION_CLICKED_SCROLL_UP && m_nHoverOption != HOVEROPTION_CLICKED_SCROLL_DOWN)
				m_nHoverOption = HOVEROPTION_OVER_SCROLL_DOWN;

		} else if (m_nMousePosX > MENU_X_RIGHT_ALIGNED(PLAYERSETUP_LIST_RIGHT - 2)
			&& m_nMousePosX < MENU_X_RIGHT_ALIGNED(PLAYERSETUP_LIST_RIGHT - PLAYERSETUP_SCROLLBAR_WIDTH - 2)
				&& m_nMousePosY > MENU_Y(PLAYERSETUP_LIST_BODY_TOP - 3)
#ifdef FIX_BUGS
				&& m_nMousePosY < MENU_Y(PLAYERSETUP_LIST_BODY_TOP + m_nScrollbarTopMargin)) {
#else
				&& m_nMousePosY < MENU_Y(SCROLLBAR_MAX_HEIGHT / m_nTotalListRow + PLAYERSETUP_LIST_BODY_TOP - 3 + m_nScrollbarTopMargin)) {
#endif
			m_nHoverOption = HOVEROPTION_PAGEUP;

		} else if (m_nMousePosX > MENU_X_RIGHT_ALIGNED(PLAYERSETUP_LIST_RIGHT - 2)
			&& m_nMousePosX < MENU_X_RIGHT_ALIGNED(PLAYERSETUP_LIST_RIGHT - PLAYERSETUP_SCROLLBAR_WIDTH - 2)
#ifdef FIX_BUGS
			&& m_nMousePosY > MENU_Y(PLAYERSETUP_LIST_BODY_TOP - 8 + m_nScrollbarTopMargin + scrollbarHeight)
#else
			&& m_nMousePosY > MENU_Y(PLAYERSETUP_LIST_BODY_TOP - 3 + m_nScrollbarTopMargin + scrollbarHeight - SCROLLBAR_MAX_HEIGHT / m_nTotalListRow)
#endif
			&& m_nMousePosY < SCREEN_SCALE_FROM_BOTTOM(PLAYERSETUP_LIST_BOTTOM + PLAYERSETUP_SCROLLBUTTON_HEIGHT + 1)) {
			m_nHoverOption = HOVEROPTION_PAGEDOWN;

		} else if (m_nMousePosX > MENU_X_RIGHT_ALIGNED(PLAYERSETUP_LIST_RIGHT - 4)
			&& m_nMousePosX < MENU_X_RIGHT_ALIGNED(PLAYERSETUP_LIST_RIGHT - PLAYERSETUP_SCROLLBAR_WIDTH)
#ifdef FIX_BUGS
			&& m_nMousePosY > MENU_Y(PLAYERSETUP_LIST_BODY_TOP + m_nScrollbarTopMargin)
			&& m_nMousePosY < MENU_Y(PLAYERSETUP_LIST_BODY_TOP - 8 + m_nScrollbarTopMargin + scrollbarHeight)) {
#else
			&& m_nMousePosY > MENU_Y(SCROLLBAR_MAX_HEIGHT / m_nTotalListRow + PLAYERSETUP_LIST_BODY_TOP - 3 + m_nScrollbarTopMargin)
			&& m_nMousePosY < MENU_Y(PLAYERSETUP_LIST_BODY_TOP - 3 + m_nScrollbarTopMargin + scrollbarHeight - SCROLLBAR_MAX_HEIGHT / m_nTotalListRow)) {
#endif
			m_nHoverOption = HOVEROPTION_HOLDING_SCROLLBAR;

		} else if (m_nMousePosX > MENU_X_LEFT_ALIGNED(PLAYERSETUP_LIST_LEFT) && m_nMousePosX < MENU_X_RIGHT_ALIGNED(PLAYERSETUP_LIST_RIGHT)
			&& m_nMousePosY > MENU_Y(PLAYERSETUP_LIST_BODY_TOP + 1) && m_nMousePosY < SCREEN_SCALE_FROM_BOTTOM(PLAYERSETUP_LIST_BOTTOM)) {
			m_nHoverOption = HOVEROPTION_LIST;

		} else {
			m_nHoverOption = HOVEROPTION_NOT_HOVERING;
		}
	}
	CFont::SetFontStyle(FONT_HEADING);
	CFont::SetScale(MENU_X(SMALLTEXT_X_SCALE), MENU_Y(SMALLTEXT_Y_SCALE));
	CFont::SetRightJustifyOn();
	CFont::SetColor(CRGBA(0, 0, 0, FadeIn(90)));

	// Back button
	for (int i = 0; i < 2; i++) {
		CFont::PrintString(MENU_X_RIGHT_ALIGNED(PLAYERSETUP_LIST_RIGHT - 3 - i), SCREEN_SCALE_FROM_BOTTOM(PLAYERSETUP_LIST_BOTTOM - 5 - i), TheText.Get("FEDS_TB"));
		if (m_nHoverOption == HOVEROPTION_BACK) {
			CFont::SetColor(CRGBA(255, 217, 106, FadeIn(255)));
		} else {
			CFont::SetColor(CRGBA(235, 170, 50, FadeIn(255)));
		}
	}
	CFont::SetRightJustifyOff();
	CFont::SetColor(CRGBA(0, 0, 0, FadeIn(90)));

	// Use skin button
	for (int i = 0; i < 2; i++) {
		CFont::PrintString(MENU_X_LEFT_ALIGNED(i + PLAYERSETUP_LIST_LEFT), SCREEN_SCALE_FROM_BOTTOM(PLAYERSETUP_LIST_BOTTOM - 5 - i), TheText.Get("FES_SET"));
		if (!strcmp(m_aSkinName, m_PrefsSkinFile)) {
			CFont::SetColor(CRGBA(155, 117, 6, FadeIn(255)));
		} else if (m_nHoverOption == HOVEROPTION_USESKIN) {
			CFont::SetColor(CRGBA(255, 217, 106, FadeIn(255)));
		} else {
			CFont::SetColor(CRGBA(235, 170, 50, FadeIn(255)));
		}
	}

}

int
CMenuManager::FadeIn(int alpha)
{
	if (m_nCurrScreen == MENUPAGE_LOADING_IN_PROGRESS ||
		m_nCurrScreen == MENUPAGE_SAVING_IN_PROGRESS ||
		m_nCurrScreen == MENUPAGE_DELETING)
		return alpha;

	return min(m_nMenuFadeAlpha, alpha);
}

void
CMenuManager::FilterOutColorMarkersFromString(wchar *str, CRGBA &newColor)
{
	int newIdx = 0;
	wchar copy[256], *c;
	UnicodeStrcpy(copy, str);

	for (c = copy; *c != '\0'; c++) {
		if (*c == '~') {
			c++;
			switch (*c) {
				case 'b': newColor = CRGBA(40, 40, 255, 255); break;
				case 'g': newColor = CRGBA(40, 235, 40, 255); break;
				// There is no case for "h", is that a mistake?
				case 'l': newColor = CRGBA(0, 0, 0, 255); break;
				case 'p': newColor = CRGBA(255, 0, 255, 255); break;
				case 'r': newColor = CRGBA(255, 0, 0, 255); break;
				case 'w': newColor = CRGBA(255, 255, 255, 255); break;
				case 'y': newColor = CRGBA(255, 255, 0, 255); break;
			}
			while (*c != '~') c++;
		} else {
			str[newIdx++] = *c;
		}
	}
	str[newIdx] = '\0';
}

int
CMenuManager::GetStartOptionsCntrlConfigScreens()
{
	int number = 0;
	switch (m_nCurrScreen) {
		case MENUPAGE_CONTROLLER_PC_OLD3:
			number = 34;
			break;
		case MENUPAGE_CONTROLLER_DEBUG:
			number = 35;
			break;
		case MENUPAGE_KEYBOARD_CONTROLS:
			number = 0;
			break;
		default:
			break;
	}
	return number;
}

void
CMenuManager::InitialiseChangedLanguageSettings()
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
		switch (m_PrefsLanguage) {
		case LANGUAGE_RUSSIAN:
			CFont::ReloadFonts(FONT_LANGSET_RUSSIAN);
			break;
		default:
			CFont::ReloadFonts(FONT_LANGSET_EFIGS);
			break;
		}
#endif

		switch (m_PrefsLanguage) {
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

void
CMenuManager::LoadAllTextures()
{
	if (m_bSpritesLoaded)
		return;

	CentreMousePointer();
	DMAudio.ChangeMusicMode(MUSICMODE_FRONTEND);
	DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_STARTING, 0);
	m_nCurrOption = 0;
	m_PrefsRadioStation = DMAudio.GetRadioInCar();

	if (DMAudio.IsMP3RadioChannelAvailable()) {
		if (m_PrefsRadioStation > USERTRACK)
			m_PrefsRadioStation = CGeneral::GetRandomNumber() % 10;
	} else if (m_PrefsRadioStation > CHATTERBOX)
		m_PrefsRadioStation = CGeneral::GetRandomNumber() % 9;
	
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
#ifdef MENU_MAP
	for (int i = 0; i < ARRAY_SIZE(MapFilenames); i++) {
		m_aMapSprites[i].SetTexture(MapFilenames[i][0], MapFilenames[i][1]);
		m_aMapSprites[i].SetAddressing(rwTEXTUREADDRESSBORDER);
	}
#endif
	m_bSpritesLoaded = true;
	CTxdStore::PopCurrentTxd();
}

void
CMenuManager::LoadSettings()
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

void
CMenuManager::SaveSettings()
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

bool DoRWStuffStartOfFrame(int16 TopRed, int16 TopGreen, int16 TopBlue, int16 BottomRed, int16 BottomGreen, int16 BottomBlue, int16 Alpha);
void DoRWStuffEndOfFrame(void);

void
CMenuManager::MessageScreen(const char *text)
{
	CSprite2d *splash = LoadSplash(nil);
	if (!DoRWStuffStartOfFrame(0, 0, 0, 0, 0, 0, 255))
		return;

	CSprite2d::SetRecipNearClip();
	CSprite2d::InitPerFrame();
	CFont::InitPerFrame();
	DefinedState();

	RwRenderStateSet(rwRENDERSTATETEXTUREADDRESS, (void*)rwTEXTUREADDRESSCLAMP);
	splash->Draw(CRect(0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT), CRGBA(255, 255, 255, 255));

	CFont::SetBackgroundOff();
	CFont::SetPropOn();
	CFont::SetJustifyOn();
	CFont::SetBackGroundOnlyTextOn();
	CFont::SetWrapx(SCREEN_SCALE_FROM_RIGHT(170.0f));
	CFont::SetRightJustifyWrap(SCREEN_SCALE_FROM_RIGHT(170.0f));
	CSprite2d::DrawRect(CRect(SCREEN_SCALE_X(120.0f), SCREEN_SCALE_Y(150.0f), SCREEN_SCALE_FROM_RIGHT(120.0f), SCREEN_SCALE_FROM_BOTTOM(220.0f)), CRGBA(50, 50, 50, 210));
	CFont::SetFontStyle(FONT_BANK);
	CFont::SetCentreSize(SCREEN_SCALE_X(380.0f));
	CFont::SetCentreOn();
	CFont::SetColor(CRGBA(255, 217, 106, 255));
	CFont::SetScale(SCREEN_SCALE_X(SMALLTEXT_X_SCALE), SCREEN_SCALE_Y(SMALLTEXT_Y_SCALE));
	CFont::PrintString(SCREEN_SCALE_X(320.0f), SCREEN_SCALE_Y(170.0f), TheText.Get(text));
	CFont::DrawFonts();
	DoRWStuffEndOfFrame();
}

void
CMenuManager::PickNewPlayerColour()
{
	m_PrefsPlayerRed = 0;
	m_PrefsPlayerGreen = 0;
	m_PrefsPlayerBlue = 0;
	while (true) {
		int sum = m_PrefsPlayerRed + m_PrefsPlayerGreen + m_PrefsPlayerBlue;
		if (sum >= 100 && sum <= 650)
			break;
		m_PrefsPlayerRed = CGeneral::GetRandomNumber();
		m_PrefsPlayerGreen = CGeneral::GetRandomNumber();
		m_PrefsPlayerBlue = CGeneral::GetRandomNumber();
	}
}

void
CMenuManager::PrintBriefs()
{
	CFont::SetColor(CRGBA(235, 170, 50, FadeIn(255)));
	CFont::SetFontStyle(FONT_BANK);
	CFont::SetRightJustifyOff();
	CFont::SetScale(MENU_X(MENU_TEXT_SIZE_X * 0.7), MENU_Y(MENU_TEXT_SIZE_Y * 0.9)); // second mulipliers are double, idk why

	float nextY = 40.0f;
	CRGBA newColor;
	for (int i = 4; i >= 0; i--) {
		tPreviousBrief &brief = CMessages::PreviousBriefs[i];
		if (brief.m_pText) {
			CMessages::InsertNumberInString(brief.m_pText,
				brief.m_nNumber[0], brief.m_nNumber[1],
				brief.m_nNumber[2], brief.m_nNumber[3],
				brief.m_nNumber[4], brief.m_nNumber[5], gUString);
			CMessages::InsertStringInString(gUString, brief.m_pString);
			CMessages::InsertPlayerControlKeysInString(gUString);
			newColor = TEXT_COLOR;
			FilterOutColorMarkersFromString(gUString, newColor);

#ifdef PS2_LIKE_MENU
			// This PS2 code was always here, but unused
			bool rgSame = newColor.r == TEXT_COLOR.r && newColor.g == TEXT_COLOR.g;
			bool bSame = rgSame && newColor.b == TEXT_COLOR.b;
			bool colorNotChanged = bSame; /* && newColor.a == TEXT_COLOR.a; */

			if (!colorNotChanged) {
				newColor.r /= 2;
				newColor.g /= 2;
				newColor.b /= 2;
			}
			CFont::SetDropColor(CRGBA(0, 0, 0, FadeIn(255))); // But this is from PS2
			CFont::SetDropShadowPosition(1);
#endif

#if defined(FIX_BUGS) || defined(PS2_LIKE_MENU)
			newColor.a = FadeIn(255);
			CFont::SetColor(newColor);
#endif
			CFont::PrintString(MENU_X_LEFT_ALIGNED(50.0f), nextY, gUString);
			nextY += MENU_Y(menuXYpadding);
		}
	}

#ifdef PS2_LIKE_MENU
	CFont::SetDropShadowPosition(0);
#endif
}

// Not sure about name. Not to be confused with CPad::PrintErrorMessage
void
CMenuManager::PrintErrorMessage()
{
	if (!CPad::bDisplayNoControllerMessage && !CPad::bObsoleteControllerMessage)
		return;

	CSprite2d::DrawRect(CRect(SCREEN_SCALE_X(20.0f), SCREEN_SCALE_Y(140.0f), SCREEN_SCALE_FROM_RIGHT(20.0f), SCREEN_SCALE_FROM_BOTTOM(120.0f)), CRGBA(64, 16, 16, 224));
	CFont::SetFontStyle(FONT_BANK);
	CFont::SetBackgroundOff();
	CFont::SetPropOn();
	CFont::SetCentreOff();
	CFont::SetJustifyOn();
	CFont::SetRightJustifyOff();
	CFont::SetBackGroundOnlyTextOn();
	CFont::SetWrapx(SCREEN_SCALE_FROM_RIGHT(40.0f));
#ifdef FIX_BUGS
	CFont::PrintString(SCREEN_SCALE_X(50.0f), SCREEN_SCALE_Y(180.0f), TheText.Get(CPad::bDisplayNoControllerMessage ? "NOCONT" : "WRCONT"));
#else
	CFont::PrintString(SCREEN_SCALE_X(50.0f), SCREEN_SCALE_Y(40.0f), TheText.Get(CPad::bDisplayNoControllerMessage ? "NOCONT" : "WRCONT"));
#endif
	CFont::DrawFonts();
}

void
CMenuManager::PrintStats()
{
	int rowNum = ConstructStatLine(99999);
	CFont::SetScale(MENU_X(MENU_TEXT_SIZE_X * 0.7), MENU_Y(MENU_TEXT_SIZE_Y * 0.9)); // second mulipliers are double, idk why
	float nextYChange, y, alphaMult;

	// Scroll stats with mouse
#ifdef SCROLLABLE_STATS_PAGE
	static float scrollY = 0;
	static uint32 lastChange = m_nScreenChangeDelayTimer;
	if (CPad::GetPad(0)->GetLeftMouse()) {
		scrollY += (m_nMouseOldPosY - m_nMousePosY);
		lastChange = CTimer::GetTimeInMillisecondsPauseMode();
	} else {
		scrollY += MENU_Y(STATS_SLIDE_Y_PER_SECOND) / 1000.0f * (CTimer::GetTimeInMillisecondsPauseMode() - lastChange);
		lastChange = CTimer::GetTimeInMillisecondsPauseMode();
	}
#else
	// MENU_Y(30.0f) per second
	float scrollY = MENU_Y(STATS_SLIDE_Y_PER_SECOND) * (CTimer::GetTimeInMillisecondsPauseMode() - m_nScreenChangeDelayTimer) / 1000.0f;
#endif

	for (int row = 0; row < rowNum; ++row) {
		// Put just got hidden text at the top back to the bottom, in circular fashion
		for (y = MENU_Y(STATS_ROW_HEIGHT - 1) * row + SCREEN_HEIGHT - scrollY; MENU_Y(STATS_PUT_BACK_TO_BOTTOM_Y) > y; y += nextYChange) {
			nextYChange = (MENU_Y(STATS_ROW_HEIGHT) + rowNum) * MENU_Y(STATS_ROW_HEIGHT - 1);
		}

		// If it's still on screen
		if (y > 0.0f && SCREEN_HEIGHT > y) {
			ConstructStatLine(row);

			// But about to dim from top
			if (y - MENU_Y(STATS_BOTTOM_MARGIN) < MENU_Y(STATS_TOP_DIMMING_AREA_LENGTH)) {
				if ((y - MENU_Y(STATS_BOTTOM_MARGIN)) / MENU_Y(STATS_TOP_DIMMING_AREA_LENGTH) < 0.0f)
					alphaMult = 0.0f;
				else
					alphaMult = (y - MENU_Y(STATS_BOTTOM_MARGIN)) / MENU_Y(STATS_TOP_DIMMING_AREA_LENGTH);

			// About to dim from bottom
			} else if (y > SCREEN_SCALE_FROM_BOTTOM(STATS_TOP_DIMMING_AREA_LENGTH) - MENU_Y(STATS_BOTTOM_DIMMING_AREA_LENGTH)) {
				if ((SCREEN_SCALE_FROM_BOTTOM(STATS_BOTTOM_DIMMING_AREA_LENGTH) - y) / MENU_Y(STATS_TOP_DIMMING_AREA_LENGTH) < 0.0f)
					alphaMult = 0.0f;
				else
					alphaMult = (SCREEN_SCALE_FROM_BOTTOM(STATS_BOTTOM_DIMMING_AREA_LENGTH) - y) / MENU_Y(STATS_TOP_DIMMING_AREA_LENGTH);
			} else
				alphaMult = 1.0f;

			CFont::SetColor(CRGBA(235, 170, 50, FadeIn(255.0f * alphaMult)));
			CFont::SetRightJustifyOff();
			CFont::PrintString(MENU_X_LEFT_ALIGNED(STATS_ROW_X_MARGIN), y - MENU_Y(STATS_BOTTOM_MARGIN - STATS_TOP_MARGIN), gUString);
			CFont::SetRightJustifyOn();
			CFont::PrintString(MENU_X_RIGHT_ALIGNED(STATS_ROW_X_MARGIN), y - MENU_Y(STATS_BOTTOM_MARGIN - STATS_TOP_MARGIN), gUString2);
		}
	}
	// Game doesn't do that, but it's better
	float nextX = MENU_X_LEFT_ALIGNED(STATS_RATING_X);

	CFont::SetColor(CRGBA(235, 170, 50, FadeIn(255)));
	CFont::SetRightJustifyOff();
	CFont::PrintString(nextX, MENU_Y(STATS_RATING_Y), TheText.Get("CRIMRA")); nextX += MENU_X(10.0f) + CFont::GetStringWidth(TheText.Get("CRIMRA"), true);
	UnicodeStrcpy(gUString, CStats::FindCriminalRatingString());
	CFont::PrintString(nextX, MENU_Y(STATS_RATING_Y), gUString); nextX += MENU_X(6.0f) + CFont::GetStringWidth(gUString, true);
	sprintf(gString, "%d", CStats::FindCriminalRatingNumber());
	AsciiToUnicode(gString, gUString);
	CFont::PrintString(nextX, MENU_Y(STATS_RATING_Y), gUString);

	// ::Draw already does that.
	/*
	CFont::SetColor(CRGBA(0, 0, 0, FadeIn(255)));
	CFont::SetRightJustifyOn();
	CFont::SetFontStyle(FONT_HEADING);
	CFont::SetScale(MENU_X(MENUHEADER_WIDTH), MENU_Y(MENUHEADER_HEIGHT));
	CFont::PrintString(MENU_X_RIGHT_ALIGNED(MENUHEADER_POS_X), SCREEN_SCALE_FROM_BOTTOM(MENUHEADER_POS_Y), TheText.Get(aScreens[m_nCurrScreen].m_ScreenName));
	*/
	CFont::SetScale(MENU_X(MENU_TEXT_SIZE_X), MENU_Y(MENU_TEXT_SIZE_Y));
}

void
CMenuManager::Process(void)
{
	m_bMenuStateChanged = false;

	if (!m_bSaveMenuActive && TheCamera.GetScreenFadeStatus() != FADE_0)
		return;

	m_bWantToRestart = false;
	InitialiseChangedLanguageSettings();

	// Just a hack by R* to not make game continuously resume/pause. But we it seems we can live with it.
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
				TheCamera.m_bUseMouse3rdPerson = m_ControlMethod == CONTROL_STANDARD;
				if (m_PrefsVsyncDisp != m_PrefsVsync)
					m_PrefsVsync = m_PrefsVsyncDisp;
				DMAudio.Service();
				m_bWantToRestart = true;
				RequestFrontEndShutDown();
				m_bWantToLoad = true;
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
				JoyButtonJustClicked = false;
				MouseButtonJustClicked = false;

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

				int32 TypeOfControl = KEYBOARD;
				if (JoyButtonJustClicked)
					TypeOfControl = JOYSTICK;
				if (MouseButtonJustClicked)
					TypeOfControl = MOUSE;
				if (*pControlEdit != rsNULL)
					TypeOfControl = KEYBOARD;

				if (!m_bKeyIsOK) {
					DMAudio.PlayFrontEndSound(SOUND_FRONTEND_FAIL, 0);
					pControlEdit = nil;
					m_bWaitingForNewKeyBind = false;
					m_KeyPressedCode = -1;
					m_bStartWaitingForKeyBind = false;
				} else if (!m_bKeyChangeNotProcessed) {
					if (*pControlEdit != rsNULL || MouseButtonJustClicked || JoyButtonJustClicked)
						CheckCodesForControls(TypeOfControl);

					field_535 = true;
				} else {
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

		if ((m_nCurrScreen == MENUPAGE_NO_MEMORY_CARD || m_nCurrScreen == MENUPAGE_PS2_LOAD_FAILED) && CTimer::GetTimeInMillisecondsPauseMode() > field_558) {
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

	if (!m_bWantToRestart) {
		if (m_bGameNotLoaded)
			DMAudio.Service();
	}
}

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
			if (m_nCurrExLayer == HOVEROPTION_LIST) {
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
				case HOVEROPTION_BACK:
				default:
					m_nCurrExLayer = HOVEROPTION_LIST;
					break;
				case HOVEROPTION_LIST:
					m_nCurrExLayer = HOVEROPTION_USESKIN;
					break;
				case HOVEROPTION_USESKIN:
					m_nCurrExLayer = HOVEROPTION_BACK;
			}
			if (((m_nCurrScreen == MENUPAGE_SKIN_SELECT) && (m_nCurrExLayer == HOVEROPTION_USESKIN)) && strcmp(m_aSkinName, m_PrefsSkinFile) == 0) {
				m_nCurrExLayer = HOVEROPTION_BACK;
			}
			if ((m_nCurrScreen == MENUPAGE_KEYBOARD_CONTROLS) && (m_nCurrExLayer == HOVEROPTION_USESKIN)) {
				m_nCurrExLayer = HOVEROPTION_BACK;
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
			m_nCurrExLayer = HOVEROPTION_LIST;
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
			m_nCurrExLayer = HOVEROPTION_LIST;
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
				m_nCurrExLayer = HOVEROPTION_LIST;
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
				m_nCurrExLayer = HOVEROPTION_LIST;
				if (!m_bPressedPgDnOnList) {
					m_bPressedPgDnOnList = true;
					lastTimeClickedScrollButton = CTimer::GetTimeInMillisecondsPauseMode();
					m_bShowMouse = false;
					DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_DENIED, 0);
					PageDownList(false);
				}
			}
			if (CPad::GetPad(0)->GetHome()) {
				m_nCurrExLayer = HOVEROPTION_LIST;
				m_bShowMouse = false;
				DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_DENIED, 0);
				if (m_nTotalListRow >= MAX_VISIBLE_LIST_ROW) {
					m_nFirstVisibleRowOnList = 0;
				}
				m_nSelectedListRow = 0;
				m_nScrollbarTopMargin = (SCROLLBAR_MAX_HEIGHT / m_nTotalListRow) * m_nFirstVisibleRowOnList;
			}
			if (CPad::GetPad(0)->GetEnd()) {
				m_nCurrExLayer = HOVEROPTION_LIST;
				m_bShowMouse = false;
				DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_DENIED, 0);
				if (m_nTotalListRow >= MAX_VISIBLE_LIST_ROW) {
					m_nFirstVisibleRowOnList = m_nTotalListRow - MAX_VISIBLE_LIST_ROW;
				}
				m_nSelectedListRow = m_nTotalListRow - 1;
				m_nScrollbarTopMargin = (SCROLLBAR_MAX_HEIGHT / m_nTotalListRow) * m_nFirstVisibleRowOnList;
			}
		}

#ifndef TIDY_UP_PBP
		if (CPad::GetPad(0)->GetEscapeJustDown() || CPad::GetPad(0)->GetBackJustDown()) {
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
				case HOVEROPTION_USESKIN:
					if (m_nSkinsTotal > 0) {
						DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_SUCCESS, 0);
						m_pSelectedSkin = m_pSkinListHead.nextSkin;
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
				case HOVEROPTION_LIST:
					m_nHoverOption = HOVEROPTION_SKIN;
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
				assert(0 && "Holding scrollbar isn't done yet");
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
		if (CPad::GetPad(0)->GetEscapeJustDown() || CPad::GetPad(0)->GetBackJustUp()) {
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
				m_PrefsRadioStation = m_nHoverOption - HOVEROPTION_RADIO_0;
				SaveSettings();
				DMAudio.SetRadioInCar(m_PrefsRadioStation);
				DMAudio.PlayFrontEndTrack(m_PrefsRadioStation, 1);
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
				m_PrefsRadioStation = HEAD_RADIO;
				SaveSettings();
				DMAudio.SetRadioInCar(m_PrefsRadioStation);
				DMAudio.PlayFrontEndTrack(m_PrefsRadioStation, 1);
				OutputDebugString("FRONTEND RADIO STATION CHANGED");
				break;
			case HOVEROPTION_RADIO_1:
				DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_SUCCESS, 0);
				m_PrefsRadioStation = DOUBLE_CLEF;
				SaveSettings();
				DMAudio.SetRadioInCar(m_PrefsRadioStation);
				DMAudio.PlayFrontEndTrack(m_PrefsRadioStation, 1);
				OutputDebugString("FRONTEND RADIO STATION CHANGED");
				break;
			case HOVEROPTION_RADIO_2:
				DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_SUCCESS, 0);
				m_PrefsRadioStation = JAH_RADIO;
				SaveSettings();
				DMAudio.SetRadioInCar(m_PrefsRadioStation);
				DMAudio.PlayFrontEndTrack(m_PrefsRadioStation, 1);
				OutputDebugString("FRONTEND RADIO STATION CHANGED");
				break;
			case HOVEROPTION_RADIO_3:
				DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_SUCCESS, 0);
				m_PrefsRadioStation = RISE_FM;
				SaveSettings();
				DMAudio.SetRadioInCar(m_PrefsRadioStation);
				DMAudio.PlayFrontEndTrack(m_PrefsRadioStation, 1);
				OutputDebugString("FRONTEND RADIO STATION CHANGED");
				break;
			case HOVEROPTION_RADIO_4:
				DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_SUCCESS, 0);
				m_PrefsRadioStation = LIPS_106;
				SaveSettings();
				DMAudio.SetRadioInCar(m_PrefsRadioStation);
				DMAudio.PlayFrontEndTrack(m_PrefsRadioStation, 1);
				OutputDebugString("FRONTEND RADIO STATION CHANGED");
				break;
			case HOVEROPTION_RADIO_5:
				DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_SUCCESS, 0);
				m_PrefsRadioStation = GAME_FM;
				SaveSettings();
				DMAudio.SetRadioInCar(m_PrefsRadioStation);
				DMAudio.PlayFrontEndTrack(m_PrefsRadioStation, 1);
				OutputDebugString("FRONTEND RADIO STATION CHANGED");
				break;
			case HOVEROPTION_RADIO_6:
				DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_SUCCESS, 0);
				m_PrefsRadioStation = MSX_FM;
				SaveSettings();
				DMAudio.SetRadioInCar(m_PrefsRadioStation);
				DMAudio.PlayFrontEndTrack(m_PrefsRadioStation, 1);
				OutputDebugString("FRONTEND RADIO STATION CHANGED");
				break;
			case HOVEROPTION_RADIO_7:
				DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_SUCCESS, 0);
				m_PrefsRadioStation = FLASHBACK;
				SaveSettings();
				DMAudio.SetRadioInCar(m_PrefsRadioStation);
				DMAudio.PlayFrontEndTrack(m_PrefsRadioStation, 1);
				OutputDebugString("FRONTEND RADIO STATION CHANGED");
				break;
			case HOVEROPTION_RADIO_8:
				DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_SUCCESS, 0);
				m_PrefsRadioStation = CHATTERBOX;
				SaveSettings();
				DMAudio.SetRadioInCar(m_PrefsRadioStation);
				DMAudio.PlayFrontEndTrack(m_PrefsRadioStation, 1);
				OutputDebugString("FRONTEND RADIO STATION CHANGED");
				break;
			case HOVEROPTION_RADIO_9:
				DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_SUCCESS, 0);
				m_PrefsRadioStation = USERTRACK;
				SaveSettings();
				DMAudio.SetRadioInCar(m_PrefsRadioStation);
				DMAudio.PlayFrontEndTrack(m_PrefsRadioStation, 1);
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
				m_PrefsBrightness = m_PrefsBrightness + 32;
				if (m_PrefsBrightness < 0) {
					m_PrefsBrightness = 0;
				}
				if (510 < m_PrefsBrightness) {
					m_PrefsBrightness = 511;
				}
				SaveSettings();
				break;
			case HOVEROPTION_DECREASE_BRIGHTNESS:
				m_PrefsBrightness = m_PrefsBrightness - 32;
				if (m_PrefsBrightness < 0) {
					m_PrefsBrightness = 0;
				}
				if (510 < m_PrefsBrightness) {
					m_PrefsBrightness = 511;
				}
				SaveSettings();
				break;
			case HOVEROPTION_INCREASE_DRAWDIST:
				m_PrefsLOD = m_PrefsLOD + (1.0f / 16);
				m_PrefsLOD = min(1.8f, m_PrefsLOD);
				CRenderer::ms_lodDistScale = m_PrefsLOD;
				SaveSettings();
				break;
			case HOVEROPTION_DECREASE_DRAWDIST:
				m_PrefsLOD = m_PrefsLOD - (1.0f / 16);
				m_PrefsLOD = max(0.8f, m_PrefsLOD);
				CRenderer::ms_lodDistScale = m_PrefsLOD;
				SaveSettings();
				break;
			case HOVEROPTION_INCREASE_MUSICVOLUME:
				m_PrefsMusicVolume = m_PrefsMusicVolume + 8;
				m_PrefsMusicVolume = clamp(m_PrefsMusicVolume, 0, 127);
				DMAudio.SetMusicMasterVolume(uchar)(m_PrefsMusicVolume);
				SaveSettings();
				break;
			case HOVEROPTION_DECREASE_MUSICVOLUME:
				m_PrefsMusicVolume = m_PrefsMusicVolume - 8;
				if (m_PrefsMusicVolume < 0) {
					m_PrefsMusicVolume = 0;
				}
				if (126 < m_PrefsMusicVolume) {
					m_PrefsMusicVolume = 127;
				}
				DMAudio.SetMusicMasterVolume(uchar)(m_PrefsMusicVolume);
				SaveSettings();
				break;
			case HOVEROPTION_INCREASE_SFXVOLUME:
				m_PrefsSFXVolume = m_PrefsSFXVolume + 8;
				if (m_PrefsSFXVolume < 0) {
					m_PrefsSFXVolume = 0;
				}
				if (126 < m_PrefsSFXVolume) {
					m_PrefsSFXVolume = 127;
				}
				DMAudio.SetEffectsMasterVolume(uchar)(m_PrefsSFXVolume);
				SaveSettings();
				break;
			case HOVEROPTION_DECREASE_SFXVOLUME:
				m_PrefsSFXVolume = m_PrefsSFXVolume - 8;
				if (m_PrefsSFXVolume < 0) {
					m_PrefsSFXVolume = 0;
				}
				if (126 < m_PrefsSFXVolume) {
					m_PrefsSFXVolume = 127;
				}
				DMAudio.SetEffectsMasterVolume(uchar)(m_PrefsSFXVolume);
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
		if (CPad::GetPad(0)->GetBackJustDown()) {
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
				if (CPad::GetPad(0)->GetEscapeJustDown() || CPad::GetPad(0)->GetBackJustUp()) {
					goBack = true;
				}
			} else {
				if (CPad::GetPad(0)->GetEscapeJustDown() || (m_nCurrScreen != MENUPAGE_PAUSE_MENU && CPad::GetPad(0)->GetBackJustDown())) {
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
				case HOVEROPTION_LIST:
					if (m_nCurrScreen == MENUPAGE_KEYBOARD_CONTROLS) {
						m_bWaitingForNewKeyBind = true;
						m_bStartWaitingForKeyBind = true;
						pControlEdit = &m_KeyPressedCode;
					}
					if (m_nCurrScreen == MENUPAGE_SKIN_SELECT) {
						strcpy(m_PrefsSkinFile, m_aSkinName);
						CWorld::Players[0].SetPlayerSkin(m_PrefsSkinFile);
						m_nCurrExLayer = HOVEROPTION_BACK;
						SaveSettings();
					}
					m_nHoverOption = HOVEROPTION_NOT_HOVERING;
					break;
				case HOVEROPTION_USESKIN:
					m_nHoverOption = HOVEROPTION_NOT_HOVERING;
					strcpy(m_PrefsSkinFile, m_aSkinName);
					CWorld::Players[0].SetPlayerSkin(m_PrefsSkinFile);
					m_nCurrExLayer = HOVEROPTION_BACK;
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
				m_pSelectedSkin = m_pSkinListHead.nextSkin;
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
					++m_PrefsRadioStation;
					if (DMAudio.IsMP3RadioChannelAvailable()) {
						if (m_PrefsRadioStation > USERTRACK)
							m_PrefsRadioStation = HEAD_RADIO;
					} else if (m_PrefsRadioStation > CHATTERBOX) {
						m_PrefsRadioStation = USERTRACK;
					}
					SaveSettings();
					DMAudio.SetRadioInCar(m_PrefsRadioStation);
					DMAudio.PlayFrontEndTrack(m_PrefsRadioStation, 1);
					OutputDebugString("FRONTEND RADIO STATION CHANGED");
#endif
					break;
				case MENUACTION_LANG_ENG:
					m_PrefsLanguage = LANGUAGE_AMERICAN;
					m_bFrontEnd_ReloadObrTxtGxt = true;
					InitialiseChangedLanguageSettings();
					SaveSettings();
					break;
				case MENUACTION_LANG_FRE:
					m_PrefsLanguage = LANGUAGE_FRENCH;
					m_bFrontEnd_ReloadObrTxtGxt = true;
					InitialiseChangedLanguageSettings();
					SaveSettings();
					break;
				case MENUACTION_LANG_GER:
					m_PrefsLanguage = LANGUAGE_GERMAN;
					m_bFrontEnd_ReloadObrTxtGxt = true;
					InitialiseChangedLanguageSettings();
					SaveSettings();
					break;
				case MENUACTION_LANG_ITA:
					m_PrefsLanguage = LANGUAGE_ITALIAN;
					m_bFrontEnd_ReloadObrTxtGxt = true;
					InitialiseChangedLanguageSettings();
					SaveSettings();
					break;
				case MENUACTION_LANG_SPA:
					m_PrefsLanguage = LANGUAGE_SPANISH;
					m_bFrontEnd_ReloadObrTxtGxt = true;
					InitialiseChangedLanguageSettings();
					SaveSettings();
					break;
#ifdef MORE_LANGUAGES
				case MENUACTION_LANG_RUS:
					m_PrefsLanguage = LANGUAGE_RUSSIAN;
					m_bFrontEnd_ReloadObrTxtGxt = true;
					InitialiseChangedLanguageSettings();
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
							ResetHelperText();
							if (!m_bGameNotLoaded)
								ChangeScreen(aScreens[m_nCurrScreen].m_PreviousPage[1], aScreens[m_nCurrScreen].m_ParentEntry[1], true, true);
							else
								ChangeScreen(aScreens[m_nCurrScreen].m_PreviousPage[0], aScreens[m_nCurrScreen].m_ParentEntry[0], true, true);
#else
							goBack = true;
							break;
#endif
						} else {
#ifdef MENU_MAP
							if (aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption].m_TargetMenu == MENUPAGE_MAP) {
								bMapLoaded = false;
							}

#endif
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
					DoSettingsBeforeStartingAGame();
					break;
				case MENUACTION_RELOADIDE:
					CFileLoader::ReloadObjectTypes("GTA3.IDE");
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
					RequestFrontEndShutDown();
					break;
				case MENUACTION_MPMAP_LIBERTY:
				case MENUACTION_MPMAP_REDLIGHT:
				case MENUACTION_MPMAP_CHINATOWN:
				case MENUACTION_MPMAP_TOWER:
				case MENUACTION_MPMAP_SEWER:
				case MENUACTION_MPMAP_INDUSTPARK:
				case MENUACTION_MPMAP_DOCKS:
				case MENUACTION_MPMAP_STAUNTON:
					m_SelectedMap = option - MENUACTION_MPMAP_LIBERTY;
					SaveSettings();
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
					m_SelectedGameType = option - MENUACTION_MPMAP_DEATHMATCH1;
					SaveSettings();
					ChangeScreen(aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption].m_TargetMenu, 0, true, true);
					break;
				case MENUACTION_REDEFCTRL:
					ChangeScreen(MENUPAGE_KEYBOARD_CONTROLS, 0, true, true);
					m_nSelectedListRow = 0;
					m_nCurrExLayer = HOVEROPTION_LIST;
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
					if (m_PrefsVsyncDisp != m_PrefsVsync) {
						m_PrefsVsync = m_PrefsVsyncDisp;
					}
					RequestFrontEndShutDown();
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
					m_nCurrExLayer = HOVEROPTION_LIST;
					m_bSkinsEnumerated = false;
					break;
				case MENUACTION_RESTOREDEF:
					if (m_nCurrScreen == MENUPAGE_SOUND_SETTINGS) {
						m_PrefsSfxVolume = 102;
						m_PrefsSpeakers = 0;
						m_PrefsMusicVolume = 102;
						m_PrefsStereoMono = 0;
						m_PrefsRadioStation = HEAD_RADIO;
						DMAudio.SetMusicMasterVolume(102);
						DMAudio.SetEffectsMasterVolume(m_PrefsSfxVolume);
						DMAudio.SetRadioInCar(m_PrefsRadioStation);
						DMAudio.PlayFrontEndTrack(m_PrefsRadioStation, 1);
						SaveSettings();
					} else if (m_nCurrScreen == MENUPAGE_GRAPHICS_SETTINGS) {
						m_PrefsFrameLimiter = true;
						m_PrefsBrightness = 256;
						m_PrefsVsyncDisp = true;
						m_PrefsLOD = 1.2f;
						m_PrefsVsync = true;
						CRenderer::ms_lodDistScale = 1.2f;
						m_PrefsUseWideScreen = false;
						m_PrefsShowSubtitles = true;
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
						m_ControlMethod = CONTROL_STANDARD;
						MousePointerStateHelper.bInvertVertically = false;
						TheCamera.m_fMouseAccelHorzntl = 0.0025f;
						CVehicle::m_bDisableMouseSteering = true;
						TheCamera.m_bHeadBob = false;
						SaveSettings();
					}
					SetHelperText(2);
					break;
				case MENUACTION_CTRLMETHOD:
#ifndef TIDY_UP_PBP
					if (m_ControlMethod == CONTROL_CLASSIC) {
						CCamera::m_bUseMouse3rdPerson = true;
						m_ControlMethod = CONTROL_STANDARD;
					} else {
						CCamera::m_bUseMouse3rdPerson = false;
						m_ControlMethod = CONTROL_CLASSIC;
					}
					SaveSettings();
#else
					assumeIncrease = true;
#endif
					break;
				case MENUACTION_LOADRADIO:
					ChangeScreen(MENUPAGE_SOUND_SETTINGS, 0, true, true);
					DMAudio.PlayFrontEndTrack(m_PrefsRadioStation, 1);
					OutputDebugString("STARTED PLAYING FRONTEND AUDIO TRACK");
					break;
			}
		}
		ProcessOnOffMenuOptions();
	}

	if (goBack) {
		ResetHelperText();
		DMAudio.PlayFrontEndSound(SOUND_FRONTEND_EXIT, 0);
#ifdef PS2_LIKE_MENU
		if (m_nCurrScreen == MENUPAGE_PAUSE_MENU || bottomBarActive) {
#else
		if (m_nCurrScreen == MENUPAGE_PAUSE_MENU) {
#endif
			if (!m_bGameNotLoaded && !m_bMenuStateChanged) {
				if (m_PrefsVsyncDisp != m_PrefsVsync) {
					m_PrefsVsync = m_PrefsVsyncDisp;
				}
				RequestFrontEndShutDown();
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
			RequestFrontEndShutDown();
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
			CheckSliderMovement(-1);
			lastSliderDecrease = CTimer::GetTimeInMillisecondsPauseMode();
		}
	} else if (CPad::GetPad(0)->GetRight() || CPad::GetPad(0)->GetPedWalkLeftRight() > 0 || CPad::GetPad(0)->GetDPadRight()) {
		static uint32 lastSliderIncrease = 0;
		if (CTimer::GetTimeInMillisecondsPauseMode() - lastSliderIncrease > 150) {
			CheckSliderMovement(1);
			lastSliderIncrease = CTimer::GetTimeInMillisecondsPauseMode();
		}
	}

	if (CPad::GetPad(0)->GetRightJustDown() || CPad::GetPad(0)->GetAnaloguePadRight() || CPad::GetPad(0)->GetDPadRightJustDown()) {
		m_bShowMouse = false;
		increase = true;
	} else if (CPad::GetPad(0)->GetMouseWheelUpJustDown() && m_nCurrScreen != MENUPAGE_KEYBOARD_CONTROLS) {
		increase = true;
		CheckSliderMovement(1);
		m_bShowMouse = true;
	}

	if (!CPad::GetPad(0)->GetLeftJustDown() && !CPad::GetPad(0)->GetAnaloguePadLeft() && !CPad::GetPad(0)->GetDPadLeftJustDown()) {
		if (CPad::GetPad(0)->GetMouseWheelDownJustDown()) {
			if (m_nCurrScreen != MENUPAGE_KEYBOARD_CONTROLS) {
				decrease = true;
				CheckSliderMovement(-1);
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
				m_PrefsRadioStation += changeValueBy;
				DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_SUCCESS, 0);
				if (DMAudio.IsMP3RadioChannelAvailable()) {
					if (m_PrefsRadioStation < HEAD_RADIO)
						m_PrefsRadioStation = USERTRACK;
					if (m_PrefsRadioStation > USERTRACK)
						m_PrefsRadioStation = HEAD_RADIO;
				} else {
					if (m_PrefsRadioStation < HEAD_RADIO)
						m_PrefsRadioStation = CHATTERBOX;
					if (m_PrefsRadioStation > CHATTERBOX)
						m_PrefsRadioStation = HEAD_RADIO;
				}
				SaveSettings();
				DMAudio.SetRadioInCar(m_PrefsRadioStation);
				DMAudio.PlayFrontEndTrack(m_PrefsRadioStation, 1);
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
					SaveSettings();
					DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_SUCCESS, 0);
				}
				break;
			case MENUACTION_CTRLMETHOD:
				m_ControlMethod = !m_ControlMethod;
				CCamera::m_bUseMouse3rdPerson = !m_ControlMethod;
				DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_SUCCESS, 0);
				SaveSettings();
				break;
		}
		ProcessOnOffMenuOptions();
		if (m_nCurrScreen == MENUPAGE_KEYBOARD_CONTROLS) {
			if (changeValueBy < 1) {
				m_nSelectedContSetupColumn = CONTSETUP_PED_COLUMN;
			} else {
				m_nSelectedContSetupColumn = CONTSETUP_VEHICLE_COLUMN;
			}
			DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_DENIED, 0);
		}
	}
}

void
CMenuManager::ProcessOnOffMenuOptions()
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
		PickNewPlayerColour();
		DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_SUCCESS, 0);
		SaveSettings();
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

void
CMenuManager::RequestFrontEndShutDown()
{
	m_bShutDownFrontEndRequested = true;
	DMAudio.ChangeMusicMode(MUSICMODE_GAME);
}

void
CMenuManager::RequestFrontEndStartUp()
{
	m_bStartUpFrontEndRequested = true;
}

void
CMenuManager::ResetHelperText() 
{
	m_nHelperTextMsgId = 0;
	m_nHelperTextAlpha = 300;
}

void
CMenuManager::SaveLoadFileError_SetUpErrorScreen()
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

void
CMenuManager::SetHelperText(int text)
{
	m_nHelperTextMsgId = text;
	m_nHelperTextAlpha = 300;
}

void
CMenuManager::ShutdownJustMenu()
{
	m_bMenuActive = false;
	CTimer::EndUserPause();
}

float
CMenuManager::StretchX(float x)
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

void
CMenuManager::SwitchMenuOnAndOff()
{
	bool menuWasActive = GetIsMenuActive();

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
			DisplayComboButtonErrMsg = false;

#ifdef REGISTER_START_BUTTON
			int16 start1 = CPad::GetPad(0)->PCTempJoyState.Start, start2 = CPad::GetPad(0)->PCTempKeyState.Start,
				start3 = CPad::GetPad(0)->OldState.Start, start4 = CPad::GetPad(0)->NewState.Start;
#endif
			CPad::GetPad(0)->Clear(false);
			CPad::GetPad(1)->Clear(false);
#ifdef REGISTER_START_BUTTON
			CPad::GetPad(0)->PCTempJoyState.Start = start1;
			CPad::GetPad(0)->PCTempKeyState.Start = start2;
			CPad::GetPad(0)->OldState.Start = start3;
			CPad::GetPad(0)->NewState.Start = start4;
#endif
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

void
CMenuManager::UnloadTextures()
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
#ifdef MENU_MAP
	for (int i = 0; i < ARRAY_SIZE(MapFilenames); ++i)
		m_aMapSprites[i].Delete();
#endif
	int menu = CTxdStore::FindTxdSlot("menu");
	CTxdStore::RemoveTxd(menu);

	m_bSpritesLoaded = false;
}

void
CMenuManager::WaitForUserCD()
{
	CSprite2d *splash;
	char *splashscreen = nil;

#ifndef RANDOMSPLASH
	if (CGame::frenchGame || CGame::germanGame || !CGame::nastyGame)
		splashscreen = "mainsc2";
	else
		splashscreen = "mainsc1";
#endif

	splash = LoadSplash(splashscreen);

	if (RsGlobal.quit)
		return;

	HandleExit();
	CPad::UpdatePads();
	MessageScreen("NO_PCCD");

	if (CPad::GetPad(0)->GetEscapeJustDown()) {
		m_bQuitGameNoCD = true;
		RsEventHandler(rsQUITAPP, nil);
	}
}

void
CMenuManager::PrintController(void)
{
	// FIX: Originally this function doesn't have StretchX/Y, everything had constant pixel size (due to screen was abandoned early?)
	//		Also texts and their alignment were very bad, so I tried to make them readable (commented out the original code, and marked the ones I added with X)

	m_aFrontEndSprites[FE_CONTROLLERSH].Draw(MENU_X_LEFT_ALIGNED(160.0f), MENU_Y(160.0f), MENU_X(240.0f), MENU_Y(180.0f), CRGBA(0, 0, 0, 255));
	m_aFrontEndSprites[FE_CONTROLLER].Draw(MENU_X_LEFT_ALIGNED(160.0f), MENU_Y(160.0f), MENU_X(235.2f), MENU_Y(175.2f), CRGBA(255, 255, 255, 255));
	if (m_DisplayControllerOnFoot) {
		if (CTimer::GetTimeInMillisecondsPauseMode() & 0x400)
			m_aFrontEndSprites[FE_ARROWS1].Draw(MENU_X_LEFT_ALIGNED(160.0f), MENU_Y(160.0f), MENU_X(235.2f), MENU_Y(175.2f), CRGBA(255, 255, 255, 255));
		else
			m_aFrontEndSprites[FE_ARROWS3].Draw(MENU_X_LEFT_ALIGNED(160.0f), MENU_Y(160.0f), MENU_X(235.2f), MENU_Y(175.2f), CRGBA(255, 255, 255, 255));
	} else {
		if (CTimer::GetTimeInMillisecondsPauseMode() & 0x400)
			m_aFrontEndSprites[FE_ARROWS2].Draw(MENU_X_LEFT_ALIGNED(160.0f), MENU_Y(160.0f), MENU_X(235.2f), MENU_Y(175.2f), CRGBA(255, 255, 255, 255));
		else
			m_aFrontEndSprites[FE_ARROWS4].Draw(MENU_X_LEFT_ALIGNED(160.0f), MENU_Y(160.0f), MENU_X(235.2f), MENU_Y(175.2f), CRGBA(255, 255, 255, 255));
	}

	CFont::SetFontStyle(FONT_BANK);  // X

	// CFont::SetScale(0.4f, 0.4f);
	CFont::SetScale(MENU_X(SMALLESTTEXT_X_SCALE), MENU_Y(SMALLESTTEXT_Y_SCALE)); // X

	// CFont::SetColor(CRGBA(128, 128, 128, FadeIn(255)));
	CFont::SetDropColor(CRGBA(0, 0, 0, FadeIn(255))); // X
	CFont::SetDropShadowPosition(1); // X
	CFont::SetColor(CRGBA(255, 255, 255, FadeIn(255))); // X

	if (m_DisplayControllerOnFoot) {
		switch (CPad::GetPad(0)->Mode) {
			case 0:
				CFont::SetRightJustifyOn(); // X
				CFont::PrintString(MENU_X_LEFT_ALIGNED(210.0f), MENU_Y(146.0f), TheText.Get("FEC_CWL"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(156.0f), MENU_Y(185.0f), TheText.Get("FEC_LOF"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(156.0f), MENU_Y(225.0f), TheText.Get("FEC_MOV"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(156.0f), MENU_Y(257.0f), TheText.Get("FEC_MOV"));
				CFont::SetJustifyOn(); // X
				CFont::PrintString(MENU_X_LEFT_ALIGNED(263.0f), MENU_Y(301.0f), TheText.Get("FEC_CAM"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(290.0f), MENU_Y(288.0f), TheText.Get("FEC_PAU"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(344.0f), MENU_Y(146.0f), TheText.Get("FEC_CWR"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(398.0f), MENU_Y(185.0f), TheText.Get("FEC_TAR"));
				CFont::SetRightJustifyOn(); // X
				CFont::PrintString(MENU_X_LEFT_ALIGNED(304.0f), MENU_Y(178.0f), TheText.Get("FEC_JUM"));
				CFont::SetJustifyOn(); // X
				CFont::PrintString(MENU_X_LEFT_ALIGNED(398.0f), MENU_Y(212.0f), TheText.Get("FEC_ENV"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(398.0f), MENU_Y(225.0f), TheText.Get("FEC_ATT"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(398.0f), MENU_Y(238.0f), TheText.Get("FEC_RUN"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(398.0f), MENU_Y(254.0f), TheText.Get("FEC_FPC"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(398.0f), MENU_Y(269.0f), TheText.Get("FEC_LB3"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(398.0f), MENU_Y(282.0f), TheText.Get("FEC_R3"));
				break;
			case 1:
				CFont::SetRightJustifyOn(); // X
				CFont::PrintString(MENU_X_LEFT_ALIGNED(210.0f), MENU_Y(146.0f), TheText.Get("FEC_CWL"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(156.0f), MENU_Y(185.0f), TheText.Get("FEC_LOF"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(156.0f), MENU_Y(225.0f), TheText.Get("FEC_CAM"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(156.0f), MENU_Y(257.0f), TheText.Get("FEC_MOV"));
				CFont::SetJustifyOn(); // X
				CFont::PrintString(MENU_X_LEFT_ALIGNED(263.0f), MENU_Y(301.0f), TheText.Get("FEC_NA"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(290.0f), MENU_Y(288.0f), TheText.Get("FEC_PAU"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(344.0f), MENU_Y(146.0f), TheText.Get("FEC_CWR"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(398.0f), MENU_Y(185.0f), TheText.Get("FEC_TAR"));
				CFont::SetRightJustifyOn(); // X
				CFont::PrintString(MENU_X_LEFT_ALIGNED(304.0f), MENU_Y(178.0f), TheText.Get("FEC_JUM"));
				CFont::SetJustifyOn(); // X
				CFont::PrintString(MENU_X_LEFT_ALIGNED(398.0f), MENU_Y(212.0f), TheText.Get("FEC_ENV"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(398.0f), MENU_Y(225.0f), TheText.Get("FEC_ATT"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(398.0f), MENU_Y(238.0f), TheText.Get("FEC_RUN"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(398.0f), MENU_Y(254.0f), TheText.Get("FEC_FPC"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(398.0f), MENU_Y(269.0f), TheText.Get("FEC_LB3"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(398.0f), MENU_Y(282.0f), TheText.Get("FEC_R3"));
				break;
			case 2:
				CFont::SetRightJustifyOn(); // X
				CFont::PrintString(MENU_X_LEFT_ALIGNED(210.0f), MENU_Y(146.0f), TheText.Get("FEC_CWL"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(156.0f), MENU_Y(185.0f), TheText.Get("FEC_ENV"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(156.0f), MENU_Y(225.0f), TheText.Get("FEC_MOV"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(156.0f), MENU_Y(257.0f), TheText.Get("FEC_MOV"));
				CFont::SetJustifyOn(); // X
				CFont::PrintString(MENU_X_LEFT_ALIGNED(263.0f), MENU_Y(301.0f), TheText.Get("FEC_CAM"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(290.0f), MENU_Y(288.0f), TheText.Get("FEC_PAU"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(344.0f), MENU_Y(146.0f), TheText.Get("FEC_CWR"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(398.0f), MENU_Y(185.0f), TheText.Get("FEC_TAR"));
				CFont::SetRightJustifyOn(); // X
				CFont::PrintString(MENU_X_LEFT_ALIGNED(304.0f), MENU_Y(178.0f), TheText.Get("FEC_JUM"));
				CFont::SetJustifyOn(); // X
				CFont::PrintString(MENU_X_LEFT_ALIGNED(398.0f), MENU_Y(212.0f), TheText.Get("FEC_LOF"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(398.0f), MENU_Y(225.0f), TheText.Get("FEC_RUN"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(398.0f), MENU_Y(238.0f), TheText.Get("FEC_ATT"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(398.0f), MENU_Y(254.0f), TheText.Get("FEC_FPC"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(398.0f), MENU_Y(269.0f), TheText.Get("FEC_LB3"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(398.0f), MENU_Y(282.0f), TheText.Get("FEC_R3"));
				break;
			case 3:
				CFont::SetRightJustifyOn(); // X
				CFont::PrintString(MENU_X_LEFT_ALIGNED(210.0f), MENU_Y(146.0f), TheText.Get("FEC_CWL"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(156.0f), MENU_Y(185.0f), TheText.Get("FEC_TAR"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(156.0f), MENU_Y(225.0f), TheText.Get("FEC_NA"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(156.0f), MENU_Y(257.0f), TheText.Get("FEC_MOV"));
				CFont::SetJustifyOn(); // X
				CFont::PrintString(MENU_X_LEFT_ALIGNED(263.0f), MENU_Y(301.0f), TheText.Get("FEC_CAM"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(290.0f), MENU_Y(288.0f), TheText.Get("FEC_PAU"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(344.0f), MENU_Y(146.0f), TheText.Get("FEC_CWR"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(398.0f), MENU_Y(185.0f), TheText.Get("FEC_TAR"));
				CFont::SetRightJustifyOn(); // X
				CFont::PrintString(MENU_X_LEFT_ALIGNED(304.0f), MENU_Y(178.0f), TheText.Get("FEC_JUM"));
				CFont::SetJustifyOn(); // X
				CFont::PrintString(MENU_X_LEFT_ALIGNED(398.0f), MENU_Y(212.0f), TheText.Get("FEC_LOF"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(398.0f), MENU_Y(225.0f), TheText.Get("FEC_RUN"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(398.0f), MENU_Y(238.0f), TheText.Get("FEC_ATT"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(398.0f), MENU_Y(254.0f), TheText.Get("FEC_FPC"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(398.0f), MENU_Y(269.0f), TheText.Get("FEC_LB3"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(398.0f), MENU_Y(282.0f), TheText.Get("FEC_R3"));
				break;
			default:
				return;
		}
	} else {
		switch (CPad::GetPad(0)->Mode) {
			case 0:
				CFont::SetRightJustifyOn(); // X
				CFont::PrintString(MENU_X_LEFT_ALIGNED(210.0f), MENU_Y(146.0f), TheText.Get("FEC_LL"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(156.0f), MENU_Y(185.0f), TheText.Get("FEC_RSC"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(156.0f), MENU_Y(225.0f), TheText.Get("FEC_VES"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(156.0f), MENU_Y(257.0f), TheText.Get("FEC_VES"));
				CFont::SetJustifyOn(); // X
				CFont::PrintString(MENU_X_LEFT_ALIGNED(263.0f), MENU_Y(301.0f), TheText.Get("FEC_HO3"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(290.0f), MENU_Y(288.0f), TheText.Get("FEC_CAM"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(344.0f), MENU_Y(146.0f), TheText.Get("FEC_PAU"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(398.0f), MENU_Y(185.0f), TheText.Get("FEC_LB"));
				CFont::SetRightJustifyOn(); // X
				CFont::PrintString(MENU_X_LEFT_ALIGNED(304.0f), MENU_Y(178.0f), TheText.Get("FEC_LR"));
				CFont::SetJustifyOn(); // X
				CFont::PrintString(MENU_X_LEFT_ALIGNED(398.0f), MENU_Y(212.0f), TheText.Get("FEC_HAB"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(398.0f), MENU_Y(225.0f), TheText.Get("FEC_BRA"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(398.0f), MENU_Y(238.0f), TheText.Get("FEC_EXV"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(398.0f), MENU_Y(254.0f), TheText.Get("FEC_CAW"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(398.0f), MENU_Y(269.0f), TheText.Get("FEC_ACC"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(398.0f), MENU_Y(282.0f), TheText.Get("FEC_TUC"));
				// FIX: Coordinates of this line is undefined in PC...
				CFont::PrintString(MENU_X_LEFT_ALIGNED(398.0f), MENU_Y(304.0f), TheText.Get("FEC_SM3"));
				break;
			case 1:
				CFont::SetRightJustifyOn(); // X
				CFont::PrintString(MENU_X_LEFT_ALIGNED(210.0f), MENU_Y(146.0f), TheText.Get("FEC_LL"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(156.0f), MENU_Y(185.0f), TheText.Get("FEC_HOR"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(156.0f), MENU_Y(225.0f), TheText.Get("FEC_CAM"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(156.0f), MENU_Y(257.0f), TheText.Get("FEC_VES"));
				CFont::SetJustifyOn(); // X
				CFont::PrintString(MENU_X_LEFT_ALIGNED(263.0f), MENU_Y(301.0f), TheText.Get("FEC_NA"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(290.0f), MENU_Y(288.0f), TheText.Get("FEC_RSC"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(344.0f), MENU_Y(146.0f), TheText.Get("FEC_PAU"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(398.0f), MENU_Y(185.0f), TheText.Get("FEC_LB"));
				CFont::SetRightJustifyOn(); // X
				CFont::PrintString(MENU_X_LEFT_ALIGNED(304.0f), MENU_Y(178.0f), TheText.Get("FEC_LR"));
				CFont::SetJustifyOn(); // X
				CFont::PrintString(MENU_X_LEFT_ALIGNED(398.0f), MENU_Y(212.0f), TheText.Get("FEC_HAB"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(398.0f), MENU_Y(225.0f), TheText.Get("FEC_BRA"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(398.0f), MENU_Y(238.0f), TheText.Get("FEC_EXV"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(398.0f), MENU_Y(254.0f), TheText.Get("FEC_CAW"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(398.0f), MENU_Y(269.0f), TheText.Get("FEC_ACC"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(398.0f), MENU_Y(282.0f), TheText.Get("FEC_TUC"));
				// FIX: Coordinates of this line is undefined in PC...
				CFont::PrintString(MENU_X_LEFT_ALIGNED(398.0f), MENU_Y(304.0f), TheText.Get("FEC_SM3"));
				break;
			case 2:
				CFont::SetRightJustifyOn(); // X
				CFont::PrintString(MENU_X_LEFT_ALIGNED(210.0f), MENU_Y(146.0f), TheText.Get("FEC_LL"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(156.0f), MENU_Y(185.0f), TheText.Get("FEC_EXV"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(156.0f), MENU_Y(225.0f), TheText.Get("FEC_VES"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(156.0f), MENU_Y(257.0f), TheText.Get("FEC_VES"));
				CFont::SetJustifyOn(); // X
				CFont::PrintString(MENU_X_LEFT_ALIGNED(263.0f), MENU_Y(301.0f), TheText.Get("FEC_RS3"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(290.0f), MENU_Y(288.0f), TheText.Get("FEC_CAM"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(344.0f), MENU_Y(146.0f), TheText.Get("FEC_PAU"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(398.0f), MENU_Y(185.0f), TheText.Get("FEC_LB"));
				CFont::SetRightJustifyOn(); // X
				CFont::PrintString(MENU_X_LEFT_ALIGNED(304.0f), MENU_Y(178.0f), TheText.Get("FEC_LR"));
				CFont::SetJustifyOn(); // X
				CFont::PrintString(MENU_X_LEFT_ALIGNED(398.0f), MENU_Y(212.0f), TheText.Get("FEC_HOR"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(398.0f), MENU_Y(225.0f), TheText.Get("FEC_BRA"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(398.0f), MENU_Y(238.0f), TheText.Get("FEC_HAB"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(398.0f), MENU_Y(254.0f), TheText.Get("FEC_CAW"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(398.0f), MENU_Y(269.0f), TheText.Get("FEC_ACC"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(398.0f), MENU_Y(282.0f), TheText.Get("FEC_TUC"));
				// FIX: Coordinates of this line is undefined in PC...
				CFont::PrintString(MENU_X_LEFT_ALIGNED(398.0f), MENU_Y(304.0f), TheText.Get("FEC_SM3"));
				break;
			case 3:
				CFont::SetRightJustifyOn(); // X
				CFont::PrintString(MENU_X_LEFT_ALIGNED(210.0f), MENU_Y(146.0f), TheText.Get("FEC_LL"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(156.0f), MENU_Y(185.0f), TheText.Get("FEC_HAB"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(156.0f), MENU_Y(225.0f), TheText.Get("FEC_TUC"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(156.0f), MENU_Y(257.0f), TheText.Get("FEC_VES"));
				CFont::SetJustifyOn(); // X
				CFont::PrintString(MENU_X_LEFT_ALIGNED(263.0f), MENU_Y(301.0f), TheText.Get("FEC_HO3"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(290.0f), MENU_Y(288.0f), TheText.Get("FEC_CAM"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(344.0f), MENU_Y(146.0f), TheText.Get("FEC_PAU"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(398.0f), MENU_Y(185.0f), TheText.Get("FEC_LB"));
				CFont::SetRightJustifyOn(); // X
				CFont::PrintString(MENU_X_LEFT_ALIGNED(304.0f), MENU_Y(178.0f), TheText.Get("FEC_LR"));
				CFont::SetJustifyOn(); // X
				CFont::PrintString(MENU_X_LEFT_ALIGNED(398.0f), MENU_Y(212.0f), TheText.Get("FEC_CAW"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(398.0f), MENU_Y(225.0f), TheText.Get("FEC_SMT"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(398.0f), MENU_Y(238.0f), TheText.Get("FEC_EXV"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(398.0f), MENU_Y(254.0f), TheText.Get("FEC_RSC"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(398.0f), MENU_Y(269.0f), TheText.Get("FEC_NA"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(398.0f), MENU_Y(282.0f), TheText.Get("FEC_ACC"));
				// FIX: Coordinates of this line is undefined in PC...
				CFont::PrintString(MENU_X_LEFT_ALIGNED(398.0f), MENU_Y(304.0f), TheText.Get("FEC_BRA"));
				break;
			default:
				return;
		}
	}

	CFont::SetDropShadowPosition(0); // X
}

#ifdef MENU_MAP

#define ZOOM(x, y, in) \
	do { \
		if(fMapSize > SCREEN_WIDTH * 2 && in) \
			break; \
		float z2 = in? 1.1f : 1.f/1.1f; \
		fMapCenterX += (x - fMapCenterX) * (1.0f - z2); \
		fMapCenterY += (y - fMapCenterY) * (1.0f - z2); \
		\
		if (fMapSize < SCREEN_WIDTH / 3 && !in) \
			break; \
		\
		fMapSize *= z2; \
	} while(0) \

void
CMenuManager::PrintMap(void)
{
	bMenuMapActive = true;
	CRadar::InitFrontEndMap();

	if (!bMapLoaded) {
		fMapCenterX = SCREEN_WIDTH / 2;
		fMapCenterY = SCREEN_HEIGHT / 3;
		fMapSize = SCREEN_HEIGHT / CDraw::GetAspectRatio();
		bMapMouseShownOnce = false;
		bMapLoaded = true;

		// Let's wait for a frame to not toggle the waypoint
		if (CPad::GetPad(0)->NewState.Cross) {
			bMenuMapActive = false;
			return;
		}
	}

	// Because fMapSize is half of the map length, and map consists of 3x3 tiles.
	float halfTile = fMapSize / 3.0f;

	// Darken background a bit
	CSprite2d::DrawRect(CRect(0, 0,
		SCREEN_WIDTH, SCREEN_HEIGHT),
		CRGBA(0, 0, 0, FadeIn(128)));

	RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void*)rwFILTERLINEAR);

	if (SCREEN_WIDTH >= fMapCenterX - fMapSize || SCREEN_HEIGHT >= fMapCenterY - fMapSize) {
		m_aMapSprites[MAPTOP1].Draw(CRect(fMapCenterX - fMapSize, fMapCenterY - fMapSize,
			fMapCenterX - halfTile, fMapCenterY - halfTile), CRGBA(255, 255, 255, FadeIn(255)));
	}

	if (SCREEN_WIDTH >= fMapCenterX - halfTile || SCREEN_HEIGHT >= fMapCenterY - fMapSize) {
		m_aMapSprites[MAPTOP2].Draw(CRect(fMapCenterX - halfTile, fMapCenterY - fMapSize,
			fMapCenterX + halfTile, fMapCenterY - halfTile), CRGBA(255, 255, 255, FadeIn(255)));
	}

	if (SCREEN_WIDTH >= fMapCenterX + halfTile || SCREEN_HEIGHT >= fMapCenterY - fMapSize) {
		m_aMapSprites[MAPTOP3].Draw(CRect(fMapCenterX + halfTile, fMapCenterY - fMapSize,
			fMapCenterX + fMapSize, fMapCenterY - halfTile), CRGBA(255, 255, 255, FadeIn(255)));
	}

	if (SCREEN_WIDTH >= fMapCenterX - fMapSize || SCREEN_HEIGHT >= fMapCenterY - halfTile) {
		m_aMapSprites[MAPMID1].Draw(CRect(fMapCenterX - fMapSize, fMapCenterY - halfTile,
			fMapCenterX - halfTile, fMapCenterY + halfTile), CRGBA(255, 255, 255, FadeIn(255)));
	}

	if (SCREEN_WIDTH >= fMapCenterX - halfTile || SCREEN_HEIGHT >= fMapCenterY - halfTile) {
		m_aMapSprites[MAPMID2].Draw(CRect(fMapCenterX - halfTile, fMapCenterY - halfTile,
			fMapCenterX + halfTile, fMapCenterY + halfTile), CRGBA(255, 255, 255, FadeIn(255)));
	}

	if (SCREEN_WIDTH >= fMapCenterX + halfTile || SCREEN_HEIGHT >= fMapCenterY - halfTile) {
		m_aMapSprites[MAPMID3].Draw(CRect(fMapCenterX + halfTile, fMapCenterY - halfTile,
			fMapCenterX + fMapSize, fMapCenterY + halfTile), CRGBA(255, 255, 255, FadeIn(255)));
	}

	if (SCREEN_WIDTH >= fMapCenterX - fMapSize || SCREEN_HEIGHT >= fMapCenterY + halfTile) {
		m_aMapSprites[MAPBOT1].Draw(CRect(fMapCenterX - fMapSize, fMapCenterY + halfTile,
			fMapCenterX - halfTile, fMapCenterY + fMapSize), CRGBA(255, 255, 255, FadeIn(255)));
	}

	if (SCREEN_WIDTH >= fMapCenterX - halfTile || SCREEN_HEIGHT >= fMapCenterY + halfTile) {
		m_aMapSprites[MAPBOT2].Draw(CRect(fMapCenterX - halfTile, fMapCenterY + halfTile,
			fMapCenterX + halfTile, fMapCenterY + fMapSize), CRGBA(255, 255, 255, FadeIn(255)));
	}

	if (SCREEN_WIDTH >= fMapCenterX + halfTile || SCREEN_HEIGHT >= fMapCenterY + halfTile) {
		m_aMapSprites[MAPBOT3].Draw(CRect(fMapCenterX + halfTile, fMapCenterY + halfTile,
			fMapCenterX + fMapSize, fMapCenterY + fMapSize), CRGBA(255, 255, 255, FadeIn(255)));
	}

	CRadar::DrawBlips();

	CVector2D mapPoint;
	mapPoint.x = m_nMousePosX;
	mapPoint.y = m_nMousePosY;

	if (m_bShowMouse) {
		bMapMouseShownOnce = true;
	} else if (!bMapMouseShownOnce) {
		mapPoint.x = SCREEN_WIDTH / 2;
		mapPoint.y = SCREEN_HEIGHT / 2;
	}

	CSprite2d::DrawRect(CRect(mapPoint.x - MENU_X(1.0f), 0.0f,
		mapPoint.x + MENU_X(1.0f), SCREEN_HEIGHT),
		CRGBA(0, 0, 0, 150));
	CSprite2d::DrawRect(CRect(0.0f, mapPoint.y + MENU_X(1.0f),
		SCREEN_WIDTH, mapPoint.y - MENU_X(1.0f)),
		CRGBA(0, 0, 0, 150));

	if (CPad::GetPad(0)->GetRightMouseJustDown() || CPad::GetPad(0)->GetCrossJustDown()) {
		if (mapPoint.y > fMapCenterY - fMapSize && mapPoint.y < fMapCenterY + fMapSize &&
			mapPoint.x > fMapCenterX - fMapSize && mapPoint.x < fMapCenterX + fMapSize) {

			float diffX = fMapCenterX - fMapSize, diffY = fMapCenterY - fMapSize;
			float x = ((mapPoint.x - diffX) / (fMapSize * 2)) * 4000.0f - 2000.0f;
			float y = 2000.0f - ((mapPoint.y - diffY) / (fMapSize * 2)) * 4000.0f;
			CRadar::ToggleTargetMarker(x, y);
			DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_SUCCESS, 0);
		}
	}

	if (CPad::GetPad(0)->GetLeftMouse()) {
		fMapCenterX += m_nMousePosX - m_nMouseOldPosX;
		fMapCenterY += m_nMousePosY - m_nMouseOldPosY;
	} else if (CPad::GetPad(0)->GetLeft() || CPad::GetPad(0)->GetDPadLeft()) {
		fMapCenterX += 15.0f;
	} else if (CPad::GetPad(0)->GetRight() || CPad::GetPad(0)->GetDPadRight()) {
		fMapCenterX -= 15.0f;
	} else if (CPad::GetPad(0)->GetLeftStickX()) {
		fMapCenterX -= CPad::GetPad(0)->GetLeftStickX() / 128.0f * 20.0f;
	}

	if (CPad::GetPad(0)->GetUp() || CPad::GetPad(0)->GetDPadUp()) {
		fMapCenterY += 15.0f;
	} else if (CPad::GetPad(0)->GetDown() || CPad::GetPad(0)->GetDPadDown()) {
		fMapCenterY -= 15.0f;
	} else if (CPad::GetPad(0)->GetLeftStickY()) {
		fMapCenterY -= CPad::GetPad(0)->GetLeftStickY() / 128.0f * 20.0f;
	}

	if (CPad::GetPad(0)->GetMouseWheelDown() || CPad::GetPad(0)->GetPageDown() || CPad::GetPad(0)->GetRightShoulder2()) {
		if (CPad::GetPad(0)->GetMouseWheelDown())
			ZOOM(mapPoint.x, mapPoint.y, false);
		else
			ZOOM(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, false);
	} else if (CPad::GetPad(0)->GetMouseWheelUp() || CPad::GetPad(0)->GetPageUp() || CPad::GetPad(0)->GetRightShoulder1()) {
		if (CPad::GetPad(0)->GetMouseWheelUp())
			ZOOM(mapPoint.x, mapPoint.y, true);
		else
			ZOOM(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, true);
	}
	
	if (fMapCenterX - fMapSize > SCREEN_WIDTH / 2)
		fMapCenterX = fMapSize + SCREEN_WIDTH / 2;

	if (fMapCenterX + fMapSize < SCREEN_WIDTH / 2)
		fMapCenterX = SCREEN_WIDTH / 2 - fMapSize;

	if (fMapCenterY + fMapSize < SCREEN_HEIGHT - MENU_Y(60.0f))
		fMapCenterY = SCREEN_HEIGHT - MENU_Y(60.0f) - fMapSize;
	
	fMapCenterY = min(fMapCenterY, fMapSize); // To not show beyond north border

	bMenuMapActive = false;

	// CFont::SetWrapx(MENU_X_RIGHT_ALIGNED(5.0f)); // From VC
	// CFont::SetRightJustifyWrap(10.0f);

	CSprite2d::DrawRect(CRect(MENU_X(14.0f), SCREEN_STRETCH_FROM_BOTTOM(95.0f),
		SCREEN_STRETCH_FROM_RIGHT(11.0f), SCREEN_STRETCH_FROM_BOTTOM(59.0f)),
		CRGBA(235, 170, 50, 255));

	CFont::SetScale(MENU_X(0.4f), MENU_Y(0.7f));
	CFont::SetFontStyle(FONT_BANK);
	CFont::SetColor(CRGBA(0, 0, 0, FadeIn(255)));

	float nextX = MENU_X(30.0f), nextY = 95.0f;
	wchar *text;
#define TEXT_PIECE(key,extraSpace) \
	text = TheText.Get(key); CFont::PrintString(nextX, SCREEN_SCALE_FROM_BOTTOM(nextY), text); nextX += CFont::GetStringWidth(text, true) + MENU_X(extraSpace);

	TEXT_PIECE("FEC_MWF", 3.0f);
	TEXT_PIECE("FEC_PGU", 1.0f);
	TEXT_PIECE("FEC_IBT", 1.0f);
	TEXT_PIECE("FEC_ZIN", 20.0f);
	TEXT_PIECE("FEC_MWB", 3.0f);
	TEXT_PIECE("FEC_PGD", 1.0f);
	TEXT_PIECE("FEC_IBT", 1.0f);
	CFont::PrintString(nextX, SCREEN_SCALE_FROM_BOTTOM(nextY), TheText.Get("FEC_ZOT")); nextX = MENU_X(30.0f); nextY -= 11.0f;
	TEXT_PIECE("FEC_UPA", 2.0f);
	TEXT_PIECE("FEC_DWA", 2.0f);
	TEXT_PIECE("FEC_LFA", 2.0f);
	TEXT_PIECE("FEC_RFA", 2.0f);
	TEXT_PIECE("FEC_MSL", 1.0f);
	TEXT_PIECE("FEC_IBT", 1.0f);
	CFont::PrintString(nextX, SCREEN_SCALE_FROM_BOTTOM(nextY), TheText.Get("FEC_MOV")); nextX = MENU_X(30.0f); nextY -= 11.0f;
	TEXT_PIECE("FEC_MSR", 2.0f);
	TEXT_PIECE("FEC_IBT", 1.0f);
	CFont::PrintString(nextX, SCREEN_SCALE_FROM_BOTTOM(nextY), TheText.Get("FEC_TAR"));
#undef TEXT_PIECE
}

#undef ZOOM
#endif

// rowIdx 99999 returns total numbers of rows. otherwise it returns 0.
int
CMenuManager::ConstructStatLine(int rowIdx)
{
#define STAT_LINE(str, left, isFloat, right) \
	do { \
		if(counter == rowIdx){ \
			BuildStatLine(str, left, isFloat, right); \
			return 0; \
		} counter++; \
	} while(0)

	int counter = 0, nTemp;

	STAT_LINE("PL_STAT", nil, false, nil);

	int percentCompleted = (CStats::TotalProgressInGame == 0 ? 0 :
		CStats::ProgressMade * 100.0f / (CGame::nastyGame ? CStats::TotalProgressInGame : CStats::TotalProgressInGame - 1));
	percentCompleted = min(percentCompleted, 100);

	STAT_LINE("PER_COM", &percentCompleted, false, nil);
	STAT_LINE("NMISON", &CStats::MissionsGiven, false, nil);
	STAT_LINE("FEST_MP", &CStats::MissionsPassed, 0, &CStats::TotalNumberMissions);
	if (CGame::nastyGame) {
		STAT_LINE("FEST_RP", &CStats::NumberKillFrenziesPassed, 0, &CStats::TotalNumberKillFrenzies);
	}
	
	CPlayerInfo &player = CWorld::Players[CWorld::PlayerInFocus];
	float packagesPercent = 0.0f;
	if (player.m_nTotalPackages != 0)
		packagesPercent = player.m_nCollectedPackages * 100.0f / player.m_nTotalPackages;

	STAT_LINE("PERPIC", &packagesPercent, 0, &(nTemp = 100));
	STAT_LINE("NOUNIF", &CStats::TotalNumberOfUniqueJumps, 0, &CStats::NumberOfUniqueJumpsFound);
	STAT_LINE("DAYSPS", &CStats::DaysPassed, false, nil);
	if (CGame::nastyGame) {
		STAT_LINE("PE_WAST", &CStats::PeopleKilledByPlayer, false, nil);
		STAT_LINE("PE_WSOT", &CStats::PeopleKilledByOthers, false, nil);
	}
	STAT_LINE("CAR_EXP", &CStats::CarsExploded, false, nil);
	STAT_LINE("TM_BUST", &CStats::TimesArrested, false, nil);
	STAT_LINE("TM_DED", &CStats::TimesDied, false, nil);
	STAT_LINE("GNG_WST", &(nTemp = CStats::PedsKilledOfThisType[PEDTYPE_GANG9] + CStats::PedsKilledOfThisType[PEDTYPE_GANG8]
			+ CStats::PedsKilledOfThisType[PEDTYPE_GANG7] + CStats::PedsKilledOfThisType[PEDTYPE_GANG6]
			+ CStats::PedsKilledOfThisType[PEDTYPE_GANG5] + CStats::PedsKilledOfThisType[PEDTYPE_GANG4]
			+ CStats::PedsKilledOfThisType[PEDTYPE_GANG3] + CStats::PedsKilledOfThisType[PEDTYPE_GANG2]
			+ CStats::PedsKilledOfThisType[PEDTYPE_GANG1]), false, nil);
	STAT_LINE("DED_CRI", &(nTemp = CStats::PedsKilledOfThisType[PEDTYPE_CRIMINAL]), false, nil);
	STAT_LINE("HEL_DST", &CStats::HelisDestroyed, false, nil);
	STAT_LINE("KGS_EXP", &CStats::KgsOfExplosivesUsed, false, nil);
	STAT_LINE("ACCURA", &(nTemp = (CStats::InstantHitsFiredByPlayer == 0 ? 0 :
			CStats::InstantHitsHitByPlayer * 100.0f / CStats::InstantHitsFiredByPlayer)), false, nil);
	
	if (CStats::ElBurroTime > 0) {
		STAT_LINE("ELBURRO", &CStats::ElBurroTime, false, nil);
	}
	if (CStats::Record4x4One > 0) {
		STAT_LINE("FEST_R1", &CStats::Record4x4One, false, nil);
	}
	if (CStats::Record4x4Two > 0) {
		STAT_LINE("FEST_R2", &CStats::Record4x4Two, false, nil);
	}
	if (CStats::Record4x4Three > 0) {
		STAT_LINE("FEST_R3", &CStats::Record4x4Three, false, nil);
	}
	if (CStats::Record4x4Mayhem > 0) {
		STAT_LINE("FEST_RM", &CStats::Record4x4Mayhem, false, nil);
	}
	if (CStats::LongestFlightInDodo > 0) {
		STAT_LINE("FEST_LF", &CStats::LongestFlightInDodo, false, nil);
	}
	if (CStats::TimeTakenDefuseMission > 0) {
		STAT_LINE("FEST_BD", &CStats::TimeTakenDefuseMission, false, nil);
	}
	STAT_LINE("CAR_CRU", &CStats::CarsCrushed, false, nil);

	if (CStats::HighestScores[0] > 0) {
		STAT_LINE("FEST_BB", nil, false, nil);
		STAT_LINE("FEST_H0", &CStats::HighestScores[0], false, nil);
	}
	if (CStats::HighestScores[4] + CStats::HighestScores[3] + CStats::HighestScores[2] + CStats::HighestScores[1] > 0) {
		STAT_LINE("FEST_GC", nil, false, nil);
	}
	if (CStats::HighestScores[1] > 0) {
		STAT_LINE("FEST_H1", &CStats::HighestScores[1], false, nil);
	}
	if (CStats::HighestScores[2] > 0) {
		STAT_LINE("FEST_H2", &CStats::HighestScores[2], false, nil);
	}
	if (CStats::HighestScores[3] > 0) {
		STAT_LINE("FEST_H3", &CStats::HighestScores[3], false, nil);
	}
	if (CStats::HighestScores[4] > 0) {
		STAT_LINE("FEST_H4", &CStats::HighestScores[4], false, nil);
	}

	switch (m_PrefsLanguage) {
		case LANGUAGE_AMERICAN:
#ifndef USE_MEASUREMENTS_IN_METERS
			float fTemp;
			STAT_LINE("FEST_DF", &(fTemp = CStats::DistanceTravelledOnFoot * MILES_IN_METER), true, nil);
			STAT_LINE("FEST_DC", &(fTemp = CStats::DistanceTravelledInVehicle * MILES_IN_METER), true, nil);
			STAT_LINE("MMRAIN", &CStats::mmRain, false, nil);
			STAT_LINE("MXCARD", &(fTemp = CStats::MaximumJumpDistance * FEET_IN_METER), true, nil);
			STAT_LINE("MXCARJ", &(fTemp = CStats::MaximumJumpHeight * FEET_IN_METER), true, nil);
			break;
#endif
		case LANGUAGE_FRENCH:
		case LANGUAGE_GERMAN:
		case LANGUAGE_ITALIAN:
		case LANGUAGE_SPANISH:
#ifdef MORE_LANGUAGES
		case LANGUAGE_RUSSIAN:
#endif
			STAT_LINE("FESTDFM", &CStats::DistanceTravelledOnFoot, true, nil);
			STAT_LINE("FESTDCM", &CStats::DistanceTravelledInVehicle, true, nil);
			STAT_LINE("MMRAIN", &CStats::mmRain, false, nil);
			STAT_LINE("MXCARDM", &CStats::MaximumJumpDistance, true, nil);
			STAT_LINE("MXCARJM", &CStats::MaximumJumpHeight, true, nil);
			break;
		default:
			break;
	}

	STAT_LINE("MXFLIP", &CStats::MaximumJumpFlips, false, nil);
	STAT_LINE("MXJUMP", &CStats::MaximumJumpSpins, false, nil);
	STAT_LINE("BSTSTU", nil, false, nil);

	if (counter == rowIdx) {
		gUString[0] = '\0';
		switch (CStats::BestStuntJump) {
			case 1:
				UnicodeStrcpy(gUString2, TheText.Get("INSTUN"));
				return 0;
			case 2:
				UnicodeStrcpy(gUString2, TheText.Get("PRINST"));
				return 0;
			case 3:
				UnicodeStrcpy(gUString2, TheText.Get("DBINST"));
				return 0;
			case 4:
				UnicodeStrcpy(gUString2, TheText.Get("DBPINS"));
				return 0;
			case 5:
				UnicodeStrcpy(gUString2, TheText.Get("TRINST"));
				return 0;
			case 6:
				UnicodeStrcpy(gUString2, TheText.Get("PRTRST"));
				return 0;
			case 7:
				UnicodeStrcpy(gUString2, TheText.Get("QUINST"));
				return 0;
			case 8:
				UnicodeStrcpy(gUString2, TheText.Get("PQUINS"));
				return 0;
			default:
				UnicodeStrcpy(gUString2, TheText.Get("NOSTUC"));
				return 0;
		}
	}
	counter++;
	STAT_LINE("PASDRO", &CStats::PassengersDroppedOffWithTaxi, false, nil);
	STAT_LINE("MONTAX", &CStats::MoneyMadeWithTaxi, false, nil);
	STAT_LINE("FEST_LS", &CStats::LivesSavedWithAmbulance, false, nil);
	STAT_LINE("FEST_HA", &CStats::HighestLevelAmbulanceMission, false, nil);
	STAT_LINE("FEST_CC", &CStats::CriminalsCaught, false, nil);
	STAT_LINE("FEST_FE", &CStats::FiresExtinguished, false, nil);
	STAT_LINE("DAYPLC", &(nTemp = CTimer::GetTimeInMilliseconds() + 100), false, nil);
	return counter;

#undef STAT_LINE
}

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

#undef GetBackJustUp
#undef GetBackJustDown

STARTPATCHES
	for (int i = 1; i < ARRAY_SIZE(aScreens); i++)
		Patch(0x611930 + sizeof(CMenuScreen) * i, aScreens[i]);
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
	InjectHook(0x489710, &CMenuManager::DrawControllerBound, PATCH_JUMP);
ENDPATCHES