#include "common.h"

#include "main.h"
#include "Pools.h"
#include "Particle.h"
#include "Stats.h"
#include "World.h"
#include "RpAnimBlend.h"
#include "Bones.h"
#include "Ped.h"
#include "Wanted.h"
#include "PlayerPed.h"
#include "PedType.h"
#include "AnimBlendClumpData.h"
#include "AnimBlendAssociation.h"
#include "Fire.h"
#include "DMAudio.h"
#include "General.h"
#include "SurfaceTable.h"
#include "VisibilityPlugins.h"
#include "AudioManager.h"
#include "HandlingMgr.h"
#include "Replay.h"
#include "Camera.h"
#include "Radar.h"
#include "PedPlacement.h"
#include "Shadows.h"
#include "Weather.h"
#include "ZoneCull.h"
#include "Population.h"
#include "Renderer.h"
#include "Lights.h"
#include "PointLights.h"
#include "Pad.h"
#include "Phones.h"
#include "Darkel.h"
#include "PathFind.h"
#include "ModelIndices.h"
#include "FileMgr.h"
#include "TempColModels.h"
#include "Pickups.h"
#include "Train.h"
#include "TrafficLights.h"
#include "PedRoutes.h"
#include "Sprite.h"
#include "RwHelper.h"
#include "Font.h"
#include "Text.h"
#include "CopPed.h"
#include "Script.h"
#include "CarCtrl.h"
#include "Garages.h"
#include "WaterLevel.h"
#include "CarAI.h"
#include "Zones.h"
#include "Cranes.h"
#include "Timecycle.h"
#include "ParticleObject.h"
#include "Floater.h"
#include "Streaming.h"
#include "PedAttractor.h"
#include "Debug.h"

#define CAN_SEE_ENTITY_ANGLE_THRESHOLD	DEGTORAD(60.0f)

CPed *gapTempPedList[50];
uint16 gnNumTempPedList;

CColPoint aTempPedColPts[MAX_COLLISION_POINTS];

// TODO(Miami)
#define AUDIO_NOT_READY

// Corresponds to ped sounds (from SOUND_PED_DEATH to SOUND_PED_TAXI_CALL)
PedAudioData CommentWaitTime[39] = {
	{500, 800, 500, 2},
	{500, 800, 500, 2},
	{500, 800, 500, 2},
	{500, 800, 500, 2},
	{100, 2, 100, 2},
	{700, 500, 1000, 500},
	{700, 500, 1000, 500},
	{5000, 2000, 15000, 3000},
	{5000, 2000, 15000, 3000},
	{5000, 2000, 15000, 3000},
	{6000, 6000, 6000, 6000},
	{1000, 1000, 2000, 2000},
	{1000, 500, 2000, 1500},
	{1000, 500, 2000, 1500},
	{800, 200, 1000, 500},
	{800, 200, 1000, 500},
	{800, 400, 2000, 1000},
	{800, 400, 2000, 1000},
	{400, 300, 2000, 1000},
	{2000, 1000, 2500, 1500},
	{200, 200, 200, 200},
	{6000, 3000, 5000, 6000},
	{6000, 3000, 9000, 5000},
	{6000, 3000, 9000, 5000},
	{6000, 3000, 9000, 5000},
	{400, 300, 4000, 1000},
	{400, 300, 4000, 1000},
	{400, 300, 4000, 1000},
	{1000, 500, 3000, 1000},
	{1000, 500, 1000, 1000},
	{3000, 2000, 3000, 2000},
	{1000, 500, 3000, 6000},
	{1000, 500, 2000, 4000},
	{1000, 500, 2000, 5000},
	{1000, 500, 3000, 2000},
	{1600, 1000, 2000, 2000},
	{3000, 2000, 5000, 3000},
	{1000, 1000, 1000, 1000},
	{1000, 1000, 5000, 5000},
};
uint16 nPlayerInComboMove;

RpClump *flyingClumpTemp;

FightMove tFightMoves[NUM_FIGHTMOVES] = {
	// TODO(Miami)
};

uint16 CPed::nThreatReactionRangeMultiplier = 1;
uint16 CPed::nEnterCarRangeMultiplier = 1;

CVector vecPedCarDoorAnimOffset;
CVector vecPedCarDoorLoAnimOffset;
CVector vecPedVanRearDoorAnimOffset;
CVector vecPedQuickDraggedOutCarAnimOffset;
CVector vecPedDraggedOutCarAnimOffset;
CVector vecPedTrainDoorAnimOffset;

bool CPed::bNastyLimbsCheat;
bool CPed::bPedCheat2;
bool CPed::bPedCheat3;
CVector2D CPed::ms_vec2DFleePosition;

void *CPed::operator new(size_t sz) { return CPools::GetPedPool()->New();  }
void *CPed::operator new(size_t sz, int handle) { return CPools::GetPedPool()->New(handle); }
void CPed::operator delete(void *p, size_t sz) { CPools::GetPedPool()->Delete((CPed*)p); }
void CPed::operator delete(void *p, int handle) { CPools::GetPedPool()->Delete((CPed*)p); }

static char ObjectiveText[][28] = {
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
#ifdef VC_PED_PORTS
	"Leave Car and Die"
#endif
};

static char StateText[][18] = {
	"None",
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
	"On Fire",
	"Unknown",
	"STATES_NO_AI",
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

static char PersonalityTypeText[][18] = {
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
	"Tramp Male",
	"Tramp Female",
	"Tourist",
	"Prostitute",
	"Criminal",
	"Busker",
	"Taxi Driver",
	"Psycho",
	"Steward",
	"Sports Fan",
	"Shopper",
	"Old Shopper"
};

static char WaitStateText[][16] = {
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

#ifdef TOGGLEABLE_BETA_FEATURES
bool CPed::bPopHeadsOnHeadshot = false;
bool CPed::bMakePedsRunToPhonesToReportCrimes = false;
#endif

#ifndef MASTER
int nDisplayDebugInfo = 0;

void
CPed::SwitchDebugDisplay(void)
{
	nDisplayDebugInfo = !nDisplayDebugInfo;
}

void
CPed::DebugRenderOnePedText(void)
{
	if ((GetPosition() - TheCamera.GetPosition()).MagnitudeSqr() < sq(30.0f)) {
		float width, height;
		RwV3d screenCoords;
		CVector bitAbove = GetPosition();
		bitAbove.z += 2.0f;
		if (CSprite::CalcScreenCoors(bitAbove, &screenCoords, &width, &height, true)) {

			float lineHeight = SCREEN_SCALE_Y(Min(height/100.0f, 0.7f) * 22.0f);

			DefinedState();
			CFont::SetPropOn();
			CFont::SetBackgroundOn();

			// Originally both of them were being divided by 60.0f.
			float xScale = Min(width / 240.0f, 0.7f);
			float yScale = Min(height / 80.0f, 0.7f);

			CFont::SetScale(SCREEN_SCALE_X(xScale), SCREEN_SCALE_Y(yScale));
			CFont::SetCentreOn();
			CFont::SetCentreSize(SCREEN_WIDTH);
			CFont::SetJustifyOff();
			CFont::SetColor(CRGBA(255, 255, 0, 255));
			CFont::SetBackGroundOnlyTextOn();
			CFont::SetFontStyle(0);
			AsciiToUnicode(StateText[m_nPedState], gUString);
			CFont::PrintString(screenCoords.x, screenCoords.y, gUString);
			AsciiToUnicode(ObjectiveText[m_objective], gUString);
			CFont::PrintString(screenCoords.x, screenCoords.y + lineHeight, gUString);
			AsciiToUnicode(PersonalityTypeText[m_pedStats->m_type], gUString);
			CFont::PrintString(screenCoords.x, screenCoords.y + 2 * lineHeight, gUString);
			AsciiToUnicode(WaitStateText[m_nWaitState], gUString);
			CFont::PrintString(screenCoords.x, screenCoords.y + 3 * lineHeight, gUString);
			if (m_nPedState == PED_SEEK_POS || m_nPedState == PED_SEEK_ENTITY) {
				sprintf(gString, "Safe distance to target: %.2f", m_distanceToCountSeekDone);
				AsciiToUnicode(gString, gUString);
				CFont::PrintString(screenCoords.x, screenCoords.y + 4 * lineHeight, gUString);
			}
			DefinedState();
		}
	}
}
#endif

CPed::~CPed(void)
{
	CWorld::Remove(this);
	if (m_attractor)
		GetPedAttractorManager()->DeRegisterPed(this, m_attractor);
	CRadar::ClearBlipForEntity(BLIP_CHAR, CPools::GetPedPool()->GetIndex(this));
	if (InVehicle()){
		uint8 door_flag = GetCarDoorFlag(m_vehEnterType);
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
	if (bCarPassenger)
		CPopulation::ms_nTotalCarPassengerPeds--;
	if (bMiamiViceCop)
		CPopulation::NumMiamiViceCops--;
	CPopulation::UpdatePedCount((ePedType)m_nPedType, true);
	DMAudio.DestroyEntity(m_audioEntityId);
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
		m_nPedState = PED_DEAD;
	else
		m_nPedState = PED_NONE;
	m_pVehicleAnim = nil;
}

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
	m_actionX = 0;
	m_actionY = 0;
	m_phoneTalkTimer = 0;
	m_stateUnused = 0;
	m_leaveCarTimer = 0;
	m_getUpTimer = 0;
	m_attackTimer = 0;
	m_timerUnused = 0;
	m_lookTimer = 0;
	m_standardTimer = 0;
	m_shootTimer = 0;
	m_hitRecoverTimer = 0;
	m_duckAndCoverTimer = 0;
	m_moved = CVector2D(0.0f, 0.0f);
	m_fRotationCur = 0.0f;
	m_headingRate = 15.0f;
	m_fRotationDest = 0.0f;
	m_vehEnterType = CAR_DOOR_LF;
	m_walkAroundType = 0;
	m_pCurrentPhysSurface = nil;
	m_vecOffsetFromPhysSurface = CVector(0.0f, 0.0f, 0.0f);
	m_pSeekTarget = nil;
	m_vecSeekPos = CVector(0.0f, 0.0f, 0.0f);
	m_wepSkills = 0;
	m_distanceToCountSeekDone = 1.0f;
	m_acceptableHeadingOffset = 0.1f;
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

	m_ceaseAttackTimer = 0;

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

	bHasACamera = false;
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

	m_gangFlags = 0xFF;

	bReachedAttractorHeadingTarget = false;
	bTurnedAroundOnAttractor = false;
	bCarPassenger = false;
	bMiamiViceCop = false;
	bMoneyHasBeenGivenByScript = false;
	bHasBeenPhotographed = false;

	bIsDrowning = false;
	bDrownsInWater = true;
#ifdef VC_PED_PORTS
	bHeadStuckInCollision = false;
#endif
	bIsPlayerFriend = true;
	bDeadPedInFrontOfCar = false;
	bStayInCarOnJack = false;

	bDontFight = false;
	bDoomAim = true;
	bCanBeShotInVehicle = true;
	bIgnoreThreatsBehindObjects = false;

	bNeverEverTargetThisPed = false;

	bBoughtIceCream = false;

	if ((CGeneral::GetRandomNumber() & 3) == 0)
		bHasACamera = true;

	m_audioEntityId = DMAudio.CreateEntity(AUDIOTYPE_PHYSICAL, this);
	DMAudio.SetEntityStatus(m_audioEntityId, 1);
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
	m_delayedWeapon = WEAPONTYPE_UNIDENTIFIED;

	for(int i = 0; i < TOTAL_WEAPON_SLOTS; i++) {
		CWeapon &weapon = GetWeapon(i);
		weapon.m_eWeaponType = WEAPONTYPE_UNARMED;
		weapon.m_eWeaponState = WEAPONSTATE_READY;
		weapon.m_nAmmoInClip = 0;
		weapon.m_nAmmoTotal = 0;
		weapon.m_nTimer = 0;
	}

	m_lastFightMove = FIGHTMOVE_NULL;
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
#ifdef PED_SKIN
	m_pWeaponModel = nil;
#endif
	CPopulation::UpdatePedCount((ePedType)m_nPedType, false);
}

// --MIAMI: Done
int32
CPed::GiveWeapon(eWeaponType weaponType, uint32 ammo, bool unused)
{
	int slot = GetWeaponSlot(weaponType);

	if (m_weapons[slot].m_eWeaponType == weaponType) {
		GetWeapon(slot).m_nAmmoTotal += ammo;
		if (weaponType < WEAPONTYPE_LAST_WEAPONTYPE && weaponType > WEAPONTYPE_UNARMED && CWeaponInfo::ms_aMaxAmmoForWeapon[weaponType] >= 0) {

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

			// TODO(Miami): Make an enum for that
			if (slot == 4 || slot == 5 || slot == 6)
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

			if (currentPedState == PED_FALL || currentPedState == PED_GETUP || currentPedState == PED_DIE || currentPedState == PED_DEAD) {
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

bool
CPed::IsPlayer(void)
{
	return m_nPedType == PEDTYPE_PLAYER1 || m_nPedType == PEDTYPE_PLAYER2 ||
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
	return !DyingOrDead() && m_nPedState != PED_ARRESTED && !EnteringCar() && m_nPedState != PED_STEAL_CAR;
}

bool
CPed::IsPedInControl(void)
{
	return m_nPedState <= PED_STATES_NO_AI
		&& !bIsInTheAir && !bIsLanding
		&& m_fHealth > 0.0f;
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
CPed::AddWeaponModel(int id)
{
	RpAtomic *atm;

	if (id != -1) {
#ifdef PED_SKIN
		if (IsClumpSkinned(GetClump())) {
			if (m_pWeaponModel)
				RemoveWeaponModel(-1);

			m_pWeaponModel = (RpAtomic*)CModelInfo::GetModelInfo(id)->CreateInstance();
		}
		else
#endif
		{
			atm = (RpAtomic*)CModelInfo::GetModelInfo(id)->CreateInstance();
			RwFrameDestroy(RpAtomicGetFrame(atm));
			RpAtomicSetFrame(atm, m_pFrames[PED_HANDR]->frame);
			RpClumpAddAtomic(GetClump(), atm);
		}
		CModelInfo::GetModelInfo(id)->AddRef();
		m_wepModelID = id;

		// TODO(Miami)
		// if (IsPlayer() && id == MI_MINIGUN)
		//	((CPlayerPed*)this)->m_pMinigunTopAtomic = (RpAtomic*)CModelInfo::GetModelInfo(MI_MINIGUN2)->CreateInstance();
	}
}

void
CPed::AimGun(void)
{
	RwV3d pos;
	CVector vector;

	if (m_pSeekTarget) {
		if (m_pSeekTarget->IsPed()) {
			((CPed*)m_pSeekTarget)->m_pedIK.GetComponentPosition(&pos, PED_MID);
			vector = pos;
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
			CPed::SetDie(ANIM_KO_SHOT_FRONT1, 4.0f, 0.0f);
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
#ifdef TOGGLEABLE_BETA_FEATURES
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

			if (nearestPed && nearestPed->m_nPedState != PED_DEAD && nearestPed != m_pSeekTarget && nearestPed != m_pedInObjective) {

				// Check if this ped wants to avoid the nearest one
				if (CPedType::GetAvoid(this->m_nPedType) & CPedType::GetFlag(nearestPed->m_nPedType)) {

					// Further codes checks whether the distance between us and ped will be equal or below 1.0, if we walk up to him by 1.25 meters.
					// If so, we want to avoid it, so we turn our body 45 degree and look to somewhere else.

					// Game converts from radians to degress and back again here, doesn't make much sense
					CVector2D forward(-Sin(m_fRotationCur), Cos(m_fRotationCur));
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
							CPed::SetLookFlag(nearestPed, false);
							CPed::SetLookTimer(CGeneral::GetRandomNumberInRange(500, 800));
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
		m_pedIK.m_flags &= ~CPedIK::AIMS_WITH_ARM;
	}

	if (IsPlayer())
		((CPlayerPed*)this)->m_fFPSMoveHeading = 0.0f;
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
			RestorePreviousState();
			ClearLookFlag();
		}
	}
}

bool
CPed::IsPedHeadAbovePos(float zOffset)
{
	return zOffset + GetPosition().z < GetNodePosition(PED_HEAD).z;
}

// --MIAMI: Done
void
CPed::FinishedReloadCB(CAnimBlendAssociation *reloadAssoc, void *arg)
{
	CPed *ped = (CPed*)arg;
	CWeaponInfo *weapon = CWeaponInfo::GetWeaponInfo(ped->GetWeapon()->m_eWeaponType);
	
	if (ped->DyingOrDead())
		return;

	if (ped->bIsDucking && ped->bCrouchWhenShooting) {
		CAnimBlendAssociation *crouchFireAssoc = nil;
		if (!!weapon->m_bCrouchFire) {
			crouchFireAssoc = RpAnimBlendClumpGetAssociation(ped->GetClump(), GetCrouchFireAnim(weapon));
		}
		if (!!weapon->m_bReload && reloadAssoc) {
			if (reloadAssoc->animId == GetCrouchReloadAnim(weapon) && !crouchFireAssoc) {
				CAnimBlendAssociation *crouchAssoc = CAnimManager::BlendAnimation(ped->GetClump(), ASSOCGRP_STD, ANIM_WEAPON_CROUCH, 8.0f);
				crouchAssoc->SetCurrentTime(crouchAssoc->hierarchy->totalLength);
				crouchAssoc->flags &= ~ASSOC_RUNNING;
			}
		}
	} else if (weapon->m_bReloadLoop2Start && ped->bIsAttacking) {
		CAnimBlendAssociation *fireAssoc =
			CAnimManager::BlendAnimation(ped->GetClump(), weapon->m_AnimToPlay, GetPrimaryFireAnim(weapon), 8.0f);
		fireAssoc->SetFinishCallback(FinishedAttackCB, ped);
		fireAssoc->SetRun();
		if (fireAssoc->currentTime != reloadAssoc->hierarchy->totalLength) {
			if (fireAssoc->currentTime >= weapon->m_fAnimLoopStart)
				return;

			fireAssoc->SetCurrentTime(Max(weapon->m_fAnimLoopStart - 0.04f, 0.0f));
		} else {
			fireAssoc->SetCurrentTime(Max(weapon->m_fAnimLoopStart - 0.04f, 0.0f));
		}
	}
}

// --MIAMI: Done
void
CPed::FinishedAttackCB(CAnimBlendAssociation *attackAssoc, void *arg)
{
	CAnimBlendAssociation *newAnim, *reloadAnimAssoc;
	CPed *ped = (CPed*)arg;
	CWeaponInfo *currentWeapon = CWeaponInfo::GetWeaponInfo(ped->GetWeapon()->m_eWeaponType);

	if (ped->m_nPedState != PED_ATTACK) {
		if (ped->bIsDucking && ped->IsPedInControl()) {
			if (currentWeapon->m_bReload) {
				reloadAnimAssoc = RpAnimBlendClumpGetAssociation(ped->GetClump(), GetCrouchReloadAnim(currentWeapon));
			}
			if (currentWeapon->m_bCrouchFire && attackAssoc) {
				if (attackAssoc->animId == GetCrouchFireAnim(currentWeapon) && !reloadAnimAssoc) {
					newAnim = CAnimManager::BlendAnimation(ped->GetClump(), ASSOCGRP_STD, ANIM_WEAPON_CROUCH, 8.0f);
					newAnim->SetCurrentTime(newAnim->hierarchy->totalLength);
					newAnim->flags &= ~ASSOC_RUNNING;
				}
			}
		}
		return;
	}
	if (attackAssoc && attackAssoc->animId == ANIM_THROWABLE_START_THROW && currentWeapon->m_AnimToPlay == ASSOCGRP_THROW) {
		if ((!ped->IsPlayer() || ((CPlayerPed*)ped)->m_bHaveTargetSelected) && ped->IsPlayer()) {
			attackAssoc->blendDelta = -1000.0f;
			newAnim = CAnimManager::AddAnimation(ped->GetClump(), currentWeapon->m_AnimToPlay, ANIM_THROWABLE_THROWU);
		} else {
			attackAssoc->blendDelta = -1000.0;
			newAnim = CAnimManager::AddAnimation(ped->GetClump(), currentWeapon->m_AnimToPlay, ANIM_THROWABLE_THROW);
		}
		newAnim->SetFinishCallback(FinishedAttackCB, ped);
		return;
	}

	if (ped->bIsDucking && ped->bCrouchWhenShooting) {
		if (currentWeapon->m_bReload) {
			reloadAnimAssoc = RpAnimBlendClumpGetAssociation(ped->GetClump(), GetCrouchReloadAnim(currentWeapon));
		}
		if (currentWeapon->m_bCrouchFire && attackAssoc) {
			if (attackAssoc->animId == GetCrouchFireAnim(currentWeapon) && !reloadAnimAssoc) {
				newAnim = CAnimManager::BlendAnimation(ped->GetClump(), ASSOCGRP_STD, ANIM_WEAPON_CROUCH, 8.0f);
				newAnim->SetCurrentTime(newAnim->hierarchy->totalLength);
				newAnim->flags &= ~ASSOC_RUNNING;
			}
		}

		if (!ped->bIsAttacking)
			ped->ClearAttack();

		return;
	}
	
	// Not for unarmed, it's for weapons using unarmed anims
	if (currentWeapon->m_bUse2nd && ped->bIsAttacking && currentWeapon->m_AnimToPlay == ASSOCGRP_UNARMED) {
		AnimationId groundAnim = GetFireAnimGround(currentWeapon);
		CAnimBlendAssociation *groundAnimAssoc = RpAnimBlendClumpGetAssociation(ped->GetClump(), groundAnim);
		if (!groundAnimAssoc || groundAnimAssoc->blendAmount <= 0.95f && groundAnimAssoc->blendDelta <= 0.0f) {
			if (attackAssoc && attackAssoc->animId == ANIM_MELEE_ATTACK) {
				newAnim = CAnimManager::BlendAnimation(
					ped->GetClump(), currentWeapon->m_AnimToPlay, ANIM_MELEE_ATTACK_2ND, 8.0f);
			} else {
				newAnim = CAnimManager::BlendAnimation(
					ped->GetClump(), currentWeapon->m_AnimToPlay, ANIM_MELEE_ATTACK, 8.0f);
			}
			newAnim->SetFinishCallback(FinishedAttackCB, ped);
		}
	} else {
		if (attackAssoc && attackAssoc->animId == ANIM_MELEE_ATTACK && currentWeapon->m_AnimToPlay == ASSOCGRP_UNARMED)
		{
			attackAssoc->blendDelta = -8.0f;
			attackAssoc->flags |= ASSOC_DELETEFADEDOUT;
			ped->ClearAttack();
			return;
		}
		if (attackAssoc)
		{
			if (currentWeapon->m_AnimToPlay == ASSOCGRP_THROW)
			{
				if ((attackAssoc->animId == ANIM_THROWABLE_THROW || attackAssoc->animId == ANIM_THROWABLE_THROWU) && ped->GetWeapon()->m_nAmmoTotal > 0)
				{
					ped->RemoveWeaponModel(currentWeapon->m_nModelId);
					ped->AddWeaponModel(currentWeapon->m_nModelId);
				}
			}
		}

		if (!ped->bIsAttacking)
			ped->ClearAttack();
	}
}

// --MIAMI: Done except commented things
void
CPed::Attack(void)
{
	CAnimBlendAssociation *weaponAnimAssoc;
	int32 weaponAnim;
	eWeaponType ourWeaponType;
	float weaponAnimTime;
	eWeaponFire ourWeaponFire;
	float animLoopEnd;
	CWeaponInfo *ourWeapon;
	bool attackShouldContinue;
	CAnimBlendAssociation *reloadAnimAssoc;
	CAnimBlendAssociation *throwAssoc;
	float delayBetweenAnimAndFire;
	float animLoopStart;
	CVector firePos;

	ourWeaponType = GetWeapon()->m_eWeaponType;
	ourWeapon = CWeaponInfo::GetWeaponInfo(ourWeaponType);
	ourWeaponFire = ourWeapon->m_eWeaponFire;
	weaponAnimAssoc = nil;
	attackShouldContinue = bIsAttacking;
	reloadAnimAssoc = nil;
	throwAssoc = nil;
	animLoopStart = ourWeapon->m_fAnimLoopStart;
	animLoopEnd = ourWeapon->m_fAnimLoopEnd;
	delayBetweenAnimAndFire = ourWeapon->m_fAnimFrameFire;
	weaponAnim = ourWeapon->m_AnimToPlay;

	if (bIsDucking) {
		if (!!ourWeapon->m_bCrouchFire && bCrouchWhenShooting) {
			weaponAnimAssoc = RpAnimBlendClumpGetAssociation(GetClump(), GetCrouchFireAnim(ourWeapon));
			if (weaponAnimAssoc) {
				animLoopStart = ourWeapon->m_fAnim2LoopStart;
				animLoopEnd = ourWeapon->m_fAnim2LoopEnd;
				delayBetweenAnimAndFire = ourWeapon->m_fAnim2FrameFire;
			}
		}
	} else {
		AnimationId anim = GetFireAnimNotDucking(ourWeapon);
		weaponAnimAssoc = RpAnimBlendClumpGetAssociation(GetClump(), anim);
		if (anim == ANIM_WEAPON_FIRE_3RD && weaponAnimAssoc) {
			animLoopStart = 11.f/30.f;
			animLoopEnd = 19.f/30.f;
			delayBetweenAnimAndFire = 14.f/30.f;
		}
	}

	if (ourWeapon->m_bReload) {
		reloadAnimAssoc = RpAnimBlendClumpGetAssociation(GetClump(), GetReloadAnim(ourWeapon));
	}

	if (!!ourWeapon->m_bReload && !reloadAnimAssoc)  {
		reloadAnimAssoc = RpAnimBlendClumpGetAssociation(GetClump(), GetCrouchReloadAnim(ourWeapon));
	}

	if ( reloadAnimAssoc && reloadAnimAssoc->IsRunning() ) {
		if (!IsPlayer() || ((CPlayerPed*)this)->m_bHaveTargetSelected)
			ClearAttack();
		return;
	}

	if ( reloadAnimAssoc ) {
		reloadAnimAssoc->flags |= ASSOC_DELETEFADEDOUT;
		if ( reloadAnimAssoc->blendDelta >= 0.0f )
			reloadAnimAssoc->blendDelta = -8.0f;
	}

	if (!!ourWeapon->m_bThrow)  {
		throwAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_THROWABLE_START_THROW);
	}

	if ( CTimer::GetTimeInMilliseconds() < m_shootTimer )
		attackShouldContinue = true;

	bool meleeAttackStarted = false;
	if ( !weaponAnimAssoc ) {
		if (!!ourWeapon->m_bPartialAttack) {
			weaponAnimAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_MELEE_ATTACK_START);
			if ( weaponAnimAssoc ) {
				if ( IsPlayer() )
					meleeAttackStarted = true;

				switch ( ourWeapon->m_AnimToPlay ) {
					case ASSOCGRP_UNARMED:
					case ASSOCGRP_SCREWDRIVER:
					case ASSOCGRP_KNIFE:
					case ASSOCGRP_BASEBALLBAT:
					case ASSOCGRP_GOLFCLUB:
					case ASSOCGRP_CHAINSAW:
						delayBetweenAnimAndFire = 0.2f;
						animLoopStart = 0.1f;
						break;
					default:
						break;
				}
				animLoopEnd = 99.9f;
			}
		}
	}
	if (!weaponAnimAssoc) {
		if (!!ourWeapon->m_bUse2nd) {
			weaponAnimAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_WEAPON_FIRE_2ND);
			if (weaponAnimAssoc) {
				animLoopStart = ourWeapon->m_fAnim2LoopStart;
				animLoopEnd = ourWeapon->m_fAnim2LoopEnd;
				delayBetweenAnimAndFire = ourWeapon->m_fAnim2FrameFire;
			}
		}
	}
	if (!weaponAnimAssoc) {
		weaponAnimAssoc = RpAnimBlendClumpGetAssociation(GetClump(), GetFireAnimGround(ourWeapon));
		if (weaponAnimAssoc) {
			animLoopStart = ourWeapon->m_fAnim2LoopStart;
			animLoopEnd = ourWeapon->m_fAnim2LoopEnd;
			delayBetweenAnimAndFire = ourWeapon->m_fAnim2FrameFire;
		}
	}

	if (!weaponAnimAssoc) {
		if (!throwAssoc) {
			if (attackShouldContinue) {
				if (ourWeaponFire != WEAPON_FIRE_PROJECTILE || !IsPlayer() || ((CPlayerPed*)this)->m_bHaveTargetSelected) {
					if (bCrouchWhenShooting && bIsDucking && !!ourWeapon->m_bCrouchFire) {
						weaponAnimAssoc = CAnimManager::BlendAnimation(GetClump(), ourWeapon->m_AnimToPlay, GetCrouchFireAnim(ourWeapon), 8.0f);

					} else if(!!ourWeapon->m_bUse2nd && CGeneral::GetRandomNumber() & 1){
						weaponAnimAssoc = CAnimManager::BlendAnimation(GetClump(), ourWeapon->m_AnimToPlay, ANIM_WEAPON_FIRE_2ND, 8.0f);

					} else if (!CGame::nastyGame || (!ourWeapon->m_bGround2nd && !ourWeapon->m_bGround3rd) ||
						ourWeaponFire != WEAPON_FIRE_MELEE || CheckForPedsOnGroundToAttack(this, nil) < PED_ON_THE_FLOOR) {

						weaponAnimAssoc = CAnimManager::BlendAnimation(GetClump(), ourWeapon->m_AnimToPlay, GetFireAnimNotDucking(ourWeapon), 8.0f);

					} else {
						weaponAnimAssoc = CAnimManager::BlendAnimation(GetClump(), ourWeapon->m_AnimToPlay, GetFireAnimGround(ourWeapon, false), 8.0f);
					}

					weaponAnimAssoc->SetFinishCallback(CPed::FinishedAttackCB, this);
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

		}
		return;
	}

	if (meleeAttackStarted && IsPlayer()) {
		if (((CPlayerPed*)this)->m_bHaveTargetSelected || ((CPlayerPed*)this)->m_fMoveSpeed < 0.5f) {
			weaponAnimAssoc->SetRun();
		} else {
			if (weaponAnimAssoc->currentTime > animLoopStart && weaponAnimAssoc->currentTime - weaponAnimAssoc->timeStep <= animLoopStart)
				weaponAnimAssoc->flags &= ~ASSOC_RUNNING;
		}
	}

	float animStart = ourWeapon->m_fAnimLoopStart * 0.4f;
	weaponAnimTime = weaponAnimAssoc->currentTime;
	if (weaponAnimTime > animStart && weaponAnimTime - weaponAnimAssoc->timeStep <= animStart) {
		if (!bIsDucking && !(m_nPedType == PEDTYPE_COP && ourWeapon->m_bCop3rd && weaponAnimAssoc->animId == ANIM_WEAPON_FIRE_3RD) && ourWeapon->m_bCanAimWithArm)
			m_pedIK.m_flags |= CPedIK::AIMS_WITH_ARM;
		else
			m_pedIK.m_flags &= ~CPedIK::AIMS_WITH_ARM;
	}

	if (ourWeaponType != WEAPONTYPE_CHAINSAW
		|| !meleeAttackStarted && delayBetweenAnimAndFire - 0.5f >= weaponAnimAssoc->currentTime
		|| weaponAnimAssoc->currentTime - weaponAnimAssoc->timeStep > delayBetweenAnimAndFire) {

		if (ourWeaponType == WEAPONTYPE_CHAINSAW) {
#ifndef AUDIO_NOT_READY
			DMAudio.PlayOneShot(m_audioEntityId, 52, 0.0f);
#endif
		} else if (weaponAnimTime <= delayBetweenAnimAndFire || weaponAnimTime - weaponAnimAssoc->timeStep > delayBetweenAnimAndFire || !weaponAnimAssoc->IsRunning()) {
			if (weaponAnimAssoc->speed < 1.0f)
				weaponAnimAssoc->speed = 1.0f;

		} else {
			firePos = ourWeapon->m_vecFireOffset;

			if (ourWeaponType != WEAPONTYPE_KATANA && ourWeaponType != WEAPONTYPE_CHAINSAW) {
				if (ourWeapon->m_eWeaponFire != WEAPON_FIRE_MELEE) {
					TransformToNode(firePos, (weaponAnimAssoc->animId == ANIM_MELEE_ATTACK_2ND && ourWeapon->m_AnimToPlay == ASSOCGRP_UNARMED) ? PED_FOOTR : PED_HANDR);
				} else {
					firePos = GetMatrix() * firePos;
				}
			} else {
				if (weaponAnimAssoc->animId == ANIM_MELEE_ATTACK_2ND)
					firePos.z = 0.7f * ourWeapon->m_fRadius - 1.0f;

				firePos = GetMatrix() * firePos;
			}
			
			GetWeapon()->Fire(this, &firePos);

			// TODO(Miami): Teargas
			if (ourWeaponType == WEAPONTYPE_MOLOTOV || ourWeaponType == WEAPONTYPE_GRENADE || ourWeaponType == WEAPONTYPE_DETONATOR_GRENADE 
				/* ourWeaponType == WEAPONTYPE_TEARGAS*/) {
				RemoveWeaponModel(ourWeapon->m_nModelId);
			}
			if (!GetWeapon()->m_nAmmoTotal && ourWeaponFire != WEAPON_FIRE_MELEE && FindPlayerPed() != this) {
				SelectGunIfArmed();
			}

			if (GetWeapon()->m_eWeaponState == WEAPONSTATE_MELEE_MADECONTACT) {
				int damagerType = ENTITY_TYPE_NOTHING;
				if (m_pDamageEntity && (m_fDamageImpulse == 0.0f || !m_pDamageEntity->IsBuilding())) {
					damagerType = m_pDamageEntity->GetType();
				}
				switch (ourWeapon->m_AnimToPlay) {
					case ASSOCGRP_UNARMED:
						if (weaponAnimAssoc->animId == ANIM_MELEE_ATTACK || weaponAnimAssoc->animId == ANIM_MELEE_ATTACK_START) {
#ifdef AUDIO_NOT_READY
							DMAudio.PlayOneShot(m_audioEntityId, SOUND_FIGHT_PUNCH_39, 0.0f);
#else
							DMAudio.PlayOneShot(m_audioEntityId, SOUND_FIGHT_PUNCH_39, (damagerType | (ourWeaponType << 8)));
#endif
						}
						break;
					case ASSOCGRP_KNIFE:
					case ASSOCGRP_BASEBALLBAT:
					case ASSOCGRP_GOLFCLUB:
					case ASSOCGRP_CHAINSAW:
#ifdef AUDIO_NOT_READY
						DMAudio.PlayOneShot(m_audioEntityId, SOUND_WEAPON_BAT_ATTACK, 1.0f);
#else
						DMAudio.PlayOneShot(m_audioEntityId, SOUND_WEAPON_BAT_ATTACK, (damagerType | (ourWeaponType << 8)));
#endif
						break;
					default:
						break;
				}

				if (bIsAttacking || CTimer::GetTimeInMilliseconds() < m_shootTimer) {
					weaponAnimAssoc->callbackType = 0;
				}
			}

			attackShouldContinue = false;
		}
	} else {
		CVector firePos = ourWeapon->m_vecFireOffset;

		if (weaponAnimAssoc->animId == 206)
			firePos.z = 0.7f * ourWeapon->m_fRadius - 1.0f;

		firePos = GetMatrix() * firePos;
		GetWeapon()->Fire(this, &firePos);
		if (GetWeapon()->m_eWeaponState == WEAPONSTATE_MELEE_MADECONTACT) {
			int damagerType = ENTITY_TYPE_PED;
			if (m_pDamageEntity)
				damagerType = m_pDamageEntity->GetType();

			DMAudio.PlayOneShot(m_audioEntityId, 54, (float)damagerType);
			if (IsPlayer()) {
				CPad::GetPad(0)->StartShake(240, 180);
			}
		} else {
			DMAudio.PlayOneShot(m_audioEntityId, 53, 0.0f);
			if (IsPlayer()) {
				CPad::GetPad(0)->StartShake(240, 90);
			}
		}
		attackShouldContinue = false;
	}

	if (ourWeapon->m_eWeaponFire == WEAPON_FIRE_INSTANT_HIT && ourWeapon->m_AnimToPlay == ASSOCGRP_SHOTGUN) {
		weaponAnimTime = weaponAnimAssoc->currentTime;
		firePos = ourWeapon->m_vecFireOffset;

		//TODO(Miami): Check
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

	// TODO(Miami): CSpecialFX::AddWeaponStreak

	// Anim breakout on running
	if (IsPlayer()) {
		if (CPad::GetPad(0)->GetSprint()) {
			if (!attackShouldContinue && weaponAnimAssoc->currentTime > ourWeapon->m_fAnimBreakout) {
				weaponAnimAssoc->blendDelta = -4.0f;
				FinishedAttackCB(nil, this);
				return;
			}
		}
	}

	weaponAnimTime = weaponAnimAssoc->currentTime;

	// Anim loop end, either start the loop again or finish the attack
	if (weaponAnimTime > animLoopEnd || !weaponAnimAssoc->IsRunning() && ourWeaponFire != WEAPON_FIRE_PROJECTILE) {
		if (GetWeapon()->m_eWeaponState == WEAPONSTATE_RELOADING) {
			if (ourWeapon->m_bReload && !reloadAnimAssoc) {
				if (!CWorld::Players[CWorld::PlayerInFocus].m_bFastReload) {
					CAnimBlendAssociation *newReloadAssoc;
					if (bIsDucking) {
						newReloadAssoc = CAnimManager::BlendAnimation(
							GetClump(), ourWeapon->m_AnimToPlay,
							GetCrouchReloadAnim(ourWeapon),
							8.0f);
					} else {
						newReloadAssoc = CAnimManager::BlendAnimation(
							GetClump(), ourWeapon->m_AnimToPlay,
							GetReloadAnim(ourWeapon),
							8.0f);
					}
					newReloadAssoc->SetFinishCallback(FinishedReloadCB, this);
				}
				ClearLookFlag();
				ClearAimFlag();
				bIsAttacking = false;
				bIsPointingGunAt = false;
				m_shootTimer = CTimer::GetTimeInMilliseconds();
#ifdef AUDIO_NOT_READY
				switch (ourWeaponType) {
					case WEAPONTYPE_UZI:
					case WEAPONTYPE_TEC9:
					case WEAPONTYPE_SILENCED_INGRAM:
					case WEAPONTYPE_MP5:
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
#else
				DMAudio.PlayOneShot(m_audioEntityId, SOUND_WEAPON_AK47_BULLET_ECHO, GetWeapon()->m_eWeaponType);
#endif
				return;
			}
		}
		if (weaponAnimTime - 2.0f * weaponAnimAssoc->timeStep <= animLoopEnd
			&& (bIsAttacking || CTimer::GetTimeInMilliseconds() < m_shootTimer)
			&& (GetWeapon()->m_eWeaponState != WEAPONSTATE_RELOADING
				/* || GetWeapon()->m_nWeaponType == WEAPONTYPE_MINIGUN */)) { // TODO(Miami): Minigun

			PedOnGroundState pedOnGroundState;
			if (ourWeapon->m_eWeaponFire == WEAPON_FIRE_MELEE &&
				(CGame::nastyGame && ((pedOnGroundState = CheckForPedsOnGroundToAttack(this, nil)) > PED_IN_FRONT_OF_ATTACKER)
				|| ourWeaponType == WEAPONTYPE_BASEBALLBAT && pedOnGroundState == NO_PED && bIsStanding && m_pCurSurface && m_pCurSurface->IsVehicle())) {

				AnimationId fireAnim = GetFireAnimGround(ourWeapon, false);
				if (weaponAnimAssoc->animId == fireAnim)
					weaponAnimAssoc->SetCurrentTime(0.1f);
				else {
					if (fireAnim) {
						weaponAnimAssoc = CAnimManager::BlendAnimation(GetClump(), ourWeapon->m_AnimToPlay, fireAnim, 8.0f);
					} else {
						weaponAnimAssoc = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_KICK_FLOOR, 8.0f);
					}
				}
				weaponAnimAssoc->SetFinishCallback(FinishedAttackCB, this);
			} else if (!!ourWeapon->m_bUse2nd) {
				if (weaponAnimAssoc->animId == ANIM_WEAPON_FIRE_2ND) {
					weaponAnimAssoc = CAnimManager::BlendAnimation(GetClump(), ourWeapon->m_AnimToPlay, ANIM_WEAPON_FIRE, 8.0f);
				} else {
					weaponAnimAssoc = CAnimManager::BlendAnimation(GetClump(), ourWeapon->m_AnimToPlay, ANIM_WEAPON_FIRE_2ND, 8.0f);
				}
				weaponAnimAssoc->SetFinishCallback(FinishedAttackCB, this);
			} else {
				weaponAnimAssoc->SetCurrentTime(animLoopStart);
				weaponAnimAssoc->SetRun();
			}
		}
	} else if (IsPlayer() && m_pPointGunAt && bIsAimingGun && GetWeapon()->m_eWeaponState != WEAPONSTATE_RELOADING) {
		weaponAnimAssoc->SetCurrentTime(animLoopEnd);
		weaponAnimAssoc->flags &= ~ASSOC_RUNNING;
		SetPointGunAt(m_pPointGunAt);
	} else {
		ClearAimFlag();

		// Echoes of bullets, at the end of the attack. (Bug: doesn't play while reloading)
		if (weaponAnimAssoc->currentTime - weaponAnimAssoc->timeStep < animLoopEnd) {

#ifdef AUDIO_NOT_READY
			switch (ourWeaponType) {
				case WEAPONTYPE_UZI:
				case WEAPONTYPE_TEC9:
				case WEAPONTYPE_SILENCED_INGRAM:
				case WEAPONTYPE_MP5:
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
#else
			DMAudio.PlayOneShot(m_audioEntityId, SOUND_WEAPON_AK47_BULLET_ECHO, ourWeaponType);
#endif
		}

		// Fun fact: removing this part leds to reloading flamethrower
		if (ourWeaponType == WEAPONTYPE_FLAMETHROWER && weaponAnimAssoc->IsRunning()) {
			weaponAnimAssoc->flags |= ASSOC_DELETEFADEDOUT;
			weaponAnimAssoc->flags &= ~ASSOC_RUNNING;
			weaponAnimAssoc->blendDelta = -4.0f;
		}
	}

	if (weaponAnimAssoc->currentTime > delayBetweenAnimAndFire)
		attackShouldContinue = false;

	bIsAttacking = attackShouldContinue;
}

// --MIAMI: Done
void
CPed::RemoveWeaponModel(int modelId)
{
	// modelId is not used!! This function just removes the current weapon.
#ifdef PED_SKIN
	if(IsClumpSkinned(GetClump())){
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
	}else
#endif
		RwFrameForAllObjects(m_pFrames[PED_HANDR]->frame,RemoveAllModelCB,nil);

	// TODO(Miami): Minigun
	if (IsPlayer() && (modelId == -1 /* || modelId == MI_MINIGUN)*/)) {
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

// --MIAMI: Done
void
CPed::SetCurrentWeapon(eWeaponType weaponType)
{
	SetCurrentWeapon(CWeaponInfo::GetWeaponInfo(weaponType)->m_nWeaponSlot);
}

// --MIAMI: Done
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

// --MIAMI: Done
// Only used while deciding which gun ped should switch to, if no ammo left.
bool
CPed::SelectGunIfArmed(void)
{
	for (int i = 0; i < TOTAL_WEAPON_SLOTS; i++) {
		if (GetWeapon(i).m_nAmmoTotal > 0) {
			eWeaponType weaponType = GetWeapon(i).m_eWeaponType;

			// First condition checks for Pistol, Python and Shotguns
			if ((weaponType >= WEAPONTYPE_COLT45 && weaponType < WEAPONTYPE_TEC9) ||
				weaponType == WEAPONTYPE_UZI || weaponType == WEAPONTYPE_M16 || weaponType == WEAPONTYPE_MP5 ||
				weaponType == WEAPONTYPE_ROCKETLAUNCHER || weaponType == WEAPONTYPE_FLAMETHROWER || weaponType == WEAPONTYPE_SNIPERRIFLE) {
				SetCurrentWeapon(i);
				return true;
			}
		}
	}
	SetCurrentWeapon(WEAPONTYPE_UNARMED);
	return false;
}

// --MIAMI: Done
void
CPed::Duck(void)
{
	if (CTimer::GetTimeInMilliseconds() > m_duckTimer)
		ClearDuck();
	else if (bIsDucking && bCrouchWhenShooting) {
		CWeaponInfo *weapon = CWeaponInfo::GetWeaponInfo(GetWeapon()->m_eWeaponType);
		CAnimBlendAssociation *attackAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_WEAPON_CROUCH);
		if (!attackAssoc) {
			if(!!weapon->m_bCrouchFire)
				attackAssoc = RpAnimBlendClumpGetAssociation(GetClump(), GetCrouchFireAnim(weapon));
		}
		if (!attackAssoc) {
			if(!!weapon->m_bReload)
				attackAssoc = RpAnimBlendClumpGetAssociation(GetClump(), GetCrouchReloadAnim(weapon));
		}
		if (!attackAssoc) {
			bIsDucking = false;
		}
	}
}

// --MIAMI: Done
void
CPed::ClearDuck(bool clearTimer)
{
	CAnimBlendAssociation *animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_DUCK_DOWN);
	if (!animAssoc) {
		animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_DUCK_LOW);
	}

	if (!animAssoc) {
		animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_WEAPON_CROUCH);
	}

	if (animAssoc) {
		animAssoc->flags |= ASSOC_DELETEFADEDOUT;
		animAssoc->blendDelta = -4.0f;
	}
	bIsDucking = false;

	if (clearTimer) {
		m_duckTimer = 0;
	}
}

// --MIAMI: Done
void
CPed::ClearPointGunAt(void)
{
	CAnimBlendAssociation *animAssoc;
	CWeaponInfo *weaponInfo;

	ClearLookFlag();
	ClearAimFlag();
	bIsPointingGunAt = false;
	if (m_nPedState == PED_AIM_GUN || m_nPedState == PED_ATTACK) {

		if (m_nPedState == PED_FOLLOW_PATH)
			ClearFollowPath();
		m_nPedState = PED_IDLE;
		RestorePreviousState();
	}
	weaponInfo = CWeaponInfo::GetWeaponInfo(GetWeapon()->m_eWeaponType);
	animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_WEAPON_FIRE);
	if (!animAssoc || animAssoc->blendDelta < 0.0f) {
		if (!!weaponInfo->m_bCrouchFire) {
			animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), GetCrouchFireAnim(weaponInfo));
		}
	}
	if (animAssoc) {
		animAssoc->flags |= ASSOC_DELETEFADEDOUT;
		animAssoc->blendDelta = -4.0f;
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
		CAnimManager::BlendAnimation(GetClump(), m_animGroup, ANIM_IDLE_STANCE, 100.0f);
		animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_CAR_SIT);
		if (!animAssoc) {
			animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_CAR_LSIT);
			if (!animAssoc) {
				animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_CAR_SITP);
				if (!animAssoc)
					animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_CAR_SITPLO);
			}
		}
		if (animAssoc)
			animAssoc->blendDelta = -1000.0f;

		if (m_vehEnterType == CAR_DOOR_LF || m_vehEnterType == CAR_DOOR_LR) {
			if (bWillBeQuickJacked) {
				enterAnim = ANIM_CAR_QJACKED;
			} else if (m_pMyVehicle->bLowVehicle) {
				enterAnim = ANIM_CAR_LJACKED_LHS;
			} else {
				enterAnim = ANIM_CAR_JACKED_LHS;
			}
		} else if (m_vehEnterType == CAR_DOOR_RF || m_vehEnterType == CAR_DOOR_RR) {
			if (m_pMyVehicle->bLowVehicle)
				enterAnim = ANIM_CAR_LJACKED_RHS;
			else
				enterAnim = ANIM_CAR_JACKED_RHS;
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
			m_pMyVehicle->ProcessOpenDoor(m_vehEnterType, NUM_STD_ANIMS, m_pVehicleAnim->currentTime * 5.0f);
		}
	}
#endif
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
CPed::PedSetDraggedOutCarCB(CAnimBlendAssociation *dragAssoc, void *arg)
{
	CAnimBlendAssociation *quickJackedAssoc;
	CVehicle *vehicle; 
	CPed *ped = (CPed*)arg;

	quickJackedAssoc = RpAnimBlendClumpGetAssociation(ped->GetClump(), ANIM_CAR_QJACKED);
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
		vehicle->m_nGettingOutFlags &= ~GetCarDoorFlag(ped->m_vehEnterType);

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
		ped->SetDie(ANIM_FLOOR_HIT, 1000.0f, 0.5f);
		return;
	}
#endif

	if (quickJackedAssoc) {
		dragAssoc->SetDeleteCallback(PedSetQuickDraggedOutCarPositionCB, ped);
	} else {
		dragAssoc->SetDeleteCallback(PedSetDraggedOutCarPositionCB, ped);
		if (ped->CanSetPedState())
			CAnimManager::BlendAnimation(ped->GetClump(), ASSOCGRP_STD, ANIM_GETUP1, 1000.0f);
	}

	ped->ReplaceWeaponWhenExitingVehicle();

	ped->m_nStoredMoveState = PEDMOVE_NONE;
	ped->bVehExitWillBeInstant = false;
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
			if (vehModel->m_vehicleType == VEHICLE_TYPE_BOAT)
				vehDoorPos = vehModel->m_positions[BOAT_POS_FRONTSEAT];
			else
				vehDoorPos = vehModel->m_positions[CAR_POS_FRONTSEAT];

			vehDoorPos.x += seatOffset;
			vehDoorOffset.x = -vehDoorOffset.x;
			break;

		case CAR_DOOR_RR:
			vehDoorPos = vehModel->m_positions[CAR_POS_BACKSEAT];
			vehDoorPos.x += seatOffset;
			vehDoorOffset.x = -vehDoorOffset.x;
			break;

		case CAR_DOOR_LF:
			if (vehModel->m_vehicleType == VEHICLE_TYPE_BOAT)
				vehDoorPos = vehModel->m_positions[BOAT_POS_FRONTSEAT];
			else
				vehDoorPos = vehModel->m_positions[CAR_POS_FRONTSEAT];

			vehDoorPos.x = -(vehDoorPos.x + seatOffset);
			break;

		case CAR_DOOR_LR:
			vehDoorPos = vehModel->m_positions[CAR_POS_BACKSEAT];
			vehDoorPos.x = -(vehDoorPos.x + seatOffset);
			break;

		default:
			if (vehModel->m_vehicleType == VEHICLE_TYPE_BOAT)
				vehDoorPos = vehModel->m_positions[BOAT_POS_FRONTSEAT];
			else
				vehDoorPos = vehModel->m_positions[CAR_POS_FRONTSEAT];

			vehDoorOffset = CVector(0.0f, 0.0f, 0.0f);
	}
	return vehDoorPos - vehDoorOffset;
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

CVector
CPed::GetPositionToOpenCarDoor(CVehicle *veh, uint32 component, float offset)
{	
	CVector doorPos;
	CMatrix vehMat(veh->GetMatrix());

	doorPos = Multiply3x3(vehMat, GetLocalPositionToOpenCarDoor(veh, component, offset));

	return veh->GetPosition() + doorPos;
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

	if (!bChangedSeat && phase != LINE_UP_TO_CAR_2) {
		if (RpAnimBlendClumpGetAssociation(GetClump(), ANIM_CAR_SIT)) {
			SetPedPositionInCar();
			return;
		}
		if (RpAnimBlendClumpGetAssociation(GetClump(), ANIM_CAR_LSIT)) {
			SetPedPositionInCar();
			return;
		}
		if (RpAnimBlendClumpGetAssociation(GetClump(), ANIM_CAR_SITP)) {
			SetPedPositionInCar();
			return;
		}
		if (RpAnimBlendClumpGetAssociation(GetClump(), ANIM_CAR_SITPLO)) {
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

	if (m_vehEnterType == CAR_DOOR_RF || m_vehEnterType == CAR_DOOR_RR) {
		if (vehIsUpsideDown) {
			m_fRotationDest = -PI + veh->GetForward().Heading();
		} else if (veh->bIsBus) {
			m_fRotationDest = 0.5f * PI + veh->GetForward().Heading();
		} else {
			m_fRotationDest = veh->GetForward().Heading();
		}
	} else if (m_vehEnterType == CAR_DOOR_LF || m_vehEnterType == CAR_DOOR_LR) {
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
			case ANIM_CAR_JACKED_RHS:
			case ANIM_CAR_LJACKED_RHS:
			case ANIM_CAR_JACKED_LHS:
			case ANIM_CAR_LJACKED_LHS:
			case ANIM_VAN_GETIN_L:
			case ANIM_VAN_GETIN:
#ifdef VC_PED_PORTS
				multExtractedFromAnim = true;
				zBlend = Max(m_pVehicleAnim->GetProgress() - 0.3f, 0.0f) / (1.0f - 0.3f);
				// fall through
#endif
			case ANIM_CAR_QJACKED:
			case ANIM_CAR_GETOUT_LHS:
			case ANIM_CAR_GETOUT_LOW_LHS:
			case ANIM_CAR_GETOUT_RHS:
			case ANIM_CAR_GETOUT_LOW_RHS:
#ifdef VC_PED_PORTS
				if (!multExtractedFromAnim) {
					multExtractedFromAnim = true;
					zBlend = Max(m_pVehicleAnim->GetProgress() - 0.5f, 0.0f) / (1.0f - 0.5f);
				}
				// fall through
#endif
			case ANIM_CAR_CRAWLOUT_RHS:
			case ANIM_CAR_CRAWLOUT_RHS2:
			case ANIM_VAN_GETOUT_L:
			case ANIM_VAN_GETOUT:
				seatPosMult = m_pVehicleAnim->GetProgress();
				break;
			case ANIM_CAR_GETIN_RHS:
			case ANIM_CAR_GETIN_LHS:
#ifdef VC_PED_PORTS
				if (veh && veh->IsCar() && veh->bIsBus) {
					multExtractedFromAnimBus = true;
					zBlend = Min(m_pVehicleAnim->GetProgress(), 0.5f) / 0.5f;
				}
				// fall through
#endif
			case ANIM_CAR_QJACK:
			case ANIM_CAR_GETIN_LOW_LHS:
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
		neededPos = GetPosition();
	} else {
		neededPos = GetPositionToOpenCarDoor(veh, m_vehEnterType, seatPosMult);
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
			if (m_pVehicleAnim && vehAnim != ANIM_VAN_GETIN_L && vehAnim != ANIM_VAN_CLOSE_L && vehAnim != ANIM_VAN_CLOSE && vehAnim != ANIM_VAN_GETIN) {
				neededPos.z = autoZPos.z;
				m_vecMoveSpeed = CVector(0.0f, 0.0f, 0.0f);
			} else if (neededPos.z <= currentZ && m_pVehicleAnim && vehAnim != ANIM_VAN_CLOSE_L && vehAnim != ANIM_VAN_CLOSE) {
				adjustedTimeStep = Min(m_pVehicleAnim->timeStep, 0.1f);

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
						(vehAnim == ANIM_CAR_GETIN_RHS || vehAnim == ANIM_CAR_GETIN_LOW_RHS || vehAnim == ANIM_CAR_GETIN_LHS || vehAnim == ANIM_CAR_GETIN_LOW_LHS
							|| vehAnim == ANIM_CAR_QJACK || vehAnim == ANIM_VAN_GETIN_L || vehAnim == ANIM_VAN_GETIN)) {
						adjustedTimeStep = Min(m_pVehicleAnim->timeStep, 0.1f);

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

	bool stillGettingInOut = false;
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
		vehDoorMat.GetPosition() += Multiply3x3(vehDoorMat, GetLocalPositionToOpenCarDoor(veh, m_vehEnterType, 0.0f));
		// VC couch anims are inverted, so they're fixing it here.
		GetMatrix() = vehDoorMat;
	}

}

static void
particleProduceFootDust(CPed *ped, CVector const &pos, float size, int times)
{
	switch (ped->m_nSurfaceTouched)
	{
		case SURFACE_TARMAC:
		case SURFACE_DIRT:
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
		runStopAsoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_RUN_STOP);
		
		if ( runStopAsoc == NULL )
			runStopAsoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_RUN_STOP_R);
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

	if (m_nSurfaceTouched == SURFACE_PUDDLE) {
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

			CVector particleDir = m_vecMoveSpeed * 0.75f;

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
	int right;
	for(right = min; right <= left; ){
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
CPed::BuildPedLists(void)
{
	if ((CTimer::GetFrameCounter() + (m_randomSeed % 256)) % 16) {

		for(int i = 0; i < ARRAY_SIZE(m_nearPeds); ) {
			if (m_nearPeds[i]) {
				if (m_nearPeds[i]->IsPointerValid()) {
					float distSqr = (GetPosition() - m_nearPeds[i]->GetPosition()).MagnitudeSqr2D();
					if (distSqr < 900.0f) {
						i++;
						continue;
					}
				}

				// If we arrive here, the ped we're checking isn't "near", so we should remove it.
				for (int j = i; j < ARRAY_SIZE(m_nearPeds) - 1; j++) {
					m_nearPeds[j] = m_nearPeds[j + 1];
					m_nearPeds[j + 1] = nil;
				}
				// Above loop won't work when it's 9, so we need to empty slot 9.
				m_nearPeds[9] = nil;
				m_numNearPeds--;
			} else
				i++;
		}
	} else {
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
	}
}

void
CPed::SetPedStats(ePedStats pedStat)
{
	m_pedStats = CPedStats::ms_apPedStats[pedStat];
}

// --MIAMI: Done
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
	CAnimManager::AddAnimation(GetClump(), m_animGroup, ANIM_IDLE_STANCE);

	// TODO(Miami): This is inlined CanUseTorsoWhenLooking
	bool canUseMyBody = false;
	if (m_nPedState != PED_DRIVING && m_nPedState != PED_DRAG_FROM_CAR && !bIsDucking) {
		if (m_animGroup != ASSOCGRP_SEXYWOMAN && m_animGroup != ASSOCGRP_WOMAN)
			canUseMyBody = true;
	}
	if (!canUseMyBody)
		m_pedIK.m_flags |= CPedIK::LOOKAROUND_HEAD_ONLY;

	// This is a mistake by R*, velocity is CVector, whereas m_vecAnimMoveDelta is CVector2D. 
	(*RPANIMBLENDCLUMPDATA(m_rwObject))->velocity = (CVector*) &m_vecAnimMoveDelta;

#ifdef PED_SKIN
	if(modelInfo->GetHitColModel() == nil)
		modelInfo->CreateHitColModelSkinned(GetClump());

	if (IsClumpSkinned(GetClump())) // condition isn't there in VC
		UpdateRpHAnim();
#endif
}

void
CPed::RemoveLighting(bool reset)
{
	if (!bRenderScorched) {
		CRenderer::RemoveVehiclePedLights(this, reset);
		if (reset)
			ReSetAmbientAndDirectionalColours();
	}
	SetAmbientColours();
	DeActivateDirectional();
}

bool
CPed::SetupLighting(void)
{
	ActivateDirectional();
	SetAmbientColoursForPedsCarsAndObjects();

#ifndef MASTER
	// Originally this was being called through iteration of Sectors, but putting it here is better.
	if (nDisplayDebugInfo && !IsPlayer())
		DebugRenderOnePedText();
#endif

	if (bRenderScorched) {
		WorldReplaceNormalLightsWithScorched(Scene.world, 0.1f);
	} else {
		// Note that this lightMult is only affected by LIGHT_DARKEN. If there's no LIGHT_DARKEN, it will be 1.0.
		float lightMult = CPointLights::GenerateLightsAffectingObject(&GetPosition());
		if (!bHasBlip && lightMult != 1.0f) {
			SetAmbientAndDirectionalColours(lightMult);
			return true;
		}
	}
	return false;
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
CPed::CalculateNewOrientation(void)
{
	if (CReplay::IsPlayingBack() || !IsPedInControl())
		return;

	SetHeading(m_fRotationCur);
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

// --MIAMI: Done
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

	CAnimBlendAssociation *idleAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_IDLE_STANCE);
	CAnimBlendAssociation *fightAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_FIGHT_IDLE);
	if(!fightAssoc)
		fightAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_IDLE_TIRED);

	if(!fightAssoc)
		fightAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_WEAPON_CROUCHRELOAD);

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

#ifdef VC_PED_PORTS
bool
CPed::CanPedJumpThis(CEntity *unused, CVector *damageNormal = nil)
{
	if (m_nSurfaceTouched == SURFACE_PUDDLE)
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
		if (damageNormal->z <= 0.5f) {
			forwardOffset += collPower * ourCol->spheres->radius * forwardOffset;
		} else {
			CVector invDamageNormal(-damageNormal->x, -damageNormal->y, 0.0f);
			invDamageNormal *= 1.0f / collPower;
			CVector estimatedJumpDist = invDamageNormal + collPower * invDamageNormal * ourCol->spheres->radius;
			forwardOffset = estimatedJumpDist * Min(2.0f / collPower, 4.0f);
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

bool
CPed::CanPedReturnToState(void)
{
	return m_nPedState <= PED_STATES_NO_AI && m_nPedState != PED_AIM_GUN && m_nPedState != PED_ATTACK &&
		m_nPedState != PED_FIGHT && m_nPedState != PED_STEP_AWAY && m_nPedState != PED_SNIPER_MODE && m_nPedState != PED_LOOK_ENTITY;
}

bool
CPed::CanSeeEntity(CEntity *entity, float threshold = CAN_SEE_ENTITY_ANGLE_THRESHOLD)
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

bool
CPed::IsTemporaryObjective(eObjective objective)
{
	return objective == OBJECTIVE_LEAVE_VEHICLE || objective == OBJECTIVE_SET_LEADER ||
#ifdef VC_PED_PORTS
		objective == OBJECTIVE_LEAVE_CAR_AND_DIE ||
#endif
		objective == OBJECTIVE_ENTER_CAR_AS_DRIVER || objective == OBJECTIVE_ENTER_CAR_AS_PASSENGER;
}

void
CPed::SetMoveState(eMoveState state)
{
	m_nMoveState = state;
}

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
CPed::ForceStoredObjective(eObjective objective)
{
	if (objective != OBJECTIVE_ENTER_CAR_AS_DRIVER && objective != OBJECTIVE_ENTER_CAR_AS_PASSENGER) {
		m_prevObjective = m_objective;
		return;
	}

	switch (m_objective)
	{
		case OBJECTIVE_FLEE_TILL_SAFE:
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

void
CPed::SetStoredObjective(void)
{
	if (m_objective == m_prevObjective)
		return;

	switch (m_objective)
	{
		case OBJECTIVE_FLEE_TILL_SAFE:
		case OBJECTIVE_KILL_CHAR_ON_FOOT:
		case OBJECTIVE_KILL_CHAR_ANY_MEANS:
		case OBJECTIVE_FLEE_CHAR_ON_FOOT_TILL_SAFE:
		case OBJECTIVE_FLEE_CHAR_ON_FOOT_ALWAYS:
		case OBJECTIVE_GOTO_CHAR_ON_FOOT:
		case OBJECTIVE_FOLLOW_PED_IN_FORMATION:
		case OBJECTIVE_LEAVE_VEHICLE:
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
CPed::RestorePreviousObjective(void)
{
	if (m_objective == OBJECTIVE_NONE)
		return;

	if (m_objective != OBJECTIVE_LEAVE_VEHICLE && m_objective != OBJECTIVE_ENTER_CAR_AS_PASSENGER && m_objective != OBJECTIVE_ENTER_CAR_AS_DRIVER
#ifdef VC_PED_PORTS
		&& m_nPedState != PED_CARJACK
#endif
		)
		m_pedInObjective = nil;

	if (m_objective == OBJECTIVE_WAIT_IN_CAR_THEN_GETOUT) {
		m_objective = OBJECTIVE_NONE;
		if (m_pMyVehicle)
			SetObjective(OBJECTIVE_LEAVE_VEHICLE, m_pMyVehicle);

	} else {
		m_objective = m_prevObjective;
		m_prevObjective = OBJECTIVE_NONE;
	}
	bObjectiveCompleted = false;
}

void
CPed::SetLeader(CEntity *leader)
{
	m_leader = (CPed*)leader;

	if(m_leader)
		m_leader->RegisterReference((CEntity **)&m_leader);
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
			case OBJECTIVE_FOLLOW_PED_IN_FORMATION:
			case OBJECTIVE_GOTO_AREA_ANY_MEANS:
			case OBJECTIVE_FIGHT_CHAR:
				if (m_pedInObjective == entity)
					return;

				break;
			case OBJECTIVE_LEAVE_VEHICLE:
			case OBJECTIVE_FLEE_CAR:
#ifdef VC_PED_PORTS
			case OBJECTIVE_LEAVE_CAR_AND_DIE:
#endif
				return;
			case OBJECTIVE_ENTER_CAR_AS_PASSENGER:
			case OBJECTIVE_ENTER_CAR_AS_DRIVER:
			case OBJECTIVE_DESTROY_CAR:
			case OBJECTIVE_SOLICIT:
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
		if ((newObj == OBJECTIVE_LEAVE_VEHICLE
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
	if (!IsTemporaryObjective(m_objective) || IsTemporaryObjective(newObj)) {
		if (m_objective != newObj) {
			if (IsTemporaryObjective(newObj))
				ForceStoredObjective(newObj);
			else
				SetStoredObjective();
		}
		m_objective = newObj;
	} else {
		m_prevObjective = newObj;
	}

	switch (newObj) {
		case OBJECTIVE_WAIT_IN_CAR_THEN_GETOUT:

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
		case OBJECTIVE_FIGHT_CHAR:
			m_vecSeekPos = CVector(0.0f, 0.0f, 0.0f);
			m_pedInObjective = (CPed*)entity;
			m_pedInObjective->RegisterReference((CEntity**)&m_pedInObjective);
			break;
		case OBJECTIVE_FOLLOW_PED_IN_FORMATION:
			m_pedInObjective = (CPed*)entity;
			m_pedInObjective->RegisterReference((CEntity**)&m_pedInObjective);
			m_pedFormation = FORMATION_REAR;
			break;
		case OBJECTIVE_LEAVE_VEHICLE:
#ifdef VC_PED_PORTS
		case OBJECTIVE_LEAVE_CAR_AND_DIE:
#endif
		case OBJECTIVE_FLEE_CAR:
			m_carInObjective = (CVehicle*)entity;
			m_carInObjective->RegisterReference((CEntity **)&m_carInObjective);
			if (!m_carInObjective->bIsBus || m_leaveCarTimer)
				break;

			for (int i = 0; i < m_carInObjective->m_nNumMaxPassengers; i++) {
				if (m_carInObjective->pPassengers[i] == this) {
					m_leaveCarTimer = CTimer::GetTimeInMilliseconds() + 1200 * i;
					break;
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
		case OBJECTIVE_SOLICIT:
		case OBJECTIVE_BUY_ICE_CREAM:
			m_carInObjective = (CVehicle*)entity;
			m_carInObjective->RegisterReference((CEntity**)&m_carInObjective);
			m_pSeekTarget = m_carInObjective;
			m_pSeekTarget->RegisterReference((CEntity**)&m_pSeekTarget);
			m_vecSeekPos = CVector(0.0f, 0.0f, 0.0f);
			if (newObj == OBJECTIVE_SOLICIT) {
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
CPed::SetIdle(void)
{
	if (m_nPedState != PED_IDLE && m_nPedState != PED_MUG && m_nPedState != PED_FLEE_ENTITY) {
#ifdef VC_PED_PORTS
		if (m_nPedState == PED_AIM_GUN)
			ClearPointGunAt();

		m_nLastPedState = PED_NONE;
#endif
		m_nPedState = PED_IDLE;
		SetMoveState(PEDMOVE_STILL);
	}
	if (m_nWaitState == WAITSTATE_FALSE) {
		m_nWaitTimer = CTimer::GetTimeInMilliseconds() + CGeneral::GetRandomNumberInRange(2000, 4000);
	}
}

void
CPed::SetObjective(eObjective newObj)
{
	if (DyingOrDead())
		return;

	if (newObj == OBJECTIVE_NONE) {
		if ((m_objective == OBJECTIVE_LEAVE_VEHICLE || m_objective == OBJECTIVE_ENTER_CAR_AS_DRIVER || m_objective == OBJECTIVE_ENTER_CAR_AS_PASSENGER
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
		m_objective = newObj;
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
CPed::ClearChat(void)
{
	CAnimBlendAssociation *animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_IDLE_CHAT);
	if (animAssoc) {
		animAssoc->blendDelta = -8.0f;
		animAssoc->flags |= ASSOC_DELETEFADEDOUT;
	}
	bIsTalking = false;
	ClearLookFlag();
	RestorePreviousState();
}

bool
CPed::IsGangMember(void)
{
	return m_nPedType >= PEDTYPE_GANG1 && m_nPedType <= PEDTYPE_GANG9;
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
CPed::QuitEnteringCar(void)
{
	CAnimBlendAssociation *animAssoc = m_pVehicleAnim;
	CVehicle *veh = m_pMyVehicle;
	if (animAssoc)
		animAssoc->blendDelta = -1000.0f;
	
	RestartNonPartialAnims();

	if (!RpAnimBlendClumpGetAssociation(GetClump(), ANIM_IDLE_STANCE))
		CAnimManager::BlendAnimation(GetClump(), m_animGroup, ANIM_IDLE_STANCE, 100.0f);
	
	if (veh) {
		if (m_objective == OBJECTIVE_ENTER_CAR_AS_DRIVER || m_nPedState == PED_CARJACK)
			veh->bIsBeingCarJacked = false;

		if (veh->m_nNumGettingIn != 0)
			veh->m_nNumGettingIn--;

#ifdef VC_PED_PORTS
		if (m_objective == OBJECTIVE_ENTER_CAR_AS_DRIVER || m_objective == OBJECTIVE_ENTER_CAR_AS_PASSENGER)
			RestorePreviousObjective();
#endif

		veh->m_nGettingInFlags &= ~GetCarDoorFlag(m_vehEnterType);
	}

	bUsesCollision = true;

	ReplaceWeaponWhenExitingVehicle();

	if (DyingOrDead()) {
		animAssoc = m_pVehicleAnim;
		if (animAssoc) {
			animAssoc->blendDelta = -4.0f;
			animAssoc->flags |= ASSOC_DELETEFADEDOUT;
			animAssoc->flags &= ~ASSOC_RUNNING;
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
		&& (m_pMyVehicle->pDriver == this || m_pMyVehicle->pDriver && m_pMyVehicle->pDriver->m_nPedState == PED_DRIVING)) {

		if (m_pMyVehicle->VehicleCreatedBy == RANDOM_VEHICLE
			&& (m_pMyVehicle->GetStatus() == STATUS_SIMPLE || m_pMyVehicle->GetStatus() == STATUS_PHYSICS)
			&& m_pMyVehicle->AutoPilot.m_nCarMission == MISSION_CRUISE) {

			CCarCtrl::SwitchVehicleToRealPhysics(m_pMyVehicle);
			m_pMyVehicle->AutoPilot.m_nDrivingStyle = DRIVINGSTYLE_AVOID_CARS;
			m_pMyVehicle->AutoPilot.m_nCruiseSpeed = GAME_SPEED_TO_CARAI_SPEED * m_pMyVehicle->pHandling->Transmission.fUnkMaxVelocity;
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
				SetDuck(CGeneral::GetRandomNumberInRange(1000,3000));
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

bool
CPed::TurnBody(void)
{
	float lookDir;
	bool turnDone = true;

	if (m_pLookTarget) {
		const CVector &lookPos = m_pLookTarget->GetPosition();

		lookDir = CGeneral::GetRadianAngleBetweenPoints(
					lookPos.x,
					lookPos.y,
					GetPosition().x,
					GetPosition().y);
	} else
		lookDir = m_fLookDirection;

	float limitedLookDir = CGeneral::LimitRadianAngle(lookDir);
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
			CAnimBlendAssociation *chatAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_IDLE_CHAT);
			if (chatAssoc) {
				chatAssoc->blendDelta = -4.0f;
				chatAssoc->flags |= ASSOC_DELETEFADEDOUT;
			}
			bIsTalking = false;
		} else
			Say(SOUND_PED_CHAT);

	} else if (!RpAnimBlendClumpGetFirstAssociation(GetClump(), ASSOC_FLAG_XPRESS)) {

		if (CGeneral::GetRandomNumber() < 20) {
			CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_XPRESS_SCRATCH, 4.0f);
		}
		if (!bIsTalking) {
			CAnimBlendAssociation *chatAssoc = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_IDLE_CHAT, 4.0f);
			float chatTime = CGeneral::GetRandomNumberInRange(0.0f, 3.0f);
			chatAssoc->SetCurrentTime(chatTime);

			bIsTalking = true;
			Say(SOUND_PED_CHAT);
		}
	}
	if (m_standardTimer && CTimer::GetTimeInMilliseconds() > m_standardTimer) {
		ClearChat();
		m_standardTimer = CTimer::GetTimeInMilliseconds() + 30000;
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

bool
CPed::MakePhonecall(void)
{
#ifdef TOGGLEABLE_BETA_FEATURES
	if (bMakePedsRunToPhonesToReportCrimes)
		if (!IsPlayer() && CTimer::GetTimeInMilliseconds() > m_phoneTalkTimer - 7000 && bRunningToPhone) {

			FindPlayerPed()->m_pWanted->RegisterCrime_Immediately(m_crimeToReportOnPhone, GetPosition(),
				(m_crimeToReportOnPhone == CRIME_POSSESSION_GUN ? (uintptr)m_threatEntity : (uintptr)((CPed*)m_pEventEntity)->m_threatEntity), false);
			bRunningToPhone = false;
		}
#endif
	if (CTimer::GetTimeInMilliseconds() <= m_phoneTalkTimer)
		return false;

	SetIdle();
	gPhoneInfo.m_aPhones[m_phoneId].m_nState = PHONE_STATE_FREE;
#ifdef TOGGLEABLE_BETA_FEATURES
	crimeReporters[m_phoneId] = nil;
#endif
	m_phoneId = -1;
	return true;
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

PointBlankNecessity
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

				PedState nearPedState = nearPed->m_nPedState;

				if (nearPedState == PED_FALL || nearPedState == PED_GETUP || nearPedState == PED_DIE || nearPedState == PED_DEAD || nearPedState == PED_DIVE_AWAY)
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
CPed::ClearAll(void)
{
	if (!IsPedInControl() && m_nPedState != PED_DEAD)
		return;

	m_nPedState = PED_NONE;
	m_nMoveState = PEDMOVE_NONE;
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

// --MIAMI: Done
void
CPed::ClearAttackByRemovingAnim(void)
{
	if (m_nPedState != PED_ATTACK)
		return;

	CWeaponInfo *weapon = CWeaponInfo::GetWeaponInfo(GetWeapon()->m_eWeaponType);
	CAnimBlendAssociation *weaponAssoc = RpAnimBlendClumpGetAssociation(GetClump(), GetPrimaryFireAnim(weapon));

	if (!weaponAssoc) {
		if (!!weapon->m_bCrouchFire)
			weaponAssoc = RpAnimBlendClumpGetAssociation(GetClump(), GetCrouchFireAnim(weapon));
	}
	if (!weaponAssoc) {
		if(!!weapon->m_bFinish3rd)
			weaponAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_WEAPON_SPECIAL);
	}
	if (!weaponAssoc) {
		if(!!weapon->m_bUse2nd)
			weaponAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_WEAPON_CROUCHFIRE);
	}
	if (!weaponAssoc) {
		if(!!weapon->m_bCop3rd)
			weaponAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_WEAPON_SPECIAL);
	}
	if (weaponAssoc) {
		weaponAssoc->blendDelta = -8.0f;
		weaponAssoc->flags &= ~ASSOC_RUNNING;
		weaponAssoc->flags |= ASSOC_DELETEFADEDOUT;
		weaponAssoc->SetDeleteCallback(FinishedAttackCB, this);
	} else {
		ClearAttack();
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
CPed::SetStoredState(void)
{
	if (m_nLastPedState != PED_NONE || !CanPedReturnToState())
		return;

	if (m_nPedState == PED_WANDER_PATH) {
		bFindNewNodeAfterStateRestore = true;
		if (m_nMoveState == PEDMOVE_NONE || m_nMoveState == PEDMOVE_STILL)
			m_nMoveState = PEDMOVE_WALK;
	}
	m_nLastPedState = m_nPedState;
	if (m_nMoveState >= m_nPrevMoveState)
		m_nPrevMoveState = m_nMoveState;
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

	m_nPedState = PED_DIE;
	if (animId == NUM_STD_ANIMS) {
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

// --MIAMI: Done except commented things
bool
CPed::InflictDamage(CEntity *damagedBy, eWeaponType method, float damage, ePedPieceTypes pedPiece, uint8 direction)
{
	CPlayerPed *player = FindPlayerPed();
	float dieDelta = 4.0f;
	float dieSpeed = 0.0f;
	AnimationId dieAnim = ANIM_KO_SHOT_FRONT1;
	bool headShot = false;
	bool willLinger = false;
	int random;

	// TODO(Miami): PlayerInfo thingies here

	if (player == this) {
		if (!player->m_bCanBeDamaged)
			return false;

		if (damagedBy && damagedBy->IsPed() && ((CPed*)damagedBy)->m_nPedType == PEDTYPE_GANG7)
			return false;

		if ((method == WEAPONTYPE_FLAMETHROWER || method == WEAPONTYPE_MOLOTOV) && CWorld::Players[CWorld::PlayerInFocus].m_bFireproof)
			return false;

		player->AnnoyPlayerPed(false);
	}

	if (DyingOrDead())
		return false;

	if (method == WEAPONTYPE_DROWNING && !bDrownsInWater)
		return false;

	if (!bUsesCollision && (!bInVehicle || m_nPedState != PED_DRIVING) && method != WEAPONTYPE_DROWNING)
		return false;

	if (bOnlyDamagedByPlayer && damagedBy != player && damagedBy != FindPlayerVehicle() &&
		method != WEAPONTYPE_DROWNING && method != WEAPONTYPE_EXPLOSION)
		return false;

	float healthImpact;
	if (IsPlayer())
		healthImpact = damage * 0.33f;
	else
		healthImpact = damage * m_pedStats->m_defendWeakness;

	if (!IsPlayer() &&
		(method == WEAPONTYPE_SCREWDRIVER || method == WEAPONTYPE_KNIFE || (method >= WEAPONTYPE_CLEAVER && method <= WEAPONTYPE_CHAINSAW)))
		m_bleedCounter = 200;

	bool detectDieAnim = true;
	if (m_nPedState == PED_GETUP) {
		if (!IsPedHeadAbovePos(-0.3f)) {
			if (RpAnimBlendClumpGetFirstAssociation(GetClump(), ASSOC_FRONTAL))
				dieAnim = ANIM_FLOOR_HIT_F;
			else
				dieAnim = ANIM_FLOOR_HIT;
			dieDelta *= 2.0f;
			dieSpeed = 0.5f;
			detectDieAnim = false;
		}
	} else if (m_nPedState == PED_FALL) {
		CAnimBlendAssociation *fallAssoc = RpAnimBlendClumpGetFirstAssociation(GetClump(), ASSOC_PARTIAL);
		if (!fallAssoc || fallAssoc->IsRunning()) {
			if (fallAssoc && fallAssoc->blendDelta >= 0.0f)
				dieAnim = NUM_STD_ANIMS;
			else
				dieAnim = ANIM_KO_SHOT_FRONT1;
		} else {
			if (fallAssoc->flags & ASSOC_FRONTAL)
				dieAnim = ANIM_FLOOR_HIT_F;
			else
				dieAnim = ANIM_FLOOR_HIT;

			dieDelta *= 2.0f;
			dieSpeed = 0.5f;
		}
		detectDieAnim = false;
	}

	if (detectDieAnim) {
		switch (method) {
			case WEAPONTYPE_UNARMED:
			case WEAPONTYPE_BRASSKNUCKLE:
				if (bMeleeProof)
					return false;

				if (m_nPedState == PED_FALL) {
					if (IsPedHeadAbovePos(-0.3f)) {
						dieAnim = NUM_STD_ANIMS;
					} else {
						if (RpAnimBlendClumpGetFirstAssociation(GetClump(), ASSOC_FRONTAL))
							dieAnim = ANIM_FLOOR_HIT_F;
						else
							dieAnim = ANIM_FLOOR_HIT;
						dieDelta = dieDelta * 2.0f;
						dieSpeed = 0.5f;
					}
				} else {
					switch (direction) {
						case 0:
							dieAnim = ANIM_KO_SKID_FRONT;
							break;
						case 1:
							dieAnim = ANIM_KO_SPIN_R;
							break;
						case 2:
							dieAnim = ANIM_KO_SKID_BACK;
							break;
						case 3:
							dieAnim = ANIM_KO_SPIN_L;
							break;
						default:
							break;
					}
				}
				break;
			case WEAPONTYPE_SCREWDRIVER:
			case WEAPONTYPE_GOLFCLUB:
			case WEAPONTYPE_NIGHTSTICK:
			case WEAPONTYPE_KNIFE:
			case WEAPONTYPE_BASEBALLBAT:
			case WEAPONTYPE_HAMMER:
			case WEAPONTYPE_CLEAVER:
			case WEAPONTYPE_MACHETE:
			case WEAPONTYPE_KATANA:
			case WEAPONTYPE_CHAINSAW:
				if (bMeleeProof)
					return false;

				if (method != WEAPONTYPE_KATANA ||
					damagedBy != FindPlayerPed()
					|| FindPlayerPed()->m_nPedState != PED_FIGHT
					|| FindPlayerPed()->m_lastFightMove != FIGHTMOVE_MELEE1 && FindPlayerPed()->m_lastFightMove != FIGHTMOVE_MELEE2
					|| CGeneral::GetRandomNumber() & 3) {

					if (m_nPedState == PED_FALL) {
						if (IsPedHeadAbovePos(-0.3f)) {
							dieAnim = NUM_STD_ANIMS;
						} else {
							if (RpAnimBlendClumpGetFirstAssociation(GetClump(), ASSOC_FRONTAL))
								dieAnim = ANIM_FLOOR_HIT_F;
							else
								dieAnim = ANIM_FLOOR_HIT;
							dieDelta = dieDelta * 2.0f;
							dieSpeed = 0.5f;
						}
					} else if (damagedBy != FindPlayerPed() || FindPlayerPed()->m_lastFightMove != FIGHTMOVE_MELEE2) {
						if (damagedBy != FindPlayerPed() || FindPlayerPed()->m_lastFightMove != FIGHTMOVE_MELEE3) {
							switch (direction) {
								case 0:
									dieAnim = ANIM_KO_SKID_FRONT;
									break;
								case 1:
									dieAnim = ANIM_KO_SPIN_R;
									break;
								case 2:
									dieAnim = ANIM_KO_SKID_BACK;
									break;
								case 3:
									dieAnim = ANIM_KO_SPIN_L;
									break;
								default:
									break;
							}
						} else {
							dieAnim = ANIM_KO_SHOT_STOM;
						}
					} else {
						dieAnim = ANIM_KO_SHOT_FACE;
					}
				} else {
					dieAnim = ANIM_KO_SHOT_FACE;
					RemoveBodyPart(PED_HEAD, direction);
					headShot = true;
					willLinger = true;
				}
				break;
			case WEAPONTYPE_COLT45:
			case WEAPONTYPE_SHOTGUN:
				// TODO(Miami): Shotguns
			case WEAPONTYPE_TEC9:
			case WEAPONTYPE_UZI:
			case WEAPONTYPE_SILENCED_INGRAM:
			case WEAPONTYPE_MP5:
			case WEAPONTYPE_M16:
			case WEAPONTYPE_AK47:
			case WEAPONTYPE_SNIPERRIFLE:
			case WEAPONTYPE_UZI_DRIVEBY:
				// TODO(Miami): Laserscope, M60, Minigun

				if (bBulletProof)
					return false;

				bool dontRemoveLimb;
				if (IsPlayer() || bNoCriticalHits)
					dontRemoveLimb = true;
				else if (method != WEAPONTYPE_M16 && method != WEAPONTYPE_AK47 && method != WEAPONTYPE_SNIPERRIFLE
						/* method != WEAPONTYPE_LASERSCOPE */) { // TODO(Miami): Laserscope
					if (method == WEAPONTYPE_SHOTGUN)
						dontRemoveLimb = CGeneral::GetRandomNumber() & 7;
					else
						dontRemoveLimb = CGeneral::GetRandomNumber() & 15;
				} else
					dontRemoveLimb = false;

				if (dontRemoveLimb) {
					if (method == WEAPONTYPE_SHOTGUN) {
						switch (direction) {
							case 0:
								dieAnim = ANIM_KO_SKID_FRONT;
								break;
							case 1:
								dieAnim = ANIM_KO_SPIN_R;
								break;
							case 2:
								dieAnim = ANIM_KO_SKID_BACK;
								break;
							case 3:
								dieAnim = ANIM_KO_SPIN_L;
								break;
							default:
								break;
						}
					} else
						dieAnim = ANIM_KO_SHOT_FRONT1;

					willLinger = false;
				} else {
					switch (pedPiece) {
						case PEDPIECE_TORSO:
							willLinger = false;
							dieAnim = ANIM_KO_SHOT_FRONT1;
							break;
						case PEDPIECE_MID:
							willLinger = false;
							dieAnim = ANIM_KO_SHOT_STOM;
							break;
						case PEDPIECE_LEFTARM:
							dieAnim = ANIM_KO_SHOT_ARML;
							RemoveBodyPart(PED_UPPERARML, direction);
							willLinger = true;
							break;
						case PEDPIECE_RIGHTARM:
							dieAnim = ANIM_KO_SHOT_ARMR;
							RemoveBodyPart(PED_UPPERARMR, direction);
							willLinger = true;
							break;
						case PEDPIECE_LEFTLEG:
							dieAnim = ANIM_KO_SHOT_LEGL;
							RemoveBodyPart(PED_UPPERLEGL, direction);
							willLinger = true;
							break;
						case PEDPIECE_RIGHTLEG:
							dieAnim = ANIM_KO_SHOT_LEGR;
							RemoveBodyPart(PED_UPPERLEGR, direction);
							willLinger = true;
							break;
						case PEDPIECE_HEAD:
							dieAnim = ANIM_KO_SHOT_FACE;
							RemoveBodyPart(PED_HEAD, direction);
							headShot = true;
							willLinger = true;
							break;
						default:
							break;
					}
				}
				break;
			case WEAPONTYPE_GRENADE:
			case WEAPONTYPE_ROCKETLAUNCHER:
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
						dieAnim = ANIM_KO_SKID_FRONT;
						break;
					case 1:
						dieAnim = ANIM_KO_SPIN_R;
						break;
					case 2:
						dieAnim = ANIM_KO_SKID_BACK;
						break;
					case 3:
						dieAnim = ANIM_KO_SPIN_L;
						break;
					default:
						break;
				}
				break;
			case WEAPONTYPE_FLAMETHROWER:
				if (bFireProof)
					return false;

				dieAnim = ANIM_KO_SHOT_FRONT1;
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
						
								dieAnim = ANIM_KO_SPIN_L;
							else
								dieAnim = ANIM_KO_SKID_FRONT;
						} else
							dieAnim = ANIM_KO_SPIN_R;

						break;
					case 1:
						if (m_nPedState == PED_DIVE_AWAY)
							dieAnim = ANIM_KD_LEFT;
						else
							dieAnim = ANIM_KO_SPIN_R;
						break;
					case 2:
						if ((pedPiece != PEDPIECE_LEFTARM || random <= 1)
							&& (pedPiece != PEDPIECE_MID || random != 1)) {
							if ((pedPiece != PEDPIECE_RIGHTARM || random <= 1)
								&& (pedPiece != PEDPIECE_MID || random != 2)) {
								dieAnim = ANIM_KO_SKID_BACK;
							} else {
								dieAnim = ANIM_KD_RIGHT;
							}
						} else
							dieAnim = ANIM_KD_LEFT;
						break;
					case 3:
						if (m_nPedState == PED_DIVE_AWAY)
							dieAnim = ANIM_KD_RIGHT;
						else
							dieAnim = ANIM_KO_SPIN_L;
						break;
					default:
						break;
				}
				if (damagedBy && pedPiece != PEDPIECE_TORSO) {
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
				dieAnim = ANIM_DROWN;
				break;
			case WEAPONTYPE_FALL:
				if (bCollisionProof)
					return false;

				switch (direction) {
					case 0:
						dieAnim = ANIM_KO_SKID_FRONT;
						break;
					case 1:
						dieAnim = ANIM_KO_SPIN_R;
						break;
					case 2:
						dieAnim = ANIM_KO_SKID_BACK;
						break;
					case 3:
						dieAnim = ANIM_KO_SPIN_L;
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
		m_lastDamEntity = damagedBy;
	}

	if (method == WEAPONTYPE_FALL) {
		if (RpAnimBlendClumpGetAssociation(GetClump(), ANIM_CAR_ROLLOUT_LHS)) {
			if (m_fHealth >= 1.0 && m_fHealth - healthImpact < 5.0f) {
				m_fHealth = Min(m_fHealth, 5.0f);
				return false;
			}
		}
	}

	if (m_fHealth - healthImpact >= 1.0f && !willLinger) {
		m_fHealth -= healthImpact;
		return false;
	}

	if (bInVehicle) {
		if (method != WEAPONTYPE_DROWNING) {
#ifdef VC_PED_PORTS
			if (m_pMyVehicle) {

				// TODO(Miami): Bikes
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
						// TODO(Miami): PlayerInfo stuff
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

		SetDie(NUM_STD_ANIMS, 4.0f, 0.0f);
		return true;
	} else {
		m_fHealth = 0.0f;
		SetDie(dieAnim, dieDelta, dieSpeed);

		if (damagedBy == player || damagedBy && damagedBy == FindPlayerVehicle()) {

			// TODO(Miami): PlayerInfo stuff
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

void
CPed::ClearFlee(void)
{
	RestorePreviousState();
	bUsePedNodeSeek = false;
	m_standardTimer = 0;
	m_fleeTimer = 0;
}

void
CPed::ClearFall(void)
{
	SetGetUp();
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
			if (m_nPedState == PED_FOLLOW_PATH)
				ClearFollowPath();

			m_nPedState = PED_GETUP;
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
		CAnimBlendAssociation *animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_SPRINT);
		if (animAssoc) {
			if (RpAnimBlendClumpGetAssociation(GetClump(), ANIM_RUN)) {
				CAnimManager::BlendAnimation(GetClump(), m_animGroup, ANIM_RUN, 8.0f);
			} else {
				CAnimManager::BlendAnimation(GetClump(), m_animGroup, ANIM_IDLE_STANCE, 8.0f);
			}
			animAssoc->flags |= ASSOC_DELETEFADEDOUT;
		}

		if (RpAnimBlendClumpGetFirstAssociation(GetClump(), ASSOC_FRONTAL))
			animAssoc = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_GETUP_FRONT, 1000.0f);
		else
			animAssoc = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_GETUP1, 1000.0f);

		animAssoc->SetFinishCallback(PedGetupCB,this);
	} else {
		m_fHealth = 0.0f;
		SetDie(NUM_STD_ANIMS, 4.0f, 0.0f);
	}
}

void
CPed::ClearInvestigateEvent(void)
{
	CAnimBlendAssociation *animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_ROAD_CROSS);
	if (!animAssoc)
		animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_XPRESS_SCRATCH);
	if (!animAssoc)
		animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_IDLE_HBHB);
	if (!animAssoc)
		animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_IDLE_CHAT);
	if (animAssoc) {
		animAssoc->blendDelta = -8.0f;
		animAssoc->flags |= ASSOC_DELETEFADEDOUT;
	}
	if (m_eventType > EVENT_EXPLOSION)
		m_standardTimer = CTimer::GetTimeInMilliseconds() + 15000;

	bGonnaInvestigateEvent = false;
	m_pEventEntity = nil;
	ClearLookFlag();
	RestorePreviousState();
	if(m_nMoveState == PEDMOVE_NONE || m_nMoveState == PEDMOVE_STILL)
		SetMoveState(PEDMOVE_WALK);
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
CPed::ClearLook(void)
{
	RestorePreviousState();
	ClearLookFlag();
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
#ifdef VC_PED_PORTS
				if(!IsPlayer())
#endif
					bWanderPathAfterExitingCar = true;

				SetObjective(OBJECTIVE_LEAVE_VEHICLE, m_pMyVehicle);
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
CPed::ClearPause(void)
{
	RestorePreviousState();
}

void
CPed::ClearSeek(void)
{
	SetIdle();
	bRunningToPhone = false;
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
			m_nPedState = PED_WANDER_PATH;
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

// --MIAMI: Done
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
CPed::RestorePreviousState(void)
{
	if(!CanSetPedState() || m_nPedState == PED_FALL)
		return;

	if (m_nPedState == PED_GETUP && !bGetUpAnimStarted)
		return;

	if (InVehicle()) {
		m_nPedState = PED_DRIVING;
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
				m_nPedState = PED_WANDER_PATH;
				bIsRunning = false;
				if (!bFindNewNodeAfterStateRestore) {
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
				m_nPedState = m_nLastPedState;
				SetMoveState((eMoveState) m_nPrevMoveState);
				break;
		}
		m_nLastPedState = PED_NONE;
	}
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
CPed::SetAimFlag(float angle)
{
	bIsAimingGun = true;
	bIsRestoringGun = false;
	m_fLookDirection = angle;
	m_lookTimer = 0;
	m_pLookTarget = nil;
	m_pSeekTarget = nil;
	if (CWeaponInfo::GetWeaponInfo(GetWeapon()->m_eWeaponType)->m_bCanAimWithArm)
		m_pedIK.m_flags |= CPedIK::AIMS_WITH_ARM;
	else
		m_pedIK.m_flags &= ~CPedIK::AIMS_WITH_ARM;
}

// --MIAMI: Done
void
CPed::SetPointGunAt(CEntity *to)
{
	if (to) {
		SetLookFlag(to, true);
		SetAimFlag(to);
		SetLookTimer(INT32_MAX);
	}

	CWeaponInfo* curWeapon = CWeaponInfo::GetWeaponInfo(GetWeapon()->m_eWeaponType);
	if (m_nPedState == PED_AIM_GUN || (bIsDucking && !IsPlayer()) || m_nWaitState == WAITSTATE_PLAYANIM_DUCK || curWeapon->m_AnimToPlay == ASSOCGRP_STD)
		return;

	if (m_nPedState != PED_ATTACK)
		SetStoredState();

	if (m_nPedState == PED_FOLLOW_PATH)
		ClearFollowPath();

	m_nPedState = PED_AIM_GUN;
	bIsPointingGunAt = true;
	SetMoveState(PEDMOVE_NONE);

	CAnimBlendAssociation *aimAssoc;

	if (bCrouchWhenShooting && bIsDucking) {
		if (!!curWeapon->m_bCrouchFire) {
			aimAssoc = RpAnimBlendClumpGetAssociation(GetClump(), GetCrouchFireAnim(curWeapon));
		}
	} else {
		aimAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_WEAPON_FIRE);
	}

	if (!aimAssoc || aimAssoc->blendDelta < 0.0f) {
		if (bCrouchWhenShooting && bIsDucking) {
			if (!!curWeapon->m_bCrouchFire) {
				aimAssoc = CAnimManager::BlendAnimation(GetClump(), curWeapon->m_AnimToPlay, GetCrouchFireAnim(curWeapon), 4.0f);
			}
		} else {
			aimAssoc = CAnimManager::AddAnimation(GetClump(), curWeapon->m_AnimToPlay, ANIM_WEAPON_FIRE);
		}

		aimAssoc->blendAmount = 0.0f;
		aimAssoc->blendDelta = 8.0f;
	}
	if (to && !IsPlayer())
		Say(SOUND_PED_ATTACK);
}

// --MIAMI: Done
void
CPed::SetAmmo(eWeaponType weaponType, uint32 ammo)
{
	int slot = CWeaponInfo::GetWeaponInfo(weaponType)->m_nWeaponSlot;
	if (slot == -1)
		return;

	GetWeapon(slot).m_nAmmoTotal = ammo;
	if (weaponType < WEAPONTYPE_LAST_WEAPONTYPE && weaponType > WEAPONTYPE_UNARMED && CWeaponInfo::ms_aMaxAmmoForWeapon[weaponType] >= 0) {

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

// --MIAMI: Done
void
CPed::GrantAmmo(eWeaponType weaponType, uint32 ammo)
{
	int slot = CWeaponInfo::GetWeaponInfo(weaponType)->m_nWeaponSlot;
	if (slot == -1)
		return;

	GetWeapon(slot).m_nAmmoTotal += ammo;
	if (weaponType < WEAPONTYPE_LAST_WEAPONTYPE && weaponType > WEAPONTYPE_UNARMED && CWeaponInfo::ms_aMaxAmmoForWeapon[weaponType] >= 0) {

		// Looks like abandoned idea. This block never runs, ms_aMaxAmmoForWeapon is always -1.
		GetWeapon(slot).m_nAmmoTotal = Min(CWeaponInfo::ms_aMaxAmmoForWeapon[weaponType], GetWeapon(slot).m_nAmmoTotal);
	} else {
		GetWeapon(slot).m_nAmmoTotal = Min(99999, GetWeapon(slot).m_nAmmoTotal);
	}

	if (GetWeapon(slot).m_eWeaponState == WEAPONSTATE_OUT_OF_AMMO && GetWeapon(slot).m_nAmmoTotal > 0)
		GetWeapon(slot).m_eWeaponState = WEAPONSTATE_READY;
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
		neededTurn = 2 * PI - neededTurn;

	CVehicle *veh = (CVehicle*)reason;
	if (reason->IsVehicle() && veh->m_vehType == VEHICLE_TYPE_CAR) {
		if (veh->m_nCarHornTimer) {
			vehPressedHorn = true;
			if (!IsPlayer())
				animType = 1;
		}
	}
	if (neededTurn <= DEGTORAD(90.0f) || veh->GetModelIndex() == MI_RCBANDIT || vehPressedHorn || animType != 0) {
		SetLookFlag(veh, true);
		if ((CGeneral::GetRandomNumber() & 1) && veh->GetModelIndex() != MI_RCBANDIT && animType == 0) {
			stepAnim = ANIM_IDLE_TAXI;
		} else {

			float vehDirection = CGeneral::GetRadianAngleBetweenPoints(
				veh->m_vecMoveSpeed.x, veh->m_vecMoveSpeed.y,
				0.0f, 0.0f);

			// Let's turn our back to the "reason"
			angleToFace += PI;

			if (angleToFace > PI)
				angleToFace -=  2*PI;

			// We don't want to run towards car's direction
			float dangerZone = angleToFace - vehDirection;
			dangerZone = CGeneral::LimitRadianAngle(dangerZone);

			// So, add or subtract 90deg (jump to left/right) according to that
			if (dangerZone <= 0.0f)
				angleToFace = 0.5f*PI + vehDirection;
			else
				angleToFace = vehDirection - 0.5f*PI;

			if (animType == 2)
				stepAnim = ANIM_HANDSCOWER;
			else if (animType < 2)
				stepAnim = ANIM_EV_STEP;
			else
				stepAnim = NUM_STD_ANIMS;
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
			m_nPedState = PED_STEP_AWAY;
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
	if (reason->IsVehicle() && veh->m_vehType == VEHICLE_TYPE_CAR && veh->m_nCarHornTimer && !IsPlayer()) {
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
		animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_HANDSUP);
		if (animAssoc)
			return;

		animAssoc = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_HANDSUP, 8.0f);
		animAssoc->flags &= ~ASSOC_DELETEFADEDOUT;
		animAssoc->SetFinishCallback(PedEvadeCB, this);
		SetStoredState();
		m_nPedState = PED_STEP_AWAY;
	} else {
		m_fRotationCur = angleToFace;
		ClearLookFlag();
		ClearAimFlag();
		SetStoredState();
		m_nPedState = PED_DIVE_AWAY;
		animAssoc = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_EV_DIVE, 8.0f);
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

// --MIAMI: Done
void
CPed::SetAttack(CEntity *victim)
{
	CPed *victimPed = nil;
	CWeaponInfo *curWeapon = CWeaponInfo::GetWeaponInfo(GetWeapon()->m_eWeaponType);
	CAnimBlendAssociation *animAssoc;

	if (victim && victim->IsPed())
		victimPed = (CPed*)victim;

	if (m_attackTimer > CTimer::GetTimeInMilliseconds() || m_nWaitState == WAITSTATE_SURPRISE || (bIsDucking && !bCrouchWhenShooting))
		return;

	if (curWeapon->m_bReload &&
		(RpAnimBlendClumpGetAssociation(GetClump(), GetReloadAnim(curWeapon)) || RpAnimBlendClumpGetAssociation(GetClump(), GetCrouchReloadAnim(curWeapon)))) {
		if (!IsPlayer() || m_nPedState != PED_ATTACK || ((CPlayerPed*)this)->m_bHaveTargetSelected)
			bIsAttacking = false;
		else
			bIsAttacking = true;

		return;
	}

	if (GetWeapon()->m_eWeaponType == WEAPONTYPE_UNARMED || curWeapon->m_bFightMode || GetWeapon()->m_eWeaponType == WEAPONTYPE_BRASSKNUCKLE) {
		if (IsPlayer() ||
			(m_nPedState != PED_FIGHT && m_nMoveState != PEDMOVE_NONE && m_nMoveState != PEDMOVE_STILL
				&& !(m_pedStats->m_flags & STAT_SHOPPING_BAGS) && curWeapon->m_bPartialAttack)) {

			if (m_nPedState != PED_ATTACK) {
				if (m_nPedState == PED_FOLLOW_PATH)
					ClearFollowPath();

				m_nPedState = PED_ATTACK;
				bIsAttacking = false;

				CAnimBlendAssociation *animAssoc = CAnimManager::BlendAnimation(GetClump(), curWeapon->m_AnimToPlay, ANIM_MELEE_ATTACK_START, 8.0f);
				animAssoc->SetRun();
				if (animAssoc->currentTime == animAssoc->hierarchy->totalLength)
					animAssoc->SetCurrentTime(0.0f);

				animAssoc->SetFinishCallback(FinishedAttackCB, this);
			}
		} else {
			StartFightAttack(CGeneral::GetRandomNumber());
		}
		return;
	}

	if (curWeapon->m_bPartialAttack &&
		(IsPlayer() && ((CPlayerPed*)this)->m_fMoveSpeed >= 1.0f ||
		m_nMoveState == PEDMOVE_WALK || m_nMoveState == PEDMOVE_RUN)) {

		if (m_nPedState != PED_ATTACK) {
			if (m_nPedState == PED_FOLLOW_PATH)
				ClearFollowPath();

			m_nPedState = PED_ATTACK;
			bIsAttacking = false;
			CAnimBlendAssociation* animAssoc = CAnimManager::BlendAnimation(GetClump(), curWeapon->m_AnimToPlay, ANIM_MELEE_ATTACK_START, 8.0f);
			animAssoc->SetRun();
			if (animAssoc->currentTime == animAssoc->hierarchy->totalLength)
				animAssoc->SetCurrentTime(0.0f);

			animAssoc->SetFinishCallback(FinishedAttackCB, this);
		}
		return;
	}

	// TODO(Miami): Clean up old referene
	m_pSeekTarget = victim;
	if (m_pSeekTarget)
		m_pSeekTarget->RegisterReference((CEntity **) &m_pSeekTarget);

	if (curWeapon->m_bCanAim) {
		CVector aimPos = GetRight() * 0.1f + GetForward() * 0.2f + GetPosition();
		aimPos += GetUp() * 0.35f;
		CEntity *obstacle = CWorld::TestSphereAgainstWorld(aimPos, 0.2f, nil, true, false, false, true, false, false);
		if (obstacle) {
			if(gaTempSphereColPoints[0].surfaceB != SURFACE_SCAFFOLD && gaTempSphereColPoints[0].surfaceB != SURFACE_METAL_FENCE &&
				gaTempSphereColPoints[0].surfaceB != SURFACE_WOOD_BOX && gaTempSphereColPoints[0].surfaceB != SURFACE_METAL_POLE) {
				if (!IsPlayer()) {
					bObstacleShowedUpDuringKillObjective = true;
					m_shootTimer = 0;
					SetAttackTimer(1500);
					m_shotTime = CTimer::GetTimeInMilliseconds();
				}
				return;
			}
		}

		m_pLookTarget = victim;
		if (victim) {
			m_pLookTarget->RegisterReference((CEntity **) &m_pLookTarget);
			m_pSeekTarget->RegisterReference((CEntity **) &m_pSeekTarget);
		}

		if (m_pLookTarget) {
			SetAimFlag(m_pLookTarget);
		} else if (this == FindPlayerPed() && TheCamera.Cams[0].Using3rdPersonMouseCam()) {
			SetAimFlag(m_fRotationCur);
			((CPlayerPed*)this)->m_fFPSMoveHeading = TheCamera.Find3rdPersonQuickAimPitch();
		} else if (curWeapon->m_bCanAimWithArm) {
			SetAimFlag(m_fRotationCur);
		}
	}
	if (m_nPedState == PED_ATTACK) {
		bIsAttacking = true;
		return;
	}

	if (IsPlayer() || (!victimPed || victimPed->IsPedInControl())) {
		if (IsPlayer())
			CPad::GetPad(0)->ResetAverageWeapon();

		PointBlankNecessity pointBlankStatus;
		if ((curWeapon->m_eWeaponFire == WEAPON_FIRE_INSTANT_HIT || GetWeapon()->m_eWeaponType == WEAPONTYPE_FLAMETHROWER)
			&& TheCamera.PlayerWeaponMode.Mode != CCam::MODE_M16_1STPERSON
			&& TheCamera.PlayerWeaponMode.Mode != CCam::MODE_M16_1STPERSON_RUNABOUT
			&& TheCamera.PlayerWeaponMode.Mode != CCam::MODE_SNIPER
			&& TheCamera.PlayerWeaponMode.Mode != CCam::MODE_SNIPER_RUNABOUT
			&& (pointBlankStatus = CheckForPointBlankPeds(victimPed)) != NO_POINT_BLANK_PED) {
			ClearAimFlag();

			// This condition is pointless
			if (pointBlankStatus == POINT_BLANK_FOR_WANTED_PED || !victimPed && (IsPlayer() || !m_carInObjective))
				StartFightAttack(200);
		} else {
			if (!curWeapon->m_bCanAim)
				m_pSeekTarget = nil;

			if (m_nPedState != PED_AIM_GUN)
				SetStoredState();

			if (m_nPedState == PED_FOLLOW_PATH)
				ClearFollowPath();

			m_nPedState = PED_ATTACK;
			SetMoveState(PEDMOVE_NONE);
			if (bCrouchWhenShooting && bIsDucking && !!curWeapon->m_bCrouchFire) {
				CAnimBlendAssociation* curMoveAssoc = RpAnimBlendClumpGetAssociation(GetClump(), GetCrouchFireAnim(curWeapon));
				if (curMoveAssoc) {
					if (strcmp(CAnimManager::GetAnimAssociation(curWeapon->m_AnimToPlay, GetCrouchFireAnim(curWeapon))->hierarchy->name, curMoveAssoc->hierarchy->name) != 0) {
						delete curMoveAssoc;
					}
				}
				animAssoc = CAnimManager::BlendAnimation(GetClump(), curWeapon->m_AnimToPlay, GetCrouchFireAnim(curWeapon), 8.0f);
			} else {
				float animDelta = 8.0f;
				if (curWeapon->m_eWeaponFire == WEAPON_FIRE_MELEE)
					animDelta = 1000.0f;

				AnimationId fireAnim;
				if (curWeapon->m_bThrow)
					fireAnim = ANIM_THROWABLE_START_THROW;
				else if (CGame::nastyGame && (curWeapon->m_bGround2nd || curWeapon->m_bGround3rd)) {
					PedOnGroundState pedOnGround = CheckForPedsOnGroundToAttack(this, nil);
					if (pedOnGround > PED_IN_FRONT_OF_ATTACKER || pedOnGround == NO_PED && bIsStanding && m_pCurSurface && m_pCurSurface->IsVehicle()) {
						fireAnim = GetFireAnimGround(curWeapon, false);
					} else {
						fireAnim = GetFireAnimNotDucking(curWeapon);
					}
				} else {
					fireAnim = GetFireAnimNotDucking(curWeapon);
				}

				CAnimBlendAssociation* curFireAssoc = RpAnimBlendClumpGetAssociation(GetClump(), fireAnim);
				if (curFireAssoc) {
					if (strcmp(CAnimManager::GetAnimAssociation(curWeapon->m_AnimToPlay, fireAnim)->hierarchy->name, curFireAssoc->hierarchy->name) != 0) {
						delete curFireAssoc;
					}
				}
				animAssoc = CAnimManager::BlendAnimation(GetClump(), curWeapon->m_AnimToPlay, fireAnim, animDelta);
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

	// TODO(Miami): New parameter
	SetLookFlag(victim, true); //true);
	SetLookTimer(100);
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

	m_nPedState = PED_FIGHT;
	m_fightButtonPressure = 0;
	RpAnimBlendClumpRemoveAssociations(GetClump(), ASSOC_REPEAT);
	CAnimBlendAssociation *animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_WALK_START);

	if (animAssoc) {
		animAssoc->flags |= ASSOC_DELETEFADEDOUT;
		animAssoc->blendDelta = -1000.0f;
	}

	animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_RUN_STOP);
	if (!animAssoc)
		animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_RUN_STOP_R);

	if (animAssoc) {
		animAssoc->flags |= ASSOC_DELETEFADEDOUT;
		animAssoc->blendDelta = -1000.0f;
		RestoreHeadingRate();
	}

	SetMoveState(PEDMOVE_NONE);
	m_nStoredMoveState = PEDMOVE_NONE;

	CAnimManager::AddAnimation(GetClump(), ASSOCGRP_STD, ANIM_FIGHT_IDLE)->blendAmount = 1.0f;

	CPed *pedOnGround = nil;
	if (IsPlayer() && CheckForPedsOnGroundToAttack(this, &pedOnGround) > PED_IN_FRONT_OF_ATTACKER) {
		m_lastFightMove = FIGHTMOVE_GROUNDKICK;
	} else if (m_pedStats->m_flags & STAT_SHOPPING_BAGS) {
		m_lastFightMove = FIGHTMOVE_ROUNDHOUSE;
	} else {
		m_lastFightMove = FIGHTMOVE_STDPUNCH;
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
	animAssoc = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, tFightMoves[m_lastFightMove].animId, 4.0f);
	animAssoc->SetFinishCallback(FinishFightMoveCB, this);
	m_fightState = FIGHTSTATE_NO_MOVE;
	m_takeAStepAfterAttack = false;
	bIsAttacking = true;

	if (IsPlayer())
		nPlayerInComboMove = 0;
}

// --MIAMI: Done
void
CPed::LoadFightData(void)
{
	float startFireTime, endFireTime, comboFollowOnTime, strikeRadius, extendReachMultiplier;
	int damage, flags;
	char line[256], moveName[32], animName[32], hitLevel;
	int moveId = 0;

	CAnimBlendAssociation *animAssoc;

	int bp, buflen;
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
		for (lp = 0; line[lp] <= ' '; lp++);

		if (lp >= linelen ||		// FIX: game uses == here, but this is safer if we have empty lines
			line[lp] == '#')
			continue;

		sscanf(
			&line[lp],
			"%s %f %f %f %f %f %c %s %d %d",
			moveName,
			&startFireTime,
			&endFireTime,
			&comboFollowOnTime,
			&strikeRadius,
			&extendReachMultiplier,
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
		tFightMoves[moveId].extendReachMultiplier = extendReachMultiplier;
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

		if (strncmp(animName, "default", 8) != 0) {
			if (strncmp(animName, "null", 5) != 0) {
				animAssoc = CAnimManager::GetAnimAssociation(ASSOCGRP_STD, animName);
				tFightMoves[moveId].animId = (AnimationId)animAssoc->animId;
			}
			else {
				tFightMoves[moveId].animId = ANIM_WALK;
			}
		}
		moveId++;
	}
}

// Actually GetLocalDirectionTo(Turn/Look)
int
CPed::GetLocalDirection(const CVector2D &posOffset)
{
	float direction;

	for (direction = posOffset.Heading() - m_fRotationCur + DEGTORAD(45.0f); direction < 0.0f; direction += TWOPI);

	for (direction = (int)RADTODEG(direction) / 90; direction > 3; direction -= 4);

	// 0-forward, 1-left, 2-backward, 3-right.
	return direction;
}

bool
CPed::FightStrike(CVector &touchedNodePos)
{
	CColModel *hisCol;
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
			maxDistanceToBeBeaten = nearPed->GetBoundRadius() + tFightMoves[m_lastFightMove].strikeRadius + 0.1f;
		else
			maxDistanceToBeBeaten = nearPed->GetBoundRadius() + tFightMoves[m_lastFightMove].strikeRadius;

		if (nearPed->bUsesCollision || nearPed->m_nPedState == PED_DEAD) {
			CVector nearPedCentre;

			// Have to animate a skinned clump because the initial col model is useless
			hisCol = ((CPedModelInfo*)CModelInfo::GetModelInfo(nearPed->GetModelIndex()))->AnimatePedColModelSkinnedWorld(nearPed->GetClump());

			nearPed->GetBoundCentre(nearPedCentre);
			CVector potentialAttackDistance = nearPedCentre - touchedNodePos;

			// He can beat us
			if (sq(maxDistanceToBeBeaten) > potentialAttackDistance.MagnitudeSqr()) {

				for (int j = 0; j < hisCol->numSpheres; j++) {
					attackDistance = hisCol->spheres[j].center;
					attackDistance -= touchedNodePos;
					CColSphere *hisPieces = hisCol->spheres;
					float maxDistanceToBeat = hisPieces[j].radius + tFightMoves[m_lastFightMove].strikeRadius;

					// We can beat him too
					if (sq(maxDistanceToBeat) > attackDistance.MagnitudeSqr()) {
						pedFound = true;
						closestPedPiece = (ePedPieceTypes) hisPieces[j].piece;
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
		int damageMult = tFightMoves[m_lastFightMove].damage * ((CGeneral::GetRandomNumber() & 1) + 2) + 1;

		CVector2D diff (GetPosition() - nearPed->GetPosition());
		int direction = nearPed->GetLocalDirection(diff);
		if (IsPlayer()) {
			if (((CPlayerPed*)this)->m_bAdrenalineActive)
				damageMult = 20;
		} else {
			damageMult *= m_pedStats->m_attackStrength;
		}

		/*
		// Change direction if we used kick.
		if (m_lastFightMove == FIGHTMOVE_KICK) {
			if (CGeneral::GetRandomNumber() & 1) {
				direction++;
				if (direction > 3)
					direction -= 4;
			}
		} */
		nearPed->ReactToAttack(this);

		// Mostly unused. if > 5, ANIM_HIT_WALK will be run, that's it.
		int unk2;
		if (GetWeapon()->m_eWeaponType != WEAPONTYPE_UNARMED && !nearPed->IsPlayer())
			unk2 = 101;
		else
			unk2 = damageMult;

		nearPed->StartFightDefend(direction, tFightMoves[m_lastFightMove].hitLevel, unk2);
		PlayHitSound(nearPed);
		m_fightState = FIGHTSTATE_JUST_ATTACKED;
		RpAnimBlendClumpGetAssociation(GetClump(), tFightMoves[m_lastFightMove].animId)->speed = 0.6f;
		if (!nearPed->DyingOrDead()) {
			nearPed->InflictDamage(this, WEAPONTYPE_UNARMED, damageMult * 3.0f, closestPedPiece, direction);
		}

		if (CGame::nastyGame
			&& tFightMoves[m_lastFightMove].hitLevel > HITLEVEL_MEDIUM
			&& nearPed->m_nPedState == PED_DIE
			&& nearPed->GetIsOnScreen()) {

			// Just for blood particle. We will restore it below.
			attackDistance /= (10.0f * attackDistance.Magnitude());
			for(int i=0; i<4; i++) {
				CParticle::AddParticle(PARTICLE_BLOOD, bloodPos, attackDistance, nil, 0.0f, 0, 0, 0, 0);
			}
		}
		if (nearPed->m_nPedState != PED_FALL && nearPed->m_nPedState != PED_DIE && nearPed->m_nPedState != PED_DEAD) {
			float curVictimHealth = nearPed->m_fHealth;
			if (curVictimHealth > 0.0f
				&& (curVictimHealth < 40.0f && oldVictimHealth > 40.0f && !nearPed->IsPlayer()
					|| nearPed->m_fHealth < 20.0f && oldVictimHealth > 20.0f
					|| GetWeapon()->m_eWeaponType != WEAPONTYPE_UNARMED && IsPlayer()
					|| nearPed->m_pedStats->m_flags & STAT_ONE_HIT_KNOCKDOWN)) {

				nearPed->SetFall(0, (AnimationId)(direction + ANIM_KO_SKID_FRONT), 0);
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
			if (m_lastFightMove == FIGHTMOVE_GROUNDKICK) {
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

	m_vecHitLastPos = *touchedNodePos;
	return false;
}

void
CPed::SetFall(int extraTime, AnimationId animId, uint8 evenIfNotInControl)
{
	if (!IsPedInControl() && (!evenIfNotInControl || DyingOrDead()))
		return;

	ClearLookFlag();
	ClearAimFlag();
	SetStoredState();
	m_nPedState = PED_FALL;
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
CPed::SetFlee(CEntity *fleeFrom, int time)
{
	if (!IsPedInControl() || bKindaStayInSamePlace || !fleeFrom)
		return;

	SetStoredState();
	m_nPedState = PED_FLEE_ENTITY;
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
CPed::SetFlee(CVector2D const &from, int time)
{
	if (CTimer::GetTimeInMilliseconds() < m_nPedStateTimer || !IsPedInControl() || bKindaStayInSamePlace)
		return;

	if (m_nPedState != PED_FLEE_ENTITY) {
		SetStoredState();
		m_nPedState = PED_FLEE_POS;
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
CPed::SetWaitState(eWaitState state, void *time)
{
	AnimationId waitAnim = NUM_STD_ANIMS;
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
			CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_IDLE_HBHB, 4.0f);
			break;
		case WAITSTATE_CROSS_ROAD_LOOK:
			CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_ROAD_CROSS, 8.0f);

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
			CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_IDLE_HBHB, 4.0f);
			break;
		case WAITSTATE_HITWALL:
			m_headingRate = 2.0f;
			m_nWaitTimer = CTimer::GetTimeInMilliseconds() + 5000;
			animAssoc = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_HIT_WALL, 16.0f);
			animAssoc->flags |= ASSOC_DELETEFADEDOUT;
			animAssoc->flags |= ASSOC_FADEOUTWHENDONE;
			animAssoc->SetDeleteCallback(FinishedWaitCB, this);

			if (m_objective == OBJECTIVE_ENTER_CAR_AS_PASSENGER && CharCreatedBy == RANDOM_CHAR && m_nPedState == PED_SEEK_CAR) {
				ClearObjective();
				RestorePreviousState();
				m_hitRecoverTimer = CTimer::GetTimeInMilliseconds() + 30000;
			}
			break;
		case WAITSTATE_TURN180:
			m_headingRate = 0.0f;
			m_nWaitTimer = CTimer::GetTimeInMilliseconds() + 5000;
			animAssoc = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_TURN_180, 4.0f);
			animAssoc->SetFinishCallback(FinishedWaitCB, this);
			animAssoc->SetDeleteCallback(RestoreHeadingRateCB, this);
			break;
		case WAITSTATE_SURPRISE:
			m_headingRate = 0.0f;
			m_nWaitTimer = CTimer::GetTimeInMilliseconds() + 2000;
			animAssoc = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_HIT_WALL, 4.0f);
			animAssoc->SetFinishCallback(FinishedWaitCB, this);
			break;
		case WAITSTATE_STUCK:
			SetMoveState(PEDMOVE_STILL);
			SetMoveAnim();
			m_headingRate = 0.0f;
			m_nWaitTimer = CTimer::GetTimeInMilliseconds() + 5000;
			CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_IDLE_TIRED, 4.0f);

			if (m_objective == OBJECTIVE_ENTER_CAR_AS_PASSENGER && CharCreatedBy == RANDOM_CHAR && m_nPedState == PED_SEEK_CAR) {
				ClearObjective();
				RestorePreviousState();
				m_hitRecoverTimer = CTimer::GetTimeInMilliseconds() + 30000;
			}
			break;
		case WAITSTATE_LOOK_ABOUT:
			SetMoveState(PEDMOVE_STILL);
			SetMoveAnim();
			m_headingRate = 0.0f;
			m_nWaitTimer = CTimer::GetTimeInMilliseconds() + 5000;
			CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_IDLE_HBHB, 4.0f);
			break;
		case WAITSTATE_PLAYANIM_COWER:
			waitAnim = ANIM_HANDSCOWER;
		case WAITSTATE_PLAYANIM_HANDSUP:
			if (waitAnim == NUM_STD_ANIMS)
				waitAnim = ANIM_HANDSUP;
		case WAITSTATE_PLAYANIM_HANDSCOWER:
			if (waitAnim == NUM_STD_ANIMS)
				waitAnim = ANIM_HANDSCOWER;
			m_headingRate = 0.0f;
			if (time)
				m_nWaitTimer = CTimer::GetTimeInMilliseconds() + *(int*)time;
			else
				m_nWaitTimer = CTimer::GetTimeInMilliseconds() + 3000;

			animAssoc = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, waitAnim, 4.0f);
			animAssoc->SetDeleteCallback(FinishedWaitCB, this);
			break;
		case WAITSTATE_PLAYANIM_DUCK:
			waitAnim = ANIM_DUCK_DOWN;
		case WAITSTATE_PLAYANIM_TAXI:
			if (waitAnim == NUM_STD_ANIMS)
				waitAnim = ANIM_IDLE_TAXI;
		case WAITSTATE_PLAYANIM_CHAT:
			if (waitAnim == NUM_STD_ANIMS)
				waitAnim = ANIM_IDLE_CHAT;
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
			CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_IDLE_TIRED, 4.0f);
			break;
		case WAITSTATE_SIT_DOWN:
			animAssoc = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_SEAT_DOWN, 4.0f);
			animAssoc->SetFinishCallback(FinishedWaitCB, this);
			m_nWaitTimer = CTimer::GetTimeInMilliseconds() + 10000;
			break;
		case WAITSTATE_SIT_UP:
			animAssoc = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_SEAT_UP, 4.0f);
			animAssoc->SetFinishCallback(FinishedWaitCB, this);
			m_nWaitTimer = CTimer::GetTimeInMilliseconds() + 100000;
			break;
		case WAITSTATE_SIT_IDLE:
			animAssoc = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_SEAT_IDLE, 5000.0f);
			animAssoc->SetFinishCallback(FinishedWaitCB, this);
			if (time)
				m_nWaitTimer = CTimer::GetTimeInMilliseconds() + *(int*)time;
			else
				m_nWaitTimer = CTimer::GetTimeInMilliseconds() + CGeneral::GetRandomNumberInRange(25000, 30000);
			break;
		case WAITSTATE_USE_ATM:
			animAssoc = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_ATM, 5000.0f);
			animAssoc->SetFinishCallback(FinishedWaitCB, this);
			if (time)
				m_nWaitTimer = CTimer::GetTimeInMilliseconds() + *(int*)time;
			else
				m_nWaitTimer = CTimer::GetTimeInMilliseconds() + 100000;
			break;
		case WAITSTATE_SUN_BATHE_PRE:
		case WAITSTATE_SUN_BATHE_DOWN:
		case WAITSTATE_SUN_BATHE_IDLE:
		case WAITSTATE_RIOT:
		case WAITSTATE_FAST_FALL:
		case WAITSTATE_BOMBER:
		case WAITSTATE_STRIPPER:
		case WAITSTATE_GROUND_ATTACK:
		case WAITSTATE_LANCESITTING:
		case WAITSTATE_PLAYANIM_HANDSUP_SIMPLE:
		default:
			ClearWaitState();
			RestoreHeadingRate();
			return;
	}
	m_nWaitState = state;
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
	if (m_lastFightMove == FIGHTMOVE_NULL || m_lastFightMove >= FIGHTMOVE_HITFRONT)
		return;

	uint16 soundId;

	// And this is why second dimension is between 13 and 22.
	if (hitTo->m_lastFightMove <= FIGHTMOVE_GROUNDKICK || hitTo->m_lastFightMove >= FIGHTMOVE_IDLE2NORM) {

		if (hitTo->m_nPedState == PED_DEAD || hitTo->UseGroundColModel()) {	
			soundId = hitSoundsByFightMoves[m_lastFightMove - FIGHTMOVE_STDPUNCH][FIGHTMOVE_HITONFLOOR - FIGHTMOVE_HITFRONT];
		} else {
			soundId = hitSoundsByFightMoves[m_lastFightMove - FIGHTMOVE_STDPUNCH][FIGHTMOVE_HITFRONT - FIGHTMOVE_HITFRONT];
		}
	} else {
		soundId = hitSoundsByFightMoves[m_lastFightMove - FIGHTMOVE_STDPUNCH][hitTo->m_lastFightMove - FIGHTMOVE_HITFRONT];
	}

	if (soundId != NO_SND)
		DMAudio.PlayOneShot(m_audioEntityId, soundId, 0.0f);
}

void
CPed::Say(uint16 audio)
{
	uint16 audioToPlay = audio;

	if (IsPlayer()) {

		// Ofc this part isn't in VC.
		switch (audio) {
			case SOUND_PED_DEATH:
				audioToPlay = SOUND_PED_DAMAGE;
				break;
			case SOUND_PED_DAMAGE:
			case SOUND_PED_HIT:
			case SOUND_PED_LAND:
				break;
			case SOUND_PED_BULLET_HIT:
			case SOUND_PED_CAR_JACKED:
			case SOUND_PED_DEFEND:
				audioToPlay = SOUND_PED_HIT;
				break;
			default:
				return;
		}
	} else {
		if (3.0f + TheCamera.GetPosition().z < GetPosition().z)
			return;

		if (TheCamera.m_CameraAverageSpeed > 1.65f) {
#ifdef VC_PED_PORTS
			if (audio != SOUND_PED_DAMAGE && audio != SOUND_PED_HIT && audio != SOUND_PED_LAND)
#endif
				return;

		} else if (TheCamera.m_CameraAverageSpeed > 1.25f) {
			if (audio != SOUND_PED_DEATH &&
#ifdef VC_PED_PORTS
				audio != SOUND_PED_DAMAGE && audio != SOUND_PED_HIT && audio != SOUND_PED_LAND &&
#endif
				audio != SOUND_PED_TAXI_WAIT && audio != SOUND_PED_EVADE)
				return;

		} else if (TheCamera.m_CameraAverageSpeed > 0.9f) {
			switch (audio) {
				case SOUND_PED_DEATH:
#ifdef VC_PED_PORTS
				case SOUND_PED_DAMAGE:
				case SOUND_PED_HIT:
				case SOUND_PED_LAND:
#endif
				case SOUND_PED_BURNING:
				case SOUND_PED_FLEE_SPRINT:
				case SOUND_PED_TAXI_WAIT:
				case SOUND_PED_EVADE:
				case SOUND_PED_CAR_COLLISION:
					break;
				default:
					return;
			}
		}
	}

	if (audioToPlay < m_queuedSound) {
		if (audioToPlay != m_lastQueuedSound || audioToPlay == SOUND_PED_DEATH
			|| CommentWaitTime[audioToPlay - SOUND_PED_DEATH].m_nOverrideMaxRandomDelayTime
				+ m_lastSoundStart
				+ (uint32) CGeneral::GetRandomNumberInRange(0, CommentWaitTime[audioToPlay - SOUND_PED_DEATH].m_nMaxRandomDelayTime) <= CTimer::GetTimeInMilliseconds()) {
			m_queuedSound = audioToPlay;
		}
	}
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
					|| m_objective == OBJECTIVE_FOLLOW_PED_IN_FORMATION && m_pedInObjective == collideWith
					|| collideWith->m_objective == OBJECTIVE_FOLLOW_PED_IN_FORMATION && collideWith->m_pedInObjective == this
#endif			
					)) {

				if (m_objective != OBJECTIVE_FOLLOW_PED_IN_FORMATION && m_objective != OBJECTIVE_GOTO_CHAR_ON_FOOT) {

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
#ifdef VC_PED_PORTS
								if (FindPlayerPed() != m_pedInObjective
									|| m_objective != OBJECTIVE_KILL_CHAR_ANY_MEANS && m_objective != OBJECTIVE_KILL_CHAR_ON_FOOT
									|| collideWith == m_pedInObjective) {
#endif
									if (weAreMissionChar || m_pedStats->m_fear <= 100 - collideWith->m_pedStats->m_temper
										|| (collideWith->IsPlayer() || collideWith->m_nMoveState == PEDMOVE_NONE || collideWith->m_nMoveState == PEDMOVE_STILL) &&
										(!collideWith->IsPlayer() || ((CPlayerPed*)collideWith)->m_fMoveSpeed <= 1.0f)) {
										SetDirectionToWalkAroundObject(collideWith);
										if (!weAreMissionChar)
											Say(SOUND_PED_CHAT);
									} else {
										SetEvasiveStep(collideWith, 2);
									}
#ifdef VC_PED_PORTS
								} else if (collideWith->m_nMoveState != PEDMOVE_STILL && GetWeapon()->IsTypeMelee()
									&& collideWith->m_pedInObjective == m_pedInObjective) {

									int colliderIsAtPlayerSafePosID = -1;
									int weAreAtPlayerSafePosID = -1;
									for (int i = 0; i < ARRAY_SIZE(((CPlayerPed*)m_pedInObjective)->m_pPedAtSafePos); i++) {
										CPed *pedAtSafePos = ((CPlayerPed*)m_pedInObjective)->m_pPedAtSafePos[i];
										if (pedAtSafePos == this) {
											weAreAtPlayerSafePosID = i;
										} else if (pedAtSafePos == collideWith) {
											colliderIsAtPlayerSafePosID = i;
										}
									}
									bool weAreCloserToTargetThenCollider = false;
									if ((GetPosition() - m_vecSeekPos).MagnitudeSqr2D() < (collideWith->GetPosition() - m_vecSeekPos).MagnitudeSqr2D())
										weAreCloserToTargetThenCollider = true;

									if (weAreAtPlayerSafePosID <= 0 || weAreCloserToTargetThenCollider) {
										if (!weAreCloserToTargetThenCollider) {
											int time = 300;
											SetWaitState(WAITSTATE_CROSS_ROAD_LOOK, &time);
											m_nPedStateTimer = CTimer::GetTimeInMilliseconds() + time;
										}
									} else if (colliderIsAtPlayerSafePosID <= 0) {
										if (collideWith->m_pedInObjective == FindPlayerPed()) {
											// VC specific
											// ((CPlayerPed*)m_pedInObjective)->RemovePedFromMeleeList(this);
											int time = 500;
											SetWaitState(WAITSTATE_CROSS_ROAD_LOOK, &time);
											m_nPedStateTimer = CTimer::GetTimeInMilliseconds() + time;
										}
									} else {
										int time = 300;
										SetWaitState(WAITSTATE_CROSS_ROAD_LOOK, &time);
										m_nPedStateTimer = CTimer::GetTimeInMilliseconds() + time;
									}
								} else {
									SetDirectionToWalkAroundObject(collideWith);
								}
#endif
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
							}
							m_nPedStateTimer = CTimer::GetTimeInMilliseconds() + CGeneral::GetRandomNumberInRange(250, 450);
						}
					}
				} else {
					if (m_pedInObjective && collideWith == m_pedInObjective && CTimer::GetTimeInMilliseconds() > m_nPedStateTimer) {
						if (heLooksToUs) {
							SetEvasiveStep(collideWith, 1);
							m_nPedStateTimer = CTimer::GetTimeInMilliseconds() + 3000;
						}
					} else if (weDontLookToHim && IsPedInControl()) {

						if (m_pedStats != collideWith->m_pedStats) {

							if (collideWith->m_pedStats->m_fear <= 100 - m_pedStats->m_temper) {

								if (collideWith->IsPlayer()) {
									// He's on our right side
									if (DotProduct(posDiff,GetRight()) <= 0.0f)
										m_fRotationCur -= m_headingRate;
									else
										m_fRotationCur += m_headingRate;
								} else {
									// He's on our right side
									if (DotProduct(posDiff, GetRight()) <= 0.0f)
										m_fRotationCur -= m_headingRate;
									else
										m_fRotationCur += m_headingRate;
								}
							} else {
								SetLookFlag(collideWith, false);
								TurnBody();
								animAssoc = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_FIGHT_PPUNCH, 8.0f);
								animAssoc->flags |= ASSOC_FADEOUTWHENDONE;
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
						animToPlay = ANIM_HIT_LEFT;
					else
						animToPlay = ANIM_SHOT_LEFT_PARTIAL;
				} else if (heLooksToUs) {
					CVector moveForce = GetRight() * -1.0f;
					moveForce.z += 0.1f;
					ApplyMoveForce(moveForce);
					if (collideWith->m_nMoveState != PEDMOVE_RUN && collideWith->m_nMoveState != PEDMOVE_SPRINT)
						animToPlay = ANIM_HIT_RIGHT;
					else
						animToPlay = ANIM_SHOT_RIGHT_PARTIAL;
				} else {
					if (collideWith->m_nMoveState != PEDMOVE_RUN && collideWith->m_nMoveState != PEDMOVE_SPRINT)
						animToPlay = ANIM_HIT_BACK;
					else
						animToPlay = ANIM_SHOT_BACK_PARTIAL;
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
						animToPlay = ANIM_KO_SPIN_L;
					else
						animToPlay = ANIM_KD_RIGHT;
				} else {
					CVector moveForce = GetRight();
					moveForce.z += 0.1f;
					ApplyMoveForce(moveForce);
					if (heLooksToUs)
						animToPlay = ANIM_KO_SPIN_R;
					else
						animToPlay = ANIM_KD_LEFT;
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

// --MIAMI: Done except commented thing
void
CPed::CreateDeadPedMoney(void)
{
	if (!CGame::nastyGame)
		return;

	int skin = GetModelIndex();

	// TODO(Miami): New flag
	if ((skin >= MI_COP && skin <= MI_FIREMAN) || CharCreatedBy == MISSION_CHAR || bInVehicle)
		return;

	int money = m_nPedMoney;
	if (money < 10)
		return;

	CVector pickupPos = GetPosition();
	CPickups::CreateSomeMoney(pickupPos, money);
	m_nPedMoney = 0;
}

void
CPed::CreateDeadPedPickupCoors(float *x, float *y, float *z)
{
	bool found = false;
	CVector pickupPos;

#define NUMBER_OF_ATTEMPTS 32
	for (int i = 0; i < NUMBER_OF_ATTEMPTS; i++) {

		pickupPos = GetPosition();
		pickupPos.x = 1.5f * Sin((CGeneral::GetRandomNumber() % 256) * PI / 128) + GetPosition().x;
		pickupPos.y = 1.5f * Cos((CGeneral::GetRandomNumber() % 256) * PI / 128) + GetPosition().y;
		pickupPos.z = CWorld::FindGroundZFor3DCoord(pickupPos.x, pickupPos.y, pickupPos.z, &found) + 0.5f;

		if (!found)
			continue;

		CVector pedPos = GetPosition();
		pedPos.z += 0.3f;

		CVector pedToPickup = pickupPos - pedPos;
		float distance = pedToPickup.Magnitude();

		// outer edge of pickup
		distance = (distance + 0.4f) / distance;
		CVector pickupPos2 = pedPos;
		pickupPos2 += distance * pedToPickup;

		if ((pickupPos - FindPlayerCoors()).Magnitude2D() > 2.0f || i > NUMBER_OF_ATTEMPTS / 2) {

			if (i > NUMBER_OF_ATTEMPTS / 2 || !CPickups::TestForPickupsInBubble(pickupPos, 1.3f)) {

				if (CWorld::GetIsLineOfSightClear(pickupPos2, pedPos,
					true, i < NUMBER_OF_ATTEMPTS / 2, false, i < NUMBER_OF_ATTEMPTS / 2, false, false, false)) {

					if (i > NUMBER_OF_ATTEMPTS / 2 || !CWorld::TestSphereAgainstWorld(pickupPos, 1.2f, nil, false, true, false, false, false, false)) {
						*x = pickupPos.x;
						*y = pickupPos.y;
						*z = pickupPos.z;
						return;
					}
				}
			}
		}
	}
	*x = GetPosition().x;
	*y = GetPosition().y;
	*z = GetPosition().z + 0.4f;
#undef NUMBER_OF_ATTEMPTS
}

void
CPed::CreateDeadPedWeaponPickups(void)
{
	CVector pickupPos;

	if (bInVehicle)
		return;

	for(int i = 0; i < TOTAL_WEAPON_SLOTS; i++) {

		eWeaponType weapon = GetWeapon(i).m_eWeaponType;
		int weaponAmmo = GetWeapon(i).m_nAmmoTotal;
		if (weapon == WEAPONTYPE_UNARMED || weapon == WEAPONTYPE_DETONATOR || (weaponAmmo == 0 && !GetWeapon(i).IsTypeMelee()))
			continue;

		int quantity = Min(weaponAmmo, AmmoForWeapon_OnStreet[weapon] / 2);
		CreateDeadPedPickupCoors(&pickupPos.x, &pickupPos.y, &pickupPos.z);
		if (!CPickups::TryToMerge_WeaponType(pickupPos, weapon, PICKUP_ONCE_TIMEOUT, quantity, false)) {
			CPickups::GenerateNewOne_WeaponType(pickupPos, weapon, PICKUP_ONCE_TIMEOUT, Min(weaponAmmo, quantity));
		}
	}
	ClearWeapons();
}

void
CPed::SetAttackTimer(uint32 time)
{
	if (CTimer::GetTimeInMilliseconds() > m_attackTimer)
		m_attackTimer = Max(m_shootTimer, CTimer::GetTimeInMilliseconds()) + time;
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
	m_vehEnterType = vehEnterType;
	if (m_vehEnterType == CAR_DOOR_LF) {
		if (veh->pDriver && veh->pDriver->IsPlayer())
			veh->SetStatus(STATUS_PLAYER_DISABLED);
		else
			veh->SetStatus(STATUS_ABANDONED);
	}
	RemoveInCarAnims();
	SetMoveState(PEDMOVE_NONE);
	LineUpPedWithCar(LINE_UP_TO_CAR_START);
	m_pVehicleAnim = nil;
	m_nPedState = PED_DRAG_FROM_CAR;
	bChangedSeat = false;
	bWillBeQuickJacked = quickJack;

	SetHeading(m_fRotationCur);

	Say(SOUND_PED_CAR_JACKED);
	SetRadioStation();
	veh->m_nGettingOutFlags |= GetCarDoorFlag(m_vehEnterType);
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
		m_nPedState = PED_BUY_ICECREAM;
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
		m_standardTimer = CTimer::GetTimeInMilliseconds() + 3000;
		m_nPedState = PED_BUY_ICECREAM;
	}
}

void
CPed::SetChat(CEntity *chatWith, uint32 time)
{
	if(m_nPedState != PED_CHAT)
		SetStoredState();

	m_nPedState = PED_CHAT;
	SetMoveState(PEDMOVE_STILL);
	SetLookFlag(chatWith, true);
	m_standardTimer = CTimer::GetTimeInMilliseconds() + time;
	m_lookTimer = CTimer::GetTimeInMilliseconds() + 3000;
}

// --MIAMI: Done
void
CPed::RemoveWeaponAnims(int unused, float animDelta)
{	
	CAnimBlendAssociation *weaponAssoc;
	//CWeaponInfo::GetWeaponInfo(unused);

	weaponAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_WEAPON_FIRE);
	if (weaponAssoc) {
		weaponAssoc->blendDelta = animDelta;
		weaponAssoc->flags |= ASSOC_DELETEFADEDOUT;
	}
	weaponAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_WEAPON_CROUCHFIRE);
	if (weaponAssoc) {
		weaponAssoc->blendDelta = animDelta;
		weaponAssoc->flags |= ASSOC_DELETEFADEDOUT;
	}
	weaponAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_WEAPON_RELOAD);
	if (weaponAssoc) {
		weaponAssoc->blendDelta = animDelta;
		weaponAssoc->flags |= ASSOC_DELETEFADEDOUT;
	}
	weaponAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_WEAPON_CROUCHRELOAD);
	if (weaponAssoc) {
		weaponAssoc->blendDelta = animDelta;
		weaponAssoc->flags |= ASSOC_DELETEFADEDOUT;
	}
	weaponAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_WEAPON_SPECIAL);
	if (weaponAssoc) {
		weaponAssoc->flags |= ASSOC_DELETEFADEDOUT;
		if (weaponAssoc->flags & ASSOC_PARTIAL)
			weaponAssoc->blendDelta = animDelta;
		else
			CAnimManager::BlendAnimation(GetClump(), m_animGroup, ANIM_IDLE_STANCE, -animDelta);
	}
}

// --MIAMI: Done
void
CPed::SetDead(void)
{
	if (!RpAnimBlendClumpGetAssociation(GetClump(), ANIM_DROWN))
		bUsesCollision = false;

	m_fHealth = 0.0f;
	if (m_nPedState == PED_DRIVING)
		bIsVisible = false;

	if (m_nPedState == PED_FOLLOW_PATH)
		ClearFollowPath();

	m_nPedState = PED_DEAD;
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

	m_nPedState = PED_SEEK_ENTITY;
	m_distanceToCountSeekDone = distanceToCountDone;
	m_pSeekTarget = seeking;
	m_pSeekTarget->RegisterReference((CEntity **) &m_pSeekTarget);
	SetMoveState(PEDMOVE_STILL);
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

	m_nPedState = PED_SEEK_POS;
	m_distanceToCountSeekDone = distanceToCountDone;
	m_vecSeekPos = pos;
}

void
CPed::DeadPedMakesTyresBloody(void)
{
	int minX = CWorld::GetSectorIndexX(GetPosition().x - 2.0f);
	if (minX < 0) minX = 0;
	int minY = CWorld::GetSectorIndexY(GetPosition().y - 2.0f);
	if (minY < 0) minY = 0;
	int maxX = CWorld::GetSectorIndexX(GetPosition().x + 2.0f);
	if (maxX > NUMSECTORS_X-1) maxX = NUMSECTORS_X-1;
	int maxY = CWorld::GetSectorIndexY(GetPosition().y + 2.0f);
	if (maxY > NUMSECTORS_Y-1) maxY = NUMSECTORS_Y-1;

	CWorld::AdvanceCurrentScanCode();

	for (int curY = minY; curY <= maxY; curY++) {
		for (int curX = minX; curX <= maxX; curX++) {
			CSector *sector = CWorld::GetSector(curX, curY);
			MakeTyresMuddySectorList(sector->m_lists[ENTITYLIST_VEHICLES]);
			MakeTyresMuddySectorList(sector->m_lists[ENTITYLIST_VEHICLES_OVERLAP]);
		}
	}
}

void
CPed::Die(void)
{
	// UNUSED: This is a perfectly empty function.
}

uint8
CPed::DoesLOSBulletHitPed(CColPoint &colPoint)
{
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
}

// TODO(Miami): Ducking doesn't work, fix this
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
			bKindaStayInSamePlace = false;
			if (bIsDucking)
				ClearDuck(true);

			bCrouchWhenShooting = false;
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
	if (bIsDucking)
		ClearDuck(true);

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

void
CPed::EndFight(uint8 endType)
{
	if (m_nPedState != PED_FIGHT)
		return;

	m_lastFightMove = FIGHTMOVE_NULL;
	RestorePreviousState();
	CAnimBlendAssociation *animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_FIGHT_IDLE);
	if (animAssoc)
		animAssoc->flags |= ASSOC_DELETEFADEDOUT;

	switch (endType) {
		case ENDFIGHT_NORMAL:
			CAnimManager::BlendAnimation(GetClump(), m_animGroup, ANIM_IDLE_STANCE, 8.0f);
			CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_FIGHT2_IDLE, 8.0f);
			break;
		case ENDFIGHT_WITH_A_STEP:
			CAnimManager::BlendAnimation(GetClump(), m_animGroup, ANIM_IDLE_STANCE, 1.0f);
			CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_WALK_START, 8.0f);
			break;
		case ENDFIGHT_FAST:
			CAnimManager::BlendAnimation(GetClump(), m_animGroup, ANIM_IDLE_STANCE, 8.0f);
			CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_FIGHT2_IDLE, 8.0f)->speed = 2.0f;
			break;
		default:
			break;
	}
	m_nWaitTimer = 0;
}

void
CPed::EnterCar(void)
{
	if (IsNotInWreckedVehicle() && m_fHealth > 0.0f) {
		CVehicle *veh = (CVehicle*)m_pSeekTarget;

		// Not used.
		// CVector posForDoor = GetPositionToOpenCarDoor(veh, m_vehEnterType);

		if (veh->CanPedOpenLocks(this)) {
			if (m_vehEnterType) {
				CAnimBlendAssociation *enterAssoc = m_pVehicleAnim;
				if (enterAssoc)
					veh->ProcessOpenDoor(m_vehEnterType, enterAssoc->animId, enterAssoc->currentTime);
			}
		}
		bIsInTheAir = false;
		LineUpPedWithCar(LINE_UP_TO_CAR_START);
	} else {
		QuitEnteringCar();
		SetDie(ANIM_KO_SHOT_FRONT1, 4.0f, 0.0f);
	}
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
			m_vehEnterType = TRAIN_POS_MID_ENTRY;
		} else {
			enterPos = rightEntryPos;
			m_vehEnterType = TRAIN_POS_RIGHT_ENTRY;
		}
	} else if (distRightEntry < distLeftEntry) {
		enterPos = rightEntryPos;
		m_vehEnterType = TRAIN_POS_RIGHT_ENTRY;
	} else {
		enterPos = leftEntryPos;
		m_vehEnterType = TRAIN_POS_LEFT_ENTRY;
	}

	return 1;
}

uint8
CPed::GetNearestTrainDoor(CVehicle *train, CVector &doorPos)
{
	GetNearestTrainPedPosition(train, doorPos);
/*
	// Not used.
	CVehicleModelInfo* trainModel = (CVehicleModelInfo*)CModelInfo::GetModelInfo(train->m_modelIndex);
	CMatrix trainMat = CMatrix(train->GetMatrix());

	doorPos = trainModel->m_positions[m_vehEnterType];
	doorPos.x -= 1.5f;
	doorPos = Multiply3x3(trainMat, doorPos);
	doorPos += train->GetPosition();
*/
	return 1;
}

#ifdef GTA_TRAIN
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
CPed::EnterTrain(void)
{
	LineUpPedWithTrain();
}

void
CPed::ExitTrain(void)
{
	LineUpPedWithTrain();
}
#endif

void
CPed::ExitCar(void)
{
	if (!m_pVehicleAnim)
		return;

	AnimationId exitAnim = (AnimationId) m_pVehicleAnim->animId;
	float animTime = m_pVehicleAnim->currentTime;

	m_pMyVehicle->ProcessOpenDoor(m_vehEnterType, exitAnim, animTime);

	if (m_pSeekTarget) {
		// Car is upside down
		if (m_pMyVehicle->GetUp().z > -0.8f) {
			if (exitAnim != ANIM_CAR_CLOSE_RHS && exitAnim != ANIM_CAR_CLOSE_LHS && animTime <= 0.3f)
				LineUpPedWithCar((m_pMyVehicle->GetModelIndex() == MI_DODO ? LINE_UP_TO_CAR_END : LINE_UP_TO_CAR_START));
			else
				LineUpPedWithCar(LINE_UP_TO_CAR_END);
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
			foundPed->SetFall(1000, ANIM_KO_SKID_FRONT, 1);
	}
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

void
CPed::Fight(void)
{
	CAnimBlendAssociation *currentAssoc = nil, *animAssoc;
	bool hasShoppingBags, punchOnly, canKick, canKneeHead, canRoundhouse;
	float angleToFace, nextAngle;
	bool goForward = false;
	int nextFightMove;

	switch (m_lastFightMove) {
		case FIGHTMOVE_NULL:
			return;
		case FIGHTMOVE_IDLE2NORM:
			m_lastFightMove = FIGHTMOVE_NULL;
			RestorePreviousState();
			break;
		case FIGHTMOVE_IDLE:
			// currentAssoc = nil;
			break;
		default:
			currentAssoc = RpAnimBlendClumpGetAssociation(GetClump(), tFightMoves[m_lastFightMove].animId);
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
		FightMove &curMove = tFightMoves[m_lastFightMove];
		if (curMove.hitLevel != HITLEVEL_NULL && animTime > curMove.startFireTime && animTime <= curMove.endFireTime && m_fightState >= FIGHTSTATE_NO_MOVE) {

			CVector touchingNodePos(0.0f, 0.0f, 0.0f);

			switch (m_lastFightMove) {
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
				case FIGHTMOVE_PUNCHJAB:
					TransformToNode(touchingNodePos, PED_HANDL);
					break;
				case FIGHTMOVE_LONGKICK:
				case FIGHTMOVE_ROUNDHOUSE:
				case FIGHTMOVE_GROUNDKICK:
					TransformToNode(touchingNodePos, PED_FOOTR);
					break;
			}

			if (m_lastFightMove == FIGHTMOVE_PUNCHJAB) {
				touchingNodePos += 0.1f * GetForward();
			} else if (m_lastFightMove == FIGHTMOVE_PUNCHHOOK) {
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
					animAssoc = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, tFightMoves[++m_lastFightMove].animId, 8.0f);
					animAssoc->SetFinishCallback(FinishFightMoveCB, this);
					animAssoc->SetCurrentTime(0.1f * animAssoc->hierarchy->totalLength);
					m_fightButtonPressure = 0;
					nPlayerInComboMove = 1;
				}
			}
		} else {
			if (curMove.startFireTime > 0.0f && m_lastFightMove != FIGHTMOVE_SHUFFLE_F && animTime > curMove.startFireTime) {
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
			nextFightMove = (m_fightButtonPressure > 190 ? FIGHTMOVE_BODYBLOW : FIGHTMOVE_KNEE);
			hasShoppingBags = false;
			canKneeHead = true;
			nPlayerInComboMove = 0;
		} else {
			nextFightMove = (m_fightButtonPressure > 120 ? FIGHTMOVE_BODYBLOW : FIGHTMOVE_KNEE);
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

					PedState nearPedState = nearPed->m_nPedState;
					if (nearPedState != PED_FALL && nearPedState != PED_GETUP && nearPedState != PED_DIE && nearPedState != PED_DEAD) {

						if (nearPedDist < 0.8f && neededTurn < DEGTORAD(75.0f) && canKneeHead) {
							canAffectMultiplePeople = false;
						} else if (nearPedDist >= 1.3f || neededTurn >= DEGTORAD(55.0f) || hasShoppingBags) {

							if (nearPedDist < 1.7f
								&& neededTurn < DEGTORAD(35.0f)
								&& (canKick || hasShoppingBags)) {

								nextFightMove = FIGHTMOVE_LONGKICK;
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

					} else if (nearPedState != PED_DEAD || pedOnGround) {
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
				PedState fightingPedState = m_pedInObjective->m_nPedState;
				if (fightingPedState != PED_FALL && fightingPedState != PED_GETUP && fightingPedState != PED_DIE && fightingPedState != PED_DEAD) {

					if (fightingPedDist >= 0.8f || !canKneeHead) {

						if (fightingPedDist >= 1.3f) {

							if (fightingPedDist < 1.7f && canKick) {
								nextFightMove = FIGHTMOVE_LONGKICK;
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
			m_lastFightMove = nextFightMove;
			animAssoc = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, tFightMoves[m_lastFightMove].animId, 4.0f);

			animAssoc->SetFinishCallback(FinishFightMoveCB, this);
			if (m_fightState == FIGHTSTATE_MOVE_FINISHED && animAssoc->currentTime != 0.0f) {
				animAssoc->SetCurrentTime(0.0f);
				animAssoc->SetRun();
			}
			m_fightButtonPressure = 0;
		}
		m_fightState = FIGHTSTATE_NO_MOVE;
	} else if (m_takeAStepAfterAttack && m_lastFightMove != FIGHTMOVE_SHUFFLE_F
#ifndef FIX_BUGS
		&& CheckForPedsOnGroundToAttack(this, nil) == 4) {
#else
		&& CheckForPedsOnGroundToAttack(this, nil) == PED_IN_FRONT_OF_ATTACKER) {
#endif
		m_lastFightMove = FIGHTMOVE_SHUFFLE_F;
		animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), tFightMoves[m_lastFightMove].animId);

		if (animAssoc) {
			animAssoc->SetCurrentTime(0.0f);
			animAssoc->blendDelta = 4.0f;
			animAssoc->SetRun();
		} else {
			animAssoc = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, tFightMoves[m_lastFightMove].animId, 32.0f);
		}
		animAssoc->SetFinishCallback(FinishFightMoveCB, this);
		m_fightState = FIGHTSTATE_NO_MOVE;
		m_fightButtonPressure = 0;
		m_takeAStepAfterAttack = false;

	} else if (m_takeAStepAfterAttack) {
		EndFight(ENDFIGHT_FAST);

	} else if (m_lastFightMove == FIGHTMOVE_IDLE) {
		if (CTimer::GetTimeInMilliseconds() > m_nWaitTimer) {
			EndFight(ENDFIGHT_NORMAL);
		}

	} else {
		m_lastFightMove = FIGHTMOVE_IDLE;
		if (IsPlayer())
			m_nWaitTimer = CTimer::GetTimeInMilliseconds() + 500;
		else
			m_nWaitTimer = CTimer::GetTimeInMilliseconds() + 2000;
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

	int closestNodeId = ThePaths.FindNodeClosestToCoors(GetPosition(), 1, 999999.9f, false, false);

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

void
CPed::FinishDieAnimCB(CAnimBlendAssociation *animAssoc, void *arg)
{
	CPed *ped = (CPed*)arg;

	if (ped->bIsPedDieAnimPlaying)
		ped->bIsPedDieAnimPlaying = false;
}

void
CPed::FinishFightMoveCB(CAnimBlendAssociation *animAssoc, void *arg)
{
	CPed *ped = (CPed*)arg;

	if (tFightMoves[ped->m_lastFightMove].animId == animAssoc->animId) {
		ped->m_fightState = FIGHTSTATE_MOVE_FINISHED;
		animAssoc->blendDelta = -1000.0f;
	}
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

void
CPed::FinishJumpCB(CAnimBlendAssociation *animAssoc, void *arg)
{
	CPed *ped = (CPed*)arg;

	ped->bResetWalkAnims = true;
	ped->bIsLanding = false;

	animAssoc->blendDelta = -1000.0f;
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
		CAnimBlendAssociation *handsCoverAssoc = CAnimManager::BlendAnimation(ped->GetClump(), ASSOCGRP_STD, ANIM_HIT_WALL, 8.0f);
		handsCoverAssoc->flags &= ~ASSOC_FADEOUTWHENDONE;
		handsCoverAssoc->SetFinishCallback(FinishHitHeadCB, ped);
		ped->bIsLanding = true;
		return;
	}

	float velocityFromAnim = 0.1f;
	CAnimBlendAssociation *sprintAssoc = RpAnimBlendClumpGetAssociation(ped->GetClump(), ANIM_SPRINT);

	if (sprintAssoc) {
		velocityFromAnim = 0.05f * sprintAssoc->blendAmount + 0.17f;
	} else {
		CAnimBlendAssociation *runAssoc = RpAnimBlendClumpGetAssociation(ped->GetClump(), ANIM_RUN);
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
	CAnimManager::AddAnimation(ped->GetClump(), ASSOCGRP_STD, ANIM_JUMP_GLIDE);

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
CPed::FinishedWaitCB(CAnimBlendAssociation *animAssoc, void *arg)
{
	CPed *ped = (CPed*)arg;

	ped->m_nWaitTimer = 0;
	ped->RestoreHeadingRate();
	ped->Wait();
}

void
CPed::Wait(void)
{
	AnimationId mustHaveAnim = NUM_STD_ANIMS;
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

				animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_IDLE_HBHB);
				if (animAssoc) {
					animAssoc->blendDelta = -8.0f;
					animAssoc->flags |= ASSOC_DELETEFADEDOUT;
				}
			}
			break;

		case WAITSTATE_CROSS_ROAD_LOOK:
			if (CTimer::GetTimeInMilliseconds() > m_nWaitTimer) {
				ClearWaitState();
				animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_ROAD_CROSS);
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
					CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_XPRESS_SCRATCH, 4.0f);
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
				if (RpAnimBlendClumpGetAssociation(GetClump(), ANIM_HIT_WALL)) {
					animAssoc = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_XPRESS_SCRATCH, 4.0f);
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

			animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_IDLE_TIRED);

			if (!animAssoc)
				animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_TURN_180);
			if (!animAssoc)
				animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_XPRESS_SCRATCH);
			if (!animAssoc)
				animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_ROAD_CROSS);

			if (animAssoc) {
				if (animAssoc->IsPartial()) {
					animAssoc->blendDelta = -8.0f;
					animAssoc->flags |= ASSOC_DELETEFADEDOUT;
				} else {
					animAssoc->flags |= ASSOC_DELETEFADEDOUT;
					CAnimManager::BlendAnimation(GetClump(), m_animGroup, ANIM_IDLE_STANCE, 4.0f);
				}

				if (animAssoc->animId == ANIM_TURN_180) {
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
					animToPlay = ANIM_ROAD_CROSS;
					break;
				case 1:
					animToPlay = ANIM_IDLE_TIRED;
					break;
				case 2:
					animToPlay = ANIM_XPRESS_SCRATCH;
					break;
				case 3:
					animToPlay = ANIM_TURN_180;
					break;
				default:
					break;
			}

			animAssoc = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, animToPlay, 4.0f);

			if (animToPlay == ANIM_TURN_180)
				animAssoc->SetFinishCallback(FinishedWaitCB, this);

			m_nWaitTimer = CTimer::GetTimeInMilliseconds() + CGeneral::GetRandomNumberInRange(1500, 5000);
			break;

		case WAITSTATE_LOOK_ABOUT:
			if (CTimer::GetTimeInMilliseconds() > m_nWaitTimer) {
				ClearWaitState();
				animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_IDLE_HBHB);
				if (animAssoc) {
					animAssoc->blendDelta = -8.0f;
					animAssoc->flags |= ASSOC_DELETEFADEDOUT;
				}
			}
			break;

		case WAITSTATE_PLAYANIM_HANDSUP:
			mustHaveAnim = ANIM_HANDSUP;
			
		case WAITSTATE_PLAYANIM_HANDSCOWER:
			if (mustHaveAnim == NUM_STD_ANIMS)
				mustHaveAnim = ANIM_HANDSCOWER;

			animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), mustHaveAnim);
			pedWeLook = (CPed*) m_pLookTarget;

			if ((!m_pLookTarget || !m_pLookTarget->IsPed() || pedWeLook->m_pPointGunAt)
				&& m_nPedState != PED_FLEE_ENTITY
				&& m_nPedState != PED_ATTACK
				&& CTimer::GetTimeInMilliseconds() <= m_nWaitTimer
				&& animAssoc) {

				TurnBody();
			} else {
				ClearWaitState();
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
			mustHaveAnim = ANIM_HANDSCOWER;

		case WAITSTATE_PLAYANIM_DUCK:
			if (mustHaveAnim == NUM_STD_ANIMS)
				mustHaveAnim = ANIM_DUCK_DOWN;

		case WAITSTATE_PLAYANIM_TAXI:
			if (mustHaveAnim == NUM_STD_ANIMS)
				mustHaveAnim = ANIM_IDLE_TAXI;

		case WAITSTATE_PLAYANIM_CHAT:
			if (mustHaveAnim == NUM_STD_ANIMS)
				mustHaveAnim = ANIM_IDLE_CHAT;

			if (CTimer::GetTimeInMilliseconds() > m_nWaitTimer) {
				animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), mustHaveAnim);
				if (animAssoc) {
					animAssoc->blendDelta = -4.0f;
					animAssoc->flags |= ASSOC_DELETEFADEDOUT;
				}
				ClearWaitState();
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
			animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_IDLE_TIRED);
			if (animAssoc) {
				if (CTimer::GetTimeInMilliseconds() > m_nWaitTimer) {
					animAssoc->flags |= ASSOC_DELETEFADEDOUT;
					CAnimManager::BlendAnimation(GetClump(), m_animGroup, ANIM_IDLE_STANCE, 4.0f);
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
				//TODO(MIAMI): scan for threats!
			}
			break;
		case WAITSTATE_SIT_IDLE:
			if (bTurnedAroundOnAttractor) {
				m_fRotationCur += PI;
				m_fRotationCur = CGeneral::LimitRadianAngle(m_fRotationCur);
				m_fRotationDest = m_fRotationCur;
				bTurnedAroundOnAttractor = false;
			}
			// TODO(MIAMI): scan for threats!
			if (CTimer::GetTimeInMilliseconds() > m_nWaitTimer) {
				ClearWaitState();
				SetWaitState(WAITSTATE_SIT_UP, 0);
			}
			break;
		case WAITSTATE_USE_ATM:
			if (CTimer::GetTimeInMilliseconds() > m_nWaitTimer) {
				if (m_attractor)
					GetPedAttractorManager()->BroadcastDeparture(this, m_attractor);
				ClearWaitState();
			}
			break;
		case WAITSTATE_SUN_BATHE_PRE:
		case WAITSTATE_SUN_BATHE_DOWN:
		case WAITSTATE_SUN_BATHE_IDLE:
		case WAITSTATE_RIOT:
		case WAITSTATE_FAST_FALL:
		case WAITSTATE_BOMBER:
		case WAITSTATE_STRIPPER:
		case WAITSTATE_GROUND_ATTACK:
		case WAITSTATE_LANCESITTING:
		case WAITSTATE_PLAYANIM_HANDSUP_SIMPLE:
			assert(0);
		default:
			break;
	}

	if(!m_nWaitState)
		RestoreHeadingRate();
}

bool
CPed::Seek(void)
{
	float distanceToCountItDone = m_distanceToCountSeekDone;
	eMoveState nextMove = PEDMOVE_NONE;

	if (m_objective != OBJECTIVE_ENTER_CAR_AS_DRIVER) {

		if (m_nPedState != PED_EXIT_TRAIN && m_nPedState != PED_ENTER_TRAIN && m_nPedState != PED_SEEK_IN_BOAT &&
			m_objective != OBJECTIVE_ENTER_CAR_AS_PASSENGER && m_objective != OBJECTIVE_SOLICIT && !bDuckAndCover) {
			
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

		if (m_objective == OBJECTIVE_FOLLOW_PED_IN_FORMATION) {

			if (m_pedInObjective->m_nMoveState != PEDMOVE_STILL)
				nextMove = m_pedInObjective->m_nMoveState;
		} else
			nextMove = PEDMOVE_WALK;

	} else if (m_objective != OBJECTIVE_FOLLOW_PED_IN_FORMATION) {

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

	if ((m_objective != OBJECTIVE_FOLLOW_PED_IN_FORMATION || m_pedInObjective->m_nMoveState == PEDMOVE_STILL) && m_nMoveState != PEDMOVE_STILL) {
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

bool
CPed::SeekFollowingPath(CVector *unused)
{
	return m_nCurPathNode <= m_nPathNodes && m_nPathNodes;
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

			if (m_pNextPathNode && CTimer::GetTimeInMilliseconds() > m_standardTimer)  {

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
				m_standardTimer = CTimer::GetTimeInMilliseconds() + 2000;
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

	double damagingThingPriorityMult = (double)(m_collidingThingTimer - CTimer::GetTimeInMilliseconds()) * 2.0 / 2500;

	if (damagingThingPriorityMult <= 1.5) {

		double angleToFleeEntity = CGeneral::GetRadianAngleBetweenPoints(
			GetPosition().x,
			GetPosition().y,
			m_collidingEntityWhileFleeing->GetPosition().x,
			m_collidingEntityWhileFleeing->GetPosition().y);
		angleToFleeEntity = CGeneral::LimitRadianAngle(angleToFleeEntity);

		// It includes projectiles, but is everything collides with us included?
		double angleToFleeDamagingThing = CGeneral::GetRadianAngleBetweenPoints(
			m_vecDamageNormal.x,
			m_vecDamageNormal.y,
			0.0f,
			0.0f);
		angleToFleeDamagingThing = CGeneral::LimitRadianAngle(angleToFleeDamagingThing);

		if (angleToFleeEntity - PI > angleToFleeDamagingThing)
			angleToFleeDamagingThing += TWOPI;
		else if (PI + angleToFleeEntity < angleToFleeDamagingThing)
			angleToFleeDamagingThing -= TWOPI;

		if (damagingThingPriorityMult <= 1.0f) {
			// Range [0.0, 1.0]

			float angleToFleeBoth = (angleToFleeDamagingThing + angleToFleeEntity) * 0.5f;

			if (m_fRotationDest - PI > angleToFleeBoth)
				angleToFleeBoth += TWOPI;
			else if (PI + m_fRotationDest < angleToFleeBoth)
				angleToFleeBoth -= TWOPI;
	
			m_fRotationDest = (1.0f - damagingThingPriorityMult) * m_fRotationDest + damagingThingPriorityMult * angleToFleeBoth;
		} else {
			// Range (1.0, 1.5]

			double adjustedMult = (damagingThingPriorityMult - 1.0f) * 2.0f;
			m_fRotationDest = angleToFleeEntity * (1.0 - adjustedMult) + adjustedMult * angleToFleeDamagingThing;
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

CVector
CPed::GetFormationPosition(void)
{
	CPed *referencePed = m_pedInObjective;

	if (referencePed->m_nPedState == PED_DEAD) {
		CPed *referencePedOfReference = referencePed->m_pedInObjective;
		if (!referencePedOfReference) {
			m_pedInObjective = nil;
			return GetPosition();
		}
		m_pedInObjective = referencePed = referencePedOfReference;
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
	return formationOffset + referencePed->GetPosition();
}

void
CPed::GetNearestDoor(CVehicle *veh, CVector &posToOpen)
{
	CVector *enterOffset = nil;
	if (m_vehEnterType == CAR_DOOR_LF && veh->pDriver
		|| m_vehEnterType == CAR_DOOR_RF && veh->pPassengers[0]
		|| m_vehEnterType == CAR_DOOR_LR && veh->pPassengers[1]
		|| m_vehEnterType == CAR_DOOR_RR && veh->pPassengers[2])
	{
		enterOffset = &vecPedQuickDraggedOutCarAnimOffset;
	}

	CVector lfPos = GetPositionToOpenCarDoor(veh, CAR_DOOR_LF);
	CVector rfPos = GetPositionToOpenCarDoor(veh, CAR_DOOR_RF);

	// Right front door is closer
	if ((lfPos - GetPosition()).MagnitudeSqr2D() >= (rfPos - GetPosition()).MagnitudeSqr2D()) {

		if (veh->IsRoomForPedToLeaveCar(CAR_DOOR_RF, enterOffset)) {

			CPed *rfPassenger = veh->pPassengers[0];
			if (!rfPassenger
				|| rfPassenger->m_leader != this && !rfPassenger->bDontDragMeOutCar && (veh->VehicleCreatedBy != MISSION_VEHICLE || m_objective != OBJECTIVE_ENTER_CAR_AS_DRIVER)
				|| veh->IsRoomForPedToLeaveCar(CAR_DOOR_LF, enterOffset) == 0) {

				if ((veh->m_nGettingInFlags & CAR_DOOR_FLAG_RF) == 0
					|| veh->IsRoomForPedToLeaveCar(CAR_DOOR_LF, enterOffset) == 0) {
					m_vehEnterType = CAR_DOOR_RF;
					posToOpen = rfPos;
					return;
				}
			}
		} else {
			if (!veh->IsRoomForPedToLeaveCar(CAR_DOOR_LF, enterOffset))
				return;
		}
		m_vehEnterType = CAR_DOOR_LF;
		posToOpen = lfPos;
		return;
	}

	if (veh->IsRoomForPedToLeaveCar(CAR_DOOR_LF, enterOffset)) {
		m_vehEnterType = CAR_DOOR_LF;
		posToOpen = lfPos;
		return;
	}

	if (veh->IsRoomForPedToLeaveCar(CAR_DOOR_RF, enterOffset)) {
		m_vehEnterType = CAR_DOOR_RF;
		posToOpen = rfPos;
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
			m_vehEnterType = CAR_DOOR_RF;
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
	m_vehEnterType = CAR_DOOR_RF;
	if (lrPosDist.MagnitudeSqr() < nextToCompare.MagnitudeSqr()) {
		m_vehEnterType = CAR_DOOR_LR;
		posToOpen = lrPos;
		nextToCompare = lrPosDist;
	}

	if (rrPosDist.MagnitudeSqr() < nextToCompare.MagnitudeSqr()) {
		m_vehEnterType = CAR_DOOR_RR;
		posToOpen = rrPos;
	}
	return canEnter;
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

uint8
CPed::GetPedRadioCategory(uint32 modelIndex)
{
	// TODO(MIAMI): remove this function and use modelinfo for radio
	return 1;
}

// --MIAMI: Done
int
CPed::GetWeaponSlot(eWeaponType weaponType)
{
	return CWeaponInfo::GetWeaponInfo(weaponType)->m_nWeaponSlot;
}

void
CPed::GoToNearestDoor(CVehicle *veh)
{
	CVector posToOpen;
	GetNearestDoor(veh, posToOpen);
	SetSeek(posToOpen, 0.5f);
	SetMoveState(PEDMOVE_RUN);
}

bool
CPed::HaveReachedNextPointOnRoute(float distToCountReached)
{
	if ((m_nextRoutePointPos - GetPosition()).Magnitude2D() >= distToCountReached)
		return false;

	m_routePointsPassed += m_routePointsBeingPassed;
	return true;
}

void
CPed::Idle(void)
{
	CVehicle *veh = m_pMyVehicle;
	if (veh && veh->m_nGettingOutFlags && m_vehEnterType) {

		if (veh->m_nGettingOutFlags & GetCarDoorFlag(m_vehEnterType)) {

			if (m_objective != OBJECTIVE_KILL_CHAR_ON_FOOT) {

				CVector doorPos = GetPositionToOpenCarDoor(veh, m_vehEnterType);
				CVector doorDist = GetPosition() - doorPos;

				if (doorDist.MagnitudeSqr() < sq(0.5f)) {
					SetMoveState(PEDMOVE_WALK);
					return;
				}
			}
		}
	}

	CAnimBlendAssociation *armedIdleAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_IDLE_ARMED);
	CAnimBlendAssociation *unarmedIdleAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_IDLE_STANCE);
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
			CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_IDLE_ARMED, 3.0f);
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
				if (!RpAnimBlendClumpGetAssociation(GetClump(), ANIM_FALL_FALL)) {
					if (m_vecMoveSpeed.z < -0.1f)
						CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_FALL_FALL, 4.0f);
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

	CAnimBlendAssociation *fallAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_FALL_FALL);
	CAnimBlendAssociation *landAssoc;

	if (fallAssoc && bIsDrowning)
		return;

	RpAnimBlendClumpSetBlendDeltas(GetClump(), ASSOC_PARTIAL, -1000.0f);
	if (fallAssoc || m_nPedType == PEDTYPE_COP && bKnockedUpIntoAir) {
		landAssoc = CAnimManager::AddAnimation(GetClump(), ASSOCGRP_STD, ANIM_FALL_COLLAPSE);
		DMAudio.PlayOneShot(m_audioEntityId, SOUND_FALL_COLLAPSE, 1.0f);

		if (IsPlayer())
			Say(SOUND_PED_LAND);

		if (m_nPedType == PEDTYPE_COP) {
			if (bKnockedUpIntoAir)
				bKnockedUpIntoAir = false;
		}

	} else {
		landAssoc = CAnimManager::AddAnimation(GetClump(), ASSOCGRP_STD, ANIM_FALL_LAND);
		DMAudio.PlayOneShot(m_audioEntityId, SOUND_FALL_LAND, 1.0f);
	}

	landAssoc->SetFinishCallback(PedLandCB, this);
	bIsInTheAir = false;
	bIsLanding = true;
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
CPed::SetAnimOffsetForEnterOrExitVehicle(void)
{
	// FIX: If there were no translations on enter anims, there were overflows all over this function.

	int vanBlock = CAnimManager::GetAnimationBlockIndex("van");
	CStreaming::RequestAnim(vanBlock, STREAMFLAGS_DEPENDENCY);
	CStreaming::LoadAllRequestedModels(false);
	CAnimManager::AddAnimBlockRef(vanBlock);

	CAnimBlendHierarchy *enterAssoc = CAnimManager::GetAnimAssociation(ASSOCGRP_STD, ANIM_CAR_JACKED_LHS)->hierarchy;
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

	enterAssoc = CAnimManager::GetAnimAssociation(ASSOCGRP_STD, ANIM_CAR_GETIN_LHS)->hierarchy;
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

	enterAssoc = CAnimManager::GetAnimAssociation(ASSOCGRP_STD, ANIM_CAR_GETIN_LOW_LHS)->hierarchy;
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

	enterAssoc = CAnimManager::GetAnimAssociation(ASSOCGRP_STD, ANIM_CAR_QJACKED)->hierarchy;
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

	enterAssoc = CAnimManager::GetAnimAssociation(ASSOCGRP_VAN, ANIM_VAN_GETIN_L)->hierarchy;
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
#ifdef GTA_TRAIN
	enterAssoc = CAnimManager::GetAnimAssociation(ASSOCGRP_STD, ANIM_TRAIN_GETOUT)->hierarchy;
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
#endif
}

void
CPed::InvestigateEvent(void)
{
	CAnimBlendAssociation *animAssoc;
	AnimationId animToPlay;
	AssocGroupId animGroup;

	if (m_nWaitState == WAITSTATE_TURN180)
		return;

	if (CTimer::GetTimeInMilliseconds() > m_standardTimer) {

		if (m_standardTimer) {
			if (m_eventType < EVENT_ASSAULT_NASTYWEAPON)
				SetWaitState(WAITSTATE_TURN180, nil);

			m_standardTimer = 0;
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
					animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_ROAD_CROSS);

					if (animAssoc) {
						animAssoc->blendDelta = -8.0f;
						animAssoc->flags |= ASSOC_DELETEFADEDOUT;
						if (m_pEventEntity)
							SetLookFlag(m_pEventEntity, true);

						SetLookTimer(CGeneral::GetRandomNumberInRange(1500, 4000));

					} else if (CGeneral::GetRandomNumber() & 3) {
						ClearLookFlag();
						CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_ROAD_CROSS, 4.0f);

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
					animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_IDLE_CAM);
					if (!animAssoc)
						animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_IDLE_STANCE);

					if (animAssoc && animAssoc->animId == ANIM_IDLE_CAM) {
						CAnimManager::BlendAnimation(GetClump(), m_animGroup, ANIM_IDLE_STANCE, 14.0f);
						SetLookTimer(CGeneral::GetRandomNumberInRange(1000, 2500));

					} else if (CGeneral::GetRandomNumber() & 3) {
						CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_IDLE_CAM, 4.0f);
						SetLookTimer(CGeneral::GetRandomNumberInRange(2500, 5000));
						Say(SOUND_PED_CHAT_EVENT);

					} else {
						m_standardTimer = 0;
					}

				} else if (CTimer::GetTimeInMilliseconds() > m_lookTimer) {
					animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_IDLE_STANCE);
					if (!animAssoc)
						animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_IDLE_HBHB);

					if (!animAssoc)
						animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_XPRESS_SCRATCH);

					if (animAssoc && animAssoc->animId == ANIM_IDLE_STANCE) {
						if (CGeneral::GetRandomNumber() & 1)
							animToPlay = ANIM_IDLE_HBHB;
						else
							animToPlay = ANIM_XPRESS_SCRATCH;

						CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, animToPlay, 4.0f);
						SetLookTimer(CGeneral::GetRandomNumberInRange(1000, 2500));

					} else if (animAssoc && animAssoc->animId == ANIM_IDLE_HBHB) {
						animAssoc->blendDelta = -8.0f;
						animAssoc->flags |= ASSOC_DELETEFADEDOUT;
						if (CGeneral::GetRandomNumber() & 1) {
							animToPlay = ANIM_IDLE_STANCE;
							animGroup = m_animGroup;
						} else {
							animToPlay = ANIM_XPRESS_SCRATCH;
							animGroup = ASSOCGRP_STD;
						}

						CAnimManager::BlendAnimation(GetClump(), animGroup, animToPlay, 4.0f);
						SetLookTimer(CGeneral::GetRandomNumberInRange(1000, 2500));

					} else {
						if (CGeneral::GetRandomNumber() & 1) {
							animToPlay = ANIM_IDLE_STANCE;
							animGroup = m_animGroup;
						} else {
							animToPlay = ANIM_IDLE_HBHB;
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
						animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_ROAD_CROSS);

						if (animAssoc) {
							animAssoc->blendDelta = -8.0f;
							animAssoc->flags |= ASSOC_DELETEFADEDOUT;
							if (m_eventType == EVENT_ICECREAM)
								animToPlay = ANIM_IDLE_CHAT;
							else
								animToPlay = ANIM_XPRESS_SCRATCH;
							CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, animToPlay,4.0f);
							SetLookTimer(CGeneral::GetRandomNumberInRange(2000, 5000));

						} else {
							animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_IDLE_CHAT);
							if (animAssoc) {
								animAssoc->blendDelta = -8.0f;
								animAssoc->flags |= ASSOC_DELETEFADEDOUT;
								ClearInvestigateEvent();
							} else {
								animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_XPRESS_SCRATCH);
								if (animAssoc) {
									animAssoc->blendDelta = -8.0f;
									animAssoc->flags |= ASSOC_DELETEFADEDOUT;
								}
								ClearInvestigateEvent();
							}
						}
					} else {
						CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_ROAD_CROSS, 4.0f);
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
		if (distSqr > 1.44f) {
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

bool
CPed::IsPedDoingDriveByShooting(void)
{
#ifdef FIX_BUGS
	if (FindPlayerPed() == this && CWeaponInfo::GetWeaponInfo(GetWeapon()->m_eWeaponType)->m_nWeaponSlot == 5) {
#else
	if (FindPlayerPed() == this && GetWeapon()->m_eWeaponType == WEAPONTYPE_UZI) {
#endif
		if (TheCamera.Cams[TheCamera.ActiveCam].LookingLeft || TheCamera.Cams[TheCamera.ActiveCam].LookingRight)
			return true;
	}
	return false;
}

bool
CPed::IsPedShootable(void)
{
	return m_nPedState <= PED_STATES_NO_ST;
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
		damageDir = CPed::GetLocalDirection(-m_vecMoveSpeed);
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
#ifdef TOGGLEABLE_BETA_FEATURES
			if (bMakePedsRunToPhonesToReportCrimes)
				m_ped_flagI40 = true;
#endif
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
		CPed::InflictDamage(car, killMethod, 1000.0f, pieceToDamage, damageDir);

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
	
		CPed::InflictDamage(car, WEAPONTYPE_RAMMEDBYCAR, damage, PEDPIECE_TORSO, fallDirection);
		CPed::SetFall(1000, (AnimationId)(fallDirection + ANIM_KO_SKID_FRONT), true);

		if ((m_nPedState == PED_FALL || m_nPedState == PED_DIE || m_nPedState == PED_DEAD)
			&& !m_pCollidingEntity &&
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

void
CPed::Look(void)
{
	TurnBody();
}

bool
CPed::LookForInterestingNodes(void)
{
	CBaseModelInfo *model;
	CPtrNode *ptrNode;
	CVector effectPos;
	CVector effectDist;
	C2dEffect *effect;
	CMatrix *objMat;

	if ((CTimer::GetFrameCounter() + (m_randomSeed % 256)) & 7 || CTimer::GetTimeInMilliseconds() <= m_standardTimer) {
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
							effectPos = veh->GetMatrix() * effect->pos;
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
							effectPos = obj->GetMatrix() * effect->pos;
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
							effectPos = building->GetMatrix() * effect->pos;
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
							effectPos = building->GetMatrix() * effect->pos;
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
		m_standardTimer = CTimer::GetTimeInMilliseconds() + 2000;
		SetLookFlag(angleToFace, true);
		SetLookTimer(1000);
		return false;
	}

	switch (effect->attractor.flags) {
		case ATTRACTORFLAG_ICECREAM:
			SetInvestigateEvent(EVENT_ICECREAM, CVector2D(effectPos), 0.1f, 15000, angleToFace);
			break;
		case ATTRACTORFLAG_STARE:
			SetInvestigateEvent(EVENT_SHOPSTALL, CVector2D(effectPos), 1.0f,
								CGeneral::GetRandomNumberInRange(8000, 10 * effect->attractor.probability + 8500),
								angleToFace);
			break;
		default:
			return true;
	}
	return true;
}

void
CPed::SetInvestigateEvent(eEventType event, CVector2D pos, float distanceToCountDone, uint16 time, float angle)
{
	if (!IsPedInControl() || CharCreatedBy == MISSION_CHAR)
		return;

	SetStoredState();
	bFindNewNodeAfterStateRestore = false;
	m_nPedState = PED_INVESTIGATE;
	m_standardTimer = CTimer::GetTimeInMilliseconds() + time;
	m_eventType = event;
	m_eventOrThreat = pos;
	m_distanceToCountSeekDone = distanceToCountDone;
	m_fAngleToEvent = angle;

	if (m_eventType >= EVENT_ICECREAM)
		m_lookTimer = 0;
	else
		CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_HANDSCOWER, 4.0f);

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
CPed::MakeTyresMuddySectorList(CPtrList &list)
{
	for (CPtrNode *node = list.first; node; node = node->next) {
		CVehicle *veh = (CVehicle*)node->item;
		if (veh->IsCar() && veh->m_scanCode != CWorld::GetCurrentScanCode()) {
			veh->m_scanCode = CWorld::GetCurrentScanCode();

			if (Abs(GetPosition().x - veh->GetPosition().x) < 10.0f) {

				if (Abs(GetPosition().y - veh->GetPosition().y) < 10.0f
					&& veh->m_vecMoveSpeed.MagnitudeSqr2D() > 0.05f) {

					for(int wheel = 0; wheel < 4; wheel++) {

						if (!((CAutomobile*)veh)->m_aWheelSkidmarkBloody[wheel]
							&& ((CAutomobile*)veh)->m_aSuspensionSpringRatio[wheel] < 1.0f) {

							CColModel *vehCol = veh->GetModelInfo()->GetColModel();
							CVector approxWheelOffset;
							switch (wheel) {
								case 0:
									approxWheelOffset = CVector(-vehCol->boundingBox.max.x, vehCol->boundingBox.max.y, 0.0f);
									break;
								case 1:
									approxWheelOffset = CVector(-vehCol->boundingBox.max.x, vehCol->boundingBox.min.y, 0.0f);
									break;
								case 2:
									approxWheelOffset = CVector(vehCol->boundingBox.max.x, vehCol->boundingBox.max.y, 0.0f);
									break;
								case 3:
									approxWheelOffset = CVector(vehCol->boundingBox.max.x, vehCol->boundingBox.min.y, 0.0f);
									break;
								default:
									break;
							}

							// I hope so
							CVector wheelPos = veh->GetMatrix() * approxWheelOffset;
							if (Abs(wheelPos.z - GetPosition().z) < 2.0f) {

								if ((wheelPos - GetPosition()).MagnitudeSqr2D() < 1.0f) {
									if (CGame::nastyGame) {
										((CAutomobile*)veh)->m_aWheelSkidmarkBloody[wheel] = true;
										DMAudio.PlayOneShot(veh->m_audioEntityId, SOUND_SPLATTER, 0.0f);
									}
									veh->ApplyMoveForce(CVector(0.0f, 0.0f, 50.0f));
									
									CVector vehAndWheelDist = wheelPos - veh->GetPosition();
									veh->ApplyTurnForce(CVector(0.0f, 0.0f, 50.0f), vehAndWheelDist);

									if (veh == FindPlayerVehicle()) {
										CPad::GetPad(0)->StartShake(300, 70);
									}
								}
							}
						}
					}
				}
			}
		}
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

			CAnimBlendAssociation *fuckUAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_FUCKU);
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
			((CPed*)m_pLookTarget)->m_pedIK.GetComponentPosition((RwV3d*) &lookPos, PED_MID);
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
		AnimationId animToPlay = NUM_STD_ANIMS;

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
						animToPlay = ANIM_FIGHT_PPUNCH;
				} else {
					animToPlay = ANIM_FUCKU;
				}
			} else if (m_pedStats->m_temper > 49 || m_pLookTarget->GetModelIndex() == MI_POLICE) {
				animToPlay = ANIM_FUCKU;
			}
		} else if (notRocketLauncher && (CGeneral::GetRandomNumber() & 1)) {
			animToPlay = ANIM_FUCKU;
		}

		if (animToPlay != NUM_STD_ANIMS) {
			CAnimBlendAssociation *newAssoc = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, animToPlay, 4.0f);

			if (newAssoc) {
				newAssoc->flags |= ASSOC_FADEOUTWHENDONE;
				newAssoc->flags |= ASSOC_DELETEFADEDOUT;
				if (newAssoc->animId == ANIM_FUCKU)
					newAssoc->SetDeleteCallback(FinishFuckUCB, this);
			}
		}
		bShakeFist = false;
		return;
	}

	if (999999.0f == m_fLookDirection) {
		ClearLookFlag();
		return;
	}

	if (!m_pedIK.LookInDirection(m_fLookDirection, 0.0f)) {
		if (!bKeepTryingToLook) {
			ClearLookFlag();
			return;
		}
	}
}

void
FinishFuckUCB(CAnimBlendAssociation *animAssoc, void *arg)
{
	CPed *ped = (CPed*)arg;

	if (animAssoc->animId == ANIM_FUCKU && ped->GetWeapon()->m_eWeaponType == WEAPONTYPE_UNARMED)
			ped->RemoveWeaponModel(0);
}

void
CPed::Pause(void)
{
	m_moved = CVector2D(0.0f, 0.0f);
	if (CTimer::GetTimeInMilliseconds() > m_leaveCarTimer)
		ClearPause();
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

	switch (ped->m_vehEnterType) {
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
				ped->m_pVehicleAnim = CAnimManager::AddAnimation(ped->GetClump(), ASSOCGRP_VAN, ANIM_VAN_GETIN);
			} else if (itsBus) {
				ped->m_pVehicleAnim = CAnimManager::AddAnimation(ped->GetClump(), ASSOCGRP_COACH, ANIM_COACH_IN_R);
#ifdef FIX_BUGS
			} else if (itsLow) {
				ped->m_pVehicleAnim = CAnimManager::AddAnimation(ped->GetClump(), ASSOCGRP_STD, ANIM_CAR_GETIN_LOW_RHS);
#endif
			} else {
				ped->m_pVehicleAnim = CAnimManager::AddAnimation(ped->GetClump(), ASSOCGRP_STD, ANIM_CAR_GETIN_RHS);
			}
		} else if (itsVan) {
			ped->m_pVehicleAnim = CAnimManager::AddAnimation(ped->GetClump(), ASSOCGRP_VAN, ANIM_VAN_GETIN_L);
		} else if (itsBus) {
			ped->m_pVehicleAnim = CAnimManager::AddAnimation(ped->GetClump(), ASSOCGRP_COACH, ANIM_COACH_IN_L);
#ifdef FIX_BUGS
		} else if (itsLow) {
			ped->m_pVehicleAnim = CAnimManager::AddAnimation(ped->GetClump(), ASSOCGRP_STD, ANIM_CAR_GETIN_LOW_LHS);
#endif
		} else {
			ped->m_pVehicleAnim = CAnimManager::AddAnimation(ped->GetClump(), ASSOCGRP_STD, ANIM_CAR_GETIN_LHS);
		}
		ped->m_pVehicleAnim->SetFinishCallback(PedAnimGetInCB, ped);

	} else if (veh->CanPedOpenLocks(ped)) {

		veh->AutoPilot.m_nCruiseSpeed = 0;
		if (enterDoor != DOOR_FRONT_LEFT && enterDoor != DOOR_REAR_LEFT) {
			if (itsVan) {
				ped->m_pVehicleAnim = CAnimManager::AddAnimation(ped->GetClump(), ASSOCGRP_VAN, ANIM_VAN_OPEN);
			} else if (itsBus) {
				ped->m_pVehicleAnim = CAnimManager::AddAnimation(ped->GetClump(), ASSOCGRP_COACH, ANIM_COACH_OPEN_R);
			} else {
				ped->m_pVehicleAnim = CAnimManager::AddAnimation(ped->GetClump(), ASSOCGRP_STD, ANIM_CAR_OPEN_RHS);
			}
		} else if (itsVan) {
			ped->m_pVehicleAnim = CAnimManager::AddAnimation(ped->GetClump(), ASSOCGRP_VAN, ANIM_VAN_OPEN_L);
		} else if (itsBus) {
			ped->m_pVehicleAnim = CAnimManager::AddAnimation(ped->GetClump(), ASSOCGRP_COACH, ANIM_COACH_OPEN_L);
		} else {

			if (ped->m_objective == OBJECTIVE_ENTER_CAR_AS_DRIVER && veh->pDriver) {

				if (!veh->bLowVehicle
					&& veh->pDriver->CharCreatedBy != MISSION_CHAR
					&& veh->pDriver->m_nPedState == PED_DRIVING) {

					ped->m_pVehicleAnim = CAnimManager::AddAnimation(ped->GetClump(), ASSOCGRP_STD, ANIM_CAR_QJACK);
					ped->m_pVehicleAnim->SetFinishCallback(PedAnimGetInCB, ped);
					veh->pDriver->SetBeingDraggedFromCar(veh, ped->m_vehEnterType, true);

					if (veh->pDriver->IsGangMember())
						veh->pDriver->RegisterThreatWithGangPeds(ped);
					return;
				}
			}
			ped->m_pVehicleAnim = CAnimManager::AddAnimation(ped->GetClump(), ASSOCGRP_STD, ANIM_CAR_OPEN_LHS);
		}
		ped->m_pVehicleAnim->SetFinishCallback(PedAnimDoorOpenCB, ped);

	} else {
		if (enterDoor != DOOR_FRONT_LEFT && enterDoor != DOOR_REAR_LEFT)
			ped->m_pVehicleAnim = CAnimManager::AddAnimation(ped->GetClump(), ASSOCGRP_STD, ANIM_CAR_DOORLOCKED_RHS);
		else
			ped->m_pVehicleAnim = CAnimManager::AddAnimation(ped->GetClump(), ASSOCGRP_STD, ANIM_CAR_DOORLOCKED_LHS);

		ped->bCancelEnteringCar = true;
		ped->m_pVehicleAnim->SetFinishCallback(PedAnimDoorOpenCB, ped);
	}
}

void
CPed::ProcessControl(void)
{
	CColPoint foundCol;
	CEntity *foundEnt = nil;

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
	bIsDrowning = false;
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
			SetDie(ANIM_KO_SHOT_FRONT1, 4.0f, 0.0f);

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
		} else if (!GetPedAttractorManager()->IsInQueue(this, m_attractor)) {
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

#ifdef VC_PED_PORTS
					if (m_nPedState == PED_FOLLOW_PATH) {
						if (DotProduct(m_vecDamageNormal, GetForward()) < -0.866f && CanPedJumpThis(collidingEnt, &m_vecDamageNormal)) {
							SetJump();
						}
						break;
					}
#endif
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
								CAnimBlendAssociation *runAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_RUN);
								if (!runAssoc)
									runAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_SPRINT);

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

								uint8 currentDir = floorf((PI + m_fRotationCur) / DEGTORAD(45.0f));
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
								SetFall(1000, (AnimationId)(dir + ANIM_KO_SKID_FRONT), false);

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

								CPed::SetDirectionToWalkAroundObject(collidingVeh);
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
								SetFall(1000, (AnimationId)(dir + ANIM_KO_SKID_FRONT), false);
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
				if (m_nPedStateTimer <= 1000 && m_nPedStateTimer) {
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
						if ((obstacleForFlyingZ <= feetZ || obstacleForFlyingOtherDirZ >= 500.0f) && (obstacleForFlyingZ <= feetZ || obstacleForFlyingOtherDirZ <= feetZ)) {
							if (obstacleForFlyingOtherDirZ > feetZ && obstacleForFlyingZ < 499.0f)
								flyDir = 2;
						} else {
							flyDir = 1;
						}

						if (flyDir != 0 && !bHeadStuckInCollision) {
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

						GetMatrix().GetPosition() += 0.25f * offsetToCheck;

						m_fRotationCur = CGeneral::GetRadianAngleBetweenPoints(offsetToCheck.x, offsetToCheck.y, 0.0f, 0.0f);
						m_fRotationCur = CGeneral::LimitRadianAngle(m_fRotationCur);
						m_fRotationDest = m_fRotationCur;
						SetHeading(m_fRotationCur);

						if (m_nPedState != PED_FALL && !bIsPedDieAnimPlaying) {
							CPed::SetFall(1000, ANIM_KO_SKID_BACK, true);
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
						CAnimBlendAssociation *jumpAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_JUMP_GLIDE);

						if (!jumpAssoc)
							jumpAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_FALL_GLIDE);

						if (jumpAssoc) {
							jumpAssoc->blendDelta = -3.0f;
							jumpAssoc->flags |= ASSOC_DELETEFADEDOUT;
						}
						if (m_nPedState == PED_JUMP)
							m_nPedState = PED_IDLE;
					} else {
						CAnimBlendAssociation *jumpAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_JUMP_GLIDE);

						if (!jumpAssoc)
							jumpAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_FALL_GLIDE);

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
						if (!bHeadStuckInCollision || FEET_OFFSET + foundCol.point.z < GetPosition().z) {
							GetMatrix().GetPosition().z = FEET_OFFSET + foundCol.point.z;
							GetMatrix().UpdateRW();
							if (bHeadStuckInCollision)
								bHeadStuckInCollision = false;
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
				SetObjective(OBJECTIVE_LEAVE_VEHICLE, m_pMyVehicle);
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
			RequestDelayedWeapon();
			PlayFootSteps();
			if (m_nPedState != PED_DEAD) {
				CalculateNewVelocity();
				CalculateNewOrientation();
			}
			UpdatePosition();
			if (IsPedInControl() && !bIsStanding && !m_pDamageEntity && CheckIfInTheAir()) {
				SetInTheAir();
#ifdef VC_PED_PORTS
				bHeadStuckInCollision = false;
#endif
			}
#ifdef VC_PED_PORTS
			if (bHeadStuckInCollision) {
				CVector posToCheck = GetPosition();
				posToCheck.z += 0.9f;
				if (!CWorld::TestSphereAgainstWorld(posToCheck, 0.2f, this, true, true, false, true, false, false))
					bHeadStuckInCollision = false;
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
				CAnimBlendAssociation *idleAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_IDLE_ARMED);
				if(idleAssoc) {
					idleAssoc->blendDelta = -8.0f;
					idleAssoc->flags |= ASSOC_DELETEFADEDOUT;
				}
			}

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
				case PED_SEEK_POS:
				case PED_SEEK_ENTITY:
				case PED_PURSUE:
				case PED_SNIPER_MODE:
				case PED_ROCKET_ODE:
				case PED_DUMMY:
				case PED_FACE_PHONE:
				case PED_MAKE_CALL:
				case PED_MUG:
				case PED_AI_CONTROL:
				case PED_FOLLOW_ROUTE:
				case PED_CPR:
				case PED_SOLICIT:
				case PED_BUY_ICECREAM:
				case PED_STEP_AWAY:
				case PED_UNKNOWN:
				case PED_STATES_NO_AI:
				case PED_JUMP:
				case PED_STAGGER:
				case PED_DIVE_AWAY:
				case PED_STATES_NO_ST:
				case PED_ARREST_PLAYER:
				case PED_PASSENGER:
				case PED_TAXI_PASSENGER:
				case PED_OPEN_DOOR:
				case PED_DEAD:
				case PED_DRAG_FROM_CAR:
				case PED_EXIT_CAR:
				case PED_STEAL_CAR:
					break;
				case PED_ENTER_CAR:
				case PED_CARJACK:
				{
#ifdef CANCELLABLE_CAR_ENTER
					if (!IsPlayer() || !m_pVehicleAnim)
						break;

					CPad *pad = CPad::GetPad(0);

					if (pad->ArePlayerControlsDisabled())
						break;

					int vehAnim = m_pVehicleAnim->animId;

					int16 padWalkX = pad->GetPedWalkLeftRight();
					int16 padWalkY = pad->GetPedWalkUpDown();
					if (Abs(padWalkX) > 0.0f || Abs(padWalkY) > 0.0f) {
						if (vehAnim == ANIM_CAR_OPEN_LHS || vehAnim == ANIM_CAR_OPEN_RHS || vehAnim == ANIM_COACH_OPEN_L || vehAnim == ANIM_COACH_OPEN_R ||
							vehAnim == ANIM_VAN_OPEN_L || vehAnim == ANIM_VAN_OPEN) {
							bCancelEnteringCar = true;
						} else if (vehAnim == ANIM_CAR_QJACK) {
							if (m_pVehicleAnim->GetTimeLeft() > 0.69f && m_pVehicleAnim->GetTimeLeft() < 0.72f) {
								QuitEnteringCar();
								RestorePreviousObjective();
							}
						}
					}
#endif
					break;
				}
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
					CPed::FollowPath();
					break;
				case PED_PAUSE:
					CPed::Pause();
					break;
				case PED_ATTACK:
					CPed::Attack();
					break;
				case PED_FIGHT:
					CPed::Fight();
					break;
				case PED_CHAT:
					CPed::Chat();
					break;
				case PED_AIM_GUN:
					if (m_pPointGunAt && m_pPointGunAt->IsPed() && ((CPed*)m_pPointGunAt)->CanSeeEntity(this, CAN_SEE_ENTITY_ANGLE_THRESHOLD * 2)) {
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

					if (m_pMyVehicle->pDriver != this) {
						LookForSexyPeds();
						LookForSexyCars();
						break;
					}

					// TODO(Miami): Start KILL_CHAR_ON_BOAT objective

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

					// TODO(Miami): This part moved to DriveVehicle in VC
					float steerAngle = m_pMyVehicle->m_fSteerAngle;
					CAnimBlendAssociation *lDriveAssoc;
					CAnimBlendAssociation *rDriveAssoc;
					CAnimBlendAssociation *lbAssoc;
					CAnimBlendAssociation *sitAssoc;
					if (m_pMyVehicle->IsBoat() && !(m_pMyVehicle->pHandling->Flags & HANDLING_SIT_IN_BOAT)) {
						sitAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_DRIVE_BOAT);

						if (!sitAssoc || sitAssoc->blendAmount < 1.0f) {
							break;
						}

						lDriveAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_DRIVE_BOAT_L);
						rDriveAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_DRIVE_BOAT_R);
						lbAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_BOAT_LB);
					} else if (m_pMyVehicle->bLowVehicle) {
						sitAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_CAR_LSIT);

						if (!sitAssoc || sitAssoc->blendAmount < 1.0f) {
							break;
						}

						lDriveAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_DRIVE_LOW_L);
						lbAssoc = nil;
						rDriveAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_DRIVE_LOW_R);
					} else {
						sitAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_CAR_SIT);

						if (!sitAssoc || sitAssoc->blendAmount < 1.0f) {
							break;
						}

						lDriveAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_DRIVE_L);
						rDriveAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_DRIVE_R);
						lbAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_CAR_LB);
					}

					if (lbAssoc &&
						TheCamera.Cams[TheCamera.ActiveCam].Mode == CCam::MODE_1STPERSON
						&& TheCamera.Cams[TheCamera.ActiveCam].DirectionWasLooking == LOOKING_LEFT) {
						lbAssoc->blendDelta = -1000.0f;
					}

					CAnimBlendAssociation *driveByAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_DRIVEBY_L);
					if (!driveByAssoc)
						driveByAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_DRIVEBY_R);
					if (!driveByAssoc)
						driveByAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_DRIVEBY_LOW_L);
					if (!driveByAssoc)
						driveByAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_DRIVEBY_LOW_R);

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
							else if (m_pMyVehicle->IsBoat() && !(m_pMyVehicle->pHandling->Flags & HANDLING_SIT_IN_BOAT))
								CAnimManager::AddAnimation(GetClump(), ASSOCGRP_STD, ANIM_DRIVE_BOAT_R);
							else if (m_pMyVehicle->bLowVehicle)
								CAnimManager::AddAnimation(GetClump(), ASSOCGRP_STD, ANIM_DRIVE_LOW_R);
							else
								CAnimManager::AddAnimation(GetClump(), ASSOCGRP_STD, ANIM_DRIVE_R);

						} else {
							if (rDriveAssoc)
								rDriveAssoc->blendAmount = 0.0f;

							if (lDriveAssoc)
								lDriveAssoc->blendAmount = clamp(steerAngle * 100.0f / 61.0f, 0.0f, 1.0f);
							else if (m_pMyVehicle->IsBoat() && !(m_pMyVehicle->pHandling->Flags & HANDLING_SIT_IN_BOAT))
								CAnimManager::AddAnimation(GetClump(), ASSOCGRP_STD, ANIM_DRIVE_BOAT_L);
							else if (m_pMyVehicle->bLowVehicle)
								CAnimManager::AddAnimation(GetClump(), ASSOCGRP_STD, ANIM_DRIVE_LOW_L);
							else
								CAnimManager::AddAnimation(GetClump(), ASSOCGRP_STD, ANIM_DRIVE_L);
						}

						if (lbAssoc)
							lbAssoc->blendDelta = -4.0f;
					} else {

						if ((TheCamera.Cams[TheCamera.ActiveCam].Mode != CCam::MODE_1STPERSON
							|| TheCamera.Cams[TheCamera.ActiveCam].DirectionWasLooking != LOOKING_LEFT)
							&& (!lbAssoc || lbAssoc->blendAmount < 1.0f && lbAssoc->blendDelta <= 0.0f)) {

							if(m_pMyVehicle->IsBoat() && !(m_pMyVehicle->pHandling->Flags & HANDLING_SIT_IN_BOAT))
								CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_BOAT_LB, 4.0f);
							else
								CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_CAR_LB, 4.0f);
						}
					}

					// TODO(Miami): This part belongs to DriveVehicle
					if (!m_pMyVehicle)
						return;
					break;
				}
				case PED_DIE:
					Die();
					break;
				case PED_HANDS_UP:
					if (m_pedStats->m_temper <= 50) {
						if (!RpAnimBlendClumpGetAssociation(GetClump(), ANIM_HANDSCOWER)) {
							CAnimManager::AddAnimation(GetClump(), ASSOCGRP_STD, ANIM_HANDSCOWER);
							Say(SOUND_PED_HANDS_COWER);
						}
					} else if (!RpAnimBlendClumpGetAssociation(GetClump(), ANIM_HANDSUP)) {
						CAnimManager::AddAnimation(GetClump(), ASSOCGRP_STD, ANIM_HANDSUP);
						Say(SOUND_PED_HANDS_UP);
					}
					break;
				default: break;
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
	} else
		ServiceTalking();
}

void
CPed::SetInTheAir(void)
{
	if (bIsInTheAir)
		return;

	bIsInTheAir = true;
	CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_FALL_GLIDE, 4.0f);

	if (m_nPedState == PED_ATTACK) {
		ClearAttack();
		ClearPointGunAt();
	} else if (m_nPedState == PED_FIGHT) {
		EndFight(ENDFIGHT_FAST);
	}

}

// --MIAMI: Done
void
CPed::RestoreHeadPosition(void)
{
	// TODO(Miami): This is inlined CanUseTorsoWhenLooking
	bool canUseMyBody = false;
	if (m_nPedState != PED_DRIVING && m_nPedState != PED_DRAG_FROM_CAR && !bIsDucking) {
		if (m_animGroup != ASSOCGRP_SEXYWOMAN && m_animGroup != ASSOCGRP_WOMAN)
			canUseMyBody = true;
	}
	if (!canUseMyBody)
		m_pedIK.m_flags |= CPedIK::LOOKAROUND_HEAD_ONLY;

	if (m_pedIK.RestoreLookAt()) {
		bIsRestoringLook = false;
		canUseMyBody = false;
		if (m_nPedState != PED_DRIVING && m_nPedState != PED_DRAG_FROM_CAR && !bIsDucking) {
			if (m_animGroup != ASSOCGRP_SEXYWOMAN && m_animGroup != ASSOCGRP_WOMAN)
				canUseMyBody = true;
		}
		if(canUseMyBody)
			m_pedIK.m_flags &= ~CPedIK::LOOKAROUND_HEAD_ONLY;
	}
}

// --MIAMI: Done
void
CPed::PointGunAt(void)
{
	CWeaponInfo *weaponInfo = CWeaponInfo::GetWeaponInfo(GetWeapon()->m_eWeaponType);
	float animLoopStart = weaponInfo->m_fAnimLoopStart;
	CAnimBlendAssociation *weaponAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_WEAPON_FIRE);
	if (!weaponAssoc || weaponAssoc->blendDelta < 0.0f) {
		if (!!weaponInfo->m_bCrouchFire) {
			weaponAssoc = RpAnimBlendClumpGetAssociation(GetClump(), GetCrouchFireAnim(weaponInfo));
			animLoopStart = weaponInfo->m_fAnim2LoopStart;
		}
	}

	if (weaponAssoc && weaponAssoc->currentTime > animLoopStart * 0.4f) {
		weaponAssoc->SetCurrentTime(animLoopStart);
		weaponAssoc->flags &= ~ASSOC_RUNNING;

		if (bIsDucking)
			m_pedIK.m_flags &= ~CPedIK::AIMS_WITH_ARM;

		if (weaponInfo->m_bCanAimWithArm)
			m_pedIK.m_flags |= CPedIK::AIMS_WITH_ARM;
		else
			m_pedIK.m_flags &= ~CPedIK::AIMS_WITH_ARM;
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
			veh->ProcessOpenDoor(ped->m_vehEnterType, ANIM_CAR_CLOSEDOOR_LHS, 1.0f);

		eDoors door;
		switch (ped->m_vehEnterType) {
			case CAR_DOOR_RF: door = DOOR_FRONT_RIGHT; break;
			case CAR_DOOR_RR: door = DOOR_REAR_RIGHT; break;
			case CAR_DOOR_LF: door = DOOR_FRONT_LEFT; break;
			case CAR_DOOR_LR: door = DOOR_REAR_LEFT; break;
			default: assert(0);
		}

		if (veh->Damage.GetDoorStatus(door) == DOOR_STATUS_SMASHED)
			veh->Damage.SetDoorStatus(door, DOOR_STATUS_OK);

		if (door == DOOR_FRONT_LEFT || ped->m_objective == OBJECTIVE_ENTER_CAR_AS_PASSENGER || veh->bIsBus) {
			PedSetInCarCB(nil, ped);
		} else if (ped->m_vehEnterType == CAR_DOOR_RF
				&& (veh->m_nGettingInFlags & CAR_DOOR_FLAG_LF ||
					(veh->pDriver != nil && 
						(veh->pDriver->m_objective != OBJECTIVE_LEAVE_VEHICLE
#ifdef VC_PED_PORTS
							&& veh->pDriver->m_objective != OBJECTIVE_LEAVE_CAR_AND_DIE
#endif
							|| !veh->IsRoomForPedToLeaveCar(CAR_DOOR_LF, nil))))) {

			if (ped->m_objective == OBJECTIVE_ENTER_CAR_AS_DRIVER
#ifdef VC_PED_PORTS
				|| ped->m_nPedState == PED_CARJACK
#endif
				)
				veh->bIsBeingCarJacked = false;

			ped->m_objective = OBJECTIVE_ENTER_CAR_AS_PASSENGER;
			PedSetInCarCB(nil, ped);

			ped->SetObjective(OBJECTIVE_LEAVE_VEHICLE, veh);
			if (!ped->IsPlayer())
				ped->bFleeAfterExitingCar = true;

			ped->bUsePedNodeSeek = true;
			ped->m_pNextPathNode = nil;

		} else {
			if (animAssoc)
				animAssoc->blendDelta = -1000.0f;

			if (isLow)
				ped->m_pVehicleAnim = CAnimManager::AddAnimation(ped->GetClump(), ASSOCGRP_STD, ANIM_CAR_LSHUFFLE_RHS);
			else
				ped->m_pVehicleAnim = CAnimManager::AddAnimation(ped->GetClump(), ASSOCGRP_STD, ANIM_CAR_SHUFFLE_RHS);

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
CPed::PedAnimDoorCloseRollingCB(CAnimBlendAssociation* animAssoc, void* arg)
{
	CPed* ped = (CPed*)arg;

	CAutomobile* veh = (CAutomobile*)(ped->m_pMyVehicle);

	if (animAssoc)
		animAssoc->blendDelta = -1000.0f;

	if (veh->bLowVehicle) {
		veh->ProcessOpenDoor(CAR_DOOR_LF, ANIM_CAR_ROLLDOOR_LOW, 1.0f);
	} else {
		veh->ProcessOpenDoor(CAR_DOOR_LF, ANIM_CAR_ROLLDOOR, 1.0f);
	}

	veh->m_nGettingOutFlags &= ~CAR_DOOR_FLAG_LF;

	if (veh->Damage.GetDoorStatus(DOOR_FRONT_LEFT) == DOOR_STATUS_SWINGING)
		veh->Damage.SetDoorStatus(DOOR_FRONT_LEFT, DOOR_STATUS_OK);
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
		ped->QuitEnteringCar();
		return;
	}

	eDoors door;
	CPed *pedInSeat = nil;
	switch (ped->m_vehEnterType) {
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
		if (ped->m_vehEnterType != CAR_DOOR_LF && ped->m_vehEnterType != CAR_DOOR_LR)
			ped->SetFall(1000, ANIM_KO_SPIN_R, false);
		else
			ped->SetFall(1000, ANIM_KO_SPIN_L, false);
		
		return;
	}
	veh->ProcessOpenDoor(ped->m_vehEnterType, ANIM_CAR_OPEN_LHS, 1.0f);

	if (ped->m_vehEnterType == CAR_DOOR_LF || ped->m_vehEnterType == CAR_DOOR_RF)
		isVan = false;

	if (ped->m_nPedState != PED_CARJACK || isBus) {
		if (ped->m_vehEnterType != CAR_DOOR_LF && ped->m_vehEnterType != CAR_DOOR_LR) {

			if (isVan) {
				ped->m_pVehicleAnim = CAnimManager::AddAnimation(ped->GetClump(), ASSOCGRP_VAN, ANIM_VAN_GETIN);
			} else if (isBus) {
				ped->m_pVehicleAnim = CAnimManager::AddAnimation(ped->GetClump(), ASSOCGRP_COACH, ANIM_COACH_IN_R);
			} else if (isLow) {
				ped->m_pVehicleAnim = CAnimManager::AddAnimation(ped->GetClump(), ASSOCGRP_STD, ANIM_CAR_GETIN_LOW_RHS);
			} else {
				ped->m_pVehicleAnim = CAnimManager::AddAnimation(ped->GetClump(), ASSOCGRP_STD, ANIM_CAR_GETIN_RHS);
			}
		} else if (isVan) {
			ped->m_pVehicleAnim = CAnimManager::AddAnimation(ped->GetClump(), ASSOCGRP_VAN, ANIM_VAN_GETIN_L);
		} else if (isBus) {
			ped->m_pVehicleAnim = CAnimManager::AddAnimation(ped->GetClump(), ASSOCGRP_COACH, ANIM_COACH_IN_L);
		} else if (isLow) {
			ped->m_pVehicleAnim = CAnimManager::AddAnimation(ped->GetClump(), ASSOCGRP_STD, ANIM_CAR_GETIN_LOW_LHS);
		} else {
			ped->m_pVehicleAnim = CAnimManager::AddAnimation(ped->GetClump(), ASSOCGRP_STD, ANIM_CAR_GETIN_LHS);
		}
		ped->m_pVehicleAnim->SetFinishCallback(PedAnimGetInCB, ped);
	} else {
		CPed *pedToDragOut = nil;
		switch (ped->m_vehEnterType) {
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

		if (ped->m_vehEnterType != CAR_DOOR_LF && ped->m_vehEnterType != CAR_DOOR_LR) {
			if (pedToDragOut && !pedToDragOut->bDontDragMeOutCar) {
				if (pedToDragOut->m_nPedState != PED_DRIVING) {
					ped->QuitEnteringCar();
					pedToDragOut = nil;
				} else {
					if (isLow)
						ped->m_pVehicleAnim = CAnimManager::AddAnimation(ped->GetClump(), ASSOCGRP_STD, ANIM_CAR_PULLOUT_LOW_RHS);
					else
						ped->m_pVehicleAnim = CAnimManager::AddAnimation(ped->GetClump(), ASSOCGRP_STD, ANIM_CAR_PULLOUT_RHS);

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
					ped->m_pVehicleAnim = CAnimManager::AddAnimation(ped->GetClump(), ASSOCGRP_STD, ANIM_CAR_GETIN_LOW_LHS);
				else
					ped->m_pVehicleAnim = CAnimManager::AddAnimation(ped->GetClump(), ASSOCGRP_STD, ANIM_CAR_GETIN_LHS);

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
						ped->m_pVehicleAnim = CAnimManager::AddAnimation(ped->GetClump(), ASSOCGRP_STD, ANIM_CAR_PULLOUT_LOW_LHS);
					else
						ped->m_pVehicleAnim = CAnimManager::AddAnimation(ped->GetClump(), ASSOCGRP_STD, ANIM_CAR_PULLOUT_LHS);
					
					ped->m_pVehicleAnim->SetFinishCallback(PedAnimPullPedOutCB, ped);
				}
			} else {
				if (isLow)
					ped->m_pVehicleAnim = CAnimManager::AddAnimation(ped->GetClump(), ASSOCGRP_STD, ANIM_CAR_GETIN_LOW_LHS);
				else
					ped->m_pVehicleAnim = CAnimManager::AddAnimation(ped->GetClump(), ASSOCGRP_STD, ANIM_CAR_GETIN_LHS);

				ped->m_pVehicleAnim->SetFinishCallback(PedAnimGetInCB, ped);
			}
		}

		if (pedToDragOut) {
			pedToDragOut->SetBeingDraggedFromCar(veh, ped->m_vehEnterType, false);
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
CPed::SetJump(void)
{
	if (!bInVehicle &&
#ifdef VC_PED_PORTS
		m_nPedState != PED_JUMP && !RpAnimBlendClumpGetAssociation(GetClump(), ANIM_JUMP_LAUNCH) &&
#endif
		(m_nSurfaceTouched != SURFACE_STONE || DotProduct(GetForward(), m_vecDamageNormal) >= 0.0f)) {
		SetStoredState();
		m_nPedState = PED_JUMP;
		CAnimBlendAssociation *jumpAssoc = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_JUMP_LAUNCH, 8.0f);
		jumpAssoc->SetFinishCallback(FinishLaunchCB, this);
		m_fRotationDest = m_fRotationCur;
	}
}

bool
CPed::ServiceTalkingWhenDead(void)
{
	return m_queuedSound == SOUND_PED_DEATH;
}

// --MIAMI: Done
void
CPed::RemoveInCarAnims(void)
{
	CAnimBlendAssociation* assoc;

	for (assoc = RpAnimBlendClumpGetFirstAssociation(GetClump(), ASSOC_DRIVING); assoc;
		assoc = RpAnimBlendGetNextAssociation(assoc, ASSOC_DRIVING)) {
		assoc->flags |= ASSOC_DELETEFADEDOUT;
		assoc->blendDelta = -1000.0f;
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
		ped->m_nPedState = PED_ARRESTED;
		ped->bGonnaKillTheCarJacker = false;
		if (veh) {
			veh->m_nNumGettingIn = 0;
			veh->m_nGettingInFlags = 0;	
			veh->bIsHandbrakeOn = true;
			veh->SetStatus(STATUS_PLAYER_DISABLED);
		}
		return;
	}
	if (ped->IsPlayer() && ped->m_vehEnterType == CAR_DOOR_LF
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
	switch (ped->m_vehEnterType) {
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
			driver->SetObjective(OBJECTIVE_LEAVE_VEHICLE, veh);
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
				ped->SetObjective(OBJECTIVE_LEAVE_VEHICLE, veh);
			}
			ped->m_leaveCarTimer = CTimer::GetTimeInMilliseconds() + 400;
			return;
		}
		if (driver != ped && ped->m_vehEnterType != CAR_DOOR_LF) {
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
				veh->pDriver->SetObjective(OBJECTIVE_LEAVE_VEHICLE, veh);
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
		if (enterDoor != DOOR_FRONT_LEFT && enterDoor != DOOR_REAR_LEFT) {
			if (isVan) {
				ped->m_pVehicleAnim = CAnimManager::AddAnimation(ped->GetClump(), ASSOCGRP_VAN, ANIM_VAN_CLOSE);
			} else if (isLow) {
				ped->m_pVehicleAnim = CAnimManager::AddAnimation(ped->GetClump(), ASSOCGRP_STD, ANIM_CAR_CLOSEDOOR_LOW_RHS);
			} else {
				ped->m_pVehicleAnim = CAnimManager::AddAnimation(ped->GetClump(), ASSOCGRP_STD, ANIM_CAR_CLOSEDOOR_RHS);
			}
		} else if (isVan) {
			ped->m_pVehicleAnim = CAnimManager::AddAnimation(ped->GetClump(), ASSOCGRP_VAN, ANIM_VAN_CLOSE_L);
		} else if (isLow) {
			ped->m_pVehicleAnim = CAnimManager::AddAnimation(ped->GetClump(), ASSOCGRP_STD, ANIM_CAR_CLOSEDOOR_LOW_LHS);
		} else {
			ped->m_pVehicleAnim = CAnimManager::AddAnimation(ped->GetClump(), ASSOCGRP_STD, ANIM_CAR_CLOSEDOOR_LHS);
		}
		ped->m_pVehicleAnim->SetFinishCallback(PedAnimDoorCloseCB, ped);
	}
}

#ifdef GTA_TRAIN
void
CPed::SetPedPositionInTrain(void)
{
	LineUpPedWithTrain();
}
#endif

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
			if (ped->m_vehEnterType != CAR_DOOR_LF && ped->m_vehEnterType != CAR_DOOR_LR) {
				if (isLow)
					animToPlay = ANIM_CAR_GETIN_LOW_RHS;
				else
					animToPlay = ANIM_CAR_GETIN_RHS;
			} else if (isLow) {
				animToPlay = ANIM_CAR_GETIN_LOW_LHS;
			} else {
				animToPlay = ANIM_CAR_GETIN_LHS;
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
		veh->ProcessOpenDoor(ped->m_vehEnterType, ANIM_CAR_GETOUT_LHS, 1.0f);

	/* 
	// Duplicate and only in PC for some reason
	if (!veh) {
		PedSetOutCarCB(nil, ped);
		return;
	}
	*/
	eDoors door;
	switch (ped->m_vehEnterType) {
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
	bool closeDoor = false;
	if (!veh->IsDoorMissing(door))
		closeDoor = true;

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
		bool engineIsIntact = false;
		if (veh->IsCar() && ((CAutomobile*)veh)->Damage.GetEngineStatus() >= 225) {
			engineIsIntact = true;
		}
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
		// POTENTIAL BUG? Why DOOR_FRONT_LEFT instead of door variable? or vice versa?
		if (!veh->IsDoorMissing(door))
			((CAutomobile*)veh)->Damage.SetDoorStatus(DOOR_FRONT_LEFT, DOOR_STATUS_SWINGING);
	} else {
		switch (door) {
			case DOOR_FRONT_LEFT:
				ped->m_pVehicleAnim = CAnimManager::AddAnimation(ped->GetClump(), ASSOCGRP_STD, ANIM_CAR_CLOSE_LHS);
				break;
			case DOOR_FRONT_RIGHT:
				ped->m_pVehicleAnim = CAnimManager::AddAnimation(ped->GetClump(), ASSOCGRP_STD, ANIM_CAR_CLOSE_RHS);
				break;
			case DOOR_REAR_LEFT:
				ped->m_pVehicleAnim = CAnimManager::AddAnimation(ped->GetClump(), ASSOCGRP_STD, ANIM_CAR_CLOSE_LHS);
				break;
			case DOOR_REAR_RIGHT:
				ped->m_pVehicleAnim = CAnimManager::AddAnimation(ped->GetClump(), ASSOCGRP_STD, ANIM_CAR_CLOSE_RHS);
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
CPed::PedEvadeCB(CAnimBlendAssociation* animAssoc, void* arg)
{
	CPed* ped = (CPed*)arg;

	if (!animAssoc) {
		ped->ClearLookFlag();
		if (ped->m_nPedState != PED_DIVE_AWAY && ped->m_nPedState != PED_STEP_AWAY)
			return;

		ped->RestorePreviousState();
	} else if (animAssoc->animId == ANIM_EV_DIVE) {
		ped->bUpdateAnimHeading = true;
		ped->ClearLookFlag();
		if (ped->m_nPedState == PED_DIVE_AWAY)
		{
			ped->m_getUpTimer = CTimer::GetTimeInMilliseconds() + 1;
			ped->m_nPedState = PED_FALL;
		}
		animAssoc->flags &= ~ASSOC_FADEOUTWHENDONE;
		animAssoc->flags |= ASSOC_DELETEFADEDOUT;
	} else if (animAssoc->flags & ASSOC_FADEOUTWHENDONE) {
		ped->ClearLookFlag();
		if (ped->m_nPedState != PED_DIVE_AWAY && ped->m_nPedState != PED_STEP_AWAY)
			return;

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
CPed::PedSetDraggedOutCarPositionCB(CAnimBlendAssociation* animAssoc, void* arg)
{
	CPed *ped = (CPed*)arg;

	ped->bUsesCollision = true;
	ped->RestartNonPartialAnims();
	bool itsRearDoor = false;

	if (ped->m_vehEnterType == CAR_DOOR_RF || ped->m_vehEnterType == CAR_DOOR_RR)
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

	if (ped->m_pMyVehicle && !ped->m_pMyVehicle->IsRoomForPedToLeaveCar(ped->m_vehEnterType, &vecPedDraggedOutCarAnimOffset)) {
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
				ped->m_nPedState = PED_NONE;
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
			ped->m_nPedState = PED_NONE;
			ped->m_nLastPedState = PED_NONE;
			ped->SetFindPathAndFlee(ped->m_pMyVehicle->GetPosition(), 10000);
		}
	}
	ped->SetGetUp();
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
	if (driver && driver->m_nPedState == PED_DRIVING && !veh->bIsBus && driver->m_objective == OBJECTIVE_LEAVE_VEHICLE
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
#ifdef VC_PED_PORTS
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
#if defined(FIX_BUGS) || defined(VC_PED_PORTS)
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

		ped->m_nPedState = PED_DRIVING;
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
				passenger->SetObjective(OBJECTIVE_LEAVE_VEHICLE, veh);
#ifdef VC_PED_PORTS
				passenger->m_leaveCarTimer = CTimer::GetTimeInMilliseconds();
#endif
			}
		}
	}
	// This shouldn't happen at all. Passengers can't enter with PED_CARJACK. Even though they did, we shouldn't call AddPassenger in here and SetDriver in below.
#ifndef VC_PED_PORTS
	else if (ped->m_objective == OBJECTIVE_ENTER_CAR_AS_PASSENGER) {
		if (ped->m_nPedState == PED_CARJACK) {
			veh->AddPassenger(ped, 0);
			ped->m_nPedState = PED_DRIVING;
			ped->RestorePreviousObjective();
			ped->SetObjective(OBJECTIVE_LEAVE_VEHICLE, veh);
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
		ped->m_nPedState = PED_DRIVING;
		if (ped->m_objective == OBJECTIVE_ENTER_CAR_AS_DRIVER) {

			if (ped->m_prevObjective == OBJECTIVE_RUN_TO_AREA || ped->m_prevObjective == OBJECTIVE_GOTO_CHAR_ON_FOOT || ped->m_prevObjective == OBJECTIVE_KILL_CHAR_ON_FOOT)
				ped->m_prevObjective = OBJECTIVE_NONE;

			ped->RestorePreviousObjective();
		}

	} else if (ped->m_objective == OBJECTIVE_ENTER_CAR_AS_PASSENGER) {
		if (veh->bIsBus) {
			veh->AddPassenger(ped);
		} else {
			switch (ped->m_vehEnterType) {
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
		ped->m_nPedState = PED_DRIVING;
		if (ped->m_prevObjective == OBJECTIVE_RUN_TO_AREA || ped->m_prevObjective == OBJECTIVE_GOTO_CHAR_ON_FOOT || ped->m_prevObjective == OBJECTIVE_KILL_CHAR_ON_FOOT)
			ped->m_prevObjective = OBJECTIVE_NONE;

		ped->RestorePreviousObjective();
#ifdef VC_PED_PORTS
		if(veh->pDriver && ped->CharCreatedBy == RANDOM_CHAR)
			veh->AutoPilot.m_nCruiseSpeed = 17;
#endif
	}

	veh->m_nGettingInFlags &= ~GetCarDoorFlag(ped->m_vehEnterType);

	if (veh->bIsBus && !veh->m_nGettingInFlags)
		((CAutomobile*)veh)->SetBusDoorTimer(1000, 1);

	switch (ped->m_objective) {
		case OBJECTIVE_KILL_CHAR_ON_FOOT:
		case OBJECTIVE_KILL_CHAR_ANY_MEANS:
		case OBJECTIVE_LEAVE_VEHICLE:
		case OBJECTIVE_FOLLOW_CAR_IN_CAR:
		case OBJECTIVE_GOTO_AREA_ANY_MEANS:
		case OBJECTIVE_GOTO_AREA_ON_FOOT:
		case OBJECTIVE_RUN_TO_AREA:
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

void
CPed::PedSetInTrainCB(CAnimBlendAssociation* animAssoc, void* arg)
{
	CPed *ped = (CPed*)arg;
	CTrain *veh = (CTrain*)ped->m_pMyVehicle;

	if (!veh)
		return;

	ped->bInVehicle = true;
	ped->m_nPedState = PED_DRIVING;
	ped->RestorePreviousObjective();
	ped->SetMoveState(PEDMOVE_STILL);
	veh->AddPassenger(ped);
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

// It's "CPhoneInfo::ProcessNearestFreePhone" in PC IDB, but it's not true, someone made it up.
bool
CPed::RunToReportCrime(eCrimeType crimeToReport)
{
#ifdef TOGGLEABLE_BETA_FEATURES
	if (!bMakePedsRunToPhonesToReportCrimes)
		return false;

	if (bRunningToPhone)
		return true;
#else
	// They changed true into false to make this function unusable. So running to phone actually starts but first frame after that cancels it.
	if (m_nPedState == PED_SEEK_POS)
		return false;
#endif

	CVector pos = GetPosition();
	int phoneId = gPhoneInfo.FindNearestFreePhone(&pos);

	if (phoneId == -1)
		return false;

	CPhone *phone = &gPhoneInfo.m_aPhones[phoneId];
	if (phone->m_nState != PHONE_STATE_FREE)
		return false;

	bRunningToPhone = true;
	SetSeek(phone->m_pEntity->GetPosition() - phone->m_pEntity->GetForward(), 1.3f); // original: phone.m_vecPos, 0.3f
	SetMoveState(PEDMOVE_RUN);
	bIsRunning = true; // not there in original
	m_phoneId = phoneId;
	m_crimeToReportOnPhone = crimeToReport;
	return true;
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
		if (!attackerPed->m_pMyVehicle || attackerPed->m_pMyVehicle->GetModelIndex() != MI_TOPFUN) {
			int16 lastVehicle;
			CEntity *vehicles[8];
			CWorld::FindObjectsInRange(GetPosition(), 30.0f, true, &lastVehicle, 6, vehicles, false, true, false, false, false);

			if (lastVehicle > 8)
				lastVehicle = 8;

			for (int j = 0; j < lastVehicle; ++j) {
				CVehicle *nearVeh = (CVehicle*) vehicles[j];

				if (nearVeh->VehicleCreatedBy != MISSION_VEHICLE) {
					CPed *nearVehDriver = nearVeh->pDriver;

					if (nearVehDriver && nearVehDriver != this && nearVehDriver->m_nPedType == m_nPedType) {

						if (nearVeh->IsVehicleNormal() && nearVeh->IsCar()) {
							nearVeh->AutoPilot.m_nCruiseSpeed = GAME_SPEED_TO_CARAI_SPEED * nearVeh->pHandling->Transmission.fUnkMaxVelocity * 0.8f;
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
			}
			if (bCrouchWhenShooting || bKindaStayInSamePlace) {
				SetDuck(CGeneral::GetRandomNumberInRange(1000, 3000));
			}

		} else if (m_nPedType != PEDTYPE_COP
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
					// (CGeneral::GetRandomNumber() % 256) * PI / 128 gives a float up to something TWOPI-ish.
					float pickupX = 1.5f * Sin((CGeneral::GetRandomNumber() % 256) * PI / 128) + GetPosition().x;
					float pickupY = 1.5f * Cos((CGeneral::GetRandomNumber() % 256) * PI / 128) + GetPosition().y;
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

	if (ped->m_objective == OBJECTIVE_LEAVE_VEHICLE)
		ped->RestorePreviousObjective();
#ifdef VC_PED_PORTS
	else if (ped->m_objective == OBJECTIVE_LEAVE_CAR_AND_DIE) {
		ped->m_fHealth = 0.0f;
		ped->SetDie(ANIM_FLOOR_HIT, 4.0f, 0.5f);
	}
#endif

	ped->bInVehicle = false;
	if (veh && veh->IsCar() && !veh->IsRoomForPedToLeaveCar(ped->m_vehEnterType, nil)) {
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
		veh->m_nGettingOutFlags &= ~GetCarDoorFlag(ped->m_vehEnterType);
		if (veh->pDriver == ped) {
			veh->RemoveDriver();
			veh->SetStatus(STATUS_ABANDONED);
			if (veh->m_nDoorLock == CARLOCK_LOCKED_INITIALLY)
				veh->m_nDoorLock = CARLOCK_UNLOCKED;
			if (ped->m_nPedType == PEDTYPE_COP && veh->IsLawEnforcementVehicle())
				veh->ChangeLawEnforcerState(false);
		} else {
			veh->RemovePassenger(ped);
		}

		if (veh->bIsBus && !veh->IsUpsideDown() && !veh->IsOnItsSide()) {
			float angleAfterExit;
			if (ped->m_vehEnterType == CAR_DOOR_LF) {
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
		ped->SetFall(1500, ANIM_KO_SKID_BACK, false);
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

// --MIAMI: Done, but enumarate weapon slots
inline void
CPed::ReplaceWeaponWhenExitingVehicle(void)
{
	eWeaponType weaponType = GetWeapon()->m_eWeaponType;

	// If it's Uzi, we may have stored weapon. Uzi is the only gun we can use in car.
	if (IsPlayer() && GetWeaponSlot(weaponType) == 5) {
		if (m_storedWeapon != WEAPONTYPE_UNIDENTIFIED) {
			SetCurrentWeapon(m_storedWeapon);
			m_storedWeapon = WEAPONTYPE_UNIDENTIFIED;
		}
	} else {
		AddWeaponModel(CWeaponInfo::GetWeaponInfo(weaponType)->m_nModelId);
	}
}

// --MIAMI: Done
inline void
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
CPed::PedSetOutTrainCB(CAnimBlendAssociation *animAssoc, void *arg)
{
	CPed *ped = (CPed*)arg;

	CVehicle *veh = ped->m_pMyVehicle;

	if (ped->m_pVehicleAnim)
		ped->m_pVehicleAnim->blendDelta = -1000.0f;

	ped->bUsesCollision = true;
	ped->m_pVehicleAnim = nil;
	ped->bInVehicle = false;
	ped->m_nPedState = PED_IDLE;
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
CPed::PedSetQuickDraggedOutCarPositionCB(CAnimBlendAssociation *animAssoc, void *arg)
{
	CPed *ped = (CPed*)arg;

	CVehicle *veh = ped->m_pMyVehicle;

	CVector finalPos;
	CVector draggedOutOffset;
	CVector finalLocalPos;

	CMatrix pedMat(ped->GetMatrix());
	ped->bUsesCollision = true;
	ped->RestartNonPartialAnims();
	draggedOutOffset = vecPedQuickDraggedOutCarAnimOffset;
	if (ped->m_vehEnterType == CAR_DOOR_RF || ped->m_vehEnterType == CAR_DOOR_RR)
		draggedOutOffset.x = -draggedOutOffset.x;

	finalLocalPos = Multiply3x3(pedMat, draggedOutOffset);
	finalPos = finalLocalPos + ped->GetPosition();
	CPedPlacement::FindZCoorForPed(&finalPos);
	ped->m_vecMoveSpeed = CVector(0.0f, 0.0f, 0.0f);
	ped->SetPosition(finalPos);

	if (veh) {
		ped->m_fRotationDest = veh->GetForward().Heading() - HALFPI;
		ped->m_fRotationCur = ped->m_fRotationDest;
		ped->CalculateNewOrientation();

		if (!veh->IsRoomForPedToLeaveCar(ped->m_vehEnterType, &vecPedQuickDraggedOutCarAnimOffset))
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
		} else {
			if (ped->m_pedStats->m_temper > ped->m_pedStats->m_fear
				&& ped->CharCreatedBy != MISSION_CHAR && veh->VehicleCreatedBy != MISSION_VEHICLE
				&& veh->pDriver && veh->pDriver->IsPlayer()
				&& !CTheScripts::IsPlayerOnAMission()) {

#ifndef VC_PED_PORTS
				if (CGeneral::GetRandomNumber() < MYRAND_MAX / 2) {
					ped->SetObjective(OBJECTIVE_KILL_CHAR_ON_FOOT, veh->pDriver);
				} else
#endif
					ped->SetObjective(OBJECTIVE_ENTER_CAR_AS_DRIVER, veh);

			} else {
#ifdef VC_PED_PORTS
				if (ped->m_pedStats->m_temper > ped->m_pedStats->m_fear && ped->CharCreatedBy != MISSION_CHAR
					&& ped->m_pMyVehicle->VehicleCreatedBy != MISSION_VEHICLE && !veh->pDriver
					&& FindPlayerPed()->m_carInObjective == ped->m_pMyVehicle && !CTheScripts::IsPlayerOnAMission())
					ped->SetObjective(OBJECTIVE_ENTER_CAR_AS_DRIVER, veh);
				else
#endif
				{
					ped->SetFindPathAndFlee(veh->GetPosition(), 10000);
					if (CGeneral::GetRandomNumber() & 1 || ped->m_pedStats->m_fear > 70) {
						ped->SetMoveState(PEDMOVE_SPRINT);
						ped->Say(SOUND_PED_FLEE_SPRINT);
					} else {
						ped->Say(SOUND_PED_FLEE_RUN);
					}
				}
			}
		}
	}
	if (ped->m_nLastPedState == PED_IDLE)
		ped->m_nLastPedState = PED_WANDER_PATH;
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
		if (m_vehEnterType == CAR_DOOR_RR) {
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
		} else if(m_vehEnterType == CAR_DOOR_RF) {
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
		} else if (m_vehEnterType == CAR_DOOR_LF) {
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
		} else if (m_vehEnterType == CAR_DOOR_LR) {
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
CPed::Render(void)
{
	if (!bInVehicle || m_nPedState == PED_EXIT_CAR || m_nPedState == PED_DRAG_FROM_CAR ||
		bRenderPedInCar && sq(25.0f * TheCamera.LODDistMultiplier) >= (TheCamera.GetPosition() - GetPosition()).MagnitudeSqr()) {
		CEntity::Render();

		if(m_pWeaponModel){
			RpHAnimHierarchy *hier = GetAnimHierarchyFromSkinClump(GetClump());
			int idx = RpHAnimIDGetIndex(hier, m_pFrames[PED_HANDR]->nodeID);
			RwMatrix *mat = &RpHAnimHierarchyGetMatrixArray(hier)[idx];
			RwFrame *frame = RpAtomicGetFrame(m_pWeaponModel);
			*RwFrameGetMatrix(frame) = *mat;
			RwFrameUpdateObjects(frame);
			RpAtomicRender(m_pWeaponModel);
		}
	}
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
			case OBJECTIVE_FIRE_AT_OBJ_FROM_VEHICLE:
			case OBJECTIVE_DESTROY_OBJ:
			case OBJECTIVE_26:
			case OBJECTIVE_27:
			case OBJECTIVE_SET_LEADER:
				break;
			case OBJECTIVE_IDLE:
				if (GetPedState() == PED_DRIVING)
					m_objective = OBJECTIVE_NONE;
				else {
					SetIdle();
					if (m_attractor) {
						if (m_objectiveTimer && CTimer::GetTimeInMilliseconds() > m_nWaitTimer) {
							GetPedAttractorManager()->BroadcastDeparture(this, m_attractor);
							m_objectiveTimer = 0;
						}
					}
					else {
						m_objective = OBJECTIVE_NONE;
						SetMoveState(PEDMOVE_STILL);
					}
				}
				break;
			case OBJECTIVE_FLEE_TILL_SAFE:
				if (InVehicle()) {
					SetObjective(OBJECTIVE_LEAVE_VEHICLE, m_pMyVehicle);
					bFleeAfterExitingCar = true;
				} else if (m_nPedState != PED_FLEE_POS) {
					SetFlee(GetPosition(), 10000);
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
				m_nPedState = PED_DRIVING;
				break;
			case OBJECTIVE_WAIT_IN_CAR_THEN_GETOUT:
				m_nPedState = PED_DRIVING;
				break;
			case OBJECTIVE_KILL_CHAR_ANY_MEANS:
			{
				if (m_pedInObjective) {
					if (m_pedInObjective->IsPlayer() && CharCreatedBy != MISSION_CHAR
						&& m_nPedType != PEDTYPE_COP && FindPlayerPed()->m_pWanted->m_CurrentCops
						&& !bKindaStayInSamePlace) {

						SetObjective(OBJECTIVE_FLEE_TILL_SAFE);
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
									m_pMyVehicle->AutoPilot.m_nCruiseSpeed = (FindPlayerPed()->m_pWanted->m_nWantedLevel * 0.1f + 0.6f) * (GAME_SPEED_TO_CARAI_SPEED * m_pMyVehicle->pHandling->Transmission.fUnkMaxVelocity);
									m_pMyVehicle->AutoPilot.m_nCarMission = CCarAI::FindPoliceCarMissionForWantedLevel();
								} else {
									m_pMyVehicle->AutoPilot.m_nCruiseSpeed = GAME_SPEED_TO_CARAI_SPEED * m_pMyVehicle->pHandling->Transmission.fUnkMaxVelocity * 0.8f;
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
								SetObjective(OBJECTIVE_LEAVE_VEHICLE, m_pMyVehicle);
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
					SetObjective(OBJECTIVE_LEAVE_VEHICLE, m_pMyVehicle);
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
				float wepRangeAdjusted;
				if (GetWeapon()->m_eWeaponType != WEAPONTYPE_UNARMED) {
					wepRangeAdjusted = wepRange / 3.0f;
				} else {
					if (m_nPedState == PED_FIGHT) {
						if (!IsPlayer() && !(m_pedStats->m_flags & STAT_CAN_KICK))
							wepRange = 2.0f;
					} else {
						wepRange = 1.3f;
					}
					wepRangeAdjusted = wepRange;
				}
				if (m_pedInObjective->m_getUpTimer > CTimer::GetTimeInMilliseconds() && wepRangeAdjusted < 2.5f) {
					wepRangeAdjusted = 2.5f;
				}
				if (m_pedInObjective->IsPlayer() && m_nPedType != PEDTYPE_COP
					&& CharCreatedBy != MISSION_CHAR && FindPlayerPed()->m_pWanted->m_CurrentCops) {
					SetObjective(OBJECTIVE_FLEE_TILL_SAFE);
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
								maxShotPos.Normalise();
								maxShotPos = maxShotPos * wepInfo->m_fRange + ourHead;

								CWorld::bIncludeDeadPeds = true;
								CColPoint foundCol;
								CEntity *foundEnt;
								CWorld::ProcessLineOfSight(ourHead, maxShotPos, foundCol, foundEnt,
									true, true, true, true, false, true, false);
								CWorld::bIncludeDeadPeds = false;
								if (foundEnt == vehOfTarget) {
									SetAttack(vehOfTarget);
									m_pPointGunAt = vehOfTarget;
									if (vehOfTarget)
										vehOfTarget->RegisterReference((CEntity **) &m_pPointGunAt);

									SetShootTimer(CGeneral::GetRandomNumberInRange(500, 2000));
									if (distWithTargetSc <= m_distanceToCountSeekDone) {
										SetAttackTimer(CGeneral::GetRandomNumberInRange(200, 500));
										SetMoveState(PEDMOVE_STILL);
									} else {
										SetAttackTimer(CGeneral::GetRandomNumberInRange(2000, 5000));
									}
								}
							}
							else if (m_nPedState != PED_ATTACK && !bKindaStayInSamePlace && !killPlayerInNoPoliceZone) {
								if (vehOfTarget) {
									if (m_nPedType == PEDTYPE_COP || vehOfTarget->bIsBus) {
										GoToNearestDoor(vehOfTarget);
									} else {
										m_vehEnterType = 0;
										if (m_pedInObjective == vehOfTarget->pDriver || vehOfTarget->bIsBus) {
											m_vehEnterType = CAR_DOOR_LF;
										} else if (m_pedInObjective == vehOfTarget->pPassengers[0]) {
											m_vehEnterType = CAR_DOOR_RF;
										} else if (m_pedInObjective == vehOfTarget->pPassengers[1]) {
											m_vehEnterType = CAR_DOOR_LR;
										} else if (m_pedInObjective == vehOfTarget->pPassengers[2]) {
											m_vehEnterType = CAR_DOOR_RR;
										}
										// Unused
										// GetPositionToOpenCarDoor(vehOfTarget, m_vehEnterType);
										SetSeekCar(vehOfTarget, m_vehEnterType);
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
							wepRangeAdjusted = 2.0f;

						if (bUsePedNodeSeek) {
							CVector bestCoords(0.0f, 0.0f, 0.0f);
							m_vecSeekPos = m_pedInObjective->GetPosition();

							if (!m_pNextPathNode)
								FindBestCoordsFromNodes(m_vecSeekPos, &bestCoords);

							if (m_pNextPathNode)
								m_vecSeekPos = m_pNextPathNode->GetPosition();

							SetSeek(m_vecSeekPos, m_distanceToCountSeekDone);
						} else {
							SetSeek(m_pedInObjective, wepRangeAdjusted);
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
						CAnimBlendAssociation *duckAnim = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_DUCK_DOWN);
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
							m_pedInObjective->m_pedIK.GetComponentPosition((RwV3d*)&target, PED_MID);
						else
							target = m_pedInObjective->GetPosition();

						target -= ourHead;
						target.Normalise();
						target = target * wepInfo->m_fRange + ourHead;

						CWorld::bIncludeDeadPeds = true;
						CEntity *foundEnt = nil;
						CColPoint foundCol;

						CWorld::ProcessLineOfSight(
							ourHead, target, foundCol, foundEnt,
							true, true, true, false, true, false);
						CWorld::bIncludeDeadPeds = 0;
						if (foundEnt == m_pedInObjective) {
							SetAttack(m_pedInObjective);
							m_pPointGunAt = m_pedInObjective;
							if (m_pedInObjective)
								m_pedInObjective->RegisterReference((CEntity **) &m_pPointGunAt);

							SetShootTimer(CGeneral::GetRandomNumberInRange(500.0f, 2000.0f));

							int time;
							if (distWithTargetSc <= wepRangeAdjusted)
								time = CGeneral::GetRandomNumberInRange(100.0f, 500.0f);
							else
								time = CGeneral::GetRandomNumberInRange(1500.0f, 3000.0f);

							SetAttackTimer(time);
							bObstacleShowedUpDuringKillObjective = false;

						} else if (foundEnt) {
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
								CAnimBlendAssociation* duckAnim = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_DUCK_DOWN);
								if (!duckAnim || duckAnim->blendDelta < 0.0f) {
									CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_DUCK_DOWN, 4.0f);
									bIsDucking = true;
								}
								break;
							} else {
								CAnimBlendAssociation* duckAnim = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_DUCK_DOWN);
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
									wepRangeAdjusted = 6.0f;
								} else if (m_fleeFrom && m_fleeFrom->IsVehicle()) {
									wepRangeAdjusted = 4.0f;
								} else {
									wepRangeAdjusted = 2.0f;
								}
							} else {
								wepRangeAdjusted = 2.0f;
							}
						}
						if (distWithTargetSc <= wepRangeAdjusted) {
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
								SetSeek(m_pedInObjective, wepRangeAdjusted);
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
						SetObjective(OBJECTIVE_LEAVE_VEHICLE, m_pMyVehicle);
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
			case OBJECTIVE_FOLLOW_PED_IN_FORMATION:
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
										m_vehEnterType = CAR_DOOR_RR;
										foundSeat = true;
									}
								} else {
									m_vehEnterType = CAR_DOOR_LR;
									foundSeat = true;
								}
							} else {
								m_vehEnterType = CAR_DOOR_RF;
								foundSeat = true;
							}
							for (int i = 2; i < m_carInObjective->m_nNumMaxPassengers; ++i) {
								if (!m_carInObjective->pPassengers[i] && !(m_carInObjective->m_nGettingInFlags & CAR_DOOR_FLAG_RF)) {
									m_vehEnterType = CAR_DOOR_RF;
									foundSeat = true;
								}
							}
							if (foundSeat) {
								SetPosition(GetPositionToOpenCarDoor(m_carInObjective, m_vehEnterType));
								SetEnterCar(m_carInObjective, m_vehEnterType);
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
								if (nEnterCarRangeMultiplier * 30.0f < distWithTarget.Magnitude()) {
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
					maxShotPos.Normalise();
					maxShotPos = maxShotPos * wepInfo->m_fRange + ourHead;

					CWorld::bIncludeDeadPeds = true;
					CColPoint foundCol;
					CEntity *foundEnt;
					CWorld::ProcessLineOfSight(ourHead, maxShotPos, foundCol, foundEnt,
						true, true, true, true, false, true, false);
					CWorld::bIncludeDeadPeds = false;
					if (foundEnt == m_carInObjective) {
						SetAttack(m_carInObjective);
						m_pPointGunAt = m_carInObjective;
						if (m_pPointGunAt)
							m_pPointGunAt->RegisterReference((CEntity **) &m_pPointGunAt);

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
						SetObjective(OBJECTIVE_LEAVE_VEHICLE, m_pMyVehicle);
					}
					break;
				}
				if (distWithTarget.Magnitude() > 30.0f) {
					if (m_pMyVehicle) {
						m_pMyVehicle->AutoPilot.m_nCruiseSpeed = 0;
					} else {
						float closestVehDist = 3600.0f;
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
							if (vehDistVec.Magnitude() < closestVehDist
								&& m_pedInObjective->m_pMyVehicle != nearVeh)
							{
								foundVeh = nearVeh;
								closestVehDist = vehDistVec.Magnitude();
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
					SetObjective(OBJECTIVE_LEAVE_VEHICLE, m_pMyVehicle);
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
			case OBJECTIVE_FIGHT_CHAR:
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
			case OBJECTIVE_SOLICIT:
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
				if (!RpAnimBlendClumpGetAssociation(GetClump(), ANIM_IDLE_TAXI) && CTimer::GetTimeInMilliseconds() > m_nWaitTimer) {
					Say(SOUND_PED_TAXI_WAIT);
					CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_IDLE_TAXI, 4.0f);
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
				} else if (m_hitRecoverTimer < CTimer::GetTimeInMilliseconds()) {
					CVehicle *carToSteal = nil;
					float closestCarDist = 30.0f;
					CVector pos = GetPosition();
					int16 lastVehicle;
					CEntity *vehicles[8];

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
						m_hitRecoverTimer = CTimer::GetTimeInMilliseconds() + 5000;
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
							CAnimBlendAssociation *reloadAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_FUCKU);
							m_fRotationDest = CGeneral::GetRadianAngleBetweenPoints(
								m_pedInObjective->GetPosition().x, m_pedInObjective->GetPosition().y,
								GetPosition().x, GetPosition().y);

							if (reloadAssoc || !m_pedInObjective->IsPedShootable()) {
								if (reloadAssoc &&
									(!reloadAssoc->IsRunning() || reloadAssoc->GetProgress() > 0.8f)) {
									CAnimBlendAssociation *punchAssoc = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_FIGHT_PPUNCH, 8.0f);
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

								CAnimBlendAssociation *newReloadAssoc = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_FUCKU, 8.0f);
								newReloadAssoc->flags |= ASSOC_DELETEFADEDOUT;
								newReloadAssoc->flags |= ASSOC_FADEOUTWHENDONE;
							}
						} else {
							SetSeek(m_pedInObjective, 1.0f);
							CAnimBlendAssociation *walkAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_WALK);

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
			case OBJECTIVE_LEAVE_VEHICLE:
				if (CTimer::GetTimeInMilliseconds() > m_leaveCarTimer) {
					if (InVehicle()
						&& (FindPlayerPed() != this || !CPad::GetPad(0)->GetAccelerate()
							|| bBusJacked)
						) {
						if (m_nPedState != PED_EXIT_CAR && m_nPedState != PED_DRAG_FROM_CAR && m_nPedState != PED_EXIT_TRAIN
							&& (m_nPedType != PEDTYPE_COP
								|| m_pMyVehicle->IsBoat()
								|| m_pMyVehicle->m_vecMoveSpeed.MagnitudeSqr2D() < sq(0.005f))) {
#ifdef GTA_TRAIN
							if (m_pMyVehicle->IsTrain())
								SetExitTrain(m_pMyVehicle);
							else
#endif
							if (m_pMyVehicle->IsBoat())
								SetExitBoat(m_pMyVehicle);

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
			case OBJECTIVE_USE_SEAT_ATTRACTOR:
			case OBJECTIVE_USE_ATM_ATTRACTOR:
			case OBJECTIVE_USE_STOP_ATTRACTOR:
			case OBJECTIVE_USE_PIZZA_ATTRACTOR:
			case OBJECTIVE_USE_SHELTER_ATTRACTOR:
			case OBJECTIVE_USE_ICECREAM_ATTRACTOR:
				if (CTimer::GetTimeInMilliseconds() > m_objectiveTimer) {
					m_objectiveTimer = 0;
					if (m_attractor)
						GetPedAttractorManager()->DeRegisterPed(this, m_attractor);
				}
				else {
					CVector distance = m_nextRoutePointPos - GetPosition();
					distance.z = 0.0f;
					if (m_objective == OBJECTIVE_USE_SHELTER_ATTRACTOR) {
						if (m_nMoveState == PEDMOVE_SPRINT && distance.Magnitude() < SQR(2.0f)) {
							SetMoveState(PEDMOVE_WALK);
							bIsRunning = false;
						}
						else if (CWeather::Rain < 0.2f && m_attractor) {
							GetPedAttractorManager()->DeRegisterPed(this, m_attractor);
							return;
						}
					}
					else if (m_objective == OBJECTIVE_USE_ICECREAM_ATTRACTOR) {
						if (m_nMoveState == PEDMOVE_SPRINT && distance.Magnitude() < SQR(4.0f)) {
							SetMoveState(PEDMOVE_WALK);
							bIsRunning = false;
						}
						CVehicle* pIceCreamVan = GetPedAttractorManager()->GetIceCreamVanForEffect(m_attractor->GetEffect());
						if (0.01f * CTimer::GetTimeStep() * 5.0f < pIceCreamVan->m_fDistanceTravelled) {
							GetPedAttractorManager()->DeRegisterPed(this, m_attractor);
							return;
						}
						if (!pIceCreamVan->pDriver ||
							!pIceCreamVan->pDriver->IsPlayer() ||
							pIceCreamVan->pDriver->GetPedState() == PED_ARRESTED ||
							pIceCreamVan->pDriver->GetPedState() == PED_DEAD) {
							GetPedAttractorManager()->DeRegisterPed(this, m_attractor);
							return;
						}
						if (!pIceCreamVan->m_bSirenOrAlarm) {
							GetPedAttractorManager()->DeRegisterPed(this, m_attractor);
							return;
						}
						if (pIceCreamVan->GetStatus() == STATUS_WRECKED) {
							GetPedAttractorManager()->DeRegisterPed(this, m_attractor);
							return;
						}
					}
					if (sq(m_distanceToCountSeekDone) < distance.MagnitudeSqr()) {
						if (CTimer::GetTimeInMilliseconds() > m_nPedStateTimer || GetPedState() != PED_SEEK_POS)
							SetSeek(m_vecSeekPos, m_distanceToCountSeekDone);
					}
					else {
						if (!bReachedAttractorHeadingTarget) {
							float fHeadingDistance = m_fRotationCur - m_attractorHeading;
							float fSinHeading = Sin(fHeadingDistance);
							float fCosHeading = Cos(fHeadingDistance);
							if (fSinHeading > 0.0f) {
								if (fCosHeading > 0.0f)
									m_attractorHeading = m_fRotationCur - Asin(fSinHeading);
								else
									m_attractorHeading = m_fRotationCur - Acos(fCosHeading);
							}
							else {
								if (fCosHeading > 0.0f)
									m_attractorHeading = m_fRotationCur - Asin(fSinHeading);
								else
									m_attractorHeading = m_fRotationCur + Acos(fCosHeading);
							}
							m_fRotationDest = m_attractorHeading;
							m_headingRate = 3.5f;
							bReachedAttractorHeadingTarget = true;
							bTurnedAroundOnAttractor = false;
						}
						if (Abs(m_fRotationCur - m_attractorHeading) >= m_acceptableHeadingOffset &&
							Abs(m_fRotationCur - m_attractorHeading + TWOPI) >= m_acceptableHeadingOffset &&
							Abs(m_fRotationCur - m_attractorHeading - TWOPI) >= m_acceptableHeadingOffset)
							SetMoveState(PEDMOVE_STILL);
						else {
							m_fRotationDest = m_fRotationCur;
							bReachedAttractorHeadingTarget = false;
							bObjectiveCompleted = true;
							bScriptObjectiveCompleted = true;
							RestoreHeadingRate();
							GetPedAttractorManager()->BroadcastArrival(this, m_attractor);
							if (GetPedAttractorManager()->IsAtHeadOfQueue(this, m_attractor)) {
								switch (m_objective) {
								case OBJECTIVE_USE_SEAT_ATTRACTOR:
									if (!bTurnedAroundOnAttractor) {
										ClearObjective();
										SetWaitState(WAITSTATE_SIT_DOWN, 0);
									}
									else {
										ClearObjective();
										SetWaitState(WAITSTATE_SIT_DOWN_RVRS, 0);
									}
									break;
								case OBJECTIVE_USE_ATM_ATTRACTOR:
									ClearObjective();
									SetWaitState(WAITSTATE_USE_ATM, 0);
									break;
								case OBJECTIVE_USE_STOP_ATTRACTOR:
									ClearObjective();
									SetObjective(OBJECTIVE_WAIT_FOR_BUS);
									break;
								case OBJECTIVE_USE_PIZZA_ATTRACTOR:
									ClearObjective();
									m_prevObjective = OBJECTIVE_NONE;
									SetObjective(OBJECTIVE_IDLE);
									m_objectiveTimer = CTimer::GetTimeInMilliseconds() + m_attractor->GetHeadOfQueueWaitTime();
									break;
								case OBJECTIVE_USE_SHELTER_ATTRACTOR:
									m_prevObjective = OBJECTIVE_NONE;
									SetObjective(OBJECTIVE_WAIT_FOR_RAIN_TO_END);
									break;
								case OBJECTIVE_USE_ICECREAM_ATTRACTOR:
									m_prevObjective = OBJECTIVE_NONE;
									SetObjective(OBJECTIVE_PURCHASE_ICECREAM);
									break;
								}
							}
						}
					}
				}
				return;
			case OBJECTIVE_WAIT_FOR_RAIN_TO_END:
				SetIdle();
				if (m_attractor && CWeather::Rain < 0.2f)
					GetPedAttractorManager()->DeRegisterPed(this, m_attractor);
				break;
			case OBJECTIVE_WAIT_FOR_BUS:
				SetIdle();
				if (m_attractor) {
					float left = GetPosition().x - 10.0f;
					float right = GetPosition().x + 10.0f;
					float top = GetPosition().y - 10.0f;
					float bottom = GetPosition().y + 10.0f;
					int xstart = Max(0, CWorld::GetSectorIndexX(left));
					int xend = Min(NUMSECTORS_X - 1, CWorld::GetSectorIndexX(right));
					int ystart = Max(0, CWorld::GetSectorIndexY(top));
					int yend = Min(NUMSECTORS_Y - 1, CWorld::GetSectorIndexY(bottom));
					assert(xstart <= xend);
					assert(ystart <= yend);

					float minDistance = SQR(10.0f);
					CVehicle* pBus = nil;

					for (int y = ystart; y <= yend; y++) {
						for (int x = xstart; x <= xend; x++) {
							CSector* s = CWorld::GetSector(x, y);
							for (CPtrNode* pNode = s->m_lists[ENTITYLIST_VEHICLES].first; pNode != nil; pNode = pNode->next) {
								CEntity* pEntity = (CEntity*)pNode->item;
								if (!pEntity->IsVehicle())
									continue;
								CVehicle* pVehicle = (CVehicle*)pEntity;
								if (!pVehicle->bIsBus)
									continue;
								if (pVehicle->GetMoveSpeed().MagnitudeSqr() >= SQR(0.005f))
									continue;
								float distanceSq = (GetPosition() - pVehicle->GetPosition()).MagnitudeSqr();
								if (distanceSq < minDistance) {
									minDistance = distanceSq;
									pBus = pVehicle;
								}
							}
						}
					}

					if (pBus) {
						if (pBus->m_nNumPassengers >= pBus->m_nNumMaxPassengers - 1)
							SetObjective(OBJECTIVE_IDLE);
						else {
							GetPedAttractorManager()->DeRegisterPed(this, m_attractor);
							SetObjective(OBJECTIVE_ENTER_CAR_AS_PASSENGER, pBus);
							bDontDragMeOutCar = true; // TODO(MIAMI): check, add more flags
						}
					}
				}
				break;
#endif
		}
		if (bObjectiveCompleted
			|| m_objectiveTimer != 0 && CTimer::GetTimeInMilliseconds() > m_objectiveTimer) {
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
CPed::SetShootTimer(uint32 time)
{
	if (CTimer::GetTimeInMilliseconds() > m_shootTimer) {
		m_shootTimer = CTimer::GetTimeInMilliseconds() + time;
	}
}

void
CPed::SetSeekCar(CVehicle *car, uint32 doorNode)
{
	if (m_nPedState == PED_SEEK_CAR)
		return;

	SetStoredState();
	m_pSeekTarget = car;
	m_pSeekTarget->RegisterReference((CEntity**) &m_pSeekTarget);
	m_carInObjective = car;
	m_carInObjective->RegisterReference((CEntity**) &m_carInObjective);
	m_pMyVehicle = car;
	m_pMyVehicle->RegisterReference((CEntity**) &m_pMyVehicle);
	// m_pSeekTarget->RegisterReference((CEntity**) &m_pSeekTarget);
	m_vehEnterType = doorNode;
	m_distanceToCountSeekDone = 0.5f;
	m_nPedState = PED_SEEK_CAR;

}

void
CPed::SetSeekBoatPosition(CVehicle *boat)
{
	if (m_nPedState == PED_SEEK_IN_BOAT || boat->pDriver
#ifdef VC_PED_PORTS
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
	m_nPedState = PED_SEEK_IN_BOAT;
}

#ifdef GTA_TRAIN
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
	m_nPedState = PED_EXIT_TRAIN;
	m_pVehicleAnim = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_TRAIN_GETOUT, 4.0f);
	m_pVehicleAnim->SetFinishCallback(PedSetOutTrainCB, this);
	bUsesCollision = false;
	LineUpPedWithTrain();
}
#endif

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

#ifdef TOGGLEABLE_BETA_FEATURES
	if (!bMakePedsRunToPhonesToReportCrimes)
#endif
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
			|| m_objective == OBJECTIVE_SOLICIT) {
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
				if (goingToEnterCar && (m_vehEnterType == CAR_DOOR_LF || m_vehEnterType == CAR_DOOR_LR)) {
					cornerToGo = tl;
					m_walkAroundType = 1;

					if (m_vehEnterType == CAR_DOOR_LR)
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
					if (goingToEnterCar && (m_vehEnterType == CAR_DOOR_RF || m_vehEnterType == CAR_DOOR_RR)) {
						cornerToGo = tr;
						m_walkAroundType = 2;

						if (m_vehEnterType == CAR_DOOR_RR)
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
					if (goingToEnterCar && (m_vehEnterType == CAR_DOOR_RF || m_vehEnterType == CAR_DOOR_RR)) {
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
					if (goingToEnterCar && (m_vehEnterType == CAR_DOOR_LF || m_vehEnterType == CAR_DOOR_LR)) {
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
						if (goingToEnterCar && (m_vehEnterType == CAR_DOOR_RF || m_vehEnterType == CAR_DOOR_RR)) {
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
						if (goingToEnterCar && (m_vehEnterType == CAR_DOOR_LF || m_vehEnterType == CAR_DOOR_LR)) {
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
				} else if (goingToEnterCar && (m_vehEnterType == CAR_DOOR_LF || m_vehEnterType == CAR_DOOR_LR)
					|| CGeneral::LimitRadianAngle(m_fRotationDest - angleToFaceObjCenter) < 0.0f) {
					if (entityOnTopLeftOfObj == 1 || entityOnTopLeftOfObj && !entityOnTopRightOfObj && !entityOnBottomRightOfObj) {
						m_walkAroundType = 3;
					}
				} else if (entityOnTopRightOfObj == 1 || entityOnTopRightOfObj && !entityOnTopLeftOfObj && !entityOnBottomLeftOfObj) {
					m_walkAroundType = 4;
				}
			} else if (goingToEnterCar && (m_vehEnterType == CAR_DOOR_LF || m_vehEnterType == CAR_DOOR_LR)
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

	CVector nextPosToHead = objMat * LocalPosForWalkAround(adjustedColMin, adjustedColMax, nextWalkAround, goingToEnterCar ? m_vehEnterType : 0, goingToEnterCarAndItsVan);
	bool nextRouteIsClear = CWorld::GetIsLineOfSightClear(GetPosition(), nextPosToHead, true, true, true, true, true, true, false);

	if(nextRouteIsClear)
		m_walkAroundType = nextWalkAround;
	else {
		CVector posToHead = objMat * LocalPosForWalkAround(adjustedColMin, adjustedColMax, m_walkAroundType, goingToEnterCar ? m_vehEnterType : 0, goingToEnterCarAndItsVan);
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

	localPosToHead = LocalPosForWalkAround(adjustedColMin, adjustedColMax, m_walkAroundType, goingToEnterCar ? m_vehEnterType : 0, goingToEnterCarAndItsVan);
#else
	if (Abs(angleDiffBtwObjCenterAndForward) < objTopRightHeading) {
		if (goingToEnterCar) {
			if (goingToEnterCarAndItsVan) {
				if (m_vehEnterType == CAR_DOOR_LR || m_vehEnterType == CAR_DOOR_RR)
					return;
			}
			if (m_vehEnterType != CAR_DOOR_LF && m_vehEnterType != CAR_DOOR_LR && (!entityOnBottomRightOfObj || entityOnBottomLeftOfObj)) {
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
				if (!goingToEnterCar || !goingToEnterCarAndItsVan || m_vehEnterType != CAR_DOOR_LR && m_vehEnterType != CAR_DOOR_RR) {
					if (goingToEnterCar) {
						if (m_vehEnterType == CAR_DOOR_RF || (m_vehEnterType == CAR_DOOR_RR && !goingToEnterCarAndItsVan))
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
			} else if (goingToEnterCar && goingToEnterCarAndItsVan && (m_vehEnterType == CAR_DOOR_LR || m_vehEnterType == CAR_DOOR_RR)) {
				m_fRotationDest = CGeneral::LimitRadianAngle(PI + dirToSet);
				localPosToHead.x = adjustedColMin.x;
				localPosToHead.z = 0.0f;
				localPosToHead.y = adjustedColMin.y;
			} else {
				if (goingToEnterCar) {
					if (m_vehEnterType == CAR_DOOR_LF || m_vehEnterType == CAR_DOOR_LR && !goingToEnterCarAndItsVan)
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
			if (goingToEnterCar && (!goingToEnterCarAndItsVan || m_vehEnterType != CAR_DOOR_LR && m_vehEnterType != CAR_DOOR_RR)) {
				if (m_vehEnterType != CAR_DOOR_LF && m_vehEnterType != CAR_DOOR_LR && (!entityOnTopRightOfObj || entityOnTopLeftOfObj)) {

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
					if(!bHeadStuckInCollision || FEET_OFFSET + intersectionPoint.point.z < GetPosition().z) {
						GetMatrix().GetPosition().z = FEET_OFFSET + intersectionPoint.point.z;
						if (bHeadStuckInCollision)
							bHeadStuckInCollision = false;
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
							if (!bHeadStuckInCollision || FEET_OFFSET + intersectionPoint.point.z < GetPosition().z) {
								GetMatrix().GetPosition().z = FEET_OFFSET + intersectionPoint.point.z;
								if (bHeadStuckInCollision)
									bHeadStuckInCollision = false;
							}
#else
							GetMatrix().GetPosition().z = FEET_OFFSET + intersectionPoint.point.z;
#endif
							m_nSurfaceTouched = intersectionPoint.surfaceB;
							if (m_nSurfaceTouched == SURFACE_STONE) {
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
						if (!bWasStanding) {
							if ((speed <= upperSpeedLimit /* || (bfFlagsL >> 5) & 1 */) && m_vecMoveSpeed.z >= lowerSpeedLimit
								|| m_pCollidingEntity == collidingEnt) {

								if (RpAnimBlendClumpGetAssociation(GetClump(), ANIM_FALL_FALL) && -0.016f * CTimer::GetTimeStep() > m_vecMoveSpeed.z) {
									InflictDamage(collidingEnt, WEAPONTYPE_FALL, 15.0f, PEDPIECE_TORSO, 2);
								}
							} else {
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
							}
						}
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

		if (ourCollidedSpheres > 0 && (collidingEnt->IsBuilding() || collidingEnt->IsStatic())) {
			bHasHitWall = true;
		}
	}
	if (collidingEnt->IsBuilding() || collidingEnt->IsStatic()) 	{

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
					bHeadStuckInCollision = true;
				}
#endif
				sphereNormal.Normalise();
				collidingPoints[sphere].normal = sphereNormal;
				if (collidingPoints[sphere].surfaceB == SURFACE_STONE)
					bHitSteepSlope = true;
			}
		}
	}
	return ourCollidedSpheres;
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

// "Wander range" state is unused in game, and you can't use it without SetWanderRange anyway
void
CPed::WanderRange(void)
{
	bool arrived = Seek();
	if (arrived) {
		Idle();
		if (((m_randomSeed % 256) + 3 * CTimer::GetFrameCounter()) % 1000 > 997) {

			int xDiff = Abs(m_wanderRangeBounds[1].x - m_wanderRangeBounds[0].x);
			int yDiff = Abs(m_wanderRangeBounds[1].y - m_wanderRangeBounds[0].y);

			CVector newCoords(
				(CGeneral::GetRandomNumber() % xDiff) + m_wanderRangeBounds[0].x,
				(CGeneral::GetRandomNumber() % yDiff) + m_wanderRangeBounds[0].y,
				GetPosition().z);

			SetSeek(newCoords, 2.5f);
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

#ifdef GTA_TRAIN
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

	m_nPedState = PED_ENTER_TRAIN;
	m_pVehicleAnim = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_TRAIN_GETIN, 4.0f);
	m_pVehicleAnim->SetFinishCallback(PedSetInTrainCB, this);
	bUsesCollision = false;
	LineUpPedWithTrain();
	if (IsPlayer()) {
		if (((CPlayerPed*)this)->m_bAdrenalineActive)
			((CPlayerPed*)this)->ClearAdrenaline();
	}
}
#endif

// --MIAMI: Done, but what is this parameter for?
void
CPed::SetDuck(uint32 time, bool sth)
{
	if (bIsDucking || CTimer::GetTimeInMilliseconds() <= m_duckTimer && !sth) {
		if (sth && CTimer::GetTimeInMilliseconds() + time > m_duckTimer)
			m_duckTimer = CTimer::GetTimeInMilliseconds() + time;
		return;
	}

	CAnimBlendAssociation *duckAssoc;
	if (bCrouchWhenShooting) {
		duckAssoc = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_WEAPON_CROUCH, 4.0f);
		duckAssoc->flags &= ~ASSOC_FADEOUTWHENDONE;
		bIsDucking = true;
		m_duckTimer = CTimer::GetTimeInMilliseconds() + time;
	} else {
		CAnimBlendAssociation *duckAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_DUCK_DOWN);
		if (!duckAssoc || duckAssoc->blendDelta < 0.0f) {
			CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_DUCK_DOWN, 4.0f);
			bIsDucking = true;
			m_duckTimer = CTimer::GetTimeInMilliseconds() + time;
		}
	}
}

void
CPed::SeekBoatPosition(void)
{
	if (m_carInObjective && !m_carInObjective->pDriver) {
		CVehicleModelInfo *boatModel = m_carInObjective->GetModelInfo();

		CVector enterOffset;
		if (boatModel->m_vehicleType == VEHICLE_TYPE_BOAT)
			enterOffset = boatModel->m_positions[BOAT_POS_FRONTSEAT];
		else
			enterOffset = boatModel->m_positions[CAR_POS_FRONTSEAT];

		enterOffset.x = 0.0f;
		CMatrix boatMat(m_carInObjective->GetMatrix());
		SetMoveState(PEDMOVE_WALK);
		m_vecSeekPos = boatMat * enterOffset;
		if (Seek()) {
			// We arrived to the boat
			m_vehEnterType = 0;
			SetEnterCar(m_carInObjective, 0);
		}
	} else
		RestorePreviousState();
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
		switch (m_vehEnterType) {
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
			SetEnterCar_AllClear(car, m_vehEnterType, doorFlag);
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

inline bool
CPed::IsNotInWreckedVehicle()
{
	return m_pMyVehicle != nil && m_pMyVehicle->GetStatus() != STATUS_WRECKED;
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
				if (m_nPedState != PED_FALL && !DyingOrDead() && m_nPedState != PED_ATTACK && m_nPedState != PED_FIGHT) {
					if (!IsPedHeadAbovePos(0.3f) || RpAnimBlendClumpGetAssociation(GetClump(), ANIM_IDLE_TIRED)) {
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
CPed::ProcessBuoyancy(void)
{
	static uint32 nGenerateRaindrops = 0;
	static uint32 nGenerateWaterCircles = 0;
	CRGBA color(((0.5f * CTimeCycle::GetDirectionalRed() + CTimeCycle::GetAmbientRed()) * 127.5f),
		((0.5f * CTimeCycle::GetDirectionalBlue() + CTimeCycle::GetAmbientBlue()) * 127.5f),
		((0.5f * CTimeCycle::GetDirectionalGreen() + CTimeCycle::GetAmbientGreen()) * 127.5f),
		(CGeneral::GetRandomNumber() % 256 * 48.0f) + 48);

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
				if (buoyancyImpulse.z / m_fMass > 0.0032f * CTimer::GetTimeStep()) {
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
						m_nPedState = PED_IDLE;
						return;
					}
				}
			}
			float speedMult = 0.0f;
			if (buoyancyImpulse.z / m_fMass > 0.006f * CTimer::GetTimeStep()
				|| mod_Buoyancy.m_waterlevel > GetPosition().z) {
				speedMult = pow(0.9f, CTimer::GetTimeStep());
				m_vecMoveSpeed.x *= speedMult;
				m_vecMoveSpeed.y *= speedMult;
				m_vecMoveSpeed.z *= speedMult;
				bIsStanding = false;
				bIsDrowning = true;
				InflictDamage(nil, WEAPONTYPE_DROWNING, 3.0f * CTimer::GetTimeStep(), PEDPIECE_TORSO, 0);
			}
			if (buoyancyImpulse.z / m_fMass > 0.002f * CTimer::GetTimeStep()) {
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
CPed::SetSolicit(uint32 time)
{
	if (m_nPedState == PED_SOLICIT || !IsPedInControl() || !m_carInObjective)
		return;

	if (CharCreatedBy != MISSION_CHAR && m_carInObjective->m_nNumGettingIn == 0
		&& CTimer::GetTimeInMilliseconds() < m_objectiveTimer) {
		if (m_vehEnterType == CAR_DOOR_LF) {
			m_fRotationDest = m_carInObjective->GetForward().Heading() - HALFPI;
		} else {
			m_fRotationDest = m_carInObjective->GetForward().Heading() + HALFPI;
		}

		if (Abs(m_fRotationDest - m_fRotationCur) < HALFPI) {
			m_standardTimer = CTimer::GetTimeInMilliseconds() + time;

			if(!m_carInObjective->bIsVan && !m_carInObjective->bIsBus)
				m_pVehicleAnim = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_CAR_HOOKERTALK, 4.0f);

			m_nPedState = PED_SOLICIT;
		}
	}
}

bool
CPed::SetFollowPath(CVector dest, float radius, eMoveState state, CEntity* pFollowedPed, CEntity*, int time)
{
	// TODO(MIAMI): new follow
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
	m_nPedState = PED_FOLLOW_PATH;
	SetMoveState(PEDMOVE_WALK);
	return true;
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
	if (someoneEntersFromOurExitDoor && m_objective == OBJECTIVE_LEAVE_VEHICLE) {
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
		m_vehEnterType = optedDoorNode;
		m_nPedState = PED_EXIT_CAR;
		if (m_pVehicleAnim && m_pVehicleAnim->flags & ASSOC_PARTIAL)
			m_pVehicleAnim->blendDelta = -1000.0f;
		SetMoveState(PEDMOVE_NONE);
		CAnimManager::BlendAnimation(GetClump(), m_animGroup, ANIM_IDLE_STANCE, 100.0f);
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
				if (veh->GetModelIndex() == MI_VOODOO)
					addDoorSmoke = true;

				switch (m_vehEnterType) {
					case CAR_DOOR_RF:
						if (veh->bIsBus) {
							m_pVehicleAnim = CAnimManager::AddAnimation(GetClump(), ASSOCGRP_COACH, ANIM_COACH_OUT_L);
						} else {
							if (isLow)
								m_pVehicleAnim = CAnimManager::AddAnimation(GetClump(), ASSOCGRP_STD, ANIM_CAR_GETOUT_LOW_RHS);
							else
								m_pVehicleAnim = CAnimManager::AddAnimation(GetClump(), ASSOCGRP_STD, ANIM_CAR_GETOUT_RHS);

							if (addDoorSmoke)
								AddYardieDoorSmoke(veh, CAR_DOOR_RF);
						}
						break;
					case CAR_DOOR_RR:
						if (veh->bIsVan) {
							m_pVehicleAnim = CAnimManager::AddAnimation(GetClump(), ASSOCGRP_VAN, ANIM_VAN_GETOUT);
						} else if (isLow) {
							m_pVehicleAnim = CAnimManager::AddAnimation(GetClump(), ASSOCGRP_STD, ANIM_CAR_GETOUT_LOW_RHS);
						} else {
							m_pVehicleAnim = CAnimManager::AddAnimation(GetClump(), ASSOCGRP_STD, ANIM_CAR_GETOUT_RHS);
						}
						break;
					case CAR_DOOR_LF:
						if (veh->bIsBus) {
							m_pVehicleAnim = CAnimManager::AddAnimation(GetClump(), ASSOCGRP_COACH, ANIM_COACH_OUT_L);
						} else {
							if (isLow)
								m_pVehicleAnim = CAnimManager::AddAnimation(GetClump(), ASSOCGRP_STD, ANIM_CAR_GETOUT_LOW_LHS);
							else
								m_pVehicleAnim = CAnimManager::AddAnimation(GetClump(), ASSOCGRP_STD, ANIM_CAR_GETOUT_LHS);

							if (addDoorSmoke)
								AddYardieDoorSmoke(veh, CAR_DOOR_LF);
						}
						break;
					case CAR_DOOR_LR:
						if (veh->bIsVan) {
							m_pVehicleAnim = CAnimManager::AddAnimation(GetClump(), ASSOCGRP_VAN, ANIM_VAN_GETOUT_L);
						} else if (isLow) {
							m_pVehicleAnim = CAnimManager::AddAnimation(GetClump(), ASSOCGRP_STD, ANIM_CAR_GETOUT_LOW_LHS);
						} else {
							m_pVehicleAnim = CAnimManager::AddAnimation(GetClump(), ASSOCGRP_STD, ANIM_CAR_GETOUT_LHS);
						}
						break;
					default:
						break;
				}
				if (!bBusJacked) {
					switch (m_vehEnterType) {
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
				if (m_vehEnterType == CAR_DOOR_RF || m_vehEnterType == CAR_DOOR_RR) {
					m_pVehicleAnim = CAnimManager::AddAnimation(GetClump(), ASSOCGRP_STD, ANIM_CAR_CRAWLOUT_RHS2);
				} else if (m_vehEnterType == CAR_DOOR_LF || m_vehEnterType == CAR_DOOR_LR) {
					m_pVehicleAnim = CAnimManager::AddAnimation(GetClump(), ASSOCGRP_STD, ANIM_CAR_CRAWLOUT_RHS);
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

	if (m_nPedType == PEDTYPE_CRIMINAL && m_hitRecoverTimer < CTimer::GetTimeInMilliseconds()) {
		if (CGeneral::GetRandomNumber() % 100 >= 10) {
			if (m_objective != OBJECTIVE_MUG_CHAR && !(CGeneral::GetRandomNumber() & 7)) {
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

				m_hitRecoverTimer = CTimer::GetTimeInMilliseconds() + 5000;
			}
		} else {
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
				m_hitRecoverTimer = CTimer::GetTimeInMilliseconds() + 5000;
				return;
			}
			m_hitRecoverTimer = CTimer::GetTimeInMilliseconds() + 5000;
		}
	}

	if (m_nPedState == PED_WANDER_PATH) {
#ifndef VC_PED_PORTS
		if (CTimer::GetTimeInMilliseconds() > m_standardTimer) {

			// += 2 is weird
			for (int i = 0; i < m_numNearPeds; i += 2) {
				if (m_nearPeds[i]->m_nPedState == PED_WANDER_PATH && WillChat(m_nearPeds[i])) {
					if (CGeneral::GetRandomNumberInRange(0, 100) >= 100)
						m_standardTimer = CTimer::GetTimeInMilliseconds() + 30000;
					else {
						if ((GetPosition() - m_nearPeds[i]->GetPosition()).Magnitude() >= 1.8f) {
							m_standardTimer = CTimer::GetTimeInMilliseconds() + 30000;
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
		if (CGeneral::GetRandomNumberInRange(0.0f, 1.0f) >= 0.5f) {
			m_standardTimer = CTimer::GetTimeInMilliseconds() + 200;
		} else {
			if (CTimer::GetTimeInMilliseconds() > m_standardTimer) {
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
						SetObjective(OBJECTIVE_SOLICIT, veh);
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

	CPed *deadPed = nil;
	if (fearFlags & PED_FLAG_DEADPEDS && CharCreatedBy != MISSION_CHAR
		&& (deadPed = CheckForDeadPeds()) != nil && (deadPed->GetPosition() - ourPos).MagnitudeSqr() < sq(20.0f)) {
		m_pEventEntity = deadPed;
		m_pEventEntity->RegisterReference((CEntity **) &m_pEventEntity);
		return PED_FLAG_DEADPEDS;
	} else {
		uint32 flagsOfSomePed = 0;

		CPed *pedToFearFrom = nil;
#ifndef VC_PED_PORTS
		for (int i = 0; i < m_numNearPeds; i++) {
			if (CharCreatedBy != RANDOM_CHAR || m_nearPeds[i]->CharCreatedBy != MISSION_CHAR || m_nearPeds[i]->IsPlayer()) {
				CPed *nearPed = m_nearPeds[i];

				// BUG: WTF Rockstar?! Putting this here will result in returning the flags of farthest ped to us, since m_nearPeds is sorted by distance.
				// Fixed at the bottom of the function.
				flagsOfSomePed = CPedType::GetFlag(nearPed->m_nPedType);

				if (CPedType::GetFlag(nearPed->m_nPedType) & fearFlags) {
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
				flagsOfSomePed = CPedType::GetFlag(m_nearPeds[i]->m_nPedType);

				if (flagsOfSomePed & fearFlags) {
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
				flagsOfSomePed = CPedType::GetFlag(driver->m_nPedType);
				if (CPedType::GetFlag(driver->m_nPedType) & fearFlags) {
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
			flagsOfSomePed = CPedType::GetFlag(((CPed*)m_threatEntity)->m_nPedType);
		else
			flagsOfSomePed = 0;
#endif

		return flagsOfSomePed;
	}
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
		if (m_vehEnterType && m_objective != OBJECTIVE_ENTER_CAR_AS_DRIVER) {
			if (IsRoomToBeCarJacked()) {
				dest = GetPositionToOpenCarDoor(vehToSeek, m_vehEnterType);
			} else if (m_nPedType == PEDTYPE_COP) {
				dest = GetPositionToOpenCarDoor(vehToSeek, CAR_DOOR_RF);
			} else {
				SetMoveState(PEDMOVE_STILL);
			}
		} else
			GetNearestDoor(vehToSeek, dest);
	} else {
		if (m_hitRecoverTimer > CTimer::GetTimeInMilliseconds()) {
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
				m_hitRecoverTimer = CTimer::GetTimeInMilliseconds() + 30000;
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

	if (vehToSeek->m_nGettingInFlags & GetCarDoorFlag(m_vehEnterType))
		bVehEnterDoorIsBlocked = true;
	else
		bVehEnterDoorIsBlocked = false;

	// Arrived to the car
	if (Seek()) {
		if (!foundBetterPosToSeek) {
			if (1.5f + GetPosition().z > dest.z && GetPosition().z - 0.5f < dest.z) {
#ifdef GTA_TRAIN
				if (vehToSeek->IsTrain()) {
					SetEnterTrain(vehToSeek, m_vehEnterType);
				} else
#endif
				{
					m_fRotationCur = m_fRotationDest;
					if (!bVehEnterDoorIsBlocked) {
						vehToSeek->bIsStatic = false;
						if (m_objective == OBJECTIVE_SOLICIT) {
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
										(m_vehEnterType == CAR_DOOR_LF && vehToSeek->pDriver || m_vehEnterType == CAR_DOOR_RF && vehToSeek->pPassengers[0] || m_vehEnterType == CAR_DOOR_LR && vehToSeek->pPassengers[1] || m_vehEnterType == CAR_DOOR_RR && vehToSeek->pPassengers[2])) {
										SetCarJack(vehToSeek);
										if (m_objective == OBJECTIVE_ENTER_CAR_AS_DRIVER && m_vehEnterType != CAR_DOOR_LF)
											vehToSeek->pDriver->bFleeAfterExitingCar = true;
									} else {
										SetEnterCar(vehToSeek, m_vehEnterType);
									}
									break;
								case STATUS_ABANDONED:
									if (m_vehEnterType == CAR_DOOR_RF && vehToSeek->pPassengers[0]) {
										if (vehToSeek->pPassengers[0]->bDontDragMeOutCar) {
											if (IsPlayer())
												CPed::SetEnterCar(vehToSeek, m_vehEnterType);
										} else {
											SetCarJack(vehToSeek);
										}
									} else {
										SetEnterCar(vehToSeek, m_vehEnterType);
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

void
CPed::ServiceTalking(void)
{
	if (!bBodyPartJustCameOff || m_bodyPartBleeding != PED_HEAD) {
		if (CGeneral::faststricmp(CModelInfo::GetModelInfo(GetModelIndex())->GetName(), "bomber")) {
			if (m_nPedState == PED_ON_FIRE)
				m_queuedSound = SOUND_PED_BURNING;
		} else {
			m_queuedSound = SOUND_PED_BOMBER;
		}
		if (m_queuedSound != SOUND_NO_SOUND) {
			if (m_queuedSound == SOUND_PED_DEATH)
				m_soundStart = CTimer::GetTimeInMilliseconds() - 1;

			if (CTimer::GetTimeInMilliseconds() > m_soundStart) {
				DMAudio.PlayOneShot(m_audioEntityId, m_queuedSound, 1.0f);
				m_lastSoundStart = CTimer::GetTimeInMilliseconds();
				m_soundStart =
					CommentWaitTime[m_queuedSound - SOUND_PED_DEATH].m_nFixedDelayTime
					+ CTimer::GetTimeInMilliseconds()
					+ CGeneral::GetRandomNumberInRange(0, CommentWaitTime[m_queuedSound - SOUND_PED_DEATH].m_nOverrideFixedDelayTime);
				m_lastQueuedSound = m_queuedSound;
				m_queuedSound = SOUND_NO_SOUND;
			}
		}
	}
}

void
CPed::StartFightDefend(uint8 direction, uint8 hitLevel, uint8 unk)
{
	if (m_nPedState == PED_DEAD) {
		if (CGame::nastyGame) {
			if (hitLevel == HITLEVEL_GROUND) {
				CAnimBlendAssociation *floorHitAssoc;
				if (RpAnimBlendClumpGetFirstAssociation(GetClump(), ASSOC_FRONTAL)) {
					floorHitAssoc = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_FLOOR_HIT_F, 8.0f);
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
				CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_FLOOR_HIT_F, 8.0f) :
				CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_FLOOR_HIT, 8.0f);
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
							shotAnim = ANIM_SHOT_LEFT_PARTIAL;
							break;
						case 2:
							shotAnim = ANIM_SHOT_BACK_PARTIAL;
							break;
						case 3:
							shotAnim = ANIM_SHOT_RIGHT_PARTIAL;
							break;
						default:
							shotAnim = ANIM_SHOT_FRONT_PARTIAL;
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
						SetFall(500, ANIM_KO_SPIN_R, false);
						break;
					case 2:
						SetFall(500, ANIM_KO_SKID_BACK, false);
						break;
					case 3:
						SetFall(500, ANIM_KO_SPIN_L, false);
						break;
					default:
						SetFall(500, ANIM_KO_SHOT_STOM, false);
						break;
				}
#else
				bool fall = true;
				AnimationId hitAnim;
				switch (direction) {
					case 1:
						hitAnim = ANIM_KO_SPIN_R;
						break;
					case 2:
						if (CGeneral::GetRandomNumber() & 1) {
							fall = false;
							hitAnim = ANIM_HIT_BACK;
						} else	{
							hitAnim = ANIM_KO_SKID_BACK;
						}
						break;
					case 3:
						hitAnim = ANIM_KO_SPIN_L;
						break;
					default:
						if (hitLevel == HITLEVEL_LOW) {
							hitAnim = ANIM_KO_SHOT_STOM;
						} else if (CGeneral::GetRandomNumber() & 1) {
							fall = false;
							hitAnim = ANIM_HIT_WALK;
						} else if (CGeneral::GetRandomNumber() & 1) {
							fall = false;
							hitAnim = ANIM_HIT_HEAD;
						} else {
							hitAnim = ANIM_KO_SHOT_FACE;
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
					m_lastFightMove = FIGHTMOVE_HITONFLOOR;
					break;
				case HITLEVEL_LOW:
#ifndef VC_PED_PORTS
					if (direction == 2) {
						CPed::SetFall(1000, ANIM_KO_SKID_BACK, false);
						return;
					}
#else
					if (direction == 2 && (!IsPlayer() || ((CGeneral::GetRandomNumber() & 1) && m_fHealth < 30.0f))) {
						CPed::SetFall(1000, ANIM_KO_SKID_BACK, false);
						return;
					} else if (direction != 2 && !IsPlayer() && (CGeneral::GetRandomNumber() & 1) && m_fHealth < 30.0f) {
						CPed::SetFall(1000, ANIM_KO_SHOT_STOM, false);
						return;
					}	
#endif
					m_lastFightMove = FIGHTMOVE_HITBODY;
					break;
				case HITLEVEL_HIGH:
					switch (direction) {
						case 1:
							m_lastFightMove = FIGHTMOVE_HITLEFT;
							break;
						case 2:
							m_lastFightMove = FIGHTMOVE_HITBACK;
							break;
						case 3:
							m_lastFightMove = FIGHTMOVE_HITRIGHT;
							break;
						default:
							if (unk <= 5)
								m_lastFightMove = FIGHTMOVE_HITHEAD;
							else
								m_lastFightMove = FIGHTMOVE_HITBIGSTEP;
							break;
					}
					break;
				default:
					switch (direction) {
						case 1:
							m_lastFightMove = FIGHTMOVE_HITLEFT;
							break;
						case 2:
							m_lastFightMove = FIGHTMOVE_HITBACK;
							break;
						case 3:
							m_lastFightMove = FIGHTMOVE_HITRIGHT;
							break;
						default:
							if (unk <= 5)
								m_lastFightMove = FIGHTMOVE_HITCHEST;
							else
								m_lastFightMove = FIGHTMOVE_HITBIGSTEP;
							break;
					}
					break;
			}
			if (m_nPedState == PED_GETUP && !IsPedHeadAbovePos(0.0f))
				m_lastFightMove = FIGHTMOVE_HITONFLOOR;

			if (m_nPedState == PED_FIGHT) {
				CAnimBlendAssociation *moveAssoc = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, tFightMoves[m_lastFightMove].animId, 8.0f);
				moveAssoc->SetCurrentTime(0.0f);
				moveAssoc->SetFinishCallback(FinishFightMoveCB, this);
				if (IsPlayer())
					moveAssoc->speed = 1.3f;

				m_takeAStepAfterAttack = 0;
				m_fightButtonPressure = 0;
			} else if (IsPlayer() && m_currentWeapon != WEAPONTYPE_UNARMED) {
				CAnimBlendAssociation *moveAssoc = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, tFightMoves[m_lastFightMove].animId, 4.0f);
				moveAssoc->SetCurrentTime(0.0f);
				moveAssoc->speed = 1.3f;
			} else {
				if (m_nPedState != PED_AIM_GUN && m_nPedState != PED_ATTACK)
					SetStoredState();

				if (m_nWaitState != WAITSTATE_FALSE) {
					m_nWaitState = WAITSTATE_FALSE;
					RestoreHeadingRate();
				}
				m_nPedState = PED_FIGHT;
				m_fightButtonPressure = 0;
				RpAnimBlendClumpRemoveAssociations(GetClump(), ASSOC_REPEAT);
				CAnimBlendAssociation *walkStartAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_WALK_START);
				if (walkStartAssoc) {
					walkStartAssoc->flags |= ASSOC_DELETEFADEDOUT;
					walkStartAssoc->blendDelta = -1000.0f;
				}
				CAnimBlendAssociation *walkStopAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_RUN_STOP);
				if (!walkStopAssoc)
					walkStopAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_RUN_STOP_R);
				if (walkStopAssoc) {
					walkStopAssoc->flags |= ASSOC_DELETEFADEDOUT;
					walkStopAssoc->blendDelta = -1000.0f;
					RestoreHeadingRate();
				}
				SetMoveState(PEDMOVE_NONE);
				m_nStoredMoveState = PEDMOVE_NONE;
				CAnimManager::AddAnimation(GetClump(), ASSOCGRP_STD, ANIM_FIGHT_IDLE)->blendAmount = 1.0f;
				CAnimBlendAssociation *moveAssoc = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, tFightMoves[m_lastFightMove].animId, 8.0f);
				moveAssoc->SetFinishCallback(FinishFightMoveCB, this);
				m_fightState = FIGHTSTATE_NO_MOVE;
				m_takeAStepAfterAttack = false;
				bIsAttacking = true;
			}
		}
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
			SetObjective(OBJECTIVE_FLEE_TILL_SAFE);
			return;
		}
		if (!m_leader->bInVehicle) {
			if (m_leader->m_objective != OBJECTIVE_ENTER_CAR_AS_DRIVER) {
				if (bInVehicle) {
					if (m_objective != OBJECTIVE_WAIT_IN_CAR_THEN_GETOUT && m_objective != OBJECTIVE_LEAVE_VEHICLE)
						SetObjective(OBJECTIVE_LEAVE_VEHICLE, m_pMyVehicle);

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
		if (bInVehicle || !m_leader->bInVehicle || m_leader->m_nPedState != PED_DRIVING) {
			if (m_leader->m_objective != OBJECTIVE_NONE && (!m_leader->IsPlayer() || m_leader->m_objective != OBJECTIVE_IDLE)
				&& m_objective != m_leader->m_objective) {

				switch (m_leader->m_objective) {
					case OBJECTIVE_IDLE:
					case OBJECTIVE_FLEE_TILL_SAFE:
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
					case OBJECTIVE_FIGHT_CHAR:
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
			} else {
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
						if (m_leader->m_objective != OBJECTIVE_NONE || m_objective != OBJECTIVE_NONE
							|| m_leader->m_nPedState != PED_CHAT || m_nPedState != PED_CHAT) {

							SetObjective(OBJECTIVE_GOTO_CHAR_ON_FOOT, m_leader);
							SetObjectiveTimer(0);
						} else {
							SetObjective(OBJECTIVE_NONE);
						}
#endif
					}
					if (m_nPedState == PED_IDLE && m_leader->IsPlayer()) {
						if (ScanForThreats() && m_threatEntity) {
							m_pLookTarget = m_threatEntity;
							m_pLookTarget->RegisterReference((CEntity **) &m_pLookTarget);
							TurnBody();
							if (m_attackTimer < CTimer::GetTimeInMilliseconds() && !GetWeapon()->IsTypeMelee()) {
								m_pPointGunAt = m_threatEntity;
								if (m_threatEntity)
									m_threatEntity->RegisterReference((CEntity **) &m_pPointGunAt);
								SetAttack(m_threatEntity);
							}
						}
					}
				}
			}
		} else {
			if (m_objective != OBJECTIVE_ENTER_CAR_AS_PASSENGER && m_objective != OBJECTIVE_ENTER_CAR_AS_DRIVER) {
				if (m_leader->m_pMyVehicle->m_nNumPassengers < m_leader->m_pMyVehicle->m_nNumMaxPassengers)
					SetObjective(OBJECTIVE_ENTER_CAR_AS_PASSENGER, m_leader->m_pMyVehicle);
			}
		}
	} else if (bInVehicle) {
		if ((!m_leader->bInVehicle || m_leader->m_nPedState == PED_EXIT_CAR) && m_objective != OBJECTIVE_WAIT_IN_CAR_THEN_GETOUT) {

			switch (m_leader->m_objective) {
				case OBJECTIVE_ENTER_CAR_AS_PASSENGER:
				case OBJECTIVE_ENTER_CAR_AS_DRIVER:
					if (m_pMyVehicle == m_leader->m_pMyVehicle || m_pMyVehicle == m_leader->m_carInObjective)
						break;

					// fall through
				default:
					if (m_pMyVehicle && m_objective != OBJECTIVE_LEAVE_VEHICLE) {
#ifdef VC_PED_PORTS
						m_leaveCarTimer = CTimer::GetTimeInMilliseconds() + 250;
#endif
						SetObjective(OBJECTIVE_LEAVE_VEHICLE, m_pMyVehicle);
					}

					break;
			}
		}
	}
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
		CPhysical *curSurface = m_pCurrentPhysSurface;
		if (!IsPlayer() && m_pCurrentPhysSurface->IsVehicle() && ((CVehicle*)m_pCurrentPhysSurface)->IsBoat()) {

			// It seems R* didn't like m_vecOffsetFromPhysSurface for boats
			CVector offsetToSurface = GetPosition() - curSurface->GetPosition();
			offsetToSurface.z -= FEET_OFFSET;

			CVector surfaceMoveVelocity = curSurface->m_vecMoveSpeed;
			CVector surfaceTurnVelocity = CrossProduct(curSurface->m_vecTurnSpeed, offsetToSurface);

			// Also we use that weird formula instead of friction if it's boat
			float slideMult = -curSurface->m_vecTurnSpeed.MagnitudeSqr();
			velocityOfSurface = slideMult * offsetToSurface * CTimer::GetTimeStep() + (surfaceTurnVelocity + surfaceMoveVelocity);
			m_vecMoveSpeed.z = slideMult * offsetToSurface.z * CTimer::GetTimeStep() + (surfaceTurnVelocity.z + surfaceMoveVelocity.z);
		} else {
			velocityOfSurface = curSurface->GetSpeed(m_vecOffsetFromPhysSurface);
		}
		// Reminder: m_moved is displacement from walking/running.
		velocityChange = m_moved + velocityOfSurface - m_vecMoveSpeed;
		m_fRotationCur += curSurface->m_vecTurnSpeed.z * CTimer::GetTimeStep();
		m_fRotationDest += curSurface->m_vecTurnSpeed.z * CTimer::GetTimeStep();
	} else if (m_nSurfaceTouched != SURFACE_STONE || m_vecDamageNormal.x == 0.0f && m_vecDamageNormal.y == 0.0f) {
		velocityChange = m_moved - m_vecMoveSpeed;
	} else {
		// Ped got damaged by steep slope
		m_vecMoveSpeed = CVector(0.0f, 0.0f, -0.001f);
		// some kind of
		CVector2D reactionForce = m_vecDamageNormal * (1.0f / m_vecDamageNormal.Magnitude2D());

		velocityChange = 0.02f * reactionForce + m_moved;

		float reactionAndVelocityDotProd = DotProduct2D(reactionForce, velocityChange);
		// they're in same direction
		if (reactionAndVelocityDotProd < 0.0f) {
			velocityChange -= reactionAndVelocityDotProd * reactionForce;
		}
	}
	
	// Take time step into account
	if (m_pCurrentPhysSurface) {
		float speedChange = velocityChange.Magnitude();
		float changeMult = speedChange;
		if (m_nPedState != PED_DIE || !m_pCurrentPhysSurface->IsVehicle()) {
			if (!m_pCurrentPhysSurface->IsVehicle() || !((CVehicle*)m_pCurrentPhysSurface)->IsBoat())
				changeMult = 0.01f * CTimer::GetTimeStep();
		} else {
			changeMult = 0.002f * CTimer::GetTimeStep();
		}

		if (speedChange > changeMult) {
			velocityChange = velocityChange * (changeMult / speedChange);
		}
	}
	m_vecMoveSpeed.x += velocityChange.x;
	m_vecMoveSpeed.y += velocityChange.y;
}

void
CPed::SetPedPositionInCar(void)
{
	if (CReplay::IsPlayingBack())
		return;

	if (bChangedSeat) {
		bool notYet = false;
		if (RpAnimBlendClumpGetAssociation(GetClump(), ANIM_CAR_GETIN_LHS)
			|| RpAnimBlendClumpGetAssociation(GetClump(), ANIM_CAR_GETIN_LOW_LHS)
			|| RpAnimBlendClumpGetAssociation(GetClump(), ANIM_CAR_CLOSEDOOR_LHS)
			|| RpAnimBlendClumpGetAssociation(GetClump(), ANIM_CAR_CLOSEDOOR_LOW_LHS)
			|| RpAnimBlendClumpGetAssociation(GetClump(), ANIM_CAR_SHUFFLE_RHS)
			|| RpAnimBlendClumpGetAssociation(GetClump(), ANIM_CAR_LSHUFFLE_RHS)
			|| RpAnimBlendClumpGetAssociation(GetClump(), ANIM_VAN_CLOSE_L)
			|| RpAnimBlendClumpGetAssociation(GetClump(), ANIM_VAN_CLOSE)
			|| RpAnimBlendClumpGetAssociation(GetClump(), ANIM_VAN_GETIN_L)
			|| RpAnimBlendClumpGetAssociation(GetClump(), ANIM_VAN_GETIN)
			|| RpAnimBlendClumpGetAssociation(GetClump(), ANIM_COACH_IN_L)
			|| RpAnimBlendClumpGetAssociation(GetClump(), ANIM_COACH_IN_R)) {
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
		if (vehModel->m_vehicleType == VEHICLE_TYPE_BOAT)
			seatPos = vehModel->m_positions[BOAT_POS_FRONTSEAT];
		else
			seatPos = vehModel->m_positions[CAR_POS_FRONTSEAT];

		if (!m_pMyVehicle->IsBoat() && m_pMyVehicle->m_vehType != VEHICLE_TYPE_BIKE)
			seatPos.x = -seatPos.x;

	} else if (m_pMyVehicle->pPassengers[0] == this) {
		if (vehModel->m_vehicleType == VEHICLE_TYPE_BOAT)
			seatPos = vehModel->m_positions[BOAT_POS_FRONTSEAT];
		else
			seatPos = vehModel->m_positions[CAR_POS_FRONTSEAT];
	} else if (m_pMyVehicle->pPassengers[1] == this) {
		seatPos = vehModel->m_positions[CAR_POS_BACKSEAT];
		seatPos.x = -seatPos.x;
	} else {
		if (m_pMyVehicle->pPassengers[2] == this) {
			seatPos = vehModel->m_positions[CAR_POS_BACKSEAT];
		} else if (vehModel->m_vehicleType == VEHICLE_TYPE_BOAT) {
			seatPos = vehModel->m_positions[BOAT_POS_FRONTSEAT];
		} else {
			seatPos = vehModel->m_positions[CAR_POS_FRONTSEAT];
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
			m_fRotationCur = HALFPI + m_pMyVehicle->GetForward().Heading();
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
	obj->bIsStatic = false;
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
CPed::WarpPedIntoCar(CVehicle *car)
{
	bInVehicle = true;
	m_pMyVehicle = car;
	m_pMyVehicle->RegisterReference((CEntity **) &m_pMyVehicle);
	m_carInObjective = car;
	m_carInObjective->RegisterReference((CEntity **) &m_carInObjective);
	m_nPedState = PED_DRIVING;
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

	RpAnimBlendClumpSetBlendDeltas(GetClump(), ASSOC_PARTIAL, -1000.0f);

	AddInCarAnims(car, car->pDriver == this);
	RemoveWeaponWhenEnteringVehicle();

	if (car->bIsBus)
		bRenderPedInCar = false;

	bChangedSeat = true;
}

void
CPed::SetObjective(eObjective newObj, float heading, const CVector& pos)
{
	switch (newObj) {
	case OBJECTIVE_USE_SEAT_ATTRACTOR:
	case OBJECTIVE_USE_ATM_ATTRACTOR:
	case OBJECTIVE_USE_STOP_ATTRACTOR:
	case OBJECTIVE_USE_PIZZA_ATTRACTOR:
	case OBJECTIVE_USE_SHELTER_ATTRACTOR:
	case OBJECTIVE_USE_ICECREAM_ATTRACTOR:
		ClearPointGunAt();
		SetObjective(newObj, pos);
		m_attractorHeading = heading;
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
		case OBJECTIVE_WAIT_IN_CAR_THEN_GETOUT:
		case OBJECTIVE_KILL_CHAR_ON_FOOT:
		case OBJECTIVE_KILL_CHAR_ANY_MEANS:
		case OBJECTIVE_FLEE_CHAR_ON_FOOT_TILL_SAFE:
		case OBJECTIVE_FLEE_CHAR_ON_FOOT_ALWAYS:
		case OBJECTIVE_GOTO_CHAR_ON_FOOT:
		case OBJECTIVE_FOLLOW_PED_IN_FORMATION:
		case OBJECTIVE_LEAVE_VEHICLE:
		case OBJECTIVE_ENTER_CAR_AS_PASSENGER:
		case OBJECTIVE_ENTER_CAR_AS_DRIVER:
		case OBJECTIVE_FOLLOW_CAR_IN_CAR:
		case OBJECTIVE_FIRE_AT_OBJ_FROM_VEHICLE:
		case OBJECTIVE_DESTROY_OBJ:
		case OBJECTIVE_DESTROY_CAR:
			break;
		case OBJECTIVE_GOTO_AREA_ANY_MEANS:
		case OBJECTIVE_GOTO_AREA_ON_FOOT:
		case OBJECTIVE_USE_SEAT_ATTRACTOR:
		case OBJECTIVE_USE_ATM_ATTRACTOR:
		case OBJECTIVE_USE_STOP_ATTRACTOR:
		case OBJECTIVE_USE_PIZZA_ATTRACTOR:
		case OBJECTIVE_USE_SHELTER_ATTRACTOR:
		case OBJECTIVE_USE_ICECREAM_ATTRACTOR:
			bIsRunning = false;
			m_pNextPathNode = nil;
			m_nextRoutePointPos = dest;
			m_vecSeekPos = m_nextRoutePointPos;
			m_distanceToCountSeekDone = 0.5f;
			if (m_objective == OBJECTIVE_USE_ATM_ATTRACTOR) {
				m_distanceToCountSeekDone = m_attractor->GetDistanceToCountSeekDone();
				m_acceptableHeadingOffset = m_attractor->GetAcceptableHeading();
			}
			if (m_objective == OBJECTIVE_USE_SEAT_ATTRACTOR) {
				m_distanceToCountSeekDone = m_attractor->GetDistanceToCountSeekDone();
				m_acceptableHeadingOffset = m_attractor->GetAcceptableHeading();
			}
			if (m_objective == OBJECTIVE_USE_STOP_ATTRACTOR) {
				m_distanceToCountSeekDone = m_attractor->GetDistanceToCountSeekDone();
				m_acceptableHeadingOffset = m_attractor->GetAcceptableHeading();
			}
			if (m_objective == OBJECTIVE_USE_PIZZA_ATTRACTOR) {
				m_distanceToCountSeekDone = m_attractor->GetDistanceToCountSeekDone();
				m_acceptableHeadingOffset = m_attractor->GetAcceptableHeading();
			}
			if (m_objective == OBJECTIVE_USE_SHELTER_ATTRACTOR) {
				bIsRunning = true;
				m_distanceToCountSeekDone = m_attractor->GetDistanceToCountSeekDone();
				m_acceptableHeadingOffset = m_attractor->GetAcceptableHeading();
			}
			if (m_objective == OBJECTIVE_USE_ICECREAM_ATTRACTOR) {
				bIsRunning = true;
				m_distanceToCountSeekDone = m_attractor->GetDistanceToCountSeekDone();
				m_acceptableHeadingOffset = m_attractor->GetAcceptableHeading();
			}
			bUsePedNodeSeek = false;
			if (sq(m_distanceToCountSeekDone) > (m_nextRoutePointPos - GetPosition()).MagnitudeSqr2D()) {
				if (!IsUseAttractorObjective(m_objective))
					return;
				if (Abs(m_fRotationCur - m_attractorHeading) < m_acceptableHeadingOffset)
					return;
			}
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
		CAnimBlendAssociation *fightIdleAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_FIGHT_IDLE);
		animAssoc = fightIdleAssoc;
		if (fightIdleAssoc && m_nPedState == PED_FIGHT)
			return;

		if (fightIdleAssoc) {
			CAnimBlendAssociation *idleAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_IDLE_STANCE);
			if (!idleAssoc || idleAssoc->blendDelta <= 0.0f) {
				animAssoc->flags |= ASSOC_DELETEFADEDOUT;
				animAssoc = CAnimManager::BlendAnimation(GetClump(), animGroupToUse, ANIM_IDLE_STANCE, 8.0f);
			}
		}
	}
	if (!animAssoc) {
		animAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_IDLE_TIRED);
		if (animAssoc)
			if (m_nWaitState == WAITSTATE_STUCK || m_nWaitState == WAITSTATE_FINISH_FLEE)
				return;

		if (animAssoc) {
			CAnimBlendAssociation *idleAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_IDLE_STANCE);
			if (!idleAssoc || idleAssoc->blendDelta <= 0.0f) {
				animAssoc->flags |= ASSOC_DELETEFADEDOUT;
				animAssoc = CAnimManager::BlendAnimation(GetClump(), animGroupToUse, ANIM_IDLE_STANCE, 4.0f);
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
				animAssoc = CAnimManager::BlendAnimation(GetClump(), animGroupToUse, ANIM_IDLE_STANCE, 4.0f);
				break;
			case PEDMOVE_WALK:
				animAssoc = CAnimManager::BlendAnimation(GetClump(), animGroupToUse, ANIM_WALK, 1.0f);
				break;
			case PEDMOVE_RUN:
				if (m_nPedState == PED_FLEE_ENTITY) {
					animAssoc = CAnimManager::BlendAnimation(GetClump(), animGroupToUse, ANIM_RUN, 3.0f);
				} else {
					animAssoc = CAnimManager::BlendAnimation(GetClump(), animGroupToUse, ANIM_RUN, 1.0f);
				}
				break;
			case PEDMOVE_SPRINT:
				animAssoc = CAnimManager::BlendAnimation(GetClump(), animGroupToUse, ANIM_SPRINT, 1.0f);
				break;
			default:
				break;
		}

		if (animAssoc) {
			if (m_leader) {
				CAnimBlendAssociation *walkAssoc = RpAnimBlendClumpGetAssociation(m_leader->GetClump(), ANIM_WALK);
				if (!walkAssoc)
					walkAssoc = RpAnimBlendClumpGetAssociation(m_leader->GetClump(), ANIM_RUN);

				if (!walkAssoc)
					walkAssoc = RpAnimBlendClumpGetAssociation(m_leader->GetClump(), ANIM_SPRINT);

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
	m_vehEnterType = doorNode;
	m_nPedState = PED_ENTER_CAR;
	if (m_vehEnterType == CAR_DOOR_RF && m_objective == OBJECTIVE_ENTER_CAR_AS_DRIVER && car->m_vehType != VEHICLE_TYPE_BIKE) {
		car->bIsBeingCarJacked = true;
	}

	m_pMyVehicle = (CVehicle*)m_pSeekTarget;
	m_pMyVehicle->RegisterReference((CEntity**) &m_pMyVehicle);
	((CVehicle*)m_pSeekTarget)->m_nNumGettingIn++;
	bUsesCollision = false;
	CVector doorOpenPos = GetPositionToOpenCarDoor(car, m_vehEnterType);

	// Because buses have stairs
	if (!m_pMyVehicle->bIsBus)
		zDiff = Max(0.0f, doorOpenPos.z - GetPosition().z);

	m_vecOffsetSeek = doorOpenPos - GetPosition();
	m_nPedStateTimer = CTimer::GetTimeInMilliseconds() + 600;
	if (car->IsBoat()) {
		if(car->pHandling->Flags & HANDLING_SIT_IN_BOAT)
			m_pVehicleAnim = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_CAR_SIT, 100.0f);
		else
			m_pVehicleAnim = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_DRIVE_BOAT, 100.0f);

		PedSetInCarCB(nil, this);
		bVehExitWillBeInstant = true;
		if (IsPlayer())
			CWaterLevel::AllocateBoatWakeArray();
	} else {
		if (zDiff > 4.4f) {
			if (m_vehEnterType == CAR_DOOR_RF || m_vehEnterType == CAR_DOOR_RR)
				m_pVehicleAnim = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_CAR_ALIGNHI_RHS, 4.0f);
			else
				m_pVehicleAnim = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_CAR_ALIGNHI_LHS, 4.0f);

		} else {
			if (m_vehEnterType == CAR_DOOR_RF || m_vehEnterType == CAR_DOOR_RR)
				m_pVehicleAnim = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_CAR_ALIGN_RHS, 4.0f);
			else
				m_pVehicleAnim = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_CAR_ALIGN_LHS, 4.0f);
		}
		m_pVehicleAnim->SetFinishCallback(PedAnimAlignCB, this);
		car->AutoPilot.m_nCruiseSpeed = 0;
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

void
CPed::SetCarJack_AllClear(CVehicle *car, uint32 doorNode, uint32 doorFlag)
{
	RemoveWeaponWhenEnteringVehicle();
	if (m_nPedState != PED_SEEK_CAR)
		SetStoredState();

	m_pSeekTarget = car;
	m_pSeekTarget->RegisterReference((CEntity**)&m_pSeekTarget);
	m_nPedState = PED_CARJACK;
	car->bIsBeingCarJacked = true;
	m_pMyVehicle = (CVehicle*)m_pSeekTarget;
	m_pMyVehicle->RegisterReference((CEntity**)&m_pMyVehicle);
	((CVehicle*)m_pSeekTarget)->m_nNumGettingIn++;

	Say(m_nPedType == PEDTYPE_COP ? SOUND_PED_ARREST_COP : SOUND_PED_CAR_JACKING);
	CVector carEnterPos;
	carEnterPos = GetPositionToOpenCarDoor(car, m_vehEnterType);

	car->m_nGettingInFlags |= doorFlag;
	m_vecOffsetSeek = carEnterPos - GetPosition();
	m_nPedStateTimer = CTimer::GetTimeInMilliseconds() + 600;
	float zDiff = Max(0.0f, carEnterPos.z - GetPosition().z);
	bUsesCollision = false;

	if (zDiff > 4.4f) {
		if (m_vehEnterType == CAR_DOOR_RF || m_vehEnterType == CAR_DOOR_RR)
			m_pVehicleAnim = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_CAR_ALIGNHI_RHS, 4.0f);
		else
			m_pVehicleAnim = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_CAR_ALIGNHI_LHS, 4.0f);

	} else {
		if (m_vehEnterType == CAR_DOOR_RF || m_vehEnterType == CAR_DOOR_RR)
			m_pVehicleAnim = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_CAR_ALIGN_RHS, 4.0f);
		else
			m_pVehicleAnim = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_CAR_ALIGN_LHS, 4.0f);
	}

	m_pVehicleAnim->SetFinishCallback(PedAnimAlignCB, this);
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

void
CPed::SetCarJack(CVehicle* car)
{
	uint8 doorFlag;
	eDoors door;
	CPed *pedInSeat = nil;

	if (car->IsBoat())
		return;

	switch (m_vehEnterType) {
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
							SetCarJack_AllClear(car, m_vehEnterType, doorFlag);
}

void
CPed::Solicit(void)
{
	if (m_standardTimer >= CTimer::GetTimeInMilliseconds() && m_carInObjective) {
		CVector doorPos = GetPositionToOpenCarDoor(m_carInObjective, m_vehEnterType, 0.0f);
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
		CAnimBlendAssociation *talkAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_CAR_HOOKERTALK);
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

// Seperate function in VC, more logical. Not sure is it inlined in III.
void
CPed::SetExitBoat(CVehicle *boat)
{
#ifndef VC_PED_PORTS
	m_nPedState = PED_IDLE;
	CVector firstPos = GetPosition();
	CAnimManager::BlendAnimation(GetClump(), m_animGroup, ANIM_IDLE_STANCE, 100.0f);
	if (boat->GetModelIndex() == MI_SPEEDER && boat->IsUpsideDown()) {
		m_pVehicleAnim = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_CAR_CRAWLOUT_RHS, 8.0f);
		m_pVehicleAnim->SetFinishCallback(CPed::PedSetOutCarCB, this);
		m_vehEnterType = CAR_DOOR_RF;
		m_nPedState = PED_EXIT_CAR;
	} else {
		m_vehEnterType = CAR_DOOR_RF;
		CPed::PedSetOutCarCB(nil, this);
		bIsStanding = true;
		m_pCurSurface = boat;
		m_pCurSurface->RegisterReference((CEntity**)&m_pCurSurface);
	}
	SetPosition(firstPos);
	SetMoveState(PEDMOVE_STILL);
	m_vecMoveSpeed = boat->m_vecMoveSpeed;
	bTryingToReachDryLand = true;
#else
	m_nPedState = PED_IDLE;
	CVector newPos = GetPosition();
	RemoveInCarAnims();
	CColModel* boatCol = boat->GetColModel();
	if (boat->IsUpsideDown()) {
		newPos = { 0.0f, 0.0f, boatCol->boundingBox.min.z };
		newPos = boat->GetMatrix() * newPos;
		newPos.z += 1.0f;
		m_vehEnterType = CAR_DOOR_RF;
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
			m_vehEnterType = CAR_DOOR_RF;
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
			m_vehEnterType = CAR_DOOR_RF;
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

void
CPed::SetNewAttraction(CPedAttractor* pAttractor, const CVector& pos, float heading, float time, int32 qid)
{
	if (!m_attractor)
		m_attractor = pAttractor;
	if (m_attractor != pAttractor)
		return;
	switch (pAttractor->GetEffect()->pedattr.type) {
	case ATTRACTOR_ATM: SetObjective(OBJECTIVE_USE_ATM_ATTRACTOR, heading, pos); break;
	case ATTRACTOR_SEAT: SetObjective(OBJECTIVE_USE_SEAT_ATTRACTOR, heading, pos); break;
	case ATTRACTOR_STOP: SetObjective(OBJECTIVE_USE_STOP_ATTRACTOR, heading, pos); break;
	case ATTRACTOR_PIZZA: SetObjective(OBJECTIVE_USE_PIZZA_ATTRACTOR, heading, pos); break;
	case ATTRACTOR_SHELTER: SetObjective(OBJECTIVE_USE_SHELTER_ATTRACTOR, heading, pos); break;
	case ATTRACTOR_ICECREAM: SetObjective(OBJECTIVE_USE_ICECREAM_ATTRACTOR, heading, pos); break;
	default: return;
	}
	SetObjectiveTimer(time);
	m_positionInQueue = qid;
}

void
CPed::ClearWaitState(void)
{
	switch (m_nWaitState) {
	case WAITSTATE_PLAYANIM_CHAT:
	case WAITSTATE_SIT_DOWN:
	case WAITSTATE_SIT_DOWN_RVRS:
	case WAITSTATE_SIT_UP:
	case WAITSTATE_SIT_IDLE:
	case WAITSTATE_USE_ATM:
		if (CTimer::GetTimeInMilliseconds() <= m_nWaitTimer) {
			AnimationId id;
			switch (m_nWaitState) { // TODO(MIAMI): actual!
			case WAITSTATE_PLAYANIM_CHAT: id = ANIM_IDLE_CHAT; break;
			case WAITSTATE_SIT_DOWN: id = ANIM_SEAT_DOWN; break;
			case WAITSTATE_SIT_DOWN_RVRS: id = ANIM_SEAT_DOWN2; break;
			case WAITSTATE_SIT_UP: id = ANIM_SEAT_UP; break;
			case WAITSTATE_SIT_IDLE: id = ANIM_SEAT_IDLE; break;
			case WAITSTATE_USE_ATM: id = ANIM_ATM; break;
			}
			CAnimBlendAssociation* pAssoc = RpAnimBlendClumpGetAssociation(GetClump(), id);
			if (pAssoc)
				pAssoc->blendDelta = -8.0f;
			if (m_attractor)
				GetPedAttractorManager()->DeRegisterPed(this, m_attractor);
		}
		break;
	case WAITSTATE_RIOT:
	case WAITSTATE_FAST_FALL:
	case WAITSTATE_BOMBER:
	case WAITSTATE_STRIPPER:
	case WAITSTATE_GROUND_ATTACK:
	case WAITSTATE_LANCESITTING:
	case WAITSTATE_PLAYANIM_HANDSUP_SIMPLE:
		assert(0);
	}
	m_nWaitState = WAITSTATE_FALSE;
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

	CWeapon bufWeapon;
	for (int i = 0; i < 13; i++) { // has to be hardcoded
		bufWeapon.Load(buf);
		if (i >= 10)
			continue; // tmp hack before we fix save/load

		if (bufWeapon.m_eWeaponType != WEAPONTYPE_UNARMED) {
			int modelId = CWeaponInfo::GetWeaponInfo(bufWeapon.m_eWeaponType)->m_nModelId;
			if (modelId != -1) {
				CStreaming::RequestModel(modelId, STREAMFLAGS_DEPENDENCY);
				int modelId2 = CWeaponInfo::GetWeaponInfo(bufWeapon.m_eWeaponType)->m_nModel2Id;
				if (modelId2 != -1)
					CStreaming::RequestModel(modelId2, STREAMFLAGS_DEPENDENCY);

				CStreaming::LoadAllRequestedModels(false);
			}
			GiveWeapon(bufWeapon.m_eWeaponType, bufWeapon.m_nAmmoTotal);
		}
	}
	SkipSaveBuf(buf, 5);
	CopyFromBuf(buf, m_maxWeaponTypeAllowed);
	SkipSaveBuf(buf, 162);
}
#undef CopyFromBuf
#undef CopyToBuf
#endif

// --MIAMI: Done
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

// --MIAMI: Done
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

// --MIAMI: Done
void
CPed::ClearFollowPath()
{
	for (int i = 0; i < ARRAY_SIZE(m_pPathNodesStates); i++) {
		m_pPathNodesStates[i] = nil;
	}
	m_nPathNodes = 0;
	m_nCurPathNode = 0;
}

// --MIAMI: Done except bikes
void
CPed::AddInCarAnims(CVehicle* car, bool isDriver)
{
	AnimationId anim;
	AssocGroupId group;
	if (car->IsBoat()) {
		if (car->pHandling->Flags & HANDLING_SIT_IN_BOAT) {
			anim = ANIM_CAR_SIT;
		} else {
			anim = ANIM_DRIVE_BOAT;
		}
		group = ASSOCGRP_STD;
	} else if (car->IsBike()) {
		if (isDriver) {
			// TODO(Miami): Bikes
		} else {
			// TODO(Miami): Bikes
		}
	} else {
		if (isDriver) {
			if (car->bLowVehicle) {
				anim = ANIM_CAR_LSIT;
			} else {
				anim = ANIM_CAR_SIT;
			}
		} else {
			if (car->bLowVehicle) {
				anim = ANIM_CAR_SITPLO;
			} else {
				anim = ANIM_CAR_SITP;
			}
		}
		group = ASSOCGRP_STD;
	}
	m_pVehicleAnim = CAnimManager::BlendAnimation(GetClump(), group, anim, 100.0f);

	StopNonPartialAnims();
}

bool
CPed::CanBeDamagedByThisGangMember(CPed* who)
{
	return m_gangFlags & (1 << (uint8)(who->m_nPedType - PEDTYPE_GANG1));
}

bool
IsPedPointerValid_NotInWorld(CPed* pPed)
{
	if (!pPed)
		return false;
	int index = CPools::GetPedPool()->GetJustIndex(pPed);
#ifdef FIX_BUGS
	if (index < 0 || index >= NUMPEDS)
#else
	if (index < 0 || index > NUMPEDS)
#endif
		return false;
	return true;
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