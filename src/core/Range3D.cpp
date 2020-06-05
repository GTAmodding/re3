#include "common.h"
#include "Range3D.h"
#include "General.h"

CRange3D::CRange3D(CVector _min, CVector _max) : min(_min), max(_max) {}

bool
CRange3D::IsInRange(CVector vec)
{
	return min.x < vec.x && max.x > vec.x && min.y < vec.y && max.y > vec.y && min.z < vec.z && max.z > vec.z;
}

void
CRange3D::DebugShowRange(float, int)
{
}

CVector
CRange3D::GetRandomPointInRange()
{
	int distX = Abs(max.x - min.x);
	int distY = Abs(max.y - min.y);
	int distZ = Abs(max.z - min.z);

	float outX = CGeneral::GetRandomNumber() % distX + min.x;
	float outY = CGeneral::GetRandomNumber() % distY + min.y;
	float outZ = CGeneral::GetRandomNumber() % distZ + min.z;

	return CVector(outX, outY, outZ);
}
