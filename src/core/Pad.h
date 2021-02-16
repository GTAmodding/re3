#pragma once

enum {
	PLAYERCONTROL_ENABLED = 0,
	PLAYERCONTROL_CAMERA = 1,
	PLAYERCONTROL_UNK2 = 2,
	PLAYERCONTROL_GARAGE = 4,
	PLAYERCONTROL_UNK8 = 8,
	PLAYERCONTROL_UNK10 = 16,
	PLAYERCONTROL_PLAYERINFO = 32,
	PLAYERCONTROL_PHONE = 64,
	PLAYERCONTROL_CUTSCENE = 128,
};

class CControllerState
{
public:
	int16 LeftStickX, LeftStickY;
	int16 RightStickX, RightStickY;
	int16 LeftShoulder1, LeftShoulder2;
	int16 RightShoulder1, RightShoulder2;
	int16 DPadUp, DPadDown, DPadLeft, DPadRight;
	int16 Start, Select;
	int16 Square, Triangle, Cross, Circle;
	int16 LeftShock, RightShock;
	int16 NetworkTalk;
	float GetLeftStickX(void) { return LeftStickX/32767.0f; };
	float GetLeftStickY(void) { return LeftStickY/32767.0f; };
	float GetRightStickX(void) { return RightStickX/32767.0f; };
	float GetRightStickY(void) { return RightStickY/32767.0f; };

	bool CheckForInput();
	void Clear(void);
};
VALIDATE_SIZE(CControllerState, 0x2A);

class CMouseControllerState
{
public:
	//uint32 btns;	// bit 0-2 button 1-3

	bool LMB;
	bool RMB;
	bool MMB;
	bool WHEELUP;
	bool WHEELDN;
	bool MXB1;
	bool MXB2;
	char _pad0;
	
	float x, y;
	
	CMouseControllerState();
	void Clear();
};

VALIDATE_SIZE(CMouseControllerState, 0x10);

class CMousePointerStateHelper
{
public:
	bool bInvertHorizontally;
	bool bInvertVertically;
	
	CMouseControllerState GetMouseSetUp();
};

VALIDATE_SIZE(CMousePointerStateHelper, 0x2);

extern CMousePointerStateHelper MousePointerStateHelper;


class CKeyboardState
{
public:
	int16 F[12];
	int16 VK_KEYS[256];
	int16 ESC;
	int16 INS;
	int16 DEL;
	int16 HOME;
	int16 END;
	int16 PGUP;
	int16 PGDN;
	int16 UP;
	int16 DOWN;
	int16 LEFT;
	int16 RIGHT;
	int16 SCROLLLOCK;
	int16 PAUSE;
	int16 NUMLOCK;
	int16 DIV;
	int16 MUL;
	int16 SUB;
	int16 ADD;
	int16 ENTER;
	int16 DECIMAL;
	int16 NUM1;
	int16 NUM2;
	int16 NUM3;
	int16 NUM4;
	int16 NUM5;
	int16 NUM6;
	int16 NUM7;
	int16 NUM8;
	int16 NUM9;
	int16 NUM0;
	int16 BACKSP;
	int16 TAB;
	int16 CAPSLOCK;
	int16 EXTENTER;
	int16 LSHIFT;
	int16 RSHIFT;
	int16 SHIFT;
	int16 LCTRL;
	int16 RCTRL;
	int16 LALT;
	int16 RALT;
	int16 LWIN;
	int16 RWIN;
	int16 APPS;
	
	void Clear();
};

VALIDATE_SIZE(CKeyboardState, 0x270);

enum
{
	// taken from miss2
	PAD1 = 0,
	PAD2,
	
	MAX_PADS
};

class CPad
{
public:
	enum
	{
		HORNHISTORY_SIZE = 5,
	};
	CControllerState NewState;
	CControllerState OldState;
	CControllerState PCTempKeyState;
	CControllerState PCTempJoyState;
	CControllerState PCTempMouseState;
	// straight out of my IDB
	int16 Phase;
	int16 Mode;
	int16 ShakeDur;
	uint8 ShakeFreq;
	bool bHornHistory[HORNHISTORY_SIZE];
	uint8 iCurrHornHistory;
	uint8 DisablePlayerControls;
	int8 bApplyBrakes;
	char CheatString[12];
	int32 LastTimeTouched;
	int32 AverageWeapon;
	int32 AverageEntries;

#ifdef DETECT_PAD_INPUT_SWITCH
	static bool IsAffectedByController;
#endif
	CPad() { }
	~CPad() { }

	static bool bDisplayNoControllerMessage;
	static bool bObsoleteControllerMessage;
	static bool bOldDisplayNoControllerMessage;
	static bool m_bMapPadOneToPadTwo;
#ifdef INVERT_LOOK_FOR_PAD
	static bool bInvertLook4Pad;
#endif
	
	static CKeyboardState OldKeyState;
	static CKeyboardState NewKeyState;
	static CKeyboardState TempKeyState;
	static char KeyBoardCheatString[20];
	static CMouseControllerState OldMouseControllerState;
	static CMouseControllerState NewMouseControllerState;
	static CMouseControllerState PCTempMouseControllerState;
	
	
#ifdef GTA_PS2_STUFF
	static void Initialise(void);
#endif
	void Clear(bool bResetPlayerControls);
	void ClearMouseHistory();
	void UpdateMouse();
	CControllerState ReconcileTwoControllersInput(CControllerState const &State1, CControllerState const &State2);
	void StartShake(int16 nDur, uint8 nFreq);
	void StartShake_Distance(int16 nDur, uint8 nFreq, float fX, float fY, float fz);
	void StartShake_Train(float fX, float fY);
#ifdef GTA_PS2_STUFF
	void AddToCheatString(char c);
#endif
	void AddToPCCheatString(char c);

	static void UpdatePads(void);
	void ProcessPCSpecificStuff(void);
	void Update(int16 pad);
	
	static void DoCheats(void);
	void DoCheats(int16 unk);
	
	static void StopPadsShaking(void);
	void StopShaking(int16 pad);
	
	static CPad *GetPad(int32 pad);
	
	int16 GetSteeringLeftRight(void);
	int16 GetSteeringUpDown(void);
	int16 GetCarGunUpDown(void);
	int16 GetCarGunLeftRight(void);
	int16 GetPedWalkLeftRight(void);
	int16 GetPedWalkUpDown(void);
	int16 GetAnalogueUpDown(void);
	bool GetLookLeft(void);
	bool GetLookRight(void);
	bool GetLookBehindForCar(void);
	bool GetLookBehindForPed(void);
	bool GetHorn(void);
	bool HornJustDown(void);
	bool GetCarGunFired(void);
	bool CarGunJustDown(void);
	int16 GetHandBrake(void);
	int16 GetBrake(void);
	bool GetExitVehicle(void);
	bool ExitVehicleJustDown(void);
	int32 GetWeapon(void);
	bool WeaponJustDown(void);
	int16 GetAccelerate(void);
	bool CycleCameraModeUpJustDown(void);
	bool CycleCameraModeDownJustDown(void);
	bool ChangeStationJustDown(void);
	bool CycleWeaponLeftJustDown(void);
	bool CycleWeaponRightJustDown(void);
	bool GetTarget(void);
	bool TargetJustDown(void);
	bool JumpJustDown(void);
	bool GetSprint(void);
	bool ShiftTargetLeftJustDown(void);
	bool ShiftTargetRightJustDown(void);
	bool GetAnaloguePadUp(void);
	bool GetAnaloguePadDown(void);
	bool GetAnaloguePadLeft(void);
	bool GetAnaloguePadRight(void);
	bool GetAnaloguePadLeftJustUp(void);
	bool GetAnaloguePadRightJustUp(void);
	bool ForceCameraBehindPlayer(void);
	bool SniperZoomIn(void);
	bool SniperZoomOut(void);
	int16 SniperModeLookLeftRight(void);
	int16 SniperModeLookUpDown(void);
	int16 LookAroundLeftRight(void);
	int16 LookAroundUpDown(void);
	void ResetAverageWeapon(void);
	static void PrintErrorMessage(void);
	static void ResetCheats(void);
	static char *EditString(char *pStr, int32 nSize);
	static int32 *EditCodesForControls(int32 *pRsKeys, int32 nSize);

#ifdef XINPUT
	static int XInputJoy1;
	static int XInputJoy2;
	void AffectFromXinput(uint32 pad);
#endif

	// mouse
	bool GetLeftMouseJustDown()           { return !!(NewMouseControllerState.LMB && !OldMouseControllerState.LMB); }
	bool GetRightMouseJustDown()          { return !!(NewMouseControllerState.RMB && !OldMouseControllerState.RMB); }
	bool GetMiddleMouseJustDown()         { return !!(NewMouseControllerState.MMB && !OldMouseControllerState.MMB); }
	bool GetMouseWheelUpJustDown()        { return !!(NewMouseControllerState.WHEELUP && !OldMouseControllerState.WHEELUP); }
	bool GetMouseWheelDownJustDown()      { return !!(NewMouseControllerState.WHEELDN && !OldMouseControllerState.WHEELDN);}
	bool GetMouseX1JustDown()             { return !!(NewMouseControllerState.MXB1    && !OldMouseControllerState.MXB1); }
	bool GetMouseX2JustDown()             { return !!(NewMouseControllerState.MXB2    && !OldMouseControllerState.MXB2); }

	bool GetLeftMouseJustUp() { return !!(!NewMouseControllerState.LMB && OldMouseControllerState.LMB); }
	bool GetRightMouseJustUp() { return !!(!NewMouseControllerState.RMB && OldMouseControllerState.RMB); }
	bool GetMiddleMouseJustUp() { return !!(!NewMouseControllerState.MMB && OldMouseControllerState.MMB); }
	bool GetMouseWheelUpJustUp() { return !!(!NewMouseControllerState.WHEELUP && OldMouseControllerState.WHEELUP); }
	bool GetMouseWheelDownJustUp() { return !!(!NewMouseControllerState.WHEELDN && OldMouseControllerState.WHEELDN); }
	bool GetMouseX1JustUp() { return !!(!NewMouseControllerState.MXB1 && OldMouseControllerState.MXB1); }
	bool GetMouseX2JustUp() { return !!(!NewMouseControllerState.MXB2 && OldMouseControllerState.MXB2); }

	bool GetLeftMouse()         { return NewMouseControllerState.LMB; }
	bool GetRightMouse()        { return NewMouseControllerState.RMB; }
	bool GetMiddleMouse()       { return NewMouseControllerState.MMB; }
	bool GetMouseWheelUp()      { return NewMouseControllerState.WHEELUP; }
	bool GetMouseWheelDown()    { return NewMouseControllerState.WHEELDN; }
	bool GetMouseX1()           { return NewMouseControllerState.MXB1; }
	bool GetMouseX2()           { return NewMouseControllerState.MXB2; }

	bool GetLeftMouseUp()       { return !OldMouseControllerState.LMB; }
	bool GetRightMouseUp()      { return !OldMouseControllerState.RMB; }
	bool GetMiddleMouseUp()     { return !OldMouseControllerState.MMB; }
	bool GetMouseWheelUpUp()    { return !OldMouseControllerState.WHEELUP; }
	bool GetMouseWheelDownUp()  { return !OldMouseControllerState.WHEELDN; }
	bool GetMouseX1Up()         { return !OldMouseControllerState.MXB1; }
	bool GetMouseX2Up()         { return !OldMouseControllerState.MXB2; }


	float GetMouseX() { return NewMouseControllerState.x; }
	float GetMouseY() { return NewMouseControllerState.y; }

	// keyboard
	
	bool GetCharJustDown(int32 c) { return !!(NewKeyState.VK_KEYS[c] && !OldKeyState.VK_KEYS[c]); }
	bool GetFJustDown(int32 n)    { return !!(NewKeyState.F[n] && !OldKeyState.F[n]); }
	bool GetEscapeJustDown()      { return !!(NewKeyState.ESC && !OldKeyState.ESC); }
	bool GetInsertJustDown()      { return !!(NewKeyState.INS && !OldKeyState.INS); }
	bool GetDeleteJustDown()      { return !!(NewKeyState.DEL && !OldKeyState.DEL); }
	bool GetHomeJustDown()        { return !!(NewKeyState.HOME && !OldKeyState.HOME); }
	bool GetEndJustDown()         { return !!(NewKeyState.END && !OldKeyState.END); }
	bool GetPageUpJustDown()      { return !!(NewKeyState.PGUP && !OldKeyState.PGUP); }
	bool GetPageDownJustDown()    { return !!(NewKeyState.PGDN && !OldKeyState.PGDN); }
	bool GetUpJustDown()          { return !!(NewKeyState.UP && !OldKeyState.UP); }
	bool GetDownJustDown()        { return !!(NewKeyState.DOWN && !OldKeyState.DOWN); }
	bool GetLeftJustDown()        { return !!(NewKeyState.LEFT && !OldKeyState.LEFT); }
	bool GetRightJustDown()       { return !!(NewKeyState.RIGHT && !OldKeyState.RIGHT); }
	bool GetScrollLockJustDown()  { return !!(NewKeyState.SCROLLLOCK && !OldKeyState.SCROLLLOCK); }
	bool GetPauseJustDown()       { return !!(NewKeyState.PAUSE && !OldKeyState.PAUSE); }
	bool GetNumLockJustDown()     { return !!(NewKeyState.NUMLOCK && !OldKeyState.NUMLOCK); }
	bool GetDivideJustDown()      { return !!(NewKeyState.DIV && !OldKeyState.DIV); }
	bool GetTimesJustDown()       { return !!(NewKeyState.MUL && !OldKeyState.MUL); }
	bool GetMinusJustDown()       { return !!(NewKeyState.SUB && !OldKeyState.SUB); }
	bool GetPlusJustDown()        { return !!(NewKeyState.ADD && !OldKeyState.ADD); }
	bool GetPadEnterJustDown()    { return !!(NewKeyState.ENTER && !OldKeyState.ENTER); }
	bool GetPadDelJustDown()      { return !!(NewKeyState.DECIMAL && !OldKeyState.DECIMAL); }
	bool GetPad1JustDown()        { return !!(NewKeyState.NUM1 && !OldKeyState.NUM1); }
	bool GetPad2JustDown()        { return !!(NewKeyState.NUM2 && !OldKeyState.NUM2); }
	bool GetPad3JustDown()        { return !!(NewKeyState.NUM3 && !OldKeyState.NUM3); }
	bool GetPad4JustDown()        { return !!(NewKeyState.NUM4 && !OldKeyState.NUM4); }
	bool GetPad5JustDown()        { return !!(NewKeyState.NUM5 && !OldKeyState.NUM5); }
	bool GetPad6JustDown()        { return !!(NewKeyState.NUM6 && !OldKeyState.NUM6); }
	bool GetPad7JustDown()        { return !!(NewKeyState.NUM7 && !OldKeyState.NUM7); }
	bool GetPad8JustDown()        { return !!(NewKeyState.NUM8 && !OldKeyState.NUM8); }
	bool GetPad9JustDown()        { return !!(NewKeyState.NUM9 && !OldKeyState.NUM9); }
	bool GetPad0JustDown()        { return !!(NewKeyState.NUM0 && !OldKeyState.NUM0); }
	bool GetBackspaceJustDown()   { return !!(NewKeyState.BACKSP && !OldKeyState.BACKSP); }
	bool GetTabJustDown()         { return !!(NewKeyState.TAB && !OldKeyState.TAB); }
	bool GetCapsLockJustDown()    { return !!(NewKeyState.CAPSLOCK && !OldKeyState.CAPSLOCK); }
	bool GetReturnJustDown()       { return !!(NewKeyState.EXTENTER && !OldKeyState.EXTENTER); }
	bool GetLeftShiftJustDown()   { return !!(NewKeyState.LSHIFT && !OldKeyState.LSHIFT); }
	bool GetShiftJustDown()       { return !!(NewKeyState.SHIFT && !OldKeyState.SHIFT); }
	bool GetRightShiftJustDown()  { return !!(NewKeyState.RSHIFT && !OldKeyState.RSHIFT); }
	bool GetLeftCtrlJustDown()    { return !!(NewKeyState.LCTRL && !OldKeyState.LCTRL); }
	bool GetRightCtrlJustDown()   { return !!(NewKeyState.RCTRL && !OldKeyState.RCTRL); }
	bool GetLeftAltJustDown()     { return !!(NewKeyState.LALT && !OldKeyState.LALT); }
	bool GetRightAltJustDown()    { return !!(NewKeyState.RALT && !OldKeyState.RALT); }
	bool GetLeftWinJustDown()     { return !!(NewKeyState.LWIN && !OldKeyState.LWIN); }
	bool GetRightWinJustDown()    { return !!(NewKeyState.RWIN && !OldKeyState.RWIN); }
	bool GetAppsJustDown()        { return !!(NewKeyState.APPS && !OldKeyState.APPS); }
	bool GetEnterJustDown()       { return GetPadEnterJustDown() || GetReturnJustDown(); }
	bool GetAltJustDown()         { return GetLeftAltJustDown() || GetRightAltJustDown(); }
								  
	bool GetLeftJustUp() { return !!(!NewKeyState.LEFT && OldKeyState.LEFT); }
	bool GetRightJustUp() { return !!(!NewKeyState.RIGHT && OldKeyState.RIGHT); }
	bool GetEnterJustUp() { return GetPadEnterJustUp() || GetReturnJustUp(); }
	bool GetReturnJustUp() { return !!(!NewKeyState.EXTENTER && OldKeyState.EXTENTER); }
	bool GetPadEnterJustUp() { return !!(!NewKeyState.ENTER && OldKeyState.ENTER); }

	bool GetChar(int32 c)         { return NewKeyState.VK_KEYS[c]; }
	bool GetF(int32 n)            { return NewKeyState.F[n]; }
	bool GetEscape()              { return NewKeyState.ESC; }
	bool GetInsert()              { return NewKeyState.INS; }
	bool GetDelete()              { return NewKeyState.DEL; }
	bool GetHome()                { return NewKeyState.HOME; }
	bool GetEnd()                 { return NewKeyState.END; }
	bool GetPageUp()              { return NewKeyState.PGUP; }
	bool GetPageDown()            { return NewKeyState.PGDN; }
	bool GetUp()                  { return NewKeyState.UP; }
	bool GetDown()                { return NewKeyState.DOWN; }
	bool GetLeft()                { return NewKeyState.LEFT; }
	bool GetRight()               { return NewKeyState.RIGHT; }
	bool GetScrollLock()          { return NewKeyState.SCROLLLOCK; }
	bool GetPause()               { return NewKeyState.PAUSE; }
	bool GetNumLock()             { return NewKeyState.NUMLOCK; }
	bool GetDivide()              { return NewKeyState.DIV; }
	bool GetTimes()               { return NewKeyState.MUL; }
	bool GetMinus()               { return NewKeyState.SUB; }
	bool GetPlus()                { return NewKeyState.ADD; }
	bool GetPadEnter()            { return NewKeyState.ENTER; } //  GetEnterJustDown
	bool GetPadDel()              { return NewKeyState.DECIMAL; }
	bool GetPad1()                { return NewKeyState.NUM1; }
	bool GetPad2()                { return NewKeyState.NUM2; }
	bool GetPad3()                { return NewKeyState.NUM3; }
	bool GetPad4()                { return NewKeyState.NUM4; }
	bool GetPad5()                { return NewKeyState.NUM5; }
	bool GetPad6()                { return NewKeyState.NUM6; }
	bool GetPad7()                { return NewKeyState.NUM7; }
	bool GetPad8()                { return NewKeyState.NUM8; }
	bool GetPad9()                { return NewKeyState.NUM9; }
	bool GetPad0()                { return NewKeyState.NUM0; }
	bool GetBackspace()           { return NewKeyState.BACKSP; }
	bool GetTab()                 { return NewKeyState.TAB; }
	bool GetCapsLock()            { return NewKeyState.CAPSLOCK; }
	bool GetEnter()               { return NewKeyState.EXTENTER; }
	bool GetLeftShift()           { return NewKeyState.LSHIFT; }
	bool GetShift()               { return NewKeyState.SHIFT; }
	bool GetRightShift()          { return NewKeyState.RSHIFT; }
	bool GetLeftCtrl()            { return NewKeyState.LCTRL; }
	bool GetRightCtrl()           { return NewKeyState.RCTRL; }
	bool GetLeftAlt()             { return NewKeyState.LALT; }
	bool GetRightAlt()            { return NewKeyState.RALT; }
	bool GetLeftWin()             { return NewKeyState.LWIN; }
	bool GetRightWin()            { return NewKeyState.RWIN; }
	bool GetApps()                { return NewKeyState.APPS; }
	// pad

	bool GetTriangleJustDown()       { return !!(NewState.Triangle && !OldState.Triangle); }
	bool GetCircleJustDown()         { return !!(NewState.Circle && !OldState.Circle); }
	bool GetCrossJustDown()          { return !!(NewState.Cross && !OldState.Cross); }
	bool GetSquareJustDown()         { return !!(NewState.Square && !OldState.Square); }
	bool GetDPadUpJustDown()         { return !!(NewState.DPadUp && !OldState.DPadUp); }
	bool GetDPadDownJustDown()       { return !!(NewState.DPadDown && !OldState.DPadDown); }
	bool GetDPadLeftJustDown()       { return !!(NewState.DPadLeft && !OldState.DPadLeft); }
	bool GetDPadRightJustDown()      { return !!(NewState.DPadRight && !OldState.DPadRight); }
	bool GetLeftShoulder1JustDown()  { return !!(NewState.LeftShoulder1 && !OldState.LeftShoulder1); }
	bool GetLeftShoulder2JustDown()  { return !!(NewState.LeftShoulder2 && !OldState.LeftShoulder2); }
	bool GetRightShoulder1JustDown() { return !!(NewState.RightShoulder1 && !OldState.RightShoulder1); }
	bool GetRightShoulder2JustDown() { return !!(NewState.RightShoulder2 && !OldState.RightShoulder2); }
	bool GetLeftShockJustDown()      { return !!(NewState.LeftShock && !OldState.LeftShock); }
	bool GetRightShockJustDown()     { return !!(NewState.RightShock && !OldState.RightShock); }
	bool GetStartJustDown()          { return !!(NewState.Start && !OldState.Start); }
	bool GetLeftStickXJustDown() { return !!(NewState.LeftStickX && !OldState.LeftStickX); }
	bool GetLeftStickYJustDown() { return !!(NewState.LeftStickY && !OldState.LeftStickY); }
  
	bool GetTriangleJustUp() { return !!(!NewState.Triangle && OldState.Triangle); }
	bool GetCircleJustUp() { return !!(!NewState.Circle && OldState.Circle); }
	bool GetCrossJustUp() { return !!(!NewState.Cross && OldState.Cross); }
	bool GetSquareJustUp() { return !!(!NewState.Square && OldState.Square); }
	bool GetDPadUpJustUp() { return !!(!NewState.DPadUp && OldState.DPadUp); }
	bool GetDPadDownJustUp() { return !!(!NewState.DPadDown && OldState.DPadDown); }
	bool GetDPadLeftJustUp() { return !!(!NewState.DPadLeft && OldState.DPadLeft); }
	bool GetDPadRightJustUp() { return !!(!NewState.DPadRight && OldState.DPadRight); }

	bool GetTriangle()           { return !!NewState.Triangle; }
	bool GetCircle()             { return !!NewState.Circle; }
	bool GetCross()              { return !!NewState.Cross; }
	bool GetSquare()             { return !!NewState.Square; }
	bool GetDPadUp()             { return !!NewState.DPadUp; }
	bool GetDPadDown()           { return !!NewState.DPadDown; }
	bool GetDPadLeft()           { return !!NewState.DPadLeft; }
	bool GetDPadRight()          { return !!NewState.DPadRight; }
	bool GetLeftShoulder1(void)  { return !!NewState.LeftShoulder1; }
	bool GetLeftShoulder2(void)  { return !!NewState.LeftShoulder2; }
	bool GetRightShoulder1(void) { return !!NewState.RightShoulder1; }
	bool GetRightShoulder2(void) { return !!NewState.RightShoulder2; }
	bool GetStart()              { return !!NewState.Start; }
	int16 GetLeftStickX(void)    { return NewState.LeftStickX; }
	int16 GetLeftStickY(void)    { return NewState.LeftStickY; }
	int16 GetRightStickX(void)    { return NewState.RightStickX; }
	int16 GetRightStickY(void)    { return NewState.RightStickY; }

	bool ArePlayerControlsDisabled(void) { return DisablePlayerControls != PLAYERCONTROL_ENABLED; }
	void SetDisablePlayerControls(uint8 who) { DisablePlayerControls |= who; }
	void SetEnablePlayerControls(uint8 who) { DisablePlayerControls &= ~who; }
	bool IsPlayerControlsDisabledBy(uint8 who) { return DisablePlayerControls & who; }
	
	int16 GetMode() { return Mode; }
	void SetMode(int16 mode) { Mode = mode; }
	
	static bool IsNoOrObsolete() { return bDisplayNoControllerMessage || bObsoleteControllerMessage; }
};

VALIDATE_SIZE(CPad, 0xFC);
extern CPad Pads[MAX_PADS];

#ifdef ALLCARSHELI_CHEAT
extern bool bAllCarCheat;
#endif
