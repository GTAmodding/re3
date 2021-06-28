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
#include "Streaming.h"
#include "PedAttractor.h"
#include "GameLogic.h"
#include "Bike.h"
#include "WindModifiers.h"
#include "CutsceneShadow.h"
#include "Clock.h"
#include "Wanted.h"
#include "SaveBuf.h"

CPed *gapTempPedList[50];
uint16 gnNumTempPedList;

static CColPoint aTempPedColPts[MAX_COLLISION_POINTS];

uint16 CPed::nThreatReactionRangeMultiplier = 1;
uint16 CPed::nEnterCarRangeMultiplier = 1;

bool CPed::bNastyLimbsCheat;
bool CPed::bFannyMagnetCheat;
bool CPed::bPedCheat3;
CVector2D CPed::ms_vec2DFleePosition;

void *CPed::operator new(size_t sz) throw() { return CPools::GetPedPool()->New();  }
void *CPed::operator new(size_t sz, int handle) throw() { return CPools::GetPedPool()->New(handle); }
void CPed::operator delete(void *p, size_t sz) throw() { CPools::GetPedPool()->Delete((CPed*)p); }
void CPed::operator delete(void *p, int handle) throw() { CPools::GetPedPool()->Delete((CPed*)p); }

float gfTommyFatness = 1.0f;

CPed::CPed(uint32 pedType) : m_pedIK(this)
{
#ifdef USE_CUTSCENE_SHADOW_FOR_PED
	m_pRTShadow = nil;
#endif
	m_vecAnimMoveDelta.x = 0.0f;
	m_vecAnimMoveDelta.y = 0.0f;
	m_fHealth = 100.0f;
	m_fArmour = 0.0f;
	m_nPedType = pedType;
	m_lastSoundStart = 0;
	m_soundStart = 0;
	m_lastQueuedSound = SOUND_NO_SOUND;
	m_queuedSound = SOUND_NO_SOUND;
	m_canTalk = true;

	m_type = ENTITY_TYPE_PED;
	bPedPhysics = true;
	bUseCollisionRecords = true;

	m_objective = OBJECTIVE_NONE;
	m_prevObjective = OBJECTIVE_NONE;
#ifdef FIX_BUGS
	m_objectiveTimer = 0;
#endif
	CharCreatedBy = RANDOM_CHAR;
	m_leader = nil;
	m_pedInObjective = nil;
	m_attractorHeading = 0.0f;
	m_carInObjective = nil;
	m_attractorHeading = 0.0f;
	bInVehicle = false;
	m_pMyVehicle = nil;
	m_pVehicleAnim = nil;
	m_vecOffsetSeek.x = 0.0f;
	m_vecOffsetSeek.y = 0.0f;
	m_vecOffsetSeek.z = 0.0f;
	m_attractor = nil;
	m_positionInQueue = -1;
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
	m_acceptableHeadingOffset = 0.1f;
	m_followPathDestPos = CVector(0.f, 0.f, 0.f);
	m_followPathAbortDist = 0.0f;
	m_followPathMoveState = PEDMOVE_NONE;
	bRunningToPhone = false;
	m_phoneId = -1;
	m_lastAccident = 0;
	m_fleeFrom = nil;
	m_fleeFromPos = CVector2D(0.0f, 0.0f);
	m_fleeTimer = 0;
	m_threatEx = nil;
	m_vecSpotToGuard = CVector(0.0f, 0.0f, 0.0f);
	m_radiusToGuard = 0.0f;
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

	for (int i = 0; i < ARRAY_SIZE(m_pathNodesToGo); i++) {
		m_pathNodesToGo[i] = nil;
	}
	m_nNumPathNodes = 0;
	m_nCurPathNodeId = 0;
	m_nPathDir = 0;
	m_pLastPathNode = nil;
	m_pNextPathNode = nil;
	m_followPathWalkAroundEnt = nil;
	m_followPathTargetEnt = nil;
	m_pathNodeTimer = 0;
	m_pCurPathNode = nil;

	m_threatFlags = 0;
	m_threatCheckTimer = 0;
	m_threatCheckInterval = CGeneral::GetRandomNumberInRange(250, 1000);

	m_routeLastPoint = -1;
	m_routeStartPoint = 0;
	m_routePointsPassed = 0;
	m_routeType = 0;

	m_fMass = 70.0f;
	m_fTurnMass = 100.0f;
	m_fAirResistance = 0.4f / m_fMass;
	m_fElasticity = 0.05f;

	m_ceaseAttackTimer = 0;
	m_bodyPartBleeding = -1;

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
	bDontAcceptIKLookAts = false;
	bReachedAttractorHeadingTarget = false;
	bTurnedAroundOnAttractor = false;
#ifdef KANGAROO_CHEAT
	m_ped_flagI80 = false;
#endif

	bHasAlreadyUsedAttractor = false;
	bHasAlreadyStoleACar = false;
	bCarPassenger = false;
	bFleeWhenStanding = false;
	bGotUpOfMyOwnAccord = false;
	bMiamiViceCop = false;
	bMoneyHasBeenGivenByScript = false;
	bHasBeenPhotographed = false;

	bIsDrowning = false;
	bDrownsInWater = true;
	bWaitForLeaderToComeCloser = false;
	bHeldHostageInCar = false;
	bIsPlayerFriend = true;
	bHeadStuckInCollision = false;
	bDeadPedInFrontOfCar = false;

	m_gangFlags = 0xFF;
	
	bStayInCarOnJack = false;

	bDontFight = false;
	bDoomAim = true;
	bCanBeShotInVehicle = true;
	bPushedAlongByCar = false;
	bRemoveMeWhenIGotIntoCar = false;
	bIgnoreThreatsBehindObjects = false;

	bNeverEverTargetThisPed = false;
	bCrouchWhenScared = false;
	bKnockedOffBike = false;
	b158_8 = false;
	bCollectBusFare = false;
	bBoughtIceCream = false;
	bDonePositionOutOfCollision = false;
	bCanAttackPlayerWithCops = false;

	if (CGeneral::GetRandomNumber() & 3)
		bHasACamera = false;
	else
		bHasACamera = true;

	if (CGeneral::GetRandomNumberInRange(0.0f, 1.0f) <= 0.95f)
		bCanGiveUpSunbathing = false;
	else
		bCanGiveUpSunbathing = true;

	m_audioEntityId = DMAudio.CreateEntity(AUDIOTYPE_PHYSICAL, this);
	DMAudio.SetEntityStatus(m_audioEntityId, TRUE);
	m_fearFlags = CPedType::GetThreats(m_nPedType);
	m_threatEntity = nil;
	m_eventOrThreat = CVector2D(0.0f, 0.0f);
	m_pEventEntity = nil;
	m_fAngleToEvent = 0.0f;
	m_numNearPeds = 0;

	for (int i = 0; i < ARRAY_SIZE(m_nearPeds); i++) {
		m_nearPeds[i] = nil;
	}
	m_maxWeaponTypeAllowed = WEAPONTYPE_UNARMED;
	m_currentWeapon = WEAPONTYPE_UNARMED;
	m_storedWeapon = WEAPONTYPE_UNIDENTIFIED;
	m_delayedWeapon = WEAPONTYPE_UNIDENTIFIED;

	for(int i = 0; i < TOTAL_WEAPON_SLOTS; i++) {
		CWeapon &weapon = GetWeapon(i);
		weapon.m_eWeaponType = WEAPONTYPE_UNARMED;
		weapon.m_eWeaponState = WEAPONSTATE_READY;
		weapon.m_nAmmoInClip = 0;
		weapon.m_nAmmoTotal = 0;
		weapon.m_nTimer = 0;
	}

	m_curFightMove = m_lastFightMove = FIGHTMOVE_IDLE;
	GiveWeapon(WEAPONTYPE_UNARMED, 0, true);
	m_wepAccuracy = 60;
	m_lastWepDam = -1;
	m_lastDamEntity = nil;
	m_attachedTo = nil;
	m_attachWepAmmo = 0;
	m_collPoly.valid = false;
	m_fCollisionSpeed = 0.0f;
	m_wepModelID = -1;
	uint16 random = CGeneral::GetRandomNumber();
	m_nPedMoney = random % 25;
	if (m_nPedMoney == 23)
		m_nPedMoney = 400;
	m_bleedCounter = 0;
	m_nExtendedRangeTimer = 0;
	m_vehicleInAccident = nil;
	m_attractor = nil;
	m_positionInQueue = -1;
	m_pWeaponModel = nil;
	m_delayedSoundID = -1;
	m_delayedSoundTimer = 0;
	CPopulation::UpdatePedCount((ePedType)m_nPedType, false);
	m_lastComment = UINT32_MAX;
}

CPed::~CPed(void)
{
#ifdef USE_CUTSCENE_SHADOW_FOR_PED
	if ( m_pRTShadow ) delete m_pRTShadow;
#endif
	CWorld::Remove(this);
	if (m_attractor)
		GetPedAttractorManager()->DeRegisterPed(this, m_attractor);
	CRadar::ClearBlipForEntity(BLIP_CHAR, CPools::GetPedPool()->GetIndex(this));
	if (InVehicle()){
		uint8 door_flag = GetCarDoorFlag(m_vehDoor);
		if (m_pMyVehicle->pDriver == this)
			m_pMyVehicle->pDriver = nil;
		else {
			// FIX: Passenger counter now being decreased after removing ourself from vehicle.
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

	ClearWeapons();
	if (bCarPassenger)
		CPopulation::ms_nTotalCarPassengerPeds--;
	if (bMiamiViceCop)
		CPopulation::NumMiamiViceCops--;
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

	if (!CanUseTorsoWhenLooking())
		m_pedIK.m_flags |= CPedIK::LOOKAROUND_HEAD_ONLY;

	(*RPANIMBLENDCLUMPDATA(m_rwObject))->velocity2d = &m_vecAnimMoveDelta;

	if(modelInfo->GetHitColModel() == nil)
		modelInfo->CreateHitColModelSkinned(GetClump());

	UpdateRpHAnim();
#ifdef USE_CUTSCENE_SHADOW_FOR_PED
	if (!m_pRTShadow)
	{
		m_pRTShadow = new CCutsceneShadow;
		m_pRTShadow->Create(m_rwObject, 10, 1, 1, 1);
		//m_pRTShadow->Create(m_rwObject, 8, 0, 0, 0);
	}
#endif
}

void
CPed::SetPedStats(ePedStats pedStat)
{
	m_pedStats = CPedStats::ms_apPedStats[pedStat];
}

void
CPed::DeleteRwObject()
{
	CEntity::DeleteRwObject();
}

void
CPed::BuildPedLists(void)
{
	if (((CTimer::GetFrameCounter() + m_randomSeed) % 16) == 0) {
		CVector centre = CEntity::GetBoundCentre();
		int deadsRegistered = 0;
		CRect rect(centre.x - 20.f * nThreatReactionRangeMultiplier,
			centre.y - 20.f * nThreatReactionRangeMultiplier,
			centre.x + 20.f * nThreatReactionRangeMultiplier,
			centre.y + 20.f * nThreatReactionRangeMultiplier);
		int xstart = CWorld::GetSectorIndexX(rect.left);
		int ystart = CWorld::GetSectorIndexY(rect.top);
		int xend = CWorld::GetSectorIndexX(rect.right);
		int yend = CWorld::GetSectorIndexY(rect.bottom);
		gnNumTempPedList = 0;

		for(int y = ystart; y <= yend; y++) {
			for(int x = xstart; x <= xend; x++) {
				for (CPtrNode *pedPtrNode = CWorld::GetSector(x,y)->m_lists[ENTITYLIST_PEDS].first; pedPtrNode; pedPtrNode = pedPtrNode->next) {
					CPed *ped = (CPed*)pedPtrNode->item;
					if (ped != this && (!ped->bInVehicle || (ped->m_pMyVehicle && ped->m_pMyVehicle->IsBike()))) {

						if (nThreatReactionRangeMultiplier * 30.0f > (ped->GetPosition() - GetPosition()).Magnitude2D()) {
							if (ped->m_nPedState == PED_DEAD) {
								if (deadsRegistered > 3)
									continue;
								deadsRegistered++;
							}
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
	}

	for(int i = 0; i < ARRAY_SIZE(m_nearPeds); ) {
		bool removePed = false;
		if (m_nearPeds[i]) {
			if (m_nearPeds[i]->IsPointerValid()) {
				float distSqr = (GetPosition() - m_nearPeds[i]->GetPosition()).MagnitudeSqr2D();
				if (distSqr > sq(nThreatReactionRangeMultiplier * 30.f)) {
					removePed = true;
				}
			} else {
				removePed = true;
			}
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

bool
CPed::OurPedCanSeeThisOne(CEntity *target, bool shootablesDoBlock)
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
	return !CWorld::ProcessLineOfSight(GetPosition() + CVector(0.f, 0.f, 1.f), target->GetPosition() + CVector(0.f, 0.f, 1.f),
		colpoint, ent, true, false, false, shootablesDoBlock, false, false, false, shootablesDoBlock);
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
	if (m_nStoredMoveState == m_nMoveState || !IsPedInControl() || m_attachedTo)
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
		animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_FIGHT_IDLE);
		if (!animAssoc)
			animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_MELEE_IDLE_FIGHTMODE);

		if (animAssoc && m_nPedState == PED_FIGHT)
			return;

		if (animAssoc) {
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
	if (m_nPedState != PED_IDLE) {
		m_nLastPedState = m_nPedState;
		if (m_nMoveState >= m_nPrevMoveState)
			m_nPrevMoveState = m_nMoveState;
	}
}

void
CPed::RestorePreviousState(void)
{
	if (!CanSetPedState() || m_nPedState == PED_FALL)
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
						CVector nextNode = CPathFind::TakeWidthIntoAccountForWandering(m_pNextPathNode, m_randomSeed);
						CVector diff = nextNode - GetPosition();
						if (diff.MagnitudeSqr() < sq(7.0f)) {
							SetMoveState(PEDMOVE_WALK);
							break;
						}
					}
				}
				SetWanderPath(m_nPedState == PED_FOLLOW_PATH ? m_nPathDir : CGeneral::GetRandomNumber() & 7);
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
	
	if (fearFlags & PED_FLAG_GUN) {
		CPed *shooter = CheckForGunShots();
		if (shooter && (m_nPedType != shooter->m_nPedType || m_nPedType == PEDTYPE_CIVMALE || m_nPedType == PEDTYPE_CIVFEMALE)) {
			if (!IsGangMember()) {
				m_threatEntity = shooter;
				m_threatEntity->RegisterReference((CEntity**)&m_threatEntity);
				return PED_FLAG_GUN;
			}

			if (CPedType::GetFlag(shooter->m_nPedType) & fearFlags || m_nPedType == PEDTYPE_GANG5) {
				if (m_threatEntity)
					m_threatEntity->CleanUpOldReference(&m_threatEntity);
				m_threatEntity = shooter;
				m_threatEntity->RegisterReference((CEntity**)&m_threatEntity);
				return CPedType::GetFlag(shooter->m_nPedType);
			}
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
		bool weSawOurEnemy = false;
		bool weMaySeeOurEnemy = false;
		float closestEnemyDist = 60.0f;
		if ((CTimer::GetFrameCounter() + (uint8)m_randomSeed + 16) & 4) {

			for (int i = 0; i < m_numNearPeds; ++i) {
				if (CharCreatedBy == RANDOM_CHAR && m_nearPeds[i]->CharCreatedBy == MISSION_CHAR && !m_nearPeds[i]->IsPlayer()) {
						continue;
				}

				// BUG: Putting this here will result in returning the flags of farthest ped to us, since m_nearPeds is sorted by distance.
				// 		Fixed at the bottom of the function.
				flagsOfNearPed = CPedType::GetFlag(m_nearPeds[i]->m_nPedType);

				if (flagsOfNearPed & fearFlags) {
					if (m_nearPeds[i]->m_fHealth > 0.0f) {
						if (OurPedCanSeeThisOne(m_nearPeds[i], !!bIgnoreThreatsBehindObjects)) {
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
								if (!CWorld::ProcessLineOfSight(ourPos, nearPed->GetPosition(), foundCol, foundEnt,
									true, false, false, !!bIgnoreThreatsBehindObjects, false, false, false)) {

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
CPed::ScanForDelayedResponseThreats(void)
{
	if (m_threatFlags)
		return;

	m_threatEntity = nil;
	m_pEventEntity = nil;
	m_threatFlags = ScanForThreats();
	if (m_threatFlags) {
		if (m_threatEntity || m_pEventEntity) {
			m_threatCheckTimer = CTimer::GetTimeInMilliseconds() + m_threatCheckInterval;
			return;
		}
		m_threatFlags = 0;
	}
	m_threatCheckTimer = 0;
}

void
CPed::CheckThreatValidity(void)
{
	if (m_threatEntity && !IsEntityPointerValid(m_threatEntity)) {
		m_threatFlags = 0;
		m_threatEntity = nil;
	}
	if (m_pEventEntity && !IsEntityPointerValid(m_pEventEntity)) {
		m_threatFlags = 0;
		m_pEventEntity = nil;
	}
	if (!m_threatEntity && !m_pEventEntity)
		m_threatFlags = 0;
}

bool
CPed::CanUseTorsoWhenLooking(void)
{
	if (m_nPedState != PED_DRIVING && m_nPedState != PED_DRAG_FROM_CAR && !bIsDucking) {
		if (m_animGroup != ASSOCGRP_SEXYWOMAN && m_animGroup != ASSOCGRP_WOMAN)
			return true;
	}
	return false;
}

void
CPed::SetLookFlag(float direction, bool keepTryingToLook, bool cancelPrevious)
{
	if (m_lookTimer < CTimer::GetTimeInMilliseconds() || cancelPrevious) {
		bIsLooking = true;
		bIsRestoringLook = false;
		m_fLookDirection = direction;
		m_pLookTarget = nil;
		m_lookTimer = 0;
		bKeepTryingToLook = keepTryingToLook;
		if (CanUseTorsoWhenLooking()) {
			m_pedIK.m_flags &= ~CPedIK::LOOKAROUND_HEAD_ONLY;
		}
	}
}

void
CPed::SetLookFlag(CEntity *target, bool keepTryingToLook, bool cancelPrevious)
{
	if (m_lookTimer < CTimer::GetTimeInMilliseconds() || cancelPrevious) {
		bIsLooking = true;
		bIsRestoringLook = false;
		m_pLookTarget = target;
		m_pLookTarget->RegisterReference((CEntity**)&m_pLookTarget);
		m_fLookDirection = 999999.0f;
		m_lookTimer = 0;
		bKeepTryingToLook = keepTryingToLook;
		if (CanUseTorsoWhenLooking()) {
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

		if (CanUseTorsoWhenLooking())
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
CPed::MoveHeadToLook(void)
{
	CVector lookPos;

	if (m_lookTimer && CTimer::GetTimeInMilliseconds() > m_lookTimer) {
		ClearLookFlag();
	} 

	if (bIsLooking || bIsRestoringLook)
		if (!CanUseTorsoWhenLooking())
			m_pedIK.m_flags |= CPedIK::LOOKAROUND_HEAD_ONLY;

	if (m_pLookTarget) {
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

		if (m_nPedState == PED_ANSWER_MOBILE)
			return;

		if (m_lookTimer - CTimer::GetTimeInMilliseconds() >= 1000)
			return;

		bool handFreeToMove = false;
		AnimationId animToPlay = ANIM_STD_NUM;

		if (!GetWeapon()->IsType2Handed() && GetWeapon()->m_eWeaponType != WEAPONTYPE_ROCKETLAUNCHER)
			handFreeToMove = true;

		if (IsPlayer() && handFreeToMove) {

			if (m_pLookTarget->IsPed()) {
#ifdef FIX_BUGS
				if (m_pedStats->m_temper > 49 || ((CPed*)m_pLookTarget)->m_nPedType == PEDTYPE_COP)
#else
				if (m_pedStats->m_temper < 49 || ((CPed*)m_pLookTarget)->m_nPedType == PEDTYPE_COP)
#endif
					animToPlay = ANIM_STD_PARTIAL_FUCKU;
				else if(m_pedStats->m_temper < 47)
					animToPlay = ANIM_STD_PARTIAL_PUNCH;
			} else {
				if (m_pedStats->m_temper > 49 || m_pLookTarget->GetModelIndex() == MI_POLICE)
					animToPlay = ANIM_STD_PARTIAL_FUCKU;
			}
		} else if (handFreeToMove && (CGeneral::GetRandomNumber() & 1)) {
			animToPlay = ANIM_STD_PARTIAL_FUCKU;
		}

		if (animToPlay != ANIM_STD_NUM) {
			CAnimBlendAssociation *newAssoc = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, animToPlay, 4.0f);

			if (newAssoc) {
				newAssoc->flags |= ASSOC_FADEOUTWHENDONE;
				newAssoc->flags |= ASSOC_DELETEFADEDOUT;
			}
		}
		bShakeFist = false;
	} else if (m_fLookDirection == 999999.0f) {
		ClearLookFlag();
	} else if (!m_pedIK.LookInDirection(m_fLookDirection, 0.0f)) {
		if (!bKeepTryingToLook)
			ClearLookFlag();
	}
}

void
CPed::RestoreHeadPosition(void)
{
	if(!CanUseTorsoWhenLooking())
		m_pedIK.m_flags |= CPedIK::LOOKAROUND_HEAD_ONLY;

	if (m_pedIK.RestoreLookAt()) {
		bIsRestoringLook = false;
		if(CanUseTorsoWhenLooking())
			m_pedIK.m_flags &= ~CPedIK::LOOKAROUND_HEAD_ONLY;
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

	if (bIsDucking)
		m_pedIK.m_flags &= ~CPedIK::AIMS_WITH_ARM;

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
	if (m_pLookTarget)
		m_pLookTarget->CleanUpOldReference(&m_pLookTarget);
	m_pLookTarget = to;
	m_pLookTarget->RegisterReference((CEntity **) &m_pLookTarget);
	if (m_pSeekTarget)
		m_pSeekTarget->CleanUpOldReference(&m_pSeekTarget);
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
		m_lookTimer = 0;
	}

	if (IsPlayer())
		((CPlayerPed*)this)->m_fFPSMoveHeading = 0.0f;
}

void
CPed::AimGun(void)
{
	CVector vector;

	if (IsPlayer() && bIsDucking)
		m_pedIK.m_flags &= ~CPedIK::AIMS_WITH_ARM;

	if (m_pSeekTarget) {
		if (m_pSeekTarget->IsPed()) {
			((CPed*)m_pSeekTarget)->m_pedIK.GetComponentPosition(vector, PED_MID);
		} else {
			vector = m_pSeekTarget->GetPosition();
		}

		if (!IsPlayer())
			Say(SOUND_PED_ATTACK);

		bCanPointGunAtTarget = m_pedIK.PointGunAtPosition(vector);
		if (m_pLookTarget != m_pSeekTarget) {
			SetLookFlag(m_pSeekTarget, true, true);
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

bool
CPed::CanWeRunAndFireWithWeapon(void)
{
	return CWeaponInfo::GetWeaponInfo(GetWeapon()->m_eWeaponType)->IsFlagSet(WEAPONFLAG_CANAIM_WITHARM);
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
						}
					}
				}
			}
		} else {
			m_chatTimer = CTimer::GetTimeInMilliseconds() + 200;
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
	if (stranger->m_nPedType == PEDTYPE_COP)
		return false;
	if (stranger->IsPlayer())
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
		|| FindPlayerPed() != this || !CanStrafeOrMouseControl()) {

		if (FindPlayerPed() == this)
			FindPlayerPed()->m_fWalkAngle = 0.0f;
		return;
	}

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
	if(!fightAssoc)
		fightAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_IDLE_TIRED);

	if(!fightAssoc)
		fightAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_MELEE_IDLE_FIGHTMODE);

	if ((!idleAssoc || idleAssoc->blendAmount < 0.5f) && !fightAssoc && !bIsDucking) {
		LimbOrientation newUpperLegs;
		newUpperLegs.yaw = localWalkAngle;

		if (newUpperLegs.yaw < -DEGTORAD(100.0f)) {
			newUpperLegs.yaw += PI;
		} else if (newUpperLegs.yaw > DEGTORAD(100.0f)) {
			newUpperLegs.yaw -= PI;
		}

		if (newUpperLegs.yaw > -DEGTORAD(50.0f) && newUpperLegs.yaw < DEGTORAD(50.0f)) {
			newUpperLegs.pitch = 0.1f;
			RwV3d Xaxis = { 1.0f, 0.0f, 0.0f };
			RwV3d Zaxis = { 0.0f, 0.0f, 1.0f };
			RtQuatRotate(&m_pFrames[PED_UPPERLEGL]->hanimFrame->q, &Zaxis, RADTODEG(newUpperLegs.pitch), rwCOMBINEPOSTCONCAT);
			RtQuatRotate(&m_pFrames[PED_UPPERLEGL]->hanimFrame->q, &Xaxis, RADTODEG(newUpperLegs.yaw), rwCOMBINEPOSTCONCAT);
			RtQuatRotate(&m_pFrames[PED_UPPERLEGR]->hanimFrame->q, &Zaxis, RADTODEG(newUpperLegs.pitch), rwCOMBINEPOSTCONCAT);
			RtQuatRotate(&m_pFrames[PED_UPPERLEGR]->hanimFrame->q, &Xaxis, RADTODEG(newUpperLegs.yaw), rwCOMBINEPOSTCONCAT);
			bDontAcceptIKLookAts = true;
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
			angle = HALFPI;
		else if (leftRight > 0.0f)
			angle = -HALFPI;
	}

	return CGeneral::LimitRadianAngle(offset + angle);
}

void
CPed::UpdatePosition(void)
{
	if (CReplay::IsPlayingBack() || !bIsStanding || m_attachedTo)
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
	if (m_pCurrentPhysSurface && (!m_pCurrentPhysSurface->bInfiniteMass || m_pCurrentPhysSurface->m_phy_flagA08)) {
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
	m_fleeFromPos = CVector2D(0.0f, 0.0f);
	m_fleeFrom = nil;
	m_fleeTimer = 0;
	m_threatEx = nil;
	bUsesCollision = true;
	ClearPointGunAt();
	bIsPointingGunAt = false;
	bRenderPedInCar = true;
	bKnockedUpIntoAir = false;
	bKnockedOffBike = false;
	m_pCollidingEntity = nil;
}

void
CPed::ProcessBuoyancy(void)
{
	float buoyancyLevel = 1.1f;
	static uint32 nGenerateRaindrops = 0;
	static uint32 nGenerateWaterCircles = 0;
	CRGBA color;

	if (bInVehicle)
		return;

	CVector buoyancyPoint;
	CVector buoyancyImpulse;

	if (DyingOrDead())
		buoyancyLevel = 1.8f;

	if (mod_Buoyancy.ProcessBuoyancy(this, GRAVITY * m_fMass * buoyancyLevel, &buoyancyPoint, &buoyancyImpulse)) {
		bTouchingWater = true;
		CEntity *entity;
		CColPoint point;
		if (CWorld::ProcessVerticalLine(GetPosition(), GetPosition().z - 3.0f, point, entity, false, true, false, false, false, false, nil)
			&& entity->IsVehicle() && ((CVehicle*)entity)->IsBoat() && !entity->bRenderScorched) {
			bIsInWater = false;
			return;
		}
		color.r = (0.5f * CTimeCycle::GetDirectionalRed() + CTimeCycle::GetAmbientRed()) * 127.5f;
		color.g = (0.5f * CTimeCycle::GetDirectionalBlue() + CTimeCycle::GetAmbientBlue()) * 127.5f;
		color.b = (0.5f * CTimeCycle::GetDirectionalGreen() + CTimeCycle::GetAmbientGreen()) * 127.5f;
		color.a = CGeneral::GetRandomNumberInRange(48.0f, 96.0f);
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
						CParticleObject::AddObject(POBJECT_PED_WATER_SPLASH, pos, CVector(0.0f, 0.0f, 0.0f), 0.0f, 50, color, true);
						m_vecMoveSpeed = CVector(0.0f, 0.0f, 0.0f);
						SetPedState(PED_IDLE);
						return;
					}
				}
			}
		}
		float speedMult = 0.0f;
		if (buoyancyImpulse.z / m_fMass > GRAVITY * CTimer::GetTimeStep()
			|| mod_Buoyancy.m_waterlevel > GetPosition().z + 0.6f) {
			speedMult = pow(0.9f, CTimer::GetTimeStep());
			m_vecMoveSpeed.x *= speedMult;
			m_vecMoveSpeed.y *= speedMult;
			m_vecMoveSpeed.z *= speedMult;
			bIsStanding = false;
			bIsDrowning = true;
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
				CVector aBitForward = 2.2f * m_vecMoveSpeed + GetPosition();
				float level = 0.0f;
				if (CWaterLevel::GetWaterLevel(aBitForward, &level, false))
					aBitForward.z = level;

				CParticleObject::AddObject(POBJECT_PED_WATER_SPLASH, aBitForward, CVector(0.0f, 0.0f, 0.1f), 0.0f, 200, color, true);
				nGenerateRaindrops = CTimer::GetTimeInMilliseconds() + 80;
				nGenerateWaterCircles = CTimer::GetTimeInMilliseconds() + 100;
			}
		}
		if (nGenerateWaterCircles && CTimer::GetTimeInMilliseconds() >= nGenerateWaterCircles) {
			CVector pos = GetPosition();
			float level = 0.0f;
			if (CWaterLevel::GetWaterLevel(pos, &level, false))
				pos.z = level;

			if (pos.z != 0.0f) {
				nGenerateWaterCircles = 0;
				for(int i = 0; i < 4; i++) {
					pos.x += CGeneral::GetRandomNumberInRange(-0.75f, 0.75f);
					pos.y += CGeneral::GetRandomNumberInRange(-0.75f, 0.75f);
					CParticle::AddParticle(PARTICLE_RAIN_SPLASH_BIGGROW, pos, CVector(0.0f, 0.0f, 0.0f), nil, 0.0f, color, 0, 0, 0, 0);
				}
			}
		}
		if (nGenerateRaindrops && CTimer::GetTimeInMilliseconds() >= nGenerateRaindrops) {
			CVector pos = GetPosition();
			float level = 0.0f;
			if (CWaterLevel::GetWaterLevel(pos, &level, false))
				pos.z = level;

			if (pos.z >= 0.0f) {
				pos.z += 0.25f;
				nGenerateRaindrops = 0;
				CParticleObject::AddObject(POBJECT_SPLASHES_AROUND, pos, CVector(0.0f, 0.0f, 0.0f), 4.5f, 1500, CRGBA(0,0,0,0), true);
			}
		}
	} else
		bTouchingWater = false;
}

void
CPed::ProcessControl(void)
{
	CColPoint foundCol;
	CEntity *foundEnt = nil;

	if (CTimer::GetFrameCounter() + m_randomSeed % 32 == 0)
		PruneReferences();

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
	bDonePositionOutOfCollision = false;
	BuildPedLists();
	bIsInWater = false;
	bIsDrowning = false;
	ProcessBuoyancy();

	if (m_nPedState != PED_ARRESTED) {
		if (m_nPedState == PED_DEAD) {
			DeadPedMakesTyresBloody();
			if (CGame::nastyGame && !bIsInWater) {
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
										&& camMode != CCam::MODE_CAMERA
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

			if (bIsInWater) {
				bIsStanding = false;
				bWasStanding = false;
				CPhysical::ProcessControl();
			}
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
			SetDie();

		if (bIsStanding)
			bPushedAlongByCar = false;

		bCollidedWithMyVehicle = false;

		CEntity *collidingEnt = m_pDamageEntity;
		if (!bUsesCollision || ((!collidingEnt || m_fDamageImpulse <= 0.0f) && (!IsPlayer() || !bIsStuck)) || m_nPedState == PED_DIE) {	
			bHitSomethingLastFrame = false;
			if (m_nPedStateTimer <= 500 && bIsInTheAir) {
				if (m_nPedStateTimer)
					m_nPedStateTimer--;
			} else if (m_nPedStateTimer < 1001) {
				m_nPedStateTimer = 0;
			}
		} else if (!GetPedAttractorManager()->IsInQueue(this, m_attractor)) {
			if (m_panicCounter == 50 && IsPedInControl()) {
				SetWaitState(WAITSTATE_STUCK, nil);
				// Leftover
				/*
				if (m_nPedType < PEDTYPE_COP) {

				} else {

				}
				*/
			} else if (collidingEnt) {
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

					if (m_nPedState == PED_FOLLOW_PATH) {
						if (DotProduct(m_vecDamageNormal, GetForward()) < -0.866f && CanPedJumpThis(collidingEnt, &m_vecDamageNormal)) {
							SetJump();
						}
						break;
					}

					if (m_pedInObjective &&
						(m_objective == OBJECTIVE_GOTO_CHAR_ON_FOOT || m_objective == OBJECTIVE_KILL_CHAR_ON_FOOT)) {

						if (m_pedInObjective->IsPlayer()
							&& (neededTurn < DEGTORAD(20.0f) || m_panicCounter > 10)) {
							if (CanPedJumpThis(collidingEnt)) {
								SetJump();
							} else if (m_objective == OBJECTIVE_KILL_CHAR_ON_FOOT) {
								if (m_nPedType == PEDTYPE_COP && m_nWaitState != WAITSTATE_LOOK_ABOUT)
									((CCopPed*)this)->field_624++;

								SetWaitState(WAITSTATE_LOOK_ABOUT, nil);
							} else {
								SetWaitState(WAITSTATE_PLAYANIM_TAXI, nil);
								m_headingRate = 0.0f;
								SetLookFlag(m_pedInObjective, true);
								SetLookTimer(3000);
								Say(SOUND_PED_TAXI_CALL);
							}
						} else {
							if (m_pLookTarget)
								m_pLookTarget->CleanUpOldReference(&m_pLookTarget);

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

								if (m_nWaitState != WAITSTATE_HITWALL)
									SetWaitState(WAITSTATE_TURN180, nil);
								m_nPedStateTimer = CTimer::GetTimeInMilliseconds() + 5000;
								Flee();
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
												CVector bestCoords = CPathFind::TakeWidthIntoAccountForWandering(m_pNextPathNode, m_randomSeed);
												angleToFace = CGeneral::GetRadianAngleBetweenPoints(
													bestCoords.x, bestCoords.y,
													GetPosition().x, GetPosition().y);

											} else if (m_nPedState == PED_FOLLOW_PATH) {
												CVector bestCoords = m_pathNodesToGo[m_nCurPathNodeId]->GetPosition();
												angleToFace = CGeneral::GetRadianAngleBetweenPoints(
													bestCoords.x, bestCoords.y,
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
													posToHead.z = ThePaths.m_pathNodes[closestNodeId].GetZ();
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

					float oldHealth = m_fHealth;
					bool playerSufferSound = false;

					if (collidingVehSpeedSqr <= 1.0f / 400.0f) {
						if (IsPedInControl()
							&& (!IsPlayer()
								|| m_objective == OBJECTIVE_GOTO_AREA_ON_FOOT
								|| m_objective == OBJECTIVE_RUN_TO_AREA
								|| m_objective == OBJECTIVE_SPRINT_TO_AREA
								|| m_objective == OBJECTIVE_ENTER_CAR_AS_DRIVER
								|| IsUseAttractorObjective(m_objective))) {

							if (collidingVeh != m_pCurrentPhysSurface || IsPlayer()) {
								if (!bVehEnterDoorIsBlocked) {
									if (collidingVeh->GetStatus() != STATUS_PLAYER || CharCreatedBy == MISSION_CHAR) {

										if (m_nPedState == PED_SEEK_CAR) {
											SetDirectionToWalkAroundObject(collidingVeh);
											CWorld::Players[CWorld::PlayerInFocus].m_nLastBumpPlayerCarTimer = m_nPedStateTimer;
										} else {
											SetDirectionToWalkAroundVehicle(collidingVeh);
										}
									} else {
										if (CTimer::GetTimeInMilliseconds() >= CWorld::Players[CWorld::PlayerInFocus].m_nLastBumpPlayerCarTimer
											|| m_nPedStateTimer >= CTimer::GetTimeInMilliseconds()) {

											if (m_nPedState == PED_SEEK_CAR) {
												SetDirectionToWalkAroundObject(collidingVeh);
												CWorld::Players[CWorld::PlayerInFocus].m_nLastBumpPlayerCarTimer = m_nPedStateTimer;
											} else {
												SetDirectionToWalkAroundVehicle(collidingVeh);
											}
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
								uint32 weaponSlot = CWeaponInfo::GetWeaponInfo(weaponType)->m_nWeaponSlot;
								if (weaponType == WEAPONTYPE_UNARMED
									|| weaponSlot == 3
									|| weaponSlot == 5
									|| weaponSlot == 1) {
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
						
						if (m_pCollidingEntity != collidingEnt)
							bPushedAlongByCar = true;
					}
					if (m_fHealth < oldHealth && playerSufferSound)
						Say(SOUND_PED_HIT);

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
										eWeaponType weaponType = player->GetWeapon()->m_eWeaponType;
										uint32 weaponSlot = CWeaponInfo::GetWeaponInfo(weaponType)->m_nWeaponSlot;
										if (weaponType == WEAPONTYPE_UNARMED
											|| weaponSlot == 3
											|| weaponSlot == 5
											|| weaponSlot == 1) {
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
			if (!bIsInTheAir && m_nPedState != PED_JUMP && m_fDamageImpulse > 0.0f) {
				forceDir = m_vecDamageNormal;
				forceDir.z = 0.0f;
				if (!bIsStanding) {
					forceDir *= 4.0f;
				} else {
					forceDir *= 0.5f;
				}

				ApplyMoveForce(forceDir);
			}
			if ((bIsInTheAir && !DyingOrDead()) || (!bIsStanding && !bWasStanding && m_nPedState == PED_FALL)) {
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
						offsetToCheck.z = 1.0f;
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

						if (flyDir != 0 && !bHeadStuckInCollision) {
							SetPosition((flyDir == 2 ? obstacleForFlyingOtherDir.point : obstacleForFlying.point));
							GetMatrix().GetPosition().z += FEET_OFFSET;
							GetMatrix().UpdateRW();
							SetLanding();
							bIsStanding = true;
						}
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
						if (m_nPedState == PED_JUMP) {
							if (m_nWaitTimer <= 2000) {
								if (m_nWaitTimer < 1000)
									m_nWaitTimer += CTimer::GetTimeStepInMilliseconds();
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
						if (!bHeadStuckInCollision || FEET_OFFSET + foundCol.point.z < GetPosition().z) {
							GetMatrix().GetPosition().z = FEET_OFFSET + foundCol.point.z;
							GetMatrix().UpdateRW();
							if (bHeadStuckInCollision)
								bHeadStuckInCollision = false;
						}
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
				bStartWanderPathOnFoot = false;
			}
		}

		if (!bIsStanding && m_vecMoveSpeed.z > 0.25) {
			float airResistance = Pow(0.95f, CTimer::GetTimeStep());

			m_vecMoveSpeed *= airResistance;
		}
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

		if (m_nPedState != PED_DIE || bIsPedDieAnimPlaying) {
			RequestDelayedWeapon();
			PlayFootSteps();
			if (m_nPedState != PED_DEAD) {
				CalculateNewVelocity();
				CalculateNewOrientation();
			}
			UpdatePosition();
			if (IsPedInControl() && !bIsStanding && !m_pDamageEntity) {
				if (m_attachedTo) {
					bIsInTheAir = false;
				} else if (CheckIfInTheAir()) {
					SetInTheAir();
					bHeadStuckInCollision = false;
				}
			}
			if (bHeadStuckInCollision) {
				CVector posToCheck = GetPosition();
				posToCheck.z += 0.9f;
				if (!CWorld::TestSphereAgainstWorld(posToCheck, 0.2f, this, true, true, false, true, false, false))
					bHeadStuckInCollision = false;
			}
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
					// III has these in here(and they were unused):
					/*
					WanderRange();
					CheckAroundForPossibleCollisions();
					*/
					break;
				case PED_WANDER_PATH:
					WanderPath();
					break;
				case PED_ENTER_CAR:
				case PED_CARJACK:
				{
					break;
				}
				case PED_FLEE_POS:
					ms_vec2DFleePosition = m_fleeFromPos;
					Flee();
					break;
				case PED_FLEE_ENTITY:
					if (!m_fleeFrom) {
						bMakeFleeScream = false;
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
				case PED_BUY_ICECREAM:
					BuyIceCream();
					break;
				case PED_INVESTIGATE:
					InvestigateEvent();
					break;
				case PED_ON_FIRE:
					if (IsPlayer())
						break;

					if (CTimer::GetTimeInMilliseconds() <= m_fleeTimer) {
						if (m_pFire) {
							if (m_fleeFrom) {
								ms_vec2DFleePosition = m_fleeFrom->GetPosition();
							} else {
								ms_vec2DFleePosition = m_fleeFromPos;
							}
							Flee();
						} else {
							m_nLastPedState = PED_NONE;
							SetWanderPath(0);
							SetWaitState(WAITSTATE_FINISH_FLEE, 0);
						}
					} else {
						if (m_pFire)
							m_pFire->Extinguish();
					}
					break;
				case PED_ANSWER_MOBILE:
					AnswerMobile();
					break;
				case PED_FALL:
					Fall();
					break;
				case PED_GETUP:
					SetGetUp();
					break;
#ifdef GTA_TRAIN
				case PED_ENTER_TRAIN:
					EnterTrain();
					break;
				case PED_EXIT_TRAIN:
					ExitTrain();
					break;
#endif
				case PED_DRIVING:
				{
					if (!m_pMyVehicle) {
						bInVehicle = false;
						FlagToDestroyWhenNextProcessed();
						return;
					}

#ifdef CAR_AIRBREAK
					if (IsPlayer()) {
						CPad* pad = CPad::GetPad(0);
						if (!pad->ArePlayerControlsDisabled()) {
							if (pad->GetHorn()) {
								float c = Cos(m_fRotationCur);
								float s = Sin(m_fRotationCur);
								m_pMyVehicle->GetRight() = CVector(1.0f, 0.0f, 0.0f);
								m_pMyVehicle->GetForward() = CVector(0.0f, 1.0f, 0.0f);
								m_pMyVehicle->GetUp() = CVector(0.0f, 0.0f, 1.0f);
								if (pad->GetAccelerate()) {
									m_pMyVehicle->ApplyMoveForce(GetForward() * 30.0f);
								}
								else if (pad->GetBrake()) {
									m_pMyVehicle->ApplyMoveForce(-GetForward() * 30.0f);
								}
								else {
									int16 lr = pad->GetSteeringLeftRight();
									if (lr < 0) {
										//m_pMyVehicle->ApplyTurnForce(20.0f * -GetRight(), GetForward());
										m_pMyVehicle->ApplyMoveForce(-GetRight() * 30.0f);
									}
									else if (lr > 0) {
										m_pMyVehicle->ApplyMoveForce(GetRight() * 30.0f);
									}
									else {
										m_pMyVehicle->ApplyMoveForce(0.0f, 0.0f, 50.0f);
									}
								}
							}
						}
					}
#endif

					if (m_pMyVehicle->pDriver == this) {
						DriveVehicle();
						if (!m_pMyVehicle)
							return;
					} else {
						LookForSexyPeds();
						LookForSexyCars();
					}
					
					if (!IsPlayer() && m_pMyVehicle->IsBoat()
						&& FindPlayerPed()->m_pCurrentPhysSurface == m_pMyVehicle
						&& (CharCreatedBy != MISSION_CHAR || !bIsPlayerFriend)) {
						SetObjective(OBJECTIVE_KILL_CHAR_ON_BOAT, FindPlayerPed());
						Say(SOUND_PED_CAR_JACKED);
					}

					break;
				}
				case PED_DIE:
					Die();
					break;
				case PED_HANDS_UP:
					if (m_pedStats->m_flags & STAT_GUN_PANIC) {
						if (!RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_HANDSCOWER)) {
							CAnimManager::AddAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_HANDSCOWER);
							Say(SOUND_PED_HANDS_COWER);
						}
					} else if (!RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_HANDSUP)) {
						CAnimManager::AddAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_HANDSUP);
						Say(SOUND_PED_HANDS_UP);
					}
					break;
				default:
					break;
			}
			SetMoveAnim();
			if (bPedIsBleeding || m_bleedCounter != 0) {
				if (CGame::nastyGame) {
					if (m_bleedCounter != 0)
						m_bleedCounter--;

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

			if (bHeldHostageInCar) {
				if (m_pMyVehicle && m_pMyVehicle->pDriver && m_pMyVehicle->pDriver->IsPlayer()) {
					Say(SOUND_PED_FLEE_SPRINT);
				}
			}

			if (m_delayedSoundID >= 0 && CTimer::GetTimeInMilliseconds() > m_delayedSoundTimer) {
				Say(m_delayedSoundID);
				m_delayedSoundID = -1;
				m_delayedSoundTimer = 0;
			}

			if (bFannyMagnetCheat && m_nPedType == PEDTYPE_CIVFEMALE
				&& m_pedStats->m_sexiness > 40 && !m_leader) {
				SetLeader(FindPlayerPed());
			}

		} else {
			if (bIsStanding && (!m_pCurrentPhysSurface || IsPlayer())
				|| bIsInWater || !bUsesCollision) {
				SetDead();
			}
			m_pCurrentPhysSurface = nil;
		}
	} else
		ServiceTalking();
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

	if (!bUsesCollision && !bJustCheckCollision)
		return false;

	if (collidingEnt->IsVehicle() && ((CVehicle*)collidingEnt)->IsBoat())
		collidedWithBoat = true;

	// ofc we're not vehicle
	if (!m_bIsVehicleBeingShifted && !bSkipLineCol && !collidingEnt->IsPed()) {
		if (!bCollisionProcessed) {
			m_pCurrentPhysSurface = nil;
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
					if(!bHeadStuckInCollision || FEET_OFFSET + intersectionPoint.point.z < GetPosition().z) {
						GetMatrix().GetPosition().z = FEET_OFFSET + intersectionPoint.point.z;
						if (bHeadStuckInCollision)
							bHeadStuckInCollision = false;
					}
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
					intersectionPoint, minDist, false, false, &m_collPoly);

				if (collidedWithBoat && bWasStanding && !belowTorsoCollided) {
					ourLine.p0.z = ourLine.p1.z;
					ourLine.p1.z = ourLine.p1.z + gravityEffect;
					belowTorsoCollided = CCollision::ProcessVerticalLine(ourLine, collidingEnt->GetMatrix(), *hisCol,
						intersectionPoint, minDist, false, false, &m_collPoly);
				}
				if (belowTorsoCollided) {
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

							if (!bHeadStuckInCollision || FEET_OFFSET + intersectionPoint.point.z < GetPosition().z) {
								GetMatrix().GetPosition().z = FEET_OFFSET + intersectionPoint.point.z;
								if (bHeadStuckInCollision)
									bHeadStuckInCollision = false;
							}
							m_nSurfaceTouched = intersectionPoint.surfaceB;
							if (m_nSurfaceTouched == SURFACE_STEEP_CLIFF) {
								bHitSteepSlope = true;
								m_vecDamageNormal = intersectionPoint.normal;
							}
						}

						float upperSpeedLimit = 0.33f;
						float lowerSpeedLimit = -0.25f;
						float speed = m_vecMoveSpeed.Magnitude2D();
						if (m_nPedState == PED_IDLE) {
							upperSpeedLimit *= 2.0f;
							lowerSpeedLimit *= 1.5f;
						}
						CAnimBlendAssociation *fallAnim = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_FALL);
						if (!bWasStanding && speed > upperSpeedLimit && (!bPushedAlongByCar || m_vecMoveSpeed.z < lowerSpeedLimit)
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
							if (CSurfaceTable::IsSoftLanding(intersectionPoint.surfaceB))
								damage *= 0.5f;

							InflictDamage(collidingEnt, WEAPONTYPE_FALL, damage, PEDPIECE_TORSO, dir);
							if (IsPlayer() && damage2 > 5.0f)
								Say(SOUND_PED_LAND);

						} else if (!bWasStanding && fallAnim && -0.016f * CTimer::GetTimeStep() > m_vecMoveSpeed.z) {
							InflictDamage(collidingEnt, WEAPONTYPE_FALL, 15.0f, PEDPIECE_TORSO, 2);
						}
						m_vecMoveSpeed.z = 0.0f;
						bIsStanding = true;
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
				if (sphereNormal.z >= -1.0f || !IsPlayer()) {
					normalLength = sphereNormal.Magnitude2D();
					if (normalLength != 0.0f) {
						sphereNormal.x = sphereNormal.x / normalLength;
						sphereNormal.y = sphereNormal.y / normalLength;
					}
				} else {
					float speed = m_vecMoveSpeed.Magnitude2D();
					sphereNormal.x = -m_vecMoveSpeed.x / Max(0.001f, speed);
					sphereNormal.y = -m_vecMoveSpeed.y / Max(0.001f, speed);
					GetMatrix().GetPosition().z -= 0.05f;
					bHeadStuckInCollision = true;
				}
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
	for (int i = 0; i < times; i++) {
		CVector adjustedPos = pos;
		adjustedPos.x += CGeneral::GetRandomNumberInRange(-0.1f, 0.1f);
		adjustedPos.y += CGeneral::GetRandomNumberInRange(-0.1f, 0.1f);

		CVector direction = ped->GetForward() * -0.05f;
		CParticle::AddParticle(PARTICLE_RAIN_SPLASHUP, adjustedPos, direction, nil, size, CRGBA(32, 32, 32, 32), 0, 0, CGeneral::GetRandomNumber() & 1, 200);
	}
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
		case SURFACE_SAND_BEACH:
		case SURFACE_CONCRETE_BEACH:
			for (int i = 0; i < times; ++i) {
				CVector adjustedPos = pos;
				adjustedPos.x += CGeneral::GetRandomNumberInRange(-0.1f, 0.1f);
				adjustedPos.y += CGeneral::GetRandomNumberInRange(-0.1f, 0.1f);
				// ??
				CGeneral::GetRandomNumber();
				CGeneral::GetRandomNumber();
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
	CAnimBlendAssociation *assoc = RpAnimBlendClumpGetFirstAssociation(GetClump());
	CAnimBlendAssociation *walkRunAssoc = nil;
	float walkRunAssocBlend = 0.0f, idleAssocBlend = 0.0f;
	bool isSkater = m_pedStats == CPedStats::ms_apPedStats[PEDSTAT_SKATER];

	CVector footPosL(0.0f, 0.0f, 0.0f), footPosR(0.0f, 0.0f, 0.0f);
	bool footPosLok = false, footPosRok = false;

	if (bDoBloodyFootprints) {
		if (m_bloodyFootprintCountOrDeathTime > 0 && m_bloodyFootprintCountOrDeathTime < 300) {
			m_bloodyFootprintCountOrDeathTime--;

			if (m_bloodyFootprintCountOrDeathTime == 0)
				bDoBloodyFootprints = false;
		}
	}

	if (!bIsStanding)
		return;

	for (; assoc; assoc = RpAnimBlendGetNextAssociation(assoc)) {
		if (assoc->flags & ASSOC_WALK) {
			walkRunAssoc = assoc;
			walkRunAssocBlend += assoc->blendAmount;
		} else if ((assoc->flags & ASSOC_NOWALK) == 0) {
			idleAssocBlend += assoc->blendAmount;
		}
	}

	if (walkRunAssoc && walkRunAssocBlend > 0.5f && idleAssocBlend < 1.0f) {

		float stepStart = 1.f / 15.f;
		float stepEnd = walkRunAssoc->hierarchy->totalLength / 2.0f + stepStart;
		float currentTime = walkRunAssoc->currentTime;

		if (isSkater) {
			// both are unused
			static float stepStartSection = 1.0f;
			static float animSections = 15.f;

			float moveStart, soundVolume, skateTime;
			if (walkRunAssoc->animId == ANIM_STD_WALK) {
				moveStart = 0.0f;
				skateTime = 8.f / 15.f;
			} else {
				moveStart = 0.0f;
				skateTime = 5.f / 15.f;
			}
			switch (CSurfaceTable::GetAdhesionGroup(m_nSurfaceTouched)) {
				case ADHESIVE_LOOSE:
					if (CGeneral::GetRandomNumber() % 128) {
						m_vecAnimMoveDelta *= 0.5f;
					} else {
						SetFall(0, ANIM_STD_HIGHIMPACT_BACK, false);
					}
					soundVolume = 0.5f;
					break;
				case ADHESIVE_SAND:
					if (CGeneral::GetRandomNumber() % 64) {
						m_vecAnimMoveDelta *= 0.2f;
					} else {
						SetFall(0, ANIM_STD_HIGHIMPACT_BACK, false);
					}
					soundVolume = 0.2f;
					break;
				case ADHESIVE_WET:
					m_vecAnimMoveDelta *= 0.3f;
					soundVolume = 0.2f;
					break;
				default:
					soundVolume = 1.f;
					break;
			}
			if (soundVolume > 0.2f && currentTime > moveStart && currentTime - walkRunAssoc->timeStep <= moveStart) {
				DMAudio.PlayOneShot(m_audioEntityId, SOUND_SKATING, ((int)(127.f * soundVolume) | (walkRunAssoc->animId << 8)));
			} else if (soundVolume > 0.2f) {
				if (currentTime > skateTime && currentTime - walkRunAssoc->timeStep <= skateTime) {
					DMAudio.PlayOneShot(m_audioEntityId, SOUND_SKATING, ((int)(127.f * soundVolume) | (walkRunAssoc->animId << 8)));
				}
			}

		} else {
			int stepPart = 0;
			
			// This section is shortened/optimized for sanity.

			if (currentTime >= stepStart && currentTime - walkRunAssoc->timeStep < stepStart)
				stepPart = 1;
			else if (currentTime >= stepEnd && currentTime - walkRunAssoc->timeStep < stepEnd)
				stepPart = 2;

			if (stepPart != 0) {
				CVector adjustedFootPos;
				if (stepPart == 1) {
					DMAudio.PlayOneShot(m_audioEntityId, SOUND_STEP_START, 1.0f);
					TransformToNode(footPosL, PED_FOOTL);
					footPosLok = true;
					adjustedFootPos = footPosL;
				} else {
					DMAudio.PlayOneShot(m_audioEntityId, SOUND_STEP_END, 1.0f);
					TransformToNode(footPosR, PED_FOOTR);
					footPosRok = true;
					adjustedFootPos = footPosR;
				}

				CVector forward = GetForward();

				adjustedFootPos.z -= 0.1f;
				adjustedFootPos += 0.2f * forward;

				if (bDoBloodyFootprints) {
					CVector2D top(forward * 0.26f);
					CVector2D right(GetRight() * (stepPart == 1 ? 0.14f : 0.1f));

					CShadows::AddPermanentShadow(SHADOWTYPE_DARK, gpBloodPoolTex, &adjustedFootPos,
						top.x, top.y,
						right.x, right.y,
						255, 255, 0, 0, 4.0f, 3000, 1.0f);

					if (m_bloodyFootprintCountOrDeathTime <= 20) {
						m_bloodyFootprintCountOrDeathTime = 0;
						bDoBloodyFootprints = false;
					} else {
						m_bloodyFootprintCountOrDeathTime -= 20;
					}
				}
				if (m_nSurfaceTouched == SURFACE_SAND || m_nSurfaceTouched == SURFACE_SAND_BEACH) {
					CVector2D top(forward * -0.26f);
					CVector2D right(GetRight() * (stepPart == 1 ? 0.1f : 0.14f));

					CShadows::AddPermanentShadow(SHADOWTYPE_DARK, gpShadowPedTex, &adjustedFootPos,
						top.x, top.y,
						right.x, right.y,
						120, 250, 250, 50, 4.0f, 5000, 1.0f);
				}
				if (CWeather::Rain <= 0.1f || CCullZones::CamNoRain() || CCullZones::PlayerNoRain()) {
					if (IsPlayer())
						particleProduceFootDust(this, adjustedFootPos, 0.0f, 4);

				} else if (stepPart == 2) {
					particleProduceFootSplash(this, adjustedFootPos, 0.15f, 4);
				}
			}
		}
	}

	if (IsPlayer() && !walkRunAssoc && bIsLanding) {
		if (!footPosLok)
			TransformToNode(footPosL, PED_FOOTL);

		CVector forward = GetForward();

		CVector adjustedFootPosL = footPosL;
		adjustedFootPosL.z -= 0.1f;
		adjustedFootPosL += 0.2f * forward;
		if (bDoBloodyFootprints) {
			CVector2D top(forward * 0.26f);
			CVector2D right(GetRight() * 0.14f);

			CShadows::AddPermanentShadow(SHADOWTYPE_DARK, gpBloodPoolTex, &adjustedFootPosL,
				top.x, top.y,
				right.x, right.y,
				255, 255, 0, 0, 4.0f, 3000, 1.0f);

			if (m_bloodyFootprintCountOrDeathTime <= 20) {
				m_bloodyFootprintCountOrDeathTime = 0;
				bDoBloodyFootprints = false;
			}
			else {
				m_bloodyFootprintCountOrDeathTime -= 20;
			}
		}
		if (!isSkater) {
			if (m_nSurfaceTouched == SURFACE_SAND || m_nSurfaceTouched == SURFACE_SAND_BEACH) {
				CVector2D top(forward * -0.26f);
				CVector2D right(GetRight() * 0.14f);

				CShadows::AddPermanentShadow(SHADOWTYPE_DARK, gpShadowPedTex, &adjustedFootPosL,
					top.x, top.y,
					right.x, right.y,
					120, 250, 250, 50, 4.0f, 5000, 1.0f);
			}
		}
		if(!footPosRok)
			TransformToNode(footPosR, PED_FOOTR);

		CVector adjustedFootPosR = footPosR;
		adjustedFootPosR.z -= 0.1f;
		adjustedFootPosR += 0.2f * forward;

		if (bDoBloodyFootprints) {
			CVector2D top(forward * 0.26f);
			CVector2D right(GetRight() * 0.1f);

			CShadows::AddPermanentShadow(SHADOWTYPE_DARK, gpBloodPoolTex, &adjustedFootPosR,
				top.x, top.y,
				right.x, right.y,
				255, 255, 0, 0, 4.0f, 3000, 1.0f);

			if (m_bloodyFootprintCountOrDeathTime <= 20) {
				m_bloodyFootprintCountOrDeathTime = 0;
				bDoBloodyFootprints = false;
			} else {
				m_bloodyFootprintCountOrDeathTime -= 20;
			}
		}
		if (!isSkater) {
			if (m_nSurfaceTouched == SURFACE_SAND || m_nSurfaceTouched == SURFACE_SAND_BEACH) {
				CVector2D top(forward * -0.26f);
				CVector2D right(GetRight() * 0.14f);

				CShadows::AddPermanentShadow(SHADOWTYPE_DARK, gpShadowPedTex, &adjustedFootPosR,
					top.x, top.y,
					right.x, right.y,
					120, 250, 250, 50, 4.0f, 5000, 1.0f);
			}
		}
	}

	if (m_nSurfaceTouched == SURFACE_WATER) {
		CRGBA rubberSmokeColor(255, 255, 255, 196);
		float pedSpeed = CVector2D(m_vecMoveSpeed).Magnitude();
		if (pedSpeed > 0.03f && CTimer::GetFrameCounter() % 2 == 0 && pedSpeed > 0.13f) {
			float particleSize = pedSpeed * 2.0f;

			if (particleSize < 0.25f)
				particleSize = 0.25f;

			if (particleSize > 0.75f)
				particleSize = 0.75f;

			CVector particlePos = GetPosition() + GetForward() * 0.3f;
			particlePos.z -= 1.2f;

			CVector particleDir = m_vecMoveSpeed * -0.75f;

			particleDir.z = CGeneral::GetRandomNumberInRange(0.01f, 0.03f);
			CParticle::AddParticle(PARTICLE_CAR_SPLASH, particlePos, particleDir, nil, 0.5f * particleSize, CRGBA(0,0,0,0), 0, 0, 0, 0);
			CParticle::AddParticle(PARTICLE_RUBBER_SMOKE, particlePos, particleDir, nil, particleSize, rubberSmokeColor, 0, 0, 0, 0);
		}

		if (m_nPedState == PED_JUMP) {
			CVector particlePos = GetPosition();
			particlePos.z -= 0.1f;

			CVector particleDir(0.0f, 0.075f, 0.0f);
			CParticle::AddParticle(PARTICLE_CAR_SPLASH, particlePos, particleDir, nil, 0.005f, CRGBA(0, 0, 0, 0), 0, 0, 0, 0);
			particleDir.x += CGeneral::GetRandomNumberInRange(-0.05f, 0.05f);
			particleDir.y += CGeneral::GetRandomNumberInRange(-0.05f, 0.05f);
			particleDir.z -= CGeneral::GetRandomNumberInRange(0.025f, 0.05f);
			CParticle::AddParticle(PARTICLE_RUBBER_SMOKE, particlePos, particleDir, nil, 0.5f, rubberSmokeColor, 0, 0, 0, 0);
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

bool
CPed::SetDirectionToWalkAroundVehicle(CVehicle* veh)
{
	return SetFollowPath(m_vecSeekPos, 0.0f, m_nMoveState, veh, m_pedInObjective, m_nMoveState == PEDMOVE_WALK ? 2000 : 250);
}

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

	if (CharCreatedBy != MISSION_CHAR && obj->GetModelIndex() == MI_PHONEBOOTH1) {
		bool isRunning = m_nMoveState == PEDMOVE_RUN || m_nMoveState == PEDMOVE_SPRINT;
		SetFindPathAndFlee(obj, 5000, !isRunning);
		return;
	}

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
	float angleToFaceObjCenter = (objColCenter - GetPosition()).Heading();
	float angleDiffBtwObjCenterAndForward = CGeneral::LimitRadianAngle(dirToSet - angleToFaceObjCenter);
	float objTopRightHeading = Atan2(adjustedColMax.x - adjustedColMin.x, adjustedColMax.y - adjustedColMin.y);

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

		if (!obj->IsVehicle() || objUpsideDown) {
			collidingThingChanged = false;
		} else {
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
		}

		if (!entityOnTopLeftOfObj && !entityOnTopRightOfObj) {
			CVector topLeftCorner(adjustedColMin.x - 0.3f, adjustedColMax.y + 0.3f, 0.0f);
			topLeftCorner = objMat * topLeftCorner;
			CVector topRightCorner(adjustedColMax.x + 0.3f, adjustedColMax.y + 0.3f, 0.0f);
			topRightCorner = objMat * topRightCorner;
			CColPoint foundCol;
			CEntity *foundEnt;
			if (CWorld::ProcessLineOfSight(topLeftCorner, topRightCorner, foundCol, foundEnt, true, true, false, true, false, false, false, false)) {
				switch (foundEnt->GetType()) {
				case ENTITY_TYPE_VEHICLE:
					entityOnTopRightOfObj = 2;
					entityOnTopLeftOfObj = 2;
					break;
				case ENTITY_TYPE_BUILDING:
					entityOnTopRightOfObj = 1;
					entityOnTopLeftOfObj = 1;
					break;
				case ENTITY_TYPE_OBJECT:
					entityOnTopRightOfObj = 3;
					entityOnTopLeftOfObj = 3;
					break;
				}
			}
		}
		if (!entityOnBottomRightOfObj && !entityOnBottomLeftOfObj) {
			CVector bottomRightCorner(adjustedColMax.x + 0.3f, adjustedColMin.y - 0.3f, 0.0f);
			bottomRightCorner = objMat * bottomRightCorner;
			CVector bottomLeftCorner(adjustedColMin.x - 0.3f, adjustedColMin.y - 0.3f, 0.0f);
			bottomLeftCorner = objMat * bottomLeftCorner;
			CColPoint foundCol;
			CEntity* foundEnt;
			if (CWorld::ProcessLineOfSight(bottomRightCorner, bottomLeftCorner, foundCol, foundEnt, true, true, false, true, false, false, false, false)) {
				switch (foundEnt->GetType()) {
				case ENTITY_TYPE_VEHICLE:
					entityOnBottomLeftOfObj = 2;
					entityOnBottomRightOfObj = 2;
					break;
				case ENTITY_TYPE_BUILDING:
					entityOnBottomLeftOfObj = 1;
					entityOnBottomRightOfObj = 1;
					break;
				case ENTITY_TYPE_OBJECT:
					entityOnBottomLeftOfObj = 3;
					entityOnBottomRightOfObj = 3;
					break;
				}
			}
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
	}
	m_collidingEntityWhileFleeing = obj;
	m_collidingEntityWhileFleeing->RegisterReference((CEntity**) &m_collidingEntityWhileFleeing);

	// TODO: This random may need to be changed.
	m_collidingThingTimer = CTimer::GetTimeInMilliseconds() + 512 + CGeneral::GetRandomNumber();

	CVector localPosToHead;

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
		m_nPedState == PED_ATTACK || m_nPedState == PED_FIGHT || m_nPedState == PED_AIM_GUN || m_nPedState == PED_JUMP || m_nPedState == PED_ANSWER_MOBILE;
}

void
CPed::PedSetPreviousStateCB(CAnimBlendAssociation* assoc, void* arg)
{
	CPed* ped = (CPed*)arg;
	ped->RestorePreviousState();
	ped->m_pVehicleAnim = nil;
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

	if (ped->bFleeWhenStanding && ped->m_threatEx) {
		ped->SetFlee(ped->m_threatEx, 10000);
		ped->Say(SOUND_PED_FLEE_SPRINT);
		ped->bFleeWhenStanding = false;
		ped->m_threatEx = nil;

	} else if (ped->bGotUpOfMyOwnAccord) {
		ped->SetObjective(OBJECTIVE_NONE);
		ped->SetWanderPath(CGeneral::GetRandomNumberInRange(0.f, 8.f));
		ped->bGotUpOfMyOwnAccord = false;

	} else {
		if (ped->m_nPedState != PED_FLEE_POS && ped->m_nPedState != PED_FLEE_ENTITY)
			ped->SetMoveState(PEDMOVE_STILL);
		else
			ped->SetMoveState(PEDMOVE_RUN);
		ped->SetMoveAnim();
	}

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
	else if (ped->m_objective == OBJECTIVE_LEAVE_CAR_AND_DIE) {
		ped->m_fHealth = 0.0f;
		ped->SetDie(ANIM_STD_HIT_FLOOR, 4.0f, 0.5f);
	}

	ped->bInVehicle = false;
	if (veh && (veh->IsCar() || veh->IsBike())) {
		CWorld::pIgnoreEntity = veh;
		if (CWorld::TestSphereAgainstWorld(ped->GetPosition() - CVector(0.f, 0.f, 0.2f),
				0.4f, veh, true, true, false, false, false, false)
			|| CWorld::TestSphereAgainstWorld(ped->GetPosition() + CVector(0.f, 0.f, 0.2f),
				0.4f, veh, true, true, false, false, false, false)
			|| !CWorld::GetIsLineOfSightClear(veh->GetPosition(), ped->GetPosition(), true, false, false, true, false, false, false)) {
			CWorld::pIgnoreEntity = nil;
			ped->PositionPedOutOfCollision();
		}
		CWorld::pIgnoreEntity = nil;
	}

	if (ped->m_nPedState == PED_EXIT_CAR) {
		if (ped->m_nPedType == PEDTYPE_COP) {
			ped->SetIdle();
			if (((CCopPed*)ped)->m_nCopType == COP_MIAMIVICE && ped->m_pMyVehicle && ped->m_pMyVehicle->pDriver == ped) {
				DMAudio.PlayOneShot(ped->m_audioEntityId, SOUND_PED_MIAMIVICE_EXITING_CAR, 0.f);
			}
		} else
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
	} else if (ped->m_nPedState == PED_DRIVING) {
		ped->m_nPedState = PED_IDLE;
	}

	if (animAssoc)
		animAssoc->blendDelta = -1000.0f;

	ped->RestartNonPartialAnims();
	ped->m_pVehicleAnim = nil;
	ped->m_vecMoveSpeed = CVector(0.0f, 0.0f, 0.0f);
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
		if (veh && veh->IsBike())
			// BUG? 
			veh->m_nGettingOutFlags &= ~GetBikeDoorFlag(ped->m_vehDoor);
		else
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
			if (veh->IsBike()) {
				if (Abs(veh->m_vecMoveSpeed.x) < 0.1 && Abs(veh->m_vecMoveSpeed.y) < 0.1f) {
					((CBike*)veh)->bIsStanding = true;
				}
			}
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
	ped->bHeldHostageInCar = false;
}

void
CPed::PedSetDraggedOutCarCB(CAnimBlendAssociation *dragAssoc, void *arg)
{
	CAnimBlendAssociation *quickJackedAssoc;
	CVehicle *vehicle; 
	CPed *ped = (CPed*)arg;

	uint8 exitFlags = 0;
	quickJackedAssoc = RpAnimBlendClumpGetAssociation(ped->GetClump(), ANIM_STD_QUICKJACKED);
	if (dragAssoc && dragAssoc->animId == ANIM_BIKE_HIT && ped->m_pMyVehicle) {
		if (ped->m_vehDoor == CAR_DOOR_LF || ped->m_vehDoor == CAR_DOOR_RF) {
			CAnimManager::BlendAnimation(ped->GetClump(), ASSOCGRP_STD, ANIM_STD_BIKE_FALLOFF, 100.0f);
			ped->m_pMyVehicle->m_nGettingOutFlags &= ~(CAR_DOOR_FLAG_RF | CAR_DOOR_FLAG_LF);
		} else {
			CAnimManager::BlendAnimation(ped->GetClump(), ASSOCGRP_STD, ANIM_STD_BIKE_FALLBACK, 100.0f);
			ped->m_pMyVehicle->m_nGettingOutFlags &= ~(CAR_DOOR_FLAG_RR | CAR_DOOR_FLAG_LR);
		}
		((CBike*)ped->m_pMyVehicle)->KnockOffRider(WEAPONTYPE_UNIDENTIFIED, 0, ped, true);
		return;
	}

	if (ped->m_nPedState != PED_ARRESTED) {
		ped->m_nLastPedState = PED_NONE;
		if (dragAssoc)
			dragAssoc->blendDelta = -1000.0f;
	}
	ped->RestartNonPartialAnims();
	ped->m_pVehicleAnim = nil;
	ped->m_pSeekTarget = nil;
	vehicle = ped->m_pMyVehicle;

	if (vehicle && vehicle->IsBike())
		exitFlags = GetBikeDoorFlagInclJumpInFromFront(ped->m_vehDoor);
	else
		exitFlags = GetCarDoorFlag(ped->m_vehDoor);

	if (vehicle)
		vehicle->m_nGettingOutFlags &= ~exitFlags;

	if (vehicle) {
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

	if (ped->m_objective == OBJECTIVE_LEAVE_CAR_AND_DIE) {
		dragAssoc->SetDeleteCallback(PedSetDraggedOutCarPositionCB, ped);
		ped->m_fHealth = 0.0f;
		ped->SetDie(ANIM_STD_HIT_FLOOR, 1000.0f, 0.5f);
		return;
	}

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

	if (ped->bRemoveMeWhenIGotIntoCar) {
		ped->bRemoveMeWhenIGotIntoCar = false;
		ped->bRemoveFromWorld = true;
	}
	if (ped->bCollectBusFare) {
		ped->bCollectBusFare = false;
		if (FindPlayerPed())
			FindPlayerPed()->m_nLastBusFareCollected += 5;
	}

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
	if (ped->m_objective == OBJECTIVE_ENTER_CAR_AS_DRIVER || ped->m_nPedState == PED_CARJACK)
		veh->bIsBeingCarJacked = false;

	if (veh->m_nNumGettingIn)
		--veh->m_nNumGettingIn;

	if (ped->IsPlayer() && ((CPlayerPed*)ped)->m_bAdrenalineActive)
		((CPlayerPed*)ped)->ClearAdrenaline();

	if (veh->IsBoat()) {
		if (ped->IsPlayer()) {
			CCarCtrl::RegisterVehicleOfInterest(veh);
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
		ped->RemoveWeaponWhenEnteringVehicle();
		return;
	}

	if (ped->m_pVehicleAnim)
		ped->m_pVehicleAnim->blendDelta = -1000.0f;

	ped->bDoBloodyFootprints = false;
	if (veh->m_nAlarmState == -1)
		veh->m_nAlarmState = 15000;

	if (ped->IsPlayer()) {
		if (ped->m_objective == OBJECTIVE_ENTER_CAR_AS_DRIVER || veh->IsBike()) {
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
			if (passenger && !passenger->bStayInCarOnJack && !passenger->bHeldHostageInCar && (passenger->m_leader != ped || !ped->bIsLeader)) {
				passenger->SetObjective(OBJECTIVE_LEAVE_CAR, veh);
				passenger->m_leaveCarTimer = CTimer::GetTimeInMilliseconds();
			}
		}
	}

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

			if (ped->m_prevObjective == OBJECTIVE_RUN_TO_AREA || ped->m_prevObjective == OBJECTIVE_GOTO_CHAR_ON_FOOT
				|| ped->m_prevObjective == OBJECTIVE_SPRINT_TO_AREA || ped->m_prevObjective == OBJECTIVE_KILL_CHAR_ON_FOOT)
				ped->m_prevObjective = OBJECTIVE_NONE;

			ped->RestorePreviousObjective();
		}

	} else {

		bool slowDown = false;
		if (ped->m_objective == OBJECTIVE_ENTER_CAR_AS_PASSENGER && veh->pDriver && ped->CharCreatedBy == RANDOM_CHAR)
			slowDown = true;

		// VC also has a dead condition in here.

		if (veh->IsBike()) {
			veh->AddPassenger(ped, 0);
		} else if (veh->bIsBus) {
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
		if (ped->m_prevObjective == OBJECTIVE_RUN_TO_AREA || ped->m_prevObjective == OBJECTIVE_GOTO_CHAR_ON_FOOT
			|| ped->m_prevObjective == OBJECTIVE_SPRINT_TO_AREA || ped->m_prevObjective == OBJECTIVE_KILL_CHAR_ON_FOOT)
			ped->m_prevObjective = OBJECTIVE_NONE;

		ped->RestorePreviousObjective();

		// VC has conditional OBJECTIVE_LEAVE_CAR here, which runs if it entered the first dead condition.

		if(slowDown)
			veh->AutoPilot.m_nCruiseSpeed = 17;
	}

	int8 doorFlag;
	if (veh->IsBike()) {
		doorFlag = GetBikeDoorFlagInclJumpInFromFront(ped->m_vehDoor);
	} else {
		doorFlag = GetEnterCarDoorFlag(ped->m_vehDoor, veh->m_nNumMaxPassengers);
	}

	veh->m_nGettingInFlags &= ~doorFlag;

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
		case OBJECTIVE_GOTO_SEAT_ON_FOOT:
		case OBJECTIVE_GOTO_ATM_ON_FOOT:
		case OBJECTIVE_GOTO_BUS_STOP_ON_FOOT:
		case OBJECTIVE_GOTO_PIZZA_ON_FOOT:
		case OBJECTIVE_GOTO_SHELTER_ON_FOOT:
		case OBJECTIVE_SPRINT_TO_AREA:
		case OBJECTIVE_GOTO_ICE_CREAM_VAN_ON_FOOT:
			break;
		default:
			ped->SetObjective(OBJECTIVE_NONE);
	}

	ped->AddInCarAnims(veh, veh->pDriver == ped);
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
		case UNK_CHAR:
			return false;
		default:
			return true;
	}
}

bool
CPed::CanBeDeletedEvenInVehicle(void)
{
	switch (CharCreatedBy) {
		case RANDOM_CHAR:
			return true;
		case MISSION_CHAR:
			return false;
		case UNK_CHAR:
			return false;
		default:
			return true;
	}
}

void
CPed::AddWeaponModel(int id)
{
	if (id != -1) {
		if (m_pWeaponModel)
			RemoveWeaponModel(-1);

		m_pWeaponModel = (RpAtomic*)CModelInfo::GetModelInfo(id)->CreateInstance();
		CModelInfo::GetModelInfo(id)->AddRef();
		m_wepModelID = id;

		if (IsPlayer() && id == MI_MINIGUN)
			((CPlayerPed*)this)->m_pMinigunTopAtomic = (RpAtomic*)CModelInfo::GetModelInfo(MI_MINIGUN2)->CreateInstance();
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
	if(m_pWeaponModel){
		if (modelId == -1
			|| CVisibilityPlugins::GetAtomicModelInfo(m_pWeaponModel) == CModelInfo::GetModelInfo(modelId)) {
			CVisibilityPlugins::GetAtomicModelInfo(m_pWeaponModel)->RemoveRef();
			RwFrame* frm = RpAtomicGetFrame(m_pWeaponModel);
			RpAtomicDestroy(m_pWeaponModel);
			RwFrameDestroy(frm);
			m_pWeaponModel = nil;
		}
	}

	if (IsPlayer() && (modelId == -1 || modelId == MI_MINIGUN)) {
		RpAtomic* &atm = ((CPlayerPed*)this)->m_pMinigunTopAtomic;
		if (atm) {
			RwFrame *frm = RpAtomicGetFrame(atm);
			RpAtomicDestroy(atm);
			RwFrameDestroy(frm);
			atm = nil;
		}
	}
	m_wepModelID = -1;
}

void
CPed::RequestDelayedWeapon()
{
	if (m_delayedWeapon != WEAPONTYPE_UNIDENTIFIED) {
		int modelId1 = CWeaponInfo::GetWeaponInfo(m_delayedWeapon)->m_nModelId;
		int modelId2 = CWeaponInfo::GetWeaponInfo(m_delayedWeapon)->m_nModel2Id;
		if (modelId1 != -1)
			CStreaming::RequestModel(modelId1, STREAMFLAGS_DEPENDENCY);
		if (modelId2 != -1)
			CStreaming::RequestModel(modelId2, STREAMFLAGS_DEPENDENCY);

		if ((modelId1 == -1 || CStreaming::HasModelLoaded(modelId1))
			&& (modelId2 == -1 || CStreaming::HasModelLoaded(modelId2))) {
			GiveWeapon(m_delayedWeapon, m_delayedWeaponAmmo, 1);
			m_delayedWeapon = WEAPONTYPE_UNIDENTIFIED;
		}
	}
}

void
CPed::GiveDelayedWeapon(eWeaponType weapon, uint32 ammo)
{
	m_delayedWeapon = weapon;
	m_delayedWeaponAmmo = ammo;
	if (m_delayedWeapon != WEAPONTYPE_UNIDENTIFIED) {
		int modelId1 = CWeaponInfo::GetWeaponInfo(m_delayedWeapon)->m_nModelId;
		int modelId2 = CWeaponInfo::GetWeaponInfo(m_delayedWeapon)->m_nModel2Id;
		if (modelId1 != -1)
			CStreaming::RequestModel(modelId1, STREAMFLAGS_DEPENDENCY);
		if (modelId2 != -1)
			CStreaming::RequestModel(modelId2, STREAMFLAGS_DEPENDENCY);

		if ((modelId1 == -1 || CStreaming::HasModelLoaded(modelId1))
			&& (modelId2 == -1 || CStreaming::HasModelLoaded(modelId2))) {
			GiveWeapon(m_delayedWeapon, m_delayedWeaponAmmo, true);
			m_delayedWeapon = WEAPONTYPE_UNIDENTIFIED;
		}
	}
}

int32
CPed::GiveWeapon(eWeaponType weaponType, uint32 ammo, bool unused)
{
	int slot = GetWeaponSlot(weaponType);

	if (m_weapons[slot].m_eWeaponType == weaponType) {
		GetWeapon(slot).m_nAmmoTotal += ammo;
		if (weaponType < WEAPONTYPE_TOTALWEAPONS && weaponType > WEAPONTYPE_UNARMED && CWeaponInfo::ms_aMaxAmmoForWeapon[weaponType] >= 0) {

			// Looks like abandoned idea. This block never runs, ms_aMaxAmmoForWeapon is always -1.
			GetWeapon(slot).m_nAmmoTotal = Min(CWeaponInfo::ms_aMaxAmmoForWeapon[weaponType], GetWeapon(slot).m_nAmmoTotal);
		} else {
			GetWeapon(slot).m_nAmmoTotal = Min(99999, GetWeapon(slot).m_nAmmoTotal);
		}
		GetWeapon(slot).Reload();
		if (GetWeapon(slot).m_eWeaponState == WEAPONSTATE_OUT_OF_AMMO && GetWeapon(slot).m_nAmmoTotal > 0)
			GetWeapon(slot).m_eWeaponState = WEAPONSTATE_READY;
	} else {
		if (HasWeaponSlot(slot)) {
			if (CWeaponInfo::IsWeaponSlotAmmoMergeable(slot))
				ammo += GetWeapon(slot).m_nAmmoTotal;

			RemoveWeaponModel(CWeaponInfo::GetWeaponInfo(GetWeapon(slot).m_eWeaponType)->m_nModelId);
			GetWeapon(slot).Shutdown();
		}
		GetWeapon(slot).Initialise(weaponType, ammo);
		if (slot == m_currentWeapon && !bInVehicle) {
			AddWeaponModel(CWeaponInfo::GetWeaponInfo(GetWeapon()->m_eWeaponType)->m_nModelId);
		}
	}
	if (GetWeapon(slot).m_eWeaponState != WEAPONSTATE_OUT_OF_AMMO)
		GetWeapon(slot).m_eWeaponState = WEAPONSTATE_READY;

	return slot;
}

int
CPed::GetWeaponSlot(eWeaponType weaponType)
{
	return CWeaponInfo::GetWeaponInfo(weaponType)->m_nWeaponSlot;
}

void
CPed::SetCurrentWeapon(int slot)
{
	if (slot == -1)
		return;

	CWeaponInfo* weaponInfo;
	if (GetWeapon()->m_eWeaponType != WEAPONTYPE_UNARMED) {
		weaponInfo = CWeaponInfo::GetWeaponInfo(GetWeapon()->m_eWeaponType);
		RemoveWeaponModel(weaponInfo->m_nModelId);
	}
	m_currentWeapon = slot;

	if (FindPlayerPed() && IsPlayer())
		((CPlayerPed*)this)->m_nSelectedWepSlot = m_currentWeapon;

	if (HasWeaponSlot(slot)) {
		weaponInfo = CWeaponInfo::GetWeaponInfo(GetWeapon()->m_eWeaponType);
		AddWeaponModel(weaponInfo->m_nModelId);
	}
}

void
CPed::SetCurrentWeapon(eWeaponType weaponType)
{
	SetCurrentWeapon(CWeaponInfo::GetWeaponInfo(weaponType)->m_nWeaponSlot);
}

void
CPed::GrantAmmo(eWeaponType weaponType, uint32 ammo)
{
	int slot = CWeaponInfo::GetWeaponInfo(weaponType)->m_nWeaponSlot;
	if (slot == -1)
		return;

	GetWeapon(slot).m_nAmmoTotal += ammo;
	if (weaponType < WEAPONTYPE_TOTALWEAPONS && weaponType > WEAPONTYPE_UNARMED && CWeaponInfo::ms_aMaxAmmoForWeapon[weaponType] >= 0) {

		// Looks like abandoned idea. This block never runs, ms_aMaxAmmoForWeapon is always -1.
		GetWeapon(slot).m_nAmmoTotal = Min(CWeaponInfo::ms_aMaxAmmoForWeapon[weaponType], GetWeapon(slot).m_nAmmoTotal);
	} else {
		GetWeapon(slot).m_nAmmoTotal = Min(99999, GetWeapon(slot).m_nAmmoTotal);
	}

	if (GetWeapon(slot).m_eWeaponState == WEAPONSTATE_OUT_OF_AMMO && GetWeapon(slot).m_nAmmoTotal > 0)
		GetWeapon(slot).m_eWeaponState = WEAPONSTATE_READY;
}

void
CPed::SetAmmo(eWeaponType weaponType, uint32 ammo)
{
	int slot = CWeaponInfo::GetWeaponInfo(weaponType)->m_nWeaponSlot;
	if (slot == -1)
		return;

	GetWeapon(slot).m_nAmmoTotal = ammo;
	if (weaponType < WEAPONTYPE_TOTALWEAPONS && weaponType > WEAPONTYPE_UNARMED && CWeaponInfo::ms_aMaxAmmoForWeapon[weaponType] >= 0) {

		// Looks like abandoned idea. This block never runs, ms_aMaxAmmoForWeapon is always -1.
		GetWeapon(slot).m_nAmmoTotal = Min(CWeaponInfo::ms_aMaxAmmoForWeapon[weaponType], GetWeapon(slot).m_nAmmoTotal);
	} else {
		GetWeapon(slot).m_nAmmoTotal = Min(99999, GetWeapon(slot).m_nAmmoTotal);
	}
	int32 newClip = GetWeapon(slot).m_nAmmoTotal;
	if (newClip >= GetWeapon(slot).m_nAmmoInClip)
		newClip = GetWeapon(slot).m_nAmmoInClip;
	GetWeapon(slot).m_nAmmoInClip = newClip;

	if (GetWeapon(slot).m_eWeaponState == WEAPONSTATE_OUT_OF_AMMO && GetWeapon(slot).m_nAmmoTotal > 0)
		GetWeapon(slot).m_eWeaponState = WEAPONSTATE_READY;
}

void
CPed::ClearWeapons(void)
{
	RemoveWeaponModel(-1);
	for (int i = 0; i < ARRAY_SIZE(m_weapons); i++) {
		GetWeapon(i).Shutdown();
	}
	SetCurrentWeapon(WEAPONTYPE_UNARMED);
}

void
CPed::RemoveWeaponWhenEnteringVehicle(void)
{
	if (IsPlayer() && HasWeaponSlot(5) && GetWeapon(5).m_nAmmoTotal > 0 && ((CPlayerPed*)this)->GetPlayerInfoForThisPlayerPed()->m_bDriveByAllowed) {
		if (m_storedWeapon == WEAPONTYPE_UNIDENTIFIED)
			m_storedWeapon = GetWeapon()->m_eWeaponType;
		SetCurrentWeapon(GetWeapon(5).m_eWeaponType);
	} else {
		CWeaponInfo *ourWeapon = CWeaponInfo::GetWeaponInfo(GetWeapon()->m_eWeaponType);
		RemoveWeaponModel(ourWeapon->m_nModelId);
	}
}
void
CPed::ReplaceWeaponWhenExitingVehicle(void)
{
	eWeaponType weaponType = GetWeapon()->m_eWeaponType;

	// If it's Uzi, we may have stored weapon. Uzi is the only gun we can use in car.
	if (IsPlayer() && GetWeaponSlot(weaponType) == WEAPONSLOT_SUBMACHINEGUN) {
		if (m_storedWeapon != WEAPONTYPE_UNIDENTIFIED) {
			SetCurrentWeapon(m_storedWeapon);
			m_storedWeapon = WEAPONTYPE_UNIDENTIFIED;
		}
	} else {
		AddWeaponModel(CWeaponInfo::GetWeaponInfo(weaponType)->m_nModelId);
	}
}

void
CPed::PreRender(void)
{
	CShadows::StoreShadowForPed(this,
		CTimeCycle::m_fShadowDisplacementX[CTimeCycle::m_CurrentStoredValue], CTimeCycle::m_fShadowDisplacementY[CTimeCycle::m_CurrentStoredValue],
		CTimeCycle::m_fShadowFrontX[CTimeCycle::m_CurrentStoredValue], CTimeCycle::m_fShadowFrontY[CTimeCycle::m_CurrentStoredValue],
		CTimeCycle::m_fShadowSideX[CTimeCycle::m_CurrentStoredValue], CTimeCycle::m_fShadowSideY[CTimeCycle::m_CurrentStoredValue]);

	UpdateRpHAnim();

	bool bIsWindModifierTurnedOn = false;
	float fAnyDirectionShift = 1.0f;
	bool bIsPedDrivingBikeOrOpenTopCar = false;
	if (IsPlayer() && CWindModifiers::FindWindModifier(GetPosition(), &fAnyDirectionShift, &fAnyDirectionShift)
		&& !CCullZones::PlayerNoRain() && GetPedState() != PED_DRIVING)
		bIsWindModifierTurnedOn = true;

	if (GetPedState() == PED_DRIVING && m_pMyVehicle) {
		if (m_pMyVehicle->m_vehType == VEHICLE_TYPE_BIKE
			|| (m_pMyVehicle->m_vehType == VEHICLE_TYPE_CAR && m_pMyVehicle->IsOpenTopCar()))
			bIsPedDrivingBikeOrOpenTopCar = true;
	}

	if (bIsWindModifierTurnedOn || bIsPedDrivingBikeOrOpenTopCar) {
		float fWindMult = 0.0f;
		if (bIsPedDrivingBikeOrOpenTopCar) {
			fWindMult = DotProduct(m_pMyVehicle->m_vecMoveSpeed, GetForward());
			if (fWindMult > 0.4f) {
				float volume = (fWindMult - 0.4f) / 0.6f;
				DMAudio.PlayOneShot(m_audioEntityId, SOUND_SHIRT_WIND_FLAP, volume);
			}
		}

		if (bIsWindModifierTurnedOn) 
			fWindMult = Min(fWindMult, Abs(fAnyDirectionShift - 1.0f));

		RpHAnimHierarchy* hier = GetAnimHierarchyFromSkinClump(GetClump());
		int32 idx;
		RwV3d scale;
		float fScaleOffset;

		fScaleOffset = fWindMult * 0.2f;
		scale.x = CGeneral::GetRandomNumberInRange(1.0f - fScaleOffset, 1.0f + fScaleOffset);
		scale.y = CGeneral::GetRandomNumberInRange(1.0f - fScaleOffset, 1.0f + fScaleOffset);
		scale.z = CGeneral::GetRandomNumberInRange(1.0f - fScaleOffset, 1.0f + fScaleOffset);

		idx = RpHAnimIDGetIndex(hier, ConvertPedNode2BoneTag(PED_NECK));
		RwMatrix* neck = &RpHAnimHierarchyGetMatrixArray(hier)[idx];
		RwMatrixScale(neck, &scale, rwCOMBINEPRECONCAT);

		fScaleOffset = fWindMult * 0.1f;
		scale.x = CGeneral::GetRandomNumberInRange(1.0f - fScaleOffset, 1.0f + fScaleOffset);
		scale.y = CGeneral::GetRandomNumberInRange(1.0f - fScaleOffset, 1.0f + fScaleOffset);
		scale.z = CGeneral::GetRandomNumberInRange(1.0f - fScaleOffset, 1.0f + fScaleOffset);

		idx = RpHAnimIDGetIndex(hier, ConvertPedNode2BoneTag(PED_CLAVICLEL));
		RwMatrix* clavicleL = &RpHAnimHierarchyGetMatrixArray(hier)[idx];
		RwMatrixScale(clavicleL, &scale, rwCOMBINEPRECONCAT);
		
		idx = RpHAnimIDGetIndex(hier, ConvertPedNode2BoneTag(PED_CLAVICLER));
		RwMatrix* clavicleR = &RpHAnimHierarchyGetMatrixArray(hier)[idx];
		RwMatrixScale(clavicleR, &scale, rwCOMBINEPRECONCAT);

		idx = RpHAnimIDGetIndex(hier, ConvertPedNode2BoneTag(PED_MID));
		RwMatrix* mid = &RpHAnimHierarchyGetMatrixArray(hier)[idx];
		RwMatrixScale(mid, &scale, rwCOMBINEPRECONCAT);

		fScaleOffset = fWindMult * 0.2f;
		scale.x = CGeneral::GetRandomNumberInRange(1.0f - fScaleOffset, 1.0f + fScaleOffset);
		scale.y = CGeneral::GetRandomNumberInRange(1.0f - fScaleOffset, 1.0f + fScaleOffset);
		scale.z = CGeneral::GetRandomNumberInRange(1.0f - fScaleOffset, 1.0f + fScaleOffset);

		idx = RpHAnimIDGetIndex(hier, ConvertPedNode2BoneTag(PED_UPPERARML));
		RwMatrix* upperArmL = &RpHAnimHierarchyGetMatrixArray(hier)[idx];
		RwMatrixScale(upperArmL, &scale, rwCOMBINEPRECONCAT);

		idx = RpHAnimIDGetIndex(hier, ConvertPedNode2BoneTag(PED_UPPERARMR));
		RwMatrix* upperArmR = &RpHAnimHierarchyGetMatrixArray(hier)[idx];
		RwMatrixScale(upperArmR, &scale, rwCOMBINEPRECONCAT);
	}

	if (bBodyPartJustCameOff && m_bodyPartBleeding == PED_HEAD) {
		// scale head to 0 if shot off
		RpHAnimHierarchy* hier = GetAnimHierarchyFromSkinClump(GetClump());
		int32 idx = RpHAnimIDGetIndex(hier, ConvertPedNode2BoneTag(PED_HEAD));
		RwMatrix* head = &RpHAnimHierarchyGetMatrixArray(hier)[idx];
		RwV3d zero = { 0.0f, 0.0f, 0.0f };
		RwMatrixScale(head, &zero, rwCOMBINEPRECONCAT);
	}

	if (IsPlayer() && gfTommyFatness != 1.0f) {
		RpHAnimHierarchy* hier = GetAnimHierarchyFromSkinClump(GetClump());
		int32 idx;
		RwV3d scale;

		scale.x = 1.0f;
		scale.y = 1.0f + gfTommyFatness * 0.7f;
		scale.z = 1.0f + gfTommyFatness * 0.7f;
		idx = RpHAnimIDGetIndex(hier, ConvertPedNode2BoneTag(PED_HEAD));
		RwMatrix* head = &RpHAnimHierarchyGetMatrixArray(hier)[idx];
		RwMatrixScale(head, &scale, rwCOMBINEPRECONCAT);

		scale.y = 1.0f + gfTommyFatness * 0.2f;
		scale.z = 1.0f + gfTommyFatness * 0.2f;
		idx = RpHAnimIDGetIndex(hier, ConvertPedNode2BoneTag(PED_NECK));
		RwMatrix* neck = &RpHAnimHierarchyGetMatrixArray(hier)[idx];
		RwMatrixScale(neck, &scale, rwCOMBINEPRECONCAT);

		scale.y = 1.0f + gfTommyFatness * 0.5f;
		scale.z = 1.0f + gfTommyFatness * 0.5f;
		idx = RpHAnimIDGetIndex(hier, ConvertPedNode2BoneTag(PED_MID));
		RwMatrix* mid = &RpHAnimHierarchyGetMatrixArray(hier)[idx];
		RwMatrixScale(mid, &scale, rwCOMBINEPRECONCAT);

		scale.y = 1.0f + gfTommyFatness;
		scale.z = 1.0f + gfTommyFatness;
		idx = RpHAnimIDGetIndex(hier, ConvertPedNode2BoneTag(PED_UPPERLEGL));
		RwMatrix* upperLegL = &RpHAnimHierarchyGetMatrixArray(hier)[idx];
		RwMatrixScale(upperLegL, &scale, rwCOMBINEPRECONCAT);

		idx = RpHAnimIDGetIndex(hier, ConvertPedNode2BoneTag(PED_UPPERLEGR));
		RwMatrix* upperLegR = &RpHAnimHierarchyGetMatrixArray(hier)[idx];
		RwMatrixScale(upperLegR, &scale, rwCOMBINEPRECONCAT);

		scale.y = 1.0f + gfTommyFatness * 0.5f;
		scale.z = 1.0f + gfTommyFatness * 0.5f;
		idx = RpHAnimIDGetIndex(hier, ConvertPedNode2BoneTag(PED_LOWERLEGR));
		RwMatrix* lowerLegR = &RpHAnimHierarchyGetMatrixArray(hier)[idx];
		RwMatrixScale(lowerLegR, &scale, rwCOMBINEPRECONCAT);

		idx = RpHAnimIDGetIndex(hier, ConvertPedNode2BoneTag(PED_LOWERLEGL));
		RwMatrix* lowerLegL = &RpHAnimHierarchyGetMatrixArray(hier)[idx];
		RwMatrixScale(lowerLegL, &scale, rwCOMBINEPRECONCAT);

		scale.y = 1.0f + gfTommyFatness * 0.23f;
		scale.z = 1.0f + gfTommyFatness * 0.23f;
		idx = RpHAnimIDGetIndex(hier, ConvertPedNode2BoneTag(PED_FOOTL));
		RwMatrix* footL = &RpHAnimHierarchyGetMatrixArray(hier)[idx];
		RwMatrixScale(footL, &scale, rwCOMBINEPRECONCAT);

		idx = RpHAnimIDGetIndex(hier, ConvertPedNode2BoneTag(PED_FOOTR));
		RwMatrix* footR = &RpHAnimHierarchyGetMatrixArray(hier)[idx];
		RwMatrixScale(footR, &scale, rwCOMBINEPRECONCAT);

		idx = RpHAnimIDGetIndex(hier, ConvertPedNode2BoneTag(PED_UPPERARML));
		RwMatrix* upperArmL = &RpHAnimHierarchyGetMatrixArray(hier)[idx];
		RwMatrixScale(upperArmL, &scale, rwCOMBINEPRECONCAT);

		idx = RpHAnimIDGetIndex(hier, ConvertPedNode2BoneTag(PED_UPPERARMR));
		RwMatrix* upperArmR = &RpHAnimHierarchyGetMatrixArray(hier)[idx];
		RwMatrixScale(upperArmR, &scale, rwCOMBINEPRECONCAT);

		scale.y = 1.0f + gfTommyFatness * 0.2f;
		scale.z = 1.0f + gfTommyFatness * 0.2f;
		idx = RpHAnimIDGetIndex(hier, ConvertPedNode2BoneTag(PED_FOREARML));
		RwMatrix* foreArmL = &RpHAnimHierarchyGetMatrixArray(hier)[idx];
		RwMatrixScale(foreArmL, &scale, rwCOMBINEPRECONCAT);

		idx = RpHAnimIDGetIndex(hier, ConvertPedNode2BoneTag(PED_FOREARMR));
		RwMatrix* foreArmR = &RpHAnimHierarchyGetMatrixArray(hier)[idx];
		RwMatrixScale(foreArmR, &scale, rwCOMBINEPRECONCAT);
	}

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

CVector vecTestTemp(-1.0f, -1.0f, -1.0f);

void
CPed::Render(void)
{
	if (bInVehicle && m_pMyVehicle && m_nPedState != PED_EXIT_CAR && m_nPedState != PED_DRAG_FROM_CAR) {
		if (!bRenderPedInCar)
			return;

		if (!m_pMyVehicle->IsBike() && !IsPlayer()) {
			float camDistSq = (TheCamera.GetPosition() - GetPosition()).MagnitudeSqr();
			if (camDistSq > SQR((m_pMyVehicle->IsBoat() ? 40.0f : 25.0f) * TheCamera.LODDistMultiplier))
				return;
		}
	}

	CEntity::Render();

	if(m_pWeaponModel){
		RpHAnimHierarchy *hier = GetAnimHierarchyFromSkinClump(GetClump());
		int idx = RpHAnimIDGetIndex(hier, m_pFrames[PED_HANDR]->nodeID);
		RwMatrix *mat = &RpHAnimHierarchyGetMatrixArray(hier)[idx];
		RwFrame *frame = RpAtomicGetFrame(m_pWeaponModel);
		*RwFrameGetMatrix(frame) = *mat;
		RwFrameUpdateObjects(frame);
		RpAtomicRender(m_pWeaponModel);
		if (IsPlayer()) {
			CPlayerPed *player = (CPlayerPed*)this;
			if (player->m_pMinigunTopAtomic) {
				frame = RpAtomicGetFrame(player->m_pMinigunTopAtomic);
				*RwFrameGetMatrix(frame) = *mat;

				player->m_fGunSpinAngle = player->m_fGunSpinSpeed * CTimer::GetTimeStep() + player->m_fGunSpinAngle;
				if (player->m_fGunSpinAngle > TWOPI)
					player->m_fGunSpinAngle -= TWOPI;

				CMatrix mgTopMat, localAdjMat;
				mgTopMat.Attach(RwFrameGetMatrix(frame));
				localAdjMat.SetRotateX(player->m_fGunSpinAngle);
				localAdjMat.Rotate(DEGTORAD(-4.477f)* vecTestTemp.x, DEGTORAD(29.731f) * vecTestTemp.y, DEGTORAD(1.064f) * vecTestTemp.z);
				localAdjMat.GetPosition() += CVector(0.829f, -0.001f, 0.226f);
				mgTopMat = mgTopMat * localAdjMat;
				mgTopMat.UpdateRW();

				RwFrameUpdateObjects(frame);
				RpAtomicRender(player->m_pMinigunTopAtomic);
			}
		}
	}
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
		if (m_nPedState == PED_AIM_GUN)
			ClearPointGunAt();

		SetPedState(PED_IDLE);
		SetMoveState(PEDMOVE_STILL);
		m_nLastPedState = PED_NONE;
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

	if (m_nMoveState != PEDMOVE_STILL && !IsPlayer())
		SetMoveState(PEDMOVE_STILL);

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
	if (m_attachedTo)
		return;

	if (!IsPedInControl() && (!evenIfNotInControl || DyingOrDead()))
		return;

	ClearLookFlag();
	ClearAimFlag();
	SetStoredState();
	SetPedState(PED_FALL);
	CAnimBlendAssociation *fallAssoc = nil;
	if (animId == ANIM_STD_NUM) {
		if (IsPlayer()) {
			fallAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_ROLLOUT_LHS);
			if (!fallAssoc)
				fallAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_ROLLOUT_RHS);
		}
	} else {
		fallAssoc = RpAnimBlendClumpGetAssociation(GetClump(), animId);

		if (fallAssoc) {
			fallAssoc->SetCurrentTime(0.0f);
			fallAssoc->blendAmount = 0.0f;
			fallAssoc->blendDelta = 8.0f;
			fallAssoc->SetRun();
		}
		else {
			fallAssoc = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, animId, 8.0f);
		}
		if (animId == ANIM_STD_BIKE_FALLBACK)
			fallAssoc->SetCurrentTime(0.4f);
	}

	if (extraTime == -1) {
		m_getUpTimer = UINT32_MAX;
	} else if (fallAssoc) {
		if (IsPlayer()) {
			if (fallAssoc->animId == ANIM_STD_ROLLOUT_LHS || fallAssoc->animId == ANIM_STD_ROLLOUT_RHS) {
				m_getUpTimer = 1000.0f * fallAssoc->hierarchy->totalLength
					+ CTimer::GetTimeInMilliseconds()
					- 1000.0f * fallAssoc->currentTime
					+ 100.0f;
			} else {
				m_getUpTimer = 1000.0f * fallAssoc->hierarchy->totalLength
					+ CTimer::GetTimeInMilliseconds()
					+ 500.0f;
			}
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
	if (m_getUpTimer != UINT32_MAX && CTimer::GetTimeInMilliseconds() > m_getUpTimer && bIsStanding)
		ClearFall();

	CAnimBlendAssociation *firstPartialAssoc;
	CAnimBlendAssociation *fallAssoc;

	if (IsPlayer() && (bKnockedUpIntoAir || bKnockedOffBike) && !bIsStanding) {
		firstPartialAssoc = RpAnimBlendClumpGetFirstAssociation(GetClump(), ASSOC_PARTIAL);

		// What???
		if (firstPartialAssoc && (firstPartialAssoc->animId == ANIM_STD_FALL_ONBACK || firstPartialAssoc->animId == ANIM_STD_FALL_ONFRONT))
			fallAssoc = firstPartialAssoc;
		else
			fallAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_FALL_ONBACK);

		if (!fallAssoc)
			fallAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_FALL_ONFRONT);

		if (!fallAssoc && firstPartialAssoc && 0.8f * firstPartialAssoc->hierarchy->totalLength < firstPartialAssoc->currentTime) {
			if (firstPartialAssoc->flags & ASSOC_FRONTAL) {
				CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_FALL_ONFRONT, 8.0f);
			} else {
				CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_FALL_ONBACK, 8.0f);
			}
		} else if (fallAssoc && fallAssoc->blendAmount > 0.3f && fallAssoc->blendDelta >= 0.0f)	{
			float time = fallAssoc->currentTime;

			if (time > 0.667f && time - fallAssoc->timeStep <= 0.667f) {
				fallAssoc->SetCurrentTime(0.0f);
				fallAssoc->SetRun();
			}
		}
	} else if ((bKnockedUpIntoAir || bKnockedOffBike) && bIsStanding && !bWasStanding) {
		fallAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_FALL_ONBACK);

		if (!fallAssoc)
			fallAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_FALL_ONFRONT);

		if (fallAssoc) {
			bKnockedUpIntoAir = false;
			bKnockedOffBike = false;
			fallAssoc->speed = 3.0f;
			if (IsPlayer())
				Say(SOUND_PED_LAND);

		} else {
			firstPartialAssoc = RpAnimBlendClumpGetFirstAssociation(GetClump(), ASSOC_PARTIAL);
			if (firstPartialAssoc && !firstPartialAssoc->IsRunning()) {
				bKnockedUpIntoAir = false;
				bKnockedOffBike = false;
			}
		}
	}
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
				if (GetPosition().z - foundCol.point.z < 1.3f || bIsStanding)
					SetLanding();
			} else if (m_nPedState != PED_ABSEIL && !RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_FALL)) {
				if (m_vecMoveSpeed.z < -0.1f)
					CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_FALL, 4.0f);
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

	if (fallAssoc && bIsDrowning)
		return;

	RpAnimBlendClumpSetBlendDeltas(GetClump(), ASSOC_PARTIAL, -1000.0f);
	if (fallAssoc || m_nPedType == PEDTYPE_COP && bKnockedUpIntoAir) {
		landAssoc = CAnimManager::AddAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_FALL_COLLAPSE);
		DMAudio.PlayOneShot(m_audioEntityId, SOUND_FALL_COLLAPSE, 1.0f);

		if (IsPlayer())
			Say(SOUND_PED_LAND);

		if (m_nPedType == PEDTYPE_COP) {
			if (bKnockedUpIntoAir)
				bKnockedUpIntoAir = false;
		}

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
		if (veh && veh->m_vehType != VEHICLE_TYPE_BIKE && veh != m_attachedTo ||
			collidingVeh && collidingVeh->IsVehicle() && collidingVeh->m_vehType != VEHICLE_TYPE_BIKE
			&& ((uint8)(CTimer::GetFrameCounter() + m_randomSeed + 5) % 8 ||
				 CCollision::ProcessColModels(GetMatrix(), *GetColModel(), collidingVeh->GetMatrix(), *collidingVeh->GetColModel(),
					aTempPedColPts, nil, nil) > 0)) {

			bGetUpAnimStarted = false;
			if (IsPlayer())
				InflictDamage(nil, WEAPONTYPE_RUNOVERBYCAR, CTimer::GetTimeStep(), PEDPIECE_TORSO, 0);
			else if(CPad::GetPad(0)->ArePlayerControlsDisabled())
				InflictDamage(nil, WEAPONTYPE_RUNOVERBYCAR, 1000.0f, PEDPIECE_TORSO, 0);
			return;
		}
		bGetUpAnimStarted = true;
		m_pCollidingEntity = nil;
		bKnockedUpIntoAir = false;
		bKnockedOffBike = false;
		CAnimBlendAssociation *animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_RUNFAST);
		if (animAssoc) {
			if (RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_RUN)) {
				CAnimManager::BlendAnimation(GetClump(), m_animGroup, ANIM_STD_RUN, 8.0f);
			} else {
				CAnimManager::BlendAnimation(GetClump(), m_animGroup, ANIM_STD_IDLE, 8.0f);
			}
			animAssoc->flags |= ASSOC_DELETEFADEDOUT;
		}

		if (m_nWaitState == WAITSTATE_SUN_BATHE_IDLE) {
			m_headingRate = 0.0f;

			// Seemingly they planned to use different getup anims for different conditions, but sadly in final game all getup anims(GETUP1, GETUP2, GETUP3) are same...
			if (bFleeWhenStanding && m_threatEx)
				animAssoc = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_GET_UP, 1000.0f);
			else
				animAssoc = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_GET_UP, 1000.0f);

		} else if (RpAnimBlendClumpGetFirstAssociation(GetClump(), ASSOC_FRONTAL))
			animAssoc = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_GET_UP_FRONT, 1000.0f);
		else
			animAssoc = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_GET_UP, 1000.0f);

		animAssoc->SetFinishCallback(PedGetupCB, this);
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

// Unused
void
CPed::SetLook(float direction)
{
	if (IsPedInControl()) {
		SetStoredState();
		SetPedState(PED_LOOK_HEADING);
		SetLookFlag(direction, false);
	}
}

void
CPed::SetLook(CEntity* to)
{
	if (IsPedInControl()) {
		SetStoredState();
		SetPedState(PED_LOOK_ENTITY);
		SetLookFlag(to, false);
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
	TurnBody();
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
		|| (m_nPedState == PED_SEEK_POS && m_vecSeekPos.x == pos.x && m_vecSeekPos.y == pos.y) || m_nPedState == PED_FOLLOW_PATH)
		return;

	if (!CWeaponInfo::GetWeaponInfo(GetWeapon()->m_eWeaponType)->IsFlagSet(WEAPONFLAG_CANAIM_WITHARM)) {
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

	if (!seeking || m_nPedState == PED_FOLLOW_PATH)
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
				&& (CTimer::GetFrameCounter() + m_randomSeed + 60) % 32 == 0) {

				CEntity *obstacle = CWorld::TestSphereAgainstWorld(m_vecSeekPos, 0.4f, nil,
									false, true, false, false, false, false);

				if (obstacle) {
					if (!obstacle->IsVehicle() || ((CVehicle*)obstacle)->IsCar()) {
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

	if (m_objective == OBJECTIVE_FOLLOW_CHAR_IN_FORMATION && !m_pedInObjective) {
		m_objective = OBJECTIVE_NONE;
		ClearObjective();
		SetWanderPath(0);
		return false;
	}

	float seekPosDist = (m_vecSeekPos - GetPosition()).Magnitude2D();
	if (seekPosDist < 2.0f || m_objective == OBJECTIVE_GOTO_AREA_ON_FOOT) {

		if (m_objective == OBJECTIVE_FOLLOW_CHAR_IN_FORMATION) {

			if (m_pedInObjective->m_nMoveState != PEDMOVE_STILL)
				nextMove = m_pedInObjective->m_nMoveState;
		} else
			nextMove = PEDMOVE_WALK;

	} else if (m_objective != OBJECTIVE_FOLLOW_CHAR_IN_FORMATION) {

		if (m_objective == OBJECTIVE_SPRINT_TO_AREA)
			nextMove = PEDMOVE_SPRINT;
		else if (m_objective == OBJECTIVE_KILL_CHAR_ON_FOOT || m_objective == OBJECTIVE_KILL_CHAR_ANY_MEANS || m_objective == OBJECTIVE_RUN_TO_AREA || bIsRunning)
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

	CVector *nextNode = SeekFollowingPath();

	if (nextNode || seekPosDist >= distanceToCountItDone) {
		if (bIsRunning && nextMove != PEDMOVE_SPRINT)
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
					m_actionX = 0.f;
					m_actionY = 0.f;
				}
			}

		} else {
			if (nextNode) {
				m_fRotationDest = CGeneral::GetRadianAngleBetweenPoints(
					nextNode->x, nextNode->y,
					GetPosition().x, GetPosition().y);
			} else {
				m_fRotationDest = CGeneral::GetRadianAngleBetweenPoints(
					m_vecSeekPos.x, m_vecSeekPos.y,
					GetPosition().x, GetPosition().y);
			}

			float neededTurn = Abs(m_fRotationDest - m_fRotationCur);

			if (neededTurn > PI)
				neededTurn = TWOPI - neededTurn;

			if (neededTurn > HALFPI) {
				if (seekPosDist >= 1.0f && neededTurn <= DEGTORAD(135.0f)) {
					if (seekPosDist < 2.0f)
						nextMove = PEDMOVE_WALK;
				} else {
					nextMove = PEDMOVE_STILL;
				}
			}
		}

		if (((m_nPedState == PED_FLEE_POS || m_nPedState == PED_FLEE_ENTITY) && m_nMoveState < nextMove)
			|| (m_nPedState != PED_FLEE_POS && m_nPedState != PED_FLEE_ENTITY && m_nPedState != PED_FOLLOW_PATH && m_objective != OBJECTIVE_GOTO_CHAR_ON_FOOT && m_nWaitState == WAITSTATE_FALSE)) {

			SetMoveState(nextMove);
		}
		
		SetMoveAnim();
		return false;
	}

	if ((m_objective != OBJECTIVE_FOLLOW_CHAR_IN_FORMATION || m_pedInObjective->m_nMoveState == PEDMOVE_STILL) && m_nMoveState != PEDMOVE_STILL) {
		m_nPedStateTimer = 0;
		m_actionX = 0.f;
		m_actionY = 0.f;
	}

	if (m_objective == OBJECTIVE_GOTO_AREA_ON_FOOT || m_objective == OBJECTIVE_RUN_TO_AREA || m_objective == OBJECTIVE_SPRINT_TO_AREA ||
		m_objective == OBJECTIVE_GOTO_AREA_ANY_MEANS || IsUseAttractorObjective(m_objective)) {

		if (m_pNextPathNode)
			m_pNextPathNode = nil;
		else
			bScriptObjectiveCompleted = true;

		bUsePedNodeSeek = true;
	}

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
		m_fleeFromPos = from;
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
			bMakeFleeScream = false;
			eMoveState moveState = m_nMoveState;
			ClearFlee();

			if (m_objective == OBJECTIVE_FLEE_CHAR_ON_FOOT_TILL_SAFE || m_objective == OBJECTIVE_FLEE_CHAR_ON_FOOT_ALWAYS) {
				bBeingChasedByPolice = false;
				RestorePreviousObjective();
			}

			if ((m_nPedState == PED_IDLE || m_nPedState == PED_WANDER_PATH) && CGeneral::GetRandomNumber() & 1) {
				SetWaitState(moveState <= PEDMOVE_WALK ? WAITSTATE_CROSS_ROAD_LOOK : WAITSTATE_FINISH_FLEE, nil);
			}
			return;
		}
		m_fleeTimer = CTimer::GetTimeInMilliseconds() + 5000;
	}

	if (bMakeFleeScream && !((CTimer::GetFrameCounter() + m_randomSeed) & 7)) {
		Say(SOUND_PED_FLEE_SPRINT);
		bMakeFleeScream = false;
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
				m_vecSeekPos = CPathFind::TakeWidthIntoAccountForWandering(m_pNextPathNode, m_randomSeed);
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

	if (IsPlayer())
		return false;

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
	m_vecSeekPos = CPathFind::TakeWidthIntoAccountForWandering(m_pNextPathNode, m_randomSeed);
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
	uint8 tryCount = 0;

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

	if (((CPedModelInfo*)CModelInfo::GetModelInfo(m_modelIndex))->m_pedStatType == PEDSTAT_SKATER) {
		if (m_pNextPathNode) {
			CVector unpacked(m_pNextPathNode->GetPosition() / 8.f);
			if (!CPopulation::IsSkateable(unpacked))
				m_pNextPathNode = nil;
		}
	}

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
			if (((CPedModelInfo*)CModelInfo::GetModelInfo(m_modelIndex))->m_pedStatType == PEDSTAT_SKATER) {
				if (m_pNextPathNode) {
					CVector unpacked(m_pNextPathNode->GetPosition() / 8.f);
					if (!CPopulation::IsSkateable(unpacked))
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

CVector*
CPed::SeekFollowingPath(void)
{
	static CVector vecNextPathNode;

	if (m_nCurPathNodeId >= m_nNumPathNodes || m_nNumPathNodes == 0)
		return nil;

	vecNextPathNode = m_pathNodesToGo[m_nCurPathNodeId]->GetPosition();

	if ((vecNextPathNode - GetPosition()).Magnitude2D() < m_distanceToCountSeekDone) {
		m_nCurPathNodeId++;
		if (m_nCurPathNodeId < m_nNumPathNodes)
			m_pCurPathNode = m_pathNodesToGo[m_nCurPathNodeId];
	}
	if (m_nCurPathNodeId == m_nNumPathNodes)
		return nil;
	else
		return &vecNextPathNode;
}

bool
CPed::SetFollowPath(CVector dest, float radius, eMoveState state, CEntity* walkAroundEnt, CEntity* targetEnt, int time)
{
	if (m_nPedState == PED_FOLLOW_PATH) {
		bool stopFollow = false;
		if (walkAroundEnt && walkAroundEnt != m_followPathWalkAroundEnt || !walkAroundEnt && m_followPathWalkAroundEnt
			|| targetEnt && targetEnt != m_followPathTargetEnt || !targetEnt && m_followPathTargetEnt) {
			stopFollow = true;

		} else if (targetEnt) {
			if ((targetEnt->GetPosition() - m_followPathDestPos).MagnitudeSqr() > 1.f)
				stopFollow = true;

		} else if (!walkAroundEnt && !targetEnt) {
			if ((dest - m_followPathDestPos).MagnitudeSqr() > 1.f)
				stopFollow = true;
		}

		if (!stopFollow)
			return false;
	}
	m_pathNodeTimer = CTimer::GetTimeInMilliseconds() + time;
	m_followPathWalkAroundEnt = walkAroundEnt;
	m_followPathTargetEnt = targetEnt;
	m_distanceToCountSeekDone = 0.5f;

	bool weHaveTargetPed = targetEnt && targetEnt->IsPed();
	bool useDestVec = !weHaveTargetPed;

	if (useDestVec)
		m_followPathDestPos = dest;
	else
		m_followPathDestPos = targetEnt->GetPosition();

	if (targetEnt && m_nPedState == PED_SEEK_POS) {
		m_followPathDestPos = m_vecSeekPos;
	}

	m_followPathAbortDist = radius > 0.f ? radius : 20.f;

	bool useGivenPedMove = state == PEDMOVE_RUN || state == PEDMOVE_WALK;
	if (useGivenPedMove)
		m_followPathMoveState = state;
	else
		m_followPathMoveState = PEDMOVE_WALK;

	if (m_followPathWalkAroundEnt)
		return SetFollowPathDynamic();
	else
		return SetFollowPathStatic();
}

bool
CPed::SetFollowPathStatic(void)
{
	ClearFollowPath();
	if (sq(m_followPathAbortDist) > (GetPosition() - m_followPathDestPos).MagnitudeSqr()
		&& CWorld::IsWanderPathClear(GetPosition(), m_followPathDestPos, 0.5f, 4)) {

		RestorePreviousState();
		if (m_objective == OBJECTIVE_NONE) {
			if (m_followPathMoveState == PEDMOVE_RUN)
				SetObjective(OBJECTIVE_RUN_TO_AREA, m_followPathDestPos);
			else
				SetObjective(OBJECTIVE_GOTO_AREA_ON_FOOT, m_followPathDestPos);			
		}
		SetPedState(PED_NONE);
	} else {
		ThePaths.DoPathSearch(PATH_PED, GetPosition(), -1, m_followPathDestPos, m_pathNodesToGo, &m_nNumPathNodes,
			ARRAY_SIZE(m_pathNodesToGo), nil, nil, 999999.9f, -1);

		if (m_nNumPathNodes != 0) {
			if (m_nNumPathNodes > 0 && m_pathNodesToGo[0] != m_pCurPathNode) {
				for (int i = 0; i < ARRAY_SIZE(m_pathNodesToGo) - 1; i++) {
					m_pathNodesToGo[i] = m_pathNodesToGo[i+1];
				}
				--m_nNumPathNodes;
			}
			for (int i = 0; i < m_nNumPathNodes; ++i) {
				CVector nodePos = m_pathNodesToGo[i]->GetPosition();
				if (sq(m_followPathAbortDist) > (nodePos - m_followPathDestPos).MagnitudeSqr()
					&& CWorld::IsWanderPathClear(nodePos, m_followPathDestPos, 0.5f, 4)) {

					m_nNumPathNodes = i + 1;
					break;
				}
			}

			m_nCurPathNodeId = 0;
			if (m_pCurPathNode) {
				for (int j = 0; j < m_nNumPathNodes; ++j) {
					if (m_pathNodesToGo[j] == m_pCurPathNode) {
						m_nCurPathNodeId = j;
						break;
					}
				}
			}
			m_pCurPathNode = m_pathNodesToGo[m_nCurPathNodeId];
			PedState oldLastState = m_nLastPedState;
			m_nLastPedState = PED_NONE;
			SetStoredState();
			if (m_nLastPedState == PED_NONE)
				m_nLastPedState = oldLastState;

			SetPedState(PED_FOLLOW_PATH);
			SetMoveState(m_followPathMoveState);
		} else {
			RestorePreviousState();
			if (m_objective == OBJECTIVE_NONE) {
				if (m_followPathMoveState == PEDMOVE_RUN)
					SetObjective(OBJECTIVE_RUN_TO_AREA, m_followPathDestPos);
				else
					SetObjective(OBJECTIVE_GOTO_AREA_ON_FOOT, m_followPathDestPos);
			}
			SetPedState(PED_NONE);
		}
	}
	return true;
}

bool
CPed::SetFollowPathDynamic(void)
{
	CVector colBoxMin = m_followPathWalkAroundEnt->GetColModel()->boundingBox.min + CVector(-0.35f, -0.35f, 0.f);
	CVector colBoxMax = m_followPathWalkAroundEnt->GetColModel()->boundingBox.max + CVector(0.35f, 0.35f, 0.f);

	CVector colCornerOffsets[4]; // BL, BR, TR, TL
	colCornerOffsets[0] = CVector(colBoxMin.x, colBoxMin.y, 0.f);
	colCornerOffsets[1] = CVector(colBoxMax.x, colBoxMin.y, 0.f);
	colCornerOffsets[2] = CVector(colBoxMax.x, colBoxMax.y, 0.f);
	colCornerOffsets[3] = CVector(colBoxMin.x, colBoxMax.y, 0.f);

	if (m_followPathWalkAroundEnt->IsVehicle() && ((CVehicle*)m_followPathWalkAroundEnt)->IsUpsideDown()) {
		CVector old0 = colCornerOffsets[0];
		colCornerOffsets[0] = colCornerOffsets[1];
		colCornerOffsets[1] = old0;
		CVector old2 = colCornerOffsets[2];
		colCornerOffsets[2] = colCornerOffsets[3];
		colCornerOffsets[3] = old2;
	}

	CVector colCornerPos[4]; // global. again BL, BR, TR, TL
	float dotProdCorrection[4];
	CVector colBoxPlaneNormal[4];

	for (int i=0; i<4; i++) {
		colCornerPos[i] = m_followPathWalkAroundEnt->GetMatrix() * colCornerOffsets[i];
		colCornerPos[i].z = GetPosition().z;
	}

	CVector prevColCorner = colCornerPos[3];  // top left
	CVector *curCornerPos;
	CVector fwdToNextCorner;

	for (int i=0; i<4; i++) {
		curCornerPos = &colCornerPos[i];
		fwdToNextCorner = *curCornerPos - prevColCorner;
		fwdToNextCorner.Normalise();
		colBoxPlaneNormal[i] = CrossProduct(fwdToNextCorner, CVector(0.f, 0.f, 1.f));
		dotProdCorrection[i] = -DotProduct(prevColCorner, colBoxPlaneNormal[i]); // yes, dp with global coord, as if in distance to plane calculation
		prevColCorner = *curCornerPos;
	}

	bool weReGoingGreat = false;
	CVector startVecCandidate = GetPosition();
	CVector targetVecCandidate = m_followPathDestPos;
	CVector dirToGo = targetVecCandidate - startVecCandidate;
	dirToGo.Normalise();
	CVector ourPos = startVecCandidate;
	
	for (int i=0; i<4; i++) {
		CVector curPlaneNormal = colBoxPlaneNormal[i];
		float minusGlobalCornerPos = dotProdCorrection[i];
		float startVecDistToPlane = DotProduct(curPlaneNormal, startVecCandidate) + minusGlobalCornerPos;

#define FRONT_OF_PLANE 1
#define ON_THE_PLANE 0
#define BEHIND_THE_PLANE -1

		int8 startVecStatus;
		int8 targetVecStatus;

		if (startVecDistToPlane > 0.1f)
			startVecStatus = FRONT_OF_PLANE;
		else if (startVecDistToPlane < -0.1f)
			startVecStatus = BEHIND_THE_PLANE;
		else
			startVecStatus = ON_THE_PLANE;

		float targetVecDistToPlane = DotProduct(curPlaneNormal, targetVecCandidate) + minusGlobalCornerPos;
		if (targetVecDistToPlane > 0.1f)
			targetVecStatus = FRONT_OF_PLANE;
		else if (targetVecDistToPlane < -0.1f)
			targetVecStatus = BEHIND_THE_PLANE;
		else
			targetVecStatus = ON_THE_PLANE;


		if (startVecStatus == BEHIND_THE_PLANE || targetVecStatus == BEHIND_THE_PLANE) {
			if (startVecStatus == BEHIND_THE_PLANE && targetVecStatus == FRONT_OF_PLANE) {
				targetVecCandidate = -(DotProduct(ourPos, curPlaneNormal) + minusGlobalCornerPos) / DotProduct(dirToGo, curPlaneNormal) * dirToGo + ourPos;

			} else if (startVecStatus == FRONT_OF_PLANE && targetVecStatus == BEHIND_THE_PLANE) {
				startVecCandidate = -(DotProduct(ourPos, curPlaneNormal) + minusGlobalCornerPos) / DotProduct(dirToGo, curPlaneNormal) * dirToGo + ourPos;
			}
		} else {
			weReGoingGreat = true;
			if (startVecStatus == ON_THE_PLANE)
				startVecCandidate += (0.1f - startVecDistToPlane) * curPlaneNormal;

			if (targetVecStatus == ON_THE_PLANE)
				targetVecCandidate += (0.1f - targetVecDistToPlane) * curPlaneNormal;
		}
#undef FRONT_OF_PLANE
#undef ON_THE_PLANE
#undef BEHIND_THE_PLANE
	}

	if (!weReGoingGreat) {
		CVector avgOfColPoints = (colCornerPos[0] + colCornerPos[1] + colCornerPos[2] + colCornerPos[3]) / 4.f;
		float radius = 0.0f;

		// Find radius of col box of the entity we follow
		for (int i=0; i<4; i++) {
			float cornerDist = (colCornerPos[i] - avgOfColPoints).MagnitudeSqr();

			if (cornerDist > radius)
				radius = cornerDist;
		}
		CColSphere followedEntSphere;
		followedEntSphere.Set(Sqrt(radius) * 1.1f, avgOfColPoints, 0, 0);
		CVector distToDest = m_followPathDestPos - GetPosition();
		distToDest.z = 0.f;
		
		if (distToDest.Magnitude() == 0.0f)
			return false;

		distToDest.Normalise();

		// Entity we follow doesn't go toward destination anymore, abort the following.
		if (!followedEntSphere.IntersectRay(GetPosition(), distToDest, startVecCandidate, targetVecCandidate)) {
			m_pathNodeTimer = 0;
			if (m_nPedState == PED_FOLLOW_PATH)
				RestorePreviousState();

			return false;
		}
	}

	int lastPlaneBehindUs = -1;
	int lastPlaneInFrontOfUs = -1;
	CVector oldstartVecCandidate = startVecCandidate;
	CVector oldDirToGo = targetVecCandidate - startVecCandidate;
	oldDirToGo.Normalise();
		

	// At least one plane should be between target and us.
	for (int i=0; i<4; i++) {
		CVector curPlaneNormal = colBoxPlaneNormal[i];
		float minusGlobalCornerPos = dotProdCorrection[i];
		float startVecDistToPlane = DotProduct(curPlaneNormal, startVecCandidate) + minusGlobalCornerPos;
		float targetVecDistToPlane = DotProduct(curPlaneNormal, targetVecCandidate) + minusGlobalCornerPos;

		if (startVecDistToPlane > 0.0f && targetVecDistToPlane < 0.0f) {
			lastPlaneInFrontOfUs = i;
			startVecCandidate = -(DotProduct(oldstartVecCandidate, curPlaneNormal) + minusGlobalCornerPos) / DotProduct(oldDirToGo, curPlaneNormal) * oldDirToGo + oldstartVecCandidate;

		} else if (startVecDistToPlane < 0.0f && targetVecDistToPlane > 0.0f) {
			lastPlaneBehindUs = i;
			targetVecCandidate = -(DotProduct(oldstartVecCandidate, curPlaneNormal) + minusGlobalCornerPos) / DotProduct(oldDirToGo, curPlaneNormal) * oldDirToGo + oldstartVecCandidate;
		}
	}

	CVector destsVariant1[5];
	CVector destsVariant2[5];

	// If not, followed entity diverged from route and we should abort the following.
	if (lastPlaneBehindUs >= 0 && lastPlaneInFrontOfUs >= 0) {

		int planeInFrontCircular = (lastPlaneInFrontOfUs + 4) % -4;
		int planeInFrontCircularMinusOne = (lastPlaneInFrontOfUs + 3) % -4;
		int planeInBehindCircular = (lastPlaneBehindUs + 4) % -4;
		int planeInBehindCircularMinusOne = (lastPlaneBehindUs + 3) % -4;

		destsVariant1[0] = GetPosition();
		destsVariant1[1] = colCornerPos[planeInFrontCircularMinusOne];

		int destsVar1LastNode = 2;
		for(; planeInFrontCircularMinusOne != planeInBehindCircular; destsVar1LastNode++) {
			planeInFrontCircularMinusOne = (planeInFrontCircularMinusOne + 3) % -4;
			destsVariant1[destsVar1LastNode] = colCornerPos[planeInFrontCircularMinusOne];
		}
		destsVariant1[destsVar1LastNode] = m_followPathDestPos;

		destsVariant2[0] = GetPosition();
		destsVariant2[1] = colCornerPos[planeInFrontCircular];

		int destsVar2LastNode = 2;
		for (; planeInFrontCircular != planeInBehindCircularMinusOne; destsVar2LastNode++) {
			planeInFrontCircular = (planeInFrontCircular + 5) % -4;
			destsVariant2[destsVar2LastNode] = colCornerPos[planeInFrontCircular];
		}
		destsVariant2[destsVar2LastNode] = m_followPathDestPos;
		CEntity *foundEnt1 = nil;
		int dests1isOk = true;
		int nodeToStopDestsVar1 = destsVar1LastNode + 1;
		CVector avgOfColPoints2 = (colCornerPos[0] + colCornerPos[1] + colCornerPos[2] + colCornerPos[3]) / 4.f;

		CVector prevDestVar1 = destsVariant1[0];
			
		for (int i = 1; i < destsVar1LastNode + 1; i++) {
			CVector *curDestVar1 = &destsVariant1[i];

			CVector routeNormalHalf = *curDestVar1 - prevDestVar1;
			routeNormalHalf.z = 0.f;
			routeNormalHalf.Normalise();
			routeNormalHalf *= 0.5f;

			float oldX = -routeNormalHalf.x;
			routeNormalHalf.z = 0.0f;
			routeNormalHalf.x = routeNormalHalf.y;
			routeNormalHalf.y = oldX;

			if (DotProduct(*curDestVar1 - avgOfColPoints2, routeNormalHalf) < 0.0f)
				routeNormalHalf *= -1.f;

			CColPoint foundCol;
			bool foundObstacle = CWorld::ProcessLineOfSight(prevDestVar1, *curDestVar1, foundCol, foundEnt1,
				true, true, true, true, false, false, false, false);

			if (!foundObstacle)
				foundObstacle = CWorld::ProcessLineOfSight(prevDestVar1 + routeNormalHalf, *curDestVar1 + routeNormalHalf, foundCol, foundEnt1, true, true, true, true, false, false, false, false);

			if (foundObstacle) {
				if (foundEnt1 == m_followPathWalkAroundEnt || foundEnt1 == this || foundEnt1 == m_pSeekTarget) {
					foundEnt1 = nil;

				} else {
					if (!foundEnt1->IsPed()) {
						dests1isOk = false;
						nodeToStopDestsVar1 = i;
						break;
					}
					if (((CPed*)foundEnt1)->m_nPedState == PED_IDLE) {
						dests1isOk = false;
						nodeToStopDestsVar1 = i;
						break;
					}
					if (DotProduct(*curDestVar1 - prevDestVar1, foundEnt1->GetForward()) < 0.f) {
						dests1isOk = false;
						nodeToStopDestsVar1 = i;
						break;
					}
					if (((CPed*)foundEnt1)->m_pedInObjective == this) {
						dests1isOk = false;
						nodeToStopDestsVar1 = i;
						break;
					}
				}
			}
			prevDestVar1 = *curDestVar1;
		}
		CEntity *foundEnt2 = nil;
		int dests2isOk = true;
		int nodeToStopDestsVar2 = destsVar2LastNode + 1;

		CVector prevDestVar2 = destsVariant2[0];

		for (int i = 1; i < destsVar2LastNode + 1; i++) {
			CVector *curDestVar2 = &destsVariant2[i];

			CVector routeNormalHalf = *curDestVar2 - prevDestVar2;
			routeNormalHalf.z = 0.f;
			routeNormalHalf.Normalise();
			routeNormalHalf *= 0.5f;

			float oldX = -routeNormalHalf.x;
			routeNormalHalf.z = 0.0f;
			routeNormalHalf.x = routeNormalHalf.y;
			routeNormalHalf.y = oldX;

			if (DotProduct(*curDestVar2 - avgOfColPoints2, routeNormalHalf) < 0.0f)
				routeNormalHalf *= -1.f;

			CColPoint foundCol;
			bool foundObstacle = CWorld::ProcessLineOfSight(prevDestVar2, *curDestVar2, foundCol, foundEnt2,
				true, true, true, true, false, false, false, false);

			if (!foundObstacle)
				foundObstacle = CWorld::ProcessLineOfSight(prevDestVar2 + routeNormalHalf, *curDestVar2 + routeNormalHalf, foundCol, foundEnt2, true, true, true, true, false, false, false, false);

				if (foundObstacle) {
					if (foundEnt2 == m_followPathWalkAroundEnt || foundEnt2 == this || foundEnt2 == m_pSeekTarget) {
						foundEnt2 = 0;
					} else {
						if (!foundEnt2->IsPed()) {
							dests2isOk = false;
							nodeToStopDestsVar2 = i;
							break;
						}
						if (((CPed*)foundEnt2)->m_nPedState == PED_IDLE) {
							dests2isOk = false;
							nodeToStopDestsVar2 = i;
							break;
						}
						if (DotProduct(*curDestVar2 - prevDestVar2, foundEnt2->GetForward()) < 0.f) {
							dests2isOk = false;
							nodeToStopDestsVar2 = i;
								break;
						}
						if (((CPed*)foundEnt2)->m_pedInObjective == this) {
							dests2isOk = false;
							nodeToStopDestsVar2 = i;
							break;
						}
					}
				}
				prevDestVar2 = *curDestVar2;
		}

		float destTotalLengthVar1 = 0.0f;
		for(int i=0; i < destsVar1LastNode; i++){
			destTotalLengthVar1 += (destsVariant1[i + 1] - destsVariant1[i]).Magnitude();
		}

		float destTotalLengthVar2 = 0.0f;
		for (int i = 0; i < destsVar2LastNode; i++) {
			destTotalLengthVar2 += (destsVariant2[i + 1] - destsVariant2[i]).Magnitude();
		}

		int destVariantToUse;
		if (dests1isOk && dests2isOk) {
			if (destTotalLengthVar1 < destTotalLengthVar2)
				destVariantToUse = 1;
			else
				destVariantToUse = 2;

		} else if (dests1isOk) {
			destVariantToUse = 1;

		} else if (dests2isOk) {
			destVariantToUse = 2;

		} else if (nodeToStopDestsVar1 == 1 && nodeToStopDestsVar2 > 1) {
			destVariantToUse = 2;

		} else if (nodeToStopDestsVar1 > 1 && nodeToStopDestsVar2 == 1) {
			destVariantToUse = 1;

		} else if (foundEnt1 == foundEnt2) {
			if (destTotalLengthVar1 < destTotalLengthVar2)
				destVariantToUse = 1;
			else
				destVariantToUse = 2;

		} else if (foundEnt1->GetColModel()->boundingSphere.radius >= foundEnt2->GetColModel()->boundingSphere.radius) {
			destVariantToUse = 2;
		} else {
			destVariantToUse = 1;
		}

		if (destVariantToUse == 1) {
			ClearFollowPath();
			for (int i = 1; i < destsVar1LastNode; i++) {
				CPathNode* nextNode = &m_pathNodeObjPool[m_nNumPathNodes];
				nextNode->SetPosition(destsVariant1[i]);
				m_pathNodesToGo[m_nNumPathNodes++] = nextNode;
			}
		} else if (destVariantToUse == 2) {
			ClearFollowPath();
			for (int i = 1; i < destsVar2LastNode; i++) {
				CPathNode *nextNode = &m_pathNodeObjPool[m_nNumPathNodes];
				nextNode->SetPosition(destsVariant2[i]);
				m_pathNodesToGo[m_nNumPathNodes++] = nextNode;
			}
		}
		if (m_nNumPathNodes != 0) {
			PedState oldLastState = m_nLastPedState;
			m_nLastPedState = PED_NONE;
			SetStoredState();
			if (m_nLastPedState == PED_NONE)
				m_nLastPedState = oldLastState;

			SetPedState(PED_FOLLOW_PATH);
			SetMoveState(m_followPathMoveState);
			return true;

		} else {
			m_pathNodeTimer = 0;
			if (m_nPedState == PED_FOLLOW_PATH)
				RestorePreviousState();

			return false;
		}
	} else {
		m_pathNodeTimer = 0;
		if (m_nPedState == PED_FOLLOW_PATH)
			RestorePreviousState();

		return false;
	}
}

void
CPed::ClearFollowPath()
{
	for (int i = 0; i < ARRAY_SIZE(m_pathNodesToGo); i++) {
		m_pathNodesToGo[i] = nil;
	}
	m_nNumPathNodes = 0;
	m_nCurPathNodeId = 0;
}

void
CPed::FollowPath(void)
{
	m_pCurPathNode = m_pathNodesToGo[m_nCurPathNodeId];
	if (m_pathNodeTimer > 0 && CTimer::GetTimeInMilliseconds() > m_pathNodeTimer) {
		RestorePreviousState();
		ClearFollowPath();
		m_pathNodeTimer = 0;
	} else {
		if (m_pathNodesToGo[m_nCurPathNodeId]) {
			m_vecSeekPos.x = m_pathNodesToGo[m_nCurPathNodeId]->GetPosition().x;
			m_vecSeekPos.y = m_pathNodesToGo[m_nCurPathNodeId]->GetPosition().y;
			m_vecSeekPos.z = GetPosition().z;

			if (Seek()) {
				if (m_nCurPathNodeId == m_nNumPathNodes) {
					RestorePreviousState();
					ClearFollowPath();
					SetFollowPath(m_followPathDestPos, m_followPathAbortDist, m_followPathMoveState, m_followPathWalkAroundEnt,
						m_followPathTargetEnt, m_pathNodeTimer - CTimer::GetTimeInMilliseconds());
				}
			}
		} else {
			RestorePreviousState();
			ClearFollowPath();
			m_pathNodeTimer = 0;
		}
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
	if (reason->IsVehicle() && veh->IsCar()) {
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

		// FIX: Peds no more select dive direction randomly. Taken from SetEvasiveStep, last if statement inverted
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

		// VC's primitve solution to dive direction problem, see above for better one. This part doesn't exist on III at all
#ifndef FIX_BUGS
		angleToFace += HALFPI;
		if (CGeneral::GetRandomNumber() & 1)
			angleToFace -= PI;
#endif
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
		if (ped->m_nPedState == PED_DIVE_AWAY) {
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
	if (m_attractor)
		GetPedAttractorManager()->DeRegisterPed(this, m_attractor);
	CPlayerPed *player = FindPlayerPed();
	if (player == this) {
		if (!player->m_bCanBeDamaged)
			return;
	}

	m_threatEntity = nil;
	if (DyingOrDead())
		return;

	CAnimBlendAssociation *dieAssoc = nil;
	if (m_nPedState == PED_FALL || m_nPedState == PED_GETUP)
		delta *=  0.5f;

	SetStoredState();
	ClearAll();
	m_fHealth = 0.0f;
	if (m_nPedState == PED_DRIVING) {
		if (!IsPlayer() && (!m_pMyVehicle || !m_pMyVehicle->IsBike()))
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
		dieAssoc = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, animId, delta);
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
	if (!CGame::nastyGame && animId == ANIM_STD_HIT_FLOOR) {
		if (dieAssoc) {
			dieAssoc->SetCurrentTime(dieAssoc->hierarchy->totalLength - 0.01f);
			dieAssoc->SetRun();
		}
	}
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
	if (!RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_DROWN))
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
		RemoveWeaponAnims(0, -1000.0f);
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
	if (m_nPedState != PED_CHAT) {
		m_nLastPedState = PED_NONE;
		SetStoredState();
	}

	SetPedState(PED_CHAT);
	SetMoveState(PEDMOVE_STILL);
	m_lookTimer = 0;
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
		m_chatTimer = CTimer::GetTimeInMilliseconds() + 30000;
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
	if (m_objective == OBJECTIVE_BUY_ICE_CREAM) {
		bBoughtIceCream = true;
		SetObjective(OBJECTIVE_NONE);
		SetWanderPath(CGeneral::GetRandomNumberInRange(0, 8));
	}
}

bool
CPed::FacePhone(void)
{
	// FIX: This function was broken since it's left unused early in development.
#ifdef FIX_BUGS
	float phoneDir = CGeneral::GetRadianAngleBetweenPoints(
		gPhoneInfo.m_aPhones[m_phoneId].m_vecPos.x, gPhoneInfo.m_aPhones[m_phoneId].m_vecPos.y,
		GetPosition().x, GetPosition().y);

	SetLookFlag(phoneDir, false);
	bool turnDone = TurnBody();
	if (turnDone) {
		SetIdle();
		ClearLookFlag();
		m_phoneTalkTimer = CTimer::GetTimeInMilliseconds() + 10000;
	}
	return turnDone;
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
	if (CTimer::GetTimeInMilliseconds() <= m_phoneTalkTimer)
		return false;

	SetIdle();
	gPhoneInfo.m_aPhones[m_phoneId].m_nState = PHONE_STATE_FREE;
	m_phoneId = -1;
	return true;
}

void
StartTalkingOnMobileCB(CAnimBlendAssociation* assoc, void* arg)
{
	CPed* ped = (CPed*)arg;
	if (ped->m_nPedState == PED_ANSWER_MOBILE)
		CAnimManager::BlendAnimation(ped->GetClump(), ASSOCGRP_STD, ANIM_STD_PHONE_TALK, 4.0f);
}

void
FinishTalkingOnMobileCB(CAnimBlendAssociation *assoc, void *arg)
{
	CPed *ped = (CPed*)arg;
	if (ped->m_storedWeapon != WEAPONTYPE_UNIDENTIFIED) {
		ped->RemoveWeaponModel(MI_MOBILE);
		ped->SetCurrentWeapon(ped->m_storedWeapon);
		ped->m_storedWeapon = WEAPONTYPE_UNIDENTIFIED;
	}
	ped->m_lookTimer = 0;
}

void
CPed::SetAnswerMobile(void)
{
	if (m_nPedState != PED_ANSWER_MOBILE && !DyingOrDead()) {
		SetPedState(PED_ANSWER_MOBILE);
		RemoveWeaponAnims(GetWeapon()->m_eWeaponType, -4.0f);
		CAnimBlendAssociation *assoc = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_PHONE_IN, 4.0f);
		assoc->SetFinishCallback(StartTalkingOnMobileCB, this);
		m_lookTimer = INT32_MAX;
		if (m_storedWeapon == WEAPONTYPE_UNIDENTIFIED)
			m_storedWeapon = GetWeapon()->m_eWeaponType;

		RemoveWeaponModel(-1);
	}
}

void
CPed::ClearAnswerMobile(void)
{
	if (m_nLastPedState == PED_ANSWER_MOBILE)
		m_nLastPedState = PED_NONE;

	if (RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_PHONE_TALK)) {
		CAnimBlendAssociation *assoc = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_PHONE_OUT, 8.0f);
		assoc->SetFinishCallback(FinishTalkingOnMobileCB, this);
	} else
		FinishTalkingOnMobileCB(nil, this);

	if (m_nPedState == PED_ANSWER_MOBILE) {
		m_nPedState = PED_IDLE;
		RestorePreviousState();
		m_pVehicleAnim = nil;
	}
}

void
CPed::AnswerMobile(void)
{
	if (!IsPedInControl())
		return;

	CAnimBlendAssociation *phoneInAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_PHONE_IN);
	CAnimBlendAssociation *phoneOutAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_PHONE_OUT);
	CAnimBlendAssociation *phoneTalkAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_PHONE_TALK);
	if (phoneInAssoc || phoneTalkAssoc || phoneOutAssoc) {
		if (phoneInAssoc) {
			if (phoneInAssoc->currentTime >= 0.85f && !m_pWeaponModel) {
				CBaseModelInfo *phoneModel = CModelInfo::GetModelInfo(MI_MOBILE);
				m_pWeaponModel = (RpAtomic*)phoneModel->CreateInstance();
				phoneModel->AddRef();
				m_wepModelID = MI_MOBILE;

				// They copied AddWeaponModel and forgot that here
				// bool unused = IsPlayer();
			}
		} else if (phoneOutAssoc) {
			if (phoneOutAssoc->currentTime >= 0.5f && phoneOutAssoc->currentTime - phoneOutAssoc->timeStep < 0.5f) {
				RemoveWeaponModel(MI_MOBILE);
				SetCurrentWeapon(m_storedWeapon);
				m_storedWeapon = WEAPONTYPE_UNIDENTIFIED;
			}
		}
	} else {
		CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_PHONE_TALK, 4.0f);
	}
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

	if (!CanSetPedState() || m_nPedState == PED_DRIVING)
		return;

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
		if (!vehToSeek->IsBike() && m_vehDoor && m_objective != OBJECTIVE_ENTER_CAR_AS_DRIVER) {
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

	if (bIsRunning ||
		vehToSeek->pDriver && distToDestSqr > sq(2.0f) && (Abs(vehToSeek->m_vecMoveSpeed.x) > 0.01f || Abs(vehToSeek->m_vecMoveSpeed.y) > 0.01f))
		SetMoveState(PEDMOVE_RUN);
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
			if (1.6f + GetPosition().z > dest.z && GetPosition().z - 0.5f < dest.z) {
#ifdef GTA_TRAIN
				if (vehToSeek->IsTrain()) {
					SetEnterTrain(vehToSeek, m_vehDoor);
				} else
#endif
				{
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
									if (vehToSeek->IsBike()) {
										if ((!m_leader || m_leader != vehToSeek->pDriver) &&
											((m_vehDoor == CAR_DOOR_LF || m_vehDoor == CAR_DOOR_RF || m_vehDoor == CAR_WINDSCREEN) && vehToSeek->pDriver ||
											(m_vehDoor == CAR_DOOR_LR || m_vehDoor == CAR_DOOR_RR) && vehToSeek->pPassengers[0])) {
											SetCarJack(vehToSeek);
										} else {
											SetEnterCar(vehToSeek, m_vehDoor);
										}
									} else if (!vehToSeek->bIsBus && (!m_leader || m_leader != vehToSeek->pDriver) &&
										(m_vehDoor == CAR_DOOR_LF && vehToSeek->pDriver || m_vehDoor == CAR_DOOR_RF && vehToSeek->pPassengers[0] || m_vehDoor == CAR_DOOR_LR && vehToSeek->pPassengers[1] || m_vehDoor == CAR_DOOR_RR && vehToSeek->pPassengers[2])) {
										SetCarJack(vehToSeek);
										if (m_objective == OBJECTIVE_ENTER_CAR_AS_DRIVER && m_vehDoor != CAR_DOOR_LF)
											vehToSeek->pDriver->bFleeAfterExitingCar = true;
									} else {
										SetEnterCar(vehToSeek, m_vehDoor);
									}
									break;
								case STATUS_ABANDONED:
									if (vehToSeek->IsBike()) {
										if ((m_vehDoor == CAR_DOOR_LR || m_vehDoor == CAR_DOOR_RR) && vehToSeek->pPassengers[0]) {
											if (vehToSeek->pPassengers[0]->bDontDragMeOutCar) {
												if (IsPlayer())
													SetEnterCar(vehToSeek, m_vehDoor);
											} else {
												SetCarJack(vehToSeek);
											}
										} else {
											SetEnterCar(vehToSeek, m_vehDoor);
										}
									} else if (m_vehDoor == CAR_DOOR_RF && vehToSeek->pPassengers[0]) {
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
#if 0
	return m_nPedType == PEDTYPE_PLAYER1; // Original
#else
	// We still have those in enum, so let's also check for them.
	return m_nPedType == PEDTYPE_PLAYER1 || m_nPedType == PEDTYPE_PLAYER2 ||
		m_nPedType == PEDTYPE_PLAYER3 || m_nPedType == PEDTYPE_PLAYER4;
#endif
}

bool
CPed::IsGangMember(void) const
{
	return m_nPedType >= PEDTYPE_GANG1 && m_nPedType <= PEDTYPE_GANG9;
}

bool
IsPedPointerValid(CPed* pPed)
{
	if (!IsPedPointerValid_NotInWorld(pPed))
		return false;
	if (pPed->bInVehicle && pPed->m_pMyVehicle)
		return IsEntityPointerValid(pPed->m_pMyVehicle);
	return pPed->m_entryInfoList.first || pPed == FindPlayerPed();
}

bool
IsPedPointerValid_NotInWorld(CPed* pPed)
{
	if (!pPed)
		return false;
	int index = CPools::GetPedPool()->GetJustIndex_NoFreeAssert(pPed);
#ifdef FIX_BUGS
	if (index < 0 || index >= NUMPEDS)
#else
	if (index < 0 || index > NUMPEDS)
#endif
		return false;
	return true;
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
	bool notYet = false;
	if (CReplay::IsPlayingBack())
		return;

	if (bChangedSeat) {
		if (m_pMyVehicle->IsBike()) {
			if (RpAnimBlendClumpGetAssociation(GetClump(), ANIM_BIKE_JUMPON_LHS)
				|| RpAnimBlendClumpGetAssociation(GetClump(), ANIM_BIKE_JUMPON_RHS)
				|| RpAnimBlendClumpGetAssociation(GetClump(), ANIM_BIKE_KICK)) {
				LineUpPedWithCar(LINE_UP_TO_CAR_START);
				return;
			}
			bChangedSeat = false;
		} else {
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
				|| RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_COACH_GET_IN_RHS)
				|| RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_CAR_JUMP_IN_LO_LHS)) {
				notYet = true;
			}
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
		if (!m_pMyVehicle->IsBoat() && !m_pMyVehicle->IsBike())
			seatPos.x = -seatPos.x;

	} else if (m_pMyVehicle->pPassengers[0] == this) {
		seatPos = m_pMyVehicle->IsBike() ? vehModel->m_positions[CAR_POS_BACKSEAT]: vehModel->GetFrontSeatPosn();

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
	if (m_pMyVehicle->IsBike()) {
		((CBike*)m_pMyVehicle)->CalculateLeanMatrix();
		newMat = ((CBike*)m_pMyVehicle)->m_leanMatrix;
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
			tempMat.Translate(0.0f, 0.6f, 0.0f);
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
	SetMatrix(newMat);
}

void
CPed::LookForSexyPeds(void)
{
	if ((!IsPedInControl() && m_nPedState != PED_DRIVING)
		|| m_lookTimer >= CTimer::GetTimeInMilliseconds() ||
#ifdef FIX_BUGS
	   (m_nPedType != PEDTYPE_CIVMALE) && !IsFemale() && (m_nPedType != PEDTYPE_CRIMINAL) && !IsGangMember()
#else
		m_nPedType != PEDTYPE_CIVMALE
#endif
		)
		return;

	for (int i = 0; i < m_numNearPeds; i++) {
		if (CanSeeEntity(m_nearPeds[i])) {
			if ((GetPosition() - m_nearPeds[i]->GetPosition()).Magnitude() < 10.0f) {
				CPed *nearPed = m_nearPeds[i];
				if((nearPed->m_pedStats->m_sexiness > m_pedStats->m_sexiness)
#ifdef FIX_BUGS
				   && ((IsFemale() && !nearPed->IsFemale()) || (!IsFemale() && nearPed->IsFemale()))) {
#else
				   && nearPed->m_nPedType == PEDTYPE_CIVFEMALE) {
#endif

					SetLookFlag(nearPed, true);
					m_lookTimer = CTimer::GetTimeInMilliseconds() + 4000;
#ifdef FIX_BUGS
					Say(IsFemale() ? SOUND_PED_CHAT_SEXY_FEMALE : SOUND_PED_CHAT_SEXY_MALE);
#else
					Say(SOUND_PED_CHAT_SEXY_MALE);
#endif
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
PlayRandomAnimationsFromAnimBlock(CPed* ped, AssocGroupId animGroup, uint32 first, uint32 amount)
{
	if (!ped->IsPedInControl())
		return;

	const char *groupName = CAnimManager::GetAnimGroupName(animGroup);
	CAnimBlock *animBlock = CAnimManager::GetAnimationBlock(groupName);
	CAnimBlendAssociation *assoc;
	for (assoc = RpAnimBlendClumpGetFirstAssociation(ped->GetClump()); assoc; assoc = RpAnimBlendGetNextAssociation(assoc)) {
		int first = animBlock->firstIndex;
		int index = assoc->hierarchy - CAnimManager::GetAnimation(0);
		if (index >= first && index < first + animBlock->numAnims) {
			break;
		}
	}

	if (CTimer::GetTimeInMilliseconds() > ped->m_nWaitTimer && assoc)
		assoc->flags &= ~ASSOC_REPEAT;

	if (!assoc || assoc->blendDelta < 0.0f) {
		int selectedAnimOffset;
		do
			selectedAnimOffset = CGeneral::GetRandomNumberInRange(0, amount);
		while (assoc && first + selectedAnimOffset == assoc->animId);

		assoc = CAnimManager::BlendAnimation(ped->GetClump(), animGroup, (AnimationId)(first + selectedAnimOffset), 3.0f);

		assoc->SetFinishCallback(CPed::FinishedWaitCB, ped);
		if (assoc->flags & ASSOC_REPEAT)
			ped->m_nWaitTimer = CTimer::GetTimeInMilliseconds() + CGeneral::GetRandomNumberInRange(3000, 8000);
		else
			ped->m_nWaitTimer = CTimer::GetTimeInMilliseconds() + 8000;
	}
}

void
CPed::ClearWaitState(void)
{
	CAnimBlendAssociation *assoc;
	switch (m_nWaitState) {
		case WAITSTATE_PLAYANIM_CHAT:
		case WAITSTATE_SIT_DOWN:
		case WAITSTATE_SIT_DOWN_RVRS:
		case WAITSTATE_SIT_UP:
		case WAITSTATE_SIT_IDLE:
		case WAITSTATE_USE_ATM:
			if (CTimer::GetTimeInMilliseconds() <= m_nWaitTimer) {
				if (m_nWaitState == WAITSTATE_USE_ATM) {
					assoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_ATM);
					if (assoc)
						assoc->blendDelta = -8.0f;
					if (m_attractor)
						GetPedAttractorManager()->DeRegisterPed(this, m_attractor);

				} else if (m_nWaitState == WAITSTATE_PLAYANIM_CHAT) {
					assoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_CHAT);
					if (assoc)
						assoc->blendDelta = -8.0f;
					if (m_attractor)
						GetPedAttractorManager()->DeRegisterPed(this, m_attractor);

				} else if (m_nWaitState == WAITSTATE_SIT_DOWN || m_nWaitState == WAITSTATE_SIT_DOWN_RVRS || m_nWaitState == WAITSTATE_SIT_IDLE || m_nWaitState == WAITSTATE_SIT_UP) {
					switch (m_nWaitState) {
						case WAITSTATE_SIT_DOWN:
							assoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_SEAT_DOWN);
							if (assoc)
								assoc->blendDelta = -8.0f;
							break;
						case WAITSTATE_SIT_IDLE:
							assoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_SEAT_IDLE);
							if (assoc)
								assoc->blendDelta = -8.0f;
							break;
						case WAITSTATE_SIT_UP:
							assoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_SEAT_UP);
							if (assoc)
								assoc->blendDelta = -8.0f;
							break;
						default:
							break;
					}
					if (m_attractor)
						GetPedAttractorManager()->DeRegisterPed(this, m_attractor);
				}
			}
			break;
		case WAITSTATE_RIOT:
		{
			CAnimBlock* riotAnimBlock = CAnimManager::GetAnimationBlock("riot");

			for (assoc = RpAnimBlendClumpGetFirstAssociation(GetClump()); assoc; assoc = RpAnimBlendGetNextAssociation(assoc)) {
				int first = riotAnimBlock->firstIndex;
				int index = assoc->hierarchy - CAnimManager::GetAnimation(0);
				if (index >= first && index < first + riotAnimBlock->numAnims) {
					assoc->blendDelta = -1000.0f;
				}
			}
			break;
		}
		case WAITSTATE_FAST_FALL:
			if (RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_HIGHIMPACT_FRONT))
				SetGetUp();

			break;
		case WAITSTATE_BOMBER:
			assoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_DETONATE);
			if (assoc)
				assoc->blendDelta = -8.0f;
			break;
		case WAITSTATE_STRIPPER:
		{
			CAnimBlock* stripAnimBlock = CAnimManager::GetAnimationBlock("strip");

			for (assoc = RpAnimBlendClumpGetFirstAssociation(GetClump()); assoc; assoc = RpAnimBlendGetNextAssociation(assoc)) {
				int first = stripAnimBlock->firstIndex;
				int index = assoc->hierarchy - CAnimManager::GetAnimation(0);
				if (index >= first && index < first + stripAnimBlock->numAnims) {
					assoc->blendDelta = -1000.0f;
				}
			}
			break;
		}
		case WAITSTATE_LANCESITTING:
			assoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_SUNBATHE_IDLE);
			if (assoc)
				assoc->blendDelta = -8.0f;
			break;
		case WAITSTATE_PLAYANIM_HANDSUP_SIMPLE:
			assoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_HANDSUP);
			if (assoc)
				assoc->blendDelta = -8.0f;
			break;
		default:
			break;
	}
	m_nWaitState = WAITSTATE_FALSE;
}

void
CPed::SetWaitState(eWaitState state, void *time)
{
	AnimationId waitAnim = ANIM_STD_NUM;
	CAnimBlendAssociation *animAssoc;

	if (!IsPedInControl())
		return;

	if (m_nWaitState == WAITSTATE_RIOT && state != WAITSTATE_FALSE)
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
		case WAITSTATE_RIOT:
		case WAITSTATE_STRIPPER:
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

			// Random char as passenger? Cop, medic etc.?
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
		case WAITSTATE_SIT_DOWN:
			animAssoc = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_SEAT_DOWN, 4.0f);
			animAssoc->SetFinishCallback(FinishedWaitCB, this);
			m_nWaitTimer = CTimer::GetTimeInMilliseconds() + 100000;
			break;
		case WAITSTATE_SIT_UP:
			animAssoc = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_SEAT_UP, 4.0f);
			animAssoc->SetFinishCallback(FinishedWaitCB, this);
			m_nWaitTimer = CTimer::GetTimeInMilliseconds() + 100000;
			break;
		case WAITSTATE_SIT_IDLE:
			animAssoc = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_SEAT_IDLE, 128.f);
			animAssoc->SetFinishCallback(FinishedWaitCB, this);
			if (time)
				m_nWaitTimer = CTimer::GetTimeInMilliseconds() + *(int*)time;
			else
				m_nWaitTimer = CTimer::GetTimeInMilliseconds() + CGeneral::GetRandomNumberInRange(25000, 30000);
			break;
		case WAITSTATE_USE_ATM:
			animAssoc = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_ATM, 4.0f);
			animAssoc->SetFinishCallback(FinishedWaitCB, this);
			if (time)
				m_nWaitTimer = CTimer::GetTimeInMilliseconds() + *(int*)time;
			else
				m_nWaitTimer = CTimer::GetTimeInMilliseconds() + 100000;
			break;
		case WAITSTATE_SUN_BATHE_IDLE:
			m_headingRate = 0.0f;
			animAssoc = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_SUNBATHE, ANIM_SUNBATHE_IDLE, 4.0f);
			animAssoc->SetDeleteCallback(DeleteSunbatheIdleAnimCB, this);
			m_nWaitTimer = CTimer::GetTimeInMilliseconds() + CGeneral::GetRandomNumberInRange(50000, 100000);
			break;
		case WAITSTATE_FAST_FALL:
			SetFall(-1, ANIM_STD_HIGHIMPACT_FRONT, true);
			break;
		case WAITSTATE_BOMBER:
			CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_DETONATE, 4.0f);
			m_nWaitTimer = CTimer::GetTimeInMilliseconds() + *(int*)time;
			break;
		case WAITSTATE_GROUND_ATTACK:
		{
			CWeaponInfo* currentWeapon = CWeaponInfo::GetWeaponInfo(GetWeapon()->m_eWeaponType);
			if (!currentWeapon)
				break;
			if (GetFireAnimGround(currentWeapon, false)) {
				if (!RpAnimBlendClumpGetAssociation(GetClump(), GetFireAnimGround(currentWeapon, false))) {
					m_nWaitTimer = CTimer::GetTimeInMilliseconds() + 5000;
					CAnimBlendAssociation* newAnim = CAnimManager::BlendAnimation(GetClump(),
						currentWeapon->m_AnimToPlay, GetFireAnimGround(currentWeapon, false), 8.0f);
					newAnim->SetDeleteCallback(FinishedWaitCB, this);
				}
			}
			break;
		}
		case WAITSTATE_LANCESITTING:
			CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_LANCE, ANIM_SUNBATHE_IDLE, 4.0f);
			break;
		case WAITSTATE_PLAYANIM_HANDSUP_SIMPLE:
			animAssoc = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_HANDSUP, 4.0f);
			animAssoc->flags &= ~ASSOC_FADEOUTWHENDONE;
			animAssoc->flags |= ASSOC_DELETEFADEDOUT;
			animAssoc->SetDeleteCallback(FinishedWaitCB, this);
			m_nWaitTimer = CTimer::GetTimeInMilliseconds() + *(int*)time;
			break;
		default:
			ClearWaitState();
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
		ClearWaitState();
		RestoreHeadingRate();
		return;
	}

	switch (m_nWaitState) {

		case WAITSTATE_TRAFFIC_LIGHTS:
			if (CTimer::GetTimeInMilliseconds() > m_nWaitTimer) {
				if (CTrafficLights::LightForPeds() == PED_LIGHTS_WALK) {
					ClearWaitState();
					SetMoveState(PEDMOVE_WALK);
				}
			}
			break;

		case WAITSTATE_CROSS_ROAD:
			if (CTimer::GetTimeInMilliseconds() > m_nWaitTimer) {
				if (CGeneral::GetRandomNumber() & 1 || !m_nWaitTimer)
					ClearWaitState();
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
				ClearWaitState();
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
				ClearWaitState();
				SetMoveState(PEDMOVE_WALK);
			}
			break;

		case WAITSTATE_HITWALL:
			if (CTimer::GetTimeInMilliseconds() <= m_nWaitTimer) {
				if (m_collidingThingTimer > CTimer::GetTimeInMilliseconds()) {
					m_collidingThingTimer = CTimer::GetTimeInMilliseconds() + 2500;
				}
			} else {
				ClearWaitState();
			}
			break;

		case WAITSTATE_TURN180:
			if (CTimer::GetTimeInMilliseconds() > m_nWaitTimer) {
				ClearWaitState();
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
					ClearWaitState();
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
					ClearWaitState();
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
				ClearWaitState();
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

				if (pedWeLook)
					TurnBody();
			} else {
				ClearWaitState();
				m_nWaitTimer = 0;
				if (m_pLookTarget && m_pLookTarget->IsPed()) {
					if (m_nPedState != PED_FLEE_ENTITY && m_nPedState != PED_ATTACK) {
						if (bCrouchWhenScared) {
							if (bIsDucking) {
								ClearDuck(false);
								SetDuck(10000, true);
							}

						} else if (m_pedStats->m_fear <= 100 - pedWeLook->m_pedStats->m_temper) {
							if (GetWeapon()->IsTypeMelee()) {
								if(m_pedStats->m_flags & STAT_GUN_PANIC) {
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
								} else {
									SetObjective(OBJECTIVE_NONE);
									SetWanderPath(CGeneral::GetRandomNumberInRange(0.0f, 8.0f));
								}
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
				if (m_attractor && m_objective == OBJECTIVE_WAIT_ON_FOOT_AT_ICE_CREAM_VAN) {
					GetPedAttractorManager()->BroadcastDeparture(this, m_attractor);
					bBoughtIceCream = true;
				}
				ClearWaitState();
			} else if (m_nWaitState == WAITSTATE_PLAYANIM_TAXI) {
				if (m_pedInObjective) {
					if (m_objective == OBJECTIVE_GOTO_CHAR_ON_FOOT || m_objective == OBJECTIVE_KILL_CHAR_ON_FOOT) {

						if (m_pLookTarget)
							m_pLookTarget->CleanUpOldReference(&m_pLookTarget);
						m_pLookTarget = m_pedInObjective;
						m_pLookTarget->RegisterReference((CEntity **) &m_pLookTarget);
						TurnBody();
					}
				}
			}
			break;

		case WAITSTATE_FINISH_FLEE:
			animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_IDLE_TIRED);
			if (animAssoc) {
				if (CTimer::GetTimeInMilliseconds() > m_nWaitTimer) {
					animAssoc->flags |= ASSOC_DELETEFADEDOUT;
					CAnimManager::BlendAnimation(GetClump(), m_animGroup, ANIM_STD_IDLE, 4.0f);
					int timer = 2000;
					ClearWaitState();
					SetWaitState(WAITSTATE_CROSS_ROAD_LOOK, &timer);
				}
			} else {
				ClearWaitState();
			}
			break;
		case WAITSTATE_SIT_DOWN:
			if (CTimer::GetTimeInMilliseconds() > m_nWaitTimer) {
				ClearWaitState();
				SetWaitState(WAITSTATE_SIT_IDLE, 0);
			}
			break;
		//case WAITSTATE_SIT_DOWN_RVRS:
		case WAITSTATE_SIT_UP:
			if (CTimer::GetTimeInMilliseconds() > m_nWaitTimer) {
				if (m_attractor)
					GetPedAttractorManager()->BroadcastDeparture(this, m_attractor);
				ClearWaitState();
				if (bFleeWhenStanding) {
					if (m_threatEx) {
						SetFlee(m_threatEx, 10000);
						bFleeWhenStanding = false;
						m_threatEx = nil;
						Say(SOUND_PED_FLEE_SPRINT);
					}
				}
			}
			break;
		case WAITSTATE_SIT_IDLE:
			if (bTurnedAroundOnAttractor) {
				m_fRotationCur += PI;
				m_fRotationCur = CGeneral::LimitRadianAngle(m_fRotationCur);
				m_fRotationDest = m_fRotationCur;
				bTurnedAroundOnAttractor = false;
			}
			if (CTimer::GetTimeInMilliseconds() > m_nWaitTimer) {
				ClearWaitState();
				SetWaitState(WAITSTATE_SIT_UP, 0);
			} else {
				if (m_fleeFrom && m_fleeFrom->IsVehicle()) {
					m_pNextPathNode = nil;
					m_threatEx = m_threatEntity;
					bFleeWhenStanding = true;
					ClearWaitState();
					SetWaitState(WAITSTATE_SIT_UP, 0);
				} else {
					uint32 threatFlag = ScanForThreats();
					if (threatFlag == PED_FLAG_GUN || threatFlag == PED_FLAG_EXPLOSION || threatFlag == PED_FLAG_DEADPEDS) {
						m_pNextPathNode = nil;
						m_threatEx = m_threatEntity;
						bFleeWhenStanding = true;
						ClearWaitState();
						SetWaitState(WAITSTATE_SIT_UP, 0);
					}
				}
			}
			break;
		case WAITSTATE_USE_ATM:
			if (CTimer::GetTimeInMilliseconds() > m_nWaitTimer) {
				if (m_attractor)
					GetPedAttractorManager()->BroadcastDeparture(this, m_attractor);
				ClearWaitState();
			}
			break;
		case WAITSTATE_SUN_BATHE_IDLE:
			if (CTimer::GetTimeInMilliseconds() > m_nWaitTimer && bCanGiveUpSunbathing) {
				m_pNextPathNode = nil;
				bGotUpOfMyOwnAccord = true;
				SetGetUp();
				ClearWaitState();

			} else if (CWeather::Rain <= 0.1f) {
				if (CClock::GetHours() <= 18 || CGeneral::GetRandomNumberInRange(0.f, 1.0f) < 0.005f) {
					uint32 threatFlag = ScanForThreats();
					if (threatFlag == PED_FLAG_GUN || threatFlag == PED_FLAG_EXPLOSION || threatFlag == PED_FLAG_DEADPEDS) {
						// Get up in case of danger
						m_pNextPathNode = nil;
						m_threatEx = m_threatEntity;
						bFleeWhenStanding = true;
						SetGetUp();
						ClearWaitState();
					}
					CPlayerPed *player = FindPlayerPed();
					if (player) {
						// Get up if player coming towards us with a car
						if (player->InVehicle()){
							CVector vehSpeedPerSec = player->m_pMyVehicle->m_vecMoveSpeed * GAME_SPEED_TO_METERS_PER_SECOND;
							CVector vehPos = player->m_pMyVehicle->GetPosition();
							CVector ourPos = GetPosition();
							float timeUntilVehReachPed = DotProduct(ourPos - vehPos, vehSpeedPerSec) / vehSpeedPerSec.MagnitudeSqr();
							if (timeUntilVehReachPed > 0.0 && timeUntilVehReachPed < 8.0f) {
								if ((ourPos - (timeUntilVehReachPed * vehSpeedPerSec + vehPos)).Magnitude() < 5.0f) {
									m_pNextPathNode = nil;
									m_threatEx = player;
									bFleeWhenStanding = true;
									SetGetUp();
									ClearWaitState();
								}
							}
						}
					}
				} else {
					m_pNextPathNode = nil;
					bGotUpOfMyOwnAccord = true;
					SetGetUp();
					ClearWaitState();
				}
			} else {
				m_pNextPathNode = nil;
				bGotUpOfMyOwnAccord = true;
				SetGetUp();
				ClearWaitState();
			}
			break;
		case WAITSTATE_RIOT:
			if (m_nPedState == PED_FLEE_ENTITY || m_nPedState == PED_ATTACK) {
				ClearWaitState();
				break;
			}

			PlayRandomAnimationsFromAnimBlock(this, ASSOCGRP_RIOT, ANIM_RIOT_ANGRY, ANIM_RIOT_FUCKYOU - ANIM_RIOT_ANGRY + 1);
			if (IsPedInControl() && CGeneral::GetRandomNumberInRange(0.f,1.f) < 0.25f
				&& CPopulation::CanJeerAtStripper(m_modelIndex)) {
				for (int i = 0; i < m_numNearPeds; ++i) {
					CPed *nearPed = m_nearPeds[i];
					if (nearPed) {
						if ((GetPosition() - nearPed->GetPosition()).MagnitudeSqr() < sq(10.f)) {
							for (int anim = ANIM_STRIP_A; anim <= ANIM_STRIP_G; anim++) {
								if (RpAnimBlendClumpGetAssociation(nearPed->GetClump(), anim))
									Say(SOUND_PED_JEER);
							}
						}
					}
				}
			}
			break;
		case WAITSTATE_BOMBER:
			if (CTimer::GetTimeInMilliseconds() > m_nWaitTimer)
				ClearWaitState();
			break;
		case WAITSTATE_STRIPPER:
			PlayRandomAnimationsFromAnimBlock(this, ASSOCGRP_STRIP, ANIM_STRIP_A, ANIM_STRIP_G - ANIM_STRIP_A + 1);
			break;
		case WAITSTATE_PLAYANIM_HANDSUP_SIMPLE:
			if (CTimer::GetTimeInMilliseconds() > m_nWaitTimer)
				ClearWaitState();
			break;
		default:
			break;
	}

	if(!m_nWaitState)
		RestoreHeadingRate();
}

void
CPed::DeleteSunbatheIdleAnimCB(CAnimBlendAssociation *assoc, void *arg)
{
	CPed *ped = (CPed*) arg;

	if (CTimer::GetTimeInMilliseconds() <= ped->m_nWaitTimer
		&& !ped->bGotUpOfMyOwnAccord && !ped->bFleeWhenStanding && !ped->m_threatEx) {
		ped->m_pNextPathNode = nil;
		ped->bFleeWhenStanding = true;
		ped->m_threatEx = FindPlayerPed();
		ped->SetGetUp();
		ped->ClearWaitState();
	}
	ped->m_nWaitTimer = 0;
	ped->RestoreHeadingRate();
	ped->Wait();
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
	((CPed*)arg)->RestoreHeadingRate();
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
		Say(SOUND_PED_SOLICIT);
		if (FindPlayerVehicle() == m_carInObjective) {
			FindPlayerPed()->Say(SOUND_PED_SOLICIT);
		}
		SetMoveState(PEDMOVE_STILL);

		// Game uses GetAngleBetweenPoints and converts it to radian
		m_fRotationDest = CGeneral::GetRadianAngleBetweenPoints(
			doorPos.x, doorPos.y,
			GetPosition().x, GetPosition().y);

		if (m_fRotationDest < 0.0f) {
			m_fRotationDest += TWOPI;
		} else if (m_fRotationDest > TWOPI) {
			m_fRotationDest -= TWOPI;
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
		Say(SOUND_PED_SOLICIT);
	}
}

void
CPed::SetBuyIceCream(void)
{
	if (m_nPedState == PED_BUY_ICECREAM || !IsPedInControl())
		return;

	if (!m_carInObjective)
		return;

	SetPedState(PED_BUY_ICECREAM);
}

void
CPed::BuyIceCream(void)
{
	if (m_carInObjective) {
		CPed *driver = m_carInObjective->pDriver;
		if (driver && CTimer::GetTimeInMilliseconds() > m_chatTimer) {
			SetChat(driver, 8000);
			driver->SetChat(this, 8000);
			return;
		}
		SetObjective(OBJECTIVE_NONE);
		SetWanderPath(CGeneral::GetRandomNumberInRange(0, 8));
	} else {
		SetObjective(OBJECTIVE_NONE);
		SetWanderPath(CGeneral::GetRandomNumberInRange(0, 8));
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

	if (m_leader) {
		m_leader->bIsLeader = true;
		m_leader->RegisterReference((CEntity**)&m_leader);
	}
}

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

		CColModel *ourCol = GetColModel();
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

void
CPed::SetJump(void)
{
	if (!bInVehicle && m_nPedState != PED_JUMP && !RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_JUMP_LAUNCH) &&
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

	CVector forward(0.09f * ped->GetForward() + ped->GetPosition());
	forward.z += CModelInfo::GetModelInfo(ped->GetModelIndex())->GetColModel()->spheres[2].center.z + 0.35f;

	CEntity *obstacle = CWorld::TestSphereAgainstWorld(forward, 0.25f, nil, true, true, false, true, false, false);
	if (!obstacle) {
		// Forward of forward
		forward += 0.15f * ped->GetForward();
		forward.z += 0.15f;
		obstacle = CWorld::TestSphereAgainstWorld(forward, 0.25f, nil, true, true, false, true, false, false);
	}

	if (!obstacle && CCullZones::CamStairsForPlayer() && CCullZones::FindZoneWithStairsAttributeForPlayer())
		obstacle = ped;

	if (obstacle) {
		animAssoc->flags |= ASSOC_DELETEFADEDOUT;
		CAnimBlendAssociation *handsCoverAssoc = CAnimManager::BlendAnimation(ped->GetClump(), ASSOCGRP_STD, ANIM_STD_HIT_WALL, 8.0f);
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

	if (ped->IsPlayer() || ped->m_pedInObjective && ped->m_pedInObjective->IsPlayer())
		ped->ApplyMoveForce(0.0f, 0.0f, 8.5f);
	else
		ped->ApplyMoveForce(0.0f, 0.0f, 4.5f);
	
	if (sq(velocityFromAnim) > ped->m_vecMoveSpeed.MagnitudeSqr2D() || ped->m_pCurrentPhysSurface) {

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

		if (ped->m_pCurrentPhysSurface) {
			ped->m_vecMoveSpeed.x += ped->m_pCurrentPhysSurface->m_vecMoveSpeed.x;
			ped->m_vecMoveSpeed.y += ped->m_pCurrentPhysSurface->m_vecMoveSpeed.y;
		}
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
void
CPed::SetRadioStation(void)
{
	CPedModelInfo* modelInfo = (CPedModelInfo*)CModelInfo::GetModelInfo(GetModelIndex());

	if (IsPlayer() || !m_pMyVehicle || m_pMyVehicle->pDriver != this)
		return;

	if (GetModelIndex() != MI_PGA && GetModelIndex() != MI_PGB) {
		if (m_pMyVehicle->m_nRadioStation != modelInfo->radio1 && m_pMyVehicle->m_nRadioStation != modelInfo->radio2) {
			if (CGeneral::GetRandomTrueFalse())
				m_pMyVehicle->m_nRadioStation = modelInfo->radio1;
			else
				m_pMyVehicle->m_nRadioStation = modelInfo->radio2;
		}
	} else {
		m_pMyVehicle->m_nRadioStation = DMAudio.GetFavouriteRadioStation();
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
		if (car->IsBike() && !car->pPassengers[0]) {
			car->pPassengers[0] = this;
			car->pPassengers[0]->RegisterReference((CEntity**) &car->pPassengers[0]);
		}
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

	RpAnimBlendClumpSetBlendDeltas(GetClump(), ASSOC_PARTIAL, -1000.0f);

	AddInCarAnims(car, car->pDriver == this);
	RemoveWeaponWhenEnteringVehicle();

	if (car->bIsBus)
		bRenderPedInCar = false;

	bChangedSeat = true;
}

bool
CPed::HasAttractor(void)
{
	return m_attractor != nil;
}

void
CPed::SetNewAttraction(CPedAttractor* pAttractor, const CVector& pos, float heading, float time, int32 qid)
{
	if (!m_attractor)
		m_attractor = pAttractor;
	if (m_attractor != pAttractor)
		return;
	switch (pAttractor->GetEffect()->pedattr.type) {
	case ATTRACTOR_ATM: SetObjective(OBJECTIVE_GOTO_ATM_ON_FOOT, heading, pos); break;
	case ATTRACTOR_SEAT: SetObjective(OBJECTIVE_GOTO_SEAT_ON_FOOT, heading, pos); break;
	case ATTRACTOR_STOP: SetObjective(OBJECTIVE_GOTO_BUS_STOP_ON_FOOT, heading, pos); break;
	case ATTRACTOR_PIZZA: SetObjective(OBJECTIVE_GOTO_PIZZA_ON_FOOT, heading, pos); break;
	case ATTRACTOR_SHELTER: SetObjective(OBJECTIVE_GOTO_SHELTER_ON_FOOT, heading, pos); break;
	case ATTRACTOR_ICECREAM: SetObjective(OBJECTIVE_GOTO_ICE_CREAM_VAN_ON_FOOT, heading, pos); break;
	default: return;
	}
	SetObjectiveTimer(time);
	m_positionInQueue = qid;
}

void
CPed::AttachPedToEntity(CEntity *ent, CVector offset, uint16 type, float rot, eWeaponType weapon)
{
	if (!ent || bInVehicle)
		return;

	m_attachedTo = ent;
	m_attachedTo->RegisterReference(&m_attachedTo);
	m_vecAttachOffset = offset;
	m_attachType = type;
	m_attachRotStep = rot;
	if (IsPlayer()) {
		bUsesCollision = false;
	} else if (ent->IsVehicle()) {
		m_pCollidingEntity = ent;
	}

	if (IsPlayer()) {
		m_objective = OBJECTIVE_NONE;
		m_prevObjective = OBJECTIVE_NONE;
	}
	SetStoredState();
	SetPedState(PED_IDLE);
	CAnimManager::BlendAnimation(GetClump(), m_animGroup, ANIM_STD_IDLE, 1000.0f);

	if (m_storedWeapon == WEAPONTYPE_UNIDENTIFIED) {
		m_storedWeapon = GetWeapon()->m_eWeaponType;
		m_attachWepAmmo = GetWeapon()->m_nAmmoTotal;
	}
	if (IsPlayer()) {
		GiveWeapon(weapon, 30000, 1);
#ifndef FIX_BUGS
		((CPlayerPed*)this)->m_nSelectedWepSlot = weapon;
#else
		((CPlayerPed*)this)->m_nSelectedWepSlot = GetWeaponSlot(weapon);
#endif
		((CPlayerPed*)this)->MakeChangesForNewWeapon(weapon);
		TheCamera.SetNewPlayerWeaponMode(CCam::MODE_HELICANNON_1STPERSON, 0, 0);
		SetPedState(PED_SNIPER_MODE);
	} else {
		GiveWeapon(weapon, 30000, true);
		SetCurrentWeapon(weapon);
	}

	PositionAttachedPed();
}

void
CPed::DettachPedFromEntity(void)
{
	CEntity* pVehicleAttachedTo = m_attachedTo;
	m_attachedTo = nil;
	if (m_nPedState == PED_DIE) {
		m_pCollidingEntity = pVehicleAttachedTo;
		ApplyMoveForce(pVehicleAttachedTo->GetForward() * -4.0f + CVector(0.0f, 0.0f, 4.0f));
		bIsStanding = false;
	} else if (m_nPedState != PED_DEAD) {
		RestorePreviousState();
		CAnimManager::BlendAnimation(GetClump(), m_animGroup, ANIM_STD_IDLE, 1000.0f);
		bUsesCollision = true;
		if (m_storedWeapon != WEAPONTYPE_UNIDENTIFIED) {
			GetWeapon()->m_nAmmoInClip = 0;
			GetWeapon()->m_nAmmoTotal = 0;
			SetCurrentWeapon(m_storedWeapon);
			GetWeapon()->m_nAmmoTotal = m_attachWepAmmo;
			m_storedWeapon = WEAPONTYPE_UNIDENTIFIED;
		}
	}
}

void
CPed::PositionAttachedPed()
{
	if(!m_attachedTo)
		return;

	CMatrix rotMatrix, targetMat;
	targetMat = m_attachedTo->GetMatrix();
	targetMat.GetPosition() += Multiply3x3(m_attachedTo->GetMatrix(), m_vecAttachOffset);
	float objAngle = m_attachedTo->GetForward().Heading();

	if (!IsPlayer()) {
		float targetAngle = objAngle;
		switch (m_attachType) {
			case 1:
				targetAngle += HALFPI;
				break;
			case 2:
				targetAngle += PI;
				break;
			case 3:
				targetAngle -= HALFPI;
				break;
			default:
				break;
		}
		targetAngle = CGeneral::LimitRadianAngle(targetAngle);
		m_fRotationCur = CGeneral::LimitRadianAngle(m_fRotationCur);
		float neededTurn = m_fRotationCur - targetAngle;

		if (neededTurn > PI)
			neededTurn -= TWOPI;
		else if (neededTurn < -PI)
			neededTurn += TWOPI;

		if (neededTurn > m_attachRotStep)
			m_fRotationCur = CGeneral::LimitRadianAngle(targetAngle + m_attachRotStep);
		else if (-m_attachRotStep > neededTurn)
			m_fRotationCur = CGeneral::LimitRadianAngle(targetAngle - m_attachRotStep);
		else
			m_fRotationCur = CGeneral::LimitRadianAngle(m_fRotationCur);
	}
	rotMatrix.SetRotateZ(m_fRotationCur - objAngle);
	targetMat = targetMat * rotMatrix;
	GetMatrix() = targetMat;
	if (m_attachedTo->IsVehicle() || m_attachedTo->IsObject()) {
		m_vecMoveSpeed = ((CPhysical*)m_attachedTo)->m_vecMoveSpeed;
		m_vecTurnSpeed = ((CPhysical*)m_attachedTo)->m_vecTurnSpeed;
	}
}

void
CPed::Undress(const char* name)
{
	int mi = GetModelIndex();
	CAnimBlendAssociation* pAnim = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_PHONE_OUT);
	if (pAnim)
		FinishTalkingOnMobileCB(pAnim, this);

	DeleteRwObject();
	if (IsPlayer())
		mi = MI_PLAYER;
	CStreaming::RequestSpecialModel(mi, name, STREAMFLAGS_DEPENDENCY | STREAMFLAGS_SCRIPTOWNED);
	CWorld::Remove(this);
}

void
CPed::Dress(void)
{
	int mi = GetModelIndex();
	m_modelIndex = -1;
	SetModelIndex(mi);
	m_nPedState = PED_IDLE;
	m_nLastPedState = PED_NONE;
	m_objective = OBJECTIVE_NONE;
	m_prevObjective = OBJECTIVE_NONE;
	m_nWaitState = WAITSTATE_FALSE;
	CWorld::Add(this);
	RestoreHeadingRate();
}

void
CPed::Say(uint16 audio, int32 time)
{
	if (m_delayedSoundID == -1) {
		m_delayedSoundID = audio;
		m_delayedSoundTimer = CTimer::GetTimeInMilliseconds() + time;
	}
}

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
	SkipSaveBuf(buf, 499);
	CopyToBuf(buf, m_fHealth);
	CopyToBuf(buf, m_fArmour);
	SkipSaveBuf(buf, 172);
	for (int i = 0; i < 10; i++) // has to be hardcoded
		m_weapons[i].Save(buf);
	SkipSaveBuf(buf, 252);
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
	SkipSaveBuf(buf, 499);
	CopyFromBuf(buf, m_fHealth);
	CopyFromBuf(buf, m_fArmour);
	SkipSaveBuf(buf, 172);
	m_currentWeapon = WEAPONTYPE_UNARMED;

	CWeapon bufWeapon;
	for (int i = 0; i < 10; i++) { // has to be hardcoded
		bufWeapon.Load(buf);

		if (bufWeapon.m_eWeaponType != WEAPONTYPE_UNARMED) {
			int modelId = CWeaponInfo::GetWeaponInfo(bufWeapon.m_eWeaponType)->m_nModelId;
			if (modelId != -1) {
				CStreaming::RequestModel(modelId, STREAMFLAGS_DEPENDENCY);
				int modelId2 = CWeaponInfo::GetWeaponInfo(bufWeapon.m_eWeaponType)->m_nModel2Id;
				if (modelId2 != -1)
					CStreaming::RequestModel(modelId2, STREAMFLAGS_DEPENDENCY);

				CStreaming::LoadAllRequestedModels(false);
			}
			GiveWeapon(bufWeapon.m_eWeaponType, bufWeapon.m_nAmmoTotal, false);
		}
	}
	SkipSaveBuf(buf, 252);
}
#undef CopyFromBuf
#undef CopyToBuf
#endif
