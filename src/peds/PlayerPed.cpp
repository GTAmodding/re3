#include "common.h"
#include "patcher.h"
#include "PlayerPed.h"
#include "Camera.h"
#include "WeaponEffects.h"
#include "ModelIndices.h"
#include "World.h"
#include "RpAnimBlend.h"
#include "General.h"
#include "Pools.h"
#include "Darkel.h"

CPlayerPed::~CPlayerPed()
{
	delete m_pWanted;
}

WRAPPER void CPlayerPed::KeepAreaAroundPlayerClear(void) { EAXJMP(0x4F3460); }
WRAPPER void CPlayerPed::ProcessControl(void) { EAXJMP(0x4EFD90); }

CPlayerPed::CPlayerPed(void) : CPed(PEDTYPE_PLAYER1)
{
	m_fMoveSpeed = 0.0f;
	SetModelIndex(MI_PLAYER);
	SetInitialState();

	m_pWanted = new CWanted();
	m_pWanted->Initialise();
	m_pArrestingCop = nil;
	m_currentWeapon = WEAPONTYPE_UNARMED;
	m_nSelectedWepSlot = WEAPONTYPE_UNARMED;
	m_nSpeedTimer = 0;
	m_bSpeedTimerFlag = 0;
	m_pPointGunAt = nil;
	m_nPedState = PED_IDLE;
	m_fMaxStamina = 150.0f;
	m_fCurrentStamina = m_fMaxStamina;
	m_fStaminaProgress = 0.0f;
	m_bShouldEvade = 0;
	field_1367 = 0;
	m_nShotDelay = 0;
	field_1376 = 0.0f;
	field_1380 = 0;
	m_bHasLockOnTarget = false;
	m_bCanBeDamaged = true;
	m_fWalkAngle = 0.0f;
	m_fFPSMoveHeading = 0.0f;
	m_nTargettableObjects[0] = m_nTargettableObjects[1] = m_nTargettableObjects[2] = m_nTargettableObjects[3] = -1;
	field_1413 = 0;
	for (int i = 0; i < 6; i++) {
		m_vecSafePos[i] = CVector(0.0f, 0.0f, 0.0f);
		m_pPedAtSafePos[i] = nil;
	}
}

void CPlayerPed::ClearWeaponTarget()
{
	if (m_nPedType == PEDTYPE_PLAYER1) {
		m_pPointGunAt = nil;
		TheCamera.ClearPlayerWeaponMode();
		CWeaponEffects::ClearCrosshair();
	}
	ClearPointGunAt();
}

void
CPlayerPed::SetWantedLevel(int32 level)
{
	m_pWanted->SetWantedLevel(level);
}

void
CPlayerPed::SetWantedLevelNoDrop(int32 level)
{
	m_pWanted->SetWantedLevelNoDrop(level);
}

void
CPlayerPed::MakeObjectTargettable(int32 handle)
{
	for (int i = 0; i < ARRAY_SIZE(m_nTargettableObjects); i++) {
		if (CPools::GetObjectPool()->GetAt(m_nTargettableObjects[i]) == nil) {
			m_nTargettableObjects[i] = handle;
			return;
		}
	}
}

// I don't know the actual purpose of parameter
void
CPlayerPed::AnnoyPlayerPed(bool annoyedByPassingEntity)
{
	if (m_pedStats->m_temper < 52) {
		m_pedStats->m_temper++;
	} else {
		if (annoyedByPassingEntity) {
			if (m_pedStats->m_temper < 55) {
				m_pedStats->m_temper++;
			} else {
				m_pedStats->m_temper = 46;
			}
		}
	}
}

void
CPlayerPed::ClearAdrenaline(void)
{
	if (m_bAdrenalineActive && m_nAdrenalineTime != 0) {
		m_nAdrenalineTime = 0;
		CTimer::SetTimeScale(1.0f);
	}
}

CPlayerInfo *
CPlayerPed::GetPlayerInfoForThisPlayerPed()
{
	if (CWorld::Players[0].m_pPed == this)
		return &CWorld::Players[0];

	return nil;
}

void
CPlayerPed::SetupPlayerPed(int32 index)
{
	CPlayerPed *player = new CPlayerPed();
	CWorld::Players[index].m_pPed = player;

	player->SetOrientation(0.0f, 0.0f, 0.0f);

	CWorld::Add(player);
	player->m_wepAccuracy = 100;
}

void
CPlayerPed::DeactivatePlayerPed(int32 index)
{
	CWorld::Remove(CWorld::Players[index].m_pPed);
}

void
CPlayerPed::ReactivatePlayerPed(int32 index)
{
	CWorld::Add(CWorld::Players[index].m_pPed);
}

void
CPlayerPed::UseSprintEnergy(void)
{
	if (m_fCurrentStamina > -150.0f && !CWorld::Players[CWorld::PlayerInFocus].m_bInfiniteSprint
		&& !m_bAdrenalineActive) {
		m_fCurrentStamina = m_fCurrentStamina - CTimer::GetTimeStep();
		m_fStaminaProgress = m_fStaminaProgress + CTimer::GetTimeStep();
	}

	if (m_fStaminaProgress >= 500.0f) {
		m_fStaminaProgress = 0;
		if (m_fMaxStamina < 1000.0f)
			m_fMaxStamina += 10.0f;
	}
}

void
CPlayerPed::MakeChangesForNewWeapon(int8 weapon)
{
	if (m_nPedState == PED_SNIPER_MODE) {
		RestorePreviousState();
		TheCamera.ClearPlayerWeaponMode();
	}
	SetCurrentWeapon(weapon);

	GetWeapon()->m_nAmmoInClip = min(GetWeapon()->m_nAmmoTotal, CWeaponInfo::GetWeaponInfo(GetWeapon()->m_eWeaponType)->m_nAmountofAmmunition);

	if (!(CWeaponInfo::GetWeaponInfo(GetWeapon()->m_eWeaponType)->m_bCanAim))
		ClearWeaponTarget();

	CAnimBlendAssociation *weaponAnim = RpAnimBlendClumpGetAssociation(GetClump(), CWeaponInfo::GetWeaponInfo(WEAPONTYPE_SNIPERRIFLE)->m_AnimToPlay);
	if (weaponAnim) {
		weaponAnim->SetRun();
		weaponAnim->flags |= ASSOC_FADEOUTWHENDONE;
	}
	TheCamera.ClearPlayerWeaponMode();
}

void
CPlayerPed::ReApplyMoveAnims(void)
{
	static AnimationId moveAnims[] = { ANIM_WALK, ANIM_RUN, ANIM_SPRINT, ANIM_IDLE_STANCE, ANIM_WALK_START };

	for(int i = 0; i < ARRAY_SIZE(moveAnims); i++) {
		CAnimBlendAssociation *curMoveAssoc = RpAnimBlendClumpGetAssociation(GetClump(), moveAnims[i]);
		if (curMoveAssoc) {
			if (strcmp(CAnimManager::GetAnimAssociation(m_animGroup, moveAnims[i])->hierarchy->name, curMoveAssoc->hierarchy->name) != 0) {
				CAnimBlendAssociation *newMoveAssoc = CAnimManager::AddAnimation(GetClump(), m_animGroup, moveAnims[i]);
				newMoveAssoc->blendDelta = curMoveAssoc->blendDelta;
				newMoveAssoc->blendAmount = curMoveAssoc->blendAmount;
				curMoveAssoc->blendDelta = -1000.0f;
				curMoveAssoc->flags |= ASSOC_DELETEFADEDOUT;
			}
		}
	}
}

void
CPlayerPed::SetInitialState(void)
{
	m_bAdrenalineActive = false;
	m_nAdrenalineTime = 0;
	CTimer::SetTimeStep(1.0f);
	m_pSeekTarget = nil;
	m_vecSeekPos = { 0.0f, 0.0f, 0.0f };
	m_fleeFromPosX = 0.0f;
	m_fleeFromPosY = 0.0f;
	m_fleeFrom = nil;
	m_fleeTimer = 0;
	m_objective = OBJECTIVE_NONE;
	m_prevObjective = OBJECTIVE_NONE;
	bUsesCollision = true;
	ClearAimFlag();
	ClearLookFlag();
	bIsPointingGunAt = false;
	bRenderPedInCar = true;
	if (m_pFire)
		m_pFire->Extinguish();
	RpAnimBlendClumpRemoveAllAssociations(GetClump());
	m_nPedState = PED_IDLE;
	SetMoveState(PEDMOVE_STILL);
	m_nLastPedState = PED_NONE;
	m_animGroup = ASSOCGRP_PLAYER;
	m_fMoveSpeed = 0.0f;
	m_nSelectedWepSlot = WEAPONTYPE_UNARMED;
	m_bShouldEvade = false;
	m_pEvadingFrom = nil;
	bIsPedDieAnimPlaying = false;
	SetRealMoveAnim();
	m_bCanBeDamaged = true;
	m_pedStats->m_temper = 50;
	m_fWalkAngle = 0.0f;
}

void
CPlayerPed::SetRealMoveAnim(void)
{
	CAnimBlendAssociation *curWalkAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_WALK);
	CAnimBlendAssociation *curRunAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_RUN);
	CAnimBlendAssociation *curSprintAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_SPRINT);
	CAnimBlendAssociation *curWalkStartAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_WALK_START);
	CAnimBlendAssociation *curIdleAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_IDLE_STANCE);
	CAnimBlendAssociation *curRunStopAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_RUN_STOP);
	CAnimBlendAssociation *curRunStopRAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_RUN_STOP_R);
	if (bResetWalkAnims) {
		if (curWalkAssoc)
			curWalkAssoc->SetCurrentTime(0.0f);
		if (curRunAssoc)
			curRunAssoc->SetCurrentTime(0.0f);
		if (curSprintAssoc)
			curSprintAssoc->SetCurrentTime(0.0f);
		bResetWalkAnims = false;
	}

	if (!curIdleAssoc)
		curIdleAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_IDLE_TIRED);
	if (!curIdleAssoc)
		curIdleAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_FIGHT_IDLE);

	if ((!curRunStopAssoc || !(curRunStopAssoc->IsRunning())) && (!curRunStopRAssoc || !(curRunStopRAssoc->IsRunning()))) {

		if (curRunStopAssoc && curRunStopAssoc->blendDelta >= 0.0f || curRunStopRAssoc && curRunStopRAssoc->blendDelta >= 0.0f) {
			if (curRunStopAssoc) {
				curRunStopAssoc->flags |= ASSOC_DELETEFADEDOUT;
				curRunStopAssoc->blendAmount = 1.0f;
				curRunStopAssoc->blendDelta = -8.0f;
			} else if (curRunStopRAssoc) {
				curRunStopRAssoc->flags |= ASSOC_DELETEFADEDOUT;
				curRunStopRAssoc->blendAmount = 1.0f;
				curRunStopRAssoc->blendDelta = -8.0f;
			}
			
			RestoreHeadingRate();
			if (!curIdleAssoc) {
				if (m_fCurrentStamina < 0.0f && !CWorld::TestSphereAgainstWorld(GetPosition(), 0.0f,
						nil, true, false, false, false, false, false)) {
					curIdleAssoc = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_IDLE_TIRED, 8.0f);

				} else {
					curIdleAssoc = CAnimManager::BlendAnimation(GetClump(), m_animGroup, ANIM_IDLE_STANCE, 8.0f);
				}
				m_nWaitTimer = CTimer::GetTimeInMilliseconds() + CGeneral::GetRandomNumberInRange(2500, 4000);
			}
			curIdleAssoc->blendAmount = 0.0f;
			curIdleAssoc->blendDelta = 8.0f;

		} else if (m_fMoveSpeed == 0.0f && !curSprintAssoc) {
			if (!curIdleAssoc) {
				if (m_fCurrentStamina < 0.0f && !CWorld::TestSphereAgainstWorld(GetPosition(), 0.0f,
						nil, true, false, false, false, false, false)) {
					curIdleAssoc = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_IDLE_TIRED, 4.0f);
					
				} else {
					curIdleAssoc = CAnimManager::BlendAnimation(GetClump(), m_animGroup, ANIM_IDLE_STANCE, 4.0f);
				}

				m_nWaitTimer = CTimer::GetTimeInMilliseconds() + CGeneral::GetRandomNumberInRange(2500, 4000);
			}

			if (m_fCurrentStamina > 0.0f && curIdleAssoc->animId == ANIM_IDLE_TIRED) {
				CAnimManager::BlendAnimation(GetClump(), m_animGroup, ANIM_IDLE_STANCE, 4.0f);

			} else if (m_nPedState != PED_FIGHT) {
				if (m_fCurrentStamina < 0.0f && curIdleAssoc->animId != ANIM_IDLE_TIRED
					&& !CWorld::TestSphereAgainstWorld(GetPosition(), 0.0f, nil, true, false, false, false, false, false)) {
					CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_IDLE_TIRED, 4.0f);

				} else if (curIdleAssoc->animId != ANIM_IDLE_STANCE) {
					CAnimManager::BlendAnimation(GetClump(), m_animGroup, ANIM_IDLE_STANCE, 4.0f);
				}
			}

			m_nMoveState = PEDMOVE_STILL;
		} else {
			if (curIdleAssoc) {
				if (curWalkStartAssoc) {
					curWalkStartAssoc->blendAmount = 1.0f;
					curWalkStartAssoc->blendDelta = 0.0f;
				} else {
					curWalkStartAssoc = CAnimManager::AddAnimation(GetClump(), m_animGroup, ANIM_WALK_START);
				}
				if (curWalkAssoc)
					curWalkAssoc->SetCurrentTime(0.0f);
				if (curRunAssoc)
					curRunAssoc->SetCurrentTime(0.0f);

				delete curIdleAssoc;
				delete RpAnimBlendClumpGetAssociation(GetClump(), ANIM_IDLE_TIRED);
				delete RpAnimBlendClumpGetAssociation(GetClump(), ANIM_FIGHT_IDLE);
				delete curSprintAssoc;

				curSprintAssoc = nil;
				m_nMoveState = PEDMOVE_WALK;
			}
			if (curRunStopAssoc) {
				delete curRunStopAssoc;
				RestoreHeadingRate();
			}
			if (curRunStopRAssoc) {
				delete curRunStopRAssoc;
				RestoreHeadingRate();
			}
			if (!curWalkAssoc) {
				curWalkAssoc = CAnimManager::AddAnimation(GetClump(), m_animGroup, ANIM_WALK);
				curWalkAssoc->blendAmount = 0.0f;
			}
			if (!curRunAssoc) {
				curRunAssoc = CAnimManager::AddAnimation(GetClump(), m_animGroup, ANIM_RUN);
				curRunAssoc->blendAmount = 0.0f;
			}
			if (curWalkStartAssoc && !(curWalkStartAssoc->IsRunning())) {
				delete curWalkStartAssoc;
				curWalkStartAssoc = nil;
				curWalkAssoc->SetRun();
				curRunAssoc->SetRun();
			}
			if (m_nMoveState == PEDMOVE_SPRINT) {
				if (m_fCurrentStamina < 0.0f && (m_fCurrentStamina <= -150.0f || !curSprintAssoc || curSprintAssoc->blendDelta < 0.0f))
					m_nMoveState = PEDMOVE_STILL;

				if (curWalkStartAssoc)
					m_nMoveState = PEDMOVE_STILL;
			}

			if (curSprintAssoc && (m_nMoveState != PEDMOVE_SPRINT || m_fMoveSpeed < 0.4f)) {
				if (curSprintAssoc->blendAmount == 0.0f) {
					curSprintAssoc->blendDelta = -1000.0f;
					curSprintAssoc->flags |= ASSOC_DELETEFADEDOUT;

				} else if (curSprintAssoc->blendDelta >= 0.0f || curSprintAssoc->blendAmount >= 0.8f) {
					if (m_fMoveSpeed < 0.4f) {
						AnimationId runStopAnim;
						if (curSprintAssoc->currentTime / curSprintAssoc->hierarchy->totalLength < 0.5) // double
							runStopAnim = ANIM_RUN_STOP;
						else
							runStopAnim = ANIM_RUN_STOP_R;
						CAnimBlendAssociation* newRunStopAssoc = CAnimManager::AddAnimation(GetClump(), ASSOCGRP_STD, runStopAnim);
						newRunStopAssoc->blendAmount = 1.0f;
						newRunStopAssoc->SetDeleteCallback(RestoreHeadingRateCB, this);
						m_headingRate = 0.0f;
						curSprintAssoc->flags |= ASSOC_DELETEFADEDOUT;
						curSprintAssoc->blendDelta = -1000.0f;
						curWalkAssoc->flags &= ~ASSOC_RUNNING;
						curWalkAssoc->blendAmount = 0.0f;
						curWalkAssoc->blendDelta = 0.0f;
						curRunAssoc->flags &= ~ASSOC_RUNNING;
						curRunAssoc->blendAmount = 0.0f;
						curRunAssoc->blendDelta = 0.0f;
					} else if (curSprintAssoc->blendDelta >= 0.0f) {

						// Stop sprinting when tired
						curSprintAssoc->flags |= ASSOC_DELETEFADEDOUT;
						curSprintAssoc->blendDelta = -1.0f;
						curRunAssoc->blendDelta = 1.0f;
					}
				} else if (m_fMoveSpeed < 1.0f) {
					curSprintAssoc->blendDelta = -8.0f;
					curRunAssoc->blendDelta = 8.0f;
				}
			} else if (curWalkStartAssoc) {
				curWalkAssoc->flags &= ~ASSOC_RUNNING;
				curRunAssoc->flags &= ~ASSOC_RUNNING;
				curWalkAssoc->blendAmount = 0.0f;
				curRunAssoc->blendAmount = 0.0f;

			} else if (m_nMoveState == PEDMOVE_SPRINT) {
				if (curSprintAssoc) {
					if (curSprintAssoc->blendDelta < 0.0f) {
						curSprintAssoc->blendDelta = 2.0f;
						curRunAssoc->blendDelta = -2.0f;
					}
				} else {
					curWalkAssoc->blendAmount = 0.0f;
					curRunAssoc->blendAmount = 1.0f;
					curSprintAssoc = CAnimManager::BlendAnimation(GetClump(), m_animGroup, ANIM_SPRINT, 2.0f);
				}
				UseSprintEnergy();
			} else {
				if (m_fMoveSpeed < 1.0f) {
					curWalkAssoc->blendAmount = 1.0f;
					curRunAssoc->blendAmount = 0.0f;
					m_nMoveState = PEDMOVE_WALK;
				} else if (m_fMoveSpeed < 2.0f) {
					curWalkAssoc->blendAmount = 2.0f - m_fMoveSpeed;
					curRunAssoc->blendAmount = m_fMoveSpeed - 1.0f;
					m_nMoveState = PEDMOVE_RUN;
				} else {
					curWalkAssoc->blendAmount = 0.0f;
					curRunAssoc->blendAmount = 1.0f;
					m_nMoveState = PEDMOVE_RUN;
				}
			}
		}
	}
	if (m_bAdrenalineActive) {
		if (CTimer::GetTimeInMilliseconds() > m_nAdrenalineTime) {
			m_bAdrenalineActive = false;
			CTimer::SetTimeScale(1.0f);
			if (curWalkStartAssoc)
				curWalkStartAssoc->speed = 1.0f;
			if (curWalkAssoc)
				curWalkAssoc->speed = 1.0f;
			if (curRunAssoc)
				curRunAssoc->speed = 1.0f;
			if (curSprintAssoc)
				curSprintAssoc->speed = 1.0f;
		} else {
			CTimer::SetTimeScale(1.0f / 3);
			if (curWalkStartAssoc)
				curWalkStartAssoc->speed = 2.0f;
			if (curWalkAssoc)
				curWalkAssoc->speed = 2.0f;
			if (curRunAssoc)
				curRunAssoc->speed = 2.0f;
			if (curSprintAssoc)
				curSprintAssoc->speed = 2.0f;
		}
	}
}

void
CPlayerPed::RestoreSprintEnergy(float restoreSpeed)
{
	if (m_fCurrentStamina < m_fMaxStamina)
		m_fCurrentStamina += restoreSpeed * CTimer::GetTimeStep() * 0.5f;
}

bool
CPlayerPed::DoWeaponSmoothSpray(void)
{
	if (m_nPedState == PED_ATTACK && !m_pPointGunAt) {
		eWeaponType weapon = GetWeapon()->m_eWeaponType;
		if (weapon == WEAPONTYPE_FLAMETHROWER || weapon == WEAPONTYPE_COLT45 || weapon == WEAPONTYPE_UZI || weapon == WEAPONTYPE_SHOTGUN || 
			weapon == WEAPONTYPE_AK47 || weapon == WEAPONTYPE_M16 || weapon == WEAPONTYPE_HELICANNON)
			return true;
	}
	return false;
}

void
CPlayerPed::DoStuffToGoOnFire(void)
{
	if (m_nPedState == PED_SNIPER_MODE)
		TheCamera.ClearPlayerWeaponMode();
}

bool
CPlayerPed::DoesTargetHaveToBeBroken(CVector target, CWeapon *weaponUsed)
{
	CVector distVec = target - GetPosition();

	if (distVec.Magnitude() > CWeaponInfo::GetWeaponInfo(weaponUsed->m_eWeaponType)->m_fRange)
		return true;

	if (weaponUsed->m_eWeaponType != WEAPONTYPE_SHOTGUN && weaponUsed->m_eWeaponType != WEAPONTYPE_AK47)
		return false;

	distVec.Normalise();

	if (DotProduct(distVec,GetForward()) < 0.4f)
		return true;

	return false;
}

// Cancels landing anim while running & jumping? I think
void
CPlayerPed::RunningLand(CPad *padUsed)
{
	CAnimBlendAssociation *landAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_FALL_LAND);
	if (landAssoc && landAssoc->currentTime == 0.0f && m_fMoveSpeed > 1.5f
		&& padUsed && (padUsed->GetPedWalkLeftRight() != 0.0f || padUsed->GetPedWalkUpDown() != 0.0f)) {

		landAssoc->blendDelta = -1000.0f;
		landAssoc->flags |= ASSOC_DELETEFADEDOUT;

		CAnimManager::AddAnimation(GetClump(), ASSOCGRP_STD, ANIM_JUMP_LAND)->SetFinishCallback(FinishJumpCB, this);

		if (m_nPedState == PED_JUMP)
			RestorePreviousState();
	}
}

bool
CPlayerPed::IsThisPedAttackingPlayer(CPed *suspect)
{
	if (suspect->m_pPointGunAt == this)
		return true;

	switch (suspect->m_objective) {
		case OBJECTIVE_KILL_CHAR_ON_FOOT:
		case OBJECTIVE_KILL_CHAR_ANY_MEANS:
			if (suspect->m_pedInObjective == this)
				return true;

			break;
		default:
			break;
	}
	return false;
}

void
CPlayerPed::PlayerControlSniper(CPad *padUsed)
{
	ProcessWeaponSwitch(padUsed);
	TheCamera.PlayerExhaustion = (1.0f - (m_fCurrentStamina - -150.0f) / 300.0f) * 0.9f + 0.1f;

	if (!padUsed->GetTarget()) {
		RestorePreviousState();
		TheCamera.ClearPlayerWeaponMode();
	}

	if (padUsed->WeaponJustDown()) {
		CVector firePos(0.0f, 0.0f, 0.6f);
		firePos = GetMatrix() * firePos;
		GetWeapon()->Fire(this, &firePos);
	}
	GetWeapon()->Update(m_audioEntityId);
}

// I think R* also used goto in here.
void
CPlayerPed::ProcessWeaponSwitch(CPad *padUsed)
{
	if (CDarkel::FrenzyOnGoing())
		goto switchDetectDone;

	// The fact that m_nSelectedWepSlot is int8 makes below loops circular loop.

	if (padUsed->CycleWeaponRightJustDown() && !m_pPointGunAt) {

		if (TheCamera.PlayerWeaponMode.Mode != CCam::MODE_M16_1STPERSON
			&& TheCamera.PlayerWeaponMode.Mode != CCam::MODE_M16_1STPERSON_RUNABOUT
			&& TheCamera.PlayerWeaponMode.Mode != CCam::MODE_SNIPER
			&& TheCamera.PlayerWeaponMode.Mode != CCam::MODE_SNIPER_RUNABOUT
			&& TheCamera.PlayerWeaponMode.Mode != CCam::MODE_ROCKETLAUNCHER
			&& TheCamera.PlayerWeaponMode.Mode != CCam::MODE_ROCKETLAUNCHER_RUNABOUT) {

			for (m_nSelectedWepSlot = m_currentWeapon + 1; m_nSelectedWepSlot < WEAPONTYPE_TOTAL_INVENTORY_WEAPONS; ++m_nSelectedWepSlot) {
				if (HasWeapon(m_nSelectedWepSlot) && GetWeapon(m_nSelectedWepSlot).HasWeaponAmmoToBeUsed()) {
					goto switchDetectDone;
				}
			}
			m_nSelectedWepSlot = WEAPONTYPE_UNARMED;
		}
	} else if (padUsed->CycleWeaponLeftJustDown() && !m_pPointGunAt) {
		if (TheCamera.PlayerWeaponMode.Mode != CCam::MODE_M16_1STPERSON
			&& TheCamera.PlayerWeaponMode.Mode != CCam::MODE_SNIPER
			&& TheCamera.PlayerWeaponMode.Mode != CCam::MODE_ROCKETLAUNCHER) {

			for (m_nSelectedWepSlot = m_currentWeapon - 1; m_nSelectedWepSlot >= 0; --m_nSelectedWepSlot) {
				if (HasWeapon(m_nSelectedWepSlot) && GetWeapon(m_nSelectedWepSlot).HasWeaponAmmoToBeUsed()) {
					goto switchDetectDone;
				}
			}
			m_nSelectedWepSlot = WEAPONTYPE_DETONATOR;
		}
	} else if (CWeaponInfo::GetWeaponInfo((eWeaponType)m_currentWeapon)->m_eWeaponFire != WEAPON_FIRE_MELEE) {
		if (GetWeapon(m_currentWeapon).m_nAmmoTotal <= 0) {
			if (TheCamera.PlayerWeaponMode.Mode != CCam::MODE_M16_1STPERSON
				&& TheCamera.PlayerWeaponMode.Mode != CCam::MODE_SNIPER
				&& TheCamera.PlayerWeaponMode.Mode != CCam::MODE_ROCKETLAUNCHER) {

				for (m_nSelectedWepSlot = m_currentWeapon - 1; m_nSelectedWepSlot >= 0; --m_nSelectedWepSlot) {
					if (m_nSelectedWepSlot == WEAPONTYPE_BASEBALLBAT && HasWeapon(WEAPONTYPE_BASEBALLBAT)
						|| GetWeapon(m_nSelectedWepSlot).m_nAmmoTotal > 0 && m_nSelectedWepSlot != WEAPONTYPE_MOLOTOV && m_nSelectedWepSlot != WEAPONTYPE_GRENADE) {
						goto switchDetectDone;
					}
				}
				m_nSelectedWepSlot = WEAPONTYPE_UNARMED;
			}
		}
	}

switchDetectDone:
	if (m_nSelectedWepSlot != m_currentWeapon) {
		if (m_nPedState != PED_ATTACK && m_nPedState != PED_AIM_GUN && m_nPedState != PED_FIGHT)
			MakeChangesForNewWeapon(m_nSelectedWepSlot);
	}
}

void
CPlayerPed::PlayerControlM16(CPad *padUsed)
{
	ProcessWeaponSwitch(padUsed);
	TheCamera.PlayerExhaustion = (1.0f - (m_fCurrentStamina - -150.0f) / 300.0f) * 0.9f + 0.1f;

	if (!padUsed->GetTarget()) {
		RestorePreviousState();
		TheCamera.ClearPlayerWeaponMode();
	}

	if (padUsed->GetWeapon()) {
		CVector firePos(0.0f, 0.0f, 0.6f);
		firePos = GetMatrix() * firePos;
		GetWeapon()->Fire(this, &firePos);
	}
	GetWeapon()->Update(m_audioEntityId);
}

void
CPlayerPed::PlayerControlFighter(CPad *padUsed)
{
	float leftRight = padUsed->GetPedWalkLeftRight();
	float upDown = padUsed->GetPedWalkUpDown();
	float displacement = sqrt(upDown * upDown + leftRight * leftRight);

	if (displacement > 0.0f) {
		m_fRotationDest = CGeneral::GetRadianAngleBetweenPoints(0.0f, 0.0f, -leftRight, upDown) - TheCamera.Orientation;
		m_takeAStepAfterAttack = displacement > 120.0f;
		if (padUsed->GetSprint() && displacement > 60.0f)
			bIsAttacking = false;
	}

	if (!CWeaponInfo::GetWeaponInfo(GetWeapon()->m_eWeaponType)->m_bHeavy && padUsed->JumpJustDown()) {
		if (m_bShouldEvade && m_pEvadingFrom) {
			SetEvasiveDive((CPhysical*)m_pEvadingFrom, 1);
			m_bShouldEvade = false;
			m_pEvadingFrom = nil;
		} else {
			SetJump();
		}
	}
}

class CPlayerPed_ : public CPlayerPed
{
public:
	CPlayerPed* ctor(void) { return ::new (this) CPlayerPed(); }
	void dtor(void) { CPlayerPed::~CPlayerPed(); }
	void SetMoveAnim_(void) { CPlayerPed::SetMoveAnim(); }
};

STARTPATCHES
	InjectHook(0x4EF7E0, &CPlayerPed_::ctor, PATCH_JUMP);
	InjectHook(0x4EFB30, &CPlayerPed_::dtor, PATCH_JUMP);
	InjectHook(0x4F3760, &CPlayerPed_::SetMoveAnim_, PATCH_JUMP);
	InjectHook(0x4F28A0, &CPlayerPed::ClearWeaponTarget, PATCH_JUMP);
	InjectHook(0x4F3700, &CPlayerPed::AnnoyPlayerPed, PATCH_JUMP);
	InjectHook(0x4F36C0, &CPlayerPed::GetPlayerInfoForThisPlayerPed, PATCH_JUMP);
	InjectHook(0x4F2560, &CPlayerPed::MakeChangesForNewWeapon, PATCH_JUMP);
	InjectHook(0x4F07C0, &CPlayerPed::ReApplyMoveAnims, PATCH_JUMP);
	InjectHook(0x4F0880, &CPlayerPed::SetRealMoveAnim, PATCH_JUMP);
	InjectHook(0x4F1810, &CPlayerPed::PlayerControlFighter, PATCH_JUMP);
	InjectHook(0x4F1340, &CPlayerPed::RestoreSprintEnergy, PATCH_JUMP);
	InjectHook(0x4F1380, &CPlayerPed::DoWeaponSmoothSpray, PATCH_JUMP);
	InjectHook(0x4F36E0, &CPlayerPed::DoStuffToGoOnFire, PATCH_JUMP);
	InjectHook(0x4F3350, &CPlayerPed::DoesTargetHaveToBeBroken, PATCH_JUMP);
	InjectHook(0x4F31D0, &CPlayerPed::RunningLand, PATCH_JUMP);
	InjectHook(0x4F2D00, &CPlayerPed::IsThisPedAttackingPlayer, PATCH_JUMP);
	InjectHook(0x4F1CF0, &CPlayerPed::PlayerControlSniper, PATCH_JUMP);
	InjectHook(0x4F2310, &CPlayerPed::ProcessWeaponSwitch, PATCH_JUMP);
	InjectHook(0x4F1DF0, &CPlayerPed::PlayerControlM16, PATCH_JUMP);
ENDPATCHES
