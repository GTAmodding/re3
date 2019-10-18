#include "common.h"
#include "patcher.h"
#include "RoadBlocks.h"

WRAPPER void CRoadBlocks::Init(void) { EAXJMP(0x436F50); }
WRAPPER void CRoadBlocks::GenerateRoadBlockCopsForCar(CVehicle*, int32, int16) { EAXJMP(0x4376A0); }
WRAPPER void CRoadBlocks::GenerateRoadBlocks(void) { EAXJMP(0x436FA0); }