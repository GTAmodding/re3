#include "common.h"

#include "Script.h"
#include "ScriptCommands.h"

#include "Boat.h"
#include "CarCtrl.h"
#include "Clock.h"
#include "Coronas.h"
#include "Cranes.h"
#include "CutsceneMgr.h"
#include "Darkel.h"
#include "Explosion.h"
#include "Fire.h"
#include "General.h"
#include "Garages.h"
#include "Heli.h"
#include "Messages.h"
#include "Pad.h"
#include "ParticleObject.h"
#include "Phones.h"
#include "Pickups.h"
#include "PointLights.h"
#include "Population.h"
#include "Pools.h"
#include "ProjectileInfo.h"
#include "Radar.h"
#include "Restart.h"
#include "Stats.h"
#include "Streaming.h"
#include "User.h"
#include "WaterLevel.h"
#include "Weather.h"
#include "Zones.h"
#include "Wanted.h"

int8 CRunningScript::ProcessCommands500To599(int32 command)
{
	switch (command) {
	case COMMAND_IS_CAR_UPSIDEDOWN:
	{
		CollectParameters(&m_nIp, 1);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		script_assert(pVehicle);
		UpdateCompareFlag(pVehicle->GetUp().z <= -0.97f);
		return 0;
	}
	case COMMAND_GET_PLAYER_CHAR:
	{
		CollectParameters(&m_nIp, 1);
		CPlayerPed* pPed = CWorld::Players[ScriptParams[0]].m_pPed;
		script_assert(pPed);
		ScriptParams[0] = CPools::GetPedPool()->GetIndex(pPed);
		StoreParameters(&m_nIp, 1);
		return 0;
	}
	case COMMAND_CANCEL_OVERRIDE_RESTART:
		CRestart::CancelOverrideRestart();
		return 0;
	case COMMAND_SET_POLICE_IGNORE_PLAYER:
	{
		CollectParameters(&m_nIp, 2);
		CPlayerPed* pPed = CWorld::Players[ScriptParams[0]].m_pPed;
		script_assert(pPed);
		if (ScriptParams[1]) {
			pPed->m_pWanted->m_bIgnoredByCops = true;
			CWorld::StopAllLawEnforcersInTheirTracks();
		}
		else {
			pPed->m_pWanted->m_bIgnoredByCops = false;
		}
		return 0;
	}
	case COMMAND_ADD_PAGER_MESSAGE_WITH_NUMBER:
	{
		wchar* text = CTheScripts::GetTextByKeyFromScript(&m_nIp);
		CollectParameters(&m_nIp, 4);
		CUserDisplay::Pager.AddMessageWithNumber(text, ScriptParams[0], -1, -1, -1, -1, -1,
			ScriptParams[1], ScriptParams[2], ScriptParams[3]);
		return 0;
	}
	case COMMAND_START_KILL_FRENZY:
	{
		wchar* text = CTheScripts::GetTextByKeyFromScript(&m_nIp);
		CollectParameters(&m_nIp, 8);
		CDarkel::StartFrenzy((eWeaponType)ScriptParams[0], ScriptParams[1], ScriptParams[2],
			ScriptParams[3], text, ScriptParams[4], ScriptParams[5],
			ScriptParams[6], ScriptParams[7] != 0, false);
		return 0;
	}
	case COMMAND_READ_KILL_FRENZY_STATUS:
	{
		ScriptParams[0] = CDarkel::ReadStatus();
		StoreParameters(&m_nIp, 1);
		return 0;
	}
	case COMMAND_SQRT:
	{
		CollectParameters(&m_nIp, 1);
		*(float*)&ScriptParams[0] = Sqrt(*(float*)&ScriptParams[0]);
		StoreParameters(&m_nIp, 1);
		return 0;
	}
	case COMMAND_LOCATE_PLAYER_ANY_MEANS_CAR_2D:
	case COMMAND_LOCATE_PLAYER_ON_FOOT_CAR_2D:
	case COMMAND_LOCATE_PLAYER_IN_CAR_CAR_2D:
	case COMMAND_LOCATE_PLAYER_ANY_MEANS_CAR_3D:
	case COMMAND_LOCATE_PLAYER_ON_FOOT_CAR_3D:
	case COMMAND_LOCATE_PLAYER_IN_CAR_CAR_3D:
		LocatePlayerCarCommand(command, &m_nIp);
		return 0;
	case COMMAND_LOCATE_CHAR_ANY_MEANS_CAR_2D:
	case COMMAND_LOCATE_CHAR_ON_FOOT_CAR_2D:
	case COMMAND_LOCATE_CHAR_IN_CAR_CAR_2D:
	case COMMAND_LOCATE_CHAR_ANY_MEANS_CAR_3D:
	case COMMAND_LOCATE_CHAR_ON_FOOT_CAR_3D:
	case COMMAND_LOCATE_CHAR_IN_CAR_CAR_3D:
		LocateCharCarCommand(command, &m_nIp);
		return 0;
	case COMMAND_GENERATE_RANDOM_FLOAT_IN_RANGE:
		CollectParameters(&m_nIp, 2);
		*(float*)&ScriptParams[0] = CGeneral::GetRandomNumberInRange(*(float*)&ScriptParams[0], *(float*)&ScriptParams[1]);
		StoreParameters(&m_nIp, 1);
		return 0;
	case COMMAND_GENERATE_RANDOM_INT_IN_RANGE:
		CollectParameters(&m_nIp, 2);
		ScriptParams[0] = CGeneral::GetRandomNumberInRange(ScriptParams[0], ScriptParams[1]);
		StoreParameters(&m_nIp, 1);
		return 0;
	case COMMAND_LOCK_CAR_DOORS:
	{
		CollectParameters(&m_nIp, 2);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		script_assert(pVehicle);
		pVehicle->m_nDoorLock = (eCarLock)ScriptParams[1];
		return 0;
	}
	case COMMAND_EXPLODE_CAR:
	{
		CollectParameters(&m_nIp, 1);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		script_assert(pVehicle);
		pVehicle->BlowUpCar(nil);
		return 0;
	}
	case COMMAND_ADD_EXPLOSION:
		CollectParameters(&m_nIp, 4);
		CExplosion::AddExplosion(nil, nil, (eExplosionType)ScriptParams[3], *(CVector*)&ScriptParams[0], 0);
		return 0;

	case COMMAND_IS_CAR_UPRIGHT:
	{
		CollectParameters(&m_nIp, 1);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		script_assert(pVehicle);
		UpdateCompareFlag(pVehicle->GetUp().z >= 0.0f);
		return 0;
	}
	case COMMAND_TURN_CHAR_TO_FACE_CHAR:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pSourcePed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		CPed* pTargetPed = CPools::GetPedPool()->GetAt(ScriptParams[1]);
		CVehicle* pVehicle = pSourcePed->bInVehicle ? pSourcePed->m_pMyVehicle : nil;
		CVector2D sourcePos = pSourcePed->bInVehicle ? pVehicle->GetPosition() : pSourcePed->GetPosition();
		CVector2D targetPos = pTargetPed->bInVehicle ? pTargetPed->m_pMyVehicle->GetPosition() : pTargetPed->GetPosition();
		float angle = CGeneral::GetATanOfXY(sourcePos.x - targetPos.x, sourcePos.y - targetPos.y) + HALFPI;
		if (angle > TWOPI)
			angle -= TWOPI;
		if (!pVehicle) {
			pSourcePed->m_fRotationCur = angle;
			pSourcePed->m_fRotationDest = angle;
			pSourcePed->SetHeading(angle);
		}
		return 0;
	}
	case COMMAND_TURN_CHAR_TO_FACE_PLAYER:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pSourcePed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		CPed* pTargetPed = CWorld::Players[ScriptParams[1]].m_pPed;
		CVehicle* pVehicle = pSourcePed->bInVehicle ? pSourcePed->m_pMyVehicle : nil;
		CVector2D sourcePos = pSourcePed->bInVehicle ? pVehicle->GetPosition() : pSourcePed->GetPosition();
		CVector2D targetPos = pTargetPed->bInVehicle ? pTargetPed->m_pMyVehicle->GetPosition() : pTargetPed->GetPosition();
		float angle = CGeneral::GetATanOfXY(sourcePos.x - targetPos.x, sourcePos.y - targetPos.y) + HALFPI;
		if (angle > TWOPI)
			angle -= TWOPI;
		if (!pVehicle) {
			pSourcePed->m_fRotationCur = angle;
			pSourcePed->m_fRotationDest = angle;
			pSourcePed->SetHeading(angle);
		}
		return 0;
	}
	case COMMAND_TURN_PLAYER_TO_FACE_CHAR:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pSourcePed = CWorld::Players[ScriptParams[0]].m_pPed;
		CPed* pTargetPed = CPools::GetPedPool()->GetAt(ScriptParams[1]);
		CVehicle* pVehicle = pSourcePed->bInVehicle ? pSourcePed->m_pMyVehicle : nil;
		CVector2D sourcePos = pSourcePed->bInVehicle ? pVehicle->GetPosition() : pSourcePed->GetPosition();
		CVector2D targetPos = pTargetPed->bInVehicle ? pTargetPed->m_pMyVehicle->GetPosition() : pTargetPed->GetPosition();
		float angle = CGeneral::GetATanOfXY(sourcePos.x - targetPos.x, sourcePos.y - targetPos.y) + HALFPI;
		if (angle > TWOPI)
			angle -= TWOPI;
		if (!pVehicle) {
			pSourcePed->m_fRotationCur = angle;
			pSourcePed->m_fRotationDest = angle;
			pSourcePed->SetHeading(angle);
		}
		return 0;
	}
	case COMMAND_SET_CHAR_OBJ_GOTO_COORD_ON_FOOT:
	{
		CollectParameters(&m_nIp, 3);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		CVector target;
		target.x = *(float*)&ScriptParams[1];
		target.y = *(float*)&ScriptParams[2];
		target.z = CWorld::FindGroundZForCoord(target.x, target.y);
		pPed->bScriptObjectiveCompleted = false;
		pPed->SetObjective(OBJECTIVE_GOTO_AREA_ON_FOOT, target);
		return 0;
	}
	/* Not implemented*/
	//case COMMAND_SET_CHAR_OBJ_GOTO_COORD_IN_CAR:
	case COMMAND_CREATE_PICKUP:
	{
		CollectParameters(&m_nIp, 5);
		int16 model = ScriptParams[0];
		if (model < 0)
			model = CTheScripts::UsedObjectArray[-model].index;
		CVector pos = *(CVector*)&ScriptParams[2];
		if (pos.z <= MAP_Z_LOW_LIMIT)
			pos.z = CWorld::FindGroundZForCoord(pos.x, pos.y) + PICKUP_PLACEMENT_OFFSET;
		CPickups::GetActualPickupIndex(CollectNextParameterWithoutIncreasingPC(m_nIp));
		ScriptParams[0] = CPickups::GenerateNewOne(pos, model, ScriptParams[1], 0);
		StoreParameters(&m_nIp, 1);
		return 0;
	}
	case COMMAND_HAS_PICKUP_BEEN_COLLECTED:
		CollectParameters(&m_nIp, 1);
		UpdateCompareFlag(CPickups::IsPickUpPickedUp(ScriptParams[0]) != 0);
		return 0;
	case COMMAND_REMOVE_PICKUP:
		CollectParameters(&m_nIp, 1);
		CPickups::RemovePickUp(ScriptParams[0]);
		return 0;
	case COMMAND_SET_TAXI_LIGHTS:
	{
		CollectParameters(&m_nIp, 2);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		script_assert(pVehicle);
		script_assert(pVehicle->m_vehType == VEHICLE_TYPE_CAR);
		((CAutomobile*)pVehicle)->SetTaxiLight(ScriptParams[1] != 0);
		return 0;
	}
	case COMMAND_PRINT_BIG_Q:
	{
		wchar* text = CTheScripts::GetTextByKeyFromScript(&m_nIp);
		CollectParameters(&m_nIp, 2);
		CMessages::AddBigMessageQ(text, ScriptParams[0], ScriptParams[1] - 1);
		return 0;
	}
	case COMMAND_PRINT_WITH_NUMBER_BIG_Q:
	{
		wchar* text = CTheScripts::GetTextByKeyFromScript(&m_nIp);
		CollectParameters(&m_nIp, 3);
		CMessages::AddBigMessageWithNumberQ(text, ScriptParams[1], ScriptParams[2] - 1,
			ScriptParams[0], -1, -1, -1, -1, -1);
		return 0;
	}
	case COMMAND_SET_GARAGE:
	{
		CollectParameters(&m_nIp, 7);
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
		ScriptParams[0] = CGarages::AddOne(CVector(infX, infY, infZ), CVector(supX, supY, supZ), ScriptParams[6], 0);
		StoreParameters(&m_nIp, 1);
		return 0;
	}
	case COMMAND_SET_GARAGE_WITH_CAR_MODEL:
	{
		CollectParameters(&m_nIp, 8);
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
		ScriptParams[0] = CGarages::AddOne(CVector(infX, infY, infZ), CVector(supX, supY, supZ), ScriptParams[6], ScriptParams[7]);
		StoreParameters(&m_nIp, 1);
		return 0;
	}
	case COMMAND_SET_TARGET_CAR_FOR_MISSION_GARAGE:
	{
		CollectParameters(&m_nIp, 2);
		CVehicle* pTarget;
		if (ScriptParams[1] >= 0) {
			pTarget = CPools::GetVehiclePool()->GetAt(ScriptParams[1]);
			script_assert(pTarget);
		}
		else {
			pTarget = nil;
		}
		CGarages::SetTargetCarForMissonGarage(ScriptParams[0], pTarget);
		return 0;
	}
	case COMMAND_IS_CAR_IN_MISSION_GARAGE:
		CollectParameters(&m_nIp, 1);
		UpdateCompareFlag(CGarages::HasCarBeenDroppedOffYet(ScriptParams[0]));
		return 0;
	case COMMAND_SET_FREE_BOMBS:
		CollectParameters(&m_nIp, 1);
		CGarages::SetFreeBombs(ScriptParams[0] != 0);
		return 0;
#if GTA_VERSION <= GTA3_PS2_160
	case COMMAND_SET_POWERPOINT:
	{
		CollectParameters(&m_nIp, 7);
		float f1 = *(float*)&ScriptParams[0];
		float f2 = *(float*)&ScriptParams[1];
		float f3 = *(float*)&ScriptParams[2];
		float f4 = *(float*)&ScriptParams[3];
		float f5 = *(float*)&ScriptParams[4];
		float f6 = *(float*)&ScriptParams[5];
		float temp;

		if (f1 > f4) {
			temp = f1;
			f1 = f4;
			f4 = temp;
		}

		if (f2 > f5) {
			temp = f2;
			f2 = f5;
			f5 = temp;
		}

		if (f3 > f6) {
			temp = f3;
			f3 = f6;
			f6 = temp;
		}

		CPowerPoints::GenerateNewOne(f1, f2, f3, f4, f5, f6, *(uint8*)&ScriptParams[6]);

		return 0;
	}
#endif // GTA_VERSION <= GTA3_PS2_160
	case COMMAND_SET_ALL_TAXI_LIGHTS:
		CollectParameters(&m_nIp, 1);
		CAutomobile::SetAllTaxiLights(ScriptParams[0] != 0);
		return 0;
	case COMMAND_IS_CAR_ARMED_WITH_ANY_BOMB:
	{
		CollectParameters(&m_nIp, 1);
		CAutomobile* pCar = (CAutomobile*)CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		script_assert(pCar);
		script_assert(pCar->m_vehType == VEHICLE_TYPE_CAR);
		UpdateCompareFlag(pCar->m_bombType != 0); //TODO: enum
		return 0;
	}
	case COMMAND_APPLY_BRAKES_TO_PLAYERS_CAR:
		CollectParameters(&m_nIp, 2);
		CPad::GetPad(ScriptParams[0])->bApplyBrakes = (ScriptParams[1] != 0);
		return 0;
	case COMMAND_SET_PLAYER_HEALTH:
	{
		CollectParameters(&m_nIp, 2);
		CPlayerPed* pPed = CWorld::Players[ScriptParams[0]].m_pPed;
		script_assert(pPed);
		pPed->m_fHealth = ScriptParams[1];
		return 0;
	}
	case COMMAND_SET_CHAR_HEALTH:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		if (ScriptParams[1]) {
			pPed->m_fHealth = ScriptParams[1];
		}
		else if (pPed->bInVehicle) {
			pPed->SetDead();
			if (!pPed->IsPlayer())
				pPed->FlagToDestroyWhenNextProcessed();
		}
		else {
			pPed->SetDie(ANIM_STD_KO_FRONT, 4.0f, 0.0f);
		}
		return 0;
	}
	case COMMAND_SET_CAR_HEALTH:
	{
		CollectParameters(&m_nIp, 2);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		script_assert(pVehicle);
		pVehicle->m_fHealth = ScriptParams[1];
		return 0;
	}
	case COMMAND_GET_PLAYER_HEALTH:
	{
		CollectParameters(&m_nIp, 1);
		CPed* pPed = CWorld::Players[ScriptParams[0]].m_pPed;
		script_assert(pPed);
		ScriptParams[0] = pPed->m_fHealth; // correct cast float to int
		StoreParameters(&m_nIp, 1);
		return 0;
	}
	case COMMAND_GET_CHAR_HEALTH:
	{
		CollectParameters(&m_nIp, 1);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		ScriptParams[0] = pPed->m_fHealth; // correct cast float to int
		StoreParameters(&m_nIp, 1);
		return 0;
	}
	case COMMAND_GET_CAR_HEALTH:
	{
		CollectParameters(&m_nIp, 1);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		script_assert(pVehicle);
		ScriptParams[0] = pVehicle->m_fHealth; // correct cast float to int
		StoreParameters(&m_nIp, 1);
		return 0;
	}
	case COMMAND_IS_CAR_ARMED_WITH_BOMB:
	{
		CollectParameters(&m_nIp, 2);
		CAutomobile* pCar = (CAutomobile*)CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		script_assert(pCar);
		script_assert(pCar->m_vehType == VEHICLE_TYPE_CAR);
		UpdateCompareFlag(pCar->m_bombType == ScriptParams[1]); //TODO: enum
		return 0;
	}
	case COMMAND_CHANGE_CAR_COLOUR:
	{
		CollectParameters(&m_nIp, 3);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		if (ScriptParams[1] >= 256 || ScriptParams[2] >= 256)
			debug("CHANGE_CAR_COLOUR - Colours must be less than %d", 256);
		pVehicle->m_currentColour1 = ScriptParams[1];
		pVehicle->m_currentColour2 = ScriptParams[2];
		return 0;
	}
	case COMMAND_SWITCH_PED_ROADS_ON:
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
		ThePaths.SwitchPedRoadsOffInArea(infX, supX, infY, supY, infZ, supZ, false);
		return 0;
	}
	case COMMAND_SWITCH_PED_ROADS_OFF:
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
		ThePaths.SwitchPedRoadsOffInArea(infX, supX, infY, supY, infZ, supZ, true);
		return 0;
	}
	case COMMAND_CHAR_LOOK_AT_CHAR_ALWAYS:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pSourcePed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pSourcePed);
		CPed* pTargetPed = CPools::GetPedPool()->GetAt(ScriptParams[1]);
		script_assert(pTargetPed);
		pSourcePed->SetLookFlag(pTargetPed, true);
		pSourcePed->SetLookTimer(60000);
		return 0;
	}
	case COMMAND_CHAR_LOOK_AT_PLAYER_ALWAYS:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pSourcePed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pSourcePed);
		CPed* pTargetPed = CWorld::Players[ScriptParams[1]].m_pPed;
		script_assert(pTargetPed);
		pSourcePed->SetLookFlag(pTargetPed, true);
		pSourcePed->SetLookTimer(60000);
		return 0;
	}
	case COMMAND_PLAYER_LOOK_AT_CHAR_ALWAYS:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pSourcePed = CWorld::Players[ScriptParams[0]].m_pPed;
		script_assert(pSourcePed);
		CPed* pTargetPed = CPools::GetPedPool()->GetAt(ScriptParams[1]);
		script_assert(pTargetPed);
		pSourcePed->SetLookFlag(pTargetPed, true);
		pSourcePed->SetLookTimer(60000);
		return 0;
	}
	case COMMAND_STOP_CHAR_LOOKING:
	{
		CollectParameters(&m_nIp, 1);
		CPed* pSourcePed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pSourcePed);
		pSourcePed->ClearLookFlag();
		pSourcePed->bKeepTryingToLook = false;
		if (pSourcePed->GetPedState() == PED_LOOK_HEADING || pSourcePed->GetPedState() == PED_LOOK_ENTITY)
			pSourcePed->RestorePreviousState();
		return 0;
	}
	case COMMAND_STOP_PLAYER_LOOKING:
	{
		CollectParameters(&m_nIp, 1);
		CPed* pSourcePed = CWorld::Players[ScriptParams[0]].m_pPed;
		script_assert(pSourcePed);
		pSourcePed->ClearLookFlag();
		pSourcePed->bKeepTryingToLook = false;
		if (pSourcePed->GetPedState() == PED_LOOK_HEADING || pSourcePed->GetPedState() == PED_LOOK_ENTITY)
			pSourcePed->RestorePreviousState();
		return 0;
	}
	case COMMAND_SWITCH_HELICOPTER:
		CollectParameters(&m_nIp, 1);
		CHeli::ActivateHeli(ScriptParams[0] != 0);
		return 0;

		//case COMMAND_SET_GANG_ATTITUDE:
		//case COMMAND_SET_GANG_GANG_ATTITUDE:
		//case COMMAND_SET_GANG_PLAYER_ATTITUDE:
		//case COMMAND_SET_GANG_PED_MODELS:
	case COMMAND_SET_GANG_CAR_MODEL:
		CollectParameters(&m_nIp, 2);
		CGangs::SetGangVehicleModel(ScriptParams[0], ScriptParams[1]);
		return 0;
	case COMMAND_SET_GANG_WEAPONS:
		CollectParameters(&m_nIp, 3);
		CGangs::SetGangWeapons(ScriptParams[0], (eWeaponType)ScriptParams[1], (eWeaponType)ScriptParams[2]);
		return 0;
	case COMMAND_SET_CHAR_OBJ_RUN_TO_AREA:
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
		pPed->SetObjective(OBJECTIVE_RUN_TO_AREA, pos, radius);
		return 0;
	}
	case COMMAND_SET_CHAR_OBJ_RUN_TO_COORD:
	{
		CollectParameters(&m_nIp, 3);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		CVector pos;
		pos.x = *(float*)&ScriptParams[1];
		pos.y = *(float*)&ScriptParams[2];
		pos.z = CWorld::FindGroundZForCoord(pos.x, pos.y);
		pPed->bScriptObjectiveCompleted = false;
		pPed->SetObjective(OBJECTIVE_RUN_TO_AREA, pos);
		return 0;
	}
	case COMMAND_IS_PLAYER_TOUCHING_OBJECT_ON_FOOT:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CWorld::Players[ScriptParams[0]].m_pPed;
		script_assert(pPed);
		CObject* pObject = CPools::GetObjectPool()->GetAt(ScriptParams[1]);
		bool isTouching = false;
		if (pPed->bInVehicle)
			isTouching = false;
		else if (pPed->GetHasCollidedWith(pObject))
			isTouching = true;
		UpdateCompareFlag(isTouching);
		return 0;
	}
	case COMMAND_IS_CHAR_TOUCHING_OBJECT_ON_FOOT:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		CObject* pObject = CPools::GetObjectPool()->GetAt(ScriptParams[1]);
		bool isTouching = false;
		if (pPed->InVehicle())
			isTouching = false;
		else if (pPed->GetHasCollidedWith(pObject))
			isTouching = true;
		UpdateCompareFlag(isTouching);
		return 0;
	}
	case COMMAND_LOAD_SPECIAL_CHARACTER:
	{
		CollectParameters(&m_nIp, 1);
		char name[16];
		strncpy(name, (char*)&CTheScripts::ScriptSpace[m_nIp], KEY_LENGTH_IN_SCRIPT);
		for (int i = 0; i < KEY_LENGTH_IN_SCRIPT; i++)
			name[i] = tolower(name[i]);
		CStreaming::RequestSpecialChar(ScriptParams[0] - 1, name, STREAMFLAGS_DEPENDENCY | STREAMFLAGS_SCRIPTOWNED);
		m_nIp += KEY_LENGTH_IN_SCRIPT;
		return 0;
	}
	case COMMAND_HAS_SPECIAL_CHARACTER_LOADED:
	{
		CollectParameters(&m_nIp, 1);
		UpdateCompareFlag(CStreaming::HasSpecialCharLoaded(ScriptParams[0] - 1));
		return 0;
	}
	case COMMAND_FLASH_CAR:
	{
		CollectParameters(&m_nIp, 2);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		script_assert(pVehicle);
		pVehicle->bHasBlip = (ScriptParams[1] != 0);
		return 0;
	}
	case COMMAND_FLASH_CHAR:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		pPed->bHasBlip = (ScriptParams[1] != 0);
		return 0;
	}
	case COMMAND_FLASH_OBJECT:
	{
		CollectParameters(&m_nIp, 2);
		CObject* pObject = CPools::GetObjectPool()->GetAt(ScriptParams[0]);
		script_assert(pObject);
		pObject->bHasBlip = (ScriptParams[1] != 0);
		return 0;
	}
	case COMMAND_IS_PLAYER_IN_REMOTE_MODE:
		CollectParameters(&m_nIp, 1);
		UpdateCompareFlag(CWorld::Players[ScriptParams[0]].IsPlayerInRemoteMode());
		return 0;
	case COMMAND_ARM_CAR_WITH_BOMB:
	{
		CollectParameters(&m_nIp, 2);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		script_assert(pVehicle);
		script_assert(pVehicle->m_vehType == VEHICLE_TYPE_CAR);
		((CAutomobile*)pVehicle)->m_bombType = ScriptParams[1];
		((CAutomobile*)pVehicle)->m_pBombRigger = FindPlayerPed();
		return 0;
	}
	case COMMAND_SET_CHAR_PERSONALITY:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		pPed->SetPedStats((ePedStats)ScriptParams[1]);
		return 0;
	}
	case COMMAND_SET_CUTSCENE_OFFSET:
		CollectParameters(&m_nIp, 3);
		CCutsceneMgr::SetCutsceneOffset(*(CVector*)&ScriptParams[0]);
		return 0;
	case COMMAND_SET_ANIM_GROUP_FOR_CHAR:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		pPed->m_animGroup = (AssocGroupId)ScriptParams[1];
		return 0;
	}
	case COMMAND_SET_ANIM_GROUP_FOR_PLAYER:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CWorld::Players[ScriptParams[0]].m_pPed;
		script_assert(pPed);
		pPed->m_animGroup = (AssocGroupId)ScriptParams[1];
		return 0;
	}
	case COMMAND_REQUEST_MODEL:
	{
		CollectParameters(&m_nIp, 1);
		int model = ScriptParams[0];
		if (model < 0)
			model = CTheScripts::UsedObjectArray[-model].index;
		CStreaming::RequestModel(model, STREAMFLAGS_DEPENDENCY | STREAMFLAGS_NOFADE | STREAMFLAGS_SCRIPTOWNED);
		return 0;
	}
	case COMMAND_HAS_MODEL_LOADED:
	{
		CollectParameters(&m_nIp, 1);
		int model = ScriptParams[0];
		if (model < 0)
			model = CTheScripts::UsedObjectArray[-model].index;
		UpdateCompareFlag(CStreaming::HasModelLoaded(model));
		return 0;
	}
	case COMMAND_MARK_MODEL_AS_NO_LONGER_NEEDED:
	{
		CollectParameters(&m_nIp, 1);
		int model = ScriptParams[0];
		if (model < 0)
			model = CTheScripts::UsedObjectArray[-model].index;
		CStreaming::SetMissionDoesntRequireModel(model);
		return 0;
	}
	case COMMAND_GRAB_PHONE:
	{
		CollectParameters(&m_nIp, 2);
		ScriptParams[0] = gPhoneInfo.GrabPhone(*(float*)&ScriptParams[0], *(float*)&ScriptParams[1]);
		StoreParameters(&m_nIp, 1);
		return 0;
	}
	case COMMAND_SET_REPEATED_PHONE_MESSAGE:
	{
		CollectParameters(&m_nIp, 1);
		wchar* text = CTheScripts::GetTextByKeyFromScript(&m_nIp);
		gPhoneInfo.SetPhoneMessage_Repeatedly(ScriptParams[0], text, nil, nil, nil, nil, nil);
		return 0;
	}
	case COMMAND_SET_PHONE_MESSAGE:
	{
		CollectParameters(&m_nIp, 1);
		wchar* text = CTheScripts::GetTextByKeyFromScript(&m_nIp);
		gPhoneInfo.SetPhoneMessage_JustOnce(ScriptParams[0], text, nil, nil, nil, nil, nil);
		return 0;
	}
	case COMMAND_HAS_PHONE_DISPLAYED_MESSAGE:
	{
		CollectParameters(&m_nIp, 1);
		UpdateCompareFlag(gPhoneInfo.HasMessageBeenDisplayed(ScriptParams[0]));
		return 0;
	}
	case COMMAND_TURN_PHONE_OFF:
	{
		CollectParameters(&m_nIp, 1);
		gPhoneInfo.SetPhoneMessage_JustOnce(ScriptParams[0], nil, nil, nil, nil, nil, nil);
		return 0;
	}
	case COMMAND_DRAW_CORONA:
	{
		CollectParameters(&m_nIp, 9);
		CVector pos = *(CVector*)&ScriptParams[0];
		if (pos.z <= MAP_Z_LOW_LIMIT)
			pos.z = CWorld::FindGroundZForCoord(pos.x, pos.y);
		CCoronas::RegisterCorona((uintptr)this + m_nIp, ScriptParams[6], ScriptParams[7], ScriptParams[8],
			255, pos, *(float*)&ScriptParams[3], 150.0f, ScriptParams[4], ScriptParams[5], 1, 0, 0, 0.0f);
		return 0;
	}
	case COMMAND_DRAW_LIGHT:
	{
		CollectParameters(&m_nIp, 6);
		CVector pos = *(CVector*)&ScriptParams[0];
		CVector unused(0.0f, 0.0f, 0.0f);
		CPointLights::AddLight(0, *(CVector*)&ScriptParams[0], CVector(0.0f, 0.0f, 0.0f), 12.0f,
			ScriptParams[3] / 255.0f, ScriptParams[4] / 255.0f, ScriptParams[5] / 255.0f, 0, true);
		return 0;
	}
	case COMMAND_STORE_WEATHER:
		CWeather::StoreWeatherState();
		return 0;
	case COMMAND_RESTORE_WEATHER:
		CWeather::RestoreWeatherState();
		return 0;
	case COMMAND_STORE_CLOCK:
		CClock::StoreClock();
		return 0;
	case COMMAND_RESTORE_CLOCK:
		CClock::RestoreClock();
		return 0;
	case COMMAND_RESTART_CRITICAL_MISSION:
	{
		CollectParameters(&m_nIp, 4);
		CVector pos = *(CVector*)&ScriptParams[0];
		if (pos.z <= MAP_Z_LOW_LIMIT)
			pos.z = CWorld::FindGroundZForCoord(pos.x, pos.y);
		CRestart::OverrideNextRestart(pos, *(float*)&ScriptParams[3]);
		if (CWorld::Players[CWorld::PlayerInFocus].m_WBState != WBSTATE_PLAYING)
			printf("RESTART_CRITICAL_MISSION - Player state is not PLAYING\n");
		CWorld::Players[CWorld::PlayerInFocus].PlayerFailedCriticalMission();
		return 0;
	}
	case COMMAND_IS_PLAYER_PLAYING:
	{
		CollectParameters(&m_nIp, 1);
		UpdateCompareFlag(CWorld::Players[ScriptParams[0]].m_WBState == WBSTATE_PLAYING);
		return 0;
	}
#ifdef GTA_SCRIPT_COLLECTIVE
	case COMMAND_SET_COLL_OBJ_NO_OBJ:
		CollectParameters(&m_nIp, 1);
		CTheScripts::SetObjectiveForAllPedsInCollective(ScriptParams[0], OBJECTIVE_NONE);
		return 0;
#endif
	default:
		script_assert(0);
	}
	return -1;
}

int8 CRunningScript::ProcessCommands600To699(int32 command)
{
	switch (command){
#ifdef GTA_SCRIPT_COLLECTIVE
	case COMMAND_SET_COLL_OBJ_WAIT_ON_FOOT:
		CollectParameters(&m_nIp, 1);
		CTheScripts::SetObjectiveForAllPedsInCollective(ScriptParams[0], OBJECTIVE_WAIT_ON_FOOT);
		return 0;
	case COMMAND_SET_COLL_OBJ_FLEE_ON_FOOT_TILL_SAFE:
		CollectParameters(&m_nIp, 1);
		CTheScripts::SetObjectiveForAllPedsInCollective(ScriptParams[0], OBJECTIVE_FLEE_ON_FOOT_TILL_SAFE);
		return 0;
	case COMMAND_SET_COLL_OBJ_GUARD_SPOT:
	{
		CollectParameters(&m_nIp, 4);
		CVector pos = *(CVector*)&ScriptParams[1];
		if (pos.z <= MAP_Z_LOW_LIMIT)
			pos.z = CWorld::FindGroundZForCoord(pos.x, pos.y);
		CTheScripts::SetObjectiveForAllPedsInCollective(ScriptParams[0], OBJECTIVE_GUARD_AREA, pos);
		return 0;
	}
	case COMMAND_SET_COLL_OBJ_GUARD_AREA:
	{
		CollectParameters(&m_nIp, 5);
		float infX = *(float*)&ScriptParams[1];
		float supX = *(float*)&ScriptParams[3];
		if (infX > supX) {
			infX = *(float*)&ScriptParams[3];
			supX = *(float*)&ScriptParams[1];
		}
		float infY = *(float*)&ScriptParams[2];
		float supY = *(float*)&ScriptParams[4];
		if (infY > supY) {
			infY = *(float*)&ScriptParams[4];
			supY = *(float*)&ScriptParams[2];
		}
		CVector pos;
		pos.x = (infX + supX) / 2;
		pos.y = (infY + supY) / 2;
		pos.z = CWorld::FindGroundZForCoord(pos.x, pos.y);
		float radius = Max(pos.x - infX, pos.y - infY);
		CTheScripts::SetObjectiveForAllPedsInCollective(ScriptParams[0], OBJECTIVE_GUARD_AREA, pos, radius);
		return 0;
	}
	case COMMAND_SET_COLL_OBJ_WAIT_IN_CAR:
		CollectParameters(&m_nIp, 1);
		CTheScripts::SetObjectiveForAllPedsInCollective(ScriptParams[0], OBJECTIVE_WAIT_IN_CAR);
		return 0;
	case COMMAND_SET_COLL_OBJ_KILL_CHAR_ON_FOOT:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[1]);
		CTheScripts::SetObjectiveForAllPedsInCollective(ScriptParams[0], OBJECTIVE_KILL_CHAR_ON_FOOT, pPed);
		return 0;
	}
	case COMMAND_SET_COLL_OBJ_KILL_PLAYER_ON_FOOT:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CWorld::Players[ScriptParams[1]].m_pPed;
		CTheScripts::SetObjectiveForAllPedsInCollective(ScriptParams[0], OBJECTIVE_KILL_CHAR_ON_FOOT, pPed);
		return 0;
	}
	case COMMAND_SET_COLL_OBJ_KILL_CHAR_ANY_MEANS:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[1]);
		CTheScripts::SetObjectiveForAllPedsInCollective(ScriptParams[0], OBJECTIVE_KILL_CHAR_ANY_MEANS, pPed);
		return 0;
	}
	case COMMAND_SET_COLL_OBJ_KILL_PLAYER_ANY_MEANS:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CWorld::Players[ScriptParams[1]].m_pPed;
		CTheScripts::SetObjectiveForAllPedsInCollective(ScriptParams[0], OBJECTIVE_KILL_CHAR_ANY_MEANS, pPed);
		return 0;
	}
	case COMMAND_SET_COLL_OBJ_FLEE_CHAR_ON_FOOT_TILL_SAFE:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[1]);
		CTheScripts::SetObjectiveForAllPedsInCollective(ScriptParams[0], OBJECTIVE_FLEE_CHAR_ON_FOOT_TILL_SAFE, pPed);
		return 0;
	}
	case COMMAND_SET_COLL_OBJ_FLEE_PLAYER_ON_FOOT_TILL_SAFE:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CWorld::Players[ScriptParams[1]].m_pPed;
		CTheScripts::SetObjectiveForAllPedsInCollective(ScriptParams[0], OBJECTIVE_FLEE_CHAR_ON_FOOT_TILL_SAFE, pPed);
		return 0;
	}
	case COMMAND_SET_COLL_OBJ_FLEE_CHAR_ON_FOOT_ALWAYS:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[1]);
		CTheScripts::SetObjectiveForAllPedsInCollective(ScriptParams[0], OBJECTIVE_FLEE_CHAR_ON_FOOT_ALWAYS, pPed);
		return 0;
	}
	case COMMAND_SET_COLL_OBJ_FLEE_PLAYER_ON_FOOT_ALWAYS:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CWorld::Players[ScriptParams[1]].m_pPed;
		CTheScripts::SetObjectiveForAllPedsInCollective(ScriptParams[0], OBJECTIVE_FLEE_CHAR_ON_FOOT_ALWAYS, pPed);
		return 0;
	}
	case COMMAND_SET_COLL_OBJ_GOTO_CHAR_ON_FOOT:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[1]);
		CTheScripts::SetObjectiveForAllPedsInCollective(ScriptParams[0], OBJECTIVE_GOTO_CHAR_ON_FOOT, pPed);
		return 0;
	}
	case COMMAND_SET_COLL_OBJ_GOTO_PLAYER_ON_FOOT:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CWorld::Players[ScriptParams[1]].m_pPed;
		CTheScripts::SetObjectiveForAllPedsInCollective(ScriptParams[0], OBJECTIVE_GOTO_CHAR_ON_FOOT, pPed);
		return 0;
	}
	case COMMAND_SET_COLL_OBJ_LEAVE_CAR:
		CollectParameters(&m_nIp, 1);
		CTheScripts::SetObjectiveForAllPedsInCollective(ScriptParams[0], OBJECTIVE_LEAVE_CAR);
		return 0;
	case COMMAND_SET_COLL_OBJ_ENTER_CAR_AS_PASSENGER:
	{
		CollectParameters(&m_nIp, 2);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[1]);
		CTheScripts::SetObjectiveForAllPedsInCollective(ScriptParams[0], OBJECTIVE_ENTER_CAR_AS_PASSENGER, pVehicle);
		return 0;
	}
	case COMMAND_SET_COLL_OBJ_ENTER_CAR_AS_DRIVER:
	{
		CollectParameters(&m_nIp, 2);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[1]);
		CTheScripts::SetObjectiveForAllPedsInCollective(ScriptParams[0], OBJECTIVE_ENTER_CAR_AS_DRIVER, pVehicle);
		return 0;
	}
	/*
	case COMMAND_SET_COLL_OBJ_FOLLOW_CAR_IN_CAR:
	case COMMAND_SET_COLL_OBJ_FIRE_AT_OBJECT_FROM_VEHICLE:
	case COMMAND_SET_COLL_OBJ_DESTROY_OBJECT:
	*/
	case COMMAND_SET_COLL_OBJ_DESTROY_CAR:
	{
		CollectParameters(&m_nIp, 2);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[1]);
		CTheScripts::SetObjectiveForAllPedsInCollective(ScriptParams[0], OBJECTIVE_DESTROY_CAR, pVehicle);
		return 0;
	}
	case COMMAND_SET_COLL_OBJ_GOTO_AREA_ON_FOOT:
	{
		CollectParameters(&m_nIp, 5);
		float infX = *(float*)&ScriptParams[1];
		float supX = *(float*)&ScriptParams[3];
		if (infX > supX) {
			infX = *(float*)&ScriptParams[3];
			supX = *(float*)&ScriptParams[1];
		}
		float infY = *(float*)&ScriptParams[2];
		float supY = *(float*)&ScriptParams[4];
		if (infY > supY) {
			infY = *(float*)&ScriptParams[4];
			supY = *(float*)&ScriptParams[2];
		}
		CVector pos;
		pos.x = (infX + supX) / 2;
		pos.y = (infY + supY) / 2;
		pos.z = CWorld::FindGroundZForCoord(pos.x, pos.y);
		float radius = Max(pos.x - infX, pos.y - infY);
		CTheScripts::SetObjectiveForAllPedsInCollective(ScriptParams[0], OBJECTIVE_GOTO_AREA_ON_FOOT, pos, radius);
		return 0;
	}
	/*
	case COMMAND_SET_COLL_OBJ_GOTO_AREA_IN_CAR:
	case COMMAND_SET_COLL_OBJ_FOLLOW_CAR_ON_FOOT_WITH_OFFSET:
	case COMMAND_SET_COLL_OBJ_GUARD_ATTACK:
	*/
	case COMMAND_SET_COLL_OBJ_FOLLOW_ROUTE:
		CollectParameters(&m_nIp, 3);
		CTheScripts::SetObjectiveForAllPedsInCollective(ScriptParams[0], OBJECTIVE_FOLLOW_ROUTE, ScriptParams[1], ScriptParams[2]);
		return 0;
	case COMMAND_SET_COLL_OBJ_GOTO_COORD_ON_FOOT:
	{
		CollectParameters(&m_nIp, 3);
		CVector pos(*(float*)&ScriptParams[1], *(float*)&ScriptParams[2], CWorld::FindGroundZForCoord(*(float*)&ScriptParams[1], *(float*)&ScriptParams[2]));
		CTheScripts::SetObjectiveForAllPedsInCollective(ScriptParams[0], OBJECTIVE_GOTO_AREA_ON_FOOT, pos);
		return 0;
	}
	//case COMMAND_SET_COLL_OBJ_GOTO_COORD_IN_CAR:
	case COMMAND_SET_COLL_OBJ_RUN_TO_AREA:
	{
		CollectParameters(&m_nIp, 5);
		float infX = *(float*)&ScriptParams[1];
		float supX = *(float*)&ScriptParams[3];
		if (infX > supX) {
			infX = *(float*)&ScriptParams[3];
			supX = *(float*)&ScriptParams[1];
		}
		float infY = *(float*)&ScriptParams[2];
		float supY = *(float*)&ScriptParams[4];
		if (infY > supY) {
			infY = *(float*)&ScriptParams[4];
			supY = *(float*)&ScriptParams[2];
		}
		CVector pos;
		pos.x = (infX + supX) / 2;
		pos.y = (infY + supY) / 2;
		pos.z = CWorld::FindGroundZForCoord(pos.x, pos.y);
		float radius = Max(pos.x - infX, pos.y - infY);
		CTheScripts::SetObjectiveForAllPedsInCollective(ScriptParams[0], OBJECTIVE_RUN_TO_AREA, pos, radius);
		return 0;
	}
	case COMMAND_SET_COLL_OBJ_RUN_TO_COORD:
	{
		CollectParameters(&m_nIp, 3);
		CVector pos(*(float*)&ScriptParams[1], *(float*)&ScriptParams[2], CWorld::FindGroundZForCoord(*(float*)&ScriptParams[1], *(float*)&ScriptParams[2]));
		CTheScripts::SetObjectiveForAllPedsInCollective(ScriptParams[0], OBJECTIVE_RUN_TO_AREA, pos);
		return 0;
	}
	case COMMAND_ADD_PEDS_IN_AREA_TO_COLL:
	{
		CollectParameters(&m_nIp, 3);
		float X = *(float*)&ScriptParams[0];
		float Y = *(float*)&ScriptParams[1];
		float Z = CWorld::FindGroundZForCoord(X, Y);
		float radius = *(float*)&ScriptParams[2];
		ScriptParams[0] = CTheScripts::AddPedsInAreaToCollective(X, Y, Z, radius);
		StoreParameters(&m_nIp, 1);
		return 0;
	}
	case COMMAND_ADD_PEDS_IN_VEHICLE_TO_COLL:
		CollectParameters(&m_nIp, 1);
		ScriptParams[0] = CTheScripts::AddPedsInVehicleToCollective(ScriptParams[0]);
		StoreParameters(&m_nIp, 1);
		return 0;
	case COMMAND_CLEAR_COLL:
		CollectParameters(&m_nIp, 1);
		for (int i = 0; i < MAX_NUM_COLLECTIVES; i++) {
			if (CTheScripts::CollectiveArray[i].colIndex == ScriptParams[0]) {
				CTheScripts::CollectiveArray[i].colIndex = -1;
				CTheScripts::CollectiveArray[i].pedIndex = 0;
			}
		}
		return 0;
	case COMMAND_IS_COLL_IN_CARS:
	{
		CollectParameters(&m_nIp, 1);
		bool result = true;
		for (int i = 0; i < MAX_NUM_COLLECTIVES; i++) {
			CPed* pPed = CPools::GetPedPool()->GetAt(CTheScripts::CollectiveArray[i].pedIndex);
			if (!pPed) {
				CTheScripts::CollectiveArray[i].colIndex = -1;
				CTheScripts::CollectiveArray[i].pedIndex = 0;
			}
			else {
				result = false;
				break;
			}
		}
		UpdateCompareFlag(result);
		return 0;
	}
	case COMMAND_LOCATE_COLL_ANY_MEANS_2D:
	case COMMAND_LOCATE_COLL_ON_FOOT_2D:
	case COMMAND_LOCATE_COLL_IN_CAR_2D:
	case COMMAND_LOCATE_STOPPED_COLL_ANY_MEANS_2D:
	case COMMAND_LOCATE_STOPPED_COLL_ON_FOOT_2D:
	case COMMAND_LOCATE_STOPPED_COLL_IN_CAR_2D:
		LocateCollectiveCommand(command, &m_nIp);
		return 0;
	case COMMAND_LOCATE_COLL_ANY_MEANS_CHAR_2D:
	case COMMAND_LOCATE_COLL_ON_FOOT_CHAR_2D:
	case COMMAND_LOCATE_COLL_IN_CAR_CHAR_2D:
		LocateCollectiveCharCommand(command, &m_nIp);
		return 0;
	case COMMAND_LOCATE_COLL_ANY_MEANS_CAR_2D:
	case COMMAND_LOCATE_COLL_ON_FOOT_CAR_2D:
	case COMMAND_LOCATE_COLL_IN_CAR_CAR_2D:
		LocateCollectiveCarCommand(command, &m_nIp);
		return 0;
	case COMMAND_LOCATE_COLL_ANY_MEANS_PLAYER_2D:
	case COMMAND_LOCATE_COLL_ON_FOOT_PLAYER_2D:
	case COMMAND_LOCATE_COLL_IN_CAR_PLAYER_2D:
		LocateCollectivePlayerCommand(command, &m_nIp);
		return 0;
	case COMMAND_IS_COLL_IN_AREA_2D:
	case COMMAND_IS_COLL_IN_AREA_ON_FOOT_2D:
	case COMMAND_IS_COLL_IN_AREA_IN_CAR_2D:
	case COMMAND_IS_COLL_STOPPED_IN_AREA_2D:
	case COMMAND_IS_COLL_STOPPED_IN_AREA_ON_FOOT_2D:
	case COMMAND_IS_COLL_STOPPED_IN_AREA_IN_CAR_2D:
		CollectiveInAreaCheckCommand(command, &m_nIp);
		return 0;
	case COMMAND_GET_NUMBER_OF_PEDS_IN_COLL:
	{
		CollectParameters(&m_nIp, 1);
		int total = 0;
		for (int i = 0; i < MAX_NUM_COLLECTIVES; i++) {
			CPed* pPed = CPools::GetPedPool()->GetAt(CTheScripts::CollectiveArray[i].pedIndex);
			if (!pPed) {
				CTheScripts::CollectiveArray[i].colIndex = -1;
				CTheScripts::CollectiveArray[i].pedIndex = 0;
			}
			else {
				total++;
			}
		}
		ScriptParams[0] = total;
		StoreParameters(&m_nIp, 1);
		return 0;
	}
#endif
	case COMMAND_SET_CHAR_HEED_THREATS:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		pPed->bRespondsToThreats = (ScriptParams[1] != 0);
		return 0;
	}
	case COMMAND_SET_PLAYER_HEED_THREATS:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CWorld::Players[ScriptParams[0]].m_pPed;
		script_assert(pPed);
		pPed->bRespondsToThreats = (ScriptParams[1] != 0);
		return 0;
	}
	case COMMAND_GET_CONTROLLER_MODE:
#if defined(GTA_PC) && !defined(DETECT_PAD_INPUT_SWITCH)
		ScriptParams[0] = 0;
#else
		ScriptParams[0] = CPad::IsAffectedByController ? CPad::GetPad(0)->Mode : 0;
#endif
		StoreParameters(&m_nIp, 1);
		return 0;
	case COMMAND_SET_CAN_RESPRAY_CAR:
	{
		CollectParameters(&m_nIp, 2);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		script_assert(pVehicle);
		script_assert(pVehicle->m_vehType == VEHICLE_TYPE_CAR);
		((CAutomobile*)pVehicle)->bFixedColour = (ScriptParams[1] == 0);
		return 0;
	}
	case COMMAND_IS_TAXI:
	{
		CollectParameters(&m_nIp, 1);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		script_assert(pVehicle);
		int mi = pVehicle->GetModelIndex();
		UpdateCompareFlag(mi == MI_TAXI || mi == MI_CABBIE || mi == MI_BORGNINE);
		return 0;
	}
	case COMMAND_UNLOAD_SPECIAL_CHARACTER:
		CollectParameters(&m_nIp, 1);
		CStreaming::SetMissionDoesntRequireSpecialChar(ScriptParams[0] - 1);
		return 0;
	case COMMAND_RESET_NUM_OF_MODELS_KILLED_BY_PLAYER:
		CDarkel::ResetModelsKilledByPlayer();
		return 0;
	case COMMAND_GET_NUM_OF_MODELS_KILLED_BY_PLAYER:
		CollectParameters(&m_nIp, 1);
		ScriptParams[0] = CDarkel::QueryModelsKilledByPlayer(ScriptParams[0]);
		StoreParameters(&m_nIp, 1);
		return 0;
	case COMMAND_ACTIVATE_GARAGE:
		CollectParameters(&m_nIp, 1);
		CGarages::ActivateGarage(ScriptParams[0]);
		return 0;
	case COMMAND_SWITCH_TAXI_TIMER:
	{
		CollectParameters(&m_nIp, 1);
		if (ScriptParams[0] != 0){
			CWorld::Players[CWorld::PlayerInFocus].m_nUnusedTaxiTimer = CTimer::GetTimeInMilliseconds();
			CWorld::Players[CWorld::PlayerInFocus].m_bUnusedTaxiThing = true;
		}else{
			CWorld::Players[CWorld::PlayerInFocus].m_bUnusedTaxiThing = false;
		}
		return 0;
	}
	case COMMAND_CREATE_OBJECT_NO_OFFSET:
	{
		CollectParameters(&m_nIp, 4);
		int mi = ScriptParams[0] >= 0 ? ScriptParams[0] : CTheScripts::UsedObjectArray[-ScriptParams[0]].index;
		CObject* pObj = new CObject(mi, false);
		pObj->ObjectCreatedBy = MISSION_OBJECT;
		CVector pos = *(CVector*)&ScriptParams[1];
		if (pos.z <= MAP_Z_LOW_LIMIT)
			pos.z = CWorld::FindGroundZForCoord(pos.x, pos.y);
		pObj->SetPosition(pos);
		pObj->SetOrientation(0.0f, 0.0f, 0.0f);
		pObj->GetMatrix().UpdateRW();
		pObj->UpdateRwFrame();
		CTheScripts::ClearSpaceForMissionEntity(pos, pObj);
		CWorld::Add(pObj);
		ScriptParams[0] = CPools::GetObjectPool()->GetIndex(pObj);
		StoreParameters(&m_nIp, 1);
		if (m_bIsMissionScript)
			CTheScripts::MissionCleanUp.AddEntityToList(ScriptParams[0], CLEANUP_OBJECT);
		return 0;
	}
	case COMMAND_IS_BOAT:
	{
		CollectParameters(&m_nIp, 1);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		script_assert(pVehicle);
		UpdateCompareFlag(pVehicle->m_vehType == VEHICLE_TYPE_BOAT);
		return 0;
	}
	case COMMAND_SET_CHAR_OBJ_GOTO_AREA_ANY_MEANS:
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
		pPed->SetObjective(OBJECTIVE_GOTO_AREA_ANY_MEANS, pos, radius);
		return 0;
	}
#ifdef GTA_SCRIPT_COLLECTIVE
	case COMMAND_SET_COLL_OBJ_GOTO_AREA_ANY_MEANS:
	{
		CollectParameters(&m_nIp, 5);
		float infX = *(float*)&ScriptParams[1];
		float supX = *(float*)&ScriptParams[3];
		if (infX > supX) {
			infX = *(float*)&ScriptParams[3];
			supX = *(float*)&ScriptParams[1];
		}
		float infY = *(float*)&ScriptParams[2];
		float supY = *(float*)&ScriptParams[4];
		if (infY > supY) {
			infY = *(float*)&ScriptParams[4];
			supY = *(float*)&ScriptParams[2];
		}
		CVector pos;
		pos.x = (infX + supX) / 2;
		pos.y = (infY + supY) / 2;
		pos.z = CWorld::FindGroundZForCoord(pos.x, pos.y);
		float radius = Max(pos.x - infX, pos.y - infY);
		CTheScripts::SetObjectiveForAllPedsInCollective(ScriptParams[0], OBJECTIVE_GOTO_AREA_ANY_MEANS, pos, radius);
		return 0;
	}
#endif
	case COMMAND_IS_PLAYER_STOPPED:
	{
		CollectParameters(&m_nIp, 1);
		CPlayerInfo* pPlayer = &CWorld::Players[ScriptParams[0]];
		UpdateCompareFlag(CTheScripts::IsPlayerStopped(pPlayer));
		return 0;
	}
	case COMMAND_IS_CHAR_STOPPED:
	{
		CollectParameters(&m_nIp, 1);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		UpdateCompareFlag(CTheScripts::IsPedStopped(pPed));
		return 0;
	}
	case COMMAND_MESSAGE_WAIT:
		CollectParameters(&m_nIp, 2);
		m_nWakeTime = CTimer::GetTimeInMilliseconds() + ScriptParams[0];
		if (ScriptParams[1] != 0)
			m_bSkipWakeTime = true;
		return 1;
	case COMMAND_ADD_PARTICLE_EFFECT:
	{
		CollectParameters(&m_nIp, 5);
		CVector pos = *(CVector*)&ScriptParams[1];
		if (pos.z <= MAP_Z_LOW_LIMIT)
			pos.z = CWorld::FindGroundZForCoord(pos.x, pos.y);
		CParticleObject::AddObject(ScriptParams[0], pos, ScriptParams[4] != 0);
		return 0;
	}
	case COMMAND_SWITCH_WIDESCREEN:
		CollectParameters(&m_nIp, 1);
		if (ScriptParams[0] != 0)
			TheCamera.SetWideScreenOn();
		else
			TheCamera.SetWideScreenOff();
		return 0;
	case COMMAND_ADD_SPRITE_BLIP_FOR_CAR:
	{
		CollectParameters(&m_nIp, 2);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		script_assert(pVehicle);
		CRadar::GetActualBlipArrayIndex(CollectNextParameterWithoutIncreasingPC(m_nIp));
		int id = CRadar::SetEntityBlip(BLIP_CAR, ScriptParams[0], 0, BLIP_DISPLAY_BOTH);
		CRadar::SetBlipSprite(id, ScriptParams[1]);
		ScriptParams[0] = id;
		StoreParameters(&m_nIp, 1);
		return 0;
	}
	case COMMAND_ADD_SPRITE_BLIP_FOR_CHAR:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		CRadar::GetActualBlipArrayIndex(CollectNextParameterWithoutIncreasingPC(m_nIp));
		int id = CRadar::SetEntityBlip(BLIP_CHAR, ScriptParams[0], 1, BLIP_DISPLAY_BOTH);
		CRadar::SetBlipSprite(id, ScriptParams[1]);
		ScriptParams[0] = id;
		StoreParameters(&m_nIp, 1);
		return 0;
	}
	case COMMAND_ADD_SPRITE_BLIP_FOR_OBJECT:
	{
		CollectParameters(&m_nIp, 2);
		CObject* pObject = CPools::GetObjectPool()->GetAt(ScriptParams[0]);
		script_assert(pObject);
		CRadar::GetActualBlipArrayIndex(CollectNextParameterWithoutIncreasingPC(m_nIp));
		int id = CRadar::SetEntityBlip(BLIP_OBJECT, ScriptParams[0], 6, BLIP_DISPLAY_BOTH);
		CRadar::SetBlipSprite(id, ScriptParams[1]);
		ScriptParams[0] = id;
		StoreParameters(&m_nIp, 1);
		return 0;
	}
	case COMMAND_ADD_SPRITE_BLIP_FOR_CONTACT_POINT:
	{
		CollectParameters(&m_nIp, 4);
		CVector pos = *(CVector*)&ScriptParams[0];
		if (pos.z <= MAP_Z_LOW_LIMIT)
			pos.z = CWorld::FindGroundZForCoord(pos.x, pos.y);
		CRadar::GetActualBlipArrayIndex(CollectNextParameterWithoutIncreasingPC(m_nIp));
		int id = CRadar::SetCoordBlip(BLIP_CONTACT_POINT, pos, 2, BLIP_DISPLAY_BOTH);
		CRadar::SetBlipSprite(id, ScriptParams[3]);
		ScriptParams[0] = id;
		StoreParameters(&m_nIp, 1);
		return 0;
	}
	case COMMAND_ADD_SPRITE_BLIP_FOR_COORD:
	{
		CollectParameters(&m_nIp, 4);
		CVector pos = *(CVector*)&ScriptParams[0];
		if (pos.z <= MAP_Z_LOW_LIMIT)
			pos.z = CWorld::FindGroundZForCoord(pos.x, pos.y);
		CRadar::GetActualBlipArrayIndex(CollectNextParameterWithoutIncreasingPC(m_nIp));
		int id = CRadar::SetCoordBlip(BLIP_COORD, pos, 5, BLIP_DISPLAY_BOTH);
		CRadar::SetBlipSprite(id, ScriptParams[3]);
		ScriptParams[0] = id;
		StoreParameters(&m_nIp, 1);
		return 0;
	}
	case COMMAND_SET_CHAR_ONLY_DAMAGED_BY_PLAYER:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		pPed->bOnlyDamagedByPlayer = (ScriptParams[1] != 0);
		return 0;
	}
	case COMMAND_SET_CAR_ONLY_DAMAGED_BY_PLAYER:
	{
		CollectParameters(&m_nIp, 2);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		script_assert(pVehicle);
		pVehicle->bOnlyDamagedByPlayer = (ScriptParams[1] != 0);
		return 0;
	}
	case COMMAND_SET_CHAR_PROOFS:
	{
		CollectParameters(&m_nIp, 6);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		pPed->bBulletProof = (ScriptParams[1] != 0);
		pPed->bFireProof = (ScriptParams[2] != 0);
		pPed->bExplosionProof = (ScriptParams[3] != 0);
		pPed->bCollisionProof = (ScriptParams[4] != 0);
		pPed->bMeleeProof = (ScriptParams[5] != 0);
		return 0;
	}
	case COMMAND_SET_CAR_PROOFS:
	{
		CollectParameters(&m_nIp, 6);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		script_assert(pVehicle);
		pVehicle->bBulletProof = (ScriptParams[1] != 0);
		pVehicle->bFireProof = (ScriptParams[2] != 0);
		pVehicle->bExplosionProof = (ScriptParams[3] != 0);
		pVehicle->bCollisionProof = (ScriptParams[4] != 0);
		pVehicle->bMeleeProof = (ScriptParams[5] != 0);
		return 0;
	}
	case COMMAND_IS_PLAYER_IN_ANGLED_AREA_2D:
	case COMMAND_IS_PLAYER_IN_ANGLED_AREA_ON_FOOT_2D:
	case COMMAND_IS_PLAYER_IN_ANGLED_AREA_IN_CAR_2D:
	case COMMAND_IS_PLAYER_STOPPED_IN_ANGLED_AREA_2D:
	case COMMAND_IS_PLAYER_STOPPED_IN_ANGLED_AREA_ON_FOOT_2D:
	case COMMAND_IS_PLAYER_STOPPED_IN_ANGLED_AREA_IN_CAR_2D:
	case COMMAND_IS_PLAYER_IN_ANGLED_AREA_3D:
	case COMMAND_IS_PLAYER_IN_ANGLED_AREA_ON_FOOT_3D:
	case COMMAND_IS_PLAYER_IN_ANGLED_AREA_IN_CAR_3D:
	case COMMAND_IS_PLAYER_STOPPED_IN_ANGLED_AREA_3D:
	case COMMAND_IS_PLAYER_STOPPED_IN_ANGLED_AREA_ON_FOOT_3D:
	case COMMAND_IS_PLAYER_STOPPED_IN_ANGLED_AREA_IN_CAR_3D:
		PlayerInAngledAreaCheckCommand(command, &m_nIp);
		return 0;
	case COMMAND_DEACTIVATE_GARAGE:
		CollectParameters(&m_nIp, 1);
		CGarages::DeActivateGarage(ScriptParams[0]);
		return 0;
	case COMMAND_GET_NUMBER_OF_CARS_COLLECTED_BY_GARAGE:
		CollectParameters(&m_nIp, 1);
		ScriptParams[0] = CGarages::QueryCarsCollected(ScriptParams[0]);
		StoreParameters(&m_nIp, 1);
		return 0;
	case COMMAND_HAS_CAR_BEEN_TAKEN_TO_GARAGE:
		CollectParameters(&m_nIp, 2);
		UpdateCompareFlag(CGarages::HasThisCarBeenCollected(ScriptParams[0], ScriptParams[1] - 1));
		return 0;
	default:
		script_assert(0);
	}
	return -1;
}

int8 CRunningScript::ProcessCommands700To799(int32 command)
{
	switch (command){
	case COMMAND_SET_SWAT_REQUIRED:
		CollectParameters(&m_nIp, 1);
		FindPlayerPed()->m_pWanted->m_bSwatRequired = (ScriptParams[0] != 0);
		return 0;
	case COMMAND_SET_FBI_REQUIRED:
		CollectParameters(&m_nIp, 1);
		FindPlayerPed()->m_pWanted->m_bFbiRequired = (ScriptParams[0] != 0);
		return 0;
	case COMMAND_SET_ARMY_REQUIRED:
		CollectParameters(&m_nIp, 1);
		FindPlayerPed()->m_pWanted->m_bArmyRequired = (ScriptParams[0] != 0);
		return 0;
	case COMMAND_IS_CAR_IN_WATER:
	{
		CollectParameters(&m_nIp, 1);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		UpdateCompareFlag(pVehicle && pVehicle->bIsInWater);
		return 0;
	}
	case COMMAND_GET_CLOSEST_CHAR_NODE:
	{
		CollectParameters(&m_nIp, 3);
		CVector pos = *(CVector*)&ScriptParams[0];
		if (pos.z <= MAP_Z_LOW_LIMIT)
			pos.z = CWorld::FindGroundZForCoord(pos.x, pos.y);
		CPathNode* pNode = &ThePaths.m_pathNodes[ThePaths.FindNodeClosestToCoors(pos, 1, 999999.9f)];
		*(CVector*)&ScriptParams[0] = pNode->GetPosition();
		StoreParameters(&m_nIp, 3);
		return 0;
	}
	case COMMAND_GET_CLOSEST_CAR_NODE:
	{
		CollectParameters(&m_nIp, 3);
		CVector pos = *(CVector*)&ScriptParams[0];
		if (pos.z <= MAP_Z_LOW_LIMIT)
			pos.z = CWorld::FindGroundZForCoord(pos.x, pos.y);
		CPathNode* pNode = &ThePaths.m_pathNodes[ThePaths.FindNodeClosestToCoors(pos, 0, 999999.9f)];
		*(CVector*)&ScriptParams[0] = pNode->GetPosition();
		StoreParameters(&m_nIp, 3);
		return 0;
	}
	case COMMAND_CAR_GOTO_COORDINATES_ACCURATE:
	{
		CollectParameters(&m_nIp, 4);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		script_assert(pVehicle);
		CVector pos = *(CVector*)&ScriptParams[1];
		if (pos.z <= MAP_Z_LOW_LIMIT)
			pos.z = CWorld::FindGroundZForCoord(pos.x, pos.y);
		pos.z += pVehicle->GetDistanceFromCentreOfMassToBaseOfModel();
		if (CCarCtrl::JoinCarWithRoadSystemGotoCoors(pVehicle, pos, false))
			pVehicle->AutoPilot.m_nCarMission = MISSION_GOTO_COORDS_STRAIGHT_ACCURATE;
		else
			pVehicle->AutoPilot.m_nCarMission = MISSION_GOTOCOORDS_ACCURATE;
		pVehicle->SetStatus(STATUS_PHYSICS);
		pVehicle->bEngineOn = true;
		pVehicle->AutoPilot.m_nCruiseSpeed = Max(6, pVehicle->AutoPilot.m_nCruiseSpeed);
		pVehicle->AutoPilot.m_nAntiReverseTimer = CTimer::GetTimeInMilliseconds();
		return 0;
	}
	case COMMAND_START_PACMAN_RACE:
		CollectParameters(&m_nIp, 1);
		CPacManPickups::StartPacManRace(ScriptParams[0]);
		return 0;
	case COMMAND_START_PACMAN_RECORD:
		CPacManPickups::StartPacManRecord();
		return 0;
	case COMMAND_GET_NUMBER_OF_POWER_PILLS_EATEN:
		ScriptParams[0] = CPacManPickups::QueryPowerPillsEatenInRace();
		StoreParameters(&m_nIp, 1);
		return 0;
	case COMMAND_CLEAR_PACMAN:
		CPacManPickups::CleanUpPacManStuff();
		return 0;
	case COMMAND_START_PACMAN_SCRAMBLE:
	{
		CollectParameters(&m_nIp, 5);
		CVector pos = *(CVector*)&ScriptParams[0];
		if (pos.z <= MAP_Z_LOW_LIMIT)
			pos.z = CWorld::FindGroundZForCoord(pos.x, pos.y);
		CPacManPickups::StartPacManScramble(pos, *(float*)&ScriptParams[3], ScriptParams[4]);
		return 0;
	}
	case COMMAND_GET_NUMBER_OF_POWER_PILLS_CARRIED:
		ScriptParams[0] = CPacManPickups::QueryPowerPillsCarriedByPlayer();
		StoreParameters(&m_nIp, 1);
		return 0;
	case COMMAND_CLEAR_NUMBER_OF_POWER_PILLS_CARRIED:
		CPacManPickups::ResetPowerPillsCarriedByPlayer();
		return 0;
	case COMMAND_IS_CAR_ON_SCREEN:
	{
		CollectParameters(&m_nIp, 1);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		script_assert(pVehicle);
		UpdateCompareFlag(TheCamera.IsSphereVisible(pVehicle->GetBoundCentre(), pVehicle->GetBoundRadius()));
		return 0;
	}
	case COMMAND_IS_CHAR_ON_SCREEN:
	{
		CollectParameters(&m_nIp, 1);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		UpdateCompareFlag(TheCamera.IsSphereVisible(pPed->GetBoundCentre(), pPed->GetBoundRadius()));
		return 0;
	}
	case COMMAND_IS_OBJECT_ON_SCREEN:
	{
		CollectParameters(&m_nIp, 1);
		CObject* pObject = CPools::GetObjectPool()->GetAt(ScriptParams[0]);
		script_assert(pObject);
		UpdateCompareFlag(TheCamera.IsSphereVisible(pObject->GetBoundCentre(), pObject->GetBoundRadius()));
		return 0;
	}
	case COMMAND_GOSUB_FILE:
	{
		CollectParameters(&m_nIp, 2);
		script_assert(m_nStackPointer < MAX_STACK_DEPTH);
		m_anStack[m_nStackPointer++] = m_nIp;
		SetIP(ScriptParams[0]);
		// ScriptParams[1] == filename
		return 0;
	}
	case COMMAND_GET_GROUND_Z_FOR_3D_COORD:
	{
		CollectParameters(&m_nIp, 3);
		CVector pos = *(CVector*)&ScriptParams[0];
		bool success;
		*(float*)&ScriptParams[0] = CWorld::FindGroundZFor3DCoord(pos.x, pos.y, pos.z, &success);
		StoreParameters(&m_nIp, 1);
		return 0;
	}
	case COMMAND_START_SCRIPT_FIRE:
	{
		CollectParameters(&m_nIp, 3);
		CVector pos = *(CVector*)&ScriptParams[0];
		if (pos.z <= MAP_Z_LOW_LIMIT)
			pos.z = CWorld::FindGroundZForCoord(pos.x, pos.y);
		ScriptParams[0] = gFireManager.StartScriptFire(pos, nil, 0.8f, 1);
		StoreParameters(&m_nIp, 1);
		return 0;
	}
	case COMMAND_IS_SCRIPT_FIRE_EXTINGUISHED:
		CollectParameters(&m_nIp, 1);
		UpdateCompareFlag(gFireManager.IsScriptFireExtinguish(ScriptParams[0]));
		return 0;
	case COMMAND_REMOVE_SCRIPT_FIRE:
		CollectParameters(&m_nIp, 1);
		gFireManager.RemoveScriptFire(ScriptParams[0]);
		return 0;
	case COMMAND_SET_COMEDY_CONTROLS:
	{
		CollectParameters(&m_nIp, 2);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		script_assert(pVehicle);
		pVehicle->bComedyControls = (ScriptParams[1] != 0);
		return 0;
	}
	case COMMAND_BOAT_GOTO_COORDS:
	{
		CollectParameters(&m_nIp, 4);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		script_assert(pVehicle);
		script_assert(pVehicle->m_vehType == VEHICLE_TYPE_BOAT);
		CBoat* pBoat = (CBoat*)pVehicle;
		CVector pos = *(CVector*)&ScriptParams[1];
		if (pos.z <= MAP_Z_LOW_LIMIT)
			CWaterLevel::GetWaterLevel(pos.x, pos.y, pos.z, &pos.z, false);
		pBoat->AutoPilot.m_nCarMission = MISSION_GOTOCOORDS_ASTHECROWSWIMS;
		pBoat->AutoPilot.m_vecDestinationCoors = pos;
		pBoat->SetStatus(STATUS_PHYSICS);
		pBoat->bEngineOn = true;
		pBoat->AutoPilot.m_nCruiseSpeed = Max(6, pBoat->AutoPilot.m_nCruiseSpeed);
		pBoat->AutoPilot.m_nAntiReverseTimer = CTimer::GetTimeInMilliseconds();
		return 0;
	}
	case COMMAND_BOAT_STOP:
	{
		CollectParameters(&m_nIp, 1);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		script_assert(pVehicle);
		script_assert(pVehicle->m_vehType == VEHICLE_TYPE_BOAT);
		CBoat* pBoat = (CBoat*)pVehicle;
		pBoat->AutoPilot.m_nCarMission = MISSION_NONE;
		pBoat->SetStatus(STATUS_PHYSICS);
		pBoat->bEngineOn = false;
		pBoat->AutoPilot.m_nCruiseSpeed = 0;
		return 0;
	}
	case COMMAND_IS_PLAYER_SHOOTING_IN_AREA:
	{
		CollectParameters(&m_nIp, 6);
		CPed* pPed = CWorld::Players[ScriptParams[0]].m_pPed;
		script_assert(pPed);
		float x1 = *(float*)&ScriptParams[1];
		float y1 = *(float*)&ScriptParams[2];
		float x2 = *(float*)&ScriptParams[3];
		float y2 = *(float*)&ScriptParams[4];
		UpdateCompareFlag(pPed->bIsShooting && pPed->IsWithinArea(x1, y1, x2, y2));
		if (ScriptParams[5])
			CTheScripts::HighlightImportantArea((uintptr)this + m_nIp, x1, y1, x2, y2, MAP_Z_LOW_LIMIT);
		if (CTheScripts::DbgFlag)
			CTheScripts::DrawDebugSquare(x1, y1, x2, y2);
		return 0;
	}
	case COMMAND_IS_CHAR_SHOOTING_IN_AREA:
	{
		CollectParameters(&m_nIp, 6);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		float x1 = *(float*)&ScriptParams[1];
		float y1 = *(float*)&ScriptParams[2];
		float x2 = *(float*)&ScriptParams[3];
		float y2 = *(float*)&ScriptParams[4];
		UpdateCompareFlag(pPed->bIsShooting && pPed->IsWithinArea(x1, y1, x2, y2));
		if (ScriptParams[5])
			CTheScripts::HighlightImportantArea((uintptr)this + m_nIp, x1, y1, x2, y2, MAP_Z_LOW_LIMIT);
		if (CTheScripts::DbgFlag)
			CTheScripts::DrawDebugSquare(x1, y1, x2, y2);
		return 0;
	}
	case COMMAND_IS_CURRENT_PLAYER_WEAPON:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CWorld::Players[ScriptParams[0]].m_pPed;
		script_assert(pPed);
		UpdateCompareFlag(ScriptParams[1] == pPed->m_weapons[pPed->m_currentWeapon].m_eWeaponType);
		return 0;
	}
	case COMMAND_IS_CURRENT_CHAR_WEAPON:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		UpdateCompareFlag(ScriptParams[1] == pPed->m_weapons[pPed->m_currentWeapon].m_eWeaponType);
		return 0;
	}
	case COMMAND_CLEAR_NUMBER_OF_POWER_PILLS_EATEN:
		CPacManPickups::ResetPowerPillsEatenInRace();
		return 0;
	case COMMAND_ADD_POWER_PILL:
	{
		CollectParameters(&m_nIp, 3);
		CVector pos = *(CVector*)&ScriptParams[0];
		if (pos.z <= MAP_Z_LOW_LIMIT)
			pos.z = CWorld::FindGroundZForCoord(pos.x, pos.y);
		CPacManPickups::GenerateOnePMPickUp(pos);
		return 0;
	}
	case COMMAND_SET_BOAT_CRUISE_SPEED:
	{
		CollectParameters(&m_nIp, 2);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		script_assert(pVehicle);
		script_assert(pVehicle->m_vehType == VEHICLE_TYPE_BOAT);
		CBoat* pBoat = (CBoat*)pVehicle;
		pBoat->AutoPilot.m_nCruiseSpeed = *(float*)&ScriptParams[1];
		return 0;
	}
	case COMMAND_GET_RANDOM_CHAR_IN_AREA:
	{
		CollectParameters(&m_nIp, 4);
		int ped_handle = -1;
		CVector pos = FindPlayerCoors();
		float x1 = *(float*)&ScriptParams[0];
		float y1 = *(float*)&ScriptParams[1];
		float x2 = *(float*)&ScriptParams[2];
		float y2 = *(float*)&ScriptParams[3];
		int i = CPools::GetPedPool()->GetSize();
		while (--i && ped_handle == -1){
			CPed* pPed = CPools::GetPedPool()->GetSlot(i);
			if (!pPed)
				continue;
			if (CTheScripts::LastRandomPedId == CPools::GetPedPool()->GetIndex(pPed))
				continue;
			if (pPed->CharCreatedBy != RANDOM_CHAR)
				continue;
			if (!pPed->IsPedInControl())
				continue;
			if (pPed->bRemoveFromWorld)
				continue;
			if (pPed->bFadeOut)
				continue;
			if (pPed->GetModelIndex() == MI_SCUM_WOM || pPed->GetModelIndex() == MI_SCUM_MAN)
				continue;
			if (!ThisIsAValidRandomPed(pPed->m_nPedType))
				continue;
			if (pPed->bIsLeader || pPed->m_leader)
				continue;
			if (!pPed->IsWithinArea(x1, y1, x2, y2))
				continue;
			if (pos.z - PED_FIND_Z_OFFSET > pPed->GetPosition().z)
				continue;
			if (pos.z + PED_FIND_Z_OFFSET < pPed->GetPosition().z)
				continue;
			ped_handle = CPools::GetPedPool()->GetIndex(pPed);
			CTheScripts::LastRandomPedId = ped_handle;
			pPed->CharCreatedBy = MISSION_CHAR;
			pPed->bRespondsToThreats = false;
			++CPopulation::ms_nTotalMissionPeds;
			if (m_bIsMissionScript)
				CTheScripts::MissionCleanUp.AddEntityToList(ped_handle, CLEANUP_CHAR);
		}
		ScriptParams[0] = ped_handle;
		StoreParameters(&m_nIp, 1);
		return 0;
	}
	case COMMAND_GET_RANDOM_CHAR_IN_ZONE:
	{
		char zone[KEY_LENGTH_IN_SCRIPT];
		strncpy(zone, (const char*)&CTheScripts::ScriptSpace[m_nIp], KEY_LENGTH_IN_SCRIPT);
		int nZone = CTheZones::FindZoneByLabelAndReturnIndex(zone);
		if (nZone != -1)
			m_nIp += KEY_LENGTH_IN_SCRIPT;
		CZone* pZone = CTheZones::GetZone(nZone);
		int ped_handle = -1;
		CVector pos = FindPlayerCoors();
		int i = CPools::GetPedPool()->GetSize();
		while (--i && ped_handle == -1) {
			CPed* pPed = CPools::GetPedPool()->GetSlot(i);
			if (!pPed)
				continue;
			if (CTheScripts::LastRandomPedId == CPools::GetPedPool()->GetIndex(pPed))
				continue;
			if (pPed->CharCreatedBy != RANDOM_CHAR)
				continue;
			if (!pPed->IsPedInControl())
				continue;
			if (pPed->bRemoveFromWorld)
				continue;
			if (pPed->bFadeOut)
				continue;
			if (pPed->GetModelIndex() == MI_SCUM_WOM || pPed->GetModelIndex() == MI_SCUM_MAN)
				continue;
			if (!ThisIsAValidRandomPed(pPed->m_nPedType))
				continue;
			if (pPed->bIsLeader || pPed->m_leader)
				continue;
			if (!CTheZones::PointLiesWithinZone(&pPed->GetPosition(), pZone))
				continue;
			if (pos.z - PED_FIND_Z_OFFSET > pPed->GetPosition().z)
				continue;
			if (pos.z + PED_FIND_Z_OFFSET < pPed->GetPosition().z)
				continue;
			ped_handle = CPools::GetPedPool()->GetIndex(pPed);
			CTheScripts::LastRandomPedId = ped_handle;
			pPed->CharCreatedBy = MISSION_CHAR;
			pPed->bRespondsToThreats = false;
			++CPopulation::ms_nTotalMissionPeds;
			if (m_bIsMissionScript)
				CTheScripts::MissionCleanUp.AddEntityToList(ped_handle, CLEANUP_CHAR);
		}
		ScriptParams[0] = ped_handle;
		StoreParameters(&m_nIp, 1);
		return 0;
	}
	case COMMAND_IS_PLAYER_IN_TAXI:
	{
		CollectParameters(&m_nIp, 1);
		CPed* pPed = CWorld::Players[ScriptParams[0]].m_pPed;
		script_assert(pPed);
		UpdateCompareFlag(pPed->bInVehicle && pPed->m_pMyVehicle->IsTaxi());
		return 0;
	}
	case COMMAND_IS_PLAYER_SHOOTING:
	{
		CollectParameters(&m_nIp, 1);
		CPed* pPed = CWorld::Players[ScriptParams[0]].m_pPed;
		script_assert(pPed);
		UpdateCompareFlag(pPed->bIsShooting);
		return 0;
	}
	case COMMAND_IS_CHAR_SHOOTING:
	{
		CollectParameters(&m_nIp, 1);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		UpdateCompareFlag(pPed->bIsShooting);
		return 0;
	}
	case COMMAND_CREATE_MONEY_PICKUP:
	{
		CollectParameters(&m_nIp, 4);
		CVector pos = *(CVector*)&ScriptParams[0];
		if (pos.z <= MAP_Z_LOW_LIMIT)
			pos.z = CWorld::FindGroundZForCoord(pos.x, pos.y) + PICKUP_PLACEMENT_OFFSET;
		CPickups::GetActualPickupIndex(CollectNextParameterWithoutIncreasingPC(m_nIp));
		ScriptParams[0] = CPickups::GenerateNewOne(pos, MI_MONEY, PICKUP_MONEY, ScriptParams[3]);
		StoreParameters(&m_nIp, 1);
		return 0;
	}
	case COMMAND_SET_CHAR_ACCURACY:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		pPed->m_wepAccuracy = ScriptParams[1];
		return 0;
	}
	case COMMAND_GET_CAR_SPEED:
	{
		CollectParameters(&m_nIp, 1);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		script_assert(pVehicle);
		*(float*)&ScriptParams[0] = pVehicle->GetSpeed().Magnitude() * GAME_SPEED_TO_METERS_PER_SECOND;
		StoreParameters(&m_nIp, 1);
		return 0;
	}
	case COMMAND_LOAD_CUTSCENE:
	{
		char name[KEY_LENGTH_IN_SCRIPT];
		strncpy(name, (const char*)&CTheScripts::ScriptSpace[m_nIp], KEY_LENGTH_IN_SCRIPT);
		m_nIp += KEY_LENGTH_IN_SCRIPT;
		CCutsceneMgr::LoadCutsceneData(name);
		return 0;
	}
	case COMMAND_CREATE_CUTSCENE_OBJECT:
	{
		CollectParameters(&m_nIp, 1);
		CCutsceneObject* pCutObj = CCutsceneMgr::CreateCutsceneObject(ScriptParams[0]);
		ScriptParams[0] = CPools::GetObjectPool()->GetIndex(pCutObj);
		StoreParameters(&m_nIp, 1);
		return 0;
	}
	case COMMAND_SET_CUTSCENE_ANIM:
	{
		CollectParameters(&m_nIp, 1);
		char name[KEY_LENGTH_IN_SCRIPT];
		CObject* pObject = CPools::GetObjectPool()->GetAt(ScriptParams[0]);
		script_assert(pObject);
		strncpy(name, (const char*)&CTheScripts::ScriptSpace[m_nIp], KEY_LENGTH_IN_SCRIPT);
		m_nIp += KEY_LENGTH_IN_SCRIPT;
		CCutsceneMgr::SetCutsceneAnim(name, pObject);
		return 0;
	}
	case COMMAND_START_CUTSCENE:
		CCutsceneMgr::ms_cutsceneLoadStatus = 1;
		return 0;
	case COMMAND_GET_CUTSCENE_TIME:
		ScriptParams[0] = CCutsceneMgr::GetCutsceneTimeInMilleseconds();
		StoreParameters(&m_nIp, 1);
		return 0;
	case COMMAND_HAS_CUTSCENE_FINISHED:
		UpdateCompareFlag(CCutsceneMgr::HasCutsceneFinished());
		return 0;
	case COMMAND_CLEAR_CUTSCENE:
		CCutsceneMgr::DeleteCutsceneData();
		return 0;
	case COMMAND_RESTORE_CAMERA_JUMPCUT:
		TheCamera.RestoreWithJumpCut();
		return 0;
	case COMMAND_CREATE_COLLECTABLE1:
	{
		CollectParameters(&m_nIp, 3);
		CVector pos = *(CVector*)&ScriptParams[0];
		if (pos.z <= MAP_Z_LOW_LIMIT)
			pos.z = CWorld::FindGroundZForCoord(pos.x, pos.y) + PICKUP_PLACEMENT_OFFSET;
		CPickups::GenerateNewOne(pos, MI_COLLECTABLE1, PICKUP_COLLECTABLE1, 0);
		return 0;
	}
	case COMMAND_SET_COLLECTABLE1_TOTAL:
		CollectParameters(&m_nIp, 1);
		CWorld::Players[CWorld::PlayerInFocus].m_nTotalPackages = ScriptParams[0];
		return 0;
	case COMMAND_IS_PROJECTILE_IN_AREA:
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
		UpdateCompareFlag(CProjectileInfo::IsProjectileInRange(infX, supX, infY, supY, infZ, supZ, false));
		if (CTheScripts::DbgFlag)
			CTheScripts::DrawDebugCube(infX, infY, infZ, supX, supY, supZ);
		return 0;
	}
	case COMMAND_DESTROY_PROJECTILES_IN_AREA:
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
		UpdateCompareFlag(CProjectileInfo::IsProjectileInRange(infX, supX, infY, supY, infZ, supZ, true));
		if (CTheScripts::DbgFlag)
			CTheScripts::DrawDebugCube(infX, infY, infZ, supX, supY, supZ);
		return 0;
	}
	case COMMAND_DROP_MINE:
	{
		CollectParameters(&m_nIp, 3);
		CVector pos = *(CVector*)&ScriptParams[0];
		if (pos.z <= MAP_Z_LOW_LIMIT)
			pos.z = CWorld::FindGroundZForCoord(pos.x, pos.y) + PICKUP_PLACEMENT_OFFSET;
		CPickups::GenerateNewOne(pos, MI_CARMINE, PICKUP_MINE_INACTIVE, 0);
		return 0;
	}
	case COMMAND_DROP_NAUTICAL_MINE:
	{
		CollectParameters(&m_nIp, 3);
		CVector pos = *(CVector*)&ScriptParams[0];
		if (pos.z <= MAP_Z_LOW_LIMIT)
			pos.z = CWorld::FindGroundZForCoord(pos.x, pos.y) + PICKUP_PLACEMENT_OFFSET;
		CPickups::GenerateNewOne(pos, MI_NAUTICALMINE, PICKUP_MINE_INACTIVE, 0);
		return 0;
	}
	case COMMAND_IS_CHAR_MODEL:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		UpdateCompareFlag(ScriptParams[1] == pPed->GetModelIndex());
		return 0;
	}
	case COMMAND_LOAD_SPECIAL_MODEL:
	{
		CollectParameters(&m_nIp, 1);
		char name[KEY_LENGTH_IN_SCRIPT];
		strncpy(name, (const char*)&CTheScripts::ScriptSpace[m_nIp], KEY_LENGTH_IN_SCRIPT);
		for (int i = 0; i < KEY_LENGTH_IN_SCRIPT; i++)
			name[i] = tolower(name[i]);
		CStreaming::RequestSpecialModel(ScriptParams[0], name, STREAMFLAGS_DEPENDENCY | STREAMFLAGS_SCRIPTOWNED);
		m_nIp += KEY_LENGTH_IN_SCRIPT;
		return 0;
	}
	case COMMAND_CREATE_CUTSCENE_HEAD:
	{
		CollectParameters(&m_nIp, 2);
		CObject* pObject = CPools::GetObjectPool()->GetAt(ScriptParams[0]);
		script_assert(pObject);
		CCutsceneHead* pCutHead = CCutsceneMgr::AddCutsceneHead(pObject, ScriptParams[1]);
		ScriptParams[0] = CPools::GetObjectPool()->GetIndex(pCutHead);
		StoreParameters(&m_nIp, 1);
		return 0;
	}
	case COMMAND_SET_CUTSCENE_HEAD_ANIM:
	{
		CollectParameters(&m_nIp, 1);
		CObject* pCutHead = CPools::GetObjectPool()->GetAt(ScriptParams[0]);
		script_assert(pCutHead);
		char name[KEY_LENGTH_IN_SCRIPT];
		strncpy(name, (const char*)&CTheScripts::ScriptSpace[m_nIp], KEY_LENGTH_IN_SCRIPT);
		m_nIp += KEY_LENGTH_IN_SCRIPT;
		CTimer::Stop();
		CCutsceneMgr::SetHeadAnim(name, pCutHead);
		CTimer::Update();
		return 0;
	}
	case COMMAND_SIN:
		CollectParameters(&m_nIp, 1);
		*(float*)&ScriptParams[0] = Sin(DEGTORAD(*(float*)&ScriptParams[0]));
		StoreParameters(&m_nIp, 1);
		return 0;
	case COMMAND_COS:
		CollectParameters(&m_nIp, 1);
		*(float*)&ScriptParams[0] = Cos(DEGTORAD(*(float*)&ScriptParams[0]));
		StoreParameters(&m_nIp, 1);
		return 0;
	case COMMAND_GET_CAR_FORWARD_X:
	{
		CollectParameters(&m_nIp, 1);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		float forwardX = pVehicle->GetForward().x / pVehicle->GetForward().Magnitude2D();
		*(float*)&ScriptParams[0] = forwardX;
		StoreParameters(&m_nIp, 1);
		return 0;
	}
	case COMMAND_GET_CAR_FORWARD_Y:
	{
		CollectParameters(&m_nIp, 1);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		float forwardY = pVehicle->GetForward().y / pVehicle->GetForward().Magnitude2D();
		*(float*)&ScriptParams[0] = forwardY;
		StoreParameters(&m_nIp, 1);
		return 0;
	}
	case COMMAND_CHANGE_GARAGE_TYPE:
		CollectParameters(&m_nIp, 2);
		CGarages::ChangeGarageType(ScriptParams[0], ScriptParams[1], 0);
		return 0;
	case COMMAND_ACTIVATE_CRUSHER_CRANE:
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
			DEGTORAD(*(float*)&ScriptParams[9]), true, false,
			*(float*)&ScriptParams[0], *(float*)&ScriptParams[1]);
		return 0;
	}
	case COMMAND_PRINT_WITH_2_NUMBERS:
	{
		wchar* text = CTheScripts::GetTextByKeyFromScript(&m_nIp);
		CollectParameters(&m_nIp, 4);
		CMessages::AddMessageWithNumber(text, ScriptParams[2], ScriptParams[3], ScriptParams[0], ScriptParams[1], -1, -1, -1, -1);
		return 0;
	}
	case COMMAND_PRINT_WITH_2_NUMBERS_NOW:
	{
		wchar* text = CTheScripts::GetTextByKeyFromScript(&m_nIp);
		CollectParameters(&m_nIp, 4);
		CMessages::AddMessageJumpQWithNumber(text, ScriptParams[2], ScriptParams[3], ScriptParams[0], ScriptParams[1], -1, -1, -1, -1);
		return 0;
	}
	case COMMAND_PRINT_WITH_2_NUMBERS_SOON:
	{
		wchar* text = CTheScripts::GetTextByKeyFromScript(&m_nIp);
		CollectParameters(&m_nIp, 4);
		CMessages::AddMessageSoonWithNumber(text, ScriptParams[2], ScriptParams[3], ScriptParams[0], ScriptParams[1], -1, -1, -1, -1);
		return 0;
	}
	case COMMAND_PRINT_WITH_3_NUMBERS:
	{
		wchar* text = CTheScripts::GetTextByKeyFromScript(&m_nIp);
		CollectParameters(&m_nIp, 5);
		CMessages::AddMessageWithNumber(text, ScriptParams[3], ScriptParams[4], ScriptParams[0], ScriptParams[1], ScriptParams[2], -1, -1, -1);
		return 0;
	}
	case COMMAND_PRINT_WITH_3_NUMBERS_NOW:
	{
		wchar* text = CTheScripts::GetTextByKeyFromScript(&m_nIp);
		CollectParameters(&m_nIp, 5);
		CMessages::AddMessageJumpQWithNumber(text, ScriptParams[3], ScriptParams[4], ScriptParams[0], ScriptParams[1], ScriptParams[2], -1, -1, -1);
		return 0;
	}
	case COMMAND_PRINT_WITH_3_NUMBERS_SOON:
	{
		wchar* text = CTheScripts::GetTextByKeyFromScript(&m_nIp);
		CollectParameters(&m_nIp, 5);
		CMessages::AddMessageSoonWithNumber(text, ScriptParams[3], ScriptParams[4], ScriptParams[0], ScriptParams[1], ScriptParams[2], -1, -1, -1);
		return 0;
	}
	case COMMAND_PRINT_WITH_4_NUMBERS:
	{
		wchar* text = CTheScripts::GetTextByKeyFromScript(&m_nIp);
		CollectParameters(&m_nIp, 6);
		CMessages::AddMessageWithNumber(text, ScriptParams[4], ScriptParams[5], ScriptParams[0], ScriptParams[1], ScriptParams[2], ScriptParams[3], -1, -1);
		return 0;
	}
	case COMMAND_PRINT_WITH_4_NUMBERS_NOW:
	{
		wchar* text = CTheScripts::GetTextByKeyFromScript(&m_nIp);
		CollectParameters(&m_nIp, 6);
		CMessages::AddMessageJumpQWithNumber(text, ScriptParams[4], ScriptParams[5], ScriptParams[0], ScriptParams[1], ScriptParams[2], ScriptParams[3], -1, -1);
		return 0;
	}
	case COMMAND_PRINT_WITH_4_NUMBERS_SOON:
	{
		wchar* text = CTheScripts::GetTextByKeyFromScript(&m_nIp);
		CollectParameters(&m_nIp, 6);
		CMessages::AddMessageSoonWithNumber(text, ScriptParams[4], ScriptParams[5], ScriptParams[0], ScriptParams[1], ScriptParams[2], ScriptParams[3], -1, -1);
		return 0;
	}
	case COMMAND_PRINT_WITH_5_NUMBERS:
	{
		wchar* text = CTheScripts::GetTextByKeyFromScript(&m_nIp);
		CollectParameters(&m_nIp, 7);
		CMessages::AddMessageWithNumber(text, ScriptParams[5], ScriptParams[6], ScriptParams[0], ScriptParams[1], ScriptParams[2], ScriptParams[3], ScriptParams[4], -1);
		return 0;
	}
	case COMMAND_PRINT_WITH_5_NUMBERS_NOW:
	{
		wchar* text = CTheScripts::GetTextByKeyFromScript(&m_nIp);
		CollectParameters(&m_nIp, 7);
		CMessages::AddMessageJumpQWithNumber(text, ScriptParams[5], ScriptParams[6], ScriptParams[0], ScriptParams[1], ScriptParams[2], ScriptParams[3], ScriptParams[4], -1);
		return 0;
	}
	case COMMAND_PRINT_WITH_5_NUMBERS_SOON:
	{
		wchar* text = CTheScripts::GetTextByKeyFromScript(&m_nIp);
		CollectParameters(&m_nIp, 7);
		CMessages::AddMessageSoonWithNumber(text, ScriptParams[5], ScriptParams[6], ScriptParams[0], ScriptParams[1], ScriptParams[2], ScriptParams[3], ScriptParams[4], -1);
		return 0;
	}
	case COMMAND_PRINT_WITH_6_NUMBERS:
	{
		wchar* text = CTheScripts::GetTextByKeyFromScript(&m_nIp);
		CollectParameters(&m_nIp, 8);
		CMessages::AddMessageWithNumber(text, ScriptParams[6], ScriptParams[7], ScriptParams[0], ScriptParams[1], ScriptParams[2], ScriptParams[3], ScriptParams[4], ScriptParams[5]);
		return 0;
	}
	case COMMAND_PRINT_WITH_6_NUMBERS_NOW:
	{
		wchar* text = CTheScripts::GetTextByKeyFromScript(&m_nIp);
		CollectParameters(&m_nIp, 8);
		CMessages::AddMessageJumpQWithNumber(text, ScriptParams[6], ScriptParams[7], ScriptParams[0], ScriptParams[1], ScriptParams[2], ScriptParams[3], ScriptParams[4], ScriptParams[5]);
		return 0;
	}
	case COMMAND_PRINT_WITH_6_NUMBERS_SOON:
	{
		wchar* text = CTheScripts::GetTextByKeyFromScript(&m_nIp);
		CollectParameters(&m_nIp, 8);
		CMessages::AddMessageSoonWithNumber(text, ScriptParams[6], ScriptParams[7], ScriptParams[0], ScriptParams[1], ScriptParams[2], ScriptParams[3], ScriptParams[4], ScriptParams[5]);
		return 0;
	}
	case COMMAND_SET_CHAR_OBJ_FOLLOW_CHAR_IN_FORMATION:
	{
		CollectParameters(&m_nIp, 3);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		CPed* pTargetPed = CPools::GetPedPool()->GetAt(ScriptParams[1]);
		pPed->bScriptObjectiveCompleted = false;
		pPed->SetObjective(OBJECTIVE_FOLLOW_CHAR_IN_FORMATION, pTargetPed);
		pPed->SetFormation((eFormation)ScriptParams[2]);
		return 0;
	}
	case COMMAND_PLAYER_MADE_PROGRESS:
		CollectParameters(&m_nIp, 1);
		CStats::ProgressMade += ScriptParams[0];
		return 0;
	case COMMAND_SET_PROGRESS_TOTAL:
		CollectParameters(&m_nIp, 1);
		CStats::TotalProgressInGame = ScriptParams[0];
		return 0;
	case COMMAND_REGISTER_JUMP_DISTANCE:
		CollectParameters(&m_nIp, 1);
		CStats::MaximumJumpDistance = Max(CStats::MaximumJumpDistance, *(float*)&ScriptParams[0]);
		return 0;
	case COMMAND_REGISTER_JUMP_HEIGHT:
		CollectParameters(&m_nIp, 1);
		CStats::MaximumJumpHeight = Max(CStats::MaximumJumpHeight, *(float*)&ScriptParams[0]);
		return 0;
	case COMMAND_REGISTER_JUMP_FLIPS:
		CollectParameters(&m_nIp, 1);
		CStats::MaximumJumpFlips = Max(CStats::MaximumJumpFlips, ScriptParams[0]);
		return 0;
	case COMMAND_REGISTER_JUMP_SPINS:
		CollectParameters(&m_nIp, 1);
		CStats::MaximumJumpSpins = Max(CStats::MaximumJumpSpins, ScriptParams[0]);
		return 0;
	case COMMAND_REGISTER_JUMP_STUNT:
		CollectParameters(&m_nIp, 1);
		CStats::BestStuntJump = Max(CStats::BestStuntJump, ScriptParams[0]);
		return 0;
	case COMMAND_REGISTER_UNIQUE_JUMP_FOUND:
		++CStats::NumberOfUniqueJumpsFound;
		return 0;
	case COMMAND_SET_UNIQUE_JUMPS_TOTAL:
		CollectParameters(&m_nIp, 1);
		CStats::TotalNumberOfUniqueJumps = ScriptParams[0];
		return 0;
	case COMMAND_REGISTER_PASSENGER_DROPPED_OFF_TAXI:
		++CStats::PassengersDroppedOffWithTaxi;
		return 0;
	case COMMAND_REGISTER_MONEY_MADE_TAXI:
		CollectParameters(&m_nIp, 1);
		CStats::MoneyMadeWithTaxi += ScriptParams[0];
		return 0;
	case COMMAND_REGISTER_MISSION_GIVEN:
		++CStats::MissionsGiven;
		return 0;
	case COMMAND_REGISTER_MISSION_PASSED:
	{
		char name[KEY_LENGTH_IN_SCRIPT];
		strncpy(name, (const char*)&CTheScripts::ScriptSpace[m_nIp], KEY_LENGTH_IN_SCRIPT);
		m_nIp += KEY_LENGTH_IN_SCRIPT;
		strncpy(CStats::LastMissionPassedName, name, KEY_LENGTH_IN_SCRIPT);
		++CStats::MissionsPassed;
		CStats::CheckPointReachedSuccessfully();
		return 0;
	}
	case COMMAND_SET_CHAR_RUNNING:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		pPed->bIsRunning = (ScriptParams[1] != 0);
		return 0;
	}
	case COMMAND_REMOVE_ALL_SCRIPT_FIRES:
		gFireManager.RemoveAllScriptFires();
		return 0;
	case COMMAND_IS_FIRST_CAR_COLOUR:
	{
		CollectParameters(&m_nIp, 2);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		script_assert(pVehicle);
		UpdateCompareFlag(pVehicle->m_currentColour1 == ScriptParams[1]);
		return 0;
	}
	case COMMAND_IS_SECOND_CAR_COLOUR:
	{
		CollectParameters(&m_nIp, 2);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		script_assert(pVehicle);
		UpdateCompareFlag(pVehicle->m_currentColour2 == ScriptParams[1]);
		return 0;
	}
	case COMMAND_HAS_CHAR_BEEN_DAMAGED_BY_WEAPON:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		if (!pPed)
			printf("HAS_CHAR_BEEN_DAMAGED_BY_WEAPON - Character doesn't exist\n");
		UpdateCompareFlag(pPed && pPed->m_lastWepDam == ScriptParams[1]);
		return 0;
	}
	case COMMAND_HAS_CAR_BEEN_DAMAGED_BY_WEAPON:
	{
		CollectParameters(&m_nIp, 2);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		if (!pVehicle)
			printf("HAS_CAR_BEEN_DAMAGED_BY_WEAPON - Vehicle doesn't exist\n");
		UpdateCompareFlag(pVehicle && pVehicle->m_nLastWeaponDamage == ScriptParams[1]);
		return 0;
	}
	case COMMAND_IS_CHAR_IN_CHARS_GROUP:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		CPed* pLeader = CPools::GetPedPool()->GetAt(ScriptParams[1]);
		script_assert(pPed);
		script_assert(pLeader);
		UpdateCompareFlag(pPed->m_leader == pLeader);
		return 0;
	}
	default:
		script_assert(0);
	}
	return -1;
}
