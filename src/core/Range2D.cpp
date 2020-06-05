#include "common.h"
#include "Range2D.h"
#include "General.h"

CRange2D::CRange2D(CVector2D _min, CVector2D _max) : min(_min), max(_max) {}

bool
CRange2D::IsInRange(CVector2D vec)
{
	return min.x < vec.x && max.x > vec.x && min.y < vec.y && max.y > vec.y;
}

void
CRange2D::DebugShowRange(float, int)
{
}

CVector2D
CRange2D::GetRandomPointInRange()
{
	int distX = Abs(max.x - min.x);
	int distY = Abs(max.y - min.y);

	float outX = CGeneral::GetRandomNumber() % distX + min.x;
	float outY = CGeneral::GetRandomNumber() % distY + min.y;

	return CVector2D(outX, outY);
}
