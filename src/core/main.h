#pragma once

struct GlobalScene
{
	RpWorld *world;
	RwCamera *camera;
};
extern GlobalScene Scene;

extern uint8 work_buff[55000];
extern char gString[256];
extern char gString2[512];
extern wchar gUString[256];
extern wchar gUString2[256];
extern bool gbPrintShite;
extern bool gbModelViewer;
#ifdef TIMEBARS
extern bool gbShowTimebars;
#else
#define gbShowTimebars false
#endif

class CSprite2d;

bool DoRWStuffStartOfFrame(int16 TopRed, int16 TopGreen, int16 TopBlue, int16 BottomRed, int16 BottomGreen, int16 BottomBlue, int16 Alpha);
bool DoRWStuffStartOfFrame_Horizon(int16 TopRed, int16 TopGreen, int16 TopBlue, int16 BottomRed, int16 BottomGreen, int16 BottomBlue, int16 Alpha);
void DoRWStuffEndOfFrame(void);
void InitialiseGame(void);
void LoadingScreen(const char *str1, const char *str2, const char *splashscreen);
void LoadingIslandScreen(const char *levelName);
CSprite2d *LoadSplash(const char *name);
void DestroySplashScreen(void);
Const char *GetLevelSplashScreen(int level);
Const char *GetRandomSplashScreen(void);
void LittleTest(void);
void ValidateVersion();
void ResetLoadingScreenBar(void);
#ifndef MASTER
void TheModelViewer(void);
#endif

#ifdef LOAD_INI_SETTINGS
void LoadINISettings();
void SaveINISettings();
#endif
