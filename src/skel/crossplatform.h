#include <time.h>
#include <limits.h>

// This is the common include for platform/renderer specific skeletons(glfw.cpp, win.cpp etc.) and using cross platform things (like Windows directories wrapper, platform specific global arrays etc.) 
// Functions that's different on glfw and win but have same signature, should be located on platform.h.

enum eWinVersion
{
    OS_WIN95 = 0,
    OS_WIN98,
    OS_WINNT,
    OS_WIN2000,
    OS_WINXP,
};

#ifdef _WIN32

// As long as WITHWINDOWS isn't defined / <Windows.h> isn't included, we only need type definitions so let's include <IntSafe.h>.
// NOTE: It's perfectly fine to include <Windows.h> here, but it can increase build size and time in *some* conditions, and maybe substantially in future if we'll use crossplatform.h more.
#ifndef _INC_WINDOWS
	#ifndef __MWERKS__
		#include <IntSafe.h>
	#else
		#include <Windows.h>
	#endif
#endif
#if defined RW_D3D9 || defined RWLIBS
#include "win.h"
#endif
extern DWORD _dwOperatingSystemVersion;
#define fcaseopen fopen
#define caserename rename
#else
char *strupr(char *str);
char *strlwr(char *str);

enum {
	LANG_OTHER,
	LANG_GERMAN,
	LANG_FRENCH,
	LANG_ENGLISH,
	LANG_ITALIAN,
	LANG_SPANISH,
};

enum {
	SUBLANG_OTHER,
	SUBLANG_ENGLISH_AUS
};

extern long _dwOperatingSystemVersion;
char *casepath(char const *path, bool checkPathFirst = true);
FILE *_fcaseopen(char const *filename, char const *mode);
#define fcaseopen _fcaseopen
int _caserename(const char *old_filename, const char *new_filename);
#define caserename _caserename
#endif

#ifdef RW_GL3
typedef struct
{
    GLFWwindow* window;
    RwBool		fullScreen;
    RwV2d		lastMousePos;
    double      mouseWheel; // glfw doesn't cache it
    bool        cursorIsInWindow;
    RwInt8        joy1id;
    RwInt8        joy2id;
}
psGlobalType;

#define PSGLOBAL(var) (((psGlobalType *)(RsGlobal.ps))->var)

void CapturePad(RwInt32 padID);
void joysChangeCB(int jid, int event);
#endif

#ifdef DETECT_JOYSTICK_MENU
extern char gSelectedJoystickName[128];
#endif

enum eGameState
{
    GS_START_UP = 0,
    GS_INIT_LOGO_MPEG,
    GS_LOGO_MPEG,
    GS_INIT_INTRO_MPEG,
    GS_INTRO_MPEG,
    GS_INIT_ONCE,
    GS_INIT_FRONTEND,
    GS_FRONTEND,
    GS_INIT_PLAYING_GAME,
    GS_PLAYING_GAME,
};
extern RwUInt32 gGameState;

RwBool IsForegroundApp();

#ifndef MAX_PATH
    #if !defined _WIN32 || defined __MINGW32__
    #define MAX_PATH PATH_MAX
    #else
    #define MAX_PATH 260
    #endif
#endif

// Codes compatible with Windows and Linux
#ifndef _WIN32
#define DeleteFile unlink

// Needed for save games
struct SYSTEMTIME {
    RwUInt16 wYear;
    RwUInt16 wMonth;
    RwUInt16 wDayOfWeek;
    RwUInt16 wDay;
    RwUInt16 wHour;
    RwUInt16 wMinute;
    RwUInt16 wSecond;
    RwUInt16 wMilliseconds;
};

void GetLocalTime_CP(SYSTEMTIME* out);
#define GetLocalTime GetLocalTime_CP
#define OutputDebugString(s) re3_debug("[DBG-2]: %s\n",s)
#endif

// Compatible with Linux/POSIX and MinGW on Windows
#ifndef _WIN32
#include <iostream>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <langinfo.h>
#include <unistd.h>

typedef void* HANDLE;
#define INVALID_HANDLE_VALUE NULL
#define FindClose(h) \
    do { \
        if (h != nil) \
            closedir((DIR*)h); \
    } while(0)

#define LOCALE_USER_DEFAULT 0
#define DATE_SHORTDATE 0

struct WIN32_FIND_DATA {
    char extension[32]; // for searching
    char folder[MAX_PATH];	// for searching
    char cFileName[256]; // because tSkinInfo has it 256
    time_t ftLastWriteTime;
};

HANDLE FindFirstFile(const char*, WIN32_FIND_DATA*);
bool FindNextFile(HANDLE, WIN32_FIND_DATA*);
void FileTimeToSystemTime(time_t*, SYSTEMTIME*);
void GetDateFormat(int, int, SYSTEMTIME*, int, char*, int);
#endif

#ifdef __SWITCH__

// tweak glfw values for switch to match expected pc bindings
#ifdef GLFW_GAMEPAD_BUTTON_A
    #undef GLFW_GAMEPAD_BUTTON_A
#endif
#define GLFW_GAMEPAD_BUTTON_A 1

#ifdef GLFW_GAMEPAD_BUTTON_B
    #undef GLFW_GAMEPAD_BUTTON_B
#endif
#define GLFW_GAMEPAD_BUTTON_B 0

#ifdef GLFW_GAMEPAD_BUTTON_X
    #undef GLFW_GAMEPAD_BUTTON_X
#endif
#define GLFW_GAMEPAD_BUTTON_X 3

#ifdef GLFW_GAMEPAD_BUTTON_Y
    #undef GLFW_GAMEPAD_BUTTON_Y
#endif
#define GLFW_GAMEPAD_BUTTON_Y 2

#endif
