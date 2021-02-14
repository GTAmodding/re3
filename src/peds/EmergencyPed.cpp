#include "common.h"

#include "EmergencyPed.h"
#include "DMAudio.h"
#include "ModelIndices.h"
#include "Vehicle.h"
#include "Fire.h"
#include "General.h"
#include "CarCtrl.h"
#include "Accident.h"

CEmergencyPed::CEmergencyPed(uint32 type) : CPed(type)
{
	switch (type){
		case PEDTYPE_EMERGENCY:
			SetModelIndex(MI_MEDIC);
			m_pRevivedPed = nil;
			field_1360 = 0;
			break;
		case PEDTYPE_FIREMAN:
			SetModelIndex(MI_FIREMAN);
			m_pRevivedPed = nil;
			break;
		default:
			break;
	}
	m_nEmergencyPedState = EMERGENCY_PED_READY;
	m_pAttendedAccident = nil;
	m_bStartedToCPR = false;
}

bool
CEmergencyPed::InRange(CPed *victim)
{
	if (!m_pMyVehicle)
		return true;

	if ((m_pMyVehicle->GetPosition() - victim->GetPosition()).Magnitude() > 30.0f)
		return false;

	return true;
}

void
CEmergencyPed::ProcessControl(void)
{
	if (m_nZoneLevel > LEVEL_GENERIC && m_nZoneLevel != CCollision::ms_collisionInMemory)
		return;

	CPed::ProcessControl();
	if (bWasPostponed)
		return;

	if(!DyingOrDead()) {
		GetWeapon()->Update(m_audioEntityId);

		if (IsPedInControl() && m_moved.Magnitude() > 0.0f)
			Avoid();

		switch (m_nPedState) {
			case PED_SEEK_POS:
				Seek();
				break;
			case PED_SEEK_ENTITY:
				if (m_pSeekTarget) {
					m_vecSeekPos = m_pSeekTarget->GetPosition();
					Seek();
				} else {
					ClearSeek();
				}
				break;
			default:
				break;
		}

		switch (m_nPedType) {
			case PEDTYPE_EMERGENCY:
				if (IsPedInControl() || m_nPedState == PED_DRIVING)
					MedicAI();
				break;
			case PEDTYPE_FIREMAN:
				if (IsPedInControl())
					FiremanAI();
				break;
			default:
				return;
		}
	}
}

// This function was buggy and incomplete in both III and VC, firemen had to be in 5.0m range of fire etc. etc.
// Copied some code from MedicAI to make it work.
void
CEmergencyPed::FiremanAI(void)
{
	float fireDist;
	CFire *nearestFire;

	switch (m_nEmergencyPedState) {
		case EMERGENCY_PED_READY:
			nearestFire = gFireManager.FindNearestFire(GetPosition(), &fireDist);
			if (nearestFire) {
				SetPedState(PED_NONE);
				SetSeek(nearestFire->m_vecPos, 1.0f);
				SetMoveState(PEDMOVE_RUN);
				m_nEmergencyPedState = EMERGENCY_PED_DETERMINE_NEXT_STATE;
				m_pAttendedFire = nearestFire;
#ifdef FIX_BUGS
				bIsRunning = true;
				++nearestFire->m_nFiremenPuttingOut;
#endif
			}
			break;
		case EMERGENCY_PED_DETERMINE_NEXT_STATE:
			nearestFire = gFireManager.FindNearestFire(GetPosition(), &fireDist);
			if (nearestFire && nearestFire != m_pAttendedFire) {
				SetPedState(PED_NONE);
				SetSeek(nearestFire->m_vecPos, 1.0f);
				SetMoveState(PEDMOVE_RUN);
#ifdef FIX_BUGS
				bIsRunning = true;
				if (m_pAttendedFire) {
					--m_pAttendedFire->m_nFiremenPuttingOut;
				}
				++nearestFire->m_nFiremenPuttingOut;
				m_pAttendedFire = nearestFire;
			} else if (!nearestFire) {
#else
				m_pAttendedFire = nearestFire;
			} else {
#endif
				m_nEmergencyPedState = EMERGENCY_PED_STOP;
			}

			// "Extinguish" the fire (Will overwrite the stop decision above if the attended and nearest fires are same)
			if (fireDist < 5.0f) {
				SetIdle();
				m_nEmergencyPedState = EMERGENCY_PED_STAND_STILL;
			}
			break;
		case EMERGENCY_PED_STAND_STILL:
			if (!m_pAttendedFire->m_bIsOngoing)
				m_nEmergencyPedState = EMERGENCY_PED_STOP;

			// Leftover
			// fireDist = 30.0f;
			nearestFire = gFireManager.FindNearestFire(GetPosition(), &fireDist);
			if (nearestFire) {
#ifdef FIX_BUGS
				if(nearestFire != m_pAttendedFire && (nearestFire->m_vecPos - GetPosition()).Magnitude() < 30.0f)
#endif
					m_nEmergencyPedState = EMERGENCY_PED_DETERMINE_NEXT_STATE;
			}
			Say(SOUND_PED_EXTINGUISHING_FIRE);
			break;
		case EMERGENCY_PED_STOP:
#ifdef FIX_BUGS
			bIsRunning = false;
			if (m_pAttendedFire)
#endif
				--m_pAttendedFire->m_nFiremenPuttingOut;

			SetPedState(PED_NONE);
			SetWanderPath(CGeneral::GetRandomNumber() & 7);
			m_pAttendedFire = nil;
			m_nEmergencyPedState = EMERGENCY_PED_READY;
			SetMoveState(PEDMOVE_WALK);
			break;
		default: break;
	}
}

void
CEmergencyPed::MedicAI(void)
{
	float distToEmergency;
	if (!bInVehicle && IsPedInControl()) {
		ScanForThreats();
		if (m_threatEntity && m_threatEntity->IsPed() && ((CPed*)m_threatEntity)->IsPlayer()) {
			if (((CPed*)m_threatEntity)->GetWeapon()->IsTypeMelee()) {
				SetObjective(OBJECTIVE_KILL_CHAR_ON_FOOT, m_threatEntity);
			} else {
				SetFlee(m_threatEntity, 6000);
				Say(SOUND_PED_FLEE_SPRINT);
			}
			return;
		}
	}

	if (InVehicle()) {
		if (m_pMyVehicle->IsCar() && m_objective != OBJECTIVE_LEAVE_CAR) {
			if (gAccidentManager.FindNearestAccident(m_pMyVehicle->GetPosition(), &distToEmergency)
				&& distToEmergency < 25.0f && m_pMyVehicle->m_vecMoveSpeed.Magnitude() < 0.01f) {

				m_pMyVehicle->AutoPilot.m_nCarMission = MISSION_NONE;
				SetObjective(OBJECTIVE_LEAVE_CAR, m_pMyVehicle);
				Say(SOUND_PED_LEAVE_VEHICLE);
			} else if (m_pMyVehicle->pDriver == this && m_nPedState == PED_DRIVING
				&& m_pMyVehicle->AutoPilot.m_nCarMission == MISSION_NONE && !(CGeneral::GetRandomNumber() & 31)) {

				bool waitUntilMedicEntersCar = false;
				for (int i = 0; i < m_numNearPeds; ++i) {
					CPed *nearPed = m_nearPeds[i];
					if (nearPed->m_nPedType == PEDTYPE_EMERGENCY) {
						if ((nearPed->m_nPedState == PED_SEEK_CAR || nearPed->m_nPedState == PED_ENTER_CAR)
							&& nearPed->m_pMyVehicle == m_pMyVehicle) {
							waitUntilMedicEntersCar = true;
							break;
						}
					}
				}
				if (!waitUntilMedicEntersCar) {
					CCarCtrl::JoinCarWithRoadSystem(m_pMyVehicle);
					m_pMyVehicle->AutoPilot.m_nCarMission = MISSION_CRUISE;
					m_pMyVehicle->m_bSirenOrAlarm = 0;
					m_pMyVehicle->AutoPilot.m_nCruiseSpeed = 12;
					m_pMyVehicle->AutoPilot.m_nDrivingStyle = DRIVINGSTYLE_SLOW_DOWN_FOR_CARS;
					if (m_pMyVehicle->bIsAmbulanceOnDuty) {
						m_pMyVehicle->bIsAmbulanceOnDuty = false;
						--CCarCtrl::NumAmbulancesOnDuty;
					}
				}
			}
		}
	}

	CVector headPos, midPos;
	CAccident *nearestAccident;
	if (IsPedInControl()) {
		switch (m_nEmergencyPedState) {
			case EMERGENCY_PED_READY:
				nearestAccident = gAccidentManager.FindNearestAccident(GetPosition(), &distToEmergency);
				field_1360 = 0;
				if (nearestAccident) {
					m_pRevivedPed = nearestAccident->m_pVictim;
					m_pRevivedPed->RegisterReference((CEntity**)&m_pRevivedPed);
					m_pRevivedPed->m_pedIK.GetComponentPosition(midPos, PED_MID);
					m_pRevivedPed->m_pedIK.GetComponentPosition(headPos, PED_HEAD);
					SetSeek((headPos + midPos) * 0.5f, 1.0f);
					SetObjective(OBJECTIVE_NONE);
					bIsRunning = true;
					m_nEmergencyPedState = EMERGENCY_PED_DETERMINE_NEXT_STATE;
					m_pAttendedAccident = nearestAccident;
					++m_pAttendedAccident->m_nMedicsAttending;
				} else {
					if (m_pMyVehicle) {
						if (!bInVehicle) {
							if (m_objective == OBJECTIVE_ENTER_CAR_AS_DRIVER || m_pMyVehicle->pDriver || m_pMyVehicle->m_nGettingInFlags) {

								CPed* driver = m_pMyVehicle->pDriver;
								if (driver && driver->m_nPedType != PEDTYPE_EMERGENCY && m_objective != OBJECTIVE_KILL_CHAR_ON_FOOT) {
									SetObjective(OBJECTIVE_KILL_CHAR_ON_FOOT, driver);
								} else if (m_objective != OBJECTIVE_ENTER_CAR_AS_DRIVER
									&& m_objective != OBJECTIVE_ENTER_CAR_AS_PASSENGER
									&& m_objective != OBJECTIVE_KILL_CHAR_ON_FOOT) {
									SetObjective(OBJECTIVE_ENTER_CAR_AS_PASSENGER, m_pMyVehicle);
								}
							} else {
								SetObjective(OBJECTIVE_ENTER_CAR_AS_DRIVER, m_pMyVehicle);
							}
						}
					} else if (m_nPedState != PED_WANDER_PATH) {
						SetWanderPath(CGeneral::GetRandomNumber() & 7);
					}
				}
				break;
			case EMERGENCY_PED_DETERMINE_NEXT_STATE:
				nearestAccident = gAccidentManager.FindNearestAccident(GetPosition(), &distToEmergency);
				if (nearestAccident) {
					if (nearestAccident != m_pAttendedAccident || m_nPedState != PED_SEEK_POS) {
						m_pRevivedPed = nearestAccident->m_pVictim;
						m_pRevivedPed->RegisterReference((CEntity**)&m_pRevivedPed);
						if (!InRange(m_pRevivedPed)) {
							m_nEmergencyPedState = EMERGENCY_PED_STOP;
							break;
						}
						m_pRevivedPed->m_pedIK.GetComponentPosition(midPos, PED_MID);
						m_pRevivedPed->m_pedIK.GetComponentPosition(headPos, PED_HEAD);
						SetSeek((headPos + midPos) * 0.5f, nearestAccident->m_nMedicsPerformingCPR * 0.5f + 1.0f);
						SetObjective(OBJECTIVE_NONE);
						bIsRunning = true;
						--m_pAttendedAccident->m_nMedicsAttending;
						++nearestAccident->m_nMedicsAttending;
						m_pAttendedAccident = nearestAccident;
					}
				} else {
					m_nEmergencyPedState = EMERGENCY_PED_STOP;
					bIsRunning = false;
				}
				if (distToEmergency < 5.0f) {
					if (m_pRevivedPed->m_pFire) {
						bIsRunning = false;
						SetMoveState(PEDMOVE_STILL);
					} else if (distToEmergency < 4.5f) {
						bIsRunning = false;
						SetMoveState(PEDMOVE_WALK);
						if (distToEmergency < 1.0f
							|| distToEmergency < 4.5f && m_pAttendedAccident->m_nMedicsPerformingCPR) {
							m_nEmergencyPedState = EMERGENCY_PED_START_CPR;
						}
					}
				}
				break;
			case EMERGENCY_PED_START_CPR:
				if (!m_pRevivedPed || m_pRevivedPed->m_fHealth > 0.0f || m_pRevivedPed->bFadeOut) {
					m_nEmergencyPedState = EMERGENCY_PED_DETERMINE_NEXT_STATE;
				} else {
					m_pRevivedPed->m_bloodyFootprintCountOrDeathTime = CTimer::GetTimeInMilliseconds();
					SetMoveState(PEDMOVE_STILL);
					SetPedState(PED_CPR);
					m_nLastPedState = PED_CPR;
					SetLookFlag(m_pRevivedPed, 0);
					SetLookTimer(500);
					Say(SOUND_PED_HEALING);
					if (m_pAttendedAccident->m_nMedicsPerformingCPR) {
						SetIdle();
						m_nEmergencyPedState = EMERGENCY_PED_STAND_STILL;
					} else {
						m_nEmergencyPedState = EMERGENCY_PED_FACE_TO_PATIENT;
						m_pVehicleAnim = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_MEDIC_CPR, 4.0f);
						bIsDucking = true;
					}
					SetLookTimer(2000);
					++m_pAttendedAccident->m_nMedicsPerformingCPR;
					m_bStartedToCPR = true;
				}
				break;
			case EMERGENCY_PED_FACE_TO_PATIENT:
				if (!m_pRevivedPed || m_pRevivedPed->m_fHealth > 0.0f)
					m_nEmergencyPedState = EMERGENCY_PED_DETERMINE_NEXT_STATE;
				else {
					m_pRevivedPed->m_pedIK.GetComponentPosition(midPos, PED_MID);
					m_pRevivedPed->m_pedIK.GetComponentPosition(headPos, PED_HEAD);
					midPos = (headPos + midPos) * 0.5f;
					m_fRotationDest = CGeneral::GetRadianAngleBetweenPoints(
						midPos.x, midPos.y,
						GetPosition().x, GetPosition().y);
					m_fRotationDest = CGeneral::LimitAngle(m_fRotationDest);
					m_pLookTarget = m_pRevivedPed;
					m_pLookTarget->RegisterReference((CEntity**)&m_pLookTarget);
					TurnBody();

					if (Abs(m_fRotationCur - m_fRotationDest) < DEGTORAD(45.0f))
						m_nEmergencyPedState = EMERGENCY_PED_PERFORM_CPR;
					else
						m_fRotationCur = (m_fRotationCur + m_fRotationDest) * 0.5f;
				}
				break;
			case EMERGENCY_PED_PERFORM_CPR:
				if (!m_pRevivedPed || m_pRevivedPed->m_fHealth > 0.0f) {
					m_nEmergencyPedState = EMERGENCY_PED_DETERMINE_NEXT_STATE;
					break;
				}
				m_pRevivedPed->m_pedIK.GetComponentPosition(midPos, PED_MID);
				m_pRevivedPed->m_pedIK.GetComponentPosition(headPos, PED_HEAD);
				midPos = (headPos + midPos) * 0.5f;
				m_fRotationDest = CGeneral::GetRadianAngleBetweenPoints(
					midPos.x, midPos.y,
					GetPosition().x, GetPosition().y);
				m_fRotationDest = CGeneral::LimitAngle(m_fRotationDest);
				m_pLookTarget = m_pRevivedPed;
				m_pLookTarget->RegisterReference((CEntity**)&m_pLookTarget);
				TurnBody();
				if (CTimer::GetTimeInMilliseconds() <= m_lookTimer) {
					SetMoveState(PEDMOVE_STILL);
					break;
				}
				m_nEmergencyPedState = EMERGENCY_PED_STOP_CPR;
				SetPedState(PED_NONE);
				SetMoveState(PEDMOVE_WALK);
				m_pVehicleAnim = nil;
				if (!m_pRevivedPed->bBodyPartJustCameOff) {
					m_pRevivedPed->m_fHealth = 100.0f;
					m_pRevivedPed->SetPedState(PED_NONE);
					m_pRevivedPed->m_nLastPedState = PED_WANDER_PATH;
					m_pRevivedPed->SetGetUp();
					m_pRevivedPed->bUsesCollision = true;
					m_pRevivedPed->SetMoveState(PEDMOVE_WALK);
					m_pRevivedPed->RestartNonPartialAnims();
					m_pRevivedPed->bIsPedDieAnimPlaying = false;
					m_pRevivedPed->bKnockedUpIntoAir = false;
					m_pRevivedPed->m_pCollidingEntity = nil;
				}
				break;
			case EMERGENCY_PED_STOP_CPR:
				m_nEmergencyPedState = EMERGENCY_PED_STOP;
				bIsDucking = true;
				break;
			case EMERGENCY_PED_STAND_STILL:
				if (!m_pRevivedPed || m_pRevivedPed->m_fHealth > 0.0f)
					m_nEmergencyPedState = EMERGENCY_PED_DETERMINE_NEXT_STATE;
				else {
					if (!m_pAttendedAccident->m_pVictim)
						m_nEmergencyPedState = EMERGENCY_PED_DETERMINE_NEXT_STATE;
					if (!m_pAttendedAccident->m_nMedicsPerformingCPR)
						m_nEmergencyPedState = EMERGENCY_PED_DETERMINE_NEXT_STATE;
					if (gAccidentManager.UnattendedAccidents())
						m_nEmergencyPedState = EMERGENCY_PED_DETERMINE_NEXT_STATE;
				}
				break;
			case EMERGENCY_PED_STOP:
				m_bStartedToCPR = false;
				SetPedState(PED_NONE);
				if (m_pAttendedAccident) {
					m_pAttendedAccident->m_pVictim = nil;
					--m_pAttendedAccident->m_nMedicsAttending;
					m_pAttendedAccident = nil;
				}
				SetWanderPath(CGeneral::GetRandomNumber() & 7);
				m_pRevivedPed = nil;
				m_nEmergencyPedState = EMERGENCY_PED_READY;
				SetMoveState(PEDMOVE_WALK);
				break;
			default: break;
		}
	}
}
