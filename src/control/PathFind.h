#pragma once

#include "Treadable.h"

class CVehicle;
class CPtrList;

#define LANE_WIDTH 5.0f

enum
{
	NodeTypeExtern = 1,
	NodeTypeIntern = 2,
};

enum
{
	PATH_CAR = 0,
	PATH_PED = 1,
};

enum
{
	SWITCH_OFF = 0,
	SWITCH_ON = 1,
};

enum 
{
	ROUTE_ADD_BLOCKADE = 0,
	ROUTE_NO_BLOCKADE = 1
};

struct CPedPathNode
{
	bool bBlockade;
	uint8 nodeIdX;
	uint8 nodeIdY;
	int16 id;
	CPedPathNode* prev;
	CPedPathNode* next;
};

VALIDATE_SIZE(CPedPathNode, 0x10);

class CPedPath {
public:
	static bool CalcPedRoute(int8 pathType, CVector position, CVector destination, CVector *pointPoses, int16 *pointsFound, int16 maxPoints);
	static void AddNodeToPathList(CPedPathNode *pNodeToAdd, int16 id, CPedPathNode *pNodeList);
	static void RemoveNodeFromList(CPedPathNode *pNode);
	static void AddNodeToList(CPedPathNode *pNode, int16 index, CPedPathNode *pList);
	static void AddBlockade(CEntity *pEntity, CPedPathNode(*pathNodes)[40], CVector *pPosition);
	static void AddBlockadeSectorList(CPtrList& list, CPedPathNode(*pathNodes)[40], CVector *pPosition);
};

struct CPathNode
{
	int16 prevIndex;
	int16 nextIndex;
	int16 x;
	int16 y;
	int16 z;
	int16 distance;		// in path search
	int16 firstLink;
	uint8 width;
	int8 group;

	uint8 numLinks : 4;
	uint8 bDeadEnd : 1;
	uint8 bDisabled : 1;
	uint8 bBetweenLevels : 1;
	uint8 bUseInRoadBlock : 1;

	uint8 bWaterPath : 1;
	uint8 bOnlySmallBoats : 1;
	uint8 flagB4 : 1;	// where is this set?
	uint8 speedLimit : 2;
	//uint8 flagB20 : 1;
	//uint8 flagB40 : 1;
	//uint8 flagB80 : 1;

	uint8 spawnRate : 4;
	uint8 flagsC : 4;

	CVector GetPosition(void) { return CVector(x/8.0f, y/8.0f, z/8.0f); }
	void SetPosition(const CVector &p) { x = p.x*8.0f; y = p.y*8.0f; z = p.z*8.0f; }
	float GetX(void) { return x/8.0f; }
	float GetY(void) { return y/8.0f; }
	float GetZ(void) { return z/8.0f; }
	bool HasDivider(void) { return width != 0; }
	float GetDividerWidth(void) { return width/(2*8.0f); }
	float GetPedNodeWidth(void) { return width*(31.f/(500.f * 8.f)); }
	CPathNode *GetPrev(void);
	CPathNode *GetNext(void);
	void SetPrev(CPathNode *node);
	void SetNext(CPathNode *node);
};

union CConnectionFlags
{
	uint8 flags;
	struct {
		uint8 bCrossesRoad : 1;
		uint8 bTrafficLight : 1;
	};
};

struct CCarPathLink
{
	int16 x;
	int16 y;
	int16 pathNodeIndex;
	int8 dirX;
	int8 dirY;
	int8 numLeftLanes : 3;
	int8 numRightLanes : 3;
	uint8 flag1 : 1;
	uint8 trafficLightType : 2;
	uint8 bBridgeLights : 1;	// at least in LCS...
	int8 width;

	CVector2D GetPosition(void) { return CVector2D(x/8.0f, y/8.0f); }
	CVector2D GetDirection(void) { return CVector2D(dirX/100.0f, dirY/100.0f); }
	float GetX(void) { return x/8.0f; }
	float GetY(void) { return y/8.0f; }
	float GetDirX(void) { return dirX/100.0f; }
	float GetDirY(void) { return dirY/100.0f; }
	float GetLaneOffset(void) { return width/(2*8.0f*LANE_WIDTH); }

	float OneWayLaneOffset()
	{
		if (numLeftLanes == 0)
			return 0.5f - 0.5f * numRightLanes;
		if (numRightLanes == 0)
			return 0.5f - 0.5f * numLeftLanes;
		return 0.5f + GetLaneOffset();
	}
};

// This is what we're reading from the files, only temporary
struct CPathInfoForObject
{
	float x;
	float y;
	float z;
	int8 type;
	int8 next;
	int8 numLeftLanes;
	int8 numRightLanes;
	int8 speedLimit;
	int8 width;

	uint8 crossing : 1;
	uint8 onlySmallBoats : 1;
	uint8 roadBlock : 1;
	uint8 disabled : 1;
	uint8 waterPath : 1;
	uint8 betweenLevels : 1;

	uint8 spawnRate : 4;

	void SwapConnectionsToBeRightWayRound(void);
};
extern CPathInfoForObject *InfoForTileCars;
extern CPathInfoForObject *InfoForTilePeds;

struct CTempNode
{
	CVector pos;
	int8 dirX;	// *100
	int8 dirY;
	int16 link1;
	int16 link2;
	int8 numLeftLanes;
	int8 numRightLanes;
	int8 width;
	bool isCross;
	int8 linkState;
};

struct CTempNodeExternal	// made up name
{
	CVector pos;
	int16 next;
	int8 numLeftLanes;
	int8 numRightLanes;
	int8 width;
	bool isCross;
};

class CPathFind
{
public:
	CPathNode m_pathNodes[NUM_PATHNODES];
	CCarPathLink m_carPathLinks[NUM_CARPATHLINKS];
	CTreadable *m_mapObjects[NUM_MAPOBJECTS];
	uint16 m_connections[NUM_PATHCONNECTIONS];	// and flags
	uint8 m_distances[NUM_PATHCONNECTIONS];
	int16 m_carPathConnections[NUM_PATHCONNECTIONS];

	int32 m_numPathNodes;
	int32 m_numCarPathNodes;
	int32 m_numPedPathNodes;
	int16 m_numMapObjects;
	int16 m_numConnections;
	int32 m_numCarPathLinks;
	int32 unk;
	uint8 m_numGroups[2];
	CPathNode m_searchNodes[512];

	void Init(void);
	void AllocatePathFindInfoMem(int16 numPathGroups);
	void RegisterMapObject(CTreadable *mapObject);
	void StoreNodeInfoPed(int16 id, int16 node, int8 type, int8 next, int16 x, int16 y, int16 z, float width, bool crossing, uint8 spawnRate);
	void StoreNodeInfoCar(int16 id, int16 node, int8 type, int8 next, int16 x, int16 y, int16 z, float width, int8 numLeft, int8 numRight,
		bool disabled, bool betweenLevels, uint8 speedLimit, bool roadBlock, bool waterPath, uint8 spawnRate);
	void StoreDetachedNodeInfoPed(int32 node, int8 type, int32 next, float x, float y, float z, float width, bool crossing,
		bool disabled, bool betweenLevels, uint8 spawnRate);
	void StoreDetachedNodeInfoCar(int32 node, int8 type, int32 next, float x, float y, float z, float width, int8 numLeft, int8 numRight,
		bool disabled, bool betweenLevels, uint8 speedLimit, bool roadBlock, bool waterPath, uint8 spawnRate, bool unk);
	void CalcNodeCoors(float x, float y, float z, int32 id, CVector *out);
	bool LoadPathFindData(void);
	void PreparePathData(void);
	void CountFloodFillGroups(uint8 type);
	void PreparePathDataForType(uint8 type, CTempNode *tempnodes, CPathInfoForObject *objectpathinfo,
		float maxdist, CPathInfoForObject *detachednodes, int32 numDetached);

	bool IsPathObject(int id) { return id < PATHNODESIZE && (InfoForTileCars[id*12].type != 0 || InfoForTilePeds[id*12].type != 0); }

	float CalcRoadDensity(float x, float y);
	bool TestForPedTrafficLight(CPathNode *n1, CPathNode *n2);
	bool TestCrossesRoad(CPathNode *n1, CPathNode *n2);
	void AddNodeToList(CPathNode *node, int32 listId);
	void RemoveNodeFromList(CPathNode *node);
	void RemoveBadStartNode(CVector pos, CPathNode **nodes, int16 *n);
	void SetLinksBridgeLights(float, float, float, float, bool);
	void SwitchOffNodeAndNeighbours(int32 nodeId, bool disable);
	void SwitchRoadsOffInArea(float x1, float x2, float y1, float y2, float z1, float z2, bool disable);
	void SwitchPedRoadsOffInArea(float x1, float x2, float y1, float y2, float z1, float z2, bool disable);
	void SwitchRoadsInAngledArea(float x1, float y1, float z1, float x2, float y2, float z2, float length, uint8 type, uint8 enable);
	void MarkRoadsBetweenLevelsNodeAndNeighbours(int32 nodeId);
	void MarkRoadsBetweenLevelsInArea(float x1, float x2, float y1, float y2, float z1, float z2);
	void PedMarkRoadsBetweenLevelsInArea(float x1, float x2, float y1, float y2, float z1, float z2);
// TODO(MIAMI): check callers for new arguments
	int32 FindNodeClosestToCoors(CVector coors, uint8 type, float distLimit, bool ignoreDisabled = false, bool ignoreBetweenLevels = false, bool ignoreFlagB4 = false, bool bWaterPath = false);
	int32 FindNodeClosestToCoorsFavourDirection(CVector coors, uint8 type, float dirX, float dirY);
	float FindNodeOrientationForCarPlacement(int32 nodeId);
	float FindNodeOrientationForCarPlacementFacingDestination(int32 nodeId, float x, float y, bool towards);
	bool NewGenerateCarCreationCoors(float x, float y, float dirX, float dirY, float spawnDist, float angleLimit, bool forward, CVector *pPosition, int32 *pNode1, int32 *pNode2, float *pPositionBetweenNodes, bool ignoreDisabled = false);
	bool GeneratePedCreationCoors(float x, float y, float minDist, float maxDist, float minDistOffScreen, float maxDistOffScreen, CVector *pPosition, int32 *pNode1, int32 *pNode2, float *pPositionBetweenNodes, CMatrix *camMatrix);
	void FindNextNodeWandering(uint8, CVector, CPathNode**, CPathNode**, uint8, uint8*);
	void DoPathSearch(uint8 type, CVector start, int32 startNodeId, CVector target, CPathNode **nodes, int16 *numNodes, int16 maxNumNodes, CVehicle *vehicle, float *dist, float distLimit, int32 forcedTargetNode);
	bool TestCoorsCloseness(CVector target, uint8 type, CVector start);
	void Save(uint8 *buf, uint32 *size);
	void Load(uint8 *buf, uint32 size);

	static CVector TakeWidthIntoAccountForWandering(CPathNode*, uint16);

	CPathNode *GetNode(int16 index);
	int16 GetIndex(CPathNode *node);

	uint16 ConnectedNode(int id) { return m_connections[id] & 0x3FFF; }
	bool ConnectionCrossesRoad(int id) { return !!(m_connections[id] & 0x8000); }
	bool ConnectionHasTrafficLight(int id) { return !!(m_connections[id] & 0x4000); }
	void ConnectionSetTrafficLight(int id) { m_connections[id] |= 0x4000; }

	void DisplayPathData(void);
};

extern CPathFind ThePaths;

inline CPathNode *CPathNode::GetPrev(void) { return ThePaths.GetNode(prevIndex); }
inline CPathNode *CPathNode::GetNext(void) { return ThePaths.GetNode(nextIndex); }
inline void CPathNode::SetPrev(CPathNode *node) { prevIndex = ThePaths.GetIndex(node); }
inline void CPathNode::SetNext(CPathNode *node) { nextIndex = ThePaths.GetIndex(node); }

extern bool gbShowPedPaths;
extern bool gbShowCarPaths;
extern bool gbShowCarPathsLinks;
