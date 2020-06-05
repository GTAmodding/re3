#pragma once

#include "ModelInfo.h"
#include "Placeable.h"

struct CReference;
class CPtrList;

enum eEntityType : uint8
{
	ENTITY_TYPE_NOTHING = 0,
	ENTITY_TYPE_BUILDING,
	ENTITY_TYPE_VEHICLE,
	ENTITY_TYPE_PED,
	ENTITY_TYPE_OBJECT,
	ENTITY_TYPE_DUMMY,
};

enum eEntityStatus : uint8
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
	STATUS_GHOST
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
	uint32 bStreamBIGBuilding : 1;	// set when draw dist <= 2000

	// flagsC
	uint32 bRenderDamaged : 1;			// use damaged LOD models for objects with applicable damage
	uint32 bBulletProof : 1;
	uint32 bFireProof : 1;
	uint32 bCollisionProof : 1;
	uint32 bMeleeProof : 1;
	uint32 bOnlyDamagedByPlayer : 1;
	uint32 bStreamingDontDelete : 1;	// Dont let the streaming remove this 
	uint32 bRemoveFromWorld : 1;		// remove this entity next time it should be processed

	// flagsD
	uint32 bHasHitWall : 1;				// has collided with a building (changes subsequent collisions)
	uint32 bImBeingRendered : 1;		// don't delete me because I'm being rendered
	uint32 bTouchingWater : 1;	// used by cBuoyancy::ProcessBuoyancy
	uint32 bIsSubway : 1;	// set when subway, but maybe different meaning?
	uint32 bDrawLast : 1;				// draw object last
	uint32 bNoBrightHeadLights : 1;
	uint32 bDoNotRender : 1;	//-- only applies to CObjects apparently
	uint32 bDistanceFade : 1;			// Fade entity because it is far away

	// flagsE
	uint32 m_flagE1 : 1;
	uint32 m_flagE2 : 1;
	uint32 bOffscreen : 1;               // offscreen flag. This can only be trusted when it is set to true
	uint32 bIsStaticWaitingForCollision : 1; // this is used by script created entities - they are static until the collision is loaded below them
	uint32 bDontStream : 1;              // tell the streaming not to stream me
	uint32 bUnderwater : 1;              // this object is underwater change drawing order
	uint32 bHasPreRenderEffects : 1; // Object has a prerender effects attached to it

	uint16 m_scanCode;
	uint16 m_randomSeed;
	int16 m_modelIndex;
	int8 m_level;
	int8 m_area;
	CReference *m_pFirstReference;

public:
	eEntityType GetType() const { return (eEntityType)m_type; }
	void SetType(eEntityType type) { m_type = type; }
	eEntityStatus GetStatus() const { return (eEntityStatus)m_status; }
	void SetStatus(eEntityStatus status) { m_status = status; }
	CColModel *GetColModel(void) { return CModelInfo::GetModelInfo(m_modelIndex)->GetColModel(); }
	bool IsStatic(void) { return bIsStatic || bIsStaticWaitingForCollision; }
#ifdef COMPATIBLE_SAVES
	void SaveEntityFlags(uint8*& buf);
	void LoadEntityFlags(uint8*& buf);
#else
	uint32* GetAddressOfEntityProperties() { /* AWFUL */ return (uint32*)((char*)&m_rwObject + sizeof(m_rwObject)); }
#endif

	CEntity(void);
	virtual ~CEntity(void);

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
// TODO(MIAMI):
	bool IsEntityOccluded(void) { return false; }
	int16 GetModelIndex(void) const { return m_modelIndex; }
	void UpdateRwFrame(void);
	void SetupBigBuilding(void);
	bool HasPreRenderEffects(void);

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

	static void AddSteamsFromGround(CPtrList& list);
};

bool IsEntityPointerValid(CEntity*);
