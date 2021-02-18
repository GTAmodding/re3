#include "common.h"

#include "Script.h"
#include "ScriptCommands.h"

#include "Bridge.h"
#include "CarCtrl.h"
#include "Camera.h"
#include "CutsceneMgr.h"
#include "Ferry.h"
#include "Garages.h"
#include "GameLogic.h"
#include "Hud.h"
#include "Messages.h"
#include "Object.h"
#include "Pad.h"
#include "Ped.h"
#include "Pools.h"
#include "Remote.h"
#include "SpecialFX.h"
#include "Stats.h"
#include "Vehicle.h"
#include "World.h"

// LCS: file done except TODOs (also check commented out strings)

int8 CRunningScript::ProcessCommands1500To1599(int32 command)
{
	switch (command) {
	case COMMAND_DISABLE_FERRY_PATH:
	{
		CollectParameters(&m_nIp, 1);
		CFerry::DissableFerryPath(GET_INTEGER_PARAM(0));
		return 0;
	}
	case COMMAND_ENABLE_FERRY_PATH:
	{
		CollectParameters(&m_nIp, 1);
		CFerry::EnableFerryPath(GET_INTEGER_PARAM(0));
		return 0;
	}
	case COMMAND_GET_CLOSEST_DOCKED_FERRY:
	{
		CollectParameters(&m_nIp, 2);
		CFerry* pFerry = CFerry::GetClosestFerry(GET_FLOAT_PARAM(0), GET_FLOAT_PARAM(1));
		int id = -1;
		if (pFerry && pFerry->IsDocked())
			id = pFerry->m_nFerryId;
		SET_INTEGER_PARAM(0, id);
		StoreParameters(&m_nIp, 1);
		return 0;
	}
	case COMMAND_OPEN_FERRY_DOOR:
	{
		CollectParameters(&m_nIp, 1);
		CFerry* pFerry = CFerry::GetFerry(GET_INTEGER_PARAM(0));
		script_assert(pFerry);
		pFerry->OpenDoor();
		return 0;
	}
	case COMMAND_CLOSE_FERRY_DOOR:
	{
		CollectParameters(&m_nIp, 1);
		CFerry* pFerry = CFerry::GetFerry(GET_INTEGER_PARAM(0));
		script_assert(pFerry);
		pFerry->CloseDoor();
		return 0;
	}
	case COMMAND_IS_FERRY_DOOR_OPEN:
	{
		CollectParameters(&m_nIp, 1);
		CFerry* pFerry = CFerry::GetFerry(GET_INTEGER_PARAM(0));
		script_assert(pFerry);
		UpdateCompareFlag(pFerry->IsDoorOpen());
		return 0;
	}
	case COMMAND_IS_FERRY_DOOR_CLOSED:
	{
		CollectParameters(&m_nIp, 1);
		CFerry* pFerry = CFerry::GetFerry(GET_INTEGER_PARAM(0));
		script_assert(pFerry);
		UpdateCompareFlag(pFerry->IsDoorClosed());
		return 0;
	}
	case COMMAND_SKIP_FERRY_TO_NEXT_DOCK:
	{
		CollectParameters(&m_nIp, 1);
		CFerry* pFerry = CFerry::GetFerry(GET_INTEGER_PARAM(0));
		script_assert(pFerry);
		pFerry->SkipFerryToNextDock();
		return 0;
	}
	case COMMAND_SET_CHAR_DROPS_WEAPONS_ON_DEATH:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CPools::GetPedPool()->GetAt(GET_INTEGER_PARAM(0));
		script_assert(pPed);
		pPed->bDropsWeaponsOnDeath = (GET_INTEGER_PARAM(1) != 0);
		return 0;
	}
	case COMMAND_IS_CHAR_CROUCHING:
	{
		CollectParameters(&m_nIp, 1);
		CPed* pPed = CPools::GetPedPool()->GetAt(GET_INTEGER_PARAM(0));
		script_assert(pPed);
		UpdateCompareFlag(pPed->bIsDucking);
		return 0;
	}
	case COMMAND_GET_FERRY_BOARDING_SPACE:
	{
		CollectParameters(&m_nIp, 4);
		CFerry* pFerry = CFerry::GetFerry(GET_INTEGER_PARAM(0));
		script_assert(pFerry);
		CVector space = pFerry->GetBoardingSpace((CFerry::eSpaceUse)GET_INTEGER_PARAM(1), (CFerry::eSpaceStyle)GET_INTEGER_PARAM(2), GET_INTEGER_PARAM(3));
		SET_FLOAT_PARAM(0, space.x);
		SET_FLOAT_PARAM(1, space.y);
		StoreParameters(&m_nIp, 2);
		return 0;
	}
	case COMMAND_GET_FERRY_HEADING:
	{
		CollectParameters(&m_nIp, 1);
		CFerry* pFerry = CFerry::GetFerry(GET_INTEGER_PARAM(0));
		script_assert(pFerry);
		float fHeading = Atan2(-pFerry->GetForward().x, pFerry->GetForward().y);
		SET_FLOAT_PARAM(0, fHeading);
		StoreParameters(&m_nIp, 1);
		return 0;
	}
	case COMMAND_SET_FERRIES_DISABLED:
	{
		CollectParameters(&m_nIp, 2);
		CFerry::SetFerriesDisabled(GET_INTEGER_PARAM(1));
		return 0;
	}
	case COMMAND_COMPLETE_FERRY_DOOR_MOVEMENT:
	{
		CollectParameters(&m_nIp, 1);
		CFerry* pFerry = CFerry::GetFerry(GET_INTEGER_PARAM(0));
		script_assert(pFerry);
		pFerry->CompleteDorrMovement();
		return 0;
	}
	case COMMAND_OVERRIDE_CAR_REMOTE_CONTROL:
	{
		CollectParameters(&m_nIp, 2);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(GET_INTEGER_PARAM(0));
		script_assert(pVehicle);
		pVehicle->SetStatus(STATUS_PLAYER_REMOTE);
		CVehicle::bDisableRemoteDetonation = true;
		CWorld::Players[CWorld::PlayerInFocus].m_pRemoteVehicle = pVehicle;
		pVehicle->RegisterReference((CEntity**)&CWorld::Players[CWorld::PlayerInFocus].m_pRemoteVehicle);
		if (pVehicle->GetVehicleAppearance() == VEHICLE_APPEARANCE_HELI || pVehicle->GetVehicleAppearance() == VEHICLE_APPEARANCE_PLANE) {
			TheCamera.TakeControl(pVehicle, CCam::MODE_CAM_ON_A_STRING, GET_INTEGER_PARAM(1) ? INTERPOLATION : JUMP_CUT, CAMCONTROL_SCRIPT);
			TheCamera.SetZoomValueCamStringScript(0);
		}
		else {
			TheCamera.TakeControl(pVehicle, CCam::MODE_1STPERSON, GET_INTEGER_PARAM(1) ? INTERPOLATION : JUMP_CUT, CAMCONTROL_SCRIPT);
			script_assert(pVehicle->IsCar());
			((CAutomobile*)pVehicle)->Damage.m_bSmashedDoorDoesntClose = true;
		}
		if (m_bIsMissionScript)
			CTheScripts::MissionCleanUp.RemoveEntityFromList(GET_INTEGER_PARAM(0), CLEANUP_CAR);
		if (FindPlayerVehicle())
			FindPlayerVehicle()->bCanBeDamaged = false;
		return 0;
	}
	case COMMAND_CANCEL_REMOTE_MODE:
	{
		if (FindPlayerVehicle())
			FindPlayerVehicle()->bCanBeDamaged = true;
		CRemote::TakeRemoteControlledCarFromPlayer(false);
		CWorld::Players[CWorld::PlayerInFocus].field_D6 = false;
		CWorld::Players[CWorld::PlayerInFocus].m_pRemoteVehicle = nil;
		TheCamera.Restore();
		return 0;
	}
	case COMMAND_REGISTER_CAR_SOLD:
		// CStats::CarsSold++;
		return 0;
	case COMMAND_ADD_MONEY_MADE_WITH_CAR_SALES:
		CollectParameters(&m_nIp, 1);
		// CStats::MoneyMadeWithCarSales += GET_INTEGER_PARAM(0);
		return 0;
	case COMMAND_SET_BRIDGE_STATE:
	{
		CollectParameters(&m_nIp, 1);
#ifdef GTA_BRIDGE
		/*
		* 0 = locked
		* 1 = unlocked
		* 2 = operational
		*/
		switch (GET_INTEGER_PARAM(0)) {
		case 0: CBridge::ForceBridgeState(STATE_BRIDGE_LOCKED); break;
		case 1: CBridge::ForceBridgeState(STATE_BRIDGE_ALWAYS_UNLOCKED); break;
		case 2:
			if (CBridge::State == STATE_LIFT_PART_IS_DOWN || CBridge::State == STATE_BRIDGE_ALWAYS_UNLOCKED)
				CBridge::ForceBridgeState(STATE_LIFT_PART_ABOUT_TO_MOVE_UP);
			else
				CBridge::ForceBridgeState(STATE_LIFT_PART_MOVING_DOWN);
			break;
		default: script_assert(false);
		}
#endif
		return 0;
	}
	case COMMAND_SET_OBJECT_TURN_SPEED:
	{
		CollectParameters(&m_nIp, 4);
		CObject* pObject = CPools::GetObjectPool()->GetAt(GET_INTEGER_PARAM(0));
		script_assert(pObject);
		CVector vSpeed = GET_VECTOR_PARAM(1) / GAME_SPEED_TO_METERS_PER_SECOND;
		pObject->SetTurnSpeed(vSpeed.x, vSpeed.y, vSpeed.z);
		return 0;
	}
	case COMMAND_SET_OBJECT_MASS:
	{
		CollectParameters(&m_nIp, 4);
		CObject* pObject = CPools::GetObjectPool()->GetAt(GET_INTEGER_PARAM(0));
		script_assert(pObject);
		pObject->m_fMass = GET_FLOAT_PARAM(1);
		pObject->m_fTurnMass = GET_FLOAT_PARAM(2);
		pObject->m_fAirResistance = GET_FLOAT_PARAM(3);
		if (pObject->m_fMass < 99998.0f) {
			pObject->bInfiniteMass = false;
			pObject->m_phy_flagA08 = false;
			pObject->bAffectedByGravity = true;
		}
		else {
			pObject->bInfiniteMass = true;
			pObject->m_phy_flagA08 = true;
			pObject->bAffectedByGravity = false;
		}
		return 0;
	}
	case COMMAND_HAS_CUTSCENE_LOADED:
		UpdateCompareFlag(CCutsceneMgr::ms_cutsceneLoadStatus == CUTSCENE_LOADED);
		return 0;
	case COMMAND_SET_UNIQUE_JUMPS_FOUND:
		CollectParameters(&m_nIp, 1);
		CStats::NumberOfUniqueJumpsFound = GET_INTEGER_PARAM(0);
		return 0;
	case COMMAND_SET_HIDDEN_PACKAGES_COLLECTED:
		CollectParameters(&m_nIp, 1);
		CWorld::Players[CWorld::PlayerInFocus].m_nCollectedPackages = GET_INTEGER_PARAM(0);
		return 0;
	case COMMAND_REGISTER_BIKE_SOLD:
		// CStats::BikesSold++;
		return 0;
	case COMMAND_ADD_MONEY_MADE_WITH_BIKE_SALES:
		CollectParameters(&m_nIp, 1);
		// CStats::MoneyMadeWithBikeSales += GET_INTEGER_PARAM(0);
		return 0;
	case COMMAND_REGISTER_PACKAGE_SMUGGLED:
		// CStats::PackagesSmuggled++;
		return 0;
	case COMMAND_REGISTER_SMUGGLER_WASTED:
		// CStats::SmugglersWasted++;
		return 0;
	case COMMAND_REGISTER_FASTEST_SMUGGLING_TIME:
		CollectParameters(&m_nIp, 1);
		// CStats::RegisterFastestSmugglingTime(GET_INTEGER_PARAM(0));
		return 0;
	case COMMAND_SET_CHAR_DIVE_FROM_CAR:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CPools::GetPedPool()->GetAt(GET_INTEGER_PARAM(0));
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(GET_INTEGER_PARAM(1));
		script_assert(pPed);
		pPed->bRespondsToThreats = true;
		pPed->SetEvasiveDive(pVehicle, 1);
		return 0;
	}
	case COMMAND_WRECK_CAR:
	{
		CollectParameters(&m_nIp, 1);
		CAutomobile* pVehicle = (CAutomobile*)CPools::GetVehiclePool()->GetAt(GET_INTEGER_PARAM(0));
		script_assert(pVehicle);
		script_assert(pVehicle->IsCar());
		pVehicle->m_fHealth = 0.0f;
		pVehicle->SetStatus(STATUS_WRECKED);
		pVehicle->bRenderScorched = true;
		pVehicle->Damage.FuckCarCompletely();
		if (pVehicle->GetModelIndex() != MI_RCBANDIT) {
			pVehicle->SetBumperDamage(CAR_BUMP_FRONT, VEHBUMPER_FRONT);
			pVehicle->SetBumperDamage(CAR_BUMP_REAR, VEHBUMPER_REAR);
			pVehicle->SetDoorDamage(CAR_BONNET, DOOR_BONNET);
			pVehicle->SetDoorDamage(CAR_BOOT, DOOR_BOOT);
			pVehicle->SetDoorDamage(CAR_DOOR_LF, DOOR_FRONT_LEFT);
			pVehicle->SetDoorDamage(CAR_DOOR_RF, DOOR_FRONT_RIGHT);
			pVehicle->SetDoorDamage(CAR_DOOR_LR, DOOR_REAR_LEFT);
			pVehicle->SetDoorDamage(CAR_DOOR_RR, DOOR_REAR_RIGHT);
		}
		pVehicle->m_bombType = CARBOMB_NONE;
		pVehicle->bEngineOn = false;
		pVehicle->bLightsOn = false;
		pVehicle->m_fHealth = 0.0f;
		pVehicle->m_nBombTimer = 0;
		pVehicle->m_bSirenOrAlarm = false;
		return 0;
	}
	case COMMAND_ADD_MONEY_MADE_IN_COACH:
		CollectParameters(&m_nIp, 1);
		// CStats::MoneyMadeInCoach += GET_INTEGER_PARAM(0);
		return 0;
	case COMMAND_ADD_MONEY_MADE_COLLECTING_TRASH:
		CollectParameters(&m_nIp, 1);
		// CStats::MoneyMadeCollectingTrash += GET_INTEGER_PARAM(0);
		return 0;
	case COMMAND_REGISTER_HITMAN_KILLED:
		// CStats::HitmenKilled++;
		return 0;
	case COMMAND_REGISTER_GUARDIAN_ANGEL_MISSION_PASSED:
		// CStats::GaurdianAngelMissionsPassed++;
		return 0;
	case COMMAND_REGISTER_HIGHEST_GUARDIAN_ANGEL_JUSTICE_DISHED:
		CollectParameters(&m_nIp, 1);
		// CStats::RegisterHighestGaurdianAngelJusticeDished(GET_INTEGER_PARAM(0));
		return 0;
	case COMMAND_REGISTER_BEST_BANDIT_LAP_TIME:
		CollectParameters(&m_nIp, 2);
		// CStats::RegisterBestBanditLapTime(GET_INTEGER_PARAM(0), GET_INTEGER_PARAM(1));
		return 0;
	case COMMAND_REGISTER_BEST_BANDIT_POSITION:
		CollectParameters(&m_nIp, 2);
		// CStats::RegisterBestBanditPosition(GET_INTEGER_PARAM(0), GET_INTEGER_PARAM(1));
		return 0;
	case COMMAND_REGISTER_MOST_TIME_LEFT_TRAIN_RACE:
		CollectParameters(&m_nIp, 1);
		// CStats::RegisterMostTimeLeftTrainRace(GET_INTEGER_PARAM(0));
		return 0;
	case COMMAND_REGISTER_HIGHEST_TRAIN_CASH_EARNED:
		CollectParameters(&m_nIp, 1);
		// CStats::RegisterHighestTrainCashEarned(GET_INTEGER_PARAM(0));
		return 0;
	case COMMAND_REGISTER_FASTEST_HELI_RACE_TIME:
		// CStats::RegisterFastestHeliRaceTime(GET_INTEGER_PARAM(0));
		CollectParameters(&m_nIp, 1);
		return 0;
	case COMMAND_REGISTER_BEST_HELI_RACE_POSITION:
		// CStats::RegisterBestHeliRacePosition(GET_INTEGER_PARAM(0));
		CollectParameters(&m_nIp, 1);
		return 0;
	case COMMAND_REGISTER_OUTFIT_CHANGE:
		// CStats::NumberOutfitChanges++;
		return 0;
	case COMMAND_REGISTER_STREET_RACE_FASTEST_TIME:
		// CStats::RegisterStreetRaceFastestTime(GET_INTEGER_PARAM(0), GET_INTEGER_PARAM(1));
		CollectParameters(&m_nIp, 2);
		return 0;
	case COMMAND_REGISTER_STREET_RACE_FASTEST_LAP:
		CollectParameters(&m_nIp, 2);
		// CStats::RegisterStreetRaceFastestLap(GET_INTEGER_PARAM(0), GET_INTEGER_PARAM(1));
		return 0;
	case COMMAND_REGISTER_STREET_RACE_BEST_POSITION:
		CollectParameters(&m_nIp, 2);
		// CStats::RegisterStreetRaceBestPosition(GET_INTEGER_PARAM(0), GET_INTEGER_PARAM(1));
		return 0;
	case COMMAND_HAS_OBJECT_BEEN_DAMAGED_BY_WEAPON:
	{
		CollectParameters(&m_nIp, 2);
		CObject* pObject = CPools::GetObjectPool()->GetAt(GET_INTEGER_PARAM(0));
		bool result = false;
		if (!pObject) {
			printf("HAS_OBJECT_BEEN_DAMAGED_BY_WEAPON - Object doesn\'t exist\n");
		}
		else {
			if (GET_INTEGER_PARAM(1) == WEAPONTYPE_ANYMELEE || GET_INTEGER_PARAM(1) == WEAPONTYPE_ANYWEAPON)
				result = CheckDamagedWeaponType(pObject->m_nLastWeaponToDamage, GET_INTEGER_PARAM(1));
			else
				result = GET_INTEGER_PARAM(1) == pObject->m_nLastWeaponToDamage;
		}
		UpdateCompareFlag(result);
		return 0;
	}
	case COMMAND_CLEAR_OBJECT_LAST_WEAPON_DAMAGE:
	{
		CObject* pObject = CPools::GetObjectPool()->GetAt(GET_INTEGER_PARAM(0));
		if (!pObject)
			printf("CLEAR_OBJECT_LAST_WEAPON_DAMAGE - pObject doesn\'t exist");
		else
			pObject->m_nLastWeaponToDamage = -1;
		return 0;
	}
	case COMMAND_SET_CAR_TURN_SPEED:
	{
		CollectParameters(&m_nIp, 4);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(GET_INTEGER_PARAM(0));
		script_assert(pVehicle);
		CVector vSpeed = GET_VECTOR_PARAM(1) / GAME_SPEED_TO_METERS_PER_SECOND;
		pVehicle->SetTurnSpeed(vSpeed.x, vSpeed.y, vSpeed.z);
		return 0;
	}
	case COMMAND_SET_CAR_MOVE_SPEED:
	{
		CollectParameters(&m_nIp, 4);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(GET_INTEGER_PARAM(0));
		script_assert(pVehicle);
		CVector vSpeed = GET_VECTOR_PARAM(1) / GAME_SPEED_TO_METERS_PER_SECOND;
		pVehicle->SetMoveSpeed(vSpeed);
		return 0;
	}
	case COMMAND_SET_OBJECT_PROOFS:
	{
		CollectParameters(&m_nIp, 6);
		CObject* pObject = CPools::GetObjectPool()->GetAt(GET_INTEGER_PARAM(0));
		script_assert(pObject);
		pObject->bBulletProof = (GET_INTEGER_PARAM(1) != 0);
		pObject->bFireProof = (GET_INTEGER_PARAM(2) != 0);
		pObject->bExplosionProof = (GET_INTEGER_PARAM(3) != 0);
		pObject->bCollisionProof = (GET_INTEGER_PARAM(4) != 0);
		pObject->bMeleeProof = (GET_INTEGER_PARAM(5) != 0);
		return 0;
	}
	case COMMAND_GET_CAMERA_PED_ZOOM_INDICATOR:
		if (TheCamera.Cams[TheCamera.ActiveCam].Mode == CCam::MODE_FOLLOWPED)
			SET_INTEGER_PARAM(0, TheCamera.PedZoomIndicator);
		else
			SET_INTEGER_PARAM(0, -1);
		StoreParameters(&m_nIp, 1);
		return 0;
	case COMMAND_SET_CAMERA_PED_ZOOM_INDICATOR:
		CollectParameters(&m_nIp, 1);
		if (TheCamera.Cams[TheCamera.ActiveCam].Mode == CCam::MODE_FOLLOWPED)
			TheCamera.PedZoomIndicator = GET_INTEGER_PARAM(0);
		return 0;
	case COMMAND_GET_CAR_ORIENTATION:
	{
		CollectParameters(&m_nIp, 1);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(GET_INTEGER_PARAM(0));
		script_assert(pVehicle);
		SET_FLOAT_PARAM(1, LimitAngleOnCircle(RADTODEG(Asin(pVehicle->GetForward().z))));
		SET_FLOAT_PARAM(2, LimitAngleOnCircle(RADTODEG(Atan2(-pVehicle->GetForward().x, pVehicle->GetForward().y))));
		SET_FLOAT_PARAM(0, LimitAngleOnCircle(RADTODEG(Atan2(-pVehicle->GetRight().z, pVehicle->GetUp().z))));
		StoreParameters(&m_nIp, 3);
		return 0;
	}
	case COMMAND_SET_CAR_ORIENTATION:
	{
		CollectParameters(&m_nIp, 4);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(GET_INTEGER_PARAM(0));
		script_assert(pVehicle);
		pVehicle->SetOrientation(DEGTORAD(GET_FLOAT_PARAM(2)), DEGTORAD(GET_FLOAT_PARAM(1)), DEGTORAD(GET_FLOAT_PARAM(3)));
		return 0;
	}
	case COMMAND_IS_DEBUG_MENU_ON:
		// on PS2 it's something actual - TODO
		UpdateCompareFlag(false);
		return 0;
	case COMMAND_OPEN_VAN_BACK_DOORS:
	{
		CollectParameters(&m_nIp, 1);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(GET_INTEGER_PARAM(0));
		assert(pVehicle);
		pVehicle->ProcessOpenDoor(CAR_DOOR_RR, ANIM_STD_VAN_OPEN_DOOR_REAR_RHS, 1.0f);
		pVehicle->ProcessOpenDoor(CAR_DOOR_LR, ANIM_STD_VAN_OPEN_DOOR_REAR_LHS, 1.0f);
		return 0;
	}
	case COMMAND_GET_CHAR_THREAT_CHAR:
	{
		CollectParameters(&m_nIp, 1);
		CPed* pPed = CPools::GetPedPool()->GetAt(GET_INTEGER_PARAM(0));
		script_assert(pPed);
		SET_INTEGER_PARAM(0, 0);
		CEntity* pThreat = pPed->m_threatEntity;
		if (pThreat && pThreat->IsPed())
			SET_INTEGER_PARAM(0, CPools::GetPedPool()->GetIndex((CPed*)pThreat));
		StoreParameters(&m_nIp, 1);
		return 0;
	}
	case COMMAND_FREEZE_PED_ZOOM_SWITCH:
		CollectParameters(&m_nIp, 1);
		TheCamera.m_bFreezePedZoomSwitch = GET_INTEGER_PARAM(0);
		return 0;
	case COMMAND_SET_OBJECT_RENDERED_DAMAGED:
	{
		CollectParameters(&m_nIp, 1);
		CObject* pObject = CPools::GetObjectPool()->GetAt(GET_INTEGER_PARAM(0));
		script_assert(pObject);
		pObject->bRenderDamaged = true;
		return 0;
	}
	case COMMAND_GET_RANDOM_CAR_IN_AREA_NO_SAVE:
	{
		CollectParameters(&m_nIp, 5);
		int handle = -1;
		uint32 i = CPools::GetVehiclePool()->GetSize();
		float infX = GET_FLOAT_PARAM(0);
		float infY = GET_FLOAT_PARAM(1);
		float supX = GET_FLOAT_PARAM(2);
		float supY = GET_FLOAT_PARAM(3);
		while (i-- && handle == -1) {
			CVehicle* pVehicle = CPools::GetVehiclePool()->GetSlot(i);
			if (!pVehicle)
				continue;
			if (pVehicle->GetVehicleAppearance() != VEHICLE_APPEARANCE_CAR && pVehicle->GetVehicleAppearance() != VEHICLE_APPEARANCE_BIKE)
				continue;
#ifdef FIX_BUGS
			if (pVehicle->m_fHealth <= 0.0f)
#else
			if (pVehicle->m_fHealth == 0.0f)
#endif
				continue;
			if (pVehicle->GetModelIndex() != GET_INTEGER_PARAM(4) && GET_INTEGER_PARAM(4) >= 0)
				continue;
			if (pVehicle->VehicleCreatedBy != RANDOM_VEHICLE)
				continue;
			if (!pVehicle->IsWithinArea(infX, infY, supX, supY))
				continue;
			handle = CPools::GetVehiclePool()->GetIndex(pVehicle);
		}
		SET_INTEGER_PARAM(0, handle);
		StoreParameters(&m_nIp, 1);
		return 0;
	}
	case COMMAND_IS_PLAYER_MADE_SAFE:
	{
		UpdateCompareFlag(CPad::GetPad(0)->IsPlayerControlsDisabledBy(PLAYERCONTROL_PLAYERINFO));
		return 0;
	}
	case COMMAND_PRINT_IF_FREE:
	{
		wchar* text = CTheScripts::GetTextByKeyFromScript(&m_nIp);
		CollectParameters(&m_nIp, 2);
		//CMessages::AddMessageIfFree(text, GET_INTEGER_PARAM(0), GET_INTEGER_PARAM(1)); TODO
		return 0;
	}
	case COMMAND_IS_E3_BUILD:
		UpdateCompareFlag(false);
		return 0;
	case COMMAND_DECLARE_FORT_STAUNTON_DESTROYED_FLAG:
		CTheScripts::FSDestroyedFlag = (uint8*)GetPointerToScriptVariable(&m_nIp, VAR_GLOBAL) - CTheScripts::ScriptSpace;
		return 0;
	case COMMAND_CLEAR_BIG_MESSAGES:
		//CMessages::ClearBigMessagesOnly(); TODO
		//CHud::ClearBigMessagesExcept(2, 2); TODO
		CGarages::MessageEndTime = CGarages::MessageStartTime;
		return 0;
	case COMMAND_CLEAR_AREA_OF_OBJECTS:
	{
		CollectParameters(&m_nIp, 6);
		uint32 i = CPools::GetObjectPool()->GetSize();
		float infX = GET_FLOAT_PARAM(0);
		float infY = GET_FLOAT_PARAM(1);
		float infZ = GET_FLOAT_PARAM(2);
		float supX = GET_FLOAT_PARAM(3);
		float supY = GET_FLOAT_PARAM(4);
		float supZ = GET_FLOAT_PARAM(5);
		while (i--) {
			CObject* pObject = CPools::GetObjectPool()->GetSlot(i);
			if (pObject && pObject->CanBeDeleted() && pObject->IsWithinArea(infX, infY, infZ, supX, supY, supZ)) {
				pObject->DeleteRwObject();
				CWorld::Remove(pObject);
				delete pObject;
			}
		}
		i = CPools::GetDummyPool()->GetSize();
		while (i--) {
			CDummy* pDummy = CPools::GetDummyPool()->GetSlot(i);
			if (pDummy && pDummy->IsObject() && pDummy->IsWithinArea(infX, infY, infZ, supX, supY, supZ)) {
				pDummy->DeleteRwObject();
				CWorld::Remove(pDummy);
				delete pDummy;
			}
		}
		return 0;
	}
	case COMMAND_LOAD_NON_STANDARD_PED_ANIM:
		CollectParameters(&m_nIp, 1);
		// CPed::LoadNonStandardPedAnim(GET_INTEGER_PARAM(0));
		return 0;
	case COMMAND_UNLOAD_NON_STANDARD_PED_ANIM:
		CollectParameters(&m_nIp, 1);
		// CPed::UnloadNonStandardPedAnim(SET_INTEGER_PARAM(0));
		return 0;
	case COMMAND_1566:
		CollectParameters(&m_nIp, 1);
		return 0;
	case COMMAND_BUILD_WORLD_GEOMETRY:
		CollectParameters(&m_nIp, 1);
		if (/*gBuildWorldGeom*/ false) {
			//base::cWorldGeom::GetInstance()->Build(GET_INTEGER_PARAM(0));
			UpdateCompareFlag(true);
		}
		else {
			UpdateCompareFlag(false);
		}
		return 0;
	case COMMAND_STORE_BUILDING_SWAP:
		CollectParameters(&m_nIp, 4);
		// base::cWorldGeom::GetInstance()->StoreBuildingSwap(GET_INTEGER_PARAM(0), GET_INTEGER_PARAM(1), GET_INTEGER_PARAM(2), GET_INTEGER_PARAM(3) != 0);
		return 0;
	case COMMAND_IS_MULTIPLAYER_ACTIVE:
		UpdateCompareFlag(false); // TODO?
		return 0;
	case COMMAND_GET_MULTIPLAYER_MODE:
		SET_INTEGER_PARAM(0, 0); // TODO
		StoreParameters(&m_nIp, 1);
		return 0;
	case COMMAND_MULTIPLAYER_SCRIPT_DONE:
		//gbStartingScriptsFromLua = false; TODO?
		return 0;
	case COMMAND_IS_MULTIPLAYER_SERVER:
		UpdateCompareFlag(false); // TODO?
		return 0;
	case COMMAND_IS_MULTIPLAYER_TEAM_GAME:
		UpdateCompareFlag(false); // TODO?
		return 0;
	case COMMAND_GET_MULTIPLAYER_TEAM_ID:
		SET_INTEGER_PARAM(0, 0); // TODO
		StoreParameters(&m_nIp, 1);
		return 0;
	case COMMAND_DOES_SHORTCUT_TAXI_EXIST:
		UpdateCompareFlag(CGameLogic::pShortCutTaxi != nil);
		return 0;
	case COMMAND_SET_ONSCREEN_TIMER_COLOUR:
		CollectParameters(&m_nIp, 4);
		// gbColour = CRGBA(GET_INTEGER_PARAM(0), GET_INTEGER_PARAM(1), GET_INTEGER_PARAM(2), GET_INTEGER_PARAM(3));
		return 0;
	case COMMAND_SET_ONSCREEN_TIMER_BACKGROUND_COLOUR:
		CollectParameters(&m_nIp, 4);
		// gbColour2 = CRGBA(GET_INTEGER_PARAM(0), GET_INTEGER_PARAM(1), GET_INTEGER_PARAM(2), GET_INTEGER_PARAM(3));
		return 0;
	case COMMAND_REMOVE_CAR_BOOT:
	{
		CollectParameters(&m_nIp, 1);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(GET_INTEGER_PARAM(0));
		script_assert(pVehicle);
		script_assert(pVehicle->IsCar());
		CAutomobile* pAutomobile = (CAutomobile*)pVehicle;
		pAutomobile->Damage.SetDoorStatus(DOOR_BOOT, DOOR_STATUS_MISSING);
		pAutomobile->SetDoorDamage(CAR_BOOT, DOOR_BOOT, true);
		return 0;
	}
	case COMMAND_ADD_POINT_3D_MARKER:
	{
		uint32 ip = m_nIp;
		uint32 id = (uint32)(uintptr)GetPointerToScriptVariable(&ip, 0);
		static CVector vPreviousLocation;
		CollectParameters(&m_nIp, 7);
		CVector pos = GET_VECTOR_PARAM(0);
		if (pos.z <= MAP_Z_LOW_LIMIT)
			pos.z = CWorld::FindGroundZForCoord(pos.x, pos.y);
		if (vPreviousLocation != pos) {
			//C3dMarkers::CopyMarker(id, id - 1, 1, 1);
		}
		// TODO: 5 is not a valid type
		//C3dMarkers::PlaceMarker(id, 5, pos, GET_FLOAT_PARAM(3) * 0.7f, GET_INTEGER_PARAM(4), GET_INTEGER_PARAM(5), GET_INTEGER_PARAM(6), 255, 128, 0.0f, 1); // TODO: 0, 100.0f, 1, 0
		vPreviousLocation = pos;
		return 0;
	}
	case COMMAND_GET_VECTOR_FROM_MULTIPLAYER:
		SET_VECTOR_PARAM(0, gVectorSetInLua);
		StoreParameters(&m_nIp, 3);
		return 0;
	case COMMAND_PRINT_HELP_ALWAYS:
	{
		// CHud::mAlwaysAllowHelpText = true; // TODO
		wchar* text = CTheScripts::GetTextByKeyFromScript(&m_nIp);
		CHud::SetHelpMessage(text, false); // + false, true
		return 0;
	}
	case COMMAND_PRINT_HELP_FOREVER_ALWAYS:
	{
		// CHud::mAlwaysAllowHelpText = true; // TODO
		wchar* text = CTheScripts::GetTextByKeyFromScript(&m_nIp);
		CHud::SetHelpMessage(text, false, true); // + true
		return 0;
	}
	case COMMAND_SWITCH_FERRY_COLLISION:
		CollectParameters(&m_nIp, 1);
		CFerry::SwitchFerryCollision(GET_INTEGER_PARAM(0));
		return 0;
	case COMMAND_SET_CHAR_MAX_HEALTH:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CPools::GetPedPool()->GetAt(GET_INTEGER_PARAM(0));
		script_assert(pPed);
		pPed->m_fHealth = GET_INTEGER_PARAM(1);
		pPed->m_fMaxHealth = GET_INTEGER_PARAM(1);
		return 0;
	}
	case COMMAND_SET_CHAR_SHOOT_TIMER:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CPools::GetPedPool()->GetAt(GET_INTEGER_PARAM(0));
		script_assert(pPed);
		pPed->m_nScriptShootTimer = GET_INTEGER_PARAM(1);
		return 0;
	}
	case COMMAND_SET_CHAR_ATTACK_TIMER:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CPools::GetPedPool()->GetAt(GET_INTEGER_PARAM(0));
		script_assert(pPed);
		pPed->m_nScriptAttackTimer = GET_INTEGER_PARAM(1);
		return 0;
	}
	case COMMAND_SET_HELI_ROTOR_BLADES_FULLSPEED:
	{
		CollectParameters(&m_nIp, 1);
		CAutomobile* pHeli = (CAutomobile*)CPools::GetVehiclePool()->GetAt(GET_INTEGER_PARAM(0));
		script_assert(pHeli);
		if (pHeli->GetVehicleAppearance() == VEHICLE_APPEARANCE_HELI)
			pHeli->m_aWheelSpeed[1] = 0.22f;
		return 0;
	}
	case COMMAND_SET_CRUSHER_REWARD_MULTIPLIER:
	{
		CollectParameters(&m_nIp, 1);
		// CGarages::CrusherRewardMultiplier = GET_INTEGER_PARAM(0);
		return 0;
	}
	case COMMAND_SWAP_BUILDINGS:
	{
		CollectParameters(&m_nIp, 2);
		// base::cWorldStream::Instance()->SwapBuilding(GET_INTEGER_PARAM(0), GET_INTEGER_PARAM(1));
		return 0;
	}
	case COMMAND_STREAM_BUILDING_SWAPS:
	{
		// UpdateCompareFlag(base::cWorldStream::Instance()->StreamBuildingSwaps());
		UpdateCompareFlag(true);
		return 0;
	}
	case COMMAND_IS_WORLD_STREAMING_COMPLETE:
	{
		// UpdateCompareFlag(base::cWorldStream::Instance()->IsStreamingComplet());
		UpdateCompareFlag(true);
		return 0;
	}
	case COMMAND_SWAP_TO_STREAMED_SECTOR:
	{
		// base::cWorldStream::Instance()->SwapToStreamedSector();
		return 0;
	}
	case COMMAND_SET_CHAR_ATTACKS_PLAYER_WITH_COPS:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CPools::GetPedPool()->GetAt(GET_INTEGER_PARAM(0));
		script_assert(pPed);
		pPed->bAttacksPlayerWithCops = (GET_INTEGER_PARAM(1) != 0);
		return 0;
	}
	case COMMAND_REGISTER_FACE_PLANT_DISTANCE:
		CollectParameters(&m_nIp, 1);
		//CStats::LongestFacePlantDist = Max(CStats::LongestFacePlantDist, GET_FLOAT_PARAM(0));
		return 0;
	case COMMAND_REGISTER_MAX_SECONDS_ON_CARNAGE_LEFT:
		CollectParameters(&m_nIp, 1);
		//CStats::MaxSecondsOnCarnageLeft = Max(CStats::MaxSecondsOnCarnageLeft, GET_INTEGER_PARAM(0));
		return 0;
	case COMMAND_REGISTER_MAX_KILLS_ON_RC_TRIAD:
		CollectParameters(&m_nIp, 1);
		//CStats::MaxKillsOnRcTriad = Max(CStats::MaxKillsOnRcTriad, GET_INTEGER_PARAM(0));
		return 0;
	case COMMAND_REGISTER_HIGHEST_LEVEL_SLASH_TV:
		CollectParameters(&m_nIp, 1);
		//CStats::HighestLevelSlashTv = Max(CStats::HighestLevelSlashTv, GET_INTEGER_PARAM(0));
		return 0;
	case COMMAND_ADD_MONEY_MADE_WITH_SLASH_TV:
		CollectParameters(&m_nIp, 1);
		//CStats::MoneyMadeWithSlashTv += GET_INTEGER_PARAM(0);
		return 0;
	case COMMAND_ADD_TOTAL_KILLS_ON_SLASH_TV:
		CollectParameters(&m_nIp, 1);
		//CStats::TotalKillsOnSlashTV += GET_INTEGER_PARAM(0);
		return 0;
	default:
		script_assert(0);
	}
	return -1;
}