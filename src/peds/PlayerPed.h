#pragma once

#include "Ped.h"

class CPad;
class CCopPed;
class CWanted;

class CPlayerPed : public CPed
{
public:
	CWanted *m_pWanted;
	CCopPed *m_pArrestingCop;
	float m_fMoveSpeed;
	float m_fCurrentStamina;
	float m_fMaxStamina;
	float m_fStaminaProgress;
	int8 m_nSelectedWepSlot;	// eWeaponType
	bool m_bSpeedTimerFlag;
	uint8 m_nEvadeAmount;
	int8 field_1367;
	uint32 m_nSpeedTimer;
	uint32 m_nHitAnimDelayTimer;
	float m_fAttackButtonCounter;
	bool m_bHaveTargetSelected;	// may have better name
	CEntity *m_pEvadingFrom;	// is this CPhysical?
	int32 m_nTargettableObjects[4];
	bool m_bAdrenalineActive;
	bool m_bHasLockOnTarget;
	uint32 m_nAdrenalineTime;
	bool m_bCanBeDamaged;
	int8 field_1413;
	CVector m_vecSafePos[6]; // safe places from the player, for example behind a tree
	CPed *m_pPedAtSafePos[6];
	float m_fWalkAngle;
	float m_fFPSMoveHeading;

	CPlayerPed();
	~CPlayerPed();
	void SetMoveAnim() { };

	void ReApplyMoveAnims(void);
	void ClearWeaponTarget(void);
	void SetWantedLevel(int32 level);
	void SetWantedLevelNoDrop(int32 level);
	void KeepAreaAroundPlayerClear(void);
	void AnnoyPlayerPed(bool);
	void MakeChangesForNewWeapon(int8);
	void SetInitialState(void);
	void ProcessControl(void);
	void ClearAdrenaline(void);
	void UseSprintEnergy(void);
	class CPlayerInfo *GetPlayerInfoForThisPlayerPed();
	void SetRealMoveAnim(void);
	void RestoreSprintEnergy(float);
	bool DoWeaponSmoothSpray(void);
	float GetWeaponSmoothSprayRate(void);
	void DoStuffToGoOnFire(void);
	bool DoesTargetHaveToBeBroken(CVector, CWeapon*);
	void RunningLand(CPad*);
	bool IsThisPedAttackingPlayer(CPed*);
	void PlayerControlSniper(CPad*);
	void PlayerControlM16(CPad*);
	void PlayerControlFighter(CPad*);
	void ProcessWeaponSwitch(CPad*);
	void MakeObjectTargettable(int32);
	void PlayerControl1stPersonRunAround(CPad *padUsed);
	void EvaluateNeighbouringTarget(CEntity*, CEntity**, float*, float, float, bool);
	void EvaluateTarget(CEntity*, CEntity**, float*, float, float, bool);
	bool FindNextWeaponLockOnTarget(CEntity*, bool);
	bool FindWeaponLockOnTarget(void);
	void ProcessAnimGroups(void);
	void ProcessPlayerWeapon(CPad*);
	void PlayerControlZelda(CPad*);

	static void SetupPlayerPed(int32);
	static void DeactivatePlayerPed(int32);
	static void ReactivatePlayerPed(int32);

#ifdef COMPATIBLE_SAVES
	virtual void Save(uint8*& buf);
	virtual void Load(uint8*& buf);
#endif

	static const uint32 nSaveStructSize;
};

#ifndef PED_SKIN
VALIDATE_SIZE(CPlayerPed, 0x5F0);
#endif
