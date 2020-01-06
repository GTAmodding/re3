#pragma once

enum eWeaponType
{
	WEAPONTYPE_UNARMED,
	WEAPONTYPE_BASEBALLBAT,
	WEAPONTYPE_COLT45,
	WEAPONTYPE_UZI,
	WEAPONTYPE_SHOTGUN,
	WEAPONTYPE_AK47,
	WEAPONTYPE_M16,
	WEAPONTYPE_SNIPERRIFLE,
	WEAPONTYPE_ROCKETLAUNCHER,
	WEAPONTYPE_FLAMETHROWER,
	WEAPONTYPE_MOLOTOV,
	WEAPONTYPE_GRENADE,
	WEAPONTYPE_DETONATOR,
	WEAPONTYPE_TOTAL_INVENTORY_WEAPONS = 13,
	WEAPONTYPE_HELICANNON = 13,
	WEAPONTYPE_TOTALWEAPONS,
	WEAPONTYPE_ARMOUR,
	WEAPONTYPE_RAMMEDBYCAR,
	WEAPONTYPE_RUNOVERBYCAR,
	WEAPONTYPE_EXPLOSION,
	WEAPONTYPE_UZI_DRIVEBY,
	WEAPONTYPE_WATER,
	WEAPONTYPE_FALL_DAMAGE,
	WEAPONTYPE_UNIDENTIFIED,
};

enum eWeaponFire {
	WEAPON_FIRE_MELEE,
	WEAPON_FIRE_INSTANT_HIT,
	WEAPON_FIRE_PROJECTILE,
	WEAPON_FIRE_AREA_EFFECT,
	WEAPON_FIRE_USE
};

// Taken from MTA SA, seems it's unchanged
enum eWeaponState
{
	WEAPONSTATE_READY,
	WEAPONSTATE_FIRING,
	WEAPONSTATE_RELOADING,
	WEAPONSTATE_OUT_OF_AMMO,
	WEAPONSTATE_MELEE_MADECONTACT
};

class CEntity;
class CAutomobile;

class CWeapon
{
public:
	eWeaponType m_eWeaponType;
	eWeaponState m_eWeaponState;
	uint32 m_nAmmoInClip;
	uint32 m_nAmmoTotal;
	uint32 m_nTimer;
	bool m_bAddRotOffset;

	CWeapon() {
		m_bAddRotOffset = false;
	}

	void Initialise(eWeaponType type, int ammo);
	void Update(int32 audioEntity);
	void Reload(void);
	bool Fire(CEntity*, CVector*);
	void FireFromCar(CAutomobile *car, bool left);
	void AddGunshell(CEntity*, CVector const&, CVector2D const&, float);
	bool IsTypeMelee(void);
	bool IsType2Handed(void);
	static void DoTankDoomAiming(CEntity *playerVehicle, CEntity *playerPed, CVector *start, CVector *end);
	bool HitsGround(CEntity* holder, CVector* firePos, CEntity* aimingTo);
	bool HasWeaponAmmoToBeUsed(void);
	static void InitialiseWeapons(void);
	static void UpdateWeapons(void);
};
static_assert(sizeof(CWeapon) == 0x18, "CWeapon: error");
