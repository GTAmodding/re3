#include "common.h"
#include "patcher.h"
#include "World.h"
#include "PlayerPed.h"
#include "CopPed.h"
#include "ModelIndices.h"
#include "Vehicle.h"
#include "RpAnimBlend.h"
#include "General.h"
#include "ZoneCull.h"
#include "PathFind.h"
#include "RoadBlocks.h"

WRAPPER void CCopPed::ProcessControl() { EAXJMP(0x4C1400); }

CCopPed::CCopPed(eCopType copType) : CPed(PEDTYPE_COP)
{
	m_nCopType = copType;
	switch (copType) {
	case COP_STREET:
		SetModelIndex(MI_COP);
		GiveWeapon(WEAPONTYPE_COLT45, 1000);
		m_currentWeapon = WEAPONTYPE_UNARMED;
		m_fArmour = 0.0f;
		m_wepSkills = 208; /* TODO: what is this? seems unused */
		m_wepAccuracy = 60;
		break;
	case COP_FBI:
		SetModelIndex(MI_FBI);
		GiveWeapon(WEAPONTYPE_COLT45, 1000);
		GiveWeapon(WEAPONTYPE_AK47, 1000);
		SetCurrentWeapon(WEAPONTYPE_AK47);
		m_fArmour = 100.0f;
		m_wepSkills = 176; /* TODO: what is this? seems unused */
		m_wepAccuracy = 76;
		break;
	case COP_SWAT:
		SetModelIndex(MI_SWAT);
		GiveWeapon(WEAPONTYPE_COLT45, 1000);
		GiveWeapon(WEAPONTYPE_UZI, 1000);
		SetCurrentWeapon(WEAPONTYPE_UZI);
		m_fArmour = 50.0f;
		m_wepSkills = 32; /* TODO: what is this? seems unused */
		m_wepAccuracy = 64;
		break;
	case COP_ARMY:
		SetModelIndex(MI_ARMY);
		GiveWeapon(WEAPONTYPE_COLT45, 1000);
		GiveWeapon(WEAPONTYPE_M16, 1000);
		GiveWeapon(WEAPONTYPE_GRENADE, 10);
		SetCurrentWeapon(WEAPONTYPE_M16);
		m_fArmour = 100.0f;
		m_wepSkills = 32; /* TODO: what is this? seems unused */
		m_wepAccuracy = 84;
		break;
	default:
		break;
	}
	m_bIsInPursuit = false;
	field_1350 = 1;
	m_bIsDisabledCop = false;
	field_1356 = 0;
	m_attackTimer = 0;
	m_bBeatingSuspect = false;
	m_bZoneDisabledButClose = false;
	m_bZoneDisabled = false;
	field_1364 = -1;
	m_pPointGunAt = nil;

	// VC also initializes in here, but it keeps object
#ifdef FIX_BUGS
	m_wRoadblockNode = -1;
#endif
}

CCopPed::~CCopPed()
{
	ClearPursuit();
}

// Parameter should always be CPlayerPed, but it seems they considered making civilians arrestable at some point
void
CCopPed::SetArrestPlayer(CPed *player)
{
	if (!IsPedInControl() || !player)
		return;

	switch (m_nCopType) {
		case COP_FBI:
			Say(SOUND_PED_ARREST_FBI);
			break;
		case COP_SWAT:
			Say(SOUND_PED_ARREST_SWAT);
			break;
		default:
			Say(SOUND_PED_ARREST_COP);
			break;
	}
	if (player->EnteringCar()) {
		if (CTimer::GetTimeInMilliseconds() > m_nPedStateTimer)
			return;

		// why?
		player->bGonnaKillTheCarJacker = true;

		// Genius
		FindPlayerPed()->m_bCanBeDamaged = false;
		((CPlayerPed*)player)->m_pArrestingCop = this;
		this->RegisterReference((CEntity**) &((CPlayerPed*)player)->m_pArrestingCop);

	} else if (player->m_nPedState != PED_DIE && player->m_nPedState != PED_DEAD && player->m_nPedState != PED_ARRESTED) {
		player->m_nLastPedState = player->m_nPedState;
		player->m_nPedState = PED_ARRESTED;

		FindPlayerPed()->m_bCanBeDamaged = false;
		((CPlayerPed*)player)->m_pArrestingCop = this;
		this->RegisterReference((CEntity**) &((CPlayerPed*)player)->m_pArrestingCop);
	}

	m_nPedState = PED_ARREST_PLAYER;
	SetObjective(OBJECTIVE_NONE);
	m_prevObjective = OBJECTIVE_NONE;
	bIsPointingGunAt = false;
	m_pSeekTarget = player;
	m_pSeekTarget->RegisterReference((CEntity**) &m_pSeekTarget);
	SetCurrentWeapon(WEAPONTYPE_COLT45);
	if (player->InVehicle()) {
		player->m_pMyVehicle->m_nNumGettingIn = 0;
		player->m_pMyVehicle->m_nGettingInFlags = 0;
		player->m_pMyVehicle->bIsHandbrakeOn = true;
		player->m_pMyVehicle->m_status = STATUS_PLAYER_DISABLED;
	}
	if (GetWeapon()->m_eWeaponType == WEAPONTYPE_UNARMED)
		SetCurrentWeapon(WEAPONTYPE_COLT45);
}

void
CCopPed::ClearPursuit(void)
{
	CPlayerPed *player = FindPlayerPed();
	if (!player)
		return;

	CWanted *wanted = player->m_pWanted;
	int ourCopId = 0;
	bool foundMyself = false;
	int biggestCopId = 0;
	if (!m_bIsInPursuit)
		return;

	m_bIsInPursuit = false;
	for (int i = 0; i < max(wanted->m_MaxCops, wanted->m_CurrentCops); ++i)  {
		if (!foundMyself && wanted->m_pCops[i] == this) {
			wanted->m_pCops[i] = nil;
			--wanted->m_CurrentCops;
			foundMyself = true;
			ourCopId = i;
			biggestCopId = i;
		} else {
			if (wanted->m_pCops[i])
				biggestCopId = i;
		}
	}
	if (foundMyself && biggestCopId > ourCopId) {
		wanted->m_pCops[ourCopId] = wanted->m_pCops[biggestCopId];
		wanted->m_pCops[biggestCopId] = nil;
	}
	m_objective = OBJECTIVE_NONE;
	m_prevObjective = OBJECTIVE_NONE;
	m_nLastPedState = PED_NONE;
	bIsRunning = false;
	bNotAllowedToDuck = false;
	bKindaStayInSamePlace = false;
	m_bZoneDisabledButClose = false;
	m_bZoneDisabled = false;
	ClearObjective();
	if (IsPedInControl()) {
		if (!m_pMyVehicle || wanted->m_nWantedLevel != 0)  {
			if (m_pMyVehicle && (m_pMyVehicle->GetPosition() - GetPosition()).MagnitudeSqr() < sq(5.0f)) {
				m_nLastPedState = PED_IDLE;
				SetSeek((CEntity*)m_pMyVehicle, 2.5f);
			} else {
				m_nLastPedState = PED_WANDER_PATH;
				SetFindPathAndFlee(FindPlayerPed()->GetPosition(), 10000, true);
			}
		} else {
			SetObjective(OBJECTIVE_ENTER_CAR_AS_DRIVER, m_pMyVehicle);
		}
	}
}

// TODO: I don't know why they needed that parameter.
void
CCopPed::SetPursuit(bool ignoreCopLimit)
{
	CWanted *wanted = FindPlayerPed()->m_pWanted;
	if (m_bIsInPursuit || !IsPedInControl())
		return;

	if (wanted->m_CurrentCops < wanted->m_MaxCops || ignoreCopLimit) {
		for (int i = 0; i < wanted->m_MaxCops; ++i) {
			if (!wanted->m_pCops[i]) {
				m_bIsInPursuit = true;
				++wanted->m_CurrentCops;
				wanted->m_pCops[i] = this;
				break;
			}
		}
		if (m_bIsInPursuit) {
			ClearObjective();
			m_prevObjective = OBJECTIVE_NONE;
			SetObjective(OBJECTIVE_KILL_CHAR_ON_FOOT, FindPlayerPed());
			SetObjectiveTimer(0);
			bNotAllowedToDuck = true;
			bIsRunning = true;
			m_bZoneDisabledButClose = false;
		}
	}
}

void
CCopPed::ArrestPlayer(void)
{
	m_pVehicleAnim = nil;
	CPed *suspect = (CPed*)m_pSeekTarget;
	if (suspect) {
		if (suspect->CanSetPedState())
			suspect->m_nPedState = PED_ARRESTED;

		if (suspect->bInVehicle && m_pMyVehicle && suspect->m_pMyVehicle == m_pMyVehicle) {

			// BUG? I will never understand why they used LINE_UP_TO_CAR_2...
			LineUpPedWithCar(LINE_UP_TO_CAR_2);
		}

		if (suspect && (suspect->m_nPedState == PED_ARRESTED || suspect->DyingOrDead() || suspect->EnteringCar())) {

			CAnimBlendAssociation *arrestAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_ARREST_GUN);
			if (!arrestAssoc || arrestAssoc->blendDelta < 0.0f)
				CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_ARREST_GUN, 4.0f);

			CVector suspMidPos;
			suspect->m_pedIK.GetComponentPosition((RwV3d*)suspMidPos, PED_MID);
			m_fRotationDest = CGeneral::GetRadianAngleBetweenPoints(suspMidPos.x, suspMidPos.y,
				GetPosition().x, GetPosition().y);

			m_fRotationCur = m_fRotationDest;
			SetOrientation(0.0f, 0.0f, m_fRotationCur);
		} else {
			ClearPursuit();
		}
	} else {
		ClearPursuit();
	}
}

void
CCopPed::ScanForCrimes(void)
{
	CVehicle *playerVeh = FindPlayerVehicle();

	// Look for car alarms
	if (playerVeh && playerVeh->IsCar()) {
		if (playerVeh->IsAlarmOn()) {
			if ((playerVeh->GetPosition() - GetPosition()).MagnitudeSqr() < sq(20.0f))
				FindPlayerPed()->SetWantedLevelNoDrop(1);
		}
	}

	// Look for stolen cop cars
	if (!m_bIsInPursuit) {
		CPlayerPed *player = FindPlayerPed();
		if ((m_objective == OBJECTIVE_ENTER_CAR_AS_DRIVER || m_objective == OBJECTIVE_ENTER_CAR_AS_PASSENGER)
			&& player->m_pWanted->m_nWantedLevel == 0) {

			if (player->m_pMyVehicle
#ifdef FIX_BUGS
				&& m_pMyVehicle == player->m_pMyVehicle
#endif
				&& player->m_pMyVehicle->bIsLawEnforcer)
				player->SetWantedLevelNoDrop(1);
		}
	}
}

void
CCopPed::CopAI(void)
{
	CWanted *wanted = FindPlayerPed()->m_pWanted;
	int wantedLevel = wanted->m_nWantedLevel;
	CPhysical *playerOrHisVeh = FindPlayerVehicle() ? (CPhysical*)FindPlayerVehicle() : (CPhysical*)FindPlayerPed();

	if (wanted->m_bIgnoredByEveryone || wanted->m_bIgnoredByCops) {
		if (m_nPedState != PED_ARREST_PLAYER)
			ClearPursuit();

		return;
	}
	if (CCullZones::NoPolice() && m_bIsInPursuit && !m_bIsDisabledCop) {
		if (bHitSomethingLastFrame) {
			m_bZoneDisabled = true;
			m_bIsDisabledCop = true;
#ifdef FIX_BUGS
			m_wRoadblockNode = -1;
#else
			m_wRoadblockNode = 0;
#endif
			bKindaStayInSamePlace = true;
			bIsRunning = false;
			bNotAllowedToDuck = false;
			bCrouchWhenShooting = false;
			SetIdle();
			ClearObjective();
			ClearPursuit();
			m_prevObjective = OBJECTIVE_NONE;
			m_nLastPedState = PED_NONE;
			SetAttackTimer(0);
			if (m_fDistanceToTarget > 15.0f)
				m_bZoneDisabledButClose = true;
		}
	} else if (m_bZoneDisabled && !CCullZones::NoPolice()) {
		m_bZoneDisabled = false;
		m_bIsDisabledCop = false;
		m_bZoneDisabledButClose = false;
		bKindaStayInSamePlace = false;
		bCrouchWhenShooting = false;
		bDuckAndCover = false;
		ClearPursuit();
	}
	if (wantedLevel > 0) {
		if (!m_bIsDisabledCop) {
			if (!m_bIsInPursuit || wanted->m_CurrentCops > wanted->m_MaxCops) {
				CCopPed *copFarthestToTarget = nil;
				float copFarthestToTargetDist = m_fDistanceToTarget;

				int oldCopNum = wanted->m_CurrentCops;
				int maxCops = wanted->m_MaxCops;
				
				for (int i = 0; i < max(maxCops, oldCopNum); i++) {
					CCopPed *cop = wanted->m_pCops[i];
					if (cop && cop->m_fDistanceToTarget > copFarthestToTargetDist) {
						copFarthestToTargetDist = cop->m_fDistanceToTarget;
						copFarthestToTarget = wanted->m_pCops[i];
					}
				}

				if (m_bIsInPursuit) {
					if (copFarthestToTarget && oldCopNum > maxCops) {
						if (copFarthestToTarget == this && m_fDistanceToTarget > 10.0f) {
							ClearPursuit();
						} else if(copFarthestToTargetDist > 10.0f)
							copFarthestToTarget->ClearPursuit();
					}
				} else {
					if (oldCopNum < maxCops) {
						SetPursuit(true);
					} else {
						if (m_fDistanceToTarget <= 10.0f || copFarthestToTarget && m_fDistanceToTarget < copFarthestToTargetDist) {
							if (copFarthestToTarget && copFarthestToTargetDist > 10.0f)
								copFarthestToTarget->ClearPursuit();

							SetPursuit(true);
						}
					}
				}
			} else
				SetPursuit(false);

			if (!m_bIsInPursuit)
				return;

			if (wantedLevel > 1 && GetWeapon()->m_eWeaponType == WEAPONTYPE_UNARMED)
				SetCurrentWeapon(WEAPONTYPE_COLT45);
			else if (wantedLevel == 1 && GetWeapon()->m_eWeaponType != WEAPONTYPE_UNARMED && !FindPlayerPed()->m_pCurrentPhysSurface) {
				// i.e. if player is on top of car, cop will still use colt45.
				SetCurrentWeapon(WEAPONTYPE_UNARMED);
			}

			if (FindPlayerVehicle()) {
				if (m_bBeatingSuspect) {
					--wanted->m_CopsBeatingSuspect;
					m_bBeatingSuspect = false;
				}
				if (m_fDistanceToTarget * FindPlayerSpeed().Magnitude() > 4.0f)
					ClearPursuit();
			}
			return;
		}
		float weaponRange = CWeaponInfo::GetWeaponInfo(GetWeapon()->m_eWeaponType)->m_fRange;
		SetLookFlag(playerOrHisVeh, true);
		TurnBody();
		SetCurrentWeapon(WEAPONTYPE_COLT45);
		if (!bIsDucking) {
			if (m_attackTimer >= CTimer::GetTimeInMilliseconds()) {
				if (m_nPedState != PED_ATTACK && m_nPedState != PED_FIGHT && !m_bZoneDisabled) {
					CVector targetDist = playerOrHisVeh->GetPosition() - GetPosition();
					if (m_fDistanceToTarget > 30.0f) {
						CAnimBlendAssociation* crouchShootAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_RBLOCK_CSHOOT);
						if (crouchShootAssoc)
							crouchShootAssoc->blendDelta = -1000.0f;

						// Target is coming onto us
						if (DotProduct(playerOrHisVeh->m_vecMoveSpeed, targetDist) > 0.0f) {
							m_bIsDisabledCop = false;
							bKindaStayInSamePlace = false;
							bNotAllowedToDuck = false;
							bDuckAndCover = false;
							SetPursuit(false);
							SetObjective(OBJECTIVE_KILL_CHAR_ANY_MEANS, FindPlayerPed());
						}
					} else if (m_fDistanceToTarget < 5.0f
							&& (!FindPlayerVehicle() || FindPlayerVehicle()->m_vecMoveSpeed.MagnitudeSqr() < sq(1.f/200.f))) {
						m_bIsDisabledCop = false;
						bKindaStayInSamePlace = false;
						bNotAllowedToDuck = false;
						bDuckAndCover = false;
					} else {
						// VC checks for != nil compared to buggy behaviour of III. I check for != -1 here.
#ifdef VC_PED_PORTS
						float dotProd;
						if (m_wRoadblockNode != -1) {
							CTreadable *roadBlockRoad = ThePaths.m_mapObjects[CRoadBlocks::RoadBlockObjects[m_wRoadblockNode]];
							dotProd = DotProduct2D(playerOrHisVeh->GetPosition() - roadBlockRoad->GetPosition(), GetPosition() - roadBlockRoad->GetPosition());
						} else
							dotProd = -1.0f;

						if(dotProd >= 0.0f) {
#else
						
#ifndef FIX_BUGS
						float copRoadDotProd, targetRoadDotProd;
#else
						float copRoadDotProd = 1.0f, targetRoadDotProd = 1.0f;
						if (m_wRoadblockNode != -1)
#endif
						{
							CTreadable* roadBlockRoad = ThePaths.m_mapObjects[CRoadBlocks::RoadBlockObjects[m_wRoadblockNode]];
							CVector2D roadFwd = roadBlockRoad->GetForward();
							copRoadDotProd = DotProduct2D(GetPosition() - roadBlockRoad->GetPosition(), roadFwd);
							targetRoadDotProd = DotProduct2D(playerOrHisVeh->GetPosition() - roadBlockRoad->GetPosition(), roadFwd);
						}
						// Roadblock may be towards road's fwd or opposite, so check both
						if ((copRoadDotProd >= 0.0f || targetRoadDotProd >= 0.0f)
							&& (copRoadDotProd <= 0.0f || targetRoadDotProd <= 0.0f)) {
#endif
							bIsPointingGunAt = true;
						} else {
							m_bIsDisabledCop = false;
							bKindaStayInSamePlace = false;
							bNotAllowedToDuck = false;
							bCrouchWhenShooting = false;
							bIsDucking = false;
							bDuckAndCover = false;
							SetPursuit(false);
						}
					}
				}
			} else {
				if (m_fDistanceToTarget < weaponRange) {
					CWeaponInfo *weaponInfo = CWeaponInfo::GetWeaponInfo(GetWeapon()->m_eWeaponType);
					CVector gunPos = weaponInfo->m_vecFireOffset;
					for (RwFrame *i = GetNodeFrame(PED_HANDR); i; i = RwFrameGetParent(i))
						RwV3dTransformPoints((RwV3d*)&gunPos, (RwV3d*)&gunPos, 1, RwFrameGetMatrix(i));

					CColPoint foundCol;
					CEntity *foundEnt;
					if (!CWorld::ProcessLineOfSight(gunPos, playerOrHisVeh->GetPosition(), foundCol, foundEnt,
						false, true, false, false, true, false, false)
						|| foundEnt && foundEnt == playerOrHisVeh) {
						m_pPointGunAt = playerOrHisVeh;
						if (playerOrHisVeh)
							playerOrHisVeh->RegisterReference((CEntity**) &m_pPointGunAt);

						SetAttack(playerOrHisVeh);
						SetShootTimer(CGeneral::GetRandomNumberInRange(500, 1000));
					}
					SetAttackTimer(CGeneral::GetRandomNumberInRange(100, 300));
				}
				SetMoveState(PEDMOVE_STILL);
			}
		}
	} else {
		if (!m_bIsDisabledCop || m_bZoneDisabled) {
			if (m_nPedState != PED_AIM_GUN) {
				if (m_bIsInPursuit)
					ClearPursuit();

				if (IsPedInControl()) {
					// Entering the vehicle
					if (m_pMyVehicle && !bInVehicle) {
						if (m_pMyVehicle->IsLawEnforcementVehicle()) {
							if (m_pMyVehicle->pDriver) {
								if (m_pMyVehicle->pDriver->m_nPedType == PEDTYPE_COP) {
									if (m_objective != OBJECTIVE_ENTER_CAR_AS_PASSENGER)
										SetObjective(OBJECTIVE_ENTER_CAR_AS_PASSENGER, m_pMyVehicle);
								} else if (m_pMyVehicle->pDriver->IsPlayer()) {
									FindPlayerPed()->SetWantedLevelNoDrop(1);
								}
							} else if (m_objective != OBJECTIVE_ENTER_CAR_AS_DRIVER) {
								SetObjective(OBJECTIVE_ENTER_CAR_AS_DRIVER, m_pMyVehicle);
							}
						} else {
							m_pMyVehicle = nil;
							ClearObjective();
							SetWanderPath(CGeneral::GetRandomNumber() & 7);
						}
					}
#ifdef VC_PED_PORTS
					else {
						if (m_objective != OBJECTIVE_KILL_CHAR_ON_FOOT && CharCreatedBy == RANDOM_CHAR) {
							for (int i = 0; i < m_numNearPeds; i++) {
								CPed *nearPed = m_nearPeds[i];
								if (nearPed->CharCreatedBy == RANDOM_CHAR) {
									if ((nearPed->m_nPedType == PEDTYPE_CRIMINAL || nearPed->IsGangMember())
										&& nearPed->IsPedInControl()) {

										bool anotherCopChasesHim = false;
										if (nearPed->m_nPedState == PED_FLEE_ENTITY) {
											if (nearPed->m_fleeFrom && nearPed->m_fleeFrom->IsPed() &&
												((CPed*)nearPed->m_fleeFrom)->m_nPedType == PEDTYPE_COP) {
												anotherCopChasesHim = true;
											}
										}
										if (!anotherCopChasesHim) {
											SetObjective(OBJECTIVE_KILL_CHAR_ON_FOOT, nearPed);
											nearPed->SetObjective(OBJECTIVE_FLEE_CHAR_ON_FOOT_TILL_SAFE, this);
											nearPed->m_ped_flagE2 = true;
											return;
										}
									}
								}
							}
						}
					}
#endif
				}
			}
		} else {
			if (m_bIsInPursuit && m_nPedState != PED_AIM_GUN)
				ClearPursuit();

			m_bIsDisabledCop = false;
			bKindaStayInSamePlace = false;
			bNotAllowedToDuck = false;
			bCrouchWhenShooting = false;
			bIsDucking = false;
			bDuckAndCover = false;
			if (m_pMyVehicle)
				SetObjective(OBJECTIVE_ENTER_CAR_AS_DRIVER, m_pMyVehicle);
		}
	}
}

class CCopPed_ : public CCopPed
{
public:
	CCopPed *ctor(eCopType type) { return ::new (this) CCopPed(type); };
	void dtor(void) { CCopPed::~CCopPed(); }
};

STARTPATCHES
	InjectHook(0x4C11B0, &CCopPed_::ctor, PATCH_JUMP);
	InjectHook(0x4C13E0, &CCopPed_::dtor, PATCH_JUMP);
	InjectHook(0x4C28C0, &CCopPed::ClearPursuit, PATCH_JUMP);
	InjectHook(0x4C2B00, &CCopPed::SetArrestPlayer, PATCH_JUMP);
	InjectHook(0x4C27D0, &CCopPed::SetPursuit, PATCH_JUMP);
	InjectHook(0x4C2C90, &CCopPed::ArrestPlayer, PATCH_JUMP);
	InjectHook(0x4C26A0, &CCopPed::ScanForCrimes, PATCH_JUMP);
	InjectHook(0x4C1B50, &CCopPed::CopAI, PATCH_JUMP);
ENDPATCHES
