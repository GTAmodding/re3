#pragma once

enum eSaveStatus
{
	SAVESTATUS_SUCCESSFUL = 0,
	SAVESTATUS_ERR_SAVE_CREATE,
	SAVESTATUS_ERR_SAVE_WRITE,
	SAVESTATUS_ERR_SAVE_CLOSE,
	SAVESTATUS_ERR_LOAD_OPEN,
	SAVESTATUS_ERR_LOAD_READ,
	SAVESTATUS_ERR_LOAD_CLOSE,
	SAVESTATUS_ERR_DATA_INVALID,

	// unused
	SAVESTATUS_DELETEFAILED8,
	SAVESTATUS_DELETEFAILED9,
	SAVESTATUS_DELETEFAILED10,
};

enum
{
	SLOT_OK = 0,
	SLOT_EMPTY,
	SLOT_CORRUPTED
};

class C_PcSave
{
public:
	eSaveStatus nErrorCode;

	C_PcSave() : nErrorCode(SAVESTATUS_SUCCESSFUL) {}
	void PopulateSlotInfo();
	bool DeleteSlot(int32 slot);
	int8 SaveSlot(int32 slot);
	bool PcClassSaveRoutine(int32 a2, uint8 *data, uint32 size);
	static void SetSaveDirectory(const char *path);
};

extern C_PcSave PcSaveHelper;
