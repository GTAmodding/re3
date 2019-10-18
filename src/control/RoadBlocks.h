#pragma once
#include "common.h"

class CVehicle;

class CRoadBlocks
{
public:
	static void Init(void);
	static void GenerateRoadBlockCopsForCar(CVehicle*, int32, int16);
	static void GenerateRoadBlocks(void);
};
