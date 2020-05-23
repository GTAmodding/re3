#include "common.h"

#include "templates.h"
#include "General.h"
#include "ModelInfo.h"
#include "Streaming.h"
#include "FileLoader.h"
#include "Script.h"
#include "Timer.h"
#include "Camera.h"
#include "Frontend.h"
#include "Physical.h"
#include "ColStore.h"

CPool<ColDef,ColDef> *CColStore::ms_pColPool;

void
CColStore::Initialise(void)
{
	if(ms_pColPool == nil)
		ms_pColPool = new CPool<ColDef,ColDef>(COLSTORESIZE, "CollisionFiles");
	AddColSlot("generic");	// slot 0. not streamed
}

void
CColStore::Shutdown(void)
{
	int i;
	for(i = 0; i < COLSTORESIZE; i++)
		RemoveColSlot(i);
	if(ms_pColPool)
		delete ms_pColPool;
	ms_pColPool = nil;
}

int
CColStore::AddColSlot(const char *name)
{
	ColDef *def = ms_pColPool->New();
	assert(def);
	def->isLoaded = false;
	def->unused = 0;
	def->bounds.left = 1000000.0f;
	def->bounds.top = 1000000.0f;
	def->bounds.right = -1000000.0f;
	def->bounds.bottom = -1000000.0f;
	def->minIndex = INT16_MAX;
	def->maxIndex = INT16_MIN;
	strcpy(def->name, name);
	return ms_pColPool->GetJustIndex(def);
}

void
CColStore::RemoveColSlot(int slot)
{
	if(GetSlot(slot)){
		if(GetSlot(slot)->isLoaded)
			RemoveCol(slot);
		ms_pColPool->Delete(GetSlot(slot));
	}
}

int
CColStore::FindColSlot(const char *name)
{
	ColDef *def;
	int size = ms_pColPool->GetSize();
	for(int i = 0; i < size; i++){
		def = GetSlot(i);
		if(def && !CGeneral::faststricmp(def->name, name))
			return i;
	}
	return -1;
}

char*
CColStore::GetColName(int32 slot)
{
	return GetSlot(slot)->name;
}

CRect&
CColStore::GetBoundingBox(int32 slot)
{
	return GetSlot(slot)->bounds;
}

void
CColStore::IncludeModelIndex(int32 slot, int32 modelIndex)
{
	ColDef *def = GetSlot(slot);
	if(modelIndex < def->minIndex)
		def->minIndex = modelIndex;
	if(modelIndex > def->maxIndex)
		def->maxIndex = modelIndex;
}

bool
CColStore::LoadCol(int32 slot, uint8 *buffer, int32 bufsize)
{
	bool success;
	ColDef *def = GetSlot(slot);
	if(def->minIndex > def->maxIndex)
		success = CFileLoader::LoadCollisionFileFirstTime(buffer, bufsize, slot);
	else
		success = CFileLoader::LoadCollisionFile(buffer, bufsize, slot);
	if(success)
		def->isLoaded = true;
	else
		debug("Failed to load Collision\n");
	return success;
}

void
CColStore::RemoveCol(int32 slot)
{
	int id;
	GetSlot(slot)->isLoaded = false;
	for(id = 0; id < MODELINFOSIZE; id++){
		CBaseModelInfo *mi = CModelInfo::GetModelInfo(id);
		if(mi){
			CColModel *col = mi->GetColModel();
			if(col && col->level == slot)
				col->RemoveCollisionVolumes();
		}
	}
}

void
CColStore::LoadAllCollision(void)
{
	int i;
	for(i = 1; i < COLSTORESIZE; i++)
		if(GetSlot(i))
			CStreaming::RequestCol(i, 0);

	CStreaming::LoadAllRequestedModels(false);
}

void
CColStore::RemoveAllCollision(void)
{
	int i;
	for(i = 1; i < COLSTORESIZE; i++)
		if(GetSlot(i))
			if(CStreaming::CanRemoveCol(i))
				CStreaming::RemoveCol(i);
}

static bool bLoadAtSecondPosition;
static CVector2D secondPosition;

void
CColStore::AddCollisionNeededAtPosn(const CVector2D &pos)
{
	bLoadAtSecondPosition = true;
	secondPosition = pos;
}

void
CColStore::LoadCollision(const CVector2D &pos)
{
	int i;

	if(CStreaming::ms_disableStreaming)
		return;

	for(i = 1; i < COLSTORESIZE; i++){
		if(GetSlot(i) == nil)
			continue;

		bool wantThisOne = false;

		if(GetBoundingBox(i).IsPointInside(pos) ||
		   bLoadAtSecondPosition && GetBoundingBox(i).IsPointInside(secondPosition, -119.0f) ||
		   CGeneral::faststrcmp(GetColName(i), "yacht") == 0){
			wantThisOne = true;
		}else{
			for (int j = 0; j < MAX_CLEANUP; j++) {
				CPhysical* pEntity = CTheScripts::MissionCleanup.DoesThisEntityWaitForCollision(j);
				if (pEntity && !pEntity->bDontLoadCollision && !pEntity->bIsFrozen) {
					if (GetBoundingBox(i).IsPointInside(pEntity->GetPosition(), -80.0f))
						wantThisOne = true;
				}
			}
		}

		if(wantThisOne)
			CStreaming::RequestCol(i, STREAMFLAGS_PRIORITY);
		else
			CStreaming::RemoveCol(i);
	}
	bLoadAtSecondPosition = false;
}

void
CColStore::RequestCollision(const CVector2D &pos)
{
	int i;

	for(i = 1; i < COLSTORESIZE; i++)
		if(GetSlot(i) && GetBoundingBox(i).IsPointInside(pos, -115.0f))
			CStreaming::RequestCol(i, STREAMFLAGS_PRIORITY);
}

void
CColStore::EnsureCollisionIsInMemory(const CVector2D &pos)
{
	int i;

	if(CStreaming::ms_disableStreaming)
		return;

	for(i = 1; i < COLSTORESIZE; i++)
		if(GetSlot(i) && GetBoundingBox(i).IsPointInside(pos, -110.0f) &&
		   !CStreaming::HasColLoaded(i)){
			CStreaming::RequestCol(i, 0);
			if(TheCamera.GetScreenFadeStatus() == FADE_0)
				FrontEndMenuManager.MessageScreen("LOADCOL", false);
			CTimer::Suspend();
			CStreaming::LoadAllRequestedModels(false);
			CTimer::Resume();
		}
}

//--MIAMI: done
bool
CColStore::HasCollisionLoaded(const CVector2D &pos)
{
	int i;

	for(i = 1; i < COLSTORESIZE; i++)
		if(GetSlot(i) && GetBoundingBox(i).IsPointInside(pos, -115.0f) &&
		   !GetSlot(i)->isLoaded)
			return false;
	return true;
}
