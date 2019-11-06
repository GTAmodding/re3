#pragma once

enum Config {
	NUMCDIMAGES = 12,	// gta3.img duplicates (not used on PC)
	MAX_CDIMAGES = 8,	// additional cdimages
	MAX_CDCHANNELS = 5,

	MODELINFOSIZE = 5500,
	TXDSTORESIZE = 850,
	EXTRADIRSIZE = 128,
	CUTSCENEDIRSIZE = 512,

	SIMPLEMODELSIZE = 5000,
	TIMEMODELSIZE = 30,
	CLUMPMODELSIZE = 5,
	PEDMODELSIZE = 90,
	VEHICLEMODELSIZE = 120,
	TWODFXSIZE = 2000,

	MAXVEHICLESLOADED = 50,	// 70 on mobile

	NUMOBJECTINFO = 168, // object.dat

	// Pool sizes
	NUMPTRNODES = 30000,  // 26000 on PS2
	NUMENTRYINFOS = 5400, // 3200 on PS2
	NUMPEDS = 140,		  // 90 on PS2
	NUMVEHICLES = 110,	// 70 on PS2
	NUMBUILDINGS = 5500,  // 4915 on PS2
	NUMTREADABLES = 1214,
	NUMOBJECTS = 450,
	NUMDUMMIES = 2802, // 2368 on PS2
	NUMAUDIOSCRIPTOBJECTS = 256,
	NUMCUTSCENEOBJECTS = 50,

	NUMTEMPOBJECTS = 30,

	// Path data
	NUM_PATHNODES = 4930,
	NUM_CARPATHLINKS = 2076,
	NUM_MAPOBJECTS = 1250,
	NUM_PATHCONNECTIONS = 10260,

	// Link list lengths
	// TODO: alpha list
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

	NUMHANDLINGS = 57,

	PATHNODESIZE = 4500,

	NUMWEATHERS = 4,
	NUMHOURS = 24,

	NUMEXTRADIRECTIONALS = 4,
	NUMANTENNAS = 8,
	NUMCORONAS = 56,
	NUMPOINTLIGHTS = 32,
	NUM3DMARKERS = 32,
	NUMMONEYMESSAGES = 16,
	NUMPICKUPMESSAGES = 16,

	NUMONSCREENTIMERENTRIES = 1,
	NUMRADARBLIPS = 32,
	NUMGENERALPICKUPS = 320,
	NUMSCRIPTEDPICKUPS = 16,
	NUMPICKUPS = NUMGENERALPICKUPS + NUMSCRIPTEDPICKUPS,
	NUMCOLLECTEDPICKUPS = 20,
	NUMEVENTS = 64,

	NUM_CARGENS = 160,

	NUM_PATH_NODES_IN_AUTOPILOT = 8,

	NUM_ACCIDENTS = 20,
	NUM_FIRES = 40,

	NUMPEDROUTES = 200,

	NUMVISIBLEENTITIES = 2000,
	NUMINVISIBLEENTITIES = 150,
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
#	define RANDOMSPLASH
#elif defined GTA_PC
#	define GTA3_1_1_PATCH
//#	define GTA3_STEAM_PATCH
#	ifdef GTA_PS2_STUFF
//#		define USE_PS2_RAND	// this is unsafe until we have the game reversed
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
#   define NO_CDCHECK
#	define CHATTYSPLASH	// print what the game is loading
#endif

#define FIX_BUGS		// fixes bugs that we've came across during reversing, TODO: use this more

// Pad
#define KANGAROO_CHEAT

// Hud & radar
#define ASPECT_RATIO_SCALE
#define TRIANGULAR_BLIPS	// height indicating triangular radar blips, as in VC

// Script
#define USE_DEBUG_SCRIPT_LOADER	// makes game load main_freeroam.scm by default

// Vehicles
#define EXPLODING_AIRTRAIN	// can blow up jumbo jet with rocket launcher
//#define REMOVE_TREADABLE_PATHFIND

// Pickups
//#define MONEY_MESSAGES

// Peds
#define ANIMATE_PED_COL_MODEL
#define VC_PED_PORTS			// various ports from VC's CPed, mostly subtle
#define NEW_WALK_AROUND_ALGORITHM	// to make walking around vehicles/objects less awkward
#define CANCELLABLE_CAR_ENTER
