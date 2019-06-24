#pragma once

// Index into the PedType array
enum
{
	PEDTYPE_PLAYER1 = 0,
	PEDTYPE_PLAYER2 = 1,
	PEDTYPE_PLAYER3 = 2,
	PEDTYPE_PLAYER4 = 3,
	PEDTYPE_CIVMALE = 4,
	PEDTYPE_CIVFEMALE = 5,
	PEDTYPE_COP = 6,
	PEDTYPE_GANG1 = 7,
	PEDTYPE_GANG2 = 8,
	PEDTYPE_GANG3 = 9,
	PEDTYPE_GANG4 = 10,
	PEDTYPE_GANG5 = 11,
	PEDTYPE_GANG6 = 12,
	PEDTYPE_GANG7 = 13,
	PEDTYPE_GANG8 = 14,
	PEDTYPE_GANG9 = 15,
	PEDTYPE_EMERGENCY = 16,
	PEDTYPE_FIREMAN = 17,
	PEDTYPE_CRIMINAL = 18,
	PEDTYPE_SPECIAL = 21,
	PEDTYPE_PROSTITUTE = 20,
	PEDTYPE_UNUSED1,
	PEDTYPE_UNUSED2,

	NUM_PEDTYPES
};

enum
{
	PED_FLAG_PLAYER1 = 1 << 0,
	PED_FLAG_PLAYER2 = 1 << 1,
	PED_FLAG_PLAYER3 = 1 << 2,
	PED_FLAG_PLAYER4 = 1 << 3,
	PED_FLAG_CIVMALE = 1 << 4,
	PED_FLAG_CIVFEMALE = 1 << 5,
	PED_FLAG_COP = 1 << 6,
	PED_FLAG_GANG1 = 1 << 7,
	PED_FLAG_GANG2 = 1 << 8,
	PED_FLAG_GANG3 = 1 << 9,
	PED_FLAG_GANG4 = 1 << 10,
	PED_FLAG_GANG5 = 1 << 11,
	PED_FLAG_GANG6 = 1 << 12,
	PED_FLAG_GANG7 = 1 << 13,
	PED_FLAG_GANG8 = 1 << 14,
	PED_FLAG_GANG9 = 1 << 15,
	PED_FLAG_EMERGENCY = 1 << 16,
	PED_FLAG_PROSTITUTE = 1 << 17,
	PED_FLAG_CRIMINAL = 1 << 18,
	PED_FLAG_SPECIAL = 1 << 19,
	PED_FLAG_GUN = 1 << 20,
	PED_FLAG_COP_CAR = 1 << 21,
	PED_FLAG_FAST_CAR = 1 << 22,
	PED_FLAG_EXPLOSION = 1 << 23,
	PED_FLAG_FIREMAN = 1 << 24,
	PED_FLAG_DEADPEDS = 1 << 25,
};

class CPedType
{
	uint32 m_flag;
	float unknown1;
	float unknown2;
	float unknown3;
	float unknown4;
	float unknown5;
	uint32 m_threats;
	uint32 m_avoid;

	static CPedType *(&ms_apPedType)[NUM_PEDTYPES];
public:

	static void Initialise(void);
	static void Shutdown(void);
	static void LoadPedData(void);
	static int32 FindPedType(char *type);
	static uint32 FindPedFlag(char *type);
	static void Save(uint8 *buffer, uint32 *length);
	static void Load(uint8 *buffer, uint32 length);

	static uint32 GetFlag(int type) { return ms_apPedType[type]->m_flag; }
	static uint32 GetAvoid(int type) { return ms_apPedType[type]->m_avoid; }
	static uint32 GetThreats(int type) { return ms_apPedType[type]->m_threats; }
};

static_assert(sizeof(CPedType) == 0x20, "CPedType: error");
