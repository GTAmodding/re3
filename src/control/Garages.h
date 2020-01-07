#pragma once
#include "Automobile.h"
#include "audio_enums.h"
#include "config.h"

class CVehicle;

enum eGarageState : int8
{
	GS_FULLYCLOSED,
	GS_OPENED,
	GS_CLOSING,
	GS_OPENING,
	GS_OPENEDCONTAINSCAR,
	GS_CLOSEDCONTAINSCAR,
	GS_AFTERDROPOFF,
};

enum eGarageType : int8
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
};

static_assert(sizeof(CStoredCar) == 0x28, "CStoredCar");

class CGarage
{
	eGarageType m_eGarageType;
	eGarageState m_eGarageState;
	char field_2;
	char m_bClosingWithoutTargetCar;
	char m_bDeactivated;
	char m_bResprayHappened;
	char field_6;
	char field_7;
	int m_nTargetModelIndex;
	CEntity *m_pDoor1;
	CEntity *m_pDoor2;
	char m_bDoor1PoolIndex;
	char m_bDoor2PoolIndex;
	char m_bIsDoor1Object;
	char m_bIsDoor2Object;
	char field_24;
	char m_bRotatedDoor;
	char m_bCameraFollowsPlayer;
	char field_27;
	CVector m_vecInf;
	CVector m_vecSup;
	float m_fDoorPos;
	float m_fDoorHeight;
	float m_fDoor1X;
	float m_fDoor1Y;
	float m_fDoor2X;
	float m_fDoor2Y;
	float m_fDoor1Z;
	float m_fDoor2Z;
	int m_nDoorOpenTime;
	char m_bCollectedCarsState;
	char field_89;
	char field_90;
	char field_91;
	CVehicle *m_pTarget;
	int field_96;
	CStoredCar m_sStoredCar;
public:
	void OpenThisGarage();
	void CloseThisGarage();
};

static_assert(sizeof(CGarage) == 140, "CGarage");

class CGarages
{
public:
	static int32 &BankVansCollected;
	static bool &BombsAreFree;
	static bool &RespraysAreFree;
	static int32 &CarsCollected;
	static int32 &CarTypesCollected;
	static int32 &CrushedCarId;
	static uint32 &LastTimeHelpMessage;
	static int32 &MessageNumberInString;
	static const char *MessageIDString;
	static int32 &MessageNumberInString2;
	static uint32 &MessageStartTime;
	static uint32 &MessageEndTime;
	static uint32 &NumGarages;
	static bool &PlayerInGarage;
	static int32 &PoliceCarsCollected;
	static uint32 &GarageToBeTidied;
	static CGarage(&Garages)[NUM_GARAGES];

public:
	static bool IsModelIndexADoor(uint32 id);
	static void TriggerMessage(const char *text, int16, uint16 time, int16);
	static void PrintMessages(void);
	static bool HasCarBeenCrushed(int32);
	static bool IsPointWithinHideOutGarage(CVector&);
	static bool IsPointWithinAnyGarage(CVector&);
	static void PlayerArrestedOrDied();
	static void Init(void);
	static void Update(void);
	static int16 AddOne(float, float, float, float, float, float, uint8, uint32);
	static void SetTargetCarForMissonGarage(int16, CVehicle*);
	static bool HasCarBeenDroppedOffYet(int16);
	static void ActivateGarage(int16);
	static void DeActivateGarage(int16);
	static int32 QueryCarsCollected(int16);
	static bool HasThisCarBeenCollected(int16, uint8);
	static void ChangeGarageType(int16, eGarageType);
	static bool HasResprayHappened(int16);
	static void GivePlayerDetonator();
};
