#include "common.h"

#include "General.h"
#include "TempColModels.h"
#include "ModelIndices.h"
#include "ModelInfo.h"
#include "Frontend.h"

CBaseModelInfo *CModelInfo::ms_modelInfoPtrs[MODELINFOSIZE];

CStore<CSimpleModelInfo, SIMPLEMODELSIZE> CModelInfo::ms_simpleModelStore;
CStore<CMloModelInfo, MLOMODELSIZE> CModelInfo::ms_mloModelStore;
CStore<CInstance, MLOINSTANCESIZE> CModelInfo::ms_mloInstanceStore;
CStore<CTimeModelInfo, TIMEMODELSIZE> CModelInfo::ms_timeModelStore;
CStore<CClumpModelInfo, CLUMPMODELSIZE> CModelInfo::ms_clumpModelStore;
CStore<CPedModelInfo, PEDMODELSIZE> CModelInfo::ms_pedModelStore;
CStore<CVehicleModelInfo, VEHICLEMODELSIZE> CModelInfo::ms_vehicleModelStore;
CStore<CXtraCompsModelInfo, XTRACOMPSMODELSIZE> CModelInfo::ms_xtraCompsModelStore;
CStore<C2dEffect, TWODFXSIZE> CModelInfo::ms_2dEffectStore;

void
CModelInfo::Initialise(void)
{
	int i;
	CSimpleModelInfo *m;

	for(i = 0; i < MODELINFOSIZE; i++)
		ms_modelInfoPtrs[i] = nil;
	ms_2dEffectStore.Clear();
	ms_mloInstanceStore.Clear();
	ms_xtraCompsModelStore.Clear();
	ms_simpleModelStore.Clear();
	ms_timeModelStore.Clear();
	ms_mloModelStore.Clear();
	ms_clumpModelStore.Clear();
	ms_pedModelStore.Clear();
	ms_vehicleModelStore.Clear();

	m = AddSimpleModel(MI_CAR_DOOR);
	m->SetColModel(&CTempColModels::ms_colModelDoor1);
	m->SetTexDictionary("generic");
	m->SetNumAtomics(1);
	m->m_lodDistances[0] = 80.0f;

	m = AddSimpleModel(MI_CAR_BUMPER);
	m->SetColModel(&CTempColModels::ms_colModelBumper1);
	m->SetTexDictionary("generic");
	m->SetNumAtomics(1);
	m->m_lodDistances[0] = 80.0f;

	m = AddSimpleModel(MI_CAR_PANEL);
	m->SetColModel(&CTempColModels::ms_colModelPanel1);
	m->SetTexDictionary("generic");
	m->SetNumAtomics(1);
	m->m_lodDistances[0] = 80.0f;

	m = AddSimpleModel(MI_CAR_BONNET);
	m->SetColModel(&CTempColModels::ms_colModelBonnet1);
	m->SetTexDictionary("generic");
	m->SetNumAtomics(1);
	m->m_lodDistances[0] = 80.0f;

	m = AddSimpleModel(MI_CAR_BOOT);
	m->SetColModel(&CTempColModels::ms_colModelBoot1);
	m->SetTexDictionary("generic");
	m->SetNumAtomics(1);
	m->m_lodDistances[0] = 80.0f;

	m = AddSimpleModel(MI_CAR_WHEEL);
	m->SetColModel(&CTempColModels::ms_colModelWheel1);
	m->SetTexDictionary("generic");
	m->SetNumAtomics(1);
	m->m_lodDistances[0] = 80.0f;

	m = AddSimpleModel(MI_BODYPARTA);
	m->SetColModel(&CTempColModels::ms_colModelBodyPart1);
	m->SetTexDictionary("generic");
	m->SetNumAtomics(1);
	m->m_lodDistances[0] = 80.0f;

	m = AddSimpleModel(MI_BODYPARTB);
	m->SetColModel(&CTempColModels::ms_colModelBodyPart2);
	m->SetTexDictionary("generic");
	m->SetNumAtomics(1);
	m->m_lodDistances[0] = 80.0f;
}

void
CModelInfo::ShutDown(void)
{
	int i;
	for(i = 0; i < ms_simpleModelStore.allocPtr; i++)
		ms_simpleModelStore.store[i].Shutdown();
	for(i = 0; i < ms_mloInstanceStore.allocPtr; i++)
		ms_mloInstanceStore.store[i].Shutdown();
	for(i = 0; i < ms_timeModelStore.allocPtr; i++)
		ms_timeModelStore.store[i].Shutdown();
	for(i = 0; i < ms_clumpModelStore.allocPtr; i++)
		ms_clumpModelStore.store[i].Shutdown();
	for(i = 0; i < ms_vehicleModelStore.allocPtr; i++)
		ms_vehicleModelStore.store[i].Shutdown();
	for(i = 0; i < ms_pedModelStore.allocPtr; i++)
		ms_pedModelStore.store[i].Shutdown();
	for(i = 0; i < ms_xtraCompsModelStore.allocPtr; i++)
		ms_xtraCompsModelStore.store[i].Shutdown();
	for(i = 0; i < ms_mloInstanceStore.allocPtr; i++)
		ms_mloInstanceStore.store[i].Shutdown();
	for(i = 0; i < ms_2dEffectStore.allocPtr; i++)
		ms_2dEffectStore.store[i].Shutdown();

	ms_2dEffectStore.Clear();
	ms_simpleModelStore.Clear();
	ms_mloInstanceStore.Clear();
	ms_mloModelStore.Clear();
	ms_xtraCompsModelStore.Clear();
	ms_timeModelStore.Clear();
	ms_pedModelStore.Clear();
	ms_clumpModelStore.Clear();
	ms_vehicleModelStore.Clear();
}

CSimpleModelInfo*
CModelInfo::AddSimpleModel(int id)
{
	CSimpleModelInfo *modelinfo;
	modelinfo = CModelInfo::ms_simpleModelStore.Alloc();
	CModelInfo::ms_modelInfoPtrs[id] = modelinfo;
	modelinfo->Init();
	return modelinfo;
}

CMloModelInfo *
CModelInfo::AddMloModel(int id)
{
	CMloModelInfo *modelinfo;
	modelinfo = CModelInfo::ms_mloModelStore.Alloc();
	CModelInfo::ms_modelInfoPtrs[id] = modelinfo;
	modelinfo->m_clump = nil;
	modelinfo->firstInstance = 0;
	modelinfo->lastInstance = 0;
	return modelinfo;
}

CTimeModelInfo*
CModelInfo::AddTimeModel(int id)
{
	CTimeModelInfo *modelinfo;
	modelinfo = CModelInfo::ms_timeModelStore.Alloc();
	CModelInfo::ms_modelInfoPtrs[id] = modelinfo;
	modelinfo->Init();
	return modelinfo;
}

CClumpModelInfo*
CModelInfo::AddClumpModel(int id)
{
	CClumpModelInfo *modelinfo;
	modelinfo = CModelInfo::ms_clumpModelStore.Alloc();
	CModelInfo::ms_modelInfoPtrs[id] = modelinfo;
	modelinfo->m_clump = nil;
	return modelinfo;
}

CPedModelInfo*
CModelInfo::AddPedModel(int id)
{
	CPedModelInfo *modelinfo;
	modelinfo = CModelInfo::ms_pedModelStore.Alloc();
	CModelInfo::ms_modelInfoPtrs[id] = modelinfo;
	modelinfo->m_clump = nil;
	return modelinfo;
}

CVehicleModelInfo*
CModelInfo::AddVehicleModel(int id)
{
	CVehicleModelInfo *modelinfo;
	modelinfo = CModelInfo::ms_vehicleModelStore.Alloc();
	CModelInfo::ms_modelInfoPtrs[id] = modelinfo;
	modelinfo->m_clump = nil;
	modelinfo->m_vehicleType = -1;
	modelinfo->m_wheelId = -1;
	modelinfo->m_materials1[0] = nil;
	modelinfo->m_materials2[0] = nil;
	modelinfo->m_bikeSteerAngle = 999.99f;
	return modelinfo;
}

CBaseModelInfo*
CModelInfo::GetModelInfo(const char *name, int *id)
{
	CBaseModelInfo *modelinfo;
	for(int i = 0; i < MODELINFOSIZE; i++){
		modelinfo = CModelInfo::ms_modelInfoPtrs[i];
	 	if(modelinfo && !CGeneral::faststricmp(modelinfo->GetModelName(), name)){
			if(id)
				*id = i;
			return modelinfo;
		}
	}
	return nil;
}

bool
CModelInfo::IsBoatModel(int32 id)
{
	return GetModelInfo(id)->GetModelType() == MITYPE_VEHICLE &&
		((CVehicleModelInfo*)GetModelInfo(id))->m_vehicleType == VEHICLE_TYPE_BOAT;
}

bool
CModelInfo::IsBikeModel(int32 id)
{
	return GetModelInfo(id)->GetModelType() == MITYPE_VEHICLE &&
		((CVehicleModelInfo*)GetModelInfo(id))->m_vehicleType == VEHICLE_TYPE_BIKE;
}

void
CModelInfo::RemoveColModelsFromOtherLevels(eLevelName level)
{
	ISLAND_LOADING_IS(LOW)
	{
		int i;
		CBaseModelInfo *mi;
		CColModel *colmodel;

		for (i = 0; i < MODELINFOSIZE; i++) {
			mi = GetModelInfo(i);
			if (mi) {
				colmodel = mi->GetColModel();
				if (colmodel && colmodel->level != LEVEL_GENERIC && colmodel->level != level)
					colmodel->RemoveCollisionVolumes();
			}
		}
	}
}

void
CModelInfo::ConstructMloClumps()
{
	for (int i = 0; i < ms_mloModelStore.allocPtr; i++)
		ms_mloModelStore.store[i].ConstructClump();
}

void
CModelInfo::ReInit2dEffects()
{
	ms_2dEffectStore.Clear();

	for (int i = 0; i < MODELINFOSIZE; i++) {
		if (ms_modelInfoPtrs[i])
			ms_modelInfoPtrs[i]->Init2dEffects();
	}
}
