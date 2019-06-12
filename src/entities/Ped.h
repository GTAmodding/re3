#pragma once

#include "Physical.h"
#include "Weapon.h"
#include "PathFind.h"

enum {
	PED_MAX_WEAPONS = 13
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

	PED_UNKNOWN,	// HANG_OUT in Fire_Head's idb

	PED_STATES_NO_AI,
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
};

enum {
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
	uint8 m_ped_flagA1 : 1;
	uint8 m_ped_flagA2 : 1;
	uint8 m_ped_flagA4 : 1;
	uint8 m_ped_flagA8 : 1;
	uint8 m_ped_flagA10 : 1;
	uint8 m_ped_flagA20 : 1;
	uint8 m_ped_flagA40 : 1;
	uint8 m_ped_flagA80 : 1;
	uint8 m_ped_flagB1 : 1;
	uint8 m_ped_flagB2 : 1;
	uint8 m_ped_flagB4 : 1;
	uint8 m_ped_flagB8 : 1;
	uint8 m_ped_flagB10 : 1;
	uint8 m_ped_flagB20 : 1;
	uint8 m_ped_flagB40 : 1;
	uint8 m_ped_flagB80 : 1;
	uint8 m_ped_flagC1 : 1;
	uint8 m_ped_flagC2 : 1;
	uint8 m_ped_flagC4 : 1;
	uint8 m_ped_flagC8 : 1;
	uint8 m_ped_flagC10 : 1;
	uint8 m_ped_flagC20 : 1;
	uint8 m_ped_flagC40 : 1;
	uint8 m_ped_flagC80 : 1;
	uint8 m_ped_flagD1 : 1;
	uint8 m_ped_flagD2 : 1;
	uint8 m_ped_flagD4 : 1;
	uint8 m_ped_flagD8 : 1;
	uint8 m_ped_flagD10 : 1;
	uint8 m_ped_flagD20 : 1;
	uint8 m_ped_flagD40 : 1;
	uint8 m_ped_flagD80 : 1;
	uint8 m_ped_flagE1 : 1;
	uint8 m_ped_flagE2 : 1;
	uint8 m_ped_flagE4 : 1;
	uint8 m_ped_flagE8 : 1;
	uint8 m_ped_flagE10 : 1;
	uint8 m_ped_flagE20 : 1;
	uint8 m_ped_flagE40 : 1;
	uint8 m_ped_flagE80 : 1;
	uint8 m_ped_flagF1 : 1;
	uint8 m_ped_flagF2 : 1;
	uint8 m_ped_flagF4 : 1;
	uint8 m_ped_flagF8 : 1;
	uint8 m_ped_flagF10 : 1;
	uint8 m_ped_flagF20 : 1;
	uint8 m_ped_flagF40 : 1;
	uint8 m_ped_flagF80 : 1;
	uint8 m_ped_flagG1 : 1;
	uint8 m_ped_flagG2 : 1;
	uint8 m_ped_flagG4 : 1;
	uint8 m_ped_flagG8 : 1;
	uint8 m_ped_flagG10 : 1;
	uint8 m_ped_flagG20 : 1;
	uint8 m_ped_flagG40 : 1;
	uint8 m_ped_flagG80 : 1;
	uint8 m_ped_flagH1 : 1;
	uint8 m_ped_flagH2 : 1;
	uint8 m_ped_flagH4 : 1;
	uint8 m_ped_flagH8 : 1;
	uint8 m_ped_flagH10 : 1;
	uint8 m_ped_flagH20 : 1;
	uint8 m_ped_flagH40 : 1;
	uint8 m_ped_flagH80 : 1;
	uint8 m_ped_flagI1 : 1;
	uint8 m_ped_flagI2 : 1;
	uint8 m_ped_flagI4 : 1;
	uint8 m_ped_flagI8 : 1;
	uint8 m_ped_flagI10 : 1;
	uint8 m_ped_flagI20 : 1;
	uint8 m_ped_flagI40 : 1;
	uint8 m_ped_flagI80 : 1;
	uint8 stuff1[199];
	int32 m_nPedState;
	int32 m_nLastPedState;
	int32 m_nMoveState;
	int m_nStoredActionState;
	int m_nPrevActionState;
	int m_nWaitState;
	int m_nWaitTimer;
private:
	uint32 stuff0[28];
public:
	unsigned short m_nPathNodes;
	short m_nCurPathNode;
	char m_nPathState;
private:
	char _pad2B5[3];
public:
	CPathNode *m_pNextPathNode;
	CPathNode *m_pLastPathNode;
	float m_fHealth;
	float m_fArmour;
	uint8 stuff2[34];
	CEntity *m_pCurrentPhysSurface;
	CVector m_vecOffsetFromPhysSurface;
	CEntity *m_pCurSurface;
	uint8 stuff3[16];
	CVehicle *m_pMyVehicle;
	bool bInVehicle;
	uint8 stuff4[23];
	int32 m_nPedType;

	uint8 stuff5[28];
	CEntity *m_pCollidingEntity;
	uint8 stuff6[12];
	CWeapon m_weapons[PED_MAX_WEAPONS];
	int32 stuff7;
	uint8 m_currentWeapon;
	uint8 stuff[163];

	static void *operator new(size_t);
	static void operator delete(void*, size_t);

	bool IsPlayer(void) { return m_nPedType == 0 || m_nPedType== 1 || m_nPedType == 2 || m_nPedType == 3; }
	bool UseGroundColModel(void);
	void KillPedWithCar(CVehicle *veh, float impulse);
	CWeapon *GetWeapon(void) { return &m_weapons[m_currentWeapon]; }
};
static_assert(offsetof(CPed, m_nPedState) == 0x224, "CPed: error");
static_assert(offsetof(CPed, m_pCurSurface) == 0x2FC, "CPed: error");
static_assert(offsetof(CPed, m_pMyVehicle) == 0x310, "CPed: error");
static_assert(offsetof(CPed, m_nPedType) == 0x32C, "CPed: error");
static_assert(offsetof(CPed, m_pCollidingEntity) == 0x34C, "CPed: error");
static_assert(offsetof(CPed, m_weapons) == 0x35C, "CPed: error");
static_assert(offsetof(CPed, m_currentWeapon) == 0x498, "CPed: error");
static_assert(sizeof(CPed) == 0x53C, "CPed: error");
