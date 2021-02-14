#if defined RW_D3D9 || defined RWLIBS || defined __MWERKS__

#define _WIN32_WINDOWS 0x0500
#define WINVER 0x0500

#include <winerror.h>
#include <windows.h>
#include <mmsystem.h>
#include <shellapi.h>

#include <windowsx.h>
#include <basetsd.h>

#include <regstr.h>
#include <shlobj.h>

#include <dbt.h>

#pragma warning( push )
#pragma warning( disable : 4005)

#ifdef __MWERKS__
#define MAPVK_VK_TO_CHAR (2) // this is missing from codewarrior win32 headers - but it gets used ... how?
#endif

#include <ddraw.h>
#include <DShow.h>
#pragma warning( pop )

#define WM_GRAPHNOTIFY	WM_USER+13

#ifndef USE_D3D9
#pragma comment( lib, "d3d8.lib" )
#endif
#pragma comment( lib, "ddraw.lib" )
#pragma comment( lib, "Winmm.lib" )
#pragma comment( lib, "dxguid.lib" )
#pragma comment( lib, "strmiids.lib" )
#pragma comment( lib, "dinput8.lib" )

#define WITHD3D
#define WITHDINPUT
#include "common.h"
#if (defined(_MSC_VER))
#include <tchar.h>
#endif /* (defined(_MSC_VER)) */
#include <stdio.h>
#include "rwcore.h"
#include "resource.h"
#include "skeleton.h"
#include "platform.h"
#include "crossplatform.h"

#define MAX_SUBSYSTEMS		(16)


static RwBool		  ForegroundApp = TRUE;

static RwBool		  RwInitialised = FALSE;

static RwSubSystemInfo GsubSysInfo[MAX_SUBSYSTEMS];
static RwInt32		GnumSubSystems = 0;
static RwInt32		GcurSel = 0, GcurSelVM = 0;

static RwBool startupDeactivate;

static RwBool useDefault;

/* Class name for the MS Window's window class. */

static const RwChar *AppClassName = RWSTRING("Grand theft auto 3");

static psGlobalType PsGlobal;


#define PSGLOBAL(var) (((psGlobalType *)(RsGlobal.ps))->var)

#undef MAKEPOINTS
#define MAKEPOINTS(l)		(*((POINTS /*FAR*/ *)&(l)))

#define SAFE_RELEASE(x) { if (x) x->Release(); x = NULL; }
#define JIF(x) if (FAILED(hr=(x))) \
	{debug(TEXT("FAILED(hr=0x%x) in ") TEXT(#x) TEXT("\n"), hr); return;}

#include "main.h"
#include "FileMgr.h"
#include "Text.h"
#include "Pad.h"
#include "Timer.h"
#include "DMAudio.h"
#include "ControllerConfig.h"
#include "Frontend.h"
#include "Game.h"
#include "PCSave.h"
#include "AnimViewer.h"
#include "MemoryMgr.h"

#ifdef PS2_MENU
#include "MemoryCard.h"
#include "Font.h"
#endif
	
VALIDATE_SIZE(psGlobalType, 0x28);

// DirectShow interfaces
IGraphBuilder *pGB = nil;
IMediaControl *pMC = nil;
IMediaEventEx *pME = nil;
IVideoWindow  *pVW = nil;
IMediaSeeking *pMS = nil;

DWORD dwDXVersion;
SIZE_T _dwMemTotalPhys;
size_t _dwMemAvailPhys;
SIZE_T _dwMemTotalVirtual;
SIZE_T _dwMemAvailVirtual;
DWORD _dwMemTotalVideo;
DWORD _dwMemAvailVideo;
DWORD _dwOperatingSystemVersion;

RwUInt32 gGameState;
CJoySticks AllValidWinJoys;

#ifdef DETECT_JOYSTICK_MENU
char gSelectedJoystickName[128] = "";
#endif

// What is that for anyway?
#ifndef IMPROVED_VIDEOMODE
static RwBool defaultFullscreenRes = TRUE;
#else
static RwBool defaultFullscreenRes = FALSE;
static RwInt32 bestWndMode = -1;
#endif

CJoySticks::CJoySticks()
{
	for (int i = 0; i < MAX_JOYSTICKS; i++)
	{
		ClearJoyInfo(i);
	}
}

void CJoySticks::ClearJoyInfo(int joyID)
{
	m_aJoys[joyID].m_State = JOYPAD_UNUSED;
	m_aJoys[joyID].m_bInitialised = false;
	m_aJoys[joyID].m_bHasAxisZ = false;
	m_aJoys[joyID].m_bHasAxisR = false;
}



/*
 *****************************************************************************
 */
void _psCreateFolder(LPCSTR path)
{
	HANDLE hfle = CreateFile(path, GENERIC_READ, 
									FILE_SHARE_READ,
									nil,
									OPEN_EXISTING,
									FILE_FLAG_BACKUP_SEMANTICS | FILE_ATTRIBUTE_NORMAL,
									nil);

	if ( hfle == INVALID_HANDLE_VALUE )
		CreateDirectory(path, nil);
	else
		CloseHandle(hfle);
}

/*
 *****************************************************************************
 */
const char *_psGetUserFilesFolder()
{
#ifdef USE_MY_DOCUMENTS
	HKEY hKey = NULL;

	static CHAR szUserFiles[256];

	if ( RegOpenKeyEx(HKEY_CURRENT_USER,
						REGSTR_PATH_SPECIAL_FOLDERS,
						REG_OPTION_RESERVED,
						KEY_READ,
						&hKey) == ERROR_SUCCESS )
	{
		DWORD KeyType;
		DWORD KeycbData = sizeof(szUserFiles);
		if ( RegQueryValueEx(hKey,
							"Personal",
							NULL,
							&KeyType,
							(LPBYTE)szUserFiles,
							&KeycbData) == ERROR_SUCCESS )
		{
			RegCloseKey(hKey);
			strcat(szUserFiles, "\\GTA3 User Files");
			_psCreateFolder(szUserFiles);
			return szUserFiles;
		}	

		RegCloseKey(hKey);		
	}
	
	strcpy(szUserFiles, "data");
	return szUserFiles;
#else
	static CHAR szUserFiles[256];
	strcpy(szUserFiles, "userfiles");
	_psCreateFolder(szUserFiles);
	return szUserFiles;
#endif
}

/*
 *****************************************************************************
 */
RwBool
psCameraBeginUpdate(RwCamera *camera)
{
	if ( !RwCameraBeginUpdate(Scene.camera) )
	{
		ForegroundApp = FALSE;
		RsEventHandler(rsACTIVATE, (void *)FALSE);
		return FALSE;
	}
	
	return TRUE;
}

/*
 *****************************************************************************
 */
void
psCameraShowRaster(RwCamera *camera)
{
	if (CMenuManager::m_PrefsVsync)
		RwCameraShowRaster(camera, PSGLOBAL(window), rwRASTERFLIPWAITVSYNC);
	else
		RwCameraShowRaster(camera, PSGLOBAL(window), rwRASTERFLIPDONTWAIT);

	return;
}


/*
 *****************************************************************************
 */
RwImage *
psGrabScreen(RwCamera *pCamera)
{
#ifndef LIBRW
	RwRaster *pRaster = RwCameraGetRaster(pCamera);
	if (RwImage *pImage = RwImageCreate(pRaster->width, pRaster->height, 32)) {
		RwImageAllocatePixels(pImage);
		RwImageSetFromRaster(pImage, pRaster);
		return pImage;
	}
#else
	rw::Image *image = RwCameraGetRaster(pCamera)->toImage();
	image->removeMask();
	if(image)
		return image;
#endif
	return nil;
}

/*
 *****************************************************************************
 */
RwUInt32
psTimer(void)
{
	RwUInt32 time;

	TIMECAPS TimeCaps;
	
	timeGetDevCaps(&TimeCaps, sizeof(TIMECAPS));
	
	timeBeginPeriod(TimeCaps.wPeriodMin);
	
	time = (RwUInt32) timeGetTime();

	timeEndPeriod(TimeCaps.wPeriodMin);
	
	return time;
}

/*
 *****************************************************************************
 */
void
psMouseSetPos(RwV2d *pos)
{
	POINT point;

	point.x = (RwInt32) pos->x;
	point.y = (RwInt32) pos->y;

	ClientToScreen(PSGLOBAL(window), &point);

	SetCursorPos(point.x, point.y);
	
	PSGLOBAL(lastMousePos.x) = (RwInt32)pos->x;

	PSGLOBAL(lastMousePos.y) = (RwInt32)pos->y;

	return;
}

/*
 *****************************************************************************
 */
RwMemoryFunctions*
psGetMemoryFunctions(void)
{
#ifdef USE_CUSTOM_ALLOCATOR
	return &memFuncs;
#else
	return nil;
#endif
}

/*
 *****************************************************************************
 */
RwBool
psInstallFileSystem(void)
{
	return (TRUE);
}


/*
 *****************************************************************************
 */
RwBool
psNativeTextureSupport(void)
{
	return RwD3D8DeviceSupportsDXTTexture();
}

/*
 *****************************************************************************
 */
static HWND
InitInstance(HANDLE instance)
{
	/*
	 * Perform any necessary initialization for this instance of the 
	 * application.
	 *
	 * Create the MS Window's window instance for this application. The
	 * initial window size is given by the defined camera size. The window 
	 * is not given a title as we set it during Init3D() with information 
	 * about the version of RenderWare being used.
	 */

	RECT rect;

	rect.left = rect.top = 0;
	rect.right = RsGlobal.maximumWidth;
	rect.bottom = RsGlobal.maximumHeight;

	AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

	return CreateWindow(AppClassName, RsGlobal.appName,
						WS_OVERLAPPEDWINDOW,
						CW_USEDEFAULT, CW_USEDEFAULT,
						rect.right - rect.left, rect.bottom - rect.top,
						(HWND)nil, (HMENU)nil, (HINSTANCE)instance, nil);
}

void _GetVideoMemInfo(LPDWORD total, LPDWORD avaible)
{
	HRESULT hr;
	LPDIRECTDRAW7 pDD7;
	
	hr = DirectDrawCreateEx(nil, (VOID**)&pDD7, IID_IDirectDraw7, nil);
	
	if ( FAILED(hr) )
		return;
	
	DDSCAPS2 caps;
	
	ZeroMemory(&caps, sizeof(DDSCAPS2));
	caps.dwCaps = DDSCAPS_VIDEOMEMORY;
	
	pDD7->GetAvailableVidMem(&caps, total, avaible);
	
	pDD7->Release();
}

/*
 *****************************************************************************
 */
typedef HRESULT(WINAPI * DIRECTDRAWCREATEEX)( GUID*, VOID**, REFIID, IUnknown* );


//-----------------------------------------------------------------------------
// Name: GetDXVersion()
// Desc: This function returns the DirectX version number as follows:
//			0x0000 = No DirectX installed
//			0x0700 = At least DirectX 7 installed.
//			0x0800 = At least DirectX 8 installed.
// 
//		 Please note that this code is intended as a general guideline. Your
//		 app will probably be able to simply query for functionality (via
//		 QueryInterface) for one or two components.
//
//		 Please also note:
//			"if( dwDXVersion != 0x500 ) return FALSE;" is VERY BAD. 
//			"if( dwDXVersion <	0x500 ) return FALSE;" is MUCH BETTER.
//		 to ensure your app will run on future releases of DirectX.
//-----------------------------------------------------------------------------
DWORD GetDXVersion()
{
	DIRECTDRAWCREATEEX	 DirectDrawCreateEx = NULL;
	HINSTANCE			 hDDrawDLL			= nil;
	HINSTANCE			 hD3D8DLL			= nil;
	HINSTANCE			 hDPNHPASTDLL		= NULL;
	DWORD				 dwDXVersion		= 0;
	//HRESULT			   hr;

	// First see if DDRAW.DLL even exists.
	hDDrawDLL = LoadLibrary( "DDRAW.DLL" );
	if( hDDrawDLL == nil )
	{
		dwDXVersion = 0;
		OutputDebugString( "Couldn't LoadLibrary DDraw\r\n" );
		return dwDXVersion;
	}


	//-------------------------------------------------------------------------
	// DirectX 7.0 Checks
	//-------------------------------------------------------------------------

	// Check for DirectX 7 by creating a DDraw7 object
	LPDIRECTDRAW7 pDD7;
	DirectDrawCreateEx = (DIRECTDRAWCREATEEX)GetProcAddress( hDDrawDLL,
													   "DirectDrawCreateEx" );
	if( nil == DirectDrawCreateEx )
	{
		FreeLibrary( hDDrawDLL );
		OutputDebugString( "Couldn't GetProcAddress DirectDrawCreateEx\r\n" );
		return dwDXVersion;
	}

	if( FAILED( DirectDrawCreateEx( nil, (VOID**)&pDD7, IID_IDirectDraw7,
									nil ) ) )
	{
		FreeLibrary( hDDrawDLL );
		OutputDebugString( "Couldn't DirectDrawCreateEx\r\n" );
		return dwDXVersion;
	}

	// DDraw7 was created successfully. We must be at least DX7.0
	dwDXVersion = 0x700;
	pDD7->Release();

#ifdef USE_D3D9
	HINSTANCE hD3D9DLL = LoadLibrary("D3D9.DLL");
	if (hD3D9DLL != nil) {
		FreeLibrary(hDDrawDLL);
		FreeLibrary(hD3D9DLL);

		dwDXVersion = 0x900;
		return dwDXVersion;
	}
#endif

	//-------------------------------------------------------------------------
	// DirectX 8.0 Checks
	//-------------------------------------------------------------------------

	// Simply see if D3D8.dll exists.
	hD3D8DLL = LoadLibrary( "D3D8.DLL" );
	if( hD3D8DLL == nil )
	{
		FreeLibrary( hDDrawDLL );
		OutputDebugString( "Couldn't LoadLibrary D3D8.DLL\r\n" );
		return dwDXVersion;
	}

	// D3D8.dll exists. We must be at least DX8.0
	dwDXVersion = 0x800;


	//-------------------------------------------------------------------------
	// DirectX 8.1 Checks
	//-------------------------------------------------------------------------

	// Simply see if dpnhpast.dll exists.
	hDPNHPASTDLL = LoadLibrary( "dpnhpast.dll" );
	if( hDPNHPASTDLL == nil )
	{
		FreeLibrary( hDPNHPASTDLL );
		OutputDebugString( "Couldn't LoadLibrary dpnhpast.dll\r\n" );
		return dwDXVersion;
	}

	// dpnhpast.dll exists. We must be at least DX8.1
	dwDXVersion = 0x801;


	//-------------------------------------------------------------------------
	// End of checking for versions of DirectX 
	//-------------------------------------------------------------------------

	// Close open libraries and return
	FreeLibrary( hDDrawDLL );
	FreeLibrary( hD3D8DLL );
	
	return dwDXVersion;
}

/*
 *****************************************************************************
 */
#ifndef _WIN64
static char cpuvendor[16] = "UnknownVendr";
__declspec(naked)  const char * _psGetCpuVendr()
{
	__asm
	{
		push	ebx
		xor		eax, eax
		cpuid
		mov		dword ptr [cpuvendor+0], ebx
		mov		dword ptr [cpuvendor+4], edx
		mov		dword ptr [cpuvendor+8], ecx
		mov		eax, offset cpuvendor
		pop		ebx
		retn
	}
}

/*
 *****************************************************************************
 */
__declspec(naked) RwUInt32 _psGetCpuFeatures()
{
	__asm
	{
		mov		eax, 1
		cpuid
		mov		eax, edx
		retn
	}
}

/*
 *****************************************************************************
 */
__declspec(naked) RwUInt32 _psGetCpuFeaturesEx()
{
	__asm
	{
		mov		eax, 80000000h
		cpuid

		cmp		eax, 80000000h
		jbe		short _NOEX

		mov		eax, 80000001h
		cpuid

		mov		eax, edx
		jmp		short _RETEX

_NOEX:
		xor		eax, eax
		mov		eax, eax
		
_RETEX:
		retn   
	}
}

void _psPrintCpuInfo()
{
	RwUInt32 features	= _psGetCpuFeatures();
	RwUInt32 FeaturesEx = _psGetCpuFeaturesEx();

	debug("Running on a %s", _psGetCpuVendr());

	if ( features & 0x800000 )
		debug("with MMX");
	if ( features & 0x2000000 )
		debug("with SSE");
	if ( FeaturesEx & 0x80000000 )
		debug("with 3DNow");
}
#endif

/*
 *****************************************************************************
 */
#ifdef UNDER_CE
#define CMDSTR	LPWSTR
#else
#define CMDSTR	LPSTR
#endif

/*
 *****************************************************************************
 */
RwBool
psInitialize(void)
{
	PsGlobal.lastMousePos.x = PsGlobal.lastMousePos.y = 0.0f;

	RsGlobal.ps = &PsGlobal;
	
	PsGlobal.fullScreen = FALSE;
	
	PsGlobal.dinterface = nil;
	PsGlobal.mouse	   = nil;
	PsGlobal.joy1	= nil;
	PsGlobal.joy2	= nil;

	CFileMgr::Initialise();

#ifdef PS2_MENU
	CPad::Initialise();
	CPad::GetPad(0)->Mode = 0;
	
	CGame::frenchGame = false;
	CGame::germanGame = false;
	CGame::nastyGame = true;
	CMenuManager::m_PrefsAllowNastyGame = true;
	
	WORD lang	= PRIMARYLANGID(GetSystemDefaultLCID());
	if ( lang  == LANG_ITALIAN )
		CMenuManager::m_PrefsLanguage = CMenuManager::LANGUAGE_ITALIAN;
	else if ( lang  == LANG_SPANISH )
		CMenuManager::m_PrefsLanguage = CMenuManager::LANGUAGE_SPANISH;
	else if ( lang  == LANG_GERMAN )
	{
		CGame::germanGame = true;
		CGame::nastyGame = false;
		CMenuManager::m_PrefsAllowNastyGame = false;
		CMenuManager::m_PrefsLanguage = CMenuManager::LANGUAGE_GERMAN;
	}
	else if ( lang  == LANG_FRENCH )
	{
		CGame::frenchGame = true;
		CGame::nastyGame = false;
		CMenuManager::m_PrefsAllowNastyGame = false;
		CMenuManager::m_PrefsLanguage = CMenuManager::LANGUAGE_FRENCH;
	}
	else
		CMenuManager::m_PrefsLanguage = CMenuManager::LANGUAGE_AMERICAN;
	
	FrontEndMenuManager.InitialiseMenuContentsAfterLoadingGame();
	
	TheMemoryCard.Init();
#else
	C_PcSave::SetSaveDirectory(_psGetUserFilesFolder());
	
	InitialiseLanguage();
#if GTA_VERSION < GTA3_PC_11
	FrontEndMenuManager.LoadSettings();
#endif

#endif
	
	gGameState = GS_START_UP;
	TRACE("gGameState = GS_START_UP");
#ifndef _WIN64
	_psPrintCpuInfo();
#endif
	OSVERSIONINFO verInfo;
	verInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	
	GetVersionEx(&verInfo);
	
	_dwOperatingSystemVersion = OS_WIN95;
	
	if ( verInfo.dwPlatformId == VER_PLATFORM_WIN32_NT )
	{
		if ( verInfo.dwMajorVersion == 4 )
		{
			debug("Operating System is WinNT\n");
			_dwOperatingSystemVersion = OS_WINNT;
		}
		else if ( verInfo.dwMajorVersion == 5 )
		{
			debug("Operating System is Win2000\n");
			_dwOperatingSystemVersion = OS_WIN2000;
		}
		else if ( verInfo.dwMajorVersion > 5 )
		{
			debug("Operating System is WinXP or greater\n");
			_dwOperatingSystemVersion = OS_WINXP;
		}
	}
	else if ( verInfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS )
	{
		if ( verInfo.dwMajorVersion > 4 || verInfo.dwMajorVersion == 4 && verInfo.dwMinorVersion != 0 )
		{
			debug("Operating System is Win98\n");
			_dwOperatingSystemVersion = OS_WIN98;
		}
		else
		{
			debug("Operating System is Win95\n");
			_dwOperatingSystemVersion = OS_WIN95;
		}
	}

#ifndef PS2_MENU

#if GTA_VERSION >= GTA3_PC_11
	FrontEndMenuManager.LoadSettings();
#endif

#endif

	dwDXVersion = GetDXVersion();
	debug("DirectX version 0x%x\n", dwDXVersion);
	
	if ( _dwOperatingSystemVersion == OS_WIN95 )
	{
		MessageBoxW(nil,
					(LPCWSTR)TheText.Get("WIN_95"),	 // Grand Theft Auto III cannot run on Windows 95
					(LPCWSTR)TheText.Get("WIN_TTL"), // Grand Theft Auto III
					MB_OK);

		return FALSE;
	}
	
	if ( dwDXVersion < 0x801 )
	{
		MessageBoxW(nil,
					(LPCWSTR)TheText.Get("WIN_DX"),	 // Grand Theft Auto III requires at least DirectX version 8.1
					(LPCWSTR)TheText.Get("WIN_TTL"), // Grand Theft Auto III
					MB_OK);

		return FALSE;
	}
	
	MEMORYSTATUS memstats;
	GlobalMemoryStatus(&memstats);
	
	_dwMemTotalPhys	   = memstats.dwTotalPhys;
	_dwMemAvailPhys	   = memstats.dwAvailPhys;
	_dwMemTotalVirtual = memstats.dwTotalVirtual;
	_dwMemAvailVirtual = memstats.dwAvailVirtual;
	
	_GetVideoMemInfo(&_dwMemTotalVideo, &_dwMemAvailVideo);
#ifdef FIX_BUGS
	debug("Physical memory size %lu\n", _dwMemTotalPhys);
	debug("Available physical memory %lu\n", _dwMemAvailPhys);
	debug("Video memory size %lu\n", _dwMemTotalVideo);
	debug("Available video memory %lu\n", _dwMemAvailVideo);
#else
	debug("Physical memory size %d\n", _dwMemTotalPhys);
	debug("Available physical memory %d\n", _dwMemAvailPhys);
	debug("Video memory size %d\n", _dwMemTotalVideo);
	debug("Available video memory %d\n", _dwMemAvailVideo);
#endif
	
	if ( _dwMemAvailVideo < (12 * 1024 * 1024) /*12 MB*/ )
	{
		MessageBoxW(nil,
					(LPCWSTR)TheText.Get("WIN_VDM"), // Grand Theft Auto III requires at least 12MB of available video memory
					(LPCWSTR)TheText.Get("WIN_TTL"), // Grand Theft Auto III
					MB_OK);

		return FALSE;
	}
	
	TheText.Unload();

	return TRUE;
}


/*
 *****************************************************************************
 */
void
psTerminate(void)
{
	return;
}

/*
 *****************************************************************************
 */
static RwChar **_VMList;

RwInt32 _psGetNumVideModes()
{
	return RwEngineGetNumVideoModes();
}

/*
 *****************************************************************************
 */
RwBool _psFreeVideoModeList()
{
	RwInt32 numModes;
	RwInt32 i;
	
	numModes = _psGetNumVideModes();
	
	if ( _VMList == nil )
		return TRUE;
	
	for ( i = 0; i < numModes; i++ )
	{
		RwFree(_VMList[i]);
	}
	
	RwFree(_VMList);
	
	_VMList = nil;
	
	return TRUE;
}
							
/*
 *****************************************************************************
 */							
RwChar **_psGetVideoModeList()
{
	RwInt32 numModes;
	RwInt32 i;
	
	if ( _VMList != nil )
	{
		return _VMList;
	}
	
	numModes = RwEngineGetNumVideoModes();
	
	_VMList = (RwChar **)RwCalloc(numModes, sizeof(RwChar*));
	
	for ( i = 0; i < numModes; i++	)
	{
		RwVideoMode			vm;
		
		RwEngineGetVideoModeInfo(&vm, i);
		
		if ( vm.flags & rwVIDEOMODEEXCLUSIVE )
		{
			if (   vm.width >= 640
				&& vm.height >= 480
				&& (vm.width == 640
				&& vm.height == 480) 
				|| !(vm.flags & rwVIDEOMODEEXCLUSIVE)
				|| (_dwMemTotalVideo - vm.depth * vm.height * vm.width / 8) > (12 * 1024 * 1024)/*12 MB*/ )
			{
				_VMList[i] = (RwChar*)RwCalloc(100, sizeof(RwChar));
				rwsprintf(_VMList[i],"%lu X %lu X %lu", vm.width, vm.height, vm.depth);
			}
			else
				_VMList[i] = nil;
		}
		else
			_VMList[i] = nil;
	}
	
	return _VMList;
}

/*
 *****************************************************************************
 */
void _psSelectScreenVM(RwInt32 videoMode)
{
	RwTexDictionarySetCurrent( nil );
	
	FrontEndMenuManager.UnloadTextures();
	
	if ( !_psSetVideoMode(RwEngineGetCurrentSubSystem(), videoMode) )
	{
		RsGlobal.quit = TRUE;
		
		ShowWindow(PSGLOBAL(window), SW_HIDE);

		MessageBoxW(nil,
			(LPCWSTR)TheText.Get("WIN_RSZ"), // Failed to select new screen resolution
			(LPCWSTR)TheText.Get("WIN_TTL"), // Grand Theft Auto III
			MB_OK);		
	}
	else
		FrontEndMenuManager.LoadAllTextures();
}

/*
 *****************************************************************************
 */
void WaitForState(FILTER_STATE State)
{
	HRESULT hr;
	
	ASSERT(pMC != nil);
	
	// Make sure we have switched to the required state
	LONG   lfs;
	do
	{
		hr = pMC->GetState(10, &lfs);
	} while (State != lfs);
}

/*
 *****************************************************************************
 */
void HandleGraphEvent(void)
{
	LONG evCode, evParam1, evParam2;
	HRESULT hr=S_OK;
	
	ASSERT(pME != nil);

	// Process all queued events
	while (SUCCEEDED(pME->GetEvent(&evCode, (LONG_PTR *)&evParam1,
		(LONG_PTR *)&evParam2, 0)))
	{
		// Free memory associated with callback, since we're not using it
		hr = pME->FreeEventParams(evCode, evParam1, evParam2);

		// If this is the end of the clip, reset to beginning
		if (EC_COMPLETE == evCode)
		{
			switch (gGameState)
			{
				case GS_LOGO_MPEG:
				{
					gGameState = GS_INIT_INTRO_MPEG;
					TRACE("gGameState = GS_INIT_INTRO_MPEG");
					break;
				}
				case GS_INTRO_MPEG:
				{
					gGameState = GS_INIT_ONCE;
					TRACE("gGameState = GS_INIT_ONCE");
					break;
				}
				default:
				{
					break;
				}
			}

			pME->SetNotifyWindow((OAHWND)NULL, 0, 0);
		}
	}
}

/*
 *****************************************************************************
 */
 
LRESULT CALLBACK
MainWndProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
	POINTS points;
	static BOOL noMemory = FALSE;

	
	switch( message )
	{
		case WM_SETCURSOR:
		{
			ShowCursor(FALSE);
			
			SetCursor(nil);
			
			break; // is this correct ?
		}
		
		case WM_SIZE:
		{
			RwRect r;

			r.x = 0;
			r.y = 0;
			r.w = LOWORD(lParam);
			r.h = HIWORD(lParam);

			if (RwInitialised && r.h > 0 && r.w > 0)
			{
				RsEventHandler(rsCAMERASIZE, &r);

				if (r.w != LOWORD(lParam) && r.h != HIWORD(lParam))
				{
					WINDOWPLACEMENT		wp;

					/* failed to create window of required size */
					noMemory = TRUE;

					/* stop re-sizing */
					ReleaseCapture();

					/* handle maximised window */
					GetWindowPlacement(window, &wp);
					if (wp.showCmd == SW_SHOWMAXIMIZED)
					{
						SendMessage(window, WM_WINDOWPOSCHANGED, 0, 0);
					}
				}
				else
				{
					noMemory = FALSE;
				}

			}

			return 0L;
		}

		case WM_SIZING:
		{
			/* 
			 * Handle event to ensure window contents are displayed during re-size
			 * as this can be disabled by the user, then if there is not enough 
			 * memory things don't work.
			 */
			RECT			   *newPos = (LPRECT) lParam;
			RECT				rect;

			/* redraw window */

			if (RwInitialised && gGameState == GS_PLAYING_GAME)
			{
				RsEventHandler(rsIDLE, (void *)TRUE);
			}

			/* Manually resize window */
			rect.left = rect.top = 0;
			rect.bottom = newPos->bottom - newPos->top;
			rect.right = newPos->right - newPos->left;

			SetWindowPos(window, HWND_TOP, rect.left, rect.top,
						 (rect.right - rect.left),
						 (rect.bottom - rect.top), SWP_NOMOVE);

			return 0L;
		}

		case WM_LBUTTONDOWN:
		{
			SetCapture(window);

			return 0L;
		}

		case WM_RBUTTONDOWN:
		{
			SetCapture(window);

			return 0L;
		}
		
		case WM_MBUTTONDOWN:
		{
			SetCapture(window);

			return 0L;
		}

		case WM_MOUSEWHEEL:
		{
			return 0L;
		}

		case WM_MOUSEMOVE:
		{
			points = MAKEPOINTS(lParam);

			FrontEndMenuManager.m_nMouseTempPosX = points.x;
			FrontEndMenuManager.m_nMouseTempPosY = points.y;

			return 0L;
		}

		case WM_LBUTTONUP:
		{
			ReleaseCapture();

			return 0L;
		}

		case WM_RBUTTONUP:
		{
			ReleaseCapture();

			return 0L;
		}
		
		case WM_MBUTTONUP:
		{
			ReleaseCapture();

			return 0L;
		}

		case WM_KEYDOWN:
		{
			RsKeyCodes ks;
			
			if ( _InputTranslateKey(&ks, lParam, wParam) )
				RsKeyboardEventHandler(rsKEYDOWN, &ks);

			if ( wParam == VK_SHIFT )
				_InputTranslateShiftKeyUpDown(&ks);
#ifdef FIX_BUGS
			break;
#else
			return 0L;
#endif
		}

		case WM_KEYUP:
		{
			RsKeyCodes ks;

			if ( _InputTranslateKey(&ks, lParam, wParam) )
				RsKeyboardEventHandler(rsKEYUP, &ks);

			if ( wParam == VK_SHIFT )
				_InputTranslateShiftKeyUpDown(&ks);

#ifdef FIX_BUGS
			break;
#else
			return 0L;
#endif
		}

		case WM_SYSKEYDOWN:
		{
			RsKeyCodes ks;
			
			if ( _InputTranslateKey(&ks, lParam, wParam) )
				RsKeyboardEventHandler(rsKEYDOWN, &ks);

			if ( wParam == VK_SHIFT )
				_InputTranslateShiftKeyUpDown(&ks);

#ifdef FIX_BUGS
			break;
#else
			return 0L;
#endif
		}

		case WM_SYSKEYUP:
		{
			RsKeyCodes ks;

			if ( _InputTranslateKey(&ks, lParam, wParam) )
				RsKeyboardEventHandler(rsKEYUP, &ks);

			if ( wParam == VK_SHIFT )
				_InputTranslateShiftKeyUpDown(&ks);

#ifdef FIX_BUGS
			break;
#else
			return 0L;
#endif
		}

		case WM_ACTIVATEAPP:
		{
			switch ( gGameState )
			{
				case GS_LOGO_MPEG:
				case GS_INTRO_MPEG:
				{
					ASSERT(pMC != nil);
					
					LONG state;
					pMC->GetState(10, &state);
					
					if ( !(BOOL)wParam ) // losing activation
					{
						if ( state == State_Running && pMC != nil )
						{
							HRESULT hr = pMC->Pause();

							if (hr == S_FALSE)
								OutputDebugString("Failed to pause the MPEG");
							else
								WaitForState(State_Paused);
						}
					}
					else
					{
						CenterVideo();
						
						if ( state != State_Running && pMC != nil )
						{
							HRESULT hr = pMC->Run();

							if ( hr == S_FALSE )
								OutputDebugString("Failed to run the MPEG");
							else
							{
								WaitForState(State_Running);
								SetFocus(PSGLOBAL(window));
							}
						}
					}
					
					break;
				}
				
				case GS_START_UP:
				{
					if ( !(BOOL)wParam && PSGLOBAL(fullScreen) ) // losing activation
						startupDeactivate = TRUE;
					
					break;
				}
			}
			
			CPad::GetPad(0)->Clear(false);
			CPad::GetPad(1)->Clear(false);
			
			return 0L;
		}
		
		case WM_TIMER:
		{
			return 0L;
		}
		
		case WM_GRAPHNOTIFY:
		{
			if (gGameState == GS_INTRO_MPEG || gGameState == GS_LOGO_MPEG)
				HandleGraphEvent();

			break;
		}

		case WM_CLOSE:
		case WM_DESTROY:
		{
			/*
			 * Quit message handling.
			 */
			ClipCursor(nil);
			
			_InputShutdown();

			PostQuitMessage(0);

			return 0L;
		}
		
		case WM_DEVICECHANGE:
		{
			if( wParam == DBT_DEVICEREMOVECOMPLETE )
			{
				PDEV_BROADCAST_HDR pDev = (PDEV_BROADCAST_HDR)lParam;
				
				if (pDev->dbch_devicetype != DBT_DEVTYP_VOLUME)
					break;

				if ( DMAudio.IsAudioInitialised() )
				{
					PDEV_BROADCAST_VOLUME pVol = (PDEV_BROADCAST_VOLUME)pDev;
					if ( pVol->dbcv_flags & DBTF_MEDIA )
					{
						char c = DMAudio.GetCDAudioDriveLetter();
						
						if ( c >= 'A' && pVol->dbcv_unitmask & (1 << (c - 'A')) )
						{
							OutputDebugString("About to check CD drive...");
							
							while ( true )
							{
								FrontEndMenuManager.WaitForUserCD();
								
								if ( !FrontEndMenuManager.m_bQuitGameNoCD )
								{
									if ( DMAudio.CheckForAnAudioFileOnCD() )
									{
										OutputDebugString("GTA3 Audio CD has been inserted");
										break;
									}
								}
								else
								{
									OutputDebugString("Exiting game as Audio CD was not inserted");
									break;
								}
							}
						}
					}
				}
			}
			
			break;
		}

	}

	/*
	 * Let Windows handle all other messages.
	 */
	return DefWindowProc(window, message, wParam, lParam);
}


/*
 *****************************************************************************
 */
static BOOL
InitApplication(HANDLE instance)
{
	/*
	 * Perform any necessary MS Windows application initialization. Basically,
	 * this means registering the window class for this application.
	 */

	WNDCLASS windowClass;

	windowClass.style = CS_BYTEALIGNWINDOW;
	windowClass.lpfnWndProc = (WNDPROC)MainWndProc;
	windowClass.cbClsExtra = 0;
	windowClass.cbWndExtra = 0;
	windowClass.hInstance = (HINSTANCE)instance;
#ifdef FIX_BUGS
	windowClass.hIcon = LoadIcon((HINSTANCE)instance, MAKEINTRESOURCE(IDI_MAIN_ICON));
#else
	windowClass.hIcon = nil;
#endif
	windowClass.hCursor = LoadCursor(nil, IDC_ARROW);
	windowClass.hbrBackground = nil;
	windowClass.lpszMenuName = NULL;
	windowClass.lpszClassName = AppClassName;

	return RegisterClass(&windowClass);
}


/*
 *****************************************************************************
 */

RwBool IsForegroundApp()
{
	return !!ForegroundApp;
}

UINT GetBestRefreshRate(UINT width, UINT height, UINT depth)
{
#ifdef USE_D3D9
	LPDIRECT3D9 d3d = Direct3DCreate9(D3D_SDK_VERSION);
#else
	LPDIRECT3D8 d3d = Direct3DCreate8(D3D_SDK_VERSION);
#endif
	ASSERT(d3d != nil);
	
	UINT refreshRate = INT_MAX;
	D3DFORMAT format;

	if ( depth == 32 )
		format = D3DFMT_X8R8G8B8;
	else if ( depth == 24 )
		format = D3DFMT_R8G8B8;
	else
		format = D3DFMT_R5G6B5;
	
#ifdef USE_D3D9
	UINT modeCount = d3d->GetAdapterModeCount(GcurSel, format);
#else
	UINT modeCount = d3d->GetAdapterModeCount(GcurSel);
#endif

	for ( UINT i = 0; i < modeCount; i++ )
	{
		D3DDISPLAYMODE mode;
		
#ifdef USE_D3D9
		d3d->EnumAdapterModes(GcurSel, format, i, &mode);
#else
		d3d->EnumAdapterModes(GcurSel, i, &mode);
#endif	
		if ( mode.Width == width && mode.Height == height && mode.Format == format )
		{
			if ( mode.RefreshRate == 0 ) {
				// From VC
#ifdef FIX_BUGS
				d3d->Release();
#endif
				return 0;
			}

			if ( mode.RefreshRate < refreshRate && mode.RefreshRate >= 60 )
				refreshRate = mode.RefreshRate;
		}
	}
	
	// From VC
#ifdef FIX_BUGS
	d3d->Release();
#endif
	
	if ( refreshRate == -1 )
		return -1;

	return refreshRate;
}

/*
 *****************************************************************************
 */
RwBool
psSelectDevice()
{
	RwVideoMode			vm;
	RwInt32				subSysNum;
	RwInt32				AutoRenderer = 0;
	

	RwBool modeFound = FALSE;
	
	if ( !useDefault )
	{
		GnumSubSystems = RwEngineGetNumSubSystems();
		if ( !GnumSubSystems )
		{
			 return FALSE;
		}
		
		/* Just to be sure ... */
		GnumSubSystems = (GnumSubSystems > MAX_SUBSYSTEMS) ? MAX_SUBSYSTEMS : GnumSubSystems;
		
		/* Get the names of all the sub systems */
		for (subSysNum = 0; subSysNum < GnumSubSystems; subSysNum++)
		{
			RwEngineGetSubSystemInfo(&GsubSysInfo[subSysNum], subSysNum);
		}
		
		/* Get the default selection */
		GcurSel = RwEngineGetCurrentSubSystem();
#ifdef IMPROVED_VIDEOMODE
		if(FrontEndMenuManager.m_nPrefsSubsystem < GnumSubSystems)
			GcurSel = FrontEndMenuManager.m_nPrefsSubsystem;
#endif
	}
	
	/* Set the driver to use the correct sub system */
	if (!RwEngineSetSubSystem(GcurSel))
	{
		return FALSE;
	}

#ifdef IMPROVED_VIDEOMODE
	FrontEndMenuManager.m_nPrefsSubsystem = GcurSel;
#endif

#ifndef IMPROVED_VIDEOMODE
	if ( !useDefault )
	{
		if ( _psGetVideoModeList()[FrontEndMenuManager.m_nDisplayVideoMode] && FrontEndMenuManager.m_nDisplayVideoMode )
		{
			FrontEndMenuManager.m_nPrefsVideoMode = FrontEndMenuManager.m_nDisplayVideoMode;
			GcurSelVM = FrontEndMenuManager.m_nDisplayVideoMode;
		}
		else
		{
#ifdef DEFAULT_NATIVE_RESOLUTION
			// get the native video mode
			HDC hDevice = GetDC(NULL);
			int w = GetDeviceCaps(hDevice, HORZRES);
			int h = GetDeviceCaps(hDevice, VERTRES);
			int d = GetDeviceCaps(hDevice, BITSPIXEL);
#else
			const int w = 640;
			const int h = 480;
			const int d = 16;
#endif
			while ( !modeFound && GcurSelVM < RwEngineGetNumVideoModes() )
			{
				RwEngineGetVideoModeInfo(&vm, GcurSelVM);
				if ( defaultFullscreenRes	&& vm.width	 != w 
											|| vm.height != h
											|| vm.depth	 != d
											|| !(vm.flags & rwVIDEOMODEEXCLUSIVE) )
					++GcurSelVM;
				else
					modeFound = TRUE;
			}
			
			if ( !modeFound )
			{
#ifdef DEFAULT_NATIVE_RESOLUTION
				GcurSelVM = 1;
#else
				MessageBox(nil, "Cannot find 640x480 video mode", "GTA3", MB_OK);
				return FALSE;
#endif
			}
		}
	}
#else
	if ( !useDefault )
	{
		if(FrontEndMenuManager.m_nPrefsWidth == 0 ||
			FrontEndMenuManager.m_nPrefsHeight == 0 ||
			FrontEndMenuManager.m_nPrefsDepth == 0){
			// Defaults if nothing specified
			FrontEndMenuManager.m_nPrefsWidth = GetSystemMetrics(SM_CXSCREEN);
			FrontEndMenuManager.m_nPrefsHeight = GetSystemMetrics(SM_CYSCREEN);
			FrontEndMenuManager.m_nPrefsDepth = 32;
			FrontEndMenuManager.m_nPrefsWindowed = 0;
		}

		// Find the videomode that best fits what we got from the settings file
		RwInt32 bestFsMode = -1;
		RwInt32 bestWidth = -1;
		RwInt32 bestHeight = -1;
		RwInt32 bestDepth = -1;
		for (GcurSelVM = 0; GcurSelVM < RwEngineGetNumVideoModes(); GcurSelVM++) {
			RwEngineGetVideoModeInfo(&vm, GcurSelVM);

			if (!(vm.flags & rwVIDEOMODEEXCLUSIVE)) {
				bestWndMode = GcurSelVM;
			} else {
				// try the largest one that isn't larger than what we wanted
				if (vm.width >= bestWidth && vm.width <= FrontEndMenuManager.m_nPrefsWidth &&
					vm.height >= bestHeight && vm.height <= FrontEndMenuManager.m_nPrefsHeight &&
					vm.depth >= bestDepth && vm.depth <= FrontEndMenuManager.m_nPrefsDepth){
					bestWidth = vm.width;
					bestHeight = vm.height;
					bestDepth = vm.depth;
					bestFsMode = GcurSelVM;
				}
			}
		}

		if(bestFsMode < 0){
			MessageBox(nil, "Cannot find desired video mode", "GTA3", MB_OK);
			return FALSE;
		}
		GcurSelVM = bestFsMode;

		FrontEndMenuManager.m_nDisplayVideoMode = GcurSelVM;
		FrontEndMenuManager.m_nPrefsVideoMode = FrontEndMenuManager.m_nDisplayVideoMode;

		FrontEndMenuManager.m_nSelectedScreenMode = FrontEndMenuManager.m_nPrefsWindowed;
	}
#endif

	RwEngineGetVideoModeInfo(&vm, GcurSelVM);

#ifdef IMPROVED_VIDEOMODE
	if (FrontEndMenuManager.m_nPrefsWindowed)
		GcurSelVM = bestWndMode;

	// Now GcurSelVM is 0 but vm has sizes(and fullscreen flag) of the video mode we want, that's why we changed the rwVIDEOMODEEXCLUSIVE conditions below
	FrontEndMenuManager.m_nPrefsWidth = vm.width;
	FrontEndMenuManager.m_nPrefsHeight = vm.height;
	FrontEndMenuManager.m_nPrefsDepth = vm.depth;
#endif

#ifndef PS2_MENU
	FrontEndMenuManager.m_nCurrOption = 0;
#endif
	
	/* Set up the video mode and set the apps window
	* dimensions to match */
	if (!RwEngineSetVideoMode(GcurSelVM))
	{
		return FALSE;
	}

#ifdef IMPROVED_VIDEOMODE
	if (!FrontEndMenuManager.m_nPrefsWindowed)
#else
	if (vm.flags & rwVIDEOMODEEXCLUSIVE)
#endif
	{
		debug("%dx%dx%d", vm.width, vm.height, vm.depth);
		
		UINT refresh = GetBestRefreshRate(vm.width, vm.height, vm.depth);
		
		if ( refresh != (UINT)-1 )
		{
			debug("refresh %d", refresh);
			RwD3D8EngineSetRefreshRate((RwUInt32)refresh);
		}
	}
	
#ifdef IMPROVED_VIDEOMODE
	if (!FrontEndMenuManager.m_nPrefsWindowed)
#else
	if (vm.flags & rwVIDEOMODEEXCLUSIVE)
#endif
	{
		RsGlobal.maximumWidth = vm.width;
		RsGlobal.maximumHeight = vm.height;
		RsGlobal.width = vm.width;
		RsGlobal.height = vm.height;
		
		PSGLOBAL(fullScreen) = TRUE;

#ifdef IMPROVED_VIDEOMODE
		SetWindowLong(PSGLOBAL(window), GWL_STYLE, WS_POPUP);
		SetWindowPos(PSGLOBAL(window), nil, 0, 0, 0, 0,
					SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|
					SWP_FRAMECHANGED);
	}else{
		RECT rect;
		rect.left = rect.top = 0;
		rect.right = FrontEndMenuManager.m_nPrefsWidth;
		rect.bottom = FrontEndMenuManager.m_nPrefsHeight;
		AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

		// center it
		int spaceX = GetSystemMetrics(SM_CXSCREEN) - (rect.right-rect.left);
		int spaceY = GetSystemMetrics(SM_CYSCREEN) - (rect.bottom-rect.top);

		SetWindowLong(PSGLOBAL(window), GWL_STYLE, WS_VISIBLE | WS_OVERLAPPEDWINDOW);
		SetWindowPos(PSGLOBAL(window), HWND_NOTOPMOST, spaceX/2, spaceY/2,
			(rect.right - rect.left),
			(rect.bottom - rect.top), 0);

		// Have to get actual size because the window perhaps didn't fit
		GetClientRect(PSGLOBAL(window), &rect);
		RsGlobal.maximumWidth = rect.right;
		RsGlobal.maximumHeight = rect.bottom;
		RsGlobal.width = rect.right;
		RsGlobal.height = rect.bottom;
		
		PSGLOBAL(fullScreen) = FALSE;
#endif
	}
#ifdef MULTISAMPLING
	RwD3D8EngineSetMultiSamplingLevels(1 << FrontEndMenuManager.m_nPrefsMSAALevel);
#endif
	return TRUE;
}

/*
 *****************************************************************************
 */
RwBool _psSetVideoMode(RwInt32 subSystem, RwInt32 videoMode)
{
	RwInitialised = FALSE;
	
	RsEventHandler(rsRWTERMINATE, nil);
	
	GcurSel = subSystem;
	GcurSelVM = videoMode;
	
	useDefault = TRUE;
	
	if ( RsEventHandler(rsRWINITIALIZE, PSGLOBAL(window)) == rsEVENTERROR )
		return FALSE;
	
	RwInitialised = TRUE;
	useDefault = FALSE;
	
	RwRect r;
	
	r.x = 0;
	r.y = 0;
	r.w = RsGlobal.maximumWidth;
	r.h = RsGlobal.maximumHeight;

	RsEventHandler(rsCAMERASIZE, &r);
	
	return TRUE;
}
 
 
/*
 *****************************************************************************
 */
static RwChar **
CommandLineToArgv(RwChar *cmdLine, RwInt32 *argCount)
{
	RwInt32 numArgs = 0;
	RwBool inArg, inString;
	RwInt32 i, len;
	RwChar *res, *str, **aptr;

	len = (int)strlen(cmdLine);

	/* 
	 * Count the number of arguments...
	 */
	inString = FALSE;
	inArg = FALSE;

	for(i=0; i<=len; i++)
	{
		if( cmdLine[i] == '"' )
		{
			inString = !inString;
		}

		if( (cmdLine[i] <= ' ' && !inString) || i == len )
		{
			if( inArg ) 
			{
				inArg = FALSE;
				
				numArgs++;
			}
		} 
		else if( !inArg )
		{
			inArg = TRUE;
		}
	}

	/* 
	 * Allocate memory for result...
	 */
	res = (RwChar *)malloc(sizeof(RwChar *) * numArgs + len + 1);
	str = res + sizeof(RwChar *) * numArgs;
	aptr = (RwChar **)res;

	strcpy(str, cmdLine);

	/*
	 * Walk through cmdLine again this time setting pointer to each arg...
	 */
	inArg = FALSE;
	inString = FALSE;

	for(i=0; i<=len; i++)
	{
		if( cmdLine[i] == '"' )
		{
			inString = !inString;
		}

		if( (cmdLine[i] <= ' ' && !inString) || i == len )
		{
			if( inArg ) 
			{
				if( str[i-1] == '"' )
				{
					str[i-1] = '\0';
				}
				else
				{
					str[i] = '\0';
				}
				
				inArg = FALSE;
			}
		} 
		else if( !inArg && cmdLine[i] != '"' )
		{
			inArg = TRUE; 
			
			*aptr++ = &str[i];
		}
	}

	*argCount = numArgs;

	return (RwChar **)res;
}

/*
 *****************************************************************************
 */
void InitialiseLanguage()
{
	WORD primUserLCID	= PRIMARYLANGID(GetSystemDefaultLCID());
	WORD primSystemLCID = PRIMARYLANGID(GetUserDefaultLCID());
	WORD primLayout		= PRIMARYLANGID((DWORD_PTR)GetKeyboardLayout(0));
	
	WORD subUserLCID	= SUBLANGID(GetSystemDefaultLCID());
	WORD subSystemLCID	= SUBLANGID(GetUserDefaultLCID());
	WORD subLayout		= SUBLANGID((DWORD_PTR)GetKeyboardLayout(0));
	
	if (   primUserLCID	  == LANG_GERMAN
		|| primSystemLCID == LANG_GERMAN
		|| primLayout	  == LANG_GERMAN )
	{
		CGame::nastyGame = false;
		CMenuManager::m_PrefsAllowNastyGame = false;
		CGame::germanGame = true;
	}
	
	if (   primUserLCID	  == LANG_FRENCH
		|| primSystemLCID == LANG_FRENCH
		|| primLayout	  == LANG_FRENCH )
	{
		CGame::nastyGame = false;
		CMenuManager::m_PrefsAllowNastyGame = false;
		CGame::frenchGame = true;
	}
	
	if (   subUserLCID	 == SUBLANG_ENGLISH_AUS
		|| subSystemLCID == SUBLANG_ENGLISH_AUS
		|| subLayout	 == SUBLANG_ENGLISH_AUS )
		CGame::noProstitutes = true;

#ifdef NASTY_GAME
	CGame::nastyGame = true;
	CMenuManager::m_PrefsAllowNastyGame = true;
	CGame::noProstitutes = false;
#endif
	
	int32 lang;
	
	switch ( primSystemLCID )
	{
		case LANG_GERMAN:
		{
			lang = LANG_GERMAN;
			break;
		}
		case LANG_FRENCH:
		{
			lang = LANG_FRENCH;
			break;
		}
		case LANG_SPANISH:
		{
			lang = LANG_SPANISH;
			break;
		}
		case LANG_ITALIAN:
		{
			lang = LANG_ITALIAN;
			break;
		}
		default:
		{
			lang = ( subSystemLCID == SUBLANG_ENGLISH_AUS ) ? -99 : LANG_ENGLISH;
			break;
		}
	}
	
	CMenuManager::OS_Language = primUserLCID;

	switch ( lang )
	{
		case LANG_GERMAN:
		{
			CMenuManager::m_PrefsLanguage = CMenuManager::LANGUAGE_GERMAN;
			break;
		}
		case LANG_SPANISH:
		{
			CMenuManager::m_PrefsLanguage = CMenuManager::LANGUAGE_SPANISH;
			break;
		}
		case LANG_FRENCH:
		{
			CMenuManager::m_PrefsLanguage = CMenuManager::LANGUAGE_FRENCH;
			break;
		}
		case LANG_ITALIAN:
		{
			CMenuManager::m_PrefsLanguage = CMenuManager::LANGUAGE_ITALIAN;
			break;
		}
		default:
		{
			CMenuManager::m_PrefsLanguage = CMenuManager::LANGUAGE_AMERICAN;
			break;
		}
	}

	TheText.Unload();
	TheText.Load();
}

/*
 *****************************************************************************
 */
void CenterVideo(void)
{
	HRESULT hr = S_OK;
	RECT rect;

	ASSERT(pVW != nil);
	
	GetClientRect(PSGLOBAL(window), &rect);

	JIF(pVW->SetWindowPosition(rect.left, rect.top, rect.right, rect.bottom));

	JIF(pVW->put_MessageDrain((OAHWND) PSGLOBAL(window)));

	SetFocus(PSGLOBAL(window));
}

/*
 *****************************************************************************
 */
void PlayMovieInWindow(int cmdShow, const char* szFile)
{
	WCHAR wFileName[256];
	HRESULT hr;

	// Clear open dialog remnants before calling RenderFile()
	UpdateWindow(PSGLOBAL(window));

	// Convert filename to wide character string
	MultiByteToWideChar(CP_ACP, 0, szFile, -1, wFileName, sizeof(wFileName) - 1);

	// Initialize COM
#ifdef FIX_BUGS // will also return S_FALSE if it has already been inited in the same thread
	CoInitialize(nil);
#else
	JIF(CoInitialize(nil));
#endif

	// Get the interface for DirectShow's GraphBuilder
	JIF(CoCreateInstance(CLSID_FilterGraph, nil, CLSCTX_INPROC, 
						 IID_IGraphBuilder, (void **)&pGB));

	if(SUCCEEDED(hr))
	{
		// Have the graph builder construct its the appropriate graph automatically
		JIF(pGB->RenderFile(&wFileName[0], nil));

		// QueryInterface for DirectShow interfaces
		JIF(pGB->QueryInterface(IID_IMediaControl, (void **)&pMC));
		JIF(pGB->QueryInterface(IID_IMediaEventEx, (void **)&pME));
		JIF(pGB->QueryInterface(IID_IMediaSeeking, (void **)&pMS));

		// Query for video interfaces, which may not be relevant for audio files
		JIF(pGB->QueryInterface(IID_IVideoWindow, (void **)&pVW));

		JIF(pVW->put_Owner((OAHWND) PSGLOBAL(window)));
		JIF(pVW->put_WindowStyle(WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN));

		// Have the graph signal event via window callbacks for performance
		JIF(pME->SetNotifyWindow((OAHWND)PSGLOBAL(window), WM_GRAPHNOTIFY, 0));

		CenterVideo();

		// Run the graph to play the media file
		JIF(pMC->Run());

		SetFocus(PSGLOBAL(window));
	}
	
	ASSERT(pGB != nil);
	ASSERT(pVW != nil);
	ASSERT(pME != nil);
	ASSERT(pMC != nil);

	if(FAILED(hr))
		CloseClip();
}

/*
 *****************************************************************************
 */
void CloseInterfaces(void)
{
	// Release and zero DirectShow interfaces
	SAFE_RELEASE(pME);
	SAFE_RELEASE(pMS);
	SAFE_RELEASE(pMC);
	SAFE_RELEASE(pVW);
	SAFE_RELEASE(pGB);
}

/*
 *****************************************************************************
 */
void CloseClip(void)
{
	HRESULT hr;

	// Stop playback
	if(pMC)
		hr = pMC->Stop();

	// Free DirectShow interfaces
	CloseInterfaces();
}

/*
 *****************************************************************************
 */
void HandleExit()
{
	MSG message;
	while ( PeekMessage(&message, nil, 0U, 0U, PM_REMOVE|PM_NOYIELD) )
	{
		if( message.message == WM_QUIT )
		{
			RsGlobal.quit = TRUE;
		}
		else
		{
			TranslateMessage(&message);
			DispatchMessage(&message);
		}
	}
}
 
/*
 *****************************************************************************
 */
int PASCAL
WinMain(HINSTANCE instance, 
		HINSTANCE prevInstance	__RWUNUSED__, 
		CMDSTR cmdLine, 
		int cmdShow)
{
	MSG message;
	RwV2d pos;
	RwInt32 argc, i;
	RwChar **argv;
	SystemParametersInfo(SPI_SETFOREGROUNDLOCKTIMEOUT, 0, nil, SPIF_SENDCHANGE);

#ifndef MASTER
	if (strstr(cmdLine, "-console"))
	{
		AllocConsole();
		freopen("CONIN$", "r", stdin);
		freopen("CONOUT$", "w", stdout);
		freopen("CONOUT$", "w", stderr);
	}
#endif

#ifdef USE_CUSTOM_ALLOCATOR
	InitMemoryMgr();
#endif

	/* 
	 * Initialize the platform independent data.
	 * This will in turn initialize the platform specific data...
	 */
	if( RsEventHandler(rsINITIALIZE, nil) == rsEVENTERROR )
	{
		return FALSE;
	}

	/*
	 * Register the window class...
	 */
	if( !InitApplication(instance) )
	{
		return FALSE;
	}

	/*
	 * Get proper command line params, cmdLine passed to us does not
	 * work properly under all circumstances...
	 */
	cmdLine = GetCommandLine();

	/*
	 * Parse command line into standard (argv, argc) parameters...
	 */
	argv = CommandLineToArgv(cmdLine, &argc);


	/* 
	 * Parse command line parameters (except program name) one at 
	 * a time BEFORE RenderWare initialization...
	 */
	for(i=1; i<argc; i++)
	{
		RsEventHandler(rsPREINITCOMMANDLINE, argv[i]);
	}

	/*
	 * Create the window...
	 */
	PSGLOBAL(window) = InitInstance(instance);
	if( PSGLOBAL(window) == nil )
	{
		return FALSE;
	}

	PSGLOBAL(instance) = instance;
	
	ControlsManager.MakeControllerActionsBlank();
	ControlsManager.InitDefaultControlConfiguration();
	
	if ( _InputInitialise() == S_OK )
	{
		_InputInitialiseMouse();
		_InputInitialiseJoys();
	}
	
	ControlsManager.InitDefaultControlConfigMouse(MousePointerStateHelper.GetMouseSetUp());

#ifndef IMPROVED_VIDEOMODE
	SetWindowLong(PSGLOBAL(window), GWL_STYLE, WS_POPUP);
	SetWindowPos(PSGLOBAL(window), nil, 0, 0, 0, 0,
				SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|
				SWP_FRAMECHANGED);
#endif

	/* 
	 * Initialize the 3D (RenderWare) components of the app...
	 */
	if( rsEVENTERROR == RsEventHandler(rsRWINITIALIZE, PSGLOBAL(window)) )
	{
		DestroyWindow(PSGLOBAL(window));

		RsEventHandler(rsTERMINATE, nil);

		return FALSE;
	}

	/* 
	 * Parse command line parameters (except program name) one at 
	 * a time AFTER RenderWare initialization...
	 */
	for(i=1; i<argc; i++)
	{
		RsEventHandler(rsCOMMANDLINE, argv[i]);
	}

	/* 
	 * Force a camera resize event...
	 */
	{
		RwRect r;

		r.x = 0;
		r.y = 0;
		r.w = RsGlobal.maximumWidth;
		r.h = RsGlobal.maximumHeight;

		RsEventHandler(rsCAMERASIZE, &r);
	}
	
	SystemParametersInfo(SPI_SETSCREENSAVEACTIVE, FALSE, nil, SPIF_SENDCHANGE);
	SystemParametersInfo(SPI_SETPOWEROFFACTIVE, FALSE, nil, SPIF_SENDCHANGE);
	SystemParametersInfo(SPI_SETLOWPOWERACTIVE, FALSE, nil, SPIF_SENDCHANGE);
	

	STICKYKEYS SavedStickyKeys;
	SavedStickyKeys.cbSize = sizeof(STICKYKEYS);
	
	SystemParametersInfo(SPI_GETSTICKYKEYS, sizeof(STICKYKEYS), &SavedStickyKeys, SPIF_SENDCHANGE);
	
	STICKYKEYS NewStickyKeys;
	NewStickyKeys.cbSize = sizeof(STICKYKEYS);
	NewStickyKeys.dwFlags = SKF_TWOKEYSOFF;
	
	SystemParametersInfo(SPI_SETSTICKYKEYS, sizeof(STICKYKEYS), &NewStickyKeys, SPIF_SENDCHANGE);
	

	/*
	 * Show the window, and refresh it...
	 */
	ShowWindow(PSGLOBAL(window), cmdShow);
	UpdateWindow(PSGLOBAL(window));
	
	{
		CFileMgr::SetDirMyDocuments();
		
#ifdef LOAD_INI_SETTINGS
		// At this point InitDefaultControlConfigJoyPad must have set all bindings to default and ms_padButtonsInited to number of detected buttons.
		// We will load stored bindings below, but let's cache ms_padButtonsInited before LoadINIControllerSettings and LoadSettings clears it,
		// so we can add new joy bindings **on top of** stored bindings.
		int connectedPadButtons = ControlsManager.ms_padButtonsInited;
#endif

		int32 gta3set = CFileMgr::OpenFile("gta3.set", "r");

		if ( gta3set )
		{
			ControlsManager.LoadSettings(gta3set);
			CFileMgr::CloseFile(gta3set);
		}
		
		CFileMgr::SetDir("");

#ifdef LOAD_INI_SETTINGS
		LoadINIControllerSettings();
		if (connectedPadButtons != 0) {
			ControlsManager.InitDefaultControlConfigJoyPad(connectedPadButtons);
			SaveINIControllerSettings();
		}
#endif
	}
	
	SetErrorMode(SEM_FAILCRITICALERRORS);

			
#ifdef PS2_MENU
	int32 r = TheMemoryCard.CheckCardStateAtGameStartUp(CARD_ONE);
	if (   r == CMemoryCard::ERR_DIRNOENTRY  || r == CMemoryCard::ERR_NOFORMAT
		&& r != CMemoryCard::ERR_OPENNOENTRY && r != CMemoryCard::ERR_NONE )
	{
		LoadingScreen(nil, nil, "loadsc0");
		
		TheText.Unload();
		TheText.Load();
		
		CFont::Initialise();
		
		FrontEndMenuManager.DrawMemoryCardStartUpMenus();
	}
#endif

#ifndef MASTER
	if (gbModelViewer) {
		// This is TheModelViewer in LCS, but not compiled on III Mobile.
		LoadingScreen("Loading the ModelViewer", NULL, GetRandomSplashScreen());
		CAnimViewer::Initialise();
		CTimer::Update();
#ifndef PS2_MENU
		FrontEndMenuManager.m_bGameNotLoaded = false;
#endif
	}
#endif

	while ( TRUE )
	{
		RwInitialised = TRUE;
		
		/* 
		* Set the initial mouse position...
		*/
		pos.x = RsGlobal.maximumWidth * 0.5f;
		pos.y = RsGlobal.maximumHeight * 0.5f;

		RsMouseSetPos(&pos);
		
		WINDOWPLACEMENT		wp;
		wp.length = sizeof(WINDOWPLACEMENT);

		/*
		* Enter the message processing loop...
		*/
#ifdef PS2_MENU
		if (TheMemoryCard.m_bWantToLoad)
			LoadSplash(GetLevelSplashScreen(CGame::currLevel));
		
		TheMemoryCard.m_bWantToLoad = false;
		
		CTimer::Update();
		
		while( !RsGlobal.quit && !(FrontEndMenuManager.m_bWantToRestart || TheMemoryCard.b_FoundRecentSavedGameWantToLoad) )
#else
		while( !RsGlobal.quit && !FrontEndMenuManager.m_bWantToRestart )
#endif
		{
			if( PeekMessage(&message, nil, 0U, 0U, PM_REMOVE|PM_NOYIELD) )
			{
				if( message.message == WM_QUIT )
				{
					break;
				}
				else
				{
					TranslateMessage(&message);
					DispatchMessage(&message);
				}
			}
#ifndef MASTER
			else if (gbModelViewer) {
				// This is TheModelViewerCore in LCS
				TheModelViewer();
			}
#endif
			else if( ForegroundApp )
			{
				switch ( gGameState )
				{
					case GS_START_UP:
					{
#ifdef NO_MOVIES
						gGameState = GS_INIT_ONCE;
#else
						gGameState = GS_INIT_LOGO_MPEG;
#endif
						TRACE("gGameState = GS_INIT_LOGO_MPEG");
						break;
					}
					
					case GS_INIT_LOGO_MPEG:
					{
						if ( !startupDeactivate )
							PlayMovieInWindow(cmdShow, "movies\\Logo.mpg");
						gGameState = GS_LOGO_MPEG;
						TRACE("gGameState = GS_LOGO_MPEG;");
						break;
					}
					
					case GS_LOGO_MPEG:
					{
						CPad::UpdatePads();

						if ( startupDeactivate || ControlsManager.GetJoyButtonJustDown() != 0 )
							++gGameState;
						else if ( CPad::GetPad(0)->GetLeftMouseJustDown() )
							++gGameState;
						else if ( CPad::GetPad(0)->GetEnterJustDown() )
							++gGameState;
						else if ( CPad::GetPad(0)->GetCharJustDown(' ') )
							++gGameState;
						else if ( CPad::GetPad(0)->GetAltJustDown() )
							++gGameState;
						else if ( CPad::GetPad(0)->GetTabJustDown() )
							++gGameState;

						break;
					}
					
					case GS_INIT_INTRO_MPEG:
					{
#ifndef NO_MOVIES
						CloseClip();
						CoUninitialize();
#endif
						
						if ( CMenuManager::OS_Language == LANG_FRENCH || CMenuManager::OS_Language == LANG_GERMAN )
							PlayMovieInWindow(cmdShow, "movies\\GTAtitlesGER.mpg");
						else
							PlayMovieInWindow(cmdShow, "movies\\GTAtitles.mpg");
						
						gGameState = GS_INTRO_MPEG;
						TRACE("gGameState = GS_INTRO_MPEG;");
						break;
					}
					
					case GS_INTRO_MPEG:
					{
						CPad::UpdatePads();

						if ( startupDeactivate || ControlsManager.GetJoyButtonJustDown() != 0 )
							++gGameState;
						else if ( CPad::GetPad(0)->GetLeftMouseJustDown() )
							++gGameState;
						else if ( CPad::GetPad(0)->GetEnterJustDown() )
							++gGameState;
						else if ( CPad::GetPad(0)->GetCharJustDown(' ') )
							++gGameState;
						else if ( CPad::GetPad(0)->GetAltJustDown() )
							++gGameState;
						else if ( CPad::GetPad(0)->GetTabJustDown() )
							++gGameState;

						break;
					}
					
					case GS_INIT_ONCE:
					{
#ifndef NO_MOVIES
						CloseClip();
						CoUninitialize();
#endif
						
#ifdef FIX_BUGS
						// draw one frame because otherwise we'll end up looking at black screen for a while if vsync is on
						RsCameraShowRaster(Scene.camera);
#endif

#ifdef PS2_MENU
						extern char version_name[64];
						if ( CGame::frenchGame || CGame::germanGame )
							LoadingScreen(NULL, version_name, "loadsc24");
						else
							LoadingScreen(NULL, version_name, "loadsc0");
						
						printf("Into TheGame!!!\n");
#else				
						LoadingScreen(nil, nil, "loadsc0");
#endif
						if ( !CGame::InitialiseOnceAfterRW() )
							RsGlobal.quit = TRUE;
				
#ifdef PS2_MENU
						gGameState = GS_INIT_PLAYING_GAME;
#else
						gGameState = GS_INIT_FRONTEND;
						TRACE("gGameState = GS_INIT_FRONTEND;");
#endif
						break;
					}
					
#ifndef PS2_MENU
					case GS_INIT_FRONTEND:
					{
						LoadingScreen(nil, nil, "loadsc0");
						
						FrontEndMenuManager.m_bGameNotLoaded = true;
						
						CMenuManager::m_bStartUpFrontEndRequested = true;
						
						if ( defaultFullscreenRes )
						{
							defaultFullscreenRes = FALSE;
							FrontEndMenuManager.m_nPrefsVideoMode = GcurSelVM;
							FrontEndMenuManager.m_nDisplayVideoMode = GcurSelVM;
						}
						
						gGameState = GS_FRONTEND;
						TRACE("gGameState = GS_FRONTEND;");
						break;
					}
					
					case GS_FRONTEND:
					{
						GetWindowPlacement(PSGLOBAL(window), &wp);
						
						if (wp.showCmd != SW_SHOWMINIMIZED)
							RsEventHandler(rsFRONTENDIDLE, nil);

#ifdef PS2_MENU
						if ( !FrontEndMenuManager.m_bMenuActive || TheMemoryCard.m_bWantToLoad )
#else
						if ( !FrontEndMenuManager.m_bMenuActive || FrontEndMenuManager.m_bWantToLoad )
#endif
						{
							gGameState = GS_INIT_PLAYING_GAME;
							TRACE("gGameState = GS_INIT_PLAYING_GAME;");
						}

#ifdef PS2_MENU
						if (TheMemoryCard.m_bWantToLoad )
#else
						if ( FrontEndMenuManager.m_bWantToLoad )
#endif
						{
							InitialiseGame();
							FrontEndMenuManager.m_bGameNotLoaded = false;
							gGameState = GS_PLAYING_GAME;
							TRACE("gGameState = GS_PLAYING_GAME;");
						}
						break;
					}
#endif
					
					case GS_INIT_PLAYING_GAME:
					{
#ifdef PS2_MENU
						CGame::Initialise("DATA\\GTA3.DAT");
						
						//LoadingScreen("Starting Game", NULL, GetRandomSplashScreen());
					
						if (   TheMemoryCard.CheckCardInserted(CARD_ONE) == CMemoryCard::NO_ERR_SUCCESS
							&& TheMemoryCard.ChangeDirectory(CARD_ONE, TheMemoryCard.Cards[CARD_ONE].dir)
							&& TheMemoryCard.FindMostRecentFileName(CARD_ONE, TheMemoryCard.MostRecentFile) == true
							&& TheMemoryCard.CheckDataNotCorrupt(TheMemoryCard.MostRecentFile))
						{
							strcpy(TheMemoryCard.LoadFileName, TheMemoryCard.MostRecentFile);
							TheMemoryCard.b_FoundRecentSavedGameWantToLoad = true;
					
							if (CMenuManager::m_PrefsLanguage != TheMemoryCard.GetLanguageToLoad())
							{
								CMenuManager::m_PrefsLanguage = TheMemoryCard.GetLanguageToLoad();
								TheText.Unload();
								TheText.Load();
							}
					
							CGame::currLevel = (eLevelName)TheMemoryCard.GetLevelToLoad();
						}
#else
						InitialiseGame();

						FrontEndMenuManager.m_bGameNotLoaded = false;
#endif
						gGameState = GS_PLAYING_GAME;
						TRACE("gGameState = GS_PLAYING_GAME;");
						break;
					}
					
					case GS_PLAYING_GAME:
					{
						float ms = (float)CTimer::GetCurrentTimeInCycles() / (float)CTimer::GetCyclesPerMillisecond();
						if ( RwInitialised )
						{
							if (!CMenuManager::m_PrefsFrameLimiter || (1000.0f / (float)RsGlobal.maxFPS) < ms)
								RsEventHandler(rsIDLE, (void *)TRUE);
						}
						break;
					}
				}
			}
			else
			{
				if ( RwCameraBeginUpdate(Scene.camera) )
				{
					RwCameraEndUpdate(Scene.camera);
					ForegroundApp = TRUE;
					RsEventHandler(rsACTIVATE, (void *)TRUE);
				}
				
				WaitMessage();
			}
		}

		
		/* 
		* About to shut down - block resize events again...
		*/
		RwInitialised = FALSE;
		
		FrontEndMenuManager.UnloadTextures();
#ifdef PS2_MENU	
		if ( !(FrontEndMenuManager.m_bWantToRestart || TheMemoryCard.b_FoundRecentSavedGameWantToLoad))
			break;
#else
		if ( !FrontEndMenuManager.m_bWantToRestart )
			break;
#endif
		
		CPad::ResetCheats();
		CPad::StopPadsShaking();
		
		DMAudio.ChangeMusicMode(MUSICMODE_DISABLE);
		
#ifdef PS2_MENU
		CGame::ShutDownForRestart();
#endif
		CTimer::Stop();
		
#ifdef PS2_MENU
		if (FrontEndMenuManager.m_bWantToRestart || TheMemoryCard.b_FoundRecentSavedGameWantToLoad)
		{
			if (TheMemoryCard.b_FoundRecentSavedGameWantToLoad)
			{
				FrontEndMenuManager.m_bWantToRestart = true;
				TheMemoryCard.m_bWantToLoad = true;
			}

			CGame::InitialiseWhenRestarting();
			DMAudio.ChangeMusicMode(MUSICMODE_GAME);
			FrontEndMenuManager.m_bWantToRestart = false;
			
			continue;
		}
		
		CGame::ShutDown();	
		CTimer::Stop();
		
		break;
#else
		if ( FrontEndMenuManager.m_bWantToLoad )
		{
			CGame::ShutDownForRestart();
			CGame::InitialiseWhenRestarting();
			DMAudio.ChangeMusicMode(MUSICMODE_GAME);
			LoadSplash(GetLevelSplashScreen(CGame::currLevel));
			FrontEndMenuManager.m_bWantToLoad = false;
		}
		else
		{
#ifndef MASTER
			if ( gbModelViewer )
				CAnimViewer::Shutdown();
			else
#endif
			if ( gGameState == GS_PLAYING_GAME )
				CGame::ShutDown();

			CTimer::Stop();
			
			if ( FrontEndMenuManager.m_bFirstTime == true )
			{
				gGameState = GS_INIT_FRONTEND;
				TRACE("gGameState = GS_INIT_FRONTEND;");
			}
			else
			{
				gGameState = GS_INIT_PLAYING_GAME;
				TRACE("gGameState = GS_INIT_PLAYING_GAME;");
			}
		}
		
		FrontEndMenuManager.m_bFirstTime = false;
		FrontEndMenuManager.m_bWantToRestart = false;
#endif
	}
	

#ifndef MASTER
	if ( gbModelViewer )
		CAnimViewer::Shutdown();
	else
#endif
	if ( gGameState == GS_PLAYING_GAME )
		CGame::ShutDown();

	DMAudio.Terminate();
	
	_psFreeVideoModeList();


	/*
	 * Tidy up the 3D (RenderWare) components of the application...
	 */
	RsEventHandler(rsRWTERMINATE, nil);

	/*
	 * Kill the window...
	 */
	DestroyWindow(PSGLOBAL(window));

	/*
	 * Free the platform dependent data...
	 */
	RsEventHandler(rsTERMINATE, nil);

	/* 
	 * Free the argv strings...
	 */
	free(argv);
	
	ShowCursor(TRUE);
	
	SystemParametersInfo(SPI_SETSTICKYKEYS, sizeof(STICKYKEYS), &SavedStickyKeys, SPIF_SENDCHANGE);
	SystemParametersInfo(SPI_SETPOWEROFFACTIVE, TRUE, nil, SPIF_SENDCHANGE);
	SystemParametersInfo(SPI_SETLOWPOWERACTIVE, TRUE, nil, SPIF_SENDCHANGE);
	SystemParametersInfo(SPI_SETSCREENSAVEACTIVE, TRUE, nil, SPIF_SENDCHANGE);

	SetErrorMode(0);

	return message.wParam;
}

/*
 *****************************************************************************
 */

#define DEVICE_AXIS_MIN -2000
#define DEVICE_AXIS_MAX 2000


HRESULT _InputInitialise()
{
	HRESULT hr;

	// Create a DInput object
	if( FAILED( hr = DirectInput8Create( GetModuleHandle(nil), DIRECTINPUT_VERSION, 
										IID_IDirectInput8, (VOID**)&PSGLOBAL(dinterface), nil ) ) )
		return hr;
		
	return S_OK;
}

HRESULT _InputInitialiseMouse()
{
	HRESULT hr;

	// Obtain an interface to the system mouse device.
	if( FAILED( hr = PSGLOBAL(dinterface)->CreateDevice( GUID_SysMouse, &PSGLOBAL(mouse), nil ) ) )
		return hr;
	
	// Set the data format to "mouse format" - a predefined data format 
	//
	// A data format specifies which controls on a device we
	// are interested in, and how they should be reported.
	//
	// This tells DirectInput that we will be passing a
	// DIMOUSESTATE2 structure to IDirectInputDevice::GetDeviceState.
	if( FAILED( hr = PSGLOBAL(mouse)->SetDataFormat( &c_dfDIMouse2 ) ) )
		return hr;
	
	if( FAILED( hr = PSGLOBAL(mouse)->SetCooperativeLevel( PSGLOBAL(window), DISCL_NONEXCLUSIVE | DISCL_FOREGROUND ) ) )
		return hr;
	
	// Acquire the newly created device
	PSGLOBAL(mouse)->Acquire();
	
	return S_OK;
}

RwV2d leftStickPos;
RwV2d rightStickPos;

HRESULT CapturePad(RwInt32 padID)
{
	HRESULT		hr;
	DIJOYSTATE2 js; 
	LPDIRECTINPUTDEVICE8 *pPad = nil;

	if( padID == 0 )
		pPad = &PSGLOBAL(joy1);
	else if( padID == 1)
		pPad = &PSGLOBAL(joy2);
	else
		assert("invalid padID");
	
	if ( nil == (*pPad) )
		return S_OK;
	
	// Poll the device to read the current state
	hr = (*pPad)->Poll();
	
	if( FAILED(hr) )
	{
		// DInput is telling us that the input stream has been
		// interrupted. We aren't tracking any state between polls, so
		// we don't have any special reset that needs to be done. We
		// just re-acquire and try again.
		hr = (*pPad)->Acquire();
		while( hr == DIERR_INPUTLOST ) 
			hr = (*pPad)->Acquire();

		// hr may be DIERR_OTHERAPPHASPRIO or other errors.	 This
		// may occur when the app is minimized or in the process of 
		// switching, so just try again later 
		
		if( FAILED(hr) )
			return hr; 
		
		hr = (*pPad)->Poll();
		if( FAILED(hr) )
			return hr; 
	}
	
	// Get the input's device state
	if( FAILED( hr = (*pPad)->GetDeviceState( sizeof(DIJOYSTATE2), &js ) ) )
		return hr; // The device should have been acquired during the Poll()
	
	if ( ControlsManager.m_bFirstCapture == true )
	{
		memcpy(&ControlsManager.m_OldState, &js, sizeof(DIJOYSTATE2));
		memcpy(&ControlsManager.m_NewState, &js, sizeof(DIJOYSTATE2));
		
		ControlsManager.m_bFirstCapture = false;
	}
	else
	{
		memcpy(&ControlsManager.m_OldState, &ControlsManager.m_NewState, sizeof(DIJOYSTATE2));
		memcpy(&ControlsManager.m_NewState, &js, sizeof(DIJOYSTATE2));
	}

	RsPadButtonStatus bs;
	bs.padID = padID;

	RsPadEventHandler(rsPADBUTTONUP, (void *)&bs);
	
	bool deviceAvailable = (*pPad) != nil;
	
	if ( deviceAvailable )
	{
		leftStickPos.x = (float)js.lX / (float)((DEVICE_AXIS_MAX - DEVICE_AXIS_MIN) / 2);
		leftStickPos.y = (float)js.lY / (float)((DEVICE_AXIS_MAX - DEVICE_AXIS_MIN) / 2);
		
		if (LOWORD(js.rgdwPOV[0]) != 0xFFFF)
		{
			float angle = DEGTORAD((float)js.rgdwPOV[0] / 100.0f);

			leftStickPos.x = Sin(angle);
			leftStickPos.y = -Cos(angle);
		}
		
		if ( AllValidWinJoys.m_aJoys[bs.padID].m_bHasAxisR && AllValidWinJoys.m_aJoys[bs.padID].m_bHasAxisZ )
		{
			rightStickPos.x = (float)js.lZ	/ (float)((DEVICE_AXIS_MAX - DEVICE_AXIS_MIN) / 2);
			rightStickPos.y = (float)js.lRz / (float)((DEVICE_AXIS_MAX - DEVICE_AXIS_MIN) / 2);
		}
	}
	
	{
		if (CPad::m_bMapPadOneToPadTwo)
			bs.padID = 1;
		
		RsPadEventHandler(rsPADBUTTONUP,   (void *)&bs);
		RsPadEventHandler(rsPADBUTTONDOWN, (void *)&bs);
	}
	
	{
		if (CPad::m_bMapPadOneToPadTwo)
			bs.padID = 1;
		
		CPad *pad = CPad::GetPad(bs.padID);

		if ( Abs(leftStickPos.x)  > 0.3f )
			pad->PCTempJoyState.LeftStickX	= (int32)(leftStickPos.x  * 128.0f);
		
		if ( Abs(leftStickPos.y)  > 0.3f )
			pad->PCTempJoyState.LeftStickY	= (int32)(leftStickPos.y  * 128.0f);
		
		if ( Abs(rightStickPos.x) > 0.3f )
			pad->PCTempJoyState.RightStickX = (int32)(rightStickPos.x * 128.0f);

		if ( Abs(rightStickPos.y) > 0.3f )
			pad->PCTempJoyState.RightStickY = (int32)(rightStickPos.y * 128.0f);
	}
	
	return S_OK;
}

void _InputInitialiseJoys()
{
	DIPROPDWORD prop;
	DIDEVCAPS devCaps;

	for ( int32 i = 0; i < _TODOCONST(2); i++ )
		AllValidWinJoys.ClearJoyInfo(i);
	
	_InputAddJoys();
	
	if ( PSGLOBAL(joy1) != nil )
	{
		devCaps.dwSize = sizeof(DIDEVCAPS);
		PSGLOBAL(joy1)->GetCapabilities(&devCaps);

		prop.diph.dwSize = sizeof(DIPROPDWORD);
		prop.diph.dwHeaderSize = sizeof(DIPROPHEADER);
		prop.diph.dwObj = 0;
		prop.diph.dwHow = 0;
		
		PSGLOBAL(joy1)->GetProperty(DIPROP_VIDPID, (LPDIPROPHEADER)&prop);
		AllValidWinJoys.m_aJoys[0].m_nVendorID = LOWORD(prop.dwData);
		AllValidWinJoys.m_aJoys[0].m_nProductID = HIWORD(prop.dwData);
		AllValidWinJoys.m_aJoys[0].m_bInitialised = true;
		
		ControlsManager.InitDefaultControlConfigJoyPad(devCaps.dwButtons);
	}
		
	if ( PSGLOBAL(joy2) != nil )
	{
		PSGLOBAL(joy2)->GetProperty(DIPROP_VIDPID, (LPDIPROPHEADER)&prop);
		AllValidWinJoys.m_aJoys[1].m_nVendorID = LOWORD(prop.dwData);
		AllValidWinJoys.m_aJoys[1].m_nProductID = HIWORD(prop.dwData);
		AllValidWinJoys.m_aJoys[1].m_bInitialised = true;
	}
}

void _InputAddJoyStick(LPDIRECTINPUTDEVICE8 lpDevice, INT num)
{
	DIDEVICEOBJECTINSTANCE objInst;

	objInst.dwSize = sizeof( DIDEVICEOBJECTINSTANCE );

	DIPROPRANGE range;
	range.diph.dwSize = sizeof(DIPROPRANGE);
	range.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	range.lMin = DEVICE_AXIS_MIN;
	range.lMax = DEVICE_AXIS_MAX;
	range.diph.dwHow = DIPH_BYOFFSET;

	// get the info about the object from the device

	range.diph.dwObj = DIJOFS_X;
	if ( lpDevice != nil )
	{
		if ( SUCCEEDED( lpDevice->GetObjectInfo( &objInst,	DIJOFS_X, DIPH_BYOFFSET ) ) )
		{
			if( FAILED( lpDevice->SetProperty( DIPROP_RANGE, (LPCDIPROPHEADER)&range ) ) ) 
				return;
			else
				;
		}
	}
	
	range.diph.dwObj = DIJOFS_Y;
	if ( lpDevice != nil )
	{
		if ( SUCCEEDED( lpDevice->GetObjectInfo( &objInst,	DIJOFS_Y, DIPH_BYOFFSET ) ) )
		{
			if( FAILED( lpDevice->SetProperty( DIPROP_RANGE, (LPCDIPROPHEADER)&range ) ) ) 
				return;
			else
				;
		}
	}
	
	range.diph.dwObj = DIJOFS_Z;
	if ( lpDevice != nil )
	{
		if ( SUCCEEDED( lpDevice->GetObjectInfo( &objInst,	DIJOFS_Z, DIPH_BYOFFSET ) ) )
		{
			if( FAILED( lpDevice->SetProperty( DIPROP_RANGE, (LPCDIPROPHEADER)&range ) ) ) 
				return;
			else
				AllValidWinJoys.m_aJoys[num].m_bHasAxisZ = true; // z rightStickPos.x
		}
	}
	
	range.diph.dwObj = DIJOFS_RZ;
	if ( lpDevice != nil )
	{
		if ( SUCCEEDED( lpDevice->GetObjectInfo( &objInst,	DIJOFS_RZ, DIPH_BYOFFSET ) ) )
		{
			if( FAILED( lpDevice->SetProperty( DIPROP_RANGE, (LPCDIPROPHEADER)&range ) ) ) 
				return;
			else
				AllValidWinJoys.m_aJoys[num].m_bHasAxisR = true; // r rightStickPos.y
		}
	}
}

HRESULT _InputAddJoys()
{
	HRESULT hr;
	
	hr = PSGLOBAL(dinterface)->EnumDevices(DI8DEVCLASS_GAMECTRL,  _InputEnumDevicesCallback, nil, DIEDFL_ATTACHEDONLY );
	 
	if( FAILED(hr) )
		return hr;
	
	if ( PSGLOBAL(joy1) == nil )
		return S_FALSE;
	
	_InputAddJoyStick(PSGLOBAL(joy1), 0);
	
	if ( PSGLOBAL(joy2) == nil )
		return S_OK;	// we have one device already so return OK and ignore second
	
	_InputAddJoyStick(PSGLOBAL(joy2), 1);
	
	return S_OK;
}

HRESULT _InputGetMouseState(DIMOUSESTATE2 *state)
{
	HRESULT		  hr;
	
	if ( PSGLOBAL(mouse) == nil )
		return S_FALSE;
	
	// Get the input's device state, and put the state in dims
	ZeroMemory( state, sizeof(DIMOUSESTATE2) );
	
	hr = PSGLOBAL(mouse)->GetDeviceState( sizeof(DIMOUSESTATE2), state );

	if( FAILED(hr) ) 
	{
		 // DirectInput may be telling us that the input stream has been
		// interrupted.	 We aren't tracking any state between polls, so
		// we don't have any special reset that needs to be done.
		// We just re-acquire and try again.
		
		// If input is lost then acquire and keep trying 
		hr = PSGLOBAL(mouse)->Acquire();
		while( hr == DIERR_INPUTLOST ) 
			hr = PSGLOBAL(mouse)->Acquire();
		
		ZeroMemory( state, sizeof(DIMOUSESTATE2) );
		hr = PSGLOBAL(mouse)->GetDeviceState( sizeof(DIMOUSESTATE2), state );
		
		return hr;
	}
	
	return S_OK;
}

void _InputShutdown()
{
	SAFE_RELEASE(PSGLOBAL(dinterface));
}

BOOL CALLBACK _InputEnumDevicesCallback( const DIDEVICEINSTANCE* pdidInstance, VOID* pContext )
{
	HRESULT hr;
	 
	static INT Count = 0;
	
	LPDIRECTINPUTDEVICE8 *pJoystick = nil;
	
	if ( Count == 0 )
		pJoystick = &PSGLOBAL(joy1);
	else if ( Count == 1 )
		pJoystick = &PSGLOBAL(joy2);
	else
		assert("too many pads");
	
	// Obtain an interface to the enumerated joystick. 
	hr = PSGLOBAL(dinterface)->CreateDevice( pdidInstance->guidInstance, pJoystick, nil );
	
	// If it failed, then we can't use this joystick. (Maybe the user unplugged
	// it while we were in the middle of enumerating it.)
	if( hr != S_OK )
		return DIENUM_CONTINUE;

	hr = (*pJoystick)->SetDataFormat( &c_dfDIJoystick2 );
	if( hr != S_OK )
	{
		(*pJoystick)->Release();
		return DIENUM_CONTINUE;
	}
	
	++Count;

	hr = (*pJoystick)->SetCooperativeLevel( PSGLOBAL(window), DISCL_NONEXCLUSIVE|DISCL_FOREGROUND );
	if( hr != S_OK )
	{
		(*pJoystick)->Release();
#ifdef FIX_BUGS
		// BUG: enum will be called with Count == 2, which will write to a null pointer
		// So decrement count again since we're not using this pad
		--Count;
#endif
		return DIENUM_CONTINUE;
	}
	
	// Stop enumeration. Note: we're just taking the first two joysticks we get. You
	// could store all the enumerated joysticks and let the user pick.
	if ( Count == 2 )
		return DIENUM_STOP;
	
	return DIENUM_CONTINUE;
}

BOOL _InputTranslateKey(RsKeyCodes *rs, UINT flag, UINT key)
{
	*rs = rsNULL;
	
	switch ( key )
	{
		case VK_SHIFT:
		{
			if ( _dwOperatingSystemVersion == OS_WIN98 )
				*rs = rsSHIFT;
			break;
		}
		
		case VK_RETURN:
		{
			if ( _InputIsExtended(flag) )
				*rs = rsPADENTER;
			else
				*rs = rsENTER;
			break;
		}
		
		case VK_CONTROL:
		{
			if ( _InputIsExtended(flag) )
				*rs = rsRCTRL;
			else
				*rs = rsLCTRL;
			break;
		}
		
		case VK_MENU:
		{
			if ( _InputIsExtended(flag) )
				*rs = rsRALT;
			else
				*rs = rsLALT;
			break;
		}
		
		case VK_APPS:
		{
			*rs = rsAPPS;
			break;
		}
		
		case VK_PAUSE:
		{
			*rs = rsPAUSE;
			break;
		}
		
		case VK_CAPITAL:
		{
			*rs = rsCAPSLK;
			break;
		}
		
		case VK_ESCAPE:
		{
			*rs = rsESC;
			break;
		}
		
		case VK_PRIOR:
		{
			if ( _InputIsExtended(flag) )
				*rs = rsPGUP;
			else
				*rs = rsPADPGUP;
			break;
		}
		
		case VK_NEXT:
		{
			if ( _InputIsExtended(flag) )
				*rs = rsPGDN;
			else
				*rs = rsPADPGDN;
			break;
		}
		
		case VK_END:
		{
			if ( _InputIsExtended(flag) )
				*rs = rsEND;
			else
				*rs = rsPADEND;
			break;
		}
		
		case VK_HOME:
		{
			if ( _InputIsExtended(flag) )
				*rs = rsHOME;
			else
				*rs = rsPADHOME;
			break;
		}
		
		case VK_LEFT:
		{
			if ( _InputIsExtended(flag) )
				*rs = rsLEFT;
			else
				*rs = rsPADLEFT;
			break;
		}
		
		case VK_UP:
		{
			if ( _InputIsExtended(flag) )
				*rs = rsUP;
			else
				*rs = rsPADUP;
			break;
		}
		
		case VK_RIGHT:
		{
			if ( _InputIsExtended(flag) )
				*rs = rsRIGHT;
			else
				*rs = rsPADRIGHT;
			break;
		}
		
		case VK_DOWN:
		{
			if ( _InputIsExtended(flag) )
				*rs = rsDOWN;
			else
				*rs = rsPADDOWN;
			break;
		}
		
		case VK_INSERT:
		{
			if ( _InputIsExtended(flag) )
				*rs = rsINS;
			else
				*rs = rsPADINS;
			break;
		}
		
		case VK_DELETE:
		{
			if ( _InputIsExtended(flag) )
				*rs = rsDEL;
			else
				*rs = rsPADDEL;
			break;
		}
		
		case VK_LWIN:
		{
			*rs = rsLWIN;
			break;
		}
		
		case VK_RWIN:
		{
			*rs = rsRWIN;
			break;
		}
		
		case VK_NUMPAD0:
		{
			*rs = rsPADINS;
			break;
		}
		
		case VK_NUMPAD1:
		{
			*rs = rsPADEND;
			break;
		}
		
		case VK_NUMPAD2:
		{
			*rs = rsPADDOWN;
			break;
		}
		
		case VK_NUMPAD3:
		{
			*rs = rsPADPGDN;
			break;
		}
		
		case VK_NUMPAD4:
		{
			*rs = rsPADLEFT;
			break;
		}
		
		case VK_NUMPAD5:
		{
			*rs = rsPAD5;
			break;
		}
		
		case VK_NUMPAD6:
		{
			*rs = rsPADRIGHT;
			break;
		}
		
		case VK_NUMPAD7:
		{
			*rs = rsPADHOME;
			break;
		}
		
		case VK_NUMPAD8:
		{
			*rs = rsPADUP;
			break;
		}
		
		case VK_NUMPAD9:
		{
			*rs = rsPADPGUP;
			break;
		}
		
		case VK_MULTIPLY:
		{
			*rs = rsTIMES;
			break;
		}
		
		case VK_DIVIDE:
		{
			*rs = rsDIVIDE;
			break;
		}
		
		case VK_ADD:
		{
			*rs = rsPLUS;
			break;
		}
		
		case VK_SUBTRACT:
		{
			*rs = rsMINUS;
			break;
		}
		
		case VK_DECIMAL:
		{
			*rs = rsPADDEL;
			break;
		}
		
		case VK_F1:
		{
			*rs = rsF1;
			break;
		}
		
		case VK_F2:
		{
			*rs = rsF2;
			break;
		}
		
		case VK_F3:
		{
			*rs = rsF3;
			break;
		}
		
		case VK_F4:
		{
			*rs = rsF4;
			break;
		}
		
		case VK_F5:
		{
			*rs = rsF5;
			break;
		}
		
		case VK_F6:
		{
			*rs = rsF6;
			break;
		}
		
		case VK_F7:
		{
			*rs = rsF7;
			break;
		}
		
		case VK_F8:
		{
			*rs = rsF8;
			break;
		}
		
		case VK_F9:	
		{
			*rs = rsF9;
			break;
		}
		
		case VK_F10:
		{
			*rs = rsF10;
			break;
		}
		
		case VK_F11:
		{
			*rs = rsF11;
			break;
		}
		
		case VK_F12:
		{
			*rs = rsF12;
			break;
		}
		
		case VK_NUMLOCK:
		{
			*rs = rsNUMLOCK;
			break;
		}
		
		case VK_SCROLL:
		{
			*rs = rsSCROLL;
			break;
		}
		
		case VK_BACK:
		{
			*rs = rsBACKSP;
			break;
		}
		
		case VK_TAB:
		{
			*rs = rsTAB;
			break;
		}
		
		default:
		{
			UINT vkey = MapVirtualKey(key, MAPVK_VK_TO_CHAR) & 0xFFFF;
			if ( vkey < 255 )
				*rs = (RsKeyCodes)vkey;
			break;
		}
	}
	
	return *rs != rsNULL;
}

void _InputTranslateShiftKeyUpDown(RsKeyCodes *rs)
{
	if ( _dwOperatingSystemVersion != OS_WIN98 )
	{
		if ( _InputTranslateShiftKey(rs, VK_LSHIFT, TRUE) )
			RsKeyboardEventHandler(rsKEYDOWN, rs);
		if ( _InputTranslateShiftKey(rs, VK_RSHIFT, TRUE) )
			RsKeyboardEventHandler(rsKEYDOWN, rs);
		if ( _InputTranslateShiftKey(rs, VK_LSHIFT, FALSE) )
			RsKeyboardEventHandler(rsKEYUP, rs);
		if ( _InputTranslateShiftKey(rs, VK_RSHIFT, FALSE) )
			RsKeyboardEventHandler(rsKEYUP, rs);
	}
}

BOOL _InputTranslateShiftKey(RsKeyCodes *rs, UINT key, BOOLEAN bDown)
{
	*rs = rsNULL;
	switch ( key )
	{
		case VK_LSHIFT:
		{
			if ( bDown == (GetKeyState(VK_LSHIFT) & 0x8000) >> 15 )
				*rs = rsLSHIFT;
			break;
		}
		
		case VK_RSHIFT:
		{
			if ( bDown == (GetKeyState(VK_RSHIFT) & 0x8000) >> 15 )
				*rs = rsRSHIFT;
			break;
		}
		
		default:
		{
			return *rs != rsNULL;
		}
	}
	
	return TRUE;
}

BOOL _InputIsExtended(INT flag)
{
	return (flag & 0x1000000) != 0;
}

#if (defined(_MSC_VER))
int strcasecmp(const char *str1, const char *str2)
{
	return _strcmpi(str1, str2);
}
#endif
#endif
