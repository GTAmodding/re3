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
#include "DMAudio.h"
#include "Radar.h"
#include "Fire.h"
#include "Darkel.h"

bool CVehicle::bWheelsOnlyCheat;
bool CVehicle::bAllDodosCheat;
bool CVehicle::bCheat3;
bool CVehicle::bCheat4;
bool CVehicle::bCheat5;
#ifdef ALT_DODO_CHEAT
bool CVehicle::bAltDodoCheat;
#endif
bool CVehicle::m_bDisableMouseSteering = true;

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
	m_fSteerInput = 0.0f;
	m_type = ENTITY_TYPE_VEHICLE;
	VehicleCreatedBy = CreatedBy;
	bIsLocked = false;
	bIsLawEnforcer = false;
	bIsAmbulanceOnDuty = false;
	bIsFireTruckOnDuty = false;
#ifdef FIX_BUGS
	bIsHandbrakeOn = false;
#endif
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
	m_bSirenOrAlarm = false;
	m_nCarHornTimer = 0;
	m_nCarHornPattern = 0;
	m_nAlarmState = 0;
	m_nDoorLock = CARLOCK_UNLOCKED;
	m_nLastWeaponDamage = -1;
	m_fMapObjectHeightAhead = m_fMapObjectHeightBehind = 0.0f;
	m_audioEntityId = DMAudio.CreateEntity(AUDIOTYPE_PHYSICAL, this);
	if(m_audioEntityId >= 0)
		DMAudio.SetEntityStatus(m_audioEntityId, true);
	m_nRadioStation = CGeneral::GetRandomNumber() % USERTRACK;
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

float
CVehicle::GetHeightAboveRoad(void)
{
	return -1.0f * GetColModel()->boundingBox.min.z;
}

const float fRCPropFallOff = 3.0f;
const float fRCAeroThrust = 0.003f;
const float fRCSideSlipMult = 0.1f;
const float fRCRudderMult = 0.2f;
const float fRCYawMult = -0.01f;
const float fRCRollMult = 0.02f;
const float fRCRollStabilise = -0.08f;
const float fRCPitchMult = 0.005f;
const float fRCTailMult = 0.3f;
const float fRCFormLiftMult = 0.02f;
const float fRCAttackLiftMult = 0.25f;
const CVector vecRCAeroResistance(0.998f, 0.998f, 0.9f);

const float fSeaPropFallOff = 2.3f;
const float fSeaThrust = 0.002f;
const float fSeaSideSlipMult = 0.1f;
const float fSeaRudderMult = 0.01f;
const float fSeaYawMult = -0.0003f;
const float fSeaRollMult = 0.0015f;
const float fSeaRollStabilise = -0.01f;
const float fSeaPitchMult = 0.0002f;
const float fSeaTailMult = 0.01f;
const float fSeaFormLiftMult = 0.012f;
const float fSeaAttackLiftMult = 0.1f;
const CVector vecSeaAeroResistance(0.995f, 0.995f, 0.85f);

const float fSpeedResistanceY = 500.0f;
const float fSpeedResistanceZ = 500.0f;

const CVector vecHeliMoveRes(0.995f, 0.995f, 0.99f);
const CVector vecRCHeliMoveRes(0.99f, 0.99f, 0.99f);
const float fThrustVar = 0.3f;
const float fRotorFallOff = 0.75f;
const float fStabiliseVar = 0.015f;
const float fPitchBrake = 10.0f;
const float fPitchVar = 0.006f;
const float fRollVar = 0.006f;
const float fYawVar = -0.001f;
const CVector vecHeliResistance(0.81f, 0.85f, 0.99f);
const CVector vecRCHeliResistance(0.92f, 0.92f, 0.998f);
const float fSpinSpeedRes = 20.0f;

void
CVehicle::FlyingControl(eFlightModel flightModel)
{
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
	{
		// thrust
		float fThrust = flightModel == FLIGHT_MODEL_RCPLANE ? fRCAeroThrust : fSeaThrust;
		float fThrustFallOff = flightModel == FLIGHT_MODEL_RCPLANE ? fRCPropFallOff : fSeaPropFallOff;
		
		float fForwSpeed = DotProduct(GetMoveSpeed(), GetForward());
		CVector vecTail = GetColModel()->boundingBox.min.y * GetForward();
		float fPedalState = (CPad::GetPad(0)->GetAccelerate() - CPad::GetPad(0)->GetBrake()) / 255.0f;
		if (fForwSpeed > 0.1f || (flightModel == FLIGHT_MODEL_RCPLANE && fForwSpeed > 0.02f))
			fPedalState += 1.0f;
		else if (fForwSpeed > 0.0f && fPedalState < 0.0f)
			fPedalState = 0.0f;
		float fThrustAccel = (fPedalState - fThrustFallOff * fForwSpeed) * fThrust;
			
		ApplyMoveForce(fThrustAccel * GetForward() * m_fMass * CTimer::GetTimeStep());

		// left/right
		float fSideSpeed = -DotProduct(GetMoveSpeed(), GetRight());
		float fSteerLR = CPad::GetPad(0)->GetSteeringLeftRight() / 128.0f;
		float fSideSlipAccel;
		if (flightModel == FLIGHT_MODEL_RCPLANE)
			fSideSlipAccel = Abs(fSideSpeed) * fSideSpeed * fRCSideSlipMult;
		else
			fSideSlipAccel = Abs(fSideSpeed) * fSideSpeed * fSeaSideSlipMult;
		ApplyMoveForce(m_fMass * GetRight() * fSideSlipAccel * CTimer::GetTimeStep());

		float fYaw = -DotProduct(GetSpeed(vecTail), GetRight());
		float fYawAccel;
		if (flightModel == FLIGHT_MODEL_RCPLANE)
			fYawAccel = fRCRudderMult * fYaw * Abs(fYaw) + fRCYawMult * fSteerLR * fForwSpeed;
		else
			fYawAccel = fSeaRudderMult * fYaw * Abs(fYaw) + fSeaYawMult * fSteerLR * fForwSpeed;
		ApplyTurnForce(fYawAccel * GetRight() * m_fTurnMass * CTimer::GetTimeStep(), vecTail);

		float fRollAccel;
		if (flightModel == FLIGHT_MODEL_RCPLANE) {
			float fDirectionMultiplier = CPad::GetPad(0)->GetLookRight();
			if (CPad::GetPad(0)->GetLookLeft())
				fDirectionMultiplier = -1;
			fRollAccel = (0.5f * fDirectionMultiplier + fSteerLR) * fRCRollMult;
		}
		else
			fRollAccel = fSteerLR * fSeaRollMult;
		ApplyTurnForce(GetRight() * fRollAccel * fForwSpeed * m_fTurnMass * CTimer::GetTimeStep(), GetUp());

		CVector vecFRight = CrossProduct(GetForward(), CVector(0.0f, 0.0f, 1.0f));
		CVector vecStabilise = (GetUp().z > 0.0f) ? vecFRight : -vecFRight;
		float fStabiliseDirection = (GetRight().z > 0.0f) ? -1.0f : 1.0f;
		float fStabiliseSpeed;
		if (flightModel == FLIGHT_MODEL_RCPLANE)
			fStabiliseSpeed = fRCRollStabilise * fStabiliseDirection * (1.0f - DotProduct(GetRight(), vecStabilise)) * (1.0f - Abs(GetForward().z));
		else
			fStabiliseSpeed = fSeaRollStabilise * fStabiliseDirection * (1.0f - DotProduct(GetRight(), vecStabilise)) * (1.0f - Abs(GetForward().z));
		ApplyTurnForce(fStabiliseSpeed * m_fTurnMass * GetRight(), GetUp()); // no CTimer::GetTimeStep(), is it right? VC doesn't have it too

		// up/down
		float fTail = -DotProduct(GetSpeed(vecTail), GetUp());
		float fSteerUD = -CPad::GetPad(0)->GetSteeringUpDown() / 128.0f;
		float fPitchAccel;
		if (flightModel == FLIGHT_MODEL_RCPLANE)
			fPitchAccel = fRCTailMult * fTail * Abs(fTail) + fRCPitchMult * fSteerUD * fForwSpeed;
		else
			fPitchAccel = fSeaTailMult * fTail * Abs(fTail) + fSeaPitchMult * fSteerUD * fForwSpeed;
		ApplyTurnForce(fPitchAccel * m_fTurnMass * GetUp() * CTimer::GetTimeStep(), vecTail);

		float fLift = DotProduct(GetMoveSpeed(), GetUp()) / Max(0.01f, GetMoveSpeed().Magnitude()); //accel*angle
		float fLiftAccel;
		if (flightModel == FLIGHT_MODEL_RCPLANE)
			fLiftAccel = (fRCFormLiftMult - fRCAttackLiftMult * fLift) * SQR(fForwSpeed);
		else
			fLiftAccel = (fSeaFormLiftMult - fSeaAttackLiftMult * fLift) * SQR(fForwSpeed);
		float fLiftImpulse = fLiftAccel * m_fMass * CTimer::GetTimeStep();
		if (GRAVITY * CTimer::GetTimeStep() * m_fMass < fLiftImpulse) {
			if (flightModel == FLIGHT_MODEL_RCPLANE && GetPosition().z > 50.0f)
				fLiftImpulse = CTimer::GetTimeStep() * 0.9f*GRAVITY * m_fMass;
			else if (flightModel == FLIGHT_MODEL_SEAPLANE && GetPosition().z > 80.0f)
				fLiftImpulse = CTimer::GetTimeStep() * 0.9f*GRAVITY * m_fMass;
		}
		ApplyMoveForce(fLiftImpulse * GetUp());

		CVector vecResistance;
		if (flightModel == FLIGHT_MODEL_RCPLANE)
			vecResistance = vecRCAeroResistance;
		else
			vecResistance = vecSeaAeroResistance;
		float rX = Pow(vecResistance.x, CTimer::GetTimeStep());
		float rY = Pow(vecResistance.y, CTimer::GetTimeStep());
		float rZ = Pow(vecResistance.z, CTimer::GetTimeStep());
		CVector vecTurnSpeed = Multiply3x3(m_vecTurnSpeed, GetMatrix());
		vecTurnSpeed.x *= rX;
		float fResistance = vecTurnSpeed.y * (1.0f / (fSpeedResistanceY * SQR(vecTurnSpeed.y) + 1.0f)) * rY - vecTurnSpeed.y;
		vecTurnSpeed.z *= rZ;
		m_vecTurnSpeed = Multiply3x3(GetMatrix(), vecTurnSpeed);
		ApplyTurnForce(-GetUp() * fResistance * m_fTurnMass, GetRight() + Multiply3x3(GetMatrix(), m_vecCentreOfMass));
		break;
	}
	case FLIGHT_MODEL_HELI:
	{
		CVector vecMoveResistance;
		if (GetModelIndex() == MI_MIAMI_SPARROW)
			vecMoveResistance = vecHeliMoveRes;
		else
			vecMoveResistance = vecRCHeliMoveRes;
		float rmX = Pow(vecMoveResistance.x, CTimer::GetTimeStep());
		float rmY = Pow(vecMoveResistance.y, CTimer::GetTimeStep());
		float rmZ = Pow(vecMoveResistance.z, CTimer::GetTimeStep());
		m_vecMoveSpeed.x *= rmX;
		m_vecMoveSpeed.y *= rmY;
		m_vecMoveSpeed.z *= rmZ;
		if (GetStatus() != STATUS_PLAYER && GetStatus() != STATUS_PLAYER_REMOTE)
			return;
		float fThrust;
		if (bCheat5)
			fThrust = CPad::GetPad(0)->GetSteeringUpDown() * fThrustVar / 128.0f + 0.95f;
		else
			fThrust = fThrustVar * (CPad::GetPad(0)->GetAccelerate() - 2 * CPad::GetPad(0)->GetBrake()) / 255.0f + 0.95f;
		fThrust -= fRotorFallOff * DotProduct(m_vecMoveSpeed, GetUp());
#if GTA_VERSION >= GTA3_PC_11
		if (fThrust > 0.9f && GetPosition().z > 80.0f)
			fThrust = 0.9f;
#endif
		ApplyMoveForce(GRAVITY * GetUp() * fThrust * m_fMass * CTimer::GetTimeStep());

		if (GetUp().z > 0.0f)
			ApplyTurnForce(-CVector(GetUp().x, GetUp().y, 0.0f) * fStabiliseVar * m_fTurnMass * CTimer::GetTimeStep(), GetUp());

		float fRoll, fPitch, fYaw;
		if (bCheat5) {
			fPitch = CPad::GetPad(0)->GetCarGunUpDown() / 128.0f;
			fRoll = -CPad::GetPad(0)->GetSteeringLeftRight() / 128.0f;
			fYaw = CPad::GetPad(0)->GetCarGunLeftRight() / 128.0f;
		}
		else {
			fPitch = CPad::GetPad(0)->GetSteeringUpDown() / 128.0f;
			fRoll = CPad::GetPad(0)->GetLookLeft();
			if (CPad::GetPad(0)->GetLookRight())
				fRoll = -1.0f;
			fYaw = CPad::GetPad(0)->GetSteeringLeftRight() / 128.0f;
		}
		if (CPad::GetPad(0)->GetHorn()) {
			fYaw = 0.0f;
			fPitch = clamp(10.0f * DotProduct(m_vecMoveSpeed, GetForward()), -200.0f, 1.3f);
			fRoll = clamp(10.0f * DotProduct(m_vecMoveSpeed, GetRight()), -200.0f, 1.3f);
		}
		ApplyTurnForce(fPitch * GetUp() * fPitchVar * m_fTurnMass * CTimer::GetTimeStep(), GetForward());
		ApplyTurnForce(fRoll * GetUp() * fRollVar * m_fTurnMass * CTimer::GetTimeStep(), GetRight());
		ApplyTurnForce(fYaw * GetForward() * fYawVar * m_fTurnMass * CTimer::GetTimeStep(), GetRight());

		CVector vecResistance;
		if (GetModelIndex() == MI_MIAMI_SPARROW)
			vecResistance = vecHeliResistance;
		else
			vecResistance = vecRCHeliResistance;
		float rX = Pow(vecResistance.x, CTimer::GetTimeStep());
		float rY = Pow(vecResistance.y, CTimer::GetTimeStep());
		float rZ = Pow(vecResistance.z, CTimer::GetTimeStep());
		CVector vecTurnSpeed = Multiply3x3(m_vecTurnSpeed, GetMatrix());
		float fResistanceMultiplier = Pow(1.0f / (fSpinSpeedRes * SQR(vecTurnSpeed.z) + 1.0f) * rZ, CTimer::GetTimeStep());
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

float fBurstSpeedMax = 0.3f;
float fBurstTyreMod = 0.1f;

void
CVehicle::ProcessWheel(CVector &wheelFwd, CVector &wheelRight, CVector &wheelContactSpeed, CVector &wheelContactPoint,
	int32 wheelsOnGround, float thrust, float brake, float adhesion, int8 wheelId, float *wheelSpeed, tWheelState *wheelState, uint16 wheelStatus)
{
	// BUG: using statics here is probably a bad idea
	static bool bAlreadySkidding = false;	// this is never reset
	static bool bBraking;
	static bool bDriving;

#ifdef FIX_SIGNIFICANT_BUGS
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
				if(GetModelIndex() == MI_RCBANDIT)
					brake = 0.2f * mod_HandlingManager.fWheelFriction / pHandling->fMass;
				else
					brake = mod_HandlingManager.fWheelFriction / pHandling->fMass;
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
		CVector direction = fwd*wheelFwd + right*wheelRight;
		float speed = direction.Magnitude();
		direction.Normalise();

		float impulse = speed*m_fMass;
		float turnImpulse = speed*GetMass(wheelContactPoint, direction);

		ApplyMoveForce(impulse * direction);
		ApplyTurnForce(turnImpulse * direction, wheelContactPoint);
	}
}

void
CVehicle::ProcessBikeWheel(CVector &wheelFwd, CVector &wheelRight, CVector &wheelContactSpeed, CVector &wheelContactPoint, int32 wheelsOnGround, float thrust,
                           float brake, float adhesion, int8 wheelId, float *wheelSpeed, tWheelState *wheelState, eBikeWheelSpecial special, uint16 wheelStatus)
{
	// TODO: mobile code
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

void
CVehicle::InflictDamage(CEntity* damagedBy, eWeaponType weaponType, float damage)
{
	if (!bCanBeDamaged)
		return;
	if (bOnlyDamagedByPlayer && (damagedBy != FindPlayerPed() && damagedBy != FindPlayerVehicle()))
		return;
	bool bFrightensDriver = false;
	switch (weaponType) {
	case WEAPONTYPE_UNARMED:
	case WEAPONTYPE_BASEBALLBAT:
		if (bMeleeProof)
			return;
		break;
	case WEAPONTYPE_COLT45:
	case WEAPONTYPE_UZI:
	case WEAPONTYPE_SHOTGUN:
	case WEAPONTYPE_AK47:
	case WEAPONTYPE_M16:
	case WEAPONTYPE_SNIPERRIFLE:
	case WEAPONTYPE_TOTAL_INVENTORY_WEAPONS:
	case WEAPONTYPE_UZI_DRIVEBY:
		if (bBulletProof)
			return;
		bFrightensDriver = true;
		break;
	case WEAPONTYPE_ROCKETLAUNCHER:
	case WEAPONTYPE_MOLOTOV:
	case WEAPONTYPE_GRENADE:
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
	if (m_fHealth > 0.0f) {
		if (VehicleCreatedBy == RANDOM_VEHICLE && pDriver &&
			(GetStatus() == STATUS_SIMPLE || GetStatus() == STATUS_PHYSICS) &&
			AutoPilot.m_nCarMission == MISSION_CRUISE) {
			if (m_randomSeed < DAMAGE_FLEE_IN_CAR_PROBABILITY_VALUE) {
				CCarCtrl::SwitchVehicleToRealPhysics(this);
				AutoPilot.m_nDrivingStyle = DRIVINGSTYLE_AVOID_CARS;
				AutoPilot.m_nCruiseSpeed = GAME_SPEED_TO_CARAI_SPEED * pHandling->Transmission.fMaxCruiseVelocity;
				SetStatus(STATUS_PHYSICS);
			}
		}
		m_nLastWeaponDamage = weaponType;
		float oldHealth = m_fHealth;
		if (m_fHealth > damage) {
			m_fHealth -= damage;
			if (VehicleCreatedBy == RANDOM_VEHICLE &&
				(m_fHealth < DAMAGE_HEALTH_TO_FLEE_ALWAYS ||
					bFrightensDriver && m_randomSeed > DAMAGE_FLEE_ON_FOOT_PROBABILITY_VALUE)) {
				switch (GetStatus()) {
				case STATUS_SIMPLE:
				case STATUS_PHYSICS:
					if (pDriver) {
						SetStatus(STATUS_ABANDONED);
						pDriver->bFleeAfterExitingCar = true;
						pDriver->SetObjective(OBJECTIVE_LEAVE_CAR, this);
					}
					for (int i = 0; i < m_nNumMaxPassengers; i++) {
						if (pPassengers[i]) {
							pPassengers[i]->bFleeAfterExitingCar = true;
							pPassengers[i]->SetObjective(OBJECTIVE_LEAVE_CAR, this);
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
	if (GetModelIndex() == MI_POLICE && damagedBy == FindPlayerPed() && damagedBy != nil && !bHasBeenOwnedByPlayer)
#else
	if (GetModelIndex() == MI_POLICE && damagedBy == FindPlayerPed())
#endif
		FindPlayerPed()->SetWantedLevelNoDrop(1);
}

void
CVehicle::DoFixedMachineGuns(void)
{
	if(CPad::GetPad(0)->GetCarGunFired() && !bGunSwitchedOff){
		if(CTimer::GetTimeInMilliseconds() > m_nGunFiringTime + 150){
			CVector source, target;
			float dx, dy, len;

			dx = GetForward().x;
			dy = GetForward().y;
			len = Sqrt(SQR(dx) + SQR(dy));
			if(len < 0.1f) len = 0.1f;
			dx /= len;
			dy /= len;

			m_nGunFiringTime = CTimer::GetTimeInMilliseconds();

			source = GetMatrix() * CVector(2.0f, 2.5f, 1.0f);
			target = source + CVector(dx, dy, 0.0f)*60.0f;
			target += CVector(
				((CGeneral::GetRandomNumber()&0xFF)-128) * 0.015f,
				((CGeneral::GetRandomNumber()&0xFF)-128) * 0.015f,
				((CGeneral::GetRandomNumber()&0xFF)-128) * 0.02f);
			CWeapon::DoTankDoomAiming(this, pDriver, &source, &target);
			FireOneInstantHitRound(&source, &target, 15);

			source = GetMatrix() * CVector(-2.0f, 2.5f, 1.0f);
			target = source + CVector(dx, dy, 0.0f)*60.0f;
			target += CVector(
				((CGeneral::GetRandomNumber()&0xFF)-128) * 0.015f,
				((CGeneral::GetRandomNumber()&0xFF)-128) * 0.015f,
				((CGeneral::GetRandomNumber()&0xFF)-128) * 0.02f);
			CWeapon::DoTankDoomAiming(this, pDriver, &source, &target);
			FireOneInstantHitRound(&source, &target, 15);

			DMAudio.PlayOneShot(m_audioEntityId, SOUND_WEAPON_SHOT_FIRED, 0.0f);

			m_nAmmoInClip--;
			if(m_nAmmoInClip == 0){
				m_nAmmoInClip = 20;
				m_nGunFiringTime = CTimer::GetTimeInMilliseconds() + 1400;
			}
		}
	}else{
		if(CTimer::GetTimeInMilliseconds() > m_nGunFiringTime + 1400)
			m_nAmmoInClip = 20;
	}
}

void
CVehicle::ExtinguishCarFire(void)
{
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
			pPassengers[2]->m_vehDoor = CAR_DOOR_RR;
			return true;
		}
		if (!pPassengers[0] && !(m_nGettingInFlags & CAR_DOOR_FLAG_RF)) {
			pPassengers[0] = pPassengers[1];
			pPassengers[1] = nil;
			pPassengers[0]->m_vehDoor = CAR_DOOR_RF;
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
			pPassengers[1]->m_vehDoor = CAR_DOOR_LR;
			return true;
		}
		if (!pPassengers[0] && !(m_nGettingInFlags & CAR_DOOR_FLAG_RF)) {
			pPassengers[0] = pPassengers[2];
			pPassengers[2] = nil;
			pPassengers[0]->m_vehDoor = CAR_DOOR_RF;
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
			pPassengers[1]->m_vehDoor = CAR_DOOR_LR;
			return true;
		}
		if (!pPassengers[2] && !(m_nGettingInFlags & CAR_DOOR_FLAG_RR)) {
			pPassengers[2] = pPassengers[0];
			pPassengers[0] = nil;
			pPassengers[2]->m_vehDoor = CAR_DOOR_RR;
			return true;
		}
		return false;
	}
	return false;
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

	if(FindPlayerVehicle() != this && m_pBlowUpEntity == FindPlayerPed())
		CWorld::Players[CWorld::PlayerInFocus].AwardMoneyForExplosion(this);
	BlowUpCar(m_pBlowUpEntity);
}

bool
CVehicle::IsLawEnforcementVehicle(void)
{
	switch(GetModelIndex()){
	case MI_FBICAR:
	case MI_POLICE:
	case MI_ENFORCER:
	case MI_PREDATOR:
	case MI_RHINO:
	case MI_BARRACKS:
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
	switch (GetModelIndex()){
	case MI_FIRETRUCK:
	case MI_AMBULAN:
	case MI_TAXI:
	case MI_POLICE:
	case MI_ENFORCER:
	case MI_BUS:
	case MI_RHINO:
	case MI_BARRACKS:
	case MI_DODO:
	case MI_COACH:
	case MI_CABBIE:
	case MI_RCBANDIT:
	case MI_BORGNINE:
		return false;
	default:
		return true;
	}
}

bool
CVehicle::CarHasRoof(void)
{
	if((pHandling->Flags & HANDLING_HAS_NO_ROOF) == 0)
		return true;
	if(m_aExtras[0] && m_aExtras[1])
		return false;
	return true;
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
	   m_nDoorLock == CARLOCK_LOCKED_PLAYER_INSIDE)
		return false;
	if(ped->IsPlayer() && m_nDoorLock == CARLOCK_LOCKOUT_PLAYER_ONLY)
		return false;
	return true;
}

bool
CVehicle::CanPedEnterCar(void)
{
	// can't enter when car is on side
	if(GetUp().z > 0.1f || GetUp().z < -0.1f){
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
CVehicle::CanPedExitCar(void)
{
	CVector up = GetUp();
	if(up.z > 0.1f || up.z < -0.1f){
#ifdef VC_PED_PORTS
		if (IsBoat())
			return true;
#endif
		// can't exit when car is moving too fast
		if(m_vecMoveSpeed.MagnitudeSqr() > 0.005f)
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

	pDriver = CPopulation::AddPedInCar(this);
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
	if(pPassengers[n])
		return pPassengers[n];

	pPassengers[n] = CPopulation::AddPedInCar(this);
	pPassengers[n]->m_pMyVehicle = this;
	pPassengers[n]->m_pMyVehicle->RegisterReference((CEntity**)&pPassengers[n]->m_pMyVehicle);
	pPassengers[n]->bInVehicle = true;
	pPassengers[n]->SetPedState(PED_DRIVING);
	if(bIsBus)
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
		if(GetModelIndex() == MI_AMBULAN)
			FindPlayerPed()->m_fHealth = Min(FindPlayerPed()->m_fHealth + 20.0f, 100.0f);
		else if(GetModelIndex() == MI_TAXI)
			CWorld::Players[CWorld::PlayerInFocus].m_nMoney += 25;
		else if(GetModelIndex() == MI_POLICE)
			driver->GiveWeapon(WEAPONTYPE_SHOTGUN, 5);
		else if(GetModelIndex() == MI_ENFORCER)
			driver->m_fArmour = Max(driver->m_fArmour, 100.0f);
		else if(GetModelIndex() == MI_CABBIE || GetModelIndex() == MI_BORGNINE)
			CWorld::Players[CWorld::PlayerInFocus].m_nMoney += 25;
		bFreebies = false;
	}

	ApplyTurnForce(0.0f, 0.0f, -0.2f*driver->m_fMass,
		driver->GetPosition().x - GetPosition().x,
		driver->GetPosition().y - GetPosition().y,
		0.0f);
}

bool
CVehicle::AddPassenger(CPed *passenger)
{
	int i;

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
#ifdef FIX_BUGS
	if (GetStatus() != STATUS_WRECKED)
#endif
		SetStatus(STATUS_ABANDONED);
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

void
CVehicle::ProcessCarAlarm(void)
{
	uint32 step;

	if(!IsAlarmOn())
		return;

	step = CTimer::GetTimeStepInMilliseconds();
	if((uint16)m_nAlarmState < step)
		m_nAlarmState = 0;
	else
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
