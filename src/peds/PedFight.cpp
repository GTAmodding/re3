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

uint16 nPlayerInComboMove;

RpClump *flyingClumpTemp;

// This is beta fistfite.dat array. Not used anymore since they're being fetched from fistfite.dat.
FightMove tFightMoves[NUM_FIGHTMOVES] = {
	{ANIM_STD_NUM, 0.0f, 0.0f, 0.0f, 0.0f, HITLEVEL_NULL, 0, 0},
	{ANIM_STD_PUNCH, 0.2f, 8.0f / 30.0f, 0.0f, 0.3f, HITLEVEL_HIGH, 1, 0},
	{ANIM_STD_FIGHT_IDLE, 0.0f, 0.0f, 0.0f, 0.0f, HITLEVEL_NULL, 0, 0},
	{ANIM_STD_FIGHT_SHUFFLE_F, 0.0f, 0.0f, 0.0f, 0.0f, HITLEVEL_NULL, 0, 0},
	{ANIM_STD_FIGHT_KNEE, 4.0f / 30.0f, 0.2f, 0.0f, 0.6f, HITLEVEL_LOW, 2, 0},
	{ANIM_STD_FIGHT_HEAD, 4.0f / 30.0f, 0.2f, 0.0f, 0.7f, HITLEVEL_HIGH, 3, 0},
	{ANIM_STD_FIGHT_PUNCH, 4.0f / 30.0f, 7.0f / 30.0f, 10.0f / 30.0f, 0.4f, HITLEVEL_HIGH, 1, 0},
	{ANIM_STD_FIGHT_LHOOK, 8.0f / 30.0f, 10.0f / 30.0f, 0.0f, 0.4f, HITLEVEL_HIGH, 3, 0},
	{ANIM_STD_FIGHT_KICK, 8.0f / 30.0f, 10.0f / 30.0f, 0.0f, 0.5, HITLEVEL_MEDIUM, 2, 0},
	{ANIM_STD_FIGHT_LONGKICK, 8.0f / 30.0f, 10.0f / 30.0f, 0.0f, 0.5, HITLEVEL_MEDIUM, 4, 0},
	{ANIM_STD_FIGHT_ROUNDHOUSE, 8.0f / 30.0f, 10.0f / 30.0f, 0.0f, 0.6f, HITLEVEL_MEDIUM, 4, 0},
	{ANIM_STD_FIGHT_BODYBLOW, 5.0f / 30.0f, 7.0f / 30.0f, 0.0f, 0.35f, HITLEVEL_LOW, 2, 0},
	{ANIM_STD_KICKGROUND, 10.0f / 30.0f, 14.0f / 30.0f, 0.0f, 0.4f, HITLEVEL_GROUND, 1, 0},
	{ANIM_STD_HIT_FRONT, 0.0f, 0.0f, 0.0f, 0.0f, HITLEVEL_NULL, 0, 0},
	{ANIM_STD_HIT_BACK, 0.0f, 0.0f, 0.0f, 0.0f, HITLEVEL_NULL, 0, 0},
	{ANIM_STD_HIT_RIGHT, 0.0f, 0.0f, 0.0f, 0.0f, HITLEVEL_NULL, 0, 0},
	{ANIM_STD_HIT_LEFT, 0.0f, 0.0f, 0.0f, 0.0f, HITLEVEL_NULL, 0, 0},
	{ANIM_STD_HIT_BODYBLOW, 0.0f, 0.0f, 0.0f, 0.0f, HITLEVEL_NULL, 0, 0},
	{ANIM_STD_HIT_CHEST, 0.0f, 0.0f, 0.0f, 0.0f, HITLEVEL_NULL, 0, 0},
	{ANIM_STD_HIT_HEAD, 0.0f, 0.0f, 0.0f, 0.0f, HITLEVEL_NULL, 0, 0},
	{ANIM_STD_HIT_WALK, 0.0f, 0.0f, 0.0f, 0.0f, HITLEVEL_NULL, 0, 0},
	{ANIM_STD_HIT_FLOOR, 0.0f, 0.0f, 0.0f, 0.0f, HITLEVEL_NULL, 0, 0},
	{ANIM_STD_HIT_BEHIND, 0.0f, 0.0f, 0.0f, 0.0f, HITLEVEL_NULL, 0, 0},
	{ANIM_STD_FIGHT_2IDLE, 0.0f, 0.0f, 0.0f, 0.0f, HITLEVEL_NULL, 0, 0},
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

void
CPed::SetPointGunAt(CEntity *to)
{
	if (to) {
		SetLookFlag(to, true);
		SetAimFlag(to);
#ifdef VC_PED_PORTS
		SetLookTimer(INT32_MAX);
#endif
	}

	if (m_nPedState == PED_AIM_GUN || bIsDucking || m_nWaitState == WAITSTATE_PLAYANIM_DUCK)
		return;

	if (m_nPedState != PED_ATTACK)
		SetStoredState();

	SetPedState(PED_AIM_GUN);
	bIsPointingGunAt = true;
	CWeaponInfo *curWeapon = CWeaponInfo::GetWeaponInfo(GetWeapon()->m_eWeaponType);
	SetMoveState(PEDMOVE_NONE);

	CAnimBlendAssociation *aimAssoc;

	if (bCrouchWhenShooting)
		aimAssoc = RpAnimBlendClumpGetAssociation(GetClump(), curWeapon->m_Anim2ToPlay);
	else
		aimAssoc = RpAnimBlendClumpGetAssociation(GetClump(), curWeapon->m_AnimToPlay);

	if (!aimAssoc || aimAssoc->blendDelta < 0.0f) {
		if (bCrouchWhenShooting)
			aimAssoc = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, curWeapon->m_Anim2ToPlay, 4.0f);
		else
			aimAssoc = CAnimManager::AddAnimation(GetClump(), ASSOCGRP_STD, curWeapon->m_AnimToPlay);

		aimAssoc->blendAmount = 0.0f;
		aimAssoc->blendDelta = 8.0f;
	}
	if (to)
		Say(SOUND_PED_ATTACK);
}

void
CPed::PointGunAt(void)
{
	CWeaponInfo *weaponInfo = CWeaponInfo::GetWeaponInfo(GetWeapon()->m_eWeaponType);
	CAnimBlendAssociation *weaponAssoc = RpAnimBlendClumpGetAssociation(GetClump(), weaponInfo->m_AnimToPlay);
	if (!weaponAssoc || weaponAssoc->blendDelta < 0.0f)
		weaponAssoc = RpAnimBlendClumpGetAssociation(GetClump(), weaponInfo->m_Anim2ToPlay);

	if (weaponAssoc && weaponAssoc->currentTime > weaponInfo->m_fAnimLoopStart) {
		weaponAssoc->SetCurrentTime(weaponInfo->m_fAnimLoopStart);
		weaponAssoc->flags &= ~ASSOC_RUNNING;

		if (weaponInfo->IsFlagSet(WEAPONFLAG_CANAIM_WITHARM))
			m_pedIK.m_flags |= CPedIK::AIMS_WITH_ARM;
		else
			m_pedIK.m_flags &= ~CPedIK::AIMS_WITH_ARM;
	}
}

void
CPed::ClearPointGunAt(void)
{
	CAnimBlendAssociation *animAssoc;
	CWeaponInfo *weaponInfo;

	ClearLookFlag();
	ClearAimFlag();
	bIsPointingGunAt = false;
#ifndef VC_PED_PORTS
	if (m_nPedState == PED_AIM_GUN) {
		RestorePreviousState();
#else
	if (m_nPedState == PED_AIM_GUN || m_nPedState == PED_ATTACK) {
		SetPedState(PED_IDLE);
		RestorePreviousState();
	}
#endif
		weaponInfo = CWeaponInfo::GetWeaponInfo(GetWeapon()->m_eWeaponType);
		animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), weaponInfo->m_AnimToPlay);
		if (!animAssoc || animAssoc->blendDelta < 0.0f) {
			animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), weaponInfo->m_Anim2ToPlay);
		}
		if (animAssoc) {
			animAssoc->flags |= ASSOC_DELETEFADEDOUT;
			animAssoc->blendDelta = -4.0f;
		}
#ifndef VC_PED_PORTS
	}
#endif
}

void
CPed::SetAttack(CEntity *victim)
{
	CPed *victimPed = nil;
	if (victim && victim->IsPed())
		victimPed = (CPed*)victim;

	CAnimBlendAssociation *animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_IDLE_BIGGUN);
	if (animAssoc) {
		animAssoc->blendDelta = -1000.0f;
		animAssoc->flags |= ASSOC_DELETEFADEDOUT;
	}

	if (m_attackTimer > CTimer::GetTimeInMilliseconds() || m_nWaitState == WAITSTATE_SURPRISE)
		return;

	if (RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_HGUN_RELOAD)) {
		bIsAttacking = false;
		return;
	}

	if (RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_AK_RELOAD)) {
		if (!IsPlayer() || m_nPedState != PED_ATTACK || ((CPlayerPed*)this)->m_bHaveTargetSelected)
			bIsAttacking = false;
		else
			bIsAttacking = true;

		return;
	}

	CWeaponInfo *curWeapon = CWeaponInfo::GetWeaponInfo(GetWeapon()->m_eWeaponType);
	if (curWeapon->m_eWeaponFire == WEAPON_FIRE_INSTANT_HIT && !IsPlayer()) {
		if (GetWeapon()->HitsGround(this, nil, victim))
			return;
	}

	if (GetWeapon()->m_eWeaponType == WEAPONTYPE_UNARMED) {
		if (IsPlayer() ||
			(m_nPedState != PED_FIGHT && m_nMoveState != PEDMOVE_NONE && m_nMoveState != PEDMOVE_STILL && !(m_pedStats->m_flags & STAT_SHOPPING_BAGS))) {

			if (m_nPedState != PED_ATTACK) {
				SetPedState(PED_ATTACK);
				bIsAttacking = false;
				animAssoc = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, curWeapon->m_AnimToPlay, 8.0f);
				animAssoc->SetRun();
				if (animAssoc->currentTime == animAssoc->hierarchy->totalLength)
					animAssoc->SetCurrentTime(0.0f);

				animAssoc->SetFinishCallback(FinishedAttackCB, this);
			}
		} else {
			StartFightAttack(CGeneral::GetRandomNumber() % 256);
		}
		return;
	}

	m_pSeekTarget = victim;
	if (m_pSeekTarget)
		m_pSeekTarget->RegisterReference((CEntity **) &m_pSeekTarget);

	if (curWeapon->IsFlagSet(WEAPONFLAG_CANAIM)) {
		CVector aimPos = GetRight() * 0.1f + GetForward() * 0.2f + GetPosition();
		CEntity *obstacle = CWorld::TestSphereAgainstWorld(aimPos, 0.2f, nil, true, false, false, true, false, false);
		if (obstacle)
			return;

		m_pLookTarget = victim;
		if (victim) {
			m_pLookTarget->RegisterReference((CEntity **) &m_pLookTarget);
			m_pSeekTarget->RegisterReference((CEntity **) &m_pSeekTarget);
		}
		if (m_pLookTarget) {
			SetAimFlag(m_pLookTarget);
		} else {
			SetAimFlag(m_fRotationCur);

			if (FindPlayerPed() == this && TheCamera.Cams[0].Using3rdPersonMouseCam())
				((CPlayerPed*)this)->m_fFPSMoveHeading = TheCamera.Find3rdPersonQuickAimPitch();
		}
	}
	if (m_nPedState == PED_ATTACK) {
		bIsAttacking = true;
		return;
	}

	if (IsPlayer() || !victimPed || victimPed->IsPedInControl()) {
		if (IsPlayer())
			CPad::GetPad(0)->ResetAverageWeapon();

		uint8 pointBlankStatus;
		if ((curWeapon->m_eWeaponFire == WEAPON_FIRE_INSTANT_HIT || GetWeapon()->m_eWeaponType == WEAPONTYPE_FLAMETHROWER)
			&& TheCamera.PlayerWeaponMode.Mode != CCam::MODE_M16_1STPERSON
			&& TheCamera.PlayerWeaponMode.Mode != CCam::MODE_M16_1STPERSON_RUNABOUT
			&& TheCamera.PlayerWeaponMode.Mode != CCam::MODE_SNIPER
			&& TheCamera.PlayerWeaponMode.Mode != CCam::MODE_SNIPER_RUNABOUT
			&& (pointBlankStatus = CheckForPointBlankPeds(victimPed)) != NO_POINT_BLANK_PED) {
			ClearAimFlag();

			// This condition is pointless
			if (pointBlankStatus == POINT_BLANK_FOR_WANTED_PED || !victimPed)
				StartFightAttack(200);
		} else {
			if (!curWeapon->IsFlagSet(WEAPONFLAG_CANAIM))
				m_pSeekTarget = nil;

			if (m_nPedState != PED_AIM_GUN)
				SetStoredState();

			SetPedState(PED_ATTACK);
			SetMoveState(PEDMOVE_NONE);
			if (bCrouchWhenShooting) {
				animAssoc = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_RBLOCK_SHOOT, 4.0f);
			} else {
				float animDelta = 8.0f;
				if (curWeapon->m_eWeaponFire == WEAPON_FIRE_MELEE)
					animDelta = 1000.0f;

				if (GetWeapon()->m_eWeaponType != WEAPONTYPE_BASEBALLBAT
					|| CheckForPedsOnGroundToAttack(this, nil) < PED_ON_THE_FLOOR) {
					animAssoc = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, curWeapon->m_AnimToPlay, animDelta);
				} else {
					animAssoc = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, curWeapon->m_Anim2ToPlay, animDelta);
				}
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

	SetLookFlag(victim, false);
	SetLookTimer(100);
}

void
CPed::ClearAttack(void)
{
	if (m_nPedState != PED_ATTACK || bIsDucking || m_nWaitState == WAITSTATE_PLAYANIM_DUCK)
		return;

#ifdef VC_PED_PORTS
	// VC uses CCamera::Using1stPersonWeaponMode
	if (FindPlayerPed() == this && (TheCamera.PlayerWeaponMode.Mode == CCam::MODE_SNIPER ||
		TheCamera.PlayerWeaponMode.Mode == CCam::MODE_M16_1STPERSON || TheCamera.PlayerWeaponMode.Mode == CCam::MODE_ROCKETLAUNCHER)) {
		SetPointGunAt(nil);
	} else
#endif
	if (bIsPointingGunAt) {
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

void
CPed::ClearAttackByRemovingAnim(void)
{
	if (m_nPedState != PED_ATTACK || bIsDucking)
		return;

	CWeaponInfo *weapon = CWeaponInfo::GetWeaponInfo(GetWeapon()->m_eWeaponType);
	CAnimBlendAssociation *weaponAssoc = RpAnimBlendClumpGetAssociation(GetClump(), weapon->m_AnimToPlay);
	if (!weaponAssoc) {
		weaponAssoc = RpAnimBlendClumpGetAssociation(GetClump(), weapon->m_Anim2ToPlay);

		if (!weaponAssoc && weapon->IsFlagSet(WEAPONFLAG_THROW))
			weaponAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_THROW_UNDER);

		if (!weaponAssoc) {
			ClearAttack();
			return;
		}
	}
	weaponAssoc->blendDelta = -8.0f;
	weaponAssoc->flags &= ~ASSOC_RUNNING;
	weaponAssoc->flags |= ASSOC_DELETEFADEDOUT;
	weaponAssoc->SetDeleteCallback(FinishedAttackCB, this);
}

void
CPed::FinishedAttackCB(CAnimBlendAssociation *attackAssoc, void *arg)
{
	CWeaponInfo *currentWeapon;
	CAnimBlendAssociation *newAnim;
	CPed *ped = (CPed*)arg;

	if (attackAssoc) {
		switch (attackAssoc->animId) {
			case ANIM_STD_START_THROW:
				// what?!
				if ((!ped->IsPlayer() || ((CPlayerPed*)ped)->m_bHaveTargetSelected) && ped->IsPlayer()) {
					attackAssoc->blendDelta = -1000.0f;
					newAnim = CAnimManager::AddAnimation(ped->GetClump(), ASSOCGRP_STD, ANIM_STD_THROW_UNDER);
				} else {
					attackAssoc->blendDelta = -1000.0f;
					newAnim = CAnimManager::AddAnimation(ped->GetClump(), ASSOCGRP_STD, ANIM_STD_WEAPON_THROW);
				}

				newAnim->SetFinishCallback(FinishedAttackCB, ped);
				return;

			case ANIM_STD_PARTIAL_PUNCH:
				attackAssoc->blendDelta = -8.0f;
				attackAssoc->flags |= ASSOC_DELETEFADEDOUT;
				ped->ClearAttack();
				return;

			case ANIM_STD_WEAPON_THROW:
			case ANIM_STD_THROW_UNDER:
				if (ped->GetWeapon()->m_nAmmoTotal > 0) {
					currentWeapon = CWeaponInfo::GetWeaponInfo(ped->GetWeapon()->m_eWeaponType);
					ped->AddWeaponModel(currentWeapon->m_nModelId);
				}
				break;
			default:
				break;
		}
	}
	
	if (!ped->bIsAttacking)
		ped->ClearAttack();
}

uint8
CPed::CheckForPointBlankPeds(CPed *pedToVerify)
{
	float pbDistance = 1.1f;
	if (GetWeapon()->IsType2Handed())
		pbDistance = 1.6f;

	for (int i = 0; i < m_numNearPeds; i++)	{
		CPed *nearPed = m_nearPeds[i];

		if (!pedToVerify || pedToVerify == nearPed) {

			CVector diff = nearPed->GetPosition() - GetPosition();
			if (diff.Magnitude() < pbDistance) {

				float neededAngle = CGeneral::GetRadianAngleBetweenPoints(
					nearPed->GetPosition().x, nearPed->GetPosition().y,
					GetPosition().x, GetPosition().y);
				neededAngle = CGeneral::LimitRadianAngle(neededAngle);
				m_fRotationCur = CGeneral::LimitRadianAngle(m_fRotationCur);

				float neededTurn = Abs(neededAngle - m_fRotationCur);

				if (neededTurn > PI)
					neededTurn = 2*PI - neededTurn;

				if (nearPed->OnGroundOrGettingUp() || nearPed->m_nPedState == PED_DIVE_AWAY)
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

void
CPed::Attack(void)
{
	CAnimBlendAssociation *weaponAnimAssoc;
	int32 weaponAnim;
	float animStart;
	float weaponAnimTime;
	float animLoopEnd;
	CWeaponInfo *ourWeapon;
	bool attackShouldContinue;
	AnimationId reloadAnim;
	CAnimBlendAssociation *reloadAnimAssoc;
	float delayBetweenAnimAndFire;
	CVector firePos;

	ourWeapon = CWeaponInfo::GetWeaponInfo(GetWeapon()->m_eWeaponType);
	weaponAnimAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ourWeapon->m_AnimToPlay);
	attackShouldContinue = bIsAttacking;
	reloadAnimAssoc = nil;
	reloadAnim = ANIM_STD_NUM;
	delayBetweenAnimAndFire = ourWeapon->m_fAnimFrameFire;
	weaponAnim = ourWeapon->m_AnimToPlay;

	if (weaponAnim == ANIM_STD_WEAPON_HGUN_BODY)
		reloadAnim = ANIM_STD_HGUN_RELOAD;
	else if (weaponAnim == ANIM_STD_WEAPON_AK_BODY)
		reloadAnim = ANIM_STD_AK_RELOAD;

	if (reloadAnim != ANIM_STD_NUM)
		reloadAnimAssoc = RpAnimBlendClumpGetAssociation(GetClump(), reloadAnim);

	if (bIsDucking)
		return;

	if (reloadAnimAssoc) {
		if (!IsPlayer() || ((CPlayerPed*)this)->m_bHaveTargetSelected)
			ClearAttack();

		return;
	}

	if (CTimer::GetTimeInMilliseconds() < m_shootTimer)
		attackShouldContinue = true;

	if (!weaponAnimAssoc) {
		weaponAnimAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ourWeapon->m_Anim2ToPlay);
		delayBetweenAnimAndFire = ourWeapon->m_fAnim2FrameFire;

		// Long throw granade, molotov
		if (!weaponAnimAssoc && ourWeapon->IsFlagSet(WEAPONFLAG_THROW)) {
			weaponAnimAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_THROW_UNDER);
			delayBetweenAnimAndFire = 0.2f;
		}

		if (!weaponAnimAssoc) {
			if (attackShouldContinue) {
				if (ourWeapon->m_eWeaponFire != WEAPON_FIRE_PROJECTILE || !IsPlayer() || ((CPlayerPed*)this)->m_bHaveTargetSelected) {
					if (!CGame::nastyGame || ourWeapon->m_eWeaponFire != WEAPON_FIRE_MELEE || CheckForPedsOnGroundToAttack(this, nil) < PED_ON_THE_FLOOR) {
						weaponAnimAssoc = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ourWeapon->m_AnimToPlay, 8.0f);
					}
					else {
						weaponAnimAssoc = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ourWeapon->m_Anim2ToPlay, 8.0f);
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

			return;
		}
	}

	animStart = ourWeapon->m_fAnimLoopStart;
	weaponAnimTime = weaponAnimAssoc->currentTime;
	if (weaponAnimTime > animStart && weaponAnimTime - weaponAnimAssoc->timeStep <= animStart) {
		if (ourWeapon->IsFlagSet(WEAPONFLAG_CANAIM_WITHARM))
			m_pedIK.m_flags |= CPedIK::AIMS_WITH_ARM;
		else
			m_pedIK.m_flags &= ~CPedIK::AIMS_WITH_ARM;
	}

	if (weaponAnimTime <= delayBetweenAnimAndFire || weaponAnimTime - weaponAnimAssoc->timeStep > delayBetweenAnimAndFire || !weaponAnimAssoc->IsRunning()) {
		if (weaponAnimAssoc->speed < 1.0f)
			weaponAnimAssoc->speed = 1.0f;

	} else {
		firePos = ourWeapon->m_vecFireOffset;
		if (GetWeapon()->m_eWeaponType == WEAPONTYPE_BASEBALLBAT) {
			if (weaponAnimAssoc->animId == ourWeapon->m_Anim2ToPlay)
				firePos.z = 0.7f * ourWeapon->m_fRadius - 1.0f;

			firePos = GetMatrix() * firePos;
		} else if (GetWeapon()->m_eWeaponType != WEAPONTYPE_UNARMED) {
			TransformToNode(firePos, weaponAnimAssoc->animId == ANIM_STD_KICKGROUND ? PED_FOOTR : PED_HANDR);
		} else {
			firePos = GetMatrix() * firePos;
		}
			
		GetWeapon()->Fire(this, &firePos);

		if (GetWeapon()->m_eWeaponType == WEAPONTYPE_MOLOTOV || GetWeapon()->m_eWeaponType == WEAPONTYPE_GRENADE) {
			RemoveWeaponModel(ourWeapon->m_nModelId);
		}
		if (!GetWeapon()->m_nAmmoTotal && ourWeapon->m_eWeaponFire != WEAPON_FIRE_MELEE && FindPlayerPed() != this) {
			SelectGunIfArmed();
		}

		if (GetWeapon()->m_eWeaponState != WEAPONSTATE_MELEE_MADECONTACT) {
			// If reloading just began, start the animation
			// Last condition will always return true, even IDA hides it
			if (GetWeapon()->m_eWeaponState == WEAPONSTATE_RELOADING && reloadAnim != ANIM_STD_NUM /* && !reloadAnimAssoc*/) {
				CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, reloadAnim, 8.0f);
				ClearLookFlag();
				ClearAimFlag();
				bIsAttacking = false;
				bIsPointingGunAt = false;
				m_shootTimer = CTimer::GetTimeInMilliseconds();
				return;
			}
		} else {
			if (weaponAnimAssoc->animId == ANIM_STD_WEAPON_BAT_V || weaponAnimAssoc->animId == ANIM_STD_WEAPON_BAT_H) {
				DMAudio.PlayOneShot(m_audioEntityId, SOUND_WEAPON_BAT_ATTACK, 1.0f);
			} else if (weaponAnimAssoc->animId == ANIM_STD_PARTIAL_PUNCH) {
				DMAudio.PlayOneShot(m_audioEntityId, SOUND_FIGHT_PUNCH_39, 0.0f);
			}

			weaponAnimAssoc->speed = 0.5f;

			if (bIsAttacking || CTimer::GetTimeInMilliseconds() < m_shootTimer) {
				weaponAnimAssoc->callbackType = 0;
			}
		}

		attackShouldContinue = false;
	}

	if (GetWeapon()->m_eWeaponType == WEAPONTYPE_SHOTGUN) {
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
#ifdef VC_PED_PORTS
	if (IsPlayer()) {
		if (CPad::GetPad(0)->GetSprint()) {
			// animBreakout is a member of WeaponInfo in VC, so it's me that added the below line.
			float animBreakOut = ((GetWeapon()->m_eWeaponType == WEAPONTYPE_FLAMETHROWER || GetWeapon()->m_eWeaponType == WEAPONTYPE_UZI || GetWeapon()->m_eWeaponType == WEAPONTYPE_SHOTGUN) ? 25 / 30.0f : 99 / 30.0f);
			if (!attackShouldContinue && weaponAnimAssoc->currentTime > animBreakOut) {
				weaponAnimAssoc->blendDelta = -4.0f;
				FinishedAttackCB(nil, this);
				return;
			}
		}
	}
#endif
	animLoopEnd = ourWeapon->m_fAnimLoopEnd;
	if (ourWeapon->m_eWeaponFire == WEAPON_FIRE_MELEE && weaponAnimAssoc->animId == ourWeapon->m_Anim2ToPlay)
		animLoopEnd = 3.4f/6.0f;

	weaponAnimTime = weaponAnimAssoc->currentTime;

	// Anim loop end, either start the loop again or finish the attack
	if (weaponAnimTime > animLoopEnd || !weaponAnimAssoc->IsRunning() && ourWeapon->m_eWeaponFire != WEAPON_FIRE_PROJECTILE) {

		if (weaponAnimTime - 2.0f * weaponAnimAssoc->timeStep <= animLoopEnd
			&& (bIsAttacking || CTimer::GetTimeInMilliseconds() < m_shootTimer)
			&& GetWeapon()->m_eWeaponState != WEAPONSTATE_RELOADING) {

			weaponAnim = weaponAnimAssoc->animId;
			if (ourWeapon->m_eWeaponFire != WEAPON_FIRE_MELEE || CheckForPedsOnGroundToAttack(this, nil) < PED_ON_THE_FLOOR) {
				if (weaponAnim != ourWeapon->m_Anim2ToPlay || weaponAnim == ANIM_STD_RBLOCK_SHOOT) {
					weaponAnimAssoc->Start(ourWeapon->m_fAnimLoopStart);
				} else {
					CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ourWeapon->m_AnimToPlay, 8.0f);
				}
			} else {
				if (weaponAnim == ourWeapon->m_Anim2ToPlay)
					weaponAnimAssoc->SetCurrentTime(0.1f);
				else
					CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ourWeapon->m_Anim2ToPlay, 8.0f);
			}
#ifdef VC_PED_PORTS
		} else if (IsPlayer() && m_pPointGunAt && bIsAimingGun && GetWeapon()->m_eWeaponState != WEAPONSTATE_RELOADING) {
			weaponAnimAssoc->SetCurrentTime(ourWeapon->m_fAnimLoopEnd);
			weaponAnimAssoc->flags &= ~ASSOC_RUNNING;
			SetPointGunAt(m_pPointGunAt);
#endif
		} else {
			ClearAimFlag();

			// Echoes of bullets, at the end of the attack. (Bug: doesn't play while reloading)
			if (weaponAnimAssoc->currentTime - weaponAnimAssoc->timeStep <= ourWeapon->m_fAnimLoopEnd) {
				switch (GetWeapon()->m_eWeaponType) {
					case WEAPONTYPE_UZI:
						DMAudio.PlayOneShot(m_audioEntityId, SOUND_WEAPON_UZI_BULLET_ECHO, 0.0f);
						break;
					case WEAPONTYPE_AK47:
						DMAudio.PlayOneShot(m_audioEntityId, SOUND_WEAPON_AK47_BULLET_ECHO, 0.0f);
						break;
					case WEAPONTYPE_M16:
						DMAudio.PlayOneShot(m_audioEntityId, SOUND_WEAPON_M16_BULLET_ECHO, 0.0f);
						break;
					default:
						break;
				}
			}

			// Fun fact: removing this part leds to reloading flamethrower
			if (GetWeapon()->m_eWeaponType == WEAPONTYPE_FLAMETHROWER && weaponAnimAssoc->IsRunning()) {
				weaponAnimAssoc->flags |= ASSOC_DELETEFADEDOUT;
				weaponAnimAssoc->flags &= ~ASSOC_RUNNING;
				weaponAnimAssoc->blendDelta = -4.0f;
			}
		}
	}
	if (weaponAnimAssoc->currentTime > delayBetweenAnimAndFire)
		attackShouldContinue = false;

	bIsAttacking = attackShouldContinue;
}

void
CPed::StartFightAttack(uint8 buttonPressure)
{
	if (!IsPedInControl() || m_attackTimer > CTimer::GetTimeInMilliseconds())
		return;

	if (m_nPedState == PED_FIGHT) {
		m_fightButtonPressure = buttonPressure;
		return;
	}

	if (m_nPedState != PED_AIM_GUN)
		SetStoredState();

	if (m_nWaitState != WAITSTATE_FALSE) {
		m_nWaitState = WAITSTATE_FALSE;
		RestoreHeadingRate();
	}

	SetPedState(PED_FIGHT);
	m_fightButtonPressure = 0;
	RpAnimBlendClumpRemoveAssociations(GetClump(), ASSOC_REPEAT);
	CAnimBlendAssociation *animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_STARTWALK);

	if (animAssoc) {
		animAssoc->flags |= ASSOC_DELETEFADEDOUT;
		animAssoc->blendDelta = -1000.0f;
	}

	animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_RUNSTOP1);
	if (!animAssoc)
		animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_RUNSTOP2);

	if (animAssoc) {
		animAssoc->flags |= ASSOC_DELETEFADEDOUT;
		animAssoc->blendDelta = -1000.0f;
		RestoreHeadingRate();
	}

	SetMoveState(PEDMOVE_NONE);
	m_nStoredMoveState = PEDMOVE_NONE;

	CAnimManager::AddAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_FIGHT_IDLE)->blendAmount = 1.0f;

	CPed *pedOnGround = nil;
	if (IsPlayer() && CheckForPedsOnGroundToAttack(this, &pedOnGround) > PED_IN_FRONT_OF_ATTACKER) {
		m_curFightMove = FIGHTMOVE_GROUNDKICK;
	} else if (m_pedStats->m_flags & STAT_SHOPPING_BAGS) {
		m_curFightMove = FIGHTMOVE_ROUNDHOUSE;
	} else {
		m_curFightMove = FIGHTMOVE_STDPUNCH;
	}

	if (pedOnGround && IsPlayer()) {
		m_fRotationDest = CGeneral::GetRadianAngleBetweenPoints(
			pedOnGround->GetPosition().x, pedOnGround->GetPosition().y,
			GetPosition().x, GetPosition().y);

		m_fRotationDest = CGeneral::LimitRadianAngle(m_fRotationDest);
		m_fRotationCur = m_fRotationDest;
		m_lookTimer = 0;
		SetLookFlag(pedOnGround, true);
		SetLookTimer(1500);
	}
	animAssoc = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, tFightMoves[m_curFightMove].animId, 4.0f);
	animAssoc->SetFinishCallback(FinishFightMoveCB, this);
	m_fightState = FIGHTSTATE_NO_MOVE;
	m_takeAStepAfterAttack = false;
	bIsAttacking = true;

	if (IsPlayer())
		nPlayerInComboMove = 0;
}

void
CPed::StartFightDefend(uint8 direction, uint8 hitLevel, uint8 unk)
{
	if (m_nPedState == PED_DEAD) {
		if (CGame::nastyGame) {
			if (hitLevel == HITLEVEL_GROUND) {
				CAnimBlendAssociation *floorHitAssoc;
				if (RpAnimBlendClumpGetFirstAssociation(GetClump(), ASSOC_FRONTAL)) {
					floorHitAssoc = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_HIT_FLOOR_FRONT, 8.0f);
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
				CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_HIT_FLOOR_FRONT, 8.0f) :
				CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_HIT_FLOOR, 8.0f);
			if (floorHitAssoc) {
				floorHitAssoc->flags &= ~ASSOC_FADEOUTWHENDONE;
				floorHitAssoc->flags |= ASSOC_DELETEFADEDOUT;
			}
		}
	} else if (IsPedInControl()) {
		if ((IsPlayer() && m_nPedState != PED_FIGHT && ((CPlayerPed*)this)->m_fMoveSpeed > 1.0f)
			|| (!IsPlayer() && m_objective == OBJECTIVE_FLEE_CHAR_ON_FOOT_TILL_SAFE)) {
#ifndef VC_PED_PORTS
			if (hitLevel != HITLEVEL_HIGH && hitLevel != HITLEVEL_LOW || (IsPlayer() || CGeneral::GetRandomNumber() & 3) && CGeneral::GetRandomNumber() & 7) {
				if (IsPlayer() || CGeneral::GetRandomNumber() & 3) {
#else
			if (hitLevel != HITLEVEL_HIGH && hitLevel != HITLEVEL_LOW || (IsPlayer() || CGeneral::GetRandomNumber() & 1) && CGeneral::GetRandomNumber() & 7) {
				if (IsPlayer() || CGeneral::GetRandomNumber() & 1) {
#endif
					AnimationId shotAnim;
					switch (direction) {
						case 1:
							shotAnim = ANIM_STD_HITBYGUN_LEFT;
							break;
						case 2:
							shotAnim = ANIM_STD_HITBYGUN_BACK;
							break;
						case 3:
							shotAnim = ANIM_STD_HITBYGUN_RIGHT;
							break;
						default:
							shotAnim = ANIM_STD_HITBYGUN_FRONT;
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
#ifndef VC_PED_PORTS
				switch (direction) {
					case 1:
						SetFall(500, ANIM_STD_HIGHIMPACT_LEFT, false);
						break;
					case 2:
						SetFall(500, ANIM_STD_HIGHIMPACT_BACK, false);
						break;
					case 3:
						SetFall(500, ANIM_STD_HIGHIMPACT_RIGHT, false);
						break;
					default:
						SetFall(500, ANIM_STD_KO_SHOT_STOMACH, false);
						break;
				}
#else
				bool fall = true;
				AnimationId hitAnim;
				switch (direction) {
					case 1:
						hitAnim = ANIM_STD_HIGHIMPACT_LEFT;
						break;
					case 2:
						if (CGeneral::GetRandomNumber() & 1) {
							fall = false;
							hitAnim = ANIM_STD_HIT_BACK;
						} else	{
							hitAnim = ANIM_STD_HIGHIMPACT_BACK;
						}
						break;
					case 3:
						hitAnim = ANIM_STD_HIGHIMPACT_RIGHT;
						break;
					default:
						if (hitLevel == HITLEVEL_LOW) {
							hitAnim = ANIM_STD_KO_SHOT_STOMACH;
						} else if (CGeneral::GetRandomNumber() & 1) {
							fall = false;
							hitAnim = ANIM_STD_HIT_WALK;
						} else if (CGeneral::GetRandomNumber() & 1) {
							fall = false;
							hitAnim = ANIM_STD_HIT_HEAD;
						} else {
							hitAnim = ANIM_STD_KO_SHOT_FACE;
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
#endif
			}
			Say(SOUND_PED_DEFEND);
		} else {
			Say(SOUND_PED_DEFEND);
			switch (hitLevel) {
				case HITLEVEL_GROUND:
					m_curFightMove = FIGHTMOVE_HITONFLOOR;
					break;
				case HITLEVEL_LOW:
#ifndef VC_PED_PORTS
					if (direction == 2) {
						SetFall(1000, ANIM_STD_HIGHIMPACT_BACK, false);
						return;
					}
#else
					if (direction == 2 && (!IsPlayer() || ((CGeneral::GetRandomNumber() & 1) && m_fHealth < 30.0f))) {
						SetFall(1000, ANIM_STD_HIGHIMPACT_BACK, false);
						return;
					} else if (direction != 2 && !IsPlayer() && (CGeneral::GetRandomNumber() & 1) && m_fHealth < 30.0f) {
						SetFall(1000, ANIM_STD_KO_SHOT_STOMACH, false);
						return;
					}	
#endif
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
					moveAssoc->speed = 1.3f;

				m_takeAStepAfterAttack = 0;
				m_fightButtonPressure = 0;
			} else if (IsPlayer() && m_currentWeapon != WEAPONTYPE_UNARMED) {
				CAnimBlendAssociation *moveAssoc = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, tFightMoves[m_curFightMove].animId, 4.0f);
				moveAssoc->SetCurrentTime(0.0f);
				moveAssoc->speed = 1.3f;
			} else {
				if (m_nPedState != PED_AIM_GUN && m_nPedState != PED_ATTACK)
					SetStoredState();

				if (m_nWaitState != WAITSTATE_FALSE) {
					m_nWaitState = WAITSTATE_FALSE;
					RestoreHeadingRate();
				}
				SetPedState(PED_FIGHT);
				m_fightButtonPressure = 0;
				RpAnimBlendClumpRemoveAssociations(GetClump(), ASSOC_REPEAT);
				CAnimBlendAssociation *walkStartAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_STARTWALK);
				if (walkStartAssoc) {
					walkStartAssoc->flags |= ASSOC_DELETEFADEDOUT;
					walkStartAssoc->blendDelta = -1000.0f;
				}
				CAnimBlendAssociation *walkStopAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_RUNSTOP1);
				if (!walkStopAssoc)
					walkStopAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_RUNSTOP2);
				if (walkStopAssoc) {
					walkStopAssoc->flags |= ASSOC_DELETEFADEDOUT;
					walkStopAssoc->blendDelta = -1000.0f;
					RestoreHeadingRate();
				}
				SetMoveState(PEDMOVE_NONE);
				m_nStoredMoveState = PEDMOVE_NONE;
				CAnimManager::AddAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_FIGHT_IDLE)->blendAmount = 1.0f;
				CAnimBlendAssociation *moveAssoc = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, tFightMoves[m_curFightMove].animId, 8.0f);
				moveAssoc->SetFinishCallback(FinishFightMoveCB, this);
				m_fightState = FIGHTSTATE_NO_MOVE;
				m_takeAStepAfterAttack = false;
				bIsAttacking = true;
			}
		}
	}
}

void
CPed::Fight(void)
{
	CAnimBlendAssociation *currentAssoc, *animAssoc;
	bool hasShoppingBags, punchOnly, canKick, canKneeHead, canRoundhouse;
	float angleToFace, nextAngle;
	bool goForward = false;
	int nextFightMove;

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

			CVector touchingNodePos(0.0f, 0.0f, 0.0f);

			switch (m_curFightMove) {
				case FIGHTMOVE_STDPUNCH:
				case FIGHTMOVE_PUNCHHOOK:
				case FIGHTMOVE_BODYBLOW:
					TransformToNode(touchingNodePos, PED_HANDR);
					break;
				case FIGHTMOVE_IDLE:
				case FIGHTMOVE_SHUFFLE_F:
					break;
				case FIGHTMOVE_KNEE:
					TransformToNode(touchingNodePos, PED_LOWERLEGR);
					break;
				case FIGHTMOVE_HEADBUTT:
					TransformToNode(touchingNodePos, PED_HEAD);
					break;
				case FIGHTMOVE_PUNCHJAB:
					TransformToNode(touchingNodePos, PED_HANDL);
					break;
				case FIGHTMOVE_KICK:
				case FIGHTMOVE_LONGKICK:
				case FIGHTMOVE_ROUNDHOUSE:
				case FIGHTMOVE_GROUNDKICK:
					TransformToNode(touchingNodePos, PED_FOOTR);
					break;
			}

			if (m_curFightMove == FIGHTMOVE_PUNCHJAB) {
				touchingNodePos += 0.1f * GetForward();
			} else if (m_curFightMove == FIGHTMOVE_PUNCHHOOK) {
				touchingNodePos += 0.22f * GetForward();
			}
			FightStrike(touchingNodePos);
			m_fightButtonPressure = 0;
			return;
		}

		if (curMove.hitLevel != HITLEVEL_NULL) {
			if (animTime > curMove.endFireTime) {
				if (IsPlayer())
					currentAssoc->speed = 1.0f;
				else
					currentAssoc->speed = 0.8f;
			}

			if (IsPlayer() && !nPlayerInComboMove) {
				if (curMove.comboFollowOnTime > 0.0f && m_fightButtonPressure != 0 && animTime > curMove.comboFollowOnTime) {

					// Notice that it increases fight move index, because we're in combo!
					animAssoc = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, tFightMoves[++m_curFightMove].animId, 8.0f);
					animAssoc->SetFinishCallback(FinishFightMoveCB, this);
					animAssoc->SetCurrentTime(0.1f * animAssoc->hierarchy->totalLength);
					m_fightButtonPressure = 0;
					nPlayerInComboMove = 1;
				}
			}
		} else {
			if (curMove.startFireTime > 0.0f && m_curFightMove != FIGHTMOVE_SHUFFLE_F && animTime > curMove.startFireTime) {
				if (IsPlayer())
					currentAssoc->speed = 1.3f;
				else
					currentAssoc->speed = 0.8f;
			}
		}
	} else if (GetWeapon()->m_eWeaponType != WEAPONTYPE_UNARMED) {
		EndFight(ENDFIGHT_FAST);

	} else if (m_fightButtonPressure != 0) {
		bool canAffectMultiplePeople = true;
		nextAngle = m_fRotationCur;
		bool kickGround = false;
		float angleForGroundKick = 0.0f;
		CPed *pedOnGround = nil;

		Say(SOUND_PED_ATTACK);

		if (IsPlayer()) {
			canRoundhouse = false;
			punchOnly = false;
			canKick = true;
			nextFightMove = (m_fightButtonPressure > 190 ? FIGHTMOVE_HEADBUTT : FIGHTMOVE_KNEE);
			hasShoppingBags = false;
			canKneeHead = true;
			nPlayerInComboMove = 0;
		} else {
			nextFightMove = (m_fightButtonPressure > 120 ? FIGHTMOVE_HEADBUTT : FIGHTMOVE_KNEE);
			uint16 pedFeatures = m_pedStats->m_flags;
			punchOnly = pedFeatures & STAT_PUNCH_ONLY;
			canRoundhouse = pedFeatures & STAT_CAN_ROUNDHOUSE;
			canKneeHead = pedFeatures & STAT_CAN_KNEE_HEAD;
			canKick = pedFeatures & STAT_CAN_KICK;
			hasShoppingBags = pedFeatures & STAT_SHOPPING_BAGS;
		}

		// Attack isn't scripted, find the victim
		if (IsPlayer() || !m_pedInObjective) {

			for (int i = 0; i < m_numNearPeds; i++) {

				CPed *nearPed = m_nearPeds[i];
				float nearPedDist = (nearPed->GetPosition() - GetPosition()).Magnitude();
				if (nearPedDist < 3.0f) {
					float angleToFace = CGeneral::GetRadianAngleBetweenPoints(
						nearPed->GetPosition().x, nearPed->GetPosition().y,
						GetPosition().x, GetPosition().y);

					nextAngle = CGeneral::LimitRadianAngle(angleToFace);
					m_fRotationCur = CGeneral::LimitRadianAngle(m_fRotationCur);

					float neededTurn = Abs(nextAngle - m_fRotationCur);
					if (neededTurn > PI)
						neededTurn = TWOPI - neededTurn;

					if (!nearPed->OnGroundOrGettingUp()) {

						if (nearPedDist < 0.8f && neededTurn < DEGTORAD(75.0f) && canKneeHead) {
							canAffectMultiplePeople = false;
						} else if (nearPedDist >= 1.3f || neededTurn >= DEGTORAD(55.0f) || hasShoppingBags) {

							if (nearPedDist < 1.7f
								&& neededTurn < DEGTORAD(35.0f)
								&& (canKick || hasShoppingBags)) {

								nextFightMove = FIGHTMOVE_KICK;
								if (hasShoppingBags) {
									nextFightMove = FIGHTMOVE_ROUNDHOUSE;
								} else if (canRoundhouse && CGeneral::GetRandomNumber() & 1) {
									nextFightMove = FIGHTMOVE_ROUNDHOUSE;
								}
								canAffectMultiplePeople = false;
							} else if (nearPedDist < 2.0f && neededTurn < DEGTORAD(30.0f) && canKick) {
								canAffectMultiplePeople = false;
								nextFightMove = FIGHTMOVE_LONGKICK;
							} else if (neededTurn < DEGTORAD(30.0f)) {
								goForward = true;
							}
						} else {
							nextFightMove += 2; // Makes it 6 or 7
							if (punchOnly)
								nextFightMove = FIGHTMOVE_PUNCHJAB;

							canAffectMultiplePeople = false;
						}
					} else if (!CGame::nastyGame
						|| nearPedDist >= 1.3f
						|| neededTurn >= DEGTORAD(55.0f)
						|| punchOnly) {

						if (nearPedDist > 0.8f
							&& nearPedDist < 3.0f
							&& neededTurn < DEGTORAD(30.0f)) {
							goForward = true;
						}

					} else if (nearPed->m_nPedState != PED_DEAD || pedOnGround) {
						if (!nearPed->IsPedHeadAbovePos(-0.3f)) {
							canAffectMultiplePeople = false;
							nextFightMove = FIGHTMOVE_GROUNDKICK;
						}

					} else {
						pedOnGround = nearPed;
						kickGround = true;
						angleForGroundKick = nextAngle;
					}
				}

				if (!canAffectMultiplePeople) {
					m_fRotationDest = nextAngle;
					if (IsPlayer()) {
						m_fRotationCur = m_fRotationDest;
						m_lookTimer = 0;
						SetLookFlag(nearPed, true);
						SetLookTimer(1500);
					}
					break;
				}
			}
		} else {
			// Because we're in a scripted fight with some particular ped.
			canAffectMultiplePeople = false;

			float fightingPedDist = (m_pedInObjective->GetPosition() - GetPosition()).Magnitude();
			if (hasShoppingBags) {
				if (fightingPedDist >= 1.7f)
					nextFightMove = FIGHTMOVE_SHUFFLE_F;
				else
					nextFightMove = FIGHTMOVE_ROUNDHOUSE;

			} else if (punchOnly) {
				if (fightingPedDist >= 1.3f)
					nextFightMove = FIGHTMOVE_SHUFFLE_F;
				else
					nextFightMove = FIGHTMOVE_PUNCHJAB;

			} else if (fightingPedDist >= 3.0f) {
				nextFightMove = FIGHTMOVE_STDPUNCH;

			} else {
				angleToFace = CGeneral::GetRadianAngleBetweenPoints(
					m_pedInObjective->GetPosition().x,
					m_pedInObjective->GetPosition().y,
					GetPosition().x,
					GetPosition().y);

				nextAngle = CGeneral::LimitRadianAngle(angleToFace);
				m_fRotationDest = nextAngle;
				m_fRotationCur = m_fRotationDest;
				if (!m_pedInObjective->OnGroundOrGettingUp()) {

					if (fightingPedDist >= 0.8f || !canKneeHead) {

						if (fightingPedDist >= 1.3f) {

							if (fightingPedDist < 1.7f && canKick) {
								nextFightMove = FIGHTMOVE_KICK;
								if (canRoundhouse && CGeneral::GetRandomNumber() & 1)
									nextFightMove = FIGHTMOVE_ROUNDHOUSE;

							} else if (fightingPedDist < 2.0f && canKick) {
								nextFightMove += 5; // Makes it 9 or 10

							} else {
								nextFightMove = FIGHTMOVE_SHUFFLE_F;

							}
						} else {
							nextFightMove += 2; // Makes it 6 or 7
						}
					}
				} else if (!CGame::nastyGame
					|| fightingPedDist >= 1.3f
					|| m_pedInObjective->IsPlayer()
					|| m_pedInObjective->m_nPedState != PED_DEAD && m_pedInObjective->IsPedHeadAbovePos(-0.3f)) {
					nextFightMove = FIGHTMOVE_IDLE;
				} else {
					nextFightMove = FIGHTMOVE_GROUNDKICK;
				}
			}
		}

		if (canAffectMultiplePeople) {
			if (kickGround && IsPlayer()) {
				m_fRotationDest = angleForGroundKick;
				nextFightMove = FIGHTMOVE_GROUNDKICK;
				m_fRotationCur = m_fRotationDest;
				m_lookTimer = 0;
				SetLookFlag(pedOnGround, true);
				SetLookTimer(1500);
			} else if (goForward) {
				nextFightMove = FIGHTMOVE_SHUFFLE_F;
			} else {
				nextFightMove = FIGHTMOVE_STDPUNCH;
			}
		}

		if (nextFightMove != FIGHTMOVE_IDLE) {
			m_curFightMove = nextFightMove;
			animAssoc = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, tFightMoves[m_curFightMove].animId, 4.0f);

			animAssoc->SetFinishCallback(FinishFightMoveCB, this);
			if (m_fightState == FIGHTSTATE_MOVE_FINISHED && animAssoc->currentTime != 0.0f) {
				animAssoc->SetCurrentTime(0.0f);
				animAssoc->SetRun();
			}
			m_fightButtonPressure = 0;
		}
		m_fightState = FIGHTSTATE_NO_MOVE;
	} else if (m_takeAStepAfterAttack && m_curFightMove != FIGHTMOVE_SHUFFLE_F
#ifndef FIX_BUGS
		&& CheckForPedsOnGroundToAttack(this, nil) == 4) {
#else
		&& CheckForPedsOnGroundToAttack(this, nil) == PED_IN_FRONT_OF_ATTACKER) {
#endif
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
		m_curFightMove = FIGHTMOVE_IDLE;
		if (IsPlayer())
			m_nWaitTimer = CTimer::GetTimeInMilliseconds() + 500;
		else
			m_nWaitTimer = CTimer::GetTimeInMilliseconds() + 2000;
	}
}

void
CPed::EndFight(uint8 endType)
{
	if (m_nPedState != PED_FIGHT)
		return;

	m_curFightMove = FIGHTMOVE_NULL;
	RestorePreviousState();
	CAnimBlendAssociation *animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_FIGHT_IDLE);
	if (animAssoc)
		animAssoc->flags |= ASSOC_DELETEFADEDOUT;

	switch (endType) {
		case ENDFIGHT_NORMAL:
			CAnimManager::BlendAnimation(GetClump(), m_animGroup, ANIM_STD_IDLE, 8.0f);
			CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_FIGHT_2IDLE, 8.0f);
			break;
		case ENDFIGHT_WITH_A_STEP:
			CAnimManager::BlendAnimation(GetClump(), m_animGroup, ANIM_STD_IDLE, 1.0f);
			CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_STARTWALK, 8.0f);
			break;
		case ENDFIGHT_FAST:
			CAnimManager::BlendAnimation(GetClump(), m_animGroup, ANIM_STD_IDLE, 8.0f);
			CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_FIGHT_2IDLE, 8.0f)->speed = 2.0f;
			break;
		default:
			break;
	}
	m_nWaitTimer = 0;
}


void
CPed::PlayHitSound(CPed *hitTo)
{
	// That was very complicated to reverse for me...
	// First index is our fight move ID (from 1 to 12, total 12), second is the one of we fight with (from 13 to 22, total 10).
	enum {
		S33 = SOUND_FIGHT_PUNCH_33,
		S34 = SOUND_FIGHT_KICK_34,
		S35 = SOUND_FIGHT_HEADBUTT_35,
		S36 = SOUND_FIGHT_PUNCH_36,
		S37 = SOUND_FIGHT_PUNCH_37,
		S38 = SOUND_FIGHT_CLOSE_PUNCH_38,
		S39 = SOUND_FIGHT_PUNCH_39,
		S40 = SOUND_FIGHT_PUNCH_OR_KICK_BELOW_40 ,
		S41 = SOUND_FIGHT_PUNCH_41,
		S42 = SOUND_FIGHT_PUNCH_FROM_BEHIND_42,
		S43 = SOUND_FIGHT_KNEE_OR_KICK_43,
		S44 = SOUND_FIGHT_KICK_44,
		NO_SND = SOUND_NO_SOUND
	};
	uint16 hitSoundsByFightMoves[12][10] = {
		{S39,S42,S43,S43,S39,S39,S39,S39,S39,S42},
		{NO_SND,NO_SND,NO_SND,NO_SND,NO_SND,NO_SND,NO_SND,NO_SND,NO_SND,NO_SND},
		{NO_SND,NO_SND,NO_SND,NO_SND,NO_SND,NO_SND,NO_SND,NO_SND,NO_SND,NO_SND},
		{S39,S39,S39,S39,S33,S43,S39,S39,S39,S39},
		{S39,S39,S39,S39,S35,S39,S38,S38,S39,S39},
		{S39,S39,S39,S39,S33,S39,S41,S36,S39,S39},
		{S39,S39,S39,S39,S37,S40,S38,S38,S39,S39},
		{S39,S39,S39,S39,S34,S43,S44,S37,S39,S39},
		{S39,S39,S39,S39,S34,S43,S44,S37,S39,S39},
		{S39,S39,S39,S39,S34,S43,S44,S37,S39,S40},
		{S39,S39,S39,S39,S33,S39,S41,S37,S39,S40},
		{S39,S39,S39,S39,S39,S39,S39,S39,S33,S33}
	};

	// This is why first dimension is between FightMove 1 and 12.
	if (m_curFightMove == FIGHTMOVE_NULL || m_curFightMove >= FIGHTMOVE_HITFRONT)
		return;

	uint16 soundId;

	// And this is why second dimension is between 13 and 22.
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
		DMAudio.PlayOneShot(m_audioEntityId, soundId, 0.0f);
}

bool
CPed::FightStrike(CVector &touchedNodePos)
{
	CColModel *ourCol;
	CVector attackDistance;
	ePedPieceTypes closestPedPiece = PEDPIECE_TORSO;
	float maxDistanceToBeBeaten;
	CPed *nearPed;
	int state = m_fightState;
	bool pedFound = false;

	if (state == FIGHTSTATE_JUST_ATTACKED)
		return false;

	// Pointless code
	if (state > FIGHTSTATE_NO_MOVE)
		attackDistance = touchedNodePos - m_vecHitLastPos;

	for (int i = 0; i < m_numNearPeds; i++) {
		nearPed = m_nearPeds[i];
		if (GetWeapon()->m_eWeaponType != WEAPONTYPE_UNARMED)
			maxDistanceToBeBeaten = nearPed->GetBoundRadius() + tFightMoves[m_curFightMove].strikeRadius + 0.1f;
		else
			maxDistanceToBeBeaten = nearPed->GetBoundRadius() + tFightMoves[m_curFightMove].strikeRadius;

		if (nearPed->bUsesCollision || nearPed->m_nPedState == PED_DEAD) {
			CVector nearPedCentre;
			nearPed->GetBoundCentre(nearPedCentre);
			CVector potentialAttackDistance = nearPedCentre - touchedNodePos;

			// He can beat us
			if (sq(maxDistanceToBeBeaten) > potentialAttackDistance.MagnitudeSqr()) {

#ifdef PED_SKIN
				// Have to animate a skinned clump because the initial col model is useless
				if(IsClumpSkinned(GetClump()))
					ourCol = ((CPedModelInfo *)CModelInfo::GetModelInfo(GetModelIndex()))->AnimatePedColModelSkinned(GetClump());
				else
#endif
				if (nearPed->OnGround() || !nearPed->IsPedHeadAbovePos(-0.3f)) {
					ourCol = &CTempColModels::ms_colModelPedGroundHit;
				} else {
#ifdef ANIMATE_PED_COL_MODEL
					ourCol = CPedModelInfo::AnimatePedColModel(((CPedModelInfo *)CModelInfo::GetModelInfo(GetModelIndex()))->GetHitColModel(),
					                                           RpClumpGetFrame(GetClump()));
#else
					ourCol = ((CPedModelInfo*)CModelInfo::GetModelInfo(m_modelIndex))->GetHitColModel();
#endif
				}

				for (int j = 0; j < ourCol->numSpheres; j++) {
					attackDistance = nearPed->GetPosition() + ourCol->spheres[j].center;
					attackDistance -= touchedNodePos;
					CColSphere *ourPieces = ourCol->spheres;
					float maxDistanceToBeat = ourPieces[j].radius + tFightMoves[m_curFightMove].strikeRadius;

					// We can beat him too
					if (sq(maxDistanceToBeat) > attackDistance.MagnitudeSqr()) {
						pedFound = true;
						closestPedPiece = (ePedPieceTypes) ourPieces[j].piece;
						break;
					}
				}
			}
		}
		if (pedFound)
			break;
	}

	if (pedFound) {
		if (nearPed->IsPlayer() && nearPed->m_nPedState == PED_GETUP)
			return false;

		float oldVictimHealth = nearPed->m_fHealth;
		CVector bloodPos = 0.5f * attackDistance + touchedNodePos;
		int damageMult = tFightMoves[m_curFightMove].damage * ((CGeneral::GetRandomNumber() & 1) + 2) + 1;

		CVector2D diff (GetPosition() - nearPed->GetPosition());
		int direction = nearPed->GetLocalDirection(diff);
		if (IsPlayer()) {
			if (((CPlayerPed*)this)->m_bAdrenalineActive)
				damageMult = 20;
		} else {
			damageMult *= m_pedStats->m_attackStrength;
		}

		// Change direction if we used kick.
		if (m_curFightMove == FIGHTMOVE_KICK) {
			if (CGeneral::GetRandomNumber() & 1) {
				direction++;
				if (direction > 3)
					direction -= 4;
			}
		}
		nearPed->ReactToAttack(this);

		// Mostly unused. if > 5, ANIM_HIT_WALK will be run, that's it.
		int unk2;
		if (GetWeapon()->m_eWeaponType != WEAPONTYPE_UNARMED && !nearPed->IsPlayer())
			unk2 = 101;
		else
			unk2 = damageMult;

		nearPed->StartFightDefend(direction, tFightMoves[m_curFightMove].hitLevel, unk2);
		PlayHitSound(nearPed);
		m_fightState = FIGHTSTATE_JUST_ATTACKED;
		RpAnimBlendClumpGetAssociation(GetClump(), tFightMoves[m_curFightMove].animId)->speed = 0.6f;
		if (!nearPed->DyingOrDead()) {
			nearPed->InflictDamage(this, WEAPONTYPE_UNARMED, damageMult * 3.0f, closestPedPiece, direction);
		}

		if (CGame::nastyGame
			&& tFightMoves[m_curFightMove].hitLevel > HITLEVEL_MEDIUM
			&& nearPed->m_nPedState == PED_DIE
			&& nearPed->GetIsOnScreen()) {

			// Just for blood particle. We will restore it below.
			attackDistance /= (10.0f * attackDistance.Magnitude());
			for(int i=0; i<4; i++) {
				CParticle::AddParticle(PARTICLE_BLOOD, bloodPos, attackDistance, nil, 0.0f, 0, 0, 0, 0);
			}
		}
		if (!nearPed->OnGround()) {
			float curVictimHealth = nearPed->m_fHealth;
			if (curVictimHealth > 0.0f
				&& (curVictimHealth < 40.0f && oldVictimHealth > 40.0f && !nearPed->IsPlayer()
					|| nearPed->m_fHealth < 20.0f && oldVictimHealth > 20.0f
					|| GetWeapon()->m_eWeaponType != WEAPONTYPE_UNARMED && IsPlayer()
					|| nearPed->m_pedStats->m_flags & STAT_ONE_HIT_KNOCKDOWN)) {

				nearPed->SetFall(0, (AnimationId)(direction + ANIM_STD_HIGHIMPACT_FRONT), 0);
				if (nearPed->m_nPedState == PED_FALL)
					nearPed->bIsStanding = false;
			}
		}
		if (nearPed->m_nPedState == PED_DIE || !nearPed->bIsStanding) {
			attackDistance = nearPed->GetPosition() - GetPosition();
			attackDistance.Normalise();
			attackDistance.z = 1.0f;
			nearPed->bIsStanding = false;

			float moveMult;
			if (m_curFightMove == FIGHTMOVE_GROUNDKICK) {
				moveMult = Min(damageMult * 0.6f, 4.0f);
			} else {
				if (nearPed->m_nPedState != PED_DIE || damageMult >= 20) {
					moveMult = damageMult;
				} else {
					moveMult = Min(damageMult * 2.0f, 14.0f);
				}
			}

			nearPed->ApplyMoveForce(moveMult * 0.6f * attackDistance);
		}
		CEventList::RegisterEvent(nearPed->m_nPedType == PEDTYPE_COP ? EVENT_ASSAULT_POLICE : EVENT_ASSAULT, EVENT_ENTITY_PED, nearPed, this, 2000);
	}

	if (m_fightState == FIGHTSTATE_NO_MOVE)
		m_fightState = FIGHTSTATE_1;

	m_vecHitLastPos = touchedNodePos;
	return false;
}

void
CPed::FinishFightMoveCB(CAnimBlendAssociation *animAssoc, void *arg)
{
	CPed *ped = (CPed*)arg;

	if (tFightMoves[ped->m_curFightMove].animId == animAssoc->animId) {
		ped->m_fightState = FIGHTSTATE_MOVE_FINISHED;
		animAssoc->blendDelta = -1000.0f;
	}
}

void
CPed::LoadFightData(void)
{
	float startFireTime, endFireTime, comboFollowOnTime, strikeRadius;
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
			"%s %f %f %f %f %c %s %d %d",
			moveName,
			&startFireTime,
			&endFireTime,
			&comboFollowOnTime,
			&strikeRadius,
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

		if (strcmp(animName, "null") != 0) {
			animAssoc = CAnimManager::GetAnimAssociation(ASSOCGRP_STD, animName);
			tFightMoves[moveId].animId = (AnimationId)animAssoc->animId;
		} else {
			tFightMoves[moveId].animId = ANIM_STD_WALK;
		}
		moveId++;
	}
}

void
CPed::SetInvestigateEvent(eEventType event, CVector2D pos, float distanceToCountDone, uint16 time, float angle)
{
	if (!IsPedInControl() || CharCreatedBy == MISSION_CHAR)
		return;

	SetStoredState();
	bFindNewNodeAfterStateRestore = false;
	SetPedState(PED_INVESTIGATE);
	m_chatTimer = CTimer::GetTimeInMilliseconds() + time;
	m_eventType = event;
	m_eventOrThreat = pos;
	m_distanceToCountSeekDone = distanceToCountDone;
	m_fAngleToEvent = angle;

	if (m_eventType >= EVENT_ICECREAM)
		m_lookTimer = 0;
	else
		CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_HANDSCOWER, 4.0f);

}

void
CPed::InvestigateEvent(void)
{
	CAnimBlendAssociation *animAssoc;
	AnimationId animToPlay;
	AssocGroupId animGroup;

	if (m_nWaitState == WAITSTATE_TURN180)
		return;

	if (CTimer::GetTimeInMilliseconds() > m_chatTimer) {

		if (m_chatTimer) {
			if (m_eventType < EVENT_ASSAULT_NASTYWEAPON)
				SetWaitState(WAITSTATE_TURN180, nil);

			m_chatTimer = 0;
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
					animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_ROADCROSS);

					if (animAssoc) {
						animAssoc->blendDelta = -8.0f;
						animAssoc->flags |= ASSOC_DELETEFADEDOUT;
						if (m_pEventEntity)
							SetLookFlag(m_pEventEntity, true);

						SetLookTimer(CGeneral::GetRandomNumberInRange(1500, 4000));

					} else if (CGeneral::GetRandomNumber() & 3) {
						ClearLookFlag();
						CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_ROADCROSS, 4.0f);

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
					animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_IDLE_CAM);
					if (!animAssoc)
						animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_IDLE);

					if (animAssoc && animAssoc->animId == ANIM_STD_IDLE_CAM) {
						CAnimManager::BlendAnimation(GetClump(), m_animGroup, ANIM_STD_IDLE, 4.0f);
						SetLookTimer(CGeneral::GetRandomNumberInRange(1000, 2500));

					} else if (CGeneral::GetRandomNumber() & 3) {
						CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_IDLE_CAM, 4.0f);
						SetLookTimer(CGeneral::GetRandomNumberInRange(2500, 5000));
						Say(SOUND_PED_CHAT_EVENT);

					} else {
						m_chatTimer = 0;
					}

				} else if (CTimer::GetTimeInMilliseconds() > m_lookTimer) {
					animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_IDLE);
					if (!animAssoc)
						animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_IDLE_HBHB);

					if (!animAssoc)
						animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_XPRESS_SCRATCH);

					if (animAssoc && animAssoc->animId == ANIM_STD_IDLE) {
						if (CGeneral::GetRandomNumber() & 1)
							animToPlay = ANIM_STD_IDLE_HBHB;
						else
							animToPlay = ANIM_STD_XPRESS_SCRATCH;

						CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, animToPlay, 4.0f);
						SetLookTimer(CGeneral::GetRandomNumberInRange(1500, 4000));

					} else if (animAssoc && animAssoc->animId == ANIM_STD_IDLE_HBHB) {
						animAssoc->blendDelta = -8.0f;
						animAssoc->flags |= ASSOC_DELETEFADEDOUT;
						if (CGeneral::GetRandomNumber() & 1) {
							animToPlay = ANIM_STD_IDLE;
							animGroup = m_animGroup;
						} else {
							animToPlay = ANIM_STD_XPRESS_SCRATCH;
							animGroup = ASSOCGRP_STD;
						}

						CAnimManager::BlendAnimation(GetClump(), animGroup, animToPlay, 4.0f);
						SetLookTimer(CGeneral::GetRandomNumberInRange(1000, 2500));

					} else {
						if (CGeneral::GetRandomNumber() & 1) {
							animToPlay = ANIM_STD_IDLE;
							animGroup = m_animGroup;
						} else {
							animToPlay = ANIM_STD_IDLE_HBHB;
							animGroup = ASSOCGRP_STD;
						}

						CAnimManager::BlendAnimation(GetClump(), animGroup, animToPlay, 4.0f);
						SetLookTimer(CGeneral::GetRandomNumberInRange(1000, 2500));
					}
					Say(SOUND_PED_CHAT_EVENT);
				}
				break;
			case EVENT_ICECREAM:
			case EVENT_SHOPSTALL:

				m_fRotationDest = m_fAngleToEvent;
				if (CTimer::GetTimeInMilliseconds() > m_lookTimer) {

					if (m_lookTimer) {
						animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_ROADCROSS);

						if (animAssoc) {
							animAssoc->blendDelta = -8.0f;
							animAssoc->flags |= ASSOC_DELETEFADEDOUT;
							if (m_eventType == EVENT_ICECREAM)
								animToPlay = ANIM_STD_CHAT;
							else
								animToPlay = ANIM_STD_XPRESS_SCRATCH;
							CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, animToPlay,4.0f);
							SetLookTimer(CGeneral::GetRandomNumberInRange(2000, 5000));

						} else {
							animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_CHAT);
							if (animAssoc) {
								animAssoc->blendDelta = -8.0f;
								animAssoc->flags |= ASSOC_DELETEFADEDOUT;
								ClearInvestigateEvent();
							} else {
								animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_XPRESS_SCRATCH);
								if (animAssoc) {
									animAssoc->blendDelta = -8.0f;
									animAssoc->flags |= ASSOC_DELETEFADEDOUT;
								}
								ClearInvestigateEvent();
							}
						}
					} else {
						CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_ROADCROSS, 4.0f);
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

		for (int i = 0; i < m_numNearPeds; i++) {
			if ((m_eventOrThreat - m_nearPeds[i]->GetPosition()).MagnitudeSqr() < sq(0.4f)) {
				SetMoveState(PEDMOVE_STILL);
				return;
			}
		}

		SetMoveState(PEDMOVE_WALK);
	}
}

void
CPed::ClearInvestigateEvent(void)
{
	CAnimBlendAssociation *animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_ROADCROSS);
	if (!animAssoc)
		animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_XPRESS_SCRATCH);
	if (!animAssoc)
		animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_IDLE_HBHB);
	if (!animAssoc)
		animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_CHAT);
	if (animAssoc) {
		animAssoc->blendDelta = -8.0f;
		animAssoc->flags |= ASSOC_DELETEFADEDOUT;
	}
	if (m_eventType > EVENT_EXPLOSION)
		m_chatTimer = CTimer::GetTimeInMilliseconds() + 15000;

	bGonnaInvestigateEvent = false;
	m_pEventEntity = nil;
	ClearLookFlag();
	RestorePreviousState();
	if(m_nMoveState == PEDMOVE_NONE || m_nMoveState == PEDMOVE_STILL)
		SetMoveState(PEDMOVE_WALK);
}

bool
CPed::InflictDamage(CEntity *damagedBy, eWeaponType method, float damage, ePedPieceTypes pedPiece, uint8 direction)
{
	CPlayerPed *player = FindPlayerPed();
	float dieDelta = 4.0f;
	float dieSpeed = 0.0f;
	AnimationId dieAnim = ANIM_STD_KO_FRONT;
	bool headShot = false;
	bool willLinger = false;
	int random;

	if (player == this) {
		if (!player->m_bCanBeDamaged)
			return false;

		player->AnnoyPlayerPed(false);
	}

	if (DyingOrDead())
		return false;

	if (!bUsesCollision && method != WEAPONTYPE_DROWNING)
		return false;

	if (bOnlyDamagedByPlayer && damagedBy != player && damagedBy != FindPlayerVehicle() &&
		method != WEAPONTYPE_DROWNING && method != WEAPONTYPE_EXPLOSION)
		return false;

	float healthImpact;
	if (IsPlayer())
		healthImpact = damage * 0.33f;
	else
		healthImpact = damage * m_pedStats->m_defendWeakness;

	bool detectDieAnim = true;
	if (m_nPedState == PED_FALL || m_nPedState == PED_GETUP) {
		if (!IsPedHeadAbovePos(-0.3f)) {
			if (RpAnimBlendClumpGetFirstAssociation(GetClump(), ASSOC_FRONTAL))
				dieAnim = ANIM_STD_HIT_FLOOR_FRONT;
			else
				dieAnim = ANIM_STD_HIT_FLOOR;
			dieDelta *= 2.0f;
			dieSpeed = 0.5f;
			detectDieAnim = false;
		} else if (m_nPedState == PED_FALL) {
			dieAnim = ANIM_STD_NUM;
			detectDieAnim = false;
		}
	}
	if (detectDieAnim) {
		switch (method) {
			case WEAPONTYPE_UNARMED:
				if (bMeleeProof)
					return false;

				if (m_nPedState == PED_FALL) {
					if (IsPedHeadAbovePos(-0.3f)) {
						dieAnim = ANIM_STD_NUM;
					} else {
						if (RpAnimBlendClumpGetFirstAssociation(GetClump(), ASSOC_FRONTAL))
							dieAnim = ANIM_STD_HIT_FLOOR_FRONT;
						else
							dieAnim = ANIM_STD_HIT_FLOOR;
						dieDelta = dieDelta * 2.0f;
						dieSpeed = 0.5f;
					}
				} else {
					switch (direction) {
						case 0:
							dieAnim = ANIM_STD_HIGHIMPACT_FRONT;
							break;
						case 1:
							dieAnim = ANIM_STD_HIGHIMPACT_LEFT;
							break;
						case 2:
							dieAnim = ANIM_STD_HIGHIMPACT_BACK;
							break;
						case 3:
							dieAnim = ANIM_STD_HIGHIMPACT_RIGHT;
							break;
						default:
							break;
					}
				}
				break;
			case WEAPONTYPE_BASEBALLBAT:
				if (bMeleeProof)
					return false;

				if (m_nPedState == PED_FALL) {
					if (IsPedHeadAbovePos(-0.3f)) {
						dieAnim = ANIM_STD_NUM;
					} else {
					    if (RpAnimBlendClumpGetFirstAssociation(GetClump(), ASSOC_FRONTAL))
							dieAnim = ANIM_STD_HIT_FLOOR_FRONT;
						else
							dieAnim = ANIM_STD_HIT_FLOOR;
						dieDelta = dieDelta * 2.0f;
						dieSpeed = 0.5f;
					}
				} else {
					switch (direction) {
						case 0:
							dieAnim = ANIM_STD_HIGHIMPACT_FRONT;
							break;
						case 1:
							dieAnim = ANIM_STD_HIGHIMPACT_LEFT;
							break;
						case 2:
							dieAnim = ANIM_STD_HIGHIMPACT_BACK;
							break;
						case 3:
							dieAnim = ANIM_STD_HIGHIMPACT_RIGHT;
							break;
						default:
							break;
					}
				}
				break;
			case WEAPONTYPE_COLT45:
			case WEAPONTYPE_UZI:
			case WEAPONTYPE_SHOTGUN:
			case WEAPONTYPE_AK47:
			case WEAPONTYPE_M16:
			case WEAPONTYPE_SNIPERRIFLE:
				if (bBulletProof)
					return false;

				bool dontRemoveLimb;
				if (IsPlayer() || bNoCriticalHits)
					dontRemoveLimb = true;
				else {
					switch (method) {
						case WEAPONTYPE_SNIPERRIFLE:
							dontRemoveLimb = false;
							break;
						case WEAPONTYPE_M16:
							dontRemoveLimb = false;
							break;
						case WEAPONTYPE_SHOTGUN:
							dontRemoveLimb = CGeneral::GetRandomNumber() & 7;
							break;
						default:
							dontRemoveLimb = CGeneral::GetRandomNumber() & 15;
							break;
					}
				}

				if (dontRemoveLimb) {
					if (method == WEAPONTYPE_SHOTGUN) {
						switch (direction) {
							case 0:
								dieAnim = ANIM_STD_HIGHIMPACT_FRONT;
								break;
							case 1:
								dieAnim = ANIM_STD_HIGHIMPACT_LEFT;
								break;
							case 2:
								dieAnim = ANIM_STD_HIGHIMPACT_BACK;
								break;
							case 3:
								dieAnim = ANIM_STD_HIGHIMPACT_RIGHT;
								break;
							default:
								break;
						}
					} else
						dieAnim = ANIM_STD_KO_FRONT;

					willLinger = false;
				} else {
					switch (pedPiece) {
						case PEDPIECE_TORSO:
							willLinger = false;
							dieAnim = ANIM_STD_KO_FRONT;
							break;
						case PEDPIECE_MID:
							willLinger = false;
							dieAnim = ANIM_STD_KO_SHOT_STOMACH;
							break;
						case PEDPIECE_LEFTARM:
							dieAnim = ANIM_STD_KO_SHOT_ARM_L;
							RemoveBodyPart(PED_UPPERARML, direction);
							willLinger = true;
							break;
						case PEDPIECE_RIGHTARM:
							dieAnim = ANIM_STD_KO_SHOT_ARM_R;
							RemoveBodyPart(PED_UPPERARMR, direction);
							willLinger = true;
							break;
						case PEDPIECE_LEFTLEG:
							dieAnim = ANIM_STD_KO_SHOT_LEG_L;
							RemoveBodyPart(PED_UPPERLEGL, direction);
							willLinger = true;
							break;
						case PEDPIECE_RIGHTLEG:
							dieAnim = ANIM_STD_KO_SHOT_LEG_R;
							RemoveBodyPart(PED_UPPERLEGR, direction);
							willLinger = true;
							break;
						case PEDPIECE_HEAD:
							dieAnim = ANIM_STD_KO_SHOT_FACE;
							RemoveBodyPart(PED_HEAD, direction);
							headShot = true;
							willLinger = true;
							break;
						default:
							break;
					}
				}
				break;
			case WEAPONTYPE_ROCKETLAUNCHER:
			case WEAPONTYPE_GRENADE:
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
						dieAnim = ANIM_STD_HIGHIMPACT_FRONT;
						break;
					case 1:
						dieAnim = ANIM_STD_HIGHIMPACT_LEFT;
						break;
					case 2:
						dieAnim = ANIM_STD_HIGHIMPACT_BACK;
						break;
					case 3:
						dieAnim = ANIM_STD_HIGHIMPACT_RIGHT;
						break;
					default:
						break;
				}
				break;
			case WEAPONTYPE_FLAMETHROWER:
				if (bFireProof)
					return false;

				dieAnim = ANIM_STD_KO_FRONT;
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
						
								dieAnim = ANIM_STD_HIGHIMPACT_RIGHT;
							else
								dieAnim = ANIM_STD_HIGHIMPACT_FRONT;
						} else
							dieAnim = ANIM_STD_HIGHIMPACT_LEFT;

						break;
					case 1:
						if (m_nPedState == PED_DIVE_AWAY)
							dieAnim = ANIM_STD_SPINFORWARD_LEFT;
						else
							dieAnim = ANIM_STD_HIGHIMPACT_LEFT;
						break;
					case 2:
						if ((pedPiece != PEDPIECE_LEFTARM || random <= 1)
							&& (pedPiece != PEDPIECE_MID || random != 1)) {
							if ((pedPiece != PEDPIECE_RIGHTARM || random <= 1)
								&& (pedPiece != PEDPIECE_MID || random != 2)) {
								dieAnim = ANIM_STD_HIGHIMPACT_BACK;
							} else {
								dieAnim = ANIM_STD_SPINFORWARD_RIGHT;
							}
						} else
							dieAnim = ANIM_STD_SPINFORWARD_LEFT;
						break;
					case 3:
						if (m_nPedState == PED_DIVE_AWAY)
							dieAnim = ANIM_STD_SPINFORWARD_RIGHT;
						else
							dieAnim = ANIM_STD_HIGHIMPACT_RIGHT;
						break;
					default:
						break;
				}
				if (damagedBy) {
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
				dieAnim = ANIM_STD_DROWN;
				break;
			case WEAPONTYPE_FALL:
				if (bCollisionProof)
					return false;

				switch (direction) {
					case 0:
						dieAnim = ANIM_STD_HIGHIMPACT_FRONT;
						break;
					case 1:
						dieAnim = ANIM_STD_HIGHIMPACT_LEFT;
						break;
					case 2:
						dieAnim = ANIM_STD_HIGHIMPACT_BACK;
						break;
					case 3:
						dieAnim = ANIM_STD_HIGHIMPACT_RIGHT;
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
	}

	if (m_fHealth - healthImpact >= 1.0f && !willLinger) {
		m_fHealth -= healthImpact;
		return false;
	}

	if (bInVehicle) {
		if (method != WEAPONTYPE_DROWNING) {
#ifdef VC_PED_PORTS
			if (m_pMyVehicle) {
				if (m_pMyVehicle->IsCar() && m_pMyVehicle->pDriver == this) {
					if (m_pMyVehicle->GetStatus() == STATUS_SIMPLE) {
						m_pMyVehicle->SetStatus(STATUS_PHYSICS);
						CCarCtrl::SwitchVehicleToRealPhysics(m_pMyVehicle);
					}
					m_pMyVehicle->AutoPilot.m_nCarMission = MISSION_NONE;
					m_pMyVehicle->AutoPilot.m_nCruiseSpeed = 0;
					m_pMyVehicle->AutoPilot.m_nTempAction = TEMPACT_HANDBRAKESTRAIGHT;
					m_pMyVehicle->AutoPilot.m_nTimeTempAction = CTimer::GetTimeInMilliseconds() + 2000;
				}
				if (m_pMyVehicle->CanPedExitCar()) {
					SetObjective(OBJECTIVE_LEAVE_CAR_AND_DIE, m_pMyVehicle);
				} else {
					m_fHealth = 0.0f;
					if (m_pMyVehicle && m_pMyVehicle->pDriver == this) {
						SetRadioStation();
						m_pMyVehicle->SetStatus(STATUS_ABANDONED);
					}
					SetDie(dieAnim, dieDelta, dieSpeed);
					/*
					if (damagedBy == FindPlayerPed() && damagedBy != this) {
						// PlayerInfo stuff
					}
					*/
				}
				for (int i = 0; i < ARRAY_SIZE(m_pMyVehicle->pPassengers); i++) {
					CPed* passenger = m_pMyVehicle->pPassengers[i];
					if (passenger && passenger != this && damagedBy)
						passenger->ReactToAttack(damagedBy);
				}

				CPed *driverOfVeh = m_pMyVehicle->pDriver;
				if (driverOfVeh && driverOfVeh != this && damagedBy)
					driverOfVeh->ReactToAttack(damagedBy);

				if (damagedBy == FindPlayerPed() || damagedBy && damagedBy == FindPlayerVehicle()) {
					CDarkel::RegisterKillByPlayer(this, method, headShot);
					m_threatEntity = FindPlayerPed();
				} else {
					CDarkel::RegisterKillNotByPlayer(this, method);
				}
			}
#endif
			m_fHealth = 1.0f;
			return false;
		}
		m_fHealth = 0.0f;
		if (player == this)
			m_pMyVehicle->SetStatus(STATUS_PLAYER_DISABLED);

		SetDie(ANIM_STD_NUM, 4.0f, 0.0f);
		return true;
	} else {
		m_fHealth = 0.0f;
		SetDie(dieAnim, dieDelta, dieSpeed);

		if (damagedBy == player || damagedBy && damagedBy == FindPlayerVehicle()) {

			// There are PlayerInfo stuff here in VC
			CDarkel::RegisterKillByPlayer(this, method, headShot);
			m_threatEntity = player;
		} else {
			CDarkel::RegisterKillNotByPlayer(this, method);
		}
		if (method == WEAPONTYPE_DROWNING)
			bIsInTheAir = false;

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

void
CPed::RemoveBodyPart(PedNode nodeId, int8 direction)
{
	RwFrame *frame;
	CVector pos;

	frame = m_pFrames[nodeId]->frame;
	if (frame) {
		if (CGame::nastyGame) {
#ifdef PED_SKIN
			if(!IsClumpSkinned(GetClump()))
#endif
			{
#ifdef DEBUGMENU
				if (bPopHeadsOnHeadshot || nodeId != PED_HEAD)
#else
				if (nodeId != PED_HEAD)
#endif
					SpawnFlyingComponent(nodeId, direction);

				RecurseFrameChildrenVisibilityCB(frame, nil);
			}
			pos.x = 0.0f;
			pos.y = 0.0f;
			pos.z = 0.0f;
			TransformToNode(pos, PED_HEAD);

			if (CEntity::GetIsOnScreen()) {
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

CObject*
CPed::SpawnFlyingComponent(int pedNode, int8 direction)
{
	if (CObject::nNoTempObjects >= NUMTEMPOBJECTS)
		return nil;

#ifdef PED_SKIN
	assert(!IsClumpSkinned(GetClump()));
#endif

	CObject *obj = new CObject();
	if (!obj)
		return nil;

	RwFrame *frame = RwFrameCreate();
	RpClump *clump = RpClumpCreate();
	RpClumpSetFrame(clump, frame);
	RwMatrix *matrix = RwFrameGetLTM(m_pFrames[pedNode]->frame);
	*RwFrameGetMatrix(frame) = *matrix;

	flyingClumpTemp = clump;
	RwFrameForAllObjects(m_pFrames[pedNode]->frame, CloneAtomicToFrameCB, frame);
	RwFrameForAllChildren(m_pFrames[pedNode]->frame, RecurseFrameChildrenToCloneCB, frame);
	flyingClumpTemp = nil;
	switch (pedNode) {
		case PED_HEAD:
			// So popping head would have wheel collision. They disabled it anyway
			obj->SetModelIndexNoCreate(MI_CAR_WHEEL);
			break;
		case PED_UPPERARML:
		case PED_UPPERARMR:
			obj->SetModelIndexNoCreate(MI_BODYPARTB);
			obj->SetCenterOfMass(0.25f, 0.0f, 0.0f);
			break;
		case PED_UPPERLEGL:
		case PED_UPPERLEGR:
			obj->SetModelIndexNoCreate(MI_BODYPARTA);
			obj->SetCenterOfMass(0.4f, 0.0f, 0.0f);
			break;
		default:
			break;
	}
	obj->RefModelInfo(GetModelIndex());
	obj->AttachToRwObject((RwObject*)clump);
	obj->m_fMass = 15.0f;
	obj->m_fTurnMass = 5.0f;
	obj->m_fAirResistance = 0.99f;
	obj->m_fElasticity = 0.03f;
	obj->m_fBuoyancy = m_fMass*GRAVITY/0.75f;
	obj->ObjectCreatedBy = TEMP_OBJECT;
	obj->SetIsStatic(false);
	obj->bIsPickup = false;
	obj->m_nSpecialCollisionResponseCases = COLLRESPONSE_SMALLBOX;

	// life time - the more objects the are, the shorter this one will live
	CObject::nNoTempObjects++;
	if (CObject::nNoTempObjects > 20)
		obj->m_nEndOfLifeTime = CTimer::GetTimeInMilliseconds() + 12000;
	else if (CObject::nNoTempObjects > 10)
		obj->m_nEndOfLifeTime = CTimer::GetTimeInMilliseconds() + 30000;
	else
		obj->m_nEndOfLifeTime = CTimer::GetTimeInMilliseconds() + 60000;

	CVector localForcePos, forceDir;

	if (direction == 2) {
		obj->m_vecMoveSpeed = 0.03f * GetForward();
		obj->m_vecMoveSpeed.z = (CGeneral::GetRandomNumber() & 0x3F) * 0.001f;
		obj->m_vecTurnSpeed = CVector(0.0f, 0.0f, 0.0f);
		localForcePos = CVector(0.0f, 0.0f, 0.0f);
		forceDir = GetForward();
	} else {
		obj->m_vecMoveSpeed = -0.03f * GetForward();
		obj->m_vecMoveSpeed.z = (CGeneral::GetRandomNumber() & 0x3F) * 0.001f;
		obj->m_vecTurnSpeed = CVector(0.0f, 0.0f, 0.0f);
		localForcePos = CVector(0.0f, 0.0f, 0.0f);
		forceDir = -GetForward();
	}
	obj->ApplyTurnForce(forceDir, localForcePos);
	CWorld::Add(obj);

	return obj;
}

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
			SetDie(ANIM_STD_KO_FRONT, 4.0f, 0.0f);
		// }

		bBodyPartJustCameOff = true;
		m_nPedStateTimer = CTimer::GetTimeInMilliseconds() + 150;

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

uint8
CPed::DoesLOSBulletHitPed(CColPoint &colPoint)
{
#ifdef FIX_BUGS
	return 1;
#else
	uint8 retVal = 2;

	float headZ = GetNodePosition(PED_HEAD).z;

	if (m_nPedState == PED_FALL)
		retVal = 1;

	float colZ = colPoint.point.z;
	if (colZ < headZ)
		retVal = 1;

	if (headZ + 0.2f <= colZ)
		retVal = 0;

	return retVal;
#endif
}

bool
CPed::IsPedHeadAbovePos(float zOffset)
{
	return zOffset + GetPosition().z < GetNodePosition(PED_HEAD).z;
}

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
		bool weDontLookToHim = DotProduct(posDiff, GetForward()) > 0.0f;
		bool heLooksToUs = DotProduct(posDiff, collideWith->GetForward()) < 0.0f;

		if (m_nMoveState != PEDMOVE_NONE && m_nMoveState != PEDMOVE_STILL) {

			if ((!IsPlayer() || ((CPlayerPed*)this)->m_fMoveSpeed <= 1.8f)
				&& (IsPlayer() || heIsMissionChar && weAreMissionChar || m_nMoveState != PEDMOVE_RUN && m_nMoveState != PEDMOVE_SPRINT
#ifdef VC_PED_PORTS
					|| m_objective == OBJECTIVE_FOLLOW_CHAR_IN_FORMATION && m_pedInObjective == collideWith
					|| collideWith->m_objective == OBJECTIVE_FOLLOW_CHAR_IN_FORMATION && collideWith->m_pedInObjective == this
#endif			
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
								if (weAreMissionChar || m_pedStats->m_fear <= 100 - collideWith->m_pedStats->m_temper
									|| (collideWith->IsPlayer() || collideWith->m_nMoveState == PEDMOVE_NONE || collideWith->m_nMoveState == PEDMOVE_STILL) &&
									(!collideWith->IsPlayer() || ((CPlayerPed*)collideWith)->m_fMoveSpeed <= 1.0f)) {
									SetDirectionToWalkAroundObject(collideWith);
									if (!weAreMissionChar)
										Say(SOUND_PED_CHAT);
								} else {
									SetEvasiveStep(collideWith, 2);
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
#ifdef VC_PED_PORTS
								m_fRotationCur = 0.3f + m_fRotationCur;
								m_fRotationDest = m_fRotationCur;
#endif
							}
							m_nPedStateTimer = CTimer::GetTimeInMilliseconds() + CGeneral::GetRandomNumberInRange(250, 450);
						}
					}
				} else {
#ifdef VC_PED_PORTS
					if (m_pedInObjective && (collideWith == m_pedInObjective || collideWith->m_pedInObjective == m_pedInObjective) && CTimer::GetTimeInMilliseconds() > m_nPedStateTimer) {
#else
					if (m_pedInObjective && collideWith == m_pedInObjective && CTimer::GetTimeInMilliseconds() > m_nPedStateTimer) {
#endif
						if (heLooksToUs) {
							SetEvasiveStep(collideWith, 1);
							m_nPedStateTimer = CTimer::GetTimeInMilliseconds() + 3000;
						}
					} else if (weDontLookToHim && IsPedInControl()) {

						if (m_pedStats != collideWith->m_pedStats) {

							if (collideWith->m_pedStats->m_fear <= 100 - m_pedStats->m_temper
#ifdef VC_PED_PORTS
								|| collideWith->IsPlayer() || CTimer::GetTimeInMilliseconds() < m_nPedStateTimer
#endif						
								) {

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
								animAssoc = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_PARTIAL_PUNCH, 8.0f);
								animAssoc->flags |= ASSOC_FADEOUTWHENDONE;
#ifdef VC_PED_PORTS
								m_nPedStateTimer = CTimer::GetTimeInMilliseconds() + 2000;
#endif
								if (!heIsMissionChar) {
									CVector2D posDiff2D(posDiff);
									int direction = collideWith->GetLocalDirection(posDiff2D);
									collideWith->StartFightDefend(direction, HITLEVEL_HIGH, 5);
								}
							}
						}
					}
				}
			} else if (collideWith->m_pedStats->m_defendWeakness <= 1.5f || heIsMissionChar
#ifdef VC_PED_PORTS
			|| m_pedStats->m_defendWeakness <= collideWith->m_pedStats->m_defendWeakness
#endif
			) {
				// He looks us and we're not at his right side
				if (heLooksToUs && DotProduct(posDiff,collideWith->GetRight()) > 0.0f) {
					CVector moveForce = GetRight();
					moveForce.z += 0.1f;
					ApplyMoveForce(moveForce);
					if (collideWith->m_nMoveState != PEDMOVE_RUN && collideWith->m_nMoveState != PEDMOVE_SPRINT)
						animToPlay = ANIM_STD_HIT_LEFT;
					else
						animToPlay = ANIM_STD_HITBYGUN_LEFT;
				} else if (heLooksToUs) {
					CVector moveForce = GetRight() * -1.0f;
					moveForce.z += 0.1f;
					ApplyMoveForce(moveForce);
					if (collideWith->m_nMoveState != PEDMOVE_RUN && collideWith->m_nMoveState != PEDMOVE_SPRINT)
						animToPlay = ANIM_STD_HIT_RIGHT;
					else
						animToPlay = ANIM_STD_HITBYGUN_RIGHT;
				} else {
					if (collideWith->m_nMoveState != PEDMOVE_RUN && collideWith->m_nMoveState != PEDMOVE_SPRINT)
						animToPlay = ANIM_STD_HIT_BACK;
					else
						animToPlay = ANIM_STD_HITBYGUN_BACK;
				}

				if (collideWith->IsPedInControl() && CTimer::GetTimeInMilliseconds() > collideWith->m_nPedStateTimer) {
					animAssoc = CAnimManager::BlendAnimation(collideWith->GetClump(), ASSOCGRP_STD, animToPlay, 8.0f);
					animAssoc->flags |= ASSOC_FADEOUTWHENDONE;
					collideWith->m_nPedStateTimer = CTimer::GetTimeInMilliseconds() + 1000;
					if (m_nPedState == PED_ATTACK)
						DMAudio.PlayOneShot(m_audioEntityId, SOUND_FIGHT_PUNCH_39, 0.0f);
				}
			} else {
				// We're at his right side
				if (DotProduct(posDiff, collideWith->GetRight()) <= 0.0f) {
					CVector moveForce = GetRight() * -1.0f;
					moveForce.z += 0.1f;
					ApplyMoveForce(moveForce);
					if (heLooksToUs)
						animToPlay = ANIM_STD_HIGHIMPACT_RIGHT;
					else
						animToPlay = ANIM_STD_SPINFORWARD_RIGHT;
				} else {
					CVector moveForce = GetRight();
					moveForce.z += 0.1f;
					ApplyMoveForce(moveForce);
					if (heLooksToUs)
						animToPlay = ANIM_STD_HIGHIMPACT_LEFT;
					else
						animToPlay = ANIM_STD_SPINFORWARD_LEFT;
				}

				if (m_nPedState == PED_ATTACK && collideWith->IsPedInControl())
					DMAudio.PlayOneShot(m_audioEntityId, SOUND_FIGHT_PUNCH_39, 0.0f);

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

void
CPed::KillPedWithCar(CVehicle *car, float impulse)
{
	CVehicleModelInfo *vehModel;
	CColModel *vehColModel;
	uint8 damageDir;
	PedNode nodeToDamage;
	eWeaponType killMethod;

	if (m_nPedState == PED_FALL || m_nPedState == PED_DIE) {
		if (!this->m_pCollidingEntity || car->GetStatus() == STATUS_PLAYER)
			this->m_pCollidingEntity = car;
		return;
	}

	if (m_nPedState == PED_DEAD)
		return;

	if (m_pCurSurface) {
		if (m_pCurSurface->IsVehicle() && (((CVehicle*)m_pCurSurface)->m_vehType == VEHICLE_TYPE_BOAT || IsPlayer()))
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
				if ((carFrontAndDistDotProd <= 0.1 || randVal == 1) && randVal != 0) {
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

					// After this point, distVec isn't distVec anymore.
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

					if (car->m_vehType == VEHICLE_TYPE_CAR) {
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

#ifdef VC_PED_PORTS
		distVec *= Min(car->m_fMass / 1400.0f, 1.0f);
#endif
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
		SetFall(1000, (AnimationId)(fallDirection + ANIM_STD_HIGHIMPACT_FRONT), true);

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
#ifdef VC_PED_PORTS
		distVec *= Min(car->m_fMass / 1400.0f, 1.0f);
#endif
		car->ApplyMoveForce(distVec * -60.0f);
		Say(SOUND_PED_DEFEND);
	}

#ifdef VC_PED_PORTS
	// Killing gang members with car wasn't triggering a fight, until now... Taken from VC.
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
#endif
}