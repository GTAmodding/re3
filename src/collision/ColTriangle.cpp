#include "common.h"
#include "ColTriangle.h"

void
CColTriangle::Set(const CompressedVector *, int a, int b, int c, uint8 surf, uint8 piece)
{
	this->a = a;
	this->b = b;
	this->c = c;
	this->surface = surf;
}

#ifdef VU_COLLISION
void
CColTrianglePlane::Set(const CVector &va, const CVector &vb, const CVector &vc)
{
	CVector norm = CrossProduct(vc-va, vb-va);
	norm.Normalise();
	float d = DotProduct(norm, va);
	normal.x = norm.x*4096.0f;
	normal.y = norm.y*4096.0f;
	normal.z = norm.z*4096.0f;
	dist = d*128.0f;
}
#else
void
CColTrianglePlane::Set(const CVector &va, const CVector &vb, const CVector &vc)
{
	normal = CrossProduct(vc-va, vb-va);
	normal.Normalise();
	dist = DotProduct(normal, va);
	CVector an(Abs(normal.x), Abs(normal.y), Abs(normal.z));
	// find out largest component and its direction
	if(an.x > an.y && an.x > an.z)
		dir = normal.x < 0.0f ? DIR_X_NEG : DIR_X_POS;
	else if(an.y > an.z)
		dir = normal.y < 0.0f ? DIR_Y_NEG : DIR_Y_POS;
	else
		dir = normal.z < 0.0f ? DIR_Z_NEG : DIR_Z_POS;
}
#endif