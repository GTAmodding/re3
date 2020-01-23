#include "common.h"
#include "patcher.h"
#include "Boat.h"
#include "HandlingMgr.h"
#include "RwHelper.h"
#include "ModelIndices.h"
#include "WaterLevel.h"
#include "Pools.h"
#include "World.h"

float &fShapeLength = *(float*)0x600E78;
float &fShapeTime   = *(float*)0x600E7C;
float &fRangeMult   = *(float*)0x600E80; //0.6f; // 0.75f gta 3
float &fTimeMult = *(float*)0x943008;

float MAX_WAKE_LENGTH = 50.0f;
float MIN_WAKE_INTERVAL = 1.0f;
float WAKE_LIFETIME = 400.0f;

CBoat * (&CBoat::apFrameWakeGeneratingBoats)[4] = *(CBoat * (*)[4])*(uintptr*)0x8620E0;

WRAPPER void CBoat::ProcessControl() { EAXJMP(0x53EF10); }
WRAPPER void CBoat::ProcessControlInputs(uint8) { EAXJMP(0x53EC70); }
WRAPPER void CBoat::BlowUpCar(CEntity* ent) { EAXJMP(0x541CB0); }

CBoat::CBoat(int mi, uint8 owner) : CVehicle(owner)
{
	CVehicleModelInfo *minfo = (CVehicleModelInfo*)CModelInfo::GetModelInfo(mi);
	m_vehType = VEHICLE_TYPE_BOAT;
	m_fAccelerate = 0.0f;
	m_fBrake = 0.0f;
	m_fSteeringLeftRight = 0.0f;
	m_nPadID = 0;
	m_fMovingHiRotation = 0.0f;
	SetModelIndex(mi);

	pHandling = mod_HandlingManager.GetHandlingData((eHandlingId)minfo->m_handlingId);
	minfo->ChooseVehicleColour(m_currentColour1, m_currentColour2);

	m_fMass = pHandling->fMass;
	m_fTurnMass = pHandling->fTurnMass / 2.0f;
	m_vecCentreOfMass = pHandling->CentreOfMass;
	m_fAirResistance = pHandling->Dimension.x * pHandling->Dimension.z / m_fMass;
	m_fElasticity = 0.1f;
	m_fBuoyancy = pHandling->fBuoyancy;
	m_fSteerAngle = 0.0f;
	m_fGasPedal = 0.0f;
	m_fBrakePedal = 0.0f;

	field_288 = 0.25f;
	field_28C = 0.35f;
	field_290 = 0.7f;
	field_294 = 0.998f;
	field_298 = 0.999f;
	field_29C = 0.85f;
	field_2A0 = 0.96f;
	field_2A4 = 0.96f;
	_unk2 = false;

	m_fTurnForceZ = 7.0f;
	field_2FC = 7.0f;
	m_vecMoveForce = CVector(0.0f, 0.0f, 0.0f);

	field_300 = 0;
	m_bBoatFlag1 = true;
	m_bBoatFlag2 = true;

	bIsInWater = true;

	unk1 = 0.0f;
	m_bIsAnchored = true;
	field_2C4 = -9999.99f;
	m_flagD8 = true;
	field_2CC = 0.0f;
	field_2D0 = 0;
	m_nNumWakePoints = 0;

	for (int16 i = 0; i < 32; i++)
		m_afWakePointLifeTime[i] = 0.0f;

	m_nAmmoInClip = 20;
}

void
CBoat::SetModelIndex(uint32 id)
{
	CEntity::SetModelIndex(id);
	SetupModelNodes();
}

void
CBoat::GetComponentWorldPosition(int32 component, CVector &pos)
{
	pos = *RwMatrixGetPos(RwFrameGetLTM(m_aBoatNodes[component]));
}

RxObjSpace3DVertex KeepWaterOutVertices[4];
RwImVertexIndex KeepWaterOutIndices[6];

void
CBoat::Render()
{
	CMatrix matrix;

	if (m_aBoatNodes[1] != nil) {
		matrix.Attach(&m_aBoatNodes[1]->modelling);

		CVector pos = matrix.GetPosition();
		matrix.SetRotateZOnly(m_fMovingHiRotation);
		matrix.Translate(pos);

		matrix.UpdateRW();
		if (CVehicle::bWheelsOnlyCheat) {
			RpAtomicRenderMacro((RpAtomic*)GetFirstObject(m_aBoatNodes[1]));
		}
	}
	m_fMovingHiRotation += 0.05f;
	((CVehicleModelInfo*)CModelInfo::GetModelInfo(GetModelIndex()))->SetVehicleColour(m_currentColour1, m_currentColour2);
	if (!CVehicle::bWheelsOnlyCheat)
		CEntity::Render();
	KeepWaterOutVertices[0].color = -1;
	KeepWaterOutIndices[0] = 0;
	KeepWaterOutVertices[1].color = -1;
	KeepWaterOutIndices[1] = 2;
	KeepWaterOutVertices[2].color = -1;
	KeepWaterOutIndices[2] = 1;
	KeepWaterOutVertices[3].color = -1;
	KeepWaterOutIndices[3] = 1;
	KeepWaterOutIndices[4] = 2;
	KeepWaterOutIndices[5] = 3;
	switch (GetModelIndex()) {
	case MI_SPEEDER:
		KeepWaterOutVertices[0].objVertex.x = -1.15f;
		KeepWaterOutVertices[0].objVertex.y = 3.61f;
		KeepWaterOutVertices[0].objVertex.z = 1.03f;
		KeepWaterOutVertices[1].objVertex.x = 1.15f;
		KeepWaterOutVertices[1].objVertex.y = 3.61f;
		KeepWaterOutVertices[1].objVertex.z = 1.03f;
		KeepWaterOutVertices[2].objVertex.x = -1.15f;
		KeepWaterOutVertices[2].objVertex.y = 0.06f;
		KeepWaterOutVertices[2].objVertex.z = 1.03f;
		KeepWaterOutVertices[3].objVertex.x = 1.15f;
		KeepWaterOutVertices[3].objVertex.y = 0.06f;
		KeepWaterOutVertices[3].objVertex.z = 1.03f;
		break;
	case MI_REEFER:
		KeepWaterOutVertices[2].objVertex.x = -1.9f;
		KeepWaterOutVertices[2].objVertex.y = 2.83f;
		KeepWaterOutVertices[2].objVertex.z = 1.0f;
		KeepWaterOutVertices[3].objVertex.x = 1.9f;
		KeepWaterOutVertices[3].objVertex.y = 2.83f;
		KeepWaterOutVertices[3].objVertex.z = 1.0f;
		KeepWaterOutVertices[0].objVertex.x = -1.66f;
		KeepWaterOutVertices[0].objVertex.y = -4.48f;
		KeepWaterOutVertices[0].objVertex.z = 0.83f;
		KeepWaterOutVertices[1].objVertex.x = 1.66f;
		KeepWaterOutVertices[1].objVertex.y = -4.48f;
		KeepWaterOutVertices[1].objVertex.z = 0.83f;
		break;
	case MI_PREDATOR:
	default:
		KeepWaterOutVertices[0].objVertex.x = -1.45f;
		KeepWaterOutVertices[0].objVertex.y = 1.9f;
		KeepWaterOutVertices[0].objVertex.z = 0.96f;
		KeepWaterOutVertices[1].objVertex.x = 1.45f;
		KeepWaterOutVertices[1].objVertex.y = 1.9f;
		KeepWaterOutVertices[1].objVertex.z = 0.96f;
		KeepWaterOutVertices[2].objVertex.x = -1.45f;
		KeepWaterOutVertices[2].objVertex.y = -3.75f;
		KeepWaterOutVertices[2].objVertex.z = 0.96f;
		KeepWaterOutVertices[3].objVertex.x = 1.45f;
		KeepWaterOutVertices[3].objVertex.y = -3.75f;
		KeepWaterOutVertices[3].objVertex.z = 0.96f;
		break;
	}
	KeepWaterOutVertices[0].u = 0.0f;
	KeepWaterOutVertices[0].v = 0.0f;
	KeepWaterOutVertices[1].u = 1.0f;
	KeepWaterOutVertices[1].v = 0.0f;
	KeepWaterOutVertices[2].u = 0.0f;
	KeepWaterOutVertices[2].v = 1.0f;
	KeepWaterOutVertices[3].u = 1.0f;
	KeepWaterOutVertices[3].v = 1.0f;
	RwRenderStateSet(rwRENDERSTATETEXTURERASTER, gpWaterRaster);
	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)TRUE);
	RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void*)FALSE);
	RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDZERO);
	RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDONE);
	if (!CVehicle::bWheelsOnlyCheat && RwIm3DTransform(KeepWaterOutVertices, 4, GetMatrix().m_attachment, rwIM3D_VERTEXUV)) {
		RwIm3DRenderIndexedPrimitive(rwPRIMTYPETRILIST, KeepWaterOutIndices, 6);
		RwIm3DEnd();
	}
	RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void*)TRUE);
	RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDSRCALPHA);
	RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDINVSRCALPHA);
}

void
CBoat::Teleport(CVector v)
{
	CWorld::Remove(this);
	GetPosition() = v;
	SetOrientation(0.0f, 0.0f, 0.0f);
	SetMoveSpeed(0.0f, 0.0f, 0.0f);
	SetTurnSpeed(0.0f, 0.0f, 0.0f);
	CWorld::Add(this);
}

bool
CBoat::IsSectorAffectedByWake(CVector2D sector, float fSize, CBoat **apBoats)
{
	uint8 numVerts = 0;
	
	if ( apFrameWakeGeneratingBoats[0] == NULL )
		return false;
	
	for ( int32 i = 0; i < 4; i++ )
	{
		CBoat *pBoat = apFrameWakeGeneratingBoats[i];
		if ( !pBoat )
			break;
		
		for ( int j = 0; j < pBoat->m_nNumWakePoints; j++ )
		{
			 float fDist = (WAKE_LIFETIME - pBoat->m_afWakePointLifeTime[j]) * fShapeTime + float(j) * fShapeLength + fSize;
			 
			 if ( Abs(pBoat->m_avec2dWakePoints[j].x - sector.x) < fDist
				&& Abs(pBoat->m_avec2dWakePoints[i].y - sector.y) < fDist )
			 {
				 apBoats[numVerts] = pBoat;
				 numVerts = 1; // += ?
				 break;
			 }
		}
	}
	
	return numVerts != 0;
}

float
CBoat::IsVertexAffectedByWake(CVector vecVertex, CBoat *pBoat)
{
	for ( int i = 0; i < pBoat->m_nNumWakePoints; i++ )
	{
		float fMaxDist = (WAKE_LIFETIME - pBoat->m_afWakePointLifeTime[i]) * fShapeTime + float(i) * fShapeLength;
		
		CVector2D vecDist = pBoat->m_avec2dWakePoints[i] - CVector2D(vecVertex);
		
		float fDist = vecDist.MagnitudeSqr();
		
		if ( fDist < SQR(fMaxDist) )
			return 1.0f - min(fRangeMult * Sqrt(fDist / SQR(fMaxDist)) + (WAKE_LIFETIME - pBoat->m_afWakePointLifeTime[i]) * fTimeMult, 1.0f);
	}

	return 0.0f;
}

void
CBoat::SetupModelNodes()
{
	m_aBoatNodes[0] = nil;
	m_aBoatNodes[1] = nil;
	m_aBoatNodes[2] = nil;
	m_aBoatNodes[3] = nil;
	CClumpModelInfo::FillFrameArray(GetClump(), m_aBoatNodes);
}

void
CBoat::FillBoatList()
{
	int16 frameId = 0;
	
	apFrameWakeGeneratingBoats[0] = nil;
	apFrameWakeGeneratingBoats[1] = nil;
	apFrameWakeGeneratingBoats[2] = nil;
	apFrameWakeGeneratingBoats[3] = nil;

	for (int i = CPools::GetVehiclePool()->GetSize() - 1; i >= 0; i--) {
		CBoat *boat = (CBoat *)(CPools::GetVehiclePool()->GetSlot(i));
		if (boat && boat->m_vehType == VEHICLE_TYPE_BOAT) {
			int16 nNumWakePoints = boat->m_nNumWakePoints;
			if (nNumWakePoints != 0) {
				if (frameId >= ARRAY_SIZE(apFrameWakeGeneratingBoats)) {
					int16 frameId2 = -1;
					for (int16 j = 0; j < ARRAY_SIZE(apFrameWakeGeneratingBoats); j++) {
						if (apFrameWakeGeneratingBoats[j]->m_nNumWakePoints < nNumWakePoints) {
							frameId2 = j;
							nNumWakePoints = apFrameWakeGeneratingBoats[j]->m_nNumWakePoints;
						}
					}

					if (frameId2 != -1)
						apFrameWakeGeneratingBoats[frameId2] = boat;
				} else {
					apFrameWakeGeneratingBoats[frameId++] = boat;
				}
			}
		}
	}
}

class CBoat_ : public CBoat
{
public:
	CBoat* ctor(int32 id, uint8 CreatedBy) { return ::new (this) CBoat(id, CreatedBy); }
	void dtor() { CBoat::~CBoat(); };
};

STARTPATCHES
	InjectHook(0x53E3E0, &CBoat_::ctor, PATCH_JUMP);
	InjectHook(0x53E790, &CBoat_::dtor, PATCH_JUMP);
	InjectHook(0x53E7D0, &CBoat::SetupModelNodes, PATCH_JUMP);
	InjectHook(0x542370, CBoat::IsSectorAffectedByWake, PATCH_JUMP);
	InjectHook(0x5424A0, CBoat::IsVertexAffectedByWake, PATCH_JUMP);
	InjectHook(0x542250, CBoat::FillBoatList, PATCH_JUMP);
ENDPATCHES
