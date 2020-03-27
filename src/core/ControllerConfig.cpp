#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include "common.h"
#include "patcher.h"
#include "ControllerConfig.h"
#include "Pad.h"
#include "FileMgr.h"
#include "Text.h"
#include "Font.h"
#include "Messages.h"
#include "Frontend.h"
#include "Ped.h"
#include "PlayerPed.h"
#include "Vehicle.h"
#include "World.h"
#include "ModelIndices.h"
#include "Camera.h"
#include "win.h"
#include "GenericGameStorage.h"

CControllerConfigManager &ControlsManager = *(CControllerConfigManager*)0x8F43A4;

CControllerConfigManager::CControllerConfigManager()
{
	m_bFirstCapture    = false;
	m_bMouseAssociated = false;

	MakeControllerActionsBlank();
	InitDefaultControlConfiguration();
	InitialiseControllerActionNameArray();
}

void CControllerConfigManager::MakeControllerActionsBlank()
{
	for (int32 i = 0; i < MAX_CONTROLLERTYPES; i++)
	{
		for (int32 j = 0; j < MAX_CONTROLLERACTIONS; j++)
		{
			ClearSettingsAssociatedWithAction((e_ControllerAction)j, (eControllerType)i);
		}
	}
}

int32 CControllerConfigManager::GetJoyButtonJustDown()
{
#ifdef __DINPUT_INCLUDED__
	for (int32 i = 0; i < JOY_BUTTONS; i++)
	{
		if (m_NewState.rgbButtons[i] & 0x80 && !(m_OldState.rgbButtons[i] & 0x80))
			return i + 1;
	}
#endif
	return 0;
}

void CControllerConfigManager::SaveSettings(int32 file)
{
	if (file)
	{
		for (int32 i = 0; i < MAX_CONTROLLERTYPES; i++)
		{
			for (int32 j = 0; j < MAX_CONTROLLERACTIONS; j++)
			{
				CFileMgr::Write(file, (char *)&ControlsManager.m_aSettings[j][i], sizeof(tControllerConfigBind));
			}
		}
	}
}

void CControllerConfigManager::LoadSettings(int32 file)
{
	bool bValid = true;

	if (file)
	{
		char buff[29];
		CFileMgr::Read(file, buff, sizeof(buff));

		if (!strncmp(buff, TopLineEmptyFile, sizeof(TopLineEmptyFile)-1))
			bValid = false;
		else
			CFileMgr::Seek(file, 0, 0);
	}

	if (bValid)
	{
		ControlsManager.MakeControllerActionsBlank();

		for (int32 i = 0; i < MAX_CONTROLLERTYPES; i++)
		{
			for (int32 j = 0; j < MAX_CONTROLLERACTIONS; j++)
			{
				CFileMgr::Read(file, (char *)&ControlsManager.m_aSettings[j][i], sizeof(tControllerConfigBind));
			}
		}
	}
}

void CControllerConfigManager::InitDefaultControlConfiguration()
{
	SetControllerKeyAssociatedWithAction    (VEHICLE_LOOKLEFT,                    rsPADEND,   KEYBOARD);
	SetControllerKeyAssociatedWithAction    (VEHICLE_LOOKLEFT,                    'Q',        OPTIONAL_EXTRA);
										    						              
	SetControllerKeyAssociatedWithAction    (VEHICLE_LOOKRIGHT,                   rsPADDOWN,  KEYBOARD);
	SetControllerKeyAssociatedWithAction    (VEHICLE_LOOKRIGHT,                   'E',        OPTIONAL_EXTRA);
																	              
	if ( _dwOperatingSystemVersion != OS_WIN98 )						              
	{																              
		SetControllerKeyAssociatedWithAction(VEHICLE_HORN,                        rsLSHIFT,   OPTIONAL_EXTRA);
		SetControllerKeyAssociatedWithAction(VEHICLE_HORN,                        rsRSHIFT,   KEYBOARD);
	}																              
	else															              
		SetControllerKeyAssociatedWithAction(VEHICLE_HORN,                        rsSHIFT,    OPTIONAL_EXTRA); // BUG: must be KEYBOARD ?
																	              
	SetControllerKeyAssociatedWithAction    (VEHICLE_HANDBRAKE,                   rsRCTRL,    KEYBOARD);
	SetControllerKeyAssociatedWithAction    (VEHICLE_HANDBRAKE,                   ' ',        OPTIONAL_EXTRA);
															                      
	SetControllerKeyAssociatedWithAction    (VEHICLE_ENTER_EXIT,                  rsENTER,    KEYBOARD);
	SetControllerKeyAssociatedWithAction    (VEHICLE_ENTER_EXIT,                  'F',        OPTIONAL_EXTRA);
										    					                  
	SetControllerKeyAssociatedWithAction    (VEHICLE_ACCELERATE,                  rsUP,       KEYBOARD);
	SetControllerKeyAssociatedWithAction    (VEHICLE_ACCELERATE,                  'W',        OPTIONAL_EXTRA);
																		          
	SetControllerKeyAssociatedWithAction    (VEHICLE_CHANGE_RADIO_STATION,        rsINS,      KEYBOARD);
	SetControllerKeyAssociatedWithAction    (VEHICLE_CHANGE_RADIO_STATION,        'R',        OPTIONAL_EXTRA);
																		          
	SetControllerKeyAssociatedWithAction    (VEHICLE_BRAKE,                       rsDOWN,     KEYBOARD);
	SetControllerKeyAssociatedWithAction    (VEHICLE_BRAKE,                       'S',        OPTIONAL_EXTRA);
																		          
	SetControllerKeyAssociatedWithAction    (TOGGLE_SUBMISSIONS,                  rsPLUS,     KEYBOARD);
	SetControllerKeyAssociatedWithAction    (TOGGLE_SUBMISSIONS,                  rsCAPSLK,   OPTIONAL_EXTRA);
																		          
	SetControllerKeyAssociatedWithAction    (GO_LEFT,                             rsLEFT,     KEYBOARD);
	SetControllerKeyAssociatedWithAction    (GO_LEFT,                             'A',        OPTIONAL_EXTRA);
																		          
	SetControllerKeyAssociatedWithAction    (GO_RIGHT,                            rsRIGHT,    KEYBOARD);
	SetControllerKeyAssociatedWithAction    (GO_RIGHT,                            'D',        OPTIONAL_EXTRA);
																		          
	SetControllerKeyAssociatedWithAction    (GO_FORWARD,                          rsUP,       KEYBOARD);
	SetControllerKeyAssociatedWithAction    (GO_FORWARD,                          'W',        OPTIONAL_EXTRA);
																		          
	SetControllerKeyAssociatedWithAction    (GO_BACK,                             rsDOWN,     KEYBOARD);
	SetControllerKeyAssociatedWithAction    (GO_BACK,                             'S',        OPTIONAL_EXTRA);
																		          
	SetControllerKeyAssociatedWithAction    (PED_LOOKBEHIND,                      rsPADEND,   KEYBOARD);
	SetControllerKeyAssociatedWithAction    (PED_LOOKBEHIND,                      rsCAPSLK,   OPTIONAL_EXTRA);
																		          
	SetControllerKeyAssociatedWithAction    (PED_FIREWEAPON,                      rsPADINS,   KEYBOARD);
	SetControllerKeyAssociatedWithAction    (PED_FIREWEAPON,                      rsLCTRL,    OPTIONAL_EXTRA);
																		          
	SetControllerKeyAssociatedWithAction    (PED_CYCLE_WEAPON_LEFT,               rsPADDEL,   KEYBOARD);

	SetControllerKeyAssociatedWithAction    (PED_CYCLE_WEAPON_RIGHT,              rsPADENTER, OPTIONAL_EXTRA); // BUG: must be KEYBOARD ?
																		          
	SetControllerKeyAssociatedWithAction    (PED_LOCK_TARGET,                     rsDEL,      KEYBOARD);
																		          
	SetControllerKeyAssociatedWithAction    (PED_JUMPING,                         rsRCTRL,    KEYBOARD);
	SetControllerKeyAssociatedWithAction    (PED_JUMPING,                         ' ',        OPTIONAL_EXTRA);
																		          
	if ( _dwOperatingSystemVersion != OS_WIN98 )							          
	{																	          
		SetControllerKeyAssociatedWithAction(PED_SPRINT,                          rsLSHIFT,   OPTIONAL_EXTRA);
#ifndef FIX_BUGS
		SetControllerKeyAssociatedWithAction(PED_SPRINT,                          rsRSHIFT,   OPTIONAL_EXTRA); // BUG: must be KEYBOARD
#else
		SetControllerKeyAssociatedWithAction(PED_SPRINT,                          rsRSHIFT,   KEYBOARD);
#endif
	}																	          
	else																          
		SetControllerKeyAssociatedWithAction(PED_SPRINT,                          rsSHIFT,    OPTIONAL_EXTRA); // BUG: must be KEYBOARD ?
																		          
	SetControllerKeyAssociatedWithAction    (PED_CYCLE_TARGET_LEFT,               '[',        KEYBOARD);

	SetControllerKeyAssociatedWithAction    (PED_CYCLE_TARGET_RIGHT,              ']',        OPTIONAL_EXTRA); // BUG: must be KEYBOARD ?
																			      
	SetControllerKeyAssociatedWithAction    (PED_CENTER_CAMERA_BEHIND_PLAYER,     '#',        KEYBOARD);
	
	SetControllerKeyAssociatedWithAction    (PED_SNIPER_ZOOM_IN,                  rsPGUP,     KEYBOARD);
	SetControllerKeyAssociatedWithAction    (PED_SNIPER_ZOOM_IN,                  'Z',        OPTIONAL_EXTRA);
										    
	SetControllerKeyAssociatedWithAction    (PED_SNIPER_ZOOM_OUT,                 rsPGDN,     KEYBOARD);
	SetControllerKeyAssociatedWithAction    (PED_SNIPER_ZOOM_OUT,                 'X',        OPTIONAL_EXTRA);
										    
	SetControllerKeyAssociatedWithAction    (PED_1RST_PERSON_LOOK_LEFT,           rsPADLEFT,  KEYBOARD);

	SetControllerKeyAssociatedWithAction    (PED_1RST_PERSON_LOOK_RIGHT,          rsPADRIGHT, KEYBOARD);
										    
	SetControllerKeyAssociatedWithAction    (PED_1RST_PERSON_LOOK_UP,             rsPADUP,    KEYBOARD);

	SetControllerKeyAssociatedWithAction    (PED_1RST_PERSON_LOOK_DOWN,           rsPAD5,     KEYBOARD);
										    
	SetControllerKeyAssociatedWithAction    (VEHICLE_TURRETLEFT,                  rsPADLEFT,  KEYBOARD);

	SetControllerKeyAssociatedWithAction    (VEHICLE_TURRETRIGHT,                 rsPAD5,     KEYBOARD);
										    
	SetControllerKeyAssociatedWithAction    (VEHICLE_TURRETUP,                    rsPADPGUP,  KEYBOARD);

	SetControllerKeyAssociatedWithAction    (VEHICLE_TURRETDOWN,                  rsPADRIGHT, KEYBOARD);
										    
	SetControllerKeyAssociatedWithAction    (CAMERA_CHANGE_VIEW_ALL_SITUATIONS,   rsHOME,     KEYBOARD);
	SetControllerKeyAssociatedWithAction    (CAMERA_CHANGE_VIEW_ALL_SITUATIONS,   'C',        OPTIONAL_EXTRA);

	for (int32 i = 0; i < MAX_SIMS; i++)
	{
		m_aSimCheckers[i][KEYBOARD]       = false;
		m_aSimCheckers[i][OPTIONAL_EXTRA] = false;
		m_aSimCheckers[i][MOUSE]          = false;
		m_aSimCheckers[i][JOYSTICK]       = false;
	}
}

void CControllerConfigManager::InitDefaultControlConfigMouse(CMouseControllerState const &availableButtons)
{
	if (availableButtons.LMB)
	{
		m_bMouseAssociated = true;
		SetMouseButtonAssociatedWithAction(PED_FIREWEAPON,               1);
	}

	if (availableButtons.RMB)
	{
		SetMouseButtonAssociatedWithAction(PED_LOCK_TARGET,              3);

		SetMouseButtonAssociatedWithAction(VEHICLE_HANDBRAKE,            3);
	}

	if (availableButtons.MMB)
	{
		SetMouseButtonAssociatedWithAction(VEHICLE_LOOKBEHIND,           2);

		SetMouseButtonAssociatedWithAction(PED_LOOKBEHIND,               2);
	}

	if (availableButtons.WHEELUP || availableButtons.WHEELDN)
	{
		SetMouseButtonAssociatedWithAction(PED_CYCLE_WEAPON_LEFT,        4);

		SetMouseButtonAssociatedWithAction(PED_CYCLE_WEAPON_RIGHT,       5);

		SetMouseButtonAssociatedWithAction(VEHICLE_CHANGE_RADIO_STATION, 4);
	}
}

void CControllerConfigManager::InitDefaultControlConfigJoyPad(uint32 buttons)
{
	m_bFirstCapture = true;

	uint32 btn = buttons;
	if (buttons > 16)
		btn = 16;

	if (   AllValidWinJoys.m_aJoys[JOYSTICK1].m_nVendorID == 0x3427
		&& AllValidWinJoys.m_aJoys[JOYSTICK1].m_nProductID == 0x1190)
	{
		//GIC USB Joystick, PS2 Gamepad ?

		switch (btn)
		{
		case 16:
			SetControllerKeyAssociatedWithAction(GO_LEFT,                           16, JOYSTICK);
		case 15:											                        
			SetControllerKeyAssociatedWithAction(GO_BACK,                           15, JOYSTICK);
		case 14:											                        
			SetControllerKeyAssociatedWithAction(GO_RIGHT,                          14, JOYSTICK);
		case 13:											                        
			SetControllerKeyAssociatedWithAction(GO_FORWARD,                        13, JOYSTICK);
		case 12:													                
		case 11:													                
			SetControllerKeyAssociatedWithAction(PED_LOOKBEHIND,                    11, JOYSTICK);
			SetControllerKeyAssociatedWithAction(TOGGLE_SUBMISSIONS,                11, JOYSTICK);
		case 10:
			SetControllerKeyAssociatedWithAction(VEHICLE_HORN,                      10, JOYSTICK);
		case 9:
			SetControllerKeyAssociatedWithAction(CAMERA_CHANGE_VIEW_ALL_SITUATIONS,  9, JOYSTICK);
		case 8:
			SetControllerKeyAssociatedWithAction(VEHICLE_HANDBRAKE,                  8, JOYSTICK);
			SetControllerKeyAssociatedWithAction(PED_LOCK_TARGET,                    8, JOYSTICK);
		case 7:
			SetControllerKeyAssociatedWithAction(PED_CENTER_CAMERA_BEHIND_PLAYER,    7, JOYSTICK);
			SetControllerKeyAssociatedWithAction(VEHICLE_CHANGE_RADIO_STATION,       7, JOYSTICK);
		case 6:
			SetControllerKeyAssociatedWithAction(PED_CYCLE_WEAPON_RIGHT,             6, JOYSTICK);
			SetControllerKeyAssociatedWithAction(VEHICLE_LOOKRIGHT,                  6, JOYSTICK);
		case 5:
			SetControllerKeyAssociatedWithAction(PED_CYCLE_WEAPON_LEFT,              5, JOYSTICK);
			SetControllerKeyAssociatedWithAction(VEHICLE_LOOKLEFT,                   5, JOYSTICK);
		/*******************************************************************************************/
		case 4:
			SetControllerKeyAssociatedWithAction(VEHICLE_BRAKE,                      4, JOYSTICK);
			SetControllerKeyAssociatedWithAction(PED_JUMPING,                        4, JOYSTICK);
			SetControllerKeyAssociatedWithAction(PED_SNIPER_ZOOM_IN,                 4, JOYSTICK);
		case 3:
			SetControllerKeyAssociatedWithAction(VEHICLE_ACCELERATE,                 3, JOYSTICK);
			SetControllerKeyAssociatedWithAction(PED_SPRINT,                         3, JOYSTICK);
			SetControllerKeyAssociatedWithAction(PED_SNIPER_ZOOM_OUT,                3, JOYSTICK);
		case 2:
			SetControllerKeyAssociatedWithAction(PED_FIREWEAPON,                     2, JOYSTICK);
		case 1:
			SetControllerKeyAssociatedWithAction(VEHICLE_ENTER_EXIT,                 1, JOYSTICK);
		/*******************************************************************************************/
		}
	}
	else
	{
		switch (btn)
		{
		case 16:
			SetControllerKeyAssociatedWithAction(GO_LEFT,                           16, JOYSTICK);
		case 15:
			SetControllerKeyAssociatedWithAction(GO_BACK,                           15, JOYSTICK);
		case 14:
			SetControllerKeyAssociatedWithAction(GO_RIGHT,                          14, JOYSTICK);
		case 13:
			SetControllerKeyAssociatedWithAction(GO_FORWARD,                        13, JOYSTICK);
		case 12:
		case 11:
			SetControllerKeyAssociatedWithAction(PED_LOOKBEHIND,                    11, JOYSTICK);
			SetControllerKeyAssociatedWithAction(TOGGLE_SUBMISSIONS,                11, JOYSTICK);
		case 10:
			SetControllerKeyAssociatedWithAction(VEHICLE_HORN,                      10, JOYSTICK);
		case 9:
			SetControllerKeyAssociatedWithAction(CAMERA_CHANGE_VIEW_ALL_SITUATIONS,  9, JOYSTICK);
		case 8:
			SetControllerKeyAssociatedWithAction(VEHICLE_HANDBRAKE,                  8, JOYSTICK);
			SetControllerKeyAssociatedWithAction(PED_LOCK_TARGET,                    8, JOYSTICK);
		case 7:
			SetControllerKeyAssociatedWithAction(PED_CENTER_CAMERA_BEHIND_PLAYER,    7, JOYSTICK);
			SetControllerKeyAssociatedWithAction(VEHICLE_CHANGE_RADIO_STATION,       7, JOYSTICK);
		case 6:
			SetControllerKeyAssociatedWithAction(PED_CYCLE_WEAPON_RIGHT,             6, JOYSTICK);
			SetControllerKeyAssociatedWithAction(VEHICLE_LOOKRIGHT,                  6, JOYSTICK);
		case 5:
			SetControllerKeyAssociatedWithAction(PED_CYCLE_WEAPON_LEFT,              5, JOYSTICK);
			SetControllerKeyAssociatedWithAction(VEHICLE_LOOKLEFT,                   5, JOYSTICK);
		/*******************************************************************************************/
		case 4:
			SetControllerKeyAssociatedWithAction(VEHICLE_ENTER_EXIT,                 4, JOYSTICK);
		case 3:
			SetControllerKeyAssociatedWithAction(VEHICLE_BRAKE,                      3, JOYSTICK);
			SetControllerKeyAssociatedWithAction(PED_JUMPING,                        3, JOYSTICK);
			SetControllerKeyAssociatedWithAction(PED_SNIPER_ZOOM_IN,                 3, JOYSTICK);
		case 2:
			SetControllerKeyAssociatedWithAction(VEHICLE_ACCELERATE,                 2, JOYSTICK);
			SetControllerKeyAssociatedWithAction(PED_SPRINT,                         2, JOYSTICK);
			SetControllerKeyAssociatedWithAction(PED_SNIPER_ZOOM_OUT,                2, JOYSTICK);
		case 1:
			SetControllerKeyAssociatedWithAction(PED_FIREWEAPON,                     1, JOYSTICK);
		/*******************************************************************************************/
		}
	}
}

void CControllerConfigManager::InitialiseControllerActionNameArray()
{
	wchar buf[ACTIONNAME_LENGTH + 2];

#define SETACTIONNAME(name) AsciiToUnicode(#name, buf); CMessages::WideStringCopy(m_aActionNames[name], buf, ACTIONNAME_LENGTH);

	SETACTIONNAME(PED_LOOKBEHIND);
	SETACTIONNAME(PED_CYCLE_WEAPON_LEFT);
	SETACTIONNAME(PED_CYCLE_WEAPON_RIGHT);
	SETACTIONNAME(PED_LOCK_TARGET);
	SETACTIONNAME(PED_JUMPING);
	SETACTIONNAME(PED_SPRINT);
	SETACTIONNAME(PED_CYCLE_TARGET_LEFT);
	SETACTIONNAME(PED_CYCLE_TARGET_RIGHT);
	SETACTIONNAME(PED_CENTER_CAMERA_BEHIND_PLAYER);
	SETACTIONNAME(VEHICLE_LOOKBEHIND);
	SETACTIONNAME(VEHICLE_LOOKLEFT);
	SETACTIONNAME(VEHICLE_LOOKRIGHT);
	SETACTIONNAME(VEHICLE_HORN);
	SETACTIONNAME(VEHICLE_HANDBRAKE);
	SETACTIONNAME(VEHICLE_ACCELERATE);
	SETACTIONNAME(VEHICLE_BRAKE);
	SETACTIONNAME(VEHICLE_CHANGE_RADIO_STATION);
	SETACTIONNAME(TOGGLE_SUBMISSIONS);
	SETACTIONNAME(PED_SNIPER_ZOOM_IN);
	SETACTIONNAME(PED_SNIPER_ZOOM_OUT);
	SETACTIONNAME(PED_1RST_PERSON_LOOK_LEFT);
	SETACTIONNAME(PED_1RST_PERSON_LOOK_RIGHT);
	SETACTIONNAME(PED_1RST_PERSON_LOOK_UP);
	SETACTIONNAME(PED_1RST_PERSON_LOOK_DOWN);
	SETACTIONNAME(SHOW_MOUSE_POINTER_TOGGLE);
	SETACTIONNAME(CAMERA_CHANGE_VIEW_ALL_SITUATIONS);
	SETACTIONNAME(PED_FIREWEAPON);
	SETACTIONNAME(VEHICLE_ENTER_EXIT);
	SETACTIONNAME(GO_LEFT);
	SETACTIONNAME(GO_RIGHT);
	SETACTIONNAME(GO_FORWARD);
	SETACTIONNAME(GO_BACK);
	SETACTIONNAME(NETWORK_TALK);
	SETACTIONNAME(TOGGLE_DPAD);
	SETACTIONNAME(SWITCH_DEBUG_CAM_ON);
	SETACTIONNAME(TAKE_SCREEN_SHOT);

#undef SETACTIONNAME
}

void CControllerConfigManager::UpdateJoyInConfigMenus_ButtonDown(int32 button, int32 padnumber)
{
	if (button != 0)
	{
		CPad *pad = CPad::GetPad(padnumber);
		if (pad != NULL)
		{
			switch (button)
			{
			case 16:
				pad->PCTempJoyState.DPadLeft = 255;
				break;
			case 15:
				pad->PCTempJoyState.DPadDown = 255;
				break;
			case 14:
				pad->PCTempJoyState.DPadRight = 255;
				break;
			case 13:
				pad->PCTempJoyState.DPadUp = 255;
				break;
#ifdef REGISTER_START_BUTTON
			case 12:
				pad->PCTempJoyState.Start = 255;
				break;
#endif
			case 11:
				pad->PCTempJoyState.RightShock = 255;
				break;
			case 10:
				pad->PCTempJoyState.LeftShock = 255;
				break;
			case 9:
				pad->PCTempJoyState.Select = 255;
				break;
			case 8:
				pad->PCTempJoyState.RightShoulder1 = 255;
				break;
			case 7:
				pad->PCTempJoyState.LeftShoulder1 = 255;
				break;
			case 6:
				pad->PCTempJoyState.RightShoulder2 = 255;
				break;
			case 5:
				pad->PCTempJoyState.LeftShoulder2 = 255;
				break;
			}

			if (   AllValidWinJoys.m_aJoys[JOYSTICK1].m_nVendorID  == 0x3427
				&& AllValidWinJoys.m_aJoys[JOYSTICK1].m_nProductID == 0x1190)
			{
				//GIC USB Joystick, PS2 Gamepad ?

				switch (button)
				{
				case 4:
					pad->PCTempJoyState.Square = 255;
					break;
				case 3:
					pad->PCTempJoyState.Cross = 255;
					break;
				case 2:
					pad->PCTempJoyState.Circle = 255;
					break;
				case 1:
					pad->PCTempJoyState.Triangle = 255;
					break;
				}
			}
			else
			{
				switch (button)
				{
				case 4:
					pad->PCTempJoyState.Triangle = 255;
					break;
				case 3:
					pad->PCTempJoyState.Square = 255;
					break;
				case 2:
					pad->PCTempJoyState.Cross = 255;
					break;
				case 1:
					pad->PCTempJoyState.Circle = 255;
					break;
				}
			}
		}
	}
}

void CControllerConfigManager::AffectControllerStateOn_ButtonDown(int32 button, eControllerType type)
{
	bool process = true;
	
	if ((type == KEYBOARD || type == OPTIONAL_EXTRA) && button == rsNULL)
		process = false;
	if (type == JOYSTICK && button == 0)
		process = false;
	if (type == MOUSE && button == 0)
		process = false;

	if (process)
	{
		CPad *pad = CPad::GetPad(PAD1);

		bool firstPerson = false;
		bool playerDriving = false;

		if (FindPlayerVehicle() != NULL)
		{
			CPlayerPed *plr = FindPlayerPed();
			if (plr != NULL)
			{
				if (plr->m_nPedState == PED_DRIVING)
					playerDriving = true;
			}
		}

		int16 mode = TheCamera.Cams[TheCamera.ActiveCam].Mode;
		if (   mode == CCam::MODE_1STPERSON
			|| mode == CCam::MODE_SNIPER
			|| mode == CCam::MODE_ROCKETLAUNCHER
			|| mode == CCam::MODE_M16_1STPERSON)
		{
			firstPerson = true;
		}

		CControllerState *state;
		
		switch (type)
		{
		case KEYBOARD:
		case OPTIONAL_EXTRA:
			state = &CPad::GetPad(PAD1)->PCTempKeyState;
			break;
		case JOYSTICK:
			state = &CPad::GetPad(PAD1)->PCTempJoyState;
			break;
		case MOUSE:
			state = &CPad::GetPad(PAD1)->PCTempMouseState;
			break;
		}

		if (pad != NULL)
		{
			if (playerDriving)
			{
				AffectControllerStateOn_ButtonDown_Driving(button, type, *state);
				AffectControllerStateOn_ButtonDown_VehicleAndThirdPersonOnly(button, type, *state);
			}
			else
			{
				AffectControllerStateOn_ButtonDown_FirstAndThirdPersonOnly(button, type, *state);
				if (firstPerson)
					AffectControllerStateOn_ButtonDown_FirstPersonOnly(button, type, *state);
				else
				{
					AffectControllerStateOn_ButtonDown_ThirdPersonOnly(button, type, *state);
					AffectControllerStateOn_ButtonDown_VehicleAndThirdPersonOnly(button, type, *state);
				}
			}

			AffectControllerStateOn_ButtonDown_AllStates(button, type, *state);
		}
	}
}

void CControllerConfigManager::AffectControllerStateOn_ButtonDown_Driving(int32 button, eControllerType type, CControllerState &state)
{
	if (button == GetControllerKeyAssociatedWithAction(VEHICLE_LOOKBEHIND, type))
	{
		state.LeftShoulder2 = 255;
		state.RightShoulder2 = 255;
	}

	if (button == GetControllerKeyAssociatedWithAction(VEHICLE_LOOKLEFT, type))
		state.LeftShoulder2 = 255;
	if (button == GetControllerKeyAssociatedWithAction(VEHICLE_LOOKRIGHT, type))
		state.RightShoulder2 = 255;
	if (button == GetControllerKeyAssociatedWithAction(VEHICLE_HORN, type))
		state.LeftShock = 255;
	if (button == GetControllerKeyAssociatedWithAction(VEHICLE_HANDBRAKE, type))
		state.RightShoulder1 = 255;
	if (button == GetControllerKeyAssociatedWithAction(VEHICLE_ACCELERATE, type))
		state.Cross = 255;
	if (button == GetControllerKeyAssociatedWithAction(VEHICLE_CHANGE_RADIO_STATION, type))
		state.LeftShoulder1 = 255;
	if (button == GetControllerKeyAssociatedWithAction(VEHICLE_BRAKE, type))
		state.Square = 255;
	if (button == GetControllerKeyAssociatedWithAction(TOGGLE_SUBMISSIONS, type))
		state.RightShock = 255;
	
	if (button == GetControllerKeyAssociatedWithAction(VEHICLE_TURRETLEFT, type))
	{
		if (state.RightStickX == 128 || m_aSimCheckers[SIM_X2][type])
		{
			state.RightStickX = 0;
			m_aSimCheckers[SIM_X2][type] = true;
		}
		else
		{
			state.RightStickX = -128;
		}
	}

	if (button == GetControllerKeyAssociatedWithAction(VEHICLE_TURRETRIGHT, type))
	{
		if (state.RightStickX == -128 || m_aSimCheckers[SIM_X2][type])
		{
			state.RightStickX = 0;
			m_aSimCheckers[SIM_X2][type] = true;
		}
		else
			state.RightStickX = 128;
	}
	
	bool isDodo = false;
	if (FindPlayerVehicle() && (FindPlayerVehicle()->IsVehicle() && FindPlayerVehicle()->GetModelIndex() == MI_DODO))
	{
		isDodo = true;
	}

	
	if (button == GetControllerKeyAssociatedWithAction(VEHICLE_TURRETUP, type))
	{
		if (isDodo == true)
		{
			if (state.LeftStickY == -128 || m_aSimCheckers[SIM_Y1][type]) // BUG: should be SIM_Y2. SIM_Y1 it's DPAD
			{
				state.LeftStickY = 0;
				m_aSimCheckers[SIM_Y2][type] = true;
			}
			else
				state.LeftStickY = 128;
		}

		else if (state.RightStickY == -128 || m_aSimCheckers[SIM_Y2][type])
		{
			state.RightStickY = 0;
			m_aSimCheckers[SIM_Y2][type] = true;
		}
		else
		{
			state.RightStickY = 128;
		}
	}
	
	if (button == GetControllerKeyAssociatedWithAction(VEHICLE_TURRETDOWN, type))
	{
		if (isDodo == true)
		{
			if (state.LeftStickY == 128 || m_aSimCheckers[SIM_Y1][type]) // BUG: should be SIM_Y2. SIM_Y1 it's DPAD
			{
				state.LeftStickY = 0;
				m_aSimCheckers[SIM_Y2][type] = true;
			}
			else
				state.LeftStickY = -128;
		}

		else if (state.RightStickY == 128 || m_aSimCheckers[SIM_Y2][type])
		{
			state.RightStickY = 0;
			m_aSimCheckers[SIM_Y2][type] = true;
		}
		else
			state.RightStickY = -128;
	}
}

void CControllerConfigManager::AffectControllerStateOn_ButtonDown_FirstPersonOnly(int32 button, eControllerType type, CControllerState &state)
{
	if (button == GetControllerKeyAssociatedWithAction(PED_SNIPER_ZOOM_IN, type))
		state.Square = 255;
	if (button == GetControllerKeyAssociatedWithAction(PED_SNIPER_ZOOM_OUT, type))
		state.Cross = 255;
}

void CControllerConfigManager::AffectControllerStateOn_ButtonDown_ThirdPersonOnly(int32 button, eControllerType type, CControllerState &state)
{
	if (button == GetControllerKeyAssociatedWithAction(PED_LOOKBEHIND, type))
		state.RightShock = 255;
	if (button == GetControllerKeyAssociatedWithAction(PED_JUMPING, type))
		state.Square = 255;
	if (button == GetControllerKeyAssociatedWithAction(PED_CYCLE_WEAPON_LEFT, type))
		state.LeftShoulder2 = 255;
	if (button == GetControllerKeyAssociatedWithAction(PED_CYCLE_WEAPON_RIGHT, type))
		state.RightShoulder2 = 255;
	if (button == GetControllerKeyAssociatedWithAction(PED_SPRINT, type))
		state.Cross = 255;
	
	if (CMenuManager::m_ControlMethod == CONTROL_CLASSIC)
	{
		if (button == GetControllerKeyAssociatedWithAction(PED_CYCLE_TARGET_LEFT, type))
			state.LeftShoulder2 = 255;
		if (button == GetControllerKeyAssociatedWithAction(PED_CYCLE_TARGET_RIGHT, type))
			state.RightShoulder2 = 255;
		if (button == GetControllerKeyAssociatedWithAction(PED_CENTER_CAMERA_BEHIND_PLAYER, type))
			state.LeftShoulder1 = 255;
	}
}

void CControllerConfigManager::AffectControllerStateOn_ButtonDown_FirstAndThirdPersonOnly(int32 button, eControllerType type, CControllerState &state)
{
	CPad *pad = CPad::GetPad(PAD1);
	
	if (button == GetControllerKeyAssociatedWithAction(PED_LOCK_TARGET, type))
		state.RightShoulder1 = 255;

	if (button == GetControllerKeyAssociatedWithAction(GO_FORWARD, type))
	{
		if (state.DPadDown || m_aSimCheckers[SIM_Y1][type])
		{
			m_aSimCheckers[SIM_Y1][type] = true;
			state.DPadDown = 0;
			state.DPadUp = 0;
		}
		else
			state.DPadUp = 255;
	}

	if (button == GetControllerKeyAssociatedWithAction(GO_BACK, type))
	{
		if (state.DPadUp || m_aSimCheckers[SIM_Y1][type])
		{
			m_aSimCheckers[SIM_Y1][type] = true;
			state.DPadDown = 0;
			state.DPadUp = 0;
		}
		else
			state.DPadDown = 255;
	}

	if (button == GetControllerKeyAssociatedWithAction(PED_1RST_PERSON_LOOK_LEFT, type))
	{
		if (state.RightStickX == 128 || m_aSimCheckers[SIM_X2][type])
		{
			state.RightStickX = 0;
			m_aSimCheckers[SIM_X2][type] = true;
		}
		else
		{
			state.RightStickX = -128;
		}
	}

	if (button == GetControllerKeyAssociatedWithAction(PED_1RST_PERSON_LOOK_RIGHT, type))
	{
		if (state.RightStickX == -128 || m_aSimCheckers[SIM_X2][type])
		{
			state.RightStickX = 0;
			m_aSimCheckers[SIM_X2][type] = true;
		}
		else
			state.RightStickX = 128;
	}

	if (CMenuManager::m_ControlMethod == CONTROL_CLASSIC)
	{
		if (button == GetControllerKeyAssociatedWithAction(PED_1RST_PERSON_LOOK_UP, type))
		{
			if (state.RightStickY == -128 || m_aSimCheckers[SIM_Y2][type])
			{
				state.RightStickY = 0;
				m_aSimCheckers[SIM_Y2][type] = true;
			}
			else
				state.RightStickY = 128;
		}

		if (button == GetControllerKeyAssociatedWithAction(PED_1RST_PERSON_LOOK_DOWN, type))
		{
			if (state.RightStickY == 128 || m_aSimCheckers[SIM_Y2][type])
			{
				state.RightStickY = 0;
				m_aSimCheckers[SIM_Y2][type] = true;
			}
			else
				state.RightStickY = -128;
		}
	}
}

void CControllerConfigManager::AffectControllerStateOn_ButtonDown_AllStates(int32 button, eControllerType type, CControllerState &state)
{
	if (button == GetControllerKeyAssociatedWithAction(CAMERA_CHANGE_VIEW_ALL_SITUATIONS, type))
		state.Select = 255;
	if (button == GetControllerKeyAssociatedWithAction(PED_FIREWEAPON, type))
		state.Circle = 255;

	if (button == GetControllerKeyAssociatedWithAction(GO_LEFT, type))
	{
		if (state.DPadRight || m_aSimCheckers[SIM_X1][type])
		{
			m_aSimCheckers[SIM_X1][type] = true;
			state.DPadLeft = 0;
			state.DPadRight = 0;
		}
		else
			state.DPadLeft = 255;
	}

	if (button == GetControllerKeyAssociatedWithAction(GO_RIGHT, type))
	{
		if (state.DPadLeft || m_aSimCheckers[SIM_X1][type])
		{
			m_aSimCheckers[SIM_X1][type] = true;
			state.DPadLeft = 0;
			state.DPadRight = 0;
		}
		else
			state.DPadRight = 255;
	}

	if (button == GetControllerKeyAssociatedWithAction(NETWORK_TALK, type))
		state.NetworkTalk = 255;
}

void CControllerConfigManager::AffectControllerStateOn_ButtonDown_VehicleAndThirdPersonOnly(int32 button, eControllerType type, CControllerState &state)
{
	if (button == GetControllerKeyAssociatedWithAction(VEHICLE_ENTER_EXIT, type))
		state.Triangle = 255;
}

void CControllerConfigManager::UpdateJoyInConfigMenus_ButtonUp(int32 button, int32 padnumber)
{
	if (button!=0)
	{
		CPad *pad = CPad::GetPad(padnumber);

		if (pad != NULL)
		{
			switch (button)
			{
			case 16:
				pad->PCTempJoyState.DPadLeft = 0;
				break;
			case 15:
				pad->PCTempJoyState.DPadDown = 0;
				break;
			case 14:
				pad->PCTempJoyState.DPadRight = 0;
				break;
			case 13:
				pad->PCTempJoyState.DPadUp = 0;
				break;
#ifdef REGISTER_START_BUTTON
			case 12:
				pad->PCTempJoyState.Start = 0;
				break;
#endif
			case 11:
				pad->PCTempJoyState.RightShock = 0;
				break;
			case 10:
				pad->PCTempJoyState.LeftShock = 0;
				break;
			case 9:
				pad->PCTempJoyState.Select = 0;
				break;
			case 8:
				pad->PCTempJoyState.RightShoulder1 = 0;
				break;
			case 7:
				pad->PCTempJoyState.LeftShoulder1 = 0;
				break;
			case 6:
				pad->PCTempJoyState.RightShoulder2 = 0;
				break;
			case 5:
				pad->PCTempJoyState.LeftShoulder2 = 0;
				break;
			}

			if (   AllValidWinJoys.m_aJoys[JOYSTICK1].m_nVendorID  == 0x3427
				&& AllValidWinJoys.m_aJoys[JOYSTICK1].m_nProductID == 0x1190)
			{
				//GIC USB Joystick, PS2 Gamepad ?

				switch (button)
				{
				case 4:
					pad->PCTempJoyState.Square = 0;
					break;
				case 3:
					pad->PCTempJoyState.Cross = 0;
					break;
				case 2:
					pad->PCTempJoyState.Circle = 0;
					break;
				case 1:
					pad->PCTempJoyState.Triangle = 0;
					break;
				}
			}
			else
			{
				switch (button)
				{
				case 4:
					pad->PCTempJoyState.Triangle = 0;
					break;
				case 3:
					pad->PCTempJoyState.Square = 0;
					break;
				case 2:
					pad->PCTempJoyState.Cross = 0;
					break;
				case 1:
					pad->PCTempJoyState.Circle = 0;
					break;
				}
			}
		}
	}
}

void CControllerConfigManager::AffectControllerStateOn_ButtonUp(int32 button, eControllerType type)
{
	bool process = true;

	if ((type == KEYBOARD || type == OPTIONAL_EXTRA) && button == rsNULL)
		process = false;
	if (type == JOYSTICK && button == 0)
		process = false;
	if (type == MOUSE && button == 0)
		process = false;

	CControllerState *state;

	switch (type)
	{
	case KEYBOARD:
	case OPTIONAL_EXTRA:
		state = &CPad::GetPad(PAD1)->PCTempKeyState;
		break;
	case MOUSE:
		state = &CPad::GetPad(PAD1)->PCTempMouseState;
		break;
	case JOYSTICK:
		state = &CPad::GetPad(PAD1)->PCTempJoyState;
		break;
	
	}

	if (process)
	{
		CPad *pad = CPad::GetPad(PAD1);
		
		if (pad != NULL)
		{
			if (FrontEndMenuManager.GetIsMenuActive())
				AffectControllerStateOn_ButtonUp_All_Player_States(button, type, *state);
		}
	}
}

void CControllerConfigManager::AffectControllerStateOn_ButtonUp_All_Player_States(int32 button, eControllerType type, CControllerState &state)
{
	if (button == GetControllerKeyAssociatedWithAction(NETWORK_TALK, type))
		state.NetworkTalk = 0;
}

void CControllerConfigManager::AffectPadFromKeyBoard()
{
	RsKeyCodes kc;
	_InputTranslateShiftKeyUpDown(&kc);

	bool processdown = false;
	if (!CPad::m_bMapPadOneToPadTwo && !FrontEndMenuManager.GetIsMenuActive())
		processdown = true;
	
	for (int32 i = 0; i < MAX_CONTROLLERACTIONS; i++)
	{
		int32 key = GetControllerKeyAssociatedWithAction((e_ControllerAction)i, KEYBOARD);
		if (GetIsKeyboardKeyDown((RsKeyCodes)key) && processdown)
			AffectControllerStateOn_ButtonDown(key, KEYBOARD);

		int32 extrakey = GetControllerKeyAssociatedWithAction((e_ControllerAction)i, OPTIONAL_EXTRA);
		if (GetIsKeyboardKeyDown((RsKeyCodes)extrakey) && processdown)
			AffectControllerStateOn_ButtonDown(extrakey, OPTIONAL_EXTRA);

		if (!GetIsKeyboardKeyDown((RsKeyCodes)key))
			AffectControllerStateOn_ButtonUp(key, KEYBOARD);
		else if ( !GetIsKeyboardKeyDown((RsKeyCodes)extrakey))
			AffectControllerStateOn_ButtonUp(key, OPTIONAL_EXTRA);
	}
}

void CControllerConfigManager::AffectPadFromMouse()
{
	bool processdown = false;
	if (!CPad::m_bMapPadOneToPadTwo && !FrontEndMenuManager.GetIsMenuActive())
		processdown = true;
	
	for (int32 i = 0; i < MAX_CONTROLLERACTIONS; i++)
	{
		int32 button = GetControllerKeyAssociatedWithAction((e_ControllerAction)i, MOUSE);
		if (GetIsMouseButtonDown((RsKeyCodes)button) && processdown)
			AffectControllerStateOn_ButtonDown(button, MOUSE);
		if (GetIsMouseButtonUp((RsKeyCodes)button))
			AffectControllerStateOn_ButtonUp(button, MOUSE);
	}
}

void CControllerConfigManager::ClearSimButtonPressCheckers()
{
	for (int32 i = 0; i < MAX_SIMS; i++)
	{
		m_aSimCheckers[i][KEYBOARD]       = false;
		m_aSimCheckers[i][OPTIONAL_EXTRA] = false;
		m_aSimCheckers[i][MOUSE]          = false;
		m_aSimCheckers[i][JOYSTICK]       = false;
	}
}

bool CControllerConfigManager::GetIsKeyboardKeyDown(RsKeyCodes keycode)
{
	if (keycode < 255)
	{
		if (CPad::GetPad(PAD1)->GetChar(keycode))
			return true;
	}

	for (int32 i = 0; i < 12; i++)
	{
		if (i + rsF1 == keycode)
		{
			if (CPad::GetPad(PAD1)->GetF(i))
				return true;
		}
	}

	switch (keycode)
	{
	case rsESC:
		if (CPad::GetPad(PAD1)->GetEscape())
			return true;
		break;
	case rsINS:
		if (CPad::GetPad(PAD1)->GetInsert())
			return true;
		break;
	case rsDEL:
		if (CPad::GetPad(PAD1)->GetDelete())
			return true;
		break;
	case rsHOME:
		if (CPad::GetPad(PAD1)->GetHome())
			return true;
		break;
	case rsEND:
		if (CPad::GetPad(PAD1)->GetEnd())
			return true;
		break;
	case rsPGUP:
		if (CPad::GetPad(PAD1)->GetPageUp())
			return true;
		break;
	case rsPGDN:
		if (CPad::GetPad(PAD1)->GetPageDown())
			return true;
		break;
	case rsUP:
		if (CPad::GetPad(PAD1)->GetUp())
			return true;
		break;
	case rsDOWN:
		if (CPad::GetPad(PAD1)->GetDown())
			return true;
		break;
	case rsLEFT:
		if (CPad::GetPad(PAD1)->GetLeft())
			return true;
		break;
	case rsRIGHT:
		if (CPad::GetPad(PAD1)->GetRight())
			return true;
		break;
	case rsSCROLL:
		if (CPad::GetPad(PAD1)->GetScrollLock())
			return true;
		break;
	case rsPAUSE:
		if (CPad::GetPad(PAD1)->GetPause())
			return true;
		break;
	case rsNUMLOCK:
		if (CPad::GetPad(PAD1)->GetNumLock())
			return true;
		break;
	case rsDIVIDE:
		if (CPad::GetPad(PAD1)->GetDivide())
			return true;
		break;
	case rsTIMES:
		if (CPad::GetPad(PAD1)->GetTimes())
			return true;
		break;
	case rsMINUS:
		if (CPad::GetPad(PAD1)->GetMinus())
			return true;
		break;
	case rsPLUS:
		if (CPad::GetPad(PAD1)->GetPlus())
			return true;
		break;
	case rsPADENTER:
		if (CPad::GetPad(PAD1)->GetPadEnter())
			return true;
		break;
	case rsPADDEL:
		if (CPad::GetPad(PAD1)->GetPadDel())
			return true;
		break;
	case rsPADEND:
		if (CPad::GetPad(PAD1)->GetPad1())
			return true;
		break;
	case rsPADDOWN:
		if (CPad::GetPad(PAD1)->GetPad2())
			return true;
		break;
	case rsPADPGDN:
		if (CPad::GetPad(PAD1)->GetPad3())
			return true;
		break;
	case rsPADLEFT:
		if (CPad::GetPad(PAD1)->GetPad4())
			return true;
		break;
	case rsPAD5:
		if (CPad::GetPad(PAD1)->GetPad5())
			return true;
		break;
	case rsPADRIGHT:
		if (CPad::GetPad(PAD1)->GetPad6())
			return true;
		break;
	case rsPADHOME:
		if (CPad::GetPad(PAD1)->GetPad7())
			return true;
		break;
	case rsPADUP:
		if (CPad::GetPad(PAD1)->GetPad8())
			return true;
		break;
	case rsPADPGUP:
		if (CPad::GetPad(PAD1)->GetPad9())
			return true;
		break;
	case rsPADINS:
		if (CPad::GetPad(PAD1)->GetPad0())
			return true;
		break;
	case rsBACKSP:
		if (CPad::GetPad(PAD1)->GetBackspace())
			return true;
		break;
	case rsTAB:
		if (CPad::GetPad(PAD1)->GetTab())
			return true;
		break;
	case rsCAPSLK:
		if (CPad::GetPad(PAD1)->GetCapsLock())
			return true;
		break;
	case rsENTER:
		if (CPad::GetPad(PAD1)->GetEnter())
			return true;
		break;
	case rsLSHIFT:
		if (CPad::GetPad(PAD1)->GetLeftShift())
			return true;
		break;
	case rsSHIFT:
		if (CPad::GetPad(PAD1)->GetShift())
			return true;
		break;
	case rsRSHIFT:
		if (CPad::GetPad(PAD1)->GetRightShift())
			return true;
		break;
	case rsLCTRL:
		if (CPad::GetPad(PAD1)->GetLeftCtrl())
			return true;
		break;
	case rsRCTRL:
		if (CPad::GetPad(PAD1)->GetRightCtrl())
			return true;
		break;
	case rsLALT:
		if (CPad::GetPad(PAD1)->GetLeftAlt())
			return true;
		break;
	case rsRALT:
		if (CPad::GetPad(PAD1)->GetRightAlt())
			return true;
		break;
	case rsLWIN:
		if (CPad::GetPad(PAD1)->GetLeftWin())
			return true;
		break;
	case rsRWIN:
		if (CPad::GetPad(PAD1)->GetRightWin())
			return true;
		break;
	case rsAPPS:
		if (CPad::GetPad(PAD1)->GetApps())
			return true;
		break;
	}

	return false;
}

bool CControllerConfigManager::GetIsKeyboardKeyJustDown(RsKeyCodes keycode)
{
	if (keycode < 255)
	{
		if (CPad::GetPad(PAD1)->GetCharJustDown(keycode))
			return true;
	}

	for (int32 i = 0; i < 12; i++)
	{
		if (i + rsF1 == keycode)
		{
			if (CPad::GetPad(PAD1)->GetFJustDown(i))
				return true;
		}
	}

	switch (keycode)
	{
	case rsESC:
		if (CPad::GetPad(PAD1)->GetEscapeJustDown())
			return true;
		break;
	case rsINS:
		if (CPad::GetPad(PAD1)->GetInsertJustDown())
			return true;
		break;
	case rsDEL:
		if (CPad::GetPad(PAD1)->GetDeleteJustDown())
			return true;
		break;
	case rsHOME:
		if (CPad::GetPad(PAD1)->GetHomeJustDown())
			return true;
		break;
	case rsEND:
		if (CPad::GetPad(PAD1)->GetEndJustDown())
			return true;
		break;
	case rsPGUP:
		if (CPad::GetPad(PAD1)->GetPageUpJustDown())
			return true;
		break;
	case rsPGDN:
		if (CPad::GetPad(PAD1)->GetPageDownJustDown())
			return true;
		break;
	case rsUP:
		if (CPad::GetPad(PAD1)->GetUpJustDown())
			return true;
		break;
	case rsDOWN:
		if (CPad::GetPad(PAD1)->GetDownJustDown())
			return true;
		break;
	case rsLEFT:
		if (CPad::GetPad(PAD1)->GetLeftJustDown())
			return true;
		break;
	case rsRIGHT:
		if (CPad::GetPad(PAD1)->GetRightJustDown())
			return true;
		break;
	case rsSCROLL:
		if (CPad::GetPad(PAD1)->GetScrollLockJustDown())
			return true;
		break;
	case rsPAUSE:
		if (CPad::GetPad(PAD1)->GetPauseJustDown())
			return true;
		break;
	case rsNUMLOCK:
		if (CPad::GetPad(PAD1)->GetNumLockJustDown())
			return true;
		break;
	case rsDIVIDE:
		if (CPad::GetPad(PAD1)->GetDivideJustDown())
			return true;
		break;
	case rsTIMES:
		if (CPad::GetPad(PAD1)->GetTimesJustDown())
			return true;
		break;
	case rsMINUS:
		if (CPad::GetPad(PAD1)->GetMinusJustDown())
			return true;
		break;
	case rsPLUS:
		if (CPad::GetPad(PAD1)->GetPlusJustDown())
			return true;
		break;
	case rsPADENTER:
		if (CPad::GetPad(PAD1)->GetPadEnterJustDown())
			return true;
		break;
	case rsPADDEL:
		if (CPad::GetPad(PAD1)->GetPadDelJustDown())
			return true;
		break;
	case rsPADEND:
		if (CPad::GetPad(PAD1)->GetPad1JustDown())
			return true;
		break;
	case rsPADDOWN:
		if (CPad::GetPad(PAD1)->GetPad2JustDown())
			return true;
		break;
	case rsPADPGDN:
		if (CPad::GetPad(PAD1)->GetPad3JustDown())
			return true;
		break;
	case rsPADLEFT:
		if (CPad::GetPad(PAD1)->GetPad4JustDown())
			return true;
		break;
	case rsPAD5:
		if (CPad::GetPad(PAD1)->GetPad5JustDown())
			return true;
		break;
	case rsPADRIGHT:
		if (CPad::GetPad(PAD1)->GetPad6JustDown())
			return true;
		break;
	case rsPADHOME:
		if (CPad::GetPad(PAD1)->GetPad7JustDown())
			return true;
		break;
	case rsPADUP:
		if (CPad::GetPad(PAD1)->GetPad8JustDown())
			return true;
		break;
	case rsPADPGUP:
		if (CPad::GetPad(PAD1)->GetPad9JustDown())
			return true;
		break;
	case rsPADINS:
		if (CPad::GetPad(PAD1)->GetPad0JustDown())
			return true;
		break;
	case rsBACKSP:
		if (CPad::GetPad(PAD1)->GetBackspaceJustDown())
			return true;
		break;
	case rsTAB:
		if (CPad::GetPad(PAD1)->GetTabJustDown())
			return true;
		break;
	case rsCAPSLK:
		if (CPad::GetPad(PAD1)->GetCapsLockJustDown())
			return true;
		break;
	case rsENTER:
		if (CPad::GetPad(PAD1)->GetReturnJustDown())
			return true;
		break;
	case rsLSHIFT:
		if (CPad::GetPad(PAD1)->GetLeftShiftJustDown())
			return true;
		break;
	case rsSHIFT:
		if (CPad::GetPad(PAD1)->GetShiftJustDown())
			return true;
		break;
	case rsRSHIFT:
		if (CPad::GetPad(PAD1)->GetRightShiftJustDown())
			return true;
		break;
	case rsLCTRL:
		if (CPad::GetPad(PAD1)->GetLeftCtrlJustDown())
			return true;
		break;
	case rsRCTRL:
		if (CPad::GetPad(PAD1)->GetRightCtrlJustDown())
			return true;
		break;
	case rsLALT:
		if (CPad::GetPad(PAD1)->GetLeftAltJustDown())
			return true;
		break;
	case rsRALT:
		if (CPad::GetPad(PAD1)->GetRightAltJustDown())
			return true;
		break;
	case rsLWIN:
		if (CPad::GetPad(PAD1)->GetLeftWinJustDown())
			return true;
		break;
	case rsRWIN:
		if (CPad::GetPad(PAD1)->GetRightWinJustDown())
			return true;
		break;
	case rsAPPS:
		if (CPad::GetPad(PAD1)->GetAppsJustDown())
			return true;
		break;
	}

	return false;
}

bool CControllerConfigManager::GetIsMouseButtonDown(RsKeyCodes keycode)
{
	switch (keycode)
	{
	case rsMOUSELEFTBUTTON:
		if (CPad::GetPad(PAD1)->GetLeftMouse())
			return true;
		break;
	case rsMOUSMIDDLEBUTTON:
		if (CPad::GetPad(PAD1)->GetMiddleMouse())
			return true;
		break;
	case rsMOUSERIGHTBUTTON:
		if (CPad::GetPad(PAD1)->GetRightMouse())
			return true;
		break;
	case rsMOUSEWHEELUPBUTTON:
		if (CPad::GetPad(PAD1)->GetMouseWheelUp())
			return true;
		break;
	case rsMOUSEWHEELDOWNBUTTON:
		if (CPad::GetPad(PAD1)->GetMouseWheelDown())
			return true;
		break;
	case rsMOUSEX1BUTTON:
		if (CPad::GetPad(PAD1)->GetMouseX1())
			return true;
		break;
	case rsMOUSEX2BUTTON:
		if (CPad::GetPad(PAD1)->GetMouseX2())
			return true;
		break;
	}

	return false;
}

bool CControllerConfigManager::GetIsMouseButtonUp(RsKeyCodes keycode)
{
	switch (keycode)
	{
	case rsMOUSELEFTBUTTON:
		if (CPad::GetPad(PAD1)->GetLeftMouseUp())
			return true;
		break;
	case rsMOUSMIDDLEBUTTON:
		if (CPad::GetPad(PAD1)->GetMiddleMouseUp())
			return true;
		break;
	case rsMOUSERIGHTBUTTON:
		if (CPad::GetPad(PAD1)->GetRightMouseUp())
			return true;
		break;
	case rsMOUSEWHEELUPBUTTON:
		if (CPad::GetPad(PAD1)->GetMouseWheelUpUp())
			return true;
		break;
	case rsMOUSEWHEELDOWNBUTTON:
		if (CPad::GetPad(PAD1)->GetMouseWheelDownUp())
			return true;
		break;
	case rsMOUSEX1BUTTON:
		if (CPad::GetPad(PAD1)->GetMouseX1Up())
			return true;
		break;
	case rsMOUSEX2BUTTON:
		if (CPad::GetPad(PAD1)->GetMouseX2Up())
			return true;
		break;
	}

	return false;
}

void CControllerConfigManager::DeleteMatchingCommonControls(e_ControllerAction action, int32 key, eControllerType type)
{
	if (!GetIsKeyBlank(key, type))
	{
		if (key == GetControllerKeyAssociatedWithAction(CAMERA_CHANGE_VIEW_ALL_SITUATIONS,type))
			ClearSettingsAssociatedWithAction(CAMERA_CHANGE_VIEW_ALL_SITUATIONS, type);
		if (key == GetControllerKeyAssociatedWithAction(PED_FIREWEAPON, type))
			ClearSettingsAssociatedWithAction(PED_FIREWEAPON, type);
		if (key == GetControllerKeyAssociatedWithAction(GO_LEFT, type))
			ClearSettingsAssociatedWithAction(GO_LEFT, type);
		if (key == GetControllerKeyAssociatedWithAction(GO_RIGHT, type))
			ClearSettingsAssociatedWithAction(GO_RIGHT, type);
		if (key == GetControllerKeyAssociatedWithAction(NETWORK_TALK, type))
			ClearSettingsAssociatedWithAction(NETWORK_TALK, type);
		if (key == GetControllerKeyAssociatedWithAction(SWITCH_DEBUG_CAM_ON, type))
			ClearSettingsAssociatedWithAction(SWITCH_DEBUG_CAM_ON, type);
		if (key == GetControllerKeyAssociatedWithAction(TOGGLE_DPAD, type))
			ClearSettingsAssociatedWithAction(TOGGLE_DPAD, type);
		if (key == GetControllerKeyAssociatedWithAction(TAKE_SCREEN_SHOT, type))
			ClearSettingsAssociatedWithAction(TAKE_SCREEN_SHOT, type);
		if (key == GetControllerKeyAssociatedWithAction(SHOW_MOUSE_POINTER_TOGGLE, type))
			ClearSettingsAssociatedWithAction(SHOW_MOUSE_POINTER_TOGGLE, type);
	}
}

void CControllerConfigManager::DeleteMatching3rdPersonControls(e_ControllerAction action, int32 key, eControllerType type)
{
	if (!GetIsKeyBlank(key, type))
	{
		if (key == GetControllerKeyAssociatedWithAction(PED_LOOKBEHIND, type))
			ClearSettingsAssociatedWithAction(PED_LOOKBEHIND, type);
		if (key == GetControllerKeyAssociatedWithAction(PED_CYCLE_WEAPON_LEFT, type))
			ClearSettingsAssociatedWithAction(PED_CYCLE_WEAPON_LEFT, type);
		if (key == GetControllerKeyAssociatedWithAction(PED_CYCLE_WEAPON_RIGHT, type))
			ClearSettingsAssociatedWithAction(PED_CYCLE_WEAPON_RIGHT, type);
		if (key == GetControllerKeyAssociatedWithAction(PED_JUMPING, type))
			ClearSettingsAssociatedWithAction(PED_JUMPING, type);
		if (key == GetControllerKeyAssociatedWithAction(PED_SPRINT, type))
			ClearSettingsAssociatedWithAction(PED_SPRINT, type);

		if (CMenuManager::m_ControlMethod == CONTROL_CLASSIC)
		{
			if (key == GetControllerKeyAssociatedWithAction(PED_CYCLE_TARGET_LEFT, type))
				ClearSettingsAssociatedWithAction(PED_CYCLE_TARGET_LEFT, type);
			if (key == GetControllerKeyAssociatedWithAction(PED_CYCLE_TARGET_RIGHT, type))
				ClearSettingsAssociatedWithAction(PED_CYCLE_TARGET_RIGHT, type);
			if (key == GetControllerKeyAssociatedWithAction(PED_CENTER_CAMERA_BEHIND_PLAYER, type))
				ClearSettingsAssociatedWithAction(PED_CENTER_CAMERA_BEHIND_PLAYER, type);
		}
	}
}

void CControllerConfigManager::DeleteMatching1rst3rdPersonControls(e_ControllerAction action, int32 key, eControllerType type)
{
	if (!GetIsKeyBlank(key, type))
	{
		if (key == GetControllerKeyAssociatedWithAction(PED_LOCK_TARGET, type))
			ClearSettingsAssociatedWithAction(PED_LOCK_TARGET, type);
		if (key == GetControllerKeyAssociatedWithAction(GO_FORWARD, type))
			ClearSettingsAssociatedWithAction(GO_FORWARD, type);
		if (key == GetControllerKeyAssociatedWithAction(GO_BACK, type))
			ClearSettingsAssociatedWithAction(GO_BACK, type);

		if (CMenuManager::m_ControlMethod == CONTROL_CLASSIC)
		{
			if (key == GetControllerKeyAssociatedWithAction(PED_1RST_PERSON_LOOK_LEFT, type))
				ClearSettingsAssociatedWithAction(PED_1RST_PERSON_LOOK_LEFT, type);
			if (key == GetControllerKeyAssociatedWithAction(PED_1RST_PERSON_LOOK_RIGHT, type))
				ClearSettingsAssociatedWithAction(PED_1RST_PERSON_LOOK_RIGHT, type);
			if (key == GetControllerKeyAssociatedWithAction(PED_1RST_PERSON_LOOK_DOWN, type))
				ClearSettingsAssociatedWithAction(PED_1RST_PERSON_LOOK_DOWN, type);
			if (key == GetControllerKeyAssociatedWithAction(PED_1RST_PERSON_LOOK_UP, type))
				ClearSettingsAssociatedWithAction(PED_1RST_PERSON_LOOK_UP, type);
		}
	}
}

void CControllerConfigManager::DeleteMatchingVehicleControls(e_ControllerAction action, int32 key, eControllerType type)
{
	if (!GetIsKeyBlank(key, type))
	{
		if (key == GetControllerKeyAssociatedWithAction(VEHICLE_LOOKBEHIND, type))
			ClearSettingsAssociatedWithAction(VEHICLE_LOOKBEHIND, type);
		if (key == GetControllerKeyAssociatedWithAction(VEHICLE_LOOKLEFT, type))
			ClearSettingsAssociatedWithAction(VEHICLE_LOOKLEFT, type);
		if (key == GetControllerKeyAssociatedWithAction(VEHICLE_LOOKRIGHT, type))
			ClearSettingsAssociatedWithAction(VEHICLE_LOOKRIGHT, type);
		if (key == GetControllerKeyAssociatedWithAction(VEHICLE_LOOKBEHIND, type)) // note: dublicate
			ClearSettingsAssociatedWithAction(VEHICLE_LOOKBEHIND, type);
		if (key == GetControllerKeyAssociatedWithAction(VEHICLE_HORN, type))
			ClearSettingsAssociatedWithAction(VEHICLE_HORN, type);
		if (key == GetControllerKeyAssociatedWithAction(VEHICLE_HANDBRAKE, type))
			ClearSettingsAssociatedWithAction(VEHICLE_HANDBRAKE, type);
		if (key == GetControllerKeyAssociatedWithAction(VEHICLE_ACCELERATE, type))
			ClearSettingsAssociatedWithAction(VEHICLE_ACCELERATE, type);
		if (key == GetControllerKeyAssociatedWithAction(VEHICLE_BRAKE, type))
			ClearSettingsAssociatedWithAction(VEHICLE_BRAKE, type);
		if (key == GetControllerKeyAssociatedWithAction(VEHICLE_CHANGE_RADIO_STATION, type))
			ClearSettingsAssociatedWithAction(VEHICLE_CHANGE_RADIO_STATION, type);
		if (key == GetControllerKeyAssociatedWithAction(TOGGLE_SUBMISSIONS, type))
			ClearSettingsAssociatedWithAction(TOGGLE_SUBMISSIONS, type);
		if (key == GetControllerKeyAssociatedWithAction(VEHICLE_TURRETLEFT, type))
			ClearSettingsAssociatedWithAction(VEHICLE_TURRETLEFT, type);
		if (key == GetControllerKeyAssociatedWithAction(VEHICLE_TURRETRIGHT, type))
			ClearSettingsAssociatedWithAction(VEHICLE_TURRETRIGHT, type);
		if (key == GetControllerKeyAssociatedWithAction(VEHICLE_TURRETUP, type))
			ClearSettingsAssociatedWithAction(VEHICLE_TURRETUP, type);
		if (key == GetControllerKeyAssociatedWithAction(VEHICLE_TURRETDOWN, type))
			ClearSettingsAssociatedWithAction(VEHICLE_TURRETDOWN, type);
	}
}

void CControllerConfigManager::DeleteMatchingVehicle_3rdPersonControls(e_ControllerAction action, int32 key, eControllerType type)
{
	if (!GetIsKeyBlank(key, type))
	{
		if (key == GetControllerKeyAssociatedWithAction(VEHICLE_ENTER_EXIT, type))
			ClearSettingsAssociatedWithAction(VEHICLE_ENTER_EXIT, type);
	}
}

void CControllerConfigManager::DeleteMatching1rstPersonControls(e_ControllerAction action, int32 key, eControllerType type)
{
	if (!GetIsKeyBlank(key, type))
	{
		if (key == GetControllerKeyAssociatedWithAction(PED_SNIPER_ZOOM_IN, type))
			ClearSettingsAssociatedWithAction(PED_SNIPER_ZOOM_IN, type);
		if (key == GetControllerKeyAssociatedWithAction(PED_SNIPER_ZOOM_OUT, type))
			ClearSettingsAssociatedWithAction(PED_SNIPER_ZOOM_OUT, type);
	}
}

void CControllerConfigManager::DeleteMatchingActionInitiators(e_ControllerAction action, int32 key, eControllerType type)
{
	if (!GetIsKeyBlank(key, type))
	{
		switch (GetActionType(action))
		{
		case ACTIONTYPE_1RSTPERSON:
			DeleteMatchingCommonControls           (action, key, type);
			DeleteMatching1rstPersonControls       (action, key, type);
			DeleteMatching1rst3rdPersonControls    (action, key, type);
			break;
		case ACTIONTYPE_3RDPERSON:
			DeleteMatching3rdPersonControls        (action, key, type);
			DeleteMatchingCommonControls           (action, key, type);
			DeleteMatchingVehicle_3rdPersonControls(action, key, type);
			DeleteMatching1rst3rdPersonControls    (action, key, type);
			break;
		case ACTIONTYPE_VEHICLE:
			DeleteMatchingVehicleControls          (action, key, type);
			DeleteMatchingCommonControls           (action, key, type);
			DeleteMatchingVehicle_3rdPersonControls(action, key, type);
			break;
		case ACTIONTYPE_VEHICLE_3RDPERSON:
			DeleteMatching3rdPersonControls        (action, key, type);
			DeleteMatchingVehicleControls          (action, key, type);
			DeleteMatchingCommonControls           (action, key, type);
			DeleteMatching1rst3rdPersonControls    (action, key, type);
			break;
		case ACTIONTYPE_1RST3RDPERSON:
			DeleteMatching1rstPersonControls       (action, key, type);
			DeleteMatching3rdPersonControls        (action, key, type);
			DeleteMatchingCommonControls           (action, key, type);
			DeleteMatchingVehicle_3rdPersonControls(action, key, type);
			DeleteMatching1rst3rdPersonControls    (action, key, type);
			break;
		case ACTIONTYPE_COMMON:
			DeleteMatching1rstPersonControls       (action, key, type);
			DeleteMatching3rdPersonControls        (action, key, type);
			DeleteMatchingVehicleControls          (action, key, type);
			DeleteMatchingVehicle_3rdPersonControls(action, key, type);
			DeleteMatchingCommonControls           (action, key, type);
			DeleteMatching1rst3rdPersonControls    (action, key, type);
			break;
		}
	}
}

bool CControllerConfigManager::GetIsKeyBlank(int32 key, eControllerType type)
{
	switch (type)
	{
	case KEYBOARD:
	case OPTIONAL_EXTRA:
		if (key != rsNULL)
			return false;
		break;

	case JOYSTICK:
		if (key != 0)
			return false;
		break;

	case MOUSE:
		if (key != 0)
			return false;
		break;
	}

	return true;
}

e_ControllerActionType CControllerConfigManager::GetActionType(e_ControllerAction action)
{
	switch (action)
	{
	case CAMERA_CHANGE_VIEW_ALL_SITUATIONS:
	case PED_FIREWEAPON:
	case GO_LEFT:
	case GO_RIGHT:
	case NETWORK_TALK:
	case SWITCH_DEBUG_CAM_ON:
	case TOGGLE_DPAD:
	case TAKE_SCREEN_SHOT:
	case SHOW_MOUSE_POINTER_TOGGLE:
		return ACTIONTYPE_COMMON;
		break;

	case PED_LOOKBEHIND:
	case PED_CYCLE_WEAPON_LEFT:
	case PED_CYCLE_WEAPON_RIGHT:
	case PED_JUMPING:
	case PED_SPRINT:
	case PED_CYCLE_TARGET_LEFT:
	case PED_CYCLE_TARGET_RIGHT:
	case PED_CENTER_CAMERA_BEHIND_PLAYER:
		return ACTIONTYPE_3RDPERSON;
		break;

	case VEHICLE_LOOKBEHIND:
	case VEHICLE_LOOKLEFT:
	case VEHICLE_LOOKRIGHT:
	case VEHICLE_HORN:
	case VEHICLE_HANDBRAKE:
	case VEHICLE_ACCELERATE:
	case VEHICLE_BRAKE:
	case VEHICLE_CHANGE_RADIO_STATION:
	case TOGGLE_SUBMISSIONS:
	case VEHICLE_TURRETLEFT:
	case VEHICLE_TURRETRIGHT:
	case VEHICLE_TURRETUP:
	case VEHICLE_TURRETDOWN:
		return ACTIONTYPE_VEHICLE;
		break;

	case VEHICLE_ENTER_EXIT:
		return ACTIONTYPE_VEHICLE_3RDPERSON;
		break;

	case PED_LOCK_TARGET:
	case GO_FORWARD:
	case GO_BACK:
	case PED_1RST_PERSON_LOOK_LEFT:
	case PED_1RST_PERSON_LOOK_RIGHT:
	case PED_1RST_PERSON_LOOK_DOWN:
	case PED_1RST_PERSON_LOOK_UP:
		return ACTIONTYPE_1RST3RDPERSON;
		break;

	case PED_SNIPER_ZOOM_IN:
	case PED_SNIPER_ZOOM_OUT:
		return ACTIONTYPE_1RSTPERSON;
		break;
	}

	return ACTIONTYPE_NONE;
}

void CControllerConfigManager::ClearSettingsAssociatedWithAction(e_ControllerAction action, eControllerType type)
{
	switch (type)
	{
	case KEYBOARD:
		m_aSettings[action][type].m_Key = rsNULL;
		m_aSettings[action][type].m_ContSetOrder = SETORDER_NONE;
		break;
	case OPTIONAL_EXTRA:
		m_aSettings[action][type].m_Key = rsNULL;
		m_aSettings[action][type].m_ContSetOrder = SETORDER_NONE;
		break;
	case MOUSE:
		m_aSettings[action][type].m_Key = 0;
		m_aSettings[action][type].m_ContSetOrder = SETORDER_NONE;
		break;
	case JOYSTICK:
		m_aSettings[action][type].m_Key = 0;
		m_aSettings[action][type].m_ContSetOrder = SETORDER_NONE;
		break;
	}

	ResetSettingOrder(action);
}

wchar *CControllerConfigManager::GetControllerSettingTextWithOrderNumber(e_ControllerAction action, eContSetOrder setorder)
{
	for (int i = 0; i < MAX_CONTROLLERTYPES; i++)
	{
		if (m_aSettings[action][i].m_ContSetOrder == setorder)
		{
			switch (i)
			{
			case KEYBOARD:
			case OPTIONAL_EXTRA:
				return GetControllerSettingTextKeyBoard(action, (eControllerType)i);
			case MOUSE:
				return GetControllerSettingTextMouse   (action);
			case JOYSTICK:
				return GetControllerSettingTextJoystick(action);
			}
		}
	}
	
	return NULL;
}

wchar *CControllerConfigManager::GetControllerSettingTextKeyBoard(e_ControllerAction action, eControllerType type)
{
	static wchar ActionText[50];
	static wchar NewStringWithNumber[30];

	for (int32 i = 0; i < ARRAYSIZE(ActionText); i++)
		ActionText[i] = '\0';

	if (GetControllerKeyAssociatedWithAction(action, type) != rsNULL)
	{
		if (   GetControllerKeyAssociatedWithAction(action, type) >= 0
			&& GetControllerKeyAssociatedWithAction(action, type) <= 255)
		{
			char c = GetControllerKeyAssociatedWithAction(action, type);
			if (c == ' ')
				return TheText.Get("FEC_SPC"); // "SPC"
			else
			{
				ActionText[0] = CFont::character_code(c);
				if (ActionText[0] == '\0')
					ActionText[0] = CFont::character_code('#');
				ActionText[1] = '\0';
				return ActionText;
			}
		}
		else
		{
			switch (GetControllerKeyAssociatedWithAction(action, type))
			{
				case rsF1:
				case rsF2:
				case rsF3:
				case rsF4:
				case rsF5:
				case rsF6:
				case rsF7:
				case rsF8:
				case rsF9:
				case rsF10:
				case rsF11:
				case rsF12:
				{
					CMessages::InsertNumberInString(TheText.Get("FEC_FNC"), // "F~1~"
						GetControllerKeyAssociatedWithAction(action, type) - rsESC,
						-1, -1, -1, -1, -1,
						NewStringWithNumber);
					return NewStringWithNumber;
					break;
				}

				case rsINS:
				{
					return TheText.Get("FEC_IRT"); // "INS"
					break;
				}

				case rsDEL:
				{
					return TheText.Get("FEC_DLL"); // "DEL"
					break;
				}

				case rsHOME:
				{
					return TheText.Get("FEC_HME"); // "HOME"
					break;
				}

				case rsEND:
				{
					return TheText.Get("FEC_END"); // "END"
					break;
				}

				case rsPGUP:
				{
					return TheText.Get("FEC_PGU"); // "PGUP"
					break;
				}

				case rsPGDN:
				{
					return TheText.Get("FEC_PGD"); // "PGDN"
					break;
				}

				case rsUP:
				{
					return TheText.Get("FEC_UPA"); // "UP"
					break;
				}

				case rsDOWN:
				{
					return TheText.Get("FEC_DWA"); // "DOWN"
					break;
				}

				case rsLEFT:
				{
					return TheText.Get("FEC_LFA"); // "LEFT"
					break;
				}

				case rsRIGHT:
				{
					return TheText.Get("FEC_RFA"); // "RIGHT"
					break;
				}

				case rsDIVIDE:
				{
					return TheText.Get("FEC_FWS"); // "NUM /"
					break;
				}

				case rsTIMES:
				{
					return TheText.Get("FEC_STR"); // "NUM STAR"
					break;
				}

				case rsPLUS:
				{
					return TheText.Get("FEC_PLS"); // "NUM +"
					break;
				}

				case rsMINUS:
				{
					return TheText.Get("FEC_MIN"); // "NUM -"
					break;
				}

				case rsPADDEL:
				{
					return TheText.Get("FEC_DOT"); // "NUM ."
					break;
				}

				case rsPADEND:
				{
					CMessages::InsertNumberInString(TheText.Get("FEC_NMN"), // "NUM~1~"
						1, -1, -1, -1, -1, -1, NewStringWithNumber);
					return NewStringWithNumber;
					break;
				}

				case rsPADDOWN:
				{
					CMessages::InsertNumberInString(TheText.Get("FEC_NMN"), // "NUM~1~"
						2, -1, -1, -1, -1, -1,
						NewStringWithNumber);
					return NewStringWithNumber;
					break;
				}

				case rsPADPGDN:
				{
					CMessages::InsertNumberInString(TheText.Get("FEC_NMN"), // "NUM~1~"
						3, -1, -1, -1, -1, -1,
						NewStringWithNumber);
					return NewStringWithNumber;
					break;
				}

				case rsPADLEFT:
				{
					CMessages::InsertNumberInString(TheText.Get("FEC_NMN"), // "NUM~1~"
						4, -1, -1, -1, -1, -1,
						NewStringWithNumber);
					return NewStringWithNumber;
					break;
				}

				case rsPAD5:
				{
					CMessages::InsertNumberInString(TheText.Get("FEC_NMN"), // "NUM~1~"
						5, -1, -1, -1, -1, -1,
						NewStringWithNumber);
					return NewStringWithNumber;
					break;
				}

				case rsNUMLOCK:
				{
					return TheText.Get("FEC_NLK"); // "NUMLOCK"
					break;
				}

				case rsPADRIGHT:
				{
					CMessages::InsertNumberInString(TheText.Get("FEC_NMN"), // "NUM~1~"
						6, -1, -1, -1, -1, -1,
						NewStringWithNumber);
					return NewStringWithNumber;
					break;
				}

				case rsPADHOME:
				{
					CMessages::InsertNumberInString(TheText.Get("FEC_NMN"), // "NUM~1~"
						7, -1, -1, -1, -1, -1,
						NewStringWithNumber);
					return NewStringWithNumber;
					break;
				}

				case rsPADUP:
				{
					CMessages::InsertNumberInString(TheText.Get("FEC_NMN"), // "NUM~1~"
						8, -1, -1, -1, -1, -1,
						NewStringWithNumber);
					return NewStringWithNumber;
					break;
				}

				case rsPADPGUP:
				{
					CMessages::InsertNumberInString(TheText.Get("FEC_NMN"), // "NUM~1~"
						9, -1, -1, -1, -1, -1,
						NewStringWithNumber);
					return NewStringWithNumber;
					break;
				}

				case rsPADINS:
				{
					CMessages::InsertNumberInString(TheText.Get("FEC_NMN"), // "NUM~1~"
						0, -1, -1, -1, -1, -1,
						NewStringWithNumber);
					return NewStringWithNumber;
					break;
				}

				case rsPADENTER:
				{
					return TheText.Get("FEC_ETR"); // "ENT"
					break;
				}

				case rsSCROLL:
				{
					return TheText.Get("FEC_SLK"); // "SCROLL LOCK"
					break;
				}

				case rsPAUSE:
				{
					return TheText.Get("FEC_PSB"); // "BREAK"
					break;
				}

				case rsBACKSP:
				{
					return TheText.Get("FEC_BSP"); // "BSPACE"
					break;
				}

				case rsTAB:
				{
					return TheText.Get("FEC_TAB"); // "TAB"
					break;
				}

				case rsCAPSLK:
				{
					return TheText.Get("FEC_CLK"); // "CAPSLOCK"
					break;
				}

				case rsENTER:
				{
					return TheText.Get("FEC_RTN"); // "RET"
					break;
				}

				case rsLSHIFT:
				{
					return TheText.Get("FEC_LSF"); // "LSHIFT"
					break;
				}

				case rsRSHIFT:
				{
					return TheText.Get("FEC_RSF"); // "RSHIFT"
					break;
				}

				case rsLCTRL:
				{
					return TheText.Get("FEC_LCT"); // "LCTRL"
					break;
				}

				case rsRCTRL:
				{
					return TheText.Get("FEC_RCT"); // "RCTRL"
					break;
				}

				case rsLALT:
				{
					return TheText.Get("FEC_LAL"); // "LALT"
					break;
				}

				case rsRALT:
				{
					return TheText.Get("FEC_RAL"); // "RALT"
					break;
				}

				case rsLWIN:
				{
					return TheText.Get("FEC_LWD"); // "LWIN"
					break;
				}

				case rsRWIN:
				{
					return TheText.Get("FEC_RWD"); // "RWIN"
					break;
				}

				case rsAPPS:
				{
					return TheText.Get("FEC_WRC"); // "WINCLICK"
					break;
				}

				case rsSHIFT:
				{
					return TheText.Get("FEC_SFT"); // "SHIFT"
					break;
				}
			}
		}
	}

	return NULL;
}

wchar *CControllerConfigManager::GetControllerSettingTextMouse(e_ControllerAction action)
{
	switch (m_aSettings[action][MOUSE].m_Key)
	{
	case 1:
		return TheText.Get("FEC_MSL");	// LMB
		break;
	case 2:
		return TheText.Get("FEC_MSM");	// MMB
		break;
	case 3:
		return TheText.Get("FEC_MSR");	// RMB
		break;
	case 4:
		return TheText.Get("FEC_MWF");	// WHEEL UP
		break;
	case 5:
		return TheText.Get("FEC_MWB");	// WHEEL DN
		break;
	case 6:
		return TheText.Get("FEC_MXO");	// MXB1
		break;
	case 7:
		return TheText.Get("FEC_MXT");	// MXB2
		break;
	}

	return NULL;
}

wchar *CControllerConfigManager::GetControllerSettingTextJoystick(e_ControllerAction action)
{
	if (m_aSettings[action][JOYSTICK].m_Key == 0) 
		return NULL;

	static wchar NewStringWithNumber[30];

	CMessages::InsertNumberInString(TheText.Get("FEC_JBO"),		// JOY ~1~
		m_aSettings[action][JOYSTICK].m_Key, -1, -1, -1, -1, -1,
		NewStringWithNumber);

	return NewStringWithNumber;
}

int32 CControllerConfigManager::GetNumOfSettingsForAction(e_ControllerAction action)
{
	int32 num = 0;

	if (m_aSettings[action][KEYBOARD].m_Key       != rsNULL) num++;
	if (m_aSettings[action][OPTIONAL_EXTRA].m_Key != rsNULL) num++;
	if (m_aSettings[action][MOUSE].m_Key          != 0)      num++;
	if (m_aSettings[action][JOYSTICK].m_Key       != 0)      num++;

	return num;
}

void CControllerConfigManager::GetWideStringOfCommandKeys(uint16 action, wchar *text, uint16 leight)
{
	int32 nums = GetNumOfSettingsForAction((e_ControllerAction)action);

	int32 sets = 0;

	for (int32 i = SETORDER_1; i < MAX_SETORDERS; i++)
	{
		wchar *textorder = ControlsManager.GetControllerSettingTextWithOrderNumber((e_ControllerAction)action, (eContSetOrder)i);
		if (textorder != NULL)
		{
			uint16 len = CMessages::GetWideStringLength(text);
			CMessages::WideStringCopy(&text[len], textorder, leight - len);

			if (++sets < nums)
			{
				if (sets == nums - 1)
				{
					// if last - text += ' or ';
					uint16 pos1 = CMessages::GetWideStringLength(text);
					text[pos1] = ' ';

					CMessages::WideStringCopy(&text[pos1 + 1],
						TheText.Get("FEC_ORR"),	// "or"
						leight - (pos1 + 1));
					
					uint16 pos2 = CMessages::GetWideStringLength(text);
					text[pos2 + 0] = ' ';
					text[pos2 + 1] = '\0';
				}
				else
				{
					// text += ', ';
					uint16 pos1 = CMessages::GetWideStringLength(text);
					text[pos1 + 0] = ',';
					text[pos1 + 1] = ' ';
					text[pos1 + 2] = '\0';

					uint16 pos2 = CMessages::GetWideStringLength(text);
				}
			}
		}
	}
}

int32 CControllerConfigManager::GetControllerKeyAssociatedWithAction(e_ControllerAction action, eControllerType type)
{
	return m_aSettings[action][type].m_Key;
}

void CControllerConfigManager::UpdateJoyButtonState(int32 padnumber)
{
	for (int32 i = 0; i < MAX_BUTTONS; i++)
		m_aButtonStates[i] = false;

#ifdef __DINPUT_INCLUDED__
	for (int32 i = 0; i < MAX_BUTTONS; i++)
	{
		if (m_NewState.rgbButtons[i] & 0x80)
			m_aButtonStates[i] = true;
		else
			m_aButtonStates[i] = false;
	}
#endif
}

bool CControllerConfigManager::GetIsActionAButtonCombo(e_ControllerAction action)
{
	switch (action)
	{
	case VEHICLE_LOOKBEHIND:
	case PED_CYCLE_TARGET_LEFT:
	case PED_CYCLE_TARGET_RIGHT:
		return true;
		break;
	}

	return false;
}

wchar *CControllerConfigManager::GetButtonComboText(e_ControllerAction action)
{
	switch (action)
	{
	case PED_CYCLE_TARGET_LEFT:
		return TheText.Get("FEC_PTL");	// Use LockTarget with Weapon Switch Left.
		break;
	
	case PED_CYCLE_TARGET_RIGHT:
		return TheText.Get("FEC_PTR");	// Use LockTarget with Weapon Switch Right.
		break;
	
	case VEHICLE_LOOKBEHIND:
		return TheText.Get("FEC_LBC");	// Use Look Left With Look Right. 
		break;
	}

	return NULL;
}

void CControllerConfigManager::SetControllerKeyAssociatedWithAction(e_ControllerAction action, int32 key, eControllerType type)
{
	ResetSettingOrder(action);
	
	m_aSettings[action][type].m_Key = key;
	m_aSettings[action][type].m_ContSetOrder = GetNumOfSettingsForAction(action) + 1;
}

int32 CControllerConfigManager::GetMouseButtonAssociatedWithAction(e_ControllerAction action)
{
	return m_aSettings[action][MOUSE].m_Key;
}

void CControllerConfigManager::SetMouseButtonAssociatedWithAction(e_ControllerAction action, int32 button)
{
	m_aSettings[action][MOUSE].m_Key = button;
	m_aSettings[action][MOUSE].m_ContSetOrder = GetNumOfSettingsForAction(action) + 1;
}

void CControllerConfigManager::ResetSettingOrder(e_ControllerAction action)
{
	int32 conttype = KEYBOARD;

	for (int32 i = SETORDER_1; i < MAX_SETORDERS; i++)
	{
		bool isexist = false;
		for (int32 j = 0; j < MAX_CONTROLLERTYPES; j++)
		{
			if (m_aSettings[action][j].m_ContSetOrder == i)
				isexist = true;
		}

		bool init = false;

		if (!isexist)
		{
			for (int32 k = 0; k < MAX_CONTROLLERTYPES; k++)
			{
				int32 setorder = m_aSettings[action][k].m_ContSetOrder;
				if (setorder > i && setorder != KEYBOARD)
				{
					if (init)
					{
						if (setorder < m_aSettings[action][conttype].m_ContSetOrder)
							conttype = k;
					}
					else
					{
						init = true;
						conttype = k;
					}
				}
			}

			if (init)
				m_aSettings[action][conttype].m_ContSetOrder = i;
		}
	}
}


STARTPATCHES
	InjectHook(0x58B7A0, &CControllerConfigManager::MakeControllerActionsBlank, PATCH_JUMP);
	InjectHook(0x58B7D0, &CControllerConfigManager::GetJoyButtonJustDown, PATCH_JUMP);
	InjectHook(0x58B800, &CControllerConfigManager::SaveSettings, PATCH_JUMP);
	InjectHook(0x58B870, &CControllerConfigManager::LoadSettings, PATCH_JUMP);
	InjectHook(0x58B930, &CControllerConfigManager::InitDefaultControlConfiguration, PATCH_JUMP);
	InjectHook(0x58BD00, &CControllerConfigManager::InitDefaultControlConfigMouse, PATCH_JUMP);
	InjectHook(0x58BD90, &CControllerConfigManager::InitDefaultControlConfigJoyPad, PATCH_JUMP);
	InjectHook(0x58C060, &CControllerConfigManager::InitialiseControllerActionNameArray, PATCH_JUMP);
	InjectHook(0x58C5E0, &CControllerConfigManager::UpdateJoyInConfigMenus_ButtonDown, PATCH_JUMP);
	InjectHook(0x58C730, &CControllerConfigManager::AffectControllerStateOn_ButtonDown, PATCH_JUMP);
	InjectHook(0x58C880, &CControllerConfigManager::AffectControllerStateOn_ButtonDown_Driving, PATCH_JUMP);
	InjectHook(0x58CAD0, &CControllerConfigManager::AffectControllerStateOn_ButtonDown_FirstPersonOnly, PATCH_JUMP);
	InjectHook(0x58CB10, &CControllerConfigManager::AffectControllerStateOn_ButtonDown_ThirdPersonOnly, PATCH_JUMP);
	InjectHook(0x58CBD0, &CControllerConfigManager::AffectControllerStateOn_ButtonDown_FirstAndThirdPersonOnly, PATCH_JUMP);
	InjectHook(0x58CD70, &CControllerConfigManager::AffectControllerStateOn_ButtonDown_AllStates, PATCH_JUMP);
	InjectHook(0x58CE50, &CControllerConfigManager::AffectControllerStateOn_ButtonDown_VehicleAndThirdPersonOnly, PATCH_JUMP);
	InjectHook(0x58CE80, &CControllerConfigManager::UpdateJoyInConfigMenus_ButtonUp, PATCH_JUMP);
	InjectHook(0x58CFD0, &CControllerConfigManager::AffectControllerStateOn_ButtonUp, PATCH_JUMP);
	InjectHook(0x58D090, &CControllerConfigManager::AffectControllerStateOn_ButtonUp_All_Player_States, PATCH_JUMP);
	InjectHook(0x58D0C0, &CControllerConfigManager::AffectPadFromKeyBoard, PATCH_JUMP);
	InjectHook(0x58D1A0, &CControllerConfigManager::AffectPadFromMouse, PATCH_JUMP);
	InjectHook(0x58D220, &CControllerConfigManager::ClearSimButtonPressCheckers, PATCH_JUMP);
	InjectHook(0x58D2A0, &CControllerConfigManager::GetIsKeyboardKeyDown, PATCH_JUMP);
	InjectHook(0x58D8A0, &CControllerConfigManager::GetIsKeyboardKeyJustDown, PATCH_JUMP);
	InjectHook(0x58E280, &CControllerConfigManager::GetIsMouseButtonDown, PATCH_JUMP);
	InjectHook(0x58E360, &CControllerConfigManager::GetIsMouseButtonUp, PATCH_JUMP);
	InjectHook(0x58E440, &CControllerConfigManager::DeleteMatchingCommonControls, PATCH_JUMP);
	InjectHook(0x58E540, &CControllerConfigManager::DeleteMatching3rdPersonControls, PATCH_JUMP);
	InjectHook(0x58E630, &CControllerConfigManager::DeleteMatching1rst3rdPersonControls, PATCH_JUMP);
	InjectHook(0x58E710, &CControllerConfigManager::DeleteMatchingVehicleControls, PATCH_JUMP);
	InjectHook(0x58E890, &CControllerConfigManager::DeleteMatchingVehicle_3rdPersonControls, PATCH_JUMP);
	InjectHook(0x58E8D0, &CControllerConfigManager::DeleteMatching1rstPersonControls, PATCH_JUMP);
	InjectHook(0x58E920, &CControllerConfigManager::DeleteMatchingActionInitiators, PATCH_JUMP);
	InjectHook(0x58EA70, &CControllerConfigManager::GetIsKeyBlank, PATCH_JUMP);
	InjectHook(0x58EAD0, &CControllerConfigManager::GetActionType, PATCH_JUMP);
	InjectHook(0x58EB40, &CControllerConfigManager::ClearSettingsAssociatedWithAction, PATCH_JUMP);
	InjectHook(0x58EBF0, &CControllerConfigManager::GetControllerSettingTextWithOrderNumber, PATCH_JUMP);
	InjectHook(0x58EC50, &CControllerConfigManager::GetControllerSettingTextKeyBoard, PATCH_JUMP);
	InjectHook(0x58F320, &CControllerConfigManager::GetControllerSettingTextMouse, PATCH_JUMP);
	InjectHook(0x58F3D0, &CControllerConfigManager::GetControllerSettingTextJoystick, PATCH_JUMP);
	InjectHook(0x58F420, &CControllerConfigManager::GetNumOfSettingsForAction, PATCH_JUMP);
	InjectHook(0x58F460, &CControllerConfigManager::GetWideStringOfCommandKeys, PATCH_JUMP);
	InjectHook(0x58F590, &CControllerConfigManager::GetControllerKeyAssociatedWithAction, PATCH_JUMP);
	InjectHook(0x58F5B0, &CControllerConfigManager::UpdateJoyButtonState, PATCH_JUMP);
	InjectHook(0x58F660, &CControllerConfigManager::GetIsActionAButtonCombo, PATCH_JUMP);
	InjectHook(0x58F690, &CControllerConfigManager::GetButtonComboText, PATCH_JUMP);
	InjectHook(0x58F700, &CControllerConfigManager::SetControllerKeyAssociatedWithAction, PATCH_JUMP);
	InjectHook(0x58F740, &CControllerConfigManager::GetMouseButtonAssociatedWithAction, PATCH_JUMP);
	InjectHook(0x58F760, &CControllerConfigManager::SetMouseButtonAssociatedWithAction, PATCH_JUMP);
	InjectHook(0x58F790, &CControllerConfigManager::ResetSettingOrder, PATCH_JUMP);
ENDPATCHES
