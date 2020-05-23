#pragma once

enum eLevelName {
	LEVEL_IGNORE = -1, // beware, this is only used in CPhysical's m_nZoneLevel
	LEVEL_NONE = 0,
	LEVEL_BEACH,
	LEVEL_MAINLAND
};

enum eAreaName {
	AREA_MAIN_MAP,
	AREA_HOTEL,
	AREA_MANSION,
	AREA_BANK,
	AREA_MALL,
	AREA_STRIP_CLUB,
	AREA_LAWYERS,
	AREA_COFFEE_SHOP,
	AREA_CONCERT_HALL,
	AREA_STUDIO,
	AREA_RIFLE_RANGE,
	AREA_BIKER_BAR,
	AREA_POLICE_STATION,
	AREA_EVERYWHERE,
	AREA_DIRT,
	AREA_BLOOD,
	AREA_OVALRING,
	AREA_MALIBU_CLUB,
	AREA_PRINT_WORKS
};

class CGame
{
public:
	static eLevelName currLevel;
	static int32 currArea;
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
	static bool Initialise(const char *datFile);
	static bool ShutDown(void);
	static void ReInitGameObjectVariables(void);
	static void ReloadIPLs(void);
	static void ShutDownForRestart(void);
	static void InitialiseWhenRestarting(void);
	static void Process(void);

	static void InitAfterFocusLoss(void);

	static bool IsInInterior(void) { return currArea != AREA_MAIN_MAP; }
	static bool CanSeeOutSideFromCurrArea(void);
	
	// NB: these do something on PS2
	static void TidyUpMemory(bool, bool);
	static void DrasticTidyUpMemory(bool);
	static void ProcessTidyUpMemory(void);
};

inline bool IsAreaVisible(int area) { return area == CGame::currArea || area == AREA_EVERYWHERE; }
