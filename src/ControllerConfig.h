#pragma once


// based on x-gtasa

enum eControllerType
{
	KEYBOARD,
	OPTIONAL_EXTRA,
	MOUSE,
	JOYSTICK,
};

class CMouseControllerState;

class CControllerConfigManager
{
public:
	struct tControllerConfigBind
	{
		RsKeyCodes m_Key;
		int32 m_ContSetOrder;
	};

	bool field_0;
	char _pad0[3];
	DIJOYSTATE2 m_OldState;
	DIJOYSTATE2 m_NewState;
	wchar m_aActionNames[41][40];
	bool m_aButtonStates[17];
	char _pad1[3];
	tControllerConfigBind m_aSettings[41][4];
	uint8 m_aSimCheckers[4][4];
	bool m_bMouseAssociated;
	char _pad2[3];
	
	void UpdateJoyButtonState(int padnumber);
	void UpdateJoyInConfigMenus_ButtonDown(int button, int padnumber);
	void AffectControllerStateOn_ButtonDown(int button, eControllerType type);
	void UpdateJoyInConfigMenus_ButtonUp(int button, int padnumber);
	void AffectControllerStateOn_ButtonUp(int button, int padnumber);
	
	int32 GetJoyButtonJustDown();
	void LoadSettings(int32 file);
	void SaveSettings(int32 file);
	void MakeControllerActionsBlank();
	void InitDefaultControlConfiguration();
	void InitDefaultControlConfigMouse(CMouseControllerState const &mousestate);
	void InitDefaultControlConfigJoyPad(unsigned int buttons);
	void ClearSimButtonPressCheckers();
	void AffectPadFromKeyBoard();
	void AffectPadFromMouse();
	
};

VALIDATE_SIZE(CControllerConfigManager, 0x143C);

extern CControllerConfigManager &ControlsManager;