#pragma once

#include "SurfaceTable.h"

struct CColSphere
{
	// NB: this has to be compatible with a CVuVector
	CVector center;
	float radius;
	uint8 surface;
	uint8 piece;
	void Set(float radius, const CVector &center, uint8 surf = SURFACE_DEFAULT, uint8 piece = 0);
};