#pragma once
#include "audio_enums.h"
#include "Camera.h"
#include "config.h"
#include "Lists.h"

class CVehicle;

enum eGarageState
{
	GS_FULLYCLOSED,
	GS_OPENED,
	GS_CLOSING,
	GS_OPENING,
	GS_OPENEDCONTAINSCAR,
	GS_CLOSEDCONTAINSCAR,
	GS_AFTERDROPOFF,
};

enum eGarageType
{
	GARAGE_NONE,
	GARAGE_MISSION,
	GARAGE_BOMBSHOP1,
	GARAGE_BOMBSHOP2,
	GARAGE_BOMBSHOP3,
	GARAGE_RESPRAY,
	GARAGE_COLLECTORSITEMS,
	GARAGE_COLLECTSPECIFICCARS,
	GARAGE_COLLECTCARS_1,
	GARAGE_COLLECTCARS_2,
	GARAGE_COLLECTCARS_3,
	GARAGE_FORCARTOCOMEOUTOF,
	GARAGE_60SECONDS,
	GARAGE_CRUSHER,
	GARAGE_MISSION_KEEPCAR,
	GARAGE_FOR_SCRIPT_TO_OPEN,
	GARAGE_HIDEOUT_ONE,
	GARAGE_HIDEOUT_TWO,
	GARAGE_HIDEOUT_THREE,
	GARAGE_FOR_SCRIPT_TO_OPEN_AND_CLOSE,
	GARAGE_KEEPS_OPENING_FOR_SPECIFIC_CAR,
	GARAGE_MISSION_KEEPCAR_REMAINCLOSED,
};

enum
{
	TOTAL_COLLECTCARS_GARAGES = GARAGE_COLLECTCARS_3 - GARAGE_COLLECTCARS_1 + 1,
	TOTAL_COLLECTCARS_CARS = 16
};

class CStoredCar
{
	int32 m_nModelIndex;
	CVector m_vecPos;
	CVector m_vecAngle;
	int32 m_bBulletproof : 1;
	int32 m_bFireproof : 1;
	int32 m_bExplosionproof : 1;
	int32 m_bCollisionproof : 1;
	int32 m_bMeleeproof : 1;
	int8 m_nPrimaryColor;
	int8 m_nSecondaryColor;
	int8 m_nRadioStation;
	int8 m_nVariationA;
	int8 m_nVariationB;
	int8 m_nCarBombType;
public:
	void Init() { m_nModelIndex = 0; }
	void Clear() { m_nModelIndex = 0; }
	bool HasCar() { return m_nModelIndex != 0; }
	const CStoredCar &operator=(const CStoredCar& other);
	void StoreCar(CVehicle*);
	CVehicle* RestoreCar();
};

VALIDATE_SIZE(CStoredCar, 0x28);

#define SWITCH_GARAGE_DISTANCE_CLOSE 40.0f

class CGarage
{
public:
	uint8 m_eGarageType;
	uint8 m_eGarageState;
	bool field_2; // unused
	bool m_bClosingWithoutTargetCar;
	bool m_bDeactivated;
	bool m_bResprayHappened;
	int m_nTargetModelIndex;
	CEntity *m_pDoor1;
	CEntity *m_pDoor2;
	uint8 m_bDoor1PoolIndex;
	uint8 m_bDoor2PoolIndex;
	bool m_bDoor1IsDummy;
	bool m_bDoor2IsDummy;
	bool m_bRecreateDoorOnNextRefresh;
	bool m_bRotatedDoor;
	bool m_bCameraFollowsPlayer;
	float m_fX1;
	float m_fX2;
	float m_fY1;
	float m_fY2;
	float m_fZ1;
	float m_fZ2;
	float m_fDoorPos;
	float m_fDoorHeight;
	float m_fDoor1X;
	float m_fDoor1Y;
	float m_fDoor2X;
	float m_fDoor2Y;
	float m_fDoor1Z;
	float m_fDoor2Z;
	uint32 m_nTimeToStartAction;
	uint8 m_bCollectedCarsState;
	CVehicle *m_pTarget;
	void* field_96; // unused
	CStoredCar m_sStoredCar; // not needed

	void OpenThisGarage();
	void CloseThisGarage();
	bool IsOpen() { return m_eGarageState == GS_OPENED || m_eGarageState == GS_OPENEDCONTAINSCAR; }
	bool IsClosed() { return m_eGarageState == GS_FULLYCLOSED; }
	bool IsUsed() { return m_eGarageType != GARAGE_NONE; }
	void Update();
	float GetGarageCenterX() { return (m_fX1 + m_fX2) / 2; }
	float GetGarageCenterY() { return (m_fY1 + m_fY2) / 2; }
	bool IsFar()
	{ 
#ifdef FIX_BUGS
		return Abs(TheCamera.GetPosition().x - GetGarageCenterX()) > SWITCH_GARAGE_DISTANCE_CLOSE ||
			Abs(TheCamera.GetPosition().y - GetGarageCenterY()) > SWITCH_GARAGE_DISTANCE_CLOSE;
#else
		return Abs(TheCamera.GetPosition().x - m_fX1) > SWITCH_GARAGE_DISTANCE_CLOSE || 
			Abs(TheCamera.GetPosition().y - m_fY1) > SWITCH_GARAGE_DISTANCE_CLOSE;
#endif
	}
	void TidyUpGarageClose();
	void TidyUpGarage();
	void RefreshDoorPointers(bool);
	void UpdateCrusherAngle();
	void UpdateDoorsHeight();
	bool IsEntityEntirelyInside3D(CEntity*, float);
	bool IsEntityEntirelyOutside(CEntity*, float);
	bool IsEntityEntirelyInside(CEntity*);
	float CalcDistToGarageRectangleSquared(float, float);
	float CalcSmallestDistToGarageDoorSquared(float, float);
	bool IsAnyOtherCarTouchingGarage(CVehicle* pException);
	bool IsStaticPlayerCarEntirelyInside();
	bool IsPlayerOutsideGarage();
	bool IsAnyCarBlockingDoor();
	void CenterCarInGarage(CVehicle*);
	bool DoesCraigNeedThisCar(int32);
	bool MarkThisCarAsCollectedForCraig(int32);
	bool HasCraigCollectedThisCar(int32);
	bool IsGarageEmpty();
	void UpdateCrusherShake(float, float);
	int32 CountCarsWithCenterPointWithinGarage(CEntity* pException);
	void RemoveCarsBlockingDoorNotInside();
	void StoreAndRemoveCarsForThisHideout(CStoredCar*, int32);
	bool RestoreCarsForThisHideout(CStoredCar*);
	bool IsEntityTouching3D(CEntity*);
	bool EntityHasASphereWayOutsideGarage(CEntity*, float);
	bool IsAnyOtherPedTouchingGarage(CPed* pException);
	void BuildRotatedDoorMatrix(CEntity*, float);
	void FindDoorsEntities();
	void FindDoorsEntitiesSectorList(CPtrList&, bool);
	void PlayerArrestedOrDied();
	bool Does60SecondsNeedThisCarAtAll(int mi);
	bool Does60SecondsNeedThisCar(int mi);
	void MarkThisCarAsCollectedFor60Seconds(int mi);
	bool IsPlayerEntirelyInsideGarage();

};

VALIDATE_SIZE(CGarage, 140);

class CGarages
{
	enum {
		MESSAGE_LENGTH = 8
	};
public:
	static int32 BankVansCollected;
	static bool BombsAreFree;
	static bool RespraysAreFree;
	static int32 CarsCollected;
	static int32 CarTypesCollected[TOTAL_COLLECTCARS_GARAGES];
	static int32 CrushedCarId;
	static uint32 LastTimeHelpMessage;
	static int32 MessageNumberInString;
	static char MessageIDString[MESSAGE_LENGTH];
	static int32 MessageNumberInString2;
	static uint32 MessageStartTime;
	static uint32 MessageEndTime;
	static uint32 NumGarages;
	static bool PlayerInGarage;
	static int32 PoliceCarsCollected;
	static CGarage aGarages[NUM_GARAGES];
	static CStoredCar aCarsInSafeHouse1[NUM_GARAGE_STORED_CARS];
	static CStoredCar aCarsInSafeHouse2[NUM_GARAGE_STORED_CARS];
	static CStoredCar aCarsInSafeHouse3[NUM_GARAGE_STORED_CARS];
	static bool bCamShouldBeOutisde;

	static void Init(void);
#ifndef PS2
	static void Shutdown(void);
#endif
	static void Update(void);

	static int16 AddOne(CVector pos1, CVector pos2, uint8 type, int32 targetId);
	static void ChangeGarageType(int16, uint8, int32);
	static void PrintMessages(void);
	static void TriggerMessage(const char* text, int16, uint16 time, int16);
	static void SetTargetCarForMissonGarage(int16, CVehicle*);
	static bool HasCarBeenDroppedOffYet(int16);
	static void DeActivateGarage(int16);
	static void ActivateGarage(int16);
	static int32 QueryCarsCollected(int16);
	static bool HasImportExportGarageCollectedThisCar(int16, int8);
	static bool IsGarageOpen(int16);
	static bool IsGarageClosed(int16);
	static bool HasThisCarBeenCollected(int16, uint8);
	static void OpenGarage(int16 garage) { aGarages[garage].OpenThisGarage(); }
	static void CloseGarage(int16 garage) { aGarages[garage].CloseThisGarage(); }
	static bool HasResprayHappened(int16);
	static void SetGarageDoorToRotate(int16);
	static void SetLeaveCameraForThisGarage(int16);
	static bool IsThisCarWithinGarageArea(int16, CEntity*);
	static bool HasCarBeenCrushed(int32);
	static bool IsPointInAGarageCameraZone(CVector);
	static bool CameraShouldBeOutside(void);
	static void GivePlayerDetonator(void);
	static void PlayerArrestedOrDied(void);
	static bool IsPointWithinHideOutGarage(Const CVector&);
	static bool IsPointWithinAnyGarage(Const CVector&);
	static void SetAllDoorsBackToOriginalHeight(void);
	static void Save(uint8* buf, uint32* size);
	static void Load(uint8* buf, uint32 size);
	static bool IsModelIndexADoor(uint32 id);
	static void SetFreeBombs(bool bValue) { BombsAreFree = bValue; }
	static void SetFreeResprays(bool bValue) { RespraysAreFree = bValue; }
	static void StopCarFromBlowingUp(CAutomobile*);

	static bool IsCarSprayable(CVehicle*);
	static float FindDoorHeightForMI(int32);
	static void CloseHideOutGaragesBeforeSave(void);
	static int32 CountCarsInHideoutGarage(uint8);
	static int32 FindMaxNumStoredCarsForGarage(uint8);
	static int32 GetBombTypeForGarageType(uint8 type) { return type - GARAGE_BOMBSHOP1 + 1; }
	static int32 GetCarsCollectedIndexForGarageType(uint8 type) { return type - GARAGE_COLLECTCARS_1; }

};
