#include "common.h"
#include "patcher.h"
#include "Radar.h"

WRAPPER void CRadar::Draw3dMarkers() { EAXJMP(0x4A4C70); }
WRAPPER void CRadar::DrawMap () { EAXJMP(0x4A4200); }
WRAPPER void CRadar::DrawBlips() { EAXJMP(0x4A42F0); }
