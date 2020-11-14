#include "common.h"
#ifdef VU_COLLISION
#include "VuVector.h"
#include "VuCollision.h"

#ifndef GTA_PS2
int16 vi01;
CVuVector vf01;
CVuVector vf02;
CVuVector vf03;

CVuVector
DistanceBetweenSphereAndLine(const CVuVector &center, const CVuVector &p0, const CVuVector &line)
{
	// center  VF12
	// p0      VF14
	// line    VF15
	CVuVector ret;	// VF16
	CVuVector p1 = p0+line;
	CVuVector dist0 = center - p0;	// VF20
	CVuVector dist1 = center - p1;	// VF25
	float lenSq = line.MagnitudeSqr();	// VF21
	float distSq0 = dist0.MagnitudeSqr();	// VF22
	float distSq1 = dist1.MagnitudeSqr();
	float dot = DotProduct(dist0, line);	// VF23
	if(dot < 0.0f){
		// not above line, closest to p0
		ret = p0;
		ret.w = distSq0;
		return ret;
	}
	float t = dot/lenSq;	// param of nearest point on infinite line
	if(t > 1.0f){
		// not above line, closest to p1
		ret = p1;
		ret.w = distSq1;
		return ret;
	}
	// closest to line
	ret = p0 + line*t;
	ret.w = (ret - center).MagnitudeSqr();
	return ret;
}
inline int SignFlags(const CVector &v)
{
	int f = 0;
	if(v.x < 0.0f) f |= 1;
	if(v.y < 0.0f) f |= 2;
	if(v.z < 0.0f) f |= 4;
	return f;
}
#endif

extern "C" void
LineToTriangleCollision(const CVuVector &p0, const CVuVector &p1,
	const CVuVector &v0, const CVuVector &v1, const CVuVector &v2,
	const CVuVector &plane)
{
#ifdef GTA_PS2
	__asm__ volatile (
		".set noreorder\n"
		"lqc2	vf12, 0x0(%0)\n"
		"lqc2	vf13, 0x0(%1)\n"
		"lqc2	vf14, 0x0(%2)\n"
		"lqc2	vf15, 0x0(%3)\n"
		"lqc2	vf16, 0x0(%4)\n"
		"lqc2	vf17, 0x0(%5)\n"
		"vcallms	Vu0LineToTriangleCollisionStart\n"
		".set reorder\n"
		:
		: "r" (&p0), "r" (&p1), "r" (&v0), "r" (&v1), "r" (&v2), "r" (&plane)
	);
#else
	float dot0 = DotProduct(plane, p0);
	float dot1 = DotProduct(plane, p1);
	float dist0 = plane.w - dot0;
	float dist1 = plane.w - dot1;

	// if points are on the same side, no collision
	if(dist0 * dist1 > 0.0f){
		vi01 = 0;
		return;
	}

	CVuVector diff = p1 - p0;
	float t = dist0/(dot1 - dot0);
	CVuVector p = p0 + diff*t;
	p.w = 0.0f;
	vf01 = p;
	vf03.x = t;

	// Check if point is inside
	CVector cross1 = CrossProduct(p-v0, v1-v0);
	CVector cross2 = CrossProduct(p-v1, v2-v1);
	CVector cross3 = CrossProduct(p-v2, v0-v2);
	// Only check relevant directions
	int flagmask = 0;
	if(Abs(plane.x) > 0.5f) flagmask |= 1;
	if(Abs(plane.y) > 0.5f) flagmask |= 2;
	if(Abs(plane.z) > 0.5f) flagmask |= 4;
	int flags1 = SignFlags(cross1) & flagmask;
	int flags2 = SignFlags(cross2) & flagmask;
	int flags3 = SignFlags(cross3) & flagmask;
	// inside if on the same side of all edges
	if(flags1 != flags2 || flags1 != flags3){
		vi01 = 0;
		return;
	}
	vi01 = 1;
	vf02 = plane;
	return;
#endif
}

extern "C" void
LineToTriangleCollisionCompressed(const CVuVector &p0, const CVuVector &p1, VuTriangle &tri)
{
#ifdef GTA_PS2
	__asm__ volatile (
		".set noreorder\n"
		"lqc2	vf12, 0x0(%0)\n"
		"lqc2	vf13, 0x0(%1)\n"
		"lqc2	vf14, 0x0(%2)\n"
		"lqc2	vf15, 0x10(%2)\n"
		"lqc2	vf16, 0x20(%2)\n"
		"lqc2	vf17, 0x30(%2)\n"
		"vcallms	Vu0LineToTriangleCollisionCompressedStart\n"
		".set reorder\n"
		:
		: "r" (&p0), "r" (&p1), "r" (&tri)
	);
#else
	CVuVector v0, v1, v2, plane;
	v0.x = tri.v0[0]/128.0f;
	v0.y = tri.v0[1]/128.0f;
	v0.z = tri.v0[2]/128.0f;
	v0.w = tri.v0[3]/128.0f;
	v1.x = tri.v1[0]/128.0f;
	v1.y = tri.v1[1]/128.0f;
	v1.z = tri.v1[2]/128.0f;
	v1.w = tri.v1[3]/128.0f;
	v2.x = tri.v2[0]/128.0f;
	v2.y = tri.v2[1]/128.0f;
	v2.z = tri.v2[2]/128.0f;
	v2.w = tri.v2[3]/128.0f;
	plane.x = tri.plane[0]/4096.0f;
	plane.y = tri.plane[1]/4096.0f;
	plane.z = tri.plane[2]/4096.0f;
	plane.w = tri.plane[3]/128.0f;
	LineToTriangleCollision(p0, p1, v0, v1, v2, plane);
#endif
}

extern "C" void
SphereToTriangleCollision(const CVuVector &sph,
	const CVuVector &v0, const CVuVector &v1, const CVuVector &v2,
	const CVuVector &plane)
{
#ifdef GTA_PS2
	__asm__ volatile (
		".set noreorder\n"
		"lqc2	vf12, 0x0(%0)\n"
		"lqc2	vf14, 0x0(%1)\n"
		"lqc2	vf15, 0x0(%2)\n"
		"lqc2	vf16, 0x0(%3)\n"
		"lqc2	vf17, 0x0(%4)\n"
		"vcallms	Vu0SphereToTriangleCollisionStart\n"
		".set reorder\n"
		:
		: "r" (&sph), "r" (&v0), "r" (&v1), "r" (&v2), "r" (&plane)
	);
#else
	float planedist = DotProduct(plane, sph) - plane.w;	// VF02
	if(Abs(planedist) > sph.w){
		vi01 = 0;
		return;
	}
	// point on plane
	CVuVector p = sph - planedist*plane;
	p.w = 0.0f;
	vf01 = p;
	planedist = Abs(planedist);
	// edges
	CVuVector v01 = v1 - v0;
	CVuVector v12 = v2 - v1;
	CVuVector v20 = v0 - v2;
	// VU code calculates normal again for some weird reason...
	// Check sides of point
	CVector cross1 = CrossProduct(p-v0, v01);
	CVector cross2 = CrossProduct(p-v1, v12);
	CVector cross3 = CrossProduct(p-v2, v20);
	// Only check relevant directions
	int flagmask = 0;
	if(Abs(plane.x) > 0.1f) flagmask |= 1;
	if(Abs(plane.y) > 0.1f) flagmask |= 2;
	if(Abs(plane.z) > 0.1f) flagmask |= 4;
	int nflags = SignFlags(plane) & flagmask;
	int flags1 = SignFlags(cross1) & flagmask;
	int flags2 = SignFlags(cross2) & flagmask;
	int flags3 = SignFlags(cross3) & flagmask;
	int testcase = 0;
	CVuVector closest(0.0f, 0.0f, 0.0f);	// VF04
	if(flags1 == nflags){
		closest += v2;
		testcase++;
	}
	if(flags2 == nflags){
		closest += v0;
		testcase++;
	}
	if(flags3 == nflags){
		closest += v1;
		testcase++;
	}
	if(testcase == 3){
		// inside triangle - dist to plane already checked
		vf02 = plane;
		vf02.w = vf03.x = planedist;
		vi01 = 1;
	}else if(testcase == 1){
		// outside two sides - closest to point opposide inside edge
		vf01 = closest;
		vf02 = sph - closest;
		float distSq = vf02.MagnitudeSqr();
		vi01 = sph.w*sph.w > distSq;
		vf03.x = Sqrt(distSq);
		vf02 *= 1.0f/vf03.x;
	}else{
		// inside two sides - closest to third edge
		if(flags1 != nflags)
			closest = DistanceBetweenSphereAndLine(sph, v0, v01);
		else if(flags2 != nflags)
			closest = DistanceBetweenSphereAndLine(sph, v1, v12);
		else
			closest = DistanceBetweenSphereAndLine(sph, v2, v20);
		vi01 = sph.w*sph.w > closest.w;
		vf01 = closest;
		vf02 = sph - closest;
		vf03.x = Sqrt(closest.w);
		vf02 *= 1.0f/vf03.x;
	}
#endif
}

extern "C" void
SphereToTriangleCollisionCompressed(const CVuVector &sph, VuTriangle &tri)
{
#ifdef GTA_PS2
	__asm__ volatile (
		".set noreorder\n"
		"lqc2	vf12, 0x0(%0)\n"
		"lqc2	vf14, 0x0(%1)\n"
		"lqc2	vf15, 0x10(%1)\n"
		"lqc2	vf16, 0x20(%1)\n"
		"lqc2	vf17, 0x30(%1)\n"
		"vcallms	Vu0SphereToTriangleCollisionCompressedStart\n"
		".set reorder\n"
		:
		: "r" (&sph), "r" (&tri)
	);
#else
	CVuVector v0, v1, v2, plane;
	v0.x = tri.v0[0]/128.0f;
	v0.y = tri.v0[1]/128.0f;
	v0.z = tri.v0[2]/128.0f;
	v0.w = tri.v0[3]/128.0f;
	v1.x = tri.v1[0]/128.0f;
	v1.y = tri.v1[1]/128.0f;
	v1.z = tri.v1[2]/128.0f;
	v1.w = tri.v1[3]/128.0f;
	v2.x = tri.v2[0]/128.0f;
	v2.y = tri.v2[1]/128.0f;
	v2.z = tri.v2[2]/128.0f;
	v2.w = tri.v2[3]/128.0f;
	plane.x = tri.plane[0]/4096.0f;
	plane.y = tri.plane[1]/4096.0f;
	plane.z = tri.plane[2]/4096.0f;
	plane.w = tri.plane[3]/128.0f;
	SphereToTriangleCollision(sph, v0, v1, v2, plane);
#endif
}
#endif