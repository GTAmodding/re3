#include "common.h"
#include "patcher.h"
#include "Cranes.h"

WRAPPER bool CCranes::IsThisCarBeingTargettedByAnyCrane(CVehicle*) { EAXJMP(0x5451E0); }
WRAPPER bool CCranes::IsThisCarBeingCarriedByAnyCrane(CVehicle*) { EAXJMP(0x545190); }
WRAPPER void CCranes::ActivateCrane(float, float, float, float, float, float, float, float, bool, bool, float, float) { EAXJMP(0x543650); }
WRAPPER void CCranes::DeActivateCrane(float, float) { EAXJMP(0x543890); }
WRAPPER void CCranes::InitCranes(void) { EAXJMP(0x543360); }
WRAPPER void CCranes::UpdateCranes(void) { EAXJMP(0x5439E0); }