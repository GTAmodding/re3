#include "common.h"
#include "patcher.h"
#include "PlayerPed.h"
#include "Camera.h"
#include "WeaponEffects.h"
#include "ModelIndices.h"
#include "World.h"

CPlayerPed::~CPlayerPed()
{
	delete m_pWanted;
}

WRAPPER void CPlayerPed::ReApplyMoveAnims(void) { EAXJMP(0x4F07C0); }
WRAPPER void CPlayerPed::SetupPlayerPed(int32) { EAXJMP(0x4EFB60); }
WRAPPER void CPlayerPed::DeactivatePlayerPed(int32) { EAXJMP(0x4EFC00); }
WRAPPER void CPlayerPed::ReactivatePlayerPed(int32) { EAXJMP(0x4EFC20); }
WRAPPER void CPlayerPed::KeepAreaAroundPlayerClear(void) { EAXJMP(0x4F3460); }
WRAPPER void CPlayerPed::MakeChangesForNewWeapon(int8) { EAXJMP(0x4F2560); }
WRAPPER void CPlayerPed::SetInitialState(void) { EAXJMP(0x4EFC40); }
WRAPPER void CPlayerPed::SetMoveAnim(void) { EAXJMP(0x4F3760); }
WRAPPER void CPlayerPed::ProcessControl(void) { EAXJMP(0x4EFD90); }

CPlayerPed::CPlayerPed(void) : CPed(PEDTYPE_PLAYER1)
{
	m_fMoveSpeed = 0.0f;
	SetModelIndex(MI_PLAYER);
	SetInitialState();

	m_pWanted = new CWanted();
	m_pWanted->Initialise();
	m_pArrestingCop = nil;
	m_currentWeapon = WEAPONTYPE_UNARMED;
	m_nSelectedWepSlot = 0;
	m_nSpeedTimer = 0;
	m_bSpeedTimerFlag = 0;
	m_pPointGunAt = nil;
	m_nPedState = PED_IDLE;
	m_fMaxStamina = 150.0f;
	m_fCurrentStamina = m_fMaxStamina;
	m_fStaminaProgress = 0.0f;
	m_bShouldEvade = 0;
	field_1367 = 0;
	m_nShotDelay = 0;
	field_1376 = 0.0f;
	field_1380 = 0;
	m_bHasLockOnTarget = false;
	m_bCanBeDamaged = true;
	m_fWalkAngle = 0.0f;
	m_fFPSMoveHeading = 0.0f;
	m_nTargettableObjects[0] = m_nTargettableObjects[1] = m_nTargettableObjects[2] = m_nTargettableObjects[3] = -1;
	field_1413 = 0;
	for (int i = 0; i < 6; i++) {
		m_vecSafePos[i] = CVector(0.0f, 0.0f, 0.0f);
		m_pPedAtSafePos[i] = nil;
	}
}

void CPlayerPed::ClearWeaponTarget()
{
	if (m_nPedType == PEDTYPE_PLAYER1) {
		m_pPointGunAt = nil;
		TheCamera.ClearPlayerWeaponMode();
		CWeaponEffects::ClearCrosshair();
	}
	ClearPointGunAt();
}

void
CPlayerPed::SetWantedLevel(int32 level)
{
	m_pWanted->SetWantedLevel(level);
}

void
CPlayerPed::SetWantedLevelNoDrop(int32 level)
{
	m_pWanted->SetWantedLevelNoDrop(level);
}

// I don't know the actual purpose of parameter
void
CPlayerPed::AnnoyPlayerPed(bool annoyedByPassingEntity)
{
	if (m_pedStats->m_temper < 52) {
		m_pedStats->m_temper++;
	} else {
		if (annoyedByPassingEntity) {
			if (m_pedStats->m_temper < 55) {
				m_pedStats->m_temper++;
			} else {
				m_pedStats->m_temper = 46;
			}
		}
	}
}

void
CPlayerPed::ClearAdrenaline(void)
{
	if (m_bAdrenalineActive && m_nAdrenalineTime != 0) {
		m_nAdrenalineTime = 0;
		CTimer::SetTimeStep(1.0f);
	}
}

CPlayerInfo *
CPlayerPed::GetPlayerInfoForThisPlayerPed()
{
	if (CWorld::Players[0].m_pPed == this)
		return &CWorld::Players[0];

	return nil;
}

class CPlayerPed_ : public CPlayerPed
{
public:
	CPlayerPed* ctor(void) { return ::new (this) CPlayerPed(); }
	void dtor(void) { CPlayerPed::~CPlayerPed(); }
};

STARTPATCHES
	InjectHook(0x4EF7E0, &CPlayerPed_::ctor, PATCH_JUMP);
	InjectHook(0x4EFB30, &CPlayerPed_::dtor, PATCH_JUMP);
	InjectHook(0x4F28A0, &CPlayerPed::ClearWeaponTarget, PATCH_JUMP);
	InjectHook(0x4F3700, &CPlayerPed::AnnoyPlayerPed, PATCH_JUMP);
	InjectHook(0x4F36C0, &CPlayerPed::GetPlayerInfoForThisPlayerPed, PATCH_JUMP);
ENDPATCHES
