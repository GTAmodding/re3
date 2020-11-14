#pragma once

#include "SurfaceTable.h"

struct CColBox
{
	CVector min;
	CVector max;
	uint8 surface;
	uint8 piece;

	void Set(const CVector &min, const CVector &max, uint8 surf = SURFACE_DEFAULT, uint8 piece = 0);
	CVector GetSize(void) { return max - min; }

	CColBox& operator=(const CColBox &other);
};