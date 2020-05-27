#include "common.h"

#include "Curves.h"

//--MIAMI: file done

float CCurves::CalcSpeedScaleFactor(CVector* pPoint1, CVector* pPoint2, float dir1X, float dir1Y, float dir2X, float dir2Y)
{
	CVector2D dir1(dir1X, dir1Y);
	CVector2D dir2(dir2X, dir2Y);
	float distance = (*pPoint1 - *pPoint2).Magnitude2D();
	float dp = DotProduct2D(dir1, dir2);
	if (dp > 0.9f)
		return distance + Abs((pPoint1->x * dir1Y - pPoint1->y * dir1X) - (pPoint2->x * dir1Y - pPoint2->y * dir1X));
	else
		return ((1.0f - dp) * 0.25f + 1.0f) * distance;
}

void CCurves::CalcCurvePoint(CVector* pPos1, CVector* pPos2, CVector* pDir1, CVector* pDir2, float between, int32 timeOnCurve, CVector* pOutPos, CVector* pOutDir)
{
	float actualFactor = CalcSpeedScaleFactor(pPos1, pPos2, pDir1->x, pDir1->y, pDir2->x, pDir2->y);
	CVector2D dir1 = *pDir1 * actualFactor;
	CVector2D dir2 = *pDir2 * actualFactor;
	float t1 = Abs(DotProduct2D(*pPos1 - *pPos2, *pDir1));
	float t2 = Abs(DotProduct2D(*pPos2 - *pPos1, *pDir2));
	float curveCoef;
	if (t1 > t2) {
		if (between < (t1 - t2) / (t1 + t2))
			curveCoef = 0.0f;
		else
			curveCoef = 0.5f - 0.5f * Cos(3.1415f * (t1 + t2) / (2 * t2) * (between - (t1 - t2) / (t1 + t2)));
	}
	else {
		if (2 * t1 / (t1 + t2) < between)
			curveCoef = 1.0f;
		else
			curveCoef = 0.5f - 0.5f * Cos(3.1415f * between * (t1 + t2) / (2 * t1));
	}
	*pOutPos = CVector(
		(pPos1->x + between * dir1.x) * (1.0f - curveCoef) + (pPos2->x - (1 - between) * dir2.x) * curveCoef,
		(pPos1->y + between * dir1.y) * (1.0f - curveCoef) + (pPos2->y - (1 - between) * dir2.y) * curveCoef,
		0.0f);
	*pOutDir = CVector(
		(dir1.x * (1.0f - curveCoef) + dir2.x * curveCoef) / (timeOnCurve * 0.001f),
		(dir1.y * (1.0f - curveCoef) + dir2.y * curveCoef) / (timeOnCurve * 0.001f),
		0.0f);
}