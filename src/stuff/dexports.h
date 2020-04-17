#pragma once

#include "dmenu.h"

DebugMenuEntry *DebugMenuAddCmd(const char *path, const char *name, TriggerFunc triggerFunc);
void DebugMenuEntrySetWrap(DebugMenuEntry *e, bool wrap);
void DebugMenuEntrySetStrings(DebugMenuEntry *e, const char **strings);
void DebugMenuEntrySetAddress(DebugMenuEntry *e, void *addr);
DebugMenuEntry *DebugMenuAddInt8(const char *path, const char *name, int8_t *ptr, TriggerFunc triggerFunc, int8_t step, int8_t lowerBound, int8_t upperBound, const char **strings);
DebugMenuEntry *DebugMenuAddInt16(const char *path, const char *name, int16_t *ptr, TriggerFunc triggerFunc, int16_t step, int16_t lowerBound, int16_t upperBound, const char **strings);
DebugMenuEntry *DebugMenuAddInt32(const char *path, const char *name, int32_t *ptr, TriggerFunc triggerFunc, int32_t step, int32_t lowerBound, int32_t upperBound, const char **strings);
DebugMenuEntry *DebugMenuAddInt64(const char *path, const char *name, int64_t *ptr, TriggerFunc triggerFunc, int64_t step, int64_t lowerBound, int64_t upperBound, const char **strings);
DebugMenuEntry *DebugMenuAddUInt8(const char *path, const char *name, uint8_t *ptr, TriggerFunc triggerFunc, uint8_t step, uint8_t lowerBound, uint8_t upperBound, const char **strings);
DebugMenuEntry *DebugMenuAddUInt16(const char *path, const char *name, uint16_t *ptr, TriggerFunc triggerFunc, uint16_t step, uint16_t lowerBound, uint16_t upperBound, const char **strings);
DebugMenuEntry *DebugMenuAddUInt32(const char *path, const char *name, uint32_t *ptr, TriggerFunc triggerFunc, uint32_t step, uint32_t lowerBound, uint32_t upperBound, const char **strings);
DebugMenuEntry *DebugMenuAddUInt64(const char *path, const char *name, uint64_t *ptr, TriggerFunc triggerFunc, uint64_t step, uint64_t lowerBound, uint64_t upperBound, const char **strings);
DebugMenuEntry *DebugMenuAddFloat32(const char *path, const char *name, float *ptr, TriggerFunc triggerFunc, float step, float lowerBound, float upperBound);
DebugMenuEntry *DebugMenuAddFloat64(const char *path, const char *name, double *ptr, TriggerFunc triggerFunc, double step, double lowerBound, double upperBound);
DebugMenuEntry *DebugMenuAddVarBool32(const char *path, const char *name, int32_t *ptr, TriggerFunc triggerFunc);
DebugMenuEntry *DebugMenuAddVarBool16(const char *path, const char *name, int16_t *ptr, TriggerFunc triggerFunc);
DebugMenuEntry *DebugMenuAddVarBool8(const char *path, const char *name, int8_t *ptr, TriggerFunc triggerFunc);

DebugMenuEntry *DebugMenuAddVar(const char *path, const char *name, int8_t *ptr, TriggerFunc triggerFunc, int8_t step, int8_t lowerBound, int8_t upperBound, const char **strings);
DebugMenuEntry *DebugMenuAddVar(const char *path, const char *name, int16_t *ptr, TriggerFunc triggerFunc, int16_t step, int16_t lowerBound, int16_t upperBound, const char **strings);
DebugMenuEntry *DebugMenuAddVar(const char *path, const char *name, int32_t *ptr, TriggerFunc triggerFunc, int32_t step, int32_t lowerBound, int32_t upperBound, const char **strings);
DebugMenuEntry *DebugMenuAddVar(const char *path, const char *name, int64_t *ptr, TriggerFunc triggerFunc, int64_t step, int64_t lowerBound, int64_t upperBound, const char **strings);
DebugMenuEntry *DebugMenuAddVar(const char *path, const char *name, uint8_t *ptr, TriggerFunc triggerFunc, uint8_t step, uint8_t lowerBound, uint8_t upperBound, const char **strings);
DebugMenuEntry *DebugMenuAddVar(const char *path, const char *name, uint16_t *ptr, TriggerFunc triggerFunc, uint16_t step, uint16_t lowerBound, uint16_t upperBound, const char **strings);
DebugMenuEntry *DebugMenuAddVar(const char *path, const char *name, uint32_t *ptr, TriggerFunc triggerFunc, uint32_t step, uint32_t lowerBound, uint32_t upperBound, const char **strings);
DebugMenuEntry *DebugMenuAddVar(const char *path, const char *name, uint64_t *ptr, TriggerFunc triggerFunc, uint64_t step, uint64_t lowerBound, uint64_t upperBound, const char **strings);
DebugMenuEntry *DebugMenuAddVar(const char *path, const char *name, float *ptr, TriggerFunc triggerFunc, float step, float lowerBound, float upperBound);
DebugMenuEntry *DebugMenuAddVar(const char *path, const char *name, double *ptr, TriggerFunc triggerFunc, double step, double lowerBound, double upperBound);

void DebugMenuInit();
bool DebugMenuLoad();
void DebugMenuProcess();
void DebugMenuRender();