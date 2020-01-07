#include "common.h"
#include "patcher.h"
#include "Building.h"
#include "Treadable.h"
#include "Train.h"
#include "Pools.h"
#include "Timer.h"
#include "Camera.h"
#include "World.h"
#include "FileMgr.h"
#include "ZoneCull.h"

int32     &CCullZones::NumCullZones = *(int*)0x8F2564;
CCullZone *CCullZones::aZones = (CCullZone*)0x864750;	// [NUMCULLZONES];
int32     &CCullZones::NumAttributeZones = *(int*)0x8E29D0;
CAttributeZone *CCullZones::aAttributeZones = (CAttributeZone*)0x709C60;	// [NUMATTRIBZONES];
uint16    *CCullZones::aIndices = (uint16*)0x847330;	// [NUMZONEINDICES];
int16     *CCullZones::aPointersToBigBuildingsForBuildings = (int16*)0x86C9D0;	// [NUMBUILDINGS];
int16     *CCullZones::aPointersToBigBuildingsForTreadables = (int16*)0x8F1B8C;	// [NUMTREADABLES];

int32 &CCullZones::CurrentWantedLevelDrop_Player = *(int32*)0x880DA8;
int32 &CCullZones::CurrentFlags_Camera = *(int32*)0x940718;
int32 &CCullZones::CurrentFlags_Player = *(int32*)0x9415F0;
int32 &CCullZones::OldCullZone = *(int32*)0x8E2C90;
int32 &CCullZones::EntityIndicesUsed = *(int32*)0x8F2508;
bool &CCullZones::bCurrentSubwayIsInvisible = *(bool*)0x95CDA5;
bool &CCullZones::bCullZonesDisabled = *(bool*)0x95CD4A;


void
CCullZones::Init(void)
{
	int i;

	NumAttributeZones = 0;
	NumCullZones = 0;
	CurrentWantedLevelDrop_Player = 0;
	CurrentFlags_Camera = 0;
	CurrentFlags_Player = 0;
	OldCullZone = -1;
	EntityIndicesUsed = 0;
	bCurrentSubwayIsInvisible = false;

	for(i = 0; i < NUMBUILDINGS; i++)
		aPointersToBigBuildingsForBuildings[i] = -1;
	for(i = 0; i < NUMTREADABLES; i++)
		aPointersToBigBuildingsForTreadables[i] = -1;
}

void
CCullZones::ResolveVisibilities(void)
{
	int fd;

	CFileMgr::SetDir("");
	fd = CFileMgr::OpenFile("DATA\\cullzone.dat", "rb");
	if(fd > 0){
		CFileMgr::Read(fd, (char*)&NumCullZones, 4);
		CFileMgr::Read(fd, (char*)aZones, NUMCULLZONES*sizeof(CCullZone));
		CFileMgr::Read(fd, (char*)&NumAttributeZones, 4);
		CFileMgr::Read(fd, (char*)aAttributeZones, NUMATTRIBZONES*sizeof(CAttributeZone));
		CFileMgr::Read(fd, (char*)aIndices, NUMZONEINDICES*2);
		CFileMgr::Read(fd, (char*)aPointersToBigBuildingsForBuildings, NUMBUILDINGS*2);
		CFileMgr::Read(fd, (char*)aPointersToBigBuildingsForTreadables, NUMTREADABLES*2);
		CFileMgr::CloseFile(fd);
	}else{
		// TODO: implement code from mobile to generate data here
	}
}

void
CCullZones::Update(void)
{
	bool invisible;

	if(bCullZonesDisabled)
		return;

	switch(CTimer::GetFrameCounter() & 7){
	case 0:
	case 4:
		/* Update Cull zone */
		ForceCullZoneCoors(TheCamera.GetGameCamPosition());
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

void
CCullZones::ForceCullZoneCoors(CVector coors)
{
	int32 z;
	z = FindCullZoneForCoors(coors);
	if(z != OldCullZone){
		if(OldCullZone >= 0)
			aZones[OldCullZone].DoStuffLeavingZone();
		if(z >= 0)
			aZones[z].DoStuffEnteringZone();
		OldCullZone = z;
	}
}

int32
CCullZones::FindCullZoneForCoors(CVector coors)
{
	int i;

	for(i = 0; i < NumCullZones; i++)
		if(coors.x >= aZones[i].minx && coors.x <= aZones[i].maxx &&
		   coors.y >= aZones[i].miny && coors.y <= aZones[i].maxy &&
		   coors.z >= aZones[i].minz && coors.z <= aZones[i].maxz)
			return i;
	return -1;
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
				*wantedLevel = max(*wantedLevel, aAttributeZones[i].wantedLevel);
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
	CCullZone *cull;
	CAttributeZone *attrib;

	CVector v;
	if((flag & ATTRZONE_NOTCULLZONE) == 0){
		cull = &aZones[NumCullZones++];
		v = position;
		// WTF is this?
		if((v-CVector(1032.14f, -624.255f, 24.93f)).Magnitude() < 1.0f)
			v = CVector(1061.7f, -613.0f, 19.0f);
		if((v-CVector(1029.48f, -495.757f, 21.98f)).Magnitude() < 1.0f)
			v = CVector(1061.4f, -506.0f, 18.5f);
		cull->position.x = clamp(v.x, minx, maxx);
		cull->position.y = clamp(v.y, miny, maxy);
		cull->position.z = clamp(v.z, minz, maxz);
		cull->minx = minx;
		cull->maxx = maxx;
		cull->miny = miny;
		cull->maxy = maxy;
		cull->minz = minz;
		cull->maxz = maxz;
		cull->unk2 = 0;
		cull->unk3 = 0;
		cull->unk4 = 0;
		cull->m_indexStart = 0;
	}
	if(flag & ~ATTRZONE_NOTCULLZONE){
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
}



void
CCullZone::DoStuffLeavingZone(void)
{
	int i;

	for(i = 0; i < m_numBuildings; i++)
		DoStuffLeavingZone_OneBuilding(CCullZones::aIndices[m_indexStart + i]);
	for(; i < m_numBuildings + m_numTreadablesPlus10m + m_numTreadables ; i++)
		DoStuffLeavingZone_OneTreadableBoth(CCullZones::aIndices[m_indexStart + i]);
}

void
CCullZone::DoStuffLeavingZone_OneBuilding(uint16 i)
{
	int16 bb;
	int j;

	if(i < 6000){
		CPools::GetBuildingPool()->GetSlot(i)->bZoneCulled = false;
		bb = CCullZones::aPointersToBigBuildingsForBuildings[i];
		if(bb != -1)
			CPools::GetBuildingPool()->GetSlot(bb)->bZoneCulled = false;
	}else{
		i -= 6000;
		for(j = 0; j < 3; j++)
			DoStuffLeavingZone_OneBuilding(CCullZones::aIndices[i+j]);
	}
}

void
CCullZone::DoStuffLeavingZone_OneTreadableBoth(uint16 i)
{
	int16 bb;
	int j;

	if(i < 6000){
		CPools::GetTreadablePool()->GetSlot(i)->bZoneCulled = false;
		CPools::GetTreadablePool()->GetSlot(i)->bZoneCulled2 = false;
		bb = CCullZones::aPointersToBigBuildingsForTreadables[i];
		if(bb != -1)
			CPools::GetBuildingPool()->GetSlot(bb)->bZoneCulled = false;
	}else{
		i -= 6000;
		for(j = 0; j < 3; j++)
			DoStuffLeavingZone_OneTreadableBoth(CCullZones::aIndices[i+j]);
	}
}

void
CCullZone::DoStuffEnteringZone(void)
{
	int i;

	for(i = 0; i < m_numBuildings; i++)
		DoStuffEnteringZone_OneBuilding(CCullZones::aIndices[m_indexStart + i]);
	for(; i < m_numBuildings + m_numTreadablesPlus10m; i++)
		DoStuffEnteringZone_OneTreadablePlus10m(CCullZones::aIndices[m_indexStart + i]);
	for(; i < m_numBuildings + m_numTreadablesPlus10m + m_numTreadables; i++)
		DoStuffEnteringZone_OneTreadable(CCullZones::aIndices[m_indexStart + i]);
}

void
CCullZone::DoStuffEnteringZone_OneBuilding(uint16 i)
{
	int16 bb;
	int j;

	if(i < 6000){
		CPools::GetBuildingPool()->GetSlot(i)->bZoneCulled = true;
		bb = CCullZones::aPointersToBigBuildingsForBuildings[i];
		if(bb != -1)
			CPools::GetBuildingPool()->GetSlot(bb)->bZoneCulled = true;
	}else{
		i -= 6000;
		for(j = 0; j < 3; j++)
			DoStuffLeavingZone_OneBuilding(CCullZones::aIndices[i+j]);
	}
}

void
CCullZone::DoStuffEnteringZone_OneTreadablePlus10m(uint16 i)
{
	int16 bb;
	int j;

	if(i < 6000){
		CPools::GetTreadablePool()->GetSlot(i)->bZoneCulled = true;;
		CPools::GetTreadablePool()->GetSlot(i)->bZoneCulled2 = true;;
		bb = CCullZones::aPointersToBigBuildingsForTreadables[i];
		if(bb != -1)
			CPools::GetBuildingPool()->GetSlot(bb)->bZoneCulled = true;
	}else{
		i -= 6000;
		for(j = 0; j < 3; j++)
			DoStuffLeavingZone_OneBuilding(CCullZones::aIndices[i+j]);
	}
}

void
CCullZone::DoStuffEnteringZone_OneTreadable(uint16 i)
{
	int16 bb;
	int j;

	if(i < 6000){
		CPools::GetTreadablePool()->GetSlot(i)->bZoneCulled = true;;
		bb = CCullZones::aPointersToBigBuildingsForTreadables[i];
		if(bb != -1)
			CPools::GetBuildingPool()->GetSlot(bb)->bZoneCulled = true;
	}else{
		i -= 6000;
		for(j = 0; j < 3; j++)
			DoStuffLeavingZone_OneBuilding(CCullZones::aIndices[i+j]);
	}
}

STARTPATCHES
	InjectHook(0x524BC0, &CCullZones::Init, PATCH_JUMP);
	InjectHook(0x524EC0, &CCullZones::ResolveVisibilities, PATCH_JUMP);
	InjectHook(0x524F80, &CCullZones::Update, PATCH_JUMP);
	InjectHook(0x525370, &CCullZones::AddCullZone, PATCH_JUMP);
	InjectHook(0x5250D0, &CCullZones::ForceCullZoneCoors, PATCH_JUMP);
	InjectHook(0x525130, &CCullZones::FindCullZoneForCoors, PATCH_JUMP);
	InjectHook(0x5251C0, &CCullZones::FindAttributesForCoors, PATCH_JUMP);
	InjectHook(0x525290, &CCullZones::FindZoneWithStairsAttributeForPlayer, PATCH_JUMP);

	InjectHook(0x525610, &CCullZone::DoStuffLeavingZone, PATCH_JUMP);
	InjectHook(0x525810, &CCullZone::DoStuffEnteringZone, PATCH_JUMP);
ENDPATCHES
