#pragma once

#include "Object.h"
#include "Weapon.h"

struct CProjectileInfo : public CObject {
	eWeaponType m_eWeaponType;
	CEntity *m_pSource;
	int m_nExplosionTime;
	char m_bInUse;
	char field_13;
	char field_14;
	char field_15;
	CVector m_vecPos;

	static CProjectileInfo *GetProjectileInfo(int32 id);
	static CProjectileInfo *ms_apProjectile;
};

extern CProjectileInfo *gaProjectileInfo;