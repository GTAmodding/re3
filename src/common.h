#pragma once

#define _CRT_SECURE_NO_WARNINGS
#define _USE_MATH_DEFINES
#pragma warning(disable: 4244)	// int to float
#pragma warning(disable: 4800)	// int to bool
#pragma warning(disable: 4305)	// double to float
#pragma warning(disable: 4838)  // narrowing conversion
#pragma warning(disable: 4996)  // POSIX names

#include <stdint.h>
#include <math.h>
//#include <assert.h>
#include <new>

#ifdef WITHD3D
#include <Windows.h>
#include <d3d8types.h>
#endif

#include <rwcore.h>
#include <rpworld.h>

#define rwVENDORID_ROCKSTAR 0x0253F2

// Get rid of bullshit windows definitions, we're not running on an 8086
#ifdef far
#undef far
#endif
#ifdef near
#undef near
#endif

typedef uint8_t uint8, UInt8;
typedef int8_t int8, Int8;
typedef uint16_t uint16, UInt16;
typedef int16_t int16, Int16;
typedef uint32_t uint32, UInt32;
typedef int32_t int32, Int32;
typedef uintptr_t uintptr;
typedef uint64_t uint64, UInt64;
typedef int64_t int64, Int64;
// hardcode ucs-2
typedef uint16_t wchar, WChar;

typedef float Float;
typedef double Double;
typedef bool Bool;
typedef char Char;

#define nil NULL

#include "config.h"

#define ALIGNPTR(p) (void*)((((uintptr)(void*)p) + sizeof(void*)-1) & ~(sizeof(void*)-1))

// little hack
extern void **rwengine;
#define RwEngineInstance (*rwengine)

#include "skel\skeleton.h"
#include "Draw.h"

#define SCREEN_ASPECT_RATIO (CDraw::GetAspectRatio())

#define SCREENW (RsGlobal.maximumWidth)
#define SCREENH (RsGlobal.maximumHeight)

#define DEFAULT_SCREEN_WIDTH (640)
#define DEFAULT_SCREEN_HEIGHT (448)
#define SCREEN_WIDTH Float(RsGlobal.width)
#define SCREEN_HEIGHT Float(RsGlobal.height)
#define SCREEN_STRETCH_X(a)   Float((a) * (SCREEN_WIDTH / Float(DEFAULT_SCREEN_WIDTH)))
#define SCREEN_STRETCH_X_AR(a)   Float((a) * (SCREEN_WIDTH / Float(DEFAULT_SCREEN_WIDTH)) * (4.0/3.0f)/SCREEN_ASPECT_RATIO)
#define SCREEN_STRETCH_Y(a)   Float((a) * (SCREEN_HEIGHT / Float(DEFAULT_SCREEN_HEIGHT)))
#define SCREEN_FROM_RIGHT(a)  Float(SCREEN_WIDTH - SCREEN_STRETCH_X(a))
#define SCREEN_FROM_BOTTOM(a) Float(SCREEN_HEIGHT - SCREEN_STRETCH_Y(a))

#define HUD_STRETCH_X SCREEN_STRETCH_X_AR
#define HUD_STRETCH_Y SCREEN_STRETCH_Y

struct GlobalScene
{
	RpWorld *world;
	RwCamera *camera;
};
extern GlobalScene &Scene;

#include "math/Vector.h"
#include "math/Vector2D.h"
#include "math/Matrix.h"
#include "math/Rect.h"

class CRGBA
{
public:
	union
	{
	    uint32 color32;
		struct { uint8 r, g, b, a; };
		struct { uint8 red, green, blue, alpha; };
#ifdef RWCORE_H
		struct  { RwRGBA rwRGBA; };
#endif
	};
	
	CRGBA(void) { }
	CRGBA(uint8 r, uint8 g, uint8 b, uint8 a) : r(r), g(g), b(b), a(a) { }
#ifdef RWCORE_H
	operator RwRGBA &(void) {
		return rwRGBA;
	}
	
	operator RwRGBA *(void) {
		return &rwRGBA;
	}
	
	operator RwRGBA (void) const {
		return rwRGBA;
	}
#endif
};

#define clamp(v, low, high) ((v)<(low) ? (low) : (v)>(high) ? (high) : (v))

inline float sq(float x) { return x*x; }
#define SQR(x) ((x) * (x))

#define PI M_PI
#define DEGTORAD(x) ((x) * PI / 180.0f)
#define RADTODEG(x) ((x) * 180.0f / PI)

#ifdef USE_PS2_RAND
#define MYRAND_MAX		65535
#else
#define MYRAND_MAX		32767
#endif

int myrand(void);
void mysrand(unsigned int seed);

extern uint8 work_buff[55000];
extern char gString[256];

void re3_debug(char *format, ...);
void re3_trace(const char *filename, unsigned int lineno, const char *func, char *format, ...);
void re3_assert(const char *expr, const char *filename, unsigned int lineno, const char *func);

#define DEBUGBREAK() __debugbreak();

#define debug(f, ...) re3_debug("[DBG]: " f, __VA_ARGS__)
#define DEV(f, ...)   re3_debug("[DEV]: " f, __VA_ARGS__)
#define TRACE(f, ...) re3_trace(__FILE__, __LINE__, __FUNCTION__, f, __VA_ARGS__)

#define assert(_Expression) (void)( (!!(_Expression)) || (re3_assert(#_Expression, __FILE__, __LINE__, __FUNCTION__), 0) )
#define ASSERT assert

#define _TODO(x)      
#define _TODOCONST(x) (x)

#define VALIDATE_SIZE(struc, size) static_assert(sizeof(struc) == size, "Invalid structure size of " #struc)
#define VALIDATE_OFFSET(struc, member, offset) static_assert(offsetof(struc, member) == offset, "The offset of " #member " in " #struc " is not " #offset "...")

#define PERCENT(x, p)                    ((Float(x) * (Float(p) / 100.0f)))
#define ARRAY_SIZE(array)                (sizeof(array) / sizeof(array[0]))
#define BIT(num)                         (1<<(num))
