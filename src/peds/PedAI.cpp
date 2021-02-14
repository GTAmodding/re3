#include "common.h"

#include "main.h"
#include "Particle.h"
#include "RpAnimBlend.h"
#include "Ped.h"
#include "Wanted.h"
#include "AnimBlendAssociation.h"
#include "DMAudio.h"
#include "General.h"
#include "HandlingMgr.h"
#include "Replay.h"
#include "Camera.h"
#include "PedPlacement.h"
#include "ZoneCull.h"
#include "Pad.h"
#include "Pickups.h"
#include "Train.h"
#include "PedRoutes.h"
#include "CopPed.h"
#include "Script.h"
#include "CarCtrl.h"
#include "WaterLevel.h"
#include "CarAI.h"
#include "Zones.h"
#include "Cranes.h"

CVector vecPedCarDoorAnimOffset;
CVector vecPedCarDoorLoAnimOffset;
CVector vecPedVanRearDoorAnimOffset;
CVector vecPedQuickDraggedOutCarAnimOffset;
CVector vecPedDraggedOutCarAnimOffset;
CVector vecPedTrainDoorAnimOffset;

void
CPed::SetObjectiveTimer(int time)
{
	if (time == 0) {
		m_objectiveTimer = 0;
	} else if (CTimer::GetTimeInMilliseconds() > m_objectiveTimer) {
		m_objectiveTimer = CTimer::GetTimeInMilliseconds() + time;
	}
}

void
CPed::SetStoredObjective(void)
{
	if (m_objective == m_prevObjective)
		return;

	switch (m_objective)
	{
		case OBJECTIVE_FLEE_ON_FOOT_TILL_SAFE:
		case OBJECTIVE_KILL_CHAR_ON_FOOT:
		case OBJECTIVE_KILL_CHAR_ANY_MEANS:
		case OBJECTIVE_FLEE_CHAR_ON_FOOT_TILL_SAFE:
		case OBJECTIVE_FLEE_CHAR_ON_FOOT_ALWAYS:
		case OBJECTIVE_GOTO_CHAR_ON_FOOT:
		case OBJECTIVE_FOLLOW_CHAR_IN_FORMATION:
		case OBJECTIVE_LEAVE_CAR:
		case OBJECTIVE_ENTER_CAR_AS_PASSENGER:
		case OBJECTIVE_ENTER_CAR_AS_DRIVER:
		case OBJECTIVE_GOTO_AREA_ON_FOOT:
		case OBJECTIVE_RUN_TO_AREA:
			return;
		default:
			m_prevObjective = m_objective;
	}
}

void
CPed::ForceStoredObjective(eObjective objective)
{
	if (objective != OBJECTIVE_ENTER_CAR_AS_DRIVER && objective != OBJECTIVE_ENTER_CAR_AS_PASSENGER) {
		m_prevObjective = m_objective;
		return;
	}

	switch (m_objective)
	{
		case OBJECTIVE_FLEE_ON_FOOT_TILL_SAFE:
		case OBJECTIVE_KILL_CHAR_ON_FOOT:
		case OBJECTIVE_FLEE_CHAR_ON_FOOT_TILL_SAFE:
		case OBJECTIVE_FLEE_CHAR_ON_FOOT_ALWAYS:
		case OBJECTIVE_GOTO_CHAR_ON_FOOT:
		case OBJECTIVE_ENTER_CAR_AS_PASSENGER:
		case OBJECTIVE_ENTER_CAR_AS_DRIVER:
		case OBJECTIVE_GOTO_AREA_ON_FOOT:
		case OBJECTIVE_RUN_TO_AREA:
			return;
		default:
			m_prevObjective = m_objective;
	}
}

bool
CPed::IsTemporaryObjective(eObjective objective)
{
	return objective == OBJECTIVE_LEAVE_CAR || objective == OBJECTIVE_SET_LEADER ||
#ifdef VC_PED_PORTS
		objective == OBJECTIVE_LEAVE_CAR_AND_DIE ||
#endif
		objective == OBJECTIVE_ENTER_CAR_AS_DRIVER || objective == OBJECTIVE_ENTER_CAR_AS_PASSENGER;
}

void
CPed::SetObjective(eObjective newObj)
{
	if (DyingOrDead())
		return;

	if (newObj == OBJECTIVE_NONE) {
		if ((m_objective == OBJECTIVE_LEAVE_CAR || m_objective == OBJECTIVE_ENTER_CAR_AS_DRIVER || m_objective == OBJECTIVE_ENTER_CAR_AS_PASSENGER
#ifdef VC_PED_PORTS
			|| m_objective == OBJECTIVE_LEAVE_CAR_AND_DIE)
			&& !IsPlayer()
#else
			)
#endif
			&& !IsPedInControl()) {

			bStartWanderPathOnFoot = true;
			return;
		}
		// Unused code from assembly...
		/*
		else if(m_objective == OBJECTIVE_FLEE_CAR) {

		} else {

		}
		*/
		m_objective = OBJECTIVE_NONE;
		m_prevObjective = OBJECTIVE_NONE;
	} else if (m_prevObjective != newObj || m_prevObjective == OBJECTIVE_NONE) {
		SetObjectiveTimer(0);

		if (m_objective == newObj)
			return;

		if (IsTemporaryObjective(m_objective)) {
			m_prevObjective = newObj;
		} else {
			if (m_objective != newObj)
				SetStoredObjective();

			m_objective = newObj;
		}
		bObjectiveCompleted = false;

		switch (newObj) {
			case OBJECTIVE_NONE:
				m_prevObjective = OBJECTIVE_NONE;
				break;
			case OBJECTIVE_HAIL_TAXI:
				m_nWaitTimer = 0;
				SetIdle();
				SetMoveState(PEDMOVE_STILL);
				break;
			default:
				break;
		}
	}
}

void
CPed::SetObjective(eObjective newObj, void *entity)
{
	if (DyingOrDead())
		return;

	if (m_prevObjective == newObj) {
		// Why?
		if (m_prevObjective != OBJECTIVE_NONE)
			return;
	}

	if (entity == this)
		return;

	SetObjectiveTimer(0);
	if (m_objective == newObj) {
		switch (newObj) {
			case OBJECTIVE_KILL_CHAR_ON_FOOT:
			case OBJECTIVE_KILL_CHAR_ANY_MEANS:
			case OBJECTIVE_GOTO_CHAR_ON_FOOT:
			case OBJECTIVE_FOLLOW_CHAR_IN_FORMATION:
			case OBJECTIVE_GOTO_AREA_ANY_MEANS:
			case OBJECTIVE_GUARD_ATTACK:
				if (m_pedInObjective == entity)
					return;

				break;
			case OBJECTIVE_LEAVE_CAR:
			case OBJECTIVE_FLEE_CAR:
#ifdef VC_PED_PORTS
			case OBJECTIVE_LEAVE_CAR_AND_DIE:
#endif
				return;
			case OBJECTIVE_ENTER_CAR_AS_PASSENGER:
			case OBJECTIVE_ENTER_CAR_AS_DRIVER:
			case OBJECTIVE_DESTROY_CAR:
			case OBJECTIVE_SOLICIT_VEHICLE:
			case OBJECTIVE_BUY_ICE_CREAM:
				if (m_carInObjective == entity)
					return;

				break;
			case OBJECTIVE_SET_LEADER:
				if (m_leader == entity)
					return;

				break;
			default:
				break;
		}
	} else {
		if ((newObj == OBJECTIVE_LEAVE_CAR
#ifdef VC_PED_PORTS
			|| newObj == OBJECTIVE_LEAVE_CAR_AND_DIE
#endif
			) && !bInVehicle)
			return;
	}

#ifdef VC_PED_PORTS
	ClearPointGunAt();
#endif
	bObjectiveCompleted = false;
	if (IsTemporaryObjective(m_objective) && !IsTemporaryObjective(newObj)) {
		m_prevObjective = newObj;
	} else {
		if (m_objective != newObj) {
			if (IsTemporaryObjective(newObj))
				ForceStoredObjective(newObj);
			else
				SetStoredObjective();
		}
		m_objective = newObj;
	}

	switch (newObj) {
		case OBJECTIVE_WAIT_IN_CAR_THEN_GET_OUT:

			// In this special case, entity parameter isn't CEntity, but int.
			SetObjectiveTimer((uintptr)entity);
			break;
		case OBJECTIVE_KILL_CHAR_ON_FOOT:
		case OBJECTIVE_KILL_CHAR_ANY_MEANS:
		case OBJECTIVE_MUG_CHAR:
			m_pNextPathNode = nil;
			bUsePedNodeSeek = false;
			m_vecSeekPos = CVector(0.0f, 0.0f, 0.0f);
			m_pedInObjective = (CPed*)entity;
			m_pedInObjective->RegisterReference((CEntity**)&m_pedInObjective);
			m_pLookTarget = (CEntity*)entity;
			m_pLookTarget->RegisterReference((CEntity**)&m_pLookTarget);
			break;
		case OBJECTIVE_FLEE_CHAR_ON_FOOT_TILL_SAFE:
		case OBJECTIVE_FLEE_CHAR_ON_FOOT_ALWAYS:
		case OBJECTIVE_GOTO_CHAR_ON_FOOT:
		case OBJECTIVE_GUARD_ATTACK:
			m_vecSeekPos = CVector(0.0f, 0.0f, 0.0f);
			m_pedInObjective = (CPed*)entity;
			m_pedInObjective->RegisterReference((CEntity**)&m_pedInObjective);
			break;
		case OBJECTIVE_FOLLOW_CHAR_IN_FORMATION:
			m_pedInObjective = (CPed*)entity;
			m_pedInObjective->RegisterReference((CEntity**)&m_pedInObjective);
			m_pedFormation = FORMATION_REAR;
			break;
		case OBJECTIVE_LEAVE_CAR:
#ifdef VC_PED_PORTS
		case OBJECTIVE_LEAVE_CAR_AND_DIE:
#endif
		case OBJECTIVE_FLEE_CAR:
			m_carInObjective = (CVehicle*)entity;
			m_carInObjective->RegisterReference((CEntity **)&m_carInObjective);
			if (m_carInObjective->bIsBus && m_leaveCarTimer == 0) {
				for (int i = 0; i < m_carInObjective->m_nNumMaxPassengers; i++) {
					if (m_carInObjective->pPassengers[i] == this) {
						m_leaveCarTimer = CTimer::GetTimeInMilliseconds() + 1200 * i;
						break;
					}
				}
			}

			break;
		case OBJECTIVE_ENTER_CAR_AS_PASSENGER:
		case OBJECTIVE_ENTER_CAR_AS_DRIVER:
			if (m_nMoveState == PEDMOVE_STILL)
				SetMoveState(PEDMOVE_RUN);

			if (((CVehicle*)entity)->m_vehType == VEHICLE_TYPE_BOAT && !IsPlayer()) {
				RestorePreviousObjective();
				break;
			}
			// fall through
		case OBJECTIVE_DESTROY_CAR:
		case OBJECTIVE_SOLICIT_VEHICLE:
		case OBJECTIVE_BUY_ICE_CREAM:
			m_carInObjective = (CVehicle*)entity;
			m_carInObjective->RegisterReference((CEntity**)&m_carInObjective);
			m_pSeekTarget = m_carInObjective;
			m_pSeekTarget->RegisterReference((CEntity**)&m_pSeekTarget);
			m_vecSeekPos = CVector(0.0f, 0.0f, 0.0f);
			if (newObj == OBJECTIVE_SOLICIT_VEHICLE) {
				m_objectiveTimer = CTimer::GetTimeInMilliseconds() + 10000;
			} else if (m_objective == OBJECTIVE_ENTER_CAR_AS_PASSENGER && CharCreatedBy == MISSION_CHAR &&
					(m_carInObjective->GetStatus() == STATUS_PLAYER_DISABLED || CPad::GetPad(CWorld::PlayerInFocus)->ArePlayerControlsDisabled())) {
				SetObjectiveTimer(14000);
			} else {
				m_objectiveTimer = 0;
			}
			break;
		case OBJECTIVE_SET_LEADER:
			SetLeader((CEntity*)entity);
			RestorePreviousObjective();
			break;
		default:
			break;
	}
}

void
CPed::SetObjective(eObjective newObj, CVector dest, float safeDist)
{
	if (DyingOrDead())
		return;

	if (m_prevObjective != OBJECTIVE_NONE && m_prevObjective == newObj)
		return;

	SetObjectiveTimer(0);
	if (m_objective == newObj) {
		if (newObj == OBJECTIVE_GOTO_AREA_ANY_MEANS || newObj == OBJECTIVE_GOTO_AREA_ON_FOOT || newObj == OBJECTIVE_RUN_TO_AREA) {
			if (m_nextRoutePointPos == dest && m_distanceToCountSeekDone == safeDist)
				return;
		} else if (newObj == OBJECTIVE_GUARD_SPOT) {
			if (m_vecSeekPosEx == dest && m_distanceToCountSeekDoneEx == safeDist)
				return;
		}
	}

#ifdef VC_PED_PORTS
	ClearPointGunAt();
#endif
	bObjectiveCompleted = false;
	if (IsTemporaryObjective(m_objective)) {
		m_prevObjective = newObj;
	} else {
		if (m_objective != newObj)
			SetStoredObjective();

		m_objective = newObj;
	}
	
	if (newObj == OBJECTIVE_GUARD_SPOT) {
		m_vecSeekPosEx = dest;
		m_distanceToCountSeekDoneEx = safeDist;
	} else if (newObj == OBJECTIVE_GOTO_AREA_ANY_MEANS || newObj == OBJECTIVE_GOTO_AREA_ON_FOOT || newObj == OBJECTIVE_RUN_TO_AREA) {
		m_pNextPathNode = nil;
		m_nextRoutePointPos = dest;
		m_vecSeekPos = m_nextRoutePointPos;
		bUsePedNodeSeek = true;
	}
}

// Only used in 01E1: SET_CHAR_OBJ_FOLLOW_ROUTE opcode
// IDA fails very badly in here, puts a fake loop and ignores SetFollowRoute call...
void
CPed::SetObjective(eObjective newObj, int16 routePoint, int16 routeType)
{
	if (DyingOrDead())
		return;

	if (m_prevObjective == newObj && m_prevObjective != OBJECTIVE_NONE)
		return;

	SetObjectiveTimer(0);

	if (m_objective == newObj && newObj == OBJECTIVE_FOLLOW_ROUTE && m_routeLastPoint == routePoint && m_routeType == routeType)
		return;

	bObjectiveCompleted = false;
	if (IsTemporaryObjective(m_objective)) {
		m_prevObjective = newObj;
	} else {
		if (m_objective != newObj)
			SetStoredObjective();

		m_objective = newObj;
	}

	if (newObj == OBJECTIVE_FOLLOW_ROUTE) {
		SetFollowRoute(routePoint, routeType);
	}
}

void
CPed::SetObjective(eObjective newObj, CVector dest)
{
	if (DyingOrDead())
		return;

	if (m_prevObjective != OBJECTIVE_NONE && m_prevObjective == newObj)
		return;

	SetObjectiveTimer(0);
	if (m_objective == newObj) {
		if (newObj == OBJECTIVE_GOTO_AREA_ANY_MEANS || newObj == OBJECTIVE_GOTO_AREA_ON_FOOT || newObj == OBJECTIVE_RUN_TO_AREA) {
			if (m_nextRoutePointPos == dest)
				return;
		} else if (newObj == OBJECTIVE_GUARD_SPOT) {
			if (m_vecSeekPosEx == dest)
				return;
		}
	}

#ifdef VC_PED_PORTS
	ClearPointGunAt();
#endif
	bObjectiveCompleted = false;
	switch (newObj) {
		case OBJECTIVE_GUARD_SPOT:
			m_vecSeekPosEx = dest;
			m_distanceToCountSeekDoneEx = 5.0f;
			SetMoveState(PEDMOVE_STILL);
			break;
		case OBJECTIVE_GUARD_AREA:
		case OBJECTIVE_WAIT_IN_CAR:
		case OBJECTIVE_WAIT_IN_CAR_THEN_GET_OUT:
		case OBJECTIVE_KILL_CHAR_ON_FOOT:
		case OBJECTIVE_KILL_CHAR_ANY_MEANS:
		case OBJECTIVE_FLEE_CHAR_ON_FOOT_TILL_SAFE:
		case OBJECTIVE_FLEE_CHAR_ON_FOOT_ALWAYS:
		case OBJECTIVE_GOTO_CHAR_ON_FOOT:
		case OBJECTIVE_FOLLOW_CHAR_IN_FORMATION:
		case OBJECTIVE_LEAVE_CAR:
		case OBJECTIVE_ENTER_CAR_AS_PASSENGER:
		case OBJECTIVE_ENTER_CAR_AS_DRIVER:
		case OBJECTIVE_FOLLOW_CAR_IN_CAR:
		case OBJECTIVE_FIRE_AT_OBJECT_FROM_VEHICLE:
		case OBJECTIVE_DESTROY_OBJECT:
		case OBJECTIVE_DESTROY_CAR:
			break;
		case OBJECTIVE_GOTO_AREA_ANY_MEANS:
		case OBJECTIVE_GOTO_AREA_ON_FOOT:
			bIsRunning = false;
			m_pNextPathNode = nil;
			m_nextRoutePointPos = dest;
			m_vecSeekPos = m_nextRoutePointPos;
			m_distanceToCountSeekDone = 0.5f;
			bUsePedNodeSeek = true;
			if (sq(m_distanceToCountSeekDone) > (m_nextRoutePointPos - GetPosition()).MagnitudeSqr2D())
				return;
			break;
		case OBJECTIVE_RUN_TO_AREA:
			bIsRunning = true;
			m_pNextPathNode = nil;
			m_nextRoutePointPos = dest;
			m_vecSeekPos = m_nextRoutePointPos;
			m_distanceToCountSeekDone = 0.5f;
			bUsePedNodeSeek = true;
			if (sq(m_distanceToCountSeekDone) > (m_nextRoutePointPos - GetPosition()).MagnitudeSqr2D())
				return;
			break;
		default: break;
	}

	if (IsTemporaryObjective(m_objective)) {
		m_prevObjective = newObj;
	} else {
		if (m_objective != newObj)
			SetStoredObjective();

		m_objective = newObj;
	}
}

void
CPed::ClearObjective(void)
{
	if (IsPedInControl() || m_nPedState == PED_DRIVING) {
		m_objective = OBJECTIVE_NONE;
#ifdef VC_PED_PORTS
		m_pedInObjective = nil;
		m_carInObjective = nil;
#endif
		if (m_nPedState == PED_DRIVING && m_pMyVehicle) {

			if (m_pMyVehicle->pDriver != this) {
#if defined VC_PED_PORTS || defined FIX_BUGS
				if(!IsPlayer())
#endif
					bWanderPathAfterExitingCar = true;

				SetObjective(OBJECTIVE_LEAVE_CAR, m_pMyVehicle);
			}
#ifdef VC_PED_PORTS
			m_nLastPedState = PED_NONE;
#endif
		} else {
			SetIdle();
			SetMoveState(PEDMOVE_STILL);
		}
	} else {
		bClearObjective = true;
	}
}

void
CPed::ClearLeader(void)
{
	if (!m_leader)
		return;

	m_leader = nil;
	if (IsPedInControl()) {
		SetObjective(OBJECTIVE_NONE);
		if (CharCreatedBy == MISSION_CHAR) {
			SetIdle();
		} else {
			SetWanderPath(CGeneral::GetRandomNumberInRange(0,8));
		}
	} else if (m_objective != OBJECTIVE_NONE) {
		bClearObjective = true;
	}
}

void
CPed::UpdateFromLeader(void)
{
	if (CTimer::GetTimeInMilliseconds() <= m_objectiveTimer)
		return;

	if (!m_leader)
		return;

	CVector leaderDist;
	if (m_leader->InVehicle())
		leaderDist = m_leader->m_pMyVehicle->GetPosition() - GetPosition();
	else
		leaderDist = m_leader->GetPosition() - GetPosition();

	if (leaderDist.Magnitude() > 30.0f) {
		if (IsPedInControl()) {
			SetObjective(OBJECTIVE_NONE);
			SetIdle();
			SetMoveState(PEDMOVE_STILL);
		}
		SetLeader(nil);
		return;
	}

	if (IsPedInControl()) {
		if (m_nWaitState == WAITSTATE_PLAYANIM_TAXI)
			WarpPedToNearLeaderOffScreen();

		if (m_leader->m_nPedState == PED_DEAD) {
			SetLeader(nil);
			SetObjective(OBJECTIVE_FLEE_ON_FOOT_TILL_SAFE);
			return;
		}
		if (!m_leader->bInVehicle) {
			if (m_leader->m_objective != OBJECTIVE_ENTER_CAR_AS_DRIVER) {
				if (bInVehicle) {
					if (m_objective != OBJECTIVE_WAIT_IN_CAR_THEN_GET_OUT && m_objective != OBJECTIVE_LEAVE_CAR)
						SetObjective(OBJECTIVE_LEAVE_CAR, m_pMyVehicle);

					return;
				}
				if (m_objective == OBJECTIVE_ENTER_CAR_AS_PASSENGER) {
					RestorePreviousObjective();
					RestorePreviousState();
				}
			}
			if (m_nPedType == PEDTYPE_PROSTITUTE && CharCreatedBy == RANDOM_CHAR) {
				SetLeader(nil);
				return;
			}
		}
		if (!bInVehicle && m_leader->bInVehicle && m_leader->m_nPedState == PED_DRIVING) {
			if (m_objective != OBJECTIVE_ENTER_CAR_AS_PASSENGER && m_objective != OBJECTIVE_ENTER_CAR_AS_DRIVER) {
				if (m_leader->m_pMyVehicle->m_nNumPassengers < m_leader->m_pMyVehicle->m_nNumMaxPassengers)
					SetObjective(OBJECTIVE_ENTER_CAR_AS_PASSENGER, m_leader->m_pMyVehicle);
			}
		} else if (m_leader->m_objective == OBJECTIVE_NONE || (m_leader->IsPlayer() && m_leader->m_objective == OBJECTIVE_WAIT_ON_FOOT)
			|| m_objective == m_leader->m_objective) {

			if (m_leader->m_nPedState == PED_ATTACK) {
				CEntity *lookTargetOfLeader = m_leader->m_pLookTarget;
				if (lookTargetOfLeader && m_objective != OBJECTIVE_KILL_CHAR_ON_FOOT
					&& lookTargetOfLeader->IsPed() && lookTargetOfLeader != this) {

					SetObjective(OBJECTIVE_KILL_CHAR_ON_FOOT, lookTargetOfLeader);
					SetObjectiveTimer(8000);
					SetLookFlag(m_leader->m_pLookTarget, false);
					SetLookTimer(500);
				}
			} else {
				if (IsPedInControl() && m_nPedState != PED_ATTACK) {
#ifndef VC_PED_PORTS
					SetObjective(OBJECTIVE_GOTO_CHAR_ON_FOOT, m_leader);
					SetObjectiveTimer(0);
#else
					if (m_leader->m_objective == OBJECTIVE_NONE && m_objective == OBJECTIVE_NONE
						&& m_leader->m_nPedState == PED_CHAT && m_nPedState == PED_CHAT) {

						SetObjective(OBJECTIVE_NONE);
					} else {
						SetObjective(OBJECTIVE_GOTO_CHAR_ON_FOOT, m_leader);
						SetObjectiveTimer(0);
					}
#endif
				}
				if (m_nPedState == PED_IDLE && m_leader->IsPlayer()) {
					if (ScanForThreats() && m_threatEntity) {
						m_pLookTarget = m_threatEntity;
						m_pLookTarget->RegisterReference((CEntity **) &m_pLookTarget);
						TurnBody();
						if (m_attackTimer < CTimer::GetTimeInMilliseconds() && !GetWeapon()->IsTypeMelee()) {
							SetWeaponLockOnTarget(m_threatEntity);
							SetAttack(m_threatEntity);
						}
					}
				}
			}
		} else {
			switch (m_leader->m_objective) {
				case OBJECTIVE_WAIT_ON_FOOT:
				case OBJECTIVE_FLEE_ON_FOOT_TILL_SAFE:
				case OBJECTIVE_WAIT_IN_CAR:
				case OBJECTIVE_FOLLOW_ROUTE:
					SetObjective(m_leader->m_objective);
					m_objectiveTimer = m_leader->m_objectiveTimer;
					break;
				case OBJECTIVE_GUARD_SPOT:
					SetObjective(OBJECTIVE_GUARD_SPOT, m_leader->m_vecSeekPosEx);
					m_objectiveTimer = m_leader->m_objectiveTimer;
					break;
				case OBJECTIVE_KILL_CHAR_ON_FOOT:
				case OBJECTIVE_KILL_CHAR_ANY_MEANS:
				case OBJECTIVE_GOTO_CHAR_ON_FOOT:
					if (m_leader->m_pedInObjective) {
						SetObjective(m_leader->m_objective, m_leader->m_pedInObjective);
						m_objectiveTimer = m_leader->m_objectiveTimer;
					}
					break;
				case OBJECTIVE_ENTER_CAR_AS_PASSENGER:
				case OBJECTIVE_ENTER_CAR_AS_DRIVER:
					if (m_leader->m_carInObjective) {
						SetObjective(OBJECTIVE_ENTER_CAR_AS_PASSENGER, m_leader->m_carInObjective);
						return;
					}
					break;
				case OBJECTIVE_GUARD_ATTACK:
					return;
				case OBJECTIVE_HAIL_TAXI:
					m_leader = nil;
					SetObjective(OBJECTIVE_NONE);
					break;
				default:
					SetObjective(OBJECTIVE_GOTO_CHAR_ON_FOOT, m_leader);
					SetObjectiveTimer(0);
					break;
			}
		}
	} else if (bInVehicle) {
		if ((!m_leader->bInVehicle || m_leader->m_nPedState == PED_EXIT_CAR) && m_objective != OBJECTIVE_WAIT_IN_CAR_THEN_GET_OUT) {

			switch (m_leader->m_objective) {
				case OBJECTIVE_ENTER_CAR_AS_PASSENGER:
				case OBJECTIVE_ENTER_CAR_AS_DRIVER:
					if (m_pMyVehicle == m_leader->m_pMyVehicle || m_pMyVehicle == m_leader->m_carInObjective)
						break;

					// fall through
				default:
					if (m_pMyVehicle && m_objective != OBJECTIVE_LEAVE_CAR) {
#ifdef VC_PED_PORTS
						m_leaveCarTimer = CTimer::GetTimeInMilliseconds() + 250;
#endif
						SetObjective(OBJECTIVE_LEAVE_CAR, m_pMyVehicle);
					}

					break;
			}
		}
	}
}

void
CPed::RestorePreviousObjective(void)
{
	if (m_objective == OBJECTIVE_NONE)
		return;

	if (m_objective != OBJECTIVE_LEAVE_CAR && m_objective != OBJECTIVE_ENTER_CAR_AS_PASSENGER && m_objective != OBJECTIVE_ENTER_CAR_AS_DRIVER
#if defined VC_PED_PORTS || defined FIX_BUGS
		&& m_nPedState != PED_CARJACK
#endif
		)
		m_pedInObjective = nil;

	if (m_objective == OBJECTIVE_WAIT_IN_CAR_THEN_GET_OUT) {
		m_objective = OBJECTIVE_NONE;
		if (m_pMyVehicle)
			SetObjective(OBJECTIVE_LEAVE_CAR, m_pMyVehicle);

	} else {
		m_objective = m_prevObjective;
		m_prevObjective = OBJECTIVE_NONE;
	}
	bObjectiveCompleted = false;
}

void
CPed::ProcessObjective(void)
{
	if (bClearObjective && (IsPedInControl() || m_nPedState == PED_DRIVING)) {
		ClearObjective();
		bClearObjective = false;
	}
	UpdateFromLeader();

	CVector carOrOurPos;
	CVector targetCarOrHisPos;
	CVector distWithTarget;

	if (m_objective != OBJECTIVE_NONE && (IsPedInControl() || m_nPedState == PED_DRIVING)) {
		if (bInVehicle) {
			if (!m_pMyVehicle) {
				bInVehicle = false;
				return;
			}
			carOrOurPos = m_pMyVehicle->GetPosition();
		} else {
			carOrOurPos = GetPosition();
		}

		if (m_pedInObjective) {
			if (m_pedInObjective->InVehicle() && m_pedInObjective->m_nPedState != PED_DRAG_FROM_CAR) {
				targetCarOrHisPos = m_pedInObjective->m_pMyVehicle->GetPosition();
			} else {
				targetCarOrHisPos = m_pedInObjective->GetPosition();
			}
			distWithTarget = targetCarOrHisPos - carOrOurPos;
		} else if (m_carInObjective) {
			targetCarOrHisPos = m_carInObjective->GetPosition();
			distWithTarget = targetCarOrHisPos - carOrOurPos;
		}

		switch (m_objective) {
			case OBJECTIVE_NONE:
			case OBJECTIVE_GUARD_AREA:
			case OBJECTIVE_FOLLOW_CAR_IN_CAR:
			case OBJECTIVE_FIRE_AT_OBJECT_FROM_VEHICLE:
			case OBJECTIVE_DESTROY_OBJECT:
			case OBJECTIVE_GOTO_AREA_IN_CAR:
			case OBJECTIVE_FOLLOW_CAR_ON_FOOT_WITH_OFFSET:
			case OBJECTIVE_SET_LEADER:
				break;
			case OBJECTIVE_WAIT_ON_FOOT:
				SetIdle();
				m_objective = OBJECTIVE_NONE;
				SetMoveState(PEDMOVE_STILL);
				break;
			case OBJECTIVE_FLEE_ON_FOOT_TILL_SAFE:
				if (InVehicle()) {
					SetObjective(OBJECTIVE_LEAVE_CAR, m_pMyVehicle);
					bFleeAfterExitingCar = true;
				} else if (m_nPedState != PED_FLEE_POS) {
					CVector2D fleePos = GetPosition();
					SetFlee(fleePos, 10000);
					bUsePedNodeSeek = true;
					m_pNextPathNode = nil;
				}
				break;
			case OBJECTIVE_GUARD_SPOT:
			{
				distWithTarget = m_vecSeekPosEx - GetPosition();
				if (m_pedInObjective) {
					SetLookFlag(m_pedInObjective, true);
					m_pLookTarget = m_pedInObjective;
					m_pLookTarget->RegisterReference((CEntity **) &m_pLookTarget);
					TurnBody();
				}
				float distWithTargetSc = distWithTarget.Magnitude();
				if (2.0f * m_distanceToCountSeekDoneEx >= distWithTargetSc) {
					if (m_pedInObjective) {
						if (distWithTargetSc <= m_distanceToCountSeekDoneEx)
							SetIdle();
						else
							SetSeek(m_vecSeekPosEx, m_distanceToCountSeekDoneEx);
					} else if (CTimer::GetTimeInMilliseconds() > m_lookTimer) {
						int threatType = ScanForThreats();
						SetLookTimer(CGeneral::GetRandomNumberInRange(500, 1500));

						// Second condition is pointless and isn't there in Mobile.
						if (threatType == PED_FLAG_GUN || (threatType == PED_FLAG_EXPLOSION && m_threatEntity) || m_threatEntity) {
							if (m_threatEntity->IsPed())
								SetObjective(OBJECTIVE_KILL_CHAR_ON_FOOT, m_threatEntity);
						}
					}
				} else {
					SetSeek(m_vecSeekPosEx, m_distanceToCountSeekDoneEx);
				}
				break;
			}
			case OBJECTIVE_WAIT_IN_CAR:
				SetPedState(PED_DRIVING);
				break;
			case OBJECTIVE_WAIT_IN_CAR_THEN_GET_OUT:
				SetPedState(PED_DRIVING);
				break;
			case OBJECTIVE_KILL_CHAR_ANY_MEANS:
			{
				if (m_pedInObjective) {
					if (m_pedInObjective->IsPlayer() && CharCreatedBy != MISSION_CHAR
						&& m_nPedType != PEDTYPE_COP && FindPlayerPed()->m_pWanted->m_CurrentCops != 0
						&& !bKindaStayInSamePlace) {

						SetObjective(OBJECTIVE_FLEE_ON_FOOT_TILL_SAFE);
						break;
					}
					if (InVehicle()) {
						if (distWithTarget.Magnitude() >= 20.0f
							|| m_pMyVehicle->m_vecMoveSpeed.MagnitudeSqr() >= sq(0.02f)) {
							if (m_pMyVehicle->pDriver == this
								&& !m_pMyVehicle->m_nGettingInFlags) {
								m_pMyVehicle->SetStatus(STATUS_PHYSICS);
								m_pMyVehicle->AutoPilot.m_nPrevRouteNode = 0;
								if (m_nPedType == PEDTYPE_COP) {
									m_pMyVehicle->AutoPilot.m_nCruiseSpeed = (FindPlayerPed()->m_pWanted->GetWantedLevel() * 0.1f + 0.6f) * (GAME_SPEED_TO_CARAI_SPEED * m_pMyVehicle->pHandling->Transmission.fMaxCruiseVelocity);
									m_pMyVehicle->AutoPilot.m_nCarMission = CCarAI::FindPoliceCarMissionForWantedLevel();
								} else {
									m_pMyVehicle->AutoPilot.m_nCruiseSpeed = GAME_SPEED_TO_CARAI_SPEED * m_pMyVehicle->pHandling->Transmission.fMaxCruiseVelocity * 0.8f;
									m_pMyVehicle->AutoPilot.m_nCarMission = MISSION_RAMPLAYER_FARAWAY;
								}
								m_pMyVehicle->AutoPilot.m_nDrivingStyle = DRIVINGSTYLE_AVOID_CARS;
							}
						} else {
							bool targetHasVeh = m_pedInObjective->bInVehicle;
							if (!targetHasVeh
								|| targetHasVeh && m_pedInObjective->m_pMyVehicle->CanPedExitCar()) {
								m_pMyVehicle->AutoPilot.m_nCruiseSpeed = 0;
								m_pMyVehicle->AutoPilot.m_nCarMission = MISSION_NONE;
								SetObjective(OBJECTIVE_LEAVE_CAR, m_pMyVehicle);
							}
						}
						break;
					}
					if (distWithTarget.Magnitude() > 30.0f && !bKindaStayInSamePlace) {
						if (m_pMyVehicle) {
							m_pMyVehicle->AutoPilot.m_nCruiseSpeed = 0;
							SetObjective(OBJECTIVE_ENTER_CAR_AS_DRIVER, m_pMyVehicle);
						} else {
							float closestVehDist = 60.0f;
							int16 lastVehicle;
							CEntity* vehicles[8];
							CWorld::FindObjectsInRange(GetPosition(), 25.0f, true, &lastVehicle, 6, vehicles, false, true, false, false, false);
							CVehicle *foundVeh = nil;
							for(int i = 0; i < lastVehicle; i++) {
								CVehicle *nearVeh = (CVehicle*)vehicles[i];
								/*
								Not used.
								CVector vehSpeed = nearVeh->GetSpeed();
								CVector ourSpeed = GetSpeed();
								*/
								CVector vehDistVec = nearVeh->GetPosition() - GetPosition();
								if (vehDistVec.Magnitude() < closestVehDist && m_pedInObjective->m_pMyVehicle != nearVeh
									&& nearVeh->CanPedOpenLocks(this)) {

									foundVeh = nearVeh;
									closestVehDist = vehDistVec.Magnitude();
								}
							}
							m_pMyVehicle = foundVeh;
							if (m_pMyVehicle) {
								m_pMyVehicle->RegisterReference((CEntity **) &m_pMyVehicle);
								m_pMyVehicle->AutoPilot.m_nCruiseSpeed = 0;
								SetObjective(OBJECTIVE_ENTER_CAR_AS_DRIVER, m_pMyVehicle);
							} else if (!GetIsOnScreen()) {
								CVector ourPos = GetPosition();
								int closestNode = ThePaths.FindNodeClosestToCoors(ourPos, PATH_CAR, 20.0f);
								if (closestNode >= 0) {
									int16 colliding;
									CWorld::FindObjectsKindaColliding(
										ThePaths.m_pathNodes[closestNode].GetPosition(), 10.0f, true, &colliding, 2, nil, false, true, true, false, false);
									if (!colliding) {
										CZoneInfo zoneInfo;
										int chosenCarClass;
										CTheZones::GetZoneInfoForTimeOfDay(&ourPos, &zoneInfo);
										int chosenModel = CCarCtrl::ChooseModel(&zoneInfo, &ourPos, &chosenCarClass);
										CAutomobile *newVeh = new CAutomobile(chosenModel, RANDOM_VEHICLE);
										if (newVeh) {
										    newVeh->GetMatrix().GetPosition() = ThePaths.m_pathNodes[closestNode].GetPosition();
										    newVeh->GetMatrix().GetPosition().z += 4.0f;
											newVeh->SetHeading(DEGTORAD(200.0f));
											newVeh->SetStatus(STATUS_ABANDONED);
											newVeh->m_nDoorLock = CARLOCK_UNLOCKED;
											CWorld::Add(newVeh);
											m_pMyVehicle = newVeh;
											if (m_pMyVehicle) {
												m_pMyVehicle->RegisterReference((CEntity **) &m_pMyVehicle);
												m_pMyVehicle->AutoPilot.m_nCruiseSpeed = 0;
												SetObjective(OBJECTIVE_ENTER_CAR_AS_DRIVER, m_pMyVehicle);
											}
										}
									}
								}
							}
						}
						break;
					}
				} else {
					ClearLookFlag();
					bObjectiveCompleted = true;
				}
			}
			case OBJECTIVE_KILL_CHAR_ON_FOOT:
			{
				bool killPlayerInNoPoliceZone = false;
				if (m_objective == OBJECTIVE_KILL_CHAR_ON_FOOT && InVehicle()) {
					SetObjective(OBJECTIVE_LEAVE_CAR, m_pMyVehicle);
					break;
				}
				if (!m_pedInObjective || m_pedInObjective->DyingOrDead()) {
					ClearLookFlag();
					bObjectiveCompleted = true;
					SetMoveAnim();
					break;
				}
				if (m_pedInObjective->IsPlayer() && CCullZones::NoPolice())
					killPlayerInNoPoliceZone = true;

				if (!bNotAllowedToDuck || killPlayerInNoPoliceZone) {
					if (m_nPedType == PEDTYPE_COP && !m_pedInObjective->GetWeapon()->IsTypeMelee() && !GetWeapon()->IsTypeMelee())
						bNotAllowedToDuck = true;
				} else {
					if (!m_pedInObjective->bInVehicle) {
						if (m_pedInObjective->GetWeapon()->IsTypeMelee() || GetWeapon()->IsTypeMelee()) {
							bNotAllowedToDuck = false;
							bCrouchWhenShooting = false;
						} else if (DuckAndCover()) {
							break;
						}
					} else {
						bNotAllowedToDuck = false;
						bCrouchWhenShooting = false;
					}
				}
				if (m_leaveCarTimer > CTimer::GetTimeInMilliseconds() && !bKindaStayInSamePlace) {
					SetMoveState(PEDMOVE_STILL);
					break;
				}
				if (m_pedInObjective->IsPlayer()) {
					CPlayerPed *player = FindPlayerPed();
					if (m_nPedType == PEDTYPE_COP && player->m_pWanted->m_bIgnoredByCops
						|| player->m_pWanted->m_bIgnoredByEveryone
						|| m_pedInObjective->bIsInWater
						|| m_pedInObjective->m_nPedState == PED_ARRESTED) {

						if (m_nPedState != PED_ARREST_PLAYER)
							SetIdle();

						break;
					}
				}
				CWeaponInfo *wepInfo = CWeaponInfo::GetWeaponInfo(GetWeapon()->m_eWeaponType);
				float wepRange = wepInfo->m_fRange;
				float maxDistToKeep;
				if (GetWeapon()->m_eWeaponType != WEAPONTYPE_UNARMED) {
					maxDistToKeep = wepRange / 3.0f;
				} else {
					if (m_nPedState == PED_FIGHT) {
						if (!IsPlayer() && !(m_pedStats->m_flags & STAT_CAN_KICK))
							wepRange = 2.0f;
					} else {
						wepRange = 1.3f;
					}
					maxDistToKeep = wepRange;
				}
				if (m_pedInObjective->m_getUpTimer > CTimer::GetTimeInMilliseconds() && maxDistToKeep < 2.5f) {
					maxDistToKeep = 2.5f;
				}
				if (m_pedInObjective->IsPlayer() && m_nPedType != PEDTYPE_COP
					&& CharCreatedBy != MISSION_CHAR && FindPlayerPed()->m_pWanted->m_CurrentCops) {
					SetObjective(OBJECTIVE_FLEE_ON_FOOT_TILL_SAFE);
					break;
				}
				if (m_pedInObjective->m_fHealth <= 0.0f) {
					bObjectiveCompleted = true;
					bScriptObjectiveCompleted = true;
					SetMoveAnim();
					break;
				}
				float distWithTargetSc = distWithTarget.Magnitude();
				if (m_pedInObjective->bInVehicle && m_pedInObjective->m_nPedState != PED_DRAG_FROM_CAR) {
					CVehicle *vehOfTarget = m_pedInObjective->m_pMyVehicle;
					if (vehOfTarget->bIsInWater || vehOfTarget->GetStatus() == STATUS_PLAYER_DISABLED
						|| m_pedInObjective->IsPlayer() && CPad::GetPad(0)->ArePlayerControlsDisabled()) {
						SetIdle();
						return;
					}
					SetLookFlag(vehOfTarget, false);
					if (m_nPedState != PED_CARJACK) {
						if (m_pedInObjective->m_nPedState != PED_ARRESTED) {
							if (m_attackTimer < CTimer::GetTimeInMilliseconds() && wepInfo->m_eWeaponFire != WEAPON_FIRE_MELEE
								&& distWithTargetSc < wepRange && distWithTargetSc > 3.0f) {

								// I hope so
								CVector ourHead = GetMatrix() * CVector(0.5f, 0.0f, 0.6f);
								CVector maxShotPos = vehOfTarget->GetPosition() - ourHead;
								maxShotPos *= wepInfo->m_fRange / maxShotPos.Magnitude();
								maxShotPos += ourHead;

								CColPoint foundCol;
								CEntity *foundEnt;

								CWorld::bIncludeDeadPeds = true;
								CWorld::ProcessLineOfSight(ourHead, maxShotPos, foundCol, foundEnt, true, true, true, true, false, true, false);
								CWorld::bIncludeDeadPeds = false;
								if (foundEnt == vehOfTarget) {
									SetAttack(vehOfTarget);
									SetWeaponLockOnTarget(vehOfTarget);
									SetShootTimer(CGeneral::GetRandomNumberInRange(500, 2000));
									if (distWithTargetSc <= m_distanceToCountSeekDone) {
										SetAttackTimer(CGeneral::GetRandomNumberInRange(200, 500));
										SetMoveState(PEDMOVE_STILL);
									} else {
										SetAttackTimer(CGeneral::GetRandomNumberInRange(2000, 5000));
									}
								}
							} else if (m_nPedState != PED_ATTACK && !bKindaStayInSamePlace && !killPlayerInNoPoliceZone) {
								if (vehOfTarget) {
									if (m_nPedType == PEDTYPE_COP || vehOfTarget->bIsBus) {
										GoToNearestDoor(vehOfTarget);
									} else {
										m_vehDoor = 0;
										if (m_pedInObjective == vehOfTarget->pDriver || vehOfTarget->bIsBus) {
											m_vehDoor = CAR_DOOR_LF;
										} else if (m_pedInObjective == vehOfTarget->pPassengers[0]) {
											m_vehDoor = CAR_DOOR_RF;
										} else if (m_pedInObjective == vehOfTarget->pPassengers[1]) {
											m_vehDoor = CAR_DOOR_LR;
										} else if (m_pedInObjective == vehOfTarget->pPassengers[2]) {
											m_vehDoor = CAR_DOOR_RR;
										}
										// Unused
										// GetPositionToOpenCarDoor(vehOfTarget, m_vehDoor);
										SetSeekCar(vehOfTarget, m_vehDoor);
										SetMoveState(PEDMOVE_RUN);
									}
								}
							}
						}
					}
					SetMoveAnim();
					break;
				}
				if (m_nMoveState == PEDMOVE_STILL && IsPedInControl()) {
					SetLookFlag(m_pedInObjective, false);
					TurnBody();
				}
				if (m_nPedType == PEDTYPE_COP && distWithTargetSc < 1.5f && m_pedInObjective->IsPlayer()) {
					if (m_pedInObjective->m_getUpTimer > CTimer::GetTimeInMilliseconds()
						|| m_pedInObjective->m_nPedState == PED_DRAG_FROM_CAR) {

						((CCopPed*)this)->SetArrestPlayer(m_pedInObjective);
						return;
					}
				}
				if (!bKindaStayInSamePlace && !bStopAndShoot && m_nPedState != PED_ATTACK && !killPlayerInNoPoliceZone) {
					if (distWithTargetSc > wepRange
						|| m_pedInObjective->m_getUpTimer > CTimer::GetTimeInMilliseconds()
						|| m_pedInObjective->m_nPedState == PED_ARRESTED
						|| m_pedInObjective->EnteringCar() && distWithTargetSc < 3.0f
						|| distWithTargetSc > m_distanceToCountSeekDone && !CanSeeEntity(m_pedInObjective)) {

						if (m_pedInObjective->EnteringCar())
							maxDistToKeep = 2.0f;

						if (bUsePedNodeSeek) {
							CVector bestCoords(0.0f, 0.0f, 0.0f);
							m_vecSeekPos = m_pedInObjective->GetPosition();

							if (!m_pNextPathNode)
								FindBestCoordsFromNodes(m_vecSeekPos, &bestCoords);

							if (m_pNextPathNode)
								m_vecSeekPos = m_pNextPathNode->GetPosition();

							SetSeek(m_vecSeekPos, m_distanceToCountSeekDone);
						} else {
							SetSeek(m_pedInObjective, maxDistToKeep);
						}
						bCrouchWhenShooting = false;
						if (m_pedInObjective->m_pCurrentPhysSurface && distWithTargetSc < 5.0f) {
							if (wepRange <= 5.0f) {
								if (m_pedInObjective->IsPlayer()
									&& FindPlayerPed()->m_bSpeedTimerFlag
									&& (IsGangMember() || m_nPedType == PEDTYPE_COP)
									&& GetWeapon()->m_eWeaponType == WEAPONTYPE_UNARMED) {
									GiveWeapon(WEAPONTYPE_COLT45, 1000);
									SetCurrentWeapon(WEAPONTYPE_COLT45);
								}
							} else {
								bStopAndShoot = true;
							}
							SetMoveState(PEDMOVE_STILL);
							SetMoveAnim();
							break;
						}
						bStopAndShoot = false;
						SetMoveAnim();
						break;
					}
				}
				if (m_attackTimer < CTimer::GetTimeInMilliseconds()
					&& distWithTargetSc < wepRange && m_pedInObjective->m_nPedState != PED_GETUP && m_pedInObjective->m_nPedState != PED_DRAG_FROM_CAR) {
					if (bIsDucking) {
						CAnimBlendAssociation *duckAnim = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_DUCK_DOWN);
						if (duckAnim) {
							duckAnim->blendDelta = -2.0f;
							break;
						}
						bIsDucking = false;
					} else if (wepRange <= 5.0f) {
						SetMoveState(PEDMOVE_STILL);
						SetAttack(m_pedInObjective);
						m_fRotationDest = CGeneral::GetRadianAngleBetweenPoints(
							m_pedInObjective->GetPosition().x, m_pedInObjective->GetPosition().y,
							GetPosition().x, GetPosition().y);
						SetShootTimer(CGeneral::GetRandomNumberInRange(0.0f, 500.0f));
						SetAttackTimer(CGeneral::GetRandomNumberInRange(0.0f, 1500.0f));
						bObstacleShowedUpDuringKillObjective = false;

					} else {
						CVector target;
						CVector ourHead = GetMatrix() * CVector(0.5f, 0.0f, 0.6f);
						if (m_pedInObjective->IsPed())
							m_pedInObjective->m_pedIK.GetComponentPosition(target, PED_MID);
						else
							target = m_pedInObjective->GetPosition();

						target -= ourHead;
						target *= wepInfo->m_fRange / target.Magnitude();
						target += ourHead;

						CColPoint foundCol;
						CEntity *foundEnt = nil;

						CWorld::bIncludeDeadPeds = true;
						CWorld::ProcessLineOfSight(ourHead, target, foundCol, foundEnt, true, true, true, true, false, true, false);
						CWorld::bIncludeDeadPeds = false;
						if (foundEnt == m_pedInObjective) {
							SetAttack(m_pedInObjective);
							SetWeaponLockOnTarget(m_pedInObjective);
							SetShootTimer(CGeneral::GetRandomNumberInRange(500.0f, 2000.0f));

							int time;
							if (distWithTargetSc <= maxDistToKeep)
								time = CGeneral::GetRandomNumberInRange(100.0f, 500.0f);
							else
								time = CGeneral::GetRandomNumberInRange(1500.0f, 3000.0f);

							SetAttackTimer(time);
							bObstacleShowedUpDuringKillObjective = false;

						} else {
							if (foundEnt) {
								if (foundEnt->IsPed()) {
									SetAttackTimer(CGeneral::GetRandomNumberInRange(500.0f, 1000.0f));
									bObstacleShowedUpDuringKillObjective = false;
								} else {
									if (foundEnt->IsObject()) {
										SetAttackTimer(CGeneral::GetRandomNumberInRange(200.0f, 400.0f));
										bObstacleShowedUpDuringKillObjective = true;
									} else if (foundEnt->IsVehicle()) {
										SetAttackTimer(CGeneral::GetRandomNumberInRange(400.0f, 600.0f));
										bObstacleShowedUpDuringKillObjective = true;
									} else {
										SetAttackTimer(CGeneral::GetRandomNumberInRange(700.0f, 1200.0f));
										bObstacleShowedUpDuringKillObjective = true;
									}
								}

								m_fleeFrom = foundEnt;
								m_fleeFrom->RegisterReference((CEntity**) &m_fleeFrom);
							}
							SetPointGunAt(m_pedInObjective);
						}
					}
				} else {
					if (!m_pedInObjective->m_pCurrentPhysSurface)
						bStopAndShoot = false;

					if (m_nPedState != PED_ATTACK && m_nPedState != PED_FIGHT) {

						// This is weird...
						if (bNotAllowedToDuck && bKindaStayInSamePlace) {
							if (!bIsDucking) {
								CAnimBlendAssociation* duckAnim = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_DUCK_DOWN);
								if (!duckAnim || duckAnim->blendDelta < 0.0f) {
									CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_DUCK_DOWN, 4.0f);
									bIsDucking = true;
								}
								break;
							} else {
								CAnimBlendAssociation* duckAnim = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_DUCK_DOWN);
								if (!duckAnim || duckAnim->blendDelta < 0.0f) {
									bIsDucking = false;
								} else {
									break;
								}
							}
						}
						if (bObstacleShowedUpDuringKillObjective) {
							if (m_nPedType == PEDTYPE_COP) {
								if (GetWeapon()->m_eWeaponType > WEAPONTYPE_COLT45
									|| m_fleeFrom && m_fleeFrom->IsObject()) {
									maxDistToKeep = 6.0f;
								} else if (m_fleeFrom && m_fleeFrom->IsVehicle()) {
									maxDistToKeep = 4.0f;
								} else {
									maxDistToKeep = 2.0f;
								}
							} else {
								maxDistToKeep = 2.0f;
							}
						}
						if (distWithTargetSc <= maxDistToKeep) {
							SetMoveState(PEDMOVE_STILL);
							bIsPointingGunAt = true;
							if (m_nPedState != PED_AIM_GUN && !bDuckAndCover) {
								m_attackTimer = CTimer::GetTimeInMilliseconds();
								SetIdle();
							}
						} else {
							if (m_nPedState != PED_SEEK_ENTITY && m_nPedState != PED_SEEK_POS
								&& !bStopAndShoot && !killPlayerInNoPoliceZone && !bKindaStayInSamePlace) {
								Say(SOUND_PED_ATTACK);
								SetSeek(m_pedInObjective, maxDistToKeep);
								bIsRunning = true;
							}
						}
					}
				}

				if (distWithTargetSc < 2.5f && wepRange > 5.0f
					&& wepInfo->m_eWeaponFire != WEAPON_FIRE_MELEE) {

					SetAttack(m_pedInObjective);
					if (m_attackTimer < CTimer::GetTimeInMilliseconds()) {
						int time = CGeneral::GetRandomNumberInRange(500.0f, 1000.0f);
						SetAttackTimer(time);
						SetShootTimer(time - 500);
					}
					SetMoveState(PEDMOVE_STILL);
				}
				if (CTimer::GetTimeInMilliseconds() > m_nPedStateTimer)
					m_fRotationDest = CGeneral::GetRadianAngleBetweenPoints(m_pedInObjective->GetPosition().x, m_pedInObjective->GetPosition().y, GetPosition().x, GetPosition().y);
				
				SetMoveAnim();
				break;
			}
			case OBJECTIVE_FLEE_CHAR_ON_FOOT_TILL_SAFE:
			case OBJECTIVE_FLEE_CHAR_ON_FOOT_ALWAYS:
			{
				if (InVehicle()) {
					if (m_nPedState == PED_DRIVING)
						SetObjective(OBJECTIVE_LEAVE_CAR, m_pMyVehicle);
				} else if (m_nPedState != PED_FLEE_ENTITY) {
					int time;
					if (m_objective == OBJECTIVE_FLEE_CHAR_ON_FOOT_ALWAYS)
						time = 0;
					else
						time = 6000;

					SetFindPathAndFlee(m_pedInObjective, time);
				}
				break;
			}
			case OBJECTIVE_GOTO_CHAR_ON_FOOT:
			{
				if (m_pedInObjective) {
					float safeDistance = 2.0f;
					if (m_pedInObjective->bInVehicle)
						safeDistance = 3.0f;

					float distWithTargetSc = distWithTarget.Magnitude();
					if (m_nPedStateTimer < CTimer::GetTimeInMilliseconds()) {
						if (distWithTargetSc <= safeDistance) {
							bScriptObjectiveCompleted = true;
							if (m_nPedState != PED_ATTACK) {
								SetIdle();
								m_pLookTarget = m_pedInObjective;
								m_pLookTarget->RegisterReference((CEntity **) &m_pLookTarget);
								TurnBody();
							}
							if (distWithTargetSc > 2.0f)
								SetMoveState(m_pedInObjective->m_nMoveState);
							else
								SetMoveState(PEDMOVE_STILL);
						} else {
							SetSeek(m_pedInObjective, safeDistance);
							if (distWithTargetSc >= 5.0f) {
								if (m_leader && m_leader->m_nMoveState == PEDMOVE_SPRINT)
									SetMoveState(PEDMOVE_SPRINT);
								else
									SetMoveState(PEDMOVE_RUN);
							} else {
								if (m_leader && m_leader->m_nMoveState != PEDMOVE_STILL
									&& m_leader->m_nMoveState != PEDMOVE_NONE) {
									if (m_leader->IsPlayer()) {
										if (distWithTargetSc >= 3.0f && FindPlayerPed()->m_fMoveSpeed >= 1.3f)
											SetMoveState(PEDMOVE_RUN);
										else
											SetMoveState(PEDMOVE_WALK);
									} else {
										SetMoveState(m_leader->m_nMoveState);
									}
								} else if (distWithTargetSc <= 3.0f) {
									SetMoveState(PEDMOVE_WALK);
								} else {
									SetMoveState(PEDMOVE_RUN);
								}
							}
						}
					}
				} else {
					SetObjective(OBJECTIVE_NONE);
				}
				break;
			}
			case OBJECTIVE_FOLLOW_CHAR_IN_FORMATION:
			{
				if (m_pedInObjective) {
					CVector posToGo = GetFormationPosition();
					distWithTarget = posToGo - carOrOurPos;
					SetSeek(posToGo, 1.0f);
					if (distWithTarget.Magnitude() <= 3.0f) {
						SetSeek(posToGo, 1.0f);
						if (m_pedInObjective->m_nMoveState != PEDMOVE_STILL)
							SetMoveState(m_pedInObjective->m_nMoveState);
					} else {
						SetSeek(posToGo, 1.0f);
						SetMoveState(PEDMOVE_RUN);
					}
				} else {
					SetObjective(OBJECTIVE_NONE);
				}
				break;
			}
			case OBJECTIVE_ENTER_CAR_AS_PASSENGER:
			{
				if (m_carInObjective) {
					if (!bInVehicle && m_carInObjective->m_nNumPassengers >= m_carInObjective->m_nNumMaxPassengers) {
						RestorePreviousObjective();
						RestorePreviousState();
						if (IsPedInControl())
							m_pMyVehicle = nil;

						break;
					}

					if (m_prevObjective == OBJECTIVE_HAIL_TAXI && !((CAutomobile*)m_carInObjective)->bTaxiLight) {
						RestorePreviousObjective();
						ClearObjective();
						SetWanderPath(CGeneral::GetRandomNumber() & 7);
						bIsRunning = false;
						break;
					}
					if (m_objectiveTimer && m_objectiveTimer < CTimer::GetTimeInMilliseconds()) {
						if (!EnteringCar()) {
							bool foundSeat = false;
							if (m_carInObjective->pPassengers[0] || m_carInObjective->m_nGettingInFlags & CAR_DOOR_FLAG_RF) {
								if (m_carInObjective->pPassengers[1] || m_carInObjective->m_nGettingInFlags & CAR_DOOR_FLAG_LR) {
									if (m_carInObjective->pPassengers[2] || m_carInObjective->m_nGettingInFlags & CAR_DOOR_FLAG_RR) {
										foundSeat = false;
									} else {
										m_vehDoor = CAR_DOOR_RR;
										foundSeat = true;
									}
								} else {
									m_vehDoor = CAR_DOOR_LR;
									foundSeat = true;
								}
							} else {
								m_vehDoor = CAR_DOOR_RF;
								foundSeat = true;
							}
							for (int i = 2; i < m_carInObjective->m_nNumMaxPassengers; ++i) {
								if (!m_carInObjective->pPassengers[i] && !(m_carInObjective->m_nGettingInFlags & CAR_DOOR_FLAG_RF)) {
									m_vehDoor = CAR_DOOR_RF;
									foundSeat = true;
								}
							}
							if (foundSeat) {
								SetPosition(GetPositionToOpenCarDoor(m_carInObjective, m_vehDoor));
								SetEnterCar(m_carInObjective, m_vehDoor);
							}
						}
						m_objectiveTimer = 0;
					}
				}
				// fall through
			}
			case OBJECTIVE_ENTER_CAR_AS_DRIVER:
			{
				if (!m_carInObjective || bInVehicle) {
#ifdef VC_PED_PORTS
					if (bInVehicle && m_pMyVehicle != m_carInObjective) {
						SetExitCar(m_pMyVehicle, 0);
					} else
#endif
					{
						bObjectiveCompleted = true;
						bScriptObjectiveCompleted = true;
						RestorePreviousState();
					}
				} else {
					if (m_leaveCarTimer > CTimer::GetTimeInMilliseconds()) {
						SetMoveState(PEDMOVE_STILL);
						break;
					}
					if (IsPedInControl()) {
						if (m_prevObjective == OBJECTIVE_KILL_CHAR_ANY_MEANS) {
							if (distWithTarget.Magnitude() < 20.0f) {
								RestorePreviousObjective();
								RestorePreviousState();
								return;
							}
							if (m_objective == OBJECTIVE_ENTER_CAR_AS_DRIVER) {
								if (m_carInObjective->pDriver
#ifdef VC_PED_PORTS
									&& !IsPlayer()
#endif
									) {
									if (m_carInObjective->pDriver->m_objective == OBJECTIVE_KILL_CHAR_ANY_MEANS && m_carInObjective->pDriver != m_pedInObjective) {
										SetObjective(OBJECTIVE_ENTER_CAR_AS_PASSENGER, m_carInObjective);
										m_carInObjective->bIsBeingCarJacked = false;
									}
								}
							}
						} else if (m_objective != OBJECTIVE_ENTER_CAR_AS_PASSENGER) {
							if (m_carInObjective->pDriver
#ifdef VC_PED_PORTS
								&& (CharCreatedBy != MISSION_CHAR || m_carInObjective->pDriver->CharCreatedBy != RANDOM_CHAR)
#endif
								) {
								if (m_carInObjective->pDriver->m_nPedType == m_nPedType) {
									SetObjective(OBJECTIVE_ENTER_CAR_AS_PASSENGER, m_carInObjective);
									m_carInObjective->bIsBeingCarJacked = false;
								}
							}
						}
						if (m_carInObjective->IsUpsideDown() && m_carInObjective->m_vehType != VEHICLE_TYPE_BIKE) {
							RestorePreviousObjective();
							RestorePreviousState();
							return;
						}
						if (!m_carInObjective->IsBoat() || m_nPedState == PED_SEEK_IN_BOAT) {
							if (m_nPedState != PED_SEEK_CAR)
								SetSeekCar(m_carInObjective, 0);
						} else {
							SetSeekBoatPosition(m_carInObjective);
						}
						if (m_nMoveState == PEDMOVE_STILL && !bVehEnterDoorIsBlocked)
							SetMoveState(PEDMOVE_RUN);

						if (m_carInObjective && m_carInObjective->m_fHealth > 0.0f) {
							distWithTarget = m_carInObjective->GetPosition() - GetPosition();
							if (!bInVehicle) {
								if (nEnterCarRangeMultiplier * ENTER_CAR_MAX_DIST < distWithTarget.Magnitude()) {
									if (!m_carInObjective->pDriver && !m_carInObjective->GetIsOnScreen() && !GetIsOnScreen())
										WarpPedToNearEntityOffScreen(m_carInObjective);

									if (CharCreatedBy != MISSION_CHAR || m_prevObjective == OBJECTIVE_KILL_CHAR_ANY_MEANS
#ifdef VC_PED_PORTS
										|| IsPlayer() && !CPad::GetPad(0)->ArePlayerControlsDisabled()
#endif
										) {
										RestorePreviousObjective();
										RestorePreviousState();
										if (IsPedInControl())
											m_pMyVehicle = nil;
									} else {
										SetIdle();
										SetMoveState(PEDMOVE_STILL);
									}
								}
							}
						} else if (!bInVehicle) {
							RestorePreviousObjective();
							RestorePreviousState();
							if (IsPedInControl())
								m_pMyVehicle = nil;
						}
					}
				}
				break;
			}
			case OBJECTIVE_DESTROY_CAR:
			{
				if (!m_carInObjective) {
					ClearLookFlag();
					bObjectiveCompleted = true;
					break;
				}
				float distWithTargetSc = distWithTarget.Magnitude();
				CWeaponInfo *wepInfo = CWeaponInfo::GetWeaponInfo(GetWeapon()->m_eWeaponType);
				float wepRange = wepInfo->m_fRange;
				m_pLookTarget = m_carInObjective;
				m_pLookTarget->RegisterReference((CEntity **) &m_pLookTarget);

				m_pSeekTarget = m_carInObjective;
				m_pSeekTarget->RegisterReference((CEntity**) &m_pSeekTarget);

				TurnBody();
				if (m_carInObjective->m_fHealth <= 0.0f) {
					ClearLookFlag();
					bScriptObjectiveCompleted = true;
					break;
				}

				if (m_attackTimer < CTimer::GetTimeInMilliseconds() && wepInfo->m_eWeaponFire != WEAPON_FIRE_MELEE
					&& distWithTargetSc < wepRange) {

					// I hope so
					CVector ourHead = GetMatrix() * CVector(0.5f, 0.0f, 0.6f);
					CVector maxShotPos = m_carInObjective->GetPosition() - ourHead;
					maxShotPos *= wepInfo->m_fRange / maxShotPos.Magnitude();
					maxShotPos += ourHead;

					CColPoint foundCol;
					CEntity *foundEnt;

					CWorld::bIncludeDeadPeds = true;
					CWorld::ProcessLineOfSight(ourHead, maxShotPos, foundCol, foundEnt, true, true, true, true, false, true, false);
					CWorld::bIncludeDeadPeds = false;
					if (foundEnt == m_carInObjective) {
						SetAttack(m_carInObjective);
						SetWeaponLockOnTarget(m_carInObjective);
						SetShootTimer(CGeneral::GetRandomNumberInRange(500, 2000));
						if (distWithTargetSc > 10.0f && !bKindaStayInSamePlace) {
							SetAttackTimer(CGeneral::GetRandomNumberInRange(2000, 5000));
						} else {
							SetAttackTimer(CGeneral::GetRandomNumberInRange(50, 300));
							SetMoveState(PEDMOVE_STILL);
						}
					}
				} else if (m_nPedState != PED_ATTACK && !bKindaStayInSamePlace) {

					float safeDistance;
					if (wepRange <= 5.0f)
						safeDistance = 3.0f;
					else
						safeDistance = wepRange * 0.25f;

					SetSeek(m_carInObjective, safeDistance);
					SetMoveState(PEDMOVE_RUN);
				}
				SetLookFlag(m_carInObjective, false);
				TurnBody();
				break;
			}
			case OBJECTIVE_GOTO_AREA_ANY_MEANS:
			{
				distWithTarget = m_nextRoutePointPos - GetPosition();
				distWithTarget.z = 0.0f;
				if (InVehicle()) {
					CCarAI::GetCarToGoToCoors(m_pMyVehicle, &m_nextRoutePointPos);
					CCarCtrl::RegisterVehicleOfInterest(m_pMyVehicle);
					if (distWithTarget.MagnitudeSqr() < sq(20.0f)) {
						m_pMyVehicle->AutoPilot.m_nCruiseSpeed = 0;
						ForceStoredObjective(OBJECTIVE_GOTO_AREA_ANY_MEANS);
						SetObjective(OBJECTIVE_LEAVE_CAR, m_pMyVehicle);
					}
					break;
				}
				if (distWithTarget.Magnitude() > 30.0f) {
					if (m_pMyVehicle) {
						m_pMyVehicle->AutoPilot.m_nCruiseSpeed = 0;
					} else {
						float closestVehDist = SQR(60.0f);
						int16 lastVehicle;
						CEntity* vehicles[8];
						CWorld::FindObjectsInRange(GetPosition(), 25.0f, true, &lastVehicle, 6, vehicles, false, true, false, false, false);
						CVehicle* foundVeh = nil;
						for (int i = 0; i < lastVehicle; i++) {
							CVehicle* nearVeh = (CVehicle*)vehicles[i];
							/*
							Not used.
							CVector vehSpeed = nearVeh->GetSpeed();
							CVector ourSpeed = GetSpeed();
							*/
							CVector vehDistVec = nearVeh->GetPosition() - GetPosition();
							if (vehDistVec.MagnitudeSqr() < closestVehDist
								&& m_pedInObjective->m_pMyVehicle != nearVeh)
							{
								foundVeh = nearVeh;
								closestVehDist = vehDistVec.MagnitudeSqr();
							}
						}
						m_pMyVehicle = foundVeh;
						if (m_pMyVehicle) {
							m_pMyVehicle->RegisterReference((CEntity **) &m_pMyVehicle);
							m_pMyVehicle->AutoPilot.m_nCruiseSpeed = 0;
							SetObjective(OBJECTIVE_ENTER_CAR_AS_DRIVER, m_pMyVehicle);
						}
					}
					break;
				}
				// fall through
			}
			case OBJECTIVE_GOTO_AREA_ON_FOOT:
			case OBJECTIVE_RUN_TO_AREA:
			{
				if ((m_objective == OBJECTIVE_GOTO_AREA_ON_FOOT || m_objective == OBJECTIVE_RUN_TO_AREA)
					&& InVehicle()) {
					SetObjective(OBJECTIVE_LEAVE_CAR, m_pMyVehicle);
				} else {
					distWithTarget = m_nextRoutePointPos - GetPosition();
					distWithTarget.z = 0.0f;
					if (sq(m_distanceToCountSeekDone) >= distWithTarget.MagnitudeSqr()) {
						bObjectiveCompleted = true;
						bScriptObjectiveCompleted = true;
						SetMoveState(PEDMOVE_STILL);
					} else if (CTimer::GetTimeInMilliseconds() > m_nPedStateTimer || m_nPedState != PED_SEEK_POS) {
						if (bUsePedNodeSeek) {
							CVector bestCoords(0.0f, 0.0f, 0.0f);
							m_vecSeekPos = m_nextRoutePointPos;

							if (!m_pNextPathNode)
								FindBestCoordsFromNodes(m_vecSeekPos, &bestCoords);

							if (m_pNextPathNode)
								m_vecSeekPos = m_pNextPathNode->GetPosition();
						}
						SetSeek(m_vecSeekPos, m_distanceToCountSeekDone);
					}
				}

				break;
			}
			case OBJECTIVE_GUARD_ATTACK:
			{
				if (m_pedInObjective) {
					SetLookFlag(m_pedInObjective, true);
					m_pLookTarget = m_pedInObjective;
					m_pLookTarget->RegisterReference((CEntity **) &m_pLookTarget);
					m_lookTimer = m_attackTimer;
					TurnBody();
					float distWithTargetSc = distWithTarget.Magnitude();
					if (distWithTargetSc >= 20.0f) {
						RestorePreviousObjective();
					} else if (m_attackTimer < CTimer::GetTimeInMilliseconds()) {
						if (m_nPedState != PED_SEEK_ENTITY && distWithTargetSc >= 2.0f) {
							SetSeek(m_pedInObjective, 1.0f);
						} else {
							SetAttack(m_pedInObjective);
							SetShootTimer(CGeneral::GetRandomNumberInRange(500.0f, 1500.0f));
						}
						SetAttackTimer(1000);
					}
				} else {
					RestorePreviousObjective();
				}
				break;
			}
			case OBJECTIVE_FOLLOW_ROUTE:
				if (HaveReachedNextPointOnRoute(1.0f)) {
					int nextPoint = GetNextPointOnRoute();
					m_nextRoutePointPos = CRouteNode::GetPointPosition(nextPoint);
				} else {
					SetSeek(m_nextRoutePointPos, 0.8f);
				}
				break;
			case OBJECTIVE_SOLICIT_VEHICLE:
				if (m_carInObjective) {
					if (m_objectiveTimer <= CTimer::GetTimeInMilliseconds()) {
						if (!bInVehicle) {
							SetObjective(OBJECTIVE_NONE);
							SetWanderPath(CGeneral::GetRandomNumber() & 7);
							m_objectiveTimer = CTimer::GetTimeInMilliseconds() + 10000;
							if (IsPedInControl())
								m_pMyVehicle = nil;
						}
					} else {
						if (m_nPedState != PED_FLEE_ENTITY && m_nPedState != PED_SOLICIT)
							SetSeekCar(m_carInObjective, 0);
					}
				} else {
					RestorePreviousObjective();
					RestorePreviousState();
					if (IsPedInControl())
						m_pMyVehicle = nil;
				}
				break;
			case OBJECTIVE_HAIL_TAXI:
				if (!RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_HAILTAXI) && CTimer::GetTimeInMilliseconds() > m_nWaitTimer) {
					Say(SOUND_PED_TAXI_WAIT);
					CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_HAILTAXI, 4.0f);
					m_nWaitTimer = CTimer::GetTimeInMilliseconds() + 2000;
				}
				break;
			case OBJECTIVE_CATCH_TRAIN:
			{
				if (m_carInObjective) {
					SetObjective(OBJECTIVE_ENTER_CAR_AS_PASSENGER, m_carInObjective);
				} else {
					CVehicle* trainToEnter = nil;
					float closestCarDist = CHECK_NEARBY_THINGS_MAX_DIST;
					CVector pos = GetPosition();
					int16 lastVehicle;
					CEntity* vehicles[8];

					CWorld::FindObjectsInRange(pos, 10.0f, true, &lastVehicle, 6, vehicles, false, true, false, false, false);
					for (int i = 0; i < lastVehicle; i++) {
						CVehicle* nearVeh = (CVehicle*)vehicles[i];
						if (nearVeh->IsTrain()) {
							CVector vehDistVec = GetPosition() - nearVeh->GetPosition();
							float vehDist = vehDistVec.Magnitude();
							if (vehDist < closestCarDist && m_pedInObjective->m_pMyVehicle != nearVeh)
							{
								trainToEnter = nearVeh;
								closestCarDist = vehDist;
							}
						}
					}
					if (trainToEnter) {
						m_carInObjective = trainToEnter;
						m_carInObjective->RegisterReference((CEntity **) &m_carInObjective);
					}
				}
				break;
			}
			case OBJECTIVE_BUY_ICE_CREAM:
				if (m_carInObjective) {
					if (m_nPedState != PED_FLEE_ENTITY && m_nPedState != PED_BUY_ICECREAM)
						SetSeekCar(m_carInObjective, 0);
				} else {
					RestorePreviousObjective();
					RestorePreviousState();
					if (IsPedInControl())
						m_pMyVehicle = nil;
				}
				break;
			case OBJECTIVE_STEAL_ANY_CAR:
			{
				if (bInVehicle) {
					bScriptObjectiveCompleted = true;
					RestorePreviousObjective();
				} else if (m_carJackTimer < CTimer::GetTimeInMilliseconds()) {
					CVehicle *carToSteal = nil;
					float closestCarDist = ENTER_CAR_MAX_DIST;
					CVector pos = GetPosition();
					int16 lastVehicle;
					CEntity *vehicles[8];

					// NB: This should've been ENTER_CAR_MAX_DIST actually, and is fixed in VC.
					CWorld::FindObjectsInRange(pos, CHECK_NEARBY_THINGS_MAX_DIST, true, &lastVehicle, 6, vehicles, false, true, false, false, false);
					for(int i = 0; i < lastVehicle; i++) {
						CVehicle *nearVeh = (CVehicle*)vehicles[i];
						if (nearVeh->VehicleCreatedBy != MISSION_VEHICLE) {
							if (nearVeh->m_vecMoveSpeed.Magnitude() <= 0.1f) {
								if (nearVeh->CanPedOpenLocks(this)) {
									CVector vehDistVec = GetPosition() - nearVeh->GetPosition();
									float vehDist = vehDistVec.Magnitude();
									if (vehDist < closestCarDist) {
										carToSteal = nearVeh;
										closestCarDist = vehDist;
									}
								}
							}
						}
					}
					if (carToSteal) {
						SetObjective(OBJECTIVE_ENTER_CAR_AS_DRIVER, carToSteal);
						m_carJackTimer = CTimer::GetTimeInMilliseconds() + 5000;
					} else {
						RestorePreviousObjective();
						RestorePreviousState();
					}
				}
				break;
			}
			case OBJECTIVE_MUG_CHAR:
			{
				if (m_pedInObjective) {
					if (m_pedInObjective->IsPlayer() || m_pedInObjective->bInVehicle || m_pedInObjective->m_fHealth <= 0.0f) {
						ClearObjective();
						return;
					}
					if (m_pedInObjective->m_nMoveState > PEDMOVE_WALK) {
						ClearObjective();
						return;
					}
					if (m_pedInObjective->m_objective == OBJECTIVE_KILL_CHAR_ON_FOOT && m_pedInObjective->m_pedInObjective == this) {
						SetObjective(OBJECTIVE_FLEE_CHAR_ON_FOOT_TILL_SAFE, m_pedInObjective);
						SetMoveState(PEDMOVE_SPRINT);
						return;
					}
					if (m_pedInObjective->m_nPedState == PED_FLEE_ENTITY && m_fleeFrom == this
						|| m_pedInObjective->m_objective == OBJECTIVE_FLEE_CHAR_ON_FOOT_TILL_SAFE && m_pedInObjective->m_pedInObjective == this) {
						ClearObjective();
						SetFindPathAndFlee(m_pedInObjective, 15000, true);
						return;
					}
					float distWithTargetScSqr = distWithTarget.MagnitudeSqr();
					if (distWithTargetScSqr <= sq(10.0f)) {
						if (distWithTargetScSqr <= sq(1.4f)) {
							CAnimBlendAssociation *reloadAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_AK_RELOAD);
							m_fRotationDest = CGeneral::GetRadianAngleBetweenPoints(
								m_pedInObjective->GetPosition().x, m_pedInObjective->GetPosition().y,
								GetPosition().x, GetPosition().y);

							if (reloadAssoc || !m_pedInObjective->IsPedShootable()) {
								if (reloadAssoc &&
									(!reloadAssoc->IsRunning() || reloadAssoc->currentTime / reloadAssoc->hierarchy->totalLength > 0.8f)) {
									CAnimBlendAssociation *punchAssoc = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_PARTIAL_PUNCH, 8.0f);
									punchAssoc->flags |= ASSOC_DELETEFADEDOUT;
									punchAssoc->flags |= ASSOC_FADEOUTWHENDONE;
									CVector2D offset(distWithTarget.x, distWithTarget.y);
									int dir = m_pedInObjective->GetLocalDirection(offset);
									m_pedInObjective->StartFightDefend(dir, HITLEVEL_HIGH, 5);
									m_pedInObjective->ReactToAttack(this);
									m_pedInObjective->Say(SOUND_PED_ROBBED);
									Say(SOUND_PED_MUGGING);
									bRichFromMugging = true;

									// VC FIX: ClearObjective() clears m_pedInObjective in VC (also same with VC_PED_PORTS), so get it before call
									CPed *victim = m_pedInObjective;
									ClearObjective();
									if (victim->m_objective != OBJECTIVE_KILL_CHAR_ON_FOOT
										|| victim->m_pedInObjective != this) {
										SetFindPathAndFlee(victim, 15000, true);
										m_nLastPedState = PED_WANDER_PATH;
									} else {
										SetObjective(OBJECTIVE_FLEE_CHAR_ON_FOOT_TILL_SAFE, victim);
										SetMoveState(PEDMOVE_SPRINT);
										m_nLastPedState = PED_WANDER_PATH;
									}
								}
							} else {
								eWeaponType weaponType = GetWeapon()->m_eWeaponType;
								if (weaponType != WEAPONTYPE_UNARMED && weaponType != WEAPONTYPE_BASEBALLBAT)
									SetCurrentWeapon(WEAPONTYPE_UNARMED);

								CAnimBlendAssociation *newReloadAssoc = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_AK_RELOAD, 8.0f);
								newReloadAssoc->flags |= ASSOC_DELETEFADEDOUT;
								newReloadAssoc->flags |= ASSOC_FADEOUTWHENDONE;
							}
						} else {
							SetSeek(m_pedInObjective, 1.0f);
							CAnimBlendAssociation *walkAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_WALK);

							if (walkAssoc)
								walkAssoc->speed = 1.3f;
						}
					} else {
						ClearObjective();
						SetWanderPath(CGeneral::GetRandomNumber() & 7);
					}
				} else {
#ifdef VC_PED_PORTS
					m_objective = OBJECTIVE_NONE;
#endif
					ClearObjective();
				}
				break;
			}
			case OBJECTIVE_FLEE_CAR:
				if (!bInVehicle && m_nPedState != PED_FLEE_ENTITY && m_pMyVehicle) {
					RestorePreviousObjective();
					SetFlee(m_pMyVehicle, 6000);
					break;
				}
				// fall through
			case OBJECTIVE_LEAVE_CAR:
				if (CTimer::GetTimeInMilliseconds() > m_leaveCarTimer) {
					if (InVehicle()
#ifdef VC_PED_PORTS
						&& (FindPlayerPed() != this || !CPad::GetPad(0)->GetAccelerate()
							|| bBusJacked)
#endif
						) {
						if (m_nPedState != PED_EXIT_CAR && m_nPedState != PED_DRAG_FROM_CAR && m_nPedState != PED_EXIT_TRAIN
							&& (m_nPedType != PEDTYPE_COP
#ifdef VC_PED_PORTS
								|| m_pMyVehicle->IsBoat()
#endif
								|| m_pMyVehicle->m_vecMoveSpeed.MagnitudeSqr2D() < sq(0.005f))) {
							if (m_pMyVehicle->IsTrain())
								SetExitTrain(m_pMyVehicle);
#ifdef VC_PED_PORTS
							else if (m_pMyVehicle->IsBoat())
								SetExitBoat(m_pMyVehicle);
#endif
							else
								SetExitCar(m_pMyVehicle, 0);
						}
					} else {
						RestorePreviousObjective();
					}
				}
				break;
#ifdef VC_PED_PORTS
			case OBJECTIVE_LEAVE_CAR_AND_DIE:
			{
				if (CTimer::GetTimeInMilliseconds() > m_leaveCarTimer) {
					if (InVehicle()) {
						if (m_nPedState != PED_EXIT_CAR && m_nPedState != PED_DRAG_FROM_CAR && m_nPedState != PED_EXIT_TRAIN) {
							if (m_pMyVehicle->IsBoat())
								SetExitBoat(m_pMyVehicle);
							else if (m_pMyVehicle->bIsBus)
								SetExitCar(m_pMyVehicle, 0);
							else {
								eCarNodes doorNode = CAR_DOOR_LF;
								if (m_pMyVehicle->pDriver != this) {
									if (m_pMyVehicle->pPassengers[0] == this) {
										doorNode = CAR_DOOR_RF;
									} else if (m_pMyVehicle->pPassengers[1] == this) {
										doorNode = CAR_DOOR_LR;
									} else if (m_pMyVehicle->pPassengers[2] == this) {
										doorNode = CAR_DOOR_RR;
									}
								}
								SetBeingDraggedFromCar(m_pMyVehicle, doorNode, false);
							}
						}
					}
				}
				break;
			}
#endif
		}
		if (bObjectiveCompleted
			|| m_objectiveTimer > 0 && CTimer::GetTimeInMilliseconds() > m_objectiveTimer) {
			RestorePreviousObjective();
			if (m_objectiveTimer > CTimer::GetTimeInMilliseconds() || !m_objectiveTimer)
				m_objectiveTimer = CTimer::GetTimeInMilliseconds() - 1;

			if (CharCreatedBy != RANDOM_CHAR || bInVehicle) {
				if (IsPedInControl())
					RestorePreviousState();
			} else {
				SetWanderPath(CGeneral::GetRandomNumber() & 7);
			}
			ClearAimFlag();
			ClearLookFlag();
		}
	}
}

void
CPed::SetFollowRoute(int16 currentPoint, int16 routeType)
{
	m_routeLastPoint = currentPoint;
	m_routeStartPoint = CRouteNode::GetRouteStart(currentPoint);
	m_routePointsPassed = 0;
	m_routeType = routeType;
	m_routePointsBeingPassed = 1;
	m_objective = OBJECTIVE_FOLLOW_ROUTE;
	m_nextRoutePointPos = CRouteNode::GetPointPosition(GetNextPointOnRoute());
}

int
CPed::GetNextPointOnRoute(void)
{
	int16 nextPoint = m_routePointsBeingPassed + m_routePointsPassed + m_routeStartPoint;

	// Route is complete
	if (nextPoint < 0 || nextPoint > NUMPEDROUTES || m_routeLastPoint != CRouteNode::GetRouteThisPointIsOn(nextPoint)) {

		switch (m_routeType) {
			case PEDROUTE_STOP_WHEN_DONE:
				nextPoint = -1;
				break;
			case PEDROUTE_GO_BACKWARD_WHEN_DONE:
				m_routePointsBeingPassed = -m_routePointsBeingPassed;
				nextPoint = m_routePointsBeingPassed + m_routePointsPassed + m_routeStartPoint;
				break;
			case PEDROUTE_GO_TO_START_WHEN_DONE:
				m_routePointsPassed = -1;
				nextPoint = m_routePointsBeingPassed + m_routePointsPassed + m_routeStartPoint;
				break;
			default:
				break;
		}
	}
	return nextPoint;
}

bool
CPed::HaveReachedNextPointOnRoute(float distToCountReached)
{
	if ((m_nextRoutePointPos - GetPosition()).Magnitude2D() >= distToCountReached)
		return false;

	m_routePointsPassed += m_routePointsBeingPassed;
	return true;
}

bool
CPed::CanSeeEntity(CEntity *entity, float threshold)
{
	float neededAngle = CGeneral::GetRadianAngleBetweenPoints(
		entity->GetPosition().x,
		entity->GetPosition().y,
		GetPosition().x,
		GetPosition().y);

	if (neededAngle < 0.0f)
		neededAngle += TWOPI;
	else if (neededAngle > TWOPI)
		neededAngle -= TWOPI;

	float ourAngle = m_fRotationCur;
	if (ourAngle < 0.0f)
		ourAngle += TWOPI;
	else if (ourAngle > TWOPI)
		ourAngle -= TWOPI;

	float neededTurn = Abs(neededAngle - ourAngle);

	return neededTurn < threshold || TWOPI - threshold < neededTurn;
}

// Only used while deciding which gun ped should switch to, if no ammo left.
bool
CPed::SelectGunIfArmed(void)
{
	for (int i = 0; i < m_maxWeaponTypeAllowed; i++) {
		if (GetWeapon(i).m_nAmmoTotal > 0) {
			eWeaponType weaponType = GetWeapon(i).m_eWeaponType;
			if (weaponType == WEAPONTYPE_BASEBALLBAT || weaponType == WEAPONTYPE_COLT45 || weaponType == WEAPONTYPE_UZI || weaponType == WEAPONTYPE_SHOTGUN ||
				weaponType == WEAPONTYPE_M16 || weaponType == WEAPONTYPE_SNIPERRIFLE || weaponType == WEAPONTYPE_ROCKETLAUNCHER) {
				SetCurrentWeapon(i);
				return true;
			}
		}
	}
	SetCurrentWeapon(WEAPONTYPE_UNARMED);
	return false;
}

void
CPed::ReactToPointGun(CEntity *entWithGun)
{
	CPed *pedWithGun = (CPed*)entWithGun;
	int waitTime;

	if (IsPlayer() || !IsPedInControl() || CharCreatedBy == MISSION_CHAR)
		return;

	if (m_leader == pedWithGun)
		return;

	if (m_nWaitState == WAITSTATE_PLAYANIM_HANDSUP || m_nWaitState == WAITSTATE_PLAYANIM_HANDSCOWER ||
		(GetPosition() - pedWithGun->GetPosition()).MagnitudeSqr2D() > 225.0f)
		return;

	if (m_leader) {
		if (FindPlayerPed() == m_leader)
			return;

		ClearLeader();
	}
	if (m_pedStats->m_flags & STAT_GUN_PANIC
		&& (m_nPedState != PED_ATTACK || GetWeapon()->IsTypeMelee())
		&& m_nPedState != PED_FLEE_ENTITY && m_nPedState != PED_AIM_GUN) {

		waitTime = CGeneral::GetRandomNumberInRange(3000, 6000);
		SetWaitState(WAITSTATE_PLAYANIM_HANDSCOWER, &waitTime);
		Say(SOUND_PED_HANDS_COWER);
		m_pLookTarget = pedWithGun;
		m_pLookTarget->RegisterReference((CEntity **) &m_pLookTarget);
		SetMoveState(PEDMOVE_NONE);

	} else if (m_nPedType != pedWithGun->m_nPedType) {
		if (IsGangMember() || m_nPedType == PEDTYPE_EMERGENCY || m_nPedType == PEDTYPE_FIREMAN) {
			RegisterThreatWithGangPeds(pedWithGun);
		}

		if (m_nPedType == PEDTYPE_COP) {
			if (pedWithGun->IsPlayer()) {
				((CPlayerPed*)pedWithGun)->m_pWanted->SetWantedLevelNoDrop(2);
				if (bCrouchWhenShooting || bKindaStayInSamePlace) {
					SetDuck(CGeneral::GetRandomNumberInRange(1000, 3000));
					return;
				}
			}
		}

		if (m_nPedType != PEDTYPE_COP
			&& (m_nPedState != PED_ATTACK || GetWeapon()->IsTypeMelee())
			&& (m_nPedState != PED_FLEE_ENTITY || pedWithGun->IsPlayer() && m_fleeFrom != pedWithGun)
			&& m_nPedState != PED_AIM_GUN && m_objective != OBJECTIVE_KILL_CHAR_ON_FOOT) {

			waitTime = CGeneral::GetRandomNumberInRange(3000, 6000);
			SetWaitState(WAITSTATE_PLAYANIM_HANDSUP, &waitTime);
			Say(SOUND_PED_HANDS_UP);
			m_pLookTarget = pedWithGun;
			m_pLookTarget->RegisterReference((CEntity **) &m_pLookTarget);
			SetMoveState(PEDMOVE_NONE);
			if (m_nPedState == PED_FLEE_ENTITY) {
				m_fleeFrom = pedWithGun;
				m_fleeFrom->RegisterReference((CEntity **) &m_fleeFrom);
			}

			if (FindPlayerPed() == pedWithGun && bRichFromMugging) {
				int money = CGeneral::GetRandomNumberInRange(100, 300);
				int pickupCount = money / 40 + 1;
				int moneyPerPickup = money / pickupCount;

				for (int i = 0; i < pickupCount; i++) {
					float pickupX = 1.5f * Sin((CGeneral::GetRandomNumber() % 256)/256.0f * TWOPI) + GetPosition().x;
					float pickupY = 1.5f * Cos((CGeneral::GetRandomNumber() % 256)/256.0f * TWOPI) + GetPosition().y;
					bool found = false;
					float groundZ = CWorld::FindGroundZFor3DCoord(pickupX, pickupY, GetPosition().z, &found) + 0.5f;
					if (found) {
						CPickups::GenerateNewOne(CVector(pickupX, pickupY, groundZ), MI_MONEY, PICKUP_MONEY, moneyPerPickup + (CGeneral::GetRandomNumber() & 7));
					}
				}
				bRichFromMugging = false;
			}
		}
	}
}

void
CPed::ReactToAttack(CEntity *attacker)
{
	if (IsPlayer() && attacker->IsPed()) {
		InformMyGangOfAttack(attacker);
		SetLookFlag(attacker, true);
		SetLookTimer(700);
		return;
	}

#ifdef VC_PED_PORTS
	if (m_nPedState == PED_DRIVING && InVehicle()
		&& (m_pMyVehicle->pDriver == this || m_pMyVehicle->pDriver && m_pMyVehicle->pDriver->m_nPedState == PED_DRIVING && m_pMyVehicle->pDriver->m_objective != OBJECTIVE_LEAVE_CAR_AND_DIE)) {

		if (m_pMyVehicle->VehicleCreatedBy == RANDOM_VEHICLE
			&& (m_pMyVehicle->GetStatus() == STATUS_SIMPLE || m_pMyVehicle->GetStatus() == STATUS_PHYSICS)
			&& m_pMyVehicle->AutoPilot.m_nCarMission == MISSION_CRUISE) {

			CCarCtrl::SwitchVehicleToRealPhysics(m_pMyVehicle);
			m_pMyVehicle->AutoPilot.m_nDrivingStyle = DRIVINGSTYLE_AVOID_CARS;
			m_pMyVehicle->AutoPilot.m_nCruiseSpeed = GAME_SPEED_TO_CARAI_SPEED * m_pMyVehicle->pHandling->Transmission.fMaxCruiseVelocity;
			m_pMyVehicle->SetStatus(STATUS_PHYSICS);
		}
	} else
#endif
	if (IsPedInControl() && (CharCreatedBy != MISSION_CHAR || bRespondsToThreats)) {
		CPed *ourLeader = m_leader;
		if (ourLeader != attacker && (!ourLeader || FindPlayerPed() != ourLeader)
			&& attacker->IsPed()) {

			CPed *attackerPed = (CPed*)attacker; 
			if (bNotAllowedToDuck) {
				if (!attackerPed->GetWeapon()->IsTypeMelee()) {
					m_duckAndCoverTimer = CTimer::GetTimeInMilliseconds();
					return;
				}
			} else if (bCrouchWhenShooting || bKindaStayInSamePlace) {
				SetDuck(CGeneral::GetRandomNumberInRange(1000, 3000));
				return;
			}

			if (m_pedStats->m_fear <= 100 - attackerPed->m_pedStats->m_temper) {
				if (m_pedStats != attackerPed->m_pedStats) {
					if (IsGangMember() || m_nPedType == PEDTYPE_EMERGENCY || m_nPedType == PEDTYPE_FIREMAN) {
						RegisterThreatWithGangPeds(attackerPed);
					}
					if (!attackerPed->GetWeapon()->IsTypeMelee() && GetWeapon()->IsTypeMelee()) {
						SetObjective(OBJECTIVE_FLEE_CHAR_ON_FOOT_TILL_SAFE, attacker);
						SetMoveState(PEDMOVE_RUN);
					} else {
						SetObjective(OBJECTIVE_KILL_CHAR_ON_FOOT, attacker);
						SetObjectiveTimer(20000);
					}
				}
			} else {
				SetObjective(OBJECTIVE_FLEE_CHAR_ON_FOOT_TILL_SAFE, attackerPed);
				SetMoveState(PEDMOVE_RUN);
				if (attackerPed->GetWeapon()->IsTypeMelee())
					Say(SOUND_PED_FLEE_RUN);
			}
		}
	}
}

void
CPed::PedAnimAlignCB(CAnimBlendAssociation *animAssoc, void *arg)
{
	CPed *ped = (CPed*)arg;

	CVehicle *veh = ped->m_pMyVehicle;
	if (animAssoc)
		animAssoc->blendDelta = -1000.0f;

	if (!ped->IsNotInWreckedVehicle())
		return;

	if (!ped->EnteringCar()) {
#ifdef VC_PED_PORTS
		if (ped->m_nPedState != PED_DRIVING)
#endif
			ped->QuitEnteringCar();

		return;
	}
	if (ped->m_fHealth == 0.0f) {
		ped->QuitEnteringCar();
		return;
	}
	bool itsVan = !!veh->bIsVan;
	bool itsBus = !!veh->bIsBus;
#ifdef FIX_BUGS
	bool itsLow = !!veh->bLowVehicle;
#endif
	eDoors enterDoor;
	AnimationId enterAnim;

	switch (ped->m_vehDoor) {
		case CAR_DOOR_RF:
			itsVan = false;
			enterDoor = DOOR_FRONT_RIGHT;
			break;
		case CAR_DOOR_RR:
			enterDoor = DOOR_REAR_RIGHT;
			break;
		case CAR_DOOR_LF:
			itsVan = false;
			enterDoor = DOOR_FRONT_LEFT;
			break;
		case CAR_DOOR_LR:
			enterDoor = DOOR_REAR_LEFT;
			break;
		default:
			break;
	}

	if (veh->IsDoorMissing(enterDoor) || veh->IsDoorFullyOpen(enterDoor)) {

		veh->AutoPilot.m_nCruiseSpeed = 0;
		if (ped->m_nPedState == PED_CARJACK) {
			ped->PedAnimDoorOpenCB(nil, ped);
			return;
		}
		if (enterDoor != DOOR_FRONT_LEFT && enterDoor != DOOR_REAR_LEFT) {
			if (itsVan) {
				enterAnim = ANIM_STD_VAN_GET_IN_REAR_RHS;
			} else if (itsBus) {
				enterAnim = ANIM_STD_COACH_GET_IN_RHS;
#ifdef FIX_BUGS
			} else if (itsLow) {
				enterAnim = ANIM_STD_CAR_GET_IN_LO_RHS;
#endif
			} else {
				enterAnim = ANIM_STD_CAR_GET_IN_RHS;
			}
		} else if (itsVan) {
			enterAnim = ANIM_STD_VAN_GET_IN_REAR_LHS;
		} else if (itsBus) {
			enterAnim = ANIM_STD_COACH_GET_IN_LHS;
#ifdef FIX_BUGS
		} else if (itsLow) {
			enterAnim = ANIM_STD_CAR_GET_IN_LO_LHS;
#endif
		} else {
			enterAnim = ANIM_STD_CAR_GET_IN_LHS;
		}
		ped->m_pVehicleAnim = CAnimManager::AddAnimation(ped->GetClump(), ASSOCGRP_STD, enterAnim);
		ped->m_pVehicleAnim->SetFinishCallback(PedAnimGetInCB, ped);

	} else if (veh->CanPedOpenLocks(ped)) {

		veh->AutoPilot.m_nCruiseSpeed = 0;
		if (enterDoor != DOOR_FRONT_LEFT && enterDoor != DOOR_REAR_LEFT) {
			if (itsVan) {
				ped->m_pVehicleAnim = CAnimManager::AddAnimation(ped->GetClump(), ASSOCGRP_STD, ANIM_STD_VAN_OPEN_DOOR_REAR_RHS);
			} else if (itsBus) {
				ped->m_pVehicleAnim = CAnimManager::AddAnimation(ped->GetClump(), ASSOCGRP_STD, ANIM_STD_COACH_OPEN_RHS);
			} else {
				ped->m_pVehicleAnim = CAnimManager::AddAnimation(ped->GetClump(), ASSOCGRP_STD, ANIM_STD_CAR_OPEN_DOOR_RHS);
			}
		} else if (itsVan) {
			ped->m_pVehicleAnim = CAnimManager::AddAnimation(ped->GetClump(), ASSOCGRP_STD, ANIM_STD_VAN_OPEN_DOOR_REAR_LHS);
		} else if (itsBus) {
			ped->m_pVehicleAnim = CAnimManager::AddAnimation(ped->GetClump(), ASSOCGRP_STD, ANIM_STD_COACH_OPEN_LHS);
		} else {

			if (ped->m_objective == OBJECTIVE_ENTER_CAR_AS_DRIVER && veh->pDriver) {

				if (!veh->bLowVehicle
					&& veh->pDriver->CharCreatedBy != MISSION_CHAR
					&& veh->pDriver->m_nPedState == PED_DRIVING) {

					ped->m_pVehicleAnim = CAnimManager::AddAnimation(ped->GetClump(), ASSOCGRP_STD, ANIM_STD_QUICKJACK);
					ped->m_pVehicleAnim->SetFinishCallback(PedAnimGetInCB, ped);
					veh->pDriver->SetBeingDraggedFromCar(veh, ped->m_vehDoor, true);

					if (veh->pDriver->IsGangMember())
						veh->pDriver->RegisterThreatWithGangPeds(ped);
					return;
				}
			}
			ped->m_pVehicleAnim = CAnimManager::AddAnimation(ped->GetClump(), ASSOCGRP_STD, ANIM_STD_CAR_OPEN_DOOR_LHS);
		}
		ped->m_pVehicleAnim->SetFinishCallback(PedAnimDoorOpenCB, ped);

	} else {
		if (enterDoor != DOOR_FRONT_LEFT && enterDoor != DOOR_REAR_LEFT)
			ped->m_pVehicleAnim = CAnimManager::AddAnimation(ped->GetClump(), ASSOCGRP_STD, ANIM_STD_CARDOOR_LOCKED_RHS);
		else
			ped->m_pVehicleAnim = CAnimManager::AddAnimation(ped->GetClump(), ASSOCGRP_STD, ANIM_STD_CARDOOR_LOCKED_LHS);

		ped->bCancelEnteringCar = true;
		ped->m_pVehicleAnim->SetFinishCallback(PedAnimDoorOpenCB, ped);
	}
}

void
CPed::PedAnimDoorOpenCB(CAnimBlendAssociation* animAssoc, void* arg)
{
	CPed* ped = (CPed*)arg;

	CVehicle* veh = ped->m_pMyVehicle;

	if (animAssoc)
		animAssoc->blendDelta = -1000.0f;

	if (!ped->IsNotInWreckedVehicle())
		return;

	if (!ped->EnteringCar()) {
#ifdef VC_PED_PORTS
		if (ped->m_nPedState != PED_DRIVING)
#endif
			ped->QuitEnteringCar();

		return;
	}

	eDoors door;
	CPed *pedInSeat = nil;
	switch (ped->m_vehDoor) {
		case CAR_DOOR_RF: door = DOOR_FRONT_RIGHT; pedInSeat = veh->pPassengers[0]; break;
		case CAR_DOOR_RR: door = DOOR_REAR_RIGHT; pedInSeat = veh->pPassengers[2]; break;
		case CAR_DOOR_LF: door = DOOR_FRONT_LEFT; pedInSeat = veh->pDriver; break;
		case CAR_DOOR_LR: door = DOOR_REAR_LEFT; pedInSeat = veh->pPassengers[1]; break;
		default: assert(0);
	}

	if (ped->m_fHealth == 0.0f || CPad::GetPad(0)->ArePlayerControlsDisabled() && pedInSeat && pedInSeat->IsPlayer()) {
		ped->QuitEnteringCar();
		return;
	}

	bool isVan = veh->bIsVan;
	bool isBus = veh->bIsBus;
	bool isLow = veh->bLowVehicle;
	bool vehUpsideDown = veh->IsUpsideDown();
	if (ped->bCancelEnteringCar) {
		if (ped->IsPlayer()) {
			if (veh->pDriver) {
				if (veh->pDriver->m_nPedType == PEDTYPE_COP) {
					FindPlayerPed()->SetWantedLevelNoDrop(1);
				}
			}
		}
#ifdef CANCELLABLE_CAR_ENTER
		if (!veh->IsDoorMissing(door) && veh->CanPedOpenLocks(ped) && veh->IsCar()) {
			((CAutomobile*)veh)->Damage.SetDoorStatus(door, DOOR_STATUS_SWINGING);
		}
#endif
		ped->QuitEnteringCar();
		ped->RestorePreviousObjective();
		ped->bCancelEnteringCar = false;
		return;
	}
	if (!veh->IsDoorMissing(door) && veh->IsCar()) {
		((CAutomobile*)veh)->Damage.SetDoorStatus(door, DOOR_STATUS_SWINGING);
	}

	if (veh->m_vecMoveSpeed.Magnitude() > 0.2f) {
		ped->QuitEnteringCar();
		if (ped->m_vehDoor != CAR_DOOR_LF && ped->m_vehDoor != CAR_DOOR_LR)
			ped->SetFall(1000, ANIM_STD_HIGHIMPACT_LEFT, false);
		else
			ped->SetFall(1000, ANIM_STD_HIGHIMPACT_RIGHT, false);
		
		return;
	}
	veh->ProcessOpenDoor(ped->m_vehDoor, ANIM_STD_CAR_OPEN_DOOR_LHS, 1.0f);

	if (ped->m_vehDoor == CAR_DOOR_LF || ped->m_vehDoor == CAR_DOOR_RF)
		isVan = false;

	if (ped->m_nPedState != PED_CARJACK || isBus) {
		AnimationId animToPlay;
		if (ped->m_vehDoor != CAR_DOOR_LF && ped->m_vehDoor != CAR_DOOR_LR) {

			if (isVan) {
				animToPlay = ANIM_STD_VAN_GET_IN_REAR_RHS;
			} else if (isBus) {
				animToPlay = ANIM_STD_COACH_GET_IN_RHS;
			} else if (isLow) {
				animToPlay = ANIM_STD_CAR_GET_IN_LO_RHS;
			} else {
				animToPlay = ANIM_STD_CAR_GET_IN_RHS;
			}
		} else if (isVan) {
			animToPlay = ANIM_STD_VAN_GET_IN_REAR_LHS;
		} else if (isBus) {
			animToPlay = ANIM_STD_COACH_GET_IN_LHS;
		} else if (isLow) {
			animToPlay = ANIM_STD_CAR_GET_IN_LO_LHS;
		} else {
			animToPlay = ANIM_STD_CAR_GET_IN_LHS;
		}
		ped->m_pVehicleAnim = CAnimManager::AddAnimation(ped->GetClump(), ASSOCGRP_STD, animToPlay);
		ped->m_pVehicleAnim->SetFinishCallback(PedAnimGetInCB, ped);
	} else {
		CPed *pedToDragOut = nil;
		switch (ped->m_vehDoor) {
			case CAR_DOOR_RF: pedToDragOut = veh->pPassengers[0]; break;
			case CAR_DOOR_RR: pedToDragOut = veh->pPassengers[2]; break;
			case CAR_DOOR_LF: pedToDragOut = veh->pDriver; break;
			case CAR_DOOR_LR: pedToDragOut = veh->pPassengers[1]; break;
			default: assert(0);
		}

		if (vehUpsideDown) {
			ped->QuitEnteringCar();
			if (ped->m_nPedType == PEDTYPE_COP)
				((CCopPed*)ped)->SetArrestPlayer(ped->m_pedInObjective);
		}

		if (ped->m_vehDoor != CAR_DOOR_LF && ped->m_vehDoor != CAR_DOOR_LR) {
			if (pedToDragOut && !pedToDragOut->bDontDragMeOutCar) {
				if (pedToDragOut->m_nPedState != PED_DRIVING) {
					ped->QuitEnteringCar();
					pedToDragOut = nil;
				} else {
					if (isLow)
						ped->m_pVehicleAnim = CAnimManager::AddAnimation(ped->GetClump(), ASSOCGRP_STD, ANIM_STD_CAR_PULL_OUT_PED_LO_RHS);
					else
						ped->m_pVehicleAnim = CAnimManager::AddAnimation(ped->GetClump(), ASSOCGRP_STD, ANIM_STD_CAR_PULL_OUT_PED_RHS);

					ped->m_pVehicleAnim->SetFinishCallback(PedAnimPullPedOutCB, ped);
				}
			} else if (ped->m_nPedType == PEDTYPE_COP) {
				ped->QuitEnteringCar();
				if (ped->m_pedInObjective && ped->m_pedInObjective->m_nPedState == PED_DRIVING) {
					veh->SetStatus(STATUS_PLAYER_DISABLED);
					((CCopPed*)ped)->SetArrestPlayer(ped->m_pedInObjective);
				} else if (!veh->IsDoorMissing(DOOR_FRONT_RIGHT)) {
					((CAutomobile*)veh)->Damage.SetDoorStatus(DOOR_FRONT_RIGHT, DOOR_STATUS_SWINGING);
				}
			} else {
				// BUG: Probably we will sit on top of the passenger if his m_ped_flagF4 is true.
				if (isLow)
					ped->m_pVehicleAnim = CAnimManager::AddAnimation(ped->GetClump(), ASSOCGRP_STD, ANIM_STD_CAR_GET_IN_LO_LHS);
				else
					ped->m_pVehicleAnim = CAnimManager::AddAnimation(ped->GetClump(), ASSOCGRP_STD, ANIM_STD_CAR_GET_IN_LHS);

				ped->m_pVehicleAnim->SetFinishCallback(PedAnimGetInCB, ped);
			}
		} else {
			if (pedToDragOut) {
				if (pedToDragOut->m_nPedState != PED_DRIVING || pedToDragOut->bDontDragMeOutCar) {

					// BUG: Player freezes in that condition due to its objective isn't restored. It's an unfinished feature, used in VC.
					ped->QuitEnteringCar();
					pedToDragOut = nil;
				} else {
					if (isLow)
						ped->m_pVehicleAnim = CAnimManager::AddAnimation(ped->GetClump(), ASSOCGRP_STD, ANIM_STD_CAR_PULL_OUT_PED_LO_LHS);
					else
						ped->m_pVehicleAnim = CAnimManager::AddAnimation(ped->GetClump(), ASSOCGRP_STD, ANIM_STD_CAR_PULL_OUT_PED_LHS);
					
					ped->m_pVehicleAnim->SetFinishCallback(PedAnimPullPedOutCB, ped);
				}
			} else {
				if (isLow)
					ped->m_pVehicleAnim = CAnimManager::AddAnimation(ped->GetClump(), ASSOCGRP_STD, ANIM_STD_CAR_GET_IN_LO_LHS);
				else
					ped->m_pVehicleAnim = CAnimManager::AddAnimation(ped->GetClump(), ASSOCGRP_STD, ANIM_STD_CAR_GET_IN_LHS);

				ped->m_pVehicleAnim->SetFinishCallback(PedAnimGetInCB, ped);
			}
		}

		if (pedToDragOut) {
			pedToDragOut->SetBeingDraggedFromCar(veh, ped->m_vehDoor, false);
			if (pedToDragOut->IsGangMember())
				pedToDragOut->RegisterThreatWithGangPeds(ped);
		}
	}

	if (veh->pDriver && ped) {
		veh->pDriver->SetLookFlag(ped, true);
		veh->pDriver->SetLookTimer(1000);
	}
	return;
}

void
CPed::PedAnimPullPedOutCB(CAnimBlendAssociation* animAssoc, void* arg)
{
	CPed* ped = (CPed*)arg;

	CVehicle* veh = ped->m_pMyVehicle;
	if (animAssoc)
		animAssoc->blendDelta = -1000.0f;

	if (ped->EnteringCar()) {
		if (!ped->IsNotInWreckedVehicle())
			return;

#ifdef CANCELLABLE_CAR_ENTER
		if (ped->bCancelEnteringCar) {
			ped->QuitEnteringCar();
			ped->RestorePreviousObjective();
			ped->bCancelEnteringCar = false;
			return;
		}
#endif

		bool isLow = !!veh->bLowVehicle;

		int padNo;
		if (ped->IsPlayer()) {

			// BUG? This will cause crash if m_nPedType is bigger then 1, there are only 2 pads
			switch (ped->m_nPedType) {
				case PEDTYPE_PLAYER1:
					padNo = 0;
					break;
				case PEDTYPE_PLAYER2:
					padNo = 1;
					break;
				case PEDTYPE_PLAYER3:
					padNo = 2;
					break;
				case PEDTYPE_PLAYER4:
					padNo = 3;
					break;
			}
			CPad *pad = CPad::GetPad(padNo);

			if (!pad->ArePlayerControlsDisabled()) {

				if (pad->GetTarget()
					|| pad->NewState.LeftStickX
					|| pad->NewState.LeftStickY
					|| pad->NewState.DPadUp
					|| pad->NewState.DPadDown
					|| pad->NewState.DPadLeft
					|| pad->NewState.DPadRight) {
					ped->QuitEnteringCar();
					ped->RestorePreviousObjective();
					return;
				}
			}
		}

		if (ped->m_objective == OBJECTIVE_ENTER_CAR_AS_DRIVER) {
			AnimationId animToPlay;
			if (ped->m_vehDoor != CAR_DOOR_LF && ped->m_vehDoor != CAR_DOOR_LR) {
				if (isLow)
					animToPlay = ANIM_STD_CAR_GET_IN_LO_RHS;
				else
					animToPlay = ANIM_STD_CAR_GET_IN_RHS;
			} else if (isLow) {
				animToPlay = ANIM_STD_CAR_GET_IN_LO_LHS;
			} else {
				animToPlay = ANIM_STD_CAR_GET_IN_LHS;
			}
			ped->m_pVehicleAnim = CAnimManager::AddAnimation(ped->GetClump(), ASSOCGRP_STD, animToPlay);
			ped->m_pVehicleAnim->SetFinishCallback(PedAnimGetInCB, ped);
		} else {
			ped->QuitEnteringCar();
		}
	} else {
		ped->QuitEnteringCar();
	}
}

void
CPed::PedAnimGetInCB(CAnimBlendAssociation *animAssoc, void *arg)
{
	CPed *ped = (CPed*) arg;

	CVehicle *veh = ped->m_pMyVehicle;
	if (animAssoc)
		animAssoc->blendDelta = -1000.0f;

	if (!ped->IsNotInWreckedVehicle() || ped->DyingOrDead())
		return;

	if (!ped->EnteringCar()) {
#ifdef VC_PED_PORTS
		if(ped->m_nPedState != PED_DRIVING)
#endif
			ped->QuitEnteringCar();
		return;
	}

	if (ped->IsPlayer() && ped->bGonnaKillTheCarJacker && ((CPlayerPed*)ped)->m_pArrestingCop) {
		PedSetInCarCB(nil, ped);
		ped->m_nLastPedState = ped->m_nPedState;
		ped->SetPedState(PED_ARRESTED);
		ped->bGonnaKillTheCarJacker = false;
		if (veh) {
			veh->m_nNumGettingIn = 0;
			veh->m_nGettingInFlags = 0;	
			veh->bIsHandbrakeOn = true;
			veh->SetStatus(STATUS_PLAYER_DISABLED);
		}
		return;
	}
	if (ped->IsPlayer() && ped->m_vehDoor == CAR_DOOR_LF
		&& (Pads[0].GetAccelerate() >= 255.0f || Pads[0].GetBrake() >= 255.0f)
		&& veh->IsCar()) {
		if (((CAutomobile*)veh)->Damage.GetDoorStatus(DOOR_FRONT_LEFT) != DOOR_STATUS_MISSING)
			((CAutomobile*)veh)->Damage.SetDoorStatus(DOOR_FRONT_LEFT, DOOR_STATUS_SWINGING);

		PedSetInCarCB(nil, ped);
		return;
	}
	bool isVan = !!veh->bIsVan;
	bool isBus = !!veh->bIsBus;
	bool isLow = !!veh->bLowVehicle;
	eDoors enterDoor;
	switch (ped->m_vehDoor) {
		case CAR_DOOR_RF:
			isVan = false;
			enterDoor = DOOR_FRONT_RIGHT;
			break;
		case CAR_DOOR_RR:
			enterDoor = DOOR_REAR_RIGHT;
			break;
		case CAR_DOOR_LF:
			isVan = false;
			enterDoor = DOOR_FRONT_LEFT;
			break;
		case CAR_DOOR_LR:
			enterDoor = DOOR_REAR_LEFT;
			break;
		default:
			break;
	}
	if (!veh->IsDoorMissing(enterDoor)) {
		if (veh->IsCar())
			((CAutomobile*)veh)->Damage.SetDoorStatus(enterDoor, DOOR_STATUS_SWINGING);
	}
	CPed *driver = veh->pDriver;
	if (driver && (ped->m_objective == OBJECTIVE_ENTER_CAR_AS_DRIVER || ped->m_nPedState == PED_CARJACK)) {
		if (veh->bIsBus) {
			driver->SetObjective(OBJECTIVE_LEAVE_CAR, veh);
			if (driver->IsPlayer()) {
				veh->bIsHandbrakeOn = true;
				veh->SetStatus(STATUS_PLAYER_DISABLED);
			}
			driver->bBusJacked = true;
			veh->bIsBeingCarJacked = false;
			PedSetInCarCB(nil, ped);
			if (ped->m_nPedType == PEDTYPE_COP
				|| ped->m_objective == OBJECTIVE_KILL_CHAR_ON_FOOT
				|| ped->m_objective == OBJECTIVE_KILL_CHAR_ANY_MEANS) {
				ped->SetObjective(OBJECTIVE_LEAVE_CAR, veh);
			}
			ped->m_leaveCarTimer = CTimer::GetTimeInMilliseconds() + 400;
			return;
		}
		if (driver != ped && ped->m_vehDoor != CAR_DOOR_LF) {
			if (!driver->IsPlayer()) {
				driver->bUsePedNodeSeek = true;
				driver->m_pLastPathNode = nil;
				if (driver->m_pedStats->m_temper <= driver->m_pedStats->m_fear
					|| driver->CharCreatedBy == MISSION_CHAR
					|| veh->VehicleCreatedBy == MISSION_VEHICLE) {
					driver->bFleeAfterExitingCar = true;
				} else {
					driver->bGonnaKillTheCarJacker = true;
					veh->pDriver->SetObjective(OBJECTIVE_KILL_CHAR_ON_FOOT, ped);

					if (veh->pDriver->m_nPedType == PEDTYPE_COP && ped->IsPlayer()) {
						FindPlayerPed()->SetWantedLevelNoDrop(1);
					}
				}
			}
			if ((ped->m_nPedType != PEDTYPE_EMERGENCY || veh->pDriver->m_nPedType != PEDTYPE_EMERGENCY)
				&& (ped->m_nPedType != PEDTYPE_COP || veh->pDriver->m_nPedType != PEDTYPE_COP)) {
				veh->pDriver->SetObjective(OBJECTIVE_LEAVE_CAR, veh);
				veh->pDriver->Say(SOUND_PED_CAR_JACKED);
#ifdef VC_PED_PORTS
				veh->pDriver->SetRadioStation();
#endif
			} else {
				ped->m_objective = OBJECTIVE_ENTER_CAR_AS_PASSENGER;
			}
		}
	}
	if (veh->IsDoorMissing(enterDoor) || isBus) {
		PedAnimDoorCloseCB(nil, ped);
	} else {
		AnimationId animToPlay;
		if (enterDoor != DOOR_FRONT_LEFT && enterDoor != DOOR_REAR_LEFT) {
			if (isVan) {
				animToPlay = ANIM_STD_VAN_CLOSE_DOOR_REAR_RHS;
			} else if (isLow) {
				animToPlay = ANIM_STD_CAR_CLOSE_DOOR_LO_RHS;
			} else {
				animToPlay = ANIM_STD_CAR_CLOSE_DOOR_RHS;
			}
		} else if (isVan) {
			animToPlay = ANIM_STD_VAN_CLOSE_DOOR_REAR_LHS;
		} else if (isLow) {
			animToPlay = ANIM_STD_CAR_CLOSE_DOOR_LO_LHS;
		} else {
			animToPlay = ANIM_STD_CAR_CLOSE_DOOR_LHS;
		}
		ped->m_pVehicleAnim = CAnimManager::AddAnimation(ped->GetClump(), ASSOCGRP_STD, animToPlay);
		ped->m_pVehicleAnim->SetFinishCallback(PedAnimDoorCloseCB, ped);
	}
}

void
CPed::PedAnimDoorCloseCB(CAnimBlendAssociation *animAssoc, void *arg)
{
	CPed *ped = (CPed*)arg;

	CAutomobile *veh = (CAutomobile*)(ped->m_pMyVehicle);
	
	if (!ped->IsNotInWreckedVehicle() || ped->DyingOrDead())
		return;

	if (ped->EnteringCar()) {
		bool isLow = !!veh->bLowVehicle;

		if (!veh->bIsBus)
			veh->ProcessOpenDoor(ped->m_vehDoor, ANIM_STD_CAR_CLOSE_DOOR_LHS, 1.0f);

		eDoors door;
		switch (ped->m_vehDoor) {
			case CAR_DOOR_RF: door = DOOR_FRONT_RIGHT; break;
			case CAR_DOOR_RR: door = DOOR_REAR_RIGHT; break;
			case CAR_DOOR_LF: door = DOOR_FRONT_LEFT; break;
			case CAR_DOOR_LR: door = DOOR_REAR_LEFT; break;
			default: assert(0);
		}

		if (veh->Damage.GetDoorStatus(door) == DOOR_STATUS_SWINGING)
			veh->Damage.SetDoorStatus(door, DOOR_STATUS_OK);

		if (door == DOOR_FRONT_LEFT || ped->m_objective == OBJECTIVE_ENTER_CAR_AS_PASSENGER || veh->bIsBus) {
			PedSetInCarCB(nil, ped);
		} else if (ped->m_vehDoor == CAR_DOOR_RF
				&& (veh->m_nGettingInFlags & CAR_DOOR_FLAG_LF ||
					(veh->pDriver != nil && 
						(veh->pDriver->m_objective != OBJECTIVE_LEAVE_CAR
#ifdef VC_PED_PORTS
							&& veh->pDriver->m_objective != OBJECTIVE_LEAVE_CAR_AND_DIE
#endif
							|| !veh->IsRoomForPedToLeaveCar(CAR_DOOR_LF, nil))))) {

			if (ped->m_objective == OBJECTIVE_ENTER_CAR_AS_DRIVER
#if defined VC_PED_PORTS || defined FIX_BUGS
				|| ped->m_nPedState == PED_CARJACK
#endif
				)
				veh->bIsBeingCarJacked = false;

			ped->m_objective = OBJECTIVE_ENTER_CAR_AS_PASSENGER;
			PedSetInCarCB(nil, ped);

			ped->SetObjective(OBJECTIVE_LEAVE_CAR, veh);
			if (!ped->IsPlayer())
				ped->bFleeAfterExitingCar = true;

			ped->bUsePedNodeSeek = true;
			ped->m_pNextPathNode = nil;

		} else {
			if (animAssoc)
				animAssoc->blendDelta = -1000.0f;

			if (isLow)
				ped->m_pVehicleAnim = CAnimManager::AddAnimation(ped->GetClump(), ASSOCGRP_STD, ANIM_STD_CAR_SHUFFLE_LO_RHS);
			else
				ped->m_pVehicleAnim = CAnimManager::AddAnimation(ped->GetClump(), ASSOCGRP_STD, ANIM_STD_CAR_SHUFFLE_RHS);

			ped->m_pVehicleAnim->SetFinishCallback(PedSetInCarCB, ped);
		}
	} else {
#ifdef VC_PED_PORTS
		if (ped->m_nPedState != PED_DRIVING)
#endif
			ped->QuitEnteringCar();
	}
}

void
CPed::SetFormation(eFormation type)
{
	// FIX: Formations in GetFormationPosition were in range 1-8, whereas in here it's 0-7.
	//      To not change the behaviour, range in here tweaked by 1 with the use of enum.

	switch (m_pedFormation) {
		case FORMATION_REAR:
		case FORMATION_REAR_LEFT:
		case FORMATION_REAR_RIGHT:
		case FORMATION_FRONT_LEFT:
		case FORMATION_FRONT_RIGHT:
		case FORMATION_LEFT:
		case FORMATION_RIGHT:
		case FORMATION_FRONT:
			break;
		default:
			Error("Unknown formation type, PedAI.cpp");
			break;
	}
	m_pedFormation = type;
}

CVector
CPed::GetFormationPosition(void)
{
	if (m_pedInObjective->m_nPedState == PED_DEAD) {
		if (!m_pedInObjective->m_pedInObjective) {
			m_pedInObjective = nil;
			return GetPosition();
		}
		m_pedInObjective = m_pedInObjective->m_pedInObjective;
	}

	CVector formationOffset;
	switch (m_pedFormation) {
		case FORMATION_REAR:
			formationOffset = CVector(0.0f, -1.5f, 0.0f);
			break;
		case FORMATION_REAR_LEFT:
			formationOffset = CVector(-1.5f, -1.5f, 0.0f);
			break;
		case FORMATION_REAR_RIGHT:
			formationOffset = CVector(1.5f, -1.5f, 0.0f);
			break;
		case FORMATION_FRONT_LEFT:
			formationOffset = CVector(-1.5f, 1.5f, 0.0f);
			break;
		case FORMATION_FRONT_RIGHT:
			formationOffset = CVector(1.5f, 1.5f, 0.0f);
			break;
		case FORMATION_LEFT:
			formationOffset = CVector(-1.5f, 0.0f, 0.0f);
			break;
		case FORMATION_RIGHT:
			formationOffset = CVector(1.5f, 0.0f, 0.0f);
			break;
		case FORMATION_FRONT:
			formationOffset = CVector(0.0f, 1.5f, 0.0f);
			break;
		default:
			formationOffset = CVector(0.0f, 0.0f, 0.0f);
			break;
	}
	return formationOffset + m_pedInObjective->GetPosition();
}

void
CPed::PedAnimStepOutCarCB(CAnimBlendAssociation* animAssoc, void* arg)
{
	CPed* ped = (CPed*)arg;

	CVehicle* veh = ped->m_pMyVehicle;
	if (animAssoc)
		animAssoc->blendDelta = -1000.0f;

	if (!veh) {
		PedSetOutCarCB(nil, ped);
		return;
	}
#ifdef VC_PED_PORTS
	CVector posForZ = ped->GetPosition();
	CPedPlacement::FindZCoorForPed(&posForZ);
	if (ped->GetPosition().z - 0.5f > posForZ.z) {
		PedSetOutCarCB(nil, ped);
		return;
	}
#endif
	veh->m_nStaticFrames = 0;
	veh->m_vecMoveSpeed += CVector(0.001f, 0.001f, 0.001f);
	veh->m_vecTurnSpeed += CVector(0.001f, 0.001f, 0.001f);
	if (!veh->bIsBus)
		veh->ProcessOpenDoor(ped->m_vehDoor, ANIM_STD_GETOUT_LHS, 1.0f);

	/* 
	// Duplicate and only in PC for some reason
	if (!veh) {
		PedSetOutCarCB(nil, ped);
		return;
	}
	*/
	eDoors door;
	switch (ped->m_vehDoor) {
		case CAR_DOOR_RF:
			door = DOOR_FRONT_RIGHT;
			break;
		case CAR_DOOR_RR:
			door = DOOR_REAR_RIGHT;
			break;
		case CAR_DOOR_LF:
			door = DOOR_FRONT_LEFT;
			break;
		case CAR_DOOR_LR:
			door = DOOR_REAR_LEFT;
			break;
		default:
			break;
	}
	bool closeDoor = !veh->IsDoorMissing(door);

	int padNo;
	if (ped->IsPlayer()) {

		// BUG? This will cause crash if m_nPedType is bigger then 1, there are only 2 pads
		switch (ped->m_nPedType) {
			case PEDTYPE_PLAYER1:
				padNo = 0;
				break;
			case PEDTYPE_PLAYER2:
				padNo = 1;
				break;
			case PEDTYPE_PLAYER3:
				padNo = 2;
				break;
			case PEDTYPE_PLAYER4:
				padNo = 3;
				break;
		}
		CPad* pad = CPad::GetPad(padNo);
		bool engineIsIntact = veh->IsCar() && ((CAutomobile*)veh)->Damage.GetEngineStatus() >= 225;
		if (!pad->ArePlayerControlsDisabled() && veh->m_nDoorLock != CARLOCK_FORCE_SHUT_DOORS
			&& (pad->GetTarget()
				|| pad->NewState.LeftStickX
				|| pad->NewState.LeftStickY
				|| pad->NewState.DPadUp
				|| pad->NewState.DPadDown
				|| pad->NewState.DPadLeft
				|| pad->NewState.DPadRight)
			|| veh->bIsBus
			|| veh->m_pCarFire
			|| engineIsIntact) {
			closeDoor = false;
		}
	}

#ifdef VC_PED_PORTS
	if (ped->m_objective == OBJECTIVE_LEAVE_CAR_AND_DIE)
		closeDoor = false;
#endif

	if (!closeDoor) {
		if (!veh->IsDoorMissing(door) && !veh->bIsBus) {
			((CAutomobile*)veh)->Damage.SetDoorStatus(door, DOOR_STATUS_SWINGING);
		}
		PedSetOutCarCB(nil, ped);
		return;
	}

	if (ped->bFleeAfterExitingCar || ped->bGonnaKillTheCarJacker) {
#ifdef FIX_BUGS
		if (!veh->IsDoorMissing(door))
			((CAutomobile*)veh)->Damage.SetDoorStatus(door, DOOR_STATUS_SWINGING);
		PedSetOutCarCB(nil, ped);
		return;
#else
		if (!veh->IsDoorMissing(door))
			((CAutomobile*)veh)->Damage.SetDoorStatus(DOOR_FRONT_LEFT, DOOR_STATUS_SWINGING);
#endif
	} else {
		switch (door) {
			case DOOR_FRONT_LEFT:
				ped->m_pVehicleAnim = CAnimManager::AddAnimation(ped->GetClump(), ASSOCGRP_STD, ANIM_STD_CAR_CLOSE_LHS);
				break;
			case DOOR_FRONT_RIGHT:
				ped->m_pVehicleAnim = CAnimManager::AddAnimation(ped->GetClump(), ASSOCGRP_STD, ANIM_STD_CAR_CLOSE_RHS);
				break;
			case DOOR_REAR_LEFT:
				ped->m_pVehicleAnim = CAnimManager::AddAnimation(ped->GetClump(), ASSOCGRP_STD, ANIM_STD_CAR_CLOSE_LHS);
				break;
			case DOOR_REAR_RIGHT:
				ped->m_pVehicleAnim = CAnimManager::AddAnimation(ped->GetClump(), ASSOCGRP_STD, ANIM_STD_CAR_CLOSE_RHS);
				break;
			default:
				break;
		}
	}

	if (ped->m_pVehicleAnim)
		ped->m_pVehicleAnim->SetFinishCallback(PedSetOutCarCB, ped);
	return;
}

void
CPed::LineUpPedWithCar(PedLineUpPhase phase)
{
	bool vehIsUpsideDown = false;
	bool stillGettingInOut = false;
	int vehAnim;
	float seatPosMult = 0.0f;
	float currentZ;
	float adjustedTimeStep;

	if (CReplay::IsPlayingBack())
		return;

	if (!bChangedSeat && phase != LINE_UP_TO_CAR_2) {
		if (RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_CAR_SIT)) {
			SetPedPositionInCar();
			return;
		}
		if (RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_CAR_SIT_LO)) {
			SetPedPositionInCar();
			return;
		}
		if (RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_CAR_SIT_P)) {
			SetPedPositionInCar();
			return;
		}
		if (RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_CAR_SIT_P_LO)) {
			SetPedPositionInCar();
			return;
		}
		bChangedSeat = true;
	}
	if (phase == LINE_UP_TO_CAR_START) {
		m_vecMoveSpeed = CVector(0.0f, 0.0f, 0.0f);
	}
	CVehicle *veh = m_pMyVehicle;

	// Not quite right, IsUpsideDown func. checks for <= -0.9f.
	if (veh->GetUp().z <= -0.8f)
		vehIsUpsideDown = true;

	if (m_vehDoor == CAR_DOOR_RF || m_vehDoor == CAR_DOOR_RR) {
		if (vehIsUpsideDown) {
			m_fRotationDest = -PI + veh->GetForward().Heading();
		} else if (veh->bIsBus) {
			m_fRotationDest = 0.5f * PI + veh->GetForward().Heading();
		} else {
			m_fRotationDest = veh->GetForward().Heading();
		}
	} else if (m_vehDoor == CAR_DOOR_LF || m_vehDoor == CAR_DOOR_LR) {
		if (vehIsUpsideDown) {
			m_fRotationDest = veh->GetForward().Heading();
		} else if (veh->bIsBus) {
			m_fRotationDest = -0.5f * PI + veh->GetForward().Heading();
		} else {
			m_fRotationDest = veh->GetForward().Heading();
		}
	} else {
		// I don't know will this part ever run(maybe boats?), but the game also handles that. I don't know is it intentional.

		if (vehIsUpsideDown) {
			m_fRotationDest = veh->GetForward().Heading();
		} else if (veh->bIsBus) {
			m_fRotationDest = 0.5f * PI + veh->GetForward().Heading();
		} else {
			m_fRotationDest = veh->GetForward().Heading();
		}
	}

	if (!bInVehicle)
		seatPosMult = 1.0f;

#ifdef VC_PED_PORTS
	bool multExtractedFromAnim = false;
	bool multExtractedFromAnimBus = false;
	float zBlend;
#endif
	if (m_pVehicleAnim) {
		vehAnim = m_pVehicleAnim->animId;

		switch (vehAnim) {
			case ANIM_STD_JACKEDCAR_RHS:
			case ANIM_STD_JACKEDCAR_LO_RHS:
			case ANIM_STD_JACKEDCAR_LHS:
			case ANIM_STD_JACKEDCAR_LO_LHS:
			case ANIM_STD_VAN_GET_IN_REAR_LHS:
			case ANIM_STD_VAN_GET_IN_REAR_RHS:
#ifdef VC_PED_PORTS
				multExtractedFromAnim = true;
				zBlend = Max(m_pVehicleAnim->currentTime / m_pVehicleAnim->hierarchy->totalLength - 0.3f, 0.0f) / (1.0f - 0.3f);
				// fall through
#endif
			case ANIM_STD_QUICKJACKED:
			case ANIM_STD_GETOUT_LHS:
			case ANIM_STD_GETOUT_LO_LHS:
			case ANIM_STD_GETOUT_RHS:
			case ANIM_STD_GETOUT_LO_RHS:
#ifdef VC_PED_PORTS
				if (!multExtractedFromAnim) {
					multExtractedFromAnim = true;
					zBlend = Max(m_pVehicleAnim->currentTime / m_pVehicleAnim->hierarchy->totalLength - 0.5f, 0.0f) / (1.0f - 0.5f);
				}
				// fall through
#endif
			case ANIM_STD_CRAWLOUT_LHS:
			case ANIM_STD_CRAWLOUT_RHS:
			case ANIM_STD_VAN_GET_OUT_REAR_LHS:
			case ANIM_STD_VAN_GET_OUT_REAR_RHS:
				seatPosMult = m_pVehicleAnim->currentTime / m_pVehicleAnim->hierarchy->totalLength;
				break;
			case ANIM_STD_CAR_GET_IN_RHS:
			case ANIM_STD_CAR_GET_IN_LHS:
#ifdef VC_PED_PORTS
				if (veh && veh->IsCar() && veh->bIsBus) {
					multExtractedFromAnimBus = true;
					zBlend = Min(m_pVehicleAnim->currentTime / m_pVehicleAnim->hierarchy->totalLength, 0.5f) / 0.5f;
				}
				// fall through
#endif
			case ANIM_STD_QUICKJACK:
			case ANIM_STD_CAR_GET_IN_LO_LHS:
			case ANIM_STD_CAR_GET_IN_LO_RHS:
			case ANIM_STD_BOAT_DRIVE:
				seatPosMult = m_pVehicleAnim->GetTimeLeft() / m_pVehicleAnim->hierarchy->totalLength;
				break;
			case ANIM_STD_CAR_CLOSE_DOOR_LHS:
			case ANIM_STD_CAR_CLOSE_DOOR_LO_LHS:
			case ANIM_STD_CAR_CLOSE_DOOR_RHS:
			case ANIM_STD_CAR_CLOSE_DOOR_LO_RHS:
			case ANIM_STD_CAR_SHUFFLE_RHS:
			case ANIM_STD_CAR_SHUFFLE_LO_RHS:
				seatPosMult = 0.0f;
				break;
			case ANIM_STD_CAR_CLOSE_LHS:
			case ANIM_STD_CAR_CLOSE_RHS:
			case ANIM_STD_COACH_OPEN_LHS:
			case ANIM_STD_COACH_OPEN_RHS:
			case ANIM_STD_COACH_GET_IN_LHS:
			case ANIM_STD_COACH_GET_IN_RHS:
			case ANIM_STD_COACH_GET_OUT_LHS:
				seatPosMult = 1.0f;
				break;
			default:
				break;
		}
	}

	CVector neededPos;

	if (phase == LINE_UP_TO_CAR_2) {
		neededPos = GetPosition();
	} else {
		neededPos = GetPositionToOpenCarDoor(veh, m_vehDoor, seatPosMult);
	}

	CVector autoZPos = neededPos;

	if (veh->bIsInWater) {
		if (veh->m_vehType == VEHICLE_TYPE_BOAT && veh->IsUpsideDown())
			autoZPos.z += 1.0f;
	} else {
		CPedPlacement::FindZCoorForPed(&autoZPos);
	}

	if (phase == LINE_UP_TO_CAR_END || phase == LINE_UP_TO_CAR_2) {
		neededPos.z = GetPosition().z;

		// Getting out
		if (!veh->bIsBus || (veh->bIsBus && vehIsUpsideDown)) {
			float nextZSpeed = m_vecMoveSpeed.z - GRAVITY * CTimer::GetTimeStep();

			// If we're not in ground at next step, apply animation
			if (neededPos.z + nextZSpeed >= autoZPos.z) {
				m_vecMoveSpeed.z = nextZSpeed;
				ApplyMoveSpeed();
				// Removing below line breaks the animation
				neededPos.z = GetPosition().z;
			} else {
				neededPos.z = autoZPos.z;
				m_vecMoveSpeed = CVector(0.0f, 0.0f, 0.0f);
			}
		}
	}

	if (autoZPos.z > neededPos.z) {
#ifdef VC_PED_PORTS
		if (multExtractedFromAnim) {
			neededPos.z += (autoZPos.z - neededPos.z) * zBlend;
		} else {
#endif
			currentZ = GetPosition().z;
			if (m_pVehicleAnim && vehAnim != ANIM_STD_VAN_GET_IN_REAR_LHS && vehAnim != ANIM_STD_VAN_CLOSE_DOOR_REAR_LHS && vehAnim != ANIM_STD_VAN_CLOSE_DOOR_REAR_RHS && vehAnim != ANIM_STD_VAN_GET_IN_REAR_RHS) {
				neededPos.z = autoZPos.z;
				m_vecMoveSpeed = CVector(0.0f, 0.0f, 0.0f);
			} else if (neededPos.z < currentZ && m_pVehicleAnim && vehAnim != ANIM_STD_VAN_CLOSE_DOOR_REAR_LHS && vehAnim != ANIM_STD_VAN_CLOSE_DOOR_REAR_RHS) {
				adjustedTimeStep = Max(m_pVehicleAnim->timeStep, 0.1f);

				// Smoothly change ped position
				neededPos.z = currentZ - (currentZ - neededPos.z) / (m_pVehicleAnim->GetTimeLeft() / adjustedTimeStep);
			}
#ifdef VC_PED_PORTS
		}
#endif
	} else {
		// We may need to raise up the ped
		if (phase == LINE_UP_TO_CAR_START) {
			currentZ = GetPosition().z;

			if (neededPos.z > currentZ) {
#ifdef VC_PED_PORTS
				if (multExtractedFromAnimBus) {
					neededPos.z = (neededPos.z - currentZ) * zBlend + currentZ;
				} else {
#endif
					if (m_pVehicleAnim &&
						(vehAnim == ANIM_STD_CAR_GET_IN_RHS || vehAnim == ANIM_STD_CAR_GET_IN_LO_RHS || vehAnim == ANIM_STD_CAR_GET_IN_LHS || vehAnim == ANIM_STD_CAR_GET_IN_LO_LHS
							|| vehAnim == ANIM_STD_QUICKJACK || vehAnim == ANIM_STD_VAN_GET_IN_REAR_LHS || vehAnim == ANIM_STD_VAN_GET_IN_REAR_RHS)) {
						adjustedTimeStep = Max(m_pVehicleAnim->timeStep, 0.1f);

						// Smoothly change ped position
						neededPos.z = (neededPos.z - currentZ) / (m_pVehicleAnim->GetTimeLeft() / adjustedTimeStep) + currentZ;
					} else if (EnteringCar()) {
						neededPos.z = Max(currentZ, autoZPos.z);
					}
#ifdef VC_PED_PORTS
				}
#endif
			}
		}
	}

	if (CTimer::GetTimeInMilliseconds() < m_nPedStateTimer)
		stillGettingInOut = veh->m_vehType != VEHICLE_TYPE_BOAT || bOnBoat;

	if (!stillGettingInOut) {
		m_fRotationCur = m_fRotationDest;
	} else {
		float limitedDest = CGeneral::LimitRadianAngle(m_fRotationDest);
		float timeUntilStateChange = (m_nPedStateTimer - CTimer::GetTimeInMilliseconds())/600.0f;

		if (timeUntilStateChange <= 0.0f) {
			m_vecOffsetSeek.x = 0.0f;
			m_vecOffsetSeek.y = 0.0f;
		}
		m_vecOffsetSeek.z = 0.0f;

		neededPos -= timeUntilStateChange * m_vecOffsetSeek;

		if (PI + m_fRotationCur < limitedDest) {
			limitedDest -= 2 * PI;
		} else if (m_fRotationCur - PI > limitedDest) {
			limitedDest += 2 * PI;
		}
		m_fRotationCur -= (m_fRotationCur - limitedDest) * (1.0f - timeUntilStateChange);
	}

	if (seatPosMult > 0.2f || vehIsUpsideDown) {
		SetPosition(neededPos);
		SetHeading(m_fRotationCur);
	} else {
		CMatrix vehDoorMat(veh->GetMatrix());
		vehDoorMat.GetPosition() += Multiply3x3(vehDoorMat, GetLocalPositionToOpenCarDoor(veh, m_vehDoor, 0.0f));
		// VC couch anims are inverted, so they're fixing it here.
		GetMatrix() = vehDoorMat;
	}

}

void
CPed::SetCarJack(CVehicle* car)
{
	uint8 doorFlag;
	eDoors door;
	CPed *pedInSeat = nil;

	if (car->IsBoat())
		return;

	switch (m_vehDoor) {
		case CAR_DOOR_RF:
			doorFlag = CAR_DOOR_FLAG_RF;
			door = DOOR_FRONT_RIGHT;
			if (car->pPassengers[0]) {
				pedInSeat = car->pPassengers[0];
			} else if (m_nPedType == PEDTYPE_COP) {
				pedInSeat = car->pDriver;
			}
			break;
		case CAR_DOOR_RR:
			doorFlag = CAR_DOOR_FLAG_RR;
			door = DOOR_REAR_RIGHT;
			pedInSeat = car->pPassengers[2];
			break;
		case CAR_DOOR_LF:
			doorFlag = CAR_DOOR_FLAG_LF;
			door = DOOR_FRONT_LEFT;
			pedInSeat = car->pDriver;
			break;
		case CAR_DOOR_LR:
			doorFlag = CAR_DOOR_FLAG_LR;
			door = DOOR_REAR_LEFT;
			pedInSeat = car->pPassengers[1];
			break;
		default:
			doorFlag = CAR_DOOR_FLAG_UNKNOWN;
			break;
	}

	if(car->bIsBus)
		pedInSeat = car->pDriver;

	if (m_fHealth > 0.0f && (IsPlayer() || m_objective == OBJECTIVE_KILL_CHAR_ON_FOOT || m_objective == OBJECTIVE_KILL_CHAR_ANY_MEANS ||
		                     (car->VehicleCreatedBy != MISSION_VEHICLE && car->GetModelIndex() != MI_DODO)))
		if (pedInSeat && !pedInSeat->IsPedDoingDriveByShooting() && pedInSeat->m_nPedState == PED_DRIVING)
			if (m_nPedState != PED_CARJACK && !m_pVehicleAnim)
				if ((car->IsDoorReady(door) || car->IsDoorFullyOpen(door)))
					if (!car->bIsBeingCarJacked && !(doorFlag & car->m_nGettingInFlags) && !(doorFlag & car->m_nGettingOutFlags))
						SetCarJack_AllClear(car, m_vehDoor, doorFlag);
}

void
CPed::SetCarJack_AllClear(CVehicle *car, uint32 doorNode, uint32 doorFlag)
{
	RemoveWeaponWhenEnteringVehicle();
	if (m_nPedState != PED_SEEK_CAR)
		SetStoredState();

	m_pSeekTarget = car;
	m_pSeekTarget->RegisterReference((CEntity**)&m_pSeekTarget);
	SetPedState(PED_CARJACK);
	car->bIsBeingCarJacked = true;
	m_pMyVehicle = (CVehicle*)m_pSeekTarget;
	m_pMyVehicle->RegisterReference((CEntity**)&m_pMyVehicle);
	((CVehicle*)m_pSeekTarget)->m_nNumGettingIn++;

	Say(m_nPedType == PEDTYPE_COP ? SOUND_PED_ARREST_COP : SOUND_PED_CAR_JACKING);
	CVector carEnterPos;
	carEnterPos = GetPositionToOpenCarDoor(car, m_vehDoor);

	car->m_nGettingInFlags |= doorFlag;
	m_vecOffsetSeek = carEnterPos - GetPosition();
	m_nPedStateTimer = CTimer::GetTimeInMilliseconds() + 600;
	float zDiff = Max(0.0f, carEnterPos.z - GetPosition().z);
	bUsesCollision = false;

	if (m_vehDoor == CAR_DOOR_LF || m_vehDoor == CAR_DOOR_LR)
		m_pVehicleAnim = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, zDiff > 4.4f ? ANIM_STD_CAR_ALIGNHI_DOOR_LHS : ANIM_STD_CAR_ALIGN_DOOR_LHS, 4.0f);
	else
		m_pVehicleAnim = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, zDiff > 4.4f ? ANIM_STD_CAR_ALIGNHI_DOOR_RHS : ANIM_STD_CAR_ALIGN_DOOR_RHS, 4.0f);

	m_pVehicleAnim->SetFinishCallback(PedAnimAlignCB, this);
}

void
CPed::SetBeingDraggedFromCar(CVehicle *veh, uint32 vehEnterType, bool quickJack)
{
	if (m_nPedState == PED_DRAG_FROM_CAR)
		return;

	bUsesCollision = false;
	m_vecMoveSpeed = CVector(0.0f, 0.0f, 0.0f);
	m_nLastPedState = PED_IDLE;
	SetMoveState(PEDMOVE_STILL);
	m_pSeekTarget = veh;
	m_pSeekTarget->RegisterReference((CEntity **) &m_pSeekTarget);
	m_vehDoor = vehEnterType;
	if (m_vehDoor == CAR_DOOR_LF) {
		if (veh->pDriver && veh->pDriver->IsPlayer())
			veh->SetStatus(STATUS_PLAYER_DISABLED);
		else
			veh->SetStatus(STATUS_ABANDONED);
	}
	RemoveInCarAnims();
	SetMoveState(PEDMOVE_NONE);
	LineUpPedWithCar(LINE_UP_TO_CAR_START);
	m_pVehicleAnim = nil;
	SetPedState(PED_DRAG_FROM_CAR);
	bChangedSeat = false;
	bWillBeQuickJacked = quickJack;

	SetHeading(m_fRotationCur);

	Say(SOUND_PED_CAR_JACKED);
	SetRadioStation();
	veh->m_nGettingOutFlags |= GetCarDoorFlag(m_vehDoor);
}

void
CPed::BeingDraggedFromCar(void)
{
	CAnimBlendAssociation *animAssoc;
	AnimationId enterAnim;
	bool dontRunAnim = false;
	PedLineUpPhase lineUpType;

	if (!m_pVehicleAnim) {
		CAnimManager::BlendAnimation(GetClump(), m_animGroup, ANIM_STD_IDLE, 100.0f);
		animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_CAR_SIT);
		if (!animAssoc) {
			animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_CAR_SIT_LO);
			if (!animAssoc) {
				animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_CAR_SIT_P);
				if (!animAssoc)
					animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_CAR_SIT_P_LO);
			}
		}
		if (animAssoc)
			animAssoc->blendDelta = -1000.0f;

		if (m_vehDoor == CAR_DOOR_LF || m_vehDoor == CAR_DOOR_LR) {
			if (bWillBeQuickJacked) {
				enterAnim = ANIM_STD_QUICKJACKED;
			} else if (m_pMyVehicle->bLowVehicle) {
				enterAnim = ANIM_STD_JACKEDCAR_LO_LHS;
			} else {
				enterAnim = ANIM_STD_JACKEDCAR_LHS;
			}
		} else if (m_vehDoor == CAR_DOOR_RF || m_vehDoor == CAR_DOOR_RR) {
			if (m_pMyVehicle->bLowVehicle)
				enterAnim = ANIM_STD_JACKEDCAR_LO_RHS;
			else
				enterAnim = ANIM_STD_JACKEDCAR_RHS;
		} else
			dontRunAnim = true;


		if (!dontRunAnim)
			m_pVehicleAnim = CAnimManager::AddAnimation(GetClump(), ASSOCGRP_STD, enterAnim);

		m_pVehicleAnim->SetFinishCallback(PedSetDraggedOutCarCB, this);
		lineUpType = LINE_UP_TO_CAR_START;
	} else if (m_pVehicleAnim->currentTime <= 1.4f) {
		m_vecMoveSpeed = CVector(0.0f, 0.0f, 0.0f);
		lineUpType = LINE_UP_TO_CAR_START;
	} else {
		lineUpType = LINE_UP_TO_CAR_2;
	}
	
	LineUpPedWithCar(lineUpType);
#ifdef VC_PED_PORTS
	if (m_objective == OBJECTIVE_LEAVE_CAR_AND_DIE) {
		if (m_pMyVehicle) {
			m_pMyVehicle->ProcessOpenDoor(m_vehDoor, ANIM_STD_NUM, m_pVehicleAnim->currentTime * 5.0f);
		}
	}
#endif
}

void
CPed::SetEnterCar(CVehicle *car, uint32 unused)
{
	if (CCranes::IsThisCarBeingCarriedByAnyCrane(car)) {
		RestorePreviousState();
		RestorePreviousObjective();
	} else {
		uint8 doorFlag;
		eDoors door;
		switch (m_vehDoor) {
			case CAR_DOOR_RF:
				doorFlag = CAR_DOOR_FLAG_RF;
				door = DOOR_FRONT_RIGHT;
				break;
			case CAR_DOOR_RR:
				doorFlag = CAR_DOOR_FLAG_RR;
				door = DOOR_REAR_RIGHT;
				break;
			case CAR_DOOR_LF:
				doorFlag = CAR_DOOR_FLAG_LF;
				door = DOOR_FRONT_LEFT;
				break;
			case CAR_DOOR_LR:
				doorFlag = CAR_DOOR_FLAG_LR;
				door = DOOR_REAR_LEFT;
				break;
			default:
				doorFlag = CAR_DOOR_FLAG_UNKNOWN;
				break;
		}
		if (!IsPedInControl() || m_fHealth <= 0.0f
			|| doorFlag & car->m_nGettingInFlags || doorFlag & car->m_nGettingOutFlags
			|| car->bIsBeingCarJacked || m_pVehicleAnim
			|| doorFlag && !car->IsDoorReady(door) && !car->IsDoorFullyOpen(door))
			SetMoveState(PEDMOVE_STILL);
		else
			SetEnterCar_AllClear(car, m_vehDoor, doorFlag);
	}
}

void
CPed::SetEnterCar_AllClear(CVehicle *car, uint32 doorNode, uint32 doorFlag)
{
	float zDiff = 0.0f;
	RemoveWeaponWhenEnteringVehicle();
	car->m_nGettingInFlags |= doorFlag;
	bVehEnterDoorIsBlocked = false;
	if (m_nPedState != PED_SEEK_CAR && m_nPedState != PED_SEEK_IN_BOAT)
		SetStoredState();

	m_pSeekTarget = car;
	m_pSeekTarget->RegisterReference((CEntity **) &m_pSeekTarget);
	m_vehDoor = doorNode;
	SetPedState(PED_ENTER_CAR);
	if (m_vehDoor == CAR_DOOR_RF && m_objective == OBJECTIVE_ENTER_CAR_AS_DRIVER && car->m_vehType != VEHICLE_TYPE_BIKE) {
		car->bIsBeingCarJacked = true;
	}

	m_pMyVehicle = (CVehicle*)m_pSeekTarget;
	m_pMyVehicle->RegisterReference((CEntity**) &m_pMyVehicle);
	((CVehicle*)m_pSeekTarget)->m_nNumGettingIn++;
	bUsesCollision = false;
	CVector doorOpenPos = GetPositionToOpenCarDoor(car, m_vehDoor);

	// Because buses have stairs
	if (!m_pMyVehicle->bIsBus)
		zDiff = Max(0.0f, doorOpenPos.z - GetPosition().z);

	m_vecOffsetSeek = doorOpenPos - GetPosition();
	m_nPedStateTimer = CTimer::GetTimeInMilliseconds() + 600;
	if (car->IsBoat()) {
#ifdef VC_PED_PORTS
		// VC checks for handling flag, but we can't do that
		if(car->GetModelIndex() == MI_SPEEDER)
			m_pVehicleAnim = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_CAR_SIT, 100.0f);
		else
			m_pVehicleAnim = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_BOAT_DRIVE, 100.0f);

		PedSetInCarCB(nil, this);
		bVehExitWillBeInstant = true;
#else

#ifndef FIX_BUGS
		m_pVehicleAnim = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_BOAT_DRIVE, 100.0f);
#else
		m_pVehicleAnim = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, car->GetDriverAnim(), 100.0f);
#endif

		m_pVehicleAnim->SetFinishCallback(PedSetInCarCB, this);
#endif
		if (IsPlayer())
			CWaterLevel::AllocateBoatWakeArray();
	} else {
		if (zDiff > 4.4f) {
			if (m_vehDoor == CAR_DOOR_RF || m_vehDoor == CAR_DOOR_RR)
				m_pVehicleAnim = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_CAR_ALIGNHI_DOOR_RHS, 4.0f);
			else
				m_pVehicleAnim = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_CAR_ALIGNHI_DOOR_LHS, 4.0f);

		} else {
			if (m_vehDoor == CAR_DOOR_RF || m_vehDoor == CAR_DOOR_RR)
				m_pVehicleAnim = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_CAR_ALIGN_DOOR_RHS, 4.0f);
			else
				m_pVehicleAnim = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_CAR_ALIGN_DOOR_LHS, 4.0f);
		}
		m_pVehicleAnim->SetFinishCallback(PedAnimAlignCB, this);
		car->AutoPilot.m_nCruiseSpeed = 0;
	}
}

void
CPed::EnterCar(void)
{
	if (IsNotInWreckedVehicle() && m_fHealth > 0.0f) {
		CVehicle *veh = (CVehicle*)m_pSeekTarget;

		// Not used.
		// CVector posForDoor = GetPositionToOpenCarDoor(veh, m_vehDoor);

		if (veh->CanPedOpenLocks(this)) {
			if (m_vehDoor && m_pVehicleAnim) {
				veh->ProcessOpenDoor(m_vehDoor, m_pVehicleAnim->animId, m_pVehicleAnim->currentTime);
			}
		}
		bIsInTheAir = false;
		LineUpPedWithCar(LINE_UP_TO_CAR_START);
	} else {
		QuitEnteringCar();
		SetDie(ANIM_STD_KO_FRONT, 4.0f, 0.0f);
	}
}

void
CPed::QuitEnteringCar(void)
{
	CVehicle *veh = m_pMyVehicle;
	if (m_pVehicleAnim)
		m_pVehicleAnim->blendDelta = -1000.0f;
	
	RestartNonPartialAnims();

	if (!RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_IDLE))
		CAnimManager::BlendAnimation(GetClump(), m_animGroup, ANIM_STD_IDLE, 100.0f);
	
	if (veh) {
		if (m_objective == OBJECTIVE_ENTER_CAR_AS_DRIVER || m_nPedState == PED_CARJACK)
			veh->bIsBeingCarJacked = false;

		if (veh->m_nNumGettingIn != 0)
			veh->m_nNumGettingIn--;

#ifdef VC_PED_PORTS
		if (m_objective == OBJECTIVE_ENTER_CAR_AS_DRIVER || m_objective == OBJECTIVE_ENTER_CAR_AS_PASSENGER)
			RestorePreviousObjective();
#endif

		veh->m_nGettingInFlags &= ~GetCarDoorFlag(m_vehDoor);
	}

	bUsesCollision = true;

	ReplaceWeaponWhenExitingVehicle();

	if (DyingOrDead()) {
		if (m_pVehicleAnim) {
			m_pVehicleAnim->blendDelta = -4.0f;
			m_pVehicleAnim->flags |= ASSOC_DELETEFADEDOUT;
			m_pVehicleAnim->flags &= ~ASSOC_RUNNING;
		}
	} else
		SetIdle();

	m_pVehicleAnim = nil;
	
	if (veh) {
#ifdef VC_PED_PORTS
		if (veh->AutoPilot.m_nCruiseSpeed == 0 && veh->VehicleCreatedBy == RANDOM_VEHICLE)
#else
		if (veh->AutoPilot.m_nCruiseSpeed == 0)
#endif
			veh->AutoPilot.m_nCruiseSpeed = 17;
	}
}

void
AddYardieDoorSmoke(CVehicle *veh, uint32 doorNode)
{
	eDoors door;
	switch (doorNode) {
		case CAR_DOOR_RF:
			door = DOOR_FRONT_RIGHT;
			break;
		case CAR_DOOR_LF:
			door = DOOR_FRONT_LEFT;
			break;
		default:
			break;
	}

	if (!veh->IsDoorMissing(door) && veh->IsComponentPresent(doorNode)) {
		CVector pos;
#ifdef FIX_BUGS
		veh->GetComponentWorldPosition(doorNode, pos);
#else
		veh->GetComponentWorldPosition(CAR_DOOR_LF, pos);
#endif
		CParticle::AddYardieDoorSmoke(pos, veh->GetMatrix());
	}
}

// Seperate function in VC, more logical. Not sure is it inlined in III.
void
CPed::SetExitBoat(CVehicle *boat)
{
#ifndef VC_PED_PORTS
	SetPedState(PED_IDLE);
	CVector firstPos = GetPosition();
	CAnimManager::BlendAnimation(GetClump(), m_animGroup, ANIM_STD_IDLE, 100.0f);
	if (boat->GetModelIndex() == MI_SPEEDER && boat->IsUpsideDown()) {
		m_pVehicleAnim = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_CRAWLOUT_LHS, 8.0f);
		m_pVehicleAnim->SetFinishCallback(PedSetOutCarCB, this);
		m_vehDoor = CAR_DOOR_RF;
		SetPedState(PED_EXIT_CAR);
	} else {
		m_vehDoor = CAR_DOOR_RF;
		PedSetOutCarCB(nil, this);
		bIsStanding = true;
		m_pCurSurface = boat;
		m_pCurSurface->RegisterReference((CEntity**)&m_pCurSurface);
	}
	SetPosition(firstPos);
	SetMoveState(PEDMOVE_STILL);
	m_vecMoveSpeed = boat->m_vecMoveSpeed;
	bTryingToReachDryLand = true;
#else
	SetPedState(PED_IDLE);
	CVector newPos = GetPosition();
	RemoveInCarAnims();
	CColModel* boatCol = boat->GetColModel();
	if (boat->IsUpsideDown()) {
		newPos = { 0.0f, 0.0f, boatCol->boundingBox.min.z };
		newPos = boat->GetMatrix() * newPos;
		newPos.z += 1.0f;
		m_vehDoor = CAR_DOOR_RF;
		PedSetOutCarCB(nil, this);
		bIsStanding = true;
		m_pCurSurface = boat;
		m_pCurSurface->RegisterReference((CEntity**)&m_pCurSurface);
		m_pCurrentPhysSurface = boat;
	} else {
/*		if (boat->m_modelIndex != MI_SKIMMER || boat->bIsInWater) {
			if (boat->m_modelIndex == MI_SKIMMER)
				newPos.z += 2.0f
*/
			m_vehDoor = CAR_DOOR_RF;
			PedSetOutCarCB(nil, this);
			bIsStanding = true;
			m_pCurSurface = boat;
			m_pCurSurface->RegisterReference((CEntity**)&m_pCurSurface);
			m_pCurrentPhysSurface = boat;
			CColPoint foundCol;
			CEntity *foundEnt = nil;
			if (CWorld::ProcessVerticalLine(newPos, newPos.z - 1.4f, foundCol, foundEnt, false, true, false, false, false, false, nil))
				newPos.z = FEET_OFFSET + foundCol.point.z;
/*		// VC specific
		} else {
			m_vehDoor = CAR_DOOR_RF;
			PedSetOutCarCB(nil, this);
			bIsStanding = true;
			SetMoveState(PEDMOVE_STILL);
			bTryingToReachDryLand = true;
			float upMult = 1.04f + boatCol->boundingBox.min.z;
			float rightMult = 0.6f * boatCol->boundingBox.max.x;
			newPos = upMult * boat->GetUp() + rightMult * boat->GetRight() + boat->GetPosition();
			GetPosition() = newPos;
			if (m_pMyVehicle) {
				PositionPedOutOfCollision();
			} else {
				m_pMyVehicle = boat;
				PositionPedOutOfCollision();
				m_pMyVehicle = nil;
			}
			return;
		}
*/	}
	SetPosition(newPos);
	SetMoveState(PEDMOVE_STILL);
	m_vecMoveSpeed = boat->m_vecMoveSpeed;
#endif
	// Not there in VC.
	CWaterLevel::FreeBoatWakeArray();
}

// wantedDoorNode = 0 means that func. will determine it
void
CPed::SetExitCar(CVehicle *veh, uint32 wantedDoorNode)
{
	uint32 optedDoorNode = wantedDoorNode;
	bool teleportNeeded = false;
	bool isLow = !!veh->bLowVehicle;
	if (!veh->CanPedExitCar()) {
		if (veh->pDriver && !veh->pDriver->IsPlayer()) {
			veh->AutoPilot.m_nCruiseSpeed = 0;
			veh->AutoPilot.m_nCarMission = MISSION_NONE;
		}
		return;
	}

	if (m_nPedState == PED_EXIT_CAR || m_nPedState == PED_DRAG_FROM_CAR)
		return;

	m_vecMoveSpeed = CVector(0.0f, 0.0f, 0.0f);
	m_vecTurnSpeed = CVector(0.0f, 0.0f, 0.0f);
	if (wantedDoorNode == 0) {
		optedDoorNode = CAR_DOOR_LF;
		if (!veh->bIsBus) {
			if (veh->pDriver == this) {
				optedDoorNode = CAR_DOOR_LF;
			} else if (veh->pPassengers[0] == this) {
				optedDoorNode = CAR_DOOR_RF;
			} else if (veh->pPassengers[1] == this) {
				optedDoorNode = CAR_DOOR_LR;
			} else if (veh->pPassengers[2] == this) {
				optedDoorNode = CAR_DOOR_RR;
			} else {
				for (int i = 3; i < veh->m_nNumMaxPassengers; ++i) {
					if (veh->pPassengers[i] == this) {
						if (i & 1)
							optedDoorNode = CAR_DOOR_RR;
						else
							optedDoorNode = CAR_DOOR_LR;

						break;
					}
				}
			}
		}
	}
	bool someoneExitsFromOurExitDoor = false;
	bool someoneEntersFromOurExitDoor = false;
	switch (optedDoorNode) {
		case CAR_DOOR_RF:
			if (veh->m_nGettingInFlags & CAR_DOOR_FLAG_RF)
				someoneEntersFromOurExitDoor = true;
			if (veh->m_nGettingOutFlags & CAR_DOOR_FLAG_RF)
				someoneExitsFromOurExitDoor = true;
			break;
		case CAR_DOOR_RR:
			if (veh->m_nGettingInFlags & CAR_DOOR_FLAG_RR)
				someoneEntersFromOurExitDoor = true;
			if (veh->m_nGettingOutFlags & CAR_DOOR_FLAG_RR)
				someoneExitsFromOurExitDoor = true;
			break;
		case CAR_DOOR_LF:
			if (veh->m_nGettingInFlags & CAR_DOOR_FLAG_LF)
				someoneEntersFromOurExitDoor = true;
			if (veh->m_nGettingOutFlags & CAR_DOOR_FLAG_LF)
				someoneExitsFromOurExitDoor = true;
			break;
		case CAR_DOOR_LR:
			if (veh->m_nGettingInFlags & CAR_DOOR_FLAG_LR)
				someoneEntersFromOurExitDoor = true;
			if (veh->m_nGettingOutFlags & CAR_DOOR_FLAG_LR)
				someoneExitsFromOurExitDoor = true;
			break;
		default:
			break;
	}
	if (someoneEntersFromOurExitDoor && m_objective == OBJECTIVE_LEAVE_CAR) {
		RestorePreviousObjective();
		return;
	}
	if (!someoneExitsFromOurExitDoor || m_nPedType == PEDTYPE_COP && veh->bIsBus) {
		// Again, unused...
		// CVector exitPos = GetPositionToOpenCarDoor(veh, optedDoorNode);
		bool thereIsRoom = veh->IsRoomForPedToLeaveCar(optedDoorNode, nil);
		if (veh->IsOnItsSide()) {
			teleportNeeded = true;
		} else if (!thereIsRoom) {
			bool trySideSeat = false;
			CPed *pedOnSideSeat = nil;
			switch (optedDoorNode) {
				case CAR_DOOR_RF:
					if (veh->pDriver || veh->m_nGettingInFlags & CAR_DOOR_FLAG_LF) {
						pedOnSideSeat = veh->pDriver;
						trySideSeat = true;
					} else
						optedDoorNode = CAR_DOOR_LF;

					break;
				case CAR_DOOR_RR:
					if (veh->pPassengers[1] || veh->m_nGettingInFlags & CAR_DOOR_FLAG_LR) {
						pedOnSideSeat = veh->pPassengers[1];
						trySideSeat = true;
					} else
						optedDoorNode = CAR_DOOR_LR;

					break;
				case CAR_DOOR_LF:
					if (veh->pPassengers[0] || veh->m_nGettingInFlags & CAR_DOOR_FLAG_RF) {
						pedOnSideSeat = veh->pPassengers[0];
						trySideSeat = true;
					} else
						optedDoorNode = CAR_DOOR_RF;

					break;
				case CAR_DOOR_LR:
					if (veh->pPassengers[2] || veh->m_nGettingInFlags & CAR_DOOR_FLAG_RR) {
						pedOnSideSeat = (CPed*)veh->pPassengers[2];
						trySideSeat = true;
					} else
						optedDoorNode = CAR_DOOR_RR;

					break;
				default:
					break;
			}
			if (trySideSeat) {
				if (!pedOnSideSeat || !IsPlayer() && CharCreatedBy != MISSION_CHAR)
					return;

				switch (optedDoorNode) {
					case CAR_DOOR_RF:
						optedDoorNode = CAR_DOOR_LF;
						break;
					case CAR_DOOR_RR:
						optedDoorNode = CAR_DOOR_LR;
						break;
					case CAR_DOOR_LF:
						optedDoorNode = CAR_DOOR_RF;
						break;
					case CAR_DOOR_LR:
						optedDoorNode = CAR_DOOR_RR;
						break;
					default:
						break;
				}
			}
			// ...
			// CVector exitPos = GetPositionToOpenCarDoor(veh, optedDoorNode);
			if (!veh->IsRoomForPedToLeaveCar(optedDoorNode, nil)) {
				if (!IsPlayer() && CharCreatedBy != MISSION_CHAR)
					return;

				teleportNeeded = true;
			}
		}
		if (m_nPedState == PED_FLEE_POS) {
			m_nLastPedState = PED_FLEE_POS;
			m_nPrevMoveState = PEDMOVE_RUN;
			SetMoveState(PEDMOVE_SPRINT);
		} else {
			m_nLastPedState = PED_IDLE;
			m_nPrevMoveState = PEDMOVE_STILL;
			SetMoveState(PEDMOVE_STILL);
		}

		ReplaceWeaponWhenExitingVehicle();
		bUsesCollision = false;
		m_pSeekTarget = veh;
		m_pSeekTarget->RegisterReference((CEntity**) &m_pSeekTarget);
		m_vehDoor = optedDoorNode;
		SetPedState(PED_EXIT_CAR);
		if (m_pVehicleAnim && m_pVehicleAnim->flags & ASSOC_PARTIAL)
			m_pVehicleAnim->blendDelta = -1000.0f;
		SetMoveState(PEDMOVE_NONE);
		CAnimManager::BlendAnimation(GetClump(), m_animGroup, ANIM_STD_IDLE, 100.0f);
		RemoveInCarAnims();
		veh->AutoPilot.m_nCruiseSpeed = 0;
		if (teleportNeeded) {
			PedSetOutCarCB(nil, this);

			// This is same code with CPedPlacement::FindZCoorForPed, except we start from z + 1.5 and also check vehicles.
			float zForPed;
			float startZ = GetPosition().z - 100.0f;
			float foundColZ = -100.0f;
			float foundColZ2 = -100.0f;
			CColPoint foundCol;
			CEntity* foundEnt;

			CVector vec = GetPosition();
			vec.z += 1.5f;

			if (CWorld::ProcessVerticalLine(vec, startZ, foundCol, foundEnt, true, true, false, false, true, false, nil))
				foundColZ = foundCol.point.z;

			// Adjust coords and do a second test
			vec.x += 0.1f;
			vec.y += 0.1f;

			if (CWorld::ProcessVerticalLine(vec, startZ, foundCol, foundEnt, true, true, false, false, true, false, nil))
				foundColZ2 = foundCol.point.z;

			zForPed = Max(foundColZ, foundColZ2);

			if (zForPed > -99.0f)
				GetMatrix().GetPosition().z = FEET_OFFSET + zForPed;
		} else {
			if (veh->GetUp().z > -0.8f) {
				bool addDoorSmoke = false;
				if (veh->GetModelIndex() == MI_YARDIE)
					addDoorSmoke = true;

				switch (m_vehDoor) {
					case CAR_DOOR_RF:
						if (veh->bIsBus) {
							m_pVehicleAnim = CAnimManager::AddAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_COACH_GET_OUT_LHS);
						} else {
							if (isLow)
								m_pVehicleAnim = CAnimManager::AddAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_GETOUT_LO_RHS);
							else
								m_pVehicleAnim = CAnimManager::AddAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_GETOUT_RHS);

							if (addDoorSmoke)
								AddYardieDoorSmoke(veh, CAR_DOOR_RF);
						}
						break;
					case CAR_DOOR_RR:
						if (veh->bIsVan) {
							m_pVehicleAnim = CAnimManager::AddAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_VAN_GET_OUT_REAR_RHS);
						} else if (isLow) {
							m_pVehicleAnim = CAnimManager::AddAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_GETOUT_LO_RHS);
						} else {
							m_pVehicleAnim = CAnimManager::AddAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_GETOUT_RHS);
						}
						break;
					case CAR_DOOR_LF:
						if (veh->bIsBus) {
							m_pVehicleAnim = CAnimManager::AddAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_COACH_GET_OUT_LHS);
						} else {
							if (isLow)
								m_pVehicleAnim = CAnimManager::AddAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_GETOUT_LO_LHS);
							else
								m_pVehicleAnim = CAnimManager::AddAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_GETOUT_LHS);

							if (addDoorSmoke)
								AddYardieDoorSmoke(veh, CAR_DOOR_LF);
						}
						break;
					case CAR_DOOR_LR:
						if (veh->bIsVan) {
							m_pVehicleAnim = CAnimManager::AddAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_VAN_GET_OUT_REAR_LHS);
						} else if (isLow) {
							m_pVehicleAnim = CAnimManager::AddAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_GETOUT_LO_LHS);
						} else {
							m_pVehicleAnim = CAnimManager::AddAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_GETOUT_LHS);
						}
						break;
					default:
						break;
				}
				if (!bBusJacked) {
					switch (m_vehDoor) {
						case CAR_DOOR_RF:
							veh->m_nGettingOutFlags |= CAR_DOOR_FLAG_RF;
							break;
						case CAR_DOOR_RR:
							veh->m_nGettingOutFlags |= CAR_DOOR_FLAG_RR;
							break;
						case CAR_DOOR_LF:
							veh->m_nGettingOutFlags |= CAR_DOOR_FLAG_LF;
							break;
						case CAR_DOOR_LR:
							veh->m_nGettingOutFlags |= CAR_DOOR_FLAG_LR;
							break;
						default:
							break;
					}
				}
				m_pVehicleAnim->SetFinishCallback(PedAnimStepOutCarCB, this);
			} else {
				if (m_vehDoor == CAR_DOOR_RF || m_vehDoor == CAR_DOOR_RR) {
					m_pVehicleAnim = CAnimManager::AddAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_CRAWLOUT_RHS);
				} else if (m_vehDoor == CAR_DOOR_LF || m_vehDoor == CAR_DOOR_LR) {
					m_pVehicleAnim = CAnimManager::AddAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_CRAWLOUT_LHS);
				}
				m_pVehicleAnim->SetFinishCallback(PedSetOutCarCB, this);
			}
		}
		bChangedSeat = false;
		if (veh->bIsBus)
			bRenderPedInCar = true;

		SetRadioStation();
		if (veh->pDriver == this) {
			if (IsPlayer())
				veh->SetStatus(STATUS_PLAYER_DISABLED);
			else
				veh->SetStatus(STATUS_ABANDONED);
		}
	}
}

void
CPed::ExitCar(void)
{
	if (!m_pVehicleAnim)
		return;

	AnimationId exitAnim = (AnimationId) m_pVehicleAnim->animId;
	float animTime = m_pVehicleAnim->currentTime;

	m_pMyVehicle->ProcessOpenDoor(m_vehDoor, exitAnim, animTime);

	if (m_pSeekTarget) {
		// Car is upside down
		if (m_pMyVehicle->GetUp().z > -0.8f) {
			if (exitAnim == ANIM_STD_CAR_CLOSE_RHS || exitAnim == ANIM_STD_CAR_CLOSE_LHS || animTime > 0.3f)
				LineUpPedWithCar(LINE_UP_TO_CAR_END);
			else
				LineUpPedWithCar((m_pMyVehicle->GetModelIndex() == MI_DODO ? LINE_UP_TO_CAR_END : LINE_UP_TO_CAR_START));
		} else {
			LineUpPedWithCar(LINE_UP_TO_CAR_END);
		}
	}

	// If there is someone in front of the door, make him fall while we exit.
	if (m_nPedState == PED_EXIT_CAR) {
		CPed *foundPed = nil;
		for (int i = 0; i < m_numNearPeds; i++) {
			if ((m_nearPeds[i]->GetPosition() - GetPosition()).MagnitudeSqr2D() < 0.04f) {
				foundPed = m_nearPeds[i];
				break;
			}
		}
		if (foundPed && animTime > 0.4f && foundPed->IsPedInControl())
			foundPed->SetFall(1000, ANIM_STD_HIGHIMPACT_FRONT, 1);
	}
}

// This function was mostly duplicate of GetLocalPositionToOpenCarDoor, so I've used it.
CVector
CPed::GetPositionToOpenCarDoor(CVehicle *veh, uint32 component)
{
	CVector localPos;
	CVector vehDoorPos;

	localPos = GetLocalPositionToOpenCarDoor(veh, component, 1.0f);
	vehDoorPos = Multiply3x3(veh->GetMatrix(), localPos) + veh->GetPosition();

/*
	// Not used.
	CVector localVehDoorOffset;

	if (veh->bIsVan && (component == VEHICLE_ENTER_REAR_LEFT || component == VEHICLE_ENTER_REAR_RIGHT)) {
		localVehDoorOffset = vecPedVanRearDoorAnimOffset;
	} else {
		if (veh->bIsLow) {
			localVehDoorOffset = vecPedCarDoorLoAnimOffset;
		} else {
			localVehDoorOffset = vecPedCarDoorAnimOffset;
		}
	}

	vehDoorPosWithoutOffset = Multiply3x3(veh->GetMatrix(), localPos + localVehDoorOffset) + veh->GetPosition();
*/
	return vehDoorPos;
}

void
CPed::GetNearestDoor(CVehicle *veh, CVector &posToOpen)
{
	CVector *enterOffset = nil;
	if (m_vehDoor == CAR_DOOR_LF && veh->pDriver
		|| m_vehDoor == CAR_DOOR_RF && veh->pPassengers[0]
		|| m_vehDoor == CAR_DOOR_LR && veh->pPassengers[1]
		|| m_vehDoor == CAR_DOOR_RR && veh->pPassengers[2])
	{
		enterOffset = &vecPedQuickDraggedOutCarAnimOffset;
	}

	CVector lfPos = GetPositionToOpenCarDoor(veh, CAR_DOOR_LF);
	CVector rfPos = GetPositionToOpenCarDoor(veh, CAR_DOOR_RF);

	// Left front door is closer
	if ((lfPos - GetPosition()).MagnitudeSqr2D() < (rfPos - GetPosition()).MagnitudeSqr2D()) {

		if (veh->IsRoomForPedToLeaveCar(CAR_DOOR_LF, enterOffset)) {
			m_vehDoor = CAR_DOOR_LF;
			posToOpen = lfPos;
		} else if (veh->IsRoomForPedToLeaveCar(CAR_DOOR_RF, enterOffset)) {
			m_vehDoor = CAR_DOOR_RF;
			posToOpen = rfPos;
		}
	} else {

		if (veh->IsRoomForPedToLeaveCar(CAR_DOOR_RF, enterOffset)) {

			CPed *rfPassenger = veh->pPassengers[0];
			if (rfPassenger && (rfPassenger->m_leader == this || rfPassenger->bDontDragMeOutCar ||
						veh->VehicleCreatedBy == MISSION_VEHICLE && m_objective == OBJECTIVE_ENTER_CAR_AS_DRIVER)
					&& veh->IsRoomForPedToLeaveCar(CAR_DOOR_LF, enterOffset)
				|| (veh->m_nGettingInFlags & CAR_DOOR_FLAG_RF) && veh->IsRoomForPedToLeaveCar(CAR_DOOR_LF, enterOffset)) {

				m_vehDoor = CAR_DOOR_LF;
				posToOpen = lfPos;
			} else {
				m_vehDoor = CAR_DOOR_RF;
				posToOpen = rfPos;
			}
		} else if (veh->IsRoomForPedToLeaveCar(CAR_DOOR_LF, enterOffset)) {
			m_vehDoor = CAR_DOOR_LF;
			posToOpen = lfPos;
		}
	}
}

bool
CPed::GetNearestPassengerDoor(CVehicle *veh, CVector &posToOpen)
{
	CVector rfPos, lrPos, rrPos;
	bool canEnter = false;

	CVehicleModelInfo *vehModel = (CVehicleModelInfo *)CModelInfo::GetModelInfo(veh->GetModelIndex());

	switch (veh->GetModelIndex()) {
		case MI_BUS:
			m_vehDoor = CAR_DOOR_RF;
			posToOpen = GetPositionToOpenCarDoor(veh, CAR_DOOR_RF);
			return true;
		case MI_RHINO:
		default:
			break;
	}

	CVector2D rfPosDist(999.0f, 999.0f);
	CVector2D lrPosDist(999.0f, 999.0f);
	CVector2D rrPosDist(999.0f, 999.0f);

	if (!veh->pPassengers[0]
		&& !(veh->m_nGettingInFlags & CAR_DOOR_FLAG_RF)
		&& veh->IsRoomForPedToLeaveCar(CAR_DOOR_RF, nil)) {

		rfPos = GetPositionToOpenCarDoor(veh, CAR_DOOR_RF);
		canEnter = true;
		rfPosDist = rfPos - GetPosition();
	}
	if (vehModel->m_numDoors == 4) {
		if (!veh->pPassengers[1]
			&& !(veh->m_nGettingInFlags & CAR_DOOR_FLAG_LR)
			&& veh->IsRoomForPedToLeaveCar(CAR_DOOR_LR, nil)) {
			lrPos = GetPositionToOpenCarDoor(veh, CAR_DOOR_LR);
			canEnter = true;
			lrPosDist = lrPos - GetPosition();
		}
		if (!veh->pPassengers[2]
			&& !(veh->m_nGettingInFlags & CAR_DOOR_FLAG_RR)
			&& veh->IsRoomForPedToLeaveCar(CAR_DOOR_RR, nil)) {
			rrPos = GetPositionToOpenCarDoor(veh, CAR_DOOR_RR);
			canEnter = true;
			rrPosDist = rrPos - GetPosition();
		}

		// When the door we should enter is blocked by some object.
		if (!canEnter)
			veh->ShufflePassengersToMakeSpace();
	}

	CVector2D nextToCompare = rfPosDist;
	posToOpen = rfPos;
	m_vehDoor = CAR_DOOR_RF;
	if (lrPosDist.MagnitudeSqr() < nextToCompare.MagnitudeSqr()) {
		m_vehDoor = CAR_DOOR_LR;
		posToOpen = lrPos;
		nextToCompare = lrPosDist;
	}

	if (rrPosDist.MagnitudeSqr() < nextToCompare.MagnitudeSqr()) {
		m_vehDoor = CAR_DOOR_RR;
		posToOpen = rrPos;
	}
	return canEnter;
}

void
CPed::GoToNearestDoor(CVehicle *veh)
{
	CVector posToOpen;
	GetNearestDoor(veh, posToOpen);
	SetSeek(posToOpen, 0.5f);
	SetMoveState(PEDMOVE_RUN);
}

void
CPed::SetAnimOffsetForEnterOrExitVehicle(void)
{
	// FIX: If there were no translations on enter anims, there were overflows all over this function.

	CAnimBlendHierarchy *enterAssoc = CAnimManager::GetAnimAssociation(ASSOCGRP_STD, ANIM_STD_JACKEDCAR_LHS)->hierarchy;
	CAnimBlendSequence *seq = enterAssoc->sequences;
	CAnimManager::UncompressAnimation(enterAssoc);
	if (seq->numFrames > 0) {
		if (!seq->HasTranslation())
			vecPedDraggedOutCarAnimOffset = CVector(0.0f, 0.0f, 0.0f);
		else {
			KeyFrameTrans *lastFrame = (KeyFrameTrans*)seq->GetKeyFrame(seq->numFrames - 1);
			vecPedDraggedOutCarAnimOffset = lastFrame->translation;
		}
	}

	enterAssoc = CAnimManager::GetAnimAssociation(ASSOCGRP_STD, ANIM_STD_CAR_GET_IN_LHS)->hierarchy;
	seq = enterAssoc->sequences;
	CAnimManager::UncompressAnimation(enterAssoc);
	if (seq->numFrames > 0) {
		if (!seq->HasTranslation())
			vecPedCarDoorAnimOffset = CVector(0.0f, 0.0f, 0.0f);
		else {
			KeyFrameTrans *lastFrame = (KeyFrameTrans*)seq->GetKeyFrame(seq->numFrames - 1);
			vecPedCarDoorAnimOffset = lastFrame->translation;
		}
	}

	enterAssoc = CAnimManager::GetAnimAssociation(ASSOCGRP_STD, ANIM_STD_CAR_GET_IN_LO_LHS)->hierarchy;
	seq = enterAssoc->sequences;
	CAnimManager::UncompressAnimation(enterAssoc);
	if (seq->numFrames > 0) {
		if (!seq->HasTranslation())
			vecPedCarDoorLoAnimOffset = CVector(0.0f, 0.0f, 0.0f);
		else {
			KeyFrameTrans *lastFrame = (KeyFrameTrans*)seq->GetKeyFrame(seq->numFrames - 1);
			vecPedCarDoorLoAnimOffset = lastFrame->translation;
		}
	}

	enterAssoc = CAnimManager::GetAnimAssociation(ASSOCGRP_STD, ANIM_STD_QUICKJACKED)->hierarchy;
	seq = enterAssoc->sequences;
	CAnimManager::UncompressAnimation(enterAssoc);
	if (seq->numFrames > 0) {
		if (!seq->HasTranslation())
			vecPedQuickDraggedOutCarAnimOffset = CVector(0.0f, 0.0f, 0.0f);
		else {
			KeyFrameTrans *lastFrame = (KeyFrameTrans*)seq->GetKeyFrame(seq->numFrames - 1);
			vecPedQuickDraggedOutCarAnimOffset = lastFrame->translation;
		}
	}

	enterAssoc = CAnimManager::GetAnimAssociation(ASSOCGRP_STD, ANIM_STD_VAN_GET_IN_REAR_LHS)->hierarchy;
	seq = enterAssoc->sequences;
	CAnimManager::UncompressAnimation(enterAssoc);
	if (seq->numFrames > 0) {
		if (!seq->HasTranslation())
			vecPedVanRearDoorAnimOffset = CVector(0.0f, 0.0f, 0.0f);
		else {
			KeyFrameTrans *lastFrame = (KeyFrameTrans*)seq->GetKeyFrame(seq->numFrames - 1);
			vecPedVanRearDoorAnimOffset = lastFrame->translation;
		}
	}

	enterAssoc = CAnimManager::GetAnimAssociation(ASSOCGRP_STD, ANIM_STD_TRAIN_GETOUT)->hierarchy;
	seq = enterAssoc->sequences;
	CAnimManager::UncompressAnimation(enterAssoc);
	if (seq->numFrames > 0) {
		if (!seq->HasTranslation())
			vecPedTrainDoorAnimOffset = CVector(0.0f, 0.0f, 0.0f);
		else {
			KeyFrameTrans *lastFrame = (KeyFrameTrans*)seq->GetKeyFrame(seq->numFrames - 1);
			vecPedTrainDoorAnimOffset = lastFrame->translation;
		}
	}
}

void
CPed::PedSetQuickDraggedOutCarPositionCB(CAnimBlendAssociation *animAssoc, void *arg)
{
	CPed *ped = (CPed*)arg;

	CVehicle *veh = ped->m_pMyVehicle;

	CVector finalPos;
	CVector draggedOutOffset;

	CMatrix pedMat(ped->GetMatrix());
	ped->bUsesCollision = true;
	ped->RestartNonPartialAnims();
	draggedOutOffset = vecPedQuickDraggedOutCarAnimOffset;
	if (ped->m_vehDoor == CAR_DOOR_RF || ped->m_vehDoor == CAR_DOOR_RR)
		draggedOutOffset.x = -draggedOutOffset.x;

	finalPos = Multiply3x3(pedMat, draggedOutOffset) + ped->GetPosition();
	CPedPlacement::FindZCoorForPed(&finalPos);
	ped->m_vecMoveSpeed = CVector(0.0f, 0.0f, 0.0f);
	ped->SetPosition(finalPos);

	if (veh) {
		ped->m_fRotationDest = veh->GetForward().Heading() - HALFPI;
		ped->m_fRotationCur = ped->m_fRotationDest;
		ped->CalculateNewOrientation();

		if (!veh->IsRoomForPedToLeaveCar(ped->m_vehDoor, &vecPedQuickDraggedOutCarAnimOffset))
			ped->PositionPedOutOfCollision();
	}

	if (!ped->CanSetPedState())
		return;

	ped->SetIdle();
	if (veh) {
		if (ped->bFleeAfterExitingCar) {
			ped->bFleeAfterExitingCar = false;
			ped->SetFlee(veh->GetPosition(), 14000);

		} else if (ped->bWanderPathAfterExitingCar) {
			ped->SetWanderPath(CGeneral::GetRandomNumberInRange(0.0f, 8.0f));
			ped->bWanderPathAfterExitingCar = false;

		} else if (ped->bGonnaKillTheCarJacker) {
			ped->bGonnaKillTheCarJacker = false;
			if (ped->m_pedInObjective && CGeneral::GetRandomNumber() & 1) {
				if (ped->m_objective != OBJECTIVE_KILL_CHAR_ON_FOOT)
					ped->SetObjective(OBJECTIVE_KILL_CHAR_ON_FOOT, ped->m_pedInObjective);

			} else {
				CPed *driver = veh->pDriver;
				if (!driver || driver == ped || driver->IsPlayer() && CTheScripts::IsPlayerOnAMission()) {
					ped->SetFlee(veh->GetPosition(), 14000);
				} else {
					ped->ClearObjective();
					ped->SetObjective(OBJECTIVE_ENTER_CAR_AS_DRIVER, veh);
				}
				ped->bUsePedNodeSeek = true;
				ped->m_pNextPathNode = nil;
				ped->Say(SOUND_PED_FLEE_RUN);
			}
		} else if (ped->m_pedStats->m_temper > ped->m_pedStats->m_fear
			&& ped->CharCreatedBy != MISSION_CHAR && veh->VehicleCreatedBy != MISSION_VEHICLE
			&& veh->pDriver && veh->pDriver->IsPlayer()
			&& !CTheScripts::IsPlayerOnAMission()) {

#ifndef VC_PED_PORTS
			if (CGeneral::GetRandomNumber() < MYRAND_MAX / 2) {
				ped->SetObjective(OBJECTIVE_KILL_CHAR_ON_FOOT, veh->pDriver);
			} else
#endif
				ped->SetObjective(OBJECTIVE_ENTER_CAR_AS_DRIVER, veh);

#ifdef VC_PED_PORTS
		} else if (ped->m_pedStats->m_temper > ped->m_pedStats->m_fear
			&& ped->CharCreatedBy != MISSION_CHAR && veh->VehicleCreatedBy != MISSION_VEHICLE
			&& !veh->pDriver && FindPlayerPed()->m_carInObjective == veh
			&& !CTheScripts::IsPlayerOnAMission()) {
			ped->SetObjective(OBJECTIVE_ENTER_CAR_AS_DRIVER, veh);
#endif
		} else {
			ped->SetFindPathAndFlee(veh->GetPosition(), 10000);
			if (CGeneral::GetRandomNumber() & 1 || ped->m_pedStats->m_fear > 70) {
				ped->SetMoveState(PEDMOVE_SPRINT);
				ped->Say(SOUND_PED_FLEE_SPRINT);
			} else {
				ped->Say(SOUND_PED_FLEE_RUN);
			}
		}
	}
	if (ped->m_nLastPedState == PED_IDLE)
		ped->m_nLastPedState = PED_WANDER_PATH;
}

void
CPed::PedSetDraggedOutCarPositionCB(CAnimBlendAssociation* animAssoc, void* arg)
{
	CPed *ped = (CPed*)arg;

	ped->bUsesCollision = true;
	ped->RestartNonPartialAnims();
	bool itsRearDoor = false;

	if (ped->m_vehDoor == CAR_DOOR_RF || ped->m_vehDoor == CAR_DOOR_RR)
		itsRearDoor = true;

	CMatrix pedMat(ped->GetMatrix());
	CVector posAfterBeingDragged = Multiply3x3(pedMat, (itsRearDoor ? -vecPedDraggedOutCarAnimOffset : vecPedDraggedOutCarAnimOffset));
	posAfterBeingDragged += ped->GetPosition();
#ifndef VC_PED_PORTS
	posAfterBeingDragged.z += 1.0f;
#endif
	CPedPlacement::FindZCoorForPed(&posAfterBeingDragged);
	ped->m_vecMoveSpeed = CVector(0.0f, 0.0f, 0.0f);
	ped->SetPosition(posAfterBeingDragged);

	if (ped->m_pMyVehicle && !ped->m_pMyVehicle->IsRoomForPedToLeaveCar(ped->m_vehDoor, &vecPedDraggedOutCarAnimOffset)) {
		ped->PositionPedOutOfCollision();
	}

	if (!ped->CanSetPedState())
		return;
	
	if (!ped->m_pMyVehicle) {
		ped->SetIdle();
		ped->SetGetUp();
		return;
	}

	CPed *driver = ped->m_pMyVehicle->pDriver;

	if (ped->IsPlayer()) {
		ped->SetIdle();

	} else if (ped->bFleeAfterExitingCar) {
		ped->bFleeAfterExitingCar = false;
		ped->SetFlee(ped->m_pMyVehicle->GetPosition(), 4000);

	} else if (ped->bWanderPathAfterExitingCar) {
		ped->SetWanderPath(CGeneral::GetRandomNumberInRange(0.0f, 8.0f));
		ped->bWanderPathAfterExitingCar = false;

	} else if (ped->bGonnaKillTheCarJacker) {
		// Kill objective is already set at this point.

		ped->bGonnaKillTheCarJacker = false;
		if (!ped->m_pedInObjective || !(CGeneral::GetRandomNumber() & 1)) {
			if (!driver || driver == ped || driver->IsPlayer() && CTheScripts::IsPlayerOnAMission()) {
				ped->SetPedState(PED_NONE);
				ped->m_nLastPedState = PED_NONE;
				ped->SetFlee(ped->m_pMyVehicle->GetPosition(), 4000);
			} else {
				ped->ClearObjective();
				ped->SetObjective(OBJECTIVE_ENTER_CAR_AS_DRIVER, ped->m_pMyVehicle);
			}
		}

	} else if (ped->m_pedStats->m_temper > ped->m_pedStats->m_fear && ped->CharCreatedBy != MISSION_CHAR
		&& ped->m_pMyVehicle->VehicleCreatedBy != MISSION_VEHICLE && driver
		&& driver->IsPlayer() && !CTheScripts::IsPlayerOnAMission()) {

#ifndef VC_PED_PORTS
		if (CGeneral::GetRandomNumber() & 1)
			ped->SetObjective(OBJECTIVE_KILL_CHAR_ON_FOOT, driver);
		else
#endif
			ped->SetObjective(OBJECTIVE_ENTER_CAR_AS_DRIVER, ped->m_pMyVehicle);

	} else {
#ifdef VC_PED_PORTS
		if (ped->m_pedStats->m_temper > ped->m_pedStats->m_fear && ped->CharCreatedBy != MISSION_CHAR
			&& ped->m_pMyVehicle->VehicleCreatedBy != MISSION_VEHICLE && !driver
			&& FindPlayerPed()->m_carInObjective == ped->m_pMyVehicle && !CTheScripts::IsPlayerOnAMission())
			ped->SetObjective(OBJECTIVE_ENTER_CAR_AS_DRIVER, ped->m_pMyVehicle);
		else
#endif
		{
			ped->SetPedState(PED_NONE);
			ped->m_nLastPedState = PED_NONE;
			ped->SetFindPathAndFlee(ped->m_pMyVehicle->GetPosition(), 10000);
		}
	}
	ped->SetGetUp();
}

uint8
CPed::GetNearestTrainDoor(CVehicle *train, CVector &doorPos)
{
	GetNearestTrainPedPosition(train, doorPos);
/*
	// Not used.
	CVehicleModelInfo* trainModel = (CVehicleModelInfo*)CModelInfo::GetModelInfo(train->m_modelIndex);
	CMatrix trainMat = CMatrix(train->GetMatrix());

	doorPos = trainModel->m_positions[m_vehDoor];
	doorPos.x -= 1.5f;
	doorPos = Multiply3x3(trainMat, doorPos);
	doorPos += train->GetPosition();
*/
	return 1;
}

uint8
CPed::GetNearestTrainPedPosition(CVehicle *train, CVector &enterPos)
{
	CVector enterStepOffset;
	CVehicleModelInfo *trainModel = (CVehicleModelInfo *)CModelInfo::GetModelInfo(train->GetModelIndex());
	CMatrix trainMat = CMatrix(train->GetMatrix());
	CVector leftEntryPos, rightEntryPos, midEntryPos;
	float distLeftEntry, distRightEntry, distMidEntry;

	// enterStepOffset = vecPedCarDoorAnimOffset;
	enterStepOffset = CVector(1.5f, 0.0f, 0.0f);

	if (train->pPassengers[TRAIN_POS_LEFT_ENTRY]) {
		distLeftEntry = 999.0f;
	} else {
		leftEntryPos = trainModel->m_positions[TRAIN_POS_LEFT_ENTRY] - enterStepOffset;
		leftEntryPos = Multiply3x3(trainMat, leftEntryPos);
		leftEntryPos += train->GetPosition();
		distLeftEntry = (leftEntryPos - GetPosition()).Magnitude();
	}

	if (train->pPassengers[TRAIN_POS_MID_ENTRY]) {
		distMidEntry = 999.0f;
	} else {
		midEntryPos = trainModel->m_positions[TRAIN_POS_MID_ENTRY] - enterStepOffset;
		midEntryPos = Multiply3x3(trainMat, midEntryPos);
		midEntryPos += train->GetPosition();
		distMidEntry = (midEntryPos - GetPosition()).Magnitude();
	}

	if (train->pPassengers[TRAIN_POS_RIGHT_ENTRY]) {
		distRightEntry = 999.0f;
	} else {
		rightEntryPos = trainModel->m_positions[TRAIN_POS_RIGHT_ENTRY] - enterStepOffset;
		rightEntryPos = Multiply3x3(trainMat, rightEntryPos);
		rightEntryPos += train->GetPosition();
		distRightEntry = (rightEntryPos - GetPosition()).Magnitude();
	}

	if (distMidEntry < distLeftEntry) {
		if (distMidEntry < distRightEntry) {
			enterPos = midEntryPos;
			m_vehDoor = TRAIN_POS_MID_ENTRY;
		} else {
			enterPos = rightEntryPos;
			m_vehDoor = TRAIN_POS_RIGHT_ENTRY;
		}
	} else if (distRightEntry < distLeftEntry) {
		enterPos = rightEntryPos;
		m_vehDoor = TRAIN_POS_RIGHT_ENTRY;
	} else {
		enterPos = leftEntryPos;
		m_vehDoor = TRAIN_POS_LEFT_ENTRY;
	}

	return 1;
}

void
CPed::PedSetInTrainCB(CAnimBlendAssociation* animAssoc, void* arg)
{
	CPed *ped = (CPed*)arg;
	CTrain *veh = (CTrain*)ped->m_pMyVehicle;

	if (!veh)
		return;

	ped->bInVehicle = true;
	ped->SetPedState(PED_DRIVING);
	ped->RestorePreviousObjective();
	ped->SetMoveState(PEDMOVE_STILL);
	veh->AddPassenger(ped);
}

void
CPed::SetEnterTrain(CVehicle *train, uint32 unused)
{
	if (m_nPedState == PED_ENTER_TRAIN || !((CTrain*)train)->Doors[0].IsFullyOpen())
		return;

	/*
	// Not used
	CVector enterPos;
	GetNearestTrainPedPosition(train, enterPos);
	*/
	m_fRotationCur = train->GetForward().Heading() - HALFPI;
	m_pMyVehicle = train;
	m_pMyVehicle->RegisterReference((CEntity **) &m_pMyVehicle);

	SetPedState(PED_ENTER_TRAIN);
	m_pVehicleAnim = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_TRAIN_GETIN, 4.0f);
	m_pVehicleAnim->SetFinishCallback(PedSetInTrainCB, this);
	bUsesCollision = false;
	LineUpPedWithTrain();
	if (IsPlayer()) {
		if (((CPlayerPed*)this)->m_bAdrenalineActive)
			((CPlayerPed*)this)->ClearAdrenaline();
	}
}

void
CPed::EnterTrain(void)
{
	LineUpPedWithTrain();
}

void
CPed::SetPedPositionInTrain(void)
{
	LineUpPedWithTrain();
}

void
CPed::LineUpPedWithTrain(void)
{
	CVector lineUpPos;
	CVehicleModelInfo *trainModel = (CVehicleModelInfo *)CModelInfo::GetModelInfo(m_pMyVehicle->GetModelIndex());
	CVector enterOffset(1.5f, 0.0f, -0.2f);

	m_vecMoveSpeed = CVector(0.0f, 0.0f, 0.0f);
	m_fRotationCur = m_pMyVehicle->GetForward().Heading() - HALFPI;
	m_fRotationDest = m_fRotationCur;

	if (!bInVehicle) {
		GetNearestTrainDoor(m_pMyVehicle, lineUpPos);
		lineUpPos.z += 0.2f;
	} else {
		if (m_pMyVehicle->pPassengers[TRAIN_POS_LEFT_ENTRY] == this) {

			lineUpPos = trainModel->m_positions[TRAIN_POS_LEFT_ENTRY] - enterOffset;

		} else if (m_pMyVehicle->pPassengers[TRAIN_POS_MID_ENTRY] == this) {

			lineUpPos = trainModel->m_positions[TRAIN_POS_MID_ENTRY] - enterOffset;

		} else if (m_pMyVehicle->pPassengers[TRAIN_POS_RIGHT_ENTRY] == this) {

			lineUpPos = trainModel->m_positions[TRAIN_POS_RIGHT_ENTRY] - enterOffset;
		}
		lineUpPos = Multiply3x3(m_pMyVehicle->GetMatrix(), lineUpPos);
		lineUpPos += m_pMyVehicle->GetPosition();
	}

	if (m_pVehicleAnim) {
		float percentageLeft = m_pVehicleAnim->GetTimeLeft() / m_pVehicleAnim->hierarchy->totalLength;
		lineUpPos += (GetPosition() - lineUpPos) * percentageLeft;
	}

	SetPosition(lineUpPos);
	SetHeading(m_fRotationCur);
}

void
CPed::SetExitTrain(CVehicle* train)
{
	if (m_nPedState == PED_EXIT_TRAIN || train->GetStatus() != STATUS_TRAIN_NOT_MOVING || !((CTrain*)train)->Doors[0].IsFullyOpen())
		return;

	/*
	// Not used
	CVector exitPos;
	GetNearestTrainPedPosition(train, exitPos);
	*/
	SetPedState(PED_EXIT_TRAIN);
	m_pVehicleAnim = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_TRAIN_GETOUT, 4.0f);
	m_pVehicleAnim->SetFinishCallback(PedSetOutTrainCB, this);
	bUsesCollision = false;
	LineUpPedWithTrain();
}

void
CPed::ExitTrain(void)
{
	LineUpPedWithTrain();
}

void
CPed::PedSetOutTrainCB(CAnimBlendAssociation *animAssoc, void *arg)
{
	CPed *ped = (CPed*)arg;

	CVehicle *veh = ped->m_pMyVehicle;

	if (ped->m_pVehicleAnim)
		ped->m_pVehicleAnim->blendDelta = -1000.0f;

	ped->bUsesCollision = true;
	ped->m_pVehicleAnim = nil;
	ped->bInVehicle = false;
	ped->SetPedState(PED_IDLE);
	ped->RestorePreviousObjective();
	ped->SetMoveState(PEDMOVE_STILL);

	CMatrix pedMat(ped->GetMatrix());
	ped->m_fRotationCur = HALFPI + veh->GetForward().Heading();
	ped->m_fRotationDest = ped->m_fRotationCur;
	CVector posAfterExit = Multiply3x3(pedMat, vecPedTrainDoorAnimOffset);
	posAfterExit += ped->GetPosition();
	CPedPlacement::FindZCoorForPed(&posAfterExit);
	ped->m_vecMoveSpeed = CVector(0.0f, 0.0f, 0.0f);
	ped->SetPosition(posAfterExit);
	ped->SetHeading(ped->m_fRotationCur);
	veh->RemovePassenger(ped);
}

void
CPed::RegisterThreatWithGangPeds(CEntity *attacker)
{
	CPed *attackerPed = nil;
	if (attacker) {
		if (m_objective != OBJECTIVE_KILL_CHAR_ON_FOOT && m_objective != OBJECTIVE_KILL_CHAR_ANY_MEANS) {
			if (attacker->IsPed()) {
				attackerPed = (CPed*)attacker;
			} else {
				if (!attacker->IsVehicle())
					return;

				attackerPed = ((CVehicle*)attacker)->pDriver;
				if (!attackerPed)
					return;
			}

			if (attackerPed && (attackerPed->IsPlayer() || attackerPed->IsGangMember())) {
				for (int i = 0; i < m_numNearPeds; ++i) {
					CPed *nearPed = m_nearPeds[i];
					if (nearPed->IsPointerValid()) {
						if (nearPed != this && nearPed->m_nPedType == m_nPedType)
							nearPed->m_fearFlags |= CPedType::GetFlag(attackerPed->m_nPedType);
					}
				}
			}
		}
	}

	if (attackerPed && attackerPed->IsPlayer() && (attackerPed->m_nPedState == PED_CARJACK || attackerPed->bInVehicle)) {
		if (!attackerPed->m_pMyVehicle || attackerPed->m_pMyVehicle->GetModelIndex() != MI_TOYZ) {
			int16 lastVehicle;
			CEntity *vehicles[8];
			CWorld::FindObjectsInRange(GetPosition(), ENTER_CAR_MAX_DIST, true, &lastVehicle, 6, vehicles, false, true, false, false, false);

			if (lastVehicle > 8)
				lastVehicle = 8;

			for (int j = 0; j < lastVehicle; ++j) {
				CVehicle *nearVeh = (CVehicle*) vehicles[j];

				if (nearVeh->VehicleCreatedBy != MISSION_VEHICLE) {
					CPed *nearVehDriver = nearVeh->pDriver;

					if (nearVehDriver && nearVehDriver != this && nearVehDriver->m_nPedType == m_nPedType) {

						if (nearVeh->IsVehicleNormal() && nearVeh->IsCar()) {
							nearVeh->AutoPilot.m_nCruiseSpeed = GAME_SPEED_TO_CARAI_SPEED * nearVeh->pHandling->Transmission.fMaxCruiseVelocity * 0.8f;
							nearVeh->AutoPilot.m_nCarMission = MISSION_RAMPLAYER_FARAWAY;
							nearVeh->SetStatus(STATUS_PHYSICS);
							nearVeh->AutoPilot.m_nTempAction = TEMPACT_NONE;
							nearVeh->AutoPilot.m_nDrivingStyle = DRIVINGSTYLE_AVOID_CARS;
						}
					}
				}
			}
		}
	}
}

// Some helper function which doesn't exist in og game.
inline void
SelectClosestNodeForSeek(CPed *ped, CPathNode *node, CVector2D closeDist, CVector2D farDist, CPathNode *closeNode, CPathNode *closeNode2, int runCount = 3)
{
	for (int i = 0; i < node->numLinks; i++) {

		CPathNode *testNode = &ThePaths.m_pathNodes[ThePaths.ConnectedNode(i + node->firstLink)];

		if (testNode && testNode != closeNode && testNode != closeNode2) {
			CVector2D posDiff(ped->m_vecSeekPos - testNode->GetPosition());
			float dist = posDiff.MagnitudeSqr();

			if (farDist.MagnitudeSqr() > dist) {

				if (closeDist.MagnitudeSqr() <= dist) {
					ped->m_pNextPathNode = closeNode;
					closeDist = posDiff;
				} else {
					ped->m_pNextPathNode = (closeNode2 ? closeNode2 : testNode);
					farDist = posDiff;
				}
			}

			if (--runCount > 0)
				SelectClosestNodeForSeek(ped, testNode, closeDist, farDist, closeNode, (closeNode2 ? closeNode2 : testNode), runCount);
		}
	}
}

bool
CPed::FindBestCoordsFromNodes(CVector unused, CVector *bestCoords)
{
	if (m_pNextPathNode || !bUsePedNodeSeek)
		return false;

	CVector ourPos = GetPosition();

	int closestNodeId = ThePaths.FindNodeClosestToCoors(GetPosition(), 1, 999999.9f);

	CVector seekObjPos = m_vecSeekPos;
	seekObjPos.z += 1.0f;

	if (CWorld::GetIsLineOfSightClear(ourPos, seekObjPos, true, false, false, true, false, false, false))
		return false;

	m_pNextPathNode = nil;

	CVector2D seekPosDist (m_vecSeekPos - ourPos);

	CPathNode *closestNode = &ThePaths.m_pathNodes[closestNodeId];
	CVector2D closeDist(m_vecSeekPos - closestNode->GetPosition());

	SelectClosestNodeForSeek(this, closestNode, closeDist, seekPosDist, closestNode, nil);

	// Above function decided that going to the next node is more logical than seeking the object.
	if (m_pNextPathNode) {

		CVector pathToNextNode = m_pNextPathNode->GetPosition() - ourPos;
		if (pathToNextNode.MagnitudeSqr2D() < seekPosDist.MagnitudeSqr()) {
			*bestCoords = m_pNextPathNode->GetPosition();
			return true;
		}
		m_pNextPathNode = nil;
	}

	return false;
}

bool
CPed::DuckAndCover(void)
{
	if (!m_pedInObjective || CTimer::GetTimeInMilliseconds() <= m_duckAndCoverTimer)
		return false;

	if (bKindaStayInSamePlace){

		if (CTimer::GetTimeInMilliseconds() <= m_leaveCarTimer) {
			if (!m_pLookTarget || m_pLookTarget != m_pedInObjective) {
				m_pLookTarget = m_pedInObjective;
				m_pLookTarget->RegisterReference((CEntity **) &m_pLookTarget);
			}
			if (!bIsAimingGun)
				SetAimFlag(m_pedInObjective);

		} else {
			bCrouchWhenShooting = false;
			bKindaStayInSamePlace = false;
			bIsDucking = false;
			bDuckAndCover = false;
			m_headingRate = 10.0f;
			m_duckAndCoverTimer = CTimer::GetTimeInMilliseconds() + CGeneral::GetRandomNumberInRange(20000,30000);
			if (m_pSeekTarget && m_pSeekTarget->IsVehicle())
				((CVehicle*)m_pSeekTarget)->m_numPedsUseItAsCover--;
		}
		return false;
	}

	bool justDucked = false;
	CVehicle *foundVeh = nil;
	float maxDist = 225.0f;
	bIsDucking = false;
	bCrouchWhenShooting = false;
	if (CTimer::GetTimeInMilliseconds() > m_leaveCarTimer) {
		CVector pos = GetPosition();
		int16 lastVehicle;
		CEntity *vehicles[8];
		CWorld::FindObjectsInRange(pos, CHECK_NEARBY_THINGS_MAX_DIST, true, &lastVehicle, 6, vehicles, false, true, false, false, false);

		for (int i = 0; i < lastVehicle; i++) {
			CVehicle *veh = (CVehicle*) vehicles[i];
			if (veh->m_vecMoveSpeed.Magnitude() <= 0.02f
				&& !veh->bIsBus
				&& !veh->bIsVan
				&& !veh->bIsBig
				&& veh->m_numPedsUseItAsCover < 3) {
				float dist = (GetPosition() - veh->GetPosition()).MagnitudeSqr();
				if (dist < maxDist) {
					maxDist = dist;
					foundVeh = veh;
				}
			}
		}
		if (foundVeh) {
			// Unused.
			// CVector lfWheelPos, rfWheelPos;
			// foundVeh->GetComponentWorldPosition(CAR_WHEEL_RF, rfWheelPos);
			// foundVeh->GetComponentWorldPosition(CAR_WHEEL_LF, lfWheelPos);
			CVector rightSide, leftSide;

			// 3 persons can use the car as cover. Found the correct position for us.
			if (foundVeh->m_numPedsUseItAsCover == 2) {
				rightSide = CVector(1.5f, -0.5f, 0.0f);
				leftSide = CVector(-1.5f, -0.5f, 0.0f);
			} else if (foundVeh->m_numPedsUseItAsCover == 1) {
				rightSide = CVector(1.5f, 0.5f, 0.0f);
				leftSide = CVector(-1.5f, 0.5f, 0.0f);
			} else if (foundVeh->m_numPedsUseItAsCover == 0) {
				rightSide = CVector(1.5f, 0.0f, 0.0f);
				leftSide = CVector(-1.5f, 0.0f, 0.0f);
			}

			CMatrix vehMatrix(foundVeh->GetMatrix());
			CVector duckAtRightSide = Multiply3x3(vehMatrix, rightSide) + foundVeh->GetPosition();

			CVector duckAtLeftSide = Multiply3x3(vehMatrix, leftSide) + foundVeh->GetPosition();

			CVector distWithPedRightSide = m_pedInObjective->GetPosition() - duckAtRightSide;
			CVector distWithPedLeftSide = m_pedInObjective->GetPosition() - duckAtLeftSide;

			CVector duckPos;
			if (distWithPedRightSide.MagnitudeSqr() <= distWithPedLeftSide.MagnitudeSqr())
				duckPos = duckAtLeftSide;
			else
				duckPos = duckAtRightSide;

			if (CWorld::TestSphereAgainstWorld(duckPos, 0.5f, nil, true, true, true, false, false, false)
				&& CWorld::GetIsLineOfSightClear(GetPosition(), duckPos, 1, 0, 0, 1, 0, 0, 0)) {
				SetSeek(duckPos, 1.0f);
				m_headingRate = 15.0f;
				bIsRunning = true;
				bDuckAndCover = true;
				justDucked = true;
				m_leaveCarTimer = CTimer::GetTimeInMilliseconds() + 500;
				if (foundVeh->bIsLawEnforcer)
					m_carInObjective = foundVeh;

				// BUG? Shouldn't we register the reference?
				m_pSeekTarget = foundVeh;
				ClearPointGunAt();
			} else {
				m_duckAndCoverTimer = CTimer::GetTimeInMilliseconds() + CGeneral::GetRandomNumberInRange(10000, 15000);
				bDuckAndCover = false;
			}
		} else {
			bDuckAndCover = false;
		}
	}

	if (!justDucked && !bDuckAndCover)
		return false;
	
	if (!Seek())
		return true;

	bKindaStayInSamePlace = true;
	bDuckAndCover = false;
	m_vecSeekPos = CVector(0.0f, 0.0f, 0.0f);
	if (m_pSeekTarget && m_pSeekTarget->IsVehicle())
		((CVehicle*)m_pSeekTarget)->m_numPedsUseItAsCover++;
	
	SetIdle();
	SetMoveState(PEDMOVE_STILL);
	SetMoveAnim();
	if (!m_pLookTarget || m_pLookTarget != m_pedInObjective) {
		m_pLookTarget = m_pedInObjective;
		m_pLookTarget->RegisterReference((CEntity **) &m_pLookTarget);
	}

	m_leaveCarTimer = CTimer::GetTimeInMilliseconds() + CGeneral::GetRandomNumberInRange(3000, 6000);
	return false;
}

CVector
CPed::GetPositionToOpenCarDoor(CVehicle *veh, uint32 component, float offset)
{	
	CVector doorPos;
	CMatrix vehMat(veh->GetMatrix());

	doorPos = Multiply3x3(vehMat, GetLocalPositionToOpenCarDoor(veh, component, offset));

	return veh->GetPosition() + doorPos;
}

CVector
CPed::GetLocalPositionToOpenCarDoor(CVehicle *veh, uint32 component, float seatPosMult)
{
	CVehicleModelInfo *vehModel; 
	CVector vehDoorPos;
	CVector vehDoorOffset;
	float seatOffset;

	vehModel = (CVehicleModelInfo *)CModelInfo::GetModelInfo(veh->GetModelIndex());
	if (veh->bIsVan && (component == CAR_DOOR_LR || component == CAR_DOOR_RR)) {
		seatOffset = 0.0f;
		vehDoorOffset = vecPedVanRearDoorAnimOffset;
	} else {
		seatOffset = veh->pHandling->fSeatOffsetDistance * seatPosMult;
		if (veh->bLowVehicle) {
			vehDoorOffset = vecPedCarDoorLoAnimOffset;
		} else {
			vehDoorOffset = vecPedCarDoorAnimOffset;
		}
	}

	switch (component) {
		case CAR_DOOR_RF:
			vehDoorPos = vehModel->GetFrontSeatPosn();
			vehDoorPos.x += seatOffset;
			vehDoorOffset.x = -vehDoorOffset.x;
			break;

		case CAR_DOOR_RR:
			vehDoorPos = vehModel->m_positions[CAR_POS_BACKSEAT];
			vehDoorPos.x += seatOffset;
			vehDoorOffset.x = -vehDoorOffset.x;
			break;

		case CAR_DOOR_LF:
			vehDoorPos = vehModel->GetFrontSeatPosn();
			vehDoorPos.x = -(vehDoorPos.x + seatOffset);
			break;

		case CAR_DOOR_LR:
			vehDoorPos = vehModel->m_positions[CAR_POS_BACKSEAT];
			vehDoorPos.x = -(vehDoorPos.x + seatOffset);
			break;

		default:
			vehDoorPos = vehModel->GetFrontSeatPosn();
			vehDoorOffset = CVector(0.0f, 0.0f, 0.0f);
	}
	return vehDoorPos - vehDoorOffset;
}

void
CPed::SetDuck(uint32 time)
{
	if (bIsDucking || CTimer::GetTimeInMilliseconds() <= m_duckTimer)
		return;

	if (bCrouchWhenShooting && (m_nPedState == PED_ATTACK || m_nPedState == PED_AIM_GUN)) {
		CAnimBlendAssociation *duckAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_DUCK_LOW);
		if (!duckAssoc || duckAssoc->blendDelta < 0.0f) {
			CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_DUCK_LOW, 4.0f);
			bIsDucking = true;
			m_duckTimer = CTimer::GetTimeInMilliseconds() + time;
		}
	} else {
		CAnimBlendAssociation *duckAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_DUCK_DOWN);
		if (!duckAssoc || duckAssoc->blendDelta < 0.0f) {
			CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_DUCK_DOWN, 4.0f);
			bIsDucking = true;
			m_duckTimer = CTimer::GetTimeInMilliseconds() + time;
		}
	}
}

void
CPed::Duck(void)
{
	if (CTimer::GetTimeInMilliseconds() > m_duckTimer)
		ClearDuck();
}

void
CPed::ClearDuck(void)
{
	CAnimBlendAssociation *animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_DUCK_DOWN);
	if (!animAssoc) {
		animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_DUCK_LOW);

		if (!animAssoc) {
			bIsDucking = false;
			return;
		}
	}

	if (!bCrouchWhenShooting)
		return;

	if (m_nPedState != PED_ATTACK && m_nPedState != PED_AIM_GUN)
		return;

	animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_RBLOCK_SHOOT);
	if (!animAssoc || animAssoc->blendDelta < 0.0f) {
		CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_RBLOCK_SHOOT, 4.0f);
	}
}

void
CPed::InformMyGangOfAttack(CEntity *attacker)
{
	CPed *attackerPed;

	if (m_objective == OBJECTIVE_KILL_CHAR_ON_FOOT || m_objective == OBJECTIVE_KILL_CHAR_ANY_MEANS)
		return;

	if (attacker->IsPed()) {
		attackerPed = (CPed*)attacker;
	} else {
		if (!attacker->IsVehicle())
			return;

		attackerPed = ((CVehicle*)attacker)->pDriver;
		if (!attackerPed)
			return;
	}

	if (attackerPed->m_nPedType == PEDTYPE_COP)
		return;

	for (int i = 0; i < m_numNearPeds; i++)	{
		CPed *nearPed = m_nearPeds[i];
		if (nearPed && nearPed != this) {
			CPed *leader = nearPed->m_leader;
			if (leader && leader == this && nearPed->m_pedStats->m_fear < nearPed->m_pedStats->m_temper)
			{
				nearPed->SetObjective(OBJECTIVE_KILL_CHAR_ON_FOOT, attackerPed);
				nearPed->SetObjectiveTimer(30000);
			}
		}
	}
}

void
CPed::PedAnimDoorCloseRollingCB(CAnimBlendAssociation* animAssoc, void* arg)
{
	CPed* ped = (CPed*)arg;

	CAutomobile* veh = (CAutomobile*)(ped->m_pMyVehicle);

	if (animAssoc)
		animAssoc->blendDelta = -1000.0f;

	if (veh->bLowVehicle) {
		veh->ProcessOpenDoor(CAR_DOOR_LF, ANIM_STD_CAR_CLOSE_DOOR_ROLLING_LO_LHS, 1.0f);
	} else {
		veh->ProcessOpenDoor(CAR_DOOR_LF, ANIM_STD_CAR_CLOSE_DOOR_ROLLING_LHS, 1.0f);
	}

	veh->m_nGettingOutFlags &= ~CAR_DOOR_FLAG_LF;

	if (veh->Damage.GetDoorStatus(DOOR_FRONT_LEFT) == DOOR_STATUS_SWINGING)
		veh->Damage.SetDoorStatus(DOOR_FRONT_LEFT, DOOR_STATUS_OK);
}

void
CPed::SetSeekBoatPosition(CVehicle *boat)
{
	if (m_nPedState == PED_SEEK_IN_BOAT || boat->pDriver
#if defined VC_PED_PORTS || defined FIX_BUGS
		|| !IsPedInControl()
#endif
		)
		return;

	SetStoredState();
	m_carInObjective = boat;
	m_carInObjective->RegisterReference((CEntity **) &m_carInObjective);
	m_pMyVehicle = boat;
	m_pMyVehicle->RegisterReference((CEntity **) &m_pMyVehicle);
	m_distanceToCountSeekDone = 0.5f;
	SetPedState(PED_SEEK_IN_BOAT);
}

void
CPed::SeekBoatPosition(void)
{
	if (m_carInObjective && !m_carInObjective->pDriver) {
		CVehicleModelInfo *boatModel = m_carInObjective->GetModelInfo();

		CVector enterOffset;
		enterOffset = boatModel->GetFrontSeatPosn();
		enterOffset.x = 0.0f;
		CMatrix boatMat(m_carInObjective->GetMatrix());
		SetMoveState(PEDMOVE_WALK);
		m_vecSeekPos = boatMat * enterOffset;
		if (Seek()) {
			// We arrived to the boat
			m_vehDoor = 0;
			SetEnterCar(m_carInObjective, 0);
		}
	} else
		RestorePreviousState();
}

bool
CPed::IsRoomToBeCarJacked(void)
{
	if (!m_pMyVehicle)
		return false;

	CVector offset;
	if (m_pMyVehicle->bLowVehicle || m_nPedType == PEDTYPE_COP) {
		offset = vecPedDraggedOutCarAnimOffset;
	} else {
		offset = vecPedQuickDraggedOutCarAnimOffset;
	}

	offset.z = 0.0f;
	if (m_pMyVehicle->IsRoomForPedToLeaveCar(CAR_DOOR_LF, &offset)) {
		return true;
	}

	return false;
}

void
CPed::RemoveInCarAnims(void)
{
	if (!IsPlayer())
		return;

	CAnimBlendAssociation *animAssoc;

	if (m_pMyVehicle && m_pMyVehicle->bLowVehicle) {
		animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_CAR_DRIVE_LEFT_LO);
		if (animAssoc)
			animAssoc->blendDelta = -1000.0f;
		animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_CAR_DRIVE_RIGHT_LO);
		if (animAssoc)
			animAssoc->blendDelta = -1000.0f;
		animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_CAR_DRIVEBY_LEFT);
		if (animAssoc)
			animAssoc->blendDelta = -1000.0f;
		animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_CAR_DRIVEBY_RIGHT);
		if (animAssoc)
			animAssoc->blendDelta = -1000.0f;
	} else {
		animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_CAR_DRIVE_LEFT);
		if (animAssoc)
			animAssoc->blendDelta = -1000.0f;
		animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_CAR_DRIVE_RIGHT);
		if (animAssoc)
			animAssoc->blendDelta = -1000.0f;
		animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_CAR_DRIVEBY_LEFT);
		if (animAssoc)
			animAssoc->blendDelta = -1000.0f;
		animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_CAR_DRIVEBY_RIGHT);
		if (animAssoc)
			animAssoc->blendDelta = -1000.0f;
	}

#ifdef VC_PED_PORTS
	animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_BOAT_DRIVE);
	if (animAssoc)
		animAssoc->blendDelta = -1000.0f;
#endif

	animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_CAR_LOOKBEHIND);
	if (animAssoc)
		animAssoc->blendDelta = -1000.0f;
}

bool
CPed::PositionPedOutOfCollision(void)
{
	CVehicle *veh;
	CVector posNearVeh;
	CVector posSomewhereClose;
	bool putNearVeh = false;
	bool putSomewhereClose = false;
	int smallestDistNearVeh = 999;
	int smallestDistSomewhereClose = 999;

	if (!m_pMyVehicle)
		return false;

	CVector vehPos = m_pMyVehicle->GetPosition();
	CVector potentialPos;
	potentialPos.y = GetPosition().y - 3.5f;
	potentialPos.z = GetPosition().z;

	for (int yTry = 0; yTry < 15; yTry++) {
		potentialPos.x = GetPosition().x - 3.5f;

		for (int xTry = 0; xTry < 15; xTry++) {
			CPedPlacement::FindZCoorForPed(&potentialPos);
			CVector distVec = potentialPos - vehPos;
			float dist = distVec.Magnitude();

			// Makes close distances bigger for some reason.
			float mult = (0.6f + dist) / dist;
			CVector adjustedPotentialPos = distVec * mult + vehPos;
			if (CWorld::GetIsLineOfSightClear(vehPos, adjustedPotentialPos, true, false, false, true, false, false, false)
				&& !CWorld::TestSphereAgainstWorld(potentialPos, 0.6f, this, true, false, false, true, false, false)) {

				float potentialChangeSqr = (potentialPos - GetPosition()).MagnitudeSqr();
				veh = (CVehicle*)CWorld::TestSphereAgainstWorld(potentialPos, 0.6f, this, false, true, false, false, false, false);
				if (veh) {
					if (potentialChangeSqr < smallestDistNearVeh) {
						posNearVeh = potentialPos;
						putNearVeh = true;
						smallestDistNearVeh = potentialChangeSqr;
					}
				} else if (potentialChangeSqr < smallestDistSomewhereClose) {
					smallestDistSomewhereClose = potentialChangeSqr;
					posSomewhereClose = potentialPos;
					putSomewhereClose = true;
				}
			}
			potentialPos.x += 0.5f;
		}
		potentialPos.y += 0.5f;
	}

	if (!putSomewhereClose && !putNearVeh)
		return false;

	// We refrain from using posNearVeh, probably because of it may be top of the vehicle.
	if (putSomewhereClose) {
		SetPosition(posSomewhereClose);
	} else {
		CVector vehSize = veh->GetModelInfo()->GetColModel()->boundingBox.max;
		posNearVeh.z += vehSize.z;
		SetPosition(posNearVeh);
	}
	return true;
}

bool
CPed::WarpPedToNearLeaderOffScreen(void)
{
	bool teleported = false;
	if (GetIsOnScreen() || m_leaveCarTimer > CTimer::GetTimeInMilliseconds())
		return false;

	CVector warpToPos = m_leader->GetPosition();
	CVector distVec = warpToPos - GetPosition();
	float halfOfDist = distVec.Magnitude() * 0.5f;
	CVector halfNormalizedDist = distVec / halfOfDist;

	CVector appropriatePos = GetPosition();
	CVector zCorrectedPos = appropriatePos;
	int tryCount = Min(10, halfOfDist);
	for (int i = 0; i < tryCount; ++i) {
		appropriatePos += halfNormalizedDist;
		CPedPlacement::FindZCoorForPed(&zCorrectedPos);

		if (Abs(zCorrectedPos.z - warpToPos.z) >= 3.0f && Abs(zCorrectedPos.z - appropriatePos.z) >= 3.0f)
			continue;

		appropriatePos.z = zCorrectedPos.z;
		if (!TheCamera.IsSphereVisible(appropriatePos, 0.6f, &TheCamera.GetCameraMatrix())
			&& CWorld::GetIsLineOfSightClear(appropriatePos, warpToPos, true, true, false, true, false, false, false)
			&& !CWorld::TestSphereAgainstWorld(appropriatePos, 0.6f, this, true, true, false, true, false, false)) {
			teleported = true;
			Teleport(appropriatePos);
		}
	}
	m_leaveCarTimer = CTimer::GetTimeInMilliseconds() + 3000;
	return teleported;
}

bool
CPed::WarpPedToNearEntityOffScreen(CEntity *warpTo)
{
	bool teleported = false;
	if (GetIsOnScreen() || m_leaveCarTimer > CTimer::GetTimeInMilliseconds())
		return false;

	CVector warpToPos = warpTo->GetPosition();
	CVector distVec = warpToPos - GetPosition();
	float halfOfDist = distVec.Magnitude() * 0.5f;
	CVector halfNormalizedDist = distVec / halfOfDist;

	CVector appropriatePos = GetPosition();
	CVector zCorrectedPos = appropriatePos;
	int tryCount = Min(10, halfOfDist);
	for (int i = 0; i < tryCount; ++i) {
		appropriatePos += halfNormalizedDist;
		CPedPlacement::FindZCoorForPed(&zCorrectedPos);

		if (Abs(zCorrectedPos.z - warpToPos.z) >= 3.0f && Abs(zCorrectedPos.z - appropriatePos.z) >= 3.0f)
			continue;

		appropriatePos.z = zCorrectedPos.z;
		if (!TheCamera.IsSphereVisible(appropriatePos, 0.6f, &TheCamera.GetCameraMatrix())
			&& CWorld::GetIsLineOfSightClear(appropriatePos, warpToPos, true, true, false, true, false, false, false)
			&& !CWorld::TestSphereAgainstWorld(appropriatePos, 0.6f, this, true, true, false, true, false, false)) {
			teleported = true;
			Teleport(appropriatePos);
		}
	}
	m_leaveCarTimer = CTimer::GetTimeInMilliseconds() + 3000;
	return teleported;
}