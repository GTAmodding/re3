#pragma once

#include "Physical.h"

enum {
	UNKNOWN_OBJECT = 0,
	GAME_OBJECT = 1,
	MISSION_OBJECT = 2,
	TEMP_OBJECT = 3,
	CUTSCENE_OBJECT = 4,
};

enum CollisionSpecialResponseCase
{
	COLLRESPONSE_NONE,
	COLLRESPONSE_LAMPOST,
	COLLRESPONSE_SMALLBOX,
	COLLRESPONSE_BIGBOX,
	COLLRESPONSE_FENCEPART,
	COLLRESPONSE_UNKNOWN5
};

enum CollisionDamageEffect
{
	DAMAGE_EFFECT_NONE,
	DAMAGE_EFFECT_CHANGE_MODEL,
	DAMAGE_EFFECT_SPLIT_MODEL,
	DAMAGE_EFFECT_SMASH_AND_DAMAGE_TRAFFICLIGHTS,

	DAMAGE_EFFECT_SMASH_COMPLETELY = 20,
	DAMAGE_EFFECT_CHANGE_THEN_SMASH,

	DAMAGE_EFFECT_SMASH_CARDBOARD_COMPLETELY = 50,
	DAMAGE_EFFECT_SMASH_YELLOW_TARGET_COMPLETELY,

	DAMAGE_EFFECT_SMASH_WOODENBOX_COMPLETELY = 60,
	DAMAGE_EFFECT_SMASH_TRAFFICCONE_COMPLETELY = 70,
	DAMAGE_EFFECT_SMASH_BARPOST_COMPLETELY = 80,

	DAMAGE_EFFECT_SMASH_NEWSTANDNEW1 = 91,
	DAMAGE_EFFECT_SMASH_NEWSTANDNEW21,
	DAMAGE_EFFECT_SMASH_NEWSTANDNEW31,
	DAMAGE_EFFECT_SMASH_NEWSTANDNEW41,
	DAMAGE_EFFECT_SMASH_NEWSTANDNEW51,

	DAMAGE_EFFECT_SMASH_BLACKBAG = 100,
	DAMAGE_EFFECT_SMASH_VEGPALM = 110,
	DAMAGE_EFFECT_BURST_BEACHBALL = 120,
	DAMAGE_EFFECT_SMASH_BEACHLOUNGE_WOOD = 131,
	DAMAGE_EFFECT_SMASH_BEACHLOUNGE_TOWEL,
};

class CVehicle;
class CDummyObject;

class CObject : public CPhysical
{
public:
	CMatrix m_objectMatrix;
	float m_fUprootLimit;
	int8 ObjectCreatedBy;
	int8 bIsPickup : 1;
	int8 bPickupObjWithMessage : 1;
	int8 bOutOfStock : 1;
	int8 bGlassCracked : 1;
	int8 bGlassBroken : 1;
	int8 bHasBeenDamaged : 1;
	int8 bUseVehicleColours : 1;
	int8 m_nBonusValue; 
	float m_fCollisionDamageMultiplier;
	uint8 m_nCollisionDamageEffect;
	uint8 m_nSpecialCollisionResponseCases;
	bool m_bCameraToAvoidThisObject;

	// this batch is unused
	int8 field_17B;
	int8 field_17C;
	int8 field_17D;
	int8 field_17E;
	int8 field_17F;

	uint32 m_nEndOfLifeTime;
	int16 m_nRefModelIndex;
	CEntity *m_pCurSurface;
	CEntity *m_pCollidingEntity;
	int8 m_colour1, m_colour2;

	static int16 nNoTempObjects;
//	static int16 nBodyCastHealth;

	static float fDistToNearestTree;

	static void *operator new(size_t);
	static void *operator new(size_t, int);
	static void operator delete(void*, size_t);
	static void operator delete(void*, int);

	CObject(void);
	CObject(int32, bool);
	CObject(CDummyObject*);
	~CObject(void);

	void ProcessControl(void);
	void Teleport(CVector vecPos);
	void Render(void);
	bool SetupLighting(void);
	void RemoveLighting(bool reset);

	void ObjectDamage(float amount);
	void RefModelInfo(int32 modelId);
	void Init(void);
	bool CanBeDeleted(void);

	static void DeleteAllMissionObjects();
	static void DeleteAllTempObjects();
	static void DeleteAllTempObjectsInArea(CVector point, float fRadius);
};

bool IsObjectPointerValid(CObject* pObject);
