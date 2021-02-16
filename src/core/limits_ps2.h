#pragma once

enum Config {
	NUMPLAYERS = 4,

	NUMCDIMAGES = 12, // gta3.img duplicates
	MAX_CDIMAGES = 8, // additional cdimages
	MAX_CDCHANNELS = 5,

	MODELINFOSIZE = 3150,
	TXDSTORESIZE = 850,
	EXTRADIRSIZE = 128,
	CUTSCENEDIRSIZE = 512,

	SIMPLEMODELSIZE = 2910,
	MLOMODELSIZE = 1,
	MLOINSTANCESIZE = 1,
	TIMEMODELSIZE = 30,
	CLUMPMODELSIZE = 5,
	PEDMODELSIZE = 90,
	VEHICLEMODELSIZE = 70,
	XTRACOMPSMODELSIZE = 2,
	TWODFXSIZE = 1210,

	MAXVEHICLESLOADED = 50,

	NUMOBJECTINFO = 168, // object.dat

	// Pool sizes
	NUMPTRNODES = 26000,
	NUMENTRYINFOS = 3200,
	NUMPEDS = 90,
	NUMVEHICLES = 70,
	NUMBUILDINGS = 4915,
	NUMTREADABLES = 1214,
	NUMOBJECTS = 450,
	NUMDUMMIES = 2368,
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
	NUM_PARTICLES = 1000,

	NUM_STOREDSHADOWS = 48,
	NUM_POLYBUNCHES = 300,
	NUM_STATICSHADOWS = 64,
	NUM_PERMAMENTSHADOWS = 48,

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
	NUM_AUDIOENTITIES = 200,

	NUM_AUDIO_REFLECTIONS = 5,
	NUM_SCRIPT_MAX_ENTITIES = 40,

	NUM_GARAGE_STORED_CARS = 6,

	NUM_CRANES = 8,

	NUM_EXPLOSIONS = 48,
};
