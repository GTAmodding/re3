#pragma once

class CObject;

class CProjectileInfo
{
public:
	static bool RemoveIfThisIsAProjectile(CObject *pObject);
	static void RemoveAllProjectiles(void);
	static bool IsProjectileInRange(float x1, float x2, float y1, float y2, float z1, float z2, bool remove);
};