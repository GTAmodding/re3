#include "common.h"
#include "patcher.h"
#include "Vehicle.h"
#include "Pools.h"
#include "CarCtrl.h"
#include "ModelIndices.h"
#include "DMAudio.h"
#include "Radar.h"

bool &CVehicle::bWheelsOnlyCheat = *(bool *)0x95CD78;
bool &CVehicle::bAllDodosCheat = *(bool *)0x95CD75;
bool &CVehicle::bCheat3 = *(bool *)0x95CD66;
bool &CVehicle::bCheat4 = *(bool *)0x95CD65;
bool &CVehicle::bCheat5 = *(bool *)0x95CD64;
bool &CVehicle::m_bDisableMouseSteering = *(bool *)0x60252C;

void *CVehicle::operator new(size_t sz) { return CPools::GetVehiclePool()->New();  }
void *CVehicle::operator new(size_t sz, int handle) { return CPools::GetVehiclePool()->New(handle); }
void CVehicle::operator delete(void *p, size_t sz) { CPools::GetVehiclePool()->Delete((CVehicle*)p); }
void CVehicle::operator delete(void *p, int handle) { CPools::GetVehiclePool()->Delete((CVehicle*)p); }

CVehicle::~CVehicle()
{
	m_nAlarmState = 0;
	if (m_audioEntityId >= 0){
		DMAudio.DestroyEntity(m_audioEntityId);
		m_audioEntityId = -5;
	}
	CRadar::ClearBlipForEntity(BLIP_CAR, CPools::GetVehiclePool()->GetIndex(this));
	if (pDriver)
		pDriver->FlagToDestroyWhenNextProcessed();
	for (int i = 0; i < m_nNumMaxPassengers; i++){
		if (pPassengers[i])
			pPassengers[i]->FlagToDestroyWhenNextProcessed();
	}
	if (m_pCarFire)
		m_pCarFire->Extinguish();
	CCarCtrl::UpdateCarCount(this, true);
	if (bIsAmbulanceOnDuty){
		CCarCtrl::NumAmbulancesOnDuty--;
		bIsAmbulanceOnDuty = false;
	}
	if (bIsFiretruckOnDuty){
		CCarCtrl::NumFiretrucksOnDuty--;
		bIsFiretruckOnDuty = false;
	}
}

bool
CVehicle::IsLawEnforcementVehicle(void)
{
	switch (m_modelIndex) {
		case MI_FBICAR:
		case MI_POLICE:
		case MI_ENFORCER:
		case MI_PREDATOR:
		case MI_RHINO:
		case MI_BARRACKS:
			return true;
		default:
			return false;
	}
}

void
CVehicle::ChangeLawEnforcerState(bool enable)
{
	if (enable) {
		if (!bIsLawEnforcer) {
			bIsLawEnforcer = true;
			CCarCtrl::NumLawEnforcerCars++;
		}
	} else {
		if (bIsLawEnforcer) {
			bIsLawEnforcer = false;
			CCarCtrl::NumLawEnforcerCars--;
		}
	}
}

void
CVehicle::RemovePassenger(CPed *p)
{
	if (IsTrain()){
		for (int i = 0; i < 8; i++){
			if (pPassengers[i] == p) {
				pPassengers[i] = nil;
				m_nNumPassengers--;
				return;
			}
		}
		return;
	}
	for (int i = 0; i < m_nNumMaxPassengers; i++){
		if (pPassengers[i] == p){
			pPassengers[i] = nil;
			m_nNumPassengers--;
			return;
		}
	}
}

void
CVehicle::RemoveDriver(void)
{
	m_status = STATUS_ABANDONED;
	pDriver = nil;
}

bool
CVehicle::IsUpsideDown(void)
{
	return GetUp().z <= -0.9f;
}

STARTPATCHES
	InjectHook(0x552820, &CVehicle::ChangeLawEnforcerState, PATCH_JUMP);
	InjectHook(0x5520A0, &CVehicle::RemoveDriver, PATCH_JUMP);
ENDPATCHES