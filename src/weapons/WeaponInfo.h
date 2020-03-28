#pragma once

enum AnimationId;
enum eWeaponFire;
enum eWeaponType;

class CWeaponInfo {
//	static CWeaponInfo(&ms_apWeaponInfos)[14];
	static CWeaponInfo ms_apWeaponInfos[14];
public:
	eWeaponFire m_eWeaponFire;
	float m_fRange;
	uint32 m_nFiringRate;
	uint32 m_nReload;
	uint32 m_nAmountofAmmunition;
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
	uint8 stuff;

	static void Initialise(void);
	static void LoadWeaponData(void);
	static CWeaponInfo *GetWeaponInfo(eWeaponType weaponType);
	static eWeaponFire FindWeaponFireType(char *name);
	static eWeaponType FindWeaponType(char *name);
	static void Shutdown(void);
};

static_assert(sizeof(CWeaponInfo) == 0x54, "CWeaponInfo: error");