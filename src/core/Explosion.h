#pragma once

class CEntity;
class CVector;

enum eExplosionType
{
	EXPLOSION_GRENADE,
	EXPLOSION_MOLOTOV,
	EXPLOSION_ROCKET,
	EXPLOSION_CAR,
	EXPLOSION_CAR_QUICK,
	EXPLOSION_HELI,
	EXPLOSION_MINE,
	EXPLOSION_BARREL,
	EXPLOSION_TANK_GRENADE,
	EXPLOSION_HELI_BOMB
};

class CExplosion
{
public:
	static void AddExplosion(CEntity *explodingEntity, CEntity *culprit, eExplosionType type,
	                         const CVector &pos, uint32);

	static int8 GetExplosionActiveCounter(uint8 id);
	static CVector *GetExplosionPosition(uint8 id);
	static uint8 GetExplosionType(uint8 id);
	static void ResetExplosionActiveCounter(uint8 id);
	static void RemoveAllExplosionsInArea(CVector, float);
	static bool TestForExplosionInArea(eExplosionType, float, float, float, float, float, float);
};
