#include "common.h"
#include "Pad.h"
#include "ControllerConfig.h"
#include "Frontend.h"
#include "Camera.h"

#include "rwcore.h"
#include "skeleton.h"
#include "events.h"


/*
 *****************************************************************************
 */
static RsEventStatus 
HandleKeyDown(RsKeyStatus *keyStatus)
{
	CPad *pad0 = CPad::GetPad(0);
	CPad *pad1 = CPad::GetPad(1);
	
	RwInt32 c = keyStatus->keyCharCode;
	
	if ( c != rsNULL )
	{
		switch (c)
		{
			case rsESC:
				{
					CPad::TempKeyState.ESC = 255;
					break;
				}
			
			case rsINS:
				{
					CPad::TempKeyState.INS = 255;
					break;
				}
				
			case rsDEL:
				{
					CPad::TempKeyState.DEL = 255;
					break;
				}
				
			case rsHOME:
				{
					CPad::TempKeyState.HOME = 255;
					break;
				}
				
			case rsEND:
				{
					CPad::TempKeyState.END = 255;
					break;
				}
				
			case rsPGUP:
				{
					CPad::TempKeyState.PGUP = 255;
					break;
				}
				
			case rsPGDN:
				{
					CPad::TempKeyState.PGDN = 255;
					break;
				}
				
			case rsUP:
				{
					CPad::TempKeyState.UP = 255;
					break;
				}
				
			case rsDOWN:
				{
					CPad::TempKeyState.DOWN = 255;
					break;
				}
				
			case rsLEFT:
				{
					CPad::TempKeyState.LEFT = 255;
					break;
				}
				
			case rsRIGHT:
				{
					CPad::TempKeyState.RIGHT = 255;
					break;
				}
				
			case rsNUMLOCK:
				{
					CPad::TempKeyState.NUMLOCK = 255;
					break;
				}
				
			case rsPADDEL:
				{
					CPad::TempKeyState.DECIMAL = 255;
					break;
				}
				
			case rsPADEND:
				{
					CPad::TempKeyState.NUM1 = 255;
					break;
				}
				
			case rsPADDOWN:
				{
					CPad::TempKeyState.NUM2 = 255;
					break;
				}
				
			case rsPADPGDN:
				{
					CPad::TempKeyState.NUM3 = 255;
					break;
				}
				
			case rsPADLEFT:
				{
					CPad::TempKeyState.NUM4 = 255;
					break;
				}
				
			case rsPAD5:
				{
					CPad::TempKeyState.NUM5 = 255;
					break;
				}
				
			case rsPADRIGHT:
				{
					CPad::TempKeyState.NUM6 = 255;
					break;
				}
				
			case rsPADHOME:
				{
					CPad::TempKeyState.NUM7 = 255;
					break;
				}
				
			case rsPADUP:
				{
					CPad::TempKeyState.NUM8 = 255;
					break;
				}
				
			case rsPADPGUP:
				{
					CPad::TempKeyState.NUM9 = 255;
					break;
				}
				
			case rsPADINS:
				{
					CPad::TempKeyState.NUM0 = 255;
					break;
				}
				
			case rsDIVIDE:
				{
					CPad::TempKeyState.DIV = 255;
					break;
				}
				
			case rsTIMES:
				{
					CPad::TempKeyState.MUL = 255;
					break;
				}
				
			case rsMINUS:
				{
					CPad::TempKeyState.SUB = 255;
					break;
				}
				
			case rsPADENTER:
				{
					CPad::TempKeyState.ENTER = 255;
					break;
				}
				
			case rsPLUS:
				{
					CPad::TempKeyState.ADD = 255;
					break;
				}
				
			case rsENTER:
				{
					CPad::TempKeyState.EXTENTER = 255;
					break;
				}
				
			case rsSCROLL:
				{
					CPad::TempKeyState.SCROLLLOCK = 255;
					break;
				}
				
			case rsPAUSE:
				{
					CPad::TempKeyState.PAUSE = 255;
					break;
				}
				
			case rsBACKSP:
				{
					CPad::TempKeyState.BACKSP = 255;
					break;
				}
				
			case rsTAB:
				{
					CPad::TempKeyState.TAB = 255;
					break;
				}
				
			case rsCAPSLK:
				{
					CPad::TempKeyState.CAPSLOCK = 255;
					break;
				}
				
			case rsLSHIFT:
				{
					CPad::TempKeyState.LSHIFT = 255;
					break;
				}
				
			case rsSHIFT:
				{
					CPad::TempKeyState.SHIFT = 255;
					break;
				}
				
			case rsRSHIFT:
				{
					CPad::TempKeyState.RSHIFT = 255;
					break;
				}
				
			case rsLCTRL:
				{
					CPad::TempKeyState.LCTRL = 255;
					break;
				}
				
			case rsRCTRL:
				{
					CPad::TempKeyState.RCTRL = 255;
					break;
				}
				
			case rsLALT:
				{
					CPad::TempKeyState.LALT = 255;
					break;
				}
				
			case rsRALT:
				{
					CPad::TempKeyState.RALT = 255;
					break;
				}
				
				
			case rsLWIN:
				{
					CPad::TempKeyState.LWIN = 255;
					break;
				}
				
			case rsRWIN:
				{
					CPad::TempKeyState.RWIN = 255;
					break;
				}
				
			case rsAPPS:
				{
					CPad::TempKeyState.APPS = 255;
					break;
				}
				
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
					CPad::TempKeyState.F[c - rsF1] = 255;
					break;
				}
				
			default:
				{
					if ( c < 255 )
					{
						CPad::TempKeyState.VK_KEYS[c] = 255;
						pad0->AddToPCCheatString(c);
					}
					break;
				}
		}
		
		if ( CPad::m_bMapPadOneToPadTwo )
		{
			if ( c == 'D' ) pad1->PCTempKeyState.LeftStickX = 128;
			if ( c == 'A' ) pad1->PCTempKeyState.LeftStickX = -128;
			if ( c == 'W' ) pad1->PCTempKeyState.LeftStickY = 128;
			if ( c == 'S' ) pad1->PCTempKeyState.LeftStickY = -128;
			if ( c == 'J' ) pad1->PCTempKeyState.RightStickX = 128;
			if ( c == 'G' ) pad1->PCTempKeyState.RightStickX = -128;
			if ( c == 'Y' ) pad1->PCTempKeyState.RightStickY = 128;
			if ( c == 'H' ) pad1->PCTempKeyState.RightStickY = -128;
			if ( c == 'Z' ) pad1->PCTempKeyState.LeftShoulder1 = 255;
			if ( c == 'X' ) pad1->PCTempKeyState.LeftShoulder2 = 255;
			if ( c == 'C' ) pad1->PCTempKeyState.RightShoulder1 = 255;
			if ( c == 'V' ) pad1->PCTempKeyState.RightShoulder2 = 255;
			if ( c == 'O' ) pad1->PCTempKeyState.DPadUp = 255;
			if ( c == 'L' ) pad1->PCTempKeyState.DPadDown = 255;
			if ( c == 'K' ) pad1->PCTempKeyState.DPadLeft = 255;
			if ( c == ';' ) pad1->PCTempKeyState.DPadRight = 255;
			if ( c == 'B' ) pad1->PCTempKeyState.Start = 255;
			if ( c == 'N' ) pad1->PCTempKeyState.Select = 255;
			if ( c == 'M' ) pad1->PCTempKeyState.Square = 255;
			if ( c == ',' ) pad1->PCTempKeyState.Triangle = 255;
			if ( c == '.' ) pad1->PCTempKeyState.Cross = 255;
			if ( c == '/' ) pad1->PCTempKeyState.Circle = 255;
			if ( c == rsRSHIFT ) pad1->PCTempKeyState.LeftShock = 255;
			if ( c == rsRCTRL ) pad1->PCTempKeyState.RightShock = 255;
		}
	}
	
	return rsEVENTPROCESSED;
}


static RsEventStatus 
HandleKeyUp(RsKeyStatus *keyStatus)
{
	CPad *pad0 = CPad::GetPad(0);
	CPad *pad1 = CPad::GetPad(1);
	
	RwInt32 c = keyStatus->keyCharCode;
	
	if ( c != rsNULL )
	{
		switch (c)
		{
			case rsESC:
				{
					CPad::TempKeyState.ESC = 0;
					break;
				}
			
			case rsINS:
				{
					CPad::TempKeyState.INS = 0;
					break;
				}
				
			case rsDEL:
				{
					CPad::TempKeyState.DEL = 0;
					break;
				}
				
			case rsHOME:
				{
					CPad::TempKeyState.HOME = 0;
					break;
				}
				
			case rsEND:
				{
					CPad::TempKeyState.END = 0;
					break;
				}
				
			case rsPGUP:
				{
					CPad::TempKeyState.PGUP = 0;
					break;
				}
				
			case rsPGDN:
				{
					CPad::TempKeyState.PGDN = 0;
					break;
				}
				
			case rsUP:
				{
					CPad::TempKeyState.UP = 0;
					break;
				}
				
			case rsDOWN:
				{
					CPad::TempKeyState.DOWN = 0;
					break;
				}
				
			case rsLEFT:
				{
					CPad::TempKeyState.LEFT = 0;
					break;
				}
				
			case rsRIGHT:
				{
					CPad::TempKeyState.RIGHT = 0;
					break;
				}
				
			case rsNUMLOCK:
				{
					CPad::TempKeyState.NUMLOCK = 0;
					break;
				}
				
			case rsPADDEL:
				{
					CPad::TempKeyState.DECIMAL = 0;
					break;
				}
				
			case rsPADEND:
				{
					CPad::TempKeyState.NUM1 = 0;
					break;
				}
				
			case rsPADDOWN:
				{
					CPad::TempKeyState.NUM2 = 0;
					break;
				}
				
			case rsPADPGDN:
				{
					CPad::TempKeyState.NUM3 = 0;
					break;
				}
				
			case rsPADLEFT:
				{
					CPad::TempKeyState.NUM4 = 0;
					break;
				}
				
			case rsPAD5:
				{
					CPad::TempKeyState.NUM5 = 0;
					break;
				}
				
			case rsPADRIGHT:
				{
					CPad::TempKeyState.NUM6 = 0;
					break;
				}
				
			case rsPADHOME:
				{
					CPad::TempKeyState.NUM7 = 0;
					break;
				}
				
			case rsPADUP:
				{
					CPad::TempKeyState.NUM8 = 0;
					break;
				}
				
			case rsPADPGUP:
				{
					CPad::TempKeyState.NUM9 = 0;
					break;
				}
				
			case rsPADINS:
				{
					CPad::TempKeyState.NUM0 = 0;
					break;
				}
				
			case rsDIVIDE:
				{
					CPad::TempKeyState.DIV = 0;
					break;
				}
				
			case rsTIMES:
				{
					CPad::TempKeyState.MUL = 0;
					break;
				}
				
			case rsMINUS:
				{
					CPad::TempKeyState.SUB = 0;
					break;
				}
				
			case rsPADENTER:
				{
					CPad::TempKeyState.ENTER = 0;
					break;
				}
				
			case rsPLUS:
				{
					CPad::TempKeyState.ADD = 0;
					break;
				}
				
			case rsENTER:
				{
					CPad::TempKeyState.EXTENTER = 0;
					break;
				}
				
			case rsSCROLL:
				{
					CPad::TempKeyState.SCROLLLOCK = 0;
					break;
				}
				
			case rsPAUSE:
				{
					CPad::TempKeyState.PAUSE = 0;
					break;
				}
				
			case rsBACKSP:
				{
					CPad::TempKeyState.BACKSP = 0;
					break;
				}
				
			case rsTAB:
				{
					CPad::TempKeyState.TAB = 0;
					break;
				}
				
			case rsCAPSLK:
				{
					CPad::TempKeyState.CAPSLOCK = 0;
					break;
				}
				
			case rsLSHIFT:
				{
					CPad::TempKeyState.LSHIFT = 0;
					break;
				}
				
			case rsSHIFT:
				{
					CPad::TempKeyState.SHIFT = 0;
					break;
				}
				
			case rsRSHIFT:
				{
					CPad::TempKeyState.RSHIFT = 0;
					break;
				}
				
			case rsLCTRL:
				{
					CPad::TempKeyState.LCTRL = 0;
					break;
				}
				
			case rsRCTRL:
				{
					CPad::TempKeyState.RCTRL = 0;
					break;
				}
				
			case rsLALT:
				{
					CPad::TempKeyState.LALT = 0;
					break;
				}
				
			case rsRALT:
				{
					CPad::TempKeyState.RALT = 0;
					break;
				}
				
				
			case rsLWIN:
				{
					CPad::TempKeyState.LWIN = 0;
					break;
				}
				
			case rsRWIN:
				{
					CPad::TempKeyState.RWIN = 0;
					break;
				}
				
			case rsAPPS:
				{
					CPad::TempKeyState.APPS = 0;
					break;
				}
				
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
					CPad::TempKeyState.F[c - rsF1] = 0;
					break;
				}
				
			default:
				{
					if ( c < 255 )
					{
						CPad::TempKeyState.VK_KEYS[c] = 0;
					}
					break;
				}
		}
		
		if ( CPad::m_bMapPadOneToPadTwo )
		{
			if ( c == 'D' ) pad1->PCTempKeyState.LeftStickX = 0;
			if ( c == 'A' ) pad1->PCTempKeyState.LeftStickX = 0;
			if ( c == 'W' ) pad1->PCTempKeyState.LeftStickY = 0;
			if ( c == 'S' ) pad1->PCTempKeyState.LeftStickY = 0;
			if ( c == 'J' ) pad1->PCTempKeyState.RightStickX = 0;
			if ( c == 'G' ) pad1->PCTempKeyState.RightStickX = 0;
			if ( c == 'Y' ) pad1->PCTempKeyState.RightStickY = 0;
			if ( c == 'H' ) pad1->PCTempKeyState.RightStickY = 0;
			if ( c == 'Z' ) pad1->PCTempKeyState.LeftShoulder1 = 0;
			if ( c == 'X' ) pad1->PCTempKeyState.LeftShoulder2 = 0;
			if ( c == 'C' ) pad1->PCTempKeyState.RightShoulder1 = 0;
			if ( c == 'V' ) pad1->PCTempKeyState.RightShoulder2 = 0;
			if ( c == 'O' ) pad1->PCTempKeyState.DPadUp = 0;
			if ( c == 'L' ) pad1->PCTempKeyState.DPadDown = 0;
			if ( c == 'K' ) pad1->PCTempKeyState.DPadLeft = 0;
			if ( c == ';' ) pad1->PCTempKeyState.DPadRight = 0;
			if ( c == 'B' ) pad1->PCTempKeyState.Start = 0;
			if ( c == 'N' ) pad1->PCTempKeyState.Select = 0;
			if ( c == 'M' ) pad1->PCTempKeyState.Square = 0;
			if ( c == ',' ) pad1->PCTempKeyState.Triangle = 0;
			if ( c == '.' ) pad1->PCTempKeyState.Cross = 0;
			if ( c == '/' ) pad1->PCTempKeyState.Circle = 0;
			if ( c == rsRSHIFT ) pad1->PCTempKeyState.LeftShock = 0;
			if ( c == rsRCTRL ) pad1->PCTempKeyState.RightShock = 0;
		}
	}
	
	return rsEVENTPROCESSED;
}


/*
 *****************************************************************************
 */
static RsEventStatus 
KeyboardHandler(RsEvent event, void *param)
{
	/*
	 * ...then the application events, if necessary...
	 */
	switch( event )
	{
		case rsKEYDOWN:
		{
			return HandleKeyDown((RsKeyStatus *)param);
		}

		case rsKEYUP:
		{
			return HandleKeyUp((RsKeyStatus *)param);
		}

		default:
		{
			return rsEVENTNOTPROCESSED;
		}
	}
}

/*
 *****************************************************************************
 */
static RsEventStatus
HandlePadButtonDown(RsPadButtonStatus *padButtonStatus)
{
	bool bPadTwo = false;
	int32 padNumber = padButtonStatus->padID;
	
	CPad *pad = CPad::GetPad(padNumber);
	
	if ( CPad::m_bMapPadOneToPadTwo )
		padNumber = 1;
	
	if ( padNumber == 1 )
		bPadTwo = true;
	
	ControlsManager.UpdateJoyButtonState(padNumber);
	
	for ( int32 i = 0; i < _TODOCONST(16); i++ )
	{
		RsPadButtons btn = rsPADNULL;
		if ( ControlsManager.m_aButtonStates[i] == TRUE )
			btn = (RsPadButtons)(i + 1);

		if ( FrontEndMenuManager.m_bMenuActive || bPadTwo )
			ControlsManager.UpdateJoyInConfigMenus_ButtonDown(btn, padNumber);
		else
			ControlsManager.AffectControllerStateOn_ButtonDown(btn, JOYSTICK);
	}
	
	return rsEVENTPROCESSED;
}


/*
 *****************************************************************************
 */
static RsEventStatus
HandlePadButtonUp(RsPadButtonStatus *padButtonStatus)
{
	bool bPadTwo = false;
	int32 padNumber = padButtonStatus->padID;

	CPad *pad = CPad::GetPad(padNumber);
	
	if ( CPad::m_bMapPadOneToPadTwo )
		padNumber = 1;

	if ( padNumber == 1 )
		bPadTwo = true;
	
	bool bCam = false;
	int16 mode = TheCamera.Cams[TheCamera.ActiveCam].Mode;
	if ( mode == CCam::MODE_FLYBY || mode == CCam::MODE_FIXED )
		bCam = true;
	
	ControlsManager.UpdateJoyButtonState(padNumber);
	
	for ( int32 i = 1; i < _TODOCONST(16); i++ )
	{
		RsPadButtons btn = rsPADNULL;
		if ( ControlsManager.m_aButtonStates[i] == FALSE )
			btn = (RsPadButtons)(i + 1); // bug ?, cycle begins from 1(not zero), 1+1==2==rsPADBUTTON2, so we skip rsPADBUTTON1, right ?
		
		if ( FrontEndMenuManager.m_bMenuActive || bPadTwo || bCam )
			ControlsManager.UpdateJoyInConfigMenus_ButtonUp(btn, padNumber);
		else
			ControlsManager.AffectControllerStateOn_ButtonUp(btn, JOYSTICK);
	}

	return rsEVENTPROCESSED;
}

/*
 *****************************************************************************
 */
static RsEventStatus 
PadHandler(RsEvent event, void *param)
{
	switch( event )
	{
		case rsPADBUTTONDOWN:
		{
			return HandlePadButtonDown((RsPadButtonStatus *)param);
		}

		case rsPADBUTTONUP:
		{
			return HandlePadButtonUp((RsPadButtonStatus *)param);
		}

		default:
		{
			return rsEVENTNOTPROCESSED;
		}
	}
}


/*
 *****************************************************************************
 */
RwBool
AttachInputDevices(void)
{
#ifndef IGNORE_MOUSE_KEYBOARD
	RsInputDeviceAttach(rsKEYBOARD, KeyboardHandler);
#endif

	RsInputDeviceAttach(rsPAD, PadHandler);

	return TRUE;
}
