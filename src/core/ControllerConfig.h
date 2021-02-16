#pragma once

#if defined RW_D3D9 || defined RWLIBS
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#endif

// based on x-gtasa

enum eControllerType
{
	KEYBOARD = 0,
	OPTIONAL_EXTRA,
	MOUSE,
	JOYSTICK,
	MAX_CONTROLLERTYPES,
};

enum e_ControllerAction
{
	PED_FIREWEAPON = 0,
	PED_CYCLE_WEAPON_RIGHT,
	PED_CYCLE_WEAPON_LEFT,
	GO_FORWARD,
	GO_BACK,
	GO_LEFT,
	GO_RIGHT,
	PED_SNIPER_ZOOM_IN,
	PED_SNIPER_ZOOM_OUT,
	VEHICLE_ENTER_EXIT,
	CAMERA_CHANGE_VIEW_ALL_SITUATIONS,
	PED_JUMPING,
	PED_SPRINT,
	PED_LOOKBEHIND,
#ifdef BIND_VEHICLE_FIREWEAPON
	VEHICLE_FIREWEAPON,
#endif
	VEHICLE_ACCELERATE,
	VEHICLE_BRAKE,
	VEHICLE_CHANGE_RADIO_STATION,
	VEHICLE_HORN,
	TOGGLE_SUBMISSIONS,
	VEHICLE_HANDBRAKE,
	PED_1RST_PERSON_LOOK_LEFT,
	PED_1RST_PERSON_LOOK_RIGHT,
	VEHICLE_LOOKLEFT,
	VEHICLE_LOOKRIGHT,
	VEHICLE_LOOKBEHIND,
	VEHICLE_TURRETLEFT,
	VEHICLE_TURRETRIGHT,
	VEHICLE_TURRETUP,
	VEHICLE_TURRETDOWN,
	PED_CYCLE_TARGET_LEFT,
	PED_CYCLE_TARGET_RIGHT,
	PED_CENTER_CAMERA_BEHIND_PLAYER,
	PED_LOCK_TARGET,
	NETWORK_TALK,
	PED_1RST_PERSON_LOOK_UP,
	PED_1RST_PERSON_LOOK_DOWN,
	_CONTROLLERACTION_36,					// Unused
	TOGGLE_DPAD,
	SWITCH_DEBUG_CAM_ON,
	TAKE_SCREEN_SHOT,
	SHOW_MOUSE_POINTER_TOGGLE,
	MAX_CONTROLLERACTIONS,
};

enum e_ControllerActionType
{
	ACTIONTYPE_1RSTPERSON = 0,
	ACTIONTYPE_3RDPERSON,
	ACTIONTYPE_VEHICLE,
	ACTIONTYPE_VEHICLE_3RDPERSON,
	ACTIONTYPE_COMMON,
	ACTIONTYPE_1RST3RDPERSON,
	ACTIONTYPE_NONE,
};

enum eContSetOrder
{
	SETORDER_NONE = 0,
	SETORDER_1,
	SETORDER_2,
	SETORDER_3,
	SETORDER_4,
	MAX_SETORDERS,
};

enum eSimCheckers
{
	SIM_X1 = 0, SIM_Y1,	// DPad
	SIM_X2, SIM_Y2,	    // LeftStick

	MAX_SIMS
};

class CMouseControllerState;
class CControllerState;


#define JOY_BUTTONS 16
#define MAX_BUTTONS (JOY_BUTTONS+1)

#define ACTIONNAME_LENGTH 40

#ifdef RW_GL3
struct GlfwJoyState {
	int8 id;
	bool isGamepad;
	uint8 numButtons;
	uint8* buttons;
	bool mappedButtons[17];
};
#endif

class CControllerConfigManager
{
public:
	struct tControllerConfigBind
	{
		int32      m_Key;
		int32      m_ContSetOrder;

		tControllerConfigBind()
		{
			m_Key = 0;
			m_ContSetOrder = 0;
		}
	};

	bool                  m_bFirstCapture;
#if defined RW_GL3
	GlfwJoyState           m_OldState;
	GlfwJoyState           m_NewState;
#else
	DIJOYSTATE2           m_OldState;
	DIJOYSTATE2           m_NewState;
#endif
	wchar                 m_aActionNames[MAX_CONTROLLERACTIONS][ACTIONNAME_LENGTH];
	bool                  m_aButtonStates[MAX_BUTTONS];
	tControllerConfigBind m_aSettings[MAX_CONTROLLERACTIONS][MAX_CONTROLLERTYPES];
	bool                  m_aSimCheckers[MAX_SIMS][MAX_CONTROLLERTYPES];
	bool                  m_bMouseAssociated;

#ifdef LOAD_INI_SETTINGS
	static uint32 ms_padButtonsInited;
#endif
	
	CControllerConfigManager();

	void  MakeControllerActionsBlank();
	
	int32 GetJoyButtonJustDown();
	
	void  SaveSettings(int32 file);
	void  LoadSettings(int32 file);
	
	void  InitDefaultControlConfiguration();
	void  InitDefaultControlConfigMouse(CMouseControllerState const &availableButtons);
	void  InitDefaultControlConfigJoyPad(uint32 buttons);
	void  InitialiseControllerActionNameArray();

	void  UpdateJoyInConfigMenus_ButtonDown                           (int32 button, int32 padnumber);
	void  AffectControllerStateOn_ButtonDown                          (int32 button, eControllerType type);
	void  AffectControllerStateOn_ButtonDown_Driving                  (int32 button, eControllerType type, CControllerState &state);
	void  AffectControllerStateOn_ButtonDown_FirstPersonOnly          (int32 button, eControllerType type, CControllerState &state);
	void  AffectControllerStateOn_ButtonDown_ThirdPersonOnly          (int32 button, eControllerType type, CControllerState &state);
	void  AffectControllerStateOn_ButtonDown_FirstAndThirdPersonOnly  (int32 button, eControllerType type, CControllerState &state);
	void  AffectControllerStateOn_ButtonDown_AllStates                (int32 button, eControllerType type, CControllerState &state);
	void  AffectControllerStateOn_ButtonDown_VehicleAndThirdPersonOnly(int32 button, eControllerType type, CControllerState &state);

	void  UpdateJoyInConfigMenus_ButtonUp(int32 button, int32 padnumber);
	void  AffectControllerStateOn_ButtonUp(int32 button, eControllerType type);
	void  AffectControllerStateOn_ButtonUp_All_Player_States(int32 button, eControllerType type, CControllerState &state);
	
	void  AffectPadFromKeyBoard();
	void  AffectPadFromMouse();
	
	void  ClearSimButtonPressCheckers();

	bool  GetIsKeyboardKeyDown    (RsKeyCodes keycode);
	bool  GetIsKeyboardKeyJustDown(RsKeyCodes keycode);
	bool  GetIsMouseButtonDown    (RsKeyCodes keycode);
	bool  GetIsMouseButtonUp      (RsKeyCodes keycode);


	void DeleteMatchingCommonControls           (e_ControllerAction action, int32 key, eControllerType type);
	void DeleteMatching3rdPersonControls        (e_ControllerAction action, int32 key, eControllerType type);
	void DeleteMatching1rst3rdPersonControls    (e_ControllerAction action, int32 key, eControllerType type);
	void DeleteMatchingVehicleControls          (e_ControllerAction action, int32 key, eControllerType type);
	void DeleteMatchingVehicle_3rdPersonControls(e_ControllerAction action, int32 key, eControllerType type);
	void DeleteMatching1rstPersonControls       (e_ControllerAction action, int32 key, eControllerType type);
	void DeleteMatchingActionInitiators         (e_ControllerAction action, int32 key, eControllerType type);

#ifdef RADIO_SCROLL_TO_PREV_STATION
	bool IsAnyVehicleActionAssignedToMouseKey(int32 key);
#endif 

	bool GetIsKeyBlank(int32 key, eControllerType type);
	e_ControllerActionType GetActionType(e_ControllerAction action);

	void ClearSettingsAssociatedWithAction        (e_ControllerAction action, eControllerType type);
	wchar *GetControllerSettingTextWithOrderNumber(e_ControllerAction action, eContSetOrder setorder);
	wchar *GetControllerSettingTextKeyBoard       (e_ControllerAction action, eControllerType type);
	wchar *GetControllerSettingTextMouse          (e_ControllerAction action);
	wchar *GetControllerSettingTextJoystick       (e_ControllerAction action);

	int32 GetNumOfSettingsForAction(e_ControllerAction action);
	void  GetWideStringOfCommandKeys(uint16 action, wchar *text, uint16 leight);
	int32 GetControllerKeyAssociatedWithAction(e_ControllerAction action, eControllerType type);

	void  UpdateJoyButtonState(int32 padnumber);
	
	bool  GetIsActionAButtonCombo             (e_ControllerAction action);
	wchar *GetButtonComboText                 (e_ControllerAction action);
	void  SetControllerKeyAssociatedWithAction(e_ControllerAction action, int32 key, eControllerType type);
	int32 GetMouseButtonAssociatedWithAction  (e_ControllerAction action);
	void  SetMouseButtonAssociatedWithAction  (e_ControllerAction action, int32 button);
	void  ResetSettingOrder                   (e_ControllerAction action);
};

#ifndef RW_GL3
VALIDATE_SIZE(CControllerConfigManager, 0x143C);
#endif

extern CControllerConfigManager ControlsManager;