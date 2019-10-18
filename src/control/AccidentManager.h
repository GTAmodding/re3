#pragma once
#include "common.h"
#include "config.h"

class CPed;

class CAccident
{
public:
	CPed *m_pVictim;
	uint32 m_nMedicsAttending;
	uint32 m_nMedicsPerformingCPR;
	CAccident() : m_pVictim(nil), m_nMedicsAttending(0), m_nMedicsPerformingCPR(0) {}
};

class CAccidentManager
{
	CAccident m_aAccidents[NUM_ACCIDENTS];
	enum {
		MAX_MEDICS_TO_ATTEND_ACCIDENT = 2
	};
public:
	uint16 CountActiveAccidents();
	CAccident* FindNearestAccident(CVector, float*);
	void Update(void);
};

extern CAccidentManager& gAccidentManager;