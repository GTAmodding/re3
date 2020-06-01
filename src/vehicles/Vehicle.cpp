#include "common.h"
#include "main.h"

#include "General.h"
#include "Timer.h"
#include "Pad.h"
#include "Vehicle.h"
#include "Pools.h"
#include "HandlingMgr.h"
#include "CarCtrl.h"
#include "Population.h"
#include "ModelIndices.h"
#include "World.h"
#include "Lights.h"
#include "PointLights.h"
#include "Renderer.h"
#include "VisibilityPlugins.h"
#include "DMAudio.h"
#include "Radar.h"
#include "Fire.h"
#include "Darkel.h"
#include "Streaming.h"
#include "Camera.h"
#include "Stats.h"
#include "Garages.h"
#include "Wanted.h"
#include "SurfaceTable.h"
#include "Particle.h"
#include "WaterLevel.h"
#include "Timecycle.h"
#include "Weather.h"
#include "Coronas.h"

//--MIAMI: done

bool CVehicle::bWheelsOnlyCheat;
bool CVehicle::bAllDodosCheat;
bool CVehicle::bCheat3;
bool CVehicle::bCheat4;
bool CVehicle::bCheat5;
#ifdef ALT_DODO_CHEAT
bool CVehicle::bAltDodoCheat;
#endif
bool CVehicle::bHoverCheat;
bool CVehicle::bAllTaxisHaveNitro;
bool CVehicle::m_bDisableMouseSteering = true;
bool CVehicle::bDisableRemoteDetonation;
bool CVehicle::bDisableRemoteDetonationOnContact;

void *CVehicle::operator new(size_t sz) { return CPools::GetVehiclePool()->New();  }
void *CVehicle::operator new(size_t sz, int handle) { return CPools::GetVehiclePool()->New(handle); }
void CVehicle::operator delete(void *p, size_t sz) { CPools::GetVehiclePool()->Delete((CVehicle*)p); }
void CVehicle::operator delete(void *p, int handle) { CPools::GetVehiclePool()->Delete((CVehicle*)p); }

#ifdef FIX_BUGS
// I think they meant that
#define DAMAGE_FLEE_IN_CAR_PROBABILITY_VALUE (MYRAND_MAX * 35 / 100)
#define DAMAGE_FLEE_ON_FOOT_PROBABILITY_VALUE (MYRAND_MAX * 70 / 100)
#else
#define DAMAGE_FLEE_IN_CAR_PROBABILITY_VALUE (35000)
#define DAMAGE_FLEE_ON_FOOT_PROBABILITY_VALUE (70000)
#endif
#define DAMAGE_HEALTH_TO_FLEE_ALWAYS (200.0f)
#define DAMAGE_HEALTH_TO_CATCH_FIRE (250.0f)


CVehicle::CVehicle(uint8 CreatedBy)
{
	int i;

	m_nCurrentGear = 1;
	m_fChangeGearTime = 0.0f;
	m_fSteerRatio = 0.0f;
	m_type = ENTITY_TYPE_VEHICLE;
	VehicleCreatedBy = CreatedBy;
	m_nRouteSeed = 0;
	bIsLocked = false;
	bIsLawEnforcer = false;
	bIsAmbulanceOnDuty = false;
	bIsFireTruckOnDuty = false;
	CCarCtrl::UpdateCarCount(this, false);
	m_fHealth = 1000.0f;
	bEngineOn = true;
	bFreebies = true;
	pDriver = nil;
	m_nNumPassengers = 0;
	m_nNumGettingIn = 0;
	m_nGettingInFlags = 0;
	m_nGettingOutFlags = 0;
	m_nNumMaxPassengers = ARRAY_SIZE(pPassengers);
	for(i = 0; i < m_nNumMaxPassengers; i++)
		pPassengers[i] = nil;
	m_nBombTimer = 0;
	m_pBlowUpEntity = nil;
	m_nPacManPickupsCarried = 0;
	bComedyControls = false;
	bCraneMessageDone = false;
	bExtendedRange = false;
	bTakeLessDamage = false;
	bIsDamaged = false;
	bFadeOut = false;
	bIsBeingCarJacked = false;
	m_nTimeOfDeath = 0;
	m_pCarFire = nil;
	bHasBeenOwnedByPlayer = false;
	bCreateRoadBlockPeds = false;
	bCanBeDamaged = true;
	bUsingSpecialColModel = false;
	bOccupantsHaveBeenGenerated = false;
	bGunSwitchedOff = false;
	m_nGunFiringTime = 0;
	m_nTimeBlocked = 0;
	bLightsOn = false;
	bVehicleColProcessed = false;
	m_numPedsUseItAsCover = 0;
	bIsCarParkVehicle = false;
	bHasAlreadyBeenRecorded = false;
	m_bSirenOrAlarm = 0;
	m_nCarHornTimer = 0;
	m_nCarHornPattern = 0;
	m_nCarHornDelay = 0;
	bPartOfConvoy = false;
	bHeliMinimumTilt = false;
	bAudioChangingGear = false;
	bIsDrowning = false;
	bTyresDontBurst = false;
	bCreatedAsPoliceVehicle = false;
	bRestingOnPhysical = false;
	bParking = false;
	bCanPark = CGeneral::GetRandomNumberInRange(0.0f, 1.0f) < 0.0f;	// never true. probably doesn't work very well
	bIsVan = false;
	bIsBus = false;
	bIsBig = false;
	bLowVehicle = false;

	m_bombType = CARBOMB_NONE;
	bDriverLastFrame = false;
	m_pBombRigger = nil;

	m_nSetPieceExtendedRangeTime = 0;
	m_nAlarmState = 0;
	m_nDoorLock = CARLOCK_UNLOCKED;
	m_nLastWeaponDamage = -1;
	m_fMapObjectHeightAhead = m_fMapObjectHeightBehind = 0.0f;
	m_audioEntityId = DMAudio.CreateEntity(AUDIOTYPE_PHYSICAL, this);
	if(m_audioEntityId >= 0)
		DMAudio.SetEntityStatus(m_audioEntityId, true);
	//m_nRadioStation = CGeneral::GetRandomNumber() % USERTRACK;
	switch(GetModelIndex()){
	case MI_HUNTER:
	case MI_ANGEL:
	case MI_FREEWAY:
		m_nRadioStation = V_ROCK;
		break;
	case MI_RCBARON:
	case MI_RCBANDIT:
	case MI_RCRAIDER:
	case MI_RCGOBLIN:
	case MI_TOPFUN:
	case MI_CADDY:
	case MI_BAGGAGE:
		m_nRadioStation = RADIO_OFF;
		break;
	default:
		m_nRadioStation = CGeneral::GetRandomNumber() % USERTRACK;
		break;
	}
	m_pCurGroundEntity = nil;
	m_bRainAudioCounter = 0;
	m_bRainSamplesCounter = 0;
	m_comedyControlState = 0;
	m_aCollPolys[0].valid = false;
	m_aCollPolys[1].valid = false;
	AutoPilot.m_nCarMission = MISSION_NONE;
	AutoPilot.m_nTempAction = TEMPACT_NONE;
	AutoPilot.m_nTimeToStartMission = CTimer::GetTimeInMilliseconds();
	AutoPilot.m_bStayInCurrentLevel = false;
	AutoPilot.m_bIgnorePathfinding = false;
	AutoPilot.m_nSwitchDistance = 20;
}

CVehicle::~CVehicle()
{
	m_nAlarmState = 0;
	if (m_audioEntityId >= 0){
		DMAudio.DestroyEntity(m_audioEntityId);
		m_audioEntityId = -5;
	}
	CRadar::ClearBlipForEntity(BLIP_CAR, CPools::GetVehiclePool()->GetIndex(this));
	if (pDriver)
		pDriver->FlagToDestroyWhenNextProcessed();
	for (int i = 0; i < m_nNumMaxPassengers; i++){
		if (pPassengers[i])
			pPassengers[i]->FlagToDestroyWhenNextProcessed();
	}
	if (m_pCarFire)
		m_pCarFire->Extinguish();
	CCarCtrl::UpdateCarCount(this, true);
	if (bIsAmbulanceOnDuty){
		CCarCtrl::NumAmbulancesOnDuty--;
		bIsAmbulanceOnDuty = false;
	}
	if (bIsFireTruckOnDuty){
		CCarCtrl::NumFiretrucksOnDuty--;
		bIsFireTruckOnDuty = false;
	}
}

void
CVehicle::SetModelIndex(uint32 id)
{
	CEntity::SetModelIndex(id);
	m_aExtras[0] = CVehicleModelInfo::ms_compsUsed[0];
	m_aExtras[1] = CVehicleModelInfo::ms_compsUsed[1];
	m_nNumMaxPassengers = CVehicleModelInfo::GetMaximumNumberOfPassengersFromNumberOfDoors(id);
}

bool
CVehicle::SetupLighting(void)
{
	ActivateDirectional();
	SetAmbientColoursForPedsCarsAndObjects();

	if(bRenderScorched){
		WorldReplaceNormalLightsWithScorched(Scene.world, 0.1f);
	}else{
		CVector coors = GetPosition();
		float lighting = CPointLights::GenerateLightsAffectingObject(&coors);
		if(!bHasBlip && lighting != 1.0f){
			SetAmbientAndDirectionalColours(lighting);
			return true;
		}
	}

	return false;
}

void
CVehicle::RemoveLighting(bool reset)
{
	CRenderer::RemoveVehiclePedLights(this, reset);
}

bool
CVehicle::IsClearToDriveAway(void)
{
	CColPoint point;
	float length = GetColModel()->boundingBox.GetSize().y;
	CEntity *ent = nil;
	CVector front = GetForward() * (length*0.5f + 3.0f);
	return !CWorld::ProcessLineOfSight(GetPosition() + front, GetPosition(),
			point, ent, true, true, false, false, false, true, true) ||
		ent == this;
}

float
CVehicle::GetHeightAboveRoad(void)
{
	return -1.0f * GetColModel()->boundingBox.min.z;
}

void
CVehicle::FlyingControl(eFlightModel flightModel)
{
	if(pFlyingHandling == nil)
		return;

	switch(flightModel){
	case FLIGHT_MODEL_DODO:
	{
		// This seems pretty magic

		// Move Left/Right
		float moveSpeed = m_vecMoveSpeed.Magnitude();
		float sideSpeed = DotProduct(m_vecMoveSpeed, GetRight());
		float sideImpulse = -1.0f * sideSpeed / moveSpeed;
		float fwdSpeed = DotProduct(m_vecMoveSpeed, GetForward());
		float magic = m_vecMoveSpeed.MagnitudeSqr() * sq(fwdSpeed);
		float turnImpulse = (sideImpulse*0.003f + m_fSteerAngle*0.001f) *
			magic*m_fTurnMass*CTimer::GetTimeStep();
		ApplyTurnForce(turnImpulse*GetRight(), -4.0f*GetForward());

		float impulse = sideImpulse*0.2f *
			magic*m_fMass*CTimer::GetTimeStep();
		ApplyMoveForce(impulse*GetRight());
		ApplyTurnForce(impulse*GetRight(), 2.0f*GetUp());


		// Move Up/Down
		moveSpeed = m_vecMoveSpeed.Magnitude();
		float upSpeed = DotProduct(m_vecMoveSpeed, GetUp());
		float upImpulse = -1.0f * upSpeed / moveSpeed;
		turnImpulse = (upImpulse*0.002f + -CPad::GetPad(0)->GetSteeringUpDown()/128.0f*0.001f) *
			magic*m_fTurnMass*CTimer::GetTimeStep();
		ApplyTurnForce(turnImpulse*GetUp(), -4.0f*GetForward());

		impulse = (upImpulse*3.5f + 0.5f)*0.05f *
			magic*m_fMass*CTimer::GetTimeStep();
		if(GRAVITY*m_fMass*CTimer::GetTimeStep() < impulse &&
		   GetPosition().z > 100.0f)
			impulse = 0.9f*GRAVITY*m_fMass*CTimer::GetTimeStep();
		CVector com = Multiply3x3(GetMatrix(), m_vecCentreOfMass);
		ApplyMoveForce(impulse*GetUp());
		ApplyTurnForce(impulse*GetUp(), 2.0f*GetUp() + com);


		m_vecTurnSpeed.y *= Pow(0.9f, CTimer::GetTimeStep());


		moveSpeed = m_vecMoveSpeed.MagnitudeSqr();
		if(moveSpeed > SQR(1.5f))
			m_vecMoveSpeed *= 1.5f/Sqrt(moveSpeed);

		float turnSpeed = m_vecTurnSpeed.MagnitudeSqr();
		if(turnSpeed > SQR(0.2f))
			m_vecTurnSpeed *= 0.2f/Sqrt(turnSpeed);
		break;
	}

	case FLIGHT_MODEL_RCPLANE:
	case FLIGHT_MODEL_SEAPLANE:
	case FLIGHT_MODEL_PLANE_UNUSED:
	case FLIGHT_MODEL_PLANE:
	{
		float fSteerLR = CPad::GetPad(0)->GetSteeringLeftRight() / 128.0f;
		float fSteerUD = -CPad::GetPad(0)->GetSteeringUpDown() / 128.0f;
		float fGunUD = Abs(CPad::GetPad(0)->GetCarGunUpDown());
		if(fGunUD > 1.0f)
			fSteerUD = -CPad::GetPad(0)->GetCarGunUpDown() / 128.0f;

		float fSteerAngle = Atan2(fSteerUD, fSteerLR);
		float fSteerMult = 1.0f;
		if(fSteerAngle > -PI/4.0f && fSteerAngle <= PI/4.0f)
			fSteerMult = 1.0f/Cos(fSteerAngle);
		else if(fSteerAngle > PI/4.0f && fSteerAngle <= PI*3.0f/4.0f)
			fSteerMult = 1.0f/Cos(fSteerAngle - HALFPI);
		else if(fSteerAngle > PI*3.0f/4.0f)
			fSteerMult = 1.0f/Cos(fSteerAngle - PI);
		else if(fSteerAngle <= -PI*3.0f/4.0f)
			fSteerMult = 1.0f/Cos(fSteerAngle + PI);
		else if(fSteerAngle > -PI*3.0f/4.0f && fSteerAngle < -PI/4.0f)
			fSteerMult = 1.0f/Cos(fSteerAngle + HALFPI);

		fSteerLR *= fSteerMult;
		fSteerUD *= -fSteerMult;

		// thrust
		float fForwSpeed = DotProduct(GetMoveSpeed(), GetForward());
		CVector vecTail = GetColModel()->boundingBox.min.y * GetForward();
		float fThrust = (CPad::GetPad(0)->GetAccelerate() - CPad::GetPad(0)->GetBrake()) / 255.0f;
		float fThrustAccel;
		if(fForwSpeed > 0.0f || fThrust > 0.0f)
			fThrustAccel = (fThrust - pFlyingHandling->fThrustFallOff * fForwSpeed) * pFlyingHandling->fThrust;
		else
			fThrustAccel = Min(fThrust - 8.0f * pFlyingHandling->fThrustFallOff * fForwSpeed, 0.0f) * pFlyingHandling->fThrust;
		if(flightModel == FLIGHT_MODEL_PLANE_UNUSED)
			fThrustAccel *= 0.3f;
		else if(flightModel == FLIGHT_MODEL_PLANE)
			fThrustAccel *= 0.1f;
		ApplyMoveForce(fThrustAccel * GetForward() * m_fMass * CTimer::GetTimeStep());

		// left/right
		float fSideSpeed = -DotProduct(GetMoveSpeed(), GetRight());
		float fSideSlipAccel = pFlyingHandling->fSideSlip * fSideSpeed * Abs(fSideSpeed);
		ApplyMoveForce(m_fMass * GetRight() * fSideSlipAccel * CTimer::GetTimeStep());

		float fYaw = -DotProduct(GetSpeed(vecTail), GetRight());
		float fYawAccel = pFlyingHandling->fYawStab * fYaw * Abs(fYaw) + pFlyingHandling->fYaw * fSteerLR * fForwSpeed;
		ApplyTurnForce(fYawAccel * GetRight() * m_fTurnMass * CTimer::GetTimeStep(), vecTail);

		float fRollAccel;
		if (flightModel == FLIGHT_MODEL_RCPLANE) {
			float fDirectionMultiplier = CPad::GetPad(0)->GetLookRight();
			if (CPad::GetPad(0)->GetLookLeft())
				fDirectionMultiplier = -1;
			fRollAccel = (0.5f * fDirectionMultiplier + fSteerLR) * pFlyingHandling->fRoll;
		}
		else
			fRollAccel = fSteerLR * pFlyingHandling->fRoll;
		ApplyTurnForce(GetRight() * fRollAccel * fForwSpeed * m_fTurnMass * CTimer::GetTimeStep(), GetUp());

		CVector vecFRight = CrossProduct(GetForward(), CVector(0.0f, 0.0f, 1.0f));
		CVector vecStabilise = (GetUp().z > 0.0f) ? vecFRight : -vecFRight;
		float fStabiliseDirection = (GetRight().z > 0.0f) ? -1.0f : 1.0f;
		float fStabiliseSpeed = pFlyingHandling->fRollStab * fStabiliseDirection * (1.0f - DotProduct(GetRight(), vecStabilise)) * (1.0f - Abs(GetForward().z));
		ApplyTurnForce(fStabiliseSpeed * m_fTurnMass * GetRight(), GetUp()); // no CTimer::GetTimeStep(), is it right?

		// up/down
		float fTail = -DotProduct(GetSpeed(vecTail), GetUp());
		float fPitchAccel = pFlyingHandling->fPitchStab * fTail * Abs(fTail) + pFlyingHandling->fPitch * fSteerUD * fForwSpeed;
		ApplyTurnForce(fPitchAccel * m_fTurnMass * GetUp() * CTimer::GetTimeStep(), vecTail);

		float fLift = -DotProduct(GetMoveSpeed(), GetUp()) / Max(0.01f, GetMoveSpeed().Magnitude());
		float fLiftAccel = (pFlyingHandling->fAttackLift * fLift + pFlyingHandling->fFormLift) * fForwSpeed * fForwSpeed;
		float fLiftImpulse = fLiftAccel * m_fMass * CTimer::GetTimeStep();
		if (GRAVITY * CTimer::GetTimeStep() * m_fMass < fLiftImpulse) {
			if (flightModel == FLIGHT_MODEL_RCPLANE && GetPosition().z > 50.0f)
				fLiftImpulse = CTimer::GetTimeStep() * 0.9f*GRAVITY * m_fMass;
			else if (flightModel == FLIGHT_MODEL_SEAPLANE && GetPosition().z > 80.0f)
				fLiftImpulse = CTimer::GetTimeStep() * 0.9f*GRAVITY * m_fMass;
		}
		ApplyMoveForce(fLiftImpulse * GetUp());

		CVector vecResistance;
		vecResistance = pFlyingHandling->vecTurnRes;
		float rX = Pow(vecResistance.x, CTimer::GetTimeStep());
		float rY = Pow(vecResistance.y, CTimer::GetTimeStep());
		float rZ = Pow(vecResistance.z, CTimer::GetTimeStep());
		CVector vecTurnSpeed = Multiply3x3(m_vecTurnSpeed, GetMatrix());
		vecTurnSpeed.x *= rX;
		float fResistance = vecTurnSpeed.y * (1.0f / (pFlyingHandling->vecSpeedRes.y * SQR(vecTurnSpeed.y) + 1.0f)) * rY - vecTurnSpeed.y;
		vecTurnSpeed.z *= rZ;
		m_vecTurnSpeed = Multiply3x3(GetMatrix(), vecTurnSpeed);
		ApplyTurnForce(-GetUp() * fResistance * m_fTurnMass, GetRight() + Multiply3x3(GetMatrix(), m_vecCentreOfMass));


		float fMoveSpeed = m_vecMoveSpeed.MagnitudeSqr();
		if(fMoveSpeed > SQR(1.5f))
			m_vecMoveSpeed *= 1.5f/Sqrt(fMoveSpeed);

		float fTurnSpeed = m_vecTurnSpeed.MagnitudeSqr();
		if(fTurnSpeed > SQR(0.2f))
			m_vecTurnSpeed *= 0.2f/Sqrt(fTurnSpeed);
		break;
	}
	case FLIGHT_MODEL_RCHELI:
	case FLIGHT_MODEL_HELI:
	{
		float rm = Pow(pFlyingHandling->fMoveRes, CTimer::GetTimeStep());
		m_vecMoveSpeed *= rm;
		if (GetStatus() != STATUS_PLAYER && GetStatus() != STATUS_PLAYER_REMOTE)
			return;
		float fUpSpeed = DotProduct(m_vecMoveSpeed, GetUp());
		float fThrust = (CPad::GetPad(0)->GetAccelerate() - CPad::GetPad(0)->GetBrake()) / 255.0f;
		if(fThrust < 0.0f)
			fThrust *= 2.0f;
		if(flightModel == FLIGHT_MODEL_RCHELI){
			fThrust = pFlyingHandling->fThrust * fThrust + 0.45f;
			ApplyMoveForce(GRAVITY * CVector(0.0f, 0.0f, 0.5f) * m_fMass * CTimer::GetTimeStep());
		}else
			fThrust = pFlyingHandling->fThrust * fThrust + 0.95f;
		fThrust -= pFlyingHandling->fThrustFallOff * fUpSpeed;
		if(flightModel == FLIGHT_MODEL_RCHELI && GetPosition().z > 40.0f)
			fThrust *= 10.0f/(GetPosition().z - 30.0f);
		else if(GetPosition().z > 80.0f)
			fThrust *= 10.0f/(GetPosition().z - 70.0f);
		ApplyMoveForce(GRAVITY * GetUp() * fThrust * m_fMass * CTimer::GetTimeStep());

		if (GetUp().z > 0.0f){
			float upRight = clamp(GetRight().z, -pFlyingHandling->fFormLift, pFlyingHandling->fFormLift);
			float upImpulseRight = -upRight * pFlyingHandling->fAttackLift * m_fTurnMass * CTimer::GetTimeStep();
			ApplyTurnForce(upImpulseRight * GetUp(), GetRight());

			float upFwd = clamp(GetForward().z, -pFlyingHandling->fFormLift, pFlyingHandling->fFormLift);
			float upImpulseFwd = -upFwd * pFlyingHandling->fAttackLift * m_fTurnMass * CTimer::GetTimeStep();
			ApplyTurnForce(upImpulseFwd * GetUp(), GetForward());
		}else{
			float upRight = GetRight().z < 0.0f ? -pFlyingHandling->fFormLift : pFlyingHandling->fFormLift;
			float upImpulseRight = -upRight * pFlyingHandling->fAttackLift * m_fTurnMass * CTimer::GetTimeStep();
			ApplyTurnForce(upImpulseRight * GetUp(), GetRight());

			float upFwd = GetForward().z < 0.0f ? -pFlyingHandling->fFormLift : pFlyingHandling->fFormLift;
			float upImpulseFwd = -upFwd * pFlyingHandling->fAttackLift * m_fTurnMass * CTimer::GetTimeStep();
			ApplyTurnForce(upImpulseFwd * GetUp(), GetForward());
		}

		float fRoll, fPitch, fYaw;
		if (bCheat5) {
			fPitch = CPad::GetPad(0)->GetSteeringUpDown() / 128.0f;
			fRoll = CPad::GetPad(0)->GetLookLeft();
			if (CPad::GetPad(0)->GetLookRight())
				fRoll = -1.0f;
			fYaw = CPad::GetPad(0)->GetSteeringLeftRight() / 128.0f;
		} else {
			fPitch = CPad::GetPad(0)->GetSteeringUpDown() / 128.0f;
			fRoll = -CPad::GetPad(0)->GetSteeringLeftRight() / 128.0f;
			fYaw = CPad::GetPad(0)->GetLookRight();
			if (CPad::GetPad(0)->GetLookLeft())
				fYaw = -1.0f;
			if(Abs(CPad::GetPad(0)->GetCarGunLeftRight()) > 1.0f)
				fYaw = CPad::GetPad(0)->GetCarGunLeftRight() / 128.0f;
		}
		if(Abs(CPad::GetPad(0)->GetCarGunUpDown()) > 1.0f)
			fPitch = -CPad::GetPad(0)->GetCarGunUpDown() / 128.0f;
		if (CPad::GetPad(0)->GetHorn()) {
			fYaw = 0.0f;
			fPitch = clamp(pFlyingHandling->fPitchStab * DotProduct(m_vecMoveSpeed, GetForward()), -200.0f, 1.3f);
			fRoll = clamp(pFlyingHandling->fRollStab * DotProduct(m_vecMoveSpeed, GetRight()), -200.0f, 1.3f);
		}
		ApplyTurnForce(fPitch * GetUp() * pFlyingHandling->fPitch * m_fTurnMass * CTimer::GetTimeStep(), GetForward());
		ApplyTurnForce(fRoll * GetUp() * pFlyingHandling->fRoll * m_fTurnMass * CTimer::GetTimeStep(), GetRight());

		float fSideSpeed = -DotProduct(GetMoveSpeed(), GetRight());
		float fSideSlipAccel = pFlyingHandling->fSideSlip * fSideSpeed * Abs(fSideSpeed);
		ApplyMoveForce(m_fMass * GetRight() * fSideSlipAccel * CTimer::GetTimeStep());
		float fYawAccel = pFlyingHandling->fYawStab * fSideSpeed * Abs(fSideSpeed) + pFlyingHandling->fYaw * fYaw;
		ApplyTurnForce(fYawAccel * GetRight() * m_fTurnMass * CTimer::GetTimeStep(), -GetForward());

		ApplyTurnForce(fYaw * GetForward() * pFlyingHandling->fYaw * m_fTurnMass * CTimer::GetTimeStep(), GetRight());

		float rX = Pow(pFlyingHandling->vecTurnRes.x, CTimer::GetTimeStep());
		float rY = Pow(pFlyingHandling->vecTurnRes.y, CTimer::GetTimeStep());
		float rZ = Pow(pFlyingHandling->vecTurnRes.z, CTimer::GetTimeStep());
		CVector vecTurnSpeed = Multiply3x3(m_vecTurnSpeed, GetMatrix());
		float fResistanceMultiplier = Pow(1.0f / (pFlyingHandling->vecSpeedRes.z * SQR(vecTurnSpeed.z) + 1.0f) * rZ, CTimer::GetTimeStep());
		float fResistance = vecTurnSpeed.z * fResistanceMultiplier - vecTurnSpeed.z;
		vecTurnSpeed.x *= rX;
		vecTurnSpeed.y *= rY;
		vecTurnSpeed.z *= fResistanceMultiplier;
		m_vecTurnSpeed = Multiply3x3(GetMatrix(), vecTurnSpeed);
		ApplyTurnForce(-GetRight() * fResistance * m_fTurnMass, GetForward() + Multiply3x3(GetMatrix(), m_vecCentreOfMass));
		break;
	}
	}
}

static CColModel rotorColModel;
static CColSphere rotorColSphere;
float ROTOR_SEMI_THICKNESS = 0.05f;
float ROTOR_TURN_SPEED = 0.2f;
float ROTOR_DISGUARD_MULT = 0.3f;
float ROTOR_COL_ELASTICITY = 1.0f;
float ROTOR_COL_TURNMULT = -0.001f;
float ROTOR_DEFAULT_DAMAGE = 100.0f;

bool
CVehicle::DoBladeCollision(CVector pos, CMatrix &matrix, int16 rotorType, float radius, float damageMult)
{
	CVector max(radius, radius, radius);
	CVector min(-radius, -radius, -radius);

	switch(rotorType){
	case ROTOR_TOP:
	case ROTOR_BOTTOM:
		min.z = -ROTOR_SEMI_THICKNESS;
		max.z = ROTOR_SEMI_THICKNESS;
		break;
	case ROTOR_FRONT:
	case ROTOR_BACK:
		min.y = -ROTOR_SEMI_THICKNESS;
		max.y = ROTOR_SEMI_THICKNESS;
		break;
	case ROTOR_RIGHT:
	case ROTOR_LEFT:
		min.x = -ROTOR_SEMI_THICKNESS;
		max.x = ROTOR_SEMI_THICKNESS;
		break;
	}

	min += pos;
	max += pos;
	rotorColModel.boundingBox.Set(min, max);
	rotorColModel.boundingSphere.Set(radius, pos);
	rotorColSphere.Set(radius, pos, 0, 0);
	rotorColModel.spheres = &rotorColSphere;
	rotorColModel.numSpheres = 1;

	pos = matrix * pos;
	bool hadCollision = false;
	int minX = CWorld::GetSectorIndexX(pos.x - radius);
	if(minX <= 0) minX = 0;

	int minY = CWorld::GetSectorIndexY(pos.y - radius);
	if(minY <= 0) minY = 0;

	int maxX = CWorld::GetSectorIndexX(pos.x + radius);
#ifdef FIX_BUGS
	if(maxX >= NUMSECTORS_X) maxX = NUMSECTORS_X - 1;
#else
	if(maxX >= NUMSECTORS_X) maxX = NUMSECTORS_X;
#endif

	int maxY = CWorld::GetSectorIndexY(pos.y + radius);
#ifdef FIX_BUGS
	if(maxY >= NUMSECTORS_Y) maxY = NUMSECTORS_Y - 1;
#else
	if(maxY >= NUMSECTORS_Y) maxY = NUMSECTORS_Y;
#endif

	CWorld::AdvanceCurrentScanCode();
	for(int curY = minY; curY <= maxY; curY++) {
		for(int curX = minX; curX <= maxX; curX++) {
			CSector *sector = CWorld::GetSector(curX, curY);
			if(BladeColSectorList(sector->m_lists[ENTITYLIST_BUILDINGS], rotorColModel, matrix, rotorType, damageMult))
				hadCollision = true;
			if(BladeColSectorList(sector->m_lists[ENTITYLIST_BUILDINGS_OVERLAP], rotorColModel, matrix, rotorType, damageMult))
				hadCollision = true;
			if(BladeColSectorList(sector->m_lists[ENTITYLIST_VEHICLES], rotorColModel, matrix, rotorType, damageMult))
				hadCollision = true;
			if(BladeColSectorList(sector->m_lists[ENTITYLIST_VEHICLES_OVERLAP], rotorColModel, matrix, rotorType, damageMult))
				hadCollision = true;
			if(BladeColSectorList(sector->m_lists[ENTITYLIST_PEDS], rotorColModel, matrix, rotorType, 0.0f))
				hadCollision = true;
			if(BladeColSectorList(sector->m_lists[ENTITYLIST_PEDS_OVERLAP], rotorColModel, matrix, rotorType, 0.0f))
				hadCollision = true;
			if(BladeColSectorList(sector->m_lists[ENTITYLIST_OBJECTS], rotorColModel, matrix, rotorType, damageMult))
				hadCollision = true;
			if(BladeColSectorList(sector->m_lists[ENTITYLIST_OBJECTS_OVERLAP], rotorColModel, matrix, rotorType, damageMult))
				hadCollision = true;
		}
	}
	rotorColModel.spheres = nil;
	rotorColModel.numSpheres = 0;

	return hadCollision;
}

bool
CVehicle::BladeColSectorList(CPtrList &list, CColModel &rotorColModel, CMatrix &matrix, int16 rotorType, float damageMult)
{
	int i;
	CVector axis;
	CVector turnSpeed(0.0f, 0.0f, 0.0f);
	switch(rotorType){
	case ROTOR_TOP:
		turnSpeed.z = -ROTOR_TURN_SPEED;
		axis = -matrix.GetUp();
		break;
	case ROTOR_BOTTOM:
		turnSpeed.z = ROTOR_TURN_SPEED;
		axis = matrix.GetUp();
		break;

	case ROTOR_FRONT:
		turnSpeed.y = -ROTOR_TURN_SPEED;
		axis = -matrix.GetForward();
		break;
	case ROTOR_BACK:
		turnSpeed.y = ROTOR_TURN_SPEED;
		axis = matrix.GetForward();
		break;

	case ROTOR_RIGHT:
		turnSpeed.x = -ROTOR_TURN_SPEED;
		axis = -matrix.GetRight();
		break;
	case ROTOR_LEFT:
		turnSpeed.x = ROTOR_TURN_SPEED;
		axis = matrix.GetRight();
		break;
	}
	turnSpeed = Multiply3x3(matrix, turnSpeed);
	CVector center = rotorColModel.boundingSphere.center;
	center = matrix*center;

	for(CPtrNode *node = list.first; node; node = node->next) {
		CEntity *entity = (CEntity *)node->item;
		if(entity == (CEntity*)this ||
		   !entity->bUsesCollision ||
		   entity->m_scanCode == CWorld::GetCurrentScanCode())
			continue;

		entity->m_scanCode = CWorld::GetCurrentScanCode();

		int numCollisions;
		CColModel *entityCol;
		if(entity->IsPed())
			entityCol = ((CPedModelInfo*)CModelInfo::GetModelInfo(entity->GetModelIndex()))->AnimatePedColModelSkinned(entity->GetClump());
		else
			entityCol = entity->GetColModel();
		if(entityCol)
			numCollisions = CCollision::ProcessColModels(matrix, rotorColModel, entity->GetMatrix(), *entityCol,
				CWorld::m_aTempColPts, nil, nil);
		else
			numCollisions = 0;

		if(numCollisions > 0 && entity->IsPed()){
			CPed *ped = (CPed*)entity;
			CVector2D dirToRotor = GetPosition() - entity->GetPosition();
			dirToRotor.Normalise();
			int localDir = ped->GetLocalDirection(dirToRotor);
			if(ped->m_attachedTo == nil){
				ped->bIsStanding = false;
				ped->ApplyMoveForce(-5.0f*dirToRotor.x, -5.0f*dirToRotor.y, 5.0f);
			}
			ped->InflictDamage(this, WEAPONTYPE_RUNOVERBYCAR, 1000.0f, PEDPIECE_TORSO, localDir);

			if(CGame::nastyGame && ped->GetIsOnScreen()){
				for(i = 0; i < 16; i++)
					CParticle::AddParticle(PARTICLE_BLOOD_SMALL, ped->GetPosition(), CVector(dirToRotor.x, dirToRotor.y, 1.0f) * 0.01f);
				CParticle::AddParticle(PARTICLE_TEST, ped->GetPosition(), CVector(0.0f, 0.0f, 0.02f), nil, 0.1f);
				CParticle::AddParticle(PARTICLE_TEST, ped->GetPosition()+CVector(0.0f, 0.0f, 0.2f), CVector(0.0f, 0.0f, -0.01f), nil, 0.1f);
			}
		}else if(numCollisions > 0 && entity->GetModelIndex() != MI_MISSILE){
			float impulse = 0.0f;
			bool hadCollision = false;
			float savedElasticity = m_fElasticity;
			m_fElasticity = ROTOR_COL_ELASTICITY;

			for(i = 0; i < numCollisions; i++){
				CVector colpos = CWorld::m_aTempColPts[i].point;
				CVector localColpos = colpos - center;
				float axisDir = DotProduct(axis, localColpos);
				float colDir = DotProduct(CWorld::m_aTempColPts[i].normal, localColpos);

				if(2.0f*ROTOR_SEMI_THICKNESS < Abs(axisDir) &&
				   ROTOR_DISGUARD_MULT*Abs(colDir) < Abs(axisDir))
					continue;

				hadCollision = true;
				colpos -= axisDir*axis;	// get rid of axis component

				CVector tangentSpeed = CrossProduct(turnSpeed, colpos - center);

				// Particles
				for(int j = 0; j < 4; j++){
					CParticle::AddParticle(PARTICLE_SPARK_SMALL, colpos, (tangentSpeed+m_vecMoveSpeed)/2.0f);
					CParticle::AddParticle(PARTICLE_SPARK, colpos, 0.1f*CWorld::m_aTempColPts[i].normal);
				}

				// Apply Collision
				if(IsCar()){
					CAutomobile *heli = (CAutomobile*)this;
					if(heli->m_aWheelSpeed[1] > 0.15f){
						ApplyCollision(CWorld::m_aTempColPts[i], impulse);
						ApplyTurnForce(m_fTurnMass*ROTOR_COL_TURNMULT*tangentSpeed, colpos - center);
						heli->m_aWheelSpeed[1] = 0.15f;
					}else if(heli->m_aWheelSpeed[1] < 0.075f && heli->m_aWheelSpeed[1] > 0.0f)
						heli->m_aWheelSpeed[1] *= -1.0f;
				}

				float damageImpulse = damageMult * Max(impulse, ROTOR_DEFAULT_DAMAGE*m_fMass/3000.0f);
				if(damageImpulse > m_fDamageImpulse)
					SetDamagedPieceRecord(0, damageImpulse, entity, CWorld::m_aTempColPts[i].normal);

			}

			if(hadCollision && !entity->IsPed())
				DMAudio.ReportCollision(this, entity, SURFACE_CAR_PANEL, SURFACE_TARMAC, 50.0f, 0.09f);
			m_fElasticity = savedElasticity;
		}
	}
	return false;
}


float fBurstSpeedMax = 0.3f;
float fBurstTyreMod = 0.13f;

void
CVehicle::ProcessWheel(CVector &wheelFwd, CVector &wheelRight, CVector &wheelContactSpeed, CVector &wheelContactPoint,
	int32 wheelsOnGround, float thrust, float brake, float adhesion, int8 wheelId, float *wheelSpeed, tWheelState *wheelState, uint16 wheelStatus)
{
	// BUG: using statics here is probably a bad idea
	static bool bAlreadySkidding = false;	// this is never reset
	static bool bBraking;
	static bool bDriving;

#ifdef FIX_BUGS
	bAlreadySkidding = false;
#endif

	// how much force we want to apply in these axes
	float fwd = 0.0f;
	float right = 0.0f;

	bBraking = brake != 0.0f;
	if(bBraking)
		thrust = 0.0f;
	bDriving = thrust != 0.0f;

	float contactSpeedFwd = DotProduct(wheelContactSpeed, wheelFwd);
	float contactSpeedRight = DotProduct(wheelContactSpeed, wheelRight);

	if(*wheelState != WHEEL_STATE_NORMAL)
		bAlreadySkidding = true;
	*wheelState = WHEEL_STATE_NORMAL;

	adhesion *= CTimer::GetTimeStep();
	if(bAlreadySkidding)
		adhesion *= pHandling->fTractionLoss;

	// moving sideways
	if(contactSpeedRight != 0.0f){
		// exert opposing force
		right = -contactSpeedRight/wheelsOnGround;
#ifdef FIX_BUGS
		// contactSpeedRight is independent of framerate but right has timestep as a factor
		// so we probably have to fix this
		right *= CTimer::GetTimeStepFix();
#endif

		if(wheelStatus == WHEEL_STATUS_BURST){
			float fwdspeed = Min(contactSpeedFwd, fBurstSpeedMax);
			right += fwdspeed * CGeneral::GetRandomNumberInRange(-fBurstTyreMod, fBurstTyreMod);
		}
	}

	if(bDriving){
		fwd = thrust;

		// limit sideways force (why?)
		if(right > 0.0f){
			if(right > adhesion)
				right = adhesion;
		}else{
			if(right < -adhesion)
				right = -adhesion;
		}
	}else if(contactSpeedFwd != 0.0f){
		fwd = -contactSpeedFwd/wheelsOnGround;
#ifdef FIX_BUGS
		// contactSpeedFwd is independent of framerate but fwd has timestep as a factor
		// so we probably have to fix this
		fwd *= CTimer::GetTimeStepFix();
#endif

		if(!bBraking){
			if(m_fGasPedal < 0.01f){
				if(IsBike())
					brake = 0.6f * mod_HandlingManager.fWheelFriction / (pHandling->fMass + 200.0f);
				else if(pHandling->fMass < 500.0f)
					brake = mod_HandlingManager.fWheelFriction / m_fMass;
				else if(GetModelIndex() == MI_RCBANDIT)
					brake = 0.2f * mod_HandlingManager.fWheelFriction / m_fMass;
				else
					brake = mod_HandlingManager.fWheelFriction / m_fMass;
#ifdef FIX_BUGS
				brake *= CTimer::GetTimeStepFix();
#endif
			}
		}

		if(brake > adhesion){
			if(Abs(contactSpeedFwd) > 0.005f)
				*wheelState = WHEEL_STATE_FIXED;
		}else {
			if(fwd > 0.0f){
				if(fwd > brake)
					fwd = brake;
			}else{
				if(fwd < -brake)
					fwd = -brake;
			}
		}
	}

	float speedSq = sq(right) + sq(fwd);
	if(sq(adhesion) < speedSq){
		if(*wheelState != WHEEL_STATE_FIXED){
			if(bDriving && contactSpeedFwd < 0.2f)
				*wheelState = WHEEL_STATE_SPINNING;
			else
				*wheelState = WHEEL_STATE_SKIDDING;
		}

		float l = Sqrt(speedSq);
		float tractionLoss = bAlreadySkidding ? 1.0f : pHandling->fTractionLoss;
		right *= adhesion * tractionLoss / l;
		fwd *= adhesion * tractionLoss / l;
	}

	if(fwd != 0.0f || right != 0.0f){
		CVector totalSpeed = fwd*wheelFwd + right*wheelRight;

		CVector turnDirection = totalSpeed;
		bool separateTurnForce = false;	// BUG: not initialized on PC
		if(pHandling->fSuspensionAntidiveMultiplier > 0.0f){
			if(bBraking){
				separateTurnForce = true;
				turnDirection = totalSpeed - pHandling->fSuspensionAntidiveMultiplier*fwd*wheelFwd;
			}else if(bDriving){
				separateTurnForce = true;
				turnDirection = totalSpeed - 0.5f*pHandling->fSuspensionAntidiveMultiplier*fwd*wheelFwd;
			}
		}

		CVector direction = totalSpeed;

		float speed = totalSpeed.Magnitude();
		float turnSpeed;
		if(separateTurnForce)
			turnSpeed = turnDirection.Magnitude();
		else
			turnSpeed = speed;
		direction.Normalise();
		if(separateTurnForce)
			turnDirection.Normalise();
		else
			turnDirection = direction;

		float impulse = speed*m_fMass;
		float turnImpulse = turnSpeed*GetMass(wheelContactPoint, turnDirection);

		ApplyMoveForce(impulse * direction);
		ApplyTurnForce(turnImpulse * direction, wheelContactPoint);
	}
}

float fBurstBikeSpeedMax = 0.12f;
float fBurstBikeTyreMod = 0.05f;
float fTweakBikeWheelTurnForce = 2.0f;

void
CVehicle::ProcessBikeWheel(CVector &wheelFwd, CVector &wheelRight, CVector &wheelContactSpeed, CVector &wheelContactPoint,
	int32 wheelsOnGround, float thrust, float brake, float adhesion, float unk, int8 wheelId, float *wheelSpeed, tWheelState *wheelState, eBikeWheelSpecial special, uint16 wheelStatus)
{
	// BUG: using statics here is probably a bad idea
	static bool bAlreadySkidding = false;	// this is never reset
	static bool bBraking;
	static bool bDriving;
	static bool bReversing;

#ifdef FIX_BUGS
	bAlreadySkidding = false;
#endif

	// how much force we want to apply in these axes
	float fwd = 0.0f;
	float right = 0.0f;

	bBraking = brake != 0.0f;
	if(bBraking)
		thrust = 0.0f;
	bDriving = thrust != 0.0f;
	bReversing = thrust < 0.0f;

	float contactSpeedFwd = DotProduct(wheelContactSpeed, wheelFwd);
	float contactSpeedRight;

	if(*wheelState != WHEEL_STATE_NORMAL)
		bAlreadySkidding = true;
	*wheelState = WHEEL_STATE_NORMAL;

	adhesion *= CTimer::GetTimeStep();
	if(bAlreadySkidding)
		adhesion *= pHandling->fTractionLoss;

	if(special == BIKE_WHEEL_2 || special == BIKE_WHEEL_3)
		contactSpeedRight = 0.0f;
	else
		contactSpeedRight = DotProduct(wheelContactSpeed, wheelRight);

	// moving sideways
	if(contactSpeedRight != 0.0f){
		// exert opposing force
		right = -contactSpeedRight/wheelsOnGround;
#ifdef FIX_BUGS
		// contactSpeedRight is independent of framerate but right has timestep as a factor
		// so we probably have to fix this
		right *= CTimer::GetTimeStepFix();
#endif

		if(wheelStatus == WHEEL_STATUS_BURST){
			float fwdspeed = Min(contactSpeedFwd, fBurstBikeSpeedMax);
			right += fwdspeed * CGeneral::GetRandomNumberInRange(-fBurstBikeTyreMod, fBurstBikeTyreMod);
		}
	}

	if(bDriving){
		fwd = thrust;

		// limit sideways force (why?)
		if(right > 0.0f){
			if(right > adhesion)
				right = adhesion;
		}else{
			if(right < -adhesion)
				right = -adhesion;
		}
	}else if(contactSpeedFwd != 0.0f){
		fwd = -contactSpeedFwd/wheelsOnGround;
#ifdef FIX_BUGS
		// contactSpeedFwd is independent of framerate but fwd has timestep as a factor
		// so we probably have to fix this
		fwd *= CTimer::GetTimeStepFix();
#endif

		if(!bBraking){
			if(m_fGasPedal < 0.01f){
				if(IsBike())
					brake = 0.6f * mod_HandlingManager.fWheelFriction / (pHandling->fMass + 200.0f);
				else if(pHandling->fMass < 500.0f)
					brake = mod_HandlingManager.fWheelFriction / m_fMass;
				else if(GetModelIndex() == MI_RCBANDIT)
					brake = 0.2f * mod_HandlingManager.fWheelFriction / m_fMass;
				else
					brake = mod_HandlingManager.fWheelFriction / m_fMass;
#ifdef FIX_BUGS
				brake *= CTimer::GetTimeStepFix();
#endif
			}
		}

		if(brake > adhesion){
			if(Abs(contactSpeedFwd) > 0.005f)
				*wheelState = WHEEL_STATE_FIXED;
		}else {
			if(fwd > 0.0f){
				if(fwd > brake)
					fwd = brake;
			}else{
				if(fwd < -brake)
					fwd = -brake;
			}
		}
	}

	float speedSq = sq(right) + sq(fwd);
	if(sq(adhesion) < speedSq){
		if(*wheelState != WHEEL_STATE_FIXED){
			if(bDriving && contactSpeedFwd < 0.2f)
				*wheelState = WHEEL_STATE_SPINNING;
			else
				*wheelState = WHEEL_STATE_SKIDDING;
		}

		float l = Sqrt(speedSq);
		float tractionLoss = bAlreadySkidding ? 1.0f : pHandling->fTractionLoss;
		right *= adhesion * tractionLoss / l;
		fwd *= adhesion * tractionLoss / l;

		if(unk < 1.0f)
			right *= unk;
	}else if(unk < 1.0f){
		if(!bAlreadySkidding)
			unk *= pHandling->fTractionLoss;
		if(sq(adhesion*unk) < speedSq){
			float l = Sqrt(speedSq);
			right *= adhesion * unk / l;
		}
	}

	if(fwd != 0.0f || right != 0.0f){
		CVector direction = fwd*wheelFwd + right*wheelRight;

		float speed = direction.Magnitude();
		direction.Normalise();

		float impulse = speed*m_fMass;
		float turnImpulse = speed*GetMass(wheelContactPoint, direction);
		CVector vTurnImpulse = turnImpulse * direction;
		float turnRight = DotProduct(vTurnImpulse, GetRight());

		ApplyMoveForce(impulse * direction);

		float contactRight = DotProduct(wheelContactPoint, GetRight());
		float contactFwd = DotProduct(wheelContactPoint, GetForward());

		if(wheelId != CARWHEEL_REAR_LEFT ||
		   !bBraking && !bReversing)
			ApplyTurnForce((vTurnImpulse - turnRight*GetRight()) * fTweakBikeWheelTurnForce,
				wheelContactPoint - contactRight*GetRight());

		ApplyTurnForce(turnRight*GetRight(), contactFwd*GetForward());
	}
}

float
CVehicle::ProcessWheelRotation(tWheelState state, const CVector &fwd, const CVector &speed, float radius)
{
	float angularVelocity;
	switch(state){
	case WHEEL_STATE_SPINNING:
		angularVelocity = -1.1f;	// constant speed forward
		break;
	case WHEEL_STATE_FIXED:
		angularVelocity = 0.0f;		// not moving
		break;
	default:
		angularVelocity = -DotProduct(fwd, speed) / radius;	// forward speed
		break;
	}
	return angularVelocity * CTimer::GetTimeStep();
}

int
CVehicle::FindTyreNearestPoint(float x, float y)
{
	CVector pos = CVector(x - GetPosition().x, y - GetPosition().y, 0.0f);
	float fwd = DotProduct(GetForward(), pos);
	float right = DotProduct(GetRight(), pos);

	int piece;
	if(IsBike()){
		piece = fwd > 0.0f ? CAR_PIECE_WHEEL_LF : CAR_PIECE_WHEEL_LR;
	}else{
		piece = fwd > 0.0f ?
			right > 0.0f ? CAR_PIECE_WHEEL_RF : CAR_PIECE_WHEEL_LF :
			right > 0.0f ? CAR_PIECE_WHEEL_RR : CAR_PIECE_WHEEL_LR;
	}
	return piece - CAR_PIECE_WHEEL_LF;
}

void
CVehicle::InflictDamage(CEntity *damagedBy, eWeaponType weaponType, float damage, CVector pos)
{
	if (!bCanBeDamaged)
		return;
	if(GetStatus() == STATUS_PLAYER && CStats::GetPercentageProgress() >= 100.0f)
		damage *= 0.5f;
	if (GetStatus() != STATUS_PLAYER && bOnlyDamagedByPlayer && (damagedBy != FindPlayerPed() && damagedBy != FindPlayerVehicle()))
		return;

	if(damage > 10.0f && (damagedBy == FindPlayerPed() || damagedBy == FindPlayerVehicle()) && GetStatus() != STATUS_WRECKED){
		CWorld::Players[CWorld::PlayerInFocus].m_nHavocLevel += 2;
		CWorld::Players[CWorld::PlayerInFocus].m_fMediaAttention += 1.0f;
		CStats::PropertyDestroyed += CGeneral::GetRandomNumberInRange(5, 25);
	}

	bool bFrightensDriver = false;
	switch (weaponType) {
	case WEAPONTYPE_UNARMED:
	case WEAPONTYPE_BRASSKNUCKLE:
	case WEAPONTYPE_SCREWDRIVER:
	case WEAPONTYPE_GOLFCLUB:
	case WEAPONTYPE_NIGHTSTICK:
	case WEAPONTYPE_KNIFE:
	case WEAPONTYPE_BASEBALLBAT:
	case WEAPONTYPE_HAMMER:
	case WEAPONTYPE_CLEAVER:
	case WEAPONTYPE_MACHETE:
	case WEAPONTYPE_KATANA:
	case WEAPONTYPE_CHAINSAW:
		if (bMeleeProof)
			return;
		break;
	case WEAPONTYPE_COLT45:
	case WEAPONTYPE_PYTHON:
	case WEAPONTYPE_SHOTGUN:
	case WEAPONTYPE_SPAS12_SHOTGUN:
	case WEAPONTYPE_STUBBY_SHOTGUN:
	case WEAPONTYPE_TEC9:
	case WEAPONTYPE_UZI:
	case WEAPONTYPE_SILENCED_INGRAM:
	case WEAPONTYPE_MP5:
	case WEAPONTYPE_M4:
	case WEAPONTYPE_RUGER:
	case WEAPONTYPE_SNIPERRIFLE:
	case WEAPONTYPE_LASERSCOPE:
	case WEAPONTYPE_M60:
	case WEAPONTYPE_MINIGUN:
	case WEAPONTYPE_HELICANNON:
	case WEAPONTYPE_UZI_DRIVEBY:
		if (bBulletProof)
			return;
		bFrightensDriver = true;
		break;
	case WEAPONTYPE_GRENADE:
	case WEAPONTYPE_MOLOTOV:
	case WEAPONTYPE_ROCKET:
	case WEAPONTYPE_EXPLOSION:
		if (bExplosionProof)
			return;
		bFrightensDriver = true;
		break;
	case WEAPONTYPE_FLAMETHROWER:
		if (bFireProof)
			return;
		break;
	case WEAPONTYPE_RAMMEDBYCAR:
		if (bCollisionProof)
			return;
		break;
	default:
		break;
	}

	if(bFrightensDriver && GetStatus() == STATUS_PLAYER && m_fHealth < 250.0f)
		return;

	// Pop tires
	if(damagedBy && damagedBy->IsPed() && (IsCar() || IsBike())){
		int accuracy = 0;
		switch(weaponType){
		case WEAPONTYPE_COLT45:
			accuracy = 10;
			break;
		case WEAPONTYPE_PYTHON:
			if(!((CPed*)damagedBy)->IsPlayer())
				accuracy = 64;
			break;
		case WEAPONTYPE_SHOTGUN:
		case WEAPONTYPE_STUBBY_SHOTGUN:
		case WEAPONTYPE_M60:
		case WEAPONTYPE_HELICANNON:
			accuracy = 25;
			break;
		case WEAPONTYPE_TEC9:
		case WEAPONTYPE_UZI:
		case WEAPONTYPE_SILENCED_INGRAM:
		case WEAPONTYPE_MP5:
		case WEAPONTYPE_UZI_DRIVEBY:
			accuracy = 15;
			break;
		case WEAPONTYPE_M4:
		case WEAPONTYPE_RUGER:
			if(!((CPed*)damagedBy)->IsPlayer())
				accuracy = 15;
			break;
		}

		if(((CPed*)damagedBy)->IsPlayer() && (CCamera::m_bUseMouse3rdPerson || TheCamera.Using1stPersonWeaponMode()))
			accuracy = 0;

		if(accuracy != 0 && !bTyresDontBurst && (CGeneral::GetRandomNumber()&0x7F) < accuracy){
			if(IsBike())
				BurstTyre(FindTyreNearestPoint(pos.x, pos.y) + CAR_PIECE_WHEEL_LF, false);
			else if(GetVehicleAppearance() == VEHICLE_APPEARANCE_CAR)
				BurstTyre(FindTyreNearestPoint(pos.x, pos.y) + CAR_PIECE_WHEEL_LF, true);
		}
	}

	if (m_fHealth > 0.0f) {
		if (VehicleCreatedBy == RANDOM_VEHICLE && pDriver &&
			(GetStatus() == STATUS_SIMPLE || GetStatus() == STATUS_PHYSICS) &&
			AutoPilot.m_nCarMission == MISSION_CRUISE) {
			if (m_randomSeed < DAMAGE_FLEE_IN_CAR_PROBABILITY_VALUE) {
				CCarCtrl::SwitchVehicleToRealPhysics(this);
				AutoPilot.m_nDrivingStyle = DRIVINGSTYLE_AVOID_CARS;
				AutoPilot.m_nCruiseSpeed = GAME_SPEED_TO_CARAI_SPEED * pHandling->Transmission.fUnkMaxVelocity;
				SetStatus(STATUS_PHYSICS);
			}
		}
		m_nLastWeaponDamage = weaponType;
		m_pLastDamageEntity = damagedBy;
		float oldHealth = m_fHealth;
		if (m_fHealth > damage) {
			m_fHealth -= damage;
			if (VehicleCreatedBy == RANDOM_VEHICLE && !IsBoat()){
				switch (GetStatus()) {
				case STATUS_SIMPLE:
				case STATUS_PHYSICS:
					if(AutoPilot.m_nDrivingStyle == DRIVINGSTYLE_PLOUGH_THROUGH ||
					   CGeneral::GetRandomNumberInRange(0.0f, 1.0f) > 0.5f && AutoPilot.m_nCarMission == MISSION_CRUISE){
						// Drive away like a maniac
						if(pDriver && pDriver->m_objective != OBJECTIVE_LEAVE_VEHICLE){
							if(AutoPilot.m_nDrivingStyle != DRIVINGSTYLE_PLOUGH_THROUGH)
								AutoPilot.m_nCruiseSpeed *= 1.5f;
							AutoPilot.m_nDrivingStyle = DRIVINGSTYLE_PLOUGH_THROUGH;
						}
					}else{
						// Leave vehicle
						if (pDriver && pDriver->CharCreatedBy != MISSION_CHAR) {
							SetStatus(STATUS_ABANDONED);
							pDriver->bFleeAfterExitingCar = true;
							pDriver->SetObjective(OBJECTIVE_LEAVE_VEHICLE, this);
							pDriver->Say(SOUND_PED_FLEE_SPRINT);
						}
						int time = 200;
						for (int i = 0; i < m_nNumMaxPassengers; i++) {
							if (pPassengers[i] &&
							    pPassengers[i]->m_objective != OBJECTIVE_LEAVE_VEHICLE &&
							    pPassengers[i]->CharCreatedBy != MISSION_CHAR) {
								pPassengers[i]->bFleeAfterExitingCar = true;
								pPassengers[i]->SetObjective(OBJECTIVE_LEAVE_VEHICLE, this);
								pPassengers[i]->m_objectiveTimer = CTimer::GetTimeInMilliseconds() + time;
								pPassengers[i]->Say(SOUND_PED_FLEE_SPRINT);
								time += 200;
							}
						}
					}
					break;
				default:
					break;
				}
			}
			if (oldHealth >= DAMAGE_HEALTH_TO_CATCH_FIRE && m_fHealth < DAMAGE_HEALTH_TO_CATCH_FIRE) {
				if (IsCar()) {
					CAutomobile* pThisCar = (CAutomobile*)this;
					pThisCar->Damage.SetEngineStatus(ENGINE_STATUS_ON_FIRE);
					pThisCar->m_pSetOnFireEntity = damagedBy;
					if (damagedBy)
						damagedBy->RegisterReference((CEntity**)&pThisCar->m_pSetOnFireEntity);
				}
			}
		}
		else {
			m_fHealth = 0.0f;
			if (weaponType == WEAPONTYPE_EXPLOSION) {
				// between 1000 and 3047. Also not very nice: can't be saved by respray or cheat
				m_nBombTimer = 1000 + CGeneral::GetRandomNumber() & 0x7FF;
				m_pBlowUpEntity = damagedBy;
				if (damagedBy)
					damagedBy->RegisterReference((CEntity**)&m_pBlowUpEntity);
			}
			else
				BlowUpCar(damagedBy);
		}
	}
#ifdef FIX_BUGS // removing dumb case when shooting police car in player's own garage gives wanted level
	if (GetModelIndex() == MI_POLICE && damagedBy == FindPlayerPed() && !bHasBeenOwnedByPlayer)
#else
	if (GetModelIndex() == MI_POLICE && damagedBy == FindPlayerPed())
#endif
		FindPlayerPed()->SetWantedLevelNoDrop(1);
}

void
CVehicle::DoFixedMachineGuns(void)
{
	if(TheCamera.Cams[TheCamera.ActiveCam].DirectionWasLooking == LOOKING_FORWARD){
		if(CPad::GetPad(0)->GetCarGunFired() && !bGunSwitchedOff){
			FireFixedMachineGuns();
		}else{
			if(CTimer::GetTimeInMilliseconds() > m_nGunFiringTime + 1400)
				m_nAmmoInClip = 20;
		}
	}
}

void
CVehicle::FireFixedMachineGuns(void)
{
	if (CTimer::GetTimeInMilliseconds() <= m_nGunFiringTime + 150)
		return;
	CVector source, target;
	float dx, dy, len;

	dx = GetForward().x;
	dy = GetForward().y;
	len = Sqrt(SQR(dx) + SQR(dy));
	if (len < 0.1f) len = 0.1f;
	dx /= len;
	dy /= len;

	m_nGunFiringTime = CTimer::GetTimeInMilliseconds();

	source = GetMatrix() * CVector(2.0f, 2.5f, 1.0f);
	target = source + CVector(dx, dy, 0.0f) * 60.0f;
	target += CVector(
		((CGeneral::GetRandomNumber() & 0xFF) - 128) * 0.015f,
		((CGeneral::GetRandomNumber() & 0xFF) - 128) * 0.015f,
		((CGeneral::GetRandomNumber() & 0xFF) - 128) * 0.02f);
	CWeapon::DoTankDoomAiming(this, pDriver, &source, &target);
	FireOneInstantHitRound(&source, &target, 15);

	source = GetMatrix() * CVector(-2.0f, 2.5f, 1.0f);
	target = source + CVector(dx, dy, 0.0f) * 60.0f;
	target += CVector(
		((CGeneral::GetRandomNumber() & 0xFF) - 128) * 0.015f,
		((CGeneral::GetRandomNumber() & 0xFF) - 128) * 0.015f,
		((CGeneral::GetRandomNumber() & 0xFF) - 128) * 0.02f);
	CWeapon::DoTankDoomAiming(this, pDriver, &source, &target);
	FireOneInstantHitRound(&source, &target, 15);

	DMAudio.PlayOneShot(m_audioEntityId, SOUND_WEAPON_SHOT_FIRED, 0.0f);

	m_nAmmoInClip--;
	if (m_nAmmoInClip == 0) {
		m_nAmmoInClip = 20;
		m_nGunFiringTime = CTimer::GetTimeInMilliseconds() + 1400;
	}
}

void
CVehicle::ActivateBomb(void)
{
	if(m_bombType == CARBOMB_TIMED){
		m_bombType = CARBOMB_TIMEDACTIVE;
		m_nBombTimer = 7000;
		m_pBlowUpEntity = FindPlayerPed();
		CGarages::TriggerMessage("GA_12", -1, 3000, -1);
		DMAudio.PlayOneShot(m_audioEntityId, SOUND_BOMB_TIMED_ACTIVATED, 1.0f);
	}else if(m_bombType == CARBOMB_ONIGNITION){
		m_bombType = CARBOMB_ONIGNITIONACTIVE;
		CGarages::TriggerMessage("GA_12", -1, 3000, -1);
		DMAudio.PlayOneShot(m_audioEntityId, SOUND_BOMB_ONIGNITION_ACTIVATED, 1.0f);
	}
}

void
CVehicle::ActivateBombWhenEntered(void)
{
	if(pDriver){
		if(!bDriverLastFrame && m_bombType == CARBOMB_ONIGNITIONACTIVE){
			// If someone enters the car and there is a bomb, detonate
			m_nBombTimer = 1000;
			m_pBlowUpEntity = m_pBombRigger;
			if(m_pBlowUpEntity)
				m_pBlowUpEntity->RegisterReference((CEntity**)&m_pBlowUpEntity);
			DMAudio.PlayOneShot(m_audioEntityId, SOUND_BOMB_TICK, 1.0f);
		}
		bDriverLastFrame = true;
	}else
		bDriverLastFrame = false;
}

void
CVehicle::ExtinguishCarFire(void)
{
	if(GetStatus() != STATUS_WRECKED)
		m_fHealth = Max(m_fHealth, 300.0f);
	if(m_pCarFire)
		m_pCarFire->Extinguish();
	if(IsCar()){
		CAutomobile *car = (CAutomobile*)this;
		if(car->Damage.GetEngineStatus() >= ENGINE_STATUS_ON_FIRE)
			car->Damage.SetEngineStatus(ENGINE_STATUS_ON_FIRE-10);
		car->m_fFireBlowUpTimer = 0.0f;
	}
}

bool
CVehicle::ShufflePassengersToMakeSpace(void)
{
	if (m_nNumPassengers >= m_nNumMaxPassengers)
		return false;
	if (pPassengers[1] &&
		!(m_nGettingInFlags & CAR_DOOR_FLAG_LR) &&
		IsRoomForPedToLeaveCar(CAR_DOOR_LR, nil)) {
		if (!pPassengers[2] && !(m_nGettingInFlags & CAR_DOOR_FLAG_RR)) {
			pPassengers[2] = pPassengers[1];
			pPassengers[1] = nil;
			pPassengers[2]->m_vehEnterType = CAR_DOOR_RR;
			return true;
		}
		if (!pPassengers[0] && !(m_nGettingInFlags & CAR_DOOR_FLAG_RF)) {
			pPassengers[0] = pPassengers[1];
			pPassengers[1] = nil;
			pPassengers[0]->m_vehEnterType = CAR_DOOR_RF;
			return true;
		}
		return false;
	}
	if (pPassengers[2] &&
		!(m_nGettingInFlags & CAR_DOOR_FLAG_RR) &&
		IsRoomForPedToLeaveCar(CAR_DOOR_RR, nil)) {
		if (!pPassengers[1] && !(m_nGettingInFlags & CAR_DOOR_FLAG_LR)) {
			pPassengers[1] = pPassengers[2];
			pPassengers[2] = nil;
			pPassengers[1]->m_vehEnterType = CAR_DOOR_LR;
			return true;
		}
		if (!pPassengers[0] && !(m_nGettingInFlags & CAR_DOOR_FLAG_RF)) {
			pPassengers[0] = pPassengers[2];
			pPassengers[2] = nil;
			pPassengers[0]->m_vehEnterType = CAR_DOOR_RF;
			return true;
		}
		return false;
	}
	if (pPassengers[0] &&
		!(m_nGettingInFlags & CAR_DOOR_FLAG_RF) &&
		IsRoomForPedToLeaveCar(CAR_DOOR_RF, nil)) {
		if (!pPassengers[1] && !(m_nGettingInFlags & CAR_DOOR_FLAG_LR)) {
			pPassengers[1] = pPassengers[0];
			pPassengers[0] = nil;
			pPassengers[1]->m_vehEnterType = CAR_DOOR_LR;
			return true;
		}
		if (!pPassengers[2] && !(m_nGettingInFlags & CAR_DOOR_FLAG_RR)) {
			pPassengers[2] = pPassengers[0];
			pPassengers[0] = nil;
			pPassengers[2]->m_vehEnterType = CAR_DOOR_RR;
			return true;
		}
		return false;
	}
	return false;
}

void
CVehicle::MakeNonDraggedPedsLeaveVehicle(CPed *ped1, CPed *ped2, CPlayerPed *&player, CCopPed *&cop)
{
	int i;
	player = nil;
	cop = nil;

	if(ped1->IsPlayer() && ped2->m_nPedType == PEDTYPE_COP &&
	   ((CPlayerPed*)ped1)->m_pWanted->m_nWantedLevel > 0 &&
	   ped2->m_pedInObjective == ped1){
		player = (CPlayerPed*)ped1;
		cop = (CCopPed*)ped2;
		return;
	}

	bool ped1IsDriver = ped1 == pDriver;

	// Just what the hell is this weird code?
	CPed *peds[9];
	CPed *peds2[9];
	int numPeds = 0;
	int numPeds2 = 0;
	for(i = 0; i < m_nNumMaxPassengers; i++){
		CPed *p = pPassengers[i];
		if(p && p != ped1 && !p->bStayInCarOnJack){
			peds[numPeds++] = p;
			// uhh what?
			if(i < 1 && !ped1IsDriver)
				continue;
			peds2[numPeds2++] = p;
		}
	}

	// So we're copying this array for no reason...
	CPed *peds3[9];
	int numPeds3 = 0;
	for(i = 0; i < numPeds; i++){
		if(peds[i]->IsPlayer() && ped2->m_nPedType == PEDTYPE_COP &&
		   ((CPlayerPed*)peds[i])->m_pWanted->m_nWantedLevel > 0 &&
		   ped2->m_pedInObjective == peds[i]){
			player = (CPlayerPed*)peds[i];
			cop = (CCopPed*)ped2;
			return;
		}
		peds3[numPeds3++] = peds[i];
	}

	int time = 1800;
	for(i = 0; i < numPeds3; i++){
		peds3[i]->m_leaveCarTimer = CTimer::GetTimeInMilliseconds() + time;
		peds3[i]->SetObjective(OBJECTIVE_LEAVE_VEHICLE, this);
		time += CGeneral::GetRandomNumberInRange(300.0f, 600.0f);
	}

	if(IsCar() && numPeds2 > 0 && CGeneral::GetRandomTrueFalse())
		for(i = 0; i < numPeds2; i++)
			if(peds2[i]->IsFemale() || CGeneral::GetRandomTrueFalse()){
				peds2[i]->m_leaveCarTimer = CTimer::GetTimeInMilliseconds() + 10000;
				peds2[i]->b156_8 = true;
				peds2[i]->bFleeAfterExitingCar = true;
			}
}

void
CVehicle::ProcessDelayedExplosion(void)
{
	if(m_nBombTimer == 0)
		return;

	int tick = CTimer::GetTimeStep()/60.0f*1000.0f;
	int16 prev = m_nBombTimer;
	if(tick > m_nBombTimer)
		m_nBombTimer = 0;
	else
		m_nBombTimer -= tick;

	if(IsCar() && ((CAutomobile*)this)->m_bombType == CARBOMB_TIMEDACTIVE && (m_nBombTimer & 0xFE00) != (prev & 0xFE00))
		DMAudio.PlayOneShot(m_audioEntityId, SOUND_CAR_BOMB_TICK, 0.0f);

	if (m_nBombTimer != 0)
		return;

	BlowUpCar(m_pBlowUpEntity);
}

bool
CVehicle::IsLawEnforcementVehicle(void)
{
	switch(GetModelIndex()){
	case MI_POLICE:
	case MI_ENFORCER:
	case MI_PREDATOR:
	case MI_RHINO:
	case MI_BARRACKS:
	case MI_FBIRANCH:
	case MI_VICECHEE:
		return true;
	default:
		return false;
	}
}

bool
CVehicle::UsesSiren(uint32 id)
{
	switch(id){
	case MI_FIRETRUCK:
	case MI_AMBULAN:
	case MI_FBICAR:
	case MI_MRWHOOP:
	case MI_POLICE:
	case MI_ENFORCER:
	case MI_PREDATOR:
	case MI_FBIRANCH:
	case MI_VICECHEE:
		return true;
	default:
		return false;
	}
}

bool
CVehicle::IsVehicleNormal(void)
{
	if (!pDriver || m_nNumPassengers != 0 || GetStatus() == STATUS_WRECKED)
		return false;
	return GetModelInfo()->m_vehicleClass != -1;
}

bool
CVehicle::CarHasRoof(void)
{
	if((pHandling->Flags & HANDLING_HAS_NO_ROOF) == 0)
		return true;
	// component 0 is assumed to be a roof
	return m_aExtras[0] == 0 || m_aExtras[1] == 0;
}

bool
CVehicle::IsUpsideDown(void)
{
	if(GetUp().z > -0.9f)
		return false;
	return true;
}

bool
CVehicle::IsOnItsSide(void)
{
	if(GetRight().z < 0.8f && GetRight().z > -0.8f)
		return false;
	return true;
}

bool
CVehicle::CanBeDeleted(void)
{
	int i;

	if(m_nNumGettingIn || m_nGettingOutFlags)
		return false;

	if(pDriver){
		// This looks like it was inlined
		if(pDriver->CharCreatedBy == MISSION_CHAR)
			return false;
		if(pDriver->GetPedState() != PED_DRIVING &&
		   pDriver->GetPedState() != PED_DEAD)
			return false;
	}

	for(i = 0; i < ARRAY_SIZE(pPassengers); i++){
		// Same check as above
		if(pPassengers[i]){
			if(pPassengers[i]->CharCreatedBy == MISSION_CHAR)
				return false;
			if(pPassengers[i]->GetPedState() != PED_DRIVING &&
			   pPassengers[i]->GetPedState() != PED_DEAD)
				return false;
		}
		// and then again... probably because something was inlined
		if(pPassengers[i]){
			if(pPassengers[i]->GetPedState() != PED_DRIVING &&
			   pPassengers[i]->GetPedState() != PED_DEAD)
				return false;
		}
	}

	switch(VehicleCreatedBy){
	case RANDOM_VEHICLE: return true;
	case MISSION_VEHICLE: return false;
	case PARKED_VEHICLE: return true;
	case PERMANENT_VEHICLE: return false;
	}
	return true;
}

bool
CVehicle::CanPedOpenLocks(CPed *ped)
{
	if(m_nDoorLock == CARLOCK_LOCKED ||
	   m_nDoorLock == CARLOCK_LOCKED_INITIALLY ||
	   m_nDoorLock == CARLOCK_LOCKED_PLAYER_INSIDE ||
	   m_nDoorLock == CARLOCK_SKIP_SHUT_DOORS)
		return false;
	if(ped->IsPlayer() && m_nDoorLock == CARLOCK_LOCKOUT_PLAYER_ONLY)
		return false;
	return true;
}

bool
CVehicle::CanDoorsBeDamaged(void)
{
	return m_nDoorLock == CARLOCK_NOT_USED ||
		m_nDoorLock == CARLOCK_UNLOCKED ||
		m_nDoorLock == CARLOCK_SKIP_SHUT_DOORS;
}

bool
CVehicle::CanPedEnterCar(void)
{
	// can't enter when car is on side
	if(IsBike() || GetUp().z > 0.1f ||  GetUp().z < -0.1f){
		// also when car is moving too fast
		if(m_vecMoveSpeed.MagnitudeSqr() > sq(0.2f))
			return false;
		if(m_vecTurnSpeed.MagnitudeSqr() > sq(0.2f))
			return false;
		return true;
	}
	return false;
}

bool
CVehicle::CanPedExitCar(bool jumpExit)
{
	CVector up = GetUp();
	if(up.z > 0.1f || up.z < -0.1f){
		if (IsBoat())
			return true;
		// can't exit when car is moving too fast
		if(m_vecMoveSpeed.MagnitudeSqr() > 0.005f && !jumpExit)
			return false;
		// if car is slow enough, check turn speed
		if(Abs(m_vecTurnSpeed.x) > 0.01f ||
		   Abs(m_vecTurnSpeed.y) > 0.01f ||
		   Abs(m_vecTurnSpeed.z) > 0.01f)
			return false;
		return true;
	}else{
		// What is this? just > replaced by >= ??

		// can't exit when car is moving too fast
		if(m_vecMoveSpeed.MagnitudeSqr() >= 0.005f)
			return false;
		// if car is slow enough, check turn speed
		if(Abs(m_vecTurnSpeed.x) >= 0.01f ||
		   Abs(m_vecTurnSpeed.y) >= 0.01f ||
		   Abs(m_vecTurnSpeed.z) >= 0.01f)
			return false;
		return true;
	}
}

bool
CVehicle::CanPedJumpOutCar(void)
{
	if(GetUp().z < 0.3f)
		return false;
	float speed = m_vecMoveSpeed.MagnitudeSqr();
	return speed < 0.1f || speed > 0.5f ? false : true;
}

bool
CVehicle::CanPedJumpOffBike(void)
{
	if(pPassengers[0])
		return false;
	return m_vecMoveSpeed.MagnitudeSqr() < 0.07f ? false : true;
}

void
CVehicle::ChangeLawEnforcerState(uint8 enable)
{
	if (enable) {
		if (!bIsLawEnforcer) {
			bIsLawEnforcer = true;
			CCarCtrl::NumLawEnforcerCars++;
		}
	} else {
		if (bIsLawEnforcer) {
			bIsLawEnforcer = false;
			CCarCtrl::NumLawEnforcerCars--;
		}
	}
}

CPed*
CVehicle::SetUpDriver(void)
{
	if(pDriver)
		return pDriver;
	if(VehicleCreatedBy != RANDOM_VEHICLE)
		return nil;

	pDriver = CPopulation::AddPedInCar(this, true);
	pDriver->m_pMyVehicle = this;
	pDriver->m_pMyVehicle->RegisterReference((CEntity**)&pDriver->m_pMyVehicle);
	pDriver->bInVehicle = true;
	pDriver->SetPedState(PED_DRIVING);
	if(bIsBus)
		pDriver->bRenderPedInCar = false;
	return pDriver;
}

CPed*
CVehicle::SetupPassenger(int n)
{
	int i;

	if(pPassengers[n])
		return pPassengers[n];

	if((IsTaxi() || IsLimo()) && n == 0)
		pPassengers[0] = nil;
	else{
		CPed *passenger = CPopulation::AddPedInCar(this, false);
		pPassengers[n] = passenger;
		passenger->m_pMyVehicle = this;
		passenger->m_pMyVehicle->RegisterReference((CEntity**)&pPassengers[n]->m_pMyVehicle);
		passenger->bInVehicle = true;
		passenger->SetPedState(PED_DRIVING);

		if(passenger->m_nPedType == PEDTYPE_CIVMALE || passenger->m_nPedType == PEDTYPE_CIVFEMALE)
			for(i = 0; i < n; i++)
				if(pPassengers[i] && pPassengers[n] &&
				   (pPassengers[i]->m_nPedType == PEDTYPE_CIVMALE || pPassengers[i]->m_nPedType == PEDTYPE_CIVFEMALE) &&
				   passenger->GetModelIndex() == pPassengers[i]->GetModelIndex()){
					pPassengers[n] = nil;
					CPopulation::RemovePed(passenger);
				}
	}
	if(bIsBus && pPassengers[n])
		pPassengers[n]->bRenderPedInCar = false;
	++m_nNumPassengers;
	return pPassengers[n];
}

void
CVehicle::SetDriver(CPed *driver)
{
	pDriver = driver;
	pDriver->RegisterReference((CEntity**)&pDriver);

	if(bFreebies && driver == FindPlayerPed()){
		bFreebies = false;
		switch(GetModelIndex()){
		case MI_AMBULAN:
			FindPlayerPed()->m_fHealth = Max(FindPlayerPed()->m_fHealth, Min(FindPlayerPed()->m_fHealth + 20.0f, CWorld::Players[0].m_nMaxHealth));
			break;

		case MI_TAXI:
		case MI_CABBIE:
		case MI_ZEBRA:
		case MI_KAUFMAN:
			CWorld::Players[CWorld::PlayerInFocus].m_nMoney += 12;
			break;

		case MI_POLICE:
			CStreaming::RequestModel(MI_SHOTGUN, STREAMFLAGS_DONT_REMOVE);
			bFreebies = true;
			break;

		case MI_ENFORCER:
			driver->m_fArmour = Max(driver->m_fArmour, CWorld::Players[0].m_nMaxArmour);
			break;

		case MI_CADDY:
			if(!(driver->IsPlayer() && ((CPlayerPed*)driver)->DoesPlayerWantNewWeapon(WEAPONTYPE_GOLFCLUB, true)))
				CStreaming::RequestModel(MI_GOLFCLUB, STREAMFLAGS_DONT_REMOVE);
			break;
		}
	}

	if(IsBike())
		ApplyMoveForce(-0.2f*driver->m_fMass * GetUp());
	else
		ApplyTurnForce(0.0f, 0.0f, -0.2f*driver->m_fMass,
			driver->GetPosition().x - GetPosition().x,
			driver->GetPosition().y - GetPosition().y,
			0.0f);
}

bool
CVehicle::AddPassenger(CPed *passenger)
{
	int i;

	if(IsBike())
		ApplyTurnForce(-0.2f*passenger->m_fMass * GetUp(), -0.1f*GetForward());
	else
		ApplyTurnForce(0.0f, 0.0f, -0.2f*passenger->m_fMass,
			passenger->GetPosition().x - GetPosition().x,
			passenger->GetPosition().y - GetPosition().y,
			0.0f);

	for(i = 0; i < m_nNumMaxPassengers; i++)
		if(pPassengers[i] == nil){
			pPassengers[i] = passenger;
			m_nNumPassengers++;
			return true;
		}
	return false;
}

bool
CVehicle::AddPassenger(CPed *passenger, uint8 n)
{
	if(bIsBus)
		return AddPassenger(passenger);

	if(IsBike())
		ApplyTurnForce(-0.2f*passenger->m_fMass * GetUp(), -0.1f*GetForward());
	else
		ApplyTurnForce(0.0f, 0.0f, -0.2f*passenger->m_fMass,
			passenger->GetPosition().x - GetPosition().x,
			passenger->GetPosition().y - GetPosition().y,
			0.0f);

	if(n < m_nNumMaxPassengers && pPassengers[n] == nil){
		pPassengers[n] = passenger;
		m_nNumPassengers++;
		return true;
	}
	return false;
}

void
CVehicle::RemoveDriver(void)
{
	SetStatus(STATUS_ABANDONED);
	if(pDriver == FindPlayerPed()){
		if(GetModelIndex() == MI_POLICE && CStreaming::HasModelLoaded(MI_SHOTGUN)){
			if(bFreebies){
				if(((CPlayerPed*)pDriver)->DoesPlayerWantNewWeapon(WEAPONTYPE_SHOTGUN, true))
					pDriver->GiveWeapon(WEAPONTYPE_SHOTGUN, 5, true);
				else
					pDriver->GrantAmmo(WEAPONTYPE_SHOTGUN, 5);
				bFreebies = false;
			}
			CStreaming::SetModelIsDeletable(MI_SHOTGUN);
		}else if(GetModelIndex() == MI_CADDY && CStreaming::HasModelLoaded(MI_GOLFCLUB)){
			if(((CPlayerPed*)pDriver)->DoesPlayerWantNewWeapon(WEAPONTYPE_GOLFCLUB, true))
				pDriver->GiveWeapon(WEAPONTYPE_GOLFCLUB, 1, true);
			CStreaming::SetModelIsDeletable(MI_GOLFCLUB);
		}
	}
	pDriver = nil;
}

void
CVehicle::RemovePassenger(CPed *p)
{
	if (IsTrain()){
		for (int i = 0; i < ARRAY_SIZE(pPassengers); i++){
			if (pPassengers[i] == p) {
				pPassengers[i] = nil;
				m_nNumPassengers--;
				return;
			}
		}
		return;
	}
	for (int i = 0; i < m_nNumMaxPassengers; i++){
		if (pPassengers[i] == p){
			pPassengers[i] = nil;
			m_nNumPassengers--;
			return;
		}
	}
}

bool
CVehicle::IsDriver(CPed *ped)
{
	if(ped == nil)
		return false;
	return ped == pDriver;
}

bool
CVehicle::IsDriver(int32 model)
{
	return pDriver && pDriver->GetModelIndex() == model;
}

bool
CVehicle::IsPassenger(CPed *ped)
{
	int i;
	if(ped == nil)
		return false;
	for(i = 0; i < 8; i++)
		if(pPassengers[i] == ped)
			return true;
	return false;
}

bool
CVehicle::IsPassenger(int32 model)
{
	int i;
	for(i = 0; i < 8; i++)
		if(pPassengers[i] && pPassengers[i]->GetModelIndex() == model)
			return true;
	return false;
}

void
CVehicle::UpdatePassengerList(void)
{
	int i;
	bool hasPassenger = false;
	if(m_nNumPassengers)
		for(i = 0; i < 8; i++)
			if(pPassengers[i]){
				hasPassenger = true;
				break;
			}
	if(!hasPassenger)
		m_nNumPassengers = 0;
}

void
CVehicle::ProcessCarAlarm(void)
{
	uint32 step;

	if(!IsAlarmOn())
		return;

	step = CTimer::GetTimeStepInMilliseconds();
	if((uint16)m_nAlarmState < step){
		m_nAlarmState = 0;
		m_nCarHornTimer = 0;
	}else
		m_nAlarmState -= step;
}

bool
CVehicle::IsSphereTouchingVehicle(float sx, float sy, float sz, float radius)
{
	float x, y, z;
	// sphere relative to vehicle
	CVector sph = CVector(sx, sy, sz) - GetPosition();
	CColModel *colmodel = GetColModel();

	x = DotProduct(sph, GetRight());
	if(colmodel->boundingBox.min.x - radius > x ||
	   colmodel->boundingBox.max.x + radius < x)
		return false;
	y = DotProduct(sph, GetForward());
	if(colmodel->boundingBox.min.y - radius > y ||
	   colmodel->boundingBox.max.y + radius < y)
		return false;
	z = DotProduct(sph, GetUp());
	if(colmodel->boundingBox.min.z - radius > z ||
	   colmodel->boundingBox.max.z + radius < z)
		return false;

	return true;
}

RpMaterial*
SetCompAlphaCB(RpMaterial *material, void *data)
{
	uint32 alpha = (uint32)(uintptr)data;
	RwRGBA *col = (RwRGBA*)RpMaterialGetColor(material);	// get rid of const
	col->alpha = alpha;
	return material;
}

void
CVehicle::SetComponentAtomicAlpha(RpAtomic *atomic, int32 alpha)
{
	RpGeometry *geo = RpAtomicGetGeometry(atomic);
	RpGeometrySetFlags(geo, RpGeometryGetFlags(geo) | rpGEOMETRYMODULATEMATERIALCOLOR);
	RpGeometryForAllMaterials(geo, SetCompAlphaCB, (void*)alpha);
}

void
CVehicle::UpdateClumpAlpha(void)
{
	int clumpAlpha = CVisibilityPlugins::GetClumpAlpha((RpClump*)m_rwObject);
	if(bFadeOut){
		clumpAlpha -= 8;
		if(clumpAlpha < 0)
			clumpAlpha = 0;
	}else if(clumpAlpha < 255){
		clumpAlpha += 16;
		if(clumpAlpha > 255)
			clumpAlpha = 255;
	}
	CVisibilityPlugins::SetClumpAlpha((RpClump*)m_rwObject, clumpAlpha);
}

void
CVehicle::HeliDustGenerate(CEntity *heli, float radius, float ground, int rnd)
{
	int i;
	float angle;
	CColPoint point;
	CEntity *entity;
	uint8 r, g, b;

	if(heli == nil)
		return;

	uint8 surface = SURFACE_TARMAC;
	int frm = CTimer::GetFrameCounter() & 7;
	float testLowZ = ground - 10.0f;
	float dustSize = 0.0f;
	float baseSize = 1.0f;
	float offset = 1.0f;	// when heli is tilted
	float particleZ = -101.0f;
	int n = 0;

	if(heli->GetModelIndex() == MI_RCGOBLIN || heli->GetModelIndex() == MI_RCRAIDER){
		radius = 3.0f;
		dustSize = 0.04f;
		baseSize = 0.07f;
		offset = 0.3f;
	}

	CVector heliPos = heli->GetPosition();

	if(heli->IsVehicle() && ((CVehicle*)heli)->IsCar()){
		heliPos.x -= (heliPos.z - ground)*heli->GetUp().x*offset*0.5f;
		heliPos.y -= (heliPos.z - ground)*heli->GetUp().y*offset*0.5f;
	}

	float steamSize = 0.25f * radius * baseSize;
	float splashSize = 0.3f * radius * baseSize;

	i = 0;
	for(i = 0; i < 32+rnd; i++){
		angle = i * TWOPI/32.0f;
		CVector pos(radius*Cos(angle), radius*Sin(angle), 0.0f);
		CVector dir = CVector(pos.x, pos.y, 1.0f)*0.01f;
		pos += heliPos;

		if(i < 32 && i == 4*frm){
			if(CWorld::ProcessVerticalLine(pos, testLowZ, point, entity, true, false, false, false, true, false, nil)){
				n = rnd;
				particleZ = point.point.z;
				surface = point.surfaceB;
			}else
				n = 0;

			float waterLevel = 0.0f;
			if(CWaterLevel::GetWaterLevel(pos, &waterLevel, false) && waterLevel > particleZ){
				surface = SURFACE_WATER;
				n = rnd;
				particleZ = waterLevel;
			}
		}

		if(n){
			pos.z = particleZ;
			if(surface == SURFACE_WATER){
				float red = (0.3*CTimeCycle::GetDirectionalRed() + CTimeCycle::GetAmbientRed_Obj())*255.0f/4.0f;
				float green = (0.3*CTimeCycle::GetDirectionalGreen() + CTimeCycle::GetAmbientGreen_Obj())*255.0f/4.0f;
				float blue = (0.3*CTimeCycle::GetDirectionalBlue() + CTimeCycle::GetAmbientBlue_Obj())*255.0f/4.0f;
				r = clamp(red, 0.0f, 255.0f);
				g = clamp(green, 0.0f, 255.0f);
				b = clamp(blue, 0.0f, 255.0f);
				RwRGBA col1 = { r, g, b, CGeneral::GetRandomNumberInRange(8, 32) };
				RwRGBA col2 = { 255, 255, 255, 32 };

				if(n&1)
					CParticle::AddParticle(PARTICLE_STEAM_NY_SLOWMOTION, pos, dir, nil, steamSize, col2);
				else
					CParticle::AddParticle(PARTICLE_CAR_SPLASH, pos, dir, nil, splashSize, col1,
						CGeneral::GetRandomNumberInRange(0.0f, 10.0f),
						CGeneral::GetRandomNumberInRange(0.0f, 90.0f), 1);
			}else{
				switch(surface){
				default:
				case SURFACE_TARMAC:
					r = 10;
					g = 10;
					b = 10;
					break;
				case SURFACE_GRASS:
					r = 10;
					g = 10;
					b = 3;
					break;
				case SURFACE_GRAVEL:
					r = 10;
					g = 8;
					b = 7;
					break;
				case SURFACE_MUD_DRY:
					r = 10;
					g = 6;
					b = 3;
					break;
				case SURFACE_SAND:
				case SURFACE_SAND_BEACH:
					r = 10;
					g = 10;
					b = 7;
					break;
				}
				RwRGBA col = { r, g, b, 32 };
				if(heliPos.z - pos.z < 20.0f)
					CParticle::AddParticle(PARTICLE_HELI_DUST, pos, dir, nil, dustSize, col);
			}

			n--;
		}
	}
}

#define GLARE_MIN_DIST (13.0f)
#define GLARE_FULL_DIST (30.0f)
#define GLARE_MIN_ANGLE (0.99f)
#define GLARE_FULL_ANGLE (0.995f)

void
CVehicle::DoSunGlare(void)
{
	if(bRenderScorched || GetPosition().z < 0.0f ||
	   GetVehicleAppearance() != VEHICLE_APPEARANCE_CAR || CWeather::SunGlare <= 0.0f)
		return;

	CVector camDir = TheCamera.GetPosition() - GetPosition();
	float dist = camDir.Magnitude();
	camDir *= 2.0f/dist;
	CVector glareVec = camDir + CTimeCycle::GetSunDirection();
	CVector localGlareVec;
	localGlareVec.x = DotProduct(glareVec, GetRight());
	localGlareVec.y = DotProduct(glareVec, GetForward());
	localGlareVec.z = 0.0;
	localGlareVec.Normalise();

	CVector2D fwd2D = GetForward();
	fwd2D.Normalise();
	CVector2D camDir2D = camDir;
	camDir2D.Normalise();
	float fwdness = Abs(DotProduct2D(fwd2D, camDir2D));

	// check angle
	float strength;
	if(fwdness > GLARE_FULL_ANGLE)
		strength = 1.0f;
	else if(fwdness > GLARE_MIN_ANGLE)
		strength = (fwdness - GLARE_MIN_ANGLE)/(GLARE_FULL_ANGLE-GLARE_MIN_ANGLE);
	else
		return;
	// check distance
	if(dist > GLARE_FULL_DIST){
		// no max distance
	}else if(dist > GLARE_MIN_DIST)
		strength *= (dist - GLARE_MIN_DIST)/(GLARE_FULL_DIST - GLARE_MIN_DIST);
	else
		return;

	float intens = 0.8f * strength * CWeather::SunGlare;
	int r = intens * (CTimeCycle::GetSunCoreRed() + 2*255)/3.0f;
	int g = intens * (CTimeCycle::GetSunCoreGreen() + 2*255)/3.0f;
	int b = intens * (CTimeCycle::GetSunCoreBlue() + 2*255)/3.0f;

	CColModel *colmodel = GetColModel();
	CCollision::CalculateTrianglePlanes(colmodel);

	int i;
	for(i = 0; i < colmodel->numTriangles-2; i += 2){
		int a1 = colmodel->triangles[i].a;
		int b1 = colmodel->triangles[i].b;
		int c1 = colmodel->triangles[i].c;
		int a2 = colmodel->triangles[i+1].a;
		int b2 = colmodel->triangles[i+1].b;
		int c2 = colmodel->triangles[i+1].c;
		CVector vert1 = colmodel->vertices[a1];
		CVector vert4;
		// Need an upward surface
		if(vert1.z <= 0.0f)
			continue;

		// trying to find a quad here
		int numTri2Verts = 0;
		if(a2 != a1 && a2 != b1 && a2 != c1){
			// a2 is not in tri1
			numTri2Verts++;
			vert4 = colmodel->vertices[a2];
		}
		if(b2 != a1 && b2 != b1 && b2 != c1){
			// b2 is not in tri1
			numTri2Verts++;
			vert4 = colmodel->vertices[b2];
		}
		if(c2 != a1 && c2 != b1 && c2 != c1){
			// c2 is not in tri1
			numTri2Verts++;
			vert4 = colmodel->vertices[c2];
		}
		// Need exactly one vertex from tri2 for a quad with tri1
		if(numTri2Verts != 1)
			continue;

		CVector mid = (vert1 + colmodel->vertices[b1] + colmodel->vertices[c1] + vert4)/4.0f;
		float dy = mid.y - vert1.y;
		float dx = mid.x - vert1.x;
		float dist = 1.4f * Min(Abs(dx), Abs(dy));
		if(dist > 0.6f){
			CVector pos = GetMatrix() * (dist * localGlareVec + mid) + camDir;
			CCoronas::RegisterCorona((uintptr)this + 27 + i,
				r, g, b, 255,
				pos, 0.9f*CWeather::SunGlare, 90.0f,
				CCoronas::TYPE_STAR, CCoronas::FLARE_NONE,
				CCoronas::REFLECTION_OFF, CCoronas::LOSCHECK_OFF,
				CCoronas::STREAK_OFF, 0.0f);
		}
	}
}

void
CVehicle::KillPedsInVehicle(void)
{
	int i;
	if(pDriver){
		CDarkel::RegisterKillByPlayer(pDriver, WEAPONTYPE_EXPLOSION);
		if(pDriver->GetPedState() == PED_DRIVING){
			pDriver->SetDead();
			if(!pDriver->IsPlayer())
				pDriver->FlagToDestroyWhenNextProcessed();
		}else
			pDriver->SetDie(ANIM_KO_SHOT_FRONT1, 4.0f, 0.0f);
	}
	for(i = 0; i < m_nNumMaxPassengers; i++){
		if(pPassengers[i]){
			CDarkel::RegisterKillByPlayer(pPassengers[i], WEAPONTYPE_EXPLOSION);
			if(pPassengers[i]->GetPedState() == PED_DRIVING){
				pPassengers[i]->SetDead();
				if(!pPassengers[i]->IsPlayer())
					pPassengers[i]->FlagToDestroyWhenNextProcessed();
			}else
				pPassengers[i]->SetDie(ANIM_KO_SHOT_FRONT1, 4.0f, 0.0f);
		}
	}
}

void
DestroyVehicleAndDriverAndPassengers(CVehicle* pVehicle)
{
	if (pVehicle->pDriver) {
		CDarkel::RegisterKillByPlayer(pVehicle->pDriver, WEAPONTYPE_UNIDENTIFIED);
		pVehicle->pDriver->FlagToDestroyWhenNextProcessed();
	}
	for (int i = 0; i < pVehicle->m_nNumMaxPassengers; i++) {
		if (pVehicle->pPassengers[i]) {
			CDarkel::RegisterKillByPlayer(pVehicle->pPassengers[i], WEAPONTYPE_UNIDENTIFIED);
			pVehicle->pPassengers[i]->FlagToDestroyWhenNextProcessed();
		}
	}
	CWorld::Remove(pVehicle);
	delete pVehicle;
}

#ifdef COMPATIBLE_SAVES
void
CVehicle::Save(uint8*& buf)
{
	SkipSaveBuf(buf, 4);
	WriteSaveBuf<float>(buf, GetRight().x);
	WriteSaveBuf<float>(buf, GetRight().y);
	WriteSaveBuf<float>(buf, GetRight().z);
	SkipSaveBuf(buf, 4);
	WriteSaveBuf<float>(buf, GetForward().x);
	WriteSaveBuf<float>(buf, GetForward().y);
	WriteSaveBuf<float>(buf, GetForward().z);
	SkipSaveBuf(buf, 4);
	WriteSaveBuf<float>(buf, GetUp().x);
	WriteSaveBuf<float>(buf, GetUp().y);
	WriteSaveBuf<float>(buf, GetUp().z);
	SkipSaveBuf(buf, 4);
	WriteSaveBuf<float>(buf, GetPosition().x);
	WriteSaveBuf<float>(buf, GetPosition().y);
	WriteSaveBuf<float>(buf, GetPosition().z);
	SkipSaveBuf(buf, 16);
	SaveEntityFlags(buf);
	SkipSaveBuf(buf, 212);
	AutoPilot.Save(buf);
	WriteSaveBuf<int8>(buf, m_currentColour1);
	WriteSaveBuf<int8>(buf, m_currentColour2);
	SkipSaveBuf(buf, 2);
	WriteSaveBuf<int16>(buf, m_nAlarmState);
	SkipSaveBuf(buf, 43);
	WriteSaveBuf<uint8>(buf, m_nNumMaxPassengers);
	SkipSaveBuf(buf, 2);
	WriteSaveBuf<float>(buf, field_1D0[0]);
	WriteSaveBuf<float>(buf, field_1D0[1]);
	WriteSaveBuf<float>(buf, field_1D0[2]);
	WriteSaveBuf<float>(buf, field_1D0[3]);
	SkipSaveBuf(buf, 8);
	WriteSaveBuf<float>(buf, m_fSteerAngle);
	WriteSaveBuf<float>(buf, m_fGasPedal);
	WriteSaveBuf<float>(buf, m_fBrakePedal);
	WriteSaveBuf<uint8>(buf, VehicleCreatedBy);
	uint8 flags = 0;
	if (bIsLawEnforcer) flags |= BIT(0);
	if (bIsLocked) flags |= BIT(3);
	if (bEngineOn) flags |= BIT(4);
	if (bIsHandbrakeOn) flags |= BIT(5);
	if (bLightsOn) flags |= BIT(6);
	if (bFreebies) flags |= BIT(7);
	WriteSaveBuf<uint8>(buf, flags);
	SkipSaveBuf(buf, 10);
	WriteSaveBuf<float>(buf, m_fHealth);
	WriteSaveBuf<uint8>(buf, m_nCurrentGear);
	SkipSaveBuf(buf, 3);
	WriteSaveBuf<float>(buf, m_fChangeGearTime);
	SkipSaveBuf(buf, 4);
	WriteSaveBuf<uint32>(buf, m_nTimeOfDeath);
	SkipSaveBuf(buf, 2);
	WriteSaveBuf<int16>(buf, m_nBombTimer);
	SkipSaveBuf(buf, 12);
	WriteSaveBuf<int8>(buf, m_nDoorLock);
	SkipSaveBuf(buf, 99);
}

void
CVehicle::Load(uint8*& buf)
{
	CMatrix tmp;
	SkipSaveBuf(buf, 4);
	tmp.GetRight().x = ReadSaveBuf<float>(buf);
	tmp.GetRight().y = ReadSaveBuf<float>(buf);
	tmp.GetRight().z = ReadSaveBuf<float>(buf);
	SkipSaveBuf(buf, 4);
	tmp.GetForward().x = ReadSaveBuf<float>(buf);
	tmp.GetForward().y = ReadSaveBuf<float>(buf);
	tmp.GetForward().z = ReadSaveBuf<float>(buf);
	SkipSaveBuf(buf, 4);
	tmp.GetUp().x = ReadSaveBuf<float>(buf);
	tmp.GetUp().y = ReadSaveBuf<float>(buf);
	tmp.GetUp().z = ReadSaveBuf<float>(buf);
	SkipSaveBuf(buf, 4);
	tmp.GetPosition().x = ReadSaveBuf<float>(buf);
	tmp.GetPosition().y = ReadSaveBuf<float>(buf);
	tmp.GetPosition().z = ReadSaveBuf<float>(buf);
	m_matrix = tmp;
	SkipSaveBuf(buf, 16);
	LoadEntityFlags(buf);
	SkipSaveBuf(buf, 212);
	AutoPilot.Load(buf);
	m_currentColour1 = ReadSaveBuf<int8>(buf);
	m_currentColour2 = ReadSaveBuf<int8>(buf);
	SkipSaveBuf(buf, 2);
	m_nAlarmState = ReadSaveBuf<int16>(buf);
	SkipSaveBuf(buf, 43);
	m_nNumMaxPassengers = ReadSaveBuf<int8>(buf);
	SkipSaveBuf(buf, 2);
	field_1D0[0] = ReadSaveBuf<float>(buf);
	field_1D0[1] = ReadSaveBuf<float>(buf);
	field_1D0[2] = ReadSaveBuf<float>(buf);
	field_1D0[3] = ReadSaveBuf<float>(buf);
	SkipSaveBuf(buf, 8);
	m_fSteerAngle = ReadSaveBuf<float>(buf);
	m_fGasPedal = ReadSaveBuf<float>(buf);
	m_fBrakePedal = ReadSaveBuf<float>(buf);
	VehicleCreatedBy = ReadSaveBuf<uint8>(buf);
	uint8 flags = ReadSaveBuf<uint8>(buf);
	bIsLawEnforcer = !!(flags & BIT(0));
	bIsLocked = !!(flags & BIT(3));
	bEngineOn = !!(flags & BIT(4));
	bIsHandbrakeOn = !!(flags & BIT(5));
	bLightsOn = !!(flags & BIT(6));
	bFreebies = !!(flags & BIT(7));
	SkipSaveBuf(buf, 10);
	m_fHealth = ReadSaveBuf<float>(buf);
	m_nCurrentGear = ReadSaveBuf<uint8>(buf);
	SkipSaveBuf(buf, 3);
	m_fChangeGearTime = ReadSaveBuf<float>(buf);
	SkipSaveBuf(buf, 4);
	m_nTimeOfDeath = ReadSaveBuf<uint32>(buf);
	SkipSaveBuf(buf, 2);
	m_nBombTimer = ReadSaveBuf<int16>(buf);
	SkipSaveBuf(buf, 12);
	m_nDoorLock = (eCarLock)ReadSaveBuf<int8>(buf);
	SkipSaveBuf(buf, 99);
}
#endif

eVehicleAppearance
CVehicle::GetVehicleAppearance(void)
{
	uint32 flags = pHandling->Flags & 0xF0000;
	if (flags == 0)
		return VEHICLE_APPEARANCE_CAR;
	if (flags == HANDLING_IS_BIKE)
		return VEHICLE_APPEARANCE_BIKE;
	if (flags == HANDLING_IS_HELI)
		return VEHICLE_APPEARANCE_HELI;
	if (flags == HANDLING_IS_PLANE)
		return VEHICLE_APPEARANCE_PLANE;
	if (flags == HANDLING_IS_BOAT)
		return VEHICLE_APPEARANCE_BOAT;
	return VEHICLE_APPEARANCE_NONE;
}

bool
IsVehiclePointerValid(CVehicle* pVehicle)
{
	if (!pVehicle)
		return false;
	int index = CPools::GetVehiclePool()->GetJustIndex(pVehicle);
#ifdef FIX_BUGS
	if (index < 0 || index >= NUMVEHICLES)
#else
	if (index < 0 || index > NUMVEHICLES)
#endif
		return false;
	return pVehicle->m_vehType == VEHICLE_TYPE_PLANE || pVehicle->m_entryInfoList.first;
}
