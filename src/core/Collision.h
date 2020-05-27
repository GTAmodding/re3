#pragma once

#include "templates.h"
#include "Game.h"	// for eLevelName

// If you spawn many tanks at once, you will see that collisions of two entity exceeds 32.
#ifdef FIX_BUGS
#define MAX_COLLISION_POINTS 64
#else
#define MAX_COLLISION_POINTS 32
#endif

struct CSphere
{
	CVector center;
	float radius;
	void Set(float radius, const CVector &center) { this->center = center; this->radius = radius; }
};

struct CBox
{
	CVector min;
	CVector max;
	CVector GetSize(void) { return max - min; }
	void Set(const CVector &min, const CVector &max) {  this->min = min; this->max = max; }
};

struct CColSphere : public CSphere
{
	uint8 surface;
	uint8 piece;

	void Set(float radius, const CVector &center, uint8 surf, uint8 piece);
	using CSphere::Set;
};

struct CColBox : public CBox
{
	uint8 surface;
	uint8 piece;

	void Set(const CVector &min, const CVector &max, uint8 surf, uint8 piece);
	using CBox::Set;
};

struct CColLine
{
	CVector p0;
	int pad0;
	CVector p1;
	int pad1;

	CColLine(void) { };
	CColLine(const CVector &p0, const CVector &p1) { this->p0 = p0; this->p1 = p1; };
	void Set(const CVector &p0, const CVector &p1);
};

struct CColTriangle
{
	uint16 a;
	uint16 b;
	uint16 c;
	uint8 surface;

	void Set(const CVector *v, int a, int b, int c, uint8 surf, uint8 piece);
};

struct CColTrianglePlane
{
	CVector normal;
	float dist;
	uint8 dir;

	void Set(const CVector *v, CColTriangle &tri);
	void GetNormal(CVector &n) const { n = normal; }
	float CalcPoint(const CVector &v) const { return DotProduct(normal, v) - dist; };
};

struct CColPoint
{
	CVector point;
	int pad1;
	// the surface normal on the surface of point
	CVector normal;
	int pad2;
	uint8 surfaceA;
	uint8 pieceA;
	uint8 surfaceB;
	uint8 pieceB;
	float depth;
};

struct CStoredCollPoly
{
	CVector verts[3];
	bool valid;
};

//--MIAMI: done struct
struct CColModel
{
	CSphere boundingSphere;
	CBox boundingBox;
	int16 numSpheres;
	int16 numBoxes;
	int16 numTriangles;
	int8 numLines;
	uint8 level;	// colstore slot but probably still named level
	bool ownsCollisionVolumes;
	CColSphere *spheres;
	CColLine *lines;
	CColBox *boxes;
	CVector *vertices;
	CColTriangle *triangles;
	CColTrianglePlane *trianglePlanes;

	CColModel(void);
	~CColModel(void);
	void RemoveCollisionVolumes(void);
	void CalculateTrianglePlanes(void);
	void RemoveTrianglePlanes(void);
	CLink<CColModel*> *GetLinkPtr(void);
	void SetLinkPtr(CLink<CColModel*>*);
	void GetTrianglePoint(CVector &v, int i) const;

	CColModel& operator=(const CColModel& other);
};

class CCollision
{
public:
	static eLevelName ms_collisionInMemory;
	static CLinkList<CColModel*> ms_colModelCache;

	static void Init(void);
	static void Shutdown(void);
	static void Update(void);
	static void LoadCollisionWhenINeedIt(bool changeLevel);
	static void SortOutCollisionAfterLoad(void);
	static void LoadCollisionScreen(eLevelName level);
	static void DrawColModel(const CMatrix &mat, const CColModel &colModel);
	static void DrawColModel_Coloured(const CMatrix &mat, const CColModel &colModel, int32 id);

	static void CalculateTrianglePlanes(CColModel *model);

	// all these return true if there's a collision
	static bool TestSphereSphere(const CSphere &s1, const CSphere &s2);
	static bool TestSphereBox(const CSphere &sph, const CBox &box);
	static bool TestLineBox(const CColLine &line, const CBox &box);
	static bool TestVerticalLineBox(const CColLine &line, const CBox &box);
	static bool TestLineTriangle(const CColLine &line, const CVector *verts, const CColTriangle &tri, const CColTrianglePlane &plane);
	static bool TestLineSphere(const CColLine &line, const CColSphere &sph);
	static bool TestSphereTriangle(const CColSphere &sphere, const CVector *verts, const CColTriangle &tri, const CColTrianglePlane &plane);
	static bool TestLineOfSight(const CColLine &line, const CMatrix &matrix, CColModel &model, bool ignoreSeeThrough, bool ignoreShootThrough);

	static bool ProcessSphereSphere(const CColSphere &s1, const CColSphere &s2, CColPoint &point, float &mindistsq);
	static bool ProcessSphereBox(const CColSphere &sph, const CColBox &box, CColPoint &point, float &mindistsq);
	static bool ProcessLineBox(const CColLine &line, const CColBox &box, CColPoint &point, float &mindist);
	static bool ProcessVerticalLineTriangle(const CColLine &line, const CVector *verts, const CColTriangle &tri, const CColTrianglePlane &plane, CColPoint &point, float &mindist, CStoredCollPoly *poly);
	static bool ProcessLineTriangle(const CColLine &line , const CVector *verts, const CColTriangle &tri, const CColTrianglePlane &plane, CColPoint &point, float &mindist);
	static bool ProcessLineSphere(const CColLine &line, const CColSphere &sphere, CColPoint &point, float &mindist);
	static bool ProcessSphereTriangle(const CColSphere &sph, const CVector *verts, const CColTriangle &tri, const CColTrianglePlane &plane, CColPoint &point, float &mindistsq);
	static bool ProcessLineOfSight(const CColLine &line, const CMatrix &matrix, CColModel &model, CColPoint &point, float &mindist, bool ignoreSeeThrough, bool ignoreShootThrough);
	static bool ProcessVerticalLine(const CColLine &line, const CMatrix &matrix, CColModel &model, CColPoint &point, float &mindist, bool ignoreSeeThrough, bool ignoreShootThrough, CStoredCollPoly *poly);
	static int32 ProcessColModels(const CMatrix &matrixA, CColModel &modelA, const CMatrix &matrixB, CColModel &modelB, CColPoint *spherepoints, CColPoint *linepoints, float *linedists);
	static bool IsStoredPolyStillValidVerticalLine(const CVector &pos, float z, CColPoint &point, CStoredCollPoly *poly);

	static float DistToLine(const CVector *l0, const CVector *l1, const CVector *point);
	static float DistToLine(const CVector *l0, const CVector *l1, const CVector *point, CVector &closest);
};
