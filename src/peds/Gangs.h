#pragma once

#include "PedType.h"

struct CGangInfo
{
	int32 m_nVehicleMI;
	int32 m_nPedModel1MI;
	int32 m_nPedModel2MI;
	int8 m_nPedModelOverride;
	int32 m_Weapon1;
	int32 m_Weapon2;

	CGangInfo();
};

VALIDATE_SIZE(CGangInfo, 0x10);

enum {
	GANG_CUBAN = 0,
	GANG_HAITIAN,
	GANG_STREET,
	GANG_DIAZ,
	GANG_SECURITY,
	GANG_BIKER,
	GANG_PLAYER,
	GANG_GOLFER,
	GANG_9,
	NUM_GANGS
};

class CGangs
{
public:
	static void Initialise(void);
	static void SetGangVehicleModel(int16, int32);
	static void SetGangWeapons(int16, int32, int32);
	static void SetGangPedModelOverride(int16, int8);
	static int8 GetGangPedModelOverride(int16);
	static void SaveAllGangData(uint8 *, uint32 *);
	static void LoadAllGangData(uint8 *, uint32);

	static void SetGangPedModels(int16, int32, int32);
	static void SetWillAttackPlayerWithCops(ePedType type, bool will);
	static bool GetWillAttackPlayerWithCops(ePedType type);
	static int32 ChooseGangPedModel(int16);

	static bool HaveGangModelsLoaded(int16 gang);
	static int32 GetGangPedModel1(int16 gang) { return Gang[gang].m_nPedModel1MI; }
	static int32 GetGangPedModel2(int16 gang) { return Gang[gang].m_nPedModel2MI; }
	static int32 GetGangVehicleModel(int16 gang) { return Gang[gang].m_nVehicleMI; }
	static CGangInfo *GetGangInfo(int16 gang) { return &Gang[gang]; }
	static CGangInfo Gang[NUM_GANGS];

private:
	static bool GangAttackWithCops[NUM_GANGS];
};
