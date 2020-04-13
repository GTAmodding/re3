#include "common.h"
#include "main.h"
#include "patcher.h"
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

bool &CVehicle::bWheelsOnlyCheat = *(bool *)0x95CD78;
bool &CVehicle::bAllDodosCheat = *(bool *)0x95CD75;
bool &CVehicle::bCheat3 = *(bool *)0x95CD66;
bool &CVehicle::bCheat4 = *(bool *)0x95CD65;
bool &CVehicle::bCheat5 = *(bool *)0x95CD64;
bool &CVehicle::m_bDisableMouseSteering = *(bool *)0x60252C;

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

	m_nCurrentGear = 0;
	m_fChangeGearTime = 0;
	m_fSteerRatio = 0.0f;
	m_type = ENTITY_TYPE_VEHICLE;
	VehicleCreatedBy = CreatedBy;
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
	m_nNumMaxPassengers = 8;
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
	field_22D = 0;
	m_nAlarmState = 0;
	m_nDoorLock = CARLOCK_UNLOCKED;
	m_nLastWeaponDamage = -1;
	m_fMapObjectHeightAhead = m_fMapObjectHeightBehind = 0.0f;
	m_audioEntityId = DMAudio.CreateEntity(AUDIOTYPE_PHYSICAL, this);
	if(m_audioEntityId)
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
		if(moveSpeed > 2.25f)
			m_vecMoveSpeed *= 1.5f/Sqrt(moveSpeed);

		float turnSpeed = m_vecTurnSpeed.MagnitudeSqr();
		if(turnSpeed > 0.04f)
			m_vecTurnSpeed *= 0.2f/Sqrt(turnSpeed);
	}
		break;

	case FLIGHT_MODEL_RCPLANE:
	case FLIGHT_MODEL_SEAPLANE:
		assert(0 && "Plane flight model not implemented");
	case FLIGHT_MODEL_HELI:
		assert(0 && "Heli flight model not implemented");
	}
}

void
CVehicle::ProcessWheel(CVector &wheelFwd, CVector &wheelRight, CVector &wheelContactSpeed, CVector &wheelContactPoint,
	int32 wheelsOnGround, float thrust, float brake, float adhesion, int8 wheelId, float *wheelSpeed, tWheelState *wheelState, uint16 wheelStatus)
{
	// BUG: using statics here is probably a bad idea
	static bool bAlreadySkidding = false;	// this is never reset
	static bool bBraking;
	static bool bDriving;

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

		if(wheelStatus == WHEEL_STATUS_BURST){
			float fwdspeed = min(contactSpeedFwd, 0.3f);
			right += fwdspeed * CGeneral::GetRandomNumberInRange(-0.1f, 0.1f);
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

		if(!bBraking){
			if(m_fGasPedal < 0.01f){
				if(GetModelIndex() == MI_RCBANDIT)
					brake = 0.2f * mod_HandlingManager.field_4 / m_fMass;
				else
					brake = mod_HandlingManager.field_4 / m_fMass;
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

	if(sq(adhesion) < sq(right) + sq(fwd)){
		if(*wheelState != WHEEL_STATE_FIXED){
			if(bDriving && contactSpeedFwd < 0.2f)
				*wheelState = WHEEL_STATE_SPINNING;
			else
				*wheelState = WHEEL_STATE_SKIDDING;
		}

		float l = Sqrt(sq(right) + sq(fwd));
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
			(m_status == STATUS_SIMPLE || m_status == STATUS_PHYSICS) &&
			AutoPilot.m_nCarMission == MISSION_CRUISE) {
			if (m_randomSeed < DAMAGE_FLEE_IN_CAR_PROBABILITY_VALUE) {
				CCarCtrl::SwitchVehicleToRealPhysics(this);
				AutoPilot.m_nDrivingStyle = DRIVINGSTYLE_AVOID_CARS;
				AutoPilot.m_nCruiseSpeed = GAME_SPEED_TO_CARAI_SPEED * pHandling->Transmission.fUnkMaxVelocity;
				m_status = STATUS_PHYSICS;
			}
		}
		m_nLastWeaponDamage = weaponType;
		float oldHealth = m_fHealth;
		if (m_fHealth > damage) {
			m_fHealth -= damage;
			if (VehicleCreatedBy == RANDOM_VEHICLE &&
				(m_fHealth < DAMAGE_HEALTH_TO_FLEE_ALWAYS ||
					bFrightensDriver && m_randomSeed > DAMAGE_FLEE_ON_FOOT_PROBABILITY_VALUE)) {
				switch (m_status) {
				case STATUS_SIMPLE:
				case STATUS_PHYSICS:
					if (pDriver) {
						m_status = STATUS_ABANDONED;
						pDriver->bFleeAfterExitingCar = true;
						pDriver->SetObjective(OBJECTIVE_LEAVE_VEHICLE, this);
					}
					for (int i = 0; i < m_nNumMaxPassengers; i++) {
						if (pPassengers[i]) {
							pPassengers[i]->bFleeAfterExitingCar = true;
							pPassengers[i]->SetObjective(OBJECTIVE_LEAVE_VEHICLE, this);
						}
					}
					break;
				default:
					break;
				}
			}
			if (oldHealth > DAMAGE_HEALTH_TO_CATCH_FIRE && m_fHealth < DAMAGE_HEALTH_TO_CATCH_FIRE) {
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
	m_fHealth = max(m_fHealth, 300.0f);
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
		IsRoomForPedToLeaveCar(COMPONENT_DOOR_REAR_LEFT, nil)) {
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
		IsRoomForPedToLeaveCar(COMPONENT_DOOR_REAR_RIGHT, nil)) {
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
		IsRoomForPedToLeaveCar(COMPONENT_DOOR_FRONT_RIGHT, nil)) {
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
	if (!pDriver || m_nNumPassengers != 0 || m_status == STATUS_WRECKED)
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

	for(i = 0; i < 8; i++){
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
	CVector up = GetUp();
	// can't enter when car is on side
	if(up.z > 0.1f || up.z < -0.1f){
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
			FindPlayerPed()->m_fHealth = min(FindPlayerPed()->m_fHealth + 20.0f, 100.0f);
		else if(GetModelIndex() == MI_TAXI)
			CWorld::Players[CWorld::PlayerInFocus].m_nMoney += 25;
		else if(GetModelIndex() == MI_POLICE)
			driver->GiveWeapon(WEAPONTYPE_SHOTGUN, 5);
		else if(GetModelIndex() == MI_ENFORCER)
			driver->m_fArmour = max(driver->m_fArmour, 100.0f);
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
	m_status = STATUS_ABANDONED;
	pDriver = nil;
}

void
CVehicle::RemovePassenger(CPed *p)
{
	if (IsTrain()){
		for (int i = 0; i < 8; i++){
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

	if(m_nAlarmState == 0 || m_nAlarmState == -1)
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
#ifndef FIX_BUGS
		// this just isn't fair
		CDarkel::RegisterKillByPlayer(pVehicle->pDriver, WEAPONTYPE_UNIDENTIFIED);
#endif
		pVehicle->pDriver->FlagToDestroyWhenNextProcessed();
	}
	for (int i = 0; i < pVehicle->m_nNumMaxPassengers; i++) {
		if (pVehicle->pPassengers[i]) {
#ifndef FIX_BUGS
			// this just isn't fair
			CDarkel::RegisterKillByPlayer(pVehicle->pPassengers[i], WEAPONTYPE_UNIDENTIFIED);
#endif
			pVehicle->pPassengers[i]->FlagToDestroyWhenNextProcessed();
		}
	}
	CWorld::Remove(pVehicle);
	delete pVehicle;
}


class CVehicle_ : public CVehicle
{
public:
	void dtor(void) { CVehicle::~CVehicle(); }
	void SetModelIndex_(uint32 id) { CVehicle::SetModelIndex(id); }
	bool SetupLighting_(void) { return CVehicle::SetupLighting(); }
	void RemoveLighting_(bool reset) { CVehicle::RemoveLighting(reset); }
	float GetHeightAboveRoad_(void) { return CVehicle::GetHeightAboveRoad(); }
};

STARTPATCHES
	InjectHook(0x551170, &CVehicle_::SetModelIndex_, PATCH_JUMP);
	InjectHook(0x4A7DD0, &CVehicle_::SetupLighting_, PATCH_JUMP);
	InjectHook(0x4A7E60, &CVehicle_::RemoveLighting_, PATCH_JUMP);
	InjectHook(0x417E60, &CVehicle_::GetHeightAboveRoad_, PATCH_JUMP);

	InjectHook(0x552BB0, &CVehicle::FlyingControl, PATCH_JUMP);
	InjectHook(0x5512E0, &CVehicle::ProcessWheel, PATCH_JUMP);
	InjectHook(0x551280, &CVehicle::ProcessWheelRotation, PATCH_JUMP);
	InjectHook(0x552AF0, &CVehicle::ExtinguishCarFire, PATCH_JUMP);
	InjectHook(0x551C90, &CVehicle::ProcessDelayedExplosion, PATCH_JUMP);
	InjectHook(0x552880, &CVehicle::IsLawEnforcementVehicle, PATCH_JUMP);
	InjectHook(0x552820, &CVehicle::ChangeLawEnforcerState, PATCH_JUMP);
	InjectHook(0x552200, &CVehicle::UsesSiren, PATCH_JUMP);
	InjectHook(0x5527E0, &CVehicle::IsVehicleNormal, PATCH_JUMP);
	InjectHook(0x552B70, &CVehicle::CarHasRoof, PATCH_JUMP);
	InjectHook(0x552230, &CVehicle::IsUpsideDown, PATCH_JUMP);
	InjectHook(0x552260, &CVehicle::IsOnItsSide, PATCH_JUMP);
	InjectHook(0x5511B0, &CVehicle::CanBeDeleted, PATCH_JUMP);
	InjectHook(0x5522A0, &CVehicle::CanPedOpenLocks, PATCH_JUMP);
	InjectHook(0x5522F0, &CVehicle::CanPedEnterCar, PATCH_JUMP);
	InjectHook(0x5523C0, &CVehicle::CanPedExitCar, PATCH_JUMP);
	InjectHook(0x5520C0, &CVehicle::SetUpDriver, PATCH_JUMP);
	InjectHook(0x552160, &CVehicle::SetupPassenger, PATCH_JUMP);
	InjectHook(0x551F20, &CVehicle::SetDriver, PATCH_JUMP);
	InjectHook(0x551D90, (bool (CVehicle::*)(CPed*))&CVehicle::AddPassenger, PATCH_JUMP);
	InjectHook(0x551E10, (bool (CVehicle::*)(CPed*,uint8))&CVehicle::AddPassenger, PATCH_JUMP);
	InjectHook(0x5520A0, &CVehicle::RemoveDriver, PATCH_JUMP);
	InjectHook(0x551EB0, &CVehicle::RemovePassenger, PATCH_JUMP);
	InjectHook(0x5525A0, &CVehicle::ProcessCarAlarm, PATCH_JUMP);
	InjectHook(0x552620, &CVehicle::IsSphereTouchingVehicle, PATCH_JUMP);
	InjectHook(0x551950, &CVehicle::InflictDamage, PATCH_JUMP);
ENDPATCHES
