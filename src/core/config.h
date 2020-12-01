#pragma once

enum Config {
	NUMPLAYERS = 1,

	NUMCDIMAGES = 12, // gta3.img duplicates (not used on PC)
	MAX_CDIMAGES = 8, // additional cdimages
	MAX_CDCHANNELS = 5,

	MODELINFOSIZE = 5500,
//	TXDSTORESIZE = 850,
	TXDSTORESIZE = 1024,	// for Xbox map
	EXTRADIRSIZE = 128,
	CUTSCENEDIRSIZE = 512,

	SIMPLEMODELSIZE = 5000,
	MLOMODELSIZE = 1,
	MLOINSTANCESIZE = 1,
	TIMEMODELSIZE = 30,
	CLUMPMODELSIZE = 5,
	PEDMODELSIZE = 90,
	VEHICLEMODELSIZE = 120,
	XTRACOMPSMODELSIZE = 2,
	TWODFXSIZE = 2000,

	MAXVEHICLESLOADED = 50, // 70 on mobile

	NUMOBJECTINFO = 168, // object.dat

	// Pool sizes
	NUMPTRNODES = 30000,  // 26000 on PS2
	NUMENTRYINFOS = 5400, // 3200 on PS2
	NUMPEDS = 140,        // 90 on PS2
	NUMVEHICLES = 110,    // 70 on PS2
	NUMBUILDINGS = 5500,  // 4915 on PS2
	NUMTREADABLES = 1214,
	NUMOBJECTS = 450,
	NUMDUMMIES = 2802, // 2368 on PS2
	NUMAUDIOSCRIPTOBJECTS = 256,
	NUMCUTSCENEOBJECTS = 50,

	NUMANIMBLOCKS = 2,
	NUMANIMATIONS = 250,

	NUMTEMPOBJECTS = 30,

	// Path data
	NUM_PATHNODES = 4930,
	NUM_CARPATHLINKS = 2076,
	NUM_MAPOBJECTS = 1250,
	NUM_PATHCONNECTIONS = 10260,

	// Link list lengths
	NUMALPHALIST = 20,
	NUMALPHAENTITYLIST = 150,
	NUMCOLCACHELINKS = 200,
	NUMREFERENCES = 800,

	// Zones
	NUMAUDIOZONES = 36,
	NUMZONES = 50,
	NUMMAPZONES = 25,

	// Cull zones
	NUMCULLZONES = 512,
	NUMATTRIBZONES = 288,
	NUMZONEINDICES = 55000,

	PATHNODESIZE = 4500,

	NUMWEATHERS = 4,
	NUMHOURS = 24,

	NUMEXTRADIRECTIONALS = 4,
	NUMANTENNAS = 8,
	NUMCORONAS = 56,
	NUMPOINTLIGHTS = 32,
	NUM3DMARKERS = 32,
	NUMBRIGHTLIGHTS = 32,
	NUMSHINYTEXTS = 32,
	NUMMONEYMESSAGES = 16,
	NUMPICKUPMESSAGES = 16,
	NUMBULLETTRACES = 16,
	NUMMBLURSTREAKS = 4,
	NUMSKIDMARKS = 32,

	NUMONSCREENTIMERENTRIES = 1,
	NUMRADARBLIPS = 32,
	NUMGENERALPICKUPS = 320,
	NUMSCRIPTEDPICKUPS = 16,
	NUMPICKUPS = NUMGENERALPICKUPS + NUMSCRIPTEDPICKUPS,
	NUMCOLLECTEDPICKUPS = 20,
	NUMPACMANPICKUPS = 256,
	NUMEVENTS = 64,

	NUM_CARGENS = 160,

	NUM_PATH_NODES_IN_AUTOPILOT = 8,

	NUM_ACCIDENTS = 20,
	NUM_FIRES = 40,
	NUM_GARAGES = 32,
	NUM_PROJECTILES = 32,

	NUM_GLASSPANES = 45,
	NUM_GLASSENTITIES = 32,
	NUM_WATERCANNONS = 3,

	NUMPEDROUTES = 200,
	NUMPHONES = 50,
	NUMPEDGROUPS = 31,
	NUMMODELSPERPEDGROUP = 8,
	NUMSHOTINFOS = 100,

	NUMROADBLOCKS = 600,

	NUMVISIBLEENTITIES = 2000,
	NUMINVISIBLEENTITIES = 150,

	NUM_AUDIOENTITY_EVENTS = 4,
	NUM_PED_COMMENTS_BANKS = 2,
	NUM_PED_COMMENTS_SLOTS = 20,

	NUM_SOUNDS_SAMPLES_BANKS = 2,
	NUM_SOUNDS_SAMPLES_SLOTS = 27,
	NUM_AUDIOENTITIES = 200,

	NUM_AUDIO_REFLECTIONS = 5,
	NUM_SCRIPT_MAX_ENTITIES = 40,

	NUM_GARAGE_STORED_CARS = 6,

	NUM_CRANES = 8,

	NUM_EXPLOSIONS = 48,
};

// We don't expect to compile for PS2 or Xbox
// but it might be interesting for documentation purposes
#define GTA_PC
//#define GTA_PS2
//#define GTA_XBOX

// This enables things from the PS2 version on PC
#define GTA_PS2_STUFF

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

// Version defines
#define GTA3_PS2_140	300
#define GTA3_PS2_160	301
#define GTA3_PC_10	310
#define GTA3_PC_11	311
#define GTA3_PC_STEAM	312
// TODO? maybe something for xbox or android?

#define GTA_VERSION	GTA3_PC_11

// quality of life fixes that should also be in FINAL
#define NASTY_GAME	// nasty game for all languages
#define NO_CDCHECK

// those infamous texts
#define DRAW_GAME_VERSION_TEXT
#define DRAW_MENU_VERSION_TEXT

// Memory allocation and compression
// #define USE_CUSTOM_ALLOCATOR		// use CMemoryHeap for allocation. use with care, not finished yet
//#define COMPRESSED_COL_VECTORS	// use compressed vectors for collision vertices
//#define ANIM_COMPRESSION	// only keep most recently used anims uncompressed

#if defined GTA_PS2
#	define GTA_PS2_STUFF
#	define RANDOMSPLASH
#	define USE_CUSTOM_ALLOCATOR
#	define VU_COLLISION
#	define ANIM_COMPRESSION
#elif defined GTA_PC
#	ifdef GTA_PS2_STUFF
#		define USE_PS2_RAND
#		define RANDOMSPLASH	// use random splash as on PS2
#		define PS2_MATFX
#	endif
#elif defined GTA_XBOX
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

	#define NO_MOVIES	// disable intro videos
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

#define FIX_BUGS		// fixes bugs that we've came across during reversing
#define MORE_LANGUAGES		// Add more translations to the game
#define COMPATIBLE_SAVES // this allows changing structs while keeping saves compatible
#define LOAD_INI_SETTINGS // as the name suggests. fundamental for CUSTOM_FRONTEND_OPTIONS

// Just debug menu entries
#ifdef DEBUGMENU
#define MISSION_SWITCHER // from debug menu
#endif

// Rendering/display
//#define EXTRA_MODEL_FLAGS	// from mobile to optimize rendering
//# define HARDCODED_MODEL_FLAGS	// sets the flags enabled above from hardcoded model names.
				// NB: keep this enabled unless your map IDEs have these flags baked in
#define ASPECT_RATIO_SCALE	// Not just makes everything scale with aspect ratio, also adds support for all aspect ratios
#define DEFAULT_NATIVE_RESOLUTION	// Set default video mode to your native resolution (fixes Windows 10 launch)
#define USE_TXD_CDIMAGE		// generate and load textures from txd.img
#define PS2_ALPHA_TEST		// emulate ps2 alpha test 
#define IMPROVED_VIDEOMODE	// save and load videomode parameters instead of a magic number
#define DISABLE_LOADING_SCREEN // disable the loading screen which vastly improves the loading time
#define DISABLE_VSYNC_ON_TEXTURE_CONVERSION // make texture conversion work faster by disabling vsync
//#define USE_TEXTURE_POOL
#ifdef LIBRW
//#define EXTENDED_COLOURFILTER		// more options for colour filter (replaces mblur)
//#define EXTENDED_PIPELINES		// custom render pipelines (includes Neo)
//#define SCREEN_DROPLETS			// neo water droplets
#endif

#ifndef EXTENDED_COLOURFILTER
#undef SCREEN_DROPLETS		// we need the backbuffer for this effect
#endif
#ifndef EXTENDED_PIPELINES
#undef SCREEN_DROPLETS		// we need neo.txd
#endif

// Particle
//#define PC_PARTICLE
//#define PS2_ALTERNATIVE_CARSPLASH // unused on PS2

// Pad
#if !defined(RW_GL3) && defined(_WIN32)
#define XINPUT
#endif
#if !defined(_WIN32) && !defined(__SWITCH__)
#define DONT_TRUST_RECOGNIZED_JOYSTICKS // Then we'll only rely on GLFW gamepad DB, and expect user to enter Controller->Detect joysticks if his joystick isn't on that list.
#endif
#define DETECT_PAD_INPUT_SWITCH // Adds automatic switch of pad related stuff between controller and kb/m
#define KANGAROO_CHEAT
#define ALLCARSHELI_CHEAT
#define ALT_DODO_CHEAT
#define REGISTER_START_BUTTON
//#define BIND_VEHICLE_FIREWEAPON // Adds ability to rebind fire key for 'in vehicle' controls
#define BUTTON_ICONS // use textures to show controller buttons

// Hud, frontend and radar
#define HUD_ENHANCEMENTS	// Adjusts some aspects to make the HUD look/behave a little bit better.
// #define BETA_SLIDING_TEXT
#define TRIANGULAR_BLIPS	// height indicating triangular radar blips, as in VC
// #define XBOX_SUBTITLES	// the infamous outlines
#define PC_MENU

#ifndef PC_MENU
#	define PS2_MENU
//#	define PS2_MENU_USEALLPAGEICONS
#else
#	define MENU_MAP			// VC-like menu map. Make sure you have new menu.txd
#	define SCROLLABLE_STATS_PAGE	// only draggable by mouse atm
#	define TRIANGLE_BACK_BUTTON
//#	define CIRCLE_BACK_BUTTON
//#	define PS2_LIKE_MENU	// An effort to recreate PS2 menu, cycling through tabs, different bg etc.
//#	define PS2_SAVE_DIALOG		// PS2 style save dialog with transparent black box
#	define CUSTOM_FRONTEND_OPTIONS

#	ifdef CUSTOM_FRONTEND_OPTIONS
#		define GRAPHICS_MENU_OPTIONS // otherwise Advanced Options menu will appear if Display is full
#		define NO_ISLAND_LOADING  // disable loadscreen between islands via loading all island data at once, consumes more memory and CPU
#		define CUTSCENE_BORDERS_SWITCH
#		define MULTISAMPLING		// adds MSAA option
#		define INVERT_LOOK_FOR_PAD // add bInvertLook4Pad from VC
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

#ifndef USE_ADVANCED_SCRIPT_DEBUG_OUTPUT
#define USE_BASIC_SCRIPT_DEBUG_OUTPUT
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
#define PED_SKIN		// support for skinned geometry on peds
#define ANIMATE_PED_COL_MODEL
// #define VC_PED_PORTS			// various ports from VC's CPed, mostly subtle
// #define NEW_WALK_AROUND_ALGORITHM	// to make walking around vehicles/objects less awkward
#define CANCELLABLE_CAR_ENTER
//#define PEDS_REPORT_CRIMES_ON_PHONE

// Camera
//#define PS2_CAM_TRANSITION	// old way of transitioning between cam modes
#define IMPROVED_CAMERA		// Better Debug cam, and maybe more in the future
#define FREE_CAM		// Rotating cam

// Audio
#ifndef AUDIO_OAL // is not working yet for openal
#define AUDIO_CACHE // cache sound lengths to speed up the cold boot
#endif
//#define PS2_AUDIO   // changes audio paths for cutscenes and radio to PS2 paths, needs vbdec to support VB with MSS

// IMG
#define BIG_IMG // allows to read larger img files

//#define SQUEEZE_PERFORMANCE
#ifdef SQUEEZE_PERFORMANCE
	#undef PS2_ALPHA_TEST
	#undef NO_ISLAND_LOADING
	#define PC_PARTICLE
	#define VC_PED_PORTS // To not process collisions always. But should be tested if that's really beneficial
#endif

#ifdef LIBRW
// these are not supported with librw yet
#endif
