#pragma once

#include "Vehicle.h"
#include "Door.h"

enum eFerryNodes
{
	FERRY_DOOR_FRONT = 1,
	FERRY_RAMP_FRONT,
	FERRY_DOOR_BACK,
	FERRY_RAMP_BACK,
	NUM_FERRY_NODES
};

enum {
	NUM_FERRY_STATIONS = 2,
	NUM_FERRIES = NUM_FERRY_PATHS * 2,
	NUM_WAKE_POINTS = 64
};

enum {
	FERRY_STOPPED = 0,
	FERRY_CRUISING,
	FERRY_SLOWING,
	FERRY_ACCELERATING
};

enum
{
	FERRY_DOOR_CLOSED = 0,
	FERRY_DOOR_OPENING,
	FERRY_DOOR_OPEN,
	FERRY_DOOR_CLOSING
};

struct CFerryNode
{
	float x;
	float y;
	float z;
	float t;
};

struct CFerryInterpolationLine
{
	uint8 type;
	float time;	// when does this keyframe start
	// initial values at start of frame
	float position;
	float speed;
	float acceleration;
};

struct CFerryPath
{
	float TotalLengthOfTrack;
	float TotalDurationOfTrack;
	int16 NumTrackNodes;
	CFerryNode* aTrackNodes;
	CFerryInterpolationLine* aLineBits;
};

class CFerry;

class CFerryInst
{
public:
	CFerryPath* pPathData[NUM_FERRY_PATHS];
	float m_afPositions[NUM_FERRIES];
	float m_afSpeeds[NUM_FERRIES];
	CFerry* m_apFerries[NUM_FERRIES];
	bool m_bFerriesDisabled;
	uint16 m_anNumWakePoints[NUM_FERRIES];
	CVector2D m_avWakePoints[NUM_FERRIES][NUM_WAKE_POINTS];
	float m_afWakePointTimer[NUM_FERRIES][NUM_WAKE_POINTS];
};

class CFerry : public CVehicle
{
public:
	uint16 m_nFerryId;
	uint16 m_isFarAway;
	uint16 m_nCurTrackNode;
	float m_fSpeed;
	bool m_bFerryDocked;
	uint32 m_nDoorTimer;
	uint32 m_nTimeLeftStation;
	int16 m_nDoorState;
	bool m_bApproachingDock;
	uint8 m_nSkipFerryStatus;
	uint32 m_nTimeAlongPath;
	bool m_bUseFrontDoor;
	CVector m_vecForwardSpeed;
	CColModel* m_pDefaultColModel;
	uint8 m_nCollision;
	CDoor m_doors[4];
	RwFrame* m_aFerryNodes[NUM_FERRY_NODES];
	bool m_bAlreadyRendered;
	bool m_bPlayerArrivedHorn;

	static CFerryInst* mspInst;

	enum eSpaceUse {
		FERRY_SPACE_PED = 0,
		FERRY_SPACE_CAR
	};
	enum eSpaceStyle {
		FERRY_SPACE_STYLE_0 = 0,
		FERRY_SPACE_STYLE_1
	};
	void Render(void);
	static void EnableFerryPath(int);
	CFerry(int32, uint8);
	void SetModelIndex(uint32);
	static void InitFerrys(void);
	static void Init(void*);
	void ProcessControl(void);
	void PlayArrivedHorn(void);
	void AddWakePoint(CVector);
	void PruneWakeTrail(void);
	void SkipFerryToNextDock(void);
	static void DissableFerryPath(int);
	void CompleteDorrMovement(void);
	bool IsDoorOpen(void);
	void CloseDoor(void);
	bool IsDocked(void);
	static CFerry* GetClosestFerry(float, float);
	CVector GetBoardingSpace(CFerry::eSpaceUse, CFerry::eSpaceStyle, uint8);
	CVector GetNearestDoor(CVector);
	void OpenFerryDoor(float);
	void FerryHitStuff(CPtrList&);
	static void RenderAllRemaning(void);
	static void UpdateFerrys(void);
	static void SwitchFerryCollision(int);
	void SetupForMultiplayer(void);
	void Write(base::cRelocatableChunkWriter&);
	virtual void OpenDoor(void);
	void PreRender(void);
	virtual bool IsDoorClosed(void);

	static CFerry* GetFerry(int i) { return mspInst ? mspInst->m_apFerries[i] : nil; }
	static void SetFerriesDisabled(bool disabled) { mspInst->m_bFerriesDisabled = disabled; }
};
