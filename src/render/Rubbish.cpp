#include "common.h"
#include "patcher.h"
#include "Rubbish.h"

WRAPPER void CRubbish::Render(void) { EAXJMP(0x512190); }
WRAPPER void CRubbish::StirUp(CVehicle *veh) { EAXJMP(0x512690); }
WRAPPER void CRubbish::Update(void) { EAXJMP(0x511B90); }
