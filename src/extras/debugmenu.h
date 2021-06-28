#pragma once

#ifdef DEBUGMENU

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
		m_pPath(strPath), m_pVarName(strName), m_pFunc(pFunc)
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
		m_pPath(strPath), m_pVarName(strName), m_pBoolVar(pBool)
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
	CTweakSwitch(void *pInt, const char *strName, int32 nMin, int32 nMax, const char **aStr,
	             void (*pFunc)(), const char *strPath)
	    : m_pPath(strPath), m_pVarName(strName), m_pIntVar(pInt), m_nMin(nMin), m_nMax(nMax),
	      m_aStr(aStr)
	{
		CTweakVars::Add(this);
	}

	void AddDBG(const char *path);
};

#define _TWEEKCLASS(name, type)                                                                    \
	class name : public CTweakVar                                                              \
	{                                                                                          \
	public:                                                                                    \
		const char *m_pPath, *m_pVarName;                                                  \
		type *m_pIntVar, m_nLoawerBound, m_nUpperBound, m_nStep;                           \
                                                                                                   \
		name(type *pInt, const char *strName, type nLower, type nUpper, type nStep,        \
		     const char *strPath)                                                          \
		    : m_pPath(strPath), m_pVarName(strName), m_pIntVar(pInt),                      \
		      m_nLoawerBound(nLower), m_nUpperBound(nUpper), m_nStep(nStep)                \
                                                                                                   \
		{                                                                                  \
			CTweakVars::Add(this);                                                     \
		}                                                                                  \
                                                                                                   \
		void AddDBG(const char *path);                                                     \
	};

_TWEEKCLASS(CTweakInt8, int8);
_TWEEKCLASS(CTweakUInt8, uint8);
_TWEEKCLASS(CTweakInt16, int16);
_TWEEKCLASS(CTweakUInt16, uint16);
_TWEEKCLASS(CTweakInt32, int32);
_TWEEKCLASS(CTweakUInt32, uint32);
_TWEEKCLASS(CTweakFloat, float);

#undef _TWEEKCLASS

typedef void (*TriggerFunc)(void);

struct Menu;

struct MenuEntry
{
	int type;
	const char *name;
	MenuEntry *next;
	RwRect r;
	Menu *menu;

	MenuEntry(const char *name);
	virtual ~MenuEntry(void) { free((void*)name); }
};

typedef MenuEntry DebugMenuEntry;

MenuEntry *DebugMenuAddInt8(const char *path, const char *name, int8 *ptr, TriggerFunc triggerFunc, int8 step, int8 lowerBound, int8 upperBound, const char **strings);
MenuEntry *DebugMenuAddInt16(const char *path, const char *name, int16 *ptr, TriggerFunc triggerFunc, int16 step, int16 lowerBound, int16 upperBound, const char **strings);
MenuEntry *DebugMenuAddInt32(const char *path, const char *name, int32 *ptr, TriggerFunc triggerFunc, int32 step, int32 lowerBound, int32 upperBound, const char **strings);
MenuEntry *DebugMenuAddInt64(const char *path, const char *name, int64 *ptr, TriggerFunc triggerFunc, int64 step, int64 lowerBound, int64 upperBound, const char **strings);
MenuEntry *DebugMenuAddUInt8(const char *path, const char *name, uint8 *ptr, TriggerFunc triggerFunc, uint8 step, uint8 lowerBound, uint8 upperBound, const char **strings);
MenuEntry *DebugMenuAddUInt16(const char *path, const char *name, uint16 *ptr, TriggerFunc triggerFunc, uint16 step, uint16 lowerBound, uint16 upperBound, const char **strings);
MenuEntry *DebugMenuAddUInt32(const char *path, const char *name, uint32 *ptr, TriggerFunc triggerFunc, uint32 step, uint32 lowerBound, uint32 upperBound, const char **strings);
MenuEntry *DebugMenuAddUInt64(const char *path, const char *name, uint64 *ptr, TriggerFunc triggerFunc, uint64 step, uint64 lowerBound, uint64 upperBound, const char **strings);
MenuEntry *DebugMenuAddFloat32(const char *path, const char *name, float *ptr, TriggerFunc triggerFunc, float step, float lowerBound, float upperBound);
MenuEntry *DebugMenuAddFloat64(const char *path, const char *name, double *ptr, TriggerFunc triggerFunc, double step, double lowerBound, double upperBound);
MenuEntry *DebugMenuAddCmd(const char *path, const char *name, TriggerFunc triggerFunc);
void DebugMenuEntrySetWrap(MenuEntry *e, bool wrap);
void DebugMenuEntrySetStrings(MenuEntry *e, const char **strings);
void DebugMenuEntrySetAddress(MenuEntry *e, void *addr);
void DebugMenuInit(void);
void DebugMenuShutdown(void);
void DebugMenuProcess(void);
void DebugMenuRender(void);


// Some overloads for simplicity
inline DebugMenuEntry *DebugMenuAddVar(const char *path, const char *name, int8_t *ptr, TriggerFunc triggerFunc, int8_t step, int8_t lowerBound, int8_t upperBound, const char **strings)
{ return DebugMenuAddInt8(path, name, ptr, triggerFunc, step, lowerBound, upperBound, strings); }
inline DebugMenuEntry *DebugMenuAddVar(const char *path, const char *name, int16_t *ptr, TriggerFunc triggerFunc, int16_t step, int16_t lowerBound, int16_t upperBound, const char **strings)
{ return DebugMenuAddInt16(path, name, ptr, triggerFunc, step, lowerBound, upperBound, strings); }
inline DebugMenuEntry *DebugMenuAddVar(const char *path, const char *name, int32_t *ptr, TriggerFunc triggerFunc, int32_t step, int32_t lowerBound, int32_t upperBound, const char **strings)
{ return DebugMenuAddInt32(path, name, ptr, triggerFunc, step, lowerBound, upperBound, strings); }
inline DebugMenuEntry *DebugMenuAddVar(const char *path, const char *name, int64_t *ptr, TriggerFunc triggerFunc, int64_t step, int64_t lowerBound, int64_t upperBound, const char **strings)
{ return DebugMenuAddInt64(path, name, ptr, triggerFunc, step, lowerBound, upperBound, strings); }
inline DebugMenuEntry *DebugMenuAddVar(const char *path, const char *name, uint8_t *ptr, TriggerFunc triggerFunc, uint8_t step, uint8_t lowerBound, uint8_t upperBound, const char **strings)
{ return DebugMenuAddUInt8(path, name, ptr, triggerFunc, step, lowerBound, upperBound, strings); }
inline DebugMenuEntry *DebugMenuAddVar(const char *path, const char *name, uint16_t *ptr, TriggerFunc triggerFunc, uint16_t step, uint16_t lowerBound, uint16_t upperBound, const char **strings)
{ return DebugMenuAddUInt16(path, name, ptr, triggerFunc, step, lowerBound, upperBound, strings); }
inline DebugMenuEntry *DebugMenuAddVar(const char *path, const char *name, uint32_t *ptr, TriggerFunc triggerFunc, uint32_t step, uint32_t lowerBound, uint32_t upperBound, const char **strings)
{ return DebugMenuAddUInt32(path, name, ptr, triggerFunc, step, lowerBound, upperBound, strings); }
inline DebugMenuEntry *DebugMenuAddVar(const char *path, const char *name, uint64_t *ptr, TriggerFunc triggerFunc, uint64_t step, uint64_t lowerBound, uint64_t upperBound, const char **strings)
{ return DebugMenuAddUInt64(path, name, ptr, triggerFunc, step, lowerBound, upperBound, strings); }
inline DebugMenuEntry *DebugMenuAddVar(const char *path, const char *name, float *ptr, TriggerFunc triggerFunc, float step, float lowerBound, float upperBound)
{ return DebugMenuAddFloat32(path, name, ptr, triggerFunc, step, lowerBound, upperBound); }
inline DebugMenuEntry *DebugMenuAddVar(const char *path, const char *name, double *ptr, TriggerFunc triggerFunc, double step, double lowerBound, double upperBound)
{ return DebugMenuAddFloat64(path, name, ptr, triggerFunc, step, lowerBound, upperBound); }

inline DebugMenuEntry *DebugMenuAddVarBool32(const char *path, const char *name, int32_t *ptr, TriggerFunc triggerFunc)
{
	static const char *boolstr[] = { "Off", "On" };
	DebugMenuEntry *e = DebugMenuAddVar(path, name, ptr, triggerFunc, 1, 0, 1, boolstr);
	DebugMenuEntrySetWrap(e, true);
	return e;
}
inline DebugMenuEntry *DebugMenuAddVarBool16(const char *path, const char *name, int16_t *ptr, TriggerFunc triggerFunc)
{
	static const char *boolstr[] = { "Off", "On" };
	DebugMenuEntry *e = DebugMenuAddVar(path, name, ptr, triggerFunc, 1, 0, 1, boolstr);
	DebugMenuEntrySetWrap(e, true);
	return e;
}
inline DebugMenuEntry *DebugMenuAddVarBool8(const char *path, const char *name, int8_t *ptr, TriggerFunc triggerFunc)
{
	static const char *boolstr[] = { "Off", "On" };
	DebugMenuEntry *e = DebugMenuAddVar(path, name, ptr, triggerFunc, 1, 0, 1, boolstr);
	DebugMenuEntrySetWrap(e, true);
	return e;
}
inline DebugMenuEntry *DebugMenuAddVarBool8(const char *path, const char *name, bool *ptr, TriggerFunc triggerFunc)
{
	return DebugMenuAddVarBool8(path, name, (int8_t*)ptr, triggerFunc);
}
#endif