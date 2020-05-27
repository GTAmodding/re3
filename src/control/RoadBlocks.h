#pragma once
#include "common.h"

class CVehicle;

class CRoadBlocks
{
public:
	static int16 NumRoadBlocks;
#ifndef MIAMI
	static int16 RoadBlockObjects[NUMROADBLOCKS];
#else
	static int16 RoadBlockNodes[NUMROADBLOCKS];
#endif
	static bool InOrOut[NUMROADBLOCKS];

	static void Init(void);
	static void GenerateRoadBlockCopsForCar(CVehicle* pVehicle, int32 roadBlockType);
	static void GenerateRoadBlocks(void);
};
