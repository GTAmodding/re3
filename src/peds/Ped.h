#pragma once

#include "RwHelper.h"
#include "AnimManager.h"
#include "Crime.h"
#include "EventList.h"
#include "PedIK.h"
#include "PedStats.h"
#include "Physical.h"
#include "Weapon.h"
#include "WeaponInfo.h"
#include "AnimationId.h"

#define FEET_OFFSET	1.04f
#define CHECK_NEARBY_THINGS_MAX_DIST	15.0f

struct CPathNode;
class CAccident;
class CObject;
class CFire;
struct AnimBlendFrameData;
class CAnimBlendAssociation;
class CPedAttractor;

struct PedAudioData
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
	float extendReachMultiplier;
	uint8 hitLevel; // FightMoveHitLevel
	uint8 damage;
	uint8 flags;
};

// TODO: This is eFightState on mobile.
enum PedFightMoves
{
	FIGHTMOVE_NULL,
	// Attacker
	FIGHTMOVE_STDPUNCH,
	FIGHTMOVE_IDLE,
	FIGHTMOVE_SHUFFLE_F,
	FIGHTMOVE_KNEE,
	FIGHTMOVE_PUNCHHOOK,
	FIGHTMOVE_PUNCHJAB,
	FIGHTMOVE_PUNCH,
	FIGHTMOVE_BODYBLOW = FIGHTMOVE_PUNCH,
	FIGHTMOVE_LONGKICK,
	FIGHTMOVE_ROUNDHOUSE,
	// Directionals
	FIGHTMOVE_FWDLEFT,
	FIGHTMOVE_FWDRIGHT,
	FIGHTMOVE_BACKKICK,
	FIGHTMOVE_BACKFLIP,
	FIGHTMOVE_BACKLEFT,
	FIGHTMOVE_BACKRIGHT,
	FIGHTMOVE_RIGHTSWEEP,
	// Special
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
	FIGHTMOVE_MELEE1,
	FIGHTMOVE_MELEE2,
	FIGHTMOVE_MELEE3,
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
	WAITSTATE_FINISH_FLEE,
	WAITSTATE_SIT_DOWN,
	WAITSTATE_SIT_DOWN_RVRS,
	WAITSTATE_SIT_UP,
	WAITSTATE_SIT_IDLE,
	WAITSTATE_USE_ATM,
	WAITSTATE_SUN_BATHE_PRE,
	WAITSTATE_SUN_BATHE_DOWN,
	WAITSTATE_SUN_BATHE_IDLE,
	WAITSTATE_RIOT,
	WAITSTATE_FAST_FALL,
	WAITSTATE_BOMBER,
	WAITSTATE_STRIPPER,
	WAITSTATE_GROUND_ATTACK,
	WAITSTATE_LANCESITTING,
	WAITSTATE_PLAYANIM_HANDSUP_SIMPLE,
};

enum eObjective : uint32 {
	OBJECTIVE_NONE,
	OBJECTIVE_IDLE,
	OBJ_2,
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
	OBJ_14,
	OBJ_15,
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
	OBJECTIVE_26,	// not implemented
	OBJECTIVE_27,	// not implemented
	OBJECTIVE_FIGHT_CHAR,
	OBJECTIVE_SET_LEADER,
	OBJECTIVE_FOLLOW_ROUTE,
	OBJECTIVE_SOLICIT,
	OBJECTIVE_HAIL_TAXI,
	OBJECTIVE_CATCH_TRAIN,
	OBJECTIVE_BUY_ICE_CREAM,
	OBJECTIVE_STEAL_ANY_CAR,
	OBJ_36,
	OBJECTIVE_MUG_CHAR,
	OBJECTIVE_LEAVE_CAR_AND_DIE,
	OBJECTIVE_USE_SEAT_ATTRACTOR,
	OBJECTIVE_USE_ATM_ATTRACTOR,
	OBJECTIVE_FLEE_CAR,
	OBJ_42,
	OBJECTIVE_USE_STOP_ATTRACTOR,
	OBJECTIVE_USE_PIZZA_ATTRACTOR,
	OBJECTIVE_USE_SHELTER_ATTRACTOR,
	OBJECTIVE_AIM_GUN_AT_PED,
	OBJ_47,
	OBJECTIVE_WAIT_FOR_RAIN_TO_END,
	OBJECTIVE_SPRINT_TO_COORD,
	OBJ_50,
	OBJ_51,
	OBJECTIVE_WAIT_FOR_BUS,
	OBJECTIVE_USE_ICECREAM_ATTRACTOR,
	OBJECTIVE_PURCHASE_ICECREAM,
	OBJ_55,
	OBJ_56,
	OBJ_57,
	OBJ_58,
	OBJ_59

};

enum {
	RANDOM_CHAR = 1,
	MISSION_CHAR,
	TODO_CHAR, // TODO(Miami)
};

enum PedLineUpPhase {
	LINE_UP_TO_CAR_START,
	LINE_UP_TO_CAR_END,
	LINE_UP_TO_CAR_2, // Buggy. Used for cops arresting you from passenger door
	LINE_UP_TO_CAR_FALL
};

enum PedOnGroundState {
	NO_PED,
	PED_IN_FRONT_OF_ATTACKER,
	PED_ON_THE_FLOOR,
	PED_DEAD_ON_THE_FLOOR
};

enum PointBlankNecessity : uint8 {
	NO_POINT_BLANK_PED,
	POINT_BLANK_FOR_WANTED_PED,
	POINT_BLANK_FOR_SOMEONE_ELSE
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
	PED_ROCKET_MODE,
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
	PED_SUN_BATHE,
	PED_FLASH,
	PED_JOG,
	PED_ANSWER_MOBILE,

	PED_UNKNOWN,	// Same with IDLE, but also infects up to 5 peds with same pedType and WANDER_PATH, so they become stone too. HANG_OUT in Fire_Head's idb

	PED_STATES_NO_AI,

	PED_ABSEIL,
	PED_SIT,
	PED_JUMP,
	PED_FALL,
	PED_GETUP,
	PED_STAGGER,
	PED_DIVE_AWAY,

	PED_STATES_NO_ST,
	PED_ENTER_TRAIN,
	PED_EXIT_TRAIN,
	PED_ARREST_PLAYER,

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
	PED_DEPLOY_STINGER
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
	uint32 bIsStanding : 1;
	uint32 bWasStanding : 1;
	uint32 bIsAttacking : 1;		// doesn't reset after fist fight
	uint32 bIsPointingGunAt : 1;
	uint32 bIsLooking : 1;
	uint32 bKeepTryingToLook : 1; // if we can't look somewhere due to unreachable angles
	uint32 bIsRestoringLook : 1;
	uint32 bIsAimingGun : 1;

	uint32 bIsRestoringGun : 1;
	uint32 bCanPointGunAtTarget : 1;
	uint32 bIsTalking : 1;
	uint32 bIsInTheAir : 1;
	uint32 bIsLanding : 1;
	uint32 bIsRunning : 1; // on some conditions
	uint32 bHitSomethingLastFrame : 1;
	uint32 bVehEnterDoorIsBlocked : 1; // because someone else enters/exits from there

	uint32 bCanPedEnterSeekedCar : 1;
	uint32 bRespondsToThreats : 1;
	uint32 bRenderPedInCar : 1;
	uint32 bChangedSeat : 1;
	uint32 bUpdateAnimHeading : 1;
	uint32 bBodyPartJustCameOff : 1;
	uint32 bIsShooting : 1;
	uint32 bFindNewNodeAfterStateRestore : 1;

	uint32 bHasACamera : 1; // does ped possess a camera to document accidents involves fire/explosion
	uint32 bGonnaInvestigateEvent : 1;
	uint32 bPedIsBleeding : 1;
	uint32 bStopAndShoot : 1; // Ped cannot reach target to attack with fist, need to use gun
	uint32 bIsPedDieAnimPlaying : 1;
	uint32 bUsePedNodeSeek : 1;
	uint32 bObjectiveCompleted : 1;
	uint32 bScriptObjectiveCompleted : 1;

	uint32 bKindaStayInSamePlace : 1;
	uint32 bBeingChasedByPolice : 1; // Unused VC leftover. Should've been set for criminal/gang members
	uint32 bNotAllowedToDuck : 1;
	uint32 bCrouchWhenShooting : 1;
	uint32 bIsDucking : 1;
	uint32 bGetUpAnimStarted : 1;
	uint32 bDoBloodyFootprints : 1;
	uint32 bFleeAfterExitingCar : 1;

	uint32 bWanderPathAfterExitingCar : 1;
	uint32 bIsLeader : 1;
	uint32 bDontDragMeOutCar : 1; // unfinished feature
	uint32 m_ped_flagF8 : 1;
	uint32 bWillBeQuickJacked : 1;
	uint32 bCancelEnteringCar : 1; // after door is opened or couldn't be opened due to it's locked
	uint32 bObstacleShowedUpDuringKillObjective : 1;
	uint32 bDuckAndCover : 1;

	uint32 bStillOnValidPoly : 1; // set if the polygon the ped is on is still valid for collision
	uint32 bAllowMedicsToReviveMe : 1;
	uint32 bResetWalkAnims : 1;
	uint32 bStartWanderPathOnFoot : 1; // exits the car if he's in it, reset after path found
	uint32 bOnBoat : 1; // not just driver, may be just standing
	uint32 bBusJacked : 1;
	uint32 bGonnaKillTheCarJacker : 1; // only set when car is jacked from right door and when arrested by police
	uint32 bFadeOut : 1;

	uint32 bKnockedUpIntoAir : 1; // has ped been knocked up into the air by a car collision
	uint32 bHitSteepSlope : 1; // has ped collided/is standing on a steep slope (surface type)
	uint32 bCullExtraFarAway : 1; // special ped only gets culled if it's extra far away (for roadblocks)
	uint32 bClearObjective : 1;
	uint32 bTryingToReachDryLand : 1; // has ped just exited boat and trying to get to dry land
	uint32 bCollidedWithMyVehicle : 1;
	uint32 bRichFromMugging : 1; // ped has lots of cash cause they've been mugging people
	uint32 bChrisCriminal : 1; // Is a criminal as killed during Chris' police mission (should be counted as such)

	uint32 bShakeFist : 1;  // test shake hand at look entity
	uint32 bNoCriticalHits : 1; // if set, limbs won't came off
	uint32 bVehExitWillBeInstant : 1;
	uint32 bHasAlreadyBeenRecorded : 1;
	uint32 bFallenDown : 1;
#ifdef PED_SKIN
	uint32 bDontAcceptIKLookAts : 1;	// TODO: find uses of this
#endif
	uint32 bReachedAttractorHeadingTarget : 1;
	uint32 bTurnedAroundOnAttractor : 1;

	uint32 bHasAlreadyUsedAttractor : 1;
	//uint32 b155_2
	uint32 bCarPassenger : 1;
	//uint32 b155_8
	//uint32 b155_10
	uint32 bMiamiViceCop : 1;
	uint32 bMoneyHasBeenGivenByScript : 1; //
	uint32 bHasBeenPhotographed : 1;  //

	uint32 bIsDrowning : 1;
	uint32 bDrownsInWater : 1;
	//uint32 b156_4
	uint32 b156_8 : 1;
	uint32 bIsPlayerFriend : 1;
#ifdef VC_PED_PORTS
	uint32 bHeadStuckInCollision : 1;
#endif
	uint32 bDeadPedInFrontOfCar : 1;
	uint32 bStayInCarOnJack : 1;

	uint32 bDontFight : 1;
	uint32 bDoomAim : 1;
	uint32 bCanBeShotInVehicle : 1;
	//uint32 b157_8
	//uint32 b157_10
	//uint32 b157_20
	//uint32 b157_40
	uint32 bIgnoreThreatsBehindObjects : 1;

	uint32 bNeverEverTargetThisPed : 1;
	//uint32 b158_2
	uint32 b158_4 : 1;
	//uint32 b158_8
	//uint32 b158_10
	uint32 bBoughtIceCream : 1;
	//uint32 b158_40
	//uint32 b158_80

	// our own flags
	uint32 m_ped_flagI40 : 1; // bMakePedsRunToPhonesToReportCrimes makes use of this as runover by car indicator
	uint32 m_ped_flagI80 : 1; // KANGAROO_CHEAT define makes use of this as cheat toggle 

	uint8 m_gangFlags;
	uint8 CharCreatedBy;
	eObjective m_objective;
	eObjective m_prevObjective;
	CPed *m_pedInObjective;
	CVehicle *m_carInObjective;
	CVector m_nextRoutePointPos;
	float m_attractorHeading;
	CPed *m_leader;
	eFormation m_pedFormation;
	uint32 m_fearFlags;
	CEntity *m_threatEntity;
	CVector2D m_eventOrThreat;
	uint32 m_eventType;
	CEntity* m_pEventEntity;
	float m_fAngleToEvent;
	AnimBlendFrameData *m_pFrames[PED_NODE_MAX];
#ifdef PED_SKIN
	// stored inside the clump with non-skin ped
	RpAtomic *m_pWeaponModel;
#endif
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
	CPathNode *m_pLastPathNode;
	CPathNode *m_pNextPathNode;
	float m_fHealth;
	float m_fArmour;
	uint32 m_nExtendedRangeTimer;
	int16 m_routeLastPoint;
	uint16 m_routeStartPoint;
	int16 m_routePointsPassed;
	int16 m_routeType;	// See PedRouteType
	int16 m_routePointsBeingPassed;
	CVector2D m_moved;
	float m_fRotationCur;
	float m_fRotationDest;
	float m_headingRate;
	uint16 m_vehEnterType;
	int16 m_walkAroundType;
	CPhysical *m_pCurrentPhysSurface;
	CVector m_vecOffsetFromPhysSurface;
	CEntity *m_pCurSurface;
	CVector m_vecSeekPos;
	CEntity *m_pSeekTarget;
	CVehicle *m_pMyVehicle;
	bool bInVehicle;
	float m_distanceToCountSeekDone;
	float m_acceptableHeadingOffset;
	CPedAttractor* m_attractor;
	int32 m_positionInQueue;
	CVehicle* m_vehicleInAccident;

	bool bRunningToPhone;
	int16 m_phoneId;
	eCrimeType m_crimeToReportOnPhone;
	uint32 m_phoneTalkTimer;
	CAccident *m_lastAccident;
	uint32 m_nPedType;
	CPedStats *m_pedStats;
	float m_fleeFromPosX;
	float m_fleeFromPosY;
	CEntity *m_fleeFrom;
	uint32 m_fleeTimer;
	CEntity* pThreatEx; // TODO(Miami): What is this?
	CEntity* m_collidingEntityWhileFleeing;
	uint32 m_collidingThingTimer;
	CEntity *m_pCollidingEntity;
	uint8 m_stateUnused;
	uint32 m_timerUnused;
	class CRange2D *m_wanderRangeBounds;
	CWeapon m_weapons[TOTAL_WEAPON_SLOTS];
	eWeaponType m_storedWeapon;
	eWeaponType m_delayedWeapon;
	uint32 m_delayedWeaponAmmo;
	uint8 m_currentWeapon;			// eWeaponType
	uint8 m_maxWeaponTypeAllowed;	// eWeaponType
	uint8 m_wepSkills;
	uint8 m_wepAccuracy;
	CEntity *m_pPointGunAt;
	CVector m_vecHitLastPos;
	uint32 m_lastFightMove;
	uint8 m_fightButtonPressure;
	FightState m_fightState;
	bool m_takeAStepAfterAttack;
	uint8 m_bleedCounter;
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
	uint32 m_bloodyFootprintCountOrDeathTime;	// Death time when bDoBloodyFootprints is false. Weird decision
	uint32 m_shotTime;
	uint32 m_ceaseAttackTimer;
	uint8 m_panicCounter;
	bool m_deadBleeding;
	int8 m_bodyPartBleeding;		// PedNode, but -1 if there isn't
	CPed *m_nearPeds[10];
	uint16 m_numNearPeds;
	uint16 m_nPedMoney;
 	int8 m_lastWepDam;
	CEntity *m_lastDamEntity;
	CEntity *m_attachedTo;
	CVector m_vecAttachOffset;
	uint16 m_attachType;
	float m_attachRot;
	uint32 m_attachWepAmmo;
	uint32 m_threatFlags;
	uint32 m_threatCheck;
	uint32 m_lastThreatCheck;
	uint32 m_sayType;
	uint32 m_sayTimer;
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
	void SetLookFlag(CEntity *target, bool keepTryingToLook);
	void SetLookFlag(float direction, bool keepTryingToLook);
	void SetLookTimer(int time);
	void SetDie(AnimationId anim = ANIM_KO_SHOT_FRONT1, float arg1 = 4.0f, float arg2 = 0.0f);
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
	void SetCurrentWeapon(eWeaponType weaponType);
	void SetCurrentWeapon(int weapon);
	void Duck(void);
	void ClearDuck(bool = false);
	void ClearPointGunAt(void);
	void BeingDraggedFromCar(void);
	void RestartNonPartialAnims(void);
	void LineUpPedWithCar(PedLineUpPhase phase);
	void SetPedPositionInCar(void);
	void PlayFootSteps(void);
	void QuitEnteringCar(void);
	void BuildPedLists(void);
	int32 GiveWeapon(eWeaponType weaponType, uint32 ammo, bool unused = false);
	void CalculateNewOrientation(void);
	float WorkOutHeadingForMovingFirstPerson(float);
	void CalculateNewVelocity(void);
	bool CanSeeEntity(CEntity*, float);
	void RestorePreviousObjective(void);
	void SetIdle(void);
#ifdef _MSC_VER
	__declspec(noinline) // workaround for a compiler bug, hooray MS :P
#endif
	void SetObjective(eObjective, void*);
	void SetObjective(eObjective);
	void SetObjective(eObjective, int16, int16);
	void SetObjective(eObjective, CVector);
	void SetObjective(eObjective, CVector, float);
	void SetObjective(eObjective, float, const CVector&);
	void ClearChat(void);
	void InformMyGangOfAttack(CEntity*);
	void ReactToAttack(CEntity*);
	void SetDuck(uint32, bool = false);
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
	PointBlankNecessity CheckForPointBlankPeds(CPed*);
	bool CheckIfInTheAir(void);
	void ClearAll(void);
	void SetPointGunAt(CEntity*);
	bool Seek(void);
	bool SetWanderPath(int8);
	bool SetFollowPath(CVector dest, float radius, eMoveState state, CEntity*, CEntity*, int);
	bool SetFollowPathStatic(void);
	bool SetFollowPathDynamic(void);
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
	void RemoveWeaponAnims(int, float);
	void CreateDeadPedMoney(void);
	void CreateDeadPedWeaponPickups(void);
	void CreateDeadPedPickupCoors(float *x, float *y, float *z);
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
	void SetEnterCar_AllClear(CVehicle*, uint32, uint32);
	void SetSolicit(uint32 time);
	void ScanForInterestingStuff(void);
	void WarpPedIntoCar(CVehicle*);
	void SetCarJack(CVehicle*);
	bool WarpPedToNearLeaderOffScreen(void);
	void Solicit(void);
	void SetExitBoat(CVehicle*);
	void ClearFollowPath();
	void GiveDelayedWeapon(eWeaponType weapon, uint32 ammo);
	void RequestDelayedWeapon();
	void AddInCarAnims(CVehicle* car, bool isDriver);
	bool CanBeDamagedByThisGangMember(CPed*);
	void AnswerMobile(void);
	void BuyIceCream(void);
	void CheckThreatValidity(void);
	void ClearAnswerMobile(void);
	void SetAnswerMobile(void);
	void AttachPedToEntity(CEntity*, CVector, uint16, float, eWeaponType);
	void DettachPedFromEntity();
	void PedShuffle();
	void DriveVehicle();

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
	static void PedSetOutTrainCB(CAnimBlendAssociation *assoc, void *arg); // TODO(Miami): Should be under GTA_TRAIN
	static void FinishedAttackCB(CAnimBlendAssociation *assoc, void *arg);
	static void FinishedReloadCB(CAnimBlendAssociation *assoc, void *arg);
	static void FinishFightMoveCB(CAnimBlendAssociation *assoc, void *arg);
	static void PedAnimDoorCloseRollingCB(CAnimBlendAssociation *assoc, void *arg);
	static void FinishJumpCB(CAnimBlendAssociation *assoc, void *arg);
	static void PedLandCB(CAnimBlendAssociation *assoc, void *arg);
	static void RestoreHeadingRateCB(CAnimBlendAssociation *assoc, void *arg);
	static void PedSetQuickDraggedOutCarPositionCB(CAnimBlendAssociation *assoc, void *arg);
	static void PedSetDraggedOutCarPositionCB(CAnimBlendAssociation *assoc, void *arg);

	bool IsPlayer(void);
	bool IsFemale(void) { return m_nPedType == PEDTYPE_CIVFEMALE || m_nPedType == PEDTYPE_PROSTITUTE; }
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
#ifdef GTA_TRAIN
	void EnterTrain(void);
	void ExitTrain(void);
	void SetExitTrain(CVehicle*);
	void SetPedPositionInTrain(void);
	void LineUpPedWithTrain(void);
	void SetEnterTrain(CVehicle*, uint32);
#endif
	void Fall(void);
	bool IsPedShootable(void);
	void Look(void);
	void SetInTheAir(void);
	void RestoreHeadPosition(void);
	void PointGunAt(void);
	bool ServiceTalkingWhenDead(void);
	void SetShootTimer(uint32);
	void SetSeekCar(CVehicle*, uint32);
	void SetSeekBoatPosition(CVehicle*);
	void WanderRange(void);
	void SetFollowRoute(int16, int16);
	void SeekBoatPosition(void);
	void UpdatePosition(void);
	CObject *SpawnFlyingComponent(int, int8);
	void SetCarJack_AllClear(CVehicle*, uint32, uint32);
	bool CanPedJumpThis(CEntity*, CVector*);

	void SetNewAttraction(CPedAttractor* pAttractor, const CVector& pos, float, float, int);
	void ClearWaitState(void);
	void Undress(const char*);
	void Dress(void);

	bool HasWeaponSlot(uint8 slot) { return m_weapons[slot].m_eWeaponType != WEAPONTYPE_UNARMED; }
	CWeapon& GetWeapon(uint8 slot) { return m_weapons[slot]; }
	CWeapon *GetWeapon(void) { return &m_weapons[m_currentWeapon]; }

	PedState GetPedState(void) { return m_nPedState; }
	void SetPedState(PedState state) 
	{
		if (GetPedState() == PED_FOLLOW_PATH)
			ClearFollowPath();
		m_nPedState = state;
	}
	bool Dead(void) { return m_nPedState == PED_DEAD; }
	bool Dying(void) { return m_nPedState == PED_DIE; }
	bool DyingOrDead(void) { return m_nPedState == PED_DIE || m_nPedState == PED_DEAD; }
	bool OnGround(void) { return m_nPedState == PED_FALL || m_nPedState == PED_DIE || m_nPedState == PED_DEAD; }
	
	bool Driving(void) { return m_nPedState == PED_DRIVING; }
	bool InVehicle(void) { return bInVehicle && m_pMyVehicle; } // True when ped is sitting/standing in vehicle, not in enter/exit state.
	bool EnteringCar(void) { return m_nPedState == PED_ENTER_CAR || m_nPedState == PED_CARJACK; }
	bool HasAttractor(void) { return m_attractor != nil; }
	bool IsUseAttractorObjective(eObjective obj) {
		return obj == OBJECTIVE_USE_ATM_ATTRACTOR || obj == OBJECTIVE_USE_ICECREAM_ATTRACTOR ||
			obj == OBJECTIVE_USE_PIZZA_ATTRACTOR || obj == OBJECTIVE_USE_SEAT_ATTRACTOR ||
			obj == OBJECTIVE_USE_SHELTER_ATTRACTOR || obj == OBJECTIVE_USE_STOP_ATTRACTOR;
	}

	void ReplaceWeaponWhenExitingVehicle(void);
	void RemoveWeaponWhenEnteringVehicle(void);
	bool IsNotInWreckedVehicle();

	// My names. Inlined in VC
	AnimationId GetFireAnimNotDucking(CWeaponInfo* weapon) {
		if (m_nPedType == PEDTYPE_COP && !!weapon->m_bCop3rd)
			return ANIM_WEAPON_FIRE_3RD;
		else
			return GetPrimaryFireAnim(weapon);
	}

	static AnimationId GetFireAnimGround(CWeaponInfo* weapon, bool kickFloorIfNone = true) {
		if (!!weapon->m_bGround2nd)
			return ANIM_WEAPON_CROUCHFIRE;
		else if (!!weapon->m_bGround3rd)
			return ANIM_WEAPON_FIRE_3RD;
		else if (kickFloorIfNone)
			return ANIM_KICK_FLOOR;
		else
			return (AnimationId)0;
	}

	static AnimationId GetPrimaryFireAnim(CWeaponInfo* weapon) {
		if (weapon->m_bAnimDetonate)
			return ANIM_BOMBER;
		else
			return ANIM_WEAPON_FIRE;
	}

	static AnimationId GetCrouchReloadAnim(CWeaponInfo* weapon) {
		if (!!weapon->m_bReload)
			return ANIM_WEAPON_CROUCHRELOAD;
		else
			return (AnimationId)0;
	}

	static AnimationId GetCrouchFireAnim(CWeaponInfo* weapon) {
		if (!!weapon->m_bCrouchFire)
			return ANIM_WEAPON_CROUCHFIRE;
		else
			return (AnimationId)0;
	}

	static AnimationId GetReloadAnim(CWeaponInfo* weapon) {
		if (!!weapon->m_bReload)
			return ANIM_WEAPON_RELOAD;
		else
			return (AnimationId)0;
	}
	// --

	// My additions, because there were many, many instances of that.
	inline void SetFindPathAndFlee(CEntity *fleeFrom, int time, bool walk = false)
	{
		SetFlee(fleeFrom, time);
		bUsePedNodeSeek = true;
		m_pNextPathNode = nil;
		if (walk)
			SetMoveState(PEDMOVE_WALK);
	}

	inline void SetFindPathAndFlee(CVector2D const &from, int time, bool walk = false)
	{
		SetFlee(from, time);
		bUsePedNodeSeek = true;
		m_pNextPathNode = nil;
		if (walk)
			SetMoveState(PEDMOVE_WALK);
	}
	// --

	// Using this to abstract nodes of skinned and non-skinned meshes
	CVector GetNodePosition(int32 node)
	{
		RwV3d pos = { 0.0f, 0.0f, 0.0f };
		RpHAnimHierarchy *hier = GetAnimHierarchyFromSkinClump(GetClump());
		int32 idx = RpHAnimIDGetIndex(hier, m_pFrames[node]->nodeID);
		RwMatrix *mats = RpHAnimHierarchyGetMatrixArray(hier);
		pos = mats[idx].pos;
		return pos;
	}
	void TransformToNode(CVector &pos, int32 node)
	{
		RpHAnimHierarchy *hier = GetAnimHierarchyFromSkinClump(GetClump());
		int32 idx = RpHAnimIDGetIndex(hier, m_pFrames[node]->nodeID);
		RwMatrix *mats = RpHAnimHierarchyGetMatrixArray(hier);
		RwV3dTransformPoints((RwV3d*)&pos, (RwV3d*)&pos, 1, &mats[idx]);
	}

	// set by 0482:set_threat_reaction_range_multiplier opcode
	static uint16 nThreatReactionRangeMultiplier;

	// set by 0481:set_enter_car_range_multiplier opcode
	static uint16 nEnterCarRangeMultiplier;

	static bool bNastyLimbsCheat;
	static bool bPedCheat2;
	static bool bPedCheat3;
	static CVector2D ms_vec2DFleePosition;

#ifdef TOGGLEABLE_BETA_FEATURES
	static bool bPopHeadsOnHeadshot;
	static bool bMakePedsRunToPhonesToReportCrimes;
#endif

#ifndef MASTER
	// Mobile things
	void DebugDrawPedDestination(CPed *, int, int);
	void DebugDrawPedDesiredHeading(CPed *, int, int);
	void DebugDrawCollisionRadius(float, float, float, float, int);
	void DebugDrawVisionRange(CVector, float);
	void DebugDrawVisionSimple(CVector, float);
	void DebugDrawLook();
	void DebugDrawPedPsyche();
	void DebugDrawDebugLines();

	static void SwitchDebugDisplay(void);
	static int GetDebugDisplay(void);

	void DebugDrawLookAtPoints();
	void DebugRenderOnePedText(void);
	void DebugRenderClosePedText();
#endif

#ifdef COMPATIBLE_SAVES
	virtual void Save(uint8*& buf);
	virtual void Load(uint8*& buf);
#endif
};

void FinishTalkingOnMobileCB(CAnimBlendAssociation* assoc, void* arg);
void StartTalkingOnMobileCB(CAnimBlendAssociation* assoc, void* arg);
void FinishFuckUCB(CAnimBlendAssociation *assoc, void *arg);

// TODO(Miami): Change those when Ped struct is done
#ifndef PED_SKIN
VALIDATE_SIZE(CPed, 0x53C);
#endif

bool IsPedPointerValid(CPed*);
bool IsPedPointerValid_NotInWorld(CPed*);
