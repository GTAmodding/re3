#include "common.h"
#include "main.h"

#include "Timer.h"
#include "ModelIndices.h"
#include "FileMgr.h"
#include "Streaming.h"
#include "Pad.h"
#include "Camera.h"
#include "Coronas.h"
#include "World.h"
#include "Ped.h"
#include "DMAudio.h"
#include "HandlingMgr.h"
#include "Train.h"
#include "AudioScriptObject.h"

static CTrainNode* pTrackNodes;
static int16 NumTrackNodes;
static float StationDist[3] = { 873.0f, 1522.0f, 2481.0f };
static float TotalLengthOfTrack;
static float TotalDurationOfTrack;
static CTrainInterpolationLine aLineBits[17];
static float EngineTrackPosition[2];
static float EngineTrackSpeed[2];

static CTrainNode* pTrackNodes_S;
static int16 NumTrackNodes_S;
static float StationDist_S[4] = { 55.0f, 1388.0f, 2337.0f, 3989.0f };
static float TotalLengthOfTrack_S;
static float TotalDurationOfTrack_S;
static CTrainInterpolationLine aLineBits_S[18];
static float EngineTrackPosition_S[4];
static float EngineTrackSpeed_S[4];

CVector CTrain::aStationCoors[3];
CVector CTrain::aStationCoors_S[4];

static bool bTrainArrivalAnnounced[3] = {false, false, false};

CTrain::CTrain(int32 id, uint8 CreatedBy)
 : CVehicle(CreatedBy)
{
	CVehicleModelInfo *mi = (CVehicleModelInfo*)CModelInfo::GetModelInfo(id);
	m_vehType = VEHICLE_TYPE_TRAIN;
	pHandling = mod_HandlingManager.GetHandlingData((tVehicleType)mi->m_handlingId);
	SetModelIndex(id);

	Doors[0].Init(0.8f, 0.0f, 1, 0);
	Doors[1].Init(-0.8f, 0.0f, 0, 0);

	m_fMass = 100000000.0f;
	m_fTurnMass = 100000000.0f;
	m_fAirResistance = 0.9994f;
	m_fElasticity = 0.05f;

	m_bProcessDoor = true;
	m_bTrainStopping = false;
	m_nTrackId = TRACK_ELTRAIN;
	m_nNumMaxPassengers = 5;
	m_nDoorTimer = CTimer::GetTimeInMilliseconds();
	m_nDoorState = TRAIN_DOOR_CLOSED;

	bUsesCollision = true;
	SetStatus(STATUS_TRAIN_MOVING);

#ifdef FIX_BUGS
	m_isFarAway = true;
#endif
}

void
CTrain::SetModelIndex(uint32 id)
{
	int i;

	CVehicle::SetModelIndex(id);
	for(i = 0; i < NUM_TRAIN_NODES; i++)
		m_aTrainNodes[i] = nil;
	CClumpModelInfo::FillFrameArray(GetClump(), m_aTrainNodes);
}

void
CTrain::ProcessControl(void)
{
	if(gbModelViewer || m_isFarAway && (CTimer::GetFrameCounter() + m_nWagonId) & 0xF)
		return;

	CTrainNode *trackNodes;
	int16 numTrackNodes;
	float totalLengthOfTrack;
	float *engineTrackPosition;
	float *engineTrackSpeed;

	if(m_nTrackId == TRACK_SUBWAY){
		trackNodes = pTrackNodes_S;
		numTrackNodes = NumTrackNodes_S;
		totalLengthOfTrack = TotalLengthOfTrack_S;
		engineTrackPosition = EngineTrackPosition_S;
		engineTrackSpeed = EngineTrackSpeed_S;
	}else{
		trackNodes = pTrackNodes;
		numTrackNodes = NumTrackNodes;
		totalLengthOfTrack = TotalLengthOfTrack;
		engineTrackPosition = EngineTrackPosition;
		engineTrackSpeed = EngineTrackSpeed;
	}

	float trackPositionRear = engineTrackPosition[m_nWagonGroup] - m_fWagonPosition;
	if(trackPositionRear < 0.0f)
		trackPositionRear += totalLengthOfTrack;

	// Advance current node to appropriate position
	float pos1, pos2;
	int nextTrackNode = m_nCurTrackNode + 1;
	pos1 = trackNodes[m_nCurTrackNode].t;
	if(nextTrackNode < numTrackNodes)
		pos2 = trackNodes[nextTrackNode].t;
	else{
		nextTrackNode = 0;
		pos2 = totalLengthOfTrack;
	}
	while(trackPositionRear < pos1 || trackPositionRear > pos2){
		m_nCurTrackNode = (m_nCurTrackNode+1) % numTrackNodes;
		nextTrackNode = m_nCurTrackNode + 1;
		pos1 = trackNodes[m_nCurTrackNode].t;
		if(nextTrackNode < numTrackNodes)
			pos2 = trackNodes[nextTrackNode].t;
		else{
			nextTrackNode = 0;
			pos2 = totalLengthOfTrack;
		}
	}
	float dist = trackNodes[nextTrackNode].t - trackNodes[m_nCurTrackNode].t;
	if(dist < 0.0f)
		dist += totalLengthOfTrack;
	float f = (trackPositionRear - trackNodes[m_nCurTrackNode].t)/dist;
	CVector posRear = (1.0f - f)*trackNodes[m_nCurTrackNode].p + f*trackNodes[nextTrackNode].p;

	// Now same again for the front
	float trackPositionFront = trackPositionRear + 20.0f;
	if(trackPositionFront > totalLengthOfTrack)
		trackPositionFront -= totalLengthOfTrack;
	int curTrackNodeFront = m_nCurTrackNode;
	int nextTrackNodeFront = curTrackNodeFront + 1;
	pos1 = trackNodes[curTrackNodeFront].t;
	if(nextTrackNodeFront < numTrackNodes)
		pos2 = trackNodes[nextTrackNodeFront].t;
	else{
		nextTrackNodeFront = 0;
		pos2 = totalLengthOfTrack;
	}
	while(trackPositionFront < pos1 || trackPositionFront > pos2){
		curTrackNodeFront = (curTrackNodeFront+1) % numTrackNodes;
		nextTrackNodeFront = curTrackNodeFront + 1;
		pos1 = trackNodes[curTrackNodeFront].t;
		if(nextTrackNodeFront < numTrackNodes)
			pos2 = trackNodes[nextTrackNodeFront].t;
		else{
			nextTrackNodeFront = 0;
			pos2 = totalLengthOfTrack;
		}
	}
	dist = trackNodes[nextTrackNodeFront].t - trackNodes[curTrackNodeFront].t;
	if(dist < 0.0f)
		dist += totalLengthOfTrack;
	f = (trackPositionFront - trackNodes[curTrackNodeFront].t)/dist;
	CVector posFront = (1.0f - f)*trackNodes[curTrackNodeFront].p + f*trackNodes[nextTrackNodeFront].p;

	// Now set matrix
	SetPosition((posRear + posFront)/2.0f);
	CVector fwd = posFront - posRear;
	fwd.Normalise();
	CVector right = CrossProduct(fwd, CVector(0.0f, 0.0f, 1.0f));
	right.Normalise();
	CVector up = CrossProduct(right, fwd);
	GetRight() = right;
	GetUp() = up;
	GetForward() = fwd;

	// Set speed
	m_vecMoveSpeed = fwd*engineTrackSpeed[m_nWagonGroup]/60.0f;
	m_fSpeed = engineTrackSpeed[m_nWagonGroup]/60.0f;
	m_vecTurnSpeed = CVector(0.0f, 0.0f, 0.0f);

	if(engineTrackSpeed[m_nWagonGroup] > 0.001f){
		SetStatus(STATUS_TRAIN_MOVING);
		m_bTrainStopping = false;
		m_bProcessDoor = true;
	}else{
		SetStatus(STATUS_TRAIN_NOT_MOVING);
		m_bTrainStopping = true;
	}

	m_isFarAway = !((posFront - TheCamera.GetPosition()).Magnitude2D() < sq(250.0f));

	if(m_fWagonPosition == 20.0f && m_fSpeed > 0.0001f)
		if(Abs(TheCamera.GetPosition().z - GetPosition().z) < 15.0f)
			CPad::GetPad(0)->StartShake_Train(GetPosition().x, GetPosition().y);

	if(m_bProcessDoor)
		switch(m_nDoorState){
		case TRAIN_DOOR_CLOSED:
			if(m_bTrainStopping){
				m_nDoorTimer = CTimer::GetTimeInMilliseconds() + 1000;
				m_nDoorState = TRAIN_DOOR_OPENING;
				DMAudio.PlayOneShot(m_audioEntityId, SOUND_TRAIN_DOOR_CLOSE, 0.0f);
			}
			break;

		case TRAIN_DOOR_OPENING:
			if(CTimer::GetTimeInMilliseconds() < m_nDoorTimer){
				OpenTrainDoor(1.0f - (m_nDoorTimer - CTimer::GetTimeInMilliseconds())/1000.0f);
			}else{
				OpenTrainDoor(1.0f);
				m_nDoorState = TRAIN_DOOR_OPEN;
			}
			break;

		case TRAIN_DOOR_OPEN:
			if(!m_bTrainStopping){
				m_nDoorTimer = CTimer::GetTimeInMilliseconds() + 1000;
				m_nDoorState = TRAIN_DOOR_CLOSING;
				DMAudio.PlayOneShot(m_audioEntityId, SOUND_TRAIN_DOOR_OPEN, 0.0f);
			}
			break;

		case TRAIN_DOOR_CLOSING:
			if(CTimer::GetTimeInMilliseconds() < m_nDoorTimer){
				OpenTrainDoor((m_nDoorTimer - CTimer::GetTimeInMilliseconds())/1000.0f);
			}else{
				OpenTrainDoor(0.0f);
				m_nDoorState = TRAIN_DOOR_CLOSED;
				m_bProcessDoor = false;
			}
			break;
		}

	GetMatrix().UpdateRW();
	UpdateRwFrame();
	RemoveAndAdd();

	bIsStuck = false;
	bIsInSafePosition = true;
	bWasPostponed = false;

	// request/remove model
	if(m_isFarAway){
		if(m_rwObject)
			DeleteRwObject();
	}else if(CStreaming::HasModelLoaded(MI_TRAIN)){
		if(m_rwObject == nil){
			m_modelIndex = -1;
			SetModelIndex(MI_TRAIN);
		}
	}else{
		if(FindPlayerCoors().z * GetPosition().z >= 0.0f)
			CStreaming::RequestModel(MI_TRAIN, STREAMFLAGS_DEPENDENCY);
	}

	// Hit stuff
	if(m_bIsFirstWagon && GetStatus()== STATUS_TRAIN_MOVING){
		CVector front = GetPosition() + GetForward()*GetColModel()->boundingBox.max.y + m_vecMoveSpeed*CTimer::GetTimeStep();

		int x, xmin, xmax;
		int y, ymin, ymax;

		xmin = CWorld::GetSectorIndexX(front.x - 3.0f);
		if(xmin < 0) xmin = 0;
		xmax = CWorld::GetSectorIndexX(front.x + 3.0f);
		if(xmax > NUMSECTORS_X-1) xmax = NUMSECTORS_X-1;
		ymin = CWorld::GetSectorIndexY(front.y - 3.0f);
		if(ymin < 0) ymin = 0;
		ymax = CWorld::GetSectorIndexY(front.y + 3.0f);
		if(ymax > NUMSECTORS_Y-1) ymax = NUMSECTORS_X-1;

		CWorld::AdvanceCurrentScanCode();

		for(y = ymin; y <= ymax; y++)
			for(x = xmin; x <= xmax; x++){
				CSector *s = CWorld::GetSector(x, y);
				TrainHitStuff(s->m_lists[ENTITYLIST_VEHICLES]);
				TrainHitStuff(s->m_lists[ENTITYLIST_VEHICLES_OVERLAP]);
				TrainHitStuff(s->m_lists[ENTITYLIST_PEDS]);
				TrainHitStuff(s->m_lists[ENTITYLIST_PEDS_OVERLAP]);
			}
	}
}

void
CTrain::PreRender(void)
{
	CVehicleModelInfo *mi = (CVehicleModelInfo*)CModelInfo::GetModelInfo(GetModelIndex());

	if(m_bIsFirstWagon){
		CVector lookVector = GetPosition() - TheCamera.GetPosition();
		float camDist = lookVector.Magnitude();
		if(camDist != 0.0f)
			lookVector *= 1.0f/camDist;
		else
			lookVector = CVector(1.0f, 0.0f, 0.0f);
		float behindness = DotProduct(lookVector, GetForward());

		if(behindness < 0.0f){
			// In front of train
			CVector lightPos = mi->m_positions[TRAIN_POS_LIGHT_FRONT];
			CVector lightR = GetMatrix() * lightPos;
			CVector lightL = lightR;
			lightL -= GetRight()*2.0f*lightPos.x;

			float intensity = -0.4f*behindness + 0.2f;
			float size = 1.0f - behindness;

			if(behindness < -0.9f && camDist < 35.0f){
				// directly in front
				CCoronas::RegisterCorona((uintptr)this + 10, 255*intensity, 255*intensity, 255*intensity, 255,
					lightL, size, 80.0f,
					CCoronas::TYPE_NORMAL, CCoronas::FLARE_HEADLIGHTS, CCoronas::REFLECTION_ON,
					CCoronas::LOSCHECK_OFF, CCoronas::STREAK_ON, 0.0f);
				CCoronas::RegisterCorona((uintptr)this + 11, 255*intensity, 255*intensity, 255*intensity, 255,
					lightR, size, 80.0f,
					CCoronas::TYPE_NORMAL, CCoronas::FLARE_HEADLIGHTS, CCoronas::REFLECTION_ON,
					CCoronas::LOSCHECK_OFF, CCoronas::STREAK_ON, 0.0f);
			}else{
				CCoronas::RegisterCorona((uintptr)this + 10, 255*intensity, 255*intensity, 255*intensity, 255,
					lightL, size, 80.0f,
					CCoronas::TYPE_NORMAL, CCoronas::FLARE_NONE, CCoronas::REFLECTION_ON,
					CCoronas::LOSCHECK_OFF, CCoronas::STREAK_ON, 0.0f);
				CCoronas::RegisterCorona((uintptr)this + 11, 255*intensity, 255*intensity, 255*intensity, 255,
					lightR, size, 80.0f,
					CCoronas::TYPE_NORMAL, CCoronas::FLARE_NONE, CCoronas::REFLECTION_ON,
					CCoronas::LOSCHECK_OFF, CCoronas::STREAK_ON, 0.0f);
			}
		}
	}

	if(m_bIsLastWagon){
		CVector lightPos = mi->m_positions[TRAIN_POS_LIGHT_REAR];
		CVector lightR = GetMatrix() * lightPos;
		CVector lightL = lightR;
		lightL -= GetRight()*2.0f*lightPos.x;

		CCoronas::RegisterCorona((uintptr)this + 12, 255, 0, 0, 255,
			lightL, 1.0f, 80.0f,
			CCoronas::TYPE_NORMAL, CCoronas::FLARE_NONE, CCoronas::REFLECTION_ON,
			CCoronas::LOSCHECK_OFF, CCoronas::STREAK_ON, 0.0f);
		CCoronas::RegisterCorona((uintptr)this + 13, 255, 0, 0, 255,
			lightR, 1.0f, 80.0f,
			CCoronas::TYPE_NORMAL, CCoronas::FLARE_NONE, CCoronas::REFLECTION_ON,
			CCoronas::LOSCHECK_OFF, CCoronas::STREAK_ON, 0.0f);
	}
}

void
CTrain::Render(void)
{
	CEntity::Render();
}

void
CTrain::TrainHitStuff(CPtrList &list)
{
	CPtrNode *node;
	CPhysical *phys;

	for(node = list.first; node; node = node->next){
		phys = (CPhysical*)node->item;
		if(phys != this && Abs(this->GetPosition().z - phys->GetPosition().z) < 1.5f)
			phys->bHitByTrain = true;
	}
}

void
CTrain::AddPassenger(CPed *ped)
{
	int i = ped->m_vehDoor;
	if((i == TRAIN_POS_LEFT_ENTRY || i == TRAIN_POS_MID_ENTRY || i == TRAIN_POS_RIGHT_ENTRY) && pPassengers[i] == nil){
		pPassengers[i] = ped;
		m_nNumPassengers++;
	}else{
		for(i = 0; i < 6; i++)
			if(pPassengers[i] == nil){
				pPassengers[i] = ped;
				m_nNumPassengers++;
				return;
			}
	}
}

void
CTrain::OpenTrainDoor(float ratio)
{
	if(m_rwObject == nil)
		return;

	CMatrix doorL(RwFrameGetMatrix(m_aTrainNodes[TRAIN_DOOR_LHS]));
	CMatrix doorR(RwFrameGetMatrix(m_aTrainNodes[TRAIN_DOOR_RHS]));
	CVector posL = doorL.GetPosition();
	CVector posR = doorR.GetPosition();

	bool isClosed = Doors[0].IsClosed();	// useless

	Doors[0].Open(ratio);
	Doors[1].Open(ratio);

	if(isClosed)
		Doors[0].RetTranslationWhenClosed();	// useless

	posL.y = Doors[0].m_fPosn;
	posR.y = Doors[1].m_fPosn;

	doorL.SetTranslate(posL);
	doorR.SetTranslate(posR);

	doorL.UpdateRW();
	doorR.UpdateRW();
}



void
CTrain::InitTrains(void)
{
	int i, j;
	CTrain *train;

	// El train
	if(pTrackNodes == nil)
		ReadAndInterpretTrackFile("data\\paths\\tracks.dat", &pTrackNodes, &NumTrackNodes, 3, StationDist,
			&TotalLengthOfTrack, &TotalDurationOfTrack, aLineBits, false);
	// Subway
	if(pTrackNodes_S == nil)
		ReadAndInterpretTrackFile("data\\paths\\tracks2.dat", &pTrackNodes_S, &NumTrackNodes_S, 4, StationDist_S,
			&TotalLengthOfTrack_S, &TotalDurationOfTrack_S, aLineBits_S, true);

	int trainId;
	CStreaming::LoadAllRequestedModels(false);
	if(CModelInfo::GetModelInfo("train", &trainId))
		CStreaming::RequestModel(trainId, 0);
	CStreaming::LoadAllRequestedModels(false);

	// El-Train wagons
	float wagonPositions[] = { 0.0f, 20.0f, 40.0f,  0.0f, 20.0f };
	int8 firstWagon[]  = { 1, 0, 0,  1, 0 };
	int8 lastWagon[]   = { 0, 0, 1,  0, 1 };
	int16 wagonGroup[] = { 0, 0, 0,  1, 1 };
	for(i = 0; i < 5; i++){
		train = new CTrain(MI_TRAIN, PERMANENT_VEHICLE);
		train->GetMatrix().SetTranslate(0.0f, 0.0f, 0.0f);
		train->SetStatus(STATUS_ABANDONED);
		train->bIsLocked = true;
		train->m_fWagonPosition = wagonPositions[i];
		train->m_bIsFirstWagon = firstWagon[i];
		train->m_bIsLastWagon = lastWagon[i];
		train->m_nWagonGroup = wagonGroup[i];
		train->m_nWagonId = i;
		train->m_nCurTrackNode = 0;
		CWorld::Add(train);
	}

	// Subway wagons
	float wagonPositions_S[] = { 0.0f, 20.0f,  0.0f, 20.0f,  0.0f, 20.0f,  0.0f, 20.0f };
	int8 firstWagon_S[]  = { 1, 0,  1, 0,  1, 0,  1, 0 };
	int8 lastWagon_S[]   = { 0, 1,  0, 1,  0, 1,  0, 1 };
	int16 wagonGroup_S[] = { 0, 0,  1, 1,  2, 2,  3, 3 };
	for(i = 0; i < 8; i++){
		train = new CTrain(MI_TRAIN, PERMANENT_VEHICLE);
		train->GetMatrix().SetTranslate(0.0f, 0.0f, 0.0f);
		train->SetStatus(STATUS_ABANDONED);
		train->bIsLocked = true;
		train->m_fWagonPosition = wagonPositions_S[i];
		train->m_bIsFirstWagon = firstWagon_S[i];
		train->m_bIsLastWagon = lastWagon_S[i];
		train->m_nWagonGroup = wagonGroup_S[i];
		train->m_nWagonId = i;
		train->m_nCurTrackNode = 0;
		train->m_nTrackId = TRACK_SUBWAY;
		CWorld::Add(train);
	}

	// This code is actually useless, it seems it was used for announcements once
	for(i = 0; i < 3; i++){
		for(j = 0; pTrackNodes[j].t < StationDist[i]; j++);
		aStationCoors[i] = pTrackNodes[j].p;
	}
	for(i = 0; i < 4; i++){
		for(j = 0; pTrackNodes_S[j].t < StationDist_S[i]; j++);
		aStationCoors_S[i] = pTrackNodes_S[j].p;
	}
}

void
CTrain::Shutdown(void)
{
	delete[] pTrackNodes;
	delete[] pTrackNodes_S;
	pTrackNodes = nil;
	pTrackNodes_S = nil;
}

void
CTrain::ReadAndInterpretTrackFile(Const char *filename, CTrainNode **nodes, int16 *numNodes, int32 numStations, float *stationDists,
		float *totalLength, float *totalDuration, CTrainInterpolationLine *interpLines, bool rightRail)
{
	bool readingFile = false;
	int bp, lp;
	int i, tmp;

	if(*nodes == nil){
		readingFile = true;

		CFileMgr::LoadFile(filename, work_buff, sizeof(work_buff), "r");
		*gString = '\0';
		for(bp = 0, lp = 0; work_buff[bp] != '\n'; bp++, lp++)
			gString[lp] = work_buff[bp];
		bp++;
		// BUG: game doesn't terminate string and uses numNodes in sscanf directly
		gString[lp] = '\0';
		sscanf(gString, "%d", &tmp);
		*numNodes = tmp;
		*nodes = new CTrainNode[*numNodes];

		for(i = 0; i < *numNodes; i++){
			*gString = '\0';
			for(lp = 0; work_buff[bp] != '\n'; bp++, lp++)
				gString[lp] = work_buff[bp];
			bp++;
			// BUG: game doesn't terminate string
			gString[lp] = '\0';
			sscanf(gString, "%f %f %f", &(*nodes)[i].p.x, &(*nodes)[i].p.y, &(*nodes)[i].p.z);
		}

		// Coordinates are of one of the rails, but we want the center
		float toCenter = rightRail ? 0.9f : -0.9f;
		CVector fwd;
		for(i = 0; i < *numNodes; i++){
			if(i == *numNodes-1)
				fwd = (*nodes)[0].p - (*nodes)[i].p;
			else
				fwd = (*nodes)[i+1].p - (*nodes)[i].p;
			CVector right = CrossProduct(fwd, CVector(0.0f, 0.0f, 1.0f));
			right.Normalise();
			(*nodes)[i].p -= right*toCenter;
		}
	}

	// Calculate length of segments and track
	float t = 0.0f;
	for(i = 0; i < *numNodes; i++){
		(*nodes)[i].t = t;
		t += ((*nodes)[(i+1) % (*numNodes)].p - (*nodes)[i].p).Magnitude2D();
	}
	*totalLength = t;

	// Find correct z values
	if(readingFile){
		CColPoint colpoint;
		CEntity *entity;
		for(i = 0; i < *numNodes; i++){
			CVector p = (*nodes)[i].p;
			p.z += 1.0f;
			if(CWorld::ProcessVerticalLine(p, p.z-0.5f, colpoint, entity, true, false, false, false, true, false, nil))
				(*nodes)[i].p.z = colpoint.point.z;
			(*nodes)[i].p.z += 0.2f;
		}
	}

	// Create animation for stopping at stations
	// TODO: figure out magic numbers?
	float position = 0.0f;
	float time = 0.0f;
	int j = 0;
	for(i = 0; i < numStations; i++){
		// Start at full speed
		interpLines[j].type = 1;
		interpLines[j].time = time;
		interpLines[j].position = position;
		interpLines[j].speed = 15.0f;
		interpLines[j].acceleration = 0.0f;
		j++;
		// distance to next keyframe
		float dist = (stationDists[i]-40.0f) - position;
		time += dist/15.0f;
		position += dist;

		// Now slow down 40 units before stop
		interpLines[j].type = 2;
		interpLines[j].time = time;
		interpLines[j].position = position;
		interpLines[j].speed = 15.0f;
		interpLines[j].acceleration = -45.0f/32.0f;
		j++;
		time += 80.0f/15.0f;
		position += 40.0f;	// at station

		// stopping
		interpLines[j].type = 0;
		interpLines[j].time = time;
		interpLines[j].position = position;
		interpLines[j].speed = 0.0f;
		interpLines[j].acceleration = 0.0f;
		j++;
		time += 25.0f;

		// accelerate again
		interpLines[j].type = 2;
		interpLines[j].time = time;
		interpLines[j].position = position;
		interpLines[j].speed = 0.0f;
		interpLines[j].acceleration = 45.0f/32.0f;
		j++;
		time += 80.0f/15.0f;
		position += 40.0f;	// after station
	}
	// last keyframe
	interpLines[j].type = 1;
	interpLines[j].time = time;
	interpLines[j].position = position;
	interpLines[j].speed = 15.0f;
	interpLines[j].acceleration = 0.0f;
	j++;
	*totalDuration = time + (*totalLength - position)/15.0f;

	// end
	interpLines[j].time = *totalDuration;
}

void
PlayAnnouncement(uint8 sound, uint8 station)
{
	// this was gone in a PC version but inlined on PS2
	cAudioScriptObject *obj = new cAudioScriptObject;
	obj->AudioId = sound;
	obj->Posn = CTrain::aStationCoors[station];
	obj->AudioEntity = AEHANDLE_NONE;
	DMAudio.CreateOneShotScriptObject(obj);
}

void
ProcessTrainAnnouncements(void)
{
	for (int i = 0; i < ARRAY_SIZE(StationDist); i++) {
		for (int j = 0; j < ARRAY_SIZE(EngineTrackPosition); j++) {
			if (!bTrainArrivalAnnounced[i]) {
				float preDist = StationDist[i] - 100.0f;
				if (preDist < 0.0f)
					preDist += TotalLengthOfTrack;
				if (EngineTrackPosition[j] > preDist && EngineTrackPosition[j] < StationDist[i]) {
					bTrainArrivalAnnounced[i] = true;
					PlayAnnouncement(SCRIPT_SOUND_TRAIN_ANNOUNCEMENT_1, i);
					break;
				}
			} else {
				float postDist = StationDist[i] + 10.0f;
#ifdef FIX_BUGS
				if (postDist > TotalLengthOfTrack)
					postDist -= TotalLengthOfTrack;
#else
				if (postDist < 0.0f) // does this even make sense here?
					postDist += TotalLengthOfTrack;
#endif
				if (EngineTrackPosition[j] > StationDist[i] && EngineTrackPosition[j] < postDist) {
					bTrainArrivalAnnounced[i] = false;
					PlayAnnouncement(SCRIPT_SOUND_TRAIN_ANNOUNCEMENT_2, i);
					break;
				}
			}
		}
	}
}

void
CTrain::UpdateTrains(void)
{
	int i, j;
	uint32 time;
	float t, deltaT;

	if(TheCamera.GetPosition().x > 200.0f && TheCamera.GetPosition().x < 1600.0f &&
	   TheCamera.GetPosition().y > -1000.0f && TheCamera.GetPosition().y < 500.0f){
		// Update El-Train

		time = CTimer::GetTimeInMilliseconds();
		for(i = 0; i < 2; i++){
			t = TotalDurationOfTrack * (float)(time & 0x1FFFF)/0x20000;
			// find current frame
			for(j = 0; t > aLineBits[j+1].time; j++);

			deltaT = t - aLineBits[j].time;
			switch(aLineBits[j].type){
			case 0:	// standing still
				EngineTrackPosition[i] = aLineBits[j].position;
				EngineTrackSpeed[i] = 0.0f;
				break;
			case 1:	// moving with constant speed
				EngineTrackPosition[i] = aLineBits[j].position + aLineBits[j].speed*deltaT;
				EngineTrackSpeed[i] = (TotalDurationOfTrack*1000.0f/0x20000) * aLineBits[j].speed;
				break;
			case 2:	// accelerating/braking
				EngineTrackPosition[i] = aLineBits[j].position + (aLineBits[j].speed + aLineBits[j].acceleration*deltaT)*deltaT;
				EngineTrackSpeed[i] = (TotalDurationOfTrack*1000.0f/0x20000)*aLineBits[j].speed + 2.0f*aLineBits[j].acceleration*deltaT;
				break;
			}

			// time offset for each train
			time += 0x20000/2;
		}

		ProcessTrainAnnouncements();
	}

	// Update Subway
	time = CTimer::GetTimeInMilliseconds();
	for(i = 0; i < 4; i++){
		t = TotalDurationOfTrack_S * (float)(time & 0x3FFFF)/0x40000;
		// find current frame
		for(j = 0; t > aLineBits_S[j+1].time; j++);

		deltaT = t - aLineBits_S[j].time;
		switch(aLineBits_S[j].type){
		case 0:	// standing still
			EngineTrackPosition_S[i] = aLineBits_S[j].position;
			EngineTrackSpeed_S[i] = 0.0f;
			break;
		case 1:	// moving with constant speed
			EngineTrackPosition_S[i] = aLineBits_S[j].position + aLineBits_S[j].speed*deltaT;
			EngineTrackSpeed_S[i] = (TotalDurationOfTrack*1000.0f/0x40000) * aLineBits_S[j].speed;
			break;
		case 2:	// accelerating/braking
			EngineTrackPosition_S[i] = aLineBits_S[j].position + (aLineBits_S[j].speed + aLineBits_S[j].acceleration*deltaT)*deltaT;
			EngineTrackSpeed_S[i] = (TotalDurationOfTrack*1000.0f/0x40000)*aLineBits_S[j].speed + 2.0f*aLineBits_S[j].acceleration*deltaT;
			break;
		}

		// time offset for each train
		time += 0x40000/4;
	}
}
