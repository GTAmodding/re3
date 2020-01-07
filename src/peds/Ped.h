#pragma once

#include "Physical.h"
#include "Weapon.h"
#include "PedStats.h"
#include "PedType.h"
#include "PedIK.h"
#include "AnimManager.h"
#include "AnimBlendClumpData.h"
#include "AnimBlendAssociation.h"
#include "WeaponInfo.h"
#include "Fire.h"
#include "DMAudio.h"
#include "EventList.h"

struct CPathNode;
class CAccident;
class CObject;

struct CPedAudioData
{
	int m_nFixedDelayTime;
	int m_nOverrideFixedDelayTime;
	int m_nOverrideMaxRandomDelayTime;
	int m_nMaxRandomDelayTime;
};

enum eFormation
{
	FORMATION_UNDEFINED,
	FORMATION_REAR,
	FORMATION_REAR_LEFT,
	FORMATION_REAR_RIGHT,
	FORMATION_FRONT_LEFT,
	FORMATION_FRONT_RIGHT,
	FORMATION_LEFT,
	FORMATION_RIGHT,
	FORMATION_FRONT
};

enum FightState : int8 {
	FIGHTSTATE_MOVE_FINISHED = -2,
	FIGHTSTATE_JUST_ATTACKED,
	FIGHTSTATE_NO_MOVE,
	FIGHTSTATE_1
};

enum
{
	ENDFIGHT_NORMAL,
	ENDFIGHT_WITH_A_STEP,
	ENDFIGHT_FAST
};

enum PedRouteType
{
	PEDROUTE_STOP_WHEN_DONE = 1,
	PEDROUTE_GO_BACKWARD_WHEN_DONE,
	PEDROUTE_GO_TO_START_WHEN_DONE
};

enum FightMoveHitLevel
{
	HITLEVEL_NULL,
	HITLEVEL_GROUND,
	HITLEVEL_LOW,
	HITLEVEL_MEDIUM,
	HITLEVEL_HIGH
};

struct FightMove
{
	AnimationId animId;
	float startFireTime;
	float endFireTime;
	float comboFollowOnTime;
	float strikeRadius;
	uint8 hitLevel; // FightMoveHitLevel
	uint8 damage;
	uint8 flags;
};
static_assert(sizeof(FightMove) == 0x18, "FightMove: error");

// TO-DO: This is eFightState on mobile.
enum PedFightMoves
{
	FIGHTMOVE_NULL,
	// Attacker
	FIGHTMOVE_STDPUNCH,
	FIGHTMOVE_IDLE,
	FIGHTMOVE_SHUFFLE_F,
	FIGHTMOVE_KNEE,
	FIGHTMOVE_HEADBUTT,
	FIGHTMOVE_PUNCHJAB,
	FIGHTMOVE_PUNCHHOOK,
	FIGHTMOVE_KICK,
	FIGHTMOVE_LONGKICK,
	FIGHTMOVE_ROUNDHOUSE,
	FIGHTMOVE_BODYBLOW,
	FIGHTMOVE_GROUNDKICK,
	// Opponent
	FIGHTMOVE_HITFRONT,
	FIGHTMOVE_HITBACK,
	FIGHTMOVE_HITRIGHT,
	FIGHTMOVE_HITLEFT,
	FIGHTMOVE_HITBODY,
	FIGHTMOVE_HITCHEST,
	FIGHTMOVE_HITHEAD,
	FIGHTMOVE_HITBIGSTEP,
	FIGHTMOVE_HITONFLOOR,
	FIGHTMOVE_HITBEHIND,
	FIGHTMOVE_IDLE2NORM,
	NUM_FIGHTMOVES
};

enum ePedPieceTypes
{
	PEDPIECE_TORSO,
	PEDPIECE_MID,
	PEDPIECE_LEFTARM,
	PEDPIECE_RIGHTARM,
	PEDPIECE_LEFTLEG,
	PEDPIECE_RIGHTLEG,
	PEDPIECE_HEAD,
};

enum eWaitState {
	WAITSTATE_FALSE,
	WAITSTATE_TRAFFIC_LIGHTS,
	WAITSTATE_CROSS_ROAD,
	WAITSTATE_CROSS_ROAD_LOOK,
	WAITSTATE_LOOK_PED,
	WAITSTATE_LOOK_SHOP,
	WAITSTATE_LOOK_ACCIDENT,
	WAITSTATE_FACEOFF_GANG,
	WAITSTATE_DOUBLEBACK,
	WAITSTATE_HITWALL,
	WAITSTATE_TURN180,
	WAITSTATE_SURPRISE,
	WAITSTATE_STUCK,
	WAITSTATE_LOOK_ABOUT,
	WAITSTATE_PLAYANIM_DUCK,
	WAITSTATE_PLAYANIM_COWER,
	WAITSTATE_PLAYANIM_TAXI,
	WAITSTATE_PLAYANIM_HANDSUP,
	WAITSTATE_PLAYANIM_HANDSCOWER,
	WAITSTATE_PLAYANIM_CHAT,
	WAITSTATE_FINISH_FLEE
};

enum eObjective : uint32 {
	OBJECTIVE_NONE,
	OBJECTIVE_IDLE,
	OBJECTIVE_FLEE_TILL_SAFE,
	OBJECTIVE_GUARD_SPOT,
	OBJECTIVE_GUARD_AREA,	// not implemented
	OBJECTIVE_WAIT_IN_CAR,
	OBJECTIVE_WAIT_IN_CAR_THEN_GETOUT,
	OBJECTIVE_KILL_CHAR_ON_FOOT,
	OBJECTIVE_KILL_CHAR_ANY_MEANS,
	OBJECTIVE_FLEE_CHAR_ON_FOOT_TILL_SAFE,
	OBJECTIVE_FLEE_CHAR_ON_FOOT_ALWAYS,
	OBJECTIVE_GOTO_CHAR_ON_FOOT,
	OBJECTIVE_FOLLOW_PED_IN_FORMATION,
	OBJECTIVE_LEAVE_VEHICLE,
	OBJECTIVE_ENTER_CAR_AS_PASSENGER,
	OBJECTIVE_ENTER_CAR_AS_DRIVER,
	OBJECTIVE_FOLLOW_CAR_IN_CAR, // seems not implemented so far
	OBJECTIVE_FIRE_AT_OBJ_FROM_VEHICLE,	// not implemented
	OBJECTIVE_DESTROY_OBJ,	// not implemented
	OBJECTIVE_DESTROY_CAR,
	OBJECTIVE_GOTO_AREA_ANY_MEANS,
	OBJECTIVE_GOTO_AREA_ON_FOOT,
	OBJECTIVE_RUN_TO_AREA,
	OBJECTIVE_23,	// not implemented
	OBJECTIVE_24,	// not implemented
	OBJECTIVE_FIGHT_CHAR,
	OBJECTIVE_SET_LEADER,
	OBJECTIVE_FOLLOW_ROUTE,
	OBJECTIVE_SOLICIT,
	OBJECTIVE_HAIL_TAXI,
	OBJECTIVE_CATCH_TRAIN,
	OBJECTIVE_BUY_ICE_CREAM,
	OBJECTIVE_STEAL_ANY_CAR,
	OBJECTIVE_MUG_CHAR,
	OBJECTIVE_FLEE_CAR,
#ifdef VC_PED_PORTS
	OBJECTIVE_LEAVE_CAR_AND_DIE
#endif
};

enum {
	RANDOM_CHAR = 1,
	MISSION_CHAR,
};

enum PedLineUpPhase {
	LINE_UP_TO_CAR_START,
	LINE_UP_TO_CAR_END,
	LINE_UP_TO_CAR_2 // Buggy. Used for cops arresting you from passenger door
};

enum PedOnGroundState {
	NO_PED,
	PED_IN_FRONT_OF_ATTACKER,
	PED_ON_THE_FLOOR,
	PED_DEAD_ON_THE_FLOOR
};

enum PedState
{
	PED_NONE,
	PED_IDLE,
	PED_LOOK_ENTITY,
	PED_LOOK_HEADING,
	PED_WANDER_RANGE,
	PED_WANDER_PATH,
	PED_SEEK_POS,
	PED_SEEK_ENTITY,
	PED_FLEE_POS,
	PED_FLEE_ENTITY,
	PED_PURSUE,
	PED_FOLLOW_PATH,
	PED_SNIPER_MODE,
	PED_ROCKET_ODE,
	PED_DUMMY,
	PED_PAUSE,
	PED_ATTACK,
	PED_FIGHT,
	PED_FACE_PHONE,
	PED_MAKE_CALL,
	PED_CHAT,
	PED_MUG,
	PED_AIM_GUN,
	PED_AI_CONTROL,
	PED_SEEK_CAR,
	PED_SEEK_IN_BOAT,
	PED_FOLLOW_ROUTE,
	PED_CPR,
	PED_SOLICIT,
	PED_BUY_ICECREAM,
	PED_INVESTIGATE,
	PED_STEP_AWAY,
	PED_ON_FIRE,

	PED_UNKNOWN,	// Same with IDLE, but also infects up to 5 peds with same pedType and WANDER_PATH, so they become stone too. HANG_OUT in Fire_Head's idb

	PED_STATES_NO_AI,

	// One of these states isn't on PS2 - start
	PED_JUMP,
	PED_FALL,
	PED_GETUP,
	PED_STAGGER,
	PED_DIVE_AWAY,

	PED_STATES_NO_ST,
	PED_ENTER_TRAIN,
	PED_EXIT_TRAIN,
	PED_ARREST_PLAYER,
	// One of these states isn't on PS2 - end

	PED_DRIVING,
	PED_PASSENGER,
	PED_TAXI_PASSENGER,
	PED_OPEN_DOOR,
	PED_DIE,
	PED_DEAD,
	PED_CARJACK,
	PED_DRAG_FROM_CAR,
	PED_ENTER_CAR,
	PED_STEAL_CAR,
	PED_EXIT_CAR,
	PED_HANDS_UP,
	PED_ARRESTED,
};

enum eMoveState {
	PEDMOVE_NONE,
	PEDMOVE_STILL,
	PEDMOVE_WALK,
	PEDMOVE_RUN,
	PEDMOVE_SPRINT,
};

class CVehicle;

class CPed : public CPhysical
{
public:
	// 0x128
	CStoredCollPoly m_collPoly;
	float m_fCollisionSpeed;

	// cf. https://github.com/DK22Pac/plugin-sdk/blob/master/plugin_sa/game_sa/CPed.h from R*
	uint8 bIsStanding : 1;
	uint8 m_ped_flagA2 : 1;	// bWasStanding?
	uint8 bIsAttacking : 1;		// doesn't reset after fist fight
	uint8 bIsPointingGunAt : 1;
	uint8 bIsLooking : 1;
	uint8 bKeepTryingToLook : 1; // if we can't look somewhere due to unreachable angles
	uint8 bIsRestoringLook : 1;
	uint8 bIsAimingGun : 1;

	uint8 bIsRestoringGun : 1;
	uint8 bCanPointGunAtTarget : 1;
	uint8 bIsTalking : 1;
	uint8 bIsInTheAir : 1;
	uint8 bIsLanding : 1;
	uint8 bIsRunning : 1; // on some conditions
	uint8 bHitSomethingLastFrame : 1;
	uint8 bVehEnterDoorIsBlocked : 1; // because someone else enters/exits from there

	uint8 bCanPedEnterSeekedCar : 1;
	uint8 bRespondsToThreats : 1;
	uint8 bRenderPedInCar : 1;
	uint8 bChangedSeat : 1;
	uint8 bUpdateAnimHeading : 1;
	uint8 bBodyPartJustCameOff : 1;
	uint8 bIsShooting : 1;
	uint8 bFindNewNodeAfterStateRestore : 1;

	uint8 bHasACamera : 1; // does ped possess a camera to document accidents involves fire/explosion
	uint8 m_ped_flagD2 : 1; // set when ped witnessed an event
	uint8 bPedIsBleeding : 1;
	uint8 bStopAndShoot : 1; // Ped cannot reach target to attack with fist, need to use gun
	uint8 bIsPedDieAnimPlaying : 1;
	uint8 bUsePedNodeSeek : 1;
	uint8 bObjectiveCompleted : 1;
	uint8 bScriptObjectiveCompleted : 1;

	uint8 bKindaStayInSamePlace : 1;
	uint8 m_ped_flagE2 : 1; // bBeingChasedByPolice?
	uint8 bNotAllowedToDuck : 1;
	uint8 bCrouchWhenShooting : 1;
	uint8 bIsDucking : 1;
	uint8 bGetUpAnimStarted : 1;
	uint8 bDoBloodyFootprints : 1;
	uint8 bFleeAfterExitingCar : 1;

	uint8 bWanderPathAfterExitingCar : 1;
	uint8 bIsLeader : 1;
	uint8 bDontDragMeOutCar : 1; // unfinished feature
	uint8 m_ped_flagF8 : 1;
	uint8 bWillBeQuickJacked : 1;
	uint8 bCancelEnteringCar : 1; // after door is opened or couldn't be opened due to it's locked
	uint8 bObstacleShowedUpDuringKillObjective : 1;
	uint8 bDuckAndCover : 1;

	uint8 bStillOnValidPoly : 1; // set if the polygon the ped is on is still valid for collision
	uint8 bAllowMedicsToReviveMe : 1;
	uint8 bResetWalkAnims : 1;
	uint8 bStartWanderPathOnFoot : 1; // exits the car if he's in it, reset after path found
	uint8 bOnBoat : 1; // not just driver, may be just standing
	uint8 bBusJacked : 1;
	uint8 bGonnaKillTheCarJacker : 1; // only set when car is jacked from right door
	uint8 bFadeOut : 1;

	uint8 m_ped_flagH1 : 1;
	uint8 bHitSteepSlope : 1; // has ped collided/is standing on a steep slope (surface type)
	uint8 bCullExtraFarAway : 1; // special ped only gets culled if it's extra far away (for roadblocks)
	uint8 bClearObjective : 1;
	uint8 bTryingToReachDryLand : 1; // has ped just exited boat and trying to get to dry land
	uint8 bCollidedWithMyVehicle : 1;
	uint8 bRichFromMugging : 1; // ped has lots of cash cause they've been mugging people
	uint8 bChrisCriminal : 1; // Is a criminal as killed during Chris' police mission (should be counted as such)

	uint8 bShakeFist : 1;  // test shake hand at look entity
	uint8 bNoCriticalHits : 1; // if set, limbs won't came off
	uint8 m_ped_flagI4 : 1; // we've been put to car by script or without align phase? - related with cars
	uint8 bHasAlreadyBeenRecorded : 1;
	uint8 bFallenDown : 1;
#ifdef VC_PED_PORTS
	uint8 bKnockedUpIntoAir : 1; // has ped been knocked up into the air by a car collision
#else
	uint8 m_ped_flagI20 : 1;
#endif
	uint8 m_ped_flagI40 : 1; // bMakePedsRunToPhonesToReportCrimes makes use of this as runover by car indicator
	uint8 m_ped_flagI80 : 1; // KANGAROO_CHEAT define makes use of this as cheat toggle 

	uint8 stuff10[3];
	uint8 CharCreatedBy;
	uint8 field_161;
	uint8 pad_162[2];
	eObjective m_objective;
	eObjective m_prevObjective;
	CPed *m_pedInObjective;
	CVehicle *m_carInObjective;
	CVector m_nextRoutePointPos;
	CPed *m_leader;
	eFormation m_pedFormation;
	uint32 m_fearFlags;
	CEntity *m_threatEntity;
	CVector2D m_eventOrThreat;
	uint32 m_eventType;
	CEntity* m_pEventEntity;
	float m_fAngleToEvent;
	AnimBlendFrameData *m_pFrames[PED_NODE_MAX];
	AssocGroupId m_animGroup;
	CAnimBlendAssociation *m_pVehicleAnim;
	CVector2D m_vecAnimMoveDelta;
	CVector m_vecOffsetSeek;
	CPedIK m_pedIK; 
	float m_actionX;
	float m_actionY;
	uint32 m_nPedStateTimer;
	PedState m_nPedState;
	PedState m_nLastPedState;
	eMoveState m_nMoveState;
	int32 m_nStoredMoveState;
	int32 m_nPrevMoveState;
	eWaitState m_nWaitState;
	uint32 m_nWaitTimer;
	void *m_pPathNodesStates[8]; // unused, probably leftover from VC
	CVector2D m_stPathNodeStates[10];
	uint16 m_nPathNodes;
	int16 m_nCurPathNode;
	int8 m_nPathDir;
private:
	int8 _pad2B5[3];
public:
	CPathNode *m_pLastPathNode;
	CPathNode *m_pNextPathNode;
	float m_fHealth;
	float m_fArmour;
	int16 m_routeLastPoint;
	uint16 m_routeStartPoint;
	int16 m_routePointsPassed;
	int16 m_routeType;	// See PedRouteType
	int16 m_routePointsBeingPassed;
	uint16 field_2D2;
	CVector2D m_moved;
	float m_fRotationCur;
	float m_fRotationDest;
	float m_headingRate;
	uint16 m_vehEnterType;	// TODO: this is more like a door, not a type
	int16 m_walkAroundType;
	CPhysical *m_pCurrentPhysSurface;
	CVector m_vecOffsetFromPhysSurface;
	CEntity *m_pCurSurface;
	CVector m_vecSeekPos;
	CEntity *m_pSeekTarget;
	CVehicle *m_pMyVehicle;
	bool bInVehicle;
	uint8 pad_315[3];
	float m_distanceToCountSeekDone;
	bool bRunningToPhone;
	uint8 field_31D;
	int16 m_phoneId;
	eCrimeType m_crimeToReportOnPhone;
	uint32 m_phoneTalkTimer;
	CAccident *m_lastAccident;
	int32 m_nPedType;
	CPedStats *m_pedStats;
	float m_fleeFromPosX;
	float m_fleeFromPosY;
	CEntity *m_fleeFrom;
	uint32 m_fleeTimer;
	CEntity* m_collidingEntityWhileFleeing;
	uint32 m_collidingThingTimer;
	CEntity *m_pCollidingEntity;
	uint8 m_stateUnused;
	uint8 pad_351[3];
	uint32 m_timerUnused;
	CVector2D *m_wanderRangeBounds;	// array with 2 CVector2D (actually unused CRange2D class) - unused
	CWeapon m_weapons[WEAPONTYPE_TOTAL_INVENTORY_WEAPONS];
	eWeaponType m_storedWeapon;
	uint8 m_currentWeapon;			// eWeaponType
	uint8 m_maxWeaponTypeAllowed;	// eWeaponType
	uint8 m_wepSkills;
	uint8 m_wepAccuracy;
	CEntity *m_pPointGunAt;
	CVector m_vecHitLastPos;
	PedFightMoves m_lastFightMove;
	uint8 m_fightButtonPressure;
	FightState m_fightState;
	bool m_takeAStepAfterAttack;
	uint8 pad_4B3;
	CFire *m_pFire;
	CEntity *m_pLookTarget;
	float m_fLookDirection;
	int32 m_wepModelID;
	uint32 m_leaveCarTimer;
	uint32 m_getUpTimer;
	uint32 m_lookTimer;
	uint32 m_standardTimer;
	uint32 m_attackTimer;
	uint32 m_shootTimer; // shooting is a part of attack
	uint32 m_hitRecoverTimer;
	uint32 m_objectiveTimer;
	uint32 m_duckTimer;
	uint32 m_duckAndCoverTimer;
	int32 m_bloodyFootprintCount;
	uint8 m_panicCounter;
	bool m_deadBleeding;
	int8 m_bodyPartBleeding;		// PedNode, but -1 if there isn't
	uint8 m_field_4F3;
	CPed *m_nearPeds[10];
	uint16 m_numNearPeds;
	int8 m_lastWepDam;
	uint8 pad_51F;
	uint32 m_lastSoundStart;
	uint32 m_soundStart;
	uint16 m_lastQueuedSound;
	uint16 m_queuedSound;
	CVector m_vecSeekPosEx; // used for OBJECTIVE_GUARD_SPOT
	float m_distanceToCountSeekDoneEx; // used for OBJECTIVE_GUARD_SPOT

	static void *operator new(size_t);
	static void *operator new(size_t, int);
	static void operator delete(void*, size_t);
	static void operator delete(void*, int);

	CPed(uint32 pedType);
	~CPed(void);

	void SetModelIndex(uint32 mi);
	void ProcessControl(void);
	void Teleport(CVector);
	void PreRender(void);
	void Render(void);
	bool SetupLighting(void);
	void RemoveLighting(bool);
	void FlagToDestroyWhenNextProcessed(void);
	int32 ProcessEntityCollision(CEntity*, CColPoint*);

	virtual void SetMoveAnim(void);

	void AddWeaponModel(int id);
	void AimGun(void);
	void KillPedWithCar(CVehicle *veh, float impulse);
	void Say(uint16 audio);
	void SetLookFlag(CEntity *target, bool unknown);
	void SetLookFlag(float direction, bool unknown);
	void SetLookTimer(int time);
	void SetDie(AnimationId anim, float arg1, float arg2);
	void SetDead(void);
	void ApplyHeadShot(eWeaponType weaponType, CVector pos, bool evenOnPlayer);
	void RemoveBodyPart(PedNode nodeId, int8 direction);
	bool OurPedCanSeeThisOne(CEntity *target);
	void Avoid(void);
	void Attack(void);
	void ClearAimFlag(void);
	void ClearLookFlag(void);
	void RestorePreviousState(void);
	void ClearAttack(void);
	bool IsPedHeadAbovePos(float zOffset);
	void RemoveWeaponModel(int modelId);
	void SetCurrentWeapon(uint32 weaponType);
	void Duck(void);
	void ClearDuck(void);
	void ClearPointGunAt(void);
	void BeingDraggedFromCar(void);
	void RestartNonPartialAnims(void);
	void LineUpPedWithCar(PedLineUpPhase phase);
	void SetPedPositionInCar(void);
	void PlayFootSteps(void);
	void QuitEnteringCar(void);
	void BuildPedLists(void);
	uint32 GiveWeapon(eWeaponType weaponType, uint32 ammo);
	void CalculateNewOrientation(void);
	float WorkOutHeadingForMovingFirstPerson(float);
	void CalculateNewVelocity(void);
	bool CanSeeEntity(CEntity*, float);
	void RestorePreviousObjective(void);
	void SetIdle(void);
	void SetObjective(eObjective, void*);
	void SetObjective(eObjective);
	void SetObjective(eObjective, int16, int16);
	void SetObjective(eObjective, CVector);
	void SetObjective(eObjective, CVector, float);
	void ClearChat(void);
	void InformMyGangOfAttack(CEntity*);
	void ReactToAttack(CEntity*);
	void SetDuck(uint32);
	void RegisterThreatWithGangPeds(CEntity*);
	bool TurnBody(void);
	void Chat(void);
	void CheckAroundForPossibleCollisions(void);
	void SetSeek(CVector, float);
	void SetSeek(CEntity*, float);
	bool MakePhonecall(void);
	bool FacePhone(void);
	CPed *CheckForDeadPeds(void);
	bool CheckForExplosions(CVector2D &area);
	CPed *CheckForGunShots(void);
	uint8 CheckForPointBlankPeds(CPed*);
	bool CheckIfInTheAir(void);
	void ClearAll(void);
	void SetPointGunAt(CEntity*);
	bool Seek(void);
	bool SetWanderPath(int8);
	bool SetFollowPath(CVector);
	void ClearAttackByRemovingAnim(void);
	void SetStoredState(void);
	void StopNonPartialAnims(void);
	bool InflictDamage(CEntity*, eWeaponType, float, ePedPieceTypes, uint8);
	void ClearFlee(void);
	void ClearFall(void);
	void SetGetUp(void);
	void ClearInvestigateEvent(void);
	void ClearLeader(void);
	void ClearLook(void);
	void ClearObjective(void);
	void ClearPause(void);
	void ClearSeek(void);
	void ClearWeapons(void);
	void RestoreGunPosition(void);
	void RestoreHeadingRate(void);
	void SetAimFlag(CEntity* to);
	void SetAimFlag(float angle);
	void SetAmmo(eWeaponType weaponType, uint32 ammo);
	void SetEvasiveStep(CEntity*, uint8);
	void GrantAmmo(eWeaponType, uint32);
	void SetEvasiveDive(CPhysical*, uint8);
	void SetAttack(CEntity*);
	void StartFightAttack(uint8);
	void SetWaitState(eWaitState, void*);
	bool FightStrike(CVector&);
	int GetLocalDirection(const CVector2D &);
	void StartFightDefend(uint8, uint8, uint8);
	void PlayHitSound(CPed*);
	void SetFall(int, AnimationId, uint8);
	void SetFlee(CEntity*, int);
	void SetFlee(CVector2D const &, int);
	void RemoveInCarAnims(void);
	void CollideWithPed(CPed*);
	void SetDirectionToWalkAroundObject(CEntity*);
	void CreateDeadPedMoney(void);
	void CreateDeadPedWeaponPickups(void);
	void SetAttackTimer(uint32);
	void SetBeingDraggedFromCar(CVehicle*, uint32, bool);
	void SetRadioStation(void);
	void SetBuyIceCream(void);
	void SetChat(CEntity*, uint32);
	void DeadPedMakesTyresBloody(void);
	void MakeTyresMuddySectorList(CPtrList&);
	uint8 DoesLOSBulletHitPed(CColPoint &point);
	bool DuckAndCover(void);
	void EndFight(uint8);
	void EnterCar(void);
	uint8 GetNearestTrainPedPosition(CVehicle*, CVector&);
	uint8 GetNearestTrainDoor(CVehicle*, CVector&);
	void LineUpPedWithTrain(void);
	void ExitCar(void);
	void Fight(void);
	bool FindBestCoordsFromNodes(CVector, CVector*);
	void Wait(void);
	void ProcessObjective(void);
	bool SeekFollowingPath(CVector*);
	void Flee(void);
	void FollowPath(void);
	CVector GetFormationPosition(void);
	void GetNearestDoor(CVehicle*, CVector&);
	bool GetNearestPassengerDoor(CVehicle*, CVector&);
	int GetNextPointOnRoute(void);
	uint8 GetPedRadioCategory(uint32);
	int GetWeaponSlot(eWeaponType);
	void GoToNearestDoor(CVehicle*);
	bool HaveReachedNextPointOnRoute(float);
	void Idle(void);
	void InTheAir(void);
	void SetLanding(void);
	void InvestigateEvent(void);
	bool IsPedDoingDriveByShooting(void);
	bool IsRoomToBeCarJacked(void);
	void SetInvestigateEvent(eEventType, CVector2D, float, uint16, float);
	bool LookForInterestingNodes(void);
	void LookForSexyCars(void);
	void LookForSexyPeds(void);
	void Mug(void);
	void MoveHeadToLook(void);
	void Pause(void);
	void ProcessBuoyancy(void);
	void ServiceTalking(void);
	void SetJump(void);
	void WanderPath(void);
	void ReactToPointGun(CEntity*);
	void SeekCar(void);
	bool PositionPedOutOfCollision(void);
	bool RunToReportCrime(eCrimeType);
	bool PlacePedOnDryLand(void);
	bool PossiblyFindBetterPosToSeekCar(CVector*, CVehicle*);
	void UpdateFromLeader(void);
	uint32 ScanForThreats(void);
	void SetEnterCar(CVehicle*, uint32);
	bool WarpPedToNearEntityOffScreen(CEntity*);
	void SetExitCar(CVehicle*, uint32);
	void SetFormation(eFormation);
	bool WillChat(CPed*);
	void SetEnterTrain(CVehicle*, uint32);
	void SetEnterCar_AllClear(CVehicle*, uint32, uint32);
	void SetSolicit(uint32 time);
	void ScanForInterestingStuff(void);
	void WarpPedIntoCar(CVehicle*);
	void SetCarJack(CVehicle*);
	bool WarpPedToNearLeaderOffScreen(void);
	void Solicit(void);
	void SetExitBoat(CVehicle*);

	// Static methods
	static CVector GetLocalPositionToOpenCarDoor(CVehicle *veh, uint32 component, float offset);
	static CVector GetPositionToOpenCarDoor(CVehicle *veh, uint32 component, float seatPosMult);
	static CVector GetPositionToOpenCarDoor(CVehicle* veh, uint32 component);
	static void Initialise(void);
	static void SetAnimOffsetForEnterOrExitVehicle(void);
	static void LoadFightData(void);

	// Callbacks
	static void PedGetupCB(CAnimBlendAssociation *assoc, void *arg);
	static void PedStaggerCB(CAnimBlendAssociation *assoc, void *arg);
	static void PedEvadeCB(CAnimBlendAssociation *assoc, void *arg);
	static void FinishDieAnimCB(CAnimBlendAssociation *assoc, void *arg);
	static void FinishedWaitCB(CAnimBlendAssociation *assoc, void *arg);
	static void FinishLaunchCB(CAnimBlendAssociation *assoc, void *arg);
	static void FinishHitHeadCB(CAnimBlendAssociation *assoc, void *arg);
	static void PedAnimGetInCB(CAnimBlendAssociation *assoc, void *arg);
	static void PedAnimDoorOpenCB(CAnimBlendAssociation *assoc, void *arg);
	static void PedAnimPullPedOutCB(CAnimBlendAssociation *assoc, void *arg);
	static void PedAnimDoorCloseCB(CAnimBlendAssociation *assoc, void *arg);
	static void PedSetInCarCB(CAnimBlendAssociation *assoc, void *arg);
	static void PedSetOutCarCB(CAnimBlendAssociation *assoc, void *arg);
	static void PedAnimAlignCB(CAnimBlendAssociation *assoc, void *arg);
	static void PedSetDraggedOutCarCB(CAnimBlendAssociation *assoc, void *arg);
	static void PedAnimStepOutCarCB(CAnimBlendAssociation *assoc, void *arg);
	static void PedSetInTrainCB(CAnimBlendAssociation *assoc, void *arg);
	static void PedSetOutTrainCB(CAnimBlendAssociation *assoc, void *arg);
	static void FinishedAttackCB(CAnimBlendAssociation *assoc, void *arg);
	static void FinishFightMoveCB(CAnimBlendAssociation *assoc, void *arg);
	static void PedAnimDoorCloseRollingCB(CAnimBlendAssociation *assoc, void *arg);
	static void FinishJumpCB(CAnimBlendAssociation *assoc, void *arg);
	static void PedLandCB(CAnimBlendAssociation *assoc, void *arg);
	static void RestoreHeadingRateCB(CAnimBlendAssociation *assoc, void *arg);
	static void PedSetQuickDraggedOutCarPositionCB(CAnimBlendAssociation *assoc, void *arg);
	static void PedSetDraggedOutCarPositionCB(CAnimBlendAssociation *assoc, void *arg);

	// functions that I see unnecessary to hook
	bool IsPlayer(void);
	bool UseGroundColModel(void);
	bool CanSetPedState(void);
	bool IsPedInControl(void);
	bool CanPedDriveOff(void);
	bool CanBeDeleted(void);
	bool CanStrafeOrMouseControl(void);
	bool CanPedReturnToState(void);
	void SetMoveState(eMoveState);
	bool IsTemporaryObjective(eObjective objective);
	void SetObjectiveTimer(int);
	bool SelectGunIfArmed(void);
	bool IsPointerValid(void);
	void SortPeds(CPed**, int, int);
	void ForceStoredObjective(eObjective);
	void SetStoredObjective(void);
	void SetLeader(CEntity* leader);
	void SetPedStats(ePedStats);
	bool IsGangMember(void);
	void Die(void);
	void EnterTrain(void);
	void ExitTrain(void);
	void Fall(void);
	bool IsPedShootable(void);
	void Look(void);
	void SetInTheAir(void);
	void RestoreHeadPosition(void);
	void PointGunAt(void);
	bool ServiceTalkingWhenDead(void);
	void SetPedPositionInTrain(void);
	void SetShootTimer(uint32);
	void SetSeekCar(CVehicle*, uint32);
	void SetSeekBoatPosition(CVehicle*);
	void SetExitTrain(CVehicle*);
	void WanderRange(void);
	void SetFollowRoute(int16, int16);
	void SeekBoatPosition(void);
	void UpdatePosition(void);
	CObject *SpawnFlyingComponent(int, int8);
	void SetCarJack_AllClear(CVehicle*, uint32, uint32);
#ifdef VC_PED_PORTS
	bool CanPedJumpThis(CEntity*, CVector*);
#else
	bool CanPedJumpThis(CEntity*);
#endif

	bool HasWeapon(uint8 weaponType) { return m_weapons[weaponType].m_eWeaponType == weaponType; }
	CWeapon &GetWeapon(uint8 weaponType) { return m_weapons[weaponType]; }
	CWeapon *GetWeapon(void) { return &m_weapons[m_currentWeapon]; }
	RwFrame *GetNodeFrame(int nodeId) { return m_pFrames[nodeId]->frame; }
	PedState GetPedState(void) { return m_nPedState; }
	void SetPedState(PedState state) { m_nPedState = state; }
	bool DyingOrDead(void) { return m_nPedState == PED_DIE || m_nPedState == PED_DEAD; }
	bool InVehicle(void) { return bInVehicle && m_pMyVehicle; } // True when ped is sitting/standing in vehicle, not in enter/exit state.
	void ReplaceWeaponWhenExitingVehicle(void);
	void RemoveWeaponWhenEnteringVehicle(void);
	bool IsNotInWreckedVehicle();

	// set by 0482:set_threat_reaction_range_multiplier opcode
	static uint16 &nThreatReactionRangeMultiplier;

	// set by 0481:set_enter_car_range_multiplier opcode
	static uint16 &nEnterCarRangeMultiplier;

	static bool &bNastyLimbsCheat;
	static bool &bPedCheat2;
	static bool &bPedCheat3;
	static CVector2D ms_vec2DFleePosition;
	static CPedAudioData (&CommentWaitTime)[38];

#ifdef TOGGLEABLE_BETA_FEATURES
	static bool bUnusedFightThingOnPlayer;
	static bool bPopHeadsOnHeadshot;
	static bool bMakePedsRunToPhonesToReportCrimes;
#endif

#ifndef MASTER
	// Mobile things
	static void SwitchDebugDisplay(void);
	void DebugRenderOnePedText(void);
#endif
};

class cPedParams
{
public:
	bool m_bDistanceCalculated;
	char gap_1[3];
	float m_fDistance;
	CPed *m_pPed;
};

void FinishFuckUCB(CAnimBlendAssociation *assoc, void *arg);

static_assert(offsetof(CPed, m_nPedState) == 0x224, "CPed: error");
static_assert(offsetof(CPed, m_pCurSurface) == 0x2FC, "CPed: error");
static_assert(offsetof(CPed, m_pMyVehicle) == 0x310, "CPed: error");
static_assert(offsetof(CPed, m_nPedType) == 0x32C, "CPed: error");
static_assert(offsetof(CPed, m_pCollidingEntity) == 0x34C, "CPed: error");
static_assert(offsetof(CPed, m_weapons) == 0x35C, "CPed: error");
static_assert(offsetof(CPed, m_currentWeapon) == 0x498, "CPed: error");
static_assert(offsetof(CPed, m_lookTimer) == 0x4CC, "CPed: error");
static_assert(offsetof(CPed, m_bodyPartBleeding) == 0x4F2, "CPed: error");
static_assert(offsetof(CPed, m_pedInObjective) == 0x16C, "CPed: error");
static_assert(offsetof(CPed, m_pEventEntity) == 0x19C, "CPed: error");
static_assert(sizeof(CPed) == 0x53C, "CPed: error");
