#pragma once

#include "Placeable.h"

// unused

class CInstance : public CPlaceable
{
public:
	int m_modelIndex;
public:
	~CInstance() { }
	void Shutdown();
};
