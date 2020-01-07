#include "common.h"
#include "patcher.h"
#include "Explosion.h"

WRAPPER void CExplosion::AddExplosion(CEntity *explodingEntity, CEntity *culprit, eExplosionType type, const CVector &pos, uint32) { EAXJMP(0x5591C0); }
WRAPPER void CExplosion::RemoveAllExplosionsInArea(CVector, float) { EAXJMP(0x55AD40); }
WRAPPER bool CExplosion::TestForExplosionInArea(eExplosionType, float, float, float, float, float, float) { EAXJMP(0x55AC80); }

WRAPPER
int8 CExplosion::GetExplosionActiveCounter(uint8 id)
{
    EAXJMP(0x559140);
}

WRAPPER
CVector *CExplosion::GetExplosionPosition(uint8 id)
{
    EAXJMP(0x5591A0);
}

WRAPPER
uint8 CExplosion::GetExplosionType(uint8 id)
{
    EAXJMP(0x559180);
}

WRAPPER
void CExplosion::ResetExplosionActiveCounter(uint8 id)
{
    EAXJMP(0x559160);
}
