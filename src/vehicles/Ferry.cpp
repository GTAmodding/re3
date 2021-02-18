#include "common.h"
#include "main.h"
#include "Ferry.h"

#include "AudioManager.h"
#include "Camera.h"
#include "Coronas.h"
#include "FileMgr.h"
#include "General.h"
#include "Leeds.h"
#include "Particle.h"
#include "PlayerPed.h"
#include "Streaming.h"
#include "TempColModels.h"
#include "WaterLevel.h"
#include "World.h"

CFerryInst* CFerry::mspInst;

#define FERRY_SPEED (0.1f)
#define FERRY_SLOWDOWN_DISTANCE (50.0f)
#define FERRY_TIME_STOPPED_AT_STATION (10.0f)

CFerry::CFerry(int32 id, uint8 owner) : CVehicle(owner)
{
	m_bPlayerArrivedHorn = false;
	m_nTimeAlongPath = 0;
	m_vehType = VEHICLE_TYPE_FERRY;
	CVehicleModelInfo* mi = (CVehicleModelInfo*)CModelInfo::GetModelInfo(id);
	pHandling = mod_HandlingManager.GetHandlingData((tVehicleType)mi->m_handlingId);
	SetModelIndex(id);
	m_doors[0].Init(DEGTORAD(90.0f), 0.0f, 1, 0);
	m_doors[1].Init(DEGTORAD(-95.0f), 0.0f, 1, 0);
	m_doors[2].Init(DEGTORAD(-90.0f), 0.0f, 1, 0);
	m_doors[3].Init(DEGTORAD(95.0f), 0.0f, 1, 0);
	m_fTurnMass = 100000000.0f;
	m_fAirResistance = 0.9994f;
	m_fElasticity = 0.05f;
	m_nNumMaxPassengers = 1;
	m_fMass = 100000000.0f;
	bInfiniteMass = true;
	m_phy_flagA08 = true;
	m_bFerryDocked = false;
	SetStatus(STATUS_FERRY_MOVING);
	bUsesCollision = true;
	m_nDoorTimer = CTimer::GetTimeInMilliseconds();
	m_nDoorState = FERRY_DOOR_CLOSED;
	m_bApproachingDock = false;
	m_nSkipFerryStatus = 0;
	m_nCollision = 0;
	m_pDefaultColModel = mi->GetColModel();
	m_level = LEVEL_GENERIC;
}

void CFerry::Init(void* pInstancePtr)
{
	mspInst = (CFerryInst*)pInstancePtr;
	if (mspInst)
		return;
	// the following code should be wrapped in a define
	mspInst = new CFerryInst();
	memset(mspInst, 0, sizeof(CFerryInst));
	for (int k = 0; k < NUM_FERRY_PATHS; k++) {
		mspInst->pPathData[k] = new CFerryPath();
		mspInst->pPathData[k]->aLineBits = new CFerryInterpolationLine[NUM_FERRY_STATIONS * 4 + 2];

		const char* filename = "Data\\PATHS\\FERRY1.DAT"; // actually base::cStringT<char> filename; filename += k+1; filename += ".DAT"

		bool readingFile = false;
		int bp, lp;
		int i, tmp;
		CFerryPath* pPath = mspInst->pPathData[k];

		if (pPath->aTrackNodes == nil) {
			readingFile = true;

			CFileMgr::LoadFile(filename, work_buff, sizeof(work_buff), "r");
			*gString = '\0';
			for (bp = 0, lp = 0; work_buff[bp] != '\n'; bp++, lp++)
				gString[lp] = work_buff[bp];
			bp++;
#ifdef FIX_BUGS
			gString[lp] = '\0';
#endif
			sscanf(gString, "%d", &tmp);
			pPath->NumTrackNodes = tmp;
			pPath->aTrackNodes = new CFerryNode[tmp];

			for (i = 0; i < pPath->NumTrackNodes; i++) {
				*gString = '\0';
				for (lp = 0; work_buff[bp] != '\n'; bp++, lp++)
					gString[lp] = work_buff[bp];
				bp++;
#ifdef FIX_BUGS
				gString[lp] = '\0';
#endif
				sscanf(gString, "%f %f %f", &pPath->aTrackNodes[i].x, &pPath->aTrackNodes[i].y, &pPath->aTrackNodes[i].z);
				pPath->aTrackNodes[i].z = 0.0f;
			}
		}

		// Calculate length of segments and track
		float t = 0.0f;
		for (i = 0; i < pPath->NumTrackNodes; i++) {
			pPath->aTrackNodes[i].t = t;
			t += Sqrt(SQR(pPath->aTrackNodes[(i + 1) % pPath->NumTrackNodes].x - pPath->aTrackNodes[i].x)) +
				(SQR(pPath->aTrackNodes[(i + 1) % pPath->NumTrackNodes].y - pPath->aTrackNodes[i].y));
		}
		pPath->TotalLengthOfTrack = t;

		// Find correct z values
		if (readingFile) {
			CColPoint colpoint;
			CEntity* entity;
			for (i = 0; i < pPath->NumTrackNodes; i++) {
				CVector p(pPath->aTrackNodes[i].x, pPath->aTrackNodes[i].y, pPath->aTrackNodes[i].z + 1.0f);
				if (CWorld::ProcessVerticalLine(p, p.z - 0.5f, colpoint, entity, true, false, false, false, true, false, nil))
					pPath->aTrackNodes[i].z = colpoint.point.z;
				pPath->aTrackNodes[i].z += 0.2f;
			}
		}

		int nStationIndices[NUM_FERRY_STATIONS];
		for (int i = 0; i < NUM_FERRY_STATIONS; i++)
			nStationIndices[i] = 0;
		int nCurrentStation = 0;

		for (i = 0; i < pPath->NumTrackNodes; i++) {
			CFerryNode* pCurNode = &pPath->aTrackNodes[i];
			CFerryNode* pNextNode = (i + 1 < pPath->NumTrackNodes) ? &pPath->aTrackNodes[i + 1] : &pPath->aTrackNodes[0];
			CFerryNode* pPrevNode = (i - 1 >= 0) ? &pPath->aTrackNodes[i - 1] : &pPath->aTrackNodes[pPath->NumTrackNodes - 1];
			if (pCurNode->x - pNextNode->x > 0.0f && pPrevNode->x - pCurNode->x < 0.0f)
				nStationIndices[nCurrentStation++] = i;
			if (pCurNode->x - pNextNode->x < 0.0f && pPrevNode->x - pCurNode->x > 0.0f)
				nStationIndices[nCurrentStation++] = i;
		}

		float stationDists[NUM_FERRY_STATIONS];
		for (i = 0; i < NUM_FERRY_STATIONS; i++)
			stationDists[i] = pPath->aTrackNodes[nStationIndices[i]].t;

		// Create animation for stopping at stations
		float position = 0.0f;
		float time = 0.0f;
		int j = 0;
		for (i = 0; i < NUM_FERRY_STATIONS; i++) {
			// Start at full speed
			pPath->aLineBits[j].type = FERRY_CRUISING;
			pPath->aLineBits[j].time = time;
			pPath->aLineBits[j].position = position;
			pPath->aLineBits[j].speed = FERRY_SPEED;
			pPath->aLineBits[j].acceleration = 0.0f;
			j++;
			// distance to next keyframe
			float dist = (stationDists[i] - FERRY_SLOWDOWN_DISTANCE) - position;
			time += dist / FERRY_SPEED;
			position += dist;

			// Now slow down 50 units before stop
			pPath->aLineBits[j].type = FERRY_SLOWING;
			pPath->aLineBits[j].time = time;
			pPath->aLineBits[j].position = position;
			pPath->aLineBits[j].speed = FERRY_SPEED;
			pPath->aLineBits[j].acceleration = -(FERRY_SPEED * FERRY_SPEED) / (4 * FERRY_SLOWDOWN_DISTANCE);
			j++;
			time += 2 * FERRY_SLOWDOWN_DISTANCE / FERRY_SPEED;
			position += FERRY_SLOWDOWN_DISTANCE;	// at station

			// stopping
			pPath->aLineBits[j].type = FERRY_STOPPED;
			pPath->aLineBits[j].time = time;
			pPath->aLineBits[j].position = position;
			pPath->aLineBits[j].speed = 0.0f;
			pPath->aLineBits[j].acceleration = 0.0f;
			j++;
			time += FERRY_TIME_STOPPED_AT_STATION;

			// accelerate again
			pPath->aLineBits[j].type = FERRY_ACCELERATING;
			pPath->aLineBits[j].time = time;
			pPath->aLineBits[j].position = position;
			pPath->aLineBits[j].speed = 0.0f;
			pPath->aLineBits[j].acceleration = (FERRY_SPEED * FERRY_SPEED) / (4 * FERRY_SLOWDOWN_DISTANCE);
			j++;
			time += 2 * FERRY_SLOWDOWN_DISTANCE / FERRY_SPEED;
			position += FERRY_SLOWDOWN_DISTANCE;	// after station
		}
		// last keyframe
		pPath->aLineBits[j].type = FERRY_CRUISING;
		pPath->aLineBits[j].time = time;
		pPath->aLineBits[j].position = position;
		pPath->aLineBits[j].speed = FERRY_SPEED;
		pPath->aLineBits[j].acceleration = 0.0f;
		j++;
		pPath->TotalDurationOfTrack = time + (pPath->TotalLengthOfTrack - position) / FERRY_SPEED;

		// end
		pPath->aLineBits[j].time = pPath->TotalDurationOfTrack;
	}
}

void CFerry::InitFerrys(void)
{
	if (!mspInst)
		Init(nil);
	for (int i = 0; i < NUM_FERRIES; i++)
		mspInst->m_apFerries[i] = nil;
	CStreaming::LoadAllRequestedModels(false);
	CStreaming::RequestModel(MI_FERRY, 0);
	CStreaming::LoadAllRequestedModels(false);
}

void CFerry::SwitchFerryCollision(int type)
{
	for (int i = 0; i < NUM_FERRIES; i++) {
		CFerry* pFerry = GetFerry(i);
		if (pFerry && pFerry->m_nCollision != type) {
			pFerry->m_nCollision = type;
			CVehicleModelInfo* mi = pFerry->GetModelInfo();
			if (type == 1)
				mi->SetColModel(&CTempColModels::ms_colModelFerryDocked);
			else
				mi->SetColModel(pFerry->m_pDefaultColModel, true);
		}
	}
}

void CFerry::UpdateFerrys(void)
{
	int i, j;
	float t, deltaT;
	if (mspInst->m_bFerriesDisabled)
		return;
	for (i = 0; i < NUM_FERRIES; i++) {
		CFerry* pFerry = GetFerry(i);
		if (pFerry) {
			pFerry->m_nTimeAlongPath += CTimer::GetTimeStepInMilliseconds();
			t = mspInst->pPathData[i/2]->TotalDurationOfTrack * (float)((pFerry->m_nTimeAlongPath + (i & 1) * 0x20000) & 0x3FFFF) / 0x40000;
			// find current frame
			for (j = 0; t > mspInst->pPathData[i / 2]->aLineBits[j + 1].time; j++);

			deltaT = t - mspInst->pPathData[i / 2]->aLineBits[j].time;
			if (pFerry->m_bFerryDocked) {
				if (mspInst->pPathData[i / 2]->aLineBits[j].type == FERRY_SLOWING) {
					pFerry->m_nTimeAlongPath += (mspInst->pPathData[i / 2]->aLineBits[j + 1].time - mspInst->pPathData[i / 2]->aLineBits[j].time);
					j++;
					if (j > NUM_FERRY_STATIONS * 4 + 1)
						j = 0;
					pFerry->m_bApproachingDock = true;
					if ((i & 1) == 0) {
						GetFerry(i + 1)->m_bApproachingDock = true;
						GetFerry(i + 1)->m_nTimeAlongPath += (mspInst->pPathData[i / 2]->aLineBits[j + 1].time - mspInst->pPathData[i / 2]->aLineBits[j].time);
					}
					else {
						GetFerry(i - 1)->m_bApproachingDock = true;
						GetFerry(i - 1)->m_nTimeAlongPath += (mspInst->pPathData[i / 2]->aLineBits[j + 1].time - mspInst->pPathData[i / 2]->aLineBits[j].time);
					}
				}
			}
			if (pFerry->m_nSkipFerryStatus == 1) {
				float fDelta = 0.0f;
				pFerry->m_nSkipFerryStatus = 2;
				while (mspInst->pPathData[i / 2]->aLineBits[j].type != FERRY_STOPPED) {
					fDelta += (mspInst->pPathData[i / 2]->aLineBits[j + 1].time - mspInst->pPathData[i / 2]->aLineBits[j].time);
					j++;
					if (j > NUM_FERRY_STATIONS * 4)
						j = 0;
				}
				pFerry->m_nTimeAlongPath += fDelta;
				pFerry->m_bApproachingDock = true;
				if ((i & 1) == 0) {
					GetFerry(i + 1)->m_bApproachingDock = true;
					GetFerry(i + 1)->m_nTimeAlongPath += fDelta;
				}
				else {
					GetFerry(i - 1)->m_bApproachingDock = true;
					GetFerry(i - 1)->m_nTimeAlongPath += fDelta;
				}
			}
			switch (mspInst->pPathData[i / 2]->aLineBits[j].type) {
			case FERRY_STOPPED:
				mspInst->m_afPositions[i] = mspInst->pPathData[i / 2]->aLineBits[j].position;
				mspInst->m_afSpeeds[i] = 0.0f;
				break;
			case FERRY_CRUISING:
				mspInst->m_afPositions[i] = mspInst->pPathData[i / 2]->aLineBits[j].position + mspInst->pPathData[i / 2]->aLineBits[j].speed * deltaT;
				mspInst->m_afSpeeds[i] = (mspInst->pPathData[i / 2]->TotalDurationOfTrack * 1000.0f / 0x40000) * mspInst->pPathData[i / 2]->aLineBits[j].speed;
				break;
			case FERRY_SLOWING:
			case FERRY_ACCELERATING:
				pFerry->m_bApproachingDock = (mspInst->pPathData[i / 2]->aLineBits[j].type == FERRY_SLOWING);
				mspInst->m_afPositions[i] = mspInst->pPathData[i / 2]->aLineBits[j].position + (mspInst->pPathData[i / 2]->aLineBits[j].speed + mspInst->pPathData[i / 2]->aLineBits[j].acceleration * deltaT) * deltaT;
				mspInst->m_afSpeeds[i] = (mspInst->pPathData[i / 2]->TotalDurationOfTrack * 1000.0f / 0x40000) * (mspInst->pPathData[i / 2]->aLineBits[j].speed + 2 * mspInst->pPathData[i / 2]->aLineBits[j].acceleration * deltaT);
				break;
			}
		}
	}
}

void CFerry::SetModelIndex(uint32 mi)
{
	CVehicle::SetModelIndex(mi);
	for (int i = 0; i < NUM_FERRY_NODES; i++)
		m_aFerryNodes[i] = nil;
	CClumpModelInfo::FillFrameArray(GetClump(), m_aFerryNodes);
}

void CFerry::PreRender(void)
{
	CVehicleModelInfo* mi = GetModelInfo();
	if (CGeneral::GetRandomTrueFalse())
		CParticle::AddParticle(PARTICLE_FERRY_CHIM_SMOKE, GetMatrix() * mi->m_positions[FERRY_POS_CHIM_LEFT], CVector(0.0f, 0.0f, 0.2f));
	CVector vVectorToCamera = GetPosition() - TheCamera.GetPosition();
	CVector vDirectionToCamera;
	float fDistanceToCamera = vVectorToCamera.Magnitude();
	if (fDistanceToCamera == 0.0f)
		vVectorToCamera = vDirectionToCamera = CVector(1.0f, 0.0f, 0.0f);
	else
		vDirectionToCamera = vVectorToCamera / fDistanceToCamera;
	float dp = DotProduct(GetForward(), vDirectionToCamera);
	if (dp < 0.0f) {
		CVector vFrontLightPosition = GetMatrix() * mi->m_positions[FERRY_POS_LIGHT_FRONT];
		CVector vFrontLightPosition2 = vFrontLightPosition - 2 * mi->m_positions[FERRY_POS_LIGHT_FRONT].x * GetRight();
		float size = -dp + 1.0f;
		float fIntensity = -dp * 0.4f + 0.2f;
		if (dp < -0.9f && fDistanceToCamera < 35.0f) {
			uint8 intensity = fIntensity * 255.0f;
			CCoronas::RegisterCorona((uint32)(uintptr)this + 26, intensity, intensity, intensity, 255, vFrontLightPosition2, size, 80.0f, CCoronas::TYPE_NORMAL, CCoronas::FLARE_HEADLIGHTS, CCoronas::REFLECTION_ON, CCoronas::LOSCHECK_OFF, CCoronas::STREAK_ON, 0.0f);
			CCoronas::RegisterCorona((uint32)(uintptr)this + 27, intensity, intensity, intensity, 255, vFrontLightPosition, size, 80.0f, CCoronas::TYPE_NORMAL, CCoronas::FLARE_HEADLIGHTS, CCoronas::REFLECTION_ON, CCoronas::LOSCHECK_OFF, CCoronas::STREAK_ON, 0.0f);
		}
		else {
			uint8 intensity = fIntensity * 255.0f;
			CCoronas::RegisterCorona((uint32)(uintptr)this + 26, intensity, intensity, intensity, 255, vFrontLightPosition2, size, 80.0f, CCoronas::TYPE_NORMAL, CCoronas::FLARE_NONE, CCoronas::REFLECTION_ON, CCoronas::LOSCHECK_OFF, CCoronas::STREAK_ON, 0.0f);
			CCoronas::RegisterCorona((uint32)(uintptr)this + 27, intensity, intensity, intensity, 255, vFrontLightPosition, size, 80.0f, CCoronas::TYPE_NORMAL, CCoronas::FLARE_NONE, CCoronas::REFLECTION_ON, CCoronas::LOSCHECK_OFF, CCoronas::STREAK_ON, 0.0f);
		}
	}
	CVector vRearLightPosition = GetMatrix() * mi->m_positions[FERRY_POS_LIGHT_REAR];
	CVector vRearLightPosition2 = vRearLightPosition - 2 * mi->m_positions[FERRY_POS_LIGHT_REAR].x * GetRight();
	CCoronas::RegisterCorona((uint32)(uintptr)this + 28, 255, 0, 0, 255, vRearLightPosition2, 1.0f, 80.0f, CCoronas::TYPE_NORMAL, CCoronas::FLARE_NONE, CCoronas::REFLECTION_ON, CCoronas::LOSCHECK_OFF, CCoronas::STREAK_ON, 0.0f);
	CCoronas::RegisterCorona((uint32)(uintptr)this + 29, 255, 0, 0, 255, vRearLightPosition, 1.0f, 80.0f, CCoronas::TYPE_NORMAL, CCoronas::FLARE_NONE, CCoronas::REFLECTION_ON, CCoronas::LOSCHECK_OFF, CCoronas::STREAK_ON, 0.0f);
}

void CFerry::Render(void)
{
	m_bAlreadyRendered = true;
	CEntity::Render();
}

void CFerry::RenderAllRemaning(void)
{
	for (int i = 0; i < NUM_FERRIES; i++) {
		CFerry* pFerry = GetFerry(i);
		if (pFerry) {
			if (!pFerry->m_bAlreadyRendered)
				pFerry->Render();
			pFerry->m_bAlreadyRendered = false;
		}
	}
}

void CFerry::FerryHitStuff(CPtrList& lst)
{
	for (CPtrNode* pNode = lst.first; pNode != nil; pNode = pNode->next) {
		CPhysical* pEntity = (CPhysical*)pNode->item;
		if (pEntity != this && Abs(GetPosition().x - pEntity->GetPosition().z) < 1.5f)
			pEntity->bHitByTrain = true;
	}
}

void CFerry::ProcessControl(void)
{
	if (gbModelViewer)
		return;
	PruneWakeTrail();
	if (m_isFarAway && (m_nFerryId + CTimer::GetFrameCounter() & 0xF) != 0)
		return;
	CFerryPath* pPath = mspInst->pPathData[m_nFerryId / 2];
	float fPosition = mspInst->m_afPositions[m_nFerryId];
	float fSpeed = mspInst->m_afSpeeds[m_nFerryId];
	if (fPosition < 0.0f)
		fPosition += pPath->TotalLengthOfTrack;

	CFerryNode* trackNodes = mspInst->pPathData[m_nFerryId / 2]->aTrackNodes;
	int16 numTrackNodes = mspInst->pPathData[m_nFerryId / 2]->NumTrackNodes;
	float totalLengthOfTrack = mspInst->pPathData[m_nFerryId / 2]->TotalLengthOfTrack;
	float trackPosition = mspInst->m_afPositions[m_nFerryId];
	float trackSpeed = mspInst->m_afSpeeds[m_nFerryId];

	float trackPositionRear = trackPosition;
	if (trackPositionRear < 0.0f)
		trackPositionRear += totalLengthOfTrack;

	// Advance current node to appropriate position
	float pos1, pos2;
	int nextTrackNode = m_nCurTrackNode + 1;
	pos1 = trackNodes[m_nCurTrackNode].t;
	if (nextTrackNode < numTrackNodes)
		pos2 = trackNodes[nextTrackNode].t;
	else {
		nextTrackNode = 0;
		pos2 = totalLengthOfTrack;
	}
	while (trackPositionRear < pos1 || trackPositionRear > pos2) {
		m_nCurTrackNode = (m_nCurTrackNode + 1) % numTrackNodes;
		nextTrackNode = m_nCurTrackNode + 1;
		pos1 = trackNodes[m_nCurTrackNode].t;
		if (nextTrackNode < numTrackNodes)
			pos2 = trackNodes[nextTrackNode].t;
		else {
			nextTrackNode = 0;
			pos2 = totalLengthOfTrack;
		}
	}
	float dist = trackNodes[nextTrackNode].t - trackNodes[m_nCurTrackNode].t;
	if (dist < 0.0f)
		dist += totalLengthOfTrack;
	float f = (trackPositionRear - trackNodes[m_nCurTrackNode].t) / dist;
	CVector posRear = (1.0f - f) * CVector(trackNodes[m_nCurTrackNode].x, trackNodes[m_nCurTrackNode].y, trackNodes[m_nCurTrackNode].z) +
		f * CVector(trackNodes[nextTrackNode].x, trackNodes[nextTrackNode].y, trackNodes[nextTrackNode].z);

	// Now same again for the front
	float trackPositionFront = trackPositionRear + 20.0f;
	if (trackPositionFront > totalLengthOfTrack)
		trackPositionFront -= totalLengthOfTrack;
	int curTrackNodeFront = m_nCurTrackNode;
	int nextTrackNodeFront = curTrackNodeFront + 1;
	pos1 = trackNodes[curTrackNodeFront].t;
	if (nextTrackNodeFront < numTrackNodes)
		pos2 = trackNodes[nextTrackNodeFront].t;
	else {
		nextTrackNodeFront = 0;
		pos2 = totalLengthOfTrack;
	}
	while (trackPositionFront < pos1 || trackPositionFront > pos2) {
		curTrackNodeFront = (curTrackNodeFront + 1) % numTrackNodes;
		nextTrackNodeFront = curTrackNodeFront + 1;
		pos1 = trackNodes[curTrackNodeFront].t;
		if (nextTrackNodeFront < numTrackNodes)
			pos2 = trackNodes[nextTrackNodeFront].t;
		else {
			nextTrackNodeFront = 0;
			pos2 = totalLengthOfTrack;
		}
	}
	dist = trackNodes[nextTrackNodeFront].t - trackNodes[curTrackNodeFront].t;
	if (dist < 0.0f)
		dist += totalLengthOfTrack;
	f = (trackPositionFront - trackNodes[curTrackNodeFront].t) / dist;
	CVector posFront = (1.0f - f) * CVector(trackNodes[curTrackNodeFront].x, trackNodes[curTrackNodeFront].y, trackNodes[curTrackNodeFront].z) +
		f * CVector(trackNodes[nextTrackNodeFront].x, trackNodes[nextTrackNodeFront].y, trackNodes[nextTrackNodeFront].z);

	// Now set matrix
	SetPosition((posRear + posFront) / 2.0f);
	CVector fwd = posFront - posRear;
	m_vecForwardSpeed = fwd;
	fwd.Normalise();
	float dp = DotProduct(fwd, GetForward());
	if (Abs(dp) > 1.001f || Abs(dp) < 0.999f) {
		CVector df = CrossProduct(fwd, GetForward());
		CMatrix tmp;
		float angle;
		if (m_nSkipFerryStatus == 2) {
			m_nSkipFerryStatus = 0;
			angle = (fwd.x < 0.0f && GetForward().x < 0.0f) ? PI - Acos(dp) : Acos(dp);
		}
		else {
			angle = 0.001f;
		}
		if (dp > 0.0f && df.z < 0.0f || dp <= 0.0f && df.z > 0.0f)
			angle = -angle;
		tmp.SetRotateZ(angle);
		fwd = Multiply3x3(GetForward(), tmp);
	}
	else
		fwd = GetForward();
	CVector right = CrossProduct(fwd, CVector(0.0f, 0.0f, 1.0f));
	right.Normalise();
	CVector up = CrossProduct(right, fwd);
	GetRight() = right;
	GetUp() = up;
	GetForward() = fwd;

	// Set speed
	m_vecMoveSpeed = fwd * trackSpeed / 60.0f;
	m_fSpeed = trackSpeed / 60.0f;
	m_vecTurnSpeed = CVector(0.0f, 0.0f, 0.0f);

	if (m_vecMoveSpeed.MagnitudeSqr() > 0.001f || !m_bApproachingDock) {
		SetStatus(STATUS_FERRY_MOVING);
		m_bFerryDocked = false;
		m_bPlayerArrivedHorn = false;
		m_nTimeLeftStation = CTimer::GetTimeInMilliseconds();
	}
	else {
		SetStatus(STATUS_FERRY_NOT_MOVING);
		PlayArrivedHorn();
		m_bFerryDocked = true;
	}

	m_isFarAway = !((posFront - TheCamera.GetPosition()).Magnitude2D() < sq(1000.0f));

	switch (m_nDoorState) {
	case FERRY_DOOR_CLOSED:
		break;
	case FERRY_DOOR_OPENING:
		if (CTimer::GetTimeInMilliseconds() < m_nDoorTimer) {
			OpenFerryDoor(1.0f - (m_nDoorTimer - CTimer::GetTimeInMilliseconds()) / 1000.0f);
		}
		else {
			OpenFerryDoor(1.0f);
			m_nDoorState = FERRY_DOOR_OPEN;
		}
		break;

	case FERRY_DOOR_OPEN:
		break;

	case FERRY_DOOR_CLOSING:
		if (CTimer::GetTimeInMilliseconds() < m_nDoorTimer) {
			OpenFerryDoor((m_nDoorTimer - CTimer::GetTimeInMilliseconds()) / 1000.0f);
		}
		else {
			OpenFerryDoor(0.0f);
			m_nDoorState = FERRY_DOOR_CLOSED;
		}
		break;
	}

	CVector wn = CWaterLevel::GetWaterNormal(GetPosition().x, GetPosition().y);
	float fRollAngle = wn.x - GetUp().x;
	if (fRollAngle < 0.0f)
		fRollAngle = Max(-0.05f, fRollAngle);
	else
		fRollAngle = Min(0.05f, fRollAngle);

	CVector oldPos = GetPosition();
	SetPosition(-GetPosition());
	CMatrix tmp2;
	tmp2.SetRotateX(fRollAngle);
	tmp2 = tmp2 * GetMatrix();
	tmp2.SetTranslateOnly(oldPos);
	GetMatrix() = tmp2;

	GetMatrix().UpdateRW();
	UpdateRwFrame();
	RemoveAndAdd();

	bIsStuck = false;
	bIsInSafePosition = true;
	bWasPostponed = false;

	// request/remove model
	if (m_isFarAway) {
		if (m_rwObject)
			DeleteRwObject();
	}
	else if (CStreaming::HasModelLoaded(MI_FERRY)) {
		if (m_rwObject == nil) {
			m_modelIndex = -1;
			SetModelIndex(MI_FERRY);
		}
	}
	else {
		if (FindPlayerCoors().z * GetPosition().z >= 0.0f)
			CStreaming::RequestModel(MI_FERRY, STREAMFLAGS_DEPENDENCY);
	}

	// Hit stuff
	if (GetStatus() == STATUS_TRAIN_MOVING) {
		CVector nfwd = GetForward() * GetColModel()->boundingBox.max.y;
		if (m_vecForwardSpeed.x > 0.0f)
			nfwd = -nfwd;
		if ((m_nFerryId & 1) == 0)
			nfwd = -nfwd;

		int x, xmin, xmax;
		int y, ymin, ymax;

		CVector front = GetPosition() + nfwd + m_vecMoveSpeed * CTimer::GetTimeStep();
		if (!m_isFarAway && m_vecMoveSpeed.Magnitude2D() > 0.05f)
			AddWakePoint(nfwd * 0.85f + GetPosition());

		xmin = CWorld::GetSectorIndexX(front.x - 3.0f);
		if (xmin < 0) xmin = 0;
		xmax = CWorld::GetSectorIndexX(front.x + 3.0f);
		if (xmax > NUMSECTORS_X - 1) xmax = NUMSECTORS_X - 1;
		ymin = CWorld::GetSectorIndexY(front.y - 3.0f);
		if (ymin < 0) ymin = 0;
		ymax = CWorld::GetSectorIndexY(front.y + 3.0f);
		if (ymax > NUMSECTORS_Y - 1) ymax = NUMSECTORS_X - 1;

		CWorld::AdvanceCurrentScanCode();

		for (y = ymin; y <= ymax; y++)
			for (x = xmin; x <= xmax; x++) {
				CSector* s = CWorld::GetSector(x, y);
				FerryHitStuff(s->m_lists[ENTITYLIST_VEHICLES]);
				FerryHitStuff(s->m_lists[ENTITYLIST_VEHICLES_OVERLAP]);
				FerryHitStuff(s->m_lists[ENTITYLIST_PEDS]);
				FerryHitStuff(s->m_lists[ENTITYLIST_PEDS_OVERLAP]);
			}
	}
}

void CFerry::OpenFerryDoor(float ratio)
{
	if (!m_rwObject)
		return;

	int door1 = 0;
	int door2 = 1;
	if (!m_bUseFrontDoor) {
		door1 = 2;
		door2 = 3;
	}
	int node1 = m_bUseFrontDoor ? FERRY_DOOR_FRONT : FERRY_DOOR_BACK;
	int node2 = m_bUseFrontDoor ? FERRY_RAMP_FRONT : FERRY_RAMP_BACK;
	CMatrix doorL(RwFrameGetMatrix(m_aFerryNodes[node1]));
	CMatrix doorR(RwFrameGetMatrix(m_aFerryNodes[node2]));
	CVector posL = doorL.GetPosition();
	CVector posR = doorR.GetPosition();

	bool isClosed = m_doors[0].IsClosed();	// useless

	m_doors[door1].Open(ratio);
	m_doors[door2].Open(ratio);

	doorL.SetRotateXOnly(m_doors[door1].m_fAngle);
	doorR.SetRotateXOnly(m_doors[door2].m_fAngle);

	doorL.UpdateRW();
	doorR.UpdateRW();
}

CVector CFerry::GetBoardingSpace(CFerry::eSpaceUse use, CFerry::eSpaceStyle style, uint8 position)
{
	CVehicleModelInfo* pModelInfo = GetModelInfo();
	CVector space;
	if (m_nFerryId & 1) {
		if (style == FERRY_SPACE_STYLE_0)
			style = FERRY_SPACE_STYLE_1;
		else
			style = FERRY_SPACE_STYLE_0;
	}
	switch (use) {
	case FERRY_SPACE_PED:
		space = pModelInfo->m_positions[FERRY_POS_PED_POINT];
		break;
	case FERRY_SPACE_CAR:
		space = pModelInfo->m_positions[FERRY_POS_CAR1 + position];
		break;
	}
	switch (style) {
	case FERRY_SPACE_STYLE_0:
		space = GetMatrix() * space;
		break;
	case FERRY_SPACE_STYLE_1:
		space = GetMatrix() * space - (2 * space.x) * GetRight() - (2 * space.y) * GetForward();
		break;
	}
	return space;
}

CFerry* CFerry::GetClosestFerry(float x, float y)
{
	int closest = -1;
	float mindist = 9999.9f;
	for (int i = 0; i < NUM_FERRIES; i++) {
		CFerry* pFerry = GetFerry(i);
		if (pFerry) {
			float dist = ((CVector2D)pFerry->GetPosition() - CVector2D(x, y)).Magnitude();
			if (dist < 300.0f && dist < mindist) {
				mindist = dist;
				closest = i;
			}
		}
	}
	if (closest == -1)
		return nil;
	return GetFerry(closest);
}

bool CFerry::IsDocked(void)
{
	return m_bFerryDocked;
}

void CFerry::OpenDoor(void)
{
	printf("opening the ferry door\n");
	m_nDoorState = FERRY_DOOR_OPENING;
	m_nDoorTimer = CTimer::GetTimeInMilliseconds() + 10000;
	CVehicleModelInfo* pModelInfo = GetModelInfo();
	CVector2D vPlayerPos = FindPlayerPed()->GetPosition();
	float fDistToCar4 = (vPlayerPos - GetMatrix() * pModelInfo->m_positions[FERRY_POS_CAR4]).Magnitude();
	float fDistToCar1 = (vPlayerPos - GetMatrix() * pModelInfo->m_positions[FERRY_POS_CAR1]).Magnitude();
	m_bUseFrontDoor = true;
	if (fDistToCar4 < fDistToCar1)
		m_bUseFrontDoor = false;
	// AudioManager.DirectlyEnqueueSample(0xb8,0,0,1,0x5622,0x7f,0x14,0); // TODO
}

void CFerry::CloseDoor(void)
{
	printf("closing the ferry door\n");
	m_nDoorState = FERRY_DOOR_CLOSING;
	m_nDoorTimer = CTimer::GetTimeInMilliseconds() + 10000;
	// AudioManager.DirectlyEnqueueSample(0xb8, 0, 0, 1, 0x5622, 0x7f, 0x14, 0); // TODO
}

bool CFerry::IsDoorOpen(void)
{
	return m_nDoorState == FERRY_DOOR_OPEN;
}

bool CFerry::IsDoorClosed(void)
{
	return m_nDoorState == FERRY_DOOR_CLOSED;
}

void CFerry::CompleteDorrMovement(void)
{
	m_nDoorTimer = 0;
}

void CFerry::DissableFerryPath(int)
{
}

void CFerry::EnableFerryPath(int path)
{
	CStreaming::LoadAllRequestedModels(false);
	CStreaming::RequestModel(MI_FERRY, 0);
	CStreaming::LoadAllRequestedModels(false);
	for (int i = path * 2; i < path * 2 + 2; i++) {
		CFerry* pFerry = new CFerry(MI_FERRY, PERMANENT_VEHICLE);
		bool bDirect = i & 1;
		mspInst->m_apFerries[i] = pFerry;
		pFerry->SetPosition(0.0f, 0.0f, 0.0f);
		pFerry->SetStatus(STATUS_ABANDONED);
		pFerry->bIsLocked = true;
		pFerry->SetHeading(bDirect ? HALFPI : 3 * HALFPI);
		pFerry->m_nFerryId = i;
		pFerry->m_nCurTrackNode = 0;
		CWorld::Add(pFerry);
	}
}

void CFerry::SkipFerryToNextDock(void)
{
	m_nSkipFerryStatus = 1;
}

void CFerry::PruneWakeTrail(void)
{
	int16 num_remaining = 0;
	for (int i = 0; i < NUM_WAKE_POINTS; i++) {
		if (mspInst->m_afWakePointTimer[m_nFerryId][i] <= 0.0f)
			break;
		if (mspInst->m_afWakePointTimer[m_nFerryId][i] <= CTimer::GetTimeStep()) {
			mspInst->m_afWakePointTimer[m_nFerryId][i] = 0.0f;
			break;
		}
		mspInst->m_afWakePointTimer[m_nFerryId][i] -= CTimer::GetTimeStep();
		num_remaining++;
	}
	mspInst->m_anNumWakePoints[m_nFerryId] = num_remaining;
}

void CFerry::AddWakePoint(CVector point)
{
	if (mspInst->m_afWakePointTimer[m_nFerryId][0] > 0.0f) {
		int nNewWakePoints = Min(NUM_WAKE_POINTS - 1, mspInst->m_anNumWakePoints[m_nFerryId]);
		for (int i = 0; i < nNewWakePoints; i++) {
			mspInst->m_avWakePoints[m_nFerryId][i + 1] = mspInst->m_avWakePoints[m_nFerryId][i];
			mspInst->m_afWakePointTimer[m_nFerryId][i + 1] = mspInst->m_afWakePointTimer[m_nFerryId][i];
		}
	}
	mspInst->m_avWakePoints[m_nFerryId][0] = point;
	mspInst->m_afWakePointTimer[m_nFerryId][0] = 900.0f; // TODO: define
	mspInst->m_anNumWakePoints[m_nFerryId] += 1;
}

void CFerry::PlayArrivedHorn(void)
{
	if (m_bPlayerArrivedHorn)
		return;
	m_bPlayerArrivedHorn = true;
	float fDistToCamera = (GetPosition() - TheCamera.GetPosition()).Magnitude();
	if (fDistToCamera < 200.0f) {
		uint8 volume = (200.0f - fDistToCamera) / 200.0f * 127;
		// AudioManager.DirectlyEnqueueSample(0x32, 0, 0, 1, 18000, volume, 0x32, 0); // TODO
	}
}

CVector CFerry::GetNearestDoor(CVector)
{
	return CVector(0.0f, 0.0f, 1.0f);
}

void CFerry::SetupForMultiplayer(void)
{
	for (int i = 0; i < NUM_FERRIES; i++)
		mspInst->m_apFerries[i] = nil;
	CStreaming::SetModelIsDeletable(MI_FERRY);
}

void CFerry::Write(base::cRelocatableChunkWriter& writer)
{
	writer.AllocateRaw(mspInst, sizeof(*mspInst), 4);
	if (!mspInst)
		return;
	for (int i = 0; i < NUM_FERRY_PATHS; i++) {
		writer.AllocateRaw(mspInst->pPathData[i], sizeof(*mspInst->pPathData[i]), 4);
		writer.AddPatch(&mspInst->pPathData[i]);
		writer.AllocateRaw(mspInst->pPathData[i]->aTrackNodes, mspInst->pPathData[i]->NumTrackNodes * sizeof(CFerryNode), 4);
		writer.AddPatch(&mspInst->pPathData[i]->aTrackNodes);
		writer.AllocateRaw(mspInst->pPathData[i]->aLineBits, (NUM_FERRY_STATIONS * 4 + 2) * sizeof(CFerryInterpolationLine), 4);
		writer.AddPatch(&mspInst->pPathData[i]->aTrackNodes);
	}
}

