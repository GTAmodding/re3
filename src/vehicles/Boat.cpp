#include "common.h"

#include "main.h"
#include "General.h"
#include "Timecycle.h"
#include "HandlingMgr.h"
#include "CarCtrl.h"
#include "RwHelper.h"
#include "ModelIndices.h"
#include "VisibilityPlugins.h"
#include "DMAudio.h"
#include "Camera.h"
#include "Darkel.h"
#include "Explosion.h"
#include "Particle.h"
#include "WaterLevel.h"
#include "Floater.h"
#include "World.h"
#include "Pools.h"
#include "Pad.h"
#include "Boat.h"

#define INVALID_ORIENTATION (-9999.99f)

float MAX_WAKE_LENGTH = 50.0f;
float MIN_WAKE_INTERVAL = 1.0f;
float WAKE_LIFETIME = 400.0f;

float fShapeLength = 0.4f;
float fShapeTime = 0.05f;
float fRangeMult = 0.75f;
float fTimeMult = 1.0f/WAKE_LIFETIME;

CBoat *CBoat::apFrameWakeGeneratingBoats[4];

const uint32 CBoat::nSaveStructSize =
#ifdef COMPATIBLE_SAVES
	1156;
#else
	sizeof(CBoat);
#endif

CBoat::CBoat(int mi, uint8 owner) : CVehicle(owner)
{
	CVehicleModelInfo *minfo = (CVehicleModelInfo*)CModelInfo::GetModelInfo(mi);
	m_vehType = VEHICLE_TYPE_BOAT;
	m_fAccelerate = 0.0f;
	m_fBrake = 0.0f;
	m_fSteeringLeftRight = 0.0f;
	m_nPadID = 0;
	m_fMovingRotation = 0.0f;
	SetModelIndex(mi);

	pHandling = mod_HandlingManager.GetHandlingData((tVehicleType)minfo->m_handlingId);
	minfo->ChooseVehicleColour(m_currentColour1, m_currentColour2);

	m_fMass = pHandling->fMass;
	m_fTurnMass = pHandling->fTurnMass / 2.0f;
	m_vecCentreOfMass = pHandling->CentreOfMass;
	m_fAirResistance = pHandling->Dimension.x * pHandling->Dimension.z / m_fMass;
	m_fElasticity = 0.1f;
	m_fBuoyancy = pHandling->fBuoyancy;
	m_fSteerAngle = 0.0f;
	m_fGasPedal = 0.0f;
	m_fBrakePedal = 0.0f;

	m_fThrustZ = 0.25f;
	m_fThrustY = 0.35f;
	m_vecMoveRes = CVector(0.7f, 0.998f, 0.999f);
	m_vecTurnRes = CVector(0.85f, 0.96f, 0.96f);
	m_boat_unused3 = false;

	m_fVolumeUnderWater = 7.0f;
	m_fPrevVolumeUnderWater = 7.0f;
	m_vecBuoyancePoint = CVector(0.0f, 0.0f, 0.0f);

	m_nDeltaVolumeUnderWater = 0;
	bBoatInWater = true;
	bPropellerInWater = true;

	bIsInWater = true;

	m_boat_unused2 = 0;
	m_bIsAnchored = true;
	m_fOrientation = INVALID_ORIENTATION;
	bTouchingWater = true;
	m_fDamage = 0.0f;
	m_pSetOnFireEntity = nil;
	m_nNumWakePoints = 0;

	for (int16 i = 0; i < ARRAY_SIZE(m_afWakePointLifeTime); i++)
		m_afWakePointLifeTime[i] = 0.0f;

	m_nAmmoInClip = 20;
}

void
CBoat::SetModelIndex(uint32 id)
{
	CEntity::SetModelIndex(id);
	SetupModelNodes();
}

void
CBoat::GetComponentWorldPosition(int32 component, CVector &pos)
{
	pos = *RwMatrixGetPos(RwFrameGetLTM(m_aBoatNodes[component]));
}

void
CBoat::ProcessControl(void)
{
	if(m_nZoneLevel > LEVEL_GENERIC && m_nZoneLevel != CCollision::ms_collisionInMemory)
		return;

	bool onLand = m_fDamageImpulse > 0.0f && m_vecDamageNormal.z > 0.1f;

	PruneWakeTrail();

	int r, g, b;
	RwRGBA splashColor, jetColor;
	r = 114.75f*(CTimeCycle::GetAmbientRed() + 0.5f*CTimeCycle::GetDirectionalRed());
	g = 114.75f*(CTimeCycle::GetAmbientGreen() + 0.5f*CTimeCycle::GetDirectionalGreen());
	b = 114.75f*(CTimeCycle::GetAmbientBlue() + 0.5f*CTimeCycle::GetDirectionalBlue());
	r = clamp(r, 0, 255);
	g = clamp(g, 0, 255);
	b = clamp(b, 0, 255);
	splashColor.red = r;
	splashColor.green = g;
	splashColor.blue = b;
	splashColor.alpha = CGeneral::GetRandomNumberInRange(128, 150);

	r = 242.25f*(CTimeCycle::GetAmbientRed() + 0.5f*CTimeCycle::GetDirectionalRed());
	g = 242.25f*(CTimeCycle::GetAmbientGreen() + 0.5f*CTimeCycle::GetDirectionalGreen());
	b = 242.25f*(CTimeCycle::GetAmbientBlue() + 0.5f*CTimeCycle::GetDirectionalBlue());
	r = clamp(r, 0, 255);
	g = clamp(g, 0, 255);
	b = clamp(b, 0, 255);
	jetColor.red = r;
	jetColor.green = g;
	jetColor.blue = b;
	jetColor.alpha = CGeneral::GetRandomNumberInRange(96, 128);

	CGeneral::GetRandomNumber();	// unused

	ProcessCarAlarm();

	switch(GetStatus()){
	case STATUS_PLAYER:
		m_bIsAnchored = false;
		m_fOrientation = INVALID_ORIENTATION;
		ProcessControlInputs(0);
		if(GetModelIndex() == MI_PREDATOR)
			DoFixedMachineGuns();
		break;
	case STATUS_SIMPLE:
		m_bIsAnchored = false;
		m_fOrientation = INVALID_ORIENTATION;
		CPhysical::ProcessControl();
		bBoatInWater = true;
		bPropellerInWater = true;
		bIsInWater = true;
		return;
	case STATUS_PHYSICS:
		m_bIsAnchored = false;
		m_fOrientation = INVALID_ORIENTATION;
		CCarCtrl::SteerAIBoatWithPhysics(this);
		break;
	case STATUS_ABANDONED:
	case STATUS_WRECKED:
		bBoatInWater = true;
		bPropellerInWater = true;
		bIsInWater = true;
		m_fSteerAngle = 0.0;
		bIsHandbrakeOn = false;
		m_fBrakePedal = 0.5f;
		m_fGasPedal = 0.0f;
		if((GetPosition() - CWorld::Players[CWorld::PlayerInFocus].GetPos()).Magnitude() > 150.0f){
			m_vecMoveSpeed = CVector(0.0f, 0.0f, 0.0f);
			m_vecTurnSpeed = CVector(0.0f, 0.0f, 0.0f);
			return;
		}
		break;
	default: break;
	}

	float collisionDamage = pHandling->fCollisionDamageMultiplier * m_fDamageImpulse;
#ifdef FIX_BUGS
	if (collisionDamage > 25.0f && GetStatus() != STATUS_WRECKED && m_fHealth >= 150.0f && !bCollisionProof) {
#else
	if(collisionDamage > 25.0f && GetStatus() != STATUS_WRECKED && m_fHealth >= 150.0f){
#endif
		float prevHealth = m_fHealth;
		if(this == FindPlayerVehicle()){
			if(bTakeLessDamage)
				m_fHealth -= (collisionDamage-25.0f)/6.0f;
			else
				m_fHealth -= (collisionDamage-25.0f)/2.0f;
		}else{
			if(collisionDamage > 60.0f && pDriver)
				pDriver->Say(SOUND_PED_ANNOYED_DRIVER);
			if(bTakeLessDamage)
				m_fHealth -= (collisionDamage-25.0f)/12.0f;
			else
				m_fHealth -= (collisionDamage-25.0f)/4.0f;
		}

		if(m_fHealth <= 0.0f && prevHealth > 0.0f){
			m_fHealth = 1.0f;
			m_pSetOnFireEntity = m_pDamageEntity;
		}
	}

	// Damage particles
	if(m_fHealth <= 600.0f && GetStatus() != STATUS_WRECKED &&
	   Abs(GetPosition().x - TheCamera.GetPosition().x) < 200.0f &&
	   Abs(GetPosition().y - TheCamera.GetPosition().y) < 200.0f){
		float speedSq = m_vecMoveSpeed.MagnitudeSqr();
		CVector smokeDir = 0.8f*m_vecMoveSpeed;
		CVector smokePos;
		switch(GetModelIndex()){
		case MI_SPEEDER:
			smokePos = CVector(0.4f, -2.4f, 0.8f);
			smokeDir += 0.05f*GetRight();
			smokeDir.z += 0.2f*m_vecMoveSpeed.z;
			break;
		case MI_REEFER:
			smokePos = CVector(2.0f, -1.0f, 0.5f);
			smokeDir += 0.07f*GetRight();
			break;
		case MI_PREDATOR:
		default:
			smokePos = CVector(-1.5f, -0.5f, 1.2f);
			smokeDir += -0.08f*GetRight();
			break;
		}

		smokePos = GetMatrix() * smokePos;

		// On fire
		if(m_fHealth < 150.0f){
			CParticle::AddParticle(PARTICLE_CARFLAME, smokePos,
				CVector(0.0f, 0.0f, CGeneral::GetRandomNumberInRange(2.25f/200.0f, 0.09f)),
				nil, 0.9f);
			CVector smokePos2 = smokePos;
			smokePos2.x += CGeneral::GetRandomNumberInRange(-2.25f/4.0f, 2.25f/4.0f);
			smokePos2.y += CGeneral::GetRandomNumberInRange(-2.25f/4.0f, 2.25f/4.0f);
			smokePos2.z += CGeneral::GetRandomNumberInRange(2.25f/4.0f, 2.25f);
			CParticle::AddParticle(PARTICLE_ENGINE_SMOKE2, smokePos2, CVector(0.0f, 0.0f, 0.0f));

			m_fDamage += CTimer::GetTimeStepInMilliseconds();
			if(m_fDamage > 5000.0f)
				BlowUpCar(m_pSetOnFireEntity);
		}

		if(speedSq < 0.25f && (CTimer::GetFrameCounter() + m_randomSeed) & 1)
			CParticle::AddParticle(PARTICLE_ENGINE_STEAM, smokePos, smokeDir);
		if(speedSq < 0.25f && m_fHealth <= 350.0f)
			CParticle::AddParticle(PARTICLE_ENGINE_SMOKE, smokePos, 1.25f*smokeDir);
	}

	CPhysical::ProcessControl();

	CVector buoyanceImpulse(0.0f, 0.0f, 0.0f);
	CVector buoyancePoint(0.0f, 0.0f, 0.0f);
	if(mod_Buoyancy.ProcessBuoyancy(this, pHandling->fBuoyancy, &buoyancePoint, &buoyanceImpulse)){
		// Process boat in water
		if(0.1f * m_fMass * GRAVITY*CTimer::GetTimeStep() < buoyanceImpulse.z){
			bBoatInWater = true;
			bIsInWater = true;
		}else{
			bBoatInWater = false;
			bIsInWater = false;
		}

		m_fVolumeUnderWater = mod_Buoyancy.m_volumeUnderWater;
		m_vecBuoyancePoint = buoyancePoint;
		ApplyMoveForce(buoyanceImpulse);
		if(!onLand)
			ApplyTurnForce(buoyanceImpulse, buoyancePoint);

		if(!onLand && bBoatInWater && GetUp().z > 0.0f){
			float impulse;
			if(m_fGasPedal > 0.05f)
				impulse = m_vecMoveSpeed.MagnitudeSqr()*pHandling->fSuspensionForceLevel*buoyanceImpulse.z*CTimer::GetTimeStep()*0.5f*m_fGasPedal;
			else
				impulse = 0.0f;
			impulse = Min(impulse, GRAVITY*pHandling->fSuspensionDampingLevel*m_fMass*CTimer::GetTimeStep());
			ApplyMoveForce(impulse*GetUp());
			ApplyTurnForce(impulse*GetUp(), buoyancePoint - pHandling->fSuspensionBias*GetForward());
		}

		// Handle boat moving forward
		if(Abs(m_fGasPedal) > 0.05f || m_vecMoveSpeed.Magnitude2D() > 0.01f){
			if(bBoatInWater)
				AddWakePoint(GetPosition());

			float steerFactor = 1.0f - DotProduct(m_vecMoveSpeed, GetForward());
			if (GetModelIndex() == MI_GHOST)
				steerFactor = 1.0f - DotProduct(m_vecMoveSpeed, GetForward())*0.3f;
			if(steerFactor < 0.0f) steerFactor = 0.0f;

			CVector propeller(0.0f, -pHandling->Dimension.y*m_fThrustY, -pHandling->Dimension.z*m_fThrustZ);
			propeller = Multiply3x3(GetMatrix(), propeller);
			CVector propellerWorld = GetPosition() + propeller;

			float steerSin = Sin(-m_fSteerAngle * steerFactor);
			float steerCos = Cos(-m_fSteerAngle * steerFactor);
			float waterLevel;
			CWaterLevel::GetWaterLevel(propellerWorld, &waterLevel, true);
			if(propellerWorld.z-0.5f < waterLevel){
				float propellerDepth = waterLevel - (propellerWorld.z - 0.5f);
				if(propellerDepth > 1.0f)
					propellerDepth = 1.0f;
				else
					propellerDepth = SQR(propellerDepth);
				bPropellerInWater = true;

				if(Abs(m_fGasPedal) > 0.05f){
					CVector forceDir = Multiply3x3(GetMatrix(), CVector(-steerSin, steerCos, -Abs(m_fSteerAngle)));
					CVector force = propellerDepth * m_fGasPedal * 40.0f * pHandling->Transmission.fEngineAcceleration * pHandling->fMass * forceDir;
					if(force.z > 0.2f)
						force.z = SQR(1.2f - force.z) + 0.2f;
					if(onLand){
						if(m_fGasPedal < 0.0f){
							force.x *= 5.0f;
							force.y *= 5.0f;
						}
						if(force.z < 0.0f)
							force.z = 0.0f;
						ApplyMoveForce(force * CTimer::GetTimeStep());
					}else{
						ApplyMoveForce(force * CTimer::GetTimeStep());
						ApplyTurnForce(force * CTimer::GetTimeStep(), propeller - pHandling->fTractionBias*GetUp());
						float rightForce = DotProduct(GetRight(), force);
						ApplyTurnForce(-rightForce*GetRight() * CTimer::GetTimeStep(), GetUp());
					}

					// Spray some particles
					CVector jetDir = -0.04f * force;
					if(m_fGasPedal > 0.0f){
						if(GetStatus() == STATUS_PLAYER){
							bool cameraHack = TheCamera.Cams[TheCamera.ActiveCam].Mode == CCam::MODE_TOPDOWN ||
								TheCamera.WhoIsInControlOfTheCamera == CAMCONTROL_OBBE;
							CVector sternPos = GetColModel()->boundingBox.min;
							sternPos.x = 0.0f;
							sternPos.z = 0.0f;
							sternPos = Multiply3x3(GetMatrix(), sternPos);

							CVector jetPos = GetPosition() + sternPos;
							if(cameraHack)
								jetPos.z = 1.0f;
							else
								jetPos.z = 0.0f;

#ifdef PC_PARTICLE
							CVector wakePos = GetPosition() + sternPos;
							wakePos.z -= 0.65f;
#else
							CVector wakePos = GetPosition() + sternPos;
							wakePos.z = -0.3f;
#endif

							CVector wakeDir = 0.75f * jetDir;

							CParticle::AddParticle(PARTICLE_BOAT_THRUSTJET, jetPos, jetDir, nil, 0.0f, jetColor);
#ifdef PC_PARTICLE
							CParticle::AddParticle(PARTICLE_CAR_SPLASH, jetPos, 0.25f * jetDir, nil, 1.0f, splashColor,
								CGeneral::GetRandomNumberInRange(0, 30),
								CGeneral::GetRandomNumberInRange(0, 90), 3);
#endif
							if(!cameraHack)
								CParticle::AddParticle(PARTICLE_BOAT_WAKE, wakePos, wakeDir, nil, 0.0f, jetColor);
						}else if((CTimer::GetFrameCounter() + m_randomSeed) & 1){
#ifdef PC_PARTICLE
							jetDir.z = 0.018f;
							jetDir.x *= 0.01f;
							jetDir.y *= 0.01f;
							propellerWorld.z += 1.5f;
							
							CParticle::AddParticle(PARTICLE_BOAT_SPLASH, propellerWorld, jetDir, nil, 1.5f, jetColor);
#else
							jetDir.z = 0.018f;
							jetDir.x *= 0.03f;
							jetDir.y *= 0.03f;
							propellerWorld.z += 1.0f;
							
							CParticle::AddParticle(PARTICLE_BOAT_SPLASH, propellerWorld, jetDir, nil, 0.0f, jetColor);
#endif
							
#ifdef PC_PARTICLE
							CParticle::AddParticle(PARTICLE_CAR_SPLASH, propellerWorld, 0.1f * jetDir, nil, 0.5f, splashColor,
								CGeneral::GetRandomNumberInRange(0, 30),
								CGeneral::GetRandomNumberInRange(0, 90), 3);
#endif
						}
					}
				}else if(!onLand){
					float force = 50.0f*DotProduct(m_vecMoveSpeed, GetForward());
					force = Min(force, 10.0f);
					CVector propellerForce = propellerDepth * Multiply3x3(GetMatrix(), force*CVector(-steerSin, 0.0f, 0.0f));
					ApplyMoveForce(propellerForce * CTimer::GetTimeStep()*0.5f);
					ApplyTurnForce(propellerForce * CTimer::GetTimeStep()*0.5f, propeller);
				}
			}else
				bPropellerInWater = false;
		}

		// Slow down or push down boat as it approaches the world limits
		m_vecMoveSpeed.x = Min(m_vecMoveSpeed.x, -(GetPosition().x - 1900.0f)*0.01f);	// east
		m_vecMoveSpeed.x = Max(m_vecMoveSpeed.x, -(GetPosition().x - -1515.0f)*0.01f);	// west
		m_vecMoveSpeed.y = Min(m_vecMoveSpeed.y, -(GetPosition().y - 600.0f)*0.01f);	// north
		m_vecMoveSpeed.y = Max(m_vecMoveSpeed.y, -(GetPosition().y - -1900.0f)*0.01f);	// south

		if(!onLand && bBoatInWater)
			ApplyWaterResistance();

		// No idea what exactly is going on here besides drag in YZ
		float fx = Pow(m_vecTurnRes.x, CTimer::GetTimeStep());
		float fy = Pow(m_vecTurnRes.y, CTimer::GetTimeStep());
		float fz = Pow(m_vecTurnRes.z, CTimer::GetTimeStep());
		m_vecTurnSpeed = Multiply3x3(m_vecTurnSpeed, GetMatrix());	// invert - to local space
		// TODO: figure this out
		float magic = 1.0f/(1000.0f * SQR(m_vecTurnSpeed.x) + 1.0f) * fx;
		m_vecTurnSpeed.y *= fy;
		m_vecTurnSpeed.z *= fz;
		float forceUp = (magic - 1.0f) * m_vecTurnSpeed.x * m_fTurnMass;
		m_vecTurnSpeed = Multiply3x3(GetMatrix(), m_vecTurnSpeed);	// back to world
		CVector com = Multiply3x3(GetMatrix(), m_vecCentreOfMass);
		ApplyTurnForce(CVector(0.0f, 0.0f, forceUp), com + GetForward());

		m_nDeltaVolumeUnderWater = (m_fVolumeUnderWater-m_fPrevVolumeUnderWater)*10000;

		// Falling into water
		if(!onLand && bBoatInWater && GetUp().z > 0.0f && m_nDeltaVolumeUnderWater > 200){
			DMAudio.PlayOneShot(m_audioEntityId, SOUND_CAR_SPLASH, m_nDeltaVolumeUnderWater);

			float speedUp = m_vecMoveSpeed.MagnitudeSqr() * m_nDeltaVolumeUnderWater * 0.0004f;
			if(speedUp + m_vecMoveSpeed.z > pHandling->fBrakeDeceleration)
				speedUp = pHandling->fBrakeDeceleration - m_vecMoveSpeed.z;
			if(speedUp < 0.0f) speedUp = 0.0f;
			float speedFwd = DotProduct(m_vecMoveSpeed, GetForward());
			speedFwd *= -m_nDeltaVolumeUnderWater * 0.01f * pHandling->fTractionLoss;
			CVector speed = speedFwd*GetForward() + CVector(0.0f, 0.0f, speedUp);
			CVector splashImpulse = speed * m_fMass;
			ApplyMoveForce(splashImpulse);
			ApplyTurnForce(splashImpulse, buoyancePoint);
		}

		// Spray particles on sides of boat
#ifdef PC_PARTICLE
		if(m_nDeltaVolumeUnderWater > 75)
#else
		if(m_nDeltaVolumeUnderWater > 120)
#endif
		{
			float speed = m_vecMoveSpeed.Magnitude();
			float splash1Size = speed;
			float splash2Size = float(m_nDeltaVolumeUnderWater) * 0.005f * 0.2f;
			float front = 0.9f * GetColModel()->boundingBox.max.y;
			if(splash1Size > 0.75f) splash1Size = 0.75f;

			CVector dir, pos;

			// right
#ifdef PC_PARTICLE
			dir = -0.5f*m_vecMoveSpeed;
			dir.z += 0.1f*speed;
			dir += 0.5f*GetRight()*speed;
			pos = front*GetForward() + 0.5f*GetRight() + GetPosition() + m_vecBuoyancePoint;
			CWaterLevel::GetWaterLevel(pos, &pos.z, true);
#else
			dir = 0.3f*m_vecMoveSpeed;
			dir.z += 0.05f*speed;
			dir += 0.5f*GetRight()*speed;
			pos = (GetPosition() + m_vecBuoyancePoint) + (1.5f*GetRight());
#endif
			
#ifdef PC_PARTICLE
			CParticle::AddParticle(PARTICLE_CAR_SPLASH, pos, 0.75f * dir, nil, splash1Size, splashColor,
				CGeneral::GetRandomNumberInRange(0, 30),
				CGeneral::GetRandomNumberInRange(0, 90), 1);
			CParticle::AddParticle(PARTICLE_BOAT_SPLASH, pos, dir, nil, splash2Size, jetColor);
#else
			CParticle::AddParticle(PARTICLE_BOAT_SPLASH, pos, dir, nil, splash2Size);
#endif
			

			// left
#ifdef PC_PARTICLE
			dir = -0.5f*m_vecMoveSpeed;
			dir.z += 0.1f*speed;
			dir -= 0.5f*GetRight()*speed;
			pos = front*GetForward() - 0.5f*GetRight() + GetPosition() + m_vecBuoyancePoint;
			CWaterLevel::GetWaterLevel(pos, &pos.z, true);
#else
			dir = 0.3f*m_vecMoveSpeed;
			dir.z += 0.05f*speed;
			dir -= 0.5f*GetRight()*speed;
			pos = (GetPosition() + m_vecBuoyancePoint) - (1.5f*GetRight());
#endif
			
#ifdef PC_PARTICLE
			CParticle::AddParticle(PARTICLE_CAR_SPLASH, pos, 0.75f * dir, nil, splash1Size, splashColor,
				CGeneral::GetRandomNumberInRange(0, 30),
				CGeneral::GetRandomNumberInRange(0, 90), 1);
			CParticle::AddParticle(PARTICLE_BOAT_SPLASH, pos, dir, nil, splash2Size, jetColor);
#else
			CParticle::AddParticle(PARTICLE_BOAT_SPLASH, pos, dir, nil, splash2Size);
#endif
		}

		m_fPrevVolumeUnderWater = m_fVolumeUnderWater;
	}else{
		bBoatInWater = false;
		bIsInWater = false;
	}

	if(m_bIsAnchored){
		m_vecMoveSpeed.x = 0.0f;
		m_vecMoveSpeed.y = 0.0f;

		if(m_fOrientation == INVALID_ORIENTATION){
			m_fOrientation = GetForward().Heading();
		}else{
			// is this some inlined CPlaceable method?
			CVector pos = GetPosition();
			GetMatrix().RotateZ(m_fOrientation - GetForward().Heading());
			GetMatrix().SetTranslateOnly(pos);
		}
	}

	ProcessDelayedExplosion();
}

void
CBoat::ProcessControlInputs(uint8 pad)
{
	m_nPadID = pad;
	if(m_nPadID > 3)
		m_nPadID = 3;

	m_fBrake += (CPad::GetPad(pad)->GetBrake()/255.0f - m_fBrake)*0.1f;
	m_fBrake = clamp(m_fBrake, 0.0f, 1.0f);

	if(m_fBrake < 0.05f){
		m_fBrake = 0.0f;
		m_fAccelerate += (CPad::GetPad(pad)->GetAccelerate()/255.0f - m_fAccelerate)*0.1f;
		m_fAccelerate = clamp(m_fAccelerate, 0.0f, 1.0f);
	}else
		m_fAccelerate = -m_fBrake*0.2f;

	m_fSteeringLeftRight += (-CPad::GetPad(pad)->GetSteeringLeftRight()/128.0f - m_fSteeringLeftRight)*0.2f;
	m_fSteeringLeftRight = clamp(m_fSteeringLeftRight, -1.0f, 1.0f);

	float steeringSq = m_fSteeringLeftRight < 0.0f ? -SQR(m_fSteeringLeftRight) : SQR(m_fSteeringLeftRight);
	m_fSteerAngle = pHandling->fSteeringLock * DEGTORAD(steeringSq);
	m_fGasPedal = m_fAccelerate;
}

void
CBoat::ApplyWaterResistance(void)
{
	float fwdSpeed = DotProduct(GetMoveSpeed(), GetForward());
	// TODO: figure out how this works
	float resistance = 0.001f * SQR(m_fVolumeUnderWater) * m_fMass;
	float magic = (SQR(fwdSpeed) + 0.05f) * resistance + 1.0f;
	magic = Abs(magic);
	float fx = Pow(m_vecMoveRes.x/magic, 0.5f*CTimer::GetTimeStep());
	float fy = Pow(m_vecMoveRes.y/magic, 0.5f*CTimer::GetTimeStep());
	float fz = Pow(m_vecMoveRes.z/magic, 0.5f*CTimer::GetTimeStep());

	m_vecMoveSpeed = Multiply3x3(m_vecMoveSpeed, GetMatrix());	// invert - to local space
	m_vecMoveSpeed.x *= fx;
	m_vecMoveSpeed.y *= fy;
	m_vecMoveSpeed.z *= fz;
	float force = (fy - 1.0f) * m_vecMoveSpeed.y * m_fMass;
	m_vecMoveSpeed = Multiply3x3(GetMatrix(), m_vecMoveSpeed);	// back to world

	ApplyTurnForce(force*GetForward(), -GetUp());

	if(m_vecMoveSpeed.z > 0.0f)
		m_vecMoveSpeed.z *= fz;
	else
		m_vecMoveSpeed.z *= (1.0f - fz)*0.5f + fz;
}

RwObject*
GetBoatAtomicObjectCB(RwObject *object, void *data)
{
	RpAtomic *atomic = (RpAtomic*)object;
	assert(RwObjectGetType(object) == rpATOMIC);
	if(RpAtomicGetFlags(atomic) & rpATOMICRENDER)
		*(RpAtomic**)data = atomic;
	return object;

	
}

void
CBoat::BlowUpCar(CEntity *culprit)
{
	RpAtomic *atomic;
	RwFrame *frame;
	RwMatrix *matrix;
	CObject *obj;

	if(!bCanBeDamaged)
		return;

	// explosion pushes vehicle up
	m_vecMoveSpeed.z += 0.13f;
	SetStatus(STATUS_WRECKED);
	bRenderScorched = true;

	m_fHealth = 0.0;
	m_nBombTimer = 0;
	TheCamera.CamShake(0.7f, GetPosition().x, GetPosition().y, GetPosition().z);

	if(this == FindPlayerVehicle())
		FindPlayerPed()->m_fHealth = 0.0f;	// kill player
	if(pDriver){
		CDarkel::RegisterKillByPlayer(pDriver, WEAPONTYPE_EXPLOSION);
		pDriver->SetDead();
		pDriver->FlagToDestroyWhenNextProcessed();
	}

	bEngineOn = false;
	bLightsOn = false;
	ChangeLawEnforcerState(false);

	CExplosion::AddExplosion(this, culprit, EXPLOSION_CAR, GetPosition(), 0);
	if(m_aBoatNodes[BOAT_MOVING] == nil)
		return;

	// much like CAutomobile::SpawnFlyingComponent from here on

	atomic = nil;
	RwFrameForAllObjects(m_aBoatNodes[BOAT_MOVING], GetBoatAtomicObjectCB, &atomic);
	if(atomic == nil)
		return;

	obj = new CObject();
	if(obj == nil)
		return;

	obj->SetModelIndexNoCreate(MI_CAR_WHEEL);

	// object needs base model
	obj->RefModelInfo(GetModelIndex());

	// create new atomic
	matrix = RwFrameGetLTM(m_aBoatNodes[BOAT_MOVING]);
	frame = RwFrameCreate();
	atomic = RpAtomicClone(atomic);
	*RwFrameGetMatrix(frame) = *matrix;
	RpAtomicSetFrame(atomic, frame);
	CVisibilityPlugins::SetAtomicRenderCallback(atomic, nil);
	obj->AttachToRwObject((RwObject*)atomic);

	// init object
	obj->m_fMass = 10.0f;
	obj->m_fTurnMass = 25.0f;
	obj->m_fAirResistance = 0.99f;
	obj->m_fElasticity = 0.1f;
	obj->m_fBuoyancy = obj->m_fMass*GRAVITY/0.75f;
	obj->ObjectCreatedBy = TEMP_OBJECT;
	obj->SetIsStatic(false);
	obj->bIsPickup = false;

	// life time
	CObject::nNoTempObjects++;
	obj->m_nEndOfLifeTime = CTimer::GetTimeInMilliseconds() + 20000;

	obj->m_vecMoveSpeed = m_vecMoveSpeed;
	if(GetUp().z > 0.0f)
		obj->m_vecMoveSpeed.z = 0.3f;
	else
		obj->m_vecMoveSpeed.z = 0.0f;

	obj->m_vecTurnSpeed = m_vecTurnSpeed*2.0f;
	obj->m_vecTurnSpeed.x = 0.5f;

	// push component away from boat
	CVector dist = obj->GetPosition() - GetPosition();
	dist.Normalise();
	if(GetUp().z > 0.0f)
		dist += GetUp();
	obj->GetMatrix().GetPosition() += dist;

	CWorld::Add(obj);

	atomic = nil;
	RwFrameForAllObjects(m_aBoatNodes[BOAT_MOVING], GetBoatAtomicObjectCB, &atomic);
	if(atomic)
		RpAtomicSetFlags(atomic, 0);
}

RwIm3DVertex KeepWaterOutVertices[4];
RwImVertexIndex KeepWaterOutIndices[6];

void
CBoat::Render()
{
	CMatrix matrix;

	if (m_aBoatNodes[BOAT_MOVING] != nil) {
		matrix.Attach(RwFrameGetMatrix(m_aBoatNodes[BOAT_MOVING]));

		CVector pos = matrix.GetPosition();
		matrix.SetRotateZ(m_fMovingRotation);
		matrix.Translate(pos);

		matrix.UpdateRW();
		if (CVehicle::bWheelsOnlyCheat) {
			RpAtomicRender((RpAtomic*)GetFirstObject(m_aBoatNodes[BOAT_MOVING]));
		}
	}
	m_fMovingRotation += 0.05f;
	((CVehicleModelInfo*)CModelInfo::GetModelInfo(GetModelIndex()))->SetVehicleColour(m_currentColour1, m_currentColour2);
	if (!CVehicle::bWheelsOnlyCheat)
		CEntity::Render();
#ifdef NEW_RENDERER
	if(!gbNewRenderer)
#endif
	RenderWaterOutPolys();	// not separate function in III
}

void
CBoat::RenderWaterOutPolys(void)
{
	KeepWaterOutIndices[0] = 0;
	KeepWaterOutIndices[1] = 2;
	KeepWaterOutIndices[2] = 1;
	KeepWaterOutIndices[3] = 1;
	KeepWaterOutIndices[4] = 2;
	KeepWaterOutIndices[5] = 3;
	RwIm3DVertexSetRGBA(&KeepWaterOutVertices[0], 255, 255, 255, 255);
	RwIm3DVertexSetRGBA(&KeepWaterOutVertices[1], 255, 255, 255, 255);
	RwIm3DVertexSetRGBA(&KeepWaterOutVertices[2], 255, 255, 255, 255);
	RwIm3DVertexSetRGBA(&KeepWaterOutVertices[3], 255, 255, 255, 255);
	switch (GetModelIndex()) {
	case MI_SPEEDER:
		RwIm3DVertexSetPos(&KeepWaterOutVertices[0], -1.15f, 3.61f, 1.03f);
		RwIm3DVertexSetPos(&KeepWaterOutVertices[1],  1.15f, 3.61f, 1.03f);
		RwIm3DVertexSetPos(&KeepWaterOutVertices[2], -1.15f, 0.06f, 1.03f);
		RwIm3DVertexSetPos(&KeepWaterOutVertices[3],  1.15f, 0.06f, 1.03f);
		break;
	case MI_REEFER:
		RwIm3DVertexSetPos(&KeepWaterOutVertices[0], -1.9f,   2.83f, 1.0f);
		RwIm3DVertexSetPos(&KeepWaterOutVertices[1],  1.9f,   2.83f, 1.0f);
		RwIm3DVertexSetPos(&KeepWaterOutVertices[2], -1.66f, -4.48f, 0.83f);
		RwIm3DVertexSetPos(&KeepWaterOutVertices[3],  1.66f, -4.48f, 0.83f);
		break;
	case MI_PREDATOR:
	default:
		RwIm3DVertexSetPos(&KeepWaterOutVertices[0], -1.45f,   1.9f, 0.96f);
		RwIm3DVertexSetPos(&KeepWaterOutVertices[1],  1.45f,   1.9f, 0.96f);
		RwIm3DVertexSetPos(&KeepWaterOutVertices[2], -1.45f, -3.75f, 0.96f);
		RwIm3DVertexSetPos(&KeepWaterOutVertices[3],  1.45f, -3.75f, 0.96f);
		break;
	}
	KeepWaterOutVertices[0].u = 0.0f;
	KeepWaterOutVertices[0].v = 0.0f;
	KeepWaterOutVertices[1].u = 1.0f;
	KeepWaterOutVertices[1].v = 0.0f;
	KeepWaterOutVertices[2].u = 0.0f;
	KeepWaterOutVertices[2].v = 1.0f;
	KeepWaterOutVertices[3].u = 1.0f;
	KeepWaterOutVertices[3].v = 1.0f;
#ifdef NEW_RENDERER
	if(!gbNewRenderer)
#endif
{
	RwRenderStateSet(rwRENDERSTATETEXTURERASTER, gpWaterRaster);
	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)TRUE);
	RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void*)FALSE);
	RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDZERO);
	RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDONE);
}
	if (!CVehicle::bWheelsOnlyCheat && RwIm3DTransform(KeepWaterOutVertices, 4, GetMatrix().m_attachment, rwIM3D_VERTEXUV)) {
		RwIm3DRenderIndexedPrimitive(rwPRIMTYPETRILIST, KeepWaterOutIndices, 6);
		RwIm3DEnd();
	}
#ifdef NEW_RENDERER
	if(!gbNewRenderer)
#endif
{
	RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void*)TRUE);
	RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDSRCALPHA);
	RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDINVSRCALPHA);
}
}

void
CBoat::Teleport(CVector v)
{
	CWorld::Remove(this);
	SetPosition(v);
	SetOrientation(0.0f, 0.0f, 0.0f);
	SetMoveSpeed(0.0f, 0.0f, 0.0f);
	SetTurnSpeed(0.0f, 0.0f, 0.0f);
	CWorld::Add(this);
}

bool
CBoat::IsSectorAffectedByWake(CVector2D sector, float fSize, CBoat **apBoats)
{
	uint8 numVerts = 0;
	
	if ( apFrameWakeGeneratingBoats[0] == NULL )
		return false;
	
	for ( int32 i = 0; i < 4; i++ )
	{
		CBoat *pBoat = apFrameWakeGeneratingBoats[i];
		if ( !pBoat )
			break;
		
		for ( int j = 0; j < pBoat->m_nNumWakePoints; j++ )
		{
			 float fDist = (WAKE_LIFETIME - pBoat->m_afWakePointLifeTime[j]) * fShapeTime + float(j) * fShapeLength + fSize;
			 
			 if ( Abs(pBoat->m_avec2dWakePoints[j].x - sector.x) < fDist
				&& Abs(pBoat->m_avec2dWakePoints[i].y - sector.y) < fDist )
			 {
				 apBoats[numVerts] = pBoat;
				 numVerts = 1; // += ?
				 break;
			 }
		}
	}
	
	return numVerts != 0;
}

float
CBoat::IsVertexAffectedByWake(CVector vecVertex, CBoat *pBoat)
{
	for ( int i = 0; i < pBoat->m_nNumWakePoints; i++ )
	{
		float fMaxDist = (WAKE_LIFETIME - pBoat->m_afWakePointLifeTime[i]) * fShapeTime + float(i) * fShapeLength;
		
		CVector2D vecDist = pBoat->m_avec2dWakePoints[i] - CVector2D(vecVertex);
		
		float fDist = vecDist.MagnitudeSqr();
		
		if ( fDist < SQR(fMaxDist) )
			return 1.0f - Min(fRangeMult * Sqrt(fDist / SQR(fMaxDist)) + (WAKE_LIFETIME - pBoat->m_afWakePointLifeTime[i]) * fTimeMult, 1.0f);
	}

	return 0.0f;
}

void
CBoat::SetupModelNodes()
{
	int i;
	for(i = 0; i < ARRAY_SIZE(m_aBoatNodes); i++)
		m_aBoatNodes[i] = nil;
	CClumpModelInfo::FillFrameArray(GetClump(), m_aBoatNodes);
}

void
CBoat::FillBoatList()
{
	int16 frameId = 0;
	
	apFrameWakeGeneratingBoats[0] = nil;
	apFrameWakeGeneratingBoats[1] = nil;
	apFrameWakeGeneratingBoats[2] = nil;
	apFrameWakeGeneratingBoats[3] = nil;

	for (int i = CPools::GetVehiclePool()->GetSize() - 1; i >= 0; i--) {
		CBoat *boat = (CBoat *)(CPools::GetVehiclePool()->GetSlot(i));
		if (boat && boat->m_vehType == VEHICLE_TYPE_BOAT) {
			int16 nNumWakePoints = boat->m_nNumWakePoints;
			if (nNumWakePoints != 0) {
				if (frameId >= ARRAY_SIZE(apFrameWakeGeneratingBoats)) {
					int16 frameId2 = -1;
					for (int16 j = 0; j < ARRAY_SIZE(apFrameWakeGeneratingBoats); j++) {
						if (apFrameWakeGeneratingBoats[j]->m_nNumWakePoints < nNumWakePoints) {
							frameId2 = j;
							nNumWakePoints = apFrameWakeGeneratingBoats[j]->m_nNumWakePoints;
						}
					}

					if (frameId2 != -1)
						apFrameWakeGeneratingBoats[frameId2] = boat;
				} else {
					apFrameWakeGeneratingBoats[frameId++] = boat;
				}
			}
		}
	}
}

void
CBoat::PruneWakeTrail(void)
{
	int i;

	for(i = 0; i < ARRAY_SIZE(m_afWakePointLifeTime); i++){
		if(m_afWakePointLifeTime[i] <= 0.0f)
			break;
		if(m_afWakePointLifeTime[i] <= CTimer::GetTimeStep()){
			m_afWakePointLifeTime[i] = 0.0f;
			break;
		}
		m_afWakePointLifeTime[i] -= CTimer::GetTimeStep();
	}
	m_nNumWakePoints = i;
}

void
CBoat::AddWakePoint(CVector point)
{
	int i;
	if(m_afWakePointLifeTime[0] > 0.0f){
		if((CVector2D(GetPosition()) - m_avec2dWakePoints[0]).MagnitudeSqr() < SQR(1.0f)){
			for(i = Min(m_nNumWakePoints, ARRAY_SIZE(m_afWakePointLifeTime)-1); i != 0; i--){
				m_avec2dWakePoints[i] = m_avec2dWakePoints[i-1];
				m_afWakePointLifeTime[i] = m_afWakePointLifeTime[i-1];
			}
			m_avec2dWakePoints[0] = point;
			m_afWakePointLifeTime[0] = 400.0f;
			if(m_nNumWakePoints < ARRAY_SIZE(m_afWakePointLifeTime))
				m_nNumWakePoints++;
		}
	}else{
		m_avec2dWakePoints[0] = point;
		m_afWakePointLifeTime[0] = 400.0f;
		m_nNumWakePoints = 1;
	}
}

#ifdef COMPATIBLE_SAVES
void
CBoat::Save(uint8*& buf)
{
	CVehicle::Save(buf);
	SkipSaveBuf(buf, 1156 - 648);
}

void
CBoat::Load(uint8*& buf)
{
	CVehicle::Load(buf);
	SkipSaveBuf(buf, 1156 - 648);
}
#endif
