#pragma once

class CWindModifiers
{
public:
	static void RegisterOne(CVector pos, int32 unk);
	static int32 FindWindModifier(CVector pos, float *x, float *y);
};
