#pragma once

class COccluder
{
public:
	int16 width, length, height;
	int16 x, y, z;
	uint16 angle;
	int16 listIndex;
};

class COcclusion
{
public:
	static int32 NumOccludersOnMap;
	static int16 FarAwayList;
	static int16 NearbyList;
	static int16 ListWalkThroughFA;
	static int16 PreviousListWalkThroughFA;

	static COccluder aOccluders[NUMOCCLUSIONVOLUMES];

	static void Init(void);
	static void AddOne(float x, float y, float z, float width, float length, float height, float angle);
	static void ProcessBeforeRendering(void);
	
	//TODO:
	static bool IsAABoxOccluded(CVector pos, float, float, float) { return false; }
};
