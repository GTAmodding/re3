#include "common.h"
#include "patcher.h"
#include "Skidmarks.h"

WRAPPER void CSkidmarks::Clear(void) { EAXJMP(0x518130); }
WRAPPER void CSkidmarks::Update() { EAXJMP(0x518200); }

WRAPPER void CSkidmarks::Render(void) { EAXJMP(0x5182E0); }
WRAPPER void CSkidmarks::RegisterOne(uint32 id, CVector pos, float fwdx, float fwdY, bool *isMuddy, bool *isBloddy) { EAXJMP(0x5185C0); }
