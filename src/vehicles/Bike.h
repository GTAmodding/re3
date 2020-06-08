#pragma once

#include "Vehicle.h"
#include "Skidmarks.h"

enum eBikeNodes {
	BIKE_NODE_NONE,
	BIKE_CHASSIS,
	BIKE_FORKS_FRONT,
	BIKE_FORKS_REAR,
	BIKE_WHEEL_FRONT,
	BIKE_WHEEL_REAR,
	BIKE_MUDGUARD,
	BIKE_HANDLEBARS,
	BIKE_NUM_NODES
};

enum {
	BIKEWHEEL_FRONT,
	BIKEWHEEL_REAR,
};

enum {
	BIKESUSP_F1,
	BIKESUSP_F2,
	BIKESUSP_R1,
	BIKESUSP_R2,
};

class CBike : public CVehicle
{
public:
	RwFrame *m_aBikeNodes[BIKE_NUM_NODES];
	bool bLeanMatrixClean;
	CMatrix m_leanMatrix;
	CVector m_vecAvgSurfaceNormal;
	CVector m_vecAvgSurfaceRight;
	tBikeHandlingData *pBikeHandling;
	AssocGroupId m_bikeAnimType;
	uint8 m_wheelStatus[2];
	CColPoint m_aWheelColPoints[4];
	float m_aSuspensionSpringRatio[4];
	float m_aSuspensionSpringRatioPrev[4];
	float m_aWheelTimer[4];
	float m_bike_unused1;
	eSkidmarkType m_aWheelSkidmarkType[2];
	bool m_aWheelSkidmarkBloody[2];
	bool m_aWheelSkidmarkUnk[2];
	float m_aWheelRotation[2];
	float m_aWheelSpeed[2];
	float m_aWheelPosition[2];
	float m_aWheelBasePosition[2];
	float m_aSuspensionSpringLength[4];
	float m_aSuspensionLineLength[4];
	float m_fHeightAboveRoad;
	float m_fTraction;
	float m_fRearForkLength;
	float m_fFrontForkY;
	float m_fFrontForkZ;
	float m_fFrontForkSlope;
	float m_fWheelAngle;
	float m_fLeanLRAngle;
	float m_fLeanLRAngle2;
	float m_fLeanInput;
	float m_fPedLeanAmountLR;
	float m_fPedLeanAmountUD;
	uint8 m_bike_unused2;
	uint8 unused[3];	// looks like padding..but for what?
	uint8 m_bike_flag01 : 1;
	uint8 m_bike_flag02 : 1;
	uint8 bWaterTight : 1;
	uint8 bIsBeingPickedUp : 1;
	uint8 bIsStanding : 1;
	uint8 bExtraSpeed : 1;	// leaning forward
	uint8 bIsOnFire : 1;
	uint8 m_bike_flag80 : 1;
	int16 m_doingBurnout;
	float m_fTireTemperature;
	float m_fBrakeDestabilization;
	uint32 field_490;
	float m_fFireBlowUpTimer;
	CPhysical *m_aGroundPhysical[4];
	CVector m_aGroundOffset[4];
	CEntity *m_pSetOnFireEntity;
	uint8 m_nWheelsOnGround;
	uint8 m_nDriveWheelsOnGround;
	uint8 m_nDriveWheelsOnGroundPrev;
	float m_fGasPedalAudio;
	tWheelState m_aWheelState[2];

	CBike(int32 id, uint8 CreatedBy);

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
	bool IsDoorReady(eDoors door);
	bool IsDoorFullyOpen(eDoors door);
	bool IsDoorClosed(eDoors door);
	bool IsDoorMissing(eDoors door);
	void RemoveRefsToVehicle(CEntity *ent);
	void BlowUpCar(CEntity *ent);
	bool SetUpWheelColModel(CColModel *colModel);
	void BurstTyre(uint8 tyre, bool applyForces);
	bool IsRoomForPedToLeaveCar(uint32 component, CVector *doorOffset);
	float GetHeightAboveRoad(void);
	void PlayCarHorn(void);

	void KnockOffRider(eWeaponType weapon, uint8 direction, CPed *ped, bool bGetBackOn);
	void VehicleDamage(void);
	void ProcessBuoyancy(void);
	void DoDriveByShootings(void);
	void AddDamagedVehicleParticles(void);
	int32 AddWheelDirtAndWater(CColPoint *colpoint, uint32 belowEffectSpeed);
	void PlayHornIfNecessary(void);
	void ResetSuspension(void);
	void SetupSuspensionLines(void);
	void CalculateLeanMatrix(void);
	void GetCorrectedWorldDoorPosition(CVector &pos, CVector p1, CVector p2);

	void Fix(void);
	void SetupModelNodes(void);
	void ReduceHornCounter(void);
};
