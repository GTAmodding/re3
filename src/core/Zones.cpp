#include "common.h"

#include <ctype.h>

#include "Zones.h"

#include "Clock.h"
#include "Text.h"
#include "World.h"
#include "Timer.h"

eLevelName CTheZones::m_CurrLevel;
CZone *CTheZones::m_pPlayersZone;
int16 CTheZones::FindIndex;

uint16 CTheZones::NumberOfAudioZones;
int16 CTheZones::AudioZoneArray[NUMAUDIOZONES];
uint16 CTheZones::TotalNumberOfMapZones;
uint16 CTheZones::TotalNumberOfZones;
CZone CTheZones::ZoneArray[NUMZONES];
CZone CTheZones::MapZoneArray[NUMMAPZONES];
uint16 CTheZones::TotalNumberOfZoneInfos;
CZoneInfo CTheZones::ZoneInfoArray[2*NUMZONES];

#define SWAPF(a, b) { float t; t = a; a = b; b = t; }

inline bool IsNormalZone(int type) { return type == ZONE_DEFAULT || type == ZONE_NAVIG || type == ZONE_INFO; }

static void
CheckZoneInfo(CZoneInfo *info)
{
	assert(info->carThreshold[0] >= 0);
	assert(info->carThreshold[0] <= info->carThreshold[1]);
	assert(info->carThreshold[1] <= info->carThreshold[2]);
	assert(info->carThreshold[2] <= info->carThreshold[3]);
	assert(info->carThreshold[3] <= info->carThreshold[4]);
	assert(info->carThreshold[4] <= info->carThreshold[5]);
	assert(info->carThreshold[5] <= info->copThreshold);
	assert(info->copThreshold <= info->gangThreshold[0]);
	assert(info->gangThreshold[0] <= info->gangThreshold[1]);
	assert(info->gangThreshold[1] <= info->gangThreshold[2]);
	assert(info->gangThreshold[2] <= info->gangThreshold[3]);
	assert(info->gangThreshold[3] <= info->gangThreshold[4]);
	assert(info->gangThreshold[4] <= info->gangThreshold[5]);
	assert(info->gangThreshold[5] <= info->gangThreshold[6]);
	assert(info->gangThreshold[6] <= info->gangThreshold[7]);
	assert(info->gangThreshold[7] <= info->gangThreshold[8]);
}

wchar*
CZone::GetTranslatedName(void)
{
	return TheText.Get(name);
}

void
CTheZones::Init(void)
{
	int i;
	for(i = 0; i < NUMAUDIOZONES; i++)
		AudioZoneArray[i] = -1;
	NumberOfAudioZones = 0;

	for(i = 0; i < NUMZONES; i++)
		memset(&ZoneArray[i], 0, sizeof(CZone));

	CZoneInfo *zonei;
	int x = 1000/6;
	for(i = 0; i < 2*NUMZONES; i++){
		zonei = &ZoneInfoArray[i];
		zonei->carDensity = 10;
		zonei->carThreshold[0] = x;
		zonei->carThreshold[1] = zonei->carThreshold[0] + x;
		zonei->carThreshold[2] = zonei->carThreshold[1] + x;
		zonei->carThreshold[3] = zonei->carThreshold[2] + x;
		zonei->carThreshold[4] = zonei->carThreshold[3];
		zonei->carThreshold[5] = zonei->carThreshold[4];
		zonei->copThreshold = zonei->carThreshold[5] + x;
		zonei->gangThreshold[0] = zonei->copThreshold;
		zonei->gangThreshold[1] = zonei->gangThreshold[0];
		zonei->gangThreshold[2] = zonei->gangThreshold[1];
		zonei->gangThreshold[3] = zonei->gangThreshold[2];
		zonei->gangThreshold[4] = zonei->gangThreshold[3];
		zonei->gangThreshold[5] = zonei->gangThreshold[4];
		zonei->gangThreshold[6] = zonei->gangThreshold[5];
		zonei->gangThreshold[7] = zonei->gangThreshold[6];
		zonei->gangThreshold[8] = zonei->gangThreshold[7];
		CheckZoneInfo(zonei);
	}

	TotalNumberOfZoneInfos = 1;	// why 1?
	TotalNumberOfZones = 1;

	m_CurrLevel = LEVEL_GENERIC;
	m_pPlayersZone = &ZoneArray[0];

	strcpy(ZoneArray[0].name, "CITYZON");
	ZoneArray[0].minx = -4000.0f;
	ZoneArray[0].miny = -4000.0f;
	ZoneArray[0].minz = -500.0f;
	ZoneArray[0].maxx =  4000.0f;
	ZoneArray[0].maxy =  4000.0f;
	ZoneArray[0].maxz =  500.0f;
	ZoneArray[0].level = LEVEL_GENERIC;

	for(i = 0; i < NUMMAPZONES; i++){
		memset(&MapZoneArray[i], 0, sizeof(CZone));
		MapZoneArray[i].type = ZONE_MAPZONE;
	}

	TotalNumberOfMapZones = 1;

	strcpy(MapZoneArray[0].name, "THEMAP");
	MapZoneArray[0].minx = -4000.0f;
	MapZoneArray[0].miny = -4000.0f;
	MapZoneArray[0].minz = -500.0f;
	MapZoneArray[0].maxx =  4000.0f;
	MapZoneArray[0].maxy =  4000.0f;
	MapZoneArray[0].maxz =  500.0f;
	MapZoneArray[0].level = LEVEL_GENERIC;
}

void
CTheZones::Update(void)
{
#ifdef SQUEEZE_PERFORMANCE
	if (CTimer::GetFrameCounter() % 5 != 0)
		return;
#endif
	CVector pos;
	pos = FindPlayerCoors();
	m_pPlayersZone = FindSmallestZonePosition(&pos);
	m_CurrLevel = GetLevelFromPosition(&pos);
}

void
CTheZones::CreateZone(char *name, eZoneType type,
	              float minx, float miny, float minz,
	              float maxx, float maxy, float maxz,
	              eLevelName level)
{
	char *p;
	char tmpname[8];

	if(minx > maxx) SWAPF(minx, maxx);
	if(miny > maxy) SWAPF(miny, maxy);
	if(minz > maxz) SWAPF(minz, maxz);

	// make upper case
	for(p = name; *p; p++) if(islower(*p)) *p = toupper(*p);

	// add zone
	strncpy(tmpname, name, 7);
	tmpname[7] = '\0';
	strcpy(ZoneArray[TotalNumberOfZones].name, tmpname);
	ZoneArray[TotalNumberOfZones].type = type;
	ZoneArray[TotalNumberOfZones].minx = minx;
	ZoneArray[TotalNumberOfZones].miny = miny;
	ZoneArray[TotalNumberOfZones].minz = minz;
	ZoneArray[TotalNumberOfZones].maxx = maxx;
	ZoneArray[TotalNumberOfZones].maxy = maxy;
	ZoneArray[TotalNumberOfZones].maxz = maxz;
	ZoneArray[TotalNumberOfZones].level = level;
	if(IsNormalZone(type)){
		ZoneArray[TotalNumberOfZones].zoneinfoDay = TotalNumberOfZoneInfos++;
		ZoneArray[TotalNumberOfZones].zoneinfoNight = TotalNumberOfZoneInfos++;
	}
	TotalNumberOfZones++;
}

void
CTheZones::CreateMapZone(char *name, eZoneType type,
	                 float minx, float miny, float minz,
	                 float maxx, float maxy, float maxz,
	                 eLevelName level)
{
	CZone *zone;
	char *p;

	if(minx > maxx) SWAPF(minx, maxx);
	if(miny > maxy) SWAPF(miny, maxy);
	if(minz > maxz) SWAPF(minz, maxz);

	// make upper case
	for(p = name; *p; p++) if(islower(*p)) *p = toupper(*p);

	// add zone
	zone = &MapZoneArray[TotalNumberOfMapZones++];
	strncpy(zone->name, name, 7);
	zone->name[7] = '\0';
	zone->type = type;
	zone->minx = minx;
	zone->miny = miny;
	zone->minz = minz;
	zone->maxx = maxx;
	zone->maxy = maxy;
	zone->maxz = maxz;
	zone->level = level;
}

void
CTheZones::PostZoneCreation(void)
{
	int i;
	for(i = 1; i < TotalNumberOfZones; i++)
		InsertZoneIntoZoneHierarchy(&ZoneArray[i]);
	InitialiseAudioZoneArray();
}

void
CTheZones::InsertZoneIntoZoneHierarchy(CZone *zone)
{
	zone->child = nil;
	zone->parent = nil;
	zone->next = nil;
	InsertZoneIntoZoneHierRecursive(zone, &ZoneArray[0]);
}

bool
CTheZones::InsertZoneIntoZoneHierRecursive(CZone *inner, CZone *outer)
{
	int n;
	CZone *child, *next, *insert;

	// return false if inner was not inserted into outer
	if(outer == nil ||
	   !ZoneIsEntirelyContainedWithinOtherZone(inner, outer))
		return false;

	// try to insert inner into children of outer
	for(child = outer->child; child; child = child->next)
		if(InsertZoneIntoZoneHierRecursive(inner, child))
			return true;

	// insert inner as child of outer
	// count number of outer's children contained within inner
	n = 0;
	for(child = outer->child; child; child = child->next)
		if(ZoneIsEntirelyContainedWithinOtherZone(child, inner))
			n++;
	inner->next = outer->child;
	inner->parent = outer;
	outer->child = inner;
	// move children from outer to inner
	if(n){
		insert = inner;
		for(child = inner->next; child; child = next){
			next = child->next;
			if(ZoneIsEntirelyContainedWithinOtherZone(child,inner)){
				insert->next = child->next;
				child->parent = inner;
				child->next = inner->child;
				inner->child = child;
			}else
				insert = child;
		}
	}

	return true;
}

bool
CTheZones::ZoneIsEntirelyContainedWithinOtherZone(CZone *inner, CZone *outer)
{
	char tmp[100];

	if(inner->minx < outer->minx ||
	   inner->maxx > outer->maxx ||
	   inner->miny < outer->miny ||
	   inner->maxy > outer->maxy ||
	   inner->minz < outer->minz ||
	   inner->maxz > outer->maxz){
		CVector vmin(inner->minx, inner->miny, inner->minz);
		if(PointLiesWithinZone(&vmin, outer))
			sprintf(tmp, "Overlapping zones %s and %s\n",
			      inner->name, outer->name);
		CVector vmax(inner->maxx, inner->maxy, inner->maxz);
		if(PointLiesWithinZone(&vmax, outer))
			sprintf(tmp, "Overlapping zones %s and %s\n",
			      inner->name, outer->name);
		return false;
	}
	return true;
}

bool
CTheZones::PointLiesWithinZone(const CVector *v, CZone *zone)
{
	return zone->minx <= v->x && v->x <= zone->maxx &&
	       zone->miny <= v->y && v->y <= zone->maxy &&
	       zone->minz <= v->z && v->z <= zone->maxz;
}

eLevelName
CTheZones::GetLevelFromPosition(CVector const *v)
{
	int i;
//	char tmp[116];
//	if(!PointLiesWithinZone(v, &MapZoneArray[0]))
//		sprintf(tmp, "x = %.3f y= %.3f z = %.3f\n", v.x, v.y, v.z);
	for(i = 1; i < TotalNumberOfMapZones; i++)
		if(PointLiesWithinZone(v, &MapZoneArray[i]))
			return MapZoneArray[i].level;
	return MapZoneArray[0].level;
}

CZone*
CTheZones::FindSmallestZonePosition(const CVector *v)
{
	CZone *best = &ZoneArray[0];
	// zone to test next
	CZone *zone = ZoneArray[0].child;
	while(zone)
		// if in zone, descent into children
		if(PointLiesWithinZone(v, zone)){
			best = zone;
			zone = zone->child;
		// otherwise try next zone
		}else
			zone = zone->next;
	return best;
}

CZone*
CTheZones::FindSmallestZonePositionType(const CVector *v, eZoneType type)
{
	CZone *best = nil;
	if(ZoneArray[0].type == type)
		best = &ZoneArray[0];
	// zone to test next
	CZone *zone = ZoneArray[0].child;
	while(zone)
		// if in zone, descent into children
		if(PointLiesWithinZone(v, zone)){
			if(zone->type == type)
				best = zone;
			zone = zone->child;
		// otherwise try next zone
		}else
			zone = zone->next;
	return best;
}

CZone*
CTheZones::FindSmallestZonePositionILN(const CVector *v)
{
	CZone *best = nil;
	if(IsNormalZone(ZoneArray[0].type))
		best = &ZoneArray[0];
	// zone to test next
	CZone *zone = ZoneArray[0].child;
	while(zone)
		// if in zone, descent into children
		if(PointLiesWithinZone(v, zone)){
			if(IsNormalZone(zone->type))
				best = zone;
			zone = zone->child;
		// otherwise try next zone
		}else
			zone = zone->next;
	return best;
}

int16
CTheZones::FindZoneByLabelAndReturnIndex(Const char *name)
{
	char str[8];
	memset(str, 0, 8);
	strncpy(str, name, 8);
	for(FindIndex = 0; FindIndex < TotalNumberOfZones; FindIndex++)
		if(strcmp(GetZone(FindIndex)->name, name) == 0)
			return FindIndex;
	return -1;
}

CZoneInfo*
CTheZones::GetZoneInfo(const CVector *v, uint8 day)
{
	CZone *zone;
	zone = FindSmallestZonePositionILN(v);
	if(zone == nil)
		return &ZoneInfoArray[0];
	return &ZoneInfoArray[day ? zone->zoneinfoDay : zone->zoneinfoNight];
}

void
CTheZones::GetZoneInfoForTimeOfDay(const CVector *pos, CZoneInfo *info)
{
	CZoneInfo *day, *night;
	float d, n;

	day = GetZoneInfo(pos, 1);
	night = GetZoneInfo(pos, 0);

	if(CClock::GetIsTimeInRange(8, 19))
		*info = *day;
	else if(CClock::GetIsTimeInRange(22, 5))
		*info = *night;
	else{
		if(CClock::GetIsTimeInRange(19, 22)){
			n = (CClock::GetHours() - 19) / 3.0f;
			assert(n >= 0.0f && n <= 1.0f);
			d = 1.0f - n;
		}else{
			d = (CClock::GetHours() - 5) / 3.0f;
			assert(d >= 0.0f && d <= 1.0f);
			n = 1.0f - d;
		}
		info->carDensity = day->carDensity * d + night->carDensity * n;
		info->carThreshold[0] = day->carThreshold[0] * d + night->carThreshold[0] * n;
		info->carThreshold[1] = day->carThreshold[1] * d + night->carThreshold[1] * n;
		info->carThreshold[2] = day->carThreshold[2] * d + night->carThreshold[2] * n;
		info->carThreshold[3] = day->carThreshold[3] * d + night->carThreshold[3] * n;
		info->carThreshold[4] = day->carThreshold[4] * d + night->carThreshold[4] * n;
		info->carThreshold[5] = day->carThreshold[5] * d + night->carThreshold[5] * n;
		info->copThreshold = day->copThreshold * d + night->copThreshold * n;
		info->gangThreshold[0] = day->gangThreshold[0] * d + night->gangThreshold[0] * n;
		info->gangThreshold[1] = day->gangThreshold[1] * d + night->gangThreshold[1] * n;
		info->gangThreshold[2] = day->gangThreshold[2] * d + night->gangThreshold[2] * n;
		info->gangThreshold[3] = day->gangThreshold[3] * d + night->gangThreshold[3] * n;
		info->gangThreshold[4] = day->gangThreshold[4] * d + night->gangThreshold[4] * n;
		info->gangThreshold[5] = day->gangThreshold[5] * d + night->gangThreshold[5] * n;
		info->gangThreshold[6] = day->gangThreshold[6] * d + night->gangThreshold[6] * n;
		info->gangThreshold[7] = day->gangThreshold[7] * d + night->gangThreshold[7] * n;
		info->gangThreshold[8] = day->gangThreshold[8] * d + night->gangThreshold[8] * n;

		info->pedDensity = day->pedDensity * d + night->pedDensity * n;
		info->copDensity = day->copDensity * d + night->copDensity * n;
		info->gangDensity[0] = day->gangDensity[0] * d + night->gangDensity[0] * n;
		info->gangDensity[1] = day->gangDensity[1] * d + night->gangDensity[1] * n;
		info->gangDensity[2] = day->gangDensity[2] * d + night->gangDensity[2] * n;
		info->gangDensity[3] = day->gangDensity[3] * d + night->gangDensity[3] * n;
		info->gangDensity[4] = day->gangDensity[4] * d + night->gangDensity[4] * n;
		info->gangDensity[5] = day->gangDensity[5] * d + night->gangDensity[5] * n;
		info->gangDensity[6] = day->gangDensity[6] * d + night->gangDensity[6] * n;
		info->gangDensity[7] = day->gangDensity[7] * d + night->gangDensity[7] * n;
		info->gangDensity[8] = day->gangDensity[8] * d + night->gangDensity[8] * n;
	}
	if(CClock::GetIsTimeInRange(5, 19))
		info->pedGroup = day->pedGroup;
	else
		info->pedGroup = night->pedGroup;

	CheckZoneInfo(info);
}

void
CTheZones::SetZoneCarInfo(uint16 zoneid, uint8 day, int16 carDensity,
	int16 gang0Num, int16 gang1Num, int16 gang2Num,
	int16 gang3Num, int16 gang4Num, int16 gang5Num,
	int16 gang6Num, int16 gang7Num, int16 gang8Num,
	int16 copNum,
	int16 car0Num, int16 car1Num, int16 car2Num,
	int16 car3Num, int16 car4Num, int16 car5Num)
{
	CZone *zone;
	CZoneInfo *info;
	zone = GetZone(zoneid);
	info = &ZoneInfoArray[day ? zone->zoneinfoDay : zone->zoneinfoNight];

	CheckZoneInfo(info);

	if(carDensity != -1) info->carDensity = carDensity;
	int16 oldCar1Num = info->carThreshold[1] - info->carThreshold[0];
	int16 oldCar2Num = info->carThreshold[2] - info->carThreshold[1];
	int16 oldCar3Num = info->carThreshold[3] - info->carThreshold[2];
	int16 oldCar4Num = info->carThreshold[4] - info->carThreshold[3];
	int16 oldCar5Num = info->carThreshold[5] - info->carThreshold[4];
	int16 oldCopNum = info->copThreshold - info->carThreshold[5];
	int16 oldGang0Num = info->gangThreshold[0] - info->copThreshold;
	int16 oldGang1Num = info->gangThreshold[1] - info->gangThreshold[0];
	int16 oldGang2Num = info->gangThreshold[2] - info->gangThreshold[1];
	int16 oldGang3Num = info->gangThreshold[3] - info->gangThreshold[2];
	int16 oldGang4Num = info->gangThreshold[4] - info->gangThreshold[3];
	int16 oldGang5Num = info->gangThreshold[5] - info->gangThreshold[4];
	int16 oldGang6Num = info->gangThreshold[6] - info->gangThreshold[5];
	int16 oldGang7Num = info->gangThreshold[7] - info->gangThreshold[6];
	int16 oldGang8Num = info->gangThreshold[8] - info->gangThreshold[7];

	if(car0Num != -1) info->carThreshold[0] = car0Num;
	if(car1Num != -1) info->carThreshold[1] = info->carThreshold[0] + car1Num;
	else              info->carThreshold[1] = info->carThreshold[0] + oldCar1Num;
	if(car2Num != -1) info->carThreshold[2] = info->carThreshold[1] + car2Num;
	else              info->carThreshold[2] = info->carThreshold[1] + oldCar2Num;
	if(car3Num != -1) info->carThreshold[3] = info->carThreshold[2] + car3Num;
	else              info->carThreshold[3] = info->carThreshold[2] + oldCar3Num;
	if(car4Num != -1) info->carThreshold[4] = info->carThreshold[3] + car4Num;
	else              info->carThreshold[4] = info->carThreshold[3] + oldCar4Num;
	if(car5Num != -1) info->carThreshold[5] = info->carThreshold[4] + car5Num;
	else              info->carThreshold[5] = info->carThreshold[4] + oldCar5Num;
	if(copNum != -1) info->copThreshold = info->carThreshold[5] + copNum;
	else             info->copThreshold = info->carThreshold[5] + oldCopNum;
	if(gang0Num != -1) info->gangThreshold[0] = info->copThreshold + gang0Num;
	else               info->gangThreshold[0] = info->copThreshold + oldGang0Num;
	if(gang1Num != -1) info->gangThreshold[1] = info->gangThreshold[0] + gang1Num;
	else               info->gangThreshold[1] = info->gangThreshold[0] + oldGang1Num;
	if(gang2Num != -1) info->gangThreshold[2] = info->gangThreshold[1] + gang2Num;
	else               info->gangThreshold[2] = info->gangThreshold[1] + oldGang2Num;
	if(gang3Num != -1) info->gangThreshold[3] = info->gangThreshold[2] + gang3Num;
	else               info->gangThreshold[3] = info->gangThreshold[2] + oldGang3Num;
	if(gang4Num != -1) info->gangThreshold[4] = info->gangThreshold[3] + gang4Num;
	else               info->gangThreshold[4] = info->gangThreshold[3] + oldGang4Num;
	if(gang5Num != -1) info->gangThreshold[5] = info->gangThreshold[4] + gang5Num;
	else               info->gangThreshold[5] = info->gangThreshold[4] + oldGang5Num;
	if(gang6Num != -1) info->gangThreshold[6] = info->gangThreshold[5] + gang6Num;
	else               info->gangThreshold[6] = info->gangThreshold[5] + oldGang6Num;
	if(gang7Num != -1) info->gangThreshold[7] = info->gangThreshold[6] + gang7Num;
	else               info->gangThreshold[7] = info->gangThreshold[6] + oldGang7Num;
	if(gang8Num != -1) info->gangThreshold[8] = info->gangThreshold[7] + gang8Num;
	else               info->gangThreshold[8] = info->gangThreshold[7] + oldGang8Num;

	CheckZoneInfo(info);
}

void
CTheZones::SetZonePedInfo(uint16 zoneid, uint8 day, int16 pedDensity,
	int16 gang0Density, int16 gang1Density, int16 gang2Density, int16 gang3Density,
	int16 gang4Density, int16 gang5Density, int16 gang6Density, int16 gang7Density,
	int16 gang8Density, int16 copDensity)
{
	CZone *zone;
	CZoneInfo *info;
	zone = GetZone(zoneid);
	info = &ZoneInfoArray[day ? zone->zoneinfoDay : zone->zoneinfoNight];
	if(pedDensity != -1) info->pedDensity = pedDensity;
	if(copDensity != -1) info->copDensity = copDensity;
	if(gang0Density != -1) info->gangDensity[0] = gang0Density;
	if(gang1Density != -1) info->gangDensity[1] = gang1Density;
	if(gang2Density != -1) info->gangDensity[2] = gang2Density;
	if(gang3Density != -1) info->gangDensity[3] = gang3Density;
	if(gang4Density != -1) info->gangDensity[4] = gang4Density;
	if(gang5Density != -1) info->gangDensity[5] = gang5Density;
	if(gang6Density != -1) info->gangDensity[6] = gang6Density;
	if(gang7Density != -1) info->gangDensity[7] = gang7Density;
	if(gang8Density != -1) info->gangDensity[8] = gang8Density;
}

void
CTheZones::SetCarDensity(uint16 zoneid, uint8 day, uint16 cardensity)
{
	CZone *zone;
	zone = GetZone(zoneid);
	if(IsNormalZone(zone->type))
		ZoneInfoArray[day ? zone->zoneinfoDay : zone->zoneinfoNight].carDensity = cardensity;
}

void
CTheZones::SetPedDensity(uint16 zoneid, uint8 day, uint16 peddensity)
{
	CZone *zone;
	zone = GetZone(zoneid);
	if(IsNormalZone(zone->type))
		ZoneInfoArray[day ? zone->zoneinfoDay : zone->zoneinfoNight].pedDensity = peddensity;
}

void
CTheZones::SetPedGroup(uint16 zoneid, uint8 day, uint16 pedgroup)
{
	CZone *zone;
	zone = GetZone(zoneid);
	if(IsNormalZone(zone->type))
		ZoneInfoArray[day ? zone->zoneinfoDay : zone->zoneinfoNight].pedGroup = pedgroup;
}

int16
CTheZones::FindAudioZone(CVector *pos)
{
	int i;

	for(i = 0; i < NumberOfAudioZones; i++)
		if(PointLiesWithinZone(pos, GetAudioZone(i)))
			return i;
	return -1;
}

eLevelName
CTheZones::FindZoneForPoint(const CVector &pos)
{
	if(PointLiesWithinZone(&pos, GetZone(FindZoneByLabelAndReturnIndex("IND_ZON"))))
		return LEVEL_INDUSTRIAL;
	if(PointLiesWithinZone(&pos, GetZone(FindZoneByLabelAndReturnIndex("COM_ZON"))))
		return LEVEL_COMMERCIAL;
	if(PointLiesWithinZone(&pos, GetZone(FindZoneByLabelAndReturnIndex("SUB_ZON"))))
		return LEVEL_SUBURBAN;
	return LEVEL_GENERIC;
}

void
CTheZones::AddZoneToAudioZoneArray(CZone *zone)
{
	int i, z;

	if(zone->type != ZONE_DEFAULT)
		return;

	/* This is a bit stupid */
	z = -1;
	for(i = 0; i < NUMZONES; i++)
		if(&ZoneArray[i] == zone)
			z = i;
	AudioZoneArray[NumberOfAudioZones++] = z;
}

void
CTheZones::InitialiseAudioZoneArray(void)
{
	bool gonext;
	CZone *zone;

	gonext = false;
	zone = &ZoneArray[0];
	// Go deep first,
	// set gonext when backing up a level to visit the next child
	while(zone)
		if(gonext){
			AddZoneToAudioZoneArray(zone);
			if(zone->next){
				gonext = false;
				zone = zone->next;
			}else
				zone = zone->parent;
		}else if(zone->child)
			zone = zone->child;
		else{
			AddZoneToAudioZoneArray(zone);
			if(zone->next)
				zone = zone->next;
			else{
				gonext = true;
				zone = zone->parent;
			}
		}
}

void
CTheZones::SaveAllZones(uint8 *buffer, uint32 *size)
{
	INITSAVEBUF
	int i;

	*size = SAVE_HEADER_SIZE
		+ sizeof(int32) // GetIndexForZonePointer
		+ sizeof(m_CurrLevel) + sizeof(FindIndex)
		+ sizeof(int16) // padding
		+ sizeof(ZoneArray) + sizeof(ZoneInfoArray)
		+ sizeof(TotalNumberOfZones) + sizeof(TotalNumberOfZoneInfos)
		+ sizeof(MapZoneArray) + sizeof(AudioZoneArray)
		+ sizeof(TotalNumberOfMapZones) + sizeof(NumberOfAudioZones);

	WriteSaveHeader(buffer, 'Z', 'N', 'S', '\0', *size - SAVE_HEADER_SIZE);

	WriteSaveBuf(buffer, (int32)GetIndexForZonePointer(m_pPlayersZone));
	WriteSaveBuf(buffer, m_CurrLevel);
	WriteSaveBuf(buffer, FindIndex);
	WriteSaveBuf(buffer, (int16)0); // padding

	for(i = 0; i < ARRAY_SIZE(ZoneArray); i++){
		CZone *zone = WriteSaveBuf(buffer, ZoneArray[i]);
		zone->child = (CZone*)GetIndexForZonePointer(ZoneArray[i].child);
		zone->parent = (CZone*)GetIndexForZonePointer(ZoneArray[i].parent);
		zone->next = (CZone*)GetIndexForZonePointer(ZoneArray[i].next);
	}

	for(i = 0; i < ARRAY_SIZE(ZoneInfoArray); i++)
		WriteSaveBuf(buffer, ZoneInfoArray[i]);

	WriteSaveBuf(buffer, TotalNumberOfZones);
	WriteSaveBuf(buffer, TotalNumberOfZoneInfos);

	for(i = 0; i < ARRAY_SIZE(MapZoneArray); i++) {
		CZone* zone = WriteSaveBuf(buffer, MapZoneArray[i]);

		/*
		The call of GetIndexForZonePointer is wrong, as it is
		meant for a different array, but the game doesn't brake
		if those fields are nil. Let's make sure they are.
		*/
		assert(MapZoneArray[i].child == nil);
		assert(MapZoneArray[i].parent == nil);
		assert(MapZoneArray[i].next == nil);
		zone->child = (CZone*)GetIndexForZonePointer(MapZoneArray[i].child);
		zone->parent = (CZone*)GetIndexForZonePointer(MapZoneArray[i].parent);
		zone->next = (CZone*)GetIndexForZonePointer(MapZoneArray[i].next);
	}

	for(i = 0; i < ARRAY_SIZE(AudioZoneArray); i++)
		WriteSaveBuf(buffer, AudioZoneArray[i]);

	WriteSaveBuf(buffer, TotalNumberOfMapZones);
	WriteSaveBuf(buffer, NumberOfAudioZones);

	VALIDATESAVEBUF(*size)
}

void
CTheZones::LoadAllZones(uint8 *buffer, uint32 size)
{
	INITSAVEBUF
	int i;

	CheckSaveHeader(buffer, 'Z', 'N', 'S', '\0', size - SAVE_HEADER_SIZE);

	m_pPlayersZone = GetPointerForZoneIndex(ReadSaveBuf<int32>(buffer));
	m_CurrLevel = ReadSaveBuf<eLevelName>(buffer);
	FindIndex = ReadSaveBuf<int16>(buffer);
	ReadSaveBuf<int16>(buffer);

	for(i = 0; i < ARRAY_SIZE(ZoneArray); i++){
		ZoneArray[i] = ReadSaveBuf<CZone>(buffer);

		ZoneArray[i].child = GetPointerForZoneIndex((uintptr)ZoneArray[i].child);
		ZoneArray[i].parent = GetPointerForZoneIndex((uintptr)ZoneArray[i].parent);
		ZoneArray[i].next = GetPointerForZoneIndex((uintptr)ZoneArray[i].next);
	}

	for(i = 0; i < ARRAY_SIZE(ZoneInfoArray); i++)
		ZoneInfoArray[i] = ReadSaveBuf<CZoneInfo>(buffer);

	TotalNumberOfZones = ReadSaveBuf<int16>(buffer);
	TotalNumberOfZoneInfos = ReadSaveBuf<int16>(buffer);

	for(i = 0; i < ARRAY_SIZE(MapZoneArray); i++){
		MapZoneArray[i] = ReadSaveBuf<CZone>(buffer);

		/*
		The call of GetPointerForZoneIndex is wrong, as it is
		meant for a different array, but the game doesn't brake
		if save data stored is -1.
		*/
		MapZoneArray[i].child = GetPointerForZoneIndex((uintptr)MapZoneArray[i].child);
		MapZoneArray[i].parent = GetPointerForZoneIndex((uintptr)MapZoneArray[i].parent);
		MapZoneArray[i].next = GetPointerForZoneIndex((uintptr)MapZoneArray[i].next);
		assert(MapZoneArray[i].child == nil);
		assert(MapZoneArray[i].parent == nil);
		assert(MapZoneArray[i].next == nil);
	}

	for(i = 0; i < ARRAY_SIZE(AudioZoneArray); i++)
		AudioZoneArray[i] = ReadSaveBuf<int16>(buffer);

	TotalNumberOfMapZones = ReadSaveBuf<uint16>(buffer);
	NumberOfAudioZones = ReadSaveBuf<uint16>(buffer);

	VALIDATESAVEBUF(size)
}
