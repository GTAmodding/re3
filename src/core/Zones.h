#pragma once

#include "Game.h"
#include "Gangs.h"
#include "CarCtrl.h"

enum eZoneType
{
	ZONE_DEFAULT,
	ZONE_NAVIG,
	ZONE_INFO,
	ZONE_MAPZONE,
};

class CZone
{
public:
	char       name[8];
	float      minx;
	float      miny;
	float      minz;
	float      maxx;
	float      maxy;
	float      maxz;
	eZoneType  type;
	eLevelName level;
	int16      zoneinfoDay;
	int16      zoneinfoNight;
	CZone     *child;
	CZone     *parent;
	CZone     *next;

	wchar *GetTranslatedName(void);
};

class CZoneInfo
{
public:
	// Car data
	int16 carDensity;
	int16 carThreshold[CCarCtrl::NUM_CAR_CLASSES];
	int16 boatThreshold[CCarCtrl::NUM_BOAT_CLASSES];
	int16 gangThreshold[NUM_GANGS];
	int16 copThreshold;

	// Ped data
	uint16 pedDensity;
	uint16 gangPedThreshold[NUM_GANGS];
	uint16 copPedThreshold;
	uint16 pedGroup;
};


class CTheZones
{
	static int16 FindIndex;

	static uint16 NumberOfAudioZones;
	static int16 AudioZoneArray[NUMAUDIOZONES];
	static uint16 TotalNumberOfMapZones;
	static uint16 TotalNumberOfInfoZones;
	static uint16 TotalNumberOfNavigationZones;
	static CZone InfoZoneArray[NUMINFOZONES];
	static CZone MapZoneArray[NUMMAPZONES];
	static CZone NavigationZoneArray[NUMNAVIGZONES];
	static uint16 TotalNumberOfZoneInfos;
	static CZoneInfo ZoneInfoArray[2*NUMINFOZONES];
public:
	static eLevelName m_CurrLevel;

	static void Init(void);
	static void Update(void);
	static void CreateZone(char *name, eZoneType type,
	                       float minx, float miny, float minz,
	                       float maxx, float maxy, float maxz,
	                       eLevelName level);
	static CZone *GetInfoZone(uint16 i) { return &InfoZoneArray[i]; }
	static CZone *GetNavigationZone(uint16 i) { return &NavigationZoneArray[i]; }
	static CZone *GetMapZone(uint16 i) { return &MapZoneArray[i]; }
	static CZone *GetAudioZone(uint16 i) { return &NavigationZoneArray[AudioZoneArray[i]]; }
	static void PostZoneCreation(void);
	static void CheckZonesForOverlap(void);
	static void InsertZoneIntoZoneHierarchy(CZone *zone);
	static bool InsertZoneIntoZoneHierRecursive(CZone *z1, CZone *z2);
	static bool ZoneIsEntirelyContainedWithinOtherZone(CZone *z1, CZone *z2);
	static bool PointLiesWithinZone(const CVector *v, CZone *zone);
	static eLevelName GetLevelFromPosition(const CVector *v);
	static CZone *FindInformationZoneForPosition(const CVector *v);
	static CZone *FindSmallestNavigationZoneForPosition(const CVector *v, bool findDefault, bool findNavig);
	static int16 FindZoneByLabelAndReturnIndex(char *name, eZoneType type);
	static int16 FindNextZoneByLabelAndReturnIndex(char *name, eZoneType type);
	static CZoneInfo *GetZoneInfo(const CVector *v, uint8 day);
	static void GetZoneInfoForTimeOfDay(const CVector *pos, CZoneInfo *info);
	static void SetZoneCarInfo(uint16 zoneid, uint8 day, int16 carDensity,
		int16 copCarDensity, const int16 *gangCarDensities /*[NUMGANGS]*/);
	static void SetZoneCivilianCarInfo(uint16 zoneid, uint8 day,
		const int16* carDensities, const int16* boatDensities);
	static void SetZonePedInfo(uint16 zoneid, uint8 day, int16 pedDensity,
		int16 gang0Density, int16 gang1Density, int16 gang2Density, int16 gang3Density,
		int16 gang4Density, int16 gang5Density, int16 gang6Density, int16 gang7Density,
		int16 gang8Density, int16 copDensity);
	static void SetCarDensity(uint16 zoneid, uint8 day, uint16 cardensity);
	static void SetPedDensity(uint16 zoneid, uint8 day, uint16 peddensity);
	static void SetPedGroup(uint16 zoneid, uint8 day, uint16 pedgroup);
	static int16 FindAudioZone(CVector *pos);
	static CZone *GetPointerForNavigationZoneIndex(ssize_t i) { return i == -1 ? nil : &NavigationZoneArray[i]; }
	static ssize_t GetIndexForNavigationZonePointer(CZone *zone) { return zone == nil ? -1 : zone - NavigationZoneArray; }
	static void AddZoneToAudioZoneArray(CZone *zone);
	static void InitialiseAudioZoneArray(void);
	static void SaveAllZones(uint8 *buffer, uint32 *length);
	static void SaveOneZone(CZone *zone, uint8 **buffer, uint32 *length, eZoneType zoneType);
	static void LoadAllZones(uint8 *buffer, uint32 length);
	static void LoadOneZone(CZone *zone, uint8 **buffer, uint32 *length, eZoneType zoneType);
};
