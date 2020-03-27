#pragma warning( push )
#pragma warning( disable : 4005)
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#pragma warning( pop )

#include "common.h"
#ifdef XINPUT
#include <Xinput.h>
#pragma comment( lib, "Xinput.lib" )
#endif
#include "patcher.h"
#include "Pad.h"
#include "ControllerConfig.h"
#include "Timer.h"
#include "Frontend.h"
#include "Camera.h"
#include "Game.h"
#include "CutsceneMgr.h"
#include "Font.h"
#include "Hud.h"
#include "Text.h"
#include "Timer.h"
#include "World.h"
#include "Vehicle.h"
#include "Ped.h"
#include "Population.h"
#include "Replay.h"
#include "Weather.h"
#include "win.h"

CPad *Pads = (CPad*)0x6F0360; // [2]
CMousePointerStateHelper &MousePointerStateHelper = *(CMousePointerStateHelper*)0x95CC8C;

bool &CPad::bDisplayNoControllerMessage = *(bool *)0x95CD52;
bool &CPad::bObsoleteControllerMessage = *(bool *)0x95CDB8;
bool &CPad::m_bMapPadOneToPadTwo = *(bool *)0x95CD48;

CKeyboardState &CPad::OldKeyState = *(CKeyboardState*)0x6F1E70;
CKeyboardState &CPad::NewKeyState = *(CKeyboardState*)0x6E60D0;
CKeyboardState &CPad::TempKeyState = *(CKeyboardState*)0x774DE8;

char CPad::KeyBoardCheatString[20];

CMouseControllerState &CPad::OldMouseControllerState = *(CMouseControllerState*)0x8472A0;
CMouseControllerState &CPad::NewMouseControllerState = *(CMouseControllerState*)0x8809F0;
CMouseControllerState &CPad::PCTempMouseControllerState = *(CMouseControllerState*)0x6F1E60;

_TODO("gbFastTime");
extern bool &gbFastTime;

WRAPPER void WeaponCheat() { EAXJMP(0x490D90); }
WRAPPER void HealthCheat() { EAXJMP(0x490E70); }
WRAPPER void TankCheat() { EAXJMP(0x490EE0); }
WRAPPER void BlowUpCarsCheat() { EAXJMP(0x491040); }
WRAPPER void ChangePlayerCheat() { EAXJMP(0x4910B0); }
WRAPPER void MayhemCheat() { EAXJMP(0x4911C0); }
WRAPPER void EverybodyAttacksPlayerCheat() { EAXJMP(0x491270); }
WRAPPER void WeaponsForAllCheat() { EAXJMP(0x491370); }
WRAPPER void FastTimeCheat() { EAXJMP(0x4913A0); }
WRAPPER void SlowTimeCheat() { EAXJMP(0x4913F0); }
WRAPPER void MoneyCheat() { EAXJMP(0x491430); }
WRAPPER void ArmourCheat() { EAXJMP(0x491460); }
WRAPPER void WantedLevelUpCheat() { EAXJMP(0x491490); }
WRAPPER void WantedLevelDownCheat() { EAXJMP(0x4914F0); }
WRAPPER void SunnyWeatherCheat() { EAXJMP(0x491520); }
WRAPPER void CloudyWeatherCheat() { EAXJMP(0x491550); }
WRAPPER void RainyWeatherCheat() { EAXJMP(0x491580); }
WRAPPER void FoggyWeatherCheat() { EAXJMP(0x4915B0); }
WRAPPER void FastWeatherCheat() { EAXJMP(0x4915E0); }
WRAPPER void OnlyRenderWheelsCheat() { EAXJMP(0x491610); }
WRAPPER void ChittyChittyBangBangCheat() { EAXJMP(0x491640); }
WRAPPER void StrongGripCheat() { EAXJMP(0x491670); }
WRAPPER void NastyLimbsCheat() { EAXJMP(0x4916A0); }
//////////////////////////////////////////////////////////////////////////

#ifdef KANGAROO_CHEAT
void KangarooCheat()
{
	wchar *string;
	CPed *playerPed = FindPlayerPed();
	int m_fMass;

	if (playerPed->m_ped_flagI80) {
		string = TheText.Get("CHEATOF");
		m_fMass = 70.0f;
	} else {
		string = TheText.Get("CHEAT1");
		m_fMass = 15.0f;
	}
	CHud::SetHelpMessage(string, 1);
	playerPed->m_ped_flagI80 = !playerPed->m_ped_flagI80;

	playerPed->m_fMass = m_fMass;
	playerPed->m_fAirResistance = 0.4f / m_fMass;
}
#endif

void
CControllerState::Clear(void)
{
	LeftStickX = LeftStickY = RightStickX = RightStickY = 0;
	LeftShoulder1 = LeftShoulder2 = RightShoulder1 = RightShoulder2 = 0;
	DPadUp = DPadDown = DPadLeft = DPadRight = 0;
	Start = Select = 0;
	Square = Triangle = Cross = Circle = 0;
	LeftShock = RightShock = 0;
	NetworkTalk = 0;
}

void CKeyboardState::Clear()
{
	for ( int32 i = 0; i < 12; i++  )
		F[i] = 0;
	
	for ( int32 i = 0; i < 256; i++  )
		VK_KEYS[i] = 0;

	ESC = INS = DEL = HOME = END = PGUP = PGDN = 0;
	
	UP = DOWN = LEFT = RIGHT = 0;
	
	NUMLOCK = 0;
	
	DIV = MUL = SUB = ADD = 0;
	
	DECIMAL = NUM1 = NUM2 = NUM3 = NUM4 = 0;
	
	NUM5 = NUM6 = NUM7 = NUM8 = 0;
	
	NUM9 = NUM0 = SCROLLLOCK = PAUSE = 0;

	BACKSP = TAB = CAPSLOCK = EXTENTER = 0;

	LSHIFT = SHIFT = RSHIFT = LCTRL = RCTRL = LALT = RALT = 0;

	LWIN = RWIN = APPS = 0;
}

void CPad::Clear(bool bResetPlayerControls)
{
	NewState.Clear();
	OldState.Clear();
	
	PCTempKeyState.Clear();
	PCTempJoyState.Clear();
	PCTempMouseState.Clear();
	
	NewKeyState.Clear();
	OldKeyState.Clear();
	TempKeyState.Clear();
	
	NewMouseControllerState.Clear();
	OldMouseControllerState.Clear();
	PCTempMouseControllerState.Clear();
	
	Phase = 0;
	ShakeFreq = 0;
	ShakeDur = 0;
	
	if ( bResetPlayerControls )
		DisablePlayerControls = PLAYERCONTROL_ENABLED;
	
	bApplyBrakes = false;
	
	
	for ( int32 i = 0; i < _TODOCONST(5); i++ )
		bHornHistory[i] = false;
	
	iCurrHornHistory = 0;
	
	for ( int32 i = 0; i < _TODOCONST(12); i++ )
		_unk[i] = ' ';
	
	LastTimeTouched = CTimer::GetTimeInMilliseconds();
	AverageWeapon = 0;
	AverageEntries = 0;
}

void CPad::ClearMouseHistory()
{
	PCTempMouseControllerState.Clear();
	NewMouseControllerState.Clear();
	OldMouseControllerState.Clear();
}

CMouseControllerState::CMouseControllerState()
{
	LMB = 0;
	RMB = 0;
	MMB = 0;
	WHEELUP = 0;
	WHEELDN = 0;
	MXB1 = 0;
	MXB2 = 0;
	
	x = 0.0f;
	y = 0.0f;
}

void CMouseControllerState::Clear()
{
	LMB = 0;
	RMB = 0;
	MMB = 0;
	WHEELUP = 0;
	WHEELDN = 0;
	MXB1 = 0;
	MXB2 = 0;
}

CMouseControllerState CMousePointerStateHelper::GetMouseSetUp()
{
	CMouseControllerState state;
	
	if ( PSGLOBAL(mouse) == nil )
		_InputInitialiseMouse();
	
	if ( PSGLOBAL(mouse) != nil )
	{
		DIDEVCAPS devCaps;
		devCaps.dwSize = sizeof(DIDEVCAPS);
		
		PSGLOBAL(mouse)->GetCapabilities(&devCaps);
		
		switch ( devCaps.dwButtons )
		{
			case 3:
			case 4:
			case 5:
			case 6:
			case 7:
			case 8:
				state.MMB = true;
				
			case 2:
				state.RMB = true;
				
			case 1:
				state.LMB = true;
		}
		
		if ( devCaps.dwAxes == 3 )
		{
			state.WHEELDN = true;
			state.WHEELUP = true;
		}
	}

	return state;
}

void CPad::UpdateMouse()
{
	if ( IsForegroundApp() )
	{
		if ( PSGLOBAL(mouse) == nil )
			_InputInitialiseMouse();
		
		DIMOUSESTATE2 state;
		
		if ( PSGLOBAL(mouse) != nil && SUCCEEDED(_InputGetMouseState(&state)) )
		{
			int32 signX = 1;
			int32 signy = 1;

			if ( !FrontEndMenuManager.m_bMenuActive )
			{
				if ( MousePointerStateHelper.bInvertVertically )
					signy = -1;
				if ( MousePointerStateHelper.bInvertHorizontally )
					signX = -1;
			}
			
			PCTempMouseControllerState.Clear();
			
			PCTempMouseControllerState.x = (float)(signX * state.lX);
			PCTempMouseControllerState.y = (float)(signy * state.lY);
			PCTempMouseControllerState.LMB = state.rgbButtons[0] & 128;
			PCTempMouseControllerState.RMB = state.rgbButtons[1] & 128;
			PCTempMouseControllerState.MMB = state.rgbButtons[2] & 128;
			PCTempMouseControllerState.MXB1 = state.rgbButtons[3] & 128;
			PCTempMouseControllerState.MXB2 = state.rgbButtons[4] & 128;
			
			if ( state.lZ > 0 )
				PCTempMouseControllerState.WHEELUP = 1;
			else if ( state.lZ < 0 )
				PCTempMouseControllerState.WHEELDN = 1;
			
			OldMouseControllerState = NewMouseControllerState;
			NewMouseControllerState = PCTempMouseControllerState;
		}
	}
}

CControllerState CPad::ReconcileTwoControllersInput(CControllerState const &State1, CControllerState const &State2)
{
	static CControllerState ReconState;
	
	ReconState.Clear();

#define _RECONCILE_BUTTON(button) \
	{ if ( State1.button || State2.button ) ReconState.button = 255; }
	
#define _RECONCILE_AXIS_POSITIVE(axis) \
	{ if ( State1.axis >= 0 && State2.axis >= 0 ) ReconState.axis = max(State1.axis, State2.axis); }

#define _RECONCILE_AXIS_NEGATIVE(axis) \
	{ if ( State1.axis <= 0 && State2.axis <= 0 ) ReconState.axis = min(State1.axis, State2.axis); }

#define _RECONCILE_AXIS(axis) \
	{ _RECONCILE_AXIS_POSITIVE(axis); _RECONCILE_AXIS_NEGATIVE(axis); }
	
#define _FIX_AXIS_DIR(axis) \
	{ if ( State1.axis > 0 && State2.axis < 0 || State1.axis < 0 && State2.axis > 0 ) ReconState.axis = 0; }
		
#define _FIX_RECON_DIR(pos, neg, axis) \
	{ if ( (ReconState.pos || ReconState.axis < 0) && (ReconState.neg || ReconState.axis > 0) ) { ReconState.pos = 0; ReconState.neg = 0; ReconState.axis = 0; } }

	_RECONCILE_BUTTON(LeftShoulder1);
	_RECONCILE_BUTTON(LeftShoulder2);
	_RECONCILE_BUTTON(RightShoulder1);
	_RECONCILE_BUTTON(RightShoulder2);
	_RECONCILE_BUTTON(Start);
	_RECONCILE_BUTTON(Select);
	_RECONCILE_BUTTON(Square);
	_RECONCILE_BUTTON(Triangle);
	_RECONCILE_BUTTON(Cross);
	_RECONCILE_BUTTON(Circle);
	_RECONCILE_BUTTON(LeftShock);
	_RECONCILE_BUTTON(RightShock);
	_RECONCILE_BUTTON(NetworkTalk);
	_RECONCILE_AXIS(LeftStickX);
	_RECONCILE_AXIS(LeftStickY);
	_FIX_AXIS_DIR(LeftStickX);
	_FIX_AXIS_DIR(LeftStickY);
	_RECONCILE_AXIS(RightStickX);
	_RECONCILE_AXIS(RightStickY);
	_FIX_AXIS_DIR(RightStickX);
	_FIX_AXIS_DIR(RightStickY);
	_RECONCILE_BUTTON(DPadUp);
	_RECONCILE_BUTTON(DPadDown);
	_RECONCILE_BUTTON(DPadLeft);
	_RECONCILE_BUTTON(DPadRight);
	_FIX_RECON_DIR(DPadUp, DPadDown, LeftStickY);
	_FIX_RECON_DIR(DPadLeft, DPadRight, LeftStickX);

	return ReconState;
	
#undef _RECONCILE_BUTTON
#undef _RECONCILE_AXIS_POSITIVE
#undef _RECONCILE_AXIS_NEGATIVE
#undef _RECONCILE_AXIS
#undef _FIX_AXIS_DIR
#undef _FIX_RECON_DIR
}

void CPad::StartShake(int16 nDur, uint8 nFreq)
{
	if ( !CMenuManager::m_PrefsUseVibration )
		return;
	
	if ( CCutsceneMgr::IsRunning() || CGame::playingIntro )
		return;
	
	if ( nFreq == 0 )
	{
		ShakeDur = 0;
		ShakeFreq = 0;
		return;
	}
	
	if ( nDur > ShakeDur )
	{
		ShakeDur = nDur;
		ShakeFreq = nFreq;
	}
}

void CPad::StartShake_Distance(int16 nDur, uint8 nFreq, float fX, float fY, float fZ)
{
	if ( !CMenuManager::m_PrefsUseVibration )
		return;
	
	if ( CCutsceneMgr::IsRunning() || CGame::playingIntro )
		return;
	
	float fDist = ( TheCamera.GetPosition() - CVector(fX, fY, fZ) ).Magnitude();
	
	if ( fDist < 70.0f )
	{
		if ( nFreq == 0 )
		{
			ShakeDur = 0;
			ShakeFreq = 0;
			return;
		}
		
		if ( nDur > ShakeDur )
		{
			ShakeDur = nDur;
			ShakeFreq = nFreq;
		}
	}
}

void CPad::StartShake_Train(float fX, float fY)
{
	if ( !CMenuManager::m_PrefsUseVibration )
		return;
	
	if ( CCutsceneMgr::IsRunning() || CGame::playingIntro )
		return;
	
	if (FindPlayerVehicle() != nil && FindPlayerVehicle()->IsTrain() )
		return;
	
	float fDist = ( TheCamera.GetPosition() - CVector(fX, fY, 0.0f) ).Magnitude2D();
	
	if ( fDist < 70.0f )
	{
		int32 freq = (int32)((70.0f - fDist) * 70.0f / 70.0f + 30.0f);

		if ( ShakeDur < 100 )
		{
			ShakeDur = 100;
			ShakeFreq = freq;
		}
	}
}

void CPad::AddToPCCheatString(char c)
{
	for ( int32 i = ARRAY_SIZE(KeyBoardCheatString) - 2; i >= 0; i-- )
		KeyBoardCheatString[i + 1] = KeyBoardCheatString[i];
	
	KeyBoardCheatString[0] = c;
	
	#define _CHEATCMP(str) strncmp(str, KeyBoardCheatString, sizeof(str)-1)
	
	// "GUNSGUNSGUNS"
	if ( !_CHEATCMP("SNUGSNUGSNUG") )
		WeaponCheat();

	// "IFIWEREARICHMAN"
	if ( !_CHEATCMP("NAMHCIRAEREWIFI") )
		MoneyCheat();
	
	// "GESUNDHEIT"
	if ( !_CHEATCMP("TIEHDNUSEG") )
		HealthCheat();
	
	// "MOREPOLICEPLEASE"
	if ( !_CHEATCMP("ESAELPECILOPEROM") )
		WantedLevelUpCheat();
	
	// "NOPOLICEPLEASE"
	if ( !_CHEATCMP("ESAELPECILOPON") )
		WantedLevelDownCheat();
	
	// "GIVEUSATANK"
	if ( !_CHEATCMP("KNATASUEVIG") )
		TankCheat();
	
	// "BANGBANGBANG"
	if ( !_CHEATCMP("GNABGNABGNAB") )
		BlowUpCarsCheat();
	
	// "ILIKEDRESSINGUP"
	if ( !_CHEATCMP("PUGNISSERDEKILI") )
		ChangePlayerCheat();
	
	// "ITSALLGOINGMAAAD"
	if ( !_CHEATCMP("DAAAMGNIOGLLASTI") )
		MayhemCheat();
	
	// "NOBODYLIKESME"
	if ( !_CHEATCMP("EMSEKILYDOBON") )
		EverybodyAttacksPlayerCheat();
	
	// "WEAPONSFORALL"
	if ( !_CHEATCMP("LLAROFSNOPAEW") )
		WeaponsForAllCheat();
	
	// "TIMEFLIESWHENYOU"
	if ( !_CHEATCMP("UOYNEHWSEILFEMIT") )
		FastTimeCheat();
	
	// "BOOOOORING"
	if ( !_CHEATCMP("GNIROOOOOB") )
		SlowTimeCheat();
	
#ifndef GTA3_1_1_PATCH
	// "TURTOISE"
	if ( !_CHEATCMP("ESIOTRUT") )
		ArmourCheat();
#else	
	// "TORTOISE"
	if ( !_CHEATCMP("ESIOTROT") )
		ArmourCheat();
#endif
	
	// "SKINCANCERFORME"
	if ( !_CHEATCMP("EMROFRECNACNIKS") )
		SunnyWeatherCheat();
	
	// "ILIKESCOTLAND"
	if ( !_CHEATCMP("DNALTOCSEKILI") )
		CloudyWeatherCheat();
	
	// "ILOVESCOTLAND"
	if ( !_CHEATCMP("DNALTOCSEVOLI") )
		RainyWeatherCheat();
	
	// "PEASOUP"
	if ( !_CHEATCMP("PUOSAEP") )
		FoggyWeatherCheat();
	
	// "MADWEATHER"
	if ( !_CHEATCMP("REHTAEWDAM") )
		FastWeatherCheat();
	
	// "ANICESETOFWHEELS"
	if ( !_CHEATCMP("SLEEHWFOTESECINA") )
		OnlyRenderWheelsCheat();
	
	// "CHITTYCHITTYBB"
	if ( !_CHEATCMP("BBYTTIHCYTTIHC") )
		ChittyChittyBangBangCheat();
	
	// "CORNERSLIKEMAD"
	if ( !_CHEATCMP("DAMEKILSRENROC") )
		StrongGripCheat();
	
	// "NASTYLIMBSCHEAT"
	if ( !_CHEATCMP("TAEHCSBMILYTSAN") )
		NastyLimbsCheat();

#ifdef KANGAROO_CHEAT
	// "KANGAROO"
	if (!_CHEATCMP("OORAGNAK"))
		KangarooCheat();
#endif

#ifndef MASTER
	// "PEDDEBUG"
	if (!_CHEATCMP("GUBEDDEP"))
		CPed::SwitchDebugDisplay();
#endif
	
	#undef _CHEATCMP
}

#ifdef XINPUT
void CPad::AffectFromXinput(uint32 pad)
{
	XINPUT_STATE xstate;
	memset(&xstate, 0, sizeof(XINPUT_STATE));
	if (XInputGetState(pad, &xstate) == ERROR_SUCCESS)
	{
		PCTempJoyState.Circle = (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_B) ? 255 : 0;
		PCTempJoyState.Cross = (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_A) ? 255 : 0;
		PCTempJoyState.Square = (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_X) ? 255 : 0;
		PCTempJoyState.Triangle = (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_Y) ? 255 : 0;
		PCTempJoyState.DPadDown = (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN) ? 255 : 0;
		PCTempJoyState.DPadLeft = (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT) ? 255 : 0;
		PCTempJoyState.DPadRight = (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) ? 255 : 0;
		PCTempJoyState.DPadUp = (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP) ? 255 : 0;
		PCTempJoyState.LeftShock = (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB) ? 255 : 0;
		PCTempJoyState.LeftShoulder1 = (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER) ? 255 : 0;
		PCTempJoyState.LeftShoulder2 = xstate.Gamepad.bLeftTrigger;
		PCTempJoyState.RightShock = (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB) ? 255 : 0;
		PCTempJoyState.RightShoulder1 = (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER) ? 255 : 0;
		PCTempJoyState.RightShoulder2 = xstate.Gamepad.bRightTrigger;

		PCTempJoyState.Select = (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_BACK) ? 255 : 0;
		PCTempJoyState.Start = (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_START) ? 255 : 0;

		float lx = (float)xstate.Gamepad.sThumbLX / (float)0x7FFF;
		float ly = (float)xstate.Gamepad.sThumbLY / (float)0x7FFF;
		float rx = (float)xstate.Gamepad.sThumbRX / (float)0x7FFF;
		float ry = (float)xstate.Gamepad.sThumbRY / (float)0x7FFF;

		if (Abs(lx) > 0.3f || Abs(ly) > 0.3f) {
			PCTempJoyState.LeftStickX = (int32)(lx * 128.0f);
			PCTempJoyState.LeftStickY = (int32)(-ly * 128.0f);
		}

		if (Abs(rx) > 0.3f || Abs(ry) > 0.3f) {
			PCTempJoyState.RightStickX = (int32)(rx * 128.0f);
			PCTempJoyState.RightStickY = (int32)(ry * 128.0f);
		}
	}
}
#endif

void CPad::UpdatePads(void) 
{
	bool bUpdate = true;
	
	GetPad(0)->UpdateMouse();
#ifdef XINPUT
	GetPad(0)->AffectFromXinput(0);
	GetPad(1)->AffectFromXinput(1);
#else
	CapturePad(0);
#endif
	

	ControlsManager.ClearSimButtonPressCheckers();
	ControlsManager.AffectPadFromKeyBoard();
	ControlsManager.AffectPadFromMouse();
	
	if ( CReplay::IsPlayingBackFromFile() )
		bUpdate = false;
	
	if ( bUpdate )
	{
		GetPad(0)->Update(0);
	}

#if defined(MASTER) && !defined(XINPUT)
	GetPad(1)->NewState.Clear();
	GetPad(1)->OldState.Clear();
#endif
	
	OldKeyState = NewKeyState;
	NewKeyState = TempKeyState;
}

void CPad::ProcessPCSpecificStuff(void)
{
	;
}

void CPad::Update(int16 unk)
{
	OldState = NewState;
	
	NewState = ReconcileTwoControllersInput(PCTempKeyState,   PCTempJoyState);
	NewState = ReconcileTwoControllersInput(PCTempMouseState, NewState);
	
	PCTempJoyState.Clear();
	PCTempKeyState.Clear();
	PCTempMouseState.Clear();
	
	ProcessPCSpecificStuff();
	
	if ( ++iCurrHornHistory >= _TODOCONST(5) )
		iCurrHornHistory = 0;

	bHornHistory[iCurrHornHistory] = GetHorn();


	if ( !bDisplayNoControllerMessage )
		CGame::bDemoMode = false;
}

void CPad::DoCheats(void)
{
	GetPad(0)->DoCheats(0);
}

void CPad::DoCheats(int16 unk)
{
#ifdef PS2
	if ( GetTriangleJustDown() )
		AddToCheatString('T');
	
	if ( GetCircleJustDown() )
		AddToCheatString('C');
	
	if ( GetCrossJustDown() )
		AddToCheatString('X');
	
	if ( GetSquareJustDown() )
		AddToCheatString('S');
	
	if ( GetDPadUpJustDown() )
		AddToCheatString('U');
	
	if ( GetDPadDownJustDown() )
		AddToCheatString('D');
	
	if ( GetDPadLeftJustDown() )
		AddToCheatString('L');
	
	if ( GetDPadRightJustDown() )
		AddToCheatString('R');
	
	if ( GetLeftShoulder1JustDown() )
		AddToCheatString('1');
	
	if ( GetLeftShoulder2JustDown() )
		AddToCheatString('2');
	
	if ( GetRightShoulder1JustDown() )
		AddToCheatString('3');
	
	if ( GetRightShoulder2JustDown() )
		AddToCheatString('4');
#endif
}

void CPad::StopPadsShaking(void)
{
	GetPad(0)->StopShaking(0);
}

void CPad::StopShaking(int16 unk)
{
	;
}

CPad *CPad::GetPad(int32 pad)
{
	return &Pads[pad];
}

int16 CPad::GetSteeringLeftRight(void)
{
	if ( ArePlayerControlsDisabled() )
		return 0;
	
	switch ( Mode )
	{
		case 0:
		case 2:
		{
			int16 axis = NewState.LeftStickX;
			int16 dpad = (NewState.DPadRight - NewState.DPadLeft) / 2;
			
			if ( Abs(axis) > Abs(dpad) )
				return axis;
			else
				return dpad;
			
			break;
		}
		
		case 1:
		case 3:
		{
			return NewState.LeftStickX;

			break;
		}
	}
	
	return 0;
}

int16 CPad::GetSteeringUpDown(void)
{
	if ( ArePlayerControlsDisabled() )
		return 0;
	
	switch ( Mode )
	{
		case 0:
		case 2:
		{
			int16 axis = NewState.LeftStickY;
			int16 dpad = (NewState.DPadUp - NewState.DPadDown) / 2;
			
			if ( Abs(axis) > Abs(dpad) )
				return axis;
			else
				return dpad;
			
			break;
		}
		
		case 1:
		case 3:
		{
			return NewState.LeftStickY;

			break;
		}
	}
	
	return 0;
}

int16 CPad::GetCarGunUpDown(void)
{
	if ( ArePlayerControlsDisabled() )
		return 0;
	
	switch ( Mode )
	{
		case 0:
		case 1:
		case 2:
		{
			return NewState.RightStickY;
			
			break;
		}
		
		case 3:
		{
			return (NewState.DPadUp - NewState.DPadDown) / 2;
			
			break;
		}
	}
	
	return 0;
}

int16 CPad::GetCarGunLeftRight(void)
{
	if ( ArePlayerControlsDisabled() )
		return 0;
	
	switch ( Mode )
	{
		case 0:
		case 1:
		case 2:
		{
			return NewState.RightStickX;
			
			break;
		}
		
		case 3:
		{
			return (NewState.DPadRight - NewState.DPadLeft) / 2;
			
			break;
		}
	}
	
	return 0;
}

int16 CPad::GetPedWalkLeftRight(void)
{
	if ( ArePlayerControlsDisabled() )
		return 0;
	
	switch ( Mode )
	{
		case 0:
		case 2:
		{
			int16 axis = NewState.LeftStickX;
			int16 dpad = (NewState.DPadRight - NewState.DPadLeft) / 2;
			
			if ( Abs(axis) > Abs(dpad) )
				return axis;
			else
				return dpad;
			
			break;
		}
		
		case 1:
		case 3:
		{
			return NewState.LeftStickX;

			break;
		}
	}
	
	return 0;
}


int16 CPad::GetPedWalkUpDown(void)
{
	if ( ArePlayerControlsDisabled() )
		return 0;
	
	switch ( Mode )
	{
		case 0:
		case 2:
		{
			int16 axis = NewState.LeftStickY;
			int16 dpad = (NewState.DPadDown - NewState.DPadUp) / 2;
			
			if ( Abs(axis) > Abs(dpad) )
				return axis;
			else
				return dpad;
			
			break;
		}
		
		case 1:
		case 3:
		{
			return NewState.LeftStickY;

			break;
		}
	}
	
	return 0;
}

int16 CPad::GetAnalogueUpDown(void)
{
	switch ( Mode )
	{
		case 0:
		case 2:
		{
			int16 axis = NewState.LeftStickY;
			int16 dpad = (NewState.DPadDown - NewState.DPadUp) / 2;
			
			if ( Abs(axis) > Abs(dpad) )
				return axis;
			else
				return dpad;
			
			break;
		}
		
		case 1:
		case 3:
		{
			return NewState.LeftStickY;

			break;
		}
	}
	
	return 0;
}

bool CPad::GetLookLeft(void)
{
	if ( ArePlayerControlsDisabled() )
		return false;
	
	return !!(NewState.LeftShoulder2 && !NewState.RightShoulder2);
}

bool CPad::GetLookRight(void)
{
	if ( ArePlayerControlsDisabled() )
		return false;
	
	return !!(NewState.RightShoulder2 && !NewState.LeftShoulder2);
}


bool CPad::GetLookBehindForCar(void)
{
	if ( ArePlayerControlsDisabled() )
		return false;
	
	return !!(NewState.RightShoulder2 && NewState.LeftShoulder2);
}

bool CPad::GetLookBehindForPed(void)
{
	if ( ArePlayerControlsDisabled() )
		return false;

	return !!NewState.RightShock;
}

bool CPad::GetHorn(void)
{
	if ( ArePlayerControlsDisabled() )
		return false;
	
	switch ( Mode )
	{
		case 0:
		{
			return !!NewState.LeftShock;

			break;
		}
		
		case 1:
		{
			return !!NewState.LeftShoulder1;

			break;
		}
		
		case 2:
		{
			return !!NewState.RightShoulder1;

			break;
		}
		
		case 3:
		{
			return !!NewState.LeftShock;

			break;
		}
	}
	
	return false;
}

bool CPad::HornJustDown(void)
{
	if ( ArePlayerControlsDisabled() )
		return false;
	
	switch ( Mode )
	{
		case 0:
		{
			return !!(NewState.LeftShock && !OldState.LeftShock);

			break;
		}
		
		case 1:
		{
			return !!(NewState.LeftShoulder1 && !OldState.LeftShoulder1);

			break;
		}
		
		case 2:
		{
			return !!(NewState.RightShoulder1 && !OldState.RightShoulder1);

			break;
		}
		
		case 3:
		{
			return !!(NewState.LeftShock && !OldState.LeftShock);

			break;
		}
	}
	
	return false;
}


bool CPad::GetCarGunFired(void)
{
	if ( ArePlayerControlsDisabled() )
		return false;
	
	switch ( Mode )
	{
		case 0:
		case 1:
		case 2:
		{
			return !!NewState.Circle;

			break;
		}
		
		case 3:
		{
			return !!NewState.RightShoulder1;

			break;
		}
	}
	
	return false;
}

bool CPad::CarGunJustDown(void)
{
	if ( ArePlayerControlsDisabled() )
		return false;
	
	switch ( Mode )
	{
		case 0:
		case 1:
		case 2:
		{
			return !!(NewState.Circle && !OldState.Circle);

			break;
		}
		
		case 3:
		{
			return !!(NewState.RightShoulder1 && !OldState.RightShoulder1);

			break;
		}
	}
	
	return false;
}

int16 CPad::GetHandBrake(void)
{
	if ( ArePlayerControlsDisabled() )
		return 0;
	
	switch ( Mode )
	{
		case 0:
		case 1:
		{
			return NewState.RightShoulder1;
			
			break;
		}
		 
		case 2:
		{
			return NewState.Triangle;
			
			break;
		}
		
		case 3:
		{
			return NewState.LeftShoulder1;
			
			break;
		}
	}
	
	return 0;
}

int16 CPad::GetBrake(void)
{
	if ( ArePlayerControlsDisabled() )
		return 0;
	
	switch ( Mode )
	{
		case 0:
		case 2:
		{
			return NewState.Square;
			
			break;
		}
			
		case 1:
		{
			return NewState.Square;

			break;
		}
		
		case 3:
		{
			int16 axis = 2 * NewState.RightStickY;
			
			if ( axis < 0 )
				return 0;
			else
				return axis;
			
			break;
		}
	}
	
	return 0;
}

bool CPad::GetExitVehicle(void)
{
	if ( ArePlayerControlsDisabled() )
		return false;
	
	switch ( Mode )
	{
		case 0:
		case 1:
		case 3:
		{
			return !!NewState.Triangle;
			
			break;
		}
		
		case 2:
		{
			return !!NewState.LeftShoulder1;
			
			break;
		}
	}
	
	return false;
}

bool CPad::ExitVehicleJustDown(void)
{
	if ( ArePlayerControlsDisabled() )
		return false;
	
	switch ( Mode )
	{
		case 0:
		case 1:
		case 3:
		{
			return !!(NewState.Triangle && !OldState.Triangle);
			
			break;
		}
		
		case 2:
		{
			return !!(NewState.LeftShoulder1 && !OldState.LeftShoulder1);
			
			break;
		}
	}
	
	return false;
}

int32 CPad::GetWeapon(void)
{
	if ( ArePlayerControlsDisabled() )
		return false;
	
	switch ( Mode )
	{
		case 0:
		case 1:
		{
			return NewState.Circle;
			
			break;
		}
		
		case 2:
		{
			return NewState.Cross;
			
			break;
		}
		
		case 3:
		{
			return NewState.RightShoulder1;
			
			break;
		}
	}
	
	return false;
}

bool CPad::WeaponJustDown(void)
{
	if ( ArePlayerControlsDisabled() )
		return false;
	
	switch ( Mode )
	{
		case 0:
		case 1:
		{
			return !!(NewState.Circle && !OldState.Circle);
			
			break;
		}
		
		case 2:
		{
			return !!(NewState.Cross && !OldState.Cross);
			
			break;
		}
		
		case 3:
		{
			return !!(NewState.RightShoulder1 && !OldState.RightShoulder1);
			
			break;
		}
	}
	
	return false;
}

int16 CPad::GetAccelerate(void)
{
	if ( ArePlayerControlsDisabled() )
		return 0;
	
	switch ( Mode )
	{
		case 0:
		case 2:
		{
			return NewState.Cross;
			
			break;
		}
		
		case 1:
		{
			return NewState.Cross;
			
			break;
		}
		
		case 3:
		{
			int16 axis = -2 * NewState.RightStickY;
			
			if ( axis < 0 )
				return 0;
			else 
				return axis;
			
			break;
		}
	}
	
	return 0;
}

bool CPad::CycleCameraModeUpJustDown(void)
{
	switch ( Mode )
	{
		case 0:
		case 2:
		case 3:
		{
			return !!(NewState.Select && !OldState.Select);
			
			break;
		}
		
		case 1:
		{
			return !!(NewState.DPadUp && !OldState.DPadUp);
			
			break;
		}
	}
	
	return false;
}

bool CPad::CycleCameraModeDownJustDown(void)
{
	switch ( Mode )
	{
		case 0:
		case 2:
		case 3:
		{
			return false;
			
			break;
		}
		
		case 1:
		{
			return !!(NewState.DPadDown && !OldState.DPadDown);
			
			break;
		}
	}
	
	return false;
}

bool CPad::ChangeStationJustDown(void)
{
	if ( ArePlayerControlsDisabled() )
		return false;
	
	switch ( Mode )
	{
		case 0:
		{
			return !!(NewState.LeftShoulder1 && !OldState.LeftShoulder1);
			
			break;
		}
		
		case 1:
		{
			return !!(NewState.Select && !OldState.Select);
			
			break;
		}
		
		case 2:
		{
			return !!(NewState.LeftShock && !OldState.LeftShock);
			
			break;
		}
		
		case 3:
		{
			return !!(NewState.Circle && !OldState.Circle);
			
			break;
		}
	}
		
	return false;
}


bool CPad::CycleWeaponLeftJustDown(void)
{
	if ( ArePlayerControlsDisabled() )
		return false;
	
	return !!(NewState.LeftShoulder2 && !OldState.LeftShoulder2);
}

bool CPad::CycleWeaponRightJustDown(void)
{
	if ( ArePlayerControlsDisabled() )
		return false;
	
	return !!(NewState.RightShoulder2 && !OldState.RightShoulder2);
}

bool CPad::GetTarget(void)
{
	if ( ArePlayerControlsDisabled() )
		return false;
	
	switch ( Mode )
	{
		case 0:
		case 1:
		case 2:
		{
			return !!NewState.RightShoulder1;
			
			break;
		}
		
		case 3:
		{
			return !!NewState.LeftShoulder1;
			
			break;
		}
	}
	
	return false;
}

bool CPad::TargetJustDown(void)
{
	if ( ArePlayerControlsDisabled() )
		return false;
	
	switch ( Mode )
	{
		case 0:
		case 1:
		case 2:
		{
			return !!(NewState.RightShoulder1 && !OldState.RightShoulder1);
			
			break;
		}
		
		case 3:
		{
			return !!(NewState.LeftShoulder1 && !OldState.LeftShoulder1);
			
			break;
		}
	}
	
	return false;
}

bool CPad::JumpJustDown(void)
{
	if ( ArePlayerControlsDisabled() )
		return false;
	
	return !!(NewState.Square && !OldState.Square);
}

bool CPad::GetSprint(void)
{
	if ( ArePlayerControlsDisabled() )
		return false;
	
	switch ( Mode )
	{
		case 0:
		case 1:
		case 3:
		{
			return !!NewState.Cross;
			
			break;
		}
		
		case 2:
		{
			return !!NewState.Circle;
			
			break;
		}
	}
	
	return false;
}

bool CPad::ShiftTargetLeftJustDown(void)
{
	if ( ArePlayerControlsDisabled() )
		return false;
	
	return !!(NewState.LeftShoulder2 && !OldState.LeftShoulder2);
}

bool CPad::ShiftTargetRightJustDown(void)
{
	if ( ArePlayerControlsDisabled() )
		return false;
	
	return !!(NewState.RightShoulder2 && !OldState.RightShoulder2);
}

bool CPad::GetAnaloguePadUp(void)
{
	static int16 oldfStickY = 0;
	
	int16 Y = CPad::GetPad(0)->GetAnalogueUpDown();
	
	if ( Y < 0 && oldfStickY >= 0 )
	{
		oldfStickY = Y;
		return true;
	}
	else
	{
		oldfStickY = Y;
		return false;
	}
}

bool CPad::GetAnaloguePadDown(void)
{
	static int16 oldfStickY = 0;
	
	int16 Y = CPad::GetPad(0)->GetAnalogueUpDown();
	
	if ( Y > 0 && oldfStickY <= 0 )
	{
		oldfStickY = Y;
		return true;
	}
	else
	{
		oldfStickY = Y;
		return false;
	}
}

bool CPad::GetAnaloguePadLeft(void)
{
	static int16 oldfStickX = 0;
	
	int16 X = CPad::GetPad(0)->GetPedWalkLeftRight();
	
	if ( X < 0 && oldfStickX >= 0 )
	{
		oldfStickX = X;
		return true;
	}
	else
	{
		oldfStickX = X;
		return false;
	}
}

bool CPad::GetAnaloguePadRight(void)
{
	static int16 oldfStickX = 0;
	
	int16 X = CPad::GetPad(0)->GetPedWalkLeftRight();
	
	if ( X > 0 && oldfStickX <= 0 )
	{
		oldfStickX = X;
		return true;
	}
	else
	{
		oldfStickX = X;
		return false;
	}
}

bool CPad::GetAnaloguePadLeftJustUp(void)
{
	static int16 oldfStickX = 0;
	
	int16 X = GetPad(0)->GetPedWalkLeftRight();
	
	if ( X == 0 && oldfStickX < 0 )
	{
		oldfStickX = X;
		
		return true;
	}
	else
	{
		oldfStickX = X;
		
		return false;
	}
}

bool CPad::GetAnaloguePadRightJustUp(void)
{
	static int16 oldfStickX = 0;
	
	int16 X = GetPad(0)->GetPedWalkLeftRight();
	
	if ( X == 0 && oldfStickX > 0 )
	{
		oldfStickX = X;
		
		return true;
	}
	else
	{
		oldfStickX = X;
		
		return false;
	}
}

bool CPad::ForceCameraBehindPlayer(void)
{
	if ( ArePlayerControlsDisabled() )
		return false;
	
	switch ( Mode )
	{
		case 0:
		case 1:
		{
			return !!NewState.LeftShoulder1;
			
			break;
		}
		
		case 2:
		{
			return !!NewState.Triangle;
			
			break;
		}
		
		case 3:
		{
			return !!NewState.Circle;
			
			break;
		}
	}
	
	return false;
}

bool CPad::SniperZoomIn(void)
{
	if ( ArePlayerControlsDisabled() )
		return false;
	
	switch ( Mode )
	{
		case 0:
		case 1:
		case 3:
		{
			return !!NewState.Square;

			break;
		}
		
		case 2:
		{
			return !!NewState.Triangle;
			
			break;
		}
	}
	
	return false;
}

bool CPad::SniperZoomOut(void)
{
	if ( ArePlayerControlsDisabled() )
		return false;
	
	switch ( Mode )
	{
		case 0:
		case 1:
		case 3:
		{
			return !!NewState.Cross;

			break;
		}
		
		case 2:
		{
			return !!NewState.Square;
			
			break;
		}
	}
	
	return false;
}


int16 CPad::SniperModeLookLeftRight(void)
{
	int16 axis = NewState.LeftStickX;
	int16 dpad = (NewState.DPadRight - NewState.DPadLeft) / 2;
	
	if ( Abs(axis) > Abs(dpad) )
		return axis;
	else
		return dpad;
}

int16 CPad::SniperModeLookUpDown(void)
{
	int16 axis = NewState.LeftStickY;
	int16 dpad = (NewState.DPadUp - NewState.DPadDown) / 2;
	
	if ( Abs(axis) > Abs(dpad) )
		return axis;
	else
		return dpad;
}

int16 CPad::LookAroundLeftRight(void)
{
	float axis = GetPad(0)->NewState.RightStickX;
	
	if ( Abs(axis) > 85 && !GetLookBehindForPed() )
		return (int16) ( (axis + ( ( axis > 0 ) ? -85 : 85) )
							* (127.0f / 32.0f) ); // 3.96875f

	else if ( TheCamera.Cams[0].Using3rdPersonMouseCam() && Abs(axis) > 10 )
		return (int16) ( (axis + ( ( axis > 0 ) ? -10 : 10) )
							* (127.0f / 64.0f) ); // 1.984375f
	
	return 0;
}

int16 CPad::LookAroundUpDown(void)
{
	int16 axis = GetPad(0)->NewState.RightStickY;
	
	if ( Abs(axis) > 85 && !GetLookBehindForPed() )
		return (int16) ( (axis + ( ( axis > 0 ) ? -85 : 85) )
							* (127.0f / 32.0f) ); // 3.96875f

	else if ( TheCamera.Cams[0].Using3rdPersonMouseCam() && Abs(axis) > 40 )
		return (int16) ( (axis + ( ( axis > 0 ) ? -40 : 40) )
							* (127.0f / 64.0f) ); // 1.984375f

	return 0;
}


void CPad::ResetAverageWeapon(void)
{
	AverageWeapon = GetWeapon();
	AverageEntries = 1;
}

void CPad::PrintErrorMessage(void)
{
	if ( bDisplayNoControllerMessage && !CGame::playingIntro && !FrontEndMenuManager.m_bMenuActive )
	{
		CFont::SetScale(0.85f, 1.0f);
		CFont::SetJustifyOff();
		CFont::SetBackgroundOff();
		CFont::SetCentreSize(SCREEN_WIDTH - 20);
		CFont::SetCentreOn();
		CFont::SetPropOn();
		CFont::SetColor(CRGBA(255, 255, 200, 200));
		CFont::SetFontStyle(FONT_BANK);
		CFont::PrintString
		(
			SCREEN_WIDTH  / 2,
			SCREEN_HEIGHT / 2,
			TheText.Get("NOCONT") // Please reconnect an analog controller (DUALSHOCK@) or analog controller (DUALSHOCK@2). to controller port 1 to continue
		);
	}
	else if ( bObsoleteControllerMessage )
	{
		CFont::SetScale(0.85f, 1.0f);
		CFont::SetJustifyOff();
		CFont::SetBackgroundOff();
		CFont::SetCentreSize(SCREEN_WIDTH - 20);
		CFont::SetCentreOn();
		CFont::SetPropOn();
		CFont::SetColor(CRGBA(255, 255, 200, 200));
		CFont::SetFontStyle(FONT_BANK);
		CFont::PrintString
		(
			SCREEN_WIDTH  / 2,
			SCREEN_HEIGHT / 2,
			TheText.Get("WRCONT") // The controller connected to controller port 1 is an unsupported controller. Grand Theft Auto III requires an analog controller (DUALSHOCK@) or analog controller (DUALSHOCK@2).
		);
	}
  
}

void LittleTest(void)
{
	static int32 Cunt = 0;
	
	Cunt++; // ???
}

void CPad::ResetCheats(void)
{
	CWeather::ReleaseWeather();
	
	CPopulation::ms_bGivePedsWeapons = false;
	
	CPed::bNastyLimbsCheat = false;
	CPed::bPedCheat2 = false;
	CPed::bPedCheat3 = false;
	
	CVehicle::bWheelsOnlyCheat = false;
	CVehicle::bAllDodosCheat = false;
	CVehicle::bCheat3 = false;
	CVehicle::bCheat4 = false;
	CVehicle::bCheat5 = false;
	
	gbFastTime = false;
	CTimer::SetTimeScale(1.0f);
}

char *CPad::EditString(char *pStr, int32 nSize)
{
	int32 pos = strlen(pStr);
	
	// letters
	for ( int32 i = 0; i < ('Z' - 'A' + 1); i++ )
	{
		if ( GetPad(0)->GetCharJustDown(i + 'A') && pos < nSize - 1 )
		{
			pStr[pos++] = i + 'A';
			pStr[pos] = '\0';
		}
		
		if ( GetPad(0)->GetCharJustDown(i + 'a') && pos < nSize - 1 )
		{
			pStr[pos++] = i + 'a';
			pStr[pos] = '\0';
		}
	}
	
	// numbers 
	for ( int32 i = 0; i < ('9' - '0' + 1); i++ )
	{
		if ( GetPad(0)->GetCharJustDown(i + '0') && pos < nSize - 1 )
		{
			pStr[pos++] = i + '0';
			pStr[pos] = '\0';
		}
	}
	
	// space
	if ( GetPad(0)->GetCharJustDown(' ') && pos < nSize - 1 )
	{
		pStr[pos++] = ' ';
		pStr[pos] = '\0';
	}
	
	
	// del
	if ( GetPad(0)->GetDeleteJustDown() || GetPad(0)->GetBackspaceJustDown() )
	{
		if ( pos > 0 )
			pStr[pos - 1] = '\0';
	}

	// extenter/up/down
	if ( GetPad(0)->GetReturnJustDown() || GetPad(0)->GetUpJustDown() || GetPad(0)->GetDownJustDown() )
		return nil;
	
	return pStr;
}

int32 *CPad::EditCodesForControls(int32 *pRsKeys, int32 nSize)
{
	*pRsKeys = rsNULL;

	for ( int32 i = 0; i < 255; i++ )
	{
		if ( GetPad(0)->GetCharJustDown(i) )
			*pRsKeys = i;
	}
	
	for ( int32 i = 0; i < 12; i++ )
	{
		if ( GetPad(0)->GetFJustDown(i) )
			*pRsKeys = i + rsF1;
	}
	
	if ( GetPad(0)->GetEscapeJustDown() )
		*pRsKeys = rsESC;
	
	if ( GetPad(0)->GetInsertJustDown() )
		*pRsKeys = rsINS;
	
	if ( GetPad(0)->GetDeleteJustDown() )
		*pRsKeys = rsDEL;
	
	if ( GetPad(0)->GetHomeJustDown() )
		*pRsKeys = rsHOME;
	
	if ( GetPad(0)->GetEndJustDown() )
		*pRsKeys = rsEND;
	
	if ( GetPad(0)->GetPageUpJustDown() )
		*pRsKeys = rsPGUP;
	
	if ( GetPad(0)->GetPageDownJustDown() )
		*pRsKeys = rsPGDN;
	
	if ( GetPad(0)->GetUpJustDown() )
		*pRsKeys = rsUP;
	
	if ( GetPad(0)->GetDownJustDown() )
		*pRsKeys = rsDOWN;
	
	if ( GetPad(0)->GetLeftJustDown() )
		*pRsKeys = rsLEFT;
	
	if ( GetPad(0)->GetRightJustDown() )
		*pRsKeys = rsRIGHT;
	
	if ( GetPad(0)->GetScrollLockJustDown() )
		*pRsKeys = rsSCROLL;
	
	if ( GetPad(0)->GetPauseJustDown() )
		*pRsKeys = rsPAUSE;
	
	if ( GetPad(0)->GetNumLockJustDown() )
		*pRsKeys = rsNUMLOCK;
	
	if ( GetPad(0)->GetDivideJustDown() )
		*pRsKeys = rsDIVIDE;
	
	if ( GetPad(0)->GetTimesJustDown() )
		*pRsKeys = rsTIMES;
	
	if ( GetPad(0)->GetMinusJustDown() )
		*pRsKeys = rsMINUS;
	
	if ( GetPad(0)->GetPlusJustDown() )
		*pRsKeys = rsPLUS;

	if ( GetPad(0)->GetPadEnterJustDown() )
		*pRsKeys = rsPADENTER;

	if ( GetPad(0)->GetPadDelJustDown() )
		*pRsKeys = rsPADDEL;

	if ( GetPad(0)->GetPad1JustDown() )
		*pRsKeys = rsPADEND;
	
	if ( GetPad(0)->GetPad2JustDown() )
		*pRsKeys = rsPADDOWN;
	
	if ( GetPad(0)->GetPad3JustDown() )
		*pRsKeys = rsPADPGDN;
	
	if ( GetPad(0)->GetPad4JustDown() )
		*pRsKeys = rsPADLEFT;
	
	if ( GetPad(0)->GetPad5JustDown() )
		*pRsKeys = rsPAD5;
	
	if ( GetPad(0)->GetPad6JustDown() )
		*pRsKeys = rsPADRIGHT;
	
	if ( GetPad(0)->GetPad7JustDown() )
		*pRsKeys = rsPADHOME;
	
	if ( GetPad(0)->GetPad8JustDown() )
		*pRsKeys = rsPADUP;
	
	if ( GetPad(0)->GetPad9JustDown() )
		*pRsKeys = rsPADPGUP;
	
	if ( GetPad(0)->GetPad0JustDown() )
		*pRsKeys = rsPADINS;

	if ( GetPad(0)->GetBackspaceJustDown() )
		*pRsKeys = rsBACKSP;
	
	if ( GetPad(0)->GetTabJustDown() )
		*pRsKeys = rsTAB;
	
	if ( GetPad(0)->GetCapsLockJustDown() )
		*pRsKeys = rsCAPSLK;
	
	if ( GetPad(0)->GetReturnJustDown() )
		*pRsKeys = rsENTER;
	
	if ( GetPad(0)->GetLeftShiftJustDown() )
		*pRsKeys = rsLSHIFT;
	
	if ( GetPad(0)->GetShiftJustDown() )
		*pRsKeys = rsSHIFT;
	
	if ( GetPad(0)->GetRightShiftJustDown() )
		*pRsKeys = rsRSHIFT;
	
	if ( GetPad(0)->GetLeftCtrlJustDown() )
		*pRsKeys = rsLCTRL;
	
	if ( GetPad(0)->GetRightCtrlJustDown() )
		*pRsKeys = rsRCTRL;
	
	if ( GetPad(0)->GetLeftAltJustDown() )
		*pRsKeys = rsLALT;
	
	if ( GetPad(0)->GetRightAltJustDown() )
		*pRsKeys = rsRALT;
	
	if ( GetPad(0)->GetLeftWinJustDown() )
		*pRsKeys = rsLWIN;
	
	if ( GetPad(0)->GetRightWinJustDown() )
		*pRsKeys = rsRWIN;
	
	if ( GetPad(0)->GetAppsJustDown() )
		*pRsKeys = rsAPPS;
	
	return pRsKeys;
}

STARTPATCHES	
	InjectHook(0x4916C0, &CControllerState::Clear, PATCH_JUMP);
	InjectHook(0x491760, &CKeyboardState::Clear, PATCH_JUMP);
	InjectHook(0x491A10, &CPad::Clear, PATCH_JUMP);
	InjectHook(0x491B50, &CPad::ClearMouseHistory, PATCH_JUMP);
	//InjectHook(0x491B80, &CMouseControllerState::CMouseControllerState, PATCH_JUMP);
	InjectHook(0x491BB0, &CMouseControllerState::Clear, PATCH_JUMP);
	InjectHook(0x491BD0, &CMousePointerStateHelper::GetMouseSetUp, PATCH_JUMP);
	InjectHook(0x491CA0, &CPad::UpdateMouse, PATCH_JUMP);
	InjectHook(0x491E60, &CPad::ReconcileTwoControllersInput, PATCH_JUMP);
	InjectHook(0x492230, &CPad::StartShake, PATCH_JUMP);
	InjectHook(0x492290, &CPad::StartShake_Distance, PATCH_JUMP);
	InjectHook(0x492360, &CPad::StartShake_Train, PATCH_JUMP);
	InjectHook(0x492450, &CPad::AddToPCCheatString, PATCH_JUMP);
	InjectHook(0x492720, CPad::UpdatePads, PATCH_JUMP);
	InjectHook(0x492C60, &CPad::ProcessPCSpecificStuff, PATCH_JUMP);
	InjectHook(0x492C70, &CPad::Update, PATCH_JUMP);
#pragma warning( push )
#pragma warning( disable : 4573)
	InjectHook(0x492F00, (void (*)())CPad::DoCheats, PATCH_JUMP);
#pragma warning( pop )
	InjectHook(0x492F20, (void (CPad::*)(int16))&CPad::DoCheats, PATCH_JUMP);
	InjectHook(0x492F30, CPad::StopPadsShaking, PATCH_JUMP);
	InjectHook(0x492F50, &CPad::StopShaking, PATCH_JUMP);
	InjectHook(0x492F60, CPad::GetPad, PATCH_JUMP);
	InjectHook(0x492F70, &CPad::GetSteeringLeftRight, PATCH_JUMP);
	InjectHook(0x492FF0, &CPad::GetSteeringUpDown, PATCH_JUMP);
	InjectHook(0x493070, &CPad::GetCarGunUpDown, PATCH_JUMP);
	InjectHook(0x4930C0, &CPad::GetCarGunLeftRight, PATCH_JUMP);
	InjectHook(0x493110, &CPad::GetPedWalkLeftRight, PATCH_JUMP);
	InjectHook(0x493190, &CPad::GetPedWalkUpDown, PATCH_JUMP);
	InjectHook(0x493210, &CPad::GetAnalogueUpDown, PATCH_JUMP);
	InjectHook(0x493290, &CPad::GetLookLeft, PATCH_JUMP);
	InjectHook(0x4932C0, &CPad::GetLookRight, PATCH_JUMP);
	InjectHook(0x4932F0, &CPad::GetLookBehindForCar, PATCH_JUMP);
	InjectHook(0x493320, &CPad::GetLookBehindForPed, PATCH_JUMP);
	InjectHook(0x493350, &CPad::GetHorn, PATCH_JUMP);
	InjectHook(0x4933F0, &CPad::HornJustDown, PATCH_JUMP);
	InjectHook(0x493490, &CPad::GetCarGunFired, PATCH_JUMP);
	InjectHook(0x4934F0, &CPad::CarGunJustDown, PATCH_JUMP);
	InjectHook(0x493560, &CPad::GetHandBrake, PATCH_JUMP);
	InjectHook(0x4935A0, &CPad::GetBrake, PATCH_JUMP);
	InjectHook(0x4935F0, &CPad::GetExitVehicle, PATCH_JUMP);
	InjectHook(0x493650, &CPad::ExitVehicleJustDown, PATCH_JUMP);
	InjectHook(0x4936C0, &CPad::GetWeapon, PATCH_JUMP);
	InjectHook(0x493700, &CPad::WeaponJustDown, PATCH_JUMP);
	InjectHook(0x493780, &CPad::GetAccelerate, PATCH_JUMP);
	InjectHook(0x4937D0, &CPad::CycleCameraModeUpJustDown, PATCH_JUMP);
	InjectHook(0x493830, &CPad::CycleCameraModeDownJustDown, PATCH_JUMP);
	InjectHook(0x493870, &CPad::ChangeStationJustDown, PATCH_JUMP);
	InjectHook(0x493910, &CPad::CycleWeaponLeftJustDown, PATCH_JUMP);
	InjectHook(0x493940, &CPad::CycleWeaponRightJustDown, PATCH_JUMP);
	InjectHook(0x493970, &CPad::GetTarget, PATCH_JUMP);
	InjectHook(0x4939D0, &CPad::TargetJustDown, PATCH_JUMP);
	InjectHook(0x493A40, &CPad::JumpJustDown, PATCH_JUMP);
	InjectHook(0x493A70, &CPad::GetSprint, PATCH_JUMP);
	InjectHook(0x493AE0, &CPad::ShiftTargetLeftJustDown, PATCH_JUMP);
	InjectHook(0x493B10, &CPad::ShiftTargetRightJustDown, PATCH_JUMP);
	InjectHook(0x493B40, &CPad::GetAnaloguePadUp, PATCH_JUMP);
	InjectHook(0x493BA0, &CPad::GetAnaloguePadDown, PATCH_JUMP);
	InjectHook(0x493C00, &CPad::GetAnaloguePadLeft, PATCH_JUMP);
	InjectHook(0x493C60, &CPad::GetAnaloguePadRight, PATCH_JUMP);
	InjectHook(0x493CC0, &CPad::GetAnaloguePadLeftJustUp, PATCH_JUMP);
	InjectHook(0x493D20, &CPad::GetAnaloguePadRightJustUp, PATCH_JUMP);
	InjectHook(0x493D80, &CPad::ForceCameraBehindPlayer, PATCH_JUMP);
	InjectHook(0x493E00, &CPad::SniperZoomIn, PATCH_JUMP);
	InjectHook(0x493E70, &CPad::SniperZoomOut, PATCH_JUMP);
	InjectHook(0x493EE0, &CPad::SniperModeLookLeftRight, PATCH_JUMP);
	InjectHook(0x493F30, &CPad::SniperModeLookUpDown, PATCH_JUMP);
	InjectHook(0x493F80, &CPad::LookAroundLeftRight, PATCH_JUMP);
	InjectHook(0x494130, &CPad::LookAroundUpDown, PATCH_JUMP);
	InjectHook(0x494290, &CPad::ResetAverageWeapon, PATCH_JUMP);
	InjectHook(0x4942B0, CPad::PrintErrorMessage, PATCH_JUMP);
	InjectHook(0x494420, LittleTest, PATCH_JUMP);
	InjectHook(0x494450, CPad::ResetCheats, PATCH_JUMP);
	InjectHook(0x4944B0, CPad::EditString, PATCH_JUMP);
	InjectHook(0x494690, CPad::EditCodesForControls, PATCH_JUMP);

	//InjectHook(0x494E50, `global constructor keyed to'Pad.cpp, PATCH_JUMP);
	//InjectHook(0x494EB0, sub_494EB0, PATCH_JUMP);
	//InjectHook(0x494ED0, &CPad::~CPad, PATCH_JUMP);
	//InjectHook(0x494EE0, &CPad::CPad, PATCH_JUMP);
ENDPATCHES
