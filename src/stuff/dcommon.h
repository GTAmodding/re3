#include <windows.h>
#include <stdint.h>
#include <assert.h>
typedef uintptr_t uintptr;
#include "rwcore.h"
#include "rwplcore.h"
#include "skeleton.h"

#define EXPORT extern "C" __declspec(dllexport)

typedef uint8_t uint8, uchar;
typedef uint16_t uint16, ushort;
typedef uint32_t uint32, uint;
typedef uint64_t uint64;
typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef uint8_t bool8;

#define RAD2DEG(x) (float)(180.0f * (x) / M_PI)
#define DEG2RAD(x) (float)((x) / 180.0f * M_PI)

#define nil NULL

RwImage *readTGA(uint8 *p);
RwImage *readTGA(int res);

struct Pt
{
	int x, y;
};

enum dFontStyle
{
	dFONT_NORMAL,
	dFONT_SEL_ACTIVE,
	dFONT_SEL_INACTIVE,
	dFONT_MOUSE
};


Pt fontPrint(const char *s, float x, float y, int style);
Pt fontGetStringSize(const char *s);
int fontGetLen(const char *s);

#include "dmenu.h"

#define JUSTDOWN(b) (pad->NewState.b && !pad->OldState.b)
#define KEYJUSTDOWN(k) ControlsManager.GetIsKeyboardKeyJustDown((RsKeyCodes)k)
#define KEYDOWN(k) ControlsManager.GetIsKeyboardKeyDown((RsKeyCodes)k)
#define CTRLJUSTDOWN(key)                                                      \
	((KEYDOWN(rsLCTRL) || KEYDOWN(rsRCTRL)) && KEYJUSTDOWN((RsKeyCodes)key) || \
	 (KEYJUSTDOWN(rsLCTRL) || KEYJUSTDOWN(rsRCTRL)) && KEYDOWN((RsKeyCodes)key))
#define CTRLDOWN(key) ((KEYDOWN(rsLCTRL) || KEYDOWN(rsRCTRL)) && KEYDOWN((RsKeyCodes)key))
