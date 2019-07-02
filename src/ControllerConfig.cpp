	#define DIRECTINPUT_VERSION 0x0800
	#include <dinput.h>
#include "common.h"
#include "patcher.h"
#include "ControllerConfig.h"
#include "Pad.h"
#include "FileMgr.h"

CControllerConfigManager &ControlsManager = *(CControllerConfigManager*)0x8F43A4;

WRAPPER void CControllerConfigManager::UpdateJoyButtonState(int padnumber) { EAXJMP(0x58F5B0); }
WRAPPER void CControllerConfigManager::UpdateJoyInConfigMenus_ButtonDown(int button, int padnumber) { EAXJMP(0x58C5E0); }
WRAPPER void CControllerConfigManager::AffectControllerStateOn_ButtonDown(int button, eControllerType type) { EAXJMP(0x58C730); }
WRAPPER void CControllerConfigManager::UpdateJoyInConfigMenus_ButtonUp(int button, int padnumber) { EAXJMP(0x58CE80); }
WRAPPER void CControllerConfigManager::AffectControllerStateOn_ButtonUp(int button, int padnumber) { EAXJMP(0x58CFD0); }
WRAPPER void CControllerConfigManager::MakeControllerActionsBlank() { EAXJMP(0x58B7A0); }
WRAPPER void CControllerConfigManager::InitDefaultControlConfiguration() { EAXJMP(0x58B930); }
WRAPPER void CControllerConfigManager::InitDefaultControlConfigMouse(CMouseControllerState const &mousestate) { EAXJMP(0x58BD00); }
WRAPPER int32 CControllerConfigManager::GetJoyButtonJustDown() { EAXJMP(0x58B7D0); }
WRAPPER void CControllerConfigManager::InitDefaultControlConfigJoyPad(unsigned int buttons) { EAXJMP(0x58BD90); }
WRAPPER void CControllerConfigManager::ClearSimButtonPressCheckers() { EAXJMP(0x58D220); }
WRAPPER void CControllerConfigManager::AffectPadFromKeyBoard() { EAXJMP(0x58D0C0); }
WRAPPER void CControllerConfigManager::AffectPadFromMouse() { EAXJMP(0x58D1A0); }

void CControllerConfigManager::LoadSettings(int32 file)
{
	bool bValid = true;
	
	if ( file )
	{
		char buff[29];
		CFileMgr::Read(file, buff, sizeof(buff));
		
		if ( !strncmp(buff, "THIS FILE IS NOT VALID YET", sizeof(buff) - 3) )
			bValid = false;
		else
			CFileMgr::Seek(file, 0, 0);
	}
	
	if ( bValid )
	{
		ControlsManager.MakeControllerActionsBlank();
		
		for ( int i = 0; i < 4; i++ )
		{
			for ( int j = 0; j < 41; j++ )
			{
				CFileMgr::Read(file, (char *)&ControlsManager.m_aSettings[j][i], sizeof(tControllerConfigBind));
			}
		}
	}
}

WRAPPER void CControllerConfigManager::SaveSettings(int32 file)
{
	EAXJMP(0x58B800);
}
