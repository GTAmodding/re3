#include "common.h"

#include "main.h"
#include "RpAnimBlend.h"
#include "AnimBlendClumpData.h"
#include "AnimBlendAssociation.h"
#include "Camera.h"
#include "CarCtrl.h"
#include "Darkel.h"
#include "DMAudio.h"
#include "FileMgr.h"
#include "General.h"
#include "Object.h"
#include "Pad.h"
#include "Particle.h"
#include "Ped.h"
#include "PlayerPed.h"
#include "Stats.h"
#include "TempColModels.h"
#include "VisibilityPlugins.h"
#include "Vehicle.h"
#include "Automobile.h"
#include "WaterLevel.h"
#include "World.h"
#include "Bike.h"
#include "Glass.h"

//--MIAMI: file done except TODOs

uint16 nPlayerInComboMove;
RpClump* flyingClumpTemp;

FightMove tFightMoves[NUM_FIGHTMOVES] =
{
  { NUM_STD_ANIMS,  0.0f,  0.0f,  0.0f,  0.0f,  1.0f, HITLEVEL_NULL, 0, 0 },
  { ANIM_PUNCH_R, 0.2f, 8.f/30.f, 0.0f, 0.3f, 1.0f, HITLEVEL_HIGH, 1, 0 },
  { ANIM_FIGHT_IDLE,  0.0f,  0.0f,  0.0f,  0.0f,  1.0f, HITLEVEL_NULL, 0, 0 },
  { ANIM_FIGHT_SH_F,  0.0f,  0.0f,  0.0f,  0.0f,  1.0f, HITLEVEL_NULL, 0, 0 },
  { ANIM_FIGHT_KNEE, 4.f/30.f, 0.2f, 0.0f, 0.6f, 1.0f, HITLEVEL_LOW, 2, 0 },
  { ANIM_FIGHT_LHOOK, 8.f/30.f, 10.f/30.f, 0.0f, 0.4f, 1.0f, HITLEVEL_HIGH, 3, 0 },
  { ANIM_FIGHT_JAB, 4.f/30.f, 0.2f, 0.0f, 0.7f, 1.0f, HITLEVEL_HIGH, 3, 0 },
  { ANIM_FIGHT_PUNCH, 4.f/30.f, 7.f/30.f, 10.f/30.f, 0.4f, 1.0f, HITLEVEL_HIGH, 1, 0 },
  { ANIM_FIGHT_LONGKICK, 8.f/30.f, 10.f/30.f, 0.0f, 0.5f, 1.0f, HITLEVEL_MEDIUM, 4, 0 },
  { ANIM_FIGHT_ROUNDHOUSE, 8.f/30.f, 10.f/30.f, 0.0f,  0.6f, 1.0f, HITLEVEL_MEDIUM, 4, 0 },
  { ANIM_FIGHT_KICK,  8.f/30.f,  10.f/30.f,  0.0f,  0.5f,  1.0f, HITLEVEL_HIGH, 2, 0 },
  { ANIM_FIGHT_HEAD,  8.f/30.f,  10.f/30.f,  0.0f,  0.5f,  1.0f, HITLEVEL_MEDIUM, 2, 0 },
  { ANIM_FIGHT_BKICK_L,  8.f/30.f,  10.f/30.f,  0.0f,  0.5f,  1.0f, HITLEVEL_LOW, 2, 0 },
  { ANIM_FIGHT_BKICK_L,  8.f/30.f,  10.f/30.f,  0.0f,  0.5f,  1.0f, HITLEVEL_LOW, 2, 0 },
  { ANIM_FIGHT_ELBOW_L,  8.f/30.f,  10.f/30.f,  0.0f,  0.5f,  1.0f, HITLEVEL_MEDIUM, 2, 0 },
  { ANIM_FIGHT_BKICK_R,  8.f/30.f,  10.f/30.f,  0.0f,  0.5f,  1.0f, HITLEVEL_MEDIUM, 2, 0 },
  { ANIM_FIGHT_ELBOW_R,  8.f/30.f,  10.f/30.f,  0.0f,  0.5f,  1.0f, HITLEVEL_HIGH, 2, 0 },
  { ANIM_KICK_FLOOR, 10.f/30.f, 14.f/30.f, 0.0f, 0.4f, 1.0f, HITLEVEL_GROUND, 1, 0 },
  { ANIM_HIT_FRONT,  0.0f,  0.0f,  0.0f,  0.0f,  1.0f, HITLEVEL_NULL, 0, 0 },
  { ANIM_HIT_BACK,  0.0f,  0.0f,  0.0f,  0.0f,  1.0f, HITLEVEL_NULL, 0, 0 },
  { ANIM_HIT_RIGHT,  0.0f,  0.0f,  0.0f,  0.0f,  1.0f, HITLEVEL_NULL, 0, 0 },
  { ANIM_HIT_LEFT,  0.0f,  0.0f,  0.0f,  0.0f,  1.0f, HITLEVEL_NULL, 0, 0 },
  { ANIM_HIT_BODYBLOW,  0.0f,  0.0f,  0.0f,  0.0f,  1.0f, HITLEVEL_NULL, 0, 0 },
  { ANIM_HIT_CHEST,  0.0f,  0.0f,  0.0f,  0.0f,  1.0f, HITLEVEL_NULL, 0, 0 },
  { ANIM_HIT_HEAD,  0.0f,  0.0f,  0.0f,  0.0f,  1.0f, HITLEVEL_NULL, 0, 0 },
  { ANIM_HIT_WALK,  0.0f,  0.0f,  0.0f,  0.0f,  1.0f, HITLEVEL_NULL, 0, 0 },
  { ANIM_FLOOR_HIT,  0.0f,  0.0f,  0.0f,  0.0f,  1.0f, HITLEVEL_NULL, 0, 0 },
  { ANIM_HIT_BEHIND,  0.0f,  0.0f,  0.0f,  0.0f,  1.0f, HITLEVEL_NULL, 0, 0 },
  { ANIM_WEAPON_FIRE, 4.f/30.f, 7.f/30.f, 10.f/30.f, 0.4f, 1.0f, HITLEVEL_HIGH, 1, 0 },
  { ANIM_WEAPON_CROUCHFIRE, 4.f/30.f, 7.f/30.f, 10.f/30.f, 0.4f, 1.0f, HITLEVEL_HIGH, 1, 0 },
  { ANIM_WEAPON_SPECIAL, 4.f / 30.f, 7.f / 30.f, 10.f / 30.f, 0.4f, 1.0f, HITLEVEL_HIGH, 1, 0 },
  { ANIM_FIGHT2_IDLE,  0.0f,  0.0f,  0.0f,  0.0f,  1.0f, HITLEVEL_NULL, 0, 0 }
};

static PedOnGroundState
CheckForPedsOnGroundToAttack(CPed *attacker, CPed **pedOnGround)
{
	PedOnGroundState stateToReturn;
	float angleToFace;
	CPed *currentPed = nil;
	PedState currentPedState;
	CPed *pedOnTheFloor = nil;
	CPed *deadPed = nil;
	CPed *pedBelow = nil;
	bool foundDead = false;
	bool foundOnTheFloor = false;
	bool foundBelow = false;
	float angleDiff;
	float distance;

	if (!CGame::nastyGame)
		return NO_PED;

	for (int currentPedId = 0; currentPedId < attacker->m_numNearPeds; currentPedId++) {

		currentPed = attacker->m_nearPeds[currentPedId];

		CVector posDifference = currentPed->GetPosition() - attacker->GetPosition();
		distance = posDifference.Magnitude();

		if (distance < 2.0f) {
			angleToFace = CGeneral::GetRadianAngleBetweenPoints(
				currentPed->GetPosition().x, currentPed->GetPosition().y,
				attacker->GetPosition().x, attacker->GetPosition().y);

			angleToFace = CGeneral::LimitRadianAngle(angleToFace);
			attacker->m_fRotationCur = CGeneral::LimitRadianAngle(attacker->m_fRotationCur);

			angleDiff = Abs(angleToFace - attacker->m_fRotationCur);

			if (angleDiff > PI)
				angleDiff = 2 * PI - angleDiff;

			currentPedState = currentPed->m_nPedState;

			if (currentPed->OnGroundOrGettingUp()) {
				if (distance < 2.0f && angleDiff < DEGTORAD(65.0f)) {
					if (currentPedState == PED_DEAD) {
						foundDead = 1;
						if (!deadPed)
							deadPed = currentPed;
					} else if (!currentPed->IsPedHeadAbovePos(-0.6f)) {
						foundOnTheFloor = 1;
						if (!pedOnTheFloor)
							pedOnTheFloor = currentPed;
					}
				}
			} else if ((distance < 0.8f && angleDiff < DEGTORAD(75.0f))
						|| (distance < 1.3f && angleDiff < DEGTORAD(55.0f))
						|| (distance < 1.7f && angleDiff < DEGTORAD(35.0f))
						|| (distance < 2.0f && angleDiff < DEGTORAD(30.0f))) {

				// Either this condition or below one was probably returning 4 early in development. See Fight().
				foundBelow = 1;
				pedBelow = currentPed;
				break;
			} else {
				if (angleDiff < DEGTORAD(75.0f)) {
					foundBelow = 1;
					if (!pedBelow)
						pedBelow = currentPed;
				}
			}
		}
	}

	if (foundOnTheFloor) {
		currentPed = pedOnTheFloor;
		stateToReturn = PED_ON_THE_FLOOR;
	} else if (foundDead) {
		currentPed = deadPed;
		stateToReturn = PED_DEAD_ON_THE_FLOOR;
	} else if (foundBelow) {
		currentPed = pedBelow;
		stateToReturn = PED_IN_FRONT_OF_ATTACKER;
	} else {
		currentPed = nil;
		stateToReturn = NO_PED;
	}

	if (pedOnGround)
		*pedOnGround = currentPed;

	return stateToReturn;
}

// --MIAMI: Done
void
CPed::SetPointGunAt(CEntity *to)
{
	if (to) {
		SetLookFlag(to, true, true);
		SetAimFlag(to);
		SetLookTimer(INT32_MAX);
	}

	CWeaponInfo* curWeapon = CWeaponInfo::GetWeaponInfo(GetWeapon()->m_eWeaponType);
	if (m_nPedState == PED_AIM_GUN || (bIsDucking && !IsPlayer()) || m_nWaitState == WAITSTATE_PLAYANIM_DUCK || curWeapon->m_AnimToPlay == ASSOCGRP_STD)
		return;

	if (m_nPedState != PED_ATTACK)
		SetStoredState();

	SetPedState(PED_AIM_GUN);
	bIsPointingGunAt = true;
	SetMoveState(PEDMOVE_STILL);

	CAnimBlendAssociation *aimAssoc;

	if (bCrouchWhenShooting && bIsDucking && GetCrouchFireAnim(curWeapon)) {
		aimAssoc = RpAnimBlendClumpGetAssociation(GetClump(), GetCrouchFireAnim(curWeapon));
	} else {
		aimAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_WEAPON_FIRE);
	}

	if (!aimAssoc || aimAssoc->blendDelta < 0.0f) {
		if (bCrouchWhenShooting && bIsDucking && GetCrouchFireAnim(curWeapon)) {
			aimAssoc = CAnimManager::BlendAnimation(GetClump(), curWeapon->m_AnimToPlay, GetCrouchFireAnim(curWeapon), 4.0f);
		} else {
			aimAssoc = CAnimManager::AddAnimation(GetClump(), curWeapon->m_AnimToPlay, ANIM_WEAPON_FIRE);
		}

		aimAssoc->blendAmount = 0.0f;
		aimAssoc->blendDelta = 8.0f;
	}
	if (to && !IsPlayer())
		Say(SOUND_PED_ATTACK);
}

// --MIAMI: Done
void
CPed::PointGunAt(void)
{
	CWeaponInfo *weaponInfo = CWeaponInfo::GetWeaponInfo(GetWeapon()->m_eWeaponType);
	float animLoopStart = weaponInfo->m_fAnimLoopStart;
	CAnimBlendAssociation *weaponAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_WEAPON_FIRE);
	if (!weaponAssoc || weaponAssoc->blendDelta < 0.0f) {
		if (!!weaponInfo->m_bCrouchFire) {
			weaponAssoc = RpAnimBlendClumpGetAssociation(GetClump(), GetCrouchFireAnim(weaponInfo));
			animLoopStart = weaponInfo->m_fAnim2LoopStart;
		}
	}

	if (weaponAssoc && weaponAssoc->currentTime > animLoopStart * 0.4f) {
		weaponAssoc->SetCurrentTime(animLoopStart);
		weaponAssoc->flags &= ~ASSOC_RUNNING;

		if (bIsDucking)
			m_pedIK.m_flags &= ~CPedIK::AIMS_WITH_ARM;

		if (weaponInfo->m_bCanAimWithArm)
			m_pedIK.m_flags |= CPedIK::AIMS_WITH_ARM;
		else
			m_pedIK.m_flags &= ~CPedIK::AIMS_WITH_ARM;
	}
}

// --MIAMI: Done
void
CPed::ClearPointGunAt(void)
{
	CAnimBlendAssociation *animAssoc;
	CWeaponInfo *weaponInfo;

	ClearLookFlag();
	ClearAimFlag();
	bIsPointingGunAt = false;
	if (m_nPedState == PED_AIM_GUN || m_nPedState == PED_ATTACK) {
		SetPedState(PED_IDLE);
		RestorePreviousState();
	}
	weaponInfo = CWeaponInfo::GetWeaponInfo(GetWeapon()->m_eWeaponType);
	animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_WEAPON_FIRE);
	if (!animAssoc || animAssoc->blendDelta < 0.0f) {
		if (!!weaponInfo->m_bCrouchFire) {
			animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), GetCrouchFireAnim(weaponInfo));
		}
	}
	if (animAssoc) {
		animAssoc->flags |= ASSOC_DELETEFADEDOUT;
		animAssoc->blendDelta = -4.0f;
	}
}

// --MIAMI: Done
void
CPed::SetAttack(CEntity *victim)
{
	CPed *victimPed = nil;
	CWeaponInfo *curWeapon = CWeaponInfo::GetWeaponInfo(GetWeapon()->m_eWeaponType);
	CAnimBlendAssociation *animAssoc;

	if (victim && victim->IsPed())
		victimPed = (CPed*)victim;

	if (m_attackTimer > CTimer::GetTimeInMilliseconds() || m_nWaitState == WAITSTATE_SURPRISE || (bIsDucking && !bCrouchWhenShooting))
		return;

	if (curWeapon->m_bReload &&
		(RpAnimBlendClumpGetAssociation(GetClump(), GetReloadAnim(curWeapon)) || RpAnimBlendClumpGetAssociation(GetClump(), GetCrouchReloadAnim(curWeapon)))) {
		if (!IsPlayer() || m_nPedState != PED_ATTACK || ((CPlayerPed*)this)->m_bHaveTargetSelected)
			bIsAttacking = false;
		else
			bIsAttacking = true;

		return;
	}

	if (GetWeapon()->m_eWeaponType == WEAPONTYPE_UNARMED || curWeapon->m_bFightMode || GetWeapon()->m_eWeaponType == WEAPONTYPE_BRASSKNUCKLE) {
		if (IsPlayer() ||
			(m_nPedState != PED_FIGHT && m_nMoveState != PEDMOVE_NONE && m_nMoveState != PEDMOVE_STILL
				&& !(m_pedStats->m_flags & STAT_SHOPPING_BAGS) && curWeapon->m_bPartialAttack)) {

			if (m_nPedState != PED_ATTACK) {
				SetPedState(PED_ATTACK);
				bIsAttacking = false;

				CAnimBlendAssociation *animAssoc = CAnimManager::BlendAnimation(GetClump(), curWeapon->m_AnimToPlay, ANIM_MELEE_ATTACK_START, 8.0f);
				animAssoc->SetRun();
				if (animAssoc->currentTime == animAssoc->hierarchy->totalLength)
					animAssoc->SetCurrentTime(0.0f);

				animAssoc->SetFinishCallback(FinishedAttackCB, this);
			}
		} else {
			StartFightAttack(CGeneral::GetRandomNumber());
		}
		return;
	}

	if (curWeapon->m_bPartialAttack &&
		(IsPlayer() && ((CPlayerPed*)this)->m_fMoveSpeed >= 1.0f ||
		m_nMoveState == PEDMOVE_WALK || m_nMoveState == PEDMOVE_RUN)) {

		if (m_nPedState != PED_ATTACK) {
			SetPedState(PED_ATTACK);
			bIsAttacking = false;
			CAnimBlendAssociation* animAssoc = CAnimManager::BlendAnimation(GetClump(), curWeapon->m_AnimToPlay, ANIM_MELEE_ATTACK_START, 8.0f);
			animAssoc->SetRun();
			if (animAssoc->currentTime == animAssoc->hierarchy->totalLength)
				animAssoc->SetCurrentTime(0.0f);

			animAssoc->SetFinishCallback(FinishedAttackCB, this);
		}
		return;
	}

	if (m_pSeekTarget)
		m_pSeekTarget->CleanUpOldReference(&m_pSeekTarget);
	m_pSeekTarget = victim;
	if (m_pSeekTarget)
		m_pSeekTarget->RegisterReference((CEntity **) &m_pSeekTarget);

	if (curWeapon->m_bCanAim) {
		CVector aimPos = GetRight() * 0.1f + GetForward() * 0.2f + GetPosition();
		aimPos += GetUp() * 0.35f;
		CEntity *obstacle = CWorld::TestSphereAgainstWorld(aimPos, 0.2f, nil, true, false, false, true, false, false);
		if (obstacle) {
			if(gaTempSphereColPoints[0].surfaceB != SURFACE_TRANSPARENT_CLOTH && gaTempSphereColPoints[0].surfaceB != SURFACE_METAL_CHAIN_FENCE &&
				gaTempSphereColPoints[0].surfaceB != SURFACE_WOOD_BENCH && gaTempSphereColPoints[0].surfaceB != SURFACE_SCAFFOLD_POLE) {
				if (!IsPlayer()) {
					bObstacleShowedUpDuringKillObjective = true;
					m_shootTimer = 0;
					SetAttackTimer(1500);
					m_shotTime = CTimer::GetTimeInMilliseconds();
				}
				return;
			}
		}

		m_pLookTarget = victim;
		if (victim) {
			m_pLookTarget->RegisterReference((CEntity **) &m_pLookTarget);
			m_pSeekTarget->RegisterReference((CEntity **) &m_pSeekTarget);
		}

		if (m_pLookTarget) {
			SetAimFlag(m_pLookTarget);
		} else if (this == FindPlayerPed() && TheCamera.Cams[0].Using3rdPersonMouseCam()) {
			SetAimFlag(m_fRotationCur);
			((CPlayerPed*)this)->m_fFPSMoveHeading = TheCamera.Find3rdPersonQuickAimPitch();
		} else if (curWeapon->m_bCanAimWithArm) {
			SetAimFlag(m_fRotationCur);
		}
	}
	if (m_nPedState == PED_ATTACK) {
		bIsAttacking = true;
		return;
	}

	if (IsPlayer() || (!victimPed || victimPed->IsPedInControl())) {
		if (IsPlayer())
			CPad::GetPad(0)->ResetAverageWeapon();

		PointBlankNecessity pointBlankStatus;
		if ((curWeapon->m_eWeaponFire == WEAPON_FIRE_INSTANT_HIT || GetWeapon()->m_eWeaponType == WEAPONTYPE_FLAMETHROWER)
			&& TheCamera.PlayerWeaponMode.Mode != CCam::MODE_M16_1STPERSON
			&& TheCamera.PlayerWeaponMode.Mode != CCam::MODE_M16_1STPERSON_RUNABOUT
			&& TheCamera.PlayerWeaponMode.Mode != CCam::MODE_SNIPER
			&& TheCamera.PlayerWeaponMode.Mode != CCam::MODE_SNIPER_RUNABOUT
			&& (pointBlankStatus = CheckForPointBlankPeds(victimPed)) != NO_POINT_BLANK_PED) {
			ClearAimFlag();

			// This condition is pointless
			if (pointBlankStatus == POINT_BLANK_FOR_WANTED_PED || !victimPed && (IsPlayer() || !m_carInObjective))
				StartFightAttack(200);
		} else {
			if (!curWeapon->m_bCanAim)
				m_pSeekTarget = nil;

			if (m_nPedState != PED_AIM_GUN)
				SetStoredState();

			SetPedState(PED_ATTACK);
			SetMoveState(PEDMOVE_NONE);
			if (bCrouchWhenShooting && bIsDucking && !!curWeapon->m_bCrouchFire) {
				CAnimBlendAssociation* curMoveAssoc = RpAnimBlendClumpGetAssociation(GetClump(), GetCrouchFireAnim(curWeapon));
				if (curMoveAssoc) {
					if (strcmp(CAnimManager::GetAnimAssociation(curWeapon->m_AnimToPlay, GetCrouchFireAnim(curWeapon))->hierarchy->name, curMoveAssoc->hierarchy->name) != 0) {
						delete curMoveAssoc;
					}
				}
				animAssoc = CAnimManager::BlendAnimation(GetClump(), curWeapon->m_AnimToPlay, GetCrouchFireAnim(curWeapon), 8.0f);
			} else {
				float animDelta = 8.0f;
				if (curWeapon->m_eWeaponFire == WEAPON_FIRE_MELEE)
					animDelta = 1000.0f;

				AnimationId fireAnim;
				if (curWeapon->m_bThrow)
					fireAnim = ANIM_THROWABLE_START_THROW;
				else if (CGame::nastyGame && (curWeapon->m_bGround2nd || curWeapon->m_bGround3rd)) {
					PedOnGroundState pedOnGround = CheckForPedsOnGroundToAttack(this, nil);
					if (pedOnGround > PED_IN_FRONT_OF_ATTACKER || pedOnGround == NO_PED && bIsStanding && m_pCurSurface && m_pCurSurface->IsVehicle()) {
						fireAnim = GetFireAnimGround(curWeapon, false);
					} else {
						fireAnim = GetFireAnimNotDucking(curWeapon);
					}
				} else {
					fireAnim = GetFireAnimNotDucking(curWeapon);
				}

				CAnimBlendAssociation* curFireAssoc = RpAnimBlendClumpGetAssociation(GetClump(), fireAnim);
				if (curFireAssoc) {
					if (strcmp(CAnimManager::GetAnimAssociation(curWeapon->m_AnimToPlay, fireAnim)->hierarchy->name, curFireAssoc->hierarchy->name) != 0) {
						delete curFireAssoc;
					}
				}
				animAssoc = CAnimManager::BlendAnimation(GetClump(), curWeapon->m_AnimToPlay, fireAnim, animDelta);
			}

			animAssoc->SetRun();
			if (animAssoc->currentTime == animAssoc->hierarchy->totalLength)
				animAssoc->SetCurrentTime(0.0f);

			animAssoc->SetFinishCallback(FinishedAttackCB, this);
		}
		return;
	}

	if (GetWeapon()->m_eWeaponType == WEAPONTYPE_BASEBALLBAT && victimPed->m_nPedState == PED_GETUP)
		SetWaitState(WAITSTATE_SURPRISE, nil);

	SetLookFlag(victim, true, true);
	SetLookTimer(100);
}

// --MIAMI: Done
void
CPed::ClearAttack(void)
{
	if (m_nPedState != PED_ATTACK || (bIsDucking && !IsPlayer()) || m_nWaitState == WAITSTATE_PLAYANIM_DUCK)
		return;

	if (FindPlayerPed() == this && TheCamera.Using1stPersonWeaponMode()) {
		SetPointGunAt(nil);
	} else if (bIsPointingGunAt) {
		if (m_pLookTarget)
			SetPointGunAt(m_pLookTarget);
		else
			ClearPointGunAt();
	} else if (m_objective != OBJECTIVE_NONE) {
		SetIdle();
	} else {
		RestorePreviousState();
	}
}

// --MIAMI: Done
void
CPed::ClearAttackByRemovingAnim(void)
{
	if (m_nPedState != PED_ATTACK)
		return;

	CWeaponInfo *weapon = CWeaponInfo::GetWeaponInfo(GetWeapon()->m_eWeaponType);
	CAnimBlendAssociation *weaponAssoc = RpAnimBlendClumpGetAssociation(GetClump(), GetPrimaryFireAnim(weapon));

	if (!weaponAssoc) {
		if (!!weapon->m_bCrouchFire)
			weaponAssoc = RpAnimBlendClumpGetAssociation(GetClump(), GetCrouchFireAnim(weapon));
	}
	if (!weaponAssoc) {
		if(!!weapon->m_bFinish3rd)
			weaponAssoc = RpAnimBlendClumpGetAssociation(GetClump(), GetFinishingAttackAnim(weapon));
	}
	if (!weaponAssoc) {
		if(!!weapon->m_bUse2nd)
			weaponAssoc = RpAnimBlendClumpGetAssociation(GetClump(), GetSecondFireAnim(weapon));
	}
	if (!weaponAssoc) {
		if(!!weapon->m_bCop3rd)
			weaponAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_WEAPON_SPECIAL);
	}
	if (weaponAssoc) {
		weaponAssoc->blendDelta = -8.0f;
		weaponAssoc->flags &= ~ASSOC_RUNNING;
		weaponAssoc->flags |= ASSOC_DELETEFADEDOUT;
		weaponAssoc->SetDeleteCallback(FinishedAttackCB, this);
	} else {
		ClearAttack();
	}
}

// --MIAMI: Done
void
CPed::FinishedAttackCB(CAnimBlendAssociation *attackAssoc, void *arg)
{
	CAnimBlendAssociation *newAnim, *reloadAnimAssoc = nil;
	CPed *ped = (CPed*)arg;
	CWeaponInfo *currentWeapon = CWeaponInfo::GetWeaponInfo(ped->GetWeapon()->m_eWeaponType);

	if (ped->m_nPedState != PED_ATTACK) {
		if (ped->bIsDucking && ped->IsPedInControl()) {
			if (currentWeapon->m_bReload) {
				reloadAnimAssoc = RpAnimBlendClumpGetAssociation(ped->GetClump(), GetCrouchReloadAnim(currentWeapon));
			}
			if (currentWeapon->m_bCrouchFire && attackAssoc) {
				if (attackAssoc->animId == GetCrouchFireAnim(currentWeapon) && !reloadAnimAssoc) {
					newAnim = CAnimManager::BlendAnimation(ped->GetClump(), ASSOCGRP_STD, ANIM_WEAPON_CROUCH, 8.0f);
					newAnim->SetCurrentTime(newAnim->hierarchy->totalLength);
					newAnim->flags &= ~ASSOC_RUNNING;
				}
			}
		}
	} else if (attackAssoc && attackAssoc->animId == ANIM_THROWABLE_START_THROW && currentWeapon->m_AnimToPlay == ASSOCGRP_THROW) {
		if ((!ped->IsPlayer() || ((CPlayerPed*)ped)->m_bHaveTargetSelected) && ped->IsPlayer()) {
			attackAssoc->blendDelta = -1000.0f;
			newAnim = CAnimManager::AddAnimation(ped->GetClump(), currentWeapon->m_AnimToPlay, ANIM_THROWABLE_THROWU);
		} else {
			attackAssoc->blendDelta = -1000.0;
			newAnim = CAnimManager::AddAnimation(ped->GetClump(), currentWeapon->m_AnimToPlay, ANIM_THROWABLE_THROW);
		}
		newAnim->SetFinishCallback(FinishedAttackCB, ped);
	} else if (ped->bIsDucking && ped->bCrouchWhenShooting) {
		if (currentWeapon->m_bReload) {
			reloadAnimAssoc = RpAnimBlendClumpGetAssociation(ped->GetClump(), GetCrouchReloadAnim(currentWeapon));
		}
		if (currentWeapon->m_bCrouchFire && attackAssoc) {
			if (attackAssoc->animId == GetCrouchFireAnim(currentWeapon) && !reloadAnimAssoc) {
				newAnim = CAnimManager::BlendAnimation(ped->GetClump(), ASSOCGRP_STD, ANIM_WEAPON_CROUCH, 8.0f);
				newAnim->SetCurrentTime(newAnim->hierarchy->totalLength);
				newAnim->flags &= ~ASSOC_RUNNING;
			}
		}

		if (!ped->bIsAttacking)
			ped->ClearAttack();

	} else if (GetSecondFireAnim(currentWeapon) && ped->bIsAttacking && currentWeapon->m_AnimToPlay != ASSOCGRP_THROW) {
		AnimationId groundAnim = GetFireAnimGround(currentWeapon);
		CAnimBlendAssociation *groundAnimAssoc = RpAnimBlendClumpGetAssociation(ped->GetClump(), groundAnim);
		if (!(groundAnimAssoc && (groundAnimAssoc->blendAmount > 0.95f || groundAnimAssoc->blendDelta > 0.0f))) {
			if (attackAssoc && attackAssoc->animId == ANIM_MELEE_ATTACK) {
				newAnim = CAnimManager::BlendAnimation(
					ped->GetClump(), currentWeapon->m_AnimToPlay, GetSecondFireAnim(currentWeapon), 8.0f);
			} else {
				newAnim = CAnimManager::BlendAnimation(
					ped->GetClump(), currentWeapon->m_AnimToPlay, ANIM_MELEE_ATTACK, 8.0f);
			}
			newAnim->SetFinishCallback(FinishedAttackCB, ped);
		}
	} else {
		if (attackAssoc && attackAssoc->animId == ANIM_MELEE_ATTACK && currentWeapon->m_AnimToPlay == ASSOCGRP_UNARMED) {
			attackAssoc->blendDelta = -8.0f;
			attackAssoc->flags |= ASSOC_DELETEFADEDOUT;
			ped->ClearAttack();
			return;
		}
		if (attackAssoc) {
			if (currentWeapon->m_AnimToPlay == ASSOCGRP_THROW) {
				if ((attackAssoc->animId == ANIM_THROWABLE_THROW || attackAssoc->animId == ANIM_THROWABLE_THROWU) && ped->GetWeapon()->m_nAmmoTotal > 0) {
					ped->RemoveWeaponModel(currentWeapon->m_nModelId);
					ped->AddWeaponModel(currentWeapon->m_nModelId);
				}
			}
		}

		if (!ped->bIsAttacking)
			ped->ClearAttack();
	}
}

// --MIAMI: Done
void
CPed::FinishedReloadCB(CAnimBlendAssociation *reloadAssoc, void *arg)
{
	CPed *ped = (CPed*)arg;
	CWeaponInfo *weapon = CWeaponInfo::GetWeaponInfo(ped->GetWeapon()->m_eWeaponType);
	
	if (ped->DyingOrDead())
		return;

	if (ped->bIsDucking && ped->bCrouchWhenShooting) {
		CAnimBlendAssociation *crouchFireAssoc = nil;
		if (!!weapon->m_bCrouchFire) {
			crouchFireAssoc = RpAnimBlendClumpGetAssociation(ped->GetClump(), GetCrouchFireAnim(weapon));
		}
		if (!!weapon->m_bReload && reloadAssoc) {
			if (reloadAssoc->animId == GetCrouchReloadAnim(weapon) && !crouchFireAssoc) {
				CAnimBlendAssociation *crouchAssoc = CAnimManager::BlendAnimation(ped->GetClump(), ASSOCGRP_STD, ANIM_WEAPON_CROUCH, 8.0f);
				crouchAssoc->SetCurrentTime(crouchAssoc->hierarchy->totalLength);
				crouchAssoc->flags &= ~ASSOC_RUNNING;
			}
		}
	} else if (weapon->m_bReloadLoop2Start && ped->bIsAttacking) {
		CAnimBlendAssociation *fireAssoc =
			CAnimManager::BlendAnimation(ped->GetClump(), weapon->m_AnimToPlay, GetPrimaryFireAnim(weapon), 8.0f);
		fireAssoc->SetFinishCallback(FinishedAttackCB, ped);
		fireAssoc->SetRun();
		if (fireAssoc->currentTime == reloadAssoc->hierarchy->totalLength)
			fireAssoc->SetCurrentTime(Max(weapon->m_fAnimLoopStart - 0.04f, 0.0f));
		else if (fireAssoc->currentTime < weapon->m_fAnimLoopStart)
			fireAssoc->SetCurrentTime(Max(weapon->m_fAnimLoopStart - 0.04f, 0.0f));
	}
}

// --MIAMI: Done
PointBlankNecessity
CPed::CheckForPointBlankPeds(CPed *pedToVerify)
{
	float pbDistance = 1.1f;
	if (GetWeapon()->IsType2Handed())
		pbDistance = 1.6f;

	for (int i = 0; i < m_numNearPeds; i++)	{
		CPed *nearPed = m_nearPeds[i];

		if (!pedToVerify || pedToVerify == nearPed) {

			CVector diff = nearPed->GetPosition() - GetPosition();
			if (diff.MagnitudeSqr() < SQR(pbDistance)) {

				float neededAngle = CGeneral::GetRadianAngleBetweenPoints(
					nearPed->GetPosition().x, nearPed->GetPosition().y,
					GetPosition().x, GetPosition().y);
				neededAngle = CGeneral::LimitRadianAngle(neededAngle);
				m_fRotationCur = CGeneral::LimitRadianAngle(m_fRotationCur);

				float neededTurn = Abs(neededAngle - m_fRotationCur);

				if (neededTurn > PI)
					neededTurn = 2*PI - neededTurn;

				PedState nearPedState = nearPed->m_nPedState;

				if (nearPedState == PED_FALL || nearPedState == PED_GETUP || nearPedState == PED_DIE || nearPedState == PED_DEAD || nearPedState == PED_DIVE_AWAY)
					return NO_POINT_BLANK_PED;

				if (neededTurn < CAN_SEE_ENTITY_ANGLE_THRESHOLD) {
					if (pedToVerify == nearPed)
						return POINT_BLANK_FOR_WANTED_PED;
					else
						return POINT_BLANK_FOR_SOMEONE_ELSE;
				}
			}
		}
	}
	return NO_POINT_BLANK_PED;
}

// --MIAMI: Done except commented things
void
CPed::Attack(void)
{
	CAnimBlendAssociation *weaponAnimAssoc;
	int32 weaponAnim;
	eWeaponType ourWeaponType;
	float weaponAnimTime;
	eWeaponFire ourWeaponFire;
	float animLoopEnd;
	CWeaponInfo *ourWeapon;
	bool attackShouldContinue;
	CAnimBlendAssociation *reloadAnimAssoc;
	CAnimBlendAssociation *throwAssoc;
	float delayBetweenAnimAndFire;
	float animLoopStart;
	CVector firePos;

	ourWeaponType = GetWeapon()->m_eWeaponType;
	ourWeapon = CWeaponInfo::GetWeaponInfo(ourWeaponType);
	ourWeaponFire = ourWeapon->m_eWeaponFire;
	weaponAnimAssoc = nil;
	attackShouldContinue = bIsAttacking;
	reloadAnimAssoc = nil;
	throwAssoc = nil;
	animLoopStart = ourWeapon->m_fAnimLoopStart;
	animLoopEnd = ourWeapon->m_fAnimLoopEnd;
	delayBetweenAnimAndFire = ourWeapon->m_fAnimFrameFire;
	weaponAnim = ourWeapon->m_AnimToPlay;

	if (bIsDucking) {
		if (!!ourWeapon->m_bCrouchFire && bCrouchWhenShooting) {
			weaponAnimAssoc = RpAnimBlendClumpGetAssociation(GetClump(), GetCrouchFireAnim(ourWeapon));
			if (weaponAnimAssoc) {
				animLoopStart = ourWeapon->m_fAnim2LoopStart;
				animLoopEnd = ourWeapon->m_fAnim2LoopEnd;
				delayBetweenAnimAndFire = ourWeapon->m_fAnim2FrameFire;
			}
		}
	} else {
		AnimationId anim = GetFireAnimNotDucking(ourWeapon);
		weaponAnimAssoc = RpAnimBlendClumpGetAssociation(GetClump(), anim);
		if (anim == ANIM_WEAPON_FIRE_3RD && weaponAnimAssoc) {
			animLoopStart = 11.f/30.f;
			animLoopEnd = 19.f/30.f;
			delayBetweenAnimAndFire = 14.f/30.f;
		}
	}

	if (ourWeapon->m_bReload) {
		reloadAnimAssoc = RpAnimBlendClumpGetAssociation(GetClump(), GetReloadAnim(ourWeapon));
	}

	if (!!ourWeapon->m_bReload && !reloadAnimAssoc)  {
		reloadAnimAssoc = RpAnimBlendClumpGetAssociation(GetClump(), GetCrouchReloadAnim(ourWeapon));
	}

	if ( reloadAnimAssoc && reloadAnimAssoc->IsRunning() ) {
		if (!IsPlayer() || ((CPlayerPed*)this)->m_bHaveTargetSelected)
			ClearAttack();
		return;
	}

	if ( reloadAnimAssoc ) {
		reloadAnimAssoc->flags |= ASSOC_DELETEFADEDOUT;
		if ( reloadAnimAssoc->blendDelta >= 0.0f )
			reloadAnimAssoc->blendDelta = -8.0f;
	}

	if (!!ourWeapon->m_bThrow)  {
		throwAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_THROWABLE_START_THROW);
	}

	if ( CTimer::GetTimeInMilliseconds() < m_shootTimer )
		attackShouldContinue = true;

	bool meleeAttackStarted = false;
	if ( !weaponAnimAssoc ) {
		if (!!ourWeapon->m_bPartialAttack) {
			weaponAnimAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_MELEE_ATTACK_START);
			if ( weaponAnimAssoc ) {
				if ( IsPlayer() )
					meleeAttackStarted = true;

				switch ( ourWeapon->m_AnimToPlay ) {
					case ASSOCGRP_UNARMED:
					case ASSOCGRP_SCREWDRIVER:
					case ASSOCGRP_KNIFE:
					case ASSOCGRP_BASEBALLBAT:
					case ASSOCGRP_GOLFCLUB:
					case ASSOCGRP_CHAINSAW:
						delayBetweenAnimAndFire = 0.2f;
						animLoopStart = 0.1f;
						break;
					default:
						break;
				}
				animLoopEnd = 99.9f;
			}
		}
	}
	if (!weaponAnimAssoc) {
		if (!!ourWeapon->m_bUse2nd) {
			weaponAnimAssoc = RpAnimBlendClumpGetAssociation(GetClump(), GetSecondFireAnim(ourWeapon));
			if (weaponAnimAssoc) {
				animLoopStart = ourWeapon->m_fAnim2LoopStart;
				animLoopEnd = ourWeapon->m_fAnim2LoopEnd;
				delayBetweenAnimAndFire = ourWeapon->m_fAnim2FrameFire;
			}
		}
	}
	if (!weaponAnimAssoc) {
		weaponAnimAssoc = RpAnimBlendClumpGetAssociation(GetClump(), GetFireAnimGround(ourWeapon));
		if (weaponAnimAssoc) {
			animLoopStart = ourWeapon->m_fAnim2LoopStart;
			animLoopEnd = ourWeapon->m_fAnim2LoopEnd;
			delayBetweenAnimAndFire = ourWeapon->m_fAnim2FrameFire;
		}
	}

	if (!weaponAnimAssoc) {
		if (!throwAssoc) {
			if (attackShouldContinue) {
				if (ourWeaponFire != WEAPON_FIRE_PROJECTILE || !IsPlayer() || ((CPlayerPed*)this)->m_bHaveTargetSelected) {
					if (bCrouchWhenShooting && bIsDucking && !!ourWeapon->m_bCrouchFire) {
						weaponAnimAssoc = CAnimManager::BlendAnimation(GetClump(), ourWeapon->m_AnimToPlay, GetCrouchFireAnim(ourWeapon), 8.0f);

					} else if(GetSecondFireAnim(ourWeapon) && CGeneral::GetRandomNumber() & 1){
						weaponAnimAssoc = CAnimManager::BlendAnimation(GetClump(), ourWeapon->m_AnimToPlay, GetSecondFireAnim(ourWeapon), 8.0f);

					} else if (!CGame::nastyGame || (!ourWeapon->m_bGround2nd && !ourWeapon->m_bGround3rd) ||
						ourWeaponFire != WEAPON_FIRE_MELEE || CheckForPedsOnGroundToAttack(this, nil) < PED_ON_THE_FLOOR) {

						weaponAnimAssoc = CAnimManager::BlendAnimation(GetClump(), ourWeapon->m_AnimToPlay, GetFireAnimNotDucking(ourWeapon), 8.0f);

					} else {
						weaponAnimAssoc = CAnimManager::BlendAnimation(GetClump(), ourWeapon->m_AnimToPlay, GetFireAnimGround(ourWeapon, false), 8.0f);
					}

					weaponAnimAssoc->SetFinishCallback(FinishedAttackCB, this);
					weaponAnimAssoc->SetRun();

					if (weaponAnimAssoc->currentTime == weaponAnimAssoc->hierarchy->totalLength)
						weaponAnimAssoc->SetCurrentTime(0.0f);

					if (IsPlayer()) {
						((CPlayerPed*)this)->m_fAttackButtonCounter = 0.0f;
						((CPlayerPed*)this)->m_bHaveTargetSelected = false;
					}
				}
			} else
				FinishedAttackCB(nil, this);

		}
		return;
	}

	if (meleeAttackStarted && IsPlayer()) {
		if (((CPlayerPed*)this)->m_bHaveTargetSelected || ((CPlayerPed*)this)->m_fMoveSpeed < 0.5f) {
			weaponAnimAssoc->SetRun();
		} else {
			if (weaponAnimAssoc->currentTime > animLoopStart && weaponAnimAssoc->currentTime - weaponAnimAssoc->timeStep <= animLoopStart)
				weaponAnimAssoc->flags &= ~ASSOC_RUNNING;
		}
	}

	float animStart = ourWeapon->m_fAnimLoopStart * 0.4f;
	weaponAnimTime = weaponAnimAssoc->currentTime;
	if (weaponAnimTime > animStart && weaponAnimTime - weaponAnimAssoc->timeStep <= animStart) {
		if (!bIsDucking && !(m_nPedType == PEDTYPE_COP && ourWeapon->m_bCop3rd && weaponAnimAssoc->animId == ANIM_WEAPON_FIRE_3RD) && ourWeapon->m_bCanAimWithArm)
			m_pedIK.m_flags |= CPedIK::AIMS_WITH_ARM;
		else
			m_pedIK.m_flags &= ~CPedIK::AIMS_WITH_ARM;
	}

	if (ourWeaponType != WEAPONTYPE_CHAINSAW
		|| !meleeAttackStarted && delayBetweenAnimAndFire - 0.5f >= weaponAnimAssoc->currentTime
		|| weaponAnimAssoc->currentTime - weaponAnimAssoc->timeStep > delayBetweenAnimAndFire) {

		if (ourWeaponType == WEAPONTYPE_CHAINSAW) {
			DMAudio.PlayOneShot(m_audioEntityId, SOUND_WEAPON_CHAINSAW_ATTACK, 0.0f);
		} else if (weaponAnimTime <= delayBetweenAnimAndFire || weaponAnimTime - weaponAnimAssoc->timeStep > delayBetweenAnimAndFire || !weaponAnimAssoc->IsRunning()) {
			if (weaponAnimAssoc->speed < 1.0f)
				weaponAnimAssoc->speed = 1.0f;

		} else {
			firePos = ourWeapon->m_vecFireOffset;

			if (ourWeaponType != WEAPONTYPE_KATANA && ourWeaponType != WEAPONTYPE_CHAINSAW) {
				if (ourWeapon->m_eWeaponFire != WEAPON_FIRE_MELEE) {
					TransformToNode(firePos, (weaponAnimAssoc->animId == ANIM_MELEE_ATTACK_2ND && ourWeapon->m_AnimToPlay == ASSOCGRP_UNARMED) ? PED_FOOTR : PED_HANDR);
				} else {
					firePos = GetMatrix() * firePos;
				}
			} else {
				if (weaponAnimAssoc->animId == ANIM_MELEE_ATTACK_2ND)
					firePos.z = 0.7f * ourWeapon->m_fRadius - 1.0f;

				firePos = GetMatrix() * firePos;
			}
			
			GetWeapon()->Fire(this, &firePos);

			if (ourWeaponType == WEAPONTYPE_MOLOTOV || ourWeaponType == WEAPONTYPE_GRENADE || ourWeaponType == WEAPONTYPE_DETONATOR_GRENADE ||
				ourWeaponType == WEAPONTYPE_TEARGAS) {
				RemoveWeaponModel(ourWeapon->m_nModelId);
			}
			if (!GetWeapon()->m_nAmmoTotal && ourWeaponFire != WEAPON_FIRE_MELEE && FindPlayerPed() != this) {
				SelectGunIfArmed();
			}

			if (GetWeapon()->m_eWeaponState == WEAPONSTATE_MELEE_MADECONTACT) {
				int damagerType = ENTITY_TYPE_NOTHING;
				if (m_pDamageEntity && (m_fDamageImpulse == 0.0f || !m_pDamageEntity->IsBuilding())) {
					damagerType = m_pDamageEntity->GetType();
				}
				switch (ourWeapon->m_AnimToPlay) {
					case ASSOCGRP_UNARMED:
						if (weaponAnimAssoc->animId == ANIM_MELEE_ATTACK || weaponAnimAssoc->animId == ANIM_MELEE_ATTACK_START) 
							DMAudio.PlayOneShot(m_audioEntityId, SOUND_FIGHT_46, (damagerType | (ourWeaponType << 8)));
						break;
					case ASSOCGRP_KNIFE:
					case ASSOCGRP_BASEBALLBAT:
					case ASSOCGRP_GOLFCLUB:
					case ASSOCGRP_CHAINSAW:
						DMAudio.PlayOneShot(m_audioEntityId, SOUND_WEAPON_BAT_ATTACK, (damagerType | (ourWeaponType << 8)));
						break;
					default:
						break;
				}

				if (bIsAttacking || CTimer::GetTimeInMilliseconds() < m_shootTimer) {
					weaponAnimAssoc->callbackType = 0;
				}
			}

			attackShouldContinue = false;
		}
	} else {
		CVector firePos = ourWeapon->m_vecFireOffset;

		if (weaponAnimAssoc->animId == ANIM_MELEE_ATTACK_2ND)
			firePos.z = 0.7f * ourWeapon->m_fRadius - 1.0f;

		firePos = GetMatrix() * firePos;
		GetWeapon()->Fire(this, &firePos);
		if (GetWeapon()->m_eWeaponState == WEAPONSTATE_MELEE_MADECONTACT) {
			int damagerType = ENTITY_TYPE_PED;
			if (m_pDamageEntity)
				damagerType = m_pDamageEntity->GetType();

			DMAudio.PlayOneShot(m_audioEntityId, SOUND_WEAPON_CHAINSAW_MADECONTACT, (float)damagerType);
			if (IsPlayer()) {
				CPad::GetPad(0)->StartShake(240, 180);
			}
		} else {
			DMAudio.PlayOneShot(m_audioEntityId, SOUND_WEAPON_CHAINSAW_IDLE, 0.0f);
			if (IsPlayer()) {
				CPad::GetPad(0)->StartShake(240, 90);
			}
		}
		attackShouldContinue = false;
	}

	if (ourWeapon->m_eWeaponFire == WEAPON_FIRE_INSTANT_HIT && ourWeapon->m_AnimToPlay == ASSOCGRP_SHOTGUN) {
		weaponAnimTime = weaponAnimAssoc->currentTime;
		firePos = ourWeapon->m_vecFireOffset;

		if (weaponAnimTime > 1.0f && weaponAnimTime - weaponAnimAssoc->timeStep <= 1.0f && weaponAnimAssoc->IsRunning()) {
			TransformToNode(firePos, PED_HANDR);

			CVector gunshellPos(
				firePos.x - 0.6f * GetForward().x,
				firePos.y - 0.6f * GetForward().y,
				firePos.z - 0.15f * GetUp().z
			);

			CVector2D gunshellRot(
				GetRight().x,
				GetRight().y
			);

			gunshellRot.Normalise();
			GetWeapon()->AddGunshell(this, gunshellPos, gunshellRot, 0.025f);
		}
	}

	// TODO(Miami): CSpecialFX::AddWeaponStreak

	// Anim breakout on running
	if (IsPlayer()) {
		if (CPad::GetPad(0)->GetSprint()) {
			if (!attackShouldContinue && weaponAnimAssoc->currentTime > ourWeapon->m_fAnimBreakout) {
				weaponAnimAssoc->blendDelta = -4.0f;
				FinishedAttackCB(nil, this);
				return;
			}
		}
	}

	weaponAnimTime = weaponAnimAssoc->currentTime;

	// Anim loop end, either start the loop again or finish the attack
	if (weaponAnimTime > animLoopEnd || !weaponAnimAssoc->IsRunning() && ourWeaponFire != WEAPON_FIRE_PROJECTILE) {
		if (GetWeapon()->m_eWeaponState == WEAPONSTATE_RELOADING) {
			if (ourWeapon->m_bReload && !reloadAnimAssoc) {
				if (!CWorld::Players[CWorld::PlayerInFocus].m_bFastReload) {
					CAnimBlendAssociation *newReloadAssoc;
					if (bIsDucking) {
						newReloadAssoc = CAnimManager::BlendAnimation(
							GetClump(), ourWeapon->m_AnimToPlay,
							GetCrouchReloadAnim(ourWeapon),
							8.0f);
					} else {
						newReloadAssoc = CAnimManager::BlendAnimation(
							GetClump(), ourWeapon->m_AnimToPlay,
							GetReloadAnim(ourWeapon),
							8.0f);
					}
					newReloadAssoc->SetFinishCallback(FinishedReloadCB, this);
				}
				ClearLookFlag();
				ClearAimFlag();
				bIsAttacking = false;
				bIsPointingGunAt = false;
				m_shootTimer = CTimer::GetTimeInMilliseconds();
				DMAudio.PlayOneShot(m_audioEntityId, SOUND_WEAPON_AK47_BULLET_ECHO, GetWeapon()->m_eWeaponType);
				return;
			}
		}
		if (weaponAnimTime - 2.0f * weaponAnimAssoc->timeStep <= animLoopEnd
			&& (bIsAttacking || CTimer::GetTimeInMilliseconds() < m_shootTimer)
			&& (GetWeapon()->m_eWeaponState != WEAPONSTATE_RELOADING
				|| GetWeapon()->m_eWeaponType == WEAPONTYPE_MINIGUN)) {

			PedOnGroundState pedOnGroundState;
			if (ourWeapon->m_eWeaponFire == WEAPON_FIRE_MELEE &&
				(CGame::nastyGame && ((pedOnGroundState = CheckForPedsOnGroundToAttack(this, nil)) > PED_IN_FRONT_OF_ATTACKER)
				|| ourWeaponType == WEAPONTYPE_BASEBALLBAT && pedOnGroundState == NO_PED && bIsStanding && m_pCurSurface && m_pCurSurface->IsVehicle())) {

				AnimationId fireAnim = GetFireAnimGround(ourWeapon, false);
				if (weaponAnimAssoc->animId == fireAnim)
					weaponAnimAssoc->SetCurrentTime(0.1f);
				else {
					if (fireAnim) {
						weaponAnimAssoc = CAnimManager::BlendAnimation(GetClump(), ourWeapon->m_AnimToPlay, fireAnim, 8.0f);
					} else {
						weaponAnimAssoc = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_KICK_FLOOR, 8.0f);
					}
				}
				weaponAnimAssoc->SetFinishCallback(FinishedAttackCB, this);
			} else if (!!ourWeapon->m_bUse2nd) {
				if (weaponAnimAssoc->animId == GetSecondFireAnim(ourWeapon)) {
					weaponAnimAssoc = CAnimManager::BlendAnimation(GetClump(), ourWeapon->m_AnimToPlay, ANIM_WEAPON_FIRE, 8.0f);
				} else {
					weaponAnimAssoc = CAnimManager::BlendAnimation(GetClump(), ourWeapon->m_AnimToPlay, GetSecondFireAnim(ourWeapon), 8.0f);
				}
				weaponAnimAssoc->SetFinishCallback(FinishedAttackCB, this);
			} else {
				weaponAnimAssoc->SetCurrentTime(animLoopStart);
				weaponAnimAssoc->SetRun();
			}
		}
	} else if (IsPlayer() && m_pPointGunAt && bIsAimingGun && GetWeapon()->m_eWeaponState != WEAPONSTATE_RELOADING) {
		weaponAnimAssoc->SetCurrentTime(animLoopEnd);
		weaponAnimAssoc->flags &= ~ASSOC_RUNNING;
		SetPointGunAt(m_pPointGunAt);
	} else {
		ClearAimFlag();

		// Echoes of bullets, at the end of the attack. (Bug: doesn't play while reloading)
		if (weaponAnimAssoc->currentTime - weaponAnimAssoc->timeStep < animLoopEnd) 
			DMAudio.PlayOneShot(m_audioEntityId, SOUND_WEAPON_AK47_BULLET_ECHO, ourWeaponType);

		// Fun fact: removing this part leds to reloading flamethrower
		if (ourWeaponType == WEAPONTYPE_FLAMETHROWER && weaponAnimAssoc->IsRunning()) {
			weaponAnimAssoc->flags |= ASSOC_DELETEFADEDOUT;
			weaponAnimAssoc->flags &= ~ASSOC_RUNNING;
			weaponAnimAssoc->blendDelta = -4.0f;
		}
	}

	if (weaponAnimAssoc->currentTime > delayBetweenAnimAndFire)
		attackShouldContinue = false;

	bIsAttacking = attackShouldContinue;
}

// --MIAMI: Done
void
CPed::StartFightAttack(uint8 buttonPressure)
{
	if (!IsPedInControl() || (m_attackTimer > CTimer::GetTimeInMilliseconds() && buttonPressure != 0))
		return;

	if (m_nPedState == PED_FIGHT) {
		m_fightButtonPressure = buttonPressure;
		return;
	}

	if (m_nPedState != PED_AIM_GUN)
		SetStoredState();

	if (m_nWaitState != WAITSTATE_FALSE) {
		ClearWaitState();
		RestoreHeadingRate();
	}

	CAnimBlendAssociation* animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_RUN_STOP);
	if (!animAssoc)
		animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_RUN_STOP_R);

	if (animAssoc) {
		RestoreHeadingRate();
	}
	SetMoveState(PEDMOVE_NONE);
	m_nStoredMoveState = PEDMOVE_NONE;
	bool fightWithWeapon = false;
	CAnimBlendAssociation *fightIdleAssoc;

	CWeaponInfo* weaponInfo = CWeaponInfo::GetWeaponInfo(GetWeapon()->m_eWeaponType);
	if (GetWeapon()->m_eWeaponType != WEAPONTYPE_UNARMED) {
		if (GetFightIdleWithMeleeAnim(weaponInfo)) {
			fightIdleAssoc = CAnimManager::BlendAnimation(GetClump(), weaponInfo->m_AnimToPlay, GetFightIdleWithMeleeAnim(weaponInfo), 1000.0f);
			fightWithWeapon = true;
		} else {
			fightIdleAssoc = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_FIGHT_IDLE, 1000.0f);
		}
	} else {
		fightIdleAssoc = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_FIGHT_IDLE, 1000.0f);
	}
	m_lastFightMove = FIGHTMOVE_IDLE;
	m_curFightMove = IsPlayer() ? ChooseAttackPlayer(buttonPressure, fightWithWeapon) : ChooseAttackAI(buttonPressure, fightWithWeapon);

	SetPedState(PED_FIGHT);
	m_fightButtonPressure = 0;

	if (m_curFightMove > FIGHTMOVE_NULL && m_curFightMove != FIGHTMOVE_IDLE) {
		animAssoc = CAnimManager::BlendAnimation(GetClump(), m_curFightMove < FIGHTMOVE_MELEE1 ? ASSOCGRP_STD : weaponInfo->m_AnimToPlay,
			tFightMoves[m_curFightMove].animId, 8.0f);

		if (weaponInfo->m_AnimToPlay == ASSOCGRP_KNIFE && m_curFightMove >= FIGHTMOVE_MELEE1) {
			switch (GetWeapon()->m_eWeaponType) {
				case WEAPONTYPE_SCREWDRIVER:
				case WEAPONTYPE_KNIFE:
					animAssoc->speed = 1.05f;
					break;
				case WEAPONTYPE_GOLFCLUB:
				case WEAPONTYPE_NIGHTSTICK:
				case WEAPONTYPE_BASEBALLBAT:
				case WEAPONTYPE_HAMMER:
				case WEAPONTYPE_KATANA:
					animAssoc->speed = 0.8f;
					break;
				case WEAPONTYPE_CLEAVER:
				case WEAPONTYPE_MACHETE:
					animAssoc->speed = 0.9f;
					break;
			}
		} else {
			if (m_curFightMove == FIGHTMOVE_BACKKICK)
				animAssoc->speed = 1.15f;
			else
				animAssoc->speed = 0.8f;
		}
		if (IsPlayer())
			animAssoc->SetCurrentTime(0.08f);

		animAssoc->SetFinishCallback(FinishFightMoveCB, this);
	} else {
		m_nWaitTimer = CTimer::GetTimeInMilliseconds() + 2000;
	}

	m_fightState = FIGHTSTATE_NO_MOVE;
	m_takeAStepAfterAttack = false;
	bIsAttacking = true;

	if (IsPlayer())
		nPlayerInComboMove = 0;
}

// --MIAMI: Done
void
CPed::StartFightDefend(uint8 direction, uint8 hitLevel, uint8 unk)
{
	if (m_nPedState == PED_DEAD) {
		if (CGame::nastyGame) {
			if (hitLevel == HITLEVEL_GROUND) {
				CAnimBlendAssociation *floorHitAssoc;
				if (RpAnimBlendClumpGetFirstAssociation(GetClump(), ASSOC_FRONTAL)) {
					floorHitAssoc = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_FLOOR_HIT_F, 8.0f);
				} else {
					floorHitAssoc = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, tFightMoves[FIGHTMOVE_HITONFLOOR].animId, 8.0f);
				}
				if (floorHitAssoc) {
					floorHitAssoc->SetCurrentTime(0.0f);
					floorHitAssoc->SetRun();
					floorHitAssoc->flags &= ~ASSOC_FADEOUTWHENDONE;
				}
			}
			if (CGame::nastyGame) {
				CVector headPos = GetNodePosition(PED_HEAD);
				for(int i = 0; i < 4; ++i) {
					CVector bloodDir(0.0f, 0.0f, 0.1f);
					CVector bloodPos = headPos - 0.2f * GetForward();
					CParticle::AddParticle(PARTICLE_BLOOD, bloodPos, bloodDir, nil, 0.0f, 0, 0, 0, 0);
				}
			}
		}
	} else if (m_nPedState == PED_FALL) {
		if (hitLevel == HITLEVEL_GROUND && !IsPedHeadAbovePos(-0.3f)) {
			CAnimBlendAssociation *floorHitAssoc = RpAnimBlendClumpGetFirstAssociation(GetClump(), ASSOC_FRONTAL) ?
				CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_FLOOR_HIT_F, 8.0f) :
				CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_FLOOR_HIT, 8.0f);
			if (floorHitAssoc) {
				floorHitAssoc->flags &= ~ASSOC_FADEOUTWHENDONE;
				floorHitAssoc->flags |= ASSOC_DELETEFADEDOUT;
			}
		}
	} else if (IsPedInControl()) {
		if ((IsPlayer() && m_nPedState != PED_FIGHT && ((CPlayerPed*)this)->m_fMoveSpeed > 1.0f)
			|| (!IsPlayer() && m_objective == OBJECTIVE_FLEE_CHAR_ON_FOOT_TILL_SAFE)) {

			if (hitLevel != HITLEVEL_HIGH && hitLevel != HITLEVEL_LOW || (IsPlayer() || CGeneral::GetRandomNumber() & 1) && CGeneral::GetRandomNumber() & 7) {
				if (IsPlayer() || CGeneral::GetRandomNumber() & 1) {
					AnimationId shotAnim;
					switch (direction) {
						case 1:
							shotAnim = ANIM_SHOT_LEFT_PARTIAL;
							break;
						case 2:
							shotAnim = ANIM_SHOT_BACK_PARTIAL;
							break;
						case 3:
							shotAnim = ANIM_SHOT_RIGHT_PARTIAL;
							break;
						default:
							shotAnim = ANIM_SHOT_FRONT_PARTIAL;
							break;
					}
					CAnimBlendAssociation *shotAssoc = RpAnimBlendClumpGetAssociation(GetClump(), shotAnim);
					if (!shotAssoc || shotAssoc->blendDelta < 0.0f)
						shotAssoc = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, shotAnim, 8.0f);

					shotAssoc->SetCurrentTime(0.0f);
					shotAssoc->SetRun();
					shotAssoc->flags |= ASSOC_FADEOUTWHENDONE;
				} else {
					int time = CGeneral::GetRandomNumberInRange(1000, 3000);
					SetWaitState(WAITSTATE_PLAYANIM_DUCK, &time);
				}
			} else {
				bool fall = true;
				AnimationId hitAnim;
				switch (direction) {
					case 1:
						hitAnim = ANIM_KO_SPIN_R;
						break;
					case 2:
						if (CGeneral::GetRandomNumber() & 1) {
							fall = false;
							hitAnim = ANIM_HIT_BACK;
						} else	{
							hitAnim = ANIM_KO_SKID_BACK;
						}
						break;
					case 3:
						hitAnim = ANIM_KO_SPIN_L;
						break;
					default:
						if (hitLevel == HITLEVEL_LOW) {
							hitAnim = ANIM_KO_SHOT_STOM;
						} else if (CGeneral::GetRandomNumber() & 1) {
							fall = false;
							hitAnim = ANIM_HIT_WALK;
						} else if (CGeneral::GetRandomNumber() & 1) {
							fall = false;
							hitAnim = ANIM_HIT_HEAD;
						} else {
							hitAnim = ANIM_KO_SHOT_FACE;
						}
						break;
				}
				if (fall) {
					SetFall(500, hitAnim, false);
				} else {
					CAnimBlendAssociation *hitAssoc = RpAnimBlendClumpGetAssociation(GetClump(), hitAnim);
					if (!hitAssoc || hitAssoc->blendDelta < 0.0f)
						hitAssoc = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, hitAnim, 8.0f);

					hitAssoc->SetCurrentTime(0.0f);
					hitAssoc->SetRun();
					hitAssoc->flags |= ASSOC_FADEOUTWHENDONE;
				}
			}
			Say(SOUND_PED_DEFEND);
		} else {
			Say(SOUND_PED_DEFEND);
			switch (hitLevel) {
				case HITLEVEL_GROUND:
					m_curFightMove = FIGHTMOVE_HITONFLOOR;
					break;
				case HITLEVEL_LOW:
					if (direction == 2 && (!IsPlayer() || ((CGeneral::GetRandomNumber() & 1) && m_fHealth < 30.0f))) {
						SetFall(1000, ANIM_KO_SKID_BACK, false);
						Say(SOUND_PED_DEFEND);
						return;
					} else if (direction != 2 && !IsPlayer() && (CGeneral::GetRandomNumber() & 1) && m_fHealth < 30.0f) {
						SetFall(1000, ANIM_KO_SHOT_STOM, false);
						Say(SOUND_PED_DEFEND);
						return;
					}	
					m_curFightMove = FIGHTMOVE_HITBODY;
					break;
				case HITLEVEL_HIGH:
					switch (direction) {
						case 1:
							m_curFightMove = FIGHTMOVE_HITLEFT;
							break;
						case 2:
							m_curFightMove = FIGHTMOVE_HITBACK;
							break;
						case 3:
							m_curFightMove = FIGHTMOVE_HITRIGHT;
							break;
						default:
							if (unk <= 5)
								m_curFightMove = FIGHTMOVE_HITHEAD;
							else
								m_curFightMove = FIGHTMOVE_HITBIGSTEP;
							break;
					}
					break;
				default:
					switch (direction) {
						case 1:
							m_curFightMove = FIGHTMOVE_HITLEFT;
							break;
						case 2:
							m_curFightMove = FIGHTMOVE_HITBACK;
							break;
						case 3:
							m_curFightMove = FIGHTMOVE_HITRIGHT;
							break;
						default:
							if (unk <= 5)
								m_curFightMove = FIGHTMOVE_HITCHEST;
							else
								m_curFightMove = FIGHTMOVE_HITBIGSTEP;
							break;
					}
					break;
			}
			if (m_nPedState == PED_GETUP && !IsPedHeadAbovePos(0.0f))
				m_curFightMove = FIGHTMOVE_HITONFLOOR;

			if (m_nPedState == PED_FIGHT) {
				CAnimBlendAssociation *moveAssoc = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, tFightMoves[m_curFightMove].animId, 8.0f);
				moveAssoc->SetCurrentTime(0.0f);
				moveAssoc->SetFinishCallback(FinishFightMoveCB, this);
				if (IsPlayer())
					moveAssoc->speed = 1.2f;

				m_takeAStepAfterAttack = 0;
				m_fightButtonPressure = 0;

			} else if (IsPlayer() && GetWeapon()->m_eWeaponType != WEAPONTYPE_UNARMED && GetWeapon()->m_eWeaponType != WEAPONTYPE_BRASSKNUCKLE &&
					!CWeaponInfo::GetWeaponInfo(GetWeapon()->m_eWeaponType)->m_bFightMode) {
				CAnimBlendAssociation *moveAssoc = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, tFightMoves[m_curFightMove].animId, 4.0f);
				moveAssoc->SetCurrentTime(0.0f);
				moveAssoc->speed = 1.2f;

			} else {
				if (m_nPedState != PED_AIM_GUN && m_nPedState != PED_ATTACK)
					SetStoredState();

				if (m_nWaitState != WAITSTATE_FALSE) {
					ClearWaitState();
					RestoreHeadingRate();
				}
				SetPedState(PED_FIGHT);
				m_fightButtonPressure = 0;
				m_lastFightMove = FIGHTMOVE_IDLE;
				RpAnimBlendClumpRemoveAssociations(GetClump(), ASSOC_REPEAT);
				CAnimBlendAssociation *walkStartAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_WALK_START);
				if (walkStartAssoc) {
					walkStartAssoc->flags |= ASSOC_DELETEFADEDOUT;
					walkStartAssoc->blendDelta = -1000.0f;
				}
				CAnimBlendAssociation *walkStopAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_RUN_STOP);
				if (!walkStopAssoc)
					walkStopAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_RUN_STOP_R);
				if (walkStopAssoc) {
					walkStopAssoc->flags |= ASSOC_DELETEFADEDOUT;
					walkStopAssoc->blendDelta = -1000.0f;
					RestoreHeadingRate();
				}
				SetMoveState(PEDMOVE_NONE);
				m_nStoredMoveState = PEDMOVE_NONE;
				CAnimBlendAssociation *fightIdleAssoc;

				if (GetWeapon()->m_eWeaponType != WEAPONTYPE_UNARMED) {
					CWeaponInfo *weaponInfo = CWeaponInfo::GetWeaponInfo(GetWeapon()->m_eWeaponType);
					if (GetFightIdleWithMeleeAnim(weaponInfo)) {
						fightIdleAssoc = CAnimManager::AddAnimation(GetClump(), weaponInfo->m_AnimToPlay, GetFightIdleWithMeleeAnim(weaponInfo));
					} else {
						fightIdleAssoc = CAnimManager::AddAnimation(GetClump(), ASSOCGRP_STD, ANIM_FIGHT_IDLE);
					}
				} else {
					fightIdleAssoc = CAnimManager::AddAnimation(GetClump(), ASSOCGRP_STD, ANIM_FIGHT_IDLE);
				}
				fightIdleAssoc->blendAmount = 1.0f;
				CAnimBlendAssociation *moveAssoc = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, tFightMoves[m_curFightMove].animId, 8.0f);
				moveAssoc->SetFinishCallback(FinishFightMoveCB, this);
				m_fightState = FIGHTSTATE_NO_MOVE;
				m_takeAStepAfterAttack = false;
				bIsAttacking = true;
			}

			if (m_pedInObjective && m_pedInObjective->IsPlayer() && !IsPlayer())
				((CPlayerPed*)m_pedInObjective)->RemovePedFromMeleeList(this);
		}
	}
}

// --MIAMI: Done
void
CPed::Fight(void)
{
	CAnimBlendAssociation *currentAssoc, *animAssoc;
	bool fightWithWeapon = false;

	eWeaponType weapon = GetWeapon()->m_eWeaponType;
	CWeaponInfo *weaponInfo = CWeaponInfo::GetWeaponInfo(weapon);

	if (weaponInfo->m_bFightMode && weapon != WEAPONTYPE_UNARMED) {
		fightWithWeapon = true;
		tFightMoves[FIGHTMOVE_MELEE1].startFireTime = weaponInfo->m_fAnimFrameFire;
		tFightMoves[FIGHTMOVE_MELEE1].endFireTime = weaponInfo->m_fAnimLoopEnd;
		tFightMoves[FIGHTMOVE_MELEE2].startFireTime = weaponInfo->m_fAnim2FrameFire;
		tFightMoves[FIGHTMOVE_MELEE2].endFireTime = weaponInfo->m_fAnim2LoopEnd;
		tFightMoves[FIGHTMOVE_MELEE3].startFireTime = weaponInfo->m_fAnim2FrameFire;
		tFightMoves[FIGHTMOVE_MELEE3].endFireTime = weaponInfo->m_fAnim2LoopEnd;
	}

	switch (m_curFightMove) {
		case FIGHTMOVE_NULL:
			return;
		case FIGHTMOVE_IDLE2NORM:
			m_curFightMove = FIGHTMOVE_NULL;
			RestorePreviousState();

			// FIX: Uninitialized
			currentAssoc = nil;
			break;
		case FIGHTMOVE_IDLE:
			currentAssoc = nil;
			break;
		default:
			currentAssoc = RpAnimBlendClumpGetAssociation(GetClump(), tFightMoves[m_curFightMove].animId);
			break;
	}

	if (m_curFightMove == FIGHTMOVE_SHUFFLE_F && !currentAssoc)
		currentAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_FIGHT_SH_BACK);

	if (IsPlayer() && currentAssoc && weapon == WEAPONTYPE_KATANA) {
		if (m_curFightMove == FIGHTMOVE_MELEE1 || m_curFightMove == FIGHTMOVE_MELEE2) {
			static float streakDelay = 0.2f;

			if (tFightMoves[m_curFightMove].startFireTime - streakDelay < currentAssoc->currentTime &&
				streakDelay + tFightMoves[m_curFightMove].endFireTime > currentAssoc->currentTime) {

				// TODO(Miami): AddWeaponStreak
				// CSpecialFX::AddWeaponStreak(v2->m_weapons[(char)v2->m_currentWeapon].nWeaponId);
			}
		}
	}

	if (!bIsAttacking && IsPlayer()) {
		if (currentAssoc) {
			currentAssoc->blendDelta = -1000.0f;
			currentAssoc->flags |= ASSOC_DELETEFADEDOUT;
			currentAssoc->flags &= ~ASSOC_RUNNING;
		}
		if (m_takeAStepAfterAttack)
			EndFight(ENDFIGHT_WITH_A_STEP);
		else
			EndFight(ENDFIGHT_FAST);

	} else if (currentAssoc && m_fightState > FIGHTSTATE_MOVE_FINISHED) {
		float animTime = currentAssoc->currentTime;
		FightMove &curMove = tFightMoves[m_curFightMove];
		if (curMove.hitLevel != HITLEVEL_NULL && animTime > curMove.startFireTime && animTime <= curMove.endFireTime && m_fightState >= FIGHTSTATE_NO_MOVE) {

			if (animTime > curMove.startFireTime && animTime - currentAssoc->timeStep < curMove.startFireTime &&
				(IsPlayer() || weapon != WEAPONTYPE_UNARMED)) {

				DMAudio.PlayOneShot(m_audioEntityId, SOUND_MELEE_ATTACK_START, weapon << 8);
			}

			CVector touchingNodePos(0.0f, 0.0f, 0.0f);

			switch (m_curFightMove) {
				case FIGHTMOVE_KNEE:
					TransformToNode(touchingNodePos, PED_LOWERLEGR);
					break;
				case FIGHTMOVE_PUNCHHOOK:
				case FIGHTMOVE_PUNCHJAB:
					TransformToNode(touchingNodePos, PED_HANDL);
					break;
				case FIGHTMOVE_LONGKICK:
				case FIGHTMOVE_ROUNDHOUSE:
				case FIGHTMOVE_FWDLEFT:
				case FIGHTMOVE_BACKRIGHT:
				case FIGHTMOVE_GROUNDKICK:
					TransformToNode(touchingNodePos, PED_FOOTR);
					break;
				case FIGHTMOVE_FWDRIGHT:
					TransformToNode(touchingNodePos, PED_HEAD);
					break;
				case FIGHTMOVE_BACKKICK:
				case FIGHTMOVE_BACKFLIP:
					TransformToNode(touchingNodePos, PED_FOOTL);
					break;
				case FIGHTMOVE_BACKLEFT:
					TransformToNode(touchingNodePos, PED_UPPERARML);
					break;
				default:
					TransformToNode(touchingNodePos, PED_HANDR);
					break;
			}

			FightStrike(touchingNodePos, fightWithWeapon);
			m_fightButtonPressure = 0;
			return;
		}

		if (curMove.hitLevel != HITLEVEL_NULL) {
			if (animTime > curMove.endFireTime && weaponInfo->m_AnimToPlay != ASSOCGRP_KNIFE) {
				if (IsPlayer())
					currentAssoc->speed = 1.0f;
				else
					currentAssoc->speed = 0.8f;
			}

			if (IsPlayer() && !nPlayerInComboMove && !fightWithWeapon) {
				if (curMove.comboFollowOnTime > 0.0f && m_fightButtonPressure != 0 && animTime > curMove.comboFollowOnTime) {

					m_lastFightMove = m_curFightMove;
					// Notice that it increases fight move index, because we're in combo!
					animAssoc = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, tFightMoves[++m_curFightMove].animId, 8.0f);
					animAssoc->SetFinishCallback(FinishFightMoveCB, this);
					animAssoc->SetCurrentTime(0.1f * animAssoc->hierarchy->totalLength);
					animAssoc->speed = 0.8f;
					m_fightButtonPressure = 0;
					nPlayerInComboMove = 1;
				}
			}
		}

	} else if (GetWeapon()->m_eWeaponType != WEAPONTYPE_UNARMED && GetWeapon()->m_eWeaponType != WEAPONTYPE_BRASSKNUCKLE && !fightWithWeapon) {
		EndFight(ENDFIGHT_FAST);

	} else if (m_fightButtonPressure != 0) {
		if (!IsPlayer())
			Say(SOUND_PED_ATTACK);

		if (m_curFightMove != FIGHTMOVE_IDLE)
			m_lastFightMove = m_curFightMove;

		m_curFightMove = IsPlayer() ? ChooseAttackPlayer(m_fightButtonPressure, fightWithWeapon) : ChooseAttackAI(m_fightButtonPressure, fightWithWeapon);

		if (m_curFightMove != FIGHTMOVE_IDLE) {

			animAssoc = CAnimManager::BlendAnimation(GetClump(), m_curFightMove < FIGHTMOVE_MELEE1 ? ASSOCGRP_STD : weaponInfo->m_AnimToPlay,
				tFightMoves[m_curFightMove].animId, 8.0f);

			if (weaponInfo->m_AnimToPlay != ASSOCGRP_KNIFE || m_curFightMove < FIGHTMOVE_MELEE1) {
				if (m_curFightMove == FIGHTMOVE_BACKKICK)
					animAssoc->speed = 1.15f;
				else
					animAssoc->speed = 0.8f;
			} else {
				switch (GetWeapon()->m_eWeaponType) {
					case WEAPONTYPE_SCREWDRIVER:
					case WEAPONTYPE_KNIFE:
						animAssoc->speed = 1.05f;
						break;
					case WEAPONTYPE_GOLFCLUB:
					case WEAPONTYPE_NIGHTSTICK:
					case WEAPONTYPE_BASEBALLBAT:
					case WEAPONTYPE_HAMMER:
					case WEAPONTYPE_KATANA:
						animAssoc->speed = 0.8f;
						break;
					case WEAPONTYPE_CLEAVER:
					case WEAPONTYPE_MACHETE:
						animAssoc->speed = 0.9f;
						break;
				}
			}

			if (m_fightState == FIGHTSTATE_MOVE_FINISHED && animAssoc->currentTime != 0.0f) {
				animAssoc->SetRun();
				if (!IsPlayer())
					animAssoc->SetCurrentTime(0.0f);
			}
			if (IsPlayer())
				animAssoc->SetCurrentTime(0.08f);

			animAssoc->SetFinishCallback(FinishFightMoveCB, this);
			m_fightButtonPressure = 0;
		}
		m_fightState = FIGHTSTATE_NO_MOVE;
	} else if (m_takeAStepAfterAttack && m_curFightMove != FIGHTMOVE_SHUFFLE_F
#ifndef FIX_BUGS
		&& CheckForPedsOnGroundToAttack(this, nil) == 4) {
#else
		&& CheckForPedsOnGroundToAttack(this, nil) == PED_IN_FRONT_OF_ATTACKER) {
#endif
		m_lastFightMove = m_curFightMove;
		m_curFightMove = FIGHTMOVE_SHUFFLE_F;
		animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), tFightMoves[m_curFightMove].animId);

		if (animAssoc) {
			animAssoc->SetCurrentTime(0.0f);
			animAssoc->blendDelta = 4.0f;
			animAssoc->SetRun();
		} else {
			animAssoc = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, tFightMoves[m_curFightMove].animId, 32.0f);
		}
		animAssoc->SetFinishCallback(FinishFightMoveCB, this);
		m_fightState = FIGHTSTATE_NO_MOVE;
		m_fightButtonPressure = 0;
		m_takeAStepAfterAttack = false;

	} else if (m_takeAStepAfterAttack) {
		EndFight(ENDFIGHT_FAST);

	} else if (m_curFightMove == FIGHTMOVE_IDLE) {
		if (CTimer::GetTimeInMilliseconds() > m_nWaitTimer) {
			EndFight(ENDFIGHT_NORMAL);
		}

	} else {
		m_lastFightMove = m_curFightMove;
		m_curFightMove = FIGHTMOVE_IDLE;
		if (IsPlayer())
			m_nWaitTimer = CTimer::GetTimeInMilliseconds() + 500;
		else
			m_nWaitTimer = CTimer::GetTimeInMilliseconds() + 2000;
	}
}

// --MIAMI: Done
int32
CPed::ChooseAttackAI(uint8 buttonPressure, bool fightWithWeapon)
{
	eWeaponType weapon = GetWeapon()->m_eWeaponType;
	CWeaponInfo *weaponInfo = CWeaponInfo::GetWeaponInfo(weapon);
	if (!fightWithWeapon && weapon != WEAPONTYPE_UNARMED && weapon != WEAPONTYPE_BRASSKNUCKLE) {
		return FIGHTMOVE_PUNCH;
	}

	if (!m_pedInObjective)
		return FIGHTMOVE_IDLE;
	if (buttonPressure == 0)
		return FIGHTMOVE_IDLE;

	uint16 pedFeatures = m_pedStats->m_flags;
	bool punchOnly = !!(pedFeatures & STAT_PUNCH_ONLY);
	bool canRoundhouse = !!(pedFeatures & STAT_CAN_ROUNDHOUSE);
	bool canKneeHead = !!(pedFeatures & STAT_CAN_KNEE_HEAD);
	bool canKick = !!(pedFeatures & STAT_CAN_KICK);
	bool hasShoppingBags = !!(pedFeatures & STAT_SHOPPING_BAGS);

	CVector distVec(m_pedInObjective->GetPosition() - GetPosition());
	float dist = distVec.Magnitude();
	m_fRotationDest = CGeneral::LimitRadianAngle(distVec.Heading());
	m_fRotationCur = m_fRotationDest;

	if (fightWithWeapon) {
		if (m_pedInObjective->OnGroundOrGettingUp()) {
			if (CGame::nastyGame && dist < 1.2f && !m_pedInObjective->IsPlayer()
				&& (m_pedInObjective->m_nPedState == PED_DEAD || !m_pedInObjective->IsPedHeadAbovePos(-0.3f))) {
				if (weaponInfo->m_bGround2nd)
					return FIGHTMOVE_MELEE2;
				if (weaponInfo->m_bGround3rd)
					return FIGHTMOVE_MELEE3;

				return FIGHTMOVE_GROUNDKICK;
			} else {
				return FIGHTMOVE_IDLE;
			}
		}
		if (dist < 2.f) {
			if (m_curFightMove == FIGHTMOVE_MELEE1) {
				if (GetSecondFireAnim(weaponInfo))
					return FIGHTMOVE_MELEE2;
			}
			if (m_curFightMove == FIGHTMOVE_MELEE2) {
				if (GetFinishingAttackAnim(weaponInfo))
					return FIGHTMOVE_MELEE3;
			}
			return FIGHTMOVE_MELEE1;
		}
		return FIGHTMOVE_SHUFFLE_F;
	}
	if (!hasShoppingBags) {
		if (punchOnly) {
			if (dist < 1.4f)
				return FIGHTMOVE_PUNCH;
		} else {
			if (m_pedInObjective->OnGroundOrGettingUp()) {
				if (CGame::nastyGame && dist < 1.2f && !m_pedInObjective->IsPlayer()
					&& (m_pedInObjective->m_nPedState == PED_DEAD || !m_pedInObjective->IsPedHeadAbovePos(-0.3f))) {

					return FIGHTMOVE_GROUNDKICK;
				} else {
					return FIGHTMOVE_IDLE;
				}
			}
			if (dist < 0.95f && canKneeHead)
				return FIGHTMOVE_KNEE;
			if (dist < 1.4f)
				return FIGHTMOVE_PUNCH;
			if (dist < 2.f && canKick) {
				int nextMove = FIGHTMOVE_LONGKICK;
				if (canRoundhouse && CGeneral::GetRandomNumber() & 1)
					nextMove = FIGHTMOVE_ROUNDHOUSE;
				return nextMove;
			}
		}
		return FIGHTMOVE_SHUFFLE_F;
	}
	if (dist < 2.f)
		return FIGHTMOVE_ROUNDHOUSE;
	else
		return FIGHTMOVE_SHUFFLE_F;
}

// --MIAMI: Done
int32
CPed::ChooseAttackPlayer(uint8 buttonPressure, bool fightWithWeapon)
{
	CWeaponInfo *weaponInfo = CWeaponInfo::GetWeaponInfo(GetWeapon()->m_eWeaponType);
	const float maxAttackDist = 2.7f;
	float weaponAttackDist = 2.0f;
	CPed *victimPed = nil;
	CPed *walkUpTo = nil;
	CPed *groundAttackDeadPed = nil;
	CPed *groundAttackAlivePed = nil;
	if (fightWithWeapon)
		weaponAttackDist = weaponInfo->m_fRange;

	bool willWalkUp = false;
	PedFightMoves choosenMove = FIGHTMOVE_IDLE;
	int numPedsWeCanReach = 0;
	if (m_takeAStepAfterAttack)
		willWalkUp = true;

	float groundAttackDeadAngle, groundAttackAliveAngle, walkAngle, victimAngle, distToVictim;

	for (int i = 0; i < m_numNearPeds; ++i) {
		CPed *nearPed = m_nearPeds[i];
		CVector distVec(nearPed->GetPosition() - GetPosition());
		float dist = distVec.Magnitude();
		if (dist < maxAttackDist) {
			float nearPedAngle = CGeneral::LimitRadianAngle(distVec.Heading());
			m_fRotationCur = CGeneral::LimitRadianAngle(m_fRotationCur);
			float neededTurn = Abs(nearPedAngle - m_fRotationCur);
			if (neededTurn > PI)
				neededTurn = TWOPI - neededTurn;

			if (!nearPed->OnGroundOrGettingUp() && nearPed->m_nWaitState != WAITSTATE_SUN_BATHE_IDLE) {
				if (!willWalkUp || neededTurn <= DEGTORAD(45.0f)) {

					if (neededTurn <= DEGTORAD(30.0f) || nearPed->m_pedInObjective == this
						&& (nearPed->m_objective == OBJECTIVE_KILL_CHAR_ON_FOOT || nearPed->m_objective == OBJECTIVE_KILL_CHAR_ANY_MEANS)) {

						if (dist < weaponAttackDist) {
							if (!victimPed
								|| nearPed->m_attackTimer < victimPed->m_attackTimer && nearPed->m_attackTimer > CTimer::GetTimeInMilliseconds() - 100) {
								victimPed = nearPed;
								victimAngle = nearPedAngle;
								distToVictim = dist;
							}
							++numPedsWeCanReach;
							
						} else {
							if (neededTurn < DEGTORAD(30.0f)) {
								walkUpTo = nearPed;
								walkAngle = nearPedAngle;
							}
						}
					}
				}
			} else if (CGame::nastyGame && dist < 1.2f && neededTurn < DEGTORAD(55.0f)) {
				if (!nearPed->DyingOrDead() || groundAttackDeadPed) {
					if (!nearPed->IsPedHeadAbovePos(-0.3f)) {
						groundAttackAlivePed = nearPed;
						groundAttackAliveAngle = nearPedAngle;
					}
				} else {
					groundAttackDeadPed = nearPed;
					groundAttackDeadAngle = nearPedAngle;
				}
				++numPedsWeCanReach;

			} else if (dist > 1.4f && dist < maxAttackDist && neededTurn < DEGTORAD(30.0f)) {
				if (!walkUpTo) {
					walkUpTo = nearPed;
					walkAngle = nearPedAngle;
				}
#ifdef FIX_BUGS
				if(dist < 2.1f)
#endif
				++numPedsWeCanReach;
			}
		}
	}

	if (victimPed) {
		float adjustedAngleDiff = victimAngle - m_fRotationCur + DEGTORAD(30.0f);
		if (adjustedAngleDiff < 0.0f)
			adjustedAngleDiff += TWOPI;

		int16 dir = Floor(adjustedAngleDiff / DEGTORAD(60.0f));

		// Just focus on who we're fighting with, don't care peds on ground
		if (numPedsWeCanReach < 2 || fightWithWeapon) {
			float angleDiff = Abs(victimAngle - m_fRotationCur);
			if (angleDiff > PI)
				angleDiff = TWOPI - angleDiff;

			if (angleDiff < DEGTORAD(60.0f))
				dir = 0; // forward
		}
		int16 randVal = CGeneral::GetRandomNumber() & 3;
		switch (dir) {
			case 0: // forward
				if (fightWithWeapon) {
					if (distToVictim < 0.95f - 0.2f && m_nPedState == PED_FIGHT) {
						choosenMove = FIGHTMOVE_KNEE;
					} else {
						if (GetWeapon()->m_eWeaponType == WEAPONTYPE_CLEAVER) {
							if (distToVictim < 0.85f * weaponInfo->m_fRange)
								choosenMove = FIGHTMOVE_MELEE1;
							else
								choosenMove = FIGHTMOVE_SHUFFLE_F;
						} else {
							float weaponRange = weaponInfo->m_fRange;
							if (distToVictim < 0.75f * weaponRange && GetWeapon()->m_eWeaponType != WEAPONTYPE_SCREWDRIVER) {
								if (m_lastFightMove == FIGHTMOVE_MELEE1 && GetFinishingAttackAnim(weaponInfo)) {
									choosenMove = FIGHTMOVE_MELEE2;
								} else if (m_lastFightMove == FIGHTMOVE_MELEE2 && GetFinishingAttackAnim(weaponInfo)) {
									choosenMove = FIGHTMOVE_MELEE3;
								} else {
									choosenMove = FIGHTMOVE_MELEE1;
								}
							} else if (distToVictim < weaponRange && GetFinishingAttackAnim(weaponInfo)) {
								choosenMove = FIGHTMOVE_MELEE3;
							} else {
								choosenMove = FIGHTMOVE_SHUFFLE_F;
							}
						}
					}
				} else if (distToVictim < 0.95f && m_nPedState == PED_FIGHT) {
					choosenMove = FIGHTMOVE_KNEE;

				} else if (distToVictim < 1.4f) {
					if (m_curFightMove == FIGHTMOVE_PUNCHJAB) {
						choosenMove = FIGHTMOVE_PUNCH;

					} else if (m_curFightMove != FIGHTMOVE_PUNCH || randVal != 1) {
						if (randVal == 2)
							choosenMove = FIGHTMOVE_PUNCH;
						else
							choosenMove = FIGHTMOVE_PUNCHJAB;
					} else {
						choosenMove = FIGHTMOVE_LONGKICK;
					}
				} else {
					choosenMove = FIGHTMOVE_LONGKICK;
				}
				break;
			case 1:
				choosenMove = FIGHTMOVE_FWDLEFT;
				break;
			case 2:
				choosenMove = FIGHTMOVE_BACKLEFT;
				break;
			case 3:
				choosenMove = FIGHTMOVE_BACKKICK;
				break;
			case 4:
				choosenMove = FIGHTMOVE_BACKRIGHT;
				break;
			default:
				choosenMove = FIGHTMOVE_FWDRIGHT;
				break;
		}

		// forward
		if (dir == 0) {
			m_fRotationDest = CGeneral::LimitRadianAngle(victimAngle);
		} else {
			m_fRotationDest = victimAngle - dir * DEGTORAD(60.0f);
			m_fRotationDest = CGeneral::LimitRadianAngle(m_fRotationDest);
		}
		
		m_fRotationCur = m_fRotationDest;
		Say(SOUND_PED_ATTACK);

	} else if (groundAttackAlivePed || groundAttackDeadPed) {
		if (fightWithWeapon && weaponInfo->m_bGround2nd) {
			choosenMove = FIGHTMOVE_MELEE2;
		} else if (fightWithWeapon && weaponInfo->m_bGround3rd) {
			choosenMove = FIGHTMOVE_MELEE3;
		} else {
			choosenMove = FIGHTMOVE_GROUNDKICK;
		}
		if (groundAttackAlivePed)
			m_fRotationDest = groundAttackAliveAngle;
		else
			m_fRotationDest = groundAttackDeadAngle;

		m_fRotationCur = m_fRotationDest;
		m_lookTimer = 0;
		if (groundAttackAlivePed)
			SetLookFlag(groundAttackAlivePed, 1, 0);
		else
			SetLookFlag(groundAttackDeadPed, 1, 0);

		SetLookTimer(1500u);

	} else if (walkUpTo) {
		choosenMove = FIGHTMOVE_SHUFFLE_F;
		m_fRotationCur = m_fRotationDest = walkAngle;
		m_lookTimer = 0;
		SetLookFlag(walkUpTo, true);
		SetLookTimer(1500);

	} else if (fightWithWeapon) {
		// No enemy, fight with space
		if (GetWeapon()->m_eWeaponType == WEAPONTYPE_SCREWDRIVER) {
			choosenMove = FIGHTMOVE_MELEE3;
		} else {
			if (m_lastFightMove == FIGHTMOVE_MELEE1 && GetFinishingAttackAnim(weaponInfo)) {
				choosenMove = FIGHTMOVE_MELEE2;
			} else if (m_lastFightMove == FIGHTMOVE_MELEE2 && GetFinishingAttackAnim(weaponInfo)) {
				choosenMove = FIGHTMOVE_MELEE3;
			} else {
				choosenMove = FIGHTMOVE_MELEE1;
			}
		}
	} else {
		// Max number GetRandomNumberInRange returns is max-1
#ifdef FIX_BUGS
		switch (CGeneral::GetRandomNumberInRange(0,4)) {
#else
		switch (CGeneral::GetRandomNumberInRange(0,3)) {
#endif
			case 0:
				choosenMove = FIGHTMOVE_PUNCHJAB;
				break;
			case 1:
				choosenMove = FIGHTMOVE_PUNCH;
				break;
			case 2:
				choosenMove = FIGHTMOVE_LONGKICK;
				break;
			case 3:
				choosenMove = FIGHTMOVE_KNEE;
				break;
			default:
				break;
		}
	}
	return choosenMove;
}

// --MIAMI: Done
void
CPed::EndFight(uint8 endType)
{
	if (m_nPedState != PED_FIGHT)
		return;

	m_curFightMove = FIGHTMOVE_NULL;
	RestorePreviousState();
	CAnimBlendAssociation *animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_FIGHT_IDLE);
	if (!animAssoc)
		animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_WEAPON_CROUCHRELOAD);

	if (animAssoc)
		animAssoc->flags |= ASSOC_DELETEFADEDOUT;

	switch (endType) {
		case ENDFIGHT_NORMAL:
			CAnimManager::BlendAnimation(GetClump(), m_animGroup, ANIM_IDLE_STANCE, 8.0f);
			CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_FIGHT2_IDLE, 8.0f);
			break;
		case ENDFIGHT_WITH_A_STEP:
			CAnimManager::BlendAnimation(GetClump(), m_animGroup, ANIM_IDLE_STANCE, 1.0f);
			CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_WALK_START, 8.0f);
			break;
		case ENDFIGHT_FAST:
			CAnimManager::BlendAnimation(GetClump(), m_animGroup, ANIM_IDLE_STANCE, 8.0f);
			CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_FIGHT2_IDLE, 8.0f)->speed = 2.0f;
			break;
		default:
			break;
	}
	m_nWaitTimer = 0;
}

// --MIAMI: Done
void
CPed::PlayHitSound(CPed *hitTo)
{
	// That was very complicated to reverse for me...
	// First index is our fight move ID (from 1 to 17, total 17), second is the one of we fight with (from 18 to 27, total 10).
	enum {
		S37 = SOUND_FIGHT_37,
		S38 = SOUND_FIGHT_38,
		S39 = SOUND_FIGHT_39,
		S40 = SOUND_FIGHT_40,
		S41 = SOUND_FIGHT_41,
		S42 = SOUND_FIGHT_42,
		S43 = SOUND_FIGHT_43,
		S44 = SOUND_FIGHT_44,
		S45 = SOUND_FIGHT_45,
		S46 = SOUND_FIGHT_46,
		S47 = SOUND_FIGHT_47,
		S48 = SOUND_FIGHT_48,
		NO_SND = SOUND_NO_SOUND
	};
	const uint16 hitSoundsByFightMoves[17][10] = {
	  { S37, S46, S41, S41, S46, S46, S40, S41, S43, S40 },
	  { NO_SND, NO_SND, NO_SND, NO_SND, NO_SND, NO_SND, NO_SND, NO_SND, NO_SND, NO_SND },
	  { NO_SND, NO_SND, NO_SND, NO_SND, NO_SND, NO_SND, NO_SND, NO_SND, NO_SND, NO_SND },
	  { S46, S46, S46, S46, S37, S47, S37, S38, S43, S38 },
	  { S46, S46, S46, S46, S46, S46, S40, S41, S43, S46 },
	  { S46, S46, S46, S46, S46, S46, S40, S41, S43, S40 },
	  { S46, S46, S46, S46, S46, S46, S40, S41, S43, S40 },
	  { S46, S46, S37, S46, S37, S47, S40, S47, S43, S37 },
	  { S46, S46, S46, S46, S46, S46, S43, S44, S43, S43 },
	  { S37, S46, S46, S46, S38, S47, S40, S38, S43, S46 },
	  { S46, S37, S46, S37, S39, S46, S40, S39, S43, S37 },
	  { S46, S37, S46, S46, S38, S47, S40, S38, S43, S46 },
	  { S37, S37, S46, S46, S38, S47, S48, S38, S43, S37 },
	  { S46, S46, S46, S46, S37, S46, S40, S38, S43, S46 },
	  { S46, S46, S46, S37, S39, S46, S40, S39, S43, S46 },
	  { S37, S46, S46, S46, S37, S46, S40, S37, S43, S46 },
	  { S43, S43, S43, S43, S43, S43, S43, S43, S43, S43 }
	};

	eWeaponType weapon = GetWeapon()->m_eWeaponType;
	CWeaponInfo *weaponInfo = CWeaponInfo::GetWeaponInfo(weapon);
	if (weaponInfo->m_AnimToPlay == ASSOCGRP_KNIFE) {
		if (m_curFightMove >= FIGHTMOVE_MELEE1) {
			if (m_curFightMove == FIGHTMOVE_MELEE3) {
				DMAudio.PlayOneShot(m_audioEntityId, SOUND_WEAPON_BAT_ATTACK, (weapon << 8) | 3);
			} else {
				DMAudio.PlayOneShot(m_audioEntityId, SOUND_WEAPON_KNIFE_ATTACK, (weapon << 8) | 3);
			}
			return;
		}
	}

	// This is why first dimension is between FightMove 1 and 17.
	if (m_curFightMove <= FIGHTMOVE_NULL || m_curFightMove >= FIGHTMOVE_HITFRONT)
		return;

	uint16 soundId;

	// And this is why second dimension is between 18 and 27.
	if (hitTo->m_curFightMove > FIGHTMOVE_GROUNDKICK && hitTo->m_curFightMove < FIGHTMOVE_IDLE2NORM) {
		soundId = hitSoundsByFightMoves[m_curFightMove - FIGHTMOVE_STDPUNCH][hitTo->m_curFightMove - FIGHTMOVE_HITFRONT];

	} else {
		if (hitTo->m_nPedState == PED_DEAD || hitTo->UseGroundColModel()) {	
			soundId = hitSoundsByFightMoves[m_curFightMove - FIGHTMOVE_STDPUNCH][FIGHTMOVE_HITONFLOOR - FIGHTMOVE_HITFRONT];
		} else {
			soundId = hitSoundsByFightMoves[m_curFightMove - FIGHTMOVE_STDPUNCH][FIGHTMOVE_HITFRONT - FIGHTMOVE_HITFRONT];
		}
	}

	if (soundId != NO_SND)
		DMAudio.PlayOneShot(m_audioEntityId, soundId, (weapon << 8) | 3);
}

// --MIAMI: Done
bool
CPed::FightStrike(CVector &touchedNodePos, bool fightWithWeapon)
{
	CColModel *hisCol;
	CVector attackDistance;
	float maxDistanceToBeat;
	CPed *nearPed;
	CVector extendedTouchPoint;

	CWeaponInfo *weaponInfo = CWeaponInfo::GetWeaponInfo(GetWeapon()->m_eWeaponType);
	float radius = tFightMoves[m_curFightMove].strikeRadius;
	if (fightWithWeapon)
		radius = weaponInfo->m_fRadius;

	if (m_fightState == FIGHTSTATE_JUST_ATTACKED)
		return false;

	if (this == FindPlayerPed() && fightWithWeapon && GetWeapon()->m_eWeaponType != WEAPONTYPE_UNARMED)
		CGlass::BreakGlassPhysically(touchedNodePos, radius);

	for (int i = 0; i < m_numNearPeds; i++) {
		int8 pedFound = 0;
		nearPed = m_nearPeds[i];
		if (!fightWithWeapon && GetWeapon()->m_eWeaponType != WEAPONTYPE_UNARMED && GetWeapon()->m_eWeaponType != WEAPONTYPE_BRASSKNUCKLE)
			maxDistanceToBeat = nearPed->GetBoundRadius() + radius + 0.1f;
		else
			maxDistanceToBeat = nearPed->GetBoundRadius() + radius;

		if ((nearPed->bUsesCollision || nearPed->m_nPedState == PED_DEAD) && (m_pedInObjective != FindPlayerPed() || nearPed == FindPlayerPed())) {
			CVector nearPedCentre;

			// Have to animate a skinned clump because the initial col model is useless
			hisCol = ((CPedModelInfo*)CModelInfo::GetModelInfo(nearPed->GetModelIndex()))->AnimatePedColModelSkinnedWorld(nearPed->GetClump());

			nearPed->GetBoundCentre(nearPedCentre);
			CVector potentialAttackDistance = nearPedCentre - touchedNodePos;

			// He can beat us
			if (sq(maxDistanceToBeat) > potentialAttackDistance.MagnitudeSqr()) {

				for (int j = 0; j < hisCol->numSpheres; j++) {
					attackDistance = hisCol->spheres[j].center;
					attackDistance -= touchedNodePos;
					CColSphere *hisPieces = hisCol->spheres;
					maxDistanceToBeat = hisPieces[j].radius + radius;

					// We can beat him too
					if (sq(maxDistanceToBeat) > attackDistance.MagnitudeSqr()) {
						FightHitPed(nearPed, touchedNodePos, attackDistance, hisPieces[j].piece);
						pedFound = 1;
						break;
					}
				}
			}
			if (!pedFound && !fightWithWeapon) {
				extendedTouchPoint = touchedNodePos - GetPosition();
				if (DotProduct(touchedNodePos - GetPosition(), nearPed->GetPosition() - GetPosition()) > 0.f) {
					if (m_curFightMove == FIGHTMOVE_GROUNDKICK) {
						extendedTouchPoint += tFightMoves[FIGHTMOVE_GROUNDKICK].extendReachMultiplier * GetForward();
					} else {
						extendedTouchPoint.x *= tFightMoves[m_curFightMove].extendReachMultiplier;
						extendedTouchPoint.y *= tFightMoves[m_curFightMove].extendReachMultiplier;
					}
					pedFound = -1;
					extendedTouchPoint += GetPosition();
				}
			}
			if (pedFound == -1) {
				CVector nearPedCentre = nearPed->GetBoundCentre();
				if (sq(maxDistanceToBeat) > (nearPedCentre - extendedTouchPoint).MagnitudeSqr()) {

					for (int j = 0; j < hisCol->numSpheres; j++) {
						attackDistance = hisCol->spheres[j].center;
						attackDistance -= extendedTouchPoint;
						CColSphere* hisPieces = hisCol->spheres;
						float maxDistanceToBeat2 = hisPieces[j].radius + tFightMoves[m_curFightMove].strikeRadius;

						// We can beat him too
						if (sq(maxDistanceToBeat2) > attackDistance.MagnitudeSqr()) {
							FightHitPed(nearPed, extendedTouchPoint, attackDistance, hisPieces[j].piece);
							break;
						}
					}
				}
			}
		}
	}


	if (m_fightState == FIGHTSTATE_NO_MOVE)
		m_fightState = FIGHTSTATE_1;

	m_vecHitLastPos = *touchedNodePos;
	return false;
}

// --MIAMI: Done
void
CPed::FightHitPed(CPed *victim, CVector &touchPoint, CVector &dir, int16 piece)
{
	if (victim->IsPlayer() && victim->m_nPedState == PED_GETUP)
		return;

	CWeaponInfo *weaponInfo = CWeaponInfo::GetWeaponInfo(GetWeapon()->m_eWeaponType);
	bool fightingWithWeapon = false;
	int damageMult = tFightMoves[m_curFightMove].damage * ((CGeneral::GetRandomNumber() & 1) + 2) + 1;

	if (weaponInfo->m_bFightMode) {
		fightingWithWeapon = true;
		if (m_curFightMove >= FIGHTMOVE_MELEE1) {
			damageMult = weaponInfo->m_nDamage;
			if (m_curFightMove == FIGHTMOVE_MELEE3 && GetWeapon()->m_eWeaponType != WEAPONTYPE_SCREWDRIVER)
				damageMult *= 5;
		}
	}

	if (IsPlayer()) {
		if (((CPlayerPed*)this)->m_bAdrenalineActive)
			damageMult = 20;
	} else if (!fightingWithWeapon) {
		damageMult *= m_pedStats->m_attackStrength;
	}

	float oldVictimHealth = victim->m_fHealth;
	CVector bloodPos = 0.5f * dir + touchPoint;
	CVector2D diff(GetPosition() - victim->GetPosition());
	int direction = victim->GetLocalDirection(diff);

	bool brassKnucklePunch = false;
	if (GetWeapon()->m_eWeaponType == WEAPONTYPE_BRASSKNUCKLE) {
		if (m_curFightMove == FIGHTMOVE_PUNCHHOOK || m_curFightMove == FIGHTMOVE_PUNCHJAB || m_curFightMove == FIGHTMOVE_BACKLEFT ||
			m_curFightMove == FIGHTMOVE_STDPUNCH || m_curFightMove == FIGHTMOVE_PUNCH) {
			brassKnucklePunch = true;
			damageMult *= 1.5f;
		}
	}
	victim->ReactToAttack(this);

	// Mostly unused. if > 5, ANIM_HIT_BIGSTEP will be run, that's it.
	int unk2;
	if (GetWeapon()->m_eWeaponType != WEAPONTYPE_UNARMED && GetWeapon()->m_eWeaponType != WEAPONTYPE_BRASSKNUCKLE &&
		!victim->IsPlayer() && !fightingWithWeapon)
		unk2 = 101;
	else
		unk2 = damageMult;

	victim->StartFightDefend(direction, tFightMoves[m_curFightMove].hitLevel, unk2);
	PlayHitSound(victim);
	m_fightState = FIGHTSTATE_JUST_ATTACKED;
	RpAnimBlendClumpGetAssociation(GetClump(), tFightMoves[m_curFightMove].animId)->speed = 0.6f;

	if (!victim->DyingOrDead()) {
		if(fightingWithWeapon)
			victim->InflictDamage(this, GetWeapon()->m_eWeaponType, damageMult, (ePedPieceTypes)piece, direction);
		else
			victim->InflictDamage(this, WEAPONTYPE_UNARMED, damageMult * 3.0f, (ePedPieceTypes)piece, direction);
	}

	if (CGame::nastyGame && weaponInfo->m_AnimToPlay == ASSOCGRP_KNIFE && m_curFightMove >= FIGHTMOVE_MELEE1
		&& victim->GetIsOnScreen()) {

		static float particleRightLen = 0.05f;
		static float particleUpLen = 0.05f;

		// Just for particles. We will restore it below.
		dir /= (20.0f * dir.Magnitude());
		if (m_curFightMove == FIGHTMOVE_MELEE1) {
			float rightMult = -particleRightLen;
			dir += particleUpLen * GetUp() + rightMult * GetRight();

		} else if (m_curFightMove == FIGHTMOVE_MELEE2) {
			float upMult = 2.0f * particleUpLen;
			dir += particleRightLen * GetRight() + upMult * GetUp();
		}
		CParticle::AddParticle(PARTICLE_BLOOD_SPURT, bloodPos, dir, nil, 0.0f, 0, 0, 0, 0);
		CParticle::AddParticle(PARTICLE_BLOOD_SPURT, bloodPos, dir, nil, 0.0f, 0, 0, 0, 0);
		if (IsPlayer()) {
			CParticle::AddParticle(PARTICLE_BLOOD_SPURT, bloodPos, dir, nil, 0.0f, 0, 0, 0, 0);
			CParticle::AddParticle(PARTICLE_BLOOD_SPURT, bloodPos, dir, nil, 0.0f, 0, 0, 0, 0);
		}
		if (!(CGeneral::GetRandomNumber() & 3)) {
			CParticle::AddParticle(PARTICLE_TEST, bloodPos, dir, nil, 0.0f, 0, 0, 0, 0);
		}
	} else if (CGame::nastyGame && (tFightMoves[m_curFightMove].hitLevel > HITLEVEL_MEDIUM || fightingWithWeapon)
			&& victim->GetIsOnScreen()) {

		// Just for particles. We will restore it below.
		dir /= (10.0f * dir.Magnitude());
		for (int i = 0; i < 4; i++) {
			CParticle::AddParticle(PARTICLE_BLOOD, bloodPos, dir, nil, 0.0f, 0, 0, 0, 0);
		}
	}

	eWeaponType weaponType = GetWeapon()->m_eWeaponType;
	if (!fightingWithWeapon) {
		if (!victim->OnGround()) {
			float curVictimHealth = victim->m_fHealth;
			if (curVictimHealth > 0.0f
				&& (curVictimHealth < 30.0f && oldVictimHealth > 30.0f
					|| weaponType != WEAPONTYPE_UNARMED && weaponType != WEAPONTYPE_BRASSKNUCKLE && IsPlayer()
					|| victim->m_pedStats->m_flags & STAT_ONE_HIT_KNOCKDOWN || brassKnucklePunch)) {

				victim->SetFall(0, (AnimationId)(direction + ANIM_KO_SKID_FRONT), 0);
				if (victim->m_nPedState == PED_FALL)
					victim->bIsStanding = false;
			}
		}
	}

	if (victim->m_nPedState == PED_DIE || !victim->bIsStanding) {
		dir = victim->GetPosition() - GetPosition();
		dir.Normalise();
		dir.z = 1.0f;
		victim->bIsStanding = false;

		float moveMult;
		if (fightingWithWeapon) {
			moveMult = Min(damageMult * 0.02f, 1.0f);
		} else if (m_curFightMove == FIGHTMOVE_GROUNDKICK) {
			moveMult = Min(damageMult * 0.6f, 4.0f);
		} else {
			if (victim->m_nPedState != PED_DIE || damageMult >= 20) {
				moveMult = damageMult;
			} else {
				moveMult = Min(damageMult * 2.0f, 14.0f);
			}
		}

		victim->ApplyMoveForce(moveMult * 0.6 * dir);
	}

	if (weaponType != WEAPONTYPE_KNIFE && weaponType != WEAPONTYPE_MACHETE
		&& weaponType != WEAPONTYPE_KATANA && weaponType != WEAPONTYPE_CHAINSAW) {

		if (victim->m_nPedType == PEDTYPE_COP)
			CEventList::RegisterEvent(EVENT_ASSAULT_POLICE, EVENT_ENTITY_PED, victim, this, 2000);
		else
			CEventList::RegisterEvent(EVENT_ASSAULT, EVENT_ENTITY_PED, victim, this, 2000);
	} else {
		if (victim->m_nPedType == PEDTYPE_COP)
			CEventList::RegisterEvent(EVENT_ASSAULT_NASTYWEAPON_POLICE, EVENT_ENTITY_PED, victim, this, 2000);
		else
			CEventList::RegisterEvent(EVENT_ASSAULT_NASTYWEAPON, EVENT_ENTITY_PED, victim, this, 2000);
	}
}

// --MIAMI: Done
void
CPed::FinishFightMoveCB(CAnimBlendAssociation *animAssoc, void *arg)
{
	CPed *ped = (CPed*)arg;

	if (tFightMoves[ped->m_curFightMove].animId == animAssoc->animId) {
		ped->m_fightState = FIGHTSTATE_MOVE_FINISHED;
		animAssoc->blendDelta = -1000.0f;
	}
}

// --MIAMI: Done
void
CPed::LoadFightData(void)
{
	float startFireTime, endFireTime, comboFollowOnTime, strikeRadius, extendReachMultiplier;
	int damage, flags;
	char line[256], moveName[32], animName[32], hitLevel;
	int moveId = 0;

	CAnimBlendAssociation *animAssoc;

	size_t bp, buflen;
	int lp, linelen;

	buflen = CFileMgr::LoadFile("DATA\\fistfite.dat", work_buff, sizeof(work_buff), "r");

	for (bp = 0; bp < buflen; ) {
		// read file line by line
		for (linelen = 0; work_buff[bp] != '\n' && bp < buflen; bp++) {
			line[linelen++] = work_buff[bp];
		}
		bp++;
		line[linelen] = '\0';

		// skip white space
		for (lp = 0; line[lp] <= ' ' && line[lp] != '\0'; lp++);

		if (line[lp] == '\0' ||
			line[lp] == '#')
			continue;

		sscanf(
			&line[lp],
			"%s %f %f %f %f %f %c %s %d %d",
			moveName,
			&startFireTime,
			&endFireTime,
			&comboFollowOnTime,
			&strikeRadius,
			&extendReachMultiplier,
			&hitLevel,
			animName,
			&damage,
			&flags);

		if (strncmp(moveName, "ENDWEAPONDATA", 13) == 0)
			return;

		tFightMoves[moveId].startFireTime = startFireTime / 30.0f;
		tFightMoves[moveId].endFireTime = endFireTime / 30.0f;
		tFightMoves[moveId].comboFollowOnTime = comboFollowOnTime / 30.0f;
		tFightMoves[moveId].strikeRadius = strikeRadius;
		tFightMoves[moveId].extendReachMultiplier = extendReachMultiplier;
		tFightMoves[moveId].damage = damage;
		tFightMoves[moveId].flags = flags;

		switch (hitLevel) {
			case 'G':
				tFightMoves[moveId].hitLevel = HITLEVEL_GROUND;
				break;
			case 'H':
				tFightMoves[moveId].hitLevel = HITLEVEL_HIGH;
				break;
			case 'L':
				tFightMoves[moveId].hitLevel = HITLEVEL_LOW;
				break;
			case 'M':
				tFightMoves[moveId].hitLevel = HITLEVEL_MEDIUM;
				break;
			case 'N':
				tFightMoves[moveId].hitLevel = HITLEVEL_NULL;
				break;
			default:
				break;
		}

		if (strncmp(animName, "default", 8) != 0) {
			if (strncmp(animName, "null", 5) != 0) {
				animAssoc = CAnimManager::GetAnimAssociation(ASSOCGRP_STD, animName);
				tFightMoves[moveId].animId = (AnimationId)animAssoc->animId;
			} else {
				tFightMoves[moveId].animId = ANIM_WALK;
			}
		}
		moveId++;
	}
}

// --MIAMI: Done
void
CPed::SetInvestigateEvent(eEventType event, CVector2D pos, float distanceToCountDone, uint16 time, float angle)
{
	if (!IsPedInControl() || CharCreatedBy == MISSION_CHAR)
		return;

	SetStoredState();
	bFindNewNodeAfterStateRestore = false;
	SetPedState(PED_INVESTIGATE);
	m_standardTimer = CTimer::GetTimeInMilliseconds() + time;
	m_eventType = event;
	m_eventOrThreat = pos;
	m_distanceToCountSeekDone = distanceToCountDone;
	m_fAngleToEvent = angle;

	if (m_eventType >= EVENT_ICECREAM)
		m_lookTimer = 0;
	else
		CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_HANDSCOWER, 4.0f);

}

// --MIAMI: Done
void
CPed::InvestigateEvent(void)
{
	CAnimBlendAssociation *animAssoc;
	AnimationId animToPlay;
	AssocGroupId animGroup;

	if (m_nWaitState == WAITSTATE_TURN180)
		return;

	if (CTimer::GetTimeInMilliseconds() > m_standardTimer) {

		if (m_standardTimer) {
			if (m_eventType < EVENT_UNK)
				SetWaitState(WAITSTATE_TURN180, nil);

			m_standardTimer = 0;
		} else {
			ClearInvestigateEvent();
		}
		return;
	}

	CVector2D vecDist = m_eventOrThreat - GetPosition();
	float distSqr = vecDist.MagnitudeSqr();
	if (sq(m_distanceToCountSeekDone) >= distSqr) {

		m_fRotationDest = CGeneral::GetRadianAngleBetweenPoints(vecDist.x, vecDist.y, 0.0f, 0.0f);
		SetMoveState(PEDMOVE_STILL);

		switch (m_eventType) {
			case EVENT_DEAD_PED:
			case EVENT_HIT_AND_RUN:
			case EVENT_HIT_AND_RUN_COP:

				if (CTimer::GetTimeInMilliseconds() > m_lookTimer) {
					animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_ROAD_CROSS);

					if (animAssoc) {
						animAssoc->blendDelta = -8.0f;
						animAssoc->flags |= ASSOC_DELETEFADEDOUT;
						if (m_pEventEntity)
							SetLookFlag(m_pEventEntity, true);

						SetLookTimer(CGeneral::GetRandomNumberInRange(1500, 4000));

					} else if (CGeneral::GetRandomNumber() & 3) {
						ClearLookFlag();
						CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_ROAD_CROSS, 4.0f);

						SetLookTimer(CGeneral::GetRandomNumberInRange(1000, 2500));
						Say(SOUND_PED_CHAT_EVENT);

					} else {
						ClearInvestigateEvent();
					}
				}
				break;
			case EVENT_FIRE:
			case EVENT_EXPLOSION:

				if (bHasACamera && CTimer::GetTimeInMilliseconds() > m_lookTimer) {
					animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_IDLE_CAM);
					if (!animAssoc)
						animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_IDLE_STANCE);

					if (animAssoc && animAssoc->animId == ANIM_IDLE_CAM) {
						CAnimManager::BlendAnimation(GetClump(), m_animGroup, ANIM_IDLE_STANCE, 4.0f);
						SetLookTimer(CGeneral::GetRandomNumberInRange(1000, 2500));

					} else if (CGeneral::GetRandomNumber() & 3) {
						CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_IDLE_CAM, 4.0f);
						SetLookTimer(CGeneral::GetRandomNumberInRange(2500, 5000));
						if (!CGame::germanGame)
							Say(SOUND_PED_CHAT_EVENT);

					} else {
						m_standardTimer = 0;
					}

				} else if (CTimer::GetTimeInMilliseconds() > m_lookTimer) {
					animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_IDLE_STANCE);
					if (!animAssoc)
						animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_IDLE_HBHB);

					if (!animAssoc)
						animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_XPRESS_SCRATCH);

					if (animAssoc && animAssoc->animId == ANIM_IDLE_STANCE) {
						if (CGeneral::GetRandomNumber() & 1)
							animToPlay = ANIM_IDLE_HBHB;
						else
							animToPlay = ANIM_XPRESS_SCRATCH;

						CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, animToPlay, 4.0f);
						SetLookTimer(CGeneral::GetRandomNumberInRange(1500, 4000));

					} else if (animAssoc && animAssoc->animId == ANIM_IDLE_HBHB) {
						animAssoc->blendDelta = -8.0f;
						animAssoc->flags |= ASSOC_DELETEFADEDOUT;
						if (CGeneral::GetRandomNumber() & 1) {
							animToPlay = ANIM_IDLE_STANCE;
							animGroup = m_animGroup;
						} else {
							animToPlay = ANIM_XPRESS_SCRATCH;
							animGroup = ASSOCGRP_STD;
						}

						CAnimManager::BlendAnimation(GetClump(), animGroup, animToPlay, 4.0f);
						SetLookTimer(CGeneral::GetRandomNumberInRange(1000, 2500));

					} else {
						if (CGeneral::GetRandomNumber() & 1) {
							animToPlay = ANIM_IDLE_STANCE;
							animGroup = m_animGroup;
						} else {
							animToPlay = ANIM_IDLE_HBHB;
							animGroup = ASSOCGRP_STD;
						}

						CAnimManager::BlendAnimation(GetClump(), animGroup, animToPlay, 4.0f);
						SetLookTimer(CGeneral::GetRandomNumberInRange(1000, 2500));
					}
					if (!CGame::germanGame)
						Say(SOUND_PED_CHAT_EVENT);
				}
				break;
			case EVENT_ICECREAM:
			case EVENT_SHOPSTALL:

				m_fRotationDest = m_fAngleToEvent;
				if (CTimer::GetTimeInMilliseconds() > m_lookTimer) {

					if (m_lookTimer) {
						animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_ROAD_CROSS);

						if (animAssoc) {
							animAssoc->blendDelta = -8.0f;
							animAssoc->flags |= ASSOC_DELETEFADEDOUT;
							if (m_eventType == EVENT_ICECREAM)
								animToPlay = ANIM_IDLE_CHAT;
							else
								animToPlay = ANIM_XPRESS_SCRATCH;
							CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, animToPlay,4.0f);
							SetLookTimer(CGeneral::GetRandomNumberInRange(2000, 5000));

						} else {
							animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_IDLE_CHAT);
							if (animAssoc) {
								animAssoc->blendDelta = -8.0f;
								animAssoc->flags |= ASSOC_DELETEFADEDOUT;
								ClearInvestigateEvent();
							} else {
								animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_XPRESS_SCRATCH);
								if (animAssoc) {
									animAssoc->blendDelta = -8.0f;
									animAssoc->flags |= ASSOC_DELETEFADEDOUT;
								}
								ClearInvestigateEvent();
							}
						}
					} else {
						CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_ROAD_CROSS, 4.0f);
						SetLookTimer(CGeneral::GetRandomNumberInRange(1000, 2500));
					}
				}
				break;
			default:
				return;
		}
	} else {
		m_vecSeekPos.x = m_eventOrThreat.x;
		m_vecSeekPos.y = m_eventOrThreat.y;
		m_vecSeekPos.z = GetPosition().z;
		Seek();

		if (m_eventType < EVENT_ICECREAM) {
			if (sq(5.0f + m_distanceToCountSeekDone) < distSqr) {
				SetMoveState(PEDMOVE_RUN);
				return;
			}
		}
		if (m_eventType <= EVENT_EXPLOSION || m_eventType >= EVENT_SHOPSTALL) {
			SetMoveState(PEDMOVE_WALK);
			return;
		}
		if (distSqr > sq(1.2f)) {
			SetMoveState(PEDMOVE_WALK);
			return;
		}

		bool willStandStill = false;
		for (int i = 0; i < m_numNearPeds; i++) {
			if ((m_eventOrThreat - m_nearPeds[i]->GetPosition()).MagnitudeSqr() < sq(0.4f)) {
				SetMoveState(PEDMOVE_STILL);
				willStandStill = true;
				break;
			}
		}

		if (!willStandStill)
			SetMoveState(PEDMOVE_WALK);
	}
}

// --MIAMI: Done
void
CPed::ClearInvestigateEvent(void)
{
	CAnimBlendAssociation *animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_ROAD_CROSS);
	if (!animAssoc)
		animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_XPRESS_SCRATCH);
	if (!animAssoc)
		animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_IDLE_HBHB);
	if (!animAssoc)
		animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_IDLE_CHAT);
	if (animAssoc) {
		animAssoc->blendDelta = -8.0f;
		animAssoc->flags |= ASSOC_DELETEFADEDOUT;
	}
	if (m_eventType > EVENT_EXPLOSION)
		m_standardTimer = CTimer::GetTimeInMilliseconds() + 15000;

	bGonnaInvestigateEvent = false;
	m_pEventEntity = nil;
	ClearLookFlag();
	RestorePreviousState();
	if(m_nMoveState == PEDMOVE_NONE || m_nMoveState == PEDMOVE_STILL)
		SetMoveState(PEDMOVE_WALK);
}

// --MIAMI: Done
bool
CPed::InflictDamage(CEntity *damagedBy, eWeaponType method, float damage, ePedPieceTypes pedPiece, uint8 direction)
{
	CPlayerPed *player = FindPlayerPed();
	float dieDelta = 4.0f;
	float dieSpeed = 0.0f;
	AnimationId dieAnim = ANIM_KO_SHOT_FRONT1;
	bool headShot = false;
	bool willLinger = false;
	int random;

	if (damagedBy == FindPlayerPed() && damagedBy != this && damage > 3.0f)
		++CWorld::Players[CWorld::PlayerInFocus].m_nHavocLevel;

	if (player == this) {
		if (!player->m_bCanBeDamaged)
			return false;

		if (damagedBy && damagedBy->IsPed() && ((CPed*)damagedBy)->m_nPedType == PEDTYPE_GANG7)
			return false;

		if ((method == WEAPONTYPE_FLAMETHROWER || method == WEAPONTYPE_MOLOTOV) && CWorld::Players[CWorld::PlayerInFocus].m_bFireproof)
			return false;

		player->AnnoyPlayerPed(false);
	}

	if (DyingOrDead())
		return false;

	if (method == WEAPONTYPE_DROWNING && !bDrownsInWater)
		return false;

	if (!bUsesCollision && (!bInVehicle || m_nPedState != PED_DRIVING) && method != WEAPONTYPE_DROWNING)
		return false;

	if (bOnlyDamagedByPlayer && damagedBy != player && damagedBy != FindPlayerVehicle() &&
		method != WEAPONTYPE_DROWNING && method != WEAPONTYPE_EXPLOSION)
		return false;

	float healthImpact;
	if (IsPlayer())
		healthImpact = damage * 0.33f;
	else
		healthImpact = damage * m_pedStats->m_defendWeakness;

	if (!IsPlayer() &&
		(method == WEAPONTYPE_SCREWDRIVER || method == WEAPONTYPE_KNIFE || (method >= WEAPONTYPE_CLEAVER && method <= WEAPONTYPE_CHAINSAW)))
		m_bleedCounter = 200;

	bool detectDieAnim = true;
	if (m_nPedState == PED_GETUP) {
		if (!IsPedHeadAbovePos(-0.3f)) {
			if (RpAnimBlendClumpGetFirstAssociation(GetClump(), ASSOC_FRONTAL))
				dieAnim = ANIM_FLOOR_HIT_F;
			else
				dieAnim = ANIM_FLOOR_HIT;
			dieDelta *= 2.0f;
			dieSpeed = 0.5f;
			detectDieAnim = false;
		}
	} else if (m_nPedState == PED_FALL) {
		CAnimBlendAssociation *fallAssoc = RpAnimBlendClumpGetFirstAssociation(GetClump(), ASSOC_PARTIAL);
		if (!fallAssoc || fallAssoc->IsRunning()) {
			if (fallAssoc && fallAssoc->blendDelta >= 0.0f)
				dieAnim = NUM_STD_ANIMS;
			else
				dieAnim = ANIM_KO_SHOT_FRONT1;
		} else {
			if (fallAssoc->flags & ASSOC_FRONTAL)
				dieAnim = ANIM_FLOOR_HIT_F;
			else
				dieAnim = ANIM_FLOOR_HIT;

			dieDelta *= 2.0f;
			dieSpeed = 0.5f;
		}
		detectDieAnim = false;
	}

	if (detectDieAnim) {
		switch (method) {
			case WEAPONTYPE_UNARMED:
			case WEAPONTYPE_BRASSKNUCKLE:
				if (bMeleeProof)
					return false;

				if (m_nPedState == PED_FALL) {
					if (IsPedHeadAbovePos(-0.3f)) {
						dieAnim = NUM_STD_ANIMS;
					} else {
						if (RpAnimBlendClumpGetFirstAssociation(GetClump(), ASSOC_FRONTAL))
							dieAnim = ANIM_FLOOR_HIT_F;
						else
							dieAnim = ANIM_FLOOR_HIT;
						dieDelta = dieDelta * 2.0f;
						dieSpeed = 0.5f;
					}
				} else {
					switch (direction) {
						case 0:
							dieAnim = ANIM_KO_SKID_FRONT;
							break;
						case 1:
							dieAnim = ANIM_KO_SPIN_R;
							break;
						case 2:
							dieAnim = ANIM_KO_SKID_BACK;
							break;
						case 3:
							dieAnim = ANIM_KO_SPIN_L;
							break;
						default:
							break;
					}
				}
				break;
			case WEAPONTYPE_SCREWDRIVER:
			case WEAPONTYPE_GOLFCLUB:
			case WEAPONTYPE_NIGHTSTICK:
			case WEAPONTYPE_KNIFE:
			case WEAPONTYPE_BASEBALLBAT:
			case WEAPONTYPE_HAMMER:
			case WEAPONTYPE_CLEAVER:
			case WEAPONTYPE_MACHETE:
			case WEAPONTYPE_KATANA:
			case WEAPONTYPE_CHAINSAW:
				if (bMeleeProof)
					return false;

				if (method != WEAPONTYPE_KATANA ||
					damagedBy != FindPlayerPed()
					|| FindPlayerPed()->m_nPedState != PED_FIGHT
					|| FindPlayerPed()->m_curFightMove != FIGHTMOVE_MELEE1 && FindPlayerPed()->m_curFightMove != FIGHTMOVE_MELEE2
					|| CGeneral::GetRandomNumber() & 3) {

					if (m_nPedState == PED_FALL) {
						if (IsPedHeadAbovePos(-0.3f)) {
							dieAnim = NUM_STD_ANIMS;
						} else {
							if (RpAnimBlendClumpGetFirstAssociation(GetClump(), ASSOC_FRONTAL))
								dieAnim = ANIM_FLOOR_HIT_F;
							else
								dieAnim = ANIM_FLOOR_HIT;
							dieDelta = dieDelta * 2.0f;
							dieSpeed = 0.5f;
						}
					} else if (damagedBy != FindPlayerPed() || FindPlayerPed()->m_curFightMove != FIGHTMOVE_MELEE2) {
						if (damagedBy != FindPlayerPed() || FindPlayerPed()->m_curFightMove != FIGHTMOVE_MELEE3) {
							switch (direction) {
								case 0:
									dieAnim = ANIM_KO_SKID_FRONT;
									break;
								case 1:
									dieAnim = ANIM_KO_SPIN_R;
									break;
								case 2:
									dieAnim = ANIM_KO_SKID_BACK;
									break;
								case 3:
									dieAnim = ANIM_KO_SPIN_L;
									break;
								default:
									break;
							}
						} else {
							dieAnim = ANIM_KO_SHOT_STOM;
						}
					} else {
						dieAnim = ANIM_KO_SHOT_FACE;
					}
				} else {
					dieAnim = ANIM_KO_SHOT_FACE;
					RemoveBodyPart(PED_HEAD, direction);
					headShot = true;
					willLinger = true;
				}
				break;
			case WEAPONTYPE_COLT45:
			case WEAPONTYPE_SHOTGUN:
			case WEAPONTYPE_STUBBY_SHOTGUN:
			case WEAPONTYPE_SPAS12_SHOTGUN:
			case WEAPONTYPE_TEC9:
			case WEAPONTYPE_UZI:
			case WEAPONTYPE_SILENCED_INGRAM:
			case WEAPONTYPE_MP5:
			case WEAPONTYPE_M4:
			case WEAPONTYPE_RUGER:
			case WEAPONTYPE_SNIPERRIFLE:
			case WEAPONTYPE_LASERSCOPE:
			case WEAPONTYPE_M60:
			case WEAPONTYPE_MINIGUN:
			case WEAPONTYPE_UZI_DRIVEBY:

				if (bBulletProof)
					return false;

				bool dontRemoveLimb;
				if (IsPlayer() || bNoCriticalHits)
					dontRemoveLimb = true;
				else if (method != WEAPONTYPE_M4 && method != WEAPONTYPE_RUGER && method != WEAPONTYPE_SNIPERRIFLE &&
						method != WEAPONTYPE_LASERSCOPE) {
					if (method == WEAPONTYPE_SHOTGUN)
						dontRemoveLimb = CGeneral::GetRandomNumber() & 7;
					else
						dontRemoveLimb = CGeneral::GetRandomNumber() & 15;
				} else
					dontRemoveLimb = false;

				if (dontRemoveLimb) {
					if (method == WEAPONTYPE_SHOTGUN) {
						switch (direction) {
							case 0:
								dieAnim = ANIM_KO_SKID_FRONT;
								break;
							case 1:
								dieAnim = ANIM_KO_SPIN_R;
								break;
							case 2:
								dieAnim = ANIM_KO_SKID_BACK;
								break;
							case 3:
								dieAnim = ANIM_KO_SPIN_L;
								break;
							default:
								break;
						}
					} else
						dieAnim = ANIM_KO_SHOT_FRONT1;

					willLinger = false;
				} else {
					switch (pedPiece) {
						case PEDPIECE_TORSO:
							willLinger = false;
							dieAnim = ANIM_KO_SHOT_FRONT1;
							break;
						case PEDPIECE_MID:
							willLinger = false;
							dieAnim = ANIM_KO_SHOT_STOM;
							break;
						case PEDPIECE_LEFTARM:
							dieAnim = ANIM_KO_SHOT_ARML;
							RemoveBodyPart(PED_UPPERARML, direction);
							willLinger = true;
							break;
						case PEDPIECE_RIGHTARM:
							dieAnim = ANIM_KO_SHOT_ARMR;
							RemoveBodyPart(PED_UPPERARMR, direction);
							willLinger = true;
							break;
						case PEDPIECE_LEFTLEG:
							dieAnim = ANIM_KO_SHOT_LEGL;
							RemoveBodyPart(PED_UPPERLEGL, direction);
							willLinger = true;
							break;
						case PEDPIECE_RIGHTLEG:
							dieAnim = ANIM_KO_SHOT_LEGR;
							RemoveBodyPart(PED_UPPERLEGR, direction);
							willLinger = true;
							break;
						case PEDPIECE_HEAD:
							dieAnim = ANIM_KO_SHOT_FACE;
							RemoveBodyPart(PED_HEAD, direction);
							headShot = true;
							willLinger = true;
							break;
						default:
							break;
					}
				}
				break;
			case WEAPONTYPE_GRENADE:
			case WEAPONTYPE_ROCKETLAUNCHER:
			case WEAPONTYPE_EXPLOSION:
				if (bExplosionProof)
					return false;

				if (CGame::nastyGame && !IsPlayer() && !bInVehicle &&
					1.0f + healthImpact > m_fArmour + m_fHealth) {

					random = CGeneral::GetRandomNumber();
					if (random & 1)
						RemoveBodyPart(PED_UPPERARML, direction);
					if (random & 2)
						RemoveBodyPart(PED_UPPERLEGR, direction);
					if (random & 4)
						RemoveBodyPart(PED_HEAD, direction);
					if (random & 8)
						RemoveBodyPart(PED_UPPERARMR, direction);
					if (random & 0x10)
						RemoveBodyPart(PED_UPPERLEGL, direction);
					if (bBodyPartJustCameOff)
						willLinger = true;
				}
				// fall through
			case WEAPONTYPE_MOLOTOV:
				if (bExplosionProof)
					return false;

				switch (direction) {
					case 0:
						dieAnim = ANIM_KO_SKID_FRONT;
						break;
					case 1:
						dieAnim = ANIM_KO_SPIN_R;
						break;
					case 2:
						dieAnim = ANIM_KO_SKID_BACK;
						break;
					case 3:
						dieAnim = ANIM_KO_SPIN_L;
						break;
					default:
						break;
				}
				break;
			case WEAPONTYPE_FLAMETHROWER:
				if (bFireProof)
					return false;

				dieAnim = ANIM_KO_SHOT_FRONT1;
				break;
			case WEAPONTYPE_RAMMEDBYCAR:
			case WEAPONTYPE_RUNOVERBYCAR:
				if (bCollisionProof)
					return false;

				random = CGeneral::GetRandomNumber() & 3;
				switch (random) {
					case 0:
						if ((pedPiece != PEDPIECE_LEFTARM || random <= 1)
							&& (pedPiece != PEDPIECE_MID || random != 1)) {
							if (pedPiece == PEDPIECE_RIGHTARM && random > 1
								|| pedPiece == PEDPIECE_MID && random == 2)
						
								dieAnim = ANIM_KO_SPIN_L;
							else
								dieAnim = ANIM_KO_SKID_FRONT;
						} else
							dieAnim = ANIM_KO_SPIN_R;

						break;
					case 1:
						if (m_nPedState == PED_DIVE_AWAY)
							dieAnim = ANIM_KD_LEFT;
						else
							dieAnim = ANIM_KO_SPIN_R;
						break;
					case 2:
						if ((pedPiece != PEDPIECE_LEFTARM || random <= 1)
							&& (pedPiece != PEDPIECE_MID || random != 1)) {
							if ((pedPiece != PEDPIECE_RIGHTARM || random <= 1)
								&& (pedPiece != PEDPIECE_MID || random != 2)) {
								dieAnim = ANIM_KO_SKID_BACK;
							} else {
								dieAnim = ANIM_KD_RIGHT;
							}
						} else
							dieAnim = ANIM_KD_LEFT;
						break;
					case 3:
						if (m_nPedState == PED_DIVE_AWAY)
							dieAnim = ANIM_KD_RIGHT;
						else
							dieAnim = ANIM_KO_SPIN_L;
						break;
					default:
						break;
				}
				if (damagedBy && pedPiece != PEDPIECE_TORSO) {
					CVehicle *vehicle = (CVehicle*)damagedBy;
					if (method == WEAPONTYPE_RAMMEDBYCAR) {
						float vehSpeed = vehicle->m_vecMoveSpeed.Magnitude();
						dieDelta = 8.0f * vehSpeed + 4.0f;
					} else {
						float vehSpeed = vehicle->m_vecMoveSpeed.Magnitude();
						dieDelta = 12.0f * vehSpeed + 4.0f;
						dieSpeed = 16.0f * vehSpeed + 1.0f;
					}
				}
				break;
			case WEAPONTYPE_DROWNING:
				dieAnim = ANIM_DROWN;
				break;
			case WEAPONTYPE_FALL:
				if (bCollisionProof)
					return false;

				switch (direction) {
					case 0:
						dieAnim = ANIM_KO_SKID_FRONT;
						break;
					case 1:
						dieAnim = ANIM_KO_SPIN_R;
						break;
					case 2:
						dieAnim = ANIM_KO_SKID_BACK;
						break;
					case 3:
						dieAnim = ANIM_KO_SPIN_L;
						break;
					default:
						break;
				}
				break;
			default:
				break;
		}
	}

	if (m_fArmour != 0.0f && method != WEAPONTYPE_DROWNING) {
		if (player == this)
			CWorld::Players[CWorld::PlayerInFocus].m_nTimeLastArmourLoss = CTimer::GetTimeInMilliseconds();

		if (healthImpact < m_fArmour) {
			m_fArmour = m_fArmour - healthImpact;
			healthImpact = 0.0f;
		} else {
			healthImpact = healthImpact - m_fArmour;
			m_fArmour = 0.0f;
		}
	}

	if (healthImpact != 0.0f) {
		if (player == this)
			CWorld::Players[CWorld::PlayerInFocus].m_nTimeLastHealthLoss = CTimer::GetTimeInMilliseconds();

		m_lastWepDam = method;
		m_lastDamEntity = damagedBy;
	}

	if (method == WEAPONTYPE_FALL) {
		if (RpAnimBlendClumpGetAssociation(GetClump(), ANIM_CAR_ROLLOUT_LHS)) {
			if (m_fHealth >= 1.0 && m_fHealth - healthImpact < 5.0f) {
				m_fHealth = Min(m_fHealth, 5.0f);
				return false;
			}
		}
	}

	if (m_fHealth - healthImpact >= 1.0f && !willLinger) {
		m_fHealth -= healthImpact;
		return false;
	}

	if (bInVehicle) {
		if (method != WEAPONTYPE_DROWNING) {
			if (m_pMyVehicle) {
				CVehicle* pVehicle = m_pMyVehicle;
				bool bDone = false;
				if (m_pMyVehicle->IsBike()) {
					m_fHealth = 0.0f;
					((CBike*)m_pMyVehicle)->KnockOffRider(method, direction, this, false);
					bDone = true;
				} else {
					if (m_pMyVehicle->GetVehicleAppearance() == VEHICLE_APPEARANCE_CAR) {
						if (m_pMyVehicle->pDriver == this) {
							if (m_pMyVehicle->GetStatus() == STATUS_SIMPLE) {
								m_pMyVehicle->SetStatus(STATUS_PHYSICS);
								CCarCtrl::SwitchVehicleToRealPhysics(m_pMyVehicle);
							}
							m_pMyVehicle->AutoPilot.m_nCarMission = MISSION_NONE;
							m_pMyVehicle->AutoPilot.m_nCruiseSpeed = 0;
							m_pMyVehicle->AutoPilot.m_nTempAction = TEMPACT_HANDBRAKESTRAIGHT;
							m_pMyVehicle->AutoPilot.m_nTimeTempAction = CTimer::GetTimeInMilliseconds() + 2000;
						}
					}
					if (m_pMyVehicle->CanPedExitCar(true)) {
						SetObjective(OBJECTIVE_LEAVE_CAR_AND_DIE, m_pMyVehicle);
					} else {
						m_fHealth = 0.0f;
						if (m_pMyVehicle && m_pMyVehicle->pDriver == this) {
							SetRadioStation();
							m_pMyVehicle->SetStatus(STATUS_ABANDONED);
						}
						SetDie(dieAnim, dieDelta, dieSpeed);

						if (damagedBy == FindPlayerPed() && damagedBy != this) {
							CWorld::Players[CWorld::PlayerInFocus].m_nHavocLevel += 10;
							CWorld::Players[CWorld::PlayerInFocus].m_fMediaAttention += 5.f;
						}
					}
				}
				for (int i = 0; i < ARRAY_SIZE(pVehicle->pPassengers); i++) {
					CPed* passenger = pVehicle->pPassengers[i];
					if (passenger && passenger != this && damagedBy)
						passenger->ReactToAttack(damagedBy);
				}

				CPed *driverOfVeh = pVehicle->pDriver;
				if (driverOfVeh && driverOfVeh != this && damagedBy)
					driverOfVeh->ReactToAttack(damagedBy);

				if (damagedBy == FindPlayerPed() || damagedBy && damagedBy == FindPlayerVehicle()) {
					CDarkel::RegisterKillByPlayer(this, method, headShot);
					m_threatEntity = FindPlayerPed();
				} else {
					CDarkel::RegisterKillNotByPlayer(this, method);
				}
				if (bDone)
					return true;
			}
			m_fHealth = 1.0f;
			return false;
		}
		m_fHealth = 0.0f;
		if (player == this)
			m_pMyVehicle->SetStatus(STATUS_PLAYER_DISABLED);

		SetDie(NUM_STD_ANIMS, 4.0f, 0.0f);
		return true;
	} else {
		m_fHealth = 0.0f;
		SetDie(dieAnim, dieDelta, dieSpeed);

		if (damagedBy == player || damagedBy && damagedBy == FindPlayerVehicle()) {
			CDarkel::RegisterKillByPlayer(this, method, headShot);
			CWorld::Players[CWorld::PlayerInFocus].m_nHavocLevel += 10;
			CWorld::Players[CWorld::PlayerInFocus].m_fMediaAttention += 5.f;
			m_threatEntity = player;
		} else {
			CDarkel::RegisterKillNotByPlayer(this, method);
		}
		if (method == WEAPONTYPE_DROWNING) {
			bIsInTheAir = false;
			if (FindPlayerPed() == this)
				CStats::TimesDrowned++;
		}

		return true;
	}
}

static RwObject*
SetPedAtomicVisibilityCB(RwObject* object, void* data)
{
	if (data == nil)
		RpAtomicSetFlags((RpAtomic*)object, 0);
	return object;
}

static RwFrame*
RecurseFrameChildrenVisibilityCB(RwFrame* frame, void* data)
{
	RwFrameForAllObjects(frame, SetPedAtomicVisibilityCB, data);
	RwFrameForAllChildren(frame, RecurseFrameChildrenVisibilityCB, nil);
	return frame;
}

static RwObject*
CloneAtomicToFrameCB(RwObject *frame, void *data)
{
	RpAtomic *newAtomic = RpAtomicClone((RpAtomic*)frame);
	RpAtomicSetFrame(newAtomic, (RwFrame*)data);
	RpClumpAddAtomic(flyingClumpTemp, newAtomic);
	CVisibilityPlugins::SetAtomicRenderCallback(newAtomic, nil);
	return frame;
}

static RwFrame*
RecurseFrameChildrenToCloneCB(RwFrame *frame, void *data)
{
	RwFrame *newFrame = RwFrameCreate();
	RwFrameAddChild((RwFrame*)data, newFrame);
	RwFrameTransform(newFrame, RwFrameGetMatrix(frame), rwCOMBINEREPLACE);
	RwFrameForAllObjects(frame, CloneAtomicToFrameCB, newFrame);
	RwFrameForAllChildren(frame, RecurseFrameChildrenToCloneCB, newFrame);
	return newFrame;
}

// --MIAMI: Done
void
CPed::RemoveBodyPart(PedNode nodeId, int8 direction)
{
	RwFrame *frame;
	CVector pos;

	frame = m_pFrames[nodeId]->frame;
	if (frame) {
		if (CGame::nastyGame) {
			if (CEntity::GetIsOnScreen()) {
				m_pedIK.GetComponentPosition(pos, nodeId);
				CParticle::AddParticle(PARTICLE_TEST, pos,
					CVector(0.0f, 0.0f, 0.0f),
					nil, 0.1f, 0, 0, 0, 0);

				for (int i = 0; i < 16; i++) {
					CParticle::AddParticle(PARTICLE_BLOOD_SMALL,
						pos,
						CVector(0.0f, 0.0f, 0.03f),
						nil, 0.0f, 0, 0, 0, 0);
				}
			}
			bBodyPartJustCameOff = true;
			m_bodyPartBleeding = nodeId;
		}
	} else {
		printf("Trying to remove ped component");
	}
}

// --MIAMI: Done
CObject*
CPed::SpawnFlyingComponent(int pedNode, int8 direction)
{
	// VC doesn't have detachable limbs :shrug:
	return nil;
}

// --MIAMI: Done
// III leftover and unused
void
CPed::ApplyHeadShot(eWeaponType weaponType, CVector pos, bool evenOnPlayer)
{
	CVector pos2 = CVector(
		pos.x,
		pos.y,
		pos.z + 0.1f
	);

	if (!IsPlayer() || evenOnPlayer) {
		++CStats::HeadsPopped;

		// BUG: This condition will always return true. Even fixing it won't work, because these states are unused.
		// if (m_nPedState != PED_PASSENGER || m_nPedState != PED_TAXI_PASSENGER) {
			SetDie();
		// }

		bBodyPartJustCameOff = true;
		m_nPedStateTimer = CTimer::GetTimeInMilliseconds() + 150;

		RemoveBodyPart(PED_HEAD, 0);
		CParticle::AddParticle(PARTICLE_TEST, pos2,
			CVector(0.0f, 0.0f, 0.0f), nil, 0.2f, 0, 0, 0, 0);

		if (CEntity::GetIsOnScreen()) {
			for(int i=0; i < 32; i++) {
				CParticle::AddParticle(PARTICLE_BLOOD_SMALL,
					pos2, CVector(0.0f, 0.0f, 0.03f),
					nil, 0.0f, 0, 0, 0, 0);
			}

			for (int i = 0; i < 16; i++) {
				CParticle::AddParticle(PARTICLE_DEBRIS2,
					pos2,
					CVector(0.0f, 0.0f, 0.01f),
					nil, 0.0f, 0, 0, 0, 0);
			}
		}
	}
}

// --MIAMI: Done
bool
CPed::IsPedHeadAbovePos(float zOffset)
{
	return zOffset + GetPosition().z < GetNodePosition(PED_HEAD).z;
}

// --MIAMI: Done
bool
CPed::PlacePedOnDryLand(void)
{
	float waterLevel = 0.0f;
	CEntity *foundEnt = nil;
	CColPoint foundCol;
	float foundColZ;

	CWaterLevel::GetWaterLevelNoWaves(GetPosition().x, GetPosition().y, GetPosition().z, &waterLevel);

	CVector potentialGround = GetPosition();
	potentialGround.z = waterLevel;

	if (!CWorld::TestSphereAgainstWorld(potentialGround, 5.0f, nil, true, false, false, false, false, false))
		return false;

	CVector potentialGroundDist = gaTempSphereColPoints[0].point - GetPosition();
	potentialGroundDist.z = 0.0f;
	potentialGroundDist.Normalise();

	CVector posToCheck = 0.5f * potentialGroundDist + gaTempSphereColPoints[0].point;
	posToCheck.z = 3.0f + waterLevel;

	if (CWorld::ProcessVerticalLine(posToCheck, waterLevel - 1.0f, foundCol, foundEnt, true, true, false, true, false, false, nil)) {
		foundColZ = foundCol.point.z;
		if (foundColZ >= waterLevel) {
			posToCheck.z = 0.8f + foundColZ;
			SetPosition(posToCheck);
			bIsStanding = true;
			bWasStanding = true;
			return true;
		}
	}

	posToCheck = 5.0f * potentialGroundDist + GetPosition();
	posToCheck.z = 3.0f + waterLevel;

	if (!CWorld::ProcessVerticalLine(posToCheck, waterLevel - 1.0f, foundCol, foundEnt, true, true, false, true, false, false, nil))
		return false;

	foundColZ = foundCol.point.z;
	if (foundColZ < waterLevel)
		return false;

	posToCheck.z = 0.8f + foundColZ;
	SetPosition(posToCheck);
	bIsStanding = true;
	bWasStanding = true;
	return true;
}

// --MIAMI: Done
void
CPed::CollideWithPed(CPed *collideWith)
{
	CAnimBlendAssociation *animAssoc;
	AnimationId animToPlay;

	bool weAreMissionChar = CharCreatedBy == MISSION_CHAR;
	bool heIsMissionChar = collideWith->CharCreatedBy == MISSION_CHAR;
	CVector posDiff = collideWith->GetPosition() - GetPosition();
	int waitTime = 0;

	if (weAreMissionChar || !collideWith->IsPlayer() || collideWith->m_nPedState != PED_MAKE_CALL) {
		if (m_nWaitState == WAITSTATE_SUN_BATHE_IDLE) {
			SetGetUp();
			return;
		}
		if (collideWith->m_nWaitState == WAITSTATE_SUN_BATHE_IDLE) {
			collideWith->SetGetUp();
			return;
		}

		bool weDontLookToHim = DotProduct(posDiff, GetForward()) > 0.0f;
		bool heLooksToUs = DotProduct(posDiff, collideWith->GetForward()) < 0.0f;

		if (m_nMoveState != PEDMOVE_NONE && m_nMoveState != PEDMOVE_STILL) {

			if ((!IsPlayer() || ((CPlayerPed*)this)->m_fMoveSpeed <= 1.8f)
				&& (IsPlayer() || heIsMissionChar && weAreMissionChar || m_nMoveState != PEDMOVE_RUN && m_nMoveState != PEDMOVE_SPRINT
					|| m_objective == OBJECTIVE_FOLLOW_CHAR_IN_FORMATION && m_pedInObjective == collideWith
					|| collideWith->m_objective == OBJECTIVE_FOLLOW_CHAR_IN_FORMATION && collideWith->m_pedInObjective == this		
					)) {

				if (m_objective != OBJECTIVE_FOLLOW_CHAR_IN_FORMATION && m_objective != OBJECTIVE_GOTO_CHAR_ON_FOOT) {

					if (CTimer::GetTimeInMilliseconds() > m_nPedStateTimer) {

						if (heIsMissionChar || !weAreMissionChar && collideWith->m_nMoveState != PEDMOVE_STILL) {

							if (weAreMissionChar && (m_nPedState == PED_SEEK_POS || m_nPedState == PED_SEEK_ENTITY)) {

								if (collideWith->m_nMoveState != PEDMOVE_STILL
									&& (!collideWith->IsPlayer() || collideWith->IsPlayer() && CPad::GetPad(0)->ArePlayerControlsDisabled())) {
									float seekPosDist = (GetPosition() - m_vecSeekPos).MagnitudeSqr2D();
									float heAndSeekPosDist = (collideWith->GetPosition() - m_vecSeekPos).MagnitudeSqr2D();

									if (seekPosDist <= heAndSeekPosDist) {
										waitTime = 1000;
										collideWith->SetWaitState(WAITSTATE_CROSS_ROAD_LOOK, &waitTime);
										collideWith->m_nPedStateTimer = CTimer::GetTimeInMilliseconds() + waitTime;
									} else {
										waitTime = 500;
										SetWaitState(WAITSTATE_CROSS_ROAD_LOOK, &waitTime);
										m_nPedStateTimer = CTimer::GetTimeInMilliseconds() + waitTime;
									}
								} else if (collideWith->m_nMoveState == PEDMOVE_STILL) {
									SetDirectionToWalkAroundObject(collideWith);
								}
							} else {
								if (FindPlayerPed() != m_pedInObjective
									|| m_objective != OBJECTIVE_KILL_CHAR_ANY_MEANS && m_objective != OBJECTIVE_KILL_CHAR_ON_FOOT
									|| collideWith == m_pedInObjective) {

									if (weAreMissionChar || m_pedStats->m_fear <= 100 - collideWith->m_pedStats->m_temper
										|| (collideWith->IsPlayer() || collideWith->m_nMoveState == PEDMOVE_NONE || collideWith->m_nMoveState == PEDMOVE_STILL) &&
										(!collideWith->IsPlayer() || ((CPlayerPed*)collideWith)->m_fMoveSpeed <= 1.0f)) {
										SetDirectionToWalkAroundObject(collideWith);
										if (!weAreMissionChar)
											Say(SOUND_PED_CHAT);
									} else {
										SetEvasiveStep(collideWith, 2);
									}
								} else if (collideWith->m_nMoveState != PEDMOVE_STILL && GetWeapon()->IsTypeMelee()
									&& collideWith->m_pedInObjective == m_pedInObjective) {

									int colliderIndexAtPlayersKillList = -1;
									int ourIndexAtPlayersKillList = -1;
									for (int i = 0; i < ARRAY_SIZE(((CPlayerPed*)m_pedInObjective)->m_pMeleeList); i++) {
										CPed *pedInKillList = ((CPlayerPed*)m_pedInObjective)->m_pMeleeList[i];
										if (pedInKillList == this) {
											ourIndexAtPlayersKillList = i;
										} else if (pedInKillList == collideWith) {
											colliderIndexAtPlayersKillList = i;
										}
									}
									bool weAreCloserToTargetThenCollider = false;
									if ((GetPosition() - m_vecSeekPos).MagnitudeSqr2D() < (collideWith->GetPosition() - m_vecSeekPos).MagnitudeSqr2D())
										weAreCloserToTargetThenCollider = true;

									if (ourIndexAtPlayersKillList > 0 && !weAreCloserToTargetThenCollider) {
										if (colliderIndexAtPlayersKillList > 0) {
											int time = 300;
											SetWaitState(WAITSTATE_CROSS_ROAD_LOOK, &time);
											m_nPedStateTimer = CTimer::GetTimeInMilliseconds() + time;

										} else if (collideWith->m_pedInObjective == FindPlayerPed()) {
											((CPlayerPed*)m_pedInObjective)->RemovePedFromMeleeList(this);
											int time = 500;
											SetWaitState(WAITSTATE_CROSS_ROAD_LOOK, &time);
											m_nPedStateTimer = CTimer::GetTimeInMilliseconds() + time;
										}
									} else if (!weAreCloserToTargetThenCollider) {
										int time = 300;
										SetWaitState(WAITSTATE_CROSS_ROAD_LOOK, &time);
										m_nPedStateTimer = CTimer::GetTimeInMilliseconds() + time;
									} 
								} else {
									SetDirectionToWalkAroundObject(collideWith);
								}
							}
						} else {
							if (m_pedStats->m_temper <= m_pedStats->m_fear
								|| GetWeapon()->m_eWeaponType != WEAPONTYPE_UNARMED
								|| weAreMissionChar
								|| collideWith->m_nPedType == PEDTYPE_CIVFEMALE
								|| collideWith->m_nPedType == m_nPedType
								|| collideWith->GetWeapon()->m_eWeaponType != WEAPONTYPE_UNARMED) {
								SetDirectionToWalkAroundObject(collideWith);
								Say(SOUND_PED_CHAT);
							} else {
								TurnBody();
								SetAttack(collideWith);
								m_fRotationCur = 0.3f + m_fRotationCur;
								m_fRotationDest = m_fRotationCur;
							}
							m_nPedStateTimer = CTimer::GetTimeInMilliseconds() + CGeneral::GetRandomNumberInRange(250, 450);
						}
					}
				} else {
					if (m_pedInObjective && (collideWith == m_pedInObjective || collideWith->m_pedInObjective == m_pedInObjective) && CTimer::GetTimeInMilliseconds() > m_nPedStateTimer) {
						if (heLooksToUs) {
							SetEvasiveStep(collideWith, 1);
							m_nPedStateTimer = CTimer::GetTimeInMilliseconds() + 3000;
						}
					} else if (weDontLookToHim && IsPedInControl()) {

						if (m_pedStats != collideWith->m_pedStats) {

							if (collideWith->m_pedStats->m_fear <= 100 - m_pedStats->m_temper || collideWith->IsPlayer()
								|| CTimer::GetTimeInMilliseconds() < m_nPedStateTimer) {

								if (collideWith->IsPlayer()) {
									// He's on our right side
									if (DotProduct(posDiff,GetRight()) <= 0.0f)
										m_fRotationCur -= m_headingRate;
									else
										m_fRotationCur += m_headingRate;
								} else {
									// He's on our right side
									if (DotProduct(posDiff, collideWith->GetRight()) <= 0.0f)
										collideWith->m_fRotationCur -= collideWith->m_headingRate;
									else
										collideWith->m_fRotationCur += collideWith->m_headingRate;
								}
							} else {
								SetLookFlag(collideWith, false);
								TurnBody();
								animAssoc = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_FIGHT_PPUNCH, 8.0f);
								animAssoc->flags |= ASSOC_FADEOUTWHENDONE;
								m_nPedStateTimer = CTimer::GetTimeInMilliseconds() + 2000;
								if (!heIsMissionChar) {
									CVector2D posDiff2D(posDiff);
									int direction = collideWith->GetLocalDirection(posDiff2D);
									collideWith->StartFightDefend(direction, HITLEVEL_HIGH, 5);
								}
							}
						}
					}
				}
			} else if (collideWith->m_pedStats->m_defendWeakness <= 1.5f || heIsMissionChar ||
				m_pedStats->m_defendWeakness <= collideWith->m_pedStats->m_defendWeakness) {

				// He looks us and we're not at his right side
				if (heLooksToUs && DotProduct(posDiff,collideWith->GetRight()) > 0.0f) {
					CVector moveForce = GetRight();
					moveForce.z += 0.1f;
					ApplyMoveForce(moveForce);
					if (collideWith->m_nMoveState != PEDMOVE_RUN && collideWith->m_nMoveState != PEDMOVE_SPRINT)
						animToPlay = ANIM_HIT_LEFT;
					else
						animToPlay = ANIM_SHOT_LEFT_PARTIAL;
				} else if (heLooksToUs) {
					CVector moveForce = GetRight() * -1.0f;
					moveForce.z += 0.1f;
					ApplyMoveForce(moveForce);
					if (collideWith->m_nMoveState != PEDMOVE_RUN && collideWith->m_nMoveState != PEDMOVE_SPRINT)
						animToPlay = ANIM_HIT_RIGHT;
					else
						animToPlay = ANIM_SHOT_RIGHT_PARTIAL;
				} else {
					if (collideWith->m_nMoveState != PEDMOVE_RUN && collideWith->m_nMoveState != PEDMOVE_SPRINT)
						animToPlay = ANIM_HIT_BACK;
					else
						animToPlay = ANIM_SHOT_BACK_PARTIAL;
				}

				if (collideWith->IsPedInControl() && CTimer::GetTimeInMilliseconds() > collideWith->m_nPedStateTimer) {
					animAssoc = CAnimManager::BlendAnimation(collideWith->GetClump(), ASSOCGRP_STD, animToPlay, 8.0f);
					animAssoc->flags |= ASSOC_FADEOUTWHENDONE;
					collideWith->m_nPedStateTimer = CTimer::GetTimeInMilliseconds() + 1000;
					if (m_nPedState == PED_ATTACK)
						DMAudio.PlayOneShot(m_audioEntityId, SOUND_FIGHT_46, 0.0f);
				}
			} else {
				// We're at his right side
				if (DotProduct(posDiff, collideWith->GetRight()) <= 0.0f) {
					CVector moveForce = GetRight() * -1.0f;
					moveForce.z += 0.1f;
					ApplyMoveForce(moveForce);
					if (heLooksToUs)
						animToPlay = ANIM_KO_SPIN_L;
					else
						animToPlay = ANIM_KD_RIGHT;
				} else {
					CVector moveForce = GetRight();
					moveForce.z += 0.1f;
					ApplyMoveForce(moveForce);
					if (heLooksToUs)
						animToPlay = ANIM_KO_SPIN_R;
					else
						animToPlay = ANIM_KD_LEFT;
				}

				if (m_nPedState == PED_ATTACK && collideWith->IsPedInControl())
					DMAudio.PlayOneShot(m_audioEntityId, SOUND_49, 0.0f);

				collideWith->SetFall(3000, animToPlay, 0);
			}
		} else {
			if (!IsPedInControl())
				return;

			if (collideWith->m_nMoveState == PEDMOVE_NONE || collideWith->m_nMoveState == PEDMOVE_STILL)
				return;

			if (m_nPedType != collideWith->m_nPedType || m_nPedType == PEDTYPE_CIVMALE || m_nPedType == PEDTYPE_CIVFEMALE) {

				if (!weAreMissionChar && heLooksToUs && m_pedStats->m_fear > 100 - collideWith->m_pedStats->m_temper) {

					if (CGeneral::GetRandomNumber() & 1 && CTimer::GetTimeInMilliseconds() < m_nPedStateTimer){
						SetEvasiveStep(collideWith, 2);
						m_nPedStateTimer = CTimer::GetTimeInMilliseconds() + 3000;
					} else if (collideWith->m_nMoveState > PEDMOVE_WALK) {
						waitTime = 2000;
						SetWaitState(WAITSTATE_PLAYANIM_DUCK, &waitTime);
					}
				}
			} else if (heLooksToUs
				&& collideWith->m_nPedState != PED_STEP_AWAY
				&& m_nPedState != PED_STEP_AWAY
				&& CTimer::GetTimeInMilliseconds() > m_nPedStateTimer) {

				SetEvasiveStep(collideWith, 1);
				m_nPedStateTimer = CTimer::GetTimeInMilliseconds() + 3000;
			}
		}

		if (IsPlayer()) {
			SetLookFlag(collideWith, true);
			SetLookTimer(800);
		}
	} else {
		bool isRunning = m_nMoveState == PEDMOVE_RUN || m_nMoveState == PEDMOVE_SPRINT;
		SetFindPathAndFlee(collideWith, 5000, !isRunning);
	}
}

// --MIAMI: Done
void
CPed::KillPedWithCar(CVehicle *car, float impulse)
{
	CVehicleModelInfo *vehModel;
	CColModel *vehColModel;
	uint8 damageDir;
	PedNode nodeToDamage;
	eWeaponType killMethod;

	if (m_nPedState == PED_FALL || m_nPedState == PED_DIE) {
		if (!m_pCollidingEntity || car->GetStatus() == STATUS_PLAYER)
			m_pCollidingEntity = car;
		return;
	}

	if (m_nPedState == PED_DEAD)
		return;

	if (m_pCurSurface) {
		if (m_pCurSurface->IsVehicle() && (((CVehicle*)m_pCurSurface)->IsBoat()|| IsPlayer()))
			return;
	}

	CVector distVec = GetPosition() - car->GetPosition();

	if ((impulse > 12.0f || car->GetModelIndex() == MI_TRAIN) && !IsPlayer()) {
		nodeToDamage = PED_TORSO;
		killMethod = WEAPONTYPE_RAMMEDBYCAR;
		uint8 randVal = CGeneral::GetRandomNumber() & 3;

		if (car == FindPlayerVehicle()) {
			float carSpeed = car->m_vecMoveSpeed.Magnitude();
			uint8 shakeFreq;
			if (100.0f * carSpeed * 2000.0f / car->m_fMass + 80.0f <= 250.0f) {
				shakeFreq = 100.0f * carSpeed * 2000.0f / car->m_fMass + 80.0f;
			} else {
				shakeFreq = 250.0f;
			}
			CPad::GetPad(0)->StartShake(40000 / shakeFreq, shakeFreq);
		}
		bIsStanding = false;
		damageDir = GetLocalDirection(-m_vecMoveSpeed);
		vehModel = (CVehicleModelInfo *)CModelInfo::GetModelInfo(car->GetModelIndex());
		vehColModel = vehModel->GetColModel();
		float carRightAndDistDotProd = DotProduct(distVec, car->GetRight());

		if (car->GetModelIndex() == MI_TRAIN) {
			killMethod = WEAPONTYPE_RUNOVERBYCAR;
			nodeToDamage = PED_HEAD;
			m_vecMoveSpeed = 0.9f * car->m_vecMoveSpeed;
			m_vecMoveSpeed.z = 0.0f;
			if (damageDir == 1 || damageDir == 3)
				damageDir = 2;
			if (CGame::nastyGame)
				DMAudio.PlayOneShot(m_audioEntityId, SOUND_SPLATTER, 0.0f);

		// Car doesn't look to us
		} else if (DotProduct(car->m_vecMoveSpeed, car->GetForward()) >= 0.0f){

			if (0.99f * vehColModel->boundingBox.max.x < Abs(carRightAndDistDotProd)) {

				// We're at the right of the car
				if (carRightAndDistDotProd <= 0.0f)
					nodeToDamage = PED_UPPERARML;
				else
					nodeToDamage = PED_UPPERARMR;

				if (Abs(DotProduct(distVec, car->GetForward())) < 0.85f * vehColModel->boundingBox.max.y) {
					killMethod = WEAPONTYPE_RUNOVERBYCAR;
					m_vecMoveSpeed = 0.9f * car->m_vecMoveSpeed;
					m_vecMoveSpeed.z = 0.0f;
					if (damageDir == 1 || damageDir == 3)
						damageDir = 2;
					if (CGame::nastyGame)
						DMAudio.PlayOneShot(m_audioEntityId, SOUND_SPLATTER, 0.0f);

				}
			} else {
				float carFrontAndDistDotProd = DotProduct(distVec, car->GetForward());

				// carFrontAndDistDotProd <= 0.0 car looks to us
				if ((carFrontAndDistDotProd <= 0.1 || randVal <= 1) && randVal != 0) {
					killMethod = WEAPONTYPE_RUNOVERBYCAR;
					nodeToDamage = PED_HEAD;
					m_vecMoveSpeed = 0.9f * car->m_vecMoveSpeed;
					m_vecMoveSpeed.z = 0.0f;
					if (damageDir == 1 || damageDir == 3)
						damageDir = 2;

					if (CGame::nastyGame)
						DMAudio.PlayOneShot(m_audioEntityId, SOUND_SPLATTER, 0.0f);

				} else {
					nodeToDamage = PED_MID;
					float vehColMaxY = vehColModel->boundingBox.max.y;
					float vehColMinY = vehColModel->boundingBox.min.y;
					float vehColMaxZ = vehColModel->boundingBox.max.z;
					float carFrontZ = car->GetForward().z;
					float carHighestZ, carLength;

					if (carFrontZ < -0.2f) {
						// Highest point of car's back
						carHighestZ = (car->GetMatrix() * CVector(0.0f, vehColMinY, vehColMaxZ)).z;
						carLength = vehColMaxY - vehColMinY;

					} else if (carFrontZ > 0.1f) {
						// Highest point of car's front
						carHighestZ = (car->GetMatrix() * CVector(0.0f, vehColMaxY, vehColMaxZ)).z;
						float highestZDist = carHighestZ - GetPosition().z;
						if (highestZDist > 0.0f) {
							GetMatrix().GetPosition().z += 0.5f * highestZDist;
							carHighestZ += highestZDist * 0.25f;
						}
						carLength = vehColMaxY;

					} else {
						// Highest point of car's front
						carHighestZ = (car->GetMatrix() * CVector(0.0f, vehColMaxY, vehColMaxZ)).z;
						carLength = vehColMaxY;
					}

					float pedJumpSpeedToReachHighestZ = (carHighestZ - GetPosition().z) / (carLength / car->m_vecMoveSpeed.Magnitude());

					// TODO: What are we doing down here?
					float unknown = ((CGeneral::GetRandomNumber() % 256) * 0.002 + 1.5) * pedJumpSpeedToReachHighestZ;

					// After this point distVec isn't really distVec.
					distVec = car->m_vecMoveSpeed;
					distVec.Normalise();
					distVec *= 0.2 * unknown;

					if (damageDir != 1 && damageDir != 3)
						distVec.z += unknown;
					else
						distVec.z += 1.5f * unknown;

					m_vecMoveSpeed = distVec;
					damageDir += 2;
					if (damageDir > 3)
						damageDir = damageDir - 4;

					if (car->IsCar()) {
						CObject *bonnet = ((CAutomobile*)car)->RemoveBonnetInPedCollision();

						if (bonnet) {
							if (CGeneral::GetRandomNumber() & 1) {
								bonnet->m_vecMoveSpeed += Multiply3x3(car->GetMatrix(), CVector(0.1f, 0.0f, 0.5f));
							} else {
								bonnet->m_vecMoveSpeed += Multiply3x3(car->GetMatrix(), CVector(-0.1f, 0.0f, 0.5f));
							}
							CVector forceDir = car->GetUp() * 10.0f;
							bonnet->ApplyTurnForce(forceDir, car->GetForward());
						}
					}
				}
			}
		}

		if (car->pDriver) {
			CEventList::RegisterEvent((m_nPedType == PEDTYPE_COP ? EVENT_HIT_AND_RUN_COP : EVENT_HIT_AND_RUN), EVENT_ENTITY_PED, this, car->pDriver, 1000);
		}

		ePedPieceTypes pieceToDamage;
		switch (nodeToDamage) {
			case PED_HEAD:
				pieceToDamage = PEDPIECE_HEAD;
				break;
			case PED_UPPERARML:
				pieceToDamage = PEDPIECE_LEFTARM;
				break;
			case PED_UPPERARMR:
				pieceToDamage = PEDPIECE_RIGHTARM;
				break;
			default:
				pieceToDamage = PEDPIECE_MID;
				break;
		}
		InflictDamage(car, killMethod, 1000.0f, pieceToDamage, damageDir);

		if (DyingOrDead()
			&& bIsPedDieAnimPlaying && !m_pCollidingEntity) {
			m_pCollidingEntity = car;
		}
		if (nodeToDamage == PED_MID)
			bKnockedUpIntoAir = true;
		else
			bKnockedUpIntoAir = false;

		distVec.Normalise();

		distVec *= Min(car->m_fMass / 1400.0f, 1.0f);
		car->ApplyMoveForce(distVec * -100.0f);
		Say(SOUND_PED_DEFEND);

	} else if (m_vecDamageNormal.z < -0.8f && impulse > 3.0f
		|| impulse > 6.0f && (!IsPlayer() || impulse > 10.0f)) {

		bIsStanding = false;
		uint8 fallDirection = GetLocalDirection(-car->m_vecMoveSpeed);
		float damage;
		if (IsPlayer() && car->GetModelIndex() == MI_TRAIN)
			damage = 150.0f;
		else
			damage = 30.0f;
	
		InflictDamage(car, WEAPONTYPE_RAMMEDBYCAR, damage, PEDPIECE_TORSO, fallDirection);
		SetFall(1000, (AnimationId)(fallDirection + ANIM_KO_SKID_FRONT), true);

		if (OnGround() && !m_pCollidingEntity &&
			(!IsPlayer() || bHasHitWall || car->GetModelIndex() == MI_TRAIN || m_vecDamageNormal.z < -0.8f)) {

			m_pCollidingEntity = car;
		}

		bKnockedUpIntoAir = false;
		if (car->GetModelIndex() != MI_TRAIN && !bHasHitWall) {
			m_vecMoveSpeed = car->m_vecMoveSpeed * 0.75f;
		}
		m_vecMoveSpeed.z = 0.0f;
		distVec.Normalise();
		distVec *= Min(car->m_fMass / 1400.0f, 1.0f);
		car->ApplyMoveForce(distVec * -60.0f);
		Say(SOUND_PED_DEFEND);
	}

	if (IsGangMember()) {
		CPed *driver = car->pDriver;
		if (driver && driver->IsPlayer()
#ifdef FIX_BUGS
			&& (CharCreatedBy != MISSION_CHAR || bRespondsToThreats) && (!m_leader || m_leader != driver)
#endif
			) {
			RegisterThreatWithGangPeds(driver);
		}
	}
}

// --MIAMI: Done
void
CPed::DriveVehicle(void)
{
	if (bOffscreen)
		return;

	CVehicle *veh = m_pMyVehicle;
	if (veh->IsBike()) {
		CBike *bike = (CBike*)veh;
		float blendDelta = 1.0f;
		float targetUDLean = 0.0f;
		CAnimBlendAssociation *leftAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_BIKE_LEFT);
		CAnimBlendAssociation *rightAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_BIKE_RIGHT);
		CAnimBlendAssociation *stillAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_BIKE_STILL);
		CAnimBlendAssociation *fwdAssoc, *backAssoc;
		if (IsPlayer()) {
			fwdAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_BIKE_FWD);
			backAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_BIKE_BACK);
		}
		CAnimBlendAssociation *walkbackAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_BIKE_PUSHES);
		CAnimBlendAssociation *drivebyAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_BIKE_DRIVEBY_RHS);
		if (!drivebyAssoc)
			drivebyAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_BIKE_DRIVEBY_LHS);
		if (!drivebyAssoc)
			drivebyAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_BIKE_DRIVEBY_FT);

		float velocityFwdDotProd = DotProduct(bike->m_vecMoveSpeed, bike->GetForward());
		if (m_vecTurnSpeed.MagnitudeSqr() > 0.09f) {
			bike->KnockOffRider(WEAPONTYPE_FALL, 2, this, false);
			if (bike->pPassengers[0])
				bike->KnockOffRider(WEAPONTYPE_FALL, 2, bike->pPassengers[0], false);
			return;
		}
		if (!drivebyAssoc && Abs(velocityFwdDotProd) < 0.02f) {
			if (!stillAssoc || stillAssoc->blendAmount < 1.0 && stillAssoc->blendDelta <= 0.0) {
				stillAssoc = CAnimManager::BlendAnimation(GetClump(), bike->m_bikeAnimType, ANIM_BIKE_STILL, 2.0f);
			}
		} else {
			if (velocityFwdDotProd >= 0.0f) {
				if (stillAssoc && stillAssoc->blendDelta >= 0.0f)
					stillAssoc->blendDelta = -4.0f;
				if (walkbackAssoc && walkbackAssoc->blendDelta >= 0.0f)
					walkbackAssoc->blendDelta = -4.0f;
			} else {
				float maxReverseSpeed = bike->pHandling->Transmission.fMaxReverseVelocity;
				if (3.5f * maxReverseSpeed > velocityFwdDotProd && (bike->m_nWheelsOnGround || bike->GetUp().z < -0.5f)) {
					bike->KnockOffRider(WEAPONTYPE_FALL, 2, this, false);
					if (bike->pPassengers[0])
						bike->KnockOffRider(WEAPONTYPE_FALL, 2, bike->pPassengers[0], false);
					return;
				}
				if (bike->m_fGasPedal >= 0.0 || velocityFwdDotProd <= maxReverseSpeed * 1.5) {
					if (IsPlayer() && velocityFwdDotProd < maxReverseSpeed * 1.5)
						targetUDLean = -1.0f;

					if (stillAssoc && stillAssoc->blendDelta >= 0.0f)
						stillAssoc->blendDelta = -4.0f;

					if (walkbackAssoc && walkbackAssoc->blendDelta >= 0.0f) {
						walkbackAssoc->blendDelta = -4.0f;
					}
				} else if (!walkbackAssoc || walkbackAssoc->blendAmount < 1.0f && walkbackAssoc->blendDelta <= 0.0f) {
					walkbackAssoc = CAnimManager::BlendAnimation(GetClump(), bike->m_bikeAnimType, ANIM_BIKE_PUSHES, 4.0f);
				}
			}
		}
		if (stillAssoc)
			blendDelta -= Min(1.0f, CTimer::GetTimeStepNonClipped() * 0.02f * stillAssoc->blendDelta + stillAssoc->blendAmount);

		if (drivebyAssoc)
			blendDelta -= Min(blendDelta, CTimer::GetTimeStepNonClipped() * 0.02f * drivebyAssoc->blendDelta + drivebyAssoc->blendAmount);

		if (walkbackAssoc)
			blendDelta -= Min(blendDelta, CTimer::GetTimeStepNonClipped() * 0.02f * walkbackAssoc->blendDelta + walkbackAssoc->blendAmount);

		float targetLRLean, timeBlend, neededAngForWheelie, stoppieAng;

		// Smooth the lean amount
		if (targetUDLean == -1.0f) {
			targetLRLean = 0.0f;
			timeBlend = Pow(0.86f, CTimer::GetTimeStep());
		} else {
			targetLRLean = clamp(bike->m_fLeanLRAngle / bike->pBikeHandling->fFullAnimLean, -1.0f, 1.0f);
			timeBlend = Pow(0.86f, CTimer::GetTimeStep());
		}

		bike->m_fPedLeanAmountLR = bike->m_fPedLeanAmountLR * timeBlend + (1.0 - timeBlend) * targetLRLean;

		if (!IsPlayer()) {
			targetUDLean = 0.0f;

		} else if (targetUDLean > -1.0f) {
			targetUDLean = bike->m_fLeanInput;
			bike->bWheelieCam = false;
			neededAngForWheelie = 1.0f;
			if (bike->m_aWheelTimer[0] != 0.0f || bike->m_aWheelTimer[1] != 0.0f || bike->GetForward().z <= 0.0f ||
				(0.0f == bike->m_aWheelTimer[2] && 0.0f == bike->m_aWheelTimer[3])) {

				if (0.0f == bike->m_aWheelTimer[2] && 0.0f == bike->m_aWheelTimer[3] &&
					(bike->GetForward().z < 0.0f && (bike->m_aWheelTimer[0] != 0.0f || bike->m_aWheelTimer[1] != 0.0f))) {

					stoppieAng = bike->pBikeHandling->fStoppieAng;
					if (stoppieAng - bike->GetForward().z > 0.6f * stoppieAng)
						bike->bWheelieCam = true;
				}
			} else {
				float wheelieAng = bike->pBikeHandling->fWheelieAng;
				neededAngForWheelie = wheelieAng - bike->GetForward().z;
				if (neededAngForWheelie < wheelieAng / 2.f)
					bike->bWheelieCam = true;
			}
			if (neededAngForWheelie >= 0.15f) {
				if (bike->m_fBrakePedal <= 0.5f || velocityFwdDotProd <= 0.01f) {
					if (bike->m_fGasPedal > 0.5f && targetUDLean <= 0.0f && 0.3f * bike->pHandling->Transmission.fUnkMaxVelocity > velocityFwdDotProd) {
						targetUDLean = Min(0.1f, targetUDLean);
					}
				} else {
					targetUDLean = Max(0.1f, targetUDLean);
				}
			} else {
				targetUDLean = Max(0.25f, targetUDLean);
			}
			float targetLRLeanABS = Abs(targetLRLean);
			if (targetLRLeanABS > 0.3f) {
				// Yes, UD
				targetUDLean *= Max(0.0f, 1.0f - (targetLRLeanABS - 0.3f) * 50.f / 13.f);
			}
		}
		if (IsPlayer()) {
			float timeBlend = Pow(0.89f, CTimer::GetTimeStep());
			bike->m_fPedLeanAmountUD = (timeBlend * bike->m_fPedLeanAmountUD) + ((1.0f - timeBlend) * targetUDLean);
		} else {
			bike->m_fPedLeanAmountUD = 0.0f;
		}

		float fwdBackLeanAmount, leftRightLeanAmount;
		if (Abs(bike->m_fPedLeanAmountLR) <= 0.56f && IsPlayer()) {

			if (Abs(bike->m_fPedLeanAmountUD) <= 0.56f) {
				CVector2D smoothedLean(bike->m_fPedLeanAmountLR, bike->m_fPedLeanAmountUD);
				float smoothLeanMag = smoothedLean.Magnitude();
				if (smoothLeanMag <= 0.01f) {
					fwdBackLeanAmount = Abs(smoothedLean.y);
					leftRightLeanAmount = Abs(smoothedLean.x);
				} else {
					fwdBackLeanAmount = Abs(smoothedLean.y / smoothLeanMag);
					leftRightLeanAmount = Abs(smoothedLean.x / smoothLeanMag);
				}
			} else {
				fwdBackLeanAmount = 1.0f;
				leftRightLeanAmount = 0.0f;
			}
		} else {
			fwdBackLeanAmount = 0.0f;
			leftRightLeanAmount = 1.0f;
		}
		float fwdBackBlend = fwdBackLeanAmount * blendDelta;
		float leftRightBlend = leftRightLeanAmount * blendDelta;
		if (IsPlayer()) {
			if (!fwdAssoc)
				fwdAssoc = CAnimManager::AddAnimation(GetClump(), bike->m_bikeAnimType, ANIM_BIKE_FWD);
			if (!backAssoc)
				backAssoc = CAnimManager::AddAnimation(GetClump(), bike->m_bikeAnimType, ANIM_BIKE_BACK);

			if (bike->m_fPedLeanAmountUD < 0.0f) {
				backAssoc->blendAmount = fwdBackBlend;
				backAssoc->SetCurrentTime(-(bike->m_fPedLeanAmountUD * backAssoc->hierarchy->totalLength));
				backAssoc->flags &= ~ASSOC_RUNNING;
				fwdAssoc->blendAmount = 0.0f;
			} else {
				fwdAssoc->blendAmount = fwdBackBlend;
				fwdAssoc->SetCurrentTime(bike->m_fPedLeanAmountUD* fwdAssoc->hierarchy->totalLength);
				fwdAssoc->flags &= ~ASSOC_RUNNING;
				backAssoc->blendAmount = 0.0f;
			}
		}
		if (!leftAssoc)
			leftAssoc = CAnimManager::AddAnimation(GetClump(), bike->m_bikeAnimType, ANIM_BIKE_LEFT);
		if (!rightAssoc)
			rightAssoc = CAnimManager::AddAnimation(GetClump(), bike->m_bikeAnimType, ANIM_BIKE_RIGHT);

		if (bike->m_fPedLeanAmountLR < 0.0f) {
			leftAssoc->blendAmount = leftRightBlend;
			leftAssoc->SetCurrentTime(-(bike->m_fPedLeanAmountLR * leftAssoc->hierarchy->totalLength));
			leftAssoc->flags &= ~ASSOC_RUNNING;
			rightAssoc->blendAmount = 0.0f;
		} else {
			rightAssoc->blendAmount = leftRightBlend;
			rightAssoc->SetCurrentTime(bike->m_fPedLeanAmountLR* rightAssoc->hierarchy->totalLength);
			rightAssoc->flags &= ~ASSOC_RUNNING;
			leftAssoc->blendAmount = 0.0f;
		}
		if (velocityFwdDotProd > 0.3f) {
			RwV3d Xaxis = { 1.0f, 0.0f, 0.0f };
			RwV3d Yaxis = { 0.0f, 1.0f, 0.0f };
			RtQuatRotate(&m_pFrames[PED_HEAD]->hanimFrame->q, &Xaxis, CGeneral::GetRandomNumberInRange(-6.0f * velocityFwdDotProd, 6.0f * velocityFwdDotProd), rwCOMBINEPOSTCONCAT);
			RtQuatRotate(&m_pFrames[PED_HEAD]->hanimFrame->q, &Yaxis, CGeneral::GetRandomNumberInRange(-6.0f * velocityFwdDotProd, 6.0f * velocityFwdDotProd), rwCOMBINEPOSTCONCAT);
			bDontAcceptIKLookAts = true;
		}
		return;
	}

	if (!IsPlayer())
		return;

	float steerAngle = m_pMyVehicle->m_fSteerAngle;
	CAnimBlendAssociation* lDriveAssoc;
	CAnimBlendAssociation* rDriveAssoc;
	CAnimBlendAssociation* lbAssoc;
	CAnimBlendAssociation* sitAssoc;
	if (m_pMyVehicle->IsBoat() && !(m_pMyVehicle->pHandling->Flags & HANDLING_SIT_IN_BOAT)) {
		sitAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_DRIVE_BOAT);

		if (!sitAssoc || sitAssoc->blendAmount < 1.0f) {
			return;
		}

		lDriveAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_DRIVE_BOAT_L);
		rDriveAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_DRIVE_BOAT_R);
		lbAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_BOAT_LB);
	} else if (m_pMyVehicle->bLowVehicle) {
		sitAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_CAR_LSIT);

		if (!sitAssoc || sitAssoc->blendAmount < 1.0f) {
			return;
		}

		lDriveAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_DRIVE_LOW_L);
		lbAssoc = nil;
		rDriveAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_DRIVE_LOW_R);
	} else {
		sitAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_CAR_SIT);

		if (!sitAssoc || sitAssoc->blendAmount < 1.0f) {
			return;
		}

		lDriveAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_DRIVE_L);
		rDriveAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_DRIVE_R);
		lbAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_CAR_LB);
	}

	if (lbAssoc &&
		TheCamera.Cams[TheCamera.ActiveCam].Mode == CCam::MODE_1STPERSON
		&& TheCamera.Cams[TheCamera.ActiveCam].DirectionWasLooking == LOOKING_LEFT) {
		lbAssoc->blendDelta = -1000.0f;
	}

	CAnimBlendAssociation* driveByAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_DRIVEBY_L);
	if (!driveByAssoc)
		driveByAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_DRIVEBY_R);
	if (!driveByAssoc)
		driveByAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_DRIVEBY_LOW_L);
	if (!driveByAssoc)
		driveByAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_DRIVEBY_LOW_R);

	if (m_pMyVehicle->bLowVehicle || m_pMyVehicle->m_fGasPedal >= 0.0f || driveByAssoc ||
		m_pMyVehicle->GetVehicleAppearance() == VEHICLE_APPEARANCE_HELI || m_pMyVehicle->GetVehicleAppearance() == VEHICLE_APPEARANCE_PLANE) {
		if (steerAngle == 0.0f || driveByAssoc) {
			if (lDriveAssoc)
				lDriveAssoc->blendAmount = 0.0f;
			if (rDriveAssoc)
				rDriveAssoc->blendAmount = 0.0f;

		} else if (steerAngle <= 0.0f) {
			if (lDriveAssoc)
				lDriveAssoc->blendAmount = 0.0f;

			if (rDriveAssoc)
				rDriveAssoc->blendAmount = clamp(steerAngle * -100.0f / 61.0f, 0.0f, 1.0f);
			else if (m_pMyVehicle->IsBoat() && !(m_pMyVehicle->pHandling->Flags & HANDLING_SIT_IN_BOAT))
				CAnimManager::AddAnimation(GetClump(), ASSOCGRP_STD, ANIM_DRIVE_BOAT_R);
			else if (m_pMyVehicle->bLowVehicle)
				CAnimManager::AddAnimation(GetClump(), ASSOCGRP_STD, ANIM_DRIVE_LOW_R);
			else
				CAnimManager::AddAnimation(GetClump(), ASSOCGRP_STD, ANIM_DRIVE_R);

		} else {
			if (rDriveAssoc)
				rDriveAssoc->blendAmount = 0.0f;

			if (lDriveAssoc)
				lDriveAssoc->blendAmount = clamp(steerAngle * 100.0f / 61.0f, 0.0f, 1.0f);
			else if (m_pMyVehicle->IsBoat() && !(m_pMyVehicle->pHandling->Flags & HANDLING_SIT_IN_BOAT))
				CAnimManager::AddAnimation(GetClump(), ASSOCGRP_STD, ANIM_DRIVE_BOAT_L);
			else if (m_pMyVehicle->bLowVehicle)
				CAnimManager::AddAnimation(GetClump(), ASSOCGRP_STD, ANIM_DRIVE_LOW_L);
			else
				CAnimManager::AddAnimation(GetClump(), ASSOCGRP_STD, ANIM_DRIVE_L);
		}

		if (lbAssoc)
			lbAssoc->blendDelta = -4.0f;
	} else {

		if ((TheCamera.Cams[TheCamera.ActiveCam].Mode != CCam::MODE_1STPERSON
			|| TheCamera.Cams[TheCamera.ActiveCam].DirectionWasLooking != LOOKING_LEFT)
			&& (!lbAssoc || lbAssoc->blendAmount < 1.0f && lbAssoc->blendDelta <= 0.0f)) {

			if (m_pMyVehicle->IsBoat() && !(m_pMyVehicle->pHandling->Flags & HANDLING_SIT_IN_BOAT))
				CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_BOAT_LB, 4.0f);
			else
				CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_CAR_LB, 4.0f);
		}
	}
}

// --MIAMI: Done
void
CPed::RemoveWeaponAnims(int unused, float animDelta)
{	
	CAnimBlendAssociation *weaponAssoc;
	//CWeaponInfo::GetWeaponInfo(unused);

	weaponAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_WEAPON_FIRE);
	if (weaponAssoc) {
		weaponAssoc->blendDelta = animDelta;
		weaponAssoc->flags |= ASSOC_DELETEFADEDOUT;
	}
	weaponAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_WEAPON_FIRE_2ND);
	if (weaponAssoc) {
		weaponAssoc->blendDelta = animDelta;
		weaponAssoc->flags |= ASSOC_DELETEFADEDOUT;
	}
	weaponAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_WEAPON_FIRE_3RD);
	if (weaponAssoc) {
		weaponAssoc->blendDelta = animDelta;
		weaponAssoc->flags |= ASSOC_DELETEFADEDOUT;
	}
	weaponAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_WEAPON_RELOAD);
	if (weaponAssoc) {
		weaponAssoc->blendDelta = animDelta;
		weaponAssoc->flags |= ASSOC_DELETEFADEDOUT;
	}
	weaponAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_WEAPON_CROUCHRELOAD);
	if (weaponAssoc) {
		weaponAssoc->flags |= ASSOC_DELETEFADEDOUT;
		if (weaponAssoc->flags & ASSOC_PARTIAL)
			weaponAssoc->blendDelta = animDelta;
		else
			CAnimManager::BlendAnimation(GetClump(), m_animGroup, ANIM_IDLE_STANCE, -animDelta);
	}
}