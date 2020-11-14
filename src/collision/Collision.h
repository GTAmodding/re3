#pragma once

#include "ColModel.h"
#include "Game.h"	// for eLevelName
#ifdef VU_COLLISION
#include "VuVector.h"
#endif

struct CStoredCollPoly
{
#ifdef VU_COLLISION
	CVuVector verts[3];
#else
	CVector verts[3];
#endif
	bool valid;
};

// If you spawn many tanks at once, you will see that collisions of two entity exceeds 32.
#if defined(FIX_BUGS) && !defined(SQUEEZE_PERFORMANCE)
#define MAX_COLLISION_POINTS 64
#else
#define MAX_COLLISION_POINTS 32
#endif

class CCollision
{
public:
	static eLevelName ms_collisionInMemory;
	static CLinkList<CColModel*> ms_colModelCache;
#ifdef NO_ISLAND_LOADING
	static bool bAlreadyLoaded;
#endif

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
	static bool TestSphereSphere(const CColSphere &s1, const CColSphere &s2);
	static bool TestSphereBox(const CColSphere &sph, const CColBox &box);
	static bool TestLineBox(const CColLine &line, const CColBox &box);
	static bool TestVerticalLineBox(const CColLine &line, const CColBox &box);
	static bool TestLineTriangle(const CColLine &line, const CompressedVector *verts, const CColTriangle &tri, const CColTrianglePlane &plane);
	static bool TestLineSphere(const CColLine &line, const CColSphere &sph);
	static bool TestSphereTriangle(const CColSphere &sphere, const CompressedVector *verts, const CColTriangle &tri, const CColTrianglePlane &plane);
	static bool TestLineOfSight(const CColLine &line, const CMatrix &matrix, CColModel &model, bool ignoreSeeThrough);

	static bool ProcessSphereSphere(const CColSphere &s1, const CColSphere &s2, CColPoint &point, float &mindistsq);
	static bool ProcessSphereBox(const CColSphere &sph, const CColBox &box, CColPoint &point, float &mindistsq);
	static bool ProcessLineBox(const CColLine &line, const CColBox &box, CColPoint &point, float &mindist);
	static bool ProcessVerticalLineTriangle(const CColLine &line, const CompressedVector *verts, const CColTriangle &tri, const CColTrianglePlane &plane, CColPoint &point, float &mindist, CStoredCollPoly *poly);
	static bool ProcessLineTriangle(const CColLine &line , const CompressedVector *verts, const CColTriangle &tri, const CColTrianglePlane &plane, CColPoint &point, float &mindist);
	static bool ProcessLineSphere(const CColLine &line, const CColSphere &sphere, CColPoint &point, float &mindist);
	static bool ProcessSphereTriangle(const CColSphere &sph, const CompressedVector *verts, const CColTriangle &tri, const CColTrianglePlane &plane, CColPoint &point, float &mindistsq);
	static bool ProcessLineOfSight(const CColLine &line, const CMatrix &matrix, CColModel &model, CColPoint &point, float &mindist, bool ignoreSeeThrough);
	static bool ProcessVerticalLine(const CColLine &line, const CMatrix &matrix, CColModel &model, CColPoint &point, float &mindist, bool ignoreSeeThrough, CStoredCollPoly *poly);
	static int32 ProcessColModels(const CMatrix &matrixA, CColModel &modelA, const CMatrix &matrixB, CColModel &modelB, CColPoint *spherepoints, CColPoint *linepoints, float *linedists);
	static bool IsStoredPolyStillValidVerticalLine(const CVector &pos, float z, CColPoint &point, CStoredCollPoly *poly);

	static float DistToLine(const CVector *l0, const CVector *l1, const CVector *point);
	static float DistToLine(const CVector *l0, const CVector *l1, const CVector *point, CVector &closest);
};
