#pragma once

#include "CompressedVector.h"

enum Direction {
	DIR_X_POS,
	DIR_X_NEG,
	DIR_Y_POS,
	DIR_Y_NEG,
	DIR_Z_POS,
	DIR_Z_NEG,
};

struct CColTriangle
{
	uint16 a;
	uint16 b;
	uint16 c;
	uint8 surface;

	void Set(const CompressedVector *v, int a, int b, int c, uint8 surf, uint8 piece);
};

struct CColTrianglePlane
{
#ifdef VU_COLLISION
	CompressedVector normal;
	int16 dist;

	void Set(const CVector &va, const CVector &vb, const CVector &vc);
	void Set(const CompressedVector *v, CColTriangle &tri) { Set(v[tri.a].Get(), v[tri.b].Get(), v[tri.c].Get()); }
	void GetNormal(CVector &n) const { n.x = normal.x/4096.0f; n.y = normal.y/4096.0f; n.z = normal.z/4096.0f; }
	float CalcPoint(const CVector &v) const { CVector n; GetNormal(n); return DotProduct(n, v) - dist/128.0f; };
#ifdef GTA_PS2
	void Unpack(uint128 &qword) const {
		__asm__ volatile (
			"lh      $8, 0(%1)\n"
			"lh      $9, 2(%1)\n"
			"lh      $10, 4(%1)\n"
			"lh      $11, 6(%1)\n"
			"pextlw  $10, $8\n"
			"pextlw  $11, $9\n"
			"pextlw  $2, $11, $10\n"
			"sq      $2, %0\n"
			: "=m" (qword)
			: "r" (this)
			: "$8", "$9", "$10", "$11", "$2"
		);
	}
#else
	void Unpack(int32 *qword) const {
		qword[0] = normal.x;
		qword[1] = normal.y;
		qword[2] = normal.z;
		qword[3] = dist;
	}
#endif
#else
	CVector normal;
	float dist;
	uint8 dir;

	void Set(const CVector &va, const CVector &vb, const CVector &vc);
	void Set(const CompressedVector *v, CColTriangle &tri) { Set(v[tri.a].Get(), v[tri.b].Get(), v[tri.c].Get()); }
	void GetNormal(CVector &n) const { n = normal; }
	float CalcPoint(const CVector &v) const { return DotProduct(normal, v) - dist; };
#endif
};