#pragma once

#include "AnimManager.h"
#include "AnimationId.h"
#include "WeaponType.h"

enum AssocGroupId;

class CWeaponInfo {
	static CWeaponInfo ms_apWeaponInfos[WEAPONTYPE_TOTALWEAPONS];
public:
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
	// flags
	uint8 m_bUseGravity : 1;
	uint8 m_bSlowsDown : 1;
	uint8 m_bDissipates : 1;
	uint8 m_bRandSpeed : 1;
	uint8 m_bExpands : 1;
	uint8 m_bExplodes : 1;
	uint8 m_bCanAim : 1;
	uint8 m_bCanAimWithArm : 1;

	uint8 m_b1stPerson : 1;
	uint8 m_bHeavy : 1;
	uint8 m_bThrow : 1;
	uint8 m_bReloadLoop2Start : 1;
	uint8 m_bUse2nd : 1;
	uint8 m_bGround2nd : 1;
	uint8 m_bFinish3rd : 1;
	uint8 m_bReload : 1;
	
	uint8 m_bFightMode : 1;
	uint8 m_bCrouchFire : 1;
	uint8 m_bCop3rd : 1;
	uint8 m_bGround3rd : 1;
	uint8 m_bPartialAttack : 1;
	uint8 m_bAnimDetonate : 1;

	uint32 m_nWeaponSlot;

	static void Initialise(void);
	static void LoadWeaponData(void);
	static CWeaponInfo *GetWeaponInfo(eWeaponType weaponType);
	static eWeaponFire FindWeaponFireType(char *name);
	static eWeaponType FindWeaponType(char *name);
	static void Shutdown(void);
};

VALIDATE_SIZE(CWeaponInfo, 0x64);
