#pragma once
#include "common.h"

class CVehicle;

class CRoadBlocks
{
public:
	static int16 (&NumRoadBlocks);
	static int16 (&RoadBlockObjects)[NUMROADBLOCKS];
	static bool (&InOrOut)[NUMROADBLOCKS];

	static void Init(void);
	static void GenerateRoadBlockCopsForCar(CVehicle*, int32, int16);
	static void GenerateRoadBlocks(void);
};
