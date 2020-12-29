#if defined RW_D3D9 || defined RWLIBS
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#endif

#define WITHWINDOWS
#include "common.h"
#ifndef PS2_MENU
#include "vehicle_enums.h" //needs to be before Vehicle.h
#include "crossplatform.h"
#include "platform.h"

#include "Camera.h"
#include "CdStream.h"
#include "ControllerConfig.h"
#include "DMAudio.h"
#include "FileLoader.h"
#include "FileMgr.h"
#include "Font.h"
#include "Frontend.h"
#include "Game.h"
#include "General.h"
#include "GenericGameStorage.h"
#include "MBlur.h"
#include "Messages.h"
#include "Pad.h"
#include "PlayerInfo.h"
#include "PlayerSkin.h"
#include "Radar.h"
#include "Renderer.h"
#include "RwHelper.h"
#include "Script.h"
#include "Stats.h"
#include "Streaming.h"
#include "Text.h"
#include "Timer.h"
#include "TxdStore.h"
#include "User.h"
#include "Vehicle.h"
#include "World.h"
#include "main.h"
#include "sampman.h"

// --MIAMI: file done

// Similar story to Hud.cpp:
// Game has colors inlined in code.
// For easier modification we collect them here:
const CRGBA LABEL_COLOR(255, 150, 225, 255);
const CRGBA SELECTIONBORDER_COLOR(25, 130, 70, 255);
const CRGBA MENUOPTION_COLOR = LABEL_COLOR;
const CRGBA SELECTEDMENUOPTION_COLOR = LABEL_COLOR;
const CRGBA HEADER_COLOR = LABEL_COLOR;
const CRGBA DARKMENUOPTION_COLOR(195, 90, 165, 255);
const CRGBA SLIDERON_COLOR(97, 194, 247, 255);
const CRGBA SLIDEROFF_COLOR(27, 89, 130, 255);
const CRGBA LIST_BACKGROUND_COLOR(49, 101, 148, 130);
const CRGBA LIST_OPTION_COLOR(155, 155, 155, 255);
const CRGBA RADIO_SELECTOR_COLOR = SLIDEROFF_COLOR;
const CRGBA INACTIVE_RADIO_COLOR(100, 100, 255, 100);
const CRGBA SCROLLBAR_COLOR = LABEL_COLOR;

#define MAP_MIN_SIZE 162.f
#define MAP_SIZE_TO_ALLOW_X_MOVE 297.f

#define MAX_VISIBLE_LIST_ROW 30
#define SCROLLBAR_MAX_HEIGHT 263.0f // not in end result
#define SCROLLABLE_PAGES

#define hasNativeList(screen) (screen == MENUPAGE_SKIN_SELECT || screen == MENUPAGE_KEYBOARD_CONTROLS)

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

#define MINUS_SCROLL_OFFSET - scrollOffset
#else
#define MAX_VISIBLE_OPTION_ON_SCREEN MAX_VISIBLE_LIST_ROW
#define SETUP_SCROLLING(screen)
#define MINUS_SCROLL_OFFSET
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

#ifdef MAP_ENHANCEMENTS
CVector2D mapCrosshair;
#endif

#ifdef CUTSCENE_BORDERS_SWITCH
bool CMenuManager::m_PrefsCutsceneBorders = true;
#endif

bool holdingScrollBar; // *(bool*)0x7039B9; // not original name

CMenuManager FrontEndMenuManager;
MenuTrapezoid menuBg(CGeneral::GetRandomNumber() % 40 + 65, CGeneral::GetRandomNumber() % 40 + 21,
	CGeneral::GetRandomNumber() % 40 + 568, CGeneral::GetRandomNumber() % 40 + 44,
	CGeneral::GetRandomNumber() % 40 + 36, CGeneral::GetRandomNumber() % 40 + 352,
	CGeneral::GetRandomNumber() % 40 + 593, CGeneral::GetRandomNumber() % 40 + 312);

MenuTrapezoid menuOptionHighlight(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);

#ifndef MASTER
bool CMenuManager::m_PrefsMarketing = false;
bool CMenuManager::m_PrefsDisableTutorials = false;
#endif // !MASTER

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

#ifdef XBOX_MESSAGE_SCREEN
bool CMenuManager::m_bDialogOpen = false;
uint32 CMenuManager::m_nDialogHideTimer = 0;
PauseModeTime CMenuManager::m_nDialogHideTimerPauseMode = 0;
bool CMenuManager::m_bSaveWasSuccessful = false;
wchar* CMenuManager::m_pDialogText = nil;
#endif

#define SET_FONT_FOR_MENU_HEADER \
	CFont::SetRightJustifyOn(); \
	CFont::SetFontStyle(FONT_LOCALE(FONT_HEADING)); \
	CFont::SetScale(MENU_X(MENUHEADER_WIDTH), MENU_Y(MENUHEADER_HEIGHT)); \
	CFont::SetDropShadowPosition(0);

#define SET_FONT_FOR_LIST_ITEM \
	CFont::SetRightJustifyOff(); \
	CFont::SetScale(MENU_X(LISTITEM_X_SCALE), MENU_Y(LISTITEM_Y_SCALE)); \
	CFont::SetFontStyle(FONT_LOCALE(FONT_STANDARD));

#define RESET_FONT_FOR_NEW_PAGE \
	CFont::SetBackgroundOff(); \
	CFont::SetScale(MENU_X(MENUACTION_SCALE_MULT), MENU_Y(MENUACTION_SCALE_MULT)); \
	CFont::SetPropOn(); \
	CFont::SetCentreOff(); \
	CFont::SetJustifyOn(); \
	CFont::SetRightJustifyOff(); \
	CFont::SetBackGroundOnlyTextOn(); \
	CFont::SetWrapx(MENU_X_RIGHT_ALIGNED(MENU_X_MARGIN)); \
	CFont::SetRightJustifyWrap(MENU_X_LEFT_ALIGNED(MENU_X_MARGIN));

#define ProcessSlider(value, origY, increaseAction, decreaseAction, hoverEndX, onlyWhenHoveringRow) \
	do { \
		float y = origY MINUS_SCROLL_OFFSET; \
		lastActiveBarX = DisplaySlider(MENU_X_LEFT_ALIGNED(MENUSLIDER_X), MENU_Y(y), MENU_Y(MENUSLIDER_SMALLEST_BAR), MENU_Y(MENUSLIDER_BIGGEST_BAR), MENU_X(MENUSLIDER_UNK), value, MENU_X(3.0f)); \
		if (i != m_nCurrOption || !itemsAreSelectable) \
			break; \
		 \
		if (CheckHover(0, lastActiveBarX - MENU_X(3.0f), MENU_Y(y), MENU_Y(MENUSLIDER_BIGGEST_BAR + y))) { \
			m_nHoverOption = decreaseAction; \
			break; \
		} \
		if (!CheckHover(MENU_X(3.0f) + lastActiveBarX, hoverEndX, MENU_Y(y), MENU_Y(MENUSLIDER_BIGGEST_BAR + y))) { \
			m_nHoverOption = HOVEROPTION_NOT_HOVERING; \
			break; \
		} \
		m_nHoverOption = increaseAction; \
		if (m_nMousePosX < MENU_X_LEFT_ALIGNED(MENUSLIDER_X)) \
			m_nHoverOption = HOVEROPTION_NOT_HOVERING; \
		\
		if (onlyWhenHoveringRow && (m_nMousePosY < MENU_Y(y) || m_nMousePosY > MENU_Y(MENUSLIDER_BIGGEST_BAR + y))) \
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

inline void
CMenuManager::PageUpList(bool playSoundOnSuccess)
{
	if (m_nTotalListRow > MAX_VISIBLE_OPTION_ON_SCREEN) {
		if (m_nFirstVisibleRowOnList > 0) {
			if(playSoundOnSuccess)
				DMAudio.PlayFrontEndSound(SOUND_FRONTEND_HIGHLIGHT_OPTION, 0);

			m_nFirstVisibleRowOnList = Max(0, m_nFirstVisibleRowOnList - MAX_VISIBLE_OPTION_ON_SCREEN);
			m_nSelectedListRow = Min(m_nSelectedListRow, m_nFirstVisibleRowOnList + MAX_VISIBLE_OPTION_ON_SCREEN - 1);
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
	if (m_nTotalListRow > MAX_VISIBLE_OPTION_ON_SCREEN) {
		if (m_nFirstVisibleRowOnList < m_nTotalListRow - MAX_VISIBLE_OPTION_ON_SCREEN) {
			if(playSoundOnSuccess)
				DMAudio.PlayFrontEndSound(SOUND_FRONTEND_HIGHLIGHT_OPTION, 0);

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
#define PLUS_LINE_HEIGHT_ON_SCREEN + (aScreens[m_nCurrScreen].layout ? aScreens[m_nCurrScreen].layout->lineHeight : MENU_DEFAULT_LINE_HEIGHT)
bool ScreenHasOption(int screen, const char* gxtKey)
{
	for (int i = 0; i < NUM_MENUROWS; i++) {
		if (strcmp(gxtKey, aScreens[screen].m_aEntries[i].m_EntryName) == 0)
			return true;
	}
	return false;
}

inline void
CMenuManager::ThingsToDoBeforeLeavingPage()
{
	if ((m_nCurrScreen == MENUPAGE_SKIN_SELECT) && strcmp(m_aSkinName, m_PrefsSkinFile) != 0) {
		CWorld::Players[0].SetPlayerSkin(m_PrefsSkinFile);

	} else if (m_nCurrScreen == MENUPAGE_SOUND_SETTINGS) {
		if (m_nPrefsAudio3DProviderIndex != NO_AUDIO_PROVIDER)
			m_nPrefsAudio3DProviderIndex = DMAudio.GetCurrent3DProviderIndex();

		DMAudio.StopFrontEndTrack();
		OutputDebugString("FRONTEND AUDIO TRACK STOPPED");

	} else if (ScreenHasOption(m_nCurrScreen, "FED_RES")) {
		m_nDisplayVideoMode = m_nPrefsVideoMode;
	}

	if (m_nCurrScreen == MENUPAGE_SKIN_SELECT) {
		CPlayerSkin::EndFrontendSkinEdit();
	}

#ifdef SCROLLABLE_PAGES
	if (SCREEN_HAS_AUTO_SCROLLBAR) {
		m_nSelectedListRow = 0;
		m_nFirstVisibleRowOnList = 0;
		m_nScrollbarTopMargin = 0;
	}
#endif

	CMenuScreenCustom::CMenuEntry &option = aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption];

	if (option.m_Action == MENUACTION_CFO_DYNAMIC)
		if(option.m_CFODynamic->buttonPressFunc)
			option.m_CFODynamic->buttonPressFunc(FEOPTION_ACTION_FOCUSLOSS);

	if (option.m_Action == MENUACTION_CFO_SELECT && option.m_CFOSelect->onlyApplyOnEnter && option.m_CFOSelect->lastSavedValue != option.m_CFOSelect->displayedValue)
		option.m_CFOSelect->displayedValue = *option.m_CFO->value = option.m_CFOSelect->lastSavedValue;

	if (aScreens[m_nCurrScreen].returnPrevPageFunc) {
		aScreens[m_nCurrScreen].returnPrevPageFunc();
	}
}

inline int8
CMenuManager::GetPreviousPageOption()
{
	int8 prevPage = !m_bGameNotLoaded ? aScreens[m_nCurrScreen].m_PreviousPage :
		(m_nCurrScreen == MENUPAGE_NEW_GAME || m_nCurrScreen == MENUPAGE_OPTIONS || m_nCurrScreen == MENUPAGE_EXIT ? MENUPAGE_START_MENU : aScreens[m_nCurrScreen].m_PreviousPage);

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
}

#else
#define PLUS_LINE_HEIGHT_ON_SCREEN + MENU_DEFAULT_LINE_HEIGHT
inline void
CMenuManager::ThingsToDoBeforeLeavingPage()
{
	switch (m_nCurrScreen) {
		case MENUPAGE_SOUND_SETTINGS:
			if (m_nPrefsAudio3DProviderIndex != NO_AUDIO_PROVIDER)
				m_nPrefsAudio3DProviderIndex = DMAudio.GetCurrent3DProviderIndex();

			DMAudio.StopFrontEndTrack();
			OutputDebugString("FRONTEND AUDIO TRACK STOPPED");
			break;
		case MENUPAGE_DISPLAY_SETTINGS:
			m_nDisplayVideoMode = m_nPrefsVideoMode;
			break;
		case MENUPAGE_SKIN_SELECT:
			if (strcmp(m_aSkinName, m_PrefsSkinFile) != 0)
				CWorld::Players[0].SetPlayerSkin(m_PrefsSkinFile);

			CPlayerSkin::EndFrontendSkinEdit();
			break;
	}

#ifdef SCROLLABLE_PAGES
	if (SCREEN_HAS_AUTO_SCROLLBAR) {
		m_nSelectedListRow = 0;
		m_nFirstVisibleRowOnList = 0;
		m_nScrollbarTopMargin = 0;
	}
#endif
}

inline int8
CMenuManager::GetPreviousPageOption()
{
	return (!m_bGameNotLoaded ? aScreens[m_nCurrScreen].m_ParentEntry :
		(m_nCurrScreen == MENUPAGE_NEW_GAME ? 0 : (m_nCurrScreen == MENUPAGE_OPTIONS ? 1 : (m_nCurrScreen == MENUPAGE_EXIT ? 2 : aScreens[m_nCurrScreen].m_ParentEntry))));
}
#endif

// ------ Functions not in the game/inlined ends

bool DoRWStuffStartOfFrame(int16 TopRed, int16 TopGreen, int16 TopBlue, int16 BottomRed, int16 BottomGreen, int16 BottomBlue, int16 Alpha);
void DoRWStuffEndOfFrame(void);

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
		int oldOption = GetPreviousPageOption();

		m_nCurrOption = oldOption;
		m_nCurrScreen = oldScreen;
	} else if (screen == 0) {
		m_nCurrScreen = aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption].m_TargetMenu;
		m_nCurrOption = 0;
	} else {
		m_nCurrOption = 0;
		m_nCurrScreen = screen;
	}
	SETUP_SCROLLING(m_nCurrScreen)
	
	if (hasNativeList(m_nPrevScreen))
		m_nTotalListRow = 0;

	if (m_nCurrScreen == MENUPAGE_CHOOSE_SAVE_SLOT)
		m_nCurrOption = 8;
	m_nMenuFadeAlpha = 0;
	m_nOptionHighlightTransitionBlend = 0;
	m_LastScreenSwitch = CTimer::GetTimeInMillisecondsPauseMode();
}

CMenuManager::CMenuManager()
{
	m_StatsScrollSpeed = 150.0f;
	m_StatsScrollDirection = 1;
	m_PrefsSfxVolume = 49;
	m_PrefsMusicVolume = 49;
	m_PrefsRadioStation = 0;
	m_PrefsStereoMono = 1;
	m_PrefsBrightness = 256;
	m_PrefsLOD = CRenderer::ms_lodDistScale;
	m_KeyPressedCode = -1;
	m_bFrontEnd_ReloadObrTxtGxt = false;
	m_PrefsMP3BoostVolume = 0;
	m_PrefsShowSubtitles = 0;
	m_PrefsShowLegends = 1;
#ifdef ASPECT_RATIO_SCALE
	m_PrefsUseWideScreen = AR_AUTO;
#else
	m_PrefsUseWideScreen = 0;
#endif
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
	m_DisplayControllerOnFoot = false;
	m_bShutDownFrontEndRequested = false;
	m_bStartUpFrontEndRequested = false;
	pEditString = nil;
	pControlEdit = nil;
	DisplayComboButtonErrMsg = false;
	m_PrefsDMA = 1;
	OS_Language = LANG_ENGLISH;
	m_ControlMethod = CONTROL_STANDARD;
#ifdef PC_PLAYER_CONTROLS
	CCamera::m_bUseMouse3rdPerson = true;
#else
	CCamera::m_bUseMouse3rdPerson = false;
#endif
	m_lastWorking3DAudioProvider = 0;
	m_nFirstVisibleRowOnList = 0;
	m_nScrollbarTopMargin = 0.0f;
	m_nSelectedListRow = 0;
	m_nSkinsTotal = 0;
	m_nPrefsAudio3DProviderIndex = AUDIO_PROVIDER_NOT_DETERMINED;
	m_bGameNotLoaded = true;
	m_nMousePosX = m_nMouseTempPosX;
	m_nMousePosY = m_nMouseTempPosY;
	m_nMouseOldPosX = m_nMousePosX;
	m_nMouseOldPosY = m_nMousePosY;
	m_bShowMouse = true;
	m_nHoverOption = HOVEROPTION_NOT_HOVERING;

	DMAudio.SetMP3BoostVolume(m_PrefsMP3BoostVolume);
	m_bMenuActive = false;
	m_bActivateSaveMenu = false;
	m_bWantToLoad = false;
	m_nMenuFadeAlpha = 0;
	m_OnlySaveMenu = false;
	m_fMapSize = MENU_Y(162.0f); // Y because of HOR+
	m_fMapCenterX = MENU_X_LEFT_ALIGNED(320.0f);
	m_fMapCenterY = MENU_Y(225.0f);
	DMAudio.SetMusicMasterVolume(m_PrefsMusicVolume);
	DMAudio.SetEffectsMasterVolume(m_PrefsSfxVolume);

#ifdef NO_ISLAND_LOADING
	m_PrefsIslandLoading = ISLAND_LOADING_LOW;
#endif
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
	m_firstStartCounter = -50; // to start from black
	m_nMenuFadeAlpha = 0;
	m_nCurrOption = 0;
	m_nOptionHighlightTransitionBlend = 0;
	CentreMousePointer();
	m_bShowMouse = true;
	m_fMapSize = MENU_Y(162.0f); // Y because of HOR+
	m_fMapCenterX = MENU_X_LEFT_ALIGNED(320.0f);
	m_fMapCenterY = MENU_Y(225.0f);
	CPad::StopPadsShaking();
	if (!m_OnlySaveMenu)
		m_nCurrScreen = MENUPAGE_NONE;
	DMAudio.ChangeMusicMode(MUSICMODE_FRONTEND);
	DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_STARTING, 0);
	DMAudio.Service();
	DMAudio.SetMusicMasterVolume(m_PrefsMusicVolume);
	DMAudio.SetEffectsMasterVolume(m_PrefsSfxVolume);
	m_PrefsRadioStation = DMAudio.GetRadioInCar();
	DMAudio.SetMP3BoostVolume(m_PrefsMP3BoostVolume);
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
		} else if (typeOfControl == MOUSE) {
			ControlsManager.DeleteMatchingActionInitiators(action, MouseButtonJustClicked, MOUSE);
		} else if (typeOfControl == JOYSTICK) {
			ControlsManager.DeleteMatchingActionInitiators(action, JoyButtonJustClicked, JOYSTICK);
		}

		if (typeOfControl == KEYBOARD) {
			ControlsManager.SetControllerKeyAssociatedWithAction(action, *pControlEdit, typeToSave);
		} else if (typeOfControl == MOUSE) {
			ControlsManager.SetControllerKeyAssociatedWithAction(action, MouseButtonJustClicked, typeToSave);
		} else if (typeOfControl == JOYSTICK) {
			ControlsManager.SetControllerKeyAssociatedWithAction(action, JoyButtonJustClicked, typeToSave);
		}
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
		m_PrefsBrightness += value * 24.19f;
		m_PrefsBrightness = clamp(m_PrefsBrightness, 0, 384);
		break;
	case MENUACTION_DRAWDIST:
		if(value > 0)
			m_PrefsLOD += ((1.8f - 0.925f) / 16.0f);
		else
			m_PrefsLOD -= ((1.8f - 0.925f) / 16.0f);
		m_PrefsLOD = clamp(m_PrefsLOD, 0.925f, 1.8f);
		CRenderer::ms_lodDistScale = m_PrefsLOD;
		break;
	case MENUACTION_MUSICVOLUME:
		if (m_nPrefsAudio3DProviderIndex != NO_AUDIO_PROVIDER) {
			m_PrefsMusicVolume += value * (128 / 32);
			m_PrefsMusicVolume = clamp(m_PrefsMusicVolume, 0, 65);
			DMAudio.SetMusicMasterVolume(m_PrefsMusicVolume);
		}
		break;
	case MENUACTION_SFXVOLUME:
		if (m_nPrefsAudio3DProviderIndex != NO_AUDIO_PROVIDER) {
			m_PrefsSfxVolume += value * (128 / 32);
			m_PrefsSfxVolume = clamp(m_PrefsSfxVolume, 0, 65);
			DMAudio.SetEffectsMasterVolume(m_PrefsSfxVolume);
		}
		break;
	case MENUACTION_MP3VOLUMEBOOST:
		if (m_nPrefsAudio3DProviderIndex != NO_AUDIO_PROVIDER) {
			if (DMAudio.IsMP3RadioChannelAvailable()) {
				m_PrefsMP3BoostVolume += value * (128 / 32);
				m_PrefsMP3BoostVolume = clamp(m_PrefsMP3BoostVolume, 0, 65);
				DMAudio.SetMP3BoostVolume(m_PrefsMP3BoostVolume);
			}
		}
		break;
	case MENUACTION_MOUSESENS:
		TheCamera.m_fMouseAccelHorzntl += value * 1.0f/200.0f/15.0f;	// ???
		TheCamera.m_fMouseAccelHorzntl = clamp(TheCamera.m_fMouseAccelHorzntl, 1.0f/3200.0f, 1.0f/200.0f);
#ifdef FIX_BUGS
		TheCamera.m_fMouseAccelVertical = TheCamera.m_fMouseAccelHorzntl + 0.0005f;
#endif
		break;
	default:
		return;
	}
	SaveSettings();
}

void
CMenuManager::DisplayHelperText(char *text)
{
	if (m_nMenuFadeAlpha != 255)
		return;

	// there was a unused static bool
	static PauseModeTime LastFlash = 0;
	int32 alpha = 255;

	CFont::SetRightJustifyOn();
	CFont::SetScale(SCREEN_SCALE_X(SMALLESTTEXT_X_SCALE), SCREEN_SCALE_Y(SMALLESTTEXT_Y_SCALE));
	CFont::SetFontStyle(FONT_LOCALE(FONT_STANDARD));
	CFont::SetDropShadowPosition(0);

	// We're using SCREEN_STRETCH_FROM_RIGHT, because we also stretch black borders
	if (text) {
		CFont::SetColor(CRGBA(255, 255, 255, 255));
		CFont::PrintString(SCREEN_STRETCH_FROM_RIGHT(HELPER_TEXT_RIGHT_MARGIN), SCREEN_SCALE_FROM_BOTTOM(HELPER_TEXT_BOTTOM_MARGIN), TheText.Get(text));
		return;
	}

	if (m_nHelperTextMsgId != 0 && m_nHelperTextMsgId != 1) {
		if (CTimer::GetTimeInMillisecondsPauseMode() - LastFlash > 10) {
			LastFlash = CTimer::GetTimeInMillisecondsPauseMode();
			m_nHelperTextAlpha -= 2;
		}

		if (m_nHelperTextAlpha < 1)
			ResetHelperText();

		alpha = m_nHelperTextAlpha > 255 ? 255 : m_nHelperTextAlpha;
	}

	CFont::SetColor(CRGBA(255, 255, 255, alpha));
	// TODO: name this cases?
	switch (m_nHelperTextMsgId) {
		case 1:
			CFont::PrintString(SCREEN_STRETCH_FROM_RIGHT(HELPER_TEXT_RIGHT_MARGIN), SCREEN_SCALE_FROM_BOTTOM(HELPER_TEXT_BOTTOM_MARGIN), TheText.Get("FET_APP"));
			break;
		case 2:
			CFont::PrintString(SCREEN_STRETCH_FROM_RIGHT(HELPER_TEXT_RIGHT_MARGIN), SCREEN_SCALE_FROM_BOTTOM(HELPER_TEXT_BOTTOM_MARGIN), TheText.Get("FET_HRD"));
			break;
		case 3:
			CFont::PrintString(SCREEN_STRETCH_FROM_RIGHT(HELPER_TEXT_RIGHT_MARGIN), SCREEN_SCALE_FROM_BOTTOM(HELPER_TEXT_BOTTOM_MARGIN), TheText.Get("FET_RSO"));
			break;
		case 4:
			CFont::PrintString(SCREEN_STRETCH_FROM_RIGHT(HELPER_TEXT_RIGHT_MARGIN), SCREEN_SCALE_FROM_BOTTOM(HELPER_TEXT_BOTTOM_MARGIN), TheText.Get("FET_STS"));
			break;
		case 5:
			CFont::PrintString(SCREEN_STRETCH_FROM_RIGHT(HELPER_TEXT_RIGHT_MARGIN), SCREEN_SCALE_FROM_BOTTOM(HELPER_TEXT_BOTTOM_MARGIN), TheText.Get("FET_RSC"));
			break;
		default:
			if (aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption].m_Action == MENUACTION_NO)
				return;

			if (aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption].m_Action == MENUACTION_MUSICVOLUME ||
				aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption].m_Action == MENUACTION_SFXVOLUME) {

				CFont::PrintString(SCREEN_STRETCH_FROM_RIGHT(HELPER_TEXT_RIGHT_MARGIN), SCREEN_SCALE_FROM_BOTTOM(HELPER_TEXT_BOTTOM_MARGIN),
					m_nPrefsAudio3DProviderIndex == NO_AUDIO_PROVIDER ? TheText.Get("FEH_NA") : TheText.Get("FET_MIG"));
				return;
			}

			if (aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption].m_Action == MENUACTION_KEYBOARDCTRLS)
				return;

			if (aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption].m_Action == MENUACTION_SCREENRES) {
				CFont::PrintString(SCREEN_STRETCH_FROM_RIGHT(HELPER_TEXT_RIGHT_MARGIN), SCREEN_SCALE_FROM_BOTTOM(HELPER_TEXT_BOTTOM_MARGIN),
					m_bGameNotLoaded ? TheText.Get("FET_MIG") : TheText.Get("FEH_NA"));
				return;
			}

			if (aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption].m_Action == MENUACTION_AUDIOHW ||
				aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption].m_Action == MENUACTION_SPEAKERCONF) {

				CFont::PrintString(SCREEN_STRETCH_FROM_RIGHT(HELPER_TEXT_RIGHT_MARGIN), SCREEN_SCALE_FROM_BOTTOM(HELPER_TEXT_BOTTOM_MARGIN),
					m_nPrefsAudio3DProviderIndex == NO_AUDIO_PROVIDER ? TheText.Get("FEH_NA") : TheText.Get("FET_MIG"));
				return;
			}

			if (aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption].m_Action == MENUACTION_RESTOREDEF)
				return;

			if (aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption].m_Action == MENUACTION_MP3VOLUMEBOOST) {
				CFont::PrintString(SCREEN_STRETCH_FROM_RIGHT(HELPER_TEXT_RIGHT_MARGIN), SCREEN_SCALE_FROM_BOTTOM(HELPER_TEXT_BOTTOM_MARGIN),
					m_nPrefsAudio3DProviderIndex == NO_AUDIO_PROVIDER ? TheText.Get("FEH_NA") : TheText.Get("FET_MIG"));
				return;
			}

			CFont::PrintString(SCREEN_STRETCH_FROM_RIGHT(HELPER_TEXT_RIGHT_MARGIN), SCREEN_SCALE_FROM_BOTTOM(HELPER_TEXT_BOTTOM_MARGIN),
				m_nCurrScreen != MENUPAGE_STATS ? TheText.Get("FET_MIG") : TheText.Get("FEH_SSA"));

			break;
	}
}

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

void
CMenuManager::DoSettingsBeforeStartingAGame()
{
#ifdef LEGACY_MENU_OPTIONS
	if (m_PrefsVsyncDisp != m_PrefsVsync)
		m_PrefsVsync = m_PrefsVsyncDisp;
#endif
	DMAudio.DestroyAllGameCreatedEntities();
	DMAudio.Service();
	m_bShutDownFrontEndRequested = true;
	m_bWantToRestart = true;
	DMAudio.SetEffectsFadeVol(0);
	DMAudio.SetMusicFadeVol(0);
	for (int i = 0; i < NUM_RADIOS; i++)
		CStats::FavoriteRadioStationList[i] = 0.0f;

	SwitchMenuOnAndOff();
	DMAudio.ResetTimers(CTimer::GetTimeInMilliseconds());
}

void
CMenuManager::DrawStandardMenus(bool activeScreen)
{
	float nextYToUse = 0.0f; // III leftover, set but unused in VC
	bool itemsAreSelectable = true;
	CFont::SetBackgroundOff();
	CFont::SetPropOn();
	CFont::SetCentreOff();
	CFont::SetJustifyOn();
	CFont::SetBackGroundOnlyTextOff();

#ifdef CUSTOM_FRONTEND_OPTIONS
	const int xMargin = aScreens[m_nCurrScreen].layout && aScreens[m_nCurrScreen].layout->xMargin != 0 ? aScreens[m_nCurrScreen].layout->xMargin : MENU_X_MARGIN;
#else
	const int xMargin = MENU_X_MARGIN;
#endif

	CFont::SetWrapx(MENU_X_RIGHT_ALIGNED(xMargin));
	CFont::SetRightJustifyWrap(MENU_X_LEFT_ALIGNED(xMargin));
#ifdef ASPECT_RATIO_SCALE
	CFont::SetCentreSize(SCREEN_SCALE_X(DEFAULT_SCREEN_WIDTH));
#else
	CFont::SetCentreSize(SCREEN_WIDTH);
#endif

	switch (m_nCurrScreen) {
		case MENUPAGE_CHOOSE_LOAD_SLOT:
		case MENUPAGE_CHOOSE_DELETE_SLOT:
		case MENUPAGE_CHOOSE_SAVE_SLOT:
			CSprite2d::Draw2DPolygon(MENU_X_LEFT_ALIGNED(38.0f), MENU_Y(85.0f),
				MENU_X_LEFT_ALIGNED(615.0f), MENU_Y(75.0f),
				MENU_X_LEFT_ALIGNED(30.0f), MENU_Y(320.0f), 
				MENU_X_LEFT_ALIGNED(605.0f), MENU_Y(330.0f), CRGBA(LIST_BACKGROUND_COLOR.r, LIST_BACKGROUND_COLOR.g, LIST_BACKGROUND_COLOR.b, FadeIn(LIST_BACKGROUND_COLOR.a)));
			break;
		case MENUPAGE_SOUND_SETTINGS:
			PrintRadioSelector();
			break;
		case MENUPAGE_STATS:
			PrintStats();
			break;
		case MENUPAGE_BRIEFS:
			PrintBriefs();
			break;
	}

	// Page name
	if (aScreens[m_nCurrScreen].m_ScreenName[0] != '\0') {

		SET_FONT_FOR_MENU_HEADER
		CFont::SetColor(CRGBA(30, 30, 30, FadeIn(255)));
		CFont::PrintString(SCREEN_STRETCH_FROM_RIGHT(MENUHEADER_POS_X) - MENU_X(7.f), SCREEN_SCALE_Y(MENUHEADER_POS_Y + 7.f), TheText.Get(aScreens[m_nCurrScreen].m_ScreenName));

		CFont::SetColor(CRGBA(HEADER_COLOR.r, HEADER_COLOR.g, HEADER_COLOR.b, FadeIn(255)));
		CFont::PrintString(SCREEN_STRETCH_FROM_RIGHT(MENUHEADER_POS_X), SCREEN_SCALE_Y(MENUHEADER_POS_Y), TheText.Get(aScreens[m_nCurrScreen].m_ScreenName));
	}

	// Label
	wchar *str;
	if (aScreens[m_nCurrScreen].m_aEntries[0].m_Action == MENUACTION_LABEL) {
		CFont::SetWrapx(MENU_X_RIGHT_ALIGNED(MENULABEL_X_MARGIN));
		CFont::SetRightJustifyWrap(MENU_X_LEFT_ALIGNED(MENULABEL_X_MARGIN));
		CFont::SetFontStyle(FONT_LOCALE(FONT_STANDARD));
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
			if (Slots[m_nCurrSaveSlot] == SLOT_OK)
				str = TheText.Get("FESZ_QO");
			else if (Slots[m_nCurrSaveSlot] == SLOT_CORRUPTED)
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
		CFont::SetWrapx(MENU_X_RIGHT_ALIGNED(xMargin));
		CFont::SetRightJustifyWrap(MENU_X_LEFT_ALIGNED(xMargin));
	}

	if (m_nCurrScreen == MENUPAGE_KEYBOARD_CONTROLS) {
		if (m_bWaitingForNewKeyBind)
			itemsAreSelectable = false;

		DrawControllerScreenExtraText(-8.0f, MENU_X_LEFT_ALIGNED(350), MENU_DEFAULT_LINE_HEIGHT);
	}

	wchar unicodeTemp[64];
	char asciiTemp[32];

	bool weHaveLabel = aScreens[m_nCurrScreen].m_aEntries[0].m_Action == MENUACTION_LABEL;
	uint8 section = 0; // 0: highlight trapezoid  1: texts

	while (section < 2) {
#endif

#ifdef SCROLLABLE_PAGES
		int firstOption = SCREEN_HAS_AUTO_SCROLLBAR ? m_nFirstVisibleRowOnList : 0;
		int scrollOffset = aScreens[m_nCurrScreen].m_aEntries[firstOption].m_Y - aScreens[m_nCurrScreen].m_aEntries[0].m_Y;
		for (int i = firstOption; i < firstOption + MAX_VISIBLE_OPTION && i < NUM_MENUROWS; ++i) {
#else
		for (int i = 0; i < NUM_MENUROWS; ++i) {
#endif
			wchar* rightText = nil;
			wchar* leftText;
			if (aScreens[m_nCurrScreen].m_aEntries[i].m_SaveSlot >= SAVESLOT_1 && aScreens[m_nCurrScreen].m_aEntries[i].m_SaveSlot <= SAVESLOT_8) {
				CFont::SetColor(CRGBA(0, 0, 0, FadeIn(255)));
				CFont::SetFontStyle(FONT_LOCALE(FONT_STANDARD));
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
			if (aScreens[m_nCurrScreen].m_aEntries[i].m_X == 0 && aScreens[m_nCurrScreen].m_aEntries[i].m_Y == 0) {
				if (i == 0 || (i == 1 && weHaveLabel)) {
#ifdef CUSTOM_FRONTEND_OPTIONS
					aScreens[m_nCurrScreen].m_aEntries[i].m_X = (aScreens[m_nCurrScreen].layout ? aScreens[m_nCurrScreen].layout->startX : MENU_DEFAULT_CONTENT_X);
					aScreens[m_nCurrScreen].m_aEntries[i].m_Y = (aScreens[m_nCurrScreen].layout ? aScreens[m_nCurrScreen].layout->startY : MENU_DEFAULT_CONTENT_Y);
#else
					aScreens[m_nCurrScreen].m_aEntries[i].m_X = MENU_DEFAULT_CONTENT_X;
					aScreens[m_nCurrScreen].m_aEntries[i].m_Y = MENU_DEFAULT_CONTENT_Y;
#endif
					
				} else {
					aScreens[m_nCurrScreen].m_aEntries[i].m_X = aScreens[m_nCurrScreen].m_aEntries[i-1].m_X;
					aScreens[m_nCurrScreen].m_aEntries[i].m_Y = aScreens[m_nCurrScreen].m_aEntries[i-1].m_Y PLUS_LINE_HEIGHT_ON_SCREEN;
				}
			}
#ifdef CUSTOM_FRONTEND_OPTIONS
			else if (aScreens[m_nCurrScreen].m_aEntries[i].m_Y == 0) {
				aScreens[m_nCurrScreen].m_aEntries[i].m_Y = aScreens[m_nCurrScreen].m_aEntries[i-1].m_Y PLUS_LINE_HEIGHT_ON_SCREEN;
			}
#endif

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

				if (m_nPrefsAudio3DProviderIndex == NO_AUDIO_PROVIDER) {
					if (strcmp(aScreens[m_nCurrScreen].m_aEntries[i].m_EntryName, "FEO_AUD") == 0) {
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
				// This one is still in enum and ProcessOnOffMenuOptions, but removed from other places
				case MENUACTION_CTRLDISPLAY:
					if (m_DisplayControllerOnFoot)
						rightText = TheText.Get("FEC_ONF");
					else
						rightText = TheText.Get("FEC_INC");
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
					if (m_nPrefsAudio3DProviderIndex == NO_AUDIO_PROVIDER)
						rightText = TheText.Get("FEA_NAH");

					break;
				case MENUACTION_RADIO:
					switch (m_PrefsRadioStation) {
						case WILDSTYLE:
							rightText = TheText.Get("FEA_FM0");
							break;
						case FLASH_FM:
							rightText = TheText.Get("FEA_FM1");
							break;
						case KCHAT:
							rightText = TheText.Get("FEA_FM2");
							break;
						case FEVER:
							rightText = TheText.Get("FEA_FM3");
							break;
						case V_ROCK:
							rightText = TheText.Get("FEA_FM4");
							break;
						case VCPR:
							rightText = TheText.Get("FEA_FM5");
							break;
						case RADIO_ESPANTOSO:
							rightText = TheText.Get("FEA_FM6");
							break;
						case EMOTION:
							rightText = TheText.Get("FEA_FM7");
							break;
						case WAVE:
							rightText = TheText.Get("FEA_FM8");
							break;
						case USERTRACK:
							rightText = TheText.Get("FEA_MP3");
							break;
					}
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
					break;
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
				case MENUACTION_AUDIOHW:
					if (m_nPrefsAudio3DProviderIndex == NO_AUDIO_PROVIDER)
						rightText = TheText.Get("FEA_NAH");
					else if (m_nPrefsAudio3DProviderIndex == -1)
						rightText = TheText.Get("FEA_ADP");
					else {
						char *rawProvider = DMAudio.Get3DProviderName(m_nPrefsAudio3DProviderIndex);
						AsciiToUnicode(rawProvider, unicodeTemp);
						char *provider = UnicodeToAscii(unicodeTemp); // genius
						strupr(provider);
						if (!strcmp(provider, "DIRECTSOUND3D HARDWARE SUPPORT")) {
							strcpy(provider, "DSOUND3D HARDWARE SUPPORT");
						} else if (!strcmp(provider, "DIRECTSOUND3D SOFTWARE EMULATION")) {
							strcpy(provider, "DSOUND3D SOFTWARE EMULATION");
						}
						AsciiToUnicode(provider, unicodeTemp);
						rightText = unicodeTemp;
					}
					break;
				case MENUACTION_SPEAKERCONF: {
					if (m_nPrefsAudio3DProviderIndex == NO_AUDIO_PROVIDER)
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
#ifdef CUSTOM_FRONTEND_OPTIONS
				case MENUACTION_CFO_DYNAMIC:
				case MENUACTION_CFO_SELECT:
					CMenuScreenCustom::CMenuEntry &option = aScreens[m_nCurrScreen].m_aEntries[i];
					if (option.m_Action == MENUACTION_CFO_SELECT) {
						// To whom manipulate option.m_CFO->value of static options externally (like RestoreDef functions)
						if (*option.m_CFO->value != option.m_CFOSelect->lastSavedValue)
							option.m_CFOSelect->displayedValue = option.m_CFOSelect->lastSavedValue = *option.m_CFO->value;

						if (option.m_CFOSelect->displayedValue >= option.m_CFOSelect->numRightTexts || option.m_CFOSelect->displayedValue < 0)
							option.m_CFOSelect->displayedValue = 0;

						rightText = TheText.Get(option.m_CFOSelect->rightTexts[option.m_CFOSelect->displayedValue]);

					} else if (option.m_Action == MENUACTION_CFO_DYNAMIC) {
						if (option.m_CFODynamic->drawFunc) {
							bool isOptionDisabled = false;
							rightText = option.m_CFODynamic->drawFunc(&isOptionDisabled, m_nCurrOption == i);
							if (isOptionDisabled)
								CFont::SetColor(CRGBA(DARKMENUOPTION_COLOR.r, DARKMENUOPTION_COLOR.g, DARKMENUOPTION_COLOR.b, FadeIn(255)));
						}
					}
					break;
#endif
				}

				// Highlight trapezoid
				if (activeScreen && i == m_nCurrOption && itemsAreSelectable && section == 0) {

					int leftXMax, rightXMin;

					// FIX: Let's don't scale those so GetStringWidth can give us unscaled width, which will be handy to other calculations below that's done without scaling in mind,
					//		and scaling will be done eventually.
					// CFont::SetScale(MENU_X(BIGTEXT_X_SCALE), MENU_Y(BIGTEXT_Y_SCALE));
					CFont::SetScale(BIGTEXT_X_SCALE, BIGTEXT_Y_SCALE);
					
					wchar *curOptionName = TheText.Get(aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption].m_EntryName);
					float curOptionWidth = CFont::GetStringWidth(curOptionName, true);

					if (CFont::Details.centre) {
						leftXMax = Max(0, aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption].m_X - curOptionWidth / 2.f);
						rightXMin = Min(DEFAULT_SCREEN_WIDTH, curOptionWidth / 2.f + aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption].m_X);

					} else if (!CFont::Details.rightJustify) {
						leftXMax = aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption].m_X;
						rightXMin = Min(DEFAULT_SCREEN_WIDTH, curOptionWidth + aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption].m_X);

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

					int y = aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption].m_Y MINUS_SCROLL_OFFSET;
					int topYMax = y;
					uint32 bottomYMin = y + MENU_DEFAULT_LINE_HEIGHT - 7; // Decreasing is not recommended. Because this actually is dependent to font scale, not line height.

					// Actually bottomRight and bottomLeft should be exchanged here(although this is original code).
					// So this shows us either R* didn't use same struct for menu BG and highlight, or they just kept fields as x1,y1 etc. Yikes.

					if (m_nOptionHighlightTransitionBlend == 0) {
						if (m_firstStartCounter == 255 && m_nMenuFadeAlpha == 255 && !bMenuChangeOngoing) {
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

					} else if (m_nOptionHighlightTransitionBlend < 255) {
						menuOptionHighlight.Translate(m_nOptionHighlightTransitionBlend);
						if (m_firstStartCounter == 255 && m_nMenuFadeAlpha == 255 && !bMenuChangeOngoing) {
							CSprite2d::Draw2DPolygon(MENU_X_LEFT_ALIGNED(menuOptionHighlight.topLeft_x), MENU_Y(menuOptionHighlight.topLeft_y),
								MENU_X_LEFT_ALIGNED(menuOptionHighlight.topRight_x), MENU_Y(menuOptionHighlight.topRight_y),
								MENU_X_LEFT_ALIGNED(menuOptionHighlight.bottomRight_x), MENU_Y(menuOptionHighlight.bottomRight_y),
								MENU_X_LEFT_ALIGNED(menuOptionHighlight.bottomLeft_x), MENU_Y(menuOptionHighlight.bottomLeft_y), SELECTIONBORDER_COLOR);
						}
					} else {
						m_nOptionHighlightTransitionBlend = 255;
						menuOptionHighlight.Translate(m_nOptionHighlightTransitionBlend);
						if (m_firstStartCounter == 255 && m_nMenuFadeAlpha == 255 && !bMenuChangeOngoing) {
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

				if (section == 1) {
					if (leftText) {
						CFont::PrintString(MENU_X_LEFT_ALIGNED(aScreens[m_nCurrScreen].m_aEntries[i].m_X), MENU_Y(aScreens[m_nCurrScreen].m_aEntries[i].m_Y MINUS_SCROLL_OFFSET), leftText);
					}

					if (rightText) {
						CFont::SetCentreOff();
						CFont::SetRightJustifyOn();
						if (aScreens[m_nCurrScreen].m_aEntries[i].m_SaveSlot >= SAVESLOT_1 && aScreens[m_nCurrScreen].m_aEntries[i].m_SaveSlot <= SAVESLOT_8) {
							CFont::SetFontStyle(FONT_LOCALE(FONT_STANDARD));
							CFont::SetScale(MENU_X(MEDIUMTEXT_X_SCALE), MENU_Y(MEDIUMTEXT_Y_SCALE));
						} else {
							CFont::SetFontStyle(FONT_LOCALE(FONT_HEADING));
							CFont::SetScale(MENU_X(BIGTEXT_X_SCALE), MENU_Y(BIGTEXT_Y_SCALE));
						}
						CFont::PrintString(MENU_X_LEFT_ALIGNED(DEFAULT_SCREEN_WIDTH - RIGHT_ALIGNED_TEXT_RIGHT_MARGIN(xMargin)), MENU_Y(aScreens[m_nCurrScreen].m_aEntries[i].m_Y MINUS_SCROLL_OFFSET), rightText);
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
#ifdef CUSTOM_FRONTEND_OPTIONS
							&& ScreenHasOption(m_nCurrScreen, "FEA_3DH")
#else
							&& m_nCurrScreen == MENUPAGE_SOUND_SETTINGS
#endif
							&& m_nPrefsAudio3DProviderIndex != NO_AUDIO_PROVIDER) {

							m_nPrefsAudio3DProviderIndex = DMAudio.GetCurrent3DProviderIndex();
							SetHelperText(3);
						}
					}
					if (m_nDisplayVideoMode != m_nPrefsVideoMode) {
						if (strcmp(aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption].m_EntryName, "FED_RES") != 0
#ifdef CUSTOM_FRONTEND_OPTIONS
							&& ScreenHasOption(m_nCurrScreen, "FED_RES")) {
#else
							&& m_nCurrScreen == MENUPAGE_DISPLAY_SETTINGS) {
#endif
							m_nDisplayVideoMode = m_nPrefsVideoMode;
							SetHelperText(3);
						}
					}

					// Sliders
					int lastActiveBarX;
					switch (aScreens[m_nCurrScreen].m_aEntries[i].m_Action) {
						case MENUACTION_BRIGHTNESS:
							ProcessSlider(m_PrefsBrightness / 384.0f, 70.0f, HOVEROPTION_INCREASE_BRIGHTNESS, HOVEROPTION_DECREASE_BRIGHTNESS, SCREEN_WIDTH, true);
							break;
						case MENUACTION_DRAWDIST:
							ProcessSlider((m_PrefsLOD - 0.925f) / 0.875f, 99.0f, HOVEROPTION_INCREASE_DRAWDIST, HOVEROPTION_DECREASE_DRAWDIST, SCREEN_WIDTH, true);
							break;
						case MENUACTION_MUSICVOLUME:
							if(m_nPrefsAudio3DProviderIndex != NO_AUDIO_PROVIDER)
								ProcessSlider(m_PrefsMusicVolume / 64.0f, 70.0f, HOVEROPTION_INCREASE_MUSICVOLUME, HOVEROPTION_DECREASE_MUSICVOLUME, SCREEN_WIDTH, true);
							break;
						case MENUACTION_SFXVOLUME:
							if (m_nPrefsAudio3DProviderIndex != NO_AUDIO_PROVIDER)
								ProcessSlider(m_PrefsSfxVolume / 64.0f, 99.0f, HOVEROPTION_INCREASE_SFXVOLUME, HOVEROPTION_DECREASE_SFXVOLUME, SCREEN_WIDTH, true);
							break;
						case MENUACTION_MOUSESENS:
							ProcessSlider(TheCamera.m_fMouseAccelHorzntl * 200.0f, 170.0f, HOVEROPTION_INCREASE_MOUSESENS, HOVEROPTION_DECREASE_MOUSESENS, SCREEN_WIDTH, false);
							break;
						case MENUACTION_MP3VOLUMEBOOST:
							if(m_nPrefsAudio3DProviderIndex != NO_AUDIO_PROVIDER && DMAudio.IsMP3RadioChannelAvailable())
								ProcessSlider(m_PrefsMP3BoostVolume / 64.f, 128.0f, HOVEROPTION_INCREASE_MP3BOOST, HOVEROPTION_DECREASE_MP3BOOST, SCREEN_WIDTH, true);
							break;
					}
					
					// Not just unused, but also collides with the bug fix in Font.cpp. Yikes.
#ifndef FIX_BUGS
					nextYToUse += MENU_DEFAULT_LINE_HEIGHT * CFont::GetNumberLines(MENU_X_LEFT_ALIGNED(60.0f), MENU_Y(nextYToUse), leftText);
#endif

					if (aScreens[m_nCurrScreen].m_aEntries[i].m_Action == MENUACTION_RADIO) {
						nextYToUse += MENURADIO_SELECTOR_HEIGHT + 5.f; // unused
					}
				}
			}
		}
		section++;
	}

#ifdef SCROLLABLE_PAGES
	#define SCROLLBAR_BOTTOM_Y 105.0f // only for background, scrollbar's itself is calculated
	#define SCROLLBAR_RIGHT_X 26.0f
	#define SCROLLBAR_WIDTH 9.5f
	#define SCROLLBAR_TOP_Y 84

	if (activeScreen && SCREEN_HAS_AUTO_SCROLLBAR) {
		// Scrollbar background
		CSprite2d::DrawRect(CRect(MENU_X_RIGHT_ALIGNED(SCROLLBAR_RIGHT_X - 2), MENU_Y(SCROLLBAR_TOP_Y),
			MENU_X_RIGHT_ALIGNED(SCROLLBAR_RIGHT_X - 2 - SCROLLBAR_WIDTH), SCREEN_SCALE_FROM_BOTTOM(SCROLLBAR_BOTTOM_Y)), CRGBA(30, 30, 30, FadeIn(150)));
		
		float scrollbarHeight = SCROLLBAR_MAX_HEIGHT / (m_nTotalListRow / (float) MAX_VISIBLE_OPTION);
		float scrollbarBottom, scrollbarTop;

		scrollbarBottom = MENU_Y(SCROLLBAR_TOP_Y - 6 + m_nScrollbarTopMargin + scrollbarHeight);
		scrollbarTop = MENU_Y(SCROLLBAR_TOP_Y + 2 + m_nScrollbarTopMargin);
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
		case MENUPAGE_STATS:
		case MENUPAGE_CONTROLLER_PC:
		case MENUPAGE_SOUND_SETTINGS:
		case MENUPAGE_DISPLAY_SETTINGS:
		case MENUPAGE_MOUSE_CONTROLS:
			DisplayHelperText(nil);
			break;
		case MENUPAGE_OPTIONS:
			if (m_nPrefsAudio3DProviderIndex == NO_AUDIO_PROVIDER && aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption].m_Action == MENUACTION_LOADRADIO)
				DisplayHelperText("FEA_NAH");
			break;
#ifdef CUSTOM_FRONTEND_OPTIONS
		default:
			if (aScreens[m_nCurrScreen].layout) {
				if (aScreens[m_nCurrScreen].layout->showLeftRightHelper) {
					DisplayHelperText(nil);
				}
			}
			break;
#endif
	}

	if (m_nCurrScreen == MENUPAGE_DELETING_IN_PROGRESS) {
		SmallMessageScreen("FEDL_WR");
	}
#ifndef XBOX_MESSAGE_SCREEN
	else if (m_nCurrScreen == MENUPAGE_SAVING_IN_PROGRESS) {
		SmallMessageScreen("FESZ_WR");
	}
#endif
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
					number = 27;
					break;
				case CONTROL_CLASSIC:
					number = 32;
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

	for (int optionIdx = 0; optionIdx < numOptions; nextY = MENU_Y(++optionIdx * rowHeight + yStart)) {
		int nextX = xStart;
		int bindingsForThisOpt = 0;
		int contSetOrder = SETORDER_1;
		CFont::SetColor(CRGBA(255, 255, 255, FadeIn(255)));

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
				case 20:
				case 21:
				case 22:
				case 23:
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
				case 18:
					controllerAction = PED_DUCK;
					break;
				case 19:
					controllerAction = PED_ANSWER_PHONE;
					break;
				case 24:
					controllerAction = PED_LOOKBEHIND;
					break;
				case 25:
					if (m_ControlMethod == CONTROL_STANDARD)
						controllerAction = -1;
					else
						controllerAction = PED_1RST_PERSON_LOOK_LEFT;
					break;
				case 26:
					if (m_ControlMethod == CONTROL_STANDARD)
						controllerAction = -1;
					else
						controllerAction = PED_1RST_PERSON_LOOK_RIGHT;
					break;
				case 27:
					controllerAction = PED_1RST_PERSON_LOOK_UP;
					break;
				case 28:
					controllerAction = PED_1RST_PERSON_LOOK_DOWN;
					break;
				case 29:
					controllerAction = PED_CYCLE_TARGET_LEFT;
					break;
				case 30:
					controllerAction = PED_CYCLE_TARGET_RIGHT;
					break;
				case 31:
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
				case 18:
				case 19:
				case 27:
				case 28:
				case 29:
				case 30:
				case 31:
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
				case 20:
					controllerAction = VEHICLE_TURRETLEFT;
					break;
				case 21:
					controllerAction = VEHICLE_TURRETRIGHT;
					break;
				case 22:
					controllerAction = VEHICLE_TURRETUP;
					break;
				case 23:
					controllerAction = VEHICLE_TURRETDOWN;
					break;
				case 24:
					controllerAction = -2;
					break;
				case 25:
					controllerAction = VEHICLE_LOOKLEFT;
					break;
				case 26:
					controllerAction = VEHICLE_LOOKRIGHT;
					break;
				default:
					break;
			}
		}

		// Highlight selected column(and make its text black)
		if (m_nSelectedListRow == optionIdx) {
			int bgY = m_nSelectedListRow * rowHeight + yStart + 1.0f;
			if (m_nCurrExLayer == HOVEROPTION_LIST) {

				if (column == CONTSETUP_PED_COLUMN && m_nSelectedContSetupColumn == CONTSETUP_PED_COLUMN) {
#ifdef FIX_BUGS
					CSprite2d::DrawRect(CRect(nextX, MENU_Y(bgY), nextX + MENU_X(CONTSETUP_BOUND_COLUMN_WIDTH),
						MENU_Y(bgY + CONTSETUP_BOUND_HIGHLIGHT_HEIGHT)), CRGBA(SELECTIONBORDER_COLOR.r, SELECTIONBORDER_COLOR.g, SELECTIONBORDER_COLOR.b, FadeIn(255)));
#else
					CSprite2d::DrawRect(CRect(MENU_X_LEFT_ALIGNED(210.0f), MENU_Y(bgY),
						MENU_X_LEFT_ALIGNED(400.0f), MENU_Y(bgY + CONTSETUP_BOUND_HIGHLIGHT_HEIGHT)),
						CRGBA(SELECTIONBORDER_COLOR.r, SELECTIONBORDER_COLOR.g, SELECTIONBORDER_COLOR.b, FadeIn(255)));
#endif
					CFont::SetColor(CRGBA(255, 255, 255, FadeIn(255)));

				} else if (column == CONTSETUP_VEHICLE_COLUMN && m_nSelectedContSetupColumn == CONTSETUP_VEHICLE_COLUMN) {
#ifdef FIX_BUGS
					CSprite2d::DrawRect(CRect(nextX, MENU_Y(bgY), nextX + MENU_X(CONTSETUP_BOUND_COLUMN_WIDTH),
						MENU_Y(bgY + CONTSETUP_BOUND_HIGHLIGHT_HEIGHT)), CRGBA(SELECTIONBORDER_COLOR.r, SELECTIONBORDER_COLOR.g, SELECTIONBORDER_COLOR.b, FadeIn(255)));
#else
					CSprite2d::DrawRect(CRect(MENU_X_LEFT_ALIGNED(410.0f), MENU_Y(bgY), MENU_X_LEFT_ALIGNED(600.0f), MENU_Y(bgY + CONTSETUP_BOUND_HIGHLIGHT_HEIGHT)),
						CRGBA(SELECTIONBORDER_COLOR.r, SELECTIONBORDER_COLOR.g, SELECTIONBORDER_COLOR.b, FadeIn(255)));
#endif
					CFont::SetColor(CRGBA(255, 255, 255, FadeIn(255)));
				}
			}
		}

		// Print bindings, including seperator (-) between them
		CFont::SetScale(MENU_X(0.25f), MENU_Y(LISTITEM_Y_SCALE));
		for (; contSetOrder < MAX_SETORDERS && controllerAction != -1; contSetOrder++) {
			wchar *settingText = ControlsManager.GetControllerSettingTextWithOrderNumber((e_ControllerAction)controllerAction, (eContSetOrder)contSetOrder);
			if (settingText) {
				++bindingsForThisOpt;
				if (bindingsForThisOpt > 1) {
					wchar *seperator = TheText.Get("FEC_IBT");
					CFont::SetColor(CRGBA(255, 255, 255, FadeIn(255)));
					CFont::PrintString(nextX, nextY, seperator);
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
			CFont::SetColor(CRGBA(0, 0, 0, FadeIn(255)));
			CFont::PrintString(nextX, nextY, TheText.Get("FEC_NUS")); // not used

		} else if (controllerAction == -2) {
			CFont::SetColor(CRGBA(0, 0, 0, FadeIn(255)));
			CFont::PrintString(nextX, nextY, TheText.Get("FEC_CMP")); // combo: l+r

		} else if (bindingsForThisOpt == 0) {
			m_NoEmptyBinding = false;
			if (m_nSelectedListRow != optionIdx) {
				CFont::SetColor(CRGBA(0, 0, 0, FadeIn(255)));
				CFont::PrintString(nextX, nextY, TheText.Get("FEC_UNB")); // unbound

			} else if (m_bWaitingForNewKeyBind) {
				if (column != m_nSelectedContSetupColumn) {
					CFont::SetColor(CRGBA(0, 0, 0, FadeIn(255)));
					CFont::PrintString(nextX, nextY, TheText.Get("FEC_UNB")); // unbound
				}
			} else {
				if (column != m_nSelectedContSetupColumn) {
					CFont::SetColor(CRGBA(0, 0, 0, FadeIn(255)));
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
						CFont::SetColor(CRGBA(255, 255, 255, FadeIn(255)));
						CFont::PrintString(nextX, nextY, TheText.Get("FEC_QUE")); // "???"
					}
					if (m_bKeyChangeNotProcessed)
						DisplayHelperText("FET_CIG");
					else
						DisplayHelperText("FET_RIG");

					SET_FONT_FOR_LIST_ITEM

					m_bKeyIsOK = true;
				} else {
					DisplayHelperText("FET_CIG");
					SET_FONT_FOR_LIST_ITEM

					m_bKeyIsOK = false;
					m_bKeyChangeNotProcessed = false;
				}
			} else if (optionIdx == m_nSelectedListRow) {
				DisplayHelperText("FET_EIG");
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
			static PauseModeTime lastStateChange = 0;
			if (CTimer::GetTimeInMillisecondsPauseMode() - lastStateChange > 150) {
				waitingTextVisible = !waitingTextVisible;
				lastStateChange = CTimer::GetTimeInMillisecondsPauseMode();
			}
			if (waitingTextVisible) {
				CFont::SetColor(CRGBA(255, 255, 0, FadeIn(255)));
				CFont::PrintString(nextX, MENU_Y(yStart), TheText.Get("FEC_QUE"));
				CFont::SetColor(CRGBA(LABEL_COLOR.r, LABEL_COLOR.g, LABEL_COLOR.b, FadeIn(255)));
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

	// Shadow
	CFont::SetColor(CRGBA(30, 30, 30, FadeIn(255)));

	if (m_ControlMethod == CONTROL_STANDARD)
		CFont::PrintString(SCREEN_STRETCH_FROM_RIGHT(MENUHEADER_POS_X) - MENU_X(7.f), SCREEN_SCALE_Y(MENUHEADER_POS_Y + 7.f), TheText.Get("FET_STI"));
	else if (m_ControlMethod == CONTROL_CLASSIC)
		CFont::PrintString(SCREEN_STRETCH_FROM_RIGHT(MENUHEADER_POS_X) - MENU_X(7.f), SCREEN_SCALE_Y(MENUHEADER_POS_Y + 7.f), TheText.Get("FET_CTI"));

	// Real header
	CFont::SetColor(CRGBA(HEADER_COLOR.r, HEADER_COLOR.g, HEADER_COLOR.b, FadeIn(255)));

	if (m_ControlMethod == CONTROL_STANDARD)
		CFont::PrintString(SCREEN_STRETCH_FROM_RIGHT(MENUHEADER_POS_X), SCREEN_SCALE_Y(MENUHEADER_POS_Y), TheText.Get("FET_STI"));
	else if (m_ControlMethod == CONTROL_CLASSIC)
		CFont::PrintString(SCREEN_STRETCH_FROM_RIGHT(MENUHEADER_POS_X), SCREEN_SCALE_Y(MENUHEADER_POS_Y), TheText.Get("FET_CTI"));

	wchar *actionTexts[33];
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
	actionTexts[18] = TheText.Get("FEC_CRO");
	actionTexts[19] = TheText.Get("FEC_ANS");
	if (m_ControlMethod == CONTROL_CLASSIC) {
		actionTexts[20] = TheText.Get("FEC_TFL");
		actionTexts[21] = TheText.Get("FEC_TFR");
		actionTexts[22] = TheText.Get("FEC_TFU");
		actionTexts[23] = TheText.Get("FEC_TFD");
		actionTexts[24] = TheText.Get("FEC_LBA");
		actionTexts[25] = TheText.Get("FEC_LOL");
		actionTexts[26] = TheText.Get("FEC_LOR");
		actionTexts[27] = TheText.Get("FEC_LUD");
		actionTexts[28] = TheText.Get("FEC_LDU");
		actionTexts[29] = TheText.Get("FEC_NTR");
		actionTexts[30] = TheText.Get("FEC_PTT");
		actionTexts[31] = TheText.Get("FEC_CEN");
		actionTexts[32] = nil;
	} else {
		actionTexts[20] = TheText.Get("FEC_TFL");
		actionTexts[21] = TheText.Get("FEC_TFR");
		actionTexts[22] = TheText.Get("FEC_TFU");
		actionTexts[23] = TheText.Get("FEC_TFD");
		actionTexts[24] = TheText.Get("FEC_LBA");
		actionTexts[25] = TheText.Get("FEC_LOL");
		actionTexts[26] = TheText.Get("FEC_LOR");
		actionTexts[27] = nil;
	}

	// Blue panel background
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
	CFont::SetDropShadowPosition(2);
	CFont::SetDropColor(CRGBA(0, 0, 0, FadeIn(255)));
	CFont::PrintString(MENU_X_LEFT_ALIGNED(CONTSETUP_COLUMN_2_X), MENU_Y(CONTSETUP_LIST_TOP), TheText.Get("FET_CFT"));
	CFont::PrintString(MENU_X_LEFT_ALIGNED(CONTSETUP_COLUMN_3_X), MENU_Y(CONTSETUP_LIST_TOP), TheText.Get("FET_CCR"));
	CFont::SetDropShadowPosition(0);
	SET_FONT_FOR_LIST_ITEM

	int yStart;
	if (m_ControlMethod == CONTROL_CLASSIC)
		yStart = CONTSETUP_LIST_TOP + 18;
	else
		yStart = CONTSETUP_LIST_TOP + 21;

	float optionYBottom = yStart + rowHeight;
	for (int i = 0; i < ARRAY_SIZE(actionTexts); ++i) {
		wchar *actionText = actionTexts[i];
		if (!actionText)
			break;

		if (!m_bWaitingForNewKeyBind) {
			if (m_nMousePosX > MENU_X_LEFT_ALIGNED(CONTSETUP_LIST_LEFT - 10.0f) &&
				m_nMousePosX < MENU_X_LEFT_ALIGNED(CONTSETUP_COLUMN_3_X + CONTSETUP_BOUND_COLUMN_WIDTH)) {

				if (m_nMousePosY > MENU_Y(i * rowHeight + yStart) && m_nMousePosY < MENU_Y(i * rowHeight + optionYBottom)) {
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
								m_nSelectedContSetupColumn = CONTSETUP_PED_COLUMN;
#ifdef FIX_BUGS
							} else if (m_nMousePosX > MENU_X_LEFT_ALIGNED(CONTSETUP_COLUMN_2_X + CONTSETUP_BOUND_COLUMN_WIDTH) && m_nMousePosX < SCREEN_WIDTH) {
#else
							} else if (m_nMousePosX > MENU_X_LEFT_ALIGNED(370.0f) && m_nMousePosX < SCREEN_WIDTH) {
#endif
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
		}
		if (m_nSelectedListRow != i)
			CFont::SetColor(CRGBA(MENUOPTION_COLOR.r, MENUOPTION_COLOR.g, MENUOPTION_COLOR.b, FadeIn(255)));
		else if (m_nCurrExLayer == HOVEROPTION_LIST)
			CFont::SetColor(CRGBA(SELECTEDMENUOPTION_COLOR.r, SELECTEDMENUOPTION_COLOR.g, SELECTEDMENUOPTION_COLOR.b, FadeIn(255)));

		CFont::SetRightJustifyOff();
		if (m_PrefsLanguage == LANGUAGE_GERMAN && (i == 20 || i == 21 || i == 22 || i == 23))
			CFont::SetScale(MENU_X(0.32f), MENU_Y(LISTITEM_Y_SCALE));
		else
			CFont::SetScale(MENU_X(LISTITEM_X_SCALE), MENU_Y(LISTITEM_Y_SCALE));

		CFont::PrintString(MENU_X_LEFT_ALIGNED(CONTSETUP_COLUMN_1_X), MENU_Y(i * rowHeight + yStart), actionText);
	}
	DrawControllerBound(yStart, MENU_X_LEFT_ALIGNED(CONTSETUP_COLUMN_2_X), rowHeight, CONTSETUP_PED_COLUMN);
	DrawControllerBound(yStart, MENU_X_LEFT_ALIGNED(CONTSETUP_COLUMN_3_X), rowHeight, CONTSETUP_VEHICLE_COLUMN);

	if (!m_bWaitingForNewKeyBind) {
		CFont::SetScale(MENU_X(BIGTEXT_X_SCALE), MENU_Y(BIGTEXT_Y_SCALE));

		if ((m_nMousePosX > MENU_X_RIGHT_ALIGNED(CONTSETUP_BACK_RIGHT) - CFont::GetStringWidth(TheText.Get("FEDS_TB"), true)
			&& m_nMousePosX < MENU_X_RIGHT_ALIGNED(CONTSETUP_BACK_RIGHT) && m_nMousePosY > SCREEN_SCALE_FROM_BOTTOM(CONTSETUP_BACK_BOTTOM)
			&& m_nMousePosY < SCREEN_SCALE_FROM_BOTTOM(CONTSETUP_BACK_BOTTOM - CONTSETUP_BACK_HEIGHT)) || m_nCurrExLayer == HOVEROPTION_BACK) {
			m_nHoverOption = HOVEROPTION_BACK;

		} else if (m_nMousePosX > MENU_X_LEFT_ALIGNED(CONTSETUP_LIST_LEFT - 10.0f) && m_nMousePosX < MENU_X_LEFT_ALIGNED(CONTSETUP_COLUMN_3_X + CONTSETUP_BOUND_COLUMN_WIDTH)
			&& m_nMousePosY > MENU_Y(CONTSETUP_LIST_TOP - 10.0f) && m_nMousePosY < SCREEN_SCALE_FROM_BOTTOM(CONTSETUP_LIST_BOTTOM)) {
			m_nHoverOption = HOVEROPTION_LIST;

		} else {
			m_nHoverOption = HOVEROPTION_NOT_HOVERING;
		}
	}

	// Back button and it's shadow
	CFont::SetFontStyle(FONT_LOCALE(FONT_HEADING));
	CFont::SetScale(MENU_X(BIGTEXT_X_SCALE), MENU_Y(BIGTEXT_Y_SCALE));
	CFont::SetRightJustifyOn();
	CFont::SetDropShadowPosition(2);
	CFont::SetDropColor(CRGBA(0, 0, 0, FadeIn(255)));
	CFont::SetColor(CRGBA(MENUOPTION_COLOR.r, MENUOPTION_COLOR.g, MENUOPTION_COLOR.b, FadeIn(255)));
	CFont::PrintString(MENU_X_RIGHT_ALIGNED(CONTSETUP_BACK_RIGHT - 2.0f), SCREEN_SCALE_FROM_BOTTOM(CONTSETUP_BACK_BOTTOM - 4.0f), TheText.Get("FEDS_TB"));
}

void
CMenuManager::DrawFrontEnd()
{
	CFont::SetAlphaFade(255.0f);
	CSprite2d::InitPerFrame();
	CFont::InitPerFrame();
	SetFrontEndRenderStates();
	m_NoEmptyBinding = true;

	if (m_nCurrScreen == MENUPAGE_NONE) {
		if (m_bGameNotLoaded) {
			m_nCurrScreen = MENUPAGE_START_MENU;
		} else {
			m_nCurrScreen = MENUPAGE_PAUSE_MENU;
		}
		SETUP_SCROLLING(m_nCurrScreen)
	}

	if (m_nCurrOption == 0 && aScreens[m_nCurrScreen].m_aEntries[0].m_Action == MENUACTION_LABEL)
		m_nCurrOption = 1;

	if (m_firstStartCounter == 255 && m_nMenuFadeAlpha == 255)
		bMenuChangeOngoing = false;

	DrawBackground(false);
}

void
CMenuManager::DrawBackground(bool transitionCall)
{
	if (!m_bSpritesLoaded)
		return;

	SetFrontEndRenderStates();

	if (m_firstStartCounter < 255) {
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
			m_firstStartCounter = 255;
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
			case MENUPAGE_DISPLAY_SETTINGS:
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
#ifdef CUSTOM_FRONTEND_OPTIONS
				if (aScreens[m_nCurrScreen].layout && aScreens[m_nCurrScreen].layout->noInvasiveBorders) {
					// Taken from the case above
					menuBg.topLeft_x = 26.0f;
					menuBg.topLeft_y = 59.0f;
					menuBg.topRight_x = 629.0f;
					menuBg.topRight_y = 29.0f;
					menuBg.bottomLeft_x = 15.0f;
					menuBg.bottomLeft_y = 426.0f;
					menuBg.bottomRight_x = 610.0f;
					menuBg.bottomRight_y = 398.0f;
					break;
				}
#endif
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
		if (m_firstStartCounter == 255)
			m_nOptionHighlightTransitionBlend = 0;
	}

	static PauseModeTime LastFade = 0;

	if (m_nMenuFadeAlpha < 255) {
		static uint8 forceFadeInCounter = 0;	
		if (CTimer::GetTimeInMillisecondsPauseMode() - LastFade > 30
#ifndef FIX_HIGH_FPS_BUGS_ON_FRONTEND // Dirty dirty hack
			|| forceFadeInCounter > 30
#endif
			) {
			m_nMenuFadeAlpha += 20;
			if (m_firstStartCounter < 255) {
				m_firstStartCounter = Min(m_firstStartCounter + 20, 255);
			}
			LastFade = CTimer::GetTimeInMillisecondsPauseMode();
		}
		forceFadeInCounter++;
	} else if (m_nMenuFadeAlpha > 255)
		m_nMenuFadeAlpha = 255;

	if (!transitionCall && m_firstStartCounter == 255) {
		int actualAlpha = m_nMenuFadeAlpha;
		if (actualAlpha < 255) {
			int actualScreen = m_nCurrScreen;
			SetFrontEndRenderStates();
			m_nCurrScreen = m_nPrevScreen;
			m_nMenuFadeAlpha = 255 - m_nMenuFadeAlpha;
			switch (m_nCurrScreen) {
				case MENUPAGE_SKIN_SELECT:
					DrawPlayerSetupScreen(false);
					break;
				case MENUPAGE_KEYBOARD_CONTROLS:
					DrawControllerSetupScreen();
					break;
				case MENUPAGE_OUTRO:
					DrawQuitGameScreen();
					break;
				default:
					DrawStandardMenus(false);
					break;
			}
			m_nCurrScreen = actualScreen;
			m_nMenuFadeAlpha = actualAlpha;
		}
	}

	switch (m_nCurrScreen) {
		case MENUPAGE_SKIN_SELECT:
			DrawPlayerSetupScreen(true);
			break;
		case MENUPAGE_KEYBOARD_CONTROLS:
			DrawControllerSetupScreen();
			break;
		case MENUPAGE_OUTRO:
			DrawQuitGameScreen();
			break;
		default:
			DrawStandardMenus(true);
			break;
	}

	CFont::DrawFonts();
	SetFrontEndRenderStates();

	if (m_nCurrScreen != MENUPAGE_OUTRO)
		if (m_firstStartCounter == 255) {
			m_aFrontEndSprites[MENUSPRITE_VCLOGO].Draw(CRect(SCREEN_STRETCH_X(27.0f), MENU_Y(8.0f), SCREEN_STRETCH_X(27.0f) + MENU_X(130.f), MENU_Y(138.0f)), CRGBA(255, 255, 255, 255));
		} else {
			m_aFrontEndSprites[MENUSPRITE_VCLOGO].Draw(CRect(SCREEN_STRETCH_X(27.0f), MENU_Y(8.0f), SCREEN_STRETCH_X(27.0f) + MENU_X(130.f), MENU_Y(138.0f)), CRGBA(255, 255, 255, FadeIn(255)));
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
		CRect mouse(0.0f, 0.0f, MENU_X(35.0f), MENU_Y(35.0f));
		CRect shad(MENU_X(10.0f), MENU_Y(3.0f), MENU_X(45.0f), MENU_Y(38.0f));

		mouse.Translate(m_nMousePosX, m_nMousePosY);
		shad.Translate(m_nMousePosX, m_nMousePosY);
		m_aFrontEndSprites[MENUSPRITE_MOUSE].Draw(shad, CRGBA(100, 100, 100, 50));
		m_aFrontEndSprites[MENUSPRITE_MOUSE].Draw(mouse, CRGBA(255, 255, 255, 255));
	}
}

void
CMenuManager::DrawPlayerSetupScreen(bool activeScreen)
{
	RESET_FONT_FOR_NEW_PAGE

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
		MENU_X_RIGHT_ALIGNED(PLAYERSETUP_LIST_RIGHT), SCREEN_SCALE_FROM_BOTTOM(PLAYERSETUP_LIST_BOTTOM)), CRGBA(LIST_BACKGROUND_COLOR.r, LIST_BACKGROUND_COLOR.g, LIST_BACKGROUND_COLOR.b, FadeIn(LIST_BACKGROUND_COLOR.a)));

	SET_FONT_FOR_MENU_HEADER
	CFont::SetColor(CRGBA(30, 30, 30, FadeIn(255)));
	CFont::PrintString(SCREEN_STRETCH_FROM_RIGHT(MENUHEADER_POS_X) - MENU_X(7.f), SCREEN_SCALE_Y(MENUHEADER_POS_Y + 7.f), TheText.Get("FET_PS"));

	CFont::SetColor(CRGBA(HEADER_COLOR.r, HEADER_COLOR.g, HEADER_COLOR.b, FadeIn(255)));
	CFont::PrintString(SCREEN_STRETCH_FROM_RIGHT(MENUHEADER_POS_X), SCREEN_SCALE_Y(MENUHEADER_POS_Y), TheText.Get("FET_PS"));

	// Header (Skin - Date)
	if (m_nCurrExLayer == HOVEROPTION_LIST) {
		CFont::SetColor(CRGBA(SELECTEDMENUOPTION_COLOR.r, SELECTEDMENUOPTION_COLOR.g, SELECTEDMENUOPTION_COLOR.b, FadeIn(255)));
	} else {
		CFont::SetColor(CRGBA(MENUOPTION_COLOR.r, MENUOPTION_COLOR.g, MENUOPTION_COLOR.b, FadeIn(255)));
	}
	CFont::SetDropShadowPosition(2);
	CFont::SetDropColor(CRGBA(0, 0, 0, 255));
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
	CFont::SetDropShadowPosition(0);

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
								strcpy(m_PrefsSkinFile, m_aSkinName);
								CWorld::Players[0].SetPlayerSkin(m_PrefsSkinFile);
								SaveSettings();
							}
						} else {
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
		// Scrollbar background - it's unchanged since III and still yellowish...
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
		if (activeScreen)
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
			CFont::SetColor(CRGBA(MENUOPTION_COLOR.r, MENUOPTION_COLOR.g, MENUOPTION_COLOR.b, FadeIn(120)));
			CFont::SetRightJustifyOff();
			CFont::PrintString(MENU_X_LEFT_ALIGNED(24.0f), MENU_Y(220.0f), TheText.Get("FET_APP"));
		}
		CFont::SetFontStyle(FONT_LOCALE(FONT_HEADING));

		CFont::SetScale(MENU_X(BIGTEXT_X_SCALE), MENU_Y(BIGTEXT_Y_SCALE));

		if ((m_nMousePosX > MENU_X_RIGHT_ALIGNED(PLAYERSETUP_LIST_RIGHT - 1) - CFont::GetStringWidth(TheText.Get("FEDS_TB"), true)
			&& m_nMousePosX < MENU_X_RIGHT_ALIGNED(PLAYERSETUP_LIST_RIGHT - 1)
			&& m_nMousePosY > SCREEN_SCALE_FROM_BOTTOM(PLAYERSETUP_LIST_BOTTOM - 3)
			&& m_nMousePosY < SCREEN_SCALE_FROM_BOTTOM(PLAYERSETUP_LIST_BOTTOM - 26))
			|| m_nCurrExLayer == HOVEROPTION_BACK) {

			m_nHoverOption = HOVEROPTION_BACK;
		} else if ((strcmp(m_aSkinName, m_PrefsSkinFile) != 0
			&& m_nMousePosX > MENU_X_LEFT_ALIGNED(PLAYERSETUP_LIST_LEFT)
			&& m_nMousePosX < MENU_X_LEFT_ALIGNED(PLAYERSETUP_LIST_LEFT) + CFont::GetStringWidth(TheText.Get("FES_SET"), true)
			&& m_nMousePosY > SCREEN_SCALE_FROM_BOTTOM(PLAYERSETUP_LIST_BOTTOM - 3)
			&& m_nMousePosY < SCREEN_SCALE_FROM_BOTTOM(PLAYERSETUP_LIST_BOTTOM - 26))
			|| m_nCurrExLayer == HOVEROPTION_USESKIN) {

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
	CFont::SetScale(MENU_X(BIGTEXT_X_SCALE), MENU_Y(BIGTEXT_Y_SCALE));
	CFont::SetRightJustifyOn();
	CFont::SetColor(CRGBA(MENUOPTION_COLOR.r, MENUOPTION_COLOR.g, MENUOPTION_COLOR.b, FadeIn(255)));
	CFont::SetDropShadowPosition(2);
	CFont::SetDropColor(CRGBA(0, 0, 0, FadeIn(255)));

	// Back button
	CFont::PrintString(MENU_X_RIGHT_ALIGNED(PLAYERSETUP_LIST_RIGHT - 3), SCREEN_SCALE_FROM_BOTTOM(PLAYERSETUP_LIST_BOTTOM - 5), TheText.Get("FEDS_TB"));
	CFont::SetRightJustifyOff();

	if (!strcmp(m_aSkinName, m_PrefsSkinFile)) {
		CFont::SetColor(CRGBA(DARKMENUOPTION_COLOR.r, DARKMENUOPTION_COLOR.g, DARKMENUOPTION_COLOR.b, FadeIn(255)));
	} else {
		CFont::SetColor(CRGBA(MENUOPTION_COLOR.r, MENUOPTION_COLOR.g, MENUOPTION_COLOR.b, FadeIn(255)));
	}

	// Use skin button
	CFont::PrintString(MENU_X_LEFT_ALIGNED(PLAYERSETUP_LIST_LEFT), SCREEN_SCALE_FROM_BOTTOM(PLAYERSETUP_LIST_BOTTOM - 5), TheText.Get("FES_SET"));
	CFont::SetDropShadowPosition(0);
}

int
CMenuManager::FadeIn(int alpha)
{
	return Min(m_nMenuFadeAlpha, alpha);
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

	// First icon is hidden behind arrow
	m_LeftMostRadioX = MENU_X_LEFT_ALIGNED(MENURADIO_ICON_FIRST_X - MENURADIO_ICON_SIZE);
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
	int fileHandle = CFileMgr::OpenFile("gta_vc.set", "r");

	int32 prevLang = m_PrefsLanguage;
	MousePointerStateHelper.bInvertVertically = true;
	CMBlur::BlurOn = false;

	// 50 is silly
	char headerText[50];
	int someVersion = 0;
	bool fileIsValid = true;

	if (fileHandle) {
		CFileMgr::Read(fileHandle, headerText, 29);

		if (strncmp(headerText, TopLineEmptyFile, sizeof(TopLineEmptyFile) - 1) == 0) {
			fileIsValid = false;
		} else {
			CFileMgr::Seek(fileHandle, 0, 0);
			CFileMgr::Read(fileHandle, (char*)&someVersion, sizeof(someVersion));
		}
		if (fileIsValid && someVersion >= 3) {
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
#ifdef LEGACY_MENU_OPTIONS
			CFileMgr::Read(fileHandle, (char*)&m_PrefsVsyncDisp, 1);
			CFileMgr::Read(fileHandle, (char*)&CMBlur::BlurOn, 1);
#else
			CFileMgr::Read(fileHandle, gString, 1);
			CFileMgr::Read(fileHandle, gString, 1);
#endif
			CFileMgr::Read(fileHandle, (char*)&TheCamera.m_bHeadBob, 1);
			CFileMgr::Read(fileHandle, (char*)&TheCamera.m_fMouseAccelHorzntl, 4);
			CFileMgr::Read(fileHandle, (char*)&MousePointerStateHelper.bInvertVertically, 1);
			CFileMgr::Read(fileHandle, (char*)&CVehicle::m_bDisableMouseSteering, 1);
			CFileMgr::Read(fileHandle, (char*)&m_PrefsSfxVolume, 1);
			CFileMgr::Read(fileHandle, (char*)&m_PrefsMusicVolume, 1);
			CFileMgr::Read(fileHandle, (char*)&m_PrefsMP3BoostVolume, 1);
			CFileMgr::Read(fileHandle, (char*)&m_PrefsRadioStation, 1);
			CFileMgr::Read(fileHandle, (char*)&m_PrefsSpeakers, 1);
			CFileMgr::Read(fileHandle, (char*)&m_nPrefsAudio3DProviderIndex, 1);
			CFileMgr::Read(fileHandle, (char*)&m_PrefsDMA, 1);
			CFileMgr::Read(fileHandle, (char*)&m_PrefsBrightness, 2);
			CFileMgr::Read(fileHandle, (char*)&m_PrefsLOD, sizeof(m_PrefsLOD));
			CFileMgr::Read(fileHandle, (char*)&m_PrefsShowSubtitles, 1);
			CFileMgr::Read(fileHandle, (char*)&m_PrefsUseWideScreen, 1);
			CFileMgr::Read(fileHandle, (char*)&m_PrefsFrameLimiter, 1);
			CFileMgr::Read(fileHandle, (char*)&m_nDisplayVideoMode, 1);
			CFileMgr::Read(fileHandle, m_PrefsSkinFile, 256);
			CFileMgr::Read(fileHandle, (char*)&m_ControlMethod, 1);
			CFileMgr::Read(fileHandle, (char*)&m_PrefsLanguage, 1);
			CFileMgr::Read(fileHandle, (char*)&m_PrefsShowHud, 1);
			CFileMgr::Read(fileHandle, (char*)&m_PrefsRadarMode, 1);
			CFileMgr::Read(fileHandle, (char*)&m_PrefsShowLegends, 1);
		}
	}

	CFileMgr::CloseFile(fileHandle);
	CFileMgr::SetDir("");

#ifdef FIX_BUGS
	TheCamera.m_fMouseAccelVertical = TheCamera.m_fMouseAccelHorzntl + 0.0005f;
#endif
#ifdef PC_PLAYER_CONTROLS
	CCamera::m_bUseMouse3rdPerson = m_ControlMethod == CONTROL_STANDARD;
#endif
#ifdef LEGACY_MENU_OPTIONS
	m_PrefsVsync = m_PrefsVsyncDisp;
#endif
	CRenderer::ms_lodDistScale = m_PrefsLOD;

	if (m_nPrefsAudio3DProviderIndex == NO_AUDIO_PROVIDER)
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
	
#ifdef LOAD_INI_SETTINGS
	LoadINISettings(); // needs frontend options to be loaded
#endif
}

void
CMenuManager::SaveSettings()
{
	static char RubbishString[48] = "stuffmorestuffevenmorestuff                 etc";
	static int SomeVersion = 3;

	CFileMgr::SetDirMyDocuments();

	int fileHandle = CFileMgr::OpenFile("gta_vc.set", "w+");
	if (fileHandle) {
		CFileMgr::Write(fileHandle, (char*)&SomeVersion, sizeof(SomeVersion));
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
#ifdef LEGACY_MENU_OPTIONS
		CFileMgr::Write(fileHandle, (char*)&m_PrefsVsyncDisp, 1);
		CFileMgr::Write(fileHandle, (char*)&CMBlur::BlurOn, 1);
#else
		CFileMgr::Write(fileHandle, RubbishString, 1);
		CFileMgr::Write(fileHandle, RubbishString, 1);
#endif
		CFileMgr::Write(fileHandle, (char*)&TheCamera.m_bHeadBob, 1);
		CFileMgr::Write(fileHandle, (char*)&TheCamera.m_fMouseAccelHorzntl, 4);
		CFileMgr::Write(fileHandle, (char*)&MousePointerStateHelper.bInvertVertically, 1);
		CFileMgr::Write(fileHandle, (char*)&CVehicle::m_bDisableMouseSteering, 1);
		CFileMgr::Write(fileHandle, (char*)&m_PrefsSfxVolume, 1);
		CFileMgr::Write(fileHandle, (char*)&m_PrefsMusicVolume, 1);
		CFileMgr::Write(fileHandle, (char*)&m_PrefsMP3BoostVolume, 1);
		CFileMgr::Write(fileHandle, (char*)&m_PrefsRadioStation, 1);
		CFileMgr::Write(fileHandle, (char*)&m_PrefsSpeakers, 1);
		CFileMgr::Write(fileHandle, (char*)&m_nPrefsAudio3DProviderIndex, 1);
		CFileMgr::Write(fileHandle, (char*)&m_PrefsDMA, 1);
		CFileMgr::Write(fileHandle, (char*)&m_PrefsBrightness, 2);
		CFileMgr::Write(fileHandle, (char*)&m_PrefsLOD, sizeof(m_PrefsLOD));
		CFileMgr::Write(fileHandle, (char*)&m_PrefsShowSubtitles, 1);
		CFileMgr::Write(fileHandle, (char*)&m_PrefsUseWideScreen, 1);
		CFileMgr::Write(fileHandle, (char*)&m_PrefsFrameLimiter, 1);
		CFileMgr::Write(fileHandle, (char*)&m_nPrefsVideoMode, 1);
		CFileMgr::Write(fileHandle, m_PrefsSkinFile, 256);
		CFileMgr::Write(fileHandle, (char*)&m_ControlMethod, 1);
		CFileMgr::Write(fileHandle, (char*)&m_PrefsLanguage, 1);
		CFileMgr::Write(fileHandle, (char*)&m_PrefsShowHud, 1);
		CFileMgr::Write(fileHandle, (char*)&m_PrefsRadarMode, 1);
		CFileMgr::Write(fileHandle, (char*)&m_PrefsShowLegends, 1);
	}
	m_lastWorking3DAudioProvider = m_nPrefsAudio3DProviderIndex;

	CFileMgr::CloseFile(fileHandle);
	CFileMgr::SetDir("");
	
#ifdef LOAD_INI_SETTINGS
	SaveINISettings();
#endif
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

void
CMenuManager::SmallMessageScreen(const char* text)
{
	CFont::SetBackgroundOff();
	CFont::SetPropOn();
	CFont::SetJustifyOn();
	CFont::SetBackGroundOnlyTextOn();
	CSprite2d::DrawRect(CRect(SCREEN_STRETCH_X(95.0f), SCREEN_SCALE_FROM_BOTTOM(165.0f), SCREEN_STRETCH_FROM_RIGHT(95.0f), SCREEN_SCALE_Y(115.0f)), CRGBA(50, 50, 50, FadeIn(210)));
	CFont::SetFontStyle(FONT_LOCALE(FONT_STANDARD));
	CFont::SetCentreSize(SCREEN_SCALE_X(430.0f));
	CFont::SetCentreOn();
	CFont::SetColor(CRGBA(LABEL_COLOR.r, LABEL_COLOR.g, LABEL_COLOR.b, FadeIn(255)));
	CFont::SetDropShadowPosition(2);
	CFont::SetDropColor(CRGBA(0, 0, 0, 255));
	CFont::SetScale(SCREEN_SCALE_X(SMALLTEXT_X_SCALE), SCREEN_SCALE_Y(SMALLTEXT_Y_SCALE));
	
	int numOfLines = CFont::GetNumberLines(SCREEN_WIDTH / 2.f, SCREEN_SCALE_Y(135.f), TheText.Get(text));
	float y;
	if (numOfLines > 1)
		y = SCREEN_SCALE_Y(192.f) - numOfLines * SCREEN_SCALE_Y(8.f);
	else
		y = SCREEN_SCALE_Y(182.f);

	CFont::PrintString(SCREEN_WIDTH / 2.f, y, TheText.Get(text));
}

void
CMenuManager::PrintBriefs()
{
	CFont::SetColor(CRGBA(0, 0, 0, FadeIn(255)));
	CFont::SetRightJustifyOff();
	CFont::SetFontStyle(FONT_LOCALE(FONT_STANDARD));
	CFont::SetScale(MENU_X(MEDIUMTEXT_X_SCALE), MENU_Y(MEDIUMTEXT_Y_SCALE));
	CFont::SetWrapx(MENU_X_RIGHT_ALIGNED(80.0f));
	CFont::SetDropShadowPosition(0);

	float nextY = BRIEFS_BOTTOM_MARGIN;
	for (int i = 4; i >= 0; i--) {
		if (nextY < BRIEFS_TOP_MARGIN)
			break;

		tPreviousBrief &brief = CMessages::PreviousBriefs[i];
		if (brief.m_pText) {
			CMessages::InsertNumberInString(brief.m_pText,
				brief.m_nNumber[0], brief.m_nNumber[1],
				brief.m_nNumber[2], brief.m_nNumber[3],
				brief.m_nNumber[4], brief.m_nNumber[5], gUString);
			CMessages::InsertStringInString(gUString, brief.m_pString);
			CMessages::InsertPlayerControlKeysInString(gUString);
			CFont::FilterOutTokensFromString(gUString);

			nextY -= CFont::GetNumberLines(MENU_X_LEFT_ALIGNED(BRIEFS_LINE_X), nextY, gUString) * BRIEFS_LINE_HEIGHT + BRIEFS_LINE_SPACING;
			CFont::PrintString(MENU_X_LEFT_ALIGNED(BRIEFS_LINE_X), MENU_Y(nextY), gUString);
		}
	}
}

void
CMenuManager::PrintStats()
{
	static uint8 pirateCheck = 0;
	static float scrollY = 0;

	int rowNum = CStats::ConstructStatLine(99999);
	CSprite2d::Draw2DPolygon(MENU_X_LEFT_ALIGNED(90.0f), MENU_Y(142.0f),
		MENU_X_LEFT_ALIGNED(543.0f), MENU_Y(142.f),
		MENU_X_LEFT_ALIGNED(107.0f), MENU_Y(316.f),
		MENU_X_LEFT_ALIGNED(531.f), MENU_Y(299.f), CRGBA(LIST_BACKGROUND_COLOR.r, LIST_BACKGROUND_COLOR.g, LIST_BACKGROUND_COLOR.b, FadeIn(LIST_BACKGROUND_COLOR.a)));

	CFont::SetFontStyle(FONT_LOCALE(FONT_STANDARD));
	CFont::SetPropOn();
	CFont::SetDropShadowPosition(0);

	if (pirateCheck == 0)
		// if not pirated game
		pirateCheck = 46;
		// else
		// pirateCheck = 45;

	if (m_PrefsLanguage == LANGUAGE_AMERICAN)
		CFont::SetScale(MENU_X(0.43f), MENU_Y(0.75f));
	else
		CFont::SetScale(MENU_X(0.37f), MENU_Y(0.75f));

	static PauseModeTime lastCheck = 0;

	if (CTimer::GetTimeInMillisecondsPauseMode() - lastCheck > 40) {

		if (m_StatsScrollSpeed > 0.f) {
			if (m_StatsScrollDirection == 0)
				scrollY -= MENU_Y(100.f) / m_StatsScrollSpeed;
			else
				scrollY += MENU_Y(100.f) / m_StatsScrollSpeed;
		}
		lastCheck = CTimer::GetTimeInMillisecondsPauseMode();
	}

	if (pirateCheck == 45)
		return;

	float nextYChange, y, alpha;

	float totalHeight = (rowNum + 7) * STATS_ROW_HEIGHT;
	for (int row = 0; row < rowNum; ++row) {
		// Put faded away text at the top back to the bottom, in circular fashion
		for (y = MENU_Y(STATS_ROW_HEIGHT) * row + MENU_Y(100.f) - scrollY; MENU_Y(STATS_FADING_AREA_LENGTH) > y; y += nextYChange) {
			nextYChange = MENU_Y(totalHeight);
		}

		// Put faded away text at the bottom back to the top
		while (SCREEN_SCALE_FROM_BOTTOM(STATS_FADING_AREA_LENGTH) < y) {
			y -= MENU_Y(totalHeight);
		}
		alpha = 0.f;

		// If it's still on screen
		if (y > MENU_Y(STATS_VISIBLE_START_Y) && y < MENU_Y(STATS_VISIBLE_END_Y)) {
			CStats::ConstructStatLine(row);

			// But about to dim from bottom
			if (y < MENU_Y(STATS_BOTTOM_Y)) {
				if (y > MENU_Y(STATS_BOTTOM_Y - STATS_FADING_AREA_LENGTH))
					alpha = (MENU_Y(STATS_BOTTOM_Y) - y) * 5.f;
			}

			// About to dim from top
			if (y > MENU_Y(STATS_TOP_Y)) {
				if (y < MENU_Y(STATS_TOP_Y + STATS_FADING_AREA_LENGTH))
					alpha = (y - MENU_Y(STATS_TOP_Y)) * 5.f;
			}

			// Content
			if (y >= MENU_Y(STATS_TOP_Y + STATS_FADING_AREA_LENGTH) && y <= MENU_Y(STATS_BOTTOM_Y - STATS_FADING_AREA_LENGTH))
				alpha = 255.0f;

			CFont::SetColor(CRGBA(0, 0, 0, FadeIn(Min(255.f, alpha))));
			CFont::SetRightJustifyOff();
			CFont::PrintString(MENU_X_LEFT_ALIGNED(STATS_ROW_LEFT_MARGIN), y, gUString);
			CFont::SetRightJustifyOn();
			CFont::PrintString(MENU_X_RIGHT_ALIGNED(STATS_ROW_RIGHT_MARGIN), y, gUString2);
		}
	}
	CFont::SetColor(CRGBA(MENUOPTION_COLOR.r, MENUOPTION_COLOR.g, MENUOPTION_COLOR.b, FadeIn(255)));
	CFont::SetDropShadowPosition(2);
	CFont::SetDropColor(CRGBA(0, 0, 0, FadeIn(255)));
	CFont::SetCentreOn();
	CFont::SetFontStyle(FONT_LOCALE(FONT_HEADING));
	CFont::SetScale(MENU_X(0.65f), MENU_Y(1.05f));
	CFont::PrintString(MENU_X_LEFT_ALIGNED(STATS_RATING_X), MENU_Y(STATS_RATING_Y_1), TheText.Get("CRIMRA"));

	CFont::SetCentreOff();
	CFont::SetRightJustifyOff();

	// FIX: Game does that in a weird way, alignment and spacing is now ok

	sprintf(gString, "(%d)", CStats::FindCriminalRatingNumber());
	AsciiToUnicode(gString, gUString);

	UnicodeStrcpy(gUString2, CStats::FindCriminalRatingString());
	UnicodeStrcat(gUString2, gUString);

	CFont::SetFontStyle(FONT_LOCALE(FONT_STANDARD));
#ifndef FIX_BUGS
	CFont::SetScale(MENU_X(0.5f), MENU_Y(0.9f));
#else
	CFont::SetScale(MENU_X(SMALLTEXT_X_SCALE), MENU_Y(SMALLTEXT_Y_SCALE));
#endif
	CFont::SetColor(CRGBA(0, 0, 0, FadeIn(255)));
	CFont::SetDropShadowPosition(0);

	CFont::PrintString(MENU_X_LEFT_ALIGNED(STATS_RATING_X) - CFont::GetStringWidth(gUString2, true) / 2.f, MENU_Y(STATS_RATING_Y_2), gUString2);
}

void
CMenuManager::Process(void)
{
#ifdef XBOX_MESSAGE_SCREEN
	ProcessDialogTimer();
#endif

	if (TheCamera.GetScreenFadeStatus() != FADE_0)
		return;

	InitialiseChangedLanguageSettings();

	if (m_bMenuActive) {
		UserInput();
		ProcessFileActions();
		DMAudio.Service();
#ifdef USE_TEXTURE_POOL
		// TODO
#endif
	}

	SwitchMenuOnAndOff();
}

#ifdef MAP_ENHANCEMENTS
#define ZOOM(x, y, in) \
	do { \
		if(m_fMapSize >= MENU_Y(1000.0f) && in) \
			break; \
		float z2 = in? 1.1f : 1.f/1.1f; \
		m_fMapCenterX += (x - m_fMapCenterX) * (1.0f - z2); \
		m_fMapCenterY += (y - m_fMapCenterY) * (1.0f - z2); \
		\
		if (m_fMapSize <= MENU_Y(MAP_MIN_SIZE) && !in) \
			break; \
		\
		m_fMapSize *= z2; \
		m_fMapCenterX = clamp(m_fMapCenterX, SCREEN_WIDTH/2 - (m_fMapSize - MENU_X(MAP_MIN_SIZE)), m_fMapSize - MENU_X(MAP_MIN_SIZE) + SCREEN_WIDTH/2); \
		m_fMapCenterY = clamp(m_fMapCenterY, SCREEN_HEIGHT/2 - (m_fMapSize - MENU_Y(MAP_MIN_SIZE)), m_fMapSize - MENU_Y(MAP_MIN_SIZE) + SCREEN_HEIGHT/2); \
	} while(0)

#endif

// Handles Map, Audio and Stats
void
CMenuManager::AdditionalOptionInput(bool &goBack)
{
	switch (m_nCurrScreen) {
		case MENUPAGE_MAP:
		{
			static PauseModeTime lastMapTick = 0;

			// FIX: All those macros were hardcoded values originally.

#ifndef MAP_ENHANCEMENTS
			if (CPad::GetPad(0)->GetMouseWheelUpJustDown() || CPad::GetPad(0)->GetMouseWheelUpJustUp() || CPad::GetPad(0)->GetPageUp() || CPad::GetPad(0)->GetRightShoulder1()) {
				if (CTimer::GetTimeInMillisecondsPauseMode() - lastMapTick > 10) {
					m_fMapSize = Min(MENU_Y(1000.0f), m_fMapSize + MENU_Y(15.f));
				}
			}
			if (CPad::GetPad(0)->GetMouseWheelDownJustDown() || CPad::GetPad(0)->GetMouseWheelDownJustUp() || CPad::GetPad(0)->GetPageDown() || CPad::GetPad(0)->GetRightShoulder2()) {
				if (CTimer::GetTimeInMillisecondsPauseMode() - lastMapTick > 10) {
					if (m_fMapSize > MENU_Y(MAP_MIN_SIZE)) {
						if (m_fMapCenterY > SCREEN_HEIGHT/2)
							m_fMapCenterY -= (m_fMapCenterY - SCREEN_HEIGHT/2) / ((m_fMapSize - MENU_Y(MAP_MIN_SIZE)) * 1/15.f);

						if (m_fMapCenterY < SCREEN_HEIGHT/2)
							m_fMapCenterY += (SCREEN_HEIGHT/2 - m_fMapCenterY) / ((m_fMapSize - MENU_Y(MAP_MIN_SIZE)) * 1/15.f);

						if (m_fMapCenterX > SCREEN_WIDTH/2)
							m_fMapCenterX -= (m_fMapCenterX - SCREEN_WIDTH/2) / ((m_fMapSize - MENU_X(MAP_MIN_SIZE)) * 1/15.f);

						if (m_fMapCenterX < SCREEN_WIDTH/2)
							m_fMapCenterX += (SCREEN_WIDTH/2 - m_fMapCenterX) / ((m_fMapSize - MENU_X(MAP_MIN_SIZE)) * 1/15.f);

						m_fMapSize = Max(MENU_Y(MAP_MIN_SIZE), m_fMapSize - MENU_Y(15.f));
						m_fMapCenterX = clamp(m_fMapCenterX, SCREEN_WIDTH/2 - (m_fMapSize - MENU_X(MAP_MIN_SIZE)), m_fMapSize - MENU_X(MAP_MIN_SIZE) + SCREEN_WIDTH/2);
						m_fMapCenterY = clamp(m_fMapCenterY, SCREEN_HEIGHT/2 - (m_fMapSize - MENU_Y(MAP_MIN_SIZE)), m_fMapSize - MENU_Y(MAP_MIN_SIZE) + SCREEN_HEIGHT/2);
					} else {
						m_fMapSize = MENU_Y(MAP_MIN_SIZE);
					}
				}
			}
#else
			// Adding marker
			if (m_nMenuFadeAlpha == 255) {
				if (CPad::GetPad(0)->GetRightMouseJustDown() || CPad::GetPad(0)->GetCrossJustDown()) {
					if (mapCrosshair.y > m_fMapCenterY - m_fMapSize && mapCrosshair.y < m_fMapCenterY + m_fMapSize &&
						mapCrosshair.x > m_fMapCenterX - m_fMapSize && mapCrosshair.x < m_fMapCenterX + m_fMapSize) {

						// Don't ask me the meanings, I don't know. Found them by trying
						float diffX = m_fMapCenterX - m_fMapSize, diffY = m_fMapCenterY - m_fMapSize;
						float x = ((mapCrosshair.x - diffX) / (m_fMapSize * 2)) * (WORLD_SIZE_X / MENU_MAP_WIDTH_SCALE) - (WORLD_SIZE_X / 2 + MENU_MAP_LEFT_OFFSET * MENU_MAP_LENGTH_UNIT);
						float y = (WORLD_SIZE_Y / 2 - MENU_MAP_TOP_OFFSET * MENU_MAP_LENGTH_UNIT) - ((mapCrosshair.y - diffY) / (m_fMapSize * 2)) * (WORLD_SIZE_Y / MENU_MAP_HEIGHT_SCALE);
						CRadar::ToggleTargetMarker(x, y);
						DMAudio.PlayFrontEndSound(SOUND_FRONTEND_ENTER_OR_ADJUST, 0);
					}
				}
			}

			if (CPad::GetPad(0)->GetMouseWheelDown() || CPad::GetPad(0)->GetPageDown() || CPad::GetPad(0)->GetRightShoulder2()) {
				if (CPad::GetPad(0)->GetMouseWheelDown() && m_fMapSize > MENU_X(MAP_SIZE_TO_ALLOW_X_MOVE))
					ZOOM(mapCrosshair.x, mapCrosshair.y, false);
				else
					ZOOM(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, false);

			} else if (CPad::GetPad(0)->GetMouseWheelUp() || CPad::GetPad(0)->GetPageUp() || CPad::GetPad(0)->GetRightShoulder1()) {
				if (CPad::GetPad(0)->GetMouseWheelUp())
					ZOOM(mapCrosshair.x, mapCrosshair.y, true);
				else
					ZOOM(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, true);
			}
			
			static bool justResetPointer = false;
			if (CPad::GetPad(0)->GetLeftMouse()) {
				if (!justResetPointer) {
					m_fMapCenterX += m_nMousePosX - m_nMouseOldPosX;
					m_fMapCenterY += m_nMousePosY - m_nMouseOldPosY;
					m_fMapCenterX = clamp(m_fMapCenterX, SCREEN_WIDTH/2 - (m_fMapSize - MENU_X(MAP_MIN_SIZE)), m_fMapSize - MENU_X(MAP_MIN_SIZE) + SCREEN_WIDTH/2);
					m_fMapCenterY = clamp(m_fMapCenterY, SCREEN_HEIGHT/2 - (m_fMapSize - MENU_Y(MAP_MIN_SIZE)), m_fMapSize - MENU_Y(MAP_MIN_SIZE) + SCREEN_HEIGHT/2);
				}
				justResetPointer = false;

			} else
#undef ZOOM
#endif

			{
				// This is else block of GetLeftMouse() if MAP_ENHANCEMENTS defined, so all of GetLeftMouse() conditions below being rendered useless. 

				if (CPad::GetPad(0)->GetLeftMouse() && m_nMousePosY < m_nMouseOldPosY || CPad::GetPad(0)->GetUp() ||
					CPad::GetPad(0)->GetDPadUp() || CPad::GetPad(0)->GetAnalogueUpDown() < 0) {
					if (CTimer::GetTimeInMillisecondsPauseMode() - lastMapTick > 10) {
						if ((m_fMapSize - MENU_Y(MAP_MIN_SIZE)) + SCREEN_HEIGHT/2 > m_fMapCenterY)
							m_fMapCenterY += MENU_Y(15.f);
						m_bShowMouse = false;
					}				
				}

				if (CPad::GetPad(0)->GetLeftMouse() && m_nMousePosY > m_nMouseOldPosY || CPad::GetPad(0)->GetDown() ||
					CPad::GetPad(0)->GetDPadDown() || CPad::GetPad(0)->GetAnalogueUpDown() > 0) {
					if (CTimer::GetTimeInMillisecondsPauseMode() - lastMapTick > 10) {
						if (SCREEN_HEIGHT/2 - (m_fMapSize - MENU_Y(MAP_MIN_SIZE)) < m_fMapCenterY)
							m_fMapCenterY -= MENU_Y(15.f);
						m_bShowMouse = false;
					}				
				}

				if (CPad::GetPad(0)->GetLeftMouse() && m_nMousePosX < m_nMouseOldPosX || CPad::GetPad(0)->GetLeft() ||
					CPad::GetPad(0)->GetDPadLeft() || CPad::GetPad(0)->GetAnalogueLeftRight() < 0) {
					if (CTimer::GetTimeInMillisecondsPauseMode() - lastMapTick > 10) {
						if (m_fMapSize > MENU_X(MAP_SIZE_TO_ALLOW_X_MOVE) && m_fMapSize - MENU_X(MAP_MIN_SIZE) + SCREEN_WIDTH/2 > m_fMapCenterX)
							m_fMapCenterX += MENU_X(15.f);
						m_bShowMouse = false;
					}				
				}

				if (CPad::GetPad(0)->GetLeftMouseJustUp()) {
					// The coordinates in aScreens->MENUPAGE_MAP.
					if (m_nMousePosX > MENU_X_LEFT_ALIGNED(60.0f) && m_nMousePosX < MENU_X_LEFT_ALIGNED(140.0f)) {
						if (m_nMousePosY > MENU_Y(375.0f) && m_nMousePosY < MENU_Y(400.0f)) {
								m_nHoverOption = HOVEROPTION_RANDOM_ITEM;
								goBack = true;
						}
					}
				}

				if (CPad::GetPad(0)->GetLeftMouse() && m_nMousePosX > m_nMouseOldPosX || CPad::GetPad(0)->GetRight() ||
					CPad::GetPad(0)->GetDPadRight() || CPad::GetPad(0)->GetAnalogueLeftRight() > 0) {
					if (CTimer::GetTimeInMillisecondsPauseMode() - lastMapTick > 10) {
						if (m_fMapSize > MENU_X(MAP_SIZE_TO_ALLOW_X_MOVE) && SCREEN_WIDTH/2 - (m_fMapSize - MENU_X(MAP_MIN_SIZE)) < m_fMapCenterX)
							m_fMapCenterX -= MENU_X(15.f);
						m_bShowMouse = false;
					}				
				}
			}


			if (CTimer::GetTimeInMillisecondsPauseMode() - lastMapTick > 10)
				lastMapTick = CTimer::GetTimeInMillisecondsPauseMode();

#ifndef MAP_ENHANCEMENTS
			if (CPad::GetPad(0)->GetLeftMouseJustUp())
				CentreMousePointer();
#endif

			if (CPad::GetPad(0)->GetLeftMouse()) {
				if (m_nMousePosX < SCREEN_STRETCH_X(20.0f) || m_nMousePosX > SCREEN_STRETCH_X(620.0f) || m_nMousePosY < SCREEN_STRETCH_Y(20.0f) || m_nMousePosY > SCREEN_STRETCH_Y(428.0f)) {
#ifdef MAP_ENHANCEMENTS
					justResetPointer = true;
#endif
					CentreMousePointer();
				}
			}
			if (!CPad::GetPad(0)->GetLeftMouse() && !m_bShowMouse && (m_nMouseOldPosX != m_nMousePosX || m_nMouseOldPosY != m_nMousePosY)) {
				m_bShowMouse = true;
			}

			static bool pressedL = false;

			if (!CPad::GetPad(0)->GetChar('L') && !CPad::GetPad(0)->GetChar('l')) {
				pressedL = false;
			}

			if (!pressedL) {
				if (CPad::GetPad(0)->GetChar('L') || CPad::GetPad(0)->GetChar('l')) {
					m_PrefsShowLegends = !m_PrefsShowLegends;
					pressedL = true;
				}
			}
			break;
		}
		case MENUPAGE_SOUND_SETTINGS:
			if (CheckHover(MENU_X_LEFT_ALIGNED(177.f), MENU_X_LEFT_ALIGNED(238.f), MENU_Y(MENURADIO_SELECTOR_START_Y - 13.f), MENU_Y(MENURADIO_SELECTOR_START_Y + MENURADIO_SELECTOR_HEIGHT - 8.f))) {
					m_nHoverOption = HOVEROPTION_PREV_RADIO;
			}

			if (CheckHover(MENU_X_LEFT_ALIGNED(422.f), MENU_X_LEFT_ALIGNED(491.f), MENU_Y(MENURADIO_SELECTOR_START_Y - 13.f), MENU_Y(MENURADIO_SELECTOR_START_Y + MENURADIO_SELECTOR_HEIGHT - 8.f))) {
					m_nHoverOption = HOVEROPTION_NEXT_RADIO;
			}
			break;
		case MENUPAGE_STATS:
		{
			if (CPad::GetPad(0)->GetMouseWheelUpJustDown() || CPad::GetPad(0)->GetMouseWheelUpJustUp() || CPad::GetPad(0)->GetUp() ||
				CPad::GetPad(0)->GetDPadUp() || CPad::GetPad(0)->GetAnalogueUpDown() < 0) {

				m_StatsScrollSpeed = 20.0f;
				m_StatsScrollDirection = 0;

			} else if (CPad::GetPad(0)->GetMouseWheelDownJustDown() || CPad::GetPad(0)->GetMouseWheelDownJustUp() || CPad::GetPad(0)->GetDown() ||
				CPad::GetPad(0)->GetDPadDown() || CPad::GetPad(0)->GetAnalogueUpDown() > 0) {
				
				m_StatsScrollSpeed = 20.0f;
				m_StatsScrollDirection = 1;

			} else if (CPad::GetPad(0)->GetChar(' ')) {
				m_StatsScrollSpeed = 0.0f;
			} else
				m_StatsScrollSpeed = 150.0f;

			static bool pressedS = false;

			if (!CPad::GetPad(0)->GetChar('S') && !CPad::GetPad(0)->GetChar('s')) {
				pressedS = false;
			}

			if (!pressedS) {
				if (CPad::GetPad(0)->GetChar('S') || CPad::GetPad(0)->GetChar('s')) {
					ExportStats();
					m_nHelperTextMsgId = 4;
					m_nHelperTextAlpha = 300;
					pressedS = true;
				}
			}
			break;
		}
	}
}

// Not original name
void
CMenuManager::ExportStats()
{
	char date[10];
	CFileMgr::SetDirMyDocuments();
	_strdate(date);
	wchar *lastMission = TheText.Get(CStats::LastMissionPassedName[0] == '\0' ? "ITBEG" : CStats::LastMissionPassedName);
	FILE *txtFile = fopen("stats.txt", "w");

	if (txtFile) {
		int statLines = CStats::ConstructStatLine(99999);
		fprintf(txtFile, "-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n");
		fprintf(txtFile, "\t\t\tGTA VICE CITY %s\n", UnicodeToAscii(TheText.Get("FEH_STA")));
		fprintf(txtFile, "-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n\n\n");
		fprintf(txtFile, "%s: ", UnicodeToAscii(TheText.Get("FES_CMI")));
		fprintf(txtFile, "%s\n", UnicodeToAscii(lastMission));
		fprintf(txtFile, "%s: ", UnicodeToAscii(TheText.Get("FES_DAT")));
		fprintf(txtFile, "%s\n\n\n", date);
		fprintf(txtFile, "%s  ", UnicodeToAscii(TheText.Get("CRIMRA")));
		UnicodeStrcpy(gUString, CStats::FindCriminalRatingString());
		fprintf(txtFile, "%s (%d)\n\n\n", UnicodeToAscii(gUString), CStats::FindCriminalRatingNumber());
		for (int i = 0; i < statLines; ++i) {
			CStats::ConstructStatLine(i);
			char *statKey = UnicodeToAscii(gUString);
			if (statKey[0] != '\0')
				fprintf(txtFile, "- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -\n%s\n", statKey);

			char *statValue = UnicodeToAscii(gUString2);
			for (int j = 0; statValue[j] != '\0'; ++j) {
				if (statValue[j] == '_')
					statValue[j] = 0xBA; // This is degree symbol, but my editors keeps messing up with it so I wrote hex representation
			}
			if (statValue)
				fprintf(txtFile, "%s\n\n", statValue);
		}
		fprintf(txtFile, "-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n\n");
	}
	fclose(txtFile);
	FILE *htmlFile = fopen("stats.html", "w");
	if (htmlFile) {
		int statLines = CStats::ConstructStatLine(99999);
		fprintf(htmlFile, "<title>Grand Theft Auto Vice City Stats</title>\n");
		fprintf(htmlFile, "<body bgcolor=\"#FF00CC\" leftmargin=\"10\" topmargin=\"10\" marginwidth=\"10\" marginheight=\"10\">\n");
		fprintf(htmlFile, "<table width=\"560\" align=\"center\" border=\"0\" cellpadding=\"5\" cellspacing=\"0\">\n");
		fprintf(htmlFile, "<tr align=\"center\" valign=\"top\"> \n");
		fprintf(htmlFile, "<td height=\"59\" colspan=\"2\" bgcolor=\"#FFCCFF\"><div align=\"center\"><font color=\"#FF00CC\" size=\"3\" "
			"face=\"Arial, \n");
		fprintf(htmlFile, "Helvetica, sans-serif\">-------------------------------------------------------------------------</font><font \n");
		fprintf(htmlFile, "size=\"3\" face=\"Arial, Helvetica, sans-serif\"><br>\n");
		fprintf(htmlFile, "<strong><font color=\"#000000\">GRAND THEFT AUTO VICE CITY ");
		fprintf(htmlFile, "%s</font></strong><br><font\n", UnicodeToAscii(TheText.Get("FEH_STA")));
		fprintf(htmlFile, "color=\"#FF00CC\">-------------------------------------------------------------------------</font></font></div></td> </tr>\n");
		fprintf(htmlFile, "<tr align=\"left\" valign=\"top\" bgcolor=\"#FFFFFF\">     <td height=\"22\" colspan=\"2\">&nbsp;</td>  </tr>\n");
		fprintf(htmlFile, "<tr align=\"left\" valign=\"top\" bgcolor=\"#FFFFFF\"> \n");
		fprintf(htmlFile,
			"<td height=\"40\" colspan=\"2\"> <p><font color=\"#00CC00\" size=\"2\" face=\"Arial, Helvetica, sans-serif\">"
			"<strong><font color=\"#009900\" size=\"1\">%s: \n", UnicodeToAscii(TheText.Get("FES_DAT")));
		fprintf(htmlFile, "%s</font><br>        %s: </strong>", date, UnicodeToAscii(TheText.Get("FES_CMI")));
		fprintf(htmlFile, "%s<strong><br></strong> </font></p></td></tr>\n", UnicodeToAscii(lastMission));
		fprintf(htmlFile, "<tr align=\"left\" valign=\"top\" bgcolor=\"#CCCCCC\"> <td height=\"5\" colspan=\"2\"></td> </tr> <tr align=\""
			"left\" valign=\"top\" bgcolor=\"#FFFFFF\"> \n");
		fprintf(htmlFile, "<td height=\"10\" colspan=\"2\"></td> </tr> <tr align=\"left\" valign=\"top\" bgcolor=\"#FFFFFF\"> \n");
		fprintf(htmlFile, "<td height=\"20\" colspan=\"2\"><font color=\"#FF00CC\" size=\"2\" face=\"Arial, Helvetica, sans-serif\"><str"
			"ong>%s</strong>\n", UnicodeToAscii(TheText.Get("CRIMRA")));

		UnicodeStrcpy(gUString, CStats::FindCriminalRatingString());
		char *statKey = UnicodeToAscii(gUString);
		int rating = CStats::FindCriminalRatingNumber();
		fprintf(htmlFile, "%s (%d)</font></td>  </tr>  <tr align=\"left\" valign=\"top\" bgcolor=\"#FFFFFF\"><td height=\"10\" colspan=\""
			"2\"></td>  </tr>\n", statKey, rating);

		for (int k = 0; k < statLines; ++k) {
			CStats::ConstructStatLine(k);
			statKey = UnicodeToAscii(gUString);
			if (statKey[0] != '\0')
				fprintf(htmlFile, "</font></strong></div></td> </tr> <tr align=\"left\" valign=\"top\" bgcolor=\"#FFFFFF\">  <td height=\"10"
					"\" colspan=\"2\"></td> </tr>\n");

			fprintf(htmlFile, "<tr align=\"left\" valign=\"top\"><td width=\"500\" height=\"22\" bgcolor=\"#FFCCFF\"><font color=\"#FF00CC"
				"\" size=\"2\" face=\"Arial, Helvetica, sans-serif\"><strong>\n");

			if (statKey[0] != '\0')
				fprintf(htmlFile, "%s", statKey);
			else
				fprintf(htmlFile, " ");

			fprintf(htmlFile, "</strong></font></td> <td width=\"500\" align=\"right\" valign=\"middle\" bgcolor=\"#FFCCFF\"> <div align=\""
				"right\"><strong><font color=\"#FF00CC\">\n");

			char *statValue = UnicodeToAscii(gUString2);
			for (int l = 0; statValue[l] != '\0'; ++l) {
				if (statValue[l] == '_')
					statValue[l] = 0xBA; // This is degree symbol, but my editors keeps messing up with it so I wrote hex representation
			}
			if (statValue)
				fprintf(htmlFile, "%s", statValue);
			else
				fprintf(htmlFile, " ");
		}
		fprintf(htmlFile, "</font></strong></div></td> </tr> <tr align=\"left\" valign=\"top\" bgcolor=\"#FFFFFF\">  <td height=\"10\" c"
			"olspan=\"2\"></td> </tr>\n");
		fprintf(htmlFile, "</table><br><table width=\"560\" border=\"0\"  align=\"center\" cellspacing=\"0\" cellpadding=\"5\"><tr align"
			"=\"center\" valign=\"middle\" bgcolor=\"#FFCCFF\">");
		fprintf(htmlFile, "<td><font color=\"#000000\" size=\"2\" face=\"Arial, Helvetica, sans-serif\"><a href=\"http://www.rockstargam"
			"es.com/vicecity\">rockstargames.com/vicecity</a></font></td>\n");
		fprintf(htmlFile, "<td><font color=\"#000000\" size=\"2\" face=\"Arial, Helvetica, sans-serif\"><a href=\"http://www.rockstargam"
			"es.com\">rockstargames.com</a></font></td>\n");
		fprintf(htmlFile, "<td><font color=\"#000000\" size=\"2\" face=\"Arial, Helvetica, sans-serif\">&nbsp;<a href=\"http://www.rocks"
			"tarnorth.com\">rockstarnorth.com</a></font></td></tr>\n");
		fprintf(htmlFile, "</table>\n</body>\n");
	}
	fclose(htmlFile);
	CFileMgr::SetDir("");
}

// Original name is unknown
void
CMenuManager::PrintRadioSelector(void)
{
	static PauseModeTime lastRadioChange = 0;

	CSprite2d::Draw2DPolygon(MENU_X_LEFT_ALIGNED(418.f), MENU_Y(MENURADIO_SELECTOR_START_Y + MENURADIO_SELECTOR_HEIGHT),
		MENU_X_LEFT_ALIGNED(228.f), MENU_Y(MENURADIO_SELECTOR_START_Y + MENURADIO_SELECTOR_HEIGHT),
		MENU_X_LEFT_ALIGNED(428.f), MENU_Y(MENURADIO_SELECTOR_START_Y),
		MENU_X_LEFT_ALIGNED(238.f), MENU_Y(MENURADIO_SELECTOR_START_Y), CRGBA(RADIO_SELECTOR_COLOR.r, RADIO_SELECTOR_COLOR.g, RADIO_SELECTOR_COLOR.b, FadeIn(180)));

	int rightMostSprite, rightMostStation;
	if (DMAudio.IsMP3RadioChannelAvailable()) {
		rightMostSprite = MENUSPRITE_MP3;
		rightMostStation = USERTRACK;
	} else {
		rightMostSprite = MENUSPRITE_WAVE;
		rightMostStation = WAVE;
	}
 #ifdef THIS_IS_STUPID

	// First radio
	if (m_ScrollRadioBy == 1) {
		if (m_PrefsRadioStation == 1) {
			m_aFrontEndSprites[rightMostSprite].Draw(m_LeftMostRadioX, MENU_Y(MENURADIO_ICON_Y), MENU_X(MENURADIO_ICON_SIZE), MENU_Y(MENURADIO_ICON_SIZE),
				CRGBA(INACTIVE_RADIO_COLOR.r, INACTIVE_RADIO_COLOR.g, INACTIVE_RADIO_COLOR.b, FadeIn(INACTIVE_RADIO_COLOR.a)));
		} else if ( m_PrefsRadioStation == 0) {
			m_aFrontEndSprites[rightMostSprite - 1].Draw(m_LeftMostRadioX, MENU_Y(MENURADIO_ICON_Y), MENU_X(MENURADIO_ICON_SIZE), MENU_Y(MENURADIO_ICON_SIZE),
				CRGBA(INACTIVE_RADIO_COLOR.r, INACTIVE_RADIO_COLOR.g, INACTIVE_RADIO_COLOR.b, FadeIn(INACTIVE_RADIO_COLOR.a)));
		} else {
			m_aFrontEndSprites[m_PrefsRadioStation + MENUSPRITE_WILDSTYLE - 2].Draw(m_LeftMostRadioX, MENU_Y(MENURADIO_ICON_Y), MENU_X(MENURADIO_ICON_SIZE), MENU_Y(MENURADIO_ICON_SIZE),
				CRGBA(INACTIVE_RADIO_COLOR.r, INACTIVE_RADIO_COLOR.g, INACTIVE_RADIO_COLOR.b, FadeIn(INACTIVE_RADIO_COLOR.a)));
		}
	}

	// Second
	if (m_PrefsRadioStation == 0) {
		m_aFrontEndSprites[rightMostSprite].Draw(m_LeftMostRadioX + MENU_X(MENURADIO_ICON_SIZE), MENU_Y(MENURADIO_ICON_Y), MENU_X(MENURADIO_ICON_SIZE), MENU_Y(MENURADIO_ICON_SIZE),
			CRGBA(INACTIVE_RADIO_COLOR.r, INACTIVE_RADIO_COLOR.g, INACTIVE_RADIO_COLOR.b, FadeIn(INACTIVE_RADIO_COLOR.a)));
	} else {
		m_aFrontEndSprites[m_PrefsRadioStation + MENUSPRITE_WILDSTYLE - 1].Draw(m_LeftMostRadioX + MENU_X(MENURADIO_ICON_SIZE), MENU_Y(MENURADIO_ICON_Y), MENU_X(MENURADIO_ICON_SIZE),
			MENU_Y(MENURADIO_ICON_SIZE), CRGBA(INACTIVE_RADIO_COLOR.r, INACTIVE_RADIO_COLOR.g, INACTIVE_RADIO_COLOR.b, FadeIn(INACTIVE_RADIO_COLOR.a)));
	}

	// Third (middle)
	int prevStation = m_PrefsRadioStation - 1;
	if (prevStation == rightMostStation) {
		m_aFrontEndSprites[MENUSPRITE_WILDSTYLE + 1].Draw(m_LeftMostRadioX + MENU_X(MENURADIO_ICON_SIZE * 3), MENU_Y(MENURADIO_ICON_Y), MENU_X(MENURADIO_ICON_SIZE), MENU_Y(MENURADIO_ICON_SIZE),
			CRGBA(INACTIVE_RADIO_COLOR.r, INACTIVE_RADIO_COLOR.g, INACTIVE_RADIO_COLOR.b, FadeIn(INACTIVE_RADIO_COLOR.a)));
	} else if ( prevStation == rightMostStation - 1) {
		m_aFrontEndSprites[MENUSPRITE_WILDSTYLE].Draw(m_LeftMostRadioX + MENU_X(MENURADIO_ICON_SIZE * 3), MENU_Y(MENURADIO_ICON_Y), MENU_X(MENURADIO_ICON_SIZE), MENU_Y(MENURADIO_ICON_SIZE),
			CRGBA(INACTIVE_RADIO_COLOR.r, INACTIVE_RADIO_COLOR.g, INACTIVE_RADIO_COLOR.b, FadeIn(INACTIVE_RADIO_COLOR.a)));
	} else {
		m_aFrontEndSprites[m_PrefsRadioStation + MENUSPRITE_WILDSTYLE + 1].Draw(m_LeftMostRadioX + MENU_X(MENURADIO_ICON_SIZE * 3), MENU_Y(MENURADIO_ICON_Y), MENU_X(MENURADIO_ICON_SIZE), MENU_Y(MENURADIO_ICON_SIZE),
			CRGBA(INACTIVE_RADIO_COLOR.r, INACTIVE_RADIO_COLOR.g, INACTIVE_RADIO_COLOR.b, FadeIn(INACTIVE_RADIO_COLOR.a)));
	}

	// Fifth
	if (m_ScrollRadioBy == -1) {
		int prevStation = m_PrefsRadioStation - 1;
		if (prevStation == rightMostStation) {
			m_aFrontEndSprites[MENUSPRITE_WILDSTYLE + 4].Draw(m_LeftMostRadioX + MENU_X(MENURADIO_ICON_SIZE * 4), MENU_Y(MENURADIO_ICON_Y), MENU_X(MENURADIO_ICON_SIZE), MENU_Y(MENURADIO_ICON_SIZE),
				CRGBA(INACTIVE_RADIO_COLOR.r, INACTIVE_RADIO_COLOR.g, INACTIVE_RADIO_COLOR.b, FadeIn(INACTIVE_RADIO_COLOR.a)));
		} else if (prevStation == rightMostStation - 1) {
			m_aFrontEndSprites[MENUSPRITE_WILDSTYLE + 1].Draw(m_LeftMostRadioX + MENU_X(MENURADIO_ICON_SIZE * 4), MENU_Y(MENURADIO_ICON_Y), MENU_X(MENURADIO_ICON_SIZE), MENU_Y(MENURADIO_ICON_SIZE),
				CRGBA(INACTIVE_RADIO_COLOR.r, INACTIVE_RADIO_COLOR.g, INACTIVE_RADIO_COLOR.b, FadeIn(INACTIVE_RADIO_COLOR.a)));
		} else if ( prevStation == rightMostStation - 2) {
			m_aFrontEndSprites[MENUSPRITE_WILDSTYLE].Draw(m_LeftMostRadioX + MENU_X(MENURADIO_ICON_SIZE * 4), MENU_Y(MENURADIO_ICON_Y), MENU_X(MENURADIO_ICON_SIZE), MENU_Y(MENURADIO_ICON_SIZE),
				CRGBA(INACTIVE_RADIO_COLOR.r, INACTIVE_RADIO_COLOR.g, INACTIVE_RADIO_COLOR.b, FadeIn(INACTIVE_RADIO_COLOR.a)));
		} else {
			m_aFrontEndSprites[m_PrefsRadioStation + MENUSPRITE_WILDSTYLE + 2].Draw(m_LeftMostRadioX + MENU_X(MENURADIO_ICON_SIZE * 4), MENU_Y(MENURADIO_ICON_Y), MENU_X(MENURADIO_ICON_SIZE), MENU_Y(MENURADIO_ICON_SIZE),
				CRGBA(INACTIVE_RADIO_COLOR.r, INACTIVE_RADIO_COLOR.g, INACTIVE_RADIO_COLOR.b, FadeIn(INACTIVE_RADIO_COLOR.a)));
		}
	}

	// Fourth
	if (m_ScrollRadioBy == 0) {
		m_aFrontEndSprites[m_PrefsRadioStation + MENUSPRITE_WILDSTYLE].Draw(m_LeftMostRadioX + MENU_X(MENURADIO_ICON_SIZE * 2 - 10.f), MENU_Y(MENURADIO_ICON_Y - 10.f), MENU_X(MENURADIO_ICON_SIZE) + MENU_X(20.f), MENU_Y(MENURADIO_ICON_SIZE) + MENU_Y(20.f),
			CRGBA(255, 255, 255, FadeIn(255)));
	} else {	
		if (m_PrefsRadioStation - 1 == rightMostStation) {
			m_aFrontEndSprites[MENUSPRITE_WILDSTYLE].Draw(m_LeftMostRadioX + MENU_X(MENURADIO_ICON_SIZE * 2), MENU_Y(MENURADIO_ICON_Y), MENU_X(MENURADIO_ICON_SIZE), MENU_Y(MENURADIO_ICON_SIZE),
				CRGBA(INACTIVE_RADIO_COLOR.r, INACTIVE_RADIO_COLOR.g, INACTIVE_RADIO_COLOR.b, FadeIn(INACTIVE_RADIO_COLOR.a)));
		} else {
			m_aFrontEndSprites[m_PrefsRadioStation + MENUSPRITE_WILDSTYLE].Draw(m_LeftMostRadioX + MENU_X(MENURADIO_ICON_SIZE * 2), MENU_Y(MENURADIO_ICON_Y), MENU_X(MENURADIO_ICON_SIZE), MENU_Y(MENURADIO_ICON_SIZE),
				CRGBA(INACTIVE_RADIO_COLOR.r, INACTIVE_RADIO_COLOR.g, INACTIVE_RADIO_COLOR.b, FadeIn(INACTIVE_RADIO_COLOR.a)));
		}
	}
#else
	int first = ((m_PrefsRadioStation - 2) + rightMostStation + 1) % (rightMostStation + 1);
	int second = ((m_PrefsRadioStation - 1) + rightMostStation + 1) % (rightMostStation + 1);
	int third = ((m_PrefsRadioStation) + rightMostStation + 1) % (rightMostStation + 1);
	int fourth = ((m_PrefsRadioStation + 1) + rightMostStation + 1) % (rightMostStation + 1);
	int fifth = ((m_PrefsRadioStation + 2) + rightMostStation + 1) % (rightMostStation + 1);

	// First one is only drawn on transition to next
	if (m_ScrollRadioBy == 1) {
		m_aFrontEndSprites[first + MENUSPRITE_WILDSTYLE].Draw(m_LeftMostRadioX, MENU_Y(MENURADIO_ICON_Y), MENU_X(MENURADIO_ICON_SIZE), MENU_Y(MENURADIO_ICON_SIZE),
			CRGBA(INACTIVE_RADIO_COLOR.r, INACTIVE_RADIO_COLOR.g, INACTIVE_RADIO_COLOR.b, FadeIn(INACTIVE_RADIO_COLOR.a)));
	}

	// Second
	m_aFrontEndSprites[second + MENUSPRITE_WILDSTYLE].Draw(m_LeftMostRadioX + MENU_X(MENURADIO_ICON_SIZE), MENU_Y(MENURADIO_ICON_Y), MENU_X(MENURADIO_ICON_SIZE), MENU_Y(MENURADIO_ICON_SIZE),
		CRGBA(INACTIVE_RADIO_COLOR.r, INACTIVE_RADIO_COLOR.g, INACTIVE_RADIO_COLOR.b, FadeIn(INACTIVE_RADIO_COLOR.a)));

	// Fourth
	m_aFrontEndSprites[fourth + MENUSPRITE_WILDSTYLE].Draw(m_LeftMostRadioX + MENU_X(MENURADIO_ICON_SIZE * 3), MENU_Y(MENURADIO_ICON_Y), MENU_X(MENURADIO_ICON_SIZE), MENU_Y(MENURADIO_ICON_SIZE),
		CRGBA(INACTIVE_RADIO_COLOR.r, INACTIVE_RADIO_COLOR.g, INACTIVE_RADIO_COLOR.b, FadeIn(INACTIVE_RADIO_COLOR.a)));

	// Fifth one is only drawn on transition to prev.
	if (m_ScrollRadioBy == -1) {
		m_aFrontEndSprites[fifth + MENUSPRITE_WILDSTYLE].Draw(m_LeftMostRadioX + MENU_X(MENURADIO_ICON_SIZE * 4), MENU_Y(MENURADIO_ICON_Y), MENU_X(MENURADIO_ICON_SIZE), MENU_Y(MENURADIO_ICON_SIZE),
			CRGBA(INACTIVE_RADIO_COLOR.r, INACTIVE_RADIO_COLOR.g, INACTIVE_RADIO_COLOR.b, FadeIn(INACTIVE_RADIO_COLOR.a)));
	}

	// Middle one(third) is colored differently depending on if it's in transition.
	// If not in transition then this icon indicates selected radio, and should be on top of all icons. thus drawn last
	if (m_ScrollRadioBy != 0) {
		m_aFrontEndSprites[third + MENUSPRITE_WILDSTYLE].Draw(m_LeftMostRadioX + MENU_X(MENURADIO_ICON_SIZE * 2), MENU_Y(MENURADIO_ICON_Y), MENU_X(MENURADIO_ICON_SIZE), MENU_Y(MENURADIO_ICON_SIZE),
			CRGBA(INACTIVE_RADIO_COLOR.r, INACTIVE_RADIO_COLOR.g, INACTIVE_RADIO_COLOR.b, FadeIn(INACTIVE_RADIO_COLOR.a)));		
	} else {
		m_aFrontEndSprites[third + MENUSPRITE_WILDSTYLE].Draw(m_LeftMostRadioX + MENU_X(MENURADIO_ICON_SIZE * 2 - 10.f), MENU_Y(MENURADIO_ICON_Y - 10.f), MENU_X(MENURADIO_ICON_SIZE) + MENU_X(20.f), MENU_Y(MENURADIO_ICON_SIZE) + MENU_Y(20.f),
			CRGBA(255, 255, 255, FadeIn(255)));
	}
#endif

	static bool radioChangeRequested = false;
	static PauseModeTime lastScrollCheck = 0;
	if (CTimer::GetTimeInMillisecondsPauseMode() - lastScrollCheck > 17) {
		if (m_ScrollRadioBy == 1) {
			if (m_LeftMostRadioX > MENU_X_LEFT_ALIGNED(MENURADIO_ICON_FIRST_X - MENURADIO_ICON_SIZE)) {
				m_LeftMostRadioX -= MENU_X(6.f);
			} else {
				m_ScrollRadioBy = 0;
				lastRadioChange = CTimer::GetTimeInMillisecondsPauseMode();
				radioChangeRequested = true;
			}
		}
		if (m_ScrollRadioBy == -1) {
			if (m_LeftMostRadioX < MENU_X_LEFT_ALIGNED(MENURADIO_ICON_FIRST_X - MENURADIO_ICON_SIZE)) {
				m_LeftMostRadioX += MENU_X(6.f);
			} else {
				m_ScrollRadioBy = 0;
				lastRadioChange = CTimer::GetTimeInMillisecondsPauseMode();
				radioChangeRequested = true;
			}
		}
		lastScrollCheck = CTimer::GetTimeInMillisecondsPauseMode();
	}
	// Background behind arrows
	CSprite2d::Draw2DPolygon(MENU_X_LEFT_ALIGNED(228.f), MENU_Y(MENURADIO_SELECTOR_START_Y + MENURADIO_SELECTOR_HEIGHT),
		MENU_X_LEFT_ALIGNED(168.f), MENU_Y(MENURADIO_SELECTOR_START_Y + MENURADIO_SELECTOR_HEIGHT),
		MENU_X_LEFT_ALIGNED(238.f), MENU_Y(MENURADIO_SELECTOR_START_Y),
		MENU_X_LEFT_ALIGNED(178.f), MENU_Y(MENURADIO_SELECTOR_START_Y),
		CRGBA(RADIO_SELECTOR_COLOR.r, RADIO_SELECTOR_COLOR.g, RADIO_SELECTOR_COLOR.b, FadeIn(255)));
	
	CSprite2d::Draw2DPolygon(MENU_X_LEFT_ALIGNED(478.f), MENU_Y(MENURADIO_SELECTOR_START_Y + MENURADIO_SELECTOR_HEIGHT),
		MENU_X_LEFT_ALIGNED(418.f), MENU_Y(MENURADIO_SELECTOR_START_Y + MENURADIO_SELECTOR_HEIGHT),
		MENU_X_LEFT_ALIGNED(488.f), MENU_Y(MENURADIO_SELECTOR_START_Y),
		MENU_X_LEFT_ALIGNED(428.f), MENU_Y(MENURADIO_SELECTOR_START_Y),
		CRGBA(RADIO_SELECTOR_COLOR.r, RADIO_SELECTOR_COLOR.g, RADIO_SELECTOR_COLOR.b, FadeIn(255)));
	
	// Arrows and their shadows
	CSprite2d::Draw2DPolygon(MENU_X_LEFT_ALIGNED(216.f), MENU_Y(MENURADIO_SELECTOR_START_Y + 48.f), MENU_X_LEFT_ALIGNED(196.f), MENU_Y(MENURADIO_SELECTOR_START_Y + 36.f), MENU_X_LEFT_ALIGNED(216.f), MENU_Y(MENURADIO_SELECTOR_START_Y + 22.f), MENU_X_LEFT_ALIGNED(196.f), MENU_Y(MENURADIO_SELECTOR_START_Y + 36.f), CRGBA(0, 0, 0, FadeIn(255)));
	CSprite2d::Draw2DPolygon(MENU_X_LEFT_ALIGNED(213.f), MENU_Y(MENURADIO_SELECTOR_START_Y + 45.f), MENU_X_LEFT_ALIGNED(193.f), MENU_Y(MENURADIO_SELECTOR_START_Y + 33.f), MENU_X_LEFT_ALIGNED(213.f), MENU_Y(MENURADIO_SELECTOR_START_Y + 19.f), MENU_X_LEFT_ALIGNED(193.f), MENU_Y(MENURADIO_SELECTOR_START_Y + 33.f), CRGBA(97, 194, 247, FadeIn(255)));
	CSprite2d::Draw2DPolygon(MENU_X_LEFT_ALIGNED(440.f), MENU_Y(MENURADIO_SELECTOR_START_Y + 48.f), MENU_X_LEFT_ALIGNED(460.f), MENU_Y(MENURADIO_SELECTOR_START_Y + 36.f), MENU_X_LEFT_ALIGNED(440.f), MENU_Y(MENURADIO_SELECTOR_START_Y + 22.f), MENU_X_LEFT_ALIGNED(460.f), MENU_Y(MENURADIO_SELECTOR_START_Y + 36.f), CRGBA(0, 0, 0, FadeIn(255)));
	CSprite2d::Draw2DPolygon(MENU_X_LEFT_ALIGNED(443.f), MENU_Y(MENURADIO_SELECTOR_START_Y + 45.f), MENU_X_LEFT_ALIGNED(463.f), MENU_Y(MENURADIO_SELECTOR_START_Y + 33.f), MENU_X_LEFT_ALIGNED(443.f), MENU_Y(MENURADIO_SELECTOR_START_Y + 19.f), MENU_X_LEFT_ALIGNED(463.f), MENU_Y(MENURADIO_SELECTOR_START_Y + 33.f), CRGBA(97, 194, 247, FadeIn(255)));
	if (radioChangeRequested) {
		if (CTimer::GetTimeInMillisecondsPauseMode() - lastRadioChange > 50) {
			DMAudio.SetRadioInCar(m_PrefsRadioStation);
			DMAudio.PlayFrontEndTrack(m_PrefsRadioStation, 1);
			OutputDebugString("FRONTEND RADIO STATION CHANGED");
			lastRadioChange = CTimer::GetTimeInMillisecondsPauseMode();
			radioChangeRequested = false;
		}
	}
}

// Original name is unknown
void
CMenuManager::ProcessList(bool &optionSelected, bool &goBack)
{
	if (m_bWaitingForNewKeyBind)
		return;

	if (m_nCurrScreen == MENUPAGE_SKIN_SELECT) {
		m_nTotalListRow = m_nSkinsTotal;
	}
	if (m_nCurrScreen == MENUPAGE_KEYBOARD_CONTROLS) {
		// GetNumOptionsCntrlConfigScreens would have been a better choice
		m_nTotalListRow = m_ControlMethod == CONTROL_CLASSIC ? 32 : 27;
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
		DMAudio.PlayFrontEndSound(SOUND_FRONTEND_HIGHLIGHT_OPTION, 0);
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
			DMAudio.PlayFrontEndSound(SOUND_FRONTEND_HIGHLIGHT_OPTION, 0);
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
			DMAudio.PlayFrontEndSound(SOUND_FRONTEND_HIGHLIGHT_OPTION, 0);
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
				DMAudio.PlayFrontEndSound(SOUND_FRONTEND_HIGHLIGHT_OPTION, 0);
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
				DMAudio.PlayFrontEndSound(SOUND_FRONTEND_HIGHLIGHT_OPTION, 0);
				PageDownList(false);
			}
		}
		if (CPad::GetPad(0)->GetHome()) {
			m_nCurrExLayer = HOVEROPTION_LIST;
			m_bShowMouse = false;
			DMAudio.PlayFrontEndSound(SOUND_FRONTEND_HIGHLIGHT_OPTION, 0);
			if (m_nTotalListRow >= MAX_VISIBLE_OPTION_ON_SCREEN) {
				m_nFirstVisibleRowOnList = 0;
			}
			m_nSelectedListRow = 0;
			m_nScrollbarTopMargin = (SCROLLBAR_MAX_HEIGHT / m_nTotalListRow) * m_nFirstVisibleRowOnList;
		}
		if (CPad::GetPad(0)->GetEnd()) {
			m_nCurrExLayer = HOVEROPTION_LIST;
			m_bShowMouse = false;
			DMAudio.PlayFrontEndSound(SOUND_FRONTEND_HIGHLIGHT_OPTION, 0);
			if (m_nTotalListRow >= MAX_VISIBLE_OPTION_ON_SCREEN) {
				m_nFirstVisibleRowOnList = m_nTotalListRow - MAX_VISIBLE_OPTION_ON_SCREEN;
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

void
CMenuManager::UserInput(void)
{
	bool goBack = false;
	bool optionSelected = false;
	bool goUp = false;
	bool goDown = false;
	int8 changeValueBy;

	if (!m_AllowNavigation && m_firstStartCounter == 255)
		m_AllowNavigation = true;
	if (!m_bShowMouse && m_nCurrScreen != MENUPAGE_MAP && (m_nMouseOldPosX != m_nMousePosX || m_nMouseOldPosY != m_nMousePosY)) {
		m_bShowMouse = true;
	}

	static int oldOption = -99;
	oldOption = m_nCurrOption;
#ifdef SCROLLABLE_PAGES
	int firstOption = SCREEN_HAS_AUTO_SCROLLBAR ? m_nFirstVisibleRowOnList : 0;
	int scrollOffset = aScreens[m_nCurrScreen].m_aEntries[firstOption].m_Y - aScreens[m_nCurrScreen].m_aEntries[0].m_Y;
	for (int rowToCheck = firstOption; rowToCheck < firstOption + MAX_VISIBLE_OPTION && rowToCheck < NUM_MENUROWS; ++rowToCheck) {
#else
	for (int rowToCheck = 0; rowToCheck < NUM_MENUROWS; ++rowToCheck) {
#endif
		if (aScreens[m_nCurrScreen].m_aEntries[rowToCheck].m_Action == MENUACTION_NOTHING || 
			aScreens[m_nCurrScreen].m_aEntries[rowToCheck].m_Action == MENUACTION_LABEL)
			continue;

		// unused: CFont::GetStringWidth(TheText.Get(aScreens[m_nCurrScreen].m_aEntries[rowToCheck].m_EntryName), true);
		// So they also wanted the compare X, but they abandoned the idea later on

		if (m_nMousePosY > MENU_Y(aScreens[m_nCurrScreen].m_aEntries[rowToCheck].m_Y MINUS_SCROLL_OFFSET) &&
			m_nMousePosY < MENU_Y(aScreens[m_nCurrScreen].m_aEntries[rowToCheck].m_Y MINUS_SCROLL_OFFSET PLUS_LINE_HEIGHT_ON_SCREEN)) {
			static int oldScreen = m_nCurrScreen;

			m_nOptionMouseHovering = rowToCheck;
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
			m_nOptionMouseHovering = oldOption;
			m_nCurrOption = oldOption;
		}
	}

	if (m_bShowMouse) {
		if (oldOption != m_nCurrOption) {
			if (aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption].m_Action == MENUACTION_LABEL) {
				++m_nCurrOption;
				++m_nOptionMouseHovering;
			}
			m_nOptionHighlightTransitionBlend = 0;
			DMAudio.PlayFrontEndSound(SOUND_FRONTEND_HIGHLIGHT_OPTION, 0);
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

	changeValueBy = 0;
	if (hasNativeList(m_nCurrScreen)) {
		ProcessList(optionSelected, goBack);
	} else {
		AdditionalOptionInput(goBack);

		if (m_AllowNavigation &&
			(CPad::GetPad(0)->GetDownJustDown() || CPad::GetPad(0)->GetAnaloguePadDown() || CPad::GetPad(0)->GetDPadDownJustDown())) {
			m_bShowMouse = false;
			goDown = true;
			m_nOptionHighlightTransitionBlend = 0;

		} else if (m_AllowNavigation &&
			(CPad::GetPad(0)->GetUpJustDown() || CPad::GetPad(0)->GetAnaloguePadUp() || CPad::GetPad(0)->GetDPadUpJustDown())) {
			m_bShowMouse = false;
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

		if (CPad::GetPad(0)->GetLeftMouseJustUp() && m_nCurrScreen != MENUPAGE_MAP) {
			if (m_nHoverOption == HOVEROPTION_RANDOM_ITEM)
				optionSelected = true;
			else if (m_nHoverOption == HOVEROPTION_NEXT_RADIO)
				ChangeRadioStation(1);
			else if (m_nHoverOption == HOVEROPTION_PREV_RADIO)
				ChangeRadioStation(-1);
		}

		if (CPad::GetPad(0)->GetLeftMouse()) {
			switch (m_nHoverOption) {
			case HOVEROPTION_INCREASE_BRIGHTNESS:
			case HOVEROPTION_INCREASE_MP3BOOST:
			case HOVEROPTION_INCREASE_DRAWDIST:
			case HOVEROPTION_INCREASE_MUSICVOLUME:
			case HOVEROPTION_INCREASE_SFXVOLUME:
			case HOVEROPTION_INCREASE_MOUSESENS:
				CheckSliderMovement(1);
				break;
			case HOVEROPTION_DECREASE_BRIGHTNESS:
			case HOVEROPTION_DECREASE_MP3BOOST:
			case HOVEROPTION_DECREASE_DRAWDIST:
			case HOVEROPTION_DECREASE_MUSICVOLUME:
			case HOVEROPTION_DECREASE_SFXVOLUME:
			case HOVEROPTION_DECREASE_MOUSESENS:
				CheckSliderMovement(-1);
				break;
			}
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
	
			if (oldOption != m_nCurrOption)
				m_nOptionHighlightTransitionBlend = 0;
		}
		
		// Prevent sound on scroll. Mouse wheel is now belongs to us!
		if (!(m_nTotalListRow > MAX_VISIBLE_OPTION && (CPad::GetPad(0)->GetMouseWheelUpJustDown() || CPad::GetPad(0)->GetMouseWheelDownJustDown())))
#endif
		if (CPad::GetPad(0)->GetLeftMouseJustUp() || CPad::GetPad(0)->GetLeftJustUp() || CPad::GetPad(0)->GetRightJustUp()
			|| CPad::GetPad(0)->GetDPadLeftJustUp() || CPad::GetPad(0)->GetDPadRightJustUp()
			|| CPad::GetPad(0)->GetAnaloguePadLeftJustUp() || CPad::GetPad(0)->GetAnaloguePadRightJustUp()
			|| CPad::GetPad(0)->GetMouseWheelUpJustDown() || CPad::GetPad(0)->GetMouseWheelDownJustDown()) {
			int option = aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption].m_Action;
			if (option == MENUACTION_BRIGHTNESS)
				DMAudio.PlayFrontEndSound(SOUND_FRONTEND_ENTER_OR_ADJUST, 0);
			else if (option == MENUACTION_SFXVOLUME)
				DMAudio.PlayFrontEndSound(SOUND_FRONTEND_AUDIO_TEST, 0);
			else if (option == MENUACTION_DRAWDIST || option == MENUACTION_MOUSESENS)
				DMAudio.PlayFrontEndSound(SOUND_FRONTEND_ENTER_OR_ADJUST, 0);

		}
		if (CPad::GetPad(0)->GetBackJustDown() || CPad::GetPad(0)->GetEscapeJustDown()) {
			if (m_nCurrScreen != MENUPAGE_START_MENU && m_nCurrScreen != MENUPAGE_PAUSE_MENU && m_nCurrScreen != MENUPAGE_CHOOSE_SAVE_SLOT
				&& m_nCurrScreen != MENUPAGE_SAVE_CHEAT_WARNING && m_nCurrScreen != MENUPAGE_SAVING_IN_PROGRESS
				&& m_nCurrScreen != MENUPAGE_DELETING_IN_PROGRESS && m_nCurrScreen != MENUPAGE_OUTRO)
			{
				m_bShowMouse = false;
				goBack = true;
			}
		}

		if (((goDown) || (goUp)) || (optionSelected)) {
			goBack = false;
		}

	}

	int curAction = aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption].m_Action;
	if (CPad::GetPad(0)->GetLeft() || CPad::GetPad(0)->GetPedWalkLeftRight() < 0 || CPad::GetPad(0)->GetDPadLeft()) {
		static PauseModeTime lastSliderDecrease = 0;
		if (CTimer::GetTimeInMillisecondsPauseMode() - lastSliderDecrease > 150) {
			if (curAction == MENUACTION_BRIGHTNESS || curAction == MENUACTION_MUSICVOLUME ||
				curAction == MENUACTION_SFXVOLUME || curAction == MENUACTION_RADIO ||
				curAction == MENUACTION_DRAWDIST || curAction == MENUACTION_MOUSESENS ||
				curAction == MENUACTION_MP3VOLUMEBOOST)
				changeValueBy = -1;

			lastSliderDecrease = CTimer::GetTimeInMillisecondsPauseMode();
		}
	} else if (CPad::GetPad(0)->GetRight() || CPad::GetPad(0)->GetPedWalkLeftRight() > 0 || CPad::GetPad(0)->GetDPadRight()) {
		static PauseModeTime lastSliderIncrease = 0;
		if (CTimer::GetTimeInMillisecondsPauseMode() - lastSliderIncrease > 150) {
			if (curAction == MENUACTION_BRIGHTNESS || curAction == MENUACTION_MUSICVOLUME ||
				curAction == MENUACTION_SFXVOLUME || curAction == MENUACTION_RADIO ||
				curAction == MENUACTION_DRAWDIST || curAction == MENUACTION_MOUSESENS ||
				curAction == MENUACTION_MP3VOLUMEBOOST)
				changeValueBy = 1;
			lastSliderIncrease = CTimer::GetTimeInMillisecondsPauseMode();
		}
	}

#ifdef SCROLLABLE_PAGES
	if (!SCREEN_HAS_AUTO_SCROLLBAR)
#endif
	{
		if (CPad::GetPad(0)->GetMouseWheelUpJustDown()) {
			changeValueBy = 1;
		} else if (CPad::GetPad(0)->GetMouseWheelDownJustDown()) {
			changeValueBy = -1;
		}
	}
	
	if (m_AllowNavigation) {
		if (CPad::GetPad(0)->GetRightJustDown() || CPad::GetPad(0)->GetAnaloguePadRight() || CPad::GetPad(0)->GetDPadRightJustDown()) {
			m_bShowMouse = false;
			changeValueBy = 1;
		}
	}

	if (m_AllowNavigation) {
		if (CPad::GetPad(0)->GetLeftJustDown() || CPad::GetPad(0)->GetAnaloguePadLeft() || CPad::GetPad(0)->GetDPadLeftJustDown()) {
			m_bShowMouse = false;
			changeValueBy = -1;
		}
	}
	if (changeValueBy != 0) {
		if ((m_nCurrScreen == MENUPAGE_SOUND_SETTINGS || m_nCurrScreen == MENUPAGE_DISPLAY_SETTINGS || m_nCurrScreen == MENUPAGE_CONTROLLER_PC || m_nCurrScreen == MENUPAGE_MOUSE_CONTROLS)
			&& aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption].m_Action != MENUACTION_NOTHING
			&& aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption].m_Action != MENUACTION_LABEL
			&& aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption].m_Action != MENUACTION_YES
			&& aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption].m_Action != MENUACTION_NO
			&& aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption].m_Action != MENUACTION_CHANGEMENU
			&& aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption].m_Action != MENUACTION_KEYBOARDCTRLS
			&& aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption].m_Action != MENUACTION_GOBACK
			&& aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption].m_Action != MENUACTION_RESTOREDEF
			&& aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption].m_Action != MENUACTION_DRAWDIST
			&& aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption].m_Action != MENUACTION_MOUSESENS
			&& aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption].m_Action != MENUACTION_MP3VOLUMEBOOST) {
			DMAudio.PlayFrontEndSound(SOUND_FRONTEND_ENTER_OR_ADJUST, 0);
		}
	}
	ProcessUserInput(goDown, goUp, optionSelected, goBack, changeValueBy);
#ifdef CUSTOM_FRONTEND_OPTIONS
	if (aScreens[m_nCurrScreen].m_aEntries[oldOption].m_Action < MENUACTION_NOTHING) {  // CFO check
		CMenuScreenCustom::CMenuEntry &oldEntry = aScreens[m_nCurrScreen].m_aEntries[oldOption];
		if (m_nCurrOption != oldOption) {
			if (oldEntry.m_Action == MENUACTION_CFO_DYNAMIC)
				if(oldEntry.m_CFODynamic->buttonPressFunc)
					oldEntry.m_CFODynamic->buttonPressFunc(FEOPTION_ACTION_FOCUSLOSS);

			if (oldEntry.m_Action == MENUACTION_CFO_SELECT && oldEntry.m_CFOSelect->onlyApplyOnEnter) {
				if (oldEntry.m_CFOSelect->displayedValue != oldEntry.m_CFOSelect->lastSavedValue)
					SetHelperText(3); // Restored original value

				oldEntry.m_CFOSelect->displayedValue = oldEntry.m_CFOSelect->lastSavedValue = *oldEntry.m_CFO->value;
			}
		} else if (oldEntry.m_Action == MENUACTION_CFO_SELECT && oldEntry.m_CFOSelect->onlyApplyOnEnter) {
			if (oldEntry.m_CFOSelect->displayedValue != *oldEntry.m_CFO->value)
				SetHelperText(1); // Enter to apply
			else if (m_nHelperTextMsgId == 1)
				ResetHelperText(); // Applied
		}
	}
#endif
}

void
CMenuManager::ProcessUserInput(uint8 goDown, uint8 goUp, uint8 optionSelected, uint8 goBack, int8 changeAmount)
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
			else if (CPad::GetPad(0)->GetMouseX1JustUp())
				MouseButtonJustClicked = 6;
			else if (CPad::GetPad(0)->GetMouseX2JustUp())
				MouseButtonJustClicked = 7;

			JoyButtonJustClicked = ControlsManager.GetJoyButtonJustDown();

			int32 TypeOfControl = KEYBOARD;
			if (JoyButtonJustClicked)
				TypeOfControl = JOYSTICK;
			if (MouseButtonJustClicked)
				TypeOfControl = MOUSE;
			if (*pControlEdit != rsNULL)
				TypeOfControl = KEYBOARD;

			if (!m_bKeyIsOK) {
				pControlEdit = nil;
				m_bWaitingForNewKeyBind = false;
				m_KeyPressedCode = -1;
				m_bStartWaitingForKeyBind = false;
			} else if (!m_bKeyChangeNotProcessed) {
				if (*pControlEdit != rsNULL || MouseButtonJustClicked || JoyButtonJustClicked)
					CheckCodesForControls(TypeOfControl);

				field_159 = true;
			} else {
				for (int i = 0; i < 4; i++)
					ControlsManager.ClearSettingsAssociatedWithAction((e_ControllerAction)m_CurrCntrlAction, (eControllerType)i);
				m_bKeyIsOK = false;
				m_bKeyChangeNotProcessed = false;
			}
		}
	}

	if (pEditString || pControlEdit)
		return;

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

	int oldOption = m_nCurrOption;
	if (goDown) {
		if (m_nCurrScreen != MENUPAGE_MAP)
			DMAudio.PlayFrontEndSound(SOUND_FRONTEND_HIGHLIGHT_OPTION, 0);

		m_nCurrOption++;
		if (m_nCurrOption == NUM_MENUROWS || (aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption].m_Action == MENUACTION_NOTHING)) {
			m_nCurrOption = 0;
		}
		if (oldOption != m_nCurrOption)
			m_nOptionHighlightTransitionBlend = 0;
	}
	if (goUp) {
		if (m_nCurrScreen != MENUPAGE_MAP)
			DMAudio.PlayFrontEndSound(SOUND_FRONTEND_HIGHLIGHT_OPTION, 0);

		if (m_nCurrOption == (aScreens[m_nCurrScreen].m_aEntries[0].m_Action == MENUACTION_LABEL)) {
			while (m_nCurrOption != NUM_MENUROWS - 1
				&& aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption + 1].m_Action != MENUACTION_NOTHING) {
				m_nCurrOption++;
			}
		} else {
			m_nCurrOption--;
		}
		if (oldOption != m_nCurrOption)
			m_nOptionHighlightTransitionBlend = 0;
	}

	if (optionSelected && m_nMenuFadeAlpha == 255) {
		if (aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption].m_TargetMenu == MENUPAGE_NEW_GAME_RELOAD && m_bGameNotLoaded) {
			DoSettingsBeforeStartingAGame();
		} else if (hasNativeList(m_nCurrScreen)) {
			switch (m_nCurrExLayer) {
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
			case HOVEROPTION_BACK:
			default:
				goBack = true;
				break;
			}
			if (m_nSkinsTotal > 0) {
				m_pSelectedSkin = m_pSkinListHead.nextSkin;
				strcpy(m_PrefsSkinFile, m_aSkinName);
				CWorld::Players[0].SetPlayerSkin(m_PrefsSkinFile);
				SaveSettings();
			}
		}

		int option = aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption].m_Action;
#ifdef FIX_BUGS
		int currScreen = m_nCurrScreen;
		int currOption = m_nCurrOption;
#endif
		switch (option) {
			case MENUACTION_CHANGEMENU:
			case MENUACTION_YES:
			case MENUACTION_NO:
				SwitchToNewScreen(aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption].m_TargetMenu);
				break;
			case MENUACTION_RADIO:
				ChangeRadioStation(1);
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
			case MENUACTION_CHECKSAVE:
			{
				int saveSlot = aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption].m_SaveSlot;

				if (saveSlot >= SAVESLOT_1 && saveSlot <= SAVESLOT_8) {
					m_nCurrSaveSlot = saveSlot - SAVESLOT_1;
					if (Slots[m_nCurrSaveSlot] != SLOT_EMPTY && Slots[m_nCurrSaveSlot] != SLOT_CORRUPTED) {
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
#ifdef LEGACY_MENU_OPTIONS
			case MENUACTION_RELOADIDE:
				CFileLoader::ReloadObjectTypes("GTA3.IDE");
				break;
#endif
			case MENUACTION_RESUME_FROM_SAVEZONE:
				RequestFrontEndShutDown();
				break;
			case MENUACTION_LOADRADIO:
				if (m_nPrefsAudio3DProviderIndex != NO_AUDIO_PROVIDER) {
					SwitchToNewScreen(MENUPAGE_SOUND_SETTINGS);
					DMAudio.PlayFrontEndTrack(m_PrefsRadioStation, 1);
					OutputDebugString("STARTED PLAYING FRONTEND AUDIO TRACK");
				}
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
			case MENUACTION_RADARMODE:
				if (++m_PrefsRadarMode > 2)
					m_PrefsRadarMode = 0;
				SaveSettings();
				break;
			case MENUACTION_GOBACK:
				goBack = true;
				break;
			case MENUACTION_KEYBOARDCTRLS:
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
#ifdef LEGACY_MENU_OPTIONS
				if (m_PrefsVsyncDisp != m_PrefsVsync) {
					m_PrefsVsync = m_PrefsVsyncDisp;
				}
#endif
				RequestFrontEndShutDown();
				break;
			case MENUACTION_DONTCANCEL:
				SwitchToNewScreen(-2);
				break;
			case MENUACTION_SCREENRES:
				if (m_nDisplayVideoMode != m_nPrefsVideoMode) {
					m_nPrefsVideoMode = m_nDisplayVideoMode;
					_psSelectScreenVM(m_nPrefsVideoMode);
					DMAudio.ChangeMusicMode(MUSICMODE_FRONTEND);
					DMAudio.Service();
					CentreMousePointer();
					m_bShowMouse = true;
					m_nCurrOption = 5; // TODO(Miami): Because selected option is resetted after res. change. We'll need to revisit that.
					m_nOptionHighlightTransitionBlend = 0;
					SaveSettings();
				}
				break;
			case MENUACTION_AUDIOHW:
			{
				int selectedProvider = m_nPrefsAudio3DProviderIndex;
				if (selectedProvider != NO_AUDIO_PROVIDER) {
					if (selectedProvider == -1)
						selectedProvider = m_nPrefsAudio3DProviderIndex = DMAudio.AutoDetect3DProviders();

					m_nPrefsAudio3DProviderIndex = DMAudio.SetCurrent3DProvider(m_nPrefsAudio3DProviderIndex);
					if (selectedProvider != m_nPrefsAudio3DProviderIndex) {
						SetHelperText(5);
					}
					SaveSettings();
				}
				break;
			}
			case MENUACTION_SPEAKERCONF:
				if (m_nPrefsAudio3DProviderIndex != NO_AUDIO_PROVIDER) {
					if (--m_PrefsSpeakers < 0)
						m_PrefsSpeakers = 2;
					DMAudio.SetSpeakerConfig(m_PrefsSpeakers);
					SaveSettings();
				}
				break;
			case MENUACTION_PLAYERSETUP:
				CPlayerSkin::BeginFrontendSkinEdit();
				SwitchToNewScreen(MENUPAGE_SKIN_SELECT);
				m_bSkinsEnumerated = false;
				m_nCurrExLayer = HOVEROPTION_LIST;
				break;
			case MENUACTION_RESTOREDEF:
				if (m_nCurrScreen == MENUPAGE_SOUND_SETTINGS) {
					m_nPrefsAudio3DProviderIndex = DMAudio.AutoDetect3DProviders();
					DMAudio.SetCurrent3DProvider(m_nPrefsAudio3DProviderIndex);
					m_PrefsSfxVolume = 49;
					m_PrefsMusicVolume = 49;
					m_PrefsRadioStation = EMOTION;
					m_PrefsMP3BoostVolume = 0;
					m_PrefsStereoMono = 1;
					m_PrefsSpeakers = 0;
					DMAudio.SetMP3BoostVolume(m_PrefsMP3BoostVolume);
					DMAudio.SetMusicMasterVolume(m_PrefsMusicVolume);
					DMAudio.SetEffectsMasterVolume(m_PrefsSfxVolume);
					DMAudio.SetRadioInCar(m_PrefsRadioStation);
					DMAudio.PlayFrontEndTrack(m_PrefsRadioStation, 1);
					SaveSettings();
				} else if (m_nCurrScreen == MENUPAGE_DISPLAY_SETTINGS) {
					m_PrefsBrightness = 256;
					m_PrefsLOD = 1.2f;
#ifdef LEGACY_MENU_OPTIONS
					m_PrefsVsync = true;
#endif
					CRenderer::ms_lodDistScale = m_PrefsLOD;
					m_PrefsShowSubtitles = false;
#ifdef ASPECT_RATIO_SCALE
					m_PrefsUseWideScreen = AR_AUTO;
#else
					m_PrefsUseWideScreen = false;
#endif
					m_PrefsShowLegends = true;
					m_PrefsVsyncDisp = true;
					m_PrefsFrameLimiter = true;
					m_PrefsRadarMode = 0;
					m_PrefsShowHud = true;
					m_nDisplayVideoMode = m_nPrefsVideoMode;
					CMBlur::BlurOn = false;
#ifdef CUSTOM_FRONTEND_OPTIONS
					extern void RestoreDefGraphics(int8);
					extern void RestoreDefDisplay(int8);

					RestoreDefGraphics(FEOPTION_ACTION_SELECT);
					RestoreDefDisplay(FEOPTION_ACTION_SELECT);
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
					MousePointerStateHelper.bInvertVertically = true;
					TheCamera.m_bHeadBob = false;
#ifdef FIX_BUGS
					TheCamera.m_fMouseAccelVertical = 0.003f;
#endif
					TheCamera.m_fMouseAccelHorzntl = 0.0025f;
					CVehicle::m_bDisableMouseSteering = true;
					m_ControlMethod = CONTROL_STANDARD;
					TheCamera.m_bUseMouse3rdPerson = true;
					SaveSettings();
				}
				SetHelperText(2);
				break;
			case MENUACTION_CTRLMETHOD:
				if (m_ControlMethod == CONTROL_CLASSIC) {
					CCamera::m_bUseMouse3rdPerson = true;
					m_ControlMethod = CONTROL_STANDARD;
				} else {
					CCamera::m_bUseMouse3rdPerson = false;
					m_ControlMethod = CONTROL_CLASSIC;
				}
				SaveSettings();
				break;
#ifdef CUSTOM_FRONTEND_OPTIONS
			case MENUACTION_CFO_SELECT:
			case MENUACTION_CFO_DYNAMIC:
				CMenuScreenCustom::CMenuEntry &option = aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption];
				if (option.m_Action == MENUACTION_CFO_SELECT) {
					if (!option.m_CFOSelect->onlyApplyOnEnter) {
						option.m_CFOSelect->displayedValue++;
						if (option.m_CFOSelect->displayedValue >= option.m_CFOSelect->numRightTexts || option.m_CFOSelect->displayedValue < 0)
							option.m_CFOSelect->displayedValue = 0;
					}
					int8 oldValue = *option.m_CFO->value;

					*option.m_CFO->value = option.m_CFOSelect->lastSavedValue = option.m_CFOSelect->displayedValue;

					if (option.m_CFOSelect->save)
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
		ProcessOnOffMenuOptions();
		if (!goBack) {
#ifdef FIX_BUGS
			int saveSlot = aScreens[currScreen].m_aEntries[currOption].m_SaveSlot;
			if (saveSlot >= SAVESLOT_1 && saveSlot <= SAVESLOT_8 && Slots[currOption] != SLOT_OK)
#else
			int saveSlot = aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption].m_SaveSlot;
			if (saveSlot >= SAVESLOT_1 && saveSlot <= SAVESLOT_8 && Slots[m_nCurrOption] != SLOT_OK)
#endif
				DMAudio.PlayFrontEndSound(SOUND_FRONTEND_FAIL, 0);
			else
				DMAudio.PlayFrontEndSound(SOUND_FRONTEND_ENTER_OR_ADJUST, 0);
		}
	}

	if (goBack) {
		if (m_NoEmptyBinding) {
			DMAudio.PlayFrontEndSound(SOUND_FRONTEND_BACK, 0);
			SwitchToNewScreen(-2);
			if (hasNativeList(m_nCurrScreen)) {
				m_nTotalListRow = 0;
			}
		} else {
			DMAudio.PlayFrontEndSound(SOUND_FRONTEND_FAIL, 0);
			m_ShowEmptyBindingError = true;
		}
	}

	if (changeAmount != 0) {
		switch (aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption].m_Action) {
			case MENUACTION_RADIO:
				ChangeRadioStation(changeAmount);
				break;
			case MENUACTION_RADARMODE:
				m_PrefsRadarMode += changeAmount;
				if (m_PrefsRadarMode < 0)
					m_PrefsRadarMode = 2;
				if (m_PrefsRadarMode > 2)
					m_PrefsRadarMode = 0;
				break;
#ifdef ASPECT_RATIO_SCALE
			case MENUACTION_WIDESCREEN:
				if (changeAmount > 0) {
					m_PrefsUseWideScreen++;
					if (m_PrefsUseWideScreen > 2)
						m_PrefsUseWideScreen = 0;
				} else {
					m_PrefsUseWideScreen--;
					if (m_PrefsUseWideScreen < 0)
						m_PrefsUseWideScreen = 2;
				}
				SaveSettings();
				break;
#endif
			case MENUACTION_SCREENRES:
				if (m_bGameNotLoaded) {
					RwChar** videoMods = _psGetVideoModeList();
					if (changeAmount > 0) {
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
				if (m_nPrefsAudio3DProviderIndex != NO_AUDIO_PROVIDER) {
					m_nPrefsAudio3DProviderIndex += changeAmount;

					bool checkIfForbidden = true;
					while (checkIfForbidden) {
						checkIfForbidden = false;

						if (m_nPrefsAudio3DProviderIndex < -1)
							m_nPrefsAudio3DProviderIndex = DMAudio.GetNum3DProvidersAvailable() - 1;
						else if (m_nPrefsAudio3DProviderIndex > DMAudio.GetNum3DProvidersAvailable() - 1)
							m_nPrefsAudio3DProviderIndex = -1;

						// what a retarded move...
						if (m_nPrefsAudio3DProviderIndex != -1) {
							char* provider = DMAudio.Get3DProviderName(m_nPrefsAudio3DProviderIndex);
							strupr(provider);
							if (!strcmp(provider, "MILES FAST 2D POSITIONAL AUDIO")) {
								m_nPrefsAudio3DProviderIndex += changeAmount;
								checkIfForbidden = true;

							} else if (!strcmp(provider, "AUREAL A3D 2.0 (TM)")) {
								m_nPrefsAudio3DProviderIndex += changeAmount;
								checkIfForbidden = true;

							} else if (!strcmp(provider, "AUREAL A3D INTERACTIVE (TM)")) {
								m_nPrefsAudio3DProviderIndex += changeAmount;
								checkIfForbidden = true;
							}
						}
					}
				}
				break;
			case MENUACTION_SPEAKERCONF:
				if (m_nPrefsAudio3DProviderIndex != NO_AUDIO_PROVIDER) {
					m_PrefsSpeakers -= changeAmount;
					m_PrefsSpeakers = clamp(m_PrefsSpeakers, 0, 2);
					DMAudio.SetSpeakerConfig(m_PrefsSpeakers);
					SaveSettings();
				}
				break;
			case MENUACTION_CTRLMETHOD:
				m_ControlMethod = !m_ControlMethod;
				CCamera::m_bUseMouse3rdPerson = !m_ControlMethod;
				SaveSettings();
				break;
#ifdef CUSTOM_FRONTEND_OPTIONS
			case MENUACTION_CFO_SELECT:
			case MENUACTION_CFO_DYNAMIC:
				CMenuScreenCustom::CMenuEntry &option = aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption];
				if (option.m_Action == MENUACTION_CFO_SELECT) {
					if (changeAmount > 0) {
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

						if (option.m_CFOSelect->save)
							SaveSettings();

						if (option.m_CFOSelect->displayedValue != oldValue && option.m_CFOSelect->changeFunc)
							option.m_CFOSelect->changeFunc(oldValue, option.m_CFOSelect->displayedValue);
					}
				} else if (option.m_Action == MENUACTION_CFO_DYNAMIC && option.m_CFODynamic->buttonPressFunc) {
					option.m_CFODynamic->buttonPressFunc(changeAmount > 0 ? FEOPTION_ACTION_RIGHT : FEOPTION_ACTION_LEFT);
				}
				break;
#endif
		}
		CheckSliderMovement(changeAmount);
		ProcessOnOffMenuOptions();
		if (m_nCurrScreen == MENUPAGE_KEYBOARD_CONTROLS) {
			if (changeAmount < 1) {
				m_nSelectedContSetupColumn = CONTSETUP_PED_COLUMN;
			} else {
				m_nSelectedContSetupColumn = CONTSETUP_VEHICLE_COLUMN;
			}
		}
	}
}

void
CMenuManager::ProcessOnOffMenuOptions()
{
	switch (aScreens[m_nCurrScreen].m_aEntries[m_nCurrOption].m_Action) {
#ifdef LEGACY_MENU_OPTIONS
	case MENUACTION_CTRLVIBRATION:
		m_PrefsUseVibration = !m_PrefsUseVibration;
		break;
	case MENUACTION_CTRLCONFIG:
		CPad::GetPad(0)->Mode++;
		if (CPad::GetPad(0)->Mode > 3)
			CPad::GetPad(0)->Mode = 0;
		break;
#endif
	case MENUACTION_INVERTPADY:
		CPad::bInvertLook4Pad = !CPad::bInvertLook4Pad;
		SaveSettings(); // FIX: Why don't SaveSettings? Because of it's an hidden option? :(
		break;
	case MENUACTION_CTRLDISPLAY:
		m_DisplayControllerOnFoot = !m_DisplayControllerOnFoot;
		break;
	case MENUACTION_FRAMESYNC:
		m_PrefsVsyncDisp = !m_PrefsVsyncDisp;
		SaveSettings(); // FIX: Again... This makes me very unhappy
		break;
	case MENUACTION_FRAMELIMIT:
		m_PrefsFrameLimiter = !m_PrefsFrameLimiter;
		SaveSettings();
		break;
	case MENUACTION_TRAILS:
		CMBlur::BlurOn = !CMBlur::BlurOn;
		SaveSettings();
		break;
	case MENUACTION_SUBTITLES:
		m_PrefsShowSubtitles = !m_PrefsShowSubtitles;
		SaveSettings();
		break;
#ifndef ASPECT_RATIO_SCALE
	case MENUACTION_WIDESCREEN:
		m_PrefsUseWideScreen = !m_PrefsUseWideScreen;
		SaveSettings();
		break;
#endif
	case MENUACTION_LEGENDS:
		m_PrefsShowLegends = !m_PrefsShowLegends;
		break;
	case MENUACTION_HUD:
		m_PrefsShowHud = !m_PrefsShowHud;
		SaveSettings();
		break;
#ifdef LEGACY_MENU_OPTIONS
	case MENUACTION_SETDBGFLAG:
		CTheScripts::InvertDebugFlag();
		break;
	case MENUACTION_SWITCHBIGWHITEDEBUGLIGHT:
		gbBigWhiteDebugLightSwitchedOn = !gbBigWhiteDebugLightSwitchedOn;
		break;
	case MENUACTION_COLLISIONPOLYS:
		gbShowCollisionPolys = !gbShowCollisionPolys;
		break;
#endif
	case MENUACTION_SHOWHEADBOB:
		TheCamera.m_bHeadBob = !TheCamera.m_bHeadBob;
		SaveSettings();
		break;
	case MENUACTION_INVVERT:
		MousePointerStateHelper.bInvertVertically = !MousePointerStateHelper.bInvertVertically;
		SaveSettings();
		break;
	case MENUACTION_DYNAMICACOUSTIC:
		m_PrefsDMA = !m_PrefsDMA;
		DMAudio.SetDynamicAcousticModelingStatus(m_PrefsDMA);
		SaveSettings();
		break;
	case MENUACTION_MOUSESTEER:
		if (m_ControlMethod == CONTROL_STANDARD) {
			CVehicle::m_bDisableMouseSteering = !CVehicle::m_bDisableMouseSteering;
			SaveSettings();
		}
		break;
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

#ifdef XBOX_MESSAGE_SCREEN
void
CMenuManager::CloseDialog(void)
{
	// We don't have this on PC GXT :shrug:
	static wchar* gameSaved = AllocUnicode("Game saved successfully!");

	if (m_bSaveWasSuccessful && DialogTextCmp("FESZ_WR")) {
		m_bSaveWasSuccessful = false; // i don't know where XBOX resets that
		m_pDialogText = gameSaved;
		SetDialogTimer(1000);
	    ProcessDialogTimer();
	} else {
		ToggleDialog(false);
	}

}

void
CMenuManager::ProcessDialogTimer(void)
{
	if (!m_bDialogOpen || m_nDialogHideTimer == 0)
		return;

	// Also XBOX has unified time source for in-game/menu, but we don't have that
	if (m_bMenuActive && CTimer::GetTimeInMilliseconds() > m_nDialogHideTimer || !m_bMenuActive && CTimer::GetTimeInMillisecondsPauseMode() > m_nDialogHideTimerPauseMode) {

		// This is originally activePage.funcs->closePage()
		CloseDialog();
	}
}

void
CMenuManager::SetDialogTimer(uint32 timer)
{
	// XBOX iterates some page list(actives?) and then sets timer variable of specified page to specified value. We only have dialog right now.
	// Also XBOX has unified time source for in-game/menu, but we don't have that, thus 2 timer variables...

	m_nDialogHideTimer = CTimer::GetTimeInMilliseconds() + timer;
	m_nDialogHideTimerPauseMode = CTimer::GetTimeInMillisecondsPauseMode() + timer;
}

void
CMenuManager::SetDialogText(const char* key)
{
	// There are many things going around here, idk why
	m_pDialogText = TheText.Get(key);
}

bool
CMenuManager::DialogTextCmp(const char* key)
{
	wchar *value = TheText.Get(key);
	wchar *i = m_pDialogText;
	for (; *i != '\0' && *value != '\0'; i++, value++) {
		if (*i != *value)
			return false;
	}
	return *i == '\0' && *value == '\0';
}

void
CMenuManager::ToggleDialog(bool toggle)
{
	// This originally calls some mysterious function on enable and close CB on disable, along with decreasing some counter. Which is no use for dialog

	// XBOX doesn't do that
	if (toggle)
		m_nDialogHideTimer = 0;

	m_bDialogOpen = toggle;
}

void
DrawDialogBg(float offset, uint8 alpha)
{
	CSprite2d::Draw2DPolygon(SCALE_AND_CENTER_X(84.f + offset), MENU_Y(126.f + offset),
		SCALE_AND_CENTER_X(512.f + offset), MENU_Y(109.f + offset),
		SCALE_AND_CENTER_X(100.f + offset), MENU_Y(303.f + offset),
		SCALE_AND_CENTER_X(474.f + offset), MENU_Y(311.f + offset), CRGBA(107, 193, 236, alpha));
	CSprite2d::Draw2DPolygon(SCALE_AND_CENTER_X(523.f + offset), MENU_Y(108.f + offset),
		SCALE_AND_CENTER_X(542.f + offset), MENU_Y(107.f + offset),
		SCALE_AND_CENTER_X(485.f + offset), MENU_Y(310.f + offset),
		SCALE_AND_CENTER_X(516.f + offset), MENU_Y(311.f + offset), CRGBA(107, 193, 236, alpha));
}

void
CMenuManager::DrawOverlays(void)
{
	// This is stripped to show only Dialog box, XBOX does much more in here.

	if (!m_bDialogOpen)
		return;

	DefinedState();

	CSprite2d::DrawRect(CRect(0, SCREEN_HEIGHT, SCREEN_WIDTH, 0), CRGBA(0, 0, 0, 160));

	// Ofc this is not hardcoded like that on Xbox, it should be a texture
	DrawDialogBg(20.f, 160); // shadow
	DrawDialogBg(0.f, 255);

	CFont::SetBackgroundOff();
	CFont::SetPropOn();
	CFont::SetJustifyOn();
	CFont::SetBackGroundOnlyTextOn();
	CFont::SetFontStyle(FONT_LOCALE(FONT_STANDARD));
	CFont::SetCentreSize(SCREEN_SCALE_X(380.0f));
	CFont::SetCentreOn();
	CFont::SetColor(CRGBA(LABEL_COLOR.r, LABEL_COLOR.g, LABEL_COLOR.b, 255));
	CFont::SetDropShadowPosition(2);
	CFont::SetDropColor(CRGBA(0, 0, 0, 255));
	// Both of those are 0.9 on Xbox, which is ofcouse wrong...
	CFont::SetScale(SCREEN_SCALE_X(BIGTEXT_X_SCALE), SCREEN_SCALE_Y(BIGTEXT_Y_SCALE));
	
	int x = SCREEN_WIDTH / 2.f - SCREEN_SCALE_X(30.0f);
	int y = SCREEN_HEIGHT / 2.f - SCREEN_SCALE_Y(30.0f);
	int numOfLines = CFont::GetNumberLines(x, y, m_pDialogText);
	CFont::PrintString(x, y - SCREEN_SCALE_Y(numOfLines / 2.f), m_pDialogText);
	CFont::DrawFonts();
}
#endif

void
CMenuManager::ProcessFileActions()
{
	switch (m_nCurrScreen) {
		case MENUPAGE_LOADING_IN_PROGRESS:
			if (CheckSlotDataValid(m_nCurrSaveSlot)) {
#ifdef USE_DEBUG_SCRIPT_LOADER
				scriptToLoad = 0;
#endif

#ifdef XBOX_MESSAGE_SCREEN
				SetDialogText("FELD_WR");
				ToggleDialog(true);
#else
				if (!m_bGameNotLoaded)
					MessageScreen("FELD_WR", true);
#endif
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
#ifdef XBOX_MESSAGE_SCREEN
			if (m_bDialogOpen && DialogTextCmp("FESZ_WR")) {
				PauseModeTime startTime = CTimer::GetTimeInMillisecondsPauseMode();
				int8 SaveSlot = PcSaveHelper.SaveSlot(m_nCurrSaveSlot);
				PcSaveHelper.PopulateSlotInfo();

				// Original code, but we don't want redundant saving text if it doesn't
#if 0
				CTimer::Update(); // not on Xbox, who updates it?

				// it compensates the lag to show saving text always one second... how cute
				int dialogDur = Max(1, startTime - CTimer::GetTimeInMillisecondsPauseMode() + 1000);
#else
				int dialogDur = 1;
#endif

				if (SaveSlot) {
					// error. PC code
					ToggleDialog(false);
					SwitchToNewScreen(MENUPAGE_SAVE_CUSTOM_WARNING);
					strncpy(aScreens[m_nCurrScreen].m_ScreenName, "FET_SG", 8);
					strncpy(aScreens[m_nCurrScreen].m_aEntries[0].m_EntryName, "FES_CMP", 8);

				} else {
					m_bSaveWasSuccessful = true;
					SetDialogTimer(dialogDur);
					ProcessDialogTimer();
					RequestFrontEndShutDown();
				}

			} else {
				SetDialogText("FESZ_WR");
				ToggleDialog(true);
			}
#else
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
#endif
			break;
		}
	}
}

void
CMenuManager::SwitchMenuOnAndOff()
{
	if (!TheCamera.m_WideScreenOn) {

		// Reminder: You need REGISTER_START_BUTTON defined to make it work.
		if ((CPad::GetPad(0)->GetStartJustDown() || CPad::GetPad(0)->GetEscapeJustDown())
			&& (!m_bMenuActive || m_nCurrScreen == MENUPAGE_PAUSE_MENU || m_nCurrScreen == MENUPAGE_CHOOSE_SAVE_SLOT || m_nCurrScreen == MENUPAGE_SAVE_CHEAT_WARNING)
			|| m_bShutDownFrontEndRequested || m_bStartUpFrontEndRequested
#ifdef REGISTER_START_BUTTON
			|| CPad::GetPad(0)->GetStartJustDown() && !m_bGameNotLoaded
#endif
			) {

			if (m_nCurrScreen != MENUPAGE_LOADING_IN_PROGRESS
#ifdef XBOX_MESSAGE_SCREEN
				&& m_nCurrScreen != MENUPAGE_SAVING_IN_PROGRESS
#endif
				) {
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
				if (_InputMouseNeedsExclusive()) {
					_InputShutdownMouse();
					_InputInitialiseMouse(false);
				}
				Initialise();
				LoadAllTextures();
			} else {
#ifdef EXTENDED_COLOURFILTER
				// we always expect CPostFX to be open
				CMBlur::BlurOn = true;
#endif
				if (CMBlur::BlurOn)
					CMBlur::MotionBlurOpen(Scene.camera);
				else
					CMBlur::MotionBlurClose();
				DoRWStuffStartOfFrame(0, 0, 0, 0, 0, 0, 255);
				DoRWStuffEndOfFrame();
				DoRWStuffStartOfFrame(0, 0, 0, 0, 0, 0, 255);
				DoRWStuffEndOfFrame();
				if (_InputMouseNeedsExclusive()) {
					_InputShutdownMouse();
					_InputInitialiseMouse(true);
				}

				m_StatsScrollSpeed = 150.0f;
#ifdef FIX_BUGS
				ThingsToDoBeforeLeavingPage();
#endif
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

		if (_InputMouseNeedsExclusive()) {
			_InputShutdownMouse();
			_InputInitialiseMouse(false);
		}

		Initialise();
		LoadAllTextures();

		if (CPad::bHasPlayerCheated) {
			m_nCurrScreen = MENUPAGE_SAVE_CHEAT_WARNING;
			m_nCurrOption = 0;
		} else {
			m_nCurrScreen = MENUPAGE_CHOOSE_SAVE_SLOT;
			m_nCurrOption = 8;
		}
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
	CUserDisplay::PlaceName.ProcessAfterFrontEndShutDown();
}

void
CMenuManager::WaitForUserCD()
{
	CSprite2d *splash;
	char *splashscreen = nil;

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

void
CMenuManager::PrintMap(void)
{
	m_bMenuMapActive = true;
	CRadar::InitFrontEndMap();

	// Because m_fMapSize is half of the map length(hence * 2), and map consists of 3x3 tiles(hence / 3).
	float halfTile = m_fMapSize * 2.f / 3.f / 2.f;

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
	if (m_PrefsShowLegends) {
		CFont::SetWrapx(MENU_X_RIGHT_ALIGNED(40.0f));
		CFont::SetRightJustifyWrap(MENU_X_LEFT_ALIGNED(84.0f));
		CFont::SetBackGroundOnlyTextOff();
		CFont::SetColor(CRGBA(LABEL_COLOR.r, LABEL_COLOR.g, LABEL_COLOR.b, FadeIn(255)));
		CFont::SetDropShadowPosition(2);
		CFont::SetDropColor(CRGBA(0, 0, 0, FadeIn(255)));
		CFont::SetCentreOn();
		CFont::SetFontStyle(FONT_LOCALE(FONT_HEADING));
		CFont::SetScale(SCREEN_SCALE_X(0.65f), SCREEN_SCALE_Y(0.95f));

		int secondColumnStart = (CRadar::MapLegendCounter - 1) / 2;
		int boxBottom = MENU_Y(100.0f);

		// + 3, because we want 19*3 px padding
		for (int i = 0; i < secondColumnStart + 3; i++) {
			boxBottom += MENU_Y(19.f);
		}

		CSprite2d::DrawRect(CRect(MENU_X_LEFT_ALIGNED(95.0f), MENU_Y(100.0f), MENU_X_LEFT_ALIGNED(555.f), boxBottom),
			CRGBA(0, 0, 0, FadeIn(190)));

		CFont::PrintString(MENU_X_LEFT_ALIGNED(320.0f), MENU_Y(102.0f), TheText.Get("FE_MLG"));
		CFont::SetRightJustifyOff();
		CFont::SetFontStyle(FONT_LOCALE(FONT_STANDARD));
		if (m_PrefsLanguage == LANGUAGE_AMERICAN)
			CFont::SetScale(SCREEN_SCALE_X(0.55f), SCREEN_SCALE_Y(0.55f));
		else
			CFont::SetScale(SCREEN_SCALE_X(0.45f), SCREEN_SCALE_Y(0.55f));

		CFont::SetColor(CRGBA(225, 225, 225, FadeIn(255)));
		CFont::SetDropShadowPosition(0);

		int y = MENU_Y(127.0f);
		int x = MENU_X_LEFT_ALIGNED(160.0f);

		for (int16 i = 0; i < CRadar::MapLegendCounter; i++) {
			CRadar::DrawLegend(x, y, CRadar::MapLegendList[i]);

			if (i == secondColumnStart) {
				x = MENU_X_LEFT_ALIGNED(350.0f);
				y = MENU_Y(127.0f);
			} else {
				y += MENU_Y(19.0f);
			}
		}
	}

#ifdef MAP_ENHANCEMENTS
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

#endif
	m_bMenuMapActive = false;

	CFont::SetWrapx(MENU_X_RIGHT_ALIGNED(MENU_X_MARGIN));
	CFont::SetRightJustifyWrap(MENU_X_LEFT_ALIGNED(MENU_X_MARGIN));
	DisplayHelperText("FEH_MPH");
}

void
CMenuManager::ChangeRadioStation(int8 increaseBy)
{
	if (m_ScrollRadioBy != 0)
		return;

	m_PrefsRadioStation += increaseBy;
	m_ScrollRadioBy = increaseBy;
	if (m_ScrollRadioBy == 1) {
		DMAudio.PlayFrontEndSound(SOUND_FRONTEND_ENTER_OR_ADJUST, 0);
		m_LeftMostRadioX = MENU_X_LEFT_ALIGNED(MENURADIO_ICON_FIRST_X);
	} else {
		DMAudio.PlayFrontEndSound(SOUND_FRONTEND_ENTER_OR_ADJUST, 0);
		m_LeftMostRadioX = MENU_X_LEFT_ALIGNED(MENURADIO_ICON_FIRST_X - (2 * MENURADIO_ICON_SIZE));
	}

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
	DMAudio.StopFrontEndTrack();
	DMAudio.PlayFrontEndSound(SOUND_RADIO_CHANGE, 0);
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
