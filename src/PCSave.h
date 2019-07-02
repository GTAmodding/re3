#pragma once

class C_PcSave
{
public:
	int32 m_nHelper;

	static void SetSaveDirectory(const char *path);
	int8 PopulateSlotInfo();
	int8 DeleteSlot(int);
	int8 SaveSlot(int);
};

extern int8 CheckSlotDataValid(int);
extern wchar *GetNameOfSavedGame(int counter);
extern wchar *GetSavedGameDateAndTime(int counter);

extern C_PcSave PcSaveHelper;
extern int *Slots;
extern int *SlotFileName;
extern int *SlotSaveDate;
