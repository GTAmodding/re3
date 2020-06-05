#pragma once

class CRange3D
{
	CVector min, max;
public:
	CRange3D(CVector _min, CVector _max);
	bool IsInRange(CVector vec);
	void DebugShowRange(float, int);
	CVector GetRandomPointInRange();
};