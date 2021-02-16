#pragma once

#include "AnimationId.h"
#include "WeaponType.h"

enum
{
	WEAPONFLAG_USE_GRAVITY = 1,
	WEAPONFLAG_SLOWS_DOWN = 1 << 1,
	WEAPONFLAG_DISSIPATES = 1 << 2,
	WEAPONFLAG_RAND_SPEED = 1 << 3,
	WEAPONFLAG_EXPANDS = 1 << 4,
	WEAPONFLAG_EXPLODES = 1 << 5,
	WEAPONFLAG_CANAIM = 1 << 6,
	WEAPONFLAG_CANAIM_WITHARM = 1 << 7,
	WEAPONFLAG_1ST_PERSON = 1 << 8,
	WEAPONFLAG_HEAVY = 1 << 9,
	WEAPONFLAG_THROW = 1 << 10,
};

class CWeaponInfo {
public:
	eWeaponFire m_eWeaponFire;
	float m_fRange;
	uint32 m_nFiringRate;
	uint32 m_nReload;
	int32 m_nAmountofAmmunition;
	uint32 m_nDamage;
	float m_fSpeed;
	float m_fRadius;
	float m_fLifespan;
	float m_fSpread;
	CVector m_vecFireOffset;
	AnimationId m_AnimToPlay;
	AnimationId m_Anim2ToPlay;
	float m_fAnimLoopStart;
	float m_fAnimLoopEnd;
	float m_fAnimFrameFire;
	float m_fAnim2FrameFire;
	int32 m_nModelId;
	uint32 m_Flags;

	static void Initialise(void);
	static void LoadWeaponData(void);
	static CWeaponInfo *GetWeaponInfo(eWeaponType weaponType);
	static eWeaponFire FindWeaponFireType(char *name);
	static eWeaponType FindWeaponType(char *name);
	static void Shutdown(void);
	bool IsFlagSet(uint32 flag) const { return (m_Flags & flag) != 0; }
};

VALIDATE_SIZE(CWeaponInfo, 0x54);