#include "common.h"

#include "Script.h"
#include "ScriptCommands.h"

#include "AnimBlendAssociation.h"
#include "BulletInfo.h"
#include "CarAI.h"
#include "CarCtrl.h"
#include "CivilianPed.h"
#include "Cranes.h"
#include "DMAudio.h"
#include "Darkel.h"
#include "Explosion.h"
#include "Fire.h"
#include "Frontend.h"
#include "Garages.h"
#include "General.h"
#include "Heli.h"
#include "Hud.h"
#include "Messages.h"
#include "ParticleObject.h"
#include "PedRoutes.h"
#include "Phones.h"
#include "Pickups.h"
#include "Plane.h"
#include "Pools.h"
#include "Population.h"
#include "Radar.h"
#include "Record.h"
#include "RpAnimBlend.h"
#include "Rubbish.h"
#include "SpecialFX.h"
#include "Stats.h"
#include "Streaming.h"
#include "TxdStore.h"
#include "User.h"
#include "WaterLevel.h"
#include "World.h"
#include "Zones.h"
#include "Wanted.h"

int8 CRunningScript::ProcessCommands800To899(int32 command)
{
	CMatrix tmp_matrix;
	switch (command) {
	case COMMAND_IS_CHAR_IN_PLAYERS_GROUP:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		CPed* pLeader = CWorld::Players[ScriptParams[1]].m_pPed;
		script_assert(pPed);
		script_assert(pLeader);
		UpdateCompareFlag(pPed->m_leader == pLeader);
		return 0;
	}
	case COMMAND_EXPLODE_CHAR_HEAD:
	{
		CollectParameters(&m_nIp, 1);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		if (pPed->m_nPedState == PED_DRIVING) {
			pPed->SetDead();
			if (!pPed->IsPlayer())
				pPed->FlagToDestroyWhenNextProcessed();
		}
		else if (CGame::nastyGame && pPed->IsPedInControl()) {
			pPed->ApplyHeadShot(WEAPONTYPE_SNIPERRIFLE, pPed->GetNodePosition(PED_HEAD), true);
		}
		else {
			pPed->SetDie(ANIM_STD_KO_FRONT, 4.0f, 0.0f);
		}
		return 0;
	}
	case COMMAND_EXPLODE_PLAYER_HEAD:
	{
		CollectParameters(&m_nIp, 1);
		CPed* pPed = CWorld::Players[ScriptParams[0]].m_pPed;
		script_assert(pPed);
		if (CGame::nastyGame) {
			pPed->ApplyHeadShot(WEAPONTYPE_SNIPERRIFLE, pPed->GetNodePosition(PED_HEAD), true);
		}
		else {
			pPed->SetDie(ANIM_STD_KO_FRONT, 4.0f, 0.0f);
		}
		return 0;
	}
	case COMMAND_ANCHOR_BOAT:
	{
		CollectParameters(&m_nIp, 2);
		CBoat* pBoat = (CBoat*)CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		script_assert(pBoat && pBoat->m_vehType == VEHICLE_TYPE_BOAT);
		pBoat->m_bIsAnchored = (ScriptParams[1] != 0);
		return 0;
	}
	case COMMAND_SET_ZONE_GROUP:
	{
		char zone[KEY_LENGTH_IN_SCRIPT];
		CTheScripts::ReadTextLabelFromScript(&m_nIp, zone);
		m_nIp += KEY_LENGTH_IN_SCRIPT;
		CollectParameters(&m_nIp, 2);
		int zone_id = CTheZones::FindZoneByLabelAndReturnIndex(zone);
		if (zone_id < 0) {
			printf("Couldn't find zone - %s\n", zone);
			return 0;
		}
		CTheZones::SetPedGroup(zone_id, ScriptParams[0], ScriptParams[1]);
		return 0;
	}
	case COMMAND_START_CAR_FIRE:
	{
		CollectParameters(&m_nIp, 1);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		script_assert(pVehicle);
		ScriptParams[0] = gFireManager.StartScriptFire(pVehicle->GetPosition(), pVehicle, 0.8f, 1);
		StoreParameters(&m_nIp, 1);
		return 0;
	}
	case COMMAND_START_CHAR_FIRE:
	{
		CollectParameters(&m_nIp, 1);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		ScriptParams[0] = gFireManager.StartScriptFire(pPed->GetPosition(), pPed, 0.8f, 1);
		StoreParameters(&m_nIp, 1);
		return 0;
	}
	case COMMAND_GET_RANDOM_CAR_OF_TYPE_IN_AREA:
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
			if (ScriptParams[4] != pVehicle->GetModelIndex() && ScriptParams[4] >= 0)
				continue;
			if (pVehicle->VehicleCreatedBy != RANDOM_VEHICLE)
				continue;
			if (!pVehicle->IsWithinArea(infX, infY, supX, supY))
				continue;
			handle = CPools::GetVehiclePool()->GetIndex(pVehicle);
			pVehicle->VehicleCreatedBy = MISSION_VEHICLE;
			++CCarCtrl::NumMissionCars;
			--CCarCtrl::NumRandomCars;
			if (m_bIsMissionScript)
				CTheScripts::MissionCleanUp.AddEntityToList(handle, CLEANUP_CAR);
		}
		ScriptParams[0] = handle;
		StoreParameters(&m_nIp, 1);
		return 0;
	}
	case COMMAND_GET_RANDOM_CAR_OF_TYPE_IN_ZONE:
	{
		char zone[KEY_LENGTH_IN_SCRIPT];
		CTheScripts::ReadTextLabelFromScript(&m_nIp, zone);
		int zone_id = CTheZones::FindZoneByLabelAndReturnIndex(zone);
		if (zone_id != -1)
			m_nIp += KEY_LENGTH_IN_SCRIPT;
		CZone* pZone = CTheZones::GetZone(zone_id);
		CollectParameters(&m_nIp, 1);
		int handle = -1;
		uint32 i = CPools::GetVehiclePool()->GetSize();
		while (i--) {
			CVehicle* pVehicle = CPools::GetVehiclePool()->GetSlot(i);
			if (!pVehicle)
				continue;
			if (ScriptParams[0] != pVehicle->GetModelIndex() && ScriptParams[0] >= 0)
				continue;
			if (pVehicle->VehicleCreatedBy != RANDOM_VEHICLE)
				continue;
			if (!CTheZones::PointLiesWithinZone(&pVehicle->GetPosition(), pZone))
				continue;
			handle = CPools::GetVehiclePool()->GetIndex(pVehicle);
			pVehicle->VehicleCreatedBy = MISSION_VEHICLE;
			++CCarCtrl::NumMissionCars;
			--CCarCtrl::NumRandomCars;
			if (m_bIsMissionScript)
				CTheScripts::MissionCleanUp.AddEntityToList(handle, CLEANUP_CAR);
		}
		ScriptParams[0] = handle;
		StoreParameters(&m_nIp, 1);
		return 0;
	}
	case COMMAND_HAS_RESPRAY_HAPPENED:
	{
		CollectParameters(&m_nIp, 1);
		UpdateCompareFlag(CGarages::HasResprayHappened(ScriptParams[0]));
		return 0;
	}
	case COMMAND_SET_CAMERA_ZOOM:
	{
		CollectParameters(&m_nIp, 1);
		if (TheCamera.Cams[TheCamera.ActiveCam].Mode == CCam::MODE_FOLLOWPED)
			TheCamera.SetZoomValueFollowPedScript(ScriptParams[0]);
		else if (TheCamera.Cams[TheCamera.ActiveCam].Mode == CCam::MODE_CAM_ON_A_STRING)
			TheCamera.SetZoomValueCamStringScript(ScriptParams[0]);
		return 0;
	}
	case COMMAND_CREATE_PICKUP_WITH_AMMO:
	{
		CollectParameters(&m_nIp, 6);
		int16 model = ScriptParams[0];
		if (model < 0)
			model = CTheScripts::UsedObjectArray[-model].index;
		CVector pos = *(CVector*)&ScriptParams[3];
		if (pos.z <= MAP_Z_LOW_LIMIT)
			pos.z = CWorld::FindGroundZForCoord(pos.x, pos.y) + PICKUP_PLACEMENT_OFFSET;
		CPickups::GetActualPickupIndex(CollectNextParameterWithoutIncreasingPC(m_nIp));
		ScriptParams[0] = CPickups::GenerateNewOne(pos, model, ScriptParams[1], ScriptParams[2]);
		StoreParameters(&m_nIp, 1);
		return 0;
	}
	case COMMAND_SET_CAR_RAM_CAR:
	{
		CollectParameters(&m_nIp, 2);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		script_assert(pVehicle);
		CVehicle* pTarget = CPools::GetVehiclePool()->GetAt(ScriptParams[1]);
		script_assert(pTarget);
		CCarAI::TellCarToRamOtherCar(pVehicle, pTarget);
		return 0;
	}
	case COMMAND_SET_CAR_BLOCK_CAR:
	{
		CollectParameters(&m_nIp, 2);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		script_assert(pVehicle);
		CVehicle* pTarget = CPools::GetVehiclePool()->GetAt(ScriptParams[1]);
		script_assert(pTarget);
		CCarAI::TellCarToBlockOtherCar(pVehicle, pTarget);
		return 0;
	}
	case COMMAND_SET_CHAR_OBJ_CATCH_TRAIN:
	{
		CollectParameters(&m_nIp, 1);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		pPed->bScriptObjectiveCompleted = false;
		pPed->SetObjective(OBJECTIVE_CATCH_TRAIN);
		return 0;
	}
#ifdef GTA_SCRIPT_COLLECTIVE
	case COMMAND_SET_COLL_OBJ_CATCH_TRAIN:
		CollectParameters(&m_nIp, 1);
		CTheScripts::SetObjectiveForAllPedsInCollective(ScriptParams[0], OBJECTIVE_CATCH_TRAIN);
		return 0;
#endif
	case COMMAND_SET_PLAYER_NEVER_GETS_TIRED:
	{
		CollectParameters(&m_nIp, 2);
		CPlayerInfo* pPlayer = &CWorld::Players[ScriptParams[0]];
		pPlayer->m_bInfiniteSprint = (ScriptParams[1] != 0);
		return 0;
	}
	case COMMAND_SET_PLAYER_FAST_RELOAD:
	{
		CollectParameters(&m_nIp, 2);
		CPlayerInfo* pPlayer = &CWorld::Players[ScriptParams[0]];
		pPlayer->m_bFastReload = (ScriptParams[1] != 0);
		return 0;
	}
	case COMMAND_SET_CHAR_BLEEDING:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		pPed->bPedIsBleeding = (ScriptParams[1] != 0);
		return 0;
	}
	case COMMAND_SET_CAR_FUNNY_SUSPENSION:
	{
		CollectParameters(&m_nIp, 2);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		script_assert(pVehicle);
		// no action
		return 0;
	}
	case COMMAND_SET_CAR_BIG_WHEELS:
	{
		CollectParameters(&m_nIp, 2);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		script_assert(pVehicle);
		script_assert(pVehicle->m_vehType == VEHICLE_TYPE_CAR);
		CAutomobile* pCar = (CAutomobile*)pVehicle;
		pCar->bBigWheels = (ScriptParams[1] != 0);
		return 0;
	}
	case COMMAND_SET_FREE_RESPRAYS:
		CollectParameters(&m_nIp, 1);
		CGarages::SetFreeResprays(ScriptParams[0] != 0);
		return 0;
	case COMMAND_SET_PLAYER_VISIBLE:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CWorld::Players[ScriptParams[0]].m_pPed;
		script_assert(pPed);
		pPed->bIsVisible = (ScriptParams[1] != 0);
		return 0;
	}
	case COMMAND_SET_CHAR_VISIBLE:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		pPed->bIsVisible = (ScriptParams[1] != 0);
		return 0;
	}
	case COMMAND_SET_CAR_VISIBLE:
	{
		CollectParameters(&m_nIp, 2);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		script_assert(pVehicle);
		pVehicle->bIsVisible = (ScriptParams[1] != 0);
		return 0;
	}
	case COMMAND_IS_AREA_OCCUPIED:
	{
		CollectParameters(&m_nIp, 11);
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
		int16 total;
		CWorld::FindObjectsIntersectingCube(CVector(infX, infY, infZ), CVector(supX, supY, supZ), &total, 2, nil,
			!!ScriptParams[6], !!ScriptParams[7], !!ScriptParams[8], !!ScriptParams[9], !!ScriptParams[10]);
		UpdateCompareFlag(total > 0);
		return 0;
	}
	case COMMAND_START_DRUG_RUN:
		CPlane::CreateIncomingCesna();
		return 0;
	case COMMAND_HAS_DRUG_RUN_BEEN_COMPLETED:
		UpdateCompareFlag(CPlane::HasCesnaLanded());
		return 0;
	case COMMAND_HAS_DRUG_PLANE_BEEN_SHOT_DOWN:
		UpdateCompareFlag(CPlane::HasCesnaBeenDestroyed());
		return 0;
	case COMMAND_SAVE_PLAYER_FROM_FIRES:
		CollectParameters(&m_nIp, 1);
		gFireManager.ExtinguishPoint(CWorld::Players[ScriptParams[0]].GetPos(), 3.0f);
		return 0;
	case COMMAND_DISPLAY_TEXT:
	{
		CollectParameters(&m_nIp, 2);
		wchar* text = CTheScripts::GetTextByKeyFromScript(&m_nIp);
		CTheScripts::IntroTextLines[CTheScripts::NumberOfIntroTextLinesThisFrame].m_fAtX = *(float*)&ScriptParams[0];
		CTheScripts::IntroTextLines[CTheScripts::NumberOfIntroTextLinesThisFrame].m_fAtY = *(float*)&ScriptParams[1];
		uint16 len = CMessages::GetWideStringLength(text);
		for (uint16 i = 0; i < len; i++)
			CTheScripts::IntroTextLines[CTheScripts::NumberOfIntroTextLinesThisFrame].m_Text[i] = text[i];
		for (uint16 i = len; i < SCRIPT_TEXT_MAX_LENGTH; i++)
			CTheScripts::IntroTextLines[CTheScripts::NumberOfIntroTextLinesThisFrame].m_Text[i] = 0;
		++CTheScripts::NumberOfIntroTextLinesThisFrame;
		return 0;
	}
	case COMMAND_SET_TEXT_SCALE:
	{
		CollectParameters(&m_nIp, 2);
		CTheScripts::IntroTextLines[CTheScripts::NumberOfIntroTextLinesThisFrame].m_fScaleX = *(float*)&ScriptParams[0];
		CTheScripts::IntroTextLines[CTheScripts::NumberOfIntroTextLinesThisFrame].m_fScaleY = *(float*)&ScriptParams[1];
		return 0;
	}
	case COMMAND_SET_TEXT_COLOUR:
	{
		CollectParameters(&m_nIp, 4);
		CTheScripts::IntroTextLines[CTheScripts::NumberOfIntroTextLinesThisFrame].m_sColor =
			CRGBA(ScriptParams[0], ScriptParams[1], ScriptParams[2], ScriptParams[3]);
		return 0;
	}
	case COMMAND_SET_TEXT_JUSTIFY:
	{
		CollectParameters(&m_nIp, 1);
		CTheScripts::IntroTextLines[CTheScripts::NumberOfIntroTextLinesThisFrame].m_bJustify = (ScriptParams[0] != 0);
		return 0;
	}
	case COMMAND_SET_TEXT_CENTRE:
	{
		CollectParameters(&m_nIp, 1);
		CTheScripts::IntroTextLines[CTheScripts::NumberOfIntroTextLinesThisFrame].m_bCentered = (ScriptParams[0] != 0);
		return 0;
	}
	case COMMAND_SET_TEXT_WRAPX:
	{
		CollectParameters(&m_nIp, 1);
		CTheScripts::IntroTextLines[CTheScripts::NumberOfIntroTextLinesThisFrame].m_fWrapX = *(float*)&ScriptParams[0];
		return 0;
	}
	case COMMAND_SET_TEXT_CENTRE_SIZE:
	{
		CollectParameters(&m_nIp, 1);
		CTheScripts::IntroTextLines[CTheScripts::NumberOfIntroTextLinesThisFrame].m_fCenterSize = *(float*)&ScriptParams[0];
		return 0;
	}
	case COMMAND_SET_TEXT_BACKGROUND:
	{
		CollectParameters(&m_nIp, 1);
		CTheScripts::IntroTextLines[CTheScripts::NumberOfIntroTextLinesThisFrame].m_bBackground = (ScriptParams[0] != 0);
		return 0;
	}
	case COMMAND_SET_TEXT_BACKGROUND_COLOUR:
	{
		CollectParameters(&m_nIp, 4);
		CTheScripts::IntroTextLines[CTheScripts::NumberOfIntroTextLinesThisFrame].m_sBackgroundColor =
			CRGBA(ScriptParams[0], ScriptParams[1], ScriptParams[2], ScriptParams[3]);
		return 0;
	}
	case COMMAND_SET_TEXT_BACKGROUND_ONLY_TEXT:
	{
		CollectParameters(&m_nIp, 1);
		CTheScripts::IntroTextLines[CTheScripts::NumberOfIntroTextLinesThisFrame].m_bBackgroundOnly = (ScriptParams[0] != 0);
		return 0;
	}
	case COMMAND_SET_TEXT_PROPORTIONAL:
	{
		CollectParameters(&m_nIp, 1);
		CTheScripts::IntroTextLines[CTheScripts::NumberOfIntroTextLinesThisFrame].m_bTextProportional = (ScriptParams[0] != 0);
		return 0;
	}
	case COMMAND_SET_TEXT_FONT:
	{
		CollectParameters(&m_nIp, 1);
		CTheScripts::IntroTextLines[CTheScripts::NumberOfIntroTextLinesThisFrame].m_nFont = ScriptParams[0];
		return 0;
	}
	case COMMAND_INDUSTRIAL_PASSED:
		CStats::IndustrialPassed = true;
		DMAudio.PlayRadioAnnouncement(STREAMED_SOUND_ANNOUNCE_COMMERCIAL_OPEN);
		return 0;
	case COMMAND_COMMERCIAL_PASSED:
		CStats::CommercialPassed = true;
		DMAudio.PlayRadioAnnouncement(STREAMED_SOUND_ANNOUNCE_SUBURBAN_OPEN);
		return 0;
	case COMMAND_SUBURBAN_PASSED:
		CStats::SuburbanPassed = true;
		return 0;
	case COMMAND_ROTATE_OBJECT:
	{
		CollectParameters(&m_nIp, 4);
		CObject* pObject = CPools::GetObjectPool()->GetAt(ScriptParams[0]);
		script_assert(pObject);
		float heading = LimitAngleOnCircle(
			RADTODEG(Atan2(-pObject->GetForward().x, pObject->GetForward().y)));
		float headingTarget = *(float*)&ScriptParams[1];
#ifdef FIX_BUGS
		float rotateBy = *(float*)&ScriptParams[2] * CTimer::GetTimeStepFix();
#else
		float rotateBy = *(float*)&ScriptParams[2];
#endif
		if (headingTarget == heading) { // using direct comparasion here is fine
			UpdateCompareFlag(true);
			return 0;
		}
		float angleClockwise = LimitAngleOnCircle(headingTarget - heading);
		float angleCounterclockwise = LimitAngleOnCircle(heading - headingTarget);
		float newHeading;
		if (angleClockwise < angleCounterclockwise)
			newHeading = rotateBy < angleClockwise ? heading + rotateBy : headingTarget;
		else
			newHeading = rotateBy < angleCounterclockwise ? heading - rotateBy : headingTarget;
		bool obstacleInPath = false;
		if (ScriptParams[3]) {
			CVector pos = pObject->GetPosition();
			tmp_matrix.SetRotateZ(DEGTORAD(newHeading));
			tmp_matrix.GetPosition() += pos;
			CColModel* pColModel = pObject->GetColModel();
			CVector cp1 = tmp_matrix * pColModel->boundingBox.min;
			CVector cp2 = tmp_matrix * CVector(pColModel->boundingBox.max.x, pColModel->boundingBox.min.y, pColModel->boundingBox.min.z);
			CVector cp3 = tmp_matrix * CVector(pColModel->boundingBox.min.x, pColModel->boundingBox.max.y, pColModel->boundingBox.min.z);
			CVector cp4 = tmp_matrix * CVector(pColModel->boundingBox.min.x, pColModel->boundingBox.min.y, pColModel->boundingBox.max.z);
			int16 collisions;
			CWorld::FindObjectsIntersectingAngledCollisionBox(pColModel->boundingBox, tmp_matrix, pos,
				Min(cp1.x, Min(cp2.x, Min(cp3.x, cp4.x))),
				Min(cp1.y, Min(cp2.y, Min(cp3.y, cp4.y))),
				Max(cp1.x, Max(cp2.x, Max(cp3.x, cp4.x))),
				Max(cp1.y, Max(cp2.y, Max(cp3.y, cp4.y))),
				&collisions, 2, nil, false, true, true, false, false);
			if (collisions > 0)
				obstacleInPath = true;
		}
		if (obstacleInPath) {
			UpdateCompareFlag(true);
			return 0;
		}
		pObject->SetHeading(DEGTORAD(newHeading));
		pObject->GetMatrix().UpdateRW();
		pObject->UpdateRwFrame();
		UpdateCompareFlag(newHeading == headingTarget); // using direct comparasion here is fine
		return 0;
	}
	case COMMAND_SLIDE_OBJECT:
	{
		CollectParameters(&m_nIp, 8);
		CObject* pObject = CPools::GetObjectPool()->GetAt(ScriptParams[0]);
		script_assert(pObject);
		CVector pos = pObject->GetPosition();
		CVector posTarget = *(CVector*)&ScriptParams[1];
#ifdef FIX_BUGS
		CVector slideBy = *(CVector*)&ScriptParams[4] * CTimer::GetTimeStepFix();
#else
		CVector slideBy = *(CVector*)&ScriptParams[4];
#endif
		if (posTarget == pos) { // using direct comparasion here is fine
			UpdateCompareFlag(true);
			return 0;
		}
		CVector posDiff = pos - posTarget;
		CVector newPosition;
		if (posDiff.x < 0)
			newPosition.x = -posDiff.x < slideBy.x ? posTarget.x : pos.x + slideBy.x;
		else
			newPosition.x = posDiff.x < slideBy.x ? posTarget.x : pos.x - slideBy.x;
		if (posDiff.y < 0)
			newPosition.y = -posDiff.y < slideBy.y ? posTarget.y : pos.y + slideBy.y;
		else
			newPosition.y = posDiff.y < slideBy.y ? posTarget.y : pos.y - slideBy.y;
		if (posDiff.z < 0)
			newPosition.z = -posDiff.z < slideBy.z ? posTarget.z : pos.z + slideBy.z;
		else
			newPosition.z = posDiff.z < slideBy.z ? posTarget.z : pos.z - slideBy.z;
		bool obstacleInPath = false;
		if (ScriptParams[7]) {
			tmp_matrix = pObject->GetMatrix();
			tmp_matrix.GetPosition() = newPosition;
			CColModel* pColModel = pObject->GetColModel();
			CVector cp1 = tmp_matrix * pColModel->boundingBox.min;
			CVector cp2 = tmp_matrix * CVector(pColModel->boundingBox.max.x, pColModel->boundingBox.min.y, pColModel->boundingBox.min.z);
			CVector cp3 = tmp_matrix * CVector(pColModel->boundingBox.min.x, pColModel->boundingBox.max.y, pColModel->boundingBox.min.z);
			CVector cp4 = tmp_matrix * CVector(pColModel->boundingBox.min.x, pColModel->boundingBox.min.y, pColModel->boundingBox.max.z);
			int16 collisions;
			CWorld::FindObjectsIntersectingAngledCollisionBox(pColModel->boundingBox, tmp_matrix, newPosition,
				Min(cp1.x, Min(cp2.x, Min(cp3.x, cp4.x))),
				Min(cp1.y, Min(cp2.y, Min(cp3.y, cp4.y))),
				Max(cp1.x, Max(cp2.x, Max(cp3.x, cp4.x))),
				Max(cp1.y, Max(cp2.y, Max(cp3.y, cp4.y))),
				&collisions, 2, nil, false, true, true, false, false);
			if (collisions > 0)
				obstacleInPath = true;
		}
		if (obstacleInPath) {
			UpdateCompareFlag(true);
			return 0;
		}
		pObject->Teleport(newPosition);
		UpdateCompareFlag(newPosition == posTarget); // using direct comparasion here is fine
		return 0;
	}
	case COMMAND_REMOVE_CHAR_ELEGANTLY:
	{
		CollectParameters(&m_nIp, 1);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		if (pPed && pPed->CharCreatedBy == MISSION_CHAR){
			CWorld::RemoveReferencesToDeletedObject(pPed);
			if (pPed->bInVehicle){
				if (pPed->m_pMyVehicle){
					if (pPed == pPed->m_pMyVehicle->pDriver){
						pPed->m_pMyVehicle->RemoveDriver();
						pPed->m_pMyVehicle->SetStatus(STATUS_ABANDONED);
						if (pPed->m_pMyVehicle->m_nDoorLock == CARLOCK_LOCKED_INITIALLY)
							pPed->m_pMyVehicle->m_nDoorLock = CARLOCK_UNLOCKED;
						if (pPed->m_nPedType == PEDTYPE_COP && pPed->m_pMyVehicle->IsLawEnforcementVehicle())
							pPed->m_pMyVehicle->ChangeLawEnforcerState(0);
					}else{
						pPed->m_pMyVehicle->RemovePassenger(pPed);
					}
				}
				delete pPed;
				--CPopulation::ms_nTotalMissionPeds;
			}else{
				pPed->CharCreatedBy = RANDOM_CHAR;
				pPed->bRespondsToThreats = true;
				pPed->bScriptObjectiveCompleted = false;
				pPed->ClearLeader();
				--CPopulation::ms_nTotalMissionPeds;
				pPed->bFadeOut = true;
			}
		}
		if (m_bIsMissionScript)
			CTheScripts::MissionCleanUp.RemoveEntityFromList(ScriptParams[0], CLEANUP_CHAR);
		return 0;
	}
	case COMMAND_SET_CHAR_STAY_IN_SAME_PLACE:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		pPed->bKindaStayInSamePlace = (ScriptParams[1] != 0);
		return 0;
	}
	case COMMAND_IS_NASTY_GAME:
		UpdateCompareFlag(CGame::nastyGame);
		return 0;
	case COMMAND_UNDRESS_CHAR:
	{
		CollectParameters(&m_nIp, 1);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		char name[KEY_LENGTH_IN_SCRIPT];
		CTheScripts::ReadTextLabelFromScript(&m_nIp, name);
		for (int i = 0; i < KEY_LENGTH_IN_SCRIPT; i++)
			name[i] = tolower(name[i]);
		int mi = pPed->GetModelIndex();
		pPed->DeleteRwObject();
		if (pPed->IsPlayer())
			mi = 0;
		CStreaming::RequestSpecialModel(mi, name, STREAMFLAGS_DEPENDENCY | STREAMFLAGS_SCRIPTOWNED);
		m_nIp += KEY_LENGTH_IN_SCRIPT;
		CWorld::Remove(pPed);
		return 0;
	}
	case COMMAND_DRESS_CHAR:
	{
		CollectParameters(&m_nIp, 1);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		int mi = pPed->GetModelIndex();
		pPed->m_modelIndex = -1;
		pPed->SetModelIndex(mi);
		CWorld::Add(pPed);
		return 0;
	}
	case COMMAND_START_CHASE_SCENE:
		CollectParameters(&m_nIp, 1);
		CTimer::Suspend();
		CStreaming::DeleteAllRwObjects();
		CRecordDataForChase::StartChaseScene(*(float*)&ScriptParams[0]);
		CTimer::Resume();
		return 0;
	case COMMAND_STOP_CHASE_SCENE:
		CRecordDataForChase::CleanUpChaseScene();
		return 0;
	case COMMAND_IS_EXPLOSION_IN_AREA:
	{
		CollectParameters(&m_nIp, 7);
		float infX = *(float*)&ScriptParams[1];
		float infY = *(float*)&ScriptParams[2];
		float infZ = *(float*)&ScriptParams[3];
		float supX = *(float*)&ScriptParams[4];
		float supY = *(float*)&ScriptParams[5];
		float supZ = *(float*)&ScriptParams[6];
		if (infX > supX) {
			infX = *(float*)&ScriptParams[4];
			supX = *(float*)&ScriptParams[1];
		}
		if (infY > supY) {
			infY = *(float*)&ScriptParams[5];
			supY = *(float*)&ScriptParams[2];
		}
		if (infZ > supZ) {
			infZ = *(float*)&ScriptParams[6];
			supZ = *(float*)&ScriptParams[3];
		}
		UpdateCompareFlag(CExplosion::TestForExplosionInArea((eExplosionType)ScriptParams[0],
			infX, supX, infY, supY, infZ, supZ));
		return 0;
	}
	case COMMAND_IS_EXPLOSION_IN_ZONE:
	{
		CollectParameters(&m_nIp, 1);
		char zone[KEY_LENGTH_IN_SCRIPT];
		CTheScripts::ReadTextLabelFromScript(&m_nIp, zone);
		int zone_id = CTheZones::FindZoneByLabelAndReturnIndex(zone);
		if (zone_id != -1)
			m_nIp += KEY_LENGTH_IN_SCRIPT;
		CZone* pZone = CTheZones::GetZone(zone_id);
		UpdateCompareFlag(CExplosion::TestForExplosionInArea((eExplosionType)ScriptParams[0],
			pZone->minx, pZone->maxx, pZone->miny, pZone->maxy, pZone->minz, pZone->maxz));
		return 0;
	}
	case COMMAND_START_DRUG_DROP_OFF:
		CPlane::CreateDropOffCesna();
		return 0;
	case COMMAND_HAS_DROP_OFF_PLANE_BEEN_SHOT_DOWN:
		UpdateCompareFlag(CPlane::HasDropOffCesnaBeenShotDown());
		return 0;
	case COMMAND_FIND_DROP_OFF_PLANE_COORDINATES:
	{
		CVector pos = CPlane::FindDropOffCesnaCoordinates();
		*(CVector*)&ScriptParams[0] = pos;
		StoreParameters(&m_nIp, 3);
		return 0;
	}
	case COMMAND_CREATE_FLOATING_PACKAGE:
	{
		CollectParameters(&m_nIp, 3);
		CVector pos = *(CVector*)&ScriptParams[0];
		if (pos.z <= MAP_Z_LOW_LIMIT)
			pos.z = CWorld::FindGroundZForCoord(pos.x, pos.y) + PICKUP_PLACEMENT_OFFSET;
		ScriptParams[0] = CPickups::GenerateNewOne(pos, MI_FLOATPACKAGE1, PICKUP_FLOATINGPACKAGE, 0);
		StoreParameters(&m_nIp, 1);
		return 0;
	}
	case COMMAND_PLACE_OBJECT_RELATIVE_TO_CAR:
	{
		CollectParameters(&m_nIp, 5);
		CObject* pObject = CPools::GetObjectPool()->GetAt(ScriptParams[0]);
		script_assert(pObject);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[1]);
		script_assert(pVehicle);
		CVector offset = *(CVector*)&ScriptParams[2];
		CPhysical::PlacePhysicalRelativeToOtherPhysical(pVehicle, pObject, offset);
		return 0;
	}
	case COMMAND_MAKE_OBJECT_TARGETTABLE:
	{
		CollectParameters(&m_nIp, 1);
		CObject* pObject = CPools::GetObjectPool()->GetAt(ScriptParams[0]);
		script_assert(pObject);
		CPlayerPed* pPlayerPed = CWorld::Players[CWorld::PlayerInFocus].m_pPed;
		script_assert(pPlayerPed);
		pPlayerPed->MakeObjectTargettable(ScriptParams[0]);
		return 0;
	}
	case COMMAND_ADD_ARMOUR_TO_PLAYER:
	{
		CollectParameters(&m_nIp, 2);
		CPlayerPed* pPlayerPed = CWorld::Players[ScriptParams[0]].m_pPed;
		script_assert(pPlayerPed);
		pPlayerPed->m_fArmour = clamp(pPlayerPed->m_fArmour + ScriptParams[1], 0.0f, 100.0f);
		return 0;
	}
	case COMMAND_ADD_ARMOUR_TO_CHAR:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		pPed->m_fArmour = clamp(pPed->m_fArmour + ScriptParams[1], 0.0f, 100.0f);
		return 0;
	}
	case COMMAND_OPEN_GARAGE:
	{
		CollectParameters(&m_nIp, 1);
		CGarages::OpenGarage(ScriptParams[0]);
		return 0;
	}
	case COMMAND_CLOSE_GARAGE:
	{
		CollectParameters(&m_nIp, 1);
		CGarages::CloseGarage(ScriptParams[0]);
		return 0;
	}
	case COMMAND_WARP_CHAR_FROM_CAR_TO_COORD:
	{
		CollectParameters(&m_nIp, 4);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		CVector pos = *(CVector*)&ScriptParams[1];
		if (pos.z <= MAP_Z_LOW_LIMIT)
			pos.z = CWorld::FindGroundZForCoord(pos.x, pos.y);
		if (pPed->bInVehicle){
			if (pPed->m_pMyVehicle->bIsBus)
				pPed->bRenderPedInCar = true;
			if (pPed->m_pMyVehicle->pDriver == pPed){
				pPed->m_pMyVehicle->RemoveDriver();
				pPed->m_pMyVehicle->SetStatus(STATUS_ABANDONED);
				pPed->m_pMyVehicle->bEngineOn = false;
				pPed->m_pMyVehicle->AutoPilot.m_nCruiseSpeed = 0;
			}else{
				pPed->m_pMyVehicle->RemovePassenger(pPed);
			}
			pPed->m_pMyVehicle->SetMoveSpeed(0.0f, 0.0f, -0.00001f);
			pPed->m_pMyVehicle->SetTurnSpeed(0.0f, 0.0f, 0.0f);
		}
		pPed->bInVehicle = false;
		pPed->m_pMyVehicle = nil;
		pPed->SetPedState(PED_IDLE);
		pPed->m_nLastPedState = PED_NONE;
		pPed->bUsesCollision = true;
		pPed->SetMoveSpeed(0.0f, 0.0f, 0.0f);
		pPed->AddWeaponModel(CWeaponInfo::GetWeaponInfo(pPed->m_weapons[pPed->m_currentWeapon].m_eWeaponType)->m_nModelId);
		pPed->RemoveInCarAnims();
		if (pPed->m_pVehicleAnim)
			pPed->m_pVehicleAnim->blendDelta = -1000.0f;
		pPed->m_pVehicleAnim = nil;
		pPed->RestartNonPartialAnims();
		pPed->SetMoveState(PEDMOVE_NONE);
		CAnimManager::BlendAnimation(pPed->GetClump(), pPed->m_animGroup, ANIM_STD_IDLE, 100.0f);
		pos.z += pPed->GetDistanceFromCentreOfMassToBaseOfModel();
		pPed->Teleport(pos);
		CTheScripts::ClearSpaceForMissionEntity(pos, pPed);
		return 0;
	}
	case COMMAND_SET_VISIBILITY_OF_CLOSEST_OBJECT_OF_TYPE:
	{
		CollectParameters(&m_nIp, 6);
		CVector pos = *(CVector*)&ScriptParams[0];
		if (pos.z <= MAP_Z_LOW_LIMIT)
			pos.z = CWorld::FindGroundZForCoord(pos.x, pos.y);
		float range = *(float*)&ScriptParams[3];
		int mi = ScriptParams[4] < 0 ? CTheScripts::UsedObjectArray[-ScriptParams[4]].index : ScriptParams[4];
		int16 total;
		CEntity* apEntities[16];
		CWorld::FindObjectsOfTypeInRange(mi, pos, range, true, &total, 16, apEntities, true, false, false, true, true);
		if (total == 0)
			CWorld::FindObjectsOfTypeInRangeSectorList(mi, CWorld::GetBigBuildingList(LEVEL_GENERIC), pos, range, true, &total, 16, apEntities);
		if (total == 0)
			CWorld::FindObjectsOfTypeInRangeSectorList(mi, CWorld::GetBigBuildingList(CTheZones::FindZoneForPoint(pos)), pos, range, true, &total, 16, apEntities);
		CEntity* pClosestEntity = nil;
		float min_dist = 2.0f * range;
		for (int i = 0; i < total; i++) {
			float dist = (apEntities[i]->GetPosition() - pos).Magnitude();
			if (dist < min_dist) {
				min_dist = dist;
				pClosestEntity = apEntities[i];
			}
		}
		if (pClosestEntity) {
			pClosestEntity->bIsVisible = (ScriptParams[5] != 0);
			CTheScripts::AddToInvisibilitySwapArray(pClosestEntity, ScriptParams[5] != 0);
		}
		return 0;
	}
	case COMMAND_HAS_CHAR_SPOTTED_CHAR:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		CPed* pTarget = CPools::GetPedPool()->GetAt(ScriptParams[1]);
		script_assert(pTarget);
		UpdateCompareFlag(pPed->OurPedCanSeeThisOne(pTarget));
		return 0;
	}
	case COMMAND_SET_CHAR_OBJ_HAIL_TAXI:
	{
		CollectParameters(&m_nIp, 1);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		pPed->bScriptObjectiveCompleted = false;
		pPed->SetObjective(OBJECTIVE_HAIL_TAXI);
		return 0;
	}
	case COMMAND_HAS_OBJECT_BEEN_DAMAGED:
	{
		CollectParameters(&m_nIp, 1);
		CObject* pObject = CPools::GetObjectPool()->GetAt(ScriptParams[0]);
		script_assert(pObject);
		UpdateCompareFlag(pObject->bRenderDamaged || !pObject->bIsVisible);
		return 0;
	}
	case COMMAND_START_KILL_FRENZY_HEADSHOT:
	{
		wchar* text = CTheScripts::GetTextByKeyFromScript(&m_nIp);
		CollectParameters(&m_nIp, 8);
		CDarkel::StartFrenzy((eWeaponType)ScriptParams[0], ScriptParams[1], ScriptParams[2],
			ScriptParams[3], text, ScriptParams[4], ScriptParams[5],
			ScriptParams[6], ScriptParams[7] != 0, true);
		return 0;
	}
	case COMMAND_ACTIVATE_MILITARY_CRANE:
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
			DEGTORAD(*(float*)&ScriptParams[9]), false, true,
			*(float*)&ScriptParams[0], *(float*)&ScriptParams[1]);
		return 0;
	}
	case COMMAND_WARP_PLAYER_INTO_CAR:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CWorld::Players[ScriptParams[0]].m_pPed;
		script_assert(pPed);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[1]);
		script_assert(pVehicle);
		pPed->SetObjective(OBJECTIVE_ENTER_CAR_AS_DRIVER, pVehicle);
		pPed->WarpPedIntoCar(pVehicle);
		return 0;
	}
	case COMMAND_WARP_CHAR_INTO_CAR:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[1]);
		script_assert(pVehicle);
		pPed->SetObjective(OBJECTIVE_ENTER_CAR_AS_DRIVER, pVehicle);
		pPed->WarpPedIntoCar(pVehicle);
		return 0;
	}
	//case COMMAND_SWITCH_CAR_RADIO:
	//case COMMAND_SET_AUDIO_STREAM:
	case COMMAND_PRINT_WITH_2_NUMBERS_BIG:
	{
		wchar* text = CTheScripts::GetTextByKeyFromScript(&m_nIp);
		CollectParameters(&m_nIp, 4);
		CMessages::AddBigMessageWithNumber(text, ScriptParams[2], ScriptParams[3] - 1, ScriptParams[0], ScriptParams[1], -1, -1, -1, -1);
		return 0;
	}
	case COMMAND_PRINT_WITH_3_NUMBERS_BIG:
	{
		wchar* text = CTheScripts::GetTextByKeyFromScript(&m_nIp);
		CollectParameters(&m_nIp, 5);
		CMessages::AddBigMessageWithNumber(text, ScriptParams[3], ScriptParams[4] - 1, ScriptParams[0], ScriptParams[1], ScriptParams[2], -1, -1, -1);
		return 0;
	}
	case COMMAND_PRINT_WITH_4_NUMBERS_BIG:
	{
		wchar* text = CTheScripts::GetTextByKeyFromScript(&m_nIp);
		CollectParameters(&m_nIp, 6);
		CMessages::AddBigMessageWithNumber(text, ScriptParams[4], ScriptParams[5] - 1, ScriptParams[0], ScriptParams[1], ScriptParams[2], ScriptParams[3], -1, -1);
		return 0;
	}
	case COMMAND_PRINT_WITH_5_NUMBERS_BIG:
	{
		wchar* text = CTheScripts::GetTextByKeyFromScript(&m_nIp);
		CollectParameters(&m_nIp, 7);
		CMessages::AddBigMessageWithNumber(text, ScriptParams[5], ScriptParams[6] - 1, ScriptParams[0], ScriptParams[1], ScriptParams[2], ScriptParams[3], ScriptParams[4], -1);
		return 0;
	}
	case COMMAND_PRINT_WITH_6_NUMBERS_BIG:
	{
		wchar* text = CTheScripts::GetTextByKeyFromScript(&m_nIp);
		CollectParameters(&m_nIp, 8);
		CMessages::AddBigMessageWithNumber(text, ScriptParams[6], ScriptParams[7] - 1, ScriptParams[0], ScriptParams[1], ScriptParams[2], ScriptParams[3], ScriptParams[4], ScriptParams[5]);
		return 0;
	}
	case COMMAND_SET_CHAR_WAIT_STATE:
	{
		CollectParameters(&m_nIp, 3);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		pPed->SetWaitState((eWaitState)ScriptParams[1], ScriptParams[2] >= 0 ? &ScriptParams[2] : nil);
		return 0;
	}
	case COMMAND_SET_CAMERA_BEHIND_PLAYER:
		TheCamera.SetCameraDirectlyBehindForFollowPed_CamOnAString();
		return 0;
	case COMMAND_SET_MOTION_BLUR:
		CollectParameters(&m_nIp, 1);
		TheCamera.SetMotionBlur(0, 0, 0, 0, ScriptParams[0]);
		return 0;
	case COMMAND_PRINT_STRING_IN_STRING:
	{
		wchar* text = CTheScripts::GetTextByKeyFromScript(&m_nIp);
		wchar* string = CTheScripts::GetTextByKeyFromScript(&m_nIp);
		CollectParameters(&m_nIp, 2);
		CMessages::AddMessageWithString(text, ScriptParams[0], ScriptParams[1], string);
		return 0;
	}
	case COMMAND_CREATE_RANDOM_CHAR:
	{
		CollectParameters(&m_nIp, 3);
		CZoneInfo zoneinfo;
		CTheZones::GetZoneInfoForTimeOfDay(&CWorld::Players[CWorld::PlayerInFocus].GetPos(), &zoneinfo);
		int mi;
		ePedType pedtype = PEDTYPE_COP;
		int attempt = 0;
		while (pedtype != PEDTYPE_CIVMALE && pedtype != PEDTYPE_CIVFEMALE && attempt < 5) {
			mi = CPopulation::ChooseCivilianOccupation(zoneinfo.pedGroup);
			if (CModelInfo::GetModelInfo(mi)->GetRwObject())
				pedtype = ((CPedModelInfo*)(CModelInfo::GetModelInfo(mi)))->m_pedType;
			attempt++;
		}
		if (!CModelInfo::GetModelInfo(mi)->GetRwObject()) {
			mi = MI_MALE01;
			pedtype = ((CPedModelInfo*)(CModelInfo::GetModelInfo(mi)))->m_pedType;
		}
		CPed* ped = new CCivilianPed(pedtype, mi);
		ped->CharCreatedBy = MISSION_CHAR;
		ped->bRespondsToThreats = false;
		ped->bAllowMedicsToReviveMe = false;
		CVector pos = *(CVector*)&ScriptParams[0];
		if (pos.z <= MAP_Z_LOW_LIMIT)
			pos.z = CWorld::FindGroundZForCoord(pos.x, pos.y);
		pos.z += 1.0f;
		ped->SetPosition(pos);
		ped->SetOrientation(0.0f, 0.0f, 0.0f);
		CTheScripts::ClearSpaceForMissionEntity(pos, ped);
		CWorld::Add(ped);
		ped->m_nZoneLevel = CTheZones::GetLevelFromPosition(&pos);
		CPopulation::ms_nTotalMissionPeds++;
		ScriptParams[0] = CPools::GetPedPool()->GetIndex(ped);
		StoreParameters(&m_nIp, 1);
		if (m_bIsMissionScript)
			CTheScripts::MissionCleanUp.AddEntityToList(ScriptParams[0], CLEANUP_CHAR);
		return 0;
	}
	case COMMAND_SET_CHAR_OBJ_STEAL_ANY_CAR:
	{
		CollectParameters(&m_nIp, 1);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		pPed->bScriptObjectiveCompleted = false;
		pPed->SetObjective(OBJECTIVE_STEAL_ANY_CAR);
		return 0;
	}
	case COMMAND_SET_2_REPEATED_PHONE_MESSAGES:
	{
		CollectParameters(&m_nIp, 1);
		wchar* text1 = CTheScripts::GetTextByKeyFromScript(&m_nIp);
		wchar* text2 = CTheScripts::GetTextByKeyFromScript(&m_nIp);
		gPhoneInfo.SetPhoneMessage_Repeatedly(ScriptParams[0], text1, text2, nil, nil, nil, nil);
		return 0;
	}
	case COMMAND_SET_2_PHONE_MESSAGES:
	{
		CollectParameters(&m_nIp, 1);
		wchar* text1 = CTheScripts::GetTextByKeyFromScript(&m_nIp);
		wchar* text2 = CTheScripts::GetTextByKeyFromScript(&m_nIp);
		gPhoneInfo.SetPhoneMessage_JustOnce(ScriptParams[0], text1, text2, nil, nil, nil, nil);
		return 0;
	}
	case COMMAND_SET_3_REPEATED_PHONE_MESSAGES:
	{
		CollectParameters(&m_nIp, 1);
		wchar* text1 = CTheScripts::GetTextByKeyFromScript(&m_nIp);
		wchar* text2 = CTheScripts::GetTextByKeyFromScript(&m_nIp);
		wchar* text3 = CTheScripts::GetTextByKeyFromScript(&m_nIp);
		gPhoneInfo.SetPhoneMessage_Repeatedly(ScriptParams[0], text1, text2, text3, nil, nil, nil);
		return 0;
	}
	case COMMAND_SET_3_PHONE_MESSAGES:
	{
		CollectParameters(&m_nIp, 1);
		wchar* text1 = CTheScripts::GetTextByKeyFromScript(&m_nIp);
		wchar* text2 = CTheScripts::GetTextByKeyFromScript(&m_nIp);
		wchar* text3 = CTheScripts::GetTextByKeyFromScript(&m_nIp);
		gPhoneInfo.SetPhoneMessage_JustOnce(ScriptParams[0], text1, text2, text3, nil, nil, nil);
		return 0;
	}
	case COMMAND_SET_4_REPEATED_PHONE_MESSAGES:
	{
		CollectParameters(&m_nIp, 1);
		wchar* text1 = CTheScripts::GetTextByKeyFromScript(&m_nIp);
		wchar* text2 = CTheScripts::GetTextByKeyFromScript(&m_nIp);
		wchar* text3 = CTheScripts::GetTextByKeyFromScript(&m_nIp);
		wchar* text4 = CTheScripts::GetTextByKeyFromScript(&m_nIp);
		gPhoneInfo.SetPhoneMessage_Repeatedly(ScriptParams[0], text1, text2, text3, text4, nil, nil);
		return 0;
	}
	case COMMAND_SET_4_PHONE_MESSAGES:
	{
		CollectParameters(&m_nIp, 1);
		wchar* text1 = CTheScripts::GetTextByKeyFromScript(&m_nIp);
		wchar* text2 = CTheScripts::GetTextByKeyFromScript(&m_nIp);
		wchar* text3 = CTheScripts::GetTextByKeyFromScript(&m_nIp);
		wchar* text4 = CTheScripts::GetTextByKeyFromScript(&m_nIp);
		gPhoneInfo.SetPhoneMessage_JustOnce(ScriptParams[0], text1, text2, text3, text4, nil, nil);
		return 0;
	}
	case COMMAND_IS_SNIPER_BULLET_IN_AREA:
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
		UpdateCompareFlag(CBulletInfo::TestForSniperBullet(infX, supX, infY, supY, infZ, supZ));
		return 0;
	}
	case COMMAND_GIVE_PLAYER_DETONATOR:
		CGarages::GivePlayerDetonator();
		return 0;
#ifdef GTA_SCRIPT_COLLECTIVE
	case COMMAND_SET_COLL_OBJ_STEAL_ANY_CAR:
		CollectParameters(&m_nIp, 1);
		CTheScripts::SetObjectiveForAllPedsInCollective(ScriptParams[0], OBJECTIVE_STEAL_ANY_CAR);
		return 0;
#endif
	case COMMAND_SET_OBJECT_VELOCITY:
	{
		CollectParameters(&m_nIp, 4);
		CObject* pObject = CPools::GetObjectPool()->GetAt(ScriptParams[0]);
		script_assert(pObject);
		pObject->SetMoveSpeed(*(CVector*)&ScriptParams[1] * METERS_PER_SECOND_TO_GAME_SPEED);
		return 0;
	}
	case COMMAND_SET_OBJECT_COLLISION:
	{
		CollectParameters(&m_nIp, 2);
		CObject* pObject = CPools::GetObjectPool()->GetAt(ScriptParams[0]);
		script_assert(pObject);
		pObject->bUsesCollision = (ScriptParams[1] != 0);
		return 0;
	}
	case COMMAND_IS_ICECREAM_JINGLE_ON:
	{
		CollectParameters(&m_nIp, 1);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		script_assert(pVehicle);
		// Adding this check to correspond to command name.
		// All original game scripts always assume that the vehicle is actually Mr. Whoopee,
		// but maybe there are mods that use it as "is alarm activated"?
		script_assert(pVehicle->GetModelIndex() == MI_MRWHOOP);
		UpdateCompareFlag(pVehicle->m_bSirenOrAlarm);
		return 0;
	}
	default:
		script_assert(0);
	}
	return -1;
}

int8 CRunningScript::ProcessCommands900To999(int32 command)
{
	char str[52];
	char onscreen_str[KEY_LENGTH_IN_SCRIPT];
	switch (command) {
	case COMMAND_PRINT_STRING_IN_STRING_NOW:
	{
		wchar* source = CTheScripts::GetTextByKeyFromScript(&m_nIp);
		wchar* pstr = CTheScripts::GetTextByKeyFromScript(&m_nIp);
		CollectParameters(&m_nIp, 2);
		CMessages::AddMessageJumpQWithString(source, ScriptParams[0], ScriptParams[1], pstr);
		return 0;
	}
	//case COMMAND_PRINT_STRING_IN_STRING_SOON:
	case COMMAND_SET_5_REPEATED_PHONE_MESSAGES:
	{
		CollectParameters(&m_nIp, 1);
		wchar* text1 = CTheScripts::GetTextByKeyFromScript(&m_nIp);
		wchar* text2 = CTheScripts::GetTextByKeyFromScript(&m_nIp);
		wchar* text3 = CTheScripts::GetTextByKeyFromScript(&m_nIp);
		wchar* text4 = CTheScripts::GetTextByKeyFromScript(&m_nIp);
		wchar* text5 = CTheScripts::GetTextByKeyFromScript(&m_nIp);
		gPhoneInfo.SetPhoneMessage_Repeatedly(ScriptParams[0], text1, text2, text3, text4, text5, nil);
		return 0;
	}
	case COMMAND_SET_5_PHONE_MESSAGES:
	{
		CollectParameters(&m_nIp, 1);
		wchar* text1 = CTheScripts::GetTextByKeyFromScript(&m_nIp);
		wchar* text2 = CTheScripts::GetTextByKeyFromScript(&m_nIp);
		wchar* text3 = CTheScripts::GetTextByKeyFromScript(&m_nIp);
		wchar* text4 = CTheScripts::GetTextByKeyFromScript(&m_nIp);
		wchar* text5 = CTheScripts::GetTextByKeyFromScript(&m_nIp);
		gPhoneInfo.SetPhoneMessage_JustOnce(ScriptParams[0], text1, text2, text3, text4, text5, nil);
		return 0;
	}
	case COMMAND_SET_6_REPEATED_PHONE_MESSAGES:
	{
		CollectParameters(&m_nIp, 1);
		wchar* text1 = CTheScripts::GetTextByKeyFromScript(&m_nIp);
		wchar* text2 = CTheScripts::GetTextByKeyFromScript(&m_nIp);
		wchar* text3 = CTheScripts::GetTextByKeyFromScript(&m_nIp);
		wchar* text4 = CTheScripts::GetTextByKeyFromScript(&m_nIp);
		wchar* text5 = CTheScripts::GetTextByKeyFromScript(&m_nIp);
		wchar* text6 = CTheScripts::GetTextByKeyFromScript(&m_nIp);
		gPhoneInfo.SetPhoneMessage_Repeatedly(ScriptParams[0], text1, text2, text3, text4, text5, text6);
		return 0;
	}
	case COMMAND_SET_6_PHONE_MESSAGES:
	{
		CollectParameters(&m_nIp, 1);
		wchar* text1 = CTheScripts::GetTextByKeyFromScript(&m_nIp);
		wchar* text2 = CTheScripts::GetTextByKeyFromScript(&m_nIp);
		wchar* text3 = CTheScripts::GetTextByKeyFromScript(&m_nIp);
		wchar* text4 = CTheScripts::GetTextByKeyFromScript(&m_nIp);
		wchar* text5 = CTheScripts::GetTextByKeyFromScript(&m_nIp);
		wchar* text6 = CTheScripts::GetTextByKeyFromScript(&m_nIp);
		gPhoneInfo.SetPhoneMessage_JustOnce(ScriptParams[0], text1, text2, text3, text4, text5, text6);
		return 0;
	}
	case COMMAND_IS_POINT_OBSCURED_BY_A_MISSION_ENTITY:
	{
		CollectParameters(&m_nIp, 6);
		float infX = *(float*)&ScriptParams[0] - *(float*)&ScriptParams[3];
		float supX = *(float*)&ScriptParams[0] + *(float*)&ScriptParams[3];
		float infY = *(float*)&ScriptParams[1] - *(float*)&ScriptParams[4];
		float supY = *(float*)&ScriptParams[1] + *(float*)&ScriptParams[4];
		float infZ = *(float*)&ScriptParams[2] - *(float*)&ScriptParams[5];
		float supZ = *(float*)&ScriptParams[2] + *(float*)&ScriptParams[5];
		if (infX > supX) {
			float tmp = infX;
			infX = supX;
			supX = tmp;
		}
		if (infY > supY) {
			float tmp = infY;
			infY = supY;
			supY = tmp;
		}
		if (infZ > supZ) {
			float tmp = infZ;
			infZ = supZ;
			supZ = tmp;
		}
		int16 total;
		CWorld::FindMissionEntitiesIntersectingCube(CVector(infX, infY, infZ), CVector(supX, supY, supZ), &total, 2, nil, true, true, true);
		UpdateCompareFlag(total > 0);
		return 0;
	}
	case COMMAND_LOAD_ALL_MODELS_NOW:
		CTimer::Stop();
		CStreaming::LoadAllRequestedModels(false);
		CTimer::Update();
		return 0;
	case COMMAND_ADD_TO_OBJECT_VELOCITY:
	{
		CollectParameters(&m_nIp, 4);
		CObject* pObject = CPools::GetObjectPool()->GetAt(ScriptParams[0]);
		script_assert(pObject);
		pObject->SetMoveSpeed(pObject->GetMoveSpeed() + METERS_PER_SECOND_TO_GAME_SPEED * *(CVector*)&ScriptParams[1]);
		return 0;
	}
	case COMMAND_DRAW_SPRITE:
	{
		CollectParameters(&m_nIp, 9);
		CTheScripts::IntroRectangles[CTheScripts::NumberOfIntroRectanglesThisFrame].m_bIsUsed = true;
		CTheScripts::IntroRectangles[CTheScripts::NumberOfIntroRectanglesThisFrame].m_nTextureId = ScriptParams[0] - 1;
		CTheScripts::IntroRectangles[CTheScripts::NumberOfIntroRectanglesThisFrame].m_sRect = CRect(
			*(float*)&ScriptParams[1], *(float*)&ScriptParams[2], *(float*)&ScriptParams[1] + *(float*)&ScriptParams[3], *(float*)&ScriptParams[2] + *(float*)&ScriptParams[4]);
		CTheScripts::IntroRectangles[CTheScripts::NumberOfIntroRectanglesThisFrame].m_sColor = CRGBA(ScriptParams[5], ScriptParams[6], ScriptParams[7], ScriptParams[8]);
		CTheScripts::NumberOfIntroRectanglesThisFrame++;
		return 0;
	}
	case COMMAND_DRAW_RECT:
	{
		CollectParameters(&m_nIp, 8);
		CTheScripts::IntroRectangles[CTheScripts::NumberOfIntroRectanglesThisFrame].m_bIsUsed = true;
		CTheScripts::IntroRectangles[CTheScripts::NumberOfIntroRectanglesThisFrame].m_nTextureId = -1;
		CTheScripts::IntroRectangles[CTheScripts::NumberOfIntroRectanglesThisFrame].m_sRect = CRect(
			*(float*)&ScriptParams[0], *(float*)&ScriptParams[1], *(float*)&ScriptParams[0] + *(float*)&ScriptParams[2], *(float*)&ScriptParams[1] + *(float*)&ScriptParams[3]);
		CTheScripts::IntroRectangles[CTheScripts::NumberOfIntroRectanglesThisFrame].m_sColor = CRGBA(ScriptParams[4], ScriptParams[5], ScriptParams[6], ScriptParams[7]);
		CTheScripts::NumberOfIntroRectanglesThisFrame++;
		return 0;
	}
	case COMMAND_LOAD_SPRITE:
	{
		CollectParameters(&m_nIp, 1);
		strncpy(str, (char*)&CTheScripts::ScriptSpace[m_nIp], KEY_LENGTH_IN_SCRIPT);
		for (int i = 0; i < KEY_LENGTH_IN_SCRIPT; i++)
			str[i] = tolower(str[i]);
		m_nIp += KEY_LENGTH_IN_SCRIPT;
		int slot = CTxdStore::FindTxdSlot("script");
		CTxdStore::PushCurrentTxd();
		CTxdStore::SetCurrentTxd(slot);
		CTheScripts::ScriptSprites[ScriptParams[0] - 1].SetTexture(str);
		CTxdStore::PopCurrentTxd();
		return 0;
	}
	case COMMAND_LOAD_TEXTURE_DICTIONARY:
	{
		strcpy(str, "models\\");
		strncpy(str + sizeof("models\\"), (char*)&CTheScripts::ScriptSpace[m_nIp], KEY_LENGTH_IN_SCRIPT);
		strcat(str, ".txd");
		m_nIp += KEY_LENGTH_IN_SCRIPT;
		int slot = CTxdStore::FindTxdSlot("script");
		if (slot == -1)
			slot = CTxdStore::AddTxdSlot("script");
		CTxdStore::LoadTxd(slot, str);
		CTxdStore::AddRef(slot);
		return 0;
	}
	case COMMAND_REMOVE_TEXTURE_DICTIONARY:
	{
		for (int i = 0; i < ARRAY_SIZE(CTheScripts::ScriptSprites); i++)
			CTheScripts::ScriptSprites[i].Delete();
		CTxdStore::RemoveTxd(CTxdStore::FindTxdSlot("script"));
		return 0;
	}
	case COMMAND_SET_OBJECT_DYNAMIC:
	{
		CollectParameters(&m_nIp, 2);
		CObject* pObject = CPools::GetObjectPool()->GetAt(ScriptParams[0]);
		script_assert(pObject);
		if (ScriptParams[1]) {
			if (pObject->bIsStatic) {
				pObject->SetIsStatic(false);
				pObject->AddToMovingList();
			}
		}
		else {
			if (!pObject->bIsStatic) {
				pObject->SetIsStatic(true);
				pObject->RemoveFromMovingList();
			}
		}
		return 0;
	}
	case COMMAND_SET_CHAR_ANIM_SPEED:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		CAnimBlendAssociation* pAssoc = RpAnimBlendClumpGetFirstAssociation(pPed->GetClump());
		if (pAssoc)
			pAssoc->speed = *(float*)&ScriptParams[1];
		return 0;
	}
	case COMMAND_PLAY_MISSION_PASSED_TUNE:
	{
		CollectParameters(&m_nIp, 1);
		DMAudio.ChangeMusicMode(MUSICMODE_FRONTEND);
		DMAudio.PlayFrontEndTrack(ScriptParams[0] + STREAMED_SOUND_MISSION_COMPLETED - 1, 0);
		return 0;
	}
	case COMMAND_CLEAR_AREA:
	{
		CollectParameters(&m_nIp, 5);
		CVector pos = *(CVector*)&ScriptParams[0];
		if (pos.z <= MAP_Z_LOW_LIMIT)
			pos.z = CWorld::FindGroundZForCoord(pos.x, pos.y);
		CWorld::ClearExcitingStuffFromArea(pos, *(float*)&ScriptParams[3], ScriptParams[4]);
		return 0;
	}
	case COMMAND_FREEZE_ONSCREEN_TIMER:
		CollectParameters(&m_nIp, 1);
		CUserDisplay::OnscnTimer.m_bDisabled = ScriptParams[0] != 0;
		return 0;
	case COMMAND_SWITCH_CAR_SIREN:
	{
		CollectParameters(&m_nIp, 2);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		script_assert(pVehicle);
		pVehicle->m_bSirenOrAlarm = ScriptParams[1] != 0;
		return 0;
	}
	case COMMAND_SWITCH_PED_ROADS_ON_ANGLED:
	{
		CollectParameters(&m_nIp, 7);
		ThePaths.SwitchRoadsInAngledArea(*(float*)&ScriptParams[0], *(float*)&ScriptParams[1], *(float*)&ScriptParams[2],
			*(float*)&ScriptParams[3], *(float*)&ScriptParams[4], *(float*)&ScriptParams[5], *(float*)&ScriptParams[6], 0, 1);
		return 0;
	}
	case COMMAND_SWITCH_PED_ROADS_OFF_ANGLED:
		CollectParameters(&m_nIp, 7);
		ThePaths.SwitchRoadsInAngledArea(*(float*)&ScriptParams[0], *(float*)&ScriptParams[1], *(float*)&ScriptParams[2],
			*(float*)&ScriptParams[3], *(float*)&ScriptParams[4], *(float*)&ScriptParams[5], *(float*)&ScriptParams[6], 0, 0);
		return 0;
	case COMMAND_SWITCH_ROADS_ON_ANGLED:
		CollectParameters(&m_nIp, 7);
		ThePaths.SwitchRoadsInAngledArea(*(float*)&ScriptParams[0], *(float*)&ScriptParams[1], *(float*)&ScriptParams[2],
			*(float*)&ScriptParams[3], *(float*)&ScriptParams[4], *(float*)&ScriptParams[5], *(float*)&ScriptParams[6], 1, 1);
		return 0;
	case COMMAND_SWITCH_ROADS_OFF_ANGLED:
		CollectParameters(&m_nIp, 7);
		ThePaths.SwitchRoadsInAngledArea(*(float*)&ScriptParams[0], *(float*)&ScriptParams[1], *(float*)&ScriptParams[2],
			*(float*)&ScriptParams[3], *(float*)&ScriptParams[4], *(float*)&ScriptParams[5], *(float*)&ScriptParams[6], 1, 0);
		return 0;
	case COMMAND_SET_CAR_WATERTIGHT:
	{
		CollectParameters(&m_nIp, 2);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		script_assert(pVehicle);
		script_assert(pVehicle->m_vehType == VEHICLE_TYPE_CAR);
		CAutomobile* pCar = (CAutomobile*)pVehicle;
		pCar->bWaterTight = ScriptParams[1] != 0;
		return 0;
	}
	case COMMAND_ADD_MOVING_PARTICLE_EFFECT:
	{
		CollectParameters(&m_nIp, 12);
		CVector pos = *(CVector*)&ScriptParams[1];
		if (pos.z <= MAP_Z_LOW_LIMIT)
			pos.z = CWorld::FindGroundZForCoord(pos.x, pos.y);
		float size = Max(0.0f, *(float*)&ScriptParams[7]);
		eParticleObjectType type = (eParticleObjectType)ScriptParams[0];
		RwRGBA color;
		if (type == POBJECT_SMOKE_TRAIL){
			color.alpha = -1;
			color.red = ScriptParams[8];
			color.green = ScriptParams[9];
			color.blue = ScriptParams[10];
		}else{
			color.alpha = color.red = color.blue = color.green = 0;
		}
		CVector target = *(CVector*)&ScriptParams[4];
		CParticleObject::AddObject(type, pos, target, size, ScriptParams[11], color, 1);
		return 0;
	}
	case COMMAND_SET_CHAR_CANT_BE_DRAGGED_OUT:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		pPed->bDontDragMeOutCar = ScriptParams[1] != 0;
		return 0;
	}
	case COMMAND_TURN_CAR_TO_FACE_COORD:
	{
		CollectParameters(&m_nIp, 3);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		script_assert(pVehicle);
		const CVector& pos = pVehicle->GetPosition();
		float heading = CGeneral::GetATanOfXY(pos.x - *(float*)&ScriptParams[1], pos.y - *(float*)&ScriptParams[2]) + HALFPI;
		if (heading > TWOPI)
			heading -= TWOPI;
		pVehicle->SetHeading(heading);
		return 0;
	}
	case COMMAND_IS_CRANE_LIFTING_CAR:
	{
		CollectParameters(&m_nIp, 3);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[2]);
		UpdateCompareFlag(CCranes::IsThisCarPickedUp(*(float*)&ScriptParams[0], *(float*)&ScriptParams[1], pVehicle));
		return 0;
	}
	case COMMAND_DRAW_SPHERE:
	{
		CollectParameters(&m_nIp, 4);
		CVector pos = *(CVector*)&ScriptParams[0];
		if (pos.z <= MAP_Z_LOW_LIMIT)
			pos.z = CWorld::FindGroundZForCoord(pos.x, pos.y);
		C3dMarkers::PlaceMarkerSet((uintptr)this + m_nIp, MARKERTYPE_CYLINDER, pos, *(float*)&ScriptParams[3],
			SPHERE_MARKER_R, SPHERE_MARKER_G, SPHERE_MARKER_B, SPHERE_MARKER_A,
			SPHERE_MARKER_PULSE_PERIOD, SPHERE_MARKER_PULSE_FRACTION, 0);
		return 0;
	}
	case COMMAND_SET_CAR_STATUS:
	{
		CollectParameters(&m_nIp, 2);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		script_assert(pVehicle);
		pVehicle->SetStatus(ScriptParams[1]);
		return 0;
	}
	case COMMAND_IS_CHAR_MALE:
	{
		CollectParameters(&m_nIp, 1);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		UpdateCompareFlag(pPed->m_nPedType != PEDTYPE_CIVFEMALE && pPed->m_nPedType != PEDTYPE_PROSTITUTE);
		return 0;
	}
	case COMMAND_SCRIPT_NAME:
	{
		strncpy(str, (char*)&CTheScripts::ScriptSpace[m_nIp], KEY_LENGTH_IN_SCRIPT);
		for (int i = 0; i < KEY_LENGTH_IN_SCRIPT; i++)
			str[i] = tolower(str[i]);
		m_nIp += KEY_LENGTH_IN_SCRIPT;
		strncpy(m_abScriptName, str, KEY_LENGTH_IN_SCRIPT);
		return 0;
	}
	case COMMAND_CHANGE_GARAGE_TYPE_WITH_CAR_MODEL:
	{
		CollectParameters(&m_nIp, 3);
		CGarages::ChangeGarageType(ScriptParams[0], ScriptParams[1], ScriptParams[2]);
		return 0;
	}
	case COMMAND_FIND_DRUG_PLANE_COORDINATES:
		*(CVector*)&ScriptParams[0] = CPlane::FindDrugPlaneCoordinates();
		StoreParameters(&m_nIp, 3);
		return 0;
	case COMMAND_SAVE_INT_TO_DEBUG_FILE:
		// TODO: implement something here
		CollectParameters(&m_nIp, 1);
		return 0;
	case COMMAND_SAVE_FLOAT_TO_DEBUG_FILE:
		CollectParameters(&m_nIp, 1);
		return 0;
	case COMMAND_SAVE_NEWLINE_TO_DEBUG_FILE:
		return 0;
	case COMMAND_POLICE_RADIO_MESSAGE:
		CollectParameters(&m_nIp, 3);
		DMAudio.PlaySuspectLastSeen(*(float*)&ScriptParams[0], *(float*)&ScriptParams[1], *(float*)&ScriptParams[2]);
		return 0;
	case COMMAND_SET_CAR_STRONG:
	{
		CollectParameters(&m_nIp, 2);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		script_assert(pVehicle);
		pVehicle->bTakeLessDamage = ScriptParams[1] != 0;
		return 0;
	}
	case COMMAND_REMOVE_ROUTE:
		CollectParameters(&m_nIp, 1);
		CRouteNode::RemoveRoute(ScriptParams[0]);
		return 0;
	case COMMAND_SWITCH_RUBBISH:
		CollectParameters(&m_nIp, 1);
		CRubbish::SetVisibility(ScriptParams[0] != 0);;
		return 0;
	case COMMAND_REMOVE_PARTICLE_EFFECTS_IN_AREA:
	{
		CollectParameters(&m_nIp, 6);
		float x1 = *(float*)&ScriptParams[0];
		float y1 = *(float*)&ScriptParams[1];
		float z1 = *(float*)&ScriptParams[2];
		float x2 = *(float*)&ScriptParams[3];
		float y2 = *(float*)&ScriptParams[4];
		float z2 = *(float*)&ScriptParams[5];
		CParticleObject* tmp = CParticleObject::pCloseListHead;
		while (tmp) {
			CParticleObject* next = tmp->m_pNext;
			if (tmp->IsWithinArea(x1, y1, z1, x2, y2, z2))
				tmp->RemoveObject();
			tmp = next;
		}
		tmp = CParticleObject::pFarListHead;
		while (tmp) {
			CParticleObject* next = tmp->m_pNext;
			if (tmp->IsWithinArea(x1, y1, z1, x2, y2, z2))
				tmp->RemoveObject();
			tmp = next;
		}
		return 0;
	}
	case COMMAND_SWITCH_STREAMING:
		CollectParameters(&m_nIp, 1);
		CStreaming::ms_disableStreaming = ScriptParams[0] == 0;
		return 0;
	case COMMAND_IS_GARAGE_OPEN:
		CollectParameters(&m_nIp, 1);
		UpdateCompareFlag(CGarages::IsGarageOpen(ScriptParams[0]));
		return 0;
	case COMMAND_IS_GARAGE_CLOSED:
		CollectParameters(&m_nIp, 1);
		UpdateCompareFlag(CGarages::IsGarageClosed(ScriptParams[0]));
		return 0;
	case COMMAND_START_CATALINA_HELI:
		CHeli::StartCatalinaFlyBy();
		return 0;
	case COMMAND_CATALINA_HELI_TAKE_OFF:
		CHeli::CatalinaTakeOff();
		return 0;
	case COMMAND_REMOVE_CATALINA_HELI:
		CHeli::RemoveCatalinaHeli();
		return 0;
	case COMMAND_HAS_CATALINA_HELI_BEEN_SHOT_DOWN:
		UpdateCompareFlag(CHeli::HasCatalinaBeenShotDown());
		return 0;
	case COMMAND_SWAP_NEAREST_BUILDING_MODEL:
	{
		CollectParameters(&m_nIp, 6);
		CVector pos = *(CVector*)&ScriptParams[0];
		if (pos.z <= MAP_Z_LOW_LIMIT)
			pos.z = CWorld::FindGroundZForCoord(pos.x, pos.y);
		float radius = *(float*)&ScriptParams[3];
		int mi1 = ScriptParams[4] >= 0 ? ScriptParams[4] : CTheScripts::UsedObjectArray[-ScriptParams[4]].index;
		int mi2 = ScriptParams[5] >= 0 ? ScriptParams[5] : CTheScripts::UsedObjectArray[-ScriptParams[5]].index;
		int16 total;
		CEntity* apEntities[16];
		CWorld::FindObjectsOfTypeInRange(mi1, pos, radius, true, &total, 16, apEntities, true, false, false, false, false);
		if (total == 0)
			CWorld::FindObjectsOfTypeInRangeSectorList(mi1, CWorld::GetBigBuildingList(LEVEL_GENERIC), pos, radius, true, &total, 16, apEntities);
		if (total == 0)
			CWorld::FindObjectsOfTypeInRangeSectorList(mi1, CWorld::GetBigBuildingList(CTheZones::FindZoneForPoint(pos)), pos, radius, true, &total, 16, apEntities);
		CEntity* pClosestEntity = nil;
		float min_dist = 2.0f * radius;
		for (int i = 0; i < total; i++) {
			float dist = (apEntities[i]->GetPosition() - pos).Magnitude();
			if (dist < min_dist) {
				min_dist = dist;
				pClosestEntity = apEntities[i];
			}
		}
		if (!pClosestEntity) {
			printf("Failed to find building\n");
			return 0;
		}
		CBuilding* pReplacedBuilding = ((CBuilding*)pClosestEntity);
		pReplacedBuilding->ReplaceWithNewModel(mi2);
		CTheScripts::AddToBuildingSwapArray(pReplacedBuilding, mi1, mi2);
		return 0;
	}
	case COMMAND_SWITCH_WORLD_PROCESSING:
		CollectParameters(&m_nIp, 1);
		CWorld::bProcessCutsceneOnly = ScriptParams[0] == 0;
		return 0;
	case COMMAND_REMOVE_ALL_PLAYER_WEAPONS:
	{
		CollectParameters(&m_nIp, 1);
		CPed* pPed = CWorld::Players[ScriptParams[0]].m_pPed;
		script_assert(pPed);
		pPed->ClearWeapons();
		return 0;
	}
	case COMMAND_GRAB_CATALINA_HELI:
	{
		CHeli* pHeli = CHeli::FindPointerToCatalinasHeli();
		ScriptParams[0] = pHeli ? CPools::GetVehiclePool()->GetIndex(pHeli) : -1;
		StoreParameters(&m_nIp, 1);
		return 0;
	}
	case COMMAND_CLEAR_AREA_OF_CARS:
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
		CWorld::ClearCarsFromArea(infX, infY, infZ, supX, supY, supZ);
		return 0;
	}
	case COMMAND_SET_ROTATING_GARAGE_DOOR:
		CollectParameters(&m_nIp, 1);
		CGarages::SetGarageDoorToRotate(ScriptParams[0]);
		return 0;
	case COMMAND_ADD_SPHERE:
	{
		CollectParameters(&m_nIp, 4);
		CVector pos = *(CVector*)&ScriptParams[0];
		if (pos.z <= MAP_Z_LOW_LIMIT)
			pos.z = CWorld::FindGroundZForCoord(pos.x, pos.y);
		float radius = *(float*)&ScriptParams[3];
		CTheScripts::GetActualScriptSphereIndex(CollectNextParameterWithoutIncreasingPC(m_nIp));
		ScriptParams[0] = CTheScripts::AddScriptSphere((uintptr)this + m_nIp, pos, radius);
		StoreParameters(&m_nIp, 1);
		return 0;
	}
	case COMMAND_REMOVE_SPHERE:
		CollectParameters(&m_nIp, 1);
		CTheScripts::RemoveScriptSphere(ScriptParams[0]);
		return 0;
	case COMMAND_CATALINA_HELI_FLY_AWAY:
		CHeli::MakeCatalinaHeliFlyAway();
		return 0;
	case COMMAND_SET_EVERYONE_IGNORE_PLAYER:
	{
		CollectParameters(&m_nIp, 2);
		CPlayerPed* pPed = CWorld::Players[ScriptParams[0]].m_pPed;
		script_assert(pPed);
		if (ScriptParams[1]) {
			pPed->m_pWanted->m_bIgnoredByEveryone = true;
			CWorld::StopAllLawEnforcersInTheirTracks();
		}
		else {
			pPed->m_pWanted->m_bIgnoredByEveryone = false;
		}
		return 0;
	}
	case COMMAND_STORE_CAR_CHAR_IS_IN_NO_SAVE:
	{
		CollectParameters(&m_nIp, 1);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		CVehicle* pVehicle = pPed->bInVehicle ? pPed->m_pMyVehicle : nil;
		ScriptParams[0] = CPools::GetVehiclePool()->GetIndex(pVehicle);
		StoreParameters(&m_nIp, 1);
		return 0;
	}
	case COMMAND_STORE_CAR_PLAYER_IS_IN_NO_SAVE:
	{
		CollectParameters(&m_nIp, 1);
		CPed* pPed = CWorld::Players[ScriptParams[0]].m_pPed;
		script_assert(pPed);
		CVehicle* pVehicle = pPed->bInVehicle ? pPed->m_pMyVehicle : nil;
		ScriptParams[0] = CPools::GetVehiclePool()->GetIndex(pVehicle);
		StoreParameters(&m_nIp, 1);
		return 0;
	}
	case COMMAND_IS_PHONE_DISPLAYING_MESSAGE:
		CollectParameters(&m_nIp, 1);
		UpdateCompareFlag(gPhoneInfo.IsMessageBeingDisplayed(ScriptParams[0]));
		return 0;
	case COMMAND_DISPLAY_ONSCREEN_TIMER_WITH_STRING:
	{
		script_assert(CTheScripts::ScriptSpace[m_nIp++] == ARGUMENT_GLOBALVAR);
		uint16 var = CTheScripts::Read2BytesFromScript(&m_nIp);
		wchar* text = TheText.Get((char*)&CTheScripts::ScriptSpace[m_nIp]); // ???
		strncpy(onscreen_str, (char*)&CTheScripts::ScriptSpace[m_nIp], KEY_LENGTH_IN_SCRIPT);
		m_nIp += KEY_LENGTH_IN_SCRIPT;
		CUserDisplay::OnscnTimer.AddClock(var, onscreen_str);
		return 0;
	}
	case COMMAND_DISPLAY_ONSCREEN_COUNTER_WITH_STRING:
	{
		script_assert(CTheScripts::ScriptSpace[m_nIp++] == ARGUMENT_GLOBALVAR);
		uint16 var = CTheScripts::Read2BytesFromScript(&m_nIp);
		CollectParameters(&m_nIp, 1);
		wchar* text = TheText.Get((char*)&CTheScripts::ScriptSpace[m_nIp]); // ???
		strncpy(onscreen_str, (char*)&CTheScripts::ScriptSpace[m_nIp], KEY_LENGTH_IN_SCRIPT);
		m_nIp += KEY_LENGTH_IN_SCRIPT;
		CUserDisplay::OnscnTimer.AddCounter(var, ScriptParams[0], onscreen_str);
		return 0;
	}
	case COMMAND_CREATE_RANDOM_CAR_FOR_CAR_PARK:
	{
		CollectParameters(&m_nIp, 4);
		if (CCarCtrl::NumRandomCars >= 30)
			return 0;
		int attempts;
		int model = -1;
		int index = CGeneral::GetRandomNumberInRange(0, 50);
		for (attempts = 0; attempts < 50; attempts++) {
			if (model != -1)
				break;
			model = CStreaming::ms_vehiclesLoaded[index];
			if (model == -1)
				continue;
			// desperatly want to believe this was inlined :|
			CBaseModelInfo* pInfo = CModelInfo::GetModelInfo(model);
			script_assert(pInfo->GetModelType() == MITYPE_VEHICLE);
			CVehicleModelInfo* pVehicleInfo = (CVehicleModelInfo*)pInfo;
			if (pVehicleInfo->m_vehicleType == VEHICLE_TYPE_CAR) {
				switch (model) {
				case MI_LANDSTAL:
				case MI_LINERUN:
				case MI_FIRETRUCK:
				case MI_TRASH:
				case MI_STRETCH:
				case MI_MULE:
				case MI_AMBULAN:
				case MI_FBICAR:
				case MI_MRWHOOP:
				case MI_BFINJECT:
				case MI_CORPSE:
				case MI_POLICE:
				case MI_ENFORCER:
				case MI_SECURICA:
				case MI_PREDATOR:
				case MI_BUS:
				case MI_RHINO:
				case MI_BARRACKS:
				case MI_TRAIN:
				case MI_CHOPPER:
				case MI_DODO:
				case MI_COACH:
				case MI_RCBANDIT:
				case MI_BELLYUP:
				case MI_MRWONGS:
				case MI_MAFIA:
				case MI_YARDIE:
				case MI_YAKUZA:
				case MI_DIABLOS:
				case MI_COLUMB:
				case MI_HOODS:
				case MI_AIRTRAIN:
				case MI_DEADDODO:
				case MI_SPEEDER:
				case MI_REEFER:
				case MI_PANLANT:
				case MI_FLATBED:
				case MI_YANKEE:
				case MI_ESCAPE:
				case MI_BORGNINE:
				case MI_TOYZ:
				case MI_GHOST:
				case MI_MIAMI_RCBARON:
				case MI_MIAMI_RCRAIDER:
					model = -1;
					break;
				case MI_IDAHO:
				case MI_STINGER:
				case MI_PEREN:
				case MI_SENTINEL:
				case MI_PATRIOT:
				case MI_MANANA:
				case MI_INFERNUS:
				case MI_BLISTA:
				case MI_PONY:
				case MI_CHEETAH:
				case MI_MOONBEAM:
				case MI_ESPERANT:
				case MI_TAXI:
				case MI_KURUMA:
				case MI_BOBCAT:
				case MI_BANSHEE:
				case MI_CABBIE:
				case MI_STALLION:
				case MI_RUMPO:
				case 151:
				case 152:
				case 153:
					break;
				default:
					printf("CREATE_RANDOM_CAR_FOR_CAR_PARK - Unknown car model %d\n", CStreaming::ms_vehiclesLoaded[index]);
					model = -1;
					break;
				}
			}
			else
				model = -1;
			if (++index >= 50)
				index = 0;
		}
		if (model == -1)
			return 0;
		CVehicle* car;
		if (!CModelInfo::IsBikeModel(model))
			car = new CAutomobile(model, RANDOM_VEHICLE);
		CVector pos = *(CVector*)&ScriptParams[0];
		pos.z += car->GetDistanceFromCentreOfMassToBaseOfModel();
		car->SetPosition(pos);
		car->SetHeading(DEGTORAD(*(float*)&ScriptParams[3]));
		CTheScripts::ClearSpaceForMissionEntity(pos, car);
		car->SetStatus(STATUS_ABANDONED);
		car->bIsLocked = false;
		car->bIsCarParkVehicle = true;
		CCarCtrl::JoinCarWithRoadSystem(car);
		car->AutoPilot.m_nCarMission = MISSION_NONE;
		car->AutoPilot.m_nTempAction = TEMPACT_NONE;
		car->AutoPilot.m_nDrivingStyle = DRIVINGSTYLE_STOP_FOR_CARS;
		car->AutoPilot.m_nCruiseSpeed = car->AutoPilot.m_fMaxTrafficSpeed = 9.0f;
		car->AutoPilot.m_nCurrentLane = car->AutoPilot.m_nNextLane = 0;
		car->bEngineOn = false;
		car->m_nZoneLevel = CTheZones::GetLevelFromPosition(&pos);
		CWorld::Add(car);
		return 0;
	}
	case COMMAND_IS_COLLISION_IN_MEMORY:
		CollectParameters(&m_nIp, 1);
		UpdateCompareFlag(CCollision::ms_collisionInMemory == ScriptParams[0]);
		return 0;
	case COMMAND_SET_WANTED_MULTIPLIER:
		CollectParameters(&m_nIp, 1);
		FindPlayerPed()->m_pWanted->m_fCrimeSensitivity = *(float*)&ScriptParams[0];
		return 0;
	case COMMAND_SET_CAMERA_IN_FRONT_OF_PLAYER:
		TheCamera.SetCameraDirectlyInFrontForFollowPed_CamOnAString();
		return 0;
	case COMMAND_IS_CAR_VISIBLY_DAMAGED:
	{
		CollectParameters(&m_nIp, 1);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		script_assert(pVehicle);
		UpdateCompareFlag(pVehicle->bIsDamaged);
		return 0;
	}
	case COMMAND_DOES_OBJECT_EXIST:
		CollectParameters(&m_nIp, 1);
		UpdateCompareFlag(CPools::GetObjectPool()->GetAt(ScriptParams[0]));
		return 0;
	case COMMAND_LOAD_SCENE:
	{
		CollectParameters(&m_nIp, 3);
		CVector pos = *(CVector*)&ScriptParams[0];
		CTimer::Stop();
		CStreaming::LoadScene(pos);
		CTimer::Update();
		return 0;
	}
	case COMMAND_ADD_STUCK_CAR_CHECK:
	{
		CollectParameters(&m_nIp, 3);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		script_assert(pVehicle);
		CTheScripts::StuckCars.AddCarToCheck(ScriptParams[0], *(float*)&ScriptParams[1], ScriptParams[2]);
		return 0;
	}
	case COMMAND_REMOVE_STUCK_CAR_CHECK:
	{
		CollectParameters(&m_nIp, 1);
		CTheScripts::StuckCars.RemoveCarFromCheck(ScriptParams[0]);
		return 0;
	}
	case COMMAND_IS_CAR_STUCK:
		CollectParameters(&m_nIp, 1);
		UpdateCompareFlag(CTheScripts::StuckCars.HasCarBeenStuckForAWhile(ScriptParams[0]));
		return 0;
	case COMMAND_LOAD_MISSION_AUDIO:
		strncpy(str, (char*)&CTheScripts::ScriptSpace[m_nIp], KEY_LENGTH_IN_SCRIPT);
		for (int i = 0; i < KEY_LENGTH_IN_SCRIPT; i++)
			str[i] = tolower(str[i]);
		m_nIp += KEY_LENGTH_IN_SCRIPT;
		DMAudio.PreloadMissionAudio(str);
		return 0;
	case COMMAND_HAS_MISSION_AUDIO_LOADED:
		UpdateCompareFlag(DMAudio.GetMissionAudioLoadingStatus() == 1);
		return 0;
	case COMMAND_PLAY_MISSION_AUDIO:
		DMAudio.PlayLoadedMissionAudio();
		return 0;
	case COMMAND_HAS_MISSION_AUDIO_FINISHED:
		UpdateCompareFlag(DMAudio.IsMissionAudioSampleFinished());
		return 0;
	case COMMAND_GET_CLOSEST_CAR_NODE_WITH_HEADING:
	{
		CollectParameters(&m_nIp, 3);
		CVector pos = *(CVector*)&ScriptParams[0];
		if (pos.z <= MAP_Z_LOW_LIMIT)
			pos.z = CWorld::FindGroundZForCoord(pos.x, pos.y);
		int node = ThePaths.FindNodeClosestToCoors(pos, 0, 999999.9f, true, true);
		*(CVector*)&ScriptParams[0] = ThePaths.m_pathNodes[node].GetPosition();
		*(float*)&ScriptParams[3] = ThePaths.FindNodeOrientationForCarPlacement(node);
		StoreParameters(&m_nIp, 4);
		return 0;
	}
	case COMMAND_HAS_IMPORT_GARAGE_SLOT_BEEN_FILLED:
	{
		CollectParameters(&m_nIp, 2);
		UpdateCompareFlag(CGarages::HasImportExportGarageCollectedThisCar(ScriptParams[0], ScriptParams[1] - 1));
		return 0;
	}
	case COMMAND_CLEAR_THIS_PRINT:
	{
		wchar* text = CTheScripts::GetTextByKeyFromScript(&m_nIp);
		CMessages::ClearThisPrint(text);
		return 0;
	}
	case COMMAND_CLEAR_THIS_BIG_PRINT:
	{
		wchar* text = CTheScripts::GetTextByKeyFromScript(&m_nIp);
		CMessages::ClearThisBigPrint(text);
		return 0;
	}
	case COMMAND_SET_MISSION_AUDIO_POSITION:
	{
		CollectParameters(&m_nIp, 3);
		CVector pos = *(CVector*)&ScriptParams[0];
		DMAudio.SetMissionAudioLocation(pos.x, pos.y, pos.z);
		return 0;
	}
	case COMMAND_ACTIVATE_SAVE_MENU:
		FrontEndMenuManager.m_bSaveMenuActive = true;
		return 0;
	case COMMAND_HAS_SAVE_GAME_FINISHED:
		UpdateCompareFlag(!FrontEndMenuManager.m_bMenuActive);
		return 0;
	case COMMAND_NO_SPECIAL_CAMERA_FOR_THIS_GARAGE:
		CollectParameters(&m_nIp, 1);
		CGarages::SetLeaveCameraForThisGarage(ScriptParams[0]);
		return 0;
	case COMMAND_ADD_BLIP_FOR_PICKUP_OLD:
	{
		CollectParameters(&m_nIp, 3);
		CObject* pObject = CPickups::aPickUps[CPickups::GetActualPickupIndex(ScriptParams[0])].m_pObject;
		CRadar::GetActualBlipArrayIndex(CollectNextParameterWithoutIncreasingPC(m_nIp));
		ScriptParams[0] = CRadar::SetEntityBlip(BLIP_OBJECT, CPools::GetObjectPool()->GetIndex(pObject), ScriptParams[1], (eBlipDisplay)ScriptParams[2]);
		StoreParameters(&m_nIp, 1);
		return 0;
	}
	case COMMAND_ADD_BLIP_FOR_PICKUP:
	{
		CollectParameters(&m_nIp, 1);
		CObject* pObject = CPickups::aPickUps[CPickups::GetActualPickupIndex(ScriptParams[0])].m_pObject;
		CRadar::GetActualBlipArrayIndex(CollectNextParameterWithoutIncreasingPC(m_nIp));
		int handle = CRadar::SetEntityBlip(BLIP_OBJECT, CPools::GetObjectPool()->GetIndex(pObject), 6, BLIP_DISPLAY_BOTH);
		CRadar::ChangeBlipScale(handle, 3);
		ScriptParams[0] = handle;
		StoreParameters(&m_nIp, 1);
		return 0;
	}
	case COMMAND_ADD_SPRITE_BLIP_FOR_PICKUP:
	{
		CollectParameters(&m_nIp, 2);
		CObject* pObject = CPickups::aPickUps[CPickups::GetActualPickupIndex(ScriptParams[0])].m_pObject;
		CRadar::GetActualBlipArrayIndex(CollectNextParameterWithoutIncreasingPC(m_nIp));
		int handle = CRadar::SetEntityBlip(BLIP_OBJECT, CPools::GetObjectPool()->GetIndex(pObject), 6, BLIP_DISPLAY_BOTH);
		CRadar::SetBlipSprite(handle, ScriptParams[1]);
		ScriptParams[0] = handle;
		StoreParameters(&m_nIp, 1);
		return 0;
	}
	case COMMAND_SET_PED_DENSITY_MULTIPLIER:
		CollectParameters(&m_nIp, 1);
		CPopulation::PedDensityMultiplier = *(float*)&ScriptParams[0];
		return 0;
	case COMMAND_FORCE_RANDOM_PED_TYPE:
		CollectParameters(&m_nIp, 1);
		CPopulation::m_AllRandomPedsThisType = ScriptParams[0];
		return 0;
	case COMMAND_SET_TEXT_DRAW_BEFORE_FADE:
		CollectParameters(&m_nIp, 1);
		CTheScripts::IntroTextLines[CTheScripts::NumberOfIntroTextLinesThisFrame].m_bTextBeforeFade = ScriptParams[0] != 0;
		return 0;
	case COMMAND_GET_COLLECTABLE1S_COLLECTED:
		ScriptParams[0] = CWorld::Players[CWorld::PlayerInFocus].m_nCollectedPackages;
		StoreParameters(&m_nIp, 1);
		return 0;
	case COMMAND_REGISTER_EL_BURRO_TIME:
		CollectParameters(&m_nIp, 1);
		CStats::RegisterElBurroTime(ScriptParams[0]);
		return 0;
	case COMMAND_SET_SPRITES_DRAW_BEFORE_FADE:
		CollectParameters(&m_nIp, 1);
		CTheScripts::IntroRectangles[CTheScripts::NumberOfIntroRectanglesThisFrame].m_bBeforeFade = ScriptParams[0] != 0;
		return 0;
	case COMMAND_SET_TEXT_RIGHT_JUSTIFY:
		CollectParameters(&m_nIp, 1);
		CTheScripts::IntroTextLines[CTheScripts::NumberOfIntroTextLinesThisFrame].m_bRightJustify = ScriptParams[0] != 0;
		return 0;
	case COMMAND_PRINT_HELP:
	{
		if (CCamera::m_bUseMouse3rdPerson && (
			strcmp((char*)&CTheScripts::ScriptSpace[m_nIp], "HELP15") == 0 ||
			strcmp((char*)&CTheScripts::ScriptSpace[m_nIp], "GUN_2A") == 0 ||
			strcmp((char*)&CTheScripts::ScriptSpace[m_nIp], "GUN_3A") == 0 ||
			strcmp((char*)&CTheScripts::ScriptSpace[m_nIp], "GUN_4A") == 0)) {
			m_nIp += KEY_LENGTH_IN_SCRIPT;
			return 0;
		}
		wchar* text = CTheScripts::GetTextByKeyFromScript(&m_nIp);
		CHud::SetHelpMessage(text, false);
		return 0;
	}
	case COMMAND_CLEAR_HELP:
		CHud::SetHelpMessage(nil, false);
		return 0;
	case COMMAND_FLASH_HUD_OBJECT:
		CollectParameters(&m_nIp, 1);
		CHud::m_ItemToFlash = ScriptParams[0];
		return 0;
	default:
		script_assert(0);
	}
	return -1;
}

int32 CTheScripts::GetNewUniqueScriptSphereIndex(int32 index)
{
	if (ScriptSphereArray[index].m_Index >= UINT16_MAX - 1)
		ScriptSphereArray[index].m_Index = 1;
	else
		ScriptSphereArray[index].m_Index++;
	return (uint16)index | ScriptSphereArray[index].m_Index << 16;
}

int32 CTheScripts::GetActualScriptSphereIndex(int32 index)
{
	if (index == -1)
		return -1;
	uint16 check = (uint32)index >> 16;
	uint16 array_idx = index & (0xFFFF);
	script_assert(array_idx < ARRAY_SIZE(ScriptSphereArray));
	if (check != ScriptSphereArray[array_idx].m_Index)
		return -1;
	return array_idx;
}

void CTheScripts::DrawScriptSpheres()
{
	for (int i = 0; i < MAX_NUM_SCRIPT_SPHERES; i++) {
		if (ScriptSphereArray[i].m_bInUse)
			C3dMarkers::PlaceMarkerSet(ScriptSphereArray[i].m_Id, MARKERTYPE_CYLINDER, ScriptSphereArray[i].m_vecCenter, ScriptSphereArray[i].m_fRadius,
				SPHERE_MARKER_R, SPHERE_MARKER_G, SPHERE_MARKER_B, SPHERE_MARKER_A, SPHERE_MARKER_PULSE_PERIOD, SPHERE_MARKER_PULSE_FRACTION, 0);
	}
}

int32 CTheScripts::AddScriptSphere(int32 id, CVector pos, float radius)
{
	int16 i = 0;
	for (i = 0; i < MAX_NUM_SCRIPT_SPHERES; i++) {
		if (!ScriptSphereArray[i].m_bInUse)
			break;
	}
#ifdef FIX_BUGS
	if (i == MAX_NUM_SCRIPT_SPHERES)
		return -1;
#endif
	ScriptSphereArray[i].m_bInUse = true;
	ScriptSphereArray[i].m_Id = id;
	ScriptSphereArray[i].m_vecCenter = pos;
	ScriptSphereArray[i].m_fRadius = radius;
	return GetNewUniqueScriptSphereIndex(i);
}

void CTheScripts::RemoveScriptSphere(int32 index)
{
	index = GetActualScriptSphereIndex(index);
	if (index == -1)
		return;
	ScriptSphereArray[index].m_bInUse = false;
	ScriptSphereArray[index].m_Id = 0;
}

void CTheScripts::AddToBuildingSwapArray(CBuilding* pBuilding, int32 old_model, int32 new_model)
{
	int i = 0;
	bool found = false;
	while (i < MAX_NUM_BUILDING_SWAPS && !found) {
		if (BuildingSwapArray[i].m_pBuilding == pBuilding)
			found = true;
		else
			i++;
	}
	if (found) {
		if (BuildingSwapArray[i].m_nOldModel == new_model) {
			BuildingSwapArray[i].m_pBuilding = nil;
			BuildingSwapArray[i].m_nOldModel = BuildingSwapArray[i].m_nNewModel = -1;
		}
		else {
			BuildingSwapArray[i].m_nNewModel = new_model;
		}
	}
	else {
		i = 0;
		while (i < MAX_NUM_BUILDING_SWAPS && !found) {
			if (BuildingSwapArray[i].m_pBuilding == nil)
				found = true;
			else
				i++;
		}
		if (found) {
			BuildingSwapArray[i].m_pBuilding = pBuilding;
			BuildingSwapArray[i].m_nNewModel = new_model;
			BuildingSwapArray[i].m_nOldModel = old_model;
		}
	}
}

void CTheScripts::AddToInvisibilitySwapArray(CEntity* pEntity, bool remove)
{
	int i = 0;
	bool found = false;
	while (i < MAX_NUM_INVISIBILITY_SETTINGS && !found) {
		if (InvisibilitySettingArray[i] == pEntity)
			found = true;
		else
			i++;
	}
	if (found) {
		if (remove)
			InvisibilitySettingArray[i] = nil;
	}
	else if (!remove) {
		i = 0;
		while (i < MAX_NUM_INVISIBILITY_SETTINGS && !found) {
			if (InvisibilitySettingArray[i] == nil)
				found = true;
			else
				i++;
		}
		if (found)
			InvisibilitySettingArray[i] = pEntity;
	}
}

void CTheScripts::UndoBuildingSwaps()
{
	for (int i = 0; i < MAX_NUM_BUILDING_SWAPS; i++) {
		if (BuildingSwapArray[i].m_pBuilding) {
			BuildingSwapArray[i].m_pBuilding->ReplaceWithNewModel(BuildingSwapArray[i].m_nOldModel);
			BuildingSwapArray[i].m_pBuilding = nil;
			BuildingSwapArray[i].m_nOldModel = BuildingSwapArray[i].m_nNewModel = -1;
		}
	}
}

void CTheScripts::UndoEntityInvisibilitySettings()
{
	for (int i = 0; i < MAX_NUM_INVISIBILITY_SETTINGS; i++) {
		if (InvisibilitySettingArray[i]) {
			InvisibilitySettingArray[i]->bIsVisible = true;
			InvisibilitySettingArray[i] = nil;
		}
	}
}
