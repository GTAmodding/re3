#include "common.h"

#include "General.h"
#include "RwHelper.h"
#include "ModelIndices.h"
#include "Timer.h"
#include "Placeable.h"
#include "Entity.h"
#include "Object.h"
#include "ParticleObject.h"
#include "Lights.h"
#include "World.h"
#include "Camera.h"
#include "Glass.h"
#include "Clock.h"
#include "Weather.h"
#include "Timecycle.h"
#include "Bridge.h"
#include "TrafficLights.h"
#include "Coronas.h"
#include "PointLights.h"
#include "Shadows.h"
#include "Pickups.h"
#include "SpecialFX.h"
#include "References.h"
#include "TxdStore.h"
#include "Zones.h"
#include "Bones.h"
#include "Debug.h"
#include "Renderer.h"

int gBuildings;

CEntity::CEntity(void)
{
	m_type = ENTITY_TYPE_NOTHING;
	m_status = STATUS_ABANDONED;

	bUsesCollision = false;
	bCollisionProcessed = false;
	bIsStatic = false;
	bHasContacted = false;
	bPedPhysics = false;
	bIsStuck = false;
	bIsInSafePosition = false;
	bUseCollisionRecords = false;

	bWasPostponed = false;
	bExplosionProof = false;
	bIsVisible = true;
	bHasCollided = false;
	bRenderScorched = false;
	bHasBlip = false;
	bIsBIGBuilding = false;
	bRenderDamaged = false;

	bBulletProof = false;
	bFireProof = false;
	bCollisionProof = false;
	bMeleeProof = false;
	bOnlyDamagedByPlayer = false;
	bStreamingDontDelete = false;
	bZoneCulled = false;
	bZoneCulled2 = false;

	bRemoveFromWorld = false;
	bHasHitWall = false;
	bImBeingRendered = false;
	bTouchingWater = false;
	bIsSubway = false;
	bDrawLast = false;
	bNoBrightHeadLights = false;
	bDoNotRender = false;

	bDistanceFade = false;
	m_flagE2 = false;

	m_scanCode = 0;
	m_modelIndex = -1;
	m_rwObject = nil;
	m_randomSeed = CGeneral::GetRandomNumber();
	m_pFirstReference = nil;
}

CEntity::~CEntity(void)
{
	DeleteRwObject();
	ResolveReferences();
}

void
CEntity::GetBoundCentre(CVector &out)
{
	out = m_matrix * CModelInfo::GetModelInfo(m_modelIndex)->GetColModel()->boundingSphere.center;
};

bool
CEntity::GetIsTouching(CVector const &center, float radius)
{
	return sq(GetBoundRadius()+radius) > (GetBoundCentre()-center).MagnitudeSqr();
}

bool
CEntity::GetIsOnScreen(void)
{
	return TheCamera.IsSphereVisible(GetBoundCentre(), GetBoundRadius(),
		&TheCamera.GetCameraMatrix());
}

bool
CEntity::GetIsOnScreenComplex(void)
{
	RwV3d boundBox[8];

	if(TheCamera.IsPointVisible(GetBoundCentre(), &TheCamera.GetCameraMatrix()))
		return true;

	CRect rect = GetBoundRect();
	CColModel *colmodel = CModelInfo::GetModelInfo(m_modelIndex)->GetColModel();
	float z = GetPosition().z;
	float minz = z + colmodel->boundingBox.min.z;
	float maxz = z + colmodel->boundingBox.max.z;
	boundBox[0].x = rect.left;
	boundBox[0].y = rect.bottom;
	boundBox[0].z = minz;
	boundBox[1].x = rect.left;
	boundBox[1].y = rect.top;
	boundBox[1].z = minz;
	boundBox[2].x = rect.right;
	boundBox[2].y = rect.bottom;
	boundBox[2].z = minz;
	boundBox[3].x = rect.right;
	boundBox[3].y = rect.top;
	boundBox[3].z = minz;
	boundBox[4].x = rect.left;
	boundBox[4].y = rect.bottom;
	boundBox[4].z = maxz;
	boundBox[5].x = rect.left;
	boundBox[5].y = rect.top;
	boundBox[5].z = maxz;
	boundBox[6].x = rect.right;
	boundBox[6].y = rect.bottom;
	boundBox[6].z = maxz;
	boundBox[7].x = rect.right;
	boundBox[7].y = rect.top;
	boundBox[7].z = maxz;

	return TheCamera.IsBoxVisible(boundBox, &TheCamera.GetCameraMatrix());
}

void
CEntity::Add(void)
{
	int x, xstart, xmid, xend;
	int y, ystart, ymid, yend;
	CSector *s;
	CPtrList *list;

	CRect bounds = GetBoundRect();
	xstart = CWorld::GetSectorIndexX(bounds.left);
	xend   = CWorld::GetSectorIndexX(bounds.right);
	xmid   = CWorld::GetSectorIndexX((bounds.left + bounds.right)/2.0f);
	ystart = CWorld::GetSectorIndexY(bounds.top);
	yend   = CWorld::GetSectorIndexY(bounds.bottom);
	ymid   = CWorld::GetSectorIndexY((bounds.top + bounds.bottom)/2.0f);
	assert(xstart >= 0);
	assert(xend < NUMSECTORS_X);
	assert(ystart >= 0);
	assert(yend < NUMSECTORS_Y);

	for(y = ystart; y <= yend; y++)
		for(x = xstart; x <= xend; x++){
			s = CWorld::GetSector(x, y);
			if(x == xmid && y == ymid) switch(m_type){
			case ENTITY_TYPE_BUILDING:
				list = &s->m_lists[ENTITYLIST_BUILDINGS];
				break;
			case ENTITY_TYPE_VEHICLE:
				list = &s->m_lists[ENTITYLIST_VEHICLES];
				break;
			case ENTITY_TYPE_PED:
				list = &s->m_lists[ENTITYLIST_PEDS];
				break;
			case ENTITY_TYPE_OBJECT:
				list = &s->m_lists[ENTITYLIST_OBJECTS];
				break;
			case ENTITY_TYPE_DUMMY:
				list = &s->m_lists[ENTITYLIST_DUMMIES];
				break;
			}else switch(m_type){
			case ENTITY_TYPE_BUILDING:
				list = &s->m_lists[ENTITYLIST_BUILDINGS_OVERLAP];
				break;
			case ENTITY_TYPE_VEHICLE:
				list = &s->m_lists[ENTITYLIST_VEHICLES_OVERLAP];
				break;
			case ENTITY_TYPE_PED:
				list = &s->m_lists[ENTITYLIST_PEDS_OVERLAP];
				break;
			case ENTITY_TYPE_OBJECT:
				list = &s->m_lists[ENTITYLIST_OBJECTS_OVERLAP];
				break;
			case ENTITY_TYPE_DUMMY:
				list = &s->m_lists[ENTITYLIST_DUMMIES_OVERLAP];
				break;
			}
			list->InsertItem(this);
		}
}

void
CEntity::Remove(void)
{
	int x, xstart, xmid, xend;
	int y, ystart, ymid, yend;
	CSector *s;
	CPtrList *list;

	CRect bounds = GetBoundRect();
	xstart = CWorld::GetSectorIndexX(bounds.left);
	xend   = CWorld::GetSectorIndexX(bounds.right);
	xmid   = CWorld::GetSectorIndexX((bounds.left + bounds.right)/2.0f);
	ystart = CWorld::GetSectorIndexY(bounds.top);
	yend   = CWorld::GetSectorIndexY(bounds.bottom);
	ymid   = CWorld::GetSectorIndexY((bounds.top + bounds.bottom)/2.0f);
	assert(xstart >= 0);
	assert(xend < NUMSECTORS_X);
	assert(ystart >= 0);
	assert(yend < NUMSECTORS_Y);

	for(y = ystart; y <= yend; y++)
		for(x = xstart; x <= xend; x++){
			s = CWorld::GetSector(x, y);
			if(x == xmid && y == ymid) switch(m_type){
			case ENTITY_TYPE_BUILDING:
				list = &s->m_lists[ENTITYLIST_BUILDINGS];
				break;
			case ENTITY_TYPE_VEHICLE:
				list = &s->m_lists[ENTITYLIST_VEHICLES];
				break;
			case ENTITY_TYPE_PED:
				list = &s->m_lists[ENTITYLIST_PEDS];
				break;
			case ENTITY_TYPE_OBJECT:
				list = &s->m_lists[ENTITYLIST_OBJECTS];
				break;
			case ENTITY_TYPE_DUMMY:
				list = &s->m_lists[ENTITYLIST_DUMMIES];
				break;
			}else switch(m_type){
			case ENTITY_TYPE_BUILDING:
				list = &s->m_lists[ENTITYLIST_BUILDINGS_OVERLAP];
				break;
			case ENTITY_TYPE_VEHICLE:
				list = &s->m_lists[ENTITYLIST_VEHICLES_OVERLAP];
				break;
			case ENTITY_TYPE_PED:
				list = &s->m_lists[ENTITYLIST_PEDS_OVERLAP];
				break;
			case ENTITY_TYPE_OBJECT:
				list = &s->m_lists[ENTITYLIST_OBJECTS_OVERLAP];
				break;
			case ENTITY_TYPE_DUMMY:
				list = &s->m_lists[ENTITYLIST_DUMMIES_OVERLAP];
				break;
			}
			list->RemoveItem(this);
		}
}

void
CEntity::CreateRwObject(void)
{
	CBaseModelInfo *mi;

	mi = CModelInfo::GetModelInfo(m_modelIndex);
	m_rwObject = mi->CreateInstance();
	if(m_rwObject){
		if(IsBuilding())
			gBuildings++;
		if(RwObjectGetType(m_rwObject) == rpATOMIC)
			m_matrix.AttachRW(RwFrameGetMatrix(RpAtomicGetFrame((RpAtomic*)m_rwObject)), false);
		else if(RwObjectGetType(m_rwObject) == rpCLUMP)
			m_matrix.AttachRW(RwFrameGetMatrix(RpClumpGetFrame((RpClump*)m_rwObject)), false);
		mi->AddRef();
	}
}

void
CEntity::DeleteRwObject(void)
{
	RwFrame *f;

	m_matrix.Detach();
	if(m_rwObject){
		if(RwObjectGetType(m_rwObject) == rpATOMIC){
			f = RpAtomicGetFrame((RpAtomic*)m_rwObject);
			RpAtomicDestroy((RpAtomic*)m_rwObject);
			RwFrameDestroy(f);
		}else if(RwObjectGetType(m_rwObject) == rpCLUMP){
#ifdef PED_SKIN
			if(IsClumpSkinned((RpClump*)m_rwObject))
				RpClumpForAllAtomics((RpClump*)m_rwObject, AtomicRemoveAnimFromSkinCB, nil);
#endif
			RpClumpDestroy((RpClump*)m_rwObject);
		}
		m_rwObject = nil;
		CModelInfo::GetModelInfo(m_modelIndex)->RemoveRef();
		if(IsBuilding())
			gBuildings--;
	}
}

void
CEntity::UpdateRwFrame(void)
{
	if(m_rwObject) {
		if(RwObjectGetType(m_rwObject) == rpATOMIC)
			RwFrameUpdateObjects(RpAtomicGetFrame((RpAtomic *)m_rwObject));
		else if(RwObjectGetType(m_rwObject) == rpCLUMP)
			RwFrameUpdateObjects(RpClumpGetFrame((RpClump *)m_rwObject));
	}
}

void
CEntity::UpdateRwFrameInterpolated(void)
{
	if(m_rwObject){
		if(RwObjectGetType(m_rwObject) == rpATOMIC)
			RwFrameUpdateObjects(RpAtomicGetFrame((RpAtomic*)m_rwObject));
		else if(RwObjectGetType(m_rwObject) == rpCLUMP)
			RwFrameUpdateObjects(RpClumpGetFrame((RpClump*)m_rwObject));
	}
}

void
CEntity::SetupBigBuilding(void)
{
	CSimpleModelInfo *mi;

	mi = (CSimpleModelInfo*)CModelInfo::GetModelInfo(m_modelIndex);
	bIsBIGBuilding = true;
	bStreamingDontDelete = true;
	bUsesCollision = false;
	m_level = CTheZones::GetLevelFromPosition(&GetPosition());
	if(m_level == LEVEL_GENERIC){
		if(mi->GetTxdSlot() != CTxdStore::FindTxdSlot("generic")){
			mi->SetTexDictionary("generic");
			printf("%d:%s txd has been set to generic\n", m_modelIndex, mi->GetName());
		}
	}
	if(mi->m_lodDistances[0] > 2000.0f)
		m_level = LEVEL_GENERIC;
}

CRect
CEntity::GetBoundRect(void)
{
	CRect rect;
	CVector v;
	CColModel *col = CModelInfo::GetModelInfo(m_modelIndex)->GetColModel();

	rect.ContainPoint(m_matrix * col->boundingBox.min);
	rect.ContainPoint(m_matrix * col->boundingBox.max);

	v = col->boundingBox.min;
	v.x = col->boundingBox.max.x;
	rect.ContainPoint(m_matrix * v);

	v = col->boundingBox.max;
	v.x = col->boundingBox.min.x;
	rect.ContainPoint(m_matrix * v);

	return rect;
}

void
CEntity::PreRender(void)
{
	switch(m_type){
	case ENTITY_TYPE_BUILDING:
		if(GetModelIndex() == MI_RAILTRACKS){
			CShadows::StoreShadowForPole(this, 0.0f, -10.949f, 5.0f, 8.0f, 1.0f, 0);
			CShadows::StoreShadowForPole(this, 0.0f, 10.949f, 5.0f, 8.0f, 1.0f, 1);
		}else if(IsTreeModel(GetModelIndex())){
			CShadows::StoreShadowForTree(this);
			ModifyMatrixForTreeInWind();
		}else if(IsBannerModel(GetModelIndex())){
			ModifyMatrixForBannerInWind();
		}
		break;
	case ENTITY_TYPE_OBJECT:
		if(GetModelIndex() == MI_COLLECTABLE1){
			CPickups::DoCollectableEffects(this);
			GetMatrix().UpdateRWInterPolated();
			UpdateRwFrame();
		}else if(GetModelIndex() == MI_MONEY){
			CPickups::DoMoneyEffects(this);
			GetMatrix().UpdateRWInterPolated();
			UpdateRwFrame();
		}else if(GetModelIndex() == MI_NAUTICALMINE ||
		         GetModelIndex() == MI_CARMINE ||
		         GetModelIndex() == MI_BRIEFCASE){
			if(((CObject*)this)->bIsPickup){
				CPickups::DoMineEffects(this);
				GetMatrix().UpdateRWInterPolated();
				UpdateRwFrame();
			}
		}else if(IsPickupModel(GetModelIndex())){
			if(((CObject*)this)->bIsPickup){
				CPickups::DoPickUpEffects(this);
				GetMatrix().UpdateRWInterPolated();
				UpdateRwFrame();
			}else if(GetModelIndex() == MI_GRENADE){
				CMotionBlurStreaks::RegisterStreak((uintptr)this,
					100, 100, 100,
					GetPosition() - 0.07f*TheCamera.GetRight(),
					GetPosition() + 0.07f*TheCamera.GetRight());
			}else if(GetModelIndex() == MI_MOLOTOV){
				CMotionBlurStreaks::RegisterStreak((uintptr)this,
					0, 100, 0,
					GetPosition() - 0.07f*TheCamera.GetRight(),
					GetPosition() + 0.07f*TheCamera.GetRight());
			}
		}else if(GetModelIndex() == MI_MISSILE){
			CVector pos = GetPosition();
			float flicker = (CGeneral::GetRandomNumber() & 0xF)/(float)0x10;
			CShadows::StoreShadowToBeRendered(SHADOWTYPE_ADDITIVE,
				gpShadowExplosionTex, &pos,
				8.0f, 0.0f, 0.0f, -8.0f,
				255, 200.0f*flicker, 160.0f*flicker, 120.0f*flicker,
				20.0f, false, 1.0f);
			CPointLights::AddLight(CPointLights::LIGHT_POINT,
				pos, CVector(0.0f, 0.0f, 0.0f),
				8.0f,
				1.0f*flicker,
				0.8f*flicker,
				0.6f*flicker,
				CPointLights::FOG_NONE, true);
			CCoronas::RegisterCorona((uintptr)this,
				255.0f*flicker, 220.0f*flicker, 190.0f*flicker, 255,
				pos, 6.0f*flicker, 80.0f, gpCoronaTexture[CCoronas::TYPE_STAR],
				CCoronas::FLARE_NONE, CCoronas::REFLECTION_ON,
				CCoronas::LOSCHECK_OFF, CCoronas::STREAK_OFF, 0.0f);
		}else if(IsGlass(GetModelIndex())){
			PreRenderForGlassWindow();
		}
		// fall through
	case ENTITY_TYPE_DUMMY:
		if(GetModelIndex() == MI_TRAFFICLIGHTS){
			CTrafficLights::DisplayActualLight(this);
			CShadows::StoreShadowForPole(this, 2.957f, 0.147f, 0.0f, 16.0f, 0.4f, 0);
		}else if(GetModelIndex() == MI_SINGLESTREETLIGHTS1)
			CShadows::StoreShadowForPole(this, 0.744f, 0.0f, 0.0f, 16.0f, 0.4f, 0);
		else if(GetModelIndex() == MI_SINGLESTREETLIGHTS2)
			CShadows::StoreShadowForPole(this, 0.043f, 0.0f, 0.0f, 16.0f, 0.4f, 0);
		else if(GetModelIndex() == MI_SINGLESTREETLIGHTS3)
			CShadows::StoreShadowForPole(this, 1.143f, 0.145f, 0.0f, 16.0f, 0.4f, 0);
		else if(GetModelIndex() == MI_DOUBLESTREETLIGHTS)
			CShadows::StoreShadowForPole(this, 0.0f, -0.048f, 0.0f, 16.0f, 0.4f, 0);
		else if(GetModelIndex() == MI_STREETLAMP1 ||
		        GetModelIndex() == MI_STREETLAMP2)
			CShadows::StoreShadowForPole(this, 0.0f, 0.0f, 0.0f, 16.0f, 0.4f, 0);
		break;
	}

	if (CModelInfo::GetModelInfo(GetModelIndex())->GetNum2dEffects() != 0)
		ProcessLightsForEntity();
}

void
CEntity::PreRenderForGlassWindow(void)
{
	CGlass::AskForObjectToBeRenderedInGlass(this);
	bIsVisible = false;
}

void
CEntity::Render(void)
{
	if(m_rwObject){
		bImBeingRendered = true;
		if(RwObjectGetType(m_rwObject) == rpATOMIC)
			RpAtomicRender((RpAtomic*)m_rwObject);
		else
			RpClumpRender((RpClump*)m_rwObject);
		bImBeingRendered = false;
	}
}

bool
CEntity::SetupLighting(void)
{
	DeActivateDirectional();
	SetAmbientColours();
	return false;
}

void
CEntity::AttachToRwObject(RwObject *obj)
{
	m_rwObject = obj;
	if(m_rwObject){
		if(RwObjectGetType(m_rwObject) == rpATOMIC)
			m_matrix.Attach(RwFrameGetMatrix(RpAtomicGetFrame((RpAtomic*)m_rwObject)), false);
		else if(RwObjectGetType(m_rwObject) == rpCLUMP)
			m_matrix.Attach(RwFrameGetMatrix(RpClumpGetFrame((RpClump*)m_rwObject)), false);
		CModelInfo::GetModelInfo(m_modelIndex)->AddRef();
	}
}

void
CEntity::DetachFromRwObject(void)
{
	if(m_rwObject)
		CModelInfo::GetModelInfo(m_modelIndex)->RemoveRef();
	m_rwObject = nil;
	m_matrix.Detach();
}

void
CEntity::RegisterReference(CEntity **pent)
{
	if(IsBuilding())
		return;
	CReference *ref;
	// check if already registered
	for(ref = m_pFirstReference; ref; ref = ref->next)
		if(ref->pentity == pent)
			return;
	// have to allocate new reference
	ref = CReferences::pEmptyList;
	if(ref){
		CReferences::pEmptyList = ref->next;

		ref->pentity = pent;
		ref->next = m_pFirstReference;
		m_pFirstReference = ref;
		return;
	}
	return;
}

// Clear all references to this entity
void
CEntity::ResolveReferences(void)
{
	CReference *ref;
	// clear pointers to this entity
	for(ref = m_pFirstReference; ref; ref = ref->next)
		if(*ref->pentity == this)
			*ref->pentity = nil;
	// free list
	if(m_pFirstReference){
		for(ref = m_pFirstReference; ref->next; ref = ref->next)
			;
		ref->next = CReferences::pEmptyList;
		CReferences::pEmptyList = m_pFirstReference;
		m_pFirstReference = nil;
	}
}

// Free all references that no longer point to this entity
void
CEntity::PruneReferences(void)
{
	CReference *ref, *next, **lastnextp;
	lastnextp = &m_pFirstReference;
	for(ref = m_pFirstReference; ref; ref = next){
		next = ref->next;
		if(*ref->pentity == this)
			lastnextp = &ref->next;
		else{
			*lastnextp = ref->next;
			ref->next = CReferences::pEmptyList;
			CReferences::pEmptyList = ref;
		}
	}
}

#ifdef PED_SKIN
void
CEntity::UpdateRpHAnim(void)
{
	RpHAnimHierarchy *hier = GetAnimHierarchyFromSkinClump(GetClump());
	RpHAnimHierarchyUpdateMatrices(hier);

#if 0
	int i;
	char buf[256];
	if(this == (CEntity*)FindPlayerPed())
	for(i = 0; i < hier->numNodes; i++){
		RpHAnimStdInterpFrame *kf = (RpHAnimStdInterpFrame*)rpHANIMHIERARCHYGETINTERPFRAME(hier, i);
		sprintf(buf, "%6.3f %6.3f %6.3f %6.3f  %6.3f %6.3f %6.3f  %d %s",
			kf->q.imag.x, kf->q.imag.y, kf->q.imag.z, kf->q.real,
			kf->t.x, kf->t.y, kf->t.z,
			HIERNODEID(hier, i),
			ConvertBoneTag2BoneName(HIERNODEID(hier, i)));
		CDebug::PrintAt(buf, 10, 1+i*3);

		RwMatrix *m = &RpHAnimHierarchyGetMatrixArray(hier)[i];
		sprintf(buf, "%6.3f %6.3f %6.3f %6.3f",
			m->right.x, m->up.x, m->at.x, m->pos.x);
		CDebug::PrintAt(buf, 80, 1+i*3+0);
		sprintf(buf, "%6.3f %6.3f %6.3f %6.3f",
			m->right.y, m->up.y, m->at.y, m->pos.y);
		CDebug::PrintAt(buf, 80, 1+i*3+1);
		sprintf(buf, "%6.3f %6.3f %6.3f %6.3f",
			m->right.z, m->up.z, m->at.z, m->pos.z);
		CDebug::PrintAt(buf, 80, 1+i*3+2);
	}

	void RenderSkeleton(RpHAnimHierarchy *hier);
	RenderSkeleton(hier);
#endif
}
#endif

void
CEntity::AddSteamsFromGround(CVector *unused)
{
	int i, n;
	C2dEffect *effect;
	CVector pos;

	n = CModelInfo::GetModelInfo(GetModelIndex())->GetNum2dEffects();
	for(i = 0; i < n; i++){
		effect = CModelInfo::GetModelInfo(GetModelIndex())->Get2dEffect(i);
		if(effect->type != EFFECT_PARTICLE)
			continue;

		pos = GetMatrix() * effect->pos;
		switch(effect->particle.particleType){
		case 0:
			CParticleObject::AddObject(POBJECT_PAVEMENT_STEAM, pos, effect->particle.dir, effect->particle.scale, false);
			break;
		case 1:
			CParticleObject::AddObject(POBJECT_WALL_STEAM, pos, effect->particle.dir, effect->particle.scale, false);
			break;
		case 2:
			CParticleObject::AddObject(POBJECT_DRY_ICE, pos, effect->particle.scale, false);
			break;
		case 3:
			CParticleObject::AddObject(POBJECT_SMALL_FIRE, pos, effect->particle.dir, effect->particle.scale, false);
			break;
		case 4:
			CParticleObject::AddObject(POBJECT_DARK_SMOKE, pos, effect->particle.dir, effect->particle.scale, false);
			break;
		}
	}
}

void
CEntity::ProcessLightsForEntity(void)
{
	int i, n;
	C2dEffect *effect;
	CVector pos;
	bool lightOn, lightFlickering;
	uint32 flashTimer1, flashTimer2, flashTimer3;

	if(bRenderDamaged || !bIsVisible || GetUp().z < 0.96f)
		return;

	flashTimer1 = 0;
	flashTimer2 = 0;
	flashTimer3 = 0;

	n = CModelInfo::GetModelInfo(GetModelIndex())->GetNum2dEffects();
	for(i = 0; i < n; i++, flashTimer1 += 0x80, flashTimer2 += 0x100, flashTimer3 += 0x200){
		effect = CModelInfo::GetModelInfo(GetModelIndex())->Get2dEffect(i);

		if(effect->type != EFFECT_LIGHT)
			continue;

		pos = GetMatrix() * effect->pos;

		lightOn = false;
		lightFlickering = false;
		switch(effect->light.lightType){
		case LIGHT_ON:
			lightOn = true;
			break;
		case LIGHT_ON_NIGHT:
			if(CClock::GetHours() > 18 || CClock::GetHours() < 7)
				lightOn = true;
			break;
		case LIGHT_FLICKER:
			if((CTimer::GetTimeInMilliseconds() ^ m_randomSeed) & 0x60)
				lightOn = true;
			else
				lightFlickering = true;
			if((CTimer::GetTimeInMilliseconds()>>11 ^ m_randomSeed) & 3)
				lightOn = true;
			break;
		case LIGHT_FLICKER_NIGHT:
			if(CClock::GetHours() > 18 || CClock::GetHours() < 7 || CWeather::WetRoads > 0.5f){
				if((CTimer::GetTimeInMilliseconds() ^ m_randomSeed) & 0x60)
					lightOn = true;
				else
					lightFlickering = true;
				if((CTimer::GetTimeInMilliseconds()>>11 ^ m_randomSeed) & 3)
					lightOn = true;
			}
			break;
		case LIGHT_FLASH1:
			if((CTimer::GetTimeInMilliseconds() + flashTimer1) & 0x200)
				lightOn = true;
			break;
		case LIGHT_FLASH1_NIGHT:
			if(CClock::GetHours() > 18 || CClock::GetHours() < 7)
				if((CTimer::GetTimeInMilliseconds() + flashTimer1) & 0x200)
					lightOn = true;
			break;
		case LIGHT_FLASH2:
			if((CTimer::GetTimeInMilliseconds() + flashTimer2) & 0x400)
				lightOn = true;
			break;
		case LIGHT_FLASH2_NIGHT:
			if(CClock::GetHours() > 18 || CClock::GetHours() < 7)
				if((CTimer::GetTimeInMilliseconds() + flashTimer2) & 0x400)
					lightOn = true;
			break;
		case LIGHT_FLASH3:
			if((CTimer::GetTimeInMilliseconds() + flashTimer3) & 0x800)
				lightOn = true;
			break;
		case LIGHT_FLASH3_NIGHT:
			if(CClock::GetHours() > 18 || CClock::GetHours() < 7)
				if((CTimer::GetTimeInMilliseconds() + flashTimer3) & 0x800)
					lightOn = true;
			break;
		case LIGHT_RANDOM_FLICKER:
			if(m_randomSeed > 16)
				lightOn = true;
			else{
				if((CTimer::GetTimeInMilliseconds() ^ m_randomSeed*8) & 0x60)
					lightOn = true;
				else
					lightFlickering = true;
				if((CTimer::GetTimeInMilliseconds()>>11 ^ m_randomSeed*8) & 3)
					lightOn = true;
			}
			break;
		case LIGHT_RANDOM_FLICKER_NIGHT:
			if(CClock::GetHours() > 18 || CClock::GetHours() < 7){
				if(m_randomSeed > 16)
					lightOn = true;
				else{
					if((CTimer::GetTimeInMilliseconds() ^ m_randomSeed*8) & 0x60)
						lightOn = true;
					else
						lightFlickering = true;
					if((CTimer::GetTimeInMilliseconds()>>11 ^ m_randomSeed*8) & 3)
						lightOn = true;
				}
			}
			break;
		case LIGHT_BRIDGE_FLASH1:
			if(CBridge::ShouldLightsBeFlashing() && CTimer::GetTimeInMilliseconds() & 0x200)
				lightOn = true;
			break;
		case LIGHT_BRIDGE_FLASH2:
			if(CBridge::ShouldLightsBeFlashing() && (CTimer::GetTimeInMilliseconds() & 0x1FF) < 60)
				lightOn = true;
			break;
		}

		// Corona
		if(lightOn)
			CCoronas::RegisterCorona((uintptr)this + i,
				effect->col.r, effect->col.g, effect->col.b, 255,
				pos, effect->light.size, effect->light.dist,
				effect->light.corona, effect->light.flareType, effect->light.roadReflection,
				effect->light.flags&LIGHTFLAG_LOSCHECK, CCoronas::STREAK_OFF, 0.0f);
		else if(lightFlickering)
			CCoronas::RegisterCorona((uintptr)this + i,
				0, 0, 0, 255,
				pos, effect->light.size, effect->light.dist,
				effect->light.corona, effect->light.flareType, effect->light.roadReflection,
				effect->light.flags&LIGHTFLAG_LOSCHECK, CCoronas::STREAK_OFF, 0.0f);

		// Pointlight
		if(effect->light.flags & LIGHTFLAG_FOG_ALWAYS){
			CPointLights::AddLight(CPointLights::LIGHT_FOGONLY_ALWAYS,
				pos, CVector(0.0f, 0.0f, 0.0f),
				effect->light.range,
				effect->col.r/255.0f, effect->col.g/255.0f, effect->col.b/255.0f,
				CPointLights::FOG_ALWAYS, true);
		}else if(effect->light.flags & LIGHTFLAG_FOG_NORMAL && lightOn && effect->light.range == 0.0f){
			CPointLights::AddLight(CPointLights::LIGHT_FOGONLY,
				pos, CVector(0.0f, 0.0f, 0.0f),
				effect->light.range,
				effect->col.r/255.0f, effect->col.g/255.0f, effect->col.b/255.0f,
				CPointLights::FOG_NORMAL, true);
		}else if(lightOn && effect->light.range != 0.0f){
			if(effect->col.r == 0 && effect->col.g == 0 && effect->col.b == 0){
				CPointLights::AddLight(CPointLights::LIGHT_POINT,
					pos, CVector(0.0f, 0.0f, 0.0f),
					effect->light.range,
					0.0f, 0.0f, 0.0f,
					CPointLights::FOG_NONE, true);
			}else{
				CPointLights::AddLight(CPointLights::LIGHT_POINT,
					pos, CVector(0.0f, 0.0f, 0.0f),
					effect->light.range,
					effect->col.r*CTimeCycle::GetSpriteBrightness()/255.0f,
					effect->col.g*CTimeCycle::GetSpriteBrightness()/255.0f,
					effect->col.b*CTimeCycle::GetSpriteBrightness()/255.0f,
					// half-useless because LIGHTFLAG_FOG_ALWAYS can't be on
					(effect->light.flags & LIGHTFLAG_FOG) >> 1,
					true);
			}
		}

		// Light shadow
		if(effect->light.shadowSize != 0.0f){
			if(lightOn){
				CShadows::StoreStaticShadow((uintptr)this + i, SHADOWTYPE_ADDITIVE,
					effect->light.shadow, &pos,
					effect->light.shadowSize, 0.0f,
					0.0f, -effect->light.shadowSize,
					128,
					effect->col.r*CTimeCycle::GetSpriteBrightness()*effect->light.shadowIntensity/255.0f,
					effect->col.g*CTimeCycle::GetSpriteBrightness()*effect->light.shadowIntensity/255.0f,
					effect->col.b*CTimeCycle::GetSpriteBrightness()*effect->light.shadowIntensity/255.0f,
					15.0f, 1.0f, 40.0f, false, 0.0f);
			}else if(lightFlickering){
				CShadows::StoreStaticShadow((uintptr)this + i, SHADOWTYPE_ADDITIVE,
					effect->light.shadow, &pos,
					effect->light.shadowSize, 0.0f,
					0.0f, -effect->light.shadowSize,
					0, 0.0f, 0.0f, 0.0f,
					15.0f, 1.0f, 40.0f, false, 0.0f);
			}
		}
	}
}

float WindTabel[] = {
	1.0f, 0.5f, 0.2f, 0.7f, 0.4f, 1.0f, 0.5f, 0.3f,
	0.2f, 0.1f, 0.7f, 0.6f, 0.3f, 1.0f, 0.5f, 0.2f,
};

void
CEntity::ModifyMatrixForTreeInWind(void)
{
	uint16 t;
	float f;
	float strength, flutter;

	if(CTimer::GetIsPaused())
		return;

	CMatrix mat(GetMatrix().m_attachment);

	if(CWeather::Wind >= 0.5){
		t = m_randomSeed + 16*CTimer::GetTimeInMilliseconds();
		f = (t & 0xFFF)/(float)0x1000;
		flutter = f * WindTabel[(t>>12)+1 & 0xF] +
			(1.0f - f) * WindTabel[(t>>12) & 0xF] +
			1.0f;
		strength = CWeather::Wind < 0.8f ? 0.008f : 0.014f;
	}else if(CWeather::Wind >= 0.2){
		t = (uintptr)this + CTimer::GetTimeInMilliseconds();
		f = (t & 0xFFF)/(float)0x1000;
		flutter = Sin(f * 6.28f);
		strength = 0.008f;
	}else{
		t = (uintptr)this + CTimer::GetTimeInMilliseconds();
		f = (t & 0xFFF)/(float)0x1000;
		flutter = Sin(f * 6.28f);
		strength = 0.005f;
	}

	mat.GetUp().x = strength * flutter;
	mat.GetUp().y = mat.GetUp().x;

	mat.UpdateRW();
	UpdateRwFrame();
}

float BannerWindTabel[] = {
	0.0f, 0.3f, 0.6f, 0.85f, 0.99f, 0.97f, 0.65f, 0.15f,
	-0.1f, 0.0f, 0.35f, 0.57f, 0.55f, 0.35f, 0.45f, 0.67f,
	0.73f, 0.45f, 0.25f, 0.35f, 0.35f, 0.11f, 0.13f, 0.21f,
	0.28f, 0.28f, 0.22f, 0.1f, 0.0f, -0.1f, -0.17f, -0.12f
};

void
CEntity::ModifyMatrixForBannerInWind(void)
{
	uint16 t;
	float f;
	float strength, flutter;
	CVector right, up;

	if(CTimer::GetIsPaused())
		return;

	if(CWeather::Wind < 0.1f)
		strength = 0.2f;
	else if(CWeather::Wind < 0.8f)
		strength = 0.43f;
	else
		strength = 0.66f;

	t = ((int)(GetMatrix().GetPosition().x + GetMatrix().GetPosition().y) << 10) + 16*CTimer::GetTimeInMilliseconds();
	f = (t & 0x7FF)/(float)0x800;
	flutter = f * BannerWindTabel[(t>>11)+1 & 0x1F] +
		(1.0f - f) * BannerWindTabel[(t>>11) & 0x1F];
	flutter *= strength;

	right = CrossProduct(GetForward(), GetUp());
	right.z = 0.0f;
	right.Normalise();
	up = right * flutter;
	up.z = Sqrt(sq(1.0f) - sq(flutter));
	GetRight() = CrossProduct(GetForward(), up);
	GetUp() = up;

	GetMatrix().UpdateRW();
	UpdateRwFrame();
}

void 
CEntity::AddSteamsFromGround(CPtrList& list) 
{
	CPtrNode *pNode = list.first;
	while (pNode) {
		((CEntity*)pNode->item)->AddSteamsFromGround(nil);
		pNode = pNode->next;
	}
}

#ifdef COMPATIBLE_SAVES
void
CEntity::SaveEntityFlags(uint8*& buf)
{
	uint32 tmp = 0;
	tmp |= (m_type & (BIT(3) - 1));
	tmp |= (m_status & (BIT(5) - 1)) << 3;

	if (bUsesCollision) tmp |= BIT(8);
	if (bCollisionProcessed) tmp |= BIT(9);
	if (bIsStatic)  tmp |= BIT(10);
	if (bHasContacted) tmp |= BIT(11);
	if (bPedPhysics) tmp |= BIT(12);
	if (bIsStuck) tmp |= BIT(13);
	if (bIsInSafePosition) tmp |= BIT(14);
	if (bUseCollisionRecords) tmp |= BIT(15);

	if (bWasPostponed) tmp |= BIT(16);
	if (bExplosionProof) tmp |= BIT(17);
	if (bIsVisible)  tmp |= BIT(18);
	if (bHasCollided) tmp |= BIT(19);
	if (bRenderScorched) tmp |= BIT(20);
	if (bHasBlip) tmp |= BIT(21);
	if (bIsBIGBuilding) tmp |= BIT(22);
	if (bRenderDamaged) tmp |= BIT(23);

	if (bBulletProof) tmp |= BIT(24);
	if (bFireProof) tmp |= BIT(25);
	if (bCollisionProof)  tmp |= BIT(26);
	if (bMeleeProof) tmp |= BIT(27);
	if (bOnlyDamagedByPlayer) tmp |= BIT(28);
	if (bStreamingDontDelete) tmp |= BIT(29);
	if (bZoneCulled) tmp |= BIT(30);
	if (bZoneCulled2) tmp |= BIT(31);

	WriteSaveBuf<uint32>(buf, tmp);

	tmp = 0;

	if (bRemoveFromWorld) tmp |= BIT(0);
	if (bHasHitWall) tmp |= BIT(1);
	if (bImBeingRendered)  tmp |= BIT(2);
	if (bTouchingWater) tmp |= BIT(3);
	if (bIsSubway) tmp |= BIT(4);
	if (bDrawLast) tmp |= BIT(5);
	if (bNoBrightHeadLights) tmp |= BIT(6);
	if (bDoNotRender) tmp |= BIT(7);

	if (bDistanceFade) tmp |= BIT(8);
	if (m_flagE2) tmp |= BIT(9);

	WriteSaveBuf<uint32>(buf, tmp);
}

void
CEntity::LoadEntityFlags(uint8*& buf)
{
	uint32 tmp = ReadSaveBuf<uint32>(buf);
	m_type = (tmp & ((BIT(3) - 1)));
	m_status = ((tmp >> 3) & (BIT(5) - 1));

	bUsesCollision = !!(tmp & BIT(8));
	bCollisionProcessed = !!(tmp & BIT(9));
	bIsStatic = !!(tmp & BIT(10));
	bHasContacted = !!(tmp & BIT(11));
	bPedPhysics = !!(tmp & BIT(12));
	bIsStuck = !!(tmp & BIT(13));
	bIsInSafePosition = !!(tmp & BIT(14));
	bUseCollisionRecords = !!(tmp & BIT(15));

	bWasPostponed = !!(tmp & BIT(16));
	bExplosionProof = !!(tmp & BIT(17));
	bIsVisible = !!(tmp & BIT(18));
	bHasCollided = !!(tmp & BIT(19));
	bRenderScorched = !!(tmp & BIT(20));
	bHasBlip = !!(tmp & BIT(21));
	bIsBIGBuilding = !!(tmp & BIT(22));
	bRenderDamaged = !!(tmp & BIT(23));

	bBulletProof = !!(tmp & BIT(24));
	bFireProof = !!(tmp & BIT(25));
	bCollisionProof = !!(tmp & BIT(26));
	bMeleeProof = !!(tmp & BIT(27));
	bOnlyDamagedByPlayer = !!(tmp & BIT(28));
	bStreamingDontDelete = !!(tmp & BIT(29));
	bZoneCulled = !!(tmp & BIT(30));
	bZoneCulled2 = !!(tmp & BIT(31));

	tmp = ReadSaveBuf<uint32>(buf);

	bRemoveFromWorld = !!(tmp & BIT(0));
	bHasHitWall = !!(tmp & BIT(1));
	bImBeingRendered = !!(tmp & BIT(2));
	bTouchingWater = !!(tmp & BIT(3));
	bIsSubway = !!(tmp & BIT(4));
	bDrawLast = !!(tmp & BIT(5));
	bNoBrightHeadLights = !!(tmp & BIT(6));
	bDoNotRender = !!(tmp & BIT(7));

	bDistanceFade = !!(tmp & BIT(8));
	m_flagE2 = !!(tmp & BIT(9));
}

#endif
