#pragma once

// disables (most) stuff that wasn't in original gta3.exe
#ifdef __MWERKS__
#define VANILLA_DEFINES
#endif

// We don't expect to compile for PS2 or Xbox
// but it might be interesting for documentation purposes
#define GTA_PC
//#define GTA_PS2
//#define GTA_XBOX

#ifdef GTA_PS2 
#include "limits_ps2.h"
#elif defined VANILLA_DEFINES
#include "limits_pc.h"
#else
#include "limits_custom.h" // some nice enhanced limits
#endif

// Version defines
#define GTA3_PS2_140	300
#define GTA3_PS2_160	301
#define GTA3_PC_10	310
#define GTA3_PC_11	311
#define GTA3_PC_STEAM	312
// TODO? maybe something for xbox or android?

#define GTA_VERSION GTA3_PC_11

#if defined GTA_PS2
#	define GTA_PS2_STUFF
#	define RANDOMSPLASH
#	define USE_CUSTOM_ALLOCATOR
#	define VU_COLLISION
#	define ANIM_COMPRESSION
#	define PS2_MENU
#elif defined GTA_PC
#	define PC_PLAYER_CONTROLS	// mouse player/cam mode
#	define GTA_REPLAY
#	define GTA_SCENE_EDIT
#	define PC_MENU
#elif defined GTA_XBOX
#endif

// This is enabled for all released games.
// any debug stuff that isn't left in any game is not in FINAL
//#define FINAL

// This is enabled for all released games except mobile
// any debug stuff that is only left in mobile, is not in MASTER
//#define MASTER

// once and for all:
// pc: FINAL & MASTER
// mobile: FINAL

// MASTER builds must be FINAL
#ifdef MASTER
#define FINAL
#endif

// these are placed here to work with VANILLA_DEFINES for compatibility
#define NO_CDCHECK // skip audio CD check
#define DEFAULT_NATIVE_RESOLUTION // Set default video mode to your native resolution (fixes Windows 10 launch)

#ifdef VANILLA_DEFINES
#if !defined(_WIN32) || defined(__LP64__) || defined(_WIN64)
#error Vanilla can only be built for win-x86
#endif

#define FINAL
#define MASTER
//#define USE_MY_DOCUMENTS
#define THIS_IS_STUPID
#define PC_PARTICLE
#define DONT_FIX_REPLAY_BUGS
#define USE_TXD_CDIMAGE // generate and load textures from txd.img
//#define USE_TEXTURE_POOL // not possible because R* used custom RW33
#else
// This enables things from the PS2 version on PC
#define GTA_PS2_STUFF

// quality of life fixes that should also be in FINAL
#define NASTY_GAME	// nasty game for all languages

// those infamous texts
#define DRAW_GAME_VERSION_TEXT
#ifdef DRAW_GAME_VERSION_TEXT
	// unlike R* development builds, ours has runtime switch on debug menu & .ini, and disabled as default.
	// If you disable this then game will fetch version from peds.col, as R* did while in development.
	//#define USE_OUR_VERSIONING // enabled from buildfiles by default
#endif
//#define DRAW_MENU_VERSION_TEXT

// Memory allocation and compression
// #define USE_CUSTOM_ALLOCATOR		// use CMemoryHeap for allocation. use with care, not finished yet
//#define COMPRESSED_COL_VECTORS	// use compressed vectors for collision vertices
//#define ANIM_COMPRESSION	// only keep most recently used anims uncompressed

#if defined GTA_PC && defined GTA_PS2_STUFF
#	define USE_PS2_RAND
#	define RANDOMSPLASH	// use random splash as on PS2
#	define PS2_MATFX
#endif

#ifdef VU_COLLISION
#define COMPRESSED_COL_VECTORS	// currently need compressed vectors in this code
#endif

#ifdef MASTER
	// only in master builds
	#undef DRAW_GAME_VERSION_TEXT
#else
	// not in master builds
	#define VALIDATE_SAVE_SIZE

	#define DEBUGMENU
#endif

#ifdef FINAL
	// in all games
#	define USE_MY_DOCUMENTS	// use my documents directory for user files
#else
	// not in any game
#	define CHATTYSPLASH	// print what the game is loading
#	define TIMEBARS		// print debug timers
#endif

#define FIX_BUGS		// fixes bugs that we've came across during reversing. You can undefine this only on release builds.
#define MORE_LANGUAGES		// Add more translations to the game
#define COMPATIBLE_SAVES // this allows changing structs while keeping saves compatible, and keeps saves compatible between platforms
#define FIX_INCOMPATIBLE_SAVES // try to fix incompatible saves, requires COMPATIBLE_SAVES
#define LOAD_INI_SETTINGS // as the name suggests. fundamental for CUSTOM_FRONTEND_OPTIONS

#define NO_MOVIES	// add option to disable intro videos

#if defined(__LP64__) || defined(_WIN64)
#define FIX_BUGS_64 // Must have fixes to be able to run 64 bit build
#endif

#define ASCII_STRCMP // use faster ascii str comparisons

#if !defined _WIN32 || defined __MINGW32__ 
#undef ASCII_STRCMP
#endif

// Just debug menu entries
#ifdef DEBUGMENU
#define MISSION_SWITCHER // from debug menu
#endif

// Rendering/display
//#define EXTRA_MODEL_FLAGS	// from mobile to optimize rendering
//# define HARDCODED_MODEL_FLAGS	// sets the flags enabled above from hardcoded model names.
				// NB: keep this enabled unless your map IDEs have these flags baked in
#define ASPECT_RATIO_SCALE	// Not just makes everything scale with aspect ratio, also adds support for all aspect ratios
#define PROPER_SCALING		// use original DEFAULT_SCREEN_WIDTH/DEFAULT_SCREEN_HEIGHT from PS2 instead of PC(R* changed HEIGHT here to make radar look better, but broke other hud elements aspect ratio).
#define DEFAULT_NATIVE_RESOLUTION	// Set default video mode to your native resolution (fixes Windows 10 launch)
#define USE_TXD_CDIMAGE		// generate and load textures from txd.img
#define PS2_ALPHA_TEST		// emulate ps2 alpha test 
#define IMPROVED_VIDEOMODE	// save and load videomode parameters instead of a magic number
#define DISABLE_LOADING_SCREEN // disable the loading screen which vastly improves the loading time
#ifdef DISABLE_LOADING_SCREEN
// enable the PC splash
#undef RANDOMSPLASH
#endif
#define DISABLE_VSYNC_ON_TEXTURE_CONVERSION // make texture conversion work faster by disabling vsync
#define ANISOTROPIC_FILTERING	// set all textures to max anisotropic filtering
//#define USE_TEXTURE_POOL
#ifdef LIBRW
#define EXTENDED_COLOURFILTER		// more options for colour filter (replaces mblur)
#define EXTENDED_PIPELINES		// custom render pipelines (includes Neo)
#define SCREEN_DROPLETS			// neo water droplets
#define NEW_RENDERER		// leeds-like world rendering, needs librw
#endif

#define FIX_SPRITES	// fix sprites aspect ratio(moon, coronas, particle etc)

#ifndef EXTENDED_COLOURFILTER
#undef SCREEN_DROPLETS		// we need the backbuffer for this effect
#endif

// Particle
//#define PC_PARTICLE
//#define PS2_ALTERNATIVE_CARSPLASH // unused on PS2

// Pad
#if !defined(RW_GL3) && defined(_WIN32)
#define XINPUT
#endif
#if defined XINPUT || (defined RW_GL3 && !defined LIBRW_SDL2 && !defined __SWITCH__)
#define DETECT_JOYSTICK_MENU // Then we'll expect user to enter Controller->Detect joysticks if his joystick isn't detected at the start.
#endif
#define DETECT_PAD_INPUT_SWITCH // Adds automatic switch of pad related stuff between controller and kb/m
#define KANGAROO_CHEAT
#define ALLCARSHELI_CHEAT
#define ALT_DODO_CHEAT
#define REGISTER_START_BUTTON
#define BIND_VEHICLE_FIREWEAPON // Adds ability to rebind fire key for 'in vehicle' controls
#define BUTTON_ICONS // use textures to show controller buttons

// Hud, frontend and radar
//#define PS2_HUD
#define HUD_ENHANCEMENTS	// Adjusts some aspects to make the HUD look/behave a little bit better.
// #define BETA_SLIDING_TEXT
#define TRIANGULAR_BLIPS	// height indicating triangular radar blips, as in VC
#define FIX_RADAR			// use radar size from early version before R* broke it
// #define XBOX_SUBTITLES	// the infamous outlines
#define RADIO_OFF_TEXT
#define PC_MENU

#ifndef PC_MENU
#	define PS2_MENU
//#	define PS2_MENU_USEALLPAGEICONS
#else

#	ifdef XINPUT
#		define GAMEPAD_MENU		// Add gamepad menu
#	endif

#	define SCROLLABLE_STATS_PAGE	// only draggable by mouse atm
#	define TRIANGLE_BACK_BUTTON
//#	define CIRCLE_BACK_BUTTON
//#	define PS2_LIKE_MENU	// An effort to recreate PS2 menu, cycling through tabs, different bg etc.
//#	define PS2_SAVE_DIALOG		// PS2 style save dialog with transparent black box
#	define CUSTOM_FRONTEND_OPTIONS

#	ifdef CUSTOM_FRONTEND_OPTIONS
#		define MENU_MAP			// VC-like menu map. Won't appear if you don't have our menu.txd
#		define GRAPHICS_MENU_OPTIONS // otherwise Display settings will be scrollable
#		define NO_ISLAND_LOADING  // disable loadscreen between islands via loading all island data at once, consumes more memory and CPU
#		define CUTSCENE_BORDERS_SWITCH
#		define MULTISAMPLING		// adds MSAA option
#		define INVERT_LOOK_FOR_PAD // add bInvertLook4Pad from VC
#		define PED_CAR_DENSITY_SLIDERS
#	endif
#endif

// Script
#define USE_DEBUG_SCRIPT_LOADER	// Loads main.scm by default. Hold R for main_freeroam.scm and D for main_d.scm
#define USE_MEASUREMENTS_IN_METERS // makes game use meters instead of feet in script
#define USE_PRECISE_MEASUREMENT_CONVERTION // makes game convert feet to meeters more precisely
#ifdef PC_MENU
#	define MISSION_REPLAY // mobile feature
#endif
//#define SIMPLIER_MISSIONS // apply simplifications from mobile
#define USE_ADVANCED_SCRIPT_DEBUG_OUTPUT
#define SCRIPT_LOG_FILE_LEVEL 0 // 0 == no log, 1 == overwrite every frame, 2 == full log

#if SCRIPT_LOG_FILE_LEVEL == 0
#undef USE_ADVANCED_SCRIPT_DEBUG_OUTPUT
#endif

#ifndef USE_ADVANCED_SCRIPT_DEBUG_OUTPUT
#define USE_BASIC_SCRIPT_DEBUG_OUTPUT
#endif

#ifdef MASTER
#undef USE_ADVANCED_SCRIPT_DEBUG_OUTPUT
#undef USE_BASIC_SCRIPT_DEBUG_OUTPUT
#endif

// Replay
//#define DONT_FIX_REPLAY_BUGS // keeps various bugs in CReplay, some of which are fairly cool!
//#define USE_BETA_REPLAY_MODE // adds another replay mode, a few seconds slomo (caution: buggy!)

// Vehicles
#define EXPLODING_AIRTRAIN	// can blow up jumbo jet with rocket launcher
//#define REMOVE_TREADABLE_PATHFIND

// Pickups
//#define MONEY_MESSAGES
#define CAMERA_PICKUP

// Peds
#define PED_SKIN		// support for skinned geometry on peds, requires COMPATIBLE_SAVES
#define ANIMATE_PED_COL_MODEL
// #define VC_PED_PORTS			// various ports from VC's CPed, mostly subtle
// #define NEW_WALK_AROUND_ALGORITHM	// to make walking around vehicles/objects less awkward
#define CANCELLABLE_CAR_ENTER
//#define PEDS_REPORT_CRIMES_ON_PHONE, requires COMPATIBLE_SAVES

// Camera
//#define PS2_CAM_TRANSITION	// old way of transitioning between cam modes
#define IMPROVED_CAMERA		// Better Debug cam, and maybe more in the future
#define FREE_CAM		// Rotating cam

// Audio
#define RADIO_SCROLL_TO_PREV_STATION
#define AUDIO_CACHE
#define PS2_AUDIO_CHANNELS // increases the maximum number of audio channels to PS2 value of 44 (PC has 28 originally)
#define PS2_AUDIO_PATHS // changes audio paths for cutscenes and radio to PS2 paths (needs vbdec on MSS builds)
//#define AUDIO_OAL_USE_SNDFILE // use libsndfile to decode WAVs instead of our internal decoder
#define AUDIO_OAL_USE_MPG123 // use mpg123 to support mp3 files
#define PAUSE_RADIO_IN_FRONTEND // pause radio when game is paused
#define MULTITHREADED_AUDIO // for streams. requires C++11 or later

#ifdef AUDIO_OPUS
#define AUDIO_OAL_USE_OPUS // enable support of opus files
#define OPUS_AUDIO_PATHS // changes audio paths to opus paths (doesn't work if AUDIO_OAL_USE_OPUS isn't enabled)
#define OPUS_SFX  // enable if your sfx.raw is encoded with opus (doesn't work if AUDIO_OAL_USE_OPUS isn't enabled)

#ifndef AUDIO_OAL_USE_OPUS
#undef OPUS_AUDIO_PATHS
#undef OPUS_SFX
#endif

#endif

// Streaming
#if !defined(_WIN32) && !defined(__SWITCH__)
	//#define ONE_THREAD_PER_CHANNEL // Don't use if you're not on SSD/Flash - also not utilized too much right now(see commented LoadAllRequestedModels in Streaming.cpp)
	#define FLUSHABLE_STREAMING // Make it possible to interrupt reading when processing file isn't needed anymore.
#endif
#define BIG_IMG // Not complete - allows to read larger img files

//#define SQUEEZE_PERFORMANCE
#ifdef SQUEEZE_PERFORMANCE
	#undef PS2_ALPHA_TEST
	#undef NO_ISLAND_LOADING
	#undef PS2_AUDIO_CHANNELS
	#define PC_PARTICLE
	#define VC_PED_PORTS // To not process collisions always. But should be tested if that's really beneficial
	#define VC_RAIN_NERF // Reduces number of rain particles
#endif

// if these defines are enabled saves are not vanilla compatible without COMPATIBLE_SAVES
#ifndef COMPATIBLE_SAVES
#undef PED_SKIN
#undef PEDS_REPORT_CRIMES_ON_PHONE
#endif

#endif // VANILLA_DEFINES
