#include "common.h"
#include "patcher.h"
#include "WeaponEffects.h"

WRAPPER void CWeaponEffects::Render(void) { EAXJMP(0x564D70); }

CWeaponEffects &gCrossHair = *(CWeaponEffects*)0x6503BC;

void
CWeaponEffects::ClearCrossHair()
{
	gCrossHair.m_bCrosshair = false;
}

void
CWeaponEffects::MarkTarget(CVector pos, uint8 red, uint8 green, uint8 blue, uint8 alpha, float size)
{
    gCrossHair.m_bCrosshair = true;
    gCrossHair.m_vecPos = pos;
    gCrossHair.m_red = red;
    gCrossHair.m_green = green;
    gCrossHair.m_blue = blue;
    gCrossHair.m_alpha = alpha;
    gCrossHair.m_size = size;
}
