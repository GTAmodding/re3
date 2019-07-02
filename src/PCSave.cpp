#include "common.h"
#include "patcher.h"
#include "Frontend.h"
#include "PCSave.h"

WRAPPER void C_PcSave::SetSaveDirectory(const char *path) { EAXJMP(0x591EA0); }
WRAPPER int8 C_PcSave::PopulateSlotInfo() { EAXJMP(0x592090); }
WRAPPER int8 C_PcSave::DeleteSlot(int) { EAXJMP(0x5922F0); }
WRAPPER int8 C_PcSave::SaveSlot(int) { EAXJMP(0x591EC0); }

WRAPPER int8 CheckSlotDataValid(int) { EAXJMP(0x591A40); }

WRAPPER wchar *GetNameOfSavedGame(int counter) { EAXJMP(0x591B60); }
WRAPPER wchar *GetSavedGameDateAndTime(int counter) { EAXJMP(0x591B50); }


C_PcSave PcSaveHelper = *(C_PcSave*)0x8E2C60;
int *Slots = (int*)0x728040;
int *SlotFileName = (int*)0x6F07C8;
int *SlotSaveDate = (int*)0x72B858;
