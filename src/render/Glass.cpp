#include "common.h"
#include "patcher.h"
#include "Glass.h"

WRAPPER void CGlass::AskForObjectToBeRenderedInGlass(CEntity *ent) { EAXJMP(0x5033F0); }

WRAPPER void
CGlass::WindowRespondsToCollision(CEntity *ent, float amount, CVector speed, CVector point, bool foo)
{
	EAXJMP(0x503F10);
}

WRAPPER void
CGlass::WindowRespondsToSoftCollision(CEntity *ent, float amount)
{
	EAXJMP(0x504630);
}

WRAPPER void CGlass::Render(void) { EAXJMP(0x502350); }
WRAPPER void CGlass::Update(void) { EAXJMP(0x502050); }
