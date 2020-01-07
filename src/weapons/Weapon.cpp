#include "common.h"
#include "patcher.h"
#include "Weapon.h"
#include "Timer.h"
#include "WeaponInfo.h"
#include "Ped.h"
#include "World.h"

WRAPPER void CWeapon::UpdateWeapons(void) { EAXJMP(0x55C310); }
WRAPPER bool CWeapon::Fire(CEntity*, CVector*) { EAXJMP(0x55C380); }
WRAPPER void CWeapon::FireFromCar(CAutomobile *car, bool left) { EAXJMP(0x55C940); }
WRAPPER void CWeapon::AddGunshell(CEntity*, CVector const&, CVector2D const&, float) { EAXJMP(0x55F770); }
WRAPPER void CWeapon::Update(int32 audioEntity) { EAXJMP(0x563A10); }
WRAPPER void CWeapon::DoTankDoomAiming(CEntity *playerVehicle, CEntity *playerPed, CVector *start, CVector *end) { EAXJMP(0x563200); }
WRAPPER void CWeapon::InitialiseWeapons(void) { EAXJMP(0x55C2D0); }

void
CWeapon::Initialise(eWeaponType type, int ammo)
{
	m_eWeaponType = type;
	m_eWeaponState = WEAPONSTATE_READY;
	if (ammo > 99999)
		m_nAmmoTotal = 99999;
	else
		m_nAmmoTotal = ammo;
	m_nAmmoInClip = 0;
	Reload();
	m_nTimer = 0;
}

void
CWeapon::Reload(void)
{
	if (m_nAmmoTotal == 0)
		return;

	CWeaponInfo *info = CWeaponInfo::GetWeaponInfo(m_eWeaponType);

	if (m_nAmmoTotal >= info->m_nAmountofAmmunition)
		m_nAmmoInClip = info->m_nAmountofAmmunition;
	else
		m_nAmmoInClip = m_nAmmoTotal;
}

bool
CWeapon::IsType2Handed(void)
{
	return m_eWeaponType >= WEAPONTYPE_SHOTGUN && m_eWeaponType <= WEAPONTYPE_FLAMETHROWER && m_eWeaponType != WEAPONTYPE_ROCKETLAUNCHER;
}

bool
CWeapon::IsTypeMelee(void)
{
	return m_eWeaponType == WEAPONTYPE_UNARMED || m_eWeaponType == WEAPONTYPE_BASEBALLBAT;
}

bool
CWeapon::HitsGround(CEntity *holder, CVector *firePos, CEntity *aimingTo)
{
	if (!holder->IsPed() || !((CPed*)holder)->m_pSeekTarget)
		return false;

	CWeaponInfo *ourType = CWeaponInfo::GetWeaponInfo(m_eWeaponType);
	CVector adjustedOffset = ourType->m_vecFireOffset;
	adjustedOffset.z += 0.6f;

	CVector point1, point2;
	CEntity *foundEnt = nil;
	CColPoint foundCol;

	if (firePos)
		point1 = *firePos;
	else
		point1 = holder->GetMatrix() * adjustedOffset;

	CEntity *aimEntity = aimingTo ? aimingTo : ((CPed*)holder)->m_pSeekTarget;
	point2 = aimEntity->GetPosition();
	point2.z += 0.6f;

	CWorld::ProcessLineOfSight(point1, point2, foundCol, foundEnt, true, false, false, false, false, false, false);
	if (foundEnt && foundEnt->IsBuilding()) {
		// That was supposed to be Magnitude, according to leftover code in assembly
		float diff = (foundCol.point.z - point1.z);
		if (diff < 0.0f && diff > -3.0f)
			return true;
	}

	return false;
}

bool
CWeapon::HasWeaponAmmoToBeUsed(void)
{
	switch (m_eWeaponType) {
		case WEAPONTYPE_UNARMED:
		case WEAPONTYPE_BASEBALLBAT:
			return true;
		default:
			return m_nAmmoTotal != 0;
	}
}

STARTPATCHES
	InjectHook(0x55C330, &CWeapon::Initialise, PATCH_JUMP);
	InjectHook(0x5639D0, &CWeapon::Reload, PATCH_JUMP);
	InjectHook(0x564890, &CWeapon::HitsGround, PATCH_JUMP);
ENDPATCHES
