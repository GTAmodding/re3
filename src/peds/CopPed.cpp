#include "common.h"

#include "World.h"
#include "PlayerPed.h"
#include "CopPed.h"
#include "Wanted.h"
#include "DMAudio.h"
#include "ModelIndices.h"
#include "Vehicle.h"
#include "RpAnimBlend.h"
#include "AnimBlendAssociation.h"
#include "General.h"
#include "ZoneCull.h"
#include "PathFind.h"
#include "RoadBlocks.h"
#include "CarCtrl.h"
#include "Renderer.h"
#include "Camera.h"

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
		m_wepAccuracy = 68;
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
	m_fAbseilPos = 0.0f;
	m_attackTimer = 0;
	m_bBeatingSuspect = false;
	m_bStopAndShootDisabledZone = false;
	m_bZoneDisabled = false;
	field_1364 = -1;
	SetWeaponLockOnTarget(nil);

	// VC also initializes in here, but as nil
#ifdef FIX_BUGS
	m_nRoadblockNode = -1;
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
		player->SetPedState(PED_ARRESTED);

		FindPlayerPed()->m_bCanBeDamaged = false;
		((CPlayerPed*)player)->m_pArrestingCop = this;
		this->RegisterReference((CEntity**) &((CPlayerPed*)player)->m_pArrestingCop);
	}

	SetPedState(PED_ARREST_PLAYER);
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
		player->m_pMyVehicle->SetStatus(STATUS_PLAYER_DISABLED);
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
	for (int i = 0; i < Max(wanted->m_MaxCops, wanted->m_CurrentCops); ++i)  {
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
	m_bStopAndShootDisabledZone = false;
	m_bZoneDisabled = false;
	ClearObjective();
	if (IsPedInControl()) {
		if (!m_pMyVehicle || wanted->GetWantedLevel() != 0)  {
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
			m_bStopAndShootDisabledZone = false;
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
			suspect->SetPedState(PED_ARRESTED);

		if (suspect->bInVehicle && m_pMyVehicle && suspect->m_pMyVehicle == m_pMyVehicle) {

			// BUG? I will never understand why they used LINE_UP_TO_CAR_2...
			LineUpPedWithCar(LINE_UP_TO_CAR_2);
		}

		if (suspect && (suspect->m_nPedState == PED_ARRESTED || suspect->DyingOrDead() || suspect->EnteringCar())) {

			CAnimBlendAssociation *arrestAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_ARREST);
			if (!arrestAssoc || arrestAssoc->blendDelta < 0.0f)
				CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_ARREST, 4.0f);

			CVector suspMidPos;
			suspect->m_pedIK.GetComponentPosition(suspMidPos, PED_MID);
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
			&& player->m_pWanted->GetWantedLevel() == 0) {

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
	int wantedLevel = wanted->GetWantedLevel();
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
			m_nRoadblockNode = -1;
#else
			m_nRoadblockNode = 0;
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

			// Safe distance for disabled zone? Or to just make game easier?
			if (m_fDistanceToTarget > 15.0f)
				m_bStopAndShootDisabledZone = true;
		}
	} else if (m_bZoneDisabled && !CCullZones::NoPolice()) {
		m_bZoneDisabled = false;
		m_bIsDisabledCop = false;
		m_bStopAndShootDisabledZone = false;
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
				
				for (int i = 0; i < Max(maxCops, oldCopNum); i++) {
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
						CAnimBlendAssociation* crouchShootAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_RBLOCK_SHOOT);
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
						if (m_nRoadblockNode != -1) {
							CTreadable *roadBlockRoad = ThePaths.m_mapObjects[CRoadBlocks::RoadBlockObjects[m_nRoadblockNode]];
							dotProd = DotProduct2D(playerOrHisVeh->GetPosition() - roadBlockRoad->GetPosition(), GetPosition() - roadBlockRoad->GetPosition());
						} else
							dotProd = -1.0f;

						if(dotProd >= 0.0f) {
#else
						
#ifndef FIX_BUGS
						float copRoadDotProd, targetRoadDotProd;
#else
						float copRoadDotProd = 1.0f, targetRoadDotProd = 1.0f;
						if (m_nRoadblockNode != -1)
#endif
						{
							CTreadable* roadBlockRoad = ThePaths.m_mapObjects[CRoadBlocks::RoadBlockObjects[m_nRoadblockNode]];
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
					TransformToNode(gunPos, PED_HANDR);

					CColPoint foundCol;
					CEntity *foundEnt;
					if (!CWorld::ProcessLineOfSight(gunPos, playerOrHisVeh->GetPosition(), foundCol, foundEnt,
						false, true, false, false, true, false, false)
						|| foundEnt && foundEnt == playerOrHisVeh) {
						SetWeaponLockOnTarget(playerOrHisVeh);
						SetAttack(playerOrHisVeh);
						SetShootTimer(CGeneral::GetRandomNumberInRange(500, 1000));
					}
					SetAttackTimer(CGeneral::GetRandomNumberInRange(200, 300));
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
											nearPed->bBeingChasedByPolice = true;
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

void
CCopPed::ProcessControl(void)
{
	if (m_nZoneLevel > LEVEL_GENERIC && m_nZoneLevel != CCollision::ms_collisionInMemory)
		return;

	CPed::ProcessControl();
	if (bWasPostponed)
		return;

	if (m_nPedState == PED_DEAD) {
		ClearPursuit();
		m_objective = OBJECTIVE_NONE;
		return;
	}
	if (m_nPedState == PED_DIE)
		return;

	if (m_nPedState == PED_ARREST_PLAYER) {
		ArrestPlayer();
		return;
	}
	GetWeapon()->Update(m_audioEntityId);
	if (m_moved.Magnitude() > 0.0f)
		Avoid();

	CPhysical *playerOrHisVeh = FindPlayerVehicle() ? (CPhysical*)FindPlayerVehicle() : (CPhysical*)FindPlayerPed();
	CPlayerPed *player = FindPlayerPed();

	m_fDistanceToTarget = (playerOrHisVeh->GetPosition() - GetPosition()).Magnitude();
	if (player->m_nPedState == PED_ARRESTED || player->DyingOrDead()) {
		if (m_fDistanceToTarget < 5.0f) {
			SetArrestPlayer(player);
			return;
		}
		if (IsPedInControl())
			SetIdle();
	}
	if (m_bIsInPursuit) {
		if (player->m_nPedState != PED_ARRESTED && !player->DyingOrDead()) {
			switch (m_nCopType) {
				case COP_FBI:
					Say(SOUND_PED_PURSUIT_FBI);
					break;
				case COP_SWAT:
					Say(SOUND_PED_PURSUIT_SWAT);
					break;
				case COP_ARMY:
					Say(SOUND_PED_PURSUIT_ARMY);
					break;
				default:
					Say(SOUND_PED_PURSUIT_COP);
					break;
			}
		}
	}

	if (IsPedInControl()) {
		CopAI();
		/* switch (m_nCopType)
		{
			case COP_FBI:
				CopAI();
				break;
			case COP_SWAT:
				CopAI();
				break;
			case COP_ARMY:
				CopAI();
				break;
			default:
				CopAI();
				break;
		} */
	} else if (InVehicle()) {
		if (m_pMyVehicle->pDriver == this && m_pMyVehicle->AutoPilot.m_nCarMission == MISSION_NONE &&
			CanPedDriveOff() && m_pMyVehicle->VehicleCreatedBy != MISSION_VEHICLE) {

			CCarCtrl::JoinCarWithRoadSystem(m_pMyVehicle);
			m_pMyVehicle->AutoPilot.m_nCarMission = MISSION_CRUISE;
			m_pMyVehicle->AutoPilot.m_nDrivingStyle = DRIVINGSTYLE_STOP_FOR_CARS;
			m_pMyVehicle->AutoPilot.m_nCruiseSpeed = 17;
		}
	}
	if (IsPedInControl() || m_nPedState == PED_DRIVING)
		ScanForCrimes();

	// They may have used goto to jump here in case of PED_ATTACK.
	if (m_nPedState == PED_IDLE || m_nPedState == PED_ATTACK) {
		if (m_objective == OBJECTIVE_KILL_CHAR_ON_FOOT &&
			player && player->EnteringCar() && m_fDistanceToTarget < 1.3f) {
			SetArrestPlayer(player);
		}
	} else {
		if (m_nPedState == PED_SEEK_POS) {
			if (player->m_nPedState == PED_ARRESTED) {
				SetIdle();
				SetLookFlag(player, false);
				SetLookTimer(1000);
				RestorePreviousObjective();
			} else {
				if (player->m_pMyVehicle && player->m_pMyVehicle->m_nNumGettingIn != 0) {
					// This is 1.3f when arresting in car without seeking first (in above)
#if defined(VC_PED_PORTS) || defined(FIX_BUGS)
					m_distanceToCountSeekDone = 1.3f;
#else
					m_distanceToCountSeekDone = 2.0f;
#endif
				}

				if (bDuckAndCover) {
#if GTA_VERSION < GTA3_PC_11 && !defined(VC_PED_PORTS)
					if (!bNotAllowedToDuck && Seek()) {
						SetMoveState(PEDMOVE_STILL);
						SetMoveAnim();
						SetPointGunAt(m_pedInObjective);
					}
#endif
				} else if (Seek()) {
					CVehicle *playerVeh = FindPlayerVehicle();
					if (!playerVeh && player && player->EnteringCar()) {
						SetArrestPlayer(player);
					} else if (1.5f + GetPosition().z <= m_vecSeekPos.z || GetPosition().z - 0.3f >= m_vecSeekPos.z) {
						SetMoveState(PEDMOVE_STILL);
					} else if (playerVeh && playerVeh->CanPedEnterCar() && playerVeh->m_nNumGettingIn == 0) {
						SetCarJack(playerVeh);
					}
				}
			}
		} else if (m_nPedState == PED_SEEK_ENTITY) {
			if (!m_pSeekTarget) {
				RestorePreviousState();
			} else {
				m_vecSeekPos = m_pSeekTarget->GetPosition();
				if (Seek()) {
					if (m_objective == OBJECTIVE_KILL_CHAR_ON_FOOT && m_fDistanceToTarget < 2.5f && player) {
						if (player->m_nPedState == PED_ARRESTED || player->m_nPedState == PED_ENTER_CAR ||
							(player->m_nPedState == PED_CARJACK && m_fDistanceToTarget < 1.3f)) {
							SetArrestPlayer(player);
						} else
							RestorePreviousState();
					} else {
						RestorePreviousState();
					}

				}
			}
		}
	}
	if (!m_bStopAndShootDisabledZone)
		return;

	bool dontShoot = false;
	if (GetIsOnScreen() && CRenderer::IsEntityCullZoneVisible(this)) {
		if (((CTimer::GetFrameCounter() + m_randomSeed) & 0x1F) == 17) {
			CEntity *foundBuilding = nil;
			CColPoint foundCol;
			CVector lookPos = GetPosition() + CVector(0.0f, 0.0f, 0.7f);
			CVector camPos = TheCamera.GetGameCamPosition();
			CWorld::ProcessLineOfSight(camPos, lookPos, foundCol, foundBuilding,
				true, false, false, false, false, false, false);

			// He's at least 15.0 far, in disabled zone, collided into somewhere (that's why m_bStopAndShootDisabledZone set),
			// and now has building on front of him. He's stupid, we don't need him.
			if (foundBuilding) {
				FlagToDestroyWhenNextProcessed();
				dontShoot = true;
			}
		}
	} else {
		FlagToDestroyWhenNextProcessed();
		dontShoot = true;
	}

	if (!dontShoot) {
		bStopAndShoot = true;
		bKindaStayInSamePlace = true;
		bIsPointingGunAt = true;
		SetAttack(m_pedInObjective);
	}
}
