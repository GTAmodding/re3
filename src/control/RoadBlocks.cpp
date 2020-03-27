#include "common.h"
#include "patcher.h"
#include "RoadBlocks.h"
#include "PathFind.h"

int16 &CRoadBlocks::NumRoadBlocks = *(int16*)0x95CC34;
int16 (&CRoadBlocks::RoadBlockObjects)[NUMROADBLOCKS] = *(int16(*)[NUMROADBLOCKS]) * (uintptr*)0x72B3A8;
bool (&CRoadBlocks::InOrOut)[NUMROADBLOCKS] = *(bool(*)[NUMROADBLOCKS]) * (uintptr*)0x733810;

WRAPPER void CRoadBlocks::GenerateRoadBlockCopsForCar(CVehicle*, int32, int16) { EAXJMP(0x4376A0); }
WRAPPER void CRoadBlocks::GenerateRoadBlocks(void) { EAXJMP(0x436FA0); }

void
CRoadBlocks::Init(void)
{
    NumRoadBlocks = 0;
    for (int objId = 0; objId < ThePaths.m_numMapObjects; objId++) {
        if (ThePaths.m_objectFlags[objId] & UseInRoadBlock) {
            if (NumRoadBlocks < 600) {
                InOrOut[NumRoadBlocks] = true;
                RoadBlockObjects[NumRoadBlocks] = objId;
                NumRoadBlocks++;
            } else {
#ifndef MASTER
                printf("Not enough room for the potential roadblocks\n");
#endif
                // FIX: Don't iterate loop after NUMROADBLOCKS
                return;
            }
        }
    }

}

STARTPATCHES
	InjectHook(0x436F50, &CRoadBlocks::Init, PATCH_JUMP);
ENDPATCHES