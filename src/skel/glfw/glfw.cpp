#if defined RW_GL3 && !defined LIBRW_SDL2

#define WITHWINDOWS
#include "common.h"

#pragma warning( push )
#pragma warning( disable : 4005)
#pragma warning( pop )

#if (defined(_MSC_VER))
#include <tchar.h>
#endif /* (defined(_MSC_VER)) */
#include <stdio.h>
#include "rwcore.h"
#include "skeleton.h"
#include "platform.h"
#include "crossplatform.h"

#include "patcher.h"
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
#include "Sprite2d.h"
#include "AnimViewer.h"


#define MAX_SUBSYSTEMS		(16)


rw::EngineOpenParams openParams;

static RwBool		  ForegroundApp = TRUE;

static RwBool		  RwInitialised = FALSE;

static RwSubSystemInfo GsubSysInfo[MAX_SUBSYSTEMS];
static RwInt32		GnumSubSystems = 0;
static RwInt32		GcurSel = 0, GcurSelVM = 0;

static RwBool useDefault;

// What is that for anyway?
#ifndef IMPROVED_VIDEOMODE
static RwBool defaultFullscreenRes = TRUE;
#else
static RwBool defaultFullscreenRes = FALSE;
static RwInt32 bestWndMode = -1;
#endif

static psGlobalType PsGlobal;


#define PSGLOBAL(var) (((psGlobalType *)(RsGlobal.ps))->var)

#undef MAKEPOINTS
#define MAKEPOINTS(l)		(*((POINTS /*FAR*/ *)&(l)))

#define SAFE_RELEASE(x) { if (x) x->Release(); x = NULL; }
#define JIF(x) if (FAILED(hr=(x))) \
	{debug(TEXT("FAILED(hr=0x%x) in ") TEXT(#x) TEXT("\n"), hr); return;}

unsigned long _dwMemAvailPhys;
RwUInt32 gGameState;

#ifdef _WIN32
DWORD _dwOperatingSystemVersion;
#include "resource.h"
#else
long _dwOperatingSystemVersion;
#include <sys/sysinfo.h>
#include <stddef.h>
#include <locale.h>
#include <signal.h>
#include <errno.h>
#endif
/*
 *****************************************************************************
 */
void _psCreateFolder(const char *path)
{
#ifdef _WIN32
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
#else
	struct stat info;
	char fullpath[PATH_MAX];
	realpath(path, fullpath);

	if (lstat(fullpath, &info) != 0) {
		if (errno == ENOENT || (errno != EACCES && !S_ISDIR(info.st_mode))) {
			mkdir(fullpath, 0755);
		}
	}
#endif
}

/*
 *****************************************************************************
 */
const char *_psGetUserFilesFolder()
{
#if defined USE_MY_DOCUMENTS && defined _WIN32
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
	static char szUserFiles[256];
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
	if (FrontEndMenuManager.m_PrefsVsync)
		RwCameraShowRaster(camera, PSGLOBAL(window), rwRASTERFLIPWAITVSYNC);
	else
		RwCameraShowRaster(camera, PSGLOBAL(window), rwRASTERFLIPDONTWAIT);

	return;
}


/*
 *****************************************************************************
 */
#ifdef _WIN32
#pragma comment( lib, "Winmm.lib" ) // Needed for time
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
#else
double
psTimer(void)
{
	struct timespec start; 
	clock_gettime(CLOCK_MONOTONIC_RAW, &start);
	return start.tv_sec * 1000.0 + start.tv_nsec/1000000.0;
}
#endif       


/*
 *****************************************************************************
 */
void
psMouseSetPos(RwV2d *pos)
{
	glfwSetCursorPos(PSGLOBAL(window), pos->x, pos->y);
	
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
	return nil;
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
	return true;
}

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
psInitialise(void)
{
	PsGlobal.lastMousePos.x = PsGlobal.lastMousePos.y = 0.0f;

	RsGlobal.ps = &PsGlobal;
	
	PsGlobal.fullScreen = FALSE;
	
	PsGlobal.joy1id	= -1;
	PsGlobal.joy2id	= -1;

	CFileMgr::Initialise();
	
	C_PcSave::SetSaveDirectory(_psGetUserFilesFolder());
	
	InitialiseLanguage();

	FrontEndMenuManager.LoadSettings();
	
	gGameState = GS_START_UP;
	TRACE("gGameState = GS_START_UP");
#ifdef _WIN32
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
		if ( verInfo.dwMajorVersion > 4 || verInfo.dwMajorVersion == 4 && verInfo.dwMinorVersion == 1 )
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

	MEMORYSTATUS memstats;
	GlobalMemoryStatus(&memstats);

	_dwMemAvailPhys = memstats.dwAvailPhys;

	debug("Physical memory size %u\n", memstats.dwTotalPhys);
	debug("Available physical memory %u\n", memstats.dwAvailPhys);
#else
	struct sysinfo systemInfo;
	sysinfo(&systemInfo);
	
	_dwMemAvailPhys = systemInfo.freeram;
	_dwOperatingSystemVersion = OS_WINXP; // To fool other classes

	debug("Physical memory size %u\n", systemInfo.totalram);
	debug("Available physical memory %u\n", systemInfo.freeram);

#endif
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
			_VMList[i] = (RwChar*)RwCalloc(100, sizeof(RwChar));
			rwsprintf(_VMList[i],"%d X %d X %d", vm.width, vm.height, vm.depth);
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
	
	if (!_psSetVideoMode(RwEngineGetCurrentSubSystem(), videoMode))
	{
		RsGlobal.quit = TRUE;

		printf("ERROR: Failed to select new screen resolution\n");
	}
	else
		FrontEndMenuManager.LoadAllTextures();
}

/*
 *****************************************************************************
 */

RwBool IsForegroundApp()
{
	return !!ForegroundApp;
}
/*
UINT GetBestRefreshRate(UINT width, UINT height, UINT depth)
{
	LPDIRECT3D8 d3d = Direct3DCreate8(D3D_SDK_VERSION);
	
	ASSERT(d3d != nil);
	
	UINT refreshRate = INT_MAX;
	D3DFORMAT format;

	if ( depth == 32 )
		format = D3DFMT_X8R8G8B8;
	else if ( depth == 24 )
		format = D3DFMT_R8G8B8;
	else
		format = D3DFMT_R5G6B5;
	
	UINT modeCount = d3d->GetAdapterModeCount(GcurSel);
	
	for ( UINT i = 0; i < modeCount; i++ )
	{
		D3DDISPLAYMODE mode;
		
		d3d->EnumAdapterModes(GcurSel, i, &mode);
		
		if ( mode.Width == width && mode.Height == height && mode.Format == format )
		{
			if ( mode.RefreshRate == 0 )
				return 0;

			if ( mode.RefreshRate < refreshRate && mode.RefreshRate >= 60 )
				refreshRate = mode.RefreshRate;
		}
	}
	
#ifdef FIX_BUGS
	d3d->Release();
#endif
	
	if ( refreshRate == -1 )
		return -1;

	return refreshRate;
}
*/
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
				printf("WARNING: Cannot find 640x480 video mode, selecting device cancelled\n");
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
			const GLFWvidmode *mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
			FrontEndMenuManager.m_nPrefsWidth = mode->width;
			FrontEndMenuManager.m_nPrefsHeight = mode->height;
			FrontEndMenuManager.m_nPrefsDepth = 32;
			FrontEndMenuManager.m_nPrefsWindowed = 0;
		}

		// Find the videomode that best fits what we got from the settings file
		RwInt32 bestFsMode = -1;
		RwInt32 bestWidth = -1;
		RwInt32 bestHeight = -1;
		RwInt32 bestDepth = -1;
		for(GcurSelVM = 0; GcurSelVM < RwEngineGetNumVideoModes(); GcurSelVM++){
			RwEngineGetVideoModeInfo(&vm, GcurSelVM);

			if (!(vm.flags & rwVIDEOMODEEXCLUSIVE)){
				bestWndMode = GcurSelVM;
			} else {
				// try the largest one that isn't larger than what we wanted
				if(vm.width >= bestWidth && vm.width <= FrontEndMenuManager.m_nPrefsWidth &&
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
			printf("WARNING: Cannot find desired video mode, selecting device cancelled\n");
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

	FrontEndMenuManager.m_nCurrOption = 0;
	
	/* Set up the video mode and set the apps window
	* dimensions to match */
	if (!RwEngineSetVideoMode(GcurSelVM))
	{
		return FALSE;
	}
	/*
	TODO
	if (vm.flags & rwVIDEOMODEEXCLUSIVE)
	{
		debug("%dx%dx%d", vm.width, vm.height, vm.depth);
		
		UINT refresh = GetBestRefreshRate(vm.width, vm.height, vm.depth);
		
		if ( refresh != (UINT)-1 )
		{
			debug("refresh %d", refresh);
			RwD3D8EngineSetRefreshRate((RwUInt32)refresh);
		}
	}
	*/
#ifndef IMPROVED_VIDEOMODE
	if (vm.flags & rwVIDEOMODEEXCLUSIVE)
	{
		RsGlobal.maximumWidth = vm.width;
		RsGlobal.maximumHeight = vm.height;
		RsGlobal.width = vm.width;
		RsGlobal.height = vm.height;
		
		PSGLOBAL(fullScreen) = TRUE;
	}
#else
		RsGlobal.maximumWidth = FrontEndMenuManager.m_nPrefsWidth;
		RsGlobal.maximumHeight = FrontEndMenuManager.m_nPrefsHeight;
		RsGlobal.width = FrontEndMenuManager.m_nPrefsWidth;
		RsGlobal.height = FrontEndMenuManager.m_nPrefsHeight;
		
		PSGLOBAL(fullScreen) = !FrontEndMenuManager.m_nPrefsWindowed;
#endif
	
	return TRUE;
}

void keypressCB(GLFWwindow* window, int key, int scancode, int action, int mods);
void resizeCB(GLFWwindow* window, int width, int height);
void scrollCB(GLFWwindow* window, double xoffset, double yoffset);
void cursorCB(GLFWwindow* window, double xpos, double ypos);
void joysChangeCB(int jid, int event);

bool IsThisJoystickBlacklisted(int i)
{
	const char *joyname = glfwGetJoystickName(i);

	// this is just a keyboard and mouse
	// Microsoft Microsoft® 2.4GHz Transceiver v8.0 Consumer Control
	// Microsoft Microsoft® 2.4GHz Transceiver v8.0 System Control
	if(strstr(joyname, "2.4GHz Transceiver"))
		return true;

	return false;
}

void _InputInitialiseJoys()
{
	for (int i = 0; i <= GLFW_JOYSTICK_LAST; i++) {
		if (glfwJoystickPresent(i) && !IsThisJoystickBlacklisted(i)) {
			if (PSGLOBAL(joy1id) == -1)
				PSGLOBAL(joy1id) = i;
			else if (PSGLOBAL(joy2id) == -1)
				PSGLOBAL(joy2id) = i;
			else
				break;
		}
	}

	if (PSGLOBAL(joy1id) != -1) {
		int count;
		glfwGetJoystickButtons(PSGLOBAL(joy1id), &count);
		ControlsManager.InitDefaultControlConfigJoyPad(count);
	}
}

void _InputInitialiseMouse()
{
	glfwSetInputMode(PSGLOBAL(window), GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
}

void psPostRWinit(void)
{
	RwVideoMode vm;
	RwEngineGetVideoModeInfo(&vm, GcurSelVM);

	glfwSetKeyCallback(PSGLOBAL(window), keypressCB);
	glfwSetWindowSizeCallback(PSGLOBAL(window), resizeCB);
	glfwSetScrollCallback(PSGLOBAL(window), scrollCB);
	glfwSetCursorPosCallback(PSGLOBAL(window), cursorCB);
	glfwSetJoystickCallback(joysChangeCB);

	_InputInitialiseJoys();
	_InputInitialiseMouse();

	if(!(vm.flags & rwVIDEOMODEEXCLUSIVE))
		glfwSetWindowSize(PSGLOBAL(window), RsGlobal.maximumWidth, RsGlobal.maximumHeight);

	// Make sure all keys are released
	CPad::GetPad(0)->Clear(true);
	CPad::GetPad(1)->Clear(true);
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
	
	if ( RsEventHandler(rsRWINITIALISE, &openParams) == rsEVENTERROR )
		return FALSE;

	RwInitialised = TRUE;
	useDefault = FALSE;
	
	RwRect r;
	
	r.x = 0;
	r.y = 0;
	r.w = RsGlobal.maximumWidth;
	r.h = RsGlobal.maximumHeight;

	RsEventHandler(rsCAMERASIZE, &r);
	
	psPostRWinit();
	
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

	len = strlen(cmdLine);

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
#ifndef _WIN32
	// Mandatory for Linux(Unix? Posix?) to set lang. to environment lang.
	setlocale(LC_ALL, "");	

	char *systemLang, *keyboardLang;

	systemLang = setlocale (LC_ALL, NULL);
	keyboardLang = setlocale (LC_CTYPE, NULL);
	
	short primUserLCID, primSystemLCID;
	primUserLCID = primSystemLCID = !strncmp(systemLang, "fr_",3) ? LANG_FRENCH :
					!strncmp(systemLang, "de_",3) ? LANG_GERMAN :
					!strncmp(systemLang, "en_",3) ? LANG_ENGLISH :
					!strncmp(systemLang, "it_",3) ? LANG_ITALIAN :
					!strncmp(systemLang, "es_",3) ? LANG_SPANISH :
					LANG_OTHER;

	short primLayout = !strncmp(keyboardLang, "fr_",3) ? LANG_FRENCH : (!strncmp(keyboardLang, "de_",3) ? LANG_GERMAN : LANG_ENGLISH);

	short subUserLCID, subSystemLCID;
	subUserLCID = subSystemLCID = !strncmp(systemLang, "en_AU",5) ? SUBLANG_ENGLISH_AUS : SUBLANG_OTHER;
	short subLayout = !strncmp(keyboardLang, "en_AU",5) ? SUBLANG_ENGLISH_AUS : SUBLANG_OTHER;

#else
	WORD primUserLCID	= PRIMARYLANGID(GetSystemDefaultLCID());
	WORD primSystemLCID = PRIMARYLANGID(GetUserDefaultLCID());
	WORD primLayout		= PRIMARYLANGID((DWORD)GetKeyboardLayout(0));
	
	WORD subUserLCID	= SUBLANGID(GetSystemDefaultLCID());
	WORD subSystemLCID	= SUBLANGID(GetUserDefaultLCID());
	WORD subLayout		= SUBLANGID((DWORD)GetKeyboardLayout(0));
#endif
	if (   primUserLCID	  == LANG_GERMAN
		|| primSystemLCID == LANG_GERMAN
		|| primLayout	  == LANG_GERMAN )
	{
		CGame::nastyGame = false;
		FrontEndMenuManager.m_PrefsAllowNastyGame = false;
		CGame::germanGame = true;
	}
	
	if (   primUserLCID	  == LANG_FRENCH
		|| primSystemLCID == LANG_FRENCH
		|| primLayout	  == LANG_FRENCH )
	{
		CGame::nastyGame = false;
		FrontEndMenuManager.m_PrefsAllowNastyGame = false;
		CGame::frenchGame = true;
	}
	
	if (   subUserLCID	 == SUBLANG_ENGLISH_AUS
		|| subSystemLCID == SUBLANG_ENGLISH_AUS
		|| subLayout	 == SUBLANG_ENGLISH_AUS )
		CGame::noProstitutes = true;

#ifdef NASTY_GAME
	CGame::nastyGame = true;
	FrontEndMenuManager.m_PrefsAllowNastyGame = true;
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
	
	FrontEndMenuManager.OS_Language = primUserLCID;

	switch ( lang )
	{
		case LANG_GERMAN:
		{
			FrontEndMenuManager.m_PrefsLanguage = LANGUAGE_GERMAN;
			break;
		}
		case LANG_SPANISH:
		{
			FrontEndMenuManager.m_PrefsLanguage = LANGUAGE_SPANISH;
			break;
		}
		case LANG_FRENCH:
		{
			FrontEndMenuManager.m_PrefsLanguage = LANGUAGE_FRENCH;
			break;
		}
		case LANG_ITALIAN:
		{
			FrontEndMenuManager.m_PrefsLanguage = LANGUAGE_ITALIAN;
			break;
		}
		default:
		{
			FrontEndMenuManager.m_PrefsLanguage = LANGUAGE_AMERICAN;
			break;
		}
	}

	TheText.Unload();
	TheText.Load();

#ifndef _WIN32
	// TODO this is needed for strcasecmp to work correctly across all languages, but can these cause other problems??
	setlocale(LC_CTYPE, "C");
	setlocale(LC_COLLATE, "C");
	setlocale(LC_NUMERIC, "C");
#endif
}

/*
 *****************************************************************************
 */

void HandleExit()
{
#ifdef _WIN32
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
#else
	// We now handle terminate message always, why handle on some cases?
	return;
#endif
}

#ifndef _WIN32
void terminateHandler(int sig, siginfo_t *info, void *ucontext) {
	RsGlobal.quit = TRUE;
}

void dummyHandler(int sig){
}
#endif

void resizeCB(GLFWwindow* window, int width, int height) {
	/*
	* Handle event to ensure window contents are displayed during re-size
	* as this can be disabled by the user, then if there is not enough
	* memory things don't work.
	*/
	/* redraw window */
	if (RwInitialised && (gGameState == GS_PLAYING_GAME || gGameState == GS_ANIMVIEWER))
	{
		RsEventHandler((gGameState == GS_PLAYING_GAME ? rsIDLE : rsANIMVIEWER), (void*)TRUE);
	}

	if (RwInitialised && height > 0 && width > 0) {
		RwRect r;

		// TODO fix artifacts of resizing with mouse
		RsGlobal.maximumHeight = height;
		RsGlobal.maximumWidth = width;

		r.x = 0;
		r.y = 0;
		r.w = width;
		r.h = height;

		RsEventHandler(rsCAMERASIZE, &r);
	}
//	glfwSetWindowPos(window, 0, 0);
}

void scrollCB(GLFWwindow* window, double xoffset, double yoffset) {
	PSGLOBAL(mouseWheel) = yoffset;
}

int keymap[GLFW_KEY_LAST + 1];

static void
initkeymap(void)
{
	int i;
	for (i = 0; i < GLFW_KEY_LAST + 1; i++)
		keymap[i] = rsNULL;

	keymap[GLFW_KEY_SPACE] = ' ';
	keymap[GLFW_KEY_APOSTROPHE] = '\'';
	keymap[GLFW_KEY_COMMA] = ',';
	keymap[GLFW_KEY_MINUS] = '-';
	keymap[GLFW_KEY_PERIOD] = '.';
	keymap[GLFW_KEY_SLASH] = '/';
	keymap[GLFW_KEY_0] = '0';
	keymap[GLFW_KEY_1] = '1';
	keymap[GLFW_KEY_2] = '2';
	keymap[GLFW_KEY_3] = '3';
	keymap[GLFW_KEY_4] = '4';
	keymap[GLFW_KEY_5] = '5';
	keymap[GLFW_KEY_6] = '6';
	keymap[GLFW_KEY_7] = '7';
	keymap[GLFW_KEY_8] = '8';
	keymap[GLFW_KEY_9] = '9';
	keymap[GLFW_KEY_SEMICOLON] = ';';
	keymap[GLFW_KEY_EQUAL] = '=';
	keymap[GLFW_KEY_A] = 'A';
	keymap[GLFW_KEY_B] = 'B';
	keymap[GLFW_KEY_C] = 'C';
	keymap[GLFW_KEY_D] = 'D';
	keymap[GLFW_KEY_E] = 'E';
	keymap[GLFW_KEY_F] = 'F';
	keymap[GLFW_KEY_G] = 'G';
	keymap[GLFW_KEY_H] = 'H';
	keymap[GLFW_KEY_I] = 'I';
	keymap[GLFW_KEY_J] = 'J';
	keymap[GLFW_KEY_K] = 'K';
	keymap[GLFW_KEY_L] = 'L';
	keymap[GLFW_KEY_M] = 'M';
	keymap[GLFW_KEY_N] = 'N';
	keymap[GLFW_KEY_O] = 'O';
	keymap[GLFW_KEY_P] = 'P';
	keymap[GLFW_KEY_Q] = 'Q';
	keymap[GLFW_KEY_R] = 'R';
	keymap[GLFW_KEY_S] = 'S';
	keymap[GLFW_KEY_T] = 'T';
	keymap[GLFW_KEY_U] = 'U';
	keymap[GLFW_KEY_V] = 'V';
	keymap[GLFW_KEY_W] = 'W';
	keymap[GLFW_KEY_X] = 'X';
	keymap[GLFW_KEY_Y] = 'Y';
	keymap[GLFW_KEY_Z] = 'Z';
	keymap[GLFW_KEY_LEFT_BRACKET] = '[';
	keymap[GLFW_KEY_BACKSLASH] = '\\';
	keymap[GLFW_KEY_RIGHT_BRACKET] = ']';
	keymap[GLFW_KEY_GRAVE_ACCENT] = '`';
	keymap[GLFW_KEY_ESCAPE] = rsESC;
	keymap[GLFW_KEY_ENTER] = rsENTER;
	keymap[GLFW_KEY_TAB] = rsTAB;
	keymap[GLFW_KEY_BACKSPACE] = rsBACKSP;
	keymap[GLFW_KEY_INSERT] = rsINS;
	keymap[GLFW_KEY_DELETE] = rsDEL;
	keymap[GLFW_KEY_RIGHT] = rsRIGHT;
	keymap[GLFW_KEY_LEFT] = rsLEFT;
	keymap[GLFW_KEY_DOWN] = rsDOWN;
	keymap[GLFW_KEY_UP] = rsUP;
	keymap[GLFW_KEY_PAGE_UP] = rsPGUP;
	keymap[GLFW_KEY_PAGE_DOWN] = rsPGDN;
	keymap[GLFW_KEY_HOME] = rsHOME;
	keymap[GLFW_KEY_END] = rsEND;
	keymap[GLFW_KEY_CAPS_LOCK] = rsCAPSLK;
	keymap[GLFW_KEY_SCROLL_LOCK] = rsSCROLL;
	keymap[GLFW_KEY_NUM_LOCK] = rsNUMLOCK;
	keymap[GLFW_KEY_PRINT_SCREEN] = rsNULL;
	keymap[GLFW_KEY_PAUSE] = rsPAUSE;

	keymap[GLFW_KEY_F1] = rsF1;
	keymap[GLFW_KEY_F2] = rsF2;
	keymap[GLFW_KEY_F3] = rsF3;
	keymap[GLFW_KEY_F4] = rsF4;
	keymap[GLFW_KEY_F5] = rsF5;
	keymap[GLFW_KEY_F6] = rsF6;
	keymap[GLFW_KEY_F7] = rsF7;
	keymap[GLFW_KEY_F8] = rsF8;
	keymap[GLFW_KEY_F9] = rsF9;
	keymap[GLFW_KEY_F10] = rsF10;
	keymap[GLFW_KEY_F11] = rsF11;
	keymap[GLFW_KEY_F12] = rsF12;
	keymap[GLFW_KEY_F13] = rsNULL;
	keymap[GLFW_KEY_F14] = rsNULL;
	keymap[GLFW_KEY_F15] = rsNULL;
	keymap[GLFW_KEY_F16] = rsNULL;
	keymap[GLFW_KEY_F17] = rsNULL;
	keymap[GLFW_KEY_F18] = rsNULL;
	keymap[GLFW_KEY_F19] = rsNULL;
	keymap[GLFW_KEY_F20] = rsNULL;
	keymap[GLFW_KEY_F21] = rsNULL;
	keymap[GLFW_KEY_F22] = rsNULL;
	keymap[GLFW_KEY_F23] = rsNULL;
	keymap[GLFW_KEY_F24] = rsNULL;
	keymap[GLFW_KEY_F25] = rsNULL;
	keymap[GLFW_KEY_KP_0] = rsPADINS;
	keymap[GLFW_KEY_KP_1] = rsPADEND;
	keymap[GLFW_KEY_KP_2] = rsPADDOWN;
	keymap[GLFW_KEY_KP_3] = rsPADPGDN;
	keymap[GLFW_KEY_KP_4] = rsPADLEFT;
	keymap[GLFW_KEY_KP_5] = rsPAD5;
	keymap[GLFW_KEY_KP_6] = rsPADRIGHT;
	keymap[GLFW_KEY_KP_7] = rsPADHOME;
	keymap[GLFW_KEY_KP_8] = rsPADUP;
	keymap[GLFW_KEY_KP_9] = rsPADPGUP;
	keymap[GLFW_KEY_KP_DECIMAL] = rsPADDEL;
	keymap[GLFW_KEY_KP_DIVIDE] = rsDIVIDE;
	keymap[GLFW_KEY_KP_MULTIPLY] = rsTIMES;
	keymap[GLFW_KEY_KP_SUBTRACT] = rsMINUS;
	keymap[GLFW_KEY_KP_ADD] = rsPLUS;
	keymap[GLFW_KEY_KP_ENTER] = rsPADENTER;
	keymap[GLFW_KEY_KP_EQUAL] = rsNULL;
	keymap[GLFW_KEY_LEFT_SHIFT] = rsLSHIFT;
	keymap[GLFW_KEY_LEFT_CONTROL] = rsLCTRL;
	keymap[GLFW_KEY_LEFT_ALT] = rsLALT;
	keymap[GLFW_KEY_LEFT_SUPER] = rsLWIN;
	keymap[GLFW_KEY_RIGHT_SHIFT] = rsRSHIFT;
	keymap[GLFW_KEY_RIGHT_CONTROL] = rsRCTRL;
	keymap[GLFW_KEY_RIGHT_ALT] = rsRALT;
	keymap[GLFW_KEY_RIGHT_SUPER] = rsRWIN;
	keymap[GLFW_KEY_MENU] = rsNULL;
}

bool lshiftStatus = false;
bool rshiftStatus = false;

void
keypressCB(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key >= 0 && key <= GLFW_KEY_LAST) {
		RsKeyCodes ks = (RsKeyCodes)keymap[key];

		if (key == GLFW_KEY_LEFT_SHIFT)
			lshiftStatus = action != GLFW_RELEASE;

		if (key == GLFW_KEY_RIGHT_SHIFT)
			rshiftStatus = action != GLFW_RELEASE;

		if (action == GLFW_RELEASE) RsKeyboardEventHandler(rsKEYUP, &ks);
		else if (action == GLFW_PRESS) RsKeyboardEventHandler(rsKEYDOWN, &ks);
		else if (action == GLFW_REPEAT) RsKeyboardEventHandler(rsKEYDOWN, &ks);
	}
}

// R* calls that in ControllerConfig, idk why
void
_InputTranslateShiftKeyUpDown(RsKeyCodes *rs) {
	RsKeyboardEventHandler(lshiftStatus ? rsKEYDOWN : rsKEYUP, &(*rs = rsLSHIFT));
	RsKeyboardEventHandler(rshiftStatus ? rsKEYDOWN : rsKEYUP, &(*rs = rsRSHIFT));
}

// TODO this only works in frontend(and luckily only frontend use this), maybe because of glfw knows that mouse pos is > 32000 in game??
void
cursorCB(GLFWwindow* window, double xpos, double ypos) {
	FrontEndMenuManager.m_nMouseTempPosX = xpos;
	FrontEndMenuManager.m_nMouseTempPosY = ypos;
}

/*
 *****************************************************************************
 */
#ifdef _WIN32
int PASCAL
WinMain(HINSTANCE instance,
	HINSTANCE prevInstance	__RWUNUSED__,
	CMDSTR cmdLine,
	int cmdShow)
{

	RwInt32 argc;
	RwChar** argv;
	SystemParametersInfo(SPI_SETFOREGROUNDLOCKTIMEOUT, 0, nil, SPIF_SENDCHANGE);
#else
int
main(int argc, char *argv[])
{
#endif
	RwV2d pos;
	RwInt32 i;
//	StaticPatcher::Apply();

#ifndef _WIN32
	struct sigaction act;
	act.sa_sigaction = terminateHandler;
	act.sa_flags = SA_SIGINFO;
	sigaction(SIGTERM, &act, NULL);
	struct sigaction sa;
	sa.sa_handler = dummyHandler;
	sa.sa_flags = 0;
	sigaction(SIGINT, &sa, NULL); // Needed for CdStreamPosix
#endif

	/* 
	 * Initialize the platform independent data.
	 * This will in turn initialize the platform specific data...
	 */
	if( RsEventHandler(rsINITIALISE, nil) == rsEVENTERROR )
	{
		return FALSE;
	}

#ifdef _WIN32
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
#endif
	for(i=1; i<argc; i++)
	{
		RsEventHandler(rsPREINITCOMMANDLINE, argv[i]);
	}

	/*
	 * Parameters to be used in RwEngineOpen / rsRWINITIALISE event
	 */

	openParams.width = RsGlobal.maximumWidth;
	openParams.height = RsGlobal.maximumHeight;
	openParams.windowtitle = RsGlobal.appName;
	openParams.window = &PSGLOBAL(window);
	
	ControlsManager.MakeControllerActionsBlank();
	ControlsManager.InitDefaultControlConfiguration();

	/* 
	 * Initialize the 3D (RenderWare) components of the app...
	 */
	if( rsEVENTERROR == RsEventHandler(rsRWINITIALISE, &openParams) )
	{
		RsEventHandler(rsTERMINATE, nil);

		return 0;
	}

	psPostRWinit();

	ControlsManager.InitDefaultControlConfigMouse(MousePointerStateHelper.GetMouseSetUp());

//	glfwSetWindowPos(PSGLOBAL(window), 0, 0);

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
#ifdef _WIN32
	SystemParametersInfo(SPI_SETPOWEROFFACTIVE, FALSE, nil, SPIF_SENDCHANGE);
	SystemParametersInfo(SPI_SETLOWPOWERACTIVE, FALSE, nil, SPIF_SENDCHANGE);
	

	STICKYKEYS SavedStickyKeys;
	SavedStickyKeys.cbSize = sizeof(STICKYKEYS);
	
	SystemParametersInfo(SPI_GETSTICKYKEYS, sizeof(STICKYKEYS), &SavedStickyKeys, SPIF_SENDCHANGE);
	
	STICKYKEYS NewStickyKeys;
	NewStickyKeys.cbSize = sizeof(STICKYKEYS);
	NewStickyKeys.dwFlags = SKF_TWOKEYSOFF;
	
	SystemParametersInfo(SPI_SETSTICKYKEYS, sizeof(STICKYKEYS), &NewStickyKeys, SPIF_SENDCHANGE);
#endif

	{
		CFileMgr::SetDirMyDocuments();
		
		int32 gta3set = CFileMgr::OpenFile("gta3.set", "r");
		
		if ( gta3set )
		{
			ControlsManager.LoadSettings(gta3set);
			CFileMgr::CloseFile(gta3set);
		}
		
		CFileMgr::SetDir("");
	}
	
#ifdef _WIN32
	SetErrorMode(SEM_FAILCRITICALERRORS);
#endif

#ifndef MASTER
	if (TurnOnAnimViewer) {
		CAnimViewer::Initialise();
		FrontEndMenuManager.m_bGameNotLoaded = false;
		gGameState = GS_ANIMVIEWER;
		TurnOnAnimViewer = false;
	}
#endif
	
	initkeymap();

	while ( TRUE )
	{
		RwInitialised = TRUE;
		
		/* 
		* Set the initial mouse position...
		*/
		pos.x = RsGlobal.maximumWidth * 0.5f;
		pos.y = RsGlobal.maximumHeight * 0.5f;

		RsMouseSetPos(&pos);
		
		/*
		* Enter the message processing loop...
		*/

		while( !RsGlobal.quit && !FrontEndMenuManager.m_bWantToRestart && !glfwWindowShouldClose(PSGLOBAL(window)))
		{
			glfwPollEvents();
			if( ForegroundApp )
			{
				switch ( gGameState )
				{
					case GS_START_UP:
					{
						gGameState = GS_INIT_ONCE;
						TRACE("gGameState = GS_INIT_ONCE");
						break;
					}

					case GS_INIT_ONCE:
					{
						//CoUninitialize();
						
						LoadingScreen(nil, nil, "loadsc0");
						
						if ( !CGame::InitialiseOnceAfterRW() )
							RsGlobal.quit = TRUE;
						
						gGameState = GS_INIT_FRONTEND;
						TRACE("gGameState = GS_INIT_FRONTEND;");
						break;
					}
					
					case GS_INIT_FRONTEND:
					{
						LoadingScreen(nil, nil, "loadsc0");
						
						FrontEndMenuManager.m_bGameNotLoaded = true;
						
						FrontEndMenuManager.m_bStartUpFrontEndRequested = true;
						
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
						if(!glfwGetWindowAttrib(PSGLOBAL(window), GLFW_ICONIFIED))
							RsEventHandler(rsFRONTENDIDLE, nil);

						if ( !FrontEndMenuManager.m_bMenuActive || FrontEndMenuManager.m_bWantToLoad )
						{
							gGameState = GS_INIT_PLAYING_GAME;
							TRACE("gGameState = GS_INIT_PLAYING_GAME;");
						}

						if ( FrontEndMenuManager.m_bWantToLoad )
						{
							InitialiseGame();
							FrontEndMenuManager.m_bGameNotLoaded = false;
							gGameState = GS_PLAYING_GAME;
							TRACE("gGameState = GS_PLAYING_GAME;");
						}
						break;
					}
					
					case GS_INIT_PLAYING_GAME:
					{
						InitialiseGame();
						FrontEndMenuManager.m_bGameNotLoaded = false;
						gGameState = GS_PLAYING_GAME;
						TRACE("gGameState = GS_PLAYING_GAME;");
						break;
					}
					
					case GS_PLAYING_GAME:
					{
						float ms = (float)CTimer::GetCurrentTimeInCycles() / (float)CTimer::GetCyclesPerMillisecond();
						if ( RwInitialised )
						{
							if (!FrontEndMenuManager.m_PrefsFrameLimiter || (1000.0f / (float)RsGlobal.maxFPS) < ms)
								RsEventHandler(rsIDLE, (void *)TRUE);
						}
						break;
					}
#ifndef MASTER
					case GS_ANIMVIEWER:
					{
						float ms = (float)CTimer::GetCurrentTimeInCycles() / (float)CTimer::GetCyclesPerMillisecond();
						if (RwInitialised)
						{
							if (!FrontEndMenuManager.m_PrefsFrameLimiter || (1000.0f / (float)RsGlobal.maxFPS) < ms)
								RsEventHandler(rsANIMVIEWER, (void*)TRUE);
						}
						break;
					}
#endif
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
				
			}
		}

		
		/* 
		* About to shut down - block resize events again...
		*/
		RwInitialised = FALSE;
		
		FrontEndMenuManager.UnloadTextures();
		if ( !FrontEndMenuManager.m_bWantToRestart )
			break;
		
		CPad::ResetCheats();
		CPad::StopPadsShaking();
		
		DMAudio.ChangeMusicMode(MUSICMODE_DISABLE);
		
		CTimer::Stop();
		
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
			if ( gGameState == GS_PLAYING_GAME )
				CGame::ShutDown();
			else if ( gGameState == GS_ANIMVIEWER )
				CAnimViewer::Shutdown();
			
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
	}
	

	if ( gGameState == GS_PLAYING_GAME )
		CGame::ShutDown();
	else if ( gGameState == GS_ANIMVIEWER )
		CAnimViewer::Shutdown();

	DMAudio.Terminate();
	
	_psFreeVideoModeList();


	/*
	 * Tidy up the 3D (RenderWare) components of the application...
	 */
	RsEventHandler(rsRWTERMINATE, nil);

	/*
	 * Free the platform dependent data...
	 */
	RsEventHandler(rsTERMINATE, nil);

#ifdef _WIN32
	/* 
	 * Free the argv strings...
	 */
	free(argv);
	
	SystemParametersInfo(SPI_SETSTICKYKEYS, sizeof(STICKYKEYS), &SavedStickyKeys, SPIF_SENDCHANGE);
	SystemParametersInfo(SPI_SETPOWEROFFACTIVE, TRUE, nil, SPIF_SENDCHANGE);
	SystemParametersInfo(SPI_SETLOWPOWERACTIVE, TRUE, nil, SPIF_SENDCHANGE);
	SetErrorMode(0);
#endif

	return 0;
}

/*
 *****************************************************************************
 */

RwV2d leftStickPos;
RwV2d rightStickPos;

void CapturePad(RwInt32 padID)
{
	int8 glfwPad = -1;

	if( padID == 0 )
		glfwPad = PSGLOBAL(joy1id);
	else if( padID == 1)
		glfwPad = PSGLOBAL(joy2id);
	else
		assert("invalid padID");
	
	if ( glfwPad == -1 )
		return;
	
	int numButtons, numAxes;
	const uint8 *buttons = glfwGetJoystickButtons(glfwPad, &numButtons);
	const float *axes = glfwGetJoystickAxes(glfwPad, &numAxes);
	GLFWgamepadstate gamepadState;

	if (ControlsManager.m_bFirstCapture == false)
	{
		memcpy(&ControlsManager.m_OldState, &ControlsManager.m_NewState, sizeof(ControlsManager.m_NewState));
	}

	ControlsManager.m_NewState.buttons = (uint8*)buttons;
	ControlsManager.m_NewState.numButtons = numButtons;
	ControlsManager.m_NewState.id = glfwPad;
	ControlsManager.m_NewState.isGamepad = glfwJoystickIsGamepad(glfwPad);
	if (ControlsManager.m_NewState.isGamepad) {
		glfwGetGamepadState(glfwPad, &gamepadState);
		memcpy(&ControlsManager.m_NewState.mappedButtons, gamepadState.buttons, sizeof(gamepadState.buttons));
		ControlsManager.m_NewState.mappedButtons[15] = gamepadState.axes[4] > -0.8f;
		ControlsManager.m_NewState.mappedButtons[16] = gamepadState.axes[5] > -0.8f;
	}
	// TODO I'm not sure how to find/what to do with L2-R2, if joystick isn't registered in SDL database.

	if (ControlsManager.m_bFirstCapture == true) {
		memcpy(&ControlsManager.m_OldState, &ControlsManager.m_NewState, sizeof(ControlsManager.m_NewState));
		
		ControlsManager.m_bFirstCapture = false;
	}

	RsPadButtonStatus bs;
	bs.padID = padID;

	RsPadEventHandler(rsPADBUTTONUP, (void *)&bs);
	
	// Gamepad axes are guaranteed to return 0.0f if that particular gamepad doesn't have that axis.
	if ( glfwPad != -1 ) {
		leftStickPos.x = ControlsManager.m_NewState.isGamepad ? gamepadState.axes[0] : numAxes >= 0 ? axes[0] : 0.0f;
		leftStickPos.y = ControlsManager.m_NewState.isGamepad ? gamepadState.axes[1] : numAxes >= 1 ? axes[1] : 0.0f;

		rightStickPos.x = ControlsManager.m_NewState.isGamepad ? gamepadState.axes[2] : numAxes >= 2 ? axes[2] : 0.0f;
		rightStickPos.y = ControlsManager.m_NewState.isGamepad ? gamepadState.axes[3] : numAxes >= 3 ? axes[3] : 0.0f;
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
	
	return;
}

void joysChangeCB(int jid, int event)
{
	if (event == GLFW_CONNECTED)
	{
		if (PSGLOBAL(joy1id) == -1)
			PSGLOBAL(joy1id) = jid;
		else if (PSGLOBAL(joy2id) == -1)
			PSGLOBAL(joy2id) = jid;
	}
	else if (event == GLFW_DISCONNECTED)
	{
		if (PSGLOBAL(joy1id) == jid)
			PSGLOBAL(joy1id) = -1;
		else if (PSGLOBAL(joy2id) == jid)
			PSGLOBAL(joy2id) = -1;
	}
}

#if (defined(_MSC_VER))
int strcasecmp(const char* str1, const char* str2)
{
	return _strcmpi(str1, str2);
}
int strncasecmp(const char *str1, const char *str2, size_t len)
{
	return _strnicmp(str1, str2, len);
}
#endif
#endif
