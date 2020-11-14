#pragma once

struct CompressedVector
{
#ifdef COMPRESSED_COL_VECTORS
	int16 x, y, z;
	CVector Get(void) const { return CVector(x, y, z)/128.0f; };
	void Set(float x, float y, float z) { this->x = x*128.0f; this->y = y*128.0f; this->z = z*128.0f; };
#ifdef GTA_PS2
	void Unpack(uint128 &qword) const {
		__asm__ volatile (
			"lh      $8, 0(%1)\n"
			"lh      $9, 2(%1)\n"
			"lh      $10, 4(%1)\n"
			"pextlw  $10, $8\n"
			"pextlw  $2, $9, $10\n"
			"sq      $2, %0\n"
			: "=m" (qword)
			: "r" (this)
			: "$8", "$9", "$10", "$2"
		);
	}
#else
	void Unpack(int32 *qword) const {
		qword[0] = x;
		qword[1] = y;
		qword[2] = z;
		qword[3] = 0;	// junk
	}
#endif
#else
	float x, y, z;
	CVector Get(void) const { return CVector(x, y, z); };
	void Set(float x, float y, float z) { this->x = x; this->y = y; this->z = z; };
#endif
};