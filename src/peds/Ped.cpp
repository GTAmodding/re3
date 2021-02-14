#include "common.h"

#include "main.h"
#include "Pools.h"
#include "Particle.h"
#include "RpAnimBlend.h"
#include "Bones.h"
#include "Ped.h"
#include "AnimBlendAssociation.h"
#include "Fire.h"
#include "DMAudio.h"
#include "General.h"
#include "VisibilityPlugins.h"
#include "HandlingMgr.h"
#include "Replay.h"
#include "Radar.h"
#include "PedPlacement.h"
#include "Shadows.h"
#include "Weather.h"
#include "ZoneCull.h"
#include "Population.h"
#include "Pad.h"
#include "Phones.h"
#include "TrafficLights.h"
#include "CopPed.h"
#include "Script.h"
#include "CarCtrl.h"
#include "Garages.h"
#include "WaterLevel.h"
#include "Timecycle.h"
#include "ParticleObject.h"
#include "Floater.h"
#include "Range2D.h"
#include "Wanted.h"

CPed *gapTempPedList[50];
uint16 gnNumTempPedList;

static CColPoint aTempPedColPts[MAX_COLLISION_POINTS];


uint16 CPed::nThreatReactionRangeMultiplier = 1;
uint16 CPed::nEnterCarRangeMultiplier = 1;

bool CPed::bNastyLimbsCheat;
bool CPed::bPedCheat2;
bool CPed::bPedCheat3;
CVector2D CPed::ms_vec2DFleePosition;

void *CPed::operator new(size_t sz) { return CPools::GetPedPool()->New();  }
void *CPed::operator new(size_t sz, int handle) { return CPools::GetPedPool()->New(handle); }
void CPed::operator delete(void *p, size_t sz) { CPools::GetPedPool()->Delete((CPed*)p); }
void CPed::operator delete(void *p, int handle) { CPools::GetPedPool()->Delete((CPed*)p); }

#ifdef DEBUGMENU
bool CPed::bPopHeadsOnHeadshot = false;
#endif

CPed::CPed(uint32 pedType) : m_pedIK(this)
{
	m_type = ENTITY_TYPE_PED;
	bPedPhysics = true;
	bUseCollisionRecords = true;

	m_vecAnimMoveDelta.x = 0.0f;
	m_vecAnimMoveDelta.y = 0.0f;
	m_fHealth = 100.0f;
	m_fArmour = 0.0f;
	m_nPedType = pedType;
	m_lastSoundStart = 0;
	m_soundStart = 0;
	m_lastQueuedSound = SOUND_NO_SOUND;
	m_queuedSound = SOUND_NO_SOUND;
	m_objective = OBJECTIVE_NONE;
	m_prevObjective = OBJECTIVE_NONE;
#ifdef FIX_BUGS
	m_objectiveTimer = 0;
#endif
	CharCreatedBy = RANDOM_CHAR;
	m_leader = nil;
	m_pedInObjective = nil;
	m_carInObjective = nil;
	bInVehicle = false;
	m_pMyVehicle = nil;
	m_pVehicleAnim = nil;
	m_vecOffsetSeek.x = 0.0f;
	m_vecOffsetSeek.y = 0.0f;
	m_vecOffsetSeek.z = 0.0f;
	m_pedFormation = FORMATION_UNDEFINED;
	m_collidingThingTimer = 0;
	m_nPedStateTimer = 0;
	m_actionX = 0.0f;
	m_actionY = 0.0f;
	m_phoneTalkTimer = 0;
	m_stateUnused = 0;
	m_leaveCarTimer = 0;
	m_getUpTimer = 0;
	m_attackTimer = 0;
	m_timerUnused = 0;
	m_lookTimer = 0;
	m_chatTimer = 0;
	m_shootTimer = 0;
	m_carJackTimer = 0;
	m_duckAndCoverTimer = 0;
	m_moved = CVector2D(0.0f, 0.0f);
	m_fRotationCur = 0.0f;
	m_headingRate = 15.0f;
	m_fRotationDest = 0.0f;
	m_vehDoor = CAR_DOOR_LF;
	m_walkAroundType = 0;
	m_pCurrentPhysSurface = nil;
	m_vecOffsetFromPhysSurface = CVector(0.0f, 0.0f, 0.0f);
	m_pSeekTarget = nil;
	m_vecSeekPos = CVector(0.0f, 0.0f, 0.0f);
	m_wepSkills = 0;
	m_distanceToCountSeekDone = 1.0f;
	bRunningToPhone = false;
	m_phoneId = -1;
	m_lastAccident = 0;
	m_fleeFrom = nil;
	m_fleeFromPosX = 0;
	m_fleeFromPosY = 0;
	m_fleeTimer = 0;
	m_vecSeekPosEx = CVector(0.0f, 0.0f, 0.0f);
	m_distanceToCountSeekDoneEx = 0.0f;
	m_nWaitState = WAITSTATE_FALSE;
	m_nWaitTimer = 0;
	m_pCollidingEntity = nil;
	m_nPedState = PED_IDLE;
	m_nLastPedState = PED_NONE;
	m_nMoveState = PEDMOVE_STILL;
#ifdef FIX_BUGS
	m_nPrevMoveState = PEDMOVE_NONE;
#endif
	m_nStoredMoveState = PEDMOVE_NONE;
	m_pFire = nil;
	m_pPointGunAt = nil;
	m_pLookTarget = nil;
	m_fLookDirection = 0.0f;
	m_pCurSurface = nil;
	m_wanderRangeBounds = nil;
	m_nPathNodes = 0;
	m_nCurPathNode = 0;
	m_nPathDir = 0;
	m_pLastPathNode = nil;
	m_pNextPathNode = nil;
	m_routeLastPoint = -1;
	m_routeStartPoint = 0;
	m_routePointsPassed = 0;
	m_routeType = 0;
	m_bodyPartBleeding = -1;

	m_fMass = 70.0f;
	m_fTurnMass = 100.0f;
	m_fAirResistance = 0.4f / m_fMass;
	m_fElasticity = 0.05f;

	bIsStanding = false;
	bWasStanding = false;
	bIsAttacking = false;
	bIsPointingGunAt = false;
	bIsLooking = false;
	bKeepTryingToLook = false;
	bIsRestoringLook = false;
	bIsAimingGun = false;

	bIsRestoringGun = false;
	bCanPointGunAtTarget = false;
	bIsTalking = false;
	bIsInTheAir = false;
	bIsLanding = false;
	bIsRunning = false;
	bHitSomethingLastFrame = false;
	bVehEnterDoorIsBlocked = false;

	bCanPedEnterSeekedCar = false;
	bRespondsToThreats = true;
	bRenderPedInCar = true;
	bChangedSeat = false;
	bUpdateAnimHeading = false;
	bBodyPartJustCameOff = false;
	bIsShooting = false;
	bFindNewNodeAfterStateRestore = false;

	bGonnaInvestigateEvent = false;
	bPedIsBleeding = false;
	bStopAndShoot = false;
	bIsPedDieAnimPlaying = false;
	bUsePedNodeSeek = false;
	bObjectiveCompleted = false;
	bScriptObjectiveCompleted = false;

	bKindaStayInSamePlace = false;
	bBeingChasedByPolice = false;
	bNotAllowedToDuck = false;
	bCrouchWhenShooting = false;
	bIsDucking = false;
	bGetUpAnimStarted = false;
	bDoBloodyFootprints = false;
	bFleeAfterExitingCar = false;

	bWanderPathAfterExitingCar = false;
	bIsLeader = false;
	bDontDragMeOutCar = false;
	m_ped_flagF8 = false;
	bWillBeQuickJacked = false;
	bCancelEnteringCar = false;
	bObstacleShowedUpDuringKillObjective = false;
	bDuckAndCover = false;

	bStillOnValidPoly = false;
	bAllowMedicsToReviveMe = true;
	bResetWalkAnims = false;
	bStartWanderPathOnFoot = false;
	bOnBoat = false;
	bBusJacked = false;
	bGonnaKillTheCarJacker = false;
	bFadeOut = false;

	bKnockedUpIntoAir = false;
	bHitSteepSlope = false;
	bCullExtraFarAway = false;
	bClearObjective = false;
	bTryingToReachDryLand = false;
	bCollidedWithMyVehicle = false;
	bRichFromMugging = false;
	bChrisCriminal = false;

	bShakeFist = false;
	bNoCriticalHits = false;
	bVehExitWillBeInstant = false;
	bHasAlreadyBeenRecorded = false;
	bFallenDown = false;
#ifdef KANGAROO_CHEAT
	m_ped_flagI80 = false;
#endif
#ifdef VC_PED_PORTS
	bSomeVCflag1 = false;
#endif

	if (CGeneral::GetRandomNumber() & 3)
		bHasACamera = false;
	else
		bHasACamera = true;

	m_audioEntityId = DMAudio.CreateEntity(AUDIOTYPE_PHYSICAL, this);
	DMAudio.SetEntityStatus(m_audioEntityId, true);
	m_fearFlags = CPedType::GetThreats(m_nPedType);
	m_threatEntity = nil;
	m_eventOrThreat = CVector2D(0.0f, 0.0f);
	m_pEventEntity = nil;
	m_fAngleToEvent = 0.0f;
	m_numNearPeds = 0;

	for (int i = 0; i < ARRAY_SIZE(m_nearPeds); i++) {
		m_nearPeds[i] = nil;
		if (i < ARRAY_SIZE(m_pPathNodesStates)) {
			m_pPathNodesStates[i] = nil;
		}
	}
	m_maxWeaponTypeAllowed = WEAPONTYPE_UNARMED;
	m_currentWeapon = WEAPONTYPE_UNARMED;
	m_storedWeapon = WEAPONTYPE_UNIDENTIFIED;

	for(int i = 0; i < WEAPONTYPE_TOTAL_INVENTORY_WEAPONS; i++) {
		CWeapon &weapon = GetWeapon(i);
		weapon.m_eWeaponType = WEAPONTYPE_UNARMED;
		weapon.m_eWeaponState = WEAPONSTATE_READY;
		weapon.m_nAmmoInClip = 0;
		weapon.m_nAmmoTotal = 0;
		weapon.m_nTimer = 0;
	}

	m_curFightMove = FIGHTMOVE_NULL;
	GiveWeapon(WEAPONTYPE_UNARMED, 0);
	m_wepAccuracy = 60;
	m_lastWepDam = -1;
	m_collPoly.valid = false;
	m_fCollisionSpeed = 0.0f;
	m_wepModelID = -1;
#ifdef PED_SKIN
	m_pWeaponModel = nil;
#endif
	CPopulation::UpdatePedCount((ePedType)m_nPedType, false);
}

CPed::~CPed(void)
{
	CWorld::Remove(this);
	CRadar::ClearBlipForEntity(BLIP_CHAR, CPools::GetPedPool()->GetIndex(this));
	if (InVehicle()){
		uint8 door_flag = GetCarDoorFlag(m_vehDoor);
		if (m_pMyVehicle->pDriver == this)
			m_pMyVehicle->pDriver = nil;
		else {
			// FIX: Passenger counter now decreasing after removing ourself from vehicle.
			m_pMyVehicle->RemovePassenger(this);
		}
		if (m_nPedState == PED_EXIT_CAR || m_nPedState == PED_DRAG_FROM_CAR)
			m_pMyVehicle->m_nGettingOutFlags &= ~door_flag;
		bInVehicle = false;
		m_pMyVehicle = nil;
	} else if (EnteringCar()) {
		QuitEnteringCar();
	}
	if (m_pFire)
		m_pFire->Extinguish();
	CPopulation::UpdatePedCount((ePedType)m_nPedType, true);
	DMAudio.DestroyEntity(m_audioEntityId);
}

void
CPed::Initialise(void)
{
	debug("Initialising CPed...\n");
	CPedType::Initialise();
	LoadFightData();
	SetAnimOffsetForEnterOrExitVehicle();
	debug("CPed ready\n");
}

void
CPed::SetModelIndex(uint32 mi)
{
	CEntity::SetModelIndex(mi);
	RpAnimBlendClumpInit(GetClump());
	RpAnimBlendClumpFillFrameArray(GetClump(), m_pFrames);
	CPedModelInfo *modelInfo = (CPedModelInfo *)CModelInfo::GetModelInfo(GetModelIndex());
	SetPedStats(modelInfo->m_pedStatType);
	m_headingRate = m_pedStats->m_headingChangeRate;
	m_animGroup = (AssocGroupId) modelInfo->m_animGroup;
	CAnimManager::AddAnimation(GetClump(), m_animGroup, ANIM_STD_IDLE);

	(*RPANIMBLENDCLUMPDATA(m_rwObject))->velocity2d = &m_vecAnimMoveDelta;

#ifdef PED_SKIN
	if(modelInfo->GetHitColModel() == nil)
		modelInfo->CreateHitColModelSkinned(GetClump());
#endif
}

void
CPed::SetPedStats(ePedStats pedStat)
{
	m_pedStats = CPedStats::ms_apPedStats[pedStat];
}

void
CPed::BuildPedLists(void)
{
	if (((CTimer::GetFrameCounter() + m_randomSeed) % 16) == 0) {
		CVector centre = CEntity::GetBoundCentre();
		CRect rect(centre.x - 20.0f,
			centre.y - 20.0f,
			centre.x + 20.0f,
			centre.y + 20.0f);
		int xstart = CWorld::GetSectorIndexX(rect.left);
		int ystart = CWorld::GetSectorIndexY(rect.top);
		int xend = CWorld::GetSectorIndexX(rect.right);
		int yend = CWorld::GetSectorIndexY(rect.bottom);
		gnNumTempPedList = 0;

		for(int y = ystart; y <= yend; y++) {
			for(int x = xstart; x <= xend; x++) {
				for (CPtrNode *pedPtrNode = CWorld::GetSector(x,y)->m_lists[ENTITYLIST_PEDS].first; pedPtrNode; pedPtrNode = pedPtrNode->next) {
					CPed *ped = (CPed*)pedPtrNode->item;
					if (ped != this && !ped->bInVehicle) {
						float dist = (ped->GetPosition() - GetPosition()).Magnitude2D();
						if (nThreatReactionRangeMultiplier * 30.0f > dist) {
							gapTempPedList[gnNumTempPedList] = ped;
							gnNumTempPedList++;
							assert(gnNumTempPedList < ARRAY_SIZE(gapTempPedList));
						}
					}
				}
			}
		}
		gapTempPedList[gnNumTempPedList] = nil;
		SortPeds(gapTempPedList, 0, gnNumTempPedList - 1);
		for (m_numNearPeds = 0; m_numNearPeds < ARRAY_SIZE(m_nearPeds); m_numNearPeds++) {
			CPed *ped = gapTempPedList[m_numNearPeds];
			if (!ped)
				break;

			m_nearPeds[m_numNearPeds] = ped;
		}
		for (int pedToClear = m_numNearPeds; pedToClear < ARRAY_SIZE(m_nearPeds); pedToClear++)
			m_nearPeds[pedToClear] = nil;
	} else {
		for(int i = 0; i < ARRAY_SIZE(m_nearPeds); ) {
			bool removePed = false;
			if (m_nearPeds[i]) {
				if (m_nearPeds[i]->IsPointerValid()) {
					float distSqr = (GetPosition() - m_nearPeds[i]->GetPosition()).MagnitudeSqr2D();
					if (distSqr > 900.0f)
						removePed = true;
				} else
					removePed = true;
			}
			if (removePed) {
				// If we arrive here, the ped we're checking isn't "near", so we should remove it.
				for (int j = i; j < ARRAY_SIZE(m_nearPeds) - 1; j++) {
					m_nearPeds[j] = m_nearPeds[j + 1];
					m_nearPeds[j + 1] = nil;
				}
				// Above loop won't work on last slot, so we need to empty it.
				m_nearPeds[ARRAY_SIZE(m_nearPeds) - 1] = nil;
				m_numNearPeds--;	
			} else
				i++;
		}
	}
}

bool
CPed::OurPedCanSeeThisOne(CEntity *target)
{
	CColPoint colpoint;
	CEntity *ent;

	CVector2D dist = CVector2D(target->GetPosition()) - CVector2D(GetPosition());

	// Check if target is behind ped
	if (DotProduct2D(dist, CVector2D(GetForward())) < 0.0f)
		return false;

	// Check if target is too far away
	if (dist.Magnitude() >= 40.0f)
		return false;

	// Check line of sight from head
	CVector headPos = this->GetPosition();
	headPos.z += 1.0f;
	return !CWorld::ProcessLineOfSight(headPos, target->GetPosition(), colpoint, ent, true, false, false, false, false, false);
}

// Some kind of binary sort
void
CPed::SortPeds(CPed **list, int min, int max)
{
	if (min >= max)
		return;

	CVector leftDiff, rightDiff;
	CVector middleDiff = GetPosition() - list[(max + min) / 2]->GetPosition();
	float middleDist = middleDiff.Magnitude();

	int left = max;
	int right = min;
	while(right <= left){
		float rightDist, leftDist;
		do {
			rightDiff = GetPosition() - list[right]->GetPosition();
			rightDist = rightDiff.Magnitude();
		} while (middleDist > rightDist && ++right);

		do {
			leftDiff = GetPosition() - list[left]->GetPosition();
			leftDist = leftDiff.Magnitude();
		} while (middleDist < leftDist && left--);

		if (right <= left) {
			CPed *ped = list[right];
			list[right] = list[left];
			list[left] = ped;
			right++;
			left--;
		}
	}
	SortPeds(list, min, left);
	SortPeds(list, right, max);
}

void
CPed::SetMoveState(eMoveState state)
{
	m_nMoveState = state;
}

void
CPed::SetMoveAnim(void)
{
	if (m_nStoredMoveState == m_nMoveState || !IsPedInControl())
		return;

	if (m_nMoveState == PEDMOVE_NONE) {
		m_nStoredMoveState = PEDMOVE_NONE;
		return;
	}

	AssocGroupId animGroupToUse;
	if (m_leader && m_leader->IsPlayer())
		animGroupToUse = ASSOCGRP_PLAYER;
	else
		animGroupToUse = m_animGroup;

	CAnimBlendAssociation *animAssoc = RpAnimBlendClumpGetFirstAssociation(GetClump(), ASSOC_BLOCK);
	if (!animAssoc) {
		CAnimBlendAssociation *fightIdleAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_FIGHT_IDLE);
		animAssoc = fightIdleAssoc;
		if (fightIdleAssoc && m_nPedState == PED_FIGHT)
			return;

		if (fightIdleAssoc) {
			CAnimBlendAssociation *idleAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_IDLE);
			if (!idleAssoc || idleAssoc->blendDelta <= 0.0f) {
				animAssoc->flags |= ASSOC_DELETEFADEDOUT;
				animAssoc = CAnimManager::BlendAnimation(GetClump(), animGroupToUse, ANIM_STD_IDLE, 8.0f);
			}
		}
	}
	if (!animAssoc) {
		animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_IDLE_TIRED);
		if (animAssoc)
			if (m_nWaitState == WAITSTATE_STUCK || m_nWaitState == WAITSTATE_FINISH_FLEE)
				return;

		if (animAssoc) {
			CAnimBlendAssociation *idleAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_IDLE);
			if (!idleAssoc || idleAssoc->blendDelta <= 0.0f) {
				animAssoc->flags |= ASSOC_DELETEFADEDOUT;
				animAssoc = CAnimManager::BlendAnimation(GetClump(), animGroupToUse, ANIM_STD_IDLE, 4.0f);
			}
		}
	}
	if (!animAssoc) {
		m_nStoredMoveState = m_nMoveState;
		if (m_nMoveState == PEDMOVE_WALK || m_nMoveState == PEDMOVE_RUN || m_nMoveState == PEDMOVE_SPRINT) {
			for (CAnimBlendAssociation *assoc = RpAnimBlendClumpGetFirstAssociation(GetClump(), ASSOC_PARTIAL);
				assoc; assoc = RpAnimBlendGetNextAssociation(assoc, ASSOC_PARTIAL)) {

				if (!(assoc->flags & ASSOC_FADEOUTWHENDONE)) {
					assoc->blendDelta = -2.0f;
					assoc->flags |= ASSOC_DELETEFADEDOUT;
				}
			}

			ClearAimFlag();
			ClearLookFlag();
		}

		switch (m_nMoveState) {
			case PEDMOVE_STILL:
				animAssoc = CAnimManager::BlendAnimation(GetClump(), animGroupToUse, ANIM_STD_IDLE, 4.0f);
				break;
			case PEDMOVE_WALK:
				animAssoc = CAnimManager::BlendAnimation(GetClump(), animGroupToUse, ANIM_STD_WALK, 1.0f);
				break;
			case PEDMOVE_RUN:
				if (m_nPedState == PED_FLEE_ENTITY) {
					animAssoc = CAnimManager::BlendAnimation(GetClump(), animGroupToUse, ANIM_STD_RUN, 3.0f);
				} else {
					animAssoc = CAnimManager::BlendAnimation(GetClump(), animGroupToUse, ANIM_STD_RUN, 1.0f);
				}
				break;
			case PEDMOVE_SPRINT:
				animAssoc = CAnimManager::BlendAnimation(GetClump(), animGroupToUse, ANIM_STD_RUNFAST, 1.0f);
				break;
			default:
				break;
		}

		if (animAssoc) {
			if (m_leader) {
				CAnimBlendAssociation *walkAssoc = RpAnimBlendClumpGetAssociation(m_leader->GetClump(), ANIM_STD_WALK);
				if (!walkAssoc)
					walkAssoc = RpAnimBlendClumpGetAssociation(m_leader->GetClump(), ANIM_STD_RUN);

				if (!walkAssoc)
					walkAssoc = RpAnimBlendClumpGetAssociation(m_leader->GetClump(), ANIM_STD_RUNFAST);

				if (walkAssoc) {
					animAssoc->speed = walkAssoc->speed;
				} else {
					if (CharCreatedBy == MISSION_CHAR)
						animAssoc->speed = 1.0f;
					else
						animAssoc->speed = 1.2f - m_randomSeed * 0.4f / MYRAND_MAX;
							
				}
			} else {
				if (CharCreatedBy == MISSION_CHAR)
					animAssoc->speed = 1.0f;
				else
					animAssoc->speed = 1.2f - m_randomSeed * 0.4f / MYRAND_MAX;
			}
		}
	}
}

void
CPed::StopNonPartialAnims(void)
{
	CAnimBlendAssociation *assoc;

	for (assoc = RpAnimBlendClumpGetFirstAssociation(GetClump()); assoc; assoc = RpAnimBlendGetNextAssociation(assoc)) {
		if (!assoc->IsPartial())
			assoc->flags &= ~ASSOC_RUNNING;
	}
}

void
CPed::RestartNonPartialAnims(void)
{
	CAnimBlendAssociation *assoc;

	for (assoc = RpAnimBlendClumpGetFirstAssociation(GetClump()); assoc; assoc = RpAnimBlendGetNextAssociation(assoc)) {
		if (!assoc->IsPartial())
			assoc->SetRun();
	}
}

void
CPed::SetStoredState(void)
{
	if (m_nLastPedState != PED_NONE || !CanPedReturnToState())
		return;

	if (m_nPedState == PED_WANDER_PATH) {
		bFindNewNodeAfterStateRestore = true;
		if (m_nMoveState == PEDMOVE_NONE || m_nMoveState == PEDMOVE_STILL)
			m_nMoveState = PEDMOVE_WALK;
	}
#ifdef VC_PED_PORTS
	if (m_nPedState != PED_IDLE)
#endif
	{
		m_nLastPedState = m_nPedState;
		if (m_nMoveState >= m_nPrevMoveState)
			m_nPrevMoveState = m_nMoveState;
	}
}

void
CPed::RestorePreviousState(void)
{
	if(!CanSetPedState() || m_nPedState == PED_FALL)
		return;

	if (m_nPedState == PED_GETUP && !bGetUpAnimStarted)
		return;

	if (InVehicle()) {
		SetPedState(PED_DRIVING);
		m_nLastPedState = PED_NONE;
	} else {
		if (m_nLastPedState == PED_NONE) {
			if (!IsPlayer() && CharCreatedBy != MISSION_CHAR && m_objective == OBJECTIVE_NONE) {
				if (SetWanderPath(CGeneral::GetRandomNumber() & 7) != 0)
					return;
			}
			SetIdle();
			return;
		}

		switch (m_nLastPedState) {
			case PED_IDLE:
				SetIdle();
				break;
			case PED_WANDER_PATH:
				SetPedState(PED_WANDER_PATH);
				bIsRunning = false;
				if (bFindNewNodeAfterStateRestore) {
					if (m_pNextPathNode) {
						CVector diff = m_pNextPathNode->GetPosition() - GetPosition();
						if (diff.MagnitudeSqr() < sq(7.0f)) {
							SetMoveState(PEDMOVE_WALK);
							break;
						}
					}
				}
				SetWanderPath(CGeneral::GetRandomNumber() & 7);
				break;
			default:
				SetPedState(m_nLastPedState);
				SetMoveState((eMoveState) m_nPrevMoveState);
				break;
		}
		m_nLastPedState = PED_NONE;
	}
}

uint32
CPed::ScanForThreats(void)
{
	int fearFlags = m_fearFlags;
	CVector ourPos = GetPosition();
	float closestPedDist = 60.0f;
	CVector2D explosionPos = GetPosition();
	if (fearFlags & PED_FLAG_EXPLOSION && CheckForExplosions(explosionPos)) {
		m_eventOrThreat = explosionPos;
		return PED_FLAG_EXPLOSION;
	}
	
	CPed *shooter = nil;
	if ((fearFlags & PED_FLAG_GUN) && (shooter = CheckForGunShots()) && (m_nPedType != shooter->m_nPedType || m_nPedType == PEDTYPE_CIVMALE || m_nPedType == PEDTYPE_CIVFEMALE)) {
		if (!IsGangMember()) {
			m_threatEntity = shooter;
			m_threatEntity->RegisterReference((CEntity **) &m_threatEntity);
			return PED_FLAG_GUN;
		}

		if (CPedType::GetFlag(shooter->m_nPedType) & fearFlags) {
			m_threatEntity = shooter;
			m_threatEntity->RegisterReference((CEntity **) &m_threatEntity);
			return CPedType::GetFlag(shooter->m_nPedType);
		}
	}

	CPed *deadPed;
	if (fearFlags & PED_FLAG_DEADPEDS && CharCreatedBy != MISSION_CHAR
		&& (deadPed = CheckForDeadPeds()) != nil && (deadPed->GetPosition() - ourPos).MagnitudeSqr() < sq(20.0f)
#ifdef FIX_BUGS
		&& !deadPed->bIsInWater
#endif
		) {
		m_pEventEntity = deadPed;
		m_pEventEntity->RegisterReference((CEntity **) &m_pEventEntity);
		return PED_FLAG_DEADPEDS;
	} else {
		uint32 flagsOfNearPed = 0;

		CPed *pedToFearFrom = nil;
#ifndef VC_PED_PORTS
		for (int i = 0; i < m_numNearPeds; i++) {
			if (CharCreatedBy != RANDOM_CHAR || m_nearPeds[i]->CharCreatedBy != MISSION_CHAR || m_nearPeds[i]->IsPlayer()) {
				CPed *nearPed = m_nearPeds[i];

				// BUG: WTF Rockstar?! Putting this here will result in returning the flags of farthest ped to us, since m_nearPeds is sorted by distance.
				// Fixed at the bottom of the function.
				flagsOfNearPed = CPedType::GetFlag(nearPed->m_nPedType);

				if (flagsOfNearPed & fearFlags) {
					if (nearPed->m_fHealth > 0.0f && OurPedCanSeeThisOne(m_nearPeds[i])) {
						// FIX: Taken from VC
#ifdef FIX_BUGS
						float nearPedDistSqr = (nearPed->GetPosition() - ourPos).MagnitudeSqr2D();
#else
						float nearPedDistSqr = (CVector2D(ourPos) - explosionPos).MagnitudeSqr();
#endif
						if (sq(closestPedDist) > nearPedDistSqr) {
							closestPedDist = Sqrt(nearPedDistSqr);
							pedToFearFrom = m_nearPeds[i];
						}
					}
				}
			}
		}
#else
		bool weSawOurEnemy = false;
		bool weMaySeeOurEnemy = false;
		float closestEnemyDist = 60.0f;
		if ((CTimer::GetFrameCounter() + (uint8)m_randomSeed + 16) & 4) {

			for (int i = 0; i < m_numNearPeds; ++i) {
				if (CharCreatedBy == RANDOM_CHAR && m_nearPeds[i]->CharCreatedBy == MISSION_CHAR && !m_nearPeds[i]->IsPlayer()) {
						continue;
				}

				// BUG: Explained at the same occurence of this bug above. Fixed at the bottom of the function.
				flagsOfNearPed = CPedType::GetFlag(m_nearPeds[i]->m_nPedType);

				if (flagsOfNearPed & fearFlags) {
					if (m_nearPeds[i]->m_fHealth > 0.0f) {

						// VC also has ability to include objects to line of sight check here (via last bit of flagsL)
						if (OurPedCanSeeThisOne(m_nearPeds[i])) {
							if (m_nearPeds[i]->m_nPedState == PED_ATTACK) {
								if (m_nearPeds[i]->m_pedInObjective == this) {

									float enemyDistSqr = (m_nearPeds[i]->GetPosition() - ourPos).MagnitudeSqr2D();
									if (sq(closestEnemyDist) > enemyDistSqr) {
										float enemyDist = Sqrt(enemyDistSqr);
										weSawOurEnemy = true;
										closestPedDist = enemyDist;
										closestEnemyDist = enemyDist;
										pedToFearFrom = m_nearPeds[i];
									}
								}
							} else {
								float nearPedDistSqr = (m_nearPeds[i]->GetPosition() - ourPos).MagnitudeSqr2D();
								if (sq(closestPedDist) > nearPedDistSqr && !weSawOurEnemy) {
									closestPedDist = Sqrt(nearPedDistSqr);
									pedToFearFrom = m_nearPeds[i];
								}
							}
						} else if (!weSawOurEnemy) {
							CPed *nearPed = m_nearPeds[i];
							if (nearPed->m_nPedState == PED_ATTACK) {
								CColPoint foundCol;
								CEntity *foundEnt;

								// We don't see him yet but he's behind a ped, vehicle or object
								// VC also has ability to include objects to line of sight check here (via last bit of flagsL)
								if (!CWorld::ProcessLineOfSight(ourPos, nearPed->GetPosition(), foundCol, foundEnt,
									true, false, false, false, false, false, false)) {

									if (nearPed->m_pedInObjective == this) {
										float enemyDistSqr = (m_nearPeds[i]->GetPosition() - ourPos).MagnitudeSqr2D();
										if (sq(closestEnemyDist) > enemyDistSqr) {
											float enemyDist = Sqrt(enemyDistSqr);
											weMaySeeOurEnemy = true;
											closestPedDist = enemyDist;
											closestEnemyDist = enemyDist;
											pedToFearFrom = m_nearPeds[i];
										}
									} else if (!nearPed->GetWeapon()->IsTypeMelee() && !weMaySeeOurEnemy) {
										float nearPedDistSqr = (m_nearPeds[i]->GetPosition() - ourPos).MagnitudeSqr2D();
										if (sq(closestPedDist) > nearPedDistSqr) {
											weMaySeeOurEnemy = true;
											closestPedDist = Sqrt(nearPedDistSqr);
											pedToFearFrom = m_nearPeds[i];
										}
									}
								}
							}
						}
					}
				}
			}
		}
#endif
		int16 lastVehicle;
		CEntity* vehicles[8];
		CWorld::FindObjectsInRange(ourPos, 20.0f, true, &lastVehicle, 6, vehicles, false, true, false, false, false);
		CVehicle* foundVeh = nil;
		for (int i = 0; i < lastVehicle; i++) {
			CVehicle* nearVeh = (CVehicle*)vehicles[i];

			CPed *driver = nearVeh->pDriver;
			if (driver) {

				// BUG: Same bug as above. Fixed at the bottom of function.
				flagsOfNearPed = CPedType::GetFlag(driver->m_nPedType);
				if (flagsOfNearPed & fearFlags) {
					if (driver->m_fHealth > 0.0f && OurPedCanSeeThisOne(nearVeh->pDriver)) {
						// FIX: Taken from VC
#ifdef FIX_BUGS
						float driverDistSqr = (driver->GetPosition() - ourPos).MagnitudeSqr2D();
#else
						float driverDistSqr = (CVector2D(ourPos) - explosionPos).MagnitudeSqr();
#endif
						if (sq(closestPedDist) > driverDistSqr) {
							closestPedDist = Sqrt(driverDistSqr);
							pedToFearFrom = nearVeh->pDriver;
						}
					}
				}
			}
		}
		m_threatEntity = pedToFearFrom;
		if (m_threatEntity)
			m_threatEntity->RegisterReference((CEntity **) &m_threatEntity);

#ifdef FIX_BUGS
		if (pedToFearFrom)
			flagsOfNearPed = CPedType::GetFlag(((CPed*)m_threatEntity)->m_nPedType);
		else
			flagsOfNearPed = 0;
#endif

		return flagsOfNearPed;
	}
}

void
CPed::SetLookFlag(float direction, bool keepTryingToLook)
{
	if (m_lookTimer < CTimer::GetTimeInMilliseconds()) {
		bIsLooking = true;
		bIsRestoringLook = false;
		m_pLookTarget = nil;
		m_fLookDirection = direction;
		m_lookTimer = 0;
		bKeepTryingToLook = keepTryingToLook;
		if (m_nPedState != PED_DRIVING) {
			m_pedIK.m_flags &= ~CPedIK::LOOKAROUND_HEAD_ONLY;
		}
	}
}

void
CPed::SetLookFlag(CEntity *target, bool keepTryingToLook)
{
	if (m_lookTimer < CTimer::GetTimeInMilliseconds()) {
		bIsLooking = true;
		bIsRestoringLook = false;
		m_pLookTarget = target;
		m_pLookTarget->RegisterReference((CEntity**)&m_pLookTarget);
		m_fLookDirection = 999999.0f;
		m_lookTimer = 0;
		bKeepTryingToLook = keepTryingToLook;
		if (m_nPedState != PED_DRIVING) {
			m_pedIK.m_flags &= ~CPedIK::LOOKAROUND_HEAD_ONLY;
		}
	}
}

void
CPed::ClearLookFlag(void) {
	if (bIsLooking) {
		bIsLooking = false;
		bIsRestoringLook = true;
		bShakeFist = false;

		m_pedIK.m_flags &= ~CPedIK::LOOKAROUND_HEAD_ONLY;
		if (IsPlayer())
			m_lookTimer = CTimer::GetTimeInMilliseconds() + 2000;
		else
			m_lookTimer = CTimer::GetTimeInMilliseconds() + 4000;

		if (m_nPedState == PED_LOOK_HEADING || m_nPedState == PED_LOOK_ENTITY) {
			ClearLook();
		}
	}
}

void
FinishFuckUCB(CAnimBlendAssociation *animAssoc, void *arg)
{
	CPed *ped = (CPed*)arg;

	if (animAssoc->animId == ANIM_STD_PARTIAL_FUCKU && ped->GetWeapon()->m_eWeaponType == WEAPONTYPE_UNARMED)
			ped->RemoveWeaponModel(0);
}

void
CPed::MoveHeadToLook(void)
{
	CVector lookPos;

	if (m_lookTimer && CTimer::GetTimeInMilliseconds() > m_lookTimer) {
		ClearLookFlag();
	} else if (m_nPedState == PED_DRIVING) {
		m_pedIK.m_flags |= CPedIK::LOOKAROUND_HEAD_ONLY;
	}

	if (m_pLookTarget) {

		if (!bShakeFist && GetWeapon()->m_eWeaponType == WEAPONTYPE_UNARMED) {

			CAnimBlendAssociation *fuckUAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_PARTIAL_FUCKU);
			if (fuckUAssoc) {

				float animTime = fuckUAssoc->currentTime;
				if (animTime > 4.0f / 30.0f && animTime - fuckUAssoc->timeStep > 4.0f / 30.0f) {

					bool lookingToCop = false;
					if (m_pLookTarget->GetModelIndex() == MI_POLICE
						|| m_pLookTarget->IsPed() && ((CPed*)m_pLookTarget)->m_nPedType == PEDTYPE_COP) {

						lookingToCop = true;
					}

					if (IsPlayer() && (m_pedStats->m_temper >= 52 || lookingToCop)) {
						AddWeaponModel(MI_FINGERS);
						((CPlayerPed*)this)->AnnoyPlayerPed(true);

					} else if ((CGeneral::GetRandomNumber() & 3) == 0) {
						AddWeaponModel(MI_FINGERS);
					}
				}
			}
		}

		if (m_pLookTarget->IsPed()) {
			((CPed*)m_pLookTarget)->m_pedIK.GetComponentPosition(lookPos, PED_MID);
		} else {
			lookPos = m_pLookTarget->GetPosition();
		}

		if (!m_pedIK.LookAtPosition(lookPos)) {
			if (!bKeepTryingToLook) {
				ClearLookFlag();
			}
			return;
		}

		if (!bShakeFist || bIsAimingGun || bIsRestoringGun)
			return;

		if (m_lookTimer - CTimer::GetTimeInMilliseconds() >= 1000)
			return;

		bool notRocketLauncher = false;
		bool notTwoHanded = false;
		AnimationId animToPlay = ANIM_STD_NUM;

		if (!GetWeapon()->IsType2Handed())
			notTwoHanded = true;

		if (notTwoHanded && GetWeapon()->m_eWeaponType != WEAPONTYPE_ROCKETLAUNCHER)
			notRocketLauncher = true;

		if (IsPlayer() && notRocketLauncher) {

			if (m_pLookTarget->IsPed()) {

				if (m_pedStats->m_temper >= 49 && ((CPed*)m_pLookTarget)->m_nPedType != PEDTYPE_COP) {

					// FIX: Unreachable and meaningless condition
#ifndef FIX_BUGS
					if (m_pedStats->m_temper < 47)
#endif
						animToPlay = ANIM_STD_PARTIAL_PUNCH;
				} else {
					animToPlay = ANIM_STD_PARTIAL_FUCKU;
				}
			} else if (m_pedStats->m_temper > 49 || m_pLookTarget->GetModelIndex() == MI_POLICE) {
				animToPlay = ANIM_STD_PARTIAL_FUCKU;
			}
		} else if (notRocketLauncher && (CGeneral::GetRandomNumber() & 1)) {
			animToPlay = ANIM_STD_PARTIAL_FUCKU;
		}

		if (animToPlay != ANIM_STD_NUM) {
			CAnimBlendAssociation *newAssoc = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, animToPlay, 4.0f);

			if (newAssoc) {
				newAssoc->flags |= ASSOC_FADEOUTWHENDONE;
				newAssoc->flags |= ASSOC_DELETEFADEDOUT;
				if (newAssoc->animId == ANIM_STD_PARTIAL_FUCKU)
					newAssoc->SetDeleteCallback(FinishFuckUCB, this);
			}
		}
		bShakeFist = false;
		return;
	} else if (999999.0f == m_fLookDirection) {
		ClearLookFlag();
	} else if (!m_pedIK.LookInDirection(m_fLookDirection, 0.0f)) {
		if (!bKeepTryingToLook)
			ClearLookFlag();
	}
}

void
CPed::RestoreHeadPosition(void)
{
	if (m_pedIK.RestoreLookAt()) {
		bIsRestoringLook = false;
	}
}

void
CPed::SetAimFlag(float angle)
{
	bIsAimingGun = true;
	bIsRestoringGun = false;
	m_fLookDirection = angle;
	m_lookTimer = 0;
	m_pLookTarget = nil;
	m_pSeekTarget = nil;
	if (CWeaponInfo::GetWeaponInfo(GetWeapon()->m_eWeaponType)->IsFlagSet(WEAPONFLAG_CANAIM_WITHARM))
		m_pedIK.m_flags |= CPedIK::AIMS_WITH_ARM;
	else
		m_pedIK.m_flags &= ~CPedIK::AIMS_WITH_ARM;
}

void
CPed::SetAimFlag(CEntity *to)
{
	bIsAimingGun = true;
	bIsRestoringGun = false;
	m_pLookTarget = to;
	m_pLookTarget->RegisterReference((CEntity **) &m_pLookTarget);
	m_pSeekTarget = to;
	m_pSeekTarget->RegisterReference((CEntity **) &m_pSeekTarget);
	m_lookTimer = 0;
}

void
CPed::ClearAimFlag(void)
{
	if (bIsAimingGun) {
		bIsAimingGun = false;
		bIsRestoringGun = true;
		m_pedIK.m_flags &= ~CPedIK::AIMS_WITH_ARM;
#if defined VC_PED_PORTS || defined FIX_BUGS
		m_lookTimer = 0;
#endif
	}

	if (IsPlayer())
		((CPlayerPed*)this)->m_fFPSMoveHeading = 0.0f;
}

void
CPed::AimGun(void)
{
	CVector vector;

	if (m_pSeekTarget) {
		if (m_pSeekTarget->IsPed()) {
			((CPed*)m_pSeekTarget)->m_pedIK.GetComponentPosition(vector, PED_MID);
		} else {
			vector = m_pSeekTarget->GetPosition();
		}
		Say(SOUND_PED_ATTACK);

		bCanPointGunAtTarget = m_pedIK.PointGunAtPosition(vector);
		if (m_pLookTarget != m_pSeekTarget) {
			SetLookFlag(m_pSeekTarget, true);
		}

	} else {
		if (IsPlayer()) {
			bCanPointGunAtTarget = m_pedIK.PointGunInDirection(m_fLookDirection, ((CPlayerPed*)this)->m_fFPSMoveHeading);
		} else {
			bCanPointGunAtTarget = m_pedIK.PointGunInDirection(m_fLookDirection, 0.0f);
		}
	}
}

void
CPed::RestoreGunPosition(void)
{
	if (bIsLooking) {
		m_pedIK.m_flags &= ~CPedIK::LOOKAROUND_HEAD_ONLY;
		bIsRestoringGun = false;
	} else if (m_pedIK.RestoreGunPosn()) {
		bIsRestoringGun = false;
	} else {
		if (IsPlayer())
			((CPlayerPed*)this)->m_fFPSMoveHeading = 0.0f;
	}
}

void
CPed::ScanForInterestingStuff(void)
{
	if (!IsPedInControl())
		return;

	if (m_objective != OBJECTIVE_NONE)
		return;

	if (CharCreatedBy == MISSION_CHAR)
		return;

	LookForSexyPeds();
	LookForSexyCars();
	if (LookForInterestingNodes())
		return;

	if (m_nPedType == PEDTYPE_CRIMINAL && m_carJackTimer < CTimer::GetTimeInMilliseconds()) {
		// Find a car to steal or a ped to mug if we haven't already decided to steal a car
		if (CGeneral::GetRandomNumber() % 100 < 10) {
			int mostExpensiveVehAround = -1;
			int bestMonetaryValue = 0;

			CVector pos = GetPosition();
			int16 lastVehicle;
			CEntity *vehicles[8];
			CWorld::FindObjectsInRange(pos, 10.0f, true, &lastVehicle, 6, vehicles, false, true, false, false, false);

			for (int i = 0; i < lastVehicle; i++) {
				CVehicle* veh = (CVehicle*)vehicles[i];

				if (veh->VehicleCreatedBy != MISSION_VEHICLE) {
					if (veh->m_vecMoveSpeed.Magnitude() <= 0.1f && veh->IsVehicleNormal()
						&& veh->IsCar() && bestMonetaryValue < veh->pHandling->nMonetaryValue) {
						mostExpensiveVehAround = i;
						bestMonetaryValue = veh->pHandling->nMonetaryValue;
					}
				}
			}
			if (bestMonetaryValue > 2000 && mostExpensiveVehAround != -1 && vehicles[mostExpensiveVehAround]) {
				SetObjective(OBJECTIVE_ENTER_CAR_AS_DRIVER, vehicles[mostExpensiveVehAround]);
				m_carJackTimer = CTimer::GetTimeInMilliseconds() + 5000;
				return;
			}
			m_carJackTimer = CTimer::GetTimeInMilliseconds() + 5000;
		} else if (m_objective != OBJECTIVE_MUG_CHAR && !(CGeneral::GetRandomNumber() & 7)) {
			CPed *charToMug = nil;
			for (int i = 0; i < m_numNearPeds; ++i) {
				CPed *nearPed = m_nearPeds[i];

				if ((nearPed->GetPosition() - GetPosition()).MagnitudeSqr() > sq(7.0f))
					break;

				if ((nearPed->m_nPedType == PEDTYPE_CIVFEMALE || nearPed->m_nPedType == PEDTYPE_CIVMALE
					|| nearPed->m_nPedType == PEDTYPE_CRIMINAL || nearPed->m_nPedType == PEDTYPE_UNUSED1
					|| nearPed->m_nPedType == PEDTYPE_PROSTITUTE)
					&& nearPed->CharCreatedBy != MISSION_CHAR
					&& nearPed->IsPedShootable()
					&& nearPed->m_objective != OBJECTIVE_MUG_CHAR) {
					charToMug = nearPed;
					break;
				}
			}
			if (charToMug)
				SetObjective(OBJECTIVE_MUG_CHAR, charToMug);

			m_carJackTimer = CTimer::GetTimeInMilliseconds() + 5000;
		}
	}

	if (m_nPedState == PED_WANDER_PATH) {
#ifndef VC_PED_PORTS
		if (CTimer::GetTimeInMilliseconds() > m_chatTimer) {

			// += 2 is weird
			for (int i = 0; i < m_numNearPeds; i += 2) {
				if (m_nearPeds[i]->m_nPedState == PED_WANDER_PATH && WillChat(m_nearPeds[i])) {
					if (CGeneral::GetRandomNumberInRange(0, 100) >= 100)
						m_chatTimer = CTimer::GetTimeInMilliseconds() + 30000;
					else {
						if ((GetPosition() - m_nearPeds[i]->GetPosition()).Magnitude() >= 1.8f) {
							m_chatTimer = CTimer::GetTimeInMilliseconds() + 30000;
						} else if (CanSeeEntity(m_nearPeds[i])) {
							int time = CGeneral::GetRandomNumber() % 4000 + 10000;
							SetChat(m_nearPeds[i], time);
							m_nearPeds[i]->SetChat(this, time);
							return;
						}
					}
				}
			}
		}
#else
		if (CGeneral::GetRandomNumberInRange(0.0f, 1.0f) < 0.5f) {
			if (CTimer::GetTimeInMilliseconds() > m_chatTimer) {
				for (int i = 0; i < m_numNearPeds; i ++) {
					if (m_nearPeds[i] && m_nearPeds[i]->m_nPedState == PED_WANDER_PATH) {
						if ((GetPosition() - m_nearPeds[i]->GetPosition()).Magnitude() < 1.8f
							&& CanSeeEntity(m_nearPeds[i])
							&& m_nearPeds[i]->CanSeeEntity(this)
							&& WillChat(m_nearPeds[i])) {

							int time = CGeneral::GetRandomNumber() % 4000 + 10000;
							SetChat(m_nearPeds[i], time);
							m_nearPeds[i]->SetChat(this, time);
							return;
						}
					}
				}
			}
		} else {
			m_chatTimer = CTimer::GetTimeInMilliseconds() + 200;
		}
#endif
	}

	// Parts below aren't there in VC, they're in somewhere else.
	if (!CGame::noProstitutes && m_nPedType == PEDTYPE_PROSTITUTE && CharCreatedBy != MISSION_CHAR
		&& m_objectiveTimer < CTimer::GetTimeInMilliseconds() && !CTheScripts::IsPlayerOnAMission()) {

		CVector pos = GetPosition();
		int16 lastVehicle;
		CEntity* vehicles[8];
		CWorld::FindObjectsInRange(pos, CHECK_NEARBY_THINGS_MAX_DIST, true, &lastVehicle, 6, vehicles, false, true, false, false, false);

		for (int i = 0; i < lastVehicle; i++) {
			CVehicle* veh = (CVehicle*)vehicles[i];

			if (veh->IsVehicleNormal()) {
				if (veh->IsCar()) {
					if ((GetPosition() - veh->GetPosition()).Magnitude() < 5.0f && veh->IsRoomForPedToLeaveCar(CAR_DOOR_LF, nil)) {
						SetObjective(OBJECTIVE_SOLICIT_VEHICLE, veh);
						Say(SOUND_PED_SOLICIT);
						return;
					}
				}
			}
		}
	}
	if (m_nPedType == PEDTYPE_CIVMALE || m_nPedType == PEDTYPE_CIVFEMALE) {
		CVector pos = GetPosition();
		int16 lastVehicle;
		CEntity* vehicles[8];
		CWorld::FindObjectsInRange(pos, CHECK_NEARBY_THINGS_MAX_DIST, true, &lastVehicle, 6, vehicles, false, true, false, false, false);

		for (int i = 0; i < lastVehicle; i++) {
			CVehicle* veh = (CVehicle*)vehicles[i];

			if (veh->GetModelIndex() == MI_MRWHOOP) {
				if (veh->GetStatus() != STATUS_ABANDONED && veh->GetStatus() != STATUS_WRECKED) {
					if ((GetPosition() - veh->GetPosition()).Magnitude() < 5.0f) {
						SetObjective(OBJECTIVE_BUY_ICE_CREAM, veh);
						return;
					}
				}
			}
		}
	}
}

bool
CPed::WillChat(CPed *stranger)
{
	if (m_pNextPathNode && m_pLastPathNode) {
		if (m_pNextPathNode != m_pLastPathNode && ThePaths.TestCrossesRoad(m_pNextPathNode, m_pLastPathNode)) {
			return false;
		}
	}
	if (m_nSurfaceTouched == SURFACE_TARMAC)
		return false;
	if (stranger == this)
		return false;
	if (m_nPedType == stranger->m_nPedType)
		return true;
	if (m_nPedType == PEDTYPE_CRIMINAL)
		return false;
	if ((IsGangMember() || stranger->IsGangMember()) && m_nPedType != stranger->m_nPedType)
		return false;
	return true;
}

void
CPed::CalculateNewVelocity(void)
{
	if (IsPedInControl()) {
		float headAmount = DEGTORAD(m_headingRate) * CTimer::GetTimeStep();
		m_fRotationCur = CGeneral::LimitRadianAngle(m_fRotationCur);
		float limitedRotDest = CGeneral::LimitRadianAngle(m_fRotationDest);

		if (m_fRotationCur - PI > limitedRotDest) {
				limitedRotDest += 2 * PI;
		} else if(PI + m_fRotationCur < limitedRotDest) {
			limitedRotDest -= 2 * PI;
		}

		if (IsPlayer() && m_nPedState == PED_ATTACK)
			headAmount /= 4.0f;

		float neededTurn = limitedRotDest - m_fRotationCur;
		if (neededTurn <= headAmount) {
			if (neededTurn > (-headAmount))
				m_fRotationCur += neededTurn;
			else
				m_fRotationCur -= headAmount;
		} else {
			m_fRotationCur += headAmount;
		}
	}

	CVector2D forward(Sin(m_fRotationCur), Cos(m_fRotationCur));

	m_moved.x = CrossProduct2D(m_vecAnimMoveDelta, forward); // (m_vecAnimMoveDelta.x * Cos(m_fRotationCur)) + -Sin(m_fRotationCur) * m_vecAnimMoveDelta.y;
	m_moved.y = DotProduct2D(m_vecAnimMoveDelta, forward);  // m_vecAnimMoveDelta.y* Cos(m_fRotationCur) + (m_vecAnimMoveDelta.x * Sin(m_fRotationCur));

	if (CTimer::GetTimeStep() >= 0.01f) {
		m_moved = m_moved * (1 / CTimer::GetTimeStep());
	} else {
		m_moved = m_moved * (1 / 100.0f);
	}

	if ((!TheCamera.Cams[TheCamera.ActiveCam].GetWeaponFirstPersonOn() && !TheCamera.Cams[0].Using3rdPersonMouseCam())
		|| FindPlayerPed() != this || !CanStrafeOrMouseControl())
		return;

	float walkAngle = WorkOutHeadingForMovingFirstPerson(m_fRotationCur);
	float pedSpeed = m_moved.Magnitude();
	float localWalkAngle = CGeneral::LimitRadianAngle(walkAngle - m_fRotationCur);

	if (localWalkAngle < -0.5f * PI) {
		localWalkAngle += PI;
	} else if (localWalkAngle > 0.5f * PI) {
		localWalkAngle -= PI;
	}

	// Interestingly this part is responsible for diagonal walking.
	if (localWalkAngle > -DEGTORAD(50.0f) && localWalkAngle < DEGTORAD(50.0f)) {
		TheCamera.Cams[TheCamera.ActiveCam].m_fPlayerVelocity = pedSpeed;
		m_moved = CVector2D(-Sin(walkAngle), Cos(walkAngle)) * pedSpeed;
	}

	CAnimBlendAssociation *idleAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_IDLE);
	CAnimBlendAssociation *fightAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_FIGHT_IDLE);
#ifdef VC_PED_PORTS
	if ((!idleAssoc || idleAssoc->blendAmount < 0.5f) && !fightAssoc && !bIsDucking) {
#else
	if ((!idleAssoc || idleAssoc->blendAmount < 0.5f) && !fightAssoc) {
#endif
		LimbOrientation newUpperLegs;
		newUpperLegs.yaw = localWalkAngle;

		if (newUpperLegs.yaw < -DEGTORAD(100.0f)) {
			newUpperLegs.yaw += PI;
		} else if (newUpperLegs.yaw > DEGTORAD(100.0f)) {
			newUpperLegs.yaw -= PI;
		}

		if (newUpperLegs.yaw > -DEGTORAD(50.0f) && newUpperLegs.yaw < DEGTORAD(50.0f)) {
#ifdef PED_SKIN
			if(IsClumpSkinned(GetClump())){
/*
				// this looks shit
				newUpperLegs.pitch = 0.0f;
				RwV3d axis = { -1.0f, 0.0f, 0.0f };
				RtQuatRotate(&m_pFrames[PED_UPPERLEGL]->hanimFrame->q, &axis, RADTODEG(newUpperLegs.yaw), rwCOMBINEPRECONCAT);
				RtQuatRotate(&m_pFrames[PED_UPPERLEGR]->hanimFrame->q, &axis, RADTODEG(newUpperLegs.yaw), rwCOMBINEPRECONCAT);
*/
				newUpperLegs.pitch = 0.1f;
				RwV3d Xaxis = { 1.0f, 0.0f, 0.0f };
				RwV3d Zaxis = { 0.0f, 0.0f, 1.0f };
				RtQuatRotate(&m_pFrames[PED_UPPERLEGL]->hanimFrame->q, &Zaxis, RADTODEG(newUpperLegs.pitch), rwCOMBINEPOSTCONCAT);
				RtQuatRotate(&m_pFrames[PED_UPPERLEGL]->hanimFrame->q, &Xaxis, RADTODEG(newUpperLegs.yaw), rwCOMBINEPOSTCONCAT);
				RtQuatRotate(&m_pFrames[PED_UPPERLEGR]->hanimFrame->q, &Zaxis, RADTODEG(newUpperLegs.pitch), rwCOMBINEPOSTCONCAT);
				RtQuatRotate(&m_pFrames[PED_UPPERLEGR]->hanimFrame->q, &Xaxis, RADTODEG(newUpperLegs.yaw), rwCOMBINEPOSTCONCAT);

				bDontAcceptIKLookAts = true;
			}else
#endif
			{
				newUpperLegs.pitch = 0.0f;
				m_pedIK.RotateTorso(m_pFrames[PED_UPPERLEGL], &newUpperLegs, false);
				m_pedIK.RotateTorso(m_pFrames[PED_UPPERLEGR], &newUpperLegs, false);
			}
		}
	}
}

float
CPed::WorkOutHeadingForMovingFirstPerson(float offset)
{
	if (!IsPlayer())
		return 0.0f;

	CPad *pad0 = CPad::GetPad(0);
	float leftRight = pad0->GetPedWalkLeftRight();
	float upDown = pad0->GetPedWalkUpDown();
	float &angle = ((CPlayerPed*)this)->m_fWalkAngle;

	if (upDown != 0.0f) {
		angle = CGeneral::GetRadianAngleBetweenPoints(0.0f, 0.0f, -leftRight, upDown);
	} else {
		if (leftRight < 0.0f)
			angle = 0.5f * PI;
		else if (leftRight > 0.0f)
			angle = -0.5f * PI;
	}

	return CGeneral::LimitRadianAngle(offset + angle);
}

void
CPed::UpdatePosition(void)
{
	if (CReplay::IsPlayingBack() || !bIsStanding)
		return;

	CVector2D velocityChange;

	SetHeading(m_fRotationCur);
	if (m_pCurrentPhysSurface) {
		CVector2D velocityOfSurface;
		if (!IsPlayer() && m_pCurrentPhysSurface->IsVehicle() && ((CVehicle*)m_pCurrentPhysSurface)->IsBoat()) {

			// It seems R* didn't like m_vecOffsetFromPhysSurface for boats
			CVector offsetToSurface = GetPosition() - m_pCurrentPhysSurface->GetPosition();
			offsetToSurface.z -= FEET_OFFSET;

			CVector surfaceMoveVelocity = m_pCurrentPhysSurface->m_vecMoveSpeed;
			CVector surfaceTurnVelocity = CrossProduct(m_pCurrentPhysSurface->m_vecTurnSpeed, offsetToSurface);

			// Also we use that weird formula instead of friction if it's boat
			float slideMult = -m_pCurrentPhysSurface->m_vecTurnSpeed.MagnitudeSqr();
			velocityOfSurface = slideMult * offsetToSurface * CTimer::GetTimeStep() + (surfaceTurnVelocity + surfaceMoveVelocity);
			m_vecMoveSpeed.z = slideMult * offsetToSurface.z * CTimer::GetTimeStep() + (surfaceTurnVelocity.z + surfaceMoveVelocity.z);
		} else {
			velocityOfSurface = m_pCurrentPhysSurface->GetSpeed(m_vecOffsetFromPhysSurface);
		}
		// Reminder: m_moved is displacement from walking/running.
		velocityChange = m_moved + velocityOfSurface - m_vecMoveSpeed;
		m_fRotationCur += m_pCurrentPhysSurface->m_vecTurnSpeed.z * CTimer::GetTimeStep();
		m_fRotationDest += m_pCurrentPhysSurface->m_vecTurnSpeed.z * CTimer::GetTimeStep();
	} else if (m_nSurfaceTouched == SURFACE_STEEP_CLIFF && (m_vecDamageNormal.x != 0.0f || m_vecDamageNormal.y != 0.0f)) {
		// Ped got damaged by steep slope
		m_vecMoveSpeed = CVector(0.0f, 0.0f, -0.001f);
		// some kind of
		CVector2D reactionForce = m_vecDamageNormal;
		reactionForce.Normalise();

		velocityChange = 0.02f * reactionForce + m_moved;

		float reactionAndVelocityDotProd = DotProduct2D(reactionForce, velocityChange);
		// they're in same direction
		if (reactionAndVelocityDotProd < 0.0f) {
			velocityChange -= reactionAndVelocityDotProd * reactionForce;
		}
	} else {
		velocityChange = m_moved - m_vecMoveSpeed;
	}
	
	// Take time step into account
	if (m_pCurrentPhysSurface) {
		float speedChange = velocityChange.Magnitude();
		float changeMult = speedChange;
		if (m_nPedState == PED_DIE && m_pCurrentPhysSurface->IsVehicle()) {
			changeMult = 0.002f * CTimer::GetTimeStep();
		} else if (!(m_pCurrentPhysSurface->IsVehicle() && ((CVehicle*)m_pCurrentPhysSurface)->IsBoat())) {
			changeMult = 0.01f * CTimer::GetTimeStep();
		}

		if (speedChange > changeMult) {
			velocityChange = velocityChange * (changeMult / speedChange);
		}
	}
	m_vecMoveSpeed.x += velocityChange.x;
	m_vecMoveSpeed.y += velocityChange.y;
}

void
CPed::CalculateNewOrientation(void)
{
	if (CReplay::IsPlayingBack() || !IsPedInControl())
		return;

	SetHeading(m_fRotationCur);
}

void
CPed::ClearAll(void)
{
	if (!IsPedInControl() && m_nPedState != PED_DEAD)
		return;

	SetPedState(PED_NONE);
	SetMoveState(PEDMOVE_NONE);
	m_pSeekTarget = nil;
	m_vecSeekPos = CVector(0.0f, 0.0f, 0.0f);
	m_fleeFromPosX = 0.0f;
	m_fleeFromPosY = 0.0f;
	m_fleeFrom = nil;
	m_fleeTimer = 0;
	bUsesCollision = true;
#ifdef VC_PED_PORTS
	ClearPointGunAt();
#else
	ClearAimFlag();
	ClearLookFlag();
#endif
	bIsPointingGunAt = false;
	bRenderPedInCar = true;
	bKnockedUpIntoAir = false;
	m_pCollidingEntity = nil;
}

void
CPed::ProcessBuoyancy(void)
{
	static uint32 nGenerateRaindrops = 0;
	static uint32 nGenerateWaterCircles = 0;
	CRGBA color(((0.5f * CTimeCycle::GetDirectionalRed() + CTimeCycle::GetAmbientRed()) * 127.5f),
		((0.5f * CTimeCycle::GetDirectionalBlue() + CTimeCycle::GetAmbientBlue()) * 127.5f),
		((0.5f * CTimeCycle::GetDirectionalGreen() + CTimeCycle::GetAmbientGreen()) * 127.5f),
		CGeneral::GetRandomNumberInRange(48.0f, 96.0f));

	if (bInVehicle)
		return;

	CVector buoyancyPoint;
	CVector buoyancyImpulse;

#ifndef VC_PED_PORTS
	float buoyancyLevel = (m_nPedState == PED_DEAD ? 1.5f : 1.3f);
#else
	float buoyancyLevel = (m_nPedState == PED_DEAD ? 1.8f : 1.1f);
#endif

	if (mod_Buoyancy.ProcessBuoyancy(this, GRAVITY * m_fMass * buoyancyLevel, &buoyancyPoint, &buoyancyImpulse)) {
		bTouchingWater = true;
		CEntity *entity;
		CColPoint point;
		if (CWorld::ProcessVerticalLine(GetPosition(), GetPosition().z - 3.0f, point, entity, false, true, false, false, false, false, nil)
			&& entity->IsVehicle() && ((CVehicle*)entity)->IsBoat()) {
			bIsInWater = false;
			return;
		}
		bIsInWater = true;
		ApplyMoveForce(buoyancyImpulse);
		if (!DyingOrDead()) {
			if (bTryingToReachDryLand) {
				if (buoyancyImpulse.z / m_fMass > GRAVITY * 0.4f * CTimer::GetTimeStep()) {
					bTryingToReachDryLand = false;
					CVector pos = GetPosition();
					if (PlacePedOnDryLand()) {
						if (m_fHealth > 20.0f)
							InflictDamage(nil, WEAPONTYPE_DROWNING, 15.0f, PEDPIECE_TORSO, false);

						if (bIsInTheAir) {
							RpAnimBlendClumpSetBlendDeltas(GetClump(), ASSOC_PARTIAL, -1000.0f);
							bIsInTheAir = false;
						}
						pos.z = pos.z - 0.8f;
#ifdef PC_PARTICLE
						CParticleObject::AddObject(POBJECT_PED_WATER_SPLASH, pos, CVector(0.0f, 0.0f, 0.0f), 0.0f, 50, color, true);
#else
						CParticleObject::AddObject(POBJECT_PED_WATER_SPLASH, pos, CVector(0.0f, 0.0f, 0.0f), 0.0f, 50, CRGBA(0, 0, 0, 0), true);
#endif
						m_vecMoveSpeed = CVector(0.0f, 0.0f, 0.0f);
						SetPedState(PED_IDLE);
						return;
					}
				}
			}
			float speedMult = 0.0f;
			if (buoyancyImpulse.z / m_fMass > GRAVITY * 0.75f * CTimer::GetTimeStep()
				|| mod_Buoyancy.m_waterlevel > GetPosition().z) {
				speedMult = pow(0.9f, CTimer::GetTimeStep());
				m_vecMoveSpeed.x *= speedMult;
				m_vecMoveSpeed.y *= speedMult;
				m_vecMoveSpeed.z *= speedMult;
				bIsStanding = false;
				InflictDamage(nil, WEAPONTYPE_DROWNING, 3.0f * CTimer::GetTimeStep(), PEDPIECE_TORSO, 0);
			}
			if (buoyancyImpulse.z / m_fMass > GRAVITY * 0.25f * CTimer::GetTimeStep()) {
				if (speedMult == 0.0f) {
					speedMult = pow(0.9f, CTimer::GetTimeStep());
				}
				m_vecMoveSpeed.x *= speedMult;
				m_vecMoveSpeed.y *= speedMult;
				if (m_vecMoveSpeed.z >= -0.1f) {
					if (m_vecMoveSpeed.z < -0.04f)
						m_vecMoveSpeed.z = -0.02f;
				} else {
					m_vecMoveSpeed.z = -0.01f;
					DMAudio.PlayOneShot(m_audioEntityId, SOUND_SPLASH, 0.0f);
#ifdef PC_PARTICLE
					CVector aBitForward = 2.2f * m_vecMoveSpeed + GetPosition();
					float level = 0.0f;
					if (CWaterLevel::GetWaterLevel(aBitForward, &level, false))
						aBitForward.z = level;

					CParticleObject::AddObject(POBJECT_PED_WATER_SPLASH, aBitForward, CVector(0.0f, 0.0f, 0.1f), 0.0f, 200, color, true);
					nGenerateRaindrops = CTimer::GetTimeInMilliseconds() + 80;
					nGenerateWaterCircles = CTimer::GetTimeInMilliseconds() + 100;
#else
					CVector aBitForward = 1.6f * m_vecMoveSpeed + GetPosition();
					float level = 0.0f;
					if (CWaterLevel::GetWaterLevel(aBitForward, &level, false))
						aBitForward.z = level + 0.5f;
					
					CVector vel = m_vecMoveSpeed * 0.1f;
					vel.z = 0.18f;
					CParticleObject::AddObject(POBJECT_PED_WATER_SPLASH, aBitForward, vel, 0.0f, 350, CRGBA(0, 0, 0, 0), true);
					nGenerateRaindrops = CTimer::GetTimeInMilliseconds() + 300;
					nGenerateWaterCircles = CTimer::GetTimeInMilliseconds() + 60;
#endif
				}
			}
		} else
			return;
	} else
		bTouchingWater = false;

	if (nGenerateWaterCircles && CTimer::GetTimeInMilliseconds() >= nGenerateWaterCircles) {
		CVector pos = GetPosition();
		float level = 0.0f;
		if (CWaterLevel::GetWaterLevel(pos, &level, false))
			pos.z = level;

		if (pos.z != 0.0f) {
			nGenerateWaterCircles = 0;
			for(int i = 0; i < 4; i++) {
#ifdef PC_PARTICLE
				pos.x += CGeneral::GetRandomNumberInRange(-0.75f, 0.75f);
				pos.y += CGeneral::GetRandomNumberInRange(-0.75f, 0.75f);
				CParticle::AddParticle(PARTICLE_RAIN_SPLASH_BIGGROW, pos, CVector(0.0f, 0.0f, 0.0f), nil, 0.0f, color, 0, 0, 0, 0);
#else
				pos.x += CGeneral::GetRandomNumberInRange(-2.5f, 2.5f);
				pos.y += CGeneral::GetRandomNumberInRange(-2.5f, 2.5f);
				CParticle::AddParticle(PARTICLE_RAIN_SPLASH_BIGGROW, pos+CVector(0.0f, 0.0f, 1.0f), CVector(0.0f, 0.0f, 0.0f));
#endif
			}
		}
	}

	if (nGenerateRaindrops && CTimer::GetTimeInMilliseconds() >= nGenerateRaindrops) {
		CVector pos = GetPosition();
		float level = 0.0f;
		if (CWaterLevel::GetWaterLevel(pos, &level, false))
			pos.z = level;

		if (pos.z >= 0.0f) {
#ifdef PC_PARTICLE
			pos.z += 0.25f;
#else
			pos.z += 0.5f;
#endif
			nGenerateRaindrops = 0;
#ifdef PC_PARTICLE
			CParticleObject::AddObject(POBJECT_SPLASHES_AROUND, pos, CVector(0.0f, 0.0f, 0.0f), 4.5f, 1500, CRGBA(0,0,0,0), true);
#else
			CParticleObject::AddObject(POBJECT_SPLASHES_AROUND, pos, CVector(0.0f, 0.0f, 0.0f), 4.5f, 2500, CRGBA(0,0,0,0), true);
#endif
		}
	}
}

void
CPed::ProcessControl(void)
{
	CColPoint foundCol;
	CEntity *foundEnt = nil;

	if (m_nZoneLevel > LEVEL_GENERIC && m_nZoneLevel != CCollision::ms_collisionInMemory)
		return;

	int alpha = CVisibilityPlugins::GetClumpAlpha(GetClump());
	if (!bFadeOut) {
		if (alpha < 255) {
			alpha += 16;
			if (alpha > 255)
				alpha = 255;
		}
	} else {
		alpha -= 8;
		if (alpha < 0)
			alpha = 0;
	}

	CVisibilityPlugins::SetClumpAlpha(GetClump(), alpha);
	bIsShooting = false;
	BuildPedLists();
	bIsInWater = false;
	ProcessBuoyancy();

	if (m_nPedState != PED_ARRESTED) {
		if (m_nPedState == PED_DEAD) {
			DeadPedMakesTyresBloody();
#ifndef VC_PED_PORTS
			if (CGame::nastyGame) {
#else
			if (CGame::nastyGame && !bIsInWater) {
#endif
				uint32 remainingBloodyFpTime = CTimer::GetTimeInMilliseconds() - m_bloodyFootprintCountOrDeathTime;
				float timeDependentDist;
				if (remainingBloodyFpTime >= 2000) {
					if (remainingBloodyFpTime <= 7000)
						timeDependentDist = (remainingBloodyFpTime - 2000) / 5000.0f * 0.75f;
					else
						timeDependentDist = 0.75f;
				} else {
					timeDependentDist = 0.0f;
				}

				for (int i = 0; i < m_numNearPeds; ++i) {
					CPed *nearPed = m_nearPeds[i];
					if (!nearPed->DyingOrDead()) {
						CVector dist = nearPed->GetPosition() - GetPosition();
						if (dist.MagnitudeSqr() < sq(timeDependentDist)) {
							nearPed->m_bloodyFootprintCountOrDeathTime = 200;
							nearPed->bDoBloodyFootprints = true;
							if (nearPed->IsPlayer()) {
								if (!nearPed->bIsLooking && nearPed->m_nPedState != PED_ATTACK) {
									int16 camMode = TheCamera.Cams[TheCamera.ActiveCam].Mode;
									if (camMode != CCam::MODE_SNIPER
										&& camMode != CCam::MODE_ROCKETLAUNCHER
										&& camMode != CCam::MODE_M16_1STPERSON
										&& camMode != CCam::MODE_1STPERSON
										&& camMode != CCam::MODE_HELICANNON_1STPERSON
										&& !TheCamera.Cams[TheCamera.ActiveCam].GetWeaponFirstPersonOn()) {

										nearPed->SetLookFlag(this, true);
										nearPed->SetLookTimer(500);
									}
								}
							}
						}
					}
				}

				if (remainingBloodyFpTime > 2000) {
					CVector bloodPos = GetPosition();
					if (remainingBloodyFpTime - 2000 >= 5000) {
						if (!m_deadBleeding) {
							CShadows::AddPermanentShadow(SHADOWTYPE_DARK, gpBloodPoolTex, &bloodPos,
								0.75f, 0.0f, 0.0f, -0.75f, 255, 255, 0, 0, 4.0f, 40000, 1.0f);
							m_deadBleeding = true;
						}
					} else {
						CShadows::StoreStaticShadow(
							(uintptr)this + 17, SHADOWTYPE_DARK, gpBloodPoolTex, &bloodPos,
							(remainingBloodyFpTime - 2000) / 5000.0f * 0.75f, 0.0f,
							0.0f, (remainingBloodyFpTime - 2000) / 5000.0f * -0.75f,
							255, 255, 0, 0, 4.0f, 1.0f, 40.0f, false, 0.0f);
					}
				}
			}
			if (ServiceTalkingWhenDead())
				ServiceTalking();

#ifdef VC_PED_PORTS
			if (bIsInWater) {
				bIsStanding = false;
				bWasStanding = false;
				CPhysical::ProcessControl();
			}
#endif
			return;
		}

		bWasStanding = false;
		if (bIsStanding) {
			if (!CWorld::bForceProcessControl) {
				if (m_pCurrentPhysSurface && m_pCurrentPhysSurface->bIsInSafePosition) {
					bWasPostponed = true;
					return;
				}
			}
		}

		if (!IsPedInControl() || m_nWaitState != WAITSTATE_FALSE || 0.01f * CTimer::GetTimeStep() <= m_fDistanceTravelled
			|| (m_nStoredMoveState != PEDMOVE_WALK && m_nStoredMoveState != PEDMOVE_RUN && m_nStoredMoveState != PEDMOVE_SPRINT))
			m_panicCounter = 0;
		else if (m_panicCounter < 50)
			++m_panicCounter;

		if (m_fHealth <= 1.0f && m_nPedState <= PED_STATES_NO_AI && !bIsInTheAir && !bIsLanding)
			SetDie(ANIM_STD_KO_FRONT, 4.0f, 0.0f);

		bCollidedWithMyVehicle = false;

		CEntity *collidingEnt = m_pDamageEntity;
#ifndef VC_PED_PORTS
		if (!bUsesCollision || m_fDamageImpulse <= 0.0f || m_nPedState == PED_DIE || !collidingEnt) {
#else
		if (!bUsesCollision || ((!collidingEnt || m_fDamageImpulse <= 0.0f) && (!IsPlayer() || !bIsStuck)) || m_nPedState == PED_DIE) {
#endif
			bHitSomethingLastFrame = false;
			if (m_nPedStateTimer <= 500 && bIsInTheAir) {
				if (m_nPedStateTimer)
					m_nPedStateTimer--;
			} else if (m_nPedStateTimer < 1001) {
				m_nPedStateTimer = 0;
			}
		} else {
			if (m_panicCounter == 50 && IsPedInControl()) {
				SetWaitState(WAITSTATE_STUCK, nil);
				// Leftover
				/*
				if (m_nPedType < PEDTYPE_COP) {

				} else {

				}
				*/
#ifndef VC_PED_PORTS
			} else {
#else
			} else if (collidingEnt) {
#endif
				switch (collidingEnt->GetType())
				{
				case ENTITY_TYPE_BUILDING:
				case ENTITY_TYPE_OBJECT:
				{
					CBaseModelInfo *collidingModel = CModelInfo::GetModelInfo(collidingEnt->GetModelIndex());
					CColModel *collidingCol = collidingModel->GetColModel();
					if (collidingEnt->IsObject() && ((CObject*)collidingEnt)->m_nSpecialCollisionResponseCases != COLLRESPONSE_FENCEPART
						|| collidingCol->boundingBox.max.x < 3.0f
						&& collidingCol->boundingBox.max.y < 3.0f) {

						if (!IsPlayer()) {
							SetDirectionToWalkAroundObject(collidingEnt);
							break;
						}
					}
					if (IsPlayer()) {
						bHitSomethingLastFrame = true;
						break;
					}

					float angleToFaceWhenHit = CGeneral::GetRadianAngleBetweenPoints(
						GetPosition().x,
						GetPosition().y,
						m_vecDamageNormal.x + GetPosition().x,
						m_vecDamageNormal.y + GetPosition().y);

					float neededTurn = Abs(m_fRotationCur - angleToFaceWhenHit);

					if (neededTurn > PI)
						neededTurn = TWOPI - neededTurn;

					float oldDestRot = CGeneral::LimitRadianAngle(m_fRotationDest);

					if (m_pedInObjective &&
						(m_objective == OBJECTIVE_GOTO_CHAR_ON_FOOT || m_objective == OBJECTIVE_KILL_CHAR_ON_FOOT)) {

						if (m_pedInObjective->IsPlayer()
							&& (neededTurn < DEGTORAD(20.0f) || m_panicCounter > 10)) {
							if (CanPedJumpThis(collidingEnt)) {
								SetJump();
							} else if (m_objective == OBJECTIVE_KILL_CHAR_ON_FOOT) {
								SetWaitState(WAITSTATE_LOOK_ABOUT, nil);
							} else {
								SetWaitState(WAITSTATE_PLAYANIM_TAXI, nil);
								m_headingRate = 0.0f;
								SetLookFlag(m_pedInObjective, true);
								SetLookTimer(3000);
								Say(SOUND_PED_TAXI_CALL);
							}
						} else {
							m_pLookTarget = m_pedInObjective;
							m_pLookTarget->RegisterReference((CEntity **) &m_pLookTarget);
							TurnBody();
						}
					} else {
						if (m_nPedType != PEDTYPE_COP && neededTurn < DEGTORAD(15.0f) && m_nWaitState == WAITSTATE_FALSE) {
							if ((m_nStoredMoveState == PEDMOVE_RUN || m_nStoredMoveState == PEDMOVE_SPRINT) && m_vecDamageNormal.z < 0.3f) {
								CAnimBlendAssociation *runAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_RUN);
								if (!runAssoc)
									runAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_RUNFAST);

								if (runAssoc && runAssoc->blendAmount > 0.9f && runAssoc->IsRunning()) {
									SetWaitState(WAITSTATE_HITWALL, nil);
								}
							}
						}
						if (m_nPedState == PED_FLEE_POS) {
							CVector2D fleePos = collidingEnt->GetPosition();
							uint32 oldFleeTimer = m_fleeTimer;
							SetFlee(fleePos, 5000);
							if (oldFleeTimer != m_fleeTimer)
								m_nPedStateTimer = CTimer::GetTimeInMilliseconds() + 500;

						} else {
							if (m_nPedState == PED_FLEE_ENTITY && (neededTurn < DEGTORAD(25.0f) || m_panicCounter > 10)) {
								m_collidingThingTimer = CTimer::GetTimeInMilliseconds() + 2500;
								m_collidingEntityWhileFleeing = collidingEnt;
								m_collidingEntityWhileFleeing->RegisterReference((CEntity **) &m_collidingEntityWhileFleeing);

								uint8 currentDir = Floor((PI + m_fRotationCur) / DEGTORAD(45.0f));
								uint8 nextDir;
								ThePaths.FindNextNodeWandering(PATH_PED, GetPosition(), &m_pLastPathNode, &m_pNextPathNode, currentDir, &nextDir);

							} else {
								if (neededTurn < DEGTORAD(60.0f)) {
									CVector posToHead = m_vecDamageNormal * 4.0f;
									posToHead.z = 0.0f;
									posToHead += GetPosition();
									int closestNodeId = ThePaths.FindNodeClosestToCoors(posToHead, PATH_PED,
										999999.9f, false, false);
									float angleToFace;

									if (m_objective != OBJECTIVE_KILL_CHAR_ON_FOOT && m_objective != OBJECTIVE_KILL_CHAR_ANY_MEANS) {
										if (m_nPedState != PED_SEEK_POS && m_nPedState != PED_SEEK_CAR) {
											if (m_nPedState == PED_WANDER_PATH) {
												m_pNextPathNode = &ThePaths.m_pathNodes[closestNodeId];
												angleToFace = CGeneral::GetRadianAngleBetweenPoints(
													m_pNextPathNode->GetX(), m_pNextPathNode->GetY(),
													GetPosition().x, GetPosition().y);
											} else {
												if (ThePaths.m_pathNodes[closestNodeId].GetX() == 0.0f
													|| ThePaths.m_pathNodes[closestNodeId].GetY() == 0.0f) {
													posToHead = (3.0f * m_vecDamageNormal) + GetPosition();
													posToHead.x += (CGeneral::GetRandomNumber() % 512) / 250.0f - 1.0f;
													posToHead.y += (CGeneral::GetRandomNumber() % 512) / 250.0f - 1.0f;
												} else {
													posToHead.x = ThePaths.m_pathNodes[closestNodeId].GetX();
													posToHead.y = ThePaths.m_pathNodes[closestNodeId].GetY();
												}
												angleToFace = CGeneral::GetRadianAngleBetweenPoints(
													posToHead.x, posToHead.y,
													GetPosition().x, GetPosition().y);

												if (m_nPedState != PED_FOLLOW_PATH)
													m_nPedStateTimer = CTimer::GetTimeInMilliseconds() + 500;
											}
										} else {
											angleToFace = CGeneral::GetRadianAngleBetweenPoints(
												ThePaths.m_pathNodes[closestNodeId].GetX(),
												ThePaths.m_pathNodes[closestNodeId].GetY(),
												GetPosition().x,
												GetPosition().y);

											CVector2D distToNode = ThePaths.m_pathNodes[closestNodeId].GetPosition() - GetPosition();
											CVector2D distToSeekPos = m_vecSeekPos - GetPosition();

											if (DotProduct2D(distToNode, distToSeekPos) < 0.0f) {
												m_fRotationCur = m_fRotationDest;
												break;
											}
										}
									} else {
										float angleToFaceAwayDamage = CGeneral::GetRadianAngleBetweenPoints(
											m_vecDamageNormal.x,
											m_vecDamageNormal.y,
											0.0f,
											0.0f);

										if (angleToFaceAwayDamage < m_fRotationCur)
											angleToFaceAwayDamage += TWOPI;

										float neededTurn = angleToFaceAwayDamage - m_fRotationCur;

										if (neededTurn <= PI) {
											angleToFace = 0.5f * neededTurn + m_fRotationCur;
											m_fRotationCur += DEGTORAD(m_pedStats->m_headingChangeRate) * 2.0f;
										} else {
											angleToFace = m_fRotationCur - (TWOPI - neededTurn) * 0.5f;
											m_fRotationCur -= DEGTORAD(m_pedStats->m_headingChangeRate) * 2.0f;
										}

										m_nPedStateTimer = CTimer::GetTimeInMilliseconds() + 200;
										if (m_nPedType == PEDTYPE_COP) {
											if (m_pedInObjective) {
												float angleToLookCriminal = CGeneral::GetRadianAngleBetweenPoints(
													m_pedInObjective->GetPosition().x,
													m_pedInObjective->GetPosition().y,
													GetPosition().x,
													GetPosition().y);

												angleToLookCriminal = CGeneral::LimitRadianAngle(angleToLookCriminal);
												angleToFace = CGeneral::LimitRadianAngle(angleToFace);

												if (angleToLookCriminal < angleToFace)
													angleToLookCriminal += TWOPI;

												float neededTurnToCriminal = angleToLookCriminal - angleToFace;

												if (neededTurnToCriminal > DEGTORAD(150.0f) && neededTurnToCriminal < DEGTORAD(210.0f)) {
													((CCopPed*)this)->m_bStopAndShootDisabledZone = true;
												}
											}
										}
									}
									m_fRotationDest = CGeneral::LimitRadianAngle(angleToFace);

									if (m_fRotationCur - PI > m_fRotationDest) {
										m_fRotationDest += TWOPI;
									} else if (PI + m_fRotationCur < m_fRotationDest) {
										m_fRotationDest -= TWOPI;
									}

									if (oldDestRot == m_fRotationDest && CTimer::GetTimeInMilliseconds() > m_nPedStateTimer) {
										m_nPedStateTimer = CTimer::GetTimeInMilliseconds() + 200;
										m_fRotationDest += HALFPI;
									}
								}
							}
						}
					}

					if (m_nPedState != PED_WANDER_PATH && m_nPedState != PED_FLEE_ENTITY)
						m_pNextPathNode = nil;

					bHitSomethingLastFrame = true;
					break;
				}
				case ENTITY_TYPE_VEHICLE:
				{
					CVehicle* collidingVeh = ((CVehicle*)collidingEnt);
					float collidingVehSpeedSqr = collidingVeh->m_vecMoveSpeed.MagnitudeSqr();

					if (collidingVeh == m_pMyVehicle)
						bCollidedWithMyVehicle = true;
#ifdef VC_PED_PORTS
					float oldHealth = m_fHealth;
					bool playerSufferSound = false;

					if (collidingVehSpeedSqr <= 1.0f / 400.0f) {
						if (IsPedInControl()
							&& (!IsPlayer()
								|| m_objective == OBJECTIVE_GOTO_AREA_ON_FOOT
								|| m_objective == OBJECTIVE_RUN_TO_AREA
								|| m_objective == OBJECTIVE_ENTER_CAR_AS_DRIVER)) {

							if (collidingVeh != m_pCurrentPhysSurface || IsPlayer()) {
								if (!bVehEnterDoorIsBlocked) {
									if (collidingVeh->GetStatus() != STATUS_PLAYER || CharCreatedBy == MISSION_CHAR) {

										// VC calls SetDirectionToWalkAroundVehicle instead if ped is in PED_SEEK_CAR.
										SetDirectionToWalkAroundObject(collidingVeh);
										CWorld::Players[CWorld::PlayerInFocus].m_nLastBumpPlayerCarTimer = m_nPedStateTimer;
									} else {
										if (CTimer::GetTimeInMilliseconds() >= CWorld::Players[CWorld::PlayerInFocus].m_nLastBumpPlayerCarTimer
											|| m_nPedStateTimer >= CTimer::GetTimeInMilliseconds()) {

											// VC calls SetDirectionToWalkAroundVehicle instead if ped is in PED_SEEK_CAR.
											SetDirectionToWalkAroundObject(collidingVeh);
											CWorld::Players[CWorld::PlayerInFocus].m_nLastBumpPlayerCarTimer = m_nPedStateTimer;

										} else if (m_fleeFrom != collidingVeh) {
											SetFlee(collidingVeh, 4000);
											bUsePedNodeSeek = false;
											SetMoveState(PEDMOVE_WALK);
										}
									}
								}
							} else {
								float angleLeftToCompleteTurn = Abs(m_fRotationCur - m_fRotationDest);
								if (angleLeftToCompleteTurn < 0.01f && CanPedJumpThis(collidingVeh)) {
									SetJump();
								}
							}
						} else if (IsPlayer() && !bIsInTheAir) {

							if (IsPedInControl() && ((CPlayerPed*)this)->m_fMoveSpeed == 0.0f
								&& !bIsLooking && CTimer::GetTimeInMilliseconds() > m_lookTimer && collidingVeh->pDriver) {

								((CPlayerPed*)this)->AnnoyPlayerPed(false);
								SetLookFlag(collidingVeh, true);
								SetLookTimer(1300);

								eWeaponType weaponType = GetWeapon()->m_eWeaponType;
								if (weaponType == WEAPONTYPE_UNARMED
									|| weaponType == WEAPONTYPE_BASEBALLBAT
									|| weaponType == WEAPONTYPE_COLT45
									|| weaponType == WEAPONTYPE_UZI) {
									bShakeFist = true;
								}
							} else {
								SetLookFlag(collidingVeh, true);
								SetLookTimer(500);
							}
						}
					} else {
						float adjustedImpulse = m_fDamageImpulse;
						if (IsPlayer()) {
							if (bIsStanding) {
								float forwardVecAndDamageDirDotProd = DotProduct(m_vecAnimMoveDelta.y * GetForward(), m_vecDamageNormal);
								if (forwardVecAndDamageDirDotProd < 0.0f) {
									adjustedImpulse = forwardVecAndDamageDirDotProd * m_fMass + m_fDamageImpulse;
									if (adjustedImpulse < 0.0f)
										adjustedImpulse = 0.0f;
								}
							}
						}
						if (m_fMass / 20.0f < adjustedImpulse)
							DMAudio.PlayOneShot(collidingVeh->m_audioEntityId, SOUND_CAR_PED_COLLISION, adjustedImpulse);

						if (IsPlayer()) {
							if (adjustedImpulse > 20.0f)
								adjustedImpulse = 20.0f;

							if (adjustedImpulse > 5.0f) {
								if (adjustedImpulse <= 13.0f)
									playerSufferSound = true;
								else
									Say(SOUND_PED_DAMAGE);
							}

							CColModel* collidingCol = CModelInfo::GetModelInfo(collidingVeh->m_modelIndex)->GetColModel();
							CVector colMinVec = collidingCol->boundingBox.min;
							CVector colMaxVec = collidingCol->boundingBox.max;

							CVector vehColCenterDist = collidingVeh->GetMatrix() * ((colMinVec + colMaxVec) * 0.5f) - GetPosition();

							// TLVC = To look vehicle center

							float angleToVehFront = collidingVeh->GetForward().Heading();
							float angleDiffFromLookingFrontTLVC = angleToVehFront - vehColCenterDist.Heading();
							angleDiffFromLookingFrontTLVC = CGeneral::LimitRadianAngle(angleDiffFromLookingFrontTLVC);

							// I don't know why do we use that
							float vehTopRightHeading = Atan2(colMaxVec.x - colMinVec.x, colMaxVec.y - colMinVec.y);

							CVector vehDist = GetPosition() - collidingVeh->GetPosition();
							vehDist.Normalise();

							float vehRightVecAndSpeedDotProd;

							if (Abs(angleDiffFromLookingFrontTLVC) >= vehTopRightHeading && Abs(angleDiffFromLookingFrontTLVC) < PI - vehTopRightHeading) {
								if (angleDiffFromLookingFrontTLVC <= 0.0f) {
									vehRightVecAndSpeedDotProd = DotProduct(collidingVeh->GetRight(), collidingVeh->m_vecMoveSpeed);

									// vehRightVecAndSpeedDotProd < 0.1f = Vehicle being overturned or spinning to it's right?
									if (collidingVehSpeedSqr > 1.0f / 100.0f && vehRightVecAndSpeedDotProd < 0.1f) {

										// Car's right faces towards us and isn't coming directly to us
										if (DotProduct(collidingVeh->GetRight(), GetForward()) < 0.0f
											&& DotProduct(vehDist, collidingVeh->m_vecMoveSpeed) > 0.0f) {
											SetEvasiveStep(collidingVeh, 1);
										}
									}
								} else {
									vehRightVecAndSpeedDotProd = DotProduct(-1.0f * collidingVeh->GetRight(), collidingVeh->m_vecMoveSpeed);

									if (collidingVehSpeedSqr > 1.0f / 100.0f && vehRightVecAndSpeedDotProd < 0.1f) {
										if (DotProduct(collidingVeh->GetRight(), GetForward()) > 0.0f
											&& DotProduct(vehDist, collidingVeh->m_vecMoveSpeed) > 0.0f) {
											SetEvasiveStep(collidingVeh, 1);
										}
									}
								}
							} else {
								vehRightVecAndSpeedDotProd = DotProduct(vehDist, collidingVeh->m_vecMoveSpeed);
							}

							if (vehRightVecAndSpeedDotProd <= 0.1f) {
								if (m_nPedState != PED_FIGHT) {
									SetLookFlag(collidingVeh, true);
									SetLookTimer(700);
								}
							} else {
								bIsStanding = false;
								CVector2D collidingEntMoveDir = -collidingVeh->m_vecMoveSpeed;
								int dir = GetLocalDirection(collidingEntMoveDir);
								SetFall(1000, (AnimationId)(dir + ANIM_STD_HIGHIMPACT_FRONT), false);

								float damage;
								if (collidingVeh->m_modelIndex == MI_TRAIN) {
									damage = 50.0f;
								} else {
									damage = 20.0f;
								}

								InflictDamage(collidingVeh, WEAPONTYPE_RAMMEDBYCAR, damage, PEDPIECE_TORSO, dir);
								Say(SOUND_PED_DAMAGE);
							}
						} else {
							KillPedWithCar(collidingVeh, m_fDamageImpulse);
						}
						
						/* VC specific
						if (m_pCollidingEntity != collidingEnt)
							bPushedAlongByCar = true;
						*/
					}
					if (m_fHealth < oldHealth && playerSufferSound)
						Say(SOUND_PED_HIT);
#else
					if (collidingVehSpeedSqr <= 1.0f / 400.0f) {
						if (!IsPedInControl()
							|| IsPlayer()
							&& m_objective != OBJECTIVE_GOTO_AREA_ON_FOOT
							&& m_objective != OBJECTIVE_ENTER_CAR_AS_DRIVER
							&& m_objective != OBJECTIVE_RUN_TO_AREA) {

							if (IsPlayer() && !bIsInTheAir) {

								if (IsPedInControl()
									&& ((CPlayerPed*)this)->m_fMoveSpeed == 0.0f
									&& !bIsLooking
									&& CTimer::GetTimeInMilliseconds() > m_lookTimer
									&& collidingVeh->pDriver) {

									((CPlayerPed*)this)->AnnoyPlayerPed(false);
									SetLookFlag(collidingVeh, true);
									SetLookTimer(1300);

									eWeaponType weaponType = GetWeapon()->m_eWeaponType;
									if (weaponType == WEAPONTYPE_UNARMED
										|| weaponType == WEAPONTYPE_BASEBALLBAT
										|| weaponType == WEAPONTYPE_COLT45
										|| weaponType == WEAPONTYPE_UZI) {
										bShakeFist = true;
									}
								} else {
									SetLookFlag(collidingVeh, true);
									SetLookTimer(500);
								}
							}

						} else if (!bVehEnterDoorIsBlocked) {
							if (collidingVeh->GetStatus() != STATUS_PLAYER || CharCreatedBy == MISSION_CHAR) {

								SetDirectionToWalkAroundObject(collidingVeh);

							} else if (CTimer::GetTimeInMilliseconds() >= CWorld::Players[CWorld::PlayerInFocus].m_nLastBumpPlayerCarTimer
								|| m_nPedStateTimer >= CTimer::GetTimeInMilliseconds()) {

								SetDirectionToWalkAroundObject(collidingVeh);
								CWorld::Players[CWorld::PlayerInFocus].m_nLastBumpPlayerCarTimer = m_nPedStateTimer;

							} else if (m_fleeFrom != collidingVeh) {
								SetFlee(collidingVeh, 4000);
								bUsePedNodeSeek = false;
								SetMoveState(PEDMOVE_WALK);
							}
						}
					} else {
						DMAudio.PlayOneShot(collidingVeh->m_audioEntityId, SOUND_CAR_PED_COLLISION, m_fDamageImpulse);
						if (IsPlayer()) {
							CColModel *collidingCol = CModelInfo::GetModelInfo(collidingVeh->GetModelIndex())->GetColModel();
							CVector colMinVec = collidingCol->boundingBox.min;
							CVector colMaxVec = collidingCol->boundingBox.max;

							CVector vehColCenterDist = collidingVeh->GetMatrix() * ((colMinVec + colMaxVec) * 0.5f) - GetPosition();

							// TLVC = To look vehicle center

							float angleToVehFront = collidingVeh->GetForward().Heading();
							float angleDiffFromLookingFrontTLVC = angleToVehFront - vehColCenterDist.Heading();
							angleDiffFromLookingFrontTLVC = CGeneral::LimitRadianAngle(angleDiffFromLookingFrontTLVC);

							// I don't know why do we use that
							float vehTopRightHeading = Atan2(colMaxVec.x - colMinVec.x, colMaxVec.y - colMinVec.y);

							CVector vehDist = GetPosition() - collidingVeh->GetPosition();
							vehDist.Normalise();

							float vehRightVecAndSpeedDotProd;

							if (Abs(angleDiffFromLookingFrontTLVC) >= vehTopRightHeading && Abs(angleDiffFromLookingFrontTLVC) < PI - vehTopRightHeading) {
								if (angleDiffFromLookingFrontTLVC <= 0.0f) {
									vehRightVecAndSpeedDotProd = DotProduct(collidingVeh->GetRight(), collidingVeh->m_vecMoveSpeed);

									// vehRightVecAndSpeedDotProd < 0.1f = Vehicle being overturned or spinning to it's right?
									if (collidingVehSpeedSqr > 1.0f / 100.0f && vehRightVecAndSpeedDotProd < 0.1f) {

										// Car's right faces towards us and isn't coming directly to us
										if (DotProduct(collidingVeh->GetRight(), GetForward()) < 0.0f
											&& DotProduct(vehDist, collidingVeh->m_vecMoveSpeed) > 0.0f) {
											SetEvasiveStep(collidingVeh, 1);
										}
									}
								} else {
									vehRightVecAndSpeedDotProd = DotProduct(-1.0f * collidingVeh->GetRight(), collidingVeh->m_vecMoveSpeed);

									if (collidingVehSpeedSqr > 1.0f / 100.0f && vehRightVecAndSpeedDotProd < 0.1f) {
										if (DotProduct(collidingVeh->GetRight(), GetForward()) > 0.0f
											&& DotProduct(vehDist, collidingVeh->m_vecMoveSpeed) > 0.0f) {
											SetEvasiveStep(collidingVeh, 1);
										}
									}
								}
							} else {
								vehRightVecAndSpeedDotProd = DotProduct(vehDist, collidingVeh->m_vecMoveSpeed);
							}

							if (vehRightVecAndSpeedDotProd <= 0.1f) {
								if (m_nPedState != PED_FIGHT) {
									SetLookFlag(collidingVeh, true);
									SetLookTimer(700);
								}
							} else {
								bIsStanding = false;
								CVector2D collidingEntMoveDir = -collidingVeh->m_vecMoveSpeed;
								int dir = GetLocalDirection(collidingEntMoveDir);
								SetFall(1000, (AnimationId)(dir + ANIM_STD_HIGHIMPACT_FRONT), false);
								CPed *driver = collidingVeh->pDriver;

								float damage;
								if (driver && driver->IsPlayer()) {
									damage = vehRightVecAndSpeedDotProd * 1000.0f;
								} else if (collidingVeh->GetModelIndex() == MI_TRAIN) {
									damage = 50.0f;
								} else {
									damage = 20.0f;
								}

								InflictDamage(collidingVeh, WEAPONTYPE_RAMMEDBYCAR, damage, PEDPIECE_TORSO, dir);
								Say(SOUND_PED_DAMAGE);
							}
						} else {
							KillPedWithCar(collidingVeh, m_fDamageImpulse);
						}
					}
#endif
					break;
				}
				case ENTITY_TYPE_PED:
				{
					CollideWithPed((CPed*)collidingEnt);
					if (((CPed*)collidingEnt)->IsPlayer()) {
						CPlayerPed *player = ((CPlayerPed*)collidingEnt);
						Say(SOUND_PED_CHAT);
						if (m_nMoveState > PEDMOVE_STILL && player->IsPedInControl()) {
							if (player->m_fMoveSpeed < 1.0f) {
								if (!player->bIsLooking) {
									if (CTimer::GetTimeInMilliseconds() > player->m_lookTimer) {
										player->AnnoyPlayerPed(false);
										player->SetLookFlag(this, true);
										player->SetLookTimer(1300);
										eWeaponType weapon = player->GetWeapon()->m_eWeaponType;
										if (weapon == WEAPONTYPE_UNARMED
											|| weapon == WEAPONTYPE_BASEBALLBAT
											|| weapon == WEAPONTYPE_COLT45
											|| weapon == WEAPONTYPE_UZI) {
											player->bShakeFist = true;
										}
									}
								}
							}
						}
					}
					break;
				}
				default:
					break;
				}
			}
			CVector forceDir;
			if (!bIsInTheAir && m_nPedState != PED_JUMP
#ifdef VC_PED_PORTS
				&& m_fDamageImpulse > 0.0f
#endif
				) {

				forceDir = m_vecDamageNormal;
				forceDir.z = 0.0f;
				if (!bIsStanding) {
					forceDir *= 4.0f;
				} else {
					forceDir *= 0.5f;
				}

				ApplyMoveForce(forceDir);
			}
			if ((bIsInTheAir && !DyingOrDead())
#ifdef VC_PED_PORTS
				|| (!bIsStanding && !bWasStanding && m_nPedState == PED_FALL)
#endif		
			) {
				if (m_nPedStateTimer > 0 && m_nPedStateTimer <= 1000) {
					forceDir = GetPosition() - m_vecHitLastPos;
				} else {
					m_nPedStateTimer = 0;
					m_vecHitLastPos = GetPosition();
					forceDir = CVector(0.0f, 0.0f, 0.0f);
				}

				CVector offsetToCheck;
				m_nPedStateTimer++;

				float adjustedTs = Max(CTimer::GetTimeStep(), 0.01f);

				CPad *pad0 = CPad::GetPad(0);
				if ((m_nPedStateTimer <= 50.0f / (4.0f * adjustedTs) || m_nPedStateTimer * 0.01f <= forceDir.MagnitudeSqr())
					&& (m_nCollisionRecords <= 1 || m_nPedStateTimer <= 50.0f / (2.0f * adjustedTs) || m_nPedStateTimer * 1.0f / 250.0f <= Abs(forceDir.z))) {

					if (m_nCollisionRecords == 1 && m_aCollisionRecords[0] != nil && m_aCollisionRecords[0]->IsBuilding()
						&& m_nPedStateTimer > 50.0f / (2.0f * adjustedTs) && m_nPedStateTimer * 1.0f / 250.0f > Abs(forceDir.z)) {
						offsetToCheck.x = -forceDir.y;
#ifdef VC_PED_PORTS
						offsetToCheck.z = 1.0f;
#else
						offsetToCheck.z = 0.0f;
#endif
						offsetToCheck.y = forceDir.x;
						offsetToCheck.Normalise();

						CVector posToCheck = GetPosition() + offsetToCheck;

						// These are either obstacle or ground to land, I don't know which one.
						float obstacleForFlyingZ, obstacleForFlyingOtherDirZ;
						CColPoint obstacleForFlying, obstacleForFlyingOtherDir;

						// Check is there any room for being knocked up in reverse direction of force
						if (CWorld::ProcessVerticalLine(posToCheck, -20.0f, obstacleForFlying, foundEnt, true, false, false, false, false, false, nil)) {
							obstacleForFlyingZ = obstacleForFlying.point.z;
						} else {
							obstacleForFlyingZ = 500.0f;
						}
						
						posToCheck = GetPosition() - offsetToCheck;

						// Now check for direction of force this time
						if (CWorld::ProcessVerticalLine(posToCheck, -20.0f, obstacleForFlyingOtherDir, foundEnt, true, false, false, false, false, false, nil)) {
							obstacleForFlyingOtherDirZ = obstacleForFlyingOtherDir.point.z;
						} else {
							obstacleForFlyingOtherDirZ = 501.0f;
						}
#ifdef VC_PED_PORTS
						uint8 flyDir = 0;
						float feetZ = GetPosition().z - FEET_OFFSET;
#ifdef FIX_BUGS
						if (obstacleForFlyingZ > feetZ && obstacleForFlyingOtherDirZ < 501.0f)
							flyDir = 1;
						else if (obstacleForFlyingOtherDirZ > feetZ && obstacleForFlyingZ < 500.0f)
							flyDir = 2;
#else
						if ((obstacleForFlyingZ > feetZ && obstacleForFlyingOtherDirZ < 500.0f) || (obstacleForFlyingZ > feetZ && obstacleForFlyingOtherDirZ > feetZ))
							flyDir = 1;
						else if (obstacleForFlyingOtherDirZ > feetZ && obstacleForFlyingZ < 499.0f)
							flyDir = 2;
#endif

						if (flyDir != 0 && !bSomeVCflag1) {
							SetPosition((flyDir == 2 ? obstacleForFlyingOtherDir.point : obstacleForFlying.point));
							GetMatrix().GetPosition().z += FEET_OFFSET;
							GetMatrix().UpdateRW();
							SetLanding();
							bIsStanding = true;
						}
#endif
						if (obstacleForFlyingZ < obstacleForFlyingOtherDirZ) {
							offsetToCheck *= -1.0f;
						}
						offsetToCheck.z = 1.0f;
						forceDir = 4.0f * offsetToCheck;
						forceDir.z = 4.0f;
						ApplyMoveForce(forceDir);

						// What was that for?? It pushes player inside of collision sometimes and kills him.
#ifdef FIX_BUGS
						if (!IsPlayer())
#endif
						GetMatrix().GetPosition() += 0.25f * offsetToCheck;

						m_fRotationCur = CGeneral::GetRadianAngleBetweenPoints(offsetToCheck.x, offsetToCheck.y, 0.0f, 0.0f);
						m_fRotationCur = CGeneral::LimitRadianAngle(m_fRotationCur);
						m_fRotationDest = m_fRotationCur;
						SetHeading(m_fRotationCur);

						if (m_nPedState != PED_FALL && !bIsPedDieAnimPlaying) {
							SetFall(1000, ANIM_STD_HIGHIMPACT_BACK, true);
						}
						bIsInTheAir = false;
					} else if (m_vecDamageNormal.z > 0.4f) {
#ifndef VC_PED_PORTS
						forceDir = m_vecDamageNormal;
						forceDir.z = 0.0f;
						forceDir.Normalise();
						ApplyMoveForce(2.0f * forceDir);
#else
						if (m_nPedState == PED_JUMP) {
							if (m_nWaitTimer <= 2000) {
								if (m_nWaitTimer < 1000)
									m_nWaitTimer += CTimer::GetTimeStep() * 0.02f * 1000.0f;
							} else {
								m_nWaitTimer = 0;
							}
						}
						forceDir = m_vecDamageNormal;
						forceDir.z = 0.0f;
						forceDir.Normalise();
						if (m_nPedState != PED_JUMP || m_nWaitTimer >= 300) {
							ApplyMoveForce(2.0f * forceDir);
						} else {
							ApplyMoveForce(-4.0f * forceDir);
						}
#endif
					}
				} else if ((CTimer::GetFrameCounter() + m_randomSeed % 256 + 3) & 7) {
					if (IsPlayer() && m_nPedState != PED_JUMP && pad0->JumpJustDown()) {
						int16 padWalkX = pad0->GetPedWalkLeftRight();
						int16 padWalkY = pad0->GetPedWalkUpDown();
						if (Abs(padWalkX) > 0.0f || Abs(padWalkY) > 0.0f) {
							m_fRotationDest = CGeneral::GetRadianAngleBetweenPoints(0.0f, 0.0f, -padWalkX, padWalkY);
							m_fRotationDest -= TheCamera.Orientation;
							m_fRotationDest = CGeneral::LimitRadianAngle(m_fRotationDest);
							m_fRotationCur = m_fRotationDest;
							SetHeading(m_fRotationCur);
						}
						SetJump();
						m_nPedStateTimer = 0;
						m_vecHitLastPos = GetPosition();

						// Why? forceDir is unused after this point.
						forceDir = CVector(0.0f, 0.0f, 0.0f);
					} else if (IsPlayer()) {
						int16 padWalkX = pad0->GetPedWalkLeftRight();
						int16 padWalkY = pad0->GetPedWalkUpDown();
						if (Abs(padWalkX) > 0.0f || Abs(padWalkY) > 0.0f) {
							m_fRotationDest = CGeneral::GetRadianAngleBetweenPoints(0.0f, 0.0f, -padWalkX, padWalkY);
							m_fRotationDest -= TheCamera.Orientation;
							m_fRotationDest = CGeneral::LimitRadianAngle(m_fRotationDest);
							m_fRotationCur = m_fRotationDest;
							SetHeading(m_fRotationCur);
						}
						CAnimBlendAssociation *jumpAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_JUMP_GLIDE);

						if (!jumpAssoc)
							jumpAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_FALL_GLIDE);

						if (jumpAssoc) {
							jumpAssoc->blendDelta = -3.0f;
							jumpAssoc->flags |= ASSOC_DELETEFADEDOUT;
						}
						if (m_nPedState == PED_JUMP)
							m_nPedState = PED_IDLE;
					} else {
						CAnimBlendAssociation *jumpAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_JUMP_GLIDE);

						if (!jumpAssoc)
							jumpAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_FALL_GLIDE);

						if (jumpAssoc) {
							jumpAssoc->blendDelta = -3.0f;
							jumpAssoc->flags |= ASSOC_DELETEFADEDOUT;
						}
					}
				} else {
					offsetToCheck = GetPosition();
					offsetToCheck.z += 0.5f;

					if (CWorld::ProcessVerticalLine(offsetToCheck, GetPosition().z - FEET_OFFSET, foundCol, foundEnt, true, true, false, true, false, false, nil)) {
#ifdef VC_PED_PORTS
						if (!bSomeVCflag1 || FEET_OFFSET + foundCol.point.z < GetPosition().z) {
							GetMatrix().GetPosition().z = FEET_OFFSET + foundCol.point.z;
							GetMatrix().UpdateRW();
							if (bSomeVCflag1)
								bSomeVCflag1 = false;
						}
#else
						GetMatrix().GetPosition().z = FEET_OFFSET + foundCol.point.z;
						GetMatrix().UpdateRW();
#endif
						SetLanding();
						bIsStanding = true;
					}
				}
			} else if (m_nPedStateTimer < 1001) {
				m_nPedStateTimer = 0;
			}
		}

		if (bIsDucking)
			Duck();

		if (bStartWanderPathOnFoot) {
			if (IsPedInControl()) {
				ClearAll();
				SetWanderPath(m_nPathDir);
				bStartWanderPathOnFoot = false;
			} else if (m_nPedState == PED_DRIVING) {
				bWanderPathAfterExitingCar = true;
				SetObjective(OBJECTIVE_LEAVE_CAR, m_pMyVehicle);
			}
		}

		if (!bIsStanding && m_vecMoveSpeed.z > 0.25f) {
			float airResistance = Pow(0.95f, CTimer::GetTimeStep());

			m_vecMoveSpeed *= airResistance;
		}
#ifdef VC_PED_PORTS
		if (IsPlayer() || !bIsStanding || m_vecMoveSpeed.x != 0.0f || m_vecMoveSpeed.y != 0.0f || m_vecMoveSpeed.z != 0.0f
			|| (m_nMoveState != PEDMOVE_NONE && m_nMoveState != PEDMOVE_STILL)
			|| m_vecAnimMoveDelta.x != 0.0f || m_vecAnimMoveDelta.y != 0.0f
			|| m_nPedState == PED_JUMP
			|| bIsInTheAir
			|| m_pCurrentPhysSurface) {

			CPhysical::ProcessControl();
		} else {
			bHasContacted = false;
			bIsInSafePosition = false;
			bWasPostponed = false;
			bHasHitWall = false;
			m_nCollisionRecords = 0;
			bHasCollided = false;
			m_nDamagePieceType = 0;
			m_fDamageImpulse = 0.0f;
			m_pDamageEntity = nil;
			m_vecTurnFriction = CVector(0.0f, 0.0f, 0.0f);
			m_vecMoveFriction = CVector(0.0f, 0.0f, 0.0f);
		}
#else
		CPhysical::ProcessControl();
#endif
		if (m_nPedState != PED_DIE || bIsPedDieAnimPlaying) {
			if (m_nPedState != PED_DEAD) {
				CalculateNewVelocity();
				CalculateNewOrientation();
			}
			UpdatePosition();
			PlayFootSteps();
			if (IsPedInControl() && !bIsStanding && !m_pDamageEntity && CheckIfInTheAir()) {
				SetInTheAir();
#ifdef VC_PED_PORTS
				bSomeVCflag1 = false;
#endif
			}
#ifdef VC_PED_PORTS
			if (bSomeVCflag1) {
				CVector posToCheck = GetPosition();
				posToCheck.z += 0.9f;
				if (!CWorld::TestSphereAgainstWorld(posToCheck, 0.2f, this, true, true, false, true, false, false))
					bSomeVCflag1 = false;
			}
#endif
			ProcessObjective();
			if (!bIsAimingGun) {
				if (bIsRestoringGun)
					RestoreGunPosition();
			} else {
				AimGun();
			}

			if (bIsLooking) {
				MoveHeadToLook();
			} else if (bIsRestoringLook) {
				RestoreHeadPosition();
			}

			if (bIsInTheAir)
				InTheAir();

			if (bUpdateAnimHeading) {
				if (m_nPedState != PED_GETUP && m_nPedState != PED_FALL) {
					m_fRotationCur -= HALFPI;
					m_fRotationDest = m_fRotationCur;
					bUpdateAnimHeading = false;
				}
			}

			if (m_nWaitState != WAITSTATE_FALSE)
				Wait();

			if (m_nPedState != PED_IDLE) {
				CAnimBlendAssociation *idleAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_IDLE_BIGGUN);
				if(idleAssoc) {
					idleAssoc->blendDelta = -8.0f;
					idleAssoc->flags |= ASSOC_DELETEFADEDOUT;
				}
			}

#ifdef CANCELLABLE_CAR_ENTER
			static bool cancelJack = false;
			if (IsPlayer()) {
				if (EnteringCar() && m_pVehicleAnim) {
					CPad *pad = CPad::GetPad(0);

					if (!pad->ArePlayerControlsDisabled()) {
						int vehAnim = m_pVehicleAnim->animId;

						int16 padWalkX = pad->GetPedWalkLeftRight();
						int16 padWalkY = pad->GetPedWalkUpDown();
						if (Abs(padWalkX) > 0.0f || Abs(padWalkY) > 0.0f) {
							if (vehAnim == ANIM_STD_CAR_OPEN_DOOR_LHS || vehAnim == ANIM_STD_CAR_OPEN_DOOR_RHS || vehAnim == ANIM_STD_COACH_OPEN_LHS || vehAnim == ANIM_STD_COACH_OPEN_RHS ||
								vehAnim == ANIM_STD_VAN_OPEN_DOOR_REAR_LHS || vehAnim == ANIM_STD_VAN_OPEN_DOOR_REAR_RHS) {

								if (!m_pMyVehicle->pDriver) {
									cancelJack = false;
									bCancelEnteringCar = true;
								} else
									cancelJack = true;
							} else if (vehAnim == ANIM_STD_QUICKJACK && m_pVehicleAnim->GetTimeLeft() > 0.75f) {
								cancelJack = true;
							} else if (vehAnim == ANIM_STD_CAR_PULL_OUT_PED_LHS || vehAnim == ANIM_STD_CAR_PULL_OUT_PED_LO_LHS || vehAnim == ANIM_STD_CAR_PULL_OUT_PED_LO_RHS || vehAnim == ANIM_STD_CAR_PULL_OUT_PED_RHS) {
								bCancelEnteringCar = true;
								cancelJack = false;
							}
						}
						if (cancelJack && vehAnim == ANIM_STD_QUICKJACK && m_pVehicleAnim->GetTimeLeft() > 0.75f && m_pVehicleAnim->GetTimeLeft() < 0.78f) {
							cancelJack = false;
							QuitEnteringCar();
							RestorePreviousObjective();
						}
						if (cancelJack && (vehAnim == ANIM_STD_CAR_PULL_OUT_PED_LHS || vehAnim == ANIM_STD_CAR_PULL_OUT_PED_LO_LHS || vehAnim == ANIM_STD_CAR_PULL_OUT_PED_LO_RHS || vehAnim == ANIM_STD_CAR_PULL_OUT_PED_RHS)) {
							cancelJack = false;
							bCancelEnteringCar = true;
						}
					}
				} else
					cancelJack = false;
			}
#endif

			switch (m_nPedState) {
				case PED_IDLE:
					Idle();
					break;
				case PED_LOOK_ENTITY:
				case PED_LOOK_HEADING:
					Look();
					break;
				case PED_WANDER_RANGE:
					WanderRange();
					CheckAroundForPossibleCollisions();
					break;
				case PED_WANDER_PATH:
					WanderPath();
					break;
				case PED_ENTER_CAR:
				case PED_CARJACK:
					break;
				case PED_FLEE_POS:
					ms_vec2DFleePosition.x = m_fleeFromPosX;
					ms_vec2DFleePosition.y = m_fleeFromPosY;
					Flee();
					break;
				case PED_FLEE_ENTITY:
					if (!m_fleeFrom) {
						SetIdle();
						break;
					}

					if (CTimer::GetTimeInMilliseconds() <= m_nPedStateTimer)
						break;

					ms_vec2DFleePosition = m_fleeFrom->GetPosition();
					Flee();
					break;
				case PED_FOLLOW_PATH:
					FollowPath();
					break;
				case PED_PAUSE:
					Pause();
					break;
				case PED_ATTACK:
					Attack();
					break;
				case PED_FIGHT:
					Fight();
					break;
				case PED_CHAT:
					Chat();
					break;
				case PED_AIM_GUN:
					if (m_pPointGunAt && m_pPointGunAt->IsPed()
#ifdef FIX_BUGS
				        && !GetWeapon()->IsTypeMelee()
#endif
						&& ((CPed*)m_pPointGunAt)->CanSeeEntity(this, CAN_SEE_ENTITY_ANGLE_THRESHOLD * 2)) {
						((CPed*)m_pPointGunAt)->ReactToPointGun(this);
					}
					PointGunAt();
					break;
				case PED_SEEK_CAR:
					SeekCar();
					break;
				case PED_SEEK_IN_BOAT:
					SeekBoatPosition();
					break;
				case PED_INVESTIGATE:
					InvestigateEvent();
					break;
				case PED_ON_FIRE:
					if (IsPlayer())
						break;

					if (CTimer::GetTimeInMilliseconds() <= m_fleeTimer) {
						if (m_fleeFrom) {
							ms_vec2DFleePosition = m_fleeFrom->GetPosition();
						} else {
							ms_vec2DFleePosition.x = m_fleeFromPosX;
							ms_vec2DFleePosition.y = m_fleeFromPosY;
						}
						Flee();
					} else {
						if (m_pFire)
							m_pFire->Extinguish();
					}
					break;
				case PED_FALL:
					Fall();
					break;
				case PED_GETUP:
					SetGetUp();
					break;
				case PED_ENTER_TRAIN:
					EnterTrain();
					break;
				case PED_EXIT_TRAIN:
					ExitTrain();
					break;
				case PED_DRIVING:
				{
					if (!m_pMyVehicle) {
						bInVehicle = false;
						FlagToDestroyWhenNextProcessed();
						return;
					}

					if (m_pMyVehicle->pDriver != this || m_pMyVehicle->IsBoat()) {
						LookForSexyPeds();
						LookForSexyCars();
						break;
					}

					if (m_pMyVehicle->m_vehType == VEHICLE_TYPE_BIKE || !m_pMyVehicle->pDriver->IsPlayer()) {
						break;
					}

					CPad* pad = CPad::GetPad(0);

#ifdef CAR_AIRBREAK
					if (!pad->ArePlayerControlsDisabled()) {
						if (pad->GetHorn()) {
							float c = Cos(m_fRotationCur);
							float s = Sin(m_fRotationCur);
							m_pMyVehicle->GetRight() = CVector(1.0f, 0.0f, 0.0f);
							m_pMyVehicle->GetForward() = CVector(0.0f, 1.0f, 0.0f);
							m_pMyVehicle->GetUp() = CVector(0.0f, 0.0f, 1.0f);
							if (pad->GetAccelerate()) {
								m_pMyVehicle->ApplyMoveForce(GetForward() * 30.0f);
							} else if (pad->GetBrake()) {
								m_pMyVehicle->ApplyMoveForce(-GetForward() * 30.0f);
							} else {
								int16 lr = pad->GetSteeringLeftRight();
								if (lr < 0) {
									//m_pMyVehicle->ApplyTurnForce(20.0f * -GetRight(), GetForward());
									m_pMyVehicle->ApplyMoveForce(-GetRight() * 30.0f);
								} else if (lr > 0) {
									m_pMyVehicle->ApplyMoveForce(GetRight() * 30.0f);
								} else {
									m_pMyVehicle->ApplyMoveForce(0.0f, 0.0f, 50.0f);
								}
							}
						}
					}
#endif
					float steerAngle = m_pMyVehicle->m_fSteerAngle;
					CAnimBlendAssociation *lDriveAssoc;
					CAnimBlendAssociation *rDriveAssoc;
					CAnimBlendAssociation *lbAssoc;
					CAnimBlendAssociation *sitAssoc;
					if (m_pMyVehicle->bLowVehicle) {
						sitAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_CAR_SIT_LO);

						if (!sitAssoc || sitAssoc->blendAmount < 1.0f) {
							break;
						}

						lDriveAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_CAR_DRIVE_LEFT_LO);
						lbAssoc = nil;
						rDriveAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_CAR_DRIVE_RIGHT_LO);
					} else {
						sitAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_CAR_SIT);

						if (!sitAssoc || sitAssoc->blendAmount < 1.0f) {
							break;
						}

						lDriveAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_CAR_DRIVE_LEFT);
						rDriveAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_CAR_DRIVE_RIGHT);
						lbAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_CAR_LOOKBEHIND);

						if (lbAssoc &&
							TheCamera.Cams[TheCamera.ActiveCam].Mode == CCam::MODE_1STPERSON
							&& TheCamera.Cams[TheCamera.ActiveCam].DirectionWasLooking == LOOKING_LEFT) {
							lbAssoc->blendDelta = -1000.0f;
						}
					}

					CAnimBlendAssociation *driveByAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_CAR_DRIVEBY_LEFT);

					if (!driveByAssoc)
						driveByAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_CAR_DRIVEBY_RIGHT);

					if (m_pMyVehicle->bLowVehicle || m_pMyVehicle->m_fGasPedal >= 0.0f || driveByAssoc) {
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
							else if (m_pMyVehicle->bLowVehicle)
								CAnimManager::AddAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_CAR_DRIVE_RIGHT_LO);
							else
								CAnimManager::AddAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_CAR_DRIVE_RIGHT);

						} else {
							if (rDriveAssoc)
								rDriveAssoc->blendAmount = 0.0f;

							if (lDriveAssoc)
								lDriveAssoc->blendAmount = clamp(steerAngle * 100.0f / 61.0f, 0.0f, 1.0f);
							else if (m_pMyVehicle->bLowVehicle)
								CAnimManager::AddAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_CAR_DRIVE_LEFT_LO);
							else
								CAnimManager::AddAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_CAR_DRIVE_LEFT);
						}

						if (lbAssoc)
							lbAssoc->blendDelta = -4.0f;
					} else {

						if ((TheCamera.Cams[TheCamera.ActiveCam].Mode != CCam::MODE_1STPERSON
							|| TheCamera.Cams[TheCamera.ActiveCam].DirectionWasLooking != LOOKING_LEFT)
							&& (!lbAssoc || lbAssoc->blendAmount < 1.0f)) {
							CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_CAR_LOOKBEHIND, 4.0f);
						}
					}
					break;
				}
				case PED_DIE:
					Die();
					break;
				case PED_HANDS_UP:
					if (m_pedStats->m_temper <= 50) {
						if (!RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_HANDSCOWER)) {
							CAnimManager::AddAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_HANDSCOWER);
							Say(SOUND_PED_HANDS_COWER);
						}
					} else if (!RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_HANDSUP)) {
						CAnimManager::AddAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_HANDSUP);
						Say(SOUND_PED_HANDS_UP);
					}
					break;
				default: break;
			}
			SetMoveAnim();
			if (bPedIsBleeding) {
				if (CGame::nastyGame) {
					if (!(CTimer::GetFrameCounter() & 3)) {
						CVector cameraDist = GetPosition() - TheCamera.GetPosition();
						if (cameraDist.MagnitudeSqr() < sq(50.0f)) {

							float length = (CGeneral::GetRandomNumber() & 127) * 0.0015f + 0.15f;
							CVector bloodPos(
								((CGeneral::GetRandomNumber() & 127) - 64) * 0.007f,
								((CGeneral::GetRandomNumber() & 127) - 64) * 0.007f,
								1.0f);
							bloodPos += GetPosition();

							CShadows::AddPermanentShadow(SHADOWTYPE_DARK, gpBloodPoolTex, &bloodPos, length, 0.0f,
								0.0f, -length, 255, 255, 0, 0, 4.0f, (CGeneral::GetRandomNumber() & 4095) + 2000, 1.0f);
						}
					}
				}
			}
			ServiceTalking();
			if (bInVehicle && !m_pMyVehicle)
				bInVehicle = false;
#ifndef VC_PED_PORTS
			m_pCurrentPhysSurface = nil;
#endif
		} else {
			if (bIsStanding && (!m_pCurrentPhysSurface || IsPlayer())
				|| bIsInWater || !bUsesCollision) {
				SetDead();
			}
			m_pCurrentPhysSurface = nil;
		}
	}
}

int32
CPed::ProcessEntityCollision(CEntity *collidingEnt, CColPoint *collidingPoints)
{
	bool collidedWithBoat = false;
	bool belowTorsoCollided = false;
	float gravityEffect = -0.15f * CTimer::GetTimeStep();
	CColPoint intersectionPoint;
	CColLine ourLine;

	CColModel *ourCol = CModelInfo::GetModelInfo(GetModelIndex())->GetColModel();
	CColModel *hisCol = CModelInfo::GetModelInfo(collidingEnt->GetModelIndex())->GetColModel();

	if (!bUsesCollision)
		return false;

	if (collidingEnt->IsVehicle() && ((CVehicle*)collidingEnt)->IsBoat())
		collidedWithBoat = true;

	// ofc we're not vehicle
	if (!m_bIsVehicleBeingShifted && !bSkipLineCol
#ifdef VC_PED_PORTS
		&& !collidingEnt->IsPed()
#endif
		) {
		if (!bCollisionProcessed) {
#ifdef VC_PED_PORTS
			m_pCurrentPhysSurface = nil;
#endif
			if (bIsStanding) {
				bIsStanding = false;
				bWasStanding = true;
			}
			bCollisionProcessed = true;
			m_fCollisionSpeed += m_vecMoveSpeed.Magnitude2D() * CTimer::GetTimeStep();
			bStillOnValidPoly = false;
			if (IsPlayer() || m_fCollisionSpeed >= 1.0f
				&& (m_fCollisionSpeed >= 2.0f || m_nPedState != PED_WANDER_PATH)) {
				m_collPoly.valid = false;
				m_fCollisionSpeed = 0.0f;
				bHitSteepSlope = false;
			} else {
				CVector pos = GetPosition();
				float potentialGroundZ = GetPosition().z - FEET_OFFSET;
				if (bWasStanding) {
					pos.z += -0.25f;
					potentialGroundZ += gravityEffect;
				}
				if (CCollision::IsStoredPolyStillValidVerticalLine(pos, potentialGroundZ, intersectionPoint, &m_collPoly)) {
					bStillOnValidPoly = true;
#ifdef VC_PED_PORTS
					if(!bSomeVCflag1 || FEET_OFFSET + intersectionPoint.point.z < GetPosition().z) {
						GetMatrix().GetPosition().z = FEET_OFFSET + intersectionPoint.point.z;
						if (bSomeVCflag1)
							bSomeVCflag1 = false;
					}
#else
					GetMatrix().GetPosition().z = FEET_OFFSET + intersectionPoint.point.z;
#endif

					m_vecMoveSpeed.z = 0.0f;
					bIsStanding = true;
				} else {
					m_collPoly.valid = false;
					m_fCollisionSpeed = 0.0f;
					bHitSteepSlope = false;
				}
			}
		}

		if (!bStillOnValidPoly) {
			CVector potentialCenter = GetPosition();
			potentialCenter.z = GetPosition().z - 0.52f;

			// 0.52f should be a ped's approx. radius
			float totalRadiusWhenCollided = collidingEnt->GetBoundRadius() + 0.52f - gravityEffect;
			if (bWasStanding) {
				if (collidedWithBoat) {
					potentialCenter.z += 2.0f * gravityEffect;
					totalRadiusWhenCollided += Abs(gravityEffect);
				} else {
					potentialCenter.z += gravityEffect;
				}
			}
			if (sq(totalRadiusWhenCollided) > (potentialCenter - collidingEnt->GetBoundCentre()).MagnitudeSqr()) {
				ourLine.p0 = GetPosition();
				ourLine.p1 = GetPosition();
				ourLine.p1.z = GetPosition().z - FEET_OFFSET;
				if (bWasStanding) {
					ourLine.p1.z = ourLine.p1.z + gravityEffect;
					ourLine.p0.z = ourLine.p0.z + -0.25f;
				}
				float minDist = 1.0f;
				belowTorsoCollided = CCollision::ProcessVerticalLine(ourLine, collidingEnt->GetMatrix(), *hisCol,
					intersectionPoint, minDist, false, &m_collPoly);

				if (collidedWithBoat && bWasStanding && !belowTorsoCollided) {
					ourLine.p0.z = ourLine.p1.z;
					ourLine.p1.z = ourLine.p1.z + gravityEffect;
					belowTorsoCollided = CCollision::ProcessVerticalLine(ourLine, collidingEnt->GetMatrix(), *hisCol,
						intersectionPoint, minDist, false, &m_collPoly);
				}
				if (belowTorsoCollided) {
#ifndef VC_PED_PORTS
					if (!collidingEnt->IsPed()) {
#endif
						if (!bIsStanding
							|| FEET_OFFSET + intersectionPoint.point.z > GetPosition().z
							|| collidedWithBoat && 3.12f + intersectionPoint.point.z > GetPosition().z) {

							if (!collidingEnt->IsVehicle() && !collidingEnt->IsObject()) {
								m_pCurSurface = collidingEnt;
								collidingEnt->RegisterReference((CEntity**)&m_pCurSurface);
								bTryingToReachDryLand = false;
								bOnBoat = false;
							} else {
								m_pCurrentPhysSurface = (CPhysical*)collidingEnt;
								collidingEnt->RegisterReference((CEntity**)&m_pCurrentPhysSurface);
								m_vecOffsetFromPhysSurface = intersectionPoint.point - collidingEnt->GetPosition();
								m_pCurSurface = collidingEnt;
								collidingEnt->RegisterReference((CEntity**)&m_pCurSurface);
								m_collPoly.valid = false;
								if (collidingEnt->IsVehicle() && ((CVehicle*)collidingEnt)->IsBoat()) {
									bOnBoat = true;
								} else {
									bOnBoat = false;
								}
							}
#ifdef VC_PED_PORTS
							if (!bSomeVCflag1 || FEET_OFFSET + intersectionPoint.point.z < GetPosition().z) {
								GetMatrix().GetPosition().z = FEET_OFFSET + intersectionPoint.point.z;
								if (bSomeVCflag1)
									bSomeVCflag1 = false;
							}
#else
						    GetMatrix().GetPosition().z = FEET_OFFSET + intersectionPoint.point.z;
#endif
							m_nSurfaceTouched = intersectionPoint.surfaceB;
							if (m_nSurfaceTouched == SURFACE_STEEP_CLIFF) {
								bHitSteepSlope = true;
								m_vecDamageNormal = intersectionPoint.normal;
							}
						}
#ifdef VC_PED_PORTS
						float upperSpeedLimit = 0.33f;
						float lowerSpeedLimit = -0.25f;
						float speed = m_vecMoveSpeed.Magnitude2D();
						if (m_nPedState == PED_IDLE) {
							upperSpeedLimit *= 2.0f;
							lowerSpeedLimit *= 1.5f;
						}
						CAnimBlendAssociation *fallAnim = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_FALL);
						if (!bWasStanding && speed > upperSpeedLimit && (/*!bPushedAlongByCar ||*/ m_vecMoveSpeed.z < lowerSpeedLimit)
							&& m_pCollidingEntity != collidingEnt) {

							float damage = 100.0f * Max(speed - 0.25f, 0.0f);
							float damage2 = damage;
							if (m_vecMoveSpeed.z < -0.25f)
								damage += (-0.25f - m_vecMoveSpeed.z) * 150.0f;

							uint8 dir = 2; // from backward
							if (m_vecMoveSpeed.x > 0.01f || m_vecMoveSpeed.x < -0.01f || m_vecMoveSpeed.y > 0.01f || m_vecMoveSpeed.y < -0.01f) {
								CVector2D offset = -m_vecMoveSpeed;
								dir = GetLocalDirection(offset);
							}

							InflictDamage(collidingEnt, WEAPONTYPE_FALL, damage, PEDPIECE_TORSO, dir);
							if (IsPlayer() && damage2 > 5.0f)
								Say(SOUND_PED_LAND);

						} else if (!bWasStanding && fallAnim && -0.016f * CTimer::GetTimeStep() > m_vecMoveSpeed.z) {
							InflictDamage(collidingEnt, WEAPONTYPE_FALL, 15.0f, PEDPIECE_TORSO, 2);
						}
#else
						float speedSqr = 0.0f;
						CAnimBlendAssociation *fallAnim = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_FALL);
						if (!bWasStanding && (m_vecMoveSpeed.z < -0.25f || (speedSqr = m_vecMoveSpeed.MagnitudeSqr()) > sq(0.5f))) {
							if (speedSqr == 0.0f)
								speedSqr = sq(m_vecMoveSpeed.z);

							uint8 dir = 2; // from backward
							if (m_vecMoveSpeed.x > 0.01f || m_vecMoveSpeed.x < -0.01f || m_vecMoveSpeed.y > 0.01f || m_vecMoveSpeed.y < -0.01f) {
								CVector2D offset = -m_vecMoveSpeed;
								dir = GetLocalDirection(offset);
							}
							InflictDamage(collidingEnt, WEAPONTYPE_FALL, 350.0f * sq(speedSqr), PEDPIECE_TORSO, dir);

						} else if (!bWasStanding && fallAnim && -0.016f * CTimer::GetTimeStep() > m_vecMoveSpeed.z) {
							InflictDamage(collidingEnt, WEAPONTYPE_FALL, 15.0f, PEDPIECE_TORSO, 2);
						}
#endif
						m_vecMoveSpeed.z = 0.0f;
						bIsStanding = true;
#ifndef VC_PED_PORTS
					} else {
						bOnBoat = false;
					}
#endif
				} else {
					bOnBoat = false;
				}
			}
		}
	}

	int ourCollidedSpheres = CCollision::ProcessColModels(GetMatrix(), *ourCol, collidingEnt->GetMatrix(), *hisCol, collidingPoints, nil, nil);
	if (ourCollidedSpheres > 0 || belowTorsoCollided) {
		AddCollisionRecord(collidingEnt);
		if (!collidingEnt->IsBuilding())
			((CPhysical*)collidingEnt)->AddCollisionRecord(this);

		if (ourCollidedSpheres > 0 && (collidingEnt->IsBuilding() || collidingEnt->GetIsStatic())) {
			bHasHitWall = true;
		}
	}
	if (collidingEnt->IsBuilding() || collidingEnt->GetIsStatic()) {

		if (bWasStanding) {
			CVector sphereNormal;
			float normalLength;
			for(int sphere = 0; sphere < ourCollidedSpheres; sphere++) {
				sphereNormal = collidingPoints[sphere].normal;
#ifdef VC_PED_PORTS
				if (sphereNormal.z >= -1.0f || !IsPlayer()) {
#endif
					normalLength = sphereNormal.Magnitude2D();
					if (normalLength != 0.0f) {
						sphereNormal.x = sphereNormal.x / normalLength;
						sphereNormal.y = sphereNormal.y / normalLength;
					}
#ifdef VC_PED_PORTS
				} else {
					float speed = m_vecMoveSpeed.Magnitude2D();
					sphereNormal.x = -m_vecMoveSpeed.x / Max(0.001f, speed);
					sphereNormal.y = -m_vecMoveSpeed.y / Max(0.001f, speed);
					GetMatrix().GetPosition().z -= 0.05f;
					bSomeVCflag1 = true;
				}
#endif
				sphereNormal.Normalise();
				collidingPoints[sphere].normal = sphereNormal;
				if (collidingPoints[sphere].surfaceB == SURFACE_STEEP_CLIFF)
					bHitSteepSlope = true;
			}
		}
	}
	return ourCollidedSpheres;
}

static void
particleProduceFootSplash(CPed *ped, CVector const &pos, float size, int times)
{
#ifdef PC_PARTICLE
	for (int i = 0; i < times; i++) {
		CVector adjustedPos = pos;
		adjustedPos.x += CGeneral::GetRandomNumberInRange(-0.1f, 0.1f);
		adjustedPos.y += CGeneral::GetRandomNumberInRange(-0.1f, 0.1f);

		CVector direction = ped->GetForward() * -0.05f;
		CParticle::AddParticle(PARTICLE_RAIN_SPLASHUP, adjustedPos, direction, nil, size, CRGBA(32, 32, 32, 32), 0, 0, CGeneral::GetRandomNumber() & 1, 200);
	}
#else
	for ( int32 i = 0; i < times; i++ )
	{
		CVector adjustedPos = pos;
		adjustedPos.x += CGeneral::GetRandomNumberInRange(-0.2f, 0.2f);
		adjustedPos.y += CGeneral::GetRandomNumberInRange(-0.2f, 0.2f);
	
		CParticle::AddParticle(PARTICLE_RAIN_SPLASHUP, adjustedPos, CVector(0.0f, 0.0f, 0.0f), nil, size, CRGBA(0, 0, 0, 0), 0, 0, CGeneral::GetRandomNumber() & 1, 200);
	}
#endif
}

static void
particleProduceFootDust(CPed *ped, CVector const &pos, float size, int times)
{
	switch (ped->m_nSurfaceTouched)
	{
		case SURFACE_TARMAC:
		case SURFACE_GRAVEL:
		case SURFACE_PAVEMENT:
		case SURFACE_SAND:
			for (int i = 0; i < times; ++i) {
				CVector adjustedPos = pos;
				adjustedPos.x += CGeneral::GetRandomNumberInRange(-0.1f, 0.1f);
				adjustedPos.y += CGeneral::GetRandomNumberInRange(-0.1f, 0.1f);
				CParticle::AddParticle(PARTICLE_PEDFOOT_DUST, adjustedPos, CVector(0.0f, 0.0f, 0.0f), nil, size, CRGBA(0, 0, 0, 0), 0, 0, 0, 0);
			}
			break;
		default:
			break;
	}
}

void
CPed::PlayFootSteps(void)
{
	if (bDoBloodyFootprints) {
		if (m_bloodyFootprintCountOrDeathTime > 0 && m_bloodyFootprintCountOrDeathTime < 300) {
			m_bloodyFootprintCountOrDeathTime--;

			if (m_bloodyFootprintCountOrDeathTime == 0)
				bDoBloodyFootprints = false;
		}
	}

	if (!bIsStanding)
		return;

	CAnimBlendAssociation *assoc = RpAnimBlendClumpGetFirstAssociation(GetClump());
	CAnimBlendAssociation *walkRunAssoc = nil;
	float walkRunAssocBlend = 0.0f, idleAssocBlend = 0.0f;

	for (; assoc; assoc = RpAnimBlendGetNextAssociation(assoc)) {
		if (assoc->flags & ASSOC_WALK) {
			walkRunAssoc = assoc;
			walkRunAssocBlend += assoc->blendAmount;
		} else if ((assoc->flags & ASSOC_NOWALK) == 0) {
			idleAssocBlend += assoc->blendAmount;
		}
	}

#ifdef GTA_PS2_STUFF
	CAnimBlendAssociation *runStopAsoc = NULL;
	
	if ( IsPlayer() )
	{	
		runStopAsoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_RUNSTOP2);
		
		if ( runStopAsoc == NULL )
			runStopAsoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_RUNSTOP2);
	}
	
	if ( runStopAsoc != NULL && runStopAsoc->blendAmount > 0.1f )
	{
		{
			CVector pos(0.0f, 0.0f, 0.0f);
			TransformToNode(pos, PED_FOOTL);
				
			pos.z -= 0.1f;
			pos += GetForward()*0.2f;
			particleProduceFootDust(this, pos, 0.02f, 1);
		}

		{
			CVector pos(0.0f, 0.0f, 0.0f);
			TransformToNode(pos, PED_FOOTR);
				
			pos.z -= 0.1f;
			pos += GetForward()*0.2f;
			particleProduceFootDust(this, pos, 0.02f, 1);
		}
	}
#endif
	

	if (walkRunAssoc && walkRunAssocBlend > 0.5f && idleAssocBlend < 1.0f) {
		float stepStart = 1 / 15.0f;
		float stepEnd = walkRunAssoc->hierarchy->totalLength / 2.0f + stepStart;
		float currentTime = walkRunAssoc->currentTime;
		int stepPart = 0;

		if (currentTime >= stepStart && currentTime - walkRunAssoc->timeStep < stepStart)
			stepPart = 1;
		else if (currentTime >= stepEnd && currentTime - walkRunAssoc->timeStep < stepEnd)
			stepPart = 2;

		if (stepPart != 0) {
			DMAudio.PlayOneShot(m_audioEntityId, stepPart == 1 ? SOUND_STEP_START : SOUND_STEP_END, 1.0f);
			CVector footPos(0.0f, 0.0f, 0.0f);
			TransformToNode(footPos, stepPart == 1 ? PED_FOOTL : PED_FOOTR);

			CVector forward = GetForward();

			footPos.z -= 0.1f;
			footPos += 0.2f * forward;

			if (bDoBloodyFootprints) {
				CVector2D top(forward * 0.26f);
				CVector2D right(GetRight() * 0.14f);

				CShadows::AddPermanentShadow(SHADOWTYPE_DARK, gpBloodPoolTex, &footPos,
					top.x, top.y,
					right.x, right.y,
					255, 255, 0, 0, 4.0f, 3000.0f, 1.0f);

				if (m_bloodyFootprintCountOrDeathTime <= 20) {
					m_bloodyFootprintCountOrDeathTime = 0;
					bDoBloodyFootprints = false;
				} else {
					m_bloodyFootprintCountOrDeathTime -= 20;
				}
			}
			if (CWeather::Rain <= 0.1f || CCullZones::CamNoRain() || CCullZones::PlayerNoRain()) {
				if(IsPlayer())
					particleProduceFootDust(this, footPos, 0.0f, 4);
			}
#ifdef PC_PARTICLE
			else if(stepPart == 2)
#else
			else
#endif
			{
				particleProduceFootSplash(this, footPos, 0.15f, 4);
			}
		}
	}

	if (m_nSurfaceTouched == SURFACE_WATER) {
		float pedSpeed = CVector2D(m_vecMoveSpeed).Magnitude();
		if (pedSpeed > 0.03f && CTimer::GetFrameCounter() % 2 == 0 && pedSpeed > 0.13f) {
#ifdef PC_PARTICLE
			float particleSize = pedSpeed * 2.0f;

			if (particleSize < 0.25f)
				particleSize = 0.25f;

			if (particleSize > 0.75f)
				particleSize = 0.75f;

			CVector particlePos = GetPosition() + GetForward() * 0.3f;
			particlePos.z -= 1.2f;

			CVector particleDir = m_vecMoveSpeed * -0.75f;

			particleDir.z = CGeneral::GetRandomNumberInRange(0.01f, 0.03f);
			CParticle::AddParticle(PARTICLE_PED_SPLASH, particlePos, particleDir, nil, 0.8f * particleSize, CRGBA(155,155,185,128), 0, 0, 0, 0);

			particleDir.z = CGeneral::GetRandomNumberInRange(0.03f, 0.05f);
			CParticle::AddParticle(PARTICLE_RUBBER_SMOKE, particlePos, particleDir, nil, particleSize, CRGBA(255,255,255,255), 0, 0, 0, 0);
#else
			CVector particlePos = (GetPosition() - 0.3f * GetUp()) + GetForward()*0.3f;
			CVector particleDir = m_vecMoveSpeed * 0.45f;
			particleDir.z = CGeneral::GetRandomNumberInRange(0.03f, 0.05f);
			CParticle::AddParticle(PARTICLE_PED_SPLASH, particlePos-CVector(0.0f, 0.0f, 1.2f), particleDir, nil, 0.0f, CRGBA(155, 185, 155, 255));
#endif
		}
	}
}

// Actually GetLocalDirectionTo(Turn/Look)
int
CPed::GetLocalDirection(const CVector2D &posOffset)
{
	int direction;
	float angle;

	for (angle = posOffset.Heading() - m_fRotationCur + DEGTORAD(45.0f); angle < 0.0f; angle += TWOPI);

	for (direction = RADTODEG(angle)/90.0f; direction > 3; direction -= 4);

	// 0-forward, 1-left, 2-backward, 3-right.
	return direction;
}

#ifdef NEW_WALK_AROUND_ALGORITHM
CVector
LocalPosForWalkAround(CVector2D colMin, CVector2D colMax, int walkAround, uint32 enterDoorNode, bool itsVan) {
	switch (walkAround) {
		case 0:
			if (enterDoorNode == CAR_DOOR_LF) 
				return CVector(colMin.x, colMax.y - 1.0f, 0.0f);
		case 1:
			return CVector(colMin.x, colMax.y, 0.0f);
		case 2:
		case 3:
			if (walkAround == 3 && enterDoorNode == CAR_DOOR_RF)
				return CVector(colMax.x, colMax.y - 1.0f, 0.0f);

			return CVector(colMax.x, colMax.y, 0.0f);
		case 4:
			if (enterDoorNode == CAR_DOOR_RR && !itsVan)
				return CVector(colMax.x, colMin.y + 1.0f, 0.0f);
		case 5:
			return CVector(colMax.x, colMin.y, 0.0f);
		case 6:
		case 7:
			if (walkAround == 7 && enterDoorNode == CAR_DOOR_LR && !itsVan)
				return CVector(colMin.x, colMin.y + 1.0f, 0.0f);

			return CVector(colMin.x, colMin.y, 0.0f);
		default:
			return CVector(0.0f, 0.0f, 0.0f);
	}
}

bool
CanWeSeeTheCorner(CVector2D dist, CVector2D fwdOffset)
{
	// because fov isn't important if dist is more then 5 unit, we want shortest way
	if (dist.Magnitude() > 5.0f)
		return true;

	if (DotProduct2D(dist, fwdOffset) < 0.0f)
		return false;

	return true;
}
#endif

// This function looks completely same on VC.
void
CPed::SetDirectionToWalkAroundObject(CEntity *obj)
{
	float distLimitForTimer = 8.0f;
	CColModel *objCol = CModelInfo::GetModelInfo(obj->GetModelIndex())->GetColModel();
	CVector objColMin = objCol->boundingBox.min;
	CVector objColMax = objCol->boundingBox.max;
	CVector objColCenter = (objColMin + objColMax) / 2.0f;
	CMatrix objMat(obj->GetMatrix());
	float dirToSet = obj->GetForward().Heading();
	bool goingToEnterCarAndItsVan = false;
	bool goingToEnterCar = false;
	bool objUpsideDown = false;

	float checkIntervalInDist = (objColMax.y - objColMin.y) * 0.1f;
	float checkIntervalInTime;

	if (m_nMoveState == PEDMOVE_NONE || m_nMoveState == PEDMOVE_STILL)
		return;

#ifndef PEDS_REPORT_CRIMES_ON_PHONE
	if (CharCreatedBy != MISSION_CHAR && obj->GetModelIndex() == MI_PHONEBOOTH1) {
		bool isRunning = m_nMoveState == PEDMOVE_RUN || m_nMoveState == PEDMOVE_SPRINT;
		SetFindPathAndFlee(obj, 5000, !isRunning);
		return;
	}
#endif

	CVector2D adjustedColMin(objColMin.x - 0.35f, objColMin.y - 0.35f);
	CVector2D adjustedColMax(objColMax.x + 0.35f, objColMax.y + 0.35f);

	checkIntervalInDist = Max(checkIntervalInDist, 0.5f);
	checkIntervalInDist = Min(checkIntervalInDist, (objColMax.z - objColMin.z) / 2.0f);
	checkIntervalInDist = Min(checkIntervalInDist, (adjustedColMax.x - adjustedColMin.x) / 2.0f);

	if (objMat.GetUp().z < 0.0f)
		objUpsideDown = true;

	if (obj->GetModelIndex() != MI_TRAFFICLIGHTS && obj->GetModelIndex() != MI_SINGLESTREETLIGHTS1 && obj->GetModelIndex() != MI_SINGLESTREETLIGHTS2) {
		objColCenter = obj->GetMatrix() * objColCenter;
	} else {
		checkIntervalInDist = 0.4f;
		if (objMat.GetUp().z <= 0.57f) {

			// Specific calculations for traffic lights, didn't get a bit.
			adjustedColMin.x = 1.2f * (adjustedColMin.x < adjustedColMin.y ? adjustedColMin.x : adjustedColMin.y);
			adjustedColMax.x = 1.2f * (adjustedColMax.x > adjustedColMax.y ? adjustedColMax.x : adjustedColMax.y);
			adjustedColMin.y = 1.2f * objColMin.z;
			adjustedColMax.y = 1.2f * objColMax.z;
			dirToSet = objMat.GetUp().Heading();
			objMat.SetUnity();
			objMat.RotateZ(dirToSet);
			objMat.GetPosition() += obj->GetPosition();
			objColCenter = obj->GetPosition();
		} else {
			objColCenter.x = adjustedColMax.x - 0.25f;
			objColCenter = obj->GetMatrix() * objColCenter;
			distLimitForTimer = 0.75f;
		}
		objUpsideDown = false;
	}
	float oldRotDest = m_fRotationDest;
#ifndef NEW_WALK_AROUND_ALGORITHM
	float angleToFaceObjCenter = (objColCenter - GetPosition()).Heading();
	float angleDiffBtwObjCenterAndForward = CGeneral::LimitRadianAngle(dirToSet - angleToFaceObjCenter);
	float objTopRightHeading = Atan2(adjustedColMax.x - adjustedColMin.x, adjustedColMax.y - adjustedColMin.y);
#endif

	if (IsPlayer()) {
		if (FindPlayerPed()->m_fMoveSpeed <= 0.0f)
			checkIntervalInTime = 0.0f;
		else
			checkIntervalInTime = 2.0f / FindPlayerPed()->m_fMoveSpeed;
	} else {
		switch (m_nMoveState) {
			case PEDMOVE_WALK:
				checkIntervalInTime = 2.0f;
				break;
			case PEDMOVE_RUN:
				checkIntervalInTime = 0.5f;
				break;
			case PEDMOVE_SPRINT:
				checkIntervalInTime = 0.5f;
				break;
			default:
				checkIntervalInTime = 0.0f;
				break;
		}
	}
	if (m_pSeekTarget == obj && obj->IsVehicle()) {
		if (m_objective == OBJECTIVE_ENTER_CAR_AS_DRIVER || m_objective == OBJECTIVE_ENTER_CAR_AS_PASSENGER
			|| m_objective == OBJECTIVE_SOLICIT_VEHICLE) {
			goingToEnterCar = true;
			if (IsPlayer())
				checkIntervalInTime = 0.0f;

			if (((CVehicle*)obj)->bIsVan)
				goingToEnterCarAndItsVan = true;
		}
	}

	int entityOnTopLeftOfObj = 0;
	int entityOnBottomLeftOfObj = 0;
	int entityOnTopRightOfObj = 0;
	int entityOnBottomRightOfObj = 0;

	if (CTimer::GetTimeInMilliseconds() > m_collidingThingTimer || m_collidingEntityWhileFleeing != obj) {
		bool collidingThingChanged = true;
		CEntity *obstacle;

#ifndef NEW_WALK_AROUND_ALGORITHM
		if (!obj->IsVehicle() || objUpsideDown) {
			collidingThingChanged = false;
		} else {		
#else
			CVector cornerToGo = CVector(10.0f, 10.0f, 10.0f);
			int dirToGo;
			m_walkAroundType = 0;
			int iWouldPreferGoingBack = 0; // 1:left 2:right
#endif
			float adjustedCheckInterval = 0.7f * checkIntervalInDist;
			CVector posToCheck;

			// Top left of obj
			posToCheck.x = adjustedColMin.x + adjustedCheckInterval;
			posToCheck.y = adjustedColMax.y - adjustedCheckInterval;
			posToCheck.z = 0.0f;
			posToCheck = objMat * posToCheck;
			posToCheck.z += 0.6f;
			obstacle = CWorld::TestSphereAgainstWorld(posToCheck, checkIntervalInDist, obj,
				true, true, false, true, false, false);
			if (obstacle) {
				if (obstacle->IsBuilding()) {
					entityOnTopLeftOfObj = 1;
				} else if (obstacle->IsVehicle()) {
					entityOnTopLeftOfObj = 2;
				} else {
					entityOnTopLeftOfObj = 3;
				}
			}
#ifdef NEW_WALK_AROUND_ALGORITHM
			else {
				CVector tl = obj->GetMatrix() * CVector(adjustedColMin.x, adjustedColMax.y, 0.0f) - GetPosition();
				if (goingToEnterCar && (m_vehDoor == CAR_DOOR_LF || m_vehDoor == CAR_DOOR_LR)) {
					cornerToGo = tl;
					m_walkAroundType = 1;

					if (m_vehDoor == CAR_DOOR_LR)
						iWouldPreferGoingBack = 1;
				} else if(CanWeSeeTheCorner(tl, GetForward())){
					cornerToGo = tl;
					dirToGo = GetLocalDirection(tl);
					if (dirToGo == 1)
						m_walkAroundType = 0; // ALL of the next turns will be right turn
					else if (dirToGo == 3)
						m_walkAroundType = 1; // ALL of the next turns will be left turn
				}
			}
#endif

			// Top right of obj
			posToCheck.x = adjustedColMax.x - adjustedCheckInterval;
			posToCheck.y = adjustedColMax.y - adjustedCheckInterval;
			posToCheck.z = 0.0f;
			posToCheck = objMat * posToCheck;
			posToCheck.z += 0.6f;
			obstacle = CWorld::TestSphereAgainstWorld(posToCheck, checkIntervalInDist, obj,
				true, true, false, true, false, false);
			if (obstacle) {
				if (obstacle->IsBuilding()) {
					entityOnTopRightOfObj = 1;
				} else if (obstacle->IsVehicle()) {
					entityOnTopRightOfObj = 2;
				} else {
					entityOnTopRightOfObj = 3;
				}
			}
#ifdef NEW_WALK_AROUND_ALGORITHM
			else {
				CVector tr = obj->GetMatrix() * CVector(adjustedColMax.x, adjustedColMax.y, 0.0f) - GetPosition();
				if (tr.Magnitude2D() < cornerToGo.Magnitude2D()) {
					if (goingToEnterCar && (m_vehDoor == CAR_DOOR_RF || m_vehDoor == CAR_DOOR_RR)) {
						cornerToGo = tr;
						m_walkAroundType = 2;

						if (m_vehDoor == CAR_DOOR_RR)
							iWouldPreferGoingBack = 2;
					} else if (CanWeSeeTheCorner(tr, GetForward())) {
						cornerToGo = tr;
						dirToGo = GetLocalDirection(tr);
						if (dirToGo == 1)
							m_walkAroundType = 2; // ALL of the next turns will be right turn
						else if (dirToGo == 3)
							m_walkAroundType = 3; // ALL of the next turns will be left turn
					}
				}
			}
#endif

			// Bottom right of obj
			posToCheck.x = adjustedColMax.x - adjustedCheckInterval;
			posToCheck.y = adjustedColMin.y + adjustedCheckInterval;
			posToCheck.z = 0.0f;
			posToCheck = objMat * posToCheck;
			posToCheck.z += 0.6f;
			obstacle = CWorld::TestSphereAgainstWorld(posToCheck, checkIntervalInDist, obj,
				true, true, false, true, false, false);
			if (obstacle) {
				if (obstacle->IsBuilding()) {
					entityOnBottomRightOfObj = 1;
				} else if (obstacle->IsVehicle()) {
					entityOnBottomRightOfObj = 2;
				} else {
					entityOnBottomRightOfObj = 3;
				}
			}
#ifdef NEW_WALK_AROUND_ALGORITHM
			else {
				CVector br = obj->GetMatrix() * CVector(adjustedColMax.x, adjustedColMin.y, 0.0f) - GetPosition();
				if (iWouldPreferGoingBack == 2)
					m_walkAroundType = 4;
				else if (br.Magnitude2D() < cornerToGo.Magnitude2D()) {
					if (goingToEnterCar && (m_vehDoor == CAR_DOOR_RF || m_vehDoor == CAR_DOOR_RR)) {
						cornerToGo = br;
						m_walkAroundType = 5;
					} else if (CanWeSeeTheCorner(br, GetForward())) {
						cornerToGo = br;
						dirToGo = GetLocalDirection(br);
						if (dirToGo == 1)
							m_walkAroundType = 4; // ALL of the next turns will be right turn
						else if (dirToGo == 3)
							m_walkAroundType = 5; // ALL of the next turns will be left turn
					}
				}
			}
#endif

			// Bottom left of obj
			posToCheck.x = adjustedColMin.x + adjustedCheckInterval;
			posToCheck.y = adjustedColMin.y + adjustedCheckInterval;
			posToCheck.z = 0.0f;
			posToCheck = objMat * posToCheck;
			posToCheck.z += 0.6f;
			obstacle = CWorld::TestSphereAgainstWorld(posToCheck, checkIntervalInDist, obj,
				true, true, false, true, false, false);
			if (obstacle) {
				if (obstacle->IsBuilding()) {
					entityOnBottomLeftOfObj = 1;
				} else if (obstacle->IsVehicle()) {
					entityOnBottomLeftOfObj = 2;
				} else {
					entityOnBottomLeftOfObj = 3;
				}
			}
#ifdef NEW_WALK_AROUND_ALGORITHM
			else {
				CVector bl = obj->GetMatrix() * CVector(adjustedColMin.x, adjustedColMin.y, 0.0f) - GetPosition();
				if (iWouldPreferGoingBack == 1)
					m_walkAroundType = 7;
				else if (bl.Magnitude2D() < cornerToGo.Magnitude2D()) {
					if (goingToEnterCar && (m_vehDoor == CAR_DOOR_LF || m_vehDoor == CAR_DOOR_LR)) {
						cornerToGo = bl;
						m_walkAroundType = 6;
					} else if (CanWeSeeTheCorner(bl, GetForward())) {
						cornerToGo = bl;
						dirToGo = GetLocalDirection(bl);
						if (dirToGo == 1)
							m_walkAroundType = 6; // ALL of the next turns will be right turn
						else if (dirToGo == 3)
							m_walkAroundType = 7; // ALL of the next turns will be left turn
					}
				}
			}
#else
		}

		if (entityOnTopLeftOfObj && entityOnTopRightOfObj && entityOnBottomRightOfObj && entityOnBottomLeftOfObj) {
			collidingThingChanged = false;
			entityOnTopLeftOfObj = 0;
			entityOnBottomLeftOfObj = 0;
			entityOnTopRightOfObj = 0;
			entityOnBottomRightOfObj = 0;
		}

		if (!collidingThingChanged) {
			m_walkAroundType = 0;
		} else {
			if (Abs(angleDiffBtwObjCenterAndForward) >= objTopRightHeading) {
				if (PI - objTopRightHeading >= Abs(angleDiffBtwObjCenterAndForward)) {
					if ((angleDiffBtwObjCenterAndForward <= 0.0f || objUpsideDown) && (angleDiffBtwObjCenterAndForward < 0.0f || !objUpsideDown)) {
						if (goingToEnterCar && (m_vehDoor == CAR_DOOR_RF || m_vehDoor == CAR_DOOR_RR)) {
							m_walkAroundType = 0;
						} else {
							if (CGeneral::LimitRadianAngle(m_fRotationDest - angleToFaceObjCenter) >= 0.0f) {
								if (entityOnBottomRightOfObj == 1 || entityOnBottomRightOfObj && !entityOnTopLeftOfObj && !entityOnTopRightOfObj) {
									m_walkAroundType = 1;
								} else if (entityOnBottomLeftOfObj == 1 || entityOnBottomLeftOfObj && !entityOnTopLeftOfObj && !entityOnTopRightOfObj) {
									m_walkAroundType = 1;
								}
							} else {
								if (entityOnTopRightOfObj == 1 || entityOnTopRightOfObj && !entityOnBottomRightOfObj && !entityOnBottomLeftOfObj) {
									m_walkAroundType = 4;
								} else if (entityOnTopLeftOfObj == 1 || entityOnTopLeftOfObj && !entityOnBottomRightOfObj && !entityOnBottomLeftOfObj) {
									m_walkAroundType = 4;
								}
							}
						}
					} else {
						if (goingToEnterCar && (m_vehDoor == CAR_DOOR_LF || m_vehDoor == CAR_DOOR_LR)) {
							m_walkAroundType = 0;
						} else {
							if (CGeneral::LimitRadianAngle(m_fRotationDest - angleToFaceObjCenter) <= 0.0f) {
								if (entityOnBottomLeftOfObj == 1 || entityOnBottomLeftOfObj && !entityOnTopLeftOfObj && !entityOnTopRightOfObj) {
									m_walkAroundType = 2;
								} else if (entityOnBottomRightOfObj == 1 || entityOnBottomRightOfObj && !entityOnTopLeftOfObj && !entityOnTopRightOfObj) {
									m_walkAroundType = 2;
								}
							} else {
								if (entityOnTopLeftOfObj == 1 || entityOnTopLeftOfObj && !entityOnBottomRightOfObj && !entityOnBottomLeftOfObj) {
									m_walkAroundType = 3;
								} else if (entityOnTopRightOfObj == 1 || entityOnTopRightOfObj && !entityOnBottomRightOfObj && !entityOnBottomLeftOfObj) {
									m_walkAroundType = 3;
								}
							}
						}
					}
				} else if (goingToEnterCar && (m_vehDoor == CAR_DOOR_LF || m_vehDoor == CAR_DOOR_LR)
					|| CGeneral::LimitRadianAngle(m_fRotationDest - angleToFaceObjCenter) < 0.0f) {
					if (entityOnTopLeftOfObj == 1 || entityOnTopLeftOfObj && !entityOnTopRightOfObj && !entityOnBottomRightOfObj) {
						m_walkAroundType = 3;
					}
				} else if (entityOnTopRightOfObj == 1 || entityOnTopRightOfObj && !entityOnTopLeftOfObj && !entityOnBottomLeftOfObj) {
					m_walkAroundType = 4;
				}
			} else if (goingToEnterCar && (m_vehDoor == CAR_DOOR_LF || m_vehDoor == CAR_DOOR_LR)
				|| CGeneral::LimitRadianAngle(m_fRotationDest - angleToFaceObjCenter) > 0.0f) {
				if (entityOnBottomLeftOfObj == 1 || entityOnBottomLeftOfObj && !entityOnTopRightOfObj && !entityOnBottomRightOfObj) {
					m_walkAroundType = 2;
				}
			} else if (entityOnBottomRightOfObj == 1 || entityOnBottomRightOfObj && !entityOnTopLeftOfObj && !entityOnBottomLeftOfObj) {
				m_walkAroundType = 1;
			} else {
				m_walkAroundType = 0;
			}
		}
#endif
	}
	m_collidingEntityWhileFleeing = obj;
	m_collidingEntityWhileFleeing->RegisterReference((CEntity**) &m_collidingEntityWhileFleeing);

	// TODO: This random may need to be changed.
	m_collidingThingTimer = CTimer::GetTimeInMilliseconds() + 512 + CGeneral::GetRandomNumber();

	CVector localPosToHead;

#ifdef NEW_WALK_AROUND_ALGORITHM
	int nextWalkAround = m_walkAroundType;
	if (m_walkAroundType % 2 == 0) {
		nextWalkAround += 2;
		if (nextWalkAround > 6)
			nextWalkAround = 0;
	} else {
		nextWalkAround -= 2;
		if (nextWalkAround < 0)
			nextWalkAround = 7;
	}

	CVector nextPosToHead = objMat * LocalPosForWalkAround(adjustedColMin, adjustedColMax, nextWalkAround, goingToEnterCar ? m_vehDoor : 0, goingToEnterCarAndItsVan);
	bool nextRouteIsClear = CWorld::GetIsLineOfSightClear(GetPosition(), nextPosToHead, true, true, true, true, true, true, false);

	if(nextRouteIsClear)
		m_walkAroundType = nextWalkAround;
	else {
		CVector posToHead = objMat * LocalPosForWalkAround(adjustedColMin, adjustedColMax, m_walkAroundType, goingToEnterCar ? m_vehDoor : 0, goingToEnterCarAndItsVan);
		bool currentRouteIsClear = CWorld::GetIsLineOfSightClear(GetPosition(), posToHead,
			true, true, true, true, true, true, false);

		/* Either;
		 *	- Some obstacle came in and it's impossible to reach current destination
		 *	- We reached to the destination, but since next route is not clear, we're turning around us
		 */
		if (!currentRouteIsClear ||
			((posToHead - GetPosition()).Magnitude2D() < 0.8f &&
				!CWorld::GetIsLineOfSightClear(GetPosition() + GetForward(), nextPosToHead,
				true, true, true, true, true, true, false))) {

			// Change both target and direction (involves changing even/oddness)
			if (m_walkAroundType % 2 == 0) {
				m_walkAroundType -= 2;
				if (m_walkAroundType < 0)
					m_walkAroundType = 7;
				else
					m_walkAroundType += 1;
			} else {
				m_walkAroundType += 2;
				if (m_walkAroundType > 7)
					m_walkAroundType = 0;
				else
					m_walkAroundType -= 1;
			}
		}
	}

	localPosToHead = LocalPosForWalkAround(adjustedColMin, adjustedColMax, m_walkAroundType, goingToEnterCar ? m_vehDoor : 0, goingToEnterCarAndItsVan);
#else
	if (Abs(angleDiffBtwObjCenterAndForward) < objTopRightHeading) {
		if (goingToEnterCar) {
			if (goingToEnterCarAndItsVan) {
				if (m_vehDoor == CAR_DOOR_LR || m_vehDoor == CAR_DOOR_RR)
					return;
			}
			if (m_vehDoor != CAR_DOOR_LF && m_vehDoor != CAR_DOOR_LR && (!entityOnBottomRightOfObj || entityOnBottomLeftOfObj)) {
				m_fRotationDest = CGeneral::LimitRadianAngle(dirToSet - HALFPI);
				localPosToHead.x = adjustedColMax.x;
				localPosToHead.z = 0.0f;
				localPosToHead.y = adjustedColMin.y;
			} else {
				m_fRotationDest = CGeneral::LimitRadianAngle(HALFPI + dirToSet);
				localPosToHead.x = adjustedColMin.x;
				localPosToHead.z = 0.0f;
				localPosToHead.y = adjustedColMin.y;
			}
		} else {
			if (m_walkAroundType != 1 && m_walkAroundType != 4
				&& (m_walkAroundType || CGeneral::LimitRadianAngle(m_fRotationDest - angleToFaceObjCenter) <= 0.0f)) {

				m_fRotationDest = CGeneral::LimitRadianAngle(dirToSet - HALFPI);
				localPosToHead.x = adjustedColMax.x;
				localPosToHead.z = 0.0f;
				localPosToHead.y = adjustedColMin.y;
			} else {
				m_fRotationDest = CGeneral::LimitRadianAngle(HALFPI + dirToSet);
				localPosToHead.x = adjustedColMin.x;
				localPosToHead.z = 0.0f;
				localPosToHead.y = adjustedColMin.y;
			}
		}
	} else {
		if (PI - objTopRightHeading >= Abs(angleDiffBtwObjCenterAndForward)) {
			if (angleDiffBtwObjCenterAndForward <= 0.0f) {
				if (!goingToEnterCar || !goingToEnterCarAndItsVan || m_vehDoor != CAR_DOOR_LR && m_vehDoor != CAR_DOOR_RR) {
					if (goingToEnterCar) {
						if (m_vehDoor == CAR_DOOR_RF || (m_vehDoor == CAR_DOOR_RR && !goingToEnterCarAndItsVan))
							return;
					}
					if (m_walkAroundType == 4 || m_walkAroundType == 3
						|| !m_walkAroundType && CGeneral::LimitRadianAngle(m_fRotationDest - angleToFaceObjCenter) > 0.0f) {

						m_fRotationDest = CGeneral::LimitRadianAngle(PI + dirToSet);
						localPosToHead.x = adjustedColMax.x;
						localPosToHead.z = 0.0f;
						localPosToHead.y = adjustedColMin.y;
					} else {
						m_fRotationDest = dirToSet;
						localPosToHead.x = adjustedColMax.x;
						localPosToHead.z = 0.0f;
						localPosToHead.y = adjustedColMax.y;
					}
				} else {
					m_fRotationDest = CGeneral::LimitRadianAngle(PI + dirToSet);
					localPosToHead.x = adjustedColMax.x;
					localPosToHead.z = 0.0f;
					localPosToHead.y = adjustedColMin.y;
				}
			} else if (goingToEnterCar && goingToEnterCarAndItsVan && (m_vehDoor == CAR_DOOR_LR || m_vehDoor == CAR_DOOR_RR)) {
				m_fRotationDest = CGeneral::LimitRadianAngle(PI + dirToSet);
				localPosToHead.x = adjustedColMin.x;
				localPosToHead.z = 0.0f;
				localPosToHead.y = adjustedColMin.y;
			} else {
				if (goingToEnterCar) {
					if (m_vehDoor == CAR_DOOR_LF || m_vehDoor == CAR_DOOR_LR && !goingToEnterCarAndItsVan)
						return;
				}
				if (m_walkAroundType == 1 || m_walkAroundType == 2
					|| !m_walkAroundType && CGeneral::LimitRadianAngle(m_fRotationDest - angleToFaceObjCenter) > 0.0f) {

					m_fRotationDest = dirToSet;
					localPosToHead.x = adjustedColMin.x;
					localPosToHead.z = 0.0f;
					localPosToHead.y = adjustedColMax.y;
				} else {
					m_fRotationDest = CGeneral::LimitRadianAngle(PI + dirToSet);
					localPosToHead.x = adjustedColMin.x;
					localPosToHead.z = 0.0f;
					localPosToHead.y = adjustedColMin.y;
				}
			}
		} else {
			if (goingToEnterCar && (!goingToEnterCarAndItsVan || m_vehDoor != CAR_DOOR_LR && m_vehDoor != CAR_DOOR_RR)) {
				if (m_vehDoor != CAR_DOOR_LF && m_vehDoor != CAR_DOOR_LR && (!entityOnTopRightOfObj || entityOnTopLeftOfObj)) {

					m_fRotationDest = CGeneral::LimitRadianAngle(dirToSet - HALFPI);
					localPosToHead.x = adjustedColMax.x;
					localPosToHead.z = 0.0f;
					localPosToHead.y = adjustedColMax.y;
				} else {
					m_fRotationDest = CGeneral::LimitRadianAngle(HALFPI + dirToSet);
					localPosToHead.x = adjustedColMin.x;
					localPosToHead.z = 0.0f;
					localPosToHead.y = adjustedColMax.y;
				}
			} else {
				if (m_walkAroundType == 2 || m_walkAroundType == 3
					|| !m_walkAroundType && CGeneral::LimitRadianAngle(m_fRotationDest - angleToFaceObjCenter) > 0.0f) {

					m_fRotationDest = CGeneral::LimitRadianAngle(dirToSet - HALFPI);
					localPosToHead.x = adjustedColMax.x;
					localPosToHead.z = 0.0f;
					localPosToHead.y = adjustedColMax.y;
				} else {
					m_fRotationDest = CGeneral::LimitRadianAngle(HALFPI + dirToSet);
					localPosToHead.x = adjustedColMin.x;
					localPosToHead.z = 0.0f;
					localPosToHead.y = adjustedColMax.y;
				}
			}
		}
	}
#endif
	if (objUpsideDown)
		localPosToHead.x = localPosToHead.x * -1.0f;

	localPosToHead = objMat * localPosToHead;
	m_actionX = localPosToHead.x;
	m_actionY = localPosToHead.y;
	localPosToHead -= GetPosition();
	m_fRotationDest = CGeneral::LimitRadianAngle(localPosToHead.Heading());

	if (m_fRotationDest != m_fRotationCur && bHitSomethingLastFrame) {
		if (m_fRotationDest == oldRotDest) {
			m_fRotationDest = oldRotDest;
		} else {
			m_fRotationDest = CGeneral::LimitRadianAngle(PI + dirToSet);
		}
	}

	float dist = localPosToHead.Magnitude2D();
	if (dist < 0.5f)
		dist = 0.5f;

	if (dist > distLimitForTimer)
		dist = distLimitForTimer;
	m_nPedStateTimer = CTimer::GetTimeInMilliseconds() + 280.0f * dist * checkIntervalInTime;
}

bool
CPed::IsPedInControl(void)
{
	return m_nPedState <= PED_STATES_NO_AI
		&& !bIsInTheAir && !bIsLanding
		&& m_fHealth > 0.0f;
}

bool
CPed::IsPedShootable(void)
{
	return m_nPedState <= PED_STATES_NO_ST;
}

bool
CPed::UseGroundColModel(void)
{
	return m_nPedState == PED_FALL ||
		m_nPedState == PED_DIVE_AWAY ||
		m_nPedState == PED_DIE ||
		m_nPedState == PED_DEAD;
}

bool
CPed::CanPedReturnToState(void)
{
	return m_nPedState <= PED_STATES_NO_AI && m_nPedState != PED_AIM_GUN && m_nPedState != PED_ATTACK &&
		m_nPedState != PED_FIGHT && m_nPedState != PED_STEP_AWAY && m_nPedState != PED_SNIPER_MODE && m_nPedState != PED_LOOK_ENTITY;
}

bool
CPed::CanSetPedState(void)
{
	return !DyingOrDead() && m_nPedState != PED_ARRESTED && !EnteringCar() && m_nPedState != PED_STEAL_CAR;
}

bool
CPed::CanStrafeOrMouseControl(void)
{
#ifdef FREE_CAM
	if (CCamera::bFreeCam)
		return false;
#endif
	return m_nPedState == PED_NONE || m_nPedState == PED_IDLE || m_nPedState == PED_FLEE_POS || m_nPedState == PED_FLEE_ENTITY ||
		m_nPedState == PED_ATTACK || m_nPedState == PED_FIGHT || m_nPedState == PED_AIM_GUN || m_nPedState == PED_JUMP;
}

void
CPed::PedGetupCB(CAnimBlendAssociation* animAssoc, void* arg)
{
	CPed* ped = (CPed*)arg;

	if (ped->m_nPedState == PED_GETUP)
		RpAnimBlendClumpSetBlendDeltas(ped->GetClump(), ASSOC_PARTIAL, -1000.0f);

	ped->bFallenDown = false;
	animAssoc->blendDelta = -1000.0f;
	if (ped->m_nPedState == PED_GETUP)
		ped->RestorePreviousState();

	if (ped->m_nPedState != PED_FLEE_POS && ped->m_nPedState != PED_FLEE_ENTITY)
		ped->SetMoveState(PEDMOVE_STILL);
	else
		ped->SetMoveState(PEDMOVE_RUN);

	ped->SetMoveAnim();
	ped->bGetUpAnimStarted = false;
}

void
CPed::PedLandCB(CAnimBlendAssociation* animAssoc, void* arg)
{
	CPed* ped = (CPed*)arg;

	animAssoc->blendDelta = -1000.0f;
	ped->bIsLanding = false;

	if (ped->m_nPedState == PED_JUMP)
		ped->RestorePreviousState();
}

void
CPed::PedStaggerCB(CAnimBlendAssociation* animAssoc, void* arg)
{
	/*
	CPed *ped = (CPed*)arg;

	if (ped->m_nPedState == PED_STAGGER)
		// nothing
	*/
}

void
CPed::PedSetOutCarCB(CAnimBlendAssociation *animAssoc, void *arg)
{
	CPed *ped = (CPed*)arg;

	CVehicle *veh = ped->m_pMyVehicle;

	bool startedToRun = false;
	ped->bUsesCollision = true;
	ped->m_actionX = 0.0f;
	ped->m_actionY = 0.0f;
	ped->bVehExitWillBeInstant = false;
	if (veh && veh->IsBoat())
		ped->ApplyMoveSpeed();

	if (ped->m_objective == OBJECTIVE_LEAVE_CAR)
		ped->RestorePreviousObjective();
#ifdef VC_PED_PORTS
	else if (ped->m_objective == OBJECTIVE_LEAVE_CAR_AND_DIE) {
		ped->m_fHealth = 0.0f;
		ped->SetDie(ANIM_STD_HIT_FLOOR, 4.0f, 0.5f);
	}
#endif

	ped->bInVehicle = false;
	if (veh && veh->IsCar() && !veh->IsRoomForPedToLeaveCar(ped->m_vehDoor, nil)) {
		ped->PositionPedOutOfCollision();
	}

	if (ped->m_nPedState == PED_EXIT_CAR) {
		if (ped->m_nPedType == PEDTYPE_COP)
			ped->SetIdle();
		else
			ped->RestorePreviousState();

		veh = ped->m_pMyVehicle;
		if (ped->bFleeAfterExitingCar && veh) {
			ped->bFleeAfterExitingCar = false;
			ped->SetFlee(veh->GetPosition(), 12000);
			ped->bUsePedNodeSeek = true;
			ped->m_pNextPathNode = nil;
			if (CGeneral::GetRandomNumber() & 1 || ped->m_pedStats->m_fear > 70) {
				ped->SetMoveState(PEDMOVE_SPRINT);
				ped->Say(SOUND_PED_FLEE_SPRINT);
			} else {
				ped->SetMoveState(PEDMOVE_RUN);
				ped->Say(SOUND_PED_FLEE_RUN);
			}
			startedToRun = true;

			// This is not a good way to do this...
			ped->m_nLastPedState = PED_WANDER_PATH;

		} else if (ped->bWanderPathAfterExitingCar) {
			ped->SetWanderPath(CGeneral::GetRandomNumberInRange(0.0f, 8.0f));
			ped->bWanderPathAfterExitingCar = false;
			if (ped->m_nPedType == PEDTYPE_PROSTITUTE)
				ped->SetObjectiveTimer(30000);
			ped->m_nLastPedState = PED_NONE;

		} else if (ped->bGonnaKillTheCarJacker) {

			// Kill objective is already given at this point.
			ped->bGonnaKillTheCarJacker = false;
			if (ped->m_pedInObjective) {
				if (!(CGeneral::GetRandomNumber() & 1)
					&& ped->m_nPedType != PEDTYPE_COP
					&& (!ped->m_pedInObjective->IsPlayer() || !CTheScripts::IsPlayerOnAMission())) {
					ped->ClearObjective();
					ped->SetObjective(OBJECTIVE_ENTER_CAR_AS_DRIVER, veh);
				}
				ped->m_leaveCarTimer = CTimer::GetTimeInMilliseconds() + 1500;
			}
			int waitTime = 1500;
			ped->SetWaitState(WAITSTATE_PLAYANIM_COWER, &waitTime);
			ped->SetMoveState(PEDMOVE_RUN);
			startedToRun = true;
		} else if (ped->m_objective == OBJECTIVE_NONE && ped->CharCreatedBy != MISSION_CHAR && ped->m_nPedState == PED_IDLE && !ped->IsPlayer()) {
			ped->SetWanderPath(CGeneral::GetRandomNumberInRange(0.0f, 8.0f));
		}
	}
#ifdef VC_PED_PORTS
	else {
		ped->m_nPedState = PED_IDLE;
	}
#endif

	if (animAssoc)
		animAssoc->blendDelta = -1000.0f;

	ped->RestartNonPartialAnims();
	ped->m_pVehicleAnim = nil;
	CVector posFromZ = ped->GetPosition();
	CPedPlacement::FindZCoorForPed(&posFromZ);
	ped->m_vecMoveSpeed = CVector(0.0f, 0.0f, 0.0f);
	ped->SetPosition(posFromZ);
	veh = ped->m_pMyVehicle;
	if (veh) {
		if (ped->m_nPedType == PEDTYPE_PROSTITUTE) {
			if (veh->pDriver) {
				if (veh->pDriver->IsPlayer() && ped->CharCreatedBy == RANDOM_CHAR) {
					CWorld::Players[CWorld::PlayerInFocus].m_nNextSexMoneyUpdateTime = 0;
					CWorld::Players[CWorld::PlayerInFocus].m_nNextSexFrequencyUpdateTime = 0;
					CWorld::Players[CWorld::PlayerInFocus].m_pHooker = nil;
					CWorld::Players[CWorld::PlayerInFocus].m_nMoney -= 100;
					if (CWorld::Players[CWorld::PlayerInFocus].m_nMoney < 0)
						CWorld::Players[CWorld::PlayerInFocus].m_nMoney = 0;
				}
			}
		}
		veh->m_nGettingOutFlags &= ~GetCarDoorFlag(ped->m_vehDoor);
		if (veh->pDriver == ped) {
			veh->RemoveDriver();
#ifndef FIX_BUGS // RemoveDriver does it anyway
			veh->SetStatus(STATUS_ABANDONED);
#endif
			if (veh->m_nDoorLock == CARLOCK_LOCKED_INITIALLY)
				veh->m_nDoorLock = CARLOCK_UNLOCKED;
			if (ped->m_nPedType == PEDTYPE_COP && veh->IsLawEnforcementVehicle())
				veh->ChangeLawEnforcerState(false);
		} else {
			veh->RemovePassenger(ped);
		}

		if (veh->bIsBus && !veh->IsUpsideDown() && !veh->IsOnItsSide()) {
			float angleAfterExit;
			if (ped->m_vehDoor == CAR_DOOR_LF) {
				angleAfterExit = HALFPI + veh->GetForward().Heading();
			} else {
				angleAfterExit = veh->GetForward().Heading() - HALFPI;
			}
			ped->SetHeading(angleAfterExit);
			ped->m_fRotationDest = angleAfterExit;
			ped->m_fRotationCur = angleAfterExit;
			if (!ped->bBusJacked)
				ped->SetMoveState(PEDMOVE_WALK);
		}
		if (CGarages::IsPointWithinAnyGarage(ped->GetPosition()))
			veh->bLightsOn = false;
	}

	if (ped->IsPlayer())
		AudioManager.PlayerJustLeftCar();

	ped->ReplaceWeaponWhenExitingVehicle();

	ped->bOnBoat = false;
	if (ped->bBusJacked) {
		ped->SetFall(1500, ANIM_STD_HIGHIMPACT_BACK, false);
		ped->bBusJacked = false;
	}
	ped->m_nStoredMoveState = PEDMOVE_NONE;
	if (!ped->IsPlayer()) {
		// It's a shame...
#ifdef FIX_BUGS
		int createdBy = ped->CharCreatedBy;
#else
		int createdBy = !ped->CharCreatedBy;
#endif

		if (createdBy == MISSION_CHAR && !startedToRun)
			ped->SetMoveState(PEDMOVE_WALK);
	}
}

void
CPed::PedSetDraggedOutCarCB(CAnimBlendAssociation *dragAssoc, void *arg)
{
	CAnimBlendAssociation *quickJackedAssoc;
	CVehicle *vehicle; 
	CPed *ped = (CPed*)arg;

	quickJackedAssoc = RpAnimBlendClumpGetAssociation(ped->GetClump(), ANIM_STD_QUICKJACKED);
	if (ped->m_nPedState != PED_ARRESTED) {
		ped->m_nLastPedState = PED_NONE;
		if (dragAssoc)
			dragAssoc->blendDelta = -1000.0f;
	}
	ped->RestartNonPartialAnims();
	ped->m_pVehicleAnim = nil;
	ped->m_pSeekTarget = nil;
	vehicle = ped->m_pMyVehicle;

	if (vehicle) {
		vehicle->m_nGettingOutFlags &= ~GetCarDoorFlag(ped->m_vehDoor);

		if (vehicle->pDriver == ped) {
			vehicle->RemoveDriver();
			if (vehicle->m_nDoorLock == CARLOCK_LOCKED_INITIALLY)
				vehicle->m_nDoorLock = CARLOCK_UNLOCKED;

			if (ped->m_nPedType == PEDTYPE_COP && vehicle->IsLawEnforcementVehicle())
				vehicle->ChangeLawEnforcerState(false);
		} else {
			vehicle->RemovePassenger(ped);
		}
	}
	ped->bInVehicle = false;
	if (ped->IsPlayer())
		AudioManager.PlayerJustLeftCar();

#ifdef VC_PED_PORTS
	if (ped->m_objective == OBJECTIVE_LEAVE_CAR_AND_DIE) {
		dragAssoc->SetDeleteCallback(PedSetDraggedOutCarPositionCB, ped);
		ped->m_fHealth = 0.0f;
		ped->SetDie(ANIM_STD_HIT_FLOOR, 1000.0f, 0.5f);
		return;
	}
#endif

	if (quickJackedAssoc) {
		dragAssoc->SetDeleteCallback(PedSetQuickDraggedOutCarPositionCB, ped);
	} else {
		dragAssoc->SetDeleteCallback(PedSetDraggedOutCarPositionCB, ped);
		if (ped->CanSetPedState())
			CAnimManager::BlendAnimation(ped->GetClump(), ASSOCGRP_STD, ANIM_STD_GET_UP, 1000.0f);
	}

	ped->ReplaceWeaponWhenExitingVehicle();

	ped->m_nStoredMoveState = PEDMOVE_NONE;
	ped->bVehExitWillBeInstant = false;
}

void
CPed::PedSetInCarCB(CAnimBlendAssociation *animAssoc, void *arg)
{
	CPed *ped = (CPed*)arg;

	CVehicle *veh = ped->m_pMyVehicle;

	// Pointless code
	if (!veh)
		return;

#ifdef VC_PED_PORTS
	// Situation of entering car as a driver while there is already a driver exiting atm.
	CPed *driver = veh->pDriver;
	if (driver && driver->m_nPedState == PED_DRIVING && !veh->bIsBus && driver->m_objective == OBJECTIVE_LEAVE_CAR
		&& (ped->m_objective == OBJECTIVE_ENTER_CAR_AS_DRIVER || ped->m_nPedState == PED_CARJACK)) {

		if (!ped->IsPlayer() && (ped->CharCreatedBy != MISSION_CHAR || driver->IsPlayer())) {
			ped->QuitEnteringCar();
			return;
		}
		if (driver->CharCreatedBy == MISSION_CHAR) {
			PedSetOutCarCB(nil, veh->pDriver);
			if (driver->m_pMyVehicle) {
				driver->PositionPedOutOfCollision();
			} else {
				driver->m_pMyVehicle = veh;
				driver->PositionPedOutOfCollision();
				driver->m_pMyVehicle = nil;
			}
			veh->pDriver = nil;
		} else {
			driver->SetDead();
			driver->FlagToDestroyWhenNextProcessed();
			veh->pDriver = nil;
		}
	}
#endif

	if (!ped->IsNotInWreckedVehicle() || ped->DyingOrDead())
		return;

	ped->bInVehicle = true;
	if (ped->m_nPedType == PEDTYPE_PROSTITUTE) {
		if (veh->pDriver) {
			if (veh->pDriver->IsPlayer() && ped->CharCreatedBy == RANDOM_CHAR) {
				CWorld::Players[CWorld::PlayerInFocus].m_nSexFrequency = 1000;
				CWorld::Players[CWorld::PlayerInFocus].m_nNextSexMoneyUpdateTime = CTimer::GetTimeInMilliseconds() + 1000;
				CWorld::Players[CWorld::PlayerInFocus].m_nNextSexFrequencyUpdateTime = CTimer::GetTimeInMilliseconds() + 3000;
				CWorld::Players[CWorld::PlayerInFocus].m_pHooker = (CCivilianPed*)ped;
			}
		}
	}
	if (ped->m_objective == OBJECTIVE_ENTER_CAR_AS_DRIVER
#if defined VC_PED_PORTS || defined FIX_BUGS
		|| ped->m_nPedState == PED_CARJACK
#endif
		)
		veh->bIsBeingCarJacked = false;

	if (veh->m_nNumGettingIn)
		--veh->m_nNumGettingIn;

	if (ped->IsPlayer() && ((CPlayerPed*)ped)->m_bAdrenalineActive)
		((CPlayerPed*)ped)->ClearAdrenaline();

	if (veh->IsBoat()) {
		if (ped->IsPlayer()) {
#if defined VC_PED_PORTS || defined FIX_BUGS
			CCarCtrl::RegisterVehicleOfInterest(veh);
#endif
			if (veh->GetStatus() == STATUS_SIMPLE) {
				veh->m_vecMoveSpeed = CVector(0.0f, 0.0f, -0.00001f);
				veh->m_vecTurnSpeed = CVector(0.0f, 0.0f, 0.0f);
			}
			veh->SetStatus(STATUS_PLAYER);
			AudioManager.PlayerJustGotInCar();
		}
		veh->SetDriver(ped);
		if (!veh->bEngineOn)
			veh->bEngineOn = true;

		ped->SetPedState(PED_DRIVING);
		ped->StopNonPartialAnims();
		return;
	}

	if (ped->m_pVehicleAnim)
		ped->m_pVehicleAnim->blendDelta = -1000.0f;

	ped->bDoBloodyFootprints = false;
	if (veh->m_nAlarmState == -1)
		veh->m_nAlarmState = 15000;

	if (ped->IsPlayer()) {
		if (ped->m_objective == OBJECTIVE_ENTER_CAR_AS_DRIVER) {
			if (veh->GetStatus() == STATUS_SIMPLE) {
				veh->m_vecMoveSpeed = CVector(0.0f, 0.0f, 0.0f);
				veh->m_vecTurnSpeed = CVector(0.0f, 0.0f, 0.0f);
			}
			veh->SetStatus(STATUS_PLAYER);
		}
		AudioManager.PlayerJustGotInCar();
	} else if (ped->m_objective == OBJECTIVE_ENTER_CAR_AS_DRIVER) {
		if (veh->GetStatus() == STATUS_SIMPLE) {
			veh->m_vecMoveSpeed = CVector(0.0f, 0.0f, 0.0f);
			veh->m_vecTurnSpeed = CVector(0.0f, 0.0f, 0.0f);
		}
		veh->SetStatus(STATUS_PHYSICS);
	}
	
	if (ped->m_objective == OBJECTIVE_ENTER_CAR_AS_DRIVER) {
		for (int i = 0; i < veh->m_nNumMaxPassengers; ++i) {
			CPed *passenger = veh->pPassengers[i];
			if (passenger && passenger->CharCreatedBy == RANDOM_CHAR) {
				passenger->SetObjective(OBJECTIVE_LEAVE_CAR, veh);
#ifdef VC_PED_PORTS
				passenger->m_leaveCarTimer = CTimer::GetTimeInMilliseconds();
#endif
			}
		}
	}
	// This shouldn't happen at all. Passengers can't enter with PED_CARJACK. Even though they did, we shouldn't call AddPassenger in here and SetDriver in below.
#if !defined VC_PED_PORTS && !defined FIX_BUGS
	else if (ped->m_objective == OBJECTIVE_ENTER_CAR_AS_PASSENGER) {
		if (ped->m_nPedState == PED_CARJACK) {
			veh->AddPassenger(ped, 0);
			ped->SetPedState(PED_DRIVING);
			ped->RestorePreviousObjective();
			ped->SetObjective(OBJECTIVE_LEAVE_CAR, veh);
		} else if (veh->pDriver && ped->CharCreatedBy == RANDOM_CHAR) {
			veh->AutoPilot.m_nCruiseSpeed = 17;
		}
	}
#endif

	if (ped->m_objective == OBJECTIVE_ENTER_CAR_AS_DRIVER || ped->m_nPedState == PED_CARJACK) {
		veh->SetDriver(ped);
		if (veh->VehicleCreatedBy == PARKED_VEHICLE) {
			veh->VehicleCreatedBy = RANDOM_VEHICLE;
			++CCarCtrl::NumRandomCars;
			--CCarCtrl::NumParkedCars;
		}
		if (veh->bIsAmbulanceOnDuty) {
			veh->bIsAmbulanceOnDuty = false;
			--CCarCtrl::NumAmbulancesOnDuty;
		}
		if (veh->bIsFireTruckOnDuty) {
			veh->bIsFireTruckOnDuty = false;
			--CCarCtrl::NumFiretrucksOnDuty;
		}
		if (ped->m_nPedType == PEDTYPE_COP && veh->IsLawEnforcementVehicle())
			veh->ChangeLawEnforcerState(true);

		if (!veh->bEngineOn) {
			veh->bEngineOn = true;
			DMAudio.PlayOneShot(ped->m_audioEntityId, SOUND_CAR_ENGINE_START, 1.0f);
		}
		if (ped->m_objective == OBJECTIVE_ENTER_CAR_AS_DRIVER && ped->CharCreatedBy == RANDOM_CHAR
			&& ped != FindPlayerPed() && ped->m_nPedType != PEDTYPE_EMERGENCY) {

			CCarCtrl::JoinCarWithRoadSystem(veh);
			veh->AutoPilot.m_nCarMission = MISSION_CRUISE;
			veh->AutoPilot.m_nTempAction = TEMPACT_NONE;
			veh->AutoPilot.m_nDrivingStyle = DRIVINGSTYLE_AVOID_CARS;
			veh->AutoPilot.m_nCruiseSpeed = 25;
		}
		ped->SetPedState(PED_DRIVING);
		if (ped->m_objective == OBJECTIVE_ENTER_CAR_AS_DRIVER) {

			if (ped->m_prevObjective == OBJECTIVE_RUN_TO_AREA || ped->m_prevObjective == OBJECTIVE_GOTO_CHAR_ON_FOOT || ped->m_prevObjective == OBJECTIVE_KILL_CHAR_ON_FOOT)
				ped->m_prevObjective = OBJECTIVE_NONE;

			ped->RestorePreviousObjective();
		}

	} else if (ped->m_objective == OBJECTIVE_ENTER_CAR_AS_PASSENGER) {
		if (veh->bIsBus) {
			veh->AddPassenger(ped);
		} else {
			switch (ped->m_vehDoor) {
				case CAR_DOOR_RF:
					veh->AddPassenger(ped, 0);
					break;
				case CAR_DOOR_RR:
					veh->AddPassenger(ped, 2);
					break;
				case CAR_DOOR_LR:
					veh->AddPassenger(ped, 1);
					break;
				default:
					veh->AddPassenger(ped);
				break;
			}
		}
		ped->SetPedState(PED_DRIVING);
		if (ped->m_prevObjective == OBJECTIVE_RUN_TO_AREA || ped->m_prevObjective == OBJECTIVE_GOTO_CHAR_ON_FOOT || ped->m_prevObjective == OBJECTIVE_KILL_CHAR_ON_FOOT)
			ped->m_prevObjective = OBJECTIVE_NONE;

		ped->RestorePreviousObjective();
#ifdef VC_PED_PORTS
		if(veh->pDriver && ped->CharCreatedBy == RANDOM_CHAR)
			veh->AutoPilot.m_nCruiseSpeed = 17;
#endif
	}

	veh->m_nGettingInFlags &= ~GetCarDoorFlag(ped->m_vehDoor);

	if (veh->bIsBus && !veh->m_nGettingInFlags)
		((CAutomobile*)veh)->SetBusDoorTimer(1000, 1);

	switch (ped->m_objective) {
		case OBJECTIVE_KILL_CHAR_ON_FOOT:
		case OBJECTIVE_KILL_CHAR_ANY_MEANS:
		case OBJECTIVE_LEAVE_CAR:
		case OBJECTIVE_FOLLOW_CAR_IN_CAR:
		case OBJECTIVE_GOTO_AREA_ANY_MEANS:
		case OBJECTIVE_GOTO_AREA_ON_FOOT:
		case OBJECTIVE_RUN_TO_AREA:
			break;
		default:
			ped->SetObjective(OBJECTIVE_NONE);
	}

	if (veh->pDriver == ped) {
		if (veh->bLowVehicle) {
			ped->m_pVehicleAnim = CAnimManager::BlendAnimation(ped->GetClump(),	ASSOCGRP_STD, ANIM_STD_CAR_SIT_LO, 100.0f);
		} else {
			ped->m_pVehicleAnim = CAnimManager::BlendAnimation(ped->GetClump(), ASSOCGRP_STD, ANIM_STD_CAR_SIT, 100.0f);
		}
	} else if (veh->bLowVehicle) {
		ped->m_pVehicleAnim = CAnimManager::BlendAnimation(ped->GetClump(), ASSOCGRP_STD, ANIM_STD_CAR_SIT_P_LO, 100.0f);
	} else {
		ped->m_pVehicleAnim = CAnimManager::BlendAnimation(ped->GetClump(), ASSOCGRP_STD, ANIM_STD_CAR_SIT_P, 100.0f);
	}
	
	ped->StopNonPartialAnims();
	if (veh->bIsBus)
		ped->bRenderPedInCar = false;

	// FIX: RegisterVehicleOfInterest not just registers the vehicle, but also updates register time. So remove the IsThisVehicleInteresting check.
#ifndef FIX_BUGS
	if (ped->IsPlayer() && !CCarCtrl::IsThisVehicleInteresting(veh) && veh->VehicleCreatedBy != MISSION_VEHICLE) {
#else
	if (ped->IsPlayer() && veh->VehicleCreatedBy != MISSION_VEHICLE) {
#endif
		CCarCtrl::RegisterVehicleOfInterest(veh);

		if (!veh->bHasBeenOwnedByPlayer && veh->VehicleCreatedBy != MISSION_VEHICLE)
			CEventList::RegisterEvent(EVENT_STEAL_CAR, EVENT_ENTITY_VEHICLE, veh, ped, 1500);

		veh->bHasBeenOwnedByPlayer = true;
	}
	ped->bChangedSeat = true;
}

bool
CPed::CanBeDeleted(void)
{
	if (bInVehicle)
		return false;

	switch (CharCreatedBy) {
		case RANDOM_CHAR:
			return true;
		case MISSION_CHAR:
			return false;
		default:
			return true;
	}
}

void
CPed::AddWeaponModel(int id)
{
	RpAtomic *atm;

	if (id != -1) {
#ifdef PED_SKIN
		if (IsClumpSkinned(GetClump())) {
			if (m_pWeaponModel)
				RemoveWeaponModel(-1);

			m_pWeaponModel = (RpAtomic*)CModelInfo::GetModelInfo(id)->CreateInstance();
		} else
#endif
		{
			atm = (RpAtomic*)CModelInfo::GetModelInfo(id)->CreateInstance();
			RwFrameDestroy(RpAtomicGetFrame(atm));
			RpAtomicSetFrame(atm, m_pFrames[PED_HANDR]->frame);
			RpClumpAddAtomic(GetClump(), atm);
		}
		m_wepModelID = id;
	}
}

static RwObject*
RemoveAllModelCB(RwObject *object, void *data)
{
	RpAtomic *atomic = (RpAtomic*)object;
	if (CVisibilityPlugins::GetAtomicModelInfo(atomic)) {
		RpClumpRemoveAtomic(RpAtomicGetClump(atomic), atomic);
		RpAtomicDestroy(atomic);
	}
	return object;
}

void
CPed::RemoveWeaponModel(int modelId)
{
	// modelId is not used!! This function just removes the current weapon.
#ifdef PED_SKIN
	if(IsClumpSkinned(GetClump())){
		if(m_pWeaponModel){
			RwFrame *frm = RpAtomicGetFrame(m_pWeaponModel);
			RpAtomicDestroy(m_pWeaponModel);
			RwFrameDestroy(frm);
			m_pWeaponModel = nil;
		}
	}else
#endif
		RwFrameForAllObjects(m_pFrames[PED_HANDR]->frame,RemoveAllModelCB,nil);
	m_wepModelID = -1;
}

uint32
CPed::GiveWeapon(eWeaponType weaponType, uint32 ammo)
{
	CWeapon &weapon = GetWeapon(weaponType);

	if (HasWeapon(weaponType)) {
		if (weapon.m_nAmmoTotal + ammo > 99999)
			weapon.m_nAmmoTotal = 99999;
		else
			weapon.m_nAmmoTotal += ammo;

		weapon.Reload();	
	} else {
		weapon.Initialise(weaponType, ammo);
		// TODO: It seems game uses this as both weapon count and max WeaponType we have, which is ofcourse erroneous.
		m_maxWeaponTypeAllowed++;
	}
	if (weapon.m_eWeaponState == WEAPONSTATE_OUT_OF_AMMO)
		weapon.m_eWeaponState = WEAPONSTATE_READY;

	return weaponType;
}

// Some kind of VC leftover I think
int
CPed::GetWeaponSlot(eWeaponType weaponType)
{
	if (HasWeapon(weaponType))
		return weaponType;
	else
		return -1;
}

void
CPed::SetCurrentWeapon(uint32 weaponType)
{
	CWeaponInfo *weaponInfo;
	if (HasWeapon(weaponType)) {
		weaponInfo = CWeaponInfo::GetWeaponInfo(GetWeapon()->m_eWeaponType);
		RemoveWeaponModel(weaponInfo->m_nModelId);

		m_currentWeapon = weaponType;

		weaponInfo = CWeaponInfo::GetWeaponInfo(GetWeapon()->m_eWeaponType);
		AddWeaponModel(weaponInfo->m_nModelId);
	}
}

void
CPed::GrantAmmo(eWeaponType weaponType, uint32 ammo)
{
	if (HasWeapon(weaponType)) {
		GetWeapon(weaponType).m_nAmmoTotal += ammo;
	} else {
		GetWeapon(weaponType).Initialise(weaponType, ammo);
		m_maxWeaponTypeAllowed++;
	}
}

void
CPed::SetAmmo(eWeaponType weaponType, uint32 ammo)
{
	if (HasWeapon(weaponType)) {
		GetWeapon(weaponType).m_nAmmoTotal = ammo;
	} else {
		GetWeapon(weaponType).Initialise(weaponType, ammo);
		m_maxWeaponTypeAllowed++;
	}
}

void
CPed::ClearWeapons(void)
{
	CWeaponInfo *currentWeapon = CWeaponInfo::GetWeaponInfo(GetWeapon()->m_eWeaponType);
	RemoveWeaponModel(currentWeapon->m_nModelId);

	m_maxWeaponTypeAllowed = WEAPONTYPE_BASEBALLBAT;
	m_currentWeapon = WEAPONTYPE_UNARMED;

	currentWeapon = CWeaponInfo::GetWeaponInfo(GetWeapon()->m_eWeaponType);
	AddWeaponModel(currentWeapon->m_nModelId);
	for(int i = 0; i < WEAPONTYPE_TOTAL_INVENTORY_WEAPONS; i++) {
		CWeapon &weapon = GetWeapon(i);
		weapon.m_eWeaponType = WEAPONTYPE_UNARMED;
		weapon.m_eWeaponState = WEAPONSTATE_READY;
		weapon.m_nAmmoInClip = 0;
		weapon.m_nAmmoTotal = 0;
		weapon.m_nTimer = 0;
	}
}

void
CPed::PreRender(void)
{
	CShadows::StoreShadowForPed(this,
		CTimeCycle::m_fShadowDisplacementX[CTimeCycle::m_CurrentStoredValue], CTimeCycle::m_fShadowDisplacementY[CTimeCycle::m_CurrentStoredValue],
		CTimeCycle::m_fShadowFrontX[CTimeCycle::m_CurrentStoredValue], CTimeCycle::m_fShadowFrontY[CTimeCycle::m_CurrentStoredValue],
		CTimeCycle::m_fShadowSideX[CTimeCycle::m_CurrentStoredValue], CTimeCycle::m_fShadowSideY[CTimeCycle::m_CurrentStoredValue]);

#ifdef PED_SKIN
	if(IsClumpSkinned(GetClump())){
		UpdateRpHAnim();

		if(bBodyPartJustCameOff && m_bodyPartBleeding == PED_HEAD){
			// scale head to 0 if shot off
			RpHAnimHierarchy *hier = GetAnimHierarchyFromSkinClump(GetClump());
			int32 idx = RpHAnimIDGetIndex(hier, ConvertPedNode2BoneTag(PED_HEAD));
			RwMatrix *head = &RpHAnimHierarchyGetMatrixArray(hier)[idx];
			RwV3d zero = { 0.0f, 0.0f, 0.0f };
			RwMatrixScale(head, &zero, rwCOMBINEPRECONCAT);
		}
	}
#endif

	if (bBodyPartJustCameOff && bIsPedDieAnimPlaying && m_bodyPartBleeding != -1 && (CTimer::GetFrameCounter() & 7) > 3) {
		CVector bloodDir(0.0f, 0.0f, 0.0f);
		CVector bloodPos(0.0f, 0.0f, 0.0f);

		TransformToNode(bloodPos, m_bodyPartBleeding);

		switch (m_bodyPartBleeding) {
			case PED_HEAD:
				bloodDir = 0.1f * GetUp();
				break;
			case PED_UPPERARML:
				bloodDir = 0.04f * GetUp() - 0.04f * GetRight();
				break;
			case PED_UPPERARMR:
				bloodDir = 0.04f * GetUp() - 0.04f * GetRight();
				break;
			case PED_UPPERLEGL:
				bloodDir = 0.04f * GetUp() + 0.05f * GetForward();
				break;
			case PED_UPPERLEGR:
				bloodDir = 0.04f * GetUp() + 0.05f * GetForward();
				break;
			default:
				bloodDir = CVector(0.0f, 0.0f, 0.0f);
				break;
		}

		for(int i = 0; i < 4; i++)
			CParticle::AddParticle(PARTICLE_BLOOD_SPURT, bloodPos, bloodDir, nil, 0.0f, 0, 0, 0, 0);
	}
	if (CWeather::Rain > 0.3f && TheCamera.SoundDistUp > 15.0f) {
		if ((TheCamera.GetPosition() - GetPosition()).Magnitude() < 25.0f) {
			bool doSplashUp = true;
			CColModel *ourCol = CModelInfo::GetModelInfo(GetModelIndex())->GetColModel();
			CVector speed = FindPlayerSpeed();

			if (Abs(speed.x) <= 0.05f && Abs(speed.y) <= 0.05f) {
				if (!OnGround() && m_nPedState != PED_ATTACK && m_nPedState != PED_FIGHT) {
					if (!IsPedHeadAbovePos(0.3f) || RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_IDLE_TIRED)) {
						doSplashUp = false;
					}
				} else
					doSplashUp = false;
			} else
				doSplashUp = false;

			if (doSplashUp && ourCol->numSpheres > 0) {
				for(int i = 0; i < ourCol->numSpheres; i++) {
					CColSphere *sphere = &ourCol->spheres[i];
					CVector splashPos;
					switch (sphere->piece) {
						case PEDPIECE_LEFTARM:
						case PEDPIECE_RIGHTARM:
						case PEDPIECE_HEAD:
							splashPos = GetMatrix() * ourCol->spheres[i].center;
							splashPos.z += 0.7f * sphere->radius;
							splashPos.x += CGeneral::GetRandomNumberInRange(-0.15f, 0.15f);
							splashPos.y += CGeneral::GetRandomNumberInRange(-0.15f, 0.15f);
							CParticle::AddParticle(PARTICLE_RAIN_SPLASHUP, splashPos, CVector(0.0f, 0.0f, 0.0f), nil, 0.0f, 0, 0, CGeneral::GetRandomNumber() & 1, 0);
							break;
						default:
							break;
					}
				}
			}
		}
	}
}

void
CPed::Render(void)
{
	if (bInVehicle && m_nPedState != PED_EXIT_CAR && m_nPedState != PED_DRAG_FROM_CAR) {
		if (!bRenderPedInCar)
			return;

		float camDistSq = (TheCamera.GetPosition() - GetPosition()).MagnitudeSqr();
		if (camDistSq > SQR(25.0f * TheCamera.LODDistMultiplier))
			return;
	}

	CEntity::Render();

#ifdef PED_SKIN
	if(IsClumpSkinned(GetClump())){
		renderLimb(PED_HEAD);
		renderLimb(PED_HANDL);
		renderLimb(PED_HANDR);
	}
	if(m_pWeaponModel && IsClumpSkinned(GetClump())){
		RpHAnimHierarchy *hier = GetAnimHierarchyFromSkinClump(GetClump());
		int idx = RpHAnimIDGetIndex(hier, m_pFrames[PED_HANDR]->nodeID);
		RwMatrix *mat = &RpHAnimHierarchyGetMatrixArray(hier)[idx];
		RwFrame *frame = RpAtomicGetFrame(m_pWeaponModel);
		*RwFrameGetMatrix(frame) = *mat;
		RwFrameUpdateObjects(frame);
		RpAtomicRender(m_pWeaponModel);
	}
#endif
}

void
CPed::CheckAroundForPossibleCollisions(void)
{
	CVector ourCentre, objCentre;
	CEntity *objects[8];
	int16 maxObject;

	if (CTimer::GetTimeInMilliseconds() <= m_nPedStateTimer)
		return;

	GetBoundCentre(ourCentre);

	CWorld::FindObjectsInRange(ourCentre, 10.0f, true, &maxObject, 6, objects, false, true, false, true, false);
	for (int i = 0; i < maxObject; i++) {
		CEntity *object = objects[i];
		if (bRunningToPhone) {
			if (gPhoneInfo.PhoneAtThisPosition(object->GetPosition()))
				break;
		}
		object->GetBoundCentre(objCentre);
		float radius = object->GetBoundRadius();
		if (radius > 4.5f || radius < 1.0f)
			radius = 1.0f;

		// Developers gave up calculating Z diff. later according to asm.
		float diff = CVector(ourCentre - objCentre).MagnitudeSqr2D();

		if (sq(radius + 1.0f) > diff)
			m_fRotationDest += DEGTORAD(22.5f);
	}
}

void
CPed::SetIdle(void)
{
	if (m_nPedState != PED_IDLE && m_nPedState != PED_MUG && m_nPedState != PED_FLEE_ENTITY) {
#ifdef VC_PED_PORTS
		if (m_nPedState == PED_AIM_GUN)
			ClearPointGunAt();

		m_nLastPedState = PED_NONE;
#endif
		SetPedState(PED_IDLE);
		SetMoveState(PEDMOVE_STILL);
	}
	if (m_nWaitState == WAITSTATE_FALSE) {
		m_nWaitTimer = CTimer::GetTimeInMilliseconds() + CGeneral::GetRandomNumberInRange(2000, 4000);
	}
}

void
CPed::Idle(void)
{
	CVehicle *veh = m_pMyVehicle;
	if (veh && veh->m_nGettingOutFlags && m_vehDoor) {

		if (veh->m_nGettingOutFlags & GetCarDoorFlag(m_vehDoor)) {

			if (m_objective != OBJECTIVE_KILL_CHAR_ON_FOOT) {

				CVector doorPos = GetPositionToOpenCarDoor(veh, m_vehDoor);
				CVector doorDist = GetPosition() - doorPos;

				if (doorDist.MagnitudeSqr() < sq(0.5f)) {
					SetMoveState(PEDMOVE_WALK);
					return;
				}
			}
		}
	}

	CAnimBlendAssociation *armedIdleAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_IDLE_BIGGUN);
	CAnimBlendAssociation *unarmedIdleAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_IDLE);
	int waitTime;

	if (m_nMoveState == PEDMOVE_STILL) {

		eWeaponType curWeapon = GetWeapon()->m_eWeaponType;
		if (!armedIdleAssoc ||
			CTimer::GetTimeInMilliseconds() <= m_nWaitTimer && curWeapon != WEAPONTYPE_UNARMED && curWeapon != WEAPONTYPE_MOLOTOV && curWeapon != WEAPONTYPE_GRENADE) {

			if ((!GetWeapon()->IsType2Handed() || curWeapon == WEAPONTYPE_SHOTGUN) && curWeapon != WEAPONTYPE_BASEBALLBAT
				|| !unarmedIdleAssoc || unarmedIdleAssoc->blendAmount <= 0.95f || m_nWaitState != WAITSTATE_FALSE || CTimer::GetTimeInMilliseconds() <= m_nWaitTimer) {

				m_moved = CVector2D(0.0f, 0.0f);
				return;
			}
			CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_IDLE_BIGGUN, 3.0f);
			waitTime = CGeneral::GetRandomNumberInRange(4000, 7500);
		} else {
			armedIdleAssoc->blendDelta = -2.0f;
			armedIdleAssoc->flags |= ASSOC_DELETEFADEDOUT;
			waitTime = CGeneral::GetRandomNumberInRange(3000, 8500);
		}
		m_nWaitTimer = CTimer::GetTimeInMilliseconds() + waitTime;
	} else {
		if (armedIdleAssoc) {
			armedIdleAssoc->blendDelta = -8.0f;
			armedIdleAssoc->flags |= ASSOC_DELETEFADEDOUT;
			m_nWaitTimer = 0;
		}
		if (!IsPlayer())
			SetMoveState(PEDMOVE_STILL);
	}
	m_moved = CVector2D(0.0f, 0.0f);
}

void
CPed::ClearPause(void)
{
	RestorePreviousState();
}

void
CPed::Pause(void)
{
	m_moved = CVector2D(0.0f, 0.0f);
	if (CTimer::GetTimeInMilliseconds() > m_leaveCarTimer)
		ClearPause();
}

void
CPed::SetFall(int extraTime, AnimationId animId, uint8 evenIfNotInControl)
{
	if (!IsPedInControl() && (!evenIfNotInControl || DyingOrDead()))
		return;

	ClearLookFlag();
	ClearAimFlag();
	SetStoredState();
	SetPedState(PED_FALL);
	CAnimBlendAssociation *fallAssoc = RpAnimBlendClumpGetAssociation(GetClump(), animId);
	
	if (fallAssoc) {
		fallAssoc->SetCurrentTime(0.0f);
		fallAssoc->blendAmount = 0.0f;
		fallAssoc->blendDelta = 8.0f;
		fallAssoc->SetRun();
	} else {
		fallAssoc = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, animId, 8.0f);
	}

	if (extraTime == -1) {
		m_getUpTimer = UINT32_MAX;
	} else if (fallAssoc) {
		if (IsPlayer()) {
			m_getUpTimer = 1000.0f * fallAssoc->hierarchy->totalLength
				+ CTimer::GetTimeInMilliseconds()
				+ 500.0f;
		} else {
			m_getUpTimer = 1000.0f * fallAssoc->hierarchy->totalLength
				+ CTimer::GetTimeInMilliseconds()
				+ extraTime
				+ ((m_randomSeed + CTimer::GetFrameCounter()) % 1000);
		}
	} else {
		m_getUpTimer = extraTime
			+ CTimer::GetTimeInMilliseconds()
			+ 1000
			+ ((m_randomSeed + CTimer::GetFrameCounter()) % 1000);
	}
	bFallenDown = true;
}

void
CPed::ClearFall(void)
{
	SetGetUp();
}

void
CPed::Fall(void)
{
	if (m_getUpTimer != UINT32_MAX && CTimer::GetTimeInMilliseconds() > m_getUpTimer
#ifdef VC_PED_PORTS
		&& bIsStanding
#endif
		)
		ClearFall();

	// VC plays animations ANIM_STD_FALL_ONBACK and ANIM_STD_FALL_ONFRONT in here, which doesn't exist in III.
}

bool
CPed::CheckIfInTheAir(void)
{
	if (bInVehicle)
		return false;

	CVector pos = GetPosition();
	CColPoint foundColPoint;
	CEntity *foundEntity;

	float startZ = pos.z - 1.54f;
	bool foundGround = CWorld::ProcessVerticalLine(pos, startZ, foundColPoint, foundEntity, true, true, false, true, false, false, nil);
	if (!foundGround && m_nPedState != PED_JUMP)
	{
		pos.z -= FEET_OFFSET;
		if (CWorld::TestSphereAgainstWorld(pos, 0.15f, this, true, false, false, false, false, false))
			foundGround = true;
	}
	return !foundGround;
}

void
CPed::SetInTheAir(void)
{
	if (bIsInTheAir)
		return;

	bIsInTheAir = true;
	CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_FALL_GLIDE, 4.0f);

	if (m_nPedState == PED_ATTACK) {
		ClearAttack();
		ClearPointGunAt();
	} else if (m_nPedState == PED_FIGHT) {
		EndFight(ENDFIGHT_FAST);
	}

}

void
CPed::InTheAir(void)
{
	CColPoint foundCol;
	CEntity *foundEnt;

	CVector ourPos = GetPosition();
	CVector bitBelow = GetPosition();
	bitBelow.z -= 4.04f;

	if (m_vecMoveSpeed.z < 0.0f && !bIsPedDieAnimPlaying) {
		if (!DyingOrDead()) {
			if (CWorld::ProcessLineOfSight(ourPos, bitBelow, foundCol, foundEnt, true, true, false, true, false, false, false)) {
				if (GetPosition().z - foundCol.point.z < 1.3f
#ifdef VC_PED_PORTS
					|| bIsStanding
#endif
					)
					SetLanding();
			} else {
				if (!RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_FALL)) {
					if (m_vecMoveSpeed.z < -0.1f)
						CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_FALL, 4.0f);
				}
			}
		}
	}
}

void
CPed::SetLanding(void)
{
	if (DyingOrDead())
		return;

	CAnimBlendAssociation *fallAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_FALL);
	CAnimBlendAssociation *landAssoc;

	RpAnimBlendClumpSetBlendDeltas(GetClump(), ASSOC_PARTIAL, -1000.0f);
	if (fallAssoc) {
		landAssoc = CAnimManager::AddAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_FALL_COLLAPSE);
		DMAudio.PlayOneShot(m_audioEntityId, SOUND_FALL_COLLAPSE, 1.0f);

		if (IsPlayer())
			Say(SOUND_PED_LAND);

	} else {
		landAssoc = CAnimManager::AddAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_FALL_LAND);
		DMAudio.PlayOneShot(m_audioEntityId, SOUND_FALL_LAND, 1.0f);
	}

	landAssoc->SetFinishCallback(PedLandCB, this);
	bIsInTheAir = false;
	bIsLanding = true;
}

void
CPed::SetGetUp(void)
{
	if (m_nPedState == PED_GETUP && bGetUpAnimStarted)
		return;

	if (!CanSetPedState())
		return;

	if (m_fHealth >= 1.0f || IsPedHeadAbovePos(-0.3f)) {
		if (bUpdateAnimHeading) {
			m_fRotationCur = CGeneral::LimitRadianAngle(m_fRotationCur);
			m_fRotationCur -= HALFPI;
			bUpdateAnimHeading = false;
		}
		if (m_nPedState != PED_GETUP) {
			SetStoredState();
			SetPedState(PED_GETUP);
		}

		CVehicle *collidingVeh = (CVehicle*)m_pCollidingEntity;
		CVehicle *veh = (CVehicle*)CPedPlacement::IsPositionClearOfCars(&GetPosition());
		if (veh && veh->m_vehType != VEHICLE_TYPE_BIKE ||
			collidingVeh && collidingVeh->IsVehicle() && collidingVeh->m_vehType != VEHICLE_TYPE_BIKE
			&& ((uint8)(CTimer::GetFrameCounter() + m_randomSeed + 5) % 8 ||
		         CCollision::ProcessColModels(GetMatrix(), *GetColModel(), collidingVeh->GetMatrix(), *collidingVeh->GetColModel(),
					aTempPedColPts, nil, nil) > 0)) {

			bGetUpAnimStarted = false;
			if (IsPlayer())
				InflictDamage(nil, WEAPONTYPE_RUNOVERBYCAR, CTimer::GetTimeStep(), PEDPIECE_TORSO, 0);
			else {
				if (!CPad::GetPad(0)->ArePlayerControlsDisabled())
					return;

				InflictDamage(nil, WEAPONTYPE_RUNOVERBYCAR, 1000.0f, PEDPIECE_TORSO, 0);
			}
			return;
		}
		bGetUpAnimStarted = true;
		m_pCollidingEntity = nil;
		bKnockedUpIntoAir = false;
		CAnimBlendAssociation *animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_RUNFAST);
		if (animAssoc) {
			if (RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_RUN)) {
				CAnimManager::BlendAnimation(GetClump(), m_animGroup, ANIM_STD_RUN, 8.0f);
			} else {
				CAnimManager::BlendAnimation(GetClump(), m_animGroup, ANIM_STD_IDLE, 8.0f);
			}
			animAssoc->flags |= ASSOC_DELETEFADEDOUT;
		}

		if (RpAnimBlendClumpGetFirstAssociation(GetClump(), ASSOC_FRONTAL))
			animAssoc = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_GET_UP_FRONT, 1000.0f);
		else
			animAssoc = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_GET_UP, 1000.0f);

		animAssoc->SetFinishCallback(PedGetupCB,this);
	} else {
		m_fHealth = 0.0f;
		SetDie(ANIM_STD_NUM, 4.0f, 0.0f);
	}
}

void
CPed::Mug(void)
{
	if (m_pSeekTarget && m_pSeekTarget->IsPed()) {

		if (CTimer::GetTimeInMilliseconds() <= m_attackTimer - 2000) {
			if ((m_pSeekTarget->GetPosition() - GetPosition()).Magnitude() > 3.0f)
				m_wepSkills = 50;

			Say(SOUND_PED_MUGGING);
			((CPed*)m_pSeekTarget)->Say(SOUND_PED_ROBBED);
		} else {
			SetWanderPath(CGeneral::GetRandomNumber() & 7);
			SetFlee(m_pSeekTarget, 20000);
		}

	} else {
		SetIdle();
	}
}

void
CPed::SetLookTimer(int time)
{
	if (CTimer::GetTimeInMilliseconds() > m_lookTimer) {
		m_lookTimer = CTimer::GetTimeInMilliseconds() + time;
	}
}

void
CPed::SetAttackTimer(uint32 time)
{
	if (CTimer::GetTimeInMilliseconds() > m_attackTimer)
		m_attackTimer = Max(m_shootTimer, CTimer::GetTimeInMilliseconds()) + time;
}

void
CPed::SetShootTimer(uint32 time)
{
	if (CTimer::GetTimeInMilliseconds() > m_shootTimer) {
		m_shootTimer = CTimer::GetTimeInMilliseconds() + time;
	}
}

void
CPed::ClearLook(void)
{
	RestorePreviousState();
	ClearLookFlag();
}

void
CPed::Look(void)
{
	// UNUSED: This is a perfectly empty function.
}

bool
CPed::TurnBody(void)
{
	bool turnDone = true;

	if (m_pLookTarget)
		m_fLookDirection = CGeneral::GetRadianAngleBetweenPoints(
					m_pLookTarget->GetPosition().x,
					m_pLookTarget->GetPosition().y,
					GetPosition().x,
					GetPosition().y);

	float limitedLookDir = CGeneral::LimitRadianAngle(m_fLookDirection);
	float currentRot = m_fRotationCur;

	if (currentRot - PI > limitedLookDir)
		limitedLookDir += 2 * PI;
	else if (PI + currentRot < limitedLookDir)
		limitedLookDir -= 2 * PI;

	float neededTurn = currentRot - limitedLookDir;
	m_fRotationDest = limitedLookDir;

	if (Abs(neededTurn) > 0.05f) {
		turnDone = false;
		currentRot -= neededTurn * 0.2f;
	}

	m_fRotationCur = currentRot;
	m_fLookDirection = limitedLookDir;
	return turnDone;
}

void
CPed::SetSeek(CVector pos, float distanceToCountDone)
{
	if (!IsPedInControl()
		|| (m_nPedState == PED_SEEK_POS && m_vecSeekPos.x == pos.x && m_vecSeekPos.y == pos.y))
		return;

	if (GetWeapon()->m_eWeaponType == WEAPONTYPE_M16
		|| GetWeapon()->m_eWeaponType == WEAPONTYPE_AK47
		|| GetWeapon()->m_eWeaponType == WEAPONTYPE_SNIPERRIFLE
		|| GetWeapon()->m_eWeaponType == WEAPONTYPE_ROCKETLAUNCHER
		|| GetWeapon()->m_eWeaponType == WEAPONTYPE_SHOTGUN) {
		ClearPointGunAt();
	}

	if (m_nPedState != PED_SEEK_POS)
		SetStoredState();

	SetPedState(PED_SEEK_POS);
	m_distanceToCountSeekDone = distanceToCountDone;
	m_vecSeekPos = pos;
}

void
CPed::SetSeek(CEntity *seeking, float distanceToCountDone)
{
	if (!IsPedInControl())
		return;

	if (m_nPedState == PED_SEEK_ENTITY && m_pSeekTarget == seeking)
		return;

	if (!seeking)
		return;

	if (m_nPedState != PED_SEEK_ENTITY)
		SetStoredState();

	SetPedState(PED_SEEK_ENTITY);
	m_distanceToCountSeekDone = distanceToCountDone;
	m_pSeekTarget = seeking;
	m_pSeekTarget->RegisterReference((CEntity **) &m_pSeekTarget);
	SetMoveState(PEDMOVE_STILL);
}

void
CPed::ClearSeek(void)
{
	SetIdle();
	bRunningToPhone = false;
}

bool
CPed::Seek(void)
{
	float distanceToCountItDone = m_distanceToCountSeekDone;
	eMoveState nextMove = PEDMOVE_NONE;

	if (m_objective != OBJECTIVE_ENTER_CAR_AS_DRIVER) {

		if (m_nPedState != PED_EXIT_TRAIN && m_nPedState != PED_ENTER_TRAIN && m_nPedState != PED_SEEK_IN_BOAT &&
			m_objective != OBJECTIVE_ENTER_CAR_AS_PASSENGER && m_objective != OBJECTIVE_SOLICIT_VEHICLE && !bDuckAndCover) {
			
			if ((!m_pedInObjective || !m_pedInObjective->bInVehicle)
				&& !((CTimer::GetFrameCounter() + (m_randomSeed % 256) + 17) & 7)) {

				CEntity *obstacle = CWorld::TestSphereAgainstWorld(m_vecSeekPos, 0.4f, nil,
									false, true, false, false, false, false);

				if (obstacle) {
					if (!obstacle->IsVehicle() || ((CVehicle*)obstacle)->m_vehType == VEHICLE_TYPE_CAR) {
						distanceToCountItDone = 2.5f;
					} else {
						CVehicleModelInfo *vehModel = (CVehicleModelInfo *)CModelInfo::GetModelInfo(obstacle->GetModelIndex());
						float yLength = vehModel->GetColModel()->boundingBox.max.y
										- vehModel->GetColModel()->boundingBox.min.y;
						distanceToCountItDone = yLength * 0.55f;
					}
				}
			}
		}
	}

	if (!m_pSeekTarget && m_nPedState == PED_SEEK_ENTITY)
		ClearSeek();

	float seekPosDist = (m_vecSeekPos - GetPosition()).Magnitude2D();
	if (seekPosDist < 2.0f || m_objective == OBJECTIVE_GOTO_AREA_ON_FOOT) {

		if (m_objective == OBJECTIVE_FOLLOW_CHAR_IN_FORMATION) {

			if (m_pedInObjective->m_nMoveState != PEDMOVE_STILL)
				nextMove = m_pedInObjective->m_nMoveState;
		} else
			nextMove = PEDMOVE_WALK;

	} else if (m_objective != OBJECTIVE_FOLLOW_CHAR_IN_FORMATION) {

		if (m_objective == OBJECTIVE_KILL_CHAR_ON_FOOT || m_objective == OBJECTIVE_KILL_CHAR_ANY_MEANS || m_objective == OBJECTIVE_RUN_TO_AREA || bIsRunning)
			nextMove = PEDMOVE_RUN;
		else
			nextMove = PEDMOVE_WALK;

	} else if (seekPosDist <= 2.0f) {

		if (m_pedInObjective->m_nMoveState != PEDMOVE_STILL)
			nextMove = m_pedInObjective->m_nMoveState;

	} else {
		nextMove = PEDMOVE_RUN;
	}

	if (m_nPedState == PED_SEEK_ENTITY) {
		if (m_pSeekTarget->IsPed()) {
			if (((CPed*)m_pSeekTarget)->bInVehicle)
				distanceToCountItDone += 2.0f;
		}
	}

	if (seekPosDist >= distanceToCountItDone) {
		if (bIsRunning)
			nextMove = PEDMOVE_RUN;

		if (CTimer::GetTimeInMilliseconds() <= m_nPedStateTimer) {

			if (m_actionX != 0.0f && m_actionY != 0.0f) {

				m_fRotationDest = CGeneral::GetRadianAngleBetweenPoints(
					m_actionX, m_actionY,
					GetPosition().x, GetPosition().y);

				float neededTurn = Abs(m_fRotationDest - m_fRotationCur);

				if (neededTurn > PI)
					neededTurn = TWOPI - neededTurn;

				if (neededTurn > HALFPI) {
					if (seekPosDist >= 1.0f) {
						if (seekPosDist < 2.0f) {
							if (bIsRunning)
								nextMove = PEDMOVE_RUN;
							else
								nextMove = PEDMOVE_WALK;
						}
					} else {
						nextMove = PEDMOVE_STILL;
					}
				}

				CVector2D moveDist(GetPosition().x - m_actionX, GetPosition().y - m_actionY);
				if (moveDist.Magnitude() < 0.5f) {
					m_nPedStateTimer = 0;
					m_actionX = 0;
					m_actionY = 0;
				}
			}
		} else {
			m_fRotationDest = CGeneral::GetRadianAngleBetweenPoints(
				m_vecSeekPos.x, m_vecSeekPos.y,
				GetPosition().x, GetPosition().y);

			float neededTurn = Abs(m_fRotationDest - m_fRotationCur);

			if (neededTurn > PI)
				neededTurn = TWOPI - neededTurn;

			if (neededTurn > HALFPI) {
				if (seekPosDist >= 1.0 && neededTurn <= DEGTORAD(135.0f)) {
					if (seekPosDist < 2.0f)
						nextMove = PEDMOVE_WALK;
				} else {
					nextMove = PEDMOVE_STILL;
				}
			}
		}

		if (((m_nPedState == PED_FLEE_POS || m_nPedState == PED_FLEE_ENTITY) && m_nMoveState < nextMove)
			|| (m_nPedState != PED_FLEE_POS && m_nPedState != PED_FLEE_ENTITY && m_objective != OBJECTIVE_GOTO_CHAR_ON_FOOT && m_nWaitState == WAITSTATE_FALSE)) {

			SetMoveState(nextMove);
		}
		
		SetMoveAnim();
		return false;
	}

	if ((m_objective != OBJECTIVE_FOLLOW_CHAR_IN_FORMATION || m_pedInObjective->m_nMoveState == PEDMOVE_STILL) && m_nMoveState != PEDMOVE_STILL) {
		m_nPedStateTimer = 0;
		m_actionX = 0;
		m_actionY = 0;
	}

	if (m_objective == OBJECTIVE_GOTO_AREA_ON_FOOT || m_objective == OBJECTIVE_RUN_TO_AREA || m_objective == OBJECTIVE_GOTO_AREA_ANY_MEANS) {
		if (m_pNextPathNode)
			m_pNextPathNode = nil;
		else
			bScriptObjectiveCompleted = true;

		bUsePedNodeSeek = true;
	}

	if (SeekFollowingPath(nil))
		m_nCurPathNode++;

	return true;
}

void
CPed::SetFlee(CVector2D const &from, int time)
{
	if (CTimer::GetTimeInMilliseconds() < m_nPedStateTimer || !IsPedInControl() || bKindaStayInSamePlace)
		return;

	if (m_nPedState != PED_FLEE_ENTITY) {
		SetStoredState();
		SetPedState(PED_FLEE_POS);
		SetMoveState(PEDMOVE_RUN);
		m_fleeFromPosX = from.x;
		m_fleeFromPosY = from.y;
	}

	bUsePedNodeSeek = true;
	m_pNextPathNode = nil;
	m_fleeTimer = CTimer::GetTimeInMilliseconds() + time;

	float angleToFace = CGeneral::GetRadianAngleBetweenPoints(
		GetPosition().x, GetPosition().y,
		from.x, from.y);

	m_fRotationDest = CGeneral::LimitRadianAngle(angleToFace);
	if (m_fRotationCur - PI > m_fRotationDest) {
		m_fRotationDest += 2 * PI;
	} else if (PI + m_fRotationCur < m_fRotationDest) {
		m_fRotationDest -= 2 * PI;
	}
}

void
CPed::SetFlee(CEntity *fleeFrom, int time)
{
	if (!IsPedInControl() || bKindaStayInSamePlace || !fleeFrom)
		return;

	SetStoredState();
	SetPedState(PED_FLEE_ENTITY);
	bUsePedNodeSeek = true;
	SetMoveState(PEDMOVE_RUN);
	m_fleeFrom = fleeFrom;
	m_fleeFrom->RegisterReference((CEntity **) &m_fleeFrom);

	if (time <= 0)
		m_fleeTimer = 0;
	else
		m_fleeTimer = CTimer::GetTimeInMilliseconds() + time;

	float angleToFace = CGeneral::GetRadianAngleBetweenPoints(
			GetPosition().x, GetPosition().y,
			fleeFrom->GetPosition().x, fleeFrom->GetPosition().y);

	m_fRotationDest = CGeneral::LimitRadianAngle(angleToFace);
	if (m_fRotationCur - PI > m_fRotationDest) {
		m_fRotationDest += 2 * PI;
	} else if (PI + m_fRotationCur < m_fRotationDest) {
		m_fRotationDest -= 2 * PI;
	}
}

void
CPed::ClearFlee(void)
{
	RestorePreviousState();
	bUsePedNodeSeek = false;
	m_chatTimer = 0;
	m_fleeTimer = 0;
}

void
CPed::Flee(void)
{
	if (CTimer::GetTimeInMilliseconds() > m_fleeTimer && m_fleeTimer) {
		bool mayFinishFleeing = true;
		if (m_nPedState == PED_FLEE_ENTITY) {
			if ((CVector2D(GetPosition()) - ms_vec2DFleePosition).MagnitudeSqr() < sq(30.0f))
				mayFinishFleeing = false;
		}

		if (mayFinishFleeing) {
			eMoveState moveState = m_nMoveState;
			ClearFlee();

			if (m_objective == OBJECTIVE_FLEE_CHAR_ON_FOOT_TILL_SAFE || m_objective == OBJECTIVE_FLEE_CHAR_ON_FOOT_ALWAYS)
				RestorePreviousObjective();

			if ((m_nPedState == PED_IDLE || m_nPedState == PED_WANDER_PATH) && CGeneral::GetRandomNumber() & 1) {
				SetWaitState(moveState <= PEDMOVE_WALK ? WAITSTATE_CROSS_ROAD_LOOK : WAITSTATE_FINISH_FLEE, nil);
			}
			return;
		}
		m_fleeTimer = CTimer::GetTimeInMilliseconds() + 5000;
	}

	if (bUsePedNodeSeek) {
		CPathNode *realLastNode = nil;
		uint8 nextDirection = 0;
		uint8 curDirectionShouldBe = 9; // means not defined yet

		if (m_nPedStateTimer < CTimer::GetTimeInMilliseconds()
			&& m_collidingThingTimer < CTimer::GetTimeInMilliseconds()) {

			if (m_pNextPathNode && CTimer::GetTimeInMilliseconds() > m_chatTimer)  {

				curDirectionShouldBe = CGeneral::GetNodeHeadingFromVector(GetPosition().x - ms_vec2DFleePosition.x, GetPosition().y - ms_vec2DFleePosition.y);
				if (m_nPathDir < curDirectionShouldBe)
					m_nPathDir += 8;

				int dirDiff = m_nPathDir - curDirectionShouldBe;
				if (dirDiff > 2 && dirDiff < 6) {
					realLastNode = nil;
					m_pLastPathNode = m_pNextPathNode;
					m_pNextPathNode = nil;
				}
			}

			if (m_pNextPathNode) {
				m_vecSeekPos = m_pNextPathNode->GetPosition();
				if (m_nMoveState == PEDMOVE_RUN)
					bIsRunning = true;

				eMoveState moveState = m_nMoveState;
				if (Seek()) {
					realLastNode = m_pLastPathNode;
					m_pLastPathNode = m_pNextPathNode;
					m_pNextPathNode = nil;
				}
				bIsRunning = false;
				SetMoveState(moveState);
			}
		}

		if (!m_pNextPathNode) {
			if (curDirectionShouldBe == 9) {
				curDirectionShouldBe = CGeneral::GetNodeHeadingFromVector(GetPosition().x - ms_vec2DFleePosition.x, GetPosition().y - ms_vec2DFleePosition.y);
			}
			ThePaths.FindNextNodeWandering(PATH_PED, GetPosition(), &m_pLastPathNode, &m_pNextPathNode,
				curDirectionShouldBe,
				&nextDirection);

			if (curDirectionShouldBe < nextDirection)
				curDirectionShouldBe += 8;
			
			if (m_pNextPathNode && m_pNextPathNode != realLastNode && m_pNextPathNode != m_pLastPathNode && curDirectionShouldBe - nextDirection != 4) {
				m_nPathDir = nextDirection;
				m_chatTimer = CTimer::GetTimeInMilliseconds() + 2000;
			} else {
				bUsePedNodeSeek = false;
				SetMoveState(PEDMOVE_RUN);
				Flee();
			}
		}
		return;
	}

	if ((m_nPedState == PED_FLEE_ENTITY || m_nPedState == PED_ON_FIRE) && m_nPedStateTimer < CTimer::GetTimeInMilliseconds()) {

		float angleToFleeFromPos = CGeneral::GetRadianAngleBetweenPoints(
			GetPosition().x,
			GetPosition().y,
			ms_vec2DFleePosition.x,
			ms_vec2DFleePosition.y);

		m_fRotationDest = CGeneral::LimitRadianAngle(angleToFleeFromPos);

		if (m_fRotationCur - PI > m_fRotationDest)
			m_fRotationDest += TWOPI;
		else if (PI + m_fRotationCur < m_fRotationDest)
			m_fRotationDest -= TWOPI;
	}

	if (CTimer::GetTimeInMilliseconds() & 0x20) {
		//CVector forwardPos = GetPosition();
		CMatrix forwardMat(GetMatrix());
		forwardMat.GetPosition() += Multiply3x3(forwardMat, CVector(0.0f, 4.0f, 0.0f));
		CVector forwardPos = forwardMat.GetPosition();

		CEntity *foundEnt;
		CColPoint foundCol;
		bool found = CWorld::ProcessVerticalLine(forwardPos, forwardMat.GetPosition().z - 100.0f, foundCol, foundEnt, 1, 0, 0, 0, 1, 0, 0);

		if (!found || Abs(forwardPos.z - forwardMat.GetPosition().z) > 1.0f) {
			m_fRotationDest += DEGTORAD(112.5f);
			m_nPedStateTimer = CTimer::GetTimeInMilliseconds() + 2000;
		}
	}

	if (CTimer::GetTimeInMilliseconds() >= m_collidingThingTimer)
		return;

	if (!m_collidingEntityWhileFleeing)
		return;

	double collidingThingPriorityMult = (double)(m_collidingThingTimer - CTimer::GetTimeInMilliseconds()) * 2.0 / 2500;

	if (collidingThingPriorityMult <= 1.5) {

		double angleToFleeEntity = CGeneral::GetRadianAngleBetweenPoints(
			GetPosition().x,
			GetPosition().y,
			m_collidingEntityWhileFleeing->GetPosition().x,
			m_collidingEntityWhileFleeing->GetPosition().y);
		angleToFleeEntity = CGeneral::LimitRadianAngle(angleToFleeEntity);

		double angleToFleeCollidingThing = CGeneral::GetRadianAngleBetweenPoints(
			m_vecDamageNormal.x,
			m_vecDamageNormal.y,
			0.0f,
			0.0f);
		angleToFleeCollidingThing = CGeneral::LimitRadianAngle(angleToFleeCollidingThing);

		if (angleToFleeEntity - PI > angleToFleeCollidingThing)
			angleToFleeCollidingThing += TWOPI;
		else if (PI + angleToFleeEntity < angleToFleeCollidingThing)
			angleToFleeCollidingThing -= TWOPI;

		if (collidingThingPriorityMult <= 1.0f) {
			// Range [0.0, 1.0]

			float angleToFleeBoth = (angleToFleeCollidingThing + angleToFleeEntity) * 0.5f;

			if (m_fRotationDest - PI > angleToFleeBoth)
				angleToFleeBoth += TWOPI;
			else if (PI + m_fRotationDest < angleToFleeBoth)
				angleToFleeBoth -= TWOPI;
	
			m_fRotationDest = (1.0f - collidingThingPriorityMult) * m_fRotationDest + collidingThingPriorityMult * angleToFleeBoth;
		} else {
			// Range (1.0, 1.5]

			double adjustedMult = (collidingThingPriorityMult - 1.0f) * 2.0f;
			m_fRotationDest = angleToFleeEntity * (1.0 - adjustedMult) + adjustedMult * angleToFleeCollidingThing;
		}
	} else {
		m_fRotationDest = CGeneral::GetRadianAngleBetweenPoints(
			m_vecDamageNormal.x,
			m_vecDamageNormal.y,
			0.0f,
			0.0f);
		m_fRotationDest = CGeneral::LimitRadianAngle(m_fRotationDest);
	}

	m_fRotationCur = CGeneral::LimitRadianAngle(m_fRotationCur);

	if (m_fRotationCur - PI > m_fRotationDest)
		m_fRotationDest += TWOPI;
	else if (PI + m_fRotationCur < m_fRotationDest)
		m_fRotationDest -= TWOPI;

}

// "Wander range" state is unused in game, and you can't use it without SetWanderRange anyway
void
CPed::WanderRange(void)
{
	bool arrived = Seek();
	if (arrived) {
		Idle();
		if ((m_randomSeed + 3 * CTimer::GetFrameCounter()) % 1000 > 997) {
			CVector2D newCoords2D = m_wanderRangeBounds->GetRandomPointInRange();
			SetSeek(CVector(newCoords2D.x, newCoords2D.y, GetPosition().z), 2.5f);
		}
	}
}

bool
CPed::SetWanderPath(int8 pathStateDest)
{
	uint8 nextPathState;

	if (IsPedInControl()) {
		if (bKindaStayInSamePlace) {
			SetIdle();
			return false;
		} else {
			m_nPathDir = pathStateDest;
			if (pathStateDest == 0)
				pathStateDest = CGeneral::GetRandomNumberInRange(1, 7);

			ThePaths.FindNextNodeWandering(PATH_PED, GetPosition(), &m_pLastPathNode, &m_pNextPathNode,
				m_nPathDir, &nextPathState);

			// Circular loop until we find a node for current m_nPathDir
			while (!m_pNextPathNode) {
				m_nPathDir = (m_nPathDir+1) % 8;

				// We're at where we started and couldn't find any node
				if (m_nPathDir == pathStateDest) {
					ClearAll();
					SetIdle();
					return false;
				}
				ThePaths.FindNextNodeWandering(PATH_PED, GetPosition(), &m_pLastPathNode, &m_pNextPathNode,
					m_nPathDir, &nextPathState);
			}

			// We did it, save next path state and return true
			m_nPathDir = nextPathState;
			SetPedState(PED_WANDER_PATH);
			SetMoveState(PEDMOVE_WALK);
			bIsRunning = false;
			return true;
		}
	} else {
		m_nPathDir = pathStateDest;
		bStartWanderPathOnFoot = true;
		return false;
	}
}

void
CPed::WanderPath(void)
{
	if (!m_pNextPathNode) {
		printf("THIS SHOULDN@T HAPPEN TOO OFTEN\n");
		SetIdle();
		return;
	}
	if (m_nWaitState == WAITSTATE_FALSE) {
		if (m_nMoveState == PEDMOVE_STILL || m_nMoveState == PEDMOVE_NONE)
			SetMoveState(PEDMOVE_WALK);
	}
	m_vecSeekPos = m_pNextPathNode->GetPosition();
	m_vecSeekPos.z += 1.0f;

	// Only returns true when ped is stuck(not stopped) I think, then we should assign new direction or wait state to him.
	if (!Seek())
		return;

  	CPathNode *previousLastNode = m_pLastPathNode;
	uint8 randVal = (m_randomSeed + 3 * CTimer::GetFrameCounter()) % 100;

	// We don't prefer 180-degree turns in normal situations
	uint8 dirWeWouldntPrefer = m_nPathDir;
	if (dirWeWouldntPrefer <= 3)
		dirWeWouldntPrefer += 4;
	else
		dirWeWouldntPrefer -= 4;

	CPathNode *nodeWeWouldntPrefer = nil;
	uint8 dirToSet = 9; // means undefined
	uint8 dirWeWouldntPrefer2 = 9; // means undefined
	if (randVal <= 90) {
		if (randVal > 80) {
			m_nPathDir += 2;
			m_nPathDir %= 8;
		}
	} else {
		m_nPathDir -= 2;
		if (m_nPathDir < 0)
			m_nPathDir += 8;
	}

	m_pLastPathNode = m_pNextPathNode;
	ThePaths.FindNextNodeWandering(PATH_PED, GetPosition(), &m_pLastPathNode, &m_pNextPathNode,
		m_nPathDir, &dirToSet);

	uint8 tryCount = 0;

	// NB: SetWanderPath checks for m_nPathDir == dirToStartWith, this one checks for tryCount > 7
	while (!m_pNextPathNode) {
		tryCount++;
		m_nPathDir = (m_nPathDir + 1) % 8;

		// We're at where we started and couldn't find any node
		if (tryCount > 7) {
			if (!nodeWeWouldntPrefer) {
				ClearAll();
				SetIdle();
				// Probably this text carried over here after copy-pasting this loop from early version of SetWanderPath.
				Error("Can't find valid path node, SetWanderPath, Ped.cpp");
				return;
			}
			m_pNextPathNode = nodeWeWouldntPrefer;
			dirToSet = dirWeWouldntPrefer2;
		} else {
			ThePaths.FindNextNodeWandering(PATH_PED, GetPosition(), &m_pLastPathNode, &m_pNextPathNode,
				m_nPathDir, &dirToSet);
			if (m_pNextPathNode) {
				if (dirToSet == dirWeWouldntPrefer) {
					nodeWeWouldntPrefer = m_pNextPathNode;
					dirWeWouldntPrefer2 = dirToSet;
					m_pNextPathNode = nil;
				}
			}
		}
	}

	m_nPathDir = dirToSet;
	if (m_pLastPathNode == m_pNextPathNode) {
		m_pNextPathNode = previousLastNode;
		SetWaitState(WAITSTATE_DOUBLEBACK, nil);
		Say(SOUND_PED_WAIT_DOUBLEBACK);
	} else if (ThePaths.TestForPedTrafficLight(m_pLastPathNode, m_pNextPathNode)) {
		SetWaitState(WAITSTATE_TRAFFIC_LIGHTS, nil);
	} else if (ThePaths.TestCrossesRoad(m_pLastPathNode, m_pNextPathNode)) {
		SetWaitState(WAITSTATE_CROSS_ROAD, nil);
	} else if (m_pNextPathNode == previousLastNode) {
		SetWaitState(WAITSTATE_DOUBLEBACK, nil);
		Say(SOUND_PED_WAIT_DOUBLEBACK);
	}
}

void
CPed::Avoid(void)
{
	CPed *nearestPed;

	if(m_pedStats->m_temper > m_pedStats->m_fear && m_pedStats->m_temper > 50)
		return;

	if (CTimer::GetTimeInMilliseconds() > m_nPedStateTimer) {

		if (m_nMoveState != PEDMOVE_NONE && m_nMoveState != PEDMOVE_STILL) {
			nearestPed = m_nearPeds[0];

			if (nearestPed && nearestPed->m_nPedState != PED_DEAD && nearestPed != m_pSeekTarget && nearestPed != m_pedInObjective) {

				// Check if this ped wants to avoid the nearest one
				if (CPedType::GetAvoid(m_nPedType) & CPedType::GetFlag(nearestPed->m_nPedType)) {

					// Further codes checks whether the distance between us and ped will be equal or below 1.0, if we walk up to him by 1.25 meters.
					// If so, we want to avoid it, so we turn our body 45 degree and look to somewhere else.

					// Game converts from radians to degress and back again here, doesn't make much sense
					CVector2D forward(-Sin(m_fRotationCur), Cos(m_fRotationCur));
					forward.Normalise();	// this is kinda pointless

					// Move forward 1.25 meters
					CVector2D testPosition = CVector2D(GetPosition()) + forward*1.25f;

					// Get distance to ped we want to avoid
					CVector2D distToPed = CVector2D(nearestPed->GetPosition()) - testPosition;

					if (distToPed.Magnitude() <= 1.0f && OurPedCanSeeThisOne((CEntity*)nearestPed)) {
						m_nPedStateTimer = CTimer::GetTimeInMilliseconds()
							+ 500 + (m_randomSeed + 3 * CTimer::GetFrameCounter())
							% 1000 / 5;

						m_fRotationDest += DEGTORAD(45.0f);
						if (!bIsLooking) {
							SetLookFlag(nearestPed, false);
							SetLookTimer(CGeneral::GetRandomNumberInRange(500, 800));
						}
					}
				}
			}
		}
	}
}

bool
CPed::SeekFollowingPath(CVector *unused)
{
	return m_nCurPathNode <= m_nPathNodes && m_nPathNodes;
}

bool
CPed::SetFollowPath(CVector dest)
{
	if (m_nPedState == PED_FOLLOW_PATH)
		return false;

	if (FindPlayerPed() != this)
		return false;

	if ((dest - GetPosition()).Magnitude() <= 2.0f)
		return false;

	CVector pointPoses[7];
	int16 pointsFound;
	CPedPath::CalcPedRoute(0, GetPosition(), dest, pointPoses, &pointsFound, 7);
	for(int i = 0; i < pointsFound; i++) {
		m_stPathNodeStates[i].x = pointPoses[i].x;
		m_stPathNodeStates[i].y = pointPoses[i].y;
	}

	m_nCurPathNode = 0;
	m_nPathNodes = pointsFound;
	if (m_nPathNodes < 1)
		return false;

	SetStoredState();
	SetPedState(PED_FOLLOW_PATH);
	SetMoveState(PEDMOVE_WALK);
	return true;
}

void
CPed::FollowPath(void)
{
	m_vecSeekPos.x = m_stPathNodeStates[m_nCurPathNode].x;
	m_vecSeekPos.y = m_stPathNodeStates[m_nCurPathNode].y;
	m_vecSeekPos.z = GetPosition().z;
	
	// Mysterious code
/*	int v4 = 0;
	int maxNodeIndex = m_nPathNodes - 1;
	if (maxNodeIndex > 0) {
		if (maxNodeIndex > 8)  {
			while (v4 < maxNodeIndex - 8)
				v4 += 8;
		}

		while (v4 < maxNodeIndex)
			v4++;

	}
*/
	if (Seek()) {
		m_nCurPathNode++;
		if (m_nCurPathNode == m_nPathNodes)
			RestorePreviousState();
	}
}

void
CPed::SetEvasiveStep(CEntity *reason, uint8 animType)
{
	AnimationId stepAnim;

	if (m_nPedState == PED_STEP_AWAY || !IsPedInControl() || ((IsPlayer() || !bRespondsToThreats) && animType == 0))
		return;

	float angleToFace = CGeneral::GetRadianAngleBetweenPoints(
		reason->GetPosition().x, reason->GetPosition().y,
		GetPosition().x, GetPosition().y);
	angleToFace = CGeneral::LimitRadianAngle(angleToFace);
	m_fRotationCur = CGeneral::LimitRadianAngle(m_fRotationCur);
	float neededTurn = Abs(angleToFace - m_fRotationCur);
	bool vehPressedHorn = false;

	if (neededTurn > PI)
		neededTurn = TWOPI - neededTurn;

	CVehicle *veh = (CVehicle*)reason;
	if (reason->IsVehicle() && veh->m_vehType == VEHICLE_TYPE_CAR) {
		if (veh->m_nCarHornTimer != 0) {
			vehPressedHorn = true;
			if (!IsPlayer())
				animType = 1;
		}
	}
	if (neededTurn <= DEGTORAD(90.0f) || veh->GetModelIndex() == MI_RCBANDIT || vehPressedHorn || animType != 0) {
		SetLookFlag(veh, true);
		if ((CGeneral::GetRandomNumber() & 1) && veh->GetModelIndex() != MI_RCBANDIT && animType == 0) {
			stepAnim = ANIM_STD_HAILTAXI;
		} else {

			float vehDirection = CGeneral::GetRadianAngleBetweenPoints(
				veh->m_vecMoveSpeed.x, veh->m_vecMoveSpeed.y,
				0.0f, 0.0f);

			// Let's turn our back to the "reason"
			angleToFace += PI;

			if (angleToFace > PI)
				angleToFace -=  TWOPI;

			// We don't want to run towards car's direction
			float dangerZone = angleToFace - vehDirection;
			dangerZone = CGeneral::LimitRadianAngle(dangerZone);

			// So, add or subtract 90deg (jump to left/right) according to that
			if (dangerZone > 0.0f)
				angleToFace = vehDirection - HALFPI;
			else
				angleToFace = vehDirection + HALFPI;

			stepAnim = ANIM_STD_NUM;
			if (animType == 0 || animType == 1)
				stepAnim = ANIM_STD_EVADE_STEP;
			else if (animType == 2)
				stepAnim = ANIM_STD_HANDSCOWER;
		}
		if (!RpAnimBlendClumpGetAssociation(GetClump(), stepAnim)) {
			CAnimBlendAssociation *stepAssoc = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, stepAnim, 8.0f);
			stepAssoc->flags &= ~ASSOC_DELETEFADEDOUT;
			stepAssoc->SetFinishCallback(PedEvadeCB, this);

			if (animType == 0)
				Say(SOUND_PED_EVADE);

			m_fRotationCur = CGeneral::LimitRadianAngle(angleToFace);
			ClearAimFlag();
			SetStoredState();
			SetPedState(PED_STEP_AWAY);
		}
	}
}

void
CPed::SetEvasiveDive(CPhysical *reason, uint8 onlyRandomJump)
{
	if (!IsPedInControl() || !bRespondsToThreats)
		return;

	CAnimBlendAssociation *animAssoc;
	float angleToFace, neededTurn;
	bool handsUp = false;

	angleToFace = m_fRotationCur;
	CVehicle *veh = (CVehicle*) reason;
	if (reason->IsVehicle() && veh->m_vehType == VEHICLE_TYPE_CAR && veh->m_nCarHornTimer != 0 && !IsPlayer()) {
		onlyRandomJump = true;
	}

	if (onlyRandomJump) {
		if (reason) {
			// Simple version of my bug fix below. Doesn't calculate "danger zone", selects jump direction randomly.
			// Also doesn't include random hands up, sound etc. Only used on player ped and peds running from gun shots.

			float vehDirection = CGeneral::GetRadianAngleBetweenPoints(
				veh->m_vecMoveSpeed.x, veh->m_vecMoveSpeed.y,
				0.0f, 0.0f);
			angleToFace = (CGeneral::GetRandomNumber() & 1) * PI + (-0.5f*PI) + vehDirection;
			angleToFace = CGeneral::LimitRadianAngle(angleToFace);
		}
	} else {
		if (IsPlayer()) {
			((CPlayerPed*)this)->m_nEvadeAmount = 5;
			((CPlayerPed*)this)->m_pEvadingFrom = reason;
			reason->RegisterReference((CEntity**) &((CPlayerPed*)this)->m_pEvadingFrom);
			return;
		}

		angleToFace = CGeneral::GetRadianAngleBetweenPoints(
			reason->GetPosition().x, reason->GetPosition().y,
			GetPosition().x, GetPosition().y);
		angleToFace = CGeneral::LimitRadianAngle(angleToFace);
		m_fRotationCur = CGeneral::LimitRadianAngle(m_fRotationCur);

		// FIX: Peds no more dive into cars. Taken from SetEvasiveStep, last if statement inverted
#ifdef FIX_BUGS
		float vehDirection = CGeneral::GetRadianAngleBetweenPoints(
			veh->m_vecMoveSpeed.x, veh->m_vecMoveSpeed.y,
			0.0f, 0.0f);

		// Let's turn our back to the "reason"
		angleToFace += PI;

		if (angleToFace > PI)
			angleToFace -= 2 * PI;

		// We don't want to dive towards car's direction
		float dangerZone = angleToFace - vehDirection;
		dangerZone = CGeneral::LimitRadianAngle(dangerZone);

		// So, add or subtract 90deg (jump to left/right) according to that
		if (dangerZone > 0.0f)
			angleToFace = 0.5f * PI + vehDirection;
		else
			angleToFace = vehDirection - 0.5f * PI;
#endif

		neededTurn = Abs(angleToFace - m_fRotationCur);

		if (neededTurn > PI)
			neededTurn = 2 * PI - neededTurn;

		if (neededTurn <= 0.5f*PI) {
			if (CGeneral::GetRandomNumber() & 1)
				handsUp = true;
		} else {
			if (CGeneral::GetRandomNumber() & 7)
				return;
		}
		Say(SOUND_PED_EVADE);
	}

	if (handsUp || !IsPlayer() && m_pedStats->m_flags & STAT_NO_DIVE) {
		m_fRotationCur = angleToFace;
		ClearLookFlag();
		ClearAimFlag();
		SetLookFlag(reason, true);
		animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_HANDSUP);
		if (animAssoc)
			return;

		animAssoc = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_HANDSUP, 8.0f);
		animAssoc->flags &= ~ASSOC_DELETEFADEDOUT;
		animAssoc->SetFinishCallback(PedEvadeCB, this);
		SetStoredState();
		SetPedState(PED_STEP_AWAY);
	} else {
		m_fRotationCur = angleToFace;
		ClearLookFlag();
		ClearAimFlag();
		SetStoredState();
		SetPedState(PED_DIVE_AWAY);
		animAssoc = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_EVADE_DIVE, 8.0f);
		animAssoc->SetFinishCallback(PedEvadeCB, this);
	}

	if (reason->IsVehicle() && m_nPedType == PEDTYPE_COP) {
		if (veh->pDriver && veh->pDriver->IsPlayer()) {
			CWanted *wanted = FindPlayerPed()->m_pWanted;
			wanted->RegisterCrime_Immediately(CRIME_RECKLESS_DRIVING, GetPosition(), (uintptr)this, false);
			wanted->RegisterCrime_Immediately(CRIME_SPEEDING, GetPosition(), (uintptr)this, false);
		}
	}
#ifdef PEDS_REPORT_CRIMES_ON_PHONE
	else if (reason->IsVehicle()) {
		if (veh->pDriver && veh->pDriver->IsPlayer()) {
			CWanted* wanted = FindPlayerPed()->m_pWanted;
			wanted->RegisterCrime(CRIME_RECKLESS_DRIVING, GetPosition(), (uintptr)this, false);
		}
	}
#endif
}

void
CPed::PedEvadeCB(CAnimBlendAssociation* animAssoc, void* arg)
{
	CPed* ped = (CPed*)arg;

	if (!animAssoc) {
		ped->ClearLookFlag();
		if (ped->m_nPedState == PED_DIVE_AWAY || ped->m_nPedState == PED_STEP_AWAY)
			ped->RestorePreviousState();
	
	} else if (animAssoc->animId == ANIM_STD_EVADE_DIVE) {
		ped->bUpdateAnimHeading = true;
		ped->ClearLookFlag();
		if (ped->m_nPedState == PED_DIVE_AWAY)
		{
			ped->m_getUpTimer = CTimer::GetTimeInMilliseconds() + 1;
			ped->SetPedState(PED_FALL);
		}
		animAssoc->flags &= ~ASSOC_FADEOUTWHENDONE;
		animAssoc->flags |= ASSOC_DELETEFADEDOUT;

	} else if (animAssoc->flags & ASSOC_FADEOUTWHENDONE) {
		ped->ClearLookFlag();
		if (ped->m_nPedState == PED_DIVE_AWAY || ped->m_nPedState == PED_STEP_AWAY)
			ped->RestorePreviousState();

	} else if (ped->m_nPedState != PED_ARRESTED) {
		animAssoc->flags |= ASSOC_DELETEFADEDOUT;
		if (animAssoc->blendDelta >= 0.0f)
			animAssoc->blendDelta = -4.0f;

		ped->ClearLookFlag();
		if (ped->m_nPedState == PED_DIVE_AWAY || ped->m_nPedState == PED_STEP_AWAY) {
			ped->RestorePreviousState();
		}
	}
}

void
CPed::SetDie(AnimationId animId, float delta, float speed)
{
	CPlayerPed *player = FindPlayerPed();
	if (player == this) {
		if (!player->m_bCanBeDamaged)
			return;
	}

	m_threatEntity = nil;
	if (DyingOrDead())
		return;

	if (m_nPedState == PED_FALL || m_nPedState == PED_GETUP)
		delta *=  0.5f;

	SetStoredState();
	ClearAll();
	m_fHealth = 0.0f;
	if (m_nPedState == PED_DRIVING) {
		if (!IsPlayer())
			FlagToDestroyWhenNextProcessed();
	} else if (bInVehicle) {
		if (m_pVehicleAnim)
			m_pVehicleAnim->blendDelta = -1000.0f;
	} else if (EnteringCar()) {
		QuitEnteringCar();
	}

	SetPedState(PED_DIE);
	if (animId == ANIM_STD_NUM) {
		bIsPedDieAnimPlaying = false;
	} else {
		CAnimBlendAssociation *dieAssoc = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, animId, delta);
		if (speed > 0.0f)
			dieAssoc->speed = speed;

		dieAssoc->flags &= ~ASSOC_FADEOUTWHENDONE;
		if (dieAssoc->IsRunning()) {
			dieAssoc->SetFinishCallback(FinishDieAnimCB, this);
			bIsPedDieAnimPlaying = true;
		}
	}

	Say(SOUND_PED_DEATH);
	if (m_nLastPedState == PED_ENTER_CAR || m_nLastPedState == PED_CARJACK)
		QuitEnteringCar();
	if (!bInVehicle)
		StopNonPartialAnims();

	m_bloodyFootprintCountOrDeathTime = CTimer::GetTimeInMilliseconds();
}

void
CPed::FinishDieAnimCB(CAnimBlendAssociation *animAssoc, void *arg)
{
	CPed *ped = (CPed*)arg;

	if (ped->bIsPedDieAnimPlaying)
		ped->bIsPedDieAnimPlaying = false;
}

void
CPed::SetDead(void)
{
	bUsesCollision = false;

	m_fHealth = 0.0f;
	if (m_nPedState == PED_DRIVING)
		bIsVisible = false;

	SetPedState(PED_DEAD);
	m_pVehicleAnim = nil;
	m_pCollidingEntity = nil;

	CWeaponInfo *weapon = CWeaponInfo::GetWeaponInfo(GetWeapon()->m_eWeaponType);
	RemoveWeaponModel(weapon->m_nModelId);

	m_currentWeapon = WEAPONTYPE_UNARMED;
	CEventList::RegisterEvent(EVENT_INJURED_PED, EVENT_ENTITY_PED, this, nil, 250);
	if (this != FindPlayerPed()) {
		CreateDeadPedWeaponPickups();
		CreateDeadPedMoney();
	}

	m_bloodyFootprintCountOrDeathTime = CTimer::GetTimeInMilliseconds();
	m_deadBleeding = false;
	bDoBloodyFootprints = false;
	bVehExitWillBeInstant = false;
	CEventList::RegisterEvent(EVENT_DEAD_PED, EVENT_ENTITY_PED, this, nil, 1000);
}

void
CPed::Die(void)
{
	// UNUSED: This is a perfectly empty function.
}

void
CPed::SetChat(CEntity *chatWith, uint32 time)
{
	if(m_nPedState != PED_CHAT)
		SetStoredState();

	SetPedState(PED_CHAT);
	SetMoveState(PEDMOVE_STILL);
#if defined VC_PED_PORTS || defined FIX_BUGS
	m_lookTimer = 0;
#endif
	SetLookFlag(chatWith, true);
	m_chatTimer = CTimer::GetTimeInMilliseconds() + time;
	m_lookTimer = CTimer::GetTimeInMilliseconds() + 3000;
}

void
CPed::Chat(void)
{
	// We're already looking to our partner
	if (bIsLooking && TurnBody())
		ClearLookFlag();

	if (!m_pLookTarget || !m_pLookTarget->IsPed()) {
		ClearChat();
		return;
	}

	CPed *partner = (CPed*) m_pLookTarget;

	if (partner->m_nPedState != PED_CHAT) {
		ClearChat();
		if (partner->m_pedInObjective) {
			if (partner->m_objective == OBJECTIVE_KILL_CHAR_ON_FOOT ||
				partner->m_objective == OBJECTIVE_FLEE_CHAR_ON_FOOT_TILL_SAFE)
				ReactToAttack(partner->m_pedInObjective);
		}
		return;
	}
	if (bIsTalking) {
		if (CGeneral::GetRandomNumber() < 512) {
			CAnimBlendAssociation *chatAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_CHAT);
			if (chatAssoc) {
				chatAssoc->blendDelta = -4.0f;
				chatAssoc->flags |= ASSOC_DELETEFADEDOUT;
			}
			bIsTalking = false;
		} else
			Say(SOUND_PED_CHAT);

	} else {

		if (CGeneral::GetRandomNumber() < 20 && !RpAnimBlendClumpGetFirstAssociation(GetClump(), ASSOC_IDLE)) {
			CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_XPRESS_SCRATCH, 4.0f);
		}
		if (!bIsTalking && !RpAnimBlendClumpGetFirstAssociation(GetClump(), ASSOC_IDLE)) {
			CAnimBlendAssociation *chatAssoc = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_CHAT, 4.0f);
			float chatTime = CGeneral::GetRandomNumberInRange(0.0f, 3.0f);
			chatAssoc->SetCurrentTime(chatTime);

			bIsTalking = true;
			Say(SOUND_PED_CHAT);
		}
	}
	if (m_chatTimer && CTimer::GetTimeInMilliseconds() > m_chatTimer) {
		ClearChat();
		m_chatTimer = CTimer::GetTimeInMilliseconds() + 30000;
	}
}

void
CPed::ClearChat(void)
{
	CAnimBlendAssociation *animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_CHAT);
	if (animAssoc) {
		animAssoc->blendDelta = -8.0f;
		animAssoc->flags |= ASSOC_DELETEFADEDOUT;
	}
	bIsTalking = false;
	ClearLookFlag();
	RestorePreviousState();
}

#ifdef PEDS_REPORT_CRIMES_ON_PHONE
void
ReportPhonePickUpCB(CAnimBlendAssociation* assoc, void* arg)
{
	CPed* ped = (CPed*)arg;
	ped->m_nMoveState = PEDMOVE_STILL;
	CAnimManager::BlendAnimation(ped->GetClump(), ASSOCGRP_STD, ANIM_STD_IDLE, 8.0f);

	if (assoc->blendAmount > 0.5f && ped) {
		CAnimManager::BlendAnimation(ped->GetClump(), ASSOCGRP_STD, ANIM_STD_PHONE_TALK, 8.0f);
	}
}

void
ReportPhonePutDownCB(CAnimBlendAssociation* assoc, void* arg)
{
	assoc->flags |= ASSOC_DELETEFADEDOUT;
	assoc->blendDelta = -1000.0f;
	CPed* ped = (CPed*)arg;

	if (ped->m_phoneId != -1 && crimeReporters[ped->m_phoneId] == ped) {
		crimeReporters[ped->m_phoneId] = nil;
		gPhoneInfo.m_aPhones[ped->m_phoneId].m_nState = PHONE_STATE_FREE;
		ped->m_phoneId = -1;
	}

	if (assoc->blendAmount > 0.5f)
		ped->bUpdateAnimHeading = true;

	ped->SetWanderPath(CGeneral::GetRandomNumber() & 7);
}
#endif

bool
CPed::FacePhone(void)
{
	// This function was broken since it's left unused early in development.
#ifdef PEDS_REPORT_CRIMES_ON_PHONE
	float phoneDir = CGeneral::GetRadianAngleBetweenPoints(
		gPhoneInfo.m_aPhones[m_phoneId].m_vecPos.x, gPhoneInfo.m_aPhones[m_phoneId].m_vecPos.y,
		GetPosition().x, GetPosition().y);

	if (m_facePhoneStart) {
		m_lookTimer = 0;
		SetLookFlag(phoneDir, true);
		m_lookTimer = CTimer::GetTimeInMilliseconds() + 3000;
		m_facePhoneStart = false;
	}

	if (bIsLooking && TurnBody()) {
		ClearLookFlag();
		SetIdle();
		m_phoneTalkTimer = CTimer::GetTimeInMilliseconds() + 10000;
		CAnimBlendAssociation* assoc = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_PHONE_IN, 4.0f);
		assoc->SetFinishCallback(ReportPhonePickUpCB, this);
		return true;
	}

	return false;
#else
	float currentRot = RADTODEG(m_fRotationCur);
	float phoneDir = CGeneral::GetRadianAngleBetweenPoints(
		gPhoneInfo.m_aPhones[m_phoneId].m_vecPos.x,
		gPhoneInfo.m_aPhones[m_phoneId].m_vecPos.y,
		GetPosition().x,
		GetPosition().y);

	SetLookFlag(phoneDir, false);
	phoneDir = CGeneral::LimitAngle(phoneDir);
	m_moved = CVector2D(0.0f, 0.0f);

	if (currentRot - 180.0f > phoneDir)
		phoneDir += 2 * 180.0f;
	else if (180.0f + currentRot < phoneDir)
		phoneDir -= 2 * 180.0f;

	float neededTurn = currentRot - phoneDir;

	if (Abs(neededTurn) <= 0.75f) {
		SetIdle();
		ClearLookFlag();
		m_phoneTalkTimer = CTimer::GetTimeInMilliseconds() + 10000;
		return true;
	} else {
		m_fRotationCur = DEGTORAD(currentRot - neededTurn * 0.2f);
		return false;
	}
#endif
}
bool
CPed::MakePhonecall(void)
{
#ifdef PEDS_REPORT_CRIMES_ON_PHONE
	if (!IsPlayer() && CTimer::GetTimeInMilliseconds() > m_phoneTalkTimer - 7000 && bRunningToPhone) {

		FindPlayerPed()->m_pWanted->RegisterCrime_Immediately(m_crimeToReportOnPhone, GetPosition(),
			(m_crimeToReportOnPhone == CRIME_POSSESSION_GUN ? (uintptr)m_threatEntity : (uintptr)m_victimOfPlayerCrime), false);

		if (m_crimeToReportOnPhone != CRIME_POSSESSION_GUN)
			FindPlayerPed()->m_pWanted->SetWantedLevelNoDrop(1);

		bRunningToPhone = false;
	}
#endif
	if (CTimer::GetTimeInMilliseconds() <= m_phoneTalkTimer)
		return false;

#ifdef PEDS_REPORT_CRIMES_ON_PHONE
	CAnimBlendAssociation* talkAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_PHONE_TALK);
	if (talkAssoc && talkAssoc->blendAmount > 0.5f) {
		CAnimBlendAssociation* endAssoc = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_PHONE_OUT, 8.0f);
		endAssoc->flags &= ~ASSOC_DELETEFADEDOUT;
		endAssoc->SetFinishCallback(ReportPhonePutDownCB, this);
	}
#endif
	SetIdle();

	gPhoneInfo.m_aPhones[m_phoneId].m_nState = PHONE_STATE_FREE;
#ifndef PEDS_REPORT_CRIMES_ON_PHONE
	m_phoneId = -1;
#endif

	// Because SetWanderPath is now done async in ReportPhonePutDownCB
#ifdef PEDS_REPORT_CRIMES_ON_PHONE
	return false;
#else
	return true;
#endif
}

void
CPed::Teleport(CVector pos)
{
	CWorld::Remove(this);
	SetPosition(pos);
	bIsStanding = false;
	m_nPedStateTimer = 0;
	m_actionX = 0.0f;
	m_actionY = 0.0f;
	m_pDamageEntity = nil;
	CWorld::Add(this);
}

void
CPed::SetSeekCar(CVehicle *car, uint32 doorNode)
{
	if (m_nPedState == PED_SEEK_CAR)
		return;

#ifdef VC_PED_PORTS
	if (!CanSetPedState() || m_nPedState == PED_DRIVING)
		return;
#endif

	SetStoredState();
	m_pSeekTarget = car;
	m_pSeekTarget->RegisterReference((CEntity**) &m_pSeekTarget);
	m_carInObjective = car;
	m_carInObjective->RegisterReference((CEntity**) &m_carInObjective);
	m_pMyVehicle = car;
	m_pMyVehicle->RegisterReference((CEntity**) &m_pMyVehicle);
	// m_pSeekTarget->RegisterReference((CEntity**) &m_pSeekTarget);
	m_vehDoor = doorNode;
	m_distanceToCountSeekDone = 0.5f;
	SetPedState(PED_SEEK_CAR);

}

void
CPed::SeekCar(void)
{
	CVehicle *vehToSeek = m_carInObjective;
	CVector dest(0.0f, 0.0f, 0.0f);
	if (!vehToSeek) {
		RestorePreviousState();
		return;
	}

	if (m_objective != OBJECTIVE_ENTER_CAR_AS_PASSENGER) {
		if (m_vehDoor && m_objective != OBJECTIVE_ENTER_CAR_AS_DRIVER) {
			if (IsRoomToBeCarJacked()) {
				dest = GetPositionToOpenCarDoor(vehToSeek, m_vehDoor);
			} else if (m_nPedType == PEDTYPE_COP) {
				dest = GetPositionToOpenCarDoor(vehToSeek, CAR_DOOR_RF);
			} else {
				SetMoveState(PEDMOVE_STILL);
			}
		} else
			GetNearestDoor(vehToSeek, dest);
	} else {
		if (m_carJackTimer > CTimer::GetTimeInMilliseconds()) {
			SetMoveState(PEDMOVE_STILL);
			return;
		}
		if (vehToSeek->GetModelIndex() == MI_COACH) {
			GetNearestDoor(vehToSeek, dest);
		} else {
			if (vehToSeek->IsTrain()) {
				if (vehToSeek->GetStatus() != STATUS_TRAIN_NOT_MOVING) {
					RestorePreviousObjective();
					RestorePreviousState();
					return;
				}
				if (!GetNearestTrainDoor(vehToSeek, dest)) {
					RestorePreviousObjective();
					RestorePreviousState();
					return;
				}
			} else {
				if (!GetNearestPassengerDoor(vehToSeek, dest)) {
					if (vehToSeek->m_nNumPassengers == vehToSeek->m_nNumMaxPassengers) {
						RestorePreviousObjective();
						RestorePreviousState();
					} else {
						SetMoveState(PEDMOVE_STILL);
					}
					bVehEnterDoorIsBlocked = true;
					return;
				}
				bVehEnterDoorIsBlocked = false;
			}
		}
	}

	if (dest.x == 0.0f && dest.y == 0.0f) {
#ifdef FIX_BUGS
		if ((!IsPlayer() && CharCreatedBy != MISSION_CHAR) || vehToSeek->VehicleCreatedBy != MISSION_VEHICLE || vehToSeek->pDriver || !vehToSeek->CanPedOpenLocks(this)) {
#else
		if ((!IsPlayer() && CharCreatedBy != MISSION_CHAR) || vehToSeek->VehicleCreatedBy != MISSION_VEHICLE || vehToSeek->pDriver) {
#endif
			RestorePreviousState();
			if (IsPlayer()) {
				ClearObjective();
			} else if (CharCreatedBy == RANDOM_CHAR) {
				m_carJackTimer = CTimer::GetTimeInMilliseconds() + 30000;
			}
			SetMoveState(PEDMOVE_STILL);
			TheCamera.ClearPlayerWeaponMode();
			CCarCtrl::RemoveFromInterestingVehicleList(vehToSeek);
			return;
		}
		dest = vehToSeek->GetPosition();
		if (bCollidedWithMyVehicle) {
			WarpPedIntoCar(m_pMyVehicle);
			return;
		}
	}
	bool foundBetterPosToSeek = PossiblyFindBetterPosToSeekCar(&dest, vehToSeek);
	m_vecSeekPos = dest;
	float distToDestSqr = (m_vecSeekPos - GetPosition()).MagnitudeSqr();
#ifndef VC_PED_PORTS
	if (bIsRunning)
		SetMoveState(PEDMOVE_RUN);
#else
	if (bIsRunning ||
		vehToSeek->pDriver && distToDestSqr > sq(2.0f) && (Abs(vehToSeek->m_vecMoveSpeed.x) > 0.01f || Abs(vehToSeek->m_vecMoveSpeed.y) > 0.01f))
		SetMoveState(PEDMOVE_RUN);
#endif
	else if (distToDestSqr < sq(2.0f))
		SetMoveState(PEDMOVE_WALK);

	if (distToDestSqr >= 1.0f)
		bCanPedEnterSeekedCar = false;
	else if (2.0f * vehToSeek->GetColModel()->boundingBox.max.x > distToDestSqr)
		bCanPedEnterSeekedCar = true;

	if (vehToSeek->m_nGettingInFlags & GetCarDoorFlag(m_vehDoor))
		bVehEnterDoorIsBlocked = true;
	else
		bVehEnterDoorIsBlocked = false;

	// Arrived to the car
	if (Seek()) {
		if (!foundBetterPosToSeek) {
			if (1.5f + GetPosition().z > dest.z && GetPosition().z - 0.5f < dest.z) {
				if (vehToSeek->IsTrain()) {
					SetEnterTrain(vehToSeek, m_vehDoor);
				} else {
					m_fRotationCur = m_fRotationDest;
					if (!bVehEnterDoorIsBlocked) {
						vehToSeek->SetIsStatic(false);
						if (m_objective == OBJECTIVE_SOLICIT_VEHICLE) {
							SetSolicit(1000);
						} else if (m_objective == OBJECTIVE_BUY_ICE_CREAM) {
							SetBuyIceCream();
						} else if (vehToSeek->m_nNumGettingIn < vehToSeek->m_nNumMaxPassengers + 1
							&& vehToSeek->CanPedEnterCar()) {

							switch (vehToSeek->GetStatus()) {
								case STATUS_PLAYER:
								case STATUS_SIMPLE:
								case STATUS_PHYSICS:
								case STATUS_PLAYER_DISABLED:
									if (!vehToSeek->bIsBus && (!m_leader || m_leader != vehToSeek->pDriver) &&
										(m_vehDoor == CAR_DOOR_LF && vehToSeek->pDriver || m_vehDoor == CAR_DOOR_RF && vehToSeek->pPassengers[0] || m_vehDoor == CAR_DOOR_LR && vehToSeek->pPassengers[1] || m_vehDoor == CAR_DOOR_RR && vehToSeek->pPassengers[2])) {
										SetCarJack(vehToSeek);
										if (m_objective == OBJECTIVE_ENTER_CAR_AS_DRIVER && m_vehDoor != CAR_DOOR_LF)
											vehToSeek->pDriver->bFleeAfterExitingCar = true;
									} else {
										SetEnterCar(vehToSeek, m_vehDoor);
									}
									break;
								case STATUS_ABANDONED:
									if (m_vehDoor == CAR_DOOR_RF && vehToSeek->pPassengers[0]) {
										if (vehToSeek->pPassengers[0]->bDontDragMeOutCar) {
											if (IsPlayer())
												SetEnterCar(vehToSeek, m_vehDoor);
										} else {
											SetCarJack(vehToSeek);
										}
									} else {
										SetEnterCar(vehToSeek, m_vehDoor);
									}
									break;
								case STATUS_WRECKED:
									SetIdle();
									break;
								default:
									return;
							}
						} else {
							RestorePreviousState();
						}
					} else {
						SetMoveState(PEDMOVE_STILL);
					}
				}
			}
		}
	}
}

bool
CPed::CheckForExplosions(CVector2D &area)
{
	int event = 0;
	if (CEventList::FindClosestEvent(EVENT_EXPLOSION, GetPosition(), &event)) {
		area.x = gaEvent[event].posn.x;
		area.y = gaEvent[event].posn.y;
		CEntity *actualEntity = nil;

		switch (gaEvent[event].entityType) {
			case EVENT_ENTITY_PED:
				actualEntity = CPools::GetPed(gaEvent[event].entityRef);
				break;
			case EVENT_ENTITY_VEHICLE:
				actualEntity = CPools::GetVehicle(gaEvent[event].entityRef);
				break;
			case EVENT_ENTITY_OBJECT:
				actualEntity = CPools::GetObject(gaEvent[event].entityRef);
				break;
			default:
				break;
		}

		if (actualEntity) {
			m_pEventEntity = actualEntity;
			m_pEventEntity->RegisterReference((CEntity **) &m_pEventEntity);
			bGonnaInvestigateEvent = true;
		} else
			bGonnaInvestigateEvent = false;

		CEventList::ClearEvent(event);
		return true;
	} else if (CEventList::FindClosestEvent(EVENT_FIRE, GetPosition(), &event)) {
		area.x = gaEvent[event].posn.x;
		area.y = gaEvent[event].posn.y;
		CEventList::ClearEvent(event);
		bGonnaInvestigateEvent = false;
		return true;
	}

	bGonnaInvestigateEvent = false;
	return false;
}

CPed *
CPed::CheckForGunShots(void)
{
	int event;
	if (CEventList::FindClosestEvent(EVENT_GUNSHOT, GetPosition(), &event)) {
		if (gaEvent[event].entityType == EVENT_ENTITY_PED) {
			// Probably due to we don't want peds to go gunshot area? (same on VC)
			bGonnaInvestigateEvent = false;
			return CPools::GetPed(gaEvent[event].entityRef);
		}
	}
	bGonnaInvestigateEvent = false;
	return nil;
}

CPed *
CPed::CheckForDeadPeds(void)
{
	int event;
	if (CEventList::FindClosestEvent(EVENT_DEAD_PED, GetPosition(), &event)) {
		int pedHandle = gaEvent[event].entityRef;
		if (pedHandle && gaEvent[event].entityType == EVENT_ENTITY_PED) {
			bGonnaInvestigateEvent = true;
			return CPools::GetPed(pedHandle);
		}
	}
	bGonnaInvestigateEvent = false;
	return nil;
}

bool
CPed::IsPlayer(void) const
{
	return m_nPedType == PEDTYPE_PLAYER1 || m_nPedType == PEDTYPE_PLAYER2 ||
		m_nPedType == PEDTYPE_PLAYER3 || m_nPedType == PEDTYPE_PLAYER4;
}

bool
CPed::IsGangMember(void) const
{
	return m_nPedType >= PEDTYPE_GANG1 && m_nPedType <= PEDTYPE_GANG9;
}

bool
CPed::IsPointerValid(void)
{
	int pedIndex = CPools::GetPedPool()->GetIndex(this) >> 8;
	if (pedIndex < 0 || pedIndex >= NUMPEDS)
		return false;

	if (m_entryInfoList.first || FindPlayerPed() == this)
		return true;

	return false;
}

void
CPed::SetPedPositionInCar(void)
{
	if (CReplay::IsPlayingBack())
		return;

	if (bChangedSeat) {
		bool notYet = false;
		if (RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_CAR_GET_IN_LHS)
			|| RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_CAR_GET_IN_LO_LHS)
			|| RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_CAR_CLOSE_DOOR_LHS)
			|| RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_CAR_CLOSE_DOOR_LO_LHS)
			|| RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_CAR_SHUFFLE_RHS)
			|| RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_CAR_SHUFFLE_LO_RHS)
			|| RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_VAN_CLOSE_DOOR_REAR_LHS)
			|| RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_VAN_CLOSE_DOOR_REAR_RHS)
			|| RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_VAN_GET_IN_REAR_LHS)
			|| RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_VAN_GET_IN_REAR_RHS)
			|| RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_COACH_GET_IN_LHS)
			|| RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_COACH_GET_IN_RHS)) {
			notYet = true;
		}
		if (notYet) {
			LineUpPedWithCar(LINE_UP_TO_CAR_START);
			bChangedSeat = false;
			return;
		}
	}
	CVehicleModelInfo *vehModel = (CVehicleModelInfo *)CModelInfo::GetModelInfo(m_pMyVehicle->GetModelIndex());
	CMatrix newMat(m_pMyVehicle->GetMatrix());
	CVector seatPos;
	if (m_pMyVehicle->pDriver == this) {
		seatPos = vehModel->GetFrontSeatPosn();
		if (!m_pMyVehicle->IsBoat() && m_pMyVehicle->m_vehType != VEHICLE_TYPE_BIKE)
			seatPos.x = -seatPos.x;

	} else if (m_pMyVehicle->pPassengers[0] == this) {
		seatPos = vehModel->GetFrontSeatPosn();
	} else if (m_pMyVehicle->pPassengers[1] == this) {
		seatPos = vehModel->m_positions[CAR_POS_BACKSEAT];
		seatPos.x = -seatPos.x;
	} else {
		if (m_pMyVehicle->pPassengers[2] == this) {
			seatPos = vehModel->m_positions[CAR_POS_BACKSEAT];
		} else {
			seatPos = vehModel->GetFrontSeatPosn();
		}
	}
	newMat.GetPosition() += Multiply3x3(newMat, seatPos);
	// Already done below (SetTranslate(0.0f, 0.0f, 0.0f))
	// tempMat.SetUnity();

	// Rear seats on vans don't face to front, so rotate them HALFPI.
	if (m_pMyVehicle->bIsVan) {
		CMatrix tempMat;
		if (m_pMyVehicle->pPassengers[1] == this) {
			m_fRotationCur = m_pMyVehicle->GetForward().Heading() - HALFPI;
			tempMat.SetTranslate(0.0f, 0.0f, 0.0f);
			tempMat.RotateZ(-HALFPI);
			newMat = newMat * tempMat;
		} else if (m_pMyVehicle->pPassengers[2] == this) {
			m_fRotationCur = m_pMyVehicle->GetForward().Heading() + HALFPI;
			tempMat.SetTranslate(0.0f, 0.0f, 0.0f);
			tempMat.RotateZ(HALFPI);
			newMat = newMat * tempMat;
		} else {
			m_fRotationCur = m_pMyVehicle->GetForward().Heading();
		}
	} else {
		m_fRotationCur = m_pMyVehicle->GetForward().Heading();
	}
	GetMatrix() = newMat;
}

void
CPed::LookForSexyPeds(void)
{
	if ((!IsPedInControl() && m_nPedState != PED_DRIVING)
		|| m_lookTimer >= CTimer::GetTimeInMilliseconds() || m_nPedType != PEDTYPE_CIVMALE)
		return;

	for (int i = 0; i < m_numNearPeds; i++) {
		if (CanSeeEntity(m_nearPeds[i])) {
			if ((GetPosition() - m_nearPeds[i]->GetPosition()).Magnitude() < 10.0f) {
				CPed *nearPed = m_nearPeds[i];
				if ((nearPed->m_pedStats->m_sexiness > m_pedStats->m_sexiness)
					&& nearPed->m_nPedType == PEDTYPE_CIVFEMALE) {

					SetLookFlag(nearPed, true);
					m_lookTimer = CTimer::GetTimeInMilliseconds() + 4000;
					Say(SOUND_PED_CHAT_SEXY);
					return;
				}
			}
		}
	}
	m_lookTimer = CTimer::GetTimeInMilliseconds() + 10000;
}

void
CPed::LookForSexyCars(void)
{
	CEntity *vehicles[8];
	CVehicle *veh;
	int foundVehId = 0;
	int bestPriceYet = 0;
	int16 lastVehicle;

	if (!IsPedInControl() && m_nPedState != PED_DRIVING)
		return;

	if (m_lookTimer < CTimer::GetTimeInMilliseconds()) {
		CWorld::FindObjectsInRange(GetPosition(), 10.0f, true, &lastVehicle, 6, vehicles, false, true, false, false, false);

		for (int vehId = 0; vehId < lastVehicle; vehId++) {
			veh = (CVehicle*)vehicles[vehId];
			if (veh != m_pMyVehicle && bestPriceYet < veh->pHandling->nMonetaryValue) {
				foundVehId = vehId;
				bestPriceYet = veh->pHandling->nMonetaryValue;
			}
		}
		if (lastVehicle > 0 && bestPriceYet > 40000)
			SetLookFlag(vehicles[foundVehId], false);

		m_lookTimer = CTimer::GetTimeInMilliseconds() + 10000;
	}
}

bool
CPed::LookForInterestingNodes(void)
{
	CBaseModelInfo *model;
	CPtrNode *ptrNode;
	CVector effectDist;
	C2dEffect *effect;
	CMatrix *objMat;

	if ((CTimer::GetFrameCounter() + (m_randomSeed % 256)) & 7 || CTimer::GetTimeInMilliseconds() <= m_chatTimer) {
		return false;
	}
	bool found = false;
	uint8 randVal = CGeneral::GetRandomNumber() % 256;

	int minX = CWorld::GetSectorIndexX(GetPosition().x - CHECK_NEARBY_THINGS_MAX_DIST);
	if (minX < 0) minX = 0;
	int minY = CWorld::GetSectorIndexY(GetPosition().y - CHECK_NEARBY_THINGS_MAX_DIST);
	if (minY < 0) minY = 0;
	int maxX = CWorld::GetSectorIndexX(GetPosition().x + CHECK_NEARBY_THINGS_MAX_DIST);
#ifdef FIX_BUGS
	if (maxX >= NUMSECTORS_X) maxX = NUMSECTORS_X - 1;
#else
	if (maxX >= NUMSECTORS_X) maxX = NUMSECTORS_X;
#endif

	int maxY = CWorld::GetSectorIndexY(GetPosition().y + CHECK_NEARBY_THINGS_MAX_DIST);
#ifdef FIX_BUGS
	if (maxY >= NUMSECTORS_Y) maxY = NUMSECTORS_Y - 1;
#else
	if (maxY >= NUMSECTORS_Y) maxY = NUMSECTORS_Y;
#endif

	for (int curY = minY; curY <= maxY && !found; curY++) {
		for (int curX = minX; curX <= maxX && !found; curX++) {
			CSector *sector = CWorld::GetSector(curX, curY);

			for (ptrNode = sector->m_lists[ENTITYLIST_VEHICLES].first; ptrNode && !found; ptrNode = ptrNode->next) {
				CVehicle *veh = (CVehicle*)ptrNode->item;
				model = veh->GetModelInfo();
				if (model->GetNum2dEffects() != 0) {
					for (int e = 0; e < model->GetNum2dEffects(); e++) {
						effect = model->Get2dEffect(e);
						if (effect->type == EFFECT_ATTRACTOR && effect->attractor.probability >= randVal) {
							objMat = &veh->GetMatrix();
							CVector effectPos = veh->GetMatrix() * effect->pos;
							effectDist = effectPos - GetPosition();
							if (effectDist.MagnitudeSqr() < sq(8.0f)) {
								found = true;
								break;
							}
						}
					}
				}
			}
			for (ptrNode = sector->m_lists[ENTITYLIST_OBJECTS].first; ptrNode && !found; ptrNode = ptrNode->next) {
				CObject *obj = (CObject*)ptrNode->item;
				model = CModelInfo::GetModelInfo(obj->GetModelIndex());
				if (model->GetNum2dEffects() != 0) {
					for (int e = 0; e < model->GetNum2dEffects(); e++) {
						effect = model->Get2dEffect(e);
						if (effect->type == EFFECT_ATTRACTOR && effect->attractor.probability >= randVal) {
							objMat = &obj->GetMatrix();
							CVector effectPos = obj->GetMatrix() * effect->pos;
							effectDist = effectPos - GetPosition();
							if (effectDist.MagnitudeSqr() < sq(8.0f)) {
								found = true;
								break;
							}
						}
					}
				}
			}
			for (ptrNode = sector->m_lists[ENTITYLIST_BUILDINGS].first; ptrNode && !found; ptrNode = ptrNode->next) {
				CBuilding *building = (CBuilding*)ptrNode->item;
				model = CModelInfo::GetModelInfo(building->GetModelIndex());
				if (model->GetNum2dEffects() != 0) {
					for (int e = 0; e < model->GetNum2dEffects(); e++) {
						effect = model->Get2dEffect(e);
						if (effect->type == EFFECT_ATTRACTOR && effect->attractor.probability >= randVal) {
							objMat = &building->GetMatrix();
							CVector effectPos = building->GetMatrix() * effect->pos;
							effectDist = effectPos - GetPosition();
							if (effectDist.MagnitudeSqr() < sq(8.0f)) {
								found = true;
								break;
							}
						}
					}
				}
			}
			for (ptrNode = sector->m_lists[ENTITYLIST_BUILDINGS_OVERLAP].first; ptrNode && !found; ptrNode = ptrNode->next) {
				CBuilding *building = (CBuilding*)ptrNode->item;
				model = CModelInfo::GetModelInfo(building->GetModelIndex());
				if (model->GetNum2dEffects() != 0) {
					for (int e = 0; e < model->GetNum2dEffects(); e++) {
						effect = model->Get2dEffect(e);
						if (effect->type == EFFECT_ATTRACTOR && effect->attractor.probability >= randVal) {
							objMat = &building->GetMatrix();
							CVector effectPos = building->GetMatrix() * effect->pos;
							effectDist = effectPos - GetPosition();
							if (effectDist.MagnitudeSqr() < sq(8.0f)) {
								found = true;
								break;
							}
						}
					}
				}
			}
		}
	}

	if (!found)
		return false;

	CVector effectFrontLocal = Multiply3x3(*objMat, effect->attractor.dir);
	float angleToFace = CGeneral::GetRadianAngleBetweenPoints(effectFrontLocal.x, effectFrontLocal.y, 0.0f, 0.0f);
	randVal = CGeneral::GetRandomNumber() % 256;
	if (randVal <= m_randomSeed % 256) {
		m_chatTimer = CTimer::GetTimeInMilliseconds() + 2000;
		SetLookFlag(angleToFace, true);
		SetLookTimer(1000);
		return false;
	}

	CVector2D effectPos = *objMat * effect->pos;
	switch (effect->attractor.type) {
		case ATTRACTORTYPE_ICECREAM:
			SetInvestigateEvent(EVENT_ICECREAM, effectPos, 0.1f, 15000, angleToFace);
			break;
		case ATTRACTORTYPE_STARE:
			SetInvestigateEvent(EVENT_SHOPSTALL, effectPos, 1.0f,
								CGeneral::GetRandomNumberInRange(8000, 10 * effect->attractor.probability + 8500),
								angleToFace);
			break;
		default:
			return true;
	}
	return true;
}

void
CPed::SetWaitState(eWaitState state, void *time)
{
	AnimationId waitAnim = ANIM_STD_NUM;
	CAnimBlendAssociation *animAssoc;

	if (!IsPedInControl())
		return;

	if (state != m_nWaitState)
		FinishedWaitCB(nil, this);

	switch (state) {
		case WAITSTATE_TRAFFIC_LIGHTS:
			m_nWaitTimer = CTimer::GetTimeInMilliseconds() + 500;
			SetMoveState(PEDMOVE_STILL);
			break;
		case WAITSTATE_CROSS_ROAD:
			m_nWaitTimer = CTimer::GetTimeInMilliseconds() + 1000;
			CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_IDLE_HBHB, 4.0f);
			break;
		case WAITSTATE_CROSS_ROAD_LOOK:
			CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_ROADCROSS, 8.0f);

			if (time)
				m_nWaitTimer = CTimer::GetTimeInMilliseconds() + *(int*)time;
			else
				m_nWaitTimer = CTimer::GetTimeInMilliseconds() + CGeneral::GetRandomNumberInRange(2000,5000);

			break;
		case WAITSTATE_LOOK_PED:
		case WAITSTATE_LOOK_SHOP:
		case WAITSTATE_LOOK_ACCIDENT:
		case WAITSTATE_FACEOFF_GANG:
			break;
		case WAITSTATE_DOUBLEBACK:
			m_headingRate = 0.0f;
			m_nWaitTimer = CTimer::GetTimeInMilliseconds() + 3500;
			animAssoc = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_IDLE_HBHB, 4.0f);
#ifdef FIX_BUGS
			animAssoc->SetFinishCallback(RestoreHeadingRateCB, this);
#endif
			break;
		case WAITSTATE_HITWALL:
			m_headingRate = 2.0f;
			m_nWaitTimer = CTimer::GetTimeInMilliseconds() + 5000;
			animAssoc = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_HIT_WALL, 16.0f);
			animAssoc->flags |= ASSOC_DELETEFADEDOUT;
			animAssoc->flags |= ASSOC_FADEOUTWHENDONE;
			animAssoc->SetDeleteCallback(FinishedWaitCB, this);

			if (m_objective == OBJECTIVE_ENTER_CAR_AS_PASSENGER && CharCreatedBy == RANDOM_CHAR && m_nPedState == PED_SEEK_CAR) {
				ClearObjective();
				RestorePreviousState();
				m_carJackTimer = CTimer::GetTimeInMilliseconds() + 30000;
			}
			break;
		case WAITSTATE_TURN180:
			m_headingRate = 0.0f;
			m_nWaitTimer = CTimer::GetTimeInMilliseconds() + 5000;
			animAssoc = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_TURN180, 4.0f);
			animAssoc->SetFinishCallback(FinishedWaitCB, this);
			animAssoc->SetDeleteCallback(RestoreHeadingRateCB, this);
			break;
		case WAITSTATE_SURPRISE:
			m_headingRate = 0.0f;
			m_nWaitTimer = CTimer::GetTimeInMilliseconds() + 2000;
			animAssoc = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_HIT_WALL, 4.0f);
			animAssoc->SetFinishCallback(FinishedWaitCB, this);
			break;
		case WAITSTATE_STUCK:
			SetMoveState(PEDMOVE_STILL);
			SetMoveAnim();
			m_headingRate = 0.0f;
			m_nWaitTimer = CTimer::GetTimeInMilliseconds() + 5000;
			animAssoc = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_IDLE_TIRED, 4.0f);
#ifdef FIX_BUGS
			animAssoc->SetFinishCallback(RestoreHeadingRateCB, this);
#endif

			if (m_objective == OBJECTIVE_ENTER_CAR_AS_PASSENGER && CharCreatedBy == RANDOM_CHAR && m_nPedState == PED_SEEK_CAR) {
				ClearObjective();
				RestorePreviousState();
				m_carJackTimer = CTimer::GetTimeInMilliseconds() + 30000;
			}
			break;
		case WAITSTATE_LOOK_ABOUT:
			SetMoveState(PEDMOVE_STILL);
			SetMoveAnim();
			m_headingRate = 0.0f;
			m_nWaitTimer = CTimer::GetTimeInMilliseconds() + 5000;
			animAssoc = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_IDLE_HBHB, 4.0f);
#ifdef FIX_BUGS
			animAssoc->SetFinishCallback(RestoreHeadingRateCB, this);
#endif

			break;
		case WAITSTATE_PLAYANIM_COWER:
			waitAnim = ANIM_STD_HANDSCOWER;
		case WAITSTATE_PLAYANIM_HANDSUP:
			if (waitAnim == ANIM_STD_NUM)
				waitAnim = ANIM_STD_HANDSUP;
		case WAITSTATE_PLAYANIM_HANDSCOWER:
			if (waitAnim == ANIM_STD_NUM)
				waitAnim = ANIM_STD_HANDSCOWER;
			m_headingRate = 0.0f;
			if (time)
				m_nWaitTimer = CTimer::GetTimeInMilliseconds() + *(int*)time;
			else
				m_nWaitTimer = CTimer::GetTimeInMilliseconds() + 3000;

			animAssoc = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, waitAnim, 4.0f);
			animAssoc->SetDeleteCallback(FinishedWaitCB, this);
			break;
		case WAITSTATE_PLAYANIM_DUCK:
			waitAnim = ANIM_STD_DUCK_DOWN;
		case WAITSTATE_PLAYANIM_TAXI:
			if (waitAnim == ANIM_STD_NUM)
				waitAnim = ANIM_STD_HAILTAXI;
		case WAITSTATE_PLAYANIM_CHAT:
			if (waitAnim == ANIM_STD_NUM)
				waitAnim = ANIM_STD_CHAT;
			if (time)
				m_nWaitTimer = CTimer::GetTimeInMilliseconds() + *(int*)time;
			else
				m_nWaitTimer = CTimer::GetTimeInMilliseconds() + 3000;

			animAssoc = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, waitAnim, 4.0f);
			animAssoc->flags &= ~ASSOC_FADEOUTWHENDONE;
			animAssoc->flags |= ASSOC_DELETEFADEDOUT;
			animAssoc->SetDeleteCallback(FinishedWaitCB, this);
			break;
		case WAITSTATE_FINISH_FLEE:
			SetMoveState(PEDMOVE_STILL);
			SetMoveAnim();
			m_headingRate = 0.0f;
			m_nWaitTimer = CTimer::GetTimeInMilliseconds() + 2500;
			animAssoc = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_IDLE_TIRED, 4.0f);
#ifdef FIX_BUGS
			animAssoc->SetFinishCallback(RestoreHeadingRateCB, this);
#endif
			break;
		default:
			m_nWaitState = WAITSTATE_FALSE;
			RestoreHeadingRate();
			return;
	}
	m_nWaitState = state;
}

void
CPed::Wait(void)
{
	AnimationId mustHaveAnim = ANIM_STD_NUM;
	CAnimBlendAssociation *animAssoc;
	CPed *pedWeLook;

	if (DyingOrDead()) {
		m_nWaitState = WAITSTATE_FALSE;
		RestoreHeadingRate();
		return;
	}

	switch (m_nWaitState) {

		case WAITSTATE_TRAFFIC_LIGHTS:
			if (CTimer::GetTimeInMilliseconds() > m_nWaitTimer) {
				if (CTrafficLights::LightForPeds() == PED_LIGHTS_WALK) {
					m_nWaitState = WAITSTATE_FALSE;
					SetMoveState(PEDMOVE_WALK);
				}
			}
			break;

		case WAITSTATE_CROSS_ROAD:
			if (CTimer::GetTimeInMilliseconds() > m_nWaitTimer) {
				if (CGeneral::GetRandomNumber() & 1 || !m_nWaitTimer)
					m_nWaitState = WAITSTATE_FALSE;
				else
					SetWaitState(WAITSTATE_CROSS_ROAD_LOOK, nil);

				animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_IDLE_HBHB);
				if (animAssoc) {
					animAssoc->blendDelta = -8.0f;
					animAssoc->flags |= ASSOC_DELETEFADEDOUT;
				}
			}
			break;

		case WAITSTATE_CROSS_ROAD_LOOK:
			if (CTimer::GetTimeInMilliseconds() > m_nWaitTimer) {
				m_nWaitState = WAITSTATE_FALSE;
				animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_ROADCROSS);
				if (animAssoc) {
					animAssoc->blendDelta = -8.0f;
					animAssoc->flags |= ASSOC_DELETEFADEDOUT;
				}
			}
			break;

		case WAITSTATE_DOUBLEBACK:
			if (CTimer::GetTimeInMilliseconds() <= m_nWaitTimer) {
				uint32 timeLeft = m_nWaitTimer - CTimer::GetTimeInMilliseconds();
				if (timeLeft < 2500 && timeLeft > 2000) {
					m_nWaitTimer -= 500;
					CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_XPRESS_SCRATCH, 4.0f);
				}
			} else {
				m_nWaitState = WAITSTATE_FALSE;
				SetMoveState(PEDMOVE_WALK);
			}
			break;

		case WAITSTATE_HITWALL:
			if (CTimer::GetTimeInMilliseconds() <= m_nWaitTimer) {
				if (m_collidingThingTimer > CTimer::GetTimeInMilliseconds()) {
					m_collidingThingTimer = CTimer::GetTimeInMilliseconds() + 2500;
				}
			} else {
				m_nWaitState = WAITSTATE_FALSE;
			}
			break;

		case WAITSTATE_TURN180:
			if (CTimer::GetTimeInMilliseconds() > m_nWaitTimer) {
				m_nWaitState = WAITSTATE_FALSE;
				SetMoveState(PEDMOVE_WALK);
				m_fRotationCur = m_fRotationCur + PI;
				if (m_nPedState == PED_INVESTIGATE)
					ClearInvestigateEvent();
			}

			if (m_collidingThingTimer > CTimer::GetTimeInMilliseconds()) {
				m_collidingThingTimer = CTimer::GetTimeInMilliseconds() + 2500;
			}
			break;

		case WAITSTATE_SURPRISE:
			if (CTimer::GetTimeInMilliseconds() > m_nWaitTimer) {
				if (RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_HIT_WALL)) {
					animAssoc = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_XPRESS_SCRATCH, 4.0f);
					animAssoc->SetFinishCallback(FinishedWaitCB, this);
					m_nWaitTimer = CTimer::GetTimeInMilliseconds() + 5000;
				} else {
					m_nWaitState = WAITSTATE_FALSE;
				}
			}
			break;

		case WAITSTATE_STUCK:
			if (CTimer::GetTimeInMilliseconds() <= m_nWaitTimer)
				break;

			animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_IDLE_TIRED);

			if (!animAssoc)
				animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_TURN180);
			if (!animAssoc)
				animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_XPRESS_SCRATCH);
			if (!animAssoc)
				animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_ROADCROSS);

			if (animAssoc) {
				if (animAssoc->IsPartial()) {
					animAssoc->blendDelta = -8.0f;
					animAssoc->flags |= ASSOC_DELETEFADEDOUT;
				} else {
					animAssoc->flags |= ASSOC_DELETEFADEDOUT;
					CAnimManager::BlendAnimation(GetClump(), m_animGroup, ANIM_STD_IDLE, 4.0f);
				}

				if (animAssoc->animId == ANIM_STD_TURN180) {
					m_fRotationCur = CGeneral::LimitRadianAngle(PI + m_fRotationCur);
					m_nWaitState = WAITSTATE_FALSE;
					SetMoveState(PEDMOVE_WALK);
					m_nStoredMoveState = PEDMOVE_NONE;
					m_panicCounter = 0;
					return;
				}
			}

			AnimationId animToPlay;

			switch (CGeneral::GetRandomNumber() & 3) {
				case 0:
					animToPlay = ANIM_STD_ROADCROSS;
					break;
				case 1:
					animToPlay = ANIM_STD_IDLE_TIRED;
					break;
				case 2:
					animToPlay = ANIM_STD_XPRESS_SCRATCH;
					break;
				case 3:
					animToPlay = ANIM_STD_TURN180;
					break;
				default:
					break;
			}

			animAssoc = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, animToPlay, 4.0f);

			if (animToPlay == ANIM_STD_TURN180)
				animAssoc->SetFinishCallback(FinishedWaitCB, this);

			m_nWaitTimer = CTimer::GetTimeInMilliseconds() + CGeneral::GetRandomNumberInRange(1500, 5000);
			break;

		case WAITSTATE_LOOK_ABOUT:
			if (CTimer::GetTimeInMilliseconds() > m_nWaitTimer) {
				m_nWaitState = WAITSTATE_FALSE;
				animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_IDLE_HBHB);
				if (animAssoc) {
					animAssoc->blendDelta = -8.0f;
					animAssoc->flags |= ASSOC_DELETEFADEDOUT;
				}
			}
			break;

		case WAITSTATE_PLAYANIM_HANDSUP:
			mustHaveAnim = ANIM_STD_HANDSUP;
			
		case WAITSTATE_PLAYANIM_HANDSCOWER:
			if (mustHaveAnim == ANIM_STD_NUM)
				mustHaveAnim = ANIM_STD_HANDSCOWER;

			animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), mustHaveAnim);
			pedWeLook = (CPed*) m_pLookTarget;

			if ((!m_pLookTarget || !m_pLookTarget->IsPed() || pedWeLook->m_pPointGunAt)
				&& m_nPedState != PED_FLEE_ENTITY
				&& m_nPedState != PED_ATTACK
				&& CTimer::GetTimeInMilliseconds() <= m_nWaitTimer
				&& animAssoc) {

				TurnBody();
			} else {
				m_nWaitState = WAITSTATE_FALSE;
				m_nWaitTimer = 0;
				if (m_pLookTarget && m_pLookTarget->IsPed()) {

					if (m_nPedState != PED_FLEE_ENTITY && m_nPedState != PED_ATTACK) {

						if (m_pedStats->m_fear <= 100 - pedWeLook->m_pedStats->m_temper) {

							if (GetWeapon()->IsTypeMelee()) {
#ifdef VC_PED_PORTS
								if(m_pedStats->m_flags & STAT_GUN_PANIC) {
#endif
								SetObjective(OBJECTIVE_FLEE_CHAR_ON_FOOT_TILL_SAFE, m_pLookTarget);
								if (m_nPedState == PED_FLEE_ENTITY || m_nPedState == PED_FLEE_POS) {

									bUsePedNodeSeek = true;
									m_pNextPathNode = nil;
								}
								if (m_nMoveState != PEDMOVE_RUN)
									SetMoveState(PEDMOVE_WALK);

								if (m_nPedType != PEDTYPE_COP) {
									ProcessObjective();
									SetMoveState(PEDMOVE_WALK);
								}
#ifdef VC_PED_PORTS
								} else {
									SetObjective(OBJECTIVE_NONE);
									SetWanderPath(CGeneral::GetRandomNumberInRange(0.0f, 8.0f));
								}
#endif
							} else {
								SetObjective(OBJECTIVE_KILL_CHAR_ON_FOOT, m_pLookTarget);
								SetObjectiveTimer(20000);
							}
						} else {
							SetObjective(OBJECTIVE_FLEE_CHAR_ON_FOOT_TILL_SAFE, m_pLookTarget);
							if (m_nPedState == PED_FLEE_ENTITY || m_nPedState == PED_FLEE_POS)
							{
								bUsePedNodeSeek = true;
								m_pNextPathNode = nil;
							}
							SetMoveState(PEDMOVE_RUN);
							Say(SOUND_PED_FLEE_RUN);
						}
					}
				}
				animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), mustHaveAnim);
				if (animAssoc) {
					animAssoc->blendDelta = -4.0f;
					animAssoc->flags |= ASSOC_DELETEFADEDOUT;
				}
			}
			break;
		case WAITSTATE_PLAYANIM_COWER:
			mustHaveAnim = ANIM_STD_HANDSCOWER;

		case WAITSTATE_PLAYANIM_DUCK:
			if (mustHaveAnim == ANIM_STD_NUM)
				mustHaveAnim = ANIM_STD_DUCK_DOWN;

		case WAITSTATE_PLAYANIM_TAXI:
			if (mustHaveAnim == ANIM_STD_NUM)
				mustHaveAnim = ANIM_STD_HAILTAXI;

		case WAITSTATE_PLAYANIM_CHAT:
			if (mustHaveAnim == ANIM_STD_NUM)
				mustHaveAnim = ANIM_STD_CHAT;

			if (CTimer::GetTimeInMilliseconds() > m_nWaitTimer) {
				animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), mustHaveAnim);
				if (animAssoc) {
					animAssoc->blendDelta = -4.0f;
					animAssoc->flags |= ASSOC_DELETEFADEDOUT;
				}
				m_nWaitState = WAITSTATE_FALSE;
			}
#ifdef VC_PED_PORTS
			else if (m_nWaitState == WAITSTATE_PLAYANIM_TAXI) {
				if (m_pedInObjective) {
					if (m_objective == OBJECTIVE_GOTO_CHAR_ON_FOOT || m_objective == OBJECTIVE_KILL_CHAR_ON_FOOT) {
						
						// VC also calls CleanUpOldReference here for old LookTarget.
						m_pLookTarget = m_pedInObjective;
						m_pLookTarget->RegisterReference((CEntity **) &m_pLookTarget);
						TurnBody();
					}
				}
			}
#endif
			break;

		case WAITSTATE_FINISH_FLEE:
			animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_IDLE_TIRED);
			if (animAssoc) {
				if (CTimer::GetTimeInMilliseconds() > m_nWaitTimer) {
					animAssoc->flags |= ASSOC_DELETEFADEDOUT;
					CAnimManager::BlendAnimation(GetClump(), m_animGroup, ANIM_STD_IDLE, 4.0f);
					int timer = 2000;
					m_nWaitState = WAITSTATE_FALSE;
					SetWaitState(WAITSTATE_CROSS_ROAD_LOOK, &timer);
				}
			} else {
				m_nWaitState = WAITSTATE_FALSE;
			}
			break;
		default:
			break;
	}

	if(!m_nWaitState)
		RestoreHeadingRate();
}

void
CPed::FinishedWaitCB(CAnimBlendAssociation *animAssoc, void *arg)
{
	CPed *ped = (CPed*)arg;

	ped->m_nWaitTimer = 0;
	ped->RestoreHeadingRate();
	ped->Wait();
}

void
CPed::RestoreHeadingRate(void)
{
	m_headingRate = m_pedStats->m_headingChangeRate;
}

void
CPed::RestoreHeadingRateCB(CAnimBlendAssociation *assoc, void *arg)
{
	((CPed*)arg)->m_headingRate = ((CPed*)arg)->m_pedStats->m_headingChangeRate;
}

void
CPed::FlagToDestroyWhenNextProcessed(void)
{
	bRemoveFromWorld = true;
	if (!InVehicle())
		return;
	if (m_pMyVehicle->pDriver == this){
		m_pMyVehicle->pDriver = nil;
		if (IsPlayer() && m_pMyVehicle->GetStatus() != STATUS_WRECKED)
			m_pMyVehicle->SetStatus(STATUS_ABANDONED);
	}else{
		m_pMyVehicle->RemovePassenger(this);
	}
	bInVehicle = false;
	m_pMyVehicle = nil;

	if (CharCreatedBy == MISSION_CHAR)
		SetPedState(PED_DEAD);
	else
		SetPedState(PED_NONE);
	m_pVehicleAnim = nil;
}

void
CPed::SetSolicit(uint32 time)
{
	if (m_nPedState == PED_SOLICIT || !IsPedInControl() || !m_carInObjective)
		return;

	if (CharCreatedBy != MISSION_CHAR && m_carInObjective->m_nNumGettingIn == 0
		&& CTimer::GetTimeInMilliseconds() < m_objectiveTimer) {
		if (m_vehDoor == CAR_DOOR_LF) {
			m_fRotationDest = m_carInObjective->GetForward().Heading() - HALFPI;
		} else {
			m_fRotationDest = m_carInObjective->GetForward().Heading() + HALFPI;
		}

		if (Abs(m_fRotationDest - m_fRotationCur) < HALFPI) {
			m_chatTimer = CTimer::GetTimeInMilliseconds() + time;

			if(!m_carInObjective->bIsVan && !m_carInObjective->bIsBus)
				m_pVehicleAnim = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_CAR_HOOKERTALK, 4.0f);

			SetPedState(PED_SOLICIT);
		}
	}
}

void
CPed::Solicit(void)
{
	if (m_chatTimer >= CTimer::GetTimeInMilliseconds() && m_carInObjective) {
		CVector doorPos = GetPositionToOpenCarDoor(m_carInObjective, m_vehDoor, 0.0f);
		SetMoveState(PEDMOVE_STILL);

		// Game uses GetAngleBetweenPoints and converts it to radian
		m_fRotationDest = CGeneral::GetRadianAngleBetweenPoints(
			doorPos.x, doorPos.y,
			GetPosition().x, GetPosition().y);

		if (m_fRotationDest < 0.0f) {
			m_fRotationDest = m_fRotationDest + TWOPI;
		} else if (m_fRotationDest > TWOPI) {
			m_fRotationDest = m_fRotationDest - TWOPI;
		}

		if ((GetPosition() - doorPos).MagnitudeSqr() <= 1.0f)
			return;
		CAnimBlendAssociation *talkAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_CAR_HOOKERTALK);
		if (talkAssoc) {
			talkAssoc->blendDelta = -1000.0f;
			talkAssoc->flags |= ASSOC_DELETEFADEDOUT;
		}
		RestorePreviousState();
		RestorePreviousObjective();
		SetObjectiveTimer(10000);
	} else if (!m_carInObjective) {
		RestorePreviousState();
		RestorePreviousObjective();
		SetObjectiveTimer(10000);
	} else if (CWorld::Players[CWorld::PlayerInFocus].m_nMoney <= 100) {
		m_carInObjective = nil;
	} else {
		m_pVehicleAnim = nil;
		SetLeader(m_carInObjective->pDriver);
	}
}

void
CPed::SetBuyIceCream(void)
{
	if (m_nPedState == PED_BUY_ICECREAM || !IsPedInControl())
		return;

	if (!m_carInObjective)
		return;

#ifdef FIX_ICECREAM

	// Simulating BuyIceCream
	CPed* driver = m_carInObjective->pDriver;
	if (driver) {
		SetPedState(PED_BUY_ICECREAM);
		bFindNewNodeAfterStateRestore = true;
		SetObjectiveTimer(8000);
		SetChat(driver, 8000);
		driver->SetChat(this, 8000);
		return;
	}
#endif

	// Side of the Ice Cream van
	m_fRotationDest = m_carInObjective->GetForward().Heading() - HALFPI;

	if (Abs(m_fRotationDest - m_fRotationCur) < HALFPI) {
		m_chatTimer = CTimer::GetTimeInMilliseconds() + 3000;
		SetPedState(PED_BUY_ICECREAM);
	}
}

bool
CPed::PossiblyFindBetterPosToSeekCar(CVector *pos, CVehicle *veh)
{
	bool foundIt = false;

	CVector helperPos = GetPosition();
	helperPos.z = pos->z - 0.5f;

	CVector foundPos = *pos;
	foundPos.z -= 0.5f;

	// If there is another car between target car and us.
	if (CWorld::TestSphereAgainstWorld((foundPos + helperPos) / 2.0f, 0.25f, veh, false, true, false, false, false, false)) {

		CColModel *vehCol = veh->GetModelInfo()->GetColModel();
		CVector *colMin = &vehCol->boundingBox.min;
		CVector *colMax = &vehCol->boundingBox.max;

		CVector leftRearPos = CVector(colMin->x - 0.5f, colMin->y - 0.5f, 0.0f);
		CVector rightRearPos = CVector(0.5f + colMax->x, colMin->y - 0.5f, 0.0f);
		CVector leftFrontPos = CVector(colMin->x - 0.5f, 0.5f + colMax->y, 0.0f);
		CVector rightFrontPos = CVector(0.5f + colMax->x, 0.5f + colMax->y, 0.0f);

		leftRearPos = veh->GetMatrix() * leftRearPos;
		rightRearPos = veh->GetMatrix() * rightRearPos;
		leftFrontPos = veh->GetMatrix() * leftFrontPos;
		rightFrontPos = veh->GetMatrix() * rightFrontPos;

		// Makes helperPos veh-ped distance vector.
		helperPos -= veh->GetPosition();

		// ?!? I think it's absurd to use this unless another function like SeekCar finds next pos. with it and we're trying to simulate it's behaviour.
		// On every run it returns another pos. for ped, with same distance to the veh.
		// Sequence of positions are not guaranteed, it depends on global pos. (So sometimes it returns positions to make ped draw circle, sometimes don't)
		helperPos = veh->GetMatrix() * helperPos;

		float vehForwardHeading = veh->GetForward().Heading();

		// I'm absolutely not sure about these namings.
		// NTVF = needed turn if we're looking to vehicle front and wanna look to...

		float potentialLrHeading = Atan2(leftRearPos.x - helperPos.x, leftRearPos.y - helperPos.y);
		float NTVF_LR = CGeneral::LimitRadianAngle(potentialLrHeading - vehForwardHeading);

		float potentialRrHeading = Atan2(rightRearPos.x - helperPos.x, rightRearPos.y - helperPos.y);
		float NTVF_RR = CGeneral::LimitRadianAngle(potentialRrHeading - vehForwardHeading);

		float potentialLfHeading = Atan2(leftFrontPos.x - helperPos.x, leftFrontPos.y - helperPos.y);
		float NTVF_LF = CGeneral::LimitRadianAngle(potentialLfHeading - vehForwardHeading);

		float potentialRfHeading = Atan2(rightFrontPos.x - helperPos.x, rightFrontPos.y - helperPos.y);
		float NTVF_RF = CGeneral::LimitRadianAngle(potentialRfHeading - vehForwardHeading);

		bool canHeadToLr = NTVF_LR <= -PI || NTVF_LR >= -HALFPI;

		bool canHeadToRr = NTVF_RR <= HALFPI || NTVF_RR >= PI;

		bool canHeadToLf = NTVF_LF >= 0.0f || NTVF_LF <= -HALFPI;

		bool canHeadToRf = NTVF_RF <= 0.0f || NTVF_RF >= HALFPI;

		// Only order of conditions are different among enterTypes.
		if (m_vehDoor == CAR_DOOR_RR) {
			if (canHeadToRr) {
				foundPos = rightRearPos;
				foundIt = true;
			} else if (canHeadToRf) {
				foundPos = rightFrontPos;
				foundIt = true;
			} else if (canHeadToLr) {
				foundPos = leftRearPos;
				foundIt = true;
			} else if (canHeadToLf) {
				foundPos = leftFrontPos;
				foundIt = true;
			}
		} else if(m_vehDoor == CAR_DOOR_RF) {
			if (canHeadToRf) {
				foundPos = rightFrontPos;
				foundIt = true;
			} else if (canHeadToRr) {
				foundPos = rightRearPos;
				foundIt = true;
			} else if (canHeadToLf) {
				foundPos = leftFrontPos;
				foundIt = true;
			} else if (canHeadToLr) {
				foundPos = leftRearPos;
				foundIt = true;
			}
		} else if (m_vehDoor == CAR_DOOR_LF) {
			if (canHeadToLf) {
				foundPos = leftFrontPos;
				foundIt = true;
			} else if (canHeadToLr) {
				foundPos = leftRearPos;
				foundIt = true;
			} else if (canHeadToRf) {
				foundPos = rightFrontPos;
				foundIt = true;
			} else if (canHeadToRr) {
				foundPos = rightRearPos;
				foundIt = true;
			}
		} else if (m_vehDoor == CAR_DOOR_LR) {
			if (canHeadToLr) {
				foundPos = leftRearPos;
				foundIt = true;
			} else if (canHeadToLf) {
				foundPos = leftFrontPos;
				foundIt = true;
			} else if (canHeadToRr) {
				foundPos = rightRearPos;
				foundIt = true;
			} else if (canHeadToRf) {
				foundPos = rightFrontPos;
				foundIt = true;
			}
		}
	}
	if (!foundIt)
		return false;

	helperPos = GetPosition() - foundPos;
	helperPos.z = 0.0f;
	if (helperPos.MagnitudeSqr() <= sq(0.5f))
		return false;

	pos->x = foundPos.x;
	pos->y = foundPos.y;
	return true;
}

void
CPed::SetLeader(CEntity *leader)
{
	m_leader = (CPed*)leader;

	if(m_leader)
		m_leader->RegisterReference((CEntity **)&m_leader);
}

#ifdef VC_PED_PORTS
bool
CPed::CanPedJumpThis(CEntity *unused, CVector *damageNormal)
{
	if (m_nSurfaceTouched == SURFACE_WATER)
		return true;

	CVector pos = GetPosition();
	CVector forwardOffset = GetForward();
	if (damageNormal && damageNormal->z > 0.17f) {
		if (damageNormal->z > 0.9f)
			return false;

		CColModel *ourCol = CModelInfo::GetModelInfo(m_modelIndex)->GetColModel();
		pos.z = ourCol->spheres->center.z - ourCol->spheres->radius * damageNormal->z + pos.z;
		pos.z = pos.z + 0.05f;
		float collPower = damageNormal->Magnitude2D();
		if (damageNormal->z > 0.5f) {
			CVector invDamageNormal(-damageNormal->x, -damageNormal->y, 0.0f);
			invDamageNormal *= 1.0f / collPower;
			CVector estimatedJumpDist = invDamageNormal + collPower * invDamageNormal * ourCol->spheres->radius;
			forwardOffset = estimatedJumpDist * Min(2.0f / collPower, 4.0f);
		} else {
			forwardOffset += collPower * ourCol->spheres->radius * forwardOffset;
		}
	} else {
		pos.z -= 0.15f;
	}

	CVector forwardPos = pos + forwardOffset;
	return CWorld::GetIsLineOfSightClear(pos, forwardPos, true, false, false, true, false, false, false);
}
#else
bool
CPed::CanPedJumpThis(CEntity *unused)
{
	CVector2D forward(-Sin(m_fRotationCur), Cos(m_fRotationCur));
	CVector pos = GetPosition();
	CVector forwardPos(
		forward.x + pos.x,
		forward.y + pos.y,
		pos.z);

	return CWorld::GetIsLineOfSightClear(pos, forwardPos, true, false, false, true, false, false, false);
}
#endif

void
CPed::SetJump(void)
{
	if (!bInVehicle &&
#if defined VC_PED_PORTS || defined FIX_BUGS
		m_nPedState != PED_JUMP && !RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_JUMP_LAUNCH) &&
#endif
		(m_nSurfaceTouched != SURFACE_STEEP_CLIFF || DotProduct(GetForward(), m_vecDamageNormal) >= 0.0f)) {
		SetStoredState();
		SetPedState(PED_JUMP);
		CAnimBlendAssociation *jumpAssoc = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_JUMP_LAUNCH, 8.0f);
		jumpAssoc->SetFinishCallback(FinishLaunchCB, this);
		m_fRotationDest = m_fRotationCur;
	}
}

void
CPed::FinishLaunchCB(CAnimBlendAssociation *animAssoc, void *arg)
{
	CPed *ped = (CPed*)arg;

	if (ped->m_nPedState != PED_JUMP)
		return;

	CVector forward(0.15f * ped->GetForward() + ped->GetPosition());
	forward.z += CModelInfo::GetModelInfo(ped->GetModelIndex())->GetColModel()->spheres->center.z + 0.25f;

	CEntity *obstacle = CWorld::TestSphereAgainstWorld(forward, 0.25f, nil, true, true, false, true, false, false);
	if (!obstacle) {
		// Forward of forward
		forward += 0.15f * ped->GetForward();
		forward.z += 0.15f;
		obstacle = CWorld::TestSphereAgainstWorld(forward, 0.25f, nil, true, true, false, true, false, false);
	}

	if (obstacle) {
		animAssoc->flags |= ASSOC_DELETEFADEDOUT;

		// ANIM_HIT_WALL in VC (which makes more sense)
		CAnimBlendAssociation *handsCoverAssoc = CAnimManager::BlendAnimation(ped->GetClump(), ASSOCGRP_STD, ANIM_STD_HANDSCOWER, 8.0f);
		handsCoverAssoc->flags &= ~ASSOC_FADEOUTWHENDONE;
		handsCoverAssoc->SetFinishCallback(FinishHitHeadCB, ped);
		ped->bIsLanding = true;
		return;
	}

	float velocityFromAnim = 0.1f;
	CAnimBlendAssociation *sprintAssoc = RpAnimBlendClumpGetAssociation(ped->GetClump(), ANIM_STD_RUNFAST);

	if (sprintAssoc) {
		velocityFromAnim = 0.05f * sprintAssoc->blendAmount + 0.17f;
	} else {
		CAnimBlendAssociation *runAssoc = RpAnimBlendClumpGetAssociation(ped->GetClump(), ANIM_STD_RUN);
		if (runAssoc) {
			velocityFromAnim = 0.07f * runAssoc->blendAmount + 0.1f;
		}
	}

	if (ped->IsPlayer()
#ifdef VC_PED_PORTS
		|| ped->m_pedInObjective && ped->m_pedInObjective->IsPlayer()
#endif
		)
		ped->ApplyMoveForce(0.0f, 0.0f, 8.5f);
	else
		ped->ApplyMoveForce(0.0f, 0.0f, 4.5f);
	
	if (sq(velocityFromAnim) > ped->m_vecMoveSpeed.MagnitudeSqr2D()
#ifdef VC_PED_PORTS
		|| ped->m_pCurrentPhysSurface
#endif
		) {

#ifdef FREE_CAM
		if (TheCamera.Cams[0].Using3rdPersonMouseCam() && !CCamera::bFreeCam) {
#else
		if (TheCamera.Cams[0].Using3rdPersonMouseCam()) {
#endif
			float fpsAngle = ped->WorkOutHeadingForMovingFirstPerson(ped->m_fRotationCur);
			ped->m_vecMoveSpeed.x = -velocityFromAnim * Sin(fpsAngle);
			ped->m_vecMoveSpeed.y = velocityFromAnim * Cos(fpsAngle);
		} else {
			ped->m_vecMoveSpeed.x = -velocityFromAnim * Sin(ped->m_fRotationCur);
			ped->m_vecMoveSpeed.y = velocityFromAnim * Cos(ped->m_fRotationCur);
		}
#ifdef VC_PED_PORTS
		if (ped->m_pCurrentPhysSurface) {
			ped->m_vecMoveSpeed.x += ped->m_pCurrentPhysSurface->m_vecMoveSpeed.x;
			ped->m_vecMoveSpeed.y += ped->m_pCurrentPhysSurface->m_vecMoveSpeed.y;
		}
#endif
	}

	ped->bIsStanding = false;
	ped->bIsInTheAir = true;
	animAssoc->blendDelta = -1000.0f;
	CAnimManager::AddAnimation(ped->GetClump(), ASSOCGRP_STD, ANIM_STD_JUMP_GLIDE);

	if (ped->bDoBloodyFootprints) {
		CVector bloodPos(0.0f, 0.0f, 0.0f);
		ped->TransformToNode(bloodPos, PED_FOOTL);

		bloodPos.z -= 0.1f;
		bloodPos += 0.2f * ped->GetForward();

		CShadows::AddPermanentShadow(SHADOWTYPE_DARK, gpBloodPoolTex, &bloodPos,
			0.26f * ped->GetForward().x,
			0.26f * ped->GetForward().y,
			0.14f * ped->GetRight().x,
			0.14f * ped->GetRight().y,
			255, 255, 0, 0, 4.0f, 3000, 1.0f);

		bloodPos = CVector(0.0f, 0.0f, 0.0f);
		ped->TransformToNode(bloodPos, PED_FOOTR);

		bloodPos.z -= 0.1f;
		bloodPos += 0.2f * ped->GetForward();
		CShadows::AddPermanentShadow(SHADOWTYPE_DARK, gpBloodPoolTex, &bloodPos,
			0.26f * ped->GetForward().x,
			0.26f * ped->GetForward().y,
			0.14f * ped->GetRight().x,
			0.14f * ped->GetRight().y,
			255, 255, 0, 0, 4.0f, 3000, 1.0f);

		if (ped->m_bloodyFootprintCountOrDeathTime <= 40) {
			ped->m_bloodyFootprintCountOrDeathTime = 0;
			ped->bDoBloodyFootprints = false;
		} else {
			ped->m_bloodyFootprintCountOrDeathTime -= 40;
		}
	}
}

void
CPed::FinishJumpCB(CAnimBlendAssociation *animAssoc, void *arg)
{
	CPed *ped = (CPed*)arg;

	ped->bResetWalkAnims = true;
	ped->bIsLanding = false;

	animAssoc->blendDelta = -1000.0f;
}

void
CPed::FinishHitHeadCB(CAnimBlendAssociation *animAssoc, void *arg)
{
	CPed *ped = (CPed*)arg;

	if (animAssoc) {
		animAssoc->blendDelta = -4.0f;
		animAssoc->flags |= ASSOC_DELETEFADEDOUT;
	}

	if (ped->m_nPedState == PED_JUMP)
		ped->RestorePreviousState();

	ped->bIsLanding = false;
}

bool
CPed::CanPedDriveOff(void)
{
	if (m_nPedState != PED_DRIVING || m_lookTimer > CTimer::GetTimeInMilliseconds())
		return false;

	for (int i = 0; i < m_numNearPeds; i++) {
		CPed *nearPed = m_nearPeds[i];
		if (nearPed->m_nPedType == m_nPedType && nearPed->m_objective == OBJECTIVE_ENTER_CAR_AS_PASSENGER && nearPed->m_carInObjective == m_carInObjective) {
			m_lookTimer = CTimer::GetTimeInMilliseconds() + 1000;
			return false;
		}
	}
	return true;
}

// These categories are purely random, most of ped models have no correlation. So I don't think making an enum.
uint8
CPed::GetPedRadioCategory(uint32 modelIndex)
{
	switch (modelIndex) {
		case MI_MALE01:
		case MI_FEMALE03:
		case MI_PROSTITUTE2:
		case MI_WORKER1:
		case MI_MOD_MAN:
		case MI_MOD_WOM:
		case MI_ST_WOM:
		case MI_FAN_WOM:
			return 3;
		case MI_TAXI_D:
		case MI_PIMP:
		case MI_MALE02:
		case MI_FEMALE02:
		case MI_FATFEMALE01:
		case MI_FATFEMALE02:
		case MI_DOCKER1:
		case MI_WORKER2:
		case MI_FAN_MAN2:
			return 9;
		case MI_GANG01:
		case MI_GANG02:
		case MI_SCUM_MAN:
		case MI_SCUM_WOM:
		case MI_HOS_WOM:
		case MI_CONST1:
			return 1;
		case MI_GANG03:
		case MI_GANG04:
		case MI_GANG07:
		case MI_GANG08:
		case MI_CT_MAN2:
		case MI_CT_WOM2:
		case MI_B_MAN3:
		case MI_SHOPPER3:
			return 4;
		case MI_GANG05:
		case MI_GANG06:
		case MI_GANG11:
		case MI_GANG12:
		case MI_CRIMINAL02:
		case MI_B_WOM2:
		case MI_ST_MAN:
		case MI_HOS_MAN:
			return 5;
		case MI_FATMALE01:
		case MI_LI_MAN2:
		case MI_SHOPPER1:
		case MI_CAS_MAN:
			return 6;
		case MI_PROSTITUTE:
		case MI_P_WOM2:
		case MI_LI_WOM2:
		case MI_B_WOM3:
		case MI_CAS_WOM:
			return 2;
		case MI_P_WOM1:
		case MI_DOCKER2:
		case MI_STUD_MAN:
			return 7;
		case MI_CT_MAN1:
		case MI_CT_WOM1:
		case MI_LI_MAN1:
		case MI_LI_WOM1:
		case MI_B_MAN1:
		case MI_B_MAN2:
		case MI_B_WOM1:
		case MI_SHOPPER2:
		case MI_STUD_WOM:
			return 8;
		default:
			return 0;
	}
}

void
CPed::SetRadioStation(void)
{
	static const uint8 radiosPerRadioCategories[10][4] = {
		{JAH_RADIO, RISE_FM, GAME_FM, MSX_FM},
		{HEAD_RADIO, DOUBLE_CLEF, LIPS_106, FLASHBACK},
		{RISE_FM, GAME_FM, MSX_FM, FLASHBACK},
		{HEAD_RADIO, RISE_FM, LIPS_106, MSX_FM},
		{HEAD_RADIO, RISE_FM, MSX_FM, FLASHBACK},
		{JAH_RADIO, RISE_FM, LIPS_106, FLASHBACK},
		{HEAD_RADIO, RISE_FM, LIPS_106, FLASHBACK},
		{HEAD_RADIO, JAH_RADIO, LIPS_106, FLASHBACK},
		{HEAD_RADIO, DOUBLE_CLEF, LIPS_106, FLASHBACK},
		{CHATTERBOX, HEAD_RADIO, LIPS_106, GAME_FM}
	};
	uint8 orderInCat = 0; // BUG: this wasn't initialized

	if (IsPlayer() || !m_pMyVehicle || m_pMyVehicle->pDriver != this)
		return;

	uint8 category = GetPedRadioCategory(GetModelIndex());
	if (DMAudio.IsMP3RadioChannelAvailable()) {
		if (CGeneral::GetRandomNumber() & 15) {
			for (orderInCat = 0; orderInCat < 4; orderInCat++) {
				if (m_pMyVehicle->m_nRadioStation == radiosPerRadioCategories[category][orderInCat])
					break;
			}
		} else {
			m_pMyVehicle->m_nRadioStation = USERTRACK;
		}
	} else {
		for (orderInCat = 0; orderInCat < 4; orderInCat++) {
			if (m_pMyVehicle->m_nRadioStation == radiosPerRadioCategories[category][orderInCat])
				break;
		}
	}
	if (orderInCat == 4) {
		if (DMAudio.IsMP3RadioChannelAvailable()) {
			if (CGeneral::GetRandomNumber() & 15)
				m_pMyVehicle->m_nRadioStation = radiosPerRadioCategories[category][CGeneral::GetRandomNumber() & 3];
			else
				m_pMyVehicle->m_nRadioStation = USERTRACK;
		} else {
			m_pMyVehicle->m_nRadioStation = radiosPerRadioCategories[category][CGeneral::GetRandomNumber() & 3];
		}
	}
}

void
CPed::WarpPedIntoCar(CVehicle *car)
{
	bInVehicle = true;
	m_pMyVehicle = car;
	m_pMyVehicle->RegisterReference((CEntity **) &m_pMyVehicle);
	m_carInObjective = car;
	m_carInObjective->RegisterReference((CEntity **) &m_carInObjective);
	SetPedState(PED_DRIVING);
	bUsesCollision = false;
	bIsInTheAir = false;
	bVehExitWillBeInstant = true;
	if (m_objective == OBJECTIVE_ENTER_CAR_AS_DRIVER) {
		car->SetDriver(this);
		car->pDriver->RegisterReference((CEntity **) &car->pDriver);

	} else if (m_objective == OBJECTIVE_ENTER_CAR_AS_PASSENGER) {
		for (int i = 0; i < 4; i++) {
			if (!car->pPassengers[i]) {
				car->pPassengers[i] = this;
				car->pPassengers[i]->RegisterReference((CEntity **) &car->pPassengers[i]);
				break;
			}
		}
	} else
		return;

	if (IsPlayer()) {
		car->SetStatus(STATUS_PLAYER);
		AudioManager.PlayerJustGotInCar();
		CCarCtrl::RegisterVehicleOfInterest(car);
	} else {
		car->SetStatus(STATUS_PHYSICS);
	}

	CWorld::Remove(this);
	SetPosition(car->GetPosition());
	CWorld::Add(this);

	if (car->bIsAmbulanceOnDuty) {
		car->bIsAmbulanceOnDuty = false;
		--CCarCtrl::NumAmbulancesOnDuty;
	}
	if (car->bIsFireTruckOnDuty) {
		car->bIsFireTruckOnDuty = false;
		--CCarCtrl::NumFiretrucksOnDuty;
	}
	if (!car->bEngineOn) {
		car->bEngineOn = true;
		DMAudio.PlayOneShot(car->m_audioEntityId, SOUND_CAR_ENGINE_START, 1.0f);
	}

#ifdef VC_PED_PORTS
	RpAnimBlendClumpSetBlendDeltas(GetClump(), ASSOC_PARTIAL, -1000.0f);

	// VC uses AddInCarAnims but we don't have that
	m_pVehicleAnim = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, car->GetDriverAnim(), 100.0f);
	RemoveWeaponWhenEnteringVehicle();
#else
	if (car->IsBoat()) {
#ifndef FIX_BUGS
		m_pVehicleAnim = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_BOAT_DRIVE, 100.0f);
#else
		m_pVehicleAnim = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, car->GetDriverAnim(), 100.0f);
#endif
		CWeaponInfo *ourWeapon = CWeaponInfo::GetWeaponInfo(GetWeapon()->m_eWeaponType);
		RemoveWeaponModel(ourWeapon->m_nModelId);
	} else {
		// Because we can use Uzi for drive by
		RemoveWeaponWhenEnteringVehicle();

		if (car->bLowVehicle)
			m_pVehicleAnim = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_CAR_SIT_LO, 100.0f);
		else
			m_pVehicleAnim = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_CAR_SIT, 100.0f);
	}
#endif

	StopNonPartialAnims();
	if (car->bIsBus)
		bRenderPedInCar = false;

	bChangedSeat = true;
}


#ifdef PEDS_REPORT_CRIMES_ON_PHONE
// returns event id, parameter is optional
int32
CPed::CheckForPlayerCrimes(CPed *victim)
{
	int i;
	float dist;
	float mindist = 60.0f;
	CPlayerPed *player = FindPlayerPed();
	int32 victimRef = (victim ? CPools::GetPedRef(victim) : 0);
	int event = -1;

	for (i = 0; i < NUMEVENTS; i++) {
		if (gaEvent[i].type == EVENT_NULL || gaEvent[i].type > EVENT_CAR_SET_ON_FIRE)
			continue;

		// those are already handled in game, also DEAD_PED isn't registered alone, most of the time there is SHOOT_PED etc.
		if (gaEvent[i].type == EVENT_DEAD_PED || gaEvent[i].type == EVENT_GUNSHOT || gaEvent[i].type == EVENT_EXPLOSION)
			continue;

		if (victim && gaEvent[i].entityRef != victimRef)
			continue;

		if (gaEvent[i].criminal != player)
			continue;

		dist = (GetPosition() - gaEvent[i].posn).Magnitude();
		if (dist < mindist) {
			mindist = dist;
			event = i;
		}
	}

	if (event != -1) {
		if (victim) {
			m_victimOfPlayerCrime = victim;
		} else {
			switch (gaEvent[event].entityType) {
			case EVENT_ENTITY_PED:
				m_victimOfPlayerCrime = CPools::GetPed(gaEvent[event].entityRef);
				break;
			case EVENT_ENTITY_VEHICLE:
				m_victimOfPlayerCrime = CPools::GetVehicle(gaEvent[event].entityRef);
				break;
			case EVENT_ENTITY_OBJECT:
				m_victimOfPlayerCrime = CPools::GetObject(gaEvent[event].entityRef);
				break;
			default:
				break;
			}
		}
	}

	return event;
}
#endif

#ifdef PED_SKIN
static RpMaterial*
SetLimbAlphaCB(RpMaterial *material, void *data)
{
	((RwRGBA*)RpMaterialGetColor(material))->alpha = *(uint8*)data;
	return material;
}

void
CPed::renderLimb(int node)
{
	RpHAnimHierarchy *hier = GetAnimHierarchyFromSkinClump(GetClump());
	int idx = RpHAnimIDGetIndex(hier, m_pFrames[node]->nodeID);
	RwMatrix *mat = &RpHAnimHierarchyGetMatrixArray(hier)[idx];
	CPedModelInfo *mi = (CPedModelInfo *)CModelInfo::GetModelInfo(GetModelIndex());
	RpAtomic *atomic;
	switch(node){
	case PED_HEAD:
		atomic = mi->getHead();
		break;
	case PED_HANDL:
		atomic = mi->getLeftHand();
		break;
	case PED_HANDR:
		atomic = mi->getRightHand();
		break;
	default:
		return;
	}
	if(atomic == nil)
		return;

	RwFrame *frame = RpAtomicGetFrame(atomic);
	*RwFrameGetMatrix(frame) = *mat;
	RwFrameUpdateObjects(frame);
	int alpha = CVisibilityPlugins::GetClumpAlpha(GetClump());
	RpGeometryForAllMaterials(RpAtomicGetGeometry(atomic), SetLimbAlphaCB, &alpha);
	RpAtomicRender(atomic);
}
#endif

#ifdef COMPATIBLE_SAVES
#define CopyFromBuf(buf, data) memcpy(&data, buf, sizeof(data)); SkipSaveBuf(buf, sizeof(data));
#define CopyToBuf(buf, data) memcpy(buf, &data, sizeof(data)); SkipSaveBuf(buf, sizeof(data));
void
CPed::Save(uint8*& buf)
{
	SkipSaveBuf(buf, 52);
	CopyToBuf(buf, GetPosition().x);
	CopyToBuf(buf, GetPosition().y);
	CopyToBuf(buf, GetPosition().z);
	SkipSaveBuf(buf, 288);
	CopyToBuf(buf, CharCreatedBy);
	SkipSaveBuf(buf, 351);
	CopyToBuf(buf, m_fHealth);
	CopyToBuf(buf, m_fArmour);
	SkipSaveBuf(buf, 148);
	for (int i = 0; i < 13; i++) // has to be hardcoded
		m_weapons[i].Save(buf);
	SkipSaveBuf(buf, 5);
	CopyToBuf(buf, m_maxWeaponTypeAllowed);
	SkipSaveBuf(buf, 162);
}

void
CPed::Load(uint8*& buf)
{
	SkipSaveBuf(buf, 52);
	CopyFromBuf(buf, GetMatrix().GetPosition().x);
	CopyFromBuf(buf, GetMatrix().GetPosition().y);
	CopyFromBuf(buf, GetMatrix().GetPosition().z);
	SkipSaveBuf(buf, 288);
	CopyFromBuf(buf, CharCreatedBy);
	SkipSaveBuf(buf, 351);
	CopyFromBuf(buf, m_fHealth);
	CopyFromBuf(buf, m_fArmour);
	SkipSaveBuf(buf, 148);
	for (int i = 0; i < 13; i++) // has to be hardcoded
		m_weapons[i].Load(buf);
	SkipSaveBuf(buf, 5);
	CopyFromBuf(buf, m_maxWeaponTypeAllowed);
	SkipSaveBuf(buf, 162);
}
#undef CopyFromBuf
#undef CopyToBuf
#endif
