#include "common.h"
#include "patcher.h"
#include "Frontend.h"
#include "Timer.h"

int &CMenuManager::OS_Language = *(int*)0x5F2F78;
char &CMenuManager::m_PrefsUseVibration = *(char*)0x95CD92;
char &CMenuManager::m_DisplayControllerOnFoot = *(char*)0x95CD8D;
char &CMenuManager::m_PrefsVsync = *(char*)0x5F2E58;
char &CMenuManager::m_PrefsVsyncDisp = *(char*)0x5F2E5C;
char &CMenuManager::m_PrefsFrameLimiter = *(char*)0x5F2E60;
char &CMenuManager::BlurOn = *(char*)0x95CDAD;
char &CMenuManager::m_PrefsShowSubtitles = *(char*)0x5F2E54;
char &CMenuManager::m_PrefsSpeakers = *(char*)0x95CD7E;
char &CMenuManager::m_ControlMethod = *(char*)0x8F5F7C;
char &CMenuManager::m_PrefsDMA = *(char*)0x5F2F74;
char &CMenuManager::m_PrefsLanguage = *(char*)0x941238;

Bool &CMenuManager::m_PrefsAllowNastyGame = *(Bool*)0x5F2E64;
Bool &CMenuManager::m_bStartUpFrontEndRequested = *(Bool*)0x95CCF4;

char &CMenuManager::m_PrefsUseWideScreen = *(char*)0x95CD23;
char &CMenuManager::m_PrefsRadioStation = *(char*)0x95CDA4;
char &CMenuManager::m_bDisableMouseSteering = *(char*)0x60252C;
int &CMenuManager::m_PrefsBrightness = *(int*)0x5F2E50;
float &CMenuManager::m_PrefsLOD = *(float*)0x8F42C4;
char &CMenuManager::m_bFrontEnd_ReloadObrTxtGxt = *(char*)0x628CFC;
int &CMenuManager::m_PrefsMusicVolume = *(int*)0x5F2E4C;
int &CMenuManager::m_PrefsSfxVolume = *(int*)0x5F2E48;

CMenuManager &FrontEndMenuManager = *(CMenuManager*)0x8F59D8;

WRAPPER void CMenuManager::Process(void) { EAXJMP(0x485100); }
WRAPPER void CMenuManager::DrawFrontEnd(void) { EAXJMP(0x47A540); }
WRAPPER void CMenuManager::UnloadTextures(void) { EAXJMP(0x47A440); }
WRAPPER void CMenuManager::LoadAllTextures(void) { EAXJMP(0x47A230); }
WRAPPER void CMenuManager::LoadSettings(void) { EAXJMP(0x488EE0); }
WRAPPER void CMenuManager::WaitForUserCD(void) { EAXJMP(0x48ADD0); }

int CMenuManager::FadeIn(int alpha) {
	FrontEndMenuManager.m_PrefsVsync = 1;
	if (FrontEndMenuManager.m_nCurrScreen == 14 || FrontEndMenuManager.m_nCurrScreen == 43 || FrontEndMenuManager.m_nCurrScreen == 45)
		return alpha;

	if (FrontEndMenuManager.m_nMenuFadeAlpha >= alpha)
		return alpha;

	return FrontEndMenuManager.m_nMenuFadeAlpha;	
}

STARTPATCHES
	InjectHook(0x48AC60, &CMenuManager::FadeIn, PATCH_JUMP);
ENDPATCHES
