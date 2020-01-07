#include "common.h"
#include "patcher.h"
#include "main.h"
#include "Lights.h"
#include "Pools.h"
#include "Radar.h"
#include "Object.h"

WRAPPER void CObject::ObjectDamage(float amount) { EAXJMP(0x4BB240); }
WRAPPER void CObject::DeleteAllTempObjectInArea(CVector, float) { EAXJMP(0x4BBED0); }
WRAPPER void CObject::Init(void) { EAXJMP(0x4BAEC0); }
WRAPPER void CObject::ProcessControl(void) { EAXJMP(0x4BB040); }
WRAPPER void CObject::Teleport(CVector) { EAXJMP(0x4BBDA0); }

int16 &CObject::nNoTempObjects = *(int16*)0x95CCA2;
int16 &CObject::nBodyCastHealth = *(int16*)0x5F7D4C;	// 1000

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
	ObjectCreatedBy = 0;
	m_nEndOfLifeTime = 0;
//	m_nRefModelIndex = -1;	// duplicate
//	bUseVehicleColours = false;	// duplicate
	m_colour2 = 0;
	m_colour1 = m_colour2;
	field_172 = 0;
	bIsPickup = false;
	m_obj_flag2 = false;
	bOutOfStock = false;
	m_obj_flag8 = false;
	m_obj_flag10 = false;
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

CObject::~CObject(void)
{
	CRadar::ClearBlipForEntity(BLIP_OBJECT, CPools::GetObjectPool()->GetIndex(this));

	if(m_nRefModelIndex != -1)
		CModelInfo::GetModelInfo(m_nRefModelIndex)->RemoveRef();

	if(ObjectCreatedBy == TEMP_OBJECT && nNoTempObjects != 0)
		nNoTempObjects--;
}

void
CObject::Render(void)
{
	if(m_flagD80)
		return;

	if(m_nRefModelIndex != -1 && ObjectCreatedBy == TEMP_OBJECT && bUseVehicleColours){
		CVehicleModelInfo *mi = (CVehicleModelInfo*)CModelInfo::GetModelInfo(m_nRefModelIndex);
		assert(mi->m_type == MITYPE_VEHICLE);
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
CObject::RefModelInfo(int32 modelId)
{
	m_nRefModelIndex = modelId;
	CModelInfo::GetModelInfo(modelId)->AddRef();
}

class CObject_ : public CObject
{
public:
	void dtor(void) { this->CObject::~CObject(); }
	void Render_(void) { CObject::Render(); }
};

STARTPATCHES
	InjectHook(0x4BAE00, &CObject_::dtor, PATCH_JUMP);
	InjectHook(0x4BB1E0, &CObject_::Render_, PATCH_JUMP);
ENDPATCHES
