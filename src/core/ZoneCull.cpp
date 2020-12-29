#include "common.h"

#include "vehicle_enums.h" //needs to be before Vehicle.h

#include "Vehicle.h"

#include "Building.h"
#include "Camera.h"
#include "FileMgr.h"
#include "Pools.h"
#include "Timer.h"
#include "Train.h"
#include "Treadable.h"
#include "World.h"
#include "ZoneCull.h"
#include "Zones.h"

//--MIAMI: done

int32     CCullZones::NumAttributeZones;
CAttributeZone CCullZones::aAttributeZones[NUMATTRIBZONES];

int32 CCullZones::CurrentWantedLevelDrop_Player;
int32 CCullZones::CurrentFlags_Camera;
int32 CCullZones::CurrentFlags_Player;
bool CCullZones::bCurrentSubwayIsInvisible;
bool CCullZones::bAtBeachForAudio;

void
CCullZones::Init(void)
{
	NumAttributeZones = 0;
	CurrentWantedLevelDrop_Player = 0;
	CurrentFlags_Camera = 0;
	CurrentFlags_Player = 0;
	bCurrentSubwayIsInvisible = false;
}

void
CCullZones::Update(void)
{
	bool invisible;

	switch(CTimer::GetFrameCounter() & 7){
	case 0:
	case 4:
		UpdateAtBeachForAudio();
		break;

	case 2:
		/* Update camera attributes */
		CurrentFlags_Camera = FindAttributesForCoors(TheCamera.GetGameCamPosition(), nil);
		invisible = (CurrentFlags_Camera & ATTRZONE_SUBWAYVISIBLE) == 0;
		if(invisible != bCurrentSubwayIsInvisible){
			MarkSubwayAsInvisible(!invisible);
			bCurrentSubwayIsInvisible = invisible;
		}
		break;

	case 6:
		/* Update player attributes */
		CurrentFlags_Player = FindAttributesForCoors(FindPlayerCoors(),
			&CurrentWantedLevelDrop_Player);
		break;
	}
}

// TODO? put somewhere else?
bool
IsPointWithinArbitraryArea(float px, float py, float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4)
{
	if((px-x1)*(x2-x1) - (py-y1)*(y2-y1) < 0.0f) return false;
	if((px-x2)*(x3-x2) - (py-y2)*(y3-y2) < 0.0f) return false;
	if((px-x3)*(x4-x3) - (py-y3)*(y4-y3) < 0.0f) return false;
	if((px-x4)*(x1-x4) - (py-y4)*(y1-y4) < 0.0f) return false;
	return true;
}

void
CCullZones::UpdateAtBeachForAudio(void)
{
	bAtBeachForAudio = IsPointWithinArbitraryArea(TheCamera.GetPosition().x, TheCamera.GetPosition().y,
		400.0f, -1644.4f,
		751.9f, 1267.8f,
		971.9f, 1216.2f,
		840.0f, -1744.0f);
}

void
CCullZones::ForceCullZoneCoors(CVector coors)
{
}

int32
CCullZones::FindAttributesForCoors(CVector coors, int32 *wantedLevel)
{
	int i;
	int32 attribs;

	if (wantedLevel)
		*wantedLevel = 0;
	attribs = 0;
	for(i = 0; i < NumAttributeZones; i++)
		if(coors.x >= aAttributeZones[i].minx && coors.x <= aAttributeZones[i].maxx &&
		   coors.y >= aAttributeZones[i].miny && coors.y <= aAttributeZones[i].maxy &&
		   coors.z >= aAttributeZones[i].minz && coors.z <= aAttributeZones[i].maxz){
			attribs |= aAttributeZones[i].attributes;
			if(wantedLevel)
				*wantedLevel = Max(*wantedLevel, aAttributeZones[i].wantedLevel);
		}
	return attribs;
}

CAttributeZone*
CCullZones::FindZoneWithStairsAttributeForPlayer(void)
{
	int i;
	CVector coors;

	coors = FindPlayerCoors();
	for(i = 0; i < NumAttributeZones; i++)
		if(aAttributeZones[i].attributes & ATTRZONE_STAIRS &&
		   coors.x >= aAttributeZones[i].minx && coors.x <= aAttributeZones[i].maxx &&
		   coors.y >= aAttributeZones[i].miny && coors.y <= aAttributeZones[i].maxy &&
		   coors.z >= aAttributeZones[i].minz && coors.z <= aAttributeZones[i].maxz)
			return &aAttributeZones[i];
	return nil;
}

void
CCullZones::MarkSubwayAsInvisible(bool visible)
{
	int i, n;
	CEntity *e;
	CVehicle *v;

	n = CPools::GetBuildingPool()->GetSize()-1;
	for(i = n; i >= 0; i--){
		e = CPools::GetBuildingPool()->GetSlot(i);
		if(e && e->bIsSubway)
			e->bIsVisible = visible;
	}

	n = CPools::GetTreadablePool()->GetSize()-1;
	for(i = n; i >= 0; i--){
		e = CPools::GetTreadablePool()->GetSlot(i);
		if(e && e->bIsSubway)
			e->bIsVisible = visible;
	}

	n = CPools::GetVehiclePool()->GetSize()-1;
	for(i = n; i >= 0; i--){
		v = CPools::GetVehiclePool()->GetSlot(i);
		if(v && v->IsTrain() && ((CTrain*)v)->m_nTrackId != TRACK_ELTRAIN)
			v->bIsVisible = visible;
	}
}

void
CCullZones::AddCullZone(CVector const &position,
                        float minx, float maxx,
                        float miny, float maxy,
                        float minz, float maxz,
                        uint16 flag, int16 wantedLevel)
{
	CAttributeZone *attrib;

	assert(NumAttributeZones < NUMATTRIBZONES);
	attrib = &aAttributeZones[NumAttributeZones++];
	attrib->minx = minx;
	attrib->maxx = maxx;
	attrib->miny = miny;
	attrib->maxy = maxy;
	attrib->minz = minz;
	attrib->maxz = maxz;
	attrib->attributes = flag;
	attrib->wantedLevel = wantedLevel;
}
