#include "common.h"
#include "main.h"

#include "General.h"
#include "RwHelper.h"
#include "Pad.h"
#include "ModelIndices.h"
#include "VisibilityPlugins.h"
#include "DMAudio.h"
#include "Clock.h"
#include "Timecycle.h"
#include "ZoneCull.h"
#include "Camera.h"
#include "Darkel.h"
#include "Rubbish.h"
#include "Fire.h"
#include "Explosion.h"
#include "Particle.h"
#include "ParticleObject.h"
#include "Glass.h"
#include "Antennas.h"
#include "Skidmarks.h"
#include "WindModifiers.h"
#include "Shadows.h"
#include "PointLights.h"
#include "Coronas.h"
#include "SpecialFX.h"
#include "WaterCannon.h"
#include "WaterLevel.h"
#include "Floater.h"
#include "World.h"
#include "SurfaceTable.h"
#include "Weather.h"
#include "HandlingMgr.h"
#include "Record.h"
#include "Remote.h"
#include "Population.h"
#include "CarCtrl.h"
#include "CarAI.h"
#include "Stats.h"
#include "Garages.h"
#include "PathFind.h"
#include "Replay.h"
#include "AnimManager.h"
#include "RpAnimBlend.h"
#include "AnimBlendAssociation.h"
#include "Ped.h"
#include "PlayerPed.h"
#include "Object.h"
#include "Automobile.h"

//--MIAMI: file done except TODOs

bool bAllCarCheat;

RwObject *GetCurrentAtomicObjectCB(RwObject *object, void *data);

bool CAutomobile::m_sAllTaxiLights;

const uint32 CAutomobile::nSaveStructSize =
#ifdef COMPATIBLE_SAVES
	1448;
#else
	sizeof(CAutomobile);
#endif

CAutomobile::CAutomobile(int32 id, uint8 CreatedBy)
 : CVehicle(CreatedBy)
{
	int i;

	m_vehType = VEHICLE_TYPE_CAR;

	CVehicleModelInfo *mi = (CVehicleModelInfo*)CModelInfo::GetModelInfo(id);
	m_fFireBlowUpTimer = 0.0f;
	m_doingBurnout = 0;
	bTaxiLight = m_sAllTaxiLights;
	bFixedColour = false;
	bBigWheels = false;
	bWaterTight = false;

	SetModelIndex(id);

	// Already done in CVehicle...
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
	}

	pHandling = mod_HandlingManager.GetHandlingData((eHandlingId)mi->m_handlingId);
	pFlyingHandling = mod_HandlingManager.GetFlyingPointer((eHandlingId)mi->m_handlingId);

	m_auto_unused1 = 20.0f;
	m_auto_unused2 = 0;

	mi->ChooseVehicleColour(m_currentColour1, m_currentColour2);

	bIsVan = !!(pHandling->Flags & HANDLING_IS_VAN);
	bIsBig = !!(pHandling->Flags & HANDLING_IS_BIG);
	bIsBus = !!(pHandling->Flags & HANDLING_IS_BUS);
	bLowVehicle = !!(pHandling->Flags & HANDLING_IS_LOW);

	// Doors
	if(bIsBus){
		Doors[DOOR_FRONT_LEFT].Init(-HALFPI, 0.0f, 0, 2);
		Doors[DOOR_FRONT_RIGHT].Init(0.0f, HALFPI, 1, 2);
	}else{
		Doors[DOOR_FRONT_LEFT].Init(-PI*0.4f, 0.0f, 0, 2);
		Doors[DOOR_FRONT_RIGHT].Init(0.0f, PI*0.4f, 1, 2);
	}
	if(bIsVan){
		Doors[DOOR_REAR_LEFT].Init(-HALFPI, 0.0f, 1, 2);
		Doors[DOOR_REAR_RIGHT].Init(0.0f, HALFPI, 0, 2);
	}else{
		Doors[DOOR_REAR_LEFT].Init(-PI*0.4f, 0.0f, 0, 2);
		Doors[DOOR_REAR_RIGHT].Init(0.0f, PI*0.4f, 1, 2);
	}
	if(pHandling->Flags & HANDLING_REV_BONNET)
		Doors[DOOR_BONNET].Init(-PI*0.3f, 0.0f, 1, 0);
	else
		Doors[DOOR_BONNET].Init(0.0f, PI*0.3f, 1, 0);
	if(pHandling->Flags & HANDLING_HANGING_BOOT)
		Doors[DOOR_BOOT].Init(-PI*0.4f, 0.0f, 0, 0);
	else if(pHandling->Flags & HANDLING_TAILGATE_BOOT)
		Doors[DOOR_BOOT].Init(0.0, HALFPI, 1, 0);
	else
		Doors[DOOR_BOOT].Init(-PI*0.3f, 0.0f, 1, 0);
	if(pHandling->Flags & HANDLING_NO_DOORS){
		Damage.SetDoorStatus(DOOR_FRONT_LEFT, DOOR_STATUS_MISSING);
		Damage.SetDoorStatus(DOOR_FRONT_RIGHT, DOOR_STATUS_MISSING);
		Damage.SetDoorStatus(DOOR_REAR_LEFT, DOOR_STATUS_MISSING);
		Damage.SetDoorStatus(DOOR_REAR_RIGHT, DOOR_STATUS_MISSING);
	}

	for(i = 0; i < 6; i++)
		m_randomValues[i] = CGeneral::GetRandomNumberInRange(-0.15f, 0.15f);

	m_fMass = pHandling->fMass;
	m_fTurnMass = pHandling->fTurnMass;
	m_vecCentreOfMass = pHandling->CentreOfMass;
	m_fAirResistance = pHandling->Dimension.x*pHandling->Dimension.z/m_fMass;
	m_fElasticity = 0.05f;
	m_fBuoyancy = pHandling->fBuoyancy;

	m_fOrientation = m_auto_unk4 = 0.0f;

	m_nBusDoorTimerEnd = 0;
	m_nBusDoorTimerStart = 0;

	m_fSteerAngle = 0.0f;
	m_fGasPedal = 0.0f;
	m_fBrakePedal = 0.0f;
	m_pSetOnFireEntity = nil;
	m_fGasPedalAudio = 0.0f;
	bNotDamagedUpsideDown = false;
	bMoreResistantToDamage = false;
	bTankDetonateCars = true;
	bStuckInSand = false;
	bHeliDestroyed = false;
	m_fVelocityChangeForAudio = 0.0f;
	m_hydraulicState = 0;

	for(i = 0; i < 4; i++){
		m_aGroundPhysical[i] = nil;
		m_aGroundOffset[i] = CVector(0.0f, 0.0f, 0.0f);
		m_aSuspensionSpringRatioPrev[i] = m_aSuspensionSpringRatio[i] = 1.0f;
		m_aWheelTimer[i] = 0.0f;
		m_aWheelRotation[i] = 0.0f;
		m_aWheelSpeed[i] = 0.0f;
		m_aWheelState[i] = WHEEL_STATE_NORMAL;
		m_aWheelSkidmarkType[i] = SKIDMARK_NORMAL;
		m_aWheelSkidmarkBloody[i] = false;
	}

	m_nWheelsOnGround = 0;
	m_nDriveWheelsOnGround = 0;
	m_nDriveWheelsOnGroundPrev = 0;
	m_fHeightAboveRoad = 0.0f;
	m_fTraction = 1.0f;
	m_fTireTemperature = 1.0f;

	CColModel *colModel = mi->GetColModel();
	if(colModel->lines == nil){
		colModel->lines = (CColLine*)RwMalloc(4*sizeof(CColLine));
		colModel->numLines = 4;
	}

	SetupSuspensionLines();

	SetStatus(STATUS_SIMPLE);
	bUseCollisionRecords = true;

	m_nNumPassengers = 0;

	if(m_nDoorLock == CARLOCK_UNLOCKED &&
	   (id == MI_POLICE || id == MI_ENFORCER || id == MI_RHINO))
		m_nDoorLock = CARLOCK_LOCKED_INITIALLY;

	m_fCarGunLR = 0.0f;
	m_fCarGunUD = 0.05f;
	m_fPropellerRotation = 0.0f;
	m_fHeliOrientation = -1.0f;
	m_weaponDoorTimerLeft = 0.0f;
	m_weaponDoorTimerRight = m_weaponDoorTimerLeft;

	if(GetModelIndex() == MI_DODO){
		RpAtomicSetFlags((RpAtomic*)GetFirstObject(m_aCarNodes[CAR_WHEEL_LF]), 0);
		CMatrix mat1;
		mat1.Attach(RwFrameGetMatrix(m_aCarNodes[CAR_WHEEL_RF]));
		CMatrix mat2(RwFrameGetMatrix(m_aCarNodes[CAR_WHEEL_LF]));
		mat1.GetPosition() += CVector(mat2.GetPosition().x + 0.1f, 0.0f, mat2.GetPosition().z);
		mat1.UpdateRW();
	}else if(GetModelIndex() == MI_HUNTER){
		RpAtomicSetFlags((RpAtomic*)GetFirstObject(m_aCarNodes[CAR_WHEEL_LB]), 0);
		RpAtomicSetFlags((RpAtomic*)GetFirstObject(m_aCarNodes[CAR_WHEEL_RB]), 0);
	}else if(IsRealHeli()){
		RpAtomicSetFlags((RpAtomic*)GetFirstObject(m_aCarNodes[CAR_WHEEL_LF]), 0);
		RpAtomicSetFlags((RpAtomic*)GetFirstObject(m_aCarNodes[CAR_WHEEL_RF]), 0);
		RpAtomicSetFlags((RpAtomic*)GetFirstObject(m_aCarNodes[CAR_WHEEL_LB]), 0);
		RpAtomicSetFlags((RpAtomic*)GetFirstObject(m_aCarNodes[CAR_WHEEL_RB]), 0);
	}else if(GetModelIndex() == MI_RHINO){
		bExplosionProof = true;
		bBulletProof = true;
	}
}

void
CAutomobile::SetModelIndex(uint32 id)
{
	CVehicle::SetModelIndex(id);
	SetupModelNodes();
}

#define SAND_SLOWDOWN (0.01f)
float CAR_BALANCE_MULT = 0.3f;
CVector vecSeaSparrowGunPos(-0.5f, 2.4f, -0.785f);
CVector vecHunterGunPos(0.0f, 4.8f, -1.3f);
CVector vecHunterRocketPos(2.5f, 1.0f, -0.5f);
CVector vecDAMAGE_ENGINE_POS_SMALL(-0.1f, -0.1f, 0.0f);
CVector vecDAMAGE_ENGINE_POS_BIG(-0.5f, -0.3f, 0.0f);

void
CAutomobile::ProcessControl(void)
{
	int i;
	float wheelRot;
	CColModel *colModel;

	if(bUsingSpecialColModel)
		colModel = &CWorld::Players[CWorld::PlayerInFocus].m_ColModel;
	else
		colModel = GetColModel();
	bool drivingInSand = false;
	bWarnedPeds = false;
	m_doingBurnout = 0;
	bStuckInSand = false;
	bRestingOnPhysical = false;

	bool carHasNitro = bAllTaxisHaveNitro && GetStatus() == STATUS_PLAYER && IsTaxi();

	if(CReplay::IsPlayingBack())
		return;

	UpdatePassengerList();

	// Heli wind
	if(IsRealHeli())
		if((GetStatus() == STATUS_PLAYER || GetStatus() == STATUS_PHYSICS) && m_aWheelSpeed[1] > 0.075f ||
		   GetStatus() == STATUS_SIMPLE)
			CWindModifiers::RegisterOne(GetPosition(), 1);

	// Improve grip of vehicles in certain cases
	bool strongGrip1 = false;
	bool strongGrip2 = false;
	if(FindPlayerVehicle() && this != FindPlayerVehicle() &&
	   (AutoPilot.m_nCarMission == MISSION_RAMPLAYER_FARAWAY || AutoPilot.m_nCarMission == MISSION_RAMPLAYER_CLOSE ||
	    AutoPilot.m_nCarMission == MISSION_BLOCKPLAYER_FARAWAY || AutoPilot.m_nCarMission == MISSION_BLOCKPLAYER_CLOSE)){
		if(FindPlayerSpeed().Magnitude() > 0.3f){
			strongGrip1 = true;
			if(FindPlayerSpeed().Magnitude() > 0.4f &&
			   m_vecMoveSpeed.Magnitude() < 0.3f)
				strongGrip2 = true;
			else if((GetPosition() - FindPlayerCoors()).Magnitude() > 50.0f)
				strongGrip2 = true;
		}
	}else if(GetModelIndex() == MI_RCBANDIT && GetStatus() != STATUS_PLAYER_REMOTE)
		strongGrip1 = true;

	if(bIsBus)
		ProcessAutoBusDoors();

	ProcessCarAlarm();

	// Scan if this car sees the player committing any crimes
	if(GetStatus() != STATUS_ABANDONED && GetStatus() != STATUS_WRECKED &&
	   GetStatus() != STATUS_PLAYER && GetStatus() != STATUS_PLAYER_REMOTE && GetStatus() != STATUS_PLAYER_DISABLED){
		switch(GetModelIndex())
		case MI_FBIRANCH:
		case MI_POLICE:
		case MI_ENFORCER:
		case MI_SECURICA:
		case MI_RHINO:
		case MI_BARRACKS:
			ScanForCrimes();
	}

	// Process driver
	if(pDriver)
		if(IsUpsideDown() && CanPedEnterCar()){
			if(!pDriver->IsPlayer() &&
			   !(pDriver->m_leader && pDriver->m_leader->bInVehicle) &&
			   pDriver->CharCreatedBy != MISSION_CHAR)
				pDriver->SetObjective(OBJECTIVE_LEAVE_VEHICLE, this);
		}

	ActivateBombWhenEntered();

	// Process passengers
	if(m_nNumPassengers != 0 && IsUpsideDown() && CanPedEnterCar()){
		for(i = 0; i < m_nNumMaxPassengers; i++)
			if(pPassengers[i])
				if(!pPassengers[i]->IsPlayer() &&
				   !(pPassengers[i]->m_leader && pPassengers[i]->m_leader->bInVehicle) &&
				   pPassengers[i]->CharCreatedBy != MISSION_CHAR)
					pPassengers[i]->SetObjective(OBJECTIVE_LEAVE_VEHICLE, this);
	}

	CRubbish::StirUp(this);

	UpdateClumpAlpha();

	AutoPilot.m_bSlowedDownBecauseOfCars = false;
	AutoPilot.m_bSlowedDownBecauseOfPeds = false;

	// Set Center of Mass to make car more stable
	if(strongGrip1 || bCheat3)
		m_vecCentreOfMass.z = 0.3f*m_aSuspensionSpringLength[0] + -1.0f*m_fHeightAboveRoad;
	else if(pHandling->Flags & HANDLING_NONPLAYER_STABILISER && GetStatus() == STATUS_PHYSICS)
		m_vecCentreOfMass.z = pHandling->CentreOfMass.z - 0.2f*pHandling->Dimension.z;
	else
		m_vecCentreOfMass = pHandling->CentreOfMass;

	// Park car
	if(bCanPark && !bParking && VehicleCreatedBy != MISSION_VEHICLE && AutoPilot.m_nCarMission == MISSION_CRUISE &&
	   ((CTimer::GetFrameCounter() + m_randomSeed)&0xF) == 0 && !IsTaxi()){
		CVector parkPosition = GetPosition() + 3.0f*GetRight() + 10.0f*GetForward();
		CEntity *ent = nil;
		CColPoint colpoint;
		if(!CWorld::ProcessLineOfSight(GetPosition(), parkPosition, colpoint, ent, true, true, true, false, false, false) ||
		   ent == this)
			CCarAI::GetCarToParkAtCoors(this, &parkPosition);
	}

	// Process depending on status

	bool playerRemote = false;
	switch(GetStatus()){
	case STATUS_PLAYER_REMOTE:
		if(CPad::GetPad(0)->WeaponJustDown() && !bDisableRemoteDetonation){
			BlowUpCar(FindPlayerPed());
			CRemote::TakeRemoteControlledCarFromPlayer();
		}

		if(GetModelIndex() == MI_RCBANDIT && !bDisableRemoteDetonationOnContact){
			//CVector pos = GetPosition();
			// FindPlayerCoors unused
			if(RcbanditCheckHitWheels() || bIsInWater){
				CRemote::TakeRemoteControlledCarFromPlayer();
				BlowUpCar(FindPlayerPed());
			}
		}

		if(CWorld::Players[CWorld::PlayerInFocus].m_pRemoteVehicle == this)
			playerRemote = true;
		// fall through
	case STATUS_PLAYER:
		if(playerRemote ||
		   pDriver && pDriver->GetPedState() != PED_EXIT_CAR && pDriver->GetPedState() != PED_DRAG_FROM_CAR && pDriver->GetPedState() != PED_ARRESTED){
			// process control input if controlled by player
			if(playerRemote || pDriver->m_nPedType == PEDTYPE_PLAYER1)
				ProcessControlInputs(0);

			PruneReferences();

			if(GetStatus() == STATUS_PLAYER && !CRecordDataForChase::IsRecording())
				DoDriveByShootings();

			// Tweak center on mass when driving on two wheels
			int twoWheelTime = CWorld::Players[CWorld::PlayerInFocus].m_nTimeNotFullyOnGround;
			if(twoWheelTime > 500 && !IsRealHeli() && !IsRealPlane()){
				float tweak = Min(twoWheelTime-500, 1000)/500.0f;
				if(GetUp().z > 0.0f){
					// positive when on left wheels, negative on right wheels
					if(GetRight().z <= 0.0f)
						tweak *= -1.0f;
					m_vecCentreOfMass.z = pHandling->CentreOfMass.z +
						CPad::GetPad(0)->GetSteeringLeftRight()/128.0f *
						CAR_BALANCE_MULT * tweak * colModel->boundingBox.max.z;
				}
			}else
				m_vecCentreOfMass.z = pHandling->CentreOfMass.z;

			if(bHoverCheat)
				DoHoverSuspensionRatios();

			if(m_aSuspensionSpringRatio[0] < 1.0f && CSurfaceTable::GetAdhesionGroup(m_aWheelColPoints[0].surfaceB) == ADHESIVE_SAND ||
			   m_aSuspensionSpringRatio[1] < 1.0f && CSurfaceTable::GetAdhesionGroup(m_aWheelColPoints[1].surfaceB) == ADHESIVE_SAND ||
			   m_aSuspensionSpringRatio[2] < 1.0f && CSurfaceTable::GetAdhesionGroup(m_aWheelColPoints[2].surfaceB) == ADHESIVE_SAND ||
			   m_aSuspensionSpringRatio[3] < 1.0f && CSurfaceTable::GetAdhesionGroup(m_aWheelColPoints[3].surfaceB) == ADHESIVE_SAND){
				if(GetModelIndex() != MI_RCBANDIT && GetModelIndex() != MI_RHINO){
					float slowdown;
					CVector parallelSpeed = m_vecMoveSpeed - DotProduct(m_vecMoveSpeed, GetUp())*GetUp();
					float fSpeed = parallelSpeed.MagnitudeSqr();
					if(fSpeed > SQR(0.3f)){
						fSpeed = Sqrt(fSpeed);
						parallelSpeed *= 0.3f / fSpeed;
						slowdown = SAND_SLOWDOWN * Max(1.0f - 2.0f*fSpeed, 0.2f);
					}else{
						bStuckInSand = true;
						slowdown = SAND_SLOWDOWN;
					}
					if(pHandling->Flags & HANDLING_GOOD_INSAND)
						slowdown *= 0.5f;
					if(CWeather::WetRoads > 0.2f)
						slowdown *= (1.2f - CWeather::WetRoads);
					ApplyMoveForce(parallelSpeed * -CTimer::GetTimeStep()*slowdown*m_fMass);
					drivingInSand = true;
				}
			}
		}else if(pDriver && pDriver->IsPlayer() &&
		         (pDriver->GetPedState() == PED_ARRESTED ||
		          pDriver->GetPedState() == PED_DRAG_FROM_CAR ||
		          (pDriver->GetPedState() == PED_EXIT_CAR || pDriver->m_objective == OBJECTIVE_LEAVE_VEHICLE) && !CanPedJumpOutCar())){
			bIsHandbrakeOn = true;
			m_fBrakePedal = 1.0f;
			m_fGasPedal = 0.0f;
		}
		if(CPad::GetPad(0)->WeaponJustDown())
			ActivateBomb();
		break;

	case STATUS_SIMPLE:
		CCarAI::UpdateCarAI(this);
		CPhysical::ProcessControl();
		CCarCtrl::UpdateCarOnRails(this);

		m_nWheelsOnGround = 4;
		m_nDriveWheelsOnGroundPrev = m_nDriveWheelsOnGround;
		m_nDriveWheelsOnGround = 4;

		pHandling->Transmission.CalculateGearForSimpleCar(AutoPilot.m_fMaxTrafficSpeed/50.0f, m_nCurrentGear);

		wheelRot = ProcessWheelRotation(WHEEL_STATE_NORMAL, GetForward(), m_vecMoveSpeed, 0.35f);
		for(i = 0; i < 4; i++)
			m_aWheelRotation[i] += wheelRot;

		PlayHornIfNecessary();
		ReduceHornCounter();
		bVehicleColProcessed = false;
		bAudioChangingGear = false;
		// that's all we do for simple vehicles
		return;

	case STATUS_PHYSICS:
		CCarAI::UpdateCarAI(this);
		CCarCtrl::SteerAICarWithPhysics(this);
		PlayHornIfNecessary();

		if(bIsBeingCarJacked){
			m_fGasPedal = 0.0f;
			m_fBrakePedal = 1.0f;
			bIsHandbrakeOn = true;
		}

		if(m_aSuspensionSpringRatio[0] < 1.0f && CSurfaceTable::GetAdhesionGroup(m_aWheelColPoints[0].surfaceB) == ADHESIVE_SAND ||
		   m_aSuspensionSpringRatio[1] < 1.0f && CSurfaceTable::GetAdhesionGroup(m_aWheelColPoints[1].surfaceB) == ADHESIVE_SAND ||
		   m_aSuspensionSpringRatio[2] < 1.0f && CSurfaceTable::GetAdhesionGroup(m_aWheelColPoints[2].surfaceB) == ADHESIVE_SAND ||
		   m_aSuspensionSpringRatio[3] < 1.0f && CSurfaceTable::GetAdhesionGroup(m_aWheelColPoints[3].surfaceB) == ADHESIVE_SAND){
			if(GetModelIndex() != MI_RCBANDIT && GetModelIndex() != MI_SANDKING && GetModelIndex() != MI_BFINJECT){
				bStuckInSand = true;
				if(CWeather::WetRoads > 0.0f)
					ApplyMoveForce(m_vecMoveSpeed * -CTimer::GetTimeStep()*SAND_SLOWDOWN*m_fMass * (1.0f-CWeather::WetRoads));
				else
					ApplyMoveForce(m_vecMoveSpeed * -CTimer::GetTimeStep()*SAND_SLOWDOWN*m_fMass);
			}
		}
		break;

	case STATUS_ABANDONED:
		if(m_vecMoveSpeed.MagnitudeSqr() < SQR(0.1f))
			m_fBrakePedal = 0.2f;
		else
			m_fBrakePedal = 0.0f;
		bIsHandbrakeOn = false;

		m_fSteerAngle = 0.0f;
		m_fGasPedal = 0.0f;
		if(!IsAlarmOn())
			m_nCarHornTimer = 0;

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
		if(!IsAlarmOn())
			m_nCarHornTimer = 0;
		break;

	case STATUS_PLAYER_DISABLED:
		if(m_vecMoveSpeed.MagnitudeSqr() < SQR(0.1f) ||
		   (pDriver && pDriver->IsPlayer() &&
		    (pDriver->GetPedState() == PED_ARRESTED ||
		     pDriver->GetPedState() == PED_DRAG_FROM_CAR ||
		     (pDriver->GetPedState() == PED_EXIT_CAR || pDriver->m_objective == OBJECTIVE_LEAVE_VEHICLE) && !CanPedJumpOutCar()))){
			bIsHandbrakeOn = true;
			m_fBrakePedal = 1.0f;
			m_fGasPedal = 0.0f;
		}else{
			m_fBrakePedal = 0.0f;
			bIsHandbrakeOn = false;
		}

		m_fSteerAngle = 0.0f;
		m_fGasPedal = 0.0f;
		if(!IsAlarmOn())
			m_nCarHornTimer = 0;
		break;
	default: break;
	}

	// Skip physics if object is found to have been static recently
	bool skipPhysics = false;
	if(!bIsStuck && (GetStatus() == STATUS_ABANDONED || GetStatus() == STATUS_WRECKED)){
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
		   !(m_fDamageImpulse > 0.0f && m_pDamageEntity && m_pDamageEntity->IsPed()) ||
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
		if(IsRealHeli() && m_aWheelSpeed[1] > 0.0f){
			skipPhysics = false;
			m_nStaticFrames = 0;
		}
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

	VehicleDamage(0.0f, 0);

	// special control
	switch(GetModelIndex()){
	case MI_FIRETRUCK:
		FireTruckControl();
		break;
	case MI_RHINO:
		TankControl();
		BlowUpCarsInPath();
		break;
	case MI_VOODOO:
		HydraulicControl();
		break;
	default:
		if(CVehicle::bCheat3 || carHasNitro){
			// Make vehicle jump when horn is sounded
			if(GetStatus() == STATUS_PLAYER && m_vecMoveSpeed.MagnitudeSqr() > sq(0.2f) &&
			// BUG: game checks [0] four times, instead of all wheels
			   m_aSuspensionSpringRatio[0] < 1.0f &&
			   CPad::GetPad(0)->HornJustDown()){

				DMAudio.PlayOneShot(m_audioEntityId, SOUND_CAR_HYDRAULIC_1, 0.0f);
				DMAudio.PlayOneShot(m_audioEntityId, SOUND_CAR_JUMP, 1.0f);

				CParticle::AddParticle(PARTICLE_ENGINE_STEAM,
					m_aWheelColPoints[0].point + 0.5f*GetUp(),
					1.3f*m_vecMoveSpeed, nil, 2.5f);
				CParticle::AddParticle(PARTICLE_ENGINE_SMOKE,
					m_aWheelColPoints[0].point + 0.5f*GetUp(),
					1.2f*m_vecMoveSpeed, nil, 2.0f);

				CParticle::AddParticle(PARTICLE_ENGINE_STEAM,
					m_aWheelColPoints[2].point + 0.5f*GetUp(),
					1.3f*m_vecMoveSpeed, nil, 2.5f);
				CParticle::AddParticle(PARTICLE_ENGINE_SMOKE,
					m_aWheelColPoints[2].point + 0.5f*GetUp(),
					1.2f*m_vecMoveSpeed, nil, 2.0f);

				CParticle::AddParticle(PARTICLE_ENGINE_STEAM,
					m_aWheelColPoints[0].point + 0.5f*GetUp() - GetForward(),
					1.3f*m_vecMoveSpeed, nil, 2.5f);
				CParticle::AddParticle(PARTICLE_ENGINE_SMOKE,
					m_aWheelColPoints[0].point + 0.5f*GetUp() - GetForward(),
					1.2f*m_vecMoveSpeed, nil, 2.0f);

				CParticle::AddParticle(PARTICLE_ENGINE_STEAM,
					m_aWheelColPoints[2].point + 0.5f*GetUp() - GetForward(),
					1.3f*m_vecMoveSpeed, nil, 2.5f);
				CParticle::AddParticle(PARTICLE_ENGINE_SMOKE,
					m_aWheelColPoints[2].point + 0.5f*GetUp() - GetForward(),
					1.2f*m_vecMoveSpeed, nil, 2.0f);

				ApplyMoveForce(CVector(0.0f, 0.0f, 1.0f)*m_fMass*0.4f);
				ApplyTurnForce(GetUp()*m_fTurnMass*0.01f, GetForward()*1.0f);
			}
		}
		break;
	}

	if(GetStatus() == STATUS_PHYSICS || GetStatus() == STATUS_SIMPLE)
		if(AutoPilot.m_nCarMission == MISSION_HELI_FLYTOCOORS ||
		   AutoPilot.m_nCarMission == MISSION_PLANE_FLYTOCOORS)
			skipPhysics = true;

	float brake;
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
		m_fTireTemperature = 1.0f;
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

		CPhysical::ProcessControl();

		ProcessBuoyancy();

		// Rescale spring ratios, i.e. subtract wheel radius
		for(i = 0; i < 4; i++){
			// wheel radius in relation to suspension line
			float wheelRadius = 1.0f - m_aSuspensionSpringLength[i]/m_aSuspensionLineLength[i];
			// rescale such that 0.0 is fully compressed and 1.0 is fully extended
			m_aSuspensionSpringRatio[i] = (m_aSuspensionSpringRatio[i]-wheelRadius)/(1.0f-wheelRadius);
		}

		float fwdSpeed = Abs(DotProduct(m_vecMoveSpeed, GetForward()));
		CVector contactPoints[4];	// relative to model
		CVector contactSpeeds[4];	// speed at contact points
		CVector springDirections[4];	// normalized, in model space

		for(i = 0; i < 4; i++){
			// Set spring under certain circumstances
			if(Damage.GetWheelStatus(i) == WHEEL_STATUS_MISSING)
				m_aSuspensionSpringRatio[i] = 1.0f;
			else if(Damage.GetWheelStatus(i) == WHEEL_STATUS_BURST){
				// wheel more bumpy the faster we are
				if(CGeneral::GetRandomNumberInRange(0, (uint16)(40*fwdSpeed) + 98) < 100){
					m_aSuspensionSpringRatio[i] += 0.3f*(m_aSuspensionLineLength[i]-m_aSuspensionSpringLength[i])/m_aSuspensionSpringLength[i];
					if(m_aSuspensionSpringRatio[i] > 1.0f)
						m_aSuspensionSpringRatio[i] = 1.0f;
				}
			}else if(CSurfaceTable::GetAdhesionGroup(m_aWheelColPoints[i].surfaceB) == ADHESIVE_SAND &&
			         GetModelIndex() != MI_RHINO){
				fwdSpeed *= 0.7f;
				float f = 1.0f - fwdSpeed/0.3f - 0.7f*CWeather::WetRoads;
				f = Max(f, 0.4f);
				m_aSuspensionSpringRatio[i] += 0.35f*f*(m_aSuspensionLineLength[i]-m_aSuspensionSpringLength[i])/m_aSuspensionSpringLength[i];
				if(m_aSuspensionSpringRatio[i] > 1.0f)
					m_aSuspensionSpringRatio[i] = 1.0f;
			}

			// get points and directions if spring is compressed
			if(m_aSuspensionSpringRatio[i] < 1.0f){
				contactPoints[i] = m_aWheelColPoints[i].point - GetPosition();
				springDirections[i] = Multiply3x3(GetMatrix(), colModel->lines[i].p1 - colModel->lines[i].p0);
				springDirections[i].Normalise();
			}
		}

		// Make springs push up vehicle
		for(i = 0; i < 4; i++){
			if(m_aSuspensionSpringRatio[i] < 1.0f){
				float bias = pHandling->fSuspensionBias;
				if(i == CARWHEEL_REAR_LEFT || i == CARWHEEL_REAR_RIGHT)
					bias = 1.0f - bias;

				ApplySpringCollisionAlt(pHandling->fSuspensionForceLevel,
					springDirections[i], contactPoints[i],
					m_aSuspensionSpringRatio[i], bias, m_aWheelColPoints[i].normal);

				m_aWheelSkidmarkUnk[i] = false;
				if(m_aWheelColPoints[i].surfaceB == SURFACE_GRASS ||
				   m_aWheelColPoints[i].surfaceB == SURFACE_MUD_DRY)
					m_aWheelSkidmarkType[i] = SKIDMARK_MUDDY;
				else if(m_aWheelColPoints[i].surfaceB == SURFACE_SAND ||
				        m_aWheelColPoints[i].surfaceB == SURFACE_SAND_BEACH){
					m_aWheelSkidmarkType[i] = SKIDMARK_SANDY;
					m_aWheelSkidmarkUnk[i] = true;
				}else
					m_aWheelSkidmarkType[i] = SKIDMARK_NORMAL;
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
			if(m_aSuspensionSpringRatio[i] < 1.0f && m_aWheelColPoints[i].normal.z > 0.35f)
				springDirections[i] = -m_aWheelColPoints[i].normal;
		}

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
		if(!strongGrip1 && !CVehicle::bCheat3)
			gripCheat = false;
		float acceleration = pHandling->Transmission.CalculateDriveAcceleration(m_fGasPedal, m_nCurrentGear, m_fChangeGearTime, fwdSpeed, gripCheat);
		acceleration /= m_fForceMultiplier;

		if(IsRealHeli() || IsRealPlane())
			acceleration = 0.0f;

		if(bAudioChangingGear && m_fGasPedal > 0.4f && m_fBrakePedal < 0.1f && fwdSpeed > 0.15f &&
		   this == FindPlayerVehicle() && TheCamera.Cams[TheCamera.ActiveCam].Mode != CCam::MODE_1STPERSON){
			if(GetStatus() == STATUS_PLAYER && pHandling->Flags & HANDLING_IS_BUS){
				if(m_nBusDoorTimerEnd == 0)
					m_nBusDoorTimerEnd = 1000;
				else if(m_nBusDoorTimerEnd > CTimer::GetTimeStepInMilliseconds())
					m_nBusDoorTimerEnd -= CTimer::GetTimeStepInMilliseconds();
			}

			if(m_aSuspensionSpringRatio[0] < 1.0f || m_aSuspensionSpringRatio[1] < 1.0f ||
			   m_aSuspensionSpringRatio[2] < 1.0f || m_aSuspensionSpringRatio[3] < 1.0f)
				ApplyTurnForce(-GRAVITY*Min(m_fTurnMass, 2500.0f)*GetUp(), -1.0f*GetForward());
		}

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
				switch(pHandling->Transmission.nDriveType){
				case '4':
					m_nDriveWheelsOnGround++;
					break;
				case 'F':
					if(i == CARWHEEL_FRONT_LEFT || i == CARWHEEL_FRONT_RIGHT)
						m_nDriveWheelsOnGround++;
					break;
				case 'R':
					if(i == CARWHEEL_REAR_LEFT || i == CARWHEEL_REAR_RIGHT)
						m_nDriveWheelsOnGround++;
					break;
				}
			}
		}

		float traction;
		if(GetStatus() == STATUS_PHYSICS)
			traction = 0.004f * m_fTraction;
		else
			traction = 0.004f;
		traction *= pHandling->fTractionMultiplier / 4.0f;
		traction /= m_fForceMultiplier;
		if(CVehicle::bCheat3)
			traction *= 4.0f;

		if(FindPlayerVehicle() != this && (strongGrip1 || CVehicle::bCheat3)){
			traction *= 1.2f;
			acceleration *= 1.4f;
			if(strongGrip2 || CVehicle::bCheat3){
				traction *= 1.3f;
				acceleration *= 1.4f;
			}
		}

		static float fThrust;
		static tWheelState WheelState[4];

		bool rearWheelsFirst = !!(pHandling->Flags & HANDLING_REARWHEEL_1ST);

		// Process front wheels on ground - first try

		if(!rearWheelsFirst){
		if(m_aWheelTimer[CARWHEEL_FRONT_LEFT] > 0.0f || m_aWheelTimer[CARWHEEL_FRONT_RIGHT] > 0.0f){
			float s = Sin(m_fSteerAngle);
			float c = Cos(m_fSteerAngle);

			CVector wheelFwd, wheelRight, tmp;

			if(m_aWheelTimer[CARWHEEL_FRONT_LEFT] > 0.0f){
				if(mod_HandlingManager.HasRearWheelDrive(pHandling->nIdentifier))
					fThrust = 0.0f;
				else
					fThrust = acceleration;

				wheelFwd = GetForward();
				wheelFwd -= DotProduct(wheelFwd, m_aWheelColPoints[CARWHEEL_FRONT_LEFT].normal)*m_aWheelColPoints[CARWHEEL_FRONT_LEFT].normal;
				wheelFwd.Normalise();
				wheelRight = CrossProduct(wheelFwd, m_aWheelColPoints[CARWHEEL_FRONT_LEFT].normal);
				wheelRight.Normalise();
				tmp = c*wheelFwd - s*wheelRight;
				wheelRight = s*wheelFwd + c*wheelRight;
				wheelFwd = tmp;

				m_aWheelColPoints[CARWHEEL_FRONT_LEFT].surfaceA = SURFACE_WHEELBASE;
				float adhesion = CSurfaceTable::GetAdhesiveLimit(m_aWheelColPoints[CARWHEEL_FRONT_LEFT])*traction;
				if(GetStatus() == STATUS_PLAYER)
					adhesion *= CSurfaceTable::GetWetMultiplier(m_aWheelColPoints[CARWHEEL_FRONT_LEFT].surfaceB);
				WheelState[CARWHEEL_FRONT_LEFT] = m_aWheelState[CARWHEEL_FRONT_LEFT];

				if(Damage.GetWheelStatus(VEHWHEEL_FRONT_LEFT) == WHEEL_STATUS_BURST)
					ProcessWheel(wheelFwd, wheelRight,
						contactSpeeds[CARWHEEL_FRONT_LEFT], contactPoints[CARWHEEL_FRONT_LEFT],
						m_nWheelsOnGround, fThrust,
						brake*brakeBiasFront,
						adhesion*tractionBiasFront*Damage.m_fWheelDamageEffect,
						CARWHEEL_FRONT_LEFT,
						&m_aWheelSpeed[CARWHEEL_FRONT_LEFT],
						&WheelState[CARWHEEL_FRONT_LEFT],
						WHEEL_STATUS_BURST);
				else
					ProcessWheel(wheelFwd, wheelRight,
						contactSpeeds[CARWHEEL_FRONT_LEFT], contactPoints[CARWHEEL_FRONT_LEFT],
						m_nWheelsOnGround, fThrust,
						brake*brakeBiasFront,
						adhesion*tractionBiasFront,
						CARWHEEL_FRONT_LEFT,
						&m_aWheelSpeed[CARWHEEL_FRONT_LEFT],
						&WheelState[CARWHEEL_FRONT_LEFT],
						WHEEL_STATUS_OK);
			}

			if(m_aWheelTimer[CARWHEEL_FRONT_RIGHT] > 0.0f){
				if(mod_HandlingManager.HasRearWheelDrive(pHandling->nIdentifier))
					fThrust = 0.0f;
				else
					fThrust = acceleration;

				wheelFwd = GetForward();
				wheelFwd -= DotProduct(wheelFwd, m_aWheelColPoints[CARWHEEL_FRONT_RIGHT].normal)*m_aWheelColPoints[CARWHEEL_FRONT_RIGHT].normal;
				wheelFwd.Normalise();
				wheelRight = CrossProduct(wheelFwd, m_aWheelColPoints[CARWHEEL_FRONT_RIGHT].normal);
				wheelRight.Normalise();
				tmp = c*wheelFwd - s*wheelRight;
				wheelRight = s*wheelFwd + c*wheelRight;
				wheelFwd = tmp;

				m_aWheelColPoints[CARWHEEL_FRONT_RIGHT].surfaceA = SURFACE_WHEELBASE;
				float adhesion = CSurfaceTable::GetAdhesiveLimit(m_aWheelColPoints[CARWHEEL_FRONT_RIGHT])*traction;
				if(GetStatus() == STATUS_PLAYER)
					adhesion *= CSurfaceTable::GetWetMultiplier(m_aWheelColPoints[CARWHEEL_FRONT_RIGHT].surfaceB);
				WheelState[CARWHEEL_FRONT_RIGHT] = m_aWheelState[CARWHEEL_FRONT_RIGHT];

				if(Damage.GetWheelStatus(VEHWHEEL_FRONT_RIGHT) == WHEEL_STATUS_BURST)
					ProcessWheel(wheelFwd, wheelRight,
						contactSpeeds[CARWHEEL_FRONT_RIGHT], contactPoints[CARWHEEL_FRONT_RIGHT],
						m_nWheelsOnGround, fThrust,
						brake*brakeBiasFront,
						adhesion*tractionBiasFront*Damage.m_fWheelDamageEffect,
						CARWHEEL_FRONT_RIGHT,
						&m_aWheelSpeed[CARWHEEL_FRONT_RIGHT],
						&WheelState[CARWHEEL_FRONT_RIGHT],
						WHEEL_STATUS_BURST);
				else
					ProcessWheel(wheelFwd, wheelRight,
						contactSpeeds[CARWHEEL_FRONT_RIGHT], contactPoints[CARWHEEL_FRONT_RIGHT],
						m_nWheelsOnGround, fThrust,
						brake*brakeBiasFront,
						adhesion*tractionBiasFront,
						CARWHEEL_FRONT_RIGHT,
						&m_aWheelSpeed[CARWHEEL_FRONT_RIGHT],
						&WheelState[CARWHEEL_FRONT_RIGHT],
						WHEEL_STATUS_OK);
			}
		}

		// Process front wheels off ground

		if(!IsRealHeli()){
			if(m_aWheelTimer[CARWHEEL_FRONT_LEFT] <= 0.0f){
				if(mod_HandlingManager.HasRearWheelDrive(pHandling->nIdentifier) || acceleration == 0.0f)
					m_aWheelSpeed[CARWHEEL_FRONT_LEFT] *= 0.95f;
				else{
					if(acceleration > 0.0f){
						if(m_aWheelSpeed[CARWHEEL_FRONT_LEFT] < 2.0f)
							m_aWheelSpeed[CARWHEEL_FRONT_LEFT] -= 0.2f;
					}else{
						if(m_aWheelSpeed[CARWHEEL_FRONT_LEFT] > -2.0f)
							m_aWheelSpeed[CARWHEEL_FRONT_LEFT] += 0.1f;
					}
				}
				m_aWheelRotation[CARWHEEL_FRONT_LEFT] += m_aWheelSpeed[CARWHEEL_FRONT_LEFT];
			}
			if(m_aWheelTimer[CARWHEEL_FRONT_RIGHT] <= 0.0f){
				if(mod_HandlingManager.HasRearWheelDrive(pHandling->nIdentifier) || acceleration == 0.0f)
					m_aWheelSpeed[CARWHEEL_FRONT_RIGHT] *= 0.95f;
				else{
					if(acceleration > 0.0f){
						if(m_aWheelSpeed[CARWHEEL_FRONT_RIGHT] < 2.0f)
							m_aWheelSpeed[CARWHEEL_FRONT_RIGHT] -= 0.2f;
					}else{
						if(m_aWheelSpeed[CARWHEEL_FRONT_RIGHT] > -2.0f)
							m_aWheelSpeed[CARWHEEL_FRONT_RIGHT] += 0.1f;
					}
				}
				m_aWheelRotation[CARWHEEL_FRONT_RIGHT] += m_aWheelSpeed[CARWHEEL_FRONT_RIGHT];
			}
		}
		}

		// Process rear wheels

		if(m_aWheelTimer[CARWHEEL_REAR_LEFT] > 0.0f || m_aWheelTimer[CARWHEEL_REAR_RIGHT] > 0.0f){
			CVector wheelFwd = GetForward();
			CVector wheelRight = GetRight();

			float rearBrake = brake;
			float rearTraction = traction;
			if(bIsHandbrakeOn){
#ifdef FIX_BUGS
				// Not sure if this is needed, but brake usually has timestep as a factor
				rearBrake = 20000.0f * CTimer::GetTimeStepFix();
#else
				rearBrake = 20000.0f;
#endif
				if(fwdSpeed > 0.1f && pHandling->Flags & HANDLING_HANDBRAKE_TYRE){
					m_fTireTemperature += 0.005*CTimer::GetTimeStep();
					if(m_fTireTemperature > 2.0f)
						m_fTireTemperature = 2.0f;
				}
			}else if(m_doingBurnout && !mod_HandlingManager.HasFrontWheelDrive(pHandling->nIdentifier)){
				rearBrake = 0.0f;
				rearTraction = 0.0f;
				// BUG: missing timestep
				ApplyTurnForce(contactPoints[CARWHEEL_REAR_LEFT], -0.001f*m_fTurnMass*m_fSteerAngle*GetRight());
			}else if(m_fTireTemperature > 1.0f){
				rearTraction *= m_fTireTemperature;
			}

			if(m_aWheelTimer[CARWHEEL_REAR_LEFT] > 0.0f){
				if(mod_HandlingManager.HasFrontWheelDrive(pHandling->nIdentifier))
					fThrust = 0.0f;
				else
					fThrust = acceleration;

				wheelFwd = GetForward();
				wheelFwd -= DotProduct(wheelFwd, m_aWheelColPoints[CARWHEEL_REAR_LEFT].normal)*m_aWheelColPoints[CARWHEEL_REAR_LEFT].normal;
				wheelFwd.Normalise();
				wheelRight = CrossProduct(wheelFwd, m_aWheelColPoints[CARWHEEL_REAR_LEFT].normal);
				wheelRight.Normalise();

				m_aWheelColPoints[CARWHEEL_REAR_LEFT].surfaceA = SURFACE_WHEELBASE;
				float adhesion = CSurfaceTable::GetAdhesiveLimit(m_aWheelColPoints[CARWHEEL_REAR_LEFT])*rearTraction;
				if(GetStatus() == STATUS_PLAYER)
					adhesion *= CSurfaceTable::GetWetMultiplier(m_aWheelColPoints[CARWHEEL_REAR_LEFT].surfaceB);
				WheelState[CARWHEEL_REAR_LEFT] = m_aWheelState[CARWHEEL_REAR_LEFT];

				if(Damage.GetWheelStatus(VEHWHEEL_REAR_LEFT) == WHEEL_STATUS_BURST)
					ProcessWheel(wheelFwd, wheelRight,
						contactSpeeds[CARWHEEL_REAR_LEFT], contactPoints[CARWHEEL_REAR_LEFT],
						m_nWheelsOnGround, fThrust,
						rearBrake*brakeBiasRear,
						adhesion*tractionBiasRear*Damage.m_fWheelDamageEffect,
						CARWHEEL_REAR_LEFT,
						&m_aWheelSpeed[CARWHEEL_REAR_LEFT],
						&WheelState[CARWHEEL_REAR_LEFT],
						WHEEL_STATUS_BURST);
				else
					ProcessWheel(wheelFwd, wheelRight,
						contactSpeeds[CARWHEEL_REAR_LEFT], contactPoints[CARWHEEL_REAR_LEFT],
						m_nWheelsOnGround, fThrust,
						rearBrake*brakeBiasRear,
						adhesion*tractionBiasRear,
						CARWHEEL_REAR_LEFT,
						&m_aWheelSpeed[CARWHEEL_REAR_LEFT],
						&WheelState[CARWHEEL_REAR_LEFT],
						WHEEL_STATUS_OK);
			}

#ifdef FIX_BUGS
			// Shouldn't we reset these after the left wheel?
			wheelFwd = GetForward();
			wheelRight = GetRight();
#endif

			if(m_aWheelTimer[CARWHEEL_REAR_RIGHT] > 0.0f){
				if(mod_HandlingManager.HasFrontWheelDrive(pHandling->nIdentifier))
					fThrust = 0.0f;
				else
					fThrust = acceleration;

				wheelFwd = GetForward();
				wheelFwd -= DotProduct(wheelFwd, m_aWheelColPoints[CARWHEEL_REAR_RIGHT].normal)*m_aWheelColPoints[CARWHEEL_REAR_RIGHT].normal;
				wheelFwd.Normalise();
				wheelRight = CrossProduct(wheelFwd, m_aWheelColPoints[CARWHEEL_REAR_RIGHT].normal);
				wheelRight.Normalise();

				m_aWheelColPoints[CARWHEEL_REAR_RIGHT].surfaceA = SURFACE_WHEELBASE;
				float adhesion = CSurfaceTable::GetAdhesiveLimit(m_aWheelColPoints[CARWHEEL_REAR_RIGHT])*rearTraction;
				if(GetStatus() == STATUS_PLAYER)
					adhesion *= CSurfaceTable::GetWetMultiplier(m_aWheelColPoints[CARWHEEL_REAR_RIGHT].surfaceB);
				WheelState[CARWHEEL_REAR_RIGHT] = m_aWheelState[CARWHEEL_REAR_RIGHT];

				if(Damage.GetWheelStatus(VEHWHEEL_REAR_RIGHT) == WHEEL_STATUS_BURST)
					ProcessWheel(wheelFwd, wheelRight,
						contactSpeeds[CARWHEEL_REAR_RIGHT], contactPoints[CARWHEEL_REAR_RIGHT],
						m_nWheelsOnGround, fThrust,
						rearBrake*brakeBiasRear,
						adhesion*tractionBiasRear*Damage.m_fWheelDamageEffect,
						CARWHEEL_REAR_RIGHT,
						&m_aWheelSpeed[CARWHEEL_REAR_RIGHT],
						&WheelState[CARWHEEL_REAR_RIGHT],
						WHEEL_STATUS_BURST);
				else
					ProcessWheel(wheelFwd, wheelRight,
						contactSpeeds[CARWHEEL_REAR_RIGHT], contactPoints[CARWHEEL_REAR_RIGHT],
						m_nWheelsOnGround, fThrust,
						rearBrake*brakeBiasRear,
						adhesion*tractionBiasRear,
						CARWHEEL_REAR_RIGHT,
						&m_aWheelSpeed[CARWHEEL_REAR_RIGHT],
						&WheelState[CARWHEEL_REAR_RIGHT],
						WHEEL_STATUS_OK);
			}
		}

		if(m_doingBurnout && !mod_HandlingManager.HasFrontWheelDrive(pHandling->nIdentifier) &&
	           (m_aWheelState[CARWHEEL_REAR_LEFT] == WHEEL_STATE_SPINNING || m_aWheelState[CARWHEEL_REAR_RIGHT] == WHEEL_STATE_SPINNING)){
			m_fTireTemperature += 0.001f*CTimer::GetTimeStep();
			if(m_fTireTemperature > 3.0f)
				m_fTireTemperature = 3.0f;
		}else if(m_fTireTemperature > 1.0f){
			m_fTireTemperature = (m_fTireTemperature - 1.0f)*Pow(0.995f, CTimer::GetTimeStep()) + 1.0f;
		}

		// Process rear wheels off ground

		if(!IsRealHeli()){
			if(m_aWheelTimer[CARWHEEL_REAR_LEFT] <= 0.0f){
				if(mod_HandlingManager.HasFrontWheelDrive(pHandling->nIdentifier) || acceleration == 0.0f)
					m_aWheelSpeed[CARWHEEL_REAR_LEFT] *= 0.95f;
				else{
					if(acceleration > 0.0f){
						if(m_aWheelSpeed[CARWHEEL_REAR_LEFT] < 2.0f)
							m_aWheelSpeed[CARWHEEL_REAR_LEFT] -= 0.2f;
					}else{
						if(m_aWheelSpeed[CARWHEEL_REAR_LEFT] > -2.0f)
							m_aWheelSpeed[CARWHEEL_REAR_LEFT] += 0.1f;
					}
				}
				m_aWheelRotation[CARWHEEL_REAR_LEFT] += m_aWheelSpeed[CARWHEEL_REAR_LEFT];
			}
			if(m_aWheelTimer[CARWHEEL_REAR_RIGHT] <= 0.0f){
				if(mod_HandlingManager.HasFrontWheelDrive(pHandling->nIdentifier) || acceleration == 0.0f)
					m_aWheelSpeed[CARWHEEL_REAR_RIGHT] *= 0.95f;
				else{
					if(acceleration > 0.0f){
						if(m_aWheelSpeed[CARWHEEL_REAR_RIGHT] < 2.0f)
							m_aWheelSpeed[CARWHEEL_REAR_RIGHT] -= 0.2f;
					}else{
						if(m_aWheelSpeed[CARWHEEL_REAR_RIGHT] > -2.0f)
							m_aWheelSpeed[CARWHEEL_REAR_RIGHT] += 0.1f;
					}
				}
				m_aWheelRotation[CARWHEEL_REAR_RIGHT] += m_aWheelSpeed[CARWHEEL_REAR_RIGHT];
			}
		}

		// Process front wheels on ground - second try

		if(rearWheelsFirst){
		if(m_aWheelTimer[CARWHEEL_FRONT_LEFT] > 0.0f || m_aWheelTimer[CARWHEEL_FRONT_RIGHT] > 0.0f){
			float s = Sin(m_fSteerAngle);
			float c = Cos(m_fSteerAngle);

			CVector wheelFwd, wheelRight, tmp;

			if(m_aWheelTimer[CARWHEEL_FRONT_LEFT] > 0.0f){
				if(mod_HandlingManager.HasRearWheelDrive(pHandling->nIdentifier))
					fThrust = 0.0f;
				else
					fThrust = acceleration;

				wheelFwd = GetForward();
				wheelFwd -= DotProduct(wheelFwd, m_aWheelColPoints[CARWHEEL_FRONT_LEFT].normal)*m_aWheelColPoints[CARWHEEL_FRONT_LEFT].normal;
				wheelFwd.Normalise();
				wheelRight = CrossProduct(wheelFwd, m_aWheelColPoints[CARWHEEL_FRONT_LEFT].normal);
				wheelRight.Normalise();
				tmp = c*wheelFwd - s*wheelRight;
				wheelRight = s*wheelFwd + c*wheelRight;
				wheelFwd = tmp;

				m_aWheelColPoints[CARWHEEL_FRONT_LEFT].surfaceA = SURFACE_WHEELBASE;
				float adhesion = CSurfaceTable::GetAdhesiveLimit(m_aWheelColPoints[CARWHEEL_FRONT_LEFT])*traction;
				if(GetStatus() == STATUS_PLAYER)
					adhesion *= CSurfaceTable::GetWetMultiplier(m_aWheelColPoints[CARWHEEL_FRONT_LEFT].surfaceB);
				WheelState[CARWHEEL_FRONT_LEFT] = m_aWheelState[CARWHEEL_FRONT_LEFT];

				if(Damage.GetWheelStatus(VEHWHEEL_FRONT_LEFT) == WHEEL_STATUS_BURST)
					ProcessWheel(wheelFwd, wheelRight,
						contactSpeeds[CARWHEEL_FRONT_LEFT], contactPoints[CARWHEEL_FRONT_LEFT],
						m_nWheelsOnGround, fThrust,
						brake*brakeBiasFront,
						adhesion*tractionBiasFront*Damage.m_fWheelDamageEffect,
						CARWHEEL_FRONT_LEFT,
						&m_aWheelSpeed[CARWHEEL_FRONT_LEFT],
						&WheelState[CARWHEEL_FRONT_LEFT],
						WHEEL_STATUS_BURST);
				else
					ProcessWheel(wheelFwd, wheelRight,
						contactSpeeds[CARWHEEL_FRONT_LEFT], contactPoints[CARWHEEL_FRONT_LEFT],
						m_nWheelsOnGround, fThrust,
						brake*brakeBiasFront,
						adhesion*tractionBiasFront,
						CARWHEEL_FRONT_LEFT,
						&m_aWheelSpeed[CARWHEEL_FRONT_LEFT],
						&WheelState[CARWHEEL_FRONT_LEFT],
						WHEEL_STATUS_OK);
			}

			if(m_aWheelTimer[CARWHEEL_FRONT_RIGHT] > 0.0f){
				if(mod_HandlingManager.HasRearWheelDrive(pHandling->nIdentifier))
					fThrust = 0.0f;
				else
					fThrust = acceleration;

				wheelFwd = GetForward();
				wheelFwd -= DotProduct(wheelFwd, m_aWheelColPoints[CARWHEEL_FRONT_RIGHT].normal)*m_aWheelColPoints[CARWHEEL_FRONT_RIGHT].normal;
				wheelFwd.Normalise();
				wheelRight = CrossProduct(wheelFwd, m_aWheelColPoints[CARWHEEL_FRONT_RIGHT].normal);
				wheelRight.Normalise();
				tmp = c*wheelFwd - s*wheelRight;
				wheelRight = s*wheelFwd + c*wheelRight;
				wheelFwd = tmp;

				m_aWheelColPoints[CARWHEEL_FRONT_RIGHT].surfaceA = SURFACE_WHEELBASE;
				float adhesion = CSurfaceTable::GetAdhesiveLimit(m_aWheelColPoints[CARWHEEL_FRONT_RIGHT])*traction;
				if(GetStatus() == STATUS_PLAYER)
					adhesion *= CSurfaceTable::GetWetMultiplier(m_aWheelColPoints[CARWHEEL_FRONT_RIGHT].surfaceB);
				WheelState[CARWHEEL_FRONT_RIGHT] = m_aWheelState[CARWHEEL_FRONT_RIGHT];

				if(Damage.GetWheelStatus(VEHWHEEL_FRONT_RIGHT) == WHEEL_STATUS_BURST)
					ProcessWheel(wheelFwd, wheelRight,
						contactSpeeds[CARWHEEL_FRONT_RIGHT], contactPoints[CARWHEEL_FRONT_RIGHT],
						m_nWheelsOnGround, fThrust,
						brake*brakeBiasFront,
						adhesion*tractionBiasFront*Damage.m_fWheelDamageEffect,
						CARWHEEL_FRONT_RIGHT,
						&m_aWheelSpeed[CARWHEEL_FRONT_RIGHT],
						&WheelState[CARWHEEL_FRONT_RIGHT],
						WHEEL_STATUS_BURST);
				else
					ProcessWheel(wheelFwd, wheelRight,
						contactSpeeds[CARWHEEL_FRONT_RIGHT], contactPoints[CARWHEEL_FRONT_RIGHT],
						m_nWheelsOnGround, fThrust,
						brake*brakeBiasFront,
						adhesion*tractionBiasFront,
						CARWHEEL_FRONT_RIGHT,
						&m_aWheelSpeed[CARWHEEL_FRONT_RIGHT],
						&WheelState[CARWHEEL_FRONT_RIGHT],
						WHEEL_STATUS_OK);
			}
		}

		// Process front wheels off ground

		if(!IsRealHeli()){
			if(m_aWheelTimer[CARWHEEL_FRONT_LEFT] <= 0.0f){
				if(mod_HandlingManager.HasRearWheelDrive(pHandling->nIdentifier) || acceleration == 0.0f)
					m_aWheelSpeed[CARWHEEL_FRONT_LEFT] *= 0.95f;
				else{
					if(acceleration > 0.0f){
						if(m_aWheelSpeed[CARWHEEL_FRONT_LEFT] < 2.0f)
							m_aWheelSpeed[CARWHEEL_FRONT_LEFT] -= 0.2f;
					}else{
						if(m_aWheelSpeed[CARWHEEL_FRONT_LEFT] > -2.0f)
							m_aWheelSpeed[CARWHEEL_FRONT_LEFT] += 0.1f;
					}
				}
				m_aWheelRotation[CARWHEEL_FRONT_LEFT] += m_aWheelSpeed[CARWHEEL_FRONT_LEFT];
			}
			if(m_aWheelTimer[CARWHEEL_FRONT_RIGHT] <= 0.0f){
				if(mod_HandlingManager.HasRearWheelDrive(pHandling->nIdentifier) || acceleration == 0.0f)
					m_aWheelSpeed[CARWHEEL_FRONT_RIGHT] *= 0.95f;
				else{
					if(acceleration > 0.0f){
						if(m_aWheelSpeed[CARWHEEL_FRONT_RIGHT] < 2.0f)
							m_aWheelSpeed[CARWHEEL_FRONT_RIGHT] -= 0.2f;
					}else{
						if(m_aWheelSpeed[CARWHEEL_FRONT_RIGHT] > -2.0f)
							m_aWheelSpeed[CARWHEEL_FRONT_RIGHT] += 0.1f;
					}
				}
				m_aWheelRotation[CARWHEEL_FRONT_RIGHT] += m_aWheelSpeed[CARWHEEL_FRONT_RIGHT];
			}
		}
		}

		for(i = 0; i < 4; i++){
			float wheelPos = colModel->lines[i].p0.z;
			if(m_aSuspensionSpringRatio[i] > 0.0f)
				wheelPos -= m_aSuspensionSpringRatio[i]*m_aSuspensionSpringLength[i];
			if(GetModelIndex() == MI_VOODOO && bUsingSpecialColModel)
				m_aWheelPosition[i] = wheelPos;
			else
				m_aWheelPosition[i] += (wheelPos - m_aWheelPosition[i])*0.75f;
		}
		for(i = 0; i < 4; i++)
			m_aWheelState[i] = WheelState[i];
		if(m_fGasPedal < 0.0f){
			if(m_aWheelState[CARWHEEL_REAR_LEFT] == WHEEL_STATE_SPINNING)
				m_aWheelState[CARWHEEL_REAR_LEFT] = WHEEL_STATE_NORMAL;
			if(m_aWheelState[CARWHEEL_REAR_RIGHT] == WHEEL_STATE_SPINNING)
				m_aWheelState[CARWHEEL_REAR_RIGHT] = WHEEL_STATE_NORMAL;
		}

		// Process horn

		if(GetStatus() != STATUS_PLAYER){
			if(!IsAlarmOn())
				ReduceHornCounter();
		}else{
			if(UsesSiren(GetModelIndex())){
				if(Pads[0].bHornHistory[Pads[0].iCurrHornHistory]){
					if(Pads[0].bHornHistory[(Pads[0].iCurrHornHistory+4) % 5] &&
					   Pads[0].bHornHistory[(Pads[0].iCurrHornHistory+3) % 5])
						m_nCarHornTimer = 1;
					else
						m_nCarHornTimer = 0;
				}else if(Pads[0].bHornHistory[(Pads[0].iCurrHornHistory+4) % 5] &&
				         !Pads[0].bHornHistory[(Pads[0].iCurrHornHistory+1) % 5]){
					m_nCarHornTimer = 0;
					m_bSirenOrAlarm = !m_bSirenOrAlarm;
				}else
					m_nCarHornTimer = 0;
			}else if(GetModelIndex() != MI_VOODOO && !CVehicle::bCheat3 && !carHasNitro){
				if(!IsAlarmOn()){
					if(Pads[0].GetHorn())
						m_nCarHornTimer = 1;
					else
						m_nCarHornTimer = 0;
				}
			}
		}

		// Flying

		bool playRotorSound = false;
		if(GetStatus() != STATUS_PLAYER && GetStatus() != STATUS_PLAYER_REMOTE && GetStatus() != STATUS_PHYSICS){
			if(IsRealHeli()){
				bEngineOn = false;
				m_aWheelSpeed[1] = Max(m_aWheelSpeed[1]-0.0005f, 0.0f);
				if(GetModelIndex() != MI_RCRAIDER && GetModelIndex() != MI_RCGOBLIN)
					if(m_aWheelSpeed[1] < 0.154f && m_aWheelSpeed[1] > 0.0044f)
						playRotorSound = true;
			}
		}else if((GetModelIndex() == MI_DODO || CVehicle::bAllDodosCheat) &&
		         m_vecMoveSpeed.Magnitude() > 0.0f && CTimer::GetTimeStep() > 0.0f){
#ifdef ALT_DODO_CHEAT
			if (bAltDodoCheat)
				FlyingControl(FLIGHT_MODEL_PLANE);
			else
#endif
			if(GetModelIndex() == MI_DODO)
				FlyingControl(FLIGHT_MODEL_DODO);
			else
				FlyingControl(FLIGHT_MODEL_PLANE);
		}else if(GetModelIndex() == MI_RCBARON){
			FlyingControl(FLIGHT_MODEL_RCPLANE);
		}else if(IsRealHeli() || bAllCarCheat){
			// Speed up rotor
			if(m_aWheelSpeed[1] < 0.22f && !bIsInWater){
				if(GetModelIndex() == MI_RCRAIDER || GetModelIndex() == MI_RCGOBLIN)
					m_aWheelSpeed[1] += 0.003f;
				else
					m_aWheelSpeed[1] += 0.001f;
			}

			// Fly
			if(m_aWheelSpeed[1] > 0.15f){
				if(GetModelIndex() == MI_RCRAIDER || GetModelIndex() == MI_RCGOBLIN)
					FlyingControl(FLIGHT_MODEL_RCHELI);
				else if(m_nWheelsOnGround < 4 && !(GetModelIndex() == MI_SEASPAR && bTouchingWater) ||
				        CPad::GetPad(0)->GetAccelerate() != 0 || CPad::GetPad(0)->GetCarGunUpDown() > 1.0f ||
				        Abs(m_vecMoveSpeed.x) > 0.02f ||
				        Abs(m_vecMoveSpeed.y) > 0.02f ||
				        Abs(m_vecMoveSpeed.z) > 0.02f)
					FlyingControl(FLIGHT_MODEL_HELI);
			}

			// Blade collision
			if(m_aWheelSpeed[1] > 0.015f && m_aCarNodes[CAR_BONNET]){
				CMatrix mat;
				mat.Attach(RwFrameGetMatrix(m_aCarNodes[CAR_BONNET]));
				if(GetModelIndex() == MI_RCRAIDER || GetModelIndex() == MI_RCGOBLIN)
					DoBladeCollision(mat.GetPosition(), GetMatrix(), ROTOR_TOP, 0.72f, 0.9f);
				else if(GetModelIndex() == MI_SPARROW || GetModelIndex() == MI_SEASPAR)
					DoBladeCollision(mat.GetPosition(), GetMatrix(), ROTOR_TOP, 5.15f, 0.8f);
				else if(GetModelIndex() == MI_HUNTER)
					DoBladeCollision(mat.GetPosition(), GetMatrix(), ROTOR_TOP, 6.15f, 0.5f);
				else
					DoBladeCollision(mat.GetPosition(), GetMatrix(), ROTOR_TOP, 6.15f, 1.0f);
			}

			// Heli weapons
			if(GetModelIndex() == MI_HUNTER && GetStatus() == STATUS_PLAYER){
				// Hunter rockets
				if(CPad::GetPad(0)->CarGunJustDown() && CTimer::GetTimeInMilliseconds() > m_nGunFiringTime+350){
					CWeapon gun(WEAPONTYPE_ROCKETLAUNCHER, 100);
					CVector source = vecHunterRocketPos;
					source = GetMatrix()*source + Max(DotProduct(m_vecMoveSpeed, GetForward()), 0.0f)*GetForward()*CTimer::GetTimeStep();
					gun.FireProjectile(this, &source, 0.0f);

					source = vecHunterRocketPos;
					source.x = -source.x;
					source = GetMatrix()*source + Max(DotProduct(m_vecMoveSpeed, GetForward()), 0.0f)*GetForward()*CTimer::GetTimeStep();
					gun.FireProjectile(this, &source, 0.0f);

					CStats::RoundsFiredByPlayer++;
					DMAudio.PlayOneShot(m_audioEntityId, SOUND_WEAPON_SHOT_FIRED, 0.0f);
					m_nGunFiringTime = CTimer::GetTimeInMilliseconds();
				// Hunter gun
				}else if(CPad::GetPad(0)->GetHandBrake() && CTimer::GetTimeInMilliseconds() > m_nGunFiringTime+60){
					CWeapon gun(WEAPONTYPE_HELICANNON, 5000);
					CVector source = vecHunterGunPos;
					source = GetMatrix()*source + m_vecMoveSpeed*CTimer::GetTimeStep();
					gun.FireInstantHit(this, &source);
					gun.AddGunshell(this, source, CVector2D(0.0f, 0.1f), 0.025f);
					CStats::RoundsFiredByPlayer++;
					DMAudio.PlayOneShot(m_audioEntityId, SOUND_WEAPON_SHOT_FIRED, 0.0f);
					m_nGunFiringTime = CTimer::GetTimeInMilliseconds();
				}
			}else if(GetModelIndex() == MI_SEASPAR && GetStatus() == STATUS_PLAYER){
				// Sea sparrow gun
				if(CPad::GetPad(0)->GetHandBrake() && CTimer::GetTimeInMilliseconds() > m_nGunFiringTime+40){
					CWeapon gun(WEAPONTYPE_M4, 5000);
					CVector source = vecSeaSparrowGunPos;
					source = GetMatrix()*source + m_vecMoveSpeed*CTimer::GetTimeStep();
					gun.FireInstantHit(this, &source);
					gun.AddGunshell(this, source, CVector2D(0.0f, 0.1f), 0.025f);
					CStats::RoundsFiredByPlayer++;
					DMAudio.PlayOneShot(m_audioEntityId, SOUND_WEAPON_SHOT_FIRED, 0.0f);
					m_nGunFiringTime = CTimer::GetTimeInMilliseconds();
				}
			}

			if(GetModelIndex() != MI_RCRAIDER && GetModelIndex() != MI_RCGOBLIN)
				if(m_aWheelSpeed[1] < 0.154f && m_aWheelSpeed[1] > 0.0044f)
					playRotorSound = true;
		}

		// Play rotor sound
		if(playRotorSound && m_aCarNodes[CAR_BONNET]){
			CVector camDist = TheCamera.GetPosition() - GetPosition();
			float distSq = camDist.MagnitudeSqr();
			if(distSq < SQR(20.0f) && Abs(m_fPropellerRotation - m_aWheelRotation[1]) > DEGTORAD(30.0f)){
				CMatrix mat;
				mat.Attach(RwFrameGetMatrix(m_aCarNodes[CAR_BONNET]));
				CVector blade = mat.GetRight();
				blade = GetMatrix() * blade;
				camDist /= Max(Sqrt(distSq), 0.01f);
				if(Abs(DotProduct(camDist, blade)) > 0.95f){
					DMAudio.PlayOneShot(m_audioEntityId, SOUND_31, 0.0f);
					m_fPropellerRotation = m_aWheelRotation[1];
				}
			}
		}
	}



	// Process car on fire
	// A similar calculation of damagePos is done elsewhere for smoke

	uint8 engineStatus = Damage.GetEngineStatus();
	CVector damagePos = ((CVehicleModelInfo*)CModelInfo::GetModelInfo(GetModelIndex()))->m_positions[CAR_POS_HEADLIGHTS];

	switch(Damage.GetDoorStatus(DOOR_BONNET)){
	case DOOR_STATUS_OK:
	case DOOR_STATUS_SMASHED:
		// Bonnet is still there, smoke comes out at the edge
		damagePos += vecDAMAGE_ENGINE_POS_SMALL;
		break;
	case DOOR_STATUS_SWINGING:
	case DOOR_STATUS_MISSING:
		// Bonnet is gone, smoke comes out at the engine
		damagePos += vecDAMAGE_ENGINE_POS_BIG;
		break;
	}

	// move fire forward if in first person
	if(this == FindPlayerVehicle() && TheCamera.GetLookingForwardFirstPerson())
		if(m_fHealth < 250.0f && GetStatus() != STATUS_WRECKED){
			if(GetModelIndex() == MI_FIRETRUCK)
				damagePos += CVector(0.0f, 3.0f, -0.2f);
			else
				damagePos += CVector(0.0f, 1.2f, -0.8f);
		}

	damagePos = GetMatrix()*damagePos;
	damagePos.z += 0.15f;

	if(m_fHealth < 250.0f && GetStatus() != STATUS_WRECKED){
		// Car is on fire

		CParticle::AddParticle(PARTICLE_CARFLAME, damagePos,
			CVector(0.0f, 0.0f, CGeneral::GetRandomNumberInRange(0.01125f, 0.09f)),
			nil, 0.63f);

		CVector coors = damagePos;
		coors.x += CGeneral::GetRandomNumberInRange(-0.5625f, 0.5625f),
		coors.y += CGeneral::GetRandomNumberInRange(-0.5625f, 0.5625f),
		coors.z += CGeneral::GetRandomNumberInRange(0.5625f, 2.25f);
		CParticle::AddParticle(PARTICLE_CARFLAME_SMOKE, coors, CVector(0.0f, 0.0f, 0.0f));

		CParticle::AddParticle(PARTICLE_ENGINE_SMOKE2, damagePos, CVector(0.0f, 0.0f, 0.0f), nil, 0.5f);

		// Blow up car after 5 seconds
		m_fFireBlowUpTimer += CTimer::GetTimeStepInMilliseconds();
		if(m_fFireBlowUpTimer > 5000.0f)
			BlowUpCar(m_pSetOnFireEntity);
	}else
		m_fFireBlowUpTimer = 0.0f;

	// Decrease car health if engine is damaged badly
	if(engineStatus > ENGINE_STATUS_ON_FIRE && m_fHealth > 250.0f)
		m_fHealth -= 2.0f;

	ProcessDelayedExplosion();


	if(m_bSirenOrAlarm && (CTimer::GetFrameCounter()&7) == 5 &&
	   UsesSiren(GetModelIndex()) && GetModelIndex() != MI_MRWHOOP)
		CCarAI::MakeWayForCarWithSiren(this);


	// Find out how much to shake the pad depending on suspension and ground surface

	float suspShake = 0.0f;
	float surfShake = 0.0f;
	float speedsq = m_vecMoveSpeed.MagnitudeSqr();
	for(i = 0; i < 4; i++){
		float suspChange = m_aSuspensionSpringRatioPrev[i] - m_aSuspensionSpringRatio[i];
		if(suspChange > 0.3f && !drivingInSand && speedsq > 0.04f){
			if(Damage.GetWheelStatus(i) == WHEEL_STATUS_BURST)
				DMAudio.PlayOneShot(m_audioEntityId, SOUND_CAR_JUMP_2, suspChange);
			else
				DMAudio.PlayOneShot(m_audioEntityId, SOUND_CAR_JUMP, suspChange);
			if(suspChange > suspShake)
				suspShake = suspChange;
		}

		uint8 surf = m_aWheelColPoints[i].surfaceB;
		if(surf == SURFACE_GRAVEL || surf == SURFACE_WATER || surf == SURFACE_HEDGE){
			if(surfShake < 0.2f)
				surfShake = 0.3f;
		}else if(surf == SURFACE_MUD_DRY){
			if(surfShake < 0.1f)
				surfShake = 0.2f;
		}else if(surf == SURFACE_GRASS){
			if(surfShake < 0.05f)
				surfShake = 0.1f;
		}

		if(this == FindPlayerVehicle())
// BUG: this only observes one of the wheels
			TheCamera.m_bVehicleSuspenHigh = Abs(suspChange) > 0.05f;

		m_aSuspensionSpringRatioPrev[i] = m_aSuspensionSpringRatio[i];
		m_aSuspensionSpringRatio[i] = 1.0f;
	}

	// Shake pad

	if(!drivingInSand && (suspShake > 0.0f || surfShake > 0.0f) && GetStatus() == STATUS_PLAYER){
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

	if(!bWarnedPeds && GetVehicleAppearance() != VEHICLE_APPEARANCE_HELI && GetVehicleAppearance() != VEHICLE_APPEARANCE_PLANE)
		CCarCtrl::ScanForPedDanger(this);


	// Turn around at the edge of the world
	// TODO: make the numbers defines

	float heading;
	if(GetPosition().x > 1950.0f-400.0f){
		if(m_vecMoveSpeed.x > 0.0f)
			m_vecMoveSpeed.x *= -1.0f;
		heading = GetForward().Heading();
		if(heading > 0.0f)	// going west
			SetHeading(-heading);
	}else if(GetPosition().x < -1950.0f-400.0f){
		if(m_vecMoveSpeed.x < 0.0f)
			m_vecMoveSpeed.x *= -1.0f;
		heading = GetForward().Heading();
		if(heading < 0.0f)	// going east
			SetHeading(-heading);
	}
	if(GetPosition().y > 1950.0f){
		if(m_vecMoveSpeed.y > 0.0f)
			m_vecMoveSpeed.y *= -1.0f;
		heading = GetForward().Heading();
		if(heading < HALFPI && heading > 0.0f)
			SetHeading(PI-heading);
		else if(heading > -HALFPI && heading < 0.0f)
			SetHeading(-PI-heading);
	}else if(GetPosition().y < -1950.0f){
		if(m_vecMoveSpeed.y < 0.0f)
			m_vecMoveSpeed.y *= -1.0f;
		heading = GetForward().Heading();
		if(heading > HALFPI)
			SetHeading(PI-heading);
		else if(heading < -HALFPI)
			SetHeading(-PI-heading);
	}

	if(bInfiniteMass){
		m_vecMoveSpeed = CVector(0.0f, 0.0f, 0.0f);
		m_vecTurnSpeed = CVector(0.0f, 0.0f, 0.0f);
		m_vecMoveFriction = CVector(0.0f, 0.0f, 0.0f);
		m_vecTurnFriction = CVector(0.0f, 0.0f, 0.0f);
	}else if(!skipPhysics &&
	         (m_fGasPedal == 0.0f && brake == 0.0f || GetStatus() == STATUS_WRECKED)){
		if(Abs(m_vecMoveSpeed.x) < 0.005f &&
		   Abs(m_vecMoveSpeed.y) < 0.005f &&
		   Abs(m_vecMoveSpeed.z) < 0.005f &&
		   !(m_fDamageImpulse > 0.0f && m_pDamageEntity == FindPlayerPed()) &&
		   (m_aSuspensionSpringRatioPrev[0] < 1.0f && m_aSuspensionSpringRatioPrev[1] < 1.0f &&
		    m_aSuspensionSpringRatioPrev[2] < 1.0f && m_aSuspensionSpringRatioPrev[3] < 1.0f)){
			m_vecMoveSpeed = CVector(0.0f, 0.0f, 0.0f);
			m_vecTurnSpeed.z = 0.0f;
		}
	}

	if(IsRealHeli() && bHeliDestroyed && !bRenderScorched){
		ApplyMoveForce(0.0f, 0.0f, -2.0f*CTimer::GetTimeStep());
		m_vecTurnSpeed.z += -0.002f*CTimer::GetTimeStep();
		m_vecTurnSpeed.x += -0.0002f*CTimer::GetTimeStep();

		RwRGBA col = { 84, 84, 84, 255 };
		CParticle::AddParticle(PARTICLE_ENGINE_SMOKE, GetMatrix()*CVector(0.0f, 0.0f, -10.0f),
			CVector(0.0f, 0.0f, 0.0f), nil, 0.7f, col, 0, 0, 0, 3000);

		if(CWorld::TestSphereAgainstWorld(GetPosition(), 10.0f, this, true, false, false, false, false, false) ||
		   GetPosition().z < 6.0f)
			if(!bRenderScorched){	// we already know this is true...
				CExplosion::AddExplosion(this, nil, EXPLOSION_CAR, GetPosition(), 0);
				bRenderScorched = true;
			}
	}
}

void
CAutomobile::Teleport(CVector pos)
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
CAutomobile::PreRender(void)
{
	int i, j, n;
	CVehicleModelInfo *mi = (CVehicleModelInfo*)CModelInfo::GetModelInfo(GetModelIndex());

	if(GetModelIndex() == MI_RHINO && m_aCarNodes[CAR_BONNET]){
		// Rotate Rhino turret
		CMatrix m;
		CVector p;
		m.Attach(RwFrameGetMatrix(m_aCarNodes[CAR_BONNET]));
		p = m.GetPosition();
		m.SetRotateZ(m_fCarGunLR);
		m.Translate(p);
		m.UpdateRW();
	}

	if(GetModelIndex() == MI_RCBANDIT){
		CVector pos = GetMatrix() * CVector(0.218f, -0.444f, 0.391f);
		CAntennas::RegisterOne((uintptr)this, GetUp(), pos, 1.0f);
	}

	float fwdSpeed = DotProduct(m_vecMoveSpeed, GetForward())*180.0f;


	// Wheel particles

	if(GetModelIndex() == MI_DODO || GetVehicleAppearance() != VEHICLE_APPEARANCE_CAR){
		; // nothing
	}else if(GetModelIndex() == MI_RCBANDIT){
		for(i = 0; i < 4; i++){
			// Game has same code three times here
			switch(m_aWheelState[i]){
			case WHEEL_STATE_SPINNING:
			case WHEEL_STATE_SKIDDING:
			case WHEEL_STATE_FIXED:
				CParticle::AddParticle(PARTICLE_RUBBER_SMOKE,
					m_aWheelColPoints[i].point + CVector(0.0f, 0.0f, 0.05f),
					CVector(0.0f, 0.0f, 0.0f), nil, 0.1f);
				break;
			default: break;
			}
		}
	}else{
		if(GetStatus() == STATUS_SIMPLE){
			CMatrix mat;
			CVector pos;

			mat.Attach(RwFrameGetMatrix(m_aCarNodes[CAR_WHEEL_RB]));
			pos = mat.GetPosition();
			pos.z = 1.5f*m_aWheelPosition[CARWHEEL_REAR_RIGHT];
			m_aWheelColPoints[CARWHEEL_REAR_RIGHT].point = GetMatrix() * pos;
			m_aWheelColPoints[CARWHEEL_REAR_RIGHT].surfaceB = SURFACE_DEFAULT;

			mat.Attach(RwFrameGetMatrix(m_aCarNodes[CAR_WHEEL_LB]));
			pos = mat.GetPosition();
			pos.z = 1.5f*m_aWheelPosition[CARWHEEL_REAR_LEFT];
			m_aWheelColPoints[CARWHEEL_REAR_LEFT].point = GetMatrix() * pos;
			m_aWheelColPoints[CARWHEEL_REAR_LEFT].surfaceB = SURFACE_DEFAULT;

			mat.Attach(RwFrameGetMatrix(m_aCarNodes[CAR_WHEEL_RF]));
			pos = mat.GetPosition();
			pos.z = 1.5f*m_aWheelPosition[CARWHEEL_FRONT_RIGHT];
			m_aWheelColPoints[CARWHEEL_FRONT_RIGHT].point = GetMatrix() * pos;
			m_aWheelColPoints[CARWHEEL_FRONT_RIGHT].surfaceB = SURFACE_DEFAULT;

			mat.Attach(RwFrameGetMatrix(m_aCarNodes[CAR_WHEEL_LF]));
			pos = mat.GetPosition();
			pos.z = 1.5f*m_aWheelPosition[CARWHEEL_FRONT_LEFT];
			m_aWheelColPoints[CARWHEEL_FRONT_LEFT].point = GetMatrix() * pos;
			m_aWheelColPoints[CARWHEEL_FRONT_LEFT].surfaceB = SURFACE_DEFAULT;
		}

		int drawParticles = Abs(fwdSpeed) < 90.0f;
		if(GetStatus() == STATUS_SIMPLE || GetStatus() == STATUS_PHYSICS ||
		   GetStatus() == STATUS_PLAYER || GetStatus() == STATUS_PLAYER_PLAYBACKFROMBUFFER){
			bool rearSkidding = false;
			if(m_aWheelState[CARWHEEL_REAR_LEFT] == WHEEL_STATE_SKIDDING ||
			   m_aWheelState[CARWHEEL_REAR_RIGHT] == WHEEL_STATE_SKIDDING)
				rearSkidding = true;

			for(i = 0; i < 4; i++){
				if(m_aSuspensionSpringRatioPrev[i] < 1.0f && m_aWheelColPoints[i].surfaceB != SURFACE_WATER)
				switch(m_aWheelState[i]){
				case WHEEL_STATE_SPINNING:
					if(AddWheelDirtAndWater(&m_aWheelColPoints[i], drawParticles)){
						CParticle::AddParticle(PARTICLE_BURNINGRUBBER_SMOKE,
							m_aWheelColPoints[i].point + CVector(0.0f, 0.0f, 0.25f),
							CVector(0.0f, 0.0f, 0.0f));

						CParticle::AddParticle(PARTICLE_BURNINGRUBBER_SMOKE,
							m_aWheelColPoints[i].point + CVector(0.0f, 0.0f, 0.25f),
							CVector(0.0f, 0.0f, 0.05f));
					}

					CParticle::AddParticle(PARTICLE_RUBBER_SMOKE,
						m_aWheelColPoints[i].point + CVector(0.0f, 0.0f, 0.25f),
						CVector(0.0f, 0.0f, 0.0f));

					if(m_aWheelTimer[i] > 0.0f)
						CSkidmarks::RegisterOne((uintptr)this + i, m_aWheelColPoints[i].point,
							GetForward().x, GetForward().y,
							m_aWheelSkidmarkType[i], &m_aWheelSkidmarkBloody[i]);
					break;

				case WHEEL_STATE_SKIDDING:
					if(i == CARWHEEL_REAR_LEFT || i == CARWHEEL_REAR_RIGHT || rearSkidding){
						// same as below

						if(Abs(fwdSpeed) > 5.0f){
							AddWheelDirtAndWater(&m_aWheelColPoints[i], drawParticles);

							CParticle::AddParticle(PARTICLE_RUBBER_SMOKE,
								m_aWheelColPoints[i].point + CVector(0.0f, 0.0f, 0.25f),
								CVector(0.0f, 0.0f, 0.0f));
						}

						if(m_aWheelTimer[i] > 0.0f)
							CSkidmarks::RegisterOne((uintptr)this + i, m_aWheelColPoints[i].point,
								GetForward().x, GetForward().y,
								m_aWheelSkidmarkType[i], &m_aWheelSkidmarkBloody[i]);
					}
					break;

				case WHEEL_STATE_FIXED:
					if(Abs(fwdSpeed) > 5.0f){
						AddWheelDirtAndWater(&m_aWheelColPoints[i], drawParticles);

						CParticle::AddParticle(PARTICLE_RUBBER_SMOKE,
							m_aWheelColPoints[i].point + CVector(0.0f, 0.0f, 0.25f),
							CVector(0.0f, 0.0f, 0.0f));
					}

					if(m_aWheelTimer[i] > 0.0f)
						CSkidmarks::RegisterOne((uintptr)this + i, m_aWheelColPoints[i].point,
							GetForward().x, GetForward().y,
							m_aWheelSkidmarkType[i], &m_aWheelSkidmarkBloody[i]);
					break;

				default:
					if(Abs(fwdSpeed) > 0.5f)
						AddWheelDirtAndWater(&m_aWheelColPoints[i], drawParticles);
					if((m_aWheelSkidmarkBloody[i] || m_aWheelSkidmarkUnk[i]) && m_aWheelTimer[i] > 0.0f)
						CSkidmarks::RegisterOne((uintptr)this + i, m_aWheelColPoints[i].point,
							GetForward().x, GetForward().y,
							m_aWheelSkidmarkType[i], &m_aWheelSkidmarkBloody[i]);
				}

				// Sparks for friction of burst wheels
				if(Damage.GetWheelStatus(i) == WHEEL_STATUS_BURST && m_aSuspensionSpringRatioPrev[i] < 1.0f){
					static float speedSq;
					speedSq = m_vecMoveSpeed.MagnitudeSqr();
					if(speedSq > SQR(0.1f) &&
					   m_aWheelColPoints[i].surfaceB != SURFACE_GRASS &&
					   m_aWheelColPoints[i].surfaceB != SURFACE_MUD_DRY &&
					   m_aWheelColPoints[i].surfaceB != SURFACE_SAND &&
					   m_aWheelColPoints[i].surfaceB != SURFACE_SAND_BEACH &&
					   m_aWheelColPoints[i].surfaceB != SURFACE_WATER){
						CVector normalSpeed = m_aWheelColPoints[i].normal * DotProduct(m_aWheelColPoints[i].normal, m_vecMoveSpeed);
						CVector frictionSpeed = m_vecMoveSpeed - normalSpeed;
						if(i == CARWHEEL_FRONT_LEFT || i == CARWHEEL_REAR_LEFT)
							frictionSpeed -= 0.05f*GetRight();
						else
							frictionSpeed += 0.05f*GetRight();
						CVector unusedRight = 0.15f*GetRight();
						CVector sparkDir = 0.25f*frictionSpeed;
						CParticle::AddParticle(PARTICLE_SPARK_SMALL, m_aWheelColPoints[i].point, sparkDir);

						if(speedSq > 0.04f)
							CParticle::AddParticle(PARTICLE_SPARK_SMALL, m_aWheelColPoints[i].point, sparkDir);
						if(speedSq > 0.16f){
							CParticle::AddParticle(PARTICLE_SPARK_SMALL, m_aWheelColPoints[i].point, sparkDir);
							CParticle::AddParticle(PARTICLE_SPARK_SMALL, m_aWheelColPoints[i].point, sparkDir);
						}
					}
				}
			}
		}
	}

	if(m_aCarNodes[CAR_WHEEL_RM]){
		// assume middle wheels are two units before rear ones
		CVector offset = GetForward()*2.0f;

		switch(m_aWheelState[CARWHEEL_REAR_LEFT]){
		// Game has same code three times here
		case WHEEL_STATE_SPINNING:
		case WHEEL_STATE_SKIDDING:
		case WHEEL_STATE_FIXED:
			CParticle::AddParticle(PARTICLE_RUBBER_SMOKE,
				m_aWheelColPoints[CARWHEEL_REAR_LEFT].point + CVector(0.0f, 0.0f, 0.25f) + offset,
				CVector(0.0f, 0.0f, 0.0f));

			if(m_aWheelTimer[CARWHEEL_REAR_LEFT] > 0.0f)
				CSkidmarks::RegisterOne((uintptr)this + CARWHEEL_REAR_LEFT,
					m_aWheelColPoints[CARWHEEL_REAR_LEFT].point + offset,
					GetForward().x, GetForward().y,
					m_aWheelSkidmarkType[CARWHEEL_REAR_LEFT], &m_aWheelSkidmarkBloody[CARWHEEL_REAR_LEFT]);
			break;
		default: break;
		}

		switch(m_aWheelState[CARWHEEL_REAR_RIGHT]){
		// Game has same code three times here
		case WHEEL_STATE_SPINNING:
		case WHEEL_STATE_SKIDDING:
		case WHEEL_STATE_FIXED:
			CParticle::AddParticle(PARTICLE_RUBBER_SMOKE,
				m_aWheelColPoints[CARWHEEL_REAR_RIGHT].point + CVector(0.0f, 0.0f, 0.25f) + offset,
				CVector(0.0f, 0.0f, 0.0f));

			if(m_aWheelTimer[CARWHEEL_REAR_RIGHT] > 0.0f)
				CSkidmarks::RegisterOne((uintptr)this + CARWHEEL_REAR_RIGHT,
					m_aWheelColPoints[CARWHEEL_REAR_RIGHT].point + offset,
					GetForward().x, GetForward().y,
					m_aWheelSkidmarkType[CARWHEEL_REAR_RIGHT], &m_aWheelSkidmarkBloody[CARWHEEL_REAR_RIGHT]);
			break;
			default: break;
		}
	}


	// Rain on roof
	if(!CCullZones::CamNoRain() && !CCullZones::PlayerNoRain() &&
	   Abs(fwdSpeed) < 20.0f && CWeather::Rain > 0.02f){
		CColModel *colModel = GetColModel();

		for(i = 0; i < colModel->numTriangles; i++){
			CVector p1, p2, p3, c;

			colModel->GetTrianglePoint(p1, colModel->triangles[i].a);
			p1 = GetMatrix() * p1;
			colModel->GetTrianglePoint(p2, colModel->triangles[i].b);
			p2 = GetMatrix() * p2;
			colModel->GetTrianglePoint(p3, colModel->triangles[i].c);
			p3 = GetMatrix() * p3;
			c = (p1 + p2 + p3)/3.0f;

			n = 6.0f*CWeather::Rain;
			for(j = 0; j <= n; j++)
				CParticle::AddParticle(PARTICLE_RAIN_SPLASHUP,
					c + CVector(CGeneral::GetRandomNumberInRange(-0.4f, 0.4f), CGeneral::GetRandomNumberInRange(-0.4f, 0.4f), 0.0f),
					CVector(0.0f, 0.0f, 0.0f),
					nil, 0.0f, 0, 0, CGeneral::GetRandomNumber() & 1);
		}
	}

	AddDamagedVehicleParticles();

	// Exhaust smoke
	if(bEngineOn && !(pHandling->Flags & HANDLING_NO_EXHAUST) && fwdSpeed < 130.0f){
		CVector exhaustPos = mi->m_positions[CAR_POS_EXHAUST];
		CVector pos1, pos2, dir1, dir2;

		if(exhaustPos != CVector(0.0f, 0.0f, 0.0f)){
			dir1.z = 0.0f;
			dir2.z = 0.0f;
			if(fwdSpeed < 10.0f){
				CVector steerFwd(-Sin(m_fSteerAngle), Cos(m_fSteerAngle), 0.0f);
				steerFwd = Multiply3x3(GetMatrix(), steerFwd);
				float r = CGeneral::GetRandomNumberInRange(-0.06f, -0.03f);
				dir1.x = steerFwd.x * r;
				dir1.y = steerFwd.y * r;
			}else{
				dir1.x = m_vecMoveSpeed.x;
				dir1.y = m_vecMoveSpeed.y;
			}

			bool dblExhaust = false;
			pos1 = GetMatrix() * exhaustPos;
			if(pHandling->Flags & HANDLING_DBL_EXHAUST){
				dblExhaust = true;
				pos2 = exhaustPos;
				pos2.x = -pos2.x;
				pos2 = GetMatrix() * pos2;
				dir2 = dir1;
			}

			static float fumesLimit = 2.0f;
			if(CGeneral::GetRandomNumberInRange(1.0f, 3.0f)*(m_fGasPedal+1.1f) > fumesLimit)
				for(i = 0; i < 4;){
					CParticle::AddParticle(PARTICLE_EXHAUST_FUMES, pos1, dir1);
					if(pHandling->Flags & HANDLING_DBL_EXHAUST)
						CParticle::AddParticle(PARTICLE_EXHAUST_FUMES, pos2, dir2);

					static float extraFumesLimit = 0.5f;
					if(m_fGasPedal > extraFumesLimit && m_nCurrentGear < 3){
						if(CGeneral::GetRandomNumber() & 1)
							CParticle::AddParticle(PARTICLE_EXHAUST_FUMES, pos1, dir1);
						else if(pHandling->Flags & HANDLING_DBL_EXHAUST)
							CParticle::AddParticle(PARTICLE_EXHAUST_FUMES, pos2, dir2);
					}

					// Fire on Cuban hermes
					if(GetModelIndex() == MI_CUBAN && i == 1 && m_fGasPedal > 0.9f){
						if(m_nCurrentGear == 1 || m_nCurrentGear == 3 && (CTimer::GetTimeInMilliseconds()%1500) > 750){
							if(CGeneral::GetRandomNumber() & 1){
								CParticle::AddParticle(PARTICLE_FIREBALL, pos1, dir1, nil, 0.05f, 0, 0, 2, 200);
								CParticle::AddParticle(PARTICLE_FIREBALL, pos1, dir1, nil, 0.05f, 0, 0, 2, 200);
							}else{
								CParticle::AddParticle(PARTICLE_FIREBALL, pos2, dir2, nil, 0.05f, 0, 0, 2, 200);
								CParticle::AddParticle(PARTICLE_FIREBALL, pos2, dir2, nil, 0.05f, 0, 0, 2, 200);
							}
						}
					}

					if(GetStatus() == STATUS_PLAYER && (CTimer::GetFrameCounter()&3) == 0 &&
					   CWeather::Rain == 0.0f && i == 0){
						CVector camDist = GetPosition() - TheCamera.GetPosition();
						if(DotProduct(GetForward(), camDist) > 0.0f ||
						   TheCamera.GetLookDirection() == LOOKING_LEFT ||
						   TheCamera.GetLookDirection() == LOOKING_RIGHT){
							CParticle::AddParticle(PARTICLE_HEATHAZE, pos1, CVector(0.0f, 0.0f, 0.0f));
							if(pHandling->Flags & HANDLING_DBL_EXHAUST)
								CParticle::AddParticle(PARTICLE_HEATHAZE, pos2, CVector(0.0f, 0.0f, 0.0f));

							CParticle::AddParticle(PARTICLE_HEATHAZE, pos1, CVector(0.0f, 0.0f, 0.0f));
							if(pHandling->Flags & HANDLING_DBL_EXHAUST)
								CParticle::AddParticle(PARTICLE_HEATHAZE, pos2, CVector(0.0f, 0.0f, 0.0f));
						}
					}

					if(GetModelIndex() == MI_CUBAN && i < 1){
						i = 1;
						pos1 = GetMatrix() * CVector(1.134f, -1.276f, -0.56f);
						pos2 = GetMatrix() * CVector(-1.134f, -1.276f, -0.56f);
						dir1 += 0.05f*GetRight();
						dir2 -= 0.05f*GetRight();
					}else
						i = 99;
				}
		}
	}


	// Siren and taxi lights
	switch(GetModelIndex()){
	case MI_FIRETRUCK:
	case MI_AMBULAN:
	case MI_POLICE:
	case MI_ENFORCER:
		if(m_bSirenOrAlarm){
			CVector pos1, pos2;
			uint8 r1, g1, b1;
			uint8 r2, g2, b2;
			uint8 r, g, b;

			switch(GetModelIndex()){
			case MI_FIRETRUCK:
				pos1 = CVector(1.1f,  1.7f, 2.0f);
				pos2 = CVector(-1.1f, 1.7f, 2.0f);
				r1 = 255; g1 = 0; b1 = 0;
				r2 = 255; g2 = 255; b2 = 0;
				break;
			case MI_AMBULAN:
				pos1 = CVector(1.1f,  0.9f, 1.6f);
				pos2 = CVector(-1.1f, 0.9f, 1.6f);
				r1 = 255; g1 = 0; b1 = 0;
				r2 = 255; g2 = 255; b2 = 255;
				break;
			case MI_POLICE:
				pos1 = CVector(0.7f,  -0.4f, 1.0f);
				pos2 = CVector(-0.7f, -0.4f, 1.0f);
				r1 = 255; g1 = 0; b1 = 0;
				r2 = 0; g2 = 0; b2 = 255;
				break;
			case MI_ENFORCER:
				pos1 = CVector(1.1f,  0.8f, 1.2f);
				pos2 = CVector(-1.1f, 0.8f, 1.2f);
				r1 = 255; g1 = 0; b1 = 0;
				r2 = 0; g2 = 0; b2 = 255;
				break;
			}

			uint32 t = CTimer::GetTimeInMilliseconds() & 0x3FF; // 1023
			if(t < 512){
				r = r1/6;
				g = g1/6;
				b = b1/6;
			}else{
				r = r2/6;
				g = g2/6;
				b = b2/6;
			}

			t = CTimer::GetTimeInMilliseconds() & 0x1FF; // 511
			if(t < 100){
				float f = t/100.0f;
				r *= f;
				g *= f;
				b *= f;
			}else if(t > 412){
				float f = (512-t)/100.0f;
				r *= f;
				g *= f;
				b *= f;
			}

			CVector pos = GetPosition();
			float angle = (CTimer::GetTimeInMilliseconds() & 0x3FF)*TWOPI/0x3FF;
			float s = 8.0f*Sin(angle);
			float c = 8.0f*Cos(angle);
			//CShadows::StoreCarLightShadow(this, (uintptr)this + 21, gpShadowHeadLightsTex,
			//	&pos, c, s, s, -c, r, g, b, 8.0f);

			CPointLights::AddLight(CPointLights::LIGHT_POINT,
				pos + GetUp()*2.0f, CVector(0.0f, 0.0f, 0.0f), 12.0f,
				r*0.02f, g*0.02f, b*0.02f, CPointLights::FOG_NONE, true);

			pos1 = GetMatrix() * pos1;
			pos2 = GetMatrix() * pos2;

			for(i = 0; i < 4; i++){
				uint8 sirenTimer = ((CTimer::GetTimeInMilliseconds() + (i<<6))>>8) & 3;
				pos = (pos1*i + pos2*(3.0f-i))/3.0f;

				switch(sirenTimer){
				case 0:
					CCoronas::RegisterCorona((uintptr)this + 21 + i,
						r1, g1, b1, 255,
						pos, 0.4f, 50.0f,
						CCoronas::TYPE_STAR,
						i == 1 ? CCoronas::FLARE_HEADLIGHTS : CCoronas::FLARE_NONE,
						CCoronas::REFLECTION_OFF, CCoronas::LOSCHECK_OFF, CCoronas::STREAK_OFF, 0.0f);
					break;
				case 2:
					CCoronas::RegisterCorona((uintptr)this + 21 + i,
						r2, g2, b2, 255,
						pos, 0.4f, 50.0f,
						CCoronas::TYPE_STAR,
						CCoronas::FLARE_NONE,
						CCoronas::REFLECTION_OFF, CCoronas::LOSCHECK_OFF, CCoronas::STREAK_OFF, 0.0f);
					break;
				default:
					CCoronas::UpdateCoronaCoors((uintptr)this + 21 + i, pos, 50.0f, 0.0f);
					break;
				}
			}
		}
		break;

	case MI_FBIRANCH:
	case MI_VICECHEE:
		if(m_bSirenOrAlarm){
			CVector pos = GetMatrix() * CVector(0.4f, 0.6f, 0.3f);
			if(CTimer::GetTimeInMilliseconds() & 0x100 &&
			   DotProduct(GetForward(), GetPosition() - TheCamera.GetPosition()) < 0.0f)
				if(GetModelIndex() == MI_VICECHEE)
					CCoronas::RegisterCorona((uintptr)this + 21,
						255, 70, 70, 255,
						pos, 0.4f, 50.0f,
						CCoronas::TYPE_STAR,
						CCoronas::FLARE_NONE,
						CCoronas::REFLECTION_OFF, CCoronas::LOSCHECK_OFF, CCoronas::STREAK_OFF, 0.0f);
				else
					CCoronas::RegisterCorona((uintptr)this + 21,
						0, 0, 255, 255,
						pos, 0.4f, 50.0f,
						CCoronas::TYPE_STAR,
						CCoronas::FLARE_NONE,
						CCoronas::REFLECTION_OFF, CCoronas::LOSCHECK_OFF, CCoronas::STREAK_OFF, 0.0f);
			else
				CCoronas::UpdateCoronaCoors((uintptr)this + 21, pos, 50.0f, 0.0f);
		}
		break;

	case MI_TAXI:
	case MI_CABBIE:
	case MI_ZEBRA:
	case MI_KAUFMAN:
		if(bTaxiLight){
			CVector pos = GetPosition() + GetUp()*0.95f;
			CCoronas::RegisterCorona((uintptr)this + 21,
				128, 128, 0, 255,
				pos, 0.8f, 50.0f,
				CCoronas::TYPE_NORMAL,
				CCoronas::FLARE_NONE,
				CCoronas::REFLECTION_ON, CCoronas::LOSCHECK_OFF, CCoronas::STREAK_OFF, 0.0f);
			CPointLights::AddLight(CPointLights::LIGHT_POINT,
				pos, CVector(0.0f, 0.0f, 0.0f), 10.0f,
				1.0f, 1.0f, 0.5f, CPointLights::FOG_NONE, true);
		}
		break;
	}

	if(GetModelIndex() != MI_RCBANDIT && GetModelIndex() != MI_DODO &&
	   GetModelIndex() != MI_RHINO && GetModelIndex() != MI_RCBARON &&
	   GetVehicleAppearance() != VEHICLE_APPEARANCE_HELI) {
	// Process lights

	// Turn lights on/off
	bool shouldLightsBeOn = 
		CClock::GetHours() > 20 ||
		CClock::GetHours() > 19 && CClock::GetMinutes() > (m_randomSeed & 0x3F) ||
		CClock::GetHours() < 7 ||
		CClock::GetHours() < 8 && CClock::GetMinutes() < (m_randomSeed & 0x3F) ||
		m_randomSeed/50000.0f < CWeather::Foggyness ||
		m_randomSeed/50000.0f < CWeather::WetRoads;
	if(shouldLightsBeOn != bLightsOn && GetStatus() != STATUS_WRECKED){
		if(GetStatus() == STATUS_ABANDONED){
			// Turn off lights on abandoned vehicles only when we they're far away
			if(bLightsOn &&
			   Abs(TheCamera.GetPosition().x - GetPosition().x) + Abs(TheCamera.GetPosition().y - GetPosition().y) > 100.0f)
				bLightsOn = false;
		}else
			bLightsOn = shouldLightsBeOn;
	}

	// Actually render the lights
	bool alarmOn = false;
	bool alarmOff = false;
	if(IsAlarmOn()){
		if(CTimer::GetTimeInMilliseconds() & 0x100)
			alarmOn = true;
		else
			alarmOff = true;
	}
	if(bEngineOn && bLightsOn || alarmOn || alarmOff){
		CVector lookVector = GetPosition() - TheCamera.GetPosition();
		float camDist = lookVector.Magnitude();
		if(camDist != 0.0f)
			lookVector *= 1.0f/camDist;
		else
			lookVector = CVector(1.0f, 0.0f, 0.0f);

		// 1.0 if directly behind car, -1.0 if in front
		float behindness = DotProduct(lookVector, GetForward());
		behindness = clamp(behindness, -1.0f, 1.0f);	// shouldn't be necessary
		// 0.0 if behind car, PI if in front
		// Abs not necessary
		float angle = Abs(Acos(behindness));

		// Headlights

		CVector headLightPos = mi->m_positions[CAR_POS_HEADLIGHTS];
		CVector lightR = GetMatrix() * headLightPos;
		CVector lightL = lightR;
		lightL -= GetRight()*2.0f*headLightPos.x;

		// Headlight coronas
		if(DotProduct(lightR-TheCamera.GetPosition(), GetForward()) < 0.0f &&
		   (TheCamera.Cams[TheCamera.ActiveCam].Mode != CCam::MODE_1STPERSON || this != FindPlayerVehicle())){
			// In front of car
			float intensity = -0.5f*behindness + 0.3f;
			float size = 1.0f - behindness;

			if(behindness < -0.97f && camDist < 30.0f){
				// Directly in front and not too far away
				if(pHandling->Flags & HANDLING_HALOGEN_LIGHTS){
					if(Damage.GetLightStatus(VEHLIGHT_FRONT_LEFT) == LIGHT_STATUS_OK)
						CCoronas::RegisterCorona((uintptr)this + 6, 150, 150, 195, 255,
							lightL, 1.2f, 45.0f*TheCamera.LODDistMultiplier,
							CCoronas::TYPE_HEADLIGHT, CCoronas::FLARE_NONE, CCoronas::REFLECTION_ON,
							CCoronas::LOSCHECK_OFF, CCoronas::STREAK_OFF, angle);
					if(Damage.GetLightStatus(VEHLIGHT_FRONT_RIGHT) == LIGHT_STATUS_OK)
						CCoronas::RegisterCorona((uintptr)this + 7, 150, 150, 195, 255,
							lightR, 1.2f, 45.0f*TheCamera.LODDistMultiplier,
							CCoronas::TYPE_HEADLIGHT, CCoronas::FLARE_NONE, CCoronas::REFLECTION_ON,
							CCoronas::LOSCHECK_OFF, CCoronas::STREAK_OFF, angle);
				}else{
					if(Damage.GetLightStatus(VEHLIGHT_FRONT_LEFT) == LIGHT_STATUS_OK)
						CCoronas::RegisterCorona((uintptr)this + 6, 160, 160, 140, 255,
							lightL, 1.2f, 45.0f*TheCamera.LODDistMultiplier,
							CCoronas::TYPE_HEADLIGHT, CCoronas::FLARE_NONE, CCoronas::REFLECTION_ON,
							CCoronas::LOSCHECK_OFF, CCoronas::STREAK_OFF, angle);
					if(Damage.GetLightStatus(VEHLIGHT_FRONT_RIGHT) == LIGHT_STATUS_OK)
						CCoronas::RegisterCorona((uintptr)this + 7, 160, 160, 140, 255,
							lightR, 1.2f, 45.0f*TheCamera.LODDistMultiplier,
							CCoronas::TYPE_HEADLIGHT, CCoronas::FLARE_NONE, CCoronas::REFLECTION_ON,
							CCoronas::LOSCHECK_OFF, CCoronas::STREAK_OFF, angle);
				}
			}

			if(alarmOff){
				if(Damage.GetLightStatus(VEHLIGHT_FRONT_LEFT) == LIGHT_STATUS_OK)
					CCoronas::RegisterCorona((uintptr)this, 0, 0, 0, 0,
						lightL, size, 0.0f,
						CCoronas::TYPE_STREAK, CCoronas::FLARE_NONE, CCoronas::REFLECTION_ON,
						CCoronas::LOSCHECK_OFF, CCoronas::STREAK_ON, angle);
				if(Damage.GetLightStatus(VEHLIGHT_FRONT_RIGHT) == LIGHT_STATUS_OK)
					CCoronas::RegisterCorona((uintptr)this + 1, 0, 0, 0, 0,
						lightR, size, 0.0f,
						CCoronas::TYPE_STREAK, CCoronas::FLARE_NONE, CCoronas::REFLECTION_ON,
						CCoronas::LOSCHECK_OFF, CCoronas::STREAK_ON, angle);
			}else{
				if(pHandling->Flags & HANDLING_HALOGEN_LIGHTS){
					if(Damage.GetLightStatus(VEHLIGHT_FRONT_LEFT) == LIGHT_STATUS_OK)
						CCoronas::RegisterCorona((uintptr)this, 190*intensity, 190*intensity, 255*intensity, 255,
							lightL, size, 50.0f*TheCamera.LODDistMultiplier,
							CCoronas::TYPE_STREAK, CCoronas::FLARE_NONE, CCoronas::REFLECTION_ON,
							CCoronas::LOSCHECK_OFF, CCoronas::STREAK_ON, angle);
					if(Damage.GetLightStatus(VEHLIGHT_FRONT_RIGHT) == LIGHT_STATUS_OK)
						CCoronas::RegisterCorona((uintptr)this + 1, 190*intensity, 190*intensity, 255*intensity, 255,
							lightR, size, 50.0f*TheCamera.LODDistMultiplier,
							CCoronas::TYPE_STREAK, CCoronas::FLARE_NONE, CCoronas::REFLECTION_ON,
							CCoronas::LOSCHECK_OFF, CCoronas::STREAK_ON, angle);
				}else{
					if(Damage.GetLightStatus(VEHLIGHT_FRONT_LEFT) == LIGHT_STATUS_OK)
						CCoronas::RegisterCorona((uintptr)this, 210*intensity, 210*intensity, 195*intensity, 255,
							lightL, size, 50.0f*TheCamera.LODDistMultiplier,
							CCoronas::TYPE_STREAK, CCoronas::FLARE_NONE, CCoronas::REFLECTION_ON,
							CCoronas::LOSCHECK_OFF, CCoronas::STREAK_ON, angle);
					if(Damage.GetLightStatus(VEHLIGHT_FRONT_RIGHT) == LIGHT_STATUS_OK)
						CCoronas::RegisterCorona((uintptr)this + 1, 210*intensity, 210*intensity, 195*intensity, 255,
							lightR, size, 50.0f*TheCamera.LODDistMultiplier,
							CCoronas::TYPE_STREAK, CCoronas::FLARE_NONE, CCoronas::REFLECTION_ON,
							CCoronas::LOSCHECK_OFF, CCoronas::STREAK_ON, angle);
				}
			}
		}else{
			// Behind car
			if(Damage.GetLightStatus(VEHLIGHT_FRONT_LEFT) == LIGHT_STATUS_OK)
				CCoronas::UpdateCoronaCoors((uintptr)this, lightL, 50.0f*TheCamera.LODDistMultiplier, angle);
			if(Damage.GetLightStatus(VEHLIGHT_FRONT_RIGHT) == LIGHT_STATUS_OK)
				CCoronas::UpdateCoronaCoors((uintptr)this + 1, lightR, 50.0f*TheCamera.LODDistMultiplier, angle);
		}

		// bright lights
		if(Damage.GetLightStatus(VEHLIGHT_FRONT_LEFT) == LIGHT_STATUS_OK && !bNoBrightHeadLights)
			CBrightLights::RegisterOne(lightL, GetUp(), GetRight(), GetForward(), pHandling->FrontLights + BRIGHTLIGHT_FRONT);
		if(Damage.GetLightStatus(VEHLIGHT_FRONT_RIGHT) == LIGHT_STATUS_OK && !bNoBrightHeadLights)
			CBrightLights::RegisterOne(lightR, GetUp(), GetRight(), GetForward(), pHandling->FrontLights + BRIGHTLIGHT_FRONT);

		// Taillights

		CVector tailLightPos = mi->m_positions[CAR_POS_TAILLIGHTS];
		lightR = GetMatrix() * tailLightPos;
		lightL = lightR;
		lightL -= GetRight()*2.0f*tailLightPos.x;

		// Taillight coronas
		if(DotProduct(lightR-TheCamera.GetPosition(), GetForward()) > 0.0f){
			// Behind car
			float intensity = 0.4f*behindness + 0.4f;
			float size = (behindness + 1.0f)/2.0f;

			if(m_fGasPedal < 0.0f){
				// reversing
				intensity += 0.4f;
				size += 0.3f;
				if(Damage.GetLightStatus(VEHLIGHT_REAR_LEFT) == LIGHT_STATUS_OK)
					CCoronas::RegisterCorona((uintptr)this + 2, 128*intensity, 128*intensity, 128*intensity, 255,
						lightL, size, 50.0f*TheCamera.LODDistMultiplier,
						CCoronas::TYPE_STREAK, CCoronas::FLARE_NONE, CCoronas::REFLECTION_ON,
						CCoronas::LOSCHECK_OFF, CCoronas::STREAK_ON, angle);
				if(Damage.GetLightStatus(VEHLIGHT_REAR_RIGHT) == LIGHT_STATUS_OK)
					CCoronas::RegisterCorona((uintptr)this + 3, 128*intensity, 128*intensity, 128*intensity, 255,
						lightR, size, 50.0f*TheCamera.LODDistMultiplier,
						CCoronas::TYPE_STREAK, CCoronas::FLARE_NONE, CCoronas::REFLECTION_ON,
						CCoronas::LOSCHECK_OFF, CCoronas::STREAK_ON, angle);
			}else{
				if(m_fBrakePedal > 0.0f){
					intensity += 0.4f;
					size += 0.3f;
				}

				if(alarmOff){
					if(Damage.GetLightStatus(VEHLIGHT_REAR_LEFT) == LIGHT_STATUS_OK)
						CCoronas::RegisterCorona((uintptr)this + 2, 0, 0, 0, 0,
							lightL, size, 0.0f,
							CCoronas::TYPE_STREAK, CCoronas::FLARE_NONE, CCoronas::REFLECTION_ON,
							CCoronas::LOSCHECK_OFF, CCoronas::STREAK_ON, angle);
					if(Damage.GetLightStatus(VEHLIGHT_REAR_RIGHT) == LIGHT_STATUS_OK)
						CCoronas::RegisterCorona((uintptr)this + 3, 0, 0, 0, 0,
							lightR, size, 0.0f,
							CCoronas::TYPE_STREAK, CCoronas::FLARE_NONE, CCoronas::REFLECTION_ON,
							CCoronas::LOSCHECK_OFF, CCoronas::STREAK_ON, angle);
				}else{
					if(Damage.GetLightStatus(VEHLIGHT_REAR_LEFT) == LIGHT_STATUS_OK)
						CCoronas::RegisterCorona((uintptr)this + 2, 128*intensity, 0, 0, 255,
							lightL, size, 50.0f*TheCamera.LODDistMultiplier,
							CCoronas::TYPE_STREAK, CCoronas::FLARE_NONE, CCoronas::REFLECTION_ON,
							CCoronas::LOSCHECK_OFF, CCoronas::STREAK_ON, angle);
					if(Damage.GetLightStatus(VEHLIGHT_REAR_RIGHT) == LIGHT_STATUS_OK)
						CCoronas::RegisterCorona((uintptr)this + 3, 128*intensity, 0, 0, 255,
							lightR, size, 50.0f*TheCamera.LODDistMultiplier,
							CCoronas::TYPE_STREAK, CCoronas::FLARE_NONE, CCoronas::REFLECTION_ON,
							CCoronas::LOSCHECK_OFF, CCoronas::STREAK_ON, angle);
				}
			}
		}else{
			// In front of car
			if(Damage.GetLightStatus(VEHLIGHT_REAR_LEFT) == LIGHT_STATUS_OK)
				CCoronas::UpdateCoronaCoors((uintptr)this + 2, lightL, 50.0f*TheCamera.LODDistMultiplier, angle);
			if(Damage.GetLightStatus(VEHLIGHT_REAR_RIGHT) == LIGHT_STATUS_OK)
				CCoronas::UpdateCoronaCoors((uintptr)this + 3, lightR, 50.0f*TheCamera.LODDistMultiplier, angle);
		}

		// bright lights
		if(Damage.GetLightStatus(VEHLIGHT_REAR_LEFT) == LIGHT_STATUS_OK)
			CBrightLights::RegisterOne(lightL, GetUp(), GetRight(), GetForward(), pHandling->RearLights + BRIGHTLIGHT_REAR);
		if(Damage.GetLightStatus(VEHLIGHT_REAR_RIGHT) == LIGHT_STATUS_OK)
			CBrightLights::RegisterOne(lightR, GetUp(), GetRight(), GetForward(), pHandling->RearLights + BRIGHTLIGHT_REAR);

		// Light shadows
		if(!alarmOff){
			CVector pos = GetPosition();
			CVector2D fwd(GetForward());
			fwd.Normalise();
			float f = headLightPos.y + 6.0f;
			pos += CVector(f*fwd.x, f*fwd.y, 2.0f);

			if(Damage.GetLightStatus(VEHLIGHT_FRONT_LEFT) == LIGHT_STATUS_OK ||
			   Damage.GetLightStatus(VEHLIGHT_FRONT_RIGHT) == LIGHT_STATUS_OK)
				CShadows::StoreCarLightShadow(this, (uintptr)this + 22, gpShadowHeadLightsTex, &pos,
					7.0f*fwd.x, 7.0f*fwd.y, 5.5f*fwd.y, -5.5f*fwd.x, 45, 45, 45, 7.0f);

			f = (tailLightPos.y - 2.5f) - (headLightPos.y + 6.0f);
			pos += CVector(f*fwd.x, f*fwd.y, 0.0f);
			if(Damage.GetLightStatus(VEHLIGHT_REAR_LEFT) == LIGHT_STATUS_OK ||
			   Damage.GetLightStatus(VEHLIGHT_REAR_RIGHT) == LIGHT_STATUS_OK)
				CShadows::StoreCarLightShadow(this, (uintptr)this + 25, gpShadowExplosionTex, &pos,
					3.0f, 0.0f, 0.0f, -3.0f, 35, 0, 0, 4.0f);
		}

		if(this == FindPlayerVehicle() && !alarmOff){
			if(Damage.GetLightStatus(VEHLIGHT_FRONT_LEFT) == LIGHT_STATUS_OK ||
			   Damage.GetLightStatus(VEHLIGHT_FRONT_RIGHT) == LIGHT_STATUS_OK)
				CPointLights::AddLight(CPointLights::LIGHT_DIRECTIONAL, GetPosition(), GetForward(),
					20.0f, 1.0f, 1.0f, 1.0f,
					FindPlayerVehicle()->m_vecMoveSpeed.MagnitudeSqr2D() < sq(0.45f) ? CPointLights::FOG_NORMAL : CPointLights::FOG_NONE,
					false);
			CVector pos = GetPosition() - 4.0f*GetForward();
			if(Damage.GetLightStatus(VEHLIGHT_REAR_LEFT) == LIGHT_STATUS_OK ||
			   Damage.GetLightStatus(VEHLIGHT_REAR_RIGHT) == LIGHT_STATUS_OK) {
				if(m_fBrakePedal > 0.0f)
					CPointLights::AddLight(CPointLights::LIGHT_POINT, pos, CVector(0.0f, 0.0f, 0.0f),
						10.0f, 1.0f, 0.0f, 0.0f,
						CPointLights::FOG_NONE, false);
				else
					CPointLights::AddLight(CPointLights::LIGHT_POINT, pos, CVector(0.0f, 0.0f, 0.0f),
						7.0f, 0.6f, 0.0f, 0.0f,
						CPointLights::FOG_NONE, false);
			}
		}
	}else if(GetStatus() != STATUS_ABANDONED && GetStatus() != STATUS_WRECKED){
		// Lights off

		CVector lightPos = mi->m_positions[CAR_POS_TAILLIGHTS];
		CVector lightR = GetMatrix() * lightPos;
		CVector lightL = lightR;
		lightL -= GetRight()*2.0f*lightPos.x;

		if(m_fBrakePedal > 0.0f || m_fGasPedal < 0.0f){
			CVector lookVector = GetPosition() - TheCamera.GetPosition();
			lookVector.Normalise();
			float behindness = DotProduct(lookVector, GetForward());
			if(behindness > 0.0f){
				if(m_fGasPedal < 0.0f){
					// reversing
					if(Damage.GetLightStatus(VEHLIGHT_REAR_LEFT) == LIGHT_STATUS_OK)
						CCoronas::RegisterCorona((uintptr)this + 2, 120, 120, 120, 255,
							lightL, 1.2f, 50.0f*TheCamera.LODDistMultiplier,
							CCoronas::TYPE_STAR, CCoronas::FLARE_NONE, CCoronas::REFLECTION_ON,
							CCoronas::LOSCHECK_OFF, CCoronas::STREAK_ON, 0.0f);
					if(Damage.GetLightStatus(VEHLIGHT_REAR_RIGHT) == LIGHT_STATUS_OK)
						CCoronas::RegisterCorona((uintptr)this + 3, 120, 120, 120, 255,
							lightR, 1.2f, 50.0f*TheCamera.LODDistMultiplier,
							CCoronas::TYPE_STAR, CCoronas::FLARE_NONE, CCoronas::REFLECTION_ON,
							CCoronas::LOSCHECK_OFF, CCoronas::STREAK_ON, 0.0f);
					if(Damage.GetLightStatus(VEHLIGHT_REAR_LEFT) == LIGHT_STATUS_OK)
						CBrightLights::RegisterOne(lightL, GetUp(), GetRight(), GetForward(), pHandling->RearLights + BRIGHTLIGHT_FRONT);
					if(Damage.GetLightStatus(VEHLIGHT_REAR_RIGHT) == LIGHT_STATUS_OK)
						CBrightLights::RegisterOne(lightR, GetUp(), GetRight(), GetForward(), pHandling->RearLights + BRIGHTLIGHT_FRONT);
				}else{
					// braking
					if(Damage.GetLightStatus(VEHLIGHT_REAR_LEFT) == LIGHT_STATUS_OK)
						CCoronas::RegisterCorona((uintptr)this + 2, 120, 0, 0, 255,
							lightL, 1.2f, 50.0f*TheCamera.LODDistMultiplier,
							CCoronas::TYPE_STAR, CCoronas::FLARE_NONE, CCoronas::REFLECTION_ON,
							CCoronas::LOSCHECK_OFF, CCoronas::STREAK_ON, 0.0f);
					if(Damage.GetLightStatus(VEHLIGHT_REAR_RIGHT) == LIGHT_STATUS_OK)
						CCoronas::RegisterCorona((uintptr)this + 3, 120, 0, 0, 255,
							lightR, 1.2f, 50.0f*TheCamera.LODDistMultiplier,
							CCoronas::TYPE_STAR, CCoronas::FLARE_NONE, CCoronas::REFLECTION_ON,
							CCoronas::LOSCHECK_OFF, CCoronas::STREAK_ON, 0.0f);
					if(Damage.GetLightStatus(VEHLIGHT_REAR_LEFT) == LIGHT_STATUS_OK)
						CBrightLights::RegisterOne(lightL, GetUp(), GetRight(), GetForward(), pHandling->RearLights + BRIGHTLIGHT_REAR);
					if(Damage.GetLightStatus(VEHLIGHT_REAR_RIGHT) == LIGHT_STATUS_OK)
						CBrightLights::RegisterOne(lightR, GetUp(), GetRight(), GetForward(), pHandling->RearLights + BRIGHTLIGHT_REAR);
				}
			}else{
				if(Damage.GetLightStatus(VEHLIGHT_REAR_LEFT) == LIGHT_STATUS_OK)
					CCoronas::UpdateCoronaCoors((uintptr)this + 2, lightL, 50.0f*TheCamera.LODDistMultiplier, 0.0f);
				if(Damage.GetLightStatus(VEHLIGHT_REAR_RIGHT) == LIGHT_STATUS_OK)
					CCoronas::UpdateCoronaCoors((uintptr)this + 3, lightR, 50.0f*TheCamera.LODDistMultiplier, 0.0f);
			}
		}else{
			if(Damage.GetLightStatus(VEHLIGHT_REAR_LEFT) == LIGHT_STATUS_OK)
				CCoronas::UpdateCoronaCoors((uintptr)this + 2, lightL, 50.0f*TheCamera.LODDistMultiplier, 0.0f);
			if(Damage.GetLightStatus(VEHLIGHT_REAR_RIGHT) == LIGHT_STATUS_OK)
				CCoronas::UpdateCoronaCoors((uintptr)this + 3, lightR, 50.0f*TheCamera.LODDistMultiplier, 0.0f);
		}
	}
	// end of lights
	}

//TODO(MIAMI): StoreShadowForVehicle once we have it
	CShadows::StoreShadowForCar(this);

	DoSunGlare();

	// Heli dust
	if(IsRealHeli() && m_aWheelSpeed[1] > 0.1125f && GetPosition().z < 30.0f){
		bool foundGround = false;
		float waterZ = -1000.0f;
		float groundZ = CWorld::FindGroundZFor3DCoord(GetPosition().x, GetPosition().y, GetPosition().z, &foundGround);
		if(!CWaterLevel::GetWaterLevel(GetPosition(), &waterZ, false))
			waterZ = 0.0f;
		groundZ = Max(groundZ, waterZ);
		float rnd = (m_aWheelSpeed[1]-0.1125f)*((int)Max(16.0f-4.0f*CTimer::GetTimeStep(),2.0f))*400.0f/43.0f;
		float radius = 10.0f;
		if(GetModelIndex() == MI_RCGOBLIN || GetModelIndex() == MI_RCRAIDER)
			radius = 3.0f;
		if(GetPosition().z - groundZ < radius)
			HeliDustGenerate(this, radius-(GetPosition().z - groundZ), groundZ, ceilf(rnd));
	}

	CMatrix mat;
	CVector pos;

	bool onlyFrontWheels = false;
	if(IsRealHeli()){
		// top rotor
		m_aWheelRotation[1] += m_aWheelSpeed[1]*CTimer::GetTimeStep();
		if(m_aWheelRotation[1] > TWOPI) m_aWheelRotation[1] -= TWOPI;
		// rear rotor
		m_aWheelRotation[3] += m_aWheelSpeed[1]*CTimer::GetTimeStep();
		if(m_aWheelRotation[3] > TWOPI) m_aWheelRotation[3] -= TWOPI;
		onlyFrontWheels = true;
	}

	CVector contactPoints[4];	// relative to model
	CVector contactSpeeds[4];	// speed at contact points
	CVector frontWheelFwd = Multiply3x3(GetMatrix(), CVector(-Sin(m_fSteerAngle), Cos(m_fSteerAngle), 0.0f));
	CVector rearWheelFwd = GetForward();
	for(i = 0; i < 4; i++){
		if (m_aWheelTimer[i] > 0.0f && (!onlyFrontWheels || i == CARWHEEL_FRONT_LEFT || i == CARWHEEL_FRONT_RIGHT)) {
			contactPoints[i] = m_aWheelColPoints[i].point - GetPosition();
			contactSpeeds[i] = GetSpeed(contactPoints[i]);
			if (i == CARWHEEL_FRONT_LEFT || i == CARWHEEL_FRONT_RIGHT)
				m_aWheelSpeed[i] = ProcessWheelRotation(m_aWheelState[i], frontWheelFwd, contactSpeeds[i], 0.5f*mi->m_wheelScale);
			else
				m_aWheelSpeed[i] = ProcessWheelRotation(m_aWheelState[i], rearWheelFwd, contactSpeeds[i], 0.5f*mi->m_wheelScale);
			m_aWheelRotation[i] += m_aWheelSpeed[i];
		}
	}

	RwRGBA hoverParticleCol = { 255, 255, 255, 32 };

	// Rear right wheel
	mat.Attach(RwFrameGetMatrix(m_aCarNodes[CAR_WHEEL_RB]));
	pos = mat.GetPosition();
	pos.z = m_aWheelPosition[CARWHEEL_REAR_RIGHT];
	if(Damage.GetWheelStatus(CARWHEEL_REAR_RIGHT) == WHEEL_STATUS_BURST)
		mat.SetRotate(m_aWheelRotation[CARWHEEL_REAR_RIGHT], 0.0f, 0.3f*Sin(m_aWheelRotation[CARWHEEL_REAR_RIGHT]));
	else
		mat.SetRotateX(m_aWheelRotation[CARWHEEL_REAR_RIGHT]);
	if(GetStatus() == STATUS_PLAYER){
		if(bHoverCheat && m_aSuspensionSpringRatioPrev[CARWHEEL_REAR_RIGHT] < 1.0f &&
		   m_aWheelColPoints[CARWHEEL_REAR_RIGHT].surfaceB == SURFACE_WATER){
			// hovering on water
			mat.RotateY(-HALFPI);
			if((CTimer::GetFrameCounter()+CARWHEEL_REAR_RIGHT) & 1){
				CParticle::AddParticle(PARTICLE_STEAM_NY_SLOWMOTION, m_aWheelColPoints[CARWHEEL_REAR_RIGHT].point,
					0.5f*m_vecMoveSpeed+0.1f*GetRight(), nil, 0.4f, hoverParticleCol);
			}else{
				CParticle::AddParticle(PARTICLE_CAR_SPLASH, m_aWheelColPoints[CARWHEEL_REAR_RIGHT].point,
					0.3f*m_vecMoveSpeed+0.15f*GetRight()+CVector(0.0f, 0.0f, 0.1f), nil, 0.15f, hoverParticleCol,
					CGeneral::GetRandomNumberInRange(0.0f, 90.0f),
					CGeneral::GetRandomNumberInRange(0.0f, 10.0f), 1);
			}
		}else{
			// tilt wheel depending oh how much it presses on ground
			float groundOffset = pos.z + m_fHeightAboveRoad - 0.5f*mi->m_wheelScale;
			if(GetModelIndex() == MI_VOODOO)
				groundOffset *= 0.6f;
			mat.RotateY(Asin(clamp(-groundOffset, -1.0f, 1.0f)));
		}
	}
	if(pHandling->Flags & HANDLING_FAT_REARW)
		mat.Scale(1.15f*mi->m_wheelScale, mi->m_wheelScale, mi->m_wheelScale);
	else
		mat.Scale(mi->m_wheelScale);
	mat.Translate(pos);
	mat.UpdateRW();

	// Rear left wheel
	mat.Attach(RwFrameGetMatrix(m_aCarNodes[CAR_WHEEL_LB]));
	pos = mat.GetPosition();
	pos.z = m_aWheelPosition[CARWHEEL_REAR_LEFT];
	if(Damage.GetWheelStatus(CARWHEEL_REAR_LEFT) == WHEEL_STATUS_BURST)
		mat.SetRotate(-m_aWheelRotation[CARWHEEL_REAR_LEFT], 0.0f, PI+0.3f*Sin(m_aWheelRotation[CARWHEEL_REAR_LEFT]));
	else
		mat.SetRotate(-m_aWheelRotation[CARWHEEL_REAR_LEFT], 0.0f, PI);
	if(GetStatus() == STATUS_PLAYER){
		if(bHoverCheat && m_aSuspensionSpringRatioPrev[CARWHEEL_REAR_LEFT] < 1.0f &&
		   m_aWheelColPoints[CARWHEEL_REAR_LEFT].surfaceB == SURFACE_WATER){
			// hovering on water
			mat.RotateY(HALFPI);
			if((CTimer::GetFrameCounter()+CARWHEEL_REAR_LEFT) & 1){
				CParticle::AddParticle(PARTICLE_STEAM_NY_SLOWMOTION, m_aWheelColPoints[CARWHEEL_REAR_LEFT].point,
					0.5f*m_vecMoveSpeed-0.1f*GetRight(), nil, 0.4f, hoverParticleCol);
			}else{
				CParticle::AddParticle(PARTICLE_CAR_SPLASH, m_aWheelColPoints[CARWHEEL_REAR_LEFT].point,
					0.3f*m_vecMoveSpeed-0.15f*GetRight()+CVector(0.0f, 0.0f, 0.1f), nil, 0.15f, hoverParticleCol,
					CGeneral::GetRandomNumberInRange(0.0f, 90.0f),
					CGeneral::GetRandomNumberInRange(0.0f, 10.0f), 1);
			}
		}else{
			// tilt wheel depending oh how much it presses on ground
			float groundOffset = pos.z + m_fHeightAboveRoad - 0.5f*mi->m_wheelScale;
			if(GetModelIndex() == MI_VOODOO)
				groundOffset *= 0.6f;
			mat.RotateY(Asin(clamp(groundOffset, -1.0f, 1.0f)));
		}
	}
	if(pHandling->Flags & HANDLING_FAT_REARW)
		mat.Scale(1.15f*mi->m_wheelScale, mi->m_wheelScale, mi->m_wheelScale);
	else
		mat.Scale(mi->m_wheelScale);
	mat.Translate(pos);
	mat.UpdateRW();

	// Mid right wheel
	if(m_aCarNodes[CAR_WHEEL_RM]){
		mat.Attach(RwFrameGetMatrix(m_aCarNodes[CAR_WHEEL_RM]));
		pos = mat.GetPosition();
		pos.z = m_aWheelPosition[CARWHEEL_REAR_RIGHT];
		if(Damage.GetWheelStatus(CARWHEEL_REAR_RIGHT) == WHEEL_STATUS_BURST)
			mat.SetRotate(m_aWheelRotation[CARWHEEL_REAR_RIGHT], 0.0f, 0.3f*Sin(m_aWheelRotation[CARWHEEL_REAR_RIGHT]));
		else
			mat.SetRotateX(m_aWheelRotation[CARWHEEL_REAR_RIGHT]);
		if(GetStatus() == STATUS_PLAYER){
			if(bHoverCheat && m_aSuspensionSpringRatioPrev[CARWHEEL_REAR_RIGHT] < 1.0f &&
			   m_aWheelColPoints[CARWHEEL_REAR_RIGHT].surfaceB == SURFACE_WATER){
				// hovering on water
				mat.RotateY(-HALFPI);
			}else{
				// tilt wheel depending oh how much it presses on ground
				float groundOffset = pos.z + m_fHeightAboveRoad - 0.5f*mi->m_wheelScale;
				if(GetModelIndex() == MI_VOODOO)
					groundOffset *= 0.6f;
				mat.RotateY(Asin(clamp(-groundOffset, -1.0f, 1.0f)));
			}
		}
		if(pHandling->Flags & HANDLING_FAT_REARW)
			mat.Scale(1.15f*mi->m_wheelScale, mi->m_wheelScale, mi->m_wheelScale);
		else
			mat.Scale(mi->m_wheelScale);
		mat.Translate(pos);
		mat.UpdateRW();
	}

	// Mid left wheel
	if(m_aCarNodes[CAR_WHEEL_LM]){
		mat.Attach(RwFrameGetMatrix(m_aCarNodes[CAR_WHEEL_LM]));
		pos = mat.GetPosition();
		pos.z = m_aWheelPosition[CARWHEEL_REAR_LEFT];
		if(Damage.GetWheelStatus(CARWHEEL_REAR_LEFT) == WHEEL_STATUS_BURST)
			mat.SetRotate(-m_aWheelRotation[CARWHEEL_REAR_LEFT], 0.0f, PI+0.3f*Sin(m_aWheelRotation[CARWHEEL_REAR_LEFT]));
		else
			mat.SetRotate(-m_aWheelRotation[CARWHEEL_REAR_LEFT], 0.0f, PI);
		if(GetStatus() == STATUS_PLAYER){
			if(bHoverCheat && m_aSuspensionSpringRatioPrev[CARWHEEL_REAR_LEFT] < 1.0f &&
			   m_aWheelColPoints[CARWHEEL_REAR_LEFT].surfaceB == SURFACE_WATER){
				// hovering on water
				mat.RotateY(HALFPI);
			}else{
				// tilt wheel depending oh how much it presses on ground
				float groundOffset = pos.z + m_fHeightAboveRoad - 0.5f*mi->m_wheelScale;
				if(GetModelIndex() == MI_VOODOO)
					groundOffset *= 0.6f;
				mat.RotateY(Asin(clamp(groundOffset, -1.0f, 1.0f)));
			}
		}
		if(pHandling->Flags & HANDLING_FAT_REARW)
			mat.Scale(1.15f*mi->m_wheelScale, mi->m_wheelScale, mi->m_wheelScale);
		else
			mat.Scale(mi->m_wheelScale);
		mat.Translate(pos);
		mat.UpdateRW();
	}

	if(GetModelIndex() == MI_DODO){
		// Front wheel
		mat.Attach(RwFrameGetMatrix(m_aCarNodes[CAR_WHEEL_RF]));
		pos = mat.GetPosition();
		pos.z = m_aWheelPosition[CARWHEEL_FRONT_RIGHT];
		if(Damage.GetWheelStatus(CARWHEEL_FRONT_RIGHT) == WHEEL_STATUS_BURST)
			mat.SetRotate(m_aWheelRotation[CARWHEEL_FRONT_RIGHT], 0.0f, m_fSteerAngle+0.3f*Sin(m_aWheelRotation[CARWHEEL_FRONT_RIGHT]));
		else
			mat.SetRotate(m_aWheelRotation[CARWHEEL_FRONT_RIGHT], 0.0f, m_fSteerAngle);
		mat.Scale(mi->m_wheelScale);
		mat.Translate(pos);
		mat.UpdateRW();

		// Rotate propeller
		if(m_aCarNodes[CAR_WINDSCREEN]){
			mat.Attach(RwFrameGetMatrix(m_aCarNodes[CAR_WINDSCREEN]));
			pos = mat.GetPosition();
			mat.SetRotateY(m_fPropellerRotation);
			mat.Translate(pos);
			mat.UpdateRW();

			m_fPropellerRotation += m_fGasPedal != 0.0f ? TWOPI/13.0f : TWOPI/26.0f;
			if(m_fPropellerRotation > TWOPI)
				m_fPropellerRotation -= TWOPI;
		}

		// Rudder
		if(Damage.GetDoorStatus(DOOR_BOOT) != DOOR_STATUS_MISSING && m_aCarNodes[CAR_BOOT]){
			mat.Attach(RwFrameGetMatrix(m_aCarNodes[CAR_BOOT]));
			pos = mat.GetPosition();
			mat.SetRotate(0.0f, 0.0f, -m_fSteerAngle);
			mat.Rotate(0.0f, Sin(m_fSteerAngle)*DEGTORAD(22.0f), 0.0f);
			mat.Translate(pos);
			mat.UpdateRW();
		}

		ProcessSwingingDoor(CAR_DOOR_LF, DOOR_FRONT_LEFT);
		ProcessSwingingDoor(CAR_DOOR_RF, DOOR_FRONT_RIGHT);
	}else if(GetModelIndex() == MI_RHINO){
		// Front right wheel
		mat.Attach(RwFrameGetMatrix(m_aCarNodes[CAR_WHEEL_RF]));
		pos = mat.GetPosition();
		pos.z = m_aWheelPosition[CARWHEEL_FRONT_RIGHT];
		// no damaged wheels or steering
		mat.SetRotate(m_aWheelRotation[CARWHEEL_FRONT_RIGHT], 0.0f, 0.0f);
		mat.Scale(mi->m_wheelScale);
		mat.Translate(pos);
		mat.UpdateRW();

		// Front left wheel
		mat.Attach(RwFrameGetMatrix(m_aCarNodes[CAR_WHEEL_LF]));
		pos = mat.GetPosition();
		pos.z = m_aWheelPosition[CARWHEEL_FRONT_LEFT];
		// no damaged wheels or steering
		mat.SetRotate(-m_aWheelRotation[CARWHEEL_FRONT_LEFT], 0.0f, PI);
		mat.Scale(mi->m_wheelScale);
		mat.Translate(pos);
		mat.UpdateRW();
	}else if(IsRealHeli()){
		// Top rotor
		if(m_aCarNodes[CAR_BONNET]){
			mat.Attach(RwFrameGetMatrix(m_aCarNodes[CAR_BONNET]));
			pos = mat.GetPosition();
			mat.SetRotateZ(m_aWheelRotation[1]);
			mat.Translate(pos);
			mat.UpdateRW();
		}
		// Blurred top rotor
		if(m_aCarNodes[CAR_WINDSCREEN]){
			mat.Attach(RwFrameGetMatrix(m_aCarNodes[CAR_WINDSCREEN]));
			pos = mat.GetPosition();
			mat.SetRotateZ(-m_aWheelRotation[1]);
			mat.Translate(pos);
			mat.UpdateRW();
		}
		// Rear rotor
		if(m_aCarNodes[CAR_BOOT]){
			mat.Attach(RwFrameGetMatrix(m_aCarNodes[CAR_BOOT]));
			pos = mat.GetPosition();
			mat.SetRotateX(m_aWheelRotation[3]);
			mat.Translate(pos);
			mat.UpdateRW();
		}
		// Blurred rear rotor
		if(m_aCarNodes[CAR_BUMP_REAR]){
			mat.Attach(RwFrameGetMatrix(m_aCarNodes[CAR_BUMP_REAR]));
			pos = mat.GetPosition();
			mat.SetRotateX(-m_aWheelRotation[3]);
			mat.Translate(pos);
			mat.UpdateRW();
		}
	}else{
		// Front right wheel
		mat.Attach(RwFrameGetMatrix(m_aCarNodes[CAR_WHEEL_RF]));
		pos = mat.GetPosition();
		pos.z = m_aWheelPosition[CARWHEEL_FRONT_RIGHT];
		if(Damage.GetWheelStatus(CARWHEEL_FRONT_RIGHT) == WHEEL_STATUS_BURST)
			mat.SetRotate(m_aWheelRotation[CARWHEEL_FRONT_RIGHT], 0.0f, m_fSteerAngle+0.3f*Sin(m_aWheelRotation[CARWHEEL_FRONT_RIGHT]));
		else
			mat.SetRotate(m_aWheelRotation[CARWHEEL_FRONT_RIGHT], 0.0f, m_fSteerAngle);
		if(GetStatus() == STATUS_PLAYER){
			if(bHoverCheat && m_aSuspensionSpringRatioPrev[CARWHEEL_FRONT_RIGHT] < 1.0f &&
			   m_aWheelColPoints[CARWHEEL_FRONT_RIGHT].surfaceB == SURFACE_WATER){
				// hovering on water
				mat.RotateY(-HALFPI);
				if((CTimer::GetFrameCounter()+CARWHEEL_FRONT_RIGHT) & 1){
					CParticle::AddParticle(PARTICLE_STEAM_NY_SLOWMOTION, m_aWheelColPoints[CARWHEEL_FRONT_RIGHT].point,
						0.5f*m_vecMoveSpeed+0.1f*GetRight(), nil, 0.4f, hoverParticleCol);
				}else{
					CParticle::AddParticle(PARTICLE_CAR_SPLASH, m_aWheelColPoints[CARWHEEL_FRONT_RIGHT].point,
						0.3f*m_vecMoveSpeed+0.15f*GetRight()+CVector(0.0f, 0.0f, 0.1f), nil, 0.15f, hoverParticleCol,
						CGeneral::GetRandomNumberInRange(0.0f, 90.0f),
						CGeneral::GetRandomNumberInRange(0.0f, 10.0f), 1);
				}
			}else{
				// tilt wheel depending oh how much it presses on ground
				float groundOffset = pos.z + m_fHeightAboveRoad - 0.5f*mi->m_wheelScale;
				if(GetModelIndex() == MI_VOODOO)
					groundOffset *= 0.6f;
				mat.RotateY(Asin(clamp(-groundOffset, -1.0f, 1.0f)));
			}
		}
		if(pHandling->Flags & HANDLING_NARROW_FRONTW)
			mat.Scale(0.7f*mi->m_wheelScale, mi->m_wheelScale, mi->m_wheelScale);
		else
			mat.Scale(mi->m_wheelScale);
		mat.Translate(pos);
		mat.UpdateRW();

		// Front left wheel
		mat.Attach(RwFrameGetMatrix(m_aCarNodes[CAR_WHEEL_LF]));
		pos = mat.GetPosition();
		pos.z = m_aWheelPosition[CARWHEEL_FRONT_LEFT];
		if(Damage.GetWheelStatus(CARWHEEL_FRONT_LEFT) == WHEEL_STATUS_BURST)
			mat.SetRotate(-m_aWheelRotation[CARWHEEL_FRONT_LEFT], 0.0f, PI+m_fSteerAngle+0.3f*Sin(m_aWheelRotation[CARWHEEL_FRONT_LEFT]));
		else
			mat.SetRotate(-m_aWheelRotation[CARWHEEL_FRONT_LEFT], 0.0f, PI+m_fSteerAngle);
		if(GetStatus() == STATUS_PLAYER){
			if(bHoverCheat && m_aSuspensionSpringRatioPrev[CARWHEEL_FRONT_LEFT] < 1.0f &&
			   m_aWheelColPoints[CARWHEEL_FRONT_LEFT].surfaceB == SURFACE_WATER){
				// hovering on water
				mat.RotateY(HALFPI);
				if((CTimer::GetFrameCounter()+CARWHEEL_FRONT_LEFT) & 1){
					CParticle::AddParticle(PARTICLE_STEAM_NY_SLOWMOTION, m_aWheelColPoints[CARWHEEL_FRONT_LEFT].point,
						0.5f*m_vecMoveSpeed-0.1f*GetRight(), nil, 0.4f, hoverParticleCol);
				}else{
					CParticle::AddParticle(PARTICLE_CAR_SPLASH, m_aWheelColPoints[CARWHEEL_FRONT_LEFT].point,
						0.3f*m_vecMoveSpeed-0.15f*GetRight()+CVector(0.0f, 0.0f, 0.1f), nil, 0.15f, hoverParticleCol,
						CGeneral::GetRandomNumberInRange(0.0f, 90.0f),
						CGeneral::GetRandomNumberInRange(0.0f, 10.0f), 1);
				}
			}else{
				// tilt wheel depending oh how much it presses on ground
				float groundOffset = pos.z + m_fHeightAboveRoad - 0.5f*mi->m_wheelScale;
				if(GetModelIndex() == MI_VOODOO)
					groundOffset *= 0.6f;
				mat.RotateY(Asin(clamp(groundOffset, -1.0f, 1.0f)));
			}
		}
		if(pHandling->Flags & HANDLING_NARROW_FRONTW)
			mat.Scale(0.7f*mi->m_wheelScale, mi->m_wheelScale, mi->m_wheelScale);
		else
			mat.Scale(mi->m_wheelScale);
		mat.Translate(pos);
		mat.UpdateRW();

		ProcessSwingingDoor(CAR_DOOR_LF, DOOR_FRONT_LEFT);
		ProcessSwingingDoor(CAR_DOOR_RF, DOOR_FRONT_RIGHT);
		ProcessSwingingDoor(CAR_DOOR_LR, DOOR_REAR_LEFT);
		ProcessSwingingDoor(CAR_DOOR_RR, DOOR_REAR_RIGHT);
		ProcessSwingingDoor(CAR_BONNET, DOOR_BONNET);
		ProcessSwingingDoor(CAR_BOOT, DOOR_BOOT);
	}

	if((GetModelIndex() == MI_PHEONIX || GetModelIndex() == MI_BFINJECT) &&
	   GetStatus() == STATUS_PLAYER && m_aCarNodes[CAR_WING_LR]){
		float rotation = 0.0f;

		if(GetModelIndex() == MI_BFINJECT)
			if(m_fPropellerRotation > TWOPI) m_fPropellerRotation -= TWOPI;

		if(Abs(m_fGasPedal) > 0.0f){
			if(GetModelIndex() == MI_BFINJECT){
				m_fPropellerRotation += 0.2f*CTimer::GetTimeStep();
				rotation = m_fPropellerRotation;
			}else{
				if(m_fPropellerRotation < 1.3f){
					m_fPropellerRotation = Min(m_fPropellerRotation+0.1f*CTimer::GetTimeStep(), 1.3f);
					rotation = m_fPropellerRotation;
				}else{
					float wave = Sin((CTimer::GetTimeInMilliseconds()%10000)/70.0f);
					rotation = m_fPropellerRotation + 0.13*wave;
				}
			}
		}else{
			if(GetModelIndex() == MI_BFINJECT){
				m_fPropellerRotation += 0.1f*CTimer::GetTimeStep();
				rotation = m_fPropellerRotation;
			}else{
				if(m_fPropellerRotation > 0.0f){
					m_fPropellerRotation = Max(m_fPropellerRotation-0.05f*CTimer::GetTimeStep(), 0.0f);
					rotation = m_fPropellerRotation;
				}
			}
		}

		mat.Attach(RwFrameGetMatrix(m_aCarNodes[CAR_WING_LR]));
		pos = mat.GetPosition();
		if(GetModelIndex() == MI_BFINJECT)
			mat.SetRotateY(rotation);
		else
			mat.SetRotateX(rotation);
		mat.Translate(pos);
		mat.UpdateRW();
	}
}

void
CAutomobile::Render(void)
{
	CVehicleModelInfo *mi = (CVehicleModelInfo*)CModelInfo::GetModelInfo(GetModelIndex());

	mi->SetVehicleColour(m_currentColour1, m_currentColour2);

	if(IsRealHeli()){
		RpAtomic *atomic = nil;
		int rotorAlpha = (1.5f - Min(1.7f*Max(m_aWheelSpeed[1],0.0f)/0.22f, 1.5f))*255.0f;
		rotorAlpha = Min(rotorAlpha, 255);
		int blurAlpha = Max(1.5f*m_aWheelSpeed[1]/0.22f - 0.4f, 0.0f)*150.0f;
		blurAlpha = Min(blurAlpha, 150);

		// Top rotor
		if(m_aCarNodes[CAR_BONNET]){
			RwFrameForAllObjects(m_aCarNodes[CAR_BONNET], GetCurrentAtomicObjectCB, &atomic);
			if(atomic)
				SetComponentAtomicAlpha(atomic, rotorAlpha);
		}
		atomic = nil;
		// Rear rotor
		if(m_aCarNodes[CAR_BOOT]){
			RwFrameForAllObjects(m_aCarNodes[CAR_BOOT], GetCurrentAtomicObjectCB, &atomic);
			if(atomic)
				SetComponentAtomicAlpha(atomic, rotorAlpha);
		}
		atomic = nil;
		// Blurred top rotor
		if(m_aCarNodes[CAR_WINDSCREEN]){
			RwFrameForAllObjects(m_aCarNodes[CAR_WINDSCREEN], GetCurrentAtomicObjectCB, &atomic);
			if(atomic)
				SetComponentAtomicAlpha(atomic, blurAlpha);
		}
		atomic = nil;
		// Blurred rear rotor
		if(m_aCarNodes[CAR_BUMP_REAR]){
			RwFrameForAllObjects(m_aCarNodes[CAR_BUMP_REAR], GetCurrentAtomicObjectCB, &atomic);
			if(atomic)
				SetComponentAtomicAlpha(atomic, blurAlpha);
		}
	}

	if(CVehicle::bWheelsOnlyCheat){
		RpAtomicRender((RpAtomic*)GetFirstObject(m_aCarNodes[CAR_WHEEL_RB]));
		RpAtomicRender((RpAtomic*)GetFirstObject(m_aCarNodes[CAR_WHEEL_LB]));
		RpAtomicRender((RpAtomic*)GetFirstObject(m_aCarNodes[CAR_WHEEL_RF]));
		RpAtomicRender((RpAtomic*)GetFirstObject(m_aCarNodes[CAR_WHEEL_LF]));
		if(m_aCarNodes[CAR_WHEEL_RM])
			RpAtomicRender((RpAtomic*)GetFirstObject(m_aCarNodes[CAR_WHEEL_RM]));
		if(m_aCarNodes[CAR_WHEEL_LM])
			RpAtomicRender((RpAtomic*)GetFirstObject(m_aCarNodes[CAR_WHEEL_LM]));
	}else
		CEntity::Render();
}

int32
CAutomobile::ProcessEntityCollision(CEntity *ent, CColPoint *colpoints)
{
	int i;
	CColModel *colModel;

	if(GetStatus() != STATUS_SIMPLE)
		bVehicleColProcessed = true;

	if(bUsingSpecialColModel)
		colModel = &CWorld::Players[CWorld::PlayerInFocus].m_ColModel;
	else
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
		for(i = 0; i < 4; i++)
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
CAutomobile::ProcessControlInputs(uint8 pad)
{
	float speed = DotProduct(m_vecMoveSpeed, GetForward());

	if(!CPad::GetPad(pad)->GetExitVehicle() ||
	   pDriver && pDriver->m_pVehicleAnim && (pDriver->m_pVehicleAnim->animId == ANIM_CAR_ROLLOUT_LHS ||
	                                          pDriver->m_pVehicleAnim->animId == ANIM_CAR_ROLLOUT_RHS))
		bIsHandbrakeOn = !!CPad::GetPad(pad)->GetHandBrake();
	else
		bIsHandbrakeOn = true;

	// Steer left/right
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
	}else{
		m_fSteerInput += (-CPad::GetPad(pad)->GetSteeringLeftRight()/128.0f - m_fSteerInput)*
			0.2f*CTimer::GetTimeStep();
		nLastControlInput = 0;
	}
	m_fSteerInput = clamp(m_fSteerInput, -1.0f, 1.0f);

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
		int rnd = CGeneral::GetRandomNumber() % 10;
		switch(m_comedyControlState){
		case 0:
			if(rnd < 2)
				m_comedyControlState = 1;
			else if(rnd < 4)
				m_comedyControlState = 2;
			break;
		case 1:
			m_fSteerAngle += 0.05f;
			if(rnd < 2)
				m_comedyControlState = 0;
			break;
		case 2:
			m_fSteerAngle -= 0.05f;
			if(rnd < 2)
				m_comedyControlState = 0;
			break;
		}
	}else
		m_comedyControlState = 0;

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
CAutomobile::FireTruckControl(void)
{
	if(this == FindPlayerVehicle()){
		if(!CPad::GetPad(0)->GetWeapon())
			return;
#ifdef FREE_CAM
		if (!CCamera::bFreeCam)
#endif 
		{
			m_fCarGunLR += CPad::GetPad(0)->GetCarGunLeftRight() * 0.00025f * CTimer::GetTimeStep();
			m_fCarGunUD += CPad::GetPad(0)->GetCarGunUpDown() * 0.0001f * CTimer::GetTimeStep();
		}
		m_fCarGunUD = clamp(m_fCarGunUD, 0.05f, 0.3f);


		CVector cannonPos(0.0f, 1.5f, 1.9f);
		cannonPos = GetMatrix() * cannonPos;
		CVector cannonDir(
			Sin(m_fCarGunLR) * Cos(m_fCarGunUD),
			Cos(m_fCarGunLR) * Cos(m_fCarGunUD),
			Sin(m_fCarGunUD));
		cannonDir = Multiply3x3(GetMatrix(), cannonDir);
		cannonDir.z += (CGeneral::GetRandomNumber()&0xF)/1000.0f;
		CWaterCannons::UpdateOne((uintptr)this, &cannonPos, &cannonDir);
	}else if(GetStatus() == STATUS_PHYSICS){
		CFire *fire = gFireManager.FindFurthestFire_NeverMindFireMen(GetPosition(), 10.0f, 35.0f);
		if(fire == nil)
			return;

		// Target cannon onto fire
		float targetAngle = CGeneral::GetATanOfXY(fire->m_vecPos.x-GetPosition().x, fire->m_vecPos.y-GetPosition().y);
		float fwdAngle = CGeneral::GetATanOfXY(GetForward().x, GetForward().y);
		float targetCannonAngle = fwdAngle - targetAngle;
		float angleDelta = CTimer::GetTimeStep()*0.01f;
		float cannonDelta = targetCannonAngle - m_fCarGunLR;
		while(cannonDelta < PI) cannonDelta += TWOPI;
		while(cannonDelta > PI) cannonDelta -= TWOPI;
		if(Abs(cannonDelta) < angleDelta)
			m_fCarGunLR = targetCannonAngle;
		else if(cannonDelta > 0.0f)
			m_fCarGunLR += angleDelta;
		else
			m_fCarGunLR -= angleDelta;

		// Go up and down a bit
		float upDown = Sin((float)(CTimer::GetTimeInMilliseconds() & 0xFFF)/0x1000 * TWOPI);
		m_fCarGunUD = 0.2f + 0.2f*upDown;

		// Spray water every once in a while
		if((CTimer::GetTimeInMilliseconds()>>10) & 3){
			CVector cannonPos(0.0f, 0.0f, 2.2f);	// different position than player's firetruck!
			cannonPos = GetMatrix() * cannonPos;
			CVector cannonDir(
				Sin(m_fCarGunLR) * Cos(m_fCarGunUD),
				Cos(m_fCarGunLR) * Cos(m_fCarGunUD),
				Sin(m_fCarGunUD));
			cannonDir = Multiply3x3(GetMatrix(), cannonDir);
			cannonDir.z += (CGeneral::GetRandomNumber()&0xF)/1000.0f;
			CWaterCannons::UpdateOne((uintptr)this, &cannonPos, &cannonDir);
		}
	}
}

void
CAutomobile::TankControl(void)
{
	int i;

	// These coords are 1 unit higher then they should be relative to model center
	CVector turrentBase(0.0f, -1.394f, 2.296f);
	CVector gunEnd(0.0f, 1.813f, 2.979f);
	CVector baseToEnd = gunEnd - turrentBase;

	if(this != FindPlayerVehicle())
		return;
	if(CWorld::Players[CWorld::PlayerInFocus].m_WBState != WBSTATE_PLAYING)
		return;

	// Rotate turret
	float prevAngle = m_fCarGunLR;
#ifdef FREE_CAM
	if(!CCamera::bFreeCam)
#endif
		m_fCarGunLR -= CPad::GetPad(0)->GetCarGunLeftRight() * 0.00015f * CTimer::GetTimeStep();

	if(m_fCarGunLR < 0.0f)
		m_fCarGunLR += TWOPI;
	if(m_fCarGunLR > TWOPI)
		m_fCarGunLR -= TWOPI;
	if(m_fCarGunLR != prevAngle)
		DMAudio.PlayOneShot(m_audioEntityId, SOUND_CAR_TANK_TURRET_ROTATE, Abs(m_fCarGunLR - prevAngle));

	// Shoot
	if(CPad::GetPad(0)->CarGunJustDown() &&
	   CTimer::GetTimeInMilliseconds() > CWorld::Players[CWorld::PlayerInFocus].m_nTimeTankShotGun + 800){
		CWorld::Players[CWorld::PlayerInFocus].m_nTimeTankShotGun = CTimer::GetTimeInMilliseconds();

		// more like -sin(angle), cos(angle), i.e. rotated (0,1,0)
		CVector turretDir = CVector(Sin(-m_fCarGunLR), Cos(-m_fCarGunLR), 0.0f);
		turretDir = Multiply3x3(GetMatrix(), turretDir);

		float c = Cos(m_fCarGunLR);
		float s = Sin(m_fCarGunLR);
		CVector rotatedEnd(
			c*baseToEnd.x - s*baseToEnd.y,
			s*baseToEnd.x + c*baseToEnd.y,
			baseToEnd.z - 1.0f);	// correct offset here
		rotatedEnd += turrentBase;

		CVector point1 = GetMatrix() * rotatedEnd;
		CVector point2 = point1 + 60.0f*turretDir;
		m_vecMoveSpeed -= 0.06f*turretDir;
		m_vecMoveSpeed.z += 0.05f;

		CWeapon::DoTankDoomAiming(FindPlayerVehicle(), FindPlayerPed(), &point1, &point2);
		CColPoint colpoint;
		CEntity *entity = nil;
		CWorld::ProcessLineOfSight(point1, point2, colpoint, entity, true, true, true, true, true, true, false);
		if(entity)
			point2 = colpoint.point - 0.04f*(colpoint.point - point1);

		CExplosion::AddExplosion(nil, FindPlayerPed(), EXPLOSION_TANK_GRENADE, point2, 0);

		// Add particles on the way to the explosion;
		float shotDist = (point2 - point1).Magnitude();
		int n = shotDist/4.0f;
		RwRGBA black = { 0, 0, 0, 0 };
		for(i = 0; i < n; i++){
			float f = (float)i/n;
			CParticle::AddParticle(PARTICLE_HELI_DUST,
				point1 + f*(point2 - point1),
				CVector(0.0f, 0.0f, 0.0f),
				nil, 0.1f, black);
		}

		// More particles
		CVector shotDir = point2 - point1;
		shotDir.Normalise();
		for(i = 0; i < 15; i++){
			float f = i/15.0f;
			CParticle::AddParticle(PARTICLE_GUNSMOKE2, point1,
				shotDir*CGeneral::GetRandomNumberInRange(0.3f, 1.0f)*f,
				nil, CGeneral::GetRandomNumberInRange(0.5f, 1.5f)*f, black);
		}

		// And some gun flashes near the gun
		CVector flashPos = point1;
		CVector nullDir(0.0f, 0.0f, 0.0f);
		int lifeSpan = 250;
		if(m_vecMoveSpeed.Magnitude() > 0.08f){
			lifeSpan = 125;
			flashPos.x += 5.0f*m_vecMoveSpeed.x;
			flashPos.y += 5.0f*m_vecMoveSpeed.y;
		}
		CParticle::AddParticle(PARTICLE_GUNFLASH, flashPos, nullDir, nil, 0.4f, black, 0, 0, 0, lifeSpan);
		flashPos += 0.3f*shotDir;
		CParticle::AddParticle(PARTICLE_GUNFLASH, flashPos, nullDir, nil, 0.2f, black, 0, 0, 0, lifeSpan);
		flashPos += 0.1f*shotDir;
		CParticle::AddParticle(PARTICLE_GUNFLASH, flashPos, nullDir, nil, 0.15f, black, 0, 0, 0, lifeSpan);
	}
}

#define HYDRAULIC_UPPER_EXT (-0.16f)
#define HYDRAULIC_LOWER_EXT (0.16f)

void
CAutomobile::HydraulicControl(void)
{
	int i;
	float wheelPositions[4];
	CVehicleModelInfo *mi = (CVehicleModelInfo*)CModelInfo::GetModelInfo(GetModelIndex());
	CColModel *normalColModel = mi->GetColModel();
	float wheelRadius = 0.5f*mi->m_wheelScale;
	CPlayerInfo *playerInfo = &CWorld::Players[CWorld::PlayerInFocus];
	CColModel *specialColModel = &playerInfo->m_ColModel;

	if(GetStatus() != STATUS_PLAYER){
		// reset hydraulics for non-player cars

		if(!bUsingSpecialColModel)
			return;
		if(specialColModel != nil)	// this is always true
			for(i = 0; i < 4; i++)
				wheelPositions[i] = specialColModel->lines[i].p0.z - m_aSuspensionSpringRatio[i]*m_aSuspensionLineLength[i];
		for(i = 0; i < 4; i++){
			m_aSuspensionSpringLength[i] = pHandling->fSuspensionUpperLimit - pHandling->fSuspensionLowerLimit;
			m_aSuspensionLineLength[i] = normalColModel->lines[i].p0.z - normalColModel->lines[i].p1.z;
			m_aSuspensionSpringRatio[i] = (normalColModel->lines[i].p0.z - wheelPositions[i]) / m_aSuspensionLineLength[i];
			if(m_aSuspensionSpringRatio[i] > 1.0f)
				m_aSuspensionSpringRatio[i] = 1.0f;
		}

		if(m_hydraulicState == 0)
			DMAudio.PlayOneShot(m_audioEntityId, SOUND_CAR_HYDRAULIC_1, 0.0f);
		else if(m_hydraulicState >= 100)
			DMAudio.PlayOneShot(m_audioEntityId, SOUND_CAR_HYDRAULIC_2, 0.0f);

		if(playerInfo->m_pVehicleEx == this)
			playerInfo->m_pVehicleEx = nil;
		bUsingSpecialColModel = false;
		m_hydraulicState = 0;
		return;
	}

	// Player car

	float normalUpperLimit = pHandling->fSuspensionUpperLimit;
	float normalLowerLimit = pHandling->fSuspensionLowerLimit;
	float normalSpringLength = normalUpperLimit - normalLowerLimit;
	float extendedUpperLimit = normalUpperLimit - 0.2f;
	float extendedLowerLimit = normalLowerLimit - 0.2f;
	float extendedSpringLength = extendedUpperLimit - extendedLowerLimit;

	if(!bUsingSpecialColModel){
		// Init special col model

		if(playerInfo->m_pVehicleEx && playerInfo->m_pVehicleEx == this)
			playerInfo->m_pVehicleEx->bUsingSpecialColModel = false;
		playerInfo->m_pVehicleEx = this;
		playerInfo->m_ColModel = *normalColModel;
		bUsingSpecialColModel = true;
		specialColModel = &playerInfo->m_ColModel;

		if(m_fVelocityChangeForAudio > 0.1f)
			m_hydraulicState = 20;
		else{
			m_hydraulicState = 0;
			normalUpperLimit += HYDRAULIC_UPPER_EXT;
			normalSpringLength = normalUpperLimit - (normalLowerLimit+HYDRAULIC_LOWER_EXT);
			DMAudio.PlayOneShot(m_audioEntityId, SOUND_CAR_HYDRAULIC_2, 0.0f);
		}

		// Setup suspension
		float normalLineLength = normalSpringLength + wheelRadius;
		CVector pos;
		for(i = 0; i < 4; i++){
			wheelPositions[i] = normalColModel->lines[i].p0.z - m_aSuspensionSpringRatio[i]*m_aSuspensionLineLength[i];
			mi->GetWheelPosn(i, pos);
			pos.z += normalUpperLimit;
			specialColModel->lines[i].p0 = pos;
			pos.z -= normalLineLength;
			specialColModel->lines[i].p1 = pos;
			m_aSuspensionSpringLength[i] = normalSpringLength;
			m_aSuspensionLineLength[i] = normalLineLength;

			if(m_aSuspensionSpringRatio[i] < 1.0f){
				m_aSuspensionSpringRatio[i] = (specialColModel->lines[i].p0.z - wheelPositions[i])/m_aSuspensionLineLength[i];
				if(m_aSuspensionSpringRatio[i] > 1.0f)
					m_aSuspensionSpringRatio[i] = 1.0f;
			}
		}
		DMAudio.PlayOneShot(m_audioEntityId, SOUND_CAR_HYDRAULIC_2, 0.0f);

		// Adjust col model
		mi->GetWheelPosn(0, pos);
		float minz = pos.z + extendedLowerLimit - wheelRadius;
		if(minz < specialColModel->boundingBox.min.z)
			specialColModel->boundingBox.min.z = minz;
		float radius = Max(specialColModel->boundingBox.min.Magnitude(), specialColModel->boundingBox.max.Magnitude());
		if(specialColModel->boundingSphere.radius < radius)
			specialColModel->boundingSphere.radius = radius;
		return;
	}

	if(playerInfo->m_WBState != WBSTATE_PLAYING)
		return;

	bool setPrevRatio = false;
	if(m_hydraulicState < 20 && m_fVelocityChangeForAudio > 0.2f){
		if(m_hydraulicState == 0){
			m_hydraulicState = 20;
			DMAudio.PlayOneShot(m_audioEntityId, SOUND_CAR_HYDRAULIC_1, 0.0f);
			setPrevRatio = true;
		}else{
			m_hydraulicState++;
		}
	}else if(m_hydraulicState != 0){	// must always be true
		if(m_hydraulicState < 21 && m_fVelocityChangeForAudio < 0.1f){
			m_hydraulicState--;
			if(m_hydraulicState == 0)
				DMAudio.PlayOneShot(m_audioEntityId, SOUND_CAR_HYDRAULIC_2, 0.0f);
		}
	}

	if(CPad::GetPad(0)->HornJustDown()){
		// Switch between normal and extended

		if(m_hydraulicState < 100)
			m_hydraulicState = 100;
		else{
			if(m_fVelocityChangeForAudio > 0.1f)
				m_hydraulicState = 20;
			else
				m_hydraulicState = 0;
		}

		if(m_hydraulicState < 100){
			if(m_hydraulicState == 0){
				normalUpperLimit += HYDRAULIC_UPPER_EXT;
				normalLowerLimit += HYDRAULIC_LOWER_EXT;
				normalSpringLength = normalUpperLimit - normalLowerLimit;
			}

			// Reset suspension to normal
			float normalLineLength = normalSpringLength + wheelRadius;
			CVector pos;
			for(i = 0; i < 4; i++){
				wheelPositions[i] = specialColModel->lines[i].p0.z - m_aSuspensionSpringRatio[i]*m_aSuspensionLineLength[i];
				mi->GetWheelPosn(i, pos);
				pos.z += normalUpperLimit;
				specialColModel->lines[i].p0 = pos;
				pos.z -= normalLineLength;
				specialColModel->lines[i].p1 = pos;
				m_aSuspensionSpringLength[i] = normalSpringLength;
				m_aSuspensionLineLength[i] = normalLineLength;

				if(m_aSuspensionSpringRatio[i] < 1.0f){
					m_aSuspensionSpringRatio[i] = (specialColModel->lines[i].p0.z - wheelPositions[i])/m_aSuspensionLineLength[i];
					if(m_aSuspensionSpringRatio[i] > 1.0f)
						m_aSuspensionSpringRatio[i] = 1.0f;
				}
			}
			DMAudio.PlayOneShot(m_audioEntityId, SOUND_CAR_HYDRAULIC_2, 0.0f);
		}else{
			// Reset suspension to extended
			float extendedLineLength = extendedSpringLength + wheelRadius;
			CVector pos;
			for(i = 0; i < 4; i++){
				wheelPositions[i] = specialColModel->lines[i].p0.z - m_aSuspensionSpringRatio[i]*m_aSuspensionLineLength[i];
				mi->GetWheelPosn(i, pos);
				pos.z += extendedUpperLimit;
				specialColModel->lines[i].p0 = pos;
				pos.z -= extendedLineLength;
				specialColModel->lines[i].p1 = pos;
				m_aSuspensionSpringLength[i] = extendedSpringLength;
				m_aSuspensionLineLength[i] = extendedLineLength;

				if(m_aSuspensionSpringRatio[i] < 1.0f){
					m_aSuspensionSpringRatio[i] = (specialColModel->lines[i].p0.z - wheelPositions[i])/m_aSuspensionLineLength[i];
					if(m_aSuspensionSpringRatio[i] > 1.0f)
						m_aSuspensionSpringRatio[i] = 1.0f;
				}

				setPrevRatio = true;
			}
			DMAudio.PlayOneShot(m_audioEntityId, SOUND_CAR_HYDRAULIC_2, 0.0f);
		}
	}else{
		float suspChange[4];
		float maxDelta = 0.0f;
		float rear = CPad::GetPad(0)->GetCarGunUpDown()/128.0f;
		float front = -rear;
		float right = CPad::GetPad(0)->GetCarGunLeftRight()/128.0f;
		float left = -right;
		suspChange[CARWHEEL_FRONT_LEFT] = Max(front+left, 0.0f);
		suspChange[CARWHEEL_REAR_LEFT] = Max(rear+left, 0.0f);
		suspChange[CARWHEEL_FRONT_RIGHT] = Max(front+right, 0.0f);
		suspChange[CARWHEEL_REAR_RIGHT] = Max(rear+right, 0.0f);

		if(m_hydraulicState < 100){
			// Lowered, move wheels up

			if(m_hydraulicState == 0){
				normalUpperLimit += HYDRAULIC_UPPER_EXT;
				normalLowerLimit += HYDRAULIC_LOWER_EXT;
				normalSpringLength = normalUpperLimit - normalLowerLimit;
			}

			// Set suspension
			CVector pos;
			for(i = 0; i < 4; i++){
				if(suspChange[i] > 1.0f)
					suspChange[i] = 1.0f;

				float upperLimit = suspChange[i]*(extendedUpperLimit-normalUpperLimit) + normalUpperLimit;
				float springLength = suspChange[i]*(extendedSpringLength-normalSpringLength) + normalSpringLength;
				float lineLength = springLength + wheelRadius;

				wheelPositions[i] = specialColModel->lines[i].p0.z - m_aSuspensionSpringRatio[i]*m_aSuspensionLineLength[i];
				mi->GetWheelPosn(i, pos);
				pos.z += upperLimit;
				specialColModel->lines[i].p0 = pos;
				pos.z -= lineLength;
				if(Abs(pos.z - specialColModel->lines[i].p1.z) > Abs(maxDelta))
					maxDelta = pos.z - specialColModel->lines[i].p1.z;
				specialColModel->lines[i].p1 = pos;
				m_aSuspensionSpringLength[i] = springLength;
				m_aSuspensionLineLength[i] = lineLength;

				if(m_aSuspensionSpringRatio[i] < 1.0f){
					m_aSuspensionSpringRatio[i] = (specialColModel->lines[i].p0.z - wheelPositions[i])/m_aSuspensionLineLength[i];
					if(m_aSuspensionSpringRatio[i] > 1.0f)
						m_aSuspensionSpringRatio[i] = 1.0f;
				}
			}
		}else{
			if(m_hydraulicState < 104)
				m_hydraulicState++;

			if(m_fVelocityChangeForAudio < 0.1f){
				normalUpperLimit += HYDRAULIC_UPPER_EXT;
				normalLowerLimit += HYDRAULIC_LOWER_EXT;
				normalSpringLength = normalUpperLimit - normalLowerLimit;
			}

			// Set suspension
			CVector pos;
			for(i = 0; i < 4; i++){
				if(suspChange[i] > 1.0f)
					suspChange[i] = 1.0f;

				float upperLimit = suspChange[i]*(normalUpperLimit-extendedUpperLimit) + extendedUpperLimit;
				float springLength = suspChange[i]*(normalSpringLength-extendedSpringLength) + extendedSpringLength;
				float lineLength = springLength + wheelRadius;

				wheelPositions[i] = specialColModel->lines[i].p0.z - m_aSuspensionSpringRatio[i]*m_aSuspensionLineLength[i];
				mi->GetWheelPosn(i, pos);
				pos.z += upperLimit;
				specialColModel->lines[i].p0 = pos;
				pos.z -= lineLength;
				if(Abs(pos.z - specialColModel->lines[i].p1.z) > Abs(maxDelta))
					maxDelta = pos.z - specialColModel->lines[i].p1.z;
				specialColModel->lines[i].p1 = pos;
				m_aSuspensionSpringLength[i] = springLength;
				m_aSuspensionLineLength[i] = lineLength;

				if(m_aSuspensionSpringRatio[i] < 1.0f){
					m_aSuspensionSpringRatio[i] = (specialColModel->lines[i].p0.z - wheelPositions[i])/m_aSuspensionLineLength[i];
					if(m_aSuspensionSpringRatio[i] > 1.0f)
						m_aSuspensionSpringRatio[i] = 1.0f;
				}
			}
		}

		float limitDiff = extendedLowerLimit - normalLowerLimit;
		if(limitDiff != 0.0f && Abs(maxDelta/limitDiff) > 0.01f){
			float f = (maxDelta + limitDiff)/2.0f/limitDiff;
			f = clamp(f, 0.0f, 1.0f);
			DMAudio.PlayOneShot(m_audioEntityId, SOUND_CAR_HYDRAULIC_3, f);
			if(f < 0.4f || f > 0.6f)
				setPrevRatio = true;
			if(f < 0.25f)
				DMAudio.PlayOneShot(m_audioEntityId, SOUND_CAR_HYDRAULIC_2, 0.0f);
			else if(f > 0.75f)
				DMAudio.PlayOneShot(m_audioEntityId, SOUND_CAR_HYDRAULIC_1, 0.0f);
		}
	}

	if(setPrevRatio)
		for(i = 0; i < 4; i++){
			// wheel radius in relation to suspension line
			float wheelRadius = 1.0f - m_aSuspensionSpringLength[i]/m_aSuspensionLineLength[i];
			m_aSuspensionSpringRatioPrev[i] = (m_aSuspensionSpringRatio[i]-wheelRadius)/(1.0f-wheelRadius);
		}
}

void
CAutomobile::ProcessBuoyancy(void)	
{
	int i;
	CVector impulse, point;

	if(mod_Buoyancy.ProcessBuoyancy(this, m_fBuoyancy, &point, &impulse)){
		float timeStep = Max(CTimer::GetTimeStep(), 0.01f);
		float impulseRatio = impulse.z / (GRAVITY * m_fMass * timeStep);
		float waterResistance = Pow(1.0f - 0.05f*impulseRatio, CTimer::GetTimeStep());
		m_vecMoveSpeed *= waterResistance;
		m_vecTurnSpeed *= waterResistance;

		bool heliHitWaterHard = false;
		if(IsRealHeli() && m_aWheelSpeed[1] > 0.15f){
			if(GetModelIndex() == MI_SEASPAR){
				if(impulseRatio > 3.0f){
					m_aWheelSpeed[1] = 0.0f;
					heliHitWaterHard = true;
				}
			}else{
				float strength = 1.0f/Max(8.0f*impulseRatio, 1.0f);
				ApplyMoveForce(-2.0f*impulse/strength);
				ApplyTurnForce(-impulse/strength, point);
				if(impulseRatio > 0.9f){
					m_aWheelSpeed[1] = 0.0f;
					heliHitWaterHard = true;
				}else
					return;
			}
		}

		bTouchingWater = true;
		ApplyMoveForce(impulse);
		ApplyTurnForce(impulse, point);
		CVector initialSpeed = m_vecMoveSpeed;

		if(m_modelIndex == MI_SEASPAR && impulseRatio < 3.0f && (GetUp().z > -0.5f || impulseRatio < 0.6f) ||
		   CVehicle::bHoverCheat && GetStatus() == STATUS_PLAYER && GetUp().z > 0.1f){
			bIsInWater = false;
			bIsDrowning = false;
		}else if(heliHitWaterHard || impulseRatio > 1.0f ||
		         impulseRatio > 0.6f && (m_aSuspensionSpringRatio[0] == 1.0f ||
		                                 m_aSuspensionSpringRatio[1] == 1.0f ||
		                                 m_aSuspensionSpringRatio[2] == 1.0f ||
		                                 m_aSuspensionSpringRatio[3] == 1.0f)){
			bIsInWater = true;
			bIsDrowning = true;
			if(m_vecMoveSpeed.z < -0.1f)
				m_vecMoveSpeed.z = -0.1f;

			if(pDriver){
				pDriver->bIsInWater = true;
				if(pDriver->IsPlayer() || !bWaterTight)
					pDriver->InflictDamage(nil, WEAPONTYPE_DROWNING, CTimer::GetTimeStep(), PEDPIECE_TORSO, 0);
			}
			for(i = 0; i < m_nNumMaxPassengers; i++)
				if(pPassengers[i]){
					pPassengers[i]->bIsInWater = true;
					if(pPassengers[i]->IsPlayer() || !bWaterTight)
						pPassengers[i]->InflictDamage(nil, WEAPONTYPE_DROWNING, CTimer::GetTimeStep(), PEDPIECE_TORSO, 0);
				}
		}else{
			bIsInWater = false;
			bIsDrowning = false;
		}

		static uint32 nGenerateRaindrops = 0;
		static uint32 nGenerateWaterCircles = 0;

		if(initialSpeed.z < -0.1f && impulse.z > 0.3f || heliHitWaterHard){
			RwRGBA color;
			color.red = (0.5f * CTimeCycle::GetDirectionalRed() + CTimeCycle::GetAmbientRed_Obj())*0.45f*255;
			color.green = (0.5f * CTimeCycle::GetDirectionalGreen() + CTimeCycle::GetAmbientGreen_Obj())*0.45f*255;
			color.blue = (0.5f * CTimeCycle::GetDirectionalBlue() + CTimeCycle::GetAmbientBlue_Obj())*0.45f*255;
			color.alpha = CGeneral::GetRandomNumberInRange(0, 32) + 128;
			CVector target = CVector(0.0f, 0.0f, CGeneral::GetRandomNumberInRange(0.15f, 0.45f));
			CParticleObject::AddObject(POBJECT_CAR_WATER_SPLASH, GetPosition(),
				target, 0.0f, 75, color, true);

			nGenerateRaindrops = CTimer::GetTimeInMilliseconds() + 300;
			nGenerateWaterCircles = CTimer::GetTimeInMilliseconds() + 60;

			if(heliHitWaterHard){
				CVector right = CrossProduct(GetForward(), CVector(0.0f, 0.0f, 1.0f));
				CParticleObject::AddObject(POBJECT_CAR_WATER_SPLASH, GetPosition() + right,
					target, 0.0f, 75, color, true);
				CParticleObject::AddObject(POBJECT_CAR_WATER_SPLASH, GetPosition() - right,
					target, 0.0f, 75, color, true);
			}

			if(m_vecMoveSpeed.z < -0.2f)
				m_vecMoveSpeed.z = -0.2f;
			DMAudio.PlayOneShot(m_audioEntityId, SOUND_WATER_FALL, 0.0f);
		}

		if(nGenerateWaterCircles > 0 && nGenerateWaterCircles <= CTimer::GetTimeInMilliseconds()){
			CVector pos = GetPosition();
			float waterLevel = 0.0f;
			if(CWaterLevel::GetWaterLevel(pos.x, pos.y, pos.z, &waterLevel, false))
				pos.z = waterLevel;
			static RwRGBA black;
			if(pos.z != 0.0f){
				nGenerateWaterCircles = 0;
				pos.z += 1.0f;
				for(i = 0; i < 4; i++){
					CVector p = pos;
					p.x += CGeneral::GetRandomNumberInRange(-2.5f, 2.5f);
					p.y += CGeneral::GetRandomNumberInRange(-2.5f, 2.5f);
					CParticle::AddParticle(PARTICLE_RAIN_SPLASH_BIGGROW,
						p, CVector(0.0f, 0.0f, 0.0f),
						nil, 0.0f, black, 0, 0, 0, 0);
				}
			}
		}

		if(nGenerateRaindrops > 0 && nGenerateRaindrops <= CTimer::GetTimeInMilliseconds()){
			CVector pos = GetPosition();
			float waterLevel = 0.0f;
			if(CWaterLevel::GetWaterLevel(pos.x, pos.y, pos.z, &waterLevel, false))
				pos.z = waterLevel;
			static RwRGBA black;
			if(pos.z >= 0.0f){
				nGenerateRaindrops = 0;
				pos.z += 0.5f;
				CParticleObject::AddObject(POBJECT_SPLASHES_AROUND,
					pos, CVector(0.0f, 0.0f, 0.0f), 6.5f, 2500, black, true);
			}
		}
	}else{
		bIsInWater = false;
		bIsDrowning = false;
		bTouchingWater = false;

		static RwRGBA splashCol = {155, 155, 185, 196};
		static RwRGBA smokeCol = {255, 255, 255, 255};

		for(i = 0; i < 4; i++){
			if(m_aSuspensionSpringRatio[i] < 1.0f && m_aWheelColPoints[i].surfaceB == SURFACE_WATER){
				CVector pos = m_aWheelColPoints[i].point + 0.3f*GetUp() - GetPosition();
				CVector vSpeed = GetSpeed(pos);
				vSpeed.z = 0.0f;
				float fSpeed = vSpeed.MagnitudeSqr();
				if(fSpeed > sq(0.05f)){
					fSpeed = Sqrt(fSpeed);

					float size = Min((fSpeed < 0.15f ? 0.25f : 0.75f)*fSpeed, 0.6f);
					CVector right = 0.2f*fSpeed*GetRight() + 0.2f*vSpeed;

					CParticle::AddParticle(PARTICLE_PED_SPLASH,
						pos + GetPosition(), -0.5f*right,
						nil, size, splashCol,
						CGeneral::GetRandomNumberInRange(0.0f, 10.0f),
						CGeneral::GetRandomNumberInRange(0.0f, 90.0f), 1, 0);

					CParticle::AddParticle(PARTICLE_RUBBER_SMOKE,
						pos + GetPosition(), -0.6f*right,
						nil, size, smokeCol, 0, 0, 0, 0);
				
					if((CTimer::GetFrameCounter() & 0xF) == 0)
						DMAudio.PlayOneShot(m_audioEntityId, SOUND_CAR_SPLASH, 2000.0f*fSpeed);
				}
			}
		}
	}
}

void
CAutomobile::DoDriveByShootings(void)
{
	CAnimBlendAssociation *anim = nil;
	CPlayerInfo* playerInfo = ((CPlayerPed*)this)->GetPlayerInfoForThisPlayerPed();
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

	AnimationId rightAnim = ANIM_DRIVEBY_R;
	AnimationId leftAnim = ANIM_DRIVEBY_L;
	if (pDriver->m_pMyVehicle->bLowVehicle) {
		rightAnim = ANIM_DRIVEBY_LOW_R;
		leftAnim = ANIM_DRIVEBY_LOW_L;
	}

	if(lookingLeft || lookingRight){
		if(lookingLeft){
			anim = RpAnimBlendClumpGetAssociation(pDriver->GetClump(), rightAnim);
			if(anim)
				anim->blendDelta = -1000.0f;
			anim = RpAnimBlendClumpGetAssociation(pDriver->GetClump(), leftAnim);
			if(anim == nil || anim->blendDelta < 0.0f)
				anim = CAnimManager::AddAnimation(pDriver->GetClump(), ASSOCGRP_STD, leftAnim);
		}else if(pDriver->m_pMyVehicle->pPassengers[0] == nil || TheCamera.Cams[TheCamera.ActiveCam].Mode == CCam::MODE_1STPERSON){
			anim = RpAnimBlendClumpGetAssociation(pDriver->GetClump(), leftAnim);
			if(anim)
				anim->blendDelta = -1000.0f;
			anim = RpAnimBlendClumpGetAssociation(pDriver->GetClump(), rightAnim);
			if(anim == nil || anim->blendDelta < 0.0f)
				anim = CAnimManager::AddAnimation(pDriver->GetClump(), ASSOCGRP_STD, rightAnim);
		}

		if (!anim || !anim->IsRunning()) {
			if (CPad::GetPad(0)->GetCarGunFired() && CTimer::GetTimeInMilliseconds() > weapon->m_nTimer) {
				weapon->FireFromCar(this, lookingLeft, true);
				weapon->m_nTimer = CTimer::GetTimeInMilliseconds() + 70;
			}
		}
	}else{
		weapon->Reload();
		anim = RpAnimBlendClumpGetAssociation(pDriver->GetClump(), leftAnim);
		if(anim)
			anim->blendDelta = -1000.0f;
		anim = RpAnimBlendClumpGetAssociation(pDriver->GetClump(), rightAnim);
		if(anim)
			anim->blendDelta = -1000.0f;
	}

	// TODO: what is this?
	if(!lookingLeft && m_weaponDoorTimerLeft > 0.0f){
		m_weaponDoorTimerLeft = Max(m_weaponDoorTimerLeft - CTimer::GetTimeStep()*0.1f, 0.0f);
		ProcessOpenDoor(CAR_DOOR_LF, NUM_STD_ANIMS, m_weaponDoorTimerLeft);
	}
	if(!lookingRight && m_weaponDoorTimerRight > 0.0f){
		m_weaponDoorTimerRight = Max(m_weaponDoorTimerRight - CTimer::GetTimeStep()*0.1f, 0.0f);
		ProcessOpenDoor(CAR_DOOR_RF, NUM_STD_ANIMS, m_weaponDoorTimerRight);
	}
}

void
CAutomobile::DoHoverSuspensionRatios(void)
{
	int i;

	if(GetUp().z < 0.1f)
		return;

	CColModel *colmodel = GetColModel();
	for(i = 0; i < 4; i++){
		float z, waterZ;
		CVector upper = GetMatrix() * colmodel->lines[i].p0;
		CVector lower = GetMatrix() * colmodel->lines[i].p1;
		if(m_aSuspensionSpringRatio[i] < 1.0f)
			z = m_aWheelColPoints[i].point.z;
		else
			z = -100.0f;
		// see if touching water
		if(CWaterLevel::GetWaterLevel(lower, &waterZ, false) &&
		   waterZ > z && lower.z-1.0f < waterZ){
			// compress spring
			if(lower.z < waterZ){
				if(upper.z < waterZ)
					m_aSuspensionSpringRatio[i] = 0.0f;
				else
					m_aSuspensionSpringRatio[i] = (upper.z - waterZ)/(upper.z - lower.z);
			}else
				m_aSuspensionSpringRatio[i] = 0.99999f;

			m_aWheelColPoints[i].point.x = (lower.x - upper.x)*m_aSuspensionSpringRatio[i] + upper.x;
			m_aWheelColPoints[i].point.y = (lower.y - upper.y)*m_aSuspensionSpringRatio[i] + upper.y;
			m_aWheelColPoints[i].point.z = waterZ;
			m_aWheelColPoints[i].normal = CVector(0.01f, 0.0f, 1.0f);
			m_aWheelColPoints[i].surfaceB = SURFACE_WATER;
		}
	}
}

int32
CAutomobile::RcbanditCheckHitWheels(void)
{
	int x, xmin, xmax;
	int y, ymin, ymax;

	xmin = CWorld::GetSectorIndexX(GetPosition().x - 2.0f);
	if(xmin < 0) xmin = 0;
	xmax = CWorld::GetSectorIndexX(GetPosition().x + 2.0f);
	if(xmax > NUMSECTORS_X-1) xmax = NUMSECTORS_X-1;
	ymin = CWorld::GetSectorIndexY(GetPosition().y - 2.0f);
	if(ymin < 0) ymin = 0;
	ymax = CWorld::GetSectorIndexY(GetPosition().y + 2.0f);
	if(ymax > NUMSECTORS_Y-1) ymax = NUMSECTORS_X-1;

	CWorld::AdvanceCurrentScanCode();

	for(y = ymin; y <= ymax; y++)
		for(x = xmin; x <= xmax; x++){
			CSector *s = CWorld::GetSector(x, y);
			if(RcbanditCheck1CarWheels(s->m_lists[ENTITYLIST_VEHICLES]) ||
			   RcbanditCheck1CarWheels(s->m_lists[ENTITYLIST_VEHICLES_OVERLAP]))
				return 1;
		}
	return 0;
}

int32
CAutomobile::RcbanditCheck1CarWheels(CPtrList &list)
{
	static CMatrix matW2B;
	int i;
	CPtrNode *node;
	CAutomobile *car;
	CColModel *colModel = GetColModel();
	CVehicleModelInfo *mi;

	for(node = list.first; node; node = node->next){
		car = (CAutomobile*)node->item;
		if(this != car && car->IsCar() && car->GetModelIndex() != MI_RCBANDIT &&
		   car->m_scanCode != CWorld::GetCurrentScanCode()){
			car->m_scanCode = CWorld::GetCurrentScanCode();

			if(Abs(this->GetPosition().x - car->GetPosition().x) < 10.0f &&
			   Abs(this->GetPosition().y - car->GetPosition().y) < 10.0f){
				mi = (CVehicleModelInfo*)CModelInfo::GetModelInfo(car->GetModelIndex());

				for(i = 0; i < 4; i++){
					if(car->m_aSuspensionSpringRatioPrev[i] < 1.0f || car->GetStatus() == STATUS_SIMPLE){
						CVector wheelPos;
						CColSphere sph;
						mi->GetWheelPosn(i, wheelPos);
						matW2B = Invert(GetMatrix());
						sph.center = matW2B * (car->GetMatrix() * wheelPos);
						sph.radius = mi->m_wheelScale*0.25f;
						if(CCollision::TestSphereBox(sph, colModel->boundingBox))
							return 1;
					}
				}
			}
		}
	}
	return 0;
}

void
CAutomobile::PlaceOnRoadProperly(void)
{
	CColPoint point;
	CEntity *entity;
	CColModel *colModel = GetColModel();
	float lenFwd, lenBack;
	float frontZ, rearZ;

	lenFwd = colModel->boundingBox.max.y;
	lenBack = -colModel->boundingBox.min.y;

	CVector front(GetPosition().x + GetForward().x*lenFwd,
	              GetPosition().y + GetForward().y*lenFwd,
	              GetPosition().z + 5.0f);
	if(CWorld::ProcessVerticalLine(front, GetPosition().z - 5.0f, point, entity,
			true, false, false, false, false, false, nil)){
		frontZ = point.point.z;
		m_pCurGroundEntity = entity;
	}else{
		frontZ = m_fMapObjectHeightAhead;
	}

	CVector rear(GetPosition().x - GetForward().x*lenBack,
	             GetPosition().y - GetForward().y*lenBack,
	             GetPosition().z + 5.0f);
	if(CWorld::ProcessVerticalLine(rear, GetPosition().z - 5.0f, point, entity,
			true, false, false, false, false, false, nil)){
		rearZ = point.point.z;
		m_pCurGroundEntity = entity;
	}else{
		rearZ = m_fMapObjectHeightBehind;
	}

	float len = lenFwd + lenBack;
	float angle = Atan((frontZ - rearZ)/len);
	float c = Cos(angle);
	float s = Sin(angle);

	GetMatrix().GetRight() = CVector((front.y - rear.y) / len, -(front.x - rear.x) / len, 0.0f);
	GetMatrix().GetForward() = CVector(-c * GetRight().y, c * GetRight().x, s);
	GetMatrix().GetUp() = CrossProduct(GetRight(), GetForward());
	GetMatrix().GetPosition() = CVector((front.x + rear.x) / 2.0f, (front.y + rear.y) / 2.0f, (frontZ + rearZ) / 2.0f + GetHeightAboveRoad());
}

void
CAutomobile::VehicleDamage(float impulse, uint16 damagedPiece)
{
	int i;
	float damageMultiplier = 0.333f;

	if(impulse == 0.0f){
		impulse = m_fDamageImpulse;
		damagedPiece = m_nDamagePieceType;
		damageMultiplier = 1.0f;
	}

	if(GetStatus() == STATUS_PLAYER && CStats::GetPercentageProgress() >= 100.0f)
		impulse *= 0.5f;

	CVector pos(0.0f, 0.0f, 0.0f);

	if(!bCanBeDamaged)
		return;

	if(m_pDamageEntity && m_pDamageEntity->IsPed() && ((CPed*)m_pDamageEntity)->bIsStanding){
		float speed = ((CPed*)m_pDamageEntity)->m_vecAnimMoveDelta.y * DotProduct(GetForward(), m_vecDamageNormal);
		if(speed < 0.0f)
			impulse = Max(impulse + ((CPed*)m_pDamageEntity)->m_fMass * speed, 0.0f);
	}

	// damage flipped over car
	if(GetUp().z < 0.0f && this != FindPlayerVehicle()){
		if(bNotDamagedUpsideDown || GetStatus() == STATUS_PLAYER_REMOTE || bIsInWater)
			return;
		if(GetStatus() != STATUS_WRECKED)
			m_fHealth = Max(m_fHealth - 4.0f*CTimer::GetTimeStep(), 0.0f);
	}

	float minImpulse = GetModelIndex() == MI_RCRAIDER || GetModelIndex() == MI_RCGOBLIN ? 1.0f : 25.0f;
	if(impulse > minImpulse && GetStatus() != STATUS_WRECKED){
		if(bIsLawEnforcer &&
		   FindPlayerVehicle() && FindPlayerVehicle() == m_pDamageEntity &&
		   GetStatus() != STATUS_ABANDONED &&
		   FindPlayerVehicle()->m_vecMoveSpeed.Magnitude() >= m_vecMoveSpeed.Magnitude() &&
		   FindPlayerVehicle()->m_vecMoveSpeed.Magnitude() > 0.1f)
			FindPlayerPed()->SetWantedLevelNoDrop(1);

		if(GetStatus() == STATUS_PLAYER && impulse > 50.0f){
			uint8 freq = Min(0.4f*impulse*2000.0f/m_fMass + 100.0f, 250.0f);
			CPad::GetPad(0)->StartShake(40000/freq, freq);
		}

		if(GetStatus() != STATUS_PLAYER && bOnlyDamagedByPlayer){
			if(m_pDamageEntity != FindPlayerPed() &&
			   m_pDamageEntity != FindPlayerVehicle())
				return;
		}

		if(m_pDamageEntity && m_pDamageEntity->IsVehicle()){
			m_nLastWeaponDamage = WEAPONTYPE_RAMMEDBYCAR;
			m_pLastDamageEntity = m_pDamageEntity;
		}

		if(bCollisionProof)
			return;

		if(m_pDamageEntity){
			if(m_pDamageEntity->IsBuilding() &&
			   DotProduct(m_vecDamageNormal, GetUp()) > 0.6f)
				return;
		}

		int oldLightStatus[4];
		for(i = 0; i < 4; i++)
			oldLightStatus[i] = Damage.GetLightStatus((eLights)i);

		if(GetUp().z > 0.0f || m_vecMoveSpeed.MagnitudeSqr() > 0.1f){
			float impulseMult = bMoreResistantToDamage ? 0.5f : 4.0f;

			switch(damagedPiece){
			case CAR_PIECE_BUMP_FRONT:
				GetComponentWorldPosition(CAR_BUMP_FRONT, pos);
				dmgDrawCarCollidingParticles(pos, impulse*damageMultiplier);
				if(Damage.ApplyDamage(COMPONENT_BUMPER_FRONT, impulse*impulseMult, pHandling->fCollisionDamageMultiplier))
					SetBumperDamage(CAR_BUMP_FRONT, VEHBUMPER_FRONT);
				if(m_aCarNodes[CAR_BONNET] && Damage.GetPanelStatus(VEHBUMPER_FRONT) == PANEL_STATUS_MISSING){
			case CAR_PIECE_BONNET:
					GetComponentWorldPosition(CAR_BONNET, pos);
					dmgDrawCarCollidingParticles(pos, impulse*damageMultiplier);
					if(GetModelIndex() != MI_DODO)
					if(Damage.ApplyDamage(COMPONENT_DOOR_BONNET, impulse*impulseMult, pHandling->fCollisionDamageMultiplier))
						SetDoorDamage(CAR_BONNET, DOOR_BONNET);
				}
				break;

			case CAR_PIECE_BUMP_REAR:
				GetComponentWorldPosition(CAR_BUMP_REAR, pos);
				dmgDrawCarCollidingParticles(pos, impulse*damageMultiplier);
				if(Damage.ApplyDamage(COMPONENT_BUMPER_REAR, impulse*impulseMult, pHandling->fCollisionDamageMultiplier))
					SetBumperDamage(CAR_BUMP_REAR, VEHBUMPER_REAR);
				if(m_aCarNodes[CAR_BOOT] && Damage.GetPanelStatus(VEHBUMPER_REAR) == PANEL_STATUS_MISSING){
			case CAR_PIECE_BOOT:
					GetComponentWorldPosition(CAR_BOOT, pos);
					dmgDrawCarCollidingParticles(pos, impulse*damageMultiplier);
					if(Damage.ApplyDamage(COMPONENT_DOOR_BOOT, impulse*impulseMult, pHandling->fCollisionDamageMultiplier))
						SetDoorDamage(CAR_BOOT, DOOR_BOOT);
				}
				break;

			case CAR_PIECE_DOOR_LF:
				GetComponentWorldPosition(CAR_DOOR_LF, pos);
				dmgDrawCarCollidingParticles(pos, impulse*damageMultiplier);
				if(Damage.ApplyDamage(COMPONENT_DOOR_FRONT_LEFT, impulse*impulseMult, pHandling->fCollisionDamageMultiplier))
					SetDoorDamage(CAR_DOOR_LF, DOOR_FRONT_LEFT);
				break;
			case CAR_PIECE_DOOR_RF:
				GetComponentWorldPosition(CAR_DOOR_RF, pos);
				dmgDrawCarCollidingParticles(pos, impulse*damageMultiplier);
				if(Damage.ApplyDamage(COMPONENT_DOOR_FRONT_RIGHT, impulse*impulseMult, pHandling->fCollisionDamageMultiplier))
					SetDoorDamage(CAR_DOOR_RF, DOOR_FRONT_RIGHT);
				break;
			case CAR_PIECE_DOOR_LR:
				GetComponentWorldPosition(CAR_DOOR_LR, pos);
				dmgDrawCarCollidingParticles(pos, impulse*damageMultiplier);
				if(Damage.ApplyDamage(COMPONENT_DOOR_REAR_LEFT, impulse*impulseMult, pHandling->fCollisionDamageMultiplier))
					SetDoorDamage(CAR_DOOR_LR, DOOR_REAR_LEFT);
				break;
			case CAR_PIECE_DOOR_RR:
				GetComponentWorldPosition(CAR_DOOR_RR, pos);
				dmgDrawCarCollidingParticles(pos, impulse*damageMultiplier);
				if(Damage.ApplyDamage(COMPONENT_DOOR_REAR_RIGHT, impulse*impulseMult, pHandling->fCollisionDamageMultiplier))
					SetDoorDamage(CAR_DOOR_RR, DOOR_REAR_RIGHT);
				break;

			case CAR_PIECE_WING_LF:
				GetComponentWorldPosition(CAR_WING_LF, pos);
				dmgDrawCarCollidingParticles(pos, impulse*damageMultiplier);
				if(Damage.ApplyDamage(COMPONENT_PANEL_FRONT_LEFT, impulse*impulseMult, pHandling->fCollisionDamageMultiplier))
					SetPanelDamage(CAR_WING_LF, VEHPANEL_FRONT_LEFT);
				break;
			case CAR_PIECE_WING_RF:
				GetComponentWorldPosition(CAR_WING_RF, pos);
				dmgDrawCarCollidingParticles(pos, impulse*damageMultiplier);
				if(Damage.ApplyDamage(COMPONENT_PANEL_FRONT_RIGHT, impulse*impulseMult, pHandling->fCollisionDamageMultiplier))
					SetPanelDamage(CAR_WING_RF, VEHPANEL_FRONT_RIGHT);
				break;
			case CAR_PIECE_WING_LR:
				GetComponentWorldPosition(CAR_WING_LR, pos);
				dmgDrawCarCollidingParticles(pos, impulse*damageMultiplier);
				if(Damage.ApplyDamage(COMPONENT_PANEL_REAR_LEFT, impulse*impulseMult, pHandling->fCollisionDamageMultiplier))
					SetPanelDamage(CAR_WING_LR, VEHPANEL_REAR_LEFT);
				break;
			case CAR_PIECE_WING_RR:
				GetComponentWorldPosition(CAR_WING_RR, pos);
				dmgDrawCarCollidingParticles(pos, impulse*damageMultiplier);
				if(Damage.ApplyDamage(COMPONENT_PANEL_REAR_RIGHT, impulse*impulseMult, pHandling->fCollisionDamageMultiplier))
					SetPanelDamage(CAR_WING_RR, VEHPANEL_REAR_RIGHT);
				break;

			case CAR_PIECE_WHEEL_LF:
			case CAR_PIECE_WHEEL_LR:
			case CAR_PIECE_WHEEL_RF:
			case CAR_PIECE_WHEEL_RR:
				break;

			case CAR_PIECE_WINDSCREEN:
				if(Damage.ApplyDamage(COMPONENT_PANEL_WINDSCREEN, impulse*impulseMult, pHandling->fCollisionDamageMultiplier)){
					uint8 oldStatus = Damage.GetPanelStatus(VEHPANEL_WINDSCREEN);
					SetPanelDamage(CAR_WINDSCREEN, VEHPANEL_WINDSCREEN);
					if(oldStatus != Damage.GetPanelStatus(VEHPANEL_WINDSCREEN)){
					//	DMAudio.PlayOneShot(m_audioEntityId, SOUND_CAR_WINDSHIELD_CRACK, 0.0f);
					}
				}
				break;
			}
		}

		float damage = (impulse-minImpulse)*pHandling->fCollisionDamageMultiplier*0.6f*damageMultiplier;

		if(GetModelIndex() == MI_SECURICA && m_pDamageEntity && m_pDamageEntity->GetStatus() == STATUS_PLAYER)
			damage *= 7.0f;

		if(GetModelIndex() == MI_RCGOBLIN || GetModelIndex() == MI_RCRAIDER)
			damage *= 30.0f;

		if(damage > 0.0f){
			if(damage > 5.0f &&
			   pDriver &&
			   m_pDamageEntity && m_pDamageEntity->IsVehicle() &&
			   (this != FindPlayerVehicle() || ((CVehicle*)m_pDamageEntity)->VehicleCreatedBy == MISSION_VEHICLE) &&
// TODO(MIAMI): enum
			   ((CVehicle*)m_pDamageEntity)->pDriver){
				if(GetVehicleAppearance() == VEHICLE_APPEARANCE_CAR)
					pDriver->Say(145);
				else
					pDriver->Say(144);
			}

			int oldHealth = m_fHealth;
			if(this == FindPlayerVehicle())
				m_fHealth -= bTakeLessDamage ? damage/6.0f : damage/2.0f;
			else if(bTakeLessDamage)
				m_fHealth -= damage/12.0f;
			else if(m_pDamageEntity && m_pDamageEntity == FindPlayerVehicle())
				m_fHealth -= damage/1.5f;
			else
				m_fHealth -= damage/4.0f;
			if(m_fHealth <= 0.0f && oldHealth > 0)
				m_fHealth = 1.0f;
		}

		// play sound if a light broke
		for(i = 0; i < 4; i++)
			if(oldLightStatus[i] != 1 && Damage.GetLightStatus((eLights)i) == 1){
				DMAudio.PlayOneShot(m_audioEntityId, SOUND_CAR_LIGHT_BREAK, i);	// BUG? i?
				break;
			}
	}

	if(m_fHealth < 250.0f){
		// Car is on fire
		if(Damage.GetEngineStatus() < ENGINE_STATUS_ON_FIRE){
			// Set engine on fire and remember who did this
			Damage.SetEngineStatus(ENGINE_STATUS_ON_FIRE);
			m_fFireBlowUpTimer = 0.0f;
			m_pSetOnFireEntity = m_pDamageEntity;
			if(m_pSetOnFireEntity)
				m_pSetOnFireEntity->RegisterReference(&m_pSetOnFireEntity);
		}
	}else{
		if(GetModelIndex() == MI_BFINJECT){
			if(m_fHealth < 400.0f)
				Damage.SetEngineStatus(200);
			else if(m_fHealth < 600.0f)
				Damage.SetEngineStatus(100);
		}
	}
}

void
CAutomobile::dmgDrawCarCollidingParticles(const CVector &pos, float amount)
{
	int i, n;

	if(!GetIsOnScreen())
		return;

	// FindPlayerSpeed() unused

	n = (int)amount/20;

	for(i = 0; i < ((n+4)&0x1F); i++)
		CParticle::AddParticle(PARTICLE_SPARK_SMALL, pos,
			CVector(CGeneral::GetRandomNumberInRange(-0.1f, 0.1f),
			        CGeneral::GetRandomNumberInRange(-0.1f, 0.1f),
			        0.006f));

	for(i = 0; i < n+2; i++)
		CParticle::AddParticle(PARTICLE_CARCOLLISION_DUST,
			CVector(CGeneral::GetRandomNumberInRange(-1.2f, 1.2f) + pos.x,
			        CGeneral::GetRandomNumberInRange(-1.2f, 1.2f) + pos.y,
			        pos.z),
			CVector(0.0f, 0.0f, 0.0f), nil, 0.5f);

	n = (int)amount/50 + 1;
	for(i = 0; i < n; i++)
		CParticle::AddParticle(PARTICLE_CAR_DEBRIS, pos,
			CVector(CGeneral::GetRandomNumberInRange(-0.25f, 0.25f),
			        CGeneral::GetRandomNumberInRange(-0.25f, 0.25f),
			        CGeneral::GetRandomNumberInRange(0.1f, 0.25f)),
			nil,
			CGeneral::GetRandomNumberInRange(0.02f, 0.08f),
			CVehicleModelInfo::ms_vehicleColourTable[m_currentColour1],
			CGeneral::GetRandomNumberInRange(-40.0f, 40.0f),
			0,
			CGeneral::GetRandomNumberInRange(0.0f, 4.0f));
}

void
CAutomobile::AddDamagedVehicleParticles(void)
{
	int i, n;

	if(this == FindPlayerVehicle() && TheCamera.GetLookingForwardFirstPerson())
		return;
	if(this != FindPlayerVehicle() && (CTimer::GetFrameCounter() + m_randomSeed) & 1)
		return;
	if(m_fHealth >= 650.0f)
		return;

	CVector direction = 0.85f*m_vecMoveSpeed;
	CVector damagePos = ((CVehicleModelInfo*)CModelInfo::GetModelInfo(GetModelIndex()))->m_positions[CAR_POS_HEADLIGHTS];

	switch(Damage.GetDoorStatus(DOOR_BONNET)){
	case DOOR_STATUS_OK:
	case DOOR_STATUS_SMASHED:
		// Bonnet is still there, smoke comes out at the edge
		damagePos += vecDAMAGE_ENGINE_POS_SMALL;
		break;
	case DOOR_STATUS_SWINGING:
	case DOOR_STATUS_MISSING:
		// Bonnet is gone, smoke comes out at the engine
		damagePos += vecDAMAGE_ENGINE_POS_BIG;
		break;
	}

	if(GetModelIndex() == MI_BFINJECT)
		damagePos = CVector(0.3f, -1.5f, -0.1f);
	else if(GetModelIndex() == MI_CADDY)
		damagePos = CVector(0.6f, -1.0f, -0.25f);
	else if(IsRealHeli()){
		damagePos.x = 0.4f*GetColModel()->boundingBox.max.x;
		damagePos.y = 0.2f*GetColModel()->boundingBox.min.y;
		damagePos.z = 0.3f*GetColModel()->boundingBox.max.z;
	}else
		damagePos.z += 0.4f*(GetColModel()->boundingBox.max.z-damagePos.z) * DotProduct(GetForward(), m_vecMoveSpeed);
	damagePos = GetMatrix()*damagePos;
	damagePos.z += 0.15f;

	bool electric = pHandling->Transmission.nEngineType == 'E';

	if(electric && m_fHealth < 320.0f && m_fHealth > 1.0f){
		direction = 0.85f*m_vecMoveSpeed;
		direction += GetRight() * CGeneral::GetRandomNumberInRange(0.0f, 0.04f) * (1.0f - 2.0f*m_vecMoveSpeed.Magnitude());
		direction.z += 0.001f;
		n = (CGeneral::GetRandomNumber() & 7) + 2;
		for(i = 0; i < n; i++)
			CParticle::AddParticle(PARTICLE_SPARK_SMALL, damagePos, direction);
		if(((CTimer::GetFrameCounter() + m_randomSeed) & 7) == 0)
			CParticle::AddParticle(PARTICLE_ENGINE_SMOKE2, damagePos, 0.8f*m_vecMoveSpeed, nil, 0.1f, 0, 0, 0, 1000);
	}else if(electric && m_fHealth < 460.0f){
		direction = 0.85f*m_vecMoveSpeed;
		direction += GetRight() * CGeneral::GetRandomNumberInRange(0.0f, 0.04f) * (1.0f - 2.0f*m_vecMoveSpeed.Magnitude());
		direction.z += 0.001f;
		n = (CGeneral::GetRandomNumber() & 3) + 2;
		for(i = 0; i < n; i++)
			CParticle::AddParticle(PARTICLE_SPARK_SMALL, damagePos, direction);
		if(((CTimer::GetFrameCounter() + m_randomSeed) & 0xF) == 0)
			CParticle::AddParticle(PARTICLE_ENGINE_SMOKE, damagePos, 0.8f*m_vecMoveSpeed, nil, 0.1f, 0, 0, 0, 1000);
	}else if(m_fHealth < 250.0f){
		// nothing
	}else if(m_fHealth < 320.0f){
		CParticle::AddParticle(PARTICLE_ENGINE_SMOKE2, damagePos, 0.8f*direction);
	}else if(m_fHealth < 390.0f){
		CParticle::AddParticle(PARTICLE_ENGINE_STEAM, damagePos, 0.75f*direction);
		CParticle::AddParticle(PARTICLE_ENGINE_SMOKE, damagePos, 0.85f*direction);
	}else if(m_fHealth < 460.0f){
		int rnd = CTimer::GetFrameCounter() + m_randomSeed;
		if(rnd < 10 ||
		   rnd < 70 && rnd > 25 ||
		   rnd < 160 && rnd > 100 ||
		   rnd < 200 && rnd > 175 ||
		   rnd > 235)
			return;
		direction.z += 0.05f*Max(1.0f - 1.6f*m_vecMoveSpeed.Magnitude(), 0.0f);
		if(electric){
			// BUG. we had that case already
			direction = 0.85f*m_vecMoveSpeed;
			direction += GetRight() * CGeneral::GetRandomNumberInRange(0.0f, 0.04f) * (1.0f - 2.0f*m_vecMoveSpeed.Magnitude());
			direction.z += 0.001f;
			n = (CGeneral::GetRandomNumber() & 2) + 2;
			for(i = 0; i < n; i++)
				CParticle::AddParticle(PARTICLE_SPARK_SMALL, damagePos, direction);
			if(((CTimer::GetFrameCounter() + m_randomSeed) & 0xF) == 0)
				CParticle::AddParticle(PARTICLE_ENGINE_SMOKE, damagePos, 0.8f*m_vecMoveSpeed, nil, 0.1f, 0, 0, 0, 1000);
		}else{
			if(TheCamera.GetLookDirection() != LOOKING_FORWARD)
				CParticle::AddParticle(PARTICLE_ENGINE_STEAM, damagePos, 0.75f*direction);
			else if(((CTimer::GetFrameCounter() + m_randomSeed) & 1) == 0)
				CParticle::AddParticle(PARTICLE_ENGINE_STEAM, damagePos, 0.85f*m_vecMoveSpeed);
		}
	}else if(((CTimer::GetFrameCounter() + m_randomSeed) & 3) == 0 ||
	         ((CTimer::GetFrameCounter() + m_randomSeed) & 3) == 2){
		CParticle::AddParticle(PARTICLE_ENGINE_STEAM, damagePos, 0.9f*direction);
	}
}

int32
CAutomobile::AddWheelDirtAndWater(CColPoint *colpoint, uint32 belowEffectSpeed)
{
	int i;
	CVector dir;
	static RwRGBA grassCol = { 8, 24, 8, 255 };
	static RwRGBA gravelCol = { 64, 64, 64, 255 };
	static RwRGBA mudCol = { 64, 32, 16, 255 };
	static RwRGBA sandCol = { 170, 165, 140, 255 };
	static RwRGBA waterCol = { 48, 48, 64, 0 };

	if(!belowEffectSpeed &&
	   colpoint->surfaceB != SURFACE_SAND && colpoint->surfaceB != SURFACE_SAND_BEACH)
		return 0;

	switch(colpoint->surfaceB){
	case SURFACE_GRASS:
		dir.x = -0.05f*m_vecMoveSpeed.x;
		dir.y = -0.05f*m_vecMoveSpeed.y;
		for(i = 0; i < 4; i++){
			dir.z = CGeneral::GetRandomNumberInRange(0.03f, 0.06f);
			CParticle::AddParticle(PARTICLE_WHEEL_DIRT, colpoint->point, dir, nil,
				CGeneral::GetRandomNumberInRange(0.02f, 0.1f), grassCol);
		}
		return 0;
	case SURFACE_GRAVEL:
		dir.x = -0.05f*m_vecMoveSpeed.x;
		dir.y = -0.05f*m_vecMoveSpeed.y;
		for(i = 0; i < 4; i++){
			dir.z = CGeneral::GetRandomNumberInRange(0.03f, 0.06f);
			CParticle::AddParticle(PARTICLE_WHEEL_DIRT, colpoint->point, dir, nil,
				CGeneral::GetRandomNumberInRange(0.05f, 0.09f), gravelCol);
		}
		return 1;
	case SURFACE_MUD_DRY:
		dir.x = -0.05f*m_vecMoveSpeed.x;
		dir.y = -0.05f*m_vecMoveSpeed.y;
		for(i = 0; i < 4; i++){
			dir.z = CGeneral::GetRandomNumberInRange(0.03f, 0.06f);
			CParticle::AddParticle(PARTICLE_WHEEL_DIRT, colpoint->point, dir, nil,
				CGeneral::GetRandomNumberInRange(0.02f, 0.06f), mudCol);
		}
		return 0;
	case SURFACE_SAND:
	case SURFACE_SAND_BEACH:
		if(CTimer::GetFrameCounter() & 2 ||
		   CGeneral::GetRandomNumberInRange(CWeather::WetRoads, 1.01f) > 0.5f)
			return 0;
		dir.x = 0.5f*m_vecMoveSpeed.x;
		dir.y = 0.5f*m_vecMoveSpeed.y;
		for(i = 0; i < 1; i++){
			dir.z = CGeneral::GetRandomNumberInRange(0.02f, 0.055f);
			CParticle::AddParticle(PARTICLE_SAND, colpoint->point, dir, nil,
				2.0f*m_vecMoveSpeed.Magnitude(), sandCol);
		}
		return 0;
	default:
		if(CWeather::WetRoads > 0.01f){
			if(CTimer::GetFrameCounter() & 1)
				CParticle::AddParticle(
					PARTICLE_WATERSPRAY,
					colpoint->point + CVector(0.0f, 0.0f, 0.25f+0.25f),
					CVector(0.0f, 0.0f, CGeneral::GetRandomNumberInRange(0.005f, 0.04f)),
					nil,
					CGeneral::GetRandomNumberInRange(0.1f, 0.5f), waterCol);
			return 0;
		}
		return 1;
	}
}

void
CAutomobile::GetComponentWorldPosition(int32 component, CVector &pos)
{
	if(m_aCarNodes[component] == nil){
		printf("CarNode missing: %d %d\n", GetModelIndex(), component);
		return;
	}
	RwMatrix *ltm = RwFrameGetLTM(m_aCarNodes[component]);
	pos = *RwMatrixGetPos(ltm);
}

bool
CAutomobile::IsComponentPresent(int32 comp)
{
	return m_aCarNodes[comp] != nil;
}

void
CAutomobile::SetComponentRotation(int32 component, CVector rotation)
{
	CMatrix mat(RwFrameGetMatrix(m_aCarNodes[component]));
	CVector pos = mat.GetPosition();
	// BUG: all these set the whole matrix
	mat.SetRotateX(DEGTORAD(rotation.x));
	mat.SetRotateY(DEGTORAD(rotation.y));
	mat.SetRotateZ(DEGTORAD(rotation.z));
	mat.Translate(pos);
	mat.UpdateRW();
}

void
CAutomobile::OpenDoor(int32 component, eDoors door, float openRatio)
{
	CMatrix mat(RwFrameGetMatrix(m_aCarNodes[component]));
	CVector pos = mat.GetPosition();
	float axes[3] = { 0.0f, 0.0f, 0.0f };
	float wasClosed = false;

	if(Doors[door].IsClosed()){
		// enable angle cull for closed doors
		RwFrameForAllObjects(m_aCarNodes[component], CVehicleModelInfo::ClearAtomicFlagCB, (void*)ATOMIC_FLAG_NOCULL);
		wasClosed = true;
	}

	Doors[door].Open(openRatio);

	if(wasClosed && Doors[door].RetAngleWhenClosed() != Doors[door].m_fAngle){
		// door opened
		HideAllComps();
		// turn off angle cull for swinging door
		RwFrameForAllObjects(m_aCarNodes[component], CVehicleModelInfo::SetAtomicFlagCB, (void*)ATOMIC_FLAG_NOCULL);
		DMAudio.PlayOneShot(m_audioEntityId, SOUND_CAR_DOOR_OPEN_BONNET + door, 0.0f);
	}

	if(!wasClosed && openRatio == 0.0f){
		// door closed
		if(Damage.GetDoorStatus(door) == DOOR_STATUS_SWINGING)
			Damage.SetDoorStatus(door, DOOR_STATUS_OK);	// huh?
		ShowAllComps();
		DMAudio.PlayOneShot(m_audioEntityId, SOUND_CAR_DOOR_CLOSE_BONNET + door, 0.0f);
	}

	axes[Doors[door].m_nAxis] = Doors[door].m_fAngle;
	mat.SetRotate(axes[0], axes[1], axes[2]);
	mat.Translate(pos);
	mat.UpdateRW();
}

inline void ProcessDoorOpenAnimation(CAutomobile *car, uint32 component, eDoors door, float time, float start, float end)
{
	if(time > start && time < end){
		float ratio = (time - start)/(end - start);
		if(car->Doors[door].GetAngleOpenRatio() < ratio)
			car->OpenDoor(component, door, ratio);
	}else if(time > end){
		car->OpenDoor(component, door, 1.0f);
	}
}

inline void ProcessDoorCloseAnimation(CAutomobile *car, uint32 component, eDoors door, float time, float start, float end)
{
	if(time > start && time < end){
		float ratio = 1.0f - (time - start)/(end - start);
		if(car->Doors[door].GetAngleOpenRatio() > ratio)
			car->OpenDoor(component, door, ratio);
	}else if(time > end){
		car->OpenDoor(component, door, 0.0f);
	}
}

inline void ProcessDoorOpenCloseAnimation(CAutomobile *car, uint32 component, eDoors door, float time, float start, float mid, float end)
{
	if(time > start && time < mid){
		// open
		float ratio = (time - start)/(mid - start);
		if(car->Doors[door].GetAngleOpenRatio() < ratio)
			car->OpenDoor(component, door, ratio);
	}else if(time > mid && time < end){
		// close
		float ratio = 1.0f - (time - mid)/(end - mid);
		if(car->Doors[door].GetAngleOpenRatio() > ratio)
			car->OpenDoor(component, door, ratio);
	}else if(time > end){
		car->OpenDoor(component, door, 0.0f);
	}
}

void
CAutomobile::ProcessOpenDoor(uint32 component, uint32 anim, float time)
{
	eDoors door;

	switch(component){
	case CAR_DOOR_RF: door = DOOR_FRONT_RIGHT; break;
	case CAR_DOOR_RR: door = DOOR_REAR_RIGHT; break;
	case CAR_DOOR_LF: door = DOOR_FRONT_LEFT; break;
	case CAR_DOOR_LR: door = DOOR_REAR_LEFT; break;
	default: assert(0);
	}

	if(IsDoorMissing(door))
		return;

	switch(anim){
	case ANIM_CAR_QJACK:
	case ANIM_CAR_OPEN_LHS:
	case ANIM_CAR_OPEN_RHS:
		ProcessDoorOpenAnimation(this, component, door, time, 0.41f, 0.89f);
		break;
	case ANIM_CAR_CLOSEDOOR_LHS:
	case ANIM_CAR_CLOSEDOOR_LOW_LHS:
	case ANIM_CAR_CLOSEDOOR_RHS:
	case ANIM_CAR_CLOSEDOOR_LOW_RHS:
		ProcessDoorCloseAnimation(this, component, door, time, 0.2f, 0.45f);
		break;
	case ANIM_CAR_ROLLDOOR:
	case ANIM_CAR_ROLLDOOR_LOW:
		ProcessDoorOpenCloseAnimation(this, component, door, time, 0.1f, 0.6f, 0.95f);
		break;
		break;
	case ANIM_CAR_GETOUT_LHS:
	case ANIM_CAR_GETOUT_LOW_LHS:
	case ANIM_CAR_GETOUT_RHS:
	case ANIM_CAR_GETOUT_LOW_RHS:
		ProcessDoorOpenAnimation(this, component, door, time, 0.06f, 0.43f);
		break;
	case ANIM_CAR_CLOSE_LHS:
	case ANIM_CAR_CLOSE_RHS:
		ProcessDoorCloseAnimation(this, component, door, time, 0.1f, 0.23f);
		break;
	case ANIM_CAR_PULLOUT_RHS:
	case ANIM_CAR_PULLOUT_LOW_RHS:
		OpenDoor(component, door, 1.0f);
	case ANIM_COACH_OPEN_L:
	case ANIM_COACH_OPEN_R:
		ProcessDoorOpenAnimation(this, component, door, time, 0.66f, 0.8f);
		break;
	case ANIM_COACH_OUT_L:
		ProcessDoorOpenAnimation(this, component, door, time, 0.0f, 0.3f);
		break;
	case ANIM_VAN_OPEN_L:
	case ANIM_VAN_OPEN:
		ProcessDoorOpenAnimation(this, component, door, time, 0.37f, 0.55f);
		break;
	case ANIM_VAN_CLOSE_L:
	case ANIM_VAN_CLOSE:
		ProcessDoorCloseAnimation(this, component, door, time, 0.5f, 0.8f);
		break;
	case ANIM_VAN_GETOUT_L:
	case ANIM_VAN_GETOUT:
		ProcessDoorOpenAnimation(this, component, door, time, 0.5f, 0.6f);
		break;
	case NUM_STD_ANIMS:
		OpenDoor(component, door, time);
		break;
	}
}

bool
CAutomobile::IsDoorReady(eDoors door)
{
	if(Doors[door].IsClosed() || IsDoorMissing(door))
		return true;
	int doorflag = 0;
	switch(door){
	case DOOR_FRONT_LEFT: doorflag = CAR_DOOR_FLAG_LF; break;
	case DOOR_FRONT_RIGHT: doorflag = CAR_DOOR_FLAG_RF; break;
	case DOOR_REAR_LEFT: doorflag = CAR_DOOR_FLAG_LR; break;
	case DOOR_REAR_RIGHT: doorflag = CAR_DOOR_FLAG_RR; break;
	default: break;
	}
	return (doorflag & m_nGettingInFlags) == 0;
}

bool
CAutomobile::IsDoorFullyOpen(eDoors door)
{
	return Doors[door].IsFullyOpen() || IsDoorMissing(door);
}

bool
CAutomobile::IsDoorClosed(eDoors door)
{
	return !!Doors[door].IsClosed();
}

bool
CAutomobile::IsDoorMissing(eDoors door)
{
	return Damage.GetDoorStatus(door) == DOOR_STATUS_MISSING;
}

bool
CAutomobile::IsDoorReady(uint32 door)
{
	switch(door){
	case CAR_DOOR_RF: return IsDoorReady(DOOR_FRONT_RIGHT);
	case CAR_DOOR_RR: return IsDoorReady(DOOR_REAR_RIGHT);
	case CAR_DOOR_LF: return IsDoorReady(DOOR_FRONT_LEFT);
	case CAR_DOOR_LR: return IsDoorReady(DOOR_REAR_LEFT);
	default:
		return false;
	}
}

bool
CAutomobile::IsDoorMissing(uint32 door)
{
	switch(door){
	case CAR_DOOR_RF: return IsDoorMissing(DOOR_FRONT_RIGHT);
	case CAR_DOOR_RR: return IsDoorMissing(DOOR_REAR_RIGHT);
	case CAR_DOOR_LF: return IsDoorMissing(DOOR_FRONT_LEFT);
	case CAR_DOOR_LR: return IsDoorMissing(DOOR_REAR_LEFT);
	default:
		return false;
	}
}

bool
CAutomobile::IsOpenTopCar(void)
{
	return GetModelIndex() == MI_STINGER ||
		// component 0 is assumed to be a roof
		GetModelIndex() == MI_COMET && m_aExtras[0] != 0 && m_aExtras[1] != 0 ||
		GetModelIndex() == MI_STALLION && m_aExtras[0] != 0 && m_aExtras[1] != 0;
}

void
CAutomobile::RemoveRefsToVehicle(CEntity *ent)
{
	int i;
	for(i = 0; i < 4; i++)
		if(m_aGroundPhysical[i] == ent)
			m_aGroundPhysical[i] = nil;
}

void
CAutomobile::BlowUpCar(CEntity *culprit)
{
	RpAtomic *atomic;

	if(!bCanBeDamaged)
		return;

	if(culprit == FindPlayerPed() || culprit == FindPlayerVehicle()){
		CWorld::Players[CWorld::PlayerInFocus].m_nHavocLevel += 20;
		CWorld::Players[CWorld::PlayerInFocus].m_fMediaAttention += 10.0f;
		CStats::PropertyDestroyed += CGeneral::GetRandomNumber()%6000 + 4000;
	}

	// explosion pushes vehicle up
	m_vecMoveSpeed.z += 0.13f;
	SetStatus(STATUS_WRECKED);
	bRenderScorched = true;
	m_nTimeOfDeath = CTimer::GetTimeInMilliseconds();
	Damage.FuckCarCompletely();

	if(GetModelIndex() != MI_RCBANDIT){
		SetBumperDamage(CAR_BUMP_FRONT, VEHBUMPER_FRONT);
		SetBumperDamage(CAR_BUMP_REAR, VEHBUMPER_REAR);
		SetDoorDamage(CAR_BONNET, DOOR_BONNET);
		SetDoorDamage(CAR_BOOT, DOOR_BOOT);
		SetDoorDamage(CAR_DOOR_LF, DOOR_FRONT_LEFT);
		SetDoorDamage(CAR_DOOR_RF, DOOR_FRONT_RIGHT);
		SetDoorDamage(CAR_DOOR_LR, DOOR_REAR_LEFT);
		SetDoorDamage(CAR_DOOR_RR, DOOR_REAR_RIGHT);
		SpawnFlyingComponent(CAR_WHEEL_LF, COMPGROUP_WHEEL);
		atomic = nil;
		RwFrameForAllObjects(m_aCarNodes[CAR_WHEEL_LF], GetCurrentAtomicObjectCB, &atomic);
		if(atomic)
			RpAtomicSetFlags(atomic, 0);
	}

	m_fHealth = 0.0f;
	m_nBombTimer = 0;
	m_bombType = CARBOMB_NONE;

	TheCamera.CamShake(0.7f, GetPosition().x, GetPosition().y, GetPosition().z);

	KillPedsInVehicle();

	bEngineOn = false;
	bLightsOn = false;
	m_bSirenOrAlarm = false;
	bTaxiLight = false;
	if(bIsAmbulanceOnDuty){
		bIsAmbulanceOnDuty = false;
		CCarCtrl::NumAmbulancesOnDuty--;
	}
	if(bIsFireTruckOnDuty){
		bIsFireTruckOnDuty = false;
		CCarCtrl::NumFiretrucksOnDuty--;
	}
	ChangeLawEnforcerState(false);

	gFireManager.StartFire(this, culprit, 0.8f, true);
	CDarkel::RegisterCarBlownUpByPlayer(this);
	if(GetModelIndex() == MI_RCBANDIT)
		CExplosion::AddExplosion(this, culprit, EXPLOSION_CAR_QUICK, GetPosition(), 0);
	else
		CExplosion::AddExplosion(this, culprit, EXPLOSION_CAR, GetPosition(), 0);
}

bool
CAutomobile::SetUpWheelColModel(CColModel *colModel)
{
	CVehicleModelInfo *mi = (CVehicleModelInfo*)CModelInfo::GetModelInfo(GetModelIndex());
	CColModel *vehColModel = mi->GetColModel();

	if(GetVehicleAppearance() == VEHICLE_APPEARANCE_HELI ||
	   GetVehicleAppearance() == VEHICLE_APPEARANCE_PLANE)
		return false;

	colModel->boundingSphere = vehColModel->boundingSphere;
	colModel->boundingBox = vehColModel->boundingBox;

	CMatrix mat;
	mat.Attach(RwFrameGetMatrix(m_aCarNodes[CAR_WHEEL_LF]));
	colModel->spheres[0].Set(mi->m_wheelScale, mat.GetPosition(), SURFACE_RUBBER, CAR_PIECE_WHEEL_LF);
	mat.Attach(RwFrameGetMatrix(m_aCarNodes[CAR_WHEEL_LB]));
	colModel->spheres[1].Set(mi->m_wheelScale, mat.GetPosition(), SURFACE_RUBBER, CAR_PIECE_WHEEL_LR);
	mat.Attach(RwFrameGetMatrix(m_aCarNodes[CAR_WHEEL_RF]));
	colModel->spheres[2].Set(mi->m_wheelScale, mat.GetPosition(), SURFACE_RUBBER, CAR_PIECE_WHEEL_RF);
	mat.Attach(RwFrameGetMatrix(m_aCarNodes[CAR_WHEEL_RB]));
	colModel->spheres[3].Set(mi->m_wheelScale, mat.GetPosition(), SURFACE_RUBBER, CAR_PIECE_WHEEL_RR);

	if(m_aCarNodes[CAR_WHEEL_LM] != nil && m_aCarNodes[CAR_WHEEL_RM] != nil){
		mat.Attach(RwFrameGetMatrix(m_aCarNodes[CAR_WHEEL_LM]));
		colModel->spheres[4].Set(mi->m_wheelScale, mat.GetPosition(), SURFACE_RUBBER, CAR_PIECE_WHEEL_LR);
		mat.Attach(RwFrameGetMatrix(m_aCarNodes[CAR_WHEEL_RM]));
		colModel->spheres[5].Set(mi->m_wheelScale, mat.GetPosition(), SURFACE_RUBBER, CAR_PIECE_WHEEL_RR);
		colModel->numSpheres = 6;
	}else
		colModel->numSpheres = 4;

	return true;
}

void
CAutomobile::BurstTyre(uint8 wheel, bool applyForces)
{
	if(GetModelIndex() == MI_RHINO || bTyresDontBurst)
		return;

	switch(wheel){
	case CAR_PIECE_WHEEL_LF: wheel = VEHWHEEL_FRONT_LEFT; break;
	case CAR_PIECE_WHEEL_LR: wheel = VEHWHEEL_REAR_LEFT; break;
	case CAR_PIECE_WHEEL_RF: wheel = VEHWHEEL_FRONT_RIGHT; break;
	case CAR_PIECE_WHEEL_RR: wheel = VEHWHEEL_REAR_RIGHT; break;
	}

	int status = Damage.GetWheelStatus(wheel);
	if(status == WHEEL_STATUS_OK){
		Damage.SetWheelStatus(wheel, WHEEL_STATUS_BURST);
		CStats::TyresPopped++;
		DMAudio.PlayOneShot(m_audioEntityId, SOUND_CAR_TYRE_POP, 0.0f);

		if(GetStatus() == STATUS_SIMPLE){
			SetStatus(STATUS_PHYSICS);
			CCarCtrl::SwitchVehicleToRealPhysics(this);
		}

		if(applyForces){
			ApplyMoveForce(GetRight() * m_fMass * CGeneral::GetRandomNumberInRange(-0.03f, 0.03f));
			ApplyTurnForce(GetRight() * m_fTurnMass * CGeneral::GetRandomNumberInRange(-0.03f, 0.03f), GetForward());
		}
	}
}

bool
CAutomobile::IsRoomForPedToLeaveCar(uint32 component, CVector *doorOffset)
{
	CColPoint colpoint;
	CEntity *ent;
	colpoint.point = CVector(0.0f, 0.0f, 0.0f);
	CVehicleModelInfo *mi = (CVehicleModelInfo*)CModelInfo::GetModelInfo(GetModelIndex());

	CVector seatPos;
	switch(component){
	case CAR_DOOR_RF:
		seatPos = mi->GetFrontSeatPosn();
		break;
	case CAR_DOOR_LF:
		seatPos = mi->GetFrontSeatPosn();
		seatPos.x = -seatPos.x;
		break;
	case CAR_DOOR_RR:
		seatPos = mi->m_positions[CAR_POS_BACKSEAT];
		break;
	case CAR_DOOR_LR:
		seatPos = mi->m_positions[CAR_POS_BACKSEAT];
		seatPos.x = -seatPos.x;
		break;
	}
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
CAutomobile::GetHeightAboveRoad(void)
{
	return m_fHeightAboveRoad;
}

void
CAutomobile::PlayCarHorn(void)
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
CAutomobile::PlayHornIfNecessary(void)
{
	if(AutoPilot.m_bSlowedDownBecauseOfPeds ||
	   AutoPilot.m_bSlowedDownBecauseOfCars)
		if(!HasCarStoppedBecauseOfLight())
			PlayCarHorn();
}

void
CAutomobile::ResetSuspension(void)
{
	int i;
	for(i = 0; i < 4; i++){
		m_aSuspensionSpringRatio[i] = 1.0f;
		m_aWheelTimer[i] = 0.0f;
		m_aWheelRotation[i] = 0.0f;
		m_aWheelState[i] = WHEEL_STATE_NORMAL;
	}
}

void
CAutomobile::SetupSuspensionLines(void)
{
	int i;
	CVector posn;
	CVehicleModelInfo *mi = (CVehicleModelInfo*)CModelInfo::GetModelInfo(GetModelIndex());
	CColModel *colModel = mi->GetColModel();

	// Each suspension line starts at the uppermost wheel position
	// and extends down to the lowermost point on the tyre
	for(i = 0; i < 4; i++){
		mi->GetWheelPosn(i, posn);
		m_aWheelPosition[i] = posn.z;

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

	// Compress spring somewhat to get normal height on road
	m_fHeightAboveRoad = m_aSuspensionSpringLength[0]*(1.0f - 1.0f/(4.0f*pHandling->fSuspensionForceLevel))
			- colModel->lines[0].p0.z + mi->m_wheelScale*0.5f;
	for(i = 0; i < 4; i++)
		m_aWheelPosition[i] = mi->m_wheelScale*0.5f - m_fHeightAboveRoad;

	// adjust col model to include suspension lines
	if(colModel->boundingBox.min.z > colModel->lines[0].p1.z)
		colModel->boundingBox.min.z = colModel->lines[0].p1.z;
	float radius = Max(colModel->boundingBox.min.Magnitude(), colModel->boundingBox.max.Magnitude());
	if(colModel->boundingSphere.radius < radius)
		colModel->boundingSphere.radius = radius;

	if(GetModelIndex() == MI_RCBANDIT){
		colModel->boundingSphere.radius = 2.0f;
		for(i = 0; i < colModel->numSpheres; i++)
			colModel->spheres[i].radius = 0.3f;
	}
}

// called on police cars
void
CAutomobile::ScanForCrimes(void)
{
	if(FindPlayerVehicle() && FindPlayerVehicle()->IsCar())
		if(FindPlayerVehicle()->IsAlarmOn())
			// if player's alarm is on, increase wanted level
			if((FindPlayerVehicle()->GetPosition() - GetPosition()).MagnitudeSqr() < sq(20.0f))
				CWorld::Players[CWorld::PlayerInFocus].m_pPed->SetWantedLevelNoDrop(1);
}

void
CAutomobile::BlowUpCarsInPath(void)
{
	int i;

	if(m_vecMoveSpeed.Magnitude() > 0.1f && bTankDetonateCars)
		for(i = 0; i < m_nCollisionRecords; i++)
			if(m_aCollisionRecords[i] &&
			   m_aCollisionRecords[i]->IsVehicle() &&
			   m_aCollisionRecords[i]->GetModelIndex() != MI_RHINO &&
			   !m_aCollisionRecords[i]->bRenderScorched){
				if(this == FindPlayerVehicle())
					CEventList::RegisterEvent(EVENT_EXPLOSION, EVENT_ENTITY_VEHICLE, this, FindPlayerPed(), 2000);
				((CVehicle*)m_aCollisionRecords[i])->BlowUpCar(this);
			}
}

bool
CAutomobile::HasCarStoppedBecauseOfLight(void)
{
	int i;

	if(GetStatus() != STATUS_SIMPLE && GetStatus() != STATUS_PHYSICS)
		return false;

	if(AutoPilot.m_nCurrentRouteNode && AutoPilot.m_nNextRouteNode){
		CPathNode *curnode = &ThePaths.m_pathNodes[AutoPilot.m_nCurrentRouteNode];
		for(i = 0; i < curnode->numLinks; i++)
			if(ThePaths.ConnectedNode(curnode->firstLink + i) == AutoPilot.m_nNextRouteNode)
				break;
		if(i < curnode->numLinks &&
		   ThePaths.m_carPathLinks[ThePaths.m_carPathConnections[curnode->firstLink + i]].trafficLightType & 3)
			return true;
	}

	if(AutoPilot.m_nCurrentRouteNode && AutoPilot.m_nPrevRouteNode){
		CPathNode *curnode = &ThePaths.m_pathNodes[AutoPilot.m_nCurrentRouteNode];
		for(i = 0; i < curnode->numLinks; i++)
			if(ThePaths.ConnectedNode(curnode->firstLink + i) == AutoPilot.m_nPrevRouteNode)
				break;
		if(i < curnode->numLinks &&
		   ThePaths.m_carPathLinks[ThePaths.m_carPathConnections[curnode->firstLink + i]].trafficLightType & 3)
			return true;
	}

	return false;
}

void
CAutomobile::SetBusDoorTimer(uint32 timer, uint8 type)
{
	if(timer < 1000)
		timer = 1000;
	if(type == 0)
		// open and close
		m_nBusDoorTimerStart = CTimer::GetTimeInMilliseconds();
	else
		// only close
		m_nBusDoorTimerStart = CTimer::GetTimeInMilliseconds() - 500;
	m_nBusDoorTimerEnd = m_nBusDoorTimerStart + timer;
}

void
CAutomobile::ProcessAutoBusDoors(void)
{
	if(CTimer::GetTimeInMilliseconds() < m_nBusDoorTimerEnd){
		if(m_nBusDoorTimerEnd != 0 && CTimer::GetTimeInMilliseconds() > m_nBusDoorTimerEnd-500){
			// close door
			if(!IsDoorMissing(DOOR_FRONT_LEFT) && (m_nGettingInFlags & CAR_DOOR_FLAG_LF) == 0){
				if(IsDoorClosed(DOOR_FRONT_LEFT)){
					m_nBusDoorTimerEnd = CTimer::GetTimeInMilliseconds();
					OpenDoor(CAR_DOOR_LF, DOOR_FRONT_LEFT, 0.0f);
				}else{
					OpenDoor(CAR_DOOR_LF, DOOR_FRONT_LEFT,
						1.0f - (CTimer::GetTimeInMilliseconds() - (m_nBusDoorTimerEnd-500))/500.0f);
				}
			}

			if(!IsDoorMissing(DOOR_FRONT_RIGHT) && (m_nGettingInFlags & CAR_DOOR_FLAG_RF) == 0){
				if(IsDoorClosed(DOOR_FRONT_RIGHT)){
					m_nBusDoorTimerEnd = CTimer::GetTimeInMilliseconds();
					OpenDoor(CAR_DOOR_RF, DOOR_FRONT_RIGHT, 0.0f);
				}else{
					OpenDoor(CAR_DOOR_RF, DOOR_FRONT_RIGHT,
						1.0f - (CTimer::GetTimeInMilliseconds() - (m_nBusDoorTimerEnd-500))/500.0f);
				}
			}
		}
	}else{
		// ended
		if(m_nBusDoorTimerStart){
			if(!IsDoorMissing(DOOR_FRONT_LEFT) && (m_nGettingInFlags & CAR_DOOR_FLAG_LF) == 0)
				OpenDoor(CAR_DOOR_LF, DOOR_FRONT_LEFT, 0.0f);
			if(!IsDoorMissing(DOOR_FRONT_RIGHT) && (m_nGettingInFlags & CAR_DOOR_FLAG_RF) == 0)
				OpenDoor(CAR_DOOR_RF, DOOR_FRONT_RIGHT, 0.0f);
			m_nBusDoorTimerStart = 0;
			m_nBusDoorTimerEnd = 0;
		}
	}
}

void
CAutomobile::ProcessSwingingDoor(int32 component, eDoors door)
{
	if(Damage.GetDoorStatus(door) != DOOR_STATUS_SWINGING)
		return;

	CMatrix mat(RwFrameGetMatrix(m_aCarNodes[component]));
	CVector pos = mat.GetPosition();
	float axes[3] = { 0.0f, 0.0f, 0.0f };

	Doors[door].Process(this);
	axes[Doors[door].m_nAxis] = Doors[door].m_fAngle;
	mat.SetRotate(axes[0], axes[1], axes[2]);
	mat.Translate(pos);
	mat.UpdateRW();

	// make wind rip off bonnet
	if(door == DOOR_BONNET && Doors[door].m_nDoorState == DOORST_OPEN &&
	   DotProduct(m_vecMoveSpeed, GetForward()) > 0.4f){
#ifdef FIX_BUGS
		CObject *comp = SpawnFlyingComponent(CAR_BONNET, COMPGROUP_BONNET);
#else
		CObject *comp = SpawnFlyingComponent(CAR_BONNET, COMPGROUP_DOOR);
#endif
		// make both doors invisible on car
		SetComponentVisibility(m_aCarNodes[CAR_BONNET], ATOMIC_FLAG_NONE);
		Damage.SetDoorStatus(DOOR_BONNET, DOOR_STATUS_MISSING);

		if(comp){
			if(CGeneral::GetRandomNumber() & 1)
				comp->m_vecMoveSpeed = 0.4f*m_vecMoveSpeed + 0.1f*GetRight() + 0.5f*GetUp();
			else
				comp->m_vecMoveSpeed = 0.4f*m_vecMoveSpeed - 0.1f*GetRight() + 0.5f*GetUp();
			comp->ApplyTurnForce(10.0f*GetUp(), GetForward());
		}
	}
}

void
CAutomobile::Fix(void)
{
	int component;

	Damage.ResetDamageStatus();

	if(pHandling->Flags & HANDLING_NO_DOORS){
		Damage.SetDoorStatus(DOOR_FRONT_LEFT, DOOR_STATUS_MISSING);
		Damage.SetDoorStatus(DOOR_FRONT_RIGHT, DOOR_STATUS_MISSING);
		Damage.SetDoorStatus(DOOR_REAR_LEFT, DOOR_STATUS_MISSING);
		Damage.SetDoorStatus(DOOR_REAR_RIGHT, DOOR_STATUS_MISSING);
	}

	bIsDamaged = false;
	RpClumpForAllAtomics((RpClump*)m_rwObject, CVehicleModelInfo::HideAllComponentsAtomicCB, (void*)ATOMIC_FLAG_DAM);

	for(component = CAR_BUMP_FRONT; component < NUM_CAR_NODES; component++){
		if(m_aCarNodes[component]){
			CMatrix mat(RwFrameGetMatrix(m_aCarNodes[component]));
			mat.SetTranslate(mat.GetPosition());
			mat.UpdateRW();
		}
	}

	for(component = 0; component < 4; component++)
		Damage.SetWheelStatus(component, WHEEL_STATUS_OK);

	if(GetModelIndex() == MI_HUNTER){
		RpAtomicSetFlags((RpAtomic*)GetFirstObject(m_aCarNodes[CAR_WHEEL_LB]), 0);
		RpAtomicSetFlags((RpAtomic*)GetFirstObject(m_aCarNodes[CAR_WHEEL_RB]), 0);
	}else if(IsRealHeli()){
		RpAtomicSetFlags((RpAtomic*)GetFirstObject(m_aCarNodes[CAR_WHEEL_LF]), 0);
		RpAtomicSetFlags((RpAtomic*)GetFirstObject(m_aCarNodes[CAR_WHEEL_RF]), 0);
		RpAtomicSetFlags((RpAtomic*)GetFirstObject(m_aCarNodes[CAR_WHEEL_LB]), 0);
		RpAtomicSetFlags((RpAtomic*)GetFirstObject(m_aCarNodes[CAR_WHEEL_RB]), 0);
	}
}

void
CAutomobile::SetupDamageAfterLoad(void)
{
	if(m_aCarNodes[CAR_BUMP_FRONT])
		SetBumperDamage(CAR_BUMP_FRONT, VEHBUMPER_FRONT);
	if(m_aCarNodes[CAR_BONNET])
		SetDoorDamage(CAR_BONNET, DOOR_BONNET);
	if(m_aCarNodes[CAR_BUMP_REAR])
		SetBumperDamage(CAR_BUMP_REAR, VEHBUMPER_REAR);
	if(m_aCarNodes[CAR_BOOT])
		SetDoorDamage(CAR_BOOT, DOOR_BOOT);
	if(m_aCarNodes[CAR_DOOR_LF])
		SetDoorDamage(CAR_DOOR_LF, DOOR_FRONT_LEFT);
	if(m_aCarNodes[CAR_DOOR_RF])
		SetDoorDamage(CAR_DOOR_RF, DOOR_FRONT_RIGHT);
	if(m_aCarNodes[CAR_DOOR_LR])
		SetDoorDamage(CAR_DOOR_LR, DOOR_REAR_LEFT);
	if(m_aCarNodes[CAR_DOOR_RR])
		SetDoorDamage(CAR_DOOR_RR, DOOR_REAR_RIGHT);
	if(m_aCarNodes[CAR_WING_LF])
		SetPanelDamage(CAR_WING_LF, VEHPANEL_FRONT_LEFT);
	if(m_aCarNodes[CAR_WING_RF])
		SetPanelDamage(CAR_WING_RF, VEHPANEL_FRONT_RIGHT);
	if(m_aCarNodes[CAR_WING_LR])
		SetPanelDamage(CAR_WING_LR, VEHPANEL_REAR_LEFT);
	if(m_aCarNodes[CAR_WING_RR])
		SetPanelDamage(CAR_WING_RR, VEHPANEL_REAR_RIGHT);
}

RwObject*
GetCurrentAtomicObjectCB(RwObject *object, void *data)
{
	RpAtomic *atomic = (RpAtomic*)object;
	assert(RwObjectGetType(object) == rpATOMIC);
	if(RpAtomicGetFlags(atomic) & rpATOMICRENDER)
		*(RpAtomic**)data = atomic;
	return object;
}

CObject*
CAutomobile::SpawnFlyingComponent(int32 component, uint32 type)
{
	RpAtomic *atomic;
	RwFrame *frame;
	RwMatrix *matrix;
	CObject *obj;

	if(CObject::nNoTempObjects >= NUMTEMPOBJECTS)
		return nil;

	atomic = nil;
	RwFrameForAllObjects(m_aCarNodes[component], GetCurrentAtomicObjectCB, &atomic);
	if(atomic == nil)
		return nil;

	obj = new CObject();
	if(obj == nil)
		return nil;

	if(component == CAR_WINDSCREEN){
		obj->SetModelIndexNoCreate(MI_CAR_BONNET);
	}else switch(type){
	case COMPGROUP_BUMPER:
		obj->SetModelIndexNoCreate(MI_CAR_BUMPER);
		break;
	case COMPGROUP_WHEEL:
		obj->SetModelIndexNoCreate(MI_CAR_WHEEL);
		break;
	case COMPGROUP_DOOR:
		obj->SetModelIndexNoCreate(MI_CAR_DOOR);
		obj->SetCenterOfMass(0.0f, -0.5f, 0.0f);
		obj->bDrawLast = true;
		break;
	case COMPGROUP_BONNET:
		obj->SetModelIndexNoCreate(MI_CAR_BONNET);
		obj->SetCenterOfMass(0.0f, 0.4f, 0.0f);
		break;
	case COMPGROUP_BOOT:
		obj->SetModelIndexNoCreate(MI_CAR_BOOT);
		obj->SetCenterOfMass(0.0f, -0.3f, 0.0f);
		break;
	case COMPGROUP_PANEL:
	default:
		obj->SetModelIndexNoCreate(MI_CAR_PANEL);
		break;
	}

	// object needs base model
	obj->RefModelInfo(GetModelIndex());

	// create new atomic
	matrix = RwFrameGetLTM(m_aCarNodes[component]);
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
	obj->m_fAirResistance = 0.97f;
	obj->m_fElasticity = 0.1f;
	obj->m_fBuoyancy = obj->m_fMass*GRAVITY/0.75f;
	obj->ObjectCreatedBy = TEMP_OBJECT;
	obj->bIsStatic = false;
	obj->bIsPickup = false;
	obj->bUseVehicleColours = true;
	obj->m_colour1 = m_currentColour1;
	obj->m_colour2 = m_currentColour2;

	// life time - the more objects the are, the shorter this one will live
	CObject::nNoTempObjects++;
	if(CObject::nNoTempObjects > 20)
		obj->m_nEndOfLifeTime = CTimer::GetTimeInMilliseconds() + 20000/5.0f;
	else if(CObject::nNoTempObjects > 10)
		obj->m_nEndOfLifeTime = CTimer::GetTimeInMilliseconds() + 20000/2.0f;
	else
		obj->m_nEndOfLifeTime = CTimer::GetTimeInMilliseconds() + 20000;

	obj->m_vecMoveSpeed = m_vecMoveSpeed;
	if(obj->m_vecMoveSpeed.z > 0.0f){
		obj->m_vecMoveSpeed.z *= 1.5f;
	}else if(GetUp().z > 0.0f &&
	         (component == COMPGROUP_BONNET || component == COMPGROUP_BOOT || component == CAR_WINDSCREEN)){
		obj->m_vecMoveSpeed.z *= -1.5f;
		obj->m_vecMoveSpeed.z += 0.04f;
	}else{
		obj->m_vecMoveSpeed.z *= 0.25f;
	}
	obj->m_vecMoveSpeed.x *= 0.75f;
	obj->m_vecMoveSpeed.y *= 0.75f;

	obj->m_vecTurnSpeed = m_vecTurnSpeed*2.0f;

	// push component away from car
	CVector dist = obj->GetPosition() - GetPosition();
	dist.Normalise();
	if(component == COMPGROUP_BONNET || component == COMPGROUP_BOOT || component == CAR_WINDSCREEN){
		// push these up some
		dist += GetUp();
		if(GetUp().z > 0.0f){
			// simulate fast upward movement if going fast
			float speed = CVector2D(m_vecMoveSpeed).Magnitude();
			obj->GetMatrix().Translate(GetUp()*speed);
		}
	}
	obj->ApplyMoveForce(dist);

	if(type == COMPGROUP_WHEEL){
		obj->m_fTurnMass = 5.0f;
		obj->m_vecTurnSpeed.x = 0.5f;
		obj->m_fAirResistance = 0.99f;
	}

	if(GetStatus() == STATUS_WRECKED && IsVisible() && DotProduct(dist, TheCamera.GetPosition() - GetPosition()) > -0.5f){
		dist = TheCamera.GetPosition() - GetPosition();
		dist.Normalise();
		dist.z += 0.3f;
		ApplyMoveForce(5.0f*dist);
	}

	if(CCollision::ProcessColModels(obj->GetMatrix(), *obj->GetColModel(),
			this->GetMatrix(), *this->GetColModel(),
			CWorld::m_aTempColPts, nil, nil) > 0)
		obj->m_pCollidingEntity = this;

	if(bRenderScorched)
		obj->bRenderScorched = true;

	CWorld::Add(obj);

	return obj;
}

CObject*
CAutomobile::RemoveBonnetInPedCollision(void)
{
	CObject *obj;

	if(Damage.GetDoorStatus(DOOR_BONNET) == DOOR_STATUS_SWINGING &&
	   Doors[DOOR_BONNET].RetAngleWhenOpen()*0.4f < Doors[DOOR_BONNET].m_fAngle){
#ifdef FIX_BUGS
		obj = SpawnFlyingComponent(CAR_BONNET, COMPGROUP_BONNET);
#else
		obj = SpawnFlyingComponent(CAR_BONNET, COMPGROUP_DOOR);
#endif
		// make both doors invisible on car
		SetComponentVisibility(m_aCarNodes[CAR_BONNET], ATOMIC_FLAG_NONE);
		Damage.SetDoorStatus(DOOR_BONNET, DOOR_STATUS_MISSING);
		return obj;
	}
	return nil;
}

void
CAutomobile::SetPanelDamage(int32 component, ePanels panel, bool noFlyingComponents)
{
	int status = Damage.GetPanelStatus(panel);
	if(m_aCarNodes[component] == nil)
		return;
	if(status == PANEL_STATUS_SMASHED1){
		DMAudio.PlayOneShot(m_audioEntityId, SOUND_CAR_WINDSHIELD_CRACK, 0.0f);
		// show damaged part
		SetComponentVisibility(m_aCarNodes[component], ATOMIC_FLAG_DAM);
	}else if(status == PANEL_STATUS_MISSING){
		if(!noFlyingComponents)
			SpawnFlyingComponent(component, COMPGROUP_PANEL);
		else
			CGlass::CarWindscreenShatters(this, false);
		// hide both
		SetComponentVisibility(m_aCarNodes[component], ATOMIC_FLAG_NONE);
	}
}

void
CAutomobile::SetBumperDamage(int32 component, ePanels panel, bool noFlyingComponents)
{
	int status = Damage.GetPanelStatus(panel);
	if(m_aCarNodes[component] == nil){
		printf("Trying to damage component %d of %s\n",
			component, CModelInfo::GetModelInfo(GetModelIndex())->GetName());
		return;
	}
	if(status == PANEL_STATUS_SMASHED1){
		// show damaged part
		SetComponentVisibility(m_aCarNodes[component], ATOMIC_FLAG_DAM);
	}else if(status == PANEL_STATUS_MISSING){
		if(!noFlyingComponents)
			SpawnFlyingComponent(component, COMPGROUP_BUMPER);
		// hide both
		SetComponentVisibility(m_aCarNodes[component], ATOMIC_FLAG_NONE);
	}
}

void
CAutomobile::SetDoorDamage(int32 component, eDoors door, bool noFlyingComponents)
{
	int status = Damage.GetDoorStatus(door);
	if(m_aCarNodes[component] == nil){
		printf("Trying to damage component %d of %s\n",
			component, CModelInfo::GetModelInfo(GetModelIndex())->GetName());
		return;
	}

	if(!CanDoorsBeDamaged() && status > DOOR_STATUS_SMASHED && door != DOOR_BONNET && door != DOOR_BOOT){
		Damage.SetDoorStatus(door, DOOR_STATUS_SMASHED);
		status = DOOR_STATUS_SMASHED;
	}

	if(door == DOOR_BOOT && status == DOOR_STATUS_SWINGING && pHandling->Flags & HANDLING_NOSWING_BOOT){
		Damage.SetDoorStatus(DOOR_BOOT, DOOR_STATUS_MISSING);
		status = DOOR_STATUS_MISSING;
	}

	switch(status){
	case DOOR_STATUS_SMASHED:
		// show damaged part
		SetComponentVisibility(m_aCarNodes[component], ATOMIC_FLAG_DAM);
		break;
	case DOOR_STATUS_SWINGING:
		// turn off angle cull for swinging doors
		RwFrameForAllObjects(m_aCarNodes[component], CVehicleModelInfo::SetAtomicFlagCB, (void*)ATOMIC_FLAG_NOCULL);
		break;
	case DOOR_STATUS_MISSING:
		if(!noFlyingComponents){
			if(door == DOOR_BONNET)
				SpawnFlyingComponent(component, COMPGROUP_BONNET);
			else if(door == DOOR_BOOT)
				SpawnFlyingComponent(component, COMPGROUP_BOOT);
			else
				SpawnFlyingComponent(component, COMPGROUP_DOOR);
		}
		// hide both
		SetComponentVisibility(m_aCarNodes[component], ATOMIC_FLAG_NONE);
		break;
	}
}


static RwObject*
SetVehicleAtomicVisibilityCB(RwObject *object, void *data)
{
	uint32 flags = (uint32)(uintptr)data;
	RpAtomic *atomic = (RpAtomic*)object;
	if((CVisibilityPlugins::GetAtomicId(atomic) & (ATOMIC_FLAG_OK|ATOMIC_FLAG_DAM)) == flags)
		RpAtomicSetFlags(atomic, rpATOMICRENDER);
	else
		RpAtomicSetFlags(atomic, 0);
	return object;
}

void
CAutomobile::SetComponentVisibility(RwFrame *frame, uint32 flags)
{
	HideAllComps();
	bIsDamaged = true;
	RwFrameForAllObjects(frame, SetVehicleAtomicVisibilityCB, (void*)flags);
}

void
CAutomobile::SetupModelNodes(void)
{
	int i;
	for(i = 0; i < NUM_CAR_NODES; i++)
		m_aCarNodes[i] = nil;
	CClumpModelInfo::FillFrameArray(GetClump(), m_aCarNodes);
}

void
CAutomobile::SetTaxiLight(bool light)
{
	bTaxiLight = light;
}

bool
CAutomobile::GetAllWheelsOffGround(void)
{
	return m_nDriveWheelsOnGround == 0;
}

void
CAutomobile::HideAllComps(void)
{
	// empty
}

void
CAutomobile::ShowAllComps(void)
{
	// empty
}

void
CAutomobile::ReduceHornCounter(void)
{
	if(m_nCarHornTimer != 0)
		m_nCarHornTimer--;
}

void
CAutomobile::SetAllTaxiLights(bool set)
{
	m_sAllTaxiLights = set;
}

void
CAutomobile::TellHeliToGoToCoors(float x, float y, float z, uint8 speed)
{
	AutoPilot.m_nCarMission = MISSION_HELI_FLYTOCOORS;
	AutoPilot.m_vecDestinationCoors.x = x;
	AutoPilot.m_vecDestinationCoors.y = y;
	AutoPilot.m_vecDestinationCoors.z = z;
	AutoPilot.m_nCruiseSpeed = speed;
	SetStatus(STATUS_PHYSICS);

	if(m_fOrientation == 0.0f){
		m_fOrientation = CGeneral::GetATanOfXY(GetForward().x, GetForward().y) + TWOPI;
		while(m_fOrientation > TWOPI) m_fOrientation -= TWOPI;
	}
}

void
CAutomobile::TellPlaneToGoToCoors(float x, float y, float z, uint8 speed)
{
	AutoPilot.m_nCarMission = MISSION_HELI_FLYTOCOORS;
	AutoPilot.m_vecDestinationCoors.x = x;
	AutoPilot.m_vecDestinationCoors.y = y;
	AutoPilot.m_vecDestinationCoors.z = z;
	AutoPilot.m_nCruiseSpeed = speed;
	SetStatus(STATUS_PHYSICS);

	if(m_fOrientation == 0.0f)
		m_fOrientation = CGeneral::GetATanOfXY(GetForward().x, GetForward().y);
}

void
CAutomobile::PopBoot(void)
{
	switch(Damage.GetDoorStatus(DOOR_BOOT)){
	case DOOR_STATUS_OK:
	case DOOR_STATUS_SMASHED:
		Doors[DOOR_BOOT].m_fAngle = Doors[DOOR_BOOT].m_fMinAngle;
		CMatrix mat(RwFrameGetMatrix(m_aCarNodes[DOOR_BOOT]));
		CVector pos = mat.GetPosition();
		float axes[3] = { 0.0f, 0.0f, 0.0f };
		axes[Doors[DOOR_BOOT].m_nAxis] = Doors[DOOR_BOOT].m_fAngle;
		mat.SetRotate(axes[0], axes[1], axes[2]);
		mat.Translate(pos);
		mat.UpdateRW();
	}
}

void
CAutomobile::PopBootUsingPhysics(void)
{
	switch(Damage.GetDoorStatus(DOOR_BOOT))
	case DOOR_STATUS_OK:
	case DOOR_STATUS_SMASHED:
		Damage.SetDoorStatus(DOOR_BOOT, DOOR_STATUS_SWINGING);
	Doors[DOOR_BOOT].m_fAngle = -2.0f;
}

void
CAutomobile::CloseAllDoors(void)
{
	CVehicleModelInfo *mi = (CVehicleModelInfo*)CModelInfo::GetModelInfo(GetModelIndex());
	if(!IsDoorMissing(DOOR_FRONT_LEFT))
		OpenDoor(CAR_DOOR_LF, DOOR_FRONT_LEFT, 0.0f);
	if(mi->m_numDoors > 1){
		if(!IsDoorMissing(DOOR_FRONT_RIGHT))
			OpenDoor(CAR_DOOR_RF, DOOR_FRONT_RIGHT, 0.0f);
		if(mi->m_numDoors > 2){
			if(!IsDoorMissing(DOOR_REAR_LEFT))
				OpenDoor(CAR_DOOR_LR, DOOR_REAR_LEFT, 0.0f);
			if(!IsDoorMissing(DOOR_REAR_RIGHT))
				OpenDoor(CAR_DOOR_RR, DOOR_REAR_RIGHT, 0.0f);
		}
	}
}

void
CAutomobile::KnockPedOutCar(eWeaponType weapon, uint16 door, CPed *ped)
{
	AnimationId anim = ANIM_KO_SHOT_FRONT1;
	if(ped == nil)
		return;

	ped->m_vehEnterType = door;
	ped->SetPedState(PED_IDLE);
	CAnimManager::BlendAnimation(ped->GetClump(), ped->m_animGroup, ANIM_IDLE_STANCE, 100.0f);
	CPed::PedSetOutCarCB(nil, ped);
	ped->SetMoveState(PEDMOVE_STILL);
	if(GetUp().z < 0.0f)
		ped->SetHeading(CGeneral::LimitRadianAngle(GetForward().Heading() + PI));
	else
		ped->SetHeading(GetForward().Heading());

	switch(weapon){
	case WEAPONTYPE_UNARMED:
	case WEAPONTYPE_UNIDENTIFIED:
		ped->m_vecMoveSpeed = m_vecMoveSpeed;
		ped->m_pCollidingEntity = this;
		anim = NUM_STD_ANIMS;
		break;

	case WEAPONTYPE_BASEBALLBAT:
	case WEAPONTYPE_RAMMEDBYCAR:
	case WEAPONTYPE_FALL:
		ped->m_vecMoveSpeed = m_vecMoveSpeed;
		anim = ANIM_KD_LEFT;
		ApplyMoveForce(4.0f*GetUp() + 8.0f*GetRight());
		break;
	}

	if(weapon != WEAPONTYPE_UNARMED){
		ped->SetFall(1000, anim, 0);
		ped->bIsStanding = false;
		ped->m_headingRate = 0.0f;
	}
	ped->m_pMyVehicle = nil;
}

#ifdef COMPATIBLE_SAVES
void
CAutomobile::Save(uint8*& buf)
{
	CVehicle::Save(buf);
	WriteSaveBuf<CDamageManager>(buf, Damage);
	SkipSaveBuf(buf, 800 - sizeof(CDamageManager));
}

void
CAutomobile::Load(uint8*& buf)
{
	CVehicle::Load(buf);
	Damage = ReadSaveBuf<CDamageManager>(buf);
	SkipSaveBuf(buf, 800 - sizeof(CDamageManager));
	SetupDamageAfterLoad();
}
#endif
