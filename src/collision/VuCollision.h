#pragma once


struct VuTriangle
{
	// Compressed int16 but unpacked
#ifdef GTA_PS2
	uint128 v0;
	uint128 v1;
	uint128 v2;
	uint128 plane;
#else
	int32 v0[4];
	int32 v1[4];
	int32 v2[4];
	int32 plane[4];
#endif
};

#ifndef GTA_PS2
extern int16 vi01;
extern CVuVector vf01;
extern CVuVector vf02;
extern CVuVector vf03;
#endif

extern "C" {
void LineToTriangleCollision(const CVuVector &p0, const CVuVector &p1, const CVuVector &v0, const CVuVector &v1, const CVuVector &v2, const CVuVector &plane);
void LineToTriangleCollisionCompressed(const CVuVector &p0, const CVuVector &p1, VuTriangle &tri);
void SphereToTriangleCollision(const CVuVector &sph, const CVuVector &v0, const CVuVector &v1, const CVuVector &v2, const CVuVector &plane);
void SphereToTriangleCollisionCompressed(const CVuVector &sph, VuTriangle &tri);
}
