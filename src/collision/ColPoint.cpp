#include "common.h"
#include "ColPoint.h"

CColPoint&
CColPoint::operator=(const CColPoint &other)
{
	point = other.point;
	normal = other.normal;
	surfaceA = other.surfaceA;
	pieceA = other.pieceA;
	surfaceB = other.surfaceB;
	pieceB = other.pieceB;

	// no depth?
	return *this;
}
