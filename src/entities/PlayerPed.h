#pragma once

#include "Ped.h"
#include "Wanted.h"

class CPlayerPed : public CPed {
public:
	CWanted *m_pWanted;
	CCopPed *m_pArrestingCop;
	Float m_fMoveSpeed;
	Float m_fCurrentStamina;
	Float m_fMaxStamina;
	Float m_fStaminaProgress;
	Bool m_bWeaponSlot;
	Bool m_bSpeedTimerFlag;
	Bool m_bShouldEvade;
	int8 field_1367;
	int32 m_nSpeedTimer;
	int32 m_nShotDelay;
	Float field_1376;
	int8 field_1380;
	int8 field_1381;
	int8 field_1382;
	int8 field_1383;
	CEntity *m_pEvadingFrom;
	int32 m_nTargettableObjects[4];
	Bool m_bAdrenalineActive;
	Bool m_bHasLockOnTarget;
	int8 field_1406;
	int8 field_1407;
	Bool m_bAdrenalineTime;
	Bool m_bCanBeDamaged;
	int8 field_1413;
	int8 field_1414;
	int8 field_1415;
	CVector field_1416[6];
	int32 field_1488[6];
	Float field_1512;
	Float m_fFPSMoveHeading;
};

static_assert(sizeof(CPlayerPed) == 0x5F0, "CPlayerPed: error");
