#include "common.h"
#include "ColBox.h"

void
CColBox::Set(const CVector &min, const CVector &max, uint8 surf, uint8 piece)
{
	this->min = min;
	this->max = max;
	this->surface = surf;
	this->piece = piece;
}

CColBox&
CColBox::operator=(const CColBox& other)
{
	min = other.min;
	max = other.max;
	surface = other.surface;
	piece = other.piece;
	return *this;
}