#include "common.h"

#include "main.h"
#include "Lights.h"
#include "Pools.h"
#include "Radar.h"
#include "Object.h"
#include "DummyObject.h"
#include "Particle.h"
#include "General.h"
#include "ObjectData.h"
#include "World.h"
#include "Floater.h"
#include "soundlist.h"

int16 CObject::nNoTempObjects;
int16 CObject::nBodyCastHealth = 1000;

void *CObject::operator new(size_t sz) { return CPools::GetObjectPool()->New();  }
void *CObject::operator new(size_t sz, int handle) { return CPools::GetObjectPool()->New(handle);};
void CObject::operator delete(void *p, size_t sz) { CPools::GetObjectPool()->Delete((CObject*)p); }
void CObject::operator delete(void *p, int handle) { CPools::GetObjectPool()->Delete((CObject*)p); }

CObject::CObject(void)
{
	m_type = ENTITY_TYPE_OBJECT;
	m_fUprootLimit = 0.0f;
	m_nCollisionDamageEffect = 0;
	m_nSpecialCollisionResponseCases = COLLRESPONSE_NONE;
	m_bCameraToAvoidThisObject = false;
	ObjectCreatedBy = UNKNOWN_OBJECT;
	m_nEndOfLifeTime = 0;
//	m_nRefModelIndex = -1;	// duplicate
//	bUseVehicleColours = false;	// duplicate
	m_colour2 = 0;
	m_colour1 = m_colour2;
	m_nBonusValue = 0;
	bIsPickup = false;
	bPickupObjWithMessage = false;
	bOutOfStock = false;
	bGlassCracked = false;
	bGlassBroken = false;
	bHasBeenDamaged = false;
	m_nRefModelIndex = -1;
	bUseVehicleColours = false;
	m_pCurSurface = nil;
	m_pCollidingEntity = nil;
}

CObject::CObject(int32 mi, bool createRW)
{
	if (createRW)
		SetModelIndex(mi);
	else
		SetModelIndexNoCreate(mi);
	Init();
}

CObject::CObject(CDummyObject *dummy)
{
	SetModelIndexNoCreate(dummy->GetModelIndex());

	if (dummy->m_rwObject)
		AttachToRwObject(dummy->m_rwObject);
	else
		GetMatrix() = dummy->GetMatrix();

	m_objectMatrix = dummy->GetMatrix();
	dummy->DetachFromRwObject();
	Init();
	m_level = dummy->m_level;
}

CObject::~CObject(void)
{
	CRadar::ClearBlipForEntity(BLIP_OBJECT, CPools::GetObjectPool()->GetIndex(this));

	if(m_nRefModelIndex != -1)
		CModelInfo::GetModelInfo(m_nRefModelIndex)->RemoveRef();

	if(ObjectCreatedBy == TEMP_OBJECT && nNoTempObjects != 0)
		nNoTempObjects--;
}

void 
CObject::ProcessControl(void) 
{ 
	CVector point, impulse;
	if (m_nCollisionDamageEffect)
		ObjectDamage(m_fDamageImpulse);
	CPhysical::ProcessControl();
	if (mod_Buoyancy.ProcessBuoyancy(this, m_fBuoyancy, &point, &impulse)) {
		bIsInWater = true;
		SetIsStatic(false);
		ApplyMoveForce(impulse);
		ApplyTurnForce(impulse, point);
		float fTimeStep = Pow(0.97f, CTimer::GetTimeStep());
		m_vecMoveSpeed *= fTimeStep;
		m_vecTurnSpeed *= fTimeStep;
	}
	if ((GetModelIndex() == MI_EXPLODINGBARREL || GetModelIndex() == MI_PETROLPUMP) && bHasBeenDamaged && bIsVisible
		&& (CGeneral::GetRandomNumber() & 0x1F) == 10) {
		bExplosionProof = true;
		bIsVisible = false;
		bUsesCollision = false;
		bAffectedByGravity = false;
		m_vecMoveSpeed = CVector(0.0f, 0.0f, 0.0f);
	}
}

void 
CObject::Teleport(CVector vecPos)
{ 
	CWorld::Remove(this);
	m_matrix.GetPosition() = vecPos;
	m_matrix.UpdateRW();
	UpdateRwFrame();
	CWorld::Add(this);
}

void
CObject::Render(void)
{
	if(bDoNotRender)
		return;

	if(m_nRefModelIndex != -1 && ObjectCreatedBy == TEMP_OBJECT && bUseVehicleColours){
		CVehicleModelInfo *mi = (CVehicleModelInfo*)CModelInfo::GetModelInfo(m_nRefModelIndex);
		assert(mi->GetModelType() == MITYPE_VEHICLE);
		mi->SetVehicleColour(m_colour1, m_colour2);
	}

	CEntity::Render();
}

bool
CObject::SetupLighting(void)
{
	DeActivateDirectional();
	SetAmbientColours();

	if(bRenderScorched){
		WorldReplaceNormalLightsWithScorched(Scene.world, 0.1f);
		return true;
	}
	return false;
}

void
CObject::RemoveLighting(bool reset)
{
	if(reset)
		WorldReplaceScorchedLightsWithNormal(Scene.world);
}

void 
CObject::ObjectDamage(float amount) 
{
	if (!m_nCollisionDamageEffect || !bUsesCollision)
		return;
	static int8 nFrameGen = 0;
	bool bBodyCastDamageEffect = false;
	if (GetModelIndex() == MI_BODYCAST) {
		if (amount > 50.0f)
			nBodyCastHealth = (int16)(nBodyCastHealth - 0.5f * amount);
		if (nBodyCastHealth < 0)
			nBodyCastHealth = 0;
		if (nBodyCastHealth < 200)
			bBodyCastDamageEffect = true;
		amount = 0.0f;
	}
	if ((amount * m_fCollisionDamageMultiplier > 150.0f || bBodyCastDamageEffect) && m_nCollisionDamageEffect) {
		const CVector& vecPos = m_matrix.GetPosition();
		const float fDirectionZ = 0.0002f * amount;
		switch (m_nCollisionDamageEffect)
		{
		case DAMAGE_EFFECT_CHANGE_MODEL: 
			bRenderDamaged = true;
			break;
		case DAMAGE_EFFECT_SPLIT_MODEL:
			break;
		case DAMAGE_EFFECT_SMASH_COMPLETELY:
			bIsVisible = false;
			bUsesCollision = false;
			SetIsStatic(true);
			bExplosionProof = true;
			SetMoveSpeed(0.0f, 0.0f, 0.0f);
			SetTurnSpeed(0.0f, 0.0f, 0.0f);
			break;
		case DAMAGE_EFFECT_CHANGE_THEN_SMASH:
			if (!bRenderDamaged) {
				bRenderDamaged = true;
			}
			else {
				bIsVisible = false;
				bUsesCollision = false;
				SetIsStatic(true);
				bExplosionProof = true;
				SetMoveSpeed(0.0f, 0.0f, 0.0f);
				SetTurnSpeed(0.0f, 0.0f, 0.0f);
			}
			break;
		case DAMAGE_EFFECT_SMASH_CARDBOARD_COMPLETELY: {
			bIsVisible = false;
			bUsesCollision = false;
			SetIsStatic(true);
			bExplosionProof = true;
			SetMoveSpeed(0.0f, 0.0f, 0.0f);
			SetTurnSpeed(0.0f, 0.0f, 0.0f);
			const RwRGBA color = { 96, 48, 0, 255 };
			for (int32 i = 0; i < 25; i++) {
				CVector vecDir(CGeneral::GetRandomNumberInRange(-0.35f, 0.35f),
					CGeneral::GetRandomNumberInRange(-0.35f, 0.35f),
					CGeneral::GetRandomNumberInRange(0.1f, 0.25f) + fDirectionZ);
				++nFrameGen;
				int32 currentFrame = nFrameGen & 3;
				float fRandom = CGeneral::GetRandomNumberInRange(0.01f, 1.0f);
				RwRGBA randomColor = { uint8(color.red * fRandom), uint8(color.green * fRandom) , color.blue, color.alpha };
				float fSize = CGeneral::GetRandomNumberInRange(0.02f, 0.20f);
				int32 nRotationSpeed = CGeneral::GetRandomNumberInRange(-40, 40);
				CParticle::AddParticle(PARTICLE_CAR_DEBRIS, vecPos, vecDir, nil, fSize, randomColor, nRotationSpeed, 0, currentFrame, 0);
			}
			PlayOneShotScriptObject(SCRIPT_SOUND_BOX_DESTROYED_2, vecPos);
			break;
		}
		case DAMAGE_EFFECT_SMASH_WOODENBOX_COMPLETELY: {
			bIsVisible = false;
			bUsesCollision = false;
			SetIsStatic(true);
			bExplosionProof = true;
			SetMoveSpeed(0.0f, 0.0f, 0.0f);
			SetTurnSpeed(0.0f, 0.0f, 0.0f);
			const RwRGBA color = { 128, 128, 128, 255 };
			for (int32 i = 0; i < 45; i++) {
				CVector vecDir(CGeneral::GetRandomNumberInRange(-0.35f, 0.35f),
					CGeneral::GetRandomNumberInRange(-0.35f, 0.35f),
					CGeneral::GetRandomNumberInRange(0.1f, 0.25f) + fDirectionZ);
				++nFrameGen;
				int32 currentFrame = nFrameGen & 3;
				float fRandom = CGeneral::GetRandomNumberInRange(0.5f, 1.0f);
				RwRGBA randomColor = { uint8(color.red * fRandom), uint8(color.green * fRandom), uint8(color.blue * fRandom), color.alpha };
				float fSize = CGeneral::GetRandomNumberInRange(0.02f, 0.20f);
				int32 nRotationSpeed = CGeneral::GetRandomNumberInRange(-40, 40);
				CParticle::AddParticle(PARTICLE_CAR_DEBRIS, vecPos, vecDir, nil, fSize, randomColor, nRotationSpeed, 0, currentFrame, 0);
			}
			PlayOneShotScriptObject(SCRIPT_SOUND_BOX_DESTROYED_1, vecPos);
			break;
		}
		case DAMAGE_EFFECT_SMASH_TRAFFICCONE_COMPLETELY: {
			bIsVisible = false;
			bUsesCollision = false;
			SetIsStatic(true);
			bExplosionProof = true;
			SetMoveSpeed(0.0f, 0.0f, 0.0f);
			SetTurnSpeed(0.0f, 0.0f, 0.0f);
			const RwRGBA color1 = { 200, 0, 0, 255 };
			const RwRGBA color2 = { 200, 200, 200, 255 };
			for (int32 i = 0; i < 10; i++) {
				CVector vecDir(CGeneral::GetRandomNumberInRange(-0.35f, 0.35f),
					CGeneral::GetRandomNumberInRange(-0.35f, 0.35f),
					CGeneral::GetRandomNumberInRange(0.1f, 0.25f) + fDirectionZ);
				++nFrameGen;
				int32 currentFrame = nFrameGen & 3;
				RwRGBA color = color2;
				if (nFrameGen & 1)
					color = color1;
				float fSize = CGeneral::GetRandomNumberInRange(0.02f, 0.20f);
				int32 nRotationSpeed = CGeneral::GetRandomNumberInRange(-40, 40);
				CParticle::AddParticle(PARTICLE_CAR_DEBRIS, vecPos, vecDir, nil, fSize, color, nRotationSpeed, 0, currentFrame, 0);
			}
			PlayOneShotScriptObject(SCRIPT_SOUND_TIRE_COLLISION, vecPos);
			break;
		}
		case DAMAGE_EFFECT_SMASH_BARPOST_COMPLETELY: {
			bIsVisible = false;
			bUsesCollision = false;
			SetIsStatic(true);
			bExplosionProof = true;
			SetMoveSpeed(0.0f, 0.0f, 0.0f);
			SetTurnSpeed(0.0f, 0.0f, 0.0f);
			const RwRGBA color1 = { 200, 0, 0, 255 };
			const RwRGBA color2 = { 200, 200, 200, 255 };
			for (int32 i = 0; i < 32; i++) {
				CVector vecDir(CGeneral::GetRandomNumberInRange(-0.35f, 0.35f),
					CGeneral::GetRandomNumberInRange(-0.35f, 0.35f),
					CGeneral::GetRandomNumberInRange(0.1f, 0.25f) + fDirectionZ);
				++nFrameGen;
				int32 currentFrame = nFrameGen & 3;
				RwRGBA color = color2;
				if (nFrameGen & 1)
					color = color1;
				float fSize = CGeneral::GetRandomNumberInRange(0.02f, 0.20f);
				int32 nRotationSpeed = CGeneral::GetRandomNumberInRange(-40, 40);
				CParticle::AddParticle(PARTICLE_CAR_DEBRIS, vecPos, vecDir, nil, fSize, color, nRotationSpeed, 0, currentFrame, 0);
			}
			PlayOneShotScriptObject(SCRIPT_SOUND_METAL_COLLISION, vecPos);
			break;
		}
		}
	}
}

void
CObject::RefModelInfo(int32 modelId)
{
	m_nRefModelIndex = modelId;
	CModelInfo::GetModelInfo(modelId)->AddRef();
}

void 
CObject::Init(void) 
{ 
	m_type = ENTITY_TYPE_OBJECT;
	CObjectData::SetObjectData(GetModelIndex(), *this);
	m_nEndOfLifeTime = 0;
	ObjectCreatedBy = GAME_OBJECT;
	SetIsStatic(true);
	bIsPickup = false;
	bPickupObjWithMessage = false;
	bOutOfStock = false;
	bGlassCracked = false;
	bGlassBroken = false;
	bHasBeenDamaged = false;
	bUseVehicleColours = false;
	m_nRefModelIndex = -1;
	m_colour1 = 0;
	m_colour2 = 0;
	m_nBonusValue = 0;
	m_pCollidingEntity = nil;
	CColPoint point;
	CEntity* outEntity = nil;
	const CVector& vecPos = m_matrix.GetPosition();
	if (CWorld::ProcessVerticalLine(vecPos, vecPos.z - 10.0f, point, outEntity, true, false, false, false, false, false, nil))
		m_pCurSurface = outEntity;
	else
		m_pCurSurface = nil;
	if (GetModelIndex() == MI_BODYCAST)
		nBodyCastHealth = 1000;
	else if (GetModelIndex() == MI_BUOY)
		bTouchingWater = true;
}

bool
CObject::CanBeDeleted(void)
{
	switch (ObjectCreatedBy) {
		case GAME_OBJECT:
			return true;
		case MISSION_OBJECT:
			return false;
		case TEMP_OBJECT:
			return true;
		case CUTSCENE_OBJECT:
			return false;
		default:
			return true;
	}
}

void
CObject::DeleteAllMissionObjects()
{
	CObjectPool* objectPool = CPools::GetObjectPool();
	for (int32 i = 0; i < objectPool->GetSize(); i++) {
		CObject* pObject = objectPool->GetSlot(i);
		if (pObject && pObject->ObjectCreatedBy == MISSION_OBJECT) {
			CWorld::Remove(pObject);
			delete pObject;
		}
	}
}

void 
CObject::DeleteAllTempObjects() 
{
	CObjectPool* objectPool = CPools::GetObjectPool();
	for (int32 i = 0; i < objectPool->GetSize(); i++) {
		CObject* pObject = objectPool->GetSlot(i);
		if (pObject && pObject->ObjectCreatedBy == TEMP_OBJECT) {
			CWorld::Remove(pObject);
			delete pObject;
		}
	}
}

void 
CObject::DeleteAllTempObjectsInArea(CVector point, float fRadius) 
{
	CObjectPool *objectPool = CPools::GetObjectPool();
	for (int32 i = 0; i < objectPool->GetSize(); i++) {
		CObject *pObject = objectPool->GetSlot(i);
		if (pObject && pObject->ObjectCreatedBy == TEMP_OBJECT && (point - pObject->GetPosition()).MagnitudeSqr() < SQR(fRadius)) {
			CWorld::Remove(pObject);
			delete pObject;
		}
	}
}
