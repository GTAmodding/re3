#include "common.h"
#include "patcher.h"
#include "AccidentManager.h"

#include "Ped.h"

CAccidentManager& gAccidentManager = *(CAccidentManager*)0x87FD10;

WRAPPER void CAccidentManager::Update(void) { EAXJMP(0x456710); }

uint16 CAccidentManager::CountActiveAccidents()
{
	uint16 accidents = 0;
	for (int i = 0; i < NUM_ACCIDENTS; i++){
		if (m_aAccidents[i].m_pVictim)
			accidents++;
	}
	return accidents;
}

CAccident* CAccidentManager::FindNearestAccident(CVector vecPos, float* pDistance)
{
	for (int i = 0; i < MAX_MEDICS_TO_ATTEND_ACCIDENT; i++){
		int accidentId = -1;
		float minDistance = 999999;
		for (int j = 0; j < NUM_ACCIDENTS; j++){
			CPed* pVictim = m_aAccidents[j].m_pVictim;
			if (!pVictim)
				continue;
			if (pVictim->CharCreatedBy == MISSION_CHAR)
				continue;
			if (pVictim->m_fHealth != 0.0f)
				continue;
			if (m_aAccidents[j].m_nMedicsPerformingCPR != i)
				continue;
			float distance = (pVictim->GetPosition() - vecPos).Magnitude2D();
			if (distance / 2 > pVictim->GetPosition().z - vecPos.z && distance < minDistance){
				minDistance = distance;
				accidentId = j;
			}
		}
		*pDistance = minDistance;
		if (accidentId != -1)
			return &m_aAccidents[accidentId];
	}
	return nil;
}