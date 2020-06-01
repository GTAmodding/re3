#include "common.h"

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
#include "Occlusion.h"
#include "Renderer.h"

//--MIAMI: file almost done, just one bike flag left

bool gbShowCollisionPolys;
bool gbShowCollisionLines;
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

CLinkList<EntityInfo> gSortedVehiclesAndPeds;

int32 CRenderer::ms_nNoOfVisibleEntities;
CEntity *CRenderer::ms_aVisibleEntityPtrs[NUMVISIBLEENTITIES];
CEntity *CRenderer::ms_aInVisibleEntityPtrs[NUMINVISIBLEENTITIES];
int32 CRenderer::ms_nNoOfInVisibleEntities;

CVector CRenderer::ms_vecCameraPosition;
CVehicle *CRenderer::m_pFirstPersonVehicle;
bool CRenderer::m_loadingPriority;
float CRenderer::ms_lodDistScale = 1.2f;

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
		CCollision::DrawColModel_Coloured(e->GetMatrix(), *CModelInfo::GetModelInfo(e->GetModelIndex())->GetColModel(), e->GetModelIndex());
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
			CCollision::DrawColModel_Coloured(e->GetMatrix(), *CModelInfo::GetModelInfo(e->GetModelIndex())->GetColModel(), e->GetModelIndex());
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

	if(e->IsVehicle()){
		// unfortunately can't use GetClump here
		CVisibilityPlugins::SetupVehicleVariables((RpClump*)e->m_rwObject);
		CVisibilityPlugins::InitAlphaAtomicList();
	}

	// Render Peds in vehicle before vehicle itself
	if(e->IsVehicle()){
		veh = (CVehicle*)e;
		if(veh->pDriver && veh->pDriver->m_nPedState == PED_DRIVING)
			veh->pDriver->Render();
		for(i = 0; i < 8; i++)
			if(veh->pPassengers[i] && veh->pPassengers[i]->m_nPedState == PED_DRIVING)
				veh->pPassengers[i]->Render();
		SetCullMode(rwCULLMODECULLNONE);
	}
	e->Render();

	if(e->IsVehicle()){
		e->bImBeingRendered = true;
		CVisibilityPlugins::RenderAlphaAtomics();
		e->bImBeingRendered = false;
		SetCullMode(rwCULLMODECULLBACK);
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

inline bool IsRoad(CEntity *e) { return e->IsBuilding() && ((CSimpleModelInfo*)CModelInfo::GetModelInfo(e->GetModelIndex()))->m_wetRoadReflection; }

void
CRenderer::RenderRoads(void)
{
	int i;
	CEntity *e;

	RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void*)TRUE);
	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)TRUE);
	SetCullMode(rwCULLMODECULLBACK);
	DeActivateDirectional();
	SetAmbientColours();

	for(i = 0; i < ms_nNoOfVisibleEntities; i++){
		e = ms_aVisibleEntityPtrs[i];
		if(IsRoad(e))
			RenderOneRoad(e);
	}
}

inline bool PutIntoSortedVehicleList(CVehicle *veh)
{
	if(veh->IsBoat()){
		int mode = TheCamera.Cams[TheCamera.ActiveCam].Mode;
		if(mode == CCam::MODE_WHEELCAM ||
		   mode == CCam::MODE_1STPERSON && TheCamera.GetLookDirection() != LOOKING_FORWARD && TheCamera.GetLookDirection() != LOOKING_BEHIND ||
		   CVisibilityPlugins::GetClumpAlpha(veh->GetClump()) != 255)
			return false;
		return true;
	}else
		return veh->bTouchingWater;		
}

void
CRenderer::RenderEverythingBarRoads(void)
{
	int i;
	CEntity *e;
	CVector dist;
	EntityInfo ei;

	RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void*)TRUE);
	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)TRUE);
	SetCullMode(rwCULLMODECULLBACK);
	gSortedVehiclesAndPeds.Clear();

	for(i = 0; i < ms_nNoOfVisibleEntities; i++){
		e = ms_aVisibleEntityPtrs[i];

		if(IsRoad(e))
			continue;

		if(e->IsVehicle() ||
		   e->IsPed() && CVisibilityPlugins::GetClumpAlpha((RpClump*)e->m_rwObject) != 255){
			if(e->IsVehicle() && PutIntoSortedVehicleList((CVehicle*)e)){
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
CRenderer::RenderBoats(void)
{
	CLink<EntityInfo> *node;

	RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void*)TRUE);
	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)TRUE);
	SetCullMode(rwCULLMODECULLBACK);

	for(node = gSortedVehiclesAndPeds.tail.prev;
	    node != &gSortedVehiclesAndPeds.head;
	    node = node->prev){
		CVehicle *v = (CVehicle*)node->item.ent;
		RenderOneNonRoad(v);
	}
}

void
CRenderer::RenderFadingInEntities(void)
{
	RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void*)TRUE);
	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)TRUE);
	SetCullMode(rwCULLMODECULLBACK);
	DeActivateDirectional();
	SetAmbientColours();
	CVisibilityPlugins::RenderFadingEntities();
}

void
CRenderer::RenderFadingInUnderwaterEntities(void)
{
	DeActivateDirectional();
	SetAmbientColours();
	CVisibilityPlugins::RenderFadingUnderwaterEntities();
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
	if(mi->GetModelType() == MITYPE_TIME){
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
		if(mi->GetModelType() != MITYPE_SIMPLE && mi->GetModelType() != MITYPE_WEAPON){
			if(FindPlayerVehicle() == ent &&
			   TheCamera.Cams[TheCamera.ActiveCam].Mode == CCam::MODE_1STPERSON &&
//TODO(MIAMI): that bike flag
			   (!FindPlayerVehicle()->IsBike() || true)){
				// Player's vehicle in first person mode
				CVehicle *veh = (CVehicle*)ent;
				int model = veh->GetModelIndex();
				int direction = TheCamera.Cams[TheCamera.ActiveCam].DirectionWasLooking;
				if(direction == LOOKING_FORWARD ||
				   ent->GetModelIndex() == MI_RHINO ||
				   ent->GetModelIndex() == MI_COACH ||
				   TheCamera.m_bInATunnelAndABigVehicle ||
				   direction == LOOKING_BEHIND && veh->pHandling->Flags & HANDLING_UNKNOWN){
					ent->bNoBrightHeadLights = true;
					return VIS_OFFSCREEN;
				}

				if(direction != LOOKING_BEHIND ||
				   !veh->IsBoat() || model == MI_REEFER || model == MI_TROPIC || model == MI_PREDATOR || model == MI_SKIMMER){
					m_pFirstPersonVehicle = (CVehicle*)ent;
					ent->bNoBrightHeadLights = false;
					return VIS_OFFSCREEN;
				}
			}

			// All sorts of Clumps
			if(ent->m_rwObject == nil || !ent->bIsVisible)
				return VIS_INVISIBLE;
			if(!ent->GetIsOnScreen() || ent->IsEntityOccluded())
				return VIS_OFFSCREEN;
			if(ent->bDrawLast){
				dist = (ent->GetPosition() - ms_vecCameraPosition).Magnitude();
				CVisibilityPlugins::InsertEntityIntoSortedList(ent, dist);
				ent->bDistanceFade = false;
				return VIS_INVISIBLE;
			}
			return VIS_VISIBLE;
		}
		if(ent->bDontStream){
			if(ent->m_rwObject == nil || !ent->bIsVisible)
				return VIS_INVISIBLE;
			if(!ent->GetIsOnScreen() || ent->IsEntityOccluded())
				return VIS_OFFSCREEN;
			if(ent->bDrawLast){
				dist = (ent->GetPosition() - ms_vecCameraPosition).Magnitude();
				CVisibilityPlugins::InsertEntityIntoSortedList(ent, dist);
				ent->bDistanceFade = false;
				return VIS_INVISIBLE;
			}
			return VIS_VISIBLE;
		}
	}

	// Simple ModelInfo

	if(!IsAreaVisible(ent->m_area))
		return VIS_INVISIBLE;

	dist = (ent->GetPosition() - ms_vecCameraPosition).Magnitude();

#ifndef FIX_BUGS
	// Whatever this is supposed to do, it breaks fading for objects
	// whose draw dist is > LOD_DISTANCE-FADE_DISTANCE, i.e. 280
	// because decreasing dist here makes the object visible above LOD_DISTANCE
	// before fading normally once below LOD_DISTANCE.
	if(LOD_DISTANCE < dist && dist < mi->GetLargestLodDistance() + FADE_DISTANCE)
		dist += mi->GetLargestLodDistance() - LOD_DISTANCE;
#endif

	if(ent->IsObject() && ent->bRenderDamaged)
		mi->m_isDamaged = true;

	RpAtomic *a = mi->GetAtomicFromDistance(dist);
	if(a){
		mi->m_isDamaged = false;
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

		if(!ent->GetIsOnScreen() || ent->IsEntityOccluded()){
			mi->m_alpha = 255;
			return VIS_OFFSCREEN;
		}

		if(mi->m_alpha != 255){
			CVisibilityPlugins::InsertEntityIntoSortedList(ent, dist);
			ent->bDistanceFade = true;
			return VIS_INVISIBLE;
		}

		if(mi->m_drawLast || ent->bDrawLast){
			if(CVisibilityPlugins::InsertEntityIntoSortedList(ent, dist)){
				ent->bDistanceFade = false;
				return VIS_INVISIBLE;
			}
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

	if(!ent->GetIsOnScreen() || ent->IsEntityOccluded()){
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

	if(!IsAreaVisible(ent->m_area))
		return VIS_INVISIBLE;

	bool request = true;
	if(mi->GetModelType() == MITYPE_TIME){
		ti = (CTimeModelInfo*)mi;
		other = ti->GetOtherTimeModel();
		if(CClock::GetIsTimeInRange(ti->GetTimeOn(), ti->GetTimeOff())){
			// don't fade in, or between time objects
			if(CANTIMECULL)
				ti->m_alpha = 255;
		}else{
			// Hide if possible
			if(CANTIMECULL){
				ent->DeleteRwObject();
				return VIS_INVISIBLE;
			}
			// can't cull, so we'll try to draw this one, but don't request
			// it since what we really want is the other one.
			request = false;
		}
	}else if(mi->GetModelType() == MITYPE_VEHICLE)
		return ent->IsVisible() ? VIS_VISIBLE : VIS_INVISIBLE;

	float dist = (ms_vecCameraPosition-ent->GetPosition()).Magnitude();
	CSimpleModelInfo *nonLOD = mi->GetRelatedModel();

	// Find out whether to draw below near distance.
	// This is only the case if there is a non-LOD which is either not
	// loaded or not completely faded in yet.
	if(dist < mi->GetNearDistance() && dist < LOD_DISTANCE){
		// No non-LOD or non-LOD is completely visible.
		if(nonLOD == nil ||
		   nonLOD->GetRwObject() && nonLOD->m_alpha == 255)
			return VIS_INVISIBLE;

		// But if it is a time object, we'd rather draw the wrong
		// non-LOD than the right LOD.
		if(nonLOD->GetModelType() == MITYPE_TIME){
			ti = (CTimeModelInfo*)nonLOD;
			other = ti->GetOtherTimeModel();
			if(other != -1 && CModelInfo::GetModelInfo(other)->GetRwObject())
				return VIS_INVISIBLE;
		}
	}

	RpAtomic *a = mi->GetFirstAtomicFromDistance(dist);
	if(a){
		if(ent->m_rwObject == nil)
			ent->CreateRwObject();
		assert(ent->m_rwObject);
		RpAtomic *rwobj = (RpAtomic*)ent->m_rwObject;

		// Make sure our atomic uses the right geometry and not
		// that of an atomic for another draw distance.
		if(RpAtomicGetGeometry(a) != RpAtomicGetGeometry(rwobj))
			RpAtomicSetGeometry(rwobj, RpAtomicGetGeometry(a), rpATOMICSAMEBOUNDINGSPHERE); // originally 5 (mistake?)
		mi->IncreaseAlpha();
		if(!ent->IsVisibleComplex() || ent->IsEntityOccluded()){
			mi->m_alpha = 255;
			return VIS_INVISIBLE;
		}

		if(mi->m_alpha != 255){
			CVisibilityPlugins::InsertEntityIntoSortedList(ent, dist);
			ent->bDistanceFade = true;
			return VIS_INVISIBLE;
		}

		if(mi->m_drawLast){
			CVisibilityPlugins::InsertEntityIntoSortedList(ent, dist);
			ent->bDistanceFade = false;
			return VIS_INVISIBLE;
		}
		return VIS_VISIBLE;
	}

	if(mi->m_noFade){
		ent->DeleteRwObject();
		return VIS_INVISIBLE;
	}


	// get faded atomic
	a = mi->GetFirstAtomicFromDistance(dist - FADE_DISTANCE);
	if(a == nil){
		if(ent->bStreamBIGBuilding && dist-STREAM_DISTANCE < mi->GetLodDistance(0) && request){
			return ent->GetIsOnScreen() ? VIS_STREAMME : VIS_INVISIBLE;
		}else{
			ent->DeleteRwObject();
			return VIS_INVISIBLE;
		}
	}

	// Fade...
	if(ent->m_rwObject == nil)
		ent->CreateRwObject();
	assert(ent->m_rwObject);
	RpAtomic *rwobj = (RpAtomic*)ent->m_rwObject;
	if(RpAtomicGetGeometry(a) != RpAtomicGetGeometry(rwobj))
		RpAtomicSetGeometry(rwobj, RpAtomicGetGeometry(a), rpATOMICSAMEBOUNDINGSPHERE); // originally 5 (mistake?)
	mi->IncreaseAlpha();
	if(!ent->IsVisibleComplex() || ent->IsEntityOccluded()){
		mi->m_alpha = 255;
		return VIS_INVISIBLE;
	}
	CVisibilityPlugins::InsertEntityIntoSortedList(ent, dist);
	ent->bDistanceFade = true;
	return VIS_INVISIBLE;
}

void
CRenderer::ConstructRenderList(void)
{
	COcclusion::ProcessBeforeRendering();

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

	// unused
	static CVector prevPos;
	static CVector prevFwd;
	static bool smallMovement;
	smallMovement = (TheCamera.GetPosition() - prevPos).MagnitudeSqr() < SQR(4.0f) &&
		DotProduct(TheCamera.GetForward(), prevFwd) > 0.98f;
	prevPos = TheCamera.GetPosition();
	prevFwd = TheCamera.GetForward();

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
#ifdef GTA_TRAIN
		CVehicle *train = FindPlayerTrain();
		if(train && train->GetPosition().z < 0.0f){
			poly[0].x = CWorld::GetSectorX(vectors[CORNER_CAM].x);
			poly[0].y = CWorld::GetSectorY(vectors[CORNER_CAM].y);
			poly[1].x = CWorld::GetSectorX(vectors[CORNER_LOD_LEFT].x);
			poly[1].y = CWorld::GetSectorY(vectors[CORNER_LOD_LEFT].y);
			poly[2].x = CWorld::GetSectorX(vectors[CORNER_LOD_RIGHT].x);
			poly[2].y = CWorld::GetSectorY(vectors[CORNER_LOD_RIGHT].y);
			ScanSectorPoly(poly, 3, ScanSectorList_Subway);
		}else
#endif
		{
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

			ScanBigBuildingList(CWorld::GetBigBuildingList(CGame::currLevel));
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
	ms_vecCameraPosition = TheCamera.GetPosition();

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
	int vis;

	int f = CTimer::GetFrameCounter() & 3;
	for(node = list.first; node; node = node->next){
		ent = (CEntity*)node->item;
		if(ent->bOffscreen || (ent->m_randomSeed&3) != f){
			ent->bOffscreen = true;
			vis = SetupBigBuildingVisibility(ent);
		}else
			vis = VIS_VISIBLE;
		switch(vis){
		case VIS_VISIBLE:
			ms_aVisibleEntityPtrs[ms_nNoOfVisibleEntities++] = ent;
			ent->bOffscreen = false;
			break;
		case VIS_STREAMME:
			if(!CStreaming::ms_disableStreaming)
				CStreaming::RequestModel(ent->GetModelIndex(), 0);
			break;
		}
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
			ent->bOffscreen = false;

			switch(SetupEntityVisibility(ent)){
			case VIS_VISIBLE:
				ms_aVisibleEntityPtrs[ms_nNoOfVisibleEntities++] = ent;
				break;
			case VIS_INVISIBLE:
				if(!IsGlass(ent->GetModelIndex()))
					break;
				// fall through
			case VIS_OFFSCREEN:
				ent->bOffscreen = true;
				dx = ms_vecCameraPosition.x - ent->GetPosition().x;
				dy = ms_vecCameraPosition.y - ent->GetPosition().y;
				if(dx > -30.0f && dx < 30.0f &&
				   dy > -30.0f && dy < 30.0f &&
				   ms_nNoOfInVisibleEntities < NUMINVISIBLEENTITIES - 1)
					ms_aInVisibleEntityPtrs[ms_nNoOfInVisibleEntities++] = ent;
				break;
			case VIS_STREAMME:
				if(!CStreaming::ms_disableStreaming)
					if(!m_loadingPriority || CStreaming::ms_numModelsRequested < 10)
						CStreaming::RequestModel(ent->GetModelIndex(), 0);
				break;
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
			ent->bOffscreen = false;

			switch(SetupEntityVisibility(ent)){
			case VIS_VISIBLE:
				ms_aVisibleEntityPtrs[ms_nNoOfVisibleEntities++] = ent;
				break;
			case VIS_INVISIBLE:
				if(!IsGlass(ent->GetModelIndex()))
					break;
				// fall through
			case VIS_OFFSCREEN:
				ent->bOffscreen = true;
				dx = ms_vecCameraPosition.x - ent->GetPosition().x;
				dy = ms_vecCameraPosition.y - ent->GetPosition().y;
				if(dx > -30.0f && dx < 30.0f &&
				   dy > -30.0f && dy < 30.0f &&
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
		}
	}
}

#ifdef GTA_TRAIN
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
			ent->bOffscreen = false;
			switch(SetupEntityVisibility(ent)){
			case VIS_VISIBLE:
				ms_aVisibleEntityPtrs[ms_nNoOfVisibleEntities++] = ent;
				break;
			case VIS_OFFSCREEN:
				ent->bOffscreen = true;
				dx = ms_vecCameraPosition.x - ent->GetPosition().x;
				dy = ms_vecCameraPosition.y - ent->GetPosition().y;
				if(dx > -30.0f && dx < 30.0f &&
				   dy > -30.0f && dy < 30.0f &&
				   ms_nNoOfInVisibleEntities < NUMINVISIBLEENTITIES - 1)
					ms_aInVisibleEntityPtrs[ms_nNoOfInVisibleEntities++] = ent;
				break;
			}
		}
	}
}
#endif

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
			if(ShouldModelBeStreamed(ent, ms_vecCameraPosition))
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
CRenderer::ShouldModelBeStreamed(CEntity *ent, const CVector &campos)
{
	if(!IsAreaVisible(ent->m_area))
		return false;
	CTimeModelInfo *mi = (CTimeModelInfo *)CModelInfo::GetModelInfo(ent->GetModelIndex());
	if(mi->GetModelType() == MITYPE_TIME)
		if(!CClock::GetIsTimeInRange(mi->GetTimeOn(), mi->GetTimeOff()))
			return false;
	float dist = (ent->GetPosition() - campos).Magnitude();
	if(mi->m_noFade)
		return dist - STREAM_DISTANCE < mi->GetLargestLodDistance();
	else
		return dist - FADE_DISTANCE - STREAM_DISTANCE < mi->GetLargestLodDistance();
}

void
CRenderer::RemoveVehiclePedLights(CEntity *ent, bool reset)
{
	if(!ent->bRenderScorched){
		CPointLights::RemoveLightsAffectingObject();
		if(reset)
			ReSetAmbientAndDirectionalColours();
	}
	SetAmbientColours();
	DeActivateDirectional();
}
