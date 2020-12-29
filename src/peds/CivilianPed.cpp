#include "common.h"

#include "vehicle_enums.h" //needs to be before Vehicle.h

#include "Vehicle.h"

#include "CarCtrl.h"
#include "CivilianPed.h"
#include "DMAudio.h"
#include "General.h"
#include "Object.h"
#include "PedAttractor.h"
#include "Phones.h"
#include "PlayerPed.h"
#include "SurfaceTable.h"
#include "Wanted.h"
#include "Weather.h"
#include "World.h"

#ifndef _WIN32
#include <float.h>
#endif

// --MIAMI: file done

CCivilianPed::CCivilianPed(ePedType pedtype, uint32 mi) : CPed(pedtype)
{
	SetModelIndex(mi);
	for (int i = 0; i < ARRAY_SIZE(m_nearPeds); i++) {
		m_nearPeds[i] = nil;
	}
	m_bLookForVacantCars = false;
	if (pedtype == PEDTYPE_CRIMINAL)
		m_bLookForVacantCars = true;

	m_nLookForVacantCarsCounter = 0;
	m_bJustStoleACar = false;
	m_bStealCarEvenIfThereIsSomeoneInIt = false;
	for (int i = 0; i < ARRAY_SIZE(m_nStealWishList); i++) {
#ifdef FIX_BUGS
		uint32 randomCarModel = CGeneral::GetRandomNumberInRange(MI_LANDSTAL, MI_LAST_VEHICLE);
#else
		uint32 randomCarModel = CGeneral::GetRandomNumberInRange(MI_LANDSTAL, MI_LANDSTAL + VEHICLEMODELSIZE);
#endif
		if (CModelInfo::IsCarModel(randomCarModel) || CModelInfo::IsBikeModel(randomCarModel))
			m_nStealWishList[i] = randomCarModel;
		else
			m_nStealWishList[i] = MI_CHEETAH;
	}
	m_nAttractorCycleState = 0;
	m_bAttractorUnk = (CGeneral::GetRandomNumberInRange(0.0f, 1.0f) < 1.25f);
}

void
CCivilianPed::CivilianAI(void)
{
	if (CTimer::GetTimeInMilliseconds() <= m_fleeTimer || m_objective != OBJECTIVE_NONE && !bRespondsToThreats
		|| !IsPedInControl())  {

		if (m_objective == OBJECTIVE_GUARD_SPOT)
			return;

		if (m_objective == OBJECTIVE_KILL_CHAR_ON_FOOT || m_objective == OBJECTIVE_KILL_CHAR_ANY_MEANS) {
			if (m_pedInObjective && m_pedInObjective->IsPlayer())
				return;
		}
		if (CTimer::GetTimeInMilliseconds() <= m_lookTimer)
			return;

		ScanForDelayedResponseThreats();
		if (!m_threatFlags || CTimer::GetTimeInMilliseconds() <= m_threatCheckTimer)
			return;
		CheckThreatValidity();
		uint32 closestThreatFlag = m_threatFlags;
		m_threatFlags = 0;
		m_threatCheckTimer = 0;
		if (closestThreatFlag == PED_FLAG_EXPLOSION) {
			float angleToFace = CGeneral::GetRadianAngleBetweenPoints(
				m_eventOrThreat.x,  m_eventOrThreat.y,
				GetPosition().x, GetPosition().y);
			SetLookFlag(angleToFace, true);
			SetLookTimer(500);

		} else if (closestThreatFlag == PED_FLAG_GUN) {
			SetLookFlag(m_threatEntity, true);
			SetLookTimer(500);
		}
		return;
	}
	ScanForDelayedResponseThreats();
	if (!m_threatFlags || CTimer::GetTimeInMilliseconds() <= m_threatCheckTimer)
		return;
	CheckThreatValidity();
	uint32 closestThreatFlag = m_threatFlags;
	m_threatFlags = 0;
	m_threatCheckTimer = 0;
	if (closestThreatFlag == PED_FLAG_GUN) {
		if (!m_threatEntity || !m_threatEntity->IsPed())
			return;

		CPed *threatPed = (CPed*)m_threatEntity;
		float threatDistSqr = (m_threatEntity->GetPosition() - GetPosition()).MagnitudeSqr2D();
		if (CharCreatedBy == MISSION_CHAR && bCrouchWhenScared) {
			SetDuck(10000, true);
			SetLookFlag(m_threatEntity, false);
			SetLookTimer(500);
			return;
		}
		if (m_pedStats->m_fear <= m_pedStats->m_lawfulness) {
			if (m_pedStats->m_temper <= m_pedStats->m_fear) {
				if (!threatPed->IsPlayer() || !RunToReportCrime(CRIME_POSSESSION_GUN)) {
					if (threatDistSqr < sq(30.0f)) {
						bMakeFleeScream = true;
						SetFindPathAndFlee(m_threatEntity, 10000);
					} else {
						SetFindPathAndFlee(m_threatEntity->GetPosition(), 5000, true);
					}
				}
			} else if (m_objective != OBJECTIVE_NONE || GetWeapon()->IsTypeMelee()) {
				SetFindPathAndFlee(m_threatEntity, 5000);
				if (threatDistSqr < sq(20.0f)) {
					SetMoveState(PEDMOVE_RUN);
					bMakeFleeScream = true;
				} else {
					SetMoveState(PEDMOVE_WALK);
				}
			} else if (threatPed->IsPlayer() && IsGangMember() && bCanAttackPlayerWithCops) {
				SetObjective(OBJECTIVE_KILL_CHAR_ON_FOOT, m_threatEntity);

			} else if (threatPed->IsPlayer() && FindPlayerPed()->m_pWanted->m_CurrentCops != 0)  {
				SetFindPathAndFlee(m_threatEntity, 5000);
				if (threatDistSqr < sq(30.0f)) {
					SetMoveState(PEDMOVE_RUN);
				} else {
					SetMoveState(PEDMOVE_WALK);
				}
			} else {
				SetObjective(OBJECTIVE_KILL_CHAR_ON_FOOT, m_threatEntity);
			}
		} else {
			if (threatDistSqr < sq(30.0f)) {
				bMakeFleeScream = true;
				SetFindPathAndFlee(m_threatEntity, 10000);
				SetMoveState(PEDMOVE_SPRINT);
			} else {
				bMakeFleeScream = false;
				SetFindPathAndFlee(m_threatEntity, 5000);
				SetMoveState(PEDMOVE_RUN);
			}
		}
		SetLookFlag(m_threatEntity, false);
		SetLookTimer(500);
	} else if (closestThreatFlag == PED_FLAG_DEADPEDS) {
		float eventDistSqr = (m_pEventEntity->GetPosition() - GetPosition()).MagnitudeSqr2D();
		if (CharCreatedBy == MISSION_CHAR && bCrouchWhenScared && eventDistSqr < sq(5.0f)) {
			SetDuck(10000, true);

		} else if (((CPed*)m_pEventEntity)->bIsDrowning || IsGangMember() && m_nPedType == ((CPed*)m_pEventEntity)->m_nPedType) {
			if (eventDistSqr < sq(5.0f)) {
				SetFindPathAndFlee(m_pEventEntity, 2000);
				SetMoveState(PEDMOVE_RUN);
			}
		} else if (IsGangMember() || eventDistSqr > sq(5.0f)) {
			bool investigateDeadPed = true;
			CEntity *killerOfDeadPed = ((CPed*)m_pEventEntity)->m_threatEntity;
			if (killerOfDeadPed && killerOfDeadPed->IsPed()) {
				CVector killerPos = killerOfDeadPed->GetPosition();
				CVector deadPedPos = m_pEventEntity->GetPosition();
				if (CVector2D(killerPos - deadPedPos).MagnitudeSqr() < sq(10.0f))
					investigateDeadPed = false;
			}

			if (investigateDeadPed)
				SetInvestigateEvent(EVENT_DEAD_PED, CVector2D(m_pEventEntity->GetPosition()), 1.0f, 20000, 0.0f);

		} else {
			SetFindPathAndFlee(m_pEventEntity, 5000);
			SetMoveState(PEDMOVE_RUN);
		}
	} else if (closestThreatFlag == PED_FLAG_EXPLOSION) {
		CVector2D eventDistVec = m_eventOrThreat - GetPosition();
		float eventDistSqr = eventDistVec.MagnitudeSqr();
		if (CharCreatedBy == MISSION_CHAR && bCrouchWhenScared && eventDistSqr < sq(20.0f)) {
			SetDuck(10000, true);

		} else if (eventDistSqr < sq(20.0f)) {
			bMakeFleeScream = true;
			SetFlee(m_eventOrThreat, 2000);
			float angleToFace = CGeneral::GetRadianAngleBetweenPoints(
				m_eventOrThreat.x, m_eventOrThreat.y,
				GetPosition().x, GetPosition().y);
			SetLookFlag(angleToFace, true);
			SetLookTimer(500);
		} else if (eventDistSqr < sq(40.0f)) {
			if (bGonnaInvestigateEvent) {
				if (CharCreatedBy != MISSION_CHAR && !IsGangMember())
					SetInvestigateEvent(EVENT_EXPLOSION, m_eventOrThreat, 6.0f, 30000, 0.0f);

			} else {
				float eventHeading = CGeneral::GetRadianAngleBetweenPoints(eventDistVec.x, eventDistVec.y, 0.0f, 0.0f);
				eventHeading = CGeneral::LimitRadianAngle(eventHeading);
				if (eventHeading < 0.0f)
					eventHeading = eventHeading + TWOPI;

				SetWanderPath(eventHeading / 8.0f);
			}
		}
	} else {
		if (m_threatEntity && m_threatEntity->IsPed()) {
			CPed *threatPed = (CPed*)m_threatEntity;
			if (m_pedStats->m_fear <= 100 - threatPed->m_pedStats->m_temper && threatPed->m_nPedType != PEDTYPE_COP) {
				if (threatPed->GetWeapon()->IsTypeMelee() || !GetWeapon()->IsTypeMelee()) {
					if (threatPed->IsPlayer() && IsGangMember() && bCanAttackPlayerWithCops) {
						SetObjective(OBJECTIVE_KILL_CHAR_ON_FOOT, m_threatEntity);

					} else if (threatPed->IsPlayer() && FindPlayerPed()->m_pWanted->m_CurrentCops != 0) {
						if (m_objective == OBJECTIVE_KILL_CHAR_ON_FOOT || m_objective == OBJECTIVE_KILL_CHAR_ANY_MEANS) {
							SetFindPathAndFlee(m_threatEntity, 10000);
						}
					} else {
						SetObjective(OBJECTIVE_KILL_CHAR_ON_FOOT, m_threatEntity);
					}
				}
			} else {
				SetFindPathAndFlee(m_threatEntity, 10000, true);
			}
		}
	}
}

void
CCivilianPed::ProcessControl(void)
{
	if (CharCreatedBy == UNK_CHAR)
		return;

	CPed::ProcessControl();

	if (bWasPostponed)
		return;

	if (DyingOrDead())
		return;

	GetWeapon()->Update(m_audioEntityId, nil);
	switch (m_nPedState) {
		case PED_WANDER_RANGE:
		case PED_WANDER_PATH:
			if (IsVisible())
				ScanForInterestingStuff();
			break;
		case PED_SEEK_ENTITY:
			if (!m_pSeekTarget) {
				RestorePreviousState();
				break;
			}
			m_vecSeekPos = m_pSeekTarget->GetPosition();

			// fall through
		case PED_SEEK_POS:
			if (Seek()) {
				if ((m_objective == OBJECTIVE_GOTO_AREA_ON_FOOT || m_objective == OBJECTIVE_RUN_TO_AREA || m_objective == OBJECTIVE_SPRINT_TO_AREA ||
					IsUseAttractorObjective(m_objective)) && m_pNextPathNode) {
					m_pNextPathNode = nil;

				} else if (bRunningToPhone) {
					if (gPhoneInfo.m_aPhones[m_phoneId].m_nState != PHONE_STATE_FREE) {
						RestorePreviousState();
						m_phoneId = -1;
					} else {
						gPhoneInfo.m_aPhones[m_phoneId].m_nState = PHONE_STATE_REPORTING_CRIME;
						SetPedState(PED_FACE_PHONE);
					}
				} else if (m_objective != OBJECTIVE_KILL_CHAR_ANY_MEANS && m_objective != OBJECTIVE_KILL_CHAR_ON_FOOT) {
					if (m_pedInObjective && m_objective == OBJECTIVE_FOLLOW_CHAR_IN_FORMATION) {
						if (m_moved.Magnitude() == 0.0f) {
							if (m_pedInObjective->m_nMoveState == PEDMOVE_STILL)
								m_fRotationDest = m_pedInObjective->m_fRotationCur;
						}
					} else if (m_objective == OBJECTIVE_GOTO_CHAR_ON_FOOT
						&& m_pedInObjective && m_pedInObjective->m_nMoveState != PEDMOVE_STILL) {
						SetMoveState(m_pedInObjective->m_nMoveState);
					} else if (m_objective == OBJECTIVE_GOTO_AREA_ON_FOOT || m_objective == OBJECTIVE_RUN_TO_AREA || m_objective == OBJECTIVE_SPRINT_TO_AREA ||
						IsUseAttractorObjective(m_objective)) {
						SetIdle();
					} else {
						RestorePreviousState();
					}
				}
			}
			break;
		case PED_FACE_PHONE:
			if (FacePhone())
				SetPedState(PED_MAKE_CALL);
			break;
		case PED_MAKE_CALL:
			if (MakePhonecall())
				SetWanderPath(CGeneral::GetRandomNumber() & 7);
			break;
		case PED_MUG:
			Mug();
			break;
		case PED_SOLICIT:
			Solicit();
			break;
		case PED_UNKNOWN:
		{
			int pedsInSameState = 0;
			Idle();
			for (int i = 0; i < m_numNearPeds; ++i) {
				CPed *nearPed = m_nearPeds[i];
				if (nearPed->m_nPedType == m_nPedType && nearPed->m_nPedState == PED_UNKNOWN) {
					++pedsInSameState;
				}
			}
			if (pedsInSameState < 5) {
				for (int j = 0; j < m_numNearPeds; ++j) {
					CPed *nearPed = m_nearPeds[j];
					if (nearPed->m_nPedType == m_nPedType && nearPed->m_nPedState == PED_WANDER_PATH) {
						nearPed->SetPedState(PED_UNKNOWN);
					}
				}
			}
			break;
		}
		case PED_DRIVING:
			if (m_nPedType != PEDTYPE_PROSTITUTE)
				break;

			if (CWorld::Players[CWorld::PlayerInFocus].m_pHooker != this)
				break;

			if (CTimer::GetTimeInMilliseconds() > CWorld::Players[CWorld::PlayerInFocus].m_nNextSexFrequencyUpdateTime) {
				if (m_nPedState == PED_DRIVING
					&& m_pMyVehicle->pDriver && m_pMyVehicle->pDriver->IsPlayer() && m_pMyVehicle->pDriver->m_nPedState == PED_DRIVING) {
					CColPoint foundCol;
					CEntity* foundEnt;

					CWorld::ProcessVerticalLine(m_pMyVehicle->GetPosition(), -100.0f,
						foundCol, foundEnt, true, false, false, false, false, false, nil);

					if (m_pMyVehicle->m_vecMoveSpeed.MagnitudeSqr() < sq(0.01f)
						&& foundCol.surfaceB != SURFACE_DEFAULT && foundCol.surfaceB != SURFACE_TARMAC && foundCol.surfaceB != SURFACE_PAVEMENT) {

						if (m_pMyVehicle->CarHasRoof()) {
							m_pMyVehicle->ApplyTurnForce(0.0f, 0.0f, CGeneral::GetRandomNumberInRange(-0.8f, -1.2f) * m_fMass,
								GetPosition().x - m_pMyVehicle->GetPosition().x, GetPosition().y - m_pMyVehicle->GetPosition().y, 0.0f);

							DMAudio.PlayOneShot(m_pMyVehicle->m_audioEntityId, SOUND_CAR_JERK, 0.0f);
							m_pMyVehicle->pDriver->Say(SOUND_PED_PLAYER_BEFORESEX);
							Say(SOUND_PED_PLAYER_BEFORESEX);

							int playerSexFrequency = CWorld::Players[CWorld::PlayerInFocus].m_nSexFrequency;
							if (CWorld::Players[CWorld::PlayerInFocus].m_nMoney >= 10 && playerSexFrequency > 250) {
									CWorld::Players[CWorld::PlayerInFocus].m_nNextSexFrequencyUpdateTime = CTimer::GetTimeInMilliseconds() + playerSexFrequency;
									if (playerSexFrequency >= 350) {
										CWorld::Players[CWorld::PlayerInFocus].m_nSexFrequency = Max(250, playerSexFrequency - 30);
									} else {
										CWorld::Players[CWorld::PlayerInFocus].m_nSexFrequency = Max(250, playerSexFrequency - 10);
									}

									m_pMyVehicle->pDriver->m_fHealth = Min(CWorld::Players[0].m_nMaxHealth + 25.0f, 1.0f + m_pMyVehicle->pDriver->m_fHealth);
									if (CWorld::Players[CWorld::PlayerInFocus].m_nSexFrequency == 250)
										CWorld::Players[CWorld::PlayerInFocus].m_nNextSexFrequencyUpdateTime = CTimer::GetTimeInMilliseconds() + 3000;
							} else {
								bWanderPathAfterExitingCar = true;
								CWorld::Players[CWorld::PlayerInFocus].m_pHooker = nil;
								ClearLeader();
								SetObjective(OBJECTIVE_LEAVE_CAR, m_pMyVehicle);
								m_pMyVehicle->pDriver->Say(SOUND_PED_PLAYER_AFTERSEX);
							}
						} else {
							bWanderPathAfterExitingCar = true;
							CWorld::Players[CWorld::PlayerInFocus].m_pHooker = nil;
							m_pMyVehicle->pDriver->m_fHealth = CWorld::Players[0].m_nMaxHealth + 25.0f;
							ClearLeader();
							SetObjective(OBJECTIVE_LEAVE_CAR, m_pMyVehicle);
							m_pMyVehicle->pDriver->Say(SOUND_PED_PLAYER_AFTERSEX);
						}
					} else {
						CWorld::Players[CWorld::PlayerInFocus].m_nNextSexFrequencyUpdateTime = CTimer::GetTimeInMilliseconds() + 3000;
						int playerSexFrequency = CWorld::Players[CWorld::PlayerInFocus].m_nSexFrequency;
						if (playerSexFrequency >= 1000 || playerSexFrequency <= 250)
							CWorld::Players[CWorld::PlayerInFocus].m_nSexFrequency = 1200;
						else
							CWorld::Players[CWorld::PlayerInFocus].m_nSexFrequency = 250;
					}
				} else {
					bWanderPathAfterExitingCar = true;
					CWorld::Players[CWorld::PlayerInFocus].m_pHooker = nil;
					ClearLeader();
					SetObjective(OBJECTIVE_LEAVE_CAR, m_pMyVehicle);
				}
			}

			if (CTimer::GetTimeInMilliseconds() > CWorld::Players[CWorld::PlayerInFocus].m_nNextSexMoneyUpdateTime) {
				int playerMoney = CWorld::Players[CWorld::PlayerInFocus].m_nMoney;
				if (playerMoney <= 1) {
					CWorld::Players[CWorld::PlayerInFocus].m_nSexFrequency = 250;
				} else {
					CWorld::Players[CWorld::PlayerInFocus].m_nMoney = Max(0, playerMoney - 1);
				}
				CWorld::Players[CWorld::PlayerInFocus].m_nNextSexMoneyUpdateTime = CTimer::GetTimeInMilliseconds() + 1000;
			}
			break;
		default:
			break;
	}
	if (IsPedInControl())
		CivilianAI();

	if (CharCreatedBy == RANDOM_CHAR) {
		EnterVacantNearbyCars();
		UseNearbyAttractors();
	}

	if (CTimer::GetTimeInMilliseconds() > m_timerUnused) {
		m_stateUnused = 0;
		m_timerUnused = 0;
	}

	if (m_moved.Magnitude() > 0.0f)
		Avoid();
}

// --MIAMI: Done
bool
CPed::RunToReportCrime(eCrimeType crimeToReport)
{
	// They changed true into false to make this function unusable. So running to phone actually starts but first frame after that cancels it.
	if (m_nPedState == PED_SEEK_POS)
		return false;

	CVector pos = GetPosition();
	int phoneId = gPhoneInfo.FindNearestFreePhone(&pos);

	if (phoneId == -1)
		return false;

	CPhone* phone = &gPhoneInfo.m_aPhones[phoneId];
	if (phone->m_nState != PHONE_STATE_FREE)
		return false;

	bRunningToPhone = true;
	SetSeek(phone->m_vecPos, 0.3f);
	SetMoveState(PEDMOVE_RUN);
	m_phoneId = phoneId;
	m_crimeToReportOnPhone = crimeToReport;
	return true;
}

const int32 gFrequencyOfAttractorAttempt = 11;
const float gDistanceToSeekAttractors = 50.0f;
const float gMaxDistanceToAttract = 10.0f;

/* Probably this was inlined */
void CCivilianPed::FindNearbyAttractorsSectorList(CPtrList& list, float& minDistance, C2dEffect*& pClosestAttractor, CEntity*& pAttractorEntity)
{
	for (CPtrNode* pNode = list.first; pNode != nil; pNode = pNode->next) {
		CEntity* pEntity = (CEntity*)pNode->item;
		if (pEntity->IsObject() && (!pEntity->GetIsStatic() || ((CObject*)pEntity)->bHasBeenDamaged))
			continue;
		CBaseModelInfo* pModelInfo = CModelInfo::GetModelInfo(pEntity->GetModelIndex());
		for (int i = 0; i < pModelInfo->GetNum2dEffects(); i++) {
			C2dEffect* pEffect = pModelInfo->Get2dEffect(i);
			if (pEffect->type != EFFECT_PED_ATTRACTOR)
				continue;
			if (!IsAttractedTo(pEffect->pedattr.type))
				continue;
			CVector pos;
			CPedAttractorManager::ComputeEffectPos(pEffect, pEntity->GetMatrix(), pos);
			if ((pos - GetPosition()).MagnitudeSqr() < minDistance) {
				CPedAttractorManager* pManager = GetPedAttractorManager();
				if (pManager->HasEmptySlot(pEffect) && pManager->IsApproachable(pEffect, pEntity->GetMatrix(), 0, this)) {
					pClosestAttractor = pEffect;
					pAttractorEntity = pEntity;
					minDistance = (pos - GetPosition()).MagnitudeSqr();
				}
			}
		}
	}
}

void CCivilianPed::UseNearbyAttractors()
{
	if (CWeather::Rain < 0.2f && !m_bAttractorUnk)
		return;
	if (HasAttractor())
		return;
	if (m_nAttractorCycleState != gFrequencyOfAttractorAttempt) {
		m_nAttractorCycleState++;
		return;
	}
	m_nAttractorCycleState = 0;
	if (!IsPedInControl())
		return;
	if (m_nPedState == PED_FLEE_ENTITY)
		return;

	float left = GetPosition().x - gDistanceToSeekAttractors;
	float right = GetPosition().x + gDistanceToSeekAttractors;
	float top = GetPosition().y - gDistanceToSeekAttractors;
	float bottom = GetPosition().y + gDistanceToSeekAttractors;
	int xstart = Max(0, CWorld::GetSectorIndexX(left));
	int xend = Min(NUMSECTORS_X - 1, CWorld::GetSectorIndexX(right));
	int ystart = Max(0, CWorld::GetSectorIndexY(top));
	int yend = Min(NUMSECTORS_Y - 1, CWorld::GetSectorIndexY(bottom));
	assert(xstart <= xend);
	assert(ystart <= yend);

	float minDistance = SQR(gMaxDistanceToAttract);
	C2dEffect* pClosestAttractor = nil;
	CEntity* pAttractorEntity = nil;

	for (int y = ystart; y <= yend; y++) {
		for (int x = xstart; x <= xend; x++) {
			CSector* s = CWorld::GetSector(x, y);
			FindNearbyAttractorsSectorList(s->m_lists[ENTITYLIST_BUILDINGS], minDistance, pClosestAttractor, pAttractorEntity);
			FindNearbyAttractorsSectorList(s->m_lists[ENTITYLIST_OBJECTS], minDistance, pClosestAttractor, pAttractorEntity);
		}
	}
	if (pClosestAttractor)
		GetPedAttractorManager()->RegisterPedWithAttractor(this, pClosestAttractor, pAttractorEntity->GetMatrix());
}

bool CCivilianPed::IsAttractedTo(int8 type)
{
	switch (type) {
	case ATTRACTOR_ATM: return true;
	case ATTRACTOR_SEAT: return true;
	case ATTRACTOR_STOP: return true;
	case ATTRACTOR_PIZZA: return true;
	case ATTRACTOR_SHELTER: return CWeather::Rain >= 0.2f;
	case ATTRACTOR_ICECREAM: return false;
	}
	return false;
}

void
CCivilianPed::EnterVacantNearbyCars(void)
{
	if (!m_bLookForVacantCars)
		return;

	if (m_bJustStoleACar && bInVehicle && m_carInObjective == m_pMyVehicle) {
		m_bJustStoleACar = false;
		m_pMyVehicle->SetStatus(STATUS_PHYSICS);
		m_pMyVehicle->AutoPilot.m_nCarMission = MISSION_CRUISE;
		m_pMyVehicle->AutoPilot.m_nCruiseSpeed = 10;
		m_pMyVehicle->bEngineOn = true;

	} else if (!bHasAlreadyStoleACar) {
		if (m_nLookForVacantCarsCounter == 8) {
			m_nLookForVacantCarsCounter = 0;
			if (IsPedInControl() && m_objective == OBJECTIVE_NONE) {

				CVehicle *foundCar = nil;
				float closestDist = FLT_MAX;
				int minX = CWorld::GetSectorIndexX(GetPosition().x - 10.0f);
				if (minX < 0) minX = 0;
				int minY = CWorld::GetSectorIndexY(GetPosition().y - 10.0f);
				if (minY < 0) minY = 0;
				int maxX = CWorld::GetSectorIndexX(GetPosition().x + 10.0f);
				if (maxX > NUMSECTORS_X - 1) maxX = NUMSECTORS_X - 1;
				int maxY = CWorld::GetSectorIndexY(GetPosition().y + 10.0f);
				if (maxY > NUMSECTORS_Y - 1) maxY = NUMSECTORS_Y - 1;

				for (int curY = minY; curY <= maxY; curY++) {
					for (int curX = minX; curX <= maxX; curX++) {
						CSector* sector = CWorld::GetSector(curX, curY);
						for (CPtrNode* node = sector->m_lists[ENTITYLIST_VEHICLES].first; node; node = node->next) {
							CVehicle* veh = (CVehicle*)node->item;
							if (veh && veh->IsCar()) {

								// Looks like PARKED_VEHICLE condition isn't there in Mobile.
								if (veh->VehicleCreatedBy == RANDOM_VEHICLE || veh->VehicleCreatedBy == PARKED_VEHICLE) {
									if (IsOnStealWishList(veh->GetModelIndex()) && !veh->IsLawEnforcementVehicle()
										&& (m_bStealCarEvenIfThereIsSomeoneInIt || !veh->pDriver && !veh->m_nNumPassengers)
										&& !veh->m_nNumGettingIn && !veh->m_nGettingInFlags && !veh->m_nGettingOutFlags
										&& !veh->m_pCarFire && veh->m_fHealth > 800.0f
										&& !veh->IsUpsideDown() && !veh->IsOnItsSide() && veh->CanPedEnterCar()) {
										float dist = (GetPosition() - veh->GetPosition()).MagnitudeSqr();
										if (dist < sq(10.0f) && dist < closestDist && veh->IsClearToDriveAway()) {
											foundCar = veh;
											closestDist = dist;
										}
									}
								}
							}
						}
					}
				}
				if (foundCar) {
					m_bJustStoleACar = true;
					bHasAlreadyStoleACar = true;
					CCarCtrl::JoinCarWithRoadSystem(foundCar);
					SetObjective(OBJECTIVE_ENTER_CAR_AS_DRIVER, foundCar);
					SetObjectiveTimer(10000);
				}
			}
		} else {
			++m_nLookForVacantCarsCounter;
		}
	}
}

bool
CCivilianPed::IsOnStealWishList(int32 model)
{
	for (int i = 0; i < ARRAY_SIZE(m_nStealWishList); i++) {
		if (model == m_nStealWishList[i]) {
			return true;
		}
	}
	return false;
}