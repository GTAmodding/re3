#include "common.h"

#include "VuVector.h"
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
#include "Frontend.h"

#ifdef VU_COLLISION
#include "VuCollision.h"

inline int
GetVUresult(void)
{
#ifdef GTA_PS2
	int ret;
	__asm__ volatile (
		"cfc2.i	%0,vi01\n"	// .i important! wait for VU0 to finish
		: "=r" (ret)
	);
	return ret;
#else
	return vi01;
#endif
}

inline int
GetVUresult(CVuVector &point, CVuVector &normal, float &dist)
{
#ifdef GTA_PS2
	int ret;
	__asm__ volatile (
		"cfc2.i	%0,vi01\n"	// .i important! wait for VU0 to finish
		"sqc2	vf01,(%1)\n"
		"sqc2	vf02,(%2)\n"
		"qmfc2	$12,vf03\n"
		"sw	$12,(%3)\n"
		: "=r" (ret)
		: "r" (&point), "r" (&normal), "r" (&dist)
		: "$12"
	);
	return ret;
#else
	point = vf01;
	normal = vf02;
	dist = vf03.x;
	return vi01;
#endif
}

#endif

eLevelName CCollision::ms_collisionInMemory;
CLinkList<CColModel*> CCollision::ms_colModelCache;

void
CCollision::Init(void)
{
	ms_colModelCache.Init(NUMCOLCACHELINKS);
	ms_collisionInMemory = LEVEL_GENERIC;
}

void
CCollision::Shutdown(void)
{
	ms_colModelCache.Shutdown();
}

void
CCollision::Update(void)
{
	CVector playerCoors;
	playerCoors = FindPlayerCoors();
	eLevelName level = CTheZones::m_CurrLevel;
	bool forceLevelChange = false;

	if(CTimer::GetTimeInMilliseconds() < 2000 || CCutsceneMgr::IsCutsceneProcessing())
		return;

	// hardcode a level if there are no zones
	if(level == LEVEL_GENERIC){
		if(CGame::currLevel == LEVEL_INDUSTRIAL &&
		   playerCoors.x < 400.0f){
			level = LEVEL_COMMERCIAL;
			forceLevelChange = true;
		}else if(CGame::currLevel == LEVEL_SUBURBAN &&
		         playerCoors.x > -450.0f && playerCoors.y < -1400.0f){
			level = LEVEL_COMMERCIAL;
			forceLevelChange = true;
		}else{
			if(playerCoors.x > 800.0f){
				level = LEVEL_INDUSTRIAL;
				forceLevelChange = true;
			}else if(playerCoors.x < -800.0f){
				level = LEVEL_SUBURBAN;
				forceLevelChange = true;
			}
		}
	}
	if(level != LEVEL_GENERIC && level != CGame::currLevel)
		CGame::currLevel = level;
	if(ms_collisionInMemory != CGame::currLevel)
		LoadCollisionWhenINeedIt(forceLevelChange);
	CStreaming::HaveAllBigBuildingsLoaded(CGame::currLevel);
}

eLevelName
GetCollisionInSectorList(CPtrList &list)
{
	CPtrNode *node;
	CEntity *e;
	int level;

	for(node = list.first; node; node = node->next){
		e = (CEntity*)node->item;
		level = CModelInfo::GetModelInfo(e->GetModelIndex())->GetColModel()->level;
		if(level != LEVEL_GENERIC)
			return (eLevelName)level;
	}
	return LEVEL_GENERIC;
}

// Get a level this sector is in based on collision models
eLevelName
GetCollisionInSector(CSector &sect)
{
	int level;

	level = GetCollisionInSectorList(sect.m_lists[ENTITYLIST_BUILDINGS]);
	if(level == LEVEL_GENERIC)
		level = GetCollisionInSectorList(sect.m_lists[ENTITYLIST_BUILDINGS_OVERLAP]);
	if(level == LEVEL_GENERIC)
		level = GetCollisionInSectorList(sect.m_lists[ENTITYLIST_OBJECTS]);
	if(level == LEVEL_GENERIC)
		level = GetCollisionInSectorList(sect.m_lists[ENTITYLIST_OBJECTS_OVERLAP]);
	if(level == LEVEL_GENERIC)
		level = GetCollisionInSectorList(sect.m_lists[ENTITYLIST_DUMMIES]);
	if(level == LEVEL_GENERIC)
		level = GetCollisionInSectorList(sect.m_lists[ENTITYLIST_DUMMIES_OVERLAP]);
	return (eLevelName)level;
}

void
CCollision::LoadCollisionWhenINeedIt(bool forceChange)
{
	eLevelName level, l;
	bool multipleLevels;
	CVector playerCoors;
	CVehicle *veh;
	CEntryInfoNode *ei;
	int sx, sy;
	int xmin, xmax, ymin, ymax;
	int x, y;

	level = LEVEL_GENERIC;

	playerCoors = FindPlayerCoors();
	sx = CWorld::GetSectorIndexX(playerCoors.x);
	sy = CWorld::GetSectorIndexY(playerCoors.y);
	multipleLevels = false;

	veh = FindPlayerVehicle();
	if(veh && veh->IsTrain()){
		if(((CTrain*)veh)->m_nDoorState != TRAIN_DOOR_OPEN)
			return;
	}else if(playerCoors.z < -4.0f && !CCullZones::DoINeedToLoadCollision())
		return;

	// Figure out whose level's collisions we're most likely to be interested in
	if(!forceChange){
		if(veh && veh->IsBoat()){
			// on water we expect to be between levels
			multipleLevels = true;
		}else{
			xmin = Max(sx - 1, 0);
			xmax = Min(sx + 1, NUMSECTORS_X-1);
			ymin = Max(sy - 1, 0);
			ymax = Min(sy + 1, NUMSECTORS_Y-1);

			for(x = xmin; x <= xmax; x++)
				for(y = ymin; y <= ymax; y++){
					l = GetCollisionInSector(*CWorld::GetSector(x, y));
					if(l != LEVEL_GENERIC){
						if(level == LEVEL_GENERIC)
							level = l;
						if(level != l)
							multipleLevels = true;
					}
				}
		}

		if(multipleLevels && veh && veh->IsBoat())
			for(ei = veh->m_entryInfoList.first; ei; ei = ei->next){
				level = GetCollisionInSector(*ei->sector);
				if(level != LEVEL_GENERIC)
					break;
			}
	}

	if (level == CGame::currLevel || forceChange) {
#ifdef FIX_BUGS
		CTimer::Suspend();
#else
		CTimer::Stop();
#endif
		ISLAND_LOADING_IS(LOW)
		{
			DMAudio.SetEffectsFadeVol(0);
			CPad::StopPadsShaking();
			LoadCollisionScreen(CGame::currLevel);
			DMAudio.Service();
		}

		CPopulation::DealWithZoneChange(ms_collisionInMemory, CGame::currLevel, false);

		ISLAND_LOADING_ISNT(HIGH)
		{
			CStreaming::RemoveIslandsNotUsed(LEVEL_INDUSTRIAL);
			CStreaming::RemoveIslandsNotUsed(LEVEL_COMMERCIAL);
			CStreaming::RemoveIslandsNotUsed(LEVEL_SUBURBAN);
		}
		ISLAND_LOADING_IS(LOW)
		{
			CStreaming::RemoveBigBuildings(LEVEL_INDUSTRIAL);
			CStreaming::RemoveBigBuildings(LEVEL_COMMERCIAL);
			CStreaming::RemoveBigBuildings(LEVEL_SUBURBAN);
			CModelInfo::RemoveColModelsFromOtherLevels(CGame::currLevel);
			CStreaming::RemoveUnusedModelsInLoadedList();
			CGame::TidyUpMemory(true, true);
			CFileLoader::LoadCollisionFromDatFile(CGame::currLevel);
		}

		ms_collisionInMemory = CGame::currLevel;
		CReplay::EmptyReplayBuffer();
		ISLAND_LOADING_IS(LOW)
		{
			if (CGame::currLevel != LEVEL_GENERIC)
				LoadSplash(GetLevelSplashScreen(CGame::currLevel));
			CStreaming::RemoveUnusedBigBuildings(CGame::currLevel);
			CStreaming::RemoveUnusedBuildings(CGame::currLevel);
			CStreaming::RequestBigBuildings(CGame::currLevel);
		}
#ifdef NO_ISLAND_LOADING
		else if (CMenuManager::m_PrefsIslandLoading == CMenuManager::ISLAND_LOADING_MEDIUM)
			CStreaming::RequestIslands(CGame::currLevel);
#endif
		CStreaming::LoadAllRequestedModels(true);

		ISLAND_LOADING_IS(LOW)
		{
			CStreaming::HaveAllBigBuildingsLoaded(CGame::currLevel);

			CGame::TidyUpMemory(true, true);
		}
#ifdef FIX_BUGS
		CTimer::Resume();
#else
		CTimer::Update();
#endif
		ISLAND_LOADING_IS(LOW)
			DMAudio.SetEffectsFadeVol(127);
	}
}

#ifdef NO_ISLAND_LOADING
bool CCollision::bAlreadyLoaded = false;
#endif
void
CCollision::SortOutCollisionAfterLoad(void)
{
	if(ms_collisionInMemory == CGame::currLevel)
		return;
	ISLAND_LOADING_IS(LOW)
		CModelInfo::RemoveColModelsFromOtherLevels(CGame::currLevel);

	if (CGame::currLevel != LEVEL_GENERIC) {
#ifdef NO_ISLAND_LOADING
		if (CMenuManager::m_PrefsIslandLoading != CMenuManager::ISLAND_LOADING_LOW) {
			if (bAlreadyLoaded) {
				ms_collisionInMemory = CGame::currLevel;
				return;
			}
			bAlreadyLoaded = true;
			CFileLoader::LoadCollisionFromDatFile(LEVEL_INDUSTRIAL);
			CFileLoader::LoadCollisionFromDatFile(LEVEL_COMMERCIAL);
			CFileLoader::LoadCollisionFromDatFile(LEVEL_SUBURBAN);
		} else
#endif
		CFileLoader::LoadCollisionFromDatFile(CGame::currLevel);
		if(!CGame::playingIntro)
			LoadSplash(GetLevelSplashScreen(CGame::currLevel));
	}
	ms_collisionInMemory = CGame::currLevel;
	CGame::TidyUpMemory(true, false);
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
CCollision::TestSphereSphere(const CColSphere &s1, const CColSphere &s2)
{
	float d = s1.radius + s2.radius;
	return (s1.center - s2.center).MagnitudeSqr() < d*d;
}

bool
CCollision::TestSphereBox(const CColSphere &sph, const CColBox &box)
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
CCollision::TestLineBox(const CColLine &line, const CColBox &box)
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
CCollision::TestVerticalLineBox(const CColLine &line, const CColBox &box)
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
CCollision::TestLineTriangle(const CColLine &line, const CompressedVector *verts, const CColTriangle &tri, const CColTrianglePlane &plane)
{
#ifdef VU_COLLISION
	// not used in favour of optimized loops
	VuTriangle vutri;
	verts[tri.a].Unpack(vutri.v0);
	verts[tri.b].Unpack(vutri.v1);
	verts[tri.c].Unpack(vutri.v2);
	plane.Unpack(vutri.plane);

	LineToTriangleCollisionCompressed(*(CVuVector*)&line.p0, *(CVuVector*)&line.p1, vutri);

	if(GetVUresult())
		return true;
	return false;
#else
	float t;
	CVector normal;
	plane.GetNormal(normal);

	// if points are on the same side, no collision
	if(plane.CalcPoint(line.p0) * plane.CalcPoint(line.p1) > 0.0f)
		return false;

	float p0dist = DotProduct(line.p1 - line.p0, normal);

#ifdef FIX_BUGS
	// line lines in the plane, assume no collision
	if (p0dist == 0.0f)
		return false;
#endif

	// intersection parameter on line
	t = -plane.CalcPoint(line.p0) / p0dist;
	// find point of intersection
	CVector p = line.p0 + (line.p1-line.p0)*t;

	const CVector &va = verts[tri.a].Get();
	const CVector &vb = verts[tri.b].Get();
	const CVector &vc = verts[tri.c].Get();
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
#endif
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
	const CompressedVector *verts, const CColTriangle &tri, const CColTrianglePlane &plane)
{
#ifdef VU_COLLISION
	// not used in favour of optimized loops
	VuTriangle vutri;
	verts[tri.a].Unpack(vutri.v0);
	verts[tri.b].Unpack(vutri.v1);
	verts[tri.c].Unpack(vutri.v2);
	plane.Unpack(vutri.plane);

	SphereToTriangleCollisionCompressed(*(CVuVector*)&sphere, vutri);

	if(GetVUresult())
		return true;
	return false;
#else
	// If sphere and plane don't intersect, no collision
	float planedist = plane.CalcPoint(sphere.center);
	if(Abs(planedist) > sphere.radius)
		return false;

	const CVector &va = verts[tri.a].Get();
	const CVector &vb = verts[tri.b].Get();
	const CVector &vc = verts[tri.c].Get();

	// calculate two orthogonal basis vectors for the triangle
	CVector vec2 = vb - va;
	float len = vec2.Magnitude();
	vec2 = vec2 * (1.0f/len);
	CVector normal;
	plane.GetNormal(normal);
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
	switch(testcase){
	case 1:
		// closest to a vertex
		if(insideAB) dist = (sphere.center - vc).Magnitude();
		else if(insideAC) dist = (sphere.center - vb).Magnitude();
		else if(insideBC) dist = (sphere.center - va).Magnitude();
		else assert(0);
		break;
	case 2:
		// closest to an edge
		// looks like original game as DistToLine manually inlined
		if(!insideAB) dist = DistToLine(&va, &vb, &sphere.center);
		else if(!insideAC) dist = DistToLine(&va, &vc, &sphere.center);
		else if(!insideBC) dist = DistToLine(&vb, &vc, &sphere.center);
		else assert(0);
		break;
	case 3:
		// center is in triangle
		dist = Abs(planedist);
		break;
	default:
		assert(0);
	}

	return dist < sphere.radius;
#endif
}

bool
CCollision::TestLineOfSight(const CColLine &line, const CMatrix &matrix, CColModel &model, bool ignoreSeeThrough)
{
#ifdef VU_COLLISION
	CMatrix matTransform;
	int i;

	// transform line to model space
	Invert(matrix, matTransform);
	CVuVector newline[2];
	TransformPoints(newline, 2, matTransform, &line.p0, sizeof(CColLine)/2);

	// If we don't intersect with the bounding box, no chance on the rest
	if(!TestLineBox(*(CColLine*)newline, model.boundingBox))
		return false;

	for(i = 0; i < model.numSpheres; i++){
		if(ignoreSeeThrough && IsSeeThrough(model.spheres[i].surface)) continue;
		if(TestLineSphere(*(CColLine*)newline, model.spheres[i]))
			return true;
	}

	for(i = 0; i < model.numBoxes; i++){
		if(ignoreSeeThrough && IsSeeThrough(model.boxes[i].surface)) continue;
		if(TestLineBox(*(CColLine*)newline, model.boxes[i]))
			return true;
	}

	CalculateTrianglePlanes(&model);
	int lastTest = -1;
	VuTriangle vutri;
	for(i = 0; i < model.numTriangles; i++){
		if(ignoreSeeThrough && IsSeeThrough(model.triangles[i].surface)) continue;

		CColTriangle *tri = &model.triangles[i];
		model.vertices[tri->a].Unpack(vutri.v0);
		model.vertices[tri->b].Unpack(vutri.v1);
		model.vertices[tri->c].Unpack(vutri.v2);
		model.trianglePlanes[i].Unpack(vutri.plane);

		LineToTriangleCollisionCompressed(newline[0], newline[1], vutri);
		lastTest = i;
		break;
	}
#ifdef FIX_BUGS
	// no need to check first again
	i++;
#endif
	for(; i < model.numTriangles; i++){
		if(ignoreSeeThrough && IsSeeThrough(model.triangles[i].surface)) continue;

		CColTriangle *tri = &model.triangles[i];
		model.vertices[tri->a].Unpack(vutri.v0);
		model.vertices[tri->b].Unpack(vutri.v1);
		model.vertices[tri->c].Unpack(vutri.v2);
		model.trianglePlanes[i].Unpack(vutri.plane);

		if(GetVUresult())
			return true;

		LineToTriangleCollisionCompressed(newline[0], newline[1], vutri);
		lastTest = i;

	}
	if(lastTest != -1 && GetVUresult())
		return true;

	return false;
#else
	static CMatrix matTransform;
	int i;

	// transform line to model space
	Invert(matrix, matTransform);
	CColLine newline(matTransform * line.p0, matTransform * line.p1);

	// If we don't intersect with the bounding box, no chance on the rest
	if(!TestLineBox(newline, model.boundingBox))
		return false;

	for(i = 0; i < model.numSpheres; i++){
		if(ignoreSeeThrough && IsSeeThrough(model.spheres[i].surface)) continue;
		if(TestLineSphere(newline, model.spheres[i]))
			return true;
	}

	for(i = 0; i < model.numBoxes; i++){
		if(ignoreSeeThrough && IsSeeThrough(model.boxes[i].surface)) continue;
		if(TestLineBox(newline, model.boxes[i]))
			return true;
	}

	CalculateTrianglePlanes(&model);
	for(i = 0; i < model.numTriangles; i++){
		if(ignoreSeeThrough && IsSeeThrough(model.triangles[i].surface)) continue;
		if(TestLineTriangle(newline, model.vertices, model.triangles[i], model.trianglePlanes[i]))
			return true;
	}

	return false;
#endif
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
	float depth = s1.radius - d;	// sphere overlap
	if(d < 0.0f) d = 0.0f;		// clamp to zero, i.e. if s1's center is inside s2
	// no collision if sphere is not close enough
	if(d*d < mindistsq && d < s1.radius){
		dist.Normalise();
		point.point = s1.center - dist*d;
		point.normal = dist;
#ifndef VU_COLLISION
		point.surfaceA = s1.surface;
		point.pieceA = s1.piece;
		point.surfaceB = s2.surface;
		point.pieceB = s2.piece;
#endif
		point.depth = depth;	
		mindistsq = d*d;		// collision radius
		return true;
	}
	return false;
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
#ifndef VU_COLLISION
			point.surfaceA = sph.surface;
			point.pieceA = sph.piece;
			point.surfaceB = box.surface;
			point.pieceB = box.piece;
#endif

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
#ifndef VU_COLLISION
			point.surfaceA = sph.surface;
			point.pieceA = sph.piece;
			point.surfaceB = box.surface;
			point.pieceB = box.piece;
#endif
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
#ifndef VU_COLLISION
	point.surfaceA = 0;
	point.pieceA = 0;
	point.surfaceB = box.surface;
	point.pieceB = box.piece;
#endif
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
#ifndef VU_COLLISION
	point.surfaceA = 0;
	point.pieceA = 0;
	point.surfaceB = sphere.surface;
	point.pieceB = sphere.piece;
#endif
	mindist = t;
	return true;
}

bool
CCollision::ProcessVerticalLineTriangle(const CColLine &line,
	const CompressedVector *verts, const CColTriangle &tri, const CColTrianglePlane &plane,
	CColPoint &point, float &mindist, CStoredCollPoly *poly)
{
#ifdef VU_COLLISION
	// not used in favour of optimized loops
	bool res = ProcessLineTriangle(line, verts, tri, plane, point, mindist);
	if(res && poly){
		poly->verts[0] = verts[tri.a].Get();
		poly->verts[1] = verts[tri.b].Get();
		poly->verts[2] = verts[tri.c].Get();
		poly->valid = true;
	}
	return res;
#else
	float t;
	CVector normal;

	const CVector &p0 = line.p0;
	const CVector &va = verts[tri.a].Get();
	const CVector &vb = verts[tri.b].Get();
	const CVector &vc = verts[tri.c].Get();

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
	if(t >= mindist) return false;
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
#endif
}

bool
CCollision::IsStoredPolyStillValidVerticalLine(const CVector &pos, float z, CColPoint &point, CStoredCollPoly *poly)
{
#ifdef VU_COLLISION
	if(!poly->valid)
		return false;

	CVuVector p0 = pos;
	CVuVector p1 = pos;
	p1.z = z;

	CVector v01 = poly->verts[1] - poly->verts[0];
	CVector v02 = poly->verts[2] - poly->verts[0];
	CVuVector plane = CrossProduct(v02, v01);
	plane.Normalise();
	plane.w = DotProduct(plane, poly->verts[0]);

	LineToTriangleCollision(p0, p1, poly->verts[0], poly->verts[1], poly->verts[2], plane);

	CVuVector pnt;
	float dist;
	if(!GetVUresult(pnt, plane, dist))
#ifdef FIX_BUGS
		// perhaps not needed but be safe
		return poly->valid = false;
#else
		return false;
#endif
	point.point = pnt;
	return true;
#else
	float t;

	if(!poly->valid)
		return false;

	// maybe inlined?
	CColTrianglePlane plane;
	plane.Set(poly->verts[0], poly->verts[1], poly->verts[2]);

	const CVector &va = poly->verts[0];
	const CVector &vb = poly->verts[1];
	const CVector &vc = poly->verts[2];
	CVector p0 = pos;
	CVector p1(pos.x, pos.y, z);

	// The rest is pretty much CCollision::ProcessLineTriangle

	// if points are on the same side, no collision
	if(plane.CalcPoint(p0) * plane.CalcPoint(p1) > 0.0f)
		return poly->valid = false;

	// intersection parameter on line
	CVector normal;
	plane.GetNormal(normal);
	t = -plane.CalcPoint(p0) / DotProduct(p1 - p0, normal);
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
#endif
}

bool
CCollision::ProcessLineTriangle(const CColLine &line,
	const CompressedVector *verts, const CColTriangle &tri, const CColTrianglePlane &plane,
	CColPoint &point, float &mindist)
{
#ifdef VU_COLLISION
	// not used in favour of optimized loops
	VuTriangle vutri;
	verts[tri.a].Unpack(vutri.v0);
	verts[tri.b].Unpack(vutri.v1);
	verts[tri.c].Unpack(vutri.v2);
	plane.Unpack(vutri.plane);

	LineToTriangleCollisionCompressed(*(CVuVector*)&line.p0, *(CVuVector*)&line.p1, vutri);

	CVuVector pnt, normal;
	float dist;
	if(GetVUresult(pnt, normal, dist)){
		if(dist < mindist){
			point.point = pnt;
			point.normal = normal;
			mindist = dist;
			return true;
		}
	}
	return false;
#else
	float t;
	CVector normal;
	plane.GetNormal(normal);

	// if points are on the same side, no collision
	if(plane.CalcPoint(line.p0) * plane.CalcPoint(line.p1) > 0.0f)
		return false;

	float p0dist = DotProduct(line.p1 - line.p0, normal);

#ifdef FIX_BUGS
	// line lines in the plane, assume no collision
	if (p0dist == 0.0f)
		return false;
#endif

	// intersection parameter on line
	t = -plane.CalcPoint(line.p0) / p0dist;

	// early out if we're beyond the mindist
	if(t >= mindist)
		return false;
	// find point of intersection
	CVector p = line.p0 + (line.p1-line.p0)*t;

	const CVector &va = verts[tri.a].Get();
	const CVector &vb = verts[tri.b].Get();
	const CVector &vc = verts[tri.c].Get();
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
	if(t >= mindist) return false;
	point.point = p;
	point.normal = normal;
	point.surfaceA = 0;
	point.pieceA = 0;
	point.surfaceB = tri.surface;
	point.pieceB = 0;
	mindist = t;
	return true;
#endif
}

bool
CCollision::ProcessSphereTriangle(const CColSphere &sphere,
	const CompressedVector *verts, const CColTriangle &tri, const CColTrianglePlane &plane,
	CColPoint &point, float &mindistsq)
{
#ifdef VU_COLLISION
	// not used in favour of optimized loops
	VuTriangle vutri;
	verts[tri.a].Unpack(vutri.v0);
	verts[tri.b].Unpack(vutri.v1);
	verts[tri.c].Unpack(vutri.v2);
	plane.Unpack(vutri.plane);

	SphereToTriangleCollisionCompressed(*(CVuVector*)&sphere, vutri);

	CVuVector pnt, normal;
	float dist;
	if(GetVUresult(pnt, normal, dist) && dist*dist < mindistsq){
		float depth = sphere.radius - dist;
		if(depth > point.depth){
			point.point = pnt;
			point.normal = normal;
			point.depth = depth;
			mindistsq = dist*dist;
			return true;
		}
	}
	return false;
#else
	// If sphere and plane don't intersect, no collision
	float planedist = plane.CalcPoint(sphere.center);
	float distsq = planedist*planedist;
	if(Abs(planedist) > sphere.radius || distsq > mindistsq)
		return false;

	const CVector &va = verts[tri.a].Get();
	const CVector &vb = verts[tri.b].Get();
	const CVector &vc = verts[tri.c].Get();

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
	switch(testcase){
	case 1:
		// closest to a vertex
		if(insideAB) p = vc;
		else if(insideAC) p = vb;
		else if(insideBC) p = va;
		else assert(0);
		dist = (sphere.center - p).Magnitude();
		break;
	case 2:
		// closest to an edge
		// looks like original game as DistToLine manually inlined
		if(!insideAB) dist = DistToLine(&va, &vb, &sphere.center, p);
		else if(!insideAC) dist = DistToLine(&va, &vc, &sphere.center, p);
		else if(!insideBC) dist = DistToLine(&vb, &vc, &sphere.center, p);
		else assert(0);
		break;
	case 3:
		// center is in triangle
		dist = Abs(planedist);
		p = sphere.center - normal*planedist;
		break;
	default:
		assert(0);
	}

	if(dist >= sphere.radius || dist*dist >= mindistsq)
		return false;

	point.point = p;
	point.normal = sphere.center - p;
	point.normal.Normalise();
#ifndef VU_COLLISION
	point.surfaceA = sphere.surface;
	point.pieceA = sphere.piece;
	point.surfaceB = tri.surface;
	point.pieceB = 0;
#endif
	point.depth = sphere.radius - dist;
	mindistsq = dist*dist;
	return true;
#endif
}

bool
CCollision::ProcessLineOfSight(const CColLine &line,
	const CMatrix &matrix, CColModel &model,
	CColPoint &point, float &mindist, bool ignoreSeeThrough)
{
#ifdef VU_COLLISION
	CMatrix matTransform;
	int i;

	// transform line to model space
	Invert(matrix, matTransform);
	CVuVector newline[2];
	TransformPoints(newline, 2, matTransform, &line.p0, sizeof(CColLine)/2);

	if(mindist < 1.0f)
		newline[1] = newline[0] + (newline[1] - newline[0])*mindist;

	// If we don't intersect with the bounding box, no chance on the rest
	if(!TestLineBox(*(CColLine*)newline, model.boundingBox))
		return false;

	float coldist = 1.0f;
	for(i = 0; i < model.numSpheres; i++){
		if(ignoreSeeThrough && IsSeeThrough(model.spheres[i].surface)) continue;
		if(ProcessLineSphere(*(CColLine*)newline, model.spheres[i], point, coldist))
			point.Set(0, 0, model.spheres[i].surface, model.spheres[i].piece);
	}

	for(i = 0; i < model.numBoxes; i++){
		if(ignoreSeeThrough && IsSeeThrough(model.boxes[i].surface)) continue;
		if(ProcessLineBox(*(CColLine*)newline, model.boxes[i], point, coldist))
			point.Set(0, 0, model.boxes[i].surface, model.boxes[i].piece);
	}

	CalculateTrianglePlanes(&model);
	VuTriangle vutri;
	CColTriangle *lasttri = nil;
	for(i = 0; i < model.numTriangles; i++){
		if(ignoreSeeThrough && IsSeeThrough(model.triangles[i].surface)) continue;

		CColTriangle *tri = &model.triangles[i];
		model.vertices[tri->a].Unpack(vutri.v0);
		model.vertices[tri->b].Unpack(vutri.v1);
		model.vertices[tri->c].Unpack(vutri.v2);
		model.trianglePlanes[i].Unpack(vutri.plane);

		LineToTriangleCollisionCompressed(newline[0], newline[1], vutri);
		lasttri = tri;
		break;
	}
#ifdef FIX_BUGS
	// no need to check first again
	i++;
#endif
	CVuVector pnt, normal;
	float dist;
	for(; i < model.numTriangles; i++){
		if(ignoreSeeThrough && IsSeeThrough(model.triangles[i].surface)) continue;

		CColTriangle *tri = &model.triangles[i];
		model.vertices[tri->a].Unpack(vutri.v0);
		model.vertices[tri->b].Unpack(vutri.v1);
		model.vertices[tri->c].Unpack(vutri.v2);
		model.trianglePlanes[i].Unpack(vutri.plane);

		if(GetVUresult(pnt, normal, dist))
			if(dist < coldist){
				point.point = pnt;
				point.normal = normal;
				point.Set(0, 0, lasttri->surface, 0);
				coldist = dist;
			}

		LineToTriangleCollisionCompressed(newline[0], newline[1], vutri);
		lasttri = tri;
	}
	if(lasttri && GetVUresult(pnt, normal, dist))
		if(dist < coldist){
			point.point = pnt;
			point.normal = normal;
			point.Set(0, 0, lasttri->surface, 0);
			coldist = dist;
		}


	if(coldist < 1.0f){
		point.point = matrix * point.point;
		point.normal = Multiply3x3(matrix, point.normal);
		mindist *= coldist;
		return true;
	}
	return false;
#else
	static CMatrix matTransform;
	int i;

	// transform line to model space
	Invert(matrix, matTransform);
	CColLine newline(matTransform * line.p0, matTransform * line.p1);

	// If we don't intersect with the bounding box, no chance on the rest
	if(!TestLineBox(newline, model.boundingBox))
		return false;

	float coldist = mindist;
	for(i = 0; i < model.numSpheres; i++){
		if(ignoreSeeThrough && IsSeeThrough(model.spheres[i].surface)) continue;
		ProcessLineSphere(newline, model.spheres[i], point, coldist);
	}

	for(i = 0; i < model.numBoxes; i++){
		if(ignoreSeeThrough && IsSeeThrough(model.boxes[i].surface)) continue;
		ProcessLineBox(newline, model.boxes[i], point, coldist);
	}

	CalculateTrianglePlanes(&model);
	for(i = 0; i < model.numTriangles; i++){
		if(ignoreSeeThrough && IsSeeThrough(model.triangles[i].surface)) continue;
		ProcessLineTriangle(newline, model.vertices, model.triangles[i], model.trianglePlanes[i], point, coldist);
	}

	if(coldist < mindist){
		point.point = matrix * point.point;
		point.normal = Multiply3x3(matrix, point.normal);
		mindist = coldist;
		return true;
	}
	return false;
#endif
}

bool
CCollision::ProcessVerticalLine(const CColLine &line,
	const CMatrix &matrix, CColModel &model,
	CColPoint &point, float &mindist, bool ignoreSeeThrough, CStoredCollPoly *poly)
{
#ifdef VU_COLLISION
	static CStoredCollPoly TempStoredPoly;
	CMatrix matTransform;
	int i;

	// transform line to model space
	Invert(matrix, matTransform);
	CVuVector newline[2];
	TransformPoints(newline, 2, matTransform, &line.p0, sizeof(CColLine)/2);

	if(mindist < 1.0f)
		newline[1] = newline[0] + (newline[1] - newline[0])*mindist;

	if(!TestLineBox(*(CColLine*)newline, model.boundingBox))
		return false;

	float coldist = 1.0f;
	for(i = 0; i < model.numSpheres; i++){
		if(ignoreSeeThrough && IsSeeThrough(model.spheres[i].surface)) continue;
		if(ProcessLineSphere(*(CColLine*)newline, model.spheres[i], point, coldist))
			point.Set(0, 0, model.spheres[i].surface, model.spheres[i].piece);
	}

	for(i = 0; i < model.numBoxes; i++){
		if(ignoreSeeThrough && IsSeeThrough(model.boxes[i].surface)) continue;
		if(ProcessLineBox(*(CColLine*)newline, model.boxes[i], point, coldist))
			point.Set(0, 0, model.boxes[i].surface, model.boxes[i].piece);
	}

	CalculateTrianglePlanes(&model);
	TempStoredPoly.valid = false;
	if(model.numTriangles){
		bool registeredCol;
		CColTriangle *lasttri = nil;
		VuTriangle vutri;
		for(i = 0; i < model.numTriangles; i++){
			if(ignoreSeeThrough && IsSeeThrough(model.triangles[i].surface)) continue;

			CColTriangle *tri = &model.triangles[i];
			model.vertices[tri->a].Unpack(vutri.v0);
			model.vertices[tri->b].Unpack(vutri.v1);
			model.vertices[tri->c].Unpack(vutri.v2);
			model.trianglePlanes[i].Unpack(vutri.plane);

			LineToTriangleCollisionCompressed(newline[0], newline[1], vutri);
			lasttri = tri;
			break;
		}
#ifdef FIX_BUGS
		// no need to check first again
		i++;
#endif
		CVuVector pnt, normal;
		float dist;
		for(; i < model.numTriangles; i++){
			if(ignoreSeeThrough && IsSeeThrough(model.triangles[i].surface)) continue;

			CColTriangle *tri = &model.triangles[i];
			model.vertices[tri->a].Unpack(vutri.v0);
			model.vertices[tri->b].Unpack(vutri.v1);
			model.vertices[tri->c].Unpack(vutri.v2);
			model.trianglePlanes[i].Unpack(vutri.plane);

			if(GetVUresult(pnt, normal, dist)){
				if(dist < coldist){
					point.point = pnt;
					point.normal = normal;
					point.Set(0, 0, lasttri->surface, 0);
					coldist = dist;
					registeredCol = true;
				}else
					registeredCol = false;
			}else
				registeredCol = false;

			if(registeredCol){
				TempStoredPoly.verts[0] = model.vertices[lasttri->a].Get();
				TempStoredPoly.verts[1] = model.vertices[lasttri->b].Get();
				TempStoredPoly.verts[2] = model.vertices[lasttri->c].Get();
				TempStoredPoly.valid = true;
			}

			LineToTriangleCollisionCompressed(newline[0], newline[1], vutri);
			lasttri = tri;
		}
		if(lasttri && GetVUresult(pnt, normal, dist)){
			if(dist < coldist){
				point.point = pnt;
				point.normal = normal;
				point.Set(0, 0, lasttri->surface, 0);
				coldist = dist;
				registeredCol = true;
			}else
				registeredCol = false;
		}else
			registeredCol = false;

		if(registeredCol){
			TempStoredPoly.verts[0] = model.vertices[lasttri->a].Get();
			TempStoredPoly.verts[1] = model.vertices[lasttri->b].Get();
			TempStoredPoly.verts[2] = model.vertices[lasttri->c].Get();
			TempStoredPoly.valid = true;
		}
	}

	if(coldist < 1.0f){
		point.point = matrix * point.point;
		point.normal = Multiply3x3(matrix, point.normal);
		if(TempStoredPoly.valid && poly){
			*poly = TempStoredPoly;
			poly->verts[0] = matrix * CVector(poly->verts[0]);
			poly->verts[1] = matrix * CVector(poly->verts[1]);
			poly->verts[2] = matrix * CVector(poly->verts[2]);
		}
		mindist *= coldist;
		return true;
	}
	return false;
#else
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
	for(i = 0; i < model.numSpheres; i++){
		if(ignoreSeeThrough && IsSeeThrough(model.spheres[i].surface)) continue;
		ProcessLineSphere(newline, model.spheres[i], point, coldist);
	}

	for(i = 0; i < model.numBoxes; i++){
		if(ignoreSeeThrough && IsSeeThrough(model.boxes[i].surface)) continue;
		ProcessLineBox(newline, model.boxes[i], point, coldist);
	}

	CalculateTrianglePlanes(&model);
	TempStoredPoly.valid = false;
	for(i = 0; i < model.numTriangles; i++){
		if(ignoreSeeThrough && IsSeeThrough(model.triangles[i].surface)) continue;
		ProcessVerticalLineTriangle(newline, model.vertices, model.triangles[i], model.trianglePlanes[i], point, coldist, &TempStoredPoly);
	}

	if(coldist < mindist){
		point.point = matrix * point.point;
		point.normal = Multiply3x3(matrix, point.normal);
		if(TempStoredPoly.valid && poly){
			*poly = TempStoredPoly;
			poly->verts[0] = matrix * poly->verts[0];
			poly->verts[1] = matrix * poly->verts[1];
			poly->verts[2] = matrix * poly->verts[2];
		}
		mindist = coldist;
		return true;
	}
	return false;
#endif
}

enum {
	MAXNUMSPHERES = 128,
	MAXNUMBOXES = 32,
	MAXNUMLINES = 16,
	MAXNUMTRIS = 600
};

#ifdef VU_COLLISION
#ifdef GTA_PS2
#define SPR(off) ((uint8*)(0x70000000 + (off)))
#else
static uint8 fakeSPR[16*1024];
#define SPR(off) ((uint8*)(fakeSPR + (off)))
#endif
#endif

// This checks model A's spheres and lines against model B's spheres, boxes and triangles.
// Returns the number of A's spheres that collide.
// Returned ColPoints are in world space.
// NB: only vehicles can have col models with lines, exactly 4, one for each wheel
int32
CCollision::ProcessColModels(const CMatrix &matrixA, CColModel &modelA,
	const CMatrix &matrixB, CColModel &modelB,
	CColPoint *spherepoints, CColPoint *linepoints, float *linedists)
{
#ifdef VU_COLLISION
	CVuVector *aSpheresA = (CVuVector*)SPR(0x0000);
	CVuVector *aSpheresB = (CVuVector*)SPR(0x0800);
	CVuVector *aLinesA = (CVuVector*)SPR(0x1000);
	int32 *aSphereIndicesA = (int32*)SPR(0x1200);
	int32 *aSphereIndicesB = (int32*)SPR(0x1400);
	int32 *aBoxIndicesB = (int32*)SPR(0x1600);
	int32 *aTriangleIndicesB = (int32*)SPR(0x1680);
	bool *aCollided = (bool*)SPR(0x1FE0);
	CMatrix &matAB = *(CMatrix*)SPR(0x1FF0);
	CMatrix &matBA = *(CMatrix*)SPR(0x2040);
	int i, j, k;

	// From model A space to model B space
	Invert(matrixB, matAB);
	matAB *= matrixA;

	CVuVector bsphereAB;	// bounding sphere of A in B space
	TransformPoint(bsphereAB, matAB, modelA.boundingSphere.center);	// inlined
	bsphereAB.w = modelA.boundingSphere.radius;
	if(!TestSphereBox(*(CColSphere*)&bsphereAB, modelB.boundingBox))
		return 0;

	// transform modelA's spheres and lines to B space
	TransformPoints(aSpheresA, modelA.numSpheres, matAB, &modelA.spheres->center, sizeof(CColSphere));
	for(i = 0; i < modelA.numSpheres; i++)
		aSpheresA[i].w = modelA.spheres[i].radius;
	TransformPoints(aLinesA, modelA.numLines*2, matAB, &modelA.lines->p0, sizeof(CColLine)/2);

	// Test them against model B's bounding volumes
	int numSpheresA = 0;
	for(i = 0; i < modelA.numSpheres; i++)
		if(TestSphereBox(*(CColSphere*)&aSpheresA[i], modelB.boundingBox))
			aSphereIndicesA[numSpheresA++] = i;
	// No collision
	if(numSpheresA == 0 && modelA.numLines == 0)
		return 0;


	// B to A space
	Invert(matrixA, matBA);
	matBA *= matrixB;

	// transform modelB's spheres to A space
	TransformPoints(aSpheresB, modelB.numSpheres, matBA, &modelB.spheres->center, sizeof(CColSphere));
	for(i = 0; i < modelB.numSpheres; i++)
		aSpheresB[i].w = modelB.spheres[i].radius;

	// Check model B against A's bounding volumes
	int numSpheresB = 0;
	int numBoxesB = 0;
	int numTrianglesB = 0;
	for(i = 0; i < modelB.numSpheres; i++)
		if(TestSphereBox(*(CColSphere*)&aSpheresB[i], modelA.boundingBox))
			aSphereIndicesB[numSpheresB++] = i;
	for(i = 0; i < modelB.numBoxes; i++)
		if(TestSphereBox(*(CColSphere*)&bsphereAB, modelB.boxes[i]))
			aBoxIndicesB[numBoxesB++] = i;
	CalculateTrianglePlanes(&modelB);
	if(modelB.numTriangles){
		VuTriangle vutri;
		// process the first triangle
		CColTriangle *tri = &modelB.triangles[0];
		modelB.vertices[tri->a].Unpack(vutri.v0);
		modelB.vertices[tri->b].Unpack(vutri.v1);
		modelB.vertices[tri->c].Unpack(vutri.v2);
		modelB.trianglePlanes[0].Unpack(vutri.plane);

		SphereToTriangleCollisionCompressed(bsphereAB, vutri);

		for(i = 1; i < modelB.numTriangles; i++){
			// set up the next triangle while VU0 is running
			tri = &modelB.triangles[i];
			modelB.vertices[tri->a].Unpack(vutri.v0);
			modelB.vertices[tri->b].Unpack(vutri.v1);
			modelB.vertices[tri->c].Unpack(vutri.v2);
			modelB.trianglePlanes[i].Unpack(vutri.plane);

			// check previous result
			if(GetVUresult())
				aTriangleIndicesB[numTrianglesB++] = i-1;

			// kick off this one
			SphereToTriangleCollisionCompressed(bsphereAB, vutri);
		}

		// check last result
		if(GetVUresult())
			aTriangleIndicesB[numTrianglesB++] = i-1;
	}
	// No collision
	if(numSpheresB == 0 && numBoxesB == 0 && numTrianglesB == 0)
		return 0;

	// We now have the collision volumes in A and B that are worth processing.

	// Process A's spheres against B's collision volumes
	int numCollisions = 0;
	spherepoints[numCollisions].depth = -1.0f;
	for(i = 0; i < numSpheresA; i++){
		float coldist = 1.0e24f;
		bool hasCollided = false;
		CColSphere *sphA = &modelA.spheres[aSphereIndicesA[i]];
		CVuVector *vusphA = &aSpheresA[aSphereIndicesA[i]];

		for(j = 0; j < numSpheresB; j++)
			// This actually looks like something was inlined here
			if(ProcessSphereSphere(*(CColSphere*)vusphA, modelB.spheres[aSphereIndicesB[j]],
					spherepoints[numCollisions], coldist)){
				spherepoints[numCollisions].Set(
					sphA->surface, sphA->piece,
					modelB.spheres[aSphereIndicesB[j]].surface, modelB.spheres[aSphereIndicesB[j]].piece);
				hasCollided = true;
			}
		for(j = 0; j < numBoxesB; j++)
			if(ProcessSphereBox(*(CColSphere*)vusphA, modelB.boxes[aBoxIndicesB[j]],
					spherepoints[numCollisions], coldist)){
				spherepoints[numCollisions].Set(
					sphA->surface, sphA->piece,
					modelB.boxes[aBoxIndicesB[j]].surface, modelB.boxes[aBoxIndicesB[j]].piece);
				hasCollided = true;
			}
		if(numTrianglesB){
			CVuVector point, normal;
			float depth;
			bool registeredCol;
			CColTriangle *lasttri;

			VuTriangle vutri;
			// process the first triangle
			k = aTriangleIndicesB[0];
			CColTriangle *tri = &modelB.triangles[k];
			modelB.vertices[tri->a].Unpack(vutri.v0);
			modelB.vertices[tri->b].Unpack(vutri.v1);
			modelB.vertices[tri->c].Unpack(vutri.v2);
			modelB.trianglePlanes[k].Unpack(vutri.plane);

			SphereToTriangleCollisionCompressed(*vusphA, vutri);
			lasttri = tri;

			for(j = 1; j < numTrianglesB; j++){
				k = aTriangleIndicesB[j];
				// set up the next triangle while VU0 is running
				tri = &modelB.triangles[k];
				modelB.vertices[tri->a].Unpack(vutri.v0);
				modelB.vertices[tri->b].Unpack(vutri.v1);
				modelB.vertices[tri->c].Unpack(vutri.v2);
				modelB.trianglePlanes[k].Unpack(vutri.plane);

				// check previous result
				// TODO: this looks inlined but spherepoints[numCollisions] does not...
				if(GetVUresult(point, normal, depth)){
					depth = sphA->radius - depth;
					if(depth > spherepoints[numCollisions].depth){
						spherepoints[numCollisions].point = point;
						spherepoints[numCollisions].normal = normal;
						spherepoints[numCollisions].Set(depth,
							sphA->surface, sphA->piece, lasttri->surface, 0);
						registeredCol = true;
					}else
						registeredCol = false;
				}else
					registeredCol = false;

				if(registeredCol)
					hasCollided = true;

				// kick off this one
				SphereToTriangleCollisionCompressed(*vusphA, vutri);
				lasttri = tri;
			}

			// check last result
			// TODO: this looks inlined but spherepoints[numCollisions] does not...
			if(GetVUresult(point, normal, depth)){
				depth = sphA->radius - depth;
				if(depth > spherepoints[numCollisions].depth){
					spherepoints[numCollisions].point = point;
					spherepoints[numCollisions].normal = normal;
					spherepoints[numCollisions].Set(depth,
						sphA->surface, sphA->piece, lasttri->surface, 0);
					registeredCol = true;
				}else
					registeredCol = false;
			}else
				registeredCol = false;

			if(registeredCol)
				hasCollided = true;
		}

		if(hasCollided){
			numCollisions++;
			if(numCollisions == MAX_COLLISION_POINTS)
				break;
			spherepoints[numCollisions].depth = -1.0f;
		}
	}
	for(i = 0; i < numCollisions; i++){
		// TODO: both VU0 macros
		spherepoints[i].point = matrixB * spherepoints[i].point;
		spherepoints[i].normal = Multiply3x3(matrixB, spherepoints[i].normal);
	}

	// And the same thing for the lines in A
	for(i = 0; i < modelA.numLines; i++){
		aCollided[i] = false;
		CVuVector *lineA = &aLinesA[i*2];

		for(j = 0; j < numSpheresB; j++)
			if(ProcessLineSphere(*(CColLine*)lineA, modelB.spheres[aSphereIndicesB[j]],
					linepoints[i], linedists[i])){
				linepoints[i].Set(0, 0,
#ifdef FIX_BUGS
					modelB.spheres[aSphereIndicesB[j]].surface, modelB.spheres[aSphereIndicesB[j]].piece);
#else
					modelB.spheres[j].surface, modelB.spheres[j].piece);
#endif
				aCollided[i] = true;
			}
		for(j = 0; j < numBoxesB; j++)
			if(ProcessLineBox(*(CColLine*)lineA, modelB.boxes[aBoxIndicesB[j]],
					linepoints[i], linedists[i])){
				linepoints[i].Set(0, 0,
					modelB.boxes[aBoxIndicesB[j]].surface, modelB.boxes[aBoxIndicesB[j]].piece);
				aCollided[i] = true;
			}
		if(numTrianglesB){
			CVuVector point, normal;
			float dist;
			bool registeredCol;
			CColTriangle *lasttri;

			VuTriangle vutri;
			// process the first triangle
			k = aTriangleIndicesB[0];
			CColTriangle *tri = &modelB.triangles[k];
			modelB.vertices[tri->a].Unpack(vutri.v0);
			modelB.vertices[tri->b].Unpack(vutri.v1);
			modelB.vertices[tri->c].Unpack(vutri.v2);
			modelB.trianglePlanes[k].Unpack(vutri.plane);

			LineToTriangleCollisionCompressed(lineA[0], lineA[1], vutri);
			lasttri = tri;

			for(j = 1; j < numTrianglesB; j++){
				k = aTriangleIndicesB[j];
				// set up the next triangle while VU0 is running
				CColTriangle *tri = &modelB.triangles[k];
				modelB.vertices[tri->a].Unpack(vutri.v0);
				modelB.vertices[tri->b].Unpack(vutri.v1);
				modelB.vertices[tri->c].Unpack(vutri.v2);
				modelB.trianglePlanes[k].Unpack(vutri.plane);

				// check previous result
				// TODO: this again somewhat looks inlined
				if(GetVUresult(point, normal, dist)){
					if(dist < linedists[i]){
						linepoints[i].point = point;
						linepoints[i].normal = normal;
						linedists[i] = dist;
						linepoints[i].Set(0, 0, lasttri->surface, 0);
						registeredCol = true;
					}else
						registeredCol = false;
				}else
					registeredCol = false;

				if(registeredCol)
					aCollided[i] = true;

				// kick of this one
				LineToTriangleCollisionCompressed(lineA[0], lineA[1], vutri);
				lasttri = tri;
			}

			// check last result
			if(GetVUresult(point, normal, dist)){
				if(dist < linedists[i]){
					linepoints[i].point = point;
					linepoints[i].normal = normal;
					linedists[i] = dist;
					linepoints[i].Set(0, 0, lasttri->surface, 0);
					registeredCol = true;
				}else
					registeredCol = false;
			}else
				registeredCol = false;

			if(registeredCol)
				aCollided[i] = true;
		}

		if(aCollided[i]){
			// TODO: both VU0 macros
			linepoints[i].point = matrixB * linepoints[i].point;
			linepoints[i].normal = Multiply3x3(matrixB, linepoints[i].normal);
		}
	}

	return numCollisions;	// sphere collisions
#else
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
	matAB = Invert(matrixB, matAB);
	matAB *= matrixA;

	CColSphere bsphereAB;	// bounding sphere of A in B space
	bsphereAB.radius = modelA.boundingSphere.radius;
	bsphereAB.center = matAB * modelA.boundingSphere.center;
	if(!TestSphereBox(bsphereAB, modelB.boundingBox))
		return 0;
	// B to A space
	matBA = Invert(matrixA, matBA);
	matBA *= matrixB;

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
		s.radius = modelB.spheres[i].radius;
		s.center = matBA * modelB.spheres[i].center;
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
#endif
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
	if(dot <= 0.0f) return (*point - *l0).Magnitude();
	if(dot >= lensq) return (*point - *l1).Magnitude();
	// distance to line
	float distSqr = (*point - *l0).MagnitudeSqr() - dot * dot / lensq;
	if(distSqr <= 0.f) return 0.f;
	return Sqrt(distSqr);
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
