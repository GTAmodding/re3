#pragma once

#include "Physical.h"
#include "AutoPilot.h"
#include "ModelIndices.h"
#include "AnimManager.h"
#include "Weapon.h"
#include "HandlingMgr.h"

class CPed;
class CPlayerPed;
class CCopPed;
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


enum
{
	BOAT_POS_FRONTSEAT
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

enum eWheels
{
	VEHWHEEL_FRONT_LEFT,
	VEHWHEEL_FRONT_RIGHT,
	VEHWHEEL_REAR_LEFT,
	VEHWHEEL_REAR_RIGHT,
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
	FLIGHT_MODEL_RCPLANE,
	FLIGHT_MODEL_RCHELI,
	FLIGHT_MODEL_SEAPLANE,
	FLIGHT_MODEL_PLANE_UNUSED,
	FLIGHT_MODEL_PLANE,
	FLIGHT_MODEL_HELI
};

enum eVehicleAppearance
{
	VEHICLE_APPEARANCE_NONE,
	VEHICLE_APPEARANCE_CAR,
	VEHICLE_APPEARANCE_BIKE,
	VEHICLE_APPEARANCE_HELI,
	VEHICLE_APPEARANCE_BOAT,
	VEHICLE_APPEARANCE_PLANE,
};

// TODO
enum eBikeWheelSpecial
{
	BIKE_WHEEL_2 = 2,
	BIKE_WHEEL_3,
};

enum
{
	ROTOR_TOP = 3,
	ROTOR_FRONT = 4,
	ROTOR_RIGHT = 5,
	ROTOR_LEFT = 7,
	ROTOR_BACK = 8,
	ROTOR_BOTTOM = 9,
};

class CVehicle : public CPhysical
{
public:
	tHandlingData *pHandling;
	tFlyingHandlingData *pFlyingHandling;
	CAutoPilot AutoPilot;
	uint8 m_currentColour1;
	uint8 m_currentColour2;
	uint8 m_aExtras[2];
	int16 m_nAlarmState;
	int16 m_nRouteSeed;
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
	uint8 bPartOfConvoy : 1;
	uint8 bHeliMinimumTilt : 1; // This heli should have almost no tilt really
	uint8 bAudioChangingGear : 1; // sounds like vehicle is changing gear

	uint8 bIsDrowning : 1; // is vehicle occupants taking damage in water (i.e. vehicle is dead in water)
	uint8 bTyresDontBurst : 1; // If this is set the tyres are invincible
	uint8 bCreatedAsPoliceVehicle : 1;// True if this guy was created as a police vehicle (enforcer, policecar, miamivice car etc)
	uint8 bRestingOnPhysical : 1; // Dont go static cause car is sitting on a physical object that might get removed
	uint8 bParking : 1;
	uint8 bCanPark : 1;

	uint8 m_bombType : 3;
	uint8 bDriverLastFrame : 1;

	int8 m_numPedsUseItAsCover;
	uint8 m_nAmmoInClip;    // Used to make the guns on boat do a reload (20 by default)
	int8 m_nPacManPickupsCarried;
	uint8 m_nRoadblockType;
	float m_fHealth;           // 1000.0f = full health. 250.0f = fire. 0 -> explode
	uint8 m_nCurrentGear;
	float m_fChangeGearTime;
	CEntity* m_pBombRigger;
	uint32 m_nSetPieceExtendedRangeTime;
	uint32 m_nGunFiringTime;    // last time when gun on vehicle was fired (used on boats)
	uint32 m_nTimeOfDeath;
	uint16 m_nTimeBlocked;
	int16 m_nBombTimer;        // goes down with each frame
	CEntity *m_pBlowUpEntity;
	float m_fMapObjectHeightAhead;	// front Z?
	float m_fMapObjectHeightBehind;	// rear Z?
	eCarLock m_nDoorLock;
	int8 m_nLastWeaponDamage; // see eWeaponType, -1 if no damage
	CEntity *m_pLastDamageEntity;
	int8 m_nRadioStation;
	uint8 m_bRainAudioCounter;
	uint8 m_bRainSamplesCounter;
	uint32 m_nCarHornTimer;
	uint8 m_nCarHornPattern;
	bool m_bSirenOrAlarm;
	uint8 m_nCarHornDelay;
	int8 m_comedyControlState;
	CStoredCollPoly m_aCollPolys[2];     // poly which is under front/rear part of car
	float m_fSteerRatio;
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
	virtual bool IsDoorReady(uint32 door) { return false; }
	virtual bool IsDoorMissing(uint32 door) { return false; }
	virtual bool IsOpenTopCar(void) { return false; }
	virtual void RemoveRefsToVehicle(CEntity *ent) {}
	virtual void BlowUpCar(CEntity *ent) {}
	virtual bool SetUpWheelColModel(CColModel *colModel) { return false; }
	virtual void BurstTyre(uint8 tyre, bool applyForces) {}
	virtual bool IsRoomForPedToLeaveCar(uint32 component, CVector *forcedDoorPos) { return false;}
	virtual bool IsClearToDriveAway(void);
	virtual float GetHeightAboveRoad(void);
	virtual void PlayCarHorn(void) {}
#ifdef COMPATIBLE_SAVES
	virtual void Save(uint8*& buf);
	virtual void Load(uint8*& buf);
#endif

	eVehicleAppearance GetVehicleAppearance(void);
	bool IsCar(void) { return m_vehType == VEHICLE_TYPE_CAR; }
	bool IsBoat(void) { return m_vehType == VEHICLE_TYPE_BOAT; }
	bool IsTrain(void) { return m_vehType == VEHICLE_TYPE_TRAIN; }
	bool IsHeli(void) { return m_vehType == VEHICLE_TYPE_HELI; }
	bool IsPlane(void) { return m_vehType == VEHICLE_TYPE_PLANE; }
	bool IsBike(void) { return m_vehType == VEHICLE_TYPE_BIKE; }

	void FlyingControl(eFlightModel flightModel);
	bool DoBladeCollision(CVector pos, CMatrix &matrix, int16 rotorType, float radius, float damageMult);
	bool BladeColSectorList(CPtrList &list, CColModel &rotorColModel, CMatrix &matrix, int16 rotorType, float damageMult);

	void ProcessWheel(CVector &wheelFwd, CVector &wheelRight, CVector &wheelContactSpeed, CVector &wheelContactPoint,
		int32 wheelsOnGround, float thrust, float brake, float adhesion, int8 wheelId, float *wheelSpeed, tWheelState *wheelState, uint16 wheelStatus);
	void ProcessBikeWheel(CVector &wheelFwd, CVector &wheelRight, CVector &wheelContactSpeed, CVector &wheelContactPoint,
		int32 wheelsOnGround, float thrust, float brake, float adhesion, float unk, int8 wheelId, float *wheelSpeed, tWheelState *wheelState, eBikeWheelSpecial special, uint16 wheelStatus);
	void ExtinguishCarFire(void);
	void ProcessDelayedExplosion(void);
	float ProcessWheelRotation(tWheelState state, const CVector &fwd, const CVector &speed, float radius);
	int FindTyreNearestPoint(float x, float y);
	bool IsLawEnforcementVehicle(void);
	void ChangeLawEnforcerState(uint8 enable);
	bool UsesSiren(uint32 id);
	bool IsVehicleNormal(void);
	bool CarHasRoof(void);
	bool IsUpsideDown(void);
	bool IsOnItsSide(void);
	bool CanBeDeleted(void);
	bool CanPedOpenLocks(CPed *ped);
	bool CanDoorsBeDamaged(void);
	bool CanPedEnterCar(void);
	bool CanPedExitCar(bool jumpExit);
	bool CanPedJumpOffBike(void);
	// do these two actually return something?
	CPed *SetUpDriver(void);
	CPed *SetupPassenger(int n);
	void SetDriver(CPed *driver);
	bool AddPassenger(CPed *passenger);
	bool AddPassenger(CPed *passenger, uint8 n);
	void RemovePassenger(CPed *passenger);
	void RemoveDriver(void);
	bool IsDriver(CPed *ped);
	bool IsDriver(int32 model);
	bool IsPassenger(CPed *ped);
	bool IsPassenger(int32 model);
	void UpdatePassengerList(void);
	void ProcessCarAlarm(void);
	bool IsSphereTouchingVehicle(float sx, float sy, float sz, float radius);
	bool ShufflePassengersToMakeSpace(void);
	void MakeNonDraggedPedsLeaveVehicle(CPed *ped1, CPed *ped2, CPlayerPed *&player, CCopPed *&cop);
	void InflictDamage(CEntity *damagedBy, eWeaponType weaponType, float damage, CVector pos = CVector(0.0f, 0.0f, 0.0f));
	void DoFixedMachineGuns(void);
	void FireFixedMachineGuns(void);
	void ActivateBomb(void);
	void ActivateBombWhenEntered(void);

	void SetComponentAtomicAlpha(RpAtomic *atomic, int32 alpha);
	void UpdateClumpAlpha(void);

	static void HeliDustGenerate(CEntity *heli, float radius, float ground, int rnd);
	void DoSunGlare(void);

	bool IsAlarmOn(void) { return m_nAlarmState != 0 && m_nAlarmState != -1; }
	CVehicleModelInfo* GetModelInfo() { return (CVehicleModelInfo*)CModelInfo::GetModelInfo(GetModelIndex()); }
	bool IsTaxi(void) { return GetModelIndex() == MI_TAXI || GetModelIndex() == MI_CABBIE || GetModelIndex() == MI_ZEBRA || GetModelIndex() == MI_KAUFMAN; }
	bool IsLimo(void) { return GetModelIndex() == MI_STRETCH || GetModelIndex() == MI_LOVEFIST; }
	bool IsRealHeli(void) { return !!(pHandling->Flags & HANDLING_IS_HELI); }

	static bool bWheelsOnlyCheat;
	static bool bAllDodosCheat;
	static bool bCheat3;
	static bool bCheat4;
	static bool bCheat5;
#ifdef ALT_DODO_CHEAT
	static bool bAltDodoCheat;
#endif
	static bool m_bDisableMouseSteering;
	static bool bDisableRemoteDetonation;
	static bool bDisableRemoteDetonationOnContact;
};

void DestroyVehicleAndDriverAndPassengers(CVehicle* pVehicle);
bool IsVehiclePointerValid(CVehicle* pVehicle);
