#include "common.h"
#include "patcher.h"
#include "Fire.h"

CFireManager &gFireManager = *(CFireManager*)0x8F31D0;

WRAPPER void CFire::Extinguish(void) { EAXJMP(0x479D40); }

CFire* CFireManager::FindNearestFire(CVector vecPos, float* pDistance)
{
	for (int i = 0; i < MAX_FIREMEN_ATTENDING; i++) {
		int fireId = -1;
		float minDistance = 999999;
		for (int j = 0; j < NUM_FIRES; j++) {
			if (!m_aFires[j].m_bIsOngoing)
				continue;
			if (m_aFires[j].m_bIsScriptFire)
				continue;
			if (m_aFires[j].m_nFiremenPuttingOut != i)
				continue;
			float distance = (m_aFires[j].m_vecPos - vecPos).Magnitude2D();
			if (distance < minDistance) {
				minDistance = distance;
				fireId = j;
			}
		}
		*pDistance = minDistance;
		if (fireId != -1)
			return &m_aFires[fireId];
	}
	return nil;
}

WRAPPER void CFireManager::StartFire(CEntity *entityOnFire, CEntity *culprit, float, uint32) { EAXJMP(0x479590); }
WRAPPER void CFireManager::Update(void) { EAXJMP(0x479310); }
WRAPPER CFire *CFireManager::FindFurthestFire_NeverMindFireMen(CVector coors, float, float) { EAXJMP(0x479430); }
