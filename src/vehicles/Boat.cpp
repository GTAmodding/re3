#include "common.h"

#include "audio_enums.h"
#include "vehicle_enums.h" //needs to be before Vehicle.h

#include "Vehicle.h"

#include "AnimBlendAssociation.h"
#include "Boat.h"
#include "Camera.h"
#include "CarAI.h"
#include "CarCtrl.h"
#include "DMAudio.h"
#include "Darkel.h"
#include "Explosion.h"
#include "Floater.h"
#include "General.h"
#include "HandlingMgr.h"
#include "ModelIndices.h"
#include "Pad.h"
#include "Particle.h"
#include "ParticleObject.h"
#include "Pools.h"
#include "Record.h"
#include "RpAnimBlend.h"
#include "RwHelper.h"
#include "Shadows.h"
#include "Stats.h"
#include "Timecycle.h"
#include "VisibilityPlugins.h"
#include "Wanted.h"
#include "WaterLevel.h"
#include "Weather.h"
#include "World.h"
#include "main.h"

//--MIAMI: file done

#define INVALID_ORIENTATION (-9999.99f)

float fShapeLength = 0.4f;
float fShapeTime = 0.05f;
float fRangeMult = 0.6f;
float fTimeMult;

float CBoat::MAX_WAKE_LENGTH = 50.0f;
float CBoat::MIN_WAKE_INTERVAL = 2.0f;
float CBoat::WAKE_LIFETIME = 150.0f;

CBoat *CBoat::apFrameWakeGeneratingBoats[4];

const uint32 CBoat::nSaveStructSize =
#ifdef COMPATIBLE_SAVES
	1216;
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
	m_fMovingSpeed = 0.0f;
	m_skimmerThingTimer = 0.0f;
	m_nPoliceShoutTimer = CTimer::GetTimeInMilliseconds();
	SetModelIndex(mi);

	pHandling = mod_HandlingManager.GetHandlingData((tVehicleType)minfo->m_handlingId);
	pFlyingHandling = mod_HandlingManager.GetFlyingPointer((tVehicleType)minfo->m_handlingId);
	pBoatHandling = mod_HandlingManager.GetBoatPointer((tVehicleType)minfo->m_handlingId);
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

	m_boat_unused3 = false;

	m_fVolumeUnderWater = 7.0f;
	m_fPrevVolumeUnderWater = 7.0f;
	m_vecBuoyancePoint = CVector(0.0f, 0.0f, 0.0f);

	m_nDeltaVolumeUnderWater = 0;
	bBoatInWater = true;
	bPropellerInWater = true;

	bIsInWater = true;

	m_phys_unused1 = 0.0f;
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

	if(GetModelIndex() == MI_MARQUIS)
		m_boom.Init(-PI/10.0f, PI/10.0f, 0, 2);
	else
		m_boom.Init(-PI/5.0f, PI/5.0f, 0, 2);
}

void
CBoat::SetModelIndex(uint32 id)
{
	CVehicle::SetModelIndex(id);
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
	bool onLand = m_fDamageImpulse > 0.0f && m_vecDamageNormal.z > 0.1f;

	PruneWakeTrail();

	if(bRenderScorched)
		m_fBuoyancy *= 0.99f;

#ifdef FIX_BUGS
	if(FindPlayerPed() && FindPlayerPed()->m_pWanted->m_nWantedLevel > 0 && GetModelIndex() == MI_PREDATOR){
#else
	if(FindPlayerPed()->m_pWanted->m_nWantedLevel > 0 && GetModelIndex() == MI_PREDATOR){
#endif
		CVehicle *playerVeh = FindPlayerVehicle();
		if(playerVeh && playerVeh->GetVehicleAppearance() == VEHICLE_APPEARANCE_BOAT &&
		   (AutoPilot.m_nCarMission == MISSION_RAMPLAYER_FARAWAY || 
		    AutoPilot.m_nCarMission == MISSION_RAMPLAYER_CLOSE || 
		    AutoPilot.m_nCarMission == MISSION_BLOCKPLAYER_FARAWAY || 
		    AutoPilot.m_nCarMission == MISSION_BLOCKPLAYER_CLOSE || 
		    AutoPilot.m_nCarMission == MISSION_ATTACKPLAYER) &&
		   CTimer::GetTimeInMilliseconds() > m_nPoliceShoutTimer){
			DMAudio.PlayOneShot(m_audioEntityId, SOUND_PED_VCPA_PLAYER_FOUND, 0.0f);
			m_nPoliceShoutTimer = CTimer::GetTimeInMilliseconds() + 4500 + (CGeneral::GetRandomNumber()&0xFFF);
		}
	}

	int r, g, b;
	RwRGBA dropColor = { 0, 0, 0, 0 };
	RwRGBA splashColor, jetColor;
	r = 127.5f*(CTimeCycle::GetAmbientRed_Obj() + 0.5f*CTimeCycle::GetDirectionalRed());
	g = 127.5f*(CTimeCycle::GetAmbientGreen_Obj() + 0.5f*CTimeCycle::GetDirectionalGreen());
	b = 127.5f*(CTimeCycle::GetAmbientBlue_Obj() + 0.5f*CTimeCycle::GetDirectionalBlue());
	r = clamp(r, 0, 255);
	g = clamp(g, 0, 255);
	b = clamp(b, 0, 255);
	splashColor.red = r;
	splashColor.green = g;
	splashColor.blue = b;
	splashColor.alpha = CGeneral::GetRandomNumberInRange(160, 196);

	r = 229.5f*(CTimeCycle::GetAmbientRed() + 0.85f*CTimeCycle::GetDirectionalRed());
	g = 229.5f*(CTimeCycle::GetAmbientGreen() + 0.85f*CTimeCycle::GetDirectionalGreen());
	b = 229.5f*(CTimeCycle::GetAmbientBlue() + 0.85f*CTimeCycle::GetDirectionalBlue());
	r = clamp(r, 0, 255);
	g = clamp(g, 0, 255);
	b = clamp(b, 0, 255);
	jetColor.red = r;
	jetColor.green = g;
	jetColor.blue = b;
	jetColor.alpha = CGeneral::GetRandomNumberInRange(196, 228);

	CGeneral::GetRandomNumber();	// unused

	UpdateClumpAlpha();
	ProcessCarAlarm();

	switch(GetStatus()){
	case STATUS_PLAYER:
		m_bIsAnchored = false;
		m_fOrientation = INVALID_ORIENTATION;
		ProcessControlInputs(0);
		if(GetModelIndex() == MI_PREDATOR)
			DoFixedMachineGuns();

		if (!CRecordDataForChase::IsRecording())
			DoDriveByShootings();
		break;
	case STATUS_SIMPLE:
		m_bIsAnchored = false;
		m_fOrientation = INVALID_ORIENTATION;
		CCarAI::UpdateCarAI(this);
		CPhysical::ProcessControl();
		bBoatInWater = true;
		bPropellerInWater = true;
		bIsInWater = true;
		return;
	case STATUS_PHYSICS:
		m_bIsAnchored = false;
		m_fOrientation = INVALID_ORIENTATION;
		CCarAI::UpdateCarAI(this);
		CCarCtrl::SteerAICarWithPhysics(this);
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
		if((GetPosition() - FindPlayerCentreOfWorld_NoSniperShift()).Magnitude() > 150.0f){
			m_vecMoveSpeed = CVector(0.0f, 0.0f, 0.0f);
			m_vecTurnSpeed = CVector(0.0f, 0.0f, 0.0f);
			return;
		}
		break;
	default: break;
	}

	float collisionDamage = pHandling->fCollisionDamageMultiplier * m_fDamageImpulse;
#ifdef FIX_BUGS
	if(GetStatus() == STATUS_PLAYER && CStats::GetPercentageProgress() >= 100.0f)
		collisionDamage *= 0.5f;
	if (collisionDamage > 25.0f && GetStatus() != STATUS_WRECKED && !bCollisionProof) {
#else
	if(collisionDamage > 25.0f && GetStatus() != STATUS_WRECKED){
#endif
		float prevHealth = m_fHealth;
		if(prevHealth >= 250.0f){
#ifndef FIX_BUGS
			// if collisionDamage < 50 we actually increase health here...
			if(GetStatus() == STATUS_PLAYER && CStats::GetPercentageProgress() >= 100.0f)
				collisionDamage *= 0.5f;
#endif
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
	}

	// Damage particles
	if(m_fHealth <= 460.0f && GetStatus() != STATUS_WRECKED &&
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
		if(m_fHealth < 250.0f){
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
		if(speedSq < 0.25f && m_fHealth <= 390.0f)
			CParticle::AddParticle(PARTICLE_ENGINE_SMOKE, smokePos, 1.25f*smokeDir);
	}

	bool bSeparateTurnForce = bHasHitWall;
	CPhysical::ProcessControl();

	CVector buoyanceImpulse(0.0f, 0.0f, 0.0f);
	CVector buoyancePoint(0.0f, 0.0f, 0.0f);
	if(mod_Buoyancy.ProcessBuoyancyBoat(this, pHandling->fBuoyancy, &buoyancePoint, &buoyanceImpulse, bSeparateTurnForce)){
		// Process boat in water
		if(0.1f * m_fMass * GRAVITY*CTimer::GetTimeStep() < buoyanceImpulse.z){
			bBoatInWater = true;
			bIsInWater = true;
			if (GetUp().z < -0.6f && Abs(GetMoveSpeed().x) < 0.05 && Abs(GetMoveSpeed().y) < 0.05) {
				bIsDrowning = true;
				if (pDriver){
					pDriver->bTouchingWater = true;
					pDriver->InflictDamage(nil, WEAPONTYPE_DROWNING, CTimer::GetTimeStep(), PEDPIECE_TORSO, 0);
				}
			}
			else
				bIsDrowning = false;
		}else{
			bBoatInWater = false;
			bIsInWater = false;
			bIsDrowning = false;
		}

		m_fVolumeUnderWater = mod_Buoyancy.m_volumeUnderWater;
		m_vecBuoyancePoint = buoyancePoint;
		if(GetModelIndex() == MI_SKIMMER && GetUp().z < -0.5f && Abs(m_vecMoveSpeed.x) < 0.2f && Abs(m_vecMoveSpeed.y) < 0.2f)
			ApplyMoveForce(0.03f*buoyanceImpulse);
		else
			ApplyMoveForce(buoyanceImpulse);
		if(bSeparateTurnForce)
			ApplyTurnForce(0.4f*buoyanceImpulse, buoyancePoint);

		// TODO: what is this?
		if(GetModelIndex() == MI_SKIMMER)
			if(m_skimmerThingTimer != 0.0f ||
			   GetForward().z < -0.5f && GetUp().z > -0.5f && m_vecMoveSpeed.z < -0.15f &&
			   buoyanceImpulse.z > 0.01f*m_fMass * GRAVITY*CTimer::GetTimeStep() &&
			   buoyanceImpulse.z < 0.4f*m_fMass * GRAVITY*CTimer::GetTimeStep()){
				float turnImpulse = -0.00017f*GetForward().z*buoyanceImpulse.z * m_fMass*CTimer::GetTimeStep();
				ApplyTurnForce(turnImpulse*GetForward(), GetUp());
				bBoatInWater = false;
				//BUG? aren't we forgetting the timestep here?
				float moveImpulse = -0.5f*DotProduct(m_vecMoveSpeed, GetForward()) * m_fMass;
				ApplyMoveForce(moveImpulse*GetForward());
				if(m_skimmerThingTimer == 0.0f)
					m_skimmerThingTimer = CTimer::GetTimeInMilliseconds() + 300.0f;
				else if(m_skimmerThingTimer < CTimer::GetTimeInMilliseconds())
					m_skimmerThingTimer = 0.0f;
			}

		if(!onLand && bBoatInWater && GetUp().z > 0.0f){
			float impulse = m_vecMoveSpeed.MagnitudeSqr()*pBoatHandling->fAqPlaneForce*buoyanceImpulse.z*CTimer::GetTimeStep()*0.5f;
			if(GetModelIndex() == MI_SKIMMER)
				impulse *= 1.0f + m_fGasPedal;
			else if(m_fGasPedal > 0.05f)
				impulse *= m_fGasPedal;
			else
				impulse = 0.0f;
			impulse = Min(impulse, GRAVITY*pBoatHandling->fAqPlaneLimit*m_fMass*CTimer::GetTimeStep());
			ApplyMoveForce(impulse*GetUp());
			ApplyTurnForce(impulse*GetUp(), buoyancePoint - pBoatHandling->fAqPlaneOffset*GetForward());
		}

		// Handle boat moving forward
		float fwdSpeed = 1.0f;
		if(Abs(m_fGasPedal) > 0.05f || (fwdSpeed = m_vecMoveSpeed.Magnitude2D()) > 0.01f){
			if(bBoatInWater && fwdSpeed > 0.05f)
				AddWakePoint(GetPosition());

			float steerFactor = 1.0f;
			if(GetStatus() == STATUS_PLAYER){
				float steerLoss = DotProduct(m_vecMoveSpeed, GetForward())*pHandling->fTractionBias;
				if(CPad::GetPad(0)->GetHandBrake())
					steerLoss *= 0.5f;
				steerFactor -= steerLoss;
				steerFactor = clamp(steerFactor, 0.0f, 1.0f);
			}

			CVector boundMin = GetColModel()->boundingBox.min;
			CVector propeller(0.0f, boundMin.y*pBoatHandling->fThrustY, boundMin.z*pBoatHandling->fThrustZ);
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

				bool bSlowAhead = false;
				if(Abs(m_fGasPedal) > 0.01f && GetModelIndex() != MI_SKIMMER){
					if(Abs(m_fGasPedal) < 0.05f)
						bSlowAhead = true;

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
						ApplyTurnForce(force * CTimer::GetTimeStep(), propeller - pBoatHandling->fThrustAppZ*GetUp());
						float rightForce = -DotProduct(GetRight(), force)*pHandling->fTractionMultiplier;
						ApplyTurnForce(rightForce*GetRight() * CTimer::GetTimeStep(), GetUp());
					}

					// Spray some particles
					CVector jetDir = -0.04f * force;
					if(m_fGasPedal > 0.0f){
						if(GetStatus() == STATUS_PLAYER){
							CVector sternPos = GetColModel()->boundingBox.min;
							sternPos.x = 0.0f;
							sternPos.z = 0.0f;
							sternPos = Multiply3x3(GetMatrix(), sternPos);

							CVector wakePos = GetPosition() + sternPos;
							// no actual particles for player...
						}else if(IsVisible() && ((CTimer::GetFrameCounter() + m_randomSeed) & 1) &&
						         CVisibilityPlugins::GetDistanceSquaredFromCamera(&propellerWorld) < SQR(70.0f * TheCamera.GenerationDistMultiplier)){
							jetDir.z = 0.015f;
							jetDir.x *= 3.5f;
							jetDir.y *= 3.5f;
							propellerWorld.z += 0.5f;
							
							CParticle::AddParticle(PARTICLE_BOAT_SPLASH, propellerWorld, jetDir, nil, 1.25f, jetColor,
								CGeneral::GetRandomNumberInRange(0, 5),
								CGeneral::GetRandomNumberInRange(0, 90), 1, 500);
							
							CParticle::AddParticle(PARTICLE_CAR_SPLASH, propellerWorld, 0.75f * jetDir, nil, 0.5f, splashColor,
								CGeneral::GetRandomNumberInRange(0, 30),
								CGeneral::GetRandomNumberInRange(0, 45), 3, 500);
						}
					}
				}else
					bSlowAhead = true;

				if(!onLand && bSlowAhead){
					float force = pHandling->fTractionLoss*DotProduct(m_vecMoveSpeed, GetForward());
					force = Min(force, 0.01f*m_fTurnMass);
					if(m_fGasPedal > 0.01f){
						if(GetStatus() == STATUS_PLAYER)
							force *= (0.55f - Abs(m_fGasPedal)) * 1.3f;
						else
							force *= (0.55f - Abs(m_fGasPedal)) * 2.5f;
					}
					if(m_fGasPedal < 0.0f && force > 0.0f || m_fGasPedal > 0.0f && force < 0.0f)
						force *= -1.0f;
					CVector propellerForce = propellerDepth * Multiply3x3(GetMatrix(), force*CVector(-steerSin, 0.0f, 0.0f));
					ApplyMoveForce(propellerForce * CTimer::GetTimeStep());
					ApplyTurnForce(propellerForce * CTimer::GetTimeStep(), propeller);
					float rightForce = -steerSin * force * 0.75f/steerFactor * Max(CTimer::GetTimeStep(), 0.01f);
					ApplyTurnForce(GetRight() * rightForce, GetUp());
				}
			}else
				bPropellerInWater = false;

			if(pHandling->fSuspensionBias != 0.0f){
				CVector right = CrossProduct(GetForward(), CVector(0.0f, 0.0f, 1.0f));
				float rightSpeed = DotProduct(m_vecMoveSpeed, right);
				float impulse = 0.1f*pHandling->fSuspensionBias * m_fMass * m_fVolumeUnderWater * rightSpeed * CTimer::GetTimeStep();
				ApplyMoveForce(right - impulse * 0.3f * CVector(-right.y, right.x, 0.0f));
			}

			if(GetStatus() == STATUS_PLAYER && CPad::GetPad(0)->GetHandBrake()){
				float fwdSpeed = DotProduct(m_vecMoveSpeed, GetForward());
				if(fwdSpeed > 0.0f){
					float impulse = -0.1f*pHandling->fSuspensionLowerLimit * m_fMass * m_fVolumeUnderWater * fwdSpeed * CTimer::GetTimeStep();
					ApplyMoveForce(impulse * GetForward());
				}
			}
		}

		// Slow down or push down boat as it approaches the world limits
		m_vecMoveSpeed.x = Min(m_vecMoveSpeed.x, -(GetPosition().x - (WORLD_MAX_X-100.0f))*0.01f);	// east
		m_vecMoveSpeed.x = Max(m_vecMoveSpeed.x, -(GetPosition().x - (WORLD_MIN_X+100.0f))*0.01f);	// west
		m_vecMoveSpeed.y = Min(m_vecMoveSpeed.y, -(GetPosition().y - (WORLD_MAX_Y-100.0f))*0.01f);	// north
		m_vecMoveSpeed.y = Max(m_vecMoveSpeed.y, -(GetPosition().y - (WORLD_MIN_Y+100.0f))*0.01f);	// south

		if(!onLand && bBoatInWater && !bSeparateTurnForce)
			ApplyWaterResistance();

		if((GetModelIndex() != MI_SKIMMER || m_skimmerThingTimer == 0.0f) && !bSeparateTurnForce){
			// No idea what exactly is going on here besides drag in YZ
			float fx = Pow(pBoatHandling->vecTurnRes.x, CTimer::GetTimeStep());
			float fy = Pow(pBoatHandling->vecTurnRes.y, CTimer::GetTimeStep());
			float fz = Pow(pBoatHandling->vecTurnRes.z, CTimer::GetTimeStep());
			m_vecTurnSpeed = Multiply3x3(m_vecTurnSpeed, GetMatrix());	// invert - to local space
			// TODO: figure this out
			float magic = 1.0f/(1000.0f * SQR(m_vecTurnSpeed.x) + 1.0f) * fx;
			m_vecTurnSpeed.y *= fy;
			m_vecTurnSpeed.z *= fz;
			float forceUp = (magic - 1.0f) * m_vecTurnSpeed.x * m_fTurnMass;
			m_vecTurnSpeed = Multiply3x3(GetMatrix(), m_vecTurnSpeed);	// back to world
			CVector com = Multiply3x3(GetMatrix(), m_vecCentreOfMass);
			ApplyTurnForce(forceUp*GetUp(), com + GetForward());
		}

		m_nDeltaVolumeUnderWater = (m_fVolumeUnderWater-m_fPrevVolumeUnderWater)*10000;

		// Falling into water
		if(!onLand && bBoatInWater && GetUp().z > 0.0f){
			float splashVol = m_nDeltaVolumeUnderWater*pBoatHandling->fWaveAudioMult;
			if(splashVol > 200.0f)
				DMAudio.PlayOneShot(m_audioEntityId, SOUND_CAR_SPLASH, splashVol);

			if(m_nDeltaVolumeUnderWater > 200){
				float speedUp = m_vecMoveSpeed.MagnitudeSqr() * m_nDeltaVolumeUnderWater * 0.001f;
				if(speedUp + m_vecMoveSpeed.z > pHandling->fBrakeDeceleration)
					speedUp = pHandling->fBrakeDeceleration - m_vecMoveSpeed.z;
				if(speedUp < 0.0f) speedUp = 0.0f;
				float speedFwd = DotProduct(m_vecMoveSpeed, GetForward());
				speedFwd *= -m_nDeltaVolumeUnderWater * 0.01f * pHandling->fBrakeBias;
				CVector speed = speedFwd*GetForward() + CVector(0.0f, 0.0f, speedUp);
				CVector splashImpulse = speed * m_fMass;
				ApplyMoveForce(splashImpulse);
				ApplyTurnForce(splashImpulse, buoyancePoint);
			}
		}

		// Splashes
		float speed = m_vecMoveSpeed.Magnitude();
		if(speed > 0.05f && GetUp().x > 0.0f && !TheCamera.GetLookingForwardFirstPerson() && IsVisible() &&
		   (AutoPilot.m_nCarMission != MISSION_CRUISE || (CTimer::GetFrameCounter()&2) == 0)){
			CVector splashPos, splashDir;
			float splashSize, front, waterLevel;

			switch(GetModelIndex()){
			case MI_RIO:
				splashSize = speed;
				front = 0.9f * GetColModel()->boundingBox.max.y;
				splashDir = -0.5f * m_vecMoveSpeed;
				splashDir.z += 0.25f*speed;
				splashDir += 0.35f*speed*GetRight();
				splashPos = GetPosition() + 1.85f*GetRight() + front*GetForward();
				splashPos.z += 0.5f;
				break;
			case MI_SQUALO:
				splashSize = speed;
				front = 0.75f * GetColModel()->boundingBox.max.y;
				splashDir = -0.125f * m_vecMoveSpeed;
				splashDir.z += 0.15f*speed;
				splashDir += 0.25f*speed*GetRight();
				splashPos = GetPosition() + m_vecBuoyancePoint + 0.5f*GetRight() + front*GetForward();
				splashPos.z += 0.5f;
				break;
			case MI_REEFER:
				splashSize = speed;
				front = 0.75f * GetColModel()->boundingBox.max.y;
				splashDir = -0.5f * m_vecMoveSpeed;
				splashDir.z += 0.15f*speed;
				splashDir += 0.5f*speed*GetRight();
				splashPos = GetPosition() + m_vecBuoyancePoint + 1.3f*GetRight() + front*GetForward();
				break;
			case MI_COASTG:
				splashSize = 0.25f*speed;
				front = 0.8f * GetColModel()->boundingBox.max.y;
				splashDir = 0.165f * m_vecMoveSpeed;
				splashDir.z += 0.25f*speed;
				splashDir += 0.15f*speed*GetRight();
				splashPos = GetPosition() + 0.65f*GetRight() + front*GetForward();
				splashPos.z += 0.5f;
				break;
			case MI_DINGHY:
				splashSize = 0.25f*speed;
				front = 0.9f * GetColModel()->boundingBox.max.y;
				splashDir = 0.35f * m_vecMoveSpeed;
				splashDir.z += 0.25f*speed;
				splashDir += 0.25f*speed*GetRight();
				splashPos = GetPosition() + 0.6f*GetRight() + front*GetForward();
				splashPos.z += 0.5f;
				break;
			default:
				splashSize = speed;
				front = 0.9f * GetColModel()->boundingBox.max.y;
				splashDir = -0.5f * m_vecMoveSpeed;
				splashDir.z += 0.25f*speed;
				splashDir += 0.35f*speed*GetRight();
				splashPos = GetPosition() + m_vecBuoyancePoint + 0.5f*GetRight() + front*GetForward();
				break;
			}
			if(splashSize > 0.75f) splashSize = 0.75f;
			if(AutoPilot.m_nCarMission == MISSION_CRUISE)
				splashDir *= 1.5f;
			static float lifeMult = 1000.0f;
			static float lifeBase = 300.0f;
			splashDir.z += 0.0003f*m_nDeltaVolumeUnderWater;
			CWaterLevel::GetWaterLevel(splashPos, &waterLevel, true);
			if(splashPos.z-waterLevel < 3.0f &&
			   CVisibilityPlugins::GetDistanceSquaredFromCamera(&splashPos) < SQR(70.0f * TheCamera.GenerationDistMultiplier)){
				splashPos.z = waterLevel + 0.1f;
				CParticle::AddParticle(PARTICLE_CAR_SPLASH, splashPos, 0.75f*splashDir, nil, splashSize+0.1f,  splashColor,
					CGeneral::GetRandomNumberInRange(0.0f, 10.0f), CGeneral::GetRandomNumberInRange(0.0f, 90.0f),
					1, lifeBase + splashDir.z*lifeMult);
				CParticle::AddParticle(PARTICLE_BOAT_SPLASH, splashPos, splashDir, nil, splashSize,  jetColor,
					CGeneral::GetRandomNumberInRange(0.0f, 0.4f), CGeneral::GetRandomNumberInRange(0.0f, 45.0f),
					0, lifeBase + splashDir.z*lifeMult);
			}

			switch(GetModelIndex()){
			case MI_RIO:
				splashDir = -0.5f * m_vecMoveSpeed;
				splashDir.z += 0.25f*speed;
				splashDir -= 0.35f*speed*GetRight();
				splashPos = GetPosition() - 1.85f*GetRight() + front*GetForward();
				splashPos.z += 0.5f;
				break;
			case MI_SQUALO:
				splashDir = -0.125f * m_vecMoveSpeed;
				splashDir.z += 0.15f*speed;
				splashDir -= 0.25f*speed*GetRight();
				splashPos = GetPosition() + m_vecBuoyancePoint - 0.5f*GetRight() + front*GetForward();
				splashPos.z += 0.5f;
				break;
			case MI_REEFER:
				splashDir = -0.5f * m_vecMoveSpeed;
				splashDir.z += 0.15f*speed;
				splashDir -= 0.5f*speed*GetRight();
				splashPos = GetPosition() + m_vecBuoyancePoint - 1.3f*GetRight() + front*GetForward();
				break;
			case MI_COASTG:
				splashDir = 0.165f * m_vecMoveSpeed;
				splashDir.z += 0.25f*speed;
				splashDir -= 0.15f*speed*GetRight();
				splashPos = GetPosition() - 0.65f*GetRight() + front*GetForward();
				splashPos.z += 0.5f;
				break;
			case MI_DINGHY:
				splashDir = 0.35f * m_vecMoveSpeed;
				splashDir.z += 0.25f*speed;
				splashDir -= 0.25f*speed*GetRight();
				splashPos = GetPosition() - 0.6f*GetRight() + front*GetForward();
				splashPos.z += 0.5f;
				break;
			default:
				splashDir = -0.5f * m_vecMoveSpeed;
				splashDir.z += 0.25f*speed;
				splashDir -= 0.35f*speed*GetRight();
				splashPos = GetPosition() + m_vecBuoyancePoint - 0.5f*GetRight() + front*GetForward();
				break;
			}
			if(AutoPilot.m_nCarMission == MISSION_CRUISE)
				splashDir *= 1.5f;
			splashDir.z += 0.0003f*m_nDeltaVolumeUnderWater;
			CWaterLevel::GetWaterLevel(splashPos, &waterLevel, true);
			if(splashPos.z-waterLevel < 3.0f &&
			   CVisibilityPlugins::GetDistanceSquaredFromCamera(&splashPos) < SQR(70.0f * TheCamera.GenerationDistMultiplier)){
				splashPos.z = waterLevel + 0.1f;
				CParticle::AddParticle(PARTICLE_CAR_SPLASH, splashPos, 0.75f*splashDir, nil, splashSize+0.1f,  splashColor,
					CGeneral::GetRandomNumberInRange(0.0f, 10.0f), CGeneral::GetRandomNumberInRange(0.0f, 90.0f),
					1, lifeBase + splashDir.z*lifeMult);
				CParticle::AddParticle(PARTICLE_BOAT_SPLASH, splashPos, splashDir, nil, splashSize,  jetColor,
					CGeneral::GetRandomNumberInRange(0.0f, 0.4f), CGeneral::GetRandomNumberInRange(0.0f, 45.0f),
					0, lifeBase + splashDir.z*lifeMult);
			}
		}

		// Spray waterdrops on screen
		if(TheCamera.GetLookingForwardFirstPerson() && FindPlayerVehicle() && FindPlayerVehicle()->IsBoat() &&
		   m_nDeltaVolumeUnderWater > 0 && numWaterDropOnScreen < 20){
			CVector dropPos;
			CVector dropDir(CGeneral::GetRandomNumberInRange(-0.25f, 0.25f), CGeneral::GetRandomNumberInRange(1.0f, 0.75f), 0.0f);

			int frm = CGeneral::GetRandomNumber() & 1;
			if(TheCamera.m_CameraAverageSpeed < 0.35f){
				dropPos.x = CGeneral::GetRandomNumberInRange(50, (int)SCREEN_WIDTH-50);
				dropPos.y = CGeneral::GetRandomNumberInRange(50, (int)SCREEN_HEIGHT-50);
			}else{
				dropPos.x = CGeneral::GetRandomNumberInRange(200, (int)SCREEN_WIDTH-200);
				dropPos.y = CGeneral::GetRandomNumberInRange(150, (int)SCREEN_HEIGHT-150);
			}
			dropPos.z = 1.0f;

			if(TheCamera.m_CameraAverageSpeed > 0.35f){
				if((int)SCREEN_WIDTH / 2 < dropPos.x)
					dropPos.x += CGeneral::GetRandomNumberInRange(0.35f, TheCamera.m_CameraAverageSpeed)*7.5f;
				else
					dropPos.x -= CGeneral::GetRandomNumberInRange(0.35f, TheCamera.m_CameraAverageSpeed)*7.5f;

				if((int)SCREEN_HEIGHT / 2 < dropPos.y)
					dropPos.y += CGeneral::GetRandomNumberInRange(0.35f, TheCamera.m_CameraAverageSpeed)*7.5f;
				else
					dropPos.y -= CGeneral::GetRandomNumberInRange(0.35f, TheCamera.m_CameraAverageSpeed)*7.5f;
			}

			if(CParticle::AddParticle(PARTICLE_WATERDROP, dropPos, dropDir, nil,
					CGeneral::GetRandomNumberInRange(0.1f, 0.15f), dropColor, 0, 0, frm))
				numWaterDropOnScreen++;
		}

		if(m_fPrevVolumeUnderWater == 0.0f && m_fVolumeUnderWater > 0.0f && GetModelIndex() == MI_SKIMMER){
			CVector splashDir(0.0f, 0.0f, 0.25f*speed);
			CVector splashPos = GetPosition();
			float level;
			CWaterLevel::GetWaterLevel(splashPos, &level, true);
			splashPos.z = level;
			CParticleObject::AddObject(POBJECT_CAR_WATER_SPLASH, splashPos, splashDir, 0.0f, 65, splashColor, true);
		}

		m_fPrevVolumeUnderWater = m_fVolumeUnderWater;
	}else{
		bBoatInWater = false;
		bIsInWater = false;
#ifdef FIX_BUGS
		bIsDrowning = false;
#endif
	}

	if(m_modelIndex == MI_SKIMMER && CTimer::GetTimeStep() > 0.0f){
		if(GetStatus() == STATUS_PLAYER){
			if(m_fMovingSpeed < 0.22f)
				m_fMovingSpeed += 0.001f*CTimer::GetTimeStep();
			FlyingControl(FLIGHT_MODEL_SEAPLANE);
		}else{
			if(m_fMovingSpeed > 0.0005f*CTimer::GetTimeStep())
				m_fMovingSpeed -= 0.0005f*CTimer::GetTimeStep();
			else
				m_fMovingSpeed = 0.0f;
		}
	}else if(bCheat8)
		FlyingControl(FLIGHT_MODEL_PLANE);

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
		m_fAccelerate = -m_fBrake*0.3f;

	m_fSteeringLeftRight += (-CPad::GetPad(pad)->GetSteeringLeftRight()/128.0f - m_fSteeringLeftRight)*0.2f;
	m_fSteeringLeftRight = clamp(m_fSteeringLeftRight, -1.0f, 1.0f);

	float steeringSq = m_fSteeringLeftRight < 0.0f ? -SQR(m_fSteeringLeftRight) : SQR(m_fSteeringLeftRight);
	m_fSteerAngle = pHandling->fSteeringLock * DEGTORAD(steeringSq);
	m_fGasPedal = m_fAccelerate;
}

float fSeaPlaneWaterResistance = 30.0f;

void
CBoat::ApplyWaterResistance(void)
{
	// TODO: figure out how this works
	float resistance = 0.001f * pHandling->fSuspensionForceLevel * SQR(m_fVolumeUnderWater) * m_fMass;
	if(GetModelIndex() == MI_SKIMMER)
		resistance *= fSeaPlaneWaterResistance;
	float fwdSpeed = DotProduct(GetMoveSpeed(), GetForward());
	float magic = (SQR(fwdSpeed) + 0.05f) * resistance + 1.0f;
	magic = Abs(magic);
	float fx = Pow(pBoatHandling->vecMoveRes.x/magic, 0.5f*CTimer::GetTimeStep());
	float fy = Pow(pBoatHandling->vecMoveRes.y/magic, 0.5f*CTimer::GetTimeStep());
	float fz = Pow(pBoatHandling->vecMoveRes.z/magic, 0.5f*CTimer::GetTimeStep());

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

	KillPedsInVehicle();

	bEngineOn = false;
	bLightsOn = false;
	ChangeLawEnforcerState(false);

	CExplosion::AddExplosion(this, culprit, EXPLOSION_BOAT, GetPosition(), 0);
	CDarkel::RegisterCarBlownUpByPlayer(this);
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
	obj->bDontStream = true;

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

void
CBoat::PreRender(void)
{
	CMatrix matrix;
	CVector pos;
	RpAtomic *atomic;

	if(GetModelIndex() == MI_SKIMMER){
		m_fMovingRotation += m_fMovingSpeed*CTimer::GetTimeStep();
		if(m_fMovingRotation > TWOPI) m_fMovingRotation -= TWOPI;
		int alpha = (1.0f - Min(2.0f*m_fMovingSpeed*8.0f/PI, 1.0f))*255.0f;
		if(GetStatus() == STATUS_PLAYER || GetStatus() == STATUS_PLAYER_REMOTE || GetStatus() == STATUS_PLAYER_PLAYBACKFROMBUFFER){
			if(m_aBoatNodes[BOAT_RUDDER]){
				float sine = Sin(m_fSteerAngle);
				matrix.Attach(RwFrameGetMatrix(m_aBoatNodes[BOAT_RUDDER]));
				pos = matrix.GetPosition();
				matrix.SetRotate(0.0f, 0.0f, -m_fSteerAngle);
				matrix.Rotate(0.0f, DEGTORAD(22.0f)*sine, 0.0f);
				matrix.Translate(pos);
				matrix.UpdateRW();
			}
			if(m_aBoatNodes[BOAT_FLAP_LEFT]){
				matrix.Attach(RwFrameGetMatrix(m_aBoatNodes[BOAT_FLAP_LEFT]));
				pos = matrix.GetPosition();
				matrix.SetRotateX(-m_fSteerAngle);
				matrix.Translate(pos);
				matrix.UpdateRW();
			}
			if(m_aBoatNodes[BOAT_FLAP_RIGHT]){
				matrix.Attach(RwFrameGetMatrix(m_aBoatNodes[BOAT_FLAP_RIGHT]));
				pos = matrix.GetPosition();
				matrix.SetRotateX(m_fSteerAngle);
				matrix.Translate(pos);
				matrix.UpdateRW();
			}
			// FIX: Planes can also be controlled with GetCarGunUpDown
#ifdef FIX_BUGS
			static float steeringUpDown = 0.0f;
			steeringUpDown += ((Abs(CPad::GetPad(0)->GetCarGunUpDown()) > 1.0f ? (-CPad::GetPad(0)->GetCarGunUpDown() / 128.0f) : (-CPad::GetPad(0)->GetSteeringUpDown() / 128.0f)) - steeringUpDown) * Min(1.f, CTimer::GetTimeStep() / 5.f);
#else
			float steeringUpDown = -CPad::GetPad(0)->GetSteeringUpDown()/128.0f;
#endif
			if(m_aBoatNodes[BOAT_REARFLAP_LEFT]){
				matrix.Attach(RwFrameGetMatrix(m_aBoatNodes[BOAT_REARFLAP_LEFT]));
				pos = matrix.GetPosition();
				matrix.SetRotateX(steeringUpDown);
				matrix.Translate(pos);
				matrix.UpdateRW();
			}
			if(m_aBoatNodes[BOAT_REARFLAP_RIGHT]){
				matrix.Attach(RwFrameGetMatrix(m_aBoatNodes[BOAT_REARFLAP_RIGHT]));
				pos = matrix.GetPosition();
				matrix.SetRotateX(steeringUpDown);
				matrix.Translate(pos);
				matrix.UpdateRW();
			}
		}
		if(m_aBoatNodes[BOAT_MOVING]){
			matrix.Attach(RwFrameGetMatrix(m_aBoatNodes[BOAT_MOVING]));
			pos = matrix.GetPosition();
			matrix.SetRotateY(m_fMovingRotation);
			matrix.Translate(pos);
			matrix.UpdateRW();

			atomic = nil;
			RwFrameForAllObjects(m_aBoatNodes[BOAT_MOVING], GetBoatAtomicObjectCB, &atomic);
			if(atomic)
				SetComponentAtomicAlpha(atomic, alpha);
		}
		if(m_aBoatNodes[BOAT_WINDSCREEN]){
			matrix.Attach(RwFrameGetMatrix(m_aBoatNodes[BOAT_WINDSCREEN]));
			pos = matrix.GetPosition();
			matrix.SetRotateY(-m_fMovingRotation);
			matrix.Translate(pos);
			matrix.UpdateRW();

			atomic = nil;
			RwFrameForAllObjects(m_aBoatNodes[BOAT_WINDSCREEN], GetBoatAtomicObjectCB, &atomic);
			if(atomic)
				SetComponentAtomicAlpha(atomic, Max(150-alpha, 0));
		}
		CShadows::StoreShadowForVehicle(this, VEH_SHD_TYPE_SEAPLANE);
	}else if(GetModelIndex() == MI_COASTG || GetModelIndex() == MI_DINGHY || GetModelIndex() == MI_RIO ||
	         GetModelIndex() == MI_SQUALO || GetModelIndex() == MI_MARQUIS){
		if(m_aBoatNodes[BOAT_RUDDER]){
			matrix.Attach(RwFrameGetMatrix(m_aBoatNodes[BOAT_RUDDER]));
			pos = matrix.GetPosition();
			matrix.SetRotateZ(-m_fSteerAngle);
			matrix.Translate(pos);
			matrix.UpdateRW();
		}
		if(m_aBoatNodes[BOAT_REARFLAP_LEFT]){
			matrix.Attach(RwFrameGetMatrix(m_aBoatNodes[BOAT_REARFLAP_LEFT]));
			pos = matrix.GetPosition();
			matrix.SetRotateZ(-m_fSteerAngle);
			matrix.Translate(pos);
			matrix.UpdateRW();
		}
		if(m_aBoatNodes[BOAT_REARFLAP_RIGHT]){
			matrix.Attach(RwFrameGetMatrix(m_aBoatNodes[BOAT_REARFLAP_RIGHT]));
			pos = matrix.GetPosition();
			matrix.SetRotateZ(-m_fSteerAngle);
			matrix.Translate(pos);
			matrix.UpdateRW();
		}
	}

	if(GetModelIndex() == MI_RIO || GetModelIndex() == MI_MARQUIS){
		float axes[3] = { 0.0f, 0.0f, 0.0f };
		m_boom.Process(this);
		axes[m_boom.m_nAxis] = m_boom.m_fAngle;
		matrix.Attach(RwFrameGetMatrix(m_aBoatNodes[BOAT_FLAP_LEFT]));
		pos = matrix.GetPosition();
		matrix.SetRotate(axes[0], axes[1], axes[2]);
		matrix.Translate(pos);
		matrix.UpdateRW();
	}

	if(GetModelIndex() == MI_RIO){
		// That little wind propeller
		if(m_aBoatNodes[BOAT_FLAP_RIGHT]){
			matrix.Attach(RwFrameGetMatrix(m_aBoatNodes[BOAT_FLAP_RIGHT]));
			pos = matrix.GetPosition();

			float flapHeading = matrix.GetForward().Heading();
			float boatHeading = GetForward().Heading();
			float rot = -DEGTORAD(45.0f) - (flapHeading + boatHeading);
			// eh what?
			rot = CGeneral::LimitRadianAngle(rot);
			if(rot > HALFPI) rot = PI;
			else if(rot < -HALFPI) rot = -PI;
			rot = clamp(rot, -DEGTORAD(63.0f), DEGTORAD(63.0f));
			m_fMovingSpeed += (0.008f * CWeather::Wind + 0.002f) * rot;
			m_fMovingSpeed *= Pow(0.9985f, CTimer::GetTimeStep())/(500.0f*SQR(m_fMovingSpeed) + 1.0f);

			matrix.SetRotateZ(flapHeading + m_fMovingSpeed*CTimer::GetTimeStep());
			matrix.Translate(pos);
			matrix.UpdateRW();
		}
		if(m_aBoatNodes[BOAT_MOVING]){
			matrix.Attach(RwFrameGetMatrix(m_aBoatNodes[BOAT_MOVING]));
			pos = matrix.GetPosition();
			matrix.SetRotateY(m_fMovingRotation);
			matrix.Translate(pos);
			matrix.UpdateRW();

			CVector wind  = CVector(0.707f, 0.707f, 0.0f) * (CWeather::Wind + 0.15f)*0.4f;
			m_fMovingRotation += (m_vecMoveSpeed + wind).Magnitude()*CTimer::GetTimeStep();
		}
	}else if(GetModelIndex() == MI_PREDATOR || GetModelIndex() == MI_REEFER){
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
		m_fMovingRotation += 0.02f * CTimer::GetTimeStep();
	}
}

RwIm3DVertex KeepWaterOutVertices[4];
RwImVertexIndex KeepWaterOutIndices[6];

void
CBoat::Render()
{
	((CVehicleModelInfo*)CModelInfo::GetModelInfo(GetModelIndex()))->SetVehicleColour(m_currentColour1, m_currentColour2);
	m_nSetPieceExtendedRangeTime = CTimer::GetTimeInMilliseconds() + 3000;
	if (!CVehicle::bWheelsOnlyCheat)
		CEntity::Render();
#ifdef NEW_RENDERER
	if(!gbNewRenderer)
#endif
	RenderWaterOutPolys();	// not separate function in VC
}

void
CBoat::RenderWaterOutPolys(void)
{
	if(GetModelIndex() == MI_SKIMMER)
		return;
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
	case MI_RIO:
		RwIm3DVertexSetPos(&KeepWaterOutVertices[0], -1.3f, -1.016f, 0.51f);
		RwIm3DVertexSetPos(&KeepWaterOutVertices[1],  1.3f, -1.016f, 0.51f);
		RwIm3DVertexSetPos(&KeepWaterOutVertices[2], -1.3f, -2.832f, 0.51f);
		RwIm3DVertexSetPos(&KeepWaterOutVertices[3],  1.3f, -2.832f, 0.51f);
		break;
	case MI_SQUALO:
		RwIm3DVertexSetPos(&KeepWaterOutVertices[0], -1.222f,  2.004f, 0.846f);
		RwIm3DVertexSetPos(&KeepWaterOutVertices[1],  1.222f,  2.004f, 0.846f);
		RwIm3DVertexSetPos(&KeepWaterOutVertices[2], -1.24f,  -1.367f, 0.846f);
		RwIm3DVertexSetPos(&KeepWaterOutVertices[3],  1.24f,  -1.367f, 0.846f);
		break;
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
		RwIm3DVertexSetPos(&KeepWaterOutVertices[0], -1.45f,   1.9f, 0.96f);
		RwIm3DVertexSetPos(&KeepWaterOutVertices[1],  1.45f,   1.9f, 0.96f);
		RwIm3DVertexSetPos(&KeepWaterOutVertices[2], -1.45f, -3.75f, 0.96f);
		RwIm3DVertexSetPos(&KeepWaterOutVertices[3],  1.45f, -3.75f, 0.96f);
		break;
	case MI_TROPIC:
		RwIm3DVertexSetPos(&KeepWaterOutVertices[0], -1.886f, -2.347f, 0.787f);
		RwIm3DVertexSetPos(&KeepWaterOutVertices[1],  1.886f, -2.347f, 0.787f);
		RwIm3DVertexSetPos(&KeepWaterOutVertices[2], -1.886f, -4.67f,  0.842f);
		RwIm3DVertexSetPos(&KeepWaterOutVertices[3],  1.886f, -4.67f,  0.842f);
		break;
	case MI_COASTG:
		RwIm3DVertexSetPos(&KeepWaterOutVertices[0], -0.663f, 3.565f, 0.382f);
		RwIm3DVertexSetPos(&KeepWaterOutVertices[1],  0.663f, 3.565f, 0.382f);
		RwIm3DVertexSetPos(&KeepWaterOutVertices[2], -1.087f, 0.83f,  0.381f);
		RwIm3DVertexSetPos(&KeepWaterOutVertices[3],  1.087f, 0.83f,  0.381f);
		break;
	case MI_DINGHY:
		RwIm3DVertexSetPos(&KeepWaterOutVertices[0], -0.797f,  1.641f, 0.573f);
		RwIm3DVertexSetPos(&KeepWaterOutVertices[1],  0.797f,  1.641f, 0.573f);
		RwIm3DVertexSetPos(&KeepWaterOutVertices[2], -0.865f, -1.444f, 0.509f);
		RwIm3DVertexSetPos(&KeepWaterOutVertices[3],  0.865f, -1.444f, 0.509f);
		break;
	case MI_MARQUIS:
		RwIm3DVertexSetPos(&KeepWaterOutVertices[0], -1.246f, -1.373f, 0.787f);
		RwIm3DVertexSetPos(&KeepWaterOutVertices[1],  1.246f, -1.373f, 0.787f);
		RwIm3DVertexSetPos(&KeepWaterOutVertices[2], -1.023f, -5.322f, 0.787f);
		RwIm3DVertexSetPos(&KeepWaterOutVertices[3],  1.023f, -5.322f, 0.787f);
		break;
	default:
		return;
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
	bool drawAnotherRect = false;
	if(GetModelIndex() == MI_COASTG){
		drawAnotherRect = true;
		RwIm3DVertexSetPos(&KeepWaterOutVertices[0], -1.087f,  0.831f, 0.381f);
		RwIm3DVertexSetPos(&KeepWaterOutVertices[1],  1.087f,  0.831f, 0.381f);
		RwIm3DVertexSetPos(&KeepWaterOutVertices[2], -1.097f, -2.977f, 0.381f);
		RwIm3DVertexSetPos(&KeepWaterOutVertices[3],  1.097f, -2.977f, 0.381f);
	}
	if(drawAnotherRect){
		KeepWaterOutVertices[0].u = 0.0f;
		KeepWaterOutVertices[0].v = 0.0f;
		KeepWaterOutVertices[1].u = 1.0f;
		KeepWaterOutVertices[1].v = 0.0f;
		KeepWaterOutVertices[2].u = 0.0f;
		KeepWaterOutVertices[2].v = 1.0f;
		KeepWaterOutVertices[3].u = 1.0f;
		KeepWaterOutVertices[3].v = 1.0f;
		if (!CVehicle::bWheelsOnlyCheat && RwIm3DTransform(KeepWaterOutVertices, 4, GetMatrix().m_attachment, rwIM3D_VERTEXUV)) {
			RwIm3DRenderIndexedPrimitive(rwPRIMTYPETRILIST, KeepWaterOutIndices, 6);
			RwIm3DEnd();
		}
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

//--MIAMI: unused
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

//--MIAMI: unused
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
	CVector2D camPos = TheCamera.GetPosition();
	CVector2D camFwd = TheCamera.GetForward();
	float camDist = camFwd.Magnitude();
	if(camDist > 0.0f)
		camFwd /= camDist;
	for (int i = CPools::GetVehiclePool()->GetSize() - 1; i >= 0; i--) {
		CBoat *boat = (CBoat *)(CPools::GetVehiclePool()->GetSlot(i));
		if (boat && boat->m_vehType == VEHICLE_TYPE_BOAT) {
			if (boat->m_nNumWakePoints != 0) {
				CVector2D camToBoat = CVector2D(boat->GetPosition()) - camPos;
				float distToCam = DotProduct2D(camFwd, camToBoat);
				if(distToCam > 100.0f || distToCam < -15.0f)
					continue;
				float distSq = camToBoat.MagnitudeSqr();
				if(distSq > SQR(70.0f))
					continue;
				if (frameId >= ARRAY_SIZE(apFrameWakeGeneratingBoats)) {
					float nearest = 999999.88f;
					int16 frameId2 = -1;
					for (int16 j = 0; j < ARRAY_SIZE(apFrameWakeGeneratingBoats); j++) {
						float tmpDistSq = (CVector2D(apFrameWakeGeneratingBoats[j]->GetPosition()) - camPos).MagnitudeSqr();
						if (tmpDistSq < nearest) {
							nearest = tmpDistSq;
							frameId2 = j;
						}
					}

					if (frameId2 != -1 &&
					    (distSq < nearest || boat->GetStatus() == STATUS_PLAYER))
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
		if((CVector2D(GetPosition()) - m_avec2dWakePoints[0]).MagnitudeSqr() < SQR(2.0f)) {
			if(GetStatus() == STATUS_PLAYER){
				if(m_nNumWakePoints >= 31)
					m_nNumWakePoints = 31;
			}else if(VehicleCreatedBy == MISSION_VEHICLE){
				if(m_nNumWakePoints >= 20)
					m_nNumWakePoints = 20;
			}else{
				if(m_nNumWakePoints >= 15)
					m_nNumWakePoints = 15;
			}
			for(i = m_nNumWakePoints; i != 0; i--){
				m_avec2dWakePoints[i] = m_avec2dWakePoints[i-1];
				m_afWakePointLifeTime[i] = m_afWakePointLifeTime[i-1];
			}
			m_avec2dWakePoints[0] = point;
			m_afWakePointLifeTime[0] = 150.0f;
			if(m_nNumWakePoints < ARRAY_SIZE(m_afWakePointLifeTime))
				m_nNumWakePoints++;
		}
	}else{
		m_avec2dWakePoints[0] = point;
		m_afWakePointLifeTime[0] = 150.0f;
		m_nNumWakePoints = 1;
	}
}

void
CBoat::DoDriveByShootings(void)
{
	CAnimBlendAssociation *anim = nil;
	CPlayerInfo* playerInfo = ((CPlayerPed*)pDriver)->GetPlayerInfoForThisPlayerPed();
	if (playerInfo && !playerInfo->m_bDriveByAllowed)
		return;

	CWeapon *weapon = pDriver->GetWeapon();
	if(CWeaponInfo::GetWeaponInfo(weapon->m_eWeaponType)->m_nWeaponSlot != 5)
		return;

	weapon->Update(pDriver->m_audioEntityId, nil);

	bool lookingLeft = false;
	bool lookingRight = false;
	if(TheCamera.Cams[TheCamera.ActiveCam].Mode == CCam::MODE_TOPDOWN ||
	   TheCamera.m_bObbeCinematicCarCamOn){
		if(CPad::GetPad(0)->GetLookLeft())
			lookingLeft = true;
		if(CPad::GetPad(0)->GetLookRight())
			lookingRight = true;
	}else{
		if(TheCamera.Cams[TheCamera.ActiveCam].LookingLeft)
			lookingLeft = true;
		if(TheCamera.Cams[TheCamera.ActiveCam].LookingRight)
			lookingRight = true;
	}

	if(lookingLeft || lookingRight){
		if(lookingLeft){
			anim = RpAnimBlendClumpGetAssociation(pDriver->GetClump(), ANIM_DRIVEBY_R);
			if(anim)
				anim->blendDelta = -1000.0f;
			anim = RpAnimBlendClumpGetAssociation(pDriver->GetClump(), ANIM_DRIVEBY_L);
			if(anim == nil || anim->blendDelta < 0.0f)
				anim = CAnimManager::AddAnimation(pDriver->GetClump(), ASSOCGRP_STD, ANIM_DRIVEBY_L);
		}else if(pDriver->m_pMyVehicle->pPassengers[0] == nil || TheCamera.Cams[TheCamera.ActiveCam].Mode == CCam::MODE_1STPERSON){
			anim = RpAnimBlendClumpGetAssociation(pDriver->GetClump(), ANIM_DRIVEBY_L);
			if(anim)
				anim->blendDelta = -1000.0f;
			anim = RpAnimBlendClumpGetAssociation(pDriver->GetClump(), ANIM_DRIVEBY_R);
			if(anim == nil || anim->blendDelta < 0.0f)
				anim = CAnimManager::AddAnimation(pDriver->GetClump(), ASSOCGRP_STD, ANIM_DRIVEBY_R);
		}

		if (!anim || !anim->IsRunning()) {
			if (CPad::GetPad(0)->GetCarGunFired() && CTimer::GetTimeInMilliseconds() > weapon->m_nTimer) {
				weapon->FireFromCar(this, lookingLeft, true);
				weapon->m_nTimer = CTimer::GetTimeInMilliseconds() + 70;
			}
		}
	}else{
		weapon->Reload();
		anim = RpAnimBlendClumpGetAssociation(pDriver->GetClump(), ANIM_DRIVEBY_L);
		if(anim)
			anim->blendDelta = -1000.0f;
		anim = RpAnimBlendClumpGetAssociation(pDriver->GetClump(), ANIM_DRIVEBY_R);
		if(anim)
			anim->blendDelta = -1000.0f;
	}
}

#ifdef COMPATIBLE_SAVES
void
CBoat::Save(uint8*& buf)
{
	CVehicle::Save(buf);
	SkipSaveBuf(buf, 1216 - 672);
}

void
CBoat::Load(uint8*& buf)
{
	CVehicle::Load(buf);
	SkipSaveBuf(buf, 1216 - 672);
}
#endif
