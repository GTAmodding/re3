#include "common.h"

#include "RwHelper.h"
#include "templates.h"
#include "main.h"
#include "Entity.h"
#include "ModelInfo.h"
#include "Lights.h"
#include "Renderer.h"
#include "Camera.h"
#include "VisibilityPlugins.h"
#include "World.h"
#include "custompipes.h"
#include "MemoryHeap.h"

CLinkList<CVisibilityPlugins::AlphaObjectInfo> CVisibilityPlugins::m_alphaList;
CLinkList<CVisibilityPlugins::AlphaObjectInfo> CVisibilityPlugins::m_alphaEntityList;
#ifdef NEW_RENDERER
CLinkList<CVisibilityPlugins::AlphaObjectInfo> CVisibilityPlugins::m_alphaBuildingList;
#endif

int32 CVisibilityPlugins::ms_atomicPluginOffset = -1;
int32 CVisibilityPlugins::ms_framePluginOffset = -1;
int32 CVisibilityPlugins::ms_clumpPluginOffset = -1;

RwCamera *CVisibilityPlugins::ms_pCamera;
RwV3d *CVisibilityPlugins::ms_pCameraPosn;
float CVisibilityPlugins::ms_cullCompsDist;
float CVisibilityPlugins::ms_vehicleLod0Dist;
float CVisibilityPlugins::ms_vehicleLod1Dist;
float CVisibilityPlugins::ms_vehicleFadeDist;
float CVisibilityPlugins::ms_bigVehicleLod0Dist;
float CVisibilityPlugins::ms_bigVehicleLod1Dist;
float CVisibilityPlugins::ms_pedLod0Dist;
float CVisibilityPlugins::ms_pedLod1Dist;
float CVisibilityPlugins::ms_pedFadeDist;

#ifdef GTA_PS2	// maybe something else?
// if wanted, delete the original geometry data after rendering
// and only keep the instanced data
bool
rpDefaultGeometryInstance(RpGeometry *geo, void *atomic, int del)
{
#if THIS_IS_COMPATIBLE_WITH_GTA3_RW31
	if(RpGeometryGetNumMorphTargets(geo) != 1)
		return false;

	// this needs R*'s modification that geometry data is
	// allocated separately from the geometry itself
	geo->instanceFlags = rpGEOMETRYINSTANCE;
	AtomicDefaultRenderCallBack((RpAtomic*)atomic);

	if(!del)
		return true;

	// New mesh without indices
	RpMeshHeader *newheader = _rpMeshHeaderCreate(sizeof(RpMesh)*geo->mesh->numMeshes + sizeof(RpMeshHeader));
	newheader->numMeshes = geo->mesh->numMeshes;
	newheader->serialNum = 1;
	newheader->totalIndicesInMesh = 0;
	newheader->firstMeshOffset = 0;
	RpMesh *oldmesh = (RpMesh*)(geo->mesh+1);
	RpMesh *newmesh = (RpMesh*)(newheader+1);
	for(int i = 0; i < geo->mesh->numMeshes; i++){
		newmesh[i].indices = nil;
		newmesh[i].numIndices = 0;
		newmesh[i].material = oldmesh[i].material;
	}

	geo->refCount++;
	RpGeometryLock(geo, rpGEOMETRYLOCKPOLYGONS | rpGEOMETRYLOCKVERTICES |
		rpGEOMETRYLOCKNORMALS | rpGEOMETRYLOCKPRELIGHT |
		rpGEOMETRYLOCKTEXCOORDS1 | rpGEOMETRYLOCKTEXCOORDS2);

	// vertices and normals
	RpMorphTarget *mt = RpGeometryGetMorphTarget(geo, 0);
	if(mt->verts){
		RwFree(mt->verts);
		mt->verts = nil;
		mt->normals = nil;
	}
	geo->numVertices = 0;

	// triangles
	for(int i = 0; i < RpGeometryGetNumTriangles(geo); i++){
		if(RpGeometryGetTriangles(geo)->matIndex == -1)
			continue;
		RpMaterialDestroy(_rpMaterialListGetMaterial(&geo->matList, RpGeometryGetTriangles(geo)->matIndex));
	}
	if(RpGeometryGetTriangles(geo)){
		RwFree(RpGeometryGetTriangles(geo));
		geo->triangles = nil;
		geo->numTriangles = 0;
	}

	// tex coords
	if(RpGeometryGetVertexTexCoords(geo, 1)){
		RwFree(RpGeometryGetVertexTexCoords(geo, 1));
		geo->texCoords[1] = nil;
	}
	if(RpGeometryGetVertexTexCoords(geo, 0)){
		RwFree(RpGeometryGetVertexTexCoords(geo, 0));
		geo->texCoords[0] = nil;
	}

	// vertex colors
	if(RpGeometryGetPreLightColors(geo)){
		RwFree(RpGeometryGetPreLightColors(geo));
		geo->preLitLum = nil;
	}

	RpGeometryUnlock(geo);

	geo->instanceFlags = rpGEOMETRYPERSISTENT;
	// BUG? don't we have to free the old mesh?
	geo->mesh = newheader;
	geo->refCount--;
#else
	// We can do something for librw here actually, maybe later
	AtomicDefaultRenderCallBack((RpAtomic*)atomic);
#endif

	return true;
}

RpAtomic*
PreInstanceRenderCB(RpAtomic *atomic)
{
	RpGeometry *geo = RpAtomicGetGeometry(atomic);
	if(RpGeometryGetTriangles(geo)){
		PUSH_MEMID(MEMID_STREAM_MODELS);
		rpDefaultGeometryInstance(geo, atomic, 1);
		POP_MEMID();
	}else
		AtomicDefaultRenderCallBack(atomic);
	return atomic;
}
#define RENDERCALLBACK PreInstanceRenderCB
#else
RpAtomic*
DefaultRenderCB_pushid(RpAtomic *atomic)
{
	PUSH_MEMID(MEMID_STREAM_MODELS);
	AtomicDefaultRenderCallBack(atomic);
	POP_MEMID();
	return atomic;
}
#define RENDERCALLBACK DefaultRenderCB_pushid
#endif

void
CVisibilityPlugins::Initialise(void)
{
	m_alphaList.Init(NUMALPHALIST);
	m_alphaList.head.item.sort = 0.0f;
	m_alphaList.tail.item.sort = 100000000.0f;
#ifdef ASPECT_RATIO_SCALE
	// default 150 if not enough for bigger FOVs
	m_alphaEntityList.Init(NUMALPHAENTITYLIST * 3);
#else
	m_alphaEntityList.Init(NUMALPHAENTITYLIST);
#endif // ASPECT_RATIO_SCALE
	m_alphaEntityList.head.item.sort = 0.0f;
	m_alphaEntityList.tail.item.sort = 100000000.0f;

#ifdef NEW_RENDERER
	m_alphaBuildingList.Init(NUMALPHAENTITYLIST);
	m_alphaBuildingList.head.item.sort = 0.0f;
	m_alphaBuildingList.tail.item.sort = 100000000.0f;
#endif
}

void
CVisibilityPlugins::Shutdown(void)
{
	m_alphaList.Shutdown();
	m_alphaEntityList.Shutdown();
#ifdef NEW_RENDERER
	m_alphaBuildingList.Shutdown();
#endif
}

void
CVisibilityPlugins::InitAlphaEntityList(void)
{
	m_alphaEntityList.Clear();
#ifdef NEW_RENDERER
	m_alphaBuildingList.Clear();
#endif
}

bool
CVisibilityPlugins::InsertEntityIntoSortedList(CEntity *e, float dist)
{
#ifdef FIX_BUGS
	if (!e->m_rwObject) return true;
#endif

	AlphaObjectInfo item;
	item.entity = e;
	item.sort = dist;
#ifdef NEW_RENDERER
	if(gbNewRenderer && e->IsBuilding())
		return !!m_alphaBuildingList.InsertSorted(item);
#endif
	bool ret = !!m_alphaEntityList.InsertSorted(item);
//	if(!ret)
//		printf("list full %d\n", m_alphaEntityList.Count());
	return ret;
}

void
CVisibilityPlugins::InitAlphaAtomicList(void)
{
	m_alphaList.Clear();
}

bool
CVisibilityPlugins::InsertAtomicIntoSortedList(RpAtomic *a, float dist)
{
	AlphaObjectInfo item;
	item.atomic = a;
	item.sort = dist;
	bool ret = !!m_alphaList.InsertSorted(item);
//	if(!ret)
//		printf("list full %d\n", m_alphaList.Count());
	return ret;
}

// can't increase this yet unfortunately...
// probably have to fix fading for this so material alpha isn't overwritten
#define VEHICLE_LODDIST_MULTIPLIER (TheCamera.GenerationDistMultiplier)

void
CVisibilityPlugins::SetRenderWareCamera(RwCamera *camera)
{
	ms_pCamera = camera;
	ms_pCameraPosn = RwMatrixGetPos(RwFrameGetMatrix(RwCameraGetFrame(camera)));

	if(TheCamera.Cams[TheCamera.ActiveCam].Mode == CCam::MODE_TOPDOWN ||
	   TheCamera.Cams[TheCamera.ActiveCam].Mode == CCam::MODE_TOP_DOWN_PED)
		ms_cullCompsDist = 1000000.0f;
	else
		ms_cullCompsDist = sq(TheCamera.LODDistMultiplier * 20.0f);

        ms_vehicleLod0Dist = sq(70.0f * VEHICLE_LODDIST_MULTIPLIER);
        ms_vehicleLod1Dist = sq(90.0f * VEHICLE_LODDIST_MULTIPLIER);
        ms_vehicleFadeDist = sq(100.0f * VEHICLE_LODDIST_MULTIPLIER);
        ms_bigVehicleLod0Dist = sq(60.0f * VEHICLE_LODDIST_MULTIPLIER);
        ms_bigVehicleLod1Dist = sq(150.0f * VEHICLE_LODDIST_MULTIPLIER);
        ms_pedLod0Dist = sq(25.0f * TheCamera.LODDistMultiplier);
        ms_pedLod1Dist = sq(60.0f * TheCamera.LODDistMultiplier);
        ms_pedFadeDist = sq(70.0f * TheCamera.LODDistMultiplier);
}

RpMaterial*
SetAlphaCB(RpMaterial *material, void *data)
{
	((RwRGBA*)RpMaterialGetColor(material))->alpha = (uint8)(uintptr)data;
	return material;
}

RpMaterial*
SetTextureCB(RpMaterial *material, void *data)
{
	RpMaterialSetTexture(material, (RwTexture*)data);
	return material;
}

void
CVisibilityPlugins::RenderAlphaAtomics(void)
{
	CLink<AlphaObjectInfo> *node;
	for(node = m_alphaList.tail.prev;
	    node != &m_alphaList.head;
	    node = node->prev)
		RENDERCALLBACK(node->item.atomic);
}

void
CVisibilityPlugins::RenderFadingEntities(void)
{
	CLink<AlphaObjectInfo> *node;
	CSimpleModelInfo *mi;
	for(node = m_alphaEntityList.tail.prev;
	    node != &m_alphaEntityList.head;
	    node = node->prev){
		CEntity *e = node->item.entity;
		if(e->m_rwObject == nil)
			continue;
#ifdef EXTENDED_PIPELINES
		if(CustomPipes::bRenderingEnvMap && (e->IsPed() || e->IsVehicle()))
			continue;
#endif
		mi = (CSimpleModelInfo *)CModelInfo::GetModelInfo(e->GetModelIndex());
#ifdef FIX_BUGS
		if(mi->GetModelType() == MITYPE_SIMPLE && mi->m_noZwrite)
#else
		if(mi->m_noZwrite)
#endif
			RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, FALSE);
#ifdef EXTRA_MODEL_FLAGS
		else if(mi->m_bIsTree)
			SetAlphaRef(128);
#endif

		if(e->bDistanceFade){
			DeActivateDirectional();
			SetAmbientColours();
			e->bImBeingRendered = true;
			RenderFadingAtomic((RpAtomic*)e->m_rwObject, node->item.sort);
			e->bImBeingRendered = false;
		}else
			CRenderer::RenderOneNonRoad(e);

#ifdef EXTRA_MODEL_FLAGS
		if(mi->m_bIsTree)
			SetAlphaRef(2);
#endif
#ifdef FIX_BUGS
		if(mi->GetModelType() == MITYPE_SIMPLE && mi->m_noZwrite)
#else
		if(mi->m_noZwrite)
#endif
			RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)TRUE);
	}
}

RpAtomic*
CVisibilityPlugins::RenderWheelAtomicCB(RpAtomic *atomic)
{
	RpAtomic *lodatm;
	RwMatrix *m;
	RwV3d view;
	float len;
	CSimpleModelInfo *mi;

	mi = GetAtomicModelInfo(atomic);
	m = RwFrameGetLTM(RpAtomicGetFrame(atomic));
	RwV3dSub(&view, RwMatrixGetPos(m), ms_pCameraPosn);
	len = RwV3dLength(&view);
#ifdef FIX_BUGS
	// from VC
	lodatm = mi->GetAtomicFromDistance(len * TheCamera.LODDistMultiplier / VEHICLE_LODDIST_MULTIPLIER);
#else
	lodatm = mi->GetAtomicFromDistance(len);
#endif
	if(lodatm){
		if(RpAtomicGetGeometry(lodatm) != RpAtomicGetGeometry(atomic))
			RpAtomicSetGeometry(atomic, RpAtomicGetGeometry(lodatm), rpATOMICSAMEBOUNDINGSPHERE);
		RENDERCALLBACK(atomic);
	}
	return atomic;
}

RpAtomic*
CVisibilityPlugins::RenderObjNormalAtomic(RpAtomic *atomic)
{
	RwMatrix *m;
	RwV3d view;
	float len;

	m = RwFrameGetLTM(RpAtomicGetFrame(atomic));
	RwV3dSub(&view, RwMatrixGetPos(m), ms_pCameraPosn);
	len = RwV3dLength(&view);
	if(RwV3dDotProduct(&view, RwMatrixGetUp(m)) < -0.3f*len && len > 8.0f)
		return atomic;
	RENDERCALLBACK(atomic);
	return atomic;
}

RpAtomic*
CVisibilityPlugins::RenderAlphaAtomic(RpAtomic *atomic, int alpha)
{
	RpGeometry *geo;
	uint32 flags;

	geo = RpAtomicGetGeometry(atomic);
	flags = RpGeometryGetFlags(geo);
	RpGeometrySetFlags(geo, flags | rpGEOMETRYMODULATEMATERIALCOLOR);
	RpGeometryForAllMaterials(geo, SetAlphaCB, (void*)alpha);
	RENDERCALLBACK(atomic);
	RpGeometryForAllMaterials(geo, SetAlphaCB, (void*)255);
	RpGeometrySetFlags(geo, flags);
	return atomic;
}

RpAtomic*
CVisibilityPlugins::RenderFadingAtomic(RpAtomic *atomic, float camdist)
{
	RpAtomic *lodatm;
	float fadefactor;
	uint32 alpha;
	CSimpleModelInfo *mi;

	mi = GetAtomicModelInfo(atomic);
	lodatm = mi->GetAtomicFromDistance(camdist - FADE_DISTANCE);
	if(mi->m_additive){
		RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDONE);
		RENDERCALLBACK(atomic);
		RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDINVSRCALPHA);
	}else{
		fadefactor = (mi->GetLargestLodDistance() - (camdist - FADE_DISTANCE))/FADE_DISTANCE;
		if(fadefactor > 1.0f)
			fadefactor = 1.0f;
		alpha = mi->m_alpha * fadefactor;
		if(alpha == 255)
			RENDERCALLBACK(atomic);
		else{
			RpGeometry *geo = RpAtomicGetGeometry(lodatm);
			uint32 flags = RpGeometryGetFlags(geo);
			RpGeometrySetFlags(geo, flags | rpGEOMETRYMODULATEMATERIALCOLOR);
			RpGeometryForAllMaterials(geo, SetAlphaCB, (void*)alpha);
			if(geo != RpAtomicGetGeometry(atomic))
				RpAtomicSetGeometry(atomic, geo, rpATOMICSAMEBOUNDINGSPHERE); // originally 5 (mistake?)
			RENDERCALLBACK(atomic);
			RpGeometryForAllMaterials(geo, SetAlphaCB, (void*)255);
			RpGeometrySetFlags(geo, flags);
		}
	}
	return atomic;
}



RpAtomic*
CVisibilityPlugins::RenderVehicleHiDetailCB(RpAtomic *atomic)
{
	RwFrame *clumpframe;
	float distsq, dot;
	uint32 flags;

	clumpframe = RpClumpGetFrame(RpAtomicGetClump(atomic));
	distsq = GetDistanceSquaredFromCamera(clumpframe);
	if(distsq < ms_vehicleLod0Dist){
		flags = GetAtomicId(atomic);
		if(distsq > ms_cullCompsDist && (flags & ATOMIC_FLAG_NOCULL) == 0){
			dot = GetDotProductWithCameraVector(RwFrameGetLTM(RpAtomicGetFrame(atomic)),
				RwFrameGetLTM(clumpframe), flags);
			if(dot > 0.0f && ((flags & ATOMIC_FLAG_ANGLECULL) || 0.1f*distsq < dot*dot))
				return atomic;
		}
		RENDERCALLBACK(atomic);
	}
	return atomic;
}

RpAtomic*
CVisibilityPlugins::RenderVehicleHiDetailAlphaCB(RpAtomic *atomic)
{
	RwFrame *clumpframe;
	float distsq, dot;
	uint32 flags;

	clumpframe = RpClumpGetFrame(RpAtomicGetClump(atomic));
	distsq = GetDistanceSquaredFromCamera(clumpframe);
	if(distsq < ms_vehicleLod0Dist){
		flags = GetAtomicId(atomic);
		dot = GetDotProductWithCameraVector(RwFrameGetLTM(RpAtomicGetFrame(atomic)),
			RwFrameGetLTM(clumpframe), flags);
		if(distsq > ms_cullCompsDist && (flags & ATOMIC_FLAG_NOCULL) == 0)
			if(dot > 0.0f && ((flags & ATOMIC_FLAG_ANGLECULL) || 0.1f*distsq < dot*dot))
				return atomic;

		if(flags & ATOMIC_FLAG_DRAWLAST){
			// sort before clump
			if(!InsertAtomicIntoSortedList(atomic, distsq - 0.0001f))
				RENDERCALLBACK(atomic);
		}else{
			if(!InsertAtomicIntoSortedList(atomic, distsq + dot))
				RENDERCALLBACK(atomic);
		}
	}
	return atomic;
}

RpAtomic*
CVisibilityPlugins::RenderVehicleHiDetailCB_BigVehicle(RpAtomic *atomic)
{
	RwFrame *clumpframe;
	float distsq, dot;
	uint32 flags;

	clumpframe = RpClumpGetFrame(RpAtomicGetClump(atomic));
	distsq = GetDistanceSquaredFromCamera(clumpframe);
	if(distsq < ms_bigVehicleLod0Dist){
		flags = GetAtomicId(atomic);
		if(distsq > ms_cullCompsDist && (flags & ATOMIC_FLAG_NOCULL) == 0){
			dot = GetDotProductWithCameraVector(RwFrameGetLTM(RpAtomicGetFrame(atomic)),
				RwFrameGetLTM(clumpframe), flags);
			if(dot > 0.0f)
				return atomic;
		}
		RENDERCALLBACK(atomic);
	}
	return atomic;
}

RpAtomic*
CVisibilityPlugins::RenderVehicleHiDetailAlphaCB_BigVehicle(RpAtomic *atomic)
{
	RwFrame *clumpframe;
	float distsq, dot;
	uint32 flags;

	clumpframe = RpClumpGetFrame(RpAtomicGetClump(atomic));
	distsq = GetDistanceSquaredFromCamera(clumpframe);
	if(distsq < ms_bigVehicleLod0Dist){
		flags = GetAtomicId(atomic);
		dot = GetDotProductWithCameraVector(RwFrameGetLTM(RpAtomicGetFrame(atomic)),
			RwFrameGetLTM(clumpframe), flags);
		if(dot > 0.0f)
			if(distsq > ms_cullCompsDist && (flags & ATOMIC_FLAG_NOCULL) == 0)
				return atomic;

		if(!InsertAtomicIntoSortedList(atomic, distsq + dot))
			RENDERCALLBACK(atomic);
	}
	return atomic;
}

RpAtomic*
CVisibilityPlugins::RenderVehicleHiDetailCB_Boat(RpAtomic *atomic)
{
	RwFrame *clumpframe;
	float distsq;

	clumpframe = RpClumpGetFrame(RpAtomicGetClump(atomic));
	distsq = GetDistanceSquaredFromCamera(clumpframe);
	if(distsq < ms_bigVehicleLod1Dist)
		RENDERCALLBACK(atomic);
	return atomic;
}

RpAtomic*
CVisibilityPlugins::RenderVehicleLowDetailCB_BigVehicle(RpAtomic *atomic)
{
	RwFrame *clumpframe;
	float distsq, dot;
	uint32 flags;

	clumpframe = RpClumpGetFrame(RpAtomicGetClump(atomic));
	distsq = GetDistanceSquaredFromCamera(clumpframe);
	if(distsq >= ms_bigVehicleLod0Dist &&
	   distsq < ms_bigVehicleLod1Dist){
		flags = GetAtomicId(atomic);
		if(distsq > ms_cullCompsDist && (flags & ATOMIC_FLAG_NOCULL) == 0){
			dot = GetDotProductWithCameraVector(RwFrameGetLTM(RpAtomicGetFrame(atomic)),
				RwFrameGetLTM(clumpframe), flags);
			if(dot > 0.0f)
				return atomic;
		}
		RENDERCALLBACK(atomic);
	}
	return atomic;
}

RpAtomic*
CVisibilityPlugins::RenderVehicleLowDetailAlphaCB_BigVehicle(RpAtomic *atomic)
{
	RwFrame *clumpframe;
	float distsq, dot;
	uint32 flags;

	clumpframe = RpClumpGetFrame(RpAtomicGetClump(atomic));
	distsq = GetDistanceSquaredFromCamera(clumpframe);
	if(distsq >= ms_bigVehicleLod0Dist &&
	   distsq < ms_bigVehicleLod1Dist){
		flags = GetAtomicId(atomic);
		dot = GetDotProductWithCameraVector(RwFrameGetLTM(RpAtomicGetFrame(atomic)),
			RwFrameGetLTM(clumpframe), flags);
		if(dot > 0.0f)
			if(distsq > ms_cullCompsDist && (flags & ATOMIC_FLAG_NOCULL) == 0)
				return atomic;

		if(!InsertAtomicIntoSortedList(atomic, distsq + dot))
			RENDERCALLBACK(atomic);
	}
	return atomic;
}

RpAtomic*
CVisibilityPlugins::RenderVehicleReallyLowDetailCB(RpAtomic *atomic)
{
	RpClump *clump;
	float dist;
	int32 alpha;

	clump = RpAtomicGetClump(atomic);
	dist = GetDistanceSquaredFromCamera(RpClumpGetFrame(clump));
	if(dist >= ms_vehicleLod0Dist){
		alpha = GetClumpAlpha(clump);
		if(alpha == 255)
			RENDERCALLBACK(atomic);
		else
			RenderAlphaAtomic(atomic, alpha);
	}
	return atomic;

}

RpAtomic*
CVisibilityPlugins::RenderVehicleReallyLowDetailCB_BigVehicle(RpAtomic *atomic)
{
	RwFrame *clumpframe;
	float distsq;

	clumpframe = RpClumpGetFrame(RpAtomicGetClump(atomic));
	distsq = GetDistanceSquaredFromCamera(clumpframe);
	if(distsq >= ms_bigVehicleLod1Dist)
		RENDERCALLBACK(atomic);
	return atomic;
}

RpAtomic*
CVisibilityPlugins::RenderTrainHiDetailCB(RpAtomic *atomic)
{
	RwFrame *clumpframe;
	float distsq, dot;
	uint32 flags;

	clumpframe = RpClumpGetFrame(RpAtomicGetClump(atomic));
	distsq = GetDistanceSquaredFromCamera(clumpframe);
	if(distsq < ms_bigVehicleLod1Dist){
		flags = GetAtomicId(atomic);
		if(distsq > ms_cullCompsDist && (flags & ATOMIC_FLAG_NOCULL) == 0){
			dot = GetDotProductWithCameraVector(RwFrameGetLTM(RpAtomicGetFrame(atomic)),
				RwFrameGetLTM(clumpframe), flags);
			if(dot > 0.0f && ((flags & ATOMIC_FLAG_ANGLECULL) || 0.1f*distsq < dot*dot))
				return atomic;
		}
		RENDERCALLBACK(atomic);
	}
	return atomic;
}

RpAtomic*
CVisibilityPlugins::RenderTrainHiDetailAlphaCB(RpAtomic *atomic)
{
	RwFrame *clumpframe;
	float distsq, dot;
	uint32 flags;

	clumpframe = RpClumpGetFrame(RpAtomicGetClump(atomic));
	distsq = GetDistanceSquaredFromCamera(clumpframe);
	if(distsq < ms_bigVehicleLod1Dist){
		flags = GetAtomicId(atomic);
		dot = GetDotProductWithCameraVector(RwFrameGetLTM(RpAtomicGetFrame(atomic)),
			RwFrameGetLTM(clumpframe), flags);
		if(distsq > ms_cullCompsDist && (flags & ATOMIC_FLAG_NOCULL) == 0)
			if(dot > 0.0f && ((flags & ATOMIC_FLAG_ANGLECULL) || 0.1f*distsq < dot*dot))
				return atomic;

		if(flags & ATOMIC_FLAG_DRAWLAST){
			// sort before clump
			if(!InsertAtomicIntoSortedList(atomic, distsq - 0.0001f))
				RENDERCALLBACK(atomic);
		}else{
			if(!InsertAtomicIntoSortedList(atomic, distsq + dot))
				RENDERCALLBACK(atomic);
		}
	}
	return atomic;
}

RpAtomic*
CVisibilityPlugins::RenderPlayerCB(RpAtomic *atomic)
{
	if(CWorld::Players[0].m_pSkinTexture)
		RpGeometryForAllMaterials(RpAtomicGetGeometry(atomic), SetTextureCB, CWorld::Players[0].m_pSkinTexture);
	RENDERCALLBACK(atomic);
	return atomic;
}

RpAtomic*
CVisibilityPlugins::RenderPedLowDetailCB(RpAtomic *atomic)
{
	RpClump *clump;
	float dist;
	int32 alpha;

	clump = RpAtomicGetClump(atomic);
	dist = GetDistanceSquaredFromCamera(RpClumpGetFrame(clump));
	if(dist >= ms_pedLod0Dist){
		alpha = GetClumpAlpha(clump);
		if(alpha == 255)
			RENDERCALLBACK(atomic);
		else
			RenderAlphaAtomic(atomic, alpha);
	}
	return atomic;
}

RpAtomic*
CVisibilityPlugins::RenderPedHiDetailCB(RpAtomic *atomic)
{
	RpClump *clump;
	float dist;
	int32 alpha;

	clump = RpAtomicGetClump(atomic);
	dist = GetDistanceSquaredFromCamera(RpClumpGetFrame(clump));
	if(dist < ms_pedLod0Dist){
		alpha = GetClumpAlpha(clump);
		if(alpha == 255)
			RENDERCALLBACK(atomic);
		else
			RenderAlphaAtomic(atomic, alpha);
	}
	return atomic;
}

// This is needed for peds with only one clump, i.e. skinned models
// strangely even the xbox version has no such thing
RpAtomic*
CVisibilityPlugins::RenderPedCB(RpAtomic *atomic)
{
	int32 alpha;
	RwV3d cam2atm;

	RwV3dSub(&cam2atm, &RwFrameGetLTM(RpAtomicGetFrame(atomic))->pos, ms_pCameraPosn);
	if(RwV3dDotProduct(&cam2atm, &cam2atm) < ms_pedLod1Dist){
		alpha = GetClumpAlpha(RpAtomicGetClump(atomic));
		if(alpha == 255)
			RENDERCALLBACK(atomic);
		else
			RenderAlphaAtomic(atomic, alpha);
	}
	return atomic;
}

float
CVisibilityPlugins::GetDistanceSquaredFromCamera(RwFrame *frame)
{
	RwMatrix *m;
	RwV3d dist;
	m = RwFrameGetLTM(frame);
	RwV3dSub(&dist, RwMatrixGetPos(m), ms_pCameraPosn);
	return RwV3dDotProduct(&dist, &dist);
}

float
CVisibilityPlugins::GetDotProductWithCameraVector(RwMatrix *atomicMat, RwMatrix *clumpMat, uint32 flags)
{
	RwV3d dist;
	float dot, dotdoor;

	// Vehicle forward is the y axis (RwMatrix.up)
	// Vehicle right is the x axis (RwMatrix.right)

	RwV3dSub(&dist, RwMatrixGetPos(atomicMat), ms_pCameraPosn);
	// forward/backward facing
	if(flags & (ATOMIC_FLAG_FRONT | ATOMIC_FLAG_REAR))
		dot = RwV3dDotProduct(&dist, RwMatrixGetUp(clumpMat));
	// left/right facing
	else if(flags & (ATOMIC_FLAG_LEFT | ATOMIC_FLAG_RIGHT))
		dot = RwV3dDotProduct(&dist, RwMatrixGetRight(clumpMat));
	else
		dot = 0.0f;
	if(flags & (ATOMIC_FLAG_LEFT | ATOMIC_FLAG_REAR))
		dot = -dot;

	if(flags & (ATOMIC_FLAG_REARDOOR | ATOMIC_FLAG_FRONTDOOR)){
		if(flags & ATOMIC_FLAG_REARDOOR)
			dotdoor = -RwV3dDotProduct(&dist, RwMatrixGetUp(clumpMat));
		else if(flags & ATOMIC_FLAG_FRONTDOOR)
			dotdoor = RwV3dDotProduct(&dist, RwMatrixGetUp(clumpMat));
		else
			dotdoor = 0.0f;

		if(dot < 0.0f && dotdoor < 0.0f)
			dot += dotdoor;
		if(dot > 0.0f && dotdoor > 0.0f)
			dot += dotdoor;
	}

	return dot;
}

/* These are all unused */

bool
CVisibilityPlugins::DefaultVisibilityCB(RpClump *clump)
{
	return true;
}

bool
CVisibilityPlugins::MloVisibilityCB(RpClump *clump)
{
	RwFrame *frame = RpClumpGetFrame(clump);
	CMloModelInfo *modelInfo = (CMloModelInfo*)GetFrameHierarchyId(frame);
	if (sq(modelInfo->field_34) < GetDistanceSquaredFromCamera(frame))
		return false;
	return CVisibilityPlugins::FrustumSphereCB(clump);
}

bool
CVisibilityPlugins::FrustumSphereCB(RpClump *clump)
{
	RwSphere sphere;
	RwFrame *frame = RpClumpGetFrame(clump);

	CClumpModelInfo *modelInfo = (CClumpModelInfo*)GetFrameHierarchyId(frame);
	sphere.radius = modelInfo->GetColModel()->boundingSphere.radius;
	sphere.center.x = modelInfo->GetColModel()->boundingSphere.center.x;
	sphere.center.y = modelInfo->GetColModel()->boundingSphere.center.y;
	sphere.center.z = modelInfo->GetColModel()->boundingSphere.center.z;
	RwV3dTransformPoints(&sphere.center, &sphere.center, 1, RwFrameGetLTM(frame));
	return RwCameraFrustumTestSphere(ms_pCamera, &sphere) != rwSPHEREOUTSIDE;
}

bool
CVisibilityPlugins::VehicleVisibilityCB(RpClump *clump)
{
	if (GetDistanceSquaredFromCamera(RpClumpGetFrame(clump)) <= ms_vehicleLod1Dist)
		return FrustumSphereCB(clump);
	return false;
}

bool
CVisibilityPlugins::VehicleVisibilityCB_BigVehicle(RpClump *clump)
{
	return FrustumSphereCB(clump);
}




//
// RW Plugins
//

enum
{
	ID_VISIBILITYATOMIC = MAKECHUNKID(rwVENDORID_ROCKSTAR, 0x00),
	ID_VISIBILITYCLUMP  = MAKECHUNKID(rwVENDORID_ROCKSTAR, 0x01),
	ID_VISIBILITYFRAME  = MAKECHUNKID(rwVENDORID_ROCKSTAR, 0x02),
};

bool
CVisibilityPlugins::PluginAttach(void)
{
	ms_atomicPluginOffset = RpAtomicRegisterPlugin(sizeof(AtomicExt),
		ID_VISIBILITYATOMIC,
		AtomicConstructor, AtomicDestructor, AtomicCopyConstructor);

	ms_framePluginOffset = RwFrameRegisterPlugin(sizeof(FrameExt),
		ID_VISIBILITYFRAME,
		FrameConstructor, FrameDestructor, FrameCopyConstructor);

	ms_clumpPluginOffset = RpClumpRegisterPlugin(sizeof(ClumpExt),
		ID_VISIBILITYCLUMP,
		ClumpConstructor, ClumpDestructor, ClumpCopyConstructor);

#if GTA_VERSION <= GTA3_PS2_160
	Initialise();
#endif

	return ms_atomicPluginOffset != -1 && ms_clumpPluginOffset != -1;
}

#define ATOMICEXT(o) (RWPLUGINOFFSET(AtomicExt, o, ms_atomicPluginOffset))
#define FRAMEEXT(o) (RWPLUGINOFFSET(FrameExt, o, ms_framePluginOffset))
#define CLUMPEXT(o) (RWPLUGINOFFSET(ClumpExt, o, ms_clumpPluginOffset))

//
// Atomic
//

void*
CVisibilityPlugins::AtomicConstructor(void *object, int32, int32)
{
	ATOMICEXT(object)->modelInfo = nil;
	return object;
}

void*
CVisibilityPlugins::AtomicDestructor(void *object, int32, int32)
{
	return object;
}

void*
CVisibilityPlugins::AtomicCopyConstructor(void *dst, const void *src, int32, int32)
{
	*ATOMICEXT(dst) = *ATOMICEXT(src);
	return dst;
}

void
CVisibilityPlugins::SetAtomicModelInfo(RpAtomic *atomic,
                                       CSimpleModelInfo *modelInfo)
{
	AtomicExt *ext = ATOMICEXT(atomic);
	ext->modelInfo = modelInfo;
	switch (modelInfo->GetModelType()) {
		case MITYPE_SIMPLE:
		case MITYPE_TIME:
			if(modelInfo->m_normalCull)
				SetAtomicRenderCallback(atomic, RenderObjNormalAtomic);
		default: break;
	}
}

CSimpleModelInfo*
CVisibilityPlugins::GetAtomicModelInfo(RpAtomic *atomic)
{
	return ATOMICEXT(atomic)->modelInfo;
}

void
CVisibilityPlugins::SetAtomicFlag(RpAtomic *atomic, int f)
{
	ATOMICEXT(atomic)->flags |= f;
}

void
CVisibilityPlugins::ClearAtomicFlag(RpAtomic *atomic, int f)
{
	ATOMICEXT(atomic)->flags &= ~f;
}

int
CVisibilityPlugins::GetAtomicId(RpAtomic *atomic)
{
	return ATOMICEXT(atomic)->flags;
}

void
CVisibilityPlugins::SetAtomicRenderCallback(RpAtomic *atomic, RpAtomicCallBackRender cb)
{
	if(cb == nil)
		cb = RENDERCALLBACK;
	RpAtomicSetRenderCallBack(atomic, cb);
}

//
// Frame
//

void*
CVisibilityPlugins::FrameConstructor(void *object, int32, int32)
{
	FRAMEEXT(object)->id = 0;
	return object;
}

void*
CVisibilityPlugins::FrameDestructor(void *object, int32, int32)
{
	return object;
}

void*
CVisibilityPlugins::FrameCopyConstructor(void *dst, const void *src, int32, int32)
{
	*FRAMEEXT(dst) = *FRAMEEXT(src);
	return dst;
}

void
CVisibilityPlugins::SetFrameHierarchyId(RwFrame *frame, intptr id)
{
	FRAMEEXT(frame)->id = id;
}

intptr
CVisibilityPlugins::GetFrameHierarchyId(RwFrame *frame)
{
	return FRAMEEXT(frame)->id;
}


//
// Clump
//

void*
CVisibilityPlugins::ClumpConstructor(void *object, int32, int32)
{
	ClumpExt *ext = CLUMPEXT(object);
	ext->visibilityCB = DefaultVisibilityCB;
	ext->alpha = 0xFF;
	return object;
}

void*
CVisibilityPlugins::ClumpDestructor(void *object, int32, int32)
{
	return object;
}

void*
CVisibilityPlugins::ClumpCopyConstructor(void *dst, const void *src, int32, int32)
{
	CLUMPEXT(dst)->visibilityCB = CLUMPEXT(src)->visibilityCB;
	return dst;
}

void
CVisibilityPlugins::SetClumpModelInfo(RpClump *clump, CClumpModelInfo *modelInfo)
{
	CVehicleModelInfo *vmi;
	SetFrameHierarchyId(RpClumpGetFrame(clump), (intptr)modelInfo);

	// Unused
	switch (modelInfo->GetModelType()) {
	// ignore MLO
	case MITYPE_VEHICLE:
		vmi = (CVehicleModelInfo*)modelInfo;
		if(vmi->m_vehicleType == VEHICLE_TYPE_TRAIN ||
		   vmi->m_vehicleType == VEHICLE_TYPE_HELI ||
		   vmi->m_vehicleType == VEHICLE_TYPE_PLANE)
			CLUMPEXT(clump)->visibilityCB = VehicleVisibilityCB_BigVehicle;
		else
			CLUMPEXT(clump)->visibilityCB = VehicleVisibilityCB;
		break;
	default: break;
	}
}

void
CVisibilityPlugins::SetClumpAlpha(RpClump *clump, int alpha)
{
	CLUMPEXT(clump)->alpha = alpha;
}

int
CVisibilityPlugins::GetClumpAlpha(RpClump *clump)
{
	return CLUMPEXT(clump)->alpha;
}
