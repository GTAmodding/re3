#pragma once

enum Config {
	NUMPLAYERS = 1,

	NUMCDIMAGES = 6, // gta3.img duplicates (not used on PC)
	MAX_CDIMAGES = 8, // additional cdimages
	MAX_CDCHANNELS = 5,

	MODELINFOSIZE = 6500,
	TXDSTORESIZE = 1385,
	COLSTORESIZE = 31,
	EXTRADIRSIZE = 256,
	CUTSCENEDIRSIZE = 512,

	SIMPLEMODELSIZE = 3885,
	TIMEMODELSIZE = 385,
	CLUMPMODELSIZE = 5,
	WEAPONMODELSIZE = 37,
	PEDMODELSIZE = 130,
	VEHICLEMODELSIZE = 110,
	TWODFXSIZE = 1210,

	MAXVEHICLESLOADED = 50, // 70 on mobile

	NUMOBJECTINFO = 400, // TODO(MIAMI): fantasy	// object.dat

	// Pool sizes
	NUMPTRNODES = 50000,
	NUMENTRYINFOS = 5400, // only 3200 in VC???
	NUMPEDS = 140,
	NUMVEHICLES = 110,
	NUMBUILDINGS = 7000,
	NUMTREADABLES = 1214,	// 1 in VC
	NUMOBJECTS = 460,
	NUMDUMMIES = 2802, // 2340 in VC
	NUMAUDIOSCRIPTOBJECTS = 256,	// 192 in VC
	NUMCUTSCENEOBJECTS = 50,	// does not exist in VC
	// TODO(MIAMI): colmodel pool

	NUMANIMBLOCKS = 35,
	NUMANIMATIONS = 450,

	NUMTEMPOBJECTS = 30,

	// Path data
	NUM_PATHNODES = 9650,
	NUM_CARPATHLINKS = 3500,
	NUM_MAPOBJECTS = 1250,
	NUM_PATHCONNECTIONS = 20400,

	// Link list lengths
	NUMALPHALIST = 20,
	NUMBOATALPHALIST = 20,
	NUMALPHAENTITYLIST = 200,
	NUMALPHAUNTERWATERENTITYLIST = 30,
	NUMCOLCACHELINKS = 200,
	NUMREFERENCES = 800,

	// Zones
	NUMAUDIOZONES = 14,
	NUMINFOZONES = 169,
	NUMMAPZONES = 39,
	NUMNAVIGZONES = 20,

	// Cull zones
	NUMATTRIBZONES = 704,

	NUMOCCLUSIONVOLUMES = 350,

	PATHNODESIZE = 4500,

	NUMWEATHERS = 7,
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

	NUM_CARGENS = 185,

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
	NUMPEDGROUPS = 67,
	NUMMODELSPERPEDGROUP = 16,
	NUMSHOTINFOS = 100,

	NUMROADBLOCKS = 300,

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

	NUM_GARAGE_STORED_CARS = 4,

	NUM_CRANES = 8,

	NUM_EXPLOSIONS = 48,

	NUM_SETPIECES = 96
};

// We'll use this once we're ready to become independent of the game
// Use it to mark bugs in the code that will prevent the game from working then
//#define STANDALONE

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

#if defined GTA_PS2
#	define GTA_PS2_STUFF
#	define RANDOMSPLASH
#elif defined GTA_PC
#	define GTA3_1_1_PATCH
//#	define GTA3_STEAM_PATCH
#	ifdef GTA_PS2_STUFF
#		define USE_PS2_RAND
#		define RANDOMSPLASH	// use random splash as on PS2
#		define PS2_MATFX
#	endif
#elif defined GTA_XBOX
#endif

#ifdef MASTER
	// only in master builds
#else
	// not in master builds
	#define VALIDATE_SAVE_SIZE
#endif

#ifdef FINAL
	// in all games
#	define USE_MY_DOCUMENTS	// use my documents directory for user files
#else
	// not in any game
#	define NASTY_GAME	// nasty game for all languages
#	define NO_MOVIES	// disable intro videos
#	define NO_CDCHECK
#	define CHATTYSPLASH	// print what the game is loading
#	define DEBUGMENU
//#	define TIMEBARS		// print debug timers
#endif

#define FIX_BUGS		// fixes bugs that we've came across during reversing, TODO: use this more
#define TOGGLEABLE_BETA_FEATURES // toggleable from debug menu. not too many things
//#define MORE_LANGUAGES		// Add more translations to the game
#define DEFAULT_NATIVE_RESOLUTION	// Set default video mode to your native resolution (fixes Windows 10 launch)
#define USE_TXD_CDIMAGE		// generate and load textures from txd.img
#define IMPROVED_VIDEOMODE	// save and load videomode parameters instead of a magic number
//#define USE_TEXTURE_POOL
#ifdef DEBUGMENU
#define RELOADABLES			// some debug menu options to reload TXD files
#endif

// Particle
#define PC_PARTICLE
//#define PS2_ALTERNATIVE_CARSPLASH // unused on PS2

// Pad
#if !defined(RW_GL3) && defined(_WIN32)
#define XINPUT
#endif
#define DETECT_PAD_INPUT_SWITCH // Adds automatic switch of pad related stuff between controller and kb/m
#define KANGAROO_CHEAT
#define ALLCARSHELI_CHEAT
#define ALT_DODO_CHEAT
#define REGISTER_START_BUTTON

// Hud, frontend and radar
#define ASPECT_RATIO_SCALE	// Not just makes everything scale with aspect ratio, also adds support for all aspect ratios
#define TRIANGULAR_BLIPS	// height indicating triangular radar blips, as in VC
// #define PS2_LIKE_MENU	// An effort to recreate PS2 menu, cycling through tabs, different bg etc.
#define MENU_MAP			// VC-like menu map. Make sure you have new menu.txd
#define SCROLLABLE_STATS_PAGE	// only draggable by mouse atm
#define TRIANGLE_BACK_BUTTON
// #define CIRCLE_BACK_BUTTON
#define HUD_ENHANCEMENTS	// Adjusts some aspects to make the HUD look/behave a little bit better.
#define BETA_SLIDING_TEXT

// Script
#define USE_DEBUG_SCRIPT_LOADER	// makes game load main_freeroam.scm by default
#define USE_MEASUREMENTS_IN_METERS // makes game use meters instead of feet in script
#define USE_PRECISE_MEASUREMENT_CONVERTION // makes game convert feet to meeters more precisely

#define COMPATIBLE_SAVES // this allows changing structs while keeping saves compatible

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
#define VC_PED_PORTS			// various ports from VC's CPed, mostly subtle
// #define NEW_WALK_AROUND_ALGORITHM	// to make walking around vehicles/objects less awkward
#define CANCELLABLE_CAR_ENTER

// Camera
#define IMPROVED_CAMERA		// Better Debug cam, and maybe more in the future
#define FREE_CAM		// Rotating cam
