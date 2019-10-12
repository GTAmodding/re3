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
#include <windows.h>
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

typedef uint8_t uint8;
typedef int8_t int8;
typedef uint16_t uint16;
typedef int16_t int16;
typedef uint32_t uint32;
typedef int32_t int32;
typedef uintptr_t uintptr;
typedef uint64_t uint64;
typedef int64_t int64;
// hardcode ucs-2
typedef uint16_t wchar;

#define nil nullptr

#include "config.h"

#define ALIGNPTR(p) (void*)((((uintptr)(void*)p) + sizeof(void*)-1) & ~(sizeof(void*)-1))

// PDP-10 like byte functions
#define MASK(p, s) (((1<<(s))-1) << (p))
inline uint32 dpb(uint32 b, uint32 p, uint32 s, uint32 w)
{
	uint32 m = MASK(p,s);
	return w & ~m | b<<p & m;
}
inline uint32 ldb(uint32 p, uint32 s, uint32 w)
{
	return w>>p & (1<<s)-1;
}


// little hack
extern void **rwengine;
#define RwEngineInstance (*rwengine)

#include "skeleton.h"
#include "Draw.h"

#define DEFAULT_SCREEN_WIDTH (640)
#define DEFAULT_SCREEN_HEIGHT (448)
#define DEFAULT_ASPECT_RATIO (4.0f/3.0f)

// game uses maximumWidth/Height, but this probably won't work
// with RW windowed mode
#define SCREEN_WIDTH ((float)RsGlobal.width)
#define SCREEN_HEIGHT ((float)RsGlobal.height)
#define SCREEN_ASPECT_RATIO (CDraw::GetAspectRatio())

// This scales from PS2 pixel coordinates to the real resolution
#define SCREEN_STRETCH_X(a)   ((a) * (float) SCREEN_WIDTH / DEFAULT_SCREEN_WIDTH)
#define SCREEN_STRETCH_Y(a)   ((a) * (float) SCREEN_HEIGHT / DEFAULT_SCREEN_HEIGHT)
#define SCREEN_STRETCH_FROM_RIGHT(a)  (SCREEN_WIDTH - SCREEN_STRETCH_X(a))
#define SCREEN_STRETCH_FROM_BOTTOM(a) (SCREEN_HEIGHT - SCREEN_STRETCH_Y(a))

// This scales from PS2 pixel coordinates while optionally maintaining the aspect ratio
#define SCREEN_SCALE_X(a) SCREEN_SCALE_AR(SCREEN_STRETCH_X(a))
#define SCREEN_SCALE_Y(a) SCREEN_STRETCH_Y(a)
#define SCREEN_SCALE_FROM_RIGHT(a) (SCREEN_WIDTH - SCREEN_SCALE_X(a))
#define SCREEN_SCALE_FROM_BOTTOM(a) (SCREEN_HEIGHT - SCREEN_SCALE_Y(a))

#ifdef ASPECT_RATIO_SCALE
#define SCREEN_SCALE_AR(a) ((a) * DEFAULT_ASPECT_RATIO / SCREEN_ASPECT_RATIO)
#define SCREEN_SCALE_AR2(a) ((a) / (DEFAULT_ASPECT_RATIO / SCREEN_ASPECT_RATIO))
#else
#define SCREEN_SCALE_AR(a) (a)
#define SCREEN_SCALE_AR2(a) (a)
#endif

#include "maths.h"
#include "Vector.h"
#include "Vector2D.h"
#include "Matrix.h"
#include "Rect.h"

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
	
	CRGBA &operator =(const CRGBA &right)
	{
		this->r = right.r;
		this->g = right.g;
		this->b = right.b;
		this->a = right.a;
		return *this;
	}
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

	CRGBA &operator =(const RwRGBA &right)
	{
		this->r = right.red;
		this->g = right.green;
		this->b = right.blue;
		this->a = right.alpha;
		return *this;
	}
#endif
};

#define clamp(v, low, high) ((v)<(low) ? (low) : (v)>(high) ? (high) : (v))

inline float sq(float x) { return x*x; }
#define SQR(x) ((x) * (x))

#define PI (float)M_PI
#define TWOPI (PI*2)
#define HALFPI (PI/2)
#define DEGTORAD(x) ((x) * PI / 180.0f)
#define RADTODEG(x) ((x) * 180.0f / PI)

#ifdef USE_PS2_RAND
#define MYRAND_MAX		65535
#else
#define MYRAND_MAX		32767
#endif

int myrand(void);
void mysrand(unsigned int seed);

void re3_debug(const char *format, ...);
void re3_trace(const char *filename, unsigned int lineno, const char *func, const char *format, ...);
void re3_assert(const char *expr, const char *filename, unsigned int lineno, const char *func);

#define DEBUGBREAK() __debugbreak();

#define debug(f, ...) re3_debug("[DBG]: " f, ## __VA_ARGS__)
#define DEV(f, ...)   re3_debug("[DEV]: " f, ## __VA_ARGS__)
#define TRACE(f, ...) re3_trace(__FILE__, __LINE__, __FUNCTION__, f, ## __VA_ARGS__)
#define Error(f, ...) re3_debug("[ERROR]: " f, ## __VA_ARGS__)

#define assert(_Expression) (void)( (!!(_Expression)) || (re3_assert(#_Expression, __FILE__, __LINE__, __FUNCTION__), 0) )
#define ASSERT assert

#define _TODO(x)
#define _TODOCONST(x) (x)

#define VALIDATE_SIZE(struc, size) static_assert(sizeof(struc) == size, "Invalid structure size of " #struc)
#define VALIDATE_OFFSET(struc, member, offset) static_assert(offsetof(struc, member) == offset, "The offset of " #member " in " #struc " is not " #offset "...")

#define PERCENT(x, p)                    ((float(x) * (float(p) / 100.0f)))
#define ARRAY_SIZE(array)                (sizeof(array) / sizeof(array[0]))
#define BIT(num)                         (1<<(num))

#define ABS(a)  (((a) < 0) ? (-(a)) : (a))
#define norm(value, min, max) (((value) < (min)) ? 0 : (((value) > (max)) ? 1 : (((value) - (min)) / ((max) - (min)))))

#define STRINGIFY(x)                    #x
#define STR(x)                          STRINGIFY(x)
#define CONCAT_(x,y) x##y
#define CONCAT(x,y) CONCAT_(x,y)

// Tweaking stuff for debugmenu
#define TWEAKPATH                                   ___tw___TWEAKPATH
#define SETTWEAKPATH(path)	                        static const char *___tw___TWEAKPATH = path;
#define TWEAKFUNC(v)                                static CTweakFunc   CONCAT(___tw___tweak, __COUNTER__)(&v, STR(v), TWEAKPATH);
#define TWEAKFUNCN(v, name)                         static CTweakFunc   CONCAT(___tw___tweak, __COUNTER__)(&v, name, TWEAKPATH);
#define TWEAKBOOL(v)                                static CTweakBool   CONCAT(___tw___tweak, __COUNTER__)(&v, STR(v), TWEAKPATH);
#define TWEAKBOOLN(v, name)                         static CTweakBool   CONCAT(___tw___tweak, __COUNTER__)(&v, name, TWEAKPATH);
#define TWEAKINT32(v, lower, upper, step)           static CTweakInt32  CONCAT(___tw___tweak, __COUNTER__)(&v, STR(v), lower, upper, step, TWEAKPATH);
#define TWEAKINT32N(v, lower, upper, step, name)    static CTweakInt32  CONCAT(___tw___tweak, __COUNTER__)(&v, name, lower, upper, step, TWEAKPATH);
#define TWEAKUINT32(v, lower, upper, step)          static CTweakUInt32 CONCAT(___tw___tweak, __COUNTER__)(&v, STR(v), lower, upper, step, TWEAKPATH);
#define TWEAKUINT32N(v, lower, upper, step, name)   static CTweakUInt32 CONCAT(___tw___tweak, __COUNTER__)(&v, name, lower, upper, step, TWEAKPATH);
#define TWEAKINT16(v, lower, upper, step)           static CTweakInt16  CONCAT(___tw___tweak, __COUNTER__)(&v, STR(v), lower, upper, step, TWEAKPATH);
#define TWEAKINT16N(v, lower, upper, step, name)    static CTweakInt16  CONCAT(___tw___tweak, __COUNTER__)(&v, name, lower, upper, step, TWEAKPATH);
#define TWEAKUINT16(v, lower, upper, step)          static CTweakUInt16 CONCAT(___tw___tweak, __COUNTER__)(&v, STR(v), lower, upper, step, TWEAKPATH);
#define TWEAKUINT16N(v, lower, upper, step, name)   static CTweakUInt16 CONCAT(___tw___tweak, __COUNTER__)(&v, name, lower, upper, step, TWEAKPATH);
#define TWEAKINT8(v, lower, upper, step)            static CTweakInt8   CONCAT(___tw___tweak, __COUNTER__)(&v, STR(v), lower, upper, step, TWEAKPATH);
#define TWEAKINT8N(v, lower, upper, step, name)     static CTweakInt8   CONCAT(___tw___tweak, __COUNTER__)(&v, name, lower, upper, step, TWEAKPATH);
#define TWEAKUINT8(v, lower, upper, step)           static CTweakUInt8  CONCAT(___tw___tweak, __COUNTER__)(&v, STR(v), lower, upper, step, TWEAKPATH);
#define TWEAKUINT8N(v, lower, upper, step, name)    static CTweakUInt8  CONCAT(___tw___tweak, __COUNTER__)(&v, name, lower, upper, step, TWEAKPATH);
#define TWEAKFLOAT(v, lower, upper, step)           static CTweakFloat  CONCAT(___tw___tweak, __COUNTER__)(&v, STR(v), lower, upper, step, TWEAKPATH);
#define TWEAKFLOATN(v, lower, upper, step, name)    static CTweakFloat  CONCAT(___tw___tweak, __COUNTER__)(&v, name, lower, upper, step, TWEAKPATH);
#define TWEAKSWITCH(v, lower, upper, str, f)        static CTweakSwitch CONCAT(___tw___tweak, __COUNTER__)(&v, STR(v), lower, upper, str, f, TWEAKPATH);
#define TWEAKSWITCHN(v, lower, upper, str, f, name) static CTweakSwitch CONCAT(___tw___tweak, __COUNTER__)(&v, name, lower, upper, str, f, TWEAKPATH);

// interface
class CTweakVar
{
public:
	virtual void AddDBG(const char *path) = 0;
};

class CTweakVars
{
public:
	static void Add(CTweakVar *var);
	static void AddDBG(const char *path);
};

class CTweakFunc : public CTweakVar
{
	const char *m_pPath, *m_pVarName;
	void (*m_pFunc)();
public:
	CTweakFunc(void (*pFunc)(), const char *strName, const char *strPath) :
		m_pFunc(pFunc), m_pVarName(strName), m_pPath(strPath)
	{
		CTweakVars::Add(this);
	}
	
	void AddDBG(const char *path);
};

class CTweakBool : public CTweakVar
{
	const char *m_pPath, *m_pVarName;
	bool *m_pBoolVar;
public:
	CTweakBool(bool *pBool, const char *strName, const char *strPath) :
		m_pBoolVar(pBool), m_pVarName(strName), m_pPath(strPath)
	{
		CTweakVars::Add(this);
	}
	
	void AddDBG(const char *path);
};

class CTweakSwitch : public CTweakVar
{
	const char *m_pPath, *m_pVarName;
	void *m_pIntVar;
	int32 m_nMin, m_nMax;
	const char **m_aStr;
	void (*m_pFunc)();
public:
	CTweakSwitch(void *pInt, const char *strName, int32 nMin, int32 nMax, const char **aStr, void (*pFunc)(), const char *strPath) :
		m_pVarName(strName), m_pPath(strPath),
		m_aStr(aStr), m_pIntVar(pInt), m_nMin(nMin), m_nMax(nMax)
	{
		CTweakVars::Add(this);
	}

	void AddDBG(const char *path);
};

#define _TWEEKCLASS(name, type) \
class name : public CTweakVar \
{ \
public: \
	const char *m_pPath, *m_pVarName; \
	type *m_pIntVar, m_nLoawerBound, m_nUpperBound, m_nStep; \
 \
	name(type *pInt, const char *strName, type nLower, type nUpper, type nStep, const char *strPath) : \
		m_pIntVar(pInt), m_nLoawerBound(nLower), m_nUpperBound(nUpper), m_nStep(nStep), \
		m_pVarName(strName), m_pPath(strPath) \
	{ \
		CTweakVars::Add(this); \
	} \
	 \
	void AddDBG(const char *path); \
};

_TWEEKCLASS(CTweakInt8, int8);
_TWEEKCLASS(CTweakUInt8, uint8);
_TWEEKCLASS(CTweakInt16, int16);
_TWEEKCLASS(CTweakUInt16, uint16);
_TWEEKCLASS(CTweakInt32, int32);
_TWEEKCLASS(CTweakUInt32, uint32);
_TWEEKCLASS(CTweakFloat, float);

#undef _TWEEKCLASS

#ifdef VALIDATE_SAVE_SIZE
extern int32 _bufBytesRead;
#define INITSAVEBUF _bufBytesRead = 0;
#define VALIDATESAVEBUF(b) assert(_bufBytesRead == b);
#else
#define INITSAVEBUF
#define VALIDATESAVEBUF(b)
#endif

inline void SkipSaveBuf(uint8 *&buf, int32 skip)
{
	buf += skip;
#ifdef VALIDATE_SAVE_SIZE
	_bufBytesRead += skip;
#endif
}

template<typename T>
inline const T ReadSaveBuf(uint8 *&buf)
{
	T &value = *(T*)buf;
	SkipSaveBuf(buf, sizeof(T));
	return value;
}

template<typename T>
inline T *WriteSaveBuf(uint8 *&buf, const T &value)
{
	T *p = (T*)buf;
	*p = value;
	SkipSaveBuf(buf, sizeof(T));
	return p;
}


#define SAVE_HEADER_SIZE (4*sizeof(char)+sizeof(uint32))

#define WriteSaveHeader(buf,a,b,c,d,size) \
	WriteSaveBuf(buf, a);\
	WriteSaveBuf(buf, b);\
	WriteSaveBuf(buf, c);\
	WriteSaveBuf(buf, d);\
	WriteSaveBuf(buf, size);

#define CheckSaveHeader(buf,a,b,c,d,size)\
	assert(ReadSaveBuf<char>(buf) == a);\
	assert(ReadSaveBuf<char>(buf) == b);\
	assert(ReadSaveBuf<char>(buf) == c);\
	assert(ReadSaveBuf<char>(buf) == d);\
	assert(ReadSaveBuf<uint32>(buf) == size);