#pragma once

#include "AnimManager.h"
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
	WEAPONFLAG_RELOAD_LOOP2START = 1 << 11,
	WEAPONFLAG_USE_2ND = 1 << 12,
	WEAPONFLAG_GROUND_2ND = 1 << 13,
	WEAPONFLAG_FINISH_3RD = 1 << 14,
	WEAPONFLAG_RELOAD = 1 << 15,
	WEAPONFLAG_FIGHTMODE = 1 << 16,
	WEAPONFLAG_CROUCHFIRE = 1 << 17,
	WEAPONFLAG_COP3_RD = 1 << 18,
	WEAPONFLAG_GROUND_3RD = 1 << 19,
	WEAPONFLAG_PARTIALATTACK = 1 << 20,
	WEAPONFLAG_ANIMDETONATE = 1 << 21,
};

class CWeaponInfo {
	static CWeaponInfo ms_apWeaponInfos[WEAPONTYPE_TOTALWEAPONS];
	static char ms_aWeaponNames[WEAPONTYPE_TOTALWEAPONS][32];
public:
	static uint16 ms_aReloadSampleTime[WEAPONTYPE_TOTALWEAPONS];
	static int32 ms_aMaxAmmoForWeapon[WEAPONTYPE_TOTALWEAPONS];

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
	AssocGroupId m_AnimToPlay;
	float m_fAnimLoopStart;
	float m_fAnimLoopEnd;
	float m_fAnimFrameFire;
	float m_fAnim2LoopStart;
	float m_fAnim2LoopEnd;
	float m_fAnim2FrameFire;
	float m_fAnimBreakout;
	int32 m_nModelId;
	int32 m_nModel2Id;
	uint32 m_Flags;

	uint32 m_nWeaponSlot;

	static void Initialise(void);
	static void LoadWeaponData(void);
	static CWeaponInfo *GetWeaponInfo(eWeaponType weaponType);
	static eWeaponFire FindWeaponFireType(char *name);
	static eWeaponType FindWeaponType(char *name);
	static void Shutdown(void);
	static bool IsWeaponSlotAmmoMergeable(uint32 slot) { return slot == WEAPONSLOT_SHOTGUN || slot == WEAPONSLOT_SUBMACHINEGUN || slot == WEAPONSLOT_RIFLE; }
	bool IsFlagSet(uint32 flag) const { return (m_Flags & flag) != 0; }
};

VALIDATE_SIZE(CWeaponInfo, 0x64);
