#include "common.h"
#if defined DETECT_JOYSTICK_MENU && defined XINPUT
#include <windows.h>
#include <xinput.h>
#if !defined(PSAPI_VERSION) || (PSAPI_VERSION > 1)
#pragma comment( lib, "Xinput9_1_0.lib" )
#else
#pragma comment( lib, "Xinput.lib" )
#endif
#endif
#include "platform.h"
#include "crossplatform.h"
#include "Renderer.h"
#include "Frontend.h"
#include "Font.h"
#include "Camera.h"
#include "main.h"
#include "MBlur.h"
#include "postfx.h"
#include "custompipes.h"
#include "RwHelper.h"
#include "Text.h"
#include "Streaming.h"
#include "FileLoader.h"
#include "Collision.h"
#include "ModelInfo.h"
#include "Pad.h"
#include "ControllerConfig.h"

// Menu screens array is at the bottom of the file.

#ifdef PC_MENU

#ifdef CUSTOM_FRONTEND_OPTIONS

#ifdef IMPROVED_VIDEOMODE
	#define VIDEOMODE_SELECTOR MENUACTION_CFO_SELECT, "FEM_SCF", { new CCFOSelect((int8*)&FrontEndMenuManager.m_nPrefsWindowed, "VideoMode", "Windowed", screenModes, 2, true, ScreenModeAfterChange, true) },
#else
	#define VIDEOMODE_SELECTOR
#endif

#ifdef MULTISAMPLING
	#define MULTISAMPLING_SELECTOR MENUACTION_CFO_DYNAMIC, "FED_AAS", { new CCFODynamic((int8*)&FrontEndMenuManager.m_nPrefsMSAALevel, "Graphics", "MultiSampling", MultiSamplingDraw, MultiSamplingButtonPress) },
#else
	#define MULTISAMPLING_SELECTOR
#endif

#ifdef CUTSCENE_BORDERS_SWITCH
	#define CUTSCENE_BORDERS_TOGGLE MENUACTION_CFO_SELECT, "FEM_CSB", { new CCFOSelect((int8 *)&CMenuManager::m_PrefsCutsceneBorders, "Display", "CutsceneBorders", off_on, 2, false) },
#else
	#define CUTSCENE_BORDERS_TOGGLE
#endif

#ifdef FREE_CAM
	#define FREE_CAM_TOGGLE MENUACTION_CFO_SELECT, "FEC_FRC", { new CCFOSelect((int8*)&TheCamera.bFreeCam, "Display", "FreeCam", off_on, 2, false) },
#else
	#define FREE_CAM_TOGGLE
#endif

#ifdef PS2_ALPHA_TEST
	#define DUALPASS_SELECTOR MENUACTION_CFO_SELECT, "FEM_2PR", { new CCFOSelect((int8*)&gPS2alphaTest, "Graphics", "PS2AlphaTest", off_on, 2, false) },
#else
	#define DUALPASS_SELECTOR 
#endif

#ifdef NO_ISLAND_LOADING
	#define ISLAND_LOADING_SELECTOR MENUACTION_CFO_SELECT, "FEM_ISL", { new CCFOSelect((int8*)&CMenuManager::m_PrefsIslandLoading, "Graphics", "IslandLoading", islandLoadingOpts, ARRAY_SIZE(islandLoadingOpts), true, IslandLoadingAfterChange) },
#else
	#define ISLAND_LOADING_SELECTOR 
#endif

#ifdef EXTENDED_COLOURFILTER
	#define POSTFX_SELECTORS \
		MENUACTION_CFO_SELECT, "FED_CLF", { new CCFOSelect((int8*)&CPostFX::EffectSwitch, "Graphics", "ColourFilter", filterNames, ARRAY_SIZE(filterNames), false) }, \
		MENUACTION_CFO_SELECT, "FED_MBL", { new CCFOSelect((int8*)&CPostFX::MotionBlurOn, "Graphics", "MotionBlur", off_on, 2, false) },
#else
	#define POSTFX_SELECTORS
#endif	

#ifdef INVERT_LOOK_FOR_PAD
	#define INVERT_PAD_SELECTOR MENUACTION_CFO_SELECT, "FEC_IVP", { new CCFOSelect((int8*)&CPad::bInvertLook4Pad, "Controller", "InvertPad", off_on, 2, false) },
#else
	#define INVERT_PAD_SELECTOR
#endif

#ifdef GAMEPAD_MENU
	#define SELECT_CONTROLLER_TYPE  MENUACTION_CFO_SELECT, "FEC_TYP", { new CCFOSelect((int8*)&CMenuManager::m_PrefsControllerType, "Controller", "Type", controllerTypes, ARRAY_SIZE(controllerTypes), false, ControllerTypeAfterChange) },
#else
	#define SELECT_CONTROLLER_TYPE
#endif

const char *filterNames[] = { "FEM_NON", "FEM_SIM", "FEM_NRM", "FEM_MOB" };
const char *off_on[] = { "FEM_OFF", "FEM_ON" };

void RestoreDefGraphics(int8 action) {
	if (action != FEOPTION_ACTION_SELECT)
		return;

	#ifdef PS2_ALPHA_TEST
		gPS2alphaTest = false;
	#endif
	#ifdef MULTISAMPLING
		FrontEndMenuManager.m_nPrefsMSAALevel = FrontEndMenuManager.m_nDisplayMSAALevel = 0;
	#endif
	#ifdef NO_ISLAND_LOADING
	    	if (!FrontEndMenuManager.m_bGameNotLoaded) {
	    		FrontEndMenuManager.m_PrefsIslandLoading = FrontEndMenuManager.ISLAND_LOADING_LOW;
			CCollision::bAlreadyLoaded = false;
			CModelInfo::RemoveColModelsFromOtherLevels(CGame::currLevel);
			CStreaming::RemoveUnusedBigBuildings(CGame::currLevel);
			CStreaming::RemoveUnusedBuildings(CGame::currLevel);
			CStreaming::RequestIslands(CGame::currLevel);
			CStreaming::LoadAllRequestedModels(true);
	    	} else
	    		FrontEndMenuManager.m_PrefsIslandLoading = FrontEndMenuManager.ISLAND_LOADING_LOW;
	#endif
	#ifdef GRAPHICS_MENU_OPTIONS // otherwise Frontend will handle those
		CMenuManager::m_PrefsFrameLimiter = true;
		CMenuManager::m_PrefsVsyncDisp = true;
		CMenuManager::m_PrefsVsync = true;
		CMenuManager::m_PrefsUseWideScreen = false;
		FrontEndMenuManager.m_nDisplayVideoMode = FrontEndMenuManager.m_nPrefsVideoMode;
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
		FrontEndMenuManager.SaveSettings();
	#endif
}

void RestoreDefDisplay(int8 action) {
	if (action != FEOPTION_ACTION_SELECT)
		return;

	#ifdef CUTSCENE_BORDERS_SWITCH
		CMenuManager::m_PrefsCutsceneBorders = true;
	#endif
	#ifdef FREE_CAM
		TheCamera.bFreeCam = false;
	#endif
	#ifdef GRAPHICS_MENU_OPTIONS // otherwise Frontend will handle those
		CMenuManager::m_PrefsBrightness = 256;
		CMenuManager::m_PrefsLOD = 1.2f;
		CRenderer::ms_lodDistScale = 1.2f;
		CMenuManager::m_PrefsShowSubtitles = true;
		FrontEndMenuManager.SaveSettings();
	#endif
}

#ifdef NO_ISLAND_LOADING
const char *islandLoadingOpts[] = { "FEM_LOW", "FEM_MED", "FEM_HIG" };
void IslandLoadingAfterChange(int8 before, int8 after) {
	if (!FrontEndMenuManager.m_bGameNotLoaded) {
		if (after > FrontEndMenuManager.ISLAND_LOADING_LOW) {
		    FrontEndMenuManager.m_PrefsIslandLoading = before; // calls below needs previous mode :shrug:
		    
		    if (after == FrontEndMenuManager.ISLAND_LOADING_HIGH)
			    CStreaming::RemoveIslandsNotUsed(LEVEL_GENERIC);
		    if (before == FrontEndMenuManager.ISLAND_LOADING_LOW) {
			    if (CGame::currLevel != LEVEL_INDUSTRIAL)
				    CFileLoader::LoadCollisionFromDatFile(LEVEL_INDUSTRIAL);
			    if (CGame::currLevel != LEVEL_COMMERCIAL)
				    CFileLoader::LoadCollisionFromDatFile(LEVEL_COMMERCIAL);
			    if (CGame::currLevel != LEVEL_SUBURBAN)
				    CFileLoader::LoadCollisionFromDatFile(LEVEL_SUBURBAN);
			    CCollision::bAlreadyLoaded = true;
			    FrontEndMenuManager.m_PrefsIslandLoading = after;
			    CStreaming::RequestBigBuildings(CGame::currLevel);
			    
		    } else if (before == FrontEndMenuManager.ISLAND_LOADING_HIGH) {
			    FrontEndMenuManager.m_PrefsIslandLoading = after;
			    CStreaming::RequestIslands(CGame::currLevel);
		    } else
		    	    FrontEndMenuManager.m_PrefsIslandLoading = after;
		    	    
		} else { // low
		    CCollision::bAlreadyLoaded = false;
		    CModelInfo::RemoveColModelsFromOtherLevels(CGame::currLevel);
		    CStreaming::RemoveUnusedBigBuildings(CGame::currLevel);
		    CStreaming::RemoveUnusedBuildings(CGame::currLevel);
		    CStreaming::RequestIslands(CGame::currLevel);
		}

		CStreaming::LoadAllRequestedModels(true);
	}

	FrontEndMenuManager.SetHelperText(0);
}
#endif

#ifndef MULTISAMPLING
void GraphicsGoBack() {
}
#else
void GraphicsGoBack() {
	FrontEndMenuManager.m_nDisplayMSAALevel = FrontEndMenuManager.m_nPrefsMSAALevel;
}

void MultiSamplingButtonPress(int8 action) {
	if (action == FEOPTION_ACTION_SELECT) {
		if (FrontEndMenuManager.m_nDisplayMSAALevel != FrontEndMenuManager.m_nPrefsMSAALevel) {
			FrontEndMenuManager.m_nPrefsMSAALevel = FrontEndMenuManager.m_nDisplayMSAALevel;
			_psSelectScreenVM(FrontEndMenuManager.m_nPrefsVideoMode);
			FrontEndMenuManager.SetHelperText(0);
			FrontEndMenuManager.SaveSettings();
		}
	} else if (action == FEOPTION_ACTION_LEFT || action == FEOPTION_ACTION_RIGHT) {
		if (FrontEndMenuManager.m_bGameNotLoaded) {
			FrontEndMenuManager.m_nDisplayMSAALevel += (action == FEOPTION_ACTION_RIGHT ? 1 : -1);

			int i = 0;
			int maxAA = RwD3D8EngineGetMaxMultiSamplingLevels();
			while (maxAA != 1) {
				i++;
				maxAA >>= 1;
			}

			if (FrontEndMenuManager.m_nDisplayMSAALevel < 0)
				FrontEndMenuManager.m_nDisplayMSAALevel = i;
			else if (FrontEndMenuManager.m_nDisplayMSAALevel > i)
				FrontEndMenuManager.m_nDisplayMSAALevel = 0;
		}
	} else if (action == FEOPTION_ACTION_FOCUSLOSS) {
		if (FrontEndMenuManager.m_nDisplayMSAALevel != FrontEndMenuManager.m_nPrefsMSAALevel) {
			FrontEndMenuManager.m_nDisplayMSAALevel = FrontEndMenuManager.m_nPrefsMSAALevel;
			FrontEndMenuManager.SetHelperText(3);
		}
	}
}

wchar* MultiSamplingDraw(bool *disabled, bool userHovering) {
	static wchar unicodeTemp[64];
	if (userHovering) {
		if (FrontEndMenuManager.m_nDisplayMSAALevel == FrontEndMenuManager.m_nPrefsMSAALevel) {
			if (FrontEndMenuManager.m_nHelperTextMsgId == 1) // Press enter to apply
				FrontEndMenuManager.ResetHelperText();
		} else {
			FrontEndMenuManager.SetHelperText(1);
		}
	} else {
		if (FrontEndMenuManager.m_nDisplayMSAALevel != FrontEndMenuManager.m_nPrefsMSAALevel) {
			FrontEndMenuManager.m_nDisplayMSAALevel = FrontEndMenuManager.m_nPrefsMSAALevel;
		}
	}

	if (!FrontEndMenuManager.m_bGameNotLoaded)
		*disabled = true;

	switch (FrontEndMenuManager.m_nDisplayMSAALevel) {
		case 0:
			return TheText.Get("FEM_OFF");
		default:
			sprintf(gString, "%iX", 1 << (FrontEndMenuManager.m_nDisplayMSAALevel));
			AsciiToUnicode(gString, unicodeTemp);
			return unicodeTemp;
	}
}
#endif

#ifdef IMPROVED_VIDEOMODE
const char* screenModes[] = { "FED_FLS", "FED_WND" };
void ScreenModeAfterChange(int8 before, int8 after)
{
	_psSelectScreenVM(FrontEndMenuManager.m_nPrefsVideoMode); // apply same resolution
	FrontEndMenuManager.SetHelperText(0);
}

#endif

#ifdef DETECT_JOYSTICK_MENU
wchar selectedJoystickUnicode[128];
int cachedButtonNum = -1;

wchar* DetectJoystickDraw(bool* disabled, bool userHovering) {

#if defined RW_GL3 && !defined LIBRW_SDL2
	int numButtons;
	int found = -1;
	const char *joyname;
	if (userHovering) {
		for (int i = 0; i <= GLFW_JOYSTICK_LAST; i++) {
			if ((joyname = glfwGetJoystickName(i))) {
				const uint8* buttons = glfwGetJoystickButtons(i, &numButtons);
				for (int j = 0; j < numButtons; j++) {
					if (buttons[j]) {
						found = i;
						break;
					}
				}
				if (found != -1)
					break;
			}
		}

		if (found != -1 && PSGLOBAL(joy1id) != found) {
			if (PSGLOBAL(joy1id) != -1 && PSGLOBAL(joy1id) != found)
				PSGLOBAL(joy2id) = PSGLOBAL(joy1id);
			else
				PSGLOBAL(joy2id) = -1;

			strcpy(gSelectedJoystickName, joyname);
			PSGLOBAL(joy1id) = found;
			cachedButtonNum = numButtons;
		}
	}
	if (PSGLOBAL(joy1id) == -1)
#elif defined XINPUT
	int found = -1;
	XINPUT_STATE xstate;
	memset(&xstate, 0, sizeof(XINPUT_STATE));
	if (userHovering) {
		for (int i = 0; i <= 3; i++) {
			if (XInputGetState(i, &xstate) == ERROR_SUCCESS) {
				if (xstate.Gamepad.bLeftTrigger || xstate.Gamepad.bRightTrigger) {
					found = i;
					break;
				}
				for (int j = XINPUT_GAMEPAD_DPAD_UP; j != XINPUT_GAMEPAD_Y << 1; j = (j << 1)) {
					if (xstate.Gamepad.wButtons & j) {
						found = i;
						break;
					}
				}
				if (found != -1)
					break;
			}
		}
		if (found != -1 && CPad::XInputJoy1 != found) {
			// We should never leave pads -1, so we can process them when they're connected and kinda support hotplug.
			CPad::XInputJoy2 = (CPad::XInputJoy1 == -1 ? (found + 1) % 4 : CPad::XInputJoy1);
			CPad::XInputJoy1 = found;
			cachedButtonNum = 0; // fake too, because xinput bypass CControllerConfig
		}
	}
	sprintf(gSelectedJoystickName, "%d", CPad::XInputJoy1); // fake, on xinput we only store gamepad ids(thanks MS) so this is a temp variable to be used below
	if (CPad::XInputJoy1 == -1)
#endif
		AsciiToUnicode("Not found", selectedJoystickUnicode);
	else
		AsciiToUnicode(gSelectedJoystickName, selectedJoystickUnicode);

	return selectedJoystickUnicode;
}

void DetectJoystickGoBack() {
	if (cachedButtonNum != -1) {
#ifdef LOAD_INI_SETTINGS
		ControlsManager.InitDefaultControlConfigJoyPad(cachedButtonNum);
		SaveINIControllerSettings();
#else
		// Otherwise no way to save gSelectedJoystickName or ms_padButtonsInited anyway :shrug: Why do you even use this config.??
#endif
		cachedButtonNum = -1;
	}
}
#endif

#ifdef GAMEPAD_MENU
const char* controllerTypes[] = { "FEC_DS2", "FEC_DS3", "FEC_DS4", "FEC_360", "FEC_ONE" };
void ControllerTypeAfterChange(int8 before, int8 after)
{
	FrontEndMenuManager.LoadController(after);
}
#endif

CMenuScreenCustom aScreens[MENUPAGES] = {
	// MENUPAGE_NONE = 0
	{ "", MENUPAGE_DISABLED, MENUPAGE_DISABLED, nil, nil, },

	// MENUPAGE_STATS = 1
	{ "FET_STA", MENUPAGE_NONE, MENUPAGE_NONE, nil, nil,
		MENUACTION_CHANGEMENU, "FEDS_TB", { nil, SAVESLOT_NONE, MENUPAGE_NONE },
	},

	// MENUPAGE_NEW_GAME = 2
	{ "FET_SGA", MENUPAGE_NONE, MENUPAGE_NONE, nil, nil,
		MENUACTION_CHANGEMENU, "FES_SNG", { nil, SAVESLOT_NONE, MENUPAGE_NEW_GAME_RELOAD },
		MENUACTION_POPULATESLOTS_CHANGEMENU, "GMLOAD",  { nil, SAVESLOT_NONE, MENUPAGE_CHOOSE_LOAD_SLOT },
		MENUACTION_POPULATESLOTS_CHANGEMENU, "FES_DGA", { nil, SAVESLOT_NONE, MENUPAGE_CHOOSE_DELETE_SLOT },
		MENUACTION_CHANGEMENU, "FEDS_TB", { nil, SAVESLOT_NONE, MENUPAGE_NONE },
	},

	// MENUPAGE_BRIEFS = 3
	{ "FET_BRE", MENUPAGE_NONE, MENUPAGE_NONE, nil, nil,
		MENUACTION_CHANGEMENU, "FEDS_TB", { nil, SAVESLOT_NONE, MENUPAGE_NONE },
	},

	// MENUPAGE_CONTROLLER_SETTINGS = 4
#ifdef GAMEPAD_MENU
	{ "FET_AGS", MENUPAGE_CONTROLLER_PC, MENUPAGE_CONTROLLER_PC, nil, nil,
#else
	{ "FET_CON", MENUPAGE_OPTIONS, MENUPAGE_OPTIONS, nil, nil,
#endif
		MENUACTION_CTRLCONFIG,		"FEC_CCF", { nil, SAVESLOT_NONE, MENUPAGE_CONTROLLER_SETTINGS },
		MENUACTION_CTRLDISPLAY,		"FEC_CDP", { nil, SAVESLOT_NONE, MENUPAGE_CONTROLLER_SETTINGS },
		INVERT_PAD_SELECTOR
		MENUACTION_CTRLVIBRATION,	"FEC_VIB", { nil, SAVESLOT_NONE, MENUPAGE_CONTROLLER_SETTINGS },
		SELECT_CONTROLLER_TYPE
		MENUACTION_CHANGEMENU,		"FEDS_TB", { nil, SAVESLOT_NONE, MENUPAGE_NONE },
	},

	// MENUPAGE_SOUND_SETTINGS = 5
	{ "FET_AUD", MENUPAGE_OPTIONS, MENUPAGE_OPTIONS, nil, nil,
		MENUACTION_MUSICVOLUME,		"FEA_MUS", { nil, SAVESLOT_NONE, MENUPAGE_SOUND_SETTINGS },
		MENUACTION_SFXVOLUME,		"FEA_SFX", { nil, SAVESLOT_NONE, MENUPAGE_SOUND_SETTINGS },
		MENUACTION_AUDIOHW,			"FEA_3DH", { nil, SAVESLOT_NONE, MENUPAGE_SOUND_SETTINGS },
		MENUACTION_SPEAKERCONF,		"FEA_SPK", { nil, SAVESLOT_NONE, MENUPAGE_SOUND_SETTINGS },
		MENUACTION_DYNAMICACOUSTIC,	"FET_DAM", { nil, SAVESLOT_NONE, MENUPAGE_SOUND_SETTINGS },
		MENUACTION_RADIO,			"FEA_RSS", { nil, SAVESLOT_NONE, MENUPAGE_SOUND_SETTINGS },
		MENUACTION_RESTOREDEF,		"FET_DEF", { nil, SAVESLOT_NONE, MENUPAGE_SOUND_SETTINGS },
		MENUACTION_CHANGEMENU,		"FEDS_TB", { nil, SAVESLOT_NONE, MENUPAGE_NONE },
	},

#ifndef GRAPHICS_MENU_OPTIONS
	// MENUPAGE_DISPLAY_SETTINGS = 6
	{ "FET_DIS", MENUPAGE_OPTIONS, MENUPAGE_OPTIONS, nil, nil,
		MENUACTION_BRIGHTNESS,	"FED_BRI", { nil, SAVESLOT_NONE, MENUPAGE_DISPLAY_SETTINGS },
		MENUACTION_DRAWDIST,	"FEM_LOD", { nil, SAVESLOT_NONE, MENUPAGE_DISPLAY_SETTINGS },
		MENUACTION_FRAMESYNC,	"FEM_VSC", { nil, SAVESLOT_NONE, MENUPAGE_DISPLAY_SETTINGS },
		MENUACTION_FRAMELIMIT,	"FEM_FRM", { nil, SAVESLOT_NONE, MENUPAGE_DISPLAY_SETTINGS },
#ifndef EXTENDED_COLOURFILTER
		MENUACTION_TRAILS,		"FED_TRA", { nil, SAVESLOT_NONE, MENUPAGE_DISPLAY_SETTINGS },
#endif
		MENUACTION_SUBTITLES,	"FED_SUB", { nil, SAVESLOT_NONE, MENUPAGE_DISPLAY_SETTINGS },
		MENUACTION_WIDESCREEN,	"FED_WIS", { nil, SAVESLOT_NONE, MENUPAGE_DISPLAY_SETTINGS },
		MENUACTION_SCREENRES,	"FED_RES", { nil, SAVESLOT_NONE, MENUPAGE_DISPLAY_SETTINGS },
		VIDEOMODE_SELECTOR
		MULTISAMPLING_SELECTOR
		ISLAND_LOADING_SELECTOR
		DUALPASS_SELECTOR
		CUTSCENE_BORDERS_TOGGLE
		FREE_CAM_TOGGLE
		POSTFX_SELECTORS
		// re3.cpp inserts here pipeline selectors if neo/neo.txd exists and EXTENDED_PIPELINES defined
		MENUACTION_RESTOREDEF,	"FET_DEF", { nil, SAVESLOT_NONE, MENUPAGE_DISPLAY_SETTINGS },
		MENUACTION_CHANGEMENU,	"FEDS_TB", { nil, SAVESLOT_NONE, MENUPAGE_NONE },
	},
#else
	// MENUPAGE_DISPLAY_SETTINGS = 6
	{ "FET_DIS", MENUPAGE_OPTIONS, MENUPAGE_OPTIONS, nil, nil,
		MENUACTION_BRIGHTNESS,	"FED_BRI", { nil, SAVESLOT_NONE, MENUPAGE_DISPLAY_SETTINGS },
		MENUACTION_DRAWDIST,	"FEM_LOD", { nil, SAVESLOT_NONE, MENUPAGE_DISPLAY_SETTINGS },
		CUTSCENE_BORDERS_TOGGLE
		FREE_CAM_TOGGLE
		MENUACTION_SUBTITLES,	"FED_SUB", { nil, SAVESLOT_NONE, MENUPAGE_DISPLAY_SETTINGS },
		MENUACTION_CFO_DYNAMIC,	"FET_DEF", { new CCFODynamic(nil, nil, nil, nil, RestoreDefDisplay) },
		MENUACTION_CHANGEMENU,	"FEDS_TB", { nil, SAVESLOT_NONE, MENUPAGE_NONE },
	},
#endif

	// MENUPAGE_LANGUAGE_SETTINGS = 7
	{ "FET_LAN", MENUPAGE_OPTIONS, MENUPAGE_OPTIONS, nil, nil,
		MENUACTION_LANG_ENG,	"FEL_ENG", { nil, SAVESLOT_NONE, MENUPAGE_LANGUAGE_SETTINGS },
		MENUACTION_LANG_FRE,	"FEL_FRE", { nil, SAVESLOT_NONE, MENUPAGE_LANGUAGE_SETTINGS },
		MENUACTION_LANG_GER,	"FEL_GER", { nil, SAVESLOT_NONE, MENUPAGE_LANGUAGE_SETTINGS },
		MENUACTION_LANG_ITA,	"FEL_ITA", { nil, SAVESLOT_NONE, MENUPAGE_LANGUAGE_SETTINGS },
		MENUACTION_LANG_SPA,    "FEL_SPA", { nil, SAVESLOT_NONE, MENUPAGE_LANGUAGE_SETTINGS },
		// CustomFrontendOptionsPopulate will add languages here, if files are found
		MENUACTION_CHANGEMENU,	"FEDS_TB", { nil, SAVESLOT_NONE, MENUPAGE_NONE },
	},

	// MENUPAGE_CHOOSE_LOAD_SLOT = 8
	{ "FET_LG", MENUPAGE_NEW_GAME, MENUPAGE_NEW_GAME, nil, nil,
		MENUACTION_CHANGEMENU,	"FESZ_CA", { nil, SAVESLOT_NONE,	MENUPAGE_NEW_GAME },
		MENUACTION_CHECKSAVE,	"FEM_SL0", { nil, SAVESLOT_1,		MENUPAGE_LOAD_SLOT_CONFIRM },
		MENUACTION_CHECKSAVE,	"FEM_SL1", { nil, SAVESLOT_2,		MENUPAGE_LOAD_SLOT_CONFIRM },
		MENUACTION_CHECKSAVE,	"FEM_SL2", { nil, SAVESLOT_3,		MENUPAGE_LOAD_SLOT_CONFIRM },
		MENUACTION_CHECKSAVE,	"FEM_SL3", { nil, SAVESLOT_4,		MENUPAGE_LOAD_SLOT_CONFIRM },
		MENUACTION_CHECKSAVE,	"FEM_SL4", { nil, SAVESLOT_5,		MENUPAGE_LOAD_SLOT_CONFIRM },
		MENUACTION_CHECKSAVE,	"FEM_SL5", { nil, SAVESLOT_6,		MENUPAGE_LOAD_SLOT_CONFIRM },
		MENUACTION_CHECKSAVE,	"FEM_SL6", { nil, SAVESLOT_7,		MENUPAGE_LOAD_SLOT_CONFIRM },
		MENUACTION_CHECKSAVE,	"FEM_SL7", { nil, SAVESLOT_8,		MENUPAGE_LOAD_SLOT_CONFIRM },
	},

	// MENUPAGE_CHOOSE_DELETE_SLOT = 9
	{ "FET_DG", MENUPAGE_NEW_GAME, MENUPAGE_NEW_GAME, nil, nil,
		MENUACTION_CHANGEMENU,	"FESZ_CA",	{ nil, SAVESLOT_NONE,	MENUPAGE_NEW_GAME },
		MENUACTION_CHANGEMENU,	"FEM_SL0",	{ nil, SAVESLOT_1,		MENUPAGE_DELETE_SLOT_CONFIRM },
		MENUACTION_CHANGEMENU,	"FEM_SL1",	{ nil, SAVESLOT_2,		MENUPAGE_DELETE_SLOT_CONFIRM },
		MENUACTION_CHANGEMENU,	"FEM_SL2",	{ nil, SAVESLOT_3,		MENUPAGE_DELETE_SLOT_CONFIRM },
		MENUACTION_CHANGEMENU,	"FEM_SL3",	{ nil, SAVESLOT_4,		MENUPAGE_DELETE_SLOT_CONFIRM },
		MENUACTION_CHANGEMENU,	"FEM_SL4",	{ nil, SAVESLOT_5,		MENUPAGE_DELETE_SLOT_CONFIRM },
		MENUACTION_CHANGEMENU,	"FEM_SL5",	{ nil, SAVESLOT_6,		MENUPAGE_DELETE_SLOT_CONFIRM },
		MENUACTION_CHANGEMENU,	"FEM_SL6",	{ nil, SAVESLOT_7,		MENUPAGE_DELETE_SLOT_CONFIRM },
		MENUACTION_CHANGEMENU,	"FEM_SL7",	{ nil, SAVESLOT_8,		MENUPAGE_DELETE_SLOT_CONFIRM },
	},

	// MENUPAGE_NEW_GAME_RELOAD = 10
	{ "FET_NG", MENUPAGE_NEW_GAME, MENUPAGE_NEW_GAME, nil, nil,
		MENUACTION_LABEL,		"FESZ_QR",	{ nil, SAVESLOT_NONE,	MENUPAGE_NONE },
		MENUACTION_CHANGEMENU,	"FEM_NO",	{ nil, SAVESLOT_NONE,	MENUPAGE_NEW_GAME },
		MENUACTION_NEWGAME,		"FEM_YES",	{ nil, SAVESLOT_NONE,	MENUPAGE_NEW_GAME_RELOAD },
	},

	// MENUPAGE_LOAD_SLOT_CONFIRM = 11
	{ "FET_LG", MENUPAGE_CHOOSE_LOAD_SLOT, MENUPAGE_CHOOSE_LOAD_SLOT, nil, nil,
		 MENUACTION_LABEL,		"FESZ_QL",	{ nil, SAVESLOT_NONE,	MENUPAGE_NONE },
		 MENUACTION_CHANGEMENU,	"FEM_NO",	{ nil, SAVESLOT_NONE,	MENUPAGE_CHOOSE_LOAD_SLOT },
		 MENUACTION_CHANGEMENU,	"FEM_YES",	{ nil, SAVESLOT_NONE,	MENUPAGE_LOADING_IN_PROGRESS },
	},

	// MENUPAGE_DELETE_SLOT_CONFIRM = 12
	{ "FET_DG", MENUPAGE_CHOOSE_DELETE_SLOT, MENUPAGE_CHOOSE_DELETE_SLOT, nil, nil,
		 MENUACTION_LABEL,		"FESZ_QD",	{ nil, SAVESLOT_NONE,  MENUPAGE_NONE },
		 MENUACTION_CHANGEMENU,	"FEM_NO",	{ nil, SAVESLOT_NONE,  MENUPAGE_CHOOSE_DELETE_SLOT },
		 MENUACTION_CHANGEMENU,	"FEM_YES",	{ nil, SAVESLOT_NONE,	MENUPAGE_DELETING },
	},

	// MENUPAGE_NO_MEMORY_CARD = 13
	{ "FES_NOC", MENUPAGE_DISABLED, MENUPAGE_DISABLED, nil, nil,
		// hud adjustment page in mobile
	},

	// MENUPAGE_LOADING_IN_PROGRESS = 14
	{ "FET_LG", MENUPAGE_DISABLED, MENUPAGE_DISABLED, nil, nil,
		MENUACTION_LABEL, "FED_LDW", { nil, SAVESLOT_NONE, MENUPAGE_LOAD_SLOT_CONFIRM },
	},

	// MENUPAGE_DELETING_IN_PROGRESS = 15
	{ "FET_DG", MENUPAGE_DISABLED, MENUPAGE_DISABLED, nil, nil,
		MENUACTION_LABEL, "FEDL_WR", { nil, SAVESLOT_NONE, MENUPAGE_NONE },
	},

	// MENUPAGE_PS2_LOAD_FAILED = 16
	{ "FET_LG", MENUPAGE_DISABLED, MENUPAGE_DISABLED, nil, nil,
		MENUACTION_LABEL, "FES_LOE", { nil, SAVESLOT_NONE, MENUPAGE_NONE },
	},

	// MENUPAGE_DELETE_FAILED = 17
	{ "FET_DG", MENUPAGE_DISABLED, MENUPAGE_DISABLED, nil, nil,
		MENUACTION_LABEL, "FES_DEE", { nil, SAVESLOT_NONE, MENUPAGE_NONE },
		MENUACTION_CHANGEMENU, "FEC_OKK", { nil, SAVESLOT_NONE, MENUPAGE_CHOOSE_DELETE_SLOT },
	},

	// MENUPAGE_DEBUG_MENU = 18
	{ "FED_DBG", MENUPAGE_NONE, MENUPAGE_NONE, nil, nil,
		MENUACTION_RELOADIDE,	"FED_RID", { nil, SAVESLOT_NONE, MENUPAGE_NONE },
		MENUACTION_RELOADIPL,	"FED_RIP", { nil, SAVESLOT_NONE, MENUPAGE_NONE },
		MENUACTION_SETDBGFLAG,	"FED_DFL", { nil, SAVESLOT_NONE, MENUPAGE_NONE },
		MENUACTION_SWITCHBIGWHITEDEBUGLIGHT,	"FED_DLS", { nil, SAVESLOT_NONE, MENUPAGE_NONE },
		MENUACTION_PEDROADGROUPS,	"FED_SPR", { nil, SAVESLOT_NONE, MENUPAGE_NONE },
		MENUACTION_CARROADGROUPS,	"FED_SCR", { nil, SAVESLOT_NONE, MENUPAGE_NONE },
		MENUACTION_COLLISIONPOLYS,	"FED_SCP", { nil, SAVESLOT_NONE, MENUPAGE_NONE },
		MENUACTION_PARSEHEAP,	"FED_PAH", { nil, SAVESLOT_NONE, MENUPAGE_NONE },
		MENUACTION_SHOWCULL,	"FED_SCZ", { nil, SAVESLOT_NONE, MENUPAGE_NONE },
		MENUACTION_DEBUGSTREAM,	"FED_DSR", { nil, SAVESLOT_NONE, MENUPAGE_NONE },
		MENUACTION_CHANGEMENU,	"FEDS_TB", { nil, SAVESLOT_NONE, MENUPAGE_NONE },
	},

	// MENUPAGE_MEMORY_CARD_DEBUG = 19
	{ "FEM_MCM", MENUPAGE_NONE, MENUPAGE_NONE, nil, nil,
		MENUACTION_REGMEMCARD1,	"FEM_RMC", { nil, SAVESLOT_NONE, MENUPAGE_NONE },
		MENUACTION_TESTFORMATMEMCARD1,	"FEM_TFM", { nil, SAVESLOT_NONE, MENUPAGE_NONE },
		MENUACTION_TESTUNFORMATMEMCARD1,	"FEM_TUM", { nil, SAVESLOT_NONE, MENUPAGE_NONE },
		MENUACTION_CREATEROOTDIR,	"FEM_CRD", { nil, SAVESLOT_NONE, MENUPAGE_NONE },
		MENUACTION_CREATELOADICONS,	"FEM_CLI", { nil, SAVESLOT_NONE, MENUPAGE_NONE },
		MENUACTION_FILLWITHGUFF,	"FEM_FFF", { nil, SAVESLOT_NONE, MENUPAGE_NONE },
		MENUACTION_SAVEONLYTHEGAME,	"FEM_SOG", { nil, SAVESLOT_NONE, MENUPAGE_NONE },
		MENUACTION_SAVEGAME,	"FEM_STG", { nil, SAVESLOT_NONE, MENUPAGE_NONE },
		MENUACTION_SAVEGAMEUNDERGTA,	"FEM_STS", { nil, SAVESLOT_NONE, MENUPAGE_NONE },
		MENUACTION_CREATECOPYPROTECTED,	"FEM_CPD", { nil, SAVESLOT_NONE, MENUPAGE_NONE },
	},

	// MENUPAGE_MEMORY_CARD_TEST = 20
	{ "FEM_MC2", MENUPAGE_DISABLED, MENUPAGE_DISABLED, nil, nil,

	},

	// MENUPAGE_MULTIPLAYER_MAIN = 21
	{ "FET_MP", MENUPAGE_DISABLED, MENUPAGE_DISABLED, nil, nil,

	},

	// MENUPAGE_PS2_SAVE_FAILED = 22
	{ "MCDNSP", MENUPAGE_DISABLED, MENUPAGE_DISABLED, nil, nil,
		MENUACTION_MEMCARDSAVECONFIRM, "JAILB_U", { nil, SAVESLOT_NONE, MENUPAGE_NONE },
	},

	// MENUPAGE_PS2_SAVE_FAILED_2 = 23
	{ "MCGNSP", MENUPAGE_DISABLED, MENUPAGE_DISABLED, nil, nil,
		MENUACTION_MEMCARDSAVECONFIRM, "JAILB_U", { nil, SAVESLOT_NONE, MENUPAGE_NONE },
	},

	// Unused in PC but anyway
	// MENUPAGE_SAVE = 24
#ifdef PS2_SAVE_DIALOG
	{ "FET_SG", MENUPAGE_DISABLED, MENUPAGE_DISABLED, nil, nil,
		MENUACTION_CHANGEMENU,			"FESZ_SA",	{ nil, SAVESLOT_NONE, MENUPAGE_CHOOSE_SAVE_SLOT },
		MENUACTION_RESUME_FROM_SAVEZONE,	"FESZ_CA",	{ nil, SAVESLOT_NONE, MENUPAGE_NONE },
	},
#else
	{ "FET_SG", MENUPAGE_DISABLED, MENUPAGE_DISABLED, nil, nil,
		MENUACTION_LABEL,				"FES_SCG",	{ nil, SAVESLOT_NONE, MENUPAGE_NONE },
		MENUACTION_POPULATESLOTS_CHANGEMENU,			"GMSAVE",	{ nil, SAVESLOT_NONE, MENUPAGE_CHOOSE_SAVE_SLOT },
		MENUACTION_RESUME_FROM_SAVEZONE,	"FESZ_CA",	{ nil, SAVESLOT_NONE, MENUPAGE_NONE },
	},
#endif

	// MENUPAGE_NO_MEMORY_CARD_2 = 25
	{ "FES_NOC", MENUPAGE_DISABLED, MENUPAGE_DISABLED, nil, nil,
		MENUACTION_CHANGEMENU,	"FESZ_CA",	{ nil, SAVESLOT_NONE, MENUPAGE_NONE },
	},

	// MENUPAGE_CHOOSE_SAVE_SLOT = 26
	{ "FET_SG", MENUPAGE_DISABLED, MENUPAGE_DISABLED, nil, nil,
		MENUACTION_RESUME_FROM_SAVEZONE,	"FESZ_CA", { nil, SAVESLOT_NONE,	MENUPAGE_NONE },
		MENUACTION_CHANGEMENU,			"FEM_SL1", { nil, SAVESLOT_1,		MENUPAGE_SAVE_OVERWRITE_CONFIRM },
		MENUACTION_CHANGEMENU,			"FEM_SL2", { nil, SAVESLOT_2,		MENUPAGE_SAVE_OVERWRITE_CONFIRM },
		MENUACTION_CHANGEMENU,			"FEM_SL3", { nil, SAVESLOT_3,		MENUPAGE_SAVE_OVERWRITE_CONFIRM },
		MENUACTION_CHANGEMENU,			"FEM_SL4", { nil, SAVESLOT_4,		MENUPAGE_SAVE_OVERWRITE_CONFIRM },
		MENUACTION_CHANGEMENU,			"FEM_SL5", { nil, SAVESLOT_5,		MENUPAGE_SAVE_OVERWRITE_CONFIRM },
		MENUACTION_CHANGEMENU,			"FEM_SL6", { nil, SAVESLOT_6,		MENUPAGE_SAVE_OVERWRITE_CONFIRM },
		MENUACTION_CHANGEMENU,			"FEM_SL7", { nil, SAVESLOT_7,		MENUPAGE_SAVE_OVERWRITE_CONFIRM },
		MENUACTION_CHANGEMENU,			"FEM_SL8", { nil, SAVESLOT_8,		MENUPAGE_SAVE_OVERWRITE_CONFIRM },
	},

	// MENUPAGE_SAVE_OVERWRITE_CONFIRM = 27
	{ "FET_SG", MENUPAGE_CHOOSE_SAVE_SLOT, MENUPAGE_CHOOSE_SAVE_SLOT, nil, nil,
		MENUACTION_LABEL,		"FESZ_QO", { nil, SAVESLOT_NONE, MENUPAGE_NONE },
		MENUACTION_CHANGEMENU,	"FEM_YES", { nil, SAVESLOT_NONE, MENUPAGE_SAVING_IN_PROGRESS },
		MENUACTION_CHANGEMENU,	"FEM_NO",  { nil, SAVESLOT_NONE, MENUPAGE_CHOOSE_SAVE_SLOT },
	},

	// MENUPAGE_MULTIPLAYER_MAP = 28
	{ "FET_MAP", MENUPAGE_DISABLED, MENUPAGE_DISABLED, nil, nil,

	},

	// MENUPAGE_MULTIPLAYER_CONNECTION = 29
	{ "FET_CON", MENUPAGE_DISABLED, MENUPAGE_DISABLED, nil, nil,

	},

	// MENUPAGE_MULTIPLAYER_FIND_GAME = 30
	{ "FET_FG", MENUPAGE_DISABLED, MENUPAGE_DISABLED, nil, nil,

	},

	// MENUPAGE_MULTIPLAYER_MODE = 31
	{ "FET_GT", MENUPAGE_DISABLED, MENUPAGE_DISABLED, nil, nil,

	},

	// MENUPAGE_MULTIPLAYER_CREATE = 32
	{ "FET_HG", MENUPAGE_DISABLED, MENUPAGE_DISABLED, nil, nil,

	},

	// MENUPAGE_MULTIPLAYER_START = 33
	{ "FEN_STA", MENUPAGE_DISABLED, MENUPAGE_DISABLED, nil, nil,

	},

	// MENUPAGE_SKIN_SELECT_OLD = 34
	{ "FET_PS", MENUPAGE_DISABLED, MENUPAGE_DISABLED, nil, nil,

	},

	// MENUPAGE_CONTROLLER_PC = 35
	{ "FET_CTL", MENUPAGE_OPTIONS, MENUPAGE_OPTIONS, nil, nil,
#ifdef PC_PLAYER_CONTROLS
		MENUACTION_CTRLMETHOD,	"FET_CME", { nil, SAVESLOT_NONE, MENUPAGE_CONTROLLER_PC },
#endif
		MENUACTION_KEYBOARDCTRLS,"FET_RDK", { nil, SAVESLOT_NONE, MENUPAGE_KEYBOARD_CONTROLS },
#ifdef GAMEPAD_MENU
		MENUACTION_CHANGEMENU, "FET_AGS", { nil, SAVESLOT_NONE, MENUPAGE_CONTROLLER_SETTINGS },
#endif
#ifdef DETECT_JOYSTICK_MENU
		MENUACTION_CHANGEMENU,	"FEC_JOD", { nil, SAVESLOT_NONE, MENUPAGE_DETECT_JOYSTICK },
#endif
		MENUACTION_CHANGEMENU,	"FET_AMS", { nil, SAVESLOT_NONE, MENUPAGE_MOUSE_CONTROLS },
		MENUACTION_RESTOREDEF,	"FET_DEF", { nil, SAVESLOT_NONE, MENUPAGE_CONTROLLER_PC },
		MENUACTION_CHANGEMENU,	"FEDS_TB", { nil, SAVESLOT_NONE, MENUPAGE_NONE },
	},

	// MENUPAGE_CONTROLLER_PC_OLD1 = 36
	{ "FET_CTL", MENUPAGE_CONTROLLER_PC, MENUPAGE_CONTROLLER_PC, nil, nil,
		MENUACTION_GETKEY,	"FEC_PLB", { nil, SAVESLOT_NONE, MENUPAGE_CONTROLLER_PC_OLD1 },
		MENUACTION_GETKEY,	"FEC_CWL", { nil, SAVESLOT_NONE, MENUPAGE_CONTROLLER_PC_OLD1 },
		MENUACTION_GETKEY,	"FEC_CWR", { nil, SAVESLOT_NONE, MENUPAGE_CONTROLLER_PC_OLD1 },
		MENUACTION_GETKEY,	"FEC_LKT", { nil, SAVESLOT_NONE, MENUPAGE_CONTROLLER_PC_OLD1 },
		MENUACTION_GETKEY,	"FEC_PJP", { nil, SAVESLOT_NONE, MENUPAGE_CONTROLLER_PC_OLD1 },
		MENUACTION_GETKEY,	"FEC_PSP", { nil, SAVESLOT_NONE, MENUPAGE_CONTROLLER_PC_OLD1 },
		MENUACTION_GETKEY,	"FEC_TLF", { nil, SAVESLOT_NONE, MENUPAGE_CONTROLLER_PC_OLD1 },
		MENUACTION_GETKEY,	"FEC_TRG", { nil, SAVESLOT_NONE, MENUPAGE_CONTROLLER_PC_OLD1 },
		MENUACTION_GETKEY,	"FEC_CCM", { nil, SAVESLOT_NONE, MENUPAGE_CONTROLLER_PC_OLD1 },
		MENUACTION_CHANGEMENU,	"FEDS_TB", { nil, SAVESLOT_NONE, MENUPAGE_NONE },
	},

	// MENUPAGE_CONTROLLER_PC_OLD2 = 37
	{ "FET_CTL", MENUPAGE_CONTROLLER_PC, MENUPAGE_CONTROLLER_PC, nil, nil,

	},

	// MENUPAGE_CONTROLLER_PC_OLD3 = 38
   { "FET_CTL", MENUPAGE_CONTROLLER_PC, MENUPAGE_CONTROLLER_PC, nil, nil,
		MENUACTION_GETKEY,	"FEC_LUP", { nil, SAVESLOT_NONE, MENUPAGE_CONTROLLER_PC_OLD3 },
		MENUACTION_GETKEY,	"FEC_LDN", { nil, SAVESLOT_NONE, MENUPAGE_CONTROLLER_PC_OLD3 },
		MENUACTION_GETKEY,	"FEC_SMS", { nil, SAVESLOT_NONE, MENUPAGE_CONTROLLER_PC_OLD3 },
		MENUACTION_SHOWHEADBOB,	"FEC_GSL", { nil, SAVESLOT_NONE, MENUPAGE_CONTROLLER_PC_OLD3 },
		MENUACTION_CHANGEMENU,	"FEDS_TB", { nil, SAVESLOT_NONE, MENUPAGE_NONE },
   },

   // MENUPAGE_CONTROLLER_PC_OLD4 = 39
   { "FET_CTL", MENUPAGE_CONTROLLER_PC, MENUPAGE_CONTROLLER_PC, nil, nil,

   },

   // MENUPAGE_CONTROLLER_DEBUG = 40
   { "FEC_DBG", MENUPAGE_CONTROLLER_PC, MENUPAGE_CONTROLLER_PC, nil, nil,
		MENUACTION_GETKEY,	"FEC_TGD",	{ nil, SAVESLOT_NONE, MENUPAGE_CONTROLLER_DEBUG },
		MENUACTION_GETKEY,	"FEC_TDO",	{ nil, SAVESLOT_NONE, MENUPAGE_CONTROLLER_DEBUG },
		MENUACTION_GETKEY,	"FEC_TSS",	{ nil, SAVESLOT_NONE, MENUPAGE_CONTROLLER_DEBUG },
		MENUACTION_GETKEY,	"FEC_SMS",	{ nil, SAVESLOT_NONE, MENUPAGE_CONTROLLER_DEBUG },
		MENUACTION_CHANGEMENU,	"FEDS_TB",	{ nil, SAVESLOT_NONE, MENUPAGE_NONE },
   },

   // MENUPAGE_OPTIONS = 41
   { "FET_OPT", MENUPAGE_NONE, MENUPAGE_NONE, nil, nil,
		MENUACTION_CHANGEMENU,		"FET_CTL", { nil, SAVESLOT_NONE, MENUPAGE_CONTROLLER_PC },
		MENUACTION_LOADRADIO,		"FET_AUD", { nil, SAVESLOT_NONE, MENUPAGE_SOUND_SETTINGS },
		MENUACTION_CHANGEMENU,		"FET_DIS", { nil, SAVESLOT_NONE, MENUPAGE_DISPLAY_SETTINGS },
#ifdef GRAPHICS_MENU_OPTIONS
		MENUACTION_CHANGEMENU,		"FET_GFX", { nil, SAVESLOT_NONE, MENUPAGE_GRAPHICS_SETTINGS },
#endif
		MENUACTION_CHANGEMENU,		"FET_LAN", { nil, SAVESLOT_NONE, MENUPAGE_LANGUAGE_SETTINGS },
		MENUACTION_PLAYERSETUP,		"FET_PSU", { nil, SAVESLOT_NONE, MENUPAGE_SKIN_SELECT },
		MENUACTION_CHANGEMENU,		"FEDS_TB", { nil, SAVESLOT_NONE, MENUPAGE_NONE },
   },

   // MENUPAGE_EXIT = 42
   { "FET_QG", MENUPAGE_NONE, MENUPAGE_NONE, nil, nil,
	   MENUACTION_LABEL,		"FEQ_SRE",	{ nil, SAVESLOT_NONE, MENUPAGE_NONE },
	   MENUACTION_DONTCANCEL,	"FEM_NO",	{ nil, SAVESLOT_NONE, MENUPAGE_NONE },
	   MENUACTION_CANCELGAME,	"FEM_YES",	{ nil, SAVESLOT_NONE, MENUPAGE_NONE },
   },

   // MENUPAGE_SAVING_IN_PROGRESS = 43
   { "", MENUPAGE_CHOOSE_SAVE_SLOT, MENUPAGE_CHOOSE_SAVE_SLOT, nil, nil,
	   MENUACTION_LABEL,	"FES_WAR",	{ nil, SAVESLOT_NONE, MENUPAGE_NONE },
   },

   // MENUPAGE_SAVE_SUCCESSFUL = 44
   { "FET_SG", MENUPAGE_CHOOSE_SAVE_SLOT, MENUPAGE_CHOOSE_SAVE_SLOT, nil, nil,
	   MENUACTION_LABEL,				"FES_SSC",	{ nil, SAVESLOT_LABEL,	MENUPAGE_NONE },
	   MENUACTION_RESUME_FROM_SAVEZONE,	"FEC_OKK",	{ nil, SAVESLOT_NONE,	MENUPAGE_CHOOSE_SAVE_SLOT },
   },

   // MENUPAGE_DELETING = 45
   { "FET_DG", MENUPAGE_CHOOSE_DELETE_SLOT, MENUPAGE_CHOOSE_DELETE_SLOT, nil, nil,
	   MENUACTION_LABEL,	"FED_DLW",	{ nil, SAVESLOT_NONE, MENUPAGE_NONE },
   },

   // MENUPAGE_DELETE_SUCCESS = 46
   { "FET_DG", MENUPAGE_CHOOSE_DELETE_SLOT, MENUPAGE_CHOOSE_DELETE_SLOT, nil, nil,
		MENUACTION_LABEL,		"DEL_FNM", { nil, SAVESLOT_NONE,	MENUPAGE_NONE },
		MENUACTION_CHANGEMENU,	"FEC_OKK", { nil, SAVESLOT_NONE,	MENUPAGE_CHOOSE_DELETE_SLOT },
   },

   // MENUPAGE_SAVE_FAILED = 47
   { "FET_SG", MENUPAGE_CHOOSE_SAVE_SLOT, MENUPAGE_CHOOSE_SAVE_SLOT, nil, nil,
		MENUACTION_LABEL,		"FEC_SVU",	{ nil, SAVESLOT_NONE,	MENUPAGE_NONE },
		MENUACTION_CHANGEMENU,	"FEC_OKK",	{ nil, SAVESLOT_NONE,	MENUPAGE_CHOOSE_SAVE_SLOT },
   },

   // MENUPAGE_LOAD_FAILED = 48
   { "FET_SG", MENUPAGE_CHOOSE_SAVE_SLOT, MENUPAGE_CHOOSE_SAVE_SLOT, nil, nil,
		MENUACTION_LABEL,	"FEC_SVU",	{ nil, SAVESLOT_NONE,	MENUPAGE_NONE },
   },

   // MENUPAGE_LOAD_FAILED_2 = 49
   { "FET_LG", MENUPAGE_CHOOSE_SAVE_SLOT, MENUPAGE_CHOOSE_SAVE_SLOT, nil, nil,
		MENUACTION_LABEL,		"FEC_LUN",	{ nil, SAVESLOT_NONE,  MENUPAGE_NONE },
		MENUACTION_CHANGEMENU,	"FEDS_TB",	{ nil, SAVESLOT_NONE,  MENUPAGE_CHOOSE_LOAD_SLOT },
   },

   // MENUPAGE_FILTER_GAME = 50
   { "FIL_FLT", MENUPAGE_DISABLED, MENUPAGE_DISABLED, nil, nil,

   },

   // MENUPAGE_START_MENU = 51
   { "FEM_MM", MENUPAGE_DISABLED, MENUPAGE_DISABLED, nil, nil,
		 MENUACTION_CHANGEMENU,	"FEN_STA",	{ nil, SAVESLOT_NONE,	MENUPAGE_NEW_GAME },
		 MENUACTION_CHANGEMENU,	"FET_OPT",	{ nil, SAVESLOT_NONE,	MENUPAGE_OPTIONS },
		 MENUACTION_CHANGEMENU,	"FEM_QT",	{ nil, SAVESLOT_NONE,	MENUPAGE_EXIT },
   },

   // MENUPAGE_PAUSE_MENU = 52
   { "FET_PAU", MENUPAGE_DISABLED, MENUPAGE_DISABLED, nil, nil,
	   MENUACTION_RESUME,		"FEM_RES",	{ nil, SAVESLOT_NONE, MENUPAGE_NONE },
	   MENUACTION_CHANGEMENU,	"FEN_STA",	{ nil, SAVESLOT_NONE, MENUPAGE_NEW_GAME },
	   // CMenuManager::LoadAllTextures will add map here, if MENU_MAP enabled and map textures are found
	   MENUACTION_CHANGEMENU,	"FEP_STA",	{ nil, SAVESLOT_NONE, MENUPAGE_STATS },
	   MENUACTION_CHANGEMENU,	"FEP_BRI",	{ nil, SAVESLOT_NONE, MENUPAGE_BRIEFS },
	   MENUACTION_CHANGEMENU,	"FET_OPT",	{ nil, SAVESLOT_NONE, MENUPAGE_OPTIONS },
	   MENUACTION_CHANGEMENU,	"FEM_QT",	{ nil, SAVESLOT_NONE, MENUPAGE_EXIT },
   },

   // MENUPAGE_CHOOSE_MODE = 53
   { "FEN_STA", MENUPAGE_NONE, MENUPAGE_NONE, nil, nil,
	   MENUACTION_CHANGEMENU,	"FET_SP",	{ nil, SAVESLOT_NONE, MENUPAGE_NEW_GAME },
	   MENUACTION_INITMP,		"FET_MP",	{ nil, SAVESLOT_NONE, MENUPAGE_MULTIPLAYER_MAIN },
	   MENUACTION_CHANGEMENU,	"FEDS_TB",	{ nil, SAVESLOT_NONE, MENUPAGE_NONE },
   },

   // MENUPAGE_SKIN_SELECT = 54
   { "FET_PSU", MENUPAGE_OPTIONS, MENUPAGE_OPTIONS, nil, nil,
		MENUACTION_CHANGEMENU,	"FEDS_TB",	{ nil, SAVESLOT_NONE, MENUPAGE_MULTIPLAYER_MAIN },
   },

   // MENUPAGE_KEYBOARD_CONTROLS = 55
   { "FET_STI", MENUPAGE_CONTROLLER_PC, MENUPAGE_CONTROLLER_PC, nil, nil,
		MENUACTION_CHANGEMENU,	"FEDS_TB",	{ nil, SAVESLOT_NONE, MENUPAGE_CONTROLLER_PC },
   },

   // MENUPAGE_MOUSE_CONTROLS = 56
   { "FET_MTI", MENUPAGE_CONTROLLER_PC, MENUPAGE_CONTROLLER_PC, nil, nil,
	   MENUACTION_MOUSESENS,	"FEC_MSH",	{ nil, SAVESLOT_NONE, MENUPAGE_MOUSE_CONTROLS },
	   MENUACTION_INVVERT,		"FEC_IVV",	{ nil, SAVESLOT_NONE, MENUPAGE_MOUSE_CONTROLS },
#ifndef GAMEPAD_MENU
       INVERT_PAD_SELECTOR
#endif
	   MENUACTION_MOUSESTEER,	"FET_MST",	{ nil, SAVESLOT_NONE, MENUPAGE_MOUSE_CONTROLS },
	   MENUACTION_CHANGEMENU,	"FEDS_TB",	{ nil, SAVESLOT_NONE, MENUPAGE_NONE },
   },
	// MENUPAGE_MISSION_RETRY = 57
#ifdef MISSION_REPLAY

   { "M_FAIL", MENUPAGE_DISABLED, MENUPAGE_DISABLED, nil, nil,
	   MENUACTION_LABEL,	    "FESZ_RM",  { nil, SAVESLOT_NONE, MENUPAGE_NONE },
	   MENUACTION_CHANGEMENU,   "FEM_YES",  { nil, SAVESLOT_NONE, MENUPAGE_LOADING_IN_PROGRESS },
	   MENUACTION_REJECT_RETRY, "FEM_NO",   { nil, SAVESLOT_NONE, MENUPAGE_NONE },
   },
#else
   { "", MENUPAGE_NONE, MENUPAGE_NONE, nil, nil,
	   // mission failed, wanna restart page in mobile
   },
#endif

#ifdef MENU_MAP
	// MENUPAGE_MAP
   { "FEG_MAP", MENUPAGE_NONE, MENUPAGE_NONE, nil, nil,
		MENUACTION_UNK110,	"", { nil, SAVESLOT_NONE, MENUPAGE_NONE }, // to prevent cross/enter to go back
		MENUACTION_CHANGEMENU,	"FEDS_TB", { nil, SAVESLOT_NONE, MENUPAGE_NONE },
   },
#endif

#ifdef GRAPHICS_MENU_OPTIONS
	// MENUPAGE_GRAPHICS_SETTINGS
	{ "FET_GFX", MENUPAGE_OPTIONS, MENUPAGE_OPTIONS,
		new CCustomScreenLayout({MENUSPRITE_MAINMENU, 50, 0, 20, FONT_HEADING, FESCREEN_LEFT_ALIGN, true, MEDIUMTEXT_X_SCALE, MEDIUMTEXT_Y_SCALE}), GraphicsGoBack,

		MENUACTION_SCREENRES,	"FED_RES", { nil, SAVESLOT_NONE, MENUPAGE_GRAPHICS_SETTINGS },
		MENUACTION_WIDESCREEN,	"FED_WIS", { nil, SAVESLOT_NONE, MENUPAGE_GRAPHICS_SETTINGS },
		VIDEOMODE_SELECTOR
		MENUACTION_FRAMESYNC,	"FEM_VSC", { nil, SAVESLOT_NONE, MENUPAGE_DISPLAY_SETTINGS },
		MENUACTION_FRAMELIMIT,	"FEM_FRM", { nil, SAVESLOT_NONE, MENUPAGE_DISPLAY_SETTINGS },
		MULTISAMPLING_SELECTOR
		ISLAND_LOADING_SELECTOR
		DUALPASS_SELECTOR
#ifdef EXTENDED_COLOURFILTER
		POSTFX_SELECTORS
#else
		MENUACTION_TRAILS,		"FED_TRA", { nil, SAVESLOT_NONE, MENUPAGE_DISPLAY_SETTINGS },
#endif
		// re3.cpp inserts here pipeline selectors if neo/neo.txd exists and EXTENDED_PIPELINES defined
		MENUACTION_CFO_DYNAMIC,	"FET_DEF", { new CCFODynamic(nil, nil, nil, nil, RestoreDefGraphics) },
		MENUACTION_CHANGEMENU,	"FEDS_TB", { nil, SAVESLOT_NONE, MENUPAGE_NONE },
	},
#endif

#ifdef DETECT_JOYSTICK_MENU
	// MENUPAGE_DETECT_JOYSTICK
	{ "FEC_JOD", MENUPAGE_CONTROLLER_PC, MENUPAGE_CONTROLLER_PC,
		new CCustomScreenLayout({MENUSPRITE_MAINMENU, 40, 60, 20, FONT_BANK, FESCREEN_LEFT_ALIGN, false, MEDIUMTEXT_X_SCALE, MEDIUMTEXT_Y_SCALE}), DetectJoystickGoBack,

		MENUACTION_LABEL,	"FEC_JPR", { nil, SAVESLOT_NONE, MENUPAGE_NONE },
		MENUACTION_CFO_DYNAMIC,	"FEC_JDE", { new CCFODynamic(nil, nil, nil, DetectJoystickDraw, nil) },
		MENUACTION_CHANGEMENU,	"FEDS_TB", { nil, SAVESLOT_NONE, MENUPAGE_NONE },
	},
#endif

   // MENUPAGE_UNK
   { "", MENUPAGE_NONE, MENUPAGE_NONE, nil, nil,

   },

};

#endif
#endif
