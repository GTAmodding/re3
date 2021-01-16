#pragma once

#include "Game.h"
#include "Lists.h"
#include "PlayerInfo.h"
#include "Collision.h"

/* Sectors span from -2000 to 2000 in x and y.
 * With 100x100 sectors, each is 40x40 units. */

#define SECTOR_SIZE_X (40.0f)
#define SECTOR_SIZE_Y (40.0f)

#define NUMSECTORS_X (100)
#define NUMSECTORS_Y (100)

#define WORLD_SIZE_X (NUMSECTORS_X * SECTOR_SIZE_X)
#define WORLD_SIZE_Y (NUMSECTORS_Y * SECTOR_SIZE_Y)

#define WORLD_MIN_X (-2000.0f)
#define WORLD_MIN_Y (-2000.0f)

#define WORLD_MAX_X (WORLD_MIN_X + WORLD_SIZE_X)
#define WORLD_MAX_Y (WORLD_MIN_Y + WORLD_SIZE_Y)

#define MAP_Z_LOW_LIMIT -100.0f

enum
{
	ENTITYLIST_BUILDINGS,
	ENTITYLIST_BUILDINGS_OVERLAP,
	ENTITYLIST_OBJECTS,
	ENTITYLIST_OBJECTS_OVERLAP,
	ENTITYLIST_VEHICLES,
	ENTITYLIST_VEHICLES_OVERLAP,
	ENTITYLIST_PEDS,
	ENTITYLIST_PEDS_OVERLAP,
	ENTITYLIST_DUMMIES,
	ENTITYLIST_DUMMIES_OVERLAP,

	NUMSECTORENTITYLISTS
};

class CSector
{
public:
	CPtrList m_lists[NUMSECTORENTITYLISTS];
};

VALIDATE_SIZE(CSector, 0x28);

class CWorld
{
	static CPtrList ms_bigBuildingsList[NUM_LEVELS];
	static CPtrList ms_listMovingEntityPtrs;
	static CSector ms_aSectors[NUMSECTORS_Y][NUMSECTORS_X];
	static uint16 ms_nCurrentScanCode;

public:
	static uint8 PlayerInFocus;
	static CPlayerInfo Players[NUMPLAYERS];
	static CEntity *pIgnoreEntity;
	static bool bIncludeDeadPeds;
	static bool bNoMoreCollisionTorque;
	static bool bSecondShift;
	static bool bForceProcessControl;
	static bool bProcessCutsceneOnly;
	static bool bDoingCarCollisions;
	static bool bIncludeCarTyres;

	static void Remove(CEntity *entity);
	static void Add(CEntity *entity);

	static CSector *GetSector(int x, int y) { return &ms_aSectors[y][x]; }
	static CPtrList &GetBigBuildingList(eLevelName i) { return ms_bigBuildingsList[i]; }
	static CPtrList &GetMovingEntityList(void) { return ms_listMovingEntityPtrs; }
	static uint16 GetCurrentScanCode(void) { return ms_nCurrentScanCode; }
	static void AdvanceCurrentScanCode(void){
		if(++CWorld::ms_nCurrentScanCode == 0){
			CWorld::ClearScanCodes();
			CWorld::ms_nCurrentScanCode = 1;
		}
	}
	static void ClearScanCodes(void);
	static void ClearExcitingStuffFromArea(const CVector &pos, float radius, bool bRemoveProjectilesAndTidyUpShadows);

	static bool CameraToIgnoreThisObject(CEntity *ent);

	static bool ProcessLineOfSight(const CVector &point1, const CVector &point2, CColPoint &point, CEntity *&entity, bool checkBuildings, bool checkVehicles, bool checkPeds, bool checkObjects, bool checkDummies, bool ignoreSeeThrough, bool ignoreSomeObjects = false);
	static bool ProcessLineOfSightSector(CSector &sector, const CColLine &line, CColPoint &point, float &dist, CEntity *&entity, bool checkBuildings, bool checkVehicles, bool checkPeds, bool checkObjects, bool checkDummies, bool ignoreSeeThrough, bool ignoreSomeObjects = false);
	static bool ProcessLineOfSightSectorList(CPtrList &list, const CColLine &line, CColPoint &point, float &dist, CEntity *&entity, bool ignoreSeeThrough, bool ignoreSomeObjects = false);
	static bool ProcessVerticalLine(const CVector &point1, float z2, CColPoint &point, CEntity *&entity, bool checkBuildings, bool checkVehicles, bool checkPeds, bool checkObjects, bool checkDummies, bool ignoreSeeThrough, CStoredCollPoly *poly);
	static bool ProcessVerticalLineSector(CSector &sector, const CColLine &line, CColPoint &point, CEntity *&entity, bool checkBuildings, bool checkVehicles, bool checkPeds, bool checkObjects, bool checkDummies, bool ignoreSeeThrough, CStoredCollPoly *poly);
	static bool ProcessVerticalLineSectorList(CPtrList &list, const CColLine &line, CColPoint &point, float &dist, CEntity *&entity, bool ignoreSeeThrough, CStoredCollPoly *poly);
	static bool GetIsLineOfSightClear(const CVector &point1, const CVector &point2, bool checkBuildings, bool checkVehicles, bool checkPeds, bool checkObjects, bool checkDummies, bool ignoreSeeThrough, bool ignoreSomeObjects = false);
	static bool GetIsLineOfSightSectorClear(CSector &sector, const CColLine &line, bool checkBuildings, bool checkVehicles, bool checkPeds, bool checkObjects, bool checkDummies, bool ignoreSeeThrough, bool ignoreSomeObjects = false);
	static bool GetIsLineOfSightSectorListClear(CPtrList &list, const CColLine &line, bool ignoreSeeThrough, bool ignoreSomeObjects = false);
	
	static CEntity *TestSphereAgainstWorld(CVector centre, float radius, CEntity *entityToIgnore, bool checkBuildings, bool checkVehicles, bool checkPeds, bool checkObjects, bool checkDummies, bool ignoreSomeObjects);
	static CEntity *TestSphereAgainstSectorList(CPtrList&, CVector, float, CEntity*, bool);
	static void FindObjectsInRangeSectorList(CPtrList &list, Const CVector &centre, float radius, bool ignoreZ, int16 *numObjects, int16 lastObject, CEntity **objects);
	static void FindObjectsInRange(Const CVector &centre, float radius, bool ignoreZ, int16 *numObjects, int16 lastObject, CEntity **objects, bool checkBuildings, bool checkVehicles, bool checkPeds, bool checkObjects, bool checkDummies);
	static void FindObjectsOfTypeInRangeSectorList(uint32 modelId, CPtrList& list, const CVector& position, float radius, bool bCheck2DOnly, int16* nEntitiesFound, int16 maxEntitiesToFind, CEntity** aEntities);
	static void FindObjectsOfTypeInRange(uint32 modelId, const CVector& position, float radius, bool bCheck2DOnly, int16* nEntitiesFound, int16 maxEntitiesToFind, CEntity** aEntities, bool bBuildings, bool bVehicles, bool bPeds, bool bObjects, bool bDummies);
	static float FindGroundZForCoord(float x, float y);
	static float FindGroundZFor3DCoord(float x, float y, float z, bool *found);
	static float FindRoofZFor3DCoord(float x, float y, float z, bool *found);
	static void RemoveReferencesToDeletedObject(CEntity*);
	static void FindObjectsKindaColliding(const CVector& position, float radius, bool bCheck2DOnly, int16* nCollidingEntities, int16 maxEntitiesToFind, CEntity** aEntities, bool bBuildings, bool bVehicles, bool bPeds, bool bObjects, bool bDummies);
	static void FindObjectsKindaCollidingSectorList(CPtrList& list, const CVector& position, float radius, bool bCheck2DOnly, int16* nCollidingEntities, int16 maxEntitiesToFind, CEntity** aEntities);
	static void FindObjectsIntersectingCube(const CVector& vecStartPos, const CVector& vecEndPos, int16* nIntersecting, int16 maxEntitiesToFind, CEntity** aEntities, bool bBuildings, bool bVehicles, bool bPeds, bool bObjects, bool bDummies);
	static void FindObjectsIntersectingCubeSectorList(CPtrList& list, const CVector& vecStartPos, const CVector& vecEndPos, int16* nIntersecting, int16 maxEntitiesToFind, CEntity** aEntities);
	static void FindObjectsIntersectingAngledCollisionBox(const CColBox &, const CMatrix &, const CVector &, float, float, float, float, int16*, int16, CEntity **, bool, bool, bool, bool, bool);
	static void FindObjectsIntersectingAngledCollisionBoxSectorList(CPtrList& list, const CColBox& boundingBox, const CMatrix& matrix, const CVector& position, int16* nEntitiesFound, int16 maxEntitiesToFind, CEntity** aEntities);
	static void FindMissionEntitiesIntersectingCube(const CVector& vecStartPos, const CVector& vecEndPos, int16* nIntersecting, int16 maxEntitiesToFind, CEntity** aEntities, bool bVehicles, bool bPeds, bool bObjects);
	static void FindMissionEntitiesIntersectingCubeSectorList(CPtrList& list, const CVector& vecStartPos, const CVector& vecEndPos, int16* nIntersecting, int16 maxEntitiesToFind, CEntity** aEntities, bool bIsVehicleList, bool bIsPedList);

	static void ClearCarsFromArea(float x1, float y1, float z1, float x2, float y2, float z2);
	static void ClearPedsFromArea(float x1, float y1, float z1, float x2, float y2, float z2);
	static void CallOffChaseForArea(float x1, float y1, float x2, float y2);
	static void CallOffChaseForAreaSectorListVehicles(CPtrList& list, float x1, float y1, float x2, float y2, float fStartX, float fStartY, float fEndX, float fEndY);
	static void CallOffChaseForAreaSectorListPeds(CPtrList& list, float x1, float y1, float x2, float y2);

	static float GetSectorX(float f) { return ((f - WORLD_MIN_X)/SECTOR_SIZE_X); }
	static float GetSectorY(float f) { return ((f - WORLD_MIN_Y)/SECTOR_SIZE_Y); }
	static int GetSectorIndexX(float f) { return (int)GetSectorX(f); }
	static int GetSectorIndexY(float f) { return (int)GetSectorY(f); }
	static float GetWorldX(int x) { return x*SECTOR_SIZE_X + WORLD_MIN_X; }
	static float GetWorldY(int y) { return y*SECTOR_SIZE_Y + WORLD_MIN_Y; }

	static void RemoveEntityInsteadOfProcessingIt(CEntity* ent);
	static void RemoveFallenPeds();
	static void RemoveFallenCars();

	static void StopAllLawEnforcersInTheirTracks();
	static void SetAllCarsCanBeDamaged(bool);
	static void ExtinguishAllCarFiresInArea(CVector, float);
	static void SetCarsOnFire(float x, float y, float z, float radius, CEntity* reason);
	static void SetPedsOnFire(float x, float y, float z, float radius, CEntity* reason);

	static void Initialise();
	static void AddParticles();
	static void ShutDown();
	static void ClearForRestart(void);
	static void RepositionCertainDynamicObjects();
	static void RepositionOneObject(CEntity* pEntity);
	static void RemoveStaticObjects();
	static void Process();
	static void TriggerExplosion(const CVector& position, float fRadius, float fPower, CEntity* pCreator, bool bProcessVehicleBombTimer);
	static void TriggerExplosionSectorList(CPtrList& list, const CVector& position, float fRadius, float fPower, CEntity* pCreator, bool bProcessVehicleBombTimer);
	static void UseDetonator(CEntity *pEntity);
};

extern CColPoint gaTempSphereColPoints[MAX_COLLISION_POINTS];

