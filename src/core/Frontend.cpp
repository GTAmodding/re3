#if defined RW_D3D9 || defined RWLIBS
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#endif

#define WITHWINDOWS
#include "common.h"
#include "crossplatform.h"
#include "platform.h"
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

// TODO(Miami): This is -3 on VC but still -1 on AudioManager?!? What the hell?
#define INVALID_AUDIO_PROVIDER -1

// Similar story to Hud.cpp:
// Game has colors inlined in code.
// For easier modification we collect them here:
CRGBA LABEL_COLOR(255, 150, 225, 255);
CRGBA SELECTIONBORDER_COLOR(25, 130, 70, 255);
CRGBA MENUOPTION_COLOR(255, 150, 225, 255);
CRGBA SELECTEDMENUOPTION_COLOR(255, 150, 225, 255);
CRGBA HEADER_COLOR(255, 150, 255, 255);
CRGBA DARKMENUOPTION_COLOR(195, 90, 165, 255);
CRGBA SLIDERON_COLOR(97, 194, 247, 255);
CRGBA SLIDEROFF_COLOR(27, 89, 130, 255);
CRGBA MAPINFOBOX_COLOR(255, 150, 225, 150);

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

uint8 CMenuManager::m_PrefsStereoMono; // unused except restore settings

// Originally that was PS2 option color, they forget it here and used in PrintBriefs once(but didn't use the output anyway)
#ifdef PS2_LIKE_MENU
const CRGBA TEXT_COLOR = CRGBA(150, 110, 30, 255);
#else
const CRGBA TEXT_COLOR = CRGBA(235, 170, 50, 255); // PC briefs text color
#endif

float MENU_TEXT_SIZE_X = SMALLTEXT_X_SCALE;
float MENU_TEXT_SIZE_Y = SMALLTEXT_Y_SCALE;

bool holdingScrollBar; // *(bool*)0x7039B9; // not original name

CMenuManager FrontEndMenuManager;
MenuTrapezoid menuBg(CGeneral::GetRandomNumber() % 40 + 65, CGeneral::GetRandomNumber() % 40 + 21,
	CGeneral::GetRandomNumber() % 40 + 568, CGeneral::GetRandomNumber() % 40 + 44,
	CGeneral::GetRandomNumber() % 40 + 36, CGeneral::GetRandomNumber() % 40 + 352,
	CGeneral::GetRandomNumber() % 40 + 593, CGeneral::GetRandomNumber() % 40 + 312);

MenuTrapezoid menuOptionHighlight(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);

// TODO(Miami): TEMPORARY
bool bMapLoaded = false;
bool bMapMouseShownOnce = false;

#ifndef MASTER
bool CMenuManager::m_PrefsMarketing = false;
bool CMenuManager::m_PrefsDisableTutorials = false;
#endif // !MASTER

// 0x68C144
const char* FrontendFilenames[][2] = {
	{"background", ""},
	{"vc_logo", "vc_logom"},
	{"mouse", "mousea"},
	{"mapTop01", "mapTop01A"},
	{"mapTop02", "mapTop02A"},
	{"mapTop03", "mapTop03A"},
	{"mapMid01", "mapMid01A"},
	{"mapMid02", "mapMid02A"},
	{"mapMid03", "mapMid03A"},
	{"mapBot01", "mapBot01A"},
	{"mapBot02", "mapBot02A"},
	{"mapBot03", "mapBot03A"},
	{"wildstyle", "wildstyleA"},
	{"flash", "flashA"},
	{"kchat", "kchatA"},
	{"fever", "feverA"},
	{"vrock", "vrockA"},
	{"vcpr", "vcprA"},
	{"espantoso", "espantosoA"},
	{"emotion", "emotionA"},
	{"wave103", "wave103A"},
	{"mp3", "mp3A"},
	{"downOff", "buttonA"},
	{"downOn", "buttonA"},
	{"upOff", "buttonA"},
	{"upOn", "buttonA"}
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

#define PREPARE_MENU_HEADER \
	CFont::SetRightJustifyOn(); \
	CFont::SetFontStyle(FONT_LOCALE(FONT_HEADING)); \
	CFont::SetScale(MENU_X(MENUHEADER_WIDTH), MENU_Y(MENUHEADER_HEIGHT)); \
	CFont::SetDropShadowPosition(0);

#define ProcessSlider(value, y, increaseAction, decreaseAction, hoverStartX, hoverEndX) \
	do { \
		lastActiveBarX = DisplaySlider(MENU_X_LEFT_ALIGNED(MENUSLIDER_X), MENU_Y(y), MENU_Y(MENUSLIDER_SMALLEST_BAR), MENU_Y(MENUSLIDER_BIGGEST_BAR), MENU_X(MENUSLIDER_UNK), value, MENU_X(3.0f)); \
		if (i != m_nCurrOption || !itemsAreSelectable) \
			break; \
		 \
		if (CheckHover(hoverStartX, lastActiveBarX - MENU_X(3.0f), MENU_Y(y), MENU_Y(MENUSLIDER_BIGGEST_BAR + y))) \
			m_nHoverOption = decreaseAction; \
		 \
		if (!CheckHover(MENU_X(3.0f) + lastActiveBarX, hoverEndX, MENU_Y(y), MENU_Y(MENUSLIDER_BIGGEST_BAR + y))) \
			break; \
		 \
		m_nHoverOption = increaseAction; \
		if (m_nMousePosX < MENU_X_LEFT_ALIGNED(MENUSLIDER_X)) \
			m_nHoverOption = HOVEROPTION_NOT_HOVERING; \
	} while(0)

// TODO: this is COMPLETELY different in VC
#define ProcessRadioIcon(sprite, x, y, radioId, hoverOpt) \
	do { \
		sprite.Draw(x, y, MENU_X(MENURADIO_ICON_SCALE), MENU_Y(MENURADIO_ICON_SCALE), radioId == m_PrefsRadioStation ? CRGBA(255, 255, 255, 255) : CRGBA(255, 255, 255, 100)); \
			if (CheckHover(x, x + MENU_X(MENURADIO_ICON_SCALE), y, y + MENU_Y(MENURADIO_ICON_SCALE))) \
				m_nHoverOption = hoverOpt; \
	} while (0)

// --- Functions not in the game/inlined starts

bool DoRWStuffStartOfFrame(int16 TopRed, int16 TopGreen, int16 TopBlue, int16 BottomRed, int16 BottomGreen, int16 BottomBlue, int16 Alpha);
void DoRWStuffEndOfFrame(void);

#ifdef PS2_LIKE_MENU
void
CMenuManager::SwitchToNewScreen(int8 screen)
{
	if (reverseAlpha) {
			m_nPrevScreen = m_nCurrScreen;
			m_nCurrScreen = pendingScreen;
			m_nCurrOption = pendingOption;
			reverseAlpha = false;
			if (updateDelay)
				m_LastScreenSwitch = CTimer::GetTimeInMillisecondsPauseMode();
	}
	if (withReverseAlpha) {
		pendingOption = option;
		pendingScreen = screen;
		reverseAlpha = true;
	} else {
		m_nPrevScreen = m_nCurrScreen;
		m_nCurrScreen = screen;
		m_nCurrOption = option;
		if (updateDelay)
			m_LastScreenSwitch = CTimer::GetTimeInMillisecondsPauseMode();
	}
	m_nMenuFadeAlpha = 255;
}
#else

// --MIAMI: Done except using VC's gMenuPages
void
CMenuManager::SwitchToNewScreen(int8 screen)
{
	bMenuChangeOngoing = true;
	DoRWStuffStartOfFrame(0, 0, 0, 0, 0, 0, 255);
	DrawBackground(true);
	DoRWStuffEndOfFrame();
	DoRWStuffStartOfFrame(0, 0, 0, 0, 0, 0, 255);
	DrawBackground(true);
	DoRWStuffEndOfFrame();
	m_nPrevScreen = m_nCurrScreen;
	m_ShowEmptyBindingError = false;
	ResetHelperText();

	ThingsToDoBeforeLeavingPage();

	if (screen == -2) {
		int oldScreen = aScreens[m_nCurrScreen].m_PreviousPage;
		int oldOption = (m_nCurrScreen == MENUPAGE_NEW_GAME ? 0 : (m_nCurrScreen == MENUPAGE_OPTIONS ? 1 : (m_nCurrScreen == MENUPAGE_EXIT ? 2 : aScreens[m_nCurrScreen].m_ParentEntry)));

		m_nCurrOption = oldOption;
		m_nCurrScreen = oldScreen;
	} else if (screen == 0) {
		m_nCurrScreen = aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption].m_TargetMenu;
		m_nCurrOption = 0;
	} else {
		m_nCurrOption = 0;
		m_nCurrScreen = screen;
	}

	if (m_nCurrScreen == MENUPAGE_CHOOSE_SAVE_SLOT)
		m_nCurrOption = 8;
	m_nMenuFadeAlpha = 0;
	m_nOptionHighlightTransitionBlend = 0;
	m_LastScreenSwitch = CTimer::GetTimeInMillisecondsPauseMode();
}
#endif

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

			m_nFirstVisibleRowOnList = Max(0, m_nFirstVisibleRowOnList - MAX_VISIBLE_LIST_ROW);
			m_nSelectedListRow = Min(m_nSelectedListRow, m_nFirstVisibleRowOnList + MAX_VISIBLE_LIST_ROW - 1);
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

			m_nFirstVisibleRowOnList = Min(m_nFirstVisibleRowOnList + MAX_VISIBLE_LIST_ROW, m_nTotalListRow - MAX_VISIBLE_LIST_ROW);
			m_nSelectedListRow = Max(m_nSelectedListRow, m_nFirstVisibleRowOnList);
		} else {
			m_nFirstVisibleRowOnList = m_nTotalListRow - MAX_VISIBLE_LIST_ROW;
			m_nSelectedListRow = m_nTotalListRow - 1;
		}
		m_nScrollbarTopMargin = (SCROLLBAR_MAX_HEIGHT / m_nTotalListRow) * m_nFirstVisibleRowOnList;
	}
}

// TODO(Miami)
inline void
CMenuManager::ThingsToDoBeforeLeavingPage()
{
	if ((m_nCurrScreen == MENUPAGE_SKIN_SELECT) && strcmp(m_aSkinName, m_PrefsSkinFile) != 0) {
		CWorld::Players[0].SetPlayerSkin(m_PrefsSkinFile);
	} else if (m_nCurrScreen == MENUPAGE_SOUND_SETTINGS) {
		if (m_nPrefsAudio3DProviderIndex != INVALID_AUDIO_PROVIDER)
			m_nPrefsAudio3DProviderIndex = DMAudio.GetCurrent3DProviderIndex();
#ifdef TIDY_UP_PBP
		DMAudio.StopFrontEndTrack();
		OutputDebugString("FRONTEND AUDIO TRACK STOPPED");
#endif
	} else if (m_nCurrScreen == MENUPAGE_GRAPHICS_SETTINGS) {
		m_nDisplayVideoMode = m_nPrefsVideoMode;
#ifdef IMPROVED_VIDEOMODE
		m_nSelectedScreenMode = m_nPrefsWindowed;
#endif
	}

	if (m_nCurrScreen == MENUPAGE_SKIN_SELECT) {
		CPlayerSkin::EndFrontendSkinEdit();
	}

	if ((m_nCurrScreen == MENUPAGE_SKIN_SELECT) || (m_nCurrScreen == MENUPAGE_KEYBOARD_CONTROLS)) {
		m_nTotalListRow = 0;
	}
}

// ------ Functions not in the game/inlined ends

CMenuManager::CMenuManager()
{
	m_StatsScrollSpeed = 150.0f;
	m_StatsScrollDirection = 1;
	m_PrefsSfxVolume = 49;
	m_PrefsMusicVolume = 49;
	m_PrefsRadioStation = 0;
	field_2C = 1;
	m_PrefsBrightness = 256;
	m_PrefsLOD = 1.2f;
	m_KeyPressedCode = -1;
	m_bFrontEnd_ReloadObrTxtGxt = false;
	m_PrefsMP3BoostVolume = 0;
	m_PrefsShowSubtitles = 0;
	m_PrefsShowLegends = 1;
	m_PrefsUseWideScreen = 0;
	m_PrefsVsync = 0;
	m_PrefsVsyncDisp = 1;
	m_PrefsFrameLimiter = 1;
	m_PrefsLanguage = 0;
	field_54 = 0;
	m_PrefsAllowNastyGame = 1;
	m_PrefsSpeakers = 0;
	field_8 = 0;
	m_PrefsUseVibration = 0;
	m_PrefsShowHud = 1;
	m_PrefsRadarMode = 0;
	field_10 = 0;
	m_bShutDownFrontEndRequested = false;
	m_bStartUpFrontEndRequested = false;
	pEditString = nil;
	pControlEdit = nil;
	DisplayComboButtonErrMsg = false;
	m_PrefsDMA = 1;
	OS_Language = LANG_ENGLISH;
	m_ControlMethod = CONTROL_STANDARD;
	CCamera::m_bUseMouse3rdPerson = true;
	m_lastWorking3DAudioProvider = 0;
	m_nFirstVisibleRowOnList = 0;
	m_nScrollbarTopMargin = 0.0f;
	m_nSelectedListRow = 0;
	m_nSkinsTotal = 0;
	m_nPrefsAudio3DProviderIndex = -99;
	m_bGameNotLoaded = true;
	m_nMousePosX = m_nMouseTempPosX;
	m_nMousePosY = m_nMouseTempPosY;
	m_nMouseOldPosX = m_nMousePosX;
	m_nMouseOldPosY = m_nMousePosY;
	m_bShowMouse = true;
	m_nHoverOption = HOVEROPTION_NOT_HOVERING;

	// TODO(Miami)
	// DMAudio.SetMP3BoostVolume(m_PrefsMP3BoostVolume);
	m_bMenuActive = false;
	m_bActivateSaveMenu = false;
	m_bWantToLoad = false;
	m_nMenuFadeAlpha = 0;
	m_OnlySaveMenu = false;
	m_fMapSize = 162.0f;
	m_fMapCenterX = 320.0f;
	m_fMapCenterY = 225.0f;
	DMAudio.SetMusicMasterVolume(m_PrefsMusicVolume);
	DMAudio.SetEffectsMasterVolume(m_PrefsSfxVolume);
}

void
CMenuManager::SetFrontEndRenderStates(void)
{
	RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void*)rwFILTERLINEAR);
	RwRenderStateSet(rwRENDERSTATETEXTUREADDRESS, (void*)rwTEXTUREADDRESSCLAMP);
	RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*)FALSE);
	RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)FALSE);
	RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDSRCALPHA);
	RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDINVSRCALPHA);
	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)TRUE);
}

void
CMenuManager::Initialise(void)
{
	DoRWStuffStartOfFrame(0, 0, 0, 0, 0, 0, 255);
	DoRWStuffEndOfFrame();
	m_AllowNavigation = false;
	m_menuTransitionProgress = -50;
	m_nMenuFadeAlpha = 0;
	m_nCurrOption = 0;
	m_nOptionHighlightTransitionBlend = 0;
	CentreMousePointer();
	m_bShowMouse = true;
	m_fMapSize = 162.0f;
	m_fMapCenterX = 320.0f;
	m_fMapCenterY = 225.0f;
	CPad::StopPadsShaking();
	if (!m_OnlySaveMenu)
		m_nCurrScreen = MENUPAGE_NONE;
	DMAudio.ChangeMusicMode(MUSICMODE_FRONTEND);
	DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_STARTING, 0);
	DMAudio.Service();
	DMAudio.SetMusicMasterVolume(m_PrefsMusicVolume);
	DMAudio.SetEffectsMasterVolume(m_PrefsSfxVolume);
	m_PrefsRadioStation = DMAudio.GetRadioInCar();

	// TODO(Miami)
	// DMAudio.SetMP3BoostVolume(m_PrefsMP3BoostVolume);
	if (DMAudio.IsMP3RadioChannelAvailable()) {
		if (m_PrefsRadioStation < WILDSTYLE || m_PrefsRadioStation > USERTRACK)
			m_PrefsRadioStation = CGeneral::GetRandomNumber() % 10;
	} else if (m_PrefsRadioStation < WILDSTYLE || m_PrefsRadioStation > WAVE)
		m_PrefsRadioStation = CGeneral::GetRandomNumber() % 9;

	CFileMgr::SetDir("");
	//CFileMgr::SetDir("");
	PcSaveHelper.PopulateSlotInfo();
	CTimer::StartUserPause();
}

void
CMenuManager::BuildStatLine(Const char *text, void *stat, bool itsFloat, void *stat2)
{
	if (!text)
		return;

#ifdef MORE_LANGUAGES
	if (CFont::IsJapanese() && stat2)
		if (itsFloat)
			sprintf(gString2, "  %.2f/%.2f", *(float*)stat, *(float*)stat2);
		else
			sprintf(gString2, "  %d/%d", *(int*)stat, *(int*)stat2);
	else
#endif
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
	if (SCREEN_WIDTH * 0.5f != 0.0f && 0.0f != SCREEN_HEIGHT * 0.5f) {
#if defined RW_D3D9 || defined RWLIBS
		tagPOINT Point;
		Point.x = SCREEN_WIDTH / 2;
		Point.y = SCREEN_HEIGHT / 2;
		ClientToScreen(PSGLOBAL(window), &Point);
		SetCursorPos(Point.x, Point.y);
#elif defined RW_GL3
		glfwSetCursorPos(PSGLOBAL(window), SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);
#endif

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
		} else if (*pControlEdit != rsF1 && *pControlEdit != rsF2 && *pControlEdit != rsF3 && *pControlEdit != rsF9 &&
			*pControlEdit != rsLWIN && *pControlEdit != rsRWIN && *pControlEdit != rsRALT) {
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

#ifdef FIX_BUGS
	if(!escPressed && !invalidKey)
#endif
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
	static PauseModeTime LastFlash = 0;
	int32 alpha;

	if (m_nHelperTextMsgId != 0 && m_nHelperTextMsgId != 1) {
		if (CTimer::GetTimeInMillisecondsPauseMode() - LastFlash > 10) {
			LastFlash = CTimer::GetTimeInMillisecondsPauseMode();
			m_nHelperTextAlpha -= 2;
		}

		if (m_nHelperTextAlpha < 1)
			ResetHelperText();

		alpha = m_nHelperTextAlpha > 255 ? 255 : m_nHelperTextAlpha;
	}

	CFont::SetCentreOn();
	CFont::SetScale(SCREEN_SCALE_X(SMALLESTTEXT_X_SCALE), SCREEN_SCALE_Y(SMALLESTTEXT_Y_SCALE));
	CFont::SetFontStyle(FONT_LOCALE(FONT_HEADING));

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

// --MIAMI: Done
int
CMenuManager::DisplaySlider(float x, float y, float mostLeftBarSize, float mostRightBarSize, float rectSize, float progress, float spacing) 
{
	CRGBA color;
	float maxBarHeight;

	int lastActiveBarX = 0;
	float curBarX = 0.0f;
	for (int i = 0; i < 16; i++) {
		curBarX = i * rectSize/16.0f + x;

		if (i / 16.0f + 1 / 32.0f < progress) {
			color = CRGBA(SLIDERON_COLOR.r, SLIDERON_COLOR.g, SLIDERON_COLOR.b, FadeIn(255));
			lastActiveBarX = curBarX;
		} else
			color = CRGBA(SLIDEROFF_COLOR.r, SLIDEROFF_COLOR.g, SLIDEROFF_COLOR.b, FadeIn(255));

		maxBarHeight = Max(mostLeftBarSize, mostRightBarSize);

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

// TODO(Miami)
void
CMenuManager::DoSettingsBeforeStartingAGame()
{
	CCamera::m_bUseMouse3rdPerson = m_ControlMethod == CONTROL_STANDARD;
	if (m_PrefsVsyncDisp != m_PrefsVsync)
		m_PrefsVsync = m_PrefsVsyncDisp;

	DMAudio.DestroyAllGameCreatedEntities();
	DMAudio.Service();
	m_bShutDownFrontEndRequested = true;
	m_bWantToRestart = true;
	DMAudio.SetEffectsFadeVol(0);
	DMAudio.SetMusicFadeVol(0);
	SwitchMenuOnAndOff();
	DMAudio.ResetTimers(CTimer::GetTimeInMilliseconds());
}

void
CMenuManager::DrawStandardMenus(bool drawCurrScreen)
{
	float nextYToUse = 0.0f; // III leftover, set but unused in VC
#ifdef PS2_LIKE_MENU
	bool itemsAreSelectable = !bottomBarActive;
#else
	bool itemsAreSelectable = true;
#endif
	CFont::SetBackgroundOff();
	CFont::SetPropOn();
	CFont::SetCentreOff();
	CFont::SetJustifyOn();
	CFont::SetBackGroundOnlyTextOff();

	CFont::SetWrapx(MENU_X_RIGHT_ALIGNED(MENU_UNK_X_MARGIN));
	CFont::SetRightJustifyWrap(SCREEN_SCALE_X(MENU_UNK_WIDTH));
	CFont::SetCentreSize(SCREEN_WIDTH);

	switch (m_nCurrScreen) {
		case MENUPAGE_CHOOSE_LOAD_SLOT:
		case MENUPAGE_CHOOSE_DELETE_SLOT:
		case MENUPAGE_CHOOSE_SAVE_SLOT:
			CSprite2d::Draw2DPolygon(MENU_X_LEFT_ALIGNED(38.0f), MENU_Y(85.0f),
				MENU_X_LEFT_ALIGNED(615.0f), MENU_Y(75.0f),
				MENU_X_LEFT_ALIGNED(30.0f), MENU_Y(320.0f), 
				MENU_X_LEFT_ALIGNED(605.0f), MENU_Y(330.0f), CRGBA(49, 101, 148, FadeIn(130)));
			break;
		/*
		// TODO(Miami)
		case MENUPAGE_SOUND_SETTINGS:
			PrintSoundSettings();
			break;
		*/
		case MENUPAGE_STATS:
			PrintStats();
			break;
		case MENUPAGE_BRIEFS:
			PrintBriefs();
			break;
	}

	// Page name
	if (aScreens[m_nCurrScreen].m_ScreenName[0] != '\0') {

		PREPARE_MENU_HEADER
		CFont::SetColor(CRGBA(30, 30, 30, FadeIn(255)));
		CFont::PrintString(SCREEN_SCALE_FROM_RIGHT(MENUHEADER_POS_X + 7.f), SCREEN_SCALE_Y(MENUHEADER_POS_Y + 7.f), TheText.Get(aScreens[m_nCurrScreen].m_ScreenName));

		CFont::SetColor(CRGBA(HEADER_COLOR.r, HEADER_COLOR.g, HEADER_COLOR.b, FadeIn(255)));
		CFont::PrintString(SCREEN_SCALE_FROM_RIGHT(MENUHEADER_POS_X), SCREEN_SCALE_Y(MENUHEADER_POS_Y), TheText.Get(aScreens[m_nCurrScreen].m_ScreenName));
	}

	// Label
	wchar *str;
	if (aScreens[m_nCurrScreen].m_aEntries[0].m_Action == MENUACTION_LABEL) {
		CFont::SetWrapx(MENU_X_RIGHT_ALIGNED(MENULABEL_X_MARGIN));
		CFont::SetRightJustifyWrap(SCREEN_SCALE_X(MENULABEL_WIDTH));
		CFont::SetFontStyle(FONT_LOCALE(FONT_BANK));
		CFont::SetScale(MENU_X(BIGTEXT2_X_SCALE), MENU_Y(BIGTEXT2_Y_SCALE));
		CFont::SetRightJustifyOff();

		CFont::SetDropShadowPosition(2);
		CFont::SetDropColor(CRGBA(0, 0, 0, FadeIn(255)));
		CFont::SetColor(CRGBA(LABEL_COLOR.r, LABEL_COLOR.g, LABEL_COLOR.b, FadeIn(255)));

		switch (m_nCurrScreen) {
		case MENUPAGE_LOAD_SLOT_CONFIRM:
			if (m_bGameNotLoaded)
				str = TheText.Get("FES_LCG");
			else
				str = TheText.Get(aScreens[m_nCurrScreen].m_aEntries[0].m_EntryName);
			break;
		case MENUPAGE_DELETE_SLOT_CONFIRM:
			str = TheText.Get(aScreens[MENUPAGE_DELETE_SLOT_CONFIRM].m_aEntries[0].m_EntryName);
			break;
		case MENUPAGE_SAVE_OVERWRITE_CONFIRM:
			if (Slots[m_nCurrSaveSlot + 1] == SLOT_OK)
				str = TheText.Get("FESZ_QO");
			else if (Slots[m_nCurrSaveSlot + 1] == SLOT_CORRUPTED)
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

		CFont::PrintString(MENU_X_LEFT_ALIGNED(100.0f), MENU_Y(97.0f), str);
		CFont::SetWrapx(MENU_X_RIGHT_ALIGNED(MENU_UNK_X_MARGIN));
		CFont::SetRightJustifyWrap(SCREEN_SCALE_X(MENU_UNK_WIDTH));
	}

	if (m_nCurrScreen == MENUPAGE_KEYBOARD_CONTROLS) {
		if (m_bWaitingForNewKeyBind)
			itemsAreSelectable = false;

		DrawControllerScreenExtraText(-8.0f, MENU_X_LEFT_ALIGNED(350), MENU_DEFAULT_LINE_HEIGHT);
	}

	bool foundTheHoveringItem = false;
	wchar unicodeTemp[64];
	char asciiTemp[32];

#ifdef MENU_MAP
	if (m_nCurrScreen == MENUPAGE_MAP) {
		// Back button
		wchar *backTx = TheText.Get("FEDS_TB");
		CFont::SetDropShadowPosition(1);
		CFont::SetDropColor(CRGBA(0, 0, 0, 255));
		CFont::SetScale(MENU_X(BIGTEXT_X_SCALE), MENU_Y(BIGTEXT_Y_SCALE));
		CFont::SetRightJustifyOff();
		CFont::SetCentreOn();
		CFont::PrintString(MENU_X(60.0f), SCREEN_SCALE_FROM_BOTTOM(120.0f), backTx);
		CFont::SetDropShadowPosition(0);
		if (!CheckHover(MENU_X(30.0f), MENU_X(30.0f) + CFont::GetStringWidth(backTx), SCREEN_SCALE_FROM_BOTTOM(125.0f), SCREEN_SCALE_FROM_BOTTOM(105.0f))) {
			m_nHoverOption = HOVEROPTION_NOT_HOVERING;
			m_nCurrOption = m_nPrevOption = 0;
		} else {
			m_nHoverOption = HOVEROPTION_RANDOM_ITEM;
			m_nCurrOption = m_nPrevOption = 1;
		}
		return;
	}
#endif

	bool weHaveLabel = aScreens[m_nCurrScreen].m_aEntries[0].m_Action == MENUACTION_LABEL;
	uint8 section = 0; // 0: highlight trapezoid  1: texts

	while (section < 2) {

		for (int i = 0; i < NUM_MENUROWS; ++i) {
			wchar* rightText = nil;
			wchar* leftText;
			if (aScreens[m_nCurrScreen].m_aEntries[i].m_SaveSlot >= SAVESLOT_1 && aScreens[m_nCurrScreen].m_aEntries[i].m_SaveSlot <= SAVESLOT_8) {
				CFont::SetColor(CRGBA(0, 0, 0, FadeIn(255)));
				CFont::SetFontStyle(FONT_LOCALE(FONT_BANK));
				CFont::SetScale(MENU_X(MEDIUMTEXT_X_SCALE), MENU_Y(MEDIUMTEXT_Y_SCALE));
				CFont::SetDropShadowPosition(0);
			} else {
				CFont::SetFontStyle(FONT_LOCALE(FONT_HEADING));
				CFont::SetScale(MENU_X(BIGTEXT_X_SCALE), MENU_Y(BIGTEXT_Y_SCALE));
				CFont::SetDropShadowPosition(2);
				CFont::SetDropColor(CRGBA(0, 0, 0, FadeIn(255)));
				CFont::SetColor(CRGBA(MENUOPTION_COLOR.r, MENUOPTION_COLOR.g, MENUOPTION_COLOR.b, FadeIn(255)));
			}
			if (aScreens[m_nCurrScreen].m_aEntries[i].m_Align == MENUALIGN_LEFT) {
				CFont::SetCentreOff();
				CFont::SetRightJustifyOff();
			} else if (aScreens[m_nCurrScreen].m_aEntries[i].m_Align == MENUALIGN_RIGHT) {
				CFont::SetCentreOff();
				CFont::SetRightJustifyOn();
			} else {
				CFont::SetRightJustifyOff();
				CFont::SetCentreOn();
			}
			if (!aScreens[m_nCurrScreen].m_aEntries[i].m_X && !aScreens[m_nCurrScreen].m_aEntries[i].m_Y) {
				if (i == 0 || (i == 1 && weHaveLabel)) {
					aScreens[m_nCurrScreen].m_aEntries[i].m_X = MENU_DEFAULT_CONTENT_X;
					aScreens[m_nCurrScreen].m_aEntries[i].m_Y = MENU_DEFAULT_CONTENT_Y;
					
				} else {
					aScreens[m_nCurrScreen].m_aEntries[i].m_X = aScreens[m_nCurrScreen].m_aEntries[i-1].m_X;
					aScreens[m_nCurrScreen].m_aEntries[i].m_Y = aScreens[m_nCurrScreen].m_aEntries[i-1].m_Y + MENU_DEFAULT_LINE_HEIGHT;
				}
			}

			if (aScreens[m_nCurrScreen].m_aEntries[i].m_Action != MENUACTION_LABEL && aScreens[m_nCurrScreen].m_aEntries[i].m_EntryName[0] != '\0') {

				if (aScreens[m_nCurrScreen].m_aEntries[i].m_SaveSlot >= SAVESLOT_1 && aScreens[m_nCurrScreen].m_aEntries[i].m_SaveSlot <= SAVESLOT_8) {
					CFont::SetRightJustifyOff();

					leftText = nil;
					if (Slots[i] == SLOT_OK) {
						leftText = GetNameOfSavedGame(i);
						rightText = GetSavedGameDateAndTime(i);
					}

					if (!leftText || leftText[0] == '\0') {
						sprintf(gString, "FEM_SL%d", i + 1);
						leftText = TheText.Get(gString);
					}
				} else {
					leftText = TheText.Get(aScreens[m_nCurrScreen].m_aEntries[i].m_EntryName);
				}

				if (m_nPrefsAudio3DProviderIndex == INVALID_AUDIO_PROVIDER) {
					if (strncmp(aScreens[m_nCurrScreen].m_aEntries[i].m_EntryName, "FEO_AUD", 8) == 0) {
						CFont::SetColor(CRGBA(DARKMENUOPTION_COLOR.r, DARKMENUOPTION_COLOR.g, DARKMENUOPTION_COLOR.b, FadeIn(255)));
					}
				}

				switch (aScreens[m_nCurrScreen].m_aEntries[i].m_Action) {
#ifdef LEGACY_MENU_OPTIONS
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
#endif
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
						sprintf(asciiTemp, "AUTO");
						break;
					case AR_4_3:
						sprintf(asciiTemp, "4:3");
						break;
					case AR_16_9:
						sprintf(asciiTemp, "16:9");
						break;
					}

					AsciiToUnicode(asciiTemp, unicodeTemp);
					rightText = unicodeTemp;
#endif
					break;

				case MENUACTION_MUSICVOLUME:
				case MENUACTION_SFXVOLUME:
					if (m_nPrefsAudio3DProviderIndex == INVALID_AUDIO_PROVIDER)
						rightText = TheText.Get("FEA_NAH");

					break;
				case MENUACTION_RADIO:
					if (m_PrefsRadioStation > USERTRACK)
						break;

					sprintf(gString, "FEA_FM%d", m_PrefsRadioStation);
					rightText = TheText.Get(gString);
					break;
				case MENUACTION_LEGENDS:
					rightText = TheText.Get(m_PrefsShowLegends ? "FEM_ON" : "FEM_OFF");
					break;
				case MENUACTION_RADARMODE:
					switch (m_PrefsRadarMode) {
						case 0:
							rightText = TheText.Get("FED_RDM");
							break;
						case 1:
							rightText = TheText.Get("FED_RDB");
							break;
						case 2:
							rightText = TheText.Get("FEM_OFF");
							break;
					}
					break;
				case MENUACTION_HUD:
					rightText = TheText.Get(m_PrefsShowHud ? "FEM_ON" : "FEM_OFF");
#ifdef LEGACY_MENU_OPTIONS
				case MENUACTION_SETDBGFLAG:
					rightText = TheText.Get(CTheScripts::IsDebugOn() ? "FEM_ON" : "FEM_OFF");
					break;
				case MENUACTION_SWITCHBIGWHITEDEBUGLIGHT:
					rightText = TheText.Get(gbBigWhiteDebugLightSwitchedOn ? "FEM_ON" : "FEM_OFF");
					break;
				case MENUACTION_COLLISIONPOLYS:
					rightText = TheText.Get(gbShowCollisionPolys ? "FEM_ON" : "FEM_OFF");
					break;
#endif
				case MENUACTION_SHOWHEADBOB:
					rightText = TheText.Get(TheCamera.m_bHeadBob ? "FEM_ON" : "FEM_OFF");
					break;
				case MENUACTION_INVVERT:
					rightText = TheText.Get(MousePointerStateHelper.bInvertVertically ? "FEM_OFF" : "FEM_ON");
					break;
				case MENUACTION_SCREENRES:
					AsciiToUnicode(_psGetVideoModeList()[m_nDisplayVideoMode], unicodeTemp);
					rightText = unicodeTemp;

					if (!m_bGameNotLoaded) {
						CFont::SetColor(CRGBA(DARKMENUOPTION_COLOR.r, DARKMENUOPTION_COLOR.g, DARKMENUOPTION_COLOR.b, FadeIn(255)));
					}
					break;
#ifdef IMPROVED_VIDEOMODE
				case MENUACTION_SCREENMODE:
					if (m_nSelectedScreenMode == 0)
						sprintf(asciiTemp, "FULLSCREEN");
					else
						sprintf(asciiTemp, "WINDOWED");

					AsciiToUnicode(asciiTemp, unicodeTemp);
					rightText = unicodeTemp;
					break;
#endif
				case MENUACTION_AUDIOHW:
					if (m_nPrefsAudio3DProviderIndex == INVALID_AUDIO_PROVIDER)
						rightText = TheText.Get("FEA_NAH");
					else if (m_nPrefsAudio3DProviderIndex == -1)
						rightText = TheText.Get("FEA_ADP");
					else {
						char* provider = DMAudio.Get3DProviderName(m_nPrefsAudio3DProviderIndex);

						if (!strcmp(strupr(provider), "DIRECTSOUND3D HARDWARE SUPPORT")) {
							strcpy(provider, "DSOUND3D HARDWARE SUPPORT");
						}
						else if (!strcmp(strupr(provider), "DIRECTSOUND3D SOFTWARE EMULATION")) {
							strcpy(provider, "DSOUND3D SOFTWARE EMULATION");
						}
						AsciiToUnicode(provider, unicodeTemp);
						rightText = unicodeTemp;
					}
					break;
				case MENUACTION_SPEAKERCONF: {
					if (m_nPrefsAudio3DProviderIndex == INVALID_AUDIO_PROVIDER)
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
					case CONTROL_STANDARD:
						leftText = TheText.Get("FET_STI");
						break;
					case CONTROL_CLASSIC:
						leftText = TheText.Get("FET_CTI");
						break;
					}
					break;
				}
				case MENUACTION_DYNAMICACOUSTIC:
					rightText = TheText.Get(m_PrefsDMA ? "FEM_ON" : "FEM_OFF");
					break;
				case MENUACTION_MOUSESTEER:
					rightText = TheText.Get(CVehicle::m_bDisableMouseSteering ? "FEM_OFF" : "FEM_ON");
					if (m_ControlMethod == CONTROL_CLASSIC) {
						CFont::SetColor(CRGBA(DARKMENUOPTION_COLOR.r, DARKMENUOPTION_COLOR.g, DARKMENUOPTION_COLOR.b, FadeIn(255)));
					}
					break;
				case MENUACTION_MP3VOLUMEBOOST:
					if (!DMAudio.IsMP3RadioChannelAvailable()) {
						rightText = TheText.Get("FEA_NM3");
					}
					break;
				}

				// Highlight trapezoid
				if (drawCurrScreen && i == m_nCurrOption && itemsAreSelectable && section == 0) {

					int leftXMax, rightXMin;
					CFont::SetScale(MENU_X(BIGTEXT_X_SCALE), MENU_Y(BIGTEXT_Y_SCALE));
					wchar *curOptionName = TheText.Get(aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption].m_EntryName);
					float curOptionWidth = CFont::GetStringWidth(curOptionName, true);

					if (CFont::Details.centre) {
						leftXMax = Max(0, aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption].m_X - curOptionWidth / 2.f);
						rightXMin = Min(SCREEN_WIDTH, curOptionWidth / 2.f + aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption].m_X);
					} else if (!CFont::Details.rightJustify) {
						leftXMax = aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption].m_X;
						rightXMin = Min(SCREEN_WIDTH, curOptionWidth + aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption].m_X);
					} else {
						leftXMax = Max(0, aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption].m_X - curOptionWidth);
						rightXMin = aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption].m_X;
					}
					CFont::SetScale(MENU_X(BIGTEXT_X_SCALE), MENU_Y(BIGTEXT_Y_SCALE));

					int action = aScreens[m_nCurrScreen].m_aEntries[i].m_Action;
					int saveSlot = aScreens[m_nCurrScreen].m_aEntries[i].m_SaveSlot;
					if (rightText || action == MENUACTION_DRAWDIST || action == MENUACTION_BRIGHTNESS || action == MENUACTION_MUSICVOLUME ||
						action == MENUACTION_SFXVOLUME || action == MENUACTION_MP3VOLUMEBOOST || action == MENUACTION_MOUSESENS ||
						saveSlot >= SAVESLOT_1 && saveSlot <= SAVESLOT_8) {
						rightXMin = 600;
						leftXMax = 40;
					}

					int y = aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption].m_Y;
					int topYMax = y;
					uint32 bottomYMin = y + 22;
					int transition = m_nOptionHighlightTransitionBlend;

					// Actually bottomRight and bottomLeft should be exchanged here(although this is original code).
					// So this shows us either R* didn't use same struct for menu BG and highlight, or they just kept fields as x1,y1 etc. Yikes.

					if (transition == 0) {
						if (m_menuTransitionProgress == 255 && m_nMenuFadeAlpha == 255 && !bMenuChangeOngoing) {
							CSprite2d::Draw2DPolygon(MENU_X_LEFT_ALIGNED(menuOptionHighlight.topLeft_x), MENU_Y(menuOptionHighlight.topLeft_y),
								MENU_X_LEFT_ALIGNED(menuOptionHighlight.topRight_x), MENU_Y(menuOptionHighlight.topRight_y),
								MENU_X_LEFT_ALIGNED(menuOptionHighlight.bottomRight_x), MENU_Y(menuOptionHighlight.bottomRight_y),
								MENU_X_LEFT_ALIGNED(menuOptionHighlight.bottomLeft_x), MENU_Y(menuOptionHighlight.bottomLeft_y), SELECTIONBORDER_COLOR);
						}
						menuOptionHighlight.SaveCurrentCoors();
						menuOptionHighlight.topLeft_x = leftXMax - 5 - CGeneral::GetRandomNumber() % 10;
						menuOptionHighlight.topLeft_y = topYMax - CGeneral::GetRandomNumber() % 7;
						menuOptionHighlight.topRight_x = rightXMin + 5 + CGeneral::GetRandomNumber() % 10;
						menuOptionHighlight.topRight_y = topYMax - CGeneral::GetRandomNumber() % 7;
						menuOptionHighlight.bottomLeft_x = rightXMin + 5 + CGeneral::GetRandomNumber() % 10;
						menuOptionHighlight.bottomLeft_y = bottomYMin + CGeneral::GetRandomNumber() % 7;
						menuOptionHighlight.bottomRight_x = leftXMax - 5 - CGeneral::GetRandomNumber() % 10;
						menuOptionHighlight.bottomRight_y = bottomYMin + CGeneral::GetRandomNumber() % 7;
						menuOptionHighlight.UpdateMultipliers();
						menuOptionHighlight.Translate(m_nOptionHighlightTransitionBlend);
					} else if (transition < 255) {
						menuOptionHighlight.Translate(m_nOptionHighlightTransitionBlend);
						if (m_menuTransitionProgress == 255 && m_nMenuFadeAlpha == 255 && !bMenuChangeOngoing) {
							CSprite2d::Draw2DPolygon(MENU_X_LEFT_ALIGNED(menuOptionHighlight.topLeft_x), MENU_Y(menuOptionHighlight.topLeft_y),
								MENU_X_LEFT_ALIGNED(menuOptionHighlight.topRight_x), MENU_Y(menuOptionHighlight.topRight_y),
								MENU_X_LEFT_ALIGNED(menuOptionHighlight.bottomRight_x), MENU_Y(menuOptionHighlight.bottomRight_y),
								MENU_X_LEFT_ALIGNED(menuOptionHighlight.bottomLeft_x), MENU_Y(menuOptionHighlight.bottomLeft_y), SELECTIONBORDER_COLOR);
						}
					} else {
						m_nOptionHighlightTransitionBlend = 255;
						menuOptionHighlight.Translate(m_nOptionHighlightTransitionBlend);
						if (m_menuTransitionProgress == 255 && m_nMenuFadeAlpha == 255 && !bMenuChangeOngoing) {
							CSprite2d::Draw2DPolygon(MENU_X_LEFT_ALIGNED(menuOptionHighlight.topLeft_x), MENU_Y(menuOptionHighlight.topLeft_y),
								MENU_X_LEFT_ALIGNED(menuOptionHighlight.topRight_x), MENU_Y(menuOptionHighlight.topRight_y),
								MENU_X_LEFT_ALIGNED(menuOptionHighlight.bottomRight_x), MENU_Y(menuOptionHighlight.bottomRight_y),
								MENU_X_LEFT_ALIGNED(menuOptionHighlight.bottomLeft_x), MENU_Y(menuOptionHighlight.bottomLeft_y), SELECTIONBORDER_COLOR);
						}
					}

					static PauseModeTime lastBlendChange = 0;
					if (m_nOptionHighlightTransitionBlend <= 255) {
						static uint32 blendChangeCounter = 0;
						if (CTimer::GetTimeInMillisecondsPauseMode() - lastBlendChange > 20
#ifndef FIX_HIGH_FPS_BUGS_ON_FRONTEND // Dirty dirty hack
							|| blendChangeCounter > 20
#endif
							) {
							m_nOptionHighlightTransitionBlend += 50;
							lastBlendChange = CTimer::GetTimeInMillisecondsPauseMode();
							blendChangeCounter = 0;
						}
						++blendChangeCounter;
					}
				}

				if (section != 0) {

					if (leftText) {
						CFont::PrintString(MENU_X_LEFT_ALIGNED(aScreens[m_nCurrScreen].m_aEntries[i].m_X), MENU_Y(aScreens[m_nCurrScreen].m_aEntries[i].m_Y), leftText);
					}

					if (rightText) {
						CFont::SetCentreOff();
						CFont::SetRightJustifyOn();
						if (aScreens[m_nCurrScreen].m_aEntries[i].m_SaveSlot >= SAVESLOT_1 && aScreens[m_nCurrScreen].m_aEntries[i].m_SaveSlot <= SAVESLOT_8) {
							CFont::SetFontStyle(FONT_LOCALE(FONT_BANK));
							CFont::SetScale(MENU_X(MEDIUMTEXT_X_SCALE), MENU_Y(MEDIUMTEXT_Y_SCALE));
						} else {
							CFont::SetFontStyle(FONT_LOCALE(FONT_HEADING));
							CFont::SetScale(MENU_X(BIGTEXT_X_SCALE), MENU_Y(BIGTEXT_Y_SCALE));
						}
						CFont::PrintString(MENU_X_LEFT_ALIGNED(600.0f), MENU_Y(aScreens[m_nCurrScreen].m_aEntries[i].m_Y), rightText);
					}

					if (m_nPrefsAudio3DProviderIndex == DMAudio.GetCurrent3DProviderIndex()) {
						if (!strcmp(aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption].m_EntryName, "FEA_3DH") && m_nHelperTextMsgId == 1)
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
							&& m_nCurrScreen == MENUPAGE_SOUND_SETTINGS && m_nPrefsAudio3DProviderIndex != INVALID_AUDIO_PROVIDER) {

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
#ifdef IMPROVED_VIDEOMODE
					if (m_nSelectedScreenMode != m_nPrefsWindowed) {
						if (strcmp(aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption].m_EntryName, "SCRFOR") != 0
							&& m_nCurrScreen == MENUPAGE_GRAPHICS_SETTINGS) {
							m_nSelectedScreenMode = m_nPrefsWindowed;
						}
					}
#endif

					// Sliders
					int lastActiveBarX;
					switch (aScreens[m_nCurrScreen].m_aEntries[i].m_Action) {
					case MENUACTION_BRIGHTNESS:
						ProcessSlider(m_PrefsBrightness / 384.0f, 70.0f, HOVEROPTION_INCREASE_BRIGHTNESS, HOVEROPTION_DECREASE_BRIGHTNESS, MENU_X_LEFT_ALIGNED(170.0f), SCREEN_WIDTH);
						break;
					case MENUACTION_DRAWDIST:
						ProcessSlider((m_PrefsLOD - 0.925f) / 0.875f, 99.0f, HOVEROPTION_INCREASE_DRAWDIST, HOVEROPTION_DECREASE_DRAWDIST, MENU_X_LEFT_ALIGNED(170.0f), SCREEN_WIDTH);
						break;
					case MENUACTION_MUSICVOLUME:
						if(m_nPrefsAudio3DProviderIndex != INVALID_AUDIO_PROVIDER)
							ProcessSlider(m_PrefsMusicVolume / 64.0f, 70.0f, HOVEROPTION_INCREASE_MUSICVOLUME, HOVEROPTION_DECREASE_MUSICVOLUME, MENU_X_LEFT_ALIGNED(170.0f), SCREEN_WIDTH);
						break;
					case MENUACTION_SFXVOLUME:
						if (m_nPrefsAudio3DProviderIndex != INVALID_AUDIO_PROVIDER)
							ProcessSlider(m_PrefsSfxVolume / 64.0f, 99.0f, HOVEROPTION_INCREASE_SFXVOLUME, HOVEROPTION_DECREASE_SFXVOLUME, MENU_X_LEFT_ALIGNED(170.0f), SCREEN_WIDTH);
						break;
					case MENUACTION_MOUSESENS:
						ProcessSlider(TheCamera.m_fMouseAccelHorzntl * 200.0f, 170.0f, HOVEROPTION_INCREASE_MOUSESENS, HOVEROPTION_DECREASE_MOUSESENS, MENU_X_LEFT_ALIGNED(200.0f), SCREEN_WIDTH);
						break;
					}

					nextYToUse = MENU_Y(150.f); // TODO(Miami): Temp

					nextYToUse += MENU_DEFAULT_LINE_HEIGHT * CFont::GetNumberLines(MENU_X_LEFT_ALIGNED(60.0f), MENU_Y(nextYToUse), leftText);

					// TODO(Miami): Remove here after audio page is done
					// Radio icons
					if (aScreens[m_nCurrScreen].m_aEntries[i].m_Action == MENUACTION_RADIO) {
						ProcessRadioIcon(m_aFrontEndSprites[MENUSPRITE_WILDSTYLE], MENU_X_LEFT_ALIGNED(30.0f), MENU_Y(nextYToUse), 0, HOVEROPTION_RADIO_0);
						ProcessRadioIcon(m_aFrontEndSprites[MENUSPRITE_FLASH], MENU_X_LEFT_ALIGNED(90.0f), MENU_Y(nextYToUse), 1, HOVEROPTION_RADIO_1);
						ProcessRadioIcon(m_aFrontEndSprites[MENUSPRITE_KCHAT], MENU_X_LEFT_ALIGNED(150.0f), MENU_Y(nextYToUse), 2, HOVEROPTION_RADIO_2);
						ProcessRadioIcon(m_aFrontEndSprites[MENUSPRITE_FEVER], MENU_X_LEFT_ALIGNED(210.0f), MENU_Y(nextYToUse), 3, HOVEROPTION_RADIO_3);
						ProcessRadioIcon(m_aFrontEndSprites[MENUSPRITE_VROCK], MENU_X_LEFT_ALIGNED(270.0f), MENU_Y(nextYToUse), 4, HOVEROPTION_RADIO_4);
						ProcessRadioIcon(m_aFrontEndSprites[MENUSPRITE_VCPR], MENU_X_LEFT_ALIGNED(320.0f), MENU_Y(nextYToUse), 5, HOVEROPTION_RADIO_5);
						ProcessRadioIcon(m_aFrontEndSprites[MENUSPRITE_ESPANTOSO], MENU_X_LEFT_ALIGNED(360.0f), MENU_Y(nextYToUse), 6, HOVEROPTION_RADIO_6);
						ProcessRadioIcon(m_aFrontEndSprites[MENUSPRITE_EMOTION], MENU_X_LEFT_ALIGNED(420.0f), MENU_Y(nextYToUse), 7, HOVEROPTION_RADIO_7);
						ProcessRadioIcon(m_aFrontEndSprites[MENUSPRITE_WAVE], MENU_X_LEFT_ALIGNED(480.0f), MENU_Y(nextYToUse), 8, HOVEROPTION_RADIO_8);

						if (DMAudio.IsMP3RadioChannelAvailable())
							ProcessRadioIcon(m_aFrontEndSprites[MENUSPRITE_MP3], MENU_X_LEFT_ALIGNED(540.0f), MENU_Y(nextYToUse), 9, HOVEROPTION_RADIO_9);

						nextYToUse += 70.0f;
					}
				}
			}
		}
		section++;
	}

	// TODO(Miami)
	/*
	switch (m_nCurrScreen) {
		case MENUPAGE_STATS:
		case MENUPAGE_CONTROLLER_PC:
		case MENUPAGE_SOUND_SETTINGS:
		case MENUPAGE_GRAPHICS_SETTINGS:
		case MENUPAGE_MOUSE_CONTROLS:
			DisplayHelperText(nil);
			break;
		case MENUPAGE_OPTIONS:
			if (m_nPrefsAudio3DProviderIndex == INVALID_AUDIO_PROVIDER && aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption].m_Action == MENUACTION_LOADRADIO)
				DisplayHelperText("FEA_NAH");
			break;
	}
	*/

	if (m_nCurrScreen == MENUPAGE_DELETING_IN_PROGRESS) {
		SmallMessageScreen("FEDL_WR");
	} else if (m_nCurrScreen == MENUPAGE_SAVING_IN_PROGRESS) {
		SmallMessageScreen("FESZ_WR");
	}
}

int
CMenuManager::GetNumOptionsCntrlConfigScreens(void)
{
	int number = 0;
	switch (m_nCurrScreen) {
#ifdef LEGACY_MENU_OPTIONS
		case MENUPAGE_CONTROLLER_PC_OLD3:
			number = 2;
			break;
		case MENUPAGE_CONTROLLER_DEBUG:
			number = 4;
			break;
#endif
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
#ifdef MORE_LANGUAGES
				if (CFont::IsJapanese())
					nextX += CFont::GetStringWidth_Jap(settingText) + bindingMargin;
				else
#endif
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
					static PauseModeTime lastWaitingTextFlash = 0;
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
					CFont::SetFontStyle(FONT_LOCALE(FONT_HEADING));
					CFont::SetColor(CRGBA(255, 255, 255, FadeIn(255)));
					if (m_bKeyChangeNotProcessed) {
						CFont::PrintString(MENU_X_LEFT_ALIGNED(275.0f), SCREEN_SCALE_FROM_BOTTOM(114.0f), TheText.Get("FET_CIG")); // BACKSPACE TO CLEAR - LMB,RETURN TO CHANGE
					} else {
						CFont::PrintString(MENU_X_LEFT_ALIGNED(275.0f), SCREEN_SCALE_FROM_BOTTOM(114.0f), TheText.Get("FET_RIG")); // SELECT A NEW CONTROL FOR THIS ACTION OR ESC TO CANCEL
					}
					
					CFont::SetRightJustifyOff();
					CFont::SetScale(MENU_X(SMALLESTTEXT_X_SCALE), MENU_Y(SMALLESTTEXT_Y_SCALE));
					CFont::SetFontStyle(FONT_LOCALE(FONT_BANK));
					if (!m_bKeyIsOK)
						DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_SUCCESS, 0);

					m_bKeyIsOK = true;
				} else {
					CFont::SetCentreOn();
					CFont::SetScale(MENU_X(SMALLESTTEXT_X_SCALE), MENU_Y(SMALLESTTEXT_Y_SCALE));
					CFont::SetFontStyle(FONT_LOCALE(FONT_HEADING));
					CFont::SetColor(CRGBA(255, 255, 255, FadeIn(255)));
					CFont::PrintString(MENU_X_LEFT_ALIGNED(275.0f), SCREEN_SCALE_FROM_BOTTOM(114.0f), TheText.Get("FET_CIG")); // BACKSPACE TO CLEAR - LMB,RETURN TO CHANGE
					CFont::SetRightJustifyOff();
					CFont::SetScale(MENU_X(SMALLESTTEXT_X_SCALE), MENU_Y(SMALLESTTEXT_Y_SCALE));
					CFont::SetFontStyle(FONT_LOCALE(FONT_BANK));
					m_bKeyIsOK = false;
					m_bKeyChangeNotProcessed = false;
				}
			} else if (optionIdx == m_nSelectedListRow) {
				CFont::SetCentreOn();
				CFont::SetScale(MENU_X(SMALLESTTEXT_X_SCALE), MENU_Y(SMALLESTTEXT_Y_SCALE));
				CFont::SetFontStyle(FONT_LOCALE(FONT_HEADING));
				CFont::SetColor(CRGBA(55, 55, 55, FadeIn(255)));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(275.0f), SCREEN_SCALE_FROM_BOTTOM(114.0f), TheText.Get("FET_EIG")); // CANNOT SET A CONTROL FOR THIS ACTION
				CFont::SetRightJustifyOff();
				CFont::SetScale(MENU_X(SMALLESTTEXT_X_SCALE), MENU_Y(SMALLESTTEXT_Y_SCALE));
				CFont::SetFontStyle(FONT_LOCALE(FONT_BANK));
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
			static PauseModeTime lastStateChange = 0;
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
	CFont::SetWrapx(MENU_X_RIGHT_ALIGNED(MENU_UNK_X_MARGIN));
	CFont::SetRightJustifyWrap(SCREEN_SCALE_X(MENU_UNK_WIDTH));

	PREPARE_MENU_HEADER

	switch (m_ControlMethod) {
		case CONTROL_STANDARD:
			CFont::SetColor(CRGBA(30, 30, 30, FadeIn(255)));
			CFont::PrintString(SCREEN_SCALE_FROM_RIGHT(MENUHEADER_POS_X + 7.f), SCREEN_SCALE_Y(MENUHEADER_POS_Y + 7.f),
				TheText.Get(aScreens[m_nCurrScreen].m_ScreenName));

			PREPARE_MENU_HEADER
			CFont::PrintString(SCREEN_SCALE_FROM_RIGHT(MENUHEADER_POS_X), SCREEN_SCALE_Y(MENUHEADER_POS_Y),
				TheText.Get(aScreens[m_nCurrScreen].m_ScreenName));
			break;
		case CONTROL_CLASSIC:
			CFont::SetColor(CRGBA(30, 30, 30, FadeIn(255)));
			CFont::PrintString(SCREEN_SCALE_FROM_RIGHT(MENUHEADER_POS_X + 7.f), SCREEN_SCALE_Y(MENUHEADER_POS_Y + 7.f),
				TheText.Get("FET_CTI"));

			PREPARE_MENU_HEADER
			CFont::PrintString(SCREEN_SCALE_FROM_RIGHT(MENUHEADER_POS_X), SCREEN_SCALE_Y(MENUHEADER_POS_Y),
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
	CFont::SetFontStyle(FONT_LOCALE(FONT_HEADING));
	CFont::SetScale(MENU_X(MENUACTION_SCALE_MULT), MENU_Y(MENUACTION_SCALE_MULT));
	CFont::SetRightJustifyOff();
	CFont::PrintString(MENU_X_LEFT_ALIGNED(CONTSETUP_COLUMN_1_X), MENU_Y(CONTSETUP_LIST_TOP), TheText.Get("FET_CAC"));
	CFont::PrintString(MENU_X_LEFT_ALIGNED(CONTSETUP_COLUMN_2_X), MENU_Y(CONTSETUP_LIST_TOP), TheText.Get("FET_CFT"));
	CFont::PrintString(MENU_X_LEFT_ALIGNED(CONTSETUP_COLUMN_3_X), MENU_Y(CONTSETUP_LIST_TOP), TheText.Get("FET_CCR"));
	CFont::SetRightJustifyOff();
	CFont::SetScale(MENU_X_LEFT_ALIGNED(SMALLESTTEXT_X_SCALE), MENU_Y(SMALLESTTEXT_Y_SCALE));
	CFont::SetFontStyle(FONT_LOCALE(FONT_BANK));
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
	CFont::SetFontStyle(FONT_LOCALE(FONT_HEADING));
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
	CSprite2d::InitPerFrame();
	CFont::InitPerFrame();
	SetFrontEndRenderStates();
	m_NoEmptyBinding = true;

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

	if (m_menuTransitionProgress == 255 && m_nMenuFadeAlpha == 255)
		bMenuChangeOngoing = false;

	DrawBackground(false);
}

#ifdef PS2_LIKE_MENU
void
CMenuManager::DrawBackground()
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
			DrawStandardMenus();
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

			CFont::SetFontStyle(FONT_LOCALE(FONT_BANK));
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
			m_aFrontEndSprites[MENUSPRITE_MOUSET].Draw(shad, CRGBA(100, 100, 100, 50));
			RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)FALSE);
			m_aFrontEndSprites[MENUSPRITE_MOUSET].Draw(mouse, CRGBA(255, 255, 255, 255));
		}else{
			m_aFrontEndSprites[MENUSPRITE_MOUSE].Draw(shad, CRGBA(100, 100, 100, 50));
			RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)FALSE);
			m_aFrontEndSprites[MENUSPRITE_MOUSE].Draw(mouse, CRGBA(255, 255, 255, 255));
		}
	}
}
#else

// --MIAMI: Done except commented things
void
CMenuManager::DrawBackground(bool transitionCall)
{
	if (!m_bSpritesLoaded)
		return;

	SetFrontEndRenderStates();

	if (m_menuTransitionProgress < 255) {
		CSprite2d::DrawRect(CRect(0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT), CRGBA(0, 0, 0, 255));
	}

	if (m_nMenuFadeAlpha != 0) {

		if (m_nMenuFadeAlpha < 255) {

			menuBg.Translate(m_nMenuFadeAlpha);
			SetFrontEndRenderStates();
			m_aFrontEndSprites[MENUSPRITE_BACKGROUND].Draw(CRect(0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT), CRGBA(255, 255, 255, FadeIn(255)));
			if (m_nCurrScreen == MENUPAGE_MAP)
				PrintMap();

			// Left border
			CSprite2d::Draw2DPolygon(SCREEN_STRETCH_X(menuBg.bottomLeft_x), SCREEN_STRETCH_Y(menuBg.bottomLeft_y), 0.0f, SCREEN_HEIGHT,
				SCREEN_STRETCH_X(menuBg.topLeft_x), SCREEN_STRETCH_Y(menuBg.topLeft_y), 0.0f, 0.0f, CRGBA(0, 0, 0, 255));

			// Top border
			CSprite2d::Draw2DPolygon(SCREEN_STRETCH_X(menuBg.topRight_x), SCREEN_STRETCH_Y(menuBg.topRight_y), 
				SCREEN_STRETCH_X(menuBg.topLeft_x), SCREEN_STRETCH_Y(menuBg.topLeft_y), SCREEN_WIDTH, 0.0f, 0.0f, 0.0f, CRGBA(0, 0, 0, 255));

			// Bottom border
			CSprite2d::Draw2DPolygon(SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, SCREEN_HEIGHT, SCREEN_STRETCH_X(menuBg.bottomRight_x), SCREEN_STRETCH_Y(menuBg.bottomRight_y),
				SCREEN_STRETCH_X(menuBg.bottomLeft_x), SCREEN_STRETCH_Y(menuBg.bottomLeft_y), CRGBA(0, 0, 0, 255));

			// Right border
			CSprite2d::Draw2DPolygon(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_STRETCH_X(menuBg.bottomRight_x), SCREEN_STRETCH_Y(menuBg.bottomRight_y),
				SCREEN_WIDTH, 0.0f, SCREEN_STRETCH_X(menuBg.topRight_x), SCREEN_STRETCH_Y(menuBg.topRight_y), CRGBA(0, 0, 0, 255));
		} else {
			m_nMenuFadeAlpha = 255;
			m_menuTransitionProgress = 255;
			m_aFrontEndSprites[MENUSPRITE_BACKGROUND].Draw(CRect(0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT), CRGBA(255, 255, 255, FadeIn(255)));
			if (m_nCurrScreen == MENUPAGE_MAP)
				PrintMap();

			// Left border
			CSprite2d::Draw2DPolygon(SCREEN_STRETCH_X(menuBg.bottomLeft_x), SCREEN_STRETCH_Y(menuBg.bottomLeft_y), 0.0f, SCREEN_HEIGHT,
				SCREEN_STRETCH_X(menuBg.topLeft_x), SCREEN_STRETCH_Y(menuBg.topLeft_y), 0.0f, 0.0f, CRGBA(0, 0, 0, 255));

			// Top border
			CSprite2d::Draw2DPolygon(SCREEN_STRETCH_X(menuBg.topRight_x), SCREEN_STRETCH_Y(menuBg.topRight_y),
				SCREEN_STRETCH_X(menuBg.topLeft_x), SCREEN_STRETCH_Y(menuBg.topLeft_y), SCREEN_WIDTH, 0.0f, 0.0f, 0.0f, CRGBA(0, 0, 0, 255));

			// Bottom border
			CSprite2d::Draw2DPolygon(SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, SCREEN_HEIGHT, SCREEN_STRETCH_X(menuBg.bottomRight_x), SCREEN_STRETCH_Y(menuBg.bottomRight_y),
				SCREEN_STRETCH_X(menuBg.bottomLeft_x), SCREEN_STRETCH_Y(menuBg.bottomLeft_y), CRGBA(0, 0, 0, 255));

			// Right border
			CSprite2d::Draw2DPolygon(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_STRETCH_X(menuBg.bottomRight_x), SCREEN_STRETCH_Y(menuBg.bottomRight_y),
				SCREEN_WIDTH, 0.0f, SCREEN_STRETCH_X(menuBg.topRight_x), SCREEN_STRETCH_Y(menuBg.topRight_y), CRGBA(0, 0, 0, 255));
		}
	} else {
		menuBg.SaveCurrentCoors();
		switch (m_nCurrScreen) {
			case MENUPAGE_STATS:
				menuBg.topLeft_x = 70.0f;
				menuBg.topLeft_y = 75.0f;
				menuBg.topRight_x = 550.0f;
				menuBg.topRight_y = 16.0f;
				menuBg.bottomLeft_x = 74.0f;
				menuBg.bottomLeft_y = 354.0f;
				menuBg.bottomRight_x = 581.0f;
				menuBg.bottomRight_y = 340.0f;
				break;
			case MENUPAGE_SOUND_SETTINGS:
				menuBg.topLeft_x = 26.0f;
				menuBg.topLeft_y = 59.0f;
				menuBg.topRight_x = 629.0f;
				menuBg.topRight_y = 29.0f;
				menuBg.bottomLeft_x = 15.0f;
				menuBg.bottomLeft_y = 438.0f;
				menuBg.bottomRight_x = 610.0f;
				menuBg.bottomRight_y = 410.0f;
				break;
			case MENUPAGE_SKIN_SELECT:
			case MENUPAGE_KEYBOARD_CONTROLS:
				menuBg.topLeft_x = 14.0f;
				menuBg.topLeft_y = 39.0f;
				menuBg.topRight_x = 636.0f;
				menuBg.topRight_y = 29.0f;
				menuBg.bottomLeft_x = 15.0f;
				menuBg.bottomLeft_y = 426.0f;
				menuBg.bottomRight_x = 630.0f;
				menuBg.bottomRight_y = 398.0f;
				break;
			case MENUPAGE_BRIEFS:
			case MENUPAGE_GRAPHICS_SETTINGS:
			case MENUPAGE_MAP:
			case MENUPAGE_CHOOSE_LOAD_SLOT:
			case MENUPAGE_CHOOSE_DELETE_SLOT:
			case MENUPAGE_CHOOSE_SAVE_SLOT:
			case MENUPAGE_MOUSE_CONTROLS:
				menuBg.topLeft_x = 26.0f;
				menuBg.topLeft_y = 59.0f;
				menuBg.topRight_x = 629.0f;
				menuBg.topRight_y = 29.0f;
				menuBg.bottomLeft_x = 15.0f;
				menuBg.bottomLeft_y = 426.0f;
				menuBg.bottomRight_x = 610.0f;
				menuBg.bottomRight_y = 398.0f;
				break;
			default:
				menuBg.topLeft_x = CGeneral::GetRandomNumber() % 40 + 65;
				menuBg.topLeft_y = CGeneral::GetRandomNumber() % 40 + 21;
				menuBg.topRight_x = CGeneral::GetRandomNumber() % 40 + 568;
				menuBg.topRight_y = CGeneral::GetRandomNumber() % 40 + 44;
				menuBg.bottomLeft_x = CGeneral::GetRandomNumber() % 40 + 36;
				menuBg.bottomLeft_y = CGeneral::GetRandomNumber() % 40 + 382;
				menuBg.bottomRight_x = CGeneral::GetRandomNumber() % 40 + 593;
				menuBg.bottomRight_y = CGeneral::GetRandomNumber() % 40 + 342;
				break;
		}

		menuBg.UpdateMultipliers();
		if (m_menuTransitionProgress == 255)
			m_nOptionHighlightTransitionBlend = 0;
	}

	static PauseModeTime LastFade = 0;
	static uint8 forceFadeInCounter = 0;

	if (m_nMenuFadeAlpha >= 255) {
		if (m_nMenuFadeAlpha > 255)
			m_nMenuFadeAlpha = 255;
	} else {
		if (CTimer::GetTimeInMillisecondsPauseMode() - LastFade > 30
#ifndef FIX_HIGH_FPS_BUGS_ON_FRONTEND // Dirty dirty hack
			|| forceFadeInCounter > 30
#endif
			) {
			m_nMenuFadeAlpha += 20;
			m_menuTransitionProgress = Min(m_menuTransitionProgress + 20, 255);
			LastFade = CTimer::GetTimeInMillisecondsPauseMode();
		}
		forceFadeInCounter++;
	}

	if (!transitionCall && m_menuTransitionProgress == 255) {
		int actualAlpha = m_nMenuFadeAlpha;
		if (actualAlpha < 255) {
			int actualScreen = m_nCurrScreen;
			SetFrontEndRenderStates();
			m_nCurrScreen = m_nPrevScreen;
			m_nMenuFadeAlpha = 255 - m_nMenuFadeAlpha;
			switch (m_nCurrScreen) {
				case MENUPAGE_SKIN_SELECT:
					CMenuManager::DrawPlayerSetupScreen();
					break;
				case MENUPAGE_KEYBOARD_CONTROLS:
					CMenuManager::DrawControllerSetupScreen();
					break;
				case MENUPAGE_OUTRO:
					CMenuManager::DrawQuitGameScreen();
					break;
				default:
					CMenuManager::DrawStandardMenus(false);
					break;
			}
			m_nCurrScreen = actualScreen;
			m_nMenuFadeAlpha = actualAlpha;
		}
	}

	switch (m_nCurrScreen) {
		case MENUPAGE_SKIN_SELECT:
			DrawPlayerSetupScreen();
			break;
		case MENUPAGE_KEYBOARD_CONTROLS:
			DrawControllerSetupScreen();
			break;
		case MENUPAGE_OUTRO:
			CMenuManager::DrawQuitGameScreen();
			break;
		default:
			DrawStandardMenus(true);
			break;
	}

	CFont::DrawFonts();
	SetFrontEndRenderStates();

	if (m_nCurrScreen != MENUPAGE_OUTRO)
		if (m_menuTransitionProgress == 255) {
			m_aFrontEndSprites[MENUSPRITE_VCLOGO].Draw(CRect(MENU_X(27.0f), MENU_Y(8.0f), MENU_X(157.0f), MENU_Y(138.0f)), CRGBA(255, 255, 255, 255));
		} else {
			m_aFrontEndSprites[MENUSPRITE_VCLOGO].Draw(CRect(MENU_X(27.0f), MENU_Y(8.0f), MENU_X(157.0f), MENU_Y(138.0f)), CRGBA(255, 255, 255, FadeIn(255)));
		}

	if (m_ShowEmptyBindingError) {
		static PauseModeTime lastBindingError = CTimer::GetTimeInMillisecondsPauseMode();
		static bool bindingErrorShown = false;
		if (bindingErrorShown) {
			lastBindingError = CTimer::GetTimeInMillisecondsPauseMode();
			bindingErrorShown = false;
		}
		SmallMessageScreen("FEC_ERI");
		CFont::DrawFonts();
		if (CTimer::GetTimeInMillisecondsPauseMode() - lastBindingError > 4000) {
			m_ShowEmptyBindingError = false;
			bindingErrorShown = true;
		}
	}

	if (m_bShowMouse) {
		CRect mouse(0.0f, 0.0f, MENU_X(40.0f), MENU_Y(40.0f));
		CRect shad(MENU_X(10.0f), MENU_Y(3.0f), MENU_X(55.0f), MENU_Y(43.0f));

		mouse.Translate(m_nMousePosX, m_nMousePosY);
		shad.Translate(m_nMousePosX, m_nMousePosY);
		m_aFrontEndSprites[MENUSPRITE_MOUSE].Draw(shad, CRGBA(100, 100, 100, 50));
		m_aFrontEndSprites[MENUSPRITE_MOUSE].Draw(mouse, CRGBA(255, 255, 255, 255));
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
	CFont::SetWrapx(MENU_X_RIGHT_ALIGNED(MENU_UNK_X_MARGIN));
	CFont::SetRightJustifyWrap(SCREEN_SCALE_X(MENU_UNK_WIDTH));

	PREPARE_MENU_HEADER

	CFont::PrintString(SCREEN_SCALE_FROM_RIGHT(MENUHEADER_POS_X), SCREEN_SCALE_FROM_BOTTOM(MENUHEADER_POS_Y), TheText.Get("FET_PS"));

	// lstrcpy's changed with strcpy

	if (!m_bSkinsEnumerated) {
		OutputDebugString("Enumerating skin filenames from skins...");
		m_pSkinListHead.nextSkin = nil;
		m_pSelectedSkin = &m_pSkinListHead;
		m_pSelectedSkin->nextSkin = new tSkinInfo;
		m_pSelectedSkin = m_pSelectedSkin->nextSkin;
		m_pSelectedSkin->skinId = 0;
		strcpy(m_pSelectedSkin->skinNameOriginal, DEFAULT_SKIN_NAME);
		strcpy(m_pSelectedSkin->skinNameDisplayed, UnicodeToAscii(TheText.Get("FET_DSN")));
		int nextSkinId = 1;
		m_pSelectedSkin->nextSkin = nil;

		WIN32_FIND_DATA FindFileData;
		SYSTEMTIME SystemTime;
		HANDLE handle = FindFirstFile("skins\\*.bmp", &FindFileData);
		for (int i = 1; handle != INVALID_HANDLE_VALUE && i; i = FindNextFile(handle, &FindFileData)) {
			if (strncmp(FindFileData.cFileName, DEFAULT_SKIN_NAME, 5) != 0) {
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
				if (!strncmp(&m_pSelectedSkin->skinNameDisplayed[k], "�", 1))
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
	CFont::SetFontStyle(FONT_LOCALE(FONT_HEADING));
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
	CFont::SetFontStyle(FONT_LOCALE(FONT_BANK));
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
			m_aFrontEndSprites[MENUSPRITE_UPON].Draw(CRect(MENU_X_RIGHT_ALIGNED(PLAYERSETUP_LIST_RIGHT - 2), MENU_Y(PLAYERSETUP_LIST_TOP),
				MENU_X_RIGHT_ALIGNED(PLAYERSETUP_LIST_RIGHT - 2 - PLAYERSETUP_SCROLLBUTTON_TXD_DIMENSION), MENU_Y(PLAYERSETUP_LIST_TOP + PLAYERSETUP_SCROLLBUTTON_TXD_DIMENSION)),
				CRGBA(255, 255, 255, FadeIn(255)));
#else
			m_aFrontEndSprites[MENUSPRITE_UPON].Draw(CRect(MENU_X_RIGHT_ALIGNED(PLAYERSETUP_LIST_RIGHT - 2), MENU_Y(PLAYERSETUP_LIST_TOP),
				MENU_X_RIGHT_ALIGNED(-20.0f), MENU_Y(PLAYERSETUP_LIST_TOP + 58)),
				CRGBA(255, 255, 255, FadeIn(255)));
#endif
		} else {
#ifdef FIX_BUGS
			m_aFrontEndSprites[MENUSPRITE_UPOFF].Draw(CRect(MENU_X_RIGHT_ALIGNED(PLAYERSETUP_LIST_RIGHT - 3), MENU_Y(PLAYERSETUP_LIST_TOP),
				MENU_X_RIGHT_ALIGNED(PLAYERSETUP_LIST_RIGHT - 3 - PLAYERSETUP_SCROLLBUTTON_TXD_DIMENSION), MENU_Y(PLAYERSETUP_LIST_TOP + PLAYERSETUP_SCROLLBUTTON_TXD_DIMENSION)),
				CRGBA(255, 255, 255, FadeIn(255)));
#else
			m_aFrontEndSprites[MENUSPRITE_UPOFF].Draw(CRect(MENU_X_RIGHT_ALIGNED(PLAYERSETUP_LIST_RIGHT - 3), MENU_Y(PLAYERSETUP_LIST_TOP),
				MENU_X_RIGHT_ALIGNED(-21.0f), MENU_Y(PLAYERSETUP_LIST_TOP + 58)),
				CRGBA(255, 255, 255, FadeIn(255)));
#endif
		}

		if (m_nHoverOption == HOVEROPTION_CLICKED_SCROLL_DOWN) {
#ifdef FIX_BUGS
			m_aFrontEndSprites[MENUSPRITE_DOWNON].Draw(CRect(MENU_X_RIGHT_ALIGNED(PLAYERSETUP_LIST_RIGHT - 2), SCREEN_SCALE_FROM_BOTTOM(PLAYERSETUP_LIST_BOTTOM + PLAYERSETUP_SCROLLBUTTON_HEIGHT + 1),
				MENU_X_RIGHT_ALIGNED(PLAYERSETUP_LIST_RIGHT - 2 - PLAYERSETUP_SCROLLBUTTON_TXD_DIMENSION), SCREEN_SCALE_FROM_BOTTOM(PLAYERSETUP_LIST_BOTTOM + PLAYERSETUP_SCROLLBUTTON_HEIGHT + 1 - PLAYERSETUP_SCROLLBUTTON_TXD_DIMENSION)),
				CRGBA(255, 255, 255, FadeIn(255)));
#else
			m_aFrontEndSprites[MENUSPRITE_DOWNON].Draw(CRect(MENU_X_RIGHT_ALIGNED(PLAYERSETUP_LIST_RIGHT - 2), SCREEN_SCALE_FROM_BOTTOM(141.0f),
					MENU_X_RIGHT_ALIGNED(-20.0f), SCREEN_SCALE_FROM_BOTTOM(83.0f)),
				CRGBA(255, 255, 255, FadeIn(255)));
#endif
		} else {
#ifdef FIX_BUGS
			m_aFrontEndSprites[MENUSPRITE_DOWNOFF].Draw(CRect(MENU_X_RIGHT_ALIGNED(PLAYERSETUP_LIST_RIGHT - 3), SCREEN_SCALE_FROM_BOTTOM(PLAYERSETUP_LIST_BOTTOM + PLAYERSETUP_SCROLLBUTTON_HEIGHT + 1),
				MENU_X_RIGHT_ALIGNED(PLAYERSETUP_LIST_RIGHT - 3 - PLAYERSETUP_SCROLLBUTTON_TXD_DIMENSION), SCREEN_SCALE_FROM_BOTTOM(PLAYERSETUP_LIST_BOTTOM + PLAYERSETUP_SCROLLBUTTON_HEIGHT + 1 - PLAYERSETUP_SCROLLBUTTON_TXD_DIMENSION)),
				CRGBA(255, 255, 255, FadeIn(255)));
#else
			m_aFrontEndSprites[MENUSPRITE_DOWNOFF].Draw(CRect(MENU_X_RIGHT_ALIGNED(PLAYERSETUP_LIST_RIGHT - 3), SCREEN_SCALE_FROM_BOTTOM(141.0f),
				MENU_X_RIGHT_ALIGNED(-21.0f), SCREEN_SCALE_FROM_BOTTOM(83.0f)),
				CRGBA(255, 255, 255, FadeIn(255)));
#endif

		}
		CPlayerSkin::RenderFrontendSkinEdit();

		// Big apply button
		if (strcmp(m_aSkinName, m_PrefsSkinFile) != 0) {
			CFont::SetFontStyle(FONT_LOCALE(FONT_HEADING));
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
		CFont::SetFontStyle(FONT_LOCALE(FONT_HEADING));

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
	CFont::SetFontStyle(FONT_LOCALE(FONT_HEADING));
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
	return Min(m_nMenuFadeAlpha, alpha);
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
#ifdef LEGACY_MENU_OPTIONS
		case MENUPAGE_CONTROLLER_PC_OLD3:
			number = 34;
			break;
		case MENUPAGE_CONTROLLER_DEBUG:
			number = 35;
			break;
#endif
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
		CGame::russianGame = false;
		CGame::japaneseGame = false;
		switch (m_PrefsLanguage) {
		case LANGUAGE_POLISH:
			CFont::ReloadFonts(FONT_LANGSET_POLISH);
			break;
		case LANGUAGE_RUSSIAN:
			CFont::ReloadFonts(FONT_LANGSET_RUSSIAN);
			break;
		case LANGUAGE_JAPANESE:
			CFont::ReloadFonts(FONT_LANGSET_JAPANESE);
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
		case LANGUAGE_JAPANESE:
			CGame::japaneseGame = true;
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

	field_F0 = SCREEN_STRETCH_X(178.0f); // TODO(Miami)
	CTimer::Stop();

	CStreaming::MakeSpaceFor(350 * CDSTREAM_SECTOR_SIZE); // twice of it in mobile
	CStreaming::ImGonnaUseStreamingMemory();
	CGame::TidyUpMemory(false, true);
	CTxdStore::PushCurrentTxd();
	int frontendTxdSlot1 = CTxdStore::FindTxdSlot("frontend1");

	if(frontendTxdSlot1 == -1)
		frontendTxdSlot1 = CTxdStore::AddTxdSlot("frontend1");

	printf("LOAD frontend1\n");
	CTxdStore::LoadTxd(frontendTxdSlot1, "MODELS/FRONTEN1.TXD");
	CTxdStore::AddRef(frontendTxdSlot1);
	CTxdStore::SetCurrentTxd(frontendTxdSlot1);

	for (int i = 0; i < 3; i++) {
		m_aFrontEndSprites[i].SetTexture(FrontendFilenames[i][0], FrontendFilenames[i][1]);
		m_aFrontEndSprites[i].SetAddressing(rwTEXTUREADDRESSBORDER);
	}

	CTxdStore::PopCurrentTxd();
	CStreaming::IHaveUsedStreamingMemory();

	if (!m_OnlySaveMenu) {
		CStreaming::MakeSpaceFor(692 * CDSTREAM_SECTOR_SIZE); // twice of it in mobile
		CStreaming::ImGonnaUseStreamingMemory();
		CTxdStore::PushCurrentTxd();

		int frontendTxdSlot2 = CTxdStore::FindTxdSlot("frontend2");

		if (frontendTxdSlot2 == -1)
			frontendTxdSlot2 = CTxdStore::AddTxdSlot("frontend2");

		printf("LOAD frontend2\n");
		CTxdStore::LoadTxd(frontendTxdSlot2, "MODELS/FRONTEN2.TXD");
		CTxdStore::AddRef(frontendTxdSlot2);
		CTxdStore::SetCurrentTxd(frontendTxdSlot2);

		for (int i = 3; i < NUM_MENU_SPRITES; i++) {
			m_aFrontEndSprites[i].SetTexture(FrontendFilenames[i][0], FrontendFilenames[i][1]);
			m_aFrontEndSprites[i].SetAddressing(rwTEXTUREADDRESSBORDER);
		}

		CTxdStore::PopCurrentTxd();
		CStreaming::IHaveUsedStreamingMemory();
	}

	m_bSpritesLoaded = true;
	CTimer::Update();
}

void
CMenuManager::LoadSettings()
{
	CFileMgr::SetDirMyDocuments();
	int fileHandle = CFileMgr::OpenFile("gta3.set", "r");

	int32 prevLang = m_PrefsLanguage;
#ifdef GTA3_1_1_PATCH
	CMBlur::BlurOn = (_dwOperatingSystemVersion != OS_WIN98);
#else
	CMBlur::BlurOn = true;
#endif
	MousePointerStateHelper.bInvertVertically = true;

	// 50 is silly
	char Ver[50];

	if (fileHandle) {
		CFileMgr::Read(fileHandle, Ver, 29);

		if (strncmp(Ver, TopLineEmptyFile, sizeof(TopLineEmptyFile) - 1)) {
			CFileMgr::Seek(fileHandle, 0, 0);
			ControlsManager.LoadSettings(fileHandle);
#ifdef IMPROVED_VIDEOMODE
			CFileMgr::Read(fileHandle, (char*)&m_nPrefsWidth, sizeof(m_nPrefsWidth));
			CFileMgr::Read(fileHandle, (char*)&m_nPrefsHeight, sizeof(m_nPrefsHeight));
			CFileMgr::Read(fileHandle, (char*)&m_nPrefsDepth, sizeof(m_nPrefsDepth));
			CFileMgr::Read(fileHandle, (char*)&m_nPrefsWindowed, sizeof(m_nPrefsWindowed));
			CFileMgr::Read(fileHandle, (char*)&m_nPrefsSubsystem, sizeof(m_nPrefsSubsystem));
			if(m_nPrefsWindowed != 0 && m_nPrefsWindowed != 1){
				// garbage data from vanilla settings file
				// let skeleton find something
				m_nPrefsWidth = 0;
				m_nPrefsHeight = 0;
				m_nPrefsDepth = 0;
				m_nPrefsWindowed = 0;
				m_nPrefsSubsystem = 0;
			}
			m_nSelectedScreenMode = m_nPrefsWindowed;
#else
			CFileMgr::Read(fileHandle, gString, 20);
#endif
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
#ifdef FREE_CAM
			CFileMgr::Read(fileHandle, (char*)&TheCamera.bFreeCam, 1);
#endif
		}
	}

	CFileMgr::CloseFile(fileHandle);
	CFileMgr::SetDir("");

	m_PrefsVsync = m_PrefsVsyncDisp;
	CRenderer::ms_lodDistScale = m_PrefsLOD;

	if (m_nPrefsAudio3DProviderIndex == INVALID_AUDIO_PROVIDER)
		m_nPrefsAudio3DProviderIndex = -2;

	m_lastWorking3DAudioProvider = m_nPrefsAudio3DProviderIndex;

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
		strcpy(m_PrefsSkinFile, DEFAULT_SKIN_NAME);
		strcpy(m_aSkinName, DEFAULT_SKIN_NAME);
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
#ifdef IMPROVED_VIDEOMODE
		CFileMgr::Write(fileHandle, (char*)&m_nPrefsWidth, sizeof(m_nPrefsWidth));
		CFileMgr::Write(fileHandle, (char*)&m_nPrefsHeight, sizeof(m_nPrefsHeight));
		CFileMgr::Write(fileHandle, (char*)&m_nPrefsDepth, sizeof(m_nPrefsDepth));
		CFileMgr::Write(fileHandle, (char*)&m_nPrefsWindowed, sizeof(m_nPrefsWindowed));
		CFileMgr::Write(fileHandle, (char*)&m_nPrefsSubsystem, sizeof(m_nPrefsSubsystem));
#else
		CFileMgr::Write(fileHandle, RubbishString, 20);
#endif
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
#ifdef FREE_CAM
		CFileMgr::Write(fileHandle, (char*)&TheCamera.bFreeCam, 1);
#endif
	}
	m_lastWorking3DAudioProvider = m_nPrefsAudio3DProviderIndex;

	CFileMgr::CloseFile(fileHandle);
	CFileMgr::SetDir("");
}

void
CMenuManager::MessageScreen(const char *text, bool blackBg)
{
	CSprite2d *splash = LoadSplash(nil);
	if (!DoRWStuffStartOfFrame(0, 0, 0, 0, 0, 0, 255))
		return;

	CSprite2d::SetRecipNearClip();
	CSprite2d::InitPerFrame();
	CFont::InitPerFrame();
	DefinedState();
	// CRGBA unused(255, 255, 255, 255);
	if (blackBg) {
		CSprite2d::DrawRect(CRect(0, SCREEN_HEIGHT, SCREEN_WIDTH, 0), CRGBA(0, 0, 0, 255));
	}

	m_nMenuFadeAlpha = 255;
	SmallMessageScreen(text);
	CFont::DrawFonts();
	DoRWStuffEndOfFrame();
}

// TODO(Miami)
void
CMenuManager::SmallMessageScreen(const char* text)
{
	CFont::SetBackgroundOff();
	CFont::SetPropOn();
	CFont::SetJustifyOn();
	CFont::SetBackGroundOnlyTextOn();
	CSprite2d::DrawRect(CRect(SCREEN_SCALE_X(95.0f), SCREEN_SCALE_FROM_BOTTOM(165.0f), SCREEN_SCALE_FROM_RIGHT(95.0f), SCREEN_SCALE_Y(115.0f)), CRGBA(50, 50, 50, FadeIn(210)));
	CFont::SetFontStyle(FONT_LOCALE(FONT_BANK));
	CFont::SetCentreSize(SCREEN_SCALE_X(430.0f));
	CFont::SetCentreOn();
	CFont::SetColor(CRGBA(255, 217, 106, FadeIn(255)));
	CFont::SetScale(SCREEN_SCALE_X(SMALLTEXT_X_SCALE), SCREEN_SCALE_Y(SMALLTEXT_Y_SCALE));
	CFont::PrintString(SCREEN_SCALE_X(320.0f), SCREEN_SCALE_Y(170.0f), TheText.Get(text));
}

void
CMenuManager::PrintBriefs()
{
	CFont::SetColor(CRGBA(235, 170, 50, FadeIn(255)));
	CFont::SetFontStyle(FONT_LOCALE(FONT_BANK));
	CFont::SetRightJustifyOff();
	CFont::SetScale(MENU_X(MENU_TEXT_SIZE_X * 0.7), MENU_Y(MENU_TEXT_SIZE_Y * 0.9)); // second mulipliers are double, idk why

	float nextY = BRIEFS_TOP_MARGIN;
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
			CFont::PrintString(MENU_X_LEFT_ALIGNED(BRIEFS_LINE_X), nextY, gUString);
			nextY += MENU_Y(BRIEFS_LINE_HEIGHT);
		}
	}

#ifdef PS2_LIKE_MENU
	CFont::SetDropShadowPosition(0);
#endif
}

void
CMenuManager::PrintStats()
{
	int rowNum = ConstructStatLine(99999);
#ifdef GTA3_1_1_PATCH
	CFont::SetFontStyle(FONT_LOCALE(FONT_BANK));
#endif
	CFont::SetScale(MENU_X(MENU_TEXT_SIZE_X * 0.7), MENU_Y(MENU_TEXT_SIZE_Y * 0.9)); // second mulipliers are double, idk why
	float nextYChange, y, alphaMult;

	// Scroll stats with mouse
#ifdef SCROLLABLE_STATS_PAGE
	static float scrollY = 0;
	static PauseModeTime lastChange = m_LastScreenSwitch;
	if (CPad::GetPad(0)->GetLeftMouse()) {
		scrollY += (m_nMouseOldPosY - m_nMousePosY);
		lastChange = CTimer::GetTimeInMillisecondsPauseMode();
	} else {
		scrollY += MENU_Y(STATS_SLIDE_Y_PER_SECOND) / 1000.0f * (CTimer::GetTimeInMillisecondsPauseMode() - lastChange);
		lastChange = CTimer::GetTimeInMillisecondsPauseMode();
	}
#else
	// MENU_Y(30.0f) per second
	float scrollY = MENU_Y(STATS_SLIDE_Y_PER_SECOND) * (CTimer::GetTimeInMillisecondsPauseMode() - m_LastScreenSwitch) / 1000.0f;
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
	CFont::PrintString(nextX, MENU_Y(STATS_RATING_Y), TheText.Get("CRIMRA"));
#ifdef MORE_LANGUAGES
	if (CFont::IsJapanese())
		nextX += MENU_X(10.0f) + CFont::GetStringWidth_Jap(TheText.Get("CRIMRA"));
	else
#endif
		nextX += MENU_X(10.0f) + CFont::GetStringWidth(TheText.Get("CRIMRA"), true);
	UnicodeStrcpy(gUString, CStats::FindCriminalRatingString());
	CFont::PrintString(nextX, MENU_Y(STATS_RATING_Y), gUString);
#ifdef MORE_LANGUAGES
	if (CFont::IsJapanese())
		nextX += MENU_X(6.0f) + CFont::GetStringWidth_Jap(gUString);
	else
#endif
		nextX += MENU_X(6.0f) + CFont::GetStringWidth(gUString, true);
	sprintf(gString, "%d", CStats::FindCriminalRatingNumber());
	AsciiToUnicode(gString, gUString);
	CFont::PrintString(nextX, MENU_Y(STATS_RATING_Y), gUString);

	// ::Draw already does that.
	/*
	PREPARE_MENU_HEADER
	CFont::PrintString(SCREEN_SCALE_FROM_RIGHT(MENUHEADER_POS_X), SCREEN_SCALE_FROM_BOTTOM(MENUHEADER_POS_Y), TheText.Get(aScreens[m_nCurrScreen].m_ScreenName));
	*/
	CFont::SetScale(MENU_X(MENU_TEXT_SIZE_X), MENU_Y(MENU_TEXT_SIZE_Y));
}

// --MIAMI: Done
void
CMenuManager::Process(void)
{
	if (TheCamera.GetScreenFadeStatus() != FADE_0)
		return;

	InitialiseChangedLanguageSettings();

	if (m_bMenuActive) {
		ProcessButtonPresses();
		ProcessFileActions();
		DMAudio.Service();

		// Game calls some texture pool cleanup functions in here
	}

	SwitchMenuOnAndOff();
}

// TODO(Miami)
void
CMenuManager::ProcessButtonPresses(void)
{
	if (m_nCurrScreen == MENUPAGE_OUTRO)
		return;

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
			}
			else if (!m_bKeyChangeNotProcessed) {
				if (*pControlEdit != rsNULL || MouseButtonJustClicked || JoyButtonJustClicked)
					CheckCodesForControls(TypeOfControl);

				field_159 = true;
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

	if (pEditString || pControlEdit)
		return;

	bool goBack = false;
	bool optionSelected = false;
	bool goUp = false;
	bool goDown = false;
#ifdef TIDY_UP_PBP
	bool assumeIncrease = false;
#endif

#ifdef USE_DEBUG_SCRIPT_LOADER
	if (m_nCurrScreen == MENUPAGE_START_MENU || m_nCurrScreen == MENUPAGE_NEW_GAME || m_nCurrScreen == MENUPAGE_NEW_GAME_RELOAD) {
#ifdef RW_GL3
		if (glfwGetKey(PSGLOBAL(window), GLFW_KEY_R) == GLFW_PRESS) {
			scriptToLoad = 1;
			DoSettingsBeforeStartingAGame();
			return;
		}
#elif defined _WIN32
		if (GetAsyncKeyState('R') & 0x8000) {
			scriptToLoad = 1;
			DoSettingsBeforeStartingAGame();
			return;
		}
#endif
	}
#endif

	if (!m_bShowMouse && (m_nMouseOldPosX != m_nMousePosX || m_nMouseOldPosY != m_nMousePosY)) {
		m_bShowMouse = true;
	}

	static int oldOption = -99;
	oldOption = m_nCurrOption;
	for (int rowToCheck = 0; rowToCheck < NUM_MENUROWS; ++rowToCheck) {
		if (aScreens[m_nCurrScreen].m_aEntries[rowToCheck].m_Action == MENUACTION_NOTHING || 
			aScreens[m_nCurrScreen].m_aEntries[rowToCheck].m_Action == MENUACTION_LABEL)
			continue;

		int extraOffset = 0;
		if (aScreens[m_nCurrScreen].m_aEntries[rowToCheck].m_Action == MENUACTION_RADIO)
			extraOffset = MENURADIO_ICON_SCALE;

		if (m_nMousePosY > MENU_Y(aScreens[m_nCurrScreen].m_aEntries[rowToCheck].m_Y) &&
			m_nMousePosY < MENU_Y(aScreens[m_nCurrScreen].m_aEntries[rowToCheck].m_Y + MENU_DEFAULT_LINE_HEIGHT)) {
			static int oldScreen = m_nCurrScreen;

			m_nPrevOption = rowToCheck;
			if (m_nMouseOldPosX != m_nMousePosX || m_nMouseOldPosY != m_nMousePosY) {
				m_nCurrOption = rowToCheck;
				m_bShowMouse = true;
			}

			int action = aScreens[m_nCurrScreen].m_aEntries[rowToCheck].m_Action;
			if (action != MENUACTION_BRIGHTNESS && action != MENUACTION_DRAWDIST && action != MENUACTION_MUSICVOLUME
				&& action != MENUACTION_SFXVOLUME && action != MENUACTION_MOUSESENS && action != MENUACTION_MP3VOLUMEBOOST)
				m_nHoverOption = HOVEROPTION_RANDOM_ITEM;

			break;
		}
		if (m_bShowMouse && m_nMenuFadeAlpha == 255) {
			m_nPrevOption = oldOption;
			m_nCurrOption = oldOption;
		}
	}

	if (m_bShowMouse) {
		if (oldOption != m_nCurrOption) {
			if (aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption].m_Action == MENUACTION_LABEL) {
				++m_nCurrOption;
				++m_nPrevOption;
			}
			m_nOptionHighlightTransitionBlend = 0;
			DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_DENIED, 0);
		}
	}

	m_nMouseOldPosX = m_nMousePosX;
	m_nMouseOldPosY = m_nMousePosY;
	m_nMousePosX = m_nMouseTempPosX;
	m_nMousePosY = m_nMouseTempPosY;

	if (m_nMousePosX < 0) m_nMousePosX = 0;
	if (m_nMousePosX > SCREEN_WIDTH) m_nMousePosX = SCREEN_WIDTH;
	if (m_nMousePosY < 0) m_nMousePosY = 0;
	if (m_nMousePosY > SCREEN_HEIGHT) m_nMousePosY = SCREEN_HEIGHT;

	if (m_nCurrScreen == MENUPAGE_SKIN_SELECT || m_nCurrScreen == MENUPAGE_KEYBOARD_CONTROLS) {

		if (m_nCurrScreen == MENUPAGE_SKIN_SELECT) {
			m_nTotalListRow = m_nSkinsTotal;
		}
		if (m_nCurrScreen == MENUPAGE_KEYBOARD_CONTROLS) {
			m_nTotalListRow = m_ControlMethod == CONTROL_CLASSIC ? 30 : 25;
			if (m_nSelectedListRow > m_nTotalListRow)
				m_nSelectedListRow = m_nTotalListRow - 1;
		}

		if (CPad::GetPad(0)->GetEnterJustDown() || CPad::GetPad(0)->GetCrossJustDown()) {
			m_bShowMouse = 0;
			optionSelected = true;
		}
		if (CPad::GetPad(0)->GetBackspaceJustDown() && m_nCurrScreen == MENUPAGE_KEYBOARD_CONTROLS && !field_159) {
			if (m_nCurrExLayer == HOVEROPTION_LIST) {
				m_nHoverOption = HOVEROPTION_NOT_HOVERING;
				m_bWaitingForNewKeyBind = true;
				m_bStartWaitingForKeyBind = true;
				m_bKeyChangeNotProcessed = true;
				pControlEdit = &m_KeyPressedCode;
			}
		} else {
			field_159 = false;
		}

		static PauseModeTime lastTimeClickedScrollButton = 0;

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

		if (CPad::GetPad(0)->GetEscapeJustDown() || CPad::GetPad(0)->GetBackJustDown()) {
			m_bShowMouse = false;
			goBack = true;
		}

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
		if (CPad::GetPad(0)->GetEnterJustDown() || CPad::GetPad(0)->GetCrossJustDown() || CPad::GetPad(0)->GetLeftMouseJustDown()) {
			optionSelected = true;
		}
		if (CPad::GetPad(0)->GetEscapeJustDown() || CPad::GetPad(0)->GetBackJustUp()) {
			if (m_nCurrScreen != MENUPAGE_START_MENU) {
				goBack = true;
			}
		}
	} else {
		if (CPad::GetPad(0)->GetDownJustDown() || CPad::GetPad(0)->GetAnaloguePadDown() || CPad::GetPad(0)->GetDPadDownJustDown()) {
			m_bShowMouse = false;
			DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_DENIED, 0);
			goDown = true;
			m_nOptionHighlightTransitionBlend = 0;
		} else if (CPad::GetPad(0)->GetUpJustDown() || CPad::GetPad(0)->GetAnaloguePadUp() || CPad::GetPad(0)->GetDPadUpJustDown()) {
			m_bShowMouse = false;
			DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_DENIED, 0);
			goUp = true;
			m_nOptionHighlightTransitionBlend = 0;
		}

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
	}


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

	if (goDown) {
		m_nCurrOption++;
		if (m_nCurrOption == NUM_MENUROWS || (aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption].m_Action == MENUACTION_NOTHING)) {
			m_nCurrOption = 0;
		}
	}
	if (goUp) {
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
		if (option == MENUACTION_CHANGEMENU || option == MENUACTION_POPULATESLOTS_CHANGEMENU || option == MENUACTION_GOBACK
			|| option == MENUACTION_YES || option == MENUACTION_NO) {
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
			&& option != MENUACTION_CHECKSAVE && option != MENUACTION_MOUSESENS
			&& option != MENUACTION_YES && option != MENUACTION_NO
			&& option != MENUACTION_SCREENRES) {

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
				SwitchToNewScreen(-2);
			}
		} else {
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
				case MENUACTION_LANG_PL:
					m_PrefsLanguage = LANGUAGE_POLISH;
					m_bFrontEnd_ReloadObrTxtGxt = true;
					InitialiseChangedLanguageSettings();
					SaveSettings();
					break;
				case MENUACTION_LANG_RUS:
					m_PrefsLanguage = LANGUAGE_RUSSIAN;
					m_bFrontEnd_ReloadObrTxtGxt = true;
					CMenuManager::InitialiseChangedLanguageSettings();
					SaveSettings();
					break;
				case MENUACTION_LANG_JAP:
					m_PrefsLanguage = LANGUAGE_JAPANESE;
					m_bFrontEnd_ReloadObrTxtGxt = true;
					InitialiseChangedLanguageSettings();
					SaveSettings();
					break;
#endif
				case MENUACTION_POPULATESLOTS_CHANGEMENU:
					PcSaveHelper.PopulateSlotInfo();

					// fall through
				case MENUACTION_CHANGEMENU:
				case MENUACTION_YES:
				case MENUACTION_NO:
				{
					// TODO(Miami): TEMP
#ifdef MENU_MAP
					if (aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption].m_TargetMenu == MENUPAGE_MAP) {
						bMapLoaded = false;
					}

#endif
					SwitchToNewScreen(0);
					break;
				}
				case MENUACTION_GOBACK:
					goBack = true;
					break;
				case MENUACTION_CHECKSAVE:
				{
					int saveSlot = aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption].m_SaveSlot;

					if (saveSlot >= 2 && saveSlot <= 9) {
						m_nCurrSaveSlot = saveSlot - 2;
						if (Slots[m_nCurrSaveSlot + 1] != SLOT_EMPTY && Slots[m_nCurrSaveSlot + 1] != SLOT_CORRUPTED) {
							if (m_nCurrScreen == MENUPAGE_CHOOSE_LOAD_SLOT) {
								SwitchToNewScreen(MENUPAGE_LOAD_SLOT_CONFIRM);
							} else if (m_nCurrScreen == MENUPAGE_CHOOSE_DELETE_SLOT) {
								SwitchToNewScreen(MENUPAGE_DELETE_SLOT_CONFIRM);
							}
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
				case MENUACTION_RESUME_FROM_SAVEZONE:
					RequestFrontEndShutDown();
					break;
				case MENUACTION_REDEFCTRL:
					SwitchToNewScreen(MENUPAGE_KEYBOARD_CONTROLS);
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
					SwitchToNewScreen(MENUPAGE_OUTRO);
					break;
				case MENUACTION_RESUME:
					if (m_PrefsVsyncDisp != m_PrefsVsync) {
						m_PrefsVsync = m_PrefsVsyncDisp;
					}
					RequestFrontEndShutDown();
					break;
				case MENUACTION_DONTCANCEL:
					SwitchToNewScreen(-2);
					break;
				case MENUACTION_SCREENRES:
					if (m_nDisplayVideoMode != m_nPrefsVideoMode) {
						m_nPrefsVideoMode = m_nDisplayVideoMode;
						_psSelectScreenVM(m_nPrefsVideoMode);
						SetHelperText(0); // TODO(Miami): Remove that
						DMAudio.ChangeMusicMode(MUSICMODE_FRONTEND);
						DMAudio.Service();
						CentreMousePointer();
						m_bShowMouse = true;
						// m_nCurrOption = 5; // Why?
						m_nOptionHighlightTransitionBlend = 0;
						SaveSettings();
					}
					break;
#ifdef IMPROVED_VIDEOMODE
				case MENUACTION_SCREENMODE:
					if (m_nSelectedScreenMode != m_nPrefsWindowed) {
						m_nPrefsWindowed = m_nSelectedScreenMode;
						_psSelectScreenVM(m_nPrefsVideoMode); // apply same resolution
						SetHelperText(0);
						SaveSettings();
					}
					break;
#endif
				case MENUACTION_AUDIOHW:
				{
					int selectedProvider = m_nPrefsAudio3DProviderIndex;
					if (selectedProvider != INVALID_AUDIO_PROVIDER) {
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
					if (m_nPrefsAudio3DProviderIndex != INVALID_AUDIO_PROVIDER) {
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
					SwitchToNewScreen(MENUPAGE_SKIN_SELECT);
					m_nCurrExLayer = HOVEROPTION_LIST;
					m_bSkinsEnumerated = false;
					break;
				case MENUACTION_RESTOREDEF:
					if (m_nCurrScreen == MENUPAGE_SOUND_SETTINGS) {
						m_PrefsSfxVolume = 102;
						m_PrefsSpeakers = 0;
						m_PrefsMusicVolume = 102;
						m_PrefsStereoMono = 0;
						m_PrefsRadioStation = WILDSTYLE;
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
#ifdef GTA3_1_1_PATCH
						if (_dwOperatingSystemVersion == OS_WIN98) {
							CMBlur::BlurOn = false;
							CMBlur::MotionBlurClose();
						}
						else {
							CMBlur::BlurOn = true;
							CMBlur::MotionBlurOpen(Scene.camera);
						}
#else
						CMBlur::BlurOn = true;
#endif
						SaveSettings();
					} else if (m_nCurrScreen == MENUPAGE_CONTROLLER_PC) {
						ControlsManager.MakeControllerActionsBlank();
						ControlsManager.InitDefaultControlConfiguration();
						ControlsManager.InitDefaultControlConfigMouse(MousePointerStateHelper.GetMouseSetUp());
#if !defined RW_GL3
						if (AllValidWinJoys.m_aJoys[JOYSTICK1].m_bInitialised) {
							DIDEVCAPS devCaps;
							devCaps.dwSize = sizeof(DIDEVCAPS);
							PSGLOBAL(joy1)->GetCapabilities(&devCaps);
							ControlsManager.InitDefaultControlConfigJoyPad(devCaps.dwButtons);
						}
#else
						if (PSGLOBAL(joy1id) != -1 && glfwJoystickPresent(PSGLOBAL(joy1id))) {
							int count;
							glfwGetJoystickButtons(PSGLOBAL(joy1id), &count);
							ControlsManager.InitDefaultControlConfigJoyPad(count);
						}
#endif
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
					SwitchToNewScreen(MENUPAGE_SOUND_SETTINGS);
					DMAudio.PlayFrontEndTrack(m_PrefsRadioStation, 1);
					OutputDebugString("STARTED PLAYING FRONTEND AUDIO TRACK");
					break;
				case MENUACTION_SAVEGAME:
				{
					int saveSlot = aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption].m_SaveSlot;

					if (saveSlot >= 2 && saveSlot <= 9) {
						m_nCurrSaveSlot = m_nCurrOption;
						SwitchToNewScreen(MENUPAGE_SAVE_OVERWRITE_CONFIRM);
					}
					break;
				}
				case MENUACTION_KEYBOARDCTRLS:
					SwitchToNewScreen(MENUPAGE_KEYBOARD_CONTROLS);
					m_nSelectedListRow = 0;
					m_nCurrExLayer = HOVEROPTION_LIST;
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
			if (!m_bGameNotLoaded) {
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

		int oldScreen = aScreens[m_nCurrScreen].m_PreviousPage;

		if (oldScreen != -1) {
			ThingsToDoBeforeLeavingPage();

#ifdef PS2_LIKE_MENU
			if (!bottomBarActive &&
				(oldScreen == MENUPAGE_NONE || oldScreen == MENUPAGE_OPTIONS)) {
				bottomBarActive = true;
			} else
#endif
			{
				SwitchToNewScreen(-2);
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
		static PauseModeTime lastSliderDecrease = 0;
		if (CTimer::GetTimeInMillisecondsPauseMode() - lastSliderDecrease > 150) {
			CheckSliderMovement(-1);
			lastSliderDecrease = CTimer::GetTimeInMillisecondsPauseMode();
		}
	} else if (CPad::GetPad(0)->GetRight() || CPad::GetPad(0)->GetPedWalkLeftRight() > 0 || CPad::GetPad(0)->GetDPadRight()) {
		static PauseModeTime lastSliderIncrease = 0;
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
					if (m_PrefsRadioStation < WILDSTYLE)
						m_PrefsRadioStation = USERTRACK;
					if (m_PrefsRadioStation > USERTRACK)
						m_PrefsRadioStation = WILDSTYLE;
				} else {
					if (m_PrefsRadioStation < WILDSTYLE)
						m_PrefsRadioStation = WAVE;
					if (m_PrefsRadioStation > WAVE)
						m_PrefsRadioStation = WILDSTYLE;
				}
				SaveSettings();
				DMAudio.SetRadioInCar(m_PrefsRadioStation);
				DMAudio.PlayFrontEndTrack(m_PrefsRadioStation, 1);
				OutputDebugString("FRONTEND RADIO STATION CHANGED");
				break;
#ifdef ASPECT_RATIO_SCALE
			case MENUACTION_WIDESCREEN:
				if (changeValueBy > 0) {
					m_PrefsUseWideScreen++;
					if (m_PrefsUseWideScreen > 2)
						m_PrefsUseWideScreen = 0;
				} else {
					m_PrefsUseWideScreen--;
					if (m_PrefsUseWideScreen < 0)
						m_PrefsUseWideScreen = 2;
				}
				DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_SUCCESS, 0);
				SaveSettings();
				break;
#endif
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
#ifdef IMPROVED_VIDEOMODE
			case MENUACTION_SCREENMODE:
				DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_DENIED, 0);
				m_nSelectedScreenMode = !m_nSelectedScreenMode;
				break;
#endif
			case MENUACTION_AUDIOHW:
				if (m_nPrefsAudio3DProviderIndex != INVALID_AUDIO_PROVIDER) {
					m_nPrefsAudio3DProviderIndex += changeValueBy;
					m_nPrefsAudio3DProviderIndex = clamp(m_nPrefsAudio3DProviderIndex, 0, DMAudio.GetNum3DProvidersAvailable() - 1);
				}
				DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_DENIED, 0);
				break;
			case MENUACTION_SPEAKERCONF:
				if (m_nPrefsAudio3DProviderIndex != INVALID_AUDIO_PROVIDER) {
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
		DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_SUCCESS, 0);
		break;
	case MENUACTION_CTRLCONFIG:
		CPad::GetPad(0)->Mode++;
		if (CPad::GetPad(0)->Mode > 3)
			CPad::GetPad(0)->Mode = 0;
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
#ifndef ASPECT_RATIO_SCALE
	case MENUACTION_WIDESCREEN:
		m_PrefsUseWideScreen = !m_PrefsUseWideScreen;
		DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_SUCCESS, 0);
		SaveSettings();
		break;
#endif
	case MENUACTION_SETDBGFLAG:
		CTheScripts::InvertDebugFlag();
		DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_SUCCESS, 0);
		break;
	case MENUACTION_SWITCHBIGWHITEDEBUGLIGHT:
		gbBigWhiteDebugLightSwitchedOn = !gbBigWhiteDebugLightSwitchedOn;
		DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_SUCCESS, 0);
		break;
	case MENUACTION_COLLISIONPOLYS:
		gbShowCollisionPolys = !gbShowCollisionPolys;
		DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_SUCCESS, 0);
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
#ifdef FREE_CAM
	case MENUACTION_FREECAM:
		TheCamera.bFreeCam = !TheCamera.bFreeCam;
		DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_SUCCESS, 0);
		SaveSettings();
		break;
#endif
	}
}

void
CMenuManager::RequestFrontEndShutDown()
{
	m_bShutDownFrontEndRequested = true;
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
CMenuManager::SetHelperText(int text)
{
	m_nHelperTextMsgId = text;
	m_nHelperTextAlpha = 300;
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
CMenuManager::ProcessFileActions()
{
	switch (m_nCurrScreen) {
		case MENUPAGE_LOADING_IN_PROGRESS:
			if (CheckSlotDataValid(m_nCurrSaveSlot)) {
#ifdef USE_DEBUG_SCRIPT_LOADER
				scriptToLoad = 0;
#endif
				if (!m_bGameNotLoaded)
					MessageScreen("FELD_WR", true);

				DoSettingsBeforeStartingAGame();
				m_bWantToLoad = true;
			} else
				SwitchToNewScreen(MENUPAGE_NEW_GAME);

			break;
		case MENUPAGE_DELETING_IN_PROGRESS:
		{
			static bool waitedForScreen = false;

			if (waitedForScreen) {
				bool SlotPopulated = false;
				if (PcSaveHelper.DeleteSlot(m_nCurrSaveSlot)) {
					PcSaveHelper.PopulateSlotInfo();
					SlotPopulated = true;
				}

				if (SlotPopulated) {
					SwitchToNewScreen(MENUPAGE_DELETE_SUCCESSFUL);
				} else {
					SwitchToNewScreen(MENUPAGE_SAVE_CUSTOM_WARNING);
					strncpy(aScreens[m_nCurrScreen].m_ScreenName, "FES_DEL", 8);
					strncpy(aScreens[m_nCurrScreen].m_aEntries[0].m_EntryName, "FES_DEE", 8);
				}
				waitedForScreen = false;
			} else if (m_nMenuFadeAlpha >= 255)
				waitedForScreen = true;

			break;
		}
		case MENUPAGE_SAVING_IN_PROGRESS:
		{
			static bool waitedForScreen = false;

			if (waitedForScreen) {
				int8 SaveSlot = PcSaveHelper.SaveSlot(m_nCurrSaveSlot);
				PcSaveHelper.PopulateSlotInfo();
				if (SaveSlot) {
					SwitchToNewScreen(MENUPAGE_SAVE_CUSTOM_WARNING);
					strncpy(aScreens[m_nCurrScreen].m_ScreenName, "FET_SG", 8);
					strncpy(aScreens[m_nCurrScreen].m_aEntries[0].m_EntryName, "FES_CMP", 8);
				} else
					SwitchToNewScreen(MENUPAGE_SAVE_SUCCESSFUL);

				waitedForScreen = false;
			} else if (m_nMenuFadeAlpha >= 255)
				waitedForScreen = true;

			break;
		}
	}
}

// --MIAMI: Done except DxInput things, are they even needed?
void
CMenuManager::SwitchMenuOnAndOff()
{
	if (!TheCamera.m_WideScreenOn) {

		// Reminder: You need REGISTER_START_BUTTON defined to make it work.
		if ((CPad::GetPad(0)->GetStartJustDown() || CPad::GetPad(0)->GetEscapeJustDown())
			&& (!m_bMenuActive || m_nCurrScreen == MENUPAGE_PAUSE_MENU || m_nCurrScreen == MENUPAGE_CHOOSE_SAVE_SLOT || m_nCurrScreen == MENUPAGE_SAVE_CHEAT_WARNING)
			|| m_bShutDownFrontEndRequested || m_bStartUpFrontEndRequested) {

			if (m_nCurrScreen != MENUPAGE_LOADING_IN_PROGRESS) {
				DoRWStuffStartOfFrame(0, 0, 0, 0, 0, 0, 255);
				DoRWStuffEndOfFrame();
				DoRWStuffStartOfFrame(0, 0, 0, 0, 0, 0, 255);
				DoRWStuffEndOfFrame();
			}

			if (m_bShutDownFrontEndRequested)
				m_bMenuActive = false;
			else if (m_bStartUpFrontEndRequested)
				m_bMenuActive = true;
			else
				m_bMenuActive = !m_bMenuActive;

			if (m_bMenuActive) {
				// TODO(Miami): DxInput??

				Initialise();
				LoadAllTextures();
			} else {
				if (CMBlur::BlurOn)
					CMBlur::MotionBlurOpen(Scene.camera);
				else
					CMBlur::MotionBlurClose();
				DoRWStuffStartOfFrame(0, 0, 0, 0, 0, 0, 255);
				DoRWStuffEndOfFrame();
				DoRWStuffStartOfFrame(0, 0, 0, 0, 0, 0, 255);
				DoRWStuffEndOfFrame();

				// TODO(Miami): DxInput??

#ifdef PS2_LIKE_MENU
				bottomBarActive = false;
#endif

				m_StatsScrollSpeed = 150.0f;
				SaveSettings();
				pControlEdit = nil;
				pEditString = nil;
				DisplayComboButtonErrMsg = false;
				m_bShutDownFrontEndRequested = false;
				m_bStartUpFrontEndRequested = false;
				m_bWaitingForNewKeyBind = false;

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
				UnloadTextures();
				CTimer::EndUserPause();
				CTimer::Update();
				m_OnlySaveMenu = false;
			}
		}
	}

	// Just entered the save/safe zone
	if (m_bActivateSaveMenu) {
		DoRWStuffStartOfFrame(0, 0, 0, 0, 0, 0, 255);
		DoRWStuffEndOfFrame();
		DoRWStuffStartOfFrame(0, 0, 0, 0, 0, 0, 255);
		DoRWStuffEndOfFrame();
		m_bActivateSaveMenu = false;
		m_bMenuActive = true;
		m_OnlySaveMenu = true;

		// TODO(Miami): DxInput??

		Initialise();
		LoadAllTextures();

		// TODO(Miami): Cheat warning
		m_nCurrScreen = MENUPAGE_CHOOSE_SAVE_SLOT;
		m_nCurrOption = 8;
	}

	m_bStartUpFrontEndRequested = false;
	m_bShutDownFrontEndRequested = false;
}

void
CMenuManager::UnloadTextures()
{
	if (m_nCurrScreen == MENUPAGE_SOUND_SETTINGS)
		DMAudio.StopFrontEndTrack();

	DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_STARTING, 0);
	DMAudio.ChangeMusicMode(MUSICMODE_GAME);
	if (m_bSpritesLoaded) {
		printf("REMOVE frontend\n");
		int frontend = CTxdStore::FindTxdSlot("frontend1");
		for (int i = 0; i < 3; ++i)
			m_aFrontEndSprites[i].Delete();

		CTxdStore::RemoveTxd(frontend);

		if (!m_OnlySaveMenu) {
			int frontend2 = CTxdStore::FindTxdSlot("frontend2");
			for (int i = 3; i < NUM_MENU_SPRITES; ++i)
				m_aFrontEndSprites[i].Delete();

			CTxdStore::RemoveTxd(frontend2);
		}

		m_bSpritesLoaded = false;
	}
	m_OnlySaveMenu = false;
	// TODO(Miami): Place name thing
}

void
CMenuManager::WaitForUserCD()
{
	CSprite2d *splash;
	char *splashscreen = nil;

#if (!(defined RANDOMSPLASH) && !(defined GTA3_1_1_PATCH))
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
	MessageScreen("NO_PCCD", true);

	if (CPad::GetPad(0)->GetEscapeJustDown()) {
		m_bQuitGameNoCD = true;
		RsEventHandler(rsQUITAPP, nil);
	}
}

void
CMenuManager::DrawQuitGameScreen(void)
{
	static int32 exitSignalTimer = 0;
#ifndef MUCH_SHORTER_OUTRO_SCREEN
	static PauseModeTime lastTickIncrease = 0;
	if (m_nMenuFadeAlpha == 255 && CTimer::GetTimeInMillisecondsPauseMode() - lastTickIncrease > 10) {
		exitSignalTimer++;
		lastTickIncrease = CTimer::GetTimeInMillisecondsPauseMode();
	}
#else
	static PauseModeTime sincePress = 0;
	sincePress += frameTime;
	if (sincePress > 500)
		exitSignalTimer = 150;
#endif
	static CSprite2d *splash = nil;

	if (splash == nil)
		splash = LoadSplash("OUTRO");

	m_aFrontEndSprites[MENUSPRITE_VCLOGO].Draw(CRect(MENU_X(28.0f), MENU_Y(8.0f), MENU_X(157.0f), MENU_Y(138.0f)), CRGBA(255, 255, 255, -(m_nMenuFadeAlpha + 1)));

	// Or we can see menu background from sides
#ifdef ASPECT_RATIO_SCALE
	CSprite2d::DrawRect(CRect(0, 0, MENU_X_LEFT_ALIGNED(0.f), SCREEN_HEIGHT), CRGBA(0, 0, 0, m_nMenuFadeAlpha));
	CSprite2d::DrawRect(CRect(MENU_X_RIGHT_ALIGNED(0.f), 0, SCREEN_WIDTH, SCREEN_HEIGHT), CRGBA(0, 0, 0, m_nMenuFadeAlpha));
#endif

	splash->Draw(CRect(MENU_X_LEFT_ALIGNED(0.f), 0, MENU_X_RIGHT_ALIGNED(0.f), SCREEN_HEIGHT), CRGBA(255, 255, 255, m_nMenuFadeAlpha));
	if (m_nMenuFadeAlpha == 255 && exitSignalTimer == 150)
		RsEventHandler(rsQUITAPP, nil);

	m_bShowMouse = false;
	m_AllowNavigation = false;
}

#ifdef MENU_MAP

#define ZOOM(x, y, in) \
	do { \
		if(m_fMapSize > SCREEN_HEIGHT * 3.0f && in) \
			break; \
		float z2 = in? 1.1f : 1.f/1.1f; \
		m_fMapCenterX += (x - m_fMapCenterX) * (1.0f - z2); \
		m_fMapCenterY += (y - m_fMapCenterY) * (1.0f - z2); \
		\
		if (m_fMapSize < SCREEN_HEIGHT / 2 && !in) \
			break; \
		\
		m_fMapSize *= z2; \
	} while(0) \

void
CMenuManager::PrintMap(void)
{
	CFont::SetJustifyOn();
	m_bMenuMapActive = true;
	CRadar::InitFrontEndMap();

	// Just entered to map
	if (!bMapLoaded) {
		m_fMapSize = SCREEN_HEIGHT * 2.0f;
		m_fMapCenterX = 0.0f;
		m_fMapCenterY = 0.0f;
		CVector2D radarSpacePlayer;
		CVector2D screenSpacePlayer;
		CRadar::TransformRealWorldPointToRadarSpace(radarSpacePlayer, CVector2D(FindPlayerCoors()));
		CRadar::TransformRadarPointToScreenSpace(screenSpacePlayer, radarSpacePlayer);

		m_fMapCenterX = (-screenSpacePlayer.x) + SCREEN_WIDTH / 2;
		m_fMapCenterY = (-screenSpacePlayer.y) + SCREEN_HEIGHT / 2;
		bMapMouseShownOnce = false;
		bMapLoaded = true;

		// Let's wait for a frame to not toggle the waypoint
		if (CPad::GetPad(0)->NewState.Cross) {
			m_bMenuMapActive = false;
			return;
		}
	}

	// Because m_fMapSize is half of the map length, and map consists of 3x3 tiles.
	float halfTile = m_fMapSize / 3.0f;

	// Darken background a bit
	CSprite2d::DrawRect(CRect(0, 0,
		SCREEN_WIDTH, SCREEN_HEIGHT),
		CRGBA(0, 0, 0, FadeIn(128)));

	RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void*)rwFILTERLINEAR);

	if (SCREEN_WIDTH >= m_fMapCenterX - m_fMapSize || SCREEN_HEIGHT >= m_fMapCenterY - m_fMapSize) {
		m_aFrontEndSprites[MENUSPRITE_MAPTOP01].Draw(CRect(m_fMapCenterX - m_fMapSize, m_fMapCenterY - m_fMapSize,
			m_fMapCenterX - halfTile, m_fMapCenterY - halfTile), CRGBA(255, 255, 255, FadeIn(255)));
	}

	if (SCREEN_WIDTH >= m_fMapCenterX - halfTile || SCREEN_HEIGHT >= m_fMapCenterY - m_fMapSize) {
		m_aFrontEndSprites[MENUSPRITE_MAPTOP02].Draw(CRect(m_fMapCenterX - halfTile, m_fMapCenterY - m_fMapSize,
			m_fMapCenterX + halfTile, m_fMapCenterY - halfTile), CRGBA(255, 255, 255, FadeIn(255)));
	}

	if (SCREEN_WIDTH >= m_fMapCenterX + halfTile || SCREEN_HEIGHT >= m_fMapCenterY - m_fMapSize) {
		m_aFrontEndSprites[MENUSPRITE_MAPTOP03].Draw(CRect(m_fMapCenterX + halfTile, m_fMapCenterY - m_fMapSize,
			m_fMapCenterX + m_fMapSize, m_fMapCenterY - halfTile), CRGBA(255, 255, 255, FadeIn(255)));
	}

	if (SCREEN_WIDTH >= m_fMapCenterX - m_fMapSize || SCREEN_HEIGHT >= m_fMapCenterY - halfTile) {
		m_aFrontEndSprites[MENUSPRITE_MAPMID01].Draw(CRect(m_fMapCenterX - m_fMapSize, m_fMapCenterY - halfTile,
			m_fMapCenterX - halfTile, m_fMapCenterY + halfTile), CRGBA(255, 255, 255, FadeIn(255)));
	}

	if (SCREEN_WIDTH >= m_fMapCenterX - halfTile || SCREEN_HEIGHT >= m_fMapCenterY - halfTile) {
		m_aFrontEndSprites[MENUSPRITE_MAPMID02].Draw(CRect(m_fMapCenterX - halfTile, m_fMapCenterY - halfTile,
			m_fMapCenterX + halfTile, m_fMapCenterY + halfTile), CRGBA(255, 255, 255, FadeIn(255)));
	}

	if (SCREEN_WIDTH >= m_fMapCenterX + halfTile || SCREEN_HEIGHT >= m_fMapCenterY - halfTile) {
		m_aFrontEndSprites[MENUSPRITE_MAPMID03].Draw(CRect(m_fMapCenterX + halfTile, m_fMapCenterY - halfTile,
			m_fMapCenterX + m_fMapSize, m_fMapCenterY + halfTile), CRGBA(255, 255, 255, FadeIn(255)));
	}

	if (SCREEN_WIDTH >= m_fMapCenterX - m_fMapSize || SCREEN_HEIGHT >= m_fMapCenterY + halfTile) {
		m_aFrontEndSprites[MENUSPRITE_MAPBOT01].Draw(CRect(m_fMapCenterX - m_fMapSize, m_fMapCenterY + halfTile,
			m_fMapCenterX - halfTile, m_fMapCenterY + m_fMapSize), CRGBA(255, 255, 255, FadeIn(255)));
	}

	if (SCREEN_WIDTH >= m_fMapCenterX - halfTile || SCREEN_HEIGHT >= m_fMapCenterY + halfTile) {
		m_aFrontEndSprites[MENUSPRITE_MAPBOT02].Draw(CRect(m_fMapCenterX - halfTile, m_fMapCenterY + halfTile,
			m_fMapCenterX + halfTile, m_fMapCenterY + m_fMapSize), CRGBA(255, 255, 255, FadeIn(255)));
	}

	if (SCREEN_WIDTH >= m_fMapCenterX + halfTile || SCREEN_HEIGHT >= m_fMapCenterY + halfTile) {
		m_aFrontEndSprites[MENUSPRITE_MAPBOT03].Draw(CRect(m_fMapCenterX + halfTile, m_fMapCenterY + halfTile,
			m_fMapCenterX + m_fMapSize, m_fMapCenterY + m_fMapSize), CRGBA(255, 255, 255, FadeIn(255)));
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
		if (mapPoint.y > m_fMapCenterY - m_fMapSize && mapPoint.y < m_fMapCenterY + m_fMapSize &&
			mapPoint.x > m_fMapCenterX - m_fMapSize && mapPoint.x < m_fMapCenterX + m_fMapSize) {

			// Don't ask me the meanings, I don't know. Found them by trying
			float diffX = m_fMapCenterX - m_fMapSize, diffY = m_fMapCenterY - m_fMapSize;
			float x = ((mapPoint.x - diffX) / (m_fMapSize * 2)) * (WORLD_SIZE_X / MENU_MAP_WIDTH_SCALE) - (WORLD_SIZE_X / 2 + MENU_MAP_LEFT_OFFSET * MENU_MAP_LENGTH_UNIT);
			float y = (WORLD_SIZE_Y / 2 - MENU_MAP_TOP_OFFSET * MENU_MAP_LENGTH_UNIT) - ((mapPoint.y - diffY) / (m_fMapSize * 2)) * (WORLD_SIZE_Y / MENU_MAP_HEIGHT_SCALE);
			CRadar::ToggleTargetMarker(x, y);
			DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_SUCCESS, 0);
		}
	}

	if (CPad::GetPad(0)->GetLeftMouse()) {
		m_fMapCenterX += m_nMousePosX - m_nMouseOldPosX;
		m_fMapCenterY += m_nMousePosY - m_nMouseOldPosY;
	} else if (CPad::GetPad(0)->GetLeft() || CPad::GetPad(0)->GetDPadLeft()) {
		m_fMapCenterX += 15.0f;
	} else if (CPad::GetPad(0)->GetRight() || CPad::GetPad(0)->GetDPadRight()) {
		m_fMapCenterX -= 15.0f;
	} else if (CPad::GetPad(0)->GetLeftStickX()) {
		m_fMapCenterX -= CPad::GetPad(0)->GetLeftStickX() / 128.0f * 20.0f;
	}

	if (CPad::GetPad(0)->GetUp() || CPad::GetPad(0)->GetDPadUp()) {
		m_fMapCenterY += 15.0f;
	} else if (CPad::GetPad(0)->GetDown() || CPad::GetPad(0)->GetDPadDown()) {
		m_fMapCenterY -= 15.0f;
	} else if (CPad::GetPad(0)->GetLeftStickY()) {
		m_fMapCenterY -= CPad::GetPad(0)->GetLeftStickY() / 128.0f * 20.0f;
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
	
	if (m_fMapCenterX - m_fMapSize > SCREEN_WIDTH / 2)
		m_fMapCenterX = m_fMapSize + SCREEN_WIDTH / 2;

	if (m_fMapCenterX + m_fMapSize < SCREEN_WIDTH / 2)
		m_fMapCenterX = SCREEN_WIDTH / 2 - m_fMapSize;

	if (m_fMapCenterY + m_fMapSize < SCREEN_HEIGHT - MENU_Y(60.0f))
		m_fMapCenterY = SCREEN_HEIGHT - MENU_Y(60.0f) - m_fMapSize;
	
	if (m_fMapCenterY - m_fMapSize > SCREEN_HEIGHT / 2)
		m_fMapCenterY = SCREEN_HEIGHT / 2 + m_fMapSize;

	m_bMenuMapActive = false;

	// CFont::SetWrapx(MENU_X_RIGHT_ALIGNED(5.0f)); // From VC
	// CFont::SetRightJustifyWrap(10.0f);

	CSprite2d::DrawRect(CRect(MENU_X(14.0f), SCREEN_STRETCH_FROM_BOTTOM(95.0f),
		SCREEN_STRETCH_FROM_RIGHT(11.0f), SCREEN_STRETCH_FROM_BOTTOM(59.0f)),
		CRGBA(MAPINFOBOX_COLOR.r, MAPINFOBOX_COLOR.g, MAPINFOBOX_COLOR.b, MAPINFOBOX_COLOR.a));

	CFont::SetScale(MENU_X(0.4f), MENU_Y(0.7f));
	CFont::SetFontStyle(FONT_LOCALE(FONT_BANK));
	CFont::SetColor(CRGBA(0, 0, 0, FadeIn(255)));

	float nextX = MENU_X(30.0f), nextY = 95.0f;
	wchar *text;
#ifdef MORE_LANGUAGES
#define TEXT_PIECE(key,extraSpace) \
	text = TheText.Get(key);\
	CFont::PrintString(nextX, SCREEN_SCALE_FROM_BOTTOM(nextY), text);\
	if (CFont::IsJapanese())\
		nextX += CFont::GetStringWidth_Jap(text) + MENU_X(extraSpace);\
	else\
		nextX += CFont::GetStringWidth(text, true) + MENU_X(extraSpace);
#else
#define TEXT_PIECE(key,extraSpace) \
	text = TheText.Get(key); CFont::PrintString(nextX, SCREEN_SCALE_FROM_BOTTOM(nextY), text); nextX += CFont::GetStringWidth(text, true) + MENU_X(extraSpace);
#endif

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

	int percentCompleted = CStats::GetPercentageProgress();

	STAT_LINE("PER_COM", &percentCompleted, false, nil);
	STAT_LINE("NMISON", &CStats::MissionsGiven, false, nil);
	STAT_LINE("FEST_MP", &CStats::MissionsPassed, false, &CStats::TotalNumberMissions);
	if (CGame::nastyGame) {
		STAT_LINE("FEST_RP", &CStats::NumberKillFrenziesPassed, false, &CStats::TotalNumberKillFrenzies);
	}
	
	CPlayerInfo &player = CWorld::Players[CWorld::PlayerInFocus];
	float packagesPercent = 0.0f;
	if (player.m_nTotalPackages != 0)
		packagesPercent = player.m_nCollectedPackages * 100.0f / player.m_nTotalPackages;

	int nPackagesPercent = packagesPercent;
	STAT_LINE("PERPIC", &nPackagesPercent, false, &(nTemp = 100));
	STAT_LINE("NOUNIF", &CStats::NumberOfUniqueJumpsFound, false, &CStats::TotalNumberOfUniqueJumps);
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
		case LANGUAGE_POLISH:
		case LANGUAGE_RUSSIAN:
		case LANGUAGE_JAPANESE:
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
