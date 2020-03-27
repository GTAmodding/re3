#include "common.h"
#include "patcher.h"
#include "main.h"
#include "Lights.h"
#include "ModelInfo.h"
#include "Treadable.h"
#include "Ped.h"
#include "Vehicle.h"
#include "Heli.h"
#include "Object.h"
#include "PathFind.h"
#include "Collision.h"
#include "VisibilityPlugins.h"
#include "Clock.h"
#include "World.h"
#include "Camera.h"
#include "ModelIndices.h"
#include "Streaming.h"
#include "Shadows.h"
#include "PointLights.h"
#include "Renderer.h"

bool gbShowPedRoadGroups;
bool gbShowCarRoadGroups;
bool gbShowCollisionPolys;
bool gbShowCollisionLines;
bool gbShowCullZoneDebugStuff;
bool gbBigWhiteDebugLightSwitchedOn;

bool gbDontRenderBuildings;
bool gbDontRenderBigBuildings;
bool gbDontRenderPeds;
bool gbDontRenderObjects;
bool gbDontRenderVehicles;

struct EntityInfo
{
	CEntity *ent;
	float sort;
};

CLinkList<EntityInfo> &gSortedVehiclesAndPeds = *(CLinkList<EntityInfo>*)0x629AC0;

int32 &CRenderer::ms_nNoOfVisibleEntities = *(int32*)0x940730;
CEntity *(&CRenderer::ms_aVisibleEntityPtrs)[NUMVISIBLEENTITIES] = *(CEntity * (*)[NUMVISIBLEENTITIES]) * (uintptr*)0x6E9920;
CEntity *(&CRenderer::ms_aInVisibleEntityPtrs)[NUMINVISIBLEENTITIES] = *(CEntity * (*)[NUMINVISIBLEENTITIES]) * (uintptr*)0x880B50;
int32 &CRenderer::ms_nNoOfInVisibleEntities = *(int32*)0x8F1B78;

CVector &CRenderer::ms_vecCameraPosition = *(CVector*)0x8E2C3C;
CVehicle *&CRenderer::m_pFirstPersonVehicle = *(CVehicle**)0x885B80;
bool &CRenderer::m_loadingPriority = *(bool*)0x95CD86;

void
CRenderer::Init(void)
{
	gSortedVehiclesAndPeds.Init(40);
	SortBIGBuildings();
}

void
CRenderer::Shutdown(void)
{
	gSortedVehiclesAndPeds.Shutdown();
}

void
CRenderer::PreRender(void)
{
	int i;
	CLink<CVisibilityPlugins::AlphaObjectInfo> *node;

	for(i = 0; i < ms_nNoOfVisibleEntities; i++)
		ms_aVisibleEntityPtrs[i]->PreRender();

	for(i = 0; i < ms_nNoOfInVisibleEntities; i++)
		ms_aInVisibleEntityPtrs[i]->PreRender();

	for(node = CVisibilityPlugins::m_alphaEntityList.head.next;
	    node != &CVisibilityPlugins::m_alphaEntityList.tail;
	    node = node->next)
		((CEntity*)node->item.entity)->PreRender();

	CHeli::SpecialHeliPreRender();
	CShadows::RenderExtraPlayerShadows();
}

void
CRenderer::RenderOneRoad(CEntity *e)
{
	if(gbDontRenderBuildings)
		return;
	if(gbShowCollisionPolys)
		CCollision::DrawColModel_Coloured(e->GetMatrix(),
			*CModelInfo::GetModelInfo(e->m_modelIndex)->GetColModel(),
			e->m_modelIndex);
	else
		e->Render();
}

void
CRenderer::RenderOneNonRoad(CEntity *e)
{
	CPed *ped;
	CVehicle *veh;
	int i;
	bool resetLights;

#ifndef MASTER
	if(gbShowCollisionPolys){
		if(!e->IsVehicle()){
			CCollision::DrawColModel_Coloured(e->GetMatrix(),
				*CModelInfo::GetModelInfo(e->m_modelIndex)->GetColModel(),
				e->m_modelIndex);
			return;
		}
	}else if(e->IsBuilding()){
		if(e->bIsBIGBuilding){
			if(gbDontRenderBigBuildings)
				return;
		}else{
			if(gbDontRenderBuildings)
				return;
		}
	}else
#endif
	if(e->IsPed()){
#ifndef MASTER
		if(gbDontRenderPeds)
			return;
#endif
		ped = (CPed*)e;
		if(ped->m_nPedState == PED_DRIVING)
			return;
	}
#ifndef MASTER
	else if(e->IsObject() || e->IsDummy()){
		if(gbDontRenderObjects)
			return;
	}else if(e->IsVehicle()){
		// re3 addition
		if(gbDontRenderVehicles)
			return;
	}
#endif

	resetLights = e->SetupLighting();

	if(e->IsVehicle())
		CVisibilityPlugins::InitAlphaAtomicList();

	// Render Peds in vehicle before vehicle itself
	if(e->IsVehicle()){
		veh = (CVehicle*)e;
		if(veh->pDriver && veh->pDriver->m_nPedState == PED_DRIVING)
			veh->pDriver->Render();
		for(i = 0; i < 8; i++)
			if(veh->pPassengers[i] && veh->pPassengers[i]->m_nPedState == PED_DRIVING)
				veh->pPassengers[i]->Render();
	}
	e->Render();

	if(e->IsVehicle()){
		e->bImBeingRendered = true;
		CVisibilityPlugins::RenderAlphaAtomics();
		e->bImBeingRendered = false;
	}

	e->RemoveLighting(resetLights);
}

void
CRenderer::RenderFirstPersonVehicle(void)
{
	if(m_pFirstPersonVehicle == nil)
		return;
	RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void*)TRUE);
	RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)TRUE);
	RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*)TRUE);
	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)TRUE);
	RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDSRCALPHA);
	RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDINVSRCALPHA);
	RenderOneNonRoad(m_pFirstPersonVehicle);
	RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void*)FALSE);
}

void
CRenderer::RenderRoads(void)
{
	int i;
	CTreadable *t;

	RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void*)TRUE);
	DeActivateDirectional();
	SetAmbientColours();

	ThePaths.m_pathNodes[-1].group = 6;

	for(i = 0; i < ms_nNoOfVisibleEntities; i++){
		t = (CTreadable*)ms_aVisibleEntityPtrs[i];
		if(t->IsBuilding() && t->GetIsATreadable()){
#ifndef MASTER
			if(gbShowCarRoadGroups || gbShowPedRoadGroups){
				int ind = 0;
				if(gbShowCarRoadGroups)
					ind += ThePaths.m_pathNodes[t->m_nodeIndices[PATH_CAR][0]].group;
				if(gbShowPedRoadGroups)
					ind += ThePaths.m_pathNodes[t->m_nodeIndices[PATH_PED][0]].group;
				SetAmbientColoursToIndicateRoadGroup(ind);
			}
#endif
			RenderOneRoad(t);
#ifndef MASTER
			if(gbShowCarRoadGroups || gbShowPedRoadGroups)
				ReSetAmbientAndDirectionalColours();
#endif
		}
	}
}

void
CRenderer::RenderEverythingBarRoads(void)
{
	int i;
	CEntity *e;
	CVector dist;
	EntityInfo ei;

	gSortedVehiclesAndPeds.Clear();

	for(i = 0; i < ms_nNoOfVisibleEntities; i++){
		e = ms_aVisibleEntityPtrs[i];

		if(e->IsBuilding() && ((CBuilding*)e)->GetIsATreadable())
			continue;

		if(e->IsVehicle() ||
		   e->IsPed() && CVisibilityPlugins::GetClumpAlpha((RpClump*)e->m_rwObject) != 255){
			if(e->IsVehicle() && ((CVehicle*)e)->IsBoat()){
				ei.ent = e;
				dist = ms_vecCameraPosition - e->GetPosition();
				ei.sort = dist.MagnitudeSqr();
				gSortedVehiclesAndPeds.InsertSorted(ei);
			}else{
				dist = ms_vecCameraPosition - e->GetPosition();
				if(!CVisibilityPlugins::InsertEntityIntoSortedList(e, dist.Magnitude())){
					printf("Ran out of space in alpha entity list");
					RenderOneNonRoad(e);
				}
			}
		}else
			RenderOneNonRoad(e);
	}
}

void
CRenderer::RenderVehiclesButNotBoats(void)
{
	// This function doesn't do anything
	// because only boats are inserted into the list
	CLink<EntityInfo> *node;

	for(node = gSortedVehiclesAndPeds.tail.prev;
	    node != &gSortedVehiclesAndPeds.head;
	    node = node->prev){
		// only boats in this list
		CVehicle *v = (CVehicle*)node->item.ent;
		if(!v->IsBoat())
			RenderOneNonRoad(v);
	}
}

void
CRenderer::RenderBoats(void)
{
	CLink<EntityInfo> *node;

	for(node = gSortedVehiclesAndPeds.tail.prev;
	    node != &gSortedVehiclesAndPeds.head;
	    node = node->prev){
		// only boats in this list
		CVehicle *v = (CVehicle*)node->item.ent;
		if(v->IsBoat())
			RenderOneNonRoad(v);
	}
}

void
CRenderer::RenderFadingInEntities(void)
{
	RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void*)TRUE);
	DeActivateDirectional();
	SetAmbientColours();
	CVisibilityPlugins::RenderFadingEntities();
}

void
CRenderer::RenderCollisionLines(void)
{
	int i;

	// game doesn't draw fading in entities
	// this should probably be fixed
	for(i = 0; i < ms_nNoOfVisibleEntities; i++){
		CEntity *e = ms_aVisibleEntityPtrs[i];
		if(Abs(e->GetPosition().x - ms_vecCameraPosition.x) < 100.0f &&
		   Abs(e->GetPosition().y - ms_vecCameraPosition.y) < 100.0f)
			CCollision::DrawColModel(e->GetMatrix(), *e->GetColModel());
	}
}

enum Visbility
{
	VIS_INVISIBLE,
	VIS_VISIBLE,
	VIS_OFFSCREEN,
	VIS_STREAMME
};

#define LOD_DISTANCE 300.0f
#define FADE_DISTANCE 20.0f
#define STREAM_DISTANCE 30.0f

// Time Objects can be time culled if
//   other == -1 || CModelInfo::GetModelInfo(other)->GetRwObject()
// i.e. we have to draw even at the wrong time if
//   other != -1 && CModelInfo::GetModelInfo(other)->GetRwObject() == nil

#define OTHERUNAVAILABLE (other != -1 && CModelInfo::GetModelInfo(other)->GetRwObject() == nil)
#define CANTIMECULL (!OTHERUNAVAILABLE)

int32
CRenderer::SetupEntityVisibility(CEntity *ent)
{
	CSimpleModelInfo *mi = (CSimpleModelInfo*)CModelInfo::GetModelInfo(ent->m_modelIndex);
	CTimeModelInfo *ti;
	int32 other;
	float dist;

	bool request = true;
	if(mi->m_type == MITYPE_TIME){
 		ti = (CTimeModelInfo*)mi;
		other = ti->GetOtherTimeModel();
		if(CClock::GetIsTimeInRange(ti->GetTimeOn(), ti->GetTimeOff())){
			// don't fade in, or between time objects
			if(CANTIMECULL)
				ti->m_alpha = 255;
		}else{
			// Hide if possible
			if(CANTIMECULL)
				return VIS_INVISIBLE;
			// can't cull, so we'll try to draw this one, but don't request
			// it since what we really want is the other one.
			request = false;
		}
	}else{
		if(mi->m_type != MITYPE_SIMPLE){
			if(FindPlayerVehicle() == ent &&
			   TheCamera.Cams[TheCamera.ActiveCam].Mode == CCam::MODE_1STPERSON){
				// Player's vehicle in first person mode
				if(TheCamera.Cams[TheCamera.ActiveCam].DirectionWasLooking == LOOKING_FORWARD ||
				   ent->GetModelIndex() == MI_RHINO ||
				   ent->GetModelIndex() == MI_COACH ||
				   TheCamera.m_bInATunnelAndABigVehicle){
					ent->bNoBrightHeadLights = true;
				}else{
					m_pFirstPersonVehicle = (CVehicle*)ent;
					ent->bNoBrightHeadLights = false;
				}
				return VIS_OFFSCREEN;
			}else{
				// All sorts of Clumps
				if(ent->m_rwObject == nil || !ent->bIsVisible)
					return VIS_INVISIBLE;
				if(!ent->GetIsOnScreen())
					return VIS_OFFSCREEN;
				if(ent->bDrawLast){
					dist = (ent->GetPosition() - ms_vecCameraPosition).Magnitude();
					CVisibilityPlugins::InsertEntityIntoSortedList(ent, dist);
					ent->bDistanceFade = false;
					return VIS_INVISIBLE;
				}else
					return VIS_VISIBLE;
			}
			return VIS_INVISIBLE;
		}
		if(ent->m_type == ENTITY_TYPE_OBJECT &&
		   ((CObject*)ent)->ObjectCreatedBy == TEMP_OBJECT){
			if(ent->m_rwObject == nil || !ent->bIsVisible)
				return VIS_INVISIBLE;
			return ent->GetIsOnScreen() ? VIS_VISIBLE : VIS_OFFSCREEN;
		}
	}

	// Simple ModelInfo

	dist = (ent->GetPosition() - ms_vecCameraPosition).Magnitude();

	// This can only happen with multi-atomic models (e.g. railtracks)
	// but why do we bump up the distance? can only be fading...
	if(LOD_DISTANCE + STREAM_DISTANCE < dist && dist < mi->GetLargestLodDistance())
		dist = mi->GetLargestLodDistance();

	if(ent->m_type == ENTITY_TYPE_OBJECT && ent->bRenderDamaged)
		mi->m_isDamaged = true;

	RpAtomic *a = mi->GetAtomicFromDistance(dist);
	if(a){
		mi->m_isDamaged = 0;
		if(ent->m_rwObject == nil)
			ent->CreateRwObject();
		assert(ent->m_rwObject);
		RpAtomic *rwobj = (RpAtomic*)ent->m_rwObject;
		// Make sure our atomic uses the right geometry and not
		// that of an atomic for another draw distance.
		if(RpAtomicGetGeometry(a) != RpAtomicGetGeometry(rwobj))
			RpAtomicSetGeometry(rwobj, RpAtomicGetGeometry(a), rpATOMICSAMEBOUNDINGSPHERE); // originally 5 (mistake?)
		mi->IncreaseAlpha();
		if(ent->m_rwObject == nil || !ent->bIsVisible)
			return VIS_INVISIBLE;

		if(!ent->GetIsOnScreen()){
			mi->m_alpha = 255;
			return VIS_OFFSCREEN;
		}

		if(mi->m_alpha != 255){
			CVisibilityPlugins::InsertEntityIntoSortedList(ent, dist);
			ent->bDistanceFade = true;
			return VIS_INVISIBLE;
		}

		if(mi->m_drawLast || ent->bDrawLast){
			CVisibilityPlugins::InsertEntityIntoSortedList(ent, dist);
			ent->bDistanceFade = false;
			return VIS_INVISIBLE;
		}
		return VIS_VISIBLE;
	}

	// Object is not loaded, figure out what to do

	if(mi->m_noFade){
		mi->m_isDamaged = false;
		// request model
		if(dist - STREAM_DISTANCE < mi->GetLargestLodDistance() && request)
			return VIS_STREAMME;
		return VIS_INVISIBLE;
	}

	// We might be fading

	a = mi->GetAtomicFromDistance(dist - FADE_DISTANCE);
	mi->m_isDamaged = false;
	if(a == nil){
		// request model
		if(dist - FADE_DISTANCE - STREAM_DISTANCE < mi->GetLargestLodDistance() && request)
			return VIS_STREAMME;
		return VIS_INVISIBLE;
	}

	if(ent->m_rwObject == nil)
		ent->CreateRwObject();
	assert(ent->m_rwObject);
	RpAtomic *rwobj = (RpAtomic*)ent->m_rwObject;
	if(RpAtomicGetGeometry(a) != RpAtomicGetGeometry(rwobj))
		RpAtomicSetGeometry(rwobj, RpAtomicGetGeometry(a), rpATOMICSAMEBOUNDINGSPHERE); // originally 5 (mistake?)
	mi->IncreaseAlpha();
	if(ent->m_rwObject == nil || !ent->bIsVisible)
		return VIS_INVISIBLE;

	if(!ent->GetIsOnScreen()){
		mi->m_alpha = 255;
		return VIS_OFFSCREEN;
	}else{
		CVisibilityPlugins::InsertEntityIntoSortedList(ent, dist);
		ent->bDistanceFade = true;
		return VIS_OFFSCREEN;	// Why this?
	}
}

int32
CRenderer::SetupBigBuildingVisibility(CEntity *ent)
{
	CSimpleModelInfo *mi = (CSimpleModelInfo*)CModelInfo::GetModelInfo(ent->m_modelIndex);
	CTimeModelInfo *ti;
	int32 other;

	if(mi->m_type == MITYPE_TIME){
 		ti = (CTimeModelInfo*)mi;
		other = ti->GetOtherTimeModel();
		// Hide objects not in time range if possible
		if(CANTIMECULL)
			if(!CClock::GetIsTimeInRange(ti->GetTimeOn(), ti->GetTimeOff()))
				return 0;
		// Draw like normal
	}else if(mi->m_type == MITYPE_VEHICLE)
		return ent->IsVisible();

	float dist = (ms_vecCameraPosition-ent->GetPosition()).Magnitude();
	CSimpleModelInfo *nonLOD = mi->GetRelatedModel();

	// Find out whether to draw below near distance.
	// This is only the case if there is a non-LOD which is either not
	// loaded or not completely faded in yet.
	if(dist < mi->GetNearDistance() && dist < LOD_DISTANCE + STREAM_DISTANCE){
		// No non-LOD or non-LOD is completely visible.
		if(nonLOD == nil ||
		   nonLOD->GetRwObject() && nonLOD->m_alpha == 255)
			return 0;

		// But if it is a time object, we'd rather draw the wrong
		// non-LOD than the right LOD.
		if(nonLOD->m_type == MITYPE_TIME){
			ti = (CTimeModelInfo*)nonLOD;
			other = ti->GetOtherTimeModel();
			if(other != -1 && CModelInfo::GetModelInfo(other)->GetRwObject())
				return 0;
		}
	}

	RpAtomic *a = mi->GetAtomicFromDistance(dist);
	if(a){
		if(ent->m_rwObject == nil)
			ent->CreateRwObject();
		assert(ent->m_rwObject);
		RpAtomic *rwobj = (RpAtomic*)ent->m_rwObject;

		// Make sure our atomic uses the right geometry and not
		// that of an atomic for another draw distance.
		if(RpAtomicGetGeometry(a) != RpAtomicGetGeometry(rwobj))
			RpAtomicSetGeometry(rwobj, RpAtomicGetGeometry(a), rpATOMICSAMEBOUNDINGSPHERE); // originally 5 (mistake?)
		if(!ent->IsVisibleComplex())
			return 0;
		if(mi->m_drawLast){
			CVisibilityPlugins::InsertEntityIntoSortedList(ent, dist);
			ent->bDistanceFade = 0;
			return 0;
		}
		return 1;
	}

	if(mi->m_noFade){
		ent->DeleteRwObject();
		return 0;
	}


	// get faded atomic
	a = mi->GetAtomicFromDistance(dist - FADE_DISTANCE);
	if(a == nil){
		ent->DeleteRwObject();
		return 0;
	}

	// Fade...
	if(ent->m_rwObject == nil)
		ent->CreateRwObject();
	assert(ent->m_rwObject);
	RpAtomic *rwobj = (RpAtomic*)ent->m_rwObject;
	if(RpAtomicGetGeometry(a) != RpAtomicGetGeometry(rwobj))
		RpAtomicSetGeometry(rwobj, RpAtomicGetGeometry(a), rpATOMICSAMEBOUNDINGSPHERE); // originally 5 (mistake?)
	if(ent->IsVisibleComplex())
		CVisibilityPlugins::InsertEntityIntoSortedList(ent, dist);
	return 0;
}

void
CRenderer::ConstructRenderList(void)
{
	ms_nNoOfVisibleEntities = 0;
	ms_nNoOfInVisibleEntities = 0;
	ms_vecCameraPosition = TheCamera.GetPosition();
	// TODO: blocked ranges, but unused
	ScanWorld();
}

void
LimitFrustumVector(CVector &vec1, const CVector &vec2, float l)
{
	float f;
	f = (l - vec2.z) / (vec1.z - vec2.z);
	vec1.x = f*(vec1.x - vec2.x) + vec2.x;
	vec1.y = f*(vec1.y - vec2.y) + vec2.y;
	vec1.z = f*(vec1.z - vec2.z) + vec2.z;
}

enum Corners
{
	CORNER_CAM = 0,
	CORNER_FAR_TOPLEFT,
	CORNER_FAR_TOPRIGHT,
	CORNER_FAR_BOTRIGHT,
	CORNER_FAR_BOTLEFT,
	CORNER_LOD_LEFT,
	CORNER_LOD_RIGHT,
	CORNER_PRIO_LEFT,
	CORNER_PRIO_RIGHT,
};

void
CRenderer::ScanWorld(void)
{
	float f = RwCameraGetFarClipPlane(TheCamera.m_pRwCamera);
	RwV2d vw = *RwCameraGetViewWindow(TheCamera.m_pRwCamera);
	CVector vectors[9];
	RwMatrix *cammatrix;
	RwV2d poly[3];

	memset(vectors, 0, sizeof(vectors));
	vectors[CORNER_FAR_TOPLEFT].x = -vw.x * f;
	vectors[CORNER_FAR_TOPLEFT].y = vw.y * f;
	vectors[CORNER_FAR_TOPLEFT].z = f;
	vectors[CORNER_FAR_TOPRIGHT].x = vw.x * f;
	vectors[CORNER_FAR_TOPRIGHT].y = vw.y * f;
	vectors[CORNER_FAR_TOPRIGHT].z = f;
	vectors[CORNER_FAR_BOTRIGHT].x = vw.x * f;
	vectors[CORNER_FAR_BOTRIGHT].y = -vw.y * f;
	vectors[CORNER_FAR_BOTRIGHT].z = f;
	vectors[CORNER_FAR_BOTLEFT].x = -vw.x * f;
	vectors[CORNER_FAR_BOTLEFT].y = -vw.y * f;
	vectors[CORNER_FAR_BOTLEFT].z = f;

	cammatrix = RwFrameGetMatrix(RwCameraGetFrame(TheCamera.m_pRwCamera));

	m_pFirstPersonVehicle = nil;
	CVisibilityPlugins::InitAlphaEntityList();
	CWorld::AdvanceCurrentScanCode();

	if(cammatrix->at.z > 0.0f){
		// looking up, bottom corners are further away
		vectors[CORNER_LOD_LEFT] = vectors[CORNER_FAR_BOTLEFT] * LOD_DISTANCE/f;
		vectors[CORNER_LOD_RIGHT] = vectors[CORNER_FAR_BOTRIGHT] * LOD_DISTANCE/f;
	}else{
		// looking down, top corners are further away
		vectors[CORNER_LOD_LEFT] = vectors[CORNER_FAR_TOPLEFT] * LOD_DISTANCE/f;
		vectors[CORNER_LOD_RIGHT] = vectors[CORNER_FAR_TOPRIGHT] * LOD_DISTANCE/f;
	}
	vectors[CORNER_PRIO_LEFT].x = vectors[CORNER_LOD_LEFT].x * 0.2f;
	vectors[CORNER_PRIO_LEFT].y = vectors[CORNER_LOD_LEFT].y * 0.2f;
	vectors[CORNER_PRIO_LEFT].z = vectors[CORNER_LOD_LEFT].z;
	vectors[CORNER_PRIO_RIGHT].x = vectors[CORNER_LOD_RIGHT].x * 0.2f;
	vectors[CORNER_PRIO_RIGHT].y = vectors[CORNER_LOD_RIGHT].y * 0.2f;
	vectors[CORNER_PRIO_RIGHT].z = vectors[CORNER_LOD_RIGHT].z;
	RwV3dTransformPoints((RwV3d*)vectors, (RwV3d*)vectors, 9, cammatrix);

	m_loadingPriority = false;
	if(TheCamera.Cams[TheCamera.ActiveCam].Mode == CCam::MODE_TOPDOWN ||
#ifdef FIX_BUGS
	   TheCamera.Cams[TheCamera.ActiveCam].Mode == CCam::MODE_GTACLASSIC ||
#endif
	   TheCamera.Cams[TheCamera.ActiveCam].Mode == CCam::MODE_TOP_DOWN_PED){
		CRect rect;
		int x1, x2, y1, y2;
		LimitFrustumVector(vectors[CORNER_FAR_TOPLEFT], vectors[CORNER_CAM], -100.0f);
		rect.ContainPoint(vectors[CORNER_FAR_TOPLEFT]);
		LimitFrustumVector(vectors[CORNER_FAR_TOPRIGHT], vectors[CORNER_CAM], -100.0f);
		rect.ContainPoint(vectors[CORNER_FAR_TOPRIGHT]);
		LimitFrustumVector(vectors[CORNER_FAR_BOTRIGHT], vectors[CORNER_CAM], -100.0f);
		rect.ContainPoint(vectors[CORNER_FAR_BOTRIGHT]);
		LimitFrustumVector(vectors[CORNER_FAR_BOTLEFT], vectors[CORNER_CAM], -100.0f);
		rect.ContainPoint(vectors[CORNER_FAR_BOTLEFT]);
		x1 = CWorld::GetSectorIndexX(rect.left);
		if(x1 < 0) x1 = 0;
		x2 = CWorld::GetSectorIndexX(rect.right);
		if(x2 >= NUMSECTORS_X-1) x2 = NUMSECTORS_X-1;
		y1 = CWorld::GetSectorIndexY(rect.top);
		if(y1 < 0) y1 = 0;
		y2 = CWorld::GetSectorIndexY(rect.bottom);
		if(y2 >= NUMSECTORS_Y-1) y2 = NUMSECTORS_Y-1;
		for(; x1 <= x2; x1++)
			for(int y = y1; y <= y2; y++)
				ScanSectorList(CWorld::GetSector(x1, y)->m_lists);
	}else{
		CVehicle *train = FindPlayerTrain();
		if(train && train->GetPosition().z < 0.0f){
			poly[0].x = CWorld::GetSectorX(vectors[CORNER_CAM].x);
			poly[0].y = CWorld::GetSectorY(vectors[CORNER_CAM].y);
			poly[1].x = CWorld::GetSectorX(vectors[CORNER_LOD_LEFT].x);
			poly[1].y = CWorld::GetSectorY(vectors[CORNER_LOD_LEFT].y);
			poly[2].x = CWorld::GetSectorX(vectors[CORNER_LOD_RIGHT].x);
			poly[2].y = CWorld::GetSectorY(vectors[CORNER_LOD_RIGHT].y);
			ScanSectorPoly(poly, 3, ScanSectorList_Subway);
		}else{
			if(f <= LOD_DISTANCE){
				poly[0].x = CWorld::GetSectorX(vectors[CORNER_CAM].x);
				poly[0].y = CWorld::GetSectorY(vectors[CORNER_CAM].y);
				poly[1].x = CWorld::GetSectorX(vectors[CORNER_FAR_TOPLEFT].x);
				poly[1].y = CWorld::GetSectorY(vectors[CORNER_FAR_TOPLEFT].y);
				poly[2].x = CWorld::GetSectorX(vectors[CORNER_FAR_TOPRIGHT].x);
				poly[2].y = CWorld::GetSectorY(vectors[CORNER_FAR_TOPRIGHT].y);
			}else{
				// priority
				poly[0].x = CWorld::GetSectorX(vectors[CORNER_CAM].x);
				poly[0].y = CWorld::GetSectorY(vectors[CORNER_CAM].y);
				poly[1].x = CWorld::GetSectorX(vectors[CORNER_PRIO_LEFT].x);
				poly[1].y = CWorld::GetSectorY(vectors[CORNER_PRIO_LEFT].y);
				poly[2].x = CWorld::GetSectorX(vectors[CORNER_PRIO_RIGHT].x);
				poly[2].y = CWorld::GetSectorY(vectors[CORNER_PRIO_RIGHT].y);
				ScanSectorPoly(poly, 3, ScanSectorList_Priority);

				// below LOD
				poly[0].x = CWorld::GetSectorX(vectors[CORNER_CAM].x);
				poly[0].y = CWorld::GetSectorY(vectors[CORNER_CAM].y);
				poly[1].x = CWorld::GetSectorX(vectors[CORNER_LOD_LEFT].x);
				poly[1].y = CWorld::GetSectorY(vectors[CORNER_LOD_LEFT].y);
				poly[2].x = CWorld::GetSectorX(vectors[CORNER_LOD_RIGHT].x);
				poly[2].y = CWorld::GetSectorY(vectors[CORNER_LOD_RIGHT].y);
			}
			ScanSectorPoly(poly, 3, ScanSectorList);

			ScanBigBuildingList(CWorld::GetBigBuildingList(CCollision::ms_collisionInMemory));
			ScanBigBuildingList(CWorld::GetBigBuildingList(LEVEL_NONE));
		}
	}
}

void
CRenderer::RequestObjectsInFrustum(void)
{
	float f = RwCameraGetFarClipPlane(TheCamera.m_pRwCamera);
	RwV2d vw = *RwCameraGetViewWindow(TheCamera.m_pRwCamera);
	CVector vectors[9];
	RwMatrix *cammatrix;
	RwV2d poly[3];

	memset(vectors, 0, sizeof(vectors));
	vectors[CORNER_FAR_TOPLEFT].x = -vw.x * f;
	vectors[CORNER_FAR_TOPLEFT].y = vw.y * f;
	vectors[CORNER_FAR_TOPLEFT].z = f;
	vectors[CORNER_FAR_TOPRIGHT].x = vw.x * f;
	vectors[CORNER_FAR_TOPRIGHT].y = vw.y * f;
	vectors[CORNER_FAR_TOPRIGHT].z = f;
	vectors[CORNER_FAR_BOTRIGHT].x = vw.x * f;
	vectors[CORNER_FAR_BOTRIGHT].y = -vw.y * f;
	vectors[CORNER_FAR_BOTRIGHT].z = f;
	vectors[CORNER_FAR_BOTLEFT].x = -vw.x * f;
	vectors[CORNER_FAR_BOTLEFT].y = -vw.y * f;
	vectors[CORNER_FAR_BOTLEFT].z = f;

	cammatrix = RwFrameGetMatrix(RwCameraGetFrame(TheCamera.m_pRwCamera));

	CWorld::AdvanceCurrentScanCode();

	if(cammatrix->at.z > 0.0f){
		// looking up, bottom corners are further away
		vectors[CORNER_LOD_LEFT] = vectors[CORNER_FAR_BOTLEFT] * LOD_DISTANCE/f;
		vectors[CORNER_LOD_RIGHT] = vectors[CORNER_FAR_BOTRIGHT] * LOD_DISTANCE/f;
	}else{
		// looking down, top corners are further away
		vectors[CORNER_LOD_LEFT] = vectors[CORNER_FAR_TOPLEFT] * LOD_DISTANCE/f;
		vectors[CORNER_LOD_RIGHT] = vectors[CORNER_FAR_TOPRIGHT] * LOD_DISTANCE/f;
	}
	vectors[CORNER_PRIO_LEFT].x = vectors[CORNER_LOD_LEFT].x * 0.2f;
	vectors[CORNER_PRIO_LEFT].y = vectors[CORNER_LOD_LEFT].y * 0.2f;
	vectors[CORNER_PRIO_LEFT].z = vectors[CORNER_LOD_LEFT].z;
	vectors[CORNER_PRIO_RIGHT].x = vectors[CORNER_LOD_RIGHT].x * 0.2f;
	vectors[CORNER_PRIO_RIGHT].y = vectors[CORNER_LOD_RIGHT].y * 0.2f;
	vectors[CORNER_PRIO_RIGHT].z = vectors[CORNER_LOD_RIGHT].z;
	RwV3dTransformPoints((RwV3d*)vectors, (RwV3d*)vectors, 9, cammatrix);

	if(TheCamera.Cams[TheCamera.ActiveCam].Mode == CCam::MODE_TOPDOWN ||
#ifdef FIX_BUGS
	   TheCamera.Cams[TheCamera.ActiveCam].Mode == CCam::MODE_GTACLASSIC ||
#endif
	   TheCamera.Cams[TheCamera.ActiveCam].Mode == CCam::MODE_TOP_DOWN_PED){
		CRect rect;
		int x1, x2, y1, y2;
		LimitFrustumVector(vectors[CORNER_FAR_TOPLEFT], vectors[CORNER_CAM], -100.0f);
		rect.ContainPoint(vectors[CORNER_FAR_TOPLEFT]);
		LimitFrustumVector(vectors[CORNER_FAR_TOPRIGHT], vectors[CORNER_CAM], -100.0f);
		rect.ContainPoint(vectors[CORNER_FAR_TOPRIGHT]);
		LimitFrustumVector(vectors[CORNER_FAR_BOTRIGHT], vectors[CORNER_CAM], -100.0f);
		rect.ContainPoint(vectors[CORNER_FAR_BOTRIGHT]);
		LimitFrustumVector(vectors[CORNER_FAR_BOTLEFT], vectors[CORNER_CAM], -100.0f);
		rect.ContainPoint(vectors[CORNER_FAR_BOTLEFT]);
		x1 = CWorld::GetSectorIndexX(rect.left);
		if(x1 < 0) x1 = 0;
		x2 = CWorld::GetSectorIndexX(rect.right);
		if(x2 >= NUMSECTORS_X-1) x2 = NUMSECTORS_X-1;
		y1 = CWorld::GetSectorIndexY(rect.top);
		if(y1 < 0) y1 = 0;
		y2 = CWorld::GetSectorIndexY(rect.bottom);
		if(y2 >= NUMSECTORS_Y-1) y2 = NUMSECTORS_Y-1;
		for(; x1 <= x2; x1++)
			for(int y = y1; y <= y2; y++)
				ScanSectorList_RequestModels(CWorld::GetSector(x1, y)->m_lists);
	}else{
		poly[0].x = CWorld::GetSectorX(vectors[CORNER_CAM].x);
		poly[0].y = CWorld::GetSectorY(vectors[CORNER_CAM].y);
		poly[1].x = CWorld::GetSectorX(vectors[CORNER_LOD_LEFT].x);
		poly[1].y = CWorld::GetSectorY(vectors[CORNER_LOD_LEFT].y);
		poly[2].x = CWorld::GetSectorX(vectors[CORNER_LOD_RIGHT].x);
		poly[2].y = CWorld::GetSectorY(vectors[CORNER_LOD_RIGHT].y);
		ScanSectorPoly(poly, 3, ScanSectorList_RequestModels);
	}
}

float
CalcNewDelta(RwV2d *a, RwV2d *b)
{
	return (b->x - a->x) / (b->y - a->y);
}

void
CRenderer::ScanSectorPoly(RwV2d *poly, int32 numVertices, void (*scanfunc)(CPtrList *))
{
	float miny, maxy;
	int y, yend;
	int x, xstart, xend;
	int i;
	int a1, a2, b1, b2;
	float deltaA, deltaB;
	float xA, xB;

	miny = poly[0].y;
	maxy = poly[0].y;
	a2 = 0;
	xstart = 9999;
	xend = -9999;

	for(i = 1; i < numVertices; i++){
		if(poly[i].y > maxy)
			maxy = poly[i].y;
		if(poly[i].y < miny){
			miny = poly[i].y;
			a2 = i;
		}
	}
	y = miny;
	yend = maxy;

	// Go left in poly to find first edge b
	b2 = a2;
	for(i = 0; i < numVertices; i++){
		b1 = b2--;
		if(b2 < 0) b2 = numVertices-1;
		if(poly[b1].x < xstart)
			xstart = poly[b1].x;
		if((int)poly[b1].y != (int)poly[b2].y)
			break;
	}
	// Go right to find first edge a
	for(i = 0; i < numVertices; i++){
		a1 = a2++;
		if(a2 == numVertices) a2 = 0;
		if(poly[a1].x > xend)
			xend = poly[a1].x;
		if((int)poly[a1].y != (int)poly[a2].y)
			break;
	}

	// prestep x1 and x2 to next integer y
	deltaA = CalcNewDelta(&poly[a1], &poly[a2]);
	xA = deltaA * (ceilf(poly[a1].y) - poly[a1].y) + poly[a1].x;
	deltaB = CalcNewDelta(&poly[b1], &poly[b2]);
	xB = deltaB * (ceilf(poly[b1].y) - poly[b1].y) + poly[b1].x;

	if(y != yend){
		if(deltaB < 0.0f && (int)xB < xstart)
			xstart = xB;
		if(deltaA >= 0.0f && (int)xA > xend)
			xend = xA;
	}

	while(y <= yend && y < NUMSECTORS_Y){
		// scan one x-line
		if(y >= 0 && xstart < NUMSECTORS_X)
			for(x = xstart; x <= xend; x++)
				if(x >= 0 && x != NUMSECTORS_X)
					scanfunc(CWorld::GetSector(x, y)->m_lists);

		// advance one scan line
		y++;
		xA += deltaA;
		xB += deltaB;

		// update left side
		if(y == (int)poly[b2].y){
			// reached end of edge
			if(y == yend){
				if(deltaB < 0.0f){
					do{
						xstart = poly[b2--].x;
						if(b2 < 0) b2 = numVertices-1;
					}while(xstart > (int)poly[b2].x);
				}else
					xstart = xB - deltaB;
			}else{
				// switch edges
				if(deltaB < 0.0f)
					xstart = poly[b2].x;
				else
					xstart = xB - deltaB;
				do{
					b1 = b2--;
					if(b2 < 0) b2 = numVertices-1;
					if((int)poly[b1].x < xstart)
						xstart = poly[b1].x;
				}while(y == (int)poly[b2].y);
				deltaB = CalcNewDelta(&poly[b1], &poly[b2]);
				xB = deltaB * (ceilf(poly[b1].y) - poly[b1].y) + poly[b1].x;
				if(deltaB < 0.0f && (int)xB < xstart)
					xstart = xB;
			}
		}else{
			if(deltaB < 0.0f)
				xstart = xB;
			else
				xstart = xB - deltaB;
		}

		// update right side
		if(y == (int)poly[a2].y){
			// reached end of edge
			if(y == yend){
				if(deltaA < 0.0f)
					xend = xA - deltaA;
				else{
					do{
						xend = poly[a2++].x;
						if(a2 == numVertices) a2 = 0;
					}while(xend < (int)poly[a2].x);
				}
			}else{
				// switch edges
				if(deltaA < 0.0f)
					xend = xA - deltaA;
				else
					xend = poly[a2].x;
				do{
					a1 = a2++;
					if(a2 == numVertices) a2 = 0;
					if((int)poly[a1].x > xend)
						xend = poly[a1].x;
				}while(y == (int)poly[a2].y);
				deltaA = CalcNewDelta(&poly[a1], &poly[a2]);
				xA = deltaA * (ceilf(poly[a1].y) - poly[a1].y) + poly[a1].x;
				if(deltaA >= 0.0f && (int)xA > xend)
					xend = xA;
			}
		}else{
			if(deltaA < 0.0f)
				xend = xA - deltaA;
			else
				xend = xA;
		}
	}
}

void
CRenderer::ScanBigBuildingList(CPtrList &list)
{
	CPtrNode *node;
	CEntity *ent;

	for(node = list.first; node; node = node->next){
		ent = (CEntity*)node->item;
		if(!ent->bZoneCulled && SetupBigBuildingVisibility(ent) == 1)
			ms_aVisibleEntityPtrs[ms_nNoOfVisibleEntities++] = ent;
	}
}

void
CRenderer::ScanSectorList(CPtrList *lists)
{
	CPtrNode *node;
	CPtrList *list;
	CEntity *ent;
	int i;
	float dx, dy;

	for(i = 0; i < NUMSECTORENTITYLISTS; i++){
		list = &lists[i];
		for(node = list->first; node; node = node->next){
			ent = (CEntity*)node->item;
			if(ent->m_scanCode == CWorld::GetCurrentScanCode())
				continue;	// already seen
			ent->m_scanCode = CWorld::GetCurrentScanCode();

			if(IsEntityCullZoneVisible(ent))
				switch(SetupEntityVisibility(ent)){
				case VIS_VISIBLE:
					ms_aVisibleEntityPtrs[ms_nNoOfVisibleEntities++] = ent;
					break;
				case VIS_INVISIBLE:
					if(!IsGlass(ent->GetModelIndex()))
						break;
					// fall through
				case VIS_OFFSCREEN:
					dx = ms_vecCameraPosition.x - ent->GetPosition().x;
					dy = ms_vecCameraPosition.y - ent->GetPosition().y;
					if(dx > -65.0f && dx < 65.0f &&
					   dy > -65.0f && dy < 65.0f &&
					   ms_nNoOfInVisibleEntities < NUMINVISIBLEENTITIES - 1)
						ms_aInVisibleEntityPtrs[ms_nNoOfInVisibleEntities++] = ent;
					break;
				case VIS_STREAMME:
					if(!CStreaming::ms_disableStreaming)
						if(!m_loadingPriority || CStreaming::ms_numModelsRequested < 10)
							CStreaming::RequestModel(ent->GetModelIndex(), 0);
					break;
				}
			else if(ent->IsBuilding() && ((CBuilding*)ent)->GetIsATreadable()){
				if(!CStreaming::ms_disableStreaming)
					if(SetupEntityVisibility(ent) == VIS_STREAMME)
						if(!m_loadingPriority || CStreaming::ms_numModelsRequested < 10)
							CStreaming::RequestModel(ent->GetModelIndex(), 0);
			}
		}
	}
}

void
CRenderer::ScanSectorList_Priority(CPtrList *lists)
{
	CPtrNode *node;
	CPtrList *list;
	CEntity *ent;
	int i;
	float dx, dy;

	for(i = 0; i < NUMSECTORENTITYLISTS; i++){
		list = &lists[i];
		for(node = list->first; node; node = node->next){
			ent = (CEntity*)node->item;
			if(ent->m_scanCode == CWorld::GetCurrentScanCode())
				continue;	// already seen
			ent->m_scanCode = CWorld::GetCurrentScanCode();

			if(IsEntityCullZoneVisible(ent))
				switch(SetupEntityVisibility(ent)){
				case VIS_VISIBLE:
					ms_aVisibleEntityPtrs[ms_nNoOfVisibleEntities++] = ent;
					break;
				case VIS_INVISIBLE:
					if(!IsGlass(ent->GetModelIndex()))
						break;
					// fall through
				case VIS_OFFSCREEN:
					dx = ms_vecCameraPosition.x - ent->GetPosition().x;
					dy = ms_vecCameraPosition.y - ent->GetPosition().y;
					if(dx > -65.0f && dx < 65.0f &&
					   dy > -65.0f && dy < 65.0f &&
					   ms_nNoOfInVisibleEntities < NUMINVISIBLEENTITIES - 1)
						ms_aInVisibleEntityPtrs[ms_nNoOfInVisibleEntities++] = ent;
					break;
				case VIS_STREAMME:
					if(!CStreaming::ms_disableStreaming){
						CStreaming::RequestModel(ent->GetModelIndex(), 0);
						if(CStreaming::ms_aInfoForModel[ent->GetModelIndex()].m_loadState != STREAMSTATE_LOADED)
							m_loadingPriority = true;
					}
					break;
				}
			else if(ent->IsBuilding() && ((CBuilding*)ent)->GetIsATreadable()){
				if(!CStreaming::ms_disableStreaming)
					if(SetupEntityVisibility(ent) == VIS_STREAMME)
						CStreaming::RequestModel(ent->GetModelIndex(), 0);
			}
		}
	}
}

void
CRenderer::ScanSectorList_Subway(CPtrList *lists)
{
	CPtrNode *node;
	CPtrList *list;
	CEntity *ent;
	int i;
	float dx, dy;

	for(i = 0; i < NUMSECTORENTITYLISTS; i++){
		list = &lists[i];
		for(node = list->first; node; node = node->next){
			ent = (CEntity*)node->item;
			if(ent->m_scanCode == CWorld::GetCurrentScanCode())
				continue;	// already seen
			ent->m_scanCode = CWorld::GetCurrentScanCode();
			switch(SetupEntityVisibility(ent)){
			case VIS_VISIBLE:
				ms_aVisibleEntityPtrs[ms_nNoOfVisibleEntities++] = ent;
				break;
			case VIS_OFFSCREEN:
				dx = ms_vecCameraPosition.x - ent->GetPosition().x;
				dy = ms_vecCameraPosition.y - ent->GetPosition().y;
				if(dx > -65.0f && dx < 65.0f &&
				   dy > -65.0f && dy < 65.0f &&
				   ms_nNoOfInVisibleEntities < NUMINVISIBLEENTITIES - 1)
					ms_aInVisibleEntityPtrs[ms_nNoOfInVisibleEntities++] = ent;
				break;
			}
		}
	}
}

void
CRenderer::ScanSectorList_RequestModels(CPtrList *lists)
{
	CPtrNode *node;
	CPtrList *list;
	CEntity *ent;
	int i;

	for(i = 0; i < NUMSECTORENTITYLISTS; i++){
		list = &lists[i];
		for(node = list->first; node; node = node->next){
			ent = (CEntity*)node->item;
			if(ent->m_scanCode == CWorld::GetCurrentScanCode())
				continue;	// already seen
			ent->m_scanCode = CWorld::GetCurrentScanCode();
			if(IsEntityCullZoneVisible(ent) && ShouldModelBeStreamed(ent))
				CStreaming::RequestModel(ent->GetModelIndex(), 0);
		}
	}
}

// Put big buildings in front
// This seems pointless because the sector lists shouldn't have big buildings in the first place
void
CRenderer::SortBIGBuildings(void)
{
	int x, y;
	for(y = 0; y < NUMSECTORS_Y; y++)
		for(x = 0; x < NUMSECTORS_X; x++){
			SortBIGBuildingsForSectorList(&CWorld::GetSector(x, y)->m_lists[ENTITYLIST_BUILDINGS]);
			SortBIGBuildingsForSectorList(&CWorld::GetSector(x, y)->m_lists[ENTITYLIST_BUILDINGS_OVERLAP]);
		}
}

void
CRenderer::SortBIGBuildingsForSectorList(CPtrList *list)
{
	CPtrNode *node;
	CEntity *ent;

	for(node = list->first; node; node = node->next){
		ent = (CEntity*)node->item;
		if(ent->bIsBIGBuilding){
			list->RemoveNode(node);
			list->InsertNode(node);
		}
	}
}

bool
CRenderer::ShouldModelBeStreamed(CEntity *ent)
{
	CSimpleModelInfo *mi = (CSimpleModelInfo*)CModelInfo::GetModelInfo(ent->m_modelIndex);
	float dist = (ent->GetPosition() - ms_vecCameraPosition).Magnitude();
	if(mi->m_noFade)
		return dist - STREAM_DISTANCE < mi->GetLargestLodDistance();
	else
		return dist - FADE_DISTANCE - STREAM_DISTANCE < mi->GetLargestLodDistance();
}

bool
CRenderer::IsEntityCullZoneVisible(CEntity *ent)
{
	CPed *ped;
	CObject *obj;

	if(ent->bZoneCulled)
		return false;

	switch(ent->m_type){
	case ENTITY_TYPE_VEHICLE:
		return IsVehicleCullZoneVisible(ent);
	case ENTITY_TYPE_PED:
		ped = (CPed*)ent;
		if (ped->bInVehicle) {
			if (ped->m_pMyVehicle)
				return IsVehicleCullZoneVisible(ped->m_pMyVehicle);
			else
				return true;
		}
		return !(ped->m_pCurSurface && ped->m_pCurSurface->bZoneCulled2);
	case ENTITY_TYPE_OBJECT:
		obj = (CObject*)ent;
		if(!obj->bIsStatic)
			return true;
		return !(obj->m_pCurSurface && obj->m_pCurSurface->bZoneCulled2);
	}
	return true;
}

bool
CRenderer::IsVehicleCullZoneVisible(CEntity *ent)
{
	CVehicle *v = (CVehicle*)ent;
	switch(v->m_status)
	case STATUS_SIMPLE:
	case STATUS_PHYSICS:
	case STATUS_ABANDONED:
	case STATUS_WRECKED:
		return !(v->m_pCurGroundEntity && v->m_pCurGroundEntity->bZoneCulled2);
	return true;
}

void
CRenderer::RemoveVehiclePedLights(CEntity *ent, bool reset)
{
	if(ent->bRenderScorched)
		WorldReplaceScorchedLightsWithNormal(Scene.world);
	CPointLights::RemoveLightsAffectingObject();
	if(reset)
		ReSetAmbientAndDirectionalColours();
}

STARTPATCHES
	InjectHook(0x4A7680, CRenderer::Init, PATCH_JUMP);
	InjectHook(0x4A76A0, CRenderer::Shutdown, PATCH_JUMP);

	InjectHook(0x4A7B90, CRenderer::RenderOneRoad, PATCH_JUMP);
	InjectHook(0x4A7BA0, CRenderer::RenderOneNonRoad, PATCH_JUMP);
	InjectHook(0x4A7B20, CRenderer::RenderFirstPersonVehicle, PATCH_JUMP);
	InjectHook(0x4A78B0, CRenderer::RenderRoads, PATCH_JUMP);
	InjectHook(0x4A7930, CRenderer::RenderEverythingBarRoads, PATCH_JUMP);
	InjectHook(0x4A7AA0, CRenderer::RenderVehiclesButNotBoats, PATCH_JUMP);
	InjectHook(0x4A7AE0, CRenderer::RenderBoats, PATCH_JUMP);
	InjectHook(0x4A7910, CRenderer::RenderFadingInEntities, PATCH_JUMP);

	InjectHook(0x4A9350, CRenderer::SetupEntityVisibility, PATCH_JUMP);
	InjectHook(0x4A9920, CRenderer::SetupBigBuildingVisibility, PATCH_JUMP);

	InjectHook(0x4A76B0, CRenderer::ConstructRenderList, PATCH_JUMP);
	InjectHook(0x4A7840, CRenderer::PreRender, PATCH_JUMP);
	InjectHook(0x4A8970, CRenderer::ScanWorld, PATCH_JUMP);
	InjectHook(0x4AA240, CRenderer::RequestObjectsInFrustum, PATCH_JUMP);
	InjectHook(0x4A7F30, CRenderer::ScanSectorPoly, PATCH_JUMP);
	InjectHook(0x4A9300, CRenderer::ScanBigBuildingList, PATCH_JUMP);
	InjectHook(0x4A9BB0, CRenderer::ScanSectorList, PATCH_JUMP);
	InjectHook(0x4A9E30, CRenderer::ScanSectorList_Priority, PATCH_JUMP);
	InjectHook(0x4AA0A0, CRenderer::ScanSectorList_Subway, PATCH_JUMP);
	InjectHook(0x4AA1D0, CRenderer::ScanSectorList_RequestModels, PATCH_JUMP);

	InjectHook(0x4AA940, CRenderer::SortBIGBuildings, PATCH_JUMP);
	InjectHook(0x4AA990, CRenderer::SortBIGBuildingsForSectorList, PATCH_JUMP);

	InjectHook(0x4A9840, CRenderer::ShouldModelBeStreamed, PATCH_JUMP);
	InjectHook(0x4AAA00, CRenderer::IsEntityCullZoneVisible, PATCH_JUMP);
	InjectHook(0x4AAAA0, CRenderer::IsVehicleCullZoneVisible, PATCH_JUMP);

	InjectHook(0x4A7CF0, CRenderer::RemoveVehiclePedLights, PATCH_JUMP);
ENDPATCHES
