#include "common.h"

#include "Record.h"

#include "FileMgr.h"
#include "Pad.h"
#include "Pools.h"
#include "Streaming.h"
#include "Timer.h"
#include "VehicleModelInfo.h"
#include "World.h"
#include "Frontend.h"

uint16 CRecordDataForGame::RecordingState;
uint8* CRecordDataForGame::pDataBuffer;
uint8* CRecordDataForGame::pDataBufferPointer;
int CRecordDataForGame::FId;
tGameBuffer CRecordDataForGame::pDataBufferForFrame;

#define MEMORY_FOR_GAME_RECORD (150000)

void CRecordDataForGame::Init(void)
{
	RecordingState = STATE_NONE;
	delete[] pDataBuffer;
	pDataBufferPointer = nil;
	pDataBuffer = nil;
#ifndef GTA_PS2 // this stuff is not present on PS2
	FId = CFileMgr::OpenFile("playback.dat", "r");
	if (FId <= 0) {
		if ((FId = CFileMgr::OpenFile("record.dat", "r")) <= 0)
			RecordingState = STATE_NONE;
		else {
			CFileMgr::CloseFile(FId);
			FId = CFileMgr::OpenFileForWriting("record.dat");
			RecordingState = STATE_RECORD;
		}
	}
	else {
		RecordingState = STATE_PLAYBACK;
	}
	if (RecordingState == STATE_PLAYBACK) {
		pDataBufferPointer = new uint8[MEMORY_FOR_GAME_RECORD];
		pDataBuffer = pDataBufferPointer;
		pDataBuffer[CFileMgr::Read(FId, (char*)pDataBufferPointer, MEMORY_FOR_GAME_RECORD) + 8] = (uint8)-1;
		CFileMgr::CloseFile(FId);
	}
#else
	RecordingState = STATE_NONE; // second time to make sure
#endif
}

void CRecordDataForGame::SaveOrRetrieveDataForThisFrame(void)
{
	switch (RecordingState) {
	case STATE_RECORD:
	{
		pDataBufferForFrame.m_fTimeStep = CTimer::GetTimeStep();
		pDataBufferForFrame.m_nTimeInMilliseconds = CTimer::GetTimeInMilliseconds();
		pDataBufferForFrame.m_nSizeOfPads[0] = 0;
		pDataBufferForFrame.m_nSizeOfPads[1] = 0;
		pDataBufferForFrame.m_nChecksum = CalcGameChecksum();
		uint8* pController1 = PackCurrentPadValues(pDataBufferForFrame.m_ControllerBuffer, &CPad::GetPad(0)->OldState, &CPad::GetPad(0)->NewState);
		pDataBufferForFrame.m_nSizeOfPads[0] = (pController1 - pDataBufferForFrame.m_ControllerBuffer) / 2;
		uint8* pController2 = PackCurrentPadValues(pController1, &CPad::GetPad(1)->OldState, &CPad::GetPad(1)->NewState);
		pDataBufferForFrame.m_nSizeOfPads[1] = (pController2 - pController1) / 2;
		uint8* pEndPtr = pController2;
		if ((pDataBufferForFrame.m_nSizeOfPads[0] + pDataBufferForFrame.m_nSizeOfPads[1]) & 1)
			pEndPtr += 2;
		CFileMgr::Write(FId, (char*)&pDataBufferForFrame, pEndPtr - (uint8*)&pDataBufferForFrame);
		break;
	}
	case STATE_PLAYBACK:
		if (pDataBufferPointer[8] == (uint8)-1)
			CPad::GetPad(0)->NewState.Clear();
		else {
			tGameBuffer* pData = (tGameBuffer*)pDataBufferPointer;
			CTimer::SetTimeInMilliseconds(pData->m_nTimeInMilliseconds);
			CTimer::SetTimeStep(pData->m_fTimeStep);
			uint8 size1 = pData->m_nSizeOfPads[0];
			uint8 size2 = pData->m_nSizeOfPads[1];
			pDataBufferPointer = (uint8*)&pData->m_ControllerBuffer;
			pDataBufferPointer = UnPackCurrentPadValues(pDataBufferPointer, size1, &CPad::GetPad(0)->NewState);
			pDataBufferPointer = UnPackCurrentPadValues(pDataBufferPointer, size2, &CPad::GetPad(1)->NewState);
			if ((size1 + size2) & 1)
				pDataBufferPointer += 2;
			if (pData->m_nChecksum != CalcGameChecksum())
				printf("Playback out of sync\n");
		}
	}
}

#define PROCESS_BUTTON_STATE_STORE(buf, os, ns, field, id) \
	do { \
		if (os->field != ns->field){ \
			*buf++ = id; \
			*buf++ = ns->field; \
		} \
	} while (0);

uint8* CRecordDataForGame::PackCurrentPadValues(uint8* buf, CControllerState* os, CControllerState* ns)
{
	PROCESS_BUTTON_STATE_STORE(buf, os, ns, LeftStickX, 0);
	PROCESS_BUTTON_STATE_STORE(buf, os, ns, LeftStickY, 1);
	PROCESS_BUTTON_STATE_STORE(buf, os, ns, RightStickX, 2);
	PROCESS_BUTTON_STATE_STORE(buf, os, ns, RightStickY, 3);
	PROCESS_BUTTON_STATE_STORE(buf, os, ns, LeftShoulder1, 4);
	PROCESS_BUTTON_STATE_STORE(buf, os, ns, LeftShoulder2, 5);
	PROCESS_BUTTON_STATE_STORE(buf, os, ns, RightShoulder1, 6);
	PROCESS_BUTTON_STATE_STORE(buf, os, ns, RightShoulder2, 7);
	PROCESS_BUTTON_STATE_STORE(buf, os, ns, DPadUp, 8);
	PROCESS_BUTTON_STATE_STORE(buf, os, ns, DPadDown, 9);
	PROCESS_BUTTON_STATE_STORE(buf, os, ns, DPadLeft, 10);
	PROCESS_BUTTON_STATE_STORE(buf, os, ns, DPadRight, 11);
	PROCESS_BUTTON_STATE_STORE(buf, os, ns, Start, 12);
	PROCESS_BUTTON_STATE_STORE(buf, os, ns, Select, 13);
	PROCESS_BUTTON_STATE_STORE(buf, os, ns, Square, 14);
	PROCESS_BUTTON_STATE_STORE(buf, os, ns, Triangle, 15);
	PROCESS_BUTTON_STATE_STORE(buf, os, ns, Cross, 16);
	PROCESS_BUTTON_STATE_STORE(buf, os, ns, Circle, 17);
	PROCESS_BUTTON_STATE_STORE(buf, os, ns, LeftShock, 18);
	PROCESS_BUTTON_STATE_STORE(buf, os, ns, RightShock, 19);
	return buf;
}
#undef PROCESS_BUTTON_STATE_STORE

#define PROCESS_BUTTON_STATE_RESTORE(buf, state, field, id) case id: state->field = *buf++; break;

uint8* CRecordDataForGame::UnPackCurrentPadValues(uint8* buf, uint8 total, CControllerState* state)
{
	for (uint8 i = 0; i < total; i++) {
		switch (*buf++) {
			PROCESS_BUTTON_STATE_RESTORE(buf, state, LeftStickX, 0);
			PROCESS_BUTTON_STATE_RESTORE(buf, state, LeftStickY, 1);
			PROCESS_BUTTON_STATE_RESTORE(buf, state, RightStickX, 2);
			PROCESS_BUTTON_STATE_RESTORE(buf, state, RightStickY, 3);
			PROCESS_BUTTON_STATE_RESTORE(buf, state, LeftShoulder1, 4);
			PROCESS_BUTTON_STATE_RESTORE(buf, state, LeftShoulder2, 5);
			PROCESS_BUTTON_STATE_RESTORE(buf, state, RightShoulder1, 6);
			PROCESS_BUTTON_STATE_RESTORE(buf, state, RightShoulder2, 7);
			PROCESS_BUTTON_STATE_RESTORE(buf, state, DPadUp, 8);
			PROCESS_BUTTON_STATE_RESTORE(buf, state, DPadDown, 9);
			PROCESS_BUTTON_STATE_RESTORE(buf, state, DPadLeft, 10);
			PROCESS_BUTTON_STATE_RESTORE(buf, state, DPadRight, 11);
			PROCESS_BUTTON_STATE_RESTORE(buf, state, Start, 12);
			PROCESS_BUTTON_STATE_RESTORE(buf, state, Select, 13);
			PROCESS_BUTTON_STATE_RESTORE(buf, state, Square, 14);
			PROCESS_BUTTON_STATE_RESTORE(buf, state, Triangle, 15);
			PROCESS_BUTTON_STATE_RESTORE(buf, state, Cross, 16);
			PROCESS_BUTTON_STATE_RESTORE(buf, state, Circle, 17);
			PROCESS_BUTTON_STATE_RESTORE(buf, state, LeftShock, 18);
			PROCESS_BUTTON_STATE_RESTORE(buf, state, RightShock, 19);
		}
	}
	return buf;
}

#undef PROCESS_BUTTON_STATE_RESTORE

uint16 CRecordDataForGame::CalcGameChecksum(void)
{
	uint32 checksum = 0;
	int i = CPools::GetPedPool()->GetSize();
	while (i--) {
		CPed* pPed = CPools::GetPedPool()->GetSlot(i);
		if (!pPed)
			continue;
		checksum ^= pPed->GetModelIndex() ^ *(uint32*)&pPed->GetPosition().z ^ *(uint32*)&pPed->GetPosition().y ^ *(uint32*)&pPed->GetPosition().x;
	}
	i = CPools::GetVehiclePool()->GetSize();
	while (i--) {
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetSlot(i);
		if (!pVehicle)
			continue;
		checksum ^= pVehicle->GetModelIndex() ^ *(uint32*)&pVehicle->GetPosition().z ^ *(uint32*)&pVehicle->GetPosition().y ^ *(uint32*)&pVehicle->GetPosition().x;
	}
	return checksum ^ checksum >> 16;
}

uint8 CRecordDataForChase::Status;
int CRecordDataForChase::PositionChanges;
uint8 CRecordDataForChase::CurrentCar;
CAutomobile* CRecordDataForChase::pChaseCars[NUM_CHASE_CARS];
uint32 CRecordDataForChase::AnimStartTime;
float CRecordDataForChase::AnimTime;
CCarStateEachFrame* CRecordDataForChase::pBaseMemForCar[NUM_CHASE_CARS];
float CRecordDataForChase::TimeMultiplier;
int CRecordDataForChase::FId2;

#define CHASE_SCENE_LENGTH_IN_SECONDS (80)
#define CHASE_SCENE_FRAMES_PER_SECOND (15) // skipping every second frame
#define CHASE_SCENE_FRAMES_IN_RECORDING (CHASE_SCENE_LENGTH_IN_SECONDS * CHASE_SCENE_FRAMES_PER_SECOND)
#define CHASE_SCENE_LENGTH_IN_FRAMES (CHASE_SCENE_FRAMES_IN_RECORDING * 2)

void CRecordDataForChase::Init(void)
{
	Status = STATE_NONE;
	PositionChanges = 0;
	CurrentCar = 0;
	for (int i = 0; i < NUM_CHASE_CARS; i++)
		pChaseCars[i] = nil;
	AnimStartTime = 0;
}

void CRecordDataForChase::SaveOrRetrieveDataForThisFrame(void)
{
	switch (Status) {
	case STATE_NONE:
		return;
	case STATE_RECORD:
	{
		if ((CTimer::GetFrameCounter() & 1) == 0)
			StoreInfoForCar(pChaseCars[CurrentCar], &pBaseMemForCar[CurrentCar][CTimer::GetFrameCounter() / 2]);
		if (CTimer::GetFrameCounter() < CHASE_SCENE_LENGTH_IN_FRAMES * 2)
			return;
		CFileMgr::SetDir("data\\paths");
		sprintf(gString, "chase%d.dat", CurrentCar);
		int fid = CFileMgr::OpenFileForWriting(gString);
		uint32 fs = CHASE_SCENE_LENGTH_IN_FRAMES * sizeof(CCarStateEachFrame);
		printf("FileSize:%d\n", fs);
		CFileMgr::Write(fid, (char*)pBaseMemForCar[CurrentCar], fs);
		CFileMgr::CloseFile(fid);
		CFileMgr::SetDir("");
		sprintf(gString, "car%d.max", CurrentCar);
		int fid2 = CFileMgr::OpenFileForWriting(gString);
		for (int i = 0; i < CHASE_SCENE_FRAMES_IN_RECORDING; i++) {
			// WTF? Was it ever used?
#ifdef FIX_BUGS
			CCarStateEachFrame* pState = pBaseMemForCar[CurrentCar];
#else
			CCarStateEachFrame* pState = (CCarStateEachFrame*)pChaseCars[CurrentCar];
#endif
			CVector right = CVector(pState->rightX, pState->rightY, pState->rightZ) / INT8_MAX;
			CVector forward = CVector(pState->forwardX, pState->forwardY, pState->forwardZ) / INT8_MAX;
			CVector up = CrossProduct(right, forward);
			sprintf(gString, "%f %f %f\n", pState->pos.x, pState->pos.y, pState->pos.z);
			CFileMgr::Write(fid2, gString, strlen(gString) - 1);
			sprintf(gString, "%f %f %f\n", right.x, right.y, right.z);
			CFileMgr::Write(fid2, gString, strlen(gString) - 1);
			sprintf(gString, "%f %f %f\n", forward.x, forward.y, forward.z);
			CFileMgr::Write(fid2, gString, strlen(gString) - 1);
			sprintf(gString, "%f %f %f\n", up.x, up.y, up.z);
			CFileMgr::Write(fid2, gString, strlen(gString) - 1);
		}
		CFileMgr::CloseFile(fid2);
	}
	case STATE_PLAYBACK:
	case STATE_PLAYBACK_BEFORE_RECORDING:
	case STATE_PLAYBACK_INIT:
		break;
	}
}

struct tCoors {
	CVector pos;
	float angle;
};

// I guess developer was filling this with actual data before running the game
tCoors NewCoorsForRecordedCars[7];

void CRecordDataForChase::SaveOrRetrieveCarPositions(void)
{
	switch (Status) {
	case STATE_NONE:
		return;
	case STATE_RECORD:
	case STATE_PLAYBACK_BEFORE_RECORDING:
		for (int i = 0; i < NUM_CHASE_CARS; i++) {
			if (i != CurrentCar && CTimer::GetFrameCounter()) {
				RestoreInfoForCar(pChaseCars[i], &pBaseMemForCar[i][CTimer::GetFrameCounter() / 2], false);
				pChaseCars[i]->GetMatrix().UpdateRW();
				pChaseCars[i]->UpdateRwFrame();
			}
		}
		if (Status == STATE_PLAYBACK_BEFORE_RECORDING && CTimer::GetFrameCounter()) {
			RestoreInfoForCar(pChaseCars[CurrentCar], &pBaseMemForCar[CurrentCar][CTimer::GetFrameCounter() / 2], false);
			pChaseCars[CurrentCar]->GetMatrix().UpdateRW();
			pChaseCars[CurrentCar]->UpdateRwFrame();
		}
		if (CPad::GetPad(0)->GetLeftShockJustDown() && CPad::GetPad(0)->GetRightShockJustDown()) {
			if (!CPad::GetPad(0)->GetRightShockJustDown()) {
				pChaseCars[CurrentCar]->SetPosition(NewCoorsForRecordedCars[PositionChanges].pos);
				pChaseCars[CurrentCar]->SetMoveSpeed(0.0f, 0.0f, 0.0f);
				pChaseCars[CurrentCar]->GetMatrix().SetRotateZOnly(DEGTORAD(NewCoorsForRecordedCars[PositionChanges].angle));
				++PositionChanges;
			}
			if (Status == STATE_PLAYBACK_BEFORE_RECORDING) {
				Status = STATE_RECORD;
				pChaseCars[CurrentCar]->SetStatus(STATUS_PLAYER);
			}
		}
		break;
	case STATE_PLAYBACK_INIT:
		Status = STATE_PLAYBACK;
		break;
	case STATE_PLAYBACK:
	{
		TimeMultiplier += CTimer::GetTimeStepNonClippedInSeconds();
		float EndOfFrameTime = CHASE_SCENE_FRAMES_PER_SECOND * Min(CHASE_SCENE_LENGTH_IN_SECONDS, TimeMultiplier);
		for (int i = 0; i < NUM_CHASE_CARS; i++) {
			if (!pBaseMemForCar[i])
				continue;
			if (!pChaseCars[i])
				continue;
			if (EndOfFrameTime < CHASE_SCENE_FRAMES_IN_RECORDING - 1) {
				int FlooredEOFTime = EndOfFrameTime;
				RestoreInfoForCar(pChaseCars[i], &pBaseMemForCar[i][FlooredEOFTime], false);
				CMatrix tmp;
				float dp = EndOfFrameTime - FlooredEOFTime;
				RestoreInfoForMatrix(tmp, &pBaseMemForCar[i][FlooredEOFTime + 1]);
				pChaseCars[i]->GetRight() += (tmp.GetRight() - pChaseCars[i]->GetRight()) * dp;
				pChaseCars[i]->GetForward() += (tmp.GetForward() - pChaseCars[i]->GetForward()) * dp;
				pChaseCars[i]->GetUp() += (tmp.GetUp() - pChaseCars[i]->GetUp()) * dp;
				pChaseCars[i]->GetMatrix().GetPosition() += (tmp.GetPosition() - pChaseCars[i]->GetPosition()) * dp;
			}
			else{
				RestoreInfoForCar(pChaseCars[i], &pBaseMemForCar[i][CHASE_SCENE_FRAMES_IN_RECORDING - 1], true);
				if (i == 0)
					pChaseCars[i]->GetMatrix().GetPosition().z += 0.2f;
			}
			pChaseCars[i]->GetMatrix().UpdateRW();
			pChaseCars[i]->UpdateRwFrame();
			pChaseCars[i]->RemoveAndAdd();
		}
		break;
	}
	}
}

void CRecordDataForChase::StoreInfoForCar(CAutomobile* pCar, CCarStateEachFrame* pState)
{
	pState->rightX = INT8_MAX * pCar->GetRight().x;
	pState->rightY = INT8_MAX * pCar->GetRight().y;
	pState->rightZ = INT8_MAX * pCar->GetRight().z;
	pState->forwardX = INT8_MAX * pCar->GetForward().x;
	pState->forwardY = INT8_MAX * pCar->GetForward().y;
	pState->forwardZ = INT8_MAX * pCar->GetForward().z;
	pState->pos = pCar->GetPosition();
	pState->velX = 0.5f * INT16_MAX * pCar->GetMoveSpeed().x;
	pState->velY = 0.5f * INT16_MAX * pCar->GetMoveSpeed().y;
	pState->velZ = 0.5f * INT16_MAX * pCar->GetMoveSpeed().z;
	pState->wheel = 20 * pCar->m_fSteerAngle;
	pState->gas = 100 * pCar->m_fGasPedal;
	pState->brake = 100 * pCar->m_fBrakePedal;
	pState->handbrake = pCar->bIsHandbrakeOn;
}

void CRecordDataForChase::RestoreInfoForMatrix(CMatrix& matrix, CCarStateEachFrame* pState)
{
	matrix.GetRight() = CVector(pState->rightX, pState->rightY, pState->rightZ) / INT8_MAX;
	matrix.GetForward() = CVector(pState->forwardX, pState->forwardY, pState->forwardZ) / INT8_MAX;
	matrix.GetUp() = CrossProduct(matrix.GetRight(), matrix.GetForward());
	matrix.GetPosition() = pState->pos;
}

void CRecordDataForChase::RestoreInfoForCar(CAutomobile* pCar, CCarStateEachFrame* pState, bool stop)
{
	CVector oldPos = pCar->GetPosition();
	RestoreInfoForMatrix(pCar->GetMatrix(), pState);
	pCar->SetMoveSpeed(CVector(pState->velX, pState->velY, pState->velZ) / INT16_MAX / 0.5f);
	pCar->SetTurnSpeed(0.0f, 0.0f, 0.0f);
	pCar->m_fSteerAngle = pState->wheel / 20.0f;
	pCar->m_fGasPedal = pState->gas / 100.0f;
	pCar->m_fBrakePedal = pState->brake / 100.0f;
	pCar->bIsHandbrakeOn = pState->handbrake;
	if ((oldPos - pCar->GetPosition()).Magnitude() > 15.0f) {
		if (pCar == pChaseCars[14]) {
			pCar->m_currentColour1 = 58;
			pCar->m_currentColour2 = 1;
		}
		else
			pCar->GetModelInfo()->ChooseVehicleColour(pCar->m_currentColour1, pCar->m_currentColour2);
	}
	pCar->m_fHealth = Min(pCar->m_fHealth, 500.0f);
	if (stop) {
		pCar->m_fGasPedal = 0.0f;
		pCar->m_fBrakePedal = 0.0f;
		pCar->SetMoveSpeed(0.0f, 0.0f, 0.0f);
		pCar->bIsHandbrakeOn = false;
	}
}

void CRecordDataForChase::ProcessControlCars(void)
{
	if (Status != STATE_PLAYBACK)
		return;
	for (int i = 0; i < NUM_CHASE_CARS; i++) {
		if (pChaseCars[i])
			pChaseCars[i]->ProcessControl();
	}
}

bool CRecordDataForChase::ShouldThisPadBeLeftAlone(uint8 pad)
{
	// may be wrong
	if (Status == STATE_PLAYBACK_INIT) // this is useless but ps2 def checks if it's STATE_PLAYBACK_INIT
		return false;

	if (Status == STATE_RECORD)
		return pad != 0;

	return false;
}

void CRecordDataForChase::GiveUsACar(int32 mi, CVector pos, float angle, CAutomobile** ppCar, uint8 colour1, uint8 colour2)
{
	CStreaming::RequestModel(mi, STREAMFLAGS_DEPENDENCY);
	CStreaming::LoadAllRequestedModels(false);
	if (!CStreaming::HasModelLoaded(mi))
		return;
	CAutomobile* pCar = new CAutomobile(mi, MISSION_VEHICLE);
	pCar->SetPosition(pos);
	pCar->SetStatus(STATUS_PLAYER_PLAYBACKFROMBUFFER);
	pCar->GetMatrix().SetRotateZOnly(DEGTORAD(angle));
	pCar->pDriver = nil;
	pCar->m_currentColour1 = colour1;
	pCar->m_currentColour2 = colour2;
	CWorld::Add(pCar);
	*ppCar = pCar;
}

void RemoveUnusedCollision(void)
{
	static const char* dontDeleteArray[] = {
		"rd_SrRoad2A50", "rd_SrRoad2A20", "rd_CrossRda1w22", "rd_CrossRda1rw22",
		"road_broadway02", "road_broadway01", "com_21way5", "com_21way50",
		"cm1waycrosscom", "com_21way20", "com_21way10", "road_broadway04",
		"com_rvroads52", "com_roadsrv", "com_roadkb23", "com_roadkb22"
	};
	for (int i = 0; i < ARRAY_SIZE(dontDeleteArray); i++)
		CModelInfo::GetModelInfo(dontDeleteArray[i], nil)->GetColModel()->level = LEVEL_GENERIC;
	CModelInfo::RemoveColModelsFromOtherLevels(LEVEL_GENERIC);
	for (int i = 0; i < ARRAY_SIZE(dontDeleteArray); i++)
		CModelInfo::GetModelInfo(dontDeleteArray[i], nil)->GetColModel()->level = LEVEL_COMMERCIAL;
}

void CRecordDataForChase::StartChaseScene(float startTime)
{
	char filename[28];
	SetUpCarsForChaseScene();
	Status = STATE_PLAYBACK;
	AnimTime = startTime;
	AnimStartTime = CTimer::GetTimeInMilliseconds();
#ifdef NO_ISLAND_LOADING
	if (CMenuManager::m_PrefsIslandLoading == CMenuManager::ISLAND_LOADING_LOW)
#endif
		RemoveUnusedCollision();
	CStreaming::RemoveIslandsNotUsed(LEVEL_SUBURBAN);
	CGame::TidyUpMemory(true, true);
	CStreaming::ImGonnaUseStreamingMemory();
	CFileMgr::SetDir("data\\paths");
	for (int i = 0; i < NUM_CHASE_CARS; i++) {
		if (!pChaseCars[i]) {
			pBaseMemForCar[i] = nil;
			continue;
		}
		sprintf(filename, "chase%d.dat", i);
		FId2 = CFileMgr::OpenFile(filename, "rb");
		if (FId2 <= 0) {
			pBaseMemForCar[i] = nil;
			continue;
		}
		pBaseMemForCar[i] = new CCarStateEachFrame[CHASE_SCENE_FRAMES_IN_RECORDING];
		for (int j = 0; j < CHASE_SCENE_FRAMES_IN_RECORDING; j++) {
			CFileMgr::Read(FId2, (char*)&pBaseMemForCar[i][j], sizeof(CCarStateEachFrame));
			CFileMgr::Seek(FId2, sizeof(CCarStateEachFrame), 1);
		}
		CFileMgr::CloseFile(FId2);
	}
	CFileMgr::SetDir("");
	CStreaming::IHaveUsedStreamingMemory();
	TimeMultiplier = 0.0f;
}

void CRecordDataForChase::CleanUpChaseScene(void)
{
	if (Status != STATE_PLAYBACK_INIT && Status != STATE_PLAYBACK)
		return;
	Status = STATE_NONE;
	CleanUpCarsForChaseScene();
	for (int i = 0; i < NUM_CHASE_CARS; i++) {
		if (pBaseMemForCar[i]) {
			delete[] pBaseMemForCar[i];
			pBaseMemForCar[i] = nil;
		}
	}
}

void CRecordDataForChase::SetUpCarsForChaseScene(void)
{
	GiveUsACar(MI_POLICE, CVector(273.54221f, -1167.1907f, 24.880601f), 63.0f, &pChaseCars[0], 2, 1);
	GiveUsACar(MI_ENFORCER, CVector(231.1783f, -1388.8322f, 25.978201f), 90.0f, &pChaseCars[1], 2, 1);
	GiveUsACar(MI_TAXI, CVector(184.3156f, -1473.251f, 25.978201f), 0.0f, &pChaseCars[4], 6, 6);
	GiveUsACar(MI_CHEETAH, CVector(173.8868f, -1377.6514f, 25.978201f), 0.0f, &pChaseCars[6], 4, 5);
	GiveUsACar(MI_STINGER, CVector(102.5946f, -943.93628f, 25.9781f), 270.0f, &pChaseCars[7], 53, 53);
	GiveUsACar(MI_CHEETAH, CVector(-177.7157f, -862.18652f, 25.978201f), 155.0f, &pChaseCars[10], 41, 1);
	GiveUsACar(MI_STINGER, CVector(-170.56979f, -889.02362f, 25.978201f), 154.0f, &pChaseCars[11], 10, 10);
	GiveUsACar(MI_KURUMA, CVector(402.60809f, -917.49628f, 37.381001f), 90.0f, &pChaseCars[14], 34, 1);
	GiveUsACar(MI_TAXI, CVector(-33.496201f, -938.4563f, 25.9781f), 266.0f, &pChaseCars[16], 6, 6);
	GiveUsACar(MI_KURUMA, CVector(49.363098f, -987.60498f, 25.9781f), 0.0f, &pChaseCars[18], 51, 1);
	GiveUsACar(MI_TAXI, CVector(179.0049f, -1154.6686f, 25.9781f), 0.0f, &pChaseCars[19], 6, 76);
	GiveUsACar(MI_RUMPO, CVector(-28.9762f, -1031.3367f, 25.990601f), 242.0f, &pChaseCars[2], 1, 75);
	GiveUsACar(MI_PATRIOT, CVector(114.1564f, -796.69379f, 24.978201f), 180.0f, &pChaseCars[3], 0, 0);
}

void CRecordDataForChase::CleanUpCarsForChaseScene(void)
{
	for (int i = 0; i < NUM_CHASE_CARS; i++)
		RemoveCarFromChase(i);
}

void CRecordDataForChase::RemoveCarFromChase(int32 i)
{
	if (!pChaseCars[i])
		return;
	CWorld::Remove(pChaseCars[i]);
	delete pChaseCars[i];
	pChaseCars[i] = nil;
}

CVehicle* CRecordDataForChase::TurnChaseCarIntoScriptCar(int32 i)
{
	CVehicle* pVehicle = pChaseCars[i];
	pChaseCars[i] = nil;
	pVehicle->SetStatus(STATUS_PHYSICS);
	return pVehicle;
}

