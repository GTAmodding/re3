#pragma once

#include "Vehicle.h"
#include "Door.h"

enum
{
	TRACK_ELTRAIN,
	TRACK_SUBWAY
};

enum
{
	TRAIN_DOOR_CLOSED,
	TRAIN_DOOR_OPENING,
	TRAIN_DOOR_OPEN,
	TRAIN_DOOR_CLOSING
};

enum eTrainNodes
{
	TRAIN_DOOR_LHS = 1,
	TRAIN_DOOR_RHS,
	NUM_TRAIN_NODES
};

struct CTrainNode
{
	CVector p;	// position
	float t;	// xy-distance from start on track
};

struct CTrainInterpolationLine
{
	uint8 type;
	float time;	// when does this keyframe start
	// initial values at start of frame
	float position;
	float speed;
	float acceleration;
};

class CTrain : public CVehicle
{
public:
	// 0x288
	float m_fWagonPosition;
	int16 m_nWagonId;
	int16 m_isFarAway;	// don't update so often?
	int16 m_nCurTrackNode;
	int16 m_nWagonGroup;
	float m_fSpeed;
	bool m_bProcessDoor;
	bool m_bTrainStopping;
	bool m_bIsFirstWagon;
	bool m_bIsLastWagon;
	uint8 m_nTrackId;	// or m_bUsesSubwayTracks?
	uint32 m_nDoorTimer;
	int16 m_nDoorState;
	CTrainDoor Doors[2];
	RwFrame *m_aTrainNodes[NUM_TRAIN_NODES];

	// unused
	static CVector aStationCoors[3];
	static CVector aStationCoors_S[4];

	CTrain(int32 id, uint8 CreatedBy);

	// from CEntity
	void SetModelIndex(uint32 id);
	void ProcessControl(void);
	void PreRender(void);
	void Render(void);

	void AddPassenger(CPed *ped);
	void OpenTrainDoor(float ratio);
	void TrainHitStuff(CPtrList &list);

	static void InitTrains(void);
	static void Shutdown(void);
	static void ReadAndInterpretTrackFile(Const char *filename, CTrainNode **nodes, int16 *numNodes, int32 numStations, float *stationDists,
		float *totalLength, float *totalDuration, CTrainInterpolationLine *interpLines, bool rightRail);
	static void UpdateTrains(void);
};
