#pragma once

enum eLevelName {
	LEVEL_IGNORE = -1, // beware, this is only used in CPhysical's m_nZoneLevel
	LEVEL_GENERIC = 0,
	LEVEL_INDUSTRIAL,
	LEVEL_COMMERCIAL,
	LEVEL_SUBURBAN,
	NUM_LEVELS
};

class CGame
{
public:
	static eLevelName currLevel;
	static bool bDemoMode;
	static bool nastyGame;
	static bool frenchGame;
	static bool germanGame;
#ifdef MORE_LANGUAGES
	static bool russianGame;
	static bool japaneseGame;
#endif
	static bool noProstitutes;
	static bool playingIntro;
	static char aDatFile[32];

	static bool InitialiseOnceBeforeRW(void);
	static bool InitialiseRenderWare(void);
	static void ShutdownRenderWare(void);
	static bool InitialiseOnceAfterRW(void);
	static void FinalShutdown(void);
#if GTA_VERSION <= GTA3_PS2_160
	static bool Initialise(void);
#else
	static bool Initialise(const char *datFile);
#endif
	static bool ShutDown(void);
	static void ReInitGameObjectVariables(void);
	static void ReloadIPLs(void);
	static void ShutDownForRestart(void);
	static void InitialiseWhenRestarting(void);
	static void Process(void);
	
	// NB: these do something on PS2
	static void TidyUpMemory(bool, bool);
	static void DrasticTidyUpMemory(bool);
	static void ProcessTidyUpMemory(void);
};
