#pragma once

#include "ModelInfo.h"
#include "Placeable.h"

struct CReference;
class CPtrList;

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
protected:
	uint32 m_type : 3;
private:
	uint32 m_status : 5;
public:
	// flagsA
	uint32 bUsesCollision : 1;			// does entity use collision
	uint32 bCollisionProcessed : 1;		// has object been processed by a ProcessEntityCollision function
	uint32 bIsStatic : 1;				// is entity static
	uint32 bHasContacted : 1;			// has entity processed some contact forces
	uint32 bPedPhysics : 1;
	uint32 bIsStuck : 1;				// is entity stuck
	uint32 bIsInSafePosition : 1;		// is entity in a collision free safe position
	uint32 bUseCollisionRecords : 1;

	// flagsB
	uint32 bWasPostponed : 1;			// was entity control processing postponed
	uint32 bExplosionProof : 1;
	uint32 bIsVisible : 1;				//is the entity visible
	uint32 bHasCollided : 1;
	uint32 bRenderScorched : 1;
	uint32 bHasBlip : 1;
	uint32 bIsBIGBuilding : 1;			// Set if this entity is a big building
	uint32 bRenderDamaged : 1;			// use damaged LOD models for objects with applicable damage

	// flagsC
	uint32 bBulletProof : 1;
	uint32 bFireProof : 1;
	uint32 bCollisionProof : 1;
	uint32 bMeleeProof : 1;
	uint32 bOnlyDamagedByPlayer : 1;
	uint32 bStreamingDontDelete : 1;	// Dont let the streaming remove this 
	uint32 bZoneCulled : 1;
	uint32 bZoneCulled2 : 1;    // only treadables+10m

	// flagsD
	uint32 bRemoveFromWorld : 1;		// remove this entity next time it should be processed
	uint32 bHasHitWall : 1;				// has collided with a building (changes subsequent collisions)
	uint32 bImBeingRendered : 1;		// don't delete me because I'm being rendered
	uint32 bTouchingWater : 1;	// used by cBuoyancy::ProcessBuoyancy
	uint32 bIsSubway : 1;	// set when subway, but maybe different meaning?
	uint32 bDrawLast : 1;				// draw object last
	uint32 bNoBrightHeadLights : 1;
	uint32 bDoNotRender : 1;

	// flagsE
	uint32 bDistanceFade : 1;			// Fade entity because it is far away
	uint32 m_flagE2 : 1;

	uint16 m_scanCode;
	uint16 m_randomSeed;
	int16 m_modelIndex;
	uint16 m_level;	// int16
	CReference *m_pFirstReference;

public:
	uint8 GetType() const { return m_type; }
	void SetType(uint8 type) { m_type = type; }
	uint8 GetStatus() const { return m_status; }
	void SetStatus(uint8 status) { m_status = status; }
	CColModel *GetColModel(void) { return CModelInfo::GetModelInfo(m_modelIndex)->GetColModel(); }
	bool GetIsStatic(void) const { return bIsStatic; }
	void SetIsStatic(bool state) { bIsStatic = state; }
#ifdef COMPATIBLE_SAVES
	void SaveEntityFlags(uint8*& buf);
	void LoadEntityFlags(uint8*& buf);
#else
	uint32* GetAddressOfEntityProperties() { /* AWFUL */ return (uint32*)((char*)&m_rwObject + sizeof(m_rwObject)); }
#endif

	CEntity(void);
	~CEntity(void);

	virtual void Add(void);
	virtual void Remove(void);
	virtual void SetModelIndex(uint32 id);
	virtual void SetModelIndexNoCreate(uint32 id);
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
	virtual void RemoveLighting(bool);
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
	CVector GetBoundCentre(void);
	float GetBoundRadius(void);
	float GetDistanceFromCentreOfMassToBaseOfModel(void);
	bool GetIsTouching(CVector const &center, float r);
	bool GetIsOnScreen(void);
	bool GetIsOnScreenComplex(void);
	bool IsVisible(void);
	bool IsVisibleComplex(void);
	int16 GetModelIndex(void) const { return m_modelIndex; }
	void UpdateRwFrame(void);
	void SetupBigBuilding(void);

	void AttachToRwObject(RwObject *obj);
	void DetachFromRwObject(void);

	void RegisterReference(CEntity **pent);
	void ResolveReferences(void);
	void PruneReferences(void);

#ifdef PED_SKIN
	void UpdateRpHAnim(void);
#endif

	void PreRenderForGlassWindow(void);
	void AddSteamsFromGround(CVector *unused);
	void ModifyMatrixForTreeInWind(void);
	void ModifyMatrixForBannerInWind(void);
	void ProcessLightsForEntity(void);
};

VALIDATE_SIZE(CEntity, 0x64);
