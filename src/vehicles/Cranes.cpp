#include "common.h"

#include "Cranes.h"

#include "Camera.h"
#include "DMAudio.h"
#include "Garages.h"
#include "General.h"
#include "Entity.h"
#include "ModelIndices.h"
#include "Replay.h"
#include "Object.h"
#include "World.h"

#define MAX_DISTANCE_TO_FIND_CRANE (10.0f)
#define CRANE_UPDATE_RADIUS (300.0f)
#define CRANE_MOVEMENT_PROCESSING_RADIUS (150.0f)
#define CRUSHER_Z (-0.951f)
#define MILITARY_Z (10.7862f)
#define DISTANCE_FROM_PLAYER_TO_REMOVE_VEHICLE (5.0f)
#define DISTANCE_FROM_HOOK_TO_VEHICLE_TO_COLLECT (0.5f)
#define CAR_REWARD_MILITARY_CRANE (1500)
#define CAR_MOVING_SPEED_THRESHOLD (0.01f)
#define CRANE_SLOWDOWN_MULTIPLIER (0.3f)

#define OSCILLATION_SPEED (0.002f)
#define CAR_ROTATION_SPEED (0.0035f)
#define CRANE_MOVEMENT_SPEED (0.001f)
#define HOOK_ANGLE_MOVEMENT_SPEED (0.004f)
#define HOOK_OFFSET_MOVEMENT_SPEED (0.1f)
#define HOOK_HEIGHT_MOVEMENT_SPEED (0.06f)

#define MESSAGE_SHOW_DURATION (4000)

#define MAX_DISTANCE (99999.9f)
#define MIN_VALID_POSITION (-10000.0f)
#define DEFAULT_OFFSET (20.0f)

uint32 TimerForCamInterpolation;

uint32 CCranes::CarsCollectedMilitaryCrane;
int32 CCranes::NumCranes;
CCrane CCranes::aCranes[NUM_CRANES];

void CCranes::InitCranes(void)
{
	CarsCollectedMilitaryCrane = 0;
	NumCranes = 0;
	for (int i = 0; i < NUMSECTORS_X; i++) {
		for (int j = 0; j < NUMSECTORS_Y; j++) {
			for (CPtrNode* pNode = CWorld::GetSector(i, j)->m_lists[ENTITYLIST_BUILDINGS].first; pNode; pNode = pNode->next) {
				CEntity* pEntity = (CEntity*)pNode->item;
				if (MODELID_CRANE_1 == pEntity->GetModelIndex() ||
					MODELID_CRANE_2 == pEntity->GetModelIndex() ||
					MODELID_CRANE_3 == pEntity->GetModelIndex())
					AddThisOneCrane(pEntity);
			}
		}
	}
	for (CPtrNode* pNode = CWorld::GetBigBuildingList(LEVEL_INDUSTRIAL).first; pNode; pNode = pNode->next) {
		CEntity* pEntity = (CEntity*)pNode->item;
		if (MODELID_CRANE_1 == pEntity->GetModelIndex() ||
			MODELID_CRANE_2 == pEntity->GetModelIndex() ||
			MODELID_CRANE_3 == pEntity->GetModelIndex())
			AddThisOneCrane(pEntity);
	}
}

void CCranes::AddThisOneCrane(CEntity* pEntity)
{
	pEntity->GetMatrix().ResetOrientation();
	if (NumCranes >= NUM_CRANES)
		return;
	CCrane* pCrane = &aCranes[NumCranes];
	pCrane->Init();
	pCrane->m_pCraneEntity = (CBuilding*)pEntity;
	pCrane->m_nCraneStatus = CCrane::NONE;
	pCrane->m_fHookAngle = NumCranes; // lol wtf
	while (pCrane->m_fHookAngle > TWOPI)
		pCrane->m_fHookAngle -= TWOPI;
	pCrane->m_fHookOffset = DEFAULT_OFFSET;
	pCrane->m_fHookHeight = DEFAULT_OFFSET;
	pCrane->m_nTimeForNextCheck = 0;
	pCrane->m_nCraneState = CCrane::IDLE;
	pCrane->m_bWasMilitaryCrane = false;
	pCrane->m_nAudioEntity = DMAudio.CreateEntity(AUDIOTYPE_CRANE, &aCranes[NumCranes]);
	if (pCrane->m_nAudioEntity >= 0)
		DMAudio.SetEntityStatus(pCrane->m_nAudioEntity, true);
	pCrane->m_bIsTop = (MODELID_CRANE_1 != pEntity->GetModelIndex());
	// Is this used to avoid military crane?
	if (pCrane->m_bIsTop || pEntity->GetPosition().y > 0.0f) {
		CObject* pHook = new CObject(MI_MAGNET, false);
		pHook->ObjectCreatedBy = MISSION_OBJECT;
		pHook->bUsesCollision = false;
		pHook->bExplosionProof = true;
		pHook->bAffectedByGravity = false;
		pCrane->m_pHook = pHook;
		pCrane->CalcHookCoordinates(&pCrane->m_vecHookCurPos.x, &pCrane->m_vecHookCurPos.y, &pCrane->m_vecHookCurPos.z);
		pCrane->SetHookMatrix();
	}
	else
		pCrane->m_pHook = nil;
	NumCranes++;
}

void CCranes::ActivateCrane(float fInfX, float fSupX, float fInfY, float fSupY, float fDropOffX, float fDropOffY, float fDropOffZ, float fHeading, bool bIsCrusher, bool bIsMilitary, float fPosX, float fPosY)
{
	float fMinDistance = MAX_DISTANCE;
	float X = fPosX, Y = fPosY;
	if (X <= MIN_VALID_POSITION || Y <= MIN_VALID_POSITION) {
		X = fDropOffX;
		Y = fDropOffY;
	}
	int index = 0;
	for (int i = 0; i < NumCranes; i++) {
		float distance = (CVector2D(X, Y) - aCranes[i].m_pCraneEntity->GetPosition()).Magnitude();
		if (distance < fMinDistance && distance < MAX_DISTANCE_TO_FIND_CRANE) {
			fMinDistance = distance;
			index = i;
		}
	}
#ifdef FIX_BUGS // classic
	if (fMinDistance == MAX_DISTANCE)
		return;
#endif
	CCrane* pCrane = &aCranes[index];
	pCrane->m_fPickupX1 = fInfX;
	pCrane->m_fPickupX2 = fSupX;
	pCrane->m_fPickupY1 = fInfY;
	pCrane->m_fPickupY2 = fSupY;
	pCrane->m_vecDropoffTarget.x = fDropOffX;
	pCrane->m_vecDropoffTarget.y = fDropOffY;
	pCrane->m_vecDropoffTarget.z = fDropOffZ;
	pCrane->m_nCraneStatus = CCrane::ACTIVATED;
	pCrane->m_pVehiclePickedUp = nil;
	pCrane->m_nVehiclesCollected = 0;
	pCrane->m_fDropoffHeading = fHeading;
	pCrane->m_bIsCrusher = bIsCrusher;
	pCrane->m_bIsMilitaryCrane = bIsMilitary;
	bool military = true;
	if (!bIsMilitary && !pCrane->m_bWasMilitaryCrane)
		military = false;
	pCrane->m_bWasMilitaryCrane = military;
	pCrane->m_nTimeForNextCheck = 0;
	pCrane->m_nCraneState = CCrane::IDLE;
	float Z;
	if (bIsCrusher)
		Z = CRUSHER_Z;
	else if (bIsMilitary)
		Z = MILITARY_Z;
	else
		Z = CWorld::FindGroundZForCoord((fInfX + fSupX) / 2, (fInfY + fSupY) / 2);
	pCrane->FindParametersForTarget((fInfX + fSupX) / 2, (fInfY + fSupY) / 2, Z, &pCrane->m_fPickupAngle, &pCrane->m_fPickupDistance, &pCrane->m_fPickupHeight);
	pCrane->FindParametersForTarget(fDropOffX, fDropOffY, fDropOffZ, &pCrane->m_fDropoffAngle, &pCrane->m_fDropoffDistance, &pCrane->m_fDropoffHeight);
}

void CCranes::DeActivateCrane(float X, float Y)
{
	float fMinDistance = MAX_DISTANCE;
	int index = 0;
	for (int i = 0; i < NumCranes; i++) {
		float distance = (CVector2D(X, Y) - aCranes[i].m_pCraneEntity->GetPosition()).Magnitude();
		if (distance < fMinDistance && distance < MAX_DISTANCE_TO_FIND_CRANE) {
			fMinDistance = distance;
			index = i;
		}
	}
#ifdef FIX_BUGS // classic
	if (fMinDistance == MAX_DISTANCE)
		return;
#endif
	aCranes[index].m_nCraneStatus = CCrane::DEACTIVATED;
	aCranes[index].m_nCraneState = CCrane::IDLE;
}

bool CCranes::IsThisCarPickedUp(float X, float Y, CVehicle* pVehicle)
{
	int index = 0;
	bool result = false;
	for (int i = 0; i < NumCranes; i++) {
		float distance = (CVector2D(X, Y) - aCranes[i].m_pCraneEntity->GetPosition()).Magnitude();
		if (distance < MAX_DISTANCE_TO_FIND_CRANE && aCranes[i].m_pVehiclePickedUp == pVehicle) {
			if (aCranes[i].m_nCraneState == CCrane::LIFTING_TARGET || aCranes[i].m_nCraneState == CCrane::ROTATING_TARGET)
				result = true;
		}
	}
	return result;
}

void CCranes::UpdateCranes(void)
{
	for (int i = 0; i < NumCranes; i++) {
		if (aCranes[i].m_bIsTop || aCranes[i].m_bIsCrusher ||
			(TheCamera.GetPosition().x + CRANE_UPDATE_RADIUS > aCranes[i].m_pCraneEntity->GetPosition().x &&
				TheCamera.GetPosition().x - CRANE_UPDATE_RADIUS < aCranes[i].m_pCraneEntity->GetPosition().x &&
				TheCamera.GetPosition().y + CRANE_UPDATE_RADIUS > aCranes[i].m_pCraneEntity->GetPosition().y &&
				TheCamera.GetPosition().y + CRANE_UPDATE_RADIUS < aCranes[i].m_pCraneEntity->GetPosition().y))
			aCranes[i].Update();
	}
}

void CCrane::Update(void)
{
	if (CReplay::IsPlayingBack())
		return;
	if (((m_nCraneStatus == ACTIVATED || m_nCraneStatus == DEACTIVATED) &&
		Abs(TheCamera.GetGameCamPosition().x - m_pCraneEntity->GetPosition().x) < CRANE_MOVEMENT_PROCESSING_RADIUS &&
		Abs(TheCamera.GetGameCamPosition().y - m_pCraneEntity->GetPosition().y) < CRANE_MOVEMENT_PROCESSING_RADIUS) ||
		m_nCraneState != IDLE) {
		switch (m_nCraneState) {
		case IDLE:
			if (GoTowardsTarget(m_fPickupAngle, m_fPickupDistance, GetHeightToPickup()) &&
				CTimer::GetTimeInMilliseconds() > m_nTimeForNextCheck) {
				CWorld::AdvanceCurrentScanCode();
#ifdef FIX_BUGS
				int xstart = Max(0, CWorld::GetSectorIndexX(m_fPickupX1));
				int xend = Min(NUMSECTORS_X - 1, CWorld::GetSectorIndexX(m_fPickupX2));
				int ystart = Max(0, CWorld::GetSectorIndexY(m_fPickupY1));
				int yend = Min(NUMSECTORS_Y - 1, CWorld::GetSectorIndexY(m_fPickupY2));
#else
				int xstart = CWorld::GetSectorIndexX(m_fPickupX1);
				int xend = CWorld::GetSectorIndexX(m_fPickupX2);
				int ystart = CWorld::GetSectorIndexY(m_fPickupY1);
				int yend = CWorld::GetSectorIndexY(m_fPickupY1);
#endif
				assert(xstart <= xend);
				assert(ystart <= yend);
				for (int i = xstart; i <= xend; i++) {
					for (int j = ystart; j <= yend; j++) {
						FindCarInSectorList(&CWorld::GetSector(i, j)->m_lists[ENTITYLIST_VEHICLES]);
						FindCarInSectorList(&CWorld::GetSector(i, j)->m_lists[ENTITYLIST_VEHICLES_OVERLAP]);
					}
				}
			}
			break;
		case GOING_TOWARDS_TARGET:
			if (m_pVehiclePickedUp){
				if (m_pVehiclePickedUp->GetPosition().x < m_fPickupX1 ||
					m_pVehiclePickedUp->GetPosition().x > m_fPickupX2 ||
					m_pVehiclePickedUp->GetPosition().y < m_fPickupY1 ||
					m_pVehiclePickedUp->GetPosition().y > m_fPickupY2 ||
					m_pVehiclePickedUp->pDriver ||
					Abs(m_pVehiclePickedUp->GetMoveSpeed().x) > CAR_MOVING_SPEED_THRESHOLD ||
					Abs(m_pVehiclePickedUp->GetMoveSpeed().y) > CAR_MOVING_SPEED_THRESHOLD ||
					Abs(m_pVehiclePickedUp->GetMoveSpeed().z) > CAR_MOVING_SPEED_THRESHOLD ||
					(FindPlayerPed()->GetPedState() == PED_ENTER_CAR
#ifdef FIX_BUGS
					|| FindPlayerPed()->GetPedState() == PED_CARJACK
#endif
					) && FindPlayerPed()->m_pSeekTarget == m_pVehiclePickedUp) {
					m_pVehiclePickedUp = nil;
					m_nCraneState = IDLE;
				}
				else {
					float fAngle, fOffset, fHeight;
					FindParametersForTarget(
						m_pVehiclePickedUp->GetPosition().x,
						m_pVehiclePickedUp->GetPosition().y,
						m_pVehiclePickedUp->GetPosition().z + m_pVehiclePickedUp->GetColModel()->boundingBox.max.z,
						&fAngle, &fOffset, &fHeight);
					if (GoTowardsTarget(fAngle, fOffset, fHeight)) {
						CVector distance = m_pVehiclePickedUp->GetPosition() - m_vecHookCurPos;
						distance.z += m_pVehiclePickedUp->GetColModel()->boundingBox.max.z;
						if (distance.MagnitudeSqr() < SQR(DISTANCE_FROM_HOOK_TO_VEHICLE_TO_COLLECT)) {
							m_nCraneState = GOING_TOWARDS_TARGET_ONLY_HEIGHT;
							m_vecHookVelocity *= 0.4f;
							m_pVehiclePickedUp->bLightsOn = false;
							m_pVehiclePickedUp->bUsesCollision = false;
							if (m_bIsCrusher)
								m_pVehiclePickedUp->bCollisionProof = true;
							DMAudio.PlayOneShot(m_nAudioEntity, SOUND_CRANE_PICKUP, 0.0f);
						}
					}
				}
			}
			else
				m_nCraneState = IDLE;
			break;
		case LIFTING_TARGET:
			RotateCarriedCarProperly();
			if (GoTowardsTarget(m_fDropoffAngle, m_fDropoffDistance, GetHeightToDropoff(), CRANE_SLOWDOWN_MULTIPLIER))
				m_nCraneState = ROTATING_TARGET;
			if (!m_pVehiclePickedUp || m_pVehiclePickedUp->pDriver) {
				m_pVehiclePickedUp = nil;
				m_nCraneState = IDLE;
			}
			break;
		case GOING_TOWARDS_TARGET_ONLY_HEIGHT:
			RotateCarriedCarProperly();
			if (GoTowardsHeightTarget(GetHeightToPickupHeight(), CRANE_SLOWDOWN_MULTIPLIER))
				m_nCraneState = LIFTING_TARGET;
			TimerForCamInterpolation = CTimer::GetTimeInMilliseconds();
			if (!m_pVehiclePickedUp || m_pVehiclePickedUp->pDriver) {
				m_pVehiclePickedUp = nil;
				m_nCraneState = IDLE;
			}
			break;
		case ROTATING_TARGET:
		{
			bool bRotateFinished = RotateCarriedCarProperly();
			bool bMovementFinished = GoTowardsTarget(m_fDropoffAngle, m_fDropoffDistance, m_fDropoffHeight, 0.3f);
			if (bMovementFinished && bRotateFinished) {
				float fDistanceFromPlayer = m_pVehiclePickedUp ? ((CVector2D)FindPlayerCoors() - (CVector2D)m_pVehiclePickedUp->GetPosition()).Magnitude() : 0.0f;
				if (fDistanceFromPlayer > DISTANCE_FROM_PLAYER_TO_REMOVE_VEHICLE || !m_bWasMilitaryCrane) {
					m_nCraneState = DROPPING_TARGET;
					if (m_pVehiclePickedUp) {
						m_pVehiclePickedUp->bUsesCollision = true;
						m_pVehiclePickedUp->m_nStaticFrames = 0;
						++m_nVehiclesCollected;
						if (m_bIsMilitaryCrane) {
							CCranes::RegisterCarForMilitaryCrane(m_pVehiclePickedUp->GetModelIndex());
							if (!CCranes::HaveAllCarsBeenCollectedByMilitaryCrane()) {
								CWorld::Players[CWorld::PlayerInFocus].m_nMoney += CAR_REWARD_MILITARY_CRANE;
								CGarages::TriggerMessage("GA_10", CAR_REWARD_MILITARY_CRANE, MESSAGE_SHOW_DURATION, -1);
							}
							CWorld::Remove(m_pVehiclePickedUp);
							delete m_pVehiclePickedUp;
						}
					}
					m_pVehiclePickedUp = nil;
				}
			}
			break;
		}
		case DROPPING_TARGET:
			if (GoTowardsTarget(m_fDropoffAngle, m_fDropoffDistance, GetHeightToDropoffHeight(), CRANE_SLOWDOWN_MULTIPLIER)) {
				m_nCraneState = IDLE;
				m_nTimeForNextCheck = CTimer::GetTimeInMilliseconds() + 10000;
			}
			break;
		default:
			break;
		}
		CVector vecHook;
		CalcHookCoordinates(&vecHook.x, &vecHook.y, &vecHook.z);
		m_vecHookVelocity += ((CVector2D)vecHook - (CVector2D)m_vecHookCurPos) * CTimer::GetTimeStep() * CRANE_MOVEMENT_SPEED;
		m_vecHookVelocity *= Pow(0.98f, CTimer::GetTimeStep());
		m_vecHookCurPos.x += m_vecHookVelocity.x * CTimer::GetTimeStep();
		m_vecHookCurPos.y += m_vecHookVelocity.y * CTimer::GetTimeStep();
		m_vecHookCurPos.z = vecHook.z;
		switch (m_nCraneState) {
		case LIFTING_TARGET:
		case GOING_TOWARDS_TARGET_ONLY_HEIGHT:
		case ROTATING_TARGET:
			if (m_pVehiclePickedUp) {
				m_pVehiclePickedUp->SetPosition(m_vecHookCurPos.x, m_vecHookCurPos.y, m_vecHookCurPos.z - m_pVehiclePickedUp->GetColModel()->boundingBox.max.z);
				m_pVehiclePickedUp->SetMoveSpeed(0.0f, 0.0f, 0.0f);
				CVector up(vecHook.x - m_vecHookCurPos.x, vecHook.y - m_vecHookCurPos.y, 20.0f);
				up.Normalise();
				m_pVehiclePickedUp->GetRight() = CrossProduct(m_pVehiclePickedUp->GetForward(), up);
				m_pVehiclePickedUp->GetForward() = CrossProduct(up, m_pVehiclePickedUp->GetRight());
				m_pVehiclePickedUp->GetUp() = up;
			}
			break;
		default:
			break;
		}
	}
	else {
		int16 rnd = (m_pCraneEntity->m_randomSeed + (CTimer::GetTimeInMilliseconds() >> 11)) & 0xF;
		// 16 options, lasting 2048 ms each
		// a bit awkward: why there are 4 periods for -= and 6 for +=? is it a bug?
		if (rnd < 4) {
			m_fHookAngle -= OSCILLATION_SPEED * CTimer::GetTimeStep();
			if (m_fHookAngle < 0.0f)
				m_fHookAngle += TWOPI;
		}
		else if (rnd > 5 && rnd < 12) {
			m_fHookAngle += OSCILLATION_SPEED * CTimer::GetTimeStep();
			if (m_fHookAngle > TWOPI)
				m_fHookAngle -= TWOPI;
		}
		CalcHookCoordinates(&m_vecHookCurPos.x, &m_vecHookCurPos.y, &m_vecHookCurPos.z);
		m_vecHookVelocity.x = m_vecHookVelocity.y = 0.0f;
	}
	float fCos = Cos(m_fHookAngle);
	float fSin = Sin(m_fHookAngle);
	m_pCraneEntity->GetRight().x = fCos;
	m_pCraneEntity->GetForward().y = fCos;
	m_pCraneEntity->GetRight().y = fSin;
	m_pCraneEntity->GetForward().x = -fSin;
	m_pCraneEntity->GetMatrix().UpdateRW();
	m_pCraneEntity->UpdateRwFrame();
	SetHookMatrix();
}

bool CCrane::RotateCarriedCarProperly()
{
	if (m_fDropoffHeading <= 0.0f)
		return true;
	if (!m_pVehiclePickedUp)
		return true;
	float fAngleDelta = m_fDropoffHeading - CGeneral::GetATanOfXY(m_pVehiclePickedUp->GetForward().x, m_pVehiclePickedUp->GetForward().y);
	while (fAngleDelta < -HALFPI)
		fAngleDelta += PI;
	while (fAngleDelta > HALFPI)
		fAngleDelta -= PI;
	float fDeltaThisFrame = CAR_ROTATION_SPEED * CTimer::GetTimeStep();
	if (Abs(fAngleDelta) <= fDeltaThisFrame) // no rotation is actually applied?
		return true;
	m_pVehiclePickedUp->GetMatrix().RotateZ(fAngleDelta < 0 ? -fDeltaThisFrame : fDeltaThisFrame);
	return false;
}

void CCrane::FindCarInSectorList(CPtrList* pList)
{
	CPtrNode* node;
	for (node = pList->first; node; node = node->next) {
		CVehicle* pVehicle = (CVehicle*)node->item;
		if (pVehicle->m_scanCode == CWorld::GetCurrentScanCode())
			continue;
		pVehicle->m_scanCode = CWorld::GetCurrentScanCode();
		if (pVehicle->GetPosition().x < m_fPickupX1 || pVehicle->GetPosition().x > m_fPickupX2 ||
			pVehicle->GetPosition().y < m_fPickupY1 || pVehicle->GetPosition().y > m_fPickupY2)
			continue;
		if (pVehicle->pDriver)
			continue;
		if (Abs(pVehicle->GetMoveSpeed().x) >= CAR_MOVING_SPEED_THRESHOLD ||
			Abs(pVehicle->GetMoveSpeed().y) >= CAR_MOVING_SPEED_THRESHOLD ||
			Abs(pVehicle->GetMoveSpeed().z) >= CAR_MOVING_SPEED_THRESHOLD)
			continue;
		if (!pVehicle->IsCar() || pVehicle->GetStatus() == STATUS_WRECKED || pVehicle->m_fHealth < 250.0f)
			continue;
		if (!DoesCranePickUpThisCarType(pVehicle->GetModelIndex()) ||
			m_bIsMilitaryCrane && CCranes::DoesMilitaryCraneHaveThisOneAlready(pVehicle->GetModelIndex())) {
			if (!pVehicle->bCraneMessageDone) {
				pVehicle->bCraneMessageDone = true;
				if (!m_bIsMilitaryCrane)
					CGarages::TriggerMessage("CR_1", -1, MESSAGE_SHOW_DURATION, -1); // Crane cannot lift this vehicle.
				else if (DoesCranePickUpThisCarType(pVehicle->GetModelIndex()))
					CGarages::TriggerMessage("GA_20", -1, MESSAGE_SHOW_DURATION, -1); // We got more of these than we can shift. Sorry man, no deal.
				else
					CGarages::TriggerMessage("GA_19", -1, MESSAGE_SHOW_DURATION, -1); // We're not interested in that model.
			}
		}
		else {
			m_pVehiclePickedUp = pVehicle;
			pVehicle->RegisterReference((CEntity**)&m_pVehiclePickedUp);
			m_nCraneState = GOING_TOWARDS_TARGET;
		}
	}
}

bool CCrane::DoesCranePickUpThisCarType(uint32 mi)
{
	if (m_bIsCrusher) {
		return mi != MI_FIRETRUCK &&
			mi != MI_TRASH &&
#ifdef FIX_BUGS
			mi != MI_COACH &&
#else
			mi != MI_BLISTA &&
#endif
			mi != MI_SECURICA &&
			mi != MI_BUS &&
			mi != MI_DODO &&
			mi != MI_RHINO;
	}
	if (m_bIsMilitaryCrane) {
		return mi == MI_FIRETRUCK ||
			mi == MI_AMBULAN ||
			mi == MI_ENFORCER ||
			mi == MI_FBICAR ||
			mi == MI_RHINO ||
			mi == MI_BARRACKS ||
			mi == MI_POLICE;
	}
	return true;
}

bool CCranes::DoesMilitaryCraneHaveThisOneAlready(uint32 mi)
{
	switch (mi) {
	case MI_FIRETRUCK: return (CarsCollectedMilitaryCrane & 1);
	case MI_AMBULAN: return (CarsCollectedMilitaryCrane & 2);
	case MI_ENFORCER: return (CarsCollectedMilitaryCrane & 4);
	case MI_FBICAR: return (CarsCollectedMilitaryCrane & 8);
	case MI_RHINO: return (CarsCollectedMilitaryCrane & 0x10);
	case MI_BARRACKS: return (CarsCollectedMilitaryCrane & 0x20);
	case MI_POLICE: return (CarsCollectedMilitaryCrane & 0x40);
	default: break;
	}
	return false;
}

void CCranes::RegisterCarForMilitaryCrane(uint32 mi)
{
	switch (mi) {
	case MI_FIRETRUCK: CarsCollectedMilitaryCrane |= 1; break;
	case MI_AMBULAN: CarsCollectedMilitaryCrane |= 2; break;
	case MI_ENFORCER: CarsCollectedMilitaryCrane |= 4; break;
	case MI_FBICAR: CarsCollectedMilitaryCrane |= 8; break;
	case MI_RHINO: CarsCollectedMilitaryCrane |= 0x10; break;
	case MI_BARRACKS: CarsCollectedMilitaryCrane |= 0x20; break;
	case MI_POLICE: CarsCollectedMilitaryCrane |= 0x40; break;
	default: break;
	}
}

bool CCranes::HaveAllCarsBeenCollectedByMilitaryCrane()
{
	return (CarsCollectedMilitaryCrane & 0x7F) == 0x7F;
}

bool CCrane::GoTowardsTarget(float fAngleToTarget, float fDistanceToTarget, float fTargetHeight, float fSpeedMultiplier)
{
	bool bAngleMovementFinished, bOffsetMovementFinished, bHeightMovementFinished;
	float fHookAngleDelta = fAngleToTarget - m_fHookAngle;
	while (fHookAngleDelta > PI)
		fHookAngleDelta -= TWOPI;
	while (fHookAngleDelta < -PI)
		fHookAngleDelta += TWOPI;
	float fHookAngleChangeThisFrame = fSpeedMultiplier * CTimer::GetTimeStep() * HOOK_ANGLE_MOVEMENT_SPEED;
	if (Abs(fHookAngleDelta) < fHookAngleChangeThisFrame) {
		m_fHookAngle = fAngleToTarget;
		bAngleMovementFinished = true;
	} else {
		if (fHookAngleDelta < 0.0f) {
			m_fHookAngle -= fHookAngleChangeThisFrame;
			if (m_fHookAngle < 0.0f)
				m_fHookAngle += TWOPI;
		}
		else {
			m_fHookAngle += fHookAngleChangeThisFrame;
			if (m_fHookAngle > TWOPI)
				m_fHookAngle -= TWOPI;
		}
		bAngleMovementFinished = false;
	}
	float fHookOffsetDelta = fDistanceToTarget - m_fHookOffset;
	float fHookOffsetChangeThisFrame = fSpeedMultiplier * CTimer::GetTimeStep() * HOOK_OFFSET_MOVEMENT_SPEED;
	if (Abs(fHookOffsetDelta) < fHookOffsetChangeThisFrame) {
		m_fHookOffset = fDistanceToTarget;
		bOffsetMovementFinished = true;
	} else {
		if (fHookOffsetDelta < 0.0f)
			m_fHookOffset -= fHookOffsetChangeThisFrame;
		else
			m_fHookOffset += fHookOffsetChangeThisFrame;
		bOffsetMovementFinished = false;
	}
	float fHookHeightDelta = fTargetHeight - m_fHookHeight;
	float fHookHeightChangeThisFrame = fSpeedMultiplier * CTimer::GetTimeStep() * HOOK_HEIGHT_MOVEMENT_SPEED;
	if (Abs(fHookHeightDelta) < fHookHeightChangeThisFrame) {
		m_fHookHeight = fTargetHeight;
		bHeightMovementFinished = true;
	} else {
		if (fHookHeightDelta < 0.0f)
			m_fHookHeight -= fHookHeightChangeThisFrame;
		else
			m_fHookHeight += fHookHeightChangeThisFrame;
		bHeightMovementFinished = false;
	}
	return bAngleMovementFinished && bOffsetMovementFinished && bHeightMovementFinished;
}

bool CCrane::GoTowardsHeightTarget(float fTargetHeight, float fSpeedMultiplier)
{
	bool bHeightMovementFinished;
	float fHookHeightDelta = fTargetHeight - m_fHookHeight;
	float fHookHeightChangeThisFrame = fSpeedMultiplier * CTimer::GetTimeStep() * HOOK_HEIGHT_MOVEMENT_SPEED;
	if (Abs(fHookHeightDelta) < fHookHeightChangeThisFrame) {
		m_fHookHeight = fTargetHeight;
		bHeightMovementFinished = true;
	} else {
		if (fHookHeightDelta < 0.0f)
			m_fHookHeight -= fHookHeightChangeThisFrame;
		else
			m_fHookHeight += fHookHeightChangeThisFrame;
		bHeightMovementFinished = false;
	}
	return bHeightMovementFinished;
}

void CCrane::FindParametersForTarget(float X, float Y, float Z, float* pAngle, float* pDistance, float* pHeight)
{
	*pAngle = CGeneral::GetATanOfXY(X - m_pCraneEntity->GetPosition().x, Y - m_pCraneEntity->GetPosition().y);
	*pDistance = ((CVector2D(X, Y) - (CVector2D)m_pCraneEntity->GetPosition())).Magnitude();
	*pHeight = Z;
}

void CCrane::CalcHookCoordinates(float* pX, float* pY, float* pZ)
{
	*pX = Cos(m_fHookAngle) * m_fHookOffset + m_pCraneEntity->GetPosition().x;
	*pY = Sin(m_fHookAngle) * m_fHookOffset + m_pCraneEntity->GetPosition().y;
	*pZ = m_fHookHeight;
}

void CCrane::SetHookMatrix()
{
	if (m_pHook == nil)
		return;
	m_pHook->SetPosition(m_vecHookCurPos);
	CVector up(m_vecHookInitPos.x - m_vecHookCurPos.x, m_vecHookInitPos.y - m_vecHookCurPos.y, 20.0f);
	up.Normalise();
	m_pHook->GetRight() = CrossProduct(CVector(0.0f, 1.0f, 0.0f), up);
	m_pHook->GetForward() = CrossProduct(up, m_pHook->GetRight());
	m_pHook->GetUp() = up;
	m_pHook->SetOrientation(0.0f, 0.0f, -HALFPI);
	m_pHook->GetMatrix().UpdateRW();
	m_pHook->UpdateRwFrame();
	CWorld::Remove(m_pHook);
	CWorld::Add(m_pHook);
}

bool CCranes::IsThisCarBeingCarriedByAnyCrane(CVehicle* pVehicle)
{
	for (int i = 0; i < NumCranes; i++) {
		if (pVehicle == aCranes[i].m_pVehiclePickedUp) {
			switch (aCranes[i].m_nCraneState) {
			case CCrane::GOING_TOWARDS_TARGET_ONLY_HEIGHT:
			case CCrane::LIFTING_TARGET:
			case CCrane::ROTATING_TARGET:
				return true;
			default:
				break;
			}
		}
	}
	return false;
}

bool CCranes::IsThisCarBeingTargettedByAnyCrane(CVehicle* pVehicle)
{
	for (int i = 0; i < NumCranes; i++) {
		if (pVehicle == aCranes[i].m_pVehiclePickedUp)
			return true;
	}
	return false;
}

void CCranes::Save(uint8* buf, uint32* size)
{
	INITSAVEBUF

	*size = 2 * sizeof(uint32) + sizeof(aCranes);
	WriteSaveBuf(buf, NumCranes);
	WriteSaveBuf(buf, CarsCollectedMilitaryCrane);
	for (int i = 0; i < NUM_CRANES; i++) {
		CCrane *pCrane = WriteSaveBuf(buf, aCranes[i]);
		if (pCrane->m_pCraneEntity != nil)
			pCrane->m_pCraneEntity = (CBuilding*)(CPools::GetBuildingPool()->GetJustIndex_NoFreeAssert(pCrane->m_pCraneEntity) + 1);
		if (pCrane->m_pHook != nil)
			pCrane->m_pHook = (CObject*)(CPools::GetObjectPool()->GetJustIndex_NoFreeAssert(pCrane->m_pHook) + 1);
		if (pCrane->m_pVehiclePickedUp != nil)
			pCrane->m_pVehiclePickedUp = (CVehicle*)(CPools::GetVehiclePool()->GetJustIndex_NoFreeAssert(pCrane->m_pVehiclePickedUp) + 1);
	}

	VALIDATESAVEBUF(*size);
}

void CCranes::Load(uint8* buf, uint32 size)
{
	INITSAVEBUF

	NumCranes = ReadSaveBuf<int32>(buf);
	CarsCollectedMilitaryCrane = ReadSaveBuf<uint32>(buf);
	for (int i = 0; i < NUM_CRANES; i++)
		aCranes[i] = ReadSaveBuf<CCrane>(buf);
	for (int i = 0; i < NUM_CRANES; i++) {
		CCrane *pCrane = &aCranes[i];
		if (pCrane->m_pCraneEntity != nil)
			pCrane->m_pCraneEntity = CPools::GetBuildingPool()->GetSlot((uintptr)pCrane->m_pCraneEntity - 1);
		if (pCrane->m_pHook != nil)
			pCrane->m_pHook = CPools::GetObjectPool()->GetSlot((uintptr)pCrane->m_pHook - 1);
		if (pCrane->m_pVehiclePickedUp != nil)
			pCrane->m_pVehiclePickedUp = CPools::GetVehiclePool()->GetSlot((uintptr)pCrane->m_pVehiclePickedUp - 1);
	}
	for (int i = 0; i < NUM_CRANES; i++) {
		aCranes[i].m_nAudioEntity = DMAudio.CreateEntity(AUDIOTYPE_CRANE, &aCranes[i]);
		if (aCranes[i].m_nAudioEntity != 0)
			DMAudio.SetEntityStatus(aCranes[i].m_nAudioEntity, true);
	}

	VALIDATESAVEBUF(size);
}
