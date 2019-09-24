#include "CProjectileInfo.h"

CProjectileInfo *gaProjectileInfo = (CProjectileInfo *)0x64ED50;
CProjectileInfo *CProjectileInfo::ms_apProjectile = (CProjectileInfo *)0x87C748;

CProjectileInfo *
CProjectileInfo::GetProjectileInfo(int32 id)
{
	return &gaProjectileInfo[id];
}
