#include "common.h"

#include "RwHelper.h"
#include "PlayerPed.h"
#include "Wanted.h"
#include "Fire.h"
#include "DMAudio.h"
#include "Pad.h"
#include "Camera.h"
#include "WeaponEffects.h"
#include "ModelIndices.h"
#include "World.h"
#include "RpAnimBlend.h"
#include "AnimBlendAssociation.h"
#include "General.h"
#include "Pools.h"
#include "Darkel.h"
#include "CarCtrl.h"
#include "MBlur.h"
#include "Streaming.h"

#define PAD_MOVE_TO_GAME_WORLD_MOVE 60.0f

const uint32 CPlayerPed::nSaveStructSize =
#ifdef COMPATIBLE_SAVES
	1520;
#else
	sizeof(CPlayerPed);
#endif

int32 idleAnimBlockIndex;

CPlayerPed::~CPlayerPed()
{
	delete m_pWanted;
}

// --MIAMI: Done except commented out things
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
	m_bSpeedTimerFlag = false;

	// TODO(Miami)
	// if (pPointGunAt)
	//	m_pPointGunAt->CleanUpOldReference(&m_pPointGunAt);

	m_pPointGunAt = nil;
	SetPedState(PED_IDLE);
	m_fMaxStamina = 150.0f;
	m_fCurrentStamina = m_fMaxStamina;
	m_fStaminaProgress = 0.0f;
	m_nEvadeAmount = 0;
	m_pEvadingFrom = nil;
	m_nHitAnimDelayTimer = 0;
	m_fAttackButtonCounter = 0.0f;
	m_bHaveTargetSelected = false;
	m_bHasLockOnTarget = false;
	m_bDrunkVisualsWearOff = true;
	m_bCanBeDamaged = true;
	m_fWalkAngle = 0.0f;
	m_fFPSMoveHeading = 0.0f;
	m_pMinigunTopAtomic = nil;
	m_fGunSpinSpeed = 0.0;
	m_fGunSpinAngle = 0.0;
	m_nTargettableObjects[0] = m_nTargettableObjects[1] = m_nTargettableObjects[2] = m_nTargettableObjects[3] = -1;
	unused1 = false;
	for (int i = 0; i < 6; i++) {
		m_vecSafePos[i] = CVector(0.0f, 0.0f, 0.0f);
		m_pPedAtSafePos[i] = nil;
		m_pCheckPlayers[i] = nil;
	}
	m_nCheckPlayersIndex = 0;
	m_nPadUpPressedInMilliseconds = 0;
	m_nPadDownPressedInMilliseconds = 0;
	idleAnimBlockIndex = CAnimManager::GetAnimationBlockIndex("playidles");
}

void CPlayerPed::ClearWeaponTarget()
{
	if (m_nPedType == PEDTYPE_PLAYER1) {

		// TODO(Miami)
		// if (m_pPointGunAt)
		//	m_pPointGunAt->CleanUpOldReference(&m_pPointGunAt);

		m_pPointGunAt = nil;
		TheCamera.ClearPlayerWeaponMode();
		CWeaponEffects::ClearCrossHair();
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
#ifdef FIX_BUGS
	player->RegisterReference((CEntity**)&CWorld::Players[index].m_pPed);
#endif

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

// --MIAMI: Use that on everywhere except ProcessPlayerWeapon
void
CPlayerPed::MakeChangesForNewWeapon(eWeaponType weapon)
{
	if (m_nPedState == PED_SNIPER_MODE) {
		RestorePreviousState();
		TheCamera.ClearPlayerWeaponMode();
	}
	SetCurrentWeapon(weapon);
	m_nSelectedWepSlot = m_currentWeapon;

	GetWeapon()->m_nAmmoInClip = Min(GetWeapon()->m_nAmmoTotal, CWeaponInfo::GetWeaponInfo(GetWeapon()->m_eWeaponType)->m_nAmountofAmmunition);

	if (!(CWeaponInfo::GetWeaponInfo(GetWeapon()->m_eWeaponType)->m_bCanAim))
		ClearWeaponTarget();

	// WEAPONTYPE_SNIPERRIFLE? Wut?
	CAnimBlendAssociation* weaponAnim = RpAnimBlendClumpGetAssociation(GetClump(), GetPrimaryFireAnim(CWeaponInfo::GetWeaponInfo(WEAPONTYPE_SNIPERRIFLE)));
	if (weaponAnim) {
		weaponAnim->SetRun();
		weaponAnim->flags |= ASSOC_FADEOUTWHENDONE;
	}
	TheCamera.ClearPlayerWeaponMode();
}

// --MIAMI: Done, but this should be only called from ProcessPlayerWeapon
void
CPlayerPed::MakeChangesForNewWeapon(int32 slot)
{
	if(slot != -1)
		MakeChangesForNewWeapon(m_weapons[slot].m_eWeaponType);
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

// --MIAMI: Done
void
CPlayerPed::SetInitialState(void)
{
	m_nDrunkenness = 0;
	m_nFadeDrunkenness = 0;
	CMBlur::ClearDrunkBlur();
	m_nDrunkCountdown = 0;
	m_bAdrenalineActive = false;
	m_nAdrenalineTime = 0;
	CTimer::SetTimeScale(1.0f);
	m_pSeekTarget = nil;
	m_vecSeekPos = CVector(0.0f, 0.0f, 0.0f);
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
	SetPedState(PED_IDLE);
	SetMoveState(PEDMOVE_STILL);
	m_nLastPedState = PED_NONE;
	m_animGroup = ASSOCGRP_PLAYER;
	m_fMoveSpeed = 0.0f;
	m_nSelectedWepSlot = WEAPONTYPE_UNARMED;
	m_nEvadeAmount = 0;
	m_pEvadingFrom = nil;
	bIsPedDieAnimPlaying = false;
	SetRealMoveAnim();
	m_bCanBeDamaged = true;
	m_pedStats->m_temper = 50;
	m_fWalkAngle = 0.0f;
	if (m_attachedTo && !bUsesCollision)
		bUsesCollision = true;

	m_attachedTo = nil;
	m_attachWepAmmo = 0;
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
						if (curSprintAssoc->GetProgress() < 0.5) // double
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


// TODO(Miami)
bool
CPlayerPed::DoWeaponSmoothSpray(void)
{
	if (m_nPedState == PED_ATTACK && !m_pPointGunAt) {
		eWeaponType weapon = GetWeapon()->m_eWeaponType;
		if (weapon == WEAPONTYPE_FLAMETHROWER || weapon == WEAPONTYPE_COLT45 || weapon == WEAPONTYPE_UZI ||
			weapon == WEAPONTYPE_TEC9 || weapon == WEAPONTYPE_SILENCED_INGRAM || weapon == WEAPONTYPE_MP5 ||
			weapon == WEAPONTYPE_SHOTGUN || weapon == WEAPONTYPE_RUGER || weapon == WEAPONTYPE_M4 || weapon == WEAPONTYPE_HELICANNON)
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

	if (weaponUsed->m_eWeaponType != WEAPONTYPE_SHOTGUN && weaponUsed->m_eWeaponType != WEAPONTYPE_RUGER)
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
	GetWeapon()->Update(m_audioEntityId, nil);
}

// --MIAMI: Done except commented thing
// I think R* also used goto in here.
void
CPlayerPed::ProcessWeaponSwitch(CPad *padUsed)
{
	if (CDarkel::FrenzyOnGoing() || m_attachedTo)
		goto switchDetectDone;

	// TODO(Miami): byte_A10B57
	if (!m_pPointGunAt && /* !byte_A10B57 && */ GetWeapon()->m_eWeaponType != WEAPONTYPE_DETONATOR) {
		if (padUsed->CycleWeaponRightJustDown()) {

			if (TheCamera.PlayerWeaponMode.Mode != CCam::MODE_M16_1STPERSON
				&& TheCamera.PlayerWeaponMode.Mode != CCam::MODE_M16_1STPERSON_RUNABOUT
				&& TheCamera.PlayerWeaponMode.Mode != CCam::MODE_SNIPER
				&& TheCamera.PlayerWeaponMode.Mode != CCam::MODE_SNIPER_RUNABOUT
				&& TheCamera.PlayerWeaponMode.Mode != CCam::MODE_ROCKETLAUNCHER
				&& TheCamera.PlayerWeaponMode.Mode != CCam::MODE_ROCKETLAUNCHER_RUNABOUT
				&& TheCamera.PlayerWeaponMode.Mode != CCam::MODE_CAMERA) {

				for (m_nSelectedWepSlot = m_currentWeapon + 1; m_nSelectedWepSlot < TOTAL_WEAPON_SLOTS; ++m_nSelectedWepSlot) {
					if (HasWeaponSlot(m_nSelectedWepSlot) && GetWeapon(m_nSelectedWepSlot).HasWeaponAmmoToBeUsed()) {
						goto spentAmmoCheck;
					}
				}
				m_nSelectedWepSlot = 0;
			}
		} else if (padUsed->CycleWeaponLeftJustDown()) {
			if (TheCamera.PlayerWeaponMode.Mode != CCam::MODE_M16_1STPERSON
				&& TheCamera.PlayerWeaponMode.Mode != CCam::MODE_SNIPER
				&& TheCamera.PlayerWeaponMode.Mode != CCam::MODE_ROCKETLAUNCHER
				&& TheCamera.PlayerWeaponMode.Mode != CCam::MODE_CAMERA) {

				for (m_nSelectedWepSlot = m_currentWeapon - 1; ; --m_nSelectedWepSlot) {
					if (m_nSelectedWepSlot < 0)
						m_nSelectedWepSlot = TOTAL_WEAPON_SLOTS - 1;

					if (m_nSelectedWepSlot == 0)
						break;

					if (HasWeaponSlot(m_nSelectedWepSlot) && GetWeapon(m_nSelectedWepSlot).HasWeaponAmmoToBeUsed()) {
						break;
					}
				}
			}
		}
	}
	
spentAmmoCheck:
	if (CWeaponInfo::GetWeaponInfo(GetWeapon()->m_eWeaponType)->m_eWeaponFire != WEAPON_FIRE_MELEE
		&& (!padUsed->GetWeapon() || GetWeapon()->m_eWeaponType != WEAPONTYPE_MINIGUN)) {
		if (GetWeapon()->m_nAmmoTotal <= 0) {
			if (TheCamera.PlayerWeaponMode.Mode == CCam::MODE_M16_1STPERSON
				|| TheCamera.PlayerWeaponMode.Mode == CCam::MODE_SNIPER
				|| TheCamera.PlayerWeaponMode.Mode == CCam::MODE_ROCKETLAUNCHER)
				return;

			if (GetWeapon()->m_eWeaponType != WEAPONTYPE_DETONATOR
				|| GetWeapon(2).m_eWeaponType != WEAPONTYPE_DETONATOR_GRENADE)
				m_nSelectedWepSlot = m_currentWeapon - 1;
			else
				m_nSelectedWepSlot = 2;

			for (; m_nSelectedWepSlot >= 0; --m_nSelectedWepSlot) {

				// BUG: m_nSelectedWepSlot and GetWeapon(..) takes slot in VC but they compared them against weapon types in whole condition! jeez
#ifdef FIX_BUGS
				if (m_nSelectedWepSlot == 1 || GetWeapon(m_nSelectedWepSlot).m_nAmmoTotal > 0 && m_nSelectedWepSlot != 2) {
#else
				if (m_nSelectedWepSlot == WEAPONTYPE_BASEBALLBAT && GetWeapon(WEAPONTYPE_BASEBALLBAT).m_eWeaponType == WEAPONTYPE_BASEBALLBAT
					|| GetWeapon(m_nSelectedWepSlot).m_nAmmoTotal > 0
					&& m_nSelectedWepSlot != WEAPONTYPE_MOLOTOV && m_nSelectedWepSlot != WEAPONTYPE_GRENADE && m_nSelectedWepSlot != WEAPONTYPE_TEARGAS) {
#endif
					goto switchDetectDone;
				}
			}
			m_nSelectedWepSlot = 0;
		}
	}

switchDetectDone:
	if (m_nSelectedWepSlot != m_currentWeapon) {
		if (m_nPedState != PED_ATTACK && m_nPedState != PED_AIM_GUN && m_nPedState != PED_FIGHT)
			RemoveWeaponAnims(m_currentWeapon, -1000.0f);
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
	GetWeapon()->Update(m_audioEntityId, nil);
}

void
CPlayerPed::PlayerControlFighter(CPad *padUsed)
{
	float leftRight = padUsed->GetPedWalkLeftRight();
	float upDown = padUsed->GetPedWalkUpDown();
	float padMove = CVector2D(leftRight, upDown).Magnitude();

	if (padMove > 0.0f) {
		m_fRotationDest = CGeneral::GetRadianAngleBetweenPoints(0.0f, 0.0f, -leftRight, upDown) - TheCamera.Orientation;
		m_takeAStepAfterAttack = padMove > 2 * PAD_MOVE_TO_GAME_WORLD_MOVE;
		if (padUsed->GetSprint() && padMove > 1 * PAD_MOVE_TO_GAME_WORLD_MOVE)
			bIsAttacking = false;
	}

	if (!CWeaponInfo::GetWeaponInfo(GetWeapon()->m_eWeaponType)->m_bHeavy && padUsed->JumpJustDown()) {
		if (m_nEvadeAmount != 0 && m_pEvadingFrom) {
			SetEvasiveDive((CPhysical*)m_pEvadingFrom, 1);
			m_nEvadeAmount = 0;
			m_pEvadingFrom = nil;
		} else {
			SetJump();
		}
	}
}

void
CPlayerPed::PlayerControl1stPersonRunAround(CPad *padUsed)
{
	float leftRight = padUsed->GetPedWalkLeftRight();
	float upDown = padUsed->GetPedWalkUpDown();
	float padMove = CVector2D(leftRight, upDown).Magnitude();
	float padMoveInGameUnit = padMove / PAD_MOVE_TO_GAME_WORLD_MOVE;
	if (padMoveInGameUnit > 0.0f) {
#ifdef FREE_CAM
		if (!CCamera::bFreeCam)
			m_fRotationDest = CGeneral::LimitRadianAngle(TheCamera.Orientation);
		else
			m_fRotationDest = CGeneral::GetRadianAngleBetweenPoints(0.0f, 0.0f, -leftRight, upDown) - TheCamera.Orientation;
#else
		m_fRotationDest = CGeneral::LimitRadianAngle(TheCamera.Orientation);
#endif
		m_fMoveSpeed = Min(padMoveInGameUnit, 0.07f * CTimer::GetTimeStep() + m_fMoveSpeed);
	} else {
		m_fMoveSpeed = 0.0f;
	}

	if (m_nPedState == PED_JUMP) {
		if (bIsInTheAir) {
			if (bUsesCollision && !bHitSteepSlope &&
				(!bHitSomethingLastFrame || m_vecDamageNormal.z > 0.6f)
				&& m_fDistanceTravelled < CTimer::GetTimeStep() * 0.02 && m_vecMoveSpeed.MagnitudeSqr() < 0.01f) {

				float angleSin = Sin(m_fRotationCur); // originally sin(DEGTORAD(RADTODEG(m_fRotationCur))) o_O
				float angleCos = Cos(m_fRotationCur);
				ApplyMoveForce(-angleSin * 3.0f, 3.0f * angleCos, 0.05f);
			}
		} else if (bIsLanding) {
			m_fMoveSpeed = 0.0f;
		}
	}
	if (!(CWeaponInfo::GetWeaponInfo(GetWeapon()->m_eWeaponType)->m_bHeavy)
		&& padUsed->GetSprint()) {
		m_nMoveState = PEDMOVE_SPRINT;
	}
	if (m_nPedState != PED_FIGHT)
		SetRealMoveAnim();

	if (!bIsInTheAir && !(CWeaponInfo::GetWeaponInfo(GetWeapon()->m_eWeaponType)->m_bHeavy)
		&& padUsed->JumpJustDown() && m_nPedState != PED_JUMP) {
		ClearAttack();
		ClearWeaponTarget();
		if (m_nEvadeAmount != 0 && m_pEvadingFrom) {
			SetEvasiveDive((CPhysical*)m_pEvadingFrom, 1);
			m_nEvadeAmount = 0;
			m_pEvadingFrom = nil;
		} else {
			SetJump();
		}
	}
}

void
CPlayerPed::KeepAreaAroundPlayerClear(void)
{
	BuildPedLists();
	for (int i = 0; i < m_numNearPeds; ++i) {
		CPed *nearPed = m_nearPeds[i];
		if (nearPed->CharCreatedBy == RANDOM_CHAR && !nearPed->DyingOrDead()) {
			if (nearPed->GetIsOnScreen()) {
				if (nearPed->m_objective == OBJECTIVE_NONE) {
					nearPed->SetFindPathAndFlee(this, 5000, true);
				} else {
					if (nearPed->EnteringCar())
						nearPed->QuitEnteringCar();

					nearPed->ClearObjective();
				}
			} else {
				nearPed->FlagToDestroyWhenNextProcessed();
			}
		}
	}
	CVector playerPos = (InVehicle() ? m_pMyVehicle->GetPosition() : GetPosition());

	CVector pos = GetPosition();
	int16 lastVehicle;
	CEntity *vehicles[8];
	CWorld::FindObjectsInRange(pos, CHECK_NEARBY_THINGS_MAX_DIST, true, &lastVehicle, 6, vehicles, false, true, false, false, false);

	for (int i = 0; i < lastVehicle; i++) {
		CVehicle *veh = (CVehicle*)vehicles[i];
		if (veh->VehicleCreatedBy != MISSION_VEHICLE) {
			if (veh->GetStatus() != STATUS_PLAYER && veh->GetStatus() != STATUS_PLAYER_DISABLED) {
				if ((veh->GetPosition() - playerPos).MagnitudeSqr() > 25.0f) {
					veh->AutoPilot.m_nTempAction = TEMPACT_WAIT;
					veh->AutoPilot.m_nTimeTempAction = CTimer::GetTimeInMilliseconds() + 5000;
				} else {
					if (DotProduct2D(playerPos - veh->GetPosition(), veh->GetForward()) > 0.0f)
						veh->AutoPilot.m_nTempAction = TEMPACT_REVERSE;
					else
						veh->AutoPilot.m_nTempAction = TEMPACT_GOFORWARD;

					veh->AutoPilot.m_nTimeTempAction = CTimer::GetTimeInMilliseconds() + 2000;
				}
				CCarCtrl::PossiblyRemoveVehicle(veh);
			}
		}
	}
}

void
CPlayerPed::EvaluateNeighbouringTarget(CEntity *candidate, CEntity **targetPtr, float *lastCloseness, float distLimit, float angleOffset, bool lookToLeft)
{
	CVector distVec = candidate->GetPosition() - GetPosition();
	if (distVec.Magnitude2D() <= distLimit) {
		if (!DoesTargetHaveToBeBroken(candidate->GetPosition(), GetWeapon())) {
#ifdef VC_PED_PORTS
			float angleBetweenUs = CGeneral::GetATanOfXY(candidate->GetPosition().x - TheCamera.GetPosition().x,
															candidate->GetPosition().y - TheCamera.GetPosition().y);
#else
			float angleBetweenUs = CGeneral::GetATanOfXY(distVec.x, distVec.y);
#endif
			angleBetweenUs = CGeneral::LimitAngle(angleBetweenUs - angleOffset);
			float closeness;
			if (lookToLeft) {
				closeness = angleBetweenUs > 0.0f ? -Abs(angleBetweenUs) : -100000.0f;
			} else {
				closeness = angleBetweenUs > 0.0f ? -100000.0f : -Abs(angleBetweenUs);
			}

			if (closeness > *lastCloseness) {
				*targetPtr = candidate;
				*lastCloseness = closeness;
			}
		}
	}
}

void
CPlayerPed::EvaluateTarget(CEntity *candidate, CEntity **targetPtr, float *lastCloseness, float distLimit, float angleOffset, bool priority)
{
	CVector distVec = candidate->GetPosition() - GetPosition();
	float dist = distVec.Magnitude2D();
	if (dist <= distLimit) {
		if (!DoesTargetHaveToBeBroken(candidate->GetPosition(), GetWeapon())) {
			float angleBetweenUs = CGeneral::GetATanOfXY(distVec.x, distVec.y);
			angleBetweenUs = CGeneral::LimitAngle(angleBetweenUs - angleOffset);

			float closeness = -dist - 5.0f * Abs(angleBetweenUs);
			if (priority) {
				closeness += 5.0f;
			}

			if (closeness > *lastCloseness) {
				*targetPtr = candidate;
				*lastCloseness = closeness;
			}
		}
	}
}

bool
CPlayerPed::FindNextWeaponLockOnTarget(CEntity *previousTarget, bool lookToLeft)
{
	CEntity *nextTarget = nil;
	float weaponRange = CWeaponInfo::GetWeaponInfo(GetWeapon()->m_eWeaponType)->m_fRange;
	// nextTarget = nil;
	float lastCloseness = -10000.0f;
	// unused
	// CGeneral::GetATanOfXY(GetForward().x, GetForward().y);
	CVector distVec = previousTarget->GetPosition() - GetPosition();
	float referenceBeta = CGeneral::GetATanOfXY(distVec.x, distVec.y);

	for (int h = CPools::GetPedPool()->GetSize() - 1; h >= 0; h--) {
		CPed *pedToCheck = CPools::GetPedPool()->GetSlot(h);
		if (pedToCheck) {
			if (pedToCheck != FindPlayerPed() && pedToCheck != previousTarget) {
				if (!pedToCheck->DyingOrDead() && !pedToCheck->bInVehicle
					&& pedToCheck->m_leader != FindPlayerPed() && OurPedCanSeeThisOne(pedToCheck)) {

					EvaluateNeighbouringTarget(pedToCheck, &nextTarget, &lastCloseness,
						weaponRange, referenceBeta, lookToLeft);
				}
			}
		}
	}
	for (int i = 0; i < ARRAY_SIZE(m_nTargettableObjects); i++) {
		CObject *obj = CPools::GetObjectPool()->GetAt(m_nTargettableObjects[i]);
		if (obj)
			EvaluateNeighbouringTarget(obj, &nextTarget, &lastCloseness, weaponRange, referenceBeta, lookToLeft);
	}
	if (!nextTarget)
		return false;

	m_pPointGunAt = nextTarget;
	if (nextTarget)
		nextTarget->RegisterReference((CEntity**)&m_pPointGunAt);
	SetPointGunAt(nextTarget);
	return true;
}

bool
CPlayerPed::FindWeaponLockOnTarget(void)
{
	CEntity *nextTarget = nil;
	float weaponRange = CWeaponInfo::GetWeaponInfo(GetWeapon()->m_eWeaponType)->m_fRange;

	if (m_pPointGunAt) {
		CVector distVec = m_pPointGunAt->GetPosition() - GetPosition();
		if (distVec.Magnitude2D() > weaponRange) {
			m_pPointGunAt = nil;
			return false;
		} else {
			return true;
		}
	}

	// nextTarget = nil;
	float lastCloseness = -10000.0f;
	float referenceBeta = CGeneral::GetATanOfXY(GetForward().x, GetForward().y);
	for (int h = CPools::GetPedPool()->GetSize() - 1; h >= 0; h--) {
		CPed *pedToCheck = CPools::GetPedPool()->GetSlot(h);
		if (pedToCheck) {
			if (pedToCheck != FindPlayerPed()) {
				if (!pedToCheck->DyingOrDead() && !pedToCheck->bInVehicle
					&& pedToCheck->m_leader != FindPlayerPed() && OurPedCanSeeThisOne(pedToCheck)) {

					EvaluateTarget(pedToCheck, &nextTarget, &lastCloseness,
						weaponRange, referenceBeta, IsThisPedAttackingPlayer(pedToCheck));
				}
			}
		}
	}
	for (int i = 0; i < ARRAY_SIZE(m_nTargettableObjects); i++) {
		CObject *obj = CPools::GetObjectPool()->GetAt(m_nTargettableObjects[i]);
		if (obj)
			EvaluateTarget(obj, &nextTarget, &lastCloseness, weaponRange, referenceBeta, false);
	}
	if (!nextTarget)
		return false;

	m_pPointGunAt = nextTarget;
	if (nextTarget)
		nextTarget->RegisterReference((CEntity**)&m_pPointGunAt);
	SetPointGunAt(nextTarget);
	return true;
}

// --MIAMI: Done
void
CPlayerPed::ProcessAnimGroups(void)
{
	AssocGroupId groupToSet;
	if ((m_fWalkAngle <= -DEGTORAD(50.0f) || m_fWalkAngle >= DEGTORAD(50.0f))
		&& TheCamera.Cams[TheCamera.ActiveCam].Using3rdPersonMouseCam()
		&& CanStrafeOrMouseControl()) {

		if (m_fWalkAngle >= -DEGTORAD(130.0f) && m_fWalkAngle <= DEGTORAD(130.0f)) {
			if (m_fWalkAngle > 0.0f) {
				if (GetWeapon()->m_eWeaponType == WEAPONTYPE_ROCKETLAUNCHER)
					groupToSet = ASSOCGRP_ROCKETLEFT;
				else if (GetWeapon()->m_eWeaponType == WEAPONTYPE_CHAINSAW ||
					GetWeapon()->m_eWeaponType == WEAPONTYPE_FLAMETHROWER ||
					GetWeapon()->m_eWeaponType == WEAPONTYPE_MINIGUN)
					groupToSet = ASSOCGRP_CHAINSAWLEFT;
				else
					groupToSet = ASSOCGRP_PLAYERLEFT;
			} else {
				if (GetWeapon()->m_eWeaponType == WEAPONTYPE_ROCKETLAUNCHER)
					groupToSet = ASSOCGRP_ROCKETRIGHT;
				else if (GetWeapon()->m_eWeaponType == WEAPONTYPE_CHAINSAW ||
					GetWeapon()->m_eWeaponType == WEAPONTYPE_FLAMETHROWER ||
					GetWeapon()->m_eWeaponType == WEAPONTYPE_MINIGUN)
					groupToSet = ASSOCGRP_CHAINSAWRIGHT;
				else
					groupToSet = ASSOCGRP_PLAYERRIGHT;
			}
		} else {
			if (GetWeapon()->m_eWeaponType == WEAPONTYPE_ROCKETLAUNCHER)
				groupToSet = ASSOCGRP_ROCKETBACK;
			else if (GetWeapon()->m_eWeaponType == WEAPONTYPE_CHAINSAW ||
				GetWeapon()->m_eWeaponType == WEAPONTYPE_FLAMETHROWER ||
				GetWeapon()->m_eWeaponType == WEAPONTYPE_MINIGUN)
				groupToSet = ASSOCGRP_CHAINSAWBACK;
			else
				groupToSet = ASSOCGRP_PLAYERBACK;
		}
	} else {
		if (GetWeapon()->m_eWeaponType == WEAPONTYPE_ROCKETLAUNCHER) {
			groupToSet = ASSOCGRP_PLAYERROCKET;
		} else {
			if (GetWeapon()->m_eWeaponType == WEAPONTYPE_BASEBALLBAT
				 || GetWeapon()->m_eWeaponType == WEAPONTYPE_MACHETE)
				groupToSet = ASSOCGRP_PLAYERBBBAT;
			else if (GetWeapon()->m_eWeaponType == WEAPONTYPE_CHAINSAW ||
				GetWeapon()->m_eWeaponType == WEAPONTYPE_FLAMETHROWER ||
				GetWeapon()->m_eWeaponType == WEAPONTYPE_MINIGUN)
				groupToSet = ASSOCGRP_PLAYERCHAINSAW;
			else if (GetWeapon()->m_eWeaponType != WEAPONTYPE_COLT45 && GetWeapon()->m_eWeaponType != WEAPONTYPE_UZI
				// I hope this is a inlined function...
				&& GetWeapon()->m_eWeaponType != WEAPONTYPE_PYTHON && GetWeapon()->m_eWeaponType != WEAPONTYPE_TEC9
				&& GetWeapon()->m_eWeaponType != WEAPONTYPE_SILENCED_INGRAM && GetWeapon()->m_eWeaponType != WEAPONTYPE_MP5
				&& GetWeapon()->m_eWeaponType != WEAPONTYPE_GOLFCLUB && GetWeapon()->m_eWeaponType != WEAPONTYPE_KATANA
				&& GetWeapon()->m_eWeaponType != WEAPONTYPE_CAMERA) {
				if (!GetWeapon()->IsType2Handed()) {
					groupToSet = ASSOCGRP_PLAYER;
				} else {
					groupToSet = ASSOCGRP_PLAYER2ARMED;
				}
			} else {
				groupToSet = ASSOCGRP_PLAYER1ARMED;
			}
		}
	}

	if (m_animGroup != groupToSet) {
		m_animGroup = groupToSet;
		ReApplyMoveAnims();
	}
}

// TODO(Miami): Hella TODO
void
CPlayerPed::ProcessPlayerWeapon(CPad *padUsed)
{
	CWeaponInfo *weaponInfo = CWeaponInfo::GetWeaponInfo(GetWeapon()->m_eWeaponType);
	if (m_bHasLockOnTarget && !m_pPointGunAt) {
		TheCamera.ClearPlayerWeaponMode();
		CWeaponEffects::ClearCrossHair();
		ClearPointGunAt();
	}
	if (!m_pFire) {
		if (GetWeapon()->m_eWeaponType == WEAPONTYPE_ROCKETLAUNCHER ||
			GetWeapon()->m_eWeaponType == WEAPONTYPE_SNIPERRIFLE || GetWeapon()->m_eWeaponType == WEAPONTYPE_M4 ||
			GetWeapon()->m_eWeaponType == WEAPONTYPE_RUGER) {
			if (padUsed->TargetJustDown()/* || TheCamera.m_bAllow1rstPersonWeaponsCamera */) { // TODO(Miami): Cam
				SetStoredState();
				m_nPedState = PED_SNIPER_MODE;
#ifdef FREE_CAM
				if (CCamera::bFreeCam && TheCamera.Cams[0].Using3rdPersonMouseCam()) {
					m_fRotationCur = CGeneral::LimitRadianAngle(-TheCamera.Orientation);
					SetHeading(m_fRotationCur);
				}
#endif
				if (GetWeapon()->m_eWeaponType == WEAPONTYPE_ROCKETLAUNCHER)
					TheCamera.SetNewPlayerWeaponMode(CCam::MODE_ROCKETLAUNCHER, 0, 0);
				else if (GetWeapon()->m_eWeaponType == WEAPONTYPE_SNIPERRIFLE)
					TheCamera.SetNewPlayerWeaponMode(CCam::MODE_SNIPER, 0, 0);
				else
					TheCamera.SetNewPlayerWeaponMode(CCam::MODE_M16_1STPERSON, 0, 0);

				m_fMoveSpeed = 0.0f;
				CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_IDLE_STANCE, 1000.0f);
			}
			if (GetWeapon()->m_eWeaponType == WEAPONTYPE_ROCKETLAUNCHER || GetWeapon()->m_eWeaponType == WEAPONTYPE_SNIPERRIFLE
				|| TheCamera.PlayerWeaponMode.Mode == CCam::MODE_M16_1STPERSON)
				return;
		}
	}

	if (padUsed->GetWeapon() && m_nMoveState != PEDMOVE_SPRINT) {
		if (m_nSelectedWepSlot == m_currentWeapon) {
			if (m_pPointGunAt) {
#ifdef FREE_CAM
				if (CCamera::bFreeCam && weaponInfo->m_eWeaponFire == WEAPON_FIRE_MELEE && m_fMoveSpeed < 1.0f)
					StartFightAttack(padUsed->GetWeapon());
				else
#endif
					SetAttack(m_pPointGunAt);
			} else {
				if (m_nPedState == PED_ATTACK) {
					if (padUsed->WeaponJustDown()) {
						m_bHaveTargetSelected = true;
					} else if (!m_bHaveTargetSelected) {
						m_fAttackButtonCounter += CTimer::GetTimeStepNonClipped();
					}
				} else {
					m_fAttackButtonCounter = 0.0f;
					m_bHaveTargetSelected = false;
				}
				if (GetWeapon()->m_eWeaponType != WEAPONTYPE_UNARMED && GetWeapon()->m_eWeaponType != WEAPONTYPE_BRASSKNUCKLE &&
					!weaponInfo->m_bFightMode) {

					if (GetWeapon()->m_eWeaponType != WEAPONTYPE_DETONATOR && GetWeapon()->m_eWeaponType != WEAPONTYPE_DETONATOR_GRENADE ||
						padUsed->WeaponJustDown())

						SetAttack(nil);
				} else if (padUsed->WeaponJustDown()) {
					if (m_fMoveSpeed < 1.0f || m_nPedState == PED_FIGHT)
						StartFightAttack(padUsed->GetWeapon());
					else
						SetAttack(nil);
				}
			}
		}
	} else {
		m_pedIK.m_flags &= ~CPedIK::LOOKAROUND_HEAD_ONLY;
		if (m_nPedState == PED_ATTACK) {
			m_bHaveTargetSelected = true;
			bIsAttacking = false;
		}
	}

#ifdef FREE_CAM
	// Rotate player/arm when shooting. We don't have auto-rotation anymore
	if (CCamera::m_bUseMouse3rdPerson && CCamera::bFreeCam &&
		m_nSelectedWepSlot == m_currentWeapon && m_nMoveState != PEDMOVE_SPRINT) {

		// Weapons except throwable and melee ones
		if (weaponInfo->m_nWeaponSlot > 2) {
			if ((padUsed->GetTarget() && weaponInfo->m_bCanAimWithArm) || padUsed->GetWeapon()) {
				float limitedCam = CGeneral::LimitRadianAngle(-TheCamera.Orientation);

				// On this one we can rotate arm.
				if (weaponInfo->m_bCanAimWithArm) {
					if (!padUsed->GetWeapon()) { // making this State != ATTACK still stops it after attack. Re-start it immediately!
						SetPointGunAt(nil);
						bIsPointingGunAt = false; // to not stop after attack
					}

					SetLookFlag(limitedCam, true);
					SetAimFlag(limitedCam);
#ifdef VC_PED_PORTS
					SetLookTimer(INT32_MAX); // removing this makes head move for real, but I experinced some bugs.
#endif
				} else {
					m_fRotationDest = limitedCam;
					m_headingRate = 12.5f;

					// Anim. fix for shotgun, ak47 and m16 (we must finish rot. it quickly)
					if (weaponInfo->m_bCanAim && padUsed->WeaponJustDown()) {
						m_fRotationCur = CGeneral::LimitRadianAngle(m_fRotationCur);
						float limitedRotDest = m_fRotationDest;

						if (m_fRotationCur - PI > m_fRotationDest) {
							limitedRotDest += 2 * PI;
						} else if (PI + m_fRotationCur < m_fRotationDest) {
							limitedRotDest -= 2 * PI;
						}

						m_fRotationCur += (limitedRotDest - m_fRotationCur) / 2;
					}
				}
			} else if (weaponInfo->m_bCanAimWithArm)
				ClearPointGunAt();
			else
				RestoreHeadingRate();
		}
	}
#endif

	if (padUsed->GetTarget() && m_nSelectedWepSlot == m_currentWeapon && m_nMoveState != PEDMOVE_SPRINT) {
		if (m_pPointGunAt) {
			// what??
			if (!m_pPointGunAt
#ifdef FREE_CAM
				|| (!CCamera::bFreeCam && CCamera::m_bUseMouse3rdPerson)
#else
				|| CCamera::m_bUseMouse3rdPerson
#endif
				|| m_pPointGunAt->IsPed() && ((CPed*)m_pPointGunAt)->bInVehicle) {
				ClearWeaponTarget();
				return;
			}
			if (CPlayerPed::DoesTargetHaveToBeBroken(m_pPointGunAt->GetPosition(), GetWeapon())) {
				ClearWeaponTarget();
				return;
			}
			if (m_pPointGunAt) {
				if (padUsed->ShiftTargetLeftJustDown())
					FindNextWeaponLockOnTarget(m_pPointGunAt, true);
				if (padUsed->ShiftTargetRightJustDown())
					FindNextWeaponLockOnTarget(m_pPointGunAt, false);
			}
			TheCamera.SetNewPlayerWeaponMode(CCam::MODE_SYPHON, 0, 0);
			TheCamera.UpdateAimingCoors(m_pPointGunAt->GetPosition());
		}
#ifdef FREE_CAM
		else if ((CCamera::bFreeCam && weaponInfo->m_eWeaponFire == WEAPON_FIRE_MELEE) || (weaponInfo->m_bCanAim && !CCamera::m_bUseMouse3rdPerson)) {
#else
		else if (weaponInfo->m_bCanAim && !CCamera::m_bUseMouse3rdPerson) {
#endif
			if (padUsed->TargetJustDown()/* || TheCamera.m_bAllow1rstPersonWeaponsCamera */) // TODO(Miami): Cam
				FindWeaponLockOnTarget();
		}
	} else if (m_pPointGunAt) {
		ClearWeaponTarget();
	}

	if (m_pPointGunAt) {
#ifndef VC_PED_PORTS
		CVector markPos = m_pPointGunAt->GetPosition();
#else
		CVector markPos;
		if (m_pPointGunAt->IsPed()) {
			((CPed*)m_pPointGunAt)->m_pedIK.GetComponentPosition(*(RwV3d *)&markPos, PED_MID);
		} else {
			markPos = m_pPointGunAt->GetPosition();
		}
#endif
		if (bCanPointGunAtTarget) {
			CWeaponEffects::MarkTarget(markPos, 64, 0, 0, 255, 0.8f);
		} else {
			CWeaponEffects::MarkTarget(markPos, 64, 32, 0, 255, 0.8f);
		}
	}
	m_bHasLockOnTarget = m_pPointGunAt != nil;
}

void
CPlayerPed::PlayerControlZelda(CPad *padUsed)
{
	bool doSmoothSpray = DoWeaponSmoothSpray();
	float camOrientation = TheCamera.Orientation;
	float leftRight = padUsed->GetPedWalkLeftRight();
	float upDown = padUsed->GetPedWalkUpDown();
	float padMoveInGameUnit;
	bool smoothSprayWithoutMove = false;

	if (doSmoothSpray && upDown > 0.0f) {
		padMoveInGameUnit = 0.0f;
		smoothSprayWithoutMove = true;
	} else {
		padMoveInGameUnit = CVector2D(leftRight, upDown).Magnitude() / PAD_MOVE_TO_GAME_WORLD_MOVE;
	}

	if (padMoveInGameUnit > 0.0f || smoothSprayWithoutMove) {
		float padHeading = CGeneral::GetRadianAngleBetweenPoints(0.0f, 0.0f, -leftRight, upDown);
		float neededTurn = CGeneral::LimitRadianAngle(padHeading - camOrientation);
		if (doSmoothSpray) {
			if (GetWeapon()->m_eWeaponType == WEAPONTYPE_FLAMETHROWER || GetWeapon()->m_eWeaponType == WEAPONTYPE_COLT45
				|| CWeaponInfo::GetWeaponInfo(GetWeapon()->m_eWeaponType)->m_nWeaponSlot == 5)
				m_fRotationDest = m_fRotationCur - leftRight / 128.0f * (PI / 80.0f) * CTimer::GetTimeStep();
			else
				m_fRotationDest = m_fRotationCur - leftRight / 128.0f * (PI / 128.0f) * CTimer::GetTimeStep();
		} else {
			m_fRotationDest = neededTurn;
		}

		float maxAcc = 0.07f * CTimer::GetTimeStep();
		m_fMoveSpeed = Min(padMoveInGameUnit, m_fMoveSpeed + maxAcc);

	} else {
		m_fMoveSpeed = 0.0f;
	}

	if (m_nPedState == PED_JUMP) {
		if (bIsInTheAir) {
			if (bUsesCollision && !bHitSteepSlope &&
				(!bHitSomethingLastFrame || m_vecDamageNormal.z > 0.6f)
				&& m_fDistanceTravelled < CTimer::GetTimeStep() * 0.02 && m_vecMoveSpeed.MagnitudeSqr() < 0.01f) {

				float angleSin = Sin(m_fRotationCur); // originally sin(DEGTORAD(RADTODEG(m_fRotationCur))) o_O
				float angleCos = Cos(m_fRotationCur);
				ApplyMoveForce(-angleSin * 3.0f, 3.0f * angleCos, 0.05f);
			}
		} else if (bIsLanding) {
			m_fMoveSpeed = 0.0f;
		}
	}

	if (!(CWeaponInfo::GetWeaponInfo(GetWeapon()->m_eWeaponType)->m_bHeavy)
		&& padUsed->GetSprint()) {
		m_nMoveState = PEDMOVE_SPRINT;
	}
	if (m_nPedState != PED_FIGHT)
		SetRealMoveAnim();

	if (!bIsInTheAir && !(CWeaponInfo::GetWeaponInfo(GetWeapon()->m_eWeaponType)->m_bHeavy)
		&& padUsed->JumpJustDown() && m_nPedState != PED_JUMP) {
		ClearAttack();
		ClearWeaponTarget();
		if (m_nEvadeAmount != 0 && m_pEvadingFrom) {
			SetEvasiveDive((CPhysical*)m_pEvadingFrom, 1);
			m_nEvadeAmount = 0;
			m_pEvadingFrom = nil;
		} else {
			SetJump();
		}
	}
	PlayIdleAnimations(padUsed);
}

void
CPlayerPed::ProcessControl(void)
{
	if (m_nEvadeAmount != 0)
		--m_nEvadeAmount;

	if (m_nEvadeAmount == 0)
		m_pEvadingFrom = nil;

	if (m_pCurrentPhysSurface && m_pCurrentPhysSurface->IsVehicle() && ((CVehicle*)m_pCurrentPhysSurface)->IsBoat()) {
		bTryingToReachDryLand = true;
	} else if (!(((uint8)CTimer::GetFrameCounter() + m_randomSeed) & 0xF)) {
		CVehicle *nearVeh = (CVehicle*)CWorld::TestSphereAgainstWorld(GetPosition(), 7.0f, nil,
								false, true, false, false, false, false);
		if (nearVeh && nearVeh->IsBoat())
			bTryingToReachDryLand = true;
		else
			bTryingToReachDryLand = false;
	}
	CPed::ProcessControl();
	if (bWasPostponed)
		return;

	CPad *padUsed = CPad::GetPad(0);
	m_pWanted->Update();
	CEntity::PruneReferences();

	if (m_nMoveState != PEDMOVE_RUN && m_nMoveState != PEDMOVE_SPRINT)
		RestoreSprintEnergy(1.0f);
	else if (m_nMoveState == PEDMOVE_RUN)
		RestoreSprintEnergy(0.3f);

	if (m_nPedState == PED_DEAD) {
		ClearWeaponTarget();
		return;
	}
	if (m_nPedState == PED_DIE) {
		ClearWeaponTarget();
		if (CTimer::GetTimeInMilliseconds() > m_bloodyFootprintCountOrDeathTime + 4000)
			SetDead();
		return;
	}
	if (m_nPedState == PED_DRIVING && m_objective != OBJECTIVE_LEAVE_VEHICLE) {
		if (m_pMyVehicle->IsCar() && ((CAutomobile*)m_pMyVehicle)->Damage.GetDoorStatus(DOOR_FRONT_LEFT) == DOOR_STATUS_SWINGING) {
			CAnimBlendAssociation *rollDoorAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_CAR_ROLLDOOR);
			if (!rollDoorAssoc) {
				rollDoorAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_CAR_ROLLDOOR_LOW);
			}

			// These comparisons are wrong, they return uint16
			if (m_pMyVehicle->m_nGettingOutFlags & CAR_DOOR_FLAG_LF || rollDoorAssoc || padUsed
				&& (padUsed->GetAccelerate() != 0.0f || padUsed->GetSteeringLeftRight() != 0.0f
					|| padUsed->GetBrake() != 0.0f)) {

				if (rollDoorAssoc)
					m_pMyVehicle->ProcessOpenDoor(CAR_DOOR_LF, ANIM_CAR_ROLLDOOR, rollDoorAssoc->currentTime);
			} else {
				m_pMyVehicle->m_nGettingOutFlags |= CAR_DOOR_FLAG_LF;
				if (m_pMyVehicle->bLowVehicle)
					rollDoorAssoc = CAnimManager::AddAnimation(GetClump(), ASSOCGRP_STD, ANIM_CAR_ROLLDOOR_LOW);
				else
					rollDoorAssoc = CAnimManager::AddAnimation(GetClump(), ASSOCGRP_STD, ANIM_CAR_ROLLDOOR);

				rollDoorAssoc->SetFinishCallback(PedAnimDoorCloseRollingCB, this);
			}
		}
		return;
	}
	if (m_objective == OBJECTIVE_NONE)
		m_nMoveState = PEDMOVE_STILL;
	if (bIsLanding)
		RunningLand(padUsed);
	if (padUsed && padUsed->WeaponJustDown() && m_nPedState != PED_SNIPER_MODE) {

		// ...Really?
		eWeaponType playerWeapon = FindPlayerPed()->GetWeapon()->m_eWeaponType;
		if (playerWeapon == WEAPONTYPE_SNIPERRIFLE) {
			DMAudio.PlayFrontEndSound(SOUND_WEAPON_SNIPER_SHOT_NO_ZOOM, 0);
		} else if (playerWeapon == WEAPONTYPE_ROCKETLAUNCHER) {
			DMAudio.PlayFrontEndSound(SOUND_WEAPON_ROCKET_SHOT_NO_ZOOM, 0);
		}
	}

	switch (m_nPedState) {
		case PED_NONE:
		case PED_IDLE:
		case PED_FLEE_POS:
		case PED_FLEE_ENTITY:
		case PED_ATTACK:
		case PED_FIGHT:
		case PED_AIM_GUN:
			if (!RpAnimBlendClumpGetFirstAssociation(GetClump(), ASSOC_BLOCK)) {
				if (TheCamera.Cams[0].Using3rdPersonMouseCam()) {
					if (padUsed)
						PlayerControl1stPersonRunAround(padUsed);
				} else if (m_nPedState == PED_FIGHT) {
					if (padUsed)
						PlayerControlFighter(padUsed);
				} else if (padUsed) {
					PlayerControlZelda(padUsed);
				}
			}
			if (IsPedInControl() && padUsed)
				ProcessPlayerWeapon(padUsed);
			break;
		case PED_LOOK_ENTITY:
		case PED_LOOK_HEADING:
		case PED_WANDER_RANGE:
		case PED_WANDER_PATH:
		case PED_PURSUE:
		case PED_FOLLOW_PATH:
		case PED_ROCKET_ODE:
		case PED_DUMMY:
		case PED_PAUSE:
		case PED_FACE_PHONE:
		case PED_MAKE_CALL:
		case PED_CHAT:
		case PED_MUG:
		case PED_AI_CONTROL:
		case PED_FOLLOW_ROUTE:
		case PED_CPR:
		case PED_SOLICIT:
		case PED_BUY_ICECREAM:
		case PED_INVESTIGATE:
		case PED_STEP_AWAY:
		case PED_ON_FIRE:
		case PED_UNKNOWN:
		case PED_STATES_NO_AI:
		case PED_STAGGER:
		case PED_DIVE_AWAY:
		case PED_STATES_NO_ST:
		case PED_ARREST_PLAYER:
		case PED_DRIVING:
		case PED_PASSENGER:
		case PED_TAXI_PASSENGER:
		case PED_OPEN_DOOR:
		case PED_DIE:
		case PED_DEAD:
		case PED_HANDS_UP:
			break;
		case PED_SEEK_ENTITY:
			m_vecSeekPos = m_pSeekTarget->GetPosition();

			// fall through
		case PED_SEEK_POS:
			switch (m_nMoveState) {
				case PEDMOVE_WALK:
					m_fMoveSpeed = 1.0f;
					break;
				case PEDMOVE_RUN:
					m_fMoveSpeed = 1.8f;
					break;
				case PEDMOVE_SPRINT:
					m_fMoveSpeed = 2.5f;
					break;
				default:
					m_fMoveSpeed = 0.0f;
					break;
			}
			SetRealMoveAnim();
			if (Seek()) {
				RestorePreviousState();
				SetMoveState(PEDMOVE_STILL);
			}
			break;
		case PED_SNIPER_MODE:
			if (FindPlayerPed()->GetWeapon()->m_eWeaponType == WEAPONTYPE_M4) {
				if (padUsed)
					PlayerControlM16(padUsed);
			} else if (padUsed) {
				PlayerControlSniper(padUsed);
			}
			break;
		case PED_SEEK_CAR:
		case PED_SEEK_IN_BOAT:
			if (bVehEnterDoorIsBlocked || bKindaStayInSamePlace) {
				m_fMoveSpeed = 0.0f;
			} else {
				m_fMoveSpeed = Min(2.0f, 2.0f * (m_vecSeekPos - GetPosition()).Magnitude2D());
			}
			if (padUsed && !padUsed->ArePlayerControlsDisabled()) {
				if (padUsed->GetTarget() || padUsed->GetLeftStickXJustDown() || padUsed->GetLeftStickYJustDown() ||
					padUsed->GetDPadUpJustDown() || padUsed->GetDPadDownJustDown() || padUsed->GetDPadLeftJustDown() ||
					padUsed->GetDPadRightJustDown()) {

					RestorePreviousState();
					if (m_objective == OBJECTIVE_ENTER_CAR_AS_PASSENGER || m_objective == OBJECTIVE_ENTER_CAR_AS_DRIVER) {
						RestorePreviousObjective();
					}
				}
			}
			if (padUsed && padUsed->GetSprint())
				m_nMoveState = PEDMOVE_SPRINT;
			SetRealMoveAnim();
			break;
		case PED_JUMP:
			if (padUsed)
				PlayerControlZelda(padUsed);
			if (bIsLanding)
				break;

			// This has been added later it seems
			return;
		case PED_FALL:
		case PED_GETUP:
		case PED_ENTER_TRAIN:
		case PED_EXIT_TRAIN:
		case PED_CARJACK:
		case PED_DRAG_FROM_CAR:
		case PED_ENTER_CAR:
		case PED_STEAL_CAR:
		case PED_EXIT_CAR:
			ClearWeaponTarget();
			break;
		case PED_ARRESTED:
			if (m_nLastPedState == PED_DRAG_FROM_CAR && m_pVehicleAnim)
				BeingDraggedFromCar();
			break;
	}
	if (padUsed && IsPedShootable()) {
		ProcessWeaponSwitch(padUsed);
		GetWeapon()->Update(m_audioEntityId, this);
	}
	ProcessAnimGroups();
	if (padUsed) {
		if (TheCamera.Cams[TheCamera.ActiveCam].Mode == CCam::MODE_FOLLOWPED
			&& TheCamera.Cams[TheCamera.ActiveCam].DirectionWasLooking == LOOKING_BEHIND) {

			m_lookTimer = 0;
			float camAngle = CGeneral::LimitRadianAngle(TheCamera.Cams[TheCamera.ActiveCam].Front.Heading());
			float angleBetweenPlayerAndCam = Abs(camAngle - m_fRotationCur);
			if (m_nPedState != PED_ATTACK
				&& angleBetweenPlayerAndCam > DEGTORAD(30.0f) && angleBetweenPlayerAndCam < DEGTORAD(330.0f)) {

				if (angleBetweenPlayerAndCam > DEGTORAD(150.0f) && angleBetweenPlayerAndCam < DEGTORAD(210.0f)) {
					float rightTurnAngle = CGeneral::LimitRadianAngle(m_fRotationCur - DEGTORAD(150.0f));
					float leftTurnAngle = CGeneral::LimitRadianAngle(DEGTORAD(150.0f) + m_fRotationCur);
					if (m_fLookDirection != 999999.0f) {
						if (Abs(rightTurnAngle - m_fLookDirection) < Abs(leftTurnAngle - m_fLookDirection))
							camAngle = rightTurnAngle;
						else
							camAngle = leftTurnAngle;
					} else {
						camAngle = rightTurnAngle;
					}
				}
				SetLookFlag(camAngle, true);
				SetLookTimer(CTimer::GetTimeStepInMilliseconds() * 5.0f);
			} else {
				ClearLookFlag();
			}
		}
	}
	if (m_nMoveState == PEDMOVE_SPRINT && bIsLooking) {
		ClearLookFlag();
		SetLookTimer(250);
	}

	if (m_vecMoveSpeed.Magnitude2D() < 0.1f) {
		if (m_nSpeedTimer) {
			if (CTimer::GetTimeInMilliseconds() > m_nSpeedTimer)
				m_bSpeedTimerFlag = true;
		} else {
			m_nSpeedTimer = CTimer::GetTimeInMilliseconds() + 500;
		}
	} else {
		m_nSpeedTimer = 0;
		m_bSpeedTimerFlag = false;
	}

#ifdef PED_SKIN
	if (!bIsVisible && IsClumpSkinned(GetClump()))
		UpdateRpHAnim();
#endif
}

bool
CPlayerPed::DoesPlayerWantNewWeapon(eWeaponType weapon, bool onlyIfSlotIsEmpty)
{
	uint32 slot = CWeaponInfo::GetWeaponInfo(weapon)->m_nWeaponSlot;

	if (!HasWeaponSlot(slot) || GetWeapon(slot).m_eWeaponType == weapon)
		return true;

	if (onlyIfSlotIsEmpty)
		return false;

	// Check if he's using that slot right now.
	return m_nPedState != PED_ATTACK && m_nPedState != PED_AIM_GUN || slot != m_currentWeapon;
}

// TODO(Miami): This only works on gamepad cam! This isn't fair
void
CPlayerPed::PlayIdleAnimations(CPad *padUsed)
{
	CAnimBlendAssociation* assoc;

	if (TheCamera.m_WideScreenOn || bIsDucking)
		return;

	struct animAndGroup {
		AnimationId animId;
		AssocGroupId groupId;
	};

	const animAndGroup idleAnims[] = {
		{ANIM_IDLE_STRETCH, ASSOCGRP_PLAYER_IDLE},
		{ANIM_IDLE_TIME, ASSOCGRP_PLAYER_IDLE},
		{ANIM_IDLE_SHOULDER, ASSOCGRP_PLAYER_IDLE},
		{ANIM_IDLE_STRETCH_LEG, ASSOCGRP_PLAYER_IDLE},
		{ANIM_XPRESS_SCRATCH, ASSOCGRP_STD},
	};

	static int32 lastTime = 0;
	static int32 lastAnim = -1;

	bool hasIdleAnim = false;
	CAnimBlock *idleAnimBlock = CAnimManager::GetAnimationBlock(idleAnimBlockIndex);
	uint32 sinceLastInput = padUsed->InputHowLongAgo();
	if (sinceLastInput <= 30000) {
		if (idleAnimBlock->isLoaded) {
			for (assoc = RpAnimBlendClumpGetFirstAssociation(GetClump()); assoc; assoc = RpAnimBlendGetNextAssociation(assoc)) {
				if (assoc->flags & ASSOC_IDLE) {
					hasIdleAnim = true;
					assoc->blendDelta = -8.0f;
				}
			}
			if (!hasIdleAnim)
				CStreaming::RemoveAnim(idleAnimBlockIndex);
		} else {
			lastTime = 0;
		}
	} else {
		CStreaming::RequestAnim(idleAnimBlockIndex, STREAMFLAGS_DONT_REMOVE);
		if (idleAnimBlock->isLoaded) {
			for(CAnimBlendAssociation *assoc = RpAnimBlendClumpGetFirstAssociation(GetClump()); assoc; assoc = RpAnimBlendGetNextAssociation(assoc)) {
				int firstIdle = idleAnimBlock->firstIndex;
				int index = assoc->hierarchy - CAnimManager::GetAnimation(0);
				if (index >= firstIdle && index < firstIdle + idleAnimBlock->numAnims) {
					hasIdleAnim = true;
					break;
				}
			}

			if (!hasIdleAnim && !bIsLooking && !bIsRestoringLook && sinceLastInput - lastTime > 25000) {
				int anim;
				do
					anim = CGeneral::GetRandomNumberInRange(0, ARRAY_SIZE(idleAnims));
				while (lastAnim == anim);

				assoc = CAnimManager::BlendAnimation(GetClump(), idleAnims[anim].groupId, idleAnims[anim].animId, 8.0f);
				assoc->flags |= ASSOC_IDLE;
				lastAnim = anim;
				lastTime = sinceLastInput;
			}
		}
	}
}

#ifdef COMPATIBLE_SAVES
#define CopyFromBuf(buf, data) memcpy(&data, buf, sizeof(data)); SkipSaveBuf(buf, sizeof(data));
#define CopyToBuf(buf, data) memcpy(buf, &data, sizeof(data)); SkipSaveBuf(buf, sizeof(data));
void
CPlayerPed::Save(uint8*& buf)
{
	CPed::Save(buf);
	SkipSaveBuf(buf, 16);
	CopyToBuf(buf, m_fMaxStamina);
	SkipSaveBuf(buf, 28);
	CopyToBuf(buf, m_nTargettableObjects[0]);
	CopyToBuf(buf, m_nTargettableObjects[1]);
	CopyToBuf(buf, m_nTargettableObjects[2]);
	CopyToBuf(buf, m_nTargettableObjects[3]);
	SkipSaveBuf(buf, 116);
}

void
CPlayerPed::Load(uint8*& buf)
{
	CPed::Load(buf);
	SkipSaveBuf(buf, 16);
	CopyFromBuf(buf, m_fMaxStamina);
	SkipSaveBuf(buf, 28);
	CopyFromBuf(buf, m_nTargettableObjects[0]);
	CopyFromBuf(buf, m_nTargettableObjects[1]);
	CopyFromBuf(buf, m_nTargettableObjects[2]);
	CopyFromBuf(buf, m_nTargettableObjects[3]);
	SkipSaveBuf(buf, 116);
}
#undef CopyFromBuf
#undef CopyToBuf
#endif
