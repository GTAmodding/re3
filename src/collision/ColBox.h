#pragma once

#include "SurfaceTable.h"

struct CBox
{
	CVector min;
	CVector max;
	CVector GetSize(void) { return max - min; }
	void Set(const CVector &min, const CVector &max) {  this->min = min; this->max = max; }
};

struct CColBox : public CBox
{
	uint8 surface;
	uint8 piece;

	void Set(const CVector &min, const CVector &max, uint8 surf, uint8 piece);
	using CBox::Set;

	CColBox& operator=(const CColBox &other);
};