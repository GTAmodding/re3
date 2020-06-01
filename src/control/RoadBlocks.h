#pragma once
#include "common.h"

class CVehicle;

class CScriptRoadblock
{
public:
	CVector m_vInf;
	CVector m_vSup;
	bool m_bInUse;
	CVector GetPosition() { return (m_vInf + m_vSup) / 2; }
};

class CRoadBlocks
{
public:
	static int16 NumRoadBlocks;
	static int16 RoadBlockNodes[NUMROADBLOCKS];
	static bool InOrOut[NUMROADBLOCKS];
	static CScriptRoadblock aScriptRoadBlocks[NUM_SCRIPT_ROADBLOCKS];

	static void Init(void);
	static void GenerateRoadBlockCopsForCar(CVehicle* pVehicle, int32 roadBlockType);
	static void GenerateRoadBlocks(void);

	static void CreateRoadBlockBetween2Points(CVector, CVector);
	static void RegisterScriptRoadBlock(CVector, CVector);
	static void ClearScriptRoadBlocks();
};
