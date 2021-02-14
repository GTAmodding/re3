#include "common.h"
#include "Camera.h"
#include "CarCtrl.h"
#include "CopPed.h"
#include "CutsceneMgr.h"
#include "DMAudio.h"
#include "EventList.h"
#include "Explosion.h"
#include "Fire.h"
#include "Garages.h"
#include "Glass.h"
#include "Messages.h"
#include "ModelIndices.h"
#include "ParticleObject.h"
#include "Population.h"
#include "ProjectileInfo.h"
#include "Record.h"
#include "References.h"
#include "Replay.h"
#include "RpAnimBlend.h"
#include "Shadows.h"
#include "TempColModels.h"
#include "WaterLevel.h"
#include "World.h"


#define OBJECT_REPOSITION_OFFSET_Z 2.0f

CColPoint gaTempSphereColPoints[MAX_COLLISION_POINTS];

CPtrList CWorld::ms_bigBuildingsList[NUM_LEVELS];
CPtrList CWorld::ms_listMovingEntityPtrs;
CSector CWorld::ms_aSectors[NUMSECTORS_Y][NUMSECTORS_X];
uint16 CWorld::ms_nCurrentScanCode;

uint8 CWorld::PlayerInFocus;
CPlayerInfo CWorld::Players[NUMPLAYERS];
bool CWorld::bNoMoreCollisionTorque;
CEntity *CWorld::pIgnoreEntity;
bool CWorld::bIncludeDeadPeds;
bool CWorld::bSecondShift;
bool CWorld::bForceProcessControl;
bool CWorld::bProcessCutsceneOnly;

bool CWorld::bDoingCarCollisions;
bool CWorld::bIncludeCarTyres;

void
CWorld::Initialise()
{
#if GTA_VERSION <= GTA3_PS2_160
	CPools::Initialise();
#endif
	pIgnoreEntity = nil;
	bDoingCarCollisions = false;
	bSecondShift = false;
	bNoMoreCollisionTorque = false;
	bProcessCutsceneOnly = false;
	bIncludeDeadPeds = false;
	bForceProcessControl = false;
	bIncludeCarTyres = false;
}

void
CWorld::Add(CEntity *ent)
{
	if(ent->IsVehicle() || ent->IsPed()) DMAudio.SetEntityStatus(((CPhysical *)ent)->m_audioEntityId, true);

	if(ent->bIsBIGBuilding)
		ms_bigBuildingsList[ent->m_level].InsertItem(ent);
	else
		ent->Add();

	if(ent->IsBuilding() || ent->IsDummy()) return;

	if(!ent->GetIsStatic()) ((CPhysical *)ent)->AddToMovingList();
}

void
CWorld::Remove(CEntity *ent)
{
	if(ent->IsVehicle() || ent->IsPed()) DMAudio.SetEntityStatus(((CPhysical *)ent)->m_audioEntityId, false);

	if(ent->bIsBIGBuilding)
		ms_bigBuildingsList[ent->m_level].RemoveItem(ent);
	else
		ent->Remove();

	if(ent->IsBuilding() || ent->IsDummy()) return;

	if(!ent->GetIsStatic()) ((CPhysical *)ent)->RemoveFromMovingList();
}

void
CWorld::ClearScanCodes(void)
{
	CPtrNode *node;
	for(int i = 0; i < NUMSECTORS_Y; i++)
		for(int j = 0; j < NUMSECTORS_X; j++) {
			CSector *s = &ms_aSectors[i][j];
			for(node = s->m_lists[ENTITYLIST_BUILDINGS].first; node; node = node->next)
				((CEntity *)node->item)->m_scanCode = 0;
			for(node = s->m_lists[ENTITYLIST_VEHICLES].first; node; node = node->next)
				((CEntity *)node->item)->m_scanCode = 0;
			for(node = s->m_lists[ENTITYLIST_PEDS].first; node; node = node->next)
				((CEntity *)node->item)->m_scanCode = 0;
			for(node = s->m_lists[ENTITYLIST_OBJECTS].first; node; node = node->next)
				((CEntity *)node->item)->m_scanCode = 0;
			for(node = s->m_lists[ENTITYLIST_DUMMIES].first; node; node = node->next)
				((CEntity *)node->item)->m_scanCode = 0;
		}
}

void
CWorld::ClearExcitingStuffFromArea(const CVector &pos, float radius, bool bRemoveProjectilesAndTidyUpShadows)
{
	CPedPool *pedPool = CPools::GetPedPool();
	for(int32 i = 0; i < pedPool->GetSize(); i++) {
		CPed *pPed = pedPool->GetSlot(i);
		if(pPed && !pPed->IsPlayer() && pPed->CanBeDeleted() &&
		   CVector2D(pPed->GetPosition() - pos).MagnitudeSqr() < SQR(radius)) {
			CPopulation::RemovePed(pPed);
		}
	}
	CVehiclePool *VehiclePool = CPools::GetVehiclePool();
	for(int32 i = 0; i < VehiclePool->GetSize(); i++) {
		CVehicle *pVehicle = VehiclePool->GetSlot(i);
		if(pVehicle && CVector2D(pVehicle->GetPosition() - pos).MagnitudeSqr() < SQR(radius) &&
		   !pVehicle->bIsLocked && pVehicle->CanBeDeleted()) {
			if(pVehicle->pDriver) {
				CPopulation::RemovePed(pVehicle->pDriver);
				pVehicle->pDriver = nil;
			}
			for(int32 j = 0; j < pVehicle->m_nNumMaxPassengers; ++j) {
				if(pVehicle->pPassengers[j]) {
					CPopulation::RemovePed(pVehicle->pPassengers[j]);
					pVehicle->pPassengers[j] = nil;
					--pVehicle->m_nNumPassengers;
				}
			}
			CCarCtrl::RemoveFromInterestingVehicleList(pVehicle);
			Remove(pVehicle);
			delete pVehicle;
		}
	}
	CObject::DeleteAllTempObjectsInArea(pos, radius);
	gFireManager.ExtinguishPoint(pos, radius);
	ExtinguishAllCarFiresInArea(pos, radius);
	CExplosion::RemoveAllExplosionsInArea(pos, radius);
	if(bRemoveProjectilesAndTidyUpShadows) {
		CProjectileInfo::RemoveAllProjectiles();
		CShadows::TidyUpShadows();
	}
}

bool
CWorld::CameraToIgnoreThisObject(CEntity *ent)
{
	if(CGarages::IsModelIndexADoor(ent->GetModelIndex())) return false;
	return ((CObject *)ent)->m_bCameraToAvoidThisObject != 1;
}

bool
CWorld::ProcessLineOfSight(const CVector &point1, const CVector &point2, CColPoint &point, CEntity *&entity,
                           bool checkBuildings, bool checkVehicles, bool checkPeds, bool checkObjects,
                           bool checkDummies, bool ignoreSeeThrough, bool ignoreSomeObjects)
{
	int x, xstart, xend;
	int y, ystart, yend;
	int y1, y2;
	float dist;

	AdvanceCurrentScanCode();

	entity = nil;
	dist = 1.0f;

	xstart = GetSectorIndexX(point1.x);
	ystart = GetSectorIndexY(point1.y);
	xend = GetSectorIndexX(point2.x);
	yend = GetSectorIndexY(point2.y);

#define LOSARGS                                                                                                        \
	CColLine(point1, point2), point, dist, entity, checkBuildings, checkVehicles, checkPeds, checkObjects,         \
	    checkDummies, ignoreSeeThrough, ignoreSomeObjects

	if(xstart == xend && ystart == yend) {
		// Only one sector
		return ProcessLineOfSightSector(*GetSector(xstart, ystart), LOSARGS);
	} else if(xstart == xend) {
		// Only step in y
		if(ystart < yend)
			for(y = ystart; y <= yend; y++) ProcessLineOfSightSector(*GetSector(xstart, y), LOSARGS);
		else
			for(y = ystart; y >= yend; y--) ProcessLineOfSightSector(*GetSector(xstart, y), LOSARGS);
		return dist < 1.0f;
	} else if(ystart == yend) {
		// Only step in x
		if(xstart < xend)
			for(x = xstart; x <= xend; x++) ProcessLineOfSightSector(*GetSector(x, ystart), LOSARGS);
		else
			for(x = xstart; x >= xend; x--) ProcessLineOfSightSector(*GetSector(x, ystart), LOSARGS);
		return dist < 1.0f;
	} else {
		if(point1.x < point2.x) {
			// Step from left to right
			float m = (point2.y - point1.y) / (point2.x - point1.x);

			y1 = ystart;
			y2 = GetSectorIndexY((GetWorldX(xstart + 1) - point1.x) * m + point1.y);
			if(y1 < y2)
				for(y = y1; y <= y2; y++) ProcessLineOfSightSector(*GetSector(xstart, y), LOSARGS);
			else
				for(y = y1; y >= y2; y--) ProcessLineOfSightSector(*GetSector(xstart, y), LOSARGS);

			for(x = xstart + 1; x < xend; x++) {
				y1 = y2;
				y2 = GetSectorIndexY((GetWorldX(x + 1) - point1.x) * m + point1.y);
				if(y1 < y2)
					for(y = y1; y <= y2; y++) ProcessLineOfSightSector(*GetSector(x, y), LOSARGS);
				else
					for(y = y1; y >= y2; y--) ProcessLineOfSightSector(*GetSector(x, y), LOSARGS);
			}

			y1 = y2;
			y2 = yend;
			if(y1 < y2)
				for(y = y1; y <= y2; y++) ProcessLineOfSightSector(*GetSector(xend, y), LOSARGS);
			else
				for(y = y1; y >= y2; y--) ProcessLineOfSightSector(*GetSector(xend, y), LOSARGS);
		} else {
			// Step from right to left
			float m = (point2.y - point1.y) / (point2.x - point1.x);

			y1 = ystart;
			y2 = GetSectorIndexY((GetWorldX(xstart) - point1.x) * m + point1.y);
			if(y1 < y2)
				for(y = y1; y <= y2; y++) ProcessLineOfSightSector(*GetSector(xstart, y), LOSARGS);
			else
				for(y = y1; y >= y2; y--) ProcessLineOfSightSector(*GetSector(xstart, y), LOSARGS);

			for(x = xstart - 1; x > xend; x--) {
				y1 = y2;
				y2 = GetSectorIndexY((GetWorldX(x) - point1.x) * m + point1.y);
				if(y1 < y2)
					for(y = y1; y <= y2; y++) ProcessLineOfSightSector(*GetSector(x, y), LOSARGS);
				else
					for(y = y1; y >= y2; y--) ProcessLineOfSightSector(*GetSector(x, y), LOSARGS);
			}

			y1 = y2;
			y2 = yend;
			if(y1 < y2)
				for(y = y1; y <= y2; y++) ProcessLineOfSightSector(*GetSector(xend, y), LOSARGS);
			else
				for(y = y1; y >= y2; y--) ProcessLineOfSightSector(*GetSector(xend, y), LOSARGS);
		}
		return dist < 1.0f;
	}

#undef LOSARGS
}

bool
CWorld::ProcessLineOfSightSector(CSector &sector, const CColLine &line, CColPoint &point, float &dist, CEntity *&entity,
                                 bool checkBuildings, bool checkVehicles, bool checkPeds, bool checkObjects,
                                 bool checkDummies, bool ignoreSeeThrough, bool ignoreSomeObjects)
{
	float mindist = dist;
	bool deadPeds = !!bIncludeDeadPeds;
	bIncludeDeadPeds = false;

	if(checkBuildings) {
		ProcessLineOfSightSectorList(sector.m_lists[ENTITYLIST_BUILDINGS], line, point, mindist, entity,
		                             ignoreSeeThrough);
		ProcessLineOfSightSectorList(sector.m_lists[ENTITYLIST_BUILDINGS_OVERLAP], line, point, mindist, entity,
		                             ignoreSeeThrough);
	}

	if(checkVehicles) {
		ProcessLineOfSightSectorList(sector.m_lists[ENTITYLIST_VEHICLES], line, point, mindist, entity,
		                             ignoreSeeThrough);
		ProcessLineOfSightSectorList(sector.m_lists[ENTITYLIST_VEHICLES_OVERLAP], line, point, mindist, entity,
		                             ignoreSeeThrough);
	}

	if(checkPeds) {
		if(deadPeds) bIncludeDeadPeds = true;
		ProcessLineOfSightSectorList(sector.m_lists[ENTITYLIST_PEDS], line, point, mindist, entity,
		                             ignoreSeeThrough);
		ProcessLineOfSightSectorList(sector.m_lists[ENTITYLIST_PEDS_OVERLAP], line, point, mindist, entity,
		                             ignoreSeeThrough);
		bIncludeDeadPeds = false;
	}

	if(checkObjects) {
		ProcessLineOfSightSectorList(sector.m_lists[ENTITYLIST_OBJECTS], line, point, mindist, entity,
		                             ignoreSeeThrough, ignoreSomeObjects);
		ProcessLineOfSightSectorList(sector.m_lists[ENTITYLIST_OBJECTS_OVERLAP], line, point, mindist, entity,
		                             ignoreSeeThrough, ignoreSomeObjects);
	}

	if(checkDummies) {
		ProcessLineOfSightSectorList(sector.m_lists[ENTITYLIST_DUMMIES], line, point, mindist, entity,
		                             ignoreSeeThrough);
		ProcessLineOfSightSectorList(sector.m_lists[ENTITYLIST_DUMMIES_OVERLAP], line, point, mindist, entity,
		                             ignoreSeeThrough);
	}

	bIncludeDeadPeds = deadPeds;

	if(mindist < dist) {
		dist = mindist;
		return true;
	} else
		return false;
}

bool
CWorld::ProcessLineOfSightSectorList(CPtrList &list, const CColLine &line, CColPoint &point, float &dist,
                                     CEntity *&entity, bool ignoreSeeThrough, bool ignoreSomeObjects)
{
	bool deadPeds = false;
	float mindist = dist;
	CPtrNode *node;
	CEntity *e;
	CColModel *colmodel;

	if(list.first && bIncludeDeadPeds && ((CEntity *)list.first->item)->IsPed()) deadPeds = true;

	for(node = list.first; node; node = node->next) {
		e = (CEntity *)node->item;
		if(e->m_scanCode != GetCurrentScanCode() && e != pIgnoreEntity && (e->bUsesCollision || deadPeds) &&
		   !(ignoreSomeObjects && CameraToIgnoreThisObject(e))) {
			colmodel = nil;
			e->m_scanCode = GetCurrentScanCode();

			if(e->IsPed()) {
				if(e->bUsesCollision || deadPeds && ((CPed *)e)->m_nPedState == PED_DEAD) {
#ifdef PED_SKIN
					if(IsClumpSkinned(e->GetClump()))
						colmodel = ((CPedModelInfo *)CModelInfo::GetModelInfo(e->GetModelIndex()))->AnimatePedColModelSkinned(e->GetClump());
					else
#endif
					if(((CPed *)e)->UseGroundColModel())
						colmodel = &CTempColModels::ms_colModelPedGroundHit;
					else
#ifdef ANIMATE_PED_COL_MODEL
						colmodel = CPedModelInfo::AnimatePedColModel(
						    ((CPedModelInfo *)CModelInfo::GetModelInfo(e->GetModelIndex()))
						        ->GetHitColModel(),
						    RpClumpGetFrame(e->GetClump()));
#else
						colmodel =
						    ((CPedModelInfo *)CModelInfo::GetModelInfo(e->GetModelIndex()))
						        ->GetHitColModel();
#endif
				} else
					colmodel = nil;
			} else if(e->bUsesCollision)
				colmodel = CModelInfo::GetModelInfo(e->GetModelIndex())->GetColModel();

			if(colmodel && CCollision::ProcessLineOfSight(line, e->GetMatrix(), *colmodel, point, mindist,
			                                              ignoreSeeThrough))
				entity = e;
		}
	}

	if(mindist < dist) {
		dist = mindist;
		return true;
	} else
		return false;
}

bool
CWorld::ProcessVerticalLine(const CVector &point1, float z2, CColPoint &point, CEntity *&entity, bool checkBuildings,
                            bool checkVehicles, bool checkPeds, bool checkObjects, bool checkDummies,
                            bool ignoreSeeThrough, CStoredCollPoly *poly)
{
	AdvanceCurrentScanCode();
	CVector point2(point1.x, point1.y, z2);
	return ProcessVerticalLineSector(*GetSector(GetSectorIndexX(point1.x), GetSectorIndexY(point1.y)),
	                                 CColLine(point1, point2), point, entity, checkBuildings, checkVehicles,
	                                 checkPeds, checkObjects, checkDummies, ignoreSeeThrough, poly);
}

bool
CWorld::ProcessVerticalLineSector(CSector &sector, const CColLine &line, CColPoint &point, CEntity *&entity,
                                  bool checkBuildings, bool checkVehicles, bool checkPeds, bool checkObjects,
                                  bool checkDummies, bool ignoreSeeThrough, CStoredCollPoly *poly)
{
	float mindist = 1.0f;

	if(checkBuildings) {
		ProcessVerticalLineSectorList(sector.m_lists[ENTITYLIST_BUILDINGS], line, point, mindist, entity,
		                              ignoreSeeThrough, poly);
		ProcessVerticalLineSectorList(sector.m_lists[ENTITYLIST_BUILDINGS_OVERLAP], line, point, mindist,
		                              entity, ignoreSeeThrough, poly);
	}

	if(checkVehicles) {
		ProcessVerticalLineSectorList(sector.m_lists[ENTITYLIST_VEHICLES], line, point, mindist, entity,
		                              ignoreSeeThrough, poly);
		ProcessVerticalLineSectorList(sector.m_lists[ENTITYLIST_VEHICLES_OVERLAP], line, point, mindist, entity,
		                              ignoreSeeThrough, poly);
	}

	if(checkPeds) {
		ProcessVerticalLineSectorList(sector.m_lists[ENTITYLIST_PEDS], line, point, mindist, entity,
		                              ignoreSeeThrough, poly);
		ProcessVerticalLineSectorList(sector.m_lists[ENTITYLIST_PEDS_OVERLAP], line, point, mindist, entity,
		                              ignoreSeeThrough, poly);
	}

	if(checkObjects) {
		ProcessVerticalLineSectorList(sector.m_lists[ENTITYLIST_OBJECTS], line, point, mindist, entity,
		                              ignoreSeeThrough, poly);
		ProcessVerticalLineSectorList(sector.m_lists[ENTITYLIST_OBJECTS_OVERLAP], line, point, mindist, entity,
		                              ignoreSeeThrough, poly);
	}

	if(checkDummies) {
		ProcessVerticalLineSectorList(sector.m_lists[ENTITYLIST_DUMMIES], line, point, mindist, entity,
		                              ignoreSeeThrough, poly);
		ProcessVerticalLineSectorList(sector.m_lists[ENTITYLIST_DUMMIES_OVERLAP], line, point, mindist, entity,
		                              ignoreSeeThrough, poly);
	}

	return mindist < 1.0f;
}

bool
CWorld::ProcessVerticalLineSectorList(CPtrList &list, const CColLine &line, CColPoint &point, float &dist,
                                      CEntity *&entity, bool ignoreSeeThrough, CStoredCollPoly *poly)
{
	float mindist = dist;
	CPtrNode *node;
	CEntity *e;
	CColModel *colmodel;

	for(node = list.first; node; node = node->next) {
		e = (CEntity *)node->item;
		if(e->m_scanCode != GetCurrentScanCode() && e->bUsesCollision) {
			e->m_scanCode = GetCurrentScanCode();

			colmodel = CModelInfo::GetModelInfo(e->GetModelIndex())->GetColModel();
			if(CCollision::ProcessVerticalLine(line, e->GetMatrix(), *colmodel, point, mindist,
			                                   ignoreSeeThrough, poly))
				entity = e;
		}
	}

	if(mindist < dist) {
		dist = mindist;
		return true;
	} else
		return false;
}

bool
CWorld::GetIsLineOfSightClear(const CVector &point1, const CVector &point2, bool checkBuildings, bool checkVehicles,
                              bool checkPeds, bool checkObjects, bool checkDummies, bool ignoreSeeThrough,
                              bool ignoreSomeObjects)
{
	int x, xstart, xend;
	int y, ystart, yend;
	int y1, y2;

	AdvanceCurrentScanCode();

	xstart = GetSectorIndexX(point1.x);
	ystart = GetSectorIndexY(point1.y);
	xend = GetSectorIndexX(point2.x);
	yend = GetSectorIndexY(point2.y);

#define LOSARGS                                                                                                        \
	CColLine(point1, point2), checkBuildings, checkVehicles, checkPeds, checkObjects, checkDummies,                \
	    ignoreSeeThrough, ignoreSomeObjects

	if(xstart == xend && ystart == yend) {
		// Only one sector
		return GetIsLineOfSightSectorClear(*GetSector(xstart, ystart), LOSARGS);
	} else if(xstart == xend) {
		// Only step in y
		if(ystart < yend) {
			for(y = ystart; y <= yend; y++)
				if(!GetIsLineOfSightSectorClear(*GetSector(xstart, y), LOSARGS)) return false;
		} else {
			for(y = ystart; y >= yend; y--)
				if(!GetIsLineOfSightSectorClear(*GetSector(xstart, y), LOSARGS)) return false;
		}
	} else if(ystart == yend) {
		// Only step in x
		if(xstart < xend) {
			for(x = xstart; x <= xend; x++)
				if(!GetIsLineOfSightSectorClear(*GetSector(x, ystart), LOSARGS)) return false;
		} else {
			for(x = xstart; x >= xend; x--)
				if(!GetIsLineOfSightSectorClear(*GetSector(x, ystart), LOSARGS)) return false;
		}
	} else {
		if(point1.x < point2.x) {
			// Step from left to right
			float m = (point2.y - point1.y) / (point2.x - point1.x);

			y1 = ystart;
			y2 = GetSectorIndexY((GetWorldX(xstart + 1) - point1.x) * m + point1.y);
			if(y1 < y2) {
				for(y = y1; y <= y2; y++)
					if(!GetIsLineOfSightSectorClear(*GetSector(xstart, y), LOSARGS)) return false;
			} else {
				for(y = y1; y >= y2; y--)
					if(!GetIsLineOfSightSectorClear(*GetSector(xstart, y), LOSARGS)) return false;
			}

			for(x = xstart + 1; x < xend; x++) {
				y1 = y2;
				y2 = GetSectorIndexY((GetWorldX(x + 1) - point1.x) * m + point1.y);
				if(y1 < y2) {
					for(y = y1; y <= y2; y++)
						if(!GetIsLineOfSightSectorClear(*GetSector(x, y), LOSARGS))
							return false;
				} else {
					for(y = y1; y >= y2; y--)
						if(!GetIsLineOfSightSectorClear(*GetSector(x, y), LOSARGS))
							return false;
				}
			}

			y1 = y2;
			y2 = yend;
			if(y1 < y2) {
				for(y = y1; y <= y2; y++)
					if(!GetIsLineOfSightSectorClear(*GetSector(xend, y), LOSARGS)) return false;
			} else {
				for(y = y1; y >= y2; y--)
					if(!GetIsLineOfSightSectorClear(*GetSector(xend, y), LOSARGS)) return false;
			}
		} else {
			// Step from right to left
			float m = (point2.y - point1.y) / (point2.x - point1.x);

			y1 = ystart;
			y2 = GetSectorIndexY((GetWorldX(xstart) - point1.x) * m + point1.y);
			if(y1 < y2) {
				for(y = y1; y <= y2; y++)
					if(!GetIsLineOfSightSectorClear(*GetSector(xstart, y), LOSARGS)) return false;
			} else {
				for(y = y1; y >= y2; y--)
					if(!GetIsLineOfSightSectorClear(*GetSector(xstart, y), LOSARGS)) return false;
			}

			for(x = xstart - 1; x > xend; x--) {
				y1 = y2;
				y2 = GetSectorIndexY((GetWorldX(x) - point1.x) * m + point1.y);
				if(y1 < y2) {
					for(y = y1; y <= y2; y++)
						if(!GetIsLineOfSightSectorClear(*GetSector(x, y), LOSARGS))
							return false;
				} else {
					for(y = y1; y >= y2; y--)
						if(!GetIsLineOfSightSectorClear(*GetSector(x, y), LOSARGS))
							return false;
				}
			}

			y1 = y2;
			y2 = yend;
			if(y1 < y2) {
				for(y = y1; y <= y2; y++)
					if(!GetIsLineOfSightSectorClear(*GetSector(xend, y), LOSARGS)) return false;
			} else {
				for(y = y1; y >= y2; y--)
					if(!GetIsLineOfSightSectorClear(*GetSector(xend, y), LOSARGS)) return false;
			}
		}
	}

	return true;

#undef LOSARGS
}

bool
CWorld::GetIsLineOfSightSectorClear(CSector &sector, const CColLine &line, bool checkBuildings, bool checkVehicles,
                                    bool checkPeds, bool checkObjects, bool checkDummies, bool ignoreSeeThrough,
                                    bool ignoreSomeObjects)
{
	if(checkBuildings) {
		if(!GetIsLineOfSightSectorListClear(sector.m_lists[ENTITYLIST_BUILDINGS], line, ignoreSeeThrough))
			return false;
		if(!GetIsLineOfSightSectorListClear(sector.m_lists[ENTITYLIST_BUILDINGS_OVERLAP], line,
		                                    ignoreSeeThrough))
			return false;
	}

	if(checkVehicles) {
		if(!GetIsLineOfSightSectorListClear(sector.m_lists[ENTITYLIST_VEHICLES], line, ignoreSeeThrough))
			return false;
		if(!GetIsLineOfSightSectorListClear(sector.m_lists[ENTITYLIST_VEHICLES_OVERLAP], line,
		                                    ignoreSeeThrough))
			return false;
	}

	if(checkPeds) {
		if(!GetIsLineOfSightSectorListClear(sector.m_lists[ENTITYLIST_PEDS], line, ignoreSeeThrough))
			return false;
		if(!GetIsLineOfSightSectorListClear(sector.m_lists[ENTITYLIST_PEDS_OVERLAP], line, ignoreSeeThrough))
			return false;
	}

	if(checkObjects) {
		if(!GetIsLineOfSightSectorListClear(sector.m_lists[ENTITYLIST_OBJECTS], line, ignoreSeeThrough,
		                                    ignoreSomeObjects))
			return false;
		if(!GetIsLineOfSightSectorListClear(sector.m_lists[ENTITYLIST_OBJECTS_OVERLAP], line, ignoreSeeThrough,
		                                    ignoreSomeObjects))
			return false;
	}

	if(checkDummies) {
		if(!GetIsLineOfSightSectorListClear(sector.m_lists[ENTITYLIST_DUMMIES], line, ignoreSeeThrough))
			return false;
		if(!GetIsLineOfSightSectorListClear(sector.m_lists[ENTITYLIST_DUMMIES_OVERLAP], line, ignoreSeeThrough))
			return false;
	}

	return true;
}

bool
CWorld::GetIsLineOfSightSectorListClear(CPtrList &list, const CColLine &line, bool ignoreSeeThrough,
                                        bool ignoreSomeObjects)
{
	CPtrNode *node;
	CEntity *e;
	CColModel *colmodel;

	for(node = list.first; node; node = node->next) {
		e = (CEntity *)node->item;
		if(e->m_scanCode != GetCurrentScanCode() && e->bUsesCollision) {

			e->m_scanCode = GetCurrentScanCode();

			if(e != pIgnoreEntity && !(ignoreSomeObjects && CameraToIgnoreThisObject(e))) {

				colmodel = CModelInfo::GetModelInfo(e->GetModelIndex())->GetColModel();

				if(CCollision::TestLineOfSight(line, e->GetMatrix(), *colmodel, ignoreSeeThrough))
					return false;
			}
		}
	}

	return true;
}

void
CWorld::FindObjectsInRangeSectorList(CPtrList &list, Const CVector &centre, float radius, bool ignoreZ, int16 *numObjects,
                                     int16 lastObject, CEntity **objects)
{
	float radiusSqr = radius * radius;
	float objDistSqr;

	for(CPtrNode *node = list.first; node; node = node->next) {
		CEntity *object = (CEntity *)node->item;
		if(object->m_scanCode != GetCurrentScanCode()) {
			object->m_scanCode = GetCurrentScanCode();

			CVector diff = centre - object->GetPosition();
			if(ignoreZ)
				objDistSqr = diff.MagnitudeSqr2D();
			else
				objDistSqr = diff.MagnitudeSqr();

			if(objDistSqr < radiusSqr && *numObjects < lastObject) {
				if(objects) { objects[*numObjects] = object; }
				(*numObjects)++;
			}
		}
	}
}

void
CWorld::FindObjectsInRange(Const CVector &centre, float radius, bool ignoreZ, int16 *numObjects, int16 lastObject,
                           CEntity **objects, bool checkBuildings, bool checkVehicles, bool checkPeds,
                           bool checkObjects, bool checkDummies)
{
	int minX = GetSectorIndexX(centre.x - radius);
	if(minX <= 0) minX = 0;

	int minY = GetSectorIndexY(centre.y - radius);
	if(minY <= 0) minY = 0;

	int maxX = GetSectorIndexX(centre.x + radius);
#ifdef FIX_BUGS
	if(maxX >= NUMSECTORS_X) maxX = NUMSECTORS_X - 1;
#else
	if(maxX >= NUMSECTORS_X) maxX = NUMSECTORS_X;
#endif

	int maxY = GetSectorIndexY(centre.y + radius);
#ifdef FIX_BUGS
	if(maxY >= NUMSECTORS_Y) maxY = NUMSECTORS_Y - 1;
#else
	if(maxY >= NUMSECTORS_Y) maxY = NUMSECTORS_Y;
#endif

	AdvanceCurrentScanCode();

	*numObjects = 0;
	for(int curY = minY; curY <= maxY; curY++) {
		for(int curX = minX; curX <= maxX; curX++) {
			CSector *sector = GetSector(curX, curY);
			if(checkBuildings) {
				FindObjectsInRangeSectorList(sector->m_lists[ENTITYLIST_BUILDINGS], centre, radius,
				                             ignoreZ, numObjects, lastObject, objects);
				FindObjectsInRangeSectorList(sector->m_lists[ENTITYLIST_BUILDINGS_OVERLAP], centre,
				                             radius, ignoreZ, numObjects, lastObject, objects);
			}
			if(checkVehicles) {
				FindObjectsInRangeSectorList(sector->m_lists[ENTITYLIST_VEHICLES], centre, radius,
				                             ignoreZ, numObjects, lastObject, objects);
				FindObjectsInRangeSectorList(sector->m_lists[ENTITYLIST_VEHICLES_OVERLAP], centre,
				                             radius, ignoreZ, numObjects, lastObject, objects);
			}
			if(checkPeds) {
				FindObjectsInRangeSectorList(sector->m_lists[ENTITYLIST_PEDS], centre, radius, ignoreZ,
				                             numObjects, lastObject, objects);
				FindObjectsInRangeSectorList(sector->m_lists[ENTITYLIST_PEDS_OVERLAP], centre, radius,
				                             ignoreZ, numObjects, lastObject, objects);
			}
			if(checkObjects) {
				FindObjectsInRangeSectorList(sector->m_lists[ENTITYLIST_OBJECTS], centre, radius,
				                             ignoreZ, numObjects, lastObject, objects);
				FindObjectsInRangeSectorList(sector->m_lists[ENTITYLIST_OBJECTS_OVERLAP], centre,
				                             radius, ignoreZ, numObjects, lastObject, objects);
			}
			if(checkDummies) {
				FindObjectsInRangeSectorList(sector->m_lists[ENTITYLIST_DUMMIES], centre, radius,
				                             ignoreZ, numObjects, lastObject, objects);
				FindObjectsInRangeSectorList(sector->m_lists[ENTITYLIST_DUMMIES_OVERLAP], centre,
				                             radius, ignoreZ, numObjects, lastObject, objects);
			}
		}
	}
}

void
CWorld::FindObjectsOfTypeInRangeSectorList(uint32 modelId, CPtrList &list, const CVector &position, float radius,
                                           bool bCheck2DOnly, int16 *nEntitiesFound, int16 maxEntitiesToFind,
                                           CEntity **aEntities)
{
	for(CPtrNode *pNode = list.first; pNode; pNode = pNode->next) {
		CEntity *pEntity = (CEntity *)pNode->item;
		if(pEntity->m_scanCode != GetCurrentScanCode()) {
			pEntity->m_scanCode = GetCurrentScanCode();
			if (modelId == pEntity->GetModelIndex()) {
				float fMagnitude = 0.0f;
				if(bCheck2DOnly)
					fMagnitude = (position - pEntity->GetPosition()).MagnitudeSqr2D();
				else
					fMagnitude = (position - pEntity->GetPosition()).MagnitudeSqr();
				if(fMagnitude < radius * radius && *nEntitiesFound < maxEntitiesToFind) {
					if(aEntities) aEntities[*nEntitiesFound] = pEntity;
					++*nEntitiesFound;
				}
			}
		}
	}
}

void
CWorld::FindObjectsOfTypeInRange(uint32 modelId, const CVector &position, float radius, bool bCheck2DOnly,
                                 int16 *nEntitiesFound, int16 maxEntitiesToFind, CEntity **aEntities, bool bBuildings,
                                 bool bVehicles, bool bPeds, bool bObjects, bool bDummies)
{
	AdvanceCurrentScanCode();
	*nEntitiesFound = 0;
	const CVector2D vecSectorStartPos(position.x - radius, position.y - radius);
	const CVector2D vecSectorEndPos(position.x + radius, position.y + radius);
	const int32 nStartX = Max(GetSectorIndexX(vecSectorStartPos.x), 0);
	const int32 nStartY = Max(GetSectorIndexY(vecSectorStartPos.y), 0);
	const int32 nEndX = Min(GetSectorIndexX(vecSectorEndPos.x), NUMSECTORS_X - 1);
	const int32 nEndY = Min(GetSectorIndexY(vecSectorEndPos.y), NUMSECTORS_Y - 1);
	for(int32 y = nStartY; y <= nEndY; y++) {
		for(int32 x = nStartX; x <= nEndX; x++) {
			CSector *pSector = GetSector(x, y);
			if(bBuildings) {
				FindObjectsOfTypeInRangeSectorList(
				    modelId, pSector->m_lists[ENTITYLIST_BUILDINGS], position, radius, bCheck2DOnly,
				    nEntitiesFound, maxEntitiesToFind, aEntities);
				FindObjectsOfTypeInRangeSectorList(
				    modelId, pSector->m_lists[ENTITYLIST_BUILDINGS_OVERLAP], position, radius,
				    bCheck2DOnly, nEntitiesFound, maxEntitiesToFind, aEntities);
			}
			if(bVehicles) {
				FindObjectsOfTypeInRangeSectorList(
				    modelId, pSector->m_lists[ENTITYLIST_VEHICLES], position, radius, bCheck2DOnly,
				    nEntitiesFound, maxEntitiesToFind, aEntities);
				FindObjectsOfTypeInRangeSectorList(
				    modelId, pSector->m_lists[ENTITYLIST_VEHICLES_OVERLAP], position, radius,
				    bCheck2DOnly, nEntitiesFound, maxEntitiesToFind, aEntities);
			}
			if(bPeds) {
				FindObjectsOfTypeInRangeSectorList(
				    modelId, pSector->m_lists[ENTITYLIST_PEDS], position, radius, bCheck2DOnly,
				    nEntitiesFound, maxEntitiesToFind, aEntities);
				FindObjectsOfTypeInRangeSectorList(
				    modelId, pSector->m_lists[ENTITYLIST_PEDS_OVERLAP], position, radius, bCheck2DOnly,
				    nEntitiesFound, maxEntitiesToFind, aEntities);
			}
			if(bObjects) {
				FindObjectsOfTypeInRangeSectorList(
				    modelId, pSector->m_lists[ENTITYLIST_OBJECTS], position, radius, bCheck2DOnly,
				    nEntitiesFound, maxEntitiesToFind, aEntities);
				FindObjectsOfTypeInRangeSectorList(
				    modelId, pSector->m_lists[ENTITYLIST_OBJECTS_OVERLAP], position, radius,
				    bCheck2DOnly, nEntitiesFound, maxEntitiesToFind, aEntities);
			}
			if(bDummies) {
				FindObjectsOfTypeInRangeSectorList(
				    modelId, pSector->m_lists[ENTITYLIST_DUMMIES], position, radius, bCheck2DOnly,
				    nEntitiesFound, maxEntitiesToFind, aEntities);
				FindObjectsOfTypeInRangeSectorList(
				    modelId, pSector->m_lists[ENTITYLIST_DUMMIES_OVERLAP], position, radius,
				    bCheck2DOnly, nEntitiesFound, maxEntitiesToFind, aEntities);
			}
		}
	}
}

CEntity *
CWorld::TestSphereAgainstWorld(CVector centre, float radius, CEntity *entityToIgnore, bool checkBuildings,
                               bool checkVehicles, bool checkPeds, bool checkObjects, bool checkDummies,
                               bool ignoreSomeObjects)
{
	CEntity *foundE = nil;

	int minX = GetSectorIndexX(centre.x - radius);
	if(minX <= 0) minX = 0;

	int minY = GetSectorIndexY(centre.y - radius);
	if(minY <= 0) minY = 0;

	int maxX = GetSectorIndexX(centre.x + radius);
#ifdef FIX_BUGS
	if(maxX >= NUMSECTORS_X) maxX = NUMSECTORS_X - 1;
#else
	if(maxX >= NUMSECTORS_X) maxX = NUMSECTORS_X;
#endif

	int maxY = GetSectorIndexY(centre.y + radius);
#ifdef FIX_BUGS
	if(maxY >= NUMSECTORS_Y) maxY = NUMSECTORS_Y - 1;
#else
	if(maxY >= NUMSECTORS_Y) maxY = NUMSECTORS_Y;
#endif

	AdvanceCurrentScanCode();

	for(int curY = minY; curY <= maxY; curY++) {
		for(int curX = minX; curX <= maxX; curX++) {
			CSector *sector = GetSector(curX, curY);
			if(checkBuildings) {
				foundE = TestSphereAgainstSectorList(sector->m_lists[ENTITYLIST_BUILDINGS], centre,
				                                     radius, entityToIgnore, false);
				if(foundE) return foundE;

				foundE = TestSphereAgainstSectorList(sector->m_lists[ENTITYLIST_BUILDINGS_OVERLAP],
				                                     centre, radius, entityToIgnore, false);
				if(foundE) return foundE;
			}
			if(checkVehicles) {
				foundE = TestSphereAgainstSectorList(sector->m_lists[ENTITYLIST_VEHICLES], centre,
				                                     radius, entityToIgnore, false);
				if(foundE) return foundE;

				foundE = TestSphereAgainstSectorList(sector->m_lists[ENTITYLIST_VEHICLES_OVERLAP],
				                                     centre, radius, entityToIgnore, false);
				if(foundE) return foundE;
			}
			if(checkPeds) {
				foundE = TestSphereAgainstSectorList(sector->m_lists[ENTITYLIST_PEDS], centre, radius,
				                                     entityToIgnore, false);
				if(foundE) return foundE;

				foundE = TestSphereAgainstSectorList(sector->m_lists[ENTITYLIST_PEDS_OVERLAP], centre,
				                                     radius, entityToIgnore, false);
				if(foundE) return foundE;
			}
			if(checkObjects) {
				foundE = TestSphereAgainstSectorList(sector->m_lists[ENTITYLIST_OBJECTS], centre,
				                                     radius, entityToIgnore, ignoreSomeObjects);
				if(foundE) return foundE;

				foundE = TestSphereAgainstSectorList(sector->m_lists[ENTITYLIST_OBJECTS_OVERLAP],
				                                     centre, radius, entityToIgnore, ignoreSomeObjects);
				if(foundE) return foundE;
			}
			if(checkDummies) {
				foundE = TestSphereAgainstSectorList(sector->m_lists[ENTITYLIST_DUMMIES], centre,
				                                     radius, entityToIgnore, false);
				if(foundE) return foundE;

				foundE = TestSphereAgainstSectorList(sector->m_lists[ENTITYLIST_DUMMIES_OVERLAP],
				                                     centre, radius, entityToIgnore, false);
				if(foundE) return foundE;
			}
		}
	}
	return foundE;
}

CEntity *
CWorld::TestSphereAgainstSectorList(CPtrList &list, CVector spherePos, float radius, CEntity *entityToIgnore,
                                    bool ignoreSomeObjects)
{
	static CColModel OurColModel;

	OurColModel.boundingSphere.center.x = 0.0f;
	OurColModel.boundingSphere.center.y = 0.0f;
	OurColModel.boundingSphere.center.z = 0.0f;
	OurColModel.boundingSphere.radius = radius;
	OurColModel.boundingBox.min.x = -radius;
	OurColModel.boundingBox.min.y = -radius;
	OurColModel.boundingBox.min.z = -radius;
	OurColModel.boundingBox.max.x = radius;
	OurColModel.boundingBox.max.y = radius;
	OurColModel.boundingBox.max.z = radius;
	OurColModel.numSpheres = 1;
	OurColModel.spheres = &OurColModel.boundingSphere;
	OurColModel.numLines = 0;
	OurColModel.numBoxes = 0;
	OurColModel.numTriangles = 0;
	OurColModel.ownsCollisionVolumes = false;

	CMatrix sphereMat;
	sphereMat.SetTranslate(spherePos);

	for(CPtrNode *node = list.first; node; node = node->next) {
		CEntity *e = (CEntity *)node->item;

		if(e->m_scanCode != GetCurrentScanCode()) {
			e->m_scanCode = GetCurrentScanCode();

			if(e != entityToIgnore && e->bUsesCollision &&
			   !(ignoreSomeObjects && CameraToIgnoreThisObject(e))) {
#ifdef FIX_BUGS
				CVector diff = spherePos - e->GetBoundCentre();
#else
				CVector diff = spherePos - e->GetPosition();
#endif
				float distance = diff.Magnitude();

				if(e->GetBoundRadius() + radius > distance) {
					CColModel *eCol = CModelInfo::GetModelInfo(e->GetModelIndex())->GetColModel();
					int collidedSpheres =
					    CCollision::ProcessColModels(sphereMat, OurColModel, e->GetMatrix(), *eCol,
					                                 gaTempSphereColPoints, nil, nil);

					if(collidedSpheres != 0 ||
					   (e->IsVehicle() && ((CVehicle *)e)->m_vehType == VEHICLE_TYPE_CAR && e->GetModelIndex() != MI_DODO &&
					    radius + eCol->boundingBox.max.x > distance)) {
						return e;
					}
				}
			}
		}
	}

	return nil;
}

float
CWorld::FindGroundZForCoord(float x, float y)
{
	CColPoint point;
	CEntity *ent;
	if(ProcessVerticalLine(CVector(x, y, 1000.0f), -1000.0f, point, ent, true, false, false, false, true, false,
	                       nil))
		return point.point.z;
	else
		return 20.0f;
}

float
CWorld::FindGroundZFor3DCoord(float x, float y, float z, bool *found)
{
	CColPoint point;
	CEntity *ent;
	if(ProcessVerticalLine(CVector(x, y, z), -1000.0f, point, ent, true, false, false, false, false, false, nil)) {
		if(found) *found = true;
		return point.point.z;
	} else {
		if(found) *found = false;
		return 0.0f;
	}
}

float
CWorld::FindRoofZFor3DCoord(float x, float y, float z, bool *found)
{
	CColPoint point;
	CEntity *ent;
	if(ProcessVerticalLine(CVector(x, y, z), 1000.0f, point, ent, true, false, false, false, true, false, nil)) {
		if(found) *found = true;
		return point.point.z;
	} else {
		if(found == nil)
			printf("THERE IS NO MAP BELOW THE FOLLOWING COORS:%f %f %f. (FindGroundZFor3DCoord)\n", x, y,
			       z);
		if(found) *found = false;
		return 20.0f;
	}
}

void
CWorld::RemoveReferencesToDeletedObject(CEntity *pDeletedObject)
{
	int32 i = CPools::GetPedPool()->GetSize();
	while(--i >= 0) {
		CPed *pPed = CPools::GetPedPool()->GetSlot(i);
		if(pPed && pPed != pDeletedObject) {
			pPed->RemoveRefsToEntity(pDeletedObject);
			if(pPed->m_pCurrentPhysSurface == pDeletedObject) pPed->m_pCurrentPhysSurface = nil;
		}
	}
	i = CPools::GetVehiclePool()->GetSize();
	while(--i >= 0) {
		CVehicle *pVehicle = CPools::GetVehiclePool()->GetSlot(i);
		if(pVehicle && pVehicle != pDeletedObject) {
			pVehicle->RemoveRefsToEntity(pDeletedObject);
			pVehicle->RemoveRefsToVehicle(pDeletedObject);
		}
	}
	i = CPools::GetObjectPool()->GetSize();
	while(--i >= 0) {
		CObject *pObject = CPools::GetObjectPool()->GetSlot(i);
		if(pObject && pObject != pDeletedObject) { pObject->RemoveRefsToEntity(pDeletedObject); }
	}
}

void
CWorld::FindObjectsKindaColliding(const CVector &position, float radius, bool bCheck2DOnly, int16 *nCollidingEntities,
                                  int16 maxEntitiesToFind, CEntity **aEntities, bool bBuildings, bool bVehicles,
                                  bool bPeds, bool bObjects, bool bDummies)
{
	AdvanceCurrentScanCode();
	*nCollidingEntities = 0;
	const CVector2D vecSectorStartPos(position.x - radius, position.y - radius);
	const CVector2D vecSectorEndPos(position.x + radius, position.y + radius);
	const int32 nStartX = Max(GetSectorIndexX(vecSectorStartPos.x), 0);
	const int32 nStartY = Max(GetSectorIndexY(vecSectorStartPos.y), 0);
	const int32 nEndX = Min(GetSectorIndexX(vecSectorEndPos.x), NUMSECTORS_X - 1);
	const int32 nEndY = Min(GetSectorIndexY(vecSectorEndPos.y), NUMSECTORS_Y - 1);
	for(int32 y = nStartY; y <= nEndY; y++) {
		for(int32 x = nStartX; x <= nEndX; x++) {
			CSector *pSector = GetSector(x, y);
			if(bBuildings) {
				FindObjectsKindaCollidingSectorList(
				    pSector->m_lists[ENTITYLIST_BUILDINGS], position, radius, bCheck2DOnly,
				    nCollidingEntities, maxEntitiesToFind, aEntities);
				FindObjectsKindaCollidingSectorList(
				    pSector->m_lists[ENTITYLIST_BUILDINGS_OVERLAP], position, radius, bCheck2DOnly,
				    nCollidingEntities, maxEntitiesToFind, aEntities);
			}
			if(bVehicles) {
				FindObjectsKindaCollidingSectorList(
				    pSector->m_lists[ENTITYLIST_VEHICLES], position, radius, bCheck2DOnly,
				    nCollidingEntities, maxEntitiesToFind, aEntities);
				FindObjectsKindaCollidingSectorList(
				    pSector->m_lists[ENTITYLIST_VEHICLES_OVERLAP], position, radius, bCheck2DOnly,
				    nCollidingEntities, maxEntitiesToFind, aEntities);
			}
			if(bPeds) {
				FindObjectsKindaCollidingSectorList(pSector->m_lists[ENTITYLIST_PEDS], position,
				                                            radius, bCheck2DOnly, nCollidingEntities,
				                                            maxEntitiesToFind, aEntities);
				FindObjectsKindaCollidingSectorList(
				    pSector->m_lists[ENTITYLIST_PEDS_OVERLAP], position, radius, bCheck2DOnly,
				    nCollidingEntities, maxEntitiesToFind, aEntities);
			}
			if(bObjects) {
				FindObjectsKindaCollidingSectorList(
				    pSector->m_lists[ENTITYLIST_OBJECTS], position, radius, bCheck2DOnly,
				    nCollidingEntities, maxEntitiesToFind, aEntities);
				FindObjectsKindaCollidingSectorList(
				    pSector->m_lists[ENTITYLIST_OBJECTS_OVERLAP], position, radius, bCheck2DOnly,
				    nCollidingEntities, maxEntitiesToFind, aEntities);
			}
			if(bDummies) {
				FindObjectsKindaCollidingSectorList(
				    pSector->m_lists[ENTITYLIST_DUMMIES], position, radius, bCheck2DOnly,
				    nCollidingEntities, maxEntitiesToFind, aEntities);
				FindObjectsKindaCollidingSectorList(
				    pSector->m_lists[ENTITYLIST_DUMMIES_OVERLAP], position, radius, bCheck2DOnly,
				    nCollidingEntities, maxEntitiesToFind, aEntities);
			}
		}
	}
}

void
CWorld::FindObjectsKindaCollidingSectorList(CPtrList &list, const CVector &position, float radius, bool bCheck2DOnly,
                                            int16 *nCollidingEntities, int16 maxEntitiesToFind, CEntity **aEntities)
{
	for(CPtrNode *pNode = list.first; pNode; pNode = pNode->next) {
		CEntity *pEntity = (CEntity *)pNode->item;
		if(pEntity->m_scanCode != GetCurrentScanCode()) {
			pEntity->m_scanCode = GetCurrentScanCode();
			float fMagnitude = 0.0f;
			if(bCheck2DOnly)
				fMagnitude = (position - pEntity->GetPosition()).Magnitude2D();
			else
				fMagnitude = (position - pEntity->GetPosition()).Magnitude();
			if(pEntity->GetBoundRadius() + radius > fMagnitude && *nCollidingEntities < maxEntitiesToFind) {
				if(aEntities) aEntities[*nCollidingEntities] = pEntity;
				++*nCollidingEntities;
			}
		}
	}
}

void
CWorld::FindObjectsIntersectingCube(const CVector &vecStartPos, const CVector &vecEndPos, int16 *nIntersecting,
                                    int16 maxEntitiesToFind, CEntity **aEntities, bool bBuildings, bool bVehicles,
                                    bool bPeds, bool bObjects, bool bDummies)
{
	AdvanceCurrentScanCode();
	*nIntersecting = 0;
	const int32 nStartX = Max(GetSectorIndexX(vecStartPos.x), 0);
	const int32 nStartY = Max(GetSectorIndexY(vecStartPos.y), 0);
	const int32 nEndX = Min(GetSectorIndexX(vecEndPos.x), NUMSECTORS_X - 1);
	const int32 nEndY = Min(GetSectorIndexY(vecEndPos.y), NUMSECTORS_Y - 1);
	for(int32 y = nStartY; y <= nEndY; y++) {
		for(int32 x = nStartX; x <= nEndX; x++) {
			CSector *pSector = GetSector(x, y);
			if(bBuildings) {
				FindObjectsIntersectingCubeSectorList(pSector->m_lists[ENTITYLIST_BUILDINGS],
				                                              vecStartPos, vecEndPos, nIntersecting,
				                                              maxEntitiesToFind, aEntities);
				FindObjectsIntersectingCubeSectorList(
				    pSector->m_lists[ENTITYLIST_BUILDINGS_OVERLAP], vecStartPos, vecEndPos,
				    nIntersecting, maxEntitiesToFind, aEntities);
			}
			if(bVehicles) {
				FindObjectsIntersectingCubeSectorList(pSector->m_lists[ENTITYLIST_VEHICLES],
				                                              vecStartPos, vecEndPos, nIntersecting,
				                                              maxEntitiesToFind, aEntities);
				FindObjectsIntersectingCubeSectorList(
				    pSector->m_lists[ENTITYLIST_VEHICLES_OVERLAP], vecStartPos, vecEndPos,
				    nIntersecting, maxEntitiesToFind, aEntities);
			}
			if(bPeds) {
				FindObjectsIntersectingCubeSectorList(pSector->m_lists[ENTITYLIST_PEDS],
				                                              vecStartPos, vecEndPos, nIntersecting,
				                                              maxEntitiesToFind, aEntities);
				FindObjectsIntersectingCubeSectorList(pSector->m_lists[ENTITYLIST_PEDS_OVERLAP],
				                                              vecStartPos, vecEndPos, nIntersecting,
				                                              maxEntitiesToFind, aEntities);
			}
			if(bObjects) {
				FindObjectsIntersectingCubeSectorList(pSector->m_lists[ENTITYLIST_OBJECTS],
				                                              vecStartPos, vecEndPos, nIntersecting,
				                                              maxEntitiesToFind, aEntities);
				FindObjectsIntersectingCubeSectorList(
				    pSector->m_lists[ENTITYLIST_OBJECTS_OVERLAP], vecStartPos, vecEndPos, nIntersecting,
				    maxEntitiesToFind, aEntities);
			}
			if(bDummies) {
				FindObjectsIntersectingCubeSectorList(pSector->m_lists[ENTITYLIST_DUMMIES],
				                                              vecStartPos, vecEndPos, nIntersecting,
				                                              maxEntitiesToFind, aEntities);
				FindObjectsIntersectingCubeSectorList(
				    pSector->m_lists[ENTITYLIST_DUMMIES_OVERLAP], vecStartPos, vecEndPos, nIntersecting,
				    maxEntitiesToFind, aEntities);
			}
		}
	}
}

void
CWorld::FindObjectsIntersectingCubeSectorList(CPtrList &list, const CVector &vecStartPos, const CVector &vecEndPos,
                                              int16 *nIntersecting, int16 maxEntitiesToFind, CEntity **aEntities)
{
	for(CPtrNode *pNode = list.first; pNode; pNode = pNode->next) {
		CEntity *pEntity = (CEntity *)pNode->item;
		if(pEntity->m_scanCode != GetCurrentScanCode()) {
			pEntity->m_scanCode = GetCurrentScanCode();
			float fRadius = pEntity->GetBoundRadius();
			const CVector &entityPos = pEntity->GetPosition();
			if(fRadius + entityPos.x >= vecStartPos.x && entityPos.x - fRadius <= vecEndPos.x &&
			   fRadius + entityPos.y >= vecStartPos.y && entityPos.y - fRadius <= vecEndPos.y &&
			   fRadius + entityPos.z >= vecStartPos.z && entityPos.z - fRadius <= vecEndPos.z &&
			   *nIntersecting < maxEntitiesToFind) {
				if(aEntities) aEntities[*nIntersecting] = pEntity;
				++*nIntersecting;
			}
		}
	}
}

void
CWorld::FindObjectsIntersectingAngledCollisionBox(const CColBox &boundingBox, const CMatrix &matrix,
                                                  const CVector &position, float fStartX, float fStartY, float fEndX,
                                                  float fEndY, int16 *nEntitiesFound, int16 maxEntitiesToFind,
                                                  CEntity **aEntities, bool bBuildings, bool bVehicles, bool bPeds,
                                                  bool bObjects, bool bDummies)
{
	AdvanceCurrentScanCode();
	*nEntitiesFound = 0;
	const int32 nStartX = Max(GetSectorIndexX(fStartX), 0);
	const int32 nStartY = Max(GetSectorIndexY(fStartY), 0);
	const int32 nEndX = Min(GetSectorIndexX(fEndX), NUMSECTORS_X - 1);
	const int32 nEndY = Min(GetSectorIndexY(fEndY), NUMSECTORS_Y - 1);
	for(int32 y = nStartY; y <= nEndY; y++) {
		for(int32 x = nStartX; x <= nEndX; x++) {
			CSector *pSector = GetSector(x, y);
			if(bBuildings) {
				FindObjectsIntersectingAngledCollisionBoxSectorList(
				    pSector->m_lists[ENTITYLIST_BUILDINGS], boundingBox, matrix, position,
				    nEntitiesFound, maxEntitiesToFind, aEntities);
				FindObjectsIntersectingAngledCollisionBoxSectorList(
				    pSector->m_lists[ENTITYLIST_BUILDINGS_OVERLAP], boundingBox, matrix, position,
				    nEntitiesFound, maxEntitiesToFind, aEntities);
			}
			if(bVehicles) {
				FindObjectsIntersectingAngledCollisionBoxSectorList(
				    pSector->m_lists[ENTITYLIST_VEHICLES], boundingBox, matrix, position,
				    nEntitiesFound, maxEntitiesToFind, aEntities);
				FindObjectsIntersectingAngledCollisionBoxSectorList(
				    pSector->m_lists[ENTITYLIST_VEHICLES_OVERLAP], boundingBox, matrix, position,
				    nEntitiesFound, maxEntitiesToFind, aEntities);
			}
			if(bPeds) {
				FindObjectsIntersectingAngledCollisionBoxSectorList(
				    pSector->m_lists[ENTITYLIST_PEDS], boundingBox, matrix, position, nEntitiesFound,
				    maxEntitiesToFind, aEntities);
				FindObjectsIntersectingAngledCollisionBoxSectorList(
				    pSector->m_lists[ENTITYLIST_PEDS_OVERLAP], boundingBox, matrix, position,
				    nEntitiesFound, maxEntitiesToFind, aEntities);
			}
			if(bObjects) {
				FindObjectsIntersectingAngledCollisionBoxSectorList(
				    pSector->m_lists[ENTITYLIST_OBJECTS], boundingBox, matrix, position, nEntitiesFound,
				    maxEntitiesToFind, aEntities);
				FindObjectsIntersectingAngledCollisionBoxSectorList(
				    pSector->m_lists[ENTITYLIST_OBJECTS_OVERLAP], boundingBox, matrix, position,
				    nEntitiesFound, maxEntitiesToFind, aEntities);
			}
			if(bDummies) {
				FindObjectsIntersectingAngledCollisionBoxSectorList(
				    pSector->m_lists[ENTITYLIST_DUMMIES], boundingBox, matrix, position, nEntitiesFound,
				    maxEntitiesToFind, aEntities);
				FindObjectsIntersectingAngledCollisionBoxSectorList(
				    pSector->m_lists[ENTITYLIST_DUMMIES_OVERLAP], boundingBox, matrix, position,
				    nEntitiesFound, maxEntitiesToFind, aEntities);
			}
		}
	}
}

void
CWorld::FindObjectsIntersectingAngledCollisionBoxSectorList(CPtrList &list, const CColBox &boundingBox,
                                                            const CMatrix &matrix, const CVector &position,
                                                            int16 *nEntitiesFound, int16 maxEntitiesToFind,
                                                            CEntity **aEntities)
{
	for(CPtrNode *pNode = list.first; pNode; pNode = pNode->next) {
		CEntity *pEntity = (CEntity *)pNode->item;
		if(pEntity->m_scanCode != GetCurrentScanCode()) {
			pEntity->m_scanCode = GetCurrentScanCode();
			CColSphere sphere;
			CVector vecDistance = pEntity->GetPosition() - position;
			sphere.radius = pEntity->GetBoundRadius();
			sphere.center = Multiply3x3(vecDistance, matrix);
			if(CCollision::TestSphereBox(sphere, boundingBox) && *nEntitiesFound < maxEntitiesToFind) {
				if(aEntities) aEntities[*nEntitiesFound] = pEntity;
				++*nEntitiesFound;
			}
		}
	}
}

void
CWorld::FindMissionEntitiesIntersectingCube(const CVector &vecStartPos, const CVector &vecEndPos, int16 *nIntersecting,
                                            int16 maxEntitiesToFind, CEntity **aEntities, bool bVehicles, bool bPeds,
                                            bool bObjects)
{
	AdvanceCurrentScanCode();
	*nIntersecting = 0;
	const int32 nStartX = Max(GetSectorIndexX(vecStartPos.x), 0);
	const int32 nStartY = Max(GetSectorIndexY(vecStartPos.y), 0);
	const int32 nEndX = Min(GetSectorIndexX(vecEndPos.x), NUMSECTORS_X - 1);
	const int32 nEndY = Min(GetSectorIndexY(vecEndPos.y), NUMSECTORS_Y - 1);
	for(int32 y = nStartY; y <= nEndY; y++) {
		for(int32 x = nStartX; x <= nEndX; x++) {
			CSector *pSector = GetSector(x, y);
			if(bVehicles) {
				FindMissionEntitiesIntersectingCubeSectorList(
				    pSector->m_lists[ENTITYLIST_VEHICLES], vecStartPos, vecEndPos, nIntersecting,
				    maxEntitiesToFind, aEntities, true, false);
				FindMissionEntitiesIntersectingCubeSectorList(
				    pSector->m_lists[ENTITYLIST_VEHICLES_OVERLAP], vecStartPos, vecEndPos,
				    nIntersecting, maxEntitiesToFind, aEntities, true, false);
			}
			if(bPeds) {
				FindMissionEntitiesIntersectingCubeSectorList(
				    pSector->m_lists[ENTITYLIST_PEDS], vecStartPos, vecEndPos, nIntersecting,
				    maxEntitiesToFind, aEntities, false, true);
				FindMissionEntitiesIntersectingCubeSectorList(
				    pSector->m_lists[ENTITYLIST_PEDS_OVERLAP], vecStartPos, vecEndPos, nIntersecting,
				    maxEntitiesToFind, aEntities, false, true);
			}
			if(bObjects) {
				FindMissionEntitiesIntersectingCubeSectorList(
				    pSector->m_lists[ENTITYLIST_OBJECTS], vecStartPos, vecEndPos, nIntersecting,
				    maxEntitiesToFind, aEntities, false, false);
				FindMissionEntitiesIntersectingCubeSectorList(
				    pSector->m_lists[ENTITYLIST_OBJECTS_OVERLAP], vecStartPos, vecEndPos, nIntersecting,
				    maxEntitiesToFind, aEntities, false, false);
			}
		}
	}
}

void
CWorld::FindMissionEntitiesIntersectingCubeSectorList(CPtrList &list, const CVector &vecStartPos,
                                                      const CVector &vecEndPos, int16 *nIntersecting,
                                                      int16 maxEntitiesToFind, CEntity **aEntities, bool bIsVehicleList,
                                                      bool bIsPedList)
{
	for(CPtrNode *pNode = list.first; pNode; pNode = pNode->next) {
		CEntity *pEntity = (CEntity *)pNode->item;
		if(pEntity->m_scanCode != GetCurrentScanCode()) {
			pEntity->m_scanCode = GetCurrentScanCode();
			bool bIsMissionEntity = false;
			if(bIsVehicleList)
				bIsMissionEntity = ((CVehicle *)pEntity)->VehicleCreatedBy == MISSION_VEHICLE;
			else if(bIsPedList)
				bIsMissionEntity = ((CPed *)pEntity)->CharCreatedBy == MISSION_CHAR;
			else
				bIsMissionEntity = ((CObject *)pEntity)->ObjectCreatedBy == MISSION_OBJECT;
			float fRadius = pEntity->GetBoundRadius();
			const CVector &entityPos = pEntity->GetPosition();
			if(bIsMissionEntity && fRadius + entityPos.x >= vecStartPos.x &&
			   entityPos.x - fRadius <= vecEndPos.x && fRadius + entityPos.y >= vecStartPos.y &&
			   entityPos.y - fRadius <= vecEndPos.y && fRadius + entityPos.z >= vecStartPos.z &&
			   entityPos.z - fRadius <= vecEndPos.z && *nIntersecting < maxEntitiesToFind) {
				if(aEntities) aEntities[*nIntersecting] = pEntity;
				++*nIntersecting;
			}
		}
	}
}

void
CWorld::ClearCarsFromArea(float x1, float y1, float z1, float x2, float y2, float z2)
{
	CVehiclePool *pVehiclePool = CPools::GetVehiclePool();
	for(int32 i = 0; i < pVehiclePool->GetSize(); i++) {
		CVehicle *pVehicle = CPools::GetVehiclePool()->GetSlot(i);
		if(pVehicle) {
			const CVector &position = pVehicle->GetPosition();
			if(position.x >= x1 && position.x <= x2 && position.y >= y1 && position.y <= y2 &&
			   position.z >= z1 && position.z <= z2 && !pVehicle->bIsLocked && pVehicle->CanBeDeleted()) {
				if(pVehicle->pDriver) {
					CPopulation::RemovePed(pVehicle->pDriver);
					pVehicle->pDriver = nil;
				}
				for(int32 j = 0; j < pVehicle->m_nNumMaxPassengers; ++j) {
					if(pVehicle->pPassengers[j]) {
						CPopulation::RemovePed(pVehicle->pPassengers[j]);
						pVehicle->pPassengers[j] = nil;
						--pVehicle->m_nNumPassengers;
					}
				}
				CCarCtrl::RemoveFromInterestingVehicleList(pVehicle);
				Remove(pVehicle);
				delete pVehicle;
			}
		}
	}
}

void
CWorld::ClearPedsFromArea(float x1, float y1, float z1, float x2, float y2, float z2)
{
	CPedPool *pPedPool = CPools::GetPedPool();
	for(int32 i = 0; i < pPedPool->GetSize(); i++) {
		CPed *pPed = CPools::GetPedPool()->GetSlot(i);
		if(pPed) {
			const CVector &position = pPed->GetPosition();
			if(!pPed->IsPlayer() && pPed->CanBeDeleted() && position.x >= x1 && position.x <= x2 &&
			   position.y >= y1 && position.y <= y2 && position.z >= z1 && position.z <= z2) {
				CPopulation::RemovePed(pPed);
			}
		}
	}
}

void
CWorld::CallOffChaseForArea(float x1, float y1, float x2, float y2)
{
	AdvanceCurrentScanCode();
	float fStartX = x1 - 10.0f;
	float fStartY = y1 - 10.0f;
	float fEndX = x2 + 10.0f;
	float fEndY = y2 + 10.0f;
	const int32 nStartX = Max(GetSectorIndexX(fStartX), 0);
	const int32 nStartY = Max(GetSectorIndexY(fStartY), 0);
	const int32 nEndX = Min(GetSectorIndexX(fEndX), NUMSECTORS_X - 1);
	const int32 nEndY = Min(GetSectorIndexY(fEndY), NUMSECTORS_Y - 1);
	for(int32 y = nStartY; y <= nEndY; y++) {
		for(int32 x = nStartX; x <= nEndX; x++) {
			CSector *pSector = GetSector(x, y);
			CallOffChaseForAreaSectorListVehicles(pSector->m_lists[ENTITYLIST_VEHICLES], x1, y1, x2,
			                                              y2, fStartX, fStartY, fEndX, fEndY);
			CallOffChaseForAreaSectorListVehicles(pSector->m_lists[ENTITYLIST_VEHICLES_OVERLAP], x1,
			                                              y1, x2, y2, fStartX, fStartY, fEndX, fEndY);
			CallOffChaseForAreaSectorListPeds(pSector->m_lists[ENTITYLIST_PEDS], x1, y1, x2, y2);
			CallOffChaseForAreaSectorListPeds(pSector->m_lists[ENTITYLIST_PEDS_OVERLAP], x1, y1, x2,
			                                          y2);
		}
	}
}

void
CWorld::CallOffChaseForAreaSectorListVehicles(CPtrList &list, float x1, float y1, float x2, float y2, float fStartX,
                                              float fStartY, float fEndX, float fEndY)
{
	for(CPtrNode *pNode = list.first; pNode; pNode = pNode->next) {
		CVehicle *pVehicle = (CVehicle *)pNode->item;
		if(pVehicle->m_scanCode != GetCurrentScanCode()) {
			pVehicle->m_scanCode = GetCurrentScanCode();
			const CVector &vehiclePos = pVehicle->GetPosition();
			uint8 carMission = pVehicle->AutoPilot.m_nCarMission;
			if(pVehicle != FindPlayerVehicle() && vehiclePos.x > fStartX && vehiclePos.x < fEndX &&
			   vehiclePos.y > fStartY && vehiclePos.y < fEndY && pVehicle->bIsLawEnforcer &&
			   (carMission == MISSION_RAMPLAYER_FARAWAY || carMission == MISSION_RAMPLAYER_CLOSE ||
			    carMission == MISSION_BLOCKPLAYER_FARAWAY || carMission == MISSION_BLOCKPLAYER_CLOSE)) {
				pVehicle->AutoPilot.m_nTimeTempAction = CTimer::GetTimeInMilliseconds() + 2000;
				CColModel *pColModel = pVehicle->GetColModel();
				bool bInsideSphere = false;
				for(int32 i = 0; i < pColModel->numSpheres; i++) {
					CVector pos = pVehicle->m_matrix * pColModel->spheres[i].center;
					float fRadius = pColModel->spheres[i].radius;
					if(pos.x + fRadius > x1 && pos.x - fRadius < x2 && pos.y + fRadius > y1 &&
					   pos.y - fRadius < y2)
						bInsideSphere = true;
					// Maybe break the loop when bInsideSphere is set to true?
				}
				if(bInsideSphere) {
					if(pVehicle->GetPosition().x <= (x1 + x2) * 0.5f)
						pVehicle->m_vecMoveSpeed.x = Min(pVehicle->m_vecMoveSpeed.x, 0.0f);
					else
						pVehicle->m_vecMoveSpeed.x = Max(pVehicle->m_vecMoveSpeed.x, 0.0f);
					if(pVehicle->GetPosition().y <= (y1 + y2) * 0.5f)
						pVehicle->m_vecMoveSpeed.y = Min(pVehicle->m_vecMoveSpeed.y, 0.0f);
					else
						pVehicle->m_vecMoveSpeed.y = Max(pVehicle->m_vecMoveSpeed.y, 0.0f);
				}
			}
		}
	}
}

void
CWorld::CallOffChaseForAreaSectorListPeds(CPtrList &list, float x1, float y1, float x2, float y2)
{
	for(CPtrNode *pNode = list.first; pNode; pNode = pNode->next) {
		CPed *pPed = (CPed *)pNode->item;
		const CVector &pedPos = pPed->GetPosition();
		if(pPed->m_scanCode != GetCurrentScanCode()) {
			pPed->m_scanCode = GetCurrentScanCode();
			if(pPed != FindPlayerPed() && pPed->m_leader != FindPlayerPed() && pedPos.x > x1 &&
			   pedPos.x < x2 && pedPos.y > y1 && pedPos.y < y2 &&
			   (pPed->m_pedInObjective == FindPlayerPed() ||
			    pPed->m_carInObjective && pPed->m_carInObjective == FindPlayerVehicle()) &&
			   pPed->m_nPedState != PED_DEAD && pPed->m_nPedState != PED_DIE &&
			   (pPed->m_objective == OBJECTIVE_KILL_CHAR_ON_FOOT ||
			    pPed->m_objective == OBJECTIVE_ENTER_CAR_AS_DRIVER ||
			    pPed->m_objective == OBJECTIVE_KILL_CHAR_ANY_MEANS)) {
				if(pPed->IsPedInControl()) {
					if(pPed->m_nPedType == PEDTYPE_COP)
						((CCopPed *)pPed)->ClearPursuit();
					else
						pPed->SetIdle();
					pPed->SetObjective(OBJECTIVE_NONE);
				} else {
					pPed->m_prevObjective = OBJECTIVE_NONE;
					pPed->m_nLastPedState = PED_IDLE;
				}
			}
		}
	}
}

void
CWorld::RemoveEntityInsteadOfProcessingIt(CEntity *ent)
{
	if(ent->IsPed()) {
		if(FindPlayerPed() == ent)
			Remove(ent);
		else
			CPopulation::RemovePed((CPed *)ent);
	} else {
		Remove(ent);
		delete ent;
	}
}

void
CWorld::RemoveFallenPeds(void)
{
	int poolSize = CPools::GetPedPool()->GetSize();
	for(int poolIndex = poolSize - 1; poolIndex >= 0; poolIndex--) {
		CPed *ped = CPools::GetPedPool()->GetSlot(poolIndex);
		if(ped) {
			if(ped->GetPosition().z < MAP_Z_LOW_LIMIT) {
				if(ped->CharCreatedBy != RANDOM_CHAR || ped->IsPlayer()) {
					int closestNode = ThePaths.FindNodeClosestToCoors(ped->GetPosition(), PATH_PED,
					                                                  999999.9f, false, false);
					CVector newPos = ThePaths.m_pathNodes[closestNode].GetPosition();
					newPos.z += 2.0f;
					ped->Teleport(newPos);
					ped->m_vecMoveSpeed = CVector(0.0f, 0.0f, 0.0f);
				} else {
					CPopulation::RemovePed(ped);
				}
			}
		}
	}
}

void
CWorld::RemoveFallenCars(void)
{
	int poolSize = CPools::GetVehiclePool()->GetSize();
	for(int poolIndex = poolSize - 1; poolIndex >= 0; poolIndex--) {
		CVehicle *veh = CPools::GetVehiclePool()->GetSlot(poolIndex);
		if(veh) {
			if(veh->GetPosition().z < MAP_Z_LOW_LIMIT) {
				if(veh->VehicleCreatedBy == MISSION_VEHICLE || veh == FindPlayerVehicle() ||
				   (veh->pDriver && veh->pDriver->IsPlayer())) {
					int closestNode = ThePaths.FindNodeClosestToCoors(veh->GetPosition(), PATH_CAR,
					                                                  999999.9f, false, false);
					CVector newPos = ThePaths.m_pathNodes[closestNode].GetPosition();
					newPos.z += 3.0f;
					veh->Teleport(newPos);
					veh->m_vecMoveSpeed = CVector(0.0f, 0.0f, 0.0f);
				} else if(veh->VehicleCreatedBy == RANDOM_VEHICLE ||
				          veh->VehicleCreatedBy == PARKED_VEHICLE) {
					Remove(veh);
					delete veh;
				}
			}
		}
	}
}

void
CWorld::StopAllLawEnforcersInTheirTracks(void)
{
	int poolSize = CPools::GetVehiclePool()->GetSize();
	for(int poolIndex = poolSize - 1; poolIndex >= 0; poolIndex--) {
		CVehicle *veh = CPools::GetVehiclePool()->GetSlot(poolIndex);
		if(veh) {
			if(veh->bIsLawEnforcer) veh->SetMoveSpeed(0.0f, 0.0f, 0.0f);
		}
	}
}

void
CWorld::SetAllCarsCanBeDamaged(bool toggle)
{
	int poolSize = CPools::GetVehiclePool()->GetSize();
	for(int poolIndex = 0; poolIndex < poolSize; poolIndex++) {
		CVehicle *veh = CPools::GetVehiclePool()->GetSlot(poolIndex);
		if(veh) veh->bCanBeDamaged = toggle;
	}
}

void
CWorld::ExtinguishAllCarFiresInArea(CVector point, float range)
{
	int poolSize = CPools::GetVehiclePool()->GetSize();
	for(int poolIndex = 0; poolIndex < poolSize; poolIndex++) {
		CVehicle *veh = CPools::GetVehiclePool()->GetSlot(poolIndex);
		if(veh) {
			if((point - veh->GetPosition()).MagnitudeSqr() < sq(range)) veh->ExtinguishCarFire();
		}
	}
}

inline void 
AddSteamsFromGround(CPtrList& list) 
{
	CPtrNode *pNode = list.first;
	while (pNode) {
		((CEntity*)pNode->item)->AddSteamsFromGround(nil);
		pNode = pNode->next;
	}
}

void
CWorld::AddParticles(void)
{
	for(int32 y = 0; y < NUMSECTORS_Y; y++) {
		for(int32 x = 0; x < NUMSECTORS_X; x++) {
			CSector *pSector = GetSector(x, y);
			AddSteamsFromGround(pSector->m_lists[ENTITYLIST_BUILDINGS]);
			AddSteamsFromGround(pSector->m_lists[ENTITYLIST_DUMMIES]);
		}
	}
}

void
CWorld::ShutDown(void)
{
	for(int i = 0; i < NUMSECTORS_X * NUMSECTORS_Y; i++) {
		CSector *pSector = GetSector(i % NUMSECTORS_X, i / NUMSECTORS_Y);
		for(CPtrNode *pNode = pSector->m_lists[ENTITYLIST_BUILDINGS].first; pNode; pNode = pNode->next) {
			CEntity *pEntity = (CEntity *)pNode->item;
			Remove(pEntity);
			delete pEntity;
		}
		for(CPtrNode *pNode = pSector->m_lists[ENTITYLIST_VEHICLES].first; pNode; pNode = pNode->next) {
			CEntity *pEntity = (CEntity *)pNode->item;
			Remove(pEntity);
			delete pEntity;
		}
		for(CPtrNode *pNode = pSector->m_lists[ENTITYLIST_PEDS].first; pNode; pNode = pNode->next) {
			CEntity *pEntity = (CEntity *)pNode->item;
			Remove(pEntity);
			delete pEntity;
		}
		for(CPtrNode *pNode = pSector->m_lists[ENTITYLIST_OBJECTS].first; pNode; pNode = pNode->next) {
			CEntity *pEntity = (CEntity *)pNode->item;
			Remove(pEntity);
			delete pEntity;
		}
		for(CPtrNode *pNode = pSector->m_lists[ENTITYLIST_DUMMIES].first; pNode; pNode = pNode->next) {
			CEntity *pEntity = (CEntity *)pNode->item;
			Remove(pEntity);
			delete pEntity;
		}
#ifndef FIX_BUGS
		pSector->m_lists[ENTITYLIST_BUILDINGS].Flush();
		pSector->m_lists[ENTITYLIST_BUILDINGS_OVERLAP].Flush();
		pSector->m_lists[ENTITYLIST_DUMMIES].Flush();
		pSector->m_lists[ENTITYLIST_DUMMIES_OVERLAP].Flush();
#endif
	}
	for(int32 i = 0; i < NUM_LEVELS; i++) {
		for(CPtrNode *pNode = ms_bigBuildingsList[i].first; pNode; pNode = pNode->next) {
			CEntity *pEntity = (CEntity *)pNode->item;
			// Maybe remove from world here?
			delete pEntity;
		}
		ms_bigBuildingsList[i].Flush();
	}
	for(int i = 0; i < NUMSECTORS_X * NUMSECTORS_Y; i++) {
		CSector *pSector = GetSector(i % NUMSECTORS_X, i / NUMSECTORS_Y);
#ifdef FIX_BUGS
		pSector->m_lists[ENTITYLIST_BUILDINGS].Flush();
		pSector->m_lists[ENTITYLIST_BUILDINGS_OVERLAP].Flush();
		pSector->m_lists[ENTITYLIST_DUMMIES].Flush();
		pSector->m_lists[ENTITYLIST_DUMMIES_OVERLAP].Flush();
#endif
		if(pSector->m_lists[ENTITYLIST_BUILDINGS].first) {
			sprintf(gString, "Building list %d,%d not empty\n", i % NUMSECTORS_X, i / NUMSECTORS_Y);
			pSector->m_lists[ENTITYLIST_BUILDINGS].Flush();
		}
		if(pSector->m_lists[ENTITYLIST_DUMMIES].first) {
			sprintf(gString, "Dummy list %d,%d not empty\n", i % NUMSECTORS_X, i / NUMSECTORS_Y);
			pSector->m_lists[ENTITYLIST_DUMMIES].Flush();
		}
		if(pSector->m_lists[ENTITYLIST_BUILDINGS_OVERLAP].first) {
			sprintf(gString, "Building overlap list %d,%d not empty\n", i % NUMSECTORS_X, i / NUMSECTORS_Y);
			pSector->m_lists[ENTITYLIST_BUILDINGS_OVERLAP].Flush();
		}
		if(pSector->m_lists[ENTITYLIST_VEHICLES_OVERLAP].first) {
			sprintf(gString, "Vehicle overlap list %d,%d not empty\n", i % NUMSECTORS_X, i / NUMSECTORS_Y);
			pSector->m_lists[ENTITYLIST_VEHICLES_OVERLAP].Flush();
		}
		if(pSector->m_lists[ENTITYLIST_PEDS_OVERLAP].first) {
			sprintf(gString, "Ped overlap list %d,%d not empty\n", i % NUMSECTORS_X, i / NUMSECTORS_Y);
			pSector->m_lists[ENTITYLIST_PEDS_OVERLAP].Flush();
		}
		if(pSector->m_lists[ENTITYLIST_OBJECTS_OVERLAP].first) {
			sprintf(gString, "Object overlap list %d,%d not empty\n", i % NUMSECTORS_X, i / NUMSECTORS_Y);
			pSector->m_lists[ENTITYLIST_OBJECTS_OVERLAP].Flush();
		}
		if(pSector->m_lists[ENTITYLIST_DUMMIES_OVERLAP].first) {
			sprintf(gString, "Dummy overlap list %d,%d not empty\n", i % NUMSECTORS_X, i / NUMSECTORS_Y);
			pSector->m_lists[ENTITYLIST_DUMMIES_OVERLAP].Flush();
		}
	}
	ms_listMovingEntityPtrs.Flush();
#if GTA_VERSION <= GTA3_PS2_160
	CPools::Shutdown();
#endif
}

void
CWorld::ClearForRestart(void)
{
	if(CCutsceneMgr::HasLoaded()) CCutsceneMgr::DeleteCutsceneData();
	CProjectileInfo::RemoveAllProjectiles();
	CObject::DeleteAllTempObjects();
	CObject::DeleteAllMissionObjects();
	CPopulation::ConvertAllObjectsToDummyObjects();
	for(int i = 0; i < NUMSECTORS_X * NUMSECTORS_Y; i++) {
		CSector *pSector = GetSector(i % NUMSECTORS_X, i / NUMSECTORS_Y);
		for(CPtrNode *pNode = pSector->m_lists[ENTITYLIST_PEDS].first; pNode; pNode = pNode->next) {
			CEntity *pEntity = (CEntity *)pNode->item;
			Remove(pEntity);
			delete pEntity;
		}
		for(CPtrNode *pNode = GetBigBuildingList(LEVEL_GENERIC).first; pNode; pNode = pNode->next) {
			CVehicle *pVehicle = (CVehicle *)pNode->item;
			if(pVehicle && pVehicle->IsVehicle() && pVehicle->IsPlane()) {
				Remove(pVehicle);
				delete pVehicle;
			}
		}
		for(CPtrNode *pNode = pSector->m_lists[ENTITYLIST_VEHICLES].first; pNode; pNode = pNode->next) {
			CEntity *pEntity = (CEntity *)pNode->item;
			Remove(pEntity);
			delete pEntity;
		}
	}
	CPools::CheckPoolsEmpty();
}

void
CWorld::RepositionCertainDynamicObjects()
{
	int32 i = CPools::GetDummyPool()->GetSize();
	while(--i >= 0) {
		CDummy *dummy = CPools::GetDummyPool()->GetSlot(i);
		if(dummy) { RepositionOneObject(dummy); }
	}
}

void
CWorld::RepositionOneObject(CEntity *pEntity)
{
	int16 modelId = pEntity->GetModelIndex();
	if (IsStreetLight(modelId) || IsTreeModel(modelId) || modelId == MI_PARKINGMETER ||
	   modelId == MI_PHONEBOOTH1 || modelId == MI_WASTEBIN || modelId == MI_BIN || modelId == MI_POSTBOX1 ||
	   modelId == MI_NEWSSTAND || modelId == MI_TRAFFICCONE || modelId == MI_DUMP1 ||
	   modelId == MI_ROADWORKBARRIER1 || modelId == MI_BUSSIGN1 || modelId == MI_NOPARKINGSIGN1 ||
	   modelId == MI_PHONESIGN || modelId == MI_TAXISIGN || modelId == MI_FISHSTALL01 ||
	   modelId == MI_FISHSTALL02 || modelId == MI_FISHSTALL03 || modelId == MI_FISHSTALL04 ||
	   modelId == MI_BAGELSTAND2 || modelId == MI_FIRE_HYDRANT || modelId == MI_BOLLARDLIGHT ||
	   modelId == MI_PARKTABLE) {
		CVector &position = pEntity->GetMatrix().GetPosition();
		float fBoundingBoxMinZ = pEntity->GetColModel()->boundingBox.min.z;
		position.z = FindGroundZFor3DCoord(position.x, position.y,
		                                           position.z + OBJECT_REPOSITION_OFFSET_Z, nil) -
		             fBoundingBoxMinZ;
		pEntity->m_matrix.UpdateRW();
		pEntity->UpdateRwFrame();
	} else if(modelId == MI_BUOY) {
		float fWaterLevel = 0.0f;
		bool bFound = true;
		const CVector &position = pEntity->GetPosition();
		float fGroundZ = FindGroundZFor3DCoord(position.x, position.y,
		                                               position.z + OBJECT_REPOSITION_OFFSET_Z, &bFound);
		if(CWaterLevel::GetWaterLevelNoWaves(position.x, position.y, position.z + OBJECT_REPOSITION_OFFSET_Z,
		                                     &fWaterLevel)) {
			if(!bFound || fWaterLevel > fGroundZ) {
				CColModel *pColModel = pEntity->GetColModel();
				float fHeight = pColModel->boundingBox.max.z - pColModel->boundingBox.min.z;
				pEntity->GetMatrix().GetPosition().z = 0.2f * fHeight + fWaterLevel - 0.5f * fHeight;
			}
		}
	}
}

void
CWorld::SetCarsOnFire(float x, float y, float z, float radius, CEntity *reason)
{
	int poolSize = CPools::GetVehiclePool()->GetSize();
	for(int poolIndex = poolSize - 1; poolIndex >= 0; poolIndex--) {
		CVehicle *veh = CPools::GetVehiclePool()->GetSlot(poolIndex);
		if(veh && veh->GetStatus() != STATUS_WRECKED && !veh->m_pCarFire && !veh->bFireProof) {
			if(Abs(veh->GetPosition().z - z) < 5.0f && Abs(veh->GetPosition().x - x) < radius &&
			   Abs(veh->GetPosition().y - y) < radius)
				gFireManager.StartFire(veh, reason, 0.8f, true);
		}
	}
}

void
CWorld::SetPedsOnFire(float x, float y, float z, float radius, CEntity *reason)
{
	int32 poolSize = CPools::GetPedPool()->GetSize();
	for(int32 i = poolSize - 1; i >= 0; i--) {
		CPed *pPed = CPools::GetPedPool()->GetSlot(i);
		if(pPed && pPed->m_nPedState != PED_DEAD && !pPed->bInVehicle && !pPed->m_pFire && !pPed->bFireProof) {
			if(Abs(pPed->GetPosition().z - z) < 5.0f && Abs(pPed->GetPosition().x - x) < radius &&
			   Abs(pPed->GetPosition().y - y) < radius)
				gFireManager.StartFire(pPed, reason, 0.8f, true);
		}
	}
}

void
CWorld::RemoveStaticObjects()
{
	for(int i = 0; i < NUMSECTORS_X * NUMSECTORS_Y; i++) {
		CSector *pSector = GetSector(i % NUMSECTORS_X, i / NUMSECTORS_Y);
		for(CPtrNode *pNode = pSector->m_lists[ENTITYLIST_BUILDINGS].first; pNode; pNode = pNode->next) {
			CEntity *pEntity = (CEntity *)pNode->item;
			Remove(pEntity);
			delete pEntity;
		}
		for(CPtrNode *pNode = pSector->m_lists[ENTITYLIST_OBJECTS].first; pNode; pNode = pNode->next) {
			CEntity *pEntity = (CEntity *)pNode->item;
			Remove(pEntity);
			delete pEntity;
		}
		for(CPtrNode *pNode = pSector->m_lists[ENTITYLIST_DUMMIES].first; pNode; pNode = pNode->next) {
			CEntity *pEntity = (CEntity *)pNode->item;
			Remove(pEntity);
			delete pEntity;
		}
		pSector->m_lists[ENTITYLIST_BUILDINGS].Flush();
		pSector->m_lists[ENTITYLIST_BUILDINGS_OVERLAP].Flush();
		pSector->m_lists[ENTITYLIST_DUMMIES].Flush();
		pSector->m_lists[ENTITYLIST_DUMMIES_OVERLAP].Flush();
	}
}

void
CWorld::Process(void)
{
	if(!(CTimer::GetFrameCounter() & 63)) CReferences::PruneAllReferencesInWorld();

	if(bProcessCutsceneOnly) {
		for(int i = 0; i < NUMCUTSCENEOBJECTS; i++) {
			CCutsceneObject *csObj = CCutsceneMgr::GetCutsceneObject(i);
			if(csObj && csObj->m_entryInfoList.first) {
				if(csObj->m_rwObject && RwObjectGetType(csObj->m_rwObject) == rpCLUMP &&
				   RpAnimBlendClumpGetFirstAssociation(csObj->GetClump())) {
					RpAnimBlendClumpUpdateAnimations(csObj->GetClump(),
					                                 csObj->IsObject()
					                                              ? CTimer::GetTimeStepNonClippedInSeconds()
					                                              : CTimer::GetTimeStepInSeconds());
				}
				csObj->ProcessControl();
				csObj->ProcessCollision();
				csObj->GetMatrix().UpdateRW();
				csObj->UpdateRwFrame();
			}
		}
		CRecordDataForChase::ProcessControlCars();
		CRecordDataForChase::SaveOrRetrieveCarPositions();
	} else {
		for(CPtrNode *node = ms_listMovingEntityPtrs.first; node; node = node->next) {
			CEntity *movingEnt = (CEntity *)node->item;
#ifdef FIX_BUGS // from VC
			if(!movingEnt->bRemoveFromWorld && movingEnt->m_rwObject && RwObjectGetType(movingEnt->m_rwObject) == rpCLUMP &&
#else
			if(movingEnt->m_rwObject && RwObjectGetType(movingEnt->m_rwObject) == rpCLUMP &&
#endif
			   RpAnimBlendClumpGetFirstAssociation(movingEnt->GetClump())) {
				RpAnimBlendClumpUpdateAnimations(movingEnt->GetClump(),
				                                 movingEnt->IsObject()
				                                              ? CTimer::GetTimeStepNonClippedInSeconds()
				                                              : CTimer::GetTimeStepInSeconds());
			}
		}
		for(CPtrNode *node = ms_listMovingEntityPtrs.first; node; node = node->next) {
			CPhysical *movingEnt = (CPhysical *)node->item;
			if(movingEnt->bRemoveFromWorld) {
				RemoveEntityInsteadOfProcessingIt(movingEnt);
			} else {
				movingEnt->ProcessControl();
				if(movingEnt->GetIsStatic()) { movingEnt->RemoveFromMovingList(); }
			}
		}
		bForceProcessControl = true;
		for(CPtrNode *node = ms_listMovingEntityPtrs.first; node; node = node->next) {
			CPhysical *movingEnt = (CPhysical *)node->item;
			if(movingEnt->bWasPostponed) {
				if(movingEnt->bRemoveFromWorld) {
					RemoveEntityInsteadOfProcessingIt(movingEnt);
				} else {
					movingEnt->ProcessControl();
					if(movingEnt->GetIsStatic()) { movingEnt->RemoveFromMovingList(); }
				}
			}
		}
		bForceProcessControl = false;
		if(CReplay::IsPlayingBack()) {
			for(CPtrNode *node = ms_listMovingEntityPtrs.first; node; node = node->next) {
				CEntity *movingEnt = (CEntity *)node->item;
				movingEnt->bIsInSafePosition = true;
				movingEnt->GetMatrix().UpdateRW();
				movingEnt->UpdateRwFrame();
			}
		} else {
			bNoMoreCollisionTorque = false;
			for(CPtrNode *node = ms_listMovingEntityPtrs.first; node; node = node->next) {
				CEntity *movingEnt = (CEntity *)node->item;
				if(!movingEnt->bIsInSafePosition) {
					movingEnt->ProcessCollision();
					movingEnt->GetMatrix().UpdateRW();
					movingEnt->UpdateRwFrame();
				}
			}
			bNoMoreCollisionTorque = true;
			for(int i = 0; i < 4; i++) {
				for(CPtrNode *node = ms_listMovingEntityPtrs.first; node; node = node->next) {
					CEntity *movingEnt = (CEntity *)node->item;
					if(!movingEnt->bIsInSafePosition) {
						movingEnt->ProcessCollision();
						movingEnt->GetMatrix().UpdateRW();
						movingEnt->UpdateRwFrame();
					}
				}
			}
			for(CPtrNode *node = ms_listMovingEntityPtrs.first; node; node = node->next) {
				CEntity *movingEnt = (CEntity *)node->item;
				if(!movingEnt->bIsInSafePosition) {
					movingEnt->bIsStuck = true;
					movingEnt->ProcessCollision();
					movingEnt->GetMatrix().UpdateRW();
					movingEnt->UpdateRwFrame();
					if(!movingEnt->bIsInSafePosition) { movingEnt->bIsStuck = true; }
				}
			}
			bSecondShift = false;
			for(CPtrNode *node = ms_listMovingEntityPtrs.first; node; node = node->next) {
				CEntity *movingEnt = (CEntity *)node->item;
				if(!movingEnt->bIsInSafePosition) {
					movingEnt->ProcessShift();
					movingEnt->GetMatrix().UpdateRW();
					movingEnt->UpdateRwFrame();
					if(!movingEnt->bIsInSafePosition) { movingEnt->bIsStuck = true; }
				}
			}
			bSecondShift = true;
			for(CPtrNode *node = ms_listMovingEntityPtrs.first; node; node = node->next) {
				CPhysical *movingEnt = (CPhysical *)node->item;
				if(!movingEnt->bIsInSafePosition) {
					movingEnt->ProcessShift();
					movingEnt->GetMatrix().UpdateRW();
					movingEnt->UpdateRwFrame();
					if(!movingEnt->bIsInSafePosition) {
						movingEnt->bIsStuck = true;
						if(movingEnt->GetStatus() == STATUS_PLAYER) {
							printf("STUCK: Final Step: Player Entity %d Is Stuck\n", movingEnt->GetModelIndex());
							movingEnt->m_vecMoveSpeed *= 0.3f;
							movingEnt->ApplyMoveSpeed();
							movingEnt->ApplyTurnSpeed();
						}
					}
				}
			}
		}
		for(CPtrNode *node = ms_listMovingEntityPtrs.first; node; node = node->next) {
			CPed *movingPed = (CPed *)node->item;
			if(movingPed->IsPed()) {
				if(movingPed->bInVehicle && movingPed->m_nPedState != PED_EXIT_TRAIN ||
				   movingPed->EnteringCar()) {
					CVehicle *movingCar = movingPed->m_pMyVehicle;
					if(movingCar) {
						if(movingCar->IsTrain()) {
							movingPed->SetPedPositionInTrain();
						} else {
							switch(movingPed->m_nPedState) {
							case PED_ENTER_CAR:
							case PED_CARJACK: movingPed->EnterCar(); break;
							case PED_DRAG_FROM_CAR: movingPed->BeingDraggedFromCar(); break;
							case PED_EXIT_CAR: movingPed->ExitCar(); break;
							case PED_ARRESTED:
								if(movingPed->m_nLastPedState == PED_DRAG_FROM_CAR) {
									movingPed->BeingDraggedFromCar();
									break;
								}
								// fall through
							default: movingPed->SetPedPositionInCar(); break;
							}
						}
						movingPed->GetMatrix().UpdateRW();
						movingPed->UpdateRwFrame();
					} else {
						movingPed->bInVehicle = false;
						movingPed->QuitEnteringCar();
					}
				}
			}
		}
		CMessages::Process();
		Players[PlayerInFocus].Process();
		CRecordDataForChase::SaveOrRetrieveCarPositions();
		if((CTimer::GetFrameCounter() & 7) == 1) {
			RemoveFallenPeds();
		} else if((CTimer::GetFrameCounter() & 7) == 5) {
			RemoveFallenCars();
		}
	}
}

void
CWorld::TriggerExplosion(const CVector &position, float fRadius, float fPower, CEntity *pCreator,
                         bool bProcessVehicleBombTimer)
{
	CVector2D vecStartPos(position.x - fRadius, position.y - fRadius);
	CVector2D vecEndPos(position.x + fRadius, position.y + fRadius);
	const int32 nStartX = Max(GetSectorIndexX(vecStartPos.x), 0);
	const int32 nStartY = Max(GetSectorIndexY(vecStartPos.y), 0);
	const int32 nEndX = Min(GetSectorIndexX(vecEndPos.x), NUMSECTORS_X - 1);
	const int32 nEndY = Min(GetSectorIndexY(vecEndPos.y), NUMSECTORS_Y - 1);
	for(int32 y = nStartY; y <= nEndY; y++) {
		for(int32 x = nStartX; x <= nEndX; x++) {
			CSector *pSector = GetSector(x, y);
			TriggerExplosionSectorList(pSector->m_lists[ENTITYLIST_VEHICLES], position, fRadius,
			                                   fPower, pCreator, bProcessVehicleBombTimer);
			TriggerExplosionSectorList(pSector->m_lists[ENTITYLIST_PEDS], position, fRadius, fPower,
			                                   pCreator, bProcessVehicleBombTimer);
			TriggerExplosionSectorList(pSector->m_lists[ENTITYLIST_OBJECTS], position, fRadius,
			                                   fPower, pCreator, bProcessVehicleBombTimer);
		}
	}
}

void
CWorld::TriggerExplosionSectorList(CPtrList &list, const CVector &position, float fRadius, float fPower,
                                   CEntity *pCreator, bool bProcessVehicleBombTimer)
{
	for(CPtrNode *pNode = list.first; pNode; pNode = pNode->next) {
		CPhysical *pEntity = (CPhysical *)pNode->item;
		CVector vecDistance = pEntity->GetPosition() - position;
		float fMagnitude = vecDistance.Magnitude();
		if(fRadius > fMagnitude) {
			CWeapon::BlowUpExplosiveThings(pEntity);
			CPed *pPed = (CPed *)pEntity;
			CObject *pObject = (CObject *)pEntity;
			CVehicle *pVehicle = (CVehicle *)pEntity;
			if(!pEntity->bExplosionProof && (!pEntity->IsPed() || !pPed->bInVehicle)) {
				if(pEntity->GetIsStatic()) {
					if(pEntity->IsObject()) {
						if (fPower > pObject->m_fUprootLimit || IsFence(pObject->GetModelIndex())) {
							if (IsGlass(pObject->GetModelIndex())) {
								CGlass::WindowRespondsToExplosion(pObject, position);
							} else {
								pObject->SetIsStatic(false);
								pObject->AddToMovingList();
								int16 modelId = pEntity->GetModelIndex();
								if(modelId != MI_FIRE_HYDRANT ||
								   pObject->bHasBeenDamaged) {
									if(pEntity->IsObject() &&
									   modelId != MI_EXPLODINGBARREL &&
									   modelId != MI_PETROLPUMP)
										pObject->bHasBeenDamaged = true;
								} else {
									CVector pos = pEntity->GetPosition();
									pos.z -= 0.5f;
									CParticleObject::AddObject(POBJECT_FIRE_HYDRANT,
									                           pos, true);
									pObject->bHasBeenDamaged = true;
								}
							}
						}
						if(pEntity->GetIsStatic()) {
							float fDamageMultiplier =
							    (fRadius - fMagnitude) * 2.0f / fRadius;
							float fDamage = 300.0f * Min(fDamageMultiplier, 1.0f);
							pObject->ObjectDamage(fDamage);
						}
					} else {
						pEntity->SetIsStatic(false);
						pEntity->AddToMovingList();
					}
				}
				if(!pEntity->GetIsStatic()) {
					float fDamageMultiplier = Min((fRadius - fMagnitude) * 2.0f / fRadius, 1.0f);
					CVector vecForceDir =
					    vecDistance * (fPower * pEntity->m_fMass / 1400.0f * fDamageMultiplier /
					                   Max(fMagnitude, 0.01f));
					vecForceDir.z = Max(vecForceDir.z, 0.0f);
					if(pEntity == FindPlayerPed()) vecForceDir.z = Min(vecForceDir.z, 1.0f);
					pEntity->ApplyMoveForce(vecForceDir);
					if(!pEntity->bPedPhysics) {
						float fBoundRadius = pEntity->GetBoundRadius();
						float fDistanceZ = position.z - pEntity->GetPosition().z;
						float fPointZ = fBoundRadius;
						if(Max(fDistanceZ, -fBoundRadius) < fBoundRadius) {
							if(fDistanceZ <= -fBoundRadius)
								fPointZ = -fBoundRadius;
							else
								fPointZ = fDistanceZ;
						}
						pEntity->ApplyTurnForce(vecForceDir.x, vecForceDir.y, vecForceDir.z,
						                        0.0f, 0.0f, fPointZ);
					}
					switch(pEntity->GetType()) {
					case ENTITY_TYPE_VEHICLE:
						if(pEntity->GetStatus() == STATUS_SIMPLE) {
							pEntity->SetStatus(STATUS_PHYSICS);
							CCarCtrl::SwitchVehicleToRealPhysics(pVehicle);
						}
						pVehicle->InflictDamage(pCreator, WEAPONTYPE_EXPLOSION,
						                        1100.0f * fDamageMultiplier);
						if(bProcessVehicleBombTimer) {
							if(pVehicle->m_nBombTimer) pVehicle->m_nBombTimer /= 10;
						}
						break;
					case ENTITY_TYPE_PED: {
						int8 direction = pPed->GetLocalDirection(-vecForceDir);
						pPed->bIsStanding = false;
						pPed->ApplyMoveForce(0.0, 0.0, 2.0f);
						float fDamage = 250.0f * fDamageMultiplier;
						pPed->InflictDamage(pCreator, WEAPONTYPE_EXPLOSION, fDamage,
						                    PEDPIECE_TORSO, direction);
						if(pPed->m_nPedState != PED_DIE)
							pPed->SetFall(2000,
							              (AnimationId)(direction + ANIM_STD_HIGHIMPACT_FRONT), 0);
						if(pCreator && pCreator->IsPed()) {
							eEventType eventType = EVENT_SHOOT_PED;
							if(pPed->m_nPedType == PEDTYPE_COP) eventType = EVENT_SHOOT_COP;
							CEventList::RegisterEvent(eventType, EVENT_ENTITY_PED, pEntity,
							                          (CPed *)pCreator, 10000);
							pPed->RegisterThreatWithGangPeds(pCreator);
						}
						break;
					}
					case ENTITY_TYPE_OBJECT:
						pObject->ObjectDamage(300.0f * fDamageMultiplier);
						break;
					default: break;
					}
				}
			}
		}
	}
}

void
CWorld::UseDetonator(CEntity *pEntity)
{
	int32 i = CPools::GetVehiclePool()->GetSize();
	while(--i >= 0) {
		CAutomobile *pVehicle = (CAutomobile *)CPools::GetVehiclePool()->GetSlot(i);
		if(pVehicle && !pVehicle->m_vehType && pVehicle->m_bombType == CARBOMB_REMOTE &&
		   pVehicle->m_pBombRigger == pEntity) {
			pVehicle->m_bombType = CARBOMB_NONE;
			pVehicle->m_nBombTimer = 500;
			pVehicle->m_pBlowUpEntity = pVehicle->m_pBombRigger;
			if(pVehicle->m_pBlowUpEntity)
				pVehicle->m_pBlowUpEntity->RegisterReference(&pVehicle->m_pBlowUpEntity);
		}
	}
}
