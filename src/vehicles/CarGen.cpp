#include "common.h"

#include "CarGen.h"

#include "Automobile.h"
#include "Boat.h"
#include "Camera.h"
#include "CarCtrl.h"
#include "CutsceneMgr.h"
#include "General.h"
#include "Pools.h"
#include "Streaming.h"
#include "Timer.h"
#include "Vehicle.h"
#include "VisibilityPlugins.h"
#include "World.h"
#include "Zones.h"

uint8 CTheCarGenerators::ProcessCounter;
uint32 CTheCarGenerators::NumOfCarGenerators;
CCarGenerator CTheCarGenerators::CarGeneratorArray[NUM_CARGENS];
uint8 CTheCarGenerators::GenerateEvenIfPlayerIsCloseCounter;
uint32 CTheCarGenerators::CurrentActiveCount;

void CCarGenerator::SwitchOff()
{
	m_nUsesRemaining = 0;
	--CTheCarGenerators::CurrentActiveCount;
}

void CCarGenerator::SwitchOn()
{
	m_nUsesRemaining = -1;
	m_nTimer = CalcNextGen();
	++CTheCarGenerators::CurrentActiveCount;
}

uint32 CCarGenerator::CalcNextGen()
{
	return CTimer::GetTimeInMilliseconds() + 4;
}

//TODO(MIAMI): check for more changes - so far only -1 mi is accounted for
void CCarGenerator::DoInternalProcessing()
{
	int mi;
	if (CCarCtrl::NumParkedCars >= 10)
		return;
	if (m_nModelIndex >= 0) {
		if (CheckForBlockage(m_nModelIndex)) {
			m_nTimer += 4;
			return;
		}
		mi = m_nModelIndex;
	}
	else {
		mi = -m_nModelIndex;
		if (m_nModelIndex == -1 || !CStreaming::HasModelLoaded(mi)) {
			CZoneInfo pZone;
			CVector pos = FindPlayerCoors();
			CTheZones::GetZoneInfoForTimeOfDay(&pos, &pZone);
			mi = CCarCtrl::ChooseCarModel(CCarCtrl::ChooseCarRating(&pZone));
			if (mi < 0)
				return;
			m_nModelIndex = -mi;
			m_nColor1 = -1;
			m_nColor2 = -1;
		}
		if (CheckForBlockage(mi)) {
			m_nTimer += 4;
			return;
		}
	}
	CStreaming::RequestModel(mi, STREAMFLAGS_DEPENDENCY);
	if (!CStreaming::HasModelLoaded(mi))
		return;
	CVehicle* pVehicle;
	if (CModelInfo::IsBoatModel(mi)){
		CBoat* pBoat = new CBoat(mi, PARKED_VEHICLE);
		pVehicle = pBoat;
		pBoat->bIsStatic = false;
		pBoat->bEngineOn = false;
		CVector pos = m_vecPos;
		if (pos.z <= -100.0f)
			pos.z = CWorld::FindGroundZForCoord(pos.x, pos.y);
		pos.z += pBoat->GetDistanceFromCentreOfMassToBaseOfModel();
		pBoat->SetPosition(pos);
		pBoat->SetOrientation(0.0f, 0.0f, DEGTORAD(m_fAngle));
		pBoat->SetStatus(STATUS_ABANDONED);
		pBoat->m_nDoorLock = CARLOCK_UNLOCKED;
	}else{ // TODO(MIAMI): bikes
		bool groundFound = false;
		CVector pos = m_vecPos;
		if (pos.z > -100.0f){
			pos.z = CWorld::FindGroundZFor3DCoord(pos.x, pos.y, pos.z, &groundFound);
		}else{
			CColPoint cp;
			CEntity* pEntity;
			groundFound = CWorld::ProcessVerticalLine(CVector(pos.x, pos.y, 1000.0f), -1000.0f,
				cp, pEntity, true, false, false, false, false, false, nil);
			if (groundFound)
				pos.z = cp.point.z;
		}
		if (!groundFound) {
			debug("CCarGenerator::DoInternalProcessing - can't find ground z for new car x = %f y = %f \n", m_vecPos.x, m_vecPos.y);
			return;
		}
		CAutomobile* pCar = new CAutomobile(mi, PARKED_VEHICLE);
		pVehicle = pCar;
		pCar->bIsStatic = false;
		pCar->bEngineOn = false;
		pos.z += pCar->GetDistanceFromCentreOfMassToBaseOfModel();
		pCar->SetPosition(pos);
		pCar->SetOrientation(0.0f, 0.0f, DEGTORAD(m_fAngle));
		pCar->SetStatus(STATUS_ABANDONED);
		pCar->bLightsOn = false;
		pCar->m_nDoorLock = CARLOCK_UNLOCKED;

	}
	CWorld::Add(pVehicle);
	if (CGeneral::GetRandomNumberInRange(0, 100) < m_nAlarm)
		pVehicle->m_nAlarmState = -1;
	if (CGeneral::GetRandomNumberInRange(0, 100) < m_nDoorlock)
		pVehicle->m_nDoorLock = CARLOCK_LOCKED;
	if (m_nColor1 != -1 && m_nColor2 != -1) {
		pVehicle->m_currentColour1 = m_nColor1;
		pVehicle->m_currentColour2 = m_nColor2;
	}
	else if (m_nModelIndex < -1) {
		m_nColor1 = pVehicle->m_currentColour1;
		m_nColor2 = pVehicle->m_currentColour2;
	}
	CVisibilityPlugins::SetClumpAlpha(pVehicle->GetClump(), 0);
	m_nVehicleHandle = CPools::GetVehiclePool()->GetIndex(pVehicle);
	if (m_nUsesRemaining < -1) /* I don't think this is a correct comparasion */
		--m_nUsesRemaining;
	m_nTimer = CalcNextGen();
	if (m_nUsesRemaining == 0)
		--CTheCarGenerators::CurrentActiveCount;
}

void CCarGenerator::Process()
{
	if (m_nVehicleHandle == -1 && 
		(CTheCarGenerators::GenerateEvenIfPlayerIsCloseCounter || CTimer::GetTimeInMilliseconds() >= m_nTimer) &&
		m_nUsesRemaining != 0 && CheckIfWithinRangeOfAnyPlayer())
		DoInternalProcessing();
	if (m_nVehicleHandle == -1)
		return;
	CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(m_nVehicleHandle);
	if (!pVehicle){
		m_nVehicleHandle = -1;
		return;
	}
	if (pVehicle->GetStatus() != STATUS_PLAYER)
		return;
	m_nTimer += 60000;
	m_nVehicleHandle = -1;
	m_bIsBlocking = true;
	pVehicle->bExtendedRange = false;
}

void CCarGenerator::Setup(float x, float y, float z, float angle, int32 mi, int16 color1, int16 color2, uint8 force, uint8 alarm, uint8 lock, uint16 min_delay, uint16 max_delay)
{
	CMatrix m1, m2, m3; /* Unused but present on stack, so I'll leave them. */
	m_vecPos = CVector(x, y, z);
	m_fAngle = angle;
	m_nModelIndex = mi;
	m_nColor1 = color1;
	m_nColor2 = color2;
	m_bForceSpawn = force;
	m_nAlarm = alarm;
	m_nDoorlock = lock;
	m_nMinDelay = min_delay;
	m_nMaxDelay = max_delay;
	m_nVehicleHandle = -1;
	m_nTimer = CTimer::GetTimeInMilliseconds() + 1;
	m_nUsesRemaining = 0;
	m_bIsBlocking = false;
}

bool CCarGenerator::CheckForBlockage(int32 mi)
{
	int16 entities;
	CEntity* pEntities[8];
	CColModel* pColModel = CModelInfo::GetModelInfo(mi)->GetColModel();
	CWorld::FindObjectsKindaColliding(CVector(m_vecPos), pColModel->boundingSphere.radius, 1, &entities, 8, pEntities, false, true, true, false, false);
	for (int i = 0; i < entities; i++) {
		if (m_vecPos.z + pColModel->boundingBox.min.z < pEntities[i]->GetPosition().z + pEntities[i]->GetColModel()->boundingBox.max.z + 1.0f &&
			m_vecPos.z + pColModel->boundingBox.max.z > pEntities[i]->GetPosition().z + pEntities[i]->GetColModel()->boundingBox.min.z - 1.0f) {
			m_bIsBlocking = true;
			return true;
		}
	}
	return false;
}

bool CCarGenerator::CheckIfWithinRangeOfAnyPlayer()
{
	CVector2D direction = FindPlayerCentreOfWorld(CWorld::PlayerInFocus) - m_vecPos;
	float distance = direction.Magnitude();
	float farclip = 110.0f * TheCamera.GenerationDistMultiplier;
	float nearclip = farclip - 20.0f;
	bool canBeRemoved = (m_nModelIndex > 0 && CModelInfo::IsBoatModel(m_nModelIndex) && 165.0f * TheCamera.GenerationDistMultiplier > distance &&
		TheCamera.IsPointVisible(m_vecPos, &TheCamera.GetCameraMatrix())); // TODO(MIAMI) COcclision::IsPositionOccluded(m_vecPos, 0.0f)
	if (distance >= farclip || canBeRemoved){
		if (m_bIsBlocking)
			m_bIsBlocking = false;
		return false;
	}
	if (CTheCarGenerators::GenerateEvenIfPlayerIsCloseCounter)
		return true;
	if (m_bIsBlocking)
		return false;
	if (distance < nearclip)
		return false;
	return DotProduct2D(direction, FindPlayerSpeed()) <= 0;
}

void CTheCarGenerators::Process()
{
	if (FindPlayerTrain() || CCutsceneMgr::IsCutsceneProcessing())
		return;
	if (++CTheCarGenerators::ProcessCounter == 4)
		CTheCarGenerators::ProcessCounter = 0;
	for (uint32 i = ProcessCounter; i < NumOfCarGenerators; i += 4)
		CTheCarGenerators::CarGeneratorArray[i].Process();
	if (GenerateEvenIfPlayerIsCloseCounter)
		GenerateEvenIfPlayerIsCloseCounter--;
}

int32 CTheCarGenerators::CreateCarGenerator(float x, float y, float z, float angle, int32 mi, int16 color1, int16 color2, uint8 force, uint8 alarm, uint8 lock, uint16 min_delay, uint16 max_delay)
{
	CarGeneratorArray[NumOfCarGenerators].Setup(x, y, z, angle, mi, color1, color2, force, alarm, lock, min_delay, max_delay);
	return NumOfCarGenerators++;
}

void CTheCarGenerators::Init()
{
	GenerateEvenIfPlayerIsCloseCounter = 0;
	NumOfCarGenerators = 0;
	ProcessCounter = 0;
	CurrentActiveCount = 0;
}

void CTheCarGenerators::SaveAllCarGenerators(uint8 *buffer, uint32 *size)
{
	const uint32 nGeneralDataSize = sizeof(NumOfCarGenerators) + sizeof(CurrentActiveCount) + sizeof(ProcessCounter) + sizeof(GenerateEvenIfPlayerIsCloseCounter) + sizeof(int16);
	*size = sizeof(int) + nGeneralDataSize + sizeof(uint32) + sizeof(CarGeneratorArray) + SAVE_HEADER_SIZE;
INITSAVEBUF
	WriteSaveHeader(buffer, 'C','G','N','\0', *size - SAVE_HEADER_SIZE);

	WriteSaveBuf(buffer, nGeneralDataSize);
	WriteSaveBuf(buffer, NumOfCarGenerators);
	WriteSaveBuf(buffer, CurrentActiveCount);
	WriteSaveBuf(buffer, ProcessCounter);
	WriteSaveBuf(buffer, GenerateEvenIfPlayerIsCloseCounter);
	WriteSaveBuf(buffer, (int16)0); // alignment
	WriteSaveBuf(buffer, sizeof(CarGeneratorArray));
	for (int i = 0; i < NUM_CARGENS; i++)
		WriteSaveBuf(buffer, CarGeneratorArray[i]);
VALIDATESAVEBUF(*size)
}

void CTheCarGenerators::LoadAllCarGenerators(uint8* buffer, uint32 size)
{
	const int32 nGeneralDataSize = sizeof(NumOfCarGenerators) + sizeof(CurrentActiveCount) + sizeof(ProcessCounter) + sizeof(GenerateEvenIfPlayerIsCloseCounter) + sizeof(int16);
	Init();
INITSAVEBUF
	CheckSaveHeader(buffer, 'C','G','N','\0', size - SAVE_HEADER_SIZE);
	assert(ReadSaveBuf<uint32>(buffer) == nGeneralDataSize);
	NumOfCarGenerators = ReadSaveBuf<uint32>(buffer);
	CurrentActiveCount = ReadSaveBuf<uint32>(buffer);
	ProcessCounter = ReadSaveBuf<uint8>(buffer);
	GenerateEvenIfPlayerIsCloseCounter = ReadSaveBuf<uint8>(buffer);
	ReadSaveBuf<int16>(buffer); // alignment
	assert(ReadSaveBuf<uint32>(buffer) == sizeof(CarGeneratorArray));
	for (int i = 0; i < NUM_CARGENS; i++) 
		CarGeneratorArray[i] = ReadSaveBuf<CCarGenerator>(buffer);
VALIDATESAVEBUF(size)
}
