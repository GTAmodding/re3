#pragma once
#ifdef PS2_MENU
#include "Frontend_PS2.h"
#else

#include "Sprite2d.h"

#ifdef PS2_LIKE_MENU
#define MENUHEADER_POS_X 50.0f
#define MENUHEADER_POS_Y 75.0f
#define MENUHEADER_HEIGHT 1.3f
#else
#define MENUHEADER_POS_X 35.0f
#define MENUHEADER_POS_Y 93.0f
#define MENUHEADER_HEIGHT 1.6f
#endif
#define MENUHEADER_WIDTH 0.84f

#define MENU_X_MARGIN 40.0f
#define MENUACTION_POS_Y 60.0f
#define MENUACTION_SCALE_MULT 0.9f

#define MENURADIO_ICON_SCALE 60.0f

#define MENUSLIDER_X 256.0f
#define MENUSLIDER_UNK 256.0f

#define MENUSLIDER_BARS 16
#define MENUSLIDER_LOGICAL_BARS MENUSLIDER_BARS

#define BIGTEXT_X_SCALE 0.75f // For FONT_HEADING
#define BIGTEXT_Y_SCALE 0.9f
#define MEDIUMTEXT_X_SCALE 0.55f // For FONT_HEADING
#define MEDIUMTEXT_Y_SCALE 0.8f
#define SMALLTEXT_X_SCALE 0.45f // used for FONT_HEADING and FONT_BANK, but looks off for HEADING 
#define SMALLTEXT_Y_SCALE 0.7f
#define SMALLESTTEXT_X_SCALE 0.4f // used for both FONT_HEADING and FONT_BANK
#define SMALLESTTEXT_Y_SCALE 0.6f

#define HELPER_TEXT_LEFT_MARGIN 320.0f
#define HELPER_TEXT_BOTTOM_MARGIN 120.0f

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

enum eFrontendSprites
{
	FE2_MAINPANEL_UL,
	FE2_MAINPANEL_UR,
	FE2_MAINPANEL_DL,
	FE2_MAINPANEL_DR,
	FE2_MAINPANEL_DR2,
	FE2_TABACTIVE,
	FE_ICONBRIEF,
	FE_ICONSTATS,
	FE_ICONCONTROLS,
	FE_ICONSAVE,
	FE_ICONAUDIO,
	FE_ICONDISPLAY,
	FE_ICONLANGUAGE,
	FE_CONTROLLER,
	FE_CONTROLLERSH,
	FE_ARROWS1,
	FE_ARROWS2,
	FE_ARROWS3,
	FE_ARROWS4,
	FE_RADIO1,
	FE_RADIO2,
	FE_RADIO3,
	FE_RADIO4,
	FE_RADIO5,
	FE_RADIO6,
	FE_RADIO7,
	FE_RADIO8,
	FE_RADIO9,

	NUM_FE_SPRITES
};

enum eMenuSprites
{
	MENUSPRITE_CONNECTION,
	MENUSPRITE_FINDGAME,
	MENUSPRITE_HOSTGAME,
	MENUSPRITE_MAINMENU,
	MENUSPRITE_PLAYERSET,
	MENUSPRITE_SINGLEPLAYER,
	MENUSPRITE_MULTIPLAYER,
	MENUSPRITE_DMALOGO,
	MENUSPRITE_GTALOGO,
	MENUSPRITE_RSTARLOGO,
	MENUSPRITE_GAMESPY,
	MENUSPRITE_MOUSE,
	MENUSPRITE_MOUSET,
	MENUSPRITE_MP3LOGO,
	MENUSPRITE_DOWNOFF,
	MENUSPRITE_DOWNON,
	MENUSPRITE_UPOFF,
	MENUSPRITE_UPON,
	MENUSPRITE_GTA3LOGO,
	MENUSPRITE_UNUSED,
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
	SAVESLOT_LABEL = 36,
};

#ifdef MENU_MAP
enum MapSprites
{
	MAPMID1,
	MAPMID2,
	MAPMID3,
	MAPBOT1,
	MAPBOT2,
	MAPBOT3,
	MAPTOP1,
	MAPTOP2,
	MAPTOP3,
	NUM_MAP_SPRITES
};
#endif

enum eMenuScreen
{
	MENUPAGE_DISABLED = -1,
	MENUPAGE_NONE = 0,
	MENUPAGE_STATS = 1,
	MENUPAGE_NEW_GAME = 2,
	MENUPAGE_BRIEFS = 3,
	MENUPAGE_CONTROLLER_SETTINGS = 4,
	MENUPAGE_SOUND_SETTINGS = 5,
	MENUPAGE_DISPLAY_SETTINGS = 6,
	MENUPAGE_LANGUAGE_SETTINGS = 7,
	MENUPAGE_CHOOSE_LOAD_SLOT = 8,
	MENUPAGE_CHOOSE_DELETE_SLOT = 9,
	MENUPAGE_NEW_GAME_RELOAD = 10,
	MENUPAGE_LOAD_SLOT_CONFIRM = 11,
	MENUPAGE_DELETE_SLOT_CONFIRM = 12,
	MENUPAGE_NO_MEMORY_CARD = 13, // hud adjustment page in mobile
	MENUPAGE_LOADING_IN_PROGRESS = 14,
	MENUPAGE_DELETING_IN_PROGRESS = 15,
	MENUPAGE_PS2_LOAD_FAILED = 16,
	MENUPAGE_DELETE_FAILED = 17,
	MENUPAGE_DEBUG_MENU = 18,
	MENUPAGE_MEMORY_CARD_DEBUG = 19,
	MENUPAGE_MEMORY_CARD_TEST = 20,
	MENUPAGE_MULTIPLAYER_MAIN = 21,
	MENUPAGE_PS2_SAVE_FAILED = 22,
	MENUPAGE_PS2_SAVE_FAILED_2 = 23,
	MENUPAGE_SAVE = 24,
	MENUPAGE_NO_MEMORY_CARD_2 = 25,
	MENUPAGE_CHOOSE_SAVE_SLOT = 26,
	MENUPAGE_SAVE_OVERWRITE_CONFIRM = 27,
	MENUPAGE_MULTIPLAYER_MAP = 28,
	MENUPAGE_MULTIPLAYER_CONNECTION = 29,
	MENUPAGE_MULTIPLAYER_FIND_GAME = 30,
	MENUPAGE_MULTIPLAYER_MODE = 31,
	MENUPAGE_MULTIPLAYER_CREATE = 32,
	MENUPAGE_MULTIPLAYER_START = 33,
	MENUPAGE_SKIN_SELECT_OLD = 34,
	MENUPAGE_CONTROLLER_PC = 35,
	MENUPAGE_CONTROLLER_PC_OLD1 = 36,
	MENUPAGE_CONTROLLER_PC_OLD2 = 37,
	MENUPAGE_CONTROLLER_PC_OLD3 = 38,
	MENUPAGE_CONTROLLER_PC_OLD4 = 39,
	MENUPAGE_CONTROLLER_DEBUG = 40,
	MENUPAGE_OPTIONS = 41,
	MENUPAGE_EXIT = 42,
	MENUPAGE_SAVING_IN_PROGRESS = 43,
	MENUPAGE_SAVE_SUCCESSFUL = 44,
	MENUPAGE_DELETING = 45,
	MENUPAGE_DELETE_SUCCESS = 46,
	MENUPAGE_SAVE_FAILED = 47,
	MENUPAGE_LOAD_FAILED = 48,
	MENUPAGE_LOAD_FAILED_2 = 49,
	MENUPAGE_FILTER_GAME = 50,
	MENUPAGE_START_MENU = 51,
	MENUPAGE_PAUSE_MENU = 52,
	MENUPAGE_CHOOSE_MODE = 53,
	MENUPAGE_SKIN_SELECT = 54,
	MENUPAGE_KEYBOARD_CONTROLS = 55,
	MENUPAGE_MOUSE_CONTROLS = 56,
	MENUPAGE_MISSION_RETRY = 57,
#ifdef CUSTOM_FRONTEND_OPTIONS

#ifdef MENU_MAP
	MENUPAGE_MAP = 58,
#endif
#ifdef GRAPHICS_MENU_OPTIONS
	MENUPAGE_GRAPHICS_SETTINGS,
#endif
#ifdef DETECT_JOYSTICK_MENU
	MENUPAGE_DETECT_JOYSTICK,
#endif

#endif
	MENUPAGE_UNK, // originally 58. Custom screens are inserted above, because last screen in CMenuScreens should always be empty to make CFO work
	MENUPAGES

};

enum eMenuAction
{
#ifdef CUSTOM_FRONTEND_OPTIONS
	MENUACTION_CFO_SLIDER = -3,
	MENUACTION_CFO_SELECT = -2,
	MENUACTION_CFO_DYNAMIC = -1,
#endif
	MENUACTION_NOTHING,
	MENUACTION_LABEL,
	MENUACTION_CHANGEMENU,
	MENUACTION_CTRLVIBRATION,
	MENUACTION_CTRLCONFIG,
	MENUACTION_CTRLDISPLAY,
	MENUACTION_FRAMESYNC,
	MENUACTION_FRAMELIMIT,
	MENUACTION_TRAILS,
	MENUACTION_SUBTITLES,
	MENUACTION_WIDESCREEN,
	MENUACTION_BRIGHTNESS,
	MENUACTION_DRAWDIST,
	MENUACTION_MUSICVOLUME,
	MENUACTION_SFXVOLUME,
	MENUACTION_UNK15,
	MENUACTION_RADIO,
	MENUACTION_LANG_ENG,
	MENUACTION_LANG_FRE,
	MENUACTION_LANG_GER,
	MENUACTION_LANG_ITA,
	MENUACTION_LANG_SPA,
	MENUACTION_POPULATESLOTS_CHANGEMENU,
	MENUACTION_CHECKSAVE,
	MENUACTION_UNK24,
	MENUACTION_NEWGAME,
	MENUACTION_RELOADIDE,
	MENUACTION_RELOADIPL,
	MENUACTION_SETDBGFLAG,
	MENUACTION_SWITCHBIGWHITEDEBUGLIGHT,
	MENUACTION_PEDROADGROUPS,
	MENUACTION_CARROADGROUPS,
	MENUACTION_COLLISIONPOLYS,
	MENUACTION_REGMEMCARD1,
	MENUACTION_TESTFORMATMEMCARD1,
	MENUACTION_TESTUNFORMATMEMCARD1,
	MENUACTION_CREATEROOTDIR,
	MENUACTION_CREATELOADICONS,
	MENUACTION_FILLWITHGUFF,
	MENUACTION_SAVEONLYTHEGAME,
	MENUACTION_SAVEGAME,
	MENUACTION_SAVEGAMEUNDERGTA,
	MENUACTION_CREATECOPYPROTECTED,
	MENUACTION_TESTSAVE,
	MENUACTION_TESTLOAD,
	MENUACTION_TESTDELETE,
	MENUACTION_PARSEHEAP,
	MENUACTION_SHOWCULL,
	MENUACTION_MEMCARDSAVECONFIRM,
	MENUACTION_RESUME_FROM_SAVEZONE,
	MENUACTION_UNK50,
	MENUACTION_DEBUGSTREAM,
	MENUACTION_MPMAP_LIBERTY,
	MENUACTION_MPMAP_REDLIGHT,
	MENUACTION_MPMAP_CHINATOWN,
	MENUACTION_MPMAP_TOWER,
	MENUACTION_MPMAP_SEWER,
	MENUACTION_MPMAP_INDUSTPARK,
	MENUACTION_MPMAP_DOCKS,
	MENUACTION_MPMAP_STAUNTON,
	MENUACTION_MPMAP_DEATHMATCH1,
	MENUACTION_MPMAP_DEATHMATCH2,
	MENUACTION_MPMAP_TEAMDEATH1,
	MENUACTION_MPMAP_TEAMDEATH2,
	MENUACTION_MPMAP_STASH,
	MENUACTION_MPMAP_CAPTURE,
	MENUACTION_MPMAP_RATRACE,
	MENUACTION_MPMAP_DOMINATION,
	MENUACTION_STARTMP,
	MENUACTION_UNK69,
	MENUACTION_UNK70,
	MENUACTION_FINDMP,
	MENUACTION_KEYBOARDCTRLS,
	MENUACTION_UNK73,
	MENUACTION_INITMP,
	MENUACTION_MP_PLAYERCOLOR,
	MENUACTION_MP_PLAYERNAME,
	MENUACTION_MP_GAMENAME,
	MENUACTION_GETKEY,
	MENUACTION_SHOWHEADBOB,
	MENUACTION_UNK80,
	MENUACTION_INVVERT,
	MENUACTION_CANCELGAME,
	MENUACTION_MP_PLAYERNUMBER,
	MENUACTION_MOUSESENS,
	MENUACTION_CHECKMPGAMES,
	MENUACTION_CHECKMPPING,
	MENUACTION_MP_SERVER,
	MENUACTION_MP_MAP,
	MENUACTION_MP_GAMETYPE,
	MENUACTION_MP_LAN,
	MENUACTION_MP_INTERNET,
	MENUACTION_RESUME,
	MENUACTION_DONTCANCEL,
	MENUACTION_SCREENRES,
	MENUACTION_AUDIOHW,
	MENUACTION_SPEAKERCONF,
	MENUACTION_PLAYERSETUP,
	MENUACTION_RESTOREDEF,
	MENUACTION_CTRLMETHOD,
	MENUACTION_DYNAMICACOUSTIC,
	MENUACTION_LOADRADIO,
	MENUACTION_MOUSESTEER,
	MENUACTION_UNK103,
	MENUACTION_UNK104,
	MENUACTION_UNK105,
	MENUACTION_UNK106,
	MENUACTION_UNK107,
	MENUACTION_UNK108,
	MENUACTION_UNK109,
	MENUACTION_UNK110,
	MENUACTION_UNK111,
	MENUACTION_UNK112,
	MENUACTION_REJECT_RETRY,
	MENUACTION_UNK114,
//#ifdef ANISOTROPIC_FILTERING
//	MENUACTION_MIPMAPS,
//	MENUACTION_TEXTURE_FILTERING,
//#endif
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
#ifdef CUSTOM_FRONTEND_OPTIONS
	HOVEROPTION_INCREASE_CFO_SLIDER,
	HOVEROPTION_DECREASE_CFO_SLIDER,
#endif
	HOVEROPTION_NOT_HOVERING,
};

enum
{
	NUM_MENUROWS = 18,
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

#ifndef CUSTOM_FRONTEND_OPTIONS
struct CMenuScreen
{
	char m_ScreenName[8];
	int32 unk; // 2 on MENUPAGE_MULTIPLAYER_START, 1 on everywhere else, 0 on unused.
	int32 m_PreviousPage[2]; // eMenuScreen
	int32 m_ParentEntry[2]; // row

	struct CMenuEntry
	{
		int32 m_Action; // eMenuAction
		char m_EntryName[8];
		int32 m_SaveSlot; // eSaveSlot
		int32 m_TargetMenu; // eMenuScreen
	} m_aEntries[NUM_MENUROWS];
};
extern CMenuScreen aScreens[MENUPAGES];
#else
#include "frontendoption.h"
struct CCustomScreenLayout {
	eMenuSprites sprite;
	int columnWidth;
	int headerHeight;
	int lineHeight;
	int8 font;
	int8 alignment;
	bool showLeftRightHelper;
	float fontScaleX;
	float fontScaleY;
};

struct CCFO
{
	void *value;
	const char *saveCat;
	const char *save;
};

struct CCFOSelect : CCFO
{
	char** rightTexts;
	int8 numRightTexts;
	bool onlyApplyOnEnter;
	int8 displayedValue; // only if onlyApplyOnEnter enabled for now
	int8 lastSavedValue; // only if onlyApplyOnEnter enabled
	ChangeFunc changeFunc;
	bool disableIfGameLoaded;

	CCFOSelect() {};
	CCFOSelect(int8* value, const char* saveCat, const char* save, const char** rightTexts, int8 numRightTexts, bool onlyApplyOnEnter, ChangeFunc changeFunc = nil, bool disableIfGameLoaded = false){
		this->value = value;
		if (value)
			this->lastSavedValue = this->displayedValue = *value;

		this->saveCat = saveCat;
		this->save = save;
		this->rightTexts = (char**)rightTexts;
		this->numRightTexts = numRightTexts;
		this->onlyApplyOnEnter = onlyApplyOnEnter;
		this->changeFunc = changeFunc;
		this->disableIfGameLoaded = disableIfGameLoaded;
	}
};

// Value is float in here
struct CCFOSlider : CCFO
{
	ChangeFuncFloat changeFunc;
	float min;
	float max;

	CCFOSlider() {};
	CCFOSlider(float* value, const char* saveCat, const char* save, float min, float max, ChangeFuncFloat changeFunc = nil){
		this->value = value;
		this->saveCat = saveCat;
		this->save = save;
		this->changeFunc = changeFunc;
		this->min = min;
		this->max = max;
	}
};

struct CCFODynamic : CCFO
{
	DrawFunc drawFunc;
	ButtonPressFunc buttonPressFunc;

	CCFODynamic() {};
	CCFODynamic(int8* value, const char* saveCat, const char* save, DrawFunc drawFunc, ButtonPressFunc buttonPressFunc){
		this->value = value;
		this->saveCat = saveCat;
		this->save = save;
		this->drawFunc = drawFunc;
		this->buttonPressFunc = buttonPressFunc;
	}
};

struct CMenuScreenCustom
{
	char m_ScreenName[8];
	int32 m_PreviousPage[2]; // eMenuScreen
	CCustomScreenLayout *layout;
	ReturnPrevPageFunc returnPrevPageFunc;
	
	struct CMenuEntry
	{
		int32 m_Action; // eMenuAction - below zero is CFO
		char m_EntryName[8];
		struct {
			union {
				CCFO *m_CFO; // for initializing
				CCFOSelect *m_CFOSelect;
				CCFODynamic *m_CFODynamic;
				CCFOSlider *m_CFOSlider;
			};
			int32 m_SaveSlot; // eSaveSlot
			int32 m_TargetMenu; // eMenuScreen
		};
	} m_aEntries[NUM_MENUROWS];
};
extern CMenuScreenCustom aScreens[MENUPAGES];
#endif

class CMenuManager
{
public:
	int32 m_nPrefsVideoMode;
	int32 m_nDisplayVideoMode;
	int8 m_nPrefsAudio3DProviderIndex;
	bool m_bKeyChangeNotProcessed;
	char m_aSkinName[256];
	int32 m_nHelperTextMsgId;
	bool m_bLanguageLoaded;
	bool m_bMenuActive;
	bool m_bMenuStateChanged;
	bool m_bWaitingForNewKeyBind;
	bool m_bWantToRestart;
	bool m_bFirstTime;
	bool m_bGameNotLoaded;
	int32 m_nMousePosX;
	int32 m_nMousePosY;
	int32 m_nMouseTempPosX;
	int32 m_nMouseTempPosY;
	bool m_bShowMouse;
	tSkinInfo m_pSkinListHead;
	tSkinInfo *m_pSelectedSkin;
	int32 m_nFirstVisibleRowOnList;
	float m_nScrollbarTopMargin;
	int32 m_nTotalListRow;
	int32 m_nSkinsTotal;
 char _unk0[4];
	int32 m_nSelectedListRow;
	bool m_bSkinsEnumerated;
	bool m_bQuitGameNoCD;
 bool m_bRenderGameInMenu;
	bool m_bSaveMenuActive;
	bool m_bWantToLoad;
 char field_455;
	bool m_bStartWaitingForKeyBind;
	bool m_bSpritesLoaded;
	CSprite2d m_aFrontEndSprites[NUM_FE_SPRITES];
	CSprite2d m_aMenuSprites[NUM_MENU_SPRITES];
 int32 field_518;
	int32 m_nMenuFadeAlpha;
	bool m_bPressedPgUpOnList;
	bool m_bPressedPgDnOnList;
	bool m_bPressedUpOnList;
	bool m_bPressedDownOnList;
	bool m_bPressedScrollButton;
	int32 m_CurrCntrlAction;
 char _unk1[4];
 int32 m_nSelectedContSetupColumn;
	 bool m_bKeyIsOK;
 bool field_535;
	int8 m_nCurrExLayer;
	int32 m_nHelperTextAlpha;
	int32 m_nMouseOldPosX;
	int32 m_nMouseOldPosY;
	int32 m_nHoverOption;
	int32 m_nCurrScreen;
	int32 m_nCurrOption;
	int32 m_nOptionMouseHovering;
	int32 m_nPrevScreen;
 uint32 field_558;
	int32 m_nCurrSaveSlot;
	int32 m_nScreenChangeDelayTimer;

#ifdef IMPROVED_VIDEOMODE
	int32 m_nPrefsWidth;
	int32 m_nPrefsHeight;
	int32 m_nPrefsDepth;
	int32 m_nPrefsWindowed;
	int32 m_nPrefsSubsystem;
	int32 m_nSelectedScreenMode;
#endif
#ifdef MULTISAMPLING
	static int8 m_nPrefsMSAALevel;
	static int8 m_nDisplayMSAALevel;
#endif

	enum LANGUAGE
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
public:
	bool GetIsMenuActive() {return !!m_bMenuActive;}

public:
	static int32 OS_Language;
	static int8 m_PrefsUseVibration;
	static int8 m_DisplayControllerOnFoot;
	static int8 m_PrefsUseWideScreen;
	static int8 m_PrefsRadioStation;
	static int8 m_PrefsVsync;
	static int8 m_PrefsVsyncDisp;
	static int8 m_PrefsFrameLimiter;
	static int8 m_PrefsShowSubtitles;
	static int8 m_PrefsSpeakers;
	static int32 m_ControlMethod;
	static int8 m_PrefsDMA;
	static int32 m_PrefsLanguage;
	static int32 m_PrefsBrightness;
	static float m_PrefsLOD;
	static int8 m_bFrontEnd_ReloadObrTxtGxt;
	static int32 m_PrefsMusicVolume;
	static int32 m_PrefsSfxVolume;
	static char m_PrefsSkinFile[256];
	static int32 m_KeyPressedCode;

	static bool m_bStartUpFrontEndRequested;
	static bool m_bShutDownFrontEndRequested;
	static bool m_PrefsAllowNastyGame;
	
	static uint8 m_PrefsStereoMono;
	static int32 m_SelectedMap;
	static int32 m_SelectedGameType;
	static uint8 m_PrefsPlayerRed;
	static uint8 m_PrefsPlayerGreen;
	static uint8 m_PrefsPlayerBlue;

#ifdef CUTSCENE_BORDERS_SWITCH
	static bool m_PrefsCutsceneBorders;
#endif

#ifndef MASTER
	static bool m_PrefsMarketing;
	static bool m_PrefsDisableTutorials;
#endif // !MASTER

#ifdef MENU_MAP
	static bool bMenuMapActive;
	static float fMapSize;
	static float fMapCenterY;
	static float fMapCenterX;
	static CSprite2d m_aMapSprites[NUM_MAP_SPRITES];
	void PrintMap();
#endif

#ifdef NO_ISLAND_LOADING
	enum
	{
		ISLAND_LOADING_LOW = 0,
		ISLAND_LOADING_MEDIUM,
		ISLAND_LOADING_HIGH
	};

	static int8 m_PrefsIslandLoading;

	#define ISLAND_LOADING_IS(p) if (CMenuManager::m_PrefsIslandLoading == CMenuManager::ISLAND_LOADING_##p)
	#define ISLAND_LOADING_ISNT(p) if (CMenuManager::m_PrefsIslandLoading != CMenuManager::ISLAND_LOADING_##p)
#else
	#define ISLAND_LOADING_IS(p)
	#define ISLAND_LOADING_ISNT(p)
#endif

#ifdef GAMEPAD_MENU
	enum
	{
		CONTROLLER_DUALSHOCK2 = 0,
		CONTROLLER_DUALSHOCK3,
		CONTROLLER_DUALSHOCK4,
		CONTROLLER_XBOX360,
		CONTROLLER_XBOXONE,
		CONTROLLER_NINTENDO_SWITCH,
	};

	static int8 m_PrefsControllerType;
#endif

public:
	static void BuildStatLine(Const char *text, void *stat, bool itsFloat, void *stat2);
	static void CentreMousePointer();
	void CheckCodesForControls(int);
	bool CheckHover(int x1, int x2, int y1, int y2);
	void CheckSliderMovement(int);
	int CostructStatLine(int);
	void DisplayHelperText();
	int DisplaySlider(float, float, float, float, float, float);
	void DoSettingsBeforeStartingAGame();
	void Draw();
	void DrawControllerBound(int32, int32, int32, int8);
	void DrawControllerScreenExtraText(int, int, int);
	void DrawControllerSetupScreen();
	void DrawFrontEnd();
	void DrawFrontEndNormal();
#ifdef PS2_SAVE_DIALOG
	void DrawFrontEndSaveZone();
#endif
	void DrawPlayerSetupScreen();
	int FadeIn(int alpha);
	void FilterOutColorMarkersFromString(wchar*, CRGBA &);
	int GetStartOptionsCntrlConfigScreens();
	static void InitialiseChangedLanguageSettings();
	void LoadAllTextures();
	void LoadSettings();
	void MessageScreen(const char *);
	void PickNewPlayerColour();
	void PrintBriefs();
	static void PrintErrorMessage();
	void PrintStats();
	void Process();
	void ProcessButtonPresses();
	void ProcessOnOffMenuOptions();
	static void RequestFrontEndShutDown();
	static void RequestFrontEndStartUp();
	void ResetHelperText();
	void SaveLoadFileError_SetUpErrorScreen();
	void SaveSettings();
	void SetHelperText(int text);
	void ShutdownJustMenu();
	float StretchX(float);
	float StretchY(float);
	void SwitchMenuOnAndOff();
	void UnloadTextures();
	void WaitForUserCD();
	void PrintController();
	int GetNumOptionsCntrlConfigScreens();
	int ConstructStatLine(int);

	// Those are either inlined in game, not in function yet, or I can't believe that they're not inlined.
	// Names were made up by me.
	void ThingsToDoBeforeGoingBack();
	void ScrollUpListByOne();
	void ScrollDownListByOne();
	void PageUpList(bool);
	void PageDownList(bool);
	int8 GetPreviousPageOption();
	void ProcessList(bool &goBack, bool &optionSelected);
#ifdef GAMEPAD_MENU
	void LoadController(int8 type);
#endif
};

#ifndef IMPROVED_VIDEOMODE
VALIDATE_SIZE(CMenuManager, 0x564);
#endif

extern CMenuManager FrontEndMenuManager;

#endif
