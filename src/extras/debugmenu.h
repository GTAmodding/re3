#pragma once

#ifdef DEBUGMENU

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