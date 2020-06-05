#pragma once

#include "Sprite2d.h"

#ifdef PS2_LIKE_MENU
#define MENUHEADER_POS_X 50.0f
#define MENUHEADER_POS_Y 75.0f
#define MENUHEADER_HEIGHT 1.3f
#else
#define MENUHEADER_POS_X 10.0f
#define MENUHEADER_POS_Y 10.0f
#define MENUHEADER_HEIGHT 2.0f
#endif
#define MENUHEADER_WIDTH 1.0f

#define MENU_UNK_X_MARGIN 10.0f
#define MENU_UNK_WIDTH 10.0f

#define MENUACTION_SCALE_MULT 0.9f

#define MENULABEL_X_MARGIN 80.0f
#define MENULABEL_WIDTH 80.0f
#define MENULABEL_POS_X 100.0f
#define MENULABEL_POS_Y 97.0f

#define MENU_DEFAULT_CONTENT_X 320
#define MENU_DEFAULT_CONTENT_Y 100
#define MENU_DEFAULT_LINE_HEIGHT 29

#define MENURADIO_ICON_SCALE 60.0f

#define MENUSLIDER_X 500.0f
#define MENUSLIDER_UNK 100.0f
#define MENUSLIDER_SMALLEST_BAR 8.0f
#define MENUSLIDER_BIGGEST_BAR 25.0f

#define BIGTEXT2_X_SCALE 0.6f
#define BIGTEXT2_Y_SCALE 1.2f
#define BIGTEXT_X_SCALE 0.6f
#define BIGTEXT_Y_SCALE 1.0f
#define MEDIUMTEXT_X_SCALE 0.48f
#define MEDIUMTEXT_Y_SCALE 1.0f
#define SMALLTEXT_X_SCALE 0.42f
#define SMALLTEXT_Y_SCALE 0.9f
#define SMALLESTTEXT_X_SCALE 0.3f
#define SMALLESTTEXT_Y_SCALE 0.7f

#define PLAYERSETUP_LIST_TOP 28.0f
#define PLAYERSETUP_LIST_BOTTOM 125.0f
#define PLAYERSETUP_LIST_LEFT 200.0f
#define PLAYERSETUP_LIST_RIGHT 36.0f
#ifdef FIX_BUGS // See the scrollbar button drawing code
#define PLAYERSETUP_SCROLLBAR_WIDTH 19.0f
#else
#define PLAYERSETUP_SCROLLBAR_WIDTH 16.0f
#endif
#define PLAYERSETUP_SCROLLBUTTON_HEIGHT 17.0f
#define PLAYERSETUP_SCROLLBUTTON_TXD_DIMENSION 64
#define PLAYERSETUP_ROW_TEXT_X_SCALE 0.4f
#define PLAYERSETUP_ROW_TEXT_Y_SCALE 0.6f
#define PLAYERSETUP_SKIN_COLUMN_LEFT 220.0f
#define PLAYERSETUP_DATE_COLUMN_RIGHT 56.0f
#define PLAYERSETUP_LIST_BODY_TOP 47
#define PLAYERSETUP_ROW_HEIGHT 9

#define STATS_SLIDE_Y_PER_SECOND 30.0f
#define STATS_ROW_HEIGHT 20.0f
#define STATS_ROW_X_MARGIN 50.0f
#define STATS_BOTTOM_MARGIN 135.0f
#define STATS_TOP_MARGIN 40.0f
#define STATS_TOP_DIMMING_AREA_LENGTH (93.0f - STATS_TOP_MARGIN)
#define STATS_BOTTOM_DIMMING_AREA_LENGTH 55.0f
#define STATS_PUT_BACK_TO_BOTTOM_Y 50.0f
#define STATS_RATING_X 24.0f
#define STATS_RATING_Y 20.0f

#define BRIEFS_TOP_MARGIN 40.0f
#define BRIEFS_LINE_X 50.0f
#define BRIEFS_LINE_HEIGHT 60.0f

#define CONTSETUP_STANDARD_ROW_HEIGHT 10.7f
#define CONTSETUP_CLASSIC_ROW_HEIGHT 9.0f
#define CONTSETUP_BOUND_HIGHLIGHT_HEIGHT 10
#define CONTSETUP_BOUND_COLUMN_WIDTH 190.0f
#define CONTSETUP_LIST_HEADER_HEIGHT 20.0f
#define CONTSETUP_LIST_TOP 28.0f
#define CONTSETUP_LIST_RIGHT 18.0f
#define CONTSETUP_LIST_BOTTOM 120.0f
#define CONTSETUP_LIST_LEFT 18.0f
#define CONTSETUP_COLUMN_1_X 40.0f
#define CONTSETUP_COLUMN_2_X 210.0f
#define CONTSETUP_COLUMN_3_X (CONTSETUP_COLUMN_2_X + CONTSETUP_BOUND_COLUMN_WIDTH + 10.0f)
#define CONTSETUP_BACK_RIGHT 35.0f
#define CONTSETUP_BACK_BOTTOM 122.0f
#define CONTSETUP_BACK_HEIGHT 25.0f

enum eLanguages
{
	LANGUAGE_AMERICAN,
	LANGUAGE_FRENCH,
	LANGUAGE_GERMAN,
	LANGUAGE_ITALIAN,
	LANGUAGE_SPANISH,
#ifdef MORE_LANGUAGES
	LANGUAGE_POLISH,
	LANGUAGE_RUSSIAN,
	LANGUAGE_JAPANESE,
#endif
};

enum
{
	MENUALIGN_LEFT = 1,
	MENUALIGN_RIGHT,
	MENUALIGN_CENTER,
};

enum eMenuSprites
{
	MENUSPRITE_BACKGROUND,
	MENUSPRITE_VCLOGO,
	MENUSPRITE_MOUSE,
	MENUSPRITE_MAPTOP01,
	MENUSPRITE_MAPTOP02,
	MENUSPRITE_MAPTOP03,
	MENUSPRITE_MAPMID01,
	MENUSPRITE_MAPMID02,
	MENUSPRITE_MAPMID03,
	MENUSPRITE_MAPBOT01,
	MENUSPRITE_MAPBOT02,
	MENUSPRITE_MAPBOT03,
	MENUSPRITE_WILDSTYLE,
	MENUSPRITE_FLASH,
	MENUSPRITE_KCHAT,
	MENUSPRITE_FEVER,
	MENUSPRITE_VROCK,
	MENUSPRITE_VCPR,
	MENUSPRITE_ESPANTOSO,
	MENUSPRITE_EMOTION,
	MENUSPRITE_WAVE,
	MENUSPRITE_MP3,
	MENUSPRITE_DOWNOFF,
	MENUSPRITE_DOWNON,
	MENUSPRITE_UPOFF,
	MENUSPRITE_UPON,
	NUM_MENU_SPRITES
};

enum eSaveSlot
{
	SAVESLOT_NONE,
	SAVESLOT_0,
	SAVESLOT_1,
	SAVESLOT_2,
	SAVESLOT_3,
	SAVESLOT_4,
	SAVESLOT_5,
	SAVESLOT_6,
	SAVESLOT_7,
	SAVESLOT_8,
	SAVESLOT_LABEL = 36
};

enum eMenuScreen
{
	MENUPAGE_DISABLED = -1,
	MENUPAGE_STATS = 0,
	MENUPAGE_NEW_GAME = 1,
	MENUPAGE_BRIEFS = 2,
	MENUPAGE_SOUND_SETTINGS = 3,
	MENUPAGE_GRAPHICS_SETTINGS = 4,
	MENUPAGE_LANGUAGE_SETTINGS = 5,
	MENUPAGE_MAP = 6,
	MENUPAGE_NEW_GAME_RELOAD = 7,
	MENUPAGE_CHOOSE_LOAD_SLOT = 8,
	MENUPAGE_CHOOSE_DELETE_SLOT = 9,
	MENUPAGE_LOAD_SLOT_CONFIRM = 10,
	MENUPAGE_DELETE_SLOT_CONFIRM = 11,
	MENUPAGE_LOADING_IN_PROGRESS = 12,
	MENUPAGE_DELETING_IN_PROGRESS = 13,
	MENUPAGE_DELETE_SUCCESSFUL = 14,
	MENUPAGE_CHOOSE_SAVE_SLOT = 15,
	MENUPAGE_SAVE_OVERWRITE_CONFIRM = 16,
	MENUPAGE_SAVING_IN_PROGRESS = 17,
	MENUPAGE_SAVE_SUCCESSFUL = 18,
	MENUPAGE_SAVE_CUSTOM_WARNING = 19,
	MENUPAGE_SAVE_CHEAT_WARNING = 20,
	MENUPAGE_SKIN_SELECT = 21,
	MENUPAGE_SAVE_UNUSED = 22,
	MENUPAGE_SAVE_FAILED = 23,
	MENUPAGE_SAVE_FAILED_2 = 24,
	MENUPAGE_LOAD_FAILED = 25,
	MENUPAGE_CONTROLLER_PC = 26,
	MENUPAGE_OPTIONS = 27,
	MENUPAGE_EXIT = 28,
	MENUPAGE_START_MENU = 29,
	MENUPAGE_KEYBOARD_CONTROLS = 30,
	MENUPAGE_MOUSE_CONTROLS = 31,
	MENUPAGE_PAUSE_MENU = 32,
	MENUPAGE_NONE = 33, // Then chooses main menu or pause menu 
	MENUPAGE_OUTRO = 34,
#ifdef LEGACY_MENU_OPTIONS
	MENUPAGE_CONTROLLER_SETTINGS,
	MENUPAGE_DEBUG_MENU,
	MENUPAGE_CONTROLLER_PC_OLD1,
	MENUPAGE_CONTROLLER_PC_OLD2,
	MENUPAGE_CONTROLLER_PC_OLD3,
	MENUPAGE_CONTROLLER_PC_OLD4,
	MENUPAGE_CONTROLLER_DEBUG,
#endif
	MENUPAGES
};

enum eMenuAction
{
	MENUACTION_NOTHING,
	MENUACTION_LABEL,
	MENUACTION_YES,
	MENUACTION_NO,
	MENUACTION_CHANGEMENU,
	MENUACTION_CTRLVIBRATION,
	MENUACTION_CTRLCONFIG,
	MENUACTION_FRAMESYNC,
	MENUACTION_FRAMELIMIT,
	MENUACTION_TRAILS,
	MENUACTION_SUBTITLES,
	MENUACTION_WIDESCREEN,
	MENUACTION_BRIGHTNESS,
	MENUACTION_MUSICVOLUME,
	MENUACTION_SFXVOLUME,
	MENUACTION_RADIO,
	MENUACTION_LANG_ENG,
	MENUACTION_LANG_FRE,
	MENUACTION_LANG_GER,
	MENUACTION_LANG_ITA,
	MENUACTION_LANG_SPA,
	MENUACTION_POPULATESLOTS_CHANGEMENU,
	MENUACTION_CHECKSAVE,
	MENUACTION_NEWGAME,
	MENUACTION_RESUME_FROM_SAVEZONE,
	MENUACTION_RELOADIDE,
	MENUACTION_SETDBGFLAG,
	MENUACTION_LOADRADIO,
	MENUACTION_SAVEGAME,

	// Below this is TODO(Miami)
	MENUACTION_DRAWDIST,
	MENUACTION_SWITCHBIGWHITEDEBUGLIGHT,
	MENUACTION_COLLISIONPOLYS,
	MENUACTION_LEGENDS,
	MENUACTION_RADARMODE,
	MENUACTION_HUD,
	MENUACTION_GOBACK,
	MENUACTION_KEYBOARDCTRLS,
	MENUACTION_PARSEHEAP,
	// MENUACTION_MEMCARDSAVECONFIRM is that on VC enum??
	MENUACTION_DEBUGSTREAM,
	MENUACTION_REDEFCTRL,
	MENUACTION_GETKEY,
	MENUACTION_SHOWHEADBOB,
	MENUACTION_UNK80,
	MENUACTION_INVVERT,
	MENUACTION_CANCELGAME,
	MENUACTION_MOUSESENS,
	MENUACTION_MP3VOLUMEBOOST,
	MENUACTION_RESUME,
	MENUACTION_DONTCANCEL,
	MENUACTION_SCREENRES,
	MENUACTION_AUDIOHW,
	MENUACTION_SPEAKERCONF,
	MENUACTION_PLAYERSETUP,
	MENUACTION_RESTOREDEF,
	MENUACTION_CTRLMETHOD,
	MENUACTION_DYNAMICACOUSTIC,
	MENUACTION_MOUSESTEER,
	MENUACTION_UNK103,
	MENUACTION_UNK104,
	MENUACTION_UNK105,
	MENUACTION_UNK106,
	MENUACTION_UNK107,
	MENUACTION_UNK108,
	MENUACTION_UNK109,
	MENUACTION_UNK110,
#ifdef MORE_LANGUAGES
	MENUACTION_LANG_PL,
	MENUACTION_LANG_RUS,
	MENUACTION_LANG_JAP,
#endif
#ifdef IMPROVED_VIDEOMODE
	MENUACTION_SCREENMODE,
#endif
#ifdef FREE_CAM
	MENUACTION_FREECAM
#endif
};

enum eCheckHover
{
	HOVEROPTION_0,
	HOVEROPTION_1,
	HOVEROPTION_RANDOM_ITEM,
	HOVEROPTION_3,
	HOVEROPTION_4,
	HOVEROPTION_5,
	HOVEROPTION_6,
	HOVEROPTION_7,
	HOVEROPTION_8,
	HOVEROPTION_BACK,	// also layer in controller setup and skin menu
	HOVEROPTION_10,
	HOVEROPTION_11,
	HOVEROPTION_OVER_SCROLL_UP,
	HOVEROPTION_OVER_SCROLL_DOWN,
	HOVEROPTION_CLICKED_SCROLL_UP,
	HOVEROPTION_CLICKED_SCROLL_DOWN,
	HOVEROPTION_HOLDING_SCROLLBAR,
	HOVEROPTION_PAGEUP,
	HOVEROPTION_PAGEDOWN,
	HOVEROPTION_LIST, // also layer in controller setup and skin menu
	HOVEROPTION_SKIN,
	HOVEROPTION_USESKIN, // also layer in controller setup and skin menu
	HOVEROPTION_RADIO_0,
	HOVEROPTION_RADIO_1,
	HOVEROPTION_RADIO_2,
	HOVEROPTION_RADIO_3,
	HOVEROPTION_RADIO_4,
	HOVEROPTION_RADIO_5,
	HOVEROPTION_RADIO_6,
	HOVEROPTION_RADIO_7,
	HOVEROPTION_RADIO_8,
	HOVEROPTION_RADIO_9,
	HOVEROPTION_INCREASE_BRIGHTNESS,
	HOVEROPTION_DECREASE_BRIGHTNESS,
	HOVEROPTION_INCREASE_DRAWDIST,
	HOVEROPTION_DECREASE_DRAWDIST,
	HOVEROPTION_INCREASE_MUSICVOLUME,
	HOVEROPTION_DECREASE_MUSICVOLUME,
	HOVEROPTION_INCREASE_SFXVOLUME,
	HOVEROPTION_DECREASE_SFXVOLUME,
	HOVEROPTION_INCREASE_MOUSESENS,
	HOVEROPTION_DECREASE_MOUSESENS,
	HOVEROPTION_NOT_HOVERING,
};

enum
{
#ifdef LEGACY_MENU_OPTIONS
	NUM_MENUROWS = 14,
#else
	NUM_MENUROWS = 12,
#endif
};

enum eControlMethod
{
	CONTROL_STANDARD = 0,
	CONTROL_CLASSIC,
};

// Why??
enum ControllerSetupColumn
{
	CONTSETUP_PED_COLUMN = 0,
	CONTSETUP_VEHICLE_COLUMN = 14,
};

struct tSkinInfo
{
	int32 skinId;
	char skinNameDisplayed[256];
	char skinNameOriginal[256];
	char date[256];
	tSkinInfo *nextSkin;
};

struct BottomBarOption
{
	char name[8];
	int32 screenId;
};

struct CMenuScreen
{
	char m_ScreenName[8];
	int32 m_PreviousPage; // eMenuScreen
	int32 m_ParentEntry; // row

	struct CMenuEntry
	{
		int32 m_Action; // eMenuAction
		char m_EntryName[8];
		int32 m_SaveSlot; // eSaveSlot
		int32 m_TargetMenu; // eMenuScreen
		uint16 m_X;
		uint16 m_Y;
		uint8 m_Align;
	} m_aEntries[NUM_MENUROWS];
};

struct MenuTrapezoid
{
	float topLeft_x;
	float topLeft_y;
	float topRight_x;
	float topRight_y;
	float bottomLeft_x;
	float bottomLeft_y;
	float bottomRight_x;
	float bottomRight_y;
	float old_topRight_x;
	float old_topRight_y;
	float old_topLeft_x;
	float old_topLeft_y;
	float old_bottomLeft_x;
	float old_bottomLeft_y;
	float old_bottomRight_x;
	float old_bottomRight_y;
	float mult_topRight_x;
	float mult_topRight_y;
	float mult_topLeft_x;
	float mult_topLeft_y;
	float mult_bottomLeft_x;
	float mult_bottomLeft_y;
	float mult_bottomRight_x;
	float mult_bottomRight_y;

	MenuTrapezoid(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4) {
		topLeft_x = x1;
		topLeft_y = y1;
		topRight_x = x2;
		topRight_y = y2;
		bottomLeft_x = x3;
		bottomLeft_y = y3;
		bottomRight_x = x4;
		bottomRight_y = y4;
	};

	void SaveCurrentCoors() {
		old_topLeft_x = topLeft_x;
		old_topLeft_y = topLeft_y;
		old_topRight_x = topRight_x;
		old_topRight_y = topRight_y;
		old_bottomLeft_x = bottomLeft_x;
		old_bottomLeft_y = bottomLeft_y;
		old_bottomRight_x = bottomRight_x;
		old_bottomRight_y = bottomRight_y;
	}

	void Translate(int delta) {
		bottomRight_x = delta * mult_bottomRight_x + old_bottomRight_x;
		bottomRight_y = delta * mult_bottomRight_y + old_bottomRight_y;
		bottomLeft_x = delta * mult_bottomLeft_x + old_bottomLeft_x;
		bottomLeft_y = delta * mult_bottomLeft_y + old_bottomLeft_y;
		topRight_x = delta * mult_topRight_x + old_topRight_x;
		topRight_y = delta * mult_topRight_y + old_topRight_y;
		topLeft_x = delta * mult_topLeft_x + old_topLeft_x;
		topLeft_y = delta * mult_topLeft_y + old_topLeft_y;
	}

	void UpdateMultipliers() {
		mult_bottomRight_x = (bottomRight_x - old_bottomRight_x) / 255.0f;
		mult_bottomRight_y = (bottomRight_y - old_bottomRight_y) / 255.0f;
		mult_bottomLeft_x = (bottomLeft_x - old_bottomLeft_x) / 255.0f;
		mult_bottomLeft_y = (bottomLeft_y - old_bottomLeft_y) / 255.0f;
		mult_topRight_x = (topRight_x - old_topRight_x) / 255.0f;
		mult_topRight_y = (topRight_y - old_topRight_y) / 255.0f;
		mult_topLeft_x = (topLeft_x - old_topLeft_x) / 255.0f;
		mult_topLeft_y = (topLeft_y - old_topLeft_y) / 255.0f;
	}
};

class CMenuManager
{
public:
	int8 m_StatsScrollDirection;
	float m_StatsScrollSpeed;
	uint8 field_8;
	bool m_PrefsUseVibration;
	bool m_PrefsShowHud;
	int32 m_PrefsRadarMode;
	uint8 field_10;
	bool m_bShutDownFrontEndRequested;
	bool m_bStartUpFrontEndRequested;
	int32 m_KeyPressedCode;
	int32 m_PrefsBrightness;
	float m_PrefsLOD;
	int8 m_PrefsShowSubtitles;
	int8 m_PrefsShowLegends;
	int8 m_PrefsUseWideScreen;
	int8 m_PrefsVsync; // TODO(Miami): Are we sure?
	int8 m_PrefsVsyncDisp;
	int8 m_PrefsFrameLimiter;
	int8 m_nPrefsAudio3DProviderIndex;
	int8 m_PrefsSpeakers;
	int8 m_PrefsDMA;
	uint8 m_PrefsSfxVolume;
	uint8 m_PrefsMusicVolume;
	uint8 m_PrefsRadioStation;
	uint8 field_2C;
	int32 m_nCurrOption;
	bool m_bQuitGameNoCD;
	bool m_bMenuMapActive;
	bool m_AllowNavigation;
	uint8 field_37;
	bool m_bMenuActive;
	bool m_bWantToRestart;
	bool m_bFirstTime;
	bool m_bActivateSaveMenu;
	bool m_bWantToLoad;
	float m_fMapSize;
	float m_fMapCenterX;
	float m_fMapCenterY;
	uint32 OS_Language;
	int32 m_PrefsLanguage;
	int32 field_54;
	int8 m_bLanguageLoaded;
	uint8 m_PrefsAllowNastyGame;
	uint8 m_PrefsMP3BoostVolume;
	uint8 m_ControlMethod;
	int32 m_nPrefsVideoMode;
	int32 m_nDisplayVideoMode;
	int32 m_nMouseTempPosX;
	int32 m_nMouseTempPosY;
	bool m_bGameNotLoaded;
	int8 m_lastWorking3DAudioProvider;
	bool m_bFrontEnd_ReloadObrTxtGxt;
	int32 *pEditString;
	uint8 field_74[4];
	int32 *pControlEdit;
	bool m_OnlySaveMenu;
	int32 m_menuTransitionProgress;
	CSprite2d m_aFrontEndSprites[NUM_MENU_SPRITES];
	bool m_bSpritesLoaded;
	int32 field_F0;
	int32 m_LastRadioScrollDir;
	int32 m_nCurrScreen;
	int32 m_nPrevScreen;
	int32 m_nCurrSaveSlot;
	uint32 m_LastScreenSwitch;
	int32 m_nMenuFadeAlpha;
	int32 m_nOptionHighlightTransitionBlend;
	bool bMenuChangeOngoing;
	int32 MouseButtonJustClicked;
	int32 JoyButtonJustClicked;
	bool DisplayComboButtonErrMsg;
	bool m_NoEmptyBinding;
	bool m_ShowEmptyBindingError;
	int32 m_nHelperTextAlpha;
	bool m_bPressedPgUpOnList;
	bool m_bPressedPgDnOnList;
	bool m_bPressedUpOnList;
	bool m_bPressedDownOnList;
	bool m_bPressedScrollButton;
	uint8 field_129;
	uint8 field_12A;
	uint8 field_12B;
	int32 m_nMousePosX;
	int32 m_nMousePosY;
	int32 m_nMouseOldPosX;
	int32 m_nMouseOldPosY;
	int32 m_nHoverOption;
	bool m_bShowMouse;
	int32 m_nPrevOption;
	bool m_bStartWaitingForKeyBind;
	bool m_bWaitingForNewKeyBind;
	bool m_bKeyChangeNotProcessed;
	int32 m_CurrCntrlAction;
	uint8 field_150;
	uint8 field_151;
	uint8 field_152;
	uint8 field_153;
	int32 m_nSelectedContSetupColumn;
	bool m_bKeyIsOK;
	bool field_159;
	uint8 m_nCurrExLayer;
	char m_PrefsSkinFile[256];
	char m_aSkinName[256];
	uint8 field_35B;
	int32 m_nHelperTextMsgId;
	tSkinInfo m_pSkinListHead;
	tSkinInfo *m_pSelectedSkin;
	int32 m_nFirstVisibleRowOnList;
	float m_nScrollbarTopMargin;
	int32 m_nTotalListRow;
	int32 m_nSkinsTotal;
	uint8 field_67C[4];
	int32 m_nSelectedListRow;
	bool m_bSkinsEnumerated;

#ifdef IMPROVED_VIDEOMODE
	int32 m_nPrefsWidth;
	int32 m_nPrefsHeight;
	int32 m_nPrefsDepth;
	int32 m_nPrefsWindowed;
	int32 m_nPrefsSubsystem;
	int32 m_nSelectedScreenMode;
#endif

	bool GetIsMenuActive() {return !!m_bMenuActive;}

	static uint8 m_PrefsStereoMono;

#ifndef MASTER
	static bool m_PrefsMarketing;
	static bool m_PrefsDisableTutorials;
#endif // !MASTER

	CMenuManager(void);
	~CMenuManager(void) { UnloadTextures(); }
	
	void Initialise();
	void PrintMap();
	void SetFrontEndRenderStates();
	static void BuildStatLine(Const char *text, void *stat, bool itsFloat, void *stat2);
	static void CentreMousePointer();
	void CheckCodesForControls(int);
	bool CheckHover(int x1, int x2, int y1, int y2);
	void CheckSliderMovement(int);
	void DisplayHelperText();
	int DisplaySlider(float, float, float, float, float, float, float);
	void DoSettingsBeforeStartingAGame();
	void DrawStandardMenus(bool);
	void DrawControllerBound(int32, int32, int32, int8);
	void DrawControllerScreenExtraText(int, int, int);
	void DrawControllerSetupScreen();
	void DrawQuitGameScreen();
	void DrawFrontEnd();
	void DrawBackground(bool transitionCall);
	void DrawPlayerSetupScreen();
	int FadeIn(int alpha);
	void FilterOutColorMarkersFromString(wchar*, CRGBA &);
	int GetStartOptionsCntrlConfigScreens();
	void InitialiseChangedLanguageSettings();
	void LoadAllTextures();
	void LoadSettings();
	void MessageScreen(const char *str, bool);
	void SmallMessageScreen(const char *str);
	void PrintBriefs();
	static void PrintErrorMessage();
	void PrintStats();
	void Process();
	void ProcessButtonPresses();
	void ProcessFileActions();
	void ProcessOnOffMenuOptions();
	void RequestFrontEndShutDown();
	void RequestFrontEndStartUp();
	void ResetHelperText();
	void SaveSettings();
	void SetHelperText(int text);
	float StretchX(float);
	float StretchY(float);
	void SwitchMenuOnAndOff();
	void UnloadTextures();
	void WaitForUserCD();
	int GetNumOptionsCntrlConfigScreens();
	int ConstructStatLine(int);
	void SwitchToNewScreen(int8);

	// New (not in function or inlined in the game)
	void ThingsToDoBeforeLeavingPage();
	void ScrollUpListByOne();
	void ScrollDownListByOne();
	void PageUpList(bool);
	void PageDownList(bool);
	
	// uint8 GetNumberOfMenuOptions();
};

#ifndef IMPROVED_VIDEOMODE
VALIDATE_SIZE(CMenuManager, 0x688);
#endif

extern CMenuManager FrontEndMenuManager;
