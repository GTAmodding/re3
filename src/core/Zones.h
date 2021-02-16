#pragma once

#include "Game.h"
#include "Gangs.h"

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
	int16 carThreshold[6];
	int16 copThreshold;
	int16 gangThreshold[NUM_GANGS];

	// Ped data
	uint16 pedDensity;
	uint16 copDensity;
	uint16 gangDensity[NUM_GANGS];
	uint16 pedGroup;
};


class CTheZones
{
	static CZone *m_pPlayersZone;
	static int16 FindIndex;

	static uint16 NumberOfAudioZones;
	static int16 AudioZoneArray[NUMAUDIOZONES];
	static uint16 TotalNumberOfMapZones;
	static uint16 TotalNumberOfZones;
	static CZone ZoneArray[NUMZONES];
	static CZone MapZoneArray[NUMMAPZONES];
	static uint16 TotalNumberOfZoneInfos;
	static CZoneInfo ZoneInfoArray[2*NUMZONES];
public:
	static eLevelName m_CurrLevel;

	static void Init(void);
	static void Update(void);
	static void CreateZone(char *name, eZoneType type,
	                       float minx, float miny, float minz,
	                       float maxx, float maxy, float maxz,
	                       eLevelName level);
	static void CreateMapZone(char *name, eZoneType type,
	                          float minx, float miny, float minz,
	                          float maxx, float maxy, float maxz,
	                          eLevelName level);
	static CZone *GetZone(uint16 i) { return &ZoneArray[i]; }
	static CZone *GetAudioZone(uint16 i) { return &ZoneArray[AudioZoneArray[i]]; }
	static void PostZoneCreation(void);
	static void InsertZoneIntoZoneHierarchy(CZone *zone);
	static bool InsertZoneIntoZoneHierRecursive(CZone *z1, CZone *z2);
	static bool ZoneIsEntirelyContainedWithinOtherZone(CZone *z1, CZone *z2);
	static bool PointLiesWithinZone(const CVector *v, CZone *zone);
	static eLevelName GetLevelFromPosition(const CVector *v);
	static CZone *FindSmallestZonePosition(const CVector *v);
	static CZone *FindSmallestZonePositionType(const CVector *v, eZoneType type);
	static CZone *FindSmallestZonePositionILN(const CVector *v);
	static int16 FindZoneByLabelAndReturnIndex(Const char *name);
	static CZoneInfo *GetZoneInfo(const CVector *v, uint8 day);
	static void GetZoneInfoForTimeOfDay(const CVector *pos, CZoneInfo *info);
	static void SetZoneCarInfo(uint16 zoneid, uint8 day, int16 carDensity,
		int16 gang0Num, int16 gang1Num, int16 gang2Num,
		int16 gang3Num, int16 gang4Num, int16 gang5Num,
		int16 gang6Num, int16 gang7Num, int16 gang8Num,
		int16 copNum,
		int16 car0Num, int16 car1Num, int16 car2Num,
		int16 car3Num, int16 car4Num, int16 car5Num);
	static void SetZonePedInfo(uint16 zoneid, uint8 day, int16 pedDensity,
		int16 gang0Density, int16 gang1Density, int16 gang2Density, int16 gang3Density,
		int16 gang4Density, int16 gang5Density, int16 gang6Density, int16 gang7Density,
		int16 gang8Density, int16 copDensity);
	static void SetCarDensity(uint16 zoneid, uint8 day, uint16 cardensity);
	static void SetPedDensity(uint16 zoneid, uint8 day, uint16 peddensity);
	static void SetPedGroup(uint16 zoneid, uint8 day, uint16 pedgroup);
	static int16 FindAudioZone(CVector *pos);
	static eLevelName FindZoneForPoint(const CVector &pos);
	static CZone *GetPointerForZoneIndex(ssize_t i) { return i == -1 ? nil : &ZoneArray[i]; }
	static ssize_t GetIndexForZonePointer(CZone *zone) { return zone == nil ? -1 : zone - ZoneArray; }
	static void AddZoneToAudioZoneArray(CZone *zone);
	static void InitialiseAudioZoneArray(void);
	static void SaveAllZones(uint8 *buffer, uint32 *length);
	static void LoadAllZones(uint8 *buffer, uint32 length);
};
