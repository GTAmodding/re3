#include "common.h"

#include "main.h"
#include "RwHelper.h"
#include "Lights.h"
#include "PointLights.h"
#include "RpAnimBlend.h"
#include "AnimBlendClumpData.h"
#include "Bones.h"
#include "Renderer.h"
#include "ModelIndices.h"
#include "Shadows.h"
#include "Timecycle.h"
#include "CutsceneObject.h"

CCutsceneObject::CCutsceneObject(void)
{
	SetStatus(STATUS_SIMPLE);
	bUsesCollision = false;
	bStreamingDontDelete = true;
	ObjectCreatedBy = CUTSCENE_OBJECT;
	m_fMass = 1.0f;
	m_fTurnMass = 1.0f;

#ifdef PED_SKIN
	bRenderHead = true;
	bRenderRightHand = true;
	bRenderLeftHand = true;
#endif
}

void
CCutsceneObject::SetModelIndex(uint32 id)
{
	CEntity::SetModelIndex(id);
	assert(RwObjectGetType(m_rwObject) == rpCLUMP);
	RpAnimBlendClumpInit((RpClump*)m_rwObject);
	(*RPANIMBLENDCLUMPDATA(m_rwObject))->velocity3d = &m_vecMoveSpeed;
	(*RPANIMBLENDCLUMPDATA(m_rwObject))->frames[0].flag |= AnimBlendFrameData::VELOCITY_EXTRACTION_3D;
}

void
CCutsceneObject::ProcessControl(void)
{
	CPhysical::ProcessControl();

	if(CTimer::GetTimeStep() < 1/100.0f)
		m_vecMoveSpeed *= 100.0f;
	else
		m_vecMoveSpeed *= 1.0f/CTimer::GetTimeStep();

	ApplyMoveSpeed();

#ifdef PED_SKIN
	if(IsClumpSkinned(GetClump()))
		UpdateRpHAnim();
#endif
}

static RpMaterial*
MaterialSetAlpha(RpMaterial *material, void *data)
{
	((RwRGBA*)RpMaterialGetColor(material))->alpha = (uint8)(uintptr)data;
	return material;
}

void
CCutsceneObject::PreRender(void)
{
	if(IsPedModel(GetModelIndex())){
		CShadows::StoreShadowForPedObject(this,
			CTimeCycle::m_fShadowDisplacementX[CTimeCycle::m_CurrentStoredValue],
			CTimeCycle::m_fShadowDisplacementY[CTimeCycle::m_CurrentStoredValue],
			CTimeCycle::m_fShadowFrontX[CTimeCycle::m_CurrentStoredValue],
			CTimeCycle::m_fShadowFrontY[CTimeCycle::m_CurrentStoredValue],
			CTimeCycle::m_fShadowSideX[CTimeCycle::m_CurrentStoredValue],
			CTimeCycle::m_fShadowSideY[CTimeCycle::m_CurrentStoredValue]);
		// For some reason xbox/android limbs are transparent here...
		RpGeometry *geometry = RpAtomicGetGeometry(GetFirstAtomic(GetClump()));
		RpGeometrySetFlags(geometry, RpGeometryGetFlags(geometry) | rpGEOMETRYMODULATEMATERIALCOLOR);
		RpGeometryForAllMaterials(geometry, MaterialSetAlpha, (void*)255);
	}
}

void
CCutsceneObject::Render(void)
{
#ifdef PED_SKIN
	if(IsClumpSkinned(GetClump())){
		if(bRenderLeftHand) RenderLimb(BONE_Lhand);
		if(bRenderRightHand) RenderLimb(BONE_Rhand);
		if(bRenderHead) RenderLimb(BONE_head);
	}
#endif
	CObject::Render();
}

#ifdef PED_SKIN
void
CCutsceneObject::RenderLimb(int32 bone)
{
	RpAtomic *atomic;
	CPedModelInfo *mi = (CPedModelInfo *)CModelInfo::GetModelInfo(GetModelIndex());
	switch(bone){
	case BONE_head:
		atomic = mi->getHead();
		break;
	case BONE_Lhand:
		atomic = mi->getLeftHand();
		break;
	case BONE_Rhand:
		atomic = mi->getRightHand();
		break;
	default:
		return;
	}
	if(atomic){
		RpHAnimHierarchy *hier = GetAnimHierarchyFromSkinClump(GetClump());
		int idx = RpHAnimIDGetIndex(hier, bone);
		RwMatrix *mat = &RpHAnimHierarchyGetMatrixArray(hier)[idx];
		RwFrame *frame = RpAtomicGetFrame(atomic);
		*RwFrameGetMatrix(frame) = *mat;
		RwFrameUpdateObjects(frame);
		RpAtomicRender(atomic);
	}
}
#endif

bool
CCutsceneObject::SetupLighting(void)
{
	ActivateDirectional();
	SetAmbientColoursForPedsCarsAndObjects();

	if(bRenderScorched){
		WorldReplaceNormalLightsWithScorched(Scene.world, 0.1f);
	}else{
		CVector coors = GetPosition();
		float lighting = CPointLights::GenerateLightsAffectingObject(&coors);
		if(!bHasBlip && lighting != 1.0f){
			SetAmbientAndDirectionalColours(lighting);
			return true;
		}
	}

	return false;
}

void
CCutsceneObject::RemoveLighting(bool reset)
{
	CRenderer::RemoveVehiclePedLights(this, reset);
}
