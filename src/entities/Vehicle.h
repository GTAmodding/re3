#pragma once

#include "Physical.h"

class CPed;
class CFire;
struct tHandlingData;

enum {
	GETTING_IN_OUT_FL = 1,
	GETTING_IN_OUT_RL = 2,
	GETTING_IN_OUT_FR = 4,
	GETTING_IN_OUT_RR = 8
};

enum eCarLock : uint8 {
	CARLOCK_NOT_USED,
	CARLOCK_UNLOCKED,
	CARLOCK_LOCKED,
	CARLOCK_LOCKOUT_PLAYER_ONLY,
	CARLOCK_LOCKED_PLAYER_INSIDE,
	CARLOCK_COP_CAR,
	CARLOCK_FORCE_SHUT_DOORS,
	CARLOCK_SKIP_SHUT_DOORS
};

class CVehicle : public CPhysical
{
public:
	// 0x128
	tHandlingData *m_handling;
	uint8 stuff1[112];
	uint8 m_currentColour1;
	uint8 m_currentColour2;
	uint8 m_anExtras[2];
	int16 m_nAlarmState; // m_nWantedStarsOnEnter on DK22
	int16 m_nMissionValue;
	CPed *pDriver;
	CPed *pPassengers[8];
	uint8 m_nNumPassengers;
	int8 m_nNumGettingIn;
	int8 m_nGettingInFlags;
	int8 m_nGettingOutFlags;
	uint8 m_nNumMaxPassengers;
	char field_1CD[19];
	CEntity *m_pCurSurface;
	CFire *m_pCarFire;
	float m_fSteerAngle;
	float m_fGasPedal;
	float m_fBreakPedal;
	uint8 m_nCreatedBy;        // eVehicleCreatedBy
	uint8 bIsLawEnforcer : 1;
	uint8 bIsAmbulanceOnDuty : 1;
	uint8 bIsFiretruckOnDuty : 1;
	uint8 m_veh_flagA8 : 1;
	uint8 m_veh_flagA10 : 1;
	uint8 m_veh_flagA20 : 1;
	uint8 m_veh_flagA40 : 1;
	uint8 m_veh_flagA80 : 1;
	uint8 bIsVan : 1;
	uint8 bIsBus : 1;
	uint8 bIsBig : 1;
	uint8 bIsLow : 1;
	uint8 m_veh_flagB10 : 1;
	uint8 m_veh_flagB20 : 1;
	uint8 m_veh_flagB40 : 1;
	uint8 m_veh_flagB80 : 1;
	uint8 m_veh_flagC1 : 1;
	uint8 m_veh_flagC2 : 1;
	uint8 m_veh_flagC4 : 1;
	uint8 m_veh_flagC8 : 1;
	uint8 m_veh_flagC10 : 1;
	uint8 m_veh_flagC20 : 1;
	uint8 m_veh_flagC40 : 1;
	uint8 m_veh_flagC80 : 1;
	uint8 m_veh_flagD1 : 1;
	uint8 m_veh_flagD2 : 1;
	uint8 m_veh_flagD4 : 1;
	uint8 m_veh_flagD8 : 1;
	uint8 bRecordedForReplay : 1;
	uint8 m_veh_flagD20 : 1;
	uint8 m_veh_flagD40 : 1;
	uint8 m_veh_flagD80 : 1;
	int8 field_1F9;
	uint8 m_nAmmoInClip[1];    // Used to make the guns on boat do a reload (20 by default)
	int8 field_1FB;
	int8 field_1FC[4];
	float m_fHealth;           // 1000.0f = full health. 0 -> explode
	uint8 m_nCurrentGear;
	int8 field_205[3];
	int field_208;
	uint32 m_nGunFiringTime;    // last time when gun on vehicle was fired (used on boats)
	uint32 m_nTimeOfDeath;
	int16 field_214;
	int16 m_nBombTimer;        // goes down with each frame
	CPed *m_pWhoDetonatedMe;
	float field_21C;
	float field_220;
	eCarLock m_nDoorLock;
	int8 m_nLastWeaponDamage; // see eWeaponType, -1 if no damage
	int8 m_nRadioStation;
	int8 field_22A;
	int8 field_22B;
	uint8 m_nCarHornTimer;
	int8 field_22D;
	uint8 m_nSirenOrAlarm;
	int8 field_22F;
	CStoredCollPoly m_frontCollPoly;     // poly which is under front part of car
	CStoredCollPoly m_rearCollPoly;      // poly which is under rear part of car
	float m_fSteerRatio;
	eVehicleType m_vehType;

	static void *operator new(size_t);
	static void *operator new(size_t sz, int slot);
	static void operator delete(void*, size_t);
	static void operator delete(void*, int);

	~CVehicle(void);

	void dtor(void) { this->CVehicle::~CVehicle(); }

	bool IsCar(void) { return m_vehType == VEHICLE_TYPE_CAR; }
	bool IsBoat(void) { return m_vehType == VEHICLE_TYPE_BOAT; }
	bool IsTrain(void) { return m_vehType == VEHICLE_TYPE_TRAIN; }
	bool IsHeli(void) { return m_vehType == VEHICLE_TYPE_HELI; }
	bool IsPlane(void) { return m_vehType == VEHICLE_TYPE_PLANE; }
	bool IsLawEnforcementVehicle(void);
	void ChangeLawEnforcerState(bool enable);
	void RemovePassenger(CPed *);
	void RemoveDriver(void);
	bool IsUpsideDown(void);
	
	static bool &bWheelsOnlyCheat;
	static bool &bAllDodosCheat;
	static bool &bCheat3;
	static bool &bCheat4;
	static bool &bCheat5;
	static bool &m_bDisableMouseSteering;
};

static_assert(sizeof(CVehicle) == 0x288, "CVehicle: error");
static_assert(offsetof(CVehicle, m_pCurSurface) == 0x1E0, "CVehicle: error");
static_assert(offsetof(CVehicle, m_nAlarmState) == 0x1A0, "CVehicle: error");
