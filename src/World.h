#pragma once

#include "Game.h"
#include "Lists.h"
#include "PlayerInfo.h"

/* Sectors span from -2000 to 2000 in x and y.
 * With 100x100 sectors, each is 40x40 units. */

#define NUMSECTORS_X 100
#define NUMSECTORS_Y 100

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
	static unsigned char &PlayerInFocus;
	static CPlayerInfo *Players;
	static CEntity *&pIgnoreEntity;
	static bool &bIncludeDeadPeds;
	static bool &bNoMoreCollisionTorque;
	static bool &bSecondShift;
	static bool &bForceProcessControl;
	static bool &bProcessCutsceneOnly;


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

	static float FindGroundZForCoord(float x, float y);
	static float FindGroundZFor3DCoord(float x, float y, float z, bool *found);
	static float FindRoofZFor3DCoord(float x, float y, float z, bool *found);

	static float GetSectorX(float f) { return ((f + 2000.0f)/40.0f); }
	static float GetSectorY(float f) { return ((f + 2000.0f)/40.0f); }
	static int GetSectorIndexX(float f) { return (int)GetSectorX(f); }
	static int GetSectorIndexY(float f) { return (int)GetSectorY(f); }
	static float GetWorldX(int x) { return x*40.0f - 2000.0f; }
	static float GetWorldY(int y) { return y*40.0f - 2000.0f; }
};

class CPed;
class CVehicle;
CPed *FindPlayerPed(void);
CVector &FindPlayerCoors(CVector &v);
CVehicle *FindPlayerVehicle(void);
CVehicle *FindPlayerTrain(void);
