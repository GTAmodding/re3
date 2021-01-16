#pragma once

#include "Physical.h"
#include "AutoPilot.h"
#include "ModelIndices.h"
#include "AnimationId.h"
#include "WeaponType.h"
#include "Collision.h"

class CPed;
class CFire;
struct tHandlingData;

enum {
	RANDOM_VEHICLE = 1,
	MISSION_VEHICLE = 2,
	PARKED_VEHICLE = 3,
	PERMANENT_VEHICLE = 4,
};

enum eCarLock {
	CARLOCK_NOT_USED,
	CARLOCK_UNLOCKED,
	CARLOCK_LOCKED,
	CARLOCK_LOCKOUT_PLAYER_ONLY,
	CARLOCK_LOCKED_PLAYER_INSIDE,
	CARLOCK_LOCKED_INITIALLY,
	CARLOCK_FORCE_SHUT_DOORS,
	CARLOCK_SKIP_SHUT_DOORS
};

enum eDoors
{
	DOOR_BONNET = 0,
	DOOR_BOOT,
	DOOR_FRONT_LEFT,
	DOOR_FRONT_RIGHT,
	DOOR_REAR_LEFT,
	DOOR_REAR_RIGHT
};

enum ePanels
{
	VEHPANEL_FRONT_LEFT,
	VEHPANEL_FRONT_RIGHT,
	VEHPANEL_REAR_LEFT,
	VEHPANEL_REAR_RIGHT,
	VEHPANEL_WINDSCREEN,
	VEHBUMPER_FRONT,
	VEHBUMPER_REAR,
};

enum eLights
{
	VEHLIGHT_FRONT_LEFT,
	VEHLIGHT_FRONT_RIGHT,
	VEHLIGHT_REAR_LEFT,
	VEHLIGHT_REAR_RIGHT,
};

enum
{
	CAR_PIECE_BONNET = 1,
	CAR_PIECE_BOOT,
	CAR_PIECE_BUMP_FRONT,
	CAR_PIECE_BUMP_REAR,
	CAR_PIECE_DOOR_LF,
	CAR_PIECE_DOOR_RF,
	CAR_PIECE_DOOR_LR,
	CAR_PIECE_DOOR_RR,
	CAR_PIECE_WING_LF,
	CAR_PIECE_WING_RF,
	CAR_PIECE_WING_LR,
	CAR_PIECE_WING_RR,
	CAR_PIECE_WHEEL_LF,
	CAR_PIECE_WHEEL_RF,
	CAR_PIECE_WHEEL_LR,
	CAR_PIECE_WHEEL_RR,
	CAR_PIECE_WINDSCREEN,
};

enum tWheelState
{
	WHEEL_STATE_NORMAL,	// standing still or rolling normally
	WHEEL_STATE_SPINNING,	// rotating but not moving
	WHEEL_STATE_SKIDDING,
	WHEEL_STATE_FIXED,	// not rotating
};

enum eFlightModel
{
	FLIGHT_MODEL_DODO,
	// not used in III
	FLIGHT_MODEL_RCPLANE,
	FLIGHT_MODEL_HELI,
	FLIGHT_MODEL_SEAPLANE
};

// TODO: what is this even?
enum eBikeWheelSpecial {
	BIKE_WHEELSPEC_0, // both wheels on ground
	BIKE_WHEELSPEC_1, // rear wheel on ground
	BIKE_WHEELSPEC_2, // only front wheel on ground
	BIKE_WHEELSPEC_3, // can't happen
};


class CVehicle : public CPhysical
{
public:
	// 0x128
	tHandlingData *pHandling;
	CAutoPilot AutoPilot;
	uint8 m_currentColour1;
	uint8 m_currentColour2;
	int8 m_aExtras[2];
	int16 m_nAlarmState;
	int16 m_nMissionValue;
	CPed *pDriver;
	CPed *pPassengers[8];
	uint8 m_nNumPassengers;
	int8 m_nNumGettingIn;
	int8 m_nGettingInFlags;
	int8 m_nGettingOutFlags;
	uint8 m_nNumMaxPassengers;
	float field_1D0[4];
	CEntity *m_pCurGroundEntity;
	CFire *m_pCarFire;
	float m_fSteerAngle;
	float m_fGasPedal;
	float m_fBrakePedal;
	uint8 VehicleCreatedBy;

	// cf. https://github.com/DK22Pac/plugin-sdk/blob/master/plugin_sa/game_sa/CVehicle.h from R*
	uint8 bIsLawEnforcer: 1; // Is this guy chasing the player at the moment
	uint8 bIsAmbulanceOnDuty: 1; // Ambulance trying to get to an accident
	uint8 bIsFireTruckOnDuty: 1; // Firetruck trying to get to a fire
	uint8 bIsLocked: 1; // Is this guy locked by the script (cannot be removed)
	uint8 bEngineOn: 1; // For sound purposes. Parked cars have their engines switched off (so do destroyed cars)
	uint8 bIsHandbrakeOn: 1; // How's the handbrake doing ?
	uint8 bLightsOn: 1; // Are the lights switched on ?
	uint8 bFreebies: 1; // Any freebies left in this vehicle ?

	uint8 bIsVan: 1; // Is this vehicle a van (doors at back of vehicle)
	uint8 bIsBus: 1; // Is this vehicle a bus
	uint8 bIsBig: 1; // Is this vehicle a bus
	uint8 bLowVehicle: 1; // Need this for sporty type cars to use low getting-in/out anims
	uint8 bComedyControls : 1; // Will make the car hard to control (hopefully in a funny way)
	uint8 bWarnedPeds : 1; // Has scan and warn peds of danger been processed?
	uint8 bCraneMessageDone : 1; // A crane message has been printed for this car allready
	uint8 bExtendedRange : 1; // This vehicle needs to be a bit further away to get deleted

	uint8 bTakeLessDamage : 1; // This vehicle is stronger (takes about 1/4 of damage)
	uint8 bIsDamaged : 1; // This vehicle has been damaged and is displaying all its components
	uint8 bHasBeenOwnedByPlayer : 1;// To work out whether stealing it is a crime
	uint8 bFadeOut : 1; // Fade vehicle out
	uint8 bIsBeingCarJacked : 1; // Fade vehicle out
	uint8 bCreateRoadBlockPeds : 1; // If this vehicle gets close enough we will create peds (coppers or gang members) round it
	uint8 bCanBeDamaged : 1; // Set to FALSE during cut scenes to avoid explosions
	uint8 bUsingSpecialColModel : 1;// Is player vehicle using special collision model, stored in player strucure

	uint8 bOccupantsHaveBeenGenerated : 1;  // Is true if the occupants have already been generated. (Shouldn't happen again)
	uint8 bGunSwitchedOff : 1;  // Level designers can use this to switch off guns on boats
	uint8 bVehicleColProcessed : 1;// Has ProcessEntityCollision been processed for this car?
	uint8 bIsCarParkVehicle : 1; // Car has been created using the special CAR_PARK script command
	uint8 bHasAlreadyBeenRecorded : 1; // Used for replays

	int8 m_numPedsUseItAsCover;
	uint8 m_nAmmoInClip;    // Used to make the guns on boat do a reload (20 by default)
	int8 m_nPacManPickupsCarried;
	uint8 m_nRoadblockType;
	int16 m_nRoadblockNode;
	float m_fHealth;           // 1000.0f = full health. 250.0f = fire. 0 -> explode
	uint8 m_nCurrentGear;
	float m_fChangeGearTime;
	uint32 m_nGunFiringTime;    // last time when gun on vehicle was fired (used on boats)
	uint32 m_nTimeOfDeath;
	uint16 m_nTimeBlocked;
	int16 m_nBombTimer;        // goes down with each frame
	CEntity *m_pBlowUpEntity;
	float m_fMapObjectHeightAhead;	// front Z?
	float m_fMapObjectHeightBehind;	// rear Z?
	eCarLock m_nDoorLock;
	int8 m_nLastWeaponDamage; // see eWeaponType, -1 if no damage
	int8 m_nRadioStation;
	uint8 m_bRainAudioCounter;
	uint8 m_bRainSamplesCounter;
	uint8 m_nCarHornTimer;
	uint8 m_nCarHornPattern; // last horn?
	bool m_bSirenOrAlarm;
	int8 m_comedyControlState;
	CStoredCollPoly m_aCollPolys[2];     // poly which is under front/rear part of car
	float m_fSteerInput;
	eVehicleType m_vehType;

	static void *operator new(size_t);
	static void *operator new(size_t sz, int slot);
	static void operator delete(void*, size_t);
	static void operator delete(void*, int);

	CVehicle(void) {}	// FAKE
	CVehicle(uint8 CreatedBy);
	~CVehicle(void);
	// from CEntity
	void SetModelIndex(uint32 id);
	bool SetupLighting(void);
	void RemoveLighting(bool);
	void FlagToDestroyWhenNextProcessed(void) {}

	virtual void ProcessControlInputs(uint8) {}
	virtual void GetComponentWorldPosition(int32 component, CVector &pos) {}
	virtual bool IsComponentPresent(int32 component) { return false; }
	virtual void SetComponentRotation(int32 component, CVector rotation) {}
	virtual void OpenDoor(int32, eDoors door, float) {}
	virtual void ProcessOpenDoor(uint32, uint32, float) {}
	virtual bool IsDoorReady(eDoors door) { return false; }
	virtual bool IsDoorFullyOpen(eDoors door) { return false; }
	virtual bool IsDoorClosed(eDoors door) { return false; }
	virtual bool IsDoorMissing(eDoors door) { return false; }
	virtual void RemoveRefsToVehicle(CEntity *ent) {}
	virtual void BlowUpCar(CEntity *ent) {}
	virtual bool SetUpWheelColModel(CColModel *colModel) { return false; }
	virtual void BurstTyre(uint8 tyre) {}
	virtual bool IsRoomForPedToLeaveCar(uint32 component, CVector *forcedDoorPos) { return false;}
	virtual float GetHeightAboveRoad(void);
	virtual void PlayCarHorn(void) {}
#ifdef COMPATIBLE_SAVES
	virtual void Save(uint8*& buf);
	virtual void Load(uint8*& buf);
#endif

	bool IsCar(void) { return m_vehType == VEHICLE_TYPE_CAR; }
	bool IsBoat(void) { return m_vehType == VEHICLE_TYPE_BOAT; }
	bool IsTrain(void) { return m_vehType == VEHICLE_TYPE_TRAIN; }
	bool IsHeli(void) { return m_vehType == VEHICLE_TYPE_HELI; }
	bool IsPlane(void) { return m_vehType == VEHICLE_TYPE_PLANE; }
	bool IsBike(void) { return m_vehType == VEHICLE_TYPE_BIKE; }

	void FlyingControl(eFlightModel flightModel);
	void ProcessWheel(CVector &wheelFwd, CVector &wheelRight, CVector &wheelContactSpeed, CVector &wheelContactPoint,
		int32 wheelsOnGround, float thrust, float brake, float adhesion, int8 wheelId, float *wheelSpeed, tWheelState *wheelState, uint16 wheelStatus);
	void ProcessBikeWheel(CVector &wheelFwd, CVector &wheelRight, CVector &wheelContactSpeed, CVector &wheelContactPoint, int32 wheelsOnGround, float thrust,
	                      float brake, float adhesion, int8 wheelId, float *wheelSpeed, tWheelState *wheelState, eBikeWheelSpecial special, uint16 wheelStatus);
	void ExtinguishCarFire(void);
	void ProcessDelayedExplosion(void);
	float ProcessWheelRotation(tWheelState state, const CVector &fwd, const CVector &speed, float radius);
	bool IsLawEnforcementVehicle(void);
	void ChangeLawEnforcerState(uint8 enable);
	bool UsesSiren(uint32 id);
	bool IsVehicleNormal(void);
	bool CarHasRoof(void);
	bool IsUpsideDown(void);
	bool IsOnItsSide(void);
	bool CanBeDeleted(void);
	bool CanPedOpenLocks(CPed *ped);
	bool CanPedEnterCar(void);
	bool CanPedExitCar(void);
	// do these two actually return something?
	CPed *SetUpDriver(void);
	CPed *SetupPassenger(int n);
	void SetDriver(CPed *driver);
	bool AddPassenger(CPed *passenger);
	bool AddPassenger(CPed *passenger, uint8 n);
	void RemovePassenger(CPed *passenger);
	void RemoveDriver(void);
	void ProcessCarAlarm(void);
	bool IsSphereTouchingVehicle(float sx, float sy, float sz, float radius);
	bool ShufflePassengersToMakeSpace(void);
	void InflictDamage(CEntity *damagedBy, eWeaponType weaponType, float damage);
	void DoFixedMachineGuns(void);

#ifdef FIX_BUGS
	bool IsAlarmOn(void) { return m_nAlarmState != 0 && m_nAlarmState != -1 && GetStatus() != STATUS_WRECKED; }
#else
	bool IsAlarmOn(void) { return m_nAlarmState != 0 && m_nAlarmState != -1; }
#endif
	CVehicleModelInfo* GetModelInfo() { return (CVehicleModelInfo*)CModelInfo::GetModelInfo(GetModelIndex()); }
	bool IsTaxi(void) { return GetModelIndex() == MI_TAXI || GetModelIndex() == MI_CABBIE || GetModelIndex() == MI_BORGNINE; }
	AnimationId GetDriverAnim(void) { return IsCar() && bLowVehicle ? ANIM_CAR_LSIT : (IsBoat() && GetModelIndex() != MI_SPEEDER ? ANIM_DRIVE_BOAT : ANIM_CAR_SIT); }

	static bool bWheelsOnlyCheat;
	static bool bAllDodosCheat;
	static bool bCheat3;
	static bool bCheat4;
	static bool bCheat5;
#ifdef ALT_DODO_CHEAT
	static bool bAltDodoCheat;
#endif
	static bool m_bDisableMouseSteering;
};

VALIDATE_SIZE(CVehicle, 0x288);

void DestroyVehicleAndDriverAndPassengers(CVehicle* pVehicle);
