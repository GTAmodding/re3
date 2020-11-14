#include "common.h"
#include "ColSphere.h"

void
CColSphere::Set(float radius, const CVector &center, uint8 surf, uint8 piece)
{
	this->radius = radius;
	this->center = center;
	this->surface = surf;
	this->piece = piece;
}