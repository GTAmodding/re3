#include "common.h"

#include <ctype.h>

#include "Zones.h"

#include "Clock.h"
#include "Text.h"
#include "World.h"

eLevelName CTheZones::m_CurrLevel;
int16 CTheZones::FindIndex;

uint16 CTheZones::NumberOfAudioZones;
int16 CTheZones::AudioZoneArray[NUMAUDIOZONES];
uint16 CTheZones::TotalNumberOfMapZones;
uint16 CTheZones::TotalNumberOfInfoZones;
uint16 CTheZones::TotalNumberOfNavigationZones;
CZone CTheZones::InfoZoneArray[NUMINFOZONES];
CZone CTheZones::MapZoneArray[NUMMAPZONES];
CZone CTheZones::NavigationZoneArray[NUMNAVIGZONES];
uint16 CTheZones::TotalNumberOfZoneInfos;
CZoneInfo CTheZones::ZoneInfoArray[2*NUMINFOZONES];


#define SWAPF(a, b) { float t; t = a; a = b; b = t; }

//--MIAMI: done
wchar*
CZone::GetTranslatedName(void)
{
	return TheText.Get(name);
}

//--MIAMI: done
void
CTheZones::Init(void)
{
	int i, j;
	for(i = 0; i < NUMAUDIOZONES; i++)
		AudioZoneArray[i] = -1;
	NumberOfAudioZones = 0;

	for(i = 0; i < NUMNAVIGZONES; i++)
		memset(&NavigationZoneArray[i], 0, sizeof(CZone));

	for(i = 0; i < NUMINFOZONES; i++)
		memset(&InfoZoneArray[i], 0, sizeof(CZone));

	int x = 1000/9;
	for(i = 0; i < 2*NUMINFOZONES; i++){
		// Cars

		ZoneInfoArray[i].carDensity = 10;
		ZoneInfoArray[i].carThreshold[0] = x;
		ZoneInfoArray[i].carThreshold[1] = ZoneInfoArray[i].carThreshold[0] + x;
		ZoneInfoArray[i].carThreshold[2] = ZoneInfoArray[i].carThreshold[1] + x;
		ZoneInfoArray[i].carThreshold[3] = ZoneInfoArray[i].carThreshold[2] + x;
		ZoneInfoArray[i].carThreshold[4] = ZoneInfoArray[i].carThreshold[3] + x;
		ZoneInfoArray[i].carThreshold[5] = ZoneInfoArray[i].carThreshold[4] + x;
		ZoneInfoArray[i].carThreshold[6] = ZoneInfoArray[i].carThreshold[5] + x;
		ZoneInfoArray[i].carThreshold[7] = ZoneInfoArray[i].carThreshold[6] + x;
		ZoneInfoArray[i].carThreshold[8] = 1000;

		ZoneInfoArray[i].boatThreshold[0] = 500;
		ZoneInfoArray[i].boatThreshold[1] = 1000;

		// What's going on here? this looks more like density
		ZoneInfoArray[i].copThreshold = 50;
		for(j = 0; j < NUM_GANGS; j++)
			ZoneInfoArray[i].gangThreshold[j] = ZoneInfoArray[i].copThreshold;

		// Peds

		ZoneInfoArray[i].pedDensity = 12;

		// What's going on here? this looks more like density
		ZoneInfoArray[i].copPedThreshold = 50;
		for(j = 0; j < NUM_GANGS; j++)
			ZoneInfoArray[i].gangPedThreshold[j] = ZoneInfoArray[i].copPedThreshold;

		ZoneInfoArray[i].pedGroup = 0;
	}

	TotalNumberOfZoneInfos = 1;	// why 1?
	TotalNumberOfNavigationZones = 1;
	TotalNumberOfInfoZones = 1;

	strcpy(InfoZoneArray[0].name, "CITYINF");
	InfoZoneArray[0].minx = -2400.0f;
	InfoZoneArray[0].miny = -2000.0f;
	InfoZoneArray[0].minz = -500.0f;
	InfoZoneArray[0].maxx =  1600.0f;
	InfoZoneArray[0].maxy =  2000.0f;
	InfoZoneArray[0].maxz =  500.0f;
	InfoZoneArray[0].level = LEVEL_NONE;
	InfoZoneArray[0].type = ZONE_INFO;

	strcpy(NavigationZoneArray[0].name, "VICE_C");
	NavigationZoneArray[0].minx = -2400.0f;
	NavigationZoneArray[0].miny = -2000.0f;
	NavigationZoneArray[0].minz = -500.0f;
	NavigationZoneArray[0].maxx =  1600.0f;
	NavigationZoneArray[0].maxy =  2000.0f;
	NavigationZoneArray[0].maxz =  500.0f;
	NavigationZoneArray[0].level = LEVEL_NONE;
	NavigationZoneArray[0].type = ZONE_NAVIG;

	m_CurrLevel = LEVEL_NONE;

	for(i = 0; i < NUMMAPZONES; i++){
		memset(&MapZoneArray[i], 0, sizeof(CZone));
		MapZoneArray[i].type = ZONE_MAPZONE;
	}
	TotalNumberOfMapZones = 1;
	strcpy(MapZoneArray[0].name, "THEMAP");
	MapZoneArray[0].minx = -2400.0f;
	MapZoneArray[0].miny = -2000.0f;
	MapZoneArray[0].minz = -500.0f;
	MapZoneArray[0].maxx =  1600.0f;
	MapZoneArray[0].maxy =  2000.0f;
	MapZoneArray[0].maxz =  500.0f;
	MapZoneArray[0].level = LEVEL_NONE;
}

//--MIAMI: done
void
CTheZones::Update(void)
{
	CVector pos;
	pos = FindPlayerCoors();
	m_CurrLevel = GetLevelFromPosition(&pos);
}

//--MIAMI: done
void
CTheZones::CreateZone(char *name, eZoneType type,
	              float minx, float miny, float minz,
	              float maxx, float maxy, float maxz,
	              eLevelName level)
{
	char tmpname[24];
	char *p;

	if(minx > maxx) SWAPF(minx, maxx);
	if(miny > maxy) SWAPF(miny, maxy);
	if(minz > maxz) SWAPF(minz, maxz);

	// make upper case
	for(p = name; *p; p++) if(islower(*p)) *p = toupper(*p);

	strncpy(tmpname, name, 7);
	tmpname[7] = '\0';

	// add zone
	switch(type){
	case ZONE_DEFAULT:
	case ZONE_NAVIG:
		assert(TotalNumberOfNavigationZones < NUMNAVIGZONES);
		strcpy(NavigationZoneArray[TotalNumberOfNavigationZones].name, tmpname);
		NavigationZoneArray[TotalNumberOfNavigationZones].type = type;
		NavigationZoneArray[TotalNumberOfNavigationZones].minx = minx;
		NavigationZoneArray[TotalNumberOfNavigationZones].miny = miny;
		NavigationZoneArray[TotalNumberOfNavigationZones].minz = minz;
		NavigationZoneArray[TotalNumberOfNavigationZones].maxx = maxx;
		NavigationZoneArray[TotalNumberOfNavigationZones].maxy = maxy;
		NavigationZoneArray[TotalNumberOfNavigationZones].maxz = maxz;
		NavigationZoneArray[TotalNumberOfNavigationZones].level = level;
		TotalNumberOfNavigationZones++;
		break;
	case ZONE_INFO:
		assert(TotalNumberOfInfoZones < NUMINFOZONES);
		strcpy(InfoZoneArray[TotalNumberOfInfoZones].name, tmpname);
		InfoZoneArray[TotalNumberOfInfoZones].type = type;
		InfoZoneArray[TotalNumberOfInfoZones].minx = minx;
		InfoZoneArray[TotalNumberOfInfoZones].miny = miny;
		InfoZoneArray[TotalNumberOfInfoZones].minz = minz;
		InfoZoneArray[TotalNumberOfInfoZones].maxx = maxx;
		InfoZoneArray[TotalNumberOfInfoZones].maxy = maxy;
		InfoZoneArray[TotalNumberOfInfoZones].maxz = maxz;
		InfoZoneArray[TotalNumberOfInfoZones].level = level;
		InfoZoneArray[TotalNumberOfInfoZones].zoneinfoDay = TotalNumberOfZoneInfos++;
		InfoZoneArray[TotalNumberOfInfoZones].zoneinfoNight = TotalNumberOfZoneInfos++;
		TotalNumberOfInfoZones++;
		break;
	case ZONE_MAPZONE:
		assert(TotalNumberOfMapZones < NUMMAPZONES);
		strcpy(MapZoneArray[TotalNumberOfMapZones].name, tmpname);
		MapZoneArray[TotalNumberOfMapZones].type = type;
		MapZoneArray[TotalNumberOfMapZones].minx = minx;
		MapZoneArray[TotalNumberOfMapZones].miny = miny;
		MapZoneArray[TotalNumberOfMapZones].minz = minz;
		MapZoneArray[TotalNumberOfMapZones].maxx = maxx;
		MapZoneArray[TotalNumberOfMapZones].maxy = maxy;
		MapZoneArray[TotalNumberOfMapZones].maxz = maxz;
		MapZoneArray[TotalNumberOfMapZones].level = level;
		TotalNumberOfMapZones++;
		break;
	}
}

//--MIAMI: done
void
CTheZones::PostZoneCreation(void)
{
	int i;
	for(i = 1; i < TotalNumberOfNavigationZones; i++)
		InsertZoneIntoZoneHierarchy(&NavigationZoneArray[i]);
	InitialiseAudioZoneArray();
}

//--MIAMI: done, but does nothing
void
CTheZones::CheckZonesForOverlap(void)
{
	int i, j;
	char str[116];

	for(i = 1; i < TotalNumberOfInfoZones; i++){
		ZoneIsEntirelyContainedWithinOtherZone(&InfoZoneArray[i], &InfoZoneArray[0]);
		
		for(j = 1; j < TotalNumberOfInfoZones; j++)
			if(i != j && ZoneIsEntirelyContainedWithinOtherZone(&InfoZoneArray[i], &InfoZoneArray[j]))
				sprintf(str, "Info zone %s contains %s\n",
						&InfoZoneArray[j].name, &InfoZoneArray[i].name);
	}
}

//--MIAMI: done
void
CTheZones::InsertZoneIntoZoneHierarchy(CZone *zone)
{
	zone->child = nil;
	zone->parent = nil;
	zone->next = nil;
	InsertZoneIntoZoneHierRecursive(zone, &NavigationZoneArray[0]);
}

//--MIAMI: done
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

//--MIAMI: done
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

//--MIAMI: done
bool
CTheZones::PointLiesWithinZone(const CVector *v, CZone *zone)
{
	return zone->minx <= v->x && v->x <= zone->maxx &&
	       zone->miny <= v->y && v->y <= zone->maxy &&
	       zone->minz <= v->z && v->z <= zone->maxz;
}

//--MIAMI: done
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

//--MIAMI: done
CZone*
CTheZones::FindInformationZoneForPosition(const CVector *v)
{
	int i;
//	char tmp[116];
//	if(!PointLiesWithinZone(v, &InfoZoneArray[0]))
//		sprintf(tmp, "x = %.3f y= %.3f z = %.3f\n", v.x, v.y, v.z);
	for(i = 1; i < TotalNumberOfInfoZones; i++)
		if(PointLiesWithinZone(v, &InfoZoneArray[i]))
			return &InfoZoneArray[i];
	return &InfoZoneArray[0];
}

//--MIAMI: done
CZone*
CTheZones::FindSmallestNavigationZoneForPosition(const CVector *v, bool findDefault, bool findNavig)
{
	CZone *best = nil;
	if(findDefault && NavigationZoneArray[0].type == ZONE_DEFAULT ||
	   findNavig && NavigationZoneArray[0].type == ZONE_NAVIG)
		best = &NavigationZoneArray[0];
	// zone to test next
	CZone *zone = NavigationZoneArray[0].child;
	while(zone)
		// if in zone, descent into children
		if(PointLiesWithinZone(v, zone)){
			if(findDefault && zone->type == ZONE_DEFAULT ||
			   findNavig && zone->type == ZONE_NAVIG)
				best = zone;
			zone = zone->child;
		// otherwise try next zone
		}else
			zone = zone->next;
	return best;
}

//--MIAMI: done
int16
CTheZones::FindZoneByLabelAndReturnIndex(char *name, eZoneType type)
{
	char str[8];
	memset(str, 0, 8);
	strncpy(str, name, 8);
	switch(type){
	case ZONE_DEFAULT:
	case ZONE_NAVIG:
		for(FindIndex = 0; FindIndex < TotalNumberOfNavigationZones; FindIndex++)
			if(strcmp(GetNavigationZone(FindIndex)->name, name) == 0)
				return FindIndex;
		break;

	case ZONE_INFO:
		for(FindIndex = 0; FindIndex < TotalNumberOfInfoZones; FindIndex++)
			if(strcmp(GetInfoZone(FindIndex)->name, name) == 0)
				return FindIndex;
		break;

	case ZONE_MAPZONE:
		for(FindIndex = 0; FindIndex < TotalNumberOfMapZones; FindIndex++)
			if(strcmp(GetMapZone(FindIndex)->name, name) == 0)
				return FindIndex;
		break;
	}
	return -1;
}

//--MIAMI: done
int16
CTheZones::FindNextZoneByLabelAndReturnIndex(char *name, eZoneType type)
{
	char str[8];
	++FindIndex;
	memset(str, 0, 8);
	strncpy(str, name, 8);
	switch(type){
	case ZONE_DEFAULT:
	case ZONE_NAVIG:
		for(; FindIndex < TotalNumberOfNavigationZones; FindIndex++)
			if(strcmp(GetNavigationZone(FindIndex)->name, name) == 0)
				return FindIndex;
		break;

	case ZONE_INFO:
		for(; FindIndex < TotalNumberOfInfoZones; FindIndex++)
			if(strcmp(GetInfoZone(FindIndex)->name, name) == 0)
				return FindIndex;
		break;

	case ZONE_MAPZONE:
		for(; FindIndex < TotalNumberOfMapZones; FindIndex++)
			if(strcmp(GetMapZone(FindIndex)->name, name) == 0)
				return FindIndex;
		break;
	}
	return -1;
}

//--MIAMI: done
CZoneInfo*
CTheZones::GetZoneInfo(const CVector *v, uint8 day)
{
	CZone *zone;
	zone = FindInformationZoneForPosition(v);
	if(zone == nil)
		return &ZoneInfoArray[0];
	return &ZoneInfoArray[day ? zone->zoneinfoDay : zone->zoneinfoNight];
}

void
CTheZones::GetZoneInfoForTimeOfDay(const CVector *pos, CZoneInfo *info)
{
	CZoneInfo *day, *night;
	float d, n;
	int i;

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
		for(i = 0; i < ARRAY_SIZE(info->carThreshold); i++)
			info->carThreshold[i] = day->carThreshold[i] * d + night->carThreshold[i] * n;
		for(i = 0; i < ARRAY_SIZE(info->boatThreshold); i++)
			info->boatThreshold[i] = day->boatThreshold[i] * d + night->boatThreshold[i] * n;
		for(i = 0; i < ARRAY_SIZE(info->gangThreshold); i++)
			info->gangThreshold[i] = day->gangThreshold[i] * d + night->gangThreshold[i] * n;

		info->copThreshold = day->copThreshold * d + night->copThreshold * n;
		info->pedDensity = day->pedDensity * d + night->pedDensity * n;
		info->copPedThreshold = day->copPedThreshold * d + night->copPedThreshold * n;
		for(i = 0; i < ARRAY_SIZE(info->gangPedThreshold); i++)
			info->gangPedThreshold[i] = day->gangPedThreshold[i] * d + night->gangPedThreshold[i] * n;
	}
	if(CClock::GetIsTimeInRange(5, 19))
		info->pedGroup = day->pedGroup;
	else
		info->pedGroup = night->pedGroup;
}

void
CTheZones::SetZoneCarInfo(uint16 zoneid, uint8 day, int16 carDensity,
	int16 copCarDensity, const int16 *gangCarDensities)
{
	CZone *zone;
	CZoneInfo *info;
	zone = GetInfoZone(zoneid);
	info = &ZoneInfoArray[day ? zone->zoneinfoDay : zone->zoneinfoNight];

	info->carDensity = carDensity;
	info->copThreshold = copCarDensity;
	info->gangThreshold[0] = gangCarDensities[0] + copCarDensity;
	info->gangThreshold[1] = gangCarDensities[1] + info->gangThreshold[0];
	info->gangThreshold[2] = gangCarDensities[2] + info->gangThreshold[1];
	info->gangThreshold[3] = gangCarDensities[3] + info->gangThreshold[2];
	info->gangThreshold[4] = gangCarDensities[4] + info->gangThreshold[3];
	info->gangThreshold[5] = gangCarDensities[5] + info->gangThreshold[4];
	info->gangThreshold[6] = gangCarDensities[6] + info->gangThreshold[5];
	info->gangThreshold[7] = gangCarDensities[7] + info->gangThreshold[6];
	info->gangThreshold[8] = gangCarDensities[8] + info->gangThreshold[7];
}

void CTheZones::SetZoneCivilianCarInfo(uint16 zoneid, uint8 day,
	const int16* carDensities, const int16* boatDensities)
{
	CZone* zone;
	CZoneInfo* info;
	zone = GetInfoZone(zoneid);
	info = &ZoneInfoArray[day ? zone->zoneinfoDay : zone->zoneinfoNight];
	info->carThreshold[0] = carDensities[0];
	for (int i = 1; i < CCarCtrl::NUM_CAR_CLASSES; i++)
		info->carThreshold[i] = carDensities[i] + info->carThreshold[i-1];
	info->boatThreshold[0] = boatDensities[0];
	for (int i = 1; i < CCarCtrl::NUM_BOAT_CLASSES; i++)
		info->boatThreshold[i] = boatDensities[i] + info->boatThreshold[i - 1];
}

void
CTheZones::SetZonePedInfo(uint16 zoneid, uint8 day, int16 pedDensity,
	int16 gang0Density, int16 gang1Density, int16 gang2Density, int16 gang3Density,
	int16 gang4Density, int16 gang5Density, int16 gang6Density, int16 gang7Density,
	int16 gang8Density, int16 copDensity)
{
	CZone *zone;
	CZoneInfo *info;
	zone = GetInfoZone(zoneid);
	info = &ZoneInfoArray[day ? zone->zoneinfoDay : zone->zoneinfoNight];
	info->pedDensity = pedDensity;
	info->copPedThreshold = copDensity;
	info->gangPedThreshold[0] = gang0Density;
	info->gangPedThreshold[1] = gang1Density;
	info->gangPedThreshold[2] = gang2Density;
	info->gangPedThreshold[3] = gang3Density;
	info->gangPedThreshold[4] = gang4Density;
	info->gangPedThreshold[5] = gang5Density;
	info->gangPedThreshold[6] = gang6Density;
	info->gangPedThreshold[7] = gang7Density;
	info->gangPedThreshold[8] = gang8Density;

	info->gangPedThreshold[0] += info->copPedThreshold;
	info->gangPedThreshold[1] = info->gangPedThreshold[0];
	info->gangPedThreshold[2] = info->gangPedThreshold[1];
	info->gangPedThreshold[3] = info->gangPedThreshold[2];
	info->gangPedThreshold[4] = info->gangPedThreshold[3];
	info->gangPedThreshold[5] = info->gangPedThreshold[4];
	info->gangPedThreshold[6] = info->gangPedThreshold[5];
	info->gangPedThreshold[7] = info->gangPedThreshold[6];
	info->gangPedThreshold[8] = info->gangPedThreshold[7];
}

//--MIAMI: done, unused
void
CTheZones::SetCarDensity(uint16 zoneid, uint8 day, uint16 cardensity)
{
	CZone *zone;
	zone = GetInfoZone(zoneid);
	ZoneInfoArray[day ? zone->zoneinfoDay : zone->zoneinfoNight].carDensity = cardensity;
}

//--MIAMI: done, unused
void
CTheZones::SetPedDensity(uint16 zoneid, uint8 day, uint16 peddensity)
{
	CZone *zone;
	zone = GetInfoZone(zoneid);
	ZoneInfoArray[day ? zone->zoneinfoDay : zone->zoneinfoNight].pedDensity = peddensity;
}

//--MIAMI: done
void
CTheZones::SetPedGroup(uint16 zoneid, uint8 day, uint16 pedgroup)
{
	CZone *zone;
	zone = GetInfoZone(zoneid);
	ZoneInfoArray[day ? zone->zoneinfoDay : zone->zoneinfoNight].pedGroup = pedgroup;
}

//--MIAMI: done
int16
CTheZones::FindAudioZone(CVector *pos)
{
	int i;

	for(i = 0; i < NumberOfAudioZones; i++)
		if(PointLiesWithinZone(pos, GetAudioZone(i)))
			return i;
	return -1;
}

//--MIAMI: done
void
CTheZones::AddZoneToAudioZoneArray(CZone *zone)
{
	int i, z;

	if(zone->type != ZONE_DEFAULT)
		return;

	/* This is a bit stupid */
	z = -1;
	for(i = 0; i < NUMNAVIGZONES; i++)
		if(&NavigationZoneArray[i] == zone)
			z = i;
	assert(NumberOfAudioZones < NUMAUDIOZONES);
	AudioZoneArray[NumberOfAudioZones++] = z;
}

//--MIAMI: done
void
CTheZones::InitialiseAudioZoneArray(void)
{
	bool gonext;
	CZone *zone;

	gonext = false;
	zone = &NavigationZoneArray[0];
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

//--MIAMI: TODO
void
CTheZones::SaveAllZones(uint8 *buffer, uint32 *size)
{
	INITSAVEBUF
	int i;

	*size = SAVE_HEADER_SIZE
		+ sizeof(m_CurrLevel) + sizeof(FindIndex)
		+ sizeof(int16) // padding
		+ sizeof(NavigationZoneArray) + sizeof(InfoZoneArray) + sizeof(ZoneInfoArray)
		+ sizeof(TotalNumberOfNavigationZones) + sizeof(TotalNumberOfInfoZones) + sizeof(TotalNumberOfZoneInfos)
		+ sizeof(MapZoneArray) + sizeof(AudioZoneArray)
		+ sizeof(TotalNumberOfMapZones) + sizeof(NumberOfAudioZones);

	WriteSaveHeader(buffer, 'Z', 'N', 'S', '\0', *size - SAVE_HEADER_SIZE);

	WriteSaveBuf(buffer, m_CurrLevel);
	WriteSaveBuf(buffer, FindIndex);
	WriteSaveBuf(buffer, (int16)0); // padding

// TODO(MIAMI): implement SaveOneZone
	for(i = 0; i < ARRAY_SIZE(NavigationZoneArray); i++){
		CZone *zone = WriteSaveBuf(buffer, NavigationZoneArray[i]);
		zone->child = (CZone*)GetIndexForZonePointer(NavigationZoneArray[i].child);
		zone->parent = (CZone*)GetIndexForZonePointer(NavigationZoneArray[i].parent);
		zone->next = (CZone*)GetIndexForZonePointer(NavigationZoneArray[i].next);
	}

	for(i = 0; i < ARRAY_SIZE(InfoZoneArray); i++){
		CZone *zone = WriteSaveBuf(buffer, InfoZoneArray[i]);
		zone->child = (CZone*)GetIndexForZonePointer(InfoZoneArray[i].child);
		zone->parent = (CZone*)GetIndexForZonePointer(InfoZoneArray[i].parent);
		zone->next = (CZone*)GetIndexForZonePointer(InfoZoneArray[i].next);
	}

	for(i = 0; i < ARRAY_SIZE(ZoneInfoArray); i++)
		WriteSaveBuf(buffer, ZoneInfoArray[i]);

	WriteSaveBuf(buffer, TotalNumberOfNavigationZones);
	WriteSaveBuf(buffer, TotalNumberOfInfoZones);
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

//--MIAMI: TODO
void
CTheZones::LoadAllZones(uint8 *buffer, uint32 size)
{
	INITSAVEBUF
	int i;

	CheckSaveHeader(buffer, 'Z', 'N', 'S', '\0', size - SAVE_HEADER_SIZE);

	m_CurrLevel = ReadSaveBuf<eLevelName>(buffer);
	FindIndex = ReadSaveBuf<int16>(buffer);
	ReadSaveBuf<int16>(buffer);

// TODO(MIAMI): implement LoadOneZone
	for(i = 0; i < ARRAY_SIZE(NavigationZoneArray); i++){
		NavigationZoneArray[i] = ReadSaveBuf<CZone>(buffer);

		NavigationZoneArray[i].child = GetPointerForZoneIndex((int32)NavigationZoneArray[i].child);
		NavigationZoneArray[i].parent = GetPointerForZoneIndex((int32)NavigationZoneArray[i].parent);
		NavigationZoneArray[i].next = GetPointerForZoneIndex((int32)NavigationZoneArray[i].next);
	}

	for(i = 0; i < ARRAY_SIZE(InfoZoneArray); i++){
		InfoZoneArray[i] = ReadSaveBuf<CZone>(buffer);

		InfoZoneArray[i].child = GetPointerForZoneIndex((int32)InfoZoneArray[i].child);
		InfoZoneArray[i].parent = GetPointerForZoneIndex((int32)InfoZoneArray[i].parent);
		InfoZoneArray[i].next = GetPointerForZoneIndex((int32)InfoZoneArray[i].next);
	}

	for(i = 0; i < ARRAY_SIZE(ZoneInfoArray); i++)
		ZoneInfoArray[i] = ReadSaveBuf<CZoneInfo>(buffer);

	TotalNumberOfNavigationZones = ReadSaveBuf<int16>(buffer);
	TotalNumberOfInfoZones = ReadSaveBuf<int16>(buffer);
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
