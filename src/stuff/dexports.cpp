#include "dcommon.h"
#include "dexports.h"

typedef void (*TriggerFunc)(void);
DebugMenuEntry *DebugMenuAddCmd(const char *path, const char *name, TriggerFunc triggerFunc)
{
	Menu *m = findMenu(path);
	if (m == nil)
		return nil;
	DebugMenuEntry *e = new DebugMenuEntry_Cmd(name, triggerFunc);
	m->appendEntry(e);
	return e;
}

void DebugMenuEntrySetWrap(DebugMenuEntry *e, bool wrap)
{
	if (e && e->type == MENUVAR)
		((DebugMenuEntry_Var *)e)->wrapAround = wrap;
}

void DebugMenuEntrySetStrings(DebugMenuEntry *e, const char **strings)
{
	if (e && e->type == MENUVAR_INT)
		((DebugMenuEntry_Int *)e)->setStrings(strings);
}
void DebugMenuEntrySetAddress(DebugMenuEntry *e, void *addr)
{
	if (e && e->type == MENUVAR)
	{
		DebugMenuEntry_Var *ev = (DebugMenuEntry_Var *)e;
		// HACK - we know the variable field is at the same address
		// for all int/float classes. let's hope it stays that way.
		if (ev->vartype = MENUVAR_INT)
			((DebugMenuEntry_Int32 *)e)->variable = (int32 *)addr;
		else if (ev->vartype = MENUVAR_FLOAT)
			((DebugMenuEntry_Float32 *)e)->variable = (float *)addr;
	}
}

DebugMenuEntry *DebugMenuAddInt8(const char *path, const char *name, int8_t *ptr, TriggerFunc triggerFunc, int8_t step, int8_t lowerBound, int8_t upperBound, const char **strings)
{
	Menu *m = findMenu(path);
	if (m == nil)
		return nil;
	DebugMenuEntry *e = new DebugMenuEntry_Int8(name, ptr, triggerFunc, step, lowerBound, upperBound, strings);
	m->appendEntry(e);
	return e;
}
DebugMenuEntry *DebugMenuAddInt16(const char *path, const char *name, int16_t *ptr, TriggerFunc triggerFunc, int16_t step, int16_t lowerBound, int16_t upperBound, const char **strings)
{
	Menu *m = findMenu(path);
	if (m == nil)
		return nil;
	DebugMenuEntry *e = new DebugMenuEntry_Int16(name, ptr, triggerFunc, step, lowerBound, upperBound, strings);
	m->appendEntry(e);
	return e;
}
DebugMenuEntry *DebugMenuAddInt32(const char *path, const char *name, int32_t *ptr, TriggerFunc triggerFunc, int32_t step, int32_t lowerBound, int32_t upperBound, const char **strings)
{
	Menu *m = findMenu(path);
	if (m == nil)
		return nil;
	DebugMenuEntry *e = new DebugMenuEntry_Int32(name, ptr, triggerFunc, step, lowerBound, upperBound, strings);
	m->appendEntry(e);
	return e;
}
DebugMenuEntry *DebugMenuAddInt64(const char *path, const char *name, int64_t *ptr, TriggerFunc triggerFunc, int64_t step, int64_t lowerBound, int64_t upperBound, const char **strings)
{
	Menu *m = findMenu(path);
	if (m == nil)
		return nil;
	DebugMenuEntry *e = new DebugMenuEntry_Int64(name, ptr, triggerFunc, step, lowerBound, upperBound, strings);
	m->appendEntry(e);
	return e;
}
DebugMenuEntry *DebugMenuAddUInt8(const char *path, const char *name, uint8_t *ptr, TriggerFunc triggerFunc, uint8_t step, uint8_t lowerBound, uint8_t upperBound, const char **strings)
{
	Menu *m = findMenu(path);
	if (m == nil)
		return nil;
	DebugMenuEntry *e = new DebugMenuEntry_UInt8(name, ptr, triggerFunc, step, lowerBound, upperBound, strings);
	m->appendEntry(e);
	return e;
}
DebugMenuEntry *DebugMenuAddUInt16(const char *path, const char *name, uint16_t *ptr, TriggerFunc triggerFunc, uint16_t step, uint16_t lowerBound, uint16_t upperBound, const char **strings)
{
	Menu *m = findMenu(path);
	if (m == nil)
		return nil;
	DebugMenuEntry *e = new DebugMenuEntry_UInt16(name, ptr, triggerFunc, step, lowerBound, upperBound, strings);
	m->appendEntry(e);
	return e;
}
DebugMenuEntry *DebugMenuAddUInt32(const char *path, const char *name, uint32_t *ptr, TriggerFunc triggerFunc, uint32_t step, uint32_t lowerBound, uint32_t upperBound, const char **strings)
{
	Menu *m = findMenu(path);
	if (m == nil)
		return nil;
	DebugMenuEntry *e = new DebugMenuEntry_UInt32(name, ptr, triggerFunc, step, lowerBound, upperBound, strings);
	m->appendEntry(e);
	return e;
}
DebugMenuEntry *DebugMenuAddUInt64(const char *path, const char *name, uint64_t *ptr, TriggerFunc triggerFunc, uint64_t step, uint64_t lowerBound, uint64_t upperBound, const char **strings)
{
	Menu *m = findMenu(path);
	if (m == nil)
		return nil;
	DebugMenuEntry *e = new DebugMenuEntry_UInt64(name, ptr, triggerFunc, step, lowerBound, upperBound, strings);
	m->appendEntry(e);
	return e;
}
DebugMenuEntry *DebugMenuAddFloat32(const char *path, const char *name, float *ptr, TriggerFunc triggerFunc, float step, float lowerBound, float upperBound)
{
	Menu *m = findMenu(path);
	if (m == nil)
		return nil;
	DebugMenuEntry *e = new DebugMenuEntry_Float32(name, ptr, triggerFunc, step, lowerBound, upperBound);
	m->appendEntry(e);
	return e;
}
DebugMenuEntry *DebugMenuAddFloat64(const char *path, const char *name, double *ptr, TriggerFunc triggerFunc, double step, double lowerBound, double upperBound)
{
	Menu *m = findMenu(path);
	if (m == nil)
		return nil;
	DebugMenuEntry *e = new DebugMenuEntry_Float64(name, ptr, triggerFunc, step, lowerBound, upperBound);
	m->appendEntry(e);
	return e;
}

DebugMenuEntry *DebugMenuAddVarBool32(const char *path, const char *name, int32_t *ptr, TriggerFunc triggerFunc)
{
	static const char *boolstr[] = {"Off", "On"};
	DebugMenuEntry *e = DebugMenuAddVar(path, name, ptr, triggerFunc, 1, 0, 1, boolstr);
	DebugMenuEntrySetWrap(e, true);
	return e;
}
DebugMenuEntry *DebugMenuAddVarBool16(const char *path, const char *name, int16_t *ptr, TriggerFunc triggerFunc)
{
	static const char *boolstr[] = {"Off", "On"};
	DebugMenuEntry *e = DebugMenuAddVar(path, name, ptr, triggerFunc, 1, 0, 1, boolstr);
	DebugMenuEntrySetWrap(e, true);
	return e;
}
DebugMenuEntry *DebugMenuAddVarBool8(const char *path, const char *name, int8_t *ptr, TriggerFunc triggerFunc)
{
	static const char *boolstr[] = {"Off", "On"};
	DebugMenuEntry *e = DebugMenuAddVar(path, name, ptr, triggerFunc, 1, 0, 1, boolstr);
	DebugMenuEntrySetWrap(e, true);
	return e;
}

DebugMenuEntry *DebugMenuAddVar(const char *path, const char *name, int8_t *ptr, TriggerFunc triggerFunc, int8_t step, int8_t lowerBound, int8_t upperBound, const char **strings)
{
	return DebugMenuAddInt8(path, name, ptr, triggerFunc, step, lowerBound, upperBound, strings);
}
DebugMenuEntry *DebugMenuAddVar(const char *path, const char *name, int16_t *ptr, TriggerFunc triggerFunc, int16_t step, int16_t lowerBound, int16_t upperBound, const char **strings)
{
	return DebugMenuAddInt16(path, name, ptr, triggerFunc, step, lowerBound, upperBound, strings);
}
DebugMenuEntry *DebugMenuAddVar(const char *path, const char *name, int32_t *ptr, TriggerFunc triggerFunc, int32_t step, int32_t lowerBound, int32_t upperBound, const char **strings)
{
	return DebugMenuAddInt32(path, name, ptr, triggerFunc, step, lowerBound, upperBound, strings);
}
DebugMenuEntry *DebugMenuAddVar(const char *path, const char *name, int64_t *ptr, TriggerFunc triggerFunc, int64_t step, int64_t lowerBound, int64_t upperBound, const char **strings)
{
	return DebugMenuAddInt64(path, name, ptr, triggerFunc, step, lowerBound, upperBound, strings);
}
DebugMenuEntry *DebugMenuAddVar(const char *path, const char *name, uint8_t *ptr, TriggerFunc triggerFunc, uint8_t step, uint8_t lowerBound, uint8_t upperBound, const char **strings)
{
	return DebugMenuAddUInt8(path, name, ptr, triggerFunc, step, lowerBound, upperBound, strings);
}
DebugMenuEntry *DebugMenuAddVar(const char *path, const char *name, uint16_t *ptr, TriggerFunc triggerFunc, uint16_t step, uint16_t lowerBound, uint16_t upperBound, const char **strings)
{
	return DebugMenuAddUInt16(path, name, ptr, triggerFunc, step, lowerBound, upperBound, strings);
}
DebugMenuEntry *DebugMenuAddVar(const char *path, const char *name, uint32_t *ptr, TriggerFunc triggerFunc, uint32_t step, uint32_t lowerBound, uint32_t upperBound, const char **strings)
{
	return DebugMenuAddUInt32(path, name, ptr, triggerFunc, step, lowerBound, upperBound, strings);
}
DebugMenuEntry *DebugMenuAddVar(const char *path, const char *name, uint64_t *ptr, TriggerFunc triggerFunc, uint64_t step, uint64_t lowerBound, uint64_t upperBound, const char **strings)
{
	return DebugMenuAddUInt64(path, name, ptr, triggerFunc, step, lowerBound, upperBound, strings);
}
DebugMenuEntry *DebugMenuAddVar(const char *path, const char *name, float *ptr, TriggerFunc triggerFunc, float step, float lowerBound, float upperBound)
{
	return DebugMenuAddFloat32(path, name, ptr, triggerFunc, step, lowerBound, upperBound);
}
DebugMenuEntry *DebugMenuAddVar(const char *path, const char *name, double *ptr, TriggerFunc triggerFunc, double step, double lowerBound, double upperBound)
{
	return DebugMenuAddFloat64(path, name, ptr, triggerFunc, step, lowerBound, upperBound);
}
