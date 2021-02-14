#pragma once

#include "Vehicle.h"
#include "DamageManager.h"
#include "Door.h"

class CObject;

enum eCarNodes
{
	CAR_WHEEL_RF = 1,
	CAR_WHEEL_RM,
	CAR_WHEEL_RB,
	CAR_WHEEL_LF,
	CAR_WHEEL_LM,
	CAR_WHEEL_LB,
	CAR_BUMP_FRONT,
	CAR_BUMP_REAR,
	CAR_WING_RF,
	CAR_WING_RR,
	CAR_DOOR_RF,
	CAR_DOOR_RR,
	CAR_WING_LF,
	CAR_WING_LR,
	CAR_DOOR_LF,
	CAR_DOOR_LR,
	CAR_BONNET,
	CAR_BOOT,
	CAR_WINDSCREEN,
	NUM_CAR_NODES,
};

enum {
	CARWHEEL_FRONT_LEFT,
	CARWHEEL_REAR_LEFT,
	CARWHEEL_FRONT_RIGHT,
	CARWHEEL_REAR_RIGHT
};

enum eBombType
{
	CARBOMB_NONE,
	CARBOMB_TIMED,
	CARBOMB_ONIGNITION,
	CARBOMB_REMOTE,
	CARBOMB_TIMEDACTIVE,
	CARBOMB_ONIGNITIONACTIVE,
};

enum {
	CAR_DOOR_FLAG_UNKNOWN = 0x0,
	CAR_DOOR_FLAG_LF = 0x1,
	CAR_DOOR_FLAG_LR = 0x2,
	CAR_DOOR_FLAG_RF = 0x4,
	CAR_DOOR_FLAG_RR = 0x8
};

class CAutomobile : public CVehicle
{
public:
	// 0x288
	CDamageManager Damage;
	CDoor Doors[6];
	RwFrame *m_aCarNodes[NUM_CAR_NODES];
	CColPoint m_aWheelColPoints[4];
	float m_aSuspensionSpringRatio[4];
	float m_aSuspensionSpringRatioPrev[4];
	float m_aWheelTimer[4];		// set to 4.0 when wheel is touching ground, then decremented
	float m_auto_unused1;
	bool m_aWheelSkidmarkMuddy[4];
	bool m_aWheelSkidmarkBloody[4];
	float m_aWheelRotation[4];
	float m_aWheelPosition[4];
	float m_aWheelSpeed[4];
	uint8 m_auto_unused2;
	uint8 m_bombType : 3;
	uint8 bTaxiLight : 1;
	uint8 bDriverLastFrame  : 1;		// for bombs
	uint8 bFixedColour : 1;
	uint8 bBigWheels : 1;
	uint8 bWaterTight : 1;	// no damage for non-player peds
	uint8 bNotDamagedUpsideDown : 1;
	uint8 bMoreResistantToDamage : 1;
	CEntity *m_pBombRigger;
	int16 m_auto_unk1;
	uint16 m_hydraulicState;
	uint32 m_nBusDoorTimerEnd;
	uint32 m_nBusDoorTimerStart;
	float m_aSuspensionSpringLength[4];
	float m_aSuspensionLineLength[4];
	float m_fHeightAboveRoad;
	float m_fTraction;
	float m_fVelocityChangeForAudio;
	float m_randomValues[6];	// used for what?
	float m_fFireBlowUpTimer;
	CPhysical *m_aGroundPhysical[4];	// physicals touching wheels
	CVector m_aGroundOffset[4];		// from ground object to colpoint
	CEntity *m_pSetOnFireEntity;
	float m_weaponDoorTimerLeft;	// still don't know what exactly this is
	float m_weaponDoorTimerRight;
	float m_fCarGunLR;
	float m_fCarGunUD;
	float m_fPropellerRotation;
	uint8 stuff4[4];
	uint8 m_nWheelsOnGround;
	uint8 m_nDriveWheelsOnGround;
	uint8 m_nDriveWheelsOnGroundPrev;
	float m_fGasPedalAudio;
	tWheelState m_aWheelState[4];

	static bool m_sAllTaxiLights;

	CAutomobile(int32 id, uint8 CreatedBy);

	// from CEntity
	void SetModelIndex(uint32 id);
	void ProcessControl(void);
	void Teleport(CVector v);
	void PreRender(void);
	void Render(void);

	// from CPhysical
	int32 ProcessEntityCollision(CEntity *ent, CColPoint *colpoints);

	// from CVehicle
	void ProcessControlInputs(uint8);
	void GetComponentWorldPosition(int32 component, CVector &pos);
	bool IsComponentPresent(int32 component);
	void SetComponentRotation(int32 component, CVector rotation);
	void OpenDoor(int32 component, eDoors door, float openRatio);
	void ProcessOpenDoor(uint32, uint32, float);
	bool IsDoorReady(eDoors door);
	bool IsDoorFullyOpen(eDoors door);
	bool IsDoorClosed(eDoors door);
	bool IsDoorMissing(eDoors door);
	void RemoveRefsToVehicle(CEntity *ent);
	void BlowUpCar(CEntity *ent);
	bool SetUpWheelColModel(CColModel *colModel);
	void BurstTyre(uint8 tyre);
	bool IsRoomForPedToLeaveCar(uint32 component, CVector *doorOffset);
	float GetHeightAboveRoad(void);
	void PlayCarHorn(void);

	void FireTruckControl(void);
	void TankControl(void);
	void HydraulicControl(void);
	void VehicleDamage(float impulse, uint16 damagedPiece);
	void ProcessBuoyancy(void);
	void DoDriveByShootings(void);
	int32 RcbanditCheckHitWheels(void);
	int32 RcbanditCheck1CarWheels(CPtrList &list);
	void PlaceOnRoadProperly(void);
	void dmgDrawCarCollidingParticles(const CVector &pos, float amount);
	void AddDamagedVehicleParticles(void);
	int32 AddWheelDirtAndWater(CColPoint *colpoint, uint32 belowEffectSpeed);
	void PlayHornIfNecessary(void);
	void ResetSuspension(void);
	void SetupSuspensionLines(void);
	void ScanForCrimes(void);
	void BlowUpCarsInPath(void);
	bool HasCarStoppedBecauseOfLight(void);
	void SetBusDoorTimer(uint32 timer, uint8 type);
	void ProcessAutoBusDoors(void);
	void ProcessSwingingDoor(int32 component, eDoors door);
	void SetupDamageAfterLoad(void);
	CObject *SpawnFlyingComponent(int32 component, uint32 type);
	CObject *RemoveBonnetInPedCollision(void);
	void SetPanelDamage(int32 component, ePanels panel, bool noFlyingComponents = false);
	void SetBumperDamage(int32 component, ePanels panel, bool noFlyingComponents = false);
	void SetDoorDamage(int32 component, eDoors door, bool noFlyingComponents = false);

	void Fix(void);
	void SetComponentVisibility(RwFrame *frame, uint32 flags);
	void SetupModelNodes(void);
	void SetTaxiLight(bool light);
	bool GetAllWheelsOffGround(void);
	void HideAllComps(void);
	void ShowAllComps(void);
	void ReduceHornCounter(void);
#ifdef COMPATIBLE_SAVES
	virtual void Save(uint8*& buf);
	virtual void Load(uint8*& buf);
#endif
	static const uint32 nSaveStructSize;

	static void SetAllTaxiLights(bool set);
};

VALIDATE_SIZE(CAutomobile, 0x5A8);

inline uint8 GetCarDoorFlag(int32 carnode) {
	switch (carnode) {
	case CAR_DOOR_LF:
		return CAR_DOOR_FLAG_LF;
	case CAR_DOOR_LR:
		return CAR_DOOR_FLAG_LR;
	case CAR_DOOR_RF:
		return CAR_DOOR_FLAG_RF;
	case CAR_DOOR_RR:
		return CAR_DOOR_FLAG_RR;
	default:
		return CAR_DOOR_FLAG_UNKNOWN;
	}
}
