#pragma once

#include "Game.h"
#include "Lists.h"
#include "PlayerInfo.h"

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
static_assert(sizeof(CSector) == 0x28, "CSector: error");

class CEntity;
struct CColPoint;
struct CColLine;
struct CStoredCollPoly;

class CWorld
{
	static CPtrList *ms_bigBuildingsList;	// [4];
	static CPtrList &ms_listMovingEntityPtrs;
	static CSector (*ms_aSectors)[NUMSECTORS_X];	// [NUMSECTORS_Y][NUMSECTORS_X];
	static uint16 &ms_nCurrentScanCode;

public:
	static CColPoint& ms_testSpherePoint;

	static uint8 &PlayerInFocus;
	static CPlayerInfo *Players;
	static CEntity *&pIgnoreEntity;
	static bool &bIncludeDeadPeds;
	static bool &bNoMoreCollisionTorque;
	static bool &bSecondShift;
	static bool &bForceProcessControl;
	static bool &bProcessCutsceneOnly;
	static bool &bDoingCarCollisions;
	static bool &bIncludeCarTyres;

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
	static void ClearExcitingStuffFromArea(const CVector &pos, float radius, uint8);

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
	
	static CEntity *TestSphereAgainstWorld(CVector centre, float distance, CEntity *entityToIgnore, bool checkBuildings, bool checkVehicles, bool checkPeds, bool checkObjects, bool checkDummies, bool ignoreSomeObjects);
	static CEntity *TestSphereAgainstSectorList(CPtrList&, CVector, float, CEntity*, bool);
	static void FindObjectsInRangeSectorList(CPtrList&, CVector&, float, bool, short*, short, CEntity**);
	static void FindObjectsInRange(CVector&, float, bool, short*, short, CEntity**, bool, bool, bool, bool, bool);
	static float FindGroundZForCoord(float x, float y);
	static float FindGroundZFor3DCoord(float x, float y, float z, bool *found);
	static float FindRoofZFor3DCoord(float x, float y, float z, bool *found);
	static void RemoveReferencesToDeletedObject(CEntity*);
	static void FindObjectsKindaColliding(const CVector &, float, bool, int16*, int16, CEntity **, bool, bool, bool, bool, bool);

	static float GetSectorX(float f) { return ((f - WORLD_MIN_X)/SECTOR_SIZE_X); }
	static float GetSectorY(float f) { return ((f - WORLD_MIN_Y)/SECTOR_SIZE_Y); }
	static int GetSectorIndexX(float f) { return (int)GetSectorX(f); }
	static int GetSectorIndexY(float f) { return (int)GetSectorY(f); }
	static float GetWorldX(int x) { return x*SECTOR_SIZE_X + WORLD_MIN_X; }
	static float GetWorldY(int y) { return y*SECTOR_SIZE_Y + WORLD_MIN_Y; }

	static void RemoveEntityInsteadOfProcessingIt(CEntity* ent);
	static void RemoveFallenPeds();
	static void RemoveFallenCars();

	static void Initialise();
	static void ShutDown();
	static void RepositionCertainDynamicObjects();
	static void RemoveStaticObjects();
	static void Process();
};

class CPlayerPed;
class CVehicle;
CPlayerPed *FindPlayerPed(void);
CVehicle *FindPlayerVehicle(void);
CVehicle *FindPlayerTrain(void);
CEntity *FindPlayerEntity(void);
CVector FindPlayerCoors(void);
CVector &FindPlayerSpeed(void);
CVector &FindPlayerCentreOfWorld(int32 player);
CVector &FindPlayerCentreOfWorld_NoSniperShift(void);
float FindPlayerHeading(void);
