#include "common.h"

#include "Pools.h"

#include "Boat.h"
#include "CarCtrl.h"
#ifdef MISSION_REPLAY
#include "GenericGameStorage.h"
#endif
#include "Population.h"
#include "ProjectileInfo.h"
#include "Streaming.h"
#include "Wanted.h"
#include "World.h"
#include "MemoryHeap.h"

CCPtrNodePool *CPools::ms_pPtrNodePool;
CEntryInfoNodePool *CPools::ms_pEntryInfoNodePool;
CPedPool *CPools::ms_pPedPool;
CVehiclePool *CPools::ms_pVehiclePool;
CBuildingPool *CPools::ms_pBuildingPool;
CTreadablePool *CPools::ms_pTreadablePool;
CObjectPool *CPools::ms_pObjectPool;
CDummyPool *CPools::ms_pDummyPool;
CAudioScriptObjectPool *CPools::ms_pAudioScriptObjectPool;

#ifdef GTA_PS2	// or USE_CUSTOM_ALLOCATOR
#define CHECKMEM(msg) CMemCheck::AllocateMemCheckBlock(msg)
#else
#define CHECKMEM(msg)
#endif

void
CPools::Initialise(void)
{
	PUSH_MEMID(MEMID_POOLS);
	CHECKMEM("before pools");
	ms_pPtrNodePool = new CCPtrNodePool(NUMPTRNODES);
	CHECKMEM("after CPtrNodePool");
	ms_pEntryInfoNodePool = new CEntryInfoNodePool(NUMENTRYINFOS);
	CHECKMEM("after CEntryInfoNodePool");
	ms_pPedPool = new CPedPool(NUMPEDS);
	CHECKMEM("after CPedPool");
	ms_pVehiclePool = new CVehiclePool(NUMVEHICLES);
	CHECKMEM("after CVehiclePool");
	ms_pBuildingPool = new CBuildingPool(NUMBUILDINGS);
	CHECKMEM("after CBuildingPool");
	ms_pTreadablePool = new CTreadablePool(NUMTREADABLES);
	CHECKMEM("after CTreadablePool");
	ms_pObjectPool = new CObjectPool(NUMOBJECTS);
	CHECKMEM("after CObjectPool");
	ms_pDummyPool = new CDummyPool(NUMDUMMIES);
	CHECKMEM("after CDummyPool");
	ms_pAudioScriptObjectPool = new CAudioScriptObjectPool(NUMAUDIOSCRIPTOBJECTS);
	CHECKMEM("after pools");
	POP_MEMID();
}

void
CPools::ShutDown(void)
{
	debug("PtrNodes left %d\n", ms_pPtrNodePool->GetNoOfUsedSpaces());
	debug("EntryInfoNodes left %d\n", ms_pEntryInfoNodePool->GetNoOfUsedSpaces());
	debug("Peds left %d\n", ms_pPedPool->GetNoOfUsedSpaces());
	debug("Vehicles left %d\n", ms_pVehiclePool->GetNoOfUsedSpaces());
	debug("Buildings left %d\n", ms_pBuildingPool->GetNoOfUsedSpaces());
	debug("Treadables left %d\n", ms_pTreadablePool->GetNoOfUsedSpaces());
	debug("Objects left %d\n", ms_pObjectPool->GetNoOfUsedSpaces());
	debug("Dummys left %d\n", ms_pDummyPool->GetNoOfUsedSpaces());
	debug("AudioScriptObjects left %d\n", ms_pAudioScriptObjectPool->GetNoOfUsedSpaces());
	printf("Shutdown pool started\n");

	delete ms_pPtrNodePool;
	delete ms_pEntryInfoNodePool;
	delete ms_pPedPool;
	delete ms_pVehiclePool;
	delete ms_pBuildingPool;
	delete ms_pTreadablePool;
	delete ms_pObjectPool;
	delete ms_pDummyPool;
	delete ms_pAudioScriptObjectPool;

	printf("Shutdown pool done\n");
}

int32 CPools::GetPedRef(CPed *ped) { return ms_pPedPool->GetIndex(ped); }
CPed *CPools::GetPed(int32 handle) { return ms_pPedPool->GetAt(handle); }
int32 CPools::GetVehicleRef(CVehicle *vehicle) { return ms_pVehiclePool->GetIndex(vehicle); }
CVehicle *CPools::GetVehicle(int32 handle) { return ms_pVehiclePool->GetAt(handle); }
int32 CPools::GetObjectRef(CObject *object) { return ms_pObjectPool->GetIndex(object); }
CObject *CPools::GetObject(int32 handle) { return ms_pObjectPool->GetAt(handle); }

void
CPools::CheckPoolsEmpty()
{
	assert(ms_pPedPool->GetNoOfUsedSpaces() == 0);
	assert(ms_pVehiclePool->GetNoOfUsedSpaces() == 0);
	printf("pools have been cleared\n");
}


void
CPools::MakeSureSlotInObjectPoolIsEmpty(int32 slot)
{
	if (ms_pObjectPool->GetIsFree(slot)) return;

	CObject *object = ms_pObjectPool->GetSlot(slot);
	if (object->ObjectCreatedBy == TEMP_OBJECT) {
		CWorld::Remove(object);
		delete object;
	} else if (!CProjectileInfo::RemoveIfThisIsAProjectile(object)) {
		// relocate to another slot??
		CObject *newObject = new CObject(object->GetModelIndex(), false);
		CWorld::Remove(object);
#if 0 // todo better
		*newObject = *object;
#else
		memcpy(newObject, object, ms_pObjectPool->GetMaxEntrySize());
#endif
		CWorld::Add(newObject);
		object->m_rwObject = nil;
		delete object;
		newObject->m_pFirstReference = nil;
	}
}

#define CopyFromBuf(buf, data) memcpy(&data, buf, sizeof(data)); SkipSaveBuf(buf, sizeof(data));
#define CopyToBuf(buf, data) memcpy(buf, &data, sizeof(data)); SkipSaveBuf(buf, sizeof(data));

void CPools::LoadVehiclePool(uint8* buf, uint32 size)
{
INITSAVEBUF
	int nNumCars = ReadSaveBuf<int>(buf);
	int nNumBoats = ReadSaveBuf<int>(buf);
	for (int i = 0; i < nNumCars + nNumBoats; i++) {
		uint32 type = ReadSaveBuf<uint32>(buf);
		int16 model = ReadSaveBuf<int16>(buf);
		CStreaming::RequestModel(model, STREAMFLAGS_DEPENDENCY);
		CStreaming::LoadAllRequestedModels(false);
		int32 slot = ReadSaveBuf<int32>(buf);
		CVehicle* pVehicle;
#ifdef COMPATIBLE_SAVES
		if (type == VEHICLE_TYPE_BOAT)
			pVehicle = new(slot) CBoat(model, RANDOM_VEHICLE);
		else if (type == VEHICLE_TYPE_CAR)
			pVehicle = new(slot) CAutomobile(model, RANDOM_VEHICLE);
		else
			assert(0);
		--CCarCtrl::NumRandomCars;
		pVehicle->Load(buf);
		CWorld::Add(pVehicle);
#else
		char* vbuf = new char[Max(CAutomobile::nSaveStructSize, CBoat::nSaveStructSize)];
		if (type == VEHICLE_TYPE_BOAT) {
			memcpy(vbuf, buf, sizeof(CBoat));
			SkipSaveBuf(buf, sizeof(CBoat));
			CBoat* pBoat = new(slot) CBoat(model, RANDOM_VEHICLE);
			pVehicle = pBoat;
			--CCarCtrl::NumRandomCars;
		}
		else if (type == VEHICLE_TYPE_CAR) {
			memcpy(vbuf, buf, sizeof(CAutomobile));
			SkipSaveBuf(buf, sizeof(CAutomobile));
			CStreaming::RequestModel(model, 0); // is it needed?
			CStreaming::LoadAllRequestedModels(false);
			CAutomobile* pAutomobile = new(slot) CAutomobile(model, RANDOM_VEHICLE);
			pVehicle = pAutomobile;
			CCarCtrl::NumRandomCars--; // why?
			pAutomobile->Damage = ((CAutomobile*)vbuf)->Damage;
			pAutomobile->SetupDamageAfterLoad();
		}
		else
			assert(0);
		CVehicle* pBufferVehicle = (CVehicle*)vbuf;
		pVehicle->GetMatrix() = pBufferVehicle->GetMatrix();
		pVehicle->VehicleCreatedBy = pBufferVehicle->VehicleCreatedBy;
		pVehicle->m_currentColour1 = pBufferVehicle->m_currentColour1;
		pVehicle->m_currentColour2 = pBufferVehicle->m_currentColour2;
		pVehicle->m_nAlarmState = pBufferVehicle->m_nAlarmState;
		pVehicle->m_nNumMaxPassengers = pBufferVehicle->m_nNumMaxPassengers;
		pVehicle->field_1D0[0] = pBufferVehicle->field_1D0[0];
		pVehicle->field_1D0[1] = pBufferVehicle->field_1D0[1];
		pVehicle->field_1D0[2] = pBufferVehicle->field_1D0[2];
		pVehicle->field_1D0[3] = pBufferVehicle->field_1D0[3];
		pVehicle->m_fSteerAngle = pBufferVehicle->m_fSteerAngle;
		pVehicle->m_fGasPedal = pBufferVehicle->m_fGasPedal;
		pVehicle->m_fBrakePedal = pBufferVehicle->m_fBrakePedal;
		pVehicle->bIsLawEnforcer = pBufferVehicle->bIsLawEnforcer;
		pVehicle->bIsLocked = pBufferVehicle->bIsLocked;
		pVehicle->bEngineOn = pBufferVehicle->bEngineOn;
		pVehicle->bIsHandbrakeOn = pBufferVehicle->bIsHandbrakeOn;
		pVehicle->bLightsOn = pBufferVehicle->bLightsOn;
		pVehicle->bFreebies = pBufferVehicle->bFreebies;
		pVehicle->m_fHealth = pBufferVehicle->m_fHealth;
		pVehicle->m_nCurrentGear = pBufferVehicle->m_nCurrentGear;
		pVehicle->m_fChangeGearTime = pBufferVehicle->m_fChangeGearTime;
		pVehicle->m_nTimeOfDeath = pBufferVehicle->m_nTimeOfDeath;
#ifdef FIX_BUGS //must be copypaste
		pVehicle->m_nBombTimer = pBufferVehicle->m_nBombTimer;
#else
		pVehicle->m_nTimeOfDeath = pBufferVehicle->m_nTimeOfDeath;
#endif
		pVehicle->m_nDoorLock = pBufferVehicle->m_nDoorLock;
		pVehicle->SetStatus(pBufferVehicle->GetStatus());
		pVehicle->SetType(pBufferVehicle->GetType());
		(pVehicle->GetAddressOfEntityProperties())[0] = (pBufferVehicle->GetAddressOfEntityProperties())[0];
		(pVehicle->GetAddressOfEntityProperties())[1] = (pBufferVehicle->GetAddressOfEntityProperties())[1];
		pVehicle->AutoPilot = pBufferVehicle->AutoPilot;
		CWorld::Add(pVehicle);
		delete[] vbuf;
#endif
	}
VALIDATESAVEBUF(size)
}

void CPools::SaveVehiclePool(uint8* buf, uint32* size)
{
INITSAVEBUF
	int nNumCars = 0;
	int nNumBoats = 0;
	int nPoolSize = GetVehiclePool()->GetSize();
	for (int i = 0; i < nPoolSize; i++) {
		CVehicle* pVehicle = GetVehiclePool()->GetSlot(i);
		if (!pVehicle)
			continue;
		bool bHasPassenger = false;
		for (int j = 0; j < ARRAY_SIZE(pVehicle->pPassengers); j++) {
			if (pVehicle->pPassengers[j])
				bHasPassenger = true;
		}
#ifdef MISSION_REPLAY
		bool bForceSaving = CWorld::Players[CWorld::PlayerInFocus].m_pPed->m_pMyVehicle == pVehicle && IsQuickSave;
#ifdef FIX_BUGS
		if ((!pVehicle->pDriver && !bHasPassenger) || bForceSaving) {
#else
		if (!pVehicle->pDriver && !bHasPassenger) {
#endif
			if (pVehicle->IsCar() && (pVehicle->VehicleCreatedBy == MISSION_VEHICLE || bForceSaving))
				++nNumCars;
			if (pVehicle->IsBoat() && (pVehicle->VehicleCreatedBy == MISSION_VEHICLE || bForceSaving))
				++nNumBoats;
#else
		if (!pVehicle->pDriver && !bHasPassenger) {
			if (pVehicle->IsCar() && pVehicle->VehicleCreatedBy == MISSION_VEHICLE)
				++nNumCars;
			if (pVehicle->IsBoat() && pVehicle->VehicleCreatedBy == MISSION_VEHICLE)
				++nNumBoats;
#endif
		}
	}
	*size = nNumCars * (sizeof(uint32) + sizeof(int16) + sizeof(int32) + CAutomobile::nSaveStructSize) + sizeof(int) +
		nNumBoats * (sizeof(uint32) + sizeof(int16) + sizeof(int32) + CBoat::nSaveStructSize) + sizeof(int);
	WriteSaveBuf(buf, nNumCars);
	WriteSaveBuf(buf, nNumBoats);
	for (int i = 0; i < nPoolSize; i++) {
		CVehicle* pVehicle = GetVehiclePool()->GetSlot(i);
		if (!pVehicle)
			continue;
		bool bHasPassenger = false;
		for (int j = 0; j < ARRAY_SIZE(pVehicle->pPassengers); j++) {
			if (pVehicle->pPassengers[j])
				bHasPassenger = true;
		}
#ifdef MISSION_REPLAY
		bool bForceSaving = CWorld::Players[CWorld::PlayerInFocus].m_pPed->m_pMyVehicle == pVehicle && IsQuickSave;
#endif
#if defined FIX_BUGS && defined MISSION_REPLAY
		if ((!pVehicle->pDriver && !bHasPassenger) || bForceSaving) {
#else
		if (!pVehicle->pDriver && !bHasPassenger) {
#endif
#ifdef COMPATIBLE_SAVES
#ifdef MISSION_REPLAY
			if ((pVehicle->IsCar() || pVehicle->IsBoat()) && (pVehicle->VehicleCreatedBy == MISSION_VEHICLE || bForceSaving)) {
#else
			if ((pVehicle->IsCar() || pVehicle->IsBoat()) && pVehicle->VehicleCreatedBy == MISSION_VEHICLE) {
#endif
				WriteSaveBuf<uint32>(buf, pVehicle->m_vehType);
				WriteSaveBuf<int16>(buf, pVehicle->GetModelIndex());
				WriteSaveBuf<int32>(buf, GetVehicleRef(pVehicle));
				pVehicle->Save(buf);
			}
#else
#ifdef MISSION_REPLAY
			if (pVehicle->IsCar() && (pVehicle->VehicleCreatedBy == MISSION_VEHICLE || bForceSaving)) {
#else
			if (pVehicle->IsCar() && pVehicle->VehicleCreatedBy == MISSION_VEHICLE) {
#endif
				WriteSaveBuf(buf, (uint32)pVehicle->m_vehType);
				WriteSaveBuf(buf, pVehicle->GetModelIndex());
				WriteSaveBuf(buf, GetVehicleRef(pVehicle));
				memcpy(buf, pVehicle, sizeof(CAutomobile));
				SkipSaveBuf(buf, sizeof(CAutomobile));
			}
#ifdef MISSION_REPLAY
			if (pVehicle->IsBoat() && (pVehicle->VehicleCreatedBy == MISSION_VEHICLE || bForceSaving)) {
#else
			if (pVehicle->IsBoat() && pVehicle->VehicleCreatedBy == MISSION_VEHICLE) {
#endif
				WriteSaveBuf(buf, (uint32)pVehicle->m_vehType);
				WriteSaveBuf(buf, pVehicle->GetModelIndex());
				WriteSaveBuf(buf, GetVehicleRef(pVehicle));
				memcpy(buf, pVehicle, sizeof(CBoat));
				SkipSaveBuf(buf, sizeof(CBoat));
			}
#endif
		}
	}
VALIDATESAVEBUF(*size)
}

void CPools::SaveObjectPool(uint8* buf, uint32* size)
{
INITSAVEBUF
	CProjectileInfo::RemoveAllProjectiles();
	CObject::DeleteAllTempObjects();
	int nObjects = 0;
	int nPoolSize = GetObjectPool()->GetSize();
	for (int i = 0; i < nPoolSize; i++) {
		CObject* pObject = GetObjectPool()->GetSlot(i);
		if (!pObject)
			continue;
		if (pObject->ObjectCreatedBy == MISSION_OBJECT)
			++nObjects;
	}
	*size = nObjects * (sizeof(int16) + sizeof(int) + sizeof(CCompressedMatrix) +
		sizeof(float) + sizeof(CCompressedMatrix) + sizeof(int8) + 7 * sizeof(bool) + sizeof(float) +
		sizeof(int8) + sizeof(int8) + sizeof(uint32) + 2 * sizeof(uint32)) + sizeof(int);
	CopyToBuf(buf, nObjects);
	for (int i = 0; i < nPoolSize; i++) {
		CObject* pObject = GetObjectPool()->GetSlot(i);
		if (!pObject)
			continue;
		if (pObject->ObjectCreatedBy == MISSION_OBJECT) {
			bool bIsPickup = pObject->bIsPickup;
			bool bPickupObjWithMessage = pObject->bPickupObjWithMessage;
			bool bOutOfStock = pObject->bOutOfStock;
			bool bGlassCracked = pObject->bGlassCracked;
			bool bGlassBroken = pObject->bGlassBroken;
			bool bHasBeenDamaged = pObject->bHasBeenDamaged;
			bool bUseVehicleColours = pObject->bUseVehicleColours;
			CCompressedMatrix tmp;
			CopyToBuf(buf, pObject->m_modelIndex);
			int32 ref = GetObjectRef(pObject);
			CopyToBuf(buf, ref);
			tmp.CompressFromFullMatrix(pObject->GetMatrix());
			CopyToBuf(buf, tmp);
			CopyToBuf(buf, pObject->m_fUprootLimit);
			tmp.CompressFromFullMatrix(pObject->m_objectMatrix);
			CopyToBuf(buf, tmp);
			CopyToBuf(buf, pObject->ObjectCreatedBy);
			CopyToBuf(buf, bIsPickup);
			CopyToBuf(buf, bPickupObjWithMessage);
			CopyToBuf(buf, bOutOfStock);
			CopyToBuf(buf, bGlassCracked);
			CopyToBuf(buf, bGlassBroken);
			CopyToBuf(buf, bHasBeenDamaged);
			CopyToBuf(buf, bUseVehicleColours);
			CopyToBuf(buf, pObject->m_fCollisionDamageMultiplier);
			CopyToBuf(buf, pObject->m_nCollisionDamageEffect);
			CopyToBuf(buf, pObject->m_nSpecialCollisionResponseCases);
			CopyToBuf(buf, pObject->m_nEndOfLifeTime);
#ifdef COMPATIBLE_SAVES
			pObject->SaveEntityFlags(buf);
#else
			CopyToBuf(buf, (pObject->GetAddressOfEntityProperties())[0]);
			CopyToBuf(buf, (pObject->GetAddressOfEntityProperties())[1]);
#endif
		}
	}
VALIDATESAVEBUF(*size)
}

void CPools::LoadObjectPool(uint8* buf, uint32 size)
{
INITSAVEBUF
	int nObjects;
	CopyFromBuf(buf, nObjects);
	for (int i = 0; i < nObjects; i++) {
		int16 mi;
		CopyFromBuf(buf, mi);
		int ref;
		CopyFromBuf(buf, ref);
		char* obuf = new char[sizeof(CObject)];
		CObject* pBufferObject = (CObject*)obuf;
		CCompressedMatrix tmp;
		CopyFromBuf(buf, tmp);
		tmp.DecompressIntoFullMatrix(pBufferObject->GetMatrix());
		CopyFromBuf(buf, pBufferObject->m_fUprootLimit);
		CopyFromBuf(buf, tmp);
		tmp.DecompressIntoFullMatrix(pBufferObject->m_objectMatrix);
		CopyFromBuf(buf, pBufferObject->ObjectCreatedBy);
		int8 bitFlag;
		CopyFromBuf(buf, bitFlag);
		pBufferObject->bIsPickup = bitFlag;
		CopyFromBuf(buf, bitFlag);
		pBufferObject->bPickupObjWithMessage = bitFlag;
		CopyFromBuf(buf, bitFlag);
		pBufferObject->bOutOfStock = bitFlag;
		CopyFromBuf(buf, bitFlag);
		pBufferObject->bGlassCracked = bitFlag;
		CopyFromBuf(buf, bitFlag);
		pBufferObject->bGlassBroken = bitFlag;
		CopyFromBuf(buf, bitFlag);
		pBufferObject->bHasBeenDamaged = bitFlag;
		CopyFromBuf(buf, bitFlag);
		pBufferObject->bUseVehicleColours = bitFlag;
		CopyFromBuf(buf, pBufferObject->m_fCollisionDamageMultiplier);
		CopyFromBuf(buf, pBufferObject->m_nCollisionDamageEffect);
		CopyFromBuf(buf, pBufferObject->m_nSpecialCollisionResponseCases);
		CopyFromBuf(buf, pBufferObject->m_nEndOfLifeTime);
#ifndef COMPATIBLE_SAVES
		CopyFromBuf(buf, (pBufferObject->GetAddressOfEntityProperties())[0]);
		CopyFromBuf(buf, (pBufferObject->GetAddressOfEntityProperties())[1]);
#endif
		if (GetObjectPool()->GetSlot(ref >> 8))
			CPopulation::ConvertToDummyObject(GetObjectPool()->GetSlot(ref >> 8));
		CObject* pObject = new(ref) CObject(mi, false);
		pObject->GetMatrix() = pBufferObject->GetMatrix();
#ifdef COMPATIBLE_SAVES
		pObject->LoadEntityFlags(buf);
#endif
		pObject->m_fUprootLimit = pBufferObject->m_fUprootLimit;
		pObject->m_objectMatrix = pBufferObject->m_objectMatrix;
		pObject->ObjectCreatedBy = pBufferObject->ObjectCreatedBy;
		pObject->bIsPickup = pBufferObject->bIsPickup;
		pObject->bPickupObjWithMessage = pBufferObject->bPickupObjWithMessage;
		pObject->bOutOfStock = pBufferObject->bOutOfStock;
		pObject->bGlassCracked = pBufferObject->bGlassCracked;
		pObject->bGlassBroken = pBufferObject->bGlassBroken;
		pObject->bHasBeenDamaged = pBufferObject->bHasBeenDamaged;
		pObject->bUseVehicleColours = pBufferObject->bUseVehicleColours;
		pObject->m_fCollisionDamageMultiplier = pBufferObject->m_fCollisionDamageMultiplier;
		pObject->m_nCollisionDamageEffect = pBufferObject->m_nCollisionDamageEffect;
		pObject->m_nSpecialCollisionResponseCases = pBufferObject->m_nSpecialCollisionResponseCases;
		pObject->m_nEndOfLifeTime = pBufferObject->m_nEndOfLifeTime;
#ifndef COMPATIBLE_SAVES
		(pObject->GetAddressOfEntityProperties())[0] = (pBufferObject->GetAddressOfEntityProperties())[0];
		(pObject->GetAddressOfEntityProperties())[1] = (pBufferObject->GetAddressOfEntityProperties())[1];
#endif
		pObject->bHasCollided = false;
		CWorld::Add(pObject);
		delete[] obuf;
	}
VALIDATESAVEBUF(size)
}

void CPools::SavePedPool(uint8* buf, uint32* size)
{
INITSAVEBUF
	int nNumPeds = 0;
	int nPoolSize = GetPedPool()->GetSize();
	for (int i = 0; i < nPoolSize; i++) {
		CPed* pPed = GetPedPool()->GetSlot(i);
		if (!pPed)
			continue;
#ifdef MISSION_REPLAY
		if ((!pPed->bInVehicle || (pPed == CWorld::Players[CWorld::PlayerInFocus].m_pPed && IsQuickSave)) && pPed->m_nPedType == PEDTYPE_PLAYER1)
#else
		if (!pPed->bInVehicle && pPed->m_nPedType == PEDTYPE_PLAYER1)
#endif
			nNumPeds++;
	}
	*size = sizeof(int) + nNumPeds * (sizeof(uint32) + sizeof(int16) + sizeof(int) + CPlayerPed::nSaveStructSize +
		sizeof(CWanted::MaximumWantedLevel) + sizeof(CWanted::nMaximumWantedLevel) + MAX_MODEL_NAME);
	CopyToBuf(buf, nNumPeds);
	for (int i = 0; i < nPoolSize; i++) {
		CPed* pPed = GetPedPool()->GetSlot(i);
		if (!pPed)
			continue;
#ifdef MISSION_REPLAY
		if ((!pPed->bInVehicle || (pPed == CWorld::Players[CWorld::PlayerInFocus].m_pPed && IsQuickSave)) && pPed->m_nPedType == PEDTYPE_PLAYER1) {
#else
		if (!pPed->bInVehicle && pPed->m_nPedType == PEDTYPE_PLAYER1) {
#endif
			CopyToBuf(buf, pPed->m_nPedType);
			CopyToBuf(buf, pPed->m_modelIndex);
			int32 ref = GetPedRef(pPed);
			CopyToBuf(buf, ref);
#ifdef COMPATIBLE_SAVES
			pPed->Save(buf);
#else
			memcpy(buf, pPed, sizeof(CPlayerPed));
			SkipSaveBuf(buf, sizeof(CPlayerPed));
#endif
			CopyToBuf(buf, CWanted::MaximumWantedLevel);
			CopyToBuf(buf, CWanted::nMaximumWantedLevel);
			memcpy(buf, CModelInfo::GetModelInfo(pPed->GetModelIndex())->GetModelName(), MAX_MODEL_NAME);
			SkipSaveBuf(buf, MAX_MODEL_NAME);
		}
	}
VALIDATESAVEBUF(*size);
#undef CopyToBuf
}

void CPools::LoadPedPool(uint8* buf, uint32 size)
{
INITSAVEBUF
	int nPeds;
	CopyFromBuf(buf, nPeds);
	for (int i = 0; i < nPeds; i++) {
		uint32 pedtype;
		int16 model;
		int ref;

		CopyFromBuf(buf, pedtype);
		CopyFromBuf(buf, model);
		CopyFromBuf(buf, ref);
#ifdef COMPATIBLE_SAVES
		CPed* pPed;

		char name[MAX_MODEL_NAME];
		// Unfortunate hack: player model is stored after ped structure.
		// It could be avoided by just using "player" because in practice it is always true.
		memcpy(name, buf + CPlayerPed::nSaveStructSize + 2 * sizeof(int32), MAX_MODEL_NAME);
		CStreaming::RequestSpecialModel(model, name, STREAMFLAGS_DONT_REMOVE);
		CStreaming::LoadAllRequestedModels(false);

		if (pedtype == PEDTYPE_PLAYER1)
			pPed = new(ref) CPlayerPed();
		else
			assert(0);

		pPed->Load(buf);
		if (pedtype == PEDTYPE_PLAYER1) {
			CopyFromBuf(buf, CWanted::MaximumWantedLevel);
			CopyFromBuf(buf, CWanted::nMaximumWantedLevel);
			SkipSaveBuf(buf, MAX_MODEL_NAME);
		}

		if (pedtype == PEDTYPE_PLAYER1) {
			pPed->m_wepAccuracy = 100;
			CWorld::Players[0].m_pPed = (CPlayerPed*)pPed;
		}
		CWorld::Add(pPed);
#else
		char* pbuf = new char[sizeof(CPlayerPed)];
		CPlayerPed* pBufferPlayer = (CPlayerPed*)pbuf;
		CPed* pPed;
		char name[MAX_MODEL_NAME];
		// the code implies that there was idea to load non-player ped
		if (pedtype == PEDTYPE_PLAYER1) { // always true
			memcpy(pbuf, buf, sizeof(CPlayerPed));
			SkipSaveBuf(buf, sizeof(CPlayerPed));
			CopyFromBuf(buf, CWanted::MaximumWantedLevel);
			CopyFromBuf(buf, CWanted::nMaximumWantedLevel);
			CopyFromBuf(buf, name);
		}
		CStreaming::RequestSpecialModel(model, name, STREAMFLAGS_DONT_REMOVE);
		CStreaming::LoadAllRequestedModels(false);
		if (pedtype == PEDTYPE_PLAYER1) {
			CPlayerPed* pPlayerPed = new(ref) CPlayerPed();
			for (int i = 0; i < ARRAY_SIZE(pPlayerPed->m_nTargettableObjects); i++)
				pPlayerPed->m_nTargettableObjects[i] = pBufferPlayer->m_nTargettableObjects[i];
			pPlayerPed->m_fMaxStamina = pBufferPlayer->m_fMaxStamina;
			pPed = pPlayerPed;
		}
		pPed->SetPosition(pBufferPlayer->GetPosition());
		pPed->m_fHealth = pBufferPlayer->m_fHealth;
		pPed->m_fArmour = pBufferPlayer->m_fArmour;
		pPed->CharCreatedBy = pBufferPlayer->CharCreatedBy;
		pPed->m_currentWeapon = 0;
		pPed->m_maxWeaponTypeAllowed = pBufferPlayer->m_maxWeaponTypeAllowed;
		for (int i = 0; i < WEAPONTYPE_TOTAL_INVENTORY_WEAPONS; i++)
			pPed->m_weapons[i] = pBufferPlayer->m_weapons[i];

		if (pedtype == PEDTYPE_PLAYER1) {
			pPed->m_wepAccuracy = 100;
			CWorld::Players[0].m_pPed = (CPlayerPed*)pPed;
		}
		CWorld::Add(pPed);
		delete[] pbuf;
#endif
	}
VALIDATESAVEBUF(size)
}

#undef CopyFromBuf
#undef CopyToBuf