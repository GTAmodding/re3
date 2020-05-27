#include "common.h"

#include "main.h"
#include "Lists.h"
#include "Game.h"
#include "Zones.h"
#include "General.h"
#include "ZoneCull.h"
#include "World.h"
#include "Entity.h"
#include "Train.h"
#include "Streaming.h"
#include "Pad.h"
#include "DMAudio.h"
#include "Population.h"
#include "FileLoader.h"
#include "Replay.h"
#include "CutsceneMgr.h"
#include "RenderBuffer.h"
#include "SurfaceTable.h"
#include "Lines.h"
#include "Collision.h"
#include "Camera.h"
#include "ColStore.h"

enum Direction
{
	DIR_X_POS,
	DIR_X_NEG,
	DIR_Y_POS,
	DIR_Y_NEG,
	DIR_Z_POS,
	DIR_Z_NEG,
};

eLevelName CCollision::ms_collisionInMemory;
CLinkList<CColModel*> CCollision::ms_colModelCache;

//--MIAMI: done
void
CCollision::Init(void)
{
	ms_colModelCache.Init(NUMCOLCACHELINKS);
	ms_collisionInMemory = LEVEL_NONE;
	CColStore::Initialise();
}

//--MIAMI: done
void
CCollision::Shutdown(void)
{
	ms_colModelCache.Shutdown();
	CColStore::Shutdown();
}

//--MIAMI: done
void
CCollision::Update(void)
{
}

//--MIAMI: unused
eLevelName
GetCollisionInSectorList(CPtrList &list)
{
	CPtrNode *node;
	CEntity *e;
	int level;

	for(node = list.first; node; node = node->next){
		e = (CEntity*)node->item;
		level = CModelInfo::GetModelInfo(e->GetModelIndex())->GetColModel()->level;
		if(level != LEVEL_NONE)
			return (eLevelName)level;
	}
	return LEVEL_NONE;
}

//--MIAMI: unused
// Get a level this sector is in based on collision models
eLevelName
GetCollisionInSector(CSector &sect)
{
	int level;

	level = GetCollisionInSectorList(sect.m_lists[ENTITYLIST_BUILDINGS]);
	if(level == LEVEL_NONE)
		level = GetCollisionInSectorList(sect.m_lists[ENTITYLIST_BUILDINGS_OVERLAP]);
	if(level == LEVEL_NONE)
		level = GetCollisionInSectorList(sect.m_lists[ENTITYLIST_OBJECTS]);
	if(level == LEVEL_NONE)
		level = GetCollisionInSectorList(sect.m_lists[ENTITYLIST_OBJECTS_OVERLAP]);
	if(level == LEVEL_NONE)
		level = GetCollisionInSectorList(sect.m_lists[ENTITYLIST_DUMMIES]);
	if(level == LEVEL_NONE)
		level = GetCollisionInSectorList(sect.m_lists[ENTITYLIST_DUMMIES_OVERLAP]);
	return (eLevelName)level;
}

//--MIAMI: done
void
CCollision::LoadCollisionWhenINeedIt(bool forceChange)
{
}

//--MIAMI: done
void
CCollision::SortOutCollisionAfterLoad(void)
{
}

void
CCollision::LoadCollisionScreen(eLevelName level)
{
	static Const char *levelNames[4] = {
		"",
		"IND_ZON",
		"COM_ZON",
		"SUB_ZON"
	};

	// Why twice?
	LoadingIslandScreen(levelNames[level]);
	LoadingIslandScreen(levelNames[level]);
}

//
// Test
//


bool
CCollision::TestSphereSphere(const CSphere &s1, const CSphere &s2)
{
	float d = s1.radius + s2.radius;
	return (s1.center - s2.center).MagnitudeSqr() < d*d;
}

bool
CCollision::TestSphereBox(const CSphere &sph, const CBox &box)
{
	if(sph.center.x + sph.radius < box.min.x) return false;
	if(sph.center.x - sph.radius > box.max.x) return false;
	if(sph.center.y + sph.radius < box.min.y) return false;
	if(sph.center.y - sph.radius > box.max.y) return false;
	if(sph.center.z + sph.radius < box.min.z) return false;
	if(sph.center.z - sph.radius > box.max.z) return false;
	return true;
}

bool
CCollision::TestLineBox(const CColLine &line, const CBox &box)
{
	float t, x, y, z;
	// If either line point is in the box, we have a collision
	if(line.p0.x > box.min.x && line.p0.x < box.max.x &&
	   line.p0.y > box.min.y && line.p0.y < box.max.y &&
	   line.p0.z > box.min.z && line.p0.z < box.max.z)
		return true;
	if(line.p1.x > box.min.x && line.p1.x < box.max.x &&
	   line.p1.y > box.min.y && line.p1.y < box.max.y &&
	   line.p1.z > box.min.z && line.p1.z < box.max.z)
		return true;

	// check if points are on opposite sides of min x plane
	if((box.min.x - line.p1.x) * (box.min.x - line.p0.x) < 0.0f){
		// parameter along line where we intersect
		t = (box.min.x - line.p0.x) / (line.p1.x - line.p0.x);
		// y of intersection
		y = line.p0.y + (line.p1.y - line.p0.y)*t;
		if(y > box.min.y && y < box.max.y){
			// z of intersection
			z = line.p0.z + (line.p1.z - line.p0.z)*t;
			if(z > box.min.z && z < box.max.z)
				return true;
		}
	}

	// same test with max x plane
	if((line.p1.x - box.max.x) * (line.p0.x - box.max.x) < 0.0f){
		t = (line.p0.x - box.max.x) / (line.p0.x - line.p1.x);
		y = line.p0.y + (line.p1.y - line.p0.y)*t;
		if(y > box.min.y && y < box.max.y){
			z = line.p0.z + (line.p1.z - line.p0.z)*t;
			if(z > box.min.z && z < box.max.z)
				return true;
		}
	}

	// min y plne
	if((box.min.y - line.p0.y) * (box.min.y - line.p1.y) < 0.0f){
		t = (box.min.y - line.p0.y) / (line.p1.y - line.p0.y);
		x = line.p0.x + (line.p1.x - line.p0.x)*t;
		if(x > box.min.x && x < box.max.x){
			z = line.p0.z + (line.p1.z - line.p0.z)*t;
			if(z > box.min.z && z < box.max.z)
				return true;
		}
	}

	// max y plane
	if((line.p0.y - box.max.y) * (line.p1.y - box.max.y) < 0.0f){
		t = (line.p0.y - box.max.y) / (line.p0.y - line.p1.y);
		x = line.p0.x + (line.p1.x - line.p0.x)*t;
		if(x > box.min.x && x < box.max.x){
			z = line.p0.z + (line.p1.z - line.p0.z)*t;
			if(z > box.min.z && z < box.max.z)
				return true;
		}
	}

	// min z plne
	if((box.min.z - line.p0.z) * (box.min.z - line.p1.z) < 0.0f){
		t = (box.min.z - line.p0.z) / (line.p1.z - line.p0.z);
		x = line.p0.x + (line.p1.x - line.p0.x)*t;
		if(x > box.min.x && x < box.max.x){
			y = line.p0.y + (line.p1.y - line.p0.y)*t;
			if(y > box.min.y && y < box.max.y)
				return true;
		}
	}

	// max z plane
	if((line.p0.z - box.max.z) * (line.p1.z - box.max.z) < 0.0f){
		t = (line.p0.z - box.max.z) / (line.p0.z - line.p1.z);
		x = line.p0.x + (line.p1.x - line.p0.x)*t;
		if(x > box.min.x && x < box.max.x){
			y = line.p0.y + (line.p1.y - line.p0.y)*t;
			if(y > box.min.y && y < box.max.y)
				return true;
		}
	}
	return false;
}

bool
CCollision::TestVerticalLineBox(const CColLine &line, const CBox &box)
{
	if(line.p0.x <= box.min.x) return false;
	if(line.p0.y <= box.min.y) return false;
	if(line.p0.x >= box.max.x) return false;
	if(line.p0.y >= box.max.y) return false;
	if(line.p0.z < line.p1.z){
		if(line.p0.z > box.max.z) return false;
		if(line.p1.z < box.min.z) return false;
	}else{
		if(line.p1.z > box.max.z) return false;
		if(line.p0.z < box.min.z) return false;
	}
	return true;
}

bool
CCollision::TestLineTriangle(const CColLine &line, const CVector *verts, const CColTriangle &tri, const CColTrianglePlane &plane)
{
	float t;
	CVector normal;
	plane.GetNormal(normal);

	// if points are on the same side, no collision
	if(plane.CalcPoint(line.p0) * plane.CalcPoint(line.p1) > 0.0f)
		return false;

	// intersection parameter on line
	t = -plane.CalcPoint(line.p0) / DotProduct(line.p1 - line.p0, normal);
	// find point of intersection
	CVector p = line.p0 + (line.p1-line.p0)*t;

	const CVector &va = verts[tri.a];
	const CVector &vb = verts[tri.b];
	const CVector &vc = verts[tri.c];
	CVector2D vec1, vec2, vec3, vect;

	// We do the test in 2D. With the plane direction we
	// can figure out how to project the vectors.
	// normal = (c-a) x (b-a)
	switch(plane.dir){
	case DIR_X_POS:
		vec1.x = va.y; vec1.y = va.z;
		vec2.x = vc.y; vec2.y = vc.z;
		vec3.x = vb.y; vec3.y = vb.z;
		vect.x = p.y; vect.y = p.z;
		break;
	case DIR_X_NEG:
		vec1.x = va.y; vec1.y = va.z;
		vec2.x = vb.y; vec2.y = vb.z;
		vec3.x = vc.y; vec3.y = vc.z;
		vect.x = p.y; vect.y = p.z;
		break;
	case DIR_Y_POS:
		vec1.x = va.z; vec1.y = va.x;
		vec2.x = vc.z; vec2.y = vc.x;
		vec3.x = vb.z; vec3.y = vb.x;
		vect.x = p.z; vect.y = p.x;
		break;
	case DIR_Y_NEG:
		vec1.x = va.z; vec1.y = va.x;
		vec2.x = vb.z; vec2.y = vb.x;
		vec3.x = vc.z; vec3.y = vc.x;
		vect.x = p.z; vect.y = p.x;
		break;
	case DIR_Z_POS:
		vec1.x = va.x; vec1.y = va.y;
		vec2.x = vc.x; vec2.y = vc.y;
		vec3.x = vb.x; vec3.y = vb.y;
		vect.x = p.x; vect.y = p.y;
		break;
	case DIR_Z_NEG:
		vec1.x = va.x; vec1.y = va.y;
		vec2.x = vb.x; vec2.y = vb.y;
		vec3.x = vc.x; vec3.y = vc.y;
		vect.x = p.x; vect.y = p.y;
		break;
	default:
		assert(0);
	}
	// This is our triangle:
	// 3-------2
	//  \  P  /
	//   \   /
	//    \ /
	//     1
	// We can use the "2d cross product" to check on which side
	// a vector is of another. Test is true if point is inside of all edges.
	if(CrossProduct2D(vec2-vec1, vect-vec1) < 0.0f) return false;
	if(CrossProduct2D(vec3-vec1, vect-vec1) > 0.0f) return false;
	if(CrossProduct2D(vec3-vec2, vect-vec2) < 0.0f) return false;
	return true;
}

// Test if line segment intersects with sphere.
// If the first point is inside the sphere this test does not register a collision!
// The code is reversed from the original code and rather ugly, see Process for a clear version.
// TODO: actually rewrite this mess
bool
CCollision::TestLineSphere(const CColLine &line, const CColSphere &sph)
{
	CVector v01 = line.p1 - line.p0;	// vector from p0 to p1
	CVector v0c = sph.center - line.p0;	// vector from p0 to center
	float linesq = v01.MagnitudeSqr();
	// I leave in the strange -2 factors even though they serve no real purpose
	float projline = -2.0f * DotProduct(v01, v0c);	// project v0c onto line
	// Square of tangent from p0 multiplied by line length so we can compare with projline.
	// The length of the tangent would be this: Sqrt((c-p0)^2 - r^2).
	// Negative if p0 is inside the sphere! This breaks the test!
	float tansq = 4.0f * linesq *
		(sph.center.MagnitudeSqr() - 2.0f*DotProduct(sph.center, line.p0) + line.p0.MagnitudeSqr() - sph.radius*sph.radius);
	float diffsq = projline*projline - tansq;
	// if diffsq < 0 that means the line is a passant, so no intersection
	if(diffsq < 0.0f)
		return false;
	// projline (negative in GTA for some reason) is the point on the line
	// in the middle of the two intersection points (startin from p0).
	// Sqrt(diffsq) somehow works out to be the distance from that
	// midpoint to the intersection points.
	// So subtract that and get rid of the awkward scaling:
	float f = (-projline - Sqrt(diffsq)) / (2.0f*linesq);
	// f should now be in range [0, 1] for [p0, p1]
	return f >= 0.0f && f <= 1.0f;
}

bool
CCollision::TestSphereTriangle(const CColSphere &sphere,
	const CVector *verts, const CColTriangle &tri, const CColTrianglePlane &plane)
{
	// If sphere and plane don't intersect, no collision
	if(Abs(plane.CalcPoint(sphere.center)) > sphere.radius)
		return false;

	const CVector &va = verts[tri.a];
	const CVector &vb = verts[tri.b];
	const CVector &vc = verts[tri.c];

	// calculate two orthogonal basis vectors for the triangle
	CVector vec2 = vb - va;
	float len = vec2.Magnitude();
	vec2 = vec2 * (1.0f/len);
	CVector vec1 = CrossProduct(vec2, plane.normal);

	// We know A has local coordinate [0,0] and B has [0,len].
	// Now calculate coordinates on triangle for these two vectors:
	CVector vac = vc - va;
	CVector vas = sphere.center - va;
	CVector2D b(0.0f, len);
	CVector2D c(DotProduct(vec1, vac), DotProduct(vec2, vac));
	CVector2D s(DotProduct(vec1, vas), DotProduct(vec2, vas));

	// The three triangle lines partition the space into 6 sectors,
	// find out in which the center lies.
	int insideAB = CrossProduct2D(s, b) >= 0.0f;
	int insideAC = CrossProduct2D(c, s) >= 0.0f;
	int insideBC = CrossProduct2D(s-b, c-b) >= 0.0f;

	int testcase = insideAB + insideAC + insideBC;
	float dist = 0.0f;
	if(testcase == 1){
		// closest to a vertex
		if(insideAB) dist = (sphere.center - vc).Magnitude();
		else if(insideAC) dist = (sphere.center - vb).Magnitude();
		else if(insideBC) dist = (sphere.center - va).Magnitude();
		else assert(0);
	}else if(testcase == 2){
		// closest to an edge
		if(!insideAB) dist = DistToLine(&va, &vb, &sphere.center);
		else if(!insideAC) dist = DistToLine(&va, &vc, &sphere.center);
		else if(!insideBC) dist = DistToLine(&vb, &vc, &sphere.center);
		else assert(0);
	}else if(testcase == 3){
		// center is in triangle
		return true;
	}else
		assert(0);	// front fell off

	return dist < sphere.radius;
}

//--MIAMI: TODO
bool
CCollision::TestLineOfSight(const CColLine &line, const CMatrix &matrix, CColModel &model, bool ignoreSeeThrough, bool ignoreShootThrough)
{
	static CMatrix matTransform;
	int i;

	// transform line to model space
	Invert(matrix, matTransform);
	CColLine newline(matTransform * line.p0, matTransform * line.p1);

	// If we don't intersect with the bounding box, no chance on the rest
	if(!TestLineBox(newline, model.boundingBox))
		return false;

	for(i = 0; i < model.numSpheres; i++)
		if(!ignoreSeeThrough || model.spheres[i].surface != SURFACE_GLASS && model.spheres[i].surface != SURFACE_TRANSPARENT_CLOTH)
			if(TestLineSphere(newline, model.spheres[i]))
				return true;

	for(i = 0; i < model.numBoxes; i++)
		if(!ignoreSeeThrough || model.boxes[i].surface != SURFACE_GLASS && model.boxes[i].surface != SURFACE_TRANSPARENT_CLOTH)
			if(TestLineBox(newline, model.boxes[i]))
				return true;

	CalculateTrianglePlanes(&model);
	for(i = 0; i < model.numTriangles; i++)
		if(!ignoreSeeThrough || model.triangles[i].surface != SURFACE_GLASS && model.triangles[i].surface != SURFACE_TRANSPARENT_CLOTH)
			if(TestLineTriangle(newline, model.vertices, model.triangles[i], model.trianglePlanes[i]))
				return true;

	return false;
}


//
// Process
//

// For Spheres mindist is the squared distance to its center
// For Lines mindist is between [0,1]

bool
CCollision::ProcessSphereSphere(const CColSphere &s1, const CColSphere &s2, CColPoint &point, float &mindistsq)
{
	CVector dist = s1.center - s2.center;
	float d = dist.Magnitude() - s2.radius;	// distance from s1's center to s2
	float dc = d < 0.0f ? 0.0f : d;		// clamp to zero, i.e. if s1's center is inside s2
	// no collision if sphere is not close enough
	if(mindistsq <= dc*dc || s1.radius <= dc)
		return false;
	dist.Normalise();
	point.point = s1.center - dist*dc;
	point.normal = dist;
	point.surfaceA = s1.surface;
	point.pieceA = s1.piece;
	point.surfaceB = s2.surface;
	point.pieceB = s2.piece;
	point.depth = s1.radius - d;	// sphere overlap
	mindistsq = dc*dc;		// collision radius
	return true;
}

bool
CCollision::ProcessSphereBox(const CColSphere &sph, const CColBox &box, CColPoint &point, float &mindistsq)
{
	CVector p;
	CVector dist;

	// GTA's code is too complicated, uses a huge 3x3x3 if statement
	// we can simplify the structure a lot

	// first make sure we have a collision at all
	if(sph.center.x + sph.radius < box.min.x) return false;
	if(sph.center.x - sph.radius > box.max.x) return false;
	if(sph.center.y + sph.radius < box.min.y) return false;
	if(sph.center.y - sph.radius > box.max.y) return false;
	if(sph.center.z + sph.radius < box.min.z) return false;
	if(sph.center.z - sph.radius > box.max.z) return false;

	// Now find out where the sphere center lies in relation to all the sides
	int xpos = sph.center.x < box.min.x ? 1 :
	           sph.center.x > box.max.x ? 2 :
	           0;
	int ypos = sph.center.y < box.min.y ? 1 :
	           sph.center.y > box.max.y ? 2 :
	           0;
	int zpos = sph.center.z < box.min.z ? 1 :
	           sph.center.z > box.max.z ? 2 :
	           0;

	if(xpos == 0 && ypos == 0 && zpos == 0){
		// sphere is inside the box
		p = (box.min + box.max)*0.5f;

		dist = sph.center - p;
		float lensq = dist.MagnitudeSqr();
		if(lensq < mindistsq){
			point.normal = dist * (1.0f/Sqrt(lensq));
			point.point = sph.center - point.normal;
			point.surfaceA = sph.surface;
			point.pieceA = sph.piece;
			point.surfaceB = box.surface;
			point.pieceB = box.piece;

			// find absolute distance to the closer side in each dimension
			float dx = dist.x > 0.0f ?
				box.max.x - sph.center.x :
				sph.center.x - box.min.x;
			float dy = dist.y > 0.0f ?
				box.max.y - sph.center.y :
				sph.center.y - box.min.y;
			float dz = dist.z > 0.0f ?
				box.max.z - sph.center.z :
				sph.center.z - box.min.z;
			// collision depth is maximum of that:
			if(dx > dy && dx > dz)
				point.depth = dx;
			else if(dy > dz)
				point.depth = dy;
			else
				point.depth = dz;
			return true;
		}
	}else{
		// sphere is outside.
		// closest point on box:
		p.x = xpos == 1 ? box.min.x :
		      xpos == 2 ? box.max.x :
		      sph.center.x;
		p.y = ypos == 1 ? box.min.y :
		      ypos == 2 ? box.max.y :
		      sph.center.y;
		p.z = zpos == 1 ? box.min.z :
		      zpos == 2 ? box.max.z :
		      sph.center.z;

		dist = sph.center - p;
		float lensq = dist.MagnitudeSqr();
		if(lensq < mindistsq){
			float len = Sqrt(lensq);
			point.point = p;
			point.normal = dist * (1.0f/len);
			point.surfaceA = sph.surface;
			point.pieceA = sph.piece;
			point.surfaceB = box.surface;
			point.pieceB = box.piece;
			point.depth = sph.radius - len;
			mindistsq = lensq;
			return true;
		}
	}
	return false;
}

bool
CCollision::ProcessLineBox(const CColLine &line, const CColBox &box, CColPoint &point, float &mindist)
{
	float mint, t, x, y, z;
	CVector normal;
	CVector p;

	mint = 1.0f;
	// check if points are on opposite sides of min x plane
	if((box.min.x - line.p1.x) * (box.min.x - line.p0.x) < 0.0f){
		// parameter along line where we intersect
		t = (box.min.x - line.p0.x) / (line.p1.x - line.p0.x);
		// y of intersection
		y = line.p0.y + (line.p1.y - line.p0.y)*t;
		if(y > box.min.y && y < box.max.y){
			// z of intersection
			z = line.p0.z + (line.p1.z - line.p0.z)*t;
			if(z > box.min.z && z < box.max.z)
				if(t < mint){
					mint = t;
					p = CVector(box.min.x, y, z);
					normal = CVector(-1.0f, 0.0f, 0.0f);
				}
		}
	}

	// max x plane
	if((line.p1.x - box.max.x) * (line.p0.x - box.max.x) < 0.0f){
		t = (line.p0.x - box.max.x) / (line.p0.x - line.p1.x);
		y = line.p0.y + (line.p1.y - line.p0.y)*t;
		if(y > box.min.y && y < box.max.y){
			z = line.p0.z + (line.p1.z - line.p0.z)*t;
			if(z > box.min.z && z < box.max.z)
				if(t < mint){
					mint = t;
					p = CVector(box.max.x, y, z);
					normal = CVector(1.0f, 0.0f, 0.0f);
				}
		}
	}

	// min y plne
	if((box.min.y - line.p0.y) * (box.min.y - line.p1.y) < 0.0f){
		t = (box.min.y - line.p0.y) / (line.p1.y - line.p0.y);
		x = line.p0.x + (line.p1.x - line.p0.x)*t;
		if(x > box.min.x && x < box.max.x){
			z = line.p0.z + (line.p1.z - line.p0.z)*t;
			if(z > box.min.z && z < box.max.z)
				if(t < mint){
					mint = t;
					p = CVector(x, box.min.y, z);
					normal = CVector(0.0f, -1.0f, 0.0f);
				}
		}
	}

	// max y plane
	if((line.p0.y - box.max.y) * (line.p1.y - box.max.y) < 0.0f){
		t = (line.p0.y - box.max.y) / (line.p0.y - line.p1.y);
		x = line.p0.x + (line.p1.x - line.p0.x)*t;
		if(x > box.min.x && x < box.max.x){
			z = line.p0.z + (line.p1.z - line.p0.z)*t;
			if(z > box.min.z && z < box.max.z)
				if(t < mint){
					mint = t;
					p = CVector(x, box.max.y, z);
					normal = CVector(0.0f, 1.0f, 0.0f);
				}
		}
	}

	// min z plne
	if((box.min.z - line.p0.z) * (box.min.z - line.p1.z) < 0.0f){
		t = (box.min.z - line.p0.z) / (line.p1.z - line.p0.z);
		x = line.p0.x + (line.p1.x - line.p0.x)*t;
		if(x > box.min.x && x < box.max.x){
			y = line.p0.y + (line.p1.y - line.p0.y)*t;
			if(y > box.min.y && y < box.max.y)
				if(t < mint){
					mint = t;
					p = CVector(x, y, box.min.z);
					normal = CVector(0.0f, 0.0f, -1.0f);
				}
		}
	}

	// max z plane
	if((line.p0.z - box.max.z) * (line.p1.z - box.max.z) < 0.0f){
		t = (line.p0.z - box.max.z) / (line.p0.z - line.p1.z);
		x = line.p0.x + (line.p1.x - line.p0.x)*t;
		if(x > box.min.x && x < box.max.x){
			y = line.p0.y + (line.p1.y - line.p0.y)*t;
			if(y > box.min.y && y < box.max.y)
				if(t < mint){
					mint = t;
					p = CVector(x, y, box.max.z);
					normal = CVector(0.0f, 0.0f, 1.0f);
				}
		}
	}

	if(mint >= mindist)
		return false;

	point.point = p;
	point.normal = normal;
	point.surfaceA = 0;
	point.pieceA = 0;
	point.surfaceB = box.surface;
	point.pieceB = box.piece;
	mindist = mint;

	return true;
}

// If line.p0 lies inside sphere, no collision is registered.
bool
CCollision::ProcessLineSphere(const CColLine &line, const CColSphere &sphere, CColPoint &point, float &mindist)
{
	CVector v01 = line.p1 - line.p0;
	CVector v0c = sphere.center - line.p0;
	float linesq = v01.MagnitudeSqr();
	// project v0c onto v01, scaled by |v01| this is the midpoint of the two intersections
	float projline = DotProduct(v01, v0c);
	// tangent of p0 to sphere, scaled by linesq just like projline^2
	float tansq = (v0c.MagnitudeSqr() - sphere.radius*sphere.radius) * linesq;
	// this works out to be the square of the distance between the midpoint and the intersections
	float diffsq = projline*projline - tansq;
	// no intersection
	if(diffsq < 0.0f)
		return false;
	// point of first intersection, in range [0,1] between p0 and p1
	float t = (projline - Sqrt(diffsq)) / linesq;
	// if not on line or beyond mindist, no intersection
	if(t < 0.0f || t > 1.0f || t >= mindist)
		return false;
	point.point = line.p0 + v01*t;
	point.normal = point.point - sphere.center;
	point.normal.Normalise();
	point.surfaceA = 0;
	point.pieceA = 0;
	point.surfaceB = sphere.surface;
	point.pieceB = sphere.piece;
	mindist = t;
	return true;
}

bool
CCollision::ProcessVerticalLineTriangle(const CColLine &line,
	const CVector *verts, const CColTriangle &tri, const CColTrianglePlane &plane,
	CColPoint &point, float &mindist, CStoredCollPoly *poly)
{
	float t;
	CVector normal;

	const CVector &p0 = line.p0;
	const CVector &va = verts[tri.a];
	const CVector &vb = verts[tri.b];
	const CVector &vc = verts[tri.c];

	// early out bound rect test
	if(p0.x < va.x && p0.x < vb.x && p0.x < vc.x) return false;
	if(p0.x > va.x && p0.x > vb.x && p0.x > vc.x) return false;
	if(p0.y < va.y && p0.y < vb.y && p0.y < vc.y) return false;
	if(p0.y > va.y && p0.y > vb.y && p0.y > vc.y) return false;

	plane.GetNormal(normal);
	// if points are on the same side, no collision
	if(plane.CalcPoint(p0) * plane.CalcPoint(line.p1) > 0.0f)
		return false;

	// intersection parameter on line
	float h = (line.p1 - p0).z;
	t = -plane.CalcPoint(p0) / (h * normal.z);
	// early out if we're beyond the mindist
	if(t >= mindist)
		return false;
	CVector p(p0.x, p0.y, p0.z + h*t);

	CVector2D vec1, vec2, vec3, vect;
	switch(plane.dir){
	case DIR_X_POS:
		vec1.x = va.y; vec1.y = va.z;
		vec2.x = vc.y; vec2.y = vc.z;
		vec3.x = vb.y; vec3.y = vb.z;
		vect.x = p.y; vect.y = p.z;
		break;
	case DIR_X_NEG:
		vec1.x = va.y; vec1.y = va.z;
		vec2.x = vb.y; vec2.y = vb.z;
		vec3.x = vc.y; vec3.y = vc.z;
		vect.x = p.y; vect.y = p.z;
		break;
	case DIR_Y_POS:
		vec1.x = va.z; vec1.y = va.x;
		vec2.x = vc.z; vec2.y = vc.x;
		vec3.x = vb.z; vec3.y = vb.x;
		vect.x = p.z; vect.y = p.x;
		break;
	case DIR_Y_NEG:
		vec1.x = va.z; vec1.y = va.x;
		vec2.x = vb.z; vec2.y = vb.x;
		vec3.x = vc.z; vec3.y = vc.x;
		vect.x = p.z; vect.y = p.x;
		break;
	case DIR_Z_POS:
		vec1.x = va.x; vec1.y = va.y;
		vec2.x = vc.x; vec2.y = vc.y;
		vec3.x = vb.x; vec3.y = vb.y;
		vect.x = p.x; vect.y = p.y;
		break;
	case DIR_Z_NEG:
		vec1.x = va.x; vec1.y = va.y;
		vec2.x = vb.x; vec2.y = vb.y;
		vec3.x = vc.x; vec3.y = vc.y;
		vect.x = p.x; vect.y = p.y;
		break;
	default:
		assert(0);
	}
	if(CrossProduct2D(vec2-vec1, vect-vec1) < 0.0f) return false;
	if(CrossProduct2D(vec3-vec1, vect-vec1) > 0.0f) return false;
	if(CrossProduct2D(vec3-vec2, vect-vec2) < 0.0f) return false;
	point.point = p;
	point.normal = normal;
	point.surfaceA = 0;
	point.pieceA = 0;
	point.surfaceB = tri.surface;
	point.pieceB = 0;
	if(poly){
		poly->verts[0] = va;
		poly->verts[1] = vb;
		poly->verts[2] = vc;
		poly->valid = true;
	}
	mindist = t;
	return true;
}

bool
CCollision::IsStoredPolyStillValidVerticalLine(const CVector &pos, float z, CColPoint &point, CStoredCollPoly *poly)
{
	float t;

	if(!poly->valid)
		return false;

	// maybe inlined?
	CColTriangle tri;
	tri.a = 0;
	tri.b = 1;
	tri.c = 2;
	CColTrianglePlane plane;
	plane.Set(poly->verts, tri);

	const CVector &va = poly->verts[tri.a];
	const CVector &vb = poly->verts[tri.b];
	const CVector &vc = poly->verts[tri.c];
	CVector p0 = pos;
	CVector p1(pos.x, pos.y, z);

	// The rest is pretty much CCollision::ProcessLineTriangle

	// if points are on the same side, no collision
	if(plane.CalcPoint(p0) * plane.CalcPoint(p1) > 0.0f)
		return poly->valid = false;

	// intersection parameter on line
	t = -plane.CalcPoint(p0) / DotProduct(p1 - p0, plane.normal);
	// find point of intersection
	CVector p = p0 + (p1-p0)*t;

	CVector2D vec1, vec2, vec3, vect;
	switch(plane.dir){
	case DIR_X_POS:
		vec1.x = va.y; vec1.y = va.z;
		vec2.x = vc.y; vec2.y = vc.z;
		vec3.x = vb.y; vec3.y = vb.z;
		vect.x = p.y; vect.y = p.z;
		break;
	case DIR_X_NEG:
		vec1.x = va.y; vec1.y = va.z;
		vec2.x = vb.y; vec2.y = vb.z;
		vec3.x = vc.y; vec3.y = vc.z;
		vect.x = p.y; vect.y = p.z;
		break;
	case DIR_Y_POS:
		vec1.x = va.z; vec1.y = va.x;
		vec2.x = vc.z; vec2.y = vc.x;
		vec3.x = vb.z; vec3.y = vb.x;
		vect.x = p.z; vect.y = p.x;
		break;
	case DIR_Y_NEG:
		vec1.x = va.z; vec1.y = va.x;
		vec2.x = vb.z; vec2.y = vb.x;
		vec3.x = vc.z; vec3.y = vc.x;
		vect.x = p.z; vect.y = p.x;
		break;
	case DIR_Z_POS:
		vec1.x = va.x; vec1.y = va.y;
		vec2.x = vc.x; vec2.y = vc.y;
		vec3.x = vb.x; vec3.y = vb.y;
		vect.x = p.x; vect.y = p.y;
		break;
	case DIR_Z_NEG:
		vec1.x = va.x; vec1.y = va.y;
		vec2.x = vb.x; vec2.y = vb.y;
		vec3.x = vc.x; vec3.y = vc.y;
		vect.x = p.x; vect.y = p.y;
		break;
	default:
		assert(0);
	}
	if(CrossProduct2D(vec2-vec1, vect-vec1) < 0.0f) return poly->valid = false;
	if(CrossProduct2D(vec3-vec1, vect-vec1) > 0.0f) return poly->valid = false;
	if(CrossProduct2D(vec3-vec2, vect-vec2) < 0.0f) return poly->valid = false;
	point.point = p;
	return poly->valid = true;
}

bool
CCollision::ProcessLineTriangle(const CColLine &line ,
	const CVector *verts, const CColTriangle &tri, const CColTrianglePlane &plane,
	CColPoint &point, float &mindist)
{
	float t;
	CVector normal;
	plane.GetNormal(normal);

	// if points are on the same side, no collision
	if(plane.CalcPoint(line.p0) * plane.CalcPoint(line.p1) > 0.0f)
		return false;

	// intersection parameter on line
	t = -plane.CalcPoint(line.p0) / DotProduct(line.p1 - line.p0, normal);
	// early out if we're beyond the mindist
	if(t >= mindist)
		return false;
	// find point of intersection
	CVector p = line.p0 + (line.p1-line.p0)*t;

	const CVector &va = verts[tri.a];
	const CVector &vb = verts[tri.b];
	const CVector &vc = verts[tri.c];
	CVector2D vec1, vec2, vec3, vect;

	switch(plane.dir){
	case DIR_X_POS:
		vec1.x = va.y; vec1.y = va.z;
		vec2.x = vc.y; vec2.y = vc.z;
		vec3.x = vb.y; vec3.y = vb.z;
		vect.x = p.y; vect.y = p.z;
		break;
	case DIR_X_NEG:
		vec1.x = va.y; vec1.y = va.z;
		vec2.x = vb.y; vec2.y = vb.z;
		vec3.x = vc.y; vec3.y = vc.z;
		vect.x = p.y; vect.y = p.z;
		break;
	case DIR_Y_POS:
		vec1.x = va.z; vec1.y = va.x;
		vec2.x = vc.z; vec2.y = vc.x;
		vec3.x = vb.z; vec3.y = vb.x;
		vect.x = p.z; vect.y = p.x;
		break;
	case DIR_Y_NEG:
		vec1.x = va.z; vec1.y = va.x;
		vec2.x = vb.z; vec2.y = vb.x;
		vec3.x = vc.z; vec3.y = vc.x;
		vect.x = p.z; vect.y = p.x;
		break;
	case DIR_Z_POS:
		vec1.x = va.x; vec1.y = va.y;
		vec2.x = vc.x; vec2.y = vc.y;
		vec3.x = vb.x; vec3.y = vb.y;
		vect.x = p.x; vect.y = p.y;
		break;
	case DIR_Z_NEG:
		vec1.x = va.x; vec1.y = va.y;
		vec2.x = vb.x; vec2.y = vb.y;
		vec3.x = vc.x; vec3.y = vc.y;
		vect.x = p.x; vect.y = p.y;
		break;
	default:
		assert(0);
	}
	if(CrossProduct2D(vec2-vec1, vect-vec1) < 0.0f) return false;
	if(CrossProduct2D(vec3-vec1, vect-vec1) > 0.0f) return false;
	if(CrossProduct2D(vec3-vec2, vect-vec2) < 0.0f) return false;
	point.point = p;
	point.normal = normal;
	point.surfaceA = 0;
	point.pieceA = 0;
	point.surfaceB = tri.surface;
	point.pieceB = 0;
	mindist = t;
	return true;
}

bool
CCollision::ProcessSphereTriangle(const CColSphere &sphere,
	const CVector *verts, const CColTriangle &tri, const CColTrianglePlane &plane,
	CColPoint &point, float &mindistsq)
{
	// If sphere and plane don't intersect, no collision
	float planedist = plane.CalcPoint(sphere.center);
	float distsq = planedist*planedist;
	if(Abs(planedist) > sphere.radius || distsq > mindistsq)
		return false;

	const CVector &va = verts[tri.a];
	const CVector &vb = verts[tri.b];
	const CVector &vc = verts[tri.c];

	// calculate two orthogonal basis vectors for the triangle
	CVector normal;
	plane.GetNormal(normal);
	CVector vec2 = vb - va;
	float len = vec2.Magnitude();
	vec2 = vec2 * (1.0f/len);
	CVector vec1 = CrossProduct(vec2, normal);

	// We know A has local coordinate [0,0] and B has [0,len].
	// Now calculate coordinates on triangle for these two vectors:
	CVector vac = vc - va;
	CVector vas = sphere.center - va;
	CVector2D b(0.0f, len);
	CVector2D c(DotProduct(vec1, vac), DotProduct(vec2, vac));
	CVector2D s(DotProduct(vec1, vas), DotProduct(vec2, vas));

	// The three triangle lines partition the space into 6 sectors,
	// find out in which the center lies.
	int insideAB = CrossProduct2D(s, b) >= 0.0f;
	int insideAC = CrossProduct2D(c, s) >= 0.0f;
	int insideBC = CrossProduct2D(s-b, c-b) >= 0.0f;

	int testcase = insideAB + insideAC + insideBC;
	float dist = 0.0f;
	CVector p;
	if(testcase == 1){
		// closest to a vertex
		if(insideAB) p = vc;
		else if(insideAC) p = vb;
		else if(insideBC) p = va;
		else assert(0);
		dist = (sphere.center - p).Magnitude();
	}else if(testcase == 2){
		// closest to an edge
		if(!insideAB) dist = DistToLine(&va, &vb, &sphere.center, p);
		else if(!insideAC) dist = DistToLine(&va, &vc, &sphere.center, p);
		else if(!insideBC) dist = DistToLine(&vb, &vc, &sphere.center, p);
		else assert(0);
	}else if(testcase == 3){
		// center is in triangle
		dist = Abs(planedist);
		p = sphere.center - normal*planedist;
	}else
		assert(0);	// front fell off

	if(dist >= sphere.radius || dist*dist >= mindistsq)
		return false;

	point.point = p;
	point.normal = sphere.center - p;
	point.normal.Normalise();
	point.surfaceA = sphere.surface;
	point.pieceA = sphere.piece;
	point.surfaceB = tri.surface;
	point.pieceB = 0;
	point.depth = sphere.radius - dist;
	mindistsq = dist*dist;
	return true;
}

//--MIAMI: TODO
bool
CCollision::ProcessLineOfSight(const CColLine &line,
	const CMatrix &matrix, CColModel &model,
	CColPoint &point, float &mindist, bool ignoreSeeThrough, bool ignoreShootThrough)
{
	static CMatrix matTransform;
	int i;

	// transform line to model space
	Invert(matrix, matTransform);
	CColLine newline(matTransform * line.p0, matTransform * line.p1);

	// If we don't intersect with the bounding box, no chance on the rest
	if(!TestLineBox(newline, model.boundingBox))
		return false;

	float coldist = mindist;
	for(i = 0; i < model.numSpheres; i++)
		if(!ignoreSeeThrough || model.spheres[i].surface != SURFACE_GLASS && model.spheres[i].surface != SURFACE_TRANSPARENT_CLOTH)
			ProcessLineSphere(newline, model.spheres[i], point, coldist);

	for(i = 0; i < model.numBoxes; i++)
		if(!ignoreSeeThrough || model.boxes[i].surface != SURFACE_GLASS && model.boxes[i].surface != SURFACE_TRANSPARENT_CLOTH)
			ProcessLineBox(newline, model.boxes[i], point, coldist);

	CalculateTrianglePlanes(&model);
	for(i = 0; i < model.numTriangles; i++)
		if(!ignoreSeeThrough || model.triangles[i].surface != SURFACE_GLASS && model.triangles[i].surface != SURFACE_TRANSPARENT_CLOTH)
			ProcessLineTriangle(newline, model.vertices, model.triangles[i], model.trianglePlanes[i], point, coldist);

	if(coldist < mindist){
		point.point = matrix * point.point;
		point.normal = Multiply3x3(matrix, point.normal);
		mindist = coldist;
		return true;
	}
	return false;
}

//--MIAMI: TODO
bool
CCollision::ProcessVerticalLine(const CColLine &line,
	const CMatrix &matrix, CColModel &model,
	CColPoint &point, float &mindist, bool ignoreSeeThrough, bool ignoreShootThrough, CStoredCollPoly *poly)
{
	static CStoredCollPoly TempStoredPoly;
	int i;

	// transform line to model space
	// Why does the game seem to do this differently than above?
	CColLine newline(MultiplyInverse(matrix, line.p0), MultiplyInverse(matrix, line.p1));
	newline.p1.x = newline.p0.x;
	newline.p1.y = newline.p0.y;

	if(!TestVerticalLineBox(newline, model.boundingBox))
		return false;

	float coldist = mindist;
	for(i = 0; i < model.numSpheres; i++)
		if(!ignoreSeeThrough || model.spheres[i].surface != SURFACE_GLASS && model.spheres[i].surface != SURFACE_TRANSPARENT_CLOTH)
			ProcessLineSphere(newline, model.spheres[i], point, coldist);

	for(i = 0; i < model.numBoxes; i++)
		if(!ignoreSeeThrough || model.boxes[i].surface != SURFACE_GLASS && model.boxes[i].surface != SURFACE_TRANSPARENT_CLOTH)
			ProcessLineBox(newline, model.boxes[i], point, coldist);

	CalculateTrianglePlanes(&model);
	TempStoredPoly.valid = false;
	for(i = 0; i < model.numTriangles; i++)
		if(!ignoreSeeThrough || model.triangles[i].surface != SURFACE_GLASS && model.triangles[i].surface != SURFACE_TRANSPARENT_CLOTH)
			ProcessVerticalLineTriangle(newline, model.vertices, model.triangles[i], model.trianglePlanes[i], point, coldist, &TempStoredPoly);

	if(coldist < mindist){
		point.point = matrix * point.point;
		point.normal = Multiply3x3(matrix, point.normal);
		if(poly && TempStoredPoly.valid){
			*poly = TempStoredPoly;
			poly->verts[0] = matrix * poly->verts[0];
			poly->verts[1] = matrix * poly->verts[1];
			poly->verts[2] = matrix * poly->verts[2];
		}
		mindist = coldist;
		return true;
	}
	return false;
}

enum {
	MAXNUMSPHERES = 128,
	MAXNUMBOXES = 32,
	MAXNUMLINES = 16,
	MAXNUMTRIS = 600
};

// This checks model A's spheres and lines against model B's spheres, boxes and triangles.
// Returns the number of A's spheres that collide.
// Returned ColPoints are in world space.
// NB: only vehicles can have col models with lines, exactly 4, one for each wheel
int32
CCollision::ProcessColModels(const CMatrix &matrixA, CColModel &modelA,
	const CMatrix &matrixB, CColModel &modelB,
	CColPoint *spherepoints, CColPoint *linepoints, float *linedists)
{
	static int aSphereIndicesA[MAXNUMSPHERES];
	static int aLineIndicesA[MAXNUMLINES];
	static int aSphereIndicesB[MAXNUMSPHERES];
	static int aBoxIndicesB[MAXNUMBOXES];
	static int aTriangleIndicesB[MAXNUMTRIS];
	static bool aCollided[MAXNUMLINES];
	static CColSphere aSpheresA[MAXNUMSPHERES];
	static CColLine aLinesA[MAXNUMLINES];
	static CMatrix matAB, matBA;
	CColSphere s;
	int i, j;

	assert(modelA.numSpheres <= MAXNUMSPHERES);
	assert(modelA.numLines <= MAXNUMLINES);

	// From model A space to model B space
	matAB = Invert(matrixB, matAB) * matrixA;

	CColSphere bsphereAB;	// bounding sphere of A in B space
	bsphereAB.Set(modelA.boundingSphere.radius, matAB * modelA.boundingSphere.center);
	if(!TestSphereBox(bsphereAB, modelB.boundingBox))
		return 0;
	// B to A space
	matBA = Invert(matrixA, matBA) * matrixB;

	// transform modelA's spheres and lines to B space
	for(i = 0; i < modelA.numSpheres; i++){
		CColSphere &s = modelA.spheres[i];
		aSpheresA[i].Set(s.radius, matAB * s.center, s.surface, s.piece);
	}
	for(i = 0; i < modelA.numLines; i++)
		aLinesA[i].Set(matAB * modelA.lines[i].p0, matAB * modelA.lines[i].p1);

	// Test them against model B's bounding volumes
	int numSpheresA = 0;
	int numLinesA = 0;
	for(i = 0; i < modelA.numSpheres; i++)
		if(TestSphereBox(aSpheresA[i], modelB.boundingBox))
			aSphereIndicesA[numSpheresA++] = i;
	// no actual check???
	for(i = 0; i < modelA.numLines; i++)
		aLineIndicesA[numLinesA++] = i;
	// No collision
	if(numSpheresA == 0 && numLinesA == 0)
		return 0;

	// Check model B against A's bounding volumes
	int numSpheresB = 0;
	int numBoxesB = 0;
	int numTrianglesB = 0;
	for(i = 0; i < modelB.numSpheres; i++){
		s.Set(modelB.spheres[i].radius, matBA * modelB.spheres[i].center);
		if(TestSphereBox(s, modelA.boundingBox))
			aSphereIndicesB[numSpheresB++] = i;
	}
	for(i = 0; i < modelB.numBoxes; i++)
		if(TestSphereBox(bsphereAB, modelB.boxes[i]))
			aBoxIndicesB[numBoxesB++] = i;
	CalculateTrianglePlanes(&modelB);
	for(i = 0; i < modelB.numTriangles; i++)
		if(TestSphereTriangle(bsphereAB, modelB.vertices, modelB.triangles[i], modelB.trianglePlanes[i]))
			aTriangleIndicesB[numTrianglesB++] = i;
	assert(numSpheresB <= MAXNUMSPHERES);
	assert(numBoxesB <= MAXNUMBOXES);
	assert(numTrianglesB <= MAXNUMTRIS);
	// No collision
	if(numSpheresB == 0 && numBoxesB == 0 && numTrianglesB == 0)
		return 0;

	// We now have the collision volumes in A and B that are worth processing.

	// Process A's spheres against B's collision volumes
	int numCollisions = 0;
	for(i = 0; i < numSpheresA; i++){
		float coldist = 1.0e24f;
		bool hasCollided = false;

		for(j = 0; j < numSpheresB; j++)
			hasCollided |= ProcessSphereSphere(
				aSpheresA[aSphereIndicesA[i]],
				modelB.spheres[aSphereIndicesB[j]],
				spherepoints[numCollisions], coldist);
		for(j = 0; j < numBoxesB; j++)
			hasCollided |= ProcessSphereBox(
				aSpheresA[aSphereIndicesA[i]],
				modelB.boxes[aBoxIndicesB[j]],
				spherepoints[numCollisions], coldist);
		for(j = 0; j < numTrianglesB; j++)
			hasCollided |= ProcessSphereTriangle(
				aSpheresA[aSphereIndicesA[i]],
				modelB.vertices,
				modelB.triangles[aTriangleIndicesB[j]],
				modelB.trianglePlanes[aTriangleIndicesB[j]],
				spherepoints[numCollisions], coldist);

		if(hasCollided)
			numCollisions++;
	}
	for(i = 0; i < numCollisions; i++){
		spherepoints[i].point = matrixB * spherepoints[i].point;
		spherepoints[i].normal = Multiply3x3(matrixB, spherepoints[i].normal);
	}

	// And the same thing for the lines in A
	for(i = 0; i < numLinesA; i++){
		aCollided[i] = false;

		for(j = 0; j < numSpheresB; j++)
			aCollided[i] |= ProcessLineSphere(
				aLinesA[aLineIndicesA[i]],
				modelB.spheres[aSphereIndicesB[j]],
				linepoints[aLineIndicesA[i]],
				linedists[aLineIndicesA[i]]);
		for(j = 0; j < numBoxesB; j++)
			aCollided[i] |= ProcessLineBox(
				aLinesA[aLineIndicesA[i]],
				modelB.boxes[aBoxIndicesB[j]],
				linepoints[aLineIndicesA[i]],
				linedists[aLineIndicesA[i]]);
		for(j = 0; j < numTrianglesB; j++)
			aCollided[i] |= ProcessLineTriangle(
				aLinesA[aLineIndicesA[i]],
				modelB.vertices,
				modelB.triangles[aTriangleIndicesB[j]],
				modelB.trianglePlanes[aTriangleIndicesB[j]],
				linepoints[aLineIndicesA[i]],
				linedists[aLineIndicesA[i]]);
	}
	for(i = 0; i < numLinesA; i++)
		if(aCollided[i]){
			j = aLineIndicesA[i];
			linepoints[j].point = matrixB * linepoints[j].point;
			linepoints[j].normal = Multiply3x3(matrixB, linepoints[j].normal);
		}

	return numCollisions;	// sphere collisions
}


//
// Misc
//

float
CCollision::DistToLine(const CVector *l0, const CVector *l1, const CVector *point)
{
	float lensq = (*l1 - *l0).MagnitudeSqr();
	float dot = DotProduct(*point - *l0, *l1 - *l0);
	// Between 0 and len we're above the line.
	// if not, calculate distance to endpoint
	if(dot <= 0.0f)
		return (*point - *l0).Magnitude();
	if(dot >= lensq)
		return (*point - *l1).Magnitude();
	// distance to line
	return Sqrt((*point - *l0).MagnitudeSqr() - dot*dot/lensq);
}

// same as above but also return the point on the line
float
CCollision::DistToLine(const CVector *l0, const CVector *l1, const CVector *point, CVector &closest)
{
	float lensq = (*l1 - *l0).MagnitudeSqr();
	float dot = DotProduct(*point - *l0, *l1 - *l0);
	// find out which point we're closest to
	if(dot <= 0.0f)
		closest = *l0;
	else if(dot >= lensq)
		closest = *l1;
	else
		closest = *l0 + (*l1 - *l0)*(dot/lensq);
	// this is the distance
	return (*point - closest).Magnitude();
}

void
CCollision::CalculateTrianglePlanes(CColModel *model)
{
	assert(model);
	if(model->numTriangles == 0)
		return;

	CLink<CColModel*> *lptr;
	if(model->trianglePlanes){
		// re-insert at front so it's not removed again soon
		lptr = model->GetLinkPtr();
		lptr->Remove();
		ms_colModelCache.head.Insert(lptr);
	}else{
		lptr = ms_colModelCache.Insert(model);
		if(lptr == nil){
			// make room if we have to, remove last in list
			lptr = ms_colModelCache.tail.prev;
			assert(lptr);
			assert(lptr->item);
			lptr->item->RemoveTrianglePlanes();
			ms_colModelCache.Remove(lptr);
			// now this cannot fail
			lptr = ms_colModelCache.Insert(model);
			assert(lptr);
		}
		model->CalculateTrianglePlanes();
		model->SetLinkPtr(lptr);
	}
}

void
CCollision::DrawColModel(const CMatrix &mat, const CColModel &colModel)
{
	int i;
	CVector min, max;
	CVector verts[8];
	CVector c;
	float r;

	RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)TRUE);
	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)TRUE);
	RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDSRCALPHA);
	RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDINVSRCALPHA);
	RwRenderStateSet(rwRENDERSTATETEXTURERASTER, nil);

	min = colModel.boundingBox.min;
	max = colModel.boundingBox.max;

	verts[0] = mat * CVector(min.x, min.y, min.z);
	verts[1] = mat * CVector(min.x, min.y, max.z);
	verts[2] = mat * CVector(min.x, max.y, min.z);
	verts[3] = mat * CVector(min.x, max.y, max.z);
	verts[4] = mat * CVector(max.x, min.y, min.z);
	verts[5] = mat * CVector(max.x, min.y, max.z);
	verts[6] = mat * CVector(max.x, max.y, min.z);
	verts[7] = mat * CVector(max.x, max.y, max.z);

	CLines::RenderLineWithClipping(
		verts[0].x, verts[0].y, verts[0].z,
		verts[1].x, verts[1].y, verts[1].z,
		0xFF0000FF, 0xFF0000FF);
	CLines::RenderLineWithClipping(
		verts[1].x, verts[1].y, verts[1].z,
		verts[3].x, verts[3].y, verts[3].z,
		0xFF0000FF, 0xFF0000FF);
	CLines::RenderLineWithClipping(
		verts[3].x, verts[3].y, verts[3].z,
		verts[2].x, verts[2].y, verts[2].z,
		0xFF0000FF, 0xFF0000FF);
	CLines::RenderLineWithClipping(
		verts[2].x, verts[2].y, verts[2].z,
		verts[0].x, verts[0].y, verts[0].z,
		0xFF0000FF, 0xFF0000FF);

	CLines::RenderLineWithClipping(
		verts[4].x, verts[4].y, verts[4].z,
		verts[5].x, verts[5].y, verts[5].z,
		0xFF0000FF, 0xFF0000FF);
	CLines::RenderLineWithClipping(
		verts[5].x, verts[5].y, verts[5].z,
		verts[7].x, verts[7].y, verts[7].z,
		0xFF0000FF, 0xFF0000FF);
	CLines::RenderLineWithClipping(
		verts[7].x, verts[7].y, verts[7].z,
		verts[6].x, verts[6].y, verts[6].z,
		0xFF0000FF, 0xFF0000FF);
	CLines::RenderLineWithClipping(
		verts[6].x, verts[6].y, verts[6].z,
		verts[4].x, verts[4].y, verts[4].z,
		0xFF0000FF, 0xFF0000FF);

	CLines::RenderLineWithClipping(
		verts[0].x, verts[0].y, verts[0].z,
		verts[4].x, verts[4].y, verts[4].z,
		0xFF0000FF, 0xFF0000FF);
	CLines::RenderLineWithClipping(
		verts[1].x, verts[1].y, verts[1].z,
		verts[5].x, verts[5].y, verts[5].z,
		0xFF0000FF, 0xFF0000FF);
	CLines::RenderLineWithClipping(
		verts[2].x, verts[2].y, verts[2].z,
		verts[6].x, verts[6].y, verts[6].z,
		0xFF0000FF, 0xFF0000FF);
	CLines::RenderLineWithClipping(
		verts[3].x, verts[3].y, verts[3].z,
		verts[7].x, verts[7].y, verts[7].z,
		0xFF0000FF, 0xFF0000FF);

	for(i = 0; i < colModel.numSpheres; i++){
		c = mat * colModel.spheres[i].center;
		r = colModel.spheres[i].radius;

		CLines::RenderLineWithClipping(
			c.x,   c.y,   c.z-r,
			c.x-r, c.y-r, c.z,
			0xFF00FFFF, 0xFF00FFFF);
		CLines::RenderLineWithClipping(
			c.x,   c.y,   c.z-r,
			c.x-r, c.y+r, c.z,
			0xFF00FFFF, 0xFF00FFFF);
		CLines::RenderLineWithClipping(
			c.x,   c.y,   c.z-r,
			c.x+r, c.y-r, c.z,
			0xFF00FFFF, 0xFF00FFFF);
		CLines::RenderLineWithClipping(
			c.x,   c.y,   c.z-r,
			c.x+r, c.y+r, c.z,
			0xFF00FFFF, 0xFF00FFFF);
		CLines::RenderLineWithClipping(
			c.x-r, c.y-r, c.z,
			c.x,   c.y,   c.z+r,
			0xFF00FFFF, 0xFF00FFFF);
		CLines::RenderLineWithClipping(
			c.x-r, c.y+r, c.z,
			c.x,   c.y,   c.z+r,
			0xFF00FFFF, 0xFF00FFFF);
		CLines::RenderLineWithClipping(
			c.x+r, c.y-r, c.z,
			c.x,   c.y,   c.z+r,
			0xFF00FFFF, 0xFF00FFFF);
		CLines::RenderLineWithClipping(
			c.x+r, c.y+r, c.z,
			c.x,   c.y,   c.z+r,
			0xFF00FFFF, 0xFF00FFFF);
	}

	for(i = 0; i < colModel.numLines; i++){
		verts[0] = colModel.lines[i].p0;
		verts[1] = colModel.lines[i].p1;

		verts[0] = mat * verts[0];
		verts[1] = mat * verts[1];

		CLines::RenderLineWithClipping(
			verts[0].x, verts[0].y, verts[0].z,
			verts[1].x, verts[1].y, verts[1].z,
			0x00FFFFFF, 0x00FFFFFF);
	}

	for(i = 0; i < colModel.numBoxes; i++){
		min = colModel.boxes[i].min;
		max = colModel.boxes[i].max;

		verts[0] = mat * CVector(min.x, min.y, min.z);
		verts[1] = mat * CVector(min.x, min.y, max.z);
		verts[2] = mat * CVector(min.x, max.y, min.z);
		verts[3] = mat * CVector(min.x, max.y, max.z);
		verts[4] = mat * CVector(max.x, min.y, min.z);
		verts[5] = mat * CVector(max.x, min.y, max.z);
		verts[6] = mat * CVector(max.x, max.y, min.z);
		verts[7] = mat * CVector(max.x, max.y, max.z);

		CLines::RenderLineWithClipping(
			verts[0].x, verts[0].y, verts[0].z,
			verts[1].x, verts[1].y, verts[1].z,
			0xFFFFFFFF, 0xFFFFFFFF);
		CLines::RenderLineWithClipping(
			verts[1].x, verts[1].y, verts[1].z,
			verts[3].x, verts[3].y, verts[3].z,
			0xFFFFFFFF, 0xFFFFFFFF);
		CLines::RenderLineWithClipping(
			verts[3].x, verts[3].y, verts[3].z,
			verts[2].x, verts[2].y, verts[2].z,
			0xFFFFFFFF, 0xFFFFFFFF);
		CLines::RenderLineWithClipping(
			verts[2].x, verts[2].y, verts[2].z,
			verts[0].x, verts[0].y, verts[0].z,
			0xFFFFFFFF, 0xFFFFFFFF);

		CLines::RenderLineWithClipping(
			verts[4].x, verts[4].y, verts[4].z,
			verts[5].x, verts[5].y, verts[5].z,
			0xFFFFFFFF, 0xFFFFFFFF);
		CLines::RenderLineWithClipping(
			verts[5].x, verts[5].y, verts[5].z,
			verts[7].x, verts[7].y, verts[7].z,
			0xFFFFFFFF, 0xFFFFFFFF);
		CLines::RenderLineWithClipping(
			verts[7].x, verts[7].y, verts[7].z,
			verts[6].x, verts[6].y, verts[6].z,
			0xFFFFFFFF, 0xFFFFFFFF);
		CLines::RenderLineWithClipping(
			verts[6].x, verts[6].y, verts[6].z,
			verts[4].x, verts[4].y, verts[4].z,
			0xFFFFFFFF, 0xFFFFFFFF);

		CLines::RenderLineWithClipping(
			verts[0].x, verts[0].y, verts[0].z,
			verts[4].x, verts[4].y, verts[4].z,
			0xFFFFFFFF, 0xFFFFFFFF);
		CLines::RenderLineWithClipping(
			verts[1].x, verts[1].y, verts[1].z,
			verts[5].x, verts[5].y, verts[5].z,
			0xFFFFFFFF, 0xFFFFFFFF);
		CLines::RenderLineWithClipping(
			verts[2].x, verts[2].y, verts[2].z,
			verts[6].x, verts[6].y, verts[6].z,
			0xFFFFFFFF, 0xFFFFFFFF);
		CLines::RenderLineWithClipping(
			verts[3].x, verts[3].y, verts[3].z,
			verts[7].x, verts[7].y, verts[7].z,
			0xFFFFFFFF, 0xFFFFFFFF);
	}

	for(i = 0; i < colModel.numTriangles; i++){
		colModel.GetTrianglePoint(verts[0], colModel.triangles[i].a);
		colModel.GetTrianglePoint(verts[1], colModel.triangles[i].b);
		colModel.GetTrianglePoint(verts[2], colModel.triangles[i].c);
		verts[0] = mat * verts[0];
		verts[1] = mat * verts[1];
		verts[2] = mat * verts[2];
		CLines::RenderLineWithClipping(
			verts[0].x, verts[0].y, verts[0].z,
			verts[1].x, verts[1].y, verts[1].z,
			0x00FF00FF, 0x00FF00FF);
		CLines::RenderLineWithClipping(
			verts[0].x, verts[0].y, verts[0].z,
			verts[2].x, verts[2].y, verts[2].z,
			0x00FF00FF, 0x00FF00FF);
		CLines::RenderLineWithClipping(
			verts[1].x, verts[1].y, verts[1].z,
			verts[2].x, verts[2].y, verts[2].z,
			0x00FF00FF, 0x00FF00FF);
	}

	RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDSRCALPHA);
	RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDINVSRCALPHA);
	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)FALSE);
	RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)TRUE);
	RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*)TRUE);
}

void
CCollision::DrawColModel_Coloured(const CMatrix &mat, const CColModel &colModel, int32 id)
{
	int i;
	int s;
	float f;
	CVector verts[8];
	CVector min, max;
	int r, g, b;
	RwImVertexIndex *iptr;
	RwIm3DVertex *vptr;

	RenderBuffer::ClearRenderBuffer();
	RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)TRUE);
	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)TRUE);
	RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDSRCALPHA);
	RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDINVSRCALPHA);
	RwRenderStateSet(rwRENDERSTATETEXTURERASTER, nil);

	for(i = 0; i < colModel.numTriangles; i++){
		colModel.GetTrianglePoint(verts[0], colModel.triangles[i].a);
		colModel.GetTrianglePoint(verts[1], colModel.triangles[i].b);
		colModel.GetTrianglePoint(verts[2], colModel.triangles[i].c);
		verts[0] = mat * verts[0];
		verts[1] = mat * verts[1];
		verts[2] = mat * verts[2];

		// game doesn't do this
		r = 255;
		g = 128;
		b = 0;

		s = colModel.triangles[i].surface;
		f = (s & 0xF)/32.0f + 0.5f;
		switch(CSurfaceTable::GetAdhesionGroup(s)){
		case ADHESIVE_RUBBER:
			r = f * 255.0f;
			g = 0;
			b = 0;
			break;
		case ADHESIVE_HARD:
			r = f*255.0f;
			g = f*255.0f;
			b = f*128.0f;
			break;
		case ADHESIVE_ROAD:
			r = f*128.0f;
			g = f*128.0f;
			b = f*128.0f;
			break;
		case ADHESIVE_LOOSE:
			r = 0;
			g = f * 255.0f;
			b = 0;
			break;
		case ADHESIVE_WET:
			r = 0;
			g = 0;
			b = f * 255.0f;
			break;
		default:
			// this doesn't make much sense
			r *= f;
			g *= f;
			b *= f;
		}

		if(s == SURFACE_TRANSPARENT_CLOTH || s == SURFACE_METAL_CHAIN_FENCE ||
		   s == SURFACE_TRANSPARENT_STONE || s == SURFACE_SCAFFOLD_POLE)
			if(CTimer::GetFrameCounter() & 1){
				r = 0;
				g = 0;
				b = 0;
			}

		if(s > SURFACE_METAL_GATE){
			r = CGeneral::GetRandomNumber();
			g = CGeneral::GetRandomNumber();
			b = CGeneral::GetRandomNumber();
			printf("Illegal surfacetype:%d on MI:%d\n", s, id);
		}

		RenderBuffer::StartStoring(6, 3, &iptr, &vptr);
		RwIm3DVertexSetRGBA(&vptr[0], r, g, b, 255);
		RwIm3DVertexSetRGBA(&vptr[1], r, g, b, 255);
		RwIm3DVertexSetRGBA(&vptr[2], r, g, b, 255);
		RwIm3DVertexSetU(&vptr[0], 0.0f);
		RwIm3DVertexSetV(&vptr[0], 0.0f);
		RwIm3DVertexSetU(&vptr[1], 0.0f);
		RwIm3DVertexSetV(&vptr[1], 1.0f);
		RwIm3DVertexSetU(&vptr[2], 1.0f);
		RwIm3DVertexSetV(&vptr[2], 1.0f);
		RwIm3DVertexSetPos(&vptr[0], verts[0].x, verts[0].y, verts[0].z);
		RwIm3DVertexSetPos(&vptr[1], verts[1].x, verts[1].y, verts[1].z);
		RwIm3DVertexSetPos(&vptr[2], verts[2].x, verts[2].y, verts[2].z);
		iptr[0] = 0; iptr[1] = 1; iptr[2] = 2;
		iptr[3] = 0; iptr[4] = 2; iptr[5] = 1;
		RenderBuffer::StopStoring();
	}

	for(i = 0; i < colModel.numBoxes; i++){
		min = colModel.boxes[i].min;
		max = colModel.boxes[i].max;

		verts[0] = mat * CVector(min.x, min.y, min.z);
		verts[1] = mat * CVector(min.x, min.y, max.z);
		verts[2] = mat * CVector(min.x, max.y, min.z);
		verts[3] = mat * CVector(min.x, max.y, max.z);
		verts[4] = mat * CVector(max.x, min.y, min.z);
		verts[5] = mat * CVector(max.x, min.y, max.z);
		verts[6] = mat * CVector(max.x, max.y, min.z);
		verts[7] = mat * CVector(max.x, max.y, max.z);

		s = colModel.boxes[i].surface;
		f = (s & 0xF)/32.0f + 0.5f;
		switch(CSurfaceTable::GetAdhesionGroup(s)){
		case ADHESIVE_RUBBER:
			r = f * 255.0f;
			g = 0;
			b = 0;
			break;
		case ADHESIVE_HARD:
			r = f*255.0f;
			g = f*255.0f;
			b = f*128.0f;
			break;
		case ADHESIVE_ROAD:
			r = f*128.0f;
			g = f*128.0f;
			b = f*128.0f;
			break;
		case ADHESIVE_LOOSE:
			r = 0;
			g = f * 255.0f;
			b = 0;
			break;
		case ADHESIVE_WET:
			r = 0;
			g = 0;
			b = f * 255.0f;
			break;
		default:
			// this doesn't make much sense
			r *= f;
			g *= f;
			b *= f;
		}

		if(s == SURFACE_TRANSPARENT_CLOTH || s == SURFACE_METAL_CHAIN_FENCE ||
		   s == SURFACE_TRANSPARENT_STONE || s == SURFACE_SCAFFOLD_POLE)
			if(CTimer::GetFrameCounter() & 1){
				r = 0;
				g = 0;
				b = 0;
			}

		RenderBuffer::StartStoring(36, 8, &iptr, &vptr);
		RwIm3DVertexSetRGBA(&vptr[0], r, g, b, 255);
		RwIm3DVertexSetRGBA(&vptr[1], r, g, b, 255);
		RwIm3DVertexSetRGBA(&vptr[2], r, g, b, 255);
		RwIm3DVertexSetRGBA(&vptr[3], r, g, b, 255);
		RwIm3DVertexSetRGBA(&vptr[4], r, g, b, 255);
		RwIm3DVertexSetRGBA(&vptr[5], r, g, b, 255);
		RwIm3DVertexSetRGBA(&vptr[6], r, g, b, 255);
		RwIm3DVertexSetRGBA(&vptr[7], r, g, b, 255);
		RwIm3DVertexSetU(&vptr[0], 0.0f);
		RwIm3DVertexSetV(&vptr[0], 0.0f);
		RwIm3DVertexSetU(&vptr[1], 0.0f);
		RwIm3DVertexSetV(&vptr[1], 1.0f);
		RwIm3DVertexSetU(&vptr[2], 1.0f);
		RwIm3DVertexSetV(&vptr[2], 1.0f);
		RwIm3DVertexSetU(&vptr[3], 0.0f);
		RwIm3DVertexSetV(&vptr[3], 0.0f);
		RwIm3DVertexSetU(&vptr[4], 0.0f);
		RwIm3DVertexSetV(&vptr[4], 1.0f);
		RwIm3DVertexSetU(&vptr[5], 1.0f);
		RwIm3DVertexSetV(&vptr[5], 1.0f);
		RwIm3DVertexSetU(&vptr[6], 0.0f);
		RwIm3DVertexSetV(&vptr[6], 1.0f);
		RwIm3DVertexSetU(&vptr[7], 1.0f);
		RwIm3DVertexSetV(&vptr[7], 1.0f);
		RwIm3DVertexSetPos(&vptr[0], verts[0].x, verts[0].y, verts[0].z);
		RwIm3DVertexSetPos(&vptr[1], verts[1].x, verts[1].y, verts[1].z);
		RwIm3DVertexSetPos(&vptr[2], verts[2].x, verts[2].y, verts[2].z);
		RwIm3DVertexSetPos(&vptr[3], verts[3].x, verts[3].y, verts[3].z);
		RwIm3DVertexSetPos(&vptr[4], verts[4].x, verts[4].y, verts[4].z);
		RwIm3DVertexSetPos(&vptr[5], verts[5].x, verts[5].y, verts[5].z);
		RwIm3DVertexSetPos(&vptr[6], verts[6].x, verts[6].y, verts[6].z);
		RwIm3DVertexSetPos(&vptr[7], verts[7].x, verts[7].y, verts[7].z);
		iptr[0] = 0; iptr[1] = 1; iptr[2] = 2;
		iptr[3] = 1; iptr[4] = 3; iptr[5] = 2;
		iptr[6] = 1; iptr[7] = 5; iptr[8] = 7;
		iptr[9] = 1; iptr[10] = 7; iptr[11] = 3;
		iptr[12] = 2; iptr[13] = 3; iptr[14] = 7;
		iptr[15] = 2; iptr[16] = 7; iptr[17] = 6;
		iptr[18] = 0; iptr[19] = 5; iptr[20] = 1;
		iptr[21] = 0; iptr[22] = 4; iptr[23] = 5;
		iptr[24] = 0; iptr[25] = 2; iptr[26] = 4;
		iptr[27] = 2; iptr[28] = 6; iptr[29] = 4;
		iptr[30] = 4; iptr[31] = 6; iptr[32] = 7;
		iptr[33] = 4; iptr[34] = 7; iptr[35] = 5;
		RenderBuffer::StopStoring();
	}
 
	RenderBuffer::RenderStuffInBuffer();
	RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDSRCALPHA);
	RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDINVSRCALPHA);
	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)FALSE);
	RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)TRUE);
	RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*)TRUE);
}


/*
 * ColModel code
 */

void
CColSphere::Set(float radius, const CVector &center, uint8 surf, uint8 piece)
{
	this->radius = radius;
	this->center = center;
	this->surface = surf;
	this->piece = piece;
}

void
CColBox::Set(const CVector &min, const CVector &max, uint8 surf, uint8 piece)
{
	this->min = min;
	this->max = max;
	this->surface = surf;
	this->piece = piece;
}

void
CColLine::Set(const CVector &p0, const CVector &p1)
{
	this->p0 = p0;
	this->p1 = p1;
}

void
CColTriangle::Set(const CVector *, int a, int b, int c, uint8 surf, uint8 piece)
{
	this->a = a;
	this->b = b;
	this->c = c;
	this->surface = surf;
}

void
CColTrianglePlane::Set(const CVector *v, CColTriangle &tri)
{
	const CVector &va = v[tri.a];
	const CVector &vb = v[tri.b];
	const CVector &vc = v[tri.c];

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

CColModel::CColModel(void)
{
	numSpheres = 0;
	spheres = nil;
	numLines = 0;
	lines = nil;
	numBoxes = 0;
	boxes = nil;
	numTriangles = 0;
	vertices = nil;
	triangles = nil;
	trianglePlanes = nil;
	level = 0;	// generic col slot
	ownsCollisionVolumes = true;
}

CColModel::~CColModel(void)
{
	RemoveCollisionVolumes();
	RemoveTrianglePlanes();
}

void
CColModel::RemoveCollisionVolumes(void)
{
	if(ownsCollisionVolumes){
		RwFree(spheres);
		RwFree(lines);
		RwFree(boxes);
		RwFree(vertices);
		RwFree(triangles);
	}
	numSpheres = 0;
	numLines = 0;
	numBoxes = 0;
	numTriangles = 0;
	spheres = nil;
	lines = nil;
	boxes = nil;
	vertices = nil;
	triangles = nil;
}

void
CColModel::CalculateTrianglePlanes(void)
{
	// HACK: allocate space for one more element to stuff the link pointer into
	trianglePlanes = (CColTrianglePlane*)RwMalloc(sizeof(CColTrianglePlane) * (numTriangles+1));
	for(int i = 0; i < numTriangles; i++)
		trianglePlanes[i].Set(vertices, triangles[i]);
}

void
CColModel::RemoveTrianglePlanes(void)
{
	RwFree(trianglePlanes);
	trianglePlanes = nil;
}

void
CColModel::SetLinkPtr(CLink<CColModel*> *lptr)
{
	assert(trianglePlanes);
	*(CLink<CColModel*>**)ALIGNPTR(&trianglePlanes[numTriangles]) = lptr;
}

CLink<CColModel*>*
CColModel::GetLinkPtr(void)
{
	assert(trianglePlanes);
	return *(CLink<CColModel*>**)ALIGNPTR(&trianglePlanes[numTriangles]);
}

void
CColModel::GetTrianglePoint(CVector &v, int i) const
{
	v = vertices[i];
}

CColModel&
CColModel::operator=(const CColModel &other)
{
	int i;
	int numVerts;

	boundingSphere = other.boundingSphere;
	boundingBox = other.boundingBox;

	// copy spheres
	if(other.numSpheres){
		if(numSpheres != other.numSpheres){
			numSpheres = other.numSpheres;
			if(spheres)
				RwFree(spheres);
			spheres = (CColSphere*)RwMalloc(numSpheres*sizeof(CColSphere));
		}
		for(i = 0; i < numSpheres; i++)
			spheres[i] = other.spheres[i];
	}else{
		numSpheres = 0;
		if(spheres)
			RwFree(spheres);
		spheres = nil;
	}

	// copy lines
	if(other.numLines){
		if(numLines != other.numLines){
			numLines = other.numLines;
			if(lines)
				RwFree(lines);
			lines = (CColLine*)RwMalloc(numLines*sizeof(CColLine));
		}
		for(i = 0; i < numLines; i++)
			lines[i] = other.lines[i];
	}else{
		numLines = 0;
		if(lines)
			RwFree(lines);
		lines = nil;
	}

	// copy boxes
	if(other.numBoxes){
		if(numBoxes != other.numBoxes){
			numBoxes = other.numBoxes;
			if(boxes)
				RwFree(boxes);
			boxes = (CColBox*)RwMalloc(numBoxes*sizeof(CColBox));
		}
		for(i = 0; i < numBoxes; i++)
			boxes[i] = other.boxes[i];
	}else{
		numBoxes = 0;
		if(boxes)
			RwFree(boxes);
		boxes = nil;
	}

	// copy mesh
	if(other.numTriangles){
		// copy vertices
		numVerts = 0;
		for(i = 0; i < other.numTriangles; i++){
			if(other.triangles[i].a > numVerts)
				numVerts = other.triangles[i].a;
			if(other.triangles[i].b > numVerts)
				numVerts = other.triangles[i].b;
			if(other.triangles[i].c > numVerts)
				numVerts = other.triangles[i].c;
		}
		numVerts++;
		if(vertices)
			RwFree(vertices);
		if(numVerts){
			vertices = (CVector*)RwMalloc(numVerts*sizeof(CVector));
			for(i = 0; i < numVerts; i++)
				vertices[i] = other.vertices[i];
		}

		// copy triangles
		if(numTriangles != other.numTriangles){
			numTriangles = other.numTriangles;
			if(triangles)
				RwFree(triangles);
			triangles = (CColTriangle*)RwMalloc(numTriangles*sizeof(CColTriangle));
		}
		for(i = 0; i < numTriangles; i++)
			triangles[i] = other.triangles[i];
	}else{
		numTriangles = 0;
		if(triangles)
			RwFree(triangles);
		triangles = nil;
		if(vertices)
			RwFree(vertices);
		vertices = nil;
	}
	return *this;
}
