#define FORCE_PC_SCALING
#define WITHWINDOWS
#define WITHDINPUT
#include "common.h"
#ifndef PS2_MENU
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
#include "frontendoption.h"

// Game has colors inlined in code.
// For easier modification we collect them here:
const CRGBA LABEL_COLOR(235, 170, 50, 255);
const CRGBA SELECTION_HIGHLIGHTBG_COLOR(100, 200, 50, 50);
const CRGBA MENUOPTION_COLOR = LABEL_COLOR;
const CRGBA SELECTEDMENUOPTION_COLOR(255, 217, 106, 255);
const CRGBA HEADER_COLOR(0, 0, 0, 255);
const CRGBA DARKMENUOPTION_COLOR(155, 117, 6, 255);
const CRGBA SLIDERON_COLOR = SELECTEDMENUOPTION_COLOR;
const CRGBA SLIDEROFF_COLOR(185, 120, 0, 255);
const CRGBA LIST_BACKGROUND_COLOR(200, 200, 50, 50);
const CRGBA LIST_OPTION_COLOR(155, 155, 155, 255);
const CRGBA INACTIVE_RADIO_COLOR(225, 0, 0, 170);
const CRGBA SCROLLBAR_COLOR = LABEL_COLOR;
const CRGBA CONTSETUP_HIGHLIGHTBG_COLOR(SELECTEDMENUOPTION_COLOR.r, SELECTEDMENUOPTION_COLOR.g, SELECTEDMENUOPTION_COLOR.b, 210);
const CRGBA CONTSETUP_DISABLED_HIGHLIGHTBG_COLOR(MENUOPTION_COLOR.r, MENUOPTION_COLOR.g, MENUOPTION_COLOR.b, 150);

// This is PS2 menu leftover, and variable name is original. They forgot it here and used in PrintBriefs once (but didn't use the output)
#if defined(FIX_BUGS) && !defined(PS2_LIKE_MENU)
const CRGBA TEXT_COLOR = LABEL_COLOR;
#else
const CRGBA TEXT_COLOR = CRGBA(150, 110, 30, 255); // PS2 option color
#endif

#define TIDY_UP_PBP // ProcessButtonPresses
#define MAX_VISIBLE_LIST_ROW 30
#define SCROLLBAR_MAX_HEIGHT 263.0f // not in end result
#define SCROLLABLE_PAGES
#define RED_DELETE_BACKGROUND

#ifdef SCROLLABLE_STATS_PAGE
#define isPlainTextScreen(screen) (screen == MENUPAGE_BRIEFS)
#else
#define isPlainTextScreen(screen) (screen == MENUPAGE_BRIEFS || screen == MENUPAGE_STATS)
#endif

#define hasNativeList(screen) (screen == MENUPAGE_MULTIPLAYER_FIND_GAME || screen == MENUPAGE_SKIN_SELECT \
		|| screen == MENUPAGE_KEYBOARD_CONTROLS)
		
#ifdef SCROLLABLE_PAGES
#define MAX_VISIBLE_OPTION 12
#define MAX_VISIBLE_OPTION_ON_SCREEN (hasNativeList(m_nCurrScreen) ? MAX_VISIBLE_LIST_ROW : MAX_VISIBLE_OPTION)
#define SCREEN_HAS_AUTO_SCROLLBAR (m_nTotalListRow > MAX_VISIBLE_OPTION && !hasNativeList(m_nCurrScreen))

int GetOptionCount(int screen)
{
	int i = 0;
	for (; i < NUM_MENUROWS && aScreens[screen].m_aEntries[i].m_Action != MENUACTION_NOTHING; i++);
	return i;
}

#define SETUP_SCROLLING(screen) \
		if (!hasNativeList(screen)) { \
			m_nTotalListRow = GetOptionCount(screen); \
			if (m_nTotalListRow > MAX_VISIBLE_OPTION) { \
				m_nSelectedListRow = 0; \
				m_nFirstVisibleRowOnList = 0; \
				m_nScrollbarTopMargin = 0; \
			} \
		}
#else
#define MAX_VISIBLE_OPTION_ON_SCREEN MAX_VISIBLE_LIST_ROW
#define SETUP_SCROLLING(screen)
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
float CMenuManager::fMapSize;
float CMenuManager::fMapCenterY;
float CMenuManager::fMapCenterX;
#endif

#ifdef PS2_LIKE_MENU
BottomBarOption bbNames[8];
int bbTabCount = 0;
bool bottomBarActive = false;
int pendingScreen = -1;
int pendingOption = -1;
int curBottomBarOption = -1;
int hoveredBottomBarOption = -1;
#endif

#ifdef CUTSCENE_BORDERS_SWITCH
bool CMenuManager::m_PrefsCutsceneBorders = true;
#endif

#ifdef MULTISAMPLING
int8 CMenuManager::m_nPrefsMSAALevel = 0;
int8 CMenuManager::m_nDisplayMSAALevel = 0;
#endif

#ifdef NO_ISLAND_LOADING
int8 CMenuManager::m_PrefsIslandLoading = ISLAND_LOADING_LOW;
#endif

#ifdef GAMEPAD_MENU
int8 CMenuManager::m_PrefsControllerType = CONTROLLER_XBOXONE;
#endif

int32 CMenuManager::OS_Language = LANG_ENGLISH;
int8 CMenuManager::m_PrefsUseVibration;
int8 CMenuManager::m_DisplayControllerOnFoot;
int8 CMenuManager::m_PrefsVsync = 1;
int8 CMenuManager::m_PrefsVsyncDisp = 1;
int8 CMenuManager::m_PrefsFrameLimiter = 1;
int8 CMenuManager::m_PrefsShowSubtitles = 1;
int8 CMenuManager::m_PrefsSpeakers;
int32 CMenuManager::m_ControlMethod;
int8 CMenuManager::m_PrefsDMA = 1;
int32 CMenuManager::m_PrefsLanguage;
uint8 CMenuManager::m_PrefsStereoMono; // unused except restore settings

bool CMenuManager::m_PrefsAllowNastyGame = true;
bool CMenuManager::m_bStartUpFrontEndRequested;
bool CMenuManager::m_bShutDownFrontEndRequested;

#ifdef ASPECT_RATIO_SCALE
int8 CMenuManager::m_PrefsUseWideScreen = AR_AUTO;
#else
int8 CMenuManager::m_PrefsUseWideScreen;
#endif

int8 CMenuManager::m_PrefsRadioStation;
int32 CMenuManager::m_PrefsBrightness = 256;
float CMenuManager::m_PrefsLOD = CRenderer::ms_lodDistScale;
int8 CMenuManager::m_bFrontEnd_ReloadObrTxtGxt;
int32 CMenuManager::m_PrefsMusicVolume = 102;
int32 CMenuManager::m_PrefsSfxVolume = 102;

char CMenuManager::m_PrefsSkinFile[256] = DEFAULT_SKIN_NAME;

int32 CMenuManager::m_KeyPressedCode = -1;

float MENU_TEXT_SIZE_X = SMALLTEXT_X_SCALE;
float MENU_TEXT_SIZE_Y = SMALLTEXT_Y_SCALE;

bool holdingScrollBar; // *(bool*)0x628D59; // not original name
int32 CMenuManager::m_SelectedMap;
int32 CMenuManager::m_SelectedGameType;

// Used in a hidden menu
uint8 CMenuManager::m_PrefsPlayerRed = 255;
uint8 CMenuManager::m_PrefsPlayerGreen = 128;
uint8 CMenuManager::m_PrefsPlayerBlue; // why??

CMenuManager FrontEndMenuManager;

uint32 TimeToStopPadShaking;
char *pEditString;
int32 *pControlEdit;
bool DisplayComboButtonErrMsg;
int32 MouseButtonJustClicked;
int32 JoyButtonJustClicked;
//int32 *pControlTemp = 0;

#ifndef MASTER
bool CMenuManager::m_PrefsMarketing = false;
bool CMenuManager::m_PrefsDisableTutorials = false;
#endif // !MASTER

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
	{"fe_radio1", "" },
	{"fe_radio2", "" },
	{"fe_radio3", "" },
	{"fe_radio4", "" },
	{"fe_radio5", "" },
	{"fe_radio6", "" },
	{"fe_radio7", "" },
	{"fe_radio8", "" },
	{"fe_radio9", "" },
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

#define MENU_X_RIGHT_ALIGNED(x) SCALE_AND_CENTER_X(DEFAULT_SCREEN_WIDTH - (x))

#ifdef ASPECT_RATIO_SCALE
// All of the defines below replace the StretchX function. Otherwise use SCREEN_SCALE_X.
#define MENU_X_LEFT_ALIGNED(x) SCALE_AND_CENTER_X(x)
#define MENU_X(x) SCREEN_SCALE_X(x)
#define MENU_Y(y) SCREEN_SCALE_Y(y)
#else
#define MENU_X_LEFT_ALIGNED(x) StretchX(x)
#define MENU_X(x) StretchX(x)
#define MENU_Y(y) StretchY(y)
#endif

#ifdef PS2_LIKE_MENU
#define PAGE_NAME_X MENU_X_RIGHT_ALIGNED
#else
#define PAGE_NAME_X SCREEN_SCALE_FROM_RIGHT
#endif

// Seperate func. in VC
#define ChangeScreen(screen, option, updateDelay, clearAlpha) \
	do { \
		m_nPrevScreen = m_nCurrScreen; \
		int newOpt = option; \
		SETUP_SCROLLING(screen) \
		m_nCurrScreen = screen; \
		m_nCurrOption = newOpt; \
		if(updateDelay) \
			m_nScreenChangeDelayTimer = CTimer::GetTimeInMillisecondsPauseMode(); \
		if(clearAlpha) \
			m_nMenuFadeAlpha = 0; \
	} while(0)

#define SET_FONT_FOR_MENU_HEADER \
	CFont::SetColor(CRGBA(HEADER_COLOR.r, HEADER_COLOR.g, HEADER_COLOR.b, FadeIn(255))); \
	CFont::SetRightJustifyOn(); \
	CFont::SetScale(MENU_X(MENUHEADER_WIDTH), MENU_Y(MENUHEADER_HEIGHT)); \
	CFont::SetFontStyle(FONT_LOCALE(FONT_HEADING));

#define RESET_FONT_FOR_NEW_PAGE \
	CFont::SetBackgroundOff(); \
	CFont::SetScale(MENU_X(MENUACTION_SCALE_MULT), MENU_Y(MENUACTION_SCALE_MULT)); \
	CFont::SetPropOn(); \
	CFont::SetCentreOff(); \
	CFont::SetJustifyOn(); \
	CFont::SetRightJustifyOff(); \
	CFont::SetBackGroundOnlyTextOn(); \
	CFont::SetWrapx(MENU_X_RIGHT_ALIGNED(MENU_X_MARGIN)); \
	CFont::SetRightJustifyWrap(MENU_X_LEFT_ALIGNED(MENU_X_MARGIN - 2.0f));

#define SET_FONT_FOR_HELPER_TEXT \
	CFont::SetCentreOn(); \
	CFont::SetScale(MENU_X(SMALLESTTEXT_X_SCALE), MENU_Y(SMALLESTTEXT_Y_SCALE)); \
	CFont::SetFontStyle(FONT_LOCALE(FONT_HEADING));

#define SET_FONT_FOR_LIST_ITEM \
	CFont::SetRightJustifyOff(); \
	CFont::SetScale(MENU_X(SMALLESTTEXT_X_SCALE), MENU_Y(SMALLESTTEXT_Y_SCALE)); \
	CFont::SetFontStyle(FONT_LOCALE(FONT_BANK));

#define ProcessSlider(value, increaseAction, decreaseAction, hoverStartX, hoverEndX) \
	do { \
		lastActiveBarX = DisplaySlider(MENU_X_RIGHT_ALIGNED(MENUSLIDER_X + columnWidth), MENU_Y(bitAboveNextItemY), MENU_Y(smallestSliderBar), MENU_Y(usableLineHeight), MENU_X(MENUSLIDER_UNK), value); \
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
		if (m_nMousePosX < MENU_X_RIGHT_ALIGNED(MENUSLIDER_X + columnWidth)) \
			m_nHoverOption = HOVEROPTION_NOT_HOVERING; \
	} while(0)

#define ProcessRadioIcon(sprite, x, y, radioId, hoverOpt) \
	do { \
		sprite.Draw(x, y, MENU_X(MENURADIO_ICON_SCALE), MENU_Y(MENURADIO_ICON_SCALE), radioId == m_PrefsRadioStation ? CRGBA(255, 255, 255, 255) : \
			CRGBA(INACTIVE_RADIO_COLOR.r, INACTIVE_RADIO_COLOR.g, INACTIVE_RADIO_COLOR.b, INACTIVE_RADIO_COLOR.a)); \
			if (CheckHover(x, x + MENU_X(MENURADIO_ICON_SCALE), y, y + MENU_Y(MENURADIO_ICON_SCALE))) \
				m_nHoverOption = hoverOpt; \
	} while (0)

// --- Functions not in the game/inlined starts

void
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

void
CMenuManager::ScrollDownListByOne()
{
	if (m_nSelectedListRow == m_nFirstVisibleRowOnList + MAX_VISIBLE_OPTION_ON_SCREEN - 1) {
		if (m_nFirstVisibleRowOnList < m_nTotalListRow - MAX_VISIBLE_OPTION_ON_SCREEN) {
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

void
CMenuManager::PageUpList(bool playSoundOnSuccess)
{
	if (m_nTotalListRow > MAX_VISIBLE_OPTION_ON_SCREEN) {
		if (m_nFirstVisibleRowOnList > 0) {
			if(playSoundOnSuccess)
				DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_NAVIGATION, 0);

			m_nFirstVisibleRowOnList = Max(0, m_nFirstVisibleRowOnList - MAX_VISIBLE_OPTION_ON_SCREEN);
			m_nSelectedListRow = Min(m_nSelectedListRow, m_nFirstVisibleRowOnList + MAX_VISIBLE_OPTION_ON_SCREEN - 1);
		} else {
			m_nFirstVisibleRowOnList = 0;
			m_nSelectedListRow = 0;
		}
		m_nScrollbarTopMargin = (SCROLLBAR_MAX_HEIGHT / m_nTotalListRow) * m_nFirstVisibleRowOnList;
	}
}

void
CMenuManager::PageDownList(bool playSoundOnSuccess)
{
	if (m_nTotalListRow > MAX_VISIBLE_OPTION_ON_SCREEN) {
		if (m_nFirstVisibleRowOnList < m_nTotalListRow - MAX_VISIBLE_OPTION_ON_SCREEN) {
			if(playSoundOnSuccess)
				DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_NAVIGATION, 0);

			m_nFirstVisibleRowOnList = Min(m_nFirstVisibleRowOnList + MAX_VISIBLE_OPTION_ON_SCREEN, m_nTotalListRow - MAX_VISIBLE_OPTION_ON_SCREEN);
			m_nSelectedListRow = Max(m_nSelectedListRow, m_nFirstVisibleRowOnList);
		} else {
			m_nFirstVisibleRowOnList = m_nTotalListRow - MAX_VISIBLE_OPTION_ON_SCREEN;
			m_nSelectedListRow = m_nTotalListRow - 1;
		}
		m_nScrollbarTopMargin = (SCROLLBAR_MAX_HEIGHT / m_nTotalListRow) * m_nFirstVisibleRowOnList;
	}
}

#ifdef CUSTOM_FRONTEND_OPTIONS
bool ScreenHasOption(int screen, const char* gxtKey)
{
	for (int i = 0; i < NUM_MENUROWS; i++) {
		if (strcmp(gxtKey, aScreens[screen].m_aEntries[i].m_EntryName) == 0)
			return true;
	}
	return false;
}
#endif

void
CMenuManager::ThingsToDoBeforeGoingBack()
{
	if ((m_nCurrScreen == MENUPAGE_SKIN_SELECT) && strcmp(m_aSkinName, m_PrefsSkinFile) != 0) {
		CWorld::Players[0].SetPlayerSkin(m_PrefsSkinFile);
#ifdef CUSTOM_FRONTEND_OPTIONS
	} else if (ScreenHasOption(m_nCurrScreen, "FEA_3DH")) {
#else
	} else if (m_nCurrScreen == MENUPAGE_SOUND_SETTINGS) {
#endif
		if (m_nPrefsAudio3DProviderIndex != -1)
			m_nPrefsAudio3DProviderIndex = DMAudio.GetCurrent3DProviderIndex();
#ifdef TIDY_UP_PBP
		DMAudio.StopFrontEndTrack();
		OutputDebugString("FRONTEND AUDIO TRACK STOPPED");
#endif

#ifdef CUSTOM_FRONTEND_OPTIONS
	} else if (ScreenHasOption(m_nCurrScreen, "FED_RES")) {
#else
	} else if (m_nCurrScreen == MENUPAGE_DISPLAY_SETTINGS) {
#endif
		m_nDisplayVideoMode = m_nPrefsVideoMode;
	}

	if (m_nCurrScreen == MENUPAGE_SKIN_SELECT) {
		CPlayerSkin::EndFrontendSkinEdit();
	}

	if ((m_nCurrScreen == MENUPAGE_SKIN_SELECT) || (m_nCurrScreen == MENUPAGE_KEYBOARD_CONTROLS)) {
		m_nTotalListRow = 0;
	}

#ifdef SCROLLABLE_PAGES
	if (SCREEN_HAS_AUTO_SCROLLBAR) {
		m_nSelectedListRow = 0;
		m_nFirstVisibleRowOnList = 0;
		m_nScrollbarTopMargin = 0;
	}
#endif

#ifdef CUSTOM_FRONTEND_OPTIONS
	CMenuScreenCustom::CMenuEntry &option = aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption];

	if (option.m_Action == MENUACTION_CFO_DYNAMIC)
		if(option.m_CFODynamic->buttonPressFunc)
			option.m_CFODynamic->buttonPressFunc(FEOPTION_ACTION_FOCUSLOSS);

	if (option.m_Action == MENUACTION_CFO_SELECT && option.m_CFOSelect->onlyApplyOnEnter && option.m_CFOSelect->lastSavedValue != option.m_CFOSelect->displayedValue)
		option.m_CFOSelect->displayedValue = *option.m_CFO->value = option.m_CFOSelect->lastSavedValue;

	if (aScreens[m_nCurrScreen].returnPrevPageFunc) {
		aScreens[m_nCurrScreen].returnPrevPageFunc();
	}
#endif
}

int8
CMenuManager::GetPreviousPageOption()
{
#ifndef CUSTOM_FRONTEND_OPTIONS
	return !m_bGameNotLoaded ? aScreens[m_nCurrScreen].m_ParentEntry[1] : aScreens[m_nCurrScreen].m_ParentEntry[0];
#else
	int8 prevPage = !m_bGameNotLoaded ? aScreens[m_nCurrScreen].m_PreviousPage[1] : aScreens[m_nCurrScreen].m_PreviousPage[0];

	if (prevPage == -1) // Game also does same
		return 0;

	prevPage = prevPage == MENUPAGE_NONE ? (!m_bGameNotLoaded ? MENUPAGE_PAUSE_MENU : MENUPAGE_START_MENU) : prevPage;

	for (int i = 0; i < NUM_MENUROWS; i++) {
		if (aScreens[prevPage].m_aEntries[i].m_Action >= MENUACTION_NOTHING) { // CFO check
			if (aScreens[prevPage].m_aEntries[i].m_TargetMenu == m_nCurrScreen) {
				return i;
			}
		}
	}
	
	// This shouldn't happen
	return 0;
#endif
}

void
CMenuManager::ProcessList(bool &goBack, bool &optionSelected)
{
	if (m_nCurrScreen == MENUPAGE_SKIN_SELECT) {
		m_nTotalListRow = m_nSkinsTotal;
	}
	if (m_nCurrScreen == MENUPAGE_KEYBOARD_CONTROLS) {
		// GetNumOptionsCntrlConfigScreens would have been a better choice
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
		DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_NAVIGATION, 0);
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
			DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_NAVIGATION, 0);
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
			DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_NAVIGATION, 0);
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
				DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_NAVIGATION, 0);
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
				DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_NAVIGATION, 0);
				PageDownList(false);
			}
		}
		if (CPad::GetPad(0)->GetHome()) {
			m_nCurrExLayer = HOVEROPTION_LIST;
			m_bShowMouse = false;
			DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_NAVIGATION, 0);
			if (m_nTotalListRow >= MAX_VISIBLE_OPTION_ON_SCREEN) {
				m_nFirstVisibleRowOnList = 0;
			}
			m_nSelectedListRow = 0;
			m_nScrollbarTopMargin = (SCROLLBAR_MAX_HEIGHT / m_nTotalListRow) * m_nFirstVisibleRowOnList;
		}
		if (CPad::GetPad(0)->GetEnd()) {
			m_nCurrExLayer = HOVEROPTION_LIST;
			m_bShowMouse = false;
			DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_NAVIGATION, 0);
			if (m_nTotalListRow >= MAX_VISIBLE_OPTION_ON_SCREEN) {
				m_nFirstVisibleRowOnList = m_nTotalListRow - MAX_VISIBLE_OPTION_ON_SCREEN;
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
					DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_SETTING_CHANGE, 0);
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
}
// ------ Functions not in the game/inlined ends

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
#ifdef LOAD_INI_SETTINGS
		SaveINIControllerSettings();
#else
		SaveSettings();
#endif
	}

	if (escPressed) {
		pControlEdit = nil;
		m_bWaitingForNewKeyBind = false;
		m_KeyPressedCode = -1;
		m_bStartWaitingForKeyBind = false;
#ifdef LOAD_INI_SETTINGS
		SaveINIControllerSettings();
#else
		SaveSettings();
#endif
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
#ifdef FIX_BUGS
		TheCamera.m_fMouseAccelVertical = TheCamera.m_fMouseAccelHorzntl + 0.0005f;
#else
		TheCamera.m_fMouseAccelVertical = TheCamera.m_fMouseAccelHorzntl;
#endif
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

	SET_FONT_FOR_HELPER_TEXT
	// TODO: name this cases?
	switch (m_nHelperTextMsgId) {
		case 0:
		{
			int action = aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption].m_Action;
			if (action != MENUACTION_CHANGEMENU && action != MENUACTION_KEYBOARDCTRLS && action != MENUACTION_RESTOREDEF) {
				CFont::SetColor(CRGBA(255, 255, 255, 255));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(HELPER_TEXT_LEFT_MARGIN), SCREEN_SCALE_FROM_BOTTOM(HELPER_TEXT_BOTTOM_MARGIN), TheText.Get("FET_MIG"));
			}
			break;
		}
		case 1:
			CFont::SetColor(CRGBA(255, 255, 255, 255));
			CFont::PrintString(MENU_X_LEFT_ALIGNED(HELPER_TEXT_LEFT_MARGIN), SCREEN_SCALE_FROM_BOTTOM(HELPER_TEXT_BOTTOM_MARGIN), TheText.Get("FET_APP"));
			break;
		case 2:
			CFont::SetColor(CRGBA(255, 255, 255, alpha));
			CFont::PrintString(MENU_X_LEFT_ALIGNED(HELPER_TEXT_LEFT_MARGIN), SCREEN_SCALE_FROM_BOTTOM(HELPER_TEXT_BOTTOM_MARGIN), TheText.Get("FET_HRD"));
			break;
		case 3:
			CFont::SetColor(CRGBA(255, 255, 255, alpha));
			CFont::PrintString(MENU_X_LEFT_ALIGNED(HELPER_TEXT_LEFT_MARGIN), SCREEN_SCALE_FROM_BOTTOM(HELPER_TEXT_BOTTOM_MARGIN), TheText.Get("FET_RSO"));
			break;
		case 4:
			CFont::SetColor(CRGBA(255, 255, 255, alpha));
			CFont::PrintString(MENU_X_LEFT_ALIGNED(HELPER_TEXT_LEFT_MARGIN), SCREEN_SCALE_FROM_BOTTOM(HELPER_TEXT_BOTTOM_MARGIN), TheText.Get("FET_RSC"));
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

void
CMenuManager::DoSettingsBeforeStartingAGame()
{
#ifdef PC_PLAYER_CONTROLS
	CCamera::m_bUseMouse3rdPerson = m_ControlMethod == CONTROL_STANDARD;
#endif
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
#if GTA_VERSION >= GTA3_PC_11 && defined(DRAW_MENU_VERSION_TEXT)
	CFont::SetColor(CRGBA(LABEL_COLOR.r, LABEL_COLOR.g, LABEL_COLOR.b, FadeIn(255)));
	CFont::SetRightJustifyOn();
	CFont::SetFontStyle(FONT_HEADING);
	CFont::SetScale(MENU_X(0.7f), MENU_Y(0.5f));
	CFont::SetWrapx(SCREEN_WIDTH);
	CFont::SetRightJustifyWrap(0.0f);
	strcpy(gString, "V1.1");
	AsciiToUnicode(gString, gUString);
	CFont::PrintString(SCREEN_WIDTH / 10, SCREEN_HEIGHT / 45, gUString);
#endif
	CFont::SetWrapx(MENU_X_RIGHT_ALIGNED(MENU_X_MARGIN));
	CFont::SetRightJustifyWrap(MENU_X_LEFT_ALIGNED(MENU_X_MARGIN - 2.0f));

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
		
		SET_FONT_FOR_MENU_HEADER
		CFont::PrintString(PAGE_NAME_X(MENUHEADER_POS_X), SCREEN_SCALE_FROM_BOTTOM(MENUHEADER_POS_Y), TheText.Get(aScreens[m_nCurrScreen].m_ScreenName));

		// Weird place to put that.
		nextYToUse += 24.0f + 10.0f;
	}

	CFont::SetFontStyle(FONT_LOCALE(FONT_BANK));
	CFont::SetScale(MENU_X(MENUACTION_SCALE_MULT * MENU_TEXT_SIZE_X), MENU_Y(MENUACTION_SCALE_MULT * MENU_TEXT_SIZE_Y));
	CFont::SetRightJustifyOff();
	CFont::SetColor(CRGBA(LABEL_COLOR.r, LABEL_COLOR.g, LABEL_COLOR.b, FadeIn(255)));

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
		CFont::PrintString(MENU_X_LEFT_ALIGNED(MENU_X_MARGIN), MENU_Y(MENUACTION_POS_Y), str);
#else
		CFont::PrintString(MENU_X_MARGIN, MENUACTION_POS_Y, str);
#endif
	}

	// Not a bug, we just want HFoV+ on menu
#ifdef ASPECT_RATIO_SCALE
	CFont::SetCentreSize(SCREEN_SCALE_X(DEFAULT_SCREEN_WIDTH));
#else
	CFont::SetCentreSize(SCREEN_WIDTH);
#endif

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
			CFont::SetFontStyle(FONT_LOCALE(FONT_HEADING));
			CFont::SetScale(MENU_X(MENU_TEXT_SIZE_X = BIGTEXT_X_SCALE), MENU_Y(MENU_TEXT_SIZE_Y = BIGTEXT_Y_SCALE));
			CFont::SetCentreOn();
			break;
#ifdef FIX_BUGS
		case MENUPAGE_CONTROLLER_SETTINGS:
			columnWidth = 50;
			headerHeight = -50;
			lineHeight = 20;
			CFont::SetFontStyle(FONT_LOCALE(FONT_HEADING));
			CFont::SetScale(MENU_X(MENU_TEXT_SIZE_X = MEDIUMTEXT_X_SCALE), MENU_Y(MENU_TEXT_SIZE_Y = MEDIUMTEXT_Y_SCALE));
			CFont::SetRightJustifyOff();
			break;
#endif
		case MENUPAGE_SOUND_SETTINGS:
		case MENUPAGE_DISPLAY_SETTINGS:
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
			CFont::SetFontStyle(FONT_LOCALE(FONT_HEADING));
			CFont::SetScale(MENU_X(MENU_TEXT_SIZE_X = MEDIUMTEXT_X_SCALE), MENU_Y(MENU_TEXT_SIZE_Y = MEDIUMTEXT_Y_SCALE));
			CFont::SetRightJustifyOff();
			break;
		case MENUPAGE_CHOOSE_LOAD_SLOT:
		case MENUPAGE_CHOOSE_DELETE_SLOT:
		case MENUPAGE_CHOOSE_SAVE_SLOT:
			columnWidth = 120;
			headerHeight = 38;
			lineHeight = 20;
			CFont::SetFontStyle(FONT_LOCALE(FONT_BANK));
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
			CFont::SetFontStyle(FONT_LOCALE(FONT_HEADING));
			CFont::SetScale(MENU_X(MENU_TEXT_SIZE_X = BIGTEXT_X_SCALE), MENU_Y(MENU_TEXT_SIZE_Y = BIGTEXT_Y_SCALE));
			CFont::SetCentreOn();
			break;
		case MENUPAGE_START_MENU:
			columnWidth = 320;
			headerHeight = 140;
			lineHeight = 24;
			CFont::SetFontStyle(FONT_LOCALE(FONT_HEADING));
			CFont::SetScale(MENU_X(MENU_TEXT_SIZE_X = BIGTEXT_X_SCALE), MENU_Y(MENU_TEXT_SIZE_Y = BIGTEXT_Y_SCALE));
			CFont::SetCentreOn();
			break;
		case MENUPAGE_PAUSE_MENU:
			columnWidth = 320;
			headerHeight = 117;
			lineHeight = 24;
			CFont::SetFontStyle(FONT_LOCALE(FONT_HEADING));
			CFont::SetScale(MENU_X(MENU_TEXT_SIZE_X = BIGTEXT_X_SCALE), MENU_Y(MENU_TEXT_SIZE_Y = BIGTEXT_Y_SCALE));
			CFont::SetCentreOn();
			break;
#ifdef PS2_SAVE_DIALOG
		case MENUPAGE_SAVE:
			columnWidth = 180;
			headerHeight = 60;
			lineHeight = 24;
			CFont::SetFontStyle(FONT_LOCALE(FONT_BANK));
			CFont::SetScale(MENU_X(MENU_TEXT_SIZE_X = BIGTEXT_X_SCALE), MENU_Y(MENU_TEXT_SIZE_Y = BIGTEXT_Y_SCALE));
			break;
#endif
		default:
#ifdef CUSTOM_FRONTEND_OPTIONS
			CCustomScreenLayout *custom = aScreens[m_nCurrScreen].layout;
			if (custom) {
				columnWidth = custom->columnWidth;
				headerHeight = custom->headerHeight;
				lineHeight = custom->lineHeight;
				CFont::SetFontStyle(FONT_LOCALE(custom->font));
				CFont::SetScale(MENU_X(MENU_TEXT_SIZE_X = custom->fontScaleX), MENU_Y(MENU_TEXT_SIZE_Y = custom->fontScaleY));
				if (custom->alignment == FESCREEN_LEFT_ALIGN) {
					CFont::SetCentreOff();
					CFont::SetRightJustifyOff();
				} else if (custom->alignment == FESCREEN_RIGHT_ALIGN) {
					CFont::SetCentreOff();
					CFont::SetRightJustifyOn();
				} else {
					CFont::SetRightJustifyOff();
					CFont::SetCentreOn();
				}
			}
			if (!custom)
#endif
			{
				columnWidth = 320;
				headerHeight = 40;
				lineHeight = 24;
				CFont::SetFontStyle(FONT_LOCALE(FONT_HEADING));
				CFont::SetScale(MENU_X(MENU_TEXT_SIZE_X = BIGTEXT_X_SCALE), MENU_Y(MENU_TEXT_SIZE_Y = BIGTEXT_Y_SCALE));
				CFont::SetCentreOn();
			}
			break;
	}

#ifdef PS2_LIKE_MENU
	CFont::SetFontStyle(FONT_LOCALE(FONT_BANK));
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
#ifdef ASPECT_RATIO_SCALE
	char asciiTemp[32];
#endif

#ifdef MENU_MAP
	if (m_nCurrScreen == MENUPAGE_MAP) {
		// Back button
		wchar *backTx = TheText.Get("FEDS_TB");
		CFont::SetDropShadowPosition(1);
		CFont::SetDropColor(CRGBA(0, 0, 0, FadeIn(255)));
		CFont::PrintString(MENU_X(60.0f), SCREEN_SCALE_FROM_BOTTOM(120.0f), backTx);
		CFont::SetDropShadowPosition(0);
		if (!CheckHover(MENU_X(30.0f), MENU_X(30.0f) + CFont::GetStringWidth(backTx), SCREEN_SCALE_FROM_BOTTOM(125.0f), SCREEN_SCALE_FROM_BOTTOM(105.0f))) {
			m_nHoverOption = HOVEROPTION_NOT_HOVERING;
			m_nCurrOption = m_nOptionMouseHovering = 0;
		} else {
			m_nHoverOption = HOVEROPTION_RANDOM_ITEM;
			m_nCurrOption = m_nOptionMouseHovering = 1;
		}
		return;
	}
#endif

#ifdef CUSTOM_FRONTEND_OPTIONS
	// Thanks R*, for checking mouse hovering in Draw().
	static int lastSelectedOpt = m_nCurrOption;
#endif

#ifdef SCROLLABLE_PAGES
	int firstOption = SCREEN_HAS_AUTO_SCROLLBAR ? m_nFirstVisibleRowOnList : 0;
	for (int i = firstOption; i < firstOption + MAX_VISIBLE_OPTION && i < NUM_MENUROWS; ++i) {
#else
	for (int i = 0; i < NUM_MENUROWS; ++i) {
#endif

#ifdef CUSTOM_FRONTEND_OPTIONS
		bool isOptionDisabled = false;
#endif
		// Hide back button
#ifdef PS2_LIKE_MENU
		if ((i == NUM_MENUROWS - 1 || aScreens[m_nCurrScreen].m_aEntries[i+1].m_EntryName[0] == '\0') && strcmp(aScreens[m_nCurrScreen].m_aEntries[i].m_EntryName, "FEDS_TB") == 0)
			break;
#endif
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
					rightText = TheText.Get("FEM_AUT");
					break;
				case AR_4_3:
					sprintf(asciiTemp, "4:3");
					AsciiToUnicode(asciiTemp, unicodeTemp);
					rightText = unicodeTemp;
					break;
				case AR_5_4:
					sprintf(asciiTemp, "5:4");
					AsciiToUnicode(asciiTemp, unicodeTemp);
					rightText = unicodeTemp;
					break;
				case AR_16_10:
					sprintf(asciiTemp, "16:10");
					AsciiToUnicode(asciiTemp, unicodeTemp);
					rightText = unicodeTemp;
					break;
				case AR_16_9:
					sprintf(asciiTemp, "16:9");
					AsciiToUnicode(asciiTemp, unicodeTemp);
					rightText = unicodeTemp;
					break;
				case AR_21_9:
					sprintf(asciiTemp, "21:9");
					AsciiToUnicode(asciiTemp, unicodeTemp);
					rightText = unicodeTemp;
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
#ifdef CUSTOM_FRONTEND_OPTIONS
			case MENUACTION_CFO_DYNAMIC:
			case MENUACTION_CFO_SELECT:
				CMenuScreenCustom::CMenuEntry &option = aScreens[m_nCurrScreen].m_aEntries[i];
				if (option.m_Action == MENUACTION_CFO_SELECT) {

					isOptionDisabled = option.m_CFOSelect->disableIfGameLoaded && !m_bGameNotLoaded;
					if (option.m_CFOSelect->onlyApplyOnEnter){
						if (m_nCurrOption != i) {
							if (option.m_CFOSelect->displayedValue != option.m_CFOSelect->lastSavedValue)
								SetHelperText(3); // Restored original value

							// If that was previously selected option, restore it to default value.
							// if (m_nCurrOption != lastSelectedOpt && lastSelectedOpt == i)
								option.m_CFOSelect->displayedValue = option.m_CFOSelect->lastSavedValue = *option.m_CFO->value;

						} else {
							if (option.m_CFOSelect->displayedValue != *option.m_CFO->value)
								SetHelperText(1); // Enter to apply
							else if (m_nHelperTextMsgId == 1)
								ResetHelperText(); // Applied
						}
					}

					// To whom manipulate option.m_CFO->value of select options externally (like RestoreDef functions)
					if (*option.m_CFO->value != option.m_CFOSelect->lastSavedValue)
						option.m_CFOSelect->displayedValue = option.m_CFOSelect->lastSavedValue = *option.m_CFO->value;

					if (option.m_CFOSelect->displayedValue >= option.m_CFOSelect->numRightTexts || option.m_CFOSelect->displayedValue < 0)
						option.m_CFOSelect->displayedValue = 0;

					rightText = TheText.Get(option.m_CFOSelect->rightTexts[option.m_CFOSelect->displayedValue]);

				} else if (option.m_Action == MENUACTION_CFO_DYNAMIC) {
					if (m_nCurrOption != lastSelectedOpt && lastSelectedOpt == i) {
						if(option.m_CFODynamic->buttonPressFunc)
							option.m_CFODynamic->buttonPressFunc(FEOPTION_ACTION_FOCUSLOSS);
					}

					if (option.m_CFODynamic->drawFunc) {
						rightText = option.m_CFODynamic->drawFunc(&isOptionDisabled, m_nCurrOption == i);
					}
				}
				break;
#endif
			}

			float nextItemY = headerHeight + nextYToUse;
			float bitAboveNextItemY = nextItemY - 2.0f;
			int nextYToCheck = bitAboveNextItemY;
			
			if (!foundTheHoveringItem) {
#ifdef SCROLLABLE_PAGES
				for (int rowToCheck = firstOption + (aScreens[m_nCurrScreen].m_aEntries[firstOption].m_Action == MENUACTION_LABEL); rowToCheck < firstOption + MAX_VISIBLE_OPTION && rowToCheck < NUM_MENUROWS; ++rowToCheck) {
#else
				for (int rowToCheck = aScreens[m_nCurrScreen].m_aEntries[0].m_Action == MENUACTION_LABEL; rowToCheck < NUM_MENUROWS; ++rowToCheck) {
#endif
					if(aScreens[m_nCurrScreen].m_aEntries[rowToCheck].m_Action == MENUACTION_NOTHING)
						break;
						
					// Hide back button
#ifdef PS2_LIKE_MENU
					if ((rowToCheck == NUM_MENUROWS - 1 || aScreens[m_nCurrScreen].m_aEntries[rowToCheck+1].m_EntryName[0] == '\0') &&
						strcmp(aScreens[m_nCurrScreen].m_aEntries[rowToCheck].m_EntryName, "FEDS_TB") == 0)
						break;
#endif

					int extraOffset = 0;
					if (aScreens[m_nCurrScreen].m_aEntries[rowToCheck].m_Action == MENUACTION_RADIO)
						extraOffset = MENURADIO_ICON_SCALE;

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

						m_nOptionMouseHovering = rowToCheck;
						if (m_nMouseOldPosX != m_nMousePosX || m_nMouseOldPosY != m_nMousePosY) {
							m_nCurrOption = rowToCheck;
							m_bShowMouse = true;
						}
						if (oldOption != m_nCurrOption) {
							if (oldScreen == m_nCurrScreen && m_bShowMouse)
								DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_NAVIGATION, 0);

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
#ifdef PS2_LIKE_MENU
				CSprite2d::DrawRect(CRect(MENU_X_LEFT_ALIGNED(29.0f), MENU_Y(bitAboveNextItemY),
											MENU_X_RIGHT_ALIGNED(29.0f), MENU_Y(usableLineHeight + nextItemY)),
											CRGBA(SELECTION_HIGHLIGHTBG_COLOR.r, SELECTION_HIGHLIGHTBG_COLOR.g, SELECTION_HIGHLIGHTBG_COLOR.b, FadeIn(SELECTION_HIGHLIGHTBG_COLOR.a)));
#else
				// We keep stretching, because we also stretch background image and we want that bar to be aligned with borders of background
				CSprite2d::DrawRect(CRect(StretchX(10.0f), MENU_Y(bitAboveNextItemY),
					SCREEN_STRETCH_FROM_RIGHT(11.0f), MENU_Y(usableLineHeight + nextItemY)),
					CRGBA(SELECTION_HIGHLIGHTBG_COLOR.r, SELECTION_HIGHLIGHTBG_COLOR.g, SELECTION_HIGHLIGHTBG_COLOR.b, FadeIn(SELECTION_HIGHLIGHTBG_COLOR.a)));
#endif
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
					
					if(textLayer == 1)
						if (!strcmp(aScreens[m_nCurrScreen].m_aEntries[i].m_EntryName, "FED_RES") && !m_bGameNotLoaded
#ifdef CUSTOM_FRONTEND_OPTIONS
							|| isOptionDisabled
#endif
							)
							CFont::SetColor(CRGBA(DARKMENUOPTION_COLOR.r, DARKMENUOPTION_COLOR.g, DARKMENUOPTION_COLOR.b, FadeIn(255)));

					CFont::PrintString(MENU_X_RIGHT_ALIGNED(columnWidth - textLayer), itemY, rightText);
				}
				if (i == m_nCurrOption && itemsAreSelectable){
					CFont::SetColor(CRGBA(SELECTEDMENUOPTION_COLOR.r, SELECTEDMENUOPTION_COLOR.g, SELECTEDMENUOPTION_COLOR.b, FadeIn(255)));
				} else {
					CFont::SetColor(CRGBA(MENUOPTION_COLOR.r, MENUOPTION_COLOR.g, MENUOPTION_COLOR.b, FadeIn(255)));
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
					// To make assigning built-in actions to new custom options possible.
#ifdef CUSTOM_FRONTEND_OPTIONS
					&& ScreenHasOption(m_nCurrScreen, "FEA_3DH")
#else
					&& m_nCurrScreen == MENUPAGE_SOUND_SETTINGS
#endif
					&& m_nPrefsAudio3DProviderIndex != -1) {

					m_nPrefsAudio3DProviderIndex = DMAudio.GetCurrent3DProviderIndex();
					SetHelperText(3);
				}
			}
			if (m_nDisplayVideoMode != m_nPrefsVideoMode) {
				if (strcmp(aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption].m_EntryName, "FED_RES") != 0
					// To make assigning built-in actions to new custom options possible.
#ifdef CUSTOM_FRONTEND_OPTIONS
					&& ScreenHasOption(m_nCurrScreen, "FED_RES")
#else
					&& m_nCurrScreen == MENUPAGE_DISPLAY_SETTINGS
#endif
					){
					m_nDisplayVideoMode = m_nPrefsVideoMode;
					SetHelperText(3);
				}
			}

			// Sliders
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

			// Needed after the bug fix in Font.cpp
#ifdef FIX_BUGS
			if (!CFont::Details.centre)
				CFont::SetRightJustifyOff();
#endif

			// 60.0 is silly
			nextYToUse += lineHeight * CFont::GetNumberLines(MENU_X_LEFT_ALIGNED(60.0f), MENU_Y(nextYToUse), leftText);

			// Radio icons
			if (aScreens[m_nCurrScreen].m_aEntries[i].m_Action == MENUACTION_RADIO) {
				ProcessRadioIcon(m_aFrontEndSprites[FE_RADIO1], MENU_X_LEFT_ALIGNED(30.0f), MENU_Y(nextYToUse), 0, HOVEROPTION_RADIO_0);
				ProcessRadioIcon(m_aFrontEndSprites[FE_RADIO2], MENU_X_LEFT_ALIGNED(90.0f), MENU_Y(nextYToUse), 1, HOVEROPTION_RADIO_1);
				ProcessRadioIcon(m_aFrontEndSprites[FE_RADIO5], MENU_X_LEFT_ALIGNED(150.0f), MENU_Y(nextYToUse), 2, HOVEROPTION_RADIO_2);
				ProcessRadioIcon(m_aFrontEndSprites[FE_RADIO7], MENU_X_LEFT_ALIGNED(210.0f), MENU_Y(nextYToUse), 3, HOVEROPTION_RADIO_3);
				ProcessRadioIcon(m_aFrontEndSprites[FE_RADIO8], MENU_X_LEFT_ALIGNED(270.0f), MENU_Y(nextYToUse), 4, HOVEROPTION_RADIO_4);
				ProcessRadioIcon(m_aFrontEndSprites[FE_RADIO3], MENU_X_LEFT_ALIGNED(320.0f), MENU_Y(nextYToUse), 5, HOVEROPTION_RADIO_5);
				ProcessRadioIcon(m_aFrontEndSprites[FE_RADIO4], MENU_X_LEFT_ALIGNED(360.0f), MENU_Y(nextYToUse), 6, HOVEROPTION_RADIO_6);
				ProcessRadioIcon(m_aFrontEndSprites[FE_RADIO6], MENU_X_LEFT_ALIGNED(420.0f), MENU_Y(nextYToUse), 7, HOVEROPTION_RADIO_7);
				ProcessRadioIcon(m_aFrontEndSprites[FE_RADIO9], MENU_X_LEFT_ALIGNED(480.0f), MENU_Y(nextYToUse), 8, HOVEROPTION_RADIO_8);

				if (DMAudio.IsMP3RadioChannelAvailable())
					ProcessRadioIcon(m_aMenuSprites[MENUSPRITE_MP3LOGO], MENU_X_LEFT_ALIGNED(540.0f), MENU_Y(nextYToUse), 9, HOVEROPTION_RADIO_9);

				nextYToUse += 70.0f;
			}
		}
	}

#ifdef CUSTOM_FRONTEND_OPTIONS
	lastSelectedOpt = m_nCurrOption;
#endif

#ifdef SCROLLABLE_PAGES
	#define SCROLLBAR_BOTTOM_Y 125.0f // only for background, scrollbar's itself is calculated
	#define SCROLLBAR_RIGHT_X 36.0f
	#define SCROLLBAR_WIDTH 9.5f
	#define SCROLLBAR_TOP_Y 64

	if (SCREEN_HAS_AUTO_SCROLLBAR) {
		// Scrollbar background
		CSprite2d::DrawRect(CRect(MENU_X_RIGHT_ALIGNED(SCROLLBAR_RIGHT_X - 2), MENU_Y(SCROLLBAR_TOP_Y),
			MENU_X_RIGHT_ALIGNED(SCROLLBAR_RIGHT_X - 2 - SCROLLBAR_WIDTH), SCREEN_SCALE_FROM_BOTTOM(SCROLLBAR_BOTTOM_Y)), CRGBA(100, 100, 66, FadeIn(205)));
		
		float scrollbarHeight = SCROLLBAR_MAX_HEIGHT / (m_nTotalListRow / (float) MAX_VISIBLE_OPTION);
		float scrollbarBottom, scrollbarTop;

		scrollbarBottom = MENU_Y(SCROLLBAR_TOP_Y - 8 + m_nScrollbarTopMargin + scrollbarHeight);
		scrollbarTop = MENU_Y(SCROLLBAR_TOP_Y + m_nScrollbarTopMargin);
		// Scrollbar shadow
		CSprite2d::DrawRect(CRect(MENU_X_RIGHT_ALIGNED(SCROLLBAR_RIGHT_X - 4), scrollbarTop,
			MENU_X_RIGHT_ALIGNED(SCROLLBAR_RIGHT_X - 1 - SCROLLBAR_WIDTH), scrollbarBottom + MENU_Y(1.0f)),
			CRGBA(50, 50, 50, FadeIn(255)));

		// Scrollbar
		CSprite2d::DrawRect(CRect(MENU_X_RIGHT_ALIGNED(SCROLLBAR_RIGHT_X - 4), scrollbarTop,
			MENU_X_RIGHT_ALIGNED(SCROLLBAR_RIGHT_X - SCROLLBAR_WIDTH), scrollbarBottom),
			CRGBA(SCROLLBAR_COLOR.r, SCROLLBAR_COLOR.g, SCROLLBAR_COLOR.b, FadeIn(255)));
			
	}
#endif

	switch (m_nCurrScreen) {
	case MENUPAGE_CONTROLLER_SETTINGS:
	case MENUPAGE_SOUND_SETTINGS:
	case MENUPAGE_DISPLAY_SETTINGS:
	case MENUPAGE_SKIN_SELECT:
	case MENUPAGE_CONTROLLER_PC:
	case MENUPAGE_MOUSE_CONTROLS:
		DisplayHelperText();
		break;
#ifdef CUSTOM_FRONTEND_OPTIONS
	default:
		if (aScreens[m_nCurrScreen].layout) {
			if (aScreens[m_nCurrScreen].layout->showLeftRightHelper) {
				DisplayHelperText();
			}
		}
		break;
#endif
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
	int nextY = MENU_Y(yStart);
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
	for (int optionIdx = 0; optionIdx < numOptions; nextY = MENU_Y(++optionIdx * rowHeight + yStart)) {
		int nextX = xStart;
		int bindingsForThisOpt = 0;
		int contSetOrder = SETORDER_1;
		CFont::SetColor(CRGBA(LIST_OPTION_COLOR.r, LIST_OPTION_COLOR.g, LIST_OPTION_COLOR.b, FadeIn(LIST_OPTION_COLOR.a)));

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
#ifdef BIND_VEHICLE_FIREWEAPON
 					controllerAction = VEHICLE_FIREWEAPON;
#else
 					controllerAction = PED_FIREWEAPON;
#endif
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
							MENU_Y(bgY + CONTSETUP_BOUND_HIGHLIGHT_HEIGHT)), CRGBA(CONTSETUP_DISABLED_HIGHLIGHTBG_COLOR.r, CONTSETUP_DISABLED_HIGHLIGHTBG_COLOR.g, CONTSETUP_DISABLED_HIGHLIGHTBG_COLOR.b, FadeIn(CONTSETUP_DISABLED_HIGHLIGHTBG_COLOR.a)));
					} else {
						CSprite2d::DrawRect(CRect(nextX, MENU_Y(bgY), nextX + MENU_X(CONTSETUP_BOUND_COLUMN_WIDTH),
							MENU_Y(bgY + CONTSETUP_BOUND_HIGHLIGHT_HEIGHT)), CRGBA(CONTSETUP_HIGHLIGHTBG_COLOR.r, CONTSETUP_HIGHLIGHTBG_COLOR.g, CONTSETUP_HIGHLIGHTBG_COLOR.b, FadeIn(CONTSETUP_HIGHLIGHTBG_COLOR.a)));
					}
#else
					if (controllerAction == -1) {
						CSprite2d::DrawRect(CRect(MENU_X_LEFT_ALIGNED(210.0f), MENU_Y(bgY),
							MENU_X_LEFT_ALIGNED(400.0f), MENU_Y(bgY + CONTSETUP_BOUND_HIGHLIGHT_HEIGHT)), CRGBA(CONTSETUP_DISABLED_HIGHLIGHTBG_COLOR.r, CONTSETUP_DISABLED_HIGHLIGHTBG_COLOR.g, CONTSETUP_DISABLED_HIGHLIGHTBG_COLOR.b, FadeIn(CONTSETUP_DISABLED_HIGHLIGHTBG_COLOR.a)));
					} else {
						CSprite2d::DrawRect(CRect(MENU_X_LEFT_ALIGNED(210.0f), MENU_Y(bgY),
							MENU_X_LEFT_ALIGNED(400.0f), MENU_Y(bgY + CONTSETUP_BOUND_HIGHLIGHT_HEIGHT)), CRGBA(CONTSETUP_HIGHLIGHTBG_COLOR.r, CONTSETUP_HIGHLIGHTBG_COLOR.g, CONTSETUP_HIGHLIGHTBG_COLOR.b, FadeIn(CONTSETUP_HIGHLIGHTBG_COLOR.a)));
					}
#endif
					CFont::SetColor(CRGBA(0, 0, 0, FadeIn(255)));
					bindingWhite = 0;

				} else if (column == CONTSETUP_VEHICLE_COLUMN && m_nSelectedContSetupColumn == CONTSETUP_VEHICLE_COLUMN) {
#ifdef FIX_BUGS
					if (controllerAction == -1) {
						CSprite2d::DrawRect(CRect(nextX, MENU_Y(bgY), nextX + MENU_X(CONTSETUP_BOUND_COLUMN_WIDTH),
							MENU_Y(bgY + CONTSETUP_BOUND_HIGHLIGHT_HEIGHT)), CRGBA(CONTSETUP_DISABLED_HIGHLIGHTBG_COLOR.r, CONTSETUP_DISABLED_HIGHLIGHTBG_COLOR.g, CONTSETUP_DISABLED_HIGHLIGHTBG_COLOR.b, FadeIn(CONTSETUP_DISABLED_HIGHLIGHTBG_COLOR.a)));
					} else {
						CSprite2d::DrawRect(CRect(nextX, MENU_Y(bgY), nextX + MENU_X(CONTSETUP_BOUND_COLUMN_WIDTH),
							MENU_Y(bgY + CONTSETUP_BOUND_HIGHLIGHT_HEIGHT)), CRGBA(CONTSETUP_HIGHLIGHTBG_COLOR.r, CONTSETUP_HIGHLIGHTBG_COLOR.g, CONTSETUP_HIGHLIGHTBG_COLOR.b, FadeIn(CONTSETUP_HIGHLIGHTBG_COLOR.a)));
					}
#else
					if (controllerAction == -1) {
						CSprite2d::DrawRect(CRect(MENU_X_LEFT_ALIGNED(410.0f), MENU_Y(bgY), MENU_X_LEFT_ALIGNED(600.0f), MENU_Y(bgY + 10)), CRGBA(CONTSETUP_DISABLED_HIGHLIGHTBG_COLOR.r, CONTSETUP_DISABLED_HIGHLIGHTBG_COLOR.g, CONTSETUP_DISABLED_HIGHLIGHTBG_COLOR.b, FadeIn(CONTSETUP_DISABLED_HIGHLIGHTBG_COLOR.a)));
					} else {
						CSprite2d::DrawRect(CRect(MENU_X_LEFT_ALIGNED(410.0f), MENU_Y(bgY), MENU_X_LEFT_ALIGNED(600.0f), MENU_Y(bgY + 10)), CRGBA(CONTSETUP_HIGHLIGHTBG_COLOR.r, CONTSETUP_HIGHLIGHTBG_COLOR.g, CONTSETUP_HIGHLIGHTBG_COLOR.b, FadeIn(CONTSETUP_HIGHLIGHTBG_COLOR.a)));
					}
#endif
					CFont::SetColor(CRGBA(0, 0, 0, FadeIn(255)));
					bindingWhite = 0;
				}
			}
		}

		// Print bindings, including seperator (-) between them
		CFont::SetScale(MENU_X(0.25f), MENU_Y(SMALLESTTEXT_Y_SCALE));
		for (; contSetOrder < MAX_SETORDERS && controllerAction != -1; contSetOrder++) {
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
					static uint32 lastWaitingTextFlash = 0;
					if (CTimer::GetTimeInMillisecondsPauseMode() - lastWaitingTextFlash > 150) {
						showWaitingText = !showWaitingText;
						lastWaitingTextFlash = CTimer::GetTimeInMillisecondsPauseMode();
					}
					if (showWaitingText) {
						CFont::SetColor(CRGBA(55, 55, 55, FadeIn(255)));
						CFont::PrintString(nextX, nextY, TheText.Get("FEC_QUE")); // "???"
					}
					SET_FONT_FOR_HELPER_TEXT
					CFont::SetColor(CRGBA(255, 255, 255, FadeIn(255)));
					if (m_bKeyChangeNotProcessed) {
						CFont::PrintString(MENU_X_LEFT_ALIGNED(275.0f), SCREEN_SCALE_FROM_BOTTOM(114.0f), TheText.Get("FET_CIG")); // BACKSPACE TO CLEAR - LMB,RETURN TO CHANGE
					} else {
						CFont::PrintString(MENU_X_LEFT_ALIGNED(275.0f), SCREEN_SCALE_FROM_BOTTOM(114.0f), TheText.Get("FET_RIG")); // SELECT A NEW CONTROL FOR THIS ACTION OR ESC TO CANCEL
					}
					
					SET_FONT_FOR_LIST_ITEM
					if (!m_bKeyIsOK)
						DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_SETTING_CHANGE, 0);

					m_bKeyIsOK = true;
				} else {
					SET_FONT_FOR_HELPER_TEXT
					CFont::SetColor(CRGBA(255, 255, 255, FadeIn(255)));
					CFont::PrintString(MENU_X_LEFT_ALIGNED(275.0f), SCREEN_SCALE_FROM_BOTTOM(114.0f), TheText.Get("FET_CIG")); // BACKSPACE TO CLEAR - LMB,RETURN TO CHANGE
					SET_FONT_FOR_LIST_ITEM
					m_bKeyIsOK = false;
					m_bKeyChangeNotProcessed = false;
				}
			} else if (optionIdx == m_nSelectedListRow) {
				SET_FONT_FOR_HELPER_TEXT
				CFont::SetColor(CRGBA(55, 55, 55, FadeIn(255)));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(275.0f), SCREEN_SCALE_FROM_BOTTOM(114.0f), TheText.Get("FET_EIG")); // CANNOT SET A CONTROL FOR THIS ACTION
				SET_FONT_FOR_LIST_ITEM
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
				CFont::SetColor(CRGBA(MENUOPTION_COLOR.r, MENUOPTION_COLOR.g, MENUOPTION_COLOR.b, FadeIn(255)));
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
	RESET_FONT_FOR_NEW_PAGE

	SET_FONT_FOR_MENU_HEADER

	switch (m_ControlMethod) {
		case CONTROL_STANDARD:
			CFont::PrintString(PAGE_NAME_X(MENUHEADER_POS_X), SCREEN_SCALE_FROM_BOTTOM(MENUHEADER_POS_Y),
				TheText.Get(aScreens[m_nCurrScreen].m_ScreenName));
			break;
		case CONTROL_CLASSIC:
			CFont::PrintString(PAGE_NAME_X(MENUHEADER_POS_X), SCREEN_SCALE_FROM_BOTTOM(MENUHEADER_POS_Y),
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
		CRGBA(LIST_BACKGROUND_COLOR.r, LIST_BACKGROUND_COLOR.g, LIST_BACKGROUND_COLOR.b, FadeIn(LIST_BACKGROUND_COLOR.a)));

	if (m_nCurrExLayer == HOVEROPTION_LIST)
		CFont::SetColor(CRGBA(SELECTEDMENUOPTION_COLOR.r, SELECTEDMENUOPTION_COLOR.g, SELECTEDMENUOPTION_COLOR.b, FadeIn(255)));
	else
		CFont::SetColor(CRGBA(MENUOPTION_COLOR.r, MENUOPTION_COLOR.g, MENUOPTION_COLOR.b, FadeIn(255)));

	// List header
	CFont::SetFontStyle(FONT_LOCALE(FONT_HEADING));
	CFont::SetScale(MENU_X(MENUACTION_SCALE_MULT), MENU_Y(MENUACTION_SCALE_MULT));
	CFont::SetRightJustifyOff();
	CFont::PrintString(MENU_X_LEFT_ALIGNED(CONTSETUP_COLUMN_1_X), MENU_Y(CONTSETUP_LIST_TOP), TheText.Get("FET_CAC"));
	CFont::PrintString(MENU_X_LEFT_ALIGNED(CONTSETUP_COLUMN_2_X), MENU_Y(CONTSETUP_LIST_TOP), TheText.Get("FET_CFT"));
	CFont::PrintString(MENU_X_LEFT_ALIGNED(CONTSETUP_COLUMN_3_X), MENU_Y(CONTSETUP_LIST_TOP), TheText.Get("FET_CCR"));
	SET_FONT_FOR_LIST_ITEM
	
	int yStart;
	if (m_ControlMethod == CONTROL_CLASSIC)
		yStart = CONTSETUP_LIST_TOP + CONTSETUP_LIST_HEADER_HEIGHT + 1;
	else
		yStart = CONTSETUP_LIST_TOP + CONTSETUP_LIST_HEADER_HEIGHT + 5;

	float optionYBottom = yStart + rowHeight;
	for (int i = 0; i < ARRAY_SIZE(actionTexts); ++i) {
		wchar *actionText = actionTexts[i];
		if (!actionText)
			break;

		if (m_nMousePosX > MENU_X_LEFT_ALIGNED(CONTSETUP_LIST_LEFT + 2.0f) &&
			m_nMousePosX < MENU_X_LEFT_ALIGNED(CONTSETUP_COLUMN_3_X + CONTSETUP_BOUND_COLUMN_WIDTH)) {

			if (m_nMousePosY > MENU_Y(i * rowHeight + yStart) && m_nMousePosY < MENU_Y(i * rowHeight + optionYBottom)) {
					if (m_nOptionMouseHovering != i && m_nCurrExLayer == HOVEROPTION_LIST)
						DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_NAVIGATION, 0);

					m_nOptionMouseHovering = i;
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
								DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_NAVIGATION, 0);

							m_nSelectedContSetupColumn = CONTSETUP_PED_COLUMN;
#ifdef FIX_BUGS
						} else if (m_nMousePosX > MENU_X_LEFT_ALIGNED(CONTSETUP_COLUMN_2_X + CONTSETUP_BOUND_COLUMN_WIDTH) && m_nMousePosX < SCREEN_WIDTH) {
#else
						} else if (m_nMousePosX > MENU_X_LEFT_ALIGNED(370.0f) && m_nMousePosX < SCREEN_WIDTH) {
#endif
							if (m_nSelectedContSetupColumn != CONTSETUP_VEHICLE_COLUMN && m_nCurrExLayer == HOVEROPTION_LIST)
								DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_NAVIGATION, 0);

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
		if (m_nSelectedListRow != i)
			CFont::SetColor(CRGBA(MENUOPTION_COLOR.r, MENUOPTION_COLOR.g, MENUOPTION_COLOR.b, FadeIn(255)));
		else if (m_nCurrExLayer == HOVEROPTION_LIST)
			CFont::SetColor(CRGBA(SELECTEDMENUOPTION_COLOR.r, SELECTEDMENUOPTION_COLOR.g, SELECTEDMENUOPTION_COLOR.b, FadeIn(255)));

		CFont::SetRightJustifyOff();
		if (m_PrefsLanguage == LANGUAGE_GERMAN && (i == 20 || i == 21))
			CFont::SetScale(MENU_X(0.32f), MENU_Y(SMALLESTTEXT_Y_SCALE));
		else
			CFont::SetScale(MENU_X(SMALLESTTEXT_X_SCALE), MENU_Y(SMALLESTTEXT_Y_SCALE));

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
			CFont::SetColor(CRGBA(SELECTEDMENUOPTION_COLOR.r, SELECTEDMENUOPTION_COLOR.g, SELECTEDMENUOPTION_COLOR.b, FadeIn(255)));
		else
			CFont::SetColor(CRGBA(MENUOPTION_COLOR.r, MENUOPTION_COLOR.g, MENUOPTION_COLOR.b, FadeIn(255)));
	}
}

void
CMenuManager::DrawFrontEnd()
{
	CFont::SetAlphaFade(255.0f);

#ifdef PS2_LIKE_MENU
	#define setBbItem(a, b, c) strcpy(a.name, b); a.screenId = c;
	if (m_nCurrScreen == MENUPAGE_NONE) {
		if (m_bGameNotLoaded) {
			if (bbTabCount != 6) {
				setBbItem(bbNames[0], "FEB_SAV",MENUPAGE_NEW_GAME)
				setBbItem(bbNames[1], "FEB_CON",MENUPAGE_CONTROLLER_PC)
				setBbItem(bbNames[2], "FEB_AUD",MENUPAGE_SOUND_SETTINGS)
				setBbItem(bbNames[3], "FEB_DIS",MENUPAGE_DISPLAY_SETTINGS)
				setBbItem(bbNames[4], "FEB_LAN",MENUPAGE_LANGUAGE_SETTINGS)
				setBbItem(bbNames[5], "FESZ_QU",MENUPAGE_EXIT)
				bbTabCount = 6;
			}
		} else {
			if (bbTabCount != 8) {
				setBbItem(bbNames[0], "FEB_STA",MENUPAGE_STATS)
				setBbItem(bbNames[1], "FEB_SAV",MENUPAGE_NEW_GAME)
				setBbItem(bbNames[2], "FEB_BRI",MENUPAGE_BRIEFS)
				setBbItem(bbNames[3], "FEB_CON",MENUPAGE_CONTROLLER_PC)
				setBbItem(bbNames[4], "FEB_AUD",MENUPAGE_SOUND_SETTINGS)
				setBbItem(bbNames[5], "FEB_DIS",MENUPAGE_DISPLAY_SETTINGS)
				setBbItem(bbNames[6], "FEB_LAN",MENUPAGE_LANGUAGE_SETTINGS)
				setBbItem(bbNames[7], "FESZ_QU",MENUPAGE_EXIT)
				bbTabCount = 8;
			}
		}
		m_nCurrScreen = bbNames[0].screenId;
		bottomBarActive = true;
		curBottomBarOption = 0;
	}
	#undef setBbItem
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
		bg->Draw(CRect(0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT), CRGBA(48, 48, 48, 255));
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
		case MENUPAGE_DISPLAY_SETTINGS:
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

	static float fadeAlpha = 0.0f;

	if (m_nMenuFadeAlpha < 255) {
		if (m_nMenuFadeAlpha == 0 && fadeAlpha > 1.0f) fadeAlpha = 0.0f;
		
		// +20 per every 33 ms (1000.f/30.f - original frame limiter fps)
		fadeAlpha += (frameTime) * 20.f / 33.f;
		m_nMenuFadeAlpha = fadeAlpha;
	} else {
		// TODO: what is this? waiting mouse?
		if(field_518 == 4){
			if(m_nHoverOption == HOVEROPTION_3 || m_nHoverOption == HOVEROPTION_4 ||
				m_nHoverOption == HOVEROPTION_5 || m_nHoverOption == HOVEROPTION_6 || m_nHoverOption == HOVEROPTION_7)

				field_518 = 2;
			else
				field_518 = 1;
		}
	}

	m_aFrontEndSprites[currentSprite].Draw(CRect(MENU_X_LEFT_ALIGNED(50.0f), MENU_Y(50.0f), MENU_X_RIGHT_ALIGNED(50.0f), SCREEN_SCALE_FROM_BOTTOM(95.0f)), CRGBA(255, 255, 255, m_nMenuFadeAlpha > 255 ? 255 : m_nMenuFadeAlpha));

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

	// Positions/style from PS2 menu, credits to Fire_Head
	/* Draw controller buttons */
	CFont::SetFontStyle(FONT_BANK);
	CFont::SetBackgroundOff();
	CFont::SetScale(SCREEN_SCALE_X(0.35f), SCREEN_SCALE_Y(0.64f));
	CFont::SetPropOn();
	CFont::SetCentreOff();
	CFont::SetJustifyOn();
	CFont::SetRightJustifyOff();
	CFont::SetBackGroundOnlyTextOn();
	CFont::SetWrapx(MENU_X_RIGHT_ALIGNED(MENU_X_MARGIN)); // 600.0f
	CFont::SetColor(CRGBA(16, 16, 16, 255));
	switch (m_nCurrScreen) {

		// Page names overlaps buttons on those.
		case MENUPAGE_MOUSE_CONTROLS:
		case MENUPAGE_KEYBOARD_CONTROLS:
			break;

		default:
		{
			CFont::PrintString(MENU_X_LEFT_ALIGNED(52.0f), MENU_Y(360.0f), TheText.Get("FEDS_SE"));
			CFont::PrintString(MENU_X_LEFT_ALIGNED(52.0f), MENU_Y(372.0f), TheText.Get("FEDS_BA"));
			if (!m_bGameNotLoaded)
				CFont::PrintString(MENU_X_LEFT_ALIGNED(52.0f), MENU_Y(384.0f), TheText.Get("FEDS_ST"));

			if (bottomBarActive)
				CFont::PrintString(MENU_X_LEFT_ALIGNED(242.0f), MENU_Y(372.0f), TheText.Get("FEDS_AM")); // <>-CHANGE MENU
			else if (m_nCurrScreen != MENUPAGE_STATS && m_nCurrScreen != MENUPAGE_BRIEFS) {
				CFont::PrintString(MENU_X_LEFT_ALIGNED(242.0f), MENU_Y(360.0f + 3.5f), TheText.Get("FEA_UP")); // ;
				CFont::PrintString(MENU_X_LEFT_ALIGNED(242.0f), MENU_Y(384.0f - 3.5f), TheText.Get("FEA_DO")); // =
				CFont::PrintString(MENU_X_LEFT_ALIGNED(242.0f - 10.0f), MENU_Y(372.0f), TheText.Get("FEA_LE")); // <
				CFont::PrintString(MENU_X_LEFT_ALIGNED(242.0f + 11.0f), MENU_Y(372.0f), TheText.Get("FEA_RI")); // >
				CFont::PrintString(MENU_X_LEFT_ALIGNED(242.0f + 20.0f), MENU_Y(372.0f), TheText.Get("FEDSAS3")); // - CHANGE SELECTION
			}

			break;
		}
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
				CFont::SetColor(CRGBA(MENUOPTION_COLOR.r, MENUOPTION_COLOR.g, MENUOPTION_COLOR.b, 255));
			else {
				if(bottomBarActive || curBottomBarOption == i)
					CFont::SetColor(CRGBA(HEADER_COLOR.r, HEADER_COLOR.g, HEADER_COLOR.b, 255));
				else
					CFont::SetColor(CRGBA(HEADER_COLOR.r, HEADER_COLOR.g, HEADER_COLOR.b, 110));
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
#ifdef CUSTOM_FRONTEND_OPTIONS
				CCustomScreenLayout *custom = aScreens[m_nPrevScreen].layout;
				if (custom) {
					previousSprite = custom->sprite;
					break;
				}
				if (!custom)
#endif
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
#ifdef CUSTOM_FRONTEND_OPTIONS
		default:
			CCustomScreenLayout *custom = aScreens[m_nCurrScreen].layout;
			if (custom) {
				previousSprite = custom->sprite;
			}
			break;
#endif
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

#ifdef RED_DELETE_BACKGROUND
	if (m_nCurrScreen == MENUPAGE_CHOOSE_DELETE_SLOT || m_nCurrScreen == MENUPAGE_DELETE_SLOT_CONFIRM) {
		CSprite2d::Draw2DPolygon(SCREEN_STRETCH_X(18.0f), MENU_Y(8.0f),
				SCREEN_WIDTH - SCREEN_STRETCH_X(20.0f), MENU_Y(8.0f),
				SCREEN_STRETCH_X(12.0f), MENU_Y(11.0f),
				SCREEN_WIDTH - SCREEN_STRETCH_X(14.0f), MENU_Y(11.0f),
				CRGBA(150, 0, 0, 140));

		CSprite2d::Draw2DPolygon(SCREEN_STRETCH_X(12.0f), MENU_Y(11.0f),
				SCREEN_WIDTH - SCREEN_STRETCH_X(14.0f), MENU_Y(11.0f),
				SCREEN_STRETCH_X(10.0f), MENU_Y(16.0f),
				SCREEN_WIDTH - SCREEN_STRETCH_X(12.0f), MENU_Y(16.0f),
				CRGBA(150, 0, 0, 140));

		CSprite2d::Draw2DPolygon(SCREEN_STRETCH_X(10.0f), MENU_Y(16.0f),
				SCREEN_WIDTH - SCREEN_STRETCH_X(12.0f), MENU_Y(16.0f),
				SCREEN_STRETCH_X(10.0f), SCREEN_SCALE_Y(431.0f),
				SCREEN_WIDTH - SCREEN_STRETCH_X(12.0f), SCREEN_SCALE_Y(431.0f),
				CRGBA(150, 0, 0, 140));

		CSprite2d::Draw2DPolygon(SCREEN_STRETCH_X(10.0f), SCREEN_SCALE_Y(431.0f),
				SCREEN_WIDTH - SCREEN_STRETCH_X(12.0f), SCREEN_SCALE_Y(431.0f),
				SCREEN_STRETCH_X(12.0f), SCREEN_SCALE_Y(435.0f),
				SCREEN_WIDTH - SCREEN_STRETCH_X(14.0f), SCREEN_SCALE_Y(435.0f),
				CRGBA(150, 0, 0, 140));

		CSprite2d::Draw2DPolygon(SCREEN_STRETCH_X(12.0f), SCREEN_SCALE_Y(435.0f),
				SCREEN_WIDTH - SCREEN_STRETCH_X(14.0f), SCREEN_SCALE_Y(435.0f),
				SCREEN_STRETCH_X(18.0f), SCREEN_SCALE_Y(438.0f),
				SCREEN_WIDTH - SCREEN_STRETCH_X(20.0f), SCREEN_SCALE_Y(438.0f),
				CRGBA(150, 0, 0, 140));

		// yellow bar
		CSprite2d::DrawRect(CRect(MENU_X(13.0f), SCREEN_STRETCH_FROM_BOTTOM(96.0f),
			SCREEN_STRETCH_FROM_RIGHT(11.0f), SCREEN_STRETCH_FROM_BOTTOM(59.0f)),
			CRGBA(235, 170, 50, 255));
	}
#endif

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
	RESET_FONT_FOR_NEW_PAGE

	SET_FONT_FOR_MENU_HEADER

	CFont::PrintString(PAGE_NAME_X(MENUHEADER_POS_X), SCREEN_SCALE_FROM_BOTTOM(MENUHEADER_POS_Y), TheText.Get("FET_PS"));

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
			if (strcmp(FindFileData.cFileName, DEFAULT_SKIN_NAME) != 0) {
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
			int oldLength = (int)strlen(m_pSelectedSkin->skinNameDisplayed);
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
		MENU_X_RIGHT_ALIGNED(PLAYERSETUP_LIST_RIGHT), SCREEN_SCALE_FROM_BOTTOM(PLAYERSETUP_LIST_BOTTOM)),
		CRGBA(LIST_BACKGROUND_COLOR.r, LIST_BACKGROUND_COLOR.g, LIST_BACKGROUND_COLOR.b, FadeIn(LIST_BACKGROUND_COLOR.a)));

	// Header (Skin - Date)
	if (m_nCurrExLayer == HOVEROPTION_LIST) {
		CFont::SetColor(CRGBA(SELECTEDMENUOPTION_COLOR.r, SELECTEDMENUOPTION_COLOR.g, SELECTEDMENUOPTION_COLOR.b, FadeIn(255)));
	} else {
		CFont::SetColor(CRGBA(MENUOPTION_COLOR.r, MENUOPTION_COLOR.g, MENUOPTION_COLOR.b, FadeIn(255)));
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
	SET_FONT_FOR_LIST_ITEM
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
					m_nOptionMouseHovering = rowIdx;
					if (m_nMouseOldPosX != m_nMousePosX || m_nMouseOldPosY != m_nMousePosY) {
						m_nCurrExLayer = HOVEROPTION_LIST;
					}
					if (m_nHoverOption == HOVEROPTION_SKIN) {
						if (rowIdx == m_nSelectedListRow) {
							m_nHoverOption = HOVEROPTION_NOT_HOVERING;
							if (m_nSkinsTotal > 0) {
								DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_SETTING_CHANGE, 0);
								strcpy(m_PrefsSkinFile, m_aSkinName);
								CWorld::Players[0].SetPlayerSkin(m_PrefsSkinFile);
								SaveSettings();
							}
						} else {
							DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_NAVIGATION, 0);
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
				CFont::SetColor(CRGBA(LIST_OPTION_COLOR.r, LIST_OPTION_COLOR.g, LIST_OPTION_COLOR.b, FadeIn(LIST_OPTION_COLOR.a)));
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
		
		float scrollbarHeight = SCROLLBAR_MAX_HEIGHT / (m_nSkinsTotal / (float) MAX_VISIBLE_LIST_ROW);
		float scrollbarBottom, scrollbarTop;
		if (m_nSkinsTotal <= MAX_VISIBLE_LIST_ROW) {
			scrollbarBottom = SCREEN_SCALE_FROM_BOTTOM(PLAYERSETUP_LIST_BOTTOM + PLAYERSETUP_SCROLLBUTTON_HEIGHT + 4.0f);
			scrollbarTop = MENU_Y(PLAYERSETUP_LIST_BODY_TOP);

			// Scrollbar shadow
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
			// Scrollbar shadow
			CSprite2d::DrawRect(CRect(MENU_X_RIGHT_ALIGNED(PLAYERSETUP_LIST_RIGHT - 4), scrollbarTop,
				MENU_X_RIGHT_ALIGNED(PLAYERSETUP_LIST_RIGHT - 1 - PLAYERSETUP_SCROLLBAR_WIDTH), scrollbarBottom + MENU_Y(1.0f)),
				CRGBA(50, 50, 50, FadeIn(255)));

		}
		// Scrollbar
		CSprite2d::DrawRect(CRect(MENU_X_RIGHT_ALIGNED(PLAYERSETUP_LIST_RIGHT - 4), scrollbarTop,
			MENU_X_RIGHT_ALIGNED(PLAYERSETUP_LIST_RIGHT - PLAYERSETUP_SCROLLBAR_WIDTH), scrollbarBottom),
			CRGBA(SCROLLBAR_COLOR.r, SCROLLBAR_COLOR.g, SCROLLBAR_COLOR.b, FadeIn(255)));

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
			CFont::SetColor(CRGBA(SELECTEDMENUOPTION_COLOR.r, SELECTEDMENUOPTION_COLOR.g, SELECTEDMENUOPTION_COLOR.b, FadeIn(120)));
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
				DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_NAVIGATION, 0);

			m_nHoverOption = HOVEROPTION_BACK;

		} else if ((strcmp(m_aSkinName, m_PrefsSkinFile) != 0
			&& m_nMousePosX > MENU_X_LEFT_ALIGNED(PLAYERSETUP_LIST_LEFT)
			&& m_nMousePosX < MENU_X_LEFT_ALIGNED(PLAYERSETUP_LIST_LEFT) + CFont::GetStringWidth(TheText.Get("FES_SET"), true)
			&& m_nMousePosY > SCREEN_SCALE_FROM_BOTTOM(PLAYERSETUP_LIST_BOTTOM - 3)
			&& m_nMousePosY < SCREEN_SCALE_FROM_BOTTOM(PLAYERSETUP_LIST_BOTTOM - 26))
			|| m_nCurrExLayer == HOVEROPTION_USESKIN) {
			if (m_nHoverOption != HOVEROPTION_USESKIN)
				DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_NAVIGATION, 0);

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
			CFont::SetColor(CRGBA(SELECTEDMENUOPTION_COLOR.r, SELECTEDMENUOPTION_COLOR.g, SELECTEDMENUOPTION_COLOR.b, FadeIn(255)));
		} else {
			CFont::SetColor(CRGBA(MENUOPTION_COLOR.r, MENUOPTION_COLOR.g, MENUOPTION_COLOR.b, FadeIn(255)));
		}
	}
	CFont::SetRightJustifyOff();
	CFont::SetColor(CRGBA(0, 0, 0, FadeIn(90)));

	// Use skin button
	for (int i = 0; i < 2; i++) {
		CFont::PrintString(MENU_X_LEFT_ALIGNED(i + PLAYERSETUP_LIST_LEFT), SCREEN_SCALE_FROM_BOTTOM(PLAYERSETUP_LIST_BOTTOM - 5 - i), TheText.Get("FES_SET"));
		if (!strcmp(m_aSkinName, m_PrefsSkinFile)) {
			CFont::SetColor(CRGBA(DARKMENUOPTION_COLOR.r, DARKMENUOPTION_COLOR.g, DARKMENUOPTION_COLOR.b, FadeIn(255)));
		} else if (m_nHoverOption == HOVEROPTION_USESKIN) {
			CFont::SetColor(CRGBA(SELECTEDMENUOPTION_COLOR.r, SELECTEDMENUOPTION_COLOR.g, SELECTEDMENUOPTION_COLOR.b, FadeIn(255)));
		} else {
			CFont::SetColor(CRGBA(MENUOPTION_COLOR.r, MENUOPTION_COLOR.g, MENUOPTION_COLOR.b, FadeIn(255)));
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

	CentreMousePointer();
	DMAudio.ChangeMusicMode(MUSICMODE_FRONTEND);
	DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_STARTING, 0);
	m_nCurrOption = 0;

#ifdef FIX_BUGS
	static bool firstTime = true;
	if (firstTime) {
		DMAudio.SetRadioInCar(m_PrefsRadioStation);
		firstTime = false;
	} else
#endif
	m_PrefsRadioStation = DMAudio.GetRadioInCar();

	if (DMAudio.IsMP3RadioChannelAvailable()) {
		if (m_PrefsRadioStation > USERTRACK)
			m_PrefsRadioStation = CGeneral::GetRandomNumber() % (USERTRACK + 1);
	} else if (m_PrefsRadioStation > CHATTERBOX)
		m_PrefsRadioStation = CGeneral::GetRandomNumber() % (CHATTERBOX + 1);
	
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
#if GTA_VERSION < GTA3_PC_11
	CStreaming::IHaveUsedStreamingMemory();
	CTimer::Update();
#endif

	for (int i = 0; i < ARRAY_SIZE(FrontendFilenames); i++) {
		m_aFrontEndSprites[i].SetTexture(FrontendFilenames[i][0], FrontendFilenames[i][1]);
		m_aFrontEndSprites[i].SetAddressing(rwTEXTUREADDRESSBORDER);
	}

#ifdef GAMEPAD_MENU
	LoadController(m_PrefsControllerType);
#endif

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
	static bool menuOptionAdded = false;
	for (int i = 0; i < ARRAY_SIZE(MapFilenames); i++) {
		if (!menuOptionAdded && RwTextureRead(MapFilenames[i][0], MapFilenames[i][1])) {
			FrontendOptionSetCursor(MENUPAGE_PAUSE_MENU, 2, false);
			FrontendOptionAddBuiltinAction("FEG_MAP", MENUACTION_CHANGEMENU, MENUPAGE_MAP, SAVESLOT_NONE);
			menuOptionAdded = true;
		}
		m_aMapSprites[i].SetTexture(MapFilenames[i][0], MapFilenames[i][1]);
		m_aMapSprites[i].SetAddressing(rwTEXTUREADDRESSBORDER);
	}
	fMapSize = SCREEN_HEIGHT * 2.0f;
	fMapCenterX = 0.0f;
	fMapCenterY = 0.0f;
#endif
#if GTA_VERSION >= GTA3_PC_11
	CStreaming::IHaveUsedStreamingMemory();
	CTimer::Update();
#endif
	m_bSpritesLoaded = true;
	CTxdStore::PopCurrentTxd();
}

#ifdef GAMEPAD_MENU
const char* controllerTypesPaths[] = {
	nil,
	"MODELS/FRONTEND_DS3.TXD",
	"MODELS/FRONTEND_DS4.TXD",
	"MODELS/FRONTEND_X360.TXD",
	"MODELS/FRONTEND_XONE.TXD",
};

void
CMenuManager::LoadController(int8 type)
{
	switch (type)
	{
	case CONTROLLER_DUALSHOCK2:
	case CONTROLLER_DUALSHOCK3:
	case CONTROLLER_DUALSHOCK4:
		CFont::LoadButtons("MODELS/PS3BTNS.TXD");
		break;
	default:
		CFont::LoadButtons("MODELS/X360BTNS.TXD");
		break;
	}

	// Unload current textures
	for (int i = FE_CONTROLLER; i <= FE_ARROWS4; i++)
		m_aFrontEndSprites[i].Delete();

	// Unload txd
	int frontend_controller = CTxdStore::FindTxdSlot("frontend_controller");
	if (frontend_controller != -1)
		CTxdStore::RemoveTxd(frontend_controller);

	// Find the new txd to load
	bool bTxdMissing = true;
	if (controllerTypesPaths[type])
		if (int file = CFileMgr::OpenFile(controllerTypesPaths[type])) {
			CFileMgr::CloseFile(file);
			bTxdMissing = false;
		}

	int txdSlot = -1;

	if (bTxdMissing)
		// Not found, fall back to original textures
		txdSlot = CTxdStore::FindTxdSlot("frontend");
	else {
		// Found, load txd
		txdSlot = frontend_controller;
		if (txdSlot == -1)
			txdSlot = CTxdStore::AddTxdSlot("frontend_controller");
		CTxdStore::LoadTxd(txdSlot, controllerTypesPaths[type]);
		CTxdStore::AddRef(txdSlot);
	}

	assert(txdSlot != -1);
	// Load new textures
	CTxdStore::SetCurrentTxd(txdSlot);
	for (int i = FE_CONTROLLER; i <= FE_ARROWS4; i++) {
		m_aFrontEndSprites[i].SetTexture(FrontendFilenames[i][0], FrontendFilenames[i][1]);
		m_aFrontEndSprites[i].SetAddressing(rwTEXTUREADDRESSBORDER);
	}
}
#endif // GAMEPAD_MENU

void
CMenuManager::LoadSettings()
{
	CFileMgr::SetDirMyDocuments();
	int fileHandle = CFileMgr::OpenFile("gta3.set", "r");

	int32 prevLang = m_PrefsLanguage;
#if GTA_VERSION >= GTA3_PC_11
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
		}
	}

	CFileMgr::CloseFile(fileHandle);
	CFileMgr::SetDir("");

#ifdef LOAD_INI_SETTINGS
	if (LoadINISettings()) {
		LoadINIControllerSettings();
	} else {
		// no re3.ini, create it
		SaveINISettings();
		SaveINIControllerSettings();
	}
#endif

	m_PrefsVsync = m_PrefsVsyncDisp;
	CRenderer::ms_lodDistScale = m_PrefsLOD;

	if (m_nPrefsAudio3DProviderIndex == -1)
		m_nPrefsAudio3DProviderIndex = -2;

	if (m_PrefsLanguage == prevLang)
		m_bLanguageLoaded = false;
	else {
		m_bLanguageLoaded = true;
		// Already called in InitialiseChangedLanguageSettings
		/*
		TheText.Unload();
		TheText.Load();
		*/
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
#ifndef LOAD_INI_SETTINGS
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
	}

	CFileMgr::CloseFile(fileHandle);
	CFileMgr::SetDir("");

#else
	SaveINISettings();
#endif
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
	CFont::SetWrapx(SCREEN_WIDTH - StretchX(170.0f)); // unused
	CFont::SetRightJustifyWrap(SCREEN_WIDTH - StretchX(170.0f)); // unused
	CSprite2d::DrawRect(CRect(StretchX(120.0f), StretchY(150.0f), SCREEN_WIDTH - StretchX(120.0f), SCREEN_HEIGHT - StretchY(220.0f)), CRGBA(50, 50, 50, 210));
	CFont::SetFontStyle(FONT_LOCALE(FONT_BANK));
	CFont::SetCentreSize(SCREEN_SCALE_X(380.0f));
	CFont::SetCentreOn();
	CFont::SetColor(CRGBA(SELECTEDMENUOPTION_COLOR.r, SELECTEDMENUOPTION_COLOR.g, SELECTEDMENUOPTION_COLOR.b, 255));
	CFont::SetScale(SCREEN_SCALE_X(SMALLTEXT_X_SCALE), SCREEN_SCALE_Y(SMALLTEXT_Y_SCALE));
	CFont::PrintString(StretchX(320.0f), StretchY(170.0f), TheText.Get(text));
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
	CFont::SetColor(CRGBA(LABEL_COLOR.r, LABEL_COLOR.g, LABEL_COLOR.b, FadeIn(255)));
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
			if (newColor != TEXT_COLOR) {
				newColor.r /= 2;
				newColor.g /= 2;
				newColor.b /= 2;
			}

#ifdef PS2_LIKE_MENU
			CFont::SetDropColor(CRGBA(0, 0, 0, FadeIn(255)));
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

// Not sure about name. Not to be confused with CPad::PrintErrorMessage
void
CMenuManager::PrintErrorMessage()
{
	if (!CPad::bDisplayNoControllerMessage && !CPad::bObsoleteControllerMessage)
		return;

	CSprite2d::DrawRect(CRect(SCREEN_SCALE_X(20.0f), SCREEN_SCALE_Y(140.0f), SCREEN_SCALE_FROM_RIGHT(20.0f), SCREEN_SCALE_FROM_BOTTOM(120.0f)), CRGBA(64, 16, 16, 224));
	CFont::SetFontStyle(FONT_LOCALE(FONT_BANK));
	CFont::SetBackgroundOff();
	CFont::SetPropOn();
	CFont::SetCentreOff();
	CFont::SetJustifyOn();
	CFont::SetRightJustifyOff();
	CFont::SetBackGroundOnlyTextOn();
	CFont::SetWrapx(SCREEN_SCALE_FROM_RIGHT(MENU_X_MARGIN));
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
#if GTA_VERSION >= GTA3_PC_11
	CFont::SetFontStyle(FONT_LOCALE(FONT_BANK));
#endif
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

			CFont::SetColor(CRGBA(LABEL_COLOR.r, LABEL_COLOR.g, LABEL_COLOR.b, FadeIn(255.0f * alphaMult)));
			CFont::SetRightJustifyOff();
			CFont::PrintString(MENU_X_LEFT_ALIGNED(STATS_ROW_X_MARGIN), y - MENU_Y(STATS_BOTTOM_MARGIN - STATS_TOP_MARGIN), gUString);
			CFont::SetRightJustifyOn();
			CFont::PrintString(MENU_X_RIGHT_ALIGNED(STATS_ROW_X_MARGIN), y - MENU_Y(STATS_BOTTOM_MARGIN - STATS_TOP_MARGIN), gUString2);
		}
	}
	// Game doesn't do that, but it's better
	float nextX = MENU_X_LEFT_ALIGNED(STATS_RATING_X);

	CFont::SetColor(CRGBA(LABEL_COLOR.r, LABEL_COLOR.g, LABEL_COLOR.b, FadeIn(255)));
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
	SET_FONT_FOR_MENU_HEADER
	CFont::PrintString(PAGE_NAME_X(MENUHEADER_POS_X), SCREEN_SCALE_FROM_BOTTOM(MENUHEADER_POS_Y), TheText.Get(aScreens[m_nCurrScreen].m_ScreenName));
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
#ifdef MISSION_REPLAY
			if (doingMissionRetry) {
				RetryMission(2, 0);
				m_nCurrSaveSlot = SLOT_COUNT;
				doingMissionRetry = false;
			}
#endif
			if (CheckSlotDataValid(m_nCurrSaveSlot)) {
#ifdef USE_DEBUG_SCRIPT_LOADER
				scriptToLoad = 0;
#endif
#ifdef PC_PLAYER_CONTROLS
				TheCamera.m_bUseMouse3rdPerson = m_ControlMethod == CONTROL_STANDARD;
#endif
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
			} else
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
					MouseButtonJustClicked = rsMOUSELEFTBUTTON;
				else if (CPad::GetPad(0)->GetRightMouseJustUp())
					MouseButtonJustClicked = rsMOUSERIGHTBUTTON;
				else if (CPad::GetPad(0)->GetMiddleMouseJustUp())
					MouseButtonJustClicked = rsMOUSMIDDLEBUTTON;
				else if (CPad::GetPad(0)->GetMouseWheelUpJustUp())
					MouseButtonJustClicked = rsMOUSEWHEELUPBUTTON;
				else if (CPad::GetPad(0)->GetMouseWheelDownJustUp())
					MouseButtonJustClicked = rsMOUSEWHEELDOWNBUTTON;
				else if (CPad::GetPad(0)->GetMouseX1JustUp())
					MouseButtonJustClicked = rsMOUSEX1BUTTON;
				else if (CPad::GetPad(0)->GetMouseX2JustUp())
					MouseButtonJustClicked = rsMOUSEX2BUTTON;

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
					DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_SETTING_CHANGE, 0);
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

#ifdef USE_DEBUG_SCRIPT_LOADER
	if (m_nCurrScreen == MENUPAGE_START_MENU || m_nCurrScreen == MENUPAGE_NEW_GAME || m_nCurrScreen == MENUPAGE_NEW_GAME_RELOAD) {
#ifdef RW_GL3
		if (glfwGetKey(PSGLOBAL(window), GLFW_KEY_R) == GLFW_PRESS) {
			scriptToLoad = 1;
			DoSettingsBeforeStartingAGame();
			return;
		}
		if (glfwGetKey(PSGLOBAL(window), GLFW_KEY_D) == GLFW_PRESS) {
			scriptToLoad = 2;
			DoSettingsBeforeStartingAGame();
			return;
		}
#elif defined _WIN32
		if (GetAsyncKeyState('R') & 0x8000) {
			scriptToLoad = 1;
			DoSettingsBeforeStartingAGame();
			return;
		}
		if (GetAsyncKeyState('D') & 0x8000) {
			scriptToLoad = 2;
			DoSettingsBeforeStartingAGame();
			return;
		}
#endif
	}
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

	if (hasNativeList(m_nCurrScreen)) {
		// Not split to seperate function in III as in VC, but we need it for scrollable pages :)
		ProcessList(goBack, optionSelected);

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
			DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_NAVIGATION, 0);
			goDown = true;
		} else if (CPad::GetPad(0)->GetUpJustDown() || CPad::GetPad(0)->GetAnaloguePadUp() || CPad::GetPad(0)->GetDPadUpJustDown()) {
			m_bShowMouse = false;
			DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_NAVIGATION, 0);
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
				m_nCurrOption = m_nOptionMouseHovering;
				optionSelected = true;
			}
		} else if (CPad::GetPad(0)->GetLeftMouseJustDown()) {
#ifdef TIDY_UP_PBP
			if (m_nHoverOption >= HOVEROPTION_RADIO_0 && m_nHoverOption <= HOVEROPTION_RADIO_9) {
				DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_SETTING_CHANGE, 0);
				m_PrefsRadioStation = m_nHoverOption - HOVEROPTION_RADIO_0;
				SaveSettings();
				DMAudio.SetRadioInCar(m_PrefsRadioStation);
				DMAudio.PlayFrontEndTrack(m_PrefsRadioStation, 1);
				OutputDebugString("FRONTEND RADIO STATION CHANGED");
			} else if (m_nHoverOption == HOVEROPTION_RANDOM_ITEM
				&& aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption].m_Action != MENUACTION_RESUME) {
				m_nCurrOption = m_nOptionMouseHovering;
				optionSelected = true;
			}
#else
			switch (m_nHoverOption) {
			case HOVEROPTION_RADIO_0:
				DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_SETTING_CHANGE, 0);
				m_PrefsRadioStation = HEAD_RADIO;
				SaveSettings();
				DMAudio.SetRadioInCar(m_PrefsRadioStation);
				DMAudio.PlayFrontEndTrack(m_PrefsRadioStation, 1);
				OutputDebugString("FRONTEND RADIO STATION CHANGED");
				break;
			case HOVEROPTION_RADIO_1:
				DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_SETTING_CHANGE, 0);
				m_PrefsRadioStation = DOUBLE_CLEF;
				SaveSettings();
				DMAudio.SetRadioInCar(m_PrefsRadioStation);
				DMAudio.PlayFrontEndTrack(m_PrefsRadioStation, 1);
				OutputDebugString("FRONTEND RADIO STATION CHANGED");
				break;
			case HOVEROPTION_RADIO_2:
				DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_SETTING_CHANGE, 0);
				m_PrefsRadioStation = JAH_RADIO;
				SaveSettings();
				DMAudio.SetRadioInCar(m_PrefsRadioStation);
				DMAudio.PlayFrontEndTrack(m_PrefsRadioStation, 1);
				OutputDebugString("FRONTEND RADIO STATION CHANGED");
				break;
			case HOVEROPTION_RADIO_3:
				DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_SETTING_CHANGE, 0);
				m_PrefsRadioStation = RISE_FM;
				SaveSettings();
				DMAudio.SetRadioInCar(m_PrefsRadioStation);
				DMAudio.PlayFrontEndTrack(m_PrefsRadioStation, 1);
				OutputDebugString("FRONTEND RADIO STATION CHANGED");
				break;
			case HOVEROPTION_RADIO_4:
				DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_SETTING_CHANGE, 0);
				m_PrefsRadioStation = LIPS_106;
				SaveSettings();
				DMAudio.SetRadioInCar(m_PrefsRadioStation);
				DMAudio.PlayFrontEndTrack(m_PrefsRadioStation, 1);
				OutputDebugString("FRONTEND RADIO STATION CHANGED");
				break;
			case HOVEROPTION_RADIO_5:
				DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_SETTING_CHANGE, 0);
				m_PrefsRadioStation = GAME_FM;
				SaveSettings();
				DMAudio.SetRadioInCar(m_PrefsRadioStation);
				DMAudio.PlayFrontEndTrack(m_PrefsRadioStation, 1);
				OutputDebugString("FRONTEND RADIO STATION CHANGED");
				break;
			case HOVEROPTION_RADIO_6:
				DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_SETTING_CHANGE, 0);
				m_PrefsRadioStation = MSX_FM;
				SaveSettings();
				DMAudio.SetRadioInCar(m_PrefsRadioStation);
				DMAudio.PlayFrontEndTrack(m_PrefsRadioStation, 1);
				OutputDebugString("FRONTEND RADIO STATION CHANGED");
				break;
			case HOVEROPTION_RADIO_7:
				DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_SETTING_CHANGE, 0);
				m_PrefsRadioStation = FLASHBACK;
				SaveSettings();
				DMAudio.SetRadioInCar(m_PrefsRadioStation);
				DMAudio.PlayFrontEndTrack(m_PrefsRadioStation, 1);
				OutputDebugString("FRONTEND RADIO STATION CHANGED");
				break;
			case HOVEROPTION_RADIO_8:
				DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_SETTING_CHANGE, 0);
				m_PrefsRadioStation = CHATTERBOX;
				SaveSettings();
				DMAudio.SetRadioInCar(m_PrefsRadioStation);
				DMAudio.PlayFrontEndTrack(m_PrefsRadioStation, 1);
				OutputDebugString("FRONTEND RADIO STATION CHANGED");
				break;
			case HOVEROPTION_RADIO_9:
				DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_SETTING_CHANGE, 0);
				m_PrefsRadioStation = USERTRACK;
				SaveSettings();
				DMAudio.SetRadioInCar(m_PrefsRadioStation);
				DMAudio.PlayFrontEndTrack(m_PrefsRadioStation, 1);
				OutputDebugString("FRONTEND RADIO STATION CHANGED");
				break;
			case HOVEROPTION_RANDOM_ITEM:
				if (((m_nCurrOption != 0) || (m_nCurrScreen != MENUPAGE_PAUSE_MENU)) {
					m_nCurrOption = m_nOptionMouseHovering;
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
#ifdef FIX_BUGS
				TheCamera.m_fMouseAccelVertical = TheCamera.m_fMouseAccelHorzntl + 0.0005f;
#else
				TheCamera.m_fMouseAccelVertical = TheCamera.m_fMouseAccelHorzntl;
#endif
				SaveSettings();
				break;
			case HOVEROPTION_DECREASE_MOUSESENS:
				TheCamera.m_fMouseAccelHorzntl -= (1.0f / 3000);
				TheCamera.m_fMouseAccelHorzntl = clamp(TheCamera.m_fMouseAccelHorzntl, 1.0f / 3200, 1.0f / 200);
#ifdef FIX_BUGS
				TheCamera.m_fMouseAccelVertical = TheCamera.m_fMouseAccelHorzntl + 0.0005f;
#else
				TheCamera.m_fMouseAccelVertical = TheCamera.m_fMouseAccelHorzntl;
#endif
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
		
#ifdef SCROLLABLE_PAGES
		if (m_nTotalListRow > MAX_VISIBLE_OPTION) {
			bool temp = false;
			
			m_nSelectedListRow = m_nCurrOption;
			
			// ignore detected back/select states, it's our screen's job
			ProcessList(temp, temp);
			
			// and ignore our screen's goUp/Down, now it's ProcessList's job
			goUp = false;
			goDown = false;
			m_nCurrOption = m_nSelectedListRow;
		}
		
		// Prevent sound on scroll. Mouse wheel is now belongs to us!
		if (!(m_nTotalListRow > MAX_VISIBLE_OPTION && (CPad::GetPad(0)->GetMouseWheelUpJustDown() || CPad::GetPad(0)->GetMouseWheelDownJustDown())))
#endif

		if (CPad::GetPad(0)->GetLeftMouseJustUp() || CPad::GetPad(0)->GetLeftJustUp() || CPad::GetPad(0)->GetRightJustUp()
			|| CPad::GetPad(0)->GetDPadLeftJustUp() || CPad::GetPad(0)->GetDPadRightJustUp()
			|| CPad::GetPad(0)->GetAnaloguePadLeftJustUp() || CPad::GetPad(0)->GetAnaloguePadRightJustUp()
			|| CPad::GetPad(0)->GetMouseWheelUpJustDown() || CPad::GetPad(0)->GetMouseWheelDownJustDown()) {
			int option = aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption].m_Action;
			if (option == MENUACTION_BRIGHTNESS || option == MENUACTION_DRAWDIST)
				DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_SETTING_CHANGE, 0);
			else if (option == MENUACTION_SFXVOLUME)
				DMAudio.PlayFrontEndSound(SOUND_FRONTEND_AUDIO_TEST, 0);
			else if (option == MENUACTION_MOUSESENS)
				DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_SETTING_CHANGE, 0);

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
		DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_NAVIGATION, 0);
		bottomBarActive = false;
		curBottomBarOption = hoveredBottomBarOption;
		ChangeScreen(bbNames[curBottomBarOption].screenId, 0, true, false);
		if (bbNames[curBottomBarOption].screenId == MENUPAGE_SOUND_SETTINGS)
			DMAudio.PlayFrontEndTrack(m_PrefsRadioStation, 1);
		return;
	} else if (bottomBarActive) {
		if (CPad::GetPad(0)->GetEnterJustDown() || CPad::GetPad(0)->GetCrossJustDown()) {
			DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_NAVIGATION, 0);
			bottomBarActive = false;

			if (bbNames[curBottomBarOption].screenId == MENUPAGE_SOUND_SETTINGS)
				DMAudio.PlayFrontEndTrack(m_PrefsRadioStation, 1);

			return;
		} else if (CPad::GetPad(0)->GetLeftJustDown() || CPad::GetPad(0)->GetAnaloguePadLeft() || CPad::GetPad(0)->GetDPadLeftJustDown()
			|| CPad::GetPad(0)->GetUpJustDown() || CPad::GetPad(0)->GetAnaloguePadUp() || CPad::GetPad(0)->GetDPadUpJustDown()) {

			m_bShowMouse = false;
			DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_NAVIGATION, 0);
			curBottomBarOption = ((curBottomBarOption + bbTabCount) - 1) % bbTabCount;
			ChangeScreen(bbNames[curBottomBarOption].screenId, 0, true, true);
			return;
		} else if (CPad::GetPad(0)->GetRightJustDown() || CPad::GetPad(0)->GetAnaloguePadRight() || CPad::GetPad(0)->GetDPadRightJustDown()
			|| CPad::GetPad(0)->GetDownJustDown() || CPad::GetPad(0)->GetAnaloguePadDown() || CPad::GetPad(0)->GetDPadDownJustDown()) {

			m_bShowMouse = false;
			DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_NAVIGATION, 0);
			curBottomBarOption = ((curBottomBarOption + bbTabCount) + 1) % bbTabCount;
			ChangeScreen(bbNames[curBottomBarOption].screenId, 0, true, true);
			return;
		}
		optionSelected = false;
		goDown = false;
		goUp = false;
	}
#endif

	int prevOption = m_nCurrOption;
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

	// Hide back button
#ifdef PS2_LIKE_MENU
	if ((goUp || goDown) && m_nCurrScreen != MENUPAGE_MULTIPLAYER_FIND_GAME && strcmp(aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption].m_EntryName, "FEDS_TB") == 0)
		m_nCurrOption = goUp ? m_nCurrOption - 1 : (aScreens[m_nCurrScreen].m_aEntries[0].m_Action == MENUACTION_LABEL);
#endif

	if (optionSelected) {
		int option = aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption].m_Action;
		if ((option == MENUACTION_CHANGEMENU) || (option == MENUACTION_POPULATESLOTS_CHANGEMENU)) {
			if (strcmp(aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption].m_EntryName, "FEDS_TB") != 0 &&
				strcmp(aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption].m_EntryName, "FESZ_CA") != 0) {

				if (m_nCurrScreen == MENUPAGE_CHOOSE_DELETE_SLOT) {
					if (Slots[aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption].m_SaveSlot - 1] == SLOT_EMPTY)
						DMAudio.PlayFrontEndSound(SOUND_FRONTEND_FAIL, 0);
					else
						DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_SETTING_CHANGE, 0);
				} else
					DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_NEW_PAGE, 0);
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
					DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_SETTING_CHANGE, 0);
				}
			}
		} else if (option != MENUACTION_CHANGEMENU && option != MENUACTION_BRIGHTNESS && option != MENUACTION_DRAWDIST
			&& option != MENUACTION_MUSICVOLUME && option != MENUACTION_SFXVOLUME
			&& option != MENUACTION_CHECKSAVE && option != MENUACTION_UNK24
			&& option != MENUACTION_MOUSESENS && option != MENUACTION_SCREENRES) {

			DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_SETTING_CHANGE, 0);
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
#ifndef TIDY_UP_PBP
				ChangeScreen(!m_bGameNotLoaded ? aScreens[m_nCurrScreen].m_PreviousPage[1] : aScreens[m_nCurrScreen].m_PreviousPage[0],
					GetPreviousPageOption(), true, true);
#else
				goBack = true;
#endif
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
						if (strcmp(aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption].m_EntryName, "FEDS_TB") == 0) {
#ifndef TIDY_UP_PBP
							ResetHelperText();
							ChangeScreen(!m_bGameNotLoaded ? aScreens[m_nCurrScreen].m_PreviousPage[1] : aScreens[m_nCurrScreen].m_PreviousPage[0],
								GetPreviousPageOption(), true, true);
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
				case MENUACTION_KEYBOARDCTRLS:
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
					ChangeScreen(!m_bGameNotLoaded ? aScreens[m_nCurrScreen].m_PreviousPage[1] : aScreens[m_nCurrScreen].m_PreviousPage[0],
						GetPreviousPageOption(), true, true);
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
							DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_SETTING_CHANGE, 0);
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
					} else if (m_nCurrScreen == MENUPAGE_DISPLAY_SETTINGS) {
						m_PrefsFrameLimiter = true;
						m_PrefsBrightness = 256;
						m_PrefsVsyncDisp = true;
						m_PrefsLOD = 1.2f;
						m_PrefsVsync = true;
						CRenderer::ms_lodDistScale = 1.2f;
#ifdef ASPECT_RATIO_SCALE
						m_PrefsUseWideScreen = AR_AUTO;
#else
						m_PrefsUseWideScreen = false;
#endif
						m_PrefsShowSubtitles = true;
						m_nDisplayVideoMode = m_nPrefsVideoMode;
#if GTA_VERSION >= GTA3_PC_11
						if (_dwOperatingSystemVersion == OS_WIN98) {
							CMBlur::BlurOn = false;
							CMBlur::MotionBlurClose();
						} else {
							CMBlur::BlurOn = true;
							CMBlur::MotionBlurOpen(Scene.camera);
						}
#else
						CMBlur::BlurOn = true;
#endif
#ifdef CUSTOM_FRONTEND_OPTIONS
						extern void RestoreDefGraphics(int8);
						extern void RestoreDefDisplay(int8);

						RestoreDefGraphics(FEOPTION_ACTION_SELECT);
						RestoreDefDisplay(FEOPTION_ACTION_SELECT);
#endif
						SaveSettings();
					} else if ((m_nCurrScreen != MENUPAGE_SKIN_SELECT_OLD) && (m_nCurrScreen == MENUPAGE_CONTROLLER_PC)) {
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
#ifdef FIX_BUGS
						MousePointerStateHelper.bInvertVertically = true;
						TheCamera.m_fMouseAccelVertical = 0.003f;
#else
						MousePointerStateHelper.bInvertVertically = false;
#endif
						TheCamera.m_fMouseAccelHorzntl = 0.0025f;
						CVehicle::m_bDisableMouseSteering = true;
						TheCamera.m_bHeadBob = false;
						SaveSettings();
#ifdef LOAD_INI_SETTINGS
						SaveINIControllerSettings();
#endif
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
#ifdef MISSION_REPLAY
				case MENUACTION_REJECT_RETRY:
					doingMissionRetry = false;
					AllowMissionReplay = 0;
					RequestFrontEndShutDown();
					break;
				case MENUACTION_UNK114:
					doingMissionRetry = false;
					RequestFrontEndShutDown();
					RetryMission(2, 0);
					return;
#endif
#ifdef CUSTOM_FRONTEND_OPTIONS
				case MENUACTION_CFO_SELECT:
				case MENUACTION_CFO_DYNAMIC:
					CMenuScreenCustom::CMenuEntry &option = aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption];
					if (option.m_Action == MENUACTION_CFO_SELECT) {
						if (option.m_CFOSelect->disableIfGameLoaded && !m_bGameNotLoaded)
							break;

						if (!option.m_CFOSelect->onlyApplyOnEnter) {
							option.m_CFOSelect->displayedValue++;
							if (option.m_CFOSelect->displayedValue >= option.m_CFOSelect->numRightTexts || option.m_CFOSelect->displayedValue < 0)
								option.m_CFOSelect->displayedValue = 0;
						}
						int8 oldValue = *option.m_CFO->value;

						*option.m_CFO->value = option.m_CFOSelect->lastSavedValue = option.m_CFOSelect->displayedValue;

						// Now everything is saved in .ini, and LOAD_INI_SETTINGS is fundamental for CFO
						// if (option.m_CFOSelect->save)
							SaveSettings();

						if (option.m_CFOSelect->displayedValue != oldValue && option.m_CFOSelect->changeFunc)
							option.m_CFOSelect->changeFunc(oldValue, option.m_CFOSelect->displayedValue);

					} else if (option.m_Action == MENUACTION_CFO_DYNAMIC) {
						if (option.m_CFODynamic->buttonPressFunc)
							option.m_CFODynamic->buttonPressFunc(FEOPTION_ACTION_SELECT);
					}

					break;
#endif
			}
		}
		ProcessOnOffMenuOptions();
	}

	if (goBack) {
		ResetHelperText();
		DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_BACK, 0);
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
#ifdef PS2_SAVE_DIALOG
		else if (m_nCurrScreen == MENUPAGE_CHOOSE_SAVE_SLOT || m_nCurrScreen == MENUPAGE_SAVE) {
#else
		else if (m_nCurrScreen == MENUPAGE_CHOOSE_SAVE_SLOT) {
#endif
			RequestFrontEndShutDown();
		}
		// It's now in ThingsToDoBeforeGoingBack()
#ifndef TIDY_UP_PBP
		else if (m_nCurrScreen == MENUPAGE_SOUND_SETTINGS) {
			DMAudio.StopFrontEndTrack();
			OutputDebugString("FRONTEND AUDIO TRACK STOPPED");
		}
#endif

		int oldScreen = !m_bGameNotLoaded ? aScreens[m_nCurrScreen].m_PreviousPage[1] : aScreens[m_nCurrScreen].m_PreviousPage[0];
		int oldOption = GetPreviousPageOption();

		if (oldScreen != -1) {
			ThingsToDoBeforeGoingBack();

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
	} else if (
#ifdef SCROLLABLE_PAGES
		!SCREEN_HAS_AUTO_SCROLLBAR &&
#endif
		CPad::GetPad(0)->GetMouseWheelUpJustDown() && m_nCurrScreen != MENUPAGE_KEYBOARD_CONTROLS) {
		increase = true;
		CheckSliderMovement(1);
		m_bShowMouse = true;
	}

	if (CPad::GetPad(0)->GetLeftJustDown() || CPad::GetPad(0)->GetAnaloguePadLeft() || CPad::GetPad(0)->GetDPadLeftJustDown()) {
		m_bShowMouse = false;
		decrease = true;
	} else if (
#ifdef SCROLLABLE_PAGES
		!SCREEN_HAS_AUTO_SCROLLBAR &&
#endif
		CPad::GetPad(0)->GetMouseWheelDownJustDown() && m_nCurrScreen != MENUPAGE_KEYBOARD_CONTROLS) {
		decrease = true;
		CheckSliderMovement(-1);
		m_bShowMouse = true;
	}

	if (increase)
		changeValueBy++;
	else if (decrease)
		changeValueBy--;

	if (changeValueBy != 0) {
		switch (aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption].m_Action) {
#ifdef FIX_BUGS
			case MENUACTION_CTRLCONFIG:
				CPad::GetPad(0)->Mode += changeValueBy;
				if (CPad::GetPad(0)->Mode > 3)
					CPad::GetPad(0)->Mode = 0;
				else if (CPad::GetPad(0)->Mode < 0)
					CPad::GetPad(0)->Mode = 3;
				SaveSettings();
				DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_SETTING_CHANGE, 0);
				break;
#endif
			case MENUACTION_RADIO:
				m_PrefsRadioStation += changeValueBy;
				DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_SETTING_CHANGE, 0);
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
#ifdef ASPECT_RATIO_SCALE
			case MENUACTION_WIDESCREEN:
				if (changeValueBy > 0) {
					m_PrefsUseWideScreen++;
					if (m_PrefsUseWideScreen > AR_MAX-1)
						m_PrefsUseWideScreen = 0;
				} else {
					m_PrefsUseWideScreen--;
					if (m_PrefsUseWideScreen < 0)
						m_PrefsUseWideScreen = AR_MAX-1;
				}
				DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_SETTING_CHANGE, 0);
				SaveSettings();
				break;
#endif
			case MENUACTION_SCREENRES:
				if (m_bGameNotLoaded) {
					RwChar** videoMods = _psGetVideoModeList();
					DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_NAVIGATION, 0);
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
				DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_NAVIGATION, 0);
				break;
			case MENUACTION_SPEAKERCONF:
				if (m_nPrefsAudio3DProviderIndex != -1) {
					m_PrefsSpeakers -= changeValueBy;
					m_PrefsSpeakers = clamp(m_PrefsSpeakers, 0, 2);
					DMAudio.SetSpeakerConfig(m_PrefsSpeakers);
					SaveSettings();
					DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_SETTING_CHANGE, 0);
				}
				break;
			case MENUACTION_CTRLMETHOD:
				m_ControlMethod = !m_ControlMethod;
				CCamera::m_bUseMouse3rdPerson = !m_ControlMethod;
				DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_SETTING_CHANGE, 0);
				SaveSettings();
				break;
#ifdef CUSTOM_FRONTEND_OPTIONS
			case MENUACTION_CFO_SELECT:
			case MENUACTION_CFO_DYNAMIC:
				CMenuScreenCustom::CMenuEntry &option = aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption];
				if (option.m_Action == MENUACTION_CFO_SELECT) {
					if (option.m_CFOSelect->disableIfGameLoaded && !m_bGameNotLoaded)
						break;
					
					if (changeValueBy > 0) {
						option.m_CFOSelect->displayedValue++;
						if (option.m_CFOSelect->displayedValue >= option.m_CFOSelect->numRightTexts)
							option.m_CFOSelect->displayedValue = 0;
					} else {
						option.m_CFOSelect->displayedValue--;
						if (option.m_CFOSelect->displayedValue < 0)
							option.m_CFOSelect->displayedValue = option.m_CFOSelect->numRightTexts - 1;
					}
					if (!option.m_CFOSelect->onlyApplyOnEnter) {
						int8 oldValue = *option.m_CFO->value;

						*option.m_CFO->value = option.m_CFOSelect->lastSavedValue = option.m_CFOSelect->displayedValue;

						// Now everything is saved in .ini, and LOAD_INI_SETTINGS is fundamental for CFO
						// if (option.m_CFOSelect->save)
							SaveSettings();

						if (option.m_CFOSelect->displayedValue != oldValue && option.m_CFOSelect->changeFunc)
							option.m_CFOSelect->changeFunc(oldValue, option.m_CFOSelect->displayedValue);
					}
				} else if (option.m_Action == MENUACTION_CFO_DYNAMIC && option.m_CFODynamic->buttonPressFunc) {
					option.m_CFODynamic->buttonPressFunc(changeValueBy > 0 ? FEOPTION_ACTION_RIGHT : FEOPTION_ACTION_LEFT);
				}
				DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_SETTING_CHANGE, 0);

				break;
#endif
		}
		ProcessOnOffMenuOptions();
		if (m_nCurrScreen == MENUPAGE_KEYBOARD_CONTROLS) {
			if (changeValueBy < 1) {
				m_nSelectedContSetupColumn = CONTSETUP_PED_COLUMN;
			} else {
				m_nSelectedContSetupColumn = CONTSETUP_VEHICLE_COLUMN;
			}
			DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_NAVIGATION, 0);
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
		DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_SETTING_CHANGE, 0);
#ifdef FIX_BUGS
		SaveSettings();
#endif // !FIX_BUGS
		break;
#ifndef FIX_BUGS
	case MENUACTION_CTRLCONFIG:
		CPad::GetPad(0)->Mode++;
		if (CPad::GetPad(0)->Mode > 3)
			CPad::GetPad(0)->Mode = 0;
		DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_SETTING_CHANGE, 0);
		break;
#endif // !FIX_BUGS
	case MENUACTION_CTRLDISPLAY:
		m_DisplayControllerOnFoot = !m_DisplayControllerOnFoot;
		DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_SETTING_CHANGE, 0);
		break;
	case MENUACTION_FRAMESYNC:
		m_PrefsVsyncDisp = !m_PrefsVsyncDisp;
		DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_SETTING_CHANGE, 0);
		SaveSettings();
		break;
	case MENUACTION_FRAMELIMIT:
		m_PrefsFrameLimiter = !m_PrefsFrameLimiter;
		DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_SETTING_CHANGE, 0);
		SaveSettings();
		break;
	case MENUACTION_TRAILS:
		CMBlur::BlurOn = !CMBlur::BlurOn;
		DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_SETTING_CHANGE, 0);
		SaveSettings();
		if (CMBlur::BlurOn)
			CMBlur::MotionBlurOpen(Scene.camera);
		else
			CMBlur::MotionBlurClose();
		break;
	case MENUACTION_SUBTITLES:
		m_PrefsShowSubtitles = !m_PrefsShowSubtitles;
		DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_SETTING_CHANGE, 0);
		SaveSettings();
		break;
#ifndef ASPECT_RATIO_SCALE
	case MENUACTION_WIDESCREEN:
		m_PrefsUseWideScreen = !m_PrefsUseWideScreen;
		DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_SETTING_CHANGE, 0);
		SaveSettings();
		break;
#endif
	case MENUACTION_SETDBGFLAG:
		CTheScripts::InvertDebugFlag();
		DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_SETTING_CHANGE, 0);
		break;
	case MENUACTION_SWITCHBIGWHITEDEBUGLIGHT:
		gbBigWhiteDebugLightSwitchedOn = !gbBigWhiteDebugLightSwitchedOn;
		DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_SETTING_CHANGE, 0);
		break;
	case MENUACTION_PEDROADGROUPS:
		gbShowPedRoadGroups = !gbShowPedRoadGroups;
		DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_SETTING_CHANGE, 0);
		break;
	case MENUACTION_CARROADGROUPS:
		gbShowCarRoadGroups = !gbShowCarRoadGroups;
		DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_SETTING_CHANGE, 0);
		break;
	case MENUACTION_COLLISIONPOLYS:
		gbShowCollisionPolys = !gbShowCollisionPolys;
		DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_SETTING_CHANGE, 0);
		break;
	case MENUACTION_MP_PLAYERCOLOR:
		PickNewPlayerColour();
		DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_SETTING_CHANGE, 0);
		SaveSettings();
		break;
	case MENUACTION_SHOWHEADBOB:
		TheCamera.m_bHeadBob = !TheCamera.m_bHeadBob;
		DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_SETTING_CHANGE, 0);
		SaveSettings();
		break;
	case MENUACTION_INVVERT:
		MousePointerStateHelper.bInvertVertically = !MousePointerStateHelper.bInvertVertically;
		DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_SETTING_CHANGE, 0);
		SaveSettings();
		break;
	case MENUACTION_DYNAMICACOUSTIC:
		m_PrefsDMA = !m_PrefsDMA;
		DMAudio.SetDynamicAcousticModelingStatus(m_PrefsDMA);
		DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_SETTING_CHANGE, 0);
		SaveSettings();
		break;
	case MENUACTION_MOUSESTEER:
		CVehicle::m_bDisableMouseSteering = !CVehicle::m_bDisableMouseSteering;
		DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_SETTING_CHANGE, 0);
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
		default: break;
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
	// In case we're windowed, keep mouse centered while in game. Done in main.cpp in other conditions.
#if defined(RW_GL3) && defined(IMPROVED_VIDEOMODE)
	glfwSetInputMode(PSGLOBAL(window), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
#endif
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
			ThingsToDoBeforeGoingBack();
#endif
			ShutdownJustMenu();
			SaveSettings();
#ifdef LOAD_INI_SETTINGS
			SaveINIControllerSettings();
#endif
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
	if (m_bMenuActive != menuWasActive) {
		m_bMenuStateChanged = true;
		
		// In case we're windowed, keep mouse centered while in game. Done in main.cpp in other conditions.
#if defined(RW_GL3) && defined(IMPROVED_VIDEOMODE)
		glfwSetInputMode(PSGLOBAL(window), GLFW_CURSOR, m_bMenuActive && m_nPrefsWindowed ? GLFW_CURSOR_HIDDEN : GLFW_CURSOR_DISABLED);
#endif
	}

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

#ifdef GAMEPAD_MENU
	int frontend_controllerTxdSlot = CTxdStore::FindTxdSlot("frontend_controller");
	if (frontend_controllerTxdSlot != -1)
		CTxdStore::RemoveTxd(frontend_controllerTxdSlot);
#endif

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

#if (!(defined RANDOMSPLASH) && GTA_VERSION < GTA3_PC_11)
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

#ifdef GAMEPAD_MENU
void
CMenuManager::PrintController(void)
{
	const float scale = 0.9f;
	const float CONTROLLER_SIZE_X = 235.2f;
	const float CONTROLLER_SIZE_Y = 175.2f;
	const float CONTROLLER_POS_X = (DEFAULT_SCREEN_WIDTH - CONTROLLER_SIZE_X) / 2.0f;
	const float CONTROLLER_POS_Y = 160.0f;

	float centerX = CONTROLLER_POS_X + CONTROLLER_SIZE_X / 2;
	float centerY = CONTROLLER_POS_Y + CONTROLLER_SIZE_Y / 2;

#define X(f) ((f)*scale + centerX)
#define Y(f) ((f)*scale + centerY)

	m_aFrontEndSprites[FE_CONTROLLERSH].Draw(MENU_X_LEFT_ALIGNED(X(-CONTROLLER_SIZE_X / 2)), MENU_Y(Y(-CONTROLLER_SIZE_Y / 2)), MENU_X((CONTROLLER_SIZE_X + 4.8f) * scale), MENU_Y((CONTROLLER_SIZE_Y + 4.8f) * scale), CRGBA(0, 0, 0, 255));
	m_aFrontEndSprites[FE_CONTROLLER].Draw(MENU_X_LEFT_ALIGNED(X(-CONTROLLER_SIZE_X / 2)), MENU_Y(Y(-CONTROLLER_SIZE_Y / 2)), MENU_X(CONTROLLER_SIZE_X * scale), MENU_Y(CONTROLLER_SIZE_Y * scale), CRGBA(255, 255, 255, 255));
	if (m_DisplayControllerOnFoot) {
		if (CTimer::GetTimeInMillisecondsPauseMode() & 0x400)
			m_aFrontEndSprites[FE_ARROWS1].Draw(MENU_X_LEFT_ALIGNED(X(-CONTROLLER_SIZE_X / 2)), MENU_Y(Y(-CONTROLLER_SIZE_Y / 2)), MENU_X(CONTROLLER_SIZE_X * scale), MENU_Y(CONTROLLER_SIZE_Y * scale), CRGBA(255, 255, 255, 255));
		else
			m_aFrontEndSprites[FE_ARROWS3].Draw(MENU_X_LEFT_ALIGNED(X(-CONTROLLER_SIZE_X / 2)), MENU_Y(Y(-CONTROLLER_SIZE_Y / 2)), MENU_X(CONTROLLER_SIZE_X * scale), MENU_Y(CONTROLLER_SIZE_Y * scale), CRGBA(255, 255, 255, 255));
	} else {
		if (CTimer::GetTimeInMillisecondsPauseMode() & 0x400)
			m_aFrontEndSprites[FE_ARROWS2].Draw(MENU_X_LEFT_ALIGNED(X(-CONTROLLER_SIZE_X / 2)), MENU_Y(Y(-CONTROLLER_SIZE_Y / 2)), MENU_X(CONTROLLER_SIZE_X * scale), MENU_Y(CONTROLLER_SIZE_Y * scale), CRGBA(255, 255, 255, 255));
		else
			m_aFrontEndSprites[FE_ARROWS4].Draw(MENU_X_LEFT_ALIGNED(X(-CONTROLLER_SIZE_X / 2)), MENU_Y(Y(-CONTROLLER_SIZE_Y / 2)), MENU_X(CONTROLLER_SIZE_X * scale), MENU_Y(CONTROLLER_SIZE_Y * scale), CRGBA(255, 255, 255, 255));
	}

	CFont::SetFontStyle(FONT_LOCALE(FONT_BANK));
	CFont::SetScale(MENU_X(SMALLESTTEXT_X_SCALE * scale), MENU_Y(SMALLESTTEXT_Y_SCALE * scale)); // X

	// CFont::SetColor(CRGBA(128, 128, 128, FadeIn(255)));
	CFont::SetDropColor(CRGBA(0, 0, 0, FadeIn(255)));
	CFont::SetDropShadowPosition(1);
	CFont::SetColor(CRGBA(255, 255, 255, FadeIn(255)));
	CFont::SetWrapx(SCREEN_WIDTH);

	float TEXT_L2_X = 50.0f + CONTROLLER_POS_X - centerX, TEXT_L2_Y = -14.0f + CONTROLLER_POS_Y - centerY;
	float TEXT_L1_X = -4.0f + CONTROLLER_POS_X - centerX, TEXT_L1_Y = 25.0f + CONTROLLER_POS_Y - centerY, TEXT_L1_Y_VEH = 3.0f + TEXT_L1_Y;
	float TEXT_DPAD_X = -4.0f + CONTROLLER_POS_X - centerX, TEXT_DPAD_Y = 65.0f + CONTROLLER_POS_Y - centerY;
	float TEXT_LSTICK_X = -4.0f + CONTROLLER_POS_X - centerX, TEXT_LSTICK_Y = 97.0f + CONTROLLER_POS_Y - centerY;
	float TEXT_SELECT_X = 103.0f + CONTROLLER_POS_X - centerX, TEXT_SELECT_Y = 141.0f + CONTROLLER_POS_Y - centerY;
	float TEXT_START_X = 130.0f + CONTROLLER_POS_X - centerX, TEXT_START_Y = 128.0f + CONTROLLER_POS_Y - centerY;
	float TEXT_R2_X = 184.0F + CONTROLLER_POS_X - centerX, TEXT_R2_Y = -14.0f + CONTROLLER_POS_Y - centerY;
	float TEXT_R1_X = 238.0f + CONTROLLER_POS_X - centerX, TEXT_R1_Y = 25.0f + CONTROLLER_POS_Y - centerY;

	float TEXT_SQUARE_X = 144.0f + CONTROLLER_POS_X - centerX, TEXT_SQUARE_Y = 18.0f + CONTROLLER_POS_Y - centerY;
	float TEXT_TRIANGLE_X = 238.0f + CONTROLLER_POS_X - centerX, TEXT_TRIANGLE_Y = 52.0f + CONTROLLER_POS_Y - centerY;
	float TEXT_CIRCLE_X = 238.0f + CONTROLLER_POS_X - centerX, TEXT_CIRCLE_Y = 65.0f + CONTROLLER_POS_Y - centerY;
	float TEXT_CROSS_X = 238.0f + CONTROLLER_POS_X - centerX, TEXT_CROSS_Y = 78.0f + CONTROLLER_POS_Y - centerY;
	float TEXT_RSTICK_X = 238.0f + CONTROLLER_POS_X - centerX, TEXT_RSTICK_Y = 94.0f + CONTROLLER_POS_Y - centerY;
	float TEXT_R3_X = 238.0f + CONTROLLER_POS_X - centerX, TEXT_R3_Y = 109.0f + CONTROLLER_POS_Y - centerY;
	float TEXT_L3_X = 84.0f + CONTROLLER_POS_X - centerX, TEXT_L3_Y = 162.0f + CONTROLLER_POS_Y - centerY;
	float TEXT_L2R2_X = 74.0f + CONTROLLER_POS_X - centerX, TEXT_L2R2_Y = -6.0f + CONTROLLER_POS_Y - centerY;

	switch (m_PrefsControllerType)
	{
	case CONTROLLER_DUALSHOCK4:
		TEXT_L1_Y += 7.0f;
		TEXT_L1_Y_VEH = TEXT_L1_Y;
		TEXT_R1_Y += 7.0f;
		TEXT_TRIANGLE_Y -= 1.0f;
		TEXT_CIRCLE_Y -= 1.0f;
		TEXT_CROSS_Y -= 1.0f;
		TEXT_RSTICK_Y -= 4.0f;
		TEXT_R3_Y -= 4.0f;
		TEXT_DPAD_Y -= 1.0f;
		TEXT_LSTICK_Y -= 6.0f;
		TEXT_L3_X -= 2.0f;
		break;
	case CONTROLLER_XBOXONE:
		TEXT_L2_X -= 2.0f;
		TEXT_R2_X += 2.0f;
		TEXT_L1_Y += 15.0f;
		TEXT_L1_Y_VEH = TEXT_L1_Y;
		TEXT_R1_Y += 15.0f;
		TEXT_TRIANGLE_Y += 4.0f;
		TEXT_CIRCLE_Y += 4.0f;
		TEXT_CROSS_Y += 4.0f;
		TEXT_RSTICK_Y += 1.0f;
		TEXT_R3_Y += 1.0f;
		TEXT_DPAD_Y += 29.0f;
		TEXT_LSTICK_Y -= 22.0f;
		TEXT_L3_X -= 36.0f;
		TEXT_L2R2_Y += 5.0f;
		TEXT_SELECT_X += 4.0f;
		break;
	case CONTROLLER_XBOX360:
		TEXT_L2_X += 8.0f;
		TEXT_R2_X -= 8.0f;
		TEXT_L1_Y += 15.0f;
		TEXT_L1_Y_VEH = TEXT_L1_Y;
		TEXT_R1_Y += 15.0f;
		TEXT_TRIANGLE_Y += 4.0f;
		TEXT_CIRCLE_Y += 4.0f;
		TEXT_CROSS_Y += 4.0f;
		TEXT_RSTICK_Y += 4.0f;
		TEXT_R3_Y += 4.0f;
		TEXT_DPAD_Y += 30.0f;
		TEXT_LSTICK_Y -= 21.0f;
		TEXT_L3_X -= 36.0f;
		TEXT_L2R2_Y += 5.0f;
		TEXT_SELECT_X += 3.0f;
		break;
	};

	if (m_DisplayControllerOnFoot) {
		switch (CPad::GetPad(0)->Mode) {
			case 0:
				CFont::SetRightJustifyOn(); // X
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_L2_X)), MENU_Y(Y(TEXT_L2_Y)), TheText.Get("FEC_CWL"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_L1_X)), MENU_Y(Y(TEXT_L1_Y)), TheText.Get("FEC_LOF"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_DPAD_X)), MENU_Y(Y(TEXT_DPAD_Y)), TheText.Get("FEC_MOV"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_LSTICK_X)), MENU_Y(Y(TEXT_LSTICK_Y)), TheText.Get("FEC_MOV"));
				CFont::SetJustifyOn(); // X
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_SELECT_X)), MENU_Y(Y(TEXT_SELECT_Y)), TheText.Get("FEC_CAM"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_START_X)), MENU_Y(Y(TEXT_START_Y)), TheText.Get("FEC_PAU"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_R2_X)), MENU_Y(Y(TEXT_R2_Y)), TheText.Get("FEC_CWR"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_R1_X)), MENU_Y(Y(TEXT_R1_Y)), TheText.Get("FEC_TAR"));
				CFont::SetRightJustifyOn(); // X
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_SQUARE_X)), MENU_Y(Y(TEXT_SQUARE_Y)), TheText.Get("FEC_JUM"));
				CFont::SetJustifyOn(); // X
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_TRIANGLE_X)), MENU_Y(Y(TEXT_TRIANGLE_Y)), TheText.Get("FEC_ENV"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_CIRCLE_X)), MENU_Y(Y(TEXT_CIRCLE_Y)), TheText.Get("FEC_ATT"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_CROSS_X)), MENU_Y(Y(TEXT_CROSS_Y)), TheText.Get("FEC_RUN"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_RSTICK_X)), MENU_Y(Y(TEXT_RSTICK_Y)), TheText.Get("FEC_FPC"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_R3_X)), MENU_Y(Y(TEXT_R3_Y)), TheText.Get("FEC_LB3"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_R3_X)), MENU_Y(Y(TEXT_R3_Y + 13.0f)), TheText.Get("FEC_R3"));
				break;
			case 1:
				CFont::SetRightJustifyOn(); // X
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_L2_X)), MENU_Y(Y(TEXT_L2_Y)), TheText.Get("FEC_CWL"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_L1_X)), MENU_Y(Y(TEXT_L1_Y)), TheText.Get("FEC_LOF"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_DPAD_X)), MENU_Y(Y(TEXT_DPAD_Y)), TheText.Get("FEC_CAM"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_LSTICK_X)), MENU_Y(Y(TEXT_LSTICK_Y)), TheText.Get("FEC_MOV"));
				CFont::SetJustifyOn(); // X
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_SELECT_X)), MENU_Y(Y(TEXT_SELECT_Y)), TheText.Get("FEC_NA"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_START_X)), MENU_Y(Y(TEXT_START_Y)), TheText.Get("FEC_PAU"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_R2_X)), MENU_Y(Y(TEXT_R2_Y)), TheText.Get("FEC_CWR"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_R1_X)), MENU_Y(Y(TEXT_R1_Y)), TheText.Get("FEC_TAR"));
				CFont::SetRightJustifyOn(); // X
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_SQUARE_X)), MENU_Y(Y(TEXT_SQUARE_Y)), TheText.Get("FEC_JUM"));
				CFont::SetJustifyOn(); // X
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_TRIANGLE_X)), MENU_Y(Y(TEXT_TRIANGLE_Y)), TheText.Get("FEC_ENV"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_CIRCLE_X)), MENU_Y(Y(TEXT_CIRCLE_Y)), TheText.Get("FEC_ATT"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_CROSS_X)), MENU_Y(Y(TEXT_CROSS_Y)), TheText.Get("FEC_RUN"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_RSTICK_X)), MENU_Y(Y(TEXT_RSTICK_Y)), TheText.Get("FEC_FPC"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_R3_X)), MENU_Y(Y(TEXT_R3_Y)), TheText.Get("FEC_LB3"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_R3_X)), MENU_Y(Y(TEXT_R3_Y + 13.0f)), TheText.Get("FEC_R3"));
				break;
			case 2:
				CFont::SetRightJustifyOn(); // X
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_L2_X)), MENU_Y(Y(TEXT_L2_Y)), TheText.Get("FEC_CWL"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_L1_X)), MENU_Y(Y(TEXT_L1_Y)), TheText.Get("FEC_ENV"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_DPAD_X)), MENU_Y(Y(TEXT_DPAD_Y)), TheText.Get("FEC_MOV"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_LSTICK_X)), MENU_Y(Y(TEXT_LSTICK_Y)), TheText.Get("FEC_MOV"));
				CFont::SetJustifyOn(); // X
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_SELECT_X)), MENU_Y(Y(TEXT_SELECT_Y)), TheText.Get("FEC_CAM"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_START_X)), MENU_Y(Y(TEXT_START_Y)), TheText.Get("FEC_PAU"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_R2_X)), MENU_Y(Y(TEXT_R2_Y)), TheText.Get("FEC_CWR"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_R1_X)), MENU_Y(Y(TEXT_R1_Y)), TheText.Get("FEC_TAR"));
				CFont::SetRightJustifyOn(); // X
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_SQUARE_X)), MENU_Y(Y(TEXT_SQUARE_Y)), TheText.Get("FEC_JUM"));
				CFont::SetJustifyOn(); // X
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_TRIANGLE_X)), MENU_Y(Y(TEXT_TRIANGLE_Y)), TheText.Get("FEC_LOF"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_CIRCLE_X)), MENU_Y(Y(TEXT_CIRCLE_Y)), TheText.Get("FEC_RUN"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_CROSS_X)), MENU_Y(Y(TEXT_CROSS_Y)), TheText.Get("FEC_ATT"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_RSTICK_X)), MENU_Y(Y(TEXT_RSTICK_Y)), TheText.Get("FEC_FPC"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_R3_X)), MENU_Y(Y(TEXT_R3_Y)), TheText.Get("FEC_LB3"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_R3_X)), MENU_Y(Y(TEXT_R3_Y + 13.0f)), TheText.Get("FEC_R3"));
				break;
			case 3:
				CFont::SetRightJustifyOn(); // X
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_L2_X)), MENU_Y(Y(TEXT_L2_Y)), TheText.Get("FEC_CWL"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_L1_X)), MENU_Y(Y(TEXT_L1_Y)), TheText.Get("FEC_TAR"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_DPAD_X)), MENU_Y(Y(TEXT_DPAD_Y)), TheText.Get("FEC_NA"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_LSTICK_X)), MENU_Y(Y(TEXT_LSTICK_Y)), TheText.Get("FEC_MOV"));
				CFont::SetJustifyOn(); // X
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_SELECT_X)), MENU_Y(Y(TEXT_SELECT_Y)), TheText.Get("FEC_CAM"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_START_X)), MENU_Y(Y(TEXT_START_Y)), TheText.Get("FEC_PAU"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_R2_X)), MENU_Y(Y(TEXT_R2_Y)), TheText.Get("FEC_CWR"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_R1_X)), MENU_Y(Y(TEXT_R1_Y)), TheText.Get("FEC_ATT"));
				CFont::SetRightJustifyOn(); // X
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_SQUARE_X)), MENU_Y(Y(TEXT_SQUARE_Y)), TheText.Get("FEC_JUM"));
				CFont::SetJustifyOn(); // X
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_TRIANGLE_X)), MENU_Y(Y(TEXT_TRIANGLE_Y)), TheText.Get("FEC_ENV"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_CIRCLE_X)), MENU_Y(Y(TEXT_CIRCLE_Y)), TheText.Get("FEC_LOF"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_CROSS_X)), MENU_Y(Y(TEXT_CROSS_Y)), TheText.Get("FEC_RUN"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_RSTICK_X)), MENU_Y(Y(TEXT_RSTICK_Y)), TheText.Get("FEC_FPC"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_R3_X)), MENU_Y(Y(TEXT_R3_Y)), TheText.Get("FEC_LB3"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_R3_X)), MENU_Y(Y(TEXT_R3_Y + 13.0f)), TheText.Get("FEC_R3"));
				break;
			default:
				return;
		}
	} else {
		CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_L2R2_X)), MENU_Y(Y(TEXT_L2R2_Y)), TheText.Get("FEC_LB"));
		switch (CPad::GetPad(0)->Mode) {
			case 0:
				CFont::SetRightJustifyOn(); // X
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_L2_X)), MENU_Y(Y(TEXT_L2_Y)), TheText.Get("FEC_LL"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_L1_X)), MENU_Y(Y(TEXT_L1_Y_VEH)), TheText.Get("FEC_RSC"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_DPAD_X)), MENU_Y(Y(TEXT_DPAD_Y)), TheText.Get("FEC_VES"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_LSTICK_X)), MENU_Y(Y(TEXT_LSTICK_Y)), TheText.Get("FEC_VES"));
				CFont::SetJustifyOn(); // X
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_L3_X)), MENU_Y(Y(TEXT_L3_Y)), TheText.Get("FEC_HO3"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_SELECT_X)), MENU_Y(Y(TEXT_SELECT_Y)), TheText.Get("FEC_CAM"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_START_X)), MENU_Y(Y(TEXT_START_Y)), TheText.Get("FEC_PAU"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_R2_X)), MENU_Y(Y(TEXT_R2_Y)), TheText.Get("FEC_LR"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_R1_X)), MENU_Y(Y(TEXT_R1_Y)), TheText.Get("FEC_HAB"));
				CFont::SetRightJustifyOn(); // X
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_SQUARE_X)), MENU_Y(Y(TEXT_SQUARE_Y)), TheText.Get("FEC_BRA"));
				CFont::SetJustifyOn(); // X
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_TRIANGLE_X)), MENU_Y(Y(TEXT_TRIANGLE_Y)), TheText.Get("FEC_EXV"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_CIRCLE_X)), MENU_Y(Y(TEXT_CIRCLE_Y)), TheText.Get("FEC_CAW"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_CROSS_X)), MENU_Y(Y(TEXT_CROSS_Y)), TheText.Get("FEC_ACC"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_RSTICK_X)), MENU_Y(Y(TEXT_RSTICK_Y)), TheText.Get("FEC_TUC"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_R3_X)), MENU_Y(Y(TEXT_R3_Y)), TheText.Get("FEC_SM3"));
				break;
			case 1:
				CFont::SetRightJustifyOn(); // X
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_L2_X)), MENU_Y(Y(TEXT_L2_Y)), TheText.Get("FEC_LL"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_L1_X)), MENU_Y(Y(TEXT_L1_Y_VEH)), TheText.Get("FEC_HOR"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_DPAD_X)), MENU_Y(Y(TEXT_DPAD_Y)), TheText.Get("FEC_CAM"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_LSTICK_X)), MENU_Y(Y(TEXT_LSTICK_Y)), TheText.Get("FEC_VES"));
				CFont::SetJustifyOn(); // X
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_L3_X)), MENU_Y(Y(TEXT_L3_Y)), TheText.Get("FEC_NA"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_SELECT_X)), MENU_Y(Y(TEXT_SELECT_Y)), TheText.Get("FEC_RSC"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_START_X)), MENU_Y(Y(TEXT_START_Y)), TheText.Get("FEC_PAU"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_R2_X)), MENU_Y(Y(TEXT_R2_Y)), TheText.Get("FEC_LR"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_R1_X)), MENU_Y(Y(TEXT_R1_Y)), TheText.Get("FEC_HAB"));
				CFont::SetRightJustifyOn(); // X
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_SQUARE_X)), MENU_Y(Y(TEXT_SQUARE_Y)), TheText.Get("FEC_BRA"));
				CFont::SetJustifyOn(); // X
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_TRIANGLE_X)), MENU_Y(Y(TEXT_TRIANGLE_Y)), TheText.Get("FEC_EXV"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_CIRCLE_X)), MENU_Y(Y(TEXT_CIRCLE_Y)), TheText.Get("FEC_CAW"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_CROSS_X)), MENU_Y(Y(TEXT_CROSS_Y)), TheText.Get("FEC_ACC"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_RSTICK_X)), MENU_Y(Y(TEXT_RSTICK_Y)), TheText.Get("FEC_TUC"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_R3_X)), MENU_Y(Y(TEXT_R3_Y)), TheText.Get("FEC_SM3"));
				break;
			case 2:
				CFont::SetRightJustifyOn(); // X
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_L2_X)), MENU_Y(Y(TEXT_L2_Y)), TheText.Get("FEC_LL"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_L1_X)), MENU_Y(Y(TEXT_L1_Y_VEH)), TheText.Get("FEC_EXV"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_DPAD_X)), MENU_Y(Y(TEXT_DPAD_Y)), TheText.Get("FEC_VES"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_LSTICK_X)), MENU_Y(Y(TEXT_LSTICK_Y)), TheText.Get("FEC_VES"));
				CFont::SetJustifyOn(); // X
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_L3_X)), MENU_Y(Y(TEXT_L3_Y)), TheText.Get("FEC_RS3"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_SELECT_X)), MENU_Y(Y(TEXT_SELECT_Y)), TheText.Get("FEC_CAM"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_START_X)), MENU_Y(Y(TEXT_START_Y)), TheText.Get("FEC_PAU"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_R2_X)), MENU_Y(Y(TEXT_R2_Y)), TheText.Get("FEC_LR"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_R1_X)), MENU_Y(Y(TEXT_R1_Y)), TheText.Get("FEC_HOR"));
				CFont::SetRightJustifyOn(); // X
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_SQUARE_X)), MENU_Y(Y(TEXT_SQUARE_Y)), TheText.Get("FEC_BRA"));
				CFont::SetJustifyOn(); // X
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_TRIANGLE_X)), MENU_Y(Y(TEXT_TRIANGLE_Y)), TheText.Get("FEC_HAB"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_CIRCLE_X)), MENU_Y(Y(TEXT_CIRCLE_Y)), TheText.Get("FEC_CAW"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_CROSS_X)), MENU_Y(Y(TEXT_CROSS_Y)), TheText.Get("FEC_ACC"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_RSTICK_X)), MENU_Y(Y(TEXT_RSTICK_Y)), TheText.Get("FEC_TUC"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_R3_X)), MENU_Y(Y(TEXT_R3_Y)), TheText.Get("FEC_SM3"));
				break;
			case 3:
				CFont::SetRightJustifyOn(); // X
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_L2_X)), MENU_Y(Y(TEXT_L2_Y)), TheText.Get("FEC_LL"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_L1_X)), MENU_Y(Y(TEXT_L1_Y_VEH)), TheText.Get("FEC_HAB"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_DPAD_X)), MENU_Y(Y(TEXT_DPAD_Y)), TheText.Get("FEC_TUC"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_LSTICK_X)), MENU_Y(Y(TEXT_LSTICK_Y)), TheText.Get("FEC_VES"));
				CFont::SetJustifyOn(); // X
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_L3_X)), MENU_Y(Y(TEXT_L3_Y)), TheText.Get("FEC_HO3"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_SELECT_X)), MENU_Y(Y(TEXT_SELECT_Y)), TheText.Get("FEC_CAM"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_START_X)), MENU_Y(Y(TEXT_START_Y)), TheText.Get("FEC_PAU"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_R2_X)), MENU_Y(Y(TEXT_R2_Y)), TheText.Get("FEC_LR"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_R1_X)), MENU_Y(Y(TEXT_R1_Y)), TheText.Get("FEC_CAW"));
				CFont::SetRightJustifyOn(); // X
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_SQUARE_X)), MENU_Y(Y(TEXT_SQUARE_Y)), TheText.Get("FEC_SMT"));
				CFont::SetJustifyOn(); // X
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_TRIANGLE_X)), MENU_Y(Y(TEXT_TRIANGLE_Y)), TheText.Get("FEC_EXV"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_CIRCLE_X)), MENU_Y(Y(TEXT_CIRCLE_Y)), TheText.Get("FEC_RSC"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_CROSS_X)), MENU_Y(Y(TEXT_CROSS_Y)), TheText.Get("FEC_NA"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_RSTICK_X)), MENU_Y(Y(TEXT_RSTICK_Y)), TheText.Get("FEC_ACC"));
				CFont::PrintString(MENU_X_LEFT_ALIGNED(X(TEXT_R3_X)), MENU_Y(Y(TEXT_R3_Y)), TheText.Get("FEC_BRA"));
				break;
			default:
				return;
		}
	}

	CFont::SetDropShadowPosition(0); // X

#undef X
#undef Y
}
#else
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

	CFont::SetFontStyle(FONT_LOCALE(FONT_BANK));  // X

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
#endif

#ifdef MENU_MAP

#define ZOOM(x, y, in) \
	do { \
		if(fMapSize > SCREEN_HEIGHT * 3.0f && in) \
			break; \
		float z2 = in? 1.1f : 1.f/1.1f; \
		fMapCenterX += (x - fMapCenterX) * (1.0f - z2); \
		fMapCenterY += (y - fMapCenterY) * (1.0f - z2); \
		\
		if (fMapSize < SCREEN_HEIGHT / 2 && !in) \
			break; \
		\
		fMapSize *= z2; \
	} while(0) \

void
CMenuManager::PrintMap(void)
{
	CFont::SetJustifyOn();
	bMenuMapActive = true;
	CRadar::InitFrontEndMap();

	if (m_nMenuFadeAlpha < 255 && fMapCenterX == 0.f && fMapCenterY == 0.f) {
		// Just entered. We need to do these transformations in here, because Radar knows whether map is active or not
		CVector2D radarSpacePlayer;
		CVector2D screenSpacePlayer;
		CRadar::TransformRealWorldPointToRadarSpace(radarSpacePlayer, CVector2D(FindPlayerCoors()));
		CRadar::TransformRadarPointToScreenSpace(screenSpacePlayer, radarSpacePlayer);
		fMapCenterX = (-screenSpacePlayer.x) + SCREEN_WIDTH / 2;
		fMapCenterY = (-screenSpacePlayer.y) + SCREEN_HEIGHT / 2;
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
	static CVector2D mapCrosshair;

	if (m_nMenuFadeAlpha != 255 && !m_bShowMouse) {
		mapCrosshair.x = SCREEN_WIDTH / 2;
		mapCrosshair.y = SCREEN_HEIGHT / 2;
	} else if (m_bShowMouse) {
		mapCrosshair.x = m_nMousePosX;
		mapCrosshair.y = m_nMousePosY;
	}

	CSprite2d::DrawRect(CRect(mapCrosshair.x - MENU_X(1.0f), 0.0f,
		mapCrosshair.x + MENU_X(1.0f), SCREEN_HEIGHT),
		CRGBA(0, 0, 0, 150));
	CSprite2d::DrawRect(CRect(0.0f, mapCrosshair.y + MENU_X(1.0f),
		SCREEN_WIDTH, mapCrosshair.y - MENU_X(1.0f)),
		CRGBA(0, 0, 0, 150));

	// Adding marker
	if (m_nMenuFadeAlpha >= 255) {
		if (CPad::GetPad(0)->GetRightMouseJustDown() || CPad::GetPad(0)->GetCrossJustDown()) {
			if (mapCrosshair.y > fMapCenterY - fMapSize && mapCrosshair.y < fMapCenterY + fMapSize &&
				mapCrosshair.x > fMapCenterX - fMapSize && mapCrosshair.x < fMapCenterX + fMapSize) {

				float diffX = fMapCenterX - fMapSize, diffY = fMapCenterY - fMapSize;
				float x = ((mapCrosshair.x - diffX) / (fMapSize * 2)) * 4000.0f - 2000.0f;
				float y = 2000.0f - ((mapCrosshair.y - diffY) / (fMapSize * 2)) * 4000.0f;
				CRadar::ToggleTargetMarker(x, y);
				DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_SETTING_CHANGE, 0);
			}
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
			ZOOM(mapCrosshair.x, mapCrosshair.y, false);
		else
			ZOOM(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, false);
	} else if (CPad::GetPad(0)->GetMouseWheelUp() || CPad::GetPad(0)->GetPageUp() || CPad::GetPad(0)->GetRightShoulder1()) {
		if (CPad::GetPad(0)->GetMouseWheelUp())
			ZOOM(mapCrosshair.x, mapCrosshair.y, true);
		else
			ZOOM(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, true);
	}
	
	if (fMapCenterX - fMapSize > SCREEN_WIDTH / 2)
		fMapCenterX = fMapSize + SCREEN_WIDTH / 2;

	if (fMapCenterX + fMapSize < SCREEN_WIDTH / 2)
		fMapCenterX = SCREEN_WIDTH / 2 - fMapSize;

	if (fMapCenterY + fMapSize < SCREEN_HEIGHT - MENU_Y(60.0f))
		fMapCenterY = SCREEN_HEIGHT - MENU_Y(60.0f) - fMapSize;
	
	fMapCenterY = Min(fMapCenterY, fMapSize); // To not show beyond north border

	bMenuMapActive = false;

	CSprite2d::DrawRect(CRect(MENU_X(14.0f), SCREEN_STRETCH_FROM_BOTTOM(95.0f),
		SCREEN_STRETCH_FROM_RIGHT(11.0f), SCREEN_STRETCH_FROM_BOTTOM(59.0f)),
		CRGBA(235, 170, 50, 255));

	CFont::SetScale(MENU_X(0.4f), MENU_Y(0.7f));
	CFont::SetFontStyle(FONT_LOCALE(FONT_BANK));
	CFont::SetColor(CRGBA(HEADER_COLOR.r, HEADER_COLOR.g, HEADER_COLOR.b, FadeIn(255)));

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
	CFont::PrintString(nextX, SCREEN_SCALE_FROM_BOTTOM(nextY), TheText.Get("FEM_TWP"));
#undef TEXT_PIECE
}

#undef ZOOM
#endif

// rowIdx 99999 returns total numbers of rows. otherwise it returns 0.
int
CMenuManager::ConstructStatLine(int rowIdx)
{
#define int_STAT_IS_FLOAT false
#define float_STAT_IS_FLOAT true
#define STAT_LINE_1(varType, left, right1) \
	do { \
		if(counter == rowIdx){ \
			varType a = right1; \
			BuildStatLine(left, &a, varType##_STAT_IS_FLOAT, nil); \
			return 0; \
		} counter++; \
	} while(0)

#define STAT_LINE_2(varType, left, right1, right2) \
	do { \
		if(counter == rowIdx){ \
			varType a = right1; \
			varType b = right2; \
			BuildStatLine(left, &a, varType##_STAT_IS_FLOAT, &b); \
			return 0; \
		} counter++; \
	} while(0)

#define TEXT_ON_LEFT_GXT(name) \
	do { \
		if(counter == rowIdx){ \
			BuildStatLine(name, nil, false, nil); \
			return 0; \
		} counter++; \
	} while(0)

#define TEXT_ON_RIGHT(text) \
	do { \
		if(counter == rowIdx){ \
			gUString[0] = '\0'; \
			UnicodeStrcpy(gUString2, text); \
			return 0; \
		} counter++; \
	} while(0)

	// Like TEXT_ON_LEFT_GXT, but counter wasn't initialized yet I think
	if (rowIdx == 0) {
		BuildStatLine("PL_STAT", nil, false, nil);
		return 0;
	}

	int percentCompleted = (CStats::TotalProgressInGame == 0 ? 0 :
		CStats::ProgressMade * 100.0f / (CGame::nastyGame ? CStats::TotalProgressInGame : CStats::TotalProgressInGame - 1));
	percentCompleted = Min(percentCompleted, 100);

	switch (rowIdx) {
		// 0 is the heading text above
		case 1: {
			BuildStatLine("PER_COM", &percentCompleted, false, nil);
			return 0;
		}
		case 2: {
			BuildStatLine("NMISON", &CStats::MissionsGiven, false, nil);
			return 0;
		}
		case 3: {
			BuildStatLine("FEST_MP", &CStats::MissionsPassed, false, &CStats::TotalNumberMissions);
			return 0;
		}
	}
	int counter = 4;

	if (CGame::nastyGame)
		STAT_LINE_2(int, "FEST_RP", CStats::NumberKillFrenziesPassed, CStats::TotalNumberKillFrenzies);
	
	CPlayerInfo &player = CWorld::Players[CWorld::PlayerInFocus];

	// Hidden packages shouldn't be shown with percent
#ifdef FIX_BUGS
	STAT_LINE_2(int, "PERPIC", player.m_nCollectedPackages, player.m_nTotalPackages);
#else
	float packagesPercent = 0.0f;
	if (player.m_nTotalPackages != 0)
		packagesPercent = player.m_nCollectedPackages * 100.0f / player.m_nTotalPackages;

	STAT_LINE_2(int, "PERPIC", packagesPercent, 100);
#endif
	STAT_LINE_2(int, "NOUNIF", CStats::NumberOfUniqueJumpsFound, CStats::TotalNumberOfUniqueJumps);
	STAT_LINE_1(int, "DAYSPS", CStats::DaysPassed);
	if (CGame::nastyGame) {
		STAT_LINE_1(int, "PE_WAST", CStats::PeopleKilledByPlayer);
		STAT_LINE_1(int, "PE_WSOT", CStats::PeopleKilledByOthers);
	}
	STAT_LINE_1(int, "CAR_EXP", CStats::CarsExploded);
	STAT_LINE_1(int, "TM_BUST", CStats::TimesArrested);
	STAT_LINE_1(int, "TM_DED", CStats::TimesDied);
	STAT_LINE_1(int, "GNG_WST", CStats::PedsKilledOfThisType[PEDTYPE_GANG9] + CStats::PedsKilledOfThisType[PEDTYPE_GANG8]
			+ CStats::PedsKilledOfThisType[PEDTYPE_GANG7] + CStats::PedsKilledOfThisType[PEDTYPE_GANG6]
			+ CStats::PedsKilledOfThisType[PEDTYPE_GANG5] + CStats::PedsKilledOfThisType[PEDTYPE_GANG4]
			+ CStats::PedsKilledOfThisType[PEDTYPE_GANG3] + CStats::PedsKilledOfThisType[PEDTYPE_GANG2]
			+ CStats::PedsKilledOfThisType[PEDTYPE_GANG1]);
	STAT_LINE_1(int, "DED_CRI", CStats::PedsKilledOfThisType[PEDTYPE_CRIMINAL]);
	STAT_LINE_1(int, "HEL_DST", CStats::HelisDestroyed);
	STAT_LINE_1(int, "KGS_EXP", CStats::KgsOfExplosivesUsed);
	STAT_LINE_1(int, "ACCURA", (CStats::InstantHitsFiredByPlayer == 0 ? 0 :
			CStats::InstantHitsHitByPlayer * 100.0f / CStats::InstantHitsFiredByPlayer));
	
	if (CStats::ElBurroTime > 0) {
		STAT_LINE_1(int, "ELBURRO", CStats::ElBurroTime);
	}
	if (CStats::Record4x4One > 0) {
		STAT_LINE_1(int, "FEST_R1", CStats::Record4x4One);
	}
	if (CStats::Record4x4Two > 0) {
		STAT_LINE_1(int, "FEST_R2", CStats::Record4x4Two);
	}
	if (CStats::Record4x4Three > 0) {
		STAT_LINE_1(int, "FEST_R3", CStats::Record4x4Three);
	}
	if (CStats::Record4x4Mayhem > 0) {
		STAT_LINE_1(int, "FEST_RM", CStats::Record4x4Mayhem);
	}
	if (CStats::LongestFlightInDodo > 0) {
		STAT_LINE_1(int, "FEST_LF", CStats::LongestFlightInDodo);
	}
	if (CStats::TimeTakenDefuseMission > 0) {
		STAT_LINE_1(int, "FEST_BD", CStats::TimeTakenDefuseMission);
	}
	STAT_LINE_1(int, "CAR_CRU", CStats::CarsCrushed);

	if (CStats::HighestScores[0] > 0) {
		TEXT_ON_LEFT_GXT("FEST_BB");
		STAT_LINE_1(int, "FEST_H0", CStats::HighestScores[0]);
	}
	if (CStats::HighestScores[4] + CStats::HighestScores[3] + CStats::HighestScores[2] + CStats::HighestScores[1] > 0) {
		TEXT_ON_LEFT_GXT("FEST_GC");
	}
	if (CStats::HighestScores[1] > 0) {
		STAT_LINE_1(int, "FEST_H1", CStats::HighestScores[1]);
	}
	if (CStats::HighestScores[2] > 0) {
		STAT_LINE_1(int, "FEST_H2", CStats::HighestScores[2]);
	}
	if (CStats::HighestScores[3] > 0) {
		STAT_LINE_1(int, "FEST_H3", CStats::HighestScores[3]);
	}
	if (CStats::HighestScores[4] > 0) {
		STAT_LINE_1(int, "FEST_H4", CStats::HighestScores[4]);
	}

	switch (m_PrefsLanguage) {
		case LANGUAGE_AMERICAN:
#ifndef USE_MEASUREMENTS_IN_METERS
			STAT_LINE_1(float, "FEST_DF", CStats::DistanceTravelledOnFoot * MILES_IN_METER);
			STAT_LINE_1(float, "FEST_DC", CStats::DistanceTravelledInVehicle * MILES_IN_METER);
			STAT_LINE_1(int, "MMRAIN", CStats::mmRain);
			STAT_LINE_1(float, "MXCARD", CStats::MaximumJumpDistance * FEET_IN_METER);
			STAT_LINE_1(float, "MXCARJ", CStats::MaximumJumpHeight * FEET_IN_METER);
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
			STAT_LINE_1(float, "FESTDFM", CStats::DistanceTravelledOnFoot);
			STAT_LINE_1(float, "FESTDCM", CStats::DistanceTravelledInVehicle);
			STAT_LINE_1(int, "MMRAIN", CStats::mmRain);
			STAT_LINE_1(float, "MXCARDM", CStats::MaximumJumpDistance);
			STAT_LINE_1(float, "MXCARJM", CStats::MaximumJumpHeight);
			break;
		default:
			break;
	}

	STAT_LINE_1(int, "MXFLIP", CStats::MaximumJumpFlips);
	STAT_LINE_1(int, "MXJUMP", CStats::MaximumJumpSpins);
	TEXT_ON_LEFT_GXT("BSTSTU");

	switch (CStats::BestStuntJump) {
		case 1:
			TEXT_ON_RIGHT(TheText.Get("INSTUN"));
			break;
		case 2:
			TEXT_ON_RIGHT(TheText.Get("PRINST"));
			break;
		case 3:
			TEXT_ON_RIGHT(TheText.Get("DBINST"));
			break;
		case 4:
			TEXT_ON_RIGHT(TheText.Get("DBPINS"));
			break;
		case 5:
			TEXT_ON_RIGHT(TheText.Get("TRINST"));
			break;
		case 6:
			TEXT_ON_RIGHT(TheText.Get("PRTRST"));
			break;
		case 7:
			TEXT_ON_RIGHT(TheText.Get("QUINST"));
			break;
		case 8:
			TEXT_ON_RIGHT(TheText.Get("PQUINS"));
			break;
		default:
			TEXT_ON_RIGHT(TheText.Get("NOSTUC"));
			break;
	}

	STAT_LINE_1(int, "PASDRO", CStats::PassengersDroppedOffWithTaxi);
	STAT_LINE_1(int, "MONTAX", CStats::MoneyMadeWithTaxi);
	STAT_LINE_1(int, "FEST_LS", CStats::LivesSavedWithAmbulance);
	STAT_LINE_1(int, "FEST_HA", CStats::HighestLevelAmbulanceMission);
	STAT_LINE_1(int, "FEST_CC", CStats::CriminalsCaught);
	STAT_LINE_1(int, "FEST_FE", CStats::FiresExtinguished);
	STAT_LINE_1(int, "DAYPLC", CTimer::GetTimeInMilliseconds() + 100);
	return counter;

#undef STAT_LINE_1
#undef STAT_LINE_2
#undef TEXT_ON_LEFT_GXT
#undef TEXT_ON_RIGHT
#undef int_STAT_IS_FLOAT
#undef float_STAT_IS_FLOAT
}

#undef GetBackJustUp
#undef GetBackJustDown
#undef ChangeScreen
    
#endif
