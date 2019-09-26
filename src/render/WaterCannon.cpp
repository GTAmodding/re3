#include "common.h"
#include "patcher.h"
#include "WaterCannon.h"

CWaterCannon* aCannons = (CWaterCannon*)0x8F2CA8;

WRAPPER void CWaterCannons::Update(void) { EAXJMP(0x522510); }
WRAPPER void CWaterCannon::UpdateOne(uint32 id, CVector *pos, CVector *dir) { EAXJMP(0x522470); }
WRAPPER void CWaterCannon::Render(void) { EAXJMP(0x522550); }
