#pragma once

#include "Entity.h"

class CBuilding : public CEntity
{
public:
	CBuilding(void) {
		m_type = ENTITY_TYPE_BUILDING;
		bUsesCollision = true;
	}
	static void *operator new(size_t);
	static void operator delete(void*, size_t);

	void ReplaceWithNewModel(int32 id);

	virtual bool GetIsATreadable(void) { return false; }
};

VALIDATE_SIZE(CBuilding, 0x64);

