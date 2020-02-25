#include "common.h"
#include "patcher.h"
#include "General.h"
#include "FileMgr.h"	// only needed for empty function
#include "Camera.h"
#include "Vehicle.h"
#include "World.h"
#include "PathFind.h"

CPathFind &ThePaths = *(CPathFind*)0x8F6754;

WRAPPER bool CPedPath::CalcPedRoute(uint8, CVector, CVector, CVector*, int16*, int16) { EAXJMP(0x42E680); }

enum
{
	NodeTypeExtern = 1,
	NodeTypeIntern = 2,

	ObjectFlag1 = 1,
	ObjectEastWest = 2,

	MAX_DIST = INT16_MAX-1
};

// object flags:
//	1
//	2	east/west road(?)

CPathInfoForObject *&InfoForTileCars = *(CPathInfoForObject**)0x8F1A8C;
CPathInfoForObject *&InfoForTilePeds  = *(CPathInfoForObject**)0x8F1AE4;
// unused
CTempDetachedNode *&DetachedNodesCars = *(CTempDetachedNode**)0x8E2824;
CTempDetachedNode *&DetachedNodesPeds = *(CTempDetachedNode**)0x8E28A0;

void
CPathFind::Init(void)
{
	int i;

	m_numPathNodes = 0;
	m_numMapObjects = 0;
	m_numConnections = 0;
	m_numCarPathLinks = 0;
	unk = 0;

	for(i = 0; i < NUM_PATHNODES; i++)
		m_pathNodes[i].distance = MAX_DIST;
}

void
CPathFind::AllocatePathFindInfoMem(int16 numPathGroups)
{
	delete[] InfoForTileCars;
	InfoForTileCars = nil;
	delete[] InfoForTilePeds;
	InfoForTilePeds = nil;

	InfoForTileCars = new CPathInfoForObject[12*numPathGroups];
	memset(InfoForTileCars, 0, 12*numPathGroups*sizeof(CPathInfoForObject));
	InfoForTilePeds = new CPathInfoForObject[12*numPathGroups];
	memset(InfoForTilePeds, 0, 12*numPathGroups*sizeof(CPathInfoForObject));

	// unused
	delete[] DetachedNodesCars;
	DetachedNodesCars = nil;
	delete[] DetachedNodesPeds;
	DetachedNodesPeds = nil;
	DetachedNodesCars = new CTempDetachedNode[100];
	memset(DetachedNodesCars, 0, 100*sizeof(CTempDetachedNode));
	DetachedNodesPeds = new CTempDetachedNode[50];
	memset(DetachedNodesPeds, 0, 50*sizeof(CTempDetachedNode));
}

void
CPathFind::RegisterMapObject(CTreadable *mapObject)
{
	m_mapObjects[m_numMapObjects++] = mapObject;
}

void
CPathFind::StoreNodeInfoPed(int16 id, int16 node, int8 type, int8 next, int16 x, int16 y, int16 z, int16 width, bool crossing)
{
	int i, j;

	i = id*12 + node;
	InfoForTilePeds[i].type = type;
	InfoForTilePeds[i].next = next;
	InfoForTilePeds[i].x = x;
	InfoForTilePeds[i].y = y;
	InfoForTilePeds[i].z = z;
	InfoForTilePeds[i].numLeftLanes = 0;
	InfoForTilePeds[i].numRightLanes = 0;
	InfoForTilePeds[i].crossing = crossing;

	if(type)
		for(i = 0; i < node; i++){
			j = id*12 + i;
			if(x == InfoForTilePeds[j].x && y == InfoForTilePeds[j].y){
				printf("^^^^^^^^^^^^^ AARON IS TOO CHICKEN TO EAT MEAT!\n");
				printf("Several ped nodes on one road segment have identical coordinates (%d==%d && %d==%d)\n",
					x, InfoForTilePeds[j].x, y, InfoForTilePeds[j].y);
				printf("Modelindex of cullprit: %d\n\n", id);
			}
		}
}

void
CPathFind::StoreNodeInfoCar(int16 id, int16 node, int8 type, int8 next, int16 x, int16 y, int16 z, int16 width, int8 numLeft, int8 numRight)
{
	int i, j;

	i = id*12 + node;
	InfoForTileCars[i].type = type;
	InfoForTileCars[i].next = next;
	InfoForTileCars[i].x = x;
	InfoForTileCars[i].y = y;
	InfoForTileCars[i].z = z;
	InfoForTileCars[i].numLeftLanes = numLeft;
	InfoForTileCars[i].numRightLanes = numRight;

	if(type)
		for(i = 0; i < node; i++){
			j = id*12 + i;
			if(x == InfoForTileCars[j].x && y == InfoForTileCars[j].y){
				printf("^^^^^^^^^^^^^ AARON IS TOO CHICKEN TO EAT MEAT!\n");
				printf("Several car nodes on one road segment have identical coordinates (%d==%d && %d==%d)\n",
					x, InfoForTileCars[j].x, y, InfoForTileCars[j].y);
				printf("Modelindex of cullprit: %d\n\n", id);
			}
		}
}

void
CPathFind::CalcNodeCoors(int16 x, int16 y, int16 z, int id, CVector *out)
{
	CVector pos;
	pos.x = x / 16.0f;
	pos.y = y / 16.0f;
	pos.z = z / 16.0f;
	*out = m_mapObjects[id]->GetMatrix() * pos;
}

bool
CPathFind::LoadPathFindData(void)
{
	CFileMgr::SetDir("");
	return false;
}

void
CPathFind::PreparePathData(void)
{
	int i, j, k;
	int numExtern, numIntern, numLanes;
	float maxX, maxY;
	CTempNode *tempNodes;

	printf("PreparePathData\n");
	if(!CPathFind::LoadPathFindData() &&	// empty
	   InfoForTileCars && InfoForTilePeds &&
	   DetachedNodesCars && DetachedNodesPeds){
		tempNodes = new CTempNode[4000];

		m_numConnections = 0;
		for(i = 0; i < PATHNODESIZE; i++)
			m_pathNodes[i].unkBits = 0;

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

		for(i = 0; i < PATHNODESIZE; i++)
			for(j = 0; j < 12; j++)
				if(InfoForTileCars[i*12 + j].type == NodeTypeExtern){
					if(InfoForTileCars[i*12 + j].numLeftLanes < 0)
						printf("ILLEGAL BLOCK. NEGATIVE NUMBER OF LANES (Obj:%d)\n", i);
					if(InfoForTileCars[i*12 + j].numRightLanes < 0)
						printf("ILLEGAL BLOCK. NEGATIVE NUMBER OF LANES (Obj:%d)\n", i);
					if(InfoForTileCars[i*12 + j].numLeftLanes + InfoForTileCars[i*12 + j].numRightLanes <= 0)
						printf("ILLEGAL BLOCK. NO LANES IN NODE (Obj:%d)\n", i);
				}

		m_numPathNodes = 0;
		PreparePathDataForType(PATH_CAR, tempNodes, InfoForTileCars, 1.0f, DetachedNodesCars, 100);
		m_numCarPathNodes = m_numPathNodes;
		PreparePathDataForType(PATH_PED, tempNodes, InfoForTilePeds, 1.0f, DetachedNodesPeds, 50);
		m_numPedPathNodes = m_numPathNodes - m_numCarPathNodes;

		// TODO: figure out what exactly is going on here
		// Some roads seem to get a west/east flag
		for(i = 0; i < m_numMapObjects; i++){
			numExtern = 0;
			numIntern = 0;
			numLanes = 0;
			maxX = 0.0f;
			maxY = 0.0f;
			for(j = 0; j < 12; j++){
				k = i*12 + j;
				if(InfoForTileCars[k].type == NodeTypeExtern){
					numExtern++;
					if(InfoForTileCars[k].numLeftLanes + InfoForTileCars[k].numRightLanes > numLanes)
						numLanes = InfoForTileCars[k].numLeftLanes + InfoForTileCars[k].numRightLanes;
					maxX = max(maxX, Abs(InfoForTileCars[k].x));
					maxY = max(maxY, Abs(InfoForTileCars[k].y));
				}else if(InfoForTileCars[k].type == NodeTypeIntern)
					numIntern++;
			}

			if(numIntern == 1 && numExtern == 2){
				if(numLanes < 4){
					if((i & 7) == 4){		// WHAT?
						m_objectFlags[i] |= ObjectFlag1;
						if(maxX > maxY)
							m_objectFlags[i] |= ObjectEastWest;
						else
							m_objectFlags[i] &= ~ObjectEastWest;
					}
				}else{
					m_objectFlags[i] |= ObjectFlag1;
					if(maxX > maxY)
						m_objectFlags[i] |= ObjectEastWest;
					else
						m_objectFlags[i] &= ~ObjectEastWest;
				}
			}
		}

		delete[] tempNodes;

		CountFloodFillGroups(PATH_CAR);
		CountFloodFillGroups(PATH_PED);

		delete[] InfoForTileCars;
		InfoForTileCars = nil;
		delete[] InfoForTilePeds;
		InfoForTilePeds = nil;
		delete[] DetachedNodesCars;
		DetachedNodesCars = nil;
		delete[] DetachedNodesPeds;
		DetachedNodesPeds = nil;
	}
	printf("Done with PreparePathData\n");
}

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
		node->next = nil;
		node->group = n;

		if(node->numLinks == 0){
			if(type == PATH_CAR)
				printf("Single car node: %f %f %f (%d)\n",
					node->pos.x, node->pos.y, node->pos.z,
					m_mapObjects[node->objectIndex]->m_modelIndex);
			else
				printf("Single ped node: %f %f %f\n",
					node->pos.x, node->pos.y, node->pos.z);
		}

		while(node){
			prev = node;
			node = node->next;
			for(i = 0; i < prev->numLinks; i++){
				l = m_connections[prev->firstLink + i];
				if(m_pathNodes[l].group == 0){
					m_pathNodes[l].group = n;
					if(m_pathNodes[l].group == 0)
						m_pathNodes[l].group = INT8_MIN;
					m_pathNodes[l].next = node;
					node = &m_pathNodes[l];
				}
			}
		}
	}

	m_numGroups[type] = n-1;
	printf("GraphType:%d. FloodFill groups:%d\n", type, n);
}

int32 TempListLength;

void
CPathFind::PreparePathDataForType(uint8 type, CTempNode *tempnodes, CPathInfoForObject *objectpathinfo,
	float maxdist, CTempDetachedNode *detachednodes, int unused)
{
	static CVector CoorsXFormed;
	int i, j, k, l;
	int l1, l2;
	int start;
	float posx, posy;
	float dx, dy, mag;
	float nearestDist;
	int nearestId;
	int next;
	int oldNumPathNodes, oldNumLinks;
	CVector dist;
	int iseg, jseg;
	int istart, jstart;
	int done, cont;

	oldNumPathNodes = m_numPathNodes;
	oldNumLinks = m_numConnections;

	// Initialize map objects
	for(i = 0; i < m_numMapObjects; i++)
		for(j = 0; j < 12; j++)
			m_mapObjects[i]->m_nodeIndices[type][j] = -1;

	// Calculate internal nodes, store them and connect them to defining object
	for(i = 0; i < m_numMapObjects; i++){
		start = 12*m_mapObjects[i]->m_modelIndex;
		for(j = 0; j < 12; j++){
			if(objectpathinfo[start + j].type != NodeTypeIntern)
				continue;
			CalcNodeCoors(
				objectpathinfo[start + j].x,
				objectpathinfo[start + j].y,
				objectpathinfo[start + j].z,
				i,
				&CoorsXFormed);
			m_pathNodes[m_numPathNodes].pos = CoorsXFormed;
			m_pathNodes[m_numPathNodes].objectIndex = i;
			m_pathNodes[m_numPathNodes].unkBits = 1;
			m_mapObjects[i]->m_nodeIndices[type][j] = m_numPathNodes++;
		}
	}

	// Insert external nodes into TempList
	TempListLength = 0;
	for(i = 0; i < m_numMapObjects; i++){
		start = 12*m_mapObjects[i]->m_modelIndex;

		for(j = 0; j < 12; j++){
			if(objectpathinfo[start + j].type != NodeTypeExtern)
				continue;
			CalcNodeCoors(
				objectpathinfo[start + j].x,
				objectpathinfo[start + j].y,
				objectpathinfo[start + j].z,
				i,
				&CoorsXFormed);

			// find closest unconnected node
			nearestId = -1;
			nearestDist = maxdist;
			for(k = 0; k < TempListLength; k++){
				if(tempnodes[k].linkState != 1)
					continue;
				dx = tempnodes[k].pos.x - CoorsXFormed.x;
				if(Abs(dx) < nearestDist){
					dy = tempnodes[k].pos.y - CoorsXFormed.y;
					if(Abs(dy) < nearestDist){
						nearestDist = max(Abs(dx), Abs(dy));
						nearestId = k;
					}
				}
			}

			if(nearestId < 0){
				// None found, add this one to temp list
				tempnodes[TempListLength].pos = CoorsXFormed;
				next = objectpathinfo[start + j].next;
				if(next < 0){
					// no link from this node, find link to this node
					next = 0;
					for(k = start; j != objectpathinfo[k].next; k++)
						next++;
				}
				// link to connecting internal node
				tempnodes[TempListLength].link1 = m_mapObjects[i]->m_nodeIndices[type][next];
				if(type == PATH_CAR){
					tempnodes[TempListLength].numLeftLanes = objectpathinfo[start + j].numLeftLanes;
					tempnodes[TempListLength].numRightLanes = objectpathinfo[start + j].numRightLanes;
				}
				tempnodes[TempListLength++].linkState = 1;
			}else{
				// Found nearest, connect it to our neighbour
				next = objectpathinfo[start + j].next;
				if(next < 0){
					// no link from this node, find link to this node
					next = 0;
					for(k = start; j != objectpathinfo[k].next; k++)
						next++;
				}
				tempnodes[nearestId].link2 = m_mapObjects[i]->m_nodeIndices[type][next];
				tempnodes[nearestId].linkState = 2;

				// collapse this node with nearest we found
				dx = m_pathNodes[tempnodes[nearestId].link1].pos.x - m_pathNodes[tempnodes[nearestId].link2].pos.x;
				dy = m_pathNodes[tempnodes[nearestId].link1].pos.y - m_pathNodes[tempnodes[nearestId].link2].pos.y;
				tempnodes[nearestId].pos = (tempnodes[nearestId].pos + CoorsXFormed)*0.5f;
				mag = Sqrt(dx*dx + dy*dy);
				tempnodes[nearestId].dirX = dx/mag;
				tempnodes[nearestId].dirY = dy/mag;
				// do something when number of lanes doesn't agree
				if(type == PATH_CAR)
					if(tempnodes[nearestId].numLeftLanes != 0 && tempnodes[nearestId].numRightLanes != 0 &&
					   (objectpathinfo[start + j].numLeftLanes == 0 || objectpathinfo[start + j].numRightLanes == 0)){
						// why switch left and right here?
						tempnodes[nearestId].numLeftLanes = objectpathinfo[start + j].numRightLanes;
						tempnodes[nearestId].numRightLanes = objectpathinfo[start + j].numLeftLanes;
					}
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
			if(tempnodes[j].link1 == i)
				m_connections[m_numConnections] = tempnodes[j].link2;
			else if(tempnodes[j].link2 == i)
				m_connections[m_numConnections] = tempnodes[j].link1;
			else
				continue;

			dist = m_pathNodes[i].pos - m_pathNodes[m_connections[m_numConnections]].pos;
			m_distances[m_numConnections] = dist.Magnitude();
			m_connectionFlags[m_numConnections].flags = 0;

			if(type == PATH_CAR){
				// IMPROVE: use a goto here
				// Find existing car path link
				for(k = 0; k < m_numCarPathLinks; k++){
					if(m_carPathLinks[k].dirX == tempnodes[j].dirX &&
					   m_carPathLinks[k].dirY == tempnodes[j].dirY &&
					   m_carPathLinks[k].posX == tempnodes[j].pos.x &&
					   m_carPathLinks[k].posY == tempnodes[j].pos.y){
						m_carPathConnections[m_numConnections] = k;
						k = m_numCarPathLinks;
					}
				}
				// k is m_numCarPathLinks+1 if we found one
				if(k == m_numCarPathLinks){
					m_carPathLinks[m_numCarPathLinks].dirX = tempnodes[j].dirX;
					m_carPathLinks[m_numCarPathLinks].dirY = tempnodes[j].dirY;
					m_carPathLinks[m_numCarPathLinks].posX = tempnodes[j].pos.x;
					m_carPathLinks[m_numCarPathLinks].posY = tempnodes[j].pos.y;
					m_carPathLinks[m_numCarPathLinks].pathNodeIndex = i;
					m_carPathLinks[m_numCarPathLinks].numLeftLanes = tempnodes[j].numLeftLanes;
					m_carPathLinks[m_numCarPathLinks].numRightLanes = tempnodes[j].numRightLanes;
					m_carPathLinks[m_numCarPathLinks].trafficLightType = 0;
					m_carPathConnections[m_numConnections] = m_numCarPathLinks++;
				}
			}

			m_pathNodes[i].numLinks++;
			m_numConnections++;
		}

		// Find i inside path segment
		iseg = 0;
		for(j = max(oldNumPathNodes, i-12); j < i; j++)
			if(m_pathNodes[j].objectIndex == m_pathNodes[i].objectIndex)
				iseg++;

		istart = 12*m_mapObjects[m_pathNodes[i].objectIndex]->m_modelIndex;
		// Add links to other internal nodes
		for(j = max(oldNumPathNodes, i-12); j < min(m_numPathNodes, i+12); j++){
			if(m_pathNodes[i].objectIndex != m_pathNodes[j].objectIndex || i == j)
				continue;
			// N.B.: in every path segment, the externals have to be at the end
			jseg = j-i + iseg;

			jstart = 12*m_mapObjects[m_pathNodes[j].objectIndex]->m_modelIndex;
			if(objectpathinfo[istart + iseg].next == jseg ||
			   objectpathinfo[jstart + jseg].next == iseg){
				// Found a link between i and j
				m_connections[m_numConnections] = j;
				dist = m_pathNodes[i].pos - m_pathNodes[j].pos;
				m_distances[m_numConnections] = dist.Magnitude();

				if(type == PATH_CAR){
					posx = (m_pathNodes[i].pos.x + m_pathNodes[j].pos.x)*0.5f;
					posy = (m_pathNodes[i].pos.y + m_pathNodes[j].pos.y)*0.5f;
					dx = m_pathNodes[j].pos.x - m_pathNodes[i].pos.x;
					dy = m_pathNodes[j].pos.y - m_pathNodes[i].pos.y;
					mag = Sqrt(dx*dx + dy*dy);
					dx /= mag;
					dy /= mag;
					if(i < j){
						dx = -dx;
						dy = -dy;
					}
					// IMPROVE: use a goto here
					// Find existing car path link
					for(k = 0; k < m_numCarPathLinks; k++){
						if(m_carPathLinks[k].dirX == dx &&
						   m_carPathLinks[k].dirY == dy &&
						   m_carPathLinks[k].posX == posx &&
						   m_carPathLinks[k].posY == posy){
							m_carPathConnections[m_numConnections] = k;
							k = m_numCarPathLinks;
						}
					}
					// k is m_numCarPathLinks+1 if we found one
					if(k == m_numCarPathLinks){
						m_carPathLinks[m_numCarPathLinks].dirX = dx;
						m_carPathLinks[m_numCarPathLinks].dirY = dy;
						m_carPathLinks[m_numCarPathLinks].posX = posx;
						m_carPathLinks[m_numCarPathLinks].posY = posy;
						m_carPathLinks[m_numCarPathLinks].pathNodeIndex = i;
						m_carPathLinks[m_numCarPathLinks].numLeftLanes = -1;
						m_carPathLinks[m_numCarPathLinks].numRightLanes = -1;
						m_carPathLinks[m_numCarPathLinks].trafficLightType = 0;
						m_carPathConnections[m_numConnections] = m_numCarPathLinks++;
					}
				}else{
					// Crosses road
					if(objectpathinfo[istart + iseg].next == jseg && objectpathinfo[istart + iseg].crossing ||
					   objectpathinfo[jstart + jseg].next == iseg && objectpathinfo[jstart + jseg].crossing)
						m_connectionFlags[m_numConnections].bCrossesRoad = true;
					else
						m_connectionFlags[m_numConnections].bCrossesRoad = false;
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
		for(k = 0; !done && k < 10; k++){
			done = 1;
			for(i = 0; i < m_numPathNodes; i++){
				if(m_pathNodes[i].numLinks != 2)
					continue;
				l1 = m_carPathConnections[m_pathNodes[i].firstLink];
				l2 = m_carPathConnections[m_pathNodes[i].firstLink+1];

				if(m_carPathLinks[l1].numLeftLanes == -1 &&
				   m_carPathLinks[l2].numLeftLanes != -1){
					done = 0;
					if(m_carPathLinks[l2].pathNodeIndex == i){
						// why switch left and right here?
						m_carPathLinks[l1].numLeftLanes = m_carPathLinks[l2].numRightLanes;
						m_carPathLinks[l1].numRightLanes = m_carPathLinks[l2].numLeftLanes;
					}else{
						m_carPathLinks[l1].numLeftLanes = m_carPathLinks[l2].numLeftLanes;
						m_carPathLinks[l1].numRightLanes = m_carPathLinks[l2].numRightLanes;
					}
					m_carPathLinks[l1].pathNodeIndex = i;
				}else if(m_carPathLinks[l1].numLeftLanes != -1 &&
				         m_carPathLinks[l2].numLeftLanes == -1){
					done = 0;
					if(m_carPathLinks[l1].pathNodeIndex == i){
						// why switch left and right here?
						m_carPathLinks[l2].numLeftLanes = m_carPathLinks[l1].numRightLanes;
						m_carPathLinks[l2].numRightLanes = m_carPathLinks[l1].numLeftLanes;
					}else{
						m_carPathLinks[l2].numLeftLanes = m_carPathLinks[l1].numLeftLanes;
						m_carPathLinks[l2].numRightLanes = m_carPathLinks[l1].numRightLanes;
					}
					m_carPathLinks[l2].pathNodeIndex = i;
				}else if(m_carPathLinks[l1].numLeftLanes == -1 &&
				         m_carPathLinks[l2].numLeftLanes == -1)
					done = 0;
			}
		}

		// Fall back to default values for number of lanes
		for(i = 0; i < m_numPathNodes; i++)
			for(j = 0; j < m_pathNodes[i].numLinks; j++){
				k = m_carPathConnections[m_pathNodes[i].firstLink + j];
				if(m_carPathLinks[k].numLeftLanes < 0)
					m_carPathLinks[k].numLeftLanes = 1;
				if(m_carPathLinks[k].numRightLanes < 0)
					m_carPathLinks[k].numRightLanes = 1;
			}
	}

	// Set flags for car nodes
	if(type == PATH_CAR){
		do{
			cont = 0;
			for(i = 0; i < m_numPathNodes; i++){
				m_pathNodes[i].bDisabled = false;
				m_pathNodes[i].bBetweenLevels = false;
				// See if node is a dead end, if so, we're not done yet
				if(!m_pathNodes[i].bDeadEnd){
					k = 0;
					for(j = 0; j < m_pathNodes[i].numLinks; j++)
						if(!m_pathNodes[m_connections[m_pathNodes[i].firstLink + j]].bDeadEnd)
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
			for(j = oldNumLinks; j < m_numConnections; j++)
				if(m_connections[j] >= i)
					m_connections[j]--;

			// Also in treadables
			for(j = 0; j < m_numMapObjects; j++)
				for(k = 0; k < 12; k++){
					if(m_mapObjects[j]->m_nodeIndices[PATH_PED][k] == i){
						// remove this one
						for(l = k; l < 12-1; l++)
							m_mapObjects[j]->m_nodeIndices[PATH_PED][l] = m_mapObjects[j]->m_nodeIndices[PATH_PED][l+1];
						m_mapObjects[j]->m_nodeIndices[PATH_PED][11] = -1;
					}else if(m_mapObjects[j]->m_nodeIndices[PATH_PED][k] > i)
						m_mapObjects[j]->m_nodeIndices[PATH_PED][k]--;
				}

			i--;
			m_numPathNodes--;
		}
}

float
CPathFind::CalcRoadDensity(float x, float y)
{
	int i, j;
	float density = 0.0f;

	for(i = 0; i < m_numCarPathNodes; i++){
		if(Abs(m_pathNodes[i].pos.x - x) < 80.0f &&
		   Abs(m_pathNodes[i].pos.y - y) < 80.0f &&
		   m_pathNodes[i].numLinks > 0){
			for(j = 0; j < m_pathNodes[i].numLinks; j++){
				int next = m_connections[m_pathNodes[i].firstLink + j];
				float dist = (m_pathNodes[i].pos - m_pathNodes[next].pos).Magnitude2D();
				next = m_carPathConnections[m_pathNodes[i].firstLink + j];
				density += m_carPathLinks[next].numLeftLanes * dist;
				density += m_carPathLinks[next].numRightLanes * dist;

				if(m_carPathLinks[next].numLeftLanes < 0)
					printf("Link from object %d to %d (MIs)\n",
						m_mapObjects[m_pathNodes[i].objectIndex]->GetModelIndex(),
						m_mapObjects[m_pathNodes[m_connections[m_pathNodes[i].firstLink + j]].objectIndex]->GetModelIndex());
				if(m_carPathLinks[next].numRightLanes < 0)
					printf("Link from object %d to %d (MIs)\n",
						m_mapObjects[m_pathNodes[i].objectIndex]->GetModelIndex(),
						m_mapObjects[m_pathNodes[m_connections[m_pathNodes[i].firstLink + j]].objectIndex]->GetModelIndex());
			}
		}
	}
	return density/2500.0f;
}

bool
CPathFind::TestForPedTrafficLight(CPathNode *n1, CPathNode *n2)
{
	int i;
	for(i = 0; i < n1->numLinks; i++)
		if(&m_pathNodes[m_connections[n1->firstLink + i]] == n2)
			return m_connectionFlags[n1->firstLink + i].bTrafficLight;
	return false;
}

bool
CPathFind::TestCrossesRoad(CPathNode *n1, CPathNode *n2)
{
	int i;
	for(i = 0; i < n1->numLinks; i++)
		if(&m_pathNodes[m_connections[n1->firstLink + i]] == n2)
			return m_connectionFlags[n1->firstLink + i].bCrossesRoad;
	return false;
}

void
CPathFind::AddNodeToList(CPathNode *node, int32 listId)
{
	int i = listId & 0x1FF;
	node->next = m_searchNodes[i].next;
	node->prev = &m_searchNodes[i];
	if(m_searchNodes[i].next)
		m_searchNodes[i].next->prev = node;
	m_searchNodes[i].next = node;
	node->distance = listId;
}

void
CPathFind::RemoveNodeFromList(CPathNode *node)
{
	node->prev->next = node->next;
	if(node->next)
		node->next->prev = node->prev;
}

void
CPathFind::RemoveBadStartNode(CVector pos, CPathNode **nodes, int16 *n)
{
	int i;
	if(*n < 2)
		return;
	if(DotProduct2D(nodes[1]->pos - pos, nodes[0]->pos - pos) < 0.0f){
		(*n)--;
		for(i = 0; i < *n; i++)
			nodes[i] = nodes[i+1];
	}
}

void
CPathFind::SetLinksBridgeLights(float x1, float x2, float y1, float y2, bool enable)
{
	int i;
	for(i = 0; i < m_numCarPathLinks; i++)
		if(x1 < m_carPathLinks[i].posX && m_carPathLinks[i].posX < x2 &&
		   y1 < m_carPathLinks[i].posY && m_carPathLinks[i].posY < y2)
			m_carPathLinks[i].bBridgeLights = enable;
}

void
CPathFind::SwitchOffNodeAndNeighbours(int32 nodeId, bool disable)
{
	int i, next;

	m_pathNodes[nodeId].bDisabled = disable;
	if(m_pathNodes[nodeId].numLinks < 3)
		for(i = 0; i < m_pathNodes[nodeId].numLinks; i++){
			next = m_connections[m_pathNodes[nodeId].firstLink + i];
			if(m_pathNodes[next].bDisabled != disable &&
			   m_pathNodes[next].numLinks < 3)
				SwitchOffNodeAndNeighbours(next, disable);
		}
}

void
CPathFind::SwitchRoadsOffInArea(float x1, float x2, float y1, float y2, float z1, float z2, bool disable)
{
	int i;

	for(i = 0; i < m_numCarPathNodes; i++)
		if (x1 <= m_pathNodes[i].pos.x && m_pathNodes[i].pos.x <= x2 &&
			y1 <= m_pathNodes[i].pos.y && m_pathNodes[i].pos.y <= y2 &&
			z1 <= m_pathNodes[i].pos.z && m_pathNodes[i].pos.z <= z2 &&
			disable != m_pathNodes[i].bDisabled)
			SwitchOffNodeAndNeighbours(i, disable);
}

void
CPathFind::SwitchPedRoadsOffInArea(float x1, float x2, float y1, float y2, float z1, float z2, bool disable)
{
	int i;

	for(i = m_numCarPathNodes; i < m_numPathNodes; i++)
		if(x1 <= m_pathNodes[i].pos.x && m_pathNodes[i].pos.x <= x2 &&
		   y1 <= m_pathNodes[i].pos.y && m_pathNodes[i].pos.y <= y2 &&
		   z1 <= m_pathNodes[i].pos.z && m_pathNodes[i].pos.z <= z2 &&
			disable != m_pathNodes[i].bDisabled)
			SwitchOffNodeAndNeighbours(i, disable);
}

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
		if(m_pathNodes[i].pos.z < z1 || m_pathNodes[i].pos.z > z2)
			continue;
		CVector2D d(m_pathNodes[i].pos.x - x1, m_pathNodes[i].pos.y - y1);
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

void
CPathFind::MarkRoadsBetweenLevelsNodeAndNeighbours(int32 nodeId)
{
	int i, next;

	m_pathNodes[nodeId].bBetweenLevels = true;
	if(m_pathNodes[nodeId].numLinks < 3)
		for(i = 0; i < m_pathNodes[nodeId].numLinks; i++){
			next = m_connections[m_pathNodes[nodeId].firstLink + i];
			if(!m_pathNodes[next].bBetweenLevels &&
			   m_pathNodes[next].numLinks < 3)
				MarkRoadsBetweenLevelsNodeAndNeighbours(next);
		}
}

void
CPathFind::MarkRoadsBetweenLevelsInArea(float x1, float x2, float y1, float y2, float z1, float z2)
{
	int i;

	for(i = 0; i < m_numPathNodes; i++)
		if(x1 < m_pathNodes[i].pos.x && m_pathNodes[i].pos.x < x2 &&
		   y1 < m_pathNodes[i].pos.y && m_pathNodes[i].pos.y < y2 &&
		   z1 < m_pathNodes[i].pos.z && m_pathNodes[i].pos.z < z2)
			MarkRoadsBetweenLevelsNodeAndNeighbours(i);
}

void
CPathFind::PedMarkRoadsBetweenLevelsInArea(float x1, float x2, float y1, float y2, float z1, float z2)
{
	int i;

	for(i = m_numCarPathNodes; i < m_numPathNodes; i++)
		if(x1 < m_pathNodes[i].pos.x && m_pathNodes[i].pos.x < x2 &&
		   y1 < m_pathNodes[i].pos.y && m_pathNodes[i].pos.y < y2 &&
		   z1 < m_pathNodes[i].pos.z && m_pathNodes[i].pos.z < z2)
			MarkRoadsBetweenLevelsNodeAndNeighbours(i);
}

int32
CPathFind::FindNodeClosestToCoors(CVector coors, uint8 type, float distLimit, bool ignoreDisabled, bool ignoreBetweenLevels)
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
		switch(m_pathNodes[i].unkBits){
		case 1:
		case 2:
			dist = Abs(m_pathNodes[i].pos.x - coors.x) +
			       Abs(m_pathNodes[i].pos.y - coors.y) +
			       3.0f*Abs(m_pathNodes[i].pos.z - coors.z);
			if(dist < closestDist){
				closestDist = dist;
				closestNode = i;
			}
			break;
		}
	}
	return closestDist < distLimit ? closestNode : -1;
}

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
		switch(m_pathNodes[i].unkBits){
		case 1:
		case 2:
			dX = m_pathNodes[i].pos.x - coors.x;
			dY = m_pathNodes[i].pos.y - coors.y;
			dist = Abs(dX) + Abs(dY) +
			       3.0f*Abs(m_pathNodes[i].pos.z - coors.z);
			if(dist < closestDist){
				NormalizeXY(dX, dY);
				dist -= (dX*dirX + dY*dirY - 1.0f)*20.0f;
				if(dist < closestDist){
					closestDist = dist;
					closestNode = i;
				}
			}
			break;
		}
	}
	return closestNode;
}


float
CPathFind::FindNodeOrientationForCarPlacement(int32 nodeId)
{
	if(m_pathNodes[nodeId].numLinks == 0)
		return 0.0f;
	CVector dir = m_pathNodes[m_connections[m_pathNodes[nodeId].firstLink]].pos - m_pathNodes[nodeId].pos;
	dir.z = 0.0f;
	dir.Normalise();
	return RADTODEG(dir.Heading());
}

float
CPathFind::FindNodeOrientationForCarPlacementFacingDestination(int32 nodeId, float x, float y, bool towards)
{
	int i;

	CVector targetDir(x - m_pathNodes[nodeId].pos.x, y - m_pathNodes[nodeId].pos.y, 0.0f);
	targetDir.Normalise();
	CVector dir;

	if(m_pathNodes[nodeId].numLinks == 0)
		return 0.0f;

	int bestNode = m_connections[m_pathNodes[nodeId].firstLink];
#ifdef FIX_BUGS
	float bestDot = towards ? -2.0f : 2.0f;
#else
	int bestDot = towards ? -2 : 2;	// why int?
#endif

	for(i = 0; i < m_pathNodes[nodeId].numLinks; i++){
		dir = m_pathNodes[m_connections[m_pathNodes[nodeId].firstLink + i]].pos - m_pathNodes[nodeId].pos;
		dir.z = 0.0f;
		dir.Normalise();
		float angle = DotProduct2D(dir, targetDir);
		if(towards){
			if(angle > bestDot){
				bestDot = angle;
				bestNode = m_connections[m_pathNodes[nodeId].firstLink + i];
			}
		}else{
			if(angle < bestDot){
				bestDot = angle;
				bestNode = m_connections[m_pathNodes[nodeId].firstLink + i];
			}
		}
	}

	dir = m_pathNodes[bestNode].pos - m_pathNodes[nodeId].pos;
	dir.z = 0.0f;
	dir.Normalise();
	return RADTODEG(dir.Heading());
}

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
		dist1 = Distance2D(m_pathNodes[node1].pos, x, y);
		if(dist1 < spawnDist + 60.0f){
			d1 = dist1 - spawnDist;
			for(j = 0; j < m_pathNodes[node1].numLinks; j++){
				node2 = m_connections[m_pathNodes[node1].firstLink + j];
				if(m_pathNodes[node2].bDisabled && !ignoreDisabled)
					continue;
				dist2 = Distance2D(m_pathNodes[node2].pos, x, y);
				d2 = dist2 - spawnDist;
				if(d1*d2 < 0.0f){
					// nodes are on different sides of spawn distance
					float f2 = Abs(d1)/(Abs(d1) + Abs(d2));
					float f1 = 1.0f - f2;
					*pPositionBetweenNodes = f2;
					CVector pos = m_pathNodes[node1].pos*f1 + m_pathNodes[node2].pos*f2;
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

bool
CPathFind::GeneratePedCreationCoors(float x, float y, float minDist, float maxDist, float minDistOffScreen, float maxDistOffScreen, CVector *pPosition, int32 *pNode1, int32 *pNode2, float *pPositionBetweenNodes, CMatrix *camMatrix)
{
	int i;
	int node1, node2;

	if(m_numPedPathNodes == 0)
		return false;

	for(i = 0; i < 400; i++){
		node1 = m_numCarPathNodes + CGeneral::GetRandomNumber() % m_numPedPathNodes;
		if(DistanceSqr2D(m_pathNodes[node1].pos, x, y) < sq(maxDist+30.0f)){
			if(m_pathNodes[node1].numLinks == 0)
				continue;
			int link = m_pathNodes[node1].firstLink + CGeneral::GetRandomNumber() % m_pathNodes[node1].numLinks;
			if(m_connectionFlags[link].bCrossesRoad)
				continue;
			node2 = m_connections[link];
			if(m_pathNodes[node1].bDisabled || m_pathNodes[node2].bDisabled)
				continue;

			float f2 = (CGeneral::GetRandomNumber()&0xFF)/256.0f;
			float f1 = 1.0f - f2;
			*pPositionBetweenNodes = f2;
			CVector pos = m_pathNodes[node1].pos*f1 + m_pathNodes[node2].pos*f2;
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

CTreadable*
CPathFind::FindRoadObjectClosestToCoors(CVector coors, uint8 type)
{
	int i, j, k;
	int node1, node2;
	CTreadable *closestMapObj = nil;
	float closestDist = 10000.0f;

	for(i = 0; i < m_numMapObjects; i++){
		CTreadable *mapObj = m_mapObjects[i];
		if(mapObj->m_nodeIndices[type][0] < 0)
			continue;
		CVector vDist = mapObj->GetPosition() - coors;
		float fDist = Abs(vDist.x) + Abs(vDist.y) + Abs(vDist.z);
		if(fDist < 200.0f || fDist < closestDist)
			for(j = 0; j < 12; j++){
				node1 = mapObj->m_nodeIndices[type][j];
				if(node1 < 0)
					break;
				// FIX: game uses ThePaths here explicitly
				for(k = 0; k < m_pathNodes[node1].numLinks; k++){
					node2 = m_connections[m_pathNodes[node1].firstLink + k];
					float lineDist = CCollision::DistToLine(&m_pathNodes[node1].pos, &m_pathNodes[node2].pos, &coors);
					if(lineDist < closestDist){
						closestDist = lineDist;
						if((coors - m_pathNodes[node1].pos).MagnitudeSqr() < (coors - m_pathNodes[node2].pos).MagnitudeSqr())
							closestMapObj = m_mapObjects[m_pathNodes[node1].objectIndex];
						else
							closestMapObj = m_mapObjects[m_pathNodes[node2].objectIndex];
					}
				}
			}
	}
	return closestMapObj;
}

void
CPathFind::FindNextNodeWandering(uint8 type, CVector coors, CPathNode **lastNode, CPathNode **nextNode, uint8 curDir, uint8 *nextDir)
{
	int i;
	CPathNode *node;

	if(lastNode == nil || (node = *lastNode) == nil || (coors - (*lastNode)->pos).MagnitudeSqr() > 7.0f){
		// need to find the node we're coming from
		node = nil;
		CTreadable *obj = FindRoadObjectClosestToCoors(coors, type);
		float nodeDist = 1000000000.0f;
		for(i = 0; i < 12; i++){
			if(obj->m_nodeIndices[type][i] < 0)
				break;
			float dist = (coors - m_pathNodes[obj->m_nodeIndices[type][i]].pos).MagnitudeSqr();
			if(dist < nodeDist){
				nodeDist = dist;
				node = &m_pathNodes[obj->m_nodeIndices[type][i]];
			}
		}
	}

	CVector2D vCurDir(Sin(curDir*PI/4.0f), Cos(curDir * PI / 4.0f));
	*nextNode = 0;
	float bestDot = -999999.0f;
	for(i = 0; i < node->numLinks; i++){
		int next = m_connections[node->firstLink+i];
		if(!node->bDisabled && m_pathNodes[next].bDisabled)
			continue;
		CVector pedCoors = coors;
		pedCoors.z += 1.0f;
		CVector nodeCoors = m_pathNodes[next].pos;
		nodeCoors.z += 1.0f;
		if(!CWorld::GetIsLineOfSightClear(pedCoors, nodeCoors, true, false, false, false, false, false))
			continue;
		CVector2D nodeDir = m_pathNodes[next].pos - node->pos;
		nodeDir.Normalise();
		float dot = DotProduct2D(nodeDir, vCurDir);
		if(dot >= bestDot){
			*nextNode = &m_pathNodes[next];
			bestDot = dot;

			// direction is 0, 2, 4, 6 for north, east, south, west
			// this could be sone simpler...
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

static CPathNode *apNodesToBeCleared[4995];

void
CPathFind::DoPathSearch(uint8 type, CVector start, int32 startNodeId, CVector target, CPathNode **nodes, int16 *pNumNodes, int16 maxNumNodes, CVehicle *vehicle, float *pDist, float distLimit, int32 forcedTargetNode)
{
	int i, j;

	// Find target
	int targetNode;
	if(forcedTargetNode < 0)
		targetNode = FindNodeClosestToCoors(target, type, distLimit);
	else
		targetNode = forcedTargetNode;
	if(targetNode < 0)
		goto fail;

	// Find start
	int numPathsToTry;
	CTreadable *startObj;
	if(startNodeId < 0){
		if(vehicle == nil || (startObj = vehicle->m_treadable[type]) == nil)
			startObj = FindRoadObjectClosestToCoors(start, type);
		numPathsToTry = 0;
		for(i = 0; i < 12; i++){
			if(startObj->m_nodeIndices[type][i] < 0)
				break;
			if(m_pathNodes[startObj->m_nodeIndices[type][i]].group == m_pathNodes[targetNode].group)
				numPathsToTry++;
		}
	}else{
		numPathsToTry = 1;
		startObj = m_mapObjects[m_pathNodes[startNodeId].objectIndex];
	}
	if(numPathsToTry == 0)
		goto fail;

	if(startNodeId < 0){
		// why only check node 0?
		if(m_pathNodes[startObj->m_nodeIndices[type][0]].group != m_pathNodes[targetNode].group)
			goto fail;
	}else{
		if(m_pathNodes[startNodeId].group != m_pathNodes[targetNode].group)
			goto fail;
	}


	for(i = 0; i < 512; i++)
		m_searchNodes[i].next = nil;
	AddNodeToList(&m_pathNodes[targetNode], 0);
	int numNodesToBeCleared = 0;
	apNodesToBeCleared[numNodesToBeCleared++] = &m_pathNodes[targetNode];

	// Dijkstra's algorithm
	// Find distances
	int numPathsFound = 0;
	if(startNodeId < 0 && m_mapObjects[m_pathNodes[targetNode].objectIndex] == startObj)
		numPathsFound++;
	for(i = 0; numPathsFound < numPathsToTry; i = (i+1) & 0x1FF){
		CPathNode *node;
		for(node = m_searchNodes[i].next; node; node = node->next){
			if(m_mapObjects[node->objectIndex] == startObj &&
			   (startNodeId < 0 || node == &m_pathNodes[startNodeId]))
				numPathsFound++;

			for(j = 0; j < node->numLinks; j++){
				int next = m_connections[node->firstLink + j];
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
	if(startNodeId < 0){
		int minDist = MAX_DIST;
		*pNumNodes = 1;
		for(i = 0; i < 12; i++){
			if(startObj->m_nodeIndices[type][i] < 0)
				break;
			int dist = (m_pathNodes[startObj->m_nodeIndices[type][i]].pos - start).Magnitude();
			if(m_pathNodes[startObj->m_nodeIndices[type][i]].distance + dist < minDist){
				minDist = m_pathNodes[startObj->m_nodeIndices[type][i]].distance + dist;
				curNode = &m_pathNodes[startObj->m_nodeIndices[type][i]];
			}
		}
		if(maxNumNodes == 0){
			*pNumNodes = 0;
		}else{
			nodes[0] = curNode;
			*pNumNodes = 1;
		}
		if(pDist)
			*pDist = minDist;
	}else{
		curNode = &m_pathNodes[startNodeId];
		*pNumNodes = 0;
		if(pDist)
			*pDist = m_pathNodes[startNodeId].distance;
	}

	// Trace back to target and update list of nodes
	while(*pNumNodes < maxNumNodes && curNode != &m_pathNodes[targetNode])
		for(i = 0; i < curNode->numLinks; i++){
			int next = m_connections[curNode->firstLink + i];
			if(curNode->distance - m_distances[curNode->firstLink + i] == m_pathNodes[next].distance){
				curNode = &m_pathNodes[next];
				nodes[(*pNumNodes)++] = curNode;
				i = 29030;	// could have used a break...
			}
		}

	for(i = 0; i < numNodesToBeCleared; i++)
		apNodesToBeCleared[i]->distance = MAX_DIST;
	return;

fail:
	*pNumNodes = 0;
	if(pDist)
		*pDist = 100000.0f;
}

static CPathNode *pNodeList[32];
static int16 DummyResult;
static int16 DummyResult2;

bool
CPathFind::TestCoorsCloseness(CVector target, uint8 type, CVector start)
{
	float dist;
	if(type == PATH_CAR)
		DoPathSearch(type, start, -1, target, pNodeList, &DummyResult, 32, nil, &dist, 999999.88f, -1);
	else
		DoPathSearch(type, start, -1, target, nil, &DummyResult2, 0, nil, &dist, 50.0f, -1);
	if(type == PATH_CAR)
		return dist < 160.0f;
	else
		return dist < 100.0f;
}

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

STARTPATCHES
	InjectHook(0x4294A0, &CPathFind::Init, PATCH_JUMP);
	InjectHook(0x42D580, &CPathFind::AllocatePathFindInfoMem, PATCH_JUMP);
	InjectHook(0x429540, &CPathFind::RegisterMapObject, PATCH_JUMP);
	InjectHook(0x42D7E0, &CPathFind::StoreNodeInfoPed, PATCH_JUMP);
	InjectHook(0x42D690, &CPathFind::StoreNodeInfoCar, PATCH_JUMP);
	InjectHook(0x429610, &CPathFind::PreparePathData, PATCH_JUMP);
	InjectHook(0x42B810, &CPathFind::CountFloodFillGroups, PATCH_JUMP);
	InjectHook(0x429C20, &CPathFind::PreparePathDataForType, PATCH_JUMP);

	InjectHook(0x42C990, &CPathFind::CalcRoadDensity, PATCH_JUMP);
	InjectHook(0x42E1B0, &CPathFind::TestForPedTrafficLight, PATCH_JUMP);
	InjectHook(0x42E340, &CPathFind::TestCrossesRoad, PATCH_JUMP);
	InjectHook(0x42CBE0, &CPathFind::AddNodeToList, PATCH_JUMP);
	InjectHook(0x42CBB0, &CPathFind::RemoveNodeFromList, PATCH_JUMP);
	InjectHook(0x42B790, &CPathFind::RemoveBadStartNode, PATCH_JUMP);
	InjectHook(0x42E3B0, &CPathFind::SetLinksBridgeLights, PATCH_JUMP);
	InjectHook(0x42DED0, &CPathFind::SwitchOffNodeAndNeighbours, PATCH_JUMP);
	InjectHook(0x42D960, &CPathFind::SwitchRoadsOffInArea, PATCH_JUMP);
	InjectHook(0x42DA50, &CPathFind::SwitchPedRoadsOffInArea, PATCH_JUMP);
	InjectHook(0x42DB50, &CPathFind::SwitchRoadsInAngledArea, PATCH_JUMP);
	InjectHook(0x42E140, &CPathFind::MarkRoadsBetweenLevelsNodeAndNeighbours, PATCH_JUMP);
	InjectHook(0x42DF50, &CPathFind::MarkRoadsBetweenLevelsInArea, PATCH_JUMP);
	InjectHook(0x42E040, &CPathFind::PedMarkRoadsBetweenLevelsInArea, PATCH_JUMP);
	InjectHook(0x42CC30, &CPathFind::FindNodeClosestToCoors, PATCH_JUMP);
	InjectHook(0x42CDC0, &CPathFind::FindNodeClosestToCoorsFavourDirection, PATCH_JUMP);
	InjectHook(0x42CFC0, &CPathFind::FindNodeOrientationForCarPlacement, PATCH_JUMP);
	InjectHook(0x42D060, &CPathFind::FindNodeOrientationForCarPlacementFacingDestination, PATCH_JUMP);
	InjectHook(0x42BF10, &CPathFind::NewGenerateCarCreationCoors, PATCH_JUMP);
	InjectHook(0x42C1E0, &CPathFind::GeneratePedCreationCoors, PATCH_JUMP);
	InjectHook(0x42D2A0, &CPathFind::FindRoadObjectClosestToCoors, PATCH_JUMP);
	InjectHook(0x42B9F0, &CPathFind::FindNextNodeWandering, PATCH_JUMP);
	InjectHook(0x42B040, &CPathFind::DoPathSearch, PATCH_JUMP);
	InjectHook(0x42C8C0, &CPathFind::TestCoorsCloseness, PATCH_JUMP);
	InjectHook(0x42E450, &CPathFind::Save, PATCH_JUMP);
	InjectHook(0x42E550, &CPathFind::Load, PATCH_JUMP);
ENDPATCHES
