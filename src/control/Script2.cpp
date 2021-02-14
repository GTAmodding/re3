#include "common.h"

#include "Script.h"
#include "ScriptCommands.h"

#include "Camera.h"
#include "CarCtrl.h"
#include "CarGen.h"
#include "CivilianPed.h"
#include "CopPed.h"
#include "Cranes.h"
#include "DMAudio.h"
#include "EmergencyPed.h"
#include "Garages.h"
#include "General.h"
#include "Messages.h"
#include "Pad.h"
#include "PedRoutes.h"
#include "Pools.h"
#include "Population.h"
#include "Radar.h"
#include "Restart.h"
#include "Shadows.h"
#include "User.h"
#include "Wanted.h"
#include "WaterLevel.h"
#include "Weather.h"
#include "World.h"
#include "Zones.h"

int8 CRunningScript::ProcessCommands300To399(int32 command)
{
	switch (command) {
	/* Not implemented.
	case COMMAND_SET_CHAR_INVINCIBLE:
	case COMMAND_SET_PLAYER_INVINCIBLE:
	case COMMAND_SET_CHAR_GRAPHIC_TYPE:
	case COMMAND_SET_PLAYER_GRAPHIC_TYPE:
	*/
	case COMMAND_HAS_PLAYER_BEEN_ARRESTED:
		CollectParameters(&m_nIp, 1);
		UpdateCompareFlag(CWorld::Players[ScriptParams[0]].m_WBState == WBSTATE_BUSTED);
		return 0;
	/* Not implemented.
	case COMMAND_STOP_CHAR_DRIVING:
	case COMMAND_KILL_CHAR:
	case COMMAND_SET_FAVOURITE_CAR_MODEL_FOR_CHAR:
	case COMMAND_SET_CHAR_OCCUPATION:
	*/
	case COMMAND_CHANGE_CAR_LOCK:
	{
		CollectParameters(&m_nIp, 2);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		script_assert(pVehicle);
		pVehicle->m_nDoorLock = (eCarLock)ScriptParams[1];
		return 0;
	}
	case COMMAND_SHAKE_CAM_WITH_POINT:
		CollectParameters(&m_nIp, 4);
		TheCamera.CamShake(ScriptParams[0] / 1000.0f,
			*(float*)&ScriptParams[1],
			*(float*)&ScriptParams[2],
			*(float*)&ScriptParams[3]);
		return 0;
	case COMMAND_IS_CAR_MODEL:
	{
		CollectParameters(&m_nIp, 2);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		script_assert(pVehicle);
		UpdateCompareFlag(pVehicle->GetModelIndex() == ScriptParams[1]);
		return 0;
	}
	/* Not implemented.
	case COMMAND_IS_CAR_REMAP:
	case COMMAND_HAS_CAR_JUST_SUNK:
	case COMMAND_SET_CAR_NO_COLLIDE:
	*/
	case COMMAND_IS_CAR_DEAD_IN_AREA_2D:
	{
		CollectParameters(&m_nIp, 6);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		script_assert(pVehicle);
		float x1 = *(float*)&ScriptParams[1];
		float y1 = *(float*)&ScriptParams[2];
		float x2 = *(float*)&ScriptParams[3];
		float y2 = *(float*)&ScriptParams[4];
		UpdateCompareFlag(pVehicle->GetStatus() == STATUS_WRECKED &&
			pVehicle->IsWithinArea(x1, y1, x2, y2));
		if (ScriptParams[5])
			CTheScripts::HighlightImportantArea((uintptr)this + m_nIp, x1, y1, x2, y2, MAP_Z_LOW_LIMIT);
		if (CTheScripts::DbgFlag)
			CTheScripts::DrawDebugSquare(x1, y1, x2, y2);
		return 0;
	}
	case COMMAND_IS_CAR_DEAD_IN_AREA_3D:
	{
		CollectParameters(&m_nIp, 8);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		script_assert(pVehicle);
		float x1 = *(float*)&ScriptParams[1];
		float y1 = *(float*)&ScriptParams[2];
		float z1 = *(float*)&ScriptParams[3];
		float x2 = *(float*)&ScriptParams[4];
		float y2 = *(float*)&ScriptParams[5];
		float z2 = *(float*)&ScriptParams[6];
		UpdateCompareFlag(pVehicle->GetStatus() == STATUS_WRECKED &&
			pVehicle->IsWithinArea(x1, y1, z1, x2, y2, z2));
		if (ScriptParams[7])
			CTheScripts::HighlightImportantArea((uintptr)this + m_nIp, x1, y1, x2, y2, (z1 + z2) / 2);
		if (CTheScripts::DbgFlag)
			CTheScripts::DrawDebugCube(x1, y1, z1, x2, y2, z2);
		return 0;
	}
	/* Not implemented.
	case COMMAND_IS_TRAILER_ATTACHED:
	case COMMAND_IS_CAR_ON_TRAILER:
	case COMMAND_HAS_CAR_GOT_WEAPON:
	case COMMAND_PARK:
	case COMMAND_HAS_PARK_FINISHED:
	case COMMAND_KILL_ALL_PASSENGERS:
	case COMMAND_SET_CAR_BULLETPROOF:
	case COMMAND_SET_CAR_FLAMEPROOF:
	case COMMAND_SET_CAR_ROCKETPROOF:
	case COMMAND_IS_CARBOMB_ACTIVE:
	case COMMAND_GIVE_CAR_ALARM:
	case COMMAND_PUT_CAR_ON_TRAILER:
	*/
	case COMMAND_IS_CAR_CRUSHED:
		CollectParameters(&m_nIp, 1);
		UpdateCompareFlag(CGarages::HasCarBeenCrushed(ScriptParams[0]));
		return 0;
	/* Not implemented.
	case COMMAND_CREATE_GANG_CAR:
	*/
	case COMMAND_CREATE_CAR_GENERATOR:
		CollectParameters(&m_nIp, 12);
		ScriptParams[0] = CTheCarGenerators::CreateCarGenerator(
			*(float*)&ScriptParams[0], *(float*)&ScriptParams[1], *(float*)&ScriptParams[2], *(float*)&ScriptParams[3],
			ScriptParams[4], ScriptParams[5], ScriptParams[6], ScriptParams[7],
			ScriptParams[8], ScriptParams[9], ScriptParams[10], ScriptParams[11]);
		StoreParameters(&m_nIp, 1);
		return 0;
	case COMMAND_SWITCH_CAR_GENERATOR:
	{
		CollectParameters(&m_nIp, 2);
		CCarGenerator* pCarGen = &CTheCarGenerators::CarGeneratorArray[ScriptParams[0]];
		if (ScriptParams[1] == 0){
			pCarGen->SwitchOff();
		}else if (ScriptParams[1] <= 100){
			pCarGen->SwitchOn();
			pCarGen->SetUsesRemaining(ScriptParams[1]);
		}else{
			pCarGen->SwitchOn();
		}
		return 0;
	}
	case COMMAND_ADD_PAGER_MESSAGE:
	{
		wchar* text = CTheScripts::GetTextByKeyFromScript(&m_nIp);
		CollectParameters(&m_nIp, 3);
		CUserDisplay::Pager.AddMessage(text, ScriptParams[0], ScriptParams[1], ScriptParams[2]);
		return 0;
	}
	case COMMAND_DISPLAY_ONSCREEN_TIMER:
	{
		script_assert(CTheScripts::ScriptSpace[m_nIp] == ARGUMENT_GLOBALVAR);
		m_nIp++;
		CUserDisplay::OnscnTimer.AddClock((uint16)CTheScripts::Read2BytesFromScript(&m_nIp), nil);
		return 0;
	}
	case COMMAND_CLEAR_ONSCREEN_TIMER:
	{
		script_assert(CTheScripts::ScriptSpace[m_nIp] == ARGUMENT_GLOBALVAR);
		m_nIp++;
		CUserDisplay::OnscnTimer.ClearClock((uint16)CTheScripts::Read2BytesFromScript(&m_nIp));
		return 0;
	}
	case COMMAND_DISPLAY_ONSCREEN_COUNTER:
	{
		script_assert(CTheScripts::ScriptSpace[m_nIp] == ARGUMENT_GLOBALVAR);
		m_nIp++;
		uint16 counter = CTheScripts::Read2BytesFromScript(&m_nIp);
		CollectParameters(&m_nIp, 1);
		CUserDisplay::OnscnTimer.AddCounter(counter, ScriptParams[0], nil);
		return 0;
	}
	case COMMAND_CLEAR_ONSCREEN_COUNTER:
	{
		script_assert(CTheScripts::ScriptSpace[m_nIp] == ARGUMENT_GLOBALVAR);
		m_nIp++;
		CUserDisplay::OnscnTimer.ClearCounter((uint16)CTheScripts::Read2BytesFromScript(&m_nIp));
		return 0;
	}
	case COMMAND_SET_ZONE_CAR_INFO:
	{
		char label[12];
		CTheScripts::ReadTextLabelFromScript(&m_nIp, label);
		m_nIp += KEY_LENGTH_IN_SCRIPT;
		CollectParameters(&m_nIp, 16);
		int zone = CTheZones::FindZoneByLabelAndReturnIndex(label);
		if (zone < 0) {
			debug("Couldn't find zone - %s\n", label);
			return 0;
		}
		CTheZones::SetZoneCarInfo(zone, ScriptParams[0], ScriptParams[1], ScriptParams[2], ScriptParams[3],
			ScriptParams[4], ScriptParams[5], ScriptParams[6], ScriptParams[7], ScriptParams[8], 0, 0,
			ScriptParams[9], ScriptParams[10], ScriptParams[11], ScriptParams[12],
			ScriptParams[13], ScriptParams[14], ScriptParams[15]);
		return 0;
	}
	/* Not implemented.
	case COMMAND_IS_CHAR_IN_GANG_ZONE:
	*/
	case COMMAND_IS_CHAR_IN_ZONE:
	{
		CollectParameters(&m_nIp, 1);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		char label[12];
		CTheScripts::ReadTextLabelFromScript(&m_nIp, label);
		int zone = CTheZones::FindZoneByLabelAndReturnIndex(label);
		if (zone != -1)
			m_nIp += KEY_LENGTH_IN_SCRIPT;
		CVector pos = pPed->bInVehicle ? pPed->m_pMyVehicle->GetPosition() : pPed->GetPosition();
		UpdateCompareFlag(CTheZones::PointLiesWithinZone(&pos, CTheZones::GetZone(zone)));
		return 0;
	}
	case COMMAND_SET_CAR_DENSITY:
	{
		char label[12];
		CTheScripts::ReadTextLabelFromScript(&m_nIp, label);
		int16 zone = CTheZones::FindZoneByLabelAndReturnIndex(label);
		m_nIp += 8;
		CollectParameters(&m_nIp, 2);
		if (zone < 0) {
			debug("Couldn't find zone - %s\n", label);
			return 0;
		}
		CTheZones::SetCarDensity(zone, ScriptParams[0], ScriptParams[1]);
		return 0;
	}
	case COMMAND_SET_PED_DENSITY:
	{
		char label[12];
		CTheScripts::ReadTextLabelFromScript(&m_nIp, label);
		int16 zone = CTheZones::FindZoneByLabelAndReturnIndex(label);
		m_nIp += KEY_LENGTH_IN_SCRIPT;
		CollectParameters(&m_nIp, 2);
		if (zone < 0) {
			debug("Couldn't find zone - %s\n", label);
			return 0;
		}
		CTheZones::SetPedDensity(zone, ScriptParams[0], ScriptParams[1]);
		return 0;
	}
	case COMMAND_POINT_CAMERA_AT_PLAYER:
	{
		CollectParameters(&m_nIp, 3);
		// ScriptParams[0] is unused.
		TheCamera.TakeControl(nil, ScriptParams[1], ScriptParams[2], CAMCONTROL_SCRIPT);
		return 0;
	}
	case COMMAND_POINT_CAMERA_AT_CAR:
	{
		CollectParameters(&m_nIp, 3);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		script_assert(pVehicle);
		TheCamera.TakeControl(pVehicle, ScriptParams[1], ScriptParams[2], CAMCONTROL_SCRIPT);
		return 0;
	}
	case COMMAND_POINT_CAMERA_AT_CHAR:
	{
		CollectParameters(&m_nIp, 3);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		TheCamera.TakeControl(pPed, ScriptParams[1], ScriptParams[2], CAMCONTROL_SCRIPT);
		return 0;
	}
	case COMMAND_RESTORE_CAMERA:
		TheCamera.Restore();
		return 0;
	case COMMAND_SHAKE_PAD:
		CPad::GetPad(ScriptParams[0])->StartShake(ScriptParams[1], ScriptParams[2]);
		return 0;
	case COMMAND_SET_ZONE_PED_INFO:
	{
		char label[12];
		CTheScripts::ReadTextLabelFromScript(&m_nIp, label);
		m_nIp += KEY_LENGTH_IN_SCRIPT;
		CollectParameters(&m_nIp, 10);
		int16 zone = CTheZones::FindZoneByLabelAndReturnIndex(label);
		if (zone < 0) {
			debug("Couldn't find zone - %s\n", label);
			return 0;
		}
		CTheZones::SetZonePedInfo(zone, ScriptParams[0], ScriptParams[1], ScriptParams[2], ScriptParams[3],
			ScriptParams[4], ScriptParams[5], ScriptParams[6], ScriptParams[7], ScriptParams[8], 0, 0, ScriptParams[9]);
		return 0;
	}
	case COMMAND_SET_TIME_SCALE:
		CollectParameters(&m_nIp, 1);
		CTimer::SetTimeScale(*(float*)&ScriptParams[0]);
		return 0;
	case COMMAND_IS_CAR_IN_AIR:
	{
		CollectParameters(&m_nIp, 1);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		script_assert(pVehicle && pVehicle->IsCar());
		CAutomobile* pCar = (CAutomobile*)pVehicle;
		UpdateCompareFlag(pCar->GetAllWheelsOffGround());
		return 0;
	}
	case COMMAND_SET_FIXED_CAMERA_POSITION:
	{
		CollectParameters(&m_nIp, 6);
		TheCamera.SetCamPositionForFixedMode(
			CVector(*(float*)&ScriptParams[0], *(float*)&ScriptParams[1], *(float*)&ScriptParams[2]),
			CVector(*(float*)&ScriptParams[3], *(float*)&ScriptParams[4], *(float*)&ScriptParams[5]));
		return 0;
	}
	case COMMAND_POINT_CAMERA_AT_POINT:
	{
		CollectParameters(&m_nIp, 4);
		CVector pos = *(CVector*)&ScriptParams[0];
		if (pos.z <= MAP_Z_LOW_LIMIT)
			pos.z = CWorld::FindGroundZForCoord(pos.x, pos.y);
		TheCamera.TakeControlNoEntity(pos, ScriptParams[3], CAMCONTROL_SCRIPT);
		return 0;
	}
	case COMMAND_ADD_BLIP_FOR_CAR_OLD:
	{
		CollectParameters(&m_nIp, 3);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		script_assert(pVehicle);
		// Useless call.
		CRadar::GetActualBlipArrayIndex(CollectNextParameterWithoutIncreasingPC(m_nIp));
		ScriptParams[0] = CRadar::SetEntityBlip(BLIP_CAR, ScriptParams[0], ScriptParams[1], (eBlipDisplay)ScriptParams[2]);
		StoreParameters(&m_nIp, 1);
		return 0;
	}
	case COMMAND_ADD_BLIP_FOR_CHAR_OLD:
	{
		CollectParameters(&m_nIp, 3);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		// Useless call.
		CRadar::GetActualBlipArrayIndex(CollectNextParameterWithoutIncreasingPC(m_nIp));
		ScriptParams[0] = CRadar::SetEntityBlip(BLIP_CHAR, ScriptParams[0], ScriptParams[1], (eBlipDisplay)ScriptParams[2]);
		StoreParameters(&m_nIp, 1);
		return 0;
	}
	case COMMAND_ADD_BLIP_FOR_OBJECT_OLD:
	{
		CollectParameters(&m_nIp, 3);
		CObject* pObject = CPools::GetObjectPool()->GetAt(ScriptParams[0]);
		script_assert(pObject);
		// Useless call.
		CRadar::GetActualBlipArrayIndex(CollectNextParameterWithoutIncreasingPC(m_nIp));
		ScriptParams[0] = CRadar::SetEntityBlip(BLIP_OBJECT, ScriptParams[0], ScriptParams[1], (eBlipDisplay)ScriptParams[2]);
		StoreParameters(&m_nIp, 1);
		return 0;
	}
	case COMMAND_REMOVE_BLIP:
		CollectParameters(&m_nIp, 1);
		CRadar::ClearBlip(ScriptParams[0]);
		return 0;
	case COMMAND_CHANGE_BLIP_COLOUR:
		CollectParameters(&m_nIp, 2);
		CRadar::ChangeBlipColour(ScriptParams[0], ScriptParams[1]);
		return 0;
	case COMMAND_DIM_BLIP:
		CollectParameters(&m_nIp, 2);
		CRadar::ChangeBlipBrightness(ScriptParams[0], ScriptParams[1]);
		return 0;
	case COMMAND_ADD_BLIP_FOR_COORD_OLD:
	{
		CollectParameters(&m_nIp, 5);
		CVector pos = *(CVector*)&ScriptParams[0];
		if (pos.z <= MAP_Z_LOW_LIMIT)
			pos.z = CWorld::FindGroundZForCoord(pos.x, pos.y);
		// Useless call
		CRadar::GetActualBlipArrayIndex(CollectNextParameterWithoutIncreasingPC(m_nIp));
		ScriptParams[0] = CRadar::SetCoordBlip(BLIP_COORD, pos, ScriptParams[3], (eBlipDisplay)ScriptParams[4]);
		StoreParameters(&m_nIp, 1);
		return 0;
	}
	case COMMAND_CHANGE_BLIP_SCALE:
		CollectParameters(&m_nIp, 2);
		CRadar::ChangeBlipScale(ScriptParams[0], ScriptParams[1]);
		return 0;
	case COMMAND_SET_FADING_COLOUR:
		CollectParameters(&m_nIp, 3);
		TheCamera.SetFadeColour(ScriptParams[0], ScriptParams[1], ScriptParams[2]);
		return 0;
	case COMMAND_DO_FADE:
		CollectParameters(&m_nIp, 2);
		TheCamera.Fade(ScriptParams[0] / 1000.0f, ScriptParams[1]);
		return 0;
	case COMMAND_GET_FADING_STATUS:
		UpdateCompareFlag(TheCamera.GetFading());
		return 0;
	case COMMAND_ADD_HOSPITAL_RESTART:
	{
		CollectParameters(&m_nIp, 4);
		CVector pos = *(CVector*)&ScriptParams[0];
		float angle = *(float*)&ScriptParams[3];
		if (pos.z <= MAP_Z_LOW_LIMIT)
			pos.z = CWorld::FindGroundZForCoord(pos.x, pos.y);
		CRestart::AddHospitalRestartPoint(pos, angle);
		return 0;
	}
	case COMMAND_ADD_POLICE_RESTART:
	{
		CollectParameters(&m_nIp, 4);
		CVector pos = *(CVector*)&ScriptParams[0];
		float angle = *(float*)&ScriptParams[3];
		if (pos.z <= MAP_Z_LOW_LIMIT)
			pos.z = CWorld::FindGroundZForCoord(pos.x, pos.y);
		CRestart::AddPoliceRestartPoint(pos, angle);
		return 0;
	}
	case COMMAND_OVERRIDE_NEXT_RESTART:
	{
		CollectParameters(&m_nIp, 4);
		CVector pos = *(CVector*)&ScriptParams[0];
		float angle = *(float*)&ScriptParams[3];
		if (pos.z <= MAP_Z_LOW_LIMIT)
			pos.z = CWorld::FindGroundZForCoord(pos.x, pos.y);
		CRestart::OverrideNextRestart(pos, angle);
		return 0;
	}
	case COMMAND_DRAW_SHADOW:
	{
		CollectParameters(&m_nIp, 10);
		CVector pos = *(CVector*)&ScriptParams[1];
		float angle = *(float*)&ScriptParams[4];
		float length = *(float*)&ScriptParams[5];
		float x, y;
		if (angle != 0.0f){
			y = cos(angle) * length;
			x = sin(angle) * length;
		}else{
			y = length;
			x = 0.0f;
		}
		float frontX = -x;
		float frontY = y;
		float sideX = y;
		float sideY = x;
		/* Not very nicely named intermediate variables. */
		CShadows::StoreShadowToBeRendered(ScriptParams[0], &pos, frontX, frontY, sideX, sideY,
			ScriptParams[6], ScriptParams[7], ScriptParams[8], ScriptParams[9]);
		return 0;
	}
	case COMMAND_GET_PLAYER_HEADING:
	{
		CollectParameters(&m_nIp, 1);
		CPed* pPed = CWorld::Players[ScriptParams[0]].m_pPed;
		float angle = pPed->bInVehicle ? pPed->m_pMyVehicle->GetForward().Heading() : pPed->GetForward().Heading();
		*(float*)&ScriptParams[0] = CGeneral::LimitAngle(RADTODEG(angle));
		StoreParameters(&m_nIp, 1);
		return 0;
	}
	case COMMAND_SET_PLAYER_HEADING:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CWorld::Players[ScriptParams[0]].m_pPed;
		if (pPed->bInVehicle){
			// Is script_assertion required?
			return 0;
		}
		pPed->m_fRotationDest = pPed->m_fRotationCur = DEGTORAD(*(float*)&ScriptParams[1]);
		pPed->SetHeading(DEGTORAD(*(float*)&ScriptParams[1]));
		return 0;
	}
	case COMMAND_GET_CHAR_HEADING:
	{
		CollectParameters(&m_nIp, 1);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		float angle = pPed->bInVehicle ? pPed->m_pMyVehicle->GetForward().Heading() : pPed->GetForward().Heading();
		*(float*)&ScriptParams[0] = CGeneral::LimitAngle(RADTODEG(angle));
		StoreParameters(&m_nIp, 1);
		return 0;
	}
	case COMMAND_SET_CHAR_HEADING:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		if (pPed->bInVehicle) {
			// Is script_assertion required?
			return 0;
		}
		pPed->m_fRotationDest = pPed->m_fRotationCur = DEGTORAD(*(float*)&ScriptParams[1]);
		pPed->SetHeading(DEGTORAD(*(float*)&ScriptParams[1]));
		return 0;
	}
	case COMMAND_GET_CAR_HEADING:
	{
		CollectParameters(&m_nIp, 1);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		script_assert(pVehicle);
		float angle = pVehicle->GetForward().Heading();
		*(float*)&ScriptParams[0] = CGeneral::LimitAngle(RADTODEG(angle));
		StoreParameters(&m_nIp, 1);
		return 0;
	}
	case COMMAND_SET_CAR_HEADING:
	{
		CollectParameters(&m_nIp, 2);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		script_assert(pVehicle);
		pVehicle->SetHeading(DEGTORAD(*(float*)&ScriptParams[1]));
		return 0;
	}
	case COMMAND_GET_OBJECT_HEADING:
	{
		CollectParameters(&m_nIp, 1);
		CObject* pObject = CPools::GetObjectPool()->GetAt(ScriptParams[0]);
		script_assert(pObject);
		float angle = pObject->GetForward().Heading();
		*(float*)&ScriptParams[0] = CGeneral::LimitAngle(RADTODEG(angle));
		StoreParameters(&m_nIp, 1);
		return 0;
	}
	case COMMAND_SET_OBJECT_HEADING:
	{
		CollectParameters(&m_nIp, 2);
		CObject* pObject = CPools::GetObjectPool()->GetAt(ScriptParams[0]);
		script_assert(pObject);
		CWorld::Remove(pObject);
		pObject->SetHeading(DEGTORAD(*(float*)&ScriptParams[1]));
		pObject->GetMatrix().UpdateRW();
		pObject->UpdateRwFrame();
		CWorld::Add(pObject);
		return 0;
	}
	case COMMAND_IS_PLAYER_TOUCHING_OBJECT:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CWorld::Players[ScriptParams[0]].m_pPed;
		CObject* pObject = CPools::GetObjectPool()->GetAt(ScriptParams[1]);
		script_assert(pObject);
		CPhysical* pEntityToTest = pPed->bInVehicle ? (CPhysical*)pPed->m_pMyVehicle : pPed;
		UpdateCompareFlag(pEntityToTest->GetHasCollidedWith(pObject));
		return 0;
	}
	case COMMAND_IS_CHAR_TOUCHING_OBJECT:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		CObject* pObject = CPools::GetObjectPool()->GetAt(ScriptParams[1]);
		script_assert(pObject);
		CPhysical* pEntityToTest = pPed->bInVehicle ? (CPhysical*)pPed->m_pMyVehicle : pPed;
		UpdateCompareFlag(pEntityToTest->GetHasCollidedWith(pObject));
		return 0;
	}
	case COMMAND_SET_PLAYER_AMMO:
	{
		CollectParameters(&m_nIp, 3);
		CWorld::Players[0].m_pPed->SetAmmo((eWeaponType)ScriptParams[1], ScriptParams[2]);
		return 0;
	}
	case COMMAND_SET_CHAR_AMMO:
	{
		CollectParameters(&m_nIp, 3);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		pPed->SetAmmo((eWeaponType)ScriptParams[1], ScriptParams[2]);
		return 0;
	}
	/* Not implemented.
	case COMMAND_SET_CAR_AMMO:
	case COMMAND_LOAD_CAMERA_SPLINE:
	case COMMAND_MOVE_CAMERA_ALONG_SPLINE:
	case COMMAND_GET_CAMERA_POSITION_ALONG_SPLINE:
	*/
	case COMMAND_DECLARE_MISSION_FLAG:
		CTheScripts::OnAMissionFlag = (uint16)CTheScripts::Read2BytesFromScript(&++m_nIp);
		return 0;
	case COMMAND_DECLARE_MISSION_FLAG_FOR_CONTACT:
		CollectParameters(&m_nIp, 1);
		CTheScripts::OnAMissionForContactFlag[ScriptParams[0]] = (uint16)CTheScripts::Read2BytesFromScript(&++m_nIp);
		return 0;
	case COMMAND_DECLARE_BASE_BRIEF_ID_FOR_CONTACT:
		CollectParameters(&m_nIp, 2);
		CTheScripts::BaseBriefIdForContact[ScriptParams[0]] = ScriptParams[1];
		return 0;
	case COMMAND_IS_PLAYER_HEALTH_GREATER:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CWorld::Players[ScriptParams[0]].m_pPed;
		UpdateCompareFlag(pPed->m_fHealth > ScriptParams[1]);
		return 0;
	}
	case COMMAND_IS_CHAR_HEALTH_GREATER:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		UpdateCompareFlag(pPed->m_fHealth > ScriptParams[1]);
		return 0;
	}
	case COMMAND_IS_CAR_HEALTH_GREATER:
	{
		CollectParameters(&m_nIp, 2);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		script_assert(pVehicle);
		UpdateCompareFlag(pVehicle->m_fHealth > ScriptParams[1]);
		return 0;
	}
	case COMMAND_ADD_BLIP_FOR_CAR:
	{
		CollectParameters(&m_nIp, 1);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		script_assert(pVehicle);
		// Useless call.
		CRadar::GetActualBlipArrayIndex(CollectNextParameterWithoutIncreasingPC(m_nIp));
		int handle = CRadar::SetEntityBlip(BLIP_CAR, ScriptParams[0], 0, BLIP_DISPLAY_BOTH);
		CRadar::ChangeBlipScale(handle, 3);
		ScriptParams[0] = handle;
		StoreParameters(&m_nIp, 1);
		return 0;
	}
	case COMMAND_ADD_BLIP_FOR_CHAR:
	{
		CollectParameters(&m_nIp, 1);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		// Useless call.
		CRadar::GetActualBlipArrayIndex(CollectNextParameterWithoutIncreasingPC(m_nIp));
		int handle = CRadar::SetEntityBlip(BLIP_CHAR, ScriptParams[0], 1, BLIP_DISPLAY_BOTH);
		CRadar::ChangeBlipScale(handle, 3);
		ScriptParams[0] = handle;
		StoreParameters(&m_nIp, 1);
		return 0;
	}
	case COMMAND_ADD_BLIP_FOR_OBJECT:
	{
		CollectParameters(&m_nIp, 1);
		CObject* pObject = CPools::GetObjectPool()->GetAt(ScriptParams[0]);
		script_assert(pObject);
		// Useless call.
		CRadar::GetActualBlipArrayIndex(CollectNextParameterWithoutIncreasingPC(m_nIp));
		int handle = CRadar::SetEntityBlip(BLIP_OBJECT, ScriptParams[0], 6, BLIP_DISPLAY_BOTH);
		CRadar::ChangeBlipScale(handle, 3);
		ScriptParams[0] = handle;
		StoreParameters(&m_nIp, 1);
		return 0;
	}
	case COMMAND_ADD_BLIP_FOR_CONTACT_POINT:
	{
		CollectParameters(&m_nIp, 3);
		CVector pos = *(CVector*)&ScriptParams[0];
		if (pos.z <= MAP_Z_LOW_LIMIT)
			pos.z = CWorld::FindGroundZForCoord(pos.x, pos.y);
		// Useless call
		CRadar::GetActualBlipArrayIndex(CollectNextParameterWithoutIncreasingPC(m_nIp));
		int handle = CRadar::SetCoordBlip(BLIP_CONTACT_POINT, pos, 2, BLIP_DISPLAY_BOTH);
		CRadar::ChangeBlipScale(handle, 3);
		ScriptParams[0] = handle;
		StoreParameters(&m_nIp, 1);
		return 0;
	}
	case COMMAND_ADD_BLIP_FOR_COORD:
	{
		CollectParameters(&m_nIp, 3);
		CVector pos = *(CVector*)&ScriptParams[0];
		if (pos.z <= MAP_Z_LOW_LIMIT)
			pos.z = CWorld::FindGroundZForCoord(pos.x, pos.y);
		// Useless call
		CRadar::GetActualBlipArrayIndex(CollectNextParameterWithoutIncreasingPC(m_nIp));
		int handle = CRadar::SetCoordBlip(BLIP_COORD, pos, 5, BLIP_DISPLAY_BOTH);
		CRadar::ChangeBlipScale(handle, 3);
		ScriptParams[0] = handle;
		StoreParameters(&m_nIp, 1);
		return 0;
	}
	case COMMAND_CHANGE_BLIP_DISPLAY:
		CollectParameters(&m_nIp, 2);
		CRadar::ChangeBlipDisplay(ScriptParams[0], (eBlipDisplay)ScriptParams[1]);
		return 0;
	case COMMAND_ADD_ONE_OFF_SOUND:
	{
		CollectParameters(&m_nIp, 4);
		switch (ScriptParams[3]) {
		case SCRIPT_SOUND_EVIDENCE_PICKUP:
			DMAudio.PlayFrontEndSound(SOUND_EVIDENCE_PICKUP, 0);
			return 0;
		case SCRIPT_SOUND_UNLOAD_GOLD:
			DMAudio.PlayFrontEndSound(SOUND_UNLOAD_GOLD, 0);
			return 0;
		case SCRIPT_SOUND_PART_MISSION_COMPLETE:
			DMAudio.PlayFrontEndSound(SOUND_PART_MISSION_COMPLETE, 0);
			return 0;
		case SCRIPT_SOUND_RACE_START_3:
			DMAudio.PlayFrontEndSound(SOUND_RACE_START_3, 0);
			return 0;
		case SCRIPT_SOUND_RACE_START_2:
			DMAudio.PlayFrontEndSound(SOUND_RACE_START_2, 0);
			return 0;
		case SCRIPT_SOUND_RACE_START_1:
			DMAudio.PlayFrontEndSound(SOUND_RACE_START_1, 0);
			return 0;
		case SCRIPT_SOUND_RACE_START_GO:
			DMAudio.PlayFrontEndSound(SOUND_RACE_START_GO, 0);
			return 0;
		default:
			break;
		}
#ifdef FIX_BUGS
		/* BUG: if audio is not initialized, this object will not be freed. */
		if (!DMAudio.IsAudioInitialised())
			return 0;
#endif
		cAudioScriptObject* obj = new cAudioScriptObject();
		obj->Posn = *(CVector*)&ScriptParams[0];
		obj->AudioId = ScriptParams[3];
		obj->AudioEntity = AEHANDLE_NONE;
		DMAudio.CreateOneShotScriptObject(obj);
		return 0;
	}
	case COMMAND_ADD_CONTINUOUS_SOUND:
	{
		CollectParameters(&m_nIp, 4);
		cAudioScriptObject* obj = new cAudioScriptObject();
		obj->Posn = *(CVector*)&ScriptParams[0];
		obj->AudioId = ScriptParams[3];
		obj->AudioEntity = DMAudio.CreateLoopingScriptObject(obj);
		ScriptParams[0] = CPools::GetAudioScriptObjectPool()->GetIndex(obj);
		StoreParameters(&m_nIp, 1);
		return 0;
	}
	case COMMAND_REMOVE_SOUND:
	{
		CollectParameters(&m_nIp, 1);
		cAudioScriptObject* obj = CPools::GetAudioScriptObjectPool()->GetAt(ScriptParams[0]);
		if (!obj){
			debug("REMOVE_SOUND - Sound doesn't exist\n");
			return 0;
		}
		DMAudio.DestroyLoopingScriptObject(obj->AudioEntity);
		delete obj;
		return 0;
	}
	case COMMAND_IS_CAR_STUCK_ON_ROOF:
	{
		CollectParameters(&m_nIp, 1);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		script_assert(pVehicle);
		UpdateCompareFlag(CTheScripts::UpsideDownCars.HasCarBeenUpsideDownForAWhile(ScriptParams[0]));
		return 0;
	}
	default:
		script_assert(0);
	}
	return -1;
}

int8 CRunningScript::ProcessCommands400To499(int32 command)
{
	switch (command) {
	case COMMAND_ADD_UPSIDEDOWN_CAR_CHECK:
	{
		CollectParameters(&m_nIp, 1);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		script_assert(pVehicle);
		CTheScripts::UpsideDownCars.AddCarToCheck(ScriptParams[0]);
		return 0;
	}
	case COMMAND_REMOVE_UPSIDEDOWN_CAR_CHECK:
	{
		CollectParameters(&m_nIp, 1);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		script_assert(pVehicle);
		CTheScripts::UpsideDownCars.RemoveCarFromCheck(ScriptParams[0]);
		return 0;
	}
	case COMMAND_SET_CHAR_OBJ_WAIT_ON_FOOT:
	{
		CollectParameters(&m_nIp, 1);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		pPed->bScriptObjectiveCompleted = false;
		pPed->SetObjective(OBJECTIVE_WAIT_ON_FOOT);
		return 0;
	}
	case COMMAND_SET_CHAR_OBJ_FLEE_ON_FOOT_TILL_SAFE:
	{
		CollectParameters(&m_nIp, 1);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		pPed->bScriptObjectiveCompleted = false;
		pPed->SetObjective(OBJECTIVE_FLEE_CHAR_ON_FOOT_TILL_SAFE);
		return 0;
	}
	case COMMAND_SET_CHAR_OBJ_GUARD_SPOT:
	{
		CollectParameters(&m_nIp, 4);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		CVector pos = *(CVector*)&ScriptParams[1];
		if (pos.z <= MAP_Z_LOW_LIMIT)
			pos.z = CWorld::FindGroundZForCoord(pos.x, pos.y);
		pPed->bScriptObjectiveCompleted = false;
		pPed->SetObjective(OBJECTIVE_GUARD_SPOT, pos);
		return 0;
	}
	case COMMAND_SET_CHAR_OBJ_GUARD_AREA:
	{
		CollectParameters(&m_nIp, 5);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		float infX = *(float*)&ScriptParams[1];
		float infY = *(float*)&ScriptParams[2];
		float supX = *(float*)&ScriptParams[3];
		float supY = *(float*)&ScriptParams[4];
		if (infX > supX){
			infX = *(float*)&ScriptParams[3];
			supX = *(float*)&ScriptParams[1];
		}
		if (infY > supY) {
			infY = *(float*)&ScriptParams[4];
			supY = *(float*)&ScriptParams[2];
		}
		CVector pos;
		pos.x = (infX + supX) / 2;
		pos.y = (infY + supY) / 2;
		pos.z = CWorld::FindGroundZForCoord(pos.x, pos.y);
		float radius = Max(pos.x - infX, pos.y - infY);
		pPed->bScriptObjectiveCompleted = false;
		pPed->SetObjective(OBJECTIVE_GUARD_SPOT, pos, radius);
		return 0;
	}
	case COMMAND_SET_CHAR_OBJ_WAIT_IN_CAR:
	{
		CollectParameters(&m_nIp, 1);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		pPed->bScriptObjectiveCompleted = false;
		pPed->SetObjective(OBJECTIVE_WAIT_IN_CAR);
		return 0;
	}
	case COMMAND_IS_PLAYER_IN_AREA_ON_FOOT_2D:
	case COMMAND_IS_PLAYER_IN_AREA_IN_CAR_2D:
	case COMMAND_IS_PLAYER_STOPPED_IN_AREA_2D:
	case COMMAND_IS_PLAYER_STOPPED_IN_AREA_ON_FOOT_2D:
	case COMMAND_IS_PLAYER_STOPPED_IN_AREA_IN_CAR_2D:
	case COMMAND_IS_PLAYER_IN_AREA_ON_FOOT_3D:
	case COMMAND_IS_PLAYER_IN_AREA_IN_CAR_3D:
	case COMMAND_IS_PLAYER_STOPPED_IN_AREA_3D:
	case COMMAND_IS_PLAYER_STOPPED_IN_AREA_ON_FOOT_3D:
	case COMMAND_IS_PLAYER_STOPPED_IN_AREA_IN_CAR_3D:
		PlayerInAreaCheckCommand(command, &m_nIp);
		return 0;
	case COMMAND_IS_CHAR_IN_AREA_ON_FOOT_2D:
	case COMMAND_IS_CHAR_IN_AREA_IN_CAR_2D:
	case COMMAND_IS_CHAR_STOPPED_IN_AREA_2D:
	case COMMAND_IS_CHAR_STOPPED_IN_AREA_ON_FOOT_2D:
	case COMMAND_IS_CHAR_STOPPED_IN_AREA_IN_CAR_2D:
	case COMMAND_IS_CHAR_IN_AREA_ON_FOOT_3D:
	case COMMAND_IS_CHAR_IN_AREA_IN_CAR_3D:
	case COMMAND_IS_CHAR_STOPPED_IN_AREA_3D:
	case COMMAND_IS_CHAR_STOPPED_IN_AREA_ON_FOOT_3D:
	case COMMAND_IS_CHAR_STOPPED_IN_AREA_IN_CAR_3D:
		CharInAreaCheckCommand(command, &m_nIp);
		return 0;
	case COMMAND_IS_CAR_STOPPED_IN_AREA_2D:
	case COMMAND_IS_CAR_STOPPED_IN_AREA_3D:
		CarInAreaCheckCommand(command, &m_nIp);
		return 0;
	case COMMAND_LOCATE_CAR_2D:
	case COMMAND_LOCATE_STOPPED_CAR_2D:
	case COMMAND_LOCATE_CAR_3D:
	case COMMAND_LOCATE_STOPPED_CAR_3D:
		LocateCarCommand(command, &m_nIp);
		return 0;
	case COMMAND_GIVE_WEAPON_TO_PLAYER:
	{
		CollectParameters(&m_nIp, 3);
		CPlayerPed* pPed = CWorld::Players[ScriptParams[0]].m_pPed;
		script_assert(pPed);
		pPed->m_nSelectedWepSlot = pPed->GiveWeapon((eWeaponType)ScriptParams[1], ScriptParams[2]);
		return 0;
	}
	case COMMAND_GIVE_WEAPON_TO_CHAR:
	{
		CollectParameters(&m_nIp, 3);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		pPed->SetCurrentWeapon(pPed->GiveWeapon((eWeaponType)ScriptParams[1], ScriptParams[2]));
		if (pPed->bInVehicle)
			pPed->RemoveWeaponModel(CWeaponInfo::GetWeaponInfo(pPed->m_weapons[pPed->m_currentWeapon].m_eWeaponType)->m_nModelId);
		return 0;
	}
	/* Not implemented */
	//case COMMAND_GIVE_WEAPON_TO_CAR:
	case COMMAND_SET_PLAYER_CONTROL:
	{
		CollectParameters(&m_nIp, 2);
		CPlayerInfo* pPlayer = &CWorld::Players[ScriptParams[0]];
		if (ScriptParams[1]){
			if (CGame::playingIntro || CTheScripts::DelayMakingPlayerUnsafeThisTime){
				CTheScripts::CountdownToMakePlayerUnsafe = 50;
				if (CTheScripts::DelayMakingPlayerUnsafeThisTime)
					CTheScripts::DelayMakingPlayerUnsafeThisTime--;
			}else{
				pPlayer->MakePlayerSafe(false);
			}
		}else{
			pPlayer->MakePlayerSafe(true);
			if (strcmp(m_abScriptName, "camera") == 0){
				pPlayer->m_pPed->SetMoveSpeed(0.0f, 0.0f, 0.0f);
				pPlayer->m_pPed->SetTurnSpeed(0.0f, 0.0f, 0.0f);
				CAnimManager::BlendAnimation((RpClump*)pPlayer->m_pPed->m_rwObject, pPlayer->m_pPed->m_animGroup, ANIM_STD_IDLE, 1000.0f);
			}
		}
		return 0;
	}
	case COMMAND_FORCE_WEATHER:
		CollectParameters(&m_nIp, 1);
		CWeather::ForceWeather(ScriptParams[0]);
		return 0;
	case COMMAND_FORCE_WEATHER_NOW:
		CollectParameters(&m_nIp, 1);
		CWeather::ForceWeatherNow(ScriptParams[0]);
		return 0;
	case COMMAND_RELEASE_WEATHER:
		CWeather::ReleaseWeather();
		return 0;
	case COMMAND_SET_CURRENT_PLAYER_WEAPON:
	{
		CollectParameters(&m_nIp, 2);
		CPlayerPed* pPed = CWorld::Players[ScriptParams[0]].m_pPed;
		for (int i = 0; i < WEAPONTYPE_TOTAL_INVENTORY_WEAPONS; i++){
			if (pPed->m_weapons[i].m_eWeaponType == ScriptParams[1])
				pPed->m_nSelectedWepSlot = i;
		}
		return 0;
	}
	case COMMAND_SET_CURRENT_CHAR_WEAPON:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		for (int i = 0; i < WEAPONTYPE_TOTAL_INVENTORY_WEAPONS; i++) {
			if (pPed->m_weapons[i].m_eWeaponType == ScriptParams[1])
				pPed->SetCurrentWeapon(i);
		}
		return 0;
	}
	/* Not implemented */
	//case COMMAND_SET_CURRENT_CAR_WEAPON:
	case COMMAND_GET_OBJECT_COORDINATES:
	{
		CollectParameters(&m_nIp, 1);
		CObject* pObject = CPools::GetObjectPool()->GetAt(ScriptParams[0]);
		script_assert(pObject);
		*(CVector*)&ScriptParams[0] = pObject->GetPosition();
		StoreParameters(&m_nIp, 3);
		return 0;
	}
	case COMMAND_SET_OBJECT_COORDINATES:
	{
		CollectParameters(&m_nIp, 4);
		CObject* pObject = CPools::GetObjectPool()->GetAt(ScriptParams[0]);
		script_assert(pObject);
		CVector pos = *(CVector*)&ScriptParams[1];
		if (pos.z <= MAP_Z_LOW_LIMIT)
			pos.z = CWorld::FindGroundZForCoord(pos.x, pos.y);
		pObject->Teleport(pos);
		CTheScripts::ClearSpaceForMissionEntity(pos, pObject);
		return 0;
	}
	case COMMAND_GET_GAME_TIMER:
		ScriptParams[0] = CTimer::GetTimeInMilliseconds();
		StoreParameters(&m_nIp, 1);
		return 0;
	case COMMAND_TURN_CHAR_TO_FACE_COORD:
	{
		CollectParameters(&m_nIp, 4);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		CVehicle* pVehicle;
		CVector pos;
		if (pPed->bInVehicle)
			pVehicle = pPed->m_pMyVehicle;
		else
			pVehicle = nil;
		if (pVehicle)
			pos = pVehicle->GetPosition();
		else
			pos = pPed->GetPosition();
		float heading = CGeneral::GetATanOfXY(pos.x - *(float*)&ScriptParams[1], pos.y - *(float*)&ScriptParams[2]);
		heading += HALFPI;
		if (heading > TWOPI)
			heading -= TWOPI;
		if (!pVehicle){
			pPed->m_fRotationCur = heading;
			pPed->m_fRotationDest = heading;
			pPed->SetHeading(heading);
		}
		return 0;
	}
	case COMMAND_TURN_PLAYER_TO_FACE_COORD:
	{
		CollectParameters(&m_nIp, 4);
		CPed* pPed = CWorld::Players[ScriptParams[0]].m_pPed;
		script_assert(pPed);
		CVehicle* pVehicle;
		CVector pos;
		if (pPed->bInVehicle)
			pVehicle = pPed->m_pMyVehicle;
		else
			pVehicle = nil;
		if (pVehicle)
			pos = pVehicle->GetPosition();
		else
			pos = pPed->GetPosition();
		float heading = CGeneral::GetATanOfXY(pos.x - *(float*)&ScriptParams[1], pos.y - *(float*)&ScriptParams[2]);
		heading += HALFPI;
		if (heading > TWOPI)
			heading -= TWOPI;
		if (!pVehicle) {
			pPed->m_fRotationCur = heading;
			pPed->m_fRotationDest = heading;
			pPed->SetHeading(heading);
		}
		return 0;
	}
	case COMMAND_STORE_WANTED_LEVEL:
	{
		CollectParameters(&m_nIp, 1);
		CPlayerPed* pPed = CWorld::Players[ScriptParams[0]].m_pPed;
		script_assert(pPed);
		ScriptParams[0] = pPed->m_pWanted->GetWantedLevel();
		StoreParameters(&m_nIp, 1);
		return 0;
	}
	case COMMAND_IS_CAR_STOPPED:
	{
		CollectParameters(&m_nIp, 1);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		script_assert(pVehicle);
		UpdateCompareFlag(CTheScripts::IsVehicleStopped(pVehicle));
		return 0;
	}
	case COMMAND_MARK_CHAR_AS_NO_LONGER_NEEDED:
	{
		CollectParameters(&m_nIp, 1);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		CTheScripts::CleanUpThisPed(pPed);
		if (m_bIsMissionScript)
			CTheScripts::MissionCleanUp.RemoveEntityFromList(ScriptParams[0], CLEANUP_CHAR);
		return 0;
	}
	case COMMAND_MARK_CAR_AS_NO_LONGER_NEEDED:
	{
		CollectParameters(&m_nIp, 1);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		CTheScripts::CleanUpThisVehicle(pVehicle);
		if (m_bIsMissionScript)
			CTheScripts::MissionCleanUp.RemoveEntityFromList(ScriptParams[0], CLEANUP_CAR);
		return 0;
	}
	case COMMAND_MARK_OBJECT_AS_NO_LONGER_NEEDED:
	{
		CollectParameters(&m_nIp, 1);
		CObject* pObject = CPools::GetObjectPool()->GetAt(ScriptParams[0]);
		CTheScripts::CleanUpThisObject(pObject);
		if (m_bIsMissionScript)
			CTheScripts::MissionCleanUp.RemoveEntityFromList(ScriptParams[0], CLEANUP_OBJECT);
		return 0;
	}
	case COMMAND_DONT_REMOVE_CHAR:
	{
		CollectParameters(&m_nIp, 1);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		CTheScripts::MissionCleanUp.RemoveEntityFromList(ScriptParams[0], CLEANUP_CHAR);
		return 0;
	}
	case COMMAND_DONT_REMOVE_CAR:
	{
		CollectParameters(&m_nIp, 1);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		script_assert(pVehicle);
		CTheScripts::MissionCleanUp.RemoveEntityFromList(ScriptParams[0], CLEANUP_CAR);
		return 0;
	}
	case COMMAND_DONT_REMOVE_OBJECT:
	{
		CollectParameters(&m_nIp, 1);
		CObject* pObject = CPools::GetObjectPool()->GetAt(ScriptParams[0]);
		script_assert(pObject);
		CTheScripts::MissionCleanUp.RemoveEntityFromList(ScriptParams[0], CLEANUP_OBJECT);
		return 0;
	}
	case COMMAND_CREATE_CHAR_AS_PASSENGER:
	{
		CollectParameters(&m_nIp, 4);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		script_assert(pVehicle);
		switch (ScriptParams[2]) {
		case MI_COP:
			if (ScriptParams[1] == PEDTYPE_COP)
				ScriptParams[2] = COP_STREET;
			break;
		case MI_SWAT:
			if (ScriptParams[1] == PEDTYPE_COP)
				ScriptParams[2] = COP_SWAT;
			break;
		case MI_FBI:
			if (ScriptParams[1] == PEDTYPE_COP)
				ScriptParams[2] = COP_FBI;
			break;
		case MI_ARMY:
			if (ScriptParams[1] == PEDTYPE_COP)
				ScriptParams[2] = COP_ARMY;
			break;
		case MI_MEDIC:
			if (ScriptParams[1] == PEDTYPE_EMERGENCY)
				ScriptParams[2] = PEDTYPE_EMERGENCY;
			break;
		case MI_FIREMAN:
			if (ScriptParams[1] == PEDTYPE_FIREMAN)
				ScriptParams[2] = PEDTYPE_FIREMAN;
			break;
		default:
			break;
		}
		CPed* pPed;
		if (ScriptParams[1] == PEDTYPE_COP)
			pPed = new CCopPed((eCopType)ScriptParams[2]);
		else if (ScriptParams[1] == PEDTYPE_EMERGENCY || ScriptParams[1] == PEDTYPE_FIREMAN)
			pPed = new CEmergencyPed(ScriptParams[2]);
		else
			pPed = new CCivilianPed((ePedType)ScriptParams[1], ScriptParams[2]);
		pPed->CharCreatedBy = MISSION_CHAR;
		pPed->bRespondsToThreats = false;
		pPed->bAllowMedicsToReviveMe = false;
		pPed->SetPosition(pVehicle->GetPosition());
		pPed->SetOrientation(0.0f, 0.0f, 0.0f);
		pPed->SetPedState(PED_DRIVING);
		CPopulation::ms_nTotalMissionPeds++;
		if (ScriptParams[3] >= 0)
			pVehicle->AddPassenger(pPed, ScriptParams[3]);
		else
			pVehicle->AddPassenger(pPed);
		pPed->m_pMyVehicle = pVehicle;
		pPed->m_pMyVehicle->RegisterReference((CEntity**)&pPed->m_pMyVehicle);
		pPed->bInVehicle = true;
		pPed->SetPedState(PED_DRIVING);
		pVehicle->SetStatus(STATUS_PHYSICS);
		pPed->bUsesCollision = false;
#ifdef FIX_BUGS
		AnimationId anim = pVehicle->GetDriverAnim();
#else
		AnimationId anim = pVehicle->bLowVehicle ? ANIM_STD_CAR_SIT_LO : ANIM_STD_CAR_SIT;
#endif
		pPed->m_pVehicleAnim = CAnimManager::BlendAnimation(pPed->GetClump(), ASSOCGRP_STD, anim, 100.0f);
		pPed->StopNonPartialAnims();
		pPed->m_nZoneLevel = CTheZones::GetLevelFromPosition(&pPed->GetPosition());
		CWorld::Add(pPed);
		ScriptParams[0] = CPools::GetPedPool()->GetIndex(pPed);
		StoreParameters(&m_nIp, 1);
		if (m_bIsMissionScript)
			CTheScripts::MissionCleanUp.AddEntityToList(ScriptParams[0], CLEANUP_CHAR);
		return 0;
	}
	case COMMAND_SET_CHAR_OBJ_KILL_CHAR_ON_FOOT:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		CPed* pTarget = CPools::GetPedPool()->GetAt(ScriptParams[1]);
		pPed->bScriptObjectiveCompleted = false;
		pPed->SetObjective(OBJECTIVE_KILL_CHAR_ON_FOOT, pTarget);
		return 0;
	}
	case COMMAND_SET_CHAR_OBJ_KILL_PLAYER_ON_FOOT:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		CPed* pTarget = CWorld::Players[ScriptParams[1]].m_pPed;
		pPed->bScriptObjectiveCompleted = false;
		pPed->SetObjective(OBJECTIVE_KILL_CHAR_ON_FOOT, pTarget);
		return 0;
	}
	case COMMAND_SET_CHAR_OBJ_KILL_CHAR_ANY_MEANS:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		CPed* pTarget = CPools::GetPedPool()->GetAt(ScriptParams[1]);
		pPed->bScriptObjectiveCompleted = false;
		pPed->SetObjective(OBJECTIVE_KILL_CHAR_ANY_MEANS, pTarget);
		return 0;
	}
	case COMMAND_SET_CHAR_OBJ_KILL_PLAYER_ANY_MEANS:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		CPed* pTarget = CWorld::Players[ScriptParams[1]].m_pPed;
		pPed->bScriptObjectiveCompleted = false;
		pPed->SetObjective(OBJECTIVE_KILL_CHAR_ANY_MEANS, pTarget);
		return 0;
	}
	case COMMAND_SET_CHAR_OBJ_FLEE_CHAR_ON_FOOT_TILL_SAFE:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		CPed* pTarget = CPools::GetPedPool()->GetAt(ScriptParams[1]);
		pPed->bScriptObjectiveCompleted = false;
		pPed->SetObjective(OBJECTIVE_FLEE_CHAR_ON_FOOT_TILL_SAFE, pTarget);
		return 0;
	}
	case COMMAND_SET_CHAR_OBJ_FLEE_PLAYER_ON_FOOT_TILL_SAFE:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		CPed* pTarget = CWorld::Players[ScriptParams[1]].m_pPed;
		pPed->bScriptObjectiveCompleted = false;
		pPed->SetObjective(OBJECTIVE_FLEE_CHAR_ON_FOOT_TILL_SAFE, pTarget);
		return 0;
	}
	case COMMAND_SET_CHAR_OBJ_FLEE_CHAR_ON_FOOT_ALWAYS:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		CPed* pTarget = CPools::GetPedPool()->GetAt(ScriptParams[1]);
		pPed->bScriptObjectiveCompleted = false;
		pPed->SetObjective(OBJECTIVE_FLEE_CHAR_ON_FOOT_ALWAYS, pTarget);
		return 0;
	}
	case COMMAND_SET_CHAR_OBJ_FLEE_PLAYER_ON_FOOT_ALWAYS:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		CPed* pTarget = CWorld::Players[ScriptParams[1]].m_pPed;
		pPed->bScriptObjectiveCompleted = false;
		pPed->SetObjective(OBJECTIVE_FLEE_CHAR_ON_FOOT_ALWAYS, pTarget);
		return 0;
	}
	case COMMAND_SET_CHAR_OBJ_GOTO_CHAR_ON_FOOT:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		CPed* pTarget = CPools::GetPedPool()->GetAt(ScriptParams[1]);
		pPed->bScriptObjectiveCompleted = false;
		pPed->SetObjective(OBJECTIVE_GOTO_CHAR_ON_FOOT, pTarget);
		return 0;
	}
	case COMMAND_SET_CHAR_OBJ_GOTO_PLAYER_ON_FOOT:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		CPed* pTarget = CWorld::Players[ScriptParams[1]].m_pPed;
		pPed->bScriptObjectiveCompleted = false;
		pPed->SetObjective(OBJECTIVE_GOTO_CHAR_ON_FOOT, pTarget);
		return 0;
	}
	case COMMAND_SET_CHAR_OBJ_LEAVE_CAR:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[1]);
		pPed->bScriptObjectiveCompleted = false;
		pPed->SetObjective(OBJECTIVE_LEAVE_CAR, pVehicle);
		return 0;
	}
	case COMMAND_SET_CHAR_OBJ_ENTER_CAR_AS_PASSENGER:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[1]);
		pPed->bScriptObjectiveCompleted = false;
		pPed->SetObjective(OBJECTIVE_ENTER_CAR_AS_PASSENGER, pVehicle);
		return 0;
	}
	case COMMAND_SET_CHAR_OBJ_ENTER_CAR_AS_DRIVER:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[1]);
		pPed->bScriptObjectiveCompleted = false;
		pPed->SetObjective(OBJECTIVE_ENTER_CAR_AS_DRIVER, pVehicle);
		return 0;
	}
	/* Not implemented.
	case COMMAND_SET_CHAR_OBJ_FOLLOW_CAR_IN_CAR:
	case COMMAND_SET_CHAR_OBJ_FIRE_AT_OBJECT_FROM_VEHICLE:
	case COMMAND_SET_CHAR_OBJ_DESTROY_OBJECT:
	*/
	case COMMAND_SET_CHAR_OBJ_DESTROY_CAR:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[1]);
		pPed->bScriptObjectiveCompleted = false;
		pPed->SetObjective(OBJECTIVE_DESTROY_CAR, pVehicle);
		return 0;
	}
	case COMMAND_SET_CHAR_OBJ_GOTO_AREA_ON_FOOT:
	{
		CollectParameters(&m_nIp, 5);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		float infX = *(float*)&ScriptParams[1];
		float infY = *(float*)&ScriptParams[2];
		float supX = *(float*)&ScriptParams[3];
		float supY = *(float*)&ScriptParams[4];
		if (infX > supX) {
			infX = *(float*)&ScriptParams[3];
			supX = *(float*)&ScriptParams[1];
		}
		if (infY > supY) {
			infY = *(float*)&ScriptParams[4];
			supY = *(float*)&ScriptParams[2];
		}
		CVector pos;
		pos.x = (infX + supX) / 2;
		pos.y = (infY + supY) / 2;
		pos.z = CWorld::FindGroundZForCoord(pos.x, pos.y);
		float radius = Max(pos.x - infX, pos.y - infY);
		pPed->bScriptObjectiveCompleted = false;
		pPed->SetObjective(OBJECTIVE_GOTO_AREA_ON_FOOT, pos, radius);
		return 0;
	}
	/* Not implemented.
	case COMMAND_SET_CHAR_OBJ_GOTO_AREA_IN_CAR:
	case COMMAND_SET_CHAR_OBJ_FOLLOW_CAR_ON_FOOT_WITH_OFFSET:
	case COMMAND_SET_CHAR_OBJ_GUARD_ATTACK:
	*/
	case COMMAND_SET_CHAR_AS_LEADER:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		CPed* pTarget = CPools::GetPedPool()->GetAt(ScriptParams[1]);
		pPed->SetObjective(OBJECTIVE_SET_LEADER, pTarget);
		return 0;
	}
	case COMMAND_SET_PLAYER_AS_LEADER:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		CPed* pTarget = CWorld::Players[ScriptParams[1]].m_pPed;
		pPed->SetObjective(OBJECTIVE_SET_LEADER, pTarget);
		return 0;
	}
	case COMMAND_LEAVE_GROUP:
	{
		CollectParameters(&m_nIp, 1);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		pPed->ClearLeader();
		return 0;
	}
	case COMMAND_SET_CHAR_OBJ_FOLLOW_ROUTE:
	{
		CollectParameters(&m_nIp, 3);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		pPed->bScriptObjectiveCompleted = false;
		pPed->SetObjective(OBJECTIVE_FOLLOW_ROUTE, ScriptParams[1], ScriptParams[2]);
		return 0;
	}
	case COMMAND_ADD_ROUTE_POINT:
	{
		CollectParameters(&m_nIp, 4);
		CRouteNode::AddRoutePoint(ScriptParams[0], *(CVector*)&ScriptParams[1]);
		return 0;
	}
	case COMMAND_PRINT_WITH_NUMBER_BIG:
	{
		wchar* text = CTheScripts::GetTextByKeyFromScript(&m_nIp);
		CollectParameters(&m_nIp, 3);
		CMessages::AddBigMessageWithNumber(text, ScriptParams[1], ScriptParams[2] - 1, ScriptParams[0], -1, -1, -1, -1, -1);
		return 0;
	}
	case COMMAND_PRINT_WITH_NUMBER:
	{
		wchar* text = CTheScripts::GetTextByKeyFromScript(&m_nIp);
		CollectParameters(&m_nIp, 3);
		CMessages::AddMessageWithNumber(text, ScriptParams[1], ScriptParams[2], ScriptParams[0], -1, -1, -1, -1, -1);
		return 0;
	}
	case COMMAND_PRINT_WITH_NUMBER_NOW:
	{
		wchar* text = CTheScripts::GetTextByKeyFromScript(&m_nIp);
		CollectParameters(&m_nIp, 3);
		CMessages::AddMessageJumpQWithNumber(text, ScriptParams[1], ScriptParams[2], ScriptParams[0], -1, -1, -1, -1, -1);
		return 0;
	}
	/* Not implemented.
	case COMMAND_PRINT_WITH_NUMBER_SOON:
	*/
	case COMMAND_SWITCH_ROADS_ON:
	{
		CollectParameters(&m_nIp, 6);
		float infX = *(float*)&ScriptParams[0];
		float infY = *(float*)&ScriptParams[1];
		float infZ = *(float*)&ScriptParams[2];
		float supX = *(float*)&ScriptParams[3];
		float supY = *(float*)&ScriptParams[4];
		float supZ = *(float*)&ScriptParams[5];
		if (infX > supX){
			infX = *(float*)&ScriptParams[3];
			supX = *(float*)&ScriptParams[0];
		}
		if (infY > supY){
			infY = *(float*)&ScriptParams[4];
			supY = *(float*)&ScriptParams[1];
		}
		if (infZ > supZ){
			infZ = *(float*)&ScriptParams[5];
			supZ = *(float*)&ScriptParams[2];
		}
		ThePaths.SwitchRoadsOffInArea(infX, supX, infY, supY, infZ, supZ, false);
		return 0;
	}
	case COMMAND_SWITCH_ROADS_OFF:
	{
		CollectParameters(&m_nIp, 6);
		float infX = *(float*)&ScriptParams[0];
		float infY = *(float*)&ScriptParams[1];
		float infZ = *(float*)&ScriptParams[2];
		float supX = *(float*)&ScriptParams[3];
		float supY = *(float*)&ScriptParams[4];
		float supZ = *(float*)&ScriptParams[5];
		if (infX > supX) {
			infX = *(float*)&ScriptParams[3];
			supX = *(float*)&ScriptParams[0];
		}
		if (infY > supY) {
			infY = *(float*)&ScriptParams[4];
			supY = *(float*)&ScriptParams[1];
		}
		if (infZ > supZ) {
			infZ = *(float*)&ScriptParams[5];
			supZ = *(float*)&ScriptParams[2];
		}
		ThePaths.SwitchRoadsOffInArea(infX, supX, infY, supY, infZ, supZ, true);
		return 0;
	}
	case COMMAND_GET_NUMBER_OF_PASSENGERS:
	{
		CollectParameters(&m_nIp, 1);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		script_assert(pVehicle);
		ScriptParams[0] = pVehicle->m_nNumPassengers;
		StoreParameters(&m_nIp, 1);
		return 0;
	}
	case COMMAND_GET_MAXIMUM_NUMBER_OF_PASSENGERS:
	{
		CollectParameters(&m_nIp, 1);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		script_assert(pVehicle);
		ScriptParams[0] = pVehicle->m_nNumMaxPassengers;
		StoreParameters(&m_nIp, 1);
		return 0;
	}
	case COMMAND_SET_CAR_DENSITY_MULTIPLIER:
	{
		CollectParameters(&m_nIp, 1);
		CCarCtrl::CarDensityMultiplier = *(float*)&ScriptParams[0];
		return 0;
	}
	case COMMAND_SET_CAR_HEAVY:
	{
		CollectParameters(&m_nIp, 2);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		script_assert(pVehicle);
		pVehicle->bIsHeavy = (ScriptParams[1] != 0);
		return 0;
	}
	case COMMAND_CLEAR_CHAR_THREAT_SEARCH:
	{
		CollectParameters(&m_nIp, 1);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		pPed->m_fearFlags = 0;
		return 0;
	}
	case COMMAND_ACTIVATE_CRANE:
	{
		CollectParameters(&m_nIp, 10);
		float infX = *(float*)&ScriptParams[2];
		float infY = *(float*)&ScriptParams[3];
		float supX = *(float*)&ScriptParams[4];
		float supY = *(float*)&ScriptParams[5];
		if (infX > supX) {
			infX = *(float*)&ScriptParams[4];
			supX = *(float*)&ScriptParams[2];
		}
		if (infY > supY) {
			infY = *(float*)&ScriptParams[5];
			supY = *(float*)&ScriptParams[3];
		}
		CCranes::ActivateCrane(infX, supX, infY, supY,
			*(float*)&ScriptParams[6], *(float*)&ScriptParams[7], *(float*)&ScriptParams[8],
			DEGTORAD(*(float*)&ScriptParams[9]), false, false,
			*(float*)&ScriptParams[0], *(float*)&ScriptParams[1]);
		return 0;
	}
	case COMMAND_DEACTIVATE_CRANE:
	{
		CollectParameters(&m_nIp, 2);
		CCranes::DeActivateCrane(*(float*)&ScriptParams[0], *(float*)&ScriptParams[1]);
		return 0;
	}
	case COMMAND_SET_MAX_WANTED_LEVEL:
	{
		CollectParameters(&m_nIp, 1);
		CWanted::SetMaximumWantedLevel(ScriptParams[0]);
		return 0;
	}
	/* Debug commands?
	case COMMAND_SAVE_VAR_INT:
	case COMMAND_SAVE_VAR_FLOAT:
	*/
	case COMMAND_IS_CAR_IN_AIR_PROPER:
	{
		CollectParameters(&m_nIp, 1);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		script_assert(pVehicle);
#ifdef FIX_BUGS
		// don't wanna get stuck in unique stunt jump cam forever
		bool usj_with_dodo = strcmp(m_abScriptName, "usj") == 0 && pVehicle->GetModelIndex() == MI_DODO;
		UpdateCompareFlag(pVehicle->m_nCollisionRecords == 0 && !usj_with_dodo);
#else
		UpdateCompareFlag(pVehicle->m_nCollisionRecords == 0);
#endif
		return 0;
	}
	default:
		script_assert(0);
	}
	return -1;
}
