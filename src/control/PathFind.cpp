#include "common.h"

#include "General.h"
#include "FileMgr.h"	// only needed for empty function
#include "Camera.h"
#include "Vehicle.h"
#include "World.h"
#include "Lines.h"	// for debug
#include "PathFind.h"

bool gbShowPedPaths;
bool gbShowCarPaths;
bool gbShowCarPathsLinks;

CPathFind ThePaths;

#define MAX_DIST INT16_MAX-1
#define MIN_PED_ROUTE_DISTANCE 23.8f


#define NUMTEMPNODES 5000
#define NUMDETACHED_CARS 1024
#define NUMDETACHED_PEDS 1214
#define NUMTEMPEXTERNALNODES 4600

CPathInfoForObject *InfoForTileCars;
CPathInfoForObject *InfoForTilePeds;

CPathInfoForObject *DetachedInfoForTileCars;
CPathInfoForObject *DetachedInfoForTilePeds;
CTempNodeExternal *TempExternalNodes;
int32 NumTempExternalNodes;
int32 NumDetachedPedNodeGroups;
int32 NumDetachedCarNodeGroups;

bool 
CPedPath::CalcPedRoute(int8 pathType, CVector position, CVector destination, CVector *pointPoses, int16 *pointsFound, int16 maxPoints)
{
	*pointsFound = 0;
	CVector vecDistance = destination - position;
	if (Abs(vecDistance.x) > MIN_PED_ROUTE_DISTANCE || Abs(vecDistance.y) > MIN_PED_ROUTE_DISTANCE || Abs(vecDistance.z) > MIN_PED_ROUTE_DISTANCE)
		return false;
	CVector vecPos = (position + destination) * 0.5f;
	CVector vecSectorStartPos (vecPos.x - 14.0f, vecPos.y - 14.0f, vecPos.z);
	CVector2D vecSectorEndPos (vecPos.x + 28.0f, vecPos.x + 28.0f);
	const int16 nodeStartX = (position.x - vecSectorStartPos.x) / 0.7f;
	const int16 nodeStartY = (position.y - vecSectorStartPos.y) / 0.7f;
	const int16 nodeEndX = (destination.x - vecSectorStartPos.x) / 0.7f;
	const int16 nodeEndY = (destination.y - vecSectorStartPos.y) / 0.7f;
	if (nodeStartX == nodeEndX && nodeStartY == nodeEndY)
		return false;
	CPedPathNode pathNodes[40][40]; 
	CPedPathNode pathNodesList[416];
	for (int32 x = 0; x < 40; x++) {
		for (int32 y = 0; y < 40; y++) {
			pathNodes[x][y].bBlockade = false;
			pathNodes[x][y].id = INT16_MAX;
			pathNodes[x][y].nodeIdX = x;
			pathNodes[x][y].nodeIdY = y;
		}
	}
	CWorld::AdvanceCurrentScanCode();
	if (pathType != ROUTE_NO_BLOCKADE) {
		const int32 nStartX = Max(CWorld::GetSectorIndexX(vecSectorStartPos.x), 0);
		const int32 nStartY = Max(CWorld::GetSectorIndexY(vecSectorStartPos.y), 0);
		const int32 nEndX = Min(CWorld::GetSectorIndexX(vecSectorEndPos.x), NUMSECTORS_X - 1);
		const int32 nEndY = Min(CWorld::GetSectorIndexY(vecSectorEndPos.y), NUMSECTORS_Y - 1);
		for (int32 y = nStartY; y <= nEndY; y++) {
			for (int32 x = nStartX; x <= nEndX; x++) {
				CSector *pSector = CWorld::GetSector(x, y);
				AddBlockadeSectorList(pSector->m_lists[ENTITYLIST_VEHICLES], pathNodes, &vecSectorStartPos);
				AddBlockadeSectorList(pSector->m_lists[ENTITYLIST_VEHICLES_OVERLAP], pathNodes, &vecSectorStartPos);
				AddBlockadeSectorList(pSector->m_lists[ENTITYLIST_OBJECTS], pathNodes, &vecSectorStartPos);
				AddBlockadeSectorList(pSector->m_lists[ENTITYLIST_OBJECTS_OVERLAP], pathNodes, &vecSectorStartPos);
			}
		}
	}
	for (int32 i = 0; i < 416; i++) {
		pathNodesList[i].prev = nil;
		pathNodesList[i].next = nil;
	}
	CPedPathNode *pStartPathNode = &pathNodes[nodeStartX][nodeStartY];
	CPedPathNode *pEndPathNode = &pathNodes[nodeEndX][nodeEndY];
	pEndPathNode->bBlockade = false;
	pEndPathNode->id = 0;
	pEndPathNode->prev = nil;
	pEndPathNode->next = pathNodesList;
	pathNodesList[0].prev = pEndPathNode;
	int32 pathNodeIndex = 0;
	CPedPathNode *pPreviousNode = nil;
	for (; pathNodeIndex < 414; pathNodeIndex++)
	{
		pPreviousNode = pathNodesList[pathNodeIndex].prev;
		while (pPreviousNode && pPreviousNode != pStartPathNode) {
			const uint8 nodeIdX = pPreviousNode->nodeIdX;
			const uint8 nodeIdY = pPreviousNode->nodeIdY;
			if (nodeIdX > 0) {
				AddNodeToPathList(&pathNodes[nodeIdX - 1][nodeIdY], pathNodeIndex + 5, pathNodesList);
				if (nodeIdY > 0)
					AddNodeToPathList(&pathNodes[nodeIdX - 1][nodeIdY - 1], pathNodeIndex + 7, pathNodesList);
				if (nodeIdY < 39)
					AddNodeToPathList(&pathNodes[nodeIdX - 1][nodeIdY + 1], pathNodeIndex + 7, pathNodesList);
			}
			if (nodeIdX < 39) {
				AddNodeToPathList(&pathNodes[nodeIdX + 1][nodeIdY], pathNodeIndex + 5, pathNodesList);
				if (nodeIdY > 0)
					AddNodeToPathList(&pathNodes[nodeIdX + 1][nodeIdY - 1], pathNodeIndex + 7, pathNodesList);
				if (nodeIdY < 39)
					AddNodeToPathList(&pathNodes[nodeIdX + 1][nodeIdY + 1], pathNodeIndex + 7, pathNodesList);
			}
			if (nodeIdY > 0)
				AddNodeToPathList(&pathNodes[nodeIdX][nodeIdY - 1], pathNodeIndex + 5, pathNodesList);
			if (nodeIdY < 39)
				AddNodeToPathList(&pathNodes[nodeIdX][nodeIdY + 1], pathNodeIndex + 5, pathNodesList);
			pPreviousNode = pPreviousNode->prev;
			if (!pPreviousNode)
				break;
		}

		if (pPreviousNode && pPreviousNode == pStartPathNode)
			break;
	}
	if (pathNodeIndex == 414)
		return false;
	CPedPathNode *pPathNode = pStartPathNode;
	for (*pointsFound = 0; pPathNode != pEndPathNode && *pointsFound < maxPoints; ++ *pointsFound) {
		const uint8 nodeIdX = pPathNode->nodeIdX;
		const uint8 nodeIdY = pPathNode->nodeIdY;
		if (nodeIdX > 0 && pathNodes[nodeIdX - 1][nodeIdY].id + 5 == pPathNode->id) 
			pPathNode = &pathNodes[nodeIdX - 1][nodeIdY];
		else if (nodeIdX > 39 && pathNodes[nodeIdX + 1][nodeIdY].id + 5 == pPathNode->id)
			pPathNode = &pathNodes[nodeIdX + 1][nodeIdY];
		else if (nodeIdY > 0 && pathNodes[nodeIdX][nodeIdY - 1].id + 5 == pPathNode->id) 
			pPathNode = &pathNodes[nodeIdX][nodeIdY - 1];
		else if (nodeIdY > 39 && pathNodes[nodeIdX][nodeIdY + 1].id + 5 == pPathNode->id) 
			pPathNode = &pathNodes[nodeIdX][nodeIdY + 1];
		else if (nodeIdX > 0 && nodeIdY > 0 && pathNodes[nodeIdX - 1][nodeIdY - 1].id + 7 == pPathNode->id)
			pPathNode = &pathNodes[nodeIdX - 1][nodeIdY - 1];
		else if (nodeIdX > 0 && nodeIdY < 39 && pathNodes[nodeIdX - 1][nodeIdY + 1].id + 7 == pPathNode->id)
			pPathNode = &pathNodes[nodeIdX - 1][nodeIdY + 1];
		else if (nodeIdX < 39 && nodeIdY > 0 && pathNodes[nodeIdX + 1][nodeIdY - 1].id + 7 == pPathNode->id)
			pPathNode = &pathNodes[nodeIdX + 1][nodeIdY - 1];
		else if (nodeIdX < 39 && nodeIdY < 39 && pathNodes[nodeIdX + 1][nodeIdY + 1].id + 7 == pPathNode->id)
			pPathNode = &pathNodes[nodeIdX + 1][nodeIdY + 1];
		pointPoses[*pointsFound] = vecSectorStartPos;
		pointPoses[*pointsFound].x += pPathNode->nodeIdX * 0.7f;
		pointPoses[*pointsFound].y += pPathNode->nodeIdY * 0.7f;
	}
	return true;
}


void 
CPedPath::AddNodeToPathList(CPedPathNode *pNodeToAdd, int16 id, CPedPathNode *pNodeList) 
{
	if (!pNodeToAdd->bBlockade && id < pNodeToAdd->id) {
		if (pNodeToAdd->id != INT16_MAX)
			RemoveNodeFromList(pNodeToAdd);
		AddNodeToList(pNodeToAdd, id, pNodeList);
	}
}

void 
CPedPath::RemoveNodeFromList(CPedPathNode *pNode)
{
	pNode->next->prev = pNode->prev;
	if (pNode->prev)
		pNode->prev->next = pNode->next;
}

void 
CPedPath::AddNodeToList(CPedPathNode *pNode, int16 index, CPedPathNode *pList)
{
	pNode->prev = pList[index].prev;
	pNode->next = &pList[index];
	if (pList[index].prev)
		pList[index].prev->next = pNode;
	pList[index].prev = pNode;
	pNode->id = index;
}

void
CPedPath::AddBlockadeSectorList(CPtrList& list, CPedPathNode(*pathNodes)[40], CVector *pPosition)
{
	CPtrNode* listNode = list.first;
	while (listNode) {
		CEntity* pEntity = (CEntity*)listNode->item;
		if (pEntity->m_scanCode != CWorld::GetCurrentScanCode() && pEntity->bUsesCollision) {
			pEntity->m_scanCode = CWorld::GetCurrentScanCode();
			AddBlockade(pEntity, pathNodes, pPosition);
		}
		listNode = listNode->next;
	}
}

void 
CPedPath::AddBlockade(CEntity *pEntity, CPedPathNode(*pathNodes)[40], CVector *pPosition)
{
	const CBox& boundingBox = pEntity->GetColModel()->boundingBox;
	const float fBoundMaxY = boundingBox.max.y + 0.3f;
	const float fBoundMinY = boundingBox.min.y - 0.3f;
	const float fBoundMaxX = boundingBox.max.x + 0.3f;
	const float fDistanceX = pPosition->x - pEntity->m_matrix.GetPosition().x;
	const float fDistanceY = pPosition->y - pEntity->m_matrix.GetPosition().y;
	const float fBoundRadius = pEntity->GetBoundRadius();
	CVector vecBoundCentre;
	pEntity->GetBoundCentre(vecBoundCentre);
	if (vecBoundCentre.x + fBoundRadius >= pPosition->x && 
		vecBoundCentre.y + fBoundRadius >= pPosition->y &&
		vecBoundCentre.x - fBoundRadius <= pPosition->x + 28.0f &&
		vecBoundCentre.y - fBoundRadius <= pPosition->y + 28.0f) {
		for (int16 x = 0; x < 40; x++) {
			const float pointX = x * 0.7f + fDistanceX;
			for (int16 y = 0; y < 40; y++) {
				if (!pathNodes[x][y].bBlockade) {
					const float pointY = y * 0.7f + fDistanceY;
					CVector2D point(pointX, pointY);
					if (fBoundMaxX > Abs(DotProduct2D(point, pEntity->m_matrix.GetRight()))) {
						float fDotProduct = DotProduct2D(point, pEntity->m_matrix.GetForward());
						if (fBoundMaxY > fDotProduct && fBoundMinY < fDotProduct)
							pathNodes[x][y].bBlockade = true;
					}
				}
			}
		}
	}
}

//--MIAMI: done
// Make sure all externals link TO an internal
void
CPathInfoForObject::SwapConnectionsToBeRightWayRound(void)
{
	int e, i;
	CPathInfoForObject *tile = this;

	for(e = 0; e < 12; e++)
	if(tile[e].type == NodeTypeExtern && tile[e].next < 0)
		for(i = 0; i < 12; i++)
		if(tile[i].type == NodeTypeIntern && tile[i].next == e){
			tile[e].next = i;
			tile[i].next = -1;
			bool tmp = !!tile[e].crossing;
			tile[e].crossing = tile[i].crossing;
			tile[i].crossing = tmp;
		}
}

//--MIAMI: done
void
CPathFind::Init(void)
{
	int i;

	m_numPathNodes = 0;
	m_numMapObjects = 0;
	m_numConnections = 0;
	m_numCarPathLinks = 0;
	unk = 0;
	NumTempExternalNodes = 0;

	for(i = 0; i < NUM_PATHNODES; i++)
		m_pathNodes[i].distance = MAX_DIST;
}

//--MIAMI: done
void
CPathFind::AllocatePathFindInfoMem(int16 numPathGroups)
{
	delete[] InfoForTileCars;
	InfoForTileCars = nil;
	delete[] InfoForTilePeds;
	InfoForTilePeds = nil;

	// NB: MIAMI doesn't use numPathGroups here but hardcodes PATHNODESIZE
	InfoForTileCars = new CPathInfoForObject[12*PATHNODESIZE];
	memset(InfoForTileCars, 0, 12*PATHNODESIZE*sizeof(CPathInfoForObject));
	InfoForTilePeds = new CPathInfoForObject[12*PATHNODESIZE];
	memset(InfoForTilePeds, 0, 12*PATHNODESIZE*sizeof(CPathInfoForObject));

	delete[] DetachedInfoForTileCars;
	DetachedInfoForTileCars = nil;
	delete[] DetachedInfoForTilePeds;
	DetachedInfoForTilePeds = nil;
	DetachedInfoForTileCars = new CPathInfoForObject[12*NUMDETACHED_CARS];
	memset(DetachedInfoForTileCars, 0, 12*NUMDETACHED_CARS*sizeof(CPathInfoForObject));
	DetachedInfoForTilePeds = new CPathInfoForObject[12*NUMDETACHED_PEDS];
	memset(DetachedInfoForTilePeds, 0, 12*NUMDETACHED_PEDS*sizeof(CPathInfoForObject));

	TempExternalNodes = new CTempNodeExternal[NUMTEMPEXTERNALNODES];
	memset(TempExternalNodes, 0, NUMTEMPEXTERNALNODES*sizeof(CTempNodeExternal));
	NumTempExternalNodes = 0;
	NumDetachedPedNodeGroups = 0;
	NumDetachedCarNodeGroups = 0;
}

//--MIAMI: done
void
CPathFind::RegisterMapObject(CTreadable *mapObject)
{
	m_mapObjects[m_numMapObjects++] = mapObject;
}

//--MIAMI: done
void
CPathFind::StoreNodeInfoPed(int16 id, int16 node, int8 type, int8 next, int16 x, int16 y, int16 z, float width, bool crossing, uint8 spawnRate)
{
	int i;

	i = id*12 + node;
	InfoForTilePeds[i].type = type;
	InfoForTilePeds[i].next = next;
	InfoForTilePeds[i].x = x/16.0f;
	InfoForTilePeds[i].y = y/16.0f;
	InfoForTilePeds[i].z = z/16.0f;
	InfoForTilePeds[i].width = 8.0f*Min(width, 15.0f);
	InfoForTilePeds[i].numLeftLanes = 0;
	InfoForTilePeds[i].numRightLanes = 0;
	InfoForTilePeds[i].crossing = crossing;
	InfoForTilePeds[i].speedLimit = 0;
	InfoForTilePeds[i].roadBlock = false;
	InfoForTilePeds[i].disabled = false;
	InfoForTilePeds[i].waterPath = false;
	InfoForTilePeds[i].onlySmallBoats = false;
	InfoForTilePeds[i].betweenLevels = false;
	InfoForTilePeds[i].spawnRate = Min(spawnRate, 15);

	if(node == 11)
		InfoForTilePeds[id*12].SwapConnectionsToBeRightWayRound();
}

//--MIAMI: done
void
CPathFind::StoreNodeInfoCar(int16 id, int16 node, int8 type, int8 next, int16 x, int16 y, int16 z, float width, int8 numLeft, int8 numRight,
	bool disabled, bool betweenLevels, uint8 speedLimit, bool roadBlock, bool waterPath, uint8 spawnRate)
{
	int i;

	i = id*12 + node;
	InfoForTileCars[i].type = type;
	InfoForTileCars[i].next = next;
	InfoForTileCars[i].x = x/16.0f;
	InfoForTileCars[i].y = y/16.0f;
	InfoForTileCars[i].z = z/16.0f;
	InfoForTilePeds[i].width = 8.0f*Min(width, 15.0f);
	InfoForTileCars[i].numLeftLanes = numLeft;
	InfoForTileCars[i].numRightLanes = numRight;
	InfoForTilePeds[i].crossing = false;
	InfoForTilePeds[i].speedLimit = 0;
	InfoForTilePeds[i].roadBlock = false;
	InfoForTilePeds[i].disabled = false;
	InfoForTilePeds[i].waterPath = false;
	InfoForTilePeds[i].onlySmallBoats = false;
	InfoForTilePeds[i].betweenLevels = false;
	InfoForTilePeds[i].spawnRate = Min(spawnRate, 15);

	if(node == 11)
		InfoForTileCars[id*12].SwapConnectionsToBeRightWayRound();
}

//--MIAMI: done
void
CPathFind::StoreDetachedNodeInfoPed(int32 node, int8 type, int32 next, float x, float y, float z, float width, bool crossing,
	bool disabled, bool betweenLevels, uint8 spawnRate)
{
	int i;

	if(NumDetachedPedNodeGroups >= NUMDETACHED_PEDS)
		return;

	i = NumDetachedPedNodeGroups*12 + node;
	DetachedInfoForTilePeds[i].type = type;
	DetachedInfoForTilePeds[i].next = next;
	DetachedInfoForTilePeds[i].x = x/16.0f;
	DetachedInfoForTilePeds[i].y = y/16.0f;
	DetachedInfoForTilePeds[i].z = z/16.0f;
	DetachedInfoForTilePeds[i].width = 8.0f*Min(width, 31.0f);
	DetachedInfoForTilePeds[i].numLeftLanes = 0;
	DetachedInfoForTilePeds[i].numRightLanes = 0;
	DetachedInfoForTilePeds[i].crossing = crossing;
	DetachedInfoForTilePeds[i].speedLimit = 0;
	DetachedInfoForTilePeds[i].roadBlock = false;
	DetachedInfoForTilePeds[i].disabled = disabled;
	DetachedInfoForTilePeds[i].waterPath = false;
	DetachedInfoForTilePeds[i].onlySmallBoats = false;
	DetachedInfoForTilePeds[i].betweenLevels = betweenLevels;
	DetachedInfoForTilePeds[i].spawnRate = Min(spawnRate, 15);

	if(node == 11){
		DetachedInfoForTilePeds[NumDetachedPedNodeGroups*12].SwapConnectionsToBeRightWayRound();
		NumDetachedPedNodeGroups++;
	}
}

//--MIAMI: done
void
CPathFind::StoreDetachedNodeInfoCar(int32 node, int8 type, int32 next, float x, float y, float z, float width, int8 numLeft, int8 numRight,
	bool disabled, bool betweenLevels, uint8 speedLimit, bool roadBlock, bool waterPath, uint8 spawnRate, bool onlySmallBoats)
{
	int i;

	if(NumDetachedCarNodeGroups >= NUMDETACHED_CARS)
		return;

	i = NumDetachedCarNodeGroups*12 + node;
	DetachedInfoForTileCars[i].type = type;
	DetachedInfoForTileCars[i].next = next;
	DetachedInfoForTileCars[i].x = x/16.0f;
	DetachedInfoForTileCars[i].y = y/16.0f;
	DetachedInfoForTileCars[i].z = z/16.0f;
	DetachedInfoForTileCars[i].width = 8.0f*Min(width, 15.0f);
	DetachedInfoForTileCars[i].numLeftLanes = numLeft;
	DetachedInfoForTileCars[i].numRightLanes = numRight;
	DetachedInfoForTileCars[i].crossing = false;
	DetachedInfoForTileCars[i].speedLimit = speedLimit;
	DetachedInfoForTileCars[i].roadBlock = roadBlock;
	DetachedInfoForTileCars[i].disabled = disabled;
	DetachedInfoForTileCars[i].waterPath = waterPath;
	DetachedInfoForTileCars[i].onlySmallBoats = onlySmallBoats;
	DetachedInfoForTileCars[i].betweenLevels = betweenLevels;
	DetachedInfoForTileCars[i].spawnRate = Min(spawnRate, 15);

	if(node == 11){
		DetachedInfoForTileCars[NumDetachedCarNodeGroups*12].SwapConnectionsToBeRightWayRound();
		NumDetachedCarNodeGroups++;
	}
}

//--MIAMI: done
void
CPathFind::CalcNodeCoors(float x, float y, float z, int id, CVector *out)
{
	CVector pos;
	pos.x = x;
	pos.y = y;
	pos.z = z;
	*out = m_mapObjects[id]->GetMatrix() * pos;
}

//--MIAMI: done
bool
CPathFind::LoadPathFindData(void)
{
	CFileMgr::SetDir("");
	return false;
}

//--MIAMI: done
void
CPathFind::PreparePathData(void)
{
	int i, j;
	int numExtern, numIntern;
	CTempNode *tempNodes;

	printf("PreparePathData\n");
	if(!CPathFind::LoadPathFindData() &&	// empty
	   InfoForTileCars && InfoForTilePeds &&
	   DetachedInfoForTileCars && DetachedInfoForTilePeds && TempExternalNodes){
		tempNodes = new CTempNode[NUMTEMPNODES];

		m_numConnections = 0;

		for(i = 0; i < PATHNODESIZE; i++){
			numExtern = 0;
			numIntern = 0;
			for(j = 0; j < 12; j++){
				if(InfoForTileCars[i*12 + j].type == NodeTypeExtern)
					numExtern++;
				if(InfoForTileCars[i*12 + j].type == NodeTypeIntern)
					numIntern++;
			}
			if(numIntern > 1 && numExtern != 2)
				printf("ILLEGAL BLOCK. MORE THAN 1 INTERNALS AND NOT 2 EXTERNALS (Modelindex:%d)\n", i);
		}

		int numExternDetached, numInternDetached;
		for(i = 0; i < NUMDETACHED_CARS; i++){
			numExternDetached = 0;
			numInternDetached = 0;
			for(j = 0; j < 12; j++){
				if(DetachedInfoForTileCars[i*12 + j].type == NodeTypeExtern)
					numExternDetached++;
				if(DetachedInfoForTilePeds[i*12 + j].type == NodeTypeIntern)
					numInternDetached++;
			}
			// no diagnostic here
		}

		for(i = 0; i < PATHNODESIZE; i++)
			for(j = 0; j < 12; j++)
				if(InfoForTileCars[i*12 + j].type == NodeTypeExtern){
					// MIAMI has MI:%d here but no argument for it
					if(InfoForTileCars[i*12 + j].numLeftLanes < 0)
						printf("ILLEGAL BLOCK. NEGATIVE NUMBER OF LANES (Obj:%d)\n", i);
					if(InfoForTileCars[i*12 + j].numRightLanes < 0)
						printf("ILLEGAL BLOCK. NEGATIVE NUMBER OF LANES (Obj:%d)\n", i);
					if(InfoForTileCars[i*12 + j].numLeftLanes + InfoForTileCars[i*12 + j].numRightLanes <= 0)
						printf("ILLEGAL BLOCK. NO LANES IN NODE (Obj:%d)\n", i);
				}
		for(i = 0; i < NUMDETACHED_CARS; i++)
			for(j = 0; j < 12; j++)
				if(DetachedInfoForTileCars[i*12 + j].type == NodeTypeExtern){
					// MI:%d here but no argument for it
					if(DetachedInfoForTileCars[i*12 + j].numLeftLanes < 0)
						printf("ILLEGAL BLOCK. NEGATIVE NUMBER OF LANES (Obj:%d)\n", i);
					if(DetachedInfoForTileCars[i*12 + j].numRightLanes < 0)
						printf("ILLEGAL BLOCK. NEGATIVE NUMBER OF LANES (Obj:%d)\n", i);
					if(DetachedInfoForTileCars[i*12 + j].numLeftLanes + DetachedInfoForTileCars[i*12 + j].numRightLanes <= 0)
						printf("ILLEGAL BLOCK. NO LANES IN NODE (Obj:%d)\n", i);
				}

		m_numPathNodes = 0;
		PreparePathDataForType(PATH_CAR, tempNodes, InfoForTileCars, 1.0f, DetachedInfoForTileCars, NumDetachedCarNodeGroups);
		m_numCarPathNodes = m_numPathNodes;
		PreparePathDataForType(PATH_PED, tempNodes, InfoForTilePeds, 1.0f, DetachedInfoForTilePeds, NumDetachedPedNodeGroups);
		m_numPedPathNodes = m_numPathNodes - m_numCarPathNodes;

		delete[] tempNodes;

		CountFloodFillGroups(PATH_CAR);
		CountFloodFillGroups(PATH_PED);

		delete[] InfoForTileCars;
		InfoForTileCars = nil;
		delete[] InfoForTilePeds;
		InfoForTilePeds = nil;

		delete[] DetachedInfoForTileCars;
		DetachedInfoForTileCars = nil;
		delete[] DetachedInfoForTilePeds;
		DetachedInfoForTilePeds = nil;
		delete[] TempExternalNodes;
		TempExternalNodes = nil;
	}
	printf("Done with PreparePathData\n");
}

//--MIAMI: done
/* String together connected nodes in a list by a flood fill algorithm */
void
CPathFind::CountFloodFillGroups(uint8 type)
{
	int start, end;
	int i, l;
	uint16 n;
	CPathNode *node, *prev;

	switch(type){
	case PATH_CAR:
		start = 0;
		end = m_numCarPathNodes;
		break;
	case PATH_PED:
		start = m_numCarPathNodes;
		end = start + m_numPedPathNodes;
		break;
	}

	for(i = start; i < end; i++)
		m_pathNodes[i].group = 0;

	n = 0;
	for(;;){
		n++;
		if(n > 1500){
			for(i = start; m_pathNodes[i].group && i < end; i++);
			printf("NumNodes:%d Accounted for:%d\n", end - start, i - start);
		}

		// Look for unvisited node
		for(i = start; m_pathNodes[i].group && i < end; i++);
		if(i == end)
			break;

		node = &m_pathNodes[i];
		node->SetNext(nil);
		node->group = n;

		if(node->numLinks == 0){
			if(type == PATH_CAR)
				printf("Single car node: %f %f %f\n",
					node->GetX(), node->GetY(), node->GetZ());
			else
				printf("Single ped node: %f %f %f\n",
					node->GetX(), node->GetY(), node->GetZ());
		}

		while(node){
			prev = node;
			node = node->GetNext();
			for(i = 0; i < prev->numLinks; i++){
				l = ConnectedNode(prev->firstLink + i);
				if(m_pathNodes[l].group == 0){
					m_pathNodes[l].group = n;
					if(m_pathNodes[l].group == 0)
						m_pathNodes[l].group = INT8_MIN;
					m_pathNodes[l].SetNext(node);
					node = &m_pathNodes[l];
				}
			}
		}
	}

	m_numGroups[type] = n-1;
	printf("GraphType:%d. FloodFill groups:%d\n", type, n);
}

int32 TempListLength;

//--MIAMI: done
void
CPathFind::PreparePathDataForType(uint8 type, CTempNode *tempnodes, CPathInfoForObject *objectpathinfo,
	float maxdist, CPathInfoForObject *detachednodes, int numDetached)
{
	static CVector CoorsXFormed;
	int i, j, k;
	int l1, l2;
	int start;
	float posx, posy;
	float dx, dy, mag;
	float nearestDist;
	int nearestId;
	int oldNumPathNodes, oldNumLinks;
	float dist;
	int iseg, jseg;
	int done, cont;
	int tileStart;

	oldNumPathNodes = m_numPathNodes;
	oldNumLinks = m_numConnections;

#define OBJECTINDEX(n) (mapObjIndices[(n)])
	int16 *mapObjIndices = new int16[NUM_PATHNODES];
	NumTempExternalNodes = 0;

	// Calculate internal nodes, store them and connect them to defining object
	for(i = 0; i < m_numMapObjects; i++){
		tileStart = m_numPathNodes;
		start = 12 * m_mapObjects[i]->GetModelIndex();
		for(j = 0; j < 12; j++){
			if(objectpathinfo[start + j].type == NodeTypeIntern){
				CalcNodeCoors(
					objectpathinfo[start + j].x,
					objectpathinfo[start + j].y,
					objectpathinfo[start + j].z,
					i,
					&CoorsXFormed);
				m_pathNodes[m_numPathNodes].SetPosition(CoorsXFormed);
				OBJECTINDEX(m_numPathNodes) = i;
				m_pathNodes[m_numPathNodes].width = objectpathinfo[start + j].width;
				m_pathNodes[m_numPathNodes].speedLimit = objectpathinfo[start + j].speedLimit;
				m_pathNodes[m_numPathNodes].spawnRate = objectpathinfo[start + j].spawnRate;
				m_pathNodes[m_numPathNodes].bUseInRoadBlock = objectpathinfo[start + j].roadBlock;
				m_pathNodes[m_numPathNodes].bDisabled = objectpathinfo[start + j].disabled;
				m_pathNodes[m_numPathNodes].bWaterPath = objectpathinfo[start + j].waterPath;
				m_pathNodes[m_numPathNodes].bOnlySmallBoats = objectpathinfo[start + j].onlySmallBoats;
				m_pathNodes[m_numPathNodes].bBetweenLevels = objectpathinfo[start + j].betweenLevels;
				m_numPathNodes++;
			}
			else if(objectpathinfo[start + j].type == NodeTypeExtern){
				CalcNodeCoors(
					objectpathinfo[start + j].x,
					objectpathinfo[start + j].y,
					objectpathinfo[start + j].z,
					i,
					&CoorsXFormed);
				TempExternalNodes[NumTempExternalNodes].pos = CoorsXFormed;
				assert(objectpathinfo[start + j].next >= 0);
				TempExternalNodes[NumTempExternalNodes].next = tileStart + objectpathinfo[start + j].next;
				TempExternalNodes[NumTempExternalNodes].numLeftLanes = objectpathinfo[start + j].numLeftLanes;
				TempExternalNodes[NumTempExternalNodes].numRightLanes = objectpathinfo[start + j].numRightLanes;
				TempExternalNodes[NumTempExternalNodes].width = objectpathinfo[start + j].width;
				TempExternalNodes[NumTempExternalNodes].isCross = !!objectpathinfo[start + j].crossing;
				NumTempExternalNodes++;
			}
		}
	}

	// Same thing for detached nodes
	for(i = 0; i < numDetached; i++){
		tileStart = m_numPathNodes;
		start = 12*i;
		for(j = 0; j < 12; j++){
			if(detachednodes[start + j].type == NodeTypeIntern){
				CVector pos;
				pos.x = detachednodes[start + j].x;
				pos.y = detachednodes[start + j].y;
				pos.z = detachednodes[start + j].z;
				m_pathNodes[m_numPathNodes].SetPosition(pos);
				mapObjIndices[m_numPathNodes] = -(i+1);
				m_pathNodes[m_numPathNodes].width = detachednodes[start + j].width;
				m_pathNodes[m_numPathNodes].speedLimit = detachednodes[start + j].speedLimit;
				m_pathNodes[m_numPathNodes].spawnRate = detachednodes[start + j].spawnRate;
				m_pathNodes[m_numPathNodes].bUseInRoadBlock = detachednodes[start + j].roadBlock;
				m_pathNodes[m_numPathNodes].bDisabled = detachednodes[start + j].disabled;
				m_pathNodes[m_numPathNodes].bWaterPath = detachednodes[start + j].waterPath;
				m_pathNodes[m_numPathNodes].bOnlySmallBoats = detachednodes[start + j].onlySmallBoats;
				m_pathNodes[m_numPathNodes].bBetweenLevels = detachednodes[start + j].betweenLevels;
				m_numPathNodes++;
			}else if(detachednodes[start + j].type == NodeTypeExtern){
				TempExternalNodes[NumTempExternalNodes].pos.x = detachednodes[start + j].x;
				TempExternalNodes[NumTempExternalNodes].pos.y = detachednodes[start + j].y;
				TempExternalNodes[NumTempExternalNodes].pos.z = detachednodes[start + j].z;
				assert(detachednodes[start + j].next >= 0);
				TempExternalNodes[NumTempExternalNodes].next = tileStart + detachednodes[start + j].next;
				TempExternalNodes[NumTempExternalNodes].numLeftLanes = detachednodes[start + j].numLeftLanes;
				TempExternalNodes[NumTempExternalNodes].numRightLanes = detachednodes[start + j].numRightLanes;
				TempExternalNodes[NumTempExternalNodes].width = detachednodes[start + j].width;
				TempExternalNodes[NumTempExternalNodes].isCross = !!detachednodes[start + j].crossing;
				NumTempExternalNodes++;
			}
		}
	}

	// Insert external nodes into TempList
	TempListLength = 0;
	for(i = 0; i < NumTempExternalNodes; i++){
		// find closest unconnected node
		nearestId = -1;
		nearestDist = maxdist;
		for(k = 0; k < TempListLength; k++){
			if(tempnodes[k].linkState != 1)
				continue;
			dx = tempnodes[k].pos.x - TempExternalNodes[i].pos.x;
			if(Abs(dx) < nearestDist){
				dy = tempnodes[k].pos.y - TempExternalNodes[i].pos.y;
				if(Abs(dy) < nearestDist){
					nearestDist = Max(Abs(dx), Abs(dy));
					nearestId = k;
				}
			}
		}

		if(nearestId < 0){
			// None found, add this one to temp list
			tempnodes[TempListLength].pos = TempExternalNodes[i].pos;
			// link to connecting internal node
			tempnodes[TempListLength].link1 = TempExternalNodes[i].next;
			if(type == PATH_CAR){
				tempnodes[TempListLength].numLeftLanes = TempExternalNodes[i].numLeftLanes;
				tempnodes[TempListLength].numRightLanes = TempExternalNodes[i].numRightLanes;
			}
			tempnodes[TempListLength].width = TempExternalNodes[i].width;
			tempnodes[TempListLength].isCross = TempExternalNodes[i].isCross;
			tempnodes[TempListLength++].linkState = 1;
		}else{
			// Found nearest, connect it to our neighbour
			tempnodes[nearestId].link2 = TempExternalNodes[i].next;
			tempnodes[nearestId].linkState = 2;

			// collapse this node with nearest we found
			dx = m_pathNodes[tempnodes[nearestId].link1].GetX() - m_pathNodes[tempnodes[nearestId].link2].GetX();
			dy = m_pathNodes[tempnodes[nearestId].link1].GetY() - m_pathNodes[tempnodes[nearestId].link2].GetY();
			tempnodes[nearestId].pos = (tempnodes[nearestId].pos + TempExternalNodes[i].pos)*0.5f;
			mag = Sqrt(dx*dx + dy*dy);
			tempnodes[nearestId].dirX = dx/mag * 100;
			tempnodes[nearestId].dirY = dy/mag * 100;
			tempnodes[nearestId].width = Max(tempnodes[nearestId].width, TempExternalNodes[i].width);
			if(TempExternalNodes[i].isCross)
				tempnodes[nearestId].isCross = true;	// TODO: is this guaranteed to be false otherwise?
			// do something when number of lanes doesn't agree
			if(type == PATH_CAR)
				if(tempnodes[nearestId].numLeftLanes != 0 && tempnodes[nearestId].numRightLanes != 0 &&
				   (TempExternalNodes[i].numLeftLanes == 0 || TempExternalNodes[i].numRightLanes == 0)){
					// why switch left and right here?
					tempnodes[nearestId].numLeftLanes = TempExternalNodes[i].numRightLanes;
					tempnodes[nearestId].numRightLanes = TempExternalNodes[i].numLeftLanes;
				}
		}
	}

	// Loop through previously added internal nodes and link them
	for(i = oldNumPathNodes; i < m_numPathNodes; i++){
		// Init link
		m_pathNodes[i].numLinks = 0;
		m_pathNodes[i].firstLink = m_numConnections;

		// See if node connects to external nodes
		for(j = 0; j < TempListLength; j++){
			if(tempnodes[j].linkState != 2)
				continue;

			// Add link to other side of the external
			// NB this clears the flags in MIAMI
			if(tempnodes[j].link1 == i)
				m_connections[m_numConnections] = tempnodes[j].link2;
			else if(tempnodes[j].link2 == i)
				m_connections[m_numConnections] = tempnodes[j].link1;
			else
				continue;

			dist = (m_pathNodes[i].GetPosition() - m_pathNodes[ConnectedNode(m_numConnections)].GetPosition()).Magnitude();
			m_distances[m_numConnections] = Min(dist, 255);
			if(tempnodes[j].isCross)
				m_connections[j] |= 0x8000;	// crosses road flag

			if(type == PATH_CAR){
				// IMPROVE: use a goto here
				// Find existing car path link
				for(k = 0; k < m_numCarPathLinks; k++){
					if(m_carPathLinks[k].dirX == tempnodes[j].dirX &&
					   m_carPathLinks[k].dirY == tempnodes[j].dirY &&
					   m_carPathLinks[k].x == (int)(tempnodes[j].pos.x*8.0f) &&
					   m_carPathLinks[k].y == (int)(tempnodes[j].pos.y*8.0f)){
						m_carPathConnections[m_numConnections] = k;
						k = m_numCarPathLinks;
					}
				}
				// k is m_numCarPathLinks+1 if we found one
				if(k == m_numCarPathLinks){
					m_carPathLinks[m_numCarPathLinks].dirX = tempnodes[j].dirX;
					m_carPathLinks[m_numCarPathLinks].dirY = tempnodes[j].dirY;
					m_carPathLinks[m_numCarPathLinks].x = tempnodes[j].pos.x*8.0f;
					m_carPathLinks[m_numCarPathLinks].y = tempnodes[j].pos.y*8.0f;
					m_carPathLinks[m_numCarPathLinks].flag1 = false;
					m_carPathLinks[m_numCarPathLinks].width = tempnodes[j].width;
					m_carPathLinks[m_numCarPathLinks].pathNodeIndex = i;
					m_carPathLinks[m_numCarPathLinks].numLeftLanes = tempnodes[j].numLeftLanes;
					m_carPathLinks[m_numCarPathLinks].numRightLanes = tempnodes[j].numRightLanes;
					m_carPathLinks[m_numCarPathLinks].trafficLightType = 0;
					assert(m_numCarPathLinks <= NUM_CARPATHLINKS);
					m_carPathConnections[m_numConnections] = m_numCarPathLinks++;
				}
			}

			m_pathNodes[i].numLinks++;
			m_numConnections++;
		}

		CPathInfoForObject *tile;
		if(mapObjIndices[i] < 0){
			if(type == PATH_CAR)
				tile = &DetachedInfoForTileCars[12 * (-1 - mapObjIndices[i])];
			else
				tile = &DetachedInfoForTilePeds[12 * (-1 - mapObjIndices[i])];
		}else{
			if(type == PATH_CAR)
				tile = &InfoForTileCars[12 * m_mapObjects[mapObjIndices[i]]->GetModelIndex()];
			else
				tile = &InfoForTilePeds[12 * m_mapObjects[mapObjIndices[i]]->GetModelIndex()];
		}

		// Find i inside path segment
		iseg = 0;
		for(j = Max(oldNumPathNodes, i-12); j < i; j++)
			if(OBJECTINDEX(j) == OBJECTINDEX(i))
				iseg++;

		// Add links to other internal nodes
		for(j = Max(oldNumPathNodes, i-12); j < Min(m_numPathNodes, i+12); j++){
			if(OBJECTINDEX(i) != OBJECTINDEX(j) || i == j)
				continue;
			// N.B.: in every path segment, the externals have to be at the end
			jseg = j-i + iseg;

			if(tile[iseg].next == jseg ||
			   tile[jseg].next == iseg){
				// Found a link between i and jConnectionSetCrossesRoad
				// NB this clears the flags in MIAMI
				m_connections[m_numConnections] = j;
				dist = (m_pathNodes[i].GetPosition() - m_pathNodes[j].GetPosition()).Magnitude();
				m_distances[m_numConnections] = Min(dist, 255);

				if(type == PATH_CAR){
					posx = (m_pathNodes[i].GetX() + m_pathNodes[j].GetX())*0.5f;
					posy = (m_pathNodes[i].GetY() + m_pathNodes[j].GetY())*0.5f;
					dx = m_pathNodes[j].GetX() - m_pathNodes[i].GetX();
					dy = m_pathNodes[j].GetY() - m_pathNodes[i].GetY();
					mag = Sqrt(dx*dx + dy*dy);
					dx /= mag;
					dy /= mag;
					int width = Max(m_pathNodes[i].width, m_pathNodes[j].width);
					if(i < j){
						dx = -dx;
						dy = -dy;
					}
					// IMPROVE: use a goto here
					// Find existing car path link
					for(k = 0; k < m_numCarPathLinks; k++){
						if(m_carPathLinks[k].dirX == (int)(dx*100.0f) &&
						   m_carPathLinks[k].dirY == (int)(dy*100.0f) &&
						   m_carPathLinks[k].x == (int)(posx*8.0f) &&
						   m_carPathLinks[k].y == (int)(posy*8.0f)){
							m_carPathConnections[m_numConnections] = k;
							k = m_numCarPathLinks;
						}
					}
					// k is m_numCarPathLinks+1 if we found one
					if(k == m_numCarPathLinks){
						m_carPathLinks[m_numCarPathLinks].dirX = dx*100.0f;
						m_carPathLinks[m_numCarPathLinks].dirY = dy*100.0f;
						m_carPathLinks[m_numCarPathLinks].x = posx*8.0f;
						m_carPathLinks[m_numCarPathLinks].y = posy*8.0f;
						m_carPathLinks[m_numCarPathLinks].flag1 = false;
						m_carPathLinks[m_numCarPathLinks].width = width;
						m_carPathLinks[m_numCarPathLinks].pathNodeIndex = i;
						m_carPathLinks[m_numCarPathLinks].numLeftLanes = -1;
						m_carPathLinks[m_numCarPathLinks].numRightLanes = -1;
						m_carPathLinks[m_numCarPathLinks].trafficLightType = 0;
						assert(m_numCarPathLinks <= NUM_CARPATHLINKS);
						m_carPathConnections[m_numConnections] = m_numCarPathLinks++;
					}
				}else{
					// Crosses road
					if(tile[iseg].next == jseg && tile[iseg].crossing ||
					   tile[jseg].next == iseg && tile[jseg].crossing)
						m_connections[m_numConnections] |= 0x8000;	// crosses road flag
				}

				m_pathNodes[i].numLinks++;
				m_numConnections++;
			}
		}
	}

	if(type == PATH_CAR){
		done = 0;
		// Set number of lanes for all nodes somehow
		// very strange code
		for(k = 0; !done && k < 12; k++){
			done = 1;
			for(i = 0; i < m_numPathNodes; i++){
				if(m_pathNodes[i].numLinks != 2)
					continue;
				l1 = m_carPathConnections[m_pathNodes[i].firstLink];
				l2 = m_carPathConnections[m_pathNodes[i].firstLink+1];

				int8 l1Left = m_carPathLinks[l1].numLeftLanes;
				int8 l1Right = m_carPathLinks[l1].numRightLanes;
				int8 l2Left = m_carPathLinks[l2].numLeftLanes;
				int8 l2Right = m_carPathLinks[l2].numRightLanes;
				int8 *l1Leftp, *l1Rightp;
				int8 *l2Leftp, *l2Rightp;
				if(m_carPathLinks[l1].pathNodeIndex == i){
					l1Leftp = &l1Left;
					l1Rightp = &l1Right;
				}else{
					l1Leftp = &l1Right;
					l1Rightp = &l1Left;
				}
				if(m_carPathLinks[l2].pathNodeIndex == i){
					l2Leftp = &l2Left;
					l2Rightp = &l2Right;
				}else{
					l2Leftp = &l2Right;
					l2Rightp = &l2Left;
				}
				if(*l1Leftp == -1 && *l2Rightp != -1){
					*l1Leftp = *l2Rightp;
					done = 0;
				}
				if(*l1Rightp == -1 && *l2Leftp != -1){
					*l1Rightp = *l2Leftp;
					done = 0;
				}
				if(*l2Leftp == -1 && *l1Rightp != -1){
					*l2Leftp = *l1Rightp;
					done = 0;
				}
				if(*l2Rightp == -1 && *l1Leftp != -1){
					*l2Rightp = *l1Leftp;
					done = 0;
				}
				if(*l1Leftp == -1 && *l2Rightp == -1)
					done = 0;
				if(*l2Leftp == -1 && *l1Rightp == -1)
					done = 0;
				m_carPathLinks[l1].numLeftLanes = l1Left;
				m_carPathLinks[l1].numRightLanes = l1Right;
				m_carPathLinks[l2].numLeftLanes = l2Left;
				m_carPathLinks[l2].numRightLanes = l2Right;
			}
		}

		// Fall back to default values for number of lanes
		for(i = 0; i < m_numPathNodes; i++)
			for(j = 0; j < m_pathNodes[i].numLinks; j++){
				k = m_carPathConnections[m_pathNodes[i].firstLink + j];
				if(m_carPathLinks[k].numLeftLanes == -1)
					m_carPathLinks[k].numLeftLanes = 0;
				if(m_carPathLinks[k].numRightLanes == -1)
					m_carPathLinks[k].numRightLanes = 0;
			}
	}

	// Set flags for car nodes
	if(type == PATH_CAR){
		do{
			cont = 0;
			for(i = 0; i < m_numPathNodes; i++){
				// See if node is a dead end, if so, we're not done yet
				if(!m_pathNodes[i].bDeadEnd){
					k = 0;
					for(j = 0; j < m_pathNodes[i].numLinks; j++)
						if(!m_pathNodes[ConnectedNode(m_pathNodes[i].firstLink + j)].bDeadEnd)
							k++;
					if(k < 2){
						m_pathNodes[i].bDeadEnd = true;
						cont = 1;
					}
				}
			}
		}while(cont);
	}

	// Remove isolated ped nodes
	if(type == PATH_PED)
		for(i = oldNumPathNodes; i < m_numPathNodes; i++){
			if(m_pathNodes[i].numLinks != 0)
				continue;

			// Remove node
			for(j = i; j < m_numPathNodes-1; j++)
				m_pathNodes[j] = m_pathNodes[j+1];

			// Fix links
			for(j = oldNumLinks; j < m_numConnections; j++){
				int node = ConnectedNode(j);
				if(node >= i)
					m_connections[j] = node-1;
			}

			i--;
			m_numPathNodes--;
		}

	delete[] mapObjIndices;
}

//--MIAMI: done
float
CPathFind::CalcRoadDensity(float x, float y)
{
	int i, j;
	float density = 0.0f;

	for(i = 0; i < m_numCarPathNodes; i++){
		if(Abs(m_pathNodes[i].GetX() - x) < 80.0f &&
		   Abs(m_pathNodes[i].GetY() - y) < 80.0f &&
		   m_pathNodes[i].numLinks > 0){
			for(j = 0; j < m_pathNodes[i].numLinks; j++){
				int next = ConnectedNode(m_pathNodes[i].firstLink + j);
				float dist = (m_pathNodes[i].GetPosition() - m_pathNodes[next].GetPosition()).Magnitude2D();
				next = m_carPathConnections[m_pathNodes[i].firstLink + j];
				density += m_carPathLinks[next].numLeftLanes * dist;
				density += m_carPathLinks[next].numRightLanes * dist;
			}
		}
	}
	return density/2500.0f;
}

//--MIAMI: done
bool
CPathFind::TestForPedTrafficLight(CPathNode *n1, CPathNode *n2)
{
	int i;
	for(i = 0; i < n1->numLinks; i++)
		if(&m_pathNodes[ConnectedNode(n1->firstLink + i)] == n2)
			return ConnectionHasTrafficLight(n1->firstLink + i);
	return false;
}

//--MIAMI: done
bool
CPathFind::TestCrossesRoad(CPathNode *n1, CPathNode *n2)
{
	int i;
	for(i = 0; i < n1->numLinks; i++)
		if(&m_pathNodes[ConnectedNode(n1->firstLink + i)] == n2)
			return ConnectionCrossesRoad(n1->firstLink + i);
	return false;
}

//--MIAMI: done
void
CPathFind::AddNodeToList(CPathNode *node, int32 listId)
{
	int i = listId & 0x1FF;
	node->SetNext(m_searchNodes[i].GetNext());
	node->SetPrev(&m_searchNodes[i]);
	if(m_searchNodes[i].GetNext())
		m_searchNodes[i].GetNext()->SetPrev(node);
	m_searchNodes[i].SetNext(node);
	node->distance = listId;
}

//--MIAMI: done
void
CPathFind::RemoveNodeFromList(CPathNode *node)
{
	node->GetPrev()->SetNext(node->GetNext());
	if(node->GetNext())
		node->GetNext()->SetPrev(node->GetPrev());
}

//--MIAMI: done
void
CPathFind::RemoveBadStartNode(CVector pos, CPathNode **nodes, int16 *n)
{
	int i;
	if(*n < 2)
		return;
	if(DotProduct2D(nodes[1]->GetPosition() - pos, nodes[0]->GetPosition() - pos) < 0.0f){
		(*n)--;
		for(i = 0; i < *n; i++)
			nodes[i] = nodes[i+1];
	}
}

#ifdef GTA_BRIDGE
void
CPathFind::SetLinksBridgeLights(float x1, float x2, float y1, float y2, bool enable)
{
	int i;
	for(i = 0; i < m_numCarPathLinks; i++){
		CVector2D pos = m_carPathLinks[i].GetPosition();
		if(x1 < pos.x && pos.x < x2 &&
		   y1 < pos.y && pos.y < y2)
			m_carPathLinks[i].bBridgeLights = enable;
	}
}
#endif

//--MIAMI: done
void
CPathFind::SwitchOffNodeAndNeighbours(int32 nodeId, bool disable)
{
	int i, next;

	m_pathNodes[nodeId].bDisabled = disable;
	if(m_pathNodes[nodeId].numLinks < 3)
		for(i = 0; i < m_pathNodes[nodeId].numLinks; i++){
			next = ConnectedNode(m_pathNodes[nodeId].firstLink + i);
			if(m_pathNodes[next].bDisabled != disable &&
			   m_pathNodes[next].numLinks < 3)
				SwitchOffNodeAndNeighbours(next, disable);
		}
}

//--MIAMI: done
void
CPathFind::SwitchRoadsOffInArea(float x1, float x2, float y1, float y2, float z1, float z2, bool disable)
{
	int i;

	for(i = 0; i < m_numCarPathNodes; i++){
		CVector pos = m_pathNodes[i].GetPosition();
		if(x1 <= pos.x && pos.x <= x2 &&
		   y1 <= pos.y && pos.y <= y2 &&
		   z1 <= pos.z && pos.z <= z2 &&
			disable != m_pathNodes[i].bDisabled)
			SwitchOffNodeAndNeighbours(i, disable);
	}
}

//--MIAMI: done
void
CPathFind::SwitchPedRoadsOffInArea(float x1, float x2, float y1, float y2, float z1, float z2, bool disable)
{
	int i;

	for(i = m_numCarPathNodes; i < m_numPathNodes; i++){
		CVector pos = m_pathNodes[i].GetPosition();
		if(x1 <= pos.x && pos.x <= x2 &&
		   y1 <= pos.y && pos.y <= y2 &&
		   z1 <= pos.z && pos.z <= z2 &&
			disable != m_pathNodes[i].bDisabled)
			SwitchOffNodeAndNeighbours(i, disable);
	}
}

//--MIAMI: unused (still needed for script here)
void
CPathFind::SwitchRoadsInAngledArea(float x1, float y1, float z1, float x2, float y2, float z2, float length, uint8 type, uint8 mode)
{
	int i;
	int firstNode, lastNode;

	// this is NOT PATH_CAR
	if(type != 0){
		firstNode = 0;
		lastNode = m_numCarPathNodes;
	}else{
		firstNode = m_numCarPathNodes;
		lastNode = m_numPathNodes;
	}

	if(z1 > z2){
		float tmp = z2;
		z2 = z1;
		z1 = tmp;
	}

	// angle of vector from p2 to p1
	float angle = CGeneral::GetRadianAngleBetweenPoints(x1, y1, x2, y2) + HALFPI;
	while(angle < 0.0f) angle += TWOPI;
	while(angle > TWOPI) angle -= TWOPI;
	// vector from p1 to p2
	CVector2D v12(x2 - x1, y2 - y1);
	float len12 = v12.Magnitude();
	v12 /= len12;

	// vector from p2 to new point p3
	CVector2D v23(Sin(angle)*length, -(Cos(angle)*length));
	v23 /= v23.Magnitude();	// obivously just 'length' but whatever

	bool disable = mode == SWITCH_OFF;
	for(i = firstNode; i < lastNode; i++){
		CVector pos = m_pathNodes[i].GetPosition();
		if(pos.z < z1 || pos.z > z2)
			continue;
		CVector2D d(pos.x - x1, pos.y - y1);
		float dot = DotProduct2D(d, v12);
		if(dot < 0.0f || dot > len12)
			continue;
		dot = DotProduct2D(d, v23);
		if(dot < 0.0f || dot > length)
			continue;
		if(m_pathNodes[i].bDisabled != disable)
			SwitchOffNodeAndNeighbours(i, disable);
	}
}

//--MIAMI: unused (still needed for script here)
void
CPathFind::MarkRoadsBetweenLevelsNodeAndNeighbours(int32 nodeId)
{
	int i, next;

	m_pathNodes[nodeId].bBetweenLevels = true;
	if(m_pathNodes[nodeId].numLinks < 3)
		for(i = 0; i < m_pathNodes[nodeId].numLinks; i++){
			next = ConnectedNode(m_pathNodes[nodeId].firstLink + i);
			if(!m_pathNodes[next].bBetweenLevels &&
			   m_pathNodes[next].numLinks < 3)
				MarkRoadsBetweenLevelsNodeAndNeighbours(next);
		}
}

//--MIAMI: unused (still needed for script here)
void
CPathFind::MarkRoadsBetweenLevelsInArea(float x1, float x2, float y1, float y2, float z1, float z2)
{
	int i;

	for(i = 0; i < m_numPathNodes; i++){
		CVector pos = m_pathNodes[i].GetPosition();
		if(x1 < pos.x && pos.x < x2 &&
		   y1 < pos.y && pos.y < y2 &&
		   z1 < pos.z && pos.z < z2)
			MarkRoadsBetweenLevelsNodeAndNeighbours(i);
	}
}

//--MIAMI: unused (still needed for script here)
void
CPathFind::PedMarkRoadsBetweenLevelsInArea(float x1, float x2, float y1, float y2, float z1, float z2)
{
	int i;

	for(i = m_numCarPathNodes; i < m_numPathNodes; i++){
		CVector pos = m_pathNodes[i].GetPosition();
		if(x1 < pos.x && pos.x < x2 &&
		   y1 < pos.y && pos.y < y2 &&
		   z1 < pos.z && pos.z < z2)
			MarkRoadsBetweenLevelsNodeAndNeighbours(i);
	}
}

//--MIAMI: done
int32
CPathFind::FindNodeClosestToCoors(CVector coors, uint8 type, float distLimit, bool ignoreDisabled, bool ignoreBetweenLevels, bool ignoreFlagB4, bool bWaterPath)
{
	int i;
	int firstNode, lastNode;
	float dist;
	float closestDist = 10000.0f;
	int closestNode = 0;

	switch(type){
	case PATH_CAR:
		firstNode = 0;
		lastNode = m_numCarPathNodes;
		break;
	case PATH_PED:
		firstNode = m_numCarPathNodes;
		lastNode = m_numPathNodes;
		break;
	}

	for(i = firstNode; i < lastNode; i++){
		if(ignoreDisabled && m_pathNodes[i].bDisabled) continue;
		if(ignoreBetweenLevels && m_pathNodes[i].bBetweenLevels) continue;
		if(ignoreFlagB4 && m_pathNodes[i].flagB4) continue;
		if(bWaterPath != m_pathNodes[i].bWaterPath) continue;
		dist = Abs(m_pathNodes[i].GetX() - coors.x) +
		       Abs(m_pathNodes[i].GetY() - coors.y) +
		       3.0f*Abs(m_pathNodes[i].GetZ() - coors.z);
		if(dist < closestDist){
			closestDist = dist;
			closestNode = i;
		}
	}
	return closestDist < distLimit ? closestNode : -1;
}

//--MIAMI: done
int32
CPathFind::FindNodeClosestToCoorsFavourDirection(CVector coors, uint8 type, float dirX, float dirY)
{
	int i;
	int firstNode, lastNode;
	float dist, dX, dY;
	NormalizeXY(dirX, dirY);
	float closestDist = 10000.0f;
	int closestNode = 0;

	switch(type){
	case PATH_CAR:
		firstNode = 0;
		lastNode = m_numCarPathNodes;
		break;
	case PATH_PED:
		firstNode = m_numCarPathNodes;
		lastNode = m_numPathNodes;
		break;
	}

	for(i = firstNode; i < lastNode; i++){
		dX = m_pathNodes[i].GetX() - coors.x;
		dY = m_pathNodes[i].GetY() - coors.y;
		dist = Abs(dX) + Abs(dY) +
		       3.0f*Abs(m_pathNodes[i].GetZ() - coors.z);
		if(dist < closestDist){
			NormalizeXY(dX, dY);
			dist -= (dX*dirX + dY*dirY - 1.0f)*20.0f;
			if(dist < closestDist){
				closestDist = dist;
				closestNode = i;
			}
		}
	}
	return closestNode;
}

//--MIAMI: done
float
CPathFind::FindNodeOrientationForCarPlacement(int32 nodeId)
{
	if(m_pathNodes[nodeId].numLinks == 0)
		return 0.0f;
	CVector dir = m_pathNodes[ConnectedNode(m_pathNodes[nodeId].firstLink)].GetPosition() - m_pathNodes[nodeId].GetPosition();
	dir.z = 0.0f;
	dir.Normalise();
	return RADTODEG(dir.Heading());
}

//--MIAMI: unused (still needed for script here)
float
CPathFind::FindNodeOrientationForCarPlacementFacingDestination(int32 nodeId, float x, float y, bool towards)
{
	int i;

	CVector targetDir(x - m_pathNodes[nodeId].GetX(), y - m_pathNodes[nodeId].GetY(), 0.0f);
	targetDir.Normalise();
	CVector dir;

	if(m_pathNodes[nodeId].numLinks == 0)
		return 0.0f;

	int bestNode = ConnectedNode(m_pathNodes[nodeId].firstLink);
#ifdef FIX_BUGS
	float bestDot = towards ? -2.0f : 2.0f;
#else
	int bestDot = towards ? -2 : 2;	// why int?
#endif

	for(i = 0; i < m_pathNodes[nodeId].numLinks; i++){
		dir = m_pathNodes[ConnectedNode(m_pathNodes[nodeId].firstLink + i)].GetPosition() - m_pathNodes[nodeId].GetPosition();
		dir.z = 0.0f;
		dir.Normalise();
		float angle = DotProduct2D(dir, targetDir);
		if(towards){
			if(angle > bestDot){
				bestDot = angle;
				bestNode = ConnectedNode(m_pathNodes[nodeId].firstLink + i);
			}
		}else{
			if(angle < bestDot){
				bestDot = angle;
				bestNode = ConnectedNode(m_pathNodes[nodeId].firstLink + i);
			}
		}
	}

	dir = m_pathNodes[bestNode].GetPosition() - m_pathNodes[nodeId].GetPosition();
	dir.z = 0.0f;
	dir.Normalise();
	return RADTODEG(dir.Heading());
}

// no "New" in MIAMI
//--MIAMI: TODO
bool
CPathFind::NewGenerateCarCreationCoors(float x, float y, float dirX, float dirY, float spawnDist, float angleLimit, bool forward, CVector *pPosition, int32 *pNode1, int32 *pNode2, float *pPositionBetweenNodes, bool ignoreDisabled)
{
	int i, j;
	int node1, node2;
	float dist1, dist2, d1, d2;

	if(m_numCarPathNodes == 0)
		return false;

	for(i = 0; i < 500; i++){
		node1 = (CGeneral::GetRandomNumber()>>3) % m_numCarPathNodes;
		if(m_pathNodes[node1].bDisabled && !ignoreDisabled)
			continue;
		dist1 = Distance2D(m_pathNodes[node1].GetPosition(), x, y);
		if(dist1 < Max(spawnDist + 70.0f, spawnDist * 1.7f)){
			d1 = m_pathNodes[node1].bWaterPath ? (dist1 - spawnDist * 1.5f) : (dist1 - spawnDist);
			for(j = 0; j < m_pathNodes[node1].numLinks; j++){
				node2 = ConnectedNode(m_pathNodes[node1].firstLink + j);
				if(m_pathNodes[node2].bDisabled && !ignoreDisabled)
					continue;
				dist2 = Distance2D(m_pathNodes[node2].GetPosition(), x, y);
				d2 = m_pathNodes[node2].bWaterPath ? (dist2 - spawnDist * 1.5f) : (dist2 - spawnDist);
				if(d1*d2 < 0.0f){
					// nodes are on different sides of spawn distance
					float f2 = Abs(d1)/(Abs(d1) + Abs(d2));
					float f1 = 1.0f - f2;
					*pPositionBetweenNodes = f2;
					CVector pos = m_pathNodes[node1].GetPosition()*f1 + m_pathNodes[node2].GetPosition()*f2;
					CVector2D dist2d(pos.x - x, pos.y - y);
					dist2d.Normalise();	// done manually in the game
					float dot = DotProduct2D(dist2d, CVector2D(dirX, dirY));
					if(forward){
						if(dot > angleLimit){
							*pNode1 = node1;
							*pNode2 = node2;
							*pPosition = pos;
							return true;
						}
					}else{
						if(dot <= angleLimit){
							*pNode1 = node1;
							*pNode2 = node2;
							*pPosition = pos;
							return true;
						}
					}
				}
			}
		}
	}
	return false;
}

//--MIAMI: TODO
bool
CPathFind::GeneratePedCreationCoors(float x, float y, float minDist, float maxDist, float minDistOffScreen, float maxDistOffScreen, CVector *pPosition, int32 *pNode1, int32 *pNode2, float *pPositionBetweenNodes, CMatrix *camMatrix)
{
	int i;
	int node1, node2;

	if(m_numPedPathNodes == 0)
		return false;

	for(i = 0; i < 400; i++){
		node1 = m_numCarPathNodes + CGeneral::GetRandomNumber() % m_numPedPathNodes;
		if(DistanceSqr2D(m_pathNodes[node1].GetPosition(), x, y) < sq(maxDist+30.0f)){
			if(m_pathNodes[node1].numLinks == 0)
				continue;
			int link = m_pathNodes[node1].firstLink + CGeneral::GetRandomNumber() % m_pathNodes[node1].numLinks;
			if(ConnectionCrossesRoad(link))
				continue;
			node2 = ConnectedNode(link);
			if(m_pathNodes[node1].bDisabled || m_pathNodes[node2].bDisabled)
				continue;

			float f2 = (CGeneral::GetRandomNumber()&0xFF)/256.0f;
			float f1 = 1.0f - f2;
			*pPositionBetweenNodes = f2;
			CVector pos = m_pathNodes[node1].GetPosition()*f1 + m_pathNodes[node2].GetPosition()*f2;
			if(Distance2D(pos, x, y) < maxDist+20.0f){
				pos.x += ((CGeneral::GetRandomNumber()&0xFF)-128)*0.01f;
				pos.y += ((CGeneral::GetRandomNumber()&0xFF)-128)*0.01f;
				float dist = Distance2D(pos, x, y);

				bool visible;
				if(camMatrix)
					visible = TheCamera.IsSphereVisible(pos, 2.0f, camMatrix);
				else
					visible = TheCamera.IsSphereVisible(pos, 2.0f);
				if(!visible){
					minDist = minDistOffScreen;
					maxDist = maxDistOffScreen;
				}
				if(minDist < dist && dist < maxDist){
					*pNode1 = node1;
					*pNode2 = node2;
					*pPosition = pos;

					bool found;
					float groundZ = CWorld::FindGroundZFor3DCoord(pos.x, pos.y, pos.z+2.0f, &found);
					if(!found)
						return false;
					if(Abs(groundZ - pos.z) > 3.0f)
						return false;
					pPosition->z = groundZ;
					return true;
				}
			}
		}
	}
	return false;
}

//--MIAMI: done
void
CPathFind::FindNextNodeWandering(uint8 type, CVector coors, CPathNode **lastNode, CPathNode **nextNode, uint8 curDir, uint8 *nextDir)
{
	int i;
	CPathNode *node;

	if(lastNode == nil || (node = *lastNode) == nil || (coors - (*lastNode)->GetPosition()).MagnitudeSqr() > 7.0f){
		int32 nodeIdx = FindNodeClosestToCoors(coors, type, 999999.88f);
		node = &m_pathNodes[nodeIdx];
	}

	CVector2D vCurDir(Sin(curDir*PI/4.0f), Cos(curDir * PI / 4.0f));
	*nextNode = 0;
	float bestDot = -999999.0f;
	for(i = 0; i < node->numLinks; i++){
		int next = ConnectedNode(node->firstLink+i);
		if(!node->bDisabled && m_pathNodes[next].bDisabled)
			continue;
		CVector pedCoors = coors;
		pedCoors.z += 1.0f;
		CVector nodeCoors = m_pathNodes[next].GetPosition();
		nodeCoors.z += 1.0f;
		if(!CWorld::GetIsLineOfSightClear(pedCoors, nodeCoors, true, false, false, false, false, false))
			continue;
		CVector2D nodeDir = m_pathNodes[next].GetPosition() - node->GetPosition();
		nodeDir.Normalise();
		float dot = DotProduct2D(nodeDir, vCurDir);
		if(dot >= bestDot){
			*nextNode = &m_pathNodes[next];
			bestDot = dot;

			// direction is 0, 2, 4, 6 for north, east, south, west
			// this could be done simpler...
			if(nodeDir.x < 0.0f){
				if(2.0f*Abs(nodeDir.y) < -nodeDir.x)
					*nextDir = 6;	// west
				else if(-2.0f*nodeDir.x < nodeDir.y)
					*nextDir = 0;	// north
				else if(2.0f*nodeDir.x > nodeDir.y)
					*nextDir = 4;	// south
				else if(nodeDir.y > 0.0f)
					*nextDir = 7;	// north west
				else
					*nextDir = 5;	// south west`
			}else{
				if(2.0f*Abs(nodeDir.y) < nodeDir.x)
					*nextDir = 2;	// east
				else if(2.0f*nodeDir.x < nodeDir.y)
					*nextDir = 0;	// north
				else if(-2.0f*nodeDir.x > nodeDir.y)
					*nextDir = 4;	// south
				else if(nodeDir.y > 0.0f)
					*nextDir = 1;	// north east
				else
					*nextDir = 3;	// south east`
			}
		}
	}
	if(*nextNode == nil){
		*nextDir = 0;
		*nextNode = node;
	}
}

static CPathNode *apNodesToBeCleared[6525];

//--MIAMI: done
void
CPathFind::DoPathSearch(uint8 type, CVector start, int32 startNodeId, CVector target, CPathNode **nodes, int16 *pNumNodes, int16 maxNumNodes, CVehicle *vehicle, float *pDist, float distLimit, int32 targetNodeId)
{
	int i, j;

	// Find target
	if(targetNodeId < 0)
		targetNodeId = FindNodeClosestToCoors(target, type, distLimit);
	if(targetNodeId < 0) {
		*pNumNodes = 0;
		if(pDist) *pDist = 100000.0f;
		return;
	}

	// Find start
	if(startNodeId < 0)
		startNodeId = FindNodeClosestToCoors(start, type, 999999.88f);
	if(startNodeId < 0) {
		*pNumNodes = 0;
		if(pDist) *pDist = 100000.0f;
		return;
	}
	if(startNodeId == targetNodeId){
		*pNumNodes = 0;
		if(pDist) *pDist = 0.0f;
		return;
	}
	if(m_pathNodes[startNodeId].group != m_pathNodes[targetNodeId].group) {
		*pNumNodes = 0;
		if(pDist) *pDist = 100000.0f;
		return;
	}

	for(i = 0; i < ARRAY_SIZE(m_searchNodes); i++)
		m_searchNodes[i].SetNext(nil);
	AddNodeToList(&m_pathNodes[targetNodeId], 0);
	int numNodesToBeCleared = 0;
	apNodesToBeCleared[numNodesToBeCleared++] = &m_pathNodes[targetNodeId];

	// Dijkstra's algorithm
	// Find distances
	int numPathsFound = 0;
	for(i = 0; numPathsFound == 0; i = (i+1) & 0x1FF){
		CPathNode *node;
		for(node = m_searchNodes[i].GetNext(); node; node = node->GetNext()){
			if(node == &m_pathNodes[startNodeId])
				numPathsFound = 1;

			for(j = 0; j < node->numLinks; j++){
				int next = ConnectedNode(node->firstLink + j);
				int dist = node->distance + m_distances[node->firstLink + j];
				if(dist < m_pathNodes[next].distance){
					if(m_pathNodes[next].distance != MAX_DIST)
						RemoveNodeFromList(&m_pathNodes[next]);
					if(m_pathNodes[next].distance == MAX_DIST)
						apNodesToBeCleared[numNodesToBeCleared++] = &m_pathNodes[next];
					AddNodeToList(&m_pathNodes[next], dist);
				}
			}

			RemoveNodeFromList(node);
		}
	}

	// Find out whence to start tracing back
	CPathNode *curNode;
	curNode = &m_pathNodes[startNodeId];
	*pNumNodes = 0;
	if(pDist)
		*pDist = m_pathNodes[startNodeId].distance;

	nodes[(*pNumNodes)++] = curNode;
	// Trace back to target and update list of nodes
	while(*pNumNodes < maxNumNodes && curNode != &m_pathNodes[targetNodeId])
		for(i = 0; i < curNode->numLinks; i++){
			int next = ConnectedNode(curNode->firstLink + i);
			if(curNode->distance - m_distances[curNode->firstLink + i] == m_pathNodes[next].distance){
				curNode = &m_pathNodes[next];
				nodes[(*pNumNodes)++] = curNode;
				i = 29030;	// could have used a break...
			}
		}

	for(i = 0; i < numNodesToBeCleared; i++)
		apNodesToBeCleared[i]->distance = MAX_DIST;
}

static CPathNode *pNodeList[32];
static int16 DummyResult;
static int16 DummyResult2;

//--MIAMI: done
bool
CPathFind::TestCoorsCloseness(CVector target, uint8 type, CVector start)
{
	float dist;

	if(type == PATH_CAR)
		DoPathSearch(type, start, -1, target, pNodeList, &DummyResult, 32, nil, &dist, 999999.88f, -1);
	else
		DoPathSearch(type, start, -1, target, nil, &DummyResult2, 0, nil, &dist, 50.0f, -1);
	if(type == PATH_CAR)
		return dist < 150.0f;
	else
		return dist < 100.0f;
}

//--MIAMI: done
void
CPathFind::Save(uint8 *buf, uint32 *size)
{
	int i;
	int n = m_numPathNodes/8 + 1;

	*size = 2*n;

	for(i = 0; i < m_numPathNodes; i++)
		if(m_pathNodes[i].bDisabled)
			buf[i/8] |= 1 << i%8;
		else
			buf[i/8] &= ~(1 << i%8);

	for(i = 0; i < m_numPathNodes; i++)
		if(m_pathNodes[i].bBetweenLevels)
			buf[i/8 + n] |= 1 << i%8;
		else
			buf[i/8 + n] &= ~(1 << i%8);
}

//--MIAMI: done
void
CPathFind::Load(uint8 *buf, uint32 size)
{
	int i;
	int n = m_numPathNodes/8 + 1;

	for(i = 0; i < m_numPathNodes; i++)
		if(buf[i/8] & (1 << i%8))
			m_pathNodes[i].bDisabled = true;
		else
			m_pathNodes[i].bDisabled = false;

	for(i = 0; i < m_numPathNodes; i++)
		if(buf[i/8 + n] & (1 << i%8))
			m_pathNodes[i].bBetweenLevels = true;
		else
			m_pathNodes[i].bBetweenLevels = false;
}

void
CPathFind::DisplayPathData(void)
{
	// Not the function from mobm_carPathLinksile but my own!

	int i, j, k;
	// Draw 50 units around camera
	CVector pos = TheCamera.GetPosition();
	const float maxDist = 50.0f;

	// Render car path nodes
	if(gbShowCarPaths)
	for(i = 0; i < m_numCarPathNodes; i++){
		if((m_pathNodes[i].GetPosition() - pos).MagnitudeSqr() > SQR(maxDist))
			continue;

		CVector n1 = m_pathNodes[i].GetPosition();
		n1.z += 0.3f;

		// Draw node itself
		CLines::RenderLineWithClipping(n1.x, n1.y, n1.z,
			n1.x, n1.y, n1.z + 1.0f,
			0xFFFFFFFF, 0xFFFFFFFF);

		for(j = 0; j < m_pathNodes[i].numLinks; j++){
			k = ConnectedNode(m_pathNodes[i].firstLink + j);
			CVector n2 = m_pathNodes[k].GetPosition();
			n2.z += 0.3f;
			// Draw links to neighbours
			CLines::RenderLineWithClipping(n1.x, n1.y, n1.z,
				n2.x, n2.y, n2.z,
				0xFFFFFFFF, 0xFFFFFFFF);
		}
	}

	// Render car path nodes
	if(gbShowCarPathsLinks)
	for(i = 0; i < m_numCarPathLinks; i++){
		CVector2D n1_2d = m_carPathLinks[i].GetPosition();
		if((n1_2d - pos).MagnitudeSqr() > SQR(maxDist))
			continue;

		int ni = m_carPathLinks[i].pathNodeIndex;
		CVector pn1 = m_pathNodes[ni].GetPosition();
		pn1.z += 0.3f;
		CVector n1(n1_2d.x, n1_2d.y, pn1.z);
		n1.z += 0.3f;

		// Draw car node itself
		CLines::RenderLineWithClipping(n1.x, n1.y, n1.z,
			n1.x, n1.y, n1.z + 1.0f,
			0xFFFFFFFF, 0xFFFFFFFF);
		CLines::RenderLineWithClipping(n1.x, n1.y, n1.z + 0.5f,
			n1.x+m_carPathLinks[i].GetDirX(), n1.y+m_carPathLinks[i].GetDirY(), n1.z + 0.5f,
			0xFFFFFFFF, 0xFFFFFFFF);

		// Draw connection to car path node
		CLines::RenderLineWithClipping(n1.x, n1.y, n1.z,
			pn1.x, pn1.y, pn1.z,
			0xFF0000FF, 0xFFFFFFFF);

		// traffic light type
		uint32 col = 0xFF;
		if((m_carPathLinks[i].trafficLightType&0x7F) == 1)
			col += 0xFF000000;
		if((m_carPathLinks[i].trafficLightType&0x7F) == 2)
			col += 0x00FF0000;
		if(m_carPathLinks[i].trafficLightType & 0x80)
			col += 0x0000FF00;
		CLines::RenderLineWithClipping(n1.x+0.2f, n1.y, n1.z,
			n1.x+0.2f, n1.y, n1.z + 1.0f,
			col, col);

		for(j = 0; j < m_pathNodes[ni].numLinks; j++){
			k = m_carPathConnections[m_pathNodes[ni].firstLink + j];
			CVector2D n2_2d = m_carPathLinks[k].GetPosition();
			int nk = m_carPathLinks[k].pathNodeIndex;
			CVector pn2 = m_pathNodes[nk].GetPosition();
			pn2.z += 0.3f;
			CVector n2(n2_2d.x, n2_2d.y, pn2.z);
			n2.z += 0.3f;

			// Draw links to neighbours
			CLines::RenderLineWithClipping(n1.x, n1.y, n1.z,
				n2.x, n2.y, n2.z,
				0xFF00FFFF, 0xFF00FFFF);
		}
	}

	// Render ped path nodes
	if(gbShowPedPaths)
	for(i = m_numCarPathNodes; i < m_numPathNodes; i++){
		if((m_pathNodes[i].GetPosition() - pos).MagnitudeSqr() > SQR(maxDist))
			continue;

		CVector n1 = m_pathNodes[i].GetPosition();
		n1.z += 0.3f;

		// Draw node itself
		CLines::RenderLineWithClipping(n1.x, n1.y, n1.z,
			n1.x, n1.y, n1.z + 1.0f,
			0xFFFFFFFF, 0xFFFFFFFF);

		for(j = 0; j < m_pathNodes[i].numLinks; j++){
			k = ConnectedNode(m_pathNodes[i].firstLink + j);
			CVector n2 = m_pathNodes[k].GetPosition();
			n2.z += 0.3f;
			// Draw links to neighbours
			CLines::RenderLineWithClipping(n1.x, n1.y, n1.z,
				n2.x, n2.y, n2.z,
				0xFFFFFFFF, 0xFFFFFFFF);

			// Draw connection flags
			CVector mid = (n1+n2)/2.0f;
			uint32 col = 0xFF;
			if(ConnectionCrossesRoad(m_pathNodes[i].firstLink + j))
				col += 0x00FF0000;
			if(ConnectionHasTrafficLight(m_pathNodes[i].firstLink + j))
				col += 0xFF000000;
			CLines::RenderLineWithClipping(mid.x, mid.y, mid.z,
				mid.x, mid.y, mid.z + 1.0f,
				col, col);
		}
	}
}

CVector
CPathFind::TakeWidthIntoAccountForWandering(CPathNode* nextNode, uint16 random)
{
	CVector pos = nextNode->GetPosition();
	float newX = (nextNode->GetPedNodeWidth() * ((random % 16) - 7)) + pos.x;
	float newY = (nextNode->GetPedNodeWidth() * (((random / 16) % 16) - 7)) + pos.y;
	return CVector(newX, newY, pos.z);
}

CPathNode*
CPathFind::GetNode(int16 index)
{
	if(index < 0)
		return nil;
	if(index < ARRAY_SIZE(ThePaths.m_searchNodes))
		return &ThePaths.m_searchNodes[index];
	return &ThePaths.m_pathNodes[index - ARRAY_SIZE(ThePaths.m_searchNodes)];
}
int16
CPathFind::GetIndex(CPathNode *node)
{
	if(node == nil)
		return -1;
	if(node >= &ThePaths.m_searchNodes[0] && node < &ThePaths.m_searchNodes[ARRAY_SIZE(ThePaths.m_searchNodes)])
		return node - ThePaths.m_searchNodes;
	else
		return (node - ThePaths.m_pathNodes) + ARRAY_SIZE(ThePaths.m_searchNodes);
}
