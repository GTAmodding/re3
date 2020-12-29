#include "common.h"

#include "Script.h"
#include "ScriptCommands.h"

#include "audio_enums.h" //needs to be before AudioManager.h
#include "vehicle_enums.h" //needs to be before AudioManager.h

#include "AudioManager.h"
#include "Vehicle.h"

#include "CarCtrl.h"
#include "ColStore.h"
#include "Coronas.h"
#include "CutsceneMgr.h"
#include "DMAudio.h"
#include "Explosion.h"
#include "Fluff.h"
#include "GameLogic.h"
#include "General.h"
#include "Glass.h"
#include "Hud.h"
#include "MBlur.h"
#include "Pad.h"
#include "Pickups.h"
#include "Pools.h"
#include "Population.h"
#include "Radar.h"
#include "RoadBlocks.h"
#include "Ropes.h"
#include "SetPieces.h"
#include "SpecialFX.h"
#include "Stats.h"
#include "Streaming.h"
#include "Timecycle.h"
#include "User.h"
#include "World.h"
#include "Zones.h"

int8 CRunningScript::ProcessCommands1200To1299(int32 command)
{
	switch (command) {
	case COMMAND_IS_INT_VAR_GREATER_THAN_CONSTANT:
	{
		int32* ptr = GetPointerToScriptVariable(&m_nIp, VAR_GLOBAL);
		CollectParameters(&m_nIp, 1);
		UpdateCompareFlag(*ptr > ScriptParams[0]);
		return 0;
	}
	case COMMAND_IS_INT_LVAR_GREATER_THAN_CONSTANT:
	{
		int32* ptr = GetPointerToScriptVariable(&m_nIp, VAR_LOCAL);
		CollectParameters(&m_nIp, 1);
		UpdateCompareFlag(*ptr > ScriptParams[0]);
		return 0;
	}
	case COMMAND_IS_CONSTANT_GREATER_THAN_INT_VAR:
	{
		CollectParameters(&m_nIp, 1);
		int32* ptr = GetPointerToScriptVariable(&m_nIp, VAR_GLOBAL);
		UpdateCompareFlag(ScriptParams[0] > *ptr);
		return 0;
	}
	case COMMAND_IS_CONSTANT_GREATER_THAN_INT_LVAR:
	{
		CollectParameters(&m_nIp, 1);
		int32* ptr = GetPointerToScriptVariable(&m_nIp, VAR_LOCAL);
		UpdateCompareFlag(ScriptParams[0] > *ptr);
		return 0;
	}
	case COMMAND_IS_INT_VAR_GREATER_OR_EQUAL_TO_CONSTANT:
	{
		int32* ptr = GetPointerToScriptVariable(&m_nIp, VAR_GLOBAL);
		CollectParameters(&m_nIp, 1);
		UpdateCompareFlag(*ptr >= ScriptParams[0]);
		return 0;
	}
	case COMMAND_IS_INT_LVAR_GREATER_OR_EQUAL_TO_CONSTANT:
	{
		int32* ptr = GetPointerToScriptVariable(&m_nIp, VAR_LOCAL);
		CollectParameters(&m_nIp, 1);
		UpdateCompareFlag(*ptr >= ScriptParams[0]);
		return 0;
	}
	case COMMAND_IS_CONSTANT_GREATER_OR_EQUAL_TO_INT_VAR:
	{
		CollectParameters(&m_nIp, 1);
		int32* ptr = GetPointerToScriptVariable(&m_nIp, VAR_GLOBAL);
		UpdateCompareFlag(ScriptParams[0] >= *ptr);
		return 0;
	}
	case COMMAND_IS_CONSTANT_GREATER_OR_EQUAL_TO_INT_LVAR:
	{
		CollectParameters(&m_nIp, 1);
		int32* ptr = GetPointerToScriptVariable(&m_nIp, VAR_LOCAL);
		UpdateCompareFlag(ScriptParams[0] >= *ptr);
		return 0;
	}
	case COMMAND_GET_CHAR_WEAPON_IN_SLOT:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		ScriptParams[0] = pPed->GetWeapon(ScriptParams[1]).m_eWeaponType;
		ScriptParams[1] = pPed->GetWeapon(ScriptParams[1]).m_nAmmoTotal;
		ScriptParams[2] = CPickups::ModelForWeapon((eWeaponType)ScriptParams[0]);
		StoreParameters(&m_nIp, 3);
		return 0;
	}
	case COMMAND_GET_CLOSEST_STRAIGHT_ROAD:
	{
		CollectParameters(&m_nIp, 5);
		int node1, node2;
		float angle;
		ThePaths.FindNodePairClosestToCoors(*(CVector*)&ScriptParams[0], PATH_CAR, &node1, &node2, &angle,
			*(float*)&ScriptParams[3], *(float*)&ScriptParams[4], true, true);
		if (node1 == -1) {
			for (int i = 0; i < 7; i++)
				ScriptParams[i] = 0;
		}
		else {
			*(CVector*)&ScriptParams[0] = ThePaths.FindNodeCoorsForScript(node1);
			*(CVector*)&ScriptParams[3] = ThePaths.FindNodeCoorsForScript(node2);
			*(float*)&ScriptParams[6] = angle;
		}
		StoreParameters(&m_nIp, 7);
		return 0;
	}
	case COMMAND_SET_CAR_FORWARD_SPEED:
	{
		CollectParameters(&m_nIp, 2);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		script_assert(pVehicle);
		float speed = *(float*)&ScriptParams[1] / GAME_SPEED_TO_CARAI_SPEED;
		pVehicle->SetMoveSpeed(pVehicle->GetForward() * speed);
		if (pVehicle->IsRealHeli() && pVehicle->IsCar())
			((CAutomobile*)pVehicle)->m_aWheelSpeed[1] = 0.22f;
		return 0;
	}
	case COMMAND_SET_AREA_VISIBLE:
		CollectParameters(&m_nIp, 1);
		CGame::currArea = ScriptParams[0];
		CStreaming::RemoveBuildingsNotInArea(ScriptParams[0]); 
		return 0;
	case COMMAND_SET_CUTSCENE_ANIM_TO_LOOP:
	{
		char key[KEY_LENGTH_IN_SCRIPT];
		CTheScripts::ReadTextLabelFromScript(&m_nIp, key);
		m_nIp += KEY_LENGTH_IN_SCRIPT;
		CCutsceneMgr::SetCutsceneAnimToLoop(key);
		return 0;
	}
	case COMMAND_MARK_CAR_AS_CONVOY_CAR:
	{
		CollectParameters(&m_nIp, 2);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		script_assert(pVehicle);
		pVehicle->bPartOfConvoy = ScriptParams[1];
		return 0;
	}
	case COMMAND_RESET_HAVOC_CAUSED_BY_PLAYER:
	{
		CollectParameters(&m_nIp, 1);
		CWorld::Players[ScriptParams[0]].m_nHavocLevel = 0;
		return 0;
	}
	case COMMAND_GET_HAVOC_CAUSED_BY_PLAYER:
	{
		CollectParameters(&m_nIp, 1);
		ScriptParams[0] = CWorld::Players[ScriptParams[0]].m_nHavocLevel;
		StoreParameters(&m_nIp, 1);
		return 0;
	}
	case COMMAND_CREATE_SCRIPT_ROADBLOCK:
	{
		CollectParameters(&m_nIp, 6);
		CRoadBlocks::RegisterScriptRoadBlock(*(CVector*)&ScriptParams[0], *(CVector*)&ScriptParams[3]);
		return 0;
	}
	case COMMAND_CLEAR_ALL_SCRIPT_ROADBLOCKS:
	{
		CRoadBlocks::ClearScriptRoadBlocks();
		return 0;
	}
	case COMMAND_SET_CHAR_OBJ_WALK_TO_CHAR:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		CPed* pTargetPed = CPools::GetPedPool()->GetAt(ScriptParams[1]);
		script_assert(pTargetPed);
		pPed->bScriptObjectiveCompleted = false;
		pPed->SetObjective(OBJECTIVE_GOTO_CHAR_ON_FOOT_WALKING, pTargetPed);
		return 0;
	}
	//case COMMAND_IS_PICKUP_IN_ZONE:
	case COMMAND_GET_OFFSET_FROM_CHAR_IN_WORLD_COORDS:
	{
		CollectParameters(&m_nIp, 4);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		CVector result = Multiply3x3(pPed->GetMatrix(), *(CVector*)&ScriptParams[1]) + pPed->GetPosition();
		*(CVector*)&ScriptParams[0] = result;
		StoreParameters(&m_nIp, 3);
		return 0;
	}
	case COMMAND_HAS_CHAR_BEEN_PHOTOGRAPHED:
	{
		CollectParameters(&m_nIp, 1);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		bool result = false;
		if (pPed->bHasBeenPhotographed) {
			result = true;
			pPed->bHasBeenPhotographed = false;
		}
		UpdateCompareFlag(result);
		return 0;
	}
	case COMMAND_SET_CHAR_OBJ_AIM_GUN_AT_CHAR:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		CPed* pTargetPed = CPools::GetPedPool()->GetAt(ScriptParams[1]);
		script_assert(pTargetPed);
		pPed->bScriptObjectiveCompleted = false;
		pPed->SetObjective(OBJECTIVE_AIM_GUN_AT, pTargetPed);
		return 0;
	}
	case COMMAND_SWITCH_SECURITY_CAMERA:
	{
		CollectParameters(&m_nIp, 1);
		CSpecialFX::bVideoCam = ScriptParams[0] != 0;
		return 0;
	}
	//case COMMAND_IS_CHAR_IN_FLYING_VEHICLE:
	case COMMAND_IS_PLAYER_IN_FLYING_VEHICLE:
	{
		CollectParameters(&m_nIp, 1);
		CPed* pPed = CWorld::Players[ScriptParams[0]].m_pPed;
		script_assert(pPed);
		UpdateCompareFlag(pPed->bInVehicle && (pPed->m_pMyVehicle->GetVehicleAppearance() == VEHICLE_APPEARANCE_HELI || pPed->m_pMyVehicle->GetVehicleAppearance() == VEHICLE_APPEARANCE_PLANE));
		return 0;
	}
	//case COMMAND_HAS_SONY_CD_BEEN_READ:
	//case COMMAND_GET_NUMBER_OF_SONY_CDS_READ:
	//case COMMAND_ADD_SHORT_RANGE_BLIP_FOR_COORD_OLD:
	//case COMMAND_ADD_SHORT_RANGE_BLIP_FOR_COORD:
	case COMMAND_ADD_SHORT_RANGE_SPRITE_BLIP_FOR_COORD:
	{
		CollectParameters(&m_nIp, 4);
		CVector pos = *(CVector*)&ScriptParams[0];
		if (pos.z <= MAP_Z_LOW_LIMIT)
			pos.z = CWorld::FindGroundZForCoord(pos.x, pos.y);
		CRadar::GetActualBlipArrayIndex(CollectNextParameterWithoutIncreasingPC(m_nIp));
		int id = CRadar::SetShortRangeCoordBlip(BLIP_COORD, pos, 5, BLIP_DISPLAY_BOTH);
		CRadar::SetBlipSprite(id, ScriptParams[3]);
		ScriptParams[0] = id;
		StoreParameters(&m_nIp, 1);
		return 0;
	}
	case COMMAND_ADD_MONEY_SPENT_ON_CLOTHES:
		CollectParameters(&m_nIp, 1);
		CStats::MoneySpentOnFashion(ScriptParams[0]);
		return 0;
		
	case COMMAND_SET_HELI_ORIENTATION:
	{
		CollectParameters(&m_nIp, 2);
		CAutomobile* pHeli = (CAutomobile*)CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		script_assert(pHeli && pHeli->IsCar() && pHeli->IsRealHeli());
		float fAngle = DEGTORAD(*(float*)&ScriptParams[1] - 90.0f);
		while (fAngle < 0.0f)
			fAngle += TWOPI;
		while (fAngle > TWOPI)
			fAngle -= TWOPI;
		pHeli->SetHeliOrientation(fAngle);
		return 0;
	}
	case COMMAND_CLEAR_HELI_ORIENTATION:
	{
		CollectParameters(&m_nIp, 1);
		CAutomobile* pHeli = (CAutomobile*)CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		script_assert(pHeli && pHeli->IsCar() && pHeli->IsRealHeli());
		pHeli->ClearHeliOrientation();
		return 0;
	}
	case COMMAND_PLANE_GOTO_COORDS:
	{
		CollectParameters(&m_nIp, 5);
		CAutomobile* pPlane = (CAutomobile*)CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		script_assert(pPlane && pPlane->IsCar() && pPlane->IsRealPlane());
		pPlane->TellPlaneToGoToCoors(*(float*)&ScriptParams[1], *(float*)&ScriptParams[2], *(float*)&ScriptParams[3], ScriptParams[4]);
		return 0;
	}
	case COMMAND_GET_NTH_CLOSEST_CAR_NODE:
	{
		CollectParameters(&m_nIp, 4);
		CVector pos = *(CVector*)&ScriptParams[0];
		if (pos.z <= MAP_Z_LOW_LIMIT)
			pos.z = CWorld::FindGroundZForCoord(pos.x, pos.y);
		*(CVector*)&ScriptParams[0] = ThePaths.FindNodeCoorsForScript(ThePaths.FindNthNodeClosestToCoors(pos, 0, 999999.9f, true, true, ScriptParams[3] - 1));
		StoreParameters(&m_nIp, 3);
		return 0;
	}
	//case COMMAND_GET_NTH_CLOSEST_CHAR_NODE:
	case COMMAND_DRAW_WEAPONSHOP_CORONA:
	{
		CollectParameters(&m_nIp, 9);
		CVector pos = *(CVector*)&ScriptParams[0];
		if (pos.z <= MAP_Z_LOW_LIMIT)
			pos.z = CWorld::FindGroundZForCoord(pos.x, pos.y);
		CCoronas::RegisterCorona((uintptr)this + m_nIp, ScriptParams[6], ScriptParams[7], ScriptParams[8], 255, pos, *(float*)&ScriptParams[3],
			150.0f, ScriptParams[4], ScriptParams[5], 1, 0, 0, 0.0f, false, 0.2f);
		return 0;
	}
	case COMMAND_SET_ENABLE_RC_DETONATE_ON_CONTACT:
	{
		CollectParameters(&m_nIp, 1);
		CVehicle::bDisableRemoteDetonationOnContact = (ScriptParams[0] == 0);
		return 0;
	}
	case COMMAND_FREEZE_CHAR_POSITION:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		pPed->bIsFrozen = ScriptParams[1];
		return 0;
	}
	case COMMAND_SET_CHAR_DROWNS_IN_WATER:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		pPed->bDrownsInWater = ScriptParams[1];
		return 0;
	}
	case COMMAND_SET_OBJECT_RECORDS_COLLISIONS:
	{
		CollectParameters(&m_nIp, 2);
		CObject* pObject = CPools::GetObjectPool()->GetAt(ScriptParams[0]);
		script_assert(pObject);
		pObject->bUseCollisionRecords = ScriptParams[1];
		return 0;
	}
	case COMMAND_HAS_OBJECT_COLLIDED_WITH_ANYTHING:
	{
		CollectParameters(&m_nIp, 1);
		CObject* pObject = CPools::GetObjectPool()->GetAt(ScriptParams[0]);
		script_assert(pObject);
		UpdateCompareFlag(pObject->m_nCollisionRecords != 0);
		return 0;
	}
	case COMMAND_REMOVE_RC_BUGGY:
	{
		CWorld::Players[CWorld::PlayerInFocus].BlowUpRCBuggy(false);
		return 0;
	}
	//case COMMAND_HAS_PHOTOGRAPH_BEEN_TAKEN:
	case COMMAND_GET_CHAR_ARMOUR:
	{
		CollectParameters(&m_nIp, 1);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		ScriptParams[0] = pPed->m_fArmour;
		StoreParameters(&m_nIp, 1);
		return 0;
	}
	//case COMMAND_SET_CHAR_ARMOUR:
	case COMMAND_SET_HELI_STABILISER:
	{
		CollectParameters(&m_nIp, 2);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		script_assert(pVehicle);
		pVehicle->bHeliMinimumTilt = ScriptParams[1];
		return 0;
	}
	case COMMAND_SET_CAR_STRAIGHT_LINE_DISTANCE:
	{
		CollectParameters(&m_nIp, 2);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		script_assert(pVehicle);
		pVehicle->AutoPilot.m_nSwitchDistance = ScriptParams[1];
		return 0;
	}
	case COMMAND_POP_CAR_BOOT:
	{
		CollectParameters(&m_nIp, 1);
		CAutomobile* pCar = (CAutomobile*)CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		script_assert(pCar&& pCar->IsCar());
		pCar->PopBoot();
		return 0;
	}
	case COMMAND_SHUT_PLAYER_UP:
	{
		CollectParameters(&m_nIp, 2);
		DMAudio.ShutUpPlayerTalking(!!ScriptParams[1]);
		return 0;
	}
	case COMMAND_SET_PLAYER_MOOD:
	{
		CollectParameters(&m_nIp, 3);
		DMAudio.SetPlayersMood(ScriptParams[1], ScriptParams[2]);
		return 0;
	}
	case COMMAND_REQUEST_COLLISION:
	{
		CollectParameters(&m_nIp, 2);
		CVector2D pos;
		pos.x = *(float*)&ScriptParams[0];
		pos.y = *(float*)&ScriptParams[1];
		CColStore::RequestCollision(pos);
		return 0;
	}
	case COMMAND_LOCATE_OBJECT_2D:
	case COMMAND_LOCATE_OBJECT_3D:
		LocateObjectCommand(command, &m_nIp);
		return 0;
	case COMMAND_IS_OBJECT_IN_WATER:
	{
		CollectParameters(&m_nIp, 1);
		CObject* pObject = CPools::GetObjectPool()->GetAt(ScriptParams[0]);
		script_assert(pObject);
		UpdateCompareFlag(pObject->bIsInWater);
		return 0;
	}
	//case COMMAND_SET_CHAR_OBJ_STEAL_ANY_CAR_EVEN_MISSION_CAR:
	case COMMAND_IS_OBJECT_IN_AREA_2D:
	case COMMAND_IS_OBJECT_IN_AREA_3D:
		ObjectInAreaCheckCommand(command, &m_nIp);
		return 0;
	case COMMAND_SET_CHAR_CROUCH:
	{
		CollectParameters(&m_nIp, 3);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		if (ScriptParams[1]) {
			pPed->bIsDucking = true;
			pPed->SetDuck(ScriptParams[2], true);
		}
		else {
			pPed->ClearDuck(true);
			pPed->bIsDucking = false;
		}
		return 0;
	}
	case COMMAND_SET_ZONE_CIVILIAN_CAR_INFO:
	{
		char label[12];
		int16 carDensities[CCarCtrl::NUM_CAR_CLASSES] = { 0 };
		int16 boatDensities[CCarCtrl::NUM_BOAT_CLASSES] = { 0 };
		int i;

		CTheScripts::ReadTextLabelFromScript(&m_nIp, label);
		m_nIp += KEY_LENGTH_IN_SCRIPT;
		CollectParameters(&m_nIp, 12);
		for (i = 0; i < CCarCtrl::NUM_CAR_CLASSES; i++)
			carDensities[i] = ScriptParams[i + 1];
		for (i = 0; i < CCarCtrl::NUM_BOAT_CLASSES; i++)
			boatDensities[i] = ScriptParams[i + 1 + CCarCtrl::NUM_CAR_CLASSES];
		int zone = CTheZones::FindZoneByLabelAndReturnIndex(label, ZONE_INFO);
		if (zone < 0) {
			debug("Couldn't find zone - %s\n", label);
			return 0;
		}
		while (zone >= 0) {
			CTheZones::SetZoneCivilianCarInfo(zone, ScriptParams[0], carDensities, boatDensities);
			zone = CTheZones::FindNextZoneByLabelAndReturnIndex(label, ZONE_INFO);
		}
		return 0;
	}
	case COMMAND_REQUEST_ANIMATION:
	{
		char key[KEY_LENGTH_IN_SCRIPT];
		CTheScripts::ReadTextLabelFromScript(&m_nIp, key);
		m_nIp += KEY_LENGTH_IN_SCRIPT;
		CStreaming::RequestAnim(CAnimManager::GetAnimationBlockIndex(key), STREAMFLAGS_SCRIPTOWNED);
		return 0;
	}
	case COMMAND_HAS_ANIMATION_LOADED:
	{
		char key[KEY_LENGTH_IN_SCRIPT];
		CTheScripts::ReadTextLabelFromScript(&m_nIp, key);
		m_nIp += KEY_LENGTH_IN_SCRIPT;
		UpdateCompareFlag(CAnimManager::GetAnimationBlock(key)->isLoaded);
		return 0;
	}
	case COMMAND_REMOVE_ANIMATION:
	{
		char key[KEY_LENGTH_IN_SCRIPT];
		CTheScripts::ReadTextLabelFromScript(&m_nIp, key);
		m_nIp += KEY_LENGTH_IN_SCRIPT;
		CStreaming::RemoveAnim(CAnimManager::GetAnimationBlockIndex(key));
		return 0;
	}
	case COMMAND_IS_CHAR_WAITING_FOR_WORLD_COLLISION:
	{
		CollectParameters(&m_nIp, 1);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		UpdateCompareFlag(pPed->bIsStaticWaitingForCollision);
		return 0;
	}
	case COMMAND_IS_CAR_WAITING_FOR_WORLD_COLLISION:
	{
		CollectParameters(&m_nIp, 1);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		script_assert(pVehicle);
		UpdateCompareFlag(pVehicle->bIsStaticWaitingForCollision);
		return 0;
	}
	case COMMAND_IS_OBJECT_WAITING_FOR_WORLD_COLLISION:
	{
		CollectParameters(&m_nIp, 1);
		CObject* pObject = CPools::GetObjectPool()->GetAt(ScriptParams[0]);
		script_assert(pObject);
		UpdateCompareFlag(pObject->bIsStaticWaitingForCollision);
		return 0;
	}
	case COMMAND_SET_CHAR_SHUFFLE_INTO_DRIVERS_SEAT:
	{
		CollectParameters(&m_nIp, 1);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		pPed->PedShuffle();
		return 0;
	}
	case COMMAND_ATTACH_CHAR_TO_OBJECT:
	{
		CollectParameters(&m_nIp, 8);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		CObject* pObject = CPools::GetObjectPool()->GetAt(ScriptParams[1]);
		pPed->AttachPedToEntity(pObject, *(CVector*)&ScriptParams[2], ScriptParams[5], DEGTORAD(ScriptParams[6]), (eWeaponType)ScriptParams[7]);
		return 0;
	}
	case COMMAND_SET_CHAR_AS_PLAYER_FRIEND:
	{
		CollectParameters(&m_nIp, 3);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		pPed->bIsPlayerFriend = ScriptParams[2];
		return 0;
	}
	//case COMMAND_DISPLAY_NTH_ONSCREEN_COUNTER:
	case COMMAND_DISPLAY_NTH_ONSCREEN_COUNTER_WITH_STRING:
	{
		char onscreen_str[12];
		script_assert(CTheScripts::ScriptSpace[m_nIp++] == ARGUMENT_GLOBALVAR);
		uint16 var = CTheScripts::Read2BytesFromScript(&m_nIp);
		CollectParameters(&m_nIp, 2);
		wchar* text = TheText.Get((char*)&CTheScripts::ScriptSpace[m_nIp]); // ???
		strncpy(onscreen_str, (char*)&CTheScripts::ScriptSpace[m_nIp], KEY_LENGTH_IN_SCRIPT);
		m_nIp += KEY_LENGTH_IN_SCRIPT;
		CUserDisplay::OnscnTimer.AddCounter(var, ScriptParams[0], onscreen_str, ScriptParams[1] - 1);
		return 0;
	}
	case COMMAND_ADD_SET_PIECE:
	{
		CollectParameters(&m_nIp, 13);
		CSetPieces::AddOne(ScriptParams[0],
			*(CVector2D*)&ScriptParams[1], *(CVector2D*)&ScriptParams[3],
			*(CVector2D*)&ScriptParams[5], *(CVector2D*)&ScriptParams[7],
			*(CVector2D*)&ScriptParams[9], *(CVector2D*)&ScriptParams[11]);
		return 0;
	}
	case COMMAND_SET_EXTRA_COLOURS:
	{
		CollectParameters(&m_nIp, 2);
		CTimeCycle::StartExtraColour(ScriptParams[0]-1, ScriptParams[1] != 0);
		return 0;
	}
	case COMMAND_CLEAR_EXTRA_COLOURS:
	{
		CollectParameters(&m_nIp, 1);
		CTimeCycle::StopExtraColour(ScriptParams[0]);
		return 0;
	}
	//case COMMAND_CLOSE_CAR_BOOT:
	case COMMAND_GET_WHEELIE_STATS:
	{
		CollectParameters(&m_nIp, 1);
		CPlayerInfo* pPlayerInfo = &CWorld::Players[ScriptParams[0]];
		ScriptParams[0] = pPlayerInfo->m_nLastTimeCarSpentOnTwoWheels;
		ScriptParams[1] = *(int*)&pPlayerInfo->m_nLastDistanceCarTravelledOnTwoWheels;
		ScriptParams[2] = pPlayerInfo->m_nLastTimeSpentOnWheelie;
		ScriptParams[3] = *(int*)&pPlayerInfo->m_nLastDistanceTravelledOnWheelie;
		ScriptParams[4] = pPlayerInfo->m_nLastTimeSpentOnStoppie;
		ScriptParams[5] = *(int*)&pPlayerInfo->m_nLastDistanceTravelledOnStoppie;
		StoreParameters(&m_nIp, 6);
		pPlayerInfo->m_nLastTimeCarSpentOnTwoWheels = 0;
		pPlayerInfo->m_nLastDistanceCarTravelledOnTwoWheels = 0.0f;
		pPlayerInfo->m_nLastTimeSpentOnWheelie = 0;
		pPlayerInfo->m_nLastDistanceTravelledOnWheelie = 0.0f;
		pPlayerInfo->m_nLastTimeSpentOnStoppie = 0;
		pPlayerInfo->m_nLastDistanceTravelledOnStoppie = 0.0f;
		return 0;
	}
	//case COMMAND_DISARM_CHAR:
	case COMMAND_BURST_CAR_TYRE:
	{
		CollectParameters(&m_nIp, 2);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		if (pVehicle->IsBike()) {
			if (ScriptParams[1] == 2)
				ScriptParams[1] = 0;
			else if (ScriptParams[1] == 3)
				ScriptParams[1] = 1;
			pVehicle->BurstTyre(ScriptParams[1], true);
		}
		else {
			pVehicle->BurstTyre(ScriptParams[1], true);
		}
		return 0;
	}
	case COMMAND_IS_CHAR_OBJ_NO_OBJ:
	{
		CollectParameters(&m_nIp, 1);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		UpdateCompareFlag(pPed->m_prevObjective == OBJECTIVE_NONE && pPed->m_objective == OBJECTIVE_NONE);
		return 0;
	}
	case COMMAND_IS_PLAYER_WEARING:
	{
		CollectParameters(&m_nIp, 1);
		char key[12];
		CTheScripts::ReadTextLabelFromScript(&m_nIp, key);
		m_nIp += KEY_LENGTH_IN_SCRIPT;
		for (int i = 0; i < KEY_LENGTH_IN_SCRIPT; i++)
			key[i] = tolower(key[i]);
		CPed* pPed = CWorld::Players[ScriptParams[0]].m_pPed;
		script_assert(pPed);
		UpdateCompareFlag(strcmp(key, CModelInfo::GetModelInfo(pPed->GetModelIndex())->GetName()) == 0);
		return 0;
	}
	case COMMAND_SET_PLAYER_CAN_DO_DRIVE_BY:
	{
		CollectParameters(&m_nIp, 2);
		CPlayerInfo* pPlayerInfo = &CWorld::Players[ScriptParams[0]];
		pPlayerInfo->m_bDriveByAllowed = ScriptParams[1];
		return 0;
	}
	case COMMAND_SET_CHAR_OBJ_SPRINT_TO_COORD:
	{
		CollectParameters(&m_nIp, 3);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		CVector pos;
		pos.x = *(float*)&ScriptParams[1];
		pos.y = *(float*)&ScriptParams[2];
		pos.z = CWorld::FindGroundZForCoord(pos.x, pos.y);
		pPed->bScriptObjectiveCompleted = false;
		pPed->SetObjective(OBJECTIVE_SPRINT_TO_AREA, pos);
		return 0;
	}
	case COMMAND_CREATE_SWAT_ROPE:
	{
		CollectParameters(&m_nIp, 3);
		CRopes::CreateRopeWithSwatComingDown(*(CVector*)&ScriptParams[0]);
		return 0;
	}
	//case COMMAND_SET_FIRST_PERSON_CONTROL_CAMERA:
	//case COMMAND_GET_NEAREST_TYRE_TO_POINT:
	case COMMAND_SET_CAR_MODEL_COMPONENTS:
	{
		CollectParameters(&m_nIp, 3);
		CVehicleModelInfo::SetComponentsToUse(ScriptParams[1], ScriptParams[2]);
		return 0;
	}
	case COMMAND_SWITCH_LIFT_CAMERA:
	{
		CollectParameters(&m_nIp, 1);
		CSpecialFX::bLiftCam = ScriptParams[0] != 0;
		return 0;
	}
	case COMMAND_CLOSE_ALL_CAR_DOORS:
	{
		CollectParameters(&m_nIp, 1);
		CAutomobile* pCar = (CAutomobile*)CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		script_assert(pCar&& pCar->IsCar());
		pCar->CloseAllDoors();
		return 0;
	}
	case COMMAND_GET_DISTANCE_BETWEEN_COORDS_2D:
	{
		CollectParameters(&m_nIp, 4);
		*(float*)&ScriptParams[0] = (*(CVector2D*)&ScriptParams[0] - *(CVector2D*)&ScriptParams[2]).Magnitude();
		StoreParameters(&m_nIp, 1);
		return 0;
	}
	case COMMAND_GET_DISTANCE_BETWEEN_COORDS_3D:
	{
		CollectParameters(&m_nIp, 6);
		*(float*)&ScriptParams[0] = (*(CVector*)&ScriptParams[0] - *(CVector*)&ScriptParams[3]).Magnitude();
		StoreParameters(&m_nIp, 1);
		return 0;
	}
	case COMMAND_POP_CAR_BOOT_USING_PHYSICS:
	{
		CollectParameters(&m_nIp, 1);
		CAutomobile* pCar = (CAutomobile*)CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		script_assert(pCar && pCar->IsCar());
		pCar->PopBootUsingPhysics();
		return 0;
	}
	//case COMMAND_SET_FIRST_PERSON_WEAPON_CAMERA:
	case COMMAND_IS_CHAR_LEAVING_VEHICLE_TO_DIE:
	{
		CollectParameters(&m_nIp, 1);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		UpdateCompareFlag(pPed->m_objective == OBJECTIVE_LEAVE_CAR_AND_DIE);
		return 0;
	}
	case COMMAND_SORT_OUT_OBJECT_COLLISION_WITH_CAR:
	{
		CollectParameters(&m_nIp, 2);
		CObject* pObject = CPools::GetObjectPool()->GetAt(ScriptParams[0]);
		script_assert(pObject);
		pObject->m_pCollidingEntity = CPools::GetVehiclePool()->GetAt(ScriptParams[1]);
		return 0;
	}
	//case COMMAND_GET_MAX_WANTED_LEVEL:
	case COMMAND_IS_CHAR_WANDER_PATH_CLEAR:
	{
		CollectParameters(&m_nIp, 5);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		UpdateCompareFlag(CWorld::IsWanderPathClear(pPed->GetPosition(), *(CVector*)&ScriptParams[0], *(float*)&ScriptParams[3], 4));
		return 0;
	}
	//case COMMAND_PRINT_HELP_WITH_NUMBER:
	case COMMAND_PRINT_HELP_FOREVER:
	{
		wchar* text = CTheScripts::GetTextByKeyFromScript(&m_nIp);
		CHud::SetHelpMessage(text, false, true);
		return 0;
	}
	//case COMMAND_PRINT_HELP_FOREVER_WITH_NUMBER:
	default:
		script_assert(0);
	}
	return -1;
}

int8 CRunningScript::ProcessCommands1300To1399(int32 command)
{
	switch (command) {
	case COMMAND_SET_CHAR_CAN_BE_DAMAGED_BY_MEMBERS_OF_GANG:
	{
		CollectParameters(&m_nIp, 3);
		CPed* pTarget = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pTarget);
		uint8 flag = 1 << (uint8)ScriptParams[1];
		if (ScriptParams[2])
			pTarget->m_gangFlags |= flag;
		else
			pTarget->m_gangFlags &= ~flag;

		return 0;
	}
	case COMMAND_LOAD_AND_LAUNCH_MISSION_EXCLUSIVE:
		return 0;
	//case COMMAND_IS_MISSION_AUDIO_PLAYING:
	case COMMAND_CREATE_LOCKED_PROPERTY_PICKUP:
	{
		CollectParameters(&m_nIp, 3);
		CVector pos = *(CVector*)&ScriptParams[0];
		if (pos.z <= MAP_Z_LOW_LIMIT)
			pos.z = CWorld::FindGroundZForCoord(pos.x, pos.y) + PICKUP_PLACEMENT_OFFSET;
		char key[KEY_LENGTH_IN_SCRIPT];
		CTheScripts::ReadTextLabelFromScript(&m_nIp, key);
		m_nIp += KEY_LENGTH_IN_SCRIPT;
		// TheText.Get(key);
		CPickups::GetActualPickupIndex(CollectNextParameterWithoutIncreasingPC(m_nIp));
		ScriptParams[0] = CPickups::GenerateNewOne(pos, MI_PICKUP_PROPERTY, PICKUP_PROPERTY_LOCKED, 0, 0, false, key);
		StoreParameters(&m_nIp, 1);
		return 0;
	}
	case COMMAND_CREATE_FORSALE_PROPERTY_PICKUP:
	{
		CollectParameters(&m_nIp, 4);
		CVector pos = *(CVector*)&ScriptParams[0];
		if (pos.z <= MAP_Z_LOW_LIMIT)
			pos.z = CWorld::FindGroundZForCoord(pos.x, pos.y) + PICKUP_PLACEMENT_OFFSET;
		char key[KEY_LENGTH_IN_SCRIPT];
		CTheScripts::ReadTextLabelFromScript(&m_nIp, key);
		m_nIp += KEY_LENGTH_IN_SCRIPT;
		// TheText.Get(key);
		CPickups::GetActualPickupIndex(CollectNextParameterWithoutIncreasingPC(m_nIp));
		ScriptParams[0] = CPickups::GenerateNewOne(pos, MI_PICKUP_PROPERTY_FORSALE, PICKUP_PROPERTY_FORSALE, ScriptParams[3], 0, false, key);
		StoreParameters(&m_nIp, 1);
		return 0;
	}
	case COMMAND_FREEZE_CAR_POSITION:
	{
		CollectParameters(&m_nIp, 2);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		script_assert(pVehicle);
		pVehicle->bIsFrozen = ScriptParams[1];
		return 0;
	}
	case COMMAND_HAS_CHAR_BEEN_DAMAGED_BY_CHAR:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		CPed* pTestedPed = CPools::GetPedPool()->GetAt(ScriptParams[1]);
		bool result = false;
		if (pPed) {
			if (pPed->m_lastDamEntity) {
				if (pPed->m_lastDamEntity == pTestedPed)
					result = true;
				if (pTestedPed->bInVehicle && pPed->m_lastDamEntity == pTestedPed->m_pMyVehicle)
					result = true;
			}
		}else
			debug("HAS_CHAR_BEEN_DAMAGED_BY_CHAR - First character doesn't exist\n");
		UpdateCompareFlag(result);
		return 0;
	}
	//case COMMAND_HAS_CHAR_BEEN_DAMAGED_BY_CAR:
	//case COMMAND_HAS_CAR_BEEN_DAMAGED_BY_CHAR:
	//case COMMAND_HAS_CAR_BEEN_DAMAGED_BY_CAR:
	//case COMMAND_GET_RADIO_CHANNEL:
	//case COMMAND_DISPLAY_TEXT_WITH_3_NUMBERS:
	//case COMMAND_IS_CAR_DROWNING_IN_WATER:
	case COMMAND_IS_CHAR_DROWNING_IN_WATER:
	{
		CollectParameters(&m_nIp, 1);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		UpdateCompareFlag(pPed && pPed->bIsDrowning);
		return 0;
	}
	case COMMAND_DISABLE_CUTSCENE_SHADOWS:
	{
		CCutsceneMgr::DisableCutsceneShadows();
		return 0;
	}
	case COMMAND_HAS_GLASS_BEEN_SHATTERED_NEARBY:
	{
		CollectParameters(&m_nIp, 3);
		
		bool shattered = false;
		if ( CGlass::HasGlassBeenShatteredAtCoors(*(float*)&ScriptParams[0], *(float*)&ScriptParams[1], *(float*)&ScriptParams[2]) )
			shattered = true;
		
		UpdateCompareFlag(shattered);
		return 0;
	}
	case COMMAND_ATTACH_CUTSCENE_OBJECT_TO_BONE:
	{
		CollectParameters(&m_nIp, 3);
		CCutsceneMgr::AttachObjectToBone(CPools::GetObjectPool()->GetAt(ScriptParams[0]), CPools::GetObjectPool()->GetAt(ScriptParams[1]), ScriptParams[2]);
		return 0;
	}
	case COMMAND_ATTACH_CUTSCENE_OBJECT_TO_COMPONENT:
	{
		CollectParameters(&m_nIp, 2);
		CObject *obj1 = CPools::GetObjectPool()->GetAt(ScriptParams[0]);
		CObject *obj2 = CPools::GetObjectPool()->GetAt(ScriptParams[1]);

		char key[KEY_LENGTH_IN_SCRIPT];
		CTheScripts::ReadTextLabelFromScript(&m_nIp, key);
		m_nIp += KEY_LENGTH_IN_SCRIPT;

		CCutsceneMgr::AttachObjectToFrame(obj1, obj2, key);
		return 0;
	}
	case COMMAND_SET_CHAR_STAY_IN_CAR_WHEN_JACKED:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		pPed->bStayInCarOnJack = ScriptParams[1];
		return 0;
	}
	//case COMMAND_IS_MISSION_AUDIO_LOADING:
	case COMMAND_ADD_MONEY_SPENT_ON_WEAPONS:
		CollectParameters(&m_nIp, 1);
		CStats::MoneySpentOnWeapons(ScriptParams[0]);
		return 0;
	case COMMAND_ADD_MONEY_SPENT_ON_PROPERTY:
		CollectParameters(&m_nIp, 1);
		CStats::MoneySpentOnProperty(ScriptParams[0]);
		return 0;
	//case COMMAND_ADD_MONEY_SPENT_ON_AUTO_PAINTING:
	case COMMAND_SET_CHAR_ANSWERING_MOBILE:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		if (ScriptParams[1])
			pPed->SetAnswerMobile();
		else
			pPed->ClearAnswerMobile();
		return 0;
	}
	case COMMAND_SET_PLAYER_DRUNKENNESS:
	{
		CollectParameters(&m_nIp, 2);
		CPlayerInfo* pPlayerInfo = &CWorld::Players[ScriptParams[0]];
		pPlayerInfo->m_pPed->m_nDrunkenness = ScriptParams[1];
		pPlayerInfo->m_pPed->m_nFadeDrunkenness = 0;
		if (pPlayerInfo->m_pPed->m_nDrunkenness == 0)
			CMBlur::ClearDrunkBlur();
		return 0;
	}
	//case COMMAND_GET_PLAYER_DRUNKENNESS:
	//case COMMAND_SET_PLAYER_DRUG_LEVEL:
	//case COMMAND_GET_PLAYER_DRUG_LEVEL:
	//case COMMAND_ADD_LOAN_SHARK_VISITS:
	case COMMAND_ADD_STORES_KNOCKED_OFF:
		CollectParameters(&m_nIp, 1);
		CStats::NumOfStoresKnockedOff(ScriptParams[0]);
		return 0;
	//case COMMAND_ADD_MOVIE_STUNTS:
	case COMMAND_ADD_NUMBER_OF_ASSASSINATIONS:
		CollectParameters(&m_nIp, 1);
		CStats::NumOfAssassinations(ScriptParams[0]);
		return 0;
	case COMMAND_ADD_PIZZAS_DELIVERED:
		CollectParameters(&m_nIp, 1);
		CStats::NumOfPizzasDelivered(ScriptParams[0]);
		return 0;
	//case COMMAND_ADD_GARBAGE_PICKUPS:
	case COMMAND_ADD_ICE_CREAMS_SOLD:
		CollectParameters(&m_nIp, 1);
		CStats::NumOfIceCreamSold(ScriptParams[0]);
		return 0;
	//case COMMAND_SET_TOP_SHOOTING_RANGE_SCORE:
	//case COMMAND_ADD_SHOOTING_RANGE_RANK:
	//case COMMAND_ADD_MONEY_SPENT_ON_GAMBLING:
	//case COMMAND_ADD_MONEY_WON_ON_GAMBLING:
	//case COMMAND_SET_LARGEST_GAMBLING_WIN:
	case COMMAND_SET_CHAR_IN_PLAYERS_GROUP_CAN_FIGHT:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		pPed->bDontFight = !ScriptParams[1];
		return 0;
	}
	case COMMAND_CLEAR_CHAR_WAIT_STATE:
	{
		CollectParameters(&m_nIp, 1);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		pPed->ClearWaitState();
		return 0;
	}
	case COMMAND_GET_RANDOM_CAR_OF_TYPE_IN_AREA_NO_SAVE:
	{
		CollectParameters(&m_nIp, 5);
		int handle = -1;
		uint32 i = CPools::GetVehiclePool()->GetSize();
		float infX = *(float*)&ScriptParams[0];
		float infY = *(float*)&ScriptParams[1];
		float supX = *(float*)&ScriptParams[2];
		float supY = *(float*)&ScriptParams[3];
		while (i--) {
			CVehicle* pVehicle = CPools::GetVehiclePool()->GetSlot(i);
			if (!pVehicle)
				continue;
			if (pVehicle->GetVehicleAppearance() != VEHICLE_APPEARANCE_CAR && pVehicle->GetVehicleAppearance() != VEHICLE_APPEARANCE_BIKE)
				continue;
			if (ScriptParams[4] != pVehicle->GetModelIndex() && ScriptParams[4] >= 0)
				continue;
			if (pVehicle->VehicleCreatedBy != RANDOM_VEHICLE)
				continue;
			if (!pVehicle->IsWithinArea(infX, infY, supX, supY))
				continue;
			handle = CPools::GetVehiclePool()->GetIndex(pVehicle);
		}
		ScriptParams[0] = handle;
		StoreParameters(&m_nIp, 1);
		return 0;
	}
	case COMMAND_SET_CAN_BURST_CAR_TYRES:
	{
		CollectParameters(&m_nIp, 2);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		script_assert(pVehicle);
		pVehicle->bTyresDontBurst = !ScriptParams[1];
		return 0;
	}
	case COMMAND_SET_PLAYER_AUTO_AIM:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CWorld::Players[ScriptParams[0]].m_pPed;
		script_assert(pPed);
		pPed->bDoomAim = ScriptParams[1];
		return 0;
	}
	case COMMAND_FIRE_HUNTER_GUN:
	{
		CollectParameters(&m_nIp, 1);
		CVehicle *pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		if (CTimer::GetTimeInMilliseconds() > pVehicle->m_nGunFiringTime + 150) {
			CWeapon gun(WEAPONTYPE_HELICANNON, 5000);
			CVector worldGunPos = (pVehicle->GetMatrix() * vecHunterGunPos) + (CTimer::GetTimeStep() * pVehicle->m_vecMoveSpeed);
			gun.FireInstantHit(pVehicle, &worldGunPos);
			gun.AddGunshell(pVehicle, worldGunPos, CVector2D(0.f, 0.1f), 0.025f);
			DMAudio.PlayOneShot(pVehicle->m_audioEntityId, SOUND_WEAPON_SHOT_FIRED, 0.f);
			pVehicle->m_nGunFiringTime = CTimer::GetTimeInMilliseconds();
		}
		return 0;
	}
	case COMMAND_SET_PROPERTY_AS_OWNED:
		CollectParameters(&m_nIp, 1);
		CStats::AddPropertyAsOwned(ScriptParams[0]);
		return 0;
	case COMMAND_ADD_BLOOD_RING_KILLS:
		CollectParameters(&m_nIp, 1);
		CStats::AddNumBloodRingKills(ScriptParams[0]);
		return 0;
	case COMMAND_SET_LONGEST_TIME_IN_BLOOD_RING:
		CollectParameters(&m_nIp, 1);
		CStats::LongestTimeInBloodRing(ScriptParams[0]);
		return 0;
	case COMMAND_REMOVE_EVERYTHING_FOR_HUGE_CUTSCENE:
	{
		CCutsceneMgr::RemoveEverythingFromTheWorldForTheBiggestFuckoffCutsceneEver();
		return 0;
	}
	case COMMAND_IS_PLAYER_TOUCHING_VEHICLE:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CWorld::Players[ScriptParams[0]].m_pPed;
		script_assert(pPed);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[1]);
		CPhysical* pTestedEntity = pPed;
		if (pPed->bInVehicle && pPed->m_pMyVehicle)
			pTestedEntity = pPed->m_pMyVehicle;
		UpdateCompareFlag(pTestedEntity->GetHasCollidedWith(pVehicle));
		return 0;
	}
	//case COMMAND_IS_CHAR_TOUCHING_VEHICLE:
	case COMMAND_CHECK_FOR_PED_MODEL_AROUND_PLAYER:
	{
		CollectParameters(&m_nIp, 6);
		CVector d1 = CWorld::Players[ScriptParams[0]].GetPos() - *(CVector*)&ScriptParams[1];
		CVector d2 = CWorld::Players[ScriptParams[0]].GetPos() + *(CVector*)&ScriptParams[1];
		int i = CPools::GetPedPool()->GetSize();
		bool result = false;
		while (i--) {
			CPed* pPed = CPools::GetPedPool()->GetSlot(i);
			if (!pPed)
				continue;
			if (ScriptParams[4] != pPed->GetModelIndex() && ScriptParams[5] != pPed->GetModelIndex())
				continue;
			if (pPed->IsWithinArea(d1.x, d1.y, d1.z, d2.x, d2.y, d2.z))
				result = true;
		}
		UpdateCompareFlag(result);
		return 0;
	}
	case COMMAND_CLEAR_CHAR_FOLLOW_PATH:
	{
		CollectParameters(&m_nIp, 1);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		if (pPed->GetPedState() == PED_FOLLOW_PATH) {
			pPed->RestorePreviousState();
			pPed->ClearFollowPath();
		}
		return 0;
	}
	case COMMAND_SET_CHAR_CAN_BE_SHOT_IN_VEHICLE:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		pPed->bCanBeShotInVehicle = ScriptParams[1];
		return 0;
	}
	case COMMAND_ATTACH_CUTSCENE_OBJECT_TO_VEHICLE:
	{
		CollectParameters(&m_nIp, 2);
		CCutsceneMgr::AttachObjectToParent(CPools::GetObjectPool()->GetAt(ScriptParams[0]), CPools::GetVehiclePool()->GetAt(ScriptParams[1]));
		return 0;
	}
	case COMMAND_LOAD_MISSION_TEXT:
	{
		char key[8];
		CTheScripts::ReadTextLabelFromScript(&m_nIp, key);
		m_nIp += KEY_LENGTH_IN_SCRIPT;
		TheText.LoadMissionText(key);
		return 0;
	}
	case COMMAND_SET_TONIGHTS_EVENT:
	{
		CollectParameters(&m_nIp, 1);
		CScrollBar::TonightsEvent = ScriptParams[0];
		return 0;
	}
	case COMMAND_CLEAR_CHAR_LAST_DAMAGE_ENTITY:
	{
		CollectParameters(&m_nIp, 1);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		if (pPed)
			pPed->m_lastDamEntity = nil;
		else
			debug("CLEAR_CHAR_LAST_DAMAGE_ENTITY - Character doesn't exist\n");
		return 0;
	}
	//case COMMAND_CLEAR_CAR_LAST_DAMAGE_ENTITY:
	case COMMAND_FREEZE_OBJECT_POSITION:
	{
		CollectParameters(&m_nIp, 2);
		CObject* pObject = CPools::GetObjectPool()->GetAt(ScriptParams[0]);
		script_assert(pObject);
		pObject->bIsFrozen = ScriptParams[1];
		return 0;
	}
	case COMMAND_SET_PLAYER_HAS_MET_DEBBIE_HARRY:
	{
		CollectParameters(&m_nIp, 1);
		CTheScripts::bPlayerHasMetDebbieHarry = ScriptParams[0];
		return 0;
	}
	case COMMAND_SET_RIOT_INTENSITY:
	{
		CollectParameters(&m_nIp, 1);
		CTheScripts::RiotIntensity = ScriptParams[0];
		return 0;
	}
	//case COMMAND_IS_CAR_IN_ANGLED_AREA_2D:
	//case COMMAND_IS_CAR_IN_ANGLED_AREA_3D:
	//case COMMAND_REMOVE_WEAPON_FROM_CHAR:
	case COMMAND_SET_UP_TAXI_SHORTCUT:
	{
		CollectParameters(&m_nIp, 8);
		CGameLogic::SetUpShortCut(
			*(CVector*)&ScriptParams[0], *(float*)&ScriptParams[3],
			*(CVector*)&ScriptParams[4], *(float*)&ScriptParams[7]);
		return 0;
	}
	case COMMAND_CLEAR_TAXI_SHORTCUT:
		CGameLogic::ClearShortCut();
		return 0;
	//case COMMAND_SET_CHAR_OBJ_GOTO_CAR_ON_FOOT:
	//case COMMAND_GET_CLOSEST_WATER_NODE:
	case COMMAND_ADD_PORN_LEAFLET_TO_RUBBISH:
		CollectParameters(&m_nIp, 1);
		CStats::PamphletMissionPassed = ScriptParams[0];
		return 0;
	case COMMAND_CREATE_CLOTHES_PICKUP:
	{
		CollectParameters(&m_nIp, 4);
		CVector pos = *(CVector*)&ScriptParams[0];
		if (pos.z <= MAP_Z_LOW_LIMIT)
			pos.z = CWorld::FindGroundZForCoord(pos.x, pos.y) + PICKUP_PLACEMENT_OFFSET;
		CPickups::GetActualPickupIndex(CollectNextParameterWithoutIncreasingPC(m_nIp));
		ScriptParams[0] = CPickups::GenerateNewOne(pos, MI_PICKUP_CLOTHES, PICKUP_ON_STREET, ScriptParams[3]);
		StoreParameters(&m_nIp, 1);
		return 0;
	}
	//case COMMAND_CHANGE_BLIP_THRESHOLD:
	case COMMAND_MAKE_PLAYER_FIRE_PROOF:
	{
		CollectParameters(&m_nIp, 2);
		CPlayerInfo* pPlayerInfo = &CWorld::Players[ScriptParams[0]];
		pPlayerInfo->m_bFireproof = ScriptParams[1];
		return 0;
	}
	case COMMAND_INCREASE_PLAYER_MAX_HEALTH:
	{
		CollectParameters(&m_nIp, 2);
		CPlayerInfo* pPlayerInfo = &CWorld::Players[ScriptParams[0]];
		pPlayerInfo->m_nMaxHealth += ScriptParams[1];
		pPlayerInfo->m_pPed->m_fHealth = pPlayerInfo->m_nMaxHealth;
		return 0;
	}
	case COMMAND_INCREASE_PLAYER_MAX_ARMOUR:
	{
		CollectParameters(&m_nIp, 2);
		CPlayerInfo* pPlayerInfo = &CWorld::Players[ScriptParams[0]];
		pPlayerInfo->m_nMaxArmour += ScriptParams[1];
		pPlayerInfo->m_pPed->m_fArmour = pPlayerInfo->m_nMaxArmour;
		return 0;
	}
	case COMMAND_CREATE_RANDOM_CHAR_AS_DRIVER:
	{
		CollectParameters(&m_nIp, 1);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		CPed* pPed = CPopulation::AddPedInCar(pVehicle, true);
		pPed->CharCreatedBy = MISSION_CHAR;
		pPed->bRespondsToThreats = false;
		pPed->bAllowMedicsToReviveMe = false;
		pPed->bIsPlayerFriend = false;
		if (pVehicle->bIsBus)
			pPed->bRenderPedInCar = false;
		pPed->SetPosition(pVehicle->GetPosition());
		pPed->SetOrientation(0.0f, 0.0f, 0.0f);
		pPed->SetPedState(PED_DRIVING);
		pPed->m_pMyVehicle = pVehicle;
		pPed->m_pMyVehicle->RegisterReference((CEntity**)&pPed->m_pMyVehicle);
		pVehicle->pDriver = pPed;
		pVehicle->pDriver->RegisterReference((CEntity**)&pVehicle->pDriver);
		pPed->bInVehicle = true;
		pVehicle->SetStatus(STATUS_PHYSICS);
		if (pVehicle->m_vehType == VEHICLE_TYPE_BOAT)
			pVehicle->AutoPilot.m_nCarMission = MISSION_CRUISE;
		pVehicle->bEngineOn = true;
		pVehicle->m_nZoneLevel = CTheZones::GetLevelFromPosition(&pVehicle->GetPosition());
		CPopulation::ms_nTotalMissionPeds++;
		ScriptParams[0] = CPools::GetPedPool()->GetIndex(pPed);
		StoreParameters(&m_nIp, 1);
		if (m_bIsMissionScript)
			CTheScripts::MissionCleanup.AddEntityToList(ScriptParams[0], CLEANUP_CHAR);
		return 0;
	}
	case COMMAND_CREATE_RANDOM_CHAR_AS_PASSENGER:
	{
		CollectParameters(&m_nIp, 2);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		CPed* pPed = CPopulation::AddPedInCar(pVehicle, false);
		pPed->CharCreatedBy = MISSION_CHAR;
		pPed->bRespondsToThreats = false;
		pPed->bAllowMedicsToReviveMe = false;
		pPed->bIsPlayerFriend = false;
		if (pVehicle->bIsBus)
			pPed->bRenderPedInCar = false;
		pPed->SetPosition(pVehicle->GetPosition());
		pPed->SetOrientation(0.0f, 0.0f, 0.0f);
		CPopulation::ms_nTotalMissionPeds++;
		pVehicle->m_nZoneLevel = CTheZones::GetLevelFromPosition(&pVehicle->GetPosition());
		if (ScriptParams[1] >= 0)
			pVehicle->AddPassenger(pPed, ScriptParams[1]);
		else
			pVehicle->AddPassenger(pPed);

		pPed->m_pMyVehicle = pVehicle;
		pPed->m_pMyVehicle->RegisterReference((CEntity**)&pPed->m_pMyVehicle);
		pPed->bInVehicle = true;
		pPed->SetPedState(PED_DRIVING);
		ScriptParams[0] = CPools::GetPedPool()->GetIndex(pPed);
		StoreParameters(&m_nIp, 1);
		if (m_bIsMissionScript)
			CTheScripts::MissionCleanup.AddEntityToList(ScriptParams[0], CLEANUP_CHAR);
		return 0;
	}
	case COMMAND_SET_CHAR_IGNORE_THREATS_BEHIND_OBJECTS:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		pPed->bIgnoreThreatsBehindObjects = ScriptParams[1];
		return 0;
	}
	case COMMAND_ENSURE_PLAYER_HAS_DRIVE_BY_WEAPON:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CWorld::Players[ScriptParams[0]].m_pPed;
		script_assert(pPed);
		if (pPed->bInVehicle) {
			if (pPed->GetWeapon(WEAPONSLOT_SUBMACHINEGUN).m_eWeaponType) {
				if (pPed->GetWeapon(WEAPONSLOT_SUBMACHINEGUN).m_nAmmoTotal < ScriptParams[1])
					pPed->SetAmmo(pPed->GetWeapon(WEAPONSLOT_SUBMACHINEGUN).m_eWeaponType, ScriptParams[1]);
			}
			else {
				pPed->GiveWeapon(WEAPONTYPE_UZI, ScriptParams[1], true);
				if (pPed->m_storedWeapon == WEAPONTYPE_UNIDENTIFIED)
					pPed->m_storedWeapon = pPed->GetWeapon()->m_eWeaponType;
				pPed->SetCurrentWeapon(WEAPONTYPE_UZI);
			}
		}
		return 0;
	}
	case COMMAND_MAKE_HELI_COME_CRASHING_DOWN:
	{
		CollectParameters(&m_nIp, 1);
		CAutomobile* pHeli = (CAutomobile*)CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		script_assert(pHeli && pHeli->IsCar() && pHeli->IsRealHeli());
		pHeli->bHeliDestroyed = true;
		return 0;
	}
	case COMMAND_ADD_EXPLOSION_NO_SOUND:
	{
		CollectParameters(&m_nIp, 4);
		CExplosion::AddExplosion(nil, nil, (eExplosionType)ScriptParams[3], *(CVector*)&ScriptParams[0], 0, false);
		return 0;
	}
	case COMMAND_SET_OBJECT_AREA_VISIBLE:
	{
		CollectParameters(&m_nIp, 2);
		CObject* pObject = CPools::GetObjectPool()->GetAt(ScriptParams[0]);
		script_assert(pObject);
		pObject->m_area = ScriptParams[1];
		return 0;
	}
	//case COMMAND_WAS_VEHICLE_EVER_POLICE:
	case COMMAND_SET_CHAR_NEVER_TARGETTED:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		pPed->bNeverEverTargetThisPed = ScriptParams[1];
		return 0;
	}
	case COMMAND_LOAD_UNCOMPRESSED_ANIM:
	{
		char key[KEY_LENGTH_IN_SCRIPT];
		CTheScripts::ReadTextLabelFromScript(&m_nIp, key);
		m_nIp += KEY_LENGTH_IN_SCRIPT;
		CCutsceneMgr::LoadAnimationUncompressed(key);
		return 0;
	}
	case COMMAND_WAS_CUTSCENE_SKIPPED:
	{
		UpdateCompareFlag(CCutsceneMgr::WasCutsceneSkipped());
		return 0;
	}
	case COMMAND_SET_CHAR_CROUCH_WHEN_THREATENED:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		pPed->bCrouchWhenScared = true;
		return 0;
	}
	case COMMAND_IS_CHAR_IN_ANY_POLICE_VEHICLE:
	{
		CollectParameters(&m_nIp, 1);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		UpdateCompareFlag(pPed->bInVehicle && pPed->m_pMyVehicle &&
			pPed->m_pMyVehicle->IsLawEnforcementVehicle() &&
			pPed->m_pMyVehicle->GetModelIndex() != MI_PREDATOR);
		return 0;
	}
	case COMMAND_DOES_CHAR_EXIST:
		CollectParameters(&m_nIp, 1);
		UpdateCompareFlag(CPools::GetPedPool()->GetAt(ScriptParams[0]) != 0);
		return 0;
	//case COMMAND_DOES_VEHICLE_EXIST:
	//case COMMAND_ADD_SHORT_RANGE_BLIP_FOR_CONTACT_POINT:
	case COMMAND_ADD_SHORT_RANGE_SPRITE_BLIP_FOR_CONTACT_POINT:
	{
		CollectParameters(&m_nIp, 4);
		CVector pos = *(CVector*)&ScriptParams[0];
		if (pos.z <= MAP_Z_LOW_LIMIT)
			pos.z = CWorld::FindGroundZForCoord(pos.x, pos.y);
		CRadar::GetActualBlipArrayIndex(CollectNextParameterWithoutIncreasingPC(m_nIp));
		int id = CRadar::SetShortRangeCoordBlip(BLIP_COORD, pos, 2, BLIP_DISPLAY_BOTH);
		CRadar::SetBlipSprite(id, ScriptParams[3]);
		ScriptParams[0] = id;
		StoreParameters(&m_nIp, 1);
		return 0;
	}
	case COMMAND_IS_CHAR_STUCK:
	{
		CollectParameters(&m_nIp, 1);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		UpdateCompareFlag(pPed->m_nWaitState == WAITSTATE_STUCK);
		return 0;
	}
	case COMMAND_SET_ALL_TAXIS_HAVE_NITRO:
	{
		CollectParameters(&m_nIp, 1);
		CVehicle::bAllTaxisHaveNitro = ScriptParams[0] != 0;
		return 0;
	}
	case COMMAND_SET_CHAR_STOP_SHOOT_DONT_SEEK_ENTITY:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		if (ScriptParams[1]) {
			pPed->bKindaStayInSamePlace = true;
			pPed->bStopAndShoot = true;
		}
		else {
			pPed->bKindaStayInSamePlace = false;
			pPed->bStopAndShoot = false;
		}
		pPed->m_nLastPedState = PED_NONE;
		return 0;
	}
	case COMMAND_FREEZE_CAR_POSITION_AND_DONT_LOAD_COLLISION:
	{
		CollectParameters(&m_nIp, 2);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		script_assert(pVehicle);
		if (ScriptParams[1]) {
			pVehicle->bIsFrozen = true;
			pVehicle->bInfiniteMass = true;
			if (m_bIsMissionScript) {
				CWorld::Remove(pVehicle);
				pVehicle->bIsStaticWaitingForCollision = true;
				CWorld::Add(pVehicle);
			}
		}
		else {
			pVehicle->bIsFrozen = false;
			pVehicle->bInfiniteMass = false;
		}
		return 0;
	}
	//case COMMAND_FREEZE_CHAR_POSITION_AND_DONT_LOAD_COLLISION:
	//case COMMAND_FREEZE_OBJECT_POSITION_AND_DONT_LOAD_COLLISION:
	//case COMMAND_SET_FADE_AND_JUMPCUT_AFTER_RC_EXPLOSION:
	default:
		script_assert(0);
	}
	return -1;
}
