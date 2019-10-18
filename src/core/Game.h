#pragma once

enum eLevelName
{
	LEVEL_NONE = 0,
	LEVEL_INDUSTRIAL,
	LEVEL_COMMERCIAL,
	LEVEL_SUBURBAN
};

class CGame
{
public:
	static eLevelName &currLevel;
	static bool &bDemoMode;
	static bool &nastyGame;
	static bool &frenchGame;
	static bool &germanGame;
	static bool &noProstitutes;
	static bool &playingIntro;
	static char *aDatFile;	//[32];

	static void Initialise(const char *datFile);
	static bool InitialiseOnceBeforeRW(void);
	static bool InitialiseRenderWare(void);
	static bool InitialiseOnceAfterRW(void);
	static void InitialiseWhenRestarting(void);
	static void ShutDown(void);
	static void ShutdownRenderWare(void);
	static void FinalShutdown(void);
	static void ShutDownForRestart(void);
	static void Process(void);
	static void ReloadIPLs(void);

	// NB: these do something on PS2
	static void TidyUpMemory(bool, bool) {}
	static void DrasticTidyUpMemory(void) {}
};
