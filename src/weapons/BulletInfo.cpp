#include "common.h"
#include "patcher.h"
#include "BulletInfo.h"

WRAPPER bool CBulletInfo::TestForSniperBullet(float x1, float x2, float y1, float y2, float z1, float z2) { EAXJMP(0x558D40); }
