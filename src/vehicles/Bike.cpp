#include "common.h"
#include "General.h"
#include "Pad.h"
#include "DMAudio.h"
#include "Camera.h"
#include "Darkel.h"
#include "Rubbish.h"
#include "Explosion.h"
#include "Particle.h"
#include "World.h"
#include "SurfaceTable.h"
#include "Record.h"
#include "CarCtrl.h"
#include "CarAI.h"
#include "Stats.h"
#include "Replay.h"
#include "AnimManager.h"
#include "RpAnimBlend.h"
#include "AnimBlendAssociation.h"
#include "Ped.h"
#include "PlayerPed.h"
#include "DamageManager.h"
#include "Vehicle.h"
#include "Automobile.h"
#include "Bike.h"
#include "Debug.h"

#define FAKESUSPENSION (99999.992f)

CBike::CBike(int32 id, uint8 CreatedBy)
 : CVehicle(CreatedBy)
{
	int i;
	CVehicleModelInfo *mi = (CVehicleModelInfo*)CModelInfo::GetModelInfo(id);
	switch(id){
	case MI_ANGEL:
	case MI_FREEWAY:
		m_bikeAnimType = ASSOCGRP_BIKE_HARLEY;
		break;
	case MI_PIZZABOY:
	case MI_FAGGIO:
		m_bikeAnimType = ASSOCGRP_BIKE_VESPA;
		break;
	case MI_PCJ600:
		m_bikeAnimType = ASSOCGRP_BIKE_STANDARD;
		break;
	case MI_SANCHEZ:
		m_bikeAnimType = ASSOCGRP_BIKE_DIRT;
		break;
	default: assert(0 && "invalid bike model ID");
	}
	m_vehType = VEHICLE_TYPE_BIKE;

	m_fFireBlowUpTimer = 0.0f;
	m_doingBurnout = 0;
	m_bike_flag01 = false;

	SetModelIndex(id);

	pHandling = mod_HandlingManager.GetHandlingData((eHandlingId)mi->m_handlingId);
	pBikeHandling = mod_HandlingManager.GetBikePointer((eHandlingId)mi->m_handlingId);
	pFlyingHandling = mod_HandlingManager.GetFlyingPointer((eHandlingId)mi->m_handlingId);

	m_bike_unused1 = 20.0f;
	m_bike_unused2 = 0;

	mi->ChooseVehicleColour(m_currentColour1, m_currentColour2);

	m_fRearForkLength = 0.0f;
	m_fFrontForkY = 0.0;
	m_fFrontForkZ = 0.0;
	m_fFrontForkSlope = Tan(DEGTORAD(mi->m_bikeSteerAngle));

	m_fMass = pHandling->fMass;
	m_fTurnMass = pHandling->fTurnMass;
	m_vecCentreOfMass = pHandling->CentreOfMass;
	m_vecCentreOfMass.z = 0.1f;
	m_fAirResistance = pHandling->Dimension.x*pHandling->Dimension.z/m_fMass;
	m_fElasticity = 0.05f;
	m_fBuoyancy = pHandling->fBuoyancy;

	m_fSteerAngle = 0.0f;
	m_fWheelAngle = 0.0f;
	m_fLeanLRAngle = 0.0f;
	m_fLeanLRAngle2 = 0.0f;
	m_fGasPedal = 0.0f;
	m_fBrakePedal = 0.0f;
	m_fLeanInput = 0.0f;
	m_fPedLeanAmountLR = 0.0f;
	m_fPedLeanAmountUD = 0.0f;
	m_pSetOnFireEntity = nil;
	m_pBombRigger = nil;
	m_fGasPedalAudio = 0.0f;
	m_bike_flag02 = false;
	bWaterTight = false;
	m_bike_flag08 = false;
	bIsStanding = false;
	bExtraSpeed = false;
	bIsOnFire = false;
	m_bike_flag80 = false;

	m_fTireTemperature = 0.0f;
	m_fBrakeDestabilization = 0.0f;
	field_490 = 0;

	for(i = 0; i < 2; i++){
		m_aWheelRotation[i] = 0.0f;
		m_aWheelSpeed[i] = 0.0f;
		m_aWheelState[i] = WHEEL_STATE_NORMAL;
		m_aWheelSkidmarkType[i] = SKIDMARK_NORMAL;
		m_aWheelSkidmarkBloody[i] = false;
		m_aWheelSkidmarkUnk[0] = false;
		m_wheelStatus[i] = WHEEL_STATUS_OK;
	}

	for(i = 0; i < 4; i++){
		m_aGroundPhysical[i] = nil;
		m_aGroundOffset[i] = CVector(0.0f, 0.0f, 0.0f);
		m_aSuspensionSpringRatioPrev[i] = m_aSuspensionSpringRatio[i] = 1.0f;
		m_aWheelTimer[i] = 0.0f;
	}

	m_nWheelsOnGround = 0;
	m_nDriveWheelsOnGround = 0;
	m_nDriveWheelsOnGroundPrev = 0;
	m_fHeightAboveRoad = 0.0f;
	m_fTraction = 1.0f;

	CColModel *colModel = mi->GetColModel();
	if(colModel->lines == nil){
		colModel->lines = (CColLine*)RwMalloc(4*sizeof(CColLine));
		colModel->numLines = 4;
	}
	// BUG? this would make more sense in the if above
	colModel->lines[0].p0.z = FAKESUSPENSION;

	SetupSuspensionLines();

	AutoPilot.m_nCarMission = MISSION_NONE;
	AutoPilot.m_nTempAction = TEMPACT_NONE;
	AutoPilot.m_nTimeToStartMission = CTimer::GetTimeInMilliseconds();
	AutoPilot.m_bStayInCurrentLevel = false;

	SetStatus(STATUS_SIMPLE);
	bUseCollisionRecords = true;
	m_nNumPassengers = 0;
	bIsVan = false;
	bIsBus = false;
	bIsBig = false;
	bLowVehicle = false;
	bPedPhysics = false;

	bLeanMatrixClean = false;
	m_leanMatrix = GetMatrix();
}

void
CBike::SetModelIndex(uint32 id)
{
	CVehicle::SetModelIndex(id);
	SetupModelNodes();
}

#define SAND_SLOWDOWN (0.02f)
CVector vecTestResistance(0.9995f, 0.9f, 0.95f);
float fDAxisX = 1.0f;
float fDAxisXExtra = 100.0f;
float fDAxisY = 1000.0f;
float fInAirXRes = 0.88f;
float fFlySpeedMult = -0.6f;

void
CBike::ProcessControl(void)
{
	int i;
	float wheelRot;
	float acceleration = 0.0f;
	bool bCanStand = false;
	bool bStuckInSand = false;
	float brake = 0.0f;
	CColModel *colModel = GetColModel();
	float wheelScale = ((CVehicleModelInfo*)CModelInfo::GetModelInfo(GetModelIndex()))->m_wheelScale;
	bWarnedPeds = false;
	bLeanMatrixClean = false;
	m_doingBurnout = 0;
	bExtraSpeed = false;
	bRestingOnPhysical = false;

	if(CReplay::IsPlayingBack())
		return;

	ProcessCarAlarm();

	ActivateBombWhenEntered();

	CRubbish::StirUp(this);

	UpdateClumpAlpha();

	AutoPilot.m_bSlowedDownBecauseOfCars = false;
	AutoPilot.m_bSlowedDownBecauseOfPeds = false;

	switch(GetStatus()){
	case STATUS_PLAYER:
		bCanStand = true;
		m_bike_flag08 = false;
		if(FindPlayerPed()->GetPedState() != PED_EXIT_CAR && FindPlayerPed()->GetPedState() != PED_DRAG_FROM_CAR){
			ProcessControlInputs(0);

			if(m_fLeanInput < 0.0f){
				m_vecCentreOfMass.y = pHandling->CentreOfMass.y + pBikeHandling->fLeanBakCOM*m_fLeanInput;
				CVector com = m_vecCentreOfMass;
#ifdef FIX_BUGS
				// center of mass has to have world space orientation. unfortunately we can't do wheelies
				// at high speed then, flipping y here is like riding south without this fix where wheelies work
				com.y = -com.y;
				com = Multiply3x3(GetMatrix(), com);
#endif
				if(m_fBrakePedal == 0.0f && !bIsHandbrakeOn || m_nWheelsOnGround == 0){
					if(GetModelIndex() == MI_SANCHEZ){
						float force = m_fLeanInput*m_fTurnMass*pBikeHandling->fLeanBackForce*Min(m_vecMoveSpeed.Magnitude(), 0.1f);
						force *= 0.7f*m_fGasPedal + 0.3f;
						ApplyTurnForce(-force*CTimer::GetTimeStep()*GetUp(), com+GetForward());
					}else{
						float force = m_fLeanInput*m_fTurnMass*pBikeHandling->fLeanBackForce*Min(m_vecMoveSpeed.Magnitude(), 0.1f);
						force *= 0.5f*m_fGasPedal + 0.5f;
						ApplyTurnForce(-force*CTimer::GetTimeStep()*GetUp(), com+GetForward());
					}
				}
			}else{
				m_vecCentreOfMass.y = pHandling->CentreOfMass.y + pBikeHandling->fLeanFwdCOM*m_fLeanInput;
				CVector com = m_vecCentreOfMass;
#ifdef FIX_BUGS
				// see above
				com.y = -com.y;
				com = Multiply3x3(GetMatrix(), com);
#endif
				if(m_fBrakePedal < 0.0f || m_nWheelsOnGround == 0){
					float force = m_fLeanInput*m_fTurnMass*pBikeHandling->fLeanFwdForce*Min(m_vecMoveSpeed.Magnitude(), 0.1f);
					ApplyTurnForce(-force*CTimer::GetTimeStep()*GetUp(), com+GetForward());
				}
			}

			PruneReferences();

			if(GetStatus() == STATUS_PLAYER && !CRecordDataForChase::IsRecording())
				DoDriveByShootings();

			if(m_aSuspensionSpringRatio[0] < 1.0f && CSurfaceTable::GetAdhesionGroup(m_aWheelColPoints[0].surfaceB) == ADHESIVE_SAND ||
			   m_aSuspensionSpringRatio[1] < 1.0f && CSurfaceTable::GetAdhesionGroup(m_aWheelColPoints[1].surfaceB) == ADHESIVE_SAND ||
			   m_aSuspensionSpringRatio[2] < 1.0f && CSurfaceTable::GetAdhesionGroup(m_aWheelColPoints[2].surfaceB) == ADHESIVE_SAND ||
			   m_aSuspensionSpringRatio[3] < 1.0f && CSurfaceTable::GetAdhesionGroup(m_aWheelColPoints[3].surfaceB) == ADHESIVE_SAND){
				CVector parallelSpeed = m_vecMoveSpeed - DotProduct(m_vecMoveSpeed, GetUp())*GetUp();
				if(m_fGasPedal > 0.3f){
					if(parallelSpeed.MagnitudeSqr() < SQR(0.3f))
						bStuckInSand = true;
					parallelSpeed -= DotProduct(parallelSpeed, GetForward())*GetForward();
				}
				ApplyMoveForce(parallelSpeed * -CTimer::GetTimeStep()*SAND_SLOWDOWN*m_fMass);
			}
		}
		if(CPad::GetPad(0)->WeaponJustDown())
			ActivateBomb();
		break;

	case STATUS_PLAYER_PLAYBACKFROMBUFFER:
		bCanStand = true;
		break;

	case STATUS_SIMPLE:
		CCarAI::UpdateCarAI(this);
		CPhysical::ProcessControl();
		CCarCtrl::UpdateCarOnRails(this);

		m_nWheelsOnGround = 2;
		m_nDriveWheelsOnGroundPrev = m_nDriveWheelsOnGround;
		m_nDriveWheelsOnGround = 2;

		pHandling->Transmission.CalculateGearForSimpleCar(AutoPilot.m_fMaxTrafficSpeed/50.0f, m_nCurrentGear);

		wheelRot = ProcessWheelRotation(WHEEL_STATE_NORMAL, GetForward(), m_vecMoveSpeed, 0.5f*wheelScale);
		for(i = 0; i < 2; i++)
			m_aWheelRotation[i] += wheelRot;

		PlayHornIfNecessary();
		ReduceHornCounter();
		bVehicleColProcessed = false;
		bAudioChangingGear = false;
		m_bike_flag80 = false;
		// that's all we do for simple vehicles
		return;

	case STATUS_PHYSICS:
		CCarAI::UpdateCarAI(this);
		CCarCtrl::SteerAICarWithPhysics(this);
		PlayHornIfNecessary();

		bCanStand = true;
		m_bike_flag80 = false;

		if(bIsBeingCarJacked){
			m_fGasPedal = 0.0f;
			m_fBrakePedal = 1.0f;
			bIsHandbrakeOn = true;
		}else
			m_bike_flag08 = false;
		break;

	case STATUS_ABANDONED:
		m_fBrakePedal = 0.0f;
		if(m_vecMoveSpeed.MagnitudeSqr() < SQR(0.1f) || bIsStanding)
			bIsHandbrakeOn = true;
		else	
			bIsHandbrakeOn = false;

		m_fGasPedal = 0.0f;
#ifdef FIX_BUGS
		if(!IsAlarmOn())
#endif
			m_nCarHornTimer = 0;

		bCanStand = (pDriver || pPassengers[0] || bIsBeingCarJacked) && !bIsStanding;
		m_fPedLeanAmountLR = 0.0f;
		m_fPedLeanAmountUD = 0.0f;
		m_bike_flag80 = false;

		if(bIsBeingCarJacked){
			m_fGasPedal = 0.0f;
			m_fBrakePedal = 1.0f;
			bIsHandbrakeOn = true;
		}
		break;

	case STATUS_WRECKED:
		m_fBrakePedal = 0.05f;
		bIsHandbrakeOn = true;

		m_fSteerAngle = 0.0f;
		m_fGasPedal = 0.0f;
#ifdef FIX_BUGS
		if(!IsAlarmOn())
#endif
			m_nCarHornTimer = 0;

		bCanStand = false;
		m_bike_flag80 = false;
		m_fPedLeanAmountLR = 0.0f;
		m_fPedLeanAmountUD = 0.0f;
		break;

	case STATUS_PLAYER_DISABLED:
		if(m_vecMoveSpeed.MagnitudeSqr() < SQR(0.1f)){
			m_fBrakePedal = 1.0f;
			bIsHandbrakeOn = true;
		}else{
			m_fBrakePedal = 0.0f;
			bIsHandbrakeOn = false;
		}

		m_fSteerAngle = 0.0f;
		m_fGasPedal = 0.0f;
#ifdef FIX_BUGS
		if(!IsAlarmOn())
#endif
			m_nCarHornTimer = 0;

		bCanStand = true;
		m_bike_flag80 = false;
		break;
	}

	if(bIsStanding)
		if(Abs(GetRight().z) > 0.35f || Abs(GetForward().z) > 0.5f)
			bIsStanding = false;

	if(bCanStand || m_bike_flag08 || bIsStanding){
		float fDx = fDAxisX;
		CVector res = vecTestResistance;
		CVector localTurnSpeed = Multiply3x3(m_vecTurnSpeed, GetMatrix());

		if(GetStatus() == STATUS_PLAYER){
			if(m_aWheelTimer[BIKESUSP_F1] == 0.0f && m_aWheelTimer[BIKESUSP_F2] == 0.0f){
				fDx = fDAxisXExtra;
				if(!(m_aWheelTimer[BIKESUSP_R1] == 0.0f && m_aWheelTimer[BIKESUSP_R2] == 0.0f) &&
				   GetForward().z > 0.0f)
					res.x -= Max(0.25f*Abs(pBikeHandling->fWheelieAng-GetForward().z), 0.07f);
				else
					res.x = fInAirXRes;
			}else if(m_aWheelTimer[BIKESUSP_R1] == 0.0f && m_aWheelTimer[BIKESUSP_R2] == 0.0f){
				fDx = fDAxisXExtra;
				if(GetForward().z < 0.0f)
					res.x *= Max(0.3f*Abs(pBikeHandling->fStoppieAng-GetForward().z), 0.1f) + 0.9f;
			}
		}

		res.x *= 1.0f/(fDx*SQR(localTurnSpeed.x) + 1.0f);
		res.y *= 1.0f/(fDAxisY*SQR(localTurnSpeed.y) + 1.0f);
		res.x = Pow(res.x, CTimer::GetTimeStep());
		res.y = Pow(res.y, CTimer::GetTimeStep());
		float turnX = localTurnSpeed.x*(res.x - 1.0f);
		float turnY = localTurnSpeed.y*(res.y - 1.0f);

		res = -GetUp() * turnY * m_fTurnMass;
		ApplyTurnForce(res, GetRight() + Multiply3x3(GetMatrix(), m_vecCentreOfMass));

		res = GetUp() * turnX * m_fTurnMass;
		ApplyTurnForce(res, GetForward() + Multiply3x3(GetMatrix(), m_vecCentreOfMass));

		if(GetStatus() != STATUS_PLAYER)
			m_vecCentreOfMass = pHandling->CentreOfMass;
	}else{
		m_vecCentreOfMass = pHandling->CentreOfMass;
		m_vecCentreOfMass.z = pBikeHandling->fNoPlayerCOMz;
	}

	// Skip physics if object is found to have been static recently
	bool skipPhysics = false;
	if(!bIsStuck && (GetStatus() == STATUS_ABANDONED || GetStatus() == STATUS_WRECKED) && !m_bike_flag08){
		bool makeStatic = false;
		float moveSpeedLimit, turnSpeedLimit, distanceLimit;

		if(!bVehicleColProcessed &&
		   m_vecMoveSpeed.IsZero() &&
		// BUG? m_aSuspensionSpringRatioPrev[3] is checked twice in the game. also, why 3?
		   m_aSuspensionSpringRatioPrev[3] != 1.0f)
			makeStatic = true;

		if(GetStatus() == STATUS_WRECKED){
			moveSpeedLimit = 0.006f;
			turnSpeedLimit = 0.0015f;
			distanceLimit = 0.015f;
		}else{
			moveSpeedLimit = 0.003f;
			turnSpeedLimit = 0.0009f;
			distanceLimit = 0.005f;
		}

		m_vecMoveSpeedAvg = (m_vecMoveSpeedAvg + m_vecMoveSpeed)/2.0f;
		m_vecTurnSpeedAvg = (m_vecTurnSpeedAvg + m_vecTurnSpeed)/2.0f;

		if(m_vecMoveSpeedAvg.MagnitudeSqr() <= sq(moveSpeedLimit*CTimer::GetTimeStep()) &&
		   m_vecTurnSpeedAvg.MagnitudeSqr() <= sq(turnSpeedLimit*CTimer::GetTimeStep()) &&
		   m_fDistanceTravelled < distanceLimit &&
		   makeStatic){
			m_nStaticFrames++;

			if(m_nStaticFrames > 10 || makeStatic)
				if(!CCarCtrl::MapCouldMoveInThisArea(GetPosition().x, GetPosition().y)){
					if(!makeStatic || m_nStaticFrames > 10)
						m_nStaticFrames = 10;

					skipPhysics = true;

					m_vecMoveSpeed = CVector(0.0f, 0.0f, 0.0f);
					m_vecTurnSpeed = CVector(0.0f, 0.0f, 0.0f);
				}
		}else
			m_nStaticFrames = 0;
	}

	// Postpone
	for(i = 0; i < 4; i++)
		if(m_aGroundPhysical[i]){
			bRestingOnPhysical = true;
			if(!CWorld::bForceProcessControl && m_aGroundPhysical[i]->bIsInSafePosition){
				bWasPostponed = true;
				return;
			}
		}

	if(bRestingOnPhysical){
		skipPhysics = false;
		m_nStaticFrames = 0;
	}

	VehicleDamage();

	if(skipPhysics){
		bHasContacted = false;
		bIsInSafePosition = false;
		bWasPostponed = false;
		bHasHitWall = false;
		m_nCollisionRecords = 0;
		bHasCollided = false;
		bVehicleColProcessed = false;
		bAudioChangingGear = false;
		m_nDamagePieceType = 0;
		m_fDamageImpulse = 0.0f;
		m_pDamageEntity = nil;
		m_vecTurnFriction = CVector(0.0f, 0.0f, 0.0f);
		m_vecMoveFriction = CVector(0.0f, 0.0f, 0.0f);
// missing. BUG?
//		m_fTireTemperature = 1.0f;

		if(bIsStanding && m_fWheelAngle < DEGTORAD(20.0f))
			m_fWheelAngle += DEGTORAD(1.0f)*CTimer::GetTimeStep();
		if(bIsStanding){
			float f = Pow(0.97f, CTimer::GetTimeStep());
			m_fLeanLRAngle2 = m_fLeanLRAngle2*f - (Asin(clamp(GetRight().z,-1.0f,1.0f))+DEGTORAD(15.0f))*(1.0f-f);
			m_fLeanLRAngle = m_fLeanLRAngle2;
		}
	}else{

		// This has to be done if ProcessEntityCollision wasn't called
		if(!bVehicleColProcessed){
			CMatrix mat(GetMatrix());
			bIsStuck = false;
			bHasContacted = false;
			bIsInSafePosition = false;
			bWasPostponed = false;
			bHasHitWall = false;
			m_fDistanceTravelled = 0.0f;
			m_bIsVehicleBeingShifted = false;
			bSkipLineCol = false;
			ApplyMoveSpeed();
			ApplyTurnSpeed();
			for(i = 0; CheckCollision() && i < 5; i++){
				GetMatrix() = mat;
				ApplyMoveSpeed();
				ApplyTurnSpeed();
			}
			bIsInSafePosition = true;
			bIsStuck = false;			
		}

		if(!(bCanStand || m_bike_flag08 || bIsStanding)){
			if(GetRight().z < 0.0f){
				if(m_fSteerAngle > -DEGTORAD(25.0f))
					m_fSteerAngle -= DEGTORAD(0.5f)*CTimer::GetTimeStep();
			}else{
				if(m_fSteerAngle < DEGTORAD(25.0f))
					m_fSteerAngle += DEGTORAD(0.5f)*CTimer::GetTimeStep();
			}
		}

		// Lean forward speed up
		float savedAirResistance = m_fAirResistance;
		if(GetStatus() == STATUS_PLAYER && pDriver){
			CAnimBlendAssociation *assoc = RpAnimBlendClumpGetAssociation(pDriver->GetClump(), ANIM_BIKE_FWD);
			if(assoc && assoc->blendAmount > 0.5f &&
			   assoc->currentTime > 0.06f && assoc->currentTime < 0.14f){
				m_fAirResistance *= 0.6f;
				if(m_fGasPedal > 0.5f && DotProduct(m_vecMoveSpeed, GetForward()) > 0.25f){
					ApplyMoveForce(0.2f*m_fMass*GRAVITY*CTimer::GetTimeStep()*GetForward());
					bExtraSpeed = true;
				}
			}
		}

		CPhysical::ProcessControl();
		m_fAirResistance = savedAirResistance;

		ProcessBuoyancy();
		// Rescale spring ratios, i.e. subtract wheel radius
		for(i = 0; i < 4; i++){
			// wheel radius in relation to suspension line
			float wheelRadius = 1.0f - m_aSuspensionSpringLength[i]/m_aSuspensionLineLength[i];
			// rescale such that 0.0 is fully compressed and 1.0 is fully extended
			m_aSuspensionSpringRatio[i] = (m_aSuspensionSpringRatio[i]-wheelRadius)/(1.0f-wheelRadius);
		}

		int rnd = 0;
		float fwdSpeed = Abs(DotProduct(m_vecMoveSpeed, GetForward()));
		CVector contactPoints[4];	// relative to model
		CVector contactSpeeds[4];	// speed at contact points
		CVector springDirections[4];	// normalized, in model space

		for(i = 0; i < 4; i++){
			// Set spring under certain circumstances
			if(m_wheelStatus[i/2] == WHEEL_STATUS_MISSING)
				m_aSuspensionSpringRatio[i] = 1.0f;
			else if(m_wheelStatus[i/2] == WHEEL_STATUS_BURST){
				// wheel more bumpy the faster we are
				if(i == BIKESUSP_F1 || BIKESUSP_R1)
					rnd = CGeneral::GetRandomNumberInRange(0, (uint16)(40*fwdSpeed) + 98) < 100;
				if(rnd){
					m_aSuspensionSpringRatio[i] += 0.3f*(m_aSuspensionLineLength[i]-m_aSuspensionSpringLength[i])/m_aSuspensionSpringLength[i];
					if(m_aSuspensionSpringRatio[i] > 1.0f)
						m_aSuspensionSpringRatio[i] = 1.0f;
				}
			}

			// get points and directions if spring is compressed
			if(m_aSuspensionSpringRatio[i] < 1.0f){
				contactPoints[i] = m_aWheelColPoints[i].point - GetPosition();
				springDirections[i] = Multiply3x3(GetMatrix(), colModel->lines[i].p1 - colModel->lines[i].p0);
				springDirections[i].Normalise();
			}
		}

		m_aWheelSkidmarkType[0] = m_aWheelSkidmarkType[1] = SKIDMARK_NORMAL;
		m_aWheelSkidmarkUnk[0] = m_aWheelSkidmarkUnk[1] = false;

		// Make springs push up vehicle
		for(i = 0; i < 4; i++){
			if(m_aSuspensionSpringRatio[i] < 1.0f){
				float bias = pHandling->fSuspensionBias;
				if(i == BIKESUSP_R1 || i == BIKESUSP_R2)
					bias = 1.0f - bias;

				if(m_aWheelColPoints[i].normal.z > 0.35f)
					ApplySpringCollisionAlt(pHandling->fSuspensionForceLevel,
						springDirections[i], contactPoints[i],
						m_aSuspensionSpringRatio[i], bias, m_aWheelColPoints[i].normal);
				else
					ApplySpringCollision(pHandling->fSuspensionForceLevel,
						springDirections[i], contactPoints[i],
						m_aSuspensionSpringRatio[i], bias);

				if(m_aWheelColPoints[i].surfaceB == SURFACE_GRASS ||
				   m_aWheelColPoints[i].surfaceB == SURFACE_MUD_DRY){
					if(i < 2)
						m_aWheelSkidmarkType[0] = SKIDMARK_MUDDY;
					else
						m_aWheelSkidmarkType[1] = SKIDMARK_MUDDY;
				}else if(m_aWheelColPoints[i].surfaceB == SURFACE_SAND ||
				         m_aWheelColPoints[i].surfaceB == SURFACE_SAND_BEACH){
					if(i < 2){
						m_aWheelSkidmarkType[0] = SKIDMARK_SANDY;
						m_aWheelSkidmarkUnk[0] = true;
					}else{
						m_aWheelSkidmarkType[1] = SKIDMARK_SANDY;
						m_aWheelSkidmarkUnk[1] = true;
					}
				}
			}else{
				contactPoints[i] = Multiply3x3(GetMatrix(), colModel->lines[i].p1);
			}
		}

		// Get speed at contact points
		for(i = 0; i < 4; i++){
			contactSpeeds[i] = GetSpeed(contactPoints[i]);
			if(m_aGroundPhysical[i]){
				// subtract movement of physical we're standing on
				contactSpeeds[i] -= m_aGroundPhysical[i]->GetSpeed(m_aGroundOffset[i]);
#ifndef FIX_BUGS
				// this shouldn't be reset because we still need it below
				m_aGroundPhysical[i] = nil;
#endif
			}
		}

		CVector normal;
		if(m_aSuspensionSpringRatio[0] < 1.0f || m_aSuspensionSpringRatio[1] < 1.0f){
			normal = m_aSuspensionSpringRatio[0] < 1.0f ? m_aWheelColPoints[0].normal : m_aWheelColPoints[1].normal;
			if(normal.z > 0.35f)
				springDirections[0] = -normal;
			normal = m_aSuspensionSpringRatio[1] < 1.0f ? m_aWheelColPoints[1].normal : m_aWheelColPoints[0].normal;
			if(normal.z > 0.35f)
				springDirections[1] = -normal;
		}
		if(m_aSuspensionSpringRatio[2] < 1.0f || m_aSuspensionSpringRatio[3] < 1.0f){
			normal = m_aSuspensionSpringRatio[2] < 1.0f ? m_aWheelColPoints[2].normal : m_aWheelColPoints[3].normal;
			if(normal.z > 0.35f)
				springDirections[2] = -normal;
			normal = m_aSuspensionSpringRatio[3] < 1.0f ? m_aWheelColPoints[3].normal : m_aWheelColPoints[2].normal;
			if(normal.z > 0.35f)
				springDirections[3] = -normal;
		}

		// game has dead code here if m_vecMoveSpeed.Magnitude() < 0.01f

		// dampen springs
		for(i = 0; i < 4; i++)
			if(m_aSuspensionSpringRatio[i] < 1.0f)
				ApplySpringDampening(pHandling->fSuspensionDampingLevel,
					springDirections[i], contactPoints[i], contactSpeeds[i]);

		// Get speed at contact points again
		for(i = 0; i < 4; i++){
			contactSpeeds[i] = GetSpeed(contactPoints[i]);
			if(m_aGroundPhysical[i]){
				// subtract movement of physical we're standing on
				contactSpeeds[i] -= m_aGroundPhysical[i]->GetSpeed(m_aGroundOffset[i]);
				m_aGroundPhysical[i] = nil;
			}
		}

		bool gripCheat = true;
		fwdSpeed = DotProduct(m_vecMoveSpeed, GetForward());
		if(!CVehicle::bCheat3)
			gripCheat = false;
		float acceleration = pHandling->Transmission.CalculateDriveAcceleration(m_fGasPedal, m_nCurrentGear, m_fChangeGearTime, fwdSpeed, gripCheat);
		acceleration /= m_fForceMultiplier;

		brake = m_fBrakePedal * pHandling->fBrakeDeceleration * CTimer::GetTimeStep();
		bool neutralHandling = GetStatus() != STATUS_PLAYER && GetStatus() != STATUS_PLAYER_REMOTE && (pHandling->Flags & HANDLING_NEUTRALHANDLING);
		float brakeBiasFront = neutralHandling ? 1.0f : 2.0f*pHandling->fBrakeBias;
		float brakeBiasRear  = neutralHandling ? 1.0f : 2.0f*(1.0f-pHandling->fBrakeBias);
		float tractionBiasFront = neutralHandling ? 1.0f : 2.0f*pHandling->fTractionBias;
		float tractionBiasRear  = neutralHandling ? 1.0f : 2.0f-tractionBiasFront;

		// Count how many wheels are touching the ground

		m_nWheelsOnGround = 0;
		m_nDriveWheelsOnGroundPrev = m_nDriveWheelsOnGround;
		m_nDriveWheelsOnGround = 0;

		for(i = 0; i < 4; i++){
			if(m_aSuspensionSpringRatio[i] < 1.0f)
				m_aWheelTimer[i] = 4.0f;
			else
				m_aWheelTimer[i] = Max(m_aWheelTimer[i]-CTimer::GetTimeStep(), 0.0f);

			if(m_aWheelTimer[i] > 0.0f){
				m_nWheelsOnGround++;
				if(i == BIKESUSP_R1 || i == BIKESUSP_R2)
					m_nDriveWheelsOnGround = 1;
				if(m_nWheelsOnGround == 1)
					m_vecAvgSurfaceNormal = m_aWheelColPoints[i].normal;
				else
					m_vecAvgSurfaceNormal += m_aWheelColPoints[i].normal;
			}
		}

		if(m_nWheelsOnGround == 0)
			m_vecAvgSurfaceNormal = CVector(0.0f, 0.0f, 1.0f);
		else{
			m_vecAvgSurfaceNormal /= m_nWheelsOnGround;
			if(DotProduct(m_vecAvgSurfaceNormal, GetUp()) < -0.5f)
				m_vecAvgSurfaceNormal *= -1.0f;
		}

		// Find contact points for wheel processing
		int frontLine = m_aSuspensionSpringRatio[BIKESUSP_F1] < m_aSuspensionSpringRatio[BIKESUSP_F2] ?
			BIKESUSP_F1 : BIKESUSP_F2;
		CVector frontContact(0.0f,
			colModel->lines[BIKESUSP_F1].p0.y,
			colModel->lines[BIKESUSP_F1].p0.z - m_aSuspensionSpringRatio[frontLine]*m_aSuspensionSpringLength[BIKESUSP_F1] - 0.5f*wheelScale);
		frontContact = Multiply3x3(GetMatrix(), frontContact);

		int rearLine = m_aSuspensionSpringRatio[BIKESUSP_R1] < m_aSuspensionSpringRatio[BIKESUSP_R2] ?
			BIKESUSP_R1 : BIKESUSP_R2;
		CVector rearContact(0.0f,
			colModel->lines[BIKESUSP_R1].p0.y,
			colModel->lines[BIKESUSP_R1].p0.z - m_aSuspensionSpringRatio[rearLine]*m_aSuspensionSpringLength[BIKESUSP_R1] - 0.5f*wheelScale);
		rearContact = Multiply3x3(GetMatrix(), rearContact);

		float traction = 0.004f * m_fTraction;
		traction *= pHandling->fTractionMultiplier / 4.0f;

		// Turn wheel
		if(GetStatus() == STATUS_PLAYER || !bIsStanding || m_bike_flag08){
			if(Abs(m_vecMoveSpeed.x) < 0.01f && Abs(m_vecMoveSpeed.y) < 0.01f && m_fSteerAngle == 0.0f){
				m_fWheelAngle *= Pow(0.96f, CTimer::GetTimeStep());
			}else{
				float f;
				if(fwdSpeed > 0.01f && m_aWheelTimer[BIKESUSP_F1] > 0.0f && m_aWheelTimer[BIKESUSP_F2] > 0.0f && GetStatus() == STATUS_PLAYER){
					CColPoint point;
					point.surfaceA = SURFACE_WHEELBASE;
					point.surfaceB = SURFACE_TARMAC;
					float steer = CSurfaceTable::GetAdhesiveLimit(point)*4.0f*pBikeHandling->fSpeedSteer*traction;
					if(CSurfaceTable::GetAdhesionGroup(m_aWheelColPoints[rearLine].surfaceB) == ADHESIVE_LOOSE ||
					   CSurfaceTable::GetAdhesionGroup(m_aWheelColPoints[rearLine].surfaceB) == ADHESIVE_SAND)
						steer *= pBikeHandling->fSlipSteer;
					f = Asin(Min(steer/SQR(fwdSpeed), 1.0))/DEGTORAD(pHandling->fSteeringLock);
					if(m_fSteerAngle < 0.0f && m_fLeanLRAngle < 0.0f &&
					   m_fSteerAngle > 0.0f && m_fLeanLRAngle > 0.0f)
						f *= 2.0f;
					f = Min(f, 1.0f);
				}else{
					f = 1.0f;
				}
				if(GetStatus() != STATUS_PLAYER)
					f = 1.0f;
				m_fWheelAngle = m_fSteerAngle*f;
			}
		}else if(m_fWheelAngle < DEGTORAD(20.0f))
			m_fWheelAngle += DEGTORAD(1.5f)*CTimer::GetTimeStep();

		static float fThrust;
		static tWheelState WheelState[2];
		CVector initialMoveSpeed = m_vecMoveSpeed;
		bool rearWheelsFirst = !!(pHandling->Flags & HANDLING_REARWHEEL_1ST);

		// Process front wheel - first try

		if(!rearWheelsFirst){
			if(m_aWheelTimer[BIKESUSP_F1] > 0.0f || m_aWheelTimer[BIKESUSP_F2] > 0.0f){
				// Wheel on ground
				eBikeWheelSpecial spec;
				if(m_aWheelTimer[BIKESUSP_R1] > 0.0f || m_aWheelTimer[BIKESUSP_R2] > 0.0f)
					spec = BIKE_WHEELSPEC_0;
				else
					spec = BIKE_WHEELSPEC_2;
				CVector wheelFwd = Multiply3x3(GetMatrix(), CVector(-Sin(m_fWheelAngle), Cos(m_fWheelAngle), 0.0f));
				wheelFwd -= DotProduct(wheelFwd, m_aWheelColPoints[frontLine].normal)*m_aWheelColPoints[frontLine].normal;
				wheelFwd.Normalise();
				CVector wheelRight = CrossProduct(wheelFwd, m_aWheelColPoints[frontLine].normal);
				wheelRight.Normalise();

				fThrust = 0.0f;
				m_aWheelColPoints[frontLine].surfaceA = SURFACE_WHEELBASE;
				float adhesion = CSurfaceTable::GetAdhesiveLimit(m_aWheelColPoints[frontLine])*traction;
				float adhesionDestab = 1.0f;
				if(m_fBrakeDestabilization > 0.0f)
					switch(CSurfaceTable::GetAdhesionGroup(m_aWheelColPoints[frontLine].surfaceB)){
					case ADHESIVE_HARD:
					case ADHESIVE_LOOSE:
						adhesionDestab = 0.9f;
						break;
					case ADHESIVE_ROAD:
						adhesionDestab = 0.7f;
						break;
					}
				if(GetStatus() == STATUS_PLAYER)
					adhesion *= CSurfaceTable::GetWetMultiplier(m_aWheelColPoints[frontLine].surfaceB);
				if(m_wheelStatus[BIKEWHEEL_FRONT] == WHEEL_STATUS_BURST)
					adhesion *= 0.4f;
				WheelState[BIKEWHEEL_FRONT] = m_aWheelState[BIKEWHEEL_FRONT];
				CVector contactSpeed = GetSpeed(frontContact);
				ProcessBikeWheel(wheelFwd, wheelRight,
					contactSpeed, frontContact,
					2, fThrust,
					brake*brakeBiasFront,
					adhesion*tractionBiasFront, adhesionDestab,
					BIKEWHEEL_FRONT,
					&m_aWheelSpeed[BIKEWHEEL_FRONT],
					&WheelState[BIKEWHEEL_FRONT],
					spec,
					m_wheelStatus[BIKEWHEEL_FRONT]);
				if(bStuckInSand && (WheelState[BIKEWHEEL_FRONT] == WHEEL_STATE_SPINNING || WheelState[BIKEWHEEL_FRONT] == WHEEL_STATE_SKIDDING))
					WheelState[BIKEWHEEL_FRONT] = WHEEL_STATE_NORMAL;
			}else{
				// Wheel in the air
				m_aWheelSpeed[BIKEWHEEL_FRONT] *= 0.95f;
				m_aWheelRotation[BIKEWHEEL_FRONT] += m_aWheelSpeed[BIKEWHEEL_FRONT];
			}
		}

		// Process rear wheel

		if(m_aWheelTimer[BIKESUSP_R1] > 0.0f || m_aWheelTimer[BIKESUSP_R2] > 0.0f){
			// Wheel on ground
			float rearBrake = brake;
			float rearTraction = traction;

			CVector wheelFwd = GetForward();
			CVector wheelRight = GetRight();
			wheelFwd -= DotProduct(wheelFwd, m_aWheelColPoints[rearLine].normal)*m_aWheelColPoints[rearLine].normal;
			wheelFwd.Normalise();
			wheelRight = CrossProduct(wheelFwd, m_aWheelColPoints[rearLine].normal);
			wheelRight.Normalise();

			if(bIsHandbrakeOn){
#ifdef FIX_BUGS
				// Not sure if this is needed, but brake usually has timestep as a factor
				rearBrake = 20000.0f * CTimer::GetTimeStepFix();
#else
				rearBrake = 20000.0f;
#endif
				m_fTireTemperature = 1.0f;
			}else if(m_doingBurnout){
				rearBrake = 0.0f;
				rearTraction = 0.0f;
				ApplyTurnForce(contactPoints[BIKESUSP_R1], -0.0007f*m_fTurnMass*m_fSteerAngle*GetRight()*CTimer::GetTimeStep());
			}else if(m_fTireTemperature < 1.0f && m_fGasPedal > 0.75f){
				rearTraction *= m_fTireTemperature;
				ApplyTurnForce(contactPoints[BIKESUSP_R1], (1.0f-m_fTireTemperature)*-0.0007f*m_fTurnMass*m_fSteerAngle*GetRight()*CTimer::GetTimeStep());
			}

			if(fThrust > 0.0f && brake > 0.0f)
				brake = 0.0f;	// only affects next front wheel. is this intended?
			fThrust = acceleration;
			m_aWheelColPoints[rearLine].surfaceA = SURFACE_WHEELBASE;
			float adhesion = CSurfaceTable::GetAdhesiveLimit(m_aWheelColPoints[rearLine])*rearTraction;
			float adhesionDestab = 1.0f;
			if(m_fBrakeDestabilization > 0.0f)
				switch(CSurfaceTable::GetAdhesionGroup(m_aWheelColPoints[rearLine].surfaceB)){
				case ADHESIVE_HARD:
				case ADHESIVE_LOOSE:
					adhesionDestab = 0.9f;
					break;
				case ADHESIVE_ROAD:
					adhesionDestab = 0.7f;
					break;
				}
			if(GetStatus() == STATUS_PLAYER)
				adhesion *= CSurfaceTable::GetWetMultiplier(m_aWheelColPoints[rearLine].surfaceB);
			if(m_wheelStatus[BIKEWHEEL_REAR] == WHEEL_STATUS_BURST)
				adhesion *= 0.4f;
			WheelState[BIKEWHEEL_REAR] = m_aWheelState[BIKEWHEEL_REAR];
			CVector contactSpeed = GetSpeed(rearContact);
			ProcessBikeWheel(wheelFwd, wheelRight,
				contactSpeed, rearContact,
				2, fThrust,
				rearBrake*brakeBiasRear,
				adhesion*tractionBiasRear, adhesionDestab,
				BIKEWHEEL_REAR,
				&m_aWheelSpeed[BIKEWHEEL_REAR],
				&WheelState[BIKEWHEEL_REAR],
				BIKE_WHEELSPEC_1,
				m_wheelStatus[BIKEWHEEL_REAR]);
			if(bStuckInSand && (WheelState[BIKEWHEEL_REAR] == WHEEL_STATE_SPINNING || WheelState[BIKEWHEEL_REAR] == WHEEL_STATE_SKIDDING))
				WheelState[BIKEWHEEL_REAR] = WHEEL_STATE_NORMAL;
		}else{
			// Wheel in the air
			if(bIsHandbrakeOn)
				m_aWheelSpeed[BIKEWHEEL_REAR] = 0.0f;
			else{
				if(acceleration > 0.0f){
					if(m_aWheelSpeed[BIKEWHEEL_REAR] < 2.0f)
						m_aWheelSpeed[BIKEWHEEL_REAR] -= 0.2f;
				}else{
					if(m_aWheelSpeed[BIKEWHEEL_REAR] > -2.0f)
						m_aWheelSpeed[BIKEWHEEL_REAR] += 0.1f;
				}
			}
			m_aWheelRotation[BIKEWHEEL_REAR] += m_aWheelSpeed[BIKEWHEEL_REAR];
		}

		if(m_doingBurnout && m_aWheelState[BIKEWHEEL_REAR] == WHEEL_STATE_SPINNING){
			m_fTireTemperature += 0.001f*CTimer::GetTimeStep();
			if(m_fTireTemperature > 3.0f)
				m_fTireTemperature = 3.0f;
		}else if(m_fTireTemperature > 1.0f){
			m_fTireTemperature = (m_fTireTemperature - 1.0f)*Pow(0.995f, CTimer::GetTimeStep()) + 1.0f;
		}

		// Process front wheel - second try

		if(rearWheelsFirst){
			if(m_aWheelTimer[BIKESUSP_F1] > 0.0f || m_aWheelTimer[BIKESUSP_F2] > 0.0f){
				// Wheel on ground
				eBikeWheelSpecial spec;
				if(m_aWheelTimer[BIKESUSP_R1] > 0.0f || m_aWheelTimer[BIKESUSP_R2] > 0.0f)
					spec = BIKE_WHEELSPEC_0;
				else
					spec = BIKE_WHEELSPEC_2;
				CVector wheelFwd = GetMatrix() * CVector(-Sin(m_fWheelAngle), Cos(m_fWheelAngle), 0.0f);
				wheelFwd -= DotProduct(wheelFwd, m_aWheelColPoints[frontLine].normal)*m_aWheelColPoints[frontLine].normal;
				wheelFwd.Normalise();
				CVector wheelRight = CrossProduct(wheelFwd, m_aWheelColPoints[frontLine].normal);
				wheelRight.Normalise();

				fThrust = 0.0f;
				m_aWheelColPoints[frontLine].surfaceA = SURFACE_WHEELBASE;
				float adhesion = CSurfaceTable::GetAdhesiveLimit(m_aWheelColPoints[frontLine])*traction;
				float adhesionDestab = 1.0f;
				if(m_fBrakeDestabilization > 0.0f)
					switch(CSurfaceTable::GetAdhesionGroup(m_aWheelColPoints[frontLine].surfaceB)){
					case ADHESIVE_HARD:
					case ADHESIVE_LOOSE:
						adhesionDestab = 0.9f;
						break;
					case ADHESIVE_ROAD:
						adhesionDestab = 0.7f;
						break;
					}
				if(GetStatus() == STATUS_PLAYER)
					adhesion *= CSurfaceTable::GetWetMultiplier(m_aWheelColPoints[frontLine].surfaceB);
				if(m_wheelStatus[BIKEWHEEL_FRONT] == WHEEL_STATUS_BURST)
					adhesion *= 0.4f;
				WheelState[BIKEWHEEL_FRONT] = m_aWheelState[BIKEWHEEL_FRONT];
				CVector contactSpeed = GetSpeed(frontContact);
				ProcessBikeWheel(wheelFwd, wheelRight,
					contactSpeed, frontContact,
					2, fThrust,
					brake*brakeBiasFront,
					adhesion*tractionBiasFront, adhesionDestab,
					BIKEWHEEL_FRONT,
					&m_aWheelSpeed[BIKEWHEEL_FRONT],
					&WheelState[BIKEWHEEL_FRONT],
					spec,
					m_wheelStatus[BIKEWHEEL_FRONT]);
				if(bStuckInSand && (WheelState[BIKEWHEEL_FRONT] == WHEEL_STATE_SPINNING || WheelState[BIKEWHEEL_FRONT] == WHEEL_STATE_SKIDDING))
					WheelState[BIKEWHEEL_FRONT] = WHEEL_STATE_NORMAL;
			}else{
				// Wheel in the air
				m_aWheelSpeed[BIKEWHEEL_FRONT] *= 0.95f;
				m_aWheelRotation[BIKEWHEEL_FRONT] += m_aWheelSpeed[BIKEWHEEL_FRONT];
			}
		}

		// Process leaning
		float idleAngle = 0.0f;
		if(pDriver){
			CAnimBlendAssociation *assoc = RpAnimBlendClumpGetAssociation(pDriver->GetClump(), ANIM_BIKE_STILL);
			if(assoc)
				idleAngle = DEGTORAD(10.0f) * assoc->blendAmount;
		}
		if(bCanStand || m_bike_flag08){
			m_vecAvgSurfaceRight = CrossProduct(GetForward(), m_vecAvgSurfaceNormal);
			m_vecAvgSurfaceRight.Normalise();
			float lean;
			if(m_nWheelsOnGround == 0)
				lean = -m_fSteerAngle/DEGTORAD(pHandling->fSteeringLock)*0.5f*GRAVITY*CTimer::GetTimeStep();
			else
				lean = DotProduct(m_vecMoveSpeed-initialMoveSpeed, m_vecAvgSurfaceRight);
			lean /= GRAVITY*Max(CTimer::GetTimeStep(), 0.01f);
			if(m_wheelStatus[BIKEWHEEL_FRONT] == WHEEL_STATUS_BURST)
				lean = clamp(lean, -0.4f*pBikeHandling->fMaxLean, 0.4f*pBikeHandling->fMaxLean);
			else
				lean = clamp(lean, -pBikeHandling->fMaxLean, pBikeHandling->fMaxLean);
			float f = Pow(pBikeHandling->fDesLean, CTimer::GetTimeStep());
			m_fLeanLRAngle2 = (Asin(lean) - idleAngle)*(1.0f-f) + m_fLeanLRAngle2*f;
		}else{
			if(bIsStanding){
				float f = Pow(0.97f, CTimer::GetTimeStep());
				m_fLeanLRAngle2 = m_fLeanLRAngle2*f - (Asin(GetRight().z) + DEGTORAD(15.0f) + idleAngle)*(1.0f-f);
			}else{
				float f = Pow(0.95f, CTimer::GetTimeStep());
				m_fLeanLRAngle2 = m_fLeanLRAngle2*f;
			}
		}
		m_fLeanLRAngle = m_fLeanLRAngle2;

		// Destabilize steering when braking
		if((m_aSuspensionSpringRatio[BIKESUSP_F1] < 1.0f || m_aSuspensionSpringRatio[BIKESUSP_F2] < 1.0f) &&
		   m_fBrakePedal - m_fGasPedal > 0.9f &&
		   fwdSpeed > 0.02f &&
		   !bIsHandbrakeOn){
			m_fBrakeDestabilization += CGeneral::GetRandomNumberInRange(0.5f, 1.0f)*0.2f*CTimer::GetTimeStep();
			if(m_aSuspensionSpringRatio[BIKESUSP_R1] < 1.0f || m_aSuspensionSpringRatio[BIKESUSP_R2] < 1.0f){
				// BUG: this clamp makes no sense and the arguments seem swapped too
				ApplyTurnForce(contactPoints[BIKESUSP_R1],
					m_fTurnMass*Sin(m_fBrakeDestabilization)*clamp(fwdSpeed, 0.5f, 0.2f)*0.013f*GetRight()*CTimer::GetTimeStep());
			}else{
				// BUG: this clamp makes no sense and the arguments seem swapped too
				ApplyTurnForce(contactPoints[BIKESUSP_R1],
					m_fTurnMass*Sin(m_fBrakeDestabilization)*clamp(fwdSpeed, 0.5f, 0.2f)*0.003f*GetRight()*CTimer::GetTimeStep());
			}
		}else
			m_fBrakeDestabilization = 0.0f;

		// Update wheel positions from suspension
		float frontWheelPos = colModel->lines[frontLine].p0.z;
		if(m_aSuspensionSpringRatio[frontLine] > 0.0f)
			frontWheelPos -= m_aSuspensionSpringRatio[frontLine]*m_aSuspensionSpringLength[frontLine];
		m_aWheelPosition[BIKEWHEEL_FRONT] += (frontWheelPos - m_aWheelPosition[BIKEWHEEL_FRONT])*0.75f;

		float rearWheelPos = colModel->lines[rearLine].p0.z;
		if(m_aSuspensionSpringRatio[rearLine] > 0.0f)
			rearWheelPos -= m_aSuspensionSpringRatio[rearLine]*m_aSuspensionSpringLength[rearLine];
		m_aWheelPosition[BIKEWHEEL_REAR] += (rearWheelPos - m_aWheelPosition[BIKEWHEEL_REAR])*0.75f;

		for(i = 0; i < 2; i++)
			m_aWheelState[i] = WheelState[i];
		// never spin when moving backwards
		if(m_fGasPedal < 0.0f && m_aWheelState[BIKEWHEEL_REAR] == WHEEL_STATE_SPINNING)
			m_aWheelState[BIKEWHEEL_REAR] = WHEEL_STATE_NORMAL;

		// Process horn

		if(GetStatus() != STATUS_PLAYER){
#ifdef FIX_BUGS
			if(!IsAlarmOn())
#endif
				ReduceHornCounter();
		}else{
#ifdef FIX_BUGS
			if(!IsAlarmOn())
#endif
			{
				if(Pads[0].GetHorn())
					m_nCarHornTimer = 1;
				else
					m_nCarHornTimer = 0;
			}
		}
	}

	if(m_fHealth < 250.0f && GetStatus() != STATUS_WRECKED){
		// Car is on fire

		CVector damagePos, fireDir;

		// move fire forward if in first person
		if(this == FindPlayerVehicle() && TheCamera.GetLookingForwardFirstPerson()){
			damagePos = CVector(0.0f, 1.2f, -0.4f);
			fireDir = CVector(0.0f, 0.0f, CGeneral::GetRandomNumberInRange(0.01125f, 0.09f));
		}else{
			damagePos = ((CVehicleModelInfo*)CModelInfo::GetModelInfo(GetModelIndex()))->m_positions[CAR_POS_BACKSEAT];
			damagePos.z -= 0.3f;
			fireDir = CGeneral::GetRandomNumberInRange(0.02025f, 0.09f) * GetRight();
			fireDir -= CGeneral::GetRandomNumberInRange(0.02025f, 0.18f) * GetForward();
			fireDir.z = CGeneral::GetRandomNumberInRange(0.00225f, 0.018f);
		}

		damagePos = GetMatrix()*damagePos;
		CParticle::AddParticle(PARTICLE_CARFLAME, damagePos, fireDir,
			nil, 0.9f);

		CParticle::AddParticle(PARTICLE_ENGINE_SMOKE2, damagePos, CVector(0.0f, 0.0f, 0.0f), nil, 0.5f);

		damagePos.x += CGeneral::GetRandomNumberInRange(-0.5625f, 0.5625f),
		damagePos.y += CGeneral::GetRandomNumberInRange(-0.5625f, 0.5625f),
		damagePos.z += CGeneral::GetRandomNumberInRange(0.5625f, 2.25f);
		CParticle::AddParticle(PARTICLE_CARFLAME_SMOKE, damagePos, CVector(0.0f, 0.0f, 0.0f));

		// Blow up car after 5 seconds
		m_fFireBlowUpTimer += CTimer::GetTimeStepInMilliseconds();
		if(m_fFireBlowUpTimer > 5000.0f)
			BlowUpCar(m_pSetOnFireEntity);
	}else
		m_fFireBlowUpTimer = 0.0f;

	ProcessDelayedExplosion();

	// Find out how much to shake the pad depending on suspension and ground surface

	float suspShake = 0.0f;
	float surfShake = 0.0f;
	float speedsq = m_vecMoveSpeed.MagnitudeSqr();
	for(i = 0; i < 4; i++){
		float suspChange = m_aSuspensionSpringRatioPrev[i] - m_aSuspensionSpringRatio[i];
		if(suspChange > 0.3f && (i == BIKESUSP_F1 || i == BIKESUSP_R1) && speedsq > 0.04f){
			if(GetStatus() == STATUS_PLAYER || GetStatus() == STATUS_PHYSICS){
				if(m_wheelStatus[i] == WHEEL_STATUS_BURST)
					DMAudio.PlayOneShot(m_audioEntityId, SOUND_CAR_JUMP_2, suspChange);
				else
					DMAudio.PlayOneShot(m_audioEntityId, SOUND_CAR_JUMP, suspChange);
				if(suspChange > suspShake)
					suspShake = suspChange;
			}
		}

		if(this == FindPlayerVehicle()){
			uint8 surf = m_aWheelColPoints[i].surfaceB;
			if(surf == SURFACE_GRAVEL || surf == SURFACE_WATER || surf == SURFACE_HEDGE){
				if(surfShake < 0.2f)
					surfShake = 0.3f;
			}else if(surf == SURFACE_MUD_DRY || surf == SURFACE_SAND || surf == SURFACE_SAND_BEACH){
				if(surfShake < 0.1f)
					surfShake = 0.2f;
			}else if(surf == SURFACE_GRASS){
				if(surfShake < 0.05f)
					surfShake = 0.1f;
			}

// BUG: this only observes one of the wheels
			TheCamera.m_bVehicleSuspenHigh = Abs(suspChange) > 0.05f;
		}

		m_aSuspensionSpringRatioPrev[i] = m_aSuspensionSpringRatio[i];
		m_aSuspensionSpringRatio[i] = 1.0f;
	}

	// Shake pad

	if((suspShake > 0.0f || surfShake > 0.0f) && GetStatus() == STATUS_PLAYER){
		float speed = m_vecMoveSpeed.MagnitudeSqr();
		if(speed > sq(0.1f)){
			speed = Sqrt(speed);
			if(suspShake > 0.0f){
				uint8 freq = Min(200.0f*suspShake*speed*2000.0f/m_fMass + 100.0f, 250.0f);
				CPad::GetPad(0)->StartShake(20000.0f*CTimer::GetTimeStep()/freq, freq);
			}else{
				uint8 freq = Min(200.0f*surfShake*speed*2000.0f/m_fMass + 40.0f, 150.0f);
				CPad::GetPad(0)->StartShake(5000.0f*CTimer::GetTimeStep()/freq, freq);
			}
		}
	}

	bVehicleColProcessed = false;
	bAudioChangingGear = false;

	if(!bWarnedPeds)
		CCarCtrl::ScanForPedDanger(this);

	if(bInfiniteMass){
		m_vecMoveSpeed = CVector(0.0f, 0.0f, 0.0f);
		m_vecTurnSpeed = CVector(0.0f, 0.0f, 0.0f);
		m_vecMoveFriction = CVector(0.0f, 0.0f, 0.0f);
		m_vecTurnFriction = CVector(0.0f, 0.0f, 0.0f);
	}else if(!skipPhysics &&
	         (acceleration == 0.0f && brake == 0.0f || GetStatus() == STATUS_WRECKED)){
		if(Abs(m_vecMoveSpeed.x) < 0.005f &&
		   Abs(m_vecMoveSpeed.y) < 0.005f &&
		   Abs(m_vecMoveSpeed.z) < 0.005f){
			m_vecMoveSpeed = CVector(0.0f, 0.0f, 0.0f);
			m_vecTurnSpeed.z = 0.0f;
		}
	}

	// Balance bike
	if(bCanStand || m_bike_flag08 || bIsStanding){
		float onSideness = clamp(DotProduct(GetRight(), m_vecAvgSurfaceNormal), -1.0f, 1.0f);
		CVector worldCOM = Multiply3x3(GetMatrix(), m_vecCentreOfMass);
		// Keep bike upright
		if(bCanStand){
			ApplyTurnForce(-0.07f*onSideness*m_fTurnMass*GetUp()*CTimer::GetTimeStep(), worldCOM+GetRight());
			bIsStanding = false;
		}else
			ApplyTurnForce(-0.1f*onSideness*m_fTurnMass*GetUp()*CTimer::GetTimeStep(), worldCOM+GetRight());

		// Wheelie/Stoppie stabilization
		if(GetStatus() == STATUS_PLAYER){
			if(m_aWheelTimer[BIKESUSP_F1] == 0.0f && m_aWheelTimer[BIKESUSP_F2] == 0.0f && GetForward().z > 0.0 &&
			   !(m_aWheelTimer[BIKESUSP_R1] == 0.0f && m_aWheelTimer[BIKESUSP_R2] == 0.0f)){
				// Wheelie
				float wheelie = pBikeHandling->fWheelieAng - GetForward().z;
				if(wheelie > 0.15f)
					// below wheelie angle
					wheelie = Max(0.3f - wheelie, 0.0f);
				else if(wheelie < -0.08f)
					// above wheelie angle
					wheelie = Min(-0.15f - wheelie, 0.0f);
				float wheelieStab = pBikeHandling->fWheelieStabMult * Min(m_vecMoveSpeed.Magnitude(), 0.1f) * wheelie;
				ApplyTurnForce(0.5f*CTimer::GetTimeStep()*wheelieStab*m_fTurnMass*GetUp(), worldCOM+GetForward());
				ApplyTurnForce(0.5f*CTimer::GetTimeStep()*m_fWheelAngle*pBikeHandling->fWheelieSteer*m_fTurnMass*GetRight(), worldCOM+GetForward());
			}else if(m_aWheelTimer[BIKESUSP_R1] == 0.0f && m_aWheelTimer[BIKESUSP_R2] == 0.0f && GetForward().z < 0.0 &&
			         !(m_aWheelTimer[BIKESUSP_F1] == 0.0f && m_aWheelTimer[BIKESUSP_F2] == 0.0f)){
				// Stoppie
				float stoppie = pBikeHandling->fStoppieAng - GetForward().z;
				if(stoppie > 0.15f)
					// below stoppie angle
					stoppie = Max(0.3f - stoppie, 0.0f);
				else if(stoppie < -0.15f)
					// above stoppie angle
					stoppie = Min(-0.3f - stoppie, 0.0f);
				float speed = m_vecMoveSpeed.Magnitude();
				float stoppieStab = pBikeHandling->fStoppieStabMult * Min(speed, 0.1f) * stoppie;
				ApplyTurnForce(0.5f*CTimer::GetTimeStep()*stoppieStab*m_fTurnMass*GetUp(), worldCOM+GetForward());
				ApplyTurnForce(0.5f*Min(5.0f*speed,1.0f)*CTimer::GetTimeStep()*m_fWheelAngle*pBikeHandling->fWheelieSteer*m_fTurnMass*GetRight(), worldCOM+GetForward());
			}
		}
	}
}

void
CBike::Teleport(CVector pos)
{
	CWorld::Remove(this);

	SetPosition(pos);
	SetOrientation(0.0f, 0.0f, 0.0f);
	SetMoveSpeed(0.0f, 0.0f, 0.0f);
	SetTurnSpeed(0.0f, 0.0f, 0.0f);

	ResetSuspension();

	CWorld::Add(this);
}

void
CBike::PreRender(void)
{
// TODO: particles and lights and such

	CMatrix mat;
	CVector pos;
	CVehicleModelInfo *mi = (CVehicleModelInfo*)CModelInfo::GetModelInfo(GetModelIndex());
	CColModel *colModel = mi->GetColModel();

	// Wheel rotation
	CVector frontWheelFwd = Multiply3x3(GetMatrix(), CVector(-Sin(m_fSteerAngle), Cos(m_fSteerAngle), 0.0f));
	CVector rearWheelFwd = GetForward();
	if(m_aWheelTimer[BIKESUSP_F1] > 0.0f || m_aWheelTimer[BIKESUSP_F2] > 0.0f){
		float springRatio = Min(m_aSuspensionSpringRatioPrev[BIKESUSP_F1], m_aSuspensionSpringRatioPrev[BIKESUSP_F2]);
		CVector contactPoint(0.0f,
			(colModel->lines[BIKESUSP_F1].p0.y - colModel->lines[BIKESUSP_F2].p0.y)/2.0f,
			colModel->lines[BIKESUSP_F1].p0.z - m_aSuspensionSpringLength[BIKESUSP_F1]*springRatio - 0.5f*mi->m_wheelScale);
		CVector contactSpeed = GetSpeed(contactPoint);
		// Why is wheel state always normal?
		m_aWheelSpeed[BIKEWHEEL_FRONT] = ProcessWheelRotation(WHEEL_STATE_NORMAL, frontWheelFwd, contactSpeed, 0.5f*mi->m_wheelScale);
		m_aWheelRotation[BIKEWHEEL_FRONT] += m_aWheelSpeed[BIKEWHEEL_FRONT];
	}
	if(m_aWheelTimer[BIKESUSP_R1] > 0.0f || m_aWheelTimer[BIKESUSP_R2] > 0.0f){
		float springRatio = Min(m_aSuspensionSpringRatioPrev[BIKESUSP_R1], m_aSuspensionSpringRatioPrev[BIKESUSP_R2]);
		CVector contactPoint(0.0f,
			(colModel->lines[BIKESUSP_R1].p0.y - colModel->lines[BIKESUSP_R2].p0.y)/2.0f,
			colModel->lines[BIKESUSP_R1].p0.z - m_aSuspensionSpringLength[BIKESUSP_R1]*springRatio - 0.5f*mi->m_wheelScale);
		CVector contactSpeed = GetSpeed(contactPoint);
		m_aWheelSpeed[BIKEWHEEL_REAR] = ProcessWheelRotation(m_aWheelState[BIKEWHEEL_REAR], rearWheelFwd, contactSpeed, 0.5f*mi->m_wheelScale);
		m_aWheelRotation[BIKEWHEEL_REAR] += m_aWheelSpeed[BIKEWHEEL_REAR];
	}

	// Front fork
	if(m_aBikeNodes[BIKE_FORKS_FRONT]){
		mat.Attach(RwFrameGetMatrix(m_aBikeNodes[BIKE_FORKS_FRONT]));
		pos = mat.GetPosition();

		RwMatrix rwrot;
		// TODO: this looks like some weird ctor we don't have
		CMatrix rot;
		rot.m_attachment = &rwrot;
		rot.SetUnity();
		rot.UpdateRW();

		// Make rotation matrix with front fork as axis
		CVector forkAxis(0.0f, Sin(DEGTORAD(mi->m_bikeSteerAngle)), -Cos(DEGTORAD(mi->m_bikeSteerAngle)));
		forkAxis.Normalise();	// as if that's not already the case
		CQuaternion quat;
		quat.Set((RwV3d*)&forkAxis, -m_fWheelAngle);
		quat.Get(rot.m_attachment);
		rot.Update();

		// Transform fork
		mat.SetUnity();
		mat = mat * rot;
		mat.Translate(pos);
		mat.UpdateRW();

		if(m_aBikeNodes[BIKE_HANDLEBARS]){
			// Transform handle
			mat.Attach(RwFrameGetMatrix(m_aBikeNodes[BIKE_HANDLEBARS]));
			pos = mat.GetPosition();
			if(GetStatus() == STATUS_ABANDONED || GetStatus() == STATUS_WRECKED){
				mat.SetUnity();
				mat = mat * rot;
				mat.Translate(pos);
			}else
				mat.SetTranslate(mat.GetPosition());
			mat.UpdateRW();
		}
	}

	// Rear fork
	if(m_aBikeNodes[BIKE_FORKS_REAR]){
		float sine = (m_aWheelPosition[BIKEWHEEL_REAR] - m_aWheelBasePosition[BIKEWHEEL_REAR])/m_fRearForkLength;
		mat.Attach(RwFrameGetMatrix(m_aBikeNodes[BIKE_FORKS_REAR]));
		pos = mat.GetPosition();
		mat.SetRotate(-Asin(sine), 0.0f, 0.0f);
		mat.Translate(pos);
		mat.UpdateRW();
	}

	// Front wheel
	mat.Attach(RwFrameGetMatrix(m_aBikeNodes[BIKE_WHEEL_FRONT]));
	pos.x = mat.GetPosition().x;
	pos.z = m_aWheelPosition[BIKEWHEEL_FRONT] - m_fFrontForkZ;
	float y = (colModel->lines[BIKESUSP_F1].p0.y+colModel->lines[BIKESUSP_F2].p0.y)/2.0f - m_fFrontForkY;
	pos.y = y - (m_aWheelPosition[BIKEWHEEL_FRONT] - m_aWheelBasePosition[BIKEWHEEL_FRONT])*m_fFrontForkSlope;
	if(m_wheelStatus[BIKEWHEEL_FRONT] == WHEEL_STATUS_BURST)
		mat.SetRotate(m_aWheelRotation[BIKEWHEEL_FRONT], 0.0f, 0.05f*Sin(m_aWheelRotation[BIKEWHEEL_FRONT]));
	else
		mat.SetRotateX(m_aWheelRotation[BIKEWHEEL_FRONT]);
	mat.Translate(pos);
	mat.UpdateRW();
	// and mudguard
	mat.Attach(RwFrameGetMatrix(m_aBikeNodes[BIKE_MUDGUARD]));
	mat.SetTranslateOnly(pos);
	mat.UpdateRW();

	// Rear wheel
	mat.Attach(RwFrameGetMatrix(m_aBikeNodes[BIKE_WHEEL_REAR]));
	pos = mat.GetPosition();
	if(m_wheelStatus[BIKEWHEEL_REAR] == WHEEL_STATUS_BURST)
		mat.SetRotate(m_aWheelRotation[BIKEWHEEL_REAR], 0.0f, 0.07f*Sin(m_aWheelRotation[BIKEWHEEL_REAR]));
	else
		mat.SetRotateX(m_aWheelRotation[BIKEWHEEL_REAR]);
	mat.Translate(pos);
	mat.UpdateRW();

	// Chassis
	if(m_aBikeNodes[BIKE_CHASSIS]){
		mat.Attach(RwFrameGetMatrix(m_aBikeNodes[BIKE_CHASSIS]));
		pos = mat.GetPosition();
		pos.z = (1.0f - Cos(m_fLeanLRAngle)) * (0.9*colModel->boundingBox.min.z);
		mat.SetRotateX(-0.05f*Abs(m_fLeanLRAngle));
		mat.RotateY(m_fLeanLRAngle);
		mat.Translate(pos);
		mat.UpdateRW();
	}

// TODO: exhaust
}

void
CBike::Render(void)
{
	CVehicleModelInfo *mi = (CVehicleModelInfo*)CModelInfo::GetModelInfo(GetModelIndex());

	m_nSetPieceExtendedRangeTime = CTimer::GetTimeInMilliseconds() + 3000;
	mi->SetVehicleColour(m_currentColour1, m_currentColour2);
	CEntity::Render();
}

int32
CBike::ProcessEntityCollision(CEntity *ent, CColPoint *colpoints)
{
	int i;
	CColModel *colModel;

	if(GetStatus() != STATUS_SIMPLE)
		bVehicleColProcessed = true;

	colModel = GetColModel();

	int numWheelCollisions = 0;
	float prevRatios[4] = { 0.0f, 0.0f, 0.0f, 0.0f};
	for(i = 0; i < 4; i++)
		prevRatios[i] = m_aSuspensionSpringRatio[i];

	if(m_bIsVehicleBeingShifted || bSkipLineCol || ent->IsPed() ||
	   GetModelIndex() == MI_DODO && ent->IsVehicle())
		colModel->numLines = 0;

	int numCollisions = CCollision::ProcessColModels(GetMatrix(), *colModel,
		ent->GetMatrix(), *ent->GetColModel(),
		colpoints,
		m_aWheelColPoints, m_aSuspensionSpringRatio);

	// m_aSuspensionSpringRatio are now set to the point where the tyre touches ground.
	// In ProcessControl these will be re-normalized to ignore the tyre radius.
	if(colModel->numLines){
		for(i = 0; i < 4; i++){
			if(m_aSuspensionSpringRatio[i] < 1.0f && m_aSuspensionSpringRatio[i] < prevRatios[i]){
				numWheelCollisions++;

				// wheel is touching a physical
				if(ent->IsVehicle() || ent->IsObject()){
					CPhysical *phys = (CPhysical*)ent;

					m_aGroundPhysical[i] = phys;
					phys->RegisterReference((CEntity**)&m_aGroundPhysical[i]);
					m_aGroundOffset[i] = m_aWheelColPoints[i].point - phys->GetPosition();
				}

				m_nSurfaceTouched = m_aWheelColPoints[i].surfaceB;
				if(ent->IsBuilding())
					m_pCurGroundEntity = ent;
			}
		}
	}else
		colModel->numLines = 4;

	if(numCollisions > 0 || numWheelCollisions > 0){
		AddCollisionRecord(ent);
		if(!ent->IsBuilding())
			((CPhysical*)ent)->AddCollisionRecord(this);

		if(numCollisions > 0)
			if(ent->IsBuilding() ||
			   ent->IsObject() && ((CPhysical*)ent)->bInfiniteMass)
				bHasHitWall = true;
	}

	return numCollisions;
}

static int16 nLastControlInput;
static float fMouseCentreRange = 0.35f;
static float fMouseSteerSens = -0.0035f;
static float fMouseCentreMult = 0.975f;

void
CBike::ProcessControlInputs(uint8 pad)
{
	float speed = DotProduct(m_vecMoveSpeed, GetForward());

	if(CPad::GetPad(pad)->GetExitVehicle())
		bIsHandbrakeOn = true;
	else
		bIsHandbrakeOn = !!CPad::GetPad(pad)->GetHandBrake();

	// Steer left/right
#ifdef FIX_BUGS
	if(CCamera::m_bUseMouse3rdPerson && !CVehicle::m_bDisableMouseSteering){
		if(CPad::GetPad(pad)->GetMouseX() != 0.0f){
			m_fSteerInput += fMouseSteerSens*CPad::GetPad(pad)->GetMouseX();
			nLastControlInput = 2;
			if(Abs(m_fSteerInput) < fMouseCentreRange)
				m_fSteerInput *= Pow(fMouseCentreMult, CTimer::GetTimeStep());
		}else if(CPad::GetPad(pad)->GetSteeringLeftRight() || nLastControlInput != 2){
			// mouse hasn't move, steer with pad like below
			m_fSteerInput += (-CPad::GetPad(pad)->GetSteeringLeftRight()/128.0f - m_fSteerInput)*
				0.2f*CTimer::GetTimeStep();
			nLastControlInput = 0;
		}
	}else
#endif
	{
		m_fSteerInput += (-CPad::GetPad(pad)->GetSteeringLeftRight()/128.0f - m_fSteerInput)*
			0.2f*CTimer::GetTimeStep();
		nLastControlInput = 0;
	}
	m_fSteerInput = clamp(m_fSteerInput, -1.0f, 1.0f);

	// Lean forward/backward
	float updown = -CPad::GetPad(pad)->GetSteeringUpDown()/128.0f + CPad::GetPad(pad)->GetCarGunUpDown()/128.0f;
	m_fLeanInput += (updown - m_fLeanInput)*0.2f*CTimer::GetTimeStep();
	m_fLeanInput = clamp(m_fLeanInput, -1.0f, 1.0f);

	// Accelerate/Brake
	float acceleration = (CPad::GetPad(pad)->GetAccelerate() - CPad::GetPad(pad)->GetBrake())/255.0f;
	if(GetModelIndex() == MI_DODO && acceleration < 0.0f)
		acceleration *= 0.3f;
	if(Abs(speed) < 0.01f){
		// standing still, go into direction we want
		if(CPad::GetPad(pad)->GetAccelerate() > 150.0f && CPad::GetPad(pad)->GetBrake() > 150.0f){
			m_fGasPedal = CPad::GetPad(pad)->GetAccelerate()/255.0f;
			m_fBrakePedal = CPad::GetPad(pad)->GetBrake()/255.0f;
			m_doingBurnout = 1;
		}else{
			m_fGasPedal = acceleration;
			m_fBrakePedal = 0.0f;
		}
	}else{
#if 1
		// simpler than the code below
		if(speed * acceleration < 0.0f){
			// if opposite directions, have to brake first
			m_fGasPedal = 0.0f;
			m_fBrakePedal = Abs(acceleration);
		}else{
			// accelerating in same direction we were already going
			m_fGasPedal = acceleration;
			m_fBrakePedal = 0.0f;
		}
#else
		if(speed < 0.0f){
			// moving backwards currently
			if(acceleration < 0.0f){
				// still go backwards
				m_fGasPedal = acceleration;
				m_fBrakePedal = 0.0f;
			}else{
				// want to go forwards, so brake
				m_fGasPedal = 0.0f;
				m_fBrakePedal = acceleration;
			}
		}else{
			// moving forwards currently
			if(acceleration < 0.0f){
				// want to go backwards, so brake
				m_fGasPedal = 0.0f;
				m_fBrakePedal = -acceleration;
			}else{
				// still go forwards
				m_fGasPedal = acceleration;
				m_fBrakePedal = 0.0f;
			}
		}
#endif
	}

	// Actually turn wheels
	static float fValue;	// why static?
	if(m_fSteerInput < 0.0f)
		fValue = -sq(m_fSteerInput);
	else
		fValue = sq(m_fSteerInput);
	m_fSteerAngle = DEGTORAD(pHandling->fSteeringLock) * fValue;

	if(bComedyControls){
		if(((CTimer::GetTimeInMilliseconds() >> 10) & 0xF) < 12)
			m_fGasPedal = 1.0f;
		if((((CTimer::GetTimeInMilliseconds() >> 10)+6) & 0xF) < 12)
			m_fBrakePedal = 0.0f;
		bIsHandbrakeOn = false;
		if(CTimer::GetTimeInMilliseconds() & 0x800)
			m_fSteerAngle += 0.08f;
		else
			m_fSteerAngle -= 0.03f;
	}

	// Brake if player isn't in control
	// BUG: game always uses pad 0 here
	if(CPad::GetPad(pad)->ArePlayerControlsDisabled()){
		m_fBrakePedal = 1.0f;
		bIsHandbrakeOn = true;
		m_fGasPedal = 0.0f;

		FindPlayerPed()->KeepAreaAroundPlayerClear();

		// slow down car immediately
		speed = m_vecMoveSpeed.Magnitude();
		if(speed > 0.28f)
			m_vecMoveSpeed *= 0.28f/speed;
	}
}

void
CBike::ProcessBuoyancy(void)
{
	// TODO
}

void
CBike::DoDriveByShootings(void)
{
	// TODO
}

void
CBike::VehicleDamage(void)
{
	// TODO
}

void
CBike::GetComponentWorldPosition(int32 component, CVector &pos)
{
	if(m_aBikeNodes[component] == nil){
		printf("BikeNode missing: %d %d\n", GetModelIndex(), component);
		return;
	}
	RwMatrix *ltm = RwFrameGetLTM(m_aBikeNodes[component]);
	pos = *RwMatrixGetPos(ltm);
}

bool
CBike::IsComponentPresent(int32 component)
{
	return m_aBikeNodes[component] != nil;
}

void
CBike::SetComponentRotation(int32 component, CVector rotation)
{
	CMatrix mat(RwFrameGetMatrix(m_aBikeNodes[component]));
	CVector pos = mat.GetPosition();
	// BUG: all these set the whole matrix
	mat.SetRotateX(DEGTORAD(rotation.x));
	mat.SetRotateY(DEGTORAD(rotation.y));
	mat.SetRotateZ(DEGTORAD(rotation.z));
	mat.Translate(pos);
	mat.UpdateRW();
}

bool
CBike::IsDoorReady(eDoors door)
{
	return true;
}

bool
CBike::IsDoorFullyOpen(eDoors door)
{
	return false;
}

bool
CBike::IsDoorClosed(eDoors door)
{
	return false;
}

bool
CBike::IsDoorMissing(eDoors door)
{
	return true;
}

void
CBike::RemoveRefsToVehicle(CEntity *ent)
{
	int i;
	for(i = 0; i < 4; i++)
		if(m_aGroundPhysical[i] == ent)
			m_aGroundPhysical[i] = nil;
}

void
CBike::BlowUpCar(CEntity *culprit)
{
	if(!bCanBeDamaged)
		return;

// TODO: property damage stuff in FIX_BUGS

	// explosion pushes vehicle up
	m_vecMoveSpeed.z += 0.13f;
	SetStatus(STATUS_WRECKED);
	bRenderScorched = true;

	m_fHealth = 0.0f;
	m_nBombTimer = 0;

	TheCamera.CamShake(0.7f, GetPosition().x, GetPosition().y, GetPosition().z);

	KillPedsInVehicle();

	bEngineOn = false;
	bLightsOn = false;
	ChangeLawEnforcerState(false);

	CExplosion::AddExplosion(this, culprit, EXPLOSION_CAR, GetPosition(), 0);
	CDarkel::RegisterCarBlownUpByPlayer(this);
}

bool
CBike::SetUpWheelColModel(CColModel *colModel)
{
	// TODO, but unused
	return true;
}

void
CBike::BurstTyre(uint8 wheel, bool applyForces)
{
	if(bTyresDontBurst)
		return;

	switch(wheel){
	case CAR_PIECE_WHEEL_LF: wheel = BIKEWHEEL_FRONT; break;
	case CAR_PIECE_WHEEL_LR: wheel = BIKEWHEEL_REAR; break;
	}

	if(m_wheelStatus[wheel] == WHEEL_STATUS_OK){
		m_wheelStatus[wheel] = WHEEL_STATUS_BURST;
#ifdef FIX_BUGS
		CStats::TyresPopped++;
#endif
// TODO(MIAMI)
//		DMAudio.PlayOneShot(m_audioEntityId, SOUND_15, 0.0f);

		if(GetStatus() == STATUS_SIMPLE){
			SetStatus(STATUS_PHYSICS);
			CCarCtrl::SwitchVehicleToRealPhysics(this);
		}

		if(applyForces){
			ApplyMoveForce(GetRight() * m_fMass * CGeneral::GetRandomNumberInRange(-0.02f, 0.02f));
			ApplyTurnForce(GetRight() * m_fTurnMass * CGeneral::GetRandomNumberInRange(-0.02f, 0.02f), GetForward());
		}
// TODO: knock off driver
	}
}

bool
CBike::IsRoomForPedToLeaveCar(uint32 component, CVector *doorOffset)
{
	CColPoint colpoint;
	CEntity *ent;
	colpoint.point = CVector(0.0f, 0.0f, 0.0f);
	CVehicleModelInfo *mi = (CVehicleModelInfo*)CModelInfo::GetModelInfo(GetModelIndex());

	CVector seatPos = mi->GetFrontSeatPosn();
	if(component == CAR_DOOR_RR || component == CAR_DOOR_LR)
		seatPos = mi->m_positions[CAR_POS_BACKSEAT];
	if(component == CAR_DOOR_LF || component == CAR_DOOR_LR)
		seatPos.x = -seatPos.x;
	seatPos = GetMatrix() * seatPos;

	CVector doorPos = CPed::GetPositionToOpenCarDoor(this, component);
	if(doorOffset){
		CVector off = *doorOffset;
		if(component == CAR_DOOR_RF || component == CAR_DOOR_RR)
			off.x = -off.x;
		doorPos += Multiply3x3(GetMatrix(), off);
	}

	if(GetUp().z < 0.0f){
		seatPos.z += 0.5f;
		doorPos.z += 0.5f;
	}

	CVector dist = doorPos - seatPos;

	// Removing that makes thiProcessEntityCollisions func. return false for van doors.
	doorPos.z += 0.5f;
	float length = dist.Magnitude();
	CVector pedPos = seatPos + dist*((length+0.6f)/length);

	if(!CWorld::GetIsLineOfSightClear(seatPos, pedPos, true, false, false, true, false, false))
		return false;
	if(CWorld::TestSphereAgainstWorld(doorPos, 0.6f, this, true, true, false, true, false, false))
		return false;
	if(CWorld::ProcessVerticalLine(doorPos, 1000.0f, colpoint, ent, true, false, false, true, false, false, nil))
		if(colpoint.point.z > doorPos.z && colpoint.point.z < doorPos.z + 0.6f)
			return false;
	float upperZ = colpoint.point.z;
	if(!CWorld::ProcessVerticalLine(doorPos, -1000.0f, colpoint, ent, true, false, false, true, false, false, nil))
		return false;
	if(upperZ != 0.0f && upperZ < colpoint.point.z)
		return false;
	return true;
}

float
CBike::GetHeightAboveRoad(void)
{
	return m_fHeightAboveRoad;
}

void
CBike::PlayCarHorn(void)
{
	int r;

	if (IsAlarmOn() || m_nCarHornTimer != 0)
		return;

	if (m_nCarHornDelay) {
		m_nCarHornDelay--;
		return;
	}

	m_nCarHornDelay = (CGeneral::GetRandomNumber() & 0x7F) + 150;
	r = m_nCarHornDelay & 7;
	if(r < 2){
		m_nCarHornTimer = 45;
	}else if(r < 4){
		if(pDriver)
			pDriver->Say(SOUND_PED_CAR_COLLISION);
		m_nCarHornTimer = 45;
	}else{
		if(pDriver)
			pDriver->Say(SOUND_PED_CAR_COLLISION);
	}
}

void
CBike::PlayHornIfNecessary(void)
{
	if(AutoPilot.m_bSlowedDownBecauseOfPeds ||
	   AutoPilot.m_bSlowedDownBecauseOfCars)
		PlayCarHorn();
}

void
CBike::ResetSuspension(void)
{
	int i;
	for(i = 0; i < 2; i++){
		m_aWheelRotation[i] = 0.0f;
		m_aWheelState[i] = WHEEL_STATE_NORMAL;
	}
	for(i = 0; i < 4; i++){
		m_aSuspensionSpringRatio[i] = 1.0f;
		m_aWheelTimer[i] = 0.0f;
	}
}

// TODO: maybe put this somewhere else
inline void
GetRelativeMatrix(RwMatrix *mat, RwFrame *frm, RwFrame *end)
{
	*mat = *RwFrameGetMatrix(frm);
	frm = RwFrameGetParent(frm);
	while(frm){
		RwMatrixTransform(mat, RwFrameGetMatrix(frm), rwCOMBINEPOSTCONCAT);
		frm = RwFrameGetParent(frm);
		if(frm == end)
			frm = nil;
	}
}

void
CBike::SetupSuspensionLines(void)
{
	int i;
	CVector posn;
	float suspOffset = 0.0f;
	RwFrame *node = nil;
	CVehicleModelInfo *mi = (CVehicleModelInfo*)CModelInfo::GetModelInfo(GetModelIndex());
	CColModel *colModel = mi->GetColModel();
	RwMatrix *mat = RwMatrixCreate();

	bool initialized = colModel->lines[0].p0.z != FAKESUSPENSION;

	for(i = 0; i < 4; i++){
		if(initialized){
			posn = colModel->lines[i].p0;
			if(i < 2)
				posn.z = m_aWheelBasePosition[0];
			else
				posn.z = m_aWheelBasePosition[1];
		}else{
			switch(i){
			case BIKESUSP_F1:
				node = m_aBikeNodes[BIKE_WHEEL_FRONT];
				suspOffset = 0.25f*mi->m_wheelScale;
				break;
			case BIKESUSP_F2:
				node = m_aBikeNodes[BIKE_WHEEL_FRONT];
				suspOffset = -0.25f*mi->m_wheelScale;
				break;
			case BIKESUSP_R1:
				node = m_aBikeNodes[BIKE_WHEEL_REAR];
				suspOffset = 0.25f*mi->m_wheelScale;
				break;
			case BIKESUSP_R2:
				node = m_aBikeNodes[BIKE_WHEEL_REAR];
				suspOffset = -0.25f*mi->m_wheelScale;
				break;
			}

			GetRelativeMatrix(mat, node, node);
			posn = *RwMatrixGetPos(mat);
			if(i == BIKESUSP_F1)
				m_aWheelBasePosition[BIKEWHEEL_FRONT] = posn.z;
			else if(i == BIKESUSP_R1){
				m_aWheelBasePosition[BIKEWHEEL_REAR] = posn.z;

				GetRelativeMatrix(mat, m_aBikeNodes[BIKE_FORKS_REAR], m_aBikeNodes[BIKE_FORKS_REAR]);
				float dz = posn.z - RwMatrixGetPos(mat)->z;
				float dy = posn.y - RwMatrixGetPos(mat)->y;
				m_fRearForkLength = Sqrt(SQR(dy) + SQR(dz));
				assert(m_fRearForkLength != 0.0f);	// we want to divide by this
			}
			posn.y += suspOffset;
		}

		// uppermost wheel position
		posn.z += pHandling->fSuspensionUpperLimit;
		colModel->lines[i].p0 = posn;

		// lowermost wheel position
		posn.z += pHandling->fSuspensionLowerLimit - pHandling->fSuspensionUpperLimit;
		// lowest point on tyre
		posn.z -= mi->m_wheelScale*0.5f;
		colModel->lines[i].p1 = posn;

		// this is length of the spring at rest
		m_aSuspensionSpringLength[i] = pHandling->fSuspensionUpperLimit - pHandling->fSuspensionLowerLimit;
		m_aSuspensionLineLength[i] = colModel->lines[i].p0.z - colModel->lines[i].p1.z;
	}

	if(!initialized){
		GetRelativeMatrix(mat, m_aBikeNodes[BIKE_FORKS_FRONT], m_aBikeNodes[BIKE_FORKS_FRONT]);
		m_fFrontForkY = RwMatrixGetPos(mat)->y;
		m_fFrontForkZ = RwMatrixGetPos(mat)->z;
	}

	// Compress spring somewhat to get normal height on road
	m_fHeightAboveRoad = m_aSuspensionSpringLength[0]*(1.0f - 1.0f/(4.0f*pHandling->fSuspensionForceLevel))
			- colModel->lines[0].p0.z + mi->m_wheelScale*0.5f;
	for(i = 0; i < 2; i++)
		m_aWheelPosition[i] = mi->m_wheelScale*0.5f - m_fHeightAboveRoad;

	// adjust col model to include suspension lines
	if(colModel->boundingBox.min.z > colModel->lines[0].p1.z)
		colModel->boundingBox.min.z = colModel->lines[0].p1.z;
	float radius = Max(colModel->boundingBox.min.Magnitude(), colModel->boundingBox.max.Magnitude());
	if(colModel->boundingSphere.radius < radius)
		colModel->boundingSphere.radius = radius;

#ifdef FIX_BUGS
	RwMatrixDestroy(mat);
#endif
}

void
CBike::CalculateLeanMatrix(void)
{
	if(bLeanMatrixClean)
		return;

	CMatrix mat;
	mat.SetRotateX(-0.05f*Abs(m_fLeanLRAngle));
	mat.RotateY(m_fLeanLRAngle);
	m_leanMatrix = GetMatrix();
	m_leanMatrix = m_leanMatrix * mat;
	// place wheel back on ground
	m_leanMatrix.GetPosition() += GetUp()*(1.0f-Cos(m_fLeanLRAngle))*GetColModel()->boundingBox.min.z;
	bLeanMatrixClean = true;
}

void
CBike::GetCorrectedWorldDoorPosition(CVector &pos, CVector p1, CVector p2)
{
	CVector &fwd = GetForward();
	CVector rightWorld = CrossProduct(fwd, CVector(0.0f, 0.0f, 1.0f));
	CVector upWorld = CrossProduct(rightWorld, fwd);
	CColModel *colModel = GetColModel();
	float onSide = DotProduct(GetUp(), rightWorld);
	float diff = Max(colModel->boundingBox.max.z-colModel->boundingBox.max.x, 0.0f);
	pos = CVector(0.0f, 0.0f, 0.0f);
	float y = p2.y - p1.y;
	float x = onSide*diff + p2.x + p1.x;
	float z = p2.z - p1.z;
	pos = x*rightWorld + y*fwd + z*upWorld + GetPosition();
}

void
CBike::Fix(void)
{
	bIsDamaged = false;
	bIsOnFire = false;
	m_wheelStatus[0] = WHEEL_STATUS_OK;
	m_wheelStatus[1] = WHEEL_STATUS_OK;
}

void
CBike::SetupModelNodes(void)
{
	int i;
	for(i = 0; i < BIKE_NUM_NODES; i++)
		m_aBikeNodes[i] = nil;
	CClumpModelInfo::FillFrameArray(GetClump(), m_aBikeNodes);
}

void
CBike::ReduceHornCounter(void)
{
	if(m_nCarHornTimer != 0)
		m_nCarHornTimer--;
}
