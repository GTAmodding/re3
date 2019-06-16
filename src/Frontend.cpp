#include "common.h"
#include "patcher.h"
#include "Frontend.h"

int32 &CMenuManager::OS_Language = *(int32*)0x5F2F78;
int8 &CMenuManager::m_PrefsUseVibration = *(int8*)0x95CD92;
int8 &CMenuManager::m_DisplayControllerOnFoot = *(int8*)0x95CD8D;
int8 &CMenuManager::m_PrefsVsync = *(int8*)0x5F2E58;
int8 &CMenuManager::m_PrefsVsyncDisp = *(int8*)0x5F2E5C;
int8 &CMenuManager::m_PrefsFrameLimiter = *(int8*)0x5F2E60;
int8 &CMenuManager::BlurOn = *(int8*)0x95CDAD;
int8 &CMenuManager::m_PrefsShowSubtitles = *(int8*)0x5F2E54;
int8 &CMenuManager::m_PrefsSpeakers = *(int8*)0x95CD7E;
int8 &CMenuManager::m_ControlMethod = *(int8*)0x8F5F7C;
int8 &CMenuManager::m_PrefsDMA = *(int8*)0x5F2F74;
int8 &CMenuManager::m_PrefsLanguage = *(int8*)0x941238;

Bool &CMenuManager::m_PrefsAllowNastyGame = *(Bool*)0x5F2E64;
Bool &CMenuManager::m_bStartUpFrontEndRequested = *(Bool*)0x95CCF4;

int8 &CMenuManager::m_PrefsUseWideScreen = *(int8*)0x95CD23;
int8 &CMenuManager::m_PrefsRadioStation = *(int8*)0x95CDA4;
int8 &CMenuManager::m_bDisableMouseSteering = *(int8*)0x60252C;
int32 &CMenuManager::m_PrefsBrightness = *(int32*)0x5F2E50;
Float &CMenuManager::m_PrefsLOD = *(Float*)0x8F42C4;
int8 &CMenuManager::m_bFrontEnd_ReloadObrTxtGxt = *(int8*)0x628CFC;
int32 &CMenuManager::m_PrefsMusicVolume = *(int32*)0x5F2E4C;
int32 &CMenuManager::m_PrefsSfxVolume = *(int32*)0x5F2E48;

CMenuManager &FrontEndMenuManager = *(CMenuManager*)0x8F59D8;

WRAPPER void CMenuManager::Process(void) { EAXJMP(0x485100); }
WRAPPER void CMenuManager::DrawFrontEnd(void) { EAXJMP(0x47A540); }
WRAPPER void CMenuManager::UnloadTextures(void) { EAXJMP(0x47A440); }
WRAPPER void CMenuManager::LoadAllTextures(void) { EAXJMP(0x47A230); }
WRAPPER void CMenuManager::LoadSettings(void) { EAXJMP(0x488EE0); }
WRAPPER void CMenuManager::WaitForUserCD(void) { EAXJMP(0x48ADD0); }

int CMenuManager::FadeIn(int alpha) {
	if (FrontEndMenuManager.m_nCurrScreen == MENU_LOADING_IN_PROGRESS ||
		FrontEndMenuManager.m_nCurrScreen == MENU_SAVING_IN_PROGRESS ||
		FrontEndMenuManager.m_nCurrScreen == MENU_DELETING)
		return alpha;

	if (FrontEndMenuManager.m_nMenuFadeAlpha >= alpha)
		return alpha;

	return FrontEndMenuManager.m_nMenuFadeAlpha;
}

STARTPATCHES
	InjectHook(0x48AC60, &CMenuManager::FadeIn, PATCH_JUMP);
ENDPATCHES