#pragma once

#include "Entity.h"

class CBuilding : public CEntity
{
public:
	CBuilding(void);
	static void *operator new(size_t);
	static void operator delete(void*, size_t);

	void ReplaceWithNewModel(int32 id);

	virtual bool GetIsATreadable(void) { return false; }
};

bool IsBuildingPointerValid(CBuilding*);
