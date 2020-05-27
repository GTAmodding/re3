#include "common.h"
#include "PedAttractor.h"

#include "General.h"
#include "Vehicle.h"
#include "World.h"

const int gcMaxSizeOfAtmQueue = 1;
const int gcMaxSizeOfSeatQueue = 1;
const int gcMaxSizeOfStopQueue = 5;
const int gcMaxSizeOfPizzaQueue = 5;
const int gcMaxSizeOfShelterQueue = 5;
const int gcMaxSizeOfIceCreamQueue = 1;

//--MIAMI: file done

std::vector<CVector> CPedShelterAttractor::ms_displacements;

CPedAttractorManager* GetPedAttractorManager()
{
	static CPedAttractorManager manager;
	return &manager;
}

CVehicleToEffect::CVehicleToEffect(CVehicle* pVehicle) : m_pVehicle(pVehicle)
{
	m_effects[1].col = CRGBA(0, 0, 0, 0);
	m_effects[1].type = EFFECT_PED_ATTRACTOR;
	m_effects[1].pos = CVector(2.0f, 1.0f, 0.0f);
	m_effects[1].pedattr.useDir = CVector(-1.0f, 0.0f, 0.0f);
	m_effects[1].pedattr.queueDir = CVector(-1.0f, 0.0f, 0.0f);
	m_effects[1].pedattr.type = ATTRACTOR_ICECREAM;

	m_effects[3].col = CRGBA(0, 0, 0, 0);
	m_effects[3].type = EFFECT_PED_ATTRACTOR;
	m_effects[3].pos = CVector(2.0f, -0.5f, 0.0f);
	m_effects[3].pedattr.useDir = CVector(-1.0f, 0.0f, 0.0f);
	m_effects[3].pedattr.queueDir = CVector(-1.0f, 0.0f, 0.0f);
	m_effects[3].pedattr.type = ATTRACTOR_ICECREAM;

	m_effects[0].col = CRGBA(0, 0, 0, 0);
	m_effects[0].type = EFFECT_PED_ATTRACTOR;
	m_effects[0].pos = CVector(-2.0f, 1.0f, 0.0f);
	m_effects[0].pedattr.useDir = CVector(1.0f, 0.0f, 0.0f);
	m_effects[0].pedattr.queueDir = CVector(1.0f, 0.0f, 0.0f);
	m_effects[0].pedattr.type = ATTRACTOR_ICECREAM;

	m_effects[2].col = CRGBA(0, 0, 0, 0);
	m_effects[2].type = EFFECT_PED_ATTRACTOR;
	m_effects[2].pos = CVector(-2.0f, -0.5f, 0.0f);
	m_effects[2].pedattr.useDir = CVector(1.0f, 0.0f, 0.0f);
	m_effects[2].pedattr.queueDir = CVector(1.0f, 0.0f, 0.0f);
	m_effects[2].pedattr.type = ATTRACTOR_ICECREAM;
}

CVehicleToEffect& CVehicleToEffect::From(const CVehicleToEffect& other)
{
	m_pVehicle = other.m_pVehicle;
	for (int i = 0; i < NUM_ATTRACTORS_FOR_ICECREAM_VAN; i++) {
		m_effects[i].col = other.m_effects[i].col;
		m_effects[i].type = other.m_effects[i].type;
		m_effects[i].pos = other.m_effects[i].pos;
		m_effects[i].pedattr = other.m_effects[i].pedattr;
	}
	return *this;
}

const C2dEffect* CVehicleToEffect::ChooseEffect(const CVector& pos) const
{
	if (!m_pVehicle)
		return nil;
	if (DotProduct(pos - m_pVehicle->GetPosition(), m_pVehicle->GetRight()) > 0.0f) {
		if (DotProduct(pos - m_pVehicle->GetPosition(), m_pVehicle->GetForward()) > 0.0f)
			return &m_effects[0];
		else
			return &m_effects[2];
	}
	else {
		if (DotProduct(pos - m_pVehicle->GetPosition(), m_pVehicle->GetForward()) > 0.0f)
			return &m_effects[1];
		else
			return &m_effects[3];
	}
}

bool CVehicleToEffect::HasThisEffect(C2dEffect* pEffect) const
{
	for (int i = 0; i < NUM_ATTRACTORS_FOR_ICECREAM_VAN; i++) {
		if (pEffect == &m_effects[i])
			return true;
	}
	return false;
}

const C2dEffect* CPedAttractorManager::GetEffectForIceCreamVan(CVehicle* pVehicle, const CVector& pos)
{
	if (!vVehicleToEffect.empty()) {
		for (std::vector<CVehicleToEffect>::const_iterator assoc = vVehicleToEffect.cbegin(); assoc != vVehicleToEffect.cend(); ++assoc) {
			if (assoc->GetVehicle() == pVehicle)
				return assoc->ChooseEffect(pos);
		}
	}
	CVehicleToEffect effect(pVehicle);
	vVehicleToEffect.push_back(effect);
	return effect.ChooseEffect(pos);
}

CVehicle* CPedAttractorManager::GetIceCreamVanForEffect(C2dEffect* pEffect)
{
	if (vVehicleToEffect.empty())
		return nil;
	for (std::vector<CVehicleToEffect>::const_iterator assoc = vVehicleToEffect.cbegin(); assoc != vVehicleToEffect.cend(); ++assoc) {
		if (assoc->HasThisEffect(pEffect))
			return assoc->GetVehicle();
	}
	return nil;
}

const CPedAttractor* CPedAttractorManager::FindAssociatedAttractor(const C2dEffect* pEffect, std::vector<CPedAttractor*>& vecAttractors)
{
	if (vecAttractors.empty())
		return nil;
	for (std::vector<CPedAttractor*>::const_iterator attractor = vecAttractors.cbegin(); attractor != vecAttractors.cend(); ++attractor) {
		if ((*attractor)->GetEffect() == pEffect)
			return *attractor;
	}
	return nil;
}

void CPedAttractorManager::RemoveIceCreamVanEffects(C2dEffect* pEffect)
{
	CVehicle* pVehicle = GetIceCreamVanForEffect(pEffect);
	if (!pVehicle)
		return;
	if (vVehicleToEffect.empty())
		return;
	for (std::vector<CVehicleToEffect>::const_iterator assoc = vVehicleToEffect.cbegin(); assoc != vVehicleToEffect.cend();) {
		if (assoc->GetVehicle() != pVehicle)
			return;
		size_t total = 0;
		for (size_t j = 0; j < NUM_ATTRACTORS_FOR_ICECREAM_VAN; j++) {
			if (FindAssociatedAttractor(assoc->GetEffect(j), vIceCreamAttractors))
				total++;
		}
		if (total > 0)
			assoc++;
		else
			assoc = vVehicleToEffect.erase(assoc);
	}
}

CPedAttractor::CPedAttractor(C2dEffect* pEffect, const CMatrix& matrix, int32 maxpeds, float qdist, float waitTime, float approachTime, float distance, float headingdiff, float posdisp, float headdisp) :
	p2dEffect(pEffect),
	m_nMaxPedsInAttractor(maxpeds),
	m_fQueueDistance(qdist),
	m_fTimeInWaitQueue(waitTime),
	m_fTimeInApproachingQueue(approachTime),
	m_fDistanceToUseAttractor(distance),
	m_fAcceptableHeading(headingdiff),
	m_fMaxPositionDisplacement(posdisp),
	m_fMaxHeadingDisplacement(headdisp)
{
	CPedAttractorManager::ComputeEffectPos(pEffect, matrix, vecEffectPos);
	CPedAttractorManager::ComputeEffectQueueDir(pEffect, matrix, vecQueueDir);
	CPedAttractorManager::ComputeEffectUseDir(pEffect, matrix, vecUseDir);
}

void CPedPizzaAttractor::UpdatePedStateOnDeparture(CPed* pPed) const
{
	if (pPed->m_nPedMoney > 10)
		pPed->m_nPedMoney -= 10;
	else
		pPed->m_nPedMoney = 0;
}

void CPedAtmAttractor::UpdatePedStateOnDeparture(CPed* pPed) const
{
	pPed->m_nPedMoney += 20 * CGeneral::GetRandomNumberInRange(1, 51);
};

float CPedAttractor::ComputeDeltaHeading() const
{
	return CGeneral::GetRandomNumberInRange(-m_fMaxHeadingDisplacement, m_fMaxHeadingDisplacement);
}

float CPedAttractor::ComputeDeltaPos() const
{
	return CGeneral::GetRandomNumberInRange(-m_fMaxPositionDisplacement, m_fMaxPositionDisplacement);
}

void CPedAttractor::ComputeAttractTime(int32 id, bool approacher, float& time) const
{
	if (approacher)
		time = m_fTimeInApproachingQueue;
	else
		time = m_fTimeInWaitQueue;
}

void CPedAttractor::ComputeAttractPos(int32 qid, CVector& pos) const
{
	if (!p2dEffect)
		return;
	pos = vecEffectPos - qid * vecQueueDir * m_fQueueDistance;
	if (qid != 0) {
		pos.x += ComputeDeltaPos();
		pos.y += ComputeDeltaPos();
	}
}

CVector CPedShelterAttractor::GetDisplacement(int32 qid) const
{
	if (ms_displacements.empty()) {
		int i = 0;
		while (i < gcMaxSizeOfShelterQueue) {
			float fRandomAngle = CGeneral::GetRandomNumberInRange(0.0f, TWOPI);
			float fRandomOffset = CGeneral::GetRandomNumberInRange(0.0f, 2.0f);
			CVector vecDisplacement(fRandomOffset * Sin(fRandomAngle), fRandomOffset * Cos(fRandomAngle), 0.0f);
			bool close = false;
			for (std::vector<CVector>::const_iterator v = ms_displacements.cbegin(); v != ms_displacements.cend(); ++v) {
				if ((*v - vecDisplacement).Magnitude() < 1.0f) {
					close = true;
					break;
				}
			}
			if (!close) {
				ms_displacements.push_back(vecDisplacement);
				i++;
			}
		}
	}
	return ms_displacements[qid];
}

void CPedShelterAttractor::ComputeAttractPos(int32 qid, CVector& pos) const
{
	if (!p2dEffect)
		return;
	pos = vecEffectPos + GetDisplacement(qid);
}

void CPedAttractor::ComputeAttractHeading(int32 qid, float& heading) const
{
	heading = CGeneral::GetRadianAngleBetweenPoints(qid != 0 ? vecQueueDir.x : vecUseDir.x, qid != 0 ? vecQueueDir.y : vecUseDir.y, 0.0f, 0.0f);
	if (qid != 0)
		heading += ComputeDeltaHeading();
}

void CPedShelterAttractor::ComputeAttractHeading(int32 qid, float& heading) const
{
	heading = CGeneral::GetRandomNumberInRange(0.0f, TWOPI);
}

bool CPedAttractor::RegisterPed(CPed* pPed)
{
	for (std::vector<CPed*>::const_iterator pPedIt = vApproachingQueue.cbegin(); pPedIt != vApproachingQueue.cend(); ++pPedIt) {
		if (*pPedIt == pPed) {
			vApproachingQueue.erase(pPedIt);
			return false;
		}
	}
	if (GetNoOfRegisteredPeds() >= m_nMaxPedsInAttractor)
		return 0;
	vApproachingQueue.push_back(pPed);
	CVector pos;
	float heading;
	float time;
	int32 slot = ComputeFreeSlot();
	ComputeAttractPos(slot, pos);
	ComputeAttractHeading(slot, heading);
	ComputeAttractTime(slot, false, time);
	pPed->SetNewAttraction(this, pos, heading, time, slot);
	return true;
}

static bool IsPedUsingAttractorOfThisType(int8 type, CPed* pPed)
{
	switch (type) {
	case ATTRACTOR_ATM:
		if (pPed->m_objective == OBJECTIVE_USE_ATM_ATTRACTOR)
			return true;
		break;
	case ATTRACTOR_SEAT:
		if (pPed->m_objective == OBJECTIVE_USE_SEAT_ATTRACTOR)
			return true;
		break;
	case ATTRACTOR_STOP:
		if (pPed->m_objective == OBJECTIVE_USE_STOP_ATTRACTOR || pPed->m_objective == OBJECTIVE_WAIT_FOR_BUS || pPed->m_objective == OBJECTIVE_IDLE)
			return true;
		break;
	case ATTRACTOR_PIZZA:
		if (pPed->m_objective == OBJECTIVE_USE_PIZZA_ATTRACTOR || pPed->m_objective == OBJECTIVE_IDLE)
			return true;
		break;
	case ATTRACTOR_SHELTER:
		if (pPed->m_objective == OBJECTIVE_USE_SHELTER_ATTRACTOR || pPed->m_objective == OBJECTIVE_WAIT_FOR_RAIN_TO_END)
			return true;
		break;
	case ATTRACTOR_ICECREAM:
		if (pPed->m_objective == OBJECTIVE_USE_ICECREAM_ATTRACTOR || pPed->m_objective == OBJECTIVE_PURCHASE_ICECREAM)
			return true;
		break;
	}
	return false;
}

bool CPedAttractor::DeRegisterPed(CPed* pPed)
{
	for (std::vector<CPed*>::const_iterator pPedIt = vApproachingQueue.cbegin(); pPedIt != vApproachingQueue.cend(); ++pPedIt) {
		if (*pPedIt != pPed)
			continue;
		pPed->m_attractor = nil;
		pPed->m_positionInQueue = -1;
		pPed->bHasAlreadyUsedAttractor = true;
		
		if (IsPedUsingAttractorOfThisType(p2dEffect->pedattr.type, pPed))
			pPed->SetObjective(OBJECTIVE_NONE);
		else if (pPed->GetPedState() != PED_IDLE && pPed->GetPedState() != PED_NONE) {
			vApproachingQueue.erase(pPedIt);
			return true;
		}
		pPed->SetWanderPath(CGeneral::GetNodeHeadingFromVector(-vecQueueDir.x, -vecQueueDir.y));
		vApproachingQueue.erase(pPedIt);
		return true;
	}
	return BroadcastDeparture(pPed);
}

bool CPedAttractor::BroadcastArrival(CPed* pPed)
{
	for (std::vector<CPed*>::const_iterator pPedIt = vWaitingQueue.cbegin(); pPedIt != vWaitingQueue.cend(); ++pPedIt) {
		if (*pPedIt == pPed)
			return false;
	}
	vWaitingQueue.push_back(pPed);
	for (std::vector<CPed*>::const_iterator pPedIt = vApproachingQueue.cbegin(); pPedIt != vApproachingQueue.cend(); ++pPedIt) {
		if (*pPedIt == pPed) {
			vApproachingQueue.erase(pPedIt);
			break;
		}
	}
	for (std::vector<CPed*>::iterator pPedIt = vApproachingQueue.begin(); pPedIt != vApproachingQueue.end(); ++pPedIt) {
		CPed* pPed = *pPedIt;
		CVector pos;
		float heading;
		float time;
		int32 slot = ComputeFreeSlot();
		ComputeAttractPos(slot, pos);
		ComputeAttractHeading(slot, heading);
		ComputeAttractTime(slot, false, time);
		pPed->SetNewAttraction(this, pos, heading, time, slot);
	}
	return true;
}

bool CPedAttractor::BroadcastDeparture(CPed* pPed)
{
	int qid = -1;
	for (size_t i = 0; i < vWaitingQueue.size(); i++){
		if (vWaitingQueue[i] == pPed)
			qid = i;
	}
	if (qid < 0)
		return false;
	for (size_t i = qid + 1; i < vWaitingQueue.size(); i++) {
		CVector pos;
		float heading;
		float time;
		ComputeAttractPos(i - 1, pos);
		ComputeAttractHeading(i - 1, heading);
		ComputeAttractTime(i - 1, true, time);
		pPed->SetNewAttraction(this, pos, heading, time, i - 1);
	}
	pPed->m_attractor = nil;
	pPed->m_positionInQueue = -1;
	pPed->bHasAlreadyUsedAttractor = true;
	if (!IsPedUsingAttractorOfThisType(p2dEffect->pedattr.type, pPed)) {
		if (pPed->GetPedState() == PED_IDLE || pPed->GetPedState() == PED_NONE)
			pPed->SetWanderPath(CGeneral::GetNodeHeadingFromVector(-vecQueueDir.x, -vecQueueDir.y));
	}
	else if (qid == 0)
		pPed->SetWanderPath(CGeneral::GetNodeHeadingFromVector(vecQueueDir.x, vecQueueDir.y));
	else if (qid == vWaitingQueue.size() - 1)
		pPed->SetWanderPath(CGeneral::GetNodeHeadingFromVector(-vecQueueDir.x, -vecQueueDir.y));
	else
		pPed->SetWanderPath(CGeneral::GetNodeHeadingFromVector(-vecQueueDir.y, -vecQueueDir.z));
	vWaitingQueue.erase(vWaitingQueue.cbegin() + qid);
	for (std::vector<CPed*>::iterator pPedIt = vApproachingQueue.begin(); pPedIt != vApproachingQueue.end(); ++pPedIt) {
		CPed* pPed = *pPedIt;
		CVector pos;
		float heading;
		float time;
		int32 slot = ComputeFreeSlot();
		ComputeAttractPos(slot, pos);
		ComputeAttractHeading(slot, heading);
		ComputeAttractTime(slot, false, time);
		pPed->SetNewAttraction(this, pos, heading, time, slot);
	}
	return true;
}

bool CPedShelterAttractor::BroadcastDeparture(CPed* pPed)
{
	int qid = -1;
	for (size_t i = 0; i < vWaitingQueue.size(); i++) {
		if (vWaitingQueue[i] == pPed)
			qid = i;
	}
	if (qid < 0)
		return false;
	pPed->m_attractor = nil;
	pPed->m_positionInQueue = -1;
	pPed->bHasAlreadyUsedAttractor = true;
	if (!IsPedUsingAttractorOfThisType(p2dEffect->pedattr.type, pPed)) {
		if (pPed->GetPedState() == PED_IDLE || pPed->GetPedState() == PED_NONE)
			pPed->SetWanderPath(CGeneral::GetNodeHeadingFromVector(-vecQueueDir.x, -vecQueueDir.y));
	}
	else if (qid == 0)
		pPed->SetWanderPath(CGeneral::GetNodeHeadingFromVector(vecQueueDir.x, vecQueueDir.y));
	else if (qid == vWaitingQueue.size() - 1)
		pPed->SetWanderPath(CGeneral::GetNodeHeadingFromVector(-vecQueueDir.x, -vecQueueDir.y));
	else
		pPed->SetWanderPath(CGeneral::GetNodeHeadingFromVector(-vecQueueDir.y, -vecQueueDir.z));
	vWaitingQueue.erase(vWaitingQueue.cbegin() + qid);
	for (std::vector<CPed*>::iterator pPedIt = vApproachingQueue.begin(); pPedIt != vApproachingQueue.end(); ++pPedIt) {
		CPed* pPed = *pPedIt;
		CVector pos;
		float heading;
		float time;
		int32 slot = ComputeFreeSlot();
		ComputeAttractPos(slot, pos);
		ComputeAttractHeading(slot, heading);
		ComputeAttractTime(slot, false, time);
		pPed->SetNewAttraction(this, pos, heading, time, slot);
	}
	return true;
}

bool CPedAttractor::IsRegisteredWithPed(CPed* pPed) const
{
	for (std::vector<CPed*>::const_iterator pPedIt = vWaitingQueue.cbegin(); pPedIt != vWaitingQueue.cend(); ++pPedIt) {
		if (*pPedIt == pPed)
			return true;
	}
	for (std::vector<CPed*>::const_iterator pPedIt = vApproachingQueue.cbegin(); pPedIt != vApproachingQueue.cend(); ++pPedIt) {
		if (*pPedIt == pPed) {
			return true;
		}
	}
	return false;
}

bool CPedAttractor::IsInQueue(CPed* pPed) const
{
	for (std::vector<CPed*>::const_iterator pPedIt = vWaitingQueue.cbegin(); pPedIt != vWaitingQueue.cend(); ++pPedIt) {
		if (*pPedIt == pPed)
			return true;
	}
	return false;
}

CPedAttractor* CPedAttractorManager::RegisterPedWithAttractor(CPed* pPed, C2dEffect* pEffect, const CMatrix& matrix)
{
	if (pEffect->type != EFFECT_PED_ATTRACTOR)
		return nil;
	if (IsPedRegisteredWithEffect(pPed))
		return nil;
	switch (pEffect->pedattr.type) {
	case ATTRACTOR_ATM: return RegisterPed(pPed, pEffect, matrix, vAtmAttractors);
	case ATTRACTOR_SEAT: return RegisterPed(pPed, pEffect, matrix, vSeatAttractors);
	case ATTRACTOR_STOP: return RegisterPed(pPed, pEffect, matrix, vStopAttractors);
	case ATTRACTOR_PIZZA: return RegisterPed(pPed, pEffect, matrix, vPizzaAttractors);
	case ATTRACTOR_SHELTER: return RegisterPed(pPed, pEffect, matrix, vShelterAttractors);
	case ATTRACTOR_ICECREAM: return RegisterPed(pPed, pEffect, matrix, vIceCreamAttractors);
	}
	return nil;
}

bool CPedAttractorManager::DeRegisterPed(CPed* pPed, CPedAttractor* pAttractor)
{
	if (!pAttractor)
		return false;
	if (pAttractor->GetEffect()->type != EFFECT_PED_ATTRACTOR)
		return nil;
	if (!IsPedRegisteredWithEffect(pPed))
		return nil;
	switch (pAttractor->GetEffect()->pedattr.type) {
	case ATTRACTOR_ATM: return DeRegisterPed(pPed, pAttractor, vAtmAttractors);
	case ATTRACTOR_SEAT: return DeRegisterPed(pPed, pAttractor, vSeatAttractors);
	case ATTRACTOR_STOP: return DeRegisterPed(pPed, pAttractor, vStopAttractors);
	case ATTRACTOR_PIZZA: return DeRegisterPed(pPed, pAttractor, vPizzaAttractors);
	case ATTRACTOR_SHELTER: return DeRegisterPed(pPed, pAttractor, vShelterAttractors);
	case ATTRACTOR_ICECREAM: return DeRegisterPed(pPed, pAttractor, vIceCreamAttractors);
	}
	return nil;
}

bool CPedAttractorManager::BroadcastArrival(CPed* pPed, CPedAttractor* pAttractor)
{
	if (!pAttractor)
		return false;
	if (pAttractor->GetEffect()->type != EFFECT_PED_ATTRACTOR)
		return nil;
	if (!IsPedRegisteredWithEffect(pPed))
		return nil;
	switch (pAttractor->GetEffect()->pedattr.type) {
	case ATTRACTOR_ATM: return BroadcastArrival(pPed, pAttractor, vAtmAttractors);
	case ATTRACTOR_SEAT: return BroadcastArrival(pPed, pAttractor, vSeatAttractors);
	case ATTRACTOR_STOP: return BroadcastArrival(pPed, pAttractor, vStopAttractors);
	case ATTRACTOR_PIZZA: return BroadcastArrival(pPed, pAttractor, vPizzaAttractors);
	case ATTRACTOR_SHELTER: return BroadcastArrival(pPed, pAttractor, vShelterAttractors);
	case ATTRACTOR_ICECREAM: return BroadcastArrival(pPed, pAttractor, vIceCreamAttractors);
	}
	return nil;
}

bool CPedAttractorManager::BroadcastDeparture(CPed* pPed, CPedAttractor* pAttractor)
{
	if (!pAttractor)
		return false;
	if (pAttractor->GetEffect()->type != EFFECT_PED_ATTRACTOR)
		return nil;
	if (!IsPedRegisteredWithEffect(pPed))
		return nil;
	switch (pAttractor->GetEffect()->pedattr.type) {
	case ATTRACTOR_ATM: return BroadcastDeparture(pPed, pAttractor, vAtmAttractors);
	case ATTRACTOR_SEAT: return BroadcastDeparture(pPed, pAttractor, vSeatAttractors);
	case ATTRACTOR_STOP: return BroadcastDeparture(pPed, pAttractor, vStopAttractors);
	case ATTRACTOR_PIZZA: return BroadcastDeparture(pPed, pAttractor, vPizzaAttractors);
	case ATTRACTOR_SHELTER: return BroadcastDeparture(pPed, pAttractor, vShelterAttractors);
	case ATTRACTOR_ICECREAM: return BroadcastDeparture(pPed, pAttractor, vIceCreamAttractors);
	}
	return nil;
}

bool CPedAttractorManager::IsAtHeadOfQueue(CPed* pPed, CPedAttractor* pAttractor)
{
	if (!pAttractor)
		return false;
	if (pAttractor->GetEffect()->type != EFFECT_PED_ATTRACTOR)
		return nil;
	if (!IsPedRegisteredWithEffect(pPed))
		return nil;
	switch (pAttractor->GetEffect()->pedattr.type) {
	case ATTRACTOR_ATM: return IsAtHeadOfQueue(pPed, pAttractor, vAtmAttractors);
	case ATTRACTOR_SEAT: return IsAtHeadOfQueue(pPed, pAttractor, vSeatAttractors);
	case ATTRACTOR_STOP: return IsAtHeadOfQueue(pPed, pAttractor, vStopAttractors);
	case ATTRACTOR_PIZZA: return IsAtHeadOfQueue(pPed, pAttractor, vPizzaAttractors);
	case ATTRACTOR_SHELTER: return IsAtHeadOfQueue(pPed, pAttractor, vShelterAttractors);
	case ATTRACTOR_ICECREAM: return IsAtHeadOfQueue(pPed, pAttractor, vIceCreamAttractors);
	}
	return nil;
}

bool CPedAttractorManager::IsInQueue(CPed* pPed, CPedAttractor* pAttractor)
{
	if (!pAttractor)
		return false;
	if (pAttractor->GetEffect()->type != EFFECT_PED_ATTRACTOR)
		return nil;
	if (!IsPedRegisteredWithEffect(pPed))
		return nil;
	switch (pAttractor->GetEffect()->pedattr.type) {
	case ATTRACTOR_ATM: return IsInQueue(pPed, pAttractor, vAtmAttractors);
	case ATTRACTOR_SEAT: return IsInQueue(pPed, pAttractor, vSeatAttractors);
	case ATTRACTOR_STOP: return IsInQueue(pPed, pAttractor, vStopAttractors);
	case ATTRACTOR_PIZZA: return IsInQueue(pPed, pAttractor, vPizzaAttractors);
	case ATTRACTOR_SHELTER: return IsInQueue(pPed, pAttractor, vShelterAttractors);
	case ATTRACTOR_ICECREAM: return IsInQueue(pPed, pAttractor, vIceCreamAttractors);
	}
	return nil;
}

bool CPedAttractorManager::HasEmptySlot(const C2dEffect* pEffect)
{
	if (!pEffect)
		return false;
	if (pEffect->type != EFFECT_PED_ATTRACTOR)
		return nil;
	const CPedAttractor* pAttractor;
	switch (pEffect->pedattr.type) {
	case ATTRACTOR_ATM: pAttractor = FindAssociatedAttractor(pEffect, vAtmAttractors); break;
	case ATTRACTOR_SEAT: pAttractor = FindAssociatedAttractor(pEffect, vSeatAttractors); break;
	case ATTRACTOR_STOP: pAttractor = FindAssociatedAttractor(pEffect, vStopAttractors); break;
	case ATTRACTOR_PIZZA: pAttractor = FindAssociatedAttractor(pEffect, vPizzaAttractors); break;
	case ATTRACTOR_SHELTER: pAttractor = FindAssociatedAttractor(pEffect, vShelterAttractors); break;
	case ATTRACTOR_ICECREAM: pAttractor = FindAssociatedAttractor(pEffect, vIceCreamAttractors); break;
	default: return true;
	}
	if (!pAttractor)
		return true;
	return pAttractor->GetNoOfRegisteredPeds() < pAttractor->GetMaxPedsInAttractor();
}

bool CPedAttractorManager::IsPedRegisteredWithEffect(CPed* pPed)
{
	return IsPedRegistered(pPed, vAtmAttractors) ||
		IsPedRegistered(pPed, vSeatAttractors) ||
		IsPedRegistered(pPed, vStopAttractors) ||
		IsPedRegistered(pPed, vPizzaAttractors) ||
		IsPedRegistered(pPed, vShelterAttractors) ||
		IsPedRegistered(pPed, vIceCreamAttractors);
}

void CPedAttractorManager::ComputeEffectPos(const C2dEffect* pEffect, const CMatrix& matrix, CVector& pos)
{
	pos = matrix.GetPosition() + Multiply3x3(matrix, pEffect->pos);
}

void CPedAttractorManager::ComputeEffectQueueDir(const C2dEffect* pEffect, const CMatrix& matrix, CVector& pos)
{
	pos = Multiply3x3(matrix, pEffect->pedattr.queueDir);
}

void CPedAttractorManager::ComputeEffectUseDir(const C2dEffect* pEffect, const CMatrix& matrix, CVector& pos)
{
	pos = Multiply3x3(matrix, pEffect->pedattr.useDir);
}

CPedAttractor* CPedAttractorManager::RegisterPed(CPed* pPed, C2dEffect* pEffect, const CMatrix& matrix, std::vector<CPedAttractor*>& vecAttractors)
{
	CPedAttractor* pRegisteredAttractor = nil;
	for (std::vector<CPedAttractor*>::const_iterator pAttractorIt = vecAttractors.cbegin(); pAttractorIt != vecAttractors.cend(); ++pAttractorIt) {
		CPedAttractor* pAttractor = *pAttractorIt;
		CVector vEffectPos;
		ComputeEffectPos(pAttractor->GetEffect(), matrix, vEffectPos);
		if (pAttractor->GetEffect() == pEffect && vEffectPos == pAttractor->GetEffectPos()) {
			if (!IsApproachable(pEffect, matrix, pAttractor->ComputeFreeSlot(), pPed))
				return nil;
			pRegisteredAttractor = pAttractor;
			break;
		}
	}
	if (pRegisteredAttractor || !IsApproachable(pEffect, matrix, 0, pPed)) {
		if (pRegisteredAttractor)
			pRegisteredAttractor->RegisterPed(pPed);
		return pRegisteredAttractor;
	}
	switch (pEffect->pedattr.type) {
	case ATTRACTOR_ATM: vecAttractors.push_back(new CPedAtmAttractor(pEffect, matrix, gcMaxSizeOfAtmQueue, 1.0f, 30000.0f, 3000.0f, 0.2f, 0.15f, 0.1f, 0.1f)); break;
	case ATTRACTOR_SEAT: vecAttractors.push_back(new CPedSeatAttractor(pEffect, matrix, gcMaxSizeOfSeatQueue, 1.0f, 30000.0f, 3000.0f, 0.125f, 0.1f, 0.1f, 0.1f)); break;
	case ATTRACTOR_STOP: vecAttractors.push_back(new CPedStopAttractor(pEffect, matrix, gcMaxSizeOfStopQueue, 1.0f, 30000.0f, 3000.0f, 0.2f, 0.1f, 0.1f, 0.1f)); break;
	case ATTRACTOR_PIZZA: vecAttractors.push_back(new CPedPizzaAttractor(pEffect, matrix, gcMaxSizeOfPizzaQueue, 1.0f, 30000.0f, 3000.0f, 0.2f, 0.1f, 0.1f, 0.1f)); break;
	case ATTRACTOR_SHELTER: vecAttractors.push_back(new CPedShelterAttractor(pEffect, matrix, gcMaxSizeOfShelterQueue, 1.0f, 30000.0f, 3000.0f, 0.5f, 6.28f, 0.1f, 0.1f)); break;
	case ATTRACTOR_ICECREAM: vecAttractors.push_back(new CPedIceCreamAttractor(pEffect, matrix, gcMaxSizeOfIceCreamQueue, 1.0f, 30000.0f, 3000.0f, 0.2f, 0.3f, 0.1f, 0.1f)); break;
	}
	if (pRegisteredAttractor)
		pRegisteredAttractor->RegisterPed(pPed);
	return pRegisteredAttractor;
}

bool CPedAttractorManager::DeRegisterPed(CPed* pPed, CPedAttractor* pAttractor, std::vector<CPedAttractor*>& vecAttractors)
{
	if (!pAttractor)
		return false;
	CPedAttractor* pFound = nil;
	for (std::vector<CPedAttractor*>::const_iterator pAttractorIt = vecAttractors.cbegin(); pAttractorIt != vecAttractors.cend(); ++pAttractorIt) {
		if (*pAttractorIt == pAttractor) {
			pFound = *pAttractorIt;
			break;
		}
	}
	if (!pFound)
		return false;
	pFound->DeRegisterPed(pPed);
	if (pFound->GetNoOfRegisteredPeds() != 0)
		return true;
	for (std::vector<CPedAttractor*>::const_iterator pAttractorIt = vecAttractors.cbegin(); pAttractorIt != vecAttractors.cend(); ++pAttractorIt) {
		if (*pAttractorIt == pAttractor) {
			vecAttractors.erase(pAttractorIt);
			break;
		}
	}
	delete pAttractor;
	return true;
}

bool CPedAttractorManager::BroadcastArrival(CPed* pPed, CPedAttractor* pAttractor, std::vector<CPedAttractor*>& vecAttractors)
{
	if (!pAttractor)
		return false;
	CPedAttractor* pFound = nil;
	for (std::vector<CPedAttractor*>::const_iterator pAttractorIt = vecAttractors.cbegin(); pAttractorIt != vecAttractors.cend(); ++pAttractorIt) {
		if (*pAttractorIt == pAttractor) {
			pFound = *pAttractorIt;
			break;
		}
	}
	if (!pFound)
		return false;
	pFound->BroadcastArrival(pPed);
	return true;
}

bool CPedAttractorManager::BroadcastDeparture(CPed* pPed, CPedAttractor* pAttractor, std::vector<CPedAttractor*>& vecAttractors)
{
	if (!pAttractor)
		return false;
	CPedAttractor* pFound = nil;
	for (std::vector<CPedAttractor*>::const_iterator pAttractorIt = vecAttractors.cbegin(); pAttractorIt != vecAttractors.cend(); ++pAttractorIt) {
		if (*pAttractorIt == pAttractor) {
			pFound = *pAttractorIt;
			break;
		}
	}
	if (!pFound)
		return false;
	pFound->DeRegisterPed(pPed);
	if (pFound->GetNoOfRegisteredPeds() != 0)
		return true;
	for (std::vector<CPedAttractor*>::const_iterator pAttractorIt = vecAttractors.cbegin(); pAttractorIt != vecAttractors.cend(); ++pAttractorIt) {
		if (*pAttractorIt == pAttractor) {
			vecAttractors.erase(pAttractorIt);
			break;
		}
	}
	delete pAttractor;
	return true;
}

bool CPedAttractorManager::IsInQueue(CPed* pPed, CPedAttractor* pAttractor, std::vector<CPedAttractor*>& vecAttractors)
{
	if (!pAttractor)
		return false;
	for (std::vector<CPedAttractor*>::const_iterator pAttractorIt = vecAttractors.cbegin(); pAttractorIt != vecAttractors.cend(); ++pAttractorIt) {
		if (*pAttractorIt == pAttractor) {
			return (*pAttractorIt)->IsInQueue(pPed);
		}
	}
	return false;
}

bool CPedAttractorManager::IsAtHeadOfQueue(CPed* pPed, CPedAttractor* pAttractor, std::vector<CPedAttractor*>& vecAttractors)
{
	if (!pAttractor)
		return false;
	for (std::vector<CPedAttractor*>::const_iterator pAttractorIt = vecAttractors.cbegin(); pAttractorIt != vecAttractors.cend(); ++pAttractorIt) {
		if (*pAttractorIt == pAttractor) {
			return (*pAttractorIt)->IsAtHeadOfQueue(pPed);
		}
	}
	return false;
}

bool CPedAttractorManager::IsPedRegistered(CPed* pPed, std::vector<CPedAttractor*>& vecAttractors)
{
	for (std::vector<CPedAttractor*>::const_iterator pAttractorIt = vecAttractors.cbegin(); pAttractorIt != vecAttractors.cend(); ++pAttractorIt) {
		if ((*pAttractorIt)->IsRegisteredWithPed(pPed))
			return true;
	}
	return false;
}

bool CPedAttractorManager::IsApproachable(C2dEffect* pEffect, const CMatrix& matrix, int32, CPed* pPed)
{
	if (pEffect->pedattr.type == ATTRACTOR_SHELTER) {
		CVector pos;
		ComputeEffectPos(pEffect, matrix, pos);
		return CWorld::GetIsLineOfSightClear(pPed->GetPosition(), pos, true, false, false, false, false, false);
	}
	CVector vecUseDir, vecEffectPos;
	ComputeEffectUseDir(pEffect, matrix, vecUseDir);
	ComputeEffectPos(pEffect, matrix, vecEffectPos);
	float dp = -DotProduct(vecUseDir, vecEffectPos);
	if (pEffect->pedattr.type == ATTRACTOR_ATM || pEffect->pedattr.type == ATTRACTOR_PIZZA || pEffect->pedattr.type == ATTRACTOR_ICECREAM) {
		vecUseDir = -vecUseDir;
		dp = -dp;
	}
	if (dp + DotProduct(vecEffectPos, pPed->GetPosition()) > 0.0f) {
		CVector vecPedToAttractor = pPed->GetPosition() - vecEffectPos;
		vecPedToAttractor.Normalise();
		if (DotProduct(vecPedToAttractor, vecUseDir) > 0.25f && CWorld::IsWanderPathClear(pPed->GetPosition(), vecEffectPos, 2.0f, 0))
			return true;
	}
	return false;
}
