#include "common.h"

#include "General.h"
#include "Building.h"
#include "Treadable.h"
#include "Train.h"
#include "Pools.h"
#include "Timer.h"
#include "Camera.h"
#include "World.h"
#include "FileMgr.h"
#include "ZoneCull.h"
#include "Zones.h"

#include "Debug.h"
#include "Renderer.h"

int32     CCullZones::NumCullZones;
CCullZone CCullZones::aZones[NUMCULLZONES];
int32     CCullZones::NumAttributeZones;
CAttributeZone CCullZones::aAttributeZones[NUMATTRIBZONES];
uint16    CCullZones::aIndices[NUMZONEINDICES];
int16     CCullZones::aPointersToBigBuildingsForBuildings[NUMBUILDINGS];
int16     CCullZones::aPointersToBigBuildingsForTreadables[NUMTREADABLES];

int32 CCullZones::CurrentWantedLevelDrop_Player;
int32 CCullZones::CurrentFlags_Camera;
int32 CCullZones::CurrentFlags_Player;
int32 CCullZones::OldCullZone;
int32 CCullZones::EntityIndicesUsed;
bool CCullZones::bCurrentSubwayIsInvisible;
bool CCullZones::bCullZonesDisabled;

#define NUMUNCOMPRESSED (6000)
#define NUMTEMPINDICES (140000)

void
CCullZones::Init(void)
{
	int i;

	NumAttributeZones = 0;
	CurrentWantedLevelDrop_Player = 0;
	CurrentFlags_Camera = 0;
	CurrentFlags_Player = 0;
	bCurrentSubwayIsInvisible = false;
	NumCullZones = 0;
	OldCullZone = -1;
	EntityIndicesUsed = 0;

	for(i = 0; i < NUMBUILDINGS; i++)
		aPointersToBigBuildingsForBuildings[i] = -1;
	for(i = 0; i < NUMTREADABLES; i++)
		aPointersToBigBuildingsForTreadables[i] = -1;
}


uint16* pTempArrayIndices;
int TempEntityIndicesUsed;

void
CCullZones::ResolveVisibilities(void)
{
	int fd;

	CFileMgr::SetDir("");
	fd = CFileMgr::OpenFile("DATA\\cullzone.dat", "rb");
	if(fd > 0){
		CFileMgr::Read(fd, (char*)&NumCullZones, sizeof(NumCullZones));
		CFileMgr::Read(fd, (char*)aZones, sizeof(aZones));
		CFileMgr::Read(fd, (char*)&NumAttributeZones, sizeof(NumAttributeZones));
		CFileMgr::Read(fd, (char*)aAttributeZones, sizeof(aAttributeZones));
		CFileMgr::Read(fd, (char*)aIndices, sizeof(aIndices));
		CFileMgr::Read(fd, (char*)aPointersToBigBuildingsForBuildings, sizeof(aPointersToBigBuildingsForBuildings));
		CFileMgr::Read(fd, (char*)aPointersToBigBuildingsForTreadables, sizeof(aPointersToBigBuildingsForTreadables));
		CFileMgr::CloseFile(fd);
	}else{
#ifndef MASTER
		EntityIndicesUsed = 0;
		BuildListForBigBuildings();
		pTempArrayIndices = new uint16[NUMTEMPINDICES];
		TempEntityIndicesUsed = 0;

//		if(!LoadTempFile())	// not in final game
		{
			for (int i = 0; i < NumCullZones; i++) {
//printf("testing zone %d (%d indices)\n", i, TempEntityIndicesUsed);
				DoVisibilityTestCullZone(i, true);
			}

//			SaveTempFile();	// not in final game
		}

		CompressIndicesArray();
		delete[] pTempArrayIndices;
		pTempArrayIndices = nil;

		fd = CFileMgr::OpenFileForWriting("data\\cullzone.dat");
		if (fd != 0) {
			CFileMgr::Write(fd, (char*)&NumCullZones, sizeof(NumCullZones));
			CFileMgr::Write(fd, (char*)aZones, sizeof(aZones));
			CFileMgr::Write(fd, (char*)&NumAttributeZones, sizeof(NumAttributeZones));
			CFileMgr::Write(fd, (char*)&aAttributeZones, sizeof(aAttributeZones));
			CFileMgr::Write(fd, (char*)&aIndices, sizeof(aIndices));
			CFileMgr::Write(fd, (char*)&aPointersToBigBuildingsForBuildings, sizeof(aPointersToBigBuildingsForBuildings));
			CFileMgr::Write(fd, (char*)&aPointersToBigBuildingsForTreadables, sizeof(aPointersToBigBuildingsForTreadables));
			CFileMgr::CloseFile(fd);
		}
#endif
	}
}

bool
CCullZones::LoadTempFile(void)
{
	int fd = CFileMgr::OpenFile("cullzone.tmp");
	if (fd != 0) {
		CFileMgr::Read(fd, (char*)&NumCullZones, sizeof(NumCullZones));
		CFileMgr::Read(fd, (char*)aZones, sizeof(aZones));
		CFileMgr::Read(fd, (char*)&NumAttributeZones, sizeof(NumAttributeZones));
		CFileMgr::Read(fd, (char*)&aAttributeZones, sizeof(aAttributeZones));
		CFileMgr::Read(fd, (char*)pTempArrayIndices, NUMTEMPINDICES*sizeof(uint16));
		CFileMgr::Read(fd, (char*)&TempEntityIndicesUsed, sizeof(TempEntityIndicesUsed));
		CFileMgr::Read(fd, (char*)&aPointersToBigBuildingsForBuildings, sizeof(aPointersToBigBuildingsForBuildings));
		CFileMgr::Read(fd, (char*)&aPointersToBigBuildingsForTreadables, sizeof(aPointersToBigBuildingsForTreadables));
		CFileMgr::CloseFile(fd);
		return true;
	}
	return false;
}

void
CCullZones::SaveTempFile(void)
{
	int fd = CFileMgr::OpenFileForWriting("cullzone.tmp");
	if (fd != 0) {
		CFileMgr::Write(fd, (char*)&NumCullZones, sizeof(NumCullZones));
		CFileMgr::Write(fd, (char*)aZones, sizeof(aZones));
		CFileMgr::Write(fd, (char*)&NumAttributeZones, sizeof(NumAttributeZones));
		CFileMgr::Write(fd, (char*)&aAttributeZones, sizeof(aAttributeZones));
		CFileMgr::Write(fd, (char*)pTempArrayIndices, NUMTEMPINDICES*sizeof(uint16));
		CFileMgr::Write(fd, (char*)&TempEntityIndicesUsed, sizeof(TempEntityIndicesUsed));
		CFileMgr::Write(fd, (char*)&aPointersToBigBuildingsForBuildings, sizeof(aPointersToBigBuildingsForBuildings));
		CFileMgr::Write(fd, (char*)&aPointersToBigBuildingsForTreadables, sizeof(aPointersToBigBuildingsForTreadables));
		CFileMgr::CloseFile(fd);
	}
}


void
CCullZones::BuildListForBigBuildings()
{
	for (int i = CPools::GetBuildingPool()->GetSize()-1; i >= 0; i--) {
		CBuilding *building = CPools::GetBuildingPool()->GetSlot(i);
		if (building == nil || !building->bIsBIGBuilding) continue;
		CSimpleModelInfo *nonlod = ((CSimpleModelInfo *)CModelInfo::GetModelInfo(building->GetModelIndex()))->GetRelatedModel();
		if (nonlod == nil) continue;

		for (int j = CPools::GetBuildingPool()->GetSize()-1; j >= 0; j--) {
			CBuilding *building2 = CPools::GetBuildingPool()->GetSlot(j);
			if (building2 == nil || building2->bIsBIGBuilding) continue;
			if (CModelInfo::GetModelInfo(building2->GetModelIndex()) == nonlod) {
				if ((building2->GetPosition() - building->GetPosition()).Magnitude() < 5.0f) {
					aPointersToBigBuildingsForBuildings[j] = i;
				}
			}
		}

		for (int j = CPools::GetTreadablePool()->GetSize()-1; j >= 0; j--) {
			CTreadable *treadable = CPools::GetTreadablePool()->GetSlot(j);
			if (treadable == nil || treadable->bIsBIGBuilding) continue;
			if (CModelInfo::GetModelInfo(treadable->GetModelIndex()) == nonlod) {
				if ((treadable->GetPosition() - building->GetPosition()).Magnitude() < 5.0f) {
					aPointersToBigBuildingsForTreadables[j] = i;
				}
			}
		}
	}
}

void
CCullZones::DoVisibilityTestCullZone(int zoneId, bool findIndices)
{
	aZones[zoneId].m_groupIndexCount[0] = 0;
	aZones[zoneId].m_groupIndexCount[1] = 0;
	aZones[zoneId].m_groupIndexCount[2] = 0;
	aZones[zoneId].m_indexStart = TempEntityIndicesUsed;
	aZones[zoneId].FindTestPoints();

	if (!findIndices) return;

	for (int i = CPools::GetBuildingPool()->GetSize() - 1; i >= 0; i--) {
		CBuilding *building = CPools::GetBuildingPool()->GetSlot(i);
		if (building != nil && !building->bIsBIGBuilding && aZones[zoneId].IsEntityCloseEnoughToZone(building, aPointersToBigBuildingsForBuildings[i] != -1)) {
			CBuilding *LODbuilding = nil;
			if (aPointersToBigBuildingsForBuildings[i] != -1)
				LODbuilding = CPools::GetBuildingPool()->GetSlot(aPointersToBigBuildingsForBuildings[i]);

			if (!aZones[zoneId].TestEntityVisibilityFromCullZone(building, 0.0f, LODbuilding)) {
				assert(TempEntityIndicesUsed < NUMTEMPINDICES);
				pTempArrayIndices[TempEntityIndicesUsed++] = i;
				aZones[zoneId].m_groupIndexCount[0]++;
			}
		}
	}

	for (int i = CPools::GetTreadablePool()->GetSize() - 1; i >= 0; i--) {
		CBuilding* building = CPools::GetTreadablePool()->GetSlot(i);
		if (building != nil && aZones[zoneId].IsEntityCloseEnoughToZone(building, aPointersToBigBuildingsForTreadables[i] != -1)) {
			CBuilding *LODbuilding = nil;
			if (aPointersToBigBuildingsForTreadables[i] != -1)
				LODbuilding = CPools::GetBuildingPool()->GetSlot(aPointersToBigBuildingsForTreadables[i]);

			if (!aZones[zoneId].TestEntityVisibilityFromCullZone(building, 10.0f, LODbuilding)) {
				assert(TempEntityIndicesUsed < NUMTEMPINDICES);
				pTempArrayIndices[TempEntityIndicesUsed++] = i;
				aZones[zoneId].m_groupIndexCount[1]++;
			}
		}
	}

	for (int i = CPools::GetTreadablePool()->GetSize() - 1; i >= 0; i--) {
		CBuilding *building = CPools::GetTreadablePool()->GetSlot(i);
		if (building != nil && aZones[zoneId].CalcDistToCullZoneSquared(building->GetPosition().x, building->GetPosition().y) < SQR(200.0f)) {
			int start = aZones[zoneId].m_groupIndexCount[0] + aZones[zoneId].m_indexStart;
			int end = aZones[zoneId].m_groupIndexCount[1] + start;

			bool alreadyAdded = false;

			for (int k = start; k < end; k++) {
#ifdef FIX_BUGS
				if (pTempArrayIndices[k] == i)
#else
				if (aIndices[k] == i)
#endif
					alreadyAdded = true;
			}

			if (!alreadyAdded) {
				CBuilding *LODbuilding = nil;
				if (aPointersToBigBuildingsForTreadables[i] != -1)
					LODbuilding = CPools::GetBuildingPool()->GetSlot(aPointersToBigBuildingsForTreadables[i]);
				if (!aZones[zoneId].TestEntityVisibilityFromCullZone(building, 0.0f, LODbuilding)) {
					assert(TempEntityIndicesUsed < NUMTEMPINDICES);
					pTempArrayIndices[TempEntityIndicesUsed++] = i;
					aZones[zoneId].m_groupIndexCount[2]++;
				}
			}
		}
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
	CCullZone *cull;
	CAttributeZone *attrib;

	CVector v;
	if((flag & ATTRZONE_NOTCULLZONE) == 0){
		cull = &aZones[NumCullZones++];
		v = position;
		// reposition start point to the start/end of the
		// alley next to the big building in the industrial district.
		// probably isn't analyzed correctly otherwise?s
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
		cull->m_groupIndexCount[0] = 0;
		cull->m_groupIndexCount[1] = 0;
		cull->m_groupIndexCount[2] = 0;
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

uint16 *pExtraArrayIndices;

void
CCullZones::CompressIndicesArray()
{
	uint16 set[3];

	// These are used to hold the compressed groups in sets of 3
	int numExtraIndices = 0;
	pExtraArrayIndices = new uint16[NUMTEMPINDICES];

	for(int numOccurrences = 6; numOccurrences > 1; numOccurrences--){
		if(NumCullZones == 0)
			break;

//printf("checking occurrences %d\n", numOccurrences);
		int attempt = 0;
		while(attempt < 10000){
			for(;;){
				attempt++;

				int zone = CGeneral::GetRandomNumber() % NumCullZones;
				int group = CGeneral::GetRandomNumber() % 3;
				if(!PickRandomSetForGroup(zone, group, set))
					break;
				if(!DoWeHaveMoreThanXOccurencesOfSet(numOccurrences, set))
					break;

				// add this set
				attempt = 1;
				int setId = numExtraIndices + NUMUNCOMPRESSED;
				pExtraArrayIndices[numExtraIndices++] = set[0];
				pExtraArrayIndices[numExtraIndices++] = set[1];
				pExtraArrayIndices[numExtraIndices++] = set[2];
				ReplaceSetForAllGroups(set, setId);
			}
		}
	}

	TidyUpAndMergeLists(pExtraArrayIndices, numExtraIndices);

	delete[] pExtraArrayIndices;
}

// Get three random indices for this group of a zone
bool
CCullZones::PickRandomSetForGroup(int32 zone, int32 group, uint16 *set)
{
	int32 start;
	int32 size;

	aZones[zone].GetGroupStartAndSize(group, start, size);
	if(size <= 0)
		return false;

	int numIndices = 0;
	for(int i = 0; i < size; i++)
		if(pTempArrayIndices[start + i] != 0xFFFF)
			numIndices++;
	if(numIndices < 3)
		return false;

	int first = CGeneral::GetRandomNumber() % (numIndices-2);

	numIndices = 0;
	int n = 0;
	for(int i = 0; i < size; i++)
		if(pTempArrayIndices[start + i] != 0xFFFF){
			if(n++ < first) continue;

			set[numIndices++] = pTempArrayIndices[start + i];
			if(numIndices == 3)
				break;
		}
	return true;
}

bool
CCullZones::DoWeHaveMoreThanXOccurencesOfSet(int32 count, uint16 *set)
{
	int32 curCount;
	int32 start;
	int32 size;

	curCount = 0;
	for (int i = 0; i < NumCullZones; i++) {
		for (int group = 0; group < 3; group++) {
			aZones[i].GetGroupStartAndSize(group, start, size);
			if(size <= 0) continue;

			// check if the set is a subset of the group
			int n = 0;
			for (int j = 0; j < size; j++) {
				for (int k = 0; k < 3; k++) {
					if (pTempArrayIndices[start+j] == set[k])
						n++;
				}
			}
			// yes it is
			if(n == 3){
				curCount++;
				// check if we have seen this set often enough
				if(curCount >= count)
					return true;
			}
		}
	}
	return false;
}

void
CCullZones::ReplaceSetForAllGroups(uint16 *set, uint16 setid)
{
	int32 start;
	int32 size;

	for(int i = 0; i < NumCullZones; i++)
		for(int group = 0; group < 3; group++){
			aZones[i].GetGroupStartAndSize(group, start, size);
			if(size <= 0) continue;

			// check if the set is a subset of the group
			int n = 0;
			for(int j = 0; j < size; j++){
				for(int k = 0; k < 3; k++){
					if(pTempArrayIndices[start+j] == set[k])
						n++;
				}
			}

			// yes it is, so replace it
			if(n == 3){
				bool insertedSet = false;
				for(int j = 0; j < size; j++){
					for(int k = 0; k < 3; k++){
						// replace first element by set, invalidate others
						if(pTempArrayIndices[start+j] == set[k]){
							if(!insertedSet)
								pTempArrayIndices[start+j] = setid;
							else
								pTempArrayIndices[start+j] = 0xFFFF;
							insertedSet = true;
						}
					}
				}
			}
		}
}

void
CCullZones::TidyUpAndMergeLists(uint16 *extraIndices, int32 numExtraIndices)
{
	int numTempIndices = 0;
	for(int i = 0; i < TempEntityIndicesUsed; i++)
		if(pTempArrayIndices[i] != 0xFFFF)
			numTempIndices++;

	// Fix up zone ranges such that there are no holes
	for(int i = 0; i < NumCullZones; i++){
		int j;
		int start = 0;
		for(j = 0; j < aZones[i].m_indexStart; j++)
			if(pTempArrayIndices[j] != 0xFFFF)
				start++;

		aZones[i].m_indexStart = start;
		aZones[i].m_numBuildings = 0;
		aZones[i].m_numTreadablesPlus10m = 0;
		aZones[i].m_numTreadables = 0;

		for(int k = 0; k < aZones[i].m_groupIndexCount[0]; k++)
			if(pTempArrayIndices[j++] != 0xFFFF)
				aZones[i].m_numBuildings++;
		for(int k = 0; k < aZones[i].m_groupIndexCount[1]; k++)
			if(pTempArrayIndices[j++] != 0xFFFF)
				aZones[i].m_numTreadablesPlus10m++;
		for(int k = 0; k < aZones[i].m_groupIndexCount[2]; k++)
			if(pTempArrayIndices[j++] != 0xFFFF)
				aZones[i].m_numTreadables++;
	}

	// Now copy the actually used indices
	EntityIndicesUsed = 0;
	for(int i = 0; i < TempEntityIndicesUsed; i++)
		if(pTempArrayIndices[i] != 0xFFFF){
			assert(EntityIndicesUsed < NUMZONEINDICES);
			if(pTempArrayIndices[i] < NUMUNCOMPRESSED)
				aIndices[EntityIndicesUsed++] = pTempArrayIndices[i];
			else
				aIndices[EntityIndicesUsed++] = pTempArrayIndices[i] + numTempIndices;
		}
	for(int i = 0; i < numExtraIndices; i++)
		if(extraIndices[i] != 0xFFFF){
			assert(EntityIndicesUsed < NUMZONEINDICES);
			if(extraIndices[i] < NUMUNCOMPRESSED)
				aIndices[EntityIndicesUsed++] = extraIndices[i];
			else
				aIndices[EntityIndicesUsed++] = extraIndices[i] + numTempIndices;
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


	if(i < NUMUNCOMPRESSED){
		CPools::GetBuildingPool()->GetSlot(i)->bZoneCulled = false;
		bb = CCullZones::aPointersToBigBuildingsForBuildings[i];
		if(bb != -1)
			CPools::GetBuildingPool()->GetSlot(bb)->bZoneCulled = false;
	}else{
		i -= NUMUNCOMPRESSED;
		for(j = 0; j < 3; j++)
			DoStuffLeavingZone_OneBuilding(CCullZones::aIndices[i+j]);
	}
}

void
CCullZone::DoStuffLeavingZone_OneTreadableBoth(uint16 i)
{
	int16 bb;
	int j;

	if(i < NUMUNCOMPRESSED){
		CPools::GetTreadablePool()->GetSlot(i)->bZoneCulled = false;
		CPools::GetTreadablePool()->GetSlot(i)->bZoneCulled2 = false;
		bb = CCullZones::aPointersToBigBuildingsForTreadables[i];
		if(bb != -1)
			CPools::GetBuildingPool()->GetSlot(bb)->bZoneCulled = false;
	}else{
		i -= NUMUNCOMPRESSED;
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

	if(i < NUMUNCOMPRESSED){
		CPools::GetBuildingPool()->GetSlot(i)->bZoneCulled = true;
		bb = CCullZones::aPointersToBigBuildingsForBuildings[i];
		if(bb != -1)
			CPools::GetBuildingPool()->GetSlot(bb)->bZoneCulled = true;
	}else{
		i -= NUMUNCOMPRESSED;
		for(j = 0; j < 3; j++)
			DoStuffEnteringZone_OneBuilding(CCullZones::aIndices[i+j]);
	}
}

void
CCullZone::DoStuffEnteringZone_OneTreadablePlus10m(uint16 i)
{
	int16 bb;
	int j;

	if(i < NUMUNCOMPRESSED){
		CPools::GetTreadablePool()->GetSlot(i)->bZoneCulled = true;
		CPools::GetTreadablePool()->GetSlot(i)->bZoneCulled2 = true;
		bb = CCullZones::aPointersToBigBuildingsForTreadables[i];
		if(bb != -1)
			CPools::GetBuildingPool()->GetSlot(bb)->bZoneCulled = true;
	}else{
		i -= NUMUNCOMPRESSED;
		for(j = 0; j < 3; j++)
			DoStuffEnteringZone_OneTreadablePlus10m(CCullZones::aIndices[i+j]);
	}
}

void
CCullZone::DoStuffEnteringZone_OneTreadable(uint16 i)
{
	int16 bb;
	int j;

	if(i < NUMUNCOMPRESSED){
		CPools::GetTreadablePool()->GetSlot(i)->bZoneCulled = true;
		bb = CCullZones::aPointersToBigBuildingsForTreadables[i];
		if(bb != -1)
			CPools::GetBuildingPool()->GetSlot(bb)->bZoneCulled = true;
	}else{
		i -= NUMUNCOMPRESSED;
		for(j = 0; j < 3; j++)
			DoStuffEnteringZone_OneTreadable(CCullZones::aIndices[i+j]);
	}
}

float
CCullZone::CalcDistToCullZoneSquared(float x, float y)
{
	float rx, ry;

	if (x < minx) rx = sq(x - minx);
	else if (x > maxx) rx = sq(x - maxx);
	else rx = 0.0f;

	if (y < miny) ry = sq(y - miny);
	else if (y > maxy) ry = sq(y - maxy);
	else ry = 0.0f;

	return rx + ry;
}

bool
CCullZone::TestLine(CVector vec1, CVector vec2)
{
	CColPoint colPoint;
	CEntity *entity;

	if (CWorld::ProcessLineOfSight(vec1, vec2, colPoint, entity, true, false, false, false, false, true, false))
		return true;
	if (CWorld::ProcessLineOfSight(CVector(vec1.x + 0.05f, vec1.y, vec1.z), CVector(vec2.x + 0.05f, vec2.y, vec2.z), colPoint, entity, true, false, false, false, false, true, false))
		return true;
	if (CWorld::ProcessLineOfSight(CVector(vec1.x - 0.05f, vec1.y, vec1.z), CVector(vec2.x - 0.05f, vec2.y, vec2.z), colPoint, entity, true, false, false, false, false, true, false))
		return true;
	if (CWorld::ProcessLineOfSight(CVector(vec1.x, vec1.y + 0.05f, vec1.z), CVector(vec2.x, vec2.y + 0.05f, vec2.z), colPoint, entity, true, false, false, false, false, true, false))
		return true;
	if (CWorld::ProcessLineOfSight(CVector(vec1.x, vec1.y - 0.05f, vec1.z), CVector(vec2.x, vec2.y - 0.05f, vec2.z), colPoint, entity, true, false, false, false, false, true, false))
		return true;
	if (CWorld::ProcessLineOfSight(CVector(vec1.x, vec1.y, vec1.z + 0.05f), CVector(vec2.x, vec2.y, vec2.z + 0.05f), colPoint, entity, true, false, false, false, false, true, false))
		return true;
	return CWorld::ProcessLineOfSight(CVector(vec1.x, vec1.y, vec1.z - 0.05f), CVector(vec2.x, vec2.y, vec2.z - 0.05f), colPoint, entity, true, false, false, false, false, true, false);
}

bool
CCullZone::DoThoroughLineTest(CVector start, CVector end, CEntity *testEntity)
{
	CColPoint colPoint;
	CEntity *entity;

	if(CWorld::ProcessLineOfSight(start, end, colPoint, entity, true, false, false, false, false, true, false) &&
	   testEntity != entity)
		return false;

	CVector side;
#ifdef FIX_BUGS
	if(start.x != end.x || start.y != end.y)
#else
	if(start.x != end.x && start.y != end.y)
#endif
		side = CVector(0.0f, 0.0f, 1.0f);
	else
		side = CVector(1.0f, 0.0f, 0.0f);
	CVector up = CrossProduct(side, end - start);
	side = CrossProduct(up, end - start);
	side.Normalise();
	up.Normalise();
	side *= 0.1f;
	up *= 0.1f;

	if(CWorld::ProcessLineOfSight(start+side, end+side, colPoint, entity, true, false, false, false, false, true, false) &&
	   testEntity != entity)
		return false;
	if(CWorld::ProcessLineOfSight(start-side, end-side, colPoint, entity, true, false, false, false, false, true, false) &&
	   testEntity != entity)
		return false;
	if(CWorld::ProcessLineOfSight(start+up, end+up, colPoint, entity, true, false, false, false, false, true, false) &&
	   testEntity != entity)
		return false;
	if(CWorld::ProcessLineOfSight(start-up, end-up, colPoint, entity, true, false, false, false, false, true, false) &&
	   testEntity != entity)
		return false;
	return true;
}

bool
CCullZone::IsEntityCloseEnoughToZone(CEntity *entity, bool checkLevel)
{
	const CVector &pos = entity->GetPosition();

	CSimpleModelInfo *minfo = (CSimpleModelInfo*)CModelInfo::GetModelInfo(entity->GetModelIndex());
	float distToZone = CalcDistToCullZone(pos.x, pos.y);
	float lodDist;
	if (minfo->m_noFade)
		lodDist = minfo->GetLargestLodDistance() + STREAM_DISTANCE;
	else
		lodDist = minfo->GetLargestLodDistance() + STREAM_DISTANCE + FADE_DISTANCE;

	if (lodDist > distToZone) return true;
	if (!checkLevel) return false;
	CVector tempPos(minx, miny, minz);
	return CTheZones::GetLevelFromPosition(&pos) == CTheZones::GetLevelFromPosition(&tempPos);
}

bool
CCullZone::PointFallsWithinZone(CVector pos, float radius)
{
	if(minx - radius > pos.x ||
	   maxx + radius < pos.x ||
	   miny - radius > pos.y ||
	   maxy + radius < pos.y ||
	   minz - radius > pos.z ||
	   maxz + radius < pos.z)
		return false;
	return true;
}


CVector ExtraFudgePointsCoors[] = {
	CVector(978.0f, -394.0f, 18.0f),
	CVector(1189.7f, -414.6f, 27.0f),
	CVector(978.8f, -391.0f, 19.0f),
	CVector(1199.0f, -502.3f, 28.0f),
	CVector(1037.0f, -391.9f, 18.4f),
	CVector(1140.0f, -608.7f, 16.0f),
	CVector(1051.0f, -26.0f, 11.0f),
	CVector(951.5f, -345.1f, 12.0f),
	CVector(958.2f, -394.6f, 16.0f),
	CVector(1036.5f, -390.0f, 15.2f),
	CVector(960.6f, -390.5f, 20.9f),
	CVector(1061.0f, -640.6f, 16.3f),
	CVector(1034.5f, -388.96f, 14.78f),
	CVector(1038.4f, -13.98f, 12.2f),
	CVector(1047.2f, -16.7f, 10.6f),
	CVector(1257.9f, -333.3f, 40.0f),
	CVector(885.6f, -424.9f, 17.0f),
	CVector(1127.5f, -795.8f, 17.7f),
	CVector(1133.0f, -716.0f, 19.0f),
	CVector(1125.0f, -694.0f, 18.5f),
	CVector(1125.0f, -670.0f, 16.3f),
	CVector(1051.6f, 36.3f, 17.9f),
	CVector(1054.6f, -11.4f, 15.0f),
	CVector(1058.9f, -278.0f, 15.0f),
	CVector(1059.4f, -261.0f, 10.9f),
	CVector(1051.5f, -638.5f, 16.5f),
	CVector(1058.2f, -643.4f, 15.5f),
	CVector(1058.2f, -643.4f, 18.0f),
	CVector(826.0f, -260.0f, 7.0f),
	CVector(826.0f, -260.0f, 11.0f),
	CVector(833.0f, -603.6f, 16.4f),
	CVector(833.0f, -603.6f, 20.0f),
	CVector(1002.0f, -318.5f, 10.5f),
	CVector(998.0f, -318.0f, 9.8f),
	CVector(1127.0f, -183.0f, 18.1f),
	CVector(1123.0f, -331.5f, 23.8f),
	CVector(1123.8f, -429.0f, 24.0f),
	CVector(1197.0f, -30.0f, 13.7f),
	CVector(1117.5f, -230.0f, 17.3f),
	CVector(1117.5f, -230.0f, 20.0f),
	CVector(1120.0f, -281.6f, 21.5f),
	CVector(1120.0f, -281.6f, 24.0f),
	CVector(1084.5f, -1022.7f, 17.0f),
	CVector(1071.5f, 5.4f, 4.6f),
	CVector(1177.2f, -215.7f, 27.6f),
	CVector(841.6f, -460.0f, 19.7f),
	CVector(874.8f, -456.6f, 16.6f),
	CVector(918.3f, -451.8f, 17.8f),
	CVector(844.0f, -495.7f, 16.7f),
	CVector(842.0f, -493.4f, 21.0f),
	CVector(1433.5f, -774.4f, 16.9f),
	CVector(1051.0f, -205.0f, 7.5f),
	CVector(885.5f, -425.6f, 15.6f),
	CVector(182.6f, -470.4f, 27.8f),
	CVector(132.5f, -930.2f, 29.0f),
	CVector(124.7f, -904.0f, 28.0f),
	CVector(-50.0f, -686.0f, 22.0f),
	CVector(-49.1f, -694.5f, 22.5f),
	CVector(1063.8f, -404.45f, 16.2f),
	CVector(1062.2f, -405.5f, 17.0f)
};
int32 NumTestPoints;
int32 aTestPointsX[100];
int32 aTestPointsY[100];
int32 aTestPointsZ[100];
CVector aTestPoints[100];
int32 ElementsX, ElementsY, ElementsZ;
float StepX, StepY, StepZ;
int32 Memsize;
uint8 *pMem;
#define MEM(x, y, z) pMem[((x)*ElementsY + (y))*ElementsZ + (z)]
#define FLAG_FREE 1
#define FLAG_PROCESSED 2

int32 MinValX, MaxValX;
int32 MinValY, MaxValY;
int32 MinValZ, MaxValZ;
int32 Point1, Point2;
int32 NewPointX, NewPointY, NewPointZ;


void
CCullZone::FindTestPoints()
{
	static int CZNumber;

	NumTestPoints = 0;
	ElementsX = (maxx-minx) < 1.0f ? 2 : (maxx-minx)+1.0f;
	ElementsY = (maxy-miny) < 1.0f ? 2 : (maxy-miny)+1.0f;
	ElementsZ = (maxz-minz) < 1.0f ? 2 : (maxz-minz)+1.0f;
	if(ElementsX > 32) ElementsX = 32;
	if(ElementsY > 32) ElementsY = 32;
	if(ElementsZ > 32) ElementsZ = 32;
	Memsize = ElementsX * ElementsY * ElementsZ;
	StepX = (maxx-minx)/(ElementsX-1);
	StepY = (maxy-miny)/(ElementsY-1);
	StepZ = (maxz-minz)/(ElementsZ-1);

	pMem = new uint8[Memsize];
	memset(pMem, 0, Memsize);

	// indices of center
	int x = ElementsX * (position.x-minx)/(maxx-minx);
	x = clamp(x, 0, ElementsX-1);
	int y = ElementsY * (position.y-miny)/(maxy-miny);
	y = clamp(y, 0, ElementsY-1);
	int z = ElementsZ * (position.z-minz)/(maxz-minz);
	z = clamp(z, 0, ElementsZ-1);

	// Mark which test points inside the zone are not occupied by buildings.
	// To do this, mark the start point as free and do a food fill.

	// NB: we just assume the start position is free here!
	MEM(x, y, z) |= FLAG_FREE;
	aTestPointsX[NumTestPoints] = x;
	aTestPointsY[NumTestPoints] = y;
	aTestPointsZ[NumTestPoints] = z;
	NumTestPoints++;

	bool notDoneYet;
	do{
		notDoneYet = false;
		for(x = 0; x < ElementsX; x++){
			for(y = 0; y < ElementsY; y++){
				for(z = 0; z < ElementsZ; z++){
					if(!(MEM(x, y, z) & FLAG_FREE) || MEM(x, y, z) & FLAG_PROCESSED)
						continue;

					float pX = x*StepX + minx;
					float pY = y*StepY + miny;
					float pZ = z*StepZ + minz;

					if(x > 0 && !(MEM(x-1, y, z) & (FLAG_FREE | FLAG_PROCESSED)) &&
					   !TestLine(CVector(pX, pY, pZ), CVector(pX-StepX, pY, pZ)))
						MEM(x-1, y, z) |= FLAG_FREE;
					if(x < ElementsX-1 && !(MEM(x+1, y, z) & (FLAG_FREE | FLAG_PROCESSED)) &&
					   !TestLine(CVector(pX, pY, pZ), CVector(pX+StepX, pY, pZ)))
						MEM(x+1, y, z) |= FLAG_FREE;

					if(y > 0 && !(MEM(x, y-1, z) & (FLAG_FREE | FLAG_PROCESSED)) &&
					   !TestLine(CVector(pX, pY, pZ), CVector(pX, pY-StepY, pZ)))
						MEM(x, y-1, z) |= FLAG_FREE;
					if(y < ElementsY-1 && !(MEM(x, y+1, z) & (FLAG_FREE | FLAG_PROCESSED)) &&
					   !TestLine(CVector(pX, pY, pZ), CVector(pX, pY+StepY, pZ)))
						MEM(x, y+1, z) |= FLAG_FREE;

					if(z > 0 && !(MEM(x, y, z-1) & (FLAG_FREE | FLAG_PROCESSED)) &&
					   !TestLine(CVector(pX, pY, pZ), CVector(pX, pY, pZ-StepZ)))
						MEM(x, y, z-1) |= FLAG_FREE;
					if(z < ElementsZ-1 && !(MEM(x, y, z+1) & (FLAG_FREE | FLAG_PROCESSED)) &&
					   !TestLine(CVector(pX, pY, pZ), CVector(pX, pY, pZ+StepZ)))
						MEM(x, y, z+1) |= FLAG_FREE;

					notDoneYet = true;
					MEM(x, y, z) |= FLAG_PROCESSED;
				}
			}
		}
	}while(notDoneYet);

	bool done;

	// Find bound planes of free space

	// increase x, bounds in y and z
	x = 0;
	do{
		done = false;
		int minA = 10000;
		int minB = 10000;
		int maxA = -10000;
		int maxB = -10000;
		for(y = 0; y < ElementsY; y++)
			for(z = 0; z < ElementsZ; z++)
				if(MEM(x, y, z) & FLAG_FREE){
					if(y + z < minA){
						minA = y + z;
						aTestPointsX[NumTestPoints] = x;
						aTestPointsY[NumTestPoints] = y;
						aTestPointsZ[NumTestPoints] = z;
					}
					if(y + z > maxA){
						maxA = y + z;
						aTestPointsX[NumTestPoints+1] = x;
						aTestPointsY[NumTestPoints+1] = y;
						aTestPointsZ[NumTestPoints+1] = z;
					}
					if(y - z < minB){
						minB = y - z;
						aTestPointsX[NumTestPoints+2] = x;
						aTestPointsY[NumTestPoints+2] = y;
						aTestPointsZ[NumTestPoints+2] = z;
					}
					if(y - z > maxB){
						maxB = y - z;
						aTestPointsX[NumTestPoints+3] = x;
						aTestPointsY[NumTestPoints+3] = y;
						aTestPointsZ[NumTestPoints+3] = z;
					}
					done = true;
				}
		x++;
	}while(!done);
	NumTestPoints += 4;

	// decrease x, bounds in y and z
	x = ElementsX-1;
	do{
		done = false;
		int minA = 10000;
		int minB = 10000;
		int maxA = -10000;
		int maxB = -10000;
		for(y = 0; y < ElementsY; y++)
			for(z = 0; z < ElementsZ; z++)
				if(MEM(x, y, z) & FLAG_FREE){
					if(y + z < minA){
						minA = y + z;
						aTestPointsX[NumTestPoints] = x;
						aTestPointsY[NumTestPoints] = y;
						aTestPointsZ[NumTestPoints] = z;
					}
					if(y + z > maxA){
						maxA = y + z;
						aTestPointsX[NumTestPoints+1] = x;
						aTestPointsY[NumTestPoints+1] = y;
						aTestPointsZ[NumTestPoints+1] = z;
					}
					if(y - z < minB){
						minB = y - z;
						aTestPointsX[NumTestPoints+2] = x;
						aTestPointsY[NumTestPoints+2] = y;
						aTestPointsZ[NumTestPoints+2] = z;
					}
					if(y - z > maxB){
						maxB = y - z;
						aTestPointsX[NumTestPoints+3] = x;
						aTestPointsY[NumTestPoints+3] = y;
						aTestPointsZ[NumTestPoints+3] = z;
					}
					done = true;
				}
		x--;
	}while(!done);
	NumTestPoints += 4;

	// increase y, bounds in x and z
	y = 0;
	do{
		done = false;
		int minA = 10000;
		int minB = 10000;
		int maxA = -10000;
		int maxB = -10000;
		for(x = 0; x < ElementsX; x++)
			for(z = 0; z < ElementsZ; z++)
				if(MEM(x, y, z) & FLAG_FREE){
					if(x + z < minA){
						minA = x + z;
						aTestPointsX[NumTestPoints] = x;
						aTestPointsY[NumTestPoints] = y;
						aTestPointsZ[NumTestPoints] = z;
					}
					if(x + z > maxA){
						maxA = x + z;
						aTestPointsX[NumTestPoints+1] = x;
						aTestPointsY[NumTestPoints+1] = y;
						aTestPointsZ[NumTestPoints+1] = z;
					}
					if(x - z < minB){
						minB = x - z;
						aTestPointsX[NumTestPoints+2] = x;
						aTestPointsY[NumTestPoints+2] = y;
						aTestPointsZ[NumTestPoints+2] = z;
					}
					if(x - z > maxB){
						maxB = x - z;
						aTestPointsX[NumTestPoints+3] = x;
						aTestPointsY[NumTestPoints+3] = y;
						aTestPointsZ[NumTestPoints+3] = z;
					}
					done = true;
				}
		y++;
	}while(!done);
	NumTestPoints += 4;

	// decrease y, bounds in x and z
	y = ElementsY-1;
	do{
		done = false;
		int minA = 10000;
		int minB = 10000;
		int maxA = -10000;
		int maxB = -10000;
		for(x = 0; x < ElementsX; x++)
			for(z = 0; z < ElementsZ; z++)
				if(MEM(x, y, z) & FLAG_FREE){
					if(x + z < minA){
						minA = x + z;
						aTestPointsX[NumTestPoints] = x;
						aTestPointsY[NumTestPoints] = y;
						aTestPointsZ[NumTestPoints] = z;
					}
					if(x + z > maxA){
						maxA = x + z;
						aTestPointsX[NumTestPoints+1] = x;
						aTestPointsY[NumTestPoints+1] = y;
						aTestPointsZ[NumTestPoints+1] = z;
					}
					if(x - z < minB){
						minB = x - z;
						aTestPointsX[NumTestPoints+2] = x;
						aTestPointsY[NumTestPoints+2] = y;
						aTestPointsZ[NumTestPoints+2] = z;
					}
					if(x - z > maxB){
						maxB = x - z;
						aTestPointsX[NumTestPoints+3] = x;
						aTestPointsY[NumTestPoints+3] = y;
						aTestPointsZ[NumTestPoints+3] = z;
					}
					done = true;
				}
		y--;
	}while(!done);
	NumTestPoints += 4;

	// increase z, bounds in x and y
	z = 0;
	do{
		done = false;
		int minA = 10000;
		int minB = 10000;
		int maxA = -10000;
		int maxB = -10000;
		for(x = 0; x < ElementsX; x++)
			for(y = 0; y < ElementsY; y++)
				if(MEM(x, y, z) & FLAG_FREE){
					if(x + y < minA){
						minA = x + y;
						aTestPointsX[NumTestPoints] = x;
						aTestPointsY[NumTestPoints] = y;
						aTestPointsZ[NumTestPoints] = z;
					}
					if(x + y > maxA){
						maxA = x + y;
						aTestPointsX[NumTestPoints+1] = x;
						aTestPointsY[NumTestPoints+1] = y;
						aTestPointsZ[NumTestPoints+1] = z;
					}
					if(x - y < minB){
						minB = x - y;
						aTestPointsX[NumTestPoints+2] = x;
						aTestPointsY[NumTestPoints+2] = y;
						aTestPointsZ[NumTestPoints+2] = z;
					}
					if(x - y > maxB){
						maxB = x - y;
						aTestPointsX[NumTestPoints+3] = x;
						aTestPointsY[NumTestPoints+3] = y;
						aTestPointsZ[NumTestPoints+3] = z;
					}
					done = true;
				}
		z++;
	}while(!done);
	NumTestPoints += 4;

	// decrease z, bounds in x and y
	z = ElementsZ-1;
	do{
		done = false;
		int minA = 10000;
		int minB = 10000;
		int maxA = -10000;
		int maxB = -10000;
		for(x = 0; x < ElementsX; x++)
			for(y = 0; y < ElementsY; y++)
				if(MEM(x, y, z) & FLAG_FREE){
					if(x + y < minA){
						minA = x + y;
						aTestPointsX[NumTestPoints] = x;
						aTestPointsY[NumTestPoints] = y;
						aTestPointsZ[NumTestPoints] = z;
					}
					if(x + y > maxA){
						maxA = x + y;
						aTestPointsX[NumTestPoints+1] = x;
						aTestPointsY[NumTestPoints+1] = y;
						aTestPointsZ[NumTestPoints+1] = z;
					}
					if(x - y < minB){
						minB = x - y;
						aTestPointsX[NumTestPoints+2] = x;
						aTestPointsY[NumTestPoints+2] = y;
						aTestPointsZ[NumTestPoints+2] = z;
					}
					if(x - y > maxB){
						maxB = x - y;
						aTestPointsX[NumTestPoints+3] = x;
						aTestPointsY[NumTestPoints+3] = y;
						aTestPointsZ[NumTestPoints+3] = z;
					}
					done = true;
				}
		z--;
	}while(!done);
	NumTestPoints += 4;

	// divide the axis aligned bounding planes into 4 and place some test points

	// x = 0 plane
	MinValY = 999999;
	MinValZ = 999999;
	MaxValY = 0;
	MaxValZ = 0;
	for(y = 0; y < ElementsY; y++)
		for(z = 0; z < ElementsZ; z++)
			if(MEM(0, y, z) & FLAG_FREE){
				if(y < MinValY) MinValY = y;
				if(z < MinValZ) MinValZ = z;
				if(y > MaxValY) MaxValY = y;
				if(z > MaxValZ) MaxValZ = z;
			}
	// pick 4 points in the found bounds and add new test points
	if(MaxValY != 0 && MaxValZ != 0)
		for(Point1 = 0; Point1 < 2; Point1++)
			for(Point2 = 0; Point2 < 2; Point2++){
				NewPointY = (Point1 + 0.5f)*(MaxValY - MinValY)*0.5f + MinValY;
				NewPointZ = (Point2 + 0.5f)*(MaxValZ - MinValZ)*0.5f + MinValZ;
				if(MEM(0, NewPointY, NewPointZ) & FLAG_FREE){
					aTestPointsX[NumTestPoints] = 0;
					aTestPointsY[NumTestPoints] = NewPointY;
					aTestPointsZ[NumTestPoints] = NewPointZ;
					NumTestPoints++;
				}
			}

	// x = ElementsX-1 plane
	MinValY = 999999;
	MinValZ = 999999;
	MaxValY = 0;
	MaxValZ = 0;
	for(y = 0; y < ElementsY; y++)
		for(z = 0; z < ElementsZ; z++)
			if(MEM(ElementsX-1, y, z) & FLAG_FREE){
				if(y < MinValY) MinValY = y;
				if(z < MinValZ) MinValZ = z;
				if(y > MaxValY) MaxValY = y;
				if(z > MaxValZ) MaxValZ = z;
			}
	// pick 4 points in the found bounds and add new test points
	if(MaxValY != 0 && MaxValZ != 0)
		for(Point1 = 0; Point1 < 2; Point1++)
			for(Point2 = 0; Point2 < 2; Point2++){
				NewPointY = (Point1 + 0.5f)*(MaxValY - MinValY)*0.5f + MinValY;
				NewPointZ = (Point2 + 0.5f)*(MaxValZ - MinValZ)*0.5f + MinValZ;
				if(MEM(ElementsX-1, NewPointY, NewPointZ) & FLAG_FREE){
					aTestPointsX[NumTestPoints] = ElementsX-1;
					aTestPointsY[NumTestPoints] = NewPointY;
					aTestPointsZ[NumTestPoints] = NewPointZ;
					NumTestPoints++;
				}
			}

	// y = 0 plane
	MinValX = 999999;
	MinValZ = 999999;
	MaxValX = 0;
	MaxValZ = 0;
	for(x = 0; x < ElementsX; x++)
		for(z = 0; z < ElementsZ; z++)
			if(MEM(x, 0, z) & FLAG_FREE){
				if(x < MinValX) MinValX = x;
				if(z < MinValZ) MinValZ = z;
				if(x > MaxValX) MaxValX = x;
				if(z > MaxValZ) MaxValZ = z;
			}
	// pick 4 points in the found bounds and add new test points
	if(MaxValX != 0 && MaxValZ != 0)
		for(Point1 = 0; Point1 < 2; Point1++)
			for(Point2 = 0; Point2 < 2; Point2++){
				NewPointX = (Point1 + 0.5f)*(MaxValX - MinValX)*0.5f + MinValX;
				NewPointZ = (Point2 + 0.5f)*(MaxValZ - MinValZ)*0.5f + MinValZ;
				if(MEM(NewPointX, 0, NewPointZ) & FLAG_FREE){
					aTestPointsX[NumTestPoints] = NewPointX;
					aTestPointsY[NumTestPoints] = 0;
					aTestPointsZ[NumTestPoints] = NewPointZ;
					NumTestPoints++;
				}
			}

	// y = ElementsY-1 plane
	MinValX = 999999;
	MinValZ = 999999;
	MaxValX = 0;
	MaxValZ = 0;
	for(x = 0; x < ElementsX; x++)
		for(z = 0; z < ElementsZ; z++)
			if(MEM(x, ElementsY-1, z) & FLAG_FREE){
				if(x < MinValX) MinValX = x;
				if(z < MinValZ) MinValZ = z;
				if(x > MaxValX) MaxValX = x;
				if(z > MaxValZ) MaxValZ = z;
			}
	// pick 4 points in the found bounds and add new test points
	if(MaxValX != 0 && MaxValZ != 0)
		for(Point1 = 0; Point1 < 2; Point1++)
			for(Point2 = 0; Point2 < 2; Point2++){
				NewPointX = (Point1 + 0.5f)*(MaxValX - MinValX)*0.5f + MinValX;
				NewPointZ = (Point2 + 0.5f)*(MaxValZ - MinValZ)*0.5f + MinValZ;
				if(MEM(NewPointX, ElementsY-1, NewPointZ) & FLAG_FREE){
					aTestPointsX[NumTestPoints] = NewPointX;
					aTestPointsY[NumTestPoints] = ElementsY-1;
					aTestPointsZ[NumTestPoints] = NewPointZ;
					NumTestPoints++;
				}
			}

	// z = 0 plane
	MinValX = 999999;
	MinValY = 999999;
	MaxValX = 0;
	MaxValY = 0;
	for(x = 0; x < ElementsX; x++)
		for(y = 0; y < ElementsY; y++)
			if(MEM(x, y, 0) & FLAG_FREE){
				if(x < MinValX) MinValX = x;
				if(y < MinValY) MinValY = y;
				if(x > MaxValX) MaxValX = x;
				if(y > MaxValY) MaxValY = y;
			}
	// pick 4 points in the found bounds and add new test points
	if(MaxValX != 0 && MaxValY != 0)
		for(Point1 = 0; Point1 < 2; Point1++)
			for(Point2 = 0; Point2 < 2; Point2++){
				NewPointX = (Point1 + 0.5f)*(MaxValX - MinValX)*0.5f + MinValX;
				NewPointY = (Point2 + 0.5f)*(MaxValY - MinValY)*0.5f + MinValY;
				if(MEM(NewPointX, NewPointY, 0) & FLAG_FREE){
					aTestPointsX[NumTestPoints] = NewPointX;
					aTestPointsY[NumTestPoints] = NewPointY;
					aTestPointsZ[NumTestPoints] = 0;
					NumTestPoints++;
				}
			}

	// z = ElementsZ-1 plane
	MinValX = 999999;
	MinValY = 999999;
	MaxValX = 0;
	MaxValY = 0;
	for(x = 0; x < ElementsX; x++)
		for(y = 0; y < ElementsY; y++)
			if(MEM(x, y, ElementsZ-1) & FLAG_FREE){
				if(x < MinValX) MinValX = x;
				if(y < MinValY) MinValY = y;
				if(x > MaxValX) MaxValX = x;
				if(y > MaxValY) MaxValY = y;
			}
	// pick 4 points in the found bounds and add new test points
	if(MaxValX != 0 && MaxValY != 0)
		for(Point1 = 0; Point1 < 2; Point1++)
			for(Point2 = 0; Point2 < 2; Point2++){
				NewPointX = (Point1 + 0.5f)*(MaxValX - MinValX)*0.5f + MinValX;
				NewPointY = (Point2 + 0.5f)*(MaxValY - MinValY)*0.5f + MinValY;
				if(MEM(NewPointX, NewPointY, ElementsZ-1) & FLAG_FREE){
					aTestPointsX[NumTestPoints] = NewPointX;
					aTestPointsY[NumTestPoints] = NewPointY;
					aTestPointsZ[NumTestPoints] = ElementsZ-1;
					NumTestPoints++;
				}
			}

	// add some hardcoded test points
	for(int i = 0; i < ARRAY_SIZE(ExtraFudgePointsCoors); i++)
		if(PointFallsWithinZone(ExtraFudgePointsCoors[i], 0.0f)){
			x = ElementsX * (ExtraFudgePointsCoors[i].x-minx)/(maxx-minx);
			y = ElementsY * (ExtraFudgePointsCoors[i].y-miny)/(maxy-miny);
			z = ElementsZ * (ExtraFudgePointsCoors[i].z-minz)/(maxz-minz);
			if(MEM(x, y, z) & FLAG_FREE){
				aTestPointsX[NumTestPoints] = x;
				aTestPointsY[NumTestPoints] = y;
				aTestPointsZ[NumTestPoints] = z;
				NumTestPoints++;
			}
		}

	// remove duplicate points
	for(int i = 0; i < NumTestPoints; i++)
		for(int j = i+1; j < NumTestPoints; j++)
			if(aTestPointsX[j] == aTestPointsX[i] &&
			   aTestPointsY[j] == aTestPointsY[i] &&
			   aTestPointsZ[j] == aTestPointsZ[i]){
				// get rid of [j]
				for(int k = j; k < NumTestPoints-1; k++){
					aTestPointsX[k] = aTestPointsX[k+1];
					aTestPointsY[k] = aTestPointsY[k+1];
					aTestPointsZ[k] = aTestPointsZ[k+1];
				}
				NumTestPoints--;
			}

	// convert points to floating point
	for(int i = 0; i < NumTestPoints; i++){
		aTestPoints[i].x = aTestPointsX[i]*StepX + minx;
		aTestPoints[i].y = aTestPointsY[i]*StepY + miny;
		aTestPoints[i].z = aTestPointsZ[i]*StepZ + minz;
	}

	CZNumber++;

	delete[] pMem;
	pMem = nil;
}

bool
CCullZone::TestEntityVisibilityFromCullZone(CEntity *entity, float extraDist, CEntity *LODentity)
{
	CColModel *colmodel = entity->GetColModel();
	float boundMaxX = colmodel->boundingBox.max.x;
	float boundMaxY = colmodel->boundingBox.max.y;
	float boundMaxZ = colmodel->boundingBox.max.z;
	float boundMinX = colmodel->boundingBox.min.x;
	float boundMinY = colmodel->boundingBox.min.y;
	float boundMinZ = colmodel->boundingBox.min.z;
	if(LODentity){
		colmodel = LODentity->GetColModel();
		boundMaxX = Max(boundMaxX, colmodel->boundingBox.max.x);
		boundMaxY = Max(boundMaxY, colmodel->boundingBox.max.y);
		boundMaxZ = Max(boundMaxZ, colmodel->boundingBox.max.z);
		boundMinX = Min(boundMinX, colmodel->boundingBox.min.x);
		boundMinY = Min(boundMinY, colmodel->boundingBox.min.y);
		boundMinZ = Min(boundMinZ, colmodel->boundingBox.min.z);
	}

	if(boundMaxZ-boundMinZ + extraDist < 0.5f)
		boundMaxZ = boundMinZ + 0.5f;
	else
		boundMaxZ += extraDist;

	CVector vecMin = entity->GetMatrix() * CVector(boundMinX, boundMinY, boundMinZ);
	CVector vecMaxX = entity->GetMatrix() * CVector(boundMaxX, boundMinY, boundMinZ);
	CVector vecMaxY = entity->GetMatrix() * CVector(boundMinX, boundMaxY, boundMinZ);
	CVector vecMaxZ = entity->GetMatrix() * CVector(boundMinX, boundMinY, boundMaxZ);
	CVector dirx = vecMaxX - vecMin;
	CVector diry = vecMaxY - vecMin;
	CVector dirz = vecMaxZ - vecMin;

	// If building intersects zone at all, it's visible
	int x, y, z;
	for(x = 0; x < 9; x++){
		CVector posX = vecMin + x/8.0f*dirx;
		for(y = 0; y < 9; y++){
			CVector posY = posX + y/8.0f*diry;
			for(z = 0; z < 9; z++){
 				CVector posZ = posY + z/8.0f*dirz;
				if(PointFallsWithinZone(posZ, 2.0f))
					return true;
			}
		}
	}

	float distToZone = CalcDistToCullZone(entity->GetPosition().x, entity->GetPosition().y)/15.0f;
	distToZone = Max(distToZone, 7.0f);
	int numX = (boundMaxX - boundMinX)/distToZone + 2.0f;
	int numY = (boundMaxY - boundMinY)/distToZone + 2.0f;
	int numZ = (boundMaxZ - boundMinZ)/distToZone + 2.0f;

	float stepX = 1.0f/(numX-1);
	float stepY = 1.0f/(numY-1);
	float stepZ = 1.0f/(numZ-1);
	float midX = (boundMaxX + boundMinX)/2.0f;
	float midY = (boundMaxY + boundMinY)/2.0f;
	float midZ = (boundMaxZ + boundMinZ)/2.0f;

	// check both xy planes
	for(int i = 0; i < NumTestPoints; i++){
		CVector testPoint = aTestPoints[i];
		CVector mid = entity->GetMatrix() * CVector(midX, midY, midZ);
		mid.z += 0.1f;
		if(DoThoroughLineTest(testPoint, mid, entity))
			return true;

		CVector ray = entity->GetPosition() - testPoint;

		float dotX = DotProduct(ray, dirx);
		float dotY = DotProduct(ray, diry);
		float dotZ = DotProduct(ray, dirz);

		for(x = 0; x < numX; x++){
			CVector pMinZ = vecMin + x*stepX*dirx;
			CVector pMaxZ = vecMin + x*stepX*dirx + dirz;
			for(y = 0; y < numY; y++)
				if(dotZ > 0.0f){
					if(DoThoroughLineTest(testPoint, pMinZ + y*stepY*diry, entity))
						return true;
				}else{
					if(DoThoroughLineTest(testPoint, pMaxZ + y*stepY*diry, entity))
						return true;
				}
		}

		for(x = 0; x < numX; x++){
			CVector pMinY = vecMin + x*stepX*dirx;
			CVector pMaxY = vecMin + x*stepX*dirx + diry;
			for(z = 1; z < numZ-1; z++)	// edge cases already handled
				if(dotY > 0.0f){
					if(DoThoroughLineTest(testPoint, pMinY + z*stepZ*dirz, entity))
						return true;
				}else{
					if(DoThoroughLineTest(testPoint, pMaxY + z*stepZ*dirz, entity))
						return true;
				}
		}

		for(y = 1; y < numY-1; y++){	// edge cases already handled
			CVector pMinX = vecMin + y*stepY*diry;
			CVector pMaxX = vecMin + y*stepY*diry + dirx;
			for(z = 1; z < numZ-1; z++)	// edge cases already handled
				if(dotX > 0.0f){
					if(DoThoroughLineTest(testPoint, pMinX + z*stepZ*dirz, entity))
						return true;
				}else{
					if(DoThoroughLineTest(testPoint, pMaxX + z*stepZ*dirz, entity))
						return true;
				}
		}
	}

	return false;
}
