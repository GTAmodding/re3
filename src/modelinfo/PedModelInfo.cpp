#include "common.h"

#include "RwHelper.h"
#include "General.h"
#include "Bones.h"
#include "SurfaceTable.h"
#include "Ped.h"
#include "NodeName.h"
#include "VisibilityPlugins.h"
#include "ModelInfo.h"

//--MIAMI: file done

void
CPedModelInfo::DeleteRwObject(void)
{
	CClumpModelInfo::DeleteRwObject();
	if(m_hitColModel)
		delete m_hitColModel;
	m_hitColModel = nil;
}

// leftover...
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

void
CPedModelInfo::SetClump(RpClump *clump)
{
	CClumpModelInfo::SetClump(clump);
	SetFrameIds(m_pPedIds);	// not needed in VC actually
	if(m_hitColModel == nil)
		CreateHitColModelSkinned(clump);
	RpClumpForAllAtomics(m_clump, SetAtomicRendererCB, (void*)CVisibilityPlugins::RenderPedCB);
	if(strcmp(GetName(), "player") == 0)
		RpClumpForAllAtomics(m_clump, SetAtomicRendererCB, (void*)CVisibilityPlugins::RenderPlayerCB);
}

struct ColNodeInfo
{
	Const char *name;
	int pedNode;
	int pieceType;
	float x, z;
	float radius;
};

#define NUMPEDINFONODES 10
ColNodeInfo m_pColNodeInfos[NUMPEDINFONODES] = {
	{ nil, PED_HEAD,	PEDPIECE_HEAD,	0.0f,   0.05f, 0.15f },
	{ nil, PED_MID,		PEDPIECE_TORSO,	0.0f,   0.15f, 0.2f },
	{ nil, PED_MID,		PEDPIECE_TORSO,	0.0f,  -0.05f, 0.25f },
	{ nil, PED_MID,		PEDPIECE_MID,	0.0f,  -0.25f, 0.25f },
	{ nil, PED_UPPERARML,	PEDPIECE_LEFTARM,	0.03f, -0.05f,  0.16f },
	{ nil, PED_UPPERARMR,	PEDPIECE_RIGHTARM,	-0.03f, -0.05f,  0.16f },
	{ nil, PED_LOWERLEGL,	PEDPIECE_LEFTLEG,	0.0f,   0.15f, 0.2f },
	{ nil, PED_LOWERLEGR,	PEDPIECE_RIGHTLEG,	0.0f,   0.15f, 0.2f },
	{ nil, PED_FOOTL,	PEDPIECE_LEFTLEG,	0.0f,   0.15f, 0.15f },
	{ nil, PED_FOOTR,	PEDPIECE_RIGHTLEG,	0.0f,   0.15f, 0.15f },
};

void
CPedModelInfo::CreateHitColModelSkinned(RpClump *clump)
{
	CVector center;
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

		center.x = pos.x + m_pColNodeInfos[i].x;
		center.y = pos.y + 0.0f;
		center.z = pos.z + m_pColNodeInfos[i].z;
		spheres[i].Set(m_pColNodeInfos[i].radius, center, SURFACE_PED, m_pColNodeInfos[i].pieceType);
	}
	RwMatrixDestroy(invmat);
	RwMatrixDestroy(mat);
	colmodel->spheres = spheres;
	colmodel->numSpheres = NUMPEDINFONODES;
	center.x = center.y = center.z = 0.0f;
	colmodel->boundingSphere.Set(2.0f, center);
	CVector min, max;
	min.x = min.y = -0.5f;
	min.z = -1.2f;
	max.x = max.y = 0.5f;
	max.z = 1.2f;
	colmodel->boundingBox.Set(min, max);
	colmodel->level = LEVEL_NONE;
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

		spheres[i].center.x = pos.x + m_pColNodeInfos[i].x;
		spheres[i].center.y = pos.y + 0.0f;
		spheres[i].center.z = pos.z + m_pColNodeInfos[i].z;
	}
	RwMatrixDestroy(invmat);
	RwMatrixDestroy(mat);
	return m_hitColModel;
}

CColModel*
CPedModelInfo::AnimatePedColModelSkinnedWorld(RpClump *clump)
{
	if(m_hitColModel == nil)
		CreateHitColModelSkinned(clump);
	CColSphere *spheres = m_hitColModel->spheres;
	RpHAnimHierarchy *hier = GetAnimHierarchyFromSkinClump(clump);
	RwMatrix *mat;

	for(int i = 0; i < NUMPEDINFONODES; i++){
		int id = ConvertPedNode2BoneTag(m_pColNodeInfos[i].pedNode);
		int idx = RpHAnimIDGetIndex(hier, id);

		mat = &RpHAnimHierarchyGetMatrixArray(hier)[idx];
		RwV3d pos = { 0.0f, 0.0f, 0.0f };
		RwV3dTransformPoints(&pos, &pos, 1, mat);

		spheres[i].center.x = pos.x + m_pColNodeInfos[i].x;
		spheres[i].center.y = pos.y + 0.0f;
		spheres[i].center.z = pos.z + m_pColNodeInfos[i].z;
	}
	return m_hitColModel;
}
