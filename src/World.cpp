#include "common.h"
#include "patcher.h"
#include "Entity.h"
#include "Ped.h"
#include "Object.h"
#include "Garages.h"
#include "TempColModels.h"
#include "World.h"

CPtrList *CWorld::ms_bigBuildingsList = (CPtrList*)0x6FAB60;
CPtrList &CWorld::ms_listMovingEntityPtrs = *(CPtrList*)0x8F433C;
CSector (*CWorld::ms_aSectors)[NUMSECTORS_X] = (CSector (*)[NUMSECTORS_Y])0x665608;
uint16 &CWorld::ms_nCurrentScanCode = *(uint16*)0x95CC64;

unsigned char &CWorld::PlayerInFocus = *(unsigned char *)0x95CD61;
CPlayerInfo *CWorld::Players = (CPlayerInfo *)0x9412F0;
bool &CWorld::bNoMoreCollisionTorque = *(bool*)0x95CDCC;
CEntity *&CWorld::pIgnoreEntity	= *(CEntity**)0x8F6494;
bool &CWorld::bIncludeDeadPeds = *(bool*)0x95CD8F;
bool &CWorld::bSecondShift = *(bool*)0x95CD54;
bool &CWorld::bForceProcessControl = *(bool*)0x95CD6C;
bool &CWorld::bProcessCutsceneOnly = *(bool*)0x95CD8B;


void
CWorld::ClearScanCodes(void)
{
	CPtrNode *node;
	for(int i = 0; i < NUMSECTORS_Y; i++)
	for(int j = 0; j < NUMSECTORS_X; j++){
		CSector *s = &ms_aSectors[i][j];
		for(node = s->m_lists[ENTITYLIST_BUILDINGS].first; node; node = node->next)
			((CEntity*)node->item)->m_scanCode = 0;
		for(node = s->m_lists[ENTITYLIST_VEHICLES].first; node; node = node->next)
			((CEntity*)node->item)->m_scanCode = 0;
		for(node = s->m_lists[ENTITYLIST_PEDS].first; node; node = node->next)
			((CEntity*)node->item)->m_scanCode = 0;
		for(node = s->m_lists[ENTITYLIST_OBJECTS].first; node; node = node->next)
			((CEntity*)node->item)->m_scanCode = 0;
		for(node = s->m_lists[ENTITYLIST_DUMMIES].first; node; node = node->next)
			((CEntity*)node->item)->m_scanCode = 0;
	}
}

bool
CWorld::CameraToIgnoreThisObject(CEntity *ent)
{
	if(CGarages::IsModelIndexADoor(ent->GetModelIndex()))
		return false;
	return ((CObject*)ent)->m_bCameraToAvoidThisObject != 1;
}

bool
CWorld::ProcessLineOfSight(const CVector &point1, const CVector &point2, CColPoint &point, CEntity *&entity, bool checkBuildings, bool checkVehicles, bool checkPeds, bool checkObjects, bool checkDummies, bool ignoreSeeThrough, bool ignoreSomeObjects)
{
	int x, xstart, xend;
	int y, ystart, yend;
	int y1, y2;
	float dist;

	AdvanceCurrentScanCode();

	entity = nil;
	dist = 1.0f;

	xstart = GetSectorIndexX(point1.x);
	ystart = GetSectorIndexX(point1.y);
	xend = GetSectorIndexX(point2.x);
	yend = GetSectorIndexX(point2.y);

#define LOSARGS CColLine(point1, point2), point, dist, entity, checkBuildings, checkVehicles, checkPeds, checkObjects, checkDummies, ignoreSeeThrough, ignoreSomeObjects

	if(xstart == xend && ystart == yend){
		// Only one sector
		return ProcessLineOfSightSector(*GetSector(xstart, ystart), LOSARGS);
	}else if(xstart == xend){
		// Only step in y
		if(ystart < yend)
			for(y = ystart; y <= yend; y++)
				ProcessLineOfSightSector(*GetSector(xstart, y), LOSARGS);
		else
			for(y = ystart; y >= yend; y--)
				ProcessLineOfSightSector(*GetSector(xstart, y), LOSARGS);
		return dist < 1.0f;
	}else if(ystart == yend){
		// Only step in x
		if(xstart < xend)
			for(x = xstart; x <= xend; x++)
				ProcessLineOfSightSector(*GetSector(x, ystart), LOSARGS);
		else
			for(x = xstart; x >= xend; x--)
				ProcessLineOfSightSector(*GetSector(x, ystart), LOSARGS);
		return dist < 1.0f;
	}else{
		if(point1.x < point2.x){
			// Step from left to right
			float m = (point2.y - point1.y) / (point2.x - point1.x);

			y1 = ystart;
			y2 = GetSectorIndexY((GetWorldX(xstart+1) - point1.x)*m + point1.y);
			if(y1 < y2)
				for(y = y1; y <= y2; y++)
					ProcessLineOfSightSector(*GetSector(xstart, y), LOSARGS);
			else
				for(y = y1; y >= y2; y--)
					ProcessLineOfSightSector(*GetSector(xstart, y), LOSARGS);

			for(x = xstart+1; x < xend; x++){
				y1 = y2;
				y2 = GetSectorIndexY((GetWorldX(x+1) - point1.x)*m + point1.y);
				if(y1 < y2)
					for(y = y1; y <= y2; y++)
						ProcessLineOfSightSector(*GetSector(x, y), LOSARGS);
				else
					for(y = y1; y >= y2; y--)
						ProcessLineOfSightSector(*GetSector(x, y), LOSARGS);
			}

			y1 = y2;
			y2 = yend;
			if(y1 < y2)
				for(y = y1; y <= y2; y++)
					ProcessLineOfSightSector(*GetSector(xend, y), LOSARGS);
			else
				for(y = y1; y >= y2; y--)
					ProcessLineOfSightSector(*GetSector(xend, y), LOSARGS);
		}else{
			// Step from right to left
			float m = (point2.y - point1.y) / (point2.x - point1.x);

			y1 = ystart;
			y2 = GetSectorIndexY((GetWorldX(xstart) - point1.x)*m + point1.y);
			if(y1 < y2)
				for(y = y1; y <= y2; y++)
					ProcessLineOfSightSector(*GetSector(xstart, y), LOSARGS);
			else
				for(y = y1; y >= y2; y--)
					ProcessLineOfSightSector(*GetSector(xstart, y), LOSARGS);

			for(x = xstart-1; x > xend; x--){
				y1 = y2;
				y2 = GetSectorIndexY((GetWorldX(x) - point1.x)*m + point1.y);
				if(y1 < y2)
					for(y = y1; y <= y2; y++)
						ProcessLineOfSightSector(*GetSector(x, y), LOSARGS);
				else
					for(y = y1; y >= y2; y--)
						ProcessLineOfSightSector(*GetSector(x, y), LOSARGS);
			}

			y1 = y2;
			y2 = yend;
			if(y1 < y2)
				for(y = y1; y <= y2; y++)
					ProcessLineOfSightSector(*GetSector(xend, y), LOSARGS);
			else
				for(y = y1; y >= y2; y--)
					ProcessLineOfSightSector(*GetSector(xend, y), LOSARGS);
		}
		return dist < 1.0f;
	}

#undef LOSARGS
}

bool
CWorld::ProcessLineOfSightSector(CSector &sector, const CColLine &line, CColPoint &point, float &dist, CEntity *&entity, bool checkBuildings, bool checkVehicles, bool checkPeds, bool checkObjects, bool checkDummies, bool ignoreSeeThrough, bool ignoreSomeObjects)
{
	float mindist = dist;
	bool deadPeds = !!bIncludeDeadPeds;
	bIncludeDeadPeds = false;

	if(checkBuildings){
		ProcessLineOfSightSectorList(sector.m_lists[ENTITYLIST_BUILDINGS], line, point, mindist, entity, ignoreSeeThrough);
		ProcessLineOfSightSectorList(sector.m_lists[ENTITYLIST_BUILDINGS_OVERLAP], line, point, mindist, entity, ignoreSeeThrough);
	}

	if(checkVehicles){
		ProcessLineOfSightSectorList(sector.m_lists[ENTITYLIST_VEHICLES], line, point, mindist, entity, ignoreSeeThrough);
		ProcessLineOfSightSectorList(sector.m_lists[ENTITYLIST_VEHICLES_OVERLAP], line, point, mindist, entity, ignoreSeeThrough);
	}

	if(checkPeds){
		if(deadPeds)
			bIncludeDeadPeds = true;
		ProcessLineOfSightSectorList(sector.m_lists[ENTITYLIST_PEDS], line, point, mindist, entity, ignoreSeeThrough);
		ProcessLineOfSightSectorList(sector.m_lists[ENTITYLIST_PEDS_OVERLAP], line, point, mindist, entity, ignoreSeeThrough);
		bIncludeDeadPeds = false;
	}

	if(checkObjects){
		ProcessLineOfSightSectorList(sector.m_lists[ENTITYLIST_OBJECTS], line, point, mindist, entity, ignoreSeeThrough, ignoreSomeObjects);
		ProcessLineOfSightSectorList(sector.m_lists[ENTITYLIST_OBJECTS_OVERLAP], line, point, mindist, entity, ignoreSeeThrough, ignoreSomeObjects);
	}

	if(checkDummies){
		ProcessLineOfSightSectorList(sector.m_lists[ENTITYLIST_DUMMIES], line, point, mindist, entity, ignoreSeeThrough);
		ProcessLineOfSightSectorList(sector.m_lists[ENTITYLIST_DUMMIES_OVERLAP], line, point, mindist, entity, ignoreSeeThrough);
	}

	bIncludeDeadPeds = deadPeds;

	if(mindist < dist){
		dist = mindist;
		return true;
	}else
		return false;
}

bool
CWorld::ProcessLineOfSightSectorList(CPtrList &list, const CColLine &line, CColPoint &point, float &dist, CEntity *&entity, bool ignoreSeeThrough, bool ignoreSomeObjects)
{
	bool deadPeds = false;
	float mindist = dist;
	CPtrNode *node;
	CEntity *e;
	CColModel *colmodel;

	if(list.first && bIncludeDeadPeds && ((CEntity*)list.first->item)->IsPed())
		deadPeds = true;

	for(node = list.first; node; node = node->next){
		e = (CEntity*)node->item;
		if(e->m_scanCode != GetCurrentScanCode() &&
		   e != pIgnoreEntity &&
		   (e->bUsesCollision || deadPeds) &&
		   !(ignoreSomeObjects && CameraToIgnoreThisObject(e))){
			colmodel = nil;
			e->m_scanCode = GetCurrentScanCode();

			if(e->IsPed()){
				if(e->bUsesCollision ||
				   deadPeds && ((CPed*)e)->m_nPedState == PED_DEAD){
					if(((CPed*)e)->UseGroundColModel())
						colmodel = &CTempColModels::ms_colModelPedGroundHit;
					else
						colmodel = CModelInfo::GetModelInfo(e->GetModelIndex())->GetColModel();
				}else
					colmodel = nil;
			}else if(e->bUsesCollision)
				colmodel = CModelInfo::GetModelInfo(e->GetModelIndex())->GetColModel();

			if(colmodel &&
			   CCollision::ProcessLineOfSight(line, e->GetMatrix(), *colmodel, point, dist, ignoreSeeThrough))
				entity = e;
		}
	}

	if(mindist < dist){
		dist = mindist;
		return true;
	}else
		return false;
}

bool 
CWorld::ProcessVerticalLine(const CVector &point1, float z2, CColPoint &point, CEntity *&entity, bool checkBuildings, bool checkVehicles, bool checkPeds, bool checkObjects, bool checkDummies, bool ignoreSeeThrough, CStoredCollPoly *poly)
{
	AdvanceCurrentScanCode();
	CVector point2(point1.x, point1.y, z2);
	return CWorld::ProcessVerticalLineSector(*GetSector(GetSectorIndexX(point1.x), GetSectorIndexX(point1.y)),
		CColLine(point1, point2), point, entity,
		checkBuildings, checkVehicles, checkPeds, checkObjects, checkDummies, ignoreSeeThrough, poly);
}

bool
CWorld::ProcessVerticalLineSector(CSector &sector, const CColLine &line, CColPoint &point, CEntity *&entity, bool checkBuildings, bool checkVehicles, bool checkPeds, bool checkObjects, bool checkDummies, bool ignoreSeeThrough, CStoredCollPoly *poly)
{
	float mindist = 1.0f;

	if(checkBuildings){
		ProcessVerticalLineSectorList(sector.m_lists[ENTITYLIST_BUILDINGS], line, point, mindist, entity, ignoreSeeThrough, poly);
		ProcessVerticalLineSectorList(sector.m_lists[ENTITYLIST_BUILDINGS_OVERLAP], line, point, mindist, entity, ignoreSeeThrough, poly);
	}

	if(checkVehicles){
		ProcessVerticalLineSectorList(sector.m_lists[ENTITYLIST_VEHICLES], line, point, mindist, entity, ignoreSeeThrough, poly);
		ProcessVerticalLineSectorList(sector.m_lists[ENTITYLIST_VEHICLES_OVERLAP], line, point, mindist, entity, ignoreSeeThrough, poly);
	}

	if(checkPeds){
		ProcessVerticalLineSectorList(sector.m_lists[ENTITYLIST_PEDS], line, point, mindist, entity, ignoreSeeThrough, poly);
		ProcessVerticalLineSectorList(sector.m_lists[ENTITYLIST_PEDS_OVERLAP], line, point, mindist, entity, ignoreSeeThrough, poly);
	}

	if(checkObjects){
		ProcessVerticalLineSectorList(sector.m_lists[ENTITYLIST_OBJECTS], line, point, mindist, entity, ignoreSeeThrough, poly);
		ProcessVerticalLineSectorList(sector.m_lists[ENTITYLIST_OBJECTS_OVERLAP], line, point, mindist, entity, ignoreSeeThrough, poly);
	}

	if(checkDummies){
		ProcessVerticalLineSectorList(sector.m_lists[ENTITYLIST_DUMMIES], line, point, mindist, entity, ignoreSeeThrough, poly);
		ProcessVerticalLineSectorList(sector.m_lists[ENTITYLIST_DUMMIES_OVERLAP], line, point, mindist, entity, ignoreSeeThrough, poly);
	}

	return mindist < 1.0f;
}

bool
CWorld::ProcessVerticalLineSectorList(CPtrList &list, const CColLine &line, CColPoint &point, float &dist, CEntity *&entity, bool ignoreSeeThrough, CStoredCollPoly *poly)
{
	float mindist = dist;
	CPtrNode *node;
	CEntity *e;
	CColModel *colmodel;

	for(node = list.first; node; node = node->next){
		e = (CEntity*)node->item;
		if(e->m_scanCode != GetCurrentScanCode() &&
		   e->bUsesCollision){
			e->m_scanCode = GetCurrentScanCode();

			colmodel = CModelInfo::GetModelInfo(e->GetModelIndex())->GetColModel();
			if(CCollision::ProcessVerticalLine(line, e->GetMatrix(), *colmodel, point, dist, ignoreSeeThrough, poly))
				entity = e;
		}
	}

	if(mindist < dist){
		dist = mindist;
		return true;
	}else
		return false;
}

bool
CWorld::GetIsLineOfSightClear(const CVector &point1, const CVector &point2, bool checkBuildings, bool checkVehicles, bool checkPeds, bool checkObjects, bool checkDummies, bool ignoreSeeThrough, bool ignoreSomeObjects)
{
	int x, xstart, xend;
	int y, ystart, yend;
	int y1, y2;

	AdvanceCurrentScanCode();

	xstart = GetSectorIndexX(point1.x);
	ystart = GetSectorIndexX(point1.y);
	xend = GetSectorIndexX(point2.x);
	yend = GetSectorIndexX(point2.y);

#define LOSARGS CColLine(point1, point2), checkBuildings, checkVehicles, checkPeds, checkObjects, checkDummies, ignoreSeeThrough, ignoreSomeObjects

	if(xstart == xend && ystart == yend){
		// Only one sector
		return GetIsLineOfSightSectorClear(*GetSector(xstart, ystart), LOSARGS);
	}else if(xstart == xend){
		// Only step in y
		if(ystart < yend){
			for(y = ystart; y <= yend; y++)
				if(!GetIsLineOfSightSectorClear(*GetSector(xstart, y), LOSARGS))
					return false;
		}else{
			for(y = ystart; y >= yend; y--)
				if(!GetIsLineOfSightSectorClear(*GetSector(xstart, y), LOSARGS))
					return false;
		}
	}else if(ystart == yend){
		// Only step in x
		if(xstart < xend){
			for(x = xstart; x <= xend; x++)
				if(!GetIsLineOfSightSectorClear(*GetSector(x, ystart), LOSARGS))
					return false;
		}else{
			for(x = xstart; x >= xend; x--)
				if(!GetIsLineOfSightSectorClear(*GetSector(x, ystart), LOSARGS))
					return false;
		}
	}else{
		if(point1.x < point2.x){
			// Step from left to right
			float m = (point2.y - point1.y) / (point2.x - point1.x);

			y1 = ystart;
			y2 = GetSectorIndexY((GetWorldX(xstart+1) - point1.x)*m + point1.y);
			if(y1 < y2){
				for(y = y1; y <= y2; y++)
					if(!GetIsLineOfSightSectorClear(*GetSector(xstart, y), LOSARGS))
						return false;
			}else{
				for(y = y1; y >= y2; y--)
					if(!GetIsLineOfSightSectorClear(*GetSector(xstart, y), LOSARGS))
						return false;
			}

			for(x = xstart+1; x < xend; x++){
				y1 = y2;
				y2 = GetSectorIndexY((GetWorldX(x+1) - point1.x)*m + point1.y);
				if(y1 < y2){
					for(y = y1; y <= y2; y++)
						if(!GetIsLineOfSightSectorClear(*GetSector(x, y), LOSARGS))
							return false;
				}else{
					for(y = y1; y >= y2; y--)
						if(!GetIsLineOfSightSectorClear(*GetSector(x, y), LOSARGS))
							return false;
				}
			}

			y1 = y2;
			y2 = yend;
			if(y1 < y2){
				for(y = y1; y <= y2; y++)
					if(!GetIsLineOfSightSectorClear(*GetSector(xend, y), LOSARGS))
						return false;
			}else{
				for(y = y1; y >= y2; y--)
					if(!GetIsLineOfSightSectorClear(*GetSector(xend, y), LOSARGS))
						return false;
			}
		}else{
			// Step from right to left
			float m = (point2.y - point1.y) / (point2.x - point1.x);

			y1 = ystart;
			y2 = GetSectorIndexY((GetWorldX(xstart) - point1.x)*m + point1.y);
			if(y1 < y2){
				for(y = y1; y <= y2; y++)
					if(!GetIsLineOfSightSectorClear(*GetSector(xstart, y), LOSARGS))
						return false;
			}else{
				for(y = y1; y >= y2; y--)
					if(!GetIsLineOfSightSectorClear(*GetSector(xstart, y), LOSARGS))
						return false;
			}

			for(x = xstart-1; x > xend; x--){
				y1 = y2;
				y2 = GetSectorIndexY((GetWorldX(x) - point1.x)*m + point1.y);
				if(y1 < y2){
					for(y = y1; y <= y2; y++)
						if(!GetIsLineOfSightSectorClear(*GetSector(x, y), LOSARGS))
							return false;
				}else{
					for(y = y1; y >= y2; y--)
						if(!GetIsLineOfSightSectorClear(*GetSector(x, y), LOSARGS))
							return false;
				}
			}

			y1 = y2;
			y2 = yend;
			if(y1 < y2){
				for(y = y1; y <= y2; y++)
					if(!GetIsLineOfSightSectorClear(*GetSector(xend, y), LOSARGS))
						return false;
			}else{
				for(y = y1; y >= y2; y--)
					if(!GetIsLineOfSightSectorClear(*GetSector(xend, y), LOSARGS))
						return false;
			}
		}
	}

	return true;

#undef LOSARGS
}

bool
CWorld::GetIsLineOfSightSectorClear(CSector &sector, const CColLine &line, bool checkBuildings, bool checkVehicles, bool checkPeds, bool checkObjects, bool checkDummies, bool ignoreSeeThrough, bool ignoreSomeObjects)
{
	if(checkBuildings){
		if(!GetIsLineOfSightSectorListClear(sector.m_lists[ENTITYLIST_BUILDINGS], line, ignoreSeeThrough))
			return false;
		if(!GetIsLineOfSightSectorListClear(sector.m_lists[ENTITYLIST_BUILDINGS_OVERLAP], line, ignoreSeeThrough))
			return false;
	}

	if(checkVehicles){
		if(!GetIsLineOfSightSectorListClear(sector.m_lists[ENTITYLIST_VEHICLES], line, ignoreSeeThrough))
			return false;
		if(!GetIsLineOfSightSectorListClear(sector.m_lists[ENTITYLIST_VEHICLES_OVERLAP], line, ignoreSeeThrough))
			return false;
	}

	if(checkPeds){
		if(!GetIsLineOfSightSectorListClear(sector.m_lists[ENTITYLIST_PEDS], line, ignoreSeeThrough))
			return false;
		if(!GetIsLineOfSightSectorListClear(sector.m_lists[ENTITYLIST_PEDS_OVERLAP], line, ignoreSeeThrough))
			return false;
	}

	if(checkObjects){
		if(!GetIsLineOfSightSectorListClear(sector.m_lists[ENTITYLIST_OBJECTS], line, ignoreSeeThrough, ignoreSomeObjects))
			return false;
		if(!GetIsLineOfSightSectorListClear(sector.m_lists[ENTITYLIST_OBJECTS_OVERLAP], line, ignoreSeeThrough, ignoreSomeObjects))
			return false;
	}

	if(checkDummies){
		if(!GetIsLineOfSightSectorListClear(sector.m_lists[ENTITYLIST_DUMMIES], line, ignoreSeeThrough))
			return false;
		if(!GetIsLineOfSightSectorListClear(sector.m_lists[ENTITYLIST_DUMMIES_OVERLAP], line, ignoreSeeThrough))
			return false;
	}

	return true;
}

bool
CWorld::GetIsLineOfSightSectorListClear(CPtrList &list, const CColLine &line, bool ignoreSeeThrough, bool ignoreSomeObjects)
{
	CPtrNode *node;
	CEntity *e;
	CColModel *colmodel;

	for(node = list.first; node; node = node->next){
		e = (CEntity*)node->item;
		if(e->m_scanCode != GetCurrentScanCode() &&
		   e->bUsesCollision){

			e->m_scanCode = GetCurrentScanCode();

			if(e != pIgnoreEntity &&
			   !(ignoreSomeObjects && CameraToIgnoreThisObject(e))){

				colmodel = CModelInfo::GetModelInfo(e->GetModelIndex())->GetColModel();

				if(CCollision::TestLineOfSight(line, e->GetMatrix(), *colmodel, ignoreSeeThrough))
					return false;
			}
		}
	}

	return true;
}

float
CWorld::FindGroundZForCoord(float x, float y)
{
	CColPoint point;
	CEntity *ent;
	if(ProcessVerticalLine(CVector(x, y, 1000.0f), -1000.0f, point, ent, true, false, false, false, true, false, nil))
		return point.point.z;
	else
		return 20.0f;
}

float
CWorld::FindGroundZFor3DCoord(float x, float y, float z, bool *found)
{
	CColPoint point;
	CEntity *ent;
	if(ProcessVerticalLine(CVector(x, y, z), -1000.0f, point, ent, true, false, false, false, false, false, nil)){
		if(found)
			*found = true;
		return point.point.z;
	}else{
		if(found)
			*found = false;
		return 0.0f;
	}
}

float
CWorld::FindRoofZFor3DCoord(float x, float y, float z, bool *found)
{
	CColPoint point;
	CEntity *ent;
	if(ProcessVerticalLine(CVector(x, y, z), 1000.0f, point, ent, true, false, false, false, true, false, nil)){
		if(found)
			*found = true;
		return point.point.z;
	}else{
		if(found == nil)
			printf("THERE IS NO MAP BELOW THE FOLLOWING COORS:%f %f %f. (FindGroundZFor3DCoord)\n", x, y, z);
		if(found)
			*found = false;
		return 20.0f;
	}
}

STARTPATCHES
	InjectHook(0x4B1F60, CWorld::ClearScanCodes, PATCH_JUMP);
	InjectHook(0x4AF970, CWorld::ProcessLineOfSight, PATCH_JUMP);
	InjectHook(0x4B0A80, CWorld::ProcessLineOfSightSector, PATCH_JUMP);
	InjectHook(0x4B0C70, CWorld::ProcessLineOfSightSectorList, PATCH_JUMP);
	InjectHook(0x4B0DE0, CWorld::ProcessVerticalLine, PATCH_JUMP);
	InjectHook(0x4B0EF0, CWorld::ProcessVerticalLineSector, PATCH_JUMP);
	InjectHook(0x4B1090, CWorld::ProcessVerticalLineSectorList, PATCH_JUMP);
	InjectHook(0x4AEAA0, CWorld::GetIsLineOfSightClear, PATCH_JUMP);
	InjectHook(0x4B2000, CWorld::GetIsLineOfSightSectorClear, PATCH_JUMP);
	InjectHook(0x4B2160, CWorld::GetIsLineOfSightSectorListClear, PATCH_JUMP);

	InjectHook(0x4B3A80, CWorld::FindGroundZForCoord, PATCH_JUMP);
	InjectHook(0x4B3AE0, CWorld::FindGroundZFor3DCoord, PATCH_JUMP);
	InjectHook(0x4B3B50, CWorld::FindRoofZFor3DCoord, PATCH_JUMP);
ENDPATCHES

WRAPPER CPed *FindPlayerPed(void) { EAXJMP(0x4A1150); }
WRAPPER CVector &FindPlayerCoors(CVector &v) { EAXJMP(0x4A1030); }
WRAPPER CVehicle *FindPlayerVehicle(void) { EAXJMP(0x4A10C0); }
WRAPPER CVehicle *FindPlayerTrain(void) { EAXJMP(0x4A1120); }
