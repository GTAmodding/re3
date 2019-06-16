#pragma once

#include "Ped.h"
#include "Wanted.h"

class CPlayerPed : public CPed
{
public:
	CWanted *m_pWanted;
	CCopPed *m_pArrestingCop;
	float m_fMoveSpeed;
	float m_fCurrentStamina;
	float m_fMaxStamina;
	float m_fStaminaProgress;
	bool m_bWeaponSlot;
	bool m_bSpeedTimerFlag;
	bool m_bShouldEvade;
	int8 field_1367;
	int32 m_nSpeedTimer;
	int32 m_nShotDelay;
	float field_1376;
	int8 field_1380;
	int8 field_1381;
	int8 field_1382;
	int8 field_1383;
	CEntity *m_pEvadingFrom;
	int32 m_nTargettableObjects[4];
	bool m_bAdrenalineActive;
	bool m_bHasLockOnTarget;
	int8 field_1406;
	int8 field_1407;
	bool m_bAdrenalineTime;
	bool m_bCanBeDamaged;
	int8 field_1413;
	int8 field_1414;
	int8 field_1415;
	CVector field_1416[6];
	int32 field_1488[6];
	float field_1512;
	float m_fFPSMoveHeading;
};

static_assert(sizeof(CPlayerPed) == 0x5F0, "CPlayerPed: error");
