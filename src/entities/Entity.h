#pragma once

#include "ModelInfo.h"
#include "Placeable.h"

struct CReference;

enum eEntityType
{
	ENTITY_TYPE_NOTHING = 0,
	ENTITY_TYPE_BUILDING,
	ENTITY_TYPE_VEHICLE,
	ENTITY_TYPE_PED,
	ENTITY_TYPE_OBJECT,
	ENTITY_TYPE_DUMMY,
};

enum eEntityStatus
{
	STATUS_PLAYER,
	STATUS_PLAYER_PLAYBACKFROMBUFFER,
	STATUS_SIMPLE,
	STATUS_PHYSICS,
	STATUS_ABANDONED,
	STATUS_WRECKED,
	STATUS_TRAIN_MOVING,
	STATUS_TRAIN_NOT_MOVING,
	STATUS_HELI,
	STATUS_PLANE,
	STATUS_PLAYER_REMOTE,
	STATUS_PLAYER_DISABLED,
};

class CEntity : public CPlaceable
{
public:
	RwObject *m_rwObject;
	uint32 m_type : 3;
	uint32 m_status : 5;

	// flagsA
	uint32 bUsesCollision : 1;
	uint32 bCollisionProcessed : 1;
	uint32 bIsStatic : 1;
	uint32 bHasContacted : 1;
	uint32 bPedPhysics : 1;
	uint32 bIsStuck : 1;
	uint32 bIsInSafePosition : 1;
	uint32 bUseCollisionRecords : 1;

	// flagsB
	uint32 bWasPostponed : 1;
	uint32 bExplosionProof : 1;
	uint32 bIsVisible : 1;
	uint32 bHasCollided : 1;	//
	uint32 bRenderScorched : 1;
	uint32 bHasBlip : 1;
	uint32 bIsBIGBuilding : 1;
	// VC inserts one more flag here: if drawdist <= 2000
	uint32 bRenderDamaged : 1;

	// flagsC
	uint32 bBulletProof : 1;
	uint32 bFireProof : 1;
	uint32 bCollisionProof : 1;
	uint32 bMeleeProof : 1;
	uint32 bOnlyDamagedByPlayer : 1;
	uint32 bStreamingDontDelete : 1;
	uint32 bZoneCulled : 1;
	uint32 bZoneCulled2 : 1;	// only treadables+10m

	// flagsD
	uint32 bRemoveFromWorld : 1;
	uint32 bHasHitWall : 1;
	uint32 bImBeingRendered : 1;
	uint32 bTouchingWater : 1;	// used by cBuoyancy::ProcessBuoyancy
	uint32 bIsSubway : 1;	// set when subway, but maybe different meaning?
	uint32 bDrawLast : 1;
	uint32 bNoBrightHeadLights : 1;
	uint32 bDoNotRender : 1;

	// flagsE
	uint32 bDistanceFade : 1;
	uint32 m_flagE2 : 1;

	uint16 m_scanCode;
	uint16 m_randomSeed;
	int16 m_modelIndex;
	uint16 m_level;	// int16
	CReference *m_pFirstReference;

	CColModel *GetColModel(void) { return CModelInfo::GetModelInfo(m_modelIndex)->GetColModel(); }
	uint32* GetAddressOfEntityProperties() { /* AWFUL */ return (uint32*)((char*)&m_rwObject + sizeof(m_rwObject)); }

	CEntity(void);
	~CEntity(void);

	virtual void Add(void);
	virtual void Remove(void);
	virtual void SetModelIndex(uint32 id) { m_modelIndex = id; CreateRwObject(); }
	virtual void SetModelIndexNoCreate(uint32 id) { m_modelIndex = id; }
	virtual void CreateRwObject(void);
	virtual void DeleteRwObject(void);
	virtual CRect GetBoundRect(void);
	virtual void ProcessControl(void) {}
	virtual void ProcessCollision(void) {}
	virtual void ProcessShift(void) {}
	virtual void Teleport(CVector v) {}
	virtual void PreRender(void);
	virtual void Render(void);
	virtual bool SetupLighting(void);
	virtual void RemoveLighting(bool) {}
	virtual void FlagToDestroyWhenNextProcessed(void) {}

	bool IsBuilding(void) { return m_type == ENTITY_TYPE_BUILDING; }
	bool IsVehicle(void) { return m_type == ENTITY_TYPE_VEHICLE; }
	bool IsPed(void) { return m_type == ENTITY_TYPE_PED; }
	bool IsObject(void) { return m_type == ENTITY_TYPE_OBJECT; }
	bool IsDummy(void) { return m_type == ENTITY_TYPE_DUMMY; }

	RpAtomic *GetAtomic(void) {
		assert(RwObjectGetType(m_rwObject) == rpATOMIC);
		return (RpAtomic*)m_rwObject;
	}
	RpClump *GetClump(void) {
		assert(RwObjectGetType(m_rwObject) == rpCLUMP);
		return (RpClump*)m_rwObject;
	}

	void GetBoundCentre(CVector &out);
	CVector GetBoundCentre(void) { CVector v; GetBoundCentre(v); return v; }
	float GetBoundRadius(void) { return CModelInfo::GetModelInfo(m_modelIndex)->GetColModel()->boundingSphere.radius; }
	float GetDistanceFromCentreOfMassToBaseOfModel(void) { return -CModelInfo::GetModelInfo(m_modelIndex)->GetColModel()->boundingBox.min.z; }
	bool GetIsTouching(CVector const &center, float r);
	bool GetIsOnScreen(void);
	bool GetIsOnScreenComplex(void);
	bool IsVisible(void) { return m_rwObject && bIsVisible && GetIsOnScreen(); }
	bool IsVisibleComplex(void) { return m_rwObject && bIsVisible && GetIsOnScreenComplex(); }
	int GetModelIndex(void) { return m_modelIndex; }
	void UpdateRwFrame(void);
	void SetupBigBuilding(void);

	void AttachToRwObject(RwObject *obj);
	void DetachFromRwObject(void);

	void RegisterReference(CEntity **pent);
	void ResolveReferences(void);
	void PruneReferences(void);

	void PreRenderForGlassWindow(void);
	void AddSteamsFromGround(CVector *unused);
	void ModifyMatrixForTreeInWind(void);
	void ModifyMatrixForBannerInWind(void);
	void ProcessLightsForEntity(void);
};
static_assert(sizeof(CEntity) == 0x64, "CEntity: error");
