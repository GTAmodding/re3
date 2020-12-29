#pragma once
#include <vector>

#include "2dEffect.h"
#include "Ped.h"

#define NUM_ATTRACTORS_FOR_ICECREAM_VAN 4

class CPedAttractor;

class CVehicleToEffect
{
	CVehicle* m_pVehicle;
	C2dEffect m_effects[NUM_ATTRACTORS_FOR_ICECREAM_VAN];

public:
	CVehicleToEffect(CVehicle* pVehicle);
	const C2dEffect* ChooseEffect(const CVector& pos) const;
	CVehicleToEffect& From(const CVehicleToEffect& other);
	CVehicleToEffect& operator=(const CVehicleToEffect& other) { return From(other); }
	~CVehicleToEffect() { m_pVehicle = nil; }
	CVehicle* GetVehicle() const { return m_pVehicle; }
	bool HasThisEffect(C2dEffect* pEffect) const;
	const C2dEffect* GetEffect(int32 i) const { return &m_effects[i]; }
};

class CPedAttractorManager
{
	std::vector<CPedAttractor*> vAtmAttractors;
	std::vector<CPedAttractor*> vSeatAttractors;
	std::vector<CPedAttractor*> vStopAttractors;
	std::vector<CPedAttractor*> vPizzaAttractors;
	std::vector<CPedAttractor*> vShelterAttractors;
	std::vector<CPedAttractor*> vIceCreamAttractors;
	std::vector<CVehicleToEffect> vVehicleToEffect;

public:
	CPedAttractor* RegisterPedWithAttractor(CPed* pPed, C2dEffect* pEffect, const CMatrix& matrix);
	CPedAttractor* RegisterPed(CPed* pPed, C2dEffect* pEffect, const CMatrix& matrix, std::vector<CPedAttractor*>& vecAttractors);
	bool BroadcastArrival(CPed* pPed, CPedAttractor* pAttractor);
	bool BroadcastArrival(CPed* pPed, CPedAttractor* pAttractor, std::vector<CPedAttractor*>& vecAttractors);
	const C2dEffect* GetEffectForIceCreamVan(CVehicle* pVehicle, const CVector& pos);
	bool IsApproachable(C2dEffect* pEffect, const CMatrix& matrix, int32, CPed* pPed);
	void RemoveIceCreamVanEffects(C2dEffect* pEffect);
	bool HasEmptySlot(const C2dEffect* pEffect);
	const CPedAttractor* FindAssociatedAttractor(const C2dEffect* pEffect, std::vector<CPedAttractor*>& vecAttractors);
	bool IsInQueue(CPed* pPed, CPedAttractor* pAttractor);
	bool IsInQueue(CPed* pPed, CPedAttractor* pAttractor, std::vector<CPedAttractor*>& vecAttractors);
	bool IsAtHeadOfQueue(CPed* pPed, CPedAttractor* pAttractor);
	bool IsAtHeadOfQueue(CPed* pPed, CPedAttractor* pAttractor, std::vector<CPedAttractor*>& vecAttractors);
	bool BroadcastDeparture(CPed* pPed, CPedAttractor* pAttractor);
	bool BroadcastDeparture(CPed* pPed, CPedAttractor* pAttractor, std::vector<CPedAttractor*>& vecAttractors);
	bool DeRegisterPed(CPed* pPed, CPedAttractor* pAttractor);
	bool DeRegisterPed(CPed* pPed, CPedAttractor* pAttractor, std::vector<CPedAttractor*>& vecAttractors);
	bool IsPedRegisteredWithEffect(CPed* pPed);
	bool IsPedRegistered(CPed* pPed, std::vector<CPedAttractor*>& vecAttractors);
	CVehicle* GetIceCreamVanForEffect(C2dEffect* pEffect);

	static void ComputeEffectPos(const C2dEffect* pEffect, const CMatrix& matrix, CVector& pos);
	static void ComputeEffectQueueDir(const C2dEffect* pEffect, const CMatrix& matrix, CVector& pos);
	static void ComputeEffectUseDir(const C2dEffect* pEffect, const CMatrix& matrix, CVector& pos);

};

CPedAttractorManager* GetPedAttractorManager();

enum ePedAttractorType
{
	ATTRACTOR_ATM = 0,
	ATTRACTOR_SEAT,
	ATTRACTOR_STOP,
	ATTRACTOR_PIZZA,
	ATTRACTOR_SHELTER,
	ATTRACTOR_ICECREAM
};

class CPedAttractor
{
protected:
	C2dEffect* p2dEffect;
	std::vector<CPed*> vApproachingQueue;
	std::vector<CPed*> vWaitingQueue;
	int32 m_nMaxPedsInAttractor;
	float m_fQueueDistance;
	float m_fTimeInWaitQueue;
	float m_fTimeInApproachingQueue;
	float m_fDistanceToUseAttractor;
	float m_fAcceptableHeading;
	float m_fMaxPositionDisplacement;
	float m_fMaxHeadingDisplacement;
	CVector vecEffectPos;
	CVector vecQueueDir;
	CVector vecUseDir;

public:
	virtual float GetHeadOfQueueWaitTime() { return 0.0f; }
	virtual ~CPedAttractor() {};
	virtual ePedAttractorType GetType() const = 0;
	virtual void UpdatePedStateOnDeparture(CPed* pPed) const = 0;
	virtual bool IsAtHeadOfQueue(CPed* pPed) const { return vWaitingQueue.front() == pPed; }
	virtual void ComputeAttractPos(int32 id, CVector& pos) const;
	virtual void ComputeAttractHeading(int32 id, float& pHeading) const;
	virtual bool BroadcastDeparture(CPed* pPed);

	bool IsRegisteredWithPed(CPed* pPed) const;
	bool DeRegisterPed(CPed* pPed);
	float ComputeDeltaHeading() const;
	float ComputeDeltaPos() const;
	void ComputeAttractTime(int32 id, bool, float& time) const;
	int32 GetNoOfRegisteredPeds() const { return (int32)(vWaitingQueue.size() + vApproachingQueue.size()); }
	int32 ComputeFreeSlot() const { return (int32)vWaitingQueue.size(); }
	bool IsInQueue(CPed*) const;
	bool RegisterPed(CPed*);
	bool BroadcastArrival(CPed*);

	CPedAttractor(C2dEffect* pEffect, const CMatrix& matrix, int32 maxpeds, float qdist, float waitTime, float approachTime, float distance, float headingdiff, float posdisp, float headdisp);

	C2dEffect* GetEffect() const { return p2dEffect; }
	const CVector& GetEffectPos() const { return vecEffectPos; }
	int32 GetMaxPedsInAttractor() const { return m_nMaxPedsInAttractor; }
	float GetDistanceToCountSeekDone() const { return m_fDistanceToUseAttractor; }
	float GetAcceptableHeading() const { return m_fAcceptableHeading; }
};

class CPedAtmAttractor : public CPedAttractor
{
public:
	virtual ePedAttractorType GetType() const override { return ATTRACTOR_ATM; };
	virtual void UpdatePedStateOnDeparture(CPed* pPed) const override;
	CPedAtmAttractor(C2dEffect* pEffect, const CMatrix& matrix, int32 maxpeds, float qdist, float waitTime, float approachTime, float distance, float headingdiff, float posdisp, float headdisp) :
		CPedAttractor(pEffect, matrix, maxpeds, qdist, waitTime, approachTime, distance, headingdiff, posdisp, headdisp)
	{};
};

class CPedIceCreamAttractor : public CPedAttractor
{
public:
	virtual ~CPedIceCreamAttractor() override { GetPedAttractorManager()->RemoveIceCreamVanEffects(p2dEffect); }
	virtual ePedAttractorType GetType() const override { return ATTRACTOR_ICECREAM; }
	virtual void UpdatePedStateOnDeparture(CPed* pPed) const override {};
	CPedIceCreamAttractor(C2dEffect* pEffect, const CMatrix& matrix, int32 maxpeds, float qdist, float waitTime, float approachTime, float distance, float headingdiff, float posdisp, float headdisp) :
		CPedAttractor(pEffect, matrix, maxpeds, qdist, waitTime, approachTime, distance, headingdiff, posdisp, headdisp)
	{};
};

class CPedPizzaAttractor : public CPedAttractor
{
public:
	virtual float GetHeadOfQueueWaitTime() override { return 2000.0f; }
	virtual ePedAttractorType GetType() const override { return ATTRACTOR_PIZZA; }
	virtual void UpdatePedStateOnDeparture(CPed* pPed) const override;
	CPedPizzaAttractor(C2dEffect* pEffect, const CMatrix& matrix, int32 maxpeds, float qdist, float waitTime, float approachTime, float distance, float headingdiff, float posdisp, float headdisp) :
		CPedAttractor(pEffect, matrix, maxpeds, qdist, waitTime, approachTime, distance, headingdiff, posdisp, headdisp)
	{};
};

class CPedSeatAttractor : public CPedAttractor
{
public:
	virtual ePedAttractorType GetType() const override { return ATTRACTOR_SEAT; }
	virtual void UpdatePedStateOnDeparture(CPed* pPed) const override {};
	CPedSeatAttractor(C2dEffect* pEffect, const CMatrix& matrix, int32 maxpeds, float qdist, float waitTime, float approachTime, float distance, float headingdiff, float posdisp, float headdisp) :
		CPedAttractor(pEffect, matrix, maxpeds, qdist, waitTime, approachTime, distance, headingdiff, posdisp, headdisp)
	{};
};

class CPedShelterAttractor : public CPedAttractor
{
	static std::vector<CVector> ms_displacements;
public:
	virtual ePedAttractorType GetType() const override { return ATTRACTOR_SHELTER; }
	virtual bool BroadcastDeparture(CPed*) override;
	virtual void UpdatePedStateOnDeparture(CPed* pPed) const override {};
	virtual bool IsAtHeadOfQueue(CPed* pPed) const override { return true; }
	virtual void ComputeAttractPos(int qid, CVector& pos) const override;
	virtual void ComputeAttractHeading(int qid, float& heading) const override;

	CPedShelterAttractor(C2dEffect* pEffect, const CMatrix& matrix, int32 maxpeds, float qdist, float waitTime, float approachTime, float distance, float headingdiff, float posdisp, float headdisp) :
		CPedAttractor(pEffect, matrix, maxpeds, qdist, waitTime, approachTime, distance, headingdiff, posdisp, headdisp)
	{};


	CVector GetDisplacement(int32 qid) const;
};

class CPedStopAttractor : public CPedAttractor
{
public:
	virtual ePedAttractorType GetType() const override { return ATTRACTOR_STOP; }
	virtual void UpdatePedStateOnDeparture(CPed* pPed) const override {};

	CPedStopAttractor(C2dEffect* pEffect, const CMatrix& matrix, int32 maxpeds, float qdist, float waitTime, float approachTime, float distance, float headingdiff, float posdisp, float headdisp) :
		CPedAttractor(pEffect, matrix, maxpeds, qdist, waitTime, approachTime, distance, headingdiff, posdisp, headdisp)
	{};
};