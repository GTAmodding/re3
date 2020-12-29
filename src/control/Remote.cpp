#include "common.h"

#include "vehicle_enums.h" //needs to be before Vehicle.h

#include "Automobile.h"
#include "Camera.h"
#include "CarCtrl.h"
#include "PlayerInfo.h"
#include "Remote.h"
#include "Timer.h"
#include "Vehicle.h"
#include "World.h"

//--MIAMI: file done

void
CRemote::GivePlayerRemoteControlledCar(float x, float y, float z, float rot, uint16 model)
{
	CAutomobile *car = new CAutomobile(model, MISSION_VEHICLE);
	bool found;

	z = car->GetDistanceFromCentreOfMassToBaseOfModel() + CWorld::FindGroundZFor3DCoord(x, y, z + 2.0f, &found);

	car->GetMatrix().SetRotateZOnly(rot);
	car->SetPosition(x, y, z);
	car->SetStatus(STATUS_PLAYER_REMOTE);
	car->bIsLocked = true;

	CCarCtrl::JoinCarWithRoadSystem(car);
	car->AutoPilot.m_nCarMission = MISSION_NONE;
	car->AutoPilot.m_nTempAction = TEMPACT_NONE;
	car->AutoPilot.m_nDrivingStyle = DRIVINGSTYLE_STOP_FOR_CARS;
	car->AutoPilot.m_nCruiseSpeed = car->AutoPilot.m_fMaxTrafficSpeed = 9.0f;
	car->AutoPilot.m_nNextLane = car->AutoPilot.m_nCurrentLane = 0;
	car->bEngineOn = true;
	CWorld::Add(car);
	if (FindPlayerVehicle() != nil) 
		FindPlayerVehicle()->SetStatus(STATUS_PLAYER_DISABLED);

	CWorld::Players[CWorld::PlayerInFocus].m_pRemoteVehicle = car;
	CWorld::Players[CWorld::PlayerInFocus].m_pRemoteVehicle->RegisterReference((CEntity**)&CWorld::Players[CWorld::PlayerInFocus].m_pRemoteVehicle);
	if (car->GetVehicleAppearance() == VEHICLE_APPEARANCE_PLANE || car->GetVehicleAppearance() == VEHICLE_APPEARANCE_HELI) {
		TheCamera.TakeControl(car, CCam::MODE_CAM_ON_A_STRING, INTERPOLATION, CAMCONTROL_SCRIPT);
		TheCamera.SetZoomValueCamStringScript(0);
	} else
		TheCamera.TakeControl(car, CCam::MODE_BEHINDCAR, INTERPOLATION, CAMCONTROL_SCRIPT);
}

void
CRemote::TakeRemoteControlledCarFromPlayer(bool blowUp)
{
	if (CWorld::Players[CWorld::PlayerInFocus].m_pRemoteVehicle->VehicleCreatedBy == MISSION_VEHICLE) {
		CWorld::Players[CWorld::PlayerInFocus].m_pRemoteVehicle->VehicleCreatedBy = RANDOM_VEHICLE;
		CCarCtrl::NumMissionCars--;
		CCarCtrl::NumRandomCars++;
	}
	CWorld::Players[CWorld::PlayerInFocus].m_pRemoteVehicle->bIsLocked = false;
	CWorld::Players[CWorld::PlayerInFocus].m_nTimeLostRemoteCar = CTimer::GetTimeInMilliseconds();
	CWorld::Players[CWorld::PlayerInFocus].m_bInRemoteMode = true;
	CWorld::Players[CWorld::PlayerInFocus].field_D5 = blowUp;
	CWorld::Players[CWorld::PlayerInFocus].field_D6 = true;
}
