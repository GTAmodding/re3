#include "common.h"
#include "patcher.h"
#include "Pools.h"
#include "Particle.h"
#include "Stats.h"
#include "World.h"
#include "DMaudio.h"
#include "RpAnimBlend.h"
#include "Ped.h"
#include "PlayerPed.h"
#include "General.h"
#include "VisibilityPlugins.h"
#include "AudioManager.h"
#include "HandlingMgr.h"
#include "Replay.h"
#include "PedPlacement.h"

bool &CPed::bNastyLimbsCheat = *(bool*)0x95CD44;
bool &CPed::bPedCheat2 = *(bool*)0x95CD5A;
bool &CPed::bPedCheat3 = *(bool*)0x95CD59;

CVector &CPed::offsetToOpenRegularCarDoor = *(CVector*)0x62E030;
CVector &CPed::offsetToOpenLowCarDoor = *(CVector*)0x62E03C;
CVector &CPed::offsetToOpenVanDoor = *(CVector*)0x62E048;

void *CPed::operator new(size_t sz) { return CPools::GetPedPool()->New();  }
void CPed::operator delete(void *p, size_t sz) { CPools::GetPedPool()->Delete((CPed*)p); }

WRAPPER void CPed::KillPedWithCar(CVehicle *veh, float impulse) { EAXJMP(0x4EC430); }
WRAPPER void CPed::Say(uint16 audio) { EAXJMP(0x4E5A10); }
WRAPPER void CPed::SetDie(AnimationId anim, float arg1, float arg2) { EAXJMP(0x4D37D0); }
WRAPPER void CPed::SpawnFlyingComponent(int, int8) { EAXJMP(0x4EB060); }
WRAPPER void CPed::RestorePreviousState(void) { EAXJMP(0x4C5E30); }
WRAPPER void CPed::ClearAttack(void) { EAXJMP(0x4E6790); }
WRAPPER void CPed::PedSetQuickDraggedOutCarPositionCB(CAnimBlendAssociation *dragAssoc, void *arg) { EAXJMP(0x4E2480); }
WRAPPER void CPed::PedSetDraggedOutCarPositionCB(CAnimBlendAssociation *dragAssoc, void *arg) { EAXJMP(0x4E2920); }
WRAPPER void CPed::SetPedPositionInCar(void) { EAXJMP(0x4D4970); }

static char ObjectiveText[34][28] = {
	"No Obj",
	"Wait on Foot",
	"Flee on Foot Till Safe",
	"Guard Spot",
	"Guard Area",
	"Wait in Car",
	"Wait in Car then Getout",
	"Kill Char on Foot",
	"Kill Char Any Means",
	"Flee Char on Foot Till Safe",
	"Flee Char on Foot Always",
	"GoTo Char on Foot",
	"Follow Char in Formation",
	"Leave Car",
	"Enter Car as Passenger",
	"Enter Car as Driver",
	"Follow Car in Car",
	"Fire at Obj from Vehicle",
	"Destroy Obj",
	"Destroy Car",
	"GoTo Area Any Means",
	"GoTo Area on Foot",
	"Run to Area",
	"GoTo Area in Car",
	"Follow Car on Foot Woffset",
	"Guard Attack",
	"Set Leader",
	"Follow Route",
	"Solicit",
	"Take Taxi",
	"Catch Train",
	"Buy IceCream",
	"Steal Any Car",
	"Mug Char",
};

static char StateText[56][18] = {
	"None",	// 1
	"Idle",
	"Look Entity",
	"Look Heading",
	"Wander Range",
	"Wander Path",
	"Seek Pos",
	"Seek Entity",
	"Flee Pos",
	"Flee Entity",
	"Pursue",
	"Follow Path",
	"Sniper Mode",
	"Rocket Mode",
	"Dummy",
	"Pause",
	"Attack",
	"Fight",
	"Face Phone",
	"Make Call",
	"Chat",
	"Mug",
	"AimGun",
	"AI Control",
	"Seek Car",
	"Seek InBoat",
	"Follow Route",
	"C.P.R.",
	"Solicit",
	"Buy IceCream",
	"Investigate",
	"Step away",
	"STATES_NO_AI",
	"On Fire",
	"Jump",
	"Fall",
	"GetUp",
	"Stagger",
	"Dive away",
	"STATES_NO_ST",
	"Enter Train",
	"Exit Train",
	"Arrest Plyr",
	"Driving",
	"Passenger",
	"Taxi Passngr",
	"Open Door",
	"Die",
	"Dead",
	"CarJack",
	"Drag fm Car",
	"Enter Car",
	"Steal Car",
	"Exit Car",
	"Hands Up",
	"Arrested",
};

static char PersonalityTypeText[32][18] = {
	"Player",
	"Cop",
	"Medic",
	"Fireman",
	"Gang 1",
	"Gang 2",
	"Gang 3",
	"Gang 4",
	"Gang 5",
	"Gang 6",
	"Gang 7",
	"Street Guy",
	"Suit Guy",
	"Sensible Guy",
	"Geek Guy",
	"Old Guy",
	"Tough Guy",
	"Street Girl",
	"Suit Girl",
	"Sensible Girl",
	"Geek Girl",
	"Old Girl",
	"Tough Girl",
	"Tramp",
	"Tourist",
	"Prostitute",
	"Criminal",
	"Busker",
	"Taxi Driver",
	"Psycho",
	"Steward",
	"Sports Fan",
};

static char WaitStateText[21][16] = {
	"No Wait",
	"Traffic Lights",
	"Pause CrossRoad",
	"Look CrossRoad",
	"Look Ped",
	"Look Shop",
	"Look Accident",
	"FaceOff Gang",
	"Double Back",
	"Hit Wall",
	"Turn 180deg",
	"Surprised",
	"Ped Stuck",
	"Look About",
	"Play Duck",
	"Play Cower",
	"Play Taxi",
	"Play HandsUp",
	"Play HandsCower",
	"Play Chat",
	"Finish Flee",
};

static RwObject*
RemoveAllModelCB(RwObject *object, void *data)
{
	RpAtomic *atomic = (RpAtomic*)object;
	if (CVisibilityPlugins::GetAtomicModelInfo(atomic)) {
		RpClumpRemoveAtomic(atomic->clump, atomic);
		RpAtomicDestroy(atomic);
	}
	return object;
}

static PedOnGroundState
CheckForPedsOnGroundToAttack(CPlayerPed *player, CPed **pedOnGround)
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

	for (int currentPedId = 0; currentPedId < player->m_numNearPeds; currentPedId++) {

		currentPed = player->m_nearPeds[currentPedId];

		CVector posDifference = currentPed->GetPosition() - player->GetPosition();
		distance = posDifference.Magnitude();

		if (distance < 2.0f) {
			angleToFace = CGeneral::GetRadianAngleBetweenPoints(
				currentPed->GetPosition().x, currentPed->GetPosition().y,
				player->GetPosition().x, player->GetPosition().y);

			angleToFace = CGeneral::LimitRadianAngle(angleToFace);
			player->m_fRotationCur = CGeneral::LimitRadianAngle(player->m_fRotationCur);

			angleDiff = fabs(angleToFace - player->m_fRotationCur);

			if (angleDiff > PI)
				angleDiff = 2 * PI - angleDiff;

			currentPedState = currentPed->m_nPedState;

			if (currentPedState == PED_FALL || currentPedState == PED_GETUP || currentPedState == PED_DIE || currentPedState == PED_DEAD) {
				if (distance < 2.0f && angleDiff < DEGTORAD(65.0f)) {
					if (currentPedState == PED_DEAD) {
						foundDead = 1;
						if (!deadPed)
							deadPed = (CPed*)currentPed;
					} else if (currentPed->IsPedHeadAbovePos(-0.6f)) {
						foundOnTheFloor = 1;
						if (!pedOnTheFloor)
							pedOnTheFloor = (CPed*)currentPed;
					}
				}
			} else if ((distance >= 0.8f || angleDiff >= DEGTORAD(75.0f))
						&& (distance >= 1.3f || angleDiff >= DEGTORAD(55.0f))
						&& (distance >= 1.7f || angleDiff >= DEGTORAD(35.0f))
						&& (distance >= 2.0f || angleDiff >= DEGTORAD(30.0f))) {

				if (angleDiff < DEGTORAD(75.0f)) {
					foundBelow = 1;
					if (!pedBelow)
						pedBelow = (CPed*)currentPed;
				}
			} else {
				foundBelow = 1;
				pedBelow = (CPed*)currentPed;
				break;
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
		stateToReturn = PED_BELOW_PLAYER;
	} else {
		currentPed = nil;
		stateToReturn = NO_PED;
	}

	if (pedOnGround)
		* pedOnGround = (CPed*)currentPed;

	return stateToReturn;
}

bool
CPed::IsPlayer(void)
{
	return m_nPedType == PEDTYPE_PLAYER1 || m_nPedType== PEDTYPE_PLAYER2 ||
		m_nPedType == PEDTYPE_PLAYER3 || m_nPedType == PEDTYPE_PLAYER4;
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
CPed::CanSetPedState(void)
{
	return m_nPedState != PED_DIE && m_nPedState != PED_ARRESTED &&
		m_nPedState != PED_ENTER_CAR && m_nPedState != PED_CARJACK && m_nPedState != PED_DRAG_FROM_CAR && m_nPedState != PED_STEAL_CAR;
}


void
CPed::AddWeaponModel(int id)
{
	RpAtomic *atm;

	if (id != -1) {
		atm = (RpAtomic*)CModelInfo::GetModelInfo(id)->CreateInstance();
		RwFrameDestroy(RpAtomicGetFrame(atm));
		RpAtomicSetFrame(atm, GetNodeFrame(PED_HANDR));
		RpClumpAddAtomic((RpClump*)m_rwObject, atm);
		m_wepModelID = id;
	}
}

void
CPed::AimGun(void)
{
	RwV3d pos;
	CVector vector;

	if (m_pSeekTarget) {
		if (m_pSeekTarget->m_status == STATUS_PHYSICS) {
			m_pSeekTarget->m_pedIK.GetComponentPosition(&pos, PED_TORSO);
			vector.x = pos.x;
			vector.y = pos.y;
			vector.z = pos.z;
		} else {
			vector = *(m_pSeekTarget->GetPosition());
		}
		CPed::Say(SOUND_PED_ATTACK);

		bCanPointGunAtTarget = m_pedIK.PointGunAtPosition(&vector);
		if (m_pLookTarget != m_pSeekTarget) {
			CPed::SetLookFlag(m_pSeekTarget, 1);
		}

	} else {
		if (CPed::IsPlayer()) {
			bCanPointGunAtTarget = m_pedIK.PointGunInDirection(m_fLookDirection, ((CPlayerPed*)this)->m_fFPSMoveHeading);
		} else {
			bCanPointGunAtTarget = m_pedIK.PointGunInDirection(m_fLookDirection, 0.0f);
		}
	}
}

void
CPed::ApplyHeadShot(eWeaponType weaponType, CVector pos, bool evenOnPlayer)
{
	CVector pos2 = CVector(
		pos.x,
		pos.y,
		pos.z + 0.1f
	);

	if (!CPed::IsPlayer() || evenOnPlayer) {
		++CStats::HeadShots;

		// BUG: This condition will always return true.
		if (m_nPedState != PED_PASSENGER || m_nPedState != PED_TAXI_PASSENGER) {
			CPed::SetDie(ANIM_KO_SHOT_FRONT1, 4.0f, 0.0f);
		}

		m_ped_flagC20 = true;
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

void
CPed::RemoveBodyPart(PedNode nodeId, int8 unk)
{
	RwFrame *frame;
	RwV3d pos;

	frame = GetNodeFrame(nodeId);
	if (frame) {
		if (CGame::nastyGame) {
			if (nodeId != PED_HEAD)
				CPed::SpawnFlyingComponent(nodeId, unk);

			RecurseFrameChildrenVisibilityCB(frame, 0);
			pos.x = 0.0f;
			pos.y = 0.0f;
			pos.z = 0.0f;

			for (frame = RwFrameGetParent(frame); frame; frame = RwFrameGetParent(frame))
				RwV3dTransformPoints(&pos, &pos, 1, RwFrameGetMatrix(frame));

			if (CEntity::GetIsOnScreen()) {
				CParticle::AddParticle(PARTICLE_TEST, pos,
					CVector(0.0f, 0.0f, 0.0f),
					nil, 0.2f, 0, 0, 0, 0);

				for (int i = 0; i < 16; i++) {
					CParticle::AddParticle(PARTICLE_BLOOD_SMALL,
						pos,
						CVector(0.0f, 0.0f, 0.03f),
						nil, 0.0f, 0, 0, 0, 0);
				}
			}
			m_ped_flagC20 = true;
			m_bodyPartBleeding = nodeId;
		}
	} else {
		printf("Trying to remove ped component");
	}
}

RwObject*
CPed::SetPedAtomicVisibilityCB(RwObject *object, void *data)
{
	if (data == 0)
		RpAtomicSetFlags(object, 0);
	return object;
}

RwFrame*
CPed::RecurseFrameChildrenVisibilityCB(RwFrame *frame, void *data)
{
	RwFrameForAllObjects(frame, SetPedAtomicVisibilityCB, data);
	RwFrameForAllChildren(frame, RecurseFrameChildrenVisibilityCB, 0);
	return frame;
}

void
CPed::SetLookFlag(CPed *target, bool unknown)
{
	if (m_lookTimer < CTimer::GetTimeInMilliseconds()) {
		bIsLooking = true;
		bIsRestoringLook = false;
		m_pLookTarget = target;
		m_pLookTarget->RegisterReference((CEntity**)&m_pLookTarget);
		m_fLookDirection = 999999.0f;
		m_lookTimer = 0;
		m_ped_flagA20 = unknown;
		if (m_nPedState != PED_DRIVING) {
			m_pedIK.m_flags &= ~CPedIK::FLAG_2;
		}
	}
}

void
CPed::SetLookFlag(float direction, bool unknown)
{
	if (m_lookTimer < CTimer::GetTimeInMilliseconds()) {
		bIsLooking = true;
		bIsRestoringLook = false;
		m_pLookTarget = nil;
		m_fLookDirection = direction;
		m_lookTimer = 0;
		m_ped_flagA20 = unknown;
		if (m_nPedState != PED_DRIVING) {
			m_pedIK.m_flags &= ~CPedIK::FLAG_2;
		}
	}
}

void
CPed::SetLookTimer(int time)
{
	if (CTimer::GetTimeInMilliseconds() > m_lookTimer) {
		m_lookTimer = CTimer::GetTimeInMilliseconds() + time;
	}
}

bool
CPed::OurPedCanSeeThisOne(CEntity *target)
{
	CColPoint colpoint;
	CEntity *ent;

	CVector2D dist = CVector2D(target->GetPosition()) - CVector2D(this->GetPosition());

	// Check if target is behind ped
	if (DotProduct2D(dist, CVector2D(this->GetForward())) < 0.0f)
		return false;

	// Check if target is too far away
	if (dist.Magnitude() >= 40.0f)
		return false;

	// Check line of sight from head
	CVector headPos = this->GetPosition();
	headPos.z += 1.0f;
	return !CWorld::ProcessLineOfSight(headPos, target->GetPosition(), colpoint, ent, true, false, false, false, false, false);
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

			if (nearestPed && nearestPed->m_nPedState != PED_DEAD && nearestPed != m_pSeekTarget && nearestPed != m_field_16C) {

				// Check if this ped wants to avoid the nearest one
				if (CPedType::GetAvoid(this->m_nPedType) & CPedType::GetFlag(nearestPed->m_nPedType)) {

					// Further codes checks whether the distance between us and ped will be equal or below 1.0, if we walk up to him by 1.25 meters.
					// If so, we want to avoid it, so we turn our body 45 degree and look to somewhere else.

					// Game converts from radians to degress and back again here, doesn't make much sense
					CVector2D forward(-sin(m_fRotationCur), cos(m_fRotationCur));
					forward.Normalise();	// this is kinda pointless

					// Move forward 1.25 meters
					CVector2D testPosition = CVector2D(GetPosition()) + forward*1.25f;

					// Get distance to ped we want to avoid
					CVector2D distToPed = CVector2D(nearestPed->GetPosition()) - testPosition;

					if (distToPed.Magnitude() <= 1.0f && CPed::OurPedCanSeeThisOne((CEntity*)nearestPed)) {
						m_nPedStateTimer = CTimer::GetTimeInMilliseconds()
							+ 500 + (m_randomSeed + 3 * CTimer::GetFrameCounter())
							% 1000 / 5;

						m_fRotationDest += DEGTORAD(45.0f);
						if (!bIsLooking) {
							CPed::SetLookFlag(nearestPed, 0);
							CPed::SetLookTimer(CGeneral::GetRandomNumberInRange(0, 300) + 500);
						}
					}
				}
			}
		}
	}
}

void
CPed::ClearAimFlag(void)
{
	if (bIsAimingGun) {
		bIsAimingGun = false;
		bIsRestoringGun = true;
		m_pedIK.m_flags &= ~CPedIK:: FLAG_4;
	}

	if (CPed::IsPlayer())
		((CPlayerPed*)this)->m_fFPSMoveHeading = 0.0;
}

void
CPed::ClearLookFlag(void) {
	if (bIsLooking) {
		bIsLooking = false;
		bIsRestoringLook = true;
		m_ped_flagI1 = false;

		m_pedIK.m_flags &= ~CPedIK::FLAG_2;
		if (CPed::IsPlayer())
			m_lookTimer = CTimer::GetTimeInMilliseconds() + 2000;
		else
			m_lookTimer = CTimer::GetTimeInMilliseconds() + 4000;

		if (m_nPedState == PED_LOOK_HEADING || m_nPedState == PED_LOOK_ENTITY) {
			CPed::RestorePreviousState();
			CPed::ClearLookFlag();
		}
	}
}

bool
CPed::IsPedHeadAbovePos(float zOffset)
{
	RwMatrix mat;
	
	CPedIK::GetWorldMatrix(GetNodeFrame(PED_HEAD), &mat);
	return zOffset + GetPosition().z <= mat.pos.z;
}

void
CPed::FinishedAttackCB(CAnimBlendAssociation *attackAssoc, void *arg)
{
	CWeaponInfo *currentWeapon;
	CAnimBlendAssociation *newAnim;
	CPed *ped = (CPed*)arg;

	if (attackAssoc) {
		switch (attackAssoc->animId) {
			case ANIM_WEAPON_START_THROW:
				if ((!ped->IsPlayer() || ((CPlayerPed*)ped)->field_1380) && ped->IsPlayer())
				{
					attackAssoc->blendDelta = -1000.0f;
					newAnim = CAnimManager::AddAnimation((RpClump*)ped->m_rwObject, ASSOCGRP_STD, ANIM_WEAPON_THROWU);
				} else {
					attackAssoc->blendDelta = -1000.0f;
					newAnim = CAnimManager::AddAnimation((RpClump*)ped->m_rwObject, ASSOCGRP_STD, ANIM_WEAPON_THROW);
				}

				newAnim->SetFinishCallback(FinishedAttackCB, ped);
				return;

			case ANIM_FIGHT_PPUNCH:
				attackAssoc->blendDelta = -8.0f;
				attackAssoc->flags |= ASSOC_DELETEFADEDOUT;
				ped->ClearAttack();
				return;

			case ANIM_WEAPON_THROW:
			case ANIM_WEAPON_THROWU:
				if (ped->GetWeapon()->m_nAmmoTotal > 0) {
					currentWeapon = CWeaponInfo::GetWeaponInfo(ped->GetWeapon()->m_eWeaponType);
					ped->AddWeaponModel(currentWeapon->m_nModelId);
				}
				break;
			default:
				break;
		}
	}
	
	if (!ped->m_ped_flagA4)
		ped->ClearAttack();
}

void
CPed::Attack(void)
{
	CAnimBlendAssociation *weaponAnimAssoc;
	int32 weaponAnim;
	float animStart;
	RwFrame *frame;
	eWeaponType ourWeaponType;
	float weaponAnimTime;
	eWeaponFire ourWeaponFire;
	float animEnd;
	CWeaponInfo *ourWeapon;
	bool lastReloadWasInFuture;
	AnimationId reloadAnim;
	CAnimBlendAssociation *reloadAnimAssoc;
	float delayBetweenAnimAndFire;
	CVector firePos;

	ourWeaponType = GetWeapon()->m_eWeaponType;
	ourWeapon = CWeaponInfo::GetWeaponInfo(ourWeaponType);
	ourWeaponFire = ourWeapon->m_eWeaponFire;
	weaponAnimAssoc = RpAnimBlendClumpGetAssociation((RpClump*)m_rwObject, ourWeapon->m_AnimToPlay);
	lastReloadWasInFuture = m_ped_flagA4;
	reloadAnimAssoc = nil;
	reloadAnim = NUM_ANIMS;
	delayBetweenAnimAndFire = ourWeapon->m_fAnimFrameFire;
	weaponAnim = ourWeapon->m_AnimToPlay;

	if (weaponAnim == ANIM_WEAPON_HGUN_BODY)
		reloadAnim = ANIM_HGUN_RELOAD;
	else if (weaponAnim == ANIM_WEAPON_AK_BODY)
		reloadAnim = ANIM_AK_RELOAD;

	if (reloadAnim != NUM_ANIMS)
		reloadAnimAssoc = RpAnimBlendClumpGetAssociation((RpClump*)m_rwObject, reloadAnim);

	if (m_ped_flagE10)
		return;

	if (reloadAnimAssoc) {
		if (!CPed::IsPlayer() || ((CPlayerPed*)this)->field_1380)
			ClearAttack();

		return;
	}

	// BUG: We currently don't know any situation this cond. could be true.
	if (CTimer::GetTimeInMilliseconds() < m_lastHitTime)
		lastReloadWasInFuture = true;

	if (!weaponAnimAssoc) {
		weaponAnimAssoc = RpAnimBlendClumpGetAssociation((RpClump*)m_rwObject, ourWeapon->m_Anim2ToPlay);
		delayBetweenAnimAndFire = ourWeapon->m_fAnim2FrameFire;

		// Long throw granade, molotov
		if (!weaponAnimAssoc && ourWeapon->m_bThrow) {
			weaponAnimAssoc = RpAnimBlendClumpGetAssociation((RpClump*) m_rwObject, ANIM_WEAPON_THROWU);
			delayBetweenAnimAndFire = 0.2f;
		}
	}
	if (weaponAnimAssoc) {
		animStart = ourWeapon->m_fAnimLoopStart;
		weaponAnimTime = weaponAnimAssoc->currentTime;
		if (weaponAnimTime > animStart && weaponAnimTime - weaponAnimAssoc->timeStep <= animStart) {
			if (ourWeapon->m_bCanAimWithArm)
				m_pedIK.m_flags |= CPedIK::FLAG_4;
			else
				m_pedIK.m_flags &= ~CPedIK::FLAG_4;
		}
		if (weaponAnimTime <= delayBetweenAnimAndFire || weaponAnimTime - weaponAnimAssoc->timeStep > delayBetweenAnimAndFire || !weaponAnimAssoc->IsRunning()) {
			if (weaponAnimAssoc->speed < 1.0f)
				weaponAnimAssoc->speed = 1.0;

		} else {
			firePos = ourWeapon->m_vecFireOffset;
			if (ourWeaponType == WEAPONTYPE_BASEBALLBAT) {
				if (weaponAnimAssoc->animId == ourWeapon->m_Anim2ToPlay)
					firePos.z = 0.7f * ourWeapon->m_fRadius - 1.0f;

				firePos = GetMatrix() * firePos;
			} else if (ourWeaponType != WEAPONTYPE_UNARMED) {
				if (weaponAnimAssoc->animId == ANIM_KICK_FLOOR)
					frame = GetNodeFrame(PED_FOOTR);
				else
					frame = GetNodeFrame(PED_HANDR);

				for (; frame; frame = RwFrameGetParent(frame))
					RwV3dTransformPoints((RwV3d*)firePos, (RwV3d*)firePos, 1, RwFrameGetMatrix(frame));
			} else {
				firePos = GetMatrix() * firePos;
			}
			
			GetWeapon()->Fire(this, &firePos);

			if (ourWeaponType == WEAPONTYPE_MOLOTOV || ourWeaponType == WEAPONTYPE_GRENADE) {
				RemoveWeaponModel(ourWeapon->m_nModelId);
			}
			if (!GetWeapon()->m_nAmmoTotal && ourWeaponFire != WEAPON_FIRE_MELEE && FindPlayerPed() != this) {
				SelectGunIfArmed();
			}

			if (GetWeapon()->m_eWeaponState != WEAPONSTATE_MELEE_MADECONTACT) {
				// If reloading just began, start the animation
				if (GetWeapon()->m_eWeaponState == WEAPONSTATE_RELOADING && reloadAnim != NUM_ANIMS && !reloadAnimAssoc) {
					CAnimManager::BlendAnimation((RpClump*) m_rwObject, ASSOCGRP_STD, reloadAnim, 8.0f);
					ClearLookFlag();
					ClearAimFlag();
					m_ped_flagA4 = false;
					bIsPointingGunAt = false;
					m_lastHitTime = CTimer::GetTimeInMilliseconds();
					return;
				}
			} else {
				if (weaponAnimAssoc->animId == ANIM_WEAPON_BAT_V || weaponAnimAssoc->animId == ANIM_WEAPON_BAT_H) {
					DMAudio.PlayOneShot(uAudioEntityId, SOUND_WEAPON_BAT_ATTACK, 1.0f);
				} else if (weaponAnimAssoc->animId == ANIM_FIGHT_PPUNCH) {
					DMAudio.PlayOneShot(uAudioEntityId, SOUND_WEAPON_PUNCH_ATTACK, 0.0f);
				}

				weaponAnimAssoc->speed = 0.5;

				// BUG: We currently don't know any situation this cond. could be true.
				if (m_ped_flagA4 || CTimer::GetTimeInMilliseconds() < m_lastHitTime) {
					weaponAnimAssoc->callbackType = 0;
				}
			}

			lastReloadWasInFuture = false;
		}

		if (ourWeaponType == WEAPONTYPE_SHOTGUN) {
			weaponAnimTime = weaponAnimAssoc->currentTime;
			firePos = ourWeapon->m_vecFireOffset;

			if (weaponAnimTime > 1.0f && weaponAnimTime - weaponAnimAssoc->timeStep <= 1.0f && weaponAnimAssoc->IsRunning()) {
				for (frame = GetNodeFrame(PED_HANDR); frame; frame = RwFrameGetParent(frame))
					RwV3dTransformPoints((RwV3d*)firePos, (RwV3d*)firePos, 1, RwFrameGetMatrix(frame));

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
		animEnd = ourWeapon->m_fAnimLoopEnd;
		if (ourWeaponFire == WEAPON_FIRE_MELEE && weaponAnimAssoc->animId == ourWeapon->m_Anim2ToPlay)
			animEnd = 0.56f;

		weaponAnimTime = weaponAnimAssoc->currentTime;

		// End of the attack
		if (weaponAnimTime > animEnd || !weaponAnimAssoc->IsRunning() && ourWeaponFire != WEAPON_FIRE_PROJECTILE) {

			if (weaponAnimTime - 2.0f * weaponAnimAssoc->timeStep <= animEnd
				&& (m_ped_flagA4 || CTimer::GetTimeInMilliseconds() < m_lastHitTime)
				&& GetWeapon()->m_eWeaponState != WEAPONSTATE_RELOADING) {

				weaponAnim = weaponAnimAssoc->animId;
				if (ourWeaponFire != WEAPON_FIRE_MELEE || CheckForPedsOnGroundToAttack(((CPlayerPed*)this), 0) < PED_ON_THE_FLOOR) {
					if (weaponAnim != ourWeapon->m_Anim2ToPlay || weaponAnim == ANIM_RBLOCK_CSHOOT) {
						weaponAnimAssoc->Start(ourWeapon->m_fAnimLoopStart);
					} else {
						CAnimManager::BlendAnimation((RpClump*) m_rwObject, ASSOCGRP_STD, ourWeapon->m_AnimToPlay, 8.0f);
					}
				} else {
					if (weaponAnim == ourWeapon->m_Anim2ToPlay)
						weaponAnimAssoc->SetCurrentTime(0.1f);
					else
						CAnimManager::BlendAnimation((RpClump*) m_rwObject, ASSOCGRP_STD, ourWeapon->m_Anim2ToPlay, 8.0f);
				}
			} else {
				ClearAimFlag();

				// Echoes of bullets, at the end of the attack. (Bug: doesn't play while reloading)
				if (weaponAnimAssoc->currentTime - weaponAnimAssoc->timeStep <= ourWeapon->m_fAnimLoopEnd) {
					switch (ourWeaponType) {
						case WEAPONTYPE_UZI:
							DMAudio.PlayOneShot(uAudioEntityId, SOUND_WEAPON_UZI_BULLET_ECHO, 0.0f);
							break;
						case WEAPONTYPE_AK47:
							DMAudio.PlayOneShot(uAudioEntityId, SOUND_WEAPON_AK47_BULLET_ECHO, 0.0f);
							break;
						case WEAPONTYPE_M16:
							DMAudio.PlayOneShot(uAudioEntityId, SOUND_WEAPON_M16_BULLET_ECHO, 0.0f);
							break;
						default:
							break;
					}
				}

				// Fun fact: removing this part leds to reloading flamethrower
				if (ourWeaponType == WEAPONTYPE_FLAMETHROWER && weaponAnimAssoc->IsRunning()) {
					weaponAnimAssoc->flags |= ASSOC_DELETEFADEDOUT;
					weaponAnimAssoc->flags &= ~ASSOC_RUNNING;
					weaponAnimAssoc->blendDelta = -4.0f;
				}
			}
		}
		if (weaponAnimAssoc->currentTime > delayBetweenAnimAndFire)
			lastReloadWasInFuture = false;

		m_ped_flagA4 = lastReloadWasInFuture;
		return;
	}

	if (lastReloadWasInFuture) {
		if (ourWeaponFire != WEAPON_FIRE_PROJECTILE || !CPed::IsPlayer() || ((CPlayerPed*)this)->field_1380) {
			if (!CGame::nastyGame || ourWeaponFire != WEAPON_FIRE_MELEE || CheckForPedsOnGroundToAttack(((CPlayerPed*)this), 0) < PED_ON_THE_FLOOR) {
				weaponAnimAssoc = CAnimManager::BlendAnimation((RpClump*)m_rwObject, ASSOCGRP_STD, ourWeapon->m_AnimToPlay, 8.0f);
			} else {
				weaponAnimAssoc = CAnimManager::BlendAnimation((RpClump*)m_rwObject, ASSOCGRP_STD, ourWeapon->m_Anim2ToPlay, 8.0f);
			}

			weaponAnimAssoc->SetFinishCallback(CPed::FinishedAttackCB, this);
			weaponAnimAssoc->flags |= ASSOC_RUNNING;

			if (weaponAnimAssoc->currentTime == weaponAnimAssoc->hierarchy->totalLength)
				weaponAnimAssoc->SetCurrentTime(0.0f);

			if (CPed::IsPlayer()) {
				((CPlayerPed*)this)->field_1376 = 0.0f;
				((CPlayerPed*)this)->field_1380 = false;
			}
		}
	}
	else
		CPed::FinishedAttackCB(0, this);
}

void
CPed::RemoveWeaponModel(int modelId)
{
	// modelId is not used!! This function just removes the current weapon.
	RwFrameForAllObjects(GetNodeFrame(PED_HANDR),RemoveAllModelCB,0);
	m_wepModelID = -1;
}

void
CPed::SetCurrentWeapon(eWeaponType weaponType)
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

// Only used while deciding which gun ped should switch to, if no ammo left.
bool
CPed::SelectGunIfArmed(void)
{
	eWeaponType weaponType;

	for (int i = 0; i < m_maxWeaponTypeAllowed; i++) {

		if (m_weapons[i].m_nAmmoTotal > 0) {
			weaponType = m_weapons[i].m_eWeaponType;

			if (weaponType >= WEAPONTYPE_COLT45 && weaponType != WEAPONTYPE_M16 && weaponType <= WEAPONTYPE_FLAMETHROWER) {
				SetCurrentWeapon(weaponType);
				return true;
			}
		}
	}
	SetCurrentWeapon(WEAPONTYPE_UNARMED);
	return false;
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
	CAnimBlendAssociation *animAssoc;

	animAssoc = RpAnimBlendClumpGetAssociation((RpClump*) m_rwObject, ANIM_DUCK_DOWN);
	if (!animAssoc)
		animAssoc = RpAnimBlendClumpGetAssociation((RpClump*) m_rwObject, ANIM_DUCK_LOW);

	if (animAssoc) {

		if (m_ped_flagE8) {

			if (m_nPedState == PED_ATTACK || m_nPedState == PED_AIM_GUN) {
				animAssoc = RpAnimBlendClumpGetAssociation((RpClump*) m_rwObject, ANIM_RBLOCK_CSHOOT);
				if (!animAssoc || animAssoc->blendDelta < 0.0f) {
					CAnimManager::BlendAnimation((RpClump*) m_rwObject, ASSOCGRP_STD, ANIM_RBLOCK_CSHOOT, 4.0f);
				}
			}
		}
	} else
		m_ped_flagE10 = false;
}

void
CPed::ClearPointGunAt(void)
{
	CAnimBlendAssociation *animAssoc;
	CWeaponInfo *weaponInfo;

	ClearLookFlag();
	ClearAimFlag();
	bIsPointingGunAt = false;
	if (m_nPedState == PED_AIM_GUN) {
		RestorePreviousState();
		weaponInfo = CWeaponInfo::GetWeaponInfo(GetWeapon()->m_eWeaponType);
		animAssoc = RpAnimBlendClumpGetAssociation((RpClump*) m_rwObject, weaponInfo->m_AnimToPlay);
		if (!animAssoc || animAssoc->blendDelta < 0.0f) {
			animAssoc = RpAnimBlendClumpGetAssociation((RpClump*) m_rwObject, weaponInfo->m_Anim2ToPlay);
		}
		if (animAssoc) {
			animAssoc->flags |= ASSOC_DELETEFADEDOUT;
			animAssoc->blendDelta = -4.0;
		}
	}
}

void
CPed::BeingDraggedFromCar(void)
{
	CAnimBlendAssociation *animAssoc;
	AnimationId enterAnim;
	bool dontRunAnim = false;
	PedLineUpPhase lineUpType;

	if (!m_pVehicleAnim) {
		CAnimManager::BlendAnimation((RpClump*) m_rwObject, m_animGroup, ANIM_IDLE_STANCE, 100.0f);
		animAssoc = RpAnimBlendClumpGetAssociation((RpClump*) m_rwObject, ANIM_CAR_SIT);
		if (!animAssoc) {
			animAssoc = RpAnimBlendClumpGetAssociation((RpClump*) m_rwObject, ANIM_CAR_LSIT);
			if (!animAssoc) {
				animAssoc = RpAnimBlendClumpGetAssociation((RpClump*) m_rwObject, ANIM_CAR_SITP);
				if (!animAssoc)
					animAssoc = RpAnimBlendClumpGetAssociation((RpClump*) m_rwObject, ANIM_CAR_SITPLO);
			}
		}
		if (animAssoc)
			animAssoc->blendDelta = -1000.0f;

		if (m_vehEnterType == VEHICLE_ENTER_FRONT_LEFT || m_vehEnterType == VEHICLE_ENTER_REAR_LEFT) {
			if (m_ped_flagF10) {
				enterAnim = ANIM_CAR_QJACKED;
			} else if (m_pMyVehicle->bIsLow) {
				enterAnim = ANIM_CAR_LJACKED_LHS;
			} else {
				enterAnim = ANIM_CAR_JACKED_LHS;
			}
		} else if (m_vehEnterType == VEHICLE_ENTER_FRONT_RIGHT || m_vehEnterType == VEHICLE_ENTER_REAR_RIGHT) {
			if (m_pMyVehicle->bIsLow)
				enterAnim = ANIM_CAR_LJACKED_RHS;
			else
				enterAnim = ANIM_CAR_JACKED_RHS;
		} else
			dontRunAnim = true;


		if (!dontRunAnim)
			m_pVehicleAnim = CAnimManager::AddAnimation((RpClump*) m_rwObject, ASSOCGRP_STD, enterAnim);

		m_pVehicleAnim->SetFinishCallback(PedSetDraggedOutCarCB, this);
		lineUpType = LINE_UP_TO_CAR_START;
	} else if (m_pVehicleAnim->currentTime <= 1.4f) {
		m_vecMoveSpeed = CVector(0.0f, 0.0f, 0.0f);
		lineUpType = LINE_UP_TO_CAR_START;
	} else {
		lineUpType = LINE_UP_TO_CAR_2;
	}
	
	LineUpPedWithCar(lineUpType);
}

void
CPed::RestartNonPartialAnims(void)
{
	CAnimBlendAssociation* assoc;

	for (assoc = RpAnimBlendClumpGetFirstAssociation((RpClump*)m_rwObject); !assoc; assoc = RpAnimBlendGetNextAssociation(assoc)) {
		if (!assoc->IsPartial())
			assoc->flags |= ASSOC_RUNNING;
	}
}

void
CPed::PedSetDraggedOutCarCB(CAnimBlendAssociation *dragAssoc, void *arg)
{
	CAnimBlendAssociation *quickJackedAssoc;
	CVehicle *vehicle; 
	CPed *ped = (CPed*)arg;
	eWeaponType weaponType = ped->GetWeapon()->m_eWeaponType;

	quickJackedAssoc = RpAnimBlendClumpGetAssociation((RpClump*) ped->m_rwObject, ANIM_CAR_QJACKED);
	if (ped->m_nPedState != PED_ARRESTED) {
		ped->m_nLastPedState = PED_NONE;
		if (dragAssoc)
			dragAssoc->blendDelta = -1000.0;
	}
	ped->RestartNonPartialAnims();
	ped->m_pVehicleAnim = nil;
	ped->m_pSeekTarget = nil;
	vehicle = ped->m_pMyVehicle;

	switch (ped->m_vehEnterType) {
		case VEHICLE_ENTER_FRONT_RIGHT:
			vehicle->m_nGettingOutFlags &= ~GETTING_IN_OUT_FR;
			break;
		case VEHICLE_ENTER_REAR_RIGHT:
			vehicle->m_nGettingOutFlags &= ~GETTING_IN_OUT_RR;
			break;
		case VEHICLE_ENTER_FRONT_LEFT:
			vehicle->m_nGettingOutFlags &= ~GETTING_IN_OUT_FL;
			break;
		case VEHICLE_ENTER_REAR_LEFT:
			vehicle->m_nGettingOutFlags &= ~GETTING_IN_OUT_RL;
			break;
	}

	if (vehicle->pDriver == ped) {
		vehicle->RemoveDriver();
		if (vehicle->m_nDoorLock == CARLOCK_COP_CAR)
			vehicle->m_nDoorLock = CARLOCK_UNLOCKED;

		if (ped->m_nPedType == PEDTYPE_COP && vehicle->IsLawEnforcementVehicle())
			vehicle->ChangeLawEnforcerState(false);
	} else {
		for (int i = 0; i < vehicle->m_nNumMaxPassengers; i++) {
			if (vehicle->pPassengers[i] == ped) {
				vehicle->pPassengers[i] = nil;
				vehicle->m_nNumPassengers--;
			}
		}
	}

	ped->bInVehicle = false;
	if (ped->IsPlayer())
		AudioManager.PlayerJustLeftCar();

	if (quickJackedAssoc) {
		dragAssoc->SetDeleteCallback(PedSetQuickDraggedOutCarPositionCB, ped);
	} else {
		dragAssoc->SetDeleteCallback(PedSetDraggedOutCarPositionCB, ped);
		if (ped->CanSetPedState())
			CAnimManager::BlendAnimation((RpClump*) ped->m_rwObject, ASSOCGRP_STD, ANIM_GETUP1, 1000.0f);
	}

	// Only uzi can be used on cars, so previous weapon was stored
	if (ped->IsPlayer() && weaponType == WEAPONTYPE_UZI) {
		if (ped->m_storedWeapon != NO_STORED_WEAPON) {
			ped->SetCurrentWeapon(ped->m_storedWeapon);
			ped->m_storedWeapon = NO_STORED_WEAPON;
		}
	} else {
		ped->AddWeaponModel(CWeaponInfo::GetWeaponInfo(weaponType)->m_nModelId);
	}
	ped->m_nStoredActionState = 0;
	ped->m_ped_flagI4 = false;
}

void
CPed::GetLocalPositionToOpenCarDoor(CVector *output, CVehicle *veh, uint32 enterType, float seatPosMult)
{
	CVehicleModelInfo *vehModel; 
	CVector vehDoorPos;
	CVector vehDoorOffset;
	float seatOffset;

	vehModel = (CVehicleModelInfo*) CModelInfo::GetModelInfo(veh->m_modelIndex);
	if (veh->bIsVan && (enterType == VEHICLE_ENTER_REAR_LEFT || enterType == VEHICLE_ENTER_REAR_RIGHT)) {
		seatOffset = 0.0f;
		vehDoorOffset = offsetToOpenVanDoor;
	} else {
		seatOffset = veh->m_handling->fSeatOffsetDistance * seatPosMult;
		if (veh->bIsLow) {
			vehDoorOffset = offsetToOpenLowCarDoor;
		} else {
			vehDoorOffset = offsetToOpenRegularCarDoor;
		}
	}

	switch (enterType) {
		case VEHICLE_ENTER_FRONT_RIGHT:
			if (vehModel->m_vehicleType == VEHICLE_TYPE_BOAT)
				vehDoorPos = vehModel->m_positions[VEHICLE_DUMMY_BOAT_RUDDER];
			else
				vehDoorPos = vehModel->m_positions[VEHICLE_DUMMY_FRONT_SEATS];

			vehDoorPos.x += seatOffset;
			vehDoorOffset.x = -vehDoorOffset.x;
			break;

		case VEHICLE_ENTER_REAR_RIGHT:
			vehDoorPos = vehModel->m_positions[VEHICLE_DUMMY_REAR_SEATS];
			vehDoorPos.x += seatOffset;
			vehDoorOffset.x = -vehDoorOffset.x;
			break;

		case VEHICLE_ENTER_FRONT_LEFT:
			if (vehModel->m_vehicleType == VEHICLE_TYPE_BOAT)
				vehDoorPos = vehModel->m_positions[VEHICLE_DUMMY_BOAT_RUDDER];
			else
				vehDoorPos = vehModel->m_positions[VEHICLE_DUMMY_FRONT_SEATS];

			vehDoorPos.x = -(vehDoorPos.x + seatOffset);
			break;

		case VEHICLE_ENTER_REAR_LEFT:
			vehDoorPos = vehModel->m_positions[VEHICLE_DUMMY_REAR_SEATS];
			vehDoorPos.x = -(vehDoorPos.x + seatOffset);
			break;

		default:
			if (vehModel->m_vehicleType == VEHICLE_TYPE_BOAT)
				vehDoorPos = vehModel->m_positions[VEHICLE_DUMMY_BOAT_RUDDER];
			else
				vehDoorPos = vehModel->m_positions[VEHICLE_DUMMY_FRONT_SEATS];

			vehDoorOffset = CVector(0.0f, 0.0f, 0.0f);
	}
	*output = vehDoorPos - vehDoorOffset;
}

// This function was mostly duplicate of GetLocalPositionToOpenCarDoor, so I've used it.
void
CPed::GetPositionToOpenCarDoor(CVector *output, CVehicle *veh, uint32 enterType)
{
	CVector localPos;
	CVector vehDoorPos;

	GetLocalPositionToOpenCarDoor(&localPos, veh, enterType, 1.0f);
	vehDoorPos = Multiply3x3(veh->GetMatrix(), localPos) + veh->GetPosition();

/*
	// Not used.
	CVector localVehDoorOffset;

	if (veh->bIsVan && (enterType == VEHICLE_ENTER_REAR_LEFT || enterType == VEHICLE_ENTER_REAR_RIGHT)) {
		localVehDoorOffset = offsetToOpenVanDoor;
	} else {
		if (veh->bIsLow) {
			localVehDoorOffset = offsetToOpenLowCarDoor;
		} else {
			localVehDoorOffset = offsetToOpenRegularCarDoor;
		}
	}

	vehDoorPosWithoutOffset = Multiply3x3(veh->GetMatrix(), localPos + localVehDoorOffset) + veh->GetPosition();
*/
	*output = vehDoorPos;
}

void
CPed::GetPositionToOpenCarDoor(CVector *output, CVehicle *veh, uint32 enterType, float offset)
{	
	CVector doorPos;
	CMatrix vehMat(veh->GetMatrix());

	GetLocalPositionToOpenCarDoor(output, veh, enterType, offset);
	doorPos = Multiply3x3(vehMat, *output);

	*output = *veh->GetPosition() + doorPos;
}

void
CPed::LineUpPedWithCar(PedLineUpPhase phase)
{
	bool vehIsUpsideDown = false;
	int vehAnim;
	float seatPosMult = 0.0f;
	float currentZ;
	float adjustedTimeStep;

	if (CReplay::IsPlayingBack())
		return;

	if (!m_ped_flagC8 && phase != LINE_UP_TO_CAR_2) {
		if (RpAnimBlendClumpGetAssociation((RpClump*)m_rwObject, ANIM_CAR_SIT)) {
			SetPedPositionInCar();
			return;
		}
		if (RpAnimBlendClumpGetAssociation((RpClump*)m_rwObject, ANIM_CAR_LSIT)) {
			SetPedPositionInCar();
			return;
		}
		if (RpAnimBlendClumpGetAssociation((RpClump*)m_rwObject, ANIM_CAR_SITP)) {
			SetPedPositionInCar();
			return;
		}
		if (RpAnimBlendClumpGetAssociation((RpClump*)m_rwObject, ANIM_CAR_SITPLO)) {
			SetPedPositionInCar();
			return;
		}
		m_ped_flagC8 = 1;
	}
	if (phase == LINE_UP_TO_CAR_START) {
		m_vecMoveSpeed = CVector(0.0f, 0.0f, 0.0f);
	}
	CVehicle *veh = m_pMyVehicle;

	// Not quite right, IsUpsideDown func. checks for <= -0.9f.
	// Since that function is also used in this file, doesn't this variable indicate upsidedownness?!
	if (veh->GetUp().z <= -0.8f)
		vehIsUpsideDown = true;

	if (m_vehEnterType == VEHICLE_ENTER_FRONT_RIGHT || m_vehEnterType == VEHICLE_ENTER_REAR_RIGHT) {
		if (vehIsUpsideDown) {
			m_fRotationDest = -PI + atan2(-veh->GetForward().x, veh->GetForward().y);
		} else if (veh->bIsBus) {
			m_fRotationDest = 0.5 * PI + atan2(-veh->GetForward().x, veh->GetForward().y);
		} else {
			m_fRotationDest = atan2(-veh->GetForward().x, veh->GetForward().y);
		}
	} else if (m_vehEnterType == VEHICLE_ENTER_FRONT_LEFT || m_vehEnterType == VEHICLE_ENTER_REAR_LEFT) {
		if (vehIsUpsideDown) {
			m_fRotationDest = atan2(-veh->GetForward().x, veh->GetForward().y);
		} else if (veh->bIsBus) {
			m_fRotationDest = -0.5 * PI + atan2(-veh->GetForward().x, veh->GetForward().y);
		} else {
			m_fRotationDest = atan2(-veh->GetForward().x, veh->GetForward().y);
		}
	}

	if (!bInVehicle)
		seatPosMult = 1.0f;

	if (m_pVehicleAnim) {
		vehAnim = m_pVehicleAnim->animId;

		switch (vehAnim) {
			case ANIM_CAR_JACKED_RHS:
			case ANIM_CAR_LJACKED_RHS:
			case ANIM_CAR_JACKED_LHS:
			case ANIM_CAR_LJACKED_LHS:
			case ANIM_CAR_QJACKED:
			case ANIM_CAR_GETOUT_LHS:
			case ANIM_CAR_GETOUT_LOW_LHS:
			case ANIM_CAR_GETOUT_RHS:
			case ANIM_CAR_GETOUT_LOW_RHS:
			case ANIM_CAR_CRAWLOUT_RHS:
			case ANIM_CAR_CRAWLOUT_RHS2:
			case ANIM_VAN_GETIN_L:
			case ANIM_VAN_GETOUT_L:
			case ANIM_VAN_GETIN:
			case ANIM_VAN_GETOUT:
				seatPosMult = m_pVehicleAnim->currentTime / m_pVehicleAnim->hierarchy->totalLength;
				break;
			case ANIM_CAR_QJACK:
			case ANIM_CAR_GETIN_LHS:
			case ANIM_CAR_GETIN_LOW_LHS:
			case ANIM_CAR_GETIN_RHS:
			case ANIM_CAR_GETIN_LOW_RHS:
			case ANIM_DRIVE_BOAT:
				seatPosMult = m_pVehicleAnim->GetTimeLeft() / m_pVehicleAnim->hierarchy->totalLength;
				break;
			case ANIM_CAR_CLOSEDOOR_LHS:
			case ANIM_CAR_CLOSEDOOR_LOW_LHS:
			case ANIM_CAR_CLOSEDOOR_RHS:
			case ANIM_CAR_CLOSEDOOR_LOW_RHS:
			case ANIM_CAR_SHUFFLE_RHS:
			case ANIM_CAR_LSHUFFLE_RHS:
				seatPosMult = 0.0f;
				break;
			case ANIM_CAR_CLOSE_LHS:
			case ANIM_CAR_CLOSE_RHS:
			case ANIM_COACH_OPEN_L:
			case ANIM_COACH_OPEN_R:
			case ANIM_COACH_IN_L:
			case ANIM_COACH_IN_R:
			case ANIM_COACH_OUT_L:
				seatPosMult = 1.0f;
				break;
			default:
				break;
		}
	}

	CVector neededPos;

	if (phase == LINE_UP_TO_CAR_2) {
		neededPos = *GetPosition();
	} else {
		GetPositionToOpenCarDoor(&neededPos, veh, m_vehEnterType, seatPosMult);
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
			float pedZSpeedOnExit = m_vecMoveSpeed.z - 0.008f * CTimer::GetTimeStep();

			// If we're not in ground at next step, apply animation
			if (neededPos.z + pedZSpeedOnExit > autoZPos.z) {
				m_vecMoveSpeed.z = pedZSpeedOnExit;
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
		currentZ = GetPosition().z;
		if (m_pVehicleAnim && vehAnim != ANIM_VAN_GETIN_L && vehAnim != ANIM_VAN_CLOSE_L && vehAnim != ANIM_VAN_CLOSE && vehAnim != ANIM_VAN_GETIN) {
			neededPos.z = autoZPos.z;
			m_vecMoveSpeed = CVector(0.0f, 0.0f, 0.0f);
		} else if (neededPos.z <= currentZ && m_pVehicleAnim && vehAnim != ANIM_VAN_CLOSE_L && vehAnim != ANIM_VAN_CLOSE) {
			adjustedTimeStep = min(m_pVehicleAnim->timeStep, 0.1f);

			// Smoothly change ped position
			neededPos.z = currentZ - (currentZ - neededPos.z) / (m_pVehicleAnim->GetTimeLeft() / adjustedTimeStep);
		}
	} else {
		// We may need to raise up the ped
		if (phase == LINE_UP_TO_CAR_START) {
			currentZ = GetPosition().z;

			if (neededPos.z > currentZ) {

				if (m_pVehicleAnim &&
					(vehAnim == ANIM_CAR_GETIN_RHS || vehAnim == ANIM_CAR_GETIN_LOW_RHS || vehAnim == ANIM_CAR_GETIN_LHS || vehAnim == ANIM_CAR_GETIN_LOW_LHS
						|| vehAnim == ANIM_CAR_QJACK || vehAnim == ANIM_VAN_GETIN_L || vehAnim == ANIM_VAN_GETIN)) {
					adjustedTimeStep = min(m_pVehicleAnim->timeStep, 0.1f);

					// Smoothly change ped position
					neededPos.z = (neededPos.z - currentZ) / (m_pVehicleAnim->GetTimeLeft() / adjustedTimeStep) + currentZ;
				} else if (m_nPedState == PED_ENTER_CAR || m_nPedState == PED_CARJACK) {
					neededPos.z = max(currentZ, autoZPos.z);
				}
			}
		}
	}

	// I hope
	bool stillGettingInOut = false;
	if (CTimer::GetTimeInMilliseconds() < m_nPedStateTimer)
		stillGettingInOut = veh->m_vehType != VEHICLE_TYPE_BOAT || m_ped_flagG10;

	if (!stillGettingInOut) {
		m_fRotationCur = m_fRotationDest;
	} else {
		float limitedAngle = CGeneral::LimitRadianAngle(m_fRotationDest);
		float timeUntilStateChange = (m_nPedStateTimer - CTimer::GetTimeInMilliseconds())/600.0f;

		m_vecOffsetSeek.z = 0.0;
		if (timeUntilStateChange <= 0.0f) {
			m_vecOffsetSeek.x = 0.0;
			m_vecOffsetSeek.y = 0.0;
		} else {
			neededPos -= timeUntilStateChange * m_vecOffsetSeek;
		}

		if (limitedAngle >= PI + m_fRotationCur) {
			limitedAngle -= 2 * PI;
		} else if (limitedAngle <= m_fRotationCur - PI) {
			limitedAngle += 2 * PI;
		}
		m_fRotationCur -= (m_fRotationCur - limitedAngle) * (1.0f - timeUntilStateChange);
	}

	if (seatPosMult > 0.2f || vehIsUpsideDown) {
		GetPosition() = neededPos;

		GetMatrix().SetRotate(0.0f, 0.0f, m_fRotationCur);

		// It will be all 0 after rotate.
		GetPosition() = neededPos;
	} else {
		CVector output;
		CMatrix vehDoorMat(veh->GetMatrix());

		GetLocalPositionToOpenCarDoor(&output, veh, m_vehEnterType, 0.0f);
		*vehDoorMat.GetPosition() += Multiply3x3(vehDoorMat, output);
		GetMatrix() = vehDoorMat;
	}

}

STARTPATCHES
	InjectHook(0x4CF8F0, &CPed::AddWeaponModel, PATCH_JUMP);
	InjectHook(0x4C6AA0, &CPed::AimGun, PATCH_JUMP);
	InjectHook(0x4EB470, &CPed::ApplyHeadShot, PATCH_JUMP);
	InjectHook(0x4EAEE0, &CPed::RemoveBodyPart, PATCH_JUMP);
	InjectHook(0x4C6460, (void (CPed::*)(CPed*, bool)) &CPed::SetLookFlag, PATCH_JUMP);
	InjectHook(0x4C63E0, (void (CPed::*)(float, bool)) &CPed::SetLookFlag, PATCH_JUMP);
	InjectHook(0x4D12E0, &CPed::SetLookTimer, PATCH_JUMP);
	InjectHook(0x4C5700, &CPed::OurPedCanSeeThisOne, PATCH_JUMP);
	InjectHook(0x4D2BB0, &CPed::Avoid, PATCH_JUMP);
	InjectHook(0x4C6A50, &CPed::ClearAimFlag, PATCH_JUMP);
	InjectHook(0x4C64F0, &CPed::ClearLookFlag, PATCH_JUMP);
	InjectHook(0x4E5BD0, &CPed::IsPedHeadAbovePos, PATCH_JUMP);
	InjectHook(0x4E68A0, &CPed::FinishedAttackCB, PATCH_JUMP);
	InjectHook(0x4E5BD0, &CheckForPedsOnGroundToAttack, PATCH_JUMP);
	InjectHook(0x4E6BA0, &CPed::Attack, PATCH_JUMP);
	InjectHook(0x4CF980, &CPed::RemoveWeaponModel, PATCH_JUMP);
	InjectHook(0x4CFA60, &CPed::SetCurrentWeapon, PATCH_JUMP);
	InjectHook(0x4DD920, &CPed::SelectGunIfArmed, PATCH_JUMP);
	InjectHook(0x4E4A10, &CPed::Duck, PATCH_JUMP);
	InjectHook(0x4E4A30, &CPed::ClearDuck, PATCH_JUMP);
	InjectHook(0x4E6180, &CPed::ClearPointGunAt, PATCH_JUMP);
	InjectHook(0x4E07D0, &CPed::BeingDraggedFromCar, PATCH_JUMP);
	InjectHook(0x4CF000, &CPed::PedSetDraggedOutCarCB, PATCH_JUMP);
	InjectHook(0x4C5D80, &CPed::RestartNonPartialAnims, PATCH_JUMP);
	InjectHook(0x4E4730, &CPed::GetLocalPositionToOpenCarDoor, PATCH_JUMP);
	InjectHook(0x4E4660, (void (*)(CVector*, CVehicle*, uint32, float)) CPed::GetPositionToOpenCarDoor, PATCH_JUMP);
	InjectHook(0x4E1A30, (void (*)(CVector*, CVehicle*, uint32)) CPed::GetPositionToOpenCarDoor, PATCH_JUMP);
	InjectHook(0x4DF940, &CPed::LineUpPedWithCar, PATCH_JUMP);
ENDPATCHES
