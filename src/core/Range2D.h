#pragma once

class CRange2D
{
	CVector2D min, max;
public:
	CRange2D(CVector2D _min, CVector2D _max);
	bool IsInRange(CVector2D vec);
	void DebugShowRange(float, int);
	CVector2D GetRandomPointInRange();
};