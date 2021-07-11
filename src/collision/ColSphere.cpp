#include "common.h"
#include "ColSphere.h"
#include "General.h"

void
CColSphere::Set(float radius, const CVector &center, uint8 surf, uint8 piece)
{
	this->radius = radius;
	this->center = center;
	this->surface = surf;
	this->piece = piece;
}

bool
CColSphere::IntersectRay(CVector const& from, CVector const& dir, CVector &entry, CVector &exit)
{
	CVector distToCenter = from - center;
	float distToTouchSqr = distToCenter.MagnitudeSqr() - sq(radius);
	float root1, root2;

	if (!CGeneral::SolveQuadratic(1.0f, DotProduct(distToCenter, dir) * 2.f, distToTouchSqr, root1, root2))
		return false;

	entry = from + dir * root1;
	exit = from + dir * root2;
	return true;
}