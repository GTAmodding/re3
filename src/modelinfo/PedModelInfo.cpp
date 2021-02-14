#include "common.h"

#include "RwHelper.h"
#include "General.h"
#include "Bones.h"
#include "SurfaceTable.h"
#include "Ped.h"
#include "NodeName.h"
#include "VisibilityPlugins.h"
#include "ModelInfo.h"
#include "custompipes.h"

void
CPedModelInfo::DeleteRwObject(void)
{
	if(m_hitColModel)
		delete m_hitColModel;
	m_hitColModel = nil;
#ifdef PED_SKIN
	RwFrame *frame;
	if(m_head){
		frame = RpAtomicGetFrame(m_head);
		RpAtomicDestroy(m_head);
		RwFrameDestroy(frame);
		m_head = nil;
	}
	if(m_lhand){
		frame = RpAtomicGetFrame(m_lhand);
		RpAtomicDestroy(m_lhand);
		RwFrameDestroy(frame);
		m_lhand = nil;
	}
	if(m_rhand){
		frame = RpAtomicGetFrame(m_rhand);
		RpAtomicDestroy(m_rhand);
		RwFrameDestroy(frame);
		m_rhand = nil;
	}
#endif
	CClumpModelInfo::DeleteRwObject();	// PC calls this first
}

RwObjectNameIdAssocation CPedModelInfo::m_pPedIds[PED_NODE_MAX] = {
	{ "Smid",	PED_MID, 0, },	// that is strange...
	{ "Shead",	PED_HEAD, 0, },
	{ "Supperarml",	PED_UPPERARML, 0, },
	{ "Supperarmr",	PED_UPPERARMR, 0, },
	{ "SLhand",	PED_HANDL, 0, },
	{ "SRhand",	PED_HANDR, 0, },
	{ "Supperlegl",	PED_UPPERLEGL, 0, },
	{ "Supperlegr",	PED_UPPERLEGR, 0, },
	{ "Sfootl",	PED_FOOTL, 0, },
	{ "Sfootr",	PED_FOOTR, 0, },
	{ "Slowerlegr",	PED_LOWERLEGR, 0, },
	{ nil,	0, 0, },
};

#ifdef PED_SKIN
struct LimbCBarg
{
	CPedModelInfo *mi;
	RpClump *clump;
	int32 frameIDs[3];
};

RpAtomic*
CPedModelInfo::findLimbsCb(RpAtomic *atomic, void *data)
{
	LimbCBarg *limbs = (LimbCBarg*)data;
	RwFrame *frame = RpAtomicGetFrame(atomic);
	const char *name = GetFrameNodeName(frame);
	if(CGeneral::faststricmp(name, "Shead01") == 0){
		limbs->frameIDs[0] = RpHAnimFrameGetID(frame);
		limbs->mi->m_head = atomic;
		RpClumpRemoveAtomic(limbs->clump, atomic);
		RwFrameRemoveChild(frame);
	}else if(CGeneral::faststricmp(name, "SLhand01") == 0){
		limbs->frameIDs[1] = RpHAnimFrameGetID(frame);
		limbs->mi->m_lhand = atomic;
		RpClumpRemoveAtomic(limbs->clump, atomic);
		RwFrameRemoveChild(frame);
	}else if(CGeneral::faststricmp(name, "SRhand01") == 0){
		limbs->frameIDs[2] = RpHAnimFrameGetID(frame);
		limbs->mi->m_rhand = atomic;
		RpClumpRemoveAtomic(limbs->clump, atomic);
		RwFrameRemoveChild(frame);
	}
	return atomic;
}
#endif

void
CPedModelInfo::SetClump(RpClump *clump)
{
#ifdef EXTENDED_PIPELINES
	CustomPipes::AttachRimPipe(clump);
#endif
#ifdef PED_SKIN
	// CB has to be set here before atomics are detached from clump
	if(strcmp(GetModelName(), "player") == 0)
		RpClumpForAllAtomics(clump, SetAtomicRendererCB, (void*)CVisibilityPlugins::RenderPlayerCB);
	if(IsClumpSkinned(clump)){
		LimbCBarg limbs = { this, clump, { 0, 0, 0 } };
		RpClumpForAllAtomics(clump, findLimbsCb, &limbs);
	}
	CClumpModelInfo::SetClump(clump);
	SetFrameIds(m_pPedIds);
	if(m_hitColModel == nil && !IsClumpSkinned(clump))
		CreateHitColModel();
	// And again because CClumpModelInfo resets it
	if(strcmp(GetModelName(), "player") == 0)
		RpClumpForAllAtomics(m_clump, SetAtomicRendererCB, (void*)CVisibilityPlugins::RenderPlayerCB);
	else if(IsClumpSkinned(clump))
		// skinned peds have no low detail version, so they don't have the right render Cb
		RpClumpForAllAtomics(m_clump, SetAtomicRendererCB, (void*)CVisibilityPlugins::RenderPedCB);
#else
	CClumpModelInfo::SetClump(clump);
	SetFrameIds(m_pPedIds);
	if(m_hitColModel == nil)
		CreateHitColModel();
	if(strcmp(GetModelName(), "player") == 0)
		RpClumpForAllAtomics(m_clump, SetAtomicRendererCB, (void*)CVisibilityPlugins::RenderPlayerCB);
#endif
}

RpAtomic*
CountAtomicsCB(RpAtomic *atomic, void *data)
{
	(*(int32*)data)++;
	return atomic;
}

RpAtomic*
GetAtomicListCB(RpAtomic *atomic, void *data)
{
	**(RpAtomic***)data = atomic;
	(*(RpAtomic***)data)++;
	return atomic;
}

RwFrame*
FindPedFrameFromNameCB(RwFrame *frame, void *data)
{
	RwObjectNameAssociation *assoc = (RwObjectNameAssociation*)data;

	if(CGeneral::faststricmp(GetFrameNodeName(frame)+1, assoc->name+1)){
		RwFrameForAllChildren(frame, FindPedFrameFromNameCB, assoc);
		return assoc->frame ? nil : frame;
	}else{
		assoc->frame = frame;
		return nil;
	}
}

void
CPedModelInfo::SetLowDetailClump(RpClump *lodclump)
{
	RpAtomic *atomics[16];
	RpAtomic **pAtm;
	int32 numAtm, numLodAtm;
	int i;
	RwObjectNameAssociation assoc;

	numAtm = 0;
	numLodAtm = 0;
	RpClumpForAllAtomics(m_clump, CountAtomicsCB, &numAtm);		// actually unused
	RpClumpForAllAtomics(lodclump, CountAtomicsCB, &numLodAtm);

	RpClumpForAllAtomics(m_clump, SetAtomicRendererCB, (void*)CVisibilityPlugins::RenderPedHiDetailCB);
	RpClumpForAllAtomics(lodclump, SetAtomicRendererCB, (void*)CVisibilityPlugins::RenderPedLowDetailCB);

	pAtm = atomics;
	RpClumpForAllAtomics(lodclump, GetAtomicListCB, &pAtm);

	for(i = 0; i < numLodAtm; i++){
		assoc.name = GetFrameNodeName(RpAtomicGetFrame(atomics[i]));
		assoc.frame = nil;
		RwFrameForAllChildren(RpClumpGetFrame(m_clump), FindPedFrameFromNameCB, &assoc);
		if(assoc.frame){
			RpAtomicSetFrame(atomics[i], assoc.frame);
			RpClumpRemoveAtomic(lodclump, atomics[i]);
			RpClumpAddAtomic(m_clump, atomics[i]);
		}
	}
}

struct ColNodeInfo
{
	Const char *name;
	int pedNode;
	int pieceType;
	float x, z;
	float radius;
};

#define NUMPEDINFONODES 8
ColNodeInfo m_pColNodeInfos[NUMPEDINFONODES] = {
	{ nil,          PED_HEAD,		PEDPIECE_HEAD,  0.0f,   0.05f, 0.2f },
	{ "Storso",     0,				PEDPIECE_TORSO,  0.0f,   0.15f, 0.2f },
	{ "Storso",     0,				PEDPIECE_TORSO,  0.0f,  -0.05f, 0.3f },
	{ nil,          PED_MID,		PEDPIECE_MID,  0.0f,  -0.07f, 0.3f },
	{ nil,          PED_UPPERARML,	PEDPIECE_LEFTARM,  0.07f, -0.1f,  0.2f },
	{ nil,          PED_UPPERARMR,	PEDPIECE_RIGHTARM, -0.07f, -0.1f,  0.2f },
	{ "Slowerlegl", 0,				PEDPIECE_LEFTLEG,  0.0f,   0.07f, 0.25f },
	{ nil,          PED_LOWERLEGR,	PEDPIECE_RIGHTLEG,  0.0f,   0.07f, 0.25f },
};

RwObject*
FindHeadRadiusCB(RwObject *object, void *data)
{
	RpAtomic *atomic = (RpAtomic*)object;
	*(float*)data = RpAtomicGetBoundingSphere(atomic)->radius;
	return nil;
}

void
CPedModelInfo::CreateHitColModel(void)
{
	RwObjectNameAssociation nameAssoc;
	RwObjectIdAssociation idAssoc;
	RwFrame *nodeFrame;
	CColModel *colmodel = new CColModel;
	CColSphere *spheres = (CColSphere*)RwMalloc(NUMPEDINFONODES*sizeof(CColSphere));
	RwFrame *root = RpClumpGetFrame(m_clump);
	RwMatrix *mat = RwMatrixCreate();
	for(int i = 0; i < NUMPEDINFONODES; i++){
		nodeFrame = nil;
		if(m_pColNodeInfos[i].name){
			nameAssoc.name = m_pColNodeInfos[i].name;
			nameAssoc.frame = nil;
			RwFrameForAllChildren(root, FindFrameFromNameCB, &nameAssoc);
			nodeFrame = nameAssoc.frame;
		}else{
			idAssoc.id = m_pColNodeInfos[i].pedNode;
			idAssoc.frame = nil;
			RwFrameForAllChildren(root, FindFrameFromIdCB, &idAssoc);
			nodeFrame = idAssoc.frame;
		}
		if(nodeFrame){
			float radius = m_pColNodeInfos[i].radius;
			if(m_pColNodeInfos[i].pieceType == PEDPIECE_HEAD)
				RwFrameForAllObjects(nodeFrame, FindHeadRadiusCB, &radius);
			RwMatrixTransform(mat, RwFrameGetMatrix(nodeFrame), rwCOMBINEREPLACE);
			const char *name = GetFrameNodeName(nodeFrame);
			for(nodeFrame = RwFrameGetParent(nodeFrame);
			    nodeFrame; 
			    nodeFrame = RwFrameGetParent(nodeFrame)){
				name = GetFrameNodeName(nodeFrame);
				RwMatrixTransform(mat, RwFrameGetMatrix(nodeFrame), rwCOMBINEPOSTCONCAT);
				if(RwFrameGetParent(nodeFrame) == root)
					break;
			}
			spheres[i].center = mat->pos + CVector(m_pColNodeInfos[i].x, 0.0f, m_pColNodeInfos[i].z);
			spheres[i].radius = radius;
			spheres[i].surface = SURFACE_PED;
			spheres[i].piece = m_pColNodeInfos[i].pieceType;
		}
	}
	RwMatrixDestroy(mat);
	colmodel->spheres = spheres;
	colmodel->numSpheres = NUMPEDINFONODES;
	colmodel->boundingSphere.Set(2.0f, CVector(0.0f, 0.0f, 0.0f), SURFACE_DEFAULT, 0);
	colmodel->boundingBox.Set(CVector(-0.5f, -0.5f, -1.2f), CVector(0.5f, 0.5f, 1.2f), SURFACE_DEFAULT, 0);
	colmodel->level = LEVEL_GENERIC;
	m_hitColModel = colmodel;
}

CColModel*
CPedModelInfo::AnimatePedColModel(CColModel* colmodel, RwFrame* frame)
{
	RwObjectNameAssociation nameAssoc;
	RwObjectIdAssociation idAssoc;
	RwMatrix* mat = RwMatrixCreate();
	CColSphere* spheres = colmodel->spheres;

	for (int i = 0; i < NUMPEDINFONODES; i++) {
		RwFrame* f = nil;
		if (m_pColNodeInfos[i].name) {
			nameAssoc.name = m_pColNodeInfos[i].name;
			nameAssoc.frame = nil;
			RwFrameForAllChildren(frame, FindFrameFromNameCB, &nameAssoc);
			f = nameAssoc.frame;
		}
		else {
			idAssoc.id = m_pColNodeInfos[i].pedNode;
			idAssoc.frame = nil;
			RwFrameForAllChildren(frame, FindFrameFromIdCB, &idAssoc);
			f = idAssoc.frame;
		}
		if (f) {
			RwMatrixCopy(mat, RwFrameGetMatrix(f));

			for (f = RwFrameGetParent(f); f; f = RwFrameGetParent(f)) {
				RwMatrixTransform(mat, RwFrameGetMatrix(f), rwCOMBINEPOSTCONCAT);
				if (RwFrameGetParent(f) == frame)
					break;
			}

			spheres[i].center = mat->pos + CVector(m_pColNodeInfos[i].x, 0.0f, m_pColNodeInfos[i].z);
		}
	}

	return colmodel;
}

#ifdef PED_SKIN
void
CPedModelInfo::CreateHitColModelSkinned(RpClump *clump)
{
	RpHAnimHierarchy *hier = GetAnimHierarchyFromSkinClump(clump);
	CColModel *colmodel = new CColModel;
	CColSphere *spheres = (CColSphere*)RwMalloc(NUMPEDINFONODES*sizeof(CColSphere));
	RwFrame *root = RpClumpGetFrame(m_clump);
	RwMatrix *invmat = RwMatrixCreate();
	RwMatrix *mat = RwMatrixCreate();
	RwMatrixInvert(invmat, RwFrameGetMatrix(RpClumpGetFrame(clump)));

	for(int i = 0; i < NUMPEDINFONODES; i++){
		*mat = *invmat;
		int id = ConvertPedNode2BoneTag(m_pColNodeInfos[i].pedNode);	// this is wrong, wtf R* ???
		int idx = RpHAnimIDGetIndex(hier, id);

		// This doesn't really work as the positions are not initialized yet
		RwMatrixTransform(mat, &RpHAnimHierarchyGetMatrixArray(hier)[idx], rwCOMBINEPRECONCAT);
		RwV3d pos = { 0.0f, 0.0f, 0.0f };
		RwV3dTransformPoints(&pos, &pos, 1, mat);

		spheres[i].center = pos + CVector(m_pColNodeInfos[i].x, 0.0f, m_pColNodeInfos[i].z);
		spheres[i].radius = m_pColNodeInfos[i].radius;
		spheres[i].surface = SURFACE_PED;
		spheres[i].piece = m_pColNodeInfos[i].pieceType;
	}
	RwMatrixDestroy(invmat);
	RwMatrixDestroy(mat);
	colmodel->spheres = spheres;
	colmodel->numSpheres = NUMPEDINFONODES;
	colmodel->boundingSphere.Set(2.0f, CVector(0.0f, 0.0f, 0.0f), SURFACE_DEFAULT, 0);
	colmodel->boundingBox.Set(CVector(-0.5f, -0.5f, -1.2f), CVector(0.5f, 0.5f, 1.2f), SURFACE_DEFAULT, 0);
	colmodel->level = LEVEL_GENERIC;
	m_hitColModel = colmodel;
}

CColModel*
CPedModelInfo::AnimatePedColModelSkinned(RpClump *clump)
{
	if(m_hitColModel == nil){
		CreateHitColModelSkinned(clump);
		return m_hitColModel;
	}
	RwMatrix *invmat, *mat;
	CColSphere *spheres = m_hitColModel->spheres;
	RpHAnimHierarchy *hier = GetAnimHierarchyFromSkinClump(clump);
	invmat = RwMatrixCreate();
	mat = RwMatrixCreate();
	RwMatrixInvert(invmat, RwFrameGetMatrix(RpClumpGetFrame(clump)));

	for(int i = 0; i < NUMPEDINFONODES; i++){
		*mat = *invmat;
		int id = ConvertPedNode2BoneTag(m_pColNodeInfos[i].pedNode);
		int idx = RpHAnimIDGetIndex(hier, id);

		RwMatrixTransform(mat, &RpHAnimHierarchyGetMatrixArray(hier)[idx], rwCOMBINEPRECONCAT);
		RwV3d pos = { 0.0f, 0.0f, 0.0f };
		RwV3dTransformPoints(&pos, &pos, 1, mat);

		spheres[i].center = pos + CVector(m_pColNodeInfos[i].x, 0.0f, m_pColNodeInfos[i].z);
	}
	RwMatrixDestroy(invmat);
	RwMatrixDestroy(mat);
	return m_hitColModel;
}

#endif
