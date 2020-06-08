#include "common.h"
#include "main.h"

#include "General.h"
#include "ModelIndices.h"
#include "FileMgr.h"
#include "Streaming.h"
#include "Replay.h"
#include "Camera.h"
#include "DMAudio.h"
#include "Wanted.h"
#include "Coronas.h"
#include "Particle.h"
#include "Explosion.h"
#include "World.h"
#include "HandlingMgr.h"
#include "Plane.h"

CPlaneNode *pPathNodes;
CPlaneNode *pPath2Nodes;
CPlaneNode *pPath3Nodes;
CPlaneNode *pPath4Nodes;
int32 NumPathNodes;
int32 NumPath2Nodes;
int32 NumPath3Nodes;
int32 NumPath4Nodes;
float TotalLengthOfFlightPath;
float TotalLengthOfFlightPath2;
float TotalLengthOfFlightPath3;
float TotalLengthOfFlightPath4;
float TotalDurationOfFlightPath;
float TotalDurationOfFlightPath2;
float TotalDurationOfFlightPath3;
float TotalDurationOfFlightPath4;
float LandingPoint;
float TakeOffPoint;
CPlaneInterpolationLine aPlaneLineBits[6];

float PlanePathPosition[3];
float OldPlanePathPosition[3];
float PlanePathSpeed[3];
float PlanePath2Position[3];
float PlanePath3Position;
float PlanePath4Position;
float PlanePath2Speed[3];
float PlanePath3Speed;
float PlanePath4Speed;


enum
{
	CESNA_STATUS_NONE,	// doesn't even exist
	CESNA_STATUS_FLYING,
	CESNA_STATUS_DESTROYED,
	CESNA_STATUS_LANDED,
};

int32 CesnaMissionStatus;
int32 CesnaMissionStartTime;
CPlane *pDrugRunCesna;
int32 DropOffCesnaMissionStatus;
int32 DropOffCesnaMissionStartTime;
CPlane *pDropOffCesna;


CPlane::CPlane(int32 id, uint8 CreatedBy)
 : CVehicle(CreatedBy)
{
	CVehicleModelInfo *mi = (CVehicleModelInfo*)CModelInfo::GetModelInfo(id);
	m_vehType = VEHICLE_TYPE_PLANE;
	pHandling = mod_HandlingManager.GetHandlingData((eHandlingId)mi->m_handlingId);
	SetModelIndex(id);

	m_fMass = 100000000.0f;
	m_fTurnMass = 100000000.0f;
	m_fAirResistance = 0.9994f;
	m_fElasticity = 0.05f;

	bUsesCollision = false;
	m_bHasBeenHit = false;
	m_bIsDrugRunCesna = false;
	m_bIsDropOffCesna = false;

	SetStatus(STATUS_PLANE);
	bIsBIGBuilding = true;
	m_level = LEVEL_NONE;

#ifdef FIX_BUGS
	m_isFarAway = true;
#endif
}

CPlane::~CPlane()
{
	DeleteRwObject();
}

void
CPlane::SetModelIndex(uint32 id)
{
	CVehicle::SetModelIndex(id);
}

void
CPlane::DeleteRwObject(void)
{
	if(m_rwObject && RwObjectGetType(m_rwObject) == rpATOMIC){
		m_matrix.Detach();
		if(RwObjectGetType(m_rwObject) == rpATOMIC){	// useless check
			RwFrame *f = RpAtomicGetFrame((RpAtomic*)m_rwObject);
			RpAtomicDestroy((RpAtomic*)m_rwObject);
			RwFrameDestroy(f);
		}
		m_rwObject = nil;
	}
	CEntity::DeleteRwObject();
}

// There's a LOT of copy and paste in here. Maybe this could be refactored somehow
void
CPlane::ProcessControl(void)
{
	int i;
	CVector pos;

	// Explosion
	if(m_bHasBeenHit){
		// BUG: since this is all based on frames, you can skip the explosion processing when you go into the menu
		if(GetModelIndex() == MI_AIRTRAIN){
			int frm = CTimer::GetFrameCounter() - m_nFrameWhenHit;
			if(frm == 20){
				static int nFrameGen;
				CRGBA colors[8];

				CExplosion::AddExplosion(nil, FindPlayerPed(), EXPLOSION_HELI, GetMatrix() * CVector(0.0f, 0.0f, 0.0f), 0);

				colors[0] = CRGBA(0, 0, 0, 255);
				colors[1] = CRGBA(224, 230, 238, 255);
				colors[2] = CRGBA(224, 230, 238, 255);
				colors[3] = CRGBA(0, 0, 0, 255);
				colors[4] = CRGBA(224, 230, 238, 255);
				colors[5] = CRGBA(0, 0, 0, 255);
				colors[6] = CRGBA(0, 0, 0, 255);
				colors[7] = CRGBA(224, 230, 238, 255);

				CVector dir;
				for(i = 0; i < 40; i++){
					dir.x = CGeneral::GetRandomNumberInRange(-2.0f, 2.0f);
					dir.y = CGeneral::GetRandomNumberInRange(-2.0f, 2.0f);
					dir.z = CGeneral::GetRandomNumberInRange(0.0f, 2.0f);
					int rotSpeed = CGeneral::GetRandomNumberInRange(10, 30);
					if(CGeneral::GetRandomNumber() & 1)
						rotSpeed = -rotSpeed;
					int f = ++nFrameGen & 3;
					CParticle::AddParticle(PARTICLE_HELI_DEBRIS, GetMatrix() * CVector(0.0f, 0.0f, 0.0f), dir,
						nil, CGeneral::GetRandomNumberInRange(0.1f, 1.0f),
						colors[nFrameGen], rotSpeed, 0, f, 0);
				}
			}
			if(frm >= 40 && frm <= 80 && frm & 1){
				if(frm & 1){
					pos.x = ((CGeneral::GetRandomNumber() & 0x3F) - 32) * 0.2f;
					pos.z = ((CGeneral::GetRandomNumber() & 0x3F) - 32) * 0.2f;
					pos.y = frm - 40;
					pos = GetMatrix() * pos;
				}else{
					pos.x = ((CGeneral::GetRandomNumber() & 0x3F) - 32) * 0.2f;
					pos.z = ((CGeneral::GetRandomNumber() & 0x3F) - 32) * 0.2f;
					pos.y = 40 - frm;
					pos = GetMatrix() * pos;
				}
				CExplosion::AddExplosion(nil, FindPlayerPed(), EXPLOSION_HELI, pos, 0);
			}
			if(frm == 60)
				bRenderScorched = true;
			if(frm == 82){
				TheCamera.SetFadeColour(255, 255, 255);
				TheCamera.Fade(0.0f, FADE_OUT);
				TheCamera.ProcessFade();
				TheCamera.Fade(1.0f, FADE_IN);
				FlagToDestroyWhenNextProcessed();
			}
		}else{
			int frm = CTimer::GetFrameCounter() - m_nFrameWhenHit;
			if(frm == 20){
				static int nFrameGen;
				CRGBA colors[8];

				CExplosion::AddExplosion(nil, FindPlayerPed(), EXPLOSION_HELI, GetMatrix() * CVector(0.0f, 0.0f, 0.0f), 0);

				colors[0] = CRGBA(0, 0, 0, 255);
				colors[1] = CRGBA(224, 230, 238, 255);
				colors[2] = CRGBA(224, 230, 238, 255);
				colors[3] = CRGBA(0, 0, 0, 255);
				colors[4] = CRGBA(252, 66, 66, 255);
				colors[5] = CRGBA(0, 0, 0, 255);
				colors[6] = CRGBA(0, 0, 0, 255);
				colors[7] = CRGBA(252, 66, 66, 255);

				for(i = 0; i < 40; i++){
					int rotSpeed = CGeneral::GetRandomNumberInRange(30.0f, 20.0f);
					if(CGeneral::GetRandomNumber() & 1)
						rotSpeed = -rotSpeed;
					int f = ++nFrameGen & 3;
					CParticle::AddParticle(PARTICLE_HELI_DEBRIS, GetMatrix() * CVector(0.0f, 0.0f, 0.0f),
						CVector(CGeneral::GetRandomNumberInRange(-2.0f, 2.0f),
						        CGeneral::GetRandomNumberInRange(-2.0f, 2.0f),
						        CGeneral::GetRandomNumberInRange(0.0f, 2.0f)),
						nil, CGeneral::GetRandomNumberInRange(0.1f, 1.0f),
						colors[nFrameGen], rotSpeed, 0, f, 0);
				}
			}
			if(frm >= 40 && frm <= 60 && frm & 1){
				if(frm & 1){
					pos.x = ((CGeneral::GetRandomNumber() & 0x3F) - 32) * 0.1f;
					pos.z = ((CGeneral::GetRandomNumber() & 0x3F) - 32) * 0.1f;
					pos.y = (frm - 40)*0.3f;
					pos = GetMatrix() * pos;
				}else{
					pos.x = ((CGeneral::GetRandomNumber() & 0x3F) - 32) * 0.1f;
					pos.z = ((CGeneral::GetRandomNumber() & 0x3F) - 32) * 0.1f;
					pos.y = (40 - frm)*0.3f;
					pos = GetMatrix() * pos;
				}
				CExplosion::AddExplosion(nil, FindPlayerPed(), EXPLOSION_HELI, pos, 0);
			}
			if(frm == 30)
				bRenderScorched = true;
			if(frm == 61){
				TheCamera.SetFadeColour(200, 200, 200);
				TheCamera.Fade(0.0f, FADE_OUT);
				TheCamera.ProcessFade();
				TheCamera.Fade(1.0f, FADE_IN);
				if(m_bIsDrugRunCesna){
					CesnaMissionStatus = CESNA_STATUS_DESTROYED;
					pDrugRunCesna = nil;
				}
				if(m_bIsDropOffCesna){
					DropOffCesnaMissionStatus = CESNA_STATUS_DESTROYED;
					pDropOffCesna = nil;
				}
				FlagToDestroyWhenNextProcessed();
			}
		}
	}

	// Update plane position and speed
	if(GetModelIndex() == MI_AIRTRAIN || !m_isFarAway || ((CTimer::GetFrameCounter() + m_randomSeed) & 7) == 0){
		if(GetModelIndex() == MI_AIRTRAIN){
			float pathPositionRear = PlanePathPosition[m_nPlaneId] - 30.0f;
			if(pathPositionRear < 0.0f)
				pathPositionRear += TotalLengthOfFlightPath;
			float pathPosition = pathPositionRear + 30.0f;

			float pitch = 0.0f;
			float distSinceTakeOff = pathPosition - TakeOffPoint;
			if(distSinceTakeOff <= 0.0f && distSinceTakeOff > -70.0f){
				// shortly before take off
				pitch = 1.0f - distSinceTakeOff/-70.0f;
			}else if(distSinceTakeOff >= 0.0f && distSinceTakeOff < 100.0f){
				// shortly after take off
				pitch = 1.0f - distSinceTakeOff/100.0f;
			}

			float distSinceLanding = pathPosition - LandingPoint;
			if(distSinceLanding <= 0.0f && distSinceLanding > -200.0f){
				// shortly before landing
				pitch = 1.0f - distSinceLanding/-200.0f;
			}else if(distSinceLanding >= 0.0f && distSinceLanding < 70.0f){
				// shortly after landing
				pitch = 1.0f - distSinceLanding/70.0f;
			}


			// Advance current node to appropriate position
			float pos1, pos2;
			int nextTrackNode = m_nCurPathNode + 1;
			pos1 = pPathNodes[m_nCurPathNode].t;
			if(nextTrackNode < NumPathNodes)
				pos2 = pPathNodes[nextTrackNode].t;
			else{
				nextTrackNode = 0;
				pos2 = TotalLengthOfFlightPath;
			}
			while(pathPositionRear < pos1 || pathPositionRear > pos2){
				m_nCurPathNode = (m_nCurPathNode+1) % NumPathNodes;
				nextTrackNode = m_nCurPathNode + 1;
				pos1 = pPathNodes[m_nCurPathNode].t;
				if(nextTrackNode < NumPathNodes)
					pos2 = pPathNodes[nextTrackNode].t;
				else{
					nextTrackNode = 0;
					pos2 = TotalLengthOfFlightPath;
				}
			}
			bool bothOnGround = pPathNodes[m_nCurPathNode].bOnGround && pPathNodes[nextTrackNode].bOnGround;
			if(PlanePathPosition[m_nPlaneId] >= LandingPoint && OldPlanePathPosition[m_nPlaneId] < LandingPoint)
				DMAudio.PlayOneShot(m_audioEntityId, SOUND_PLANE_ON_GROUND, 0.0f);
			float dist = pPathNodes[nextTrackNode].t - pPathNodes[m_nCurPathNode].t;
			if(dist < 0.0f)
				dist += TotalLengthOfFlightPath;
			float f = (pathPositionRear - pPathNodes[m_nCurPathNode].t)/dist;
			CVector posRear = (1.0f - f)*pPathNodes[m_nCurPathNode].p + f*pPathNodes[nextTrackNode].p;

			// Same for the front
			float pathPositionFront = pathPositionRear + 60.0f;
			if(pathPositionFront > TotalLengthOfFlightPath)
				pathPositionFront -= TotalLengthOfFlightPath;
			int curPathNodeFront = m_nCurPathNode;
			int nextPathNodeFront = curPathNodeFront + 1;
			pos1 = pPathNodes[curPathNodeFront].t;
			if(nextPathNodeFront < NumPathNodes)
				pos2 = pPathNodes[nextPathNodeFront].t;
			else{
				nextPathNodeFront = 0;
				pos2 = TotalLengthOfFlightPath;
			}
			while(pathPositionFront < pos1 || pathPositionFront > pos2){
				curPathNodeFront = (curPathNodeFront+1) % NumPathNodes;
				nextPathNodeFront = curPathNodeFront + 1;
				pos1 = pPathNodes[curPathNodeFront].t;
				if(nextPathNodeFront < NumPathNodes)
					pos2 = pPathNodes[nextPathNodeFront].t;
				else{
					nextPathNodeFront = 0;
					pos2 = TotalLengthOfFlightPath;
				}
			}
			dist = pPathNodes[nextPathNodeFront].t - pPathNodes[curPathNodeFront].t;
			if(dist < 0.0f)
				dist += TotalLengthOfFlightPath;
			f = (pathPositionFront - pPathNodes[curPathNodeFront].t)/dist;
			CVector posFront = (1.0f - f)*pPathNodes[curPathNodeFront].p + f*pPathNodes[nextPathNodeFront].p;

			// And for another point 60 units in front of the plane, used to calculate roll
			float pathPositionFront2 = pathPositionFront + 60.0f;
			if(pathPositionFront2 > TotalLengthOfFlightPath)
				pathPositionFront2 -= TotalLengthOfFlightPath;
			int curPathNodeFront2 = m_nCurPathNode;
			int nextPathNodeFront2 = curPathNodeFront2 + 1;
			pos1 = pPathNodes[curPathNodeFront2].t;
			if(nextPathNodeFront2 < NumPathNodes)
				pos2 = pPathNodes[nextPathNodeFront2].t;
			else{
				nextPathNodeFront2 = 0;
				pos2 = TotalLengthOfFlightPath;
			}
			while(pathPositionFront2 < pos1 || pathPositionFront2 > pos2){
				curPathNodeFront2 = (curPathNodeFront2+1) % NumPathNodes;
				nextPathNodeFront2 = curPathNodeFront2 + 1;
				pos1 = pPathNodes[curPathNodeFront2].t;
				if(nextPathNodeFront2 < NumPathNodes)
					pos2 = pPathNodes[nextPathNodeFront2].t;
				else{
					nextPathNodeFront2 = 0;
					pos2 = TotalLengthOfFlightPath;
				}
			}
			dist = pPathNodes[nextPathNodeFront2].t - pPathNodes[curPathNodeFront2].t;
			if(dist < 0.0f)
				dist += TotalLengthOfFlightPath;
			f = (pathPositionFront2 - pPathNodes[curPathNodeFront2].t)/dist;
			CVector posFront2 = (1.0f - f)*pPathNodes[curPathNodeFront2].p + f*pPathNodes[nextPathNodeFront2].p;

			// Now set matrix
			GetMatrix().GetPosition() = (posRear + posFront) / 2.0f;
			GetMatrix().GetPosition().z += 4.3f;
			CVector fwd = posFront - posRear;
			fwd.Normalise();
			if(pitch != 0.0f){
				fwd.z += 0.4f*pitch;
				fwd.Normalise();
			}
			CVector fwd2 = posFront2 - posRear;
			fwd2.Normalise();
			CVector roll = CrossProduct(fwd, fwd2);
			CVector right = CrossProduct(fwd, CVector(0.0f, 0.0f, 1.0f));
			if(!bothOnGround)
				right.z += 3.0f*roll.z;
			right.Normalise();
			CVector up = CrossProduct(right, fwd);
			GetMatrix().GetRight() = right;
			GetMatrix().GetUp() = up;
			GetMatrix().GetForward() = fwd;

			// Set speed
			m_vecMoveSpeed = fwd*PlanePathSpeed[m_nPlaneId]/60.0f;
			m_fSpeed = PlanePathSpeed[m_nPlaneId]/60.0f;
			m_vecTurnSpeed = CVector(0.0f, 0.0f, 0.0f);

			m_isFarAway = !((posFront - TheCamera.GetPosition()).Magnitude2D() < sq(300.0f));
		}else{
			float planePathPosition;
			float totalLengthOfFlightPath;
			CPlaneNode *pathNodes;
			float planePathSpeed;
			int numPathNodes;

			if(m_bIsDrugRunCesna){
				planePathPosition = PlanePath3Position;
				totalLengthOfFlightPath = TotalLengthOfFlightPath3;
				pathNodes = pPath3Nodes;
				planePathSpeed = PlanePath3Speed;
				numPathNodes = NumPath3Nodes;
				if(CesnaMissionStatus == CESNA_STATUS_LANDED){
					pDrugRunCesna = nil;
					FlagToDestroyWhenNextProcessed();
				}
			}else if(m_bIsDropOffCesna){
				planePathPosition = PlanePath4Position;
				totalLengthOfFlightPath = TotalLengthOfFlightPath4;
				pathNodes = pPath4Nodes;
				planePathSpeed = PlanePath4Speed;
				numPathNodes = NumPath4Nodes;
				if(DropOffCesnaMissionStatus == CESNA_STATUS_LANDED){
					pDropOffCesna = nil;
					FlagToDestroyWhenNextProcessed();
				}
			}else{
				planePathPosition = PlanePath2Position[m_nPlaneId];
				totalLengthOfFlightPath = TotalLengthOfFlightPath2;
				pathNodes = pPath2Nodes;
				planePathSpeed = PlanePath2Speed[m_nPlaneId];
				numPathNodes = NumPath2Nodes;
			}

			// Advance current node to appropriate position
			float pathPositionRear = planePathPosition - 10.0f;
			if(pathPositionRear < 0.0f)
				pathPositionRear += totalLengthOfFlightPath;
			float pos1, pos2;
			int nextTrackNode = m_nCurPathNode + 1;
			pos1 = pathNodes[m_nCurPathNode].t;
			if(nextTrackNode < numPathNodes)
				pos2 = pathNodes[nextTrackNode].t;
			else{
				nextTrackNode = 0;
				pos2 = totalLengthOfFlightPath;
			}
			while(pathPositionRear < pos1 || pathPositionRear > pos2){
				m_nCurPathNode = (m_nCurPathNode+1) % numPathNodes;
				nextTrackNode = m_nCurPathNode + 1;
				pos1 = pathNodes[m_nCurPathNode].t;
				if(nextTrackNode < numPathNodes)
					pos2 = pathNodes[nextTrackNode].t;
				else{
					nextTrackNode = 0;
					pos2 = totalLengthOfFlightPath;
				}
			}
			float dist = pathNodes[nextTrackNode].t - pathNodes[m_nCurPathNode].t;
			if(dist < 0.0f)
				dist += totalLengthOfFlightPath;
			float f = (pathPositionRear - pathNodes[m_nCurPathNode].t)/dist;
			CVector posRear = (1.0f - f)*pathNodes[m_nCurPathNode].p + f*pathNodes[nextTrackNode].p;

			// Same for the front
			float pathPositionFront = pathPositionRear + 20.0f;
			if(pathPositionFront > totalLengthOfFlightPath)
				pathPositionFront -= totalLengthOfFlightPath;
			int curPathNodeFront = m_nCurPathNode;
			int nextPathNodeFront = curPathNodeFront + 1;
			pos1 = pathNodes[curPathNodeFront].t;
			if(nextPathNodeFront < numPathNodes)
				pos2 = pathNodes[nextPathNodeFront].t;
			else{
				nextPathNodeFront = 0;
				pos2 = totalLengthOfFlightPath;
			}
			while(pathPositionFront < pos1 || pathPositionFront > pos2){
				curPathNodeFront = (curPathNodeFront+1) % numPathNodes;
				nextPathNodeFront = curPathNodeFront + 1;
				pos1 = pathNodes[curPathNodeFront].t;
				if(nextPathNodeFront < numPathNodes)
					pos2 = pathNodes[nextPathNodeFront].t;
				else{
					nextPathNodeFront = 0;
					pos2 = totalLengthOfFlightPath;
				}
			}
			dist = pathNodes[nextPathNodeFront].t - pathNodes[curPathNodeFront].t;
			if(dist < 0.0f)
				dist += totalLengthOfFlightPath;
			f = (pathPositionFront - pathNodes[curPathNodeFront].t)/dist;
			CVector posFront = (1.0f - f)*pathNodes[curPathNodeFront].p + f*pathNodes[nextPathNodeFront].p;

			// And for another point 60 units in front of the plane, used to calculate roll
			float pathPositionFront2 = pathPositionFront + 30.0f;
			if(pathPositionFront2 > totalLengthOfFlightPath)
				pathPositionFront2 -= totalLengthOfFlightPath;
			int curPathNodeFront2 = m_nCurPathNode;
			int nextPathNodeFront2 = curPathNodeFront2 + 1;
			pos1 = pathNodes[curPathNodeFront2].t;
			if(nextPathNodeFront2 < numPathNodes)
				pos2 = pathNodes[nextPathNodeFront2].t;
			else{
				nextPathNodeFront2 = 0;
				pos2 = totalLengthOfFlightPath;
			}
			while(pathPositionFront2 < pos1 || pathPositionFront2 > pos2){
				curPathNodeFront2 = (curPathNodeFront2+1) % numPathNodes;
				nextPathNodeFront2 = curPathNodeFront2 + 1;
				pos1 = pathNodes[curPathNodeFront2].t;
				if(nextPathNodeFront2 < numPathNodes)
					pos2 = pathNodes[nextPathNodeFront2].t;
				else{
					nextPathNodeFront2 = 0;
					pos2 = totalLengthOfFlightPath;
				}
			}
			dist = pathNodes[nextPathNodeFront2].t - pathNodes[curPathNodeFront2].t;
			if(dist < 0.0f)
				dist += totalLengthOfFlightPath;
			f = (pathPositionFront2 - pathNodes[curPathNodeFront2].t)/dist;
			CVector posFront2 = (1.0f - f)*pathNodes[curPathNodeFront2].p + f*pathNodes[nextPathNodeFront2].p;

			// Now set matrix
			GetMatrix().GetPosition() = (posRear + posFront) / 2.0f;
			GetMatrix().GetPosition().z += 1.0f;
			CVector fwd = posFront - posRear;
			fwd.Normalise();
			CVector fwd2 = posFront2 - posRear;
			fwd2.Normalise();
			CVector roll = CrossProduct(fwd, fwd2);
			CVector right = CrossProduct(fwd, CVector(0.0f, 0.0f, 1.0f));
			right.z += 3.0f*roll.z;
			right.Normalise();
			CVector up = CrossProduct(right, fwd);
			GetMatrix().GetRight() = right;
			GetMatrix().GetUp() = up;
			GetMatrix().GetForward() = fwd;

			// Set speed
			m_vecMoveSpeed = fwd*planePathSpeed/60.0f;
			m_fSpeed = planePathSpeed/60.0f;
			m_vecTurnSpeed = CVector(0.0f, 0.0f, 0.0f);

			m_isFarAway = !((posFront - TheCamera.GetPosition()).Magnitude2D() < sq(300.0f));
		}
	}

	bIsInSafePosition = true;
	GetMatrix().UpdateRW();
	UpdateRwFrame();

	// Handle streaming and such
	CVehicleModelInfo *mi = (CVehicleModelInfo*)CModelInfo::GetModelInfo(GetModelIndex());
	if(m_isFarAway){
		// Switch to LOD model
		if(m_rwObject && RwObjectGetType(m_rwObject) == rpCLUMP){
			DeleteRwObject();
			if(mi->m_planeLodId != -1){
				m_rwObject = CModelInfo::GetModelInfo(mi->m_planeLodId)->CreateInstance();
				if(m_rwObject)
					m_matrix.Attach(RwFrameGetMatrix(RpAtomicGetFrame((RpAtomic*)m_rwObject)));
			}
		}
	}else if(CStreaming::HasModelLoaded(GetModelIndex())){
		if(m_rwObject && RwObjectGetType(m_rwObject) == rpATOMIC){
			// Get rid of LOD model
			m_matrix.Detach();
			if(m_rwObject){	// useless check
				if(RwObjectGetType(m_rwObject) == rpATOMIC){	// useless check
					RwFrame *f = RpAtomicGetFrame((RpAtomic*)m_rwObject);
					RpAtomicDestroy((RpAtomic*)m_rwObject);
					RwFrameDestroy(f);
				}
				m_rwObject = nil;
			}
		}
		// Set high detail model
		if(m_rwObject == nil){
			int id = GetModelIndex();
			m_modelIndex = -1;
			SetModelIndex(id);
		}
	}else{
		CStreaming::RequestModel(GetModelIndex(), STREAMFLAGS_DEPENDENCY);
	}
}

void
CPlane::PreRender(void)
{
	CVehicleModelInfo *mi = (CVehicleModelInfo*)CModelInfo::GetModelInfo(GetModelIndex());

	CVector lookVector = GetPosition() - TheCamera.GetPosition();
	float camDist = lookVector.Magnitude();
	if(camDist != 0.0f)
		lookVector *= 1.0f/camDist;
	else
		lookVector = CVector(1.0f, 0.0f, 0.0f);
	float behindness = DotProduct(lookVector, GetForward());

	// Wing lights
	if(behindness < 0.0f){
		// in front of plane
		CVector lightPos = mi->m_positions[PLANE_POS_LIGHT_RIGHT];
		CVector lightR = GetMatrix() * lightPos;
		CVector lightL = lightR;
		lightL -= GetRight()*2.0f*lightPos.x;

		float intensity = -0.6f*behindness + 0.4f;
		float size = 1.0f - behindness;

		if(behindness < -0.9f && camDist < 50.0f){
			// directly in front
			CCoronas::RegisterCorona((uintptr)this + 10, 255*intensity, 255*intensity, 255*intensity, 255,
				lightL, size, 240.0f,
				CCoronas::TYPE_NORMAL, CCoronas::FLARE_HEADLIGHTS, CCoronas::REFLECTION_ON,
				CCoronas::LOSCHECK_OFF, CCoronas::STREAK_ON, 0.0f);
			CCoronas::RegisterCorona((uintptr)this + 11, 255*intensity, 255*intensity, 255*intensity, 255,
				lightR, size, 240.0f,
				CCoronas::TYPE_NORMAL, CCoronas::FLARE_HEADLIGHTS, CCoronas::REFLECTION_ON,
				CCoronas::LOSCHECK_OFF, CCoronas::STREAK_ON, 0.0f);
		}else{
			CCoronas::RegisterCorona((uintptr)this + 10, 255*intensity, 255*intensity, 255*intensity, 255,
				lightL, size, 240.0f,
				CCoronas::TYPE_NORMAL, CCoronas::FLARE_NONE, CCoronas::REFLECTION_ON,
				CCoronas::LOSCHECK_OFF, CCoronas::STREAK_ON, 0.0f);
			CCoronas::RegisterCorona((uintptr)this + 11, 255*intensity, 255*intensity, 255*intensity, 255,
				lightR, size, 240.0f,
				CCoronas::TYPE_NORMAL, CCoronas::FLARE_NONE, CCoronas::REFLECTION_ON,
				CCoronas::LOSCHECK_OFF, CCoronas::STREAK_ON, 0.0f);
		}
	}

	// Tail light
	if(CTimer::GetTimeInMilliseconds() & 0x200){
		CVector pos = GetMatrix() * mi->m_positions[PLANE_POS_LIGHT_TAIL];

		CCoronas::RegisterCorona((uintptr)this + 12, 255, 0, 0, 255,
			pos, 1.0f, 120.0f,
			CCoronas::TYPE_NORMAL, CCoronas::FLARE_NONE, CCoronas::REFLECTION_ON,
			CCoronas::LOSCHECK_OFF, CCoronas::STREAK_ON, 0.0f);
	}
}

void
CPlane::Render(void)
{
	CEntity::Render();
}

#define CRUISE_SPEED (50.0f)
#define TAXI_SPEED (5.0f)

void
CPlane::InitPlanes(void)
{
	int i;

	CesnaMissionStatus = CESNA_STATUS_NONE;

	// Jumbo
	if(pPathNodes == nil){
		pPathNodes = LoadPath("data\\paths\\flight.dat", NumPathNodes, TotalLengthOfFlightPath, true);

		// Figure out which nodes are on ground
		CColPoint colpoint;
		CEntity *entity;
		for(i = 0; i < NumPathNodes; i++){
			if(CWorld::ProcessVerticalLine(pPathNodes[i].p, 1000.0f, colpoint, entity, true, false, false, false, true, false, nil)){
				pPathNodes[i].p.z = colpoint.point.z;
				pPathNodes[i].bOnGround = true;
			}else
				pPathNodes[i].bOnGround = false;
		}

		// Find lading and takeoff points
		LandingPoint = -1.0f;
		TakeOffPoint = -1.0f;
		bool lastOnGround = pPathNodes[NumPathNodes-1].bOnGround;
		for(i = 0; i < NumPathNodes; i++){
			if(pPathNodes[i].bOnGround && !lastOnGround)
				LandingPoint = pPathNodes[i].t;
			else if(!pPathNodes[i].bOnGround && lastOnGround)
				TakeOffPoint = pPathNodes[i].t;
			lastOnGround = pPathNodes[i].bOnGround;
		}

		// Animation
		float time = 0.0f;
		float position = 0.0f;
		// Start on ground with slow speed
		aPlaneLineBits[0].type = 1;
		aPlaneLineBits[0].time = time;
		aPlaneLineBits[0].position = position;
		aPlaneLineBits[0].speed = TAXI_SPEED;
		aPlaneLineBits[0].acceleration = 0.0f;
		float dist = (TakeOffPoint-600.0f) - position;
		time += dist/TAXI_SPEED;
		position += dist;

		// Accelerate to take off
		aPlaneLineBits[1].type = 2;
		aPlaneLineBits[1].time = time;
		aPlaneLineBits[1].position = position;
		aPlaneLineBits[1].speed = TAXI_SPEED;
		aPlaneLineBits[1].acceleration = 33.0f/32.0f;
		time += 600.0f/((CRUISE_SPEED+TAXI_SPEED)/2.0f);
		position += 600.0f;

		// Fly at cruise speed
		aPlaneLineBits[2].type = 1;
		aPlaneLineBits[2].time = time;
		aPlaneLineBits[2].position = position;
		aPlaneLineBits[2].speed = CRUISE_SPEED;
		aPlaneLineBits[2].acceleration = 0.0f;
		dist = LandingPoint - TakeOffPoint;
		time += dist/CRUISE_SPEED;
		position += dist;

		// Brake after landing
		aPlaneLineBits[3].type = 2;
		aPlaneLineBits[3].time = time;
		aPlaneLineBits[3].position = position;
		aPlaneLineBits[3].speed = CRUISE_SPEED;
		aPlaneLineBits[3].acceleration = -33.0f/32.0f;
		time += 600.0f/((CRUISE_SPEED+TAXI_SPEED)/2.0f);
		position += 600.0f;

		// Taxi
		aPlaneLineBits[4].type = 1;
		aPlaneLineBits[4].time = time;
		aPlaneLineBits[4].position = position;
		aPlaneLineBits[4].speed = TAXI_SPEED;
		aPlaneLineBits[4].acceleration = 0.0f;
		time += (TotalLengthOfFlightPath - position)/TAXI_SPEED;

		// end
		aPlaneLineBits[5].time = time;
		TotalDurationOfFlightPath = time;
	}

	// Dodo
	if(pPath2Nodes == nil){
		pPath2Nodes = LoadPath("data\\paths\\flight2.dat", NumPath2Nodes, TotalLengthOfFlightPath2, true);
		TotalDurationOfFlightPath2 = TotalLengthOfFlightPath2/CRUISE_SPEED;
	}

/*
	// Mission Cesna
	if(pPath3Nodes == nil){
		pPath3Nodes = LoadPath("data\\paths\\flight3.dat", NumPath3Nodes, TotalLengthOfFlightPath3, false);
		TotalDurationOfFlightPath3 = TotalLengthOfFlightPath3/CRUISE_SPEED;
	}

	// Mission Cesna
	if(pPath4Nodes == nil){
		pPath4Nodes = LoadPath("data\\paths\\flight4.dat", NumPath4Nodes, TotalLengthOfFlightPath4, false);
		TotalDurationOfFlightPath4 = TotalLengthOfFlightPath4/CRUISE_SPEED;
	}
*/

	CStreaming::LoadAllRequestedModels(false);
	CStreaming::RequestModel(MI_AIRTRAIN, 0);
	CStreaming::LoadAllRequestedModels(false);

	for(i = 0; i < 3; i++){
		CPlane *plane = new CPlane(MI_AIRTRAIN, PERMANENT_VEHICLE);
		plane->GetMatrix().SetTranslate(0.0f, 0.0f, 0.0f);
		plane->SetStatus(STATUS_ABANDONED);
		plane->bIsLocked = true;
		plane->m_nPlaneId = i;
		plane->m_nCurPathNode = 0;
		CWorld::Add(plane);
	}


	CStreaming::RequestModel(MI_DEADDODO, 0);
	CStreaming::LoadAllRequestedModels(false);

	for(i = 0; i < 3; i++){
		CPlane *plane = new CPlane(MI_DEADDODO, PERMANENT_VEHICLE);
		plane->GetMatrix().SetTranslate(0.0f, 0.0f, 0.0f);
		plane->SetStatus(STATUS_ABANDONED);
		plane->bIsLocked = true;
		plane->m_nPlaneId = i;
		plane->m_nCurPathNode = 0;
		CWorld::Add(plane);
	}
}

void
CPlane::Shutdown(void)
{
	delete[] pPathNodes;
	delete[] pPath2Nodes;
	delete[] pPath3Nodes;
	delete[] pPath4Nodes;
	pPathNodes = nil;
	pPath2Nodes = nil;
	pPath3Nodes = nil;
	pPath4Nodes = nil;
}

CPlaneNode*
CPlane::LoadPath(char const *filename, int32 &numNodes, float &totalLength, bool loop)
{
	int bp, lp;
	int i;

	CFileMgr::LoadFile(filename, work_buff, sizeof(work_buff), "r");
	*gString = '\0';
	for(bp = 0, lp = 0; work_buff[bp] != '\n'; bp++, lp++)
		gString[lp] = work_buff[bp];
	bp++;
	gString[lp] = '\0';
	sscanf(gString, "%d", &numNodes);
	CPlaneNode *nodes = new CPlaneNode[numNodes];

	for(i = 0; i < numNodes; i++){
		*gString = '\0';
		for(lp = 0; work_buff[bp] != '\n' && work_buff[bp] != '\0'; bp++, lp++)
			gString[lp] = work_buff[bp];
		bp++;
		// BUG: game doesn't terminate string
		gString[lp] = '\0';
		sscanf(gString, "%f %f %f", &nodes[i].p.x, &nodes[i].p.y, &nodes[i].p.z);
	}

	// Calculate length of segments and path
	totalLength = 0.0f;
	for(i = 0; i < numNodes; i++){
		nodes[i].t = totalLength;
		float l = (nodes[(i+1) % numNodes].p - nodes[i].p).Magnitude2D();
		if(!loop && i == numNodes-1)
			l = 0.0f;
		totalLength += l;
	}

	return nodes;
}

void
CPlane::UpdatePlanes(void)
{
	int i, j;
	uint32 time;
	float t, deltaT;

	if(CReplay::IsPlayingBack())
		return;

	// Jumbo jets
	time = CTimer::GetTimeInMilliseconds();
	for(i = 0; i < 3; i++){
		t = TotalDurationOfFlightPath * (float)(time & 0x7FFFF)/0x80000;
		// find current frame
		for(j = 0; t > aPlaneLineBits[j+1].time; j++);

		OldPlanePathPosition[i] = PlanePathPosition[i];
		deltaT = t - aPlaneLineBits[j].time;
		switch(aPlaneLineBits[j].type){
		case 0:	// standing still
			PlanePathPosition[i] = aPlaneLineBits[j].position;
			PlanePathSpeed[i] = 0.0f;
			break;
		case 1:	// moving with constant speed
			PlanePathPosition[i] = aPlaneLineBits[j].position + aPlaneLineBits[j].speed*deltaT;
			PlanePathSpeed[i] = (TotalDurationOfFlightPath*1000.0f/0x80000) * aPlaneLineBits[j].speed;
			break;
		case 2:	// accelerating/braking
			PlanePathPosition[i] = aPlaneLineBits[j].position + (aPlaneLineBits[j].speed + aPlaneLineBits[j].acceleration*deltaT)*deltaT;
			PlanePathSpeed[i] = (TotalDurationOfFlightPath*1000.0f/0x80000)*aPlaneLineBits[j].speed + 2.0f*aPlaneLineBits[j].acceleration*deltaT;
			break;
		}

		// time offset for each plane
		time += 0x80000/3;
	}

	time = CTimer::GetTimeInMilliseconds();

	t = TotalDurationOfFlightPath2/0x80000;
	PlanePath2Position[0] = CRUISE_SPEED * (time & 0x7FFFF)*t;
	PlanePath2Position[1] = CRUISE_SPEED * ((time + 0x80000/3) & 0x7FFFF)*t;
	PlanePath2Position[2] = CRUISE_SPEED * ((time + 0x80000/3*2) & 0x7FFFF)*t;
	PlanePath2Speed[0] = CRUISE_SPEED*t;
	PlanePath2Speed[1] = CRUISE_SPEED*t;
	PlanePath2Speed[2] = CRUISE_SPEED*t;

	if(CesnaMissionStatus == CESNA_STATUS_FLYING){
		PlanePath3Speed = CRUISE_SPEED*TotalDurationOfFlightPath3/0x20000;
		PlanePath3Position = PlanePath3Speed * ((time - CesnaMissionStartTime) & 0x1FFFF);
		if(time - CesnaMissionStartTime >= 128072)
			CesnaMissionStatus = CESNA_STATUS_LANDED;
	}

	if(DropOffCesnaMissionStatus == CESNA_STATUS_FLYING){
		PlanePath4Speed = CRUISE_SPEED*TotalDurationOfFlightPath4/0x80000;
		PlanePath4Position = PlanePath4Speed * ((time - DropOffCesnaMissionStartTime) & 0x7FFFF);
		if(time - DropOffCesnaMissionStartTime >= 521288)
			DropOffCesnaMissionStatus = CESNA_STATUS_LANDED;
	}
}

bool
CPlane::TestRocketCollision(CVector *rocketPos)
{
	int i;

	i = CPools::GetVehiclePool()->GetSize();
	while(--i >= 0){
		CPlane *plane = (CPlane*)CPools::GetVehiclePool()->GetSlot(i);
		if(plane &&
#ifdef EXPLODING_AIRTRAIN
		   (plane->GetModelIndex() == MI_AIRTRAIN || plane->GetModelIndex() == MI_DEADDODO) &&
#else
		   plane->GetModelIndex() != MI_AIRTRAIN && plane->GetModelIndex() == MI_DEADDODO &&	// strange check
#endif
		   !plane->m_bHasBeenHit && (*rocketPos - plane->GetPosition()).Magnitude() < 25.0f){
			plane->m_nFrameWhenHit = CTimer::GetFrameCounter();
			plane->m_bHasBeenHit = true;
			CWorld::Players[CWorld::PlayerInFocus].m_pPed->m_pWanted->RegisterCrime_Immediately(CRIME_DESTROYED_CESSNA,
				plane->GetPosition(), i+1983, false);
			return true;
		}
	}
	return false;
}

// BUG: not in CPlane in the game
void
CPlane::CreateIncomingCesna(void)
{
	if(CesnaMissionStatus == CESNA_STATUS_FLYING){
		CWorld::Remove(pDrugRunCesna);
		delete pDrugRunCesna;
		pDrugRunCesna = nil;
	}
	pDrugRunCesna = new CPlane(MI_DEADDODO, PERMANENT_VEHICLE);
	pDrugRunCesna->GetMatrix().SetTranslate(0.0f, 0.0f, 0.0f);
	pDrugRunCesna->SetStatus(STATUS_ABANDONED);
	pDrugRunCesna->bIsLocked = true;
	pDrugRunCesna->m_nPlaneId = 0;
	pDrugRunCesna->m_nCurPathNode = 0;
	pDrugRunCesna->m_bIsDrugRunCesna = true;
	CWorld::Add(pDrugRunCesna);

        CesnaMissionStatus = CESNA_STATUS_FLYING;
        CesnaMissionStartTime = CTimer::GetTimeInMilliseconds();
        printf("CPlane::CreateIncomingCesna(void)\n");
}

void
CPlane::CreateDropOffCesna(void)
{
	if(DropOffCesnaMissionStatus == CESNA_STATUS_FLYING){
		CWorld::Remove(pDropOffCesna);
		delete pDropOffCesna;
		pDropOffCesna = nil;
	}
	pDropOffCesna = new CPlane(MI_DEADDODO, PERMANENT_VEHICLE);
	pDropOffCesna->GetMatrix().SetTranslate(0.0f, 0.0f, 0.0f);
	pDropOffCesna->SetStatus(STATUS_ABANDONED);
	pDropOffCesna->bIsLocked = true;
	pDropOffCesna->m_nPlaneId = 0;
	pDropOffCesna->m_nCurPathNode = 0;
	pDropOffCesna->m_bIsDropOffCesna = true;
	CWorld::Add(pDropOffCesna);

        DropOffCesnaMissionStatus = CESNA_STATUS_FLYING;
        DropOffCesnaMissionStartTime = CTimer::GetTimeInMilliseconds();
        printf("CPlane::CreateDropOffCesna(void)\n");
}

const CVector CPlane::FindDrugPlaneCoordinates(void) { return pDrugRunCesna->GetPosition(); }
const CVector CPlane::FindDropOffCesnaCoordinates(void) { return pDropOffCesna->GetPosition(); }
bool CPlane::HasCesnaLanded(void) { return CesnaMissionStatus == CESNA_STATUS_LANDED; }
bool CPlane::HasCesnaBeenDestroyed(void) { return CesnaMissionStatus == CESNA_STATUS_DESTROYED; }
bool CPlane::HasDropOffCesnaBeenShotDown(void) { return DropOffCesnaMissionStatus == CESNA_STATUS_DESTROYED; }
