#include "common.h"
#include "patcher.h"

#include "Script.h"
#include "ScriptCommands.h"

#include "Boat.h"
#include "BulletInfo.h"
#include "Camera.h"
#include "CarAI.h"
#include "CarCtrl.h"
#include "CarGen.h"
#include "CivilianPed.h"
#include "Clock.h"
#include "CopPed.h"
#include "Coronas.h"
#include "Cranes.h"
#include "CutsceneMgr.h"
#include "Darkel.h"
#include "DMAudio.h"
#include "EmergencyPed.h"
#include "Explosion.h"
#include "FileMgr.h"
#include "Gangs.h"
#include "Garages.h"
#include "General.h"
#include "HandlingMgr.h"
#include "Heli.h"
#include "Hud.h"
#include "Messages.h"
#include "ModelIndices.h"
#include "Pad.h"
#include "ParticleObject.h"
#include "PedRoutes.h"
#include "Phones.h"
#include "Pickups.h"
#include "Plane.h"
#include "PlayerInfo.h"
#include "PlayerPed.h"
#include "PointLights.h"
#include "Pools.h"
#include "Population.h"
#include "ProjectileInfo.h"
#include "Record.h"
#include "Remote.h"
#include "Restart.h"
#include "Replay.h"
#include "Shadows.h"
#include "Stats.h"
#include "Streaming.h"
#include "Text.h"
#include "User.h"
#include "WaterLevel.h"
#include "Weather.h"
#include "World.h"
#include "Zones.h"

#define PICKUP_PLACEMENT_OFFSET 0.5f
#define PED_FIND_Z_OFFSET 5.0f

uint8 (&CTheScripts::ScriptSpace)[SIZE_SCRIPT_SPACE] = *(uint8(*)[SIZE_SCRIPT_SPACE])*(uintptr*)0x74B248;
CRunningScript(&CTheScripts::ScriptsArray)[MAX_NUM_SCRIPTS] = *(CRunningScript(*)[MAX_NUM_SCRIPTS])*(uintptr*)0x6F5C08;
int32(&CTheScripts::BaseBriefIdForContact)[MAX_NUM_CONTACTS] = *(int32(*)[MAX_NUM_CONTACTS])*(uintptr*)0x880200;
int32(&CTheScripts::OnAMissionForContactFlag)[MAX_NUM_CONTACTS] = *(int32(*)[MAX_NUM_CONTACTS])*(uintptr*)0x8622F0;
CTextLine (&CTheScripts::IntroTextLines)[MAX_NUM_INTRO_TEXT_LINES] = *(CTextLine (*)[MAX_NUM_INTRO_TEXT_LINES])*(uintptr*)0x70EA68;
CScriptRectangle (&CTheScripts::IntroRectangles)[MAX_NUM_INTRO_RECTANGLES] = *(CScriptRectangle (*)[MAX_NUM_INTRO_RECTANGLES])*(uintptr*)0x72D108;
CSprite2d (&CTheScripts::ScriptSprites)[MAX_NUM_SCRIPT_SRPITES] = *(CSprite2d(*)[MAX_NUM_SCRIPT_SRPITES])*(uintptr*)0x72B090;
CScriptSphere(&CTheScripts::ScriptSphereArray)[MAX_NUM_SCRIPT_SPHERES] = *(CScriptSphere(*)[MAX_NUM_SCRIPT_SPHERES])*(uintptr*)0x727D60;
tCollectiveData(&CTheScripts::CollectiveArray)[MAX_NUM_COLLECTIVES] = *(tCollectiveData(*)[MAX_NUM_COLLECTIVES])*(uintptr*)0x6FA008;
tUsedObject(&CTheScripts::UsedObjectArray)[MAX_NUM_USED_OBJECTS] = *(tUsedObject(*)[MAX_NUM_USED_OBJECTS])*(uintptr*)0x6E69C8;
int32(&CTheScripts::MultiScriptArray)[MAX_NUM_MISSION_SCRIPTS] = *(int32(*)[MAX_NUM_MISSION_SCRIPTS])*(uintptr*)0x6F0558;
tBuildingSwap(&CTheScripts::BuildingSwapArray)[MAX_NUM_BUILDING_SWAPS] = *(tBuildingSwap(*)[MAX_NUM_BUILDING_SWAPS])*(uintptr*)0x880E30;
CEntity*(&CTheScripts::InvisibilitySettingArray)[MAX_NUM_INVISIBILITY_SETTINGS] = *(CEntity*(*)[MAX_NUM_INVISIBILITY_SETTINGS])*(uintptr*)0x8620F0;
bool &CTheScripts::DbgFlag = *(bool*)0x95CD87;
uint32 &CTheScripts::OnAMissionFlag = *(uint32*)0x8F1B64;
int32 &CTheScripts::StoreVehicleIndex = *(int32*)0x8F5F3C;
bool &CTheScripts::StoreVehicleWasRandom = *(bool*)0x95CDBC;
CRunningScript *&CTheScripts::pIdleScripts = *(CRunningScript**)0x9430D4;
CRunningScript *&CTheScripts::pActiveScripts = *(CRunningScript**)0x8E2BF4;
uint32 &CTheScripts::NextFreeCollectiveIndex = *(uint32*)0x942F98;
int32 &CTheScripts::LastRandomPedId = *(int32*)0x8F251C;
uint16 &CTheScripts::NumberOfUsedObjects = *(uint16*)0x95CC72;
bool &CTheScripts::bAlreadyRunningAMissionScript = *(bool*)0x95CDB3;
bool &CTheScripts::bUsingAMultiScriptFile = *(bool*)0x95CD55;
uint16 &CTheScripts::NumberOfMissionScripts = *(uint16*)0x95CC9A;
uint32 &CTheScripts::LargestMissionScriptSize = *(uint32*)0x9414C8;
uint32 &CTheScripts::MainScriptSize = *(uint32*)0x9405A4;
uint8 &CTheScripts::FailCurrentMission = *(uint8*)0x95CD41;
uint8 &CTheScripts::CountdownToMakePlayerUnsafe = *(uint8*)0x95CD51;
uint8 &CTheScripts::DelayMakingPlayerUnsafeThisTime = *(uint8*)0x95CD88;
uint16 &CTheScripts::NumScriptDebugLines = *(uint16*)0x95CC42;
uint16 &CTheScripts::NumberOfIntroRectanglesThisFrame = *(uint16*)0x95CC88;
uint16 &CTheScripts::NumberOfIntroTextLinesThisFrame = *(uint16*)0x95CC32;
bool &CTheScripts::UseTextCommands = *(bool*)0x95CD57;
CMissionCleanup (&CTheScripts::MissionCleanup) = *(CMissionCleanup*)0x8F2A24;
CUpsideDownCarCheck (&CTheScripts::UpsideDownCars) = *(CUpsideDownCarCheck*)0x6EE450;
CStuckCarCheck (&CTheScripts::StuckCars) = *(CStuckCarCheck*)0x87C588;
uint16 &CTheScripts::CommandsExecuted = *(uint16*)0x95CCA6;
uint16 &CTheScripts::ScriptsUpdated = *(uint16*)0x95CC5E;
int32(&ScriptParams)[32] = *(int32(*)[32])*(uintptr*)0x6ED460;

CMissionCleanup::CMissionCleanup()
{
	Init();
}

void CMissionCleanup::Init()
{
	m_nCount = 0;
	for (int i = 0; i < MAX_CLEANUP; i++){
		m_sEntities[i].type = CLEANUP_UNUSED;
		m_sEntities[i].id = 0;
	}
}

CMissionCleanupEntity* CMissionCleanup::FindFree()
{
	for (int i = 0; i < MAX_CLEANUP; i++){
		if (m_sEntities[i].type == CLEANUP_UNUSED)
			return &m_sEntities[i];
	}
	assert(0);
	return nil;
}

void CMissionCleanup::AddEntityToList(int32 id, uint8 type)
{
	CMissionCleanupEntity* pNew = FindFree();
	if (!pNew)
		return;
	pNew->id = id;
	pNew->type = type;
	m_nCount++;
}

void CMissionCleanup::RemoveEntityFromList(int32 id, uint8 type)
{
	for (int i = 0; i < MAX_CLEANUP; i++){
		if (m_sEntities[i].type == type && m_sEntities[i].id == id){
			m_sEntities[i].id = 0;
			m_sEntities[i].type = CLEANUP_UNUSED;
			m_nCount--;
		}
	}
}

void CMissionCleanup::Process()
{
	CPopulation::m_AllRandomPedsThisType = -1;
	CPopulation::PedDensityMultiplier = 1.0f;
	CCarCtrl::CarDensityMultiplier = 1.0;
	FindPlayerPed()->m_pWanted->m_fCrimeSensitivity = 1.0f;
	TheCamera.Restore();
	TheCamera.SetWideScreenOff();
	DMAudio.ClearMissionAudio();
	CWeather::ReleaseWeather();
	for (int i = 0; i < NUM_OF_SPECIAL_CHARS; i++)
		CStreaming::SetMissionDoesntRequireSpecialChar(i);
	for (int i = 0; i < NUM_OF_CUTSCENE_OBJECTS; i++)
		CStreaming::SetMissionDoesntRequireModel(MI_CUTOBJ01 + i);
	CStreaming::ms_disableStreaming = false;
	CHud::m_ItemToFlash = -1;
	CHud::SetHelpMessage(nil, false);
	CUserDisplay::OnscnTimer.m_bDisabled = false;
	CWorld::Players[0].m_pPed->m_pWanted->m_bIgnoredByCops = false;
	CWorld::Players[0].m_pPed->m_pWanted->m_bIgnoredByEveryone = false;
	CWorld::Players[0].MakePlayerSafe(false);
	CTheScripts::StoreVehicleIndex = -1;
	CTheScripts::StoreVehicleWasRandom = true;
	CTheScripts::UpsideDownCars.Init();
	CTheScripts::StuckCars.Init();
	for (int i = 0; i < MAX_CLEANUP; i++){
		if (m_sEntities[i].type == CLEANUP_UNUSED)
			continue;
		switch (m_sEntities[i].type) {
		case CLEANUP_CAR:
		{
			CVehicle* v = CPools::GetVehiclePool()->GetAt(m_sEntities[i].id);
			if (v)
				CTheScripts::CleanUpThisVehicle(v);
			break;
		}
		case CLEANUP_CHAR:
		{
			CPed* p = CPools::GetPedPool()->GetAt(m_sEntities[i].id);
			if (p)
				CTheScripts::CleanUpThisPed(p);
			break;
		}
		case CLEANUP_OBJECT:
		{
			CObject* o = CPools::GetObjectPool()->GetAt(m_sEntities[i].id);
			if (o)
				CTheScripts::CleanUpThisObject(o);
			break;
		}
		default:
			break;
		}
		m_sEntities[i].id = 0;
		m_sEntities[i].type = CLEANUP_UNUSED;
		m_nCount--;
	}
}

/* NB: CUpsideDownCarCheck is not used by actual script at all
 * It has a weird usage: AreAnyCarsUpsideDown would fail any mission
 * just like death or arrest. */

void CUpsideDownCarCheck::Init()
{
	for (int i = 0; i < MAX_UPSIDEDOWN_CAR_CHECKS; i++){
		m_sCars[i].m_nVehicleIndex = -1;
		m_sCars[i].m_nUpsideDownTimer = 0;
	}
}

bool CUpsideDownCarCheck::IsCarUpsideDown(int32 id)
{
	CVehicle* v = CPools::GetVehiclePool()->GetAt(id);
	return v->GetUp().z <= -0.97f &&
		v->GetMoveSpeed().Magnitude() < 0.01f &&
		v->GetTurnSpeed().Magnitude() < 0.02f;
}

void CUpsideDownCarCheck::UpdateTimers()
{
	uint32 timeStep = CTimer::GetTimeStepInMilliseconds();
	for (int i = 0; i < MAX_UPSIDEDOWN_CAR_CHECKS; i++){
#ifdef FIX_BUGS
		if (m_sCars[i].m_nVehicleIndex == -1)
			continue;
#endif
		CVehicle* v = CPools::GetVehiclePool()->GetAt(m_sCars[i].m_nVehicleIndex);
		if (v){
			if (IsCarUpsideDown(m_sCars[i].m_nVehicleIndex))
				m_sCars[i].m_nUpsideDownTimer += timeStep;
			else
				m_sCars[i].m_nUpsideDownTimer = 0;
		}else{
			m_sCars[i].m_nVehicleIndex = -1;
			m_sCars[i].m_nUpsideDownTimer = 0;
		}
	}
}

bool CUpsideDownCarCheck::AreAnyCarsUpsideDown()
{
	for (int i = 0; i < MAX_UPSIDEDOWN_CAR_CHECKS; i++){
		if (m_sCars[i].m_nVehicleIndex >= 0 && m_sCars[i].m_nUpsideDownTimer > 1000)
			return true;
	}
	return false;
}

void CUpsideDownCarCheck::AddCarToCheck(int32 id)
{
	uint16 index = 0;
	while (index < MAX_UPSIDEDOWN_CAR_CHECKS && m_sCars[index].m_nVehicleIndex >= 0)
		index++;
	if (index >= MAX_UPSIDEDOWN_CAR_CHECKS)
		return;
	m_sCars[index].m_nVehicleIndex = id;
	m_sCars[index].m_nUpsideDownTimer = 0;
}

void CUpsideDownCarCheck::RemoveCarFromCheck(int32 id)
{
	for (int i = 0; i < MAX_UPSIDEDOWN_CAR_CHECKS; i++){
		if (m_sCars[i].m_nVehicleIndex == id){
			m_sCars[i].m_nVehicleIndex = -1;
			m_sCars[i].m_nUpsideDownTimer = 0;
		}
	}
}

bool CUpsideDownCarCheck::HasCarBeenUpsideDownForAWhile(int32 id)
{
	for (int i = 0; i < MAX_UPSIDEDOWN_CAR_CHECKS; i++){
		if (m_sCars[i].m_nVehicleIndex == id)
			return m_sCars[i].m_nUpsideDownTimer > 1000;
	}
	return false;
}

void CStuckCarCheckEntry::Reset()
{
	m_nVehicleIndex = -1;
	m_vecPos = CVector(-5000.0f, -5000.0f, -5000.0f);
	m_nLastCheck = -1;
	m_fRadius = 0.0f;
	m_nStuckTime = 0;
	m_bStuck = false;
}

void CStuckCarCheck::Init()
{
	for (int i = 0; i < MAX_STUCK_CAR_CHECKS; i++) {
		m_sCars[i].Reset();
	}
}

void CStuckCarCheck::Process()
{
	uint32 timer = CTimer::GetTimeInMilliseconds();
	for (int i = 0; i < MAX_STUCK_CAR_CHECKS; i++){
		if (m_sCars[i].m_nVehicleIndex < 0)
			continue;
		if (timer <= m_sCars[i].m_nStuckTime + m_sCars[i].m_nLastCheck)
			continue;
		CVehicle* pv = CPools::GetVehiclePool()->GetAt(m_sCars[i].m_nVehicleIndex);
		if (!pv){
			m_sCars[i].Reset();
			continue;
		}
		float distance = (pv->GetPosition() - m_sCars[i].m_vecPos).Magnitude();
		m_sCars[i].m_bStuck = distance < m_sCars[i].m_fRadius;
		m_sCars[i].m_vecPos = pv->GetPosition();
		m_sCars[i].m_nLastCheck = timer;
	}
}

void CStuckCarCheck::AddCarToCheck(int32 id, float radius, uint32 time)
{
	CVehicle* pv = CPools::GetVehiclePool()->GetAt(id);
	if (!pv)
		return;
	int index = 0;
	while (index < MAX_STUCK_CAR_CHECKS && m_sCars[index].m_nVehicleIndex >= 0)
		index++;
	/* Would be nice to return if index >= MAX_STUCK_CAR_CHECKS... */
	m_sCars[index].m_nVehicleIndex = id;
	m_sCars[index].m_vecPos = pv->GetPosition();
	m_sCars[index].m_nLastCheck = CTimer::GetTimeInMilliseconds();
	m_sCars[index].m_fRadius = radius;
	m_sCars[index].m_nStuckTime = time;
	m_sCars[index].m_bStuck = false;
}

void CStuckCarCheck::RemoveCarFromCheck(int32 id)
{
	for (int i = 0; i < MAX_STUCK_CAR_CHECKS; i++){
		if (m_sCars[i].m_nVehicleIndex == id){
			m_sCars[i].Reset();
		}
	}
}

bool CStuckCarCheck::HasCarBeenStuckForAWhile(int32 id)
{
	for (int i = 0; i < MAX_STUCK_CAR_CHECKS; i++){
		if (m_sCars[i].m_nVehicleIndex == id)
			return m_sCars[i].m_bStuck;
	}
	return false;
}

void CRunningScript::CollectParameters(uint32* pIp, int16 total)
{
	for (int16 i = 0; i < total; i++){
		float tmp;
		switch (CTheScripts::Read1ByteFromScript(pIp))
		{
		case ARGUMENT_INT32:
			ScriptParams[i] = CTheScripts::Read4BytesFromScript(pIp);
			break;
		case ARGUMENT_GLOBALVAR:
			ScriptParams[i] = *((int32*)&CTheScripts::ScriptSpace[CTheScripts::Read2BytesFromScript(pIp)]);
			break;
		case ARGUMENT_LOCALVAR:
			ScriptParams[i] = m_anLocalVariables[CTheScripts::Read2BytesFromScript(pIp)];
			break;
		case ARGUMENT_INT8:
			ScriptParams[i] = CTheScripts::Read1ByteFromScript(pIp);
			break;
		case ARGUMENT_INT16:
			ScriptParams[i] = CTheScripts::Read2BytesFromScript(pIp);
			break;
		case ARGUMENT_FLOAT:
			tmp = CTheScripts::ReadFloatFromScript(pIp);
			ScriptParams[i] = *(int32*)&tmp;
			break;
		default:
			assert(0);
			break;
		}
	}
}

int32 CRunningScript::CollectNextParameterWithoutIncreasingPC(uint32 ip)
{
	uint32* pIp = &ip;
	float tmp;
	switch (CTheScripts::Read1ByteFromScript(pIp))
	{
	case ARGUMENT_INT32:
		return CTheScripts::Read4BytesFromScript(pIp);
	case ARGUMENT_GLOBALVAR:
		return *((int32*)&CTheScripts::ScriptSpace[CTheScripts::Read2BytesFromScript(pIp)]);
	case ARGUMENT_LOCALVAR:
		return m_anLocalVariables[CTheScripts::Read2BytesFromScript(pIp)];
	case ARGUMENT_INT8:
		return CTheScripts::Read1ByteFromScript(pIp);
	case ARGUMENT_INT16:
		return CTheScripts::Read2BytesFromScript(pIp);
	case ARGUMENT_FLOAT:
		tmp = CTheScripts::ReadFloatFromScript(pIp);
		return *(int32*)&tmp;
	default:
		assert(0);
	}
	return -1;
}

void CRunningScript::StoreParameters(uint32* pIp, int16 number)
{
	for (int16 i = 0; i < number; i++){
		switch (CTheScripts::Read1ByteFromScript(pIp)) {
		case ARGUMENT_GLOBALVAR:
			*(int32*)&CTheScripts::ScriptSpace[CTheScripts::Read2BytesFromScript(pIp)] = ScriptParams[i];
			break;
		case ARGUMENT_LOCALVAR:
			m_anLocalVariables[CTheScripts::Read2BytesFromScript(pIp)] = ScriptParams[i];
			break;
		default:
			assert(0);
		}
	}
}

int32 *CRunningScript::GetPointerToScriptVariable(uint32* pIp, int16 type)
{
	switch (CTheScripts::Read1ByteFromScript(pIp))
	{
	case ARGUMENT_GLOBALVAR:
		assert(type == VAR_GLOBAL);
		return (int32*)&CTheScripts::ScriptSpace[CTheScripts::Read2BytesFromScript(pIp)];
	case ARGUMENT_LOCALVAR:
		assert(type == VAR_LOCAL);
		return &m_anLocalVariables[CTheScripts::Read2BytesFromScript(pIp)];
	default:
		assert(0);
	}
	return nil;
}

void CRunningScript::Init()
{
	strcpy(m_abScriptName, "noname");
	next = prev = nil;
	m_nIp = 0;
	for (int i = 0; i < MAX_STACK_DEPTH; i++)
		m_anStack[i] = 0;
	m_nStackPointer = 0;
	m_nWakeTime = 0;
	m_bCondResult = false;
	m_bIsMissionScript = false;
	m_bSkipWakeTime = false;
	for (int i = 0; i < NUM_LOCAL_VARS + NUM_TIMERS; i++)
		m_anLocalVariables[i] = 0;
	m_nAndOrState = 0;
	m_bNotFlag = false;
	m_bDeatharrestEnabled = true;
	m_bDeatharrestExecuted = false;
	m_bMissionFlag = false;
}

#ifdef USE_DEBUG_SCRIPT_LOADER
int open_script()
{
	static int scriptToLoad = 1;

	if (GetAsyncKeyState('G') & 0x8000)
		scriptToLoad = 0;
	if (GetAsyncKeyState('R') & 0x8000)
		scriptToLoad = 1;
	if (GetAsyncKeyState('D') & 0x8000)
		scriptToLoad = 2;

	switch (scriptToLoad) {
	case 0: return CFileMgr::OpenFile("main.scm", "rb");
	case 1: return CFileMgr::OpenFile("main_freeroam.scm", "rb");
	case 2: return CFileMgr::OpenFile("main_d.scm", "rb");
	}
	return CFileMgr::OpenFile("main.scm", "rb");
}
#endif

void CTextLine::Reset()
{
	m_fScaleX = 0.48f;
	m_fScaleY = 1.12f;
	m_sColor = CRGBA(225, 225, 225, 255);
	m_bJustify = false;
	m_bRightJustify = false;
	m_bCentered = false;
	m_bBackground = false;
	m_bBackgroundOnly = false;
	m_fWrapX = 182.0f; /* TODO: scaling as bugfix */
	m_fCenterSize = 640.0f; /* --||-- */
	m_sBackgroundColor = CRGBA(128, 128, 128, 128);
	m_bTextProportional = true;
	m_bTextBeforeFade = false;
	m_nFont = 2; /* enum? */
	m_fAtX = 0.0f;
	m_fAtY = 0.0f;
	memset(&m_Text, 0, sizeof(m_Text));
}

void CTheScripts::Init()
{
	for (int i = 0; i < SIZE_SCRIPT_SPACE; i++)
		ScriptSpace[i] = 0;
	pActiveScripts = pIdleScripts = nil;
	for (int i = 0; i < MAX_NUM_SCRIPTS; i++){
		ScriptsArray[i].Init();
		ScriptsArray[i].AddScriptToList(&pIdleScripts);
	}
	MissionCleanup.Init();
	UpsideDownCars.Init();
	StuckCars.Init();
	CFileMgr::SetDir("data");
#ifdef USE_DEBUG_SCRIPT_LOADER
	int mainf = open_script();
#else
	int mainf = CFileMgr::OpenFile("main.scm", "rb");
#endif
	CFileMgr::Read(mainf, (char*)ScriptSpace, SIZE_MAIN_SCRIPT);
	CFileMgr::CloseFile(mainf);
	CFileMgr::SetDir("");
	StoreVehicleIndex = -1;
	StoreVehicleWasRandom = true;
	OnAMissionFlag = 0;
	for (int i = 0; i < MAX_NUM_CONTACTS; i++){
		BaseBriefIdForContact[i] = 0;
		OnAMissionForContactFlag[i] = 0;
	}
	for (int i = 0; i < MAX_NUM_COLLECTIVES; i++){
		CollectiveArray[i].index = -1;
		CollectiveArray[i].unk_data = 0;
	}
	NextFreeCollectiveIndex = 0;
	LastRandomPedId = -1;
	for (int i = 0; i < MAX_NUM_USED_OBJECTS; i++){
		memset(&UsedObjectArray[i].name, 0, sizeof(UsedObjectArray[i].name));
		UsedObjectArray[i].index = 0;
	}
	NumberOfUsedObjects = 0;
	ReadObjectNamesFromScript();
	UpdateObjectIndices();
	bAlreadyRunningAMissionScript = false;
	bUsingAMultiScriptFile = true;
	for (int i = 0; i < MAX_NUM_MISSION_SCRIPTS; i++)
		MultiScriptArray[i] = 0;
	NumberOfMissionScripts = 0;
	LargestMissionScriptSize = 0;
	MainScriptSize = 0;
	ReadMultiScriptFileOffsetsFromScript();
	FailCurrentMission = 0;
	CountdownToMakePlayerUnsafe = 0;
	DbgFlag = 0;
	DelayMakingPlayerUnsafeThisTime = 1;
	NumScriptDebugLines = 0;
	for (int i = 0; i < MAX_NUM_SCRIPT_SPHERES; i++){
		ScriptSphereArray[i].m_bInUse = false;
		ScriptSphereArray[i].m_Index = 1;
		ScriptSphereArray[i].m_Id = 0;
		ScriptSphereArray[i].m_vecCenter = CVector(0.0f, 0.0f, 0.0f);
		ScriptSphereArray[i].m_fRadius = 0.0f;
	}
	for (int i = 0; i < MAX_NUM_INTRO_TEXT_LINES; i++){
		IntroTextLines[i].Reset();
	}
	NumberOfIntroTextLinesThisFrame = 0;
	UseTextCommands = false;
	for (int i = 0; i < MAX_NUM_INTRO_RECTANGLES; i++){
		IntroRectangles[i].m_bIsUsed = false;
		IntroRectangles[i].m_bBeforeFade = false;
		IntroRectangles[i].m_nTextureId = -1;
		IntroRectangles[i].m_sRect = CRect(0.0f, 0.0f, 0.0f, 0.0f);
		IntroRectangles[i].m_sColor = CRGBA(255, 255, 255, 255);
	}
	NumberOfIntroRectanglesThisFrame = 0;
	for (int i = 0; i < MAX_NUM_BUILDING_SWAPS; i++){
		BuildingSwapArray[i].m_pBuilding = nil;
		BuildingSwapArray[i].m_nNewModel = -1;
		BuildingSwapArray[i].m_nOldModel = -1;
	}
	for (int i = 0; i < MAX_NUM_INVISIBILITY_SETTINGS; i++)
		InvisibilitySettingArray[i] = nil;
}

void CRunningScript::RemoveScriptFromList(CRunningScript** ppScript)
{
	if (prev)
		prev->next = next;
	else
		*ppScript = next;
	if (next)
		next->prev = prev;
}

void CRunningScript::AddScriptToList(CRunningScript** ppScript)
{
	next = *ppScript;
	prev = nil;
	if (*ppScript)
		(*ppScript)->prev = this;
	*ppScript = this;
}

CRunningScript* CTheScripts::StartNewScript(uint32 ip)
{
	CRunningScript* pNew = pIdleScripts;
	assert(pNew);
	pNew->RemoveScriptFromList(&pIdleScripts);
	pNew->Init();
	pNew->SetIP(ip);
	pNew->AddScriptToList(&pActiveScripts);
	return pNew;
}

void CTheScripts::Process()
{
	if (CReplay::IsPlayingBack())
		return;
	CommandsExecuted = 0;
	ScriptsUpdated = 0;
	float timeStep = CTimer::GetTimeStepInMilliseconds();
	UpsideDownCars.UpdateTimers();
	StuckCars.Process();
	DrawScriptSpheres();
	if (FailCurrentMission)
		--FailCurrentMission;
	if (CountdownToMakePlayerUnsafe){
		if (--CountdownToMakePlayerUnsafe == 0)
			CWorld::Players[0].MakePlayerSafe(false);
	}
	if (UseTextCommands){
		for (int i = 0; i < MAX_NUM_INTRO_TEXT_LINES; i++)
			IntroTextLines[i].Reset();
		NumberOfIntroTextLinesThisFrame = 0;
		for (int i = 0; i < MAX_NUM_INTRO_RECTANGLES; i++){
			IntroRectangles[i].m_bIsUsed = false;
			IntroRectangles[i].m_bBeforeFade = false;
		}
		NumberOfIntroRectanglesThisFrame = 0;
		if (UseTextCommands == 1)
			UseTextCommands = 0;
	}
	CRunningScript* script = pActiveScripts;
	while (script != nil){
		CRunningScript* next = script->GetNext();
		++ScriptsUpdated;
		script->UpdateTimers(timeStep);
		script->Process();
		script = next;
	}
	DbgFlag = false;
}

CRunningScript* CTheScripts::StartTestScript()
{
	return StartNewScript(0);
}

bool CTheScripts::IsPlayerOnAMission()
{
	return OnAMissionFlag && *(int32*)&ScriptSpace[OnAMissionFlag] == 1;
}

void CRunningScript::Process()
{
	if (m_bIsMissionScript)
		DoDeatharrestCheck();
	if (m_bMissionFlag && CTheScripts::FailCurrentMission == 1 && m_nStackPointer == 1)
		m_nIp = m_anStack[--m_nStackPointer];
	if (CTimer::GetTimeInMilliseconds() >= m_nWakeTime){
		while (!ProcessOneCommand())
			;
		return;
	}
	if (!m_bSkipWakeTime)
		return;
	if (!CPad::GetPad(0)->GetCrossJustDown())
		return;
	m_nWakeTime = 0;
	for (int i = 0; i < NUMBIGMESSAGES; i++){
		if (CMessages::BIGMessages[i].m_Stack[0].m_pText != nil)
			CMessages::BIGMessages[i].m_Stack[0].m_nStartTime = 0;
	}
	if (CMessages::BriefMessages[0].m_pText != nil)
		CMessages::BriefMessages[0].m_nStartTime = 0;
}

int8 CRunningScript::ProcessOneCommand()
{
	++CTheScripts::CommandsExecuted;
	int32 command = CTheScripts::Read2BytesFromScript(&m_nIp);
	m_bNotFlag = (command & 0x8000);
	command &= 0x7FFF;
	if (command < 100)
		return ProcessCommandsFrom0To99(command);
	if (command < 200)
		return ProcessCommandsFrom100To199(command);
	if (command < 300)
		return ProcessCommandsFrom200To299(command);
	if (command < 400)
		return ProcessCommandsFrom300To399(command);
	if (command < 500)
		return ProcessCommandsFrom400To499(command);
	if (command < 600)
		return ProcessCommandsFrom500To599(command);
	if (command < 700)
		return ProcessCommandsFrom600To699(command);
	if (command < 800)
		return ProcessCommandsFrom700To799(command);
	if (command < 900)
		return ProcessCommandsFrom800To899(command);
	if (command < 1000)
		return ProcessCommandsFrom900To999(command);
	if (command < 1100)
		return ProcessCommandsFrom1000To1099(command);
	if (command < 1200)
		return ProcessCommandsFrom1100To1199(command);
	return -1;
}

int8 CRunningScript::ProcessCommandsFrom0To99(int32 command)
{
	switch (command) {
	case COMMAND_NOP:
		return 0;
	case COMMAND_WAIT:
		CollectParameters(&m_nIp, 1);
		m_nWakeTime = CTimer::GetTimeInMilliseconds() + ScriptParams[0];
		return 1;
	case COMMAND_GOTO:
		CollectParameters(&m_nIp, 1);
		SetIP(ScriptParams[0] >= 0 ? ScriptParams[0] : SIZE_MAIN_SCRIPT - ScriptParams[0]);
		/* Known issue: GOTO to 0. It might have been "better" to use > instead of >= */
		/* simply because it never makes sense to jump to start of the script */
		/* but jumping to start of a custom mission is an issue for simple mission-like scripts */
		/* However, it's not an issue for actual mission scripts, because they follow a structure */
		/* and never start with a loop. */
		return 0;
	case COMMAND_SHAKE_CAM:
		CollectParameters(&m_nIp, 1);
		CamShakeNoPos(&TheCamera, ScriptParams[0] / 1000.0f);
		return 0;
	case COMMAND_SET_VAR_INT:
	{
		int32* ptr = GetPointerToScriptVariable(&m_nIp, VAR_GLOBAL);
		CollectParameters(&m_nIp, 1);
		*ptr = ScriptParams[0];
		return 0;
	}
	case COMMAND_SET_VAR_FLOAT:
	{
		int32* ptr = GetPointerToScriptVariable(&m_nIp, VAR_GLOBAL);
		CollectParameters(&m_nIp, 1);
		*(float*)ptr = *(float*)&ScriptParams[0];
		return 0;
	}
	case COMMAND_SET_LVAR_INT:
	{
		int32* ptr = GetPointerToScriptVariable(&m_nIp, VAR_LOCAL);
		CollectParameters(&m_nIp, 1);
		*ptr = ScriptParams[0];
		return 0;
	}
	case COMMAND_SET_LVAR_FLOAT:
	{
		int32* ptr = GetPointerToScriptVariable(&m_nIp, VAR_LOCAL);
		CollectParameters(&m_nIp, 1);
		*(float*)ptr = *(float*)&ScriptParams[0];
		return 0;
	}
	case COMMAND_ADD_VAL_TO_INT_VAR:
	{
		int32* ptr = GetPointerToScriptVariable(&m_nIp, VAR_GLOBAL);
		CollectParameters(&m_nIp, 1);
		*ptr += ScriptParams[0];
		return 0;
	}
	case COMMAND_ADD_VAL_TO_FLOAT_VAR:
	{
		int32* ptr = GetPointerToScriptVariable(&m_nIp, VAR_GLOBAL);
		CollectParameters(&m_nIp, 1);
		*(float*)ptr += *(float*)&ScriptParams[0];
		return 0;
	}
	case COMMAND_ADD_VAL_TO_INT_LVAR:
	{
		int32* ptr = GetPointerToScriptVariable(&m_nIp, VAR_LOCAL);
		CollectParameters(&m_nIp, 1);
		*ptr += ScriptParams[0];
		return 0;
	}
	case COMMAND_ADD_VAL_TO_FLOAT_LVAR:
	{
		int32* ptr = GetPointerToScriptVariable(&m_nIp, VAR_LOCAL);
		CollectParameters(&m_nIp, 1);
		*(float*)ptr += *(float*)&ScriptParams[0];
		return 0;
	}
	case COMMAND_SUB_VAL_FROM_INT_VAR:
	{
		int32* ptr = GetPointerToScriptVariable(&m_nIp, VAR_GLOBAL);
		CollectParameters(&m_nIp, 1);
		*ptr -= ScriptParams[0];
		return 0;
	}
	case COMMAND_SUB_VAL_FROM_FLOAT_VAR:
	{
		int32* ptr = GetPointerToScriptVariable(&m_nIp, VAR_GLOBAL);
		CollectParameters(&m_nIp, 1);
		*(float*)ptr -= *(float*)&ScriptParams[0];
		return 0;
	}
	case COMMAND_SUB_VAL_FROM_INT_LVAR:
	{
		int32* ptr = GetPointerToScriptVariable(&m_nIp, VAR_LOCAL);
		CollectParameters(&m_nIp, 1);
		*ptr -= ScriptParams[0];
		return 0;
	}
	case COMMAND_SUB_VAL_FROM_FLOAT_LVAR:
	{
		int32* ptr = GetPointerToScriptVariable(&m_nIp, VAR_LOCAL);
		CollectParameters(&m_nIp, 1);
		*(float*)ptr -= *(float*)&ScriptParams[0];
		return 0;
	}
	case COMMAND_MULT_INT_VAR_BY_VAL:
	{
		int32* ptr = GetPointerToScriptVariable(&m_nIp, VAR_GLOBAL);
		CollectParameters(&m_nIp, 1);
		*ptr *= ScriptParams[0];
		return 0;
	}
	case COMMAND_MULT_FLOAT_VAR_BY_VAL:
	{
		int32* ptr = GetPointerToScriptVariable(&m_nIp, VAR_GLOBAL);
		CollectParameters(&m_nIp, 1);
		*(float*)ptr *= *(float*)&ScriptParams[0];
		return 0;
	}
	case COMMAND_MULT_INT_LVAR_BY_VAL:
	{
		int32* ptr = GetPointerToScriptVariable(&m_nIp, VAR_LOCAL);
		CollectParameters(&m_nIp, 1);
		*ptr *= ScriptParams[0];
		return 0;
	}
	case COMMAND_MULT_FLOAT_LVAR_BY_VAL:
	{
		int32* ptr = GetPointerToScriptVariable(&m_nIp, VAR_LOCAL);
		CollectParameters(&m_nIp, 1);
		*(float*)ptr *= *(float*)&ScriptParams[0];
		return 0;
	}
	case COMMAND_DIV_INT_VAR_BY_VAL:
	{
		int32* ptr = GetPointerToScriptVariable(&m_nIp, VAR_GLOBAL);
		CollectParameters(&m_nIp, 1);
		*ptr /= ScriptParams[0];
		return 0;
	}
	case COMMAND_DIV_FLOAT_VAR_BY_VAL:
	{
		int32* ptr = GetPointerToScriptVariable(&m_nIp, VAR_GLOBAL);
		CollectParameters(&m_nIp, 1);
		*(float*)ptr /= *(float*)&ScriptParams[0];
		return 0;
	}
	case COMMAND_DIV_INT_LVAR_BY_VAL:
	{
		int32* ptr = GetPointerToScriptVariable(&m_nIp, VAR_LOCAL);
		CollectParameters(&m_nIp, 1);
		*ptr /= ScriptParams[0];
		return 0;
	}
	case COMMAND_DIV_FLOAT_LVAR_BY_VAL:
	{
		int32* ptr = GetPointerToScriptVariable(&m_nIp, VAR_LOCAL);
		CollectParameters(&m_nIp, 1);
		*(float*)ptr /= *(float*)&ScriptParams[0];
		return 0;
	}
	case COMMAND_IS_INT_VAR_GREATER_THAN_NUMBER:
	{
		int32* ptr = GetPointerToScriptVariable(&m_nIp, VAR_GLOBAL);
		CollectParameters(&m_nIp, 1);
		UpdateCompareFlag(*ptr > ScriptParams[0]);
		return 0;
	}
	case COMMAND_IS_INT_LVAR_GREATER_THAN_NUMBER:
	{
		int32* ptr = GetPointerToScriptVariable(&m_nIp, VAR_LOCAL);
		CollectParameters(&m_nIp, 1);
		UpdateCompareFlag(*ptr > ScriptParams[0]);
		return 0;
	}
	case COMMAND_IS_NUMBER_GREATER_THAN_INT_VAR:
	{
		CollectParameters(&m_nIp, 1);
		int32* ptr = GetPointerToScriptVariable(&m_nIp, VAR_GLOBAL);
		UpdateCompareFlag(ScriptParams[0] > *ptr);
		return 0;
	}
	case COMMAND_IS_NUMBER_GREATER_THAN_INT_LVAR:
	{
		CollectParameters(&m_nIp, 1);
		int32* ptr = GetPointerToScriptVariable(&m_nIp, VAR_LOCAL);
		UpdateCompareFlag(ScriptParams[0] > *ptr);
		return 0;
	}
	case COMMAND_IS_INT_VAR_GREATER_THAN_INT_VAR:
	{
		int32* ptr1 = GetPointerToScriptVariable(&m_nIp, VAR_GLOBAL);
		int32* ptr2 = GetPointerToScriptVariable(&m_nIp, VAR_GLOBAL);
		UpdateCompareFlag(*ptr1 > *ptr2);
		return 0;
	}
	case COMMAND_IS_INT_LVAR_GREATER_THAN_INT_VAR:
	{
		int32* ptr1 = GetPointerToScriptVariable(&m_nIp, VAR_LOCAL);
		int32* ptr2 = GetPointerToScriptVariable(&m_nIp, VAR_GLOBAL);
		UpdateCompareFlag(*ptr1 > *ptr2);
		return 0;
	}
	case COMMAND_IS_INT_VAR_GREATER_THAN_INT_LVAR:
	{
		int32* ptr1 = GetPointerToScriptVariable(&m_nIp, VAR_GLOBAL);
		int32* ptr2 = GetPointerToScriptVariable(&m_nIp, VAR_LOCAL);
		UpdateCompareFlag(*ptr1 > *ptr2);
		return 0;
	}
	case COMMAND_IS_INT_LVAR_GREATER_THAN_INT_LVAR:
	{
		int32* ptr1 = GetPointerToScriptVariable(&m_nIp, VAR_LOCAL);
		int32* ptr2 = GetPointerToScriptVariable(&m_nIp, VAR_LOCAL);
		UpdateCompareFlag(*ptr1 > *ptr2);
		return 0;
	}
	case COMMAND_IS_FLOAT_VAR_GREATER_THAN_NUMBER:
	{
		int32* ptr = GetPointerToScriptVariable(&m_nIp, VAR_GLOBAL);
		CollectParameters(&m_nIp, 1);
		UpdateCompareFlag(*(float*)ptr > *(float*)&ScriptParams[0]);
		return 0;
	}
	case COMMAND_IS_FLOAT_LVAR_GREATER_THAN_NUMBER:
	{
		int32* ptr = GetPointerToScriptVariable(&m_nIp, VAR_LOCAL);
		CollectParameters(&m_nIp, 1);
		UpdateCompareFlag(*(float*)ptr > *(float*)&ScriptParams[0]);
		return 0;
	}
	case COMMAND_IS_NUMBER_GREATER_THAN_FLOAT_VAR:
	{
		CollectParameters(&m_nIp, 1);
		int32* ptr = GetPointerToScriptVariable(&m_nIp, VAR_GLOBAL);
		UpdateCompareFlag(*(float*)&ScriptParams[0] > *(float*)ptr);
		return 0;
	}
	case COMMAND_IS_NUMBER_GREATER_THAN_FLOAT_LVAR:
	{
		CollectParameters(&m_nIp, 1);
		int32* ptr = GetPointerToScriptVariable(&m_nIp, VAR_LOCAL);
		UpdateCompareFlag(*(float*)&ScriptParams[0] > *(float*)ptr);
		return 0;
	}
	case COMMAND_IS_FLOAT_VAR_GREATER_THAN_FLOAT_VAR:
	{
		int32* ptr1 = GetPointerToScriptVariable(&m_nIp, VAR_GLOBAL);
		int32* ptr2 = GetPointerToScriptVariable(&m_nIp, VAR_GLOBAL);
		UpdateCompareFlag(*(float*)ptr1 > *(float*)ptr2);
		return 0;
	}
	case COMMAND_IS_FLOAT_LVAR_GREATER_THAN_FLOAT_VAR:
	{
		int32* ptr1 = GetPointerToScriptVariable(&m_nIp, VAR_LOCAL);
		int32* ptr2 = GetPointerToScriptVariable(&m_nIp, VAR_GLOBAL);
		UpdateCompareFlag(*(float*)ptr1 > *(float*)ptr2);
		return 0;
	}
	case COMMAND_IS_FLOAT_VAR_GREATER_THAN_FLOAT_LVAR:
	{
		int32* ptr1 = GetPointerToScriptVariable(&m_nIp, VAR_GLOBAL);
		int32* ptr2 = GetPointerToScriptVariable(&m_nIp, VAR_LOCAL);
		UpdateCompareFlag(*(float*)ptr1 > *(float*)ptr2);
		return 0;
	}
	case COMMAND_IS_FLOAT_LVAR_GREATER_THAN_FLOAT_LVAR:
	{
		int32* ptr1 = GetPointerToScriptVariable(&m_nIp, VAR_LOCAL);
		int32* ptr2 = GetPointerToScriptVariable(&m_nIp, VAR_LOCAL);
		UpdateCompareFlag(*(float*)ptr1 > *(float*)ptr2);
		return 0;
	}
	case COMMAND_IS_INT_VAR_GREATER_OR_EQUAL_TO_NUMBER:
	{
		int32* ptr = GetPointerToScriptVariable(&m_nIp, VAR_GLOBAL);
		CollectParameters(&m_nIp, 1);
		UpdateCompareFlag(*ptr >= ScriptParams[0]);
		return 0;
	}
	case COMMAND_IS_INT_LVAR_GREATER_OR_EQUAL_TO_NUMBER:
	{
		int32* ptr = GetPointerToScriptVariable(&m_nIp, VAR_LOCAL);
		CollectParameters(&m_nIp, 1);
		UpdateCompareFlag(*ptr >= ScriptParams[0]);
		return 0;
	}
	case COMMAND_IS_NUMBER_GREATER_OR_EQUAL_TO_INT_VAR:
	{
		CollectParameters(&m_nIp, 1);
		int32* ptr = GetPointerToScriptVariable(&m_nIp, VAR_GLOBAL);
		UpdateCompareFlag(ScriptParams[0] >= *ptr);
		return 0;
	}
	case COMMAND_IS_NUMBER_GREATER_OR_EQUAL_TO_INT_LVAR:
	{
		CollectParameters(&m_nIp, 1);
		int32* ptr = GetPointerToScriptVariable(&m_nIp, VAR_LOCAL);
		UpdateCompareFlag(ScriptParams[0] >= *ptr);
		return 0;
	}
	case COMMAND_IS_INT_VAR_GREATER_OR_EQUAL_TO_INT_VAR:
	{
		int32* ptr1 = GetPointerToScriptVariable(&m_nIp, VAR_GLOBAL);
		int32* ptr2 = GetPointerToScriptVariable(&m_nIp, VAR_GLOBAL);
		UpdateCompareFlag(*ptr1 >= *ptr2);
		return 0;
	}
	case COMMAND_IS_INT_LVAR_GREATER_OR_EQUAL_TO_INT_VAR:
	{
		int32* ptr1 = GetPointerToScriptVariable(&m_nIp, VAR_LOCAL);
		int32* ptr2 = GetPointerToScriptVariable(&m_nIp, VAR_GLOBAL);
		UpdateCompareFlag(*ptr1 >= *ptr2);
		return 0;
	}
	case COMMAND_IS_INT_VAR_GREATER_OR_EQUAL_TO_INT_LVAR:
	{
		int32* ptr1 = GetPointerToScriptVariable(&m_nIp, VAR_GLOBAL);
		int32* ptr2 = GetPointerToScriptVariable(&m_nIp, VAR_LOCAL);
		UpdateCompareFlag(*ptr1 >= *ptr2);
		return 0;
	}
	case COMMAND_IS_INT_LVAR_GREATER_OR_EQUAL_TO_INT_LVAR:
	{
		int32* ptr1 = GetPointerToScriptVariable(&m_nIp, VAR_LOCAL);
		int32* ptr2 = GetPointerToScriptVariable(&m_nIp, VAR_LOCAL);
		UpdateCompareFlag(*ptr1 >= *ptr2);
		return 0;
	}
	case COMMAND_IS_FLOAT_VAR_GREATER_OR_EQUAL_TO_NUMBER:
	{
		int32* ptr = GetPointerToScriptVariable(&m_nIp, VAR_GLOBAL);
		CollectParameters(&m_nIp, 1);
		UpdateCompareFlag(*(float*)ptr >= *(float*)&ScriptParams[0]);
		return 0;
	}
	case COMMAND_IS_FLOAT_LVAR_GREATER_OR_EQUAL_TO_NUMBER:
	{
		int32* ptr = GetPointerToScriptVariable(&m_nIp, VAR_LOCAL);
		CollectParameters(&m_nIp, 1);
		UpdateCompareFlag(*(float*)ptr >= *(float*)&ScriptParams[0]);
		return 0;
	}
	case COMMAND_IS_NUMBER_GREATER_OR_EQUAL_TO_FLOAT_VAR:
	{
		CollectParameters(&m_nIp, 1);
		int32* ptr = GetPointerToScriptVariable(&m_nIp, VAR_GLOBAL);
		UpdateCompareFlag(*(float*)&ScriptParams[0] >= *(float*)ptr);
		return 0;
	}
	case COMMAND_IS_NUMBER_GREATER_OR_EQUAL_TO_FLOAT_LVAR:
	{
		CollectParameters(&m_nIp, 1);
		int32* ptr = GetPointerToScriptVariable(&m_nIp, VAR_LOCAL);
		UpdateCompareFlag(*(float*)&ScriptParams[0] >= *(float*)ptr);
		return 0;
	}
	case COMMAND_IS_FLOAT_VAR_GREATER_OR_EQUAL_TO_FLOAT_VAR:
	{
		int32* ptr1 = GetPointerToScriptVariable(&m_nIp, VAR_GLOBAL);
		int32* ptr2 = GetPointerToScriptVariable(&m_nIp, VAR_GLOBAL);
		UpdateCompareFlag(*(float*)ptr1 >= *(float*)ptr2);
		return 0;
	}
	case COMMAND_IS_FLOAT_LVAR_GREATER_OR_EQUAL_TO_FLOAT_VAR:
	{
		int32* ptr1 = GetPointerToScriptVariable(&m_nIp, VAR_LOCAL);
		int32* ptr2 = GetPointerToScriptVariable(&m_nIp, VAR_GLOBAL);
		UpdateCompareFlag(*(float*)ptr1 >= *(float*)ptr2);
		return 0;
	}
	case COMMAND_IS_FLOAT_VAR_GREATER_OR_EQUAL_TO_FLOAT_LVAR:
	{
		int32* ptr1 = GetPointerToScriptVariable(&m_nIp, VAR_GLOBAL);
		int32* ptr2 = GetPointerToScriptVariable(&m_nIp, VAR_LOCAL);
		UpdateCompareFlag(*(float*)ptr1 >= *(float*)ptr2);
		return 0;
	}
	case COMMAND_IS_FLOAT_LVAR_GREATER_OR_EQUAL_TO_FLOAT_LVAR:
	{
		int32* ptr1 = GetPointerToScriptVariable(&m_nIp, VAR_LOCAL);
		int32* ptr2 = GetPointerToScriptVariable(&m_nIp, VAR_LOCAL);
		UpdateCompareFlag(*(float*)ptr1 >= *(float*)ptr2);
		return 0;
	}
	case COMMAND_IS_INT_VAR_EQUAL_TO_NUMBER:
	{
		int32* ptr = GetPointerToScriptVariable(&m_nIp, VAR_GLOBAL);
		CollectParameters(&m_nIp, 1);
		UpdateCompareFlag(*ptr == ScriptParams[0]);
		return 0;
	}
	case COMMAND_IS_INT_LVAR_EQUAL_TO_NUMBER:
	{
		int32* ptr = GetPointerToScriptVariable(&m_nIp, VAR_LOCAL);
		CollectParameters(&m_nIp, 1);
		UpdateCompareFlag(*ptr == ScriptParams[0]);
		return 0;
	}
	case COMMAND_IS_INT_VAR_EQUAL_TO_INT_VAR:
	{
		int32* ptr1 = GetPointerToScriptVariable(&m_nIp, VAR_GLOBAL);
		int32* ptr2 = GetPointerToScriptVariable(&m_nIp, VAR_GLOBAL);
		UpdateCompareFlag(*ptr1 == *ptr2);
		return 0;
	}
	case COMMAND_IS_INT_VAR_EQUAL_TO_INT_LVAR:
	{
		int32* ptr1 = GetPointerToScriptVariable(&m_nIp, VAR_GLOBAL);
		int32* ptr2 = GetPointerToScriptVariable(&m_nIp, VAR_LOCAL);
		UpdateCompareFlag(*ptr1 == *ptr2);
		return 0;
	}
	case COMMAND_IS_INT_LVAR_EQUAL_TO_INT_LVAR:
	{
		int32* ptr1 = GetPointerToScriptVariable(&m_nIp, VAR_LOCAL);
		int32* ptr2 = GetPointerToScriptVariable(&m_nIp, VAR_LOCAL);
		UpdateCompareFlag(*ptr1 == *ptr2);
		return 0;
	}
	/* Following commands are not implemented, and go to default case
	case COMMAND_IS_INT_VAR_NOT_EQUAL_TO_NUMBER:
	case COMMAND_IS_INT_LVAR_NOT_EQUAL_TO_NUMBER:
	case COMMAND_IS_INT_VAR_NOT_EQUAL_TO_INT_VAR:
	case COMMAND_IS_INT_LVAR_NOT_EQUAL_TO_INT_LVAR:
	case COMMAND_IS_INT_VAR_NOT_EQUAL_TO_INT_LVAR:
	*/
	case COMMAND_IS_FLOAT_VAR_EQUAL_TO_NUMBER:
	{
		int32* ptr = GetPointerToScriptVariable(&m_nIp, VAR_GLOBAL);
		CollectParameters(&m_nIp, 1);
		UpdateCompareFlag(*(float*)ptr == *(float*)&ScriptParams[0]);
		return 0;
	}
	case COMMAND_IS_FLOAT_LVAR_EQUAL_TO_NUMBER:
	{
		int32* ptr = GetPointerToScriptVariable(&m_nIp, VAR_LOCAL);
		CollectParameters(&m_nIp, 1);
		UpdateCompareFlag(*(float*)ptr == *(float*)&ScriptParams[0]);
		return 0;
	}
	case COMMAND_IS_FLOAT_VAR_EQUAL_TO_FLOAT_VAR:
	{
		int32* ptr1 = GetPointerToScriptVariable(&m_nIp, VAR_GLOBAL);
		int32* ptr2 = GetPointerToScriptVariable(&m_nIp, VAR_GLOBAL);
		UpdateCompareFlag(*(float*)ptr1 == *(float*)ptr2);
		return 0;
	}
	case COMMAND_IS_FLOAT_VAR_EQUAL_TO_FLOAT_LVAR:
	{
		int32* ptr1 = GetPointerToScriptVariable(&m_nIp, VAR_GLOBAL);
		int32* ptr2 = GetPointerToScriptVariable(&m_nIp, VAR_LOCAL);
		UpdateCompareFlag(*(float*)ptr1 == *(float*)ptr2);
		return 0;
	}
	case COMMAND_IS_FLOAT_LVAR_EQUAL_TO_FLOAT_LVAR:
	{
		int32* ptr1 = GetPointerToScriptVariable(&m_nIp, VAR_LOCAL);
		int32* ptr2 = GetPointerToScriptVariable(&m_nIp, VAR_LOCAL);
		UpdateCompareFlag(*(float*)ptr1 == *(float*)ptr2);
		return 0;
	}
	/* Following commands are not implemented, and go to default case
	case COMMAND_IS_FLOAT_VAR_NOT_EQUAL_TO_NUMBER:
	case COMMAND_IS_FLOAT_LVAR_NOT_EQUAL_TO_NUMBER:
	case COMMAND_IS_FLOAT_VAR_NOT_EQUAL_TO_FLOAT_VAR:
	case COMMAND_IS_FLOAT_LVAR_NOT_EQUAL_TO_FLOAT_LVAR:
	case COMMAND_IS_FLOAT_VAR_NOT_EQUAL_TO_FLOAT_LVAR:
	*/
	case COMMAND_GOTO_IF_TRUE:
		CollectParameters(&m_nIp, 1);
		if (m_bCondResult)
			SetIP(ScriptParams[0] >= 0 ? ScriptParams[0] : SIZE_MAIN_SCRIPT - ScriptParams[0]);
		/* Check COMMAND_GOTO note. */
		return 0;
	case COMMAND_GOTO_IF_FALSE:
		CollectParameters(&m_nIp, 1);
		if (!m_bCondResult)
			SetIP(ScriptParams[0] >= 0 ? ScriptParams[0] : SIZE_MAIN_SCRIPT - ScriptParams[0]);
		/* Check COMMAND_GOTO note. */
		return 0;
	case COMMAND_TERMINATE_THIS_SCRIPT:
		if (m_bMissionFlag)
			CTheScripts::bAlreadyRunningAMissionScript = false;
		RemoveScriptFromList(&CTheScripts::pActiveScripts);
		AddScriptToList(&CTheScripts::pIdleScripts);
		return 1;
	case COMMAND_START_NEW_SCRIPT:
	{
		CollectParameters(&m_nIp, 1);
		assert(ScriptParams[0] >= 0);
		CRunningScript* pNew = CTheScripts::StartNewScript(ScriptParams[0]);
		int8 type = CTheScripts::Read1ByteFromScript(&m_nIp);
		float tmp;
		for (int i = 0; type != ARGUMENT_END; type = CTheScripts::Read1ByteFromScript(&m_nIp), i++) {
			switch (type) {
			case ARGUMENT_INT32:
				pNew->m_anLocalVariables[i] = CTheScripts::Read4BytesFromScript(&m_nIp);
				break;
			case ARGUMENT_GLOBALVAR:
				pNew->m_anLocalVariables[i] = *(int32*)&CTheScripts::ScriptSpace[CTheScripts::Read2BytesFromScript(&m_nIp)];
				break;
			case ARGUMENT_LOCALVAR:
				pNew->m_anLocalVariables[i] = m_anLocalVariables[CTheScripts::Read2BytesFromScript(&m_nIp)];
				break;
			case ARGUMENT_INT8:
				pNew->m_anLocalVariables[i] = CTheScripts::Read1ByteFromScript(&m_nIp);
				break;
			case ARGUMENT_INT16:
				pNew->m_anLocalVariables[i] = CTheScripts::Read2BytesFromScript(&m_nIp);
				break;
			case ARGUMENT_FLOAT:
				tmp = CTheScripts::ReadFloatFromScript(&m_nIp);
				pNew->m_anLocalVariables[i] = *(int32*)&tmp;
				break;
			default:
				break;
			}
		}
		return 0;
	}
	case COMMAND_GOSUB:
		CollectParameters(&m_nIp, 1);
		assert(m_nStackPointer < MAX_STACK_DEPTH);
		m_anStack[m_nStackPointer++] = m_nIp;
		SetIP(ScriptParams[0] >= 0 ? ScriptParams[0] : SIZE_MAIN_SCRIPT - ScriptParams[0]);
		return 0;
	case COMMAND_RETURN:
		assert(m_nStackPointer > 0); /* No more SSU */
		SetIP(m_anStack[--m_nStackPointer]);
		return 0;
	case COMMAND_LINE:
		CollectParameters(&m_nIp, 6);
		/* Something must have been here */
		return 0;
	case COMMAND_CREATE_PLAYER:
	{
		CollectParameters(&m_nIp, 4);
		int32 index = ScriptParams[0];
		assert(index < 1); /* Constant? Also no more double player glitch */
		debug("&&&&&&&&&&&&&Creating player: %d\n", index);
		if (!CStreaming::HasModelLoaded(MI_PLAYER)) {
			CStreaming::RequestSpecialModel(MI_PLAYER, "player", STREAMFLAGS_DONT_REMOVE | STREAMFLAGS_DEPENDENCY);
			CStreaming::LoadAllRequestedModels(false);
		}
		CPlayerPed::SetupPlayerPed(index);
		CWorld::Players[index].m_pPed->CharCreatedBy = MISSION_CHAR;
		CPlayerPed::DeactivatePlayerPed(index);
		CVector pos = *(CVector*)&ScriptParams[1];
		if (pos.z <= MAP_Z_LOW_LIMIT)
			pos.z = CWorld::FindGroundZForCoord(pos.x, pos.y);
		pos.z += CWorld::Players[index].m_pPed->GetDistanceFromCentreOfMassToBaseOfModel();
		CWorld::Players[index].m_pPed->GetPosition() = pos;
		CTheScripts::ClearSpaceForMissionEntity(pos, CWorld::Players[index].m_pPed);
		CPlayerPed::ReactivatePlayerPed(index);
		ScriptParams[0] = index;
		StoreParameters(&m_nIp, 1);
		return 0;
	}
	case COMMAND_GET_PLAYER_COORDINATES:
	{
		CVector pos;
		CollectParameters(&m_nIp, 1);
		if (CWorld::Players[ScriptParams[0]].m_pPed->bInVehicle)
			pos = CWorld::Players[ScriptParams[0]].m_pPed->m_pMyVehicle->GetPosition();
		else
			pos = CWorld::Players[ScriptParams[0]].m_pPed->GetPosition();
		*(CVector*)&ScriptParams[0] = pos;
		StoreParameters(&m_nIp, 3);
		return 0;
	}
	case COMMAND_SET_PLAYER_COORDINATES:
	{
		CollectParameters(&m_nIp, 4);
		CVector pos = *(CVector*)&ScriptParams[1];
		int index = ScriptParams[0];
		if (pos.z <= MAP_Z_LOW_LIMIT)
			pos.z = CWorld::FindGroundZForCoord(pos.x, pos.y);
		CPlayerPed* ped = CWorld::Players[index].m_pPed;
		if (!ped->bInVehicle) {
			pos.z += ped->GetDistanceFromCentreOfMassToBaseOfModel();
			ped->Teleport(pos);
			CTheScripts::ClearSpaceForMissionEntity(pos, ped);
			return 0;
		}
		pos.z += ped->m_pMyVehicle->GetDistanceFromCentreOfMassToBaseOfModel();
		if (ped->m_pMyVehicle->IsBoat())
			ped->m_pMyVehicle->Teleport(pos);
		else
			ped->m_pMyVehicle->Teleport(pos);
		/* I'll keep this condition here but obviously it is absolutely pointless */
		/* It's clearly present in disassembly so it had to be in original code */
		CTheScripts::ClearSpaceForMissionEntity(pos, ped->m_pMyVehicle);
		return 0;
	}
	case COMMAND_IS_PLAYER_IN_AREA_2D:
	{
		CollectParameters(&m_nIp, 6);
		CPlayerPed* ped = CWorld::Players[ScriptParams[0]].m_pPed;
		float x1 = *(float*)&ScriptParams[1];
		float y1 = *(float*)&ScriptParams[2];
		float x2 = *(float*)&ScriptParams[3];
		float y2 = *(float*)&ScriptParams[4];
		if (!ped->bInVehicle)
			UpdateCompareFlag(ped->IsWithinArea(x1, y1, x2, y2));
		else
			UpdateCompareFlag(ped->m_pMyVehicle->IsWithinArea(x1, y1, x2, y2));
		if (!ScriptParams[5])
			return 0;
		CTheScripts::HighlightImportantArea((uint32)this + m_nIp, x1, y1, x2, y2, MAP_Z_LOW_LIMIT);
		if (CTheScripts::DbgFlag)
			CTheScripts::DrawDebugSquare(x1, y1, x2, y2);
		return 0;
	}
	case COMMAND_IS_PLAYER_IN_AREA_3D:
	{
		CollectParameters(&m_nIp, 8);
		CPlayerPed* ped = CWorld::Players[ScriptParams[0]].m_pPed;
		float x1 = *(float*)&ScriptParams[1];
		float y1 = *(float*)&ScriptParams[2];
		float z1 = *(float*)&ScriptParams[3];
		float x2 = *(float*)&ScriptParams[4];
		float y2 = *(float*)&ScriptParams[5];
		float z2 = *(float*)&ScriptParams[6];
		if (ped->bInVehicle)
			UpdateCompareFlag(ped->m_pMyVehicle->IsWithinArea(x1, y1, z1, x2, y2, z2));
		else
			UpdateCompareFlag(ped->IsWithinArea(x1, y1, z1, x2, y2, z2));
		if (!ScriptParams[7])
			return 0;
		CTheScripts::HighlightImportantArea((uint32)this + m_nIp, x1, y1, x2, y2, (z1 + z2) / 2);
		if (CTheScripts::DbgFlag)
			CTheScripts::DrawDebugCube(x1, y1, z1, x2, y2, z2);
		return 0;
	}
	case COMMAND_ADD_INT_VAR_TO_INT_VAR:
		*GetPointerToScriptVariable(&m_nIp, VAR_GLOBAL) += *GetPointerToScriptVariable(&m_nIp, VAR_GLOBAL);
		return 0;
	case COMMAND_ADD_INT_LVAR_TO_INT_VAR:
		*GetPointerToScriptVariable(&m_nIp, VAR_GLOBAL) += *GetPointerToScriptVariable(&m_nIp, VAR_LOCAL);
		return 0;
	case COMMAND_ADD_INT_VAR_TO_INT_LVAR:
		*GetPointerToScriptVariable(&m_nIp, VAR_LOCAL) += *GetPointerToScriptVariable(&m_nIp, VAR_GLOBAL);
		return 0;
	case COMMAND_ADD_INT_LVAR_TO_INT_LVAR:
		*GetPointerToScriptVariable(&m_nIp, VAR_LOCAL) += *GetPointerToScriptVariable(&m_nIp, VAR_LOCAL);
		return 0;
	case COMMAND_ADD_FLOAT_VAR_TO_FLOAT_VAR:
		*(float*)GetPointerToScriptVariable(&m_nIp, VAR_GLOBAL) += *(float*)GetPointerToScriptVariable(&m_nIp, VAR_GLOBAL);
		return 0;
	case COMMAND_ADD_FLOAT_LVAR_TO_FLOAT_VAR:
		*(float*)GetPointerToScriptVariable(&m_nIp, VAR_GLOBAL) += *(float*)GetPointerToScriptVariable(&m_nIp, VAR_LOCAL);
		return 0;
	case COMMAND_ADD_FLOAT_VAR_TO_FLOAT_LVAR:
		*(float*)GetPointerToScriptVariable(&m_nIp, VAR_LOCAL) += *(float*)GetPointerToScriptVariable(&m_nIp, VAR_GLOBAL);
		return 0;
	case COMMAND_ADD_FLOAT_LVAR_TO_FLOAT_LVAR:
		*(float*)GetPointerToScriptVariable(&m_nIp, VAR_LOCAL) += *(float*)GetPointerToScriptVariable(&m_nIp, VAR_LOCAL);
		return 0;
	case COMMAND_SUB_INT_VAR_FROM_INT_VAR:
		*GetPointerToScriptVariable(&m_nIp, VAR_GLOBAL) -= *GetPointerToScriptVariable(&m_nIp, VAR_GLOBAL);
		return 0;
	case COMMAND_SUB_INT_LVAR_FROM_INT_LVAR:
		*GetPointerToScriptVariable(&m_nIp, VAR_LOCAL) -= *GetPointerToScriptVariable(&m_nIp, VAR_LOCAL);
		return 0;
	case COMMAND_SUB_FLOAT_VAR_FROM_FLOAT_VAR:
		*(float*)GetPointerToScriptVariable(&m_nIp, VAR_GLOBAL) -= *(float*)GetPointerToScriptVariable(&m_nIp, VAR_GLOBAL);
		return 0;
	case COMMAND_SUB_FLOAT_LVAR_FROM_FLOAT_LVAR:
		*(float*)GetPointerToScriptVariable(&m_nIp, VAR_LOCAL) -= *(float*)GetPointerToScriptVariable(&m_nIp, VAR_LOCAL);
		return 0;
	default:
		assert(0);
		break;
	}
	return -1;
}

void CRunningScript::UpdateCompareFlag(bool flag)
{
	if (m_bNotFlag)
		flag = !flag;
	if (m_nAndOrState == ANDOR_NONE){
		m_bCondResult = flag;
		return;
	}
	if (m_nAndOrState >= ANDS_1 && m_nAndOrState <= ANDS_8){
		m_bCondResult &= flag;
		if (m_nAndOrState == ANDS_1){
			m_nAndOrState = ANDOR_NONE;
			return;
		}
	}else if (m_nAndOrState >= ORS_1 && m_nAndOrState <= ORS_8){
		m_bCondResult |= flag;
		if (m_nAndOrState == ORS_1) {
			m_nAndOrState = ANDOR_NONE;
			return;
		}
	}else{
		return;
	}
	m_nAndOrState--;
}


int8 CRunningScript::ProcessCommandsFrom100To199(int32 command)
{
	switch (command) {
	case COMMAND_SUB_INT_LVAR_FROM_INT_VAR:
		*GetPointerToScriptVariable(&m_nIp, VAR_GLOBAL) -= *GetPointerToScriptVariable(&m_nIp, VAR_LOCAL);
		return 0;
	case COMMAND_SUB_INT_VAR_FROM_INT_LVAR:
		*GetPointerToScriptVariable(&m_nIp, VAR_LOCAL) -= *GetPointerToScriptVariable(&m_nIp, VAR_GLOBAL);
		return 0;
	case COMMAND_SUB_FLOAT_LVAR_FROM_FLOAT_VAR:
		*(float*)GetPointerToScriptVariable(&m_nIp, VAR_GLOBAL) -= *(float*)GetPointerToScriptVariable(&m_nIp, VAR_LOCAL);
		return 0;
	case COMMAND_SUB_FLOAT_VAR_FROM_FLOAT_LVAR:
		*(float*)GetPointerToScriptVariable(&m_nIp, VAR_LOCAL) -= *(float*)GetPointerToScriptVariable(&m_nIp, VAR_GLOBAL);
		return 0;
	case COMMAND_MULT_INT_VAR_BY_INT_VAR:
		*GetPointerToScriptVariable(&m_nIp, VAR_GLOBAL) *= *GetPointerToScriptVariable(&m_nIp, VAR_GLOBAL);
		return 0;
	case COMMAND_MULT_INT_LVAR_BY_INT_VAR:
		*GetPointerToScriptVariable(&m_nIp, VAR_GLOBAL) *= *GetPointerToScriptVariable(&m_nIp, VAR_LOCAL);
		return 0;
	case COMMAND_MULT_INT_VAR_BY_INT_LVAR:
		*GetPointerToScriptVariable(&m_nIp, VAR_LOCAL) *= *GetPointerToScriptVariable(&m_nIp, VAR_GLOBAL);
		return 0;
	case COMMAND_MULT_INT_LVAR_BY_INT_LVAR:
		*GetPointerToScriptVariable(&m_nIp, VAR_LOCAL) *= *GetPointerToScriptVariable(&m_nIp, VAR_LOCAL);
		return 0;
	case COMMAND_MULT_FLOAT_VAR_BY_FLOAT_VAR:
		*(float*)GetPointerToScriptVariable(&m_nIp, VAR_GLOBAL) *= *(float*)GetPointerToScriptVariable(&m_nIp, VAR_GLOBAL);
		return 0;
	case COMMAND_MULT_FLOAT_LVAR_BY_FLOAT_VAR:
		*(float*)GetPointerToScriptVariable(&m_nIp, VAR_GLOBAL) *= *(float*)GetPointerToScriptVariable(&m_nIp, VAR_LOCAL);
		return 0;
	case COMMAND_MULT_FLOAT_VAR_BY_FLOAT_LVAR:
		*(float*)GetPointerToScriptVariable(&m_nIp, VAR_LOCAL) *= *(float*)GetPointerToScriptVariable(&m_nIp, VAR_GLOBAL);
		return 0;
	case COMMAND_MULT_FLOAT_LVAR_BY_FLOAT_LVAR:
		*(float*)GetPointerToScriptVariable(&m_nIp, VAR_LOCAL) *= *(float*)GetPointerToScriptVariable(&m_nIp, VAR_LOCAL);
		return 0;
	case COMMAND_DIV_INT_VAR_BY_INT_VAR:
		*GetPointerToScriptVariable(&m_nIp, VAR_GLOBAL) /= *GetPointerToScriptVariable(&m_nIp, VAR_GLOBAL);
		return 0;
	case COMMAND_DIV_INT_LVAR_BY_INT_VAR:
		*GetPointerToScriptVariable(&m_nIp, VAR_GLOBAL) /= *GetPointerToScriptVariable(&m_nIp, VAR_LOCAL);
		return 0;
	case COMMAND_DIV_INT_VAR_BY_INT_LVAR:
		*GetPointerToScriptVariable(&m_nIp, VAR_LOCAL) /= *GetPointerToScriptVariable(&m_nIp, VAR_GLOBAL);
		return 0;
	case COMMAND_DIV_INT_LVAR_BY_INT_LVAR:
		*GetPointerToScriptVariable(&m_nIp, VAR_LOCAL) /= *GetPointerToScriptVariable(&m_nIp, VAR_LOCAL);
		return 0;
	case COMMAND_DIV_FLOAT_VAR_BY_FLOAT_VAR:
		*(float*)GetPointerToScriptVariable(&m_nIp, VAR_GLOBAL) /= *(float*)GetPointerToScriptVariable(&m_nIp, VAR_GLOBAL);
		return 0;
	case COMMAND_DIV_FLOAT_LVAR_BY_FLOAT_VAR:
		*(float*)GetPointerToScriptVariable(&m_nIp, VAR_GLOBAL) /= *(float*)GetPointerToScriptVariable(&m_nIp, VAR_LOCAL);
		return 0;
	case COMMAND_DIV_FLOAT_VAR_BY_FLOAT_LVAR:
		*(float*)GetPointerToScriptVariable(&m_nIp, VAR_LOCAL) /= *(float*)GetPointerToScriptVariable(&m_nIp, VAR_GLOBAL);
		return 0;
	case COMMAND_DIV_FLOAT_LVAR_BY_FLOAT_LVAR:
		*(float*)GetPointerToScriptVariable(&m_nIp, VAR_LOCAL) /= *(float*)GetPointerToScriptVariable(&m_nIp, VAR_LOCAL);
		return 0;
	case COMMAND_ADD_TIMED_VAL_TO_FLOAT_VAR:
	{
		int32* ptr = GetPointerToScriptVariable(&m_nIp, VAR_GLOBAL);
		CollectParameters(&m_nIp, 1);
		*(float*)ptr += CTimer::GetTimeStep() * *(float*)&ScriptParams[0];
		return 0;
	}
	case COMMAND_ADD_TIMED_VAL_TO_FLOAT_LVAR:
	{
		int32* ptr = GetPointerToScriptVariable(&m_nIp, VAR_LOCAL);
		CollectParameters(&m_nIp, 1);
		*(float*)ptr += CTimer::GetTimeStep() * *(float*)&ScriptParams[0];
		return 0;
	}
	case COMMAND_ADD_TIMED_FLOAT_VAR_TO_FLOAT_VAR:
		*(float*)GetPointerToScriptVariable(&m_nIp, VAR_GLOBAL) += CTimer::GetTimeStep() * *(float*)GetPointerToScriptVariable(&m_nIp, VAR_GLOBAL);
		return 0;
	case COMMAND_ADD_TIMED_FLOAT_LVAR_TO_FLOAT_VAR:
		*(float*)GetPointerToScriptVariable(&m_nIp, VAR_GLOBAL) += CTimer::GetTimeStep() * *(float*)GetPointerToScriptVariable(&m_nIp, VAR_LOCAL);
		return 0;
	case COMMAND_ADD_TIMED_FLOAT_VAR_TO_FLOAT_LVAR:
		*(float*)GetPointerToScriptVariable(&m_nIp, VAR_LOCAL) += CTimer::GetTimeStep() * *(float*)GetPointerToScriptVariable(&m_nIp, VAR_GLOBAL);
		return 0;
	case COMMAND_ADD_TIMED_FLOAT_LVAR_TO_FLOAT_LVAR:
		*(float*)GetPointerToScriptVariable(&m_nIp, VAR_LOCAL) += CTimer::GetTimeStep() * *(float*)GetPointerToScriptVariable(&m_nIp, VAR_LOCAL);
		return 0;
	case COMMAND_SUB_TIMED_VAL_FROM_FLOAT_VAR:
	{
		int32* ptr = GetPointerToScriptVariable(&m_nIp, VAR_GLOBAL);
		CollectParameters(&m_nIp, 1);
		*(float*)ptr -= CTimer::GetTimeStep() * *(float*)&ScriptParams[0];
		return 0;
	}
	case COMMAND_SUB_TIMED_VAL_FROM_FLOAT_LVAR:
	{
		int32* ptr = GetPointerToScriptVariable(&m_nIp, VAR_LOCAL);
		CollectParameters(&m_nIp, 1);
		*(float*)ptr -= CTimer::GetTimeStep() * *(float*)&ScriptParams[0];
		return 0;
	}
	case COMMAND_SUB_TIMED_FLOAT_VAR_FROM_FLOAT_VAR:
		*(float*)GetPointerToScriptVariable(&m_nIp, VAR_GLOBAL) -= CTimer::GetTimeStep() * *(float*)GetPointerToScriptVariable(&m_nIp, VAR_GLOBAL);
		return 0;
	case COMMAND_SUB_TIMED_FLOAT_LVAR_FROM_FLOAT_VAR:
		*(float*)GetPointerToScriptVariable(&m_nIp, VAR_GLOBAL) -= CTimer::GetTimeStep() * *(float*)GetPointerToScriptVariable(&m_nIp, VAR_LOCAL);
		return 0;
	case COMMAND_SUB_TIMED_FLOAT_VAR_FROM_FLOAT_LVAR:
		*(float*)GetPointerToScriptVariable(&m_nIp, VAR_LOCAL) -= CTimer::GetTimeStep() * *(float*)GetPointerToScriptVariable(&m_nIp, VAR_GLOBAL);
		return 0;
	case COMMAND_SUB_TIMED_FLOAT_LVAR_FROM_FLOAT_LVAR:
		*(float*)GetPointerToScriptVariable(&m_nIp, VAR_LOCAL) -= CTimer::GetTimeStep() * *(float*)GetPointerToScriptVariable(&m_nIp, VAR_LOCAL);
		return 0;
	case COMMAND_SET_VAR_INT_TO_VAR_INT:
	{
		int32* ptr = GetPointerToScriptVariable(&m_nIp, VAR_GLOBAL);
		*ptr = *GetPointerToScriptVariable(&m_nIp, VAR_GLOBAL);
		return 0;
	}
	case COMMAND_SET_LVAR_INT_TO_VAR_INT:
	{
		int32* ptr = GetPointerToScriptVariable(&m_nIp, VAR_GLOBAL);
		*ptr = *GetPointerToScriptVariable(&m_nIp, VAR_LOCAL);
		return 0;
	}
	case COMMAND_SET_VAR_INT_TO_LVAR_INT:
	{
		int32* ptr = GetPointerToScriptVariable(&m_nIp, VAR_LOCAL);
		*ptr = *GetPointerToScriptVariable(&m_nIp, VAR_GLOBAL);
		return 0;
	}
	case COMMAND_SET_LVAR_INT_TO_LVAR_INT:
	{
		int32* ptr = GetPointerToScriptVariable(&m_nIp, VAR_LOCAL);
		*ptr = *GetPointerToScriptVariable(&m_nIp, VAR_LOCAL);
		return 0;
	}
	case COMMAND_SET_VAR_FLOAT_TO_VAR_FLOAT:
	{
		float* ptr = (float*)GetPointerToScriptVariable(&m_nIp, VAR_GLOBAL);
		*ptr = *(float*)GetPointerToScriptVariable(&m_nIp, VAR_GLOBAL);
		return 0;
	}
	case COMMAND_SET_LVAR_FLOAT_TO_VAR_FLOAT:
	{
		float* ptr = (float*)GetPointerToScriptVariable(&m_nIp, VAR_GLOBAL);
		*ptr = *(float*)GetPointerToScriptVariable(&m_nIp, VAR_LOCAL);
		return 0;
	}
	case COMMAND_SET_VAR_FLOAT_TO_LVAR_FLOAT:
	{
		float* ptr = (float*)GetPointerToScriptVariable(&m_nIp, VAR_LOCAL);
		*ptr = *(float*)GetPointerToScriptVariable(&m_nIp, VAR_GLOBAL);
		return 0;
	}
	case COMMAND_SET_LVAR_FLOAT_TO_LVAR_FLOAT:
	{
		float* ptr = (float*)GetPointerToScriptVariable(&m_nIp, VAR_LOCAL);
		*ptr = *(float*)GetPointerToScriptVariable(&m_nIp, VAR_LOCAL);
		return 0;
	}
	case COMMAND_CSET_VAR_INT_TO_VAR_FLOAT:
	{
		int32* ptr = GetPointerToScriptVariable(&m_nIp, VAR_GLOBAL);
		*ptr = *(float*)GetPointerToScriptVariable(&m_nIp, VAR_GLOBAL);
		return 0;
	}
	case COMMAND_CSET_LVAR_INT_TO_VAR_FLOAT:
	{
		int32* ptr = GetPointerToScriptVariable(&m_nIp, VAR_GLOBAL);
		*ptr = *(float*)GetPointerToScriptVariable(&m_nIp, VAR_LOCAL);
		return 0;
	}
	case COMMAND_CSET_VAR_INT_TO_LVAR_FLOAT:
	{
		int32* ptr = GetPointerToScriptVariable(&m_nIp, VAR_LOCAL);
		*ptr = *(float*)GetPointerToScriptVariable(&m_nIp, VAR_GLOBAL);
		return 0;
	}
	case COMMAND_CSET_LVAR_INT_TO_LVAR_FLOAT:
	{
		int32* ptr = GetPointerToScriptVariable(&m_nIp, VAR_LOCAL);
		*ptr = *(float*)GetPointerToScriptVariable(&m_nIp, VAR_LOCAL);
		return 0;
	}
	case COMMAND_CSET_VAR_FLOAT_TO_VAR_INT:
	{
		float* ptr = (float*)GetPointerToScriptVariable(&m_nIp, VAR_GLOBAL);
		*ptr = *GetPointerToScriptVariable(&m_nIp, VAR_GLOBAL);
		return 0;
	}
	case COMMAND_CSET_LVAR_FLOAT_TO_VAR_INT:
	{
		float* ptr = (float*)GetPointerToScriptVariable(&m_nIp, VAR_GLOBAL);
		*ptr = *GetPointerToScriptVariable(&m_nIp, VAR_LOCAL);
		return 0;
	}
	case COMMAND_CSET_VAR_FLOAT_TO_LVAR_INT:
	{
		float* ptr = (float*)GetPointerToScriptVariable(&m_nIp, VAR_LOCAL);
		*ptr = *GetPointerToScriptVariable(&m_nIp, VAR_GLOBAL);
		return 0;
	}
	case COMMAND_CSET_LVAR_FLOAT_TO_LVAR_INT:
	{
		float* ptr = (float*)GetPointerToScriptVariable(&m_nIp, VAR_LOCAL);
		*ptr = *GetPointerToScriptVariable(&m_nIp, VAR_LOCAL);
		return 0;
	}
	case COMMAND_ABS_VAR_INT:
	{
		int32* ptr = GetPointerToScriptVariable(&m_nIp, VAR_GLOBAL);
		*ptr = ABS(*ptr);
		return 0;
	}
	case COMMAND_ABS_LVAR_INT:
	{
		int32* ptr = GetPointerToScriptVariable(&m_nIp, VAR_LOCAL);
		*ptr = ABS(*ptr);
		return 0;
	}
	case COMMAND_ABS_VAR_FLOAT:
	{
		float* ptr = (float*)GetPointerToScriptVariable(&m_nIp, VAR_GLOBAL);
		*ptr = ABS(*ptr);
		return 0;
	}
	case COMMAND_ABS_LVAR_FLOAT:
	{
		float* ptr = (float*)GetPointerToScriptVariable(&m_nIp, VAR_LOCAL);
		*ptr = ABS(*ptr);
		return 0;
	}
	case COMMAND_GENERATE_RANDOM_FLOAT:
	{
		float* ptr = (float*)GetPointerToScriptVariable(&m_nIp, VAR_GLOBAL);
		CGeneral::GetRandomNumber();
		CGeneral::GetRandomNumber();
		CGeneral::GetRandomNumber(); /* To make it EXTRA random! */
#ifdef FIX_BUGS
		*ptr = CGeneral::GetRandomNumberInRange(0.0f, 1.0f);
#else
		*ptr = CGeneral::GetRandomNumber() / 65536.0f;
		/* Between 0 and 0.5 on PC (oh well...), never used in original script. */
#endif

		return 0;
	}
	case COMMAND_GENERATE_RANDOM_INT:
#ifdef FIX_BUGS
		// Not a very good fix but before switching to PS2 rand, it sort of works
		*GetPointerToScriptVariable(&m_nIp, VAR_GLOBAL) = CGeneral::GetRandomNumberInRange(0, 65535);
#else
		/* On PC between 0 and 32767, even though script expects values between 0 and 65536 */
		*GetPointerToScriptVariable(&m_nIp, VAR_GLOBAL) = CGeneral::GetRandomNumber();
#endif
		return 0;
	case COMMAND_CREATE_CHAR:
	{
		CollectParameters(&m_nIp, 5);
		switch (ScriptParams[1]) {
		case MI_COP:
			if (ScriptParams[0] == PEDTYPE_COP)
				ScriptParams[1] = COP_STREET;
			break;
		case MI_SWAT:
			if (ScriptParams[0] == PEDTYPE_COP)
				ScriptParams[1] = COP_SWAT;
			break;
		case MI_FBI:
			if (ScriptParams[0] == PEDTYPE_COP)
				ScriptParams[1] = COP_FBI;
			break;
		case MI_ARMY:
			if (ScriptParams[0] == PEDTYPE_COP)
				ScriptParams[1] = COP_ARMY;
			break;
		case MI_MEDIC:
			if (ScriptParams[0] == PEDTYPE_EMERGENCY)
				ScriptParams[1] = PEDTYPE_EMERGENCY;
			break;
		case MI_FIREMAN:
			if (ScriptParams[0] == PEDTYPE_FIREMAN)
				ScriptParams[1] = PEDTYPE_FIREMAN;
			break;
		default:
			break;
		}
		CPed* ped;
		if (ScriptParams[0] == PEDTYPE_COP)
			ped = new CCopPed((eCopType)ScriptParams[1]);
		else if (ScriptParams[0] == PEDTYPE_EMERGENCY || ScriptParams[0] == PEDTYPE_FIREMAN)
			ped = new CEmergencyPed(ScriptParams[1]);
		else
			ped = new CCivilianPed(ScriptParams[0], ScriptParams[1]);
		ped->CharCreatedBy = MISSION_CHAR;
		ped->bRespondsToThreats = false;
		ped->bAllowMedicsToReviveMe = false;
		CVector pos = *(CVector*)&ScriptParams[2];
		if (pos.z <= MAP_Z_LOW_LIMIT)
			pos.z = CWorld::FindGroundZForCoord(pos.x, pos.y);
		pos.z += 1.0f;
		ped->GetPosition() = pos;
		ped->SetOrientation(0.0f, 0.0f, 0.0f);
		CTheScripts::ClearSpaceForMissionEntity(pos, ped);
		CWorld::Add(ped);
		ped->m_nZoneLevel = CTheZones::GetLevelFromPosition(pos);
		CPopulation::ms_nTotalMissionPeds++;
		ScriptParams[0] = CPools::GetPedPool()->GetIndex(ped);
		StoreParameters(&m_nIp, 1);
		if (m_bIsMissionScript)
			CTheScripts::MissionCleanup.AddEntityToList(ScriptParams[0], CLEANUP_CHAR);
		return 0;
	}
	case COMMAND_DELETE_CHAR:
	{
		CollectParameters(&m_nIp, 1);
		CPed* ped = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		if (ped) {
			if (ped->InVehicle()) {
				if (ped->m_pMyVehicle->pDriver == ped) {
					ped->m_pMyVehicle->RemoveDriver();
					ped->m_pMyVehicle->m_status = STATUS_ABANDONED;
					if (ped->m_pMyVehicle->m_nDoorLock == CARLOCK_LOCKED_INITIALLY)
						ped->m_pMyVehicle->m_nDoorLock = CARLOCK_UNLOCKED;
					if (ped->m_nPedType == PEDTYPE_COP && ped->m_pMyVehicle->IsLawEnforcementVehicle())
						ped->m_pMyVehicle->ChangeLawEnforcerState(0);
				}
				else {
					ped->m_pMyVehicle->RemovePassenger(ped);
				}
			}
			CWorld::RemoveReferencesToDeletedObject(ped);
			delete ped;
			--CPopulation::ms_nTotalMissionPeds;
		}
		if (m_bIsMissionScript)
			CTheScripts::MissionCleanup.RemoveEntityFromList(ScriptParams[0], CLEANUP_CHAR);
		return 0;
	}
	case COMMAND_CHAR_WANDER_DIR:
	{
		CollectParameters(&m_nIp, 2);
		CPed* ped = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		assert(ped);
		ped->ClearAll();
		int8 path = ScriptParams[1];
		if (ScriptParams[1] < 0 || ScriptParams[1] > 7)
			path = CGeneral::GetRandomNumberInRange(0, 7);
		ped->SetWanderPath(path);
		return 0;
	}
	/* Not implemented.
	case COMMAND_CHAR_WANDER_RANGE:
	*/
	case COMMAND_CHAR_FOLLOW_PATH:
	{
		CollectParameters(&m_nIp, 4);
		CPed* ped = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		assert(ped);
		CVector pos = *(CVector*)&ScriptParams[1];
		if (pos.z <= MAP_Z_LOW_LIMIT)
			pos.z = CWorld::FindGroundZForCoord(pos.x, pos.y);
		ped->ClearAll();
		ped->SetFollowPath(pos);
		return 0;
	}
	case COMMAND_CHAR_SET_IDLE:
	{
		CollectParameters(&m_nIp, 1);
		CPed* ped = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		assert(ped);
		ped->bScriptObjectiveCompleted = false;
		ped->SetObjective(OBJECTIVE_IDLE);
		return 0;
	}
	case COMMAND_GET_CHAR_COORDINATES:
	{
		CollectParameters(&m_nIp, 1);
		CPed* ped = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		assert(ped);
		CVehicle* vehicle;
		CVector pos;
		/* Seems a bit clumsy but I'll leave original flow */
		if (ped->bInVehicle)
			vehicle = ped->m_pMyVehicle;
		else
			vehicle = nil;
		if (vehicle)
			pos = vehicle->GetPosition();
		else
			pos = ped->GetPosition();
		*(CVector*)&ScriptParams[0] = pos;
		StoreParameters(&m_nIp, 3);
		return 0;
	}
	case COMMAND_SET_CHAR_COORDINATES:
	{
		CollectParameters(&m_nIp, 4);
		CPed* ped = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		assert(ped);
		CVehicle* vehicle;
		if (ped->bInVehicle)
			vehicle = ped->m_pMyVehicle;
		else
			vehicle = nil;
		CVector pos = *(CVector*)&ScriptParams[1];
		if (pos.z <= MAP_Z_LOW_LIMIT)
			pos.z = CWorld::FindGroundZForCoord(pos.x, pos.y);
		/* The following block was once again written
		 * by someone not familiar with virtual functions.
		 * It doesn't require any ifs at all.
		 * To keep as close to original as possible, I'll keep it.
		 * Maybe there was more commented out/debug
		 * stuff, but I doubt it.
		 */
		if (!vehicle) {
			pos.z += ped->GetDistanceFromCentreOfMassToBaseOfModel();
			ped->Teleport(pos);
			CTheScripts::ClearSpaceForMissionEntity(pos, ped);
		}
		else if (vehicle->IsBoat()) {
			pos.z += vehicle->GetDistanceFromCentreOfMassToBaseOfModel();
			vehicle->Teleport(pos);
			CTheScripts::ClearSpaceForMissionEntity(pos, vehicle);
		}
		else {
			pos.z += vehicle->GetDistanceFromCentreOfMassToBaseOfModel();
			vehicle->Teleport(pos);
			CTheScripts::ClearSpaceForMissionEntity(pos, vehicle);
		}
		/* Short version of this command.
		 *
		 * CollectParameters(&m_nIp, 4);
		 * CPed* ped = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		 * assert(ped);
		 * CEntity* entityToMove = ped->bInVehicle ? ped->m_pMyVehicle : ped;
		 * CVector pos = *(CVector*)&ScriptParams[1];
		 * if (pos.z <= MAP_Z_LOW_LIMIT)
		 *	pos.z = CWorld::FindGroundZForCoord(pos.x, pos.y);
		 * pos.z += entityToMove->GetDistanceFromCentreOfMassToBaseOfModel();
		 * entityToMove->Teleport(pos);
		 * CTheScripts::ClearSpaceForMissionEntity(pos, entityToMove);
		 *
		 */
		return 0;
	}
	case COMMAND_IS_CHAR_STILL_ALIVE:
	{
		CollectParameters(&m_nIp, 1);
		CPed* ped = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		UpdateCompareFlag(ped && ped->GetPedState() != PED_DEAD && ped->GetPedState() != PED_DIE);
		return 0;
	}
	case COMMAND_IS_CHAR_IN_AREA_2D:
	{
		CollectParameters(&m_nIp, 6);
		CPed* ped = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		assert(ped);
		CVehicle* vehicle;
		if (ped->bInVehicle)
			vehicle = ped->m_pMyVehicle;
		else
			vehicle = nil;
		float x1 = *(float*)&ScriptParams[1];
		float y1 = *(float*)&ScriptParams[2];
		float x2 = *(float*)&ScriptParams[3];
		float y2 = *(float*)&ScriptParams[4];
		if (vehicle)
			UpdateCompareFlag(ped->m_pMyVehicle->IsWithinArea(x1, y1, x2, y2));
		else
			UpdateCompareFlag(ped->IsWithinArea(x1, y1, x2, y2));
		if (!ScriptParams[5])
			return 0;
		CTheScripts::HighlightImportantArea((uint32)this + m_nIp, x1, y1, x2, y2, MAP_Z_LOW_LIMIT);
		if (CTheScripts::DbgFlag)
			CTheScripts::DrawDebugSquare(x1, y1, x2, y2);
		return 0;
	}
	case COMMAND_IS_CHAR_IN_AREA_3D:
	{
		CollectParameters(&m_nIp, 8);
		CPed* ped = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		assert(ped);
		CVehicle* vehicle;
		if (ped->bInVehicle)
			vehicle = ped->m_pMyVehicle;
		else
			vehicle = nil;
		float x1 = *(float*)&ScriptParams[1];
		float y1 = *(float*)&ScriptParams[2];
		float z1 = *(float*)&ScriptParams[3];
		float x2 = *(float*)&ScriptParams[4];
		float y2 = *(float*)&ScriptParams[5];
		float z2 = *(float*)&ScriptParams[6];
		if (vehicle)
			UpdateCompareFlag(ped->m_pMyVehicle->IsWithinArea(x1, y1, z1, x2, y2, z2));
		else
			UpdateCompareFlag(ped->IsWithinArea(x1, y1, z1, x2, y2, z2));
		if (!ScriptParams[7])
			return 0;
		CTheScripts::HighlightImportantArea((uint32)this + m_nIp, x1, y1, x2, y2, (z1 + z2) / 2);
		if (CTheScripts::DbgFlag)
			CTheScripts::DrawDebugCube(x1, y1, z1, x2, y2, z2);
		return 0;
	}
	case COMMAND_CREATE_CAR:
	{
		CollectParameters(&m_nIp, 4);
		int32 handle;
		if (CModelInfo::IsBoatModel(ScriptParams[0])) {
			CBoat* boat = new CBoat(ScriptParams[0], MISSION_VEHICLE);
			CVector pos = *(CVector*)&ScriptParams[1];
			if (pos.z <= MAP_Z_LOW_LIMIT)
				pos.z = CWorld::FindGroundZForCoord(pos.x, pos.y);
			pos.z += boat->GetDistanceFromCentreOfMassToBaseOfModel();
			boat->GetPosition() = pos;
			CTheScripts::ClearSpaceForMissionEntity(pos, boat);
			boat->m_status = STATUS_ABANDONED;
			boat->bIsLocked = true;
			boat->AutoPilot.m_nCarMission = MISSION_NONE;
			boat->AutoPilot.m_nTempAction = TEMPACT_NONE; /* Animation ID? */
			boat->AutoPilot.m_nCruiseSpeed = boat->AutoPilot.m_fMaxTrafficSpeed = 20.0f;
			CWorld::Add(boat);
			handle = CPools::GetVehiclePool()->GetIndex(boat);
		}
		else {
			CVehicle* car;
			if (!CModelInfo::IsBikeModel(ScriptParams[0]))
				car = new CAutomobile(ScriptParams[0], MISSION_VEHICLE);
			CVector pos = *(CVector*)&ScriptParams[1];
			if (pos.z <= MAP_Z_LOW_LIMIT)
				pos.z = CWorld::FindGroundZForCoord(pos.x, pos.y);
			pos.z += car->GetDistanceFromCentreOfMassToBaseOfModel();
			car->GetPosition() = pos;
			CTheScripts::ClearSpaceForMissionEntity(pos, car);
			car->m_status = STATUS_ABANDONED;
			car->bIsLocked = true;
			CCarCtrl::JoinCarWithRoadSystem(car);
			car->AutoPilot.m_nCarMission = MISSION_NONE;
			car->AutoPilot.m_nTempAction = TEMPACT_NONE; /* Animation ID? */
			car->AutoPilot.m_nDrivingStyle = DRIVINGSTYLE_STOP_FOR_CARS;
			car->AutoPilot.m_nCruiseSpeed = car->AutoPilot.m_fMaxTrafficSpeed = 9.0f;
			car->AutoPilot.m_nCurrentLane = car->AutoPilot.m_nNextLane = 0;
			car->bEngineOn = false;
			car->m_nZoneLevel = CTheZones::GetLevelFromPosition(pos);
			car->bHasBeenOwnedByPlayer = true;
			CWorld::Add(car);
			handle = CPools::GetVehiclePool()->GetIndex(car);
		}
		ScriptParams[0] = handle;
		StoreParameters(&m_nIp, 1);
		if (m_bIsMissionScript)
			CTheScripts::MissionCleanup.AddEntityToList(handle, CLEANUP_CAR);
		return 0;
	}
	case COMMAND_DELETE_CAR:
	{
		CollectParameters(&m_nIp, 1);
		CVehicle* car = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		if (car) {
			CWorld::Remove(car);
			CWorld::RemoveReferencesToDeletedObject(car);
			delete car;
		}
		if (m_bIsMissionScript)
			CTheScripts::MissionCleanup.RemoveEntityFromList(ScriptParams[0], CLEANUP_CAR);
		return 0;
	}
	case COMMAND_CAR_GOTO_COORDINATES:
	{
		CollectParameters(&m_nIp, 4);
		CVehicle* car = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		assert(car);
		CVector pos = *(CVector*)&ScriptParams[1];
		if (pos.z <= MAP_Z_LOW_LIMIT)
			pos.z = CWorld::FindGroundZForCoord(pos.x, pos.y);
		pos.z += car->GetDistanceFromCentreOfMassToBaseOfModel();
		if (CCarCtrl::JoinCarWithRoadSystemGotoCoors(car, pos, false))
			car->AutoPilot.m_nCarMission = MISSION_GOTOCOORDS_STRAIGHT;
		else
			car->AutoPilot.m_nCarMission = MISSION_GOTOCOORDS;
		car->m_status = STATUS_PHYSICS;
		car->bEngineOn = true;
		car->AutoPilot.m_nCruiseSpeed = max(car->AutoPilot.m_nCruiseSpeed, 6);
		car->AutoPilot.m_nAntiReverseTimer = CTimer::GetTimeInMilliseconds();
		return 0;
	}
	case COMMAND_CAR_WANDER_RANDOMLY:
	{
		CollectParameters(&m_nIp, 1);
		CVehicle* car = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		assert(car);
		CCarCtrl::JoinCarWithRoadSystem(car);
		car->AutoPilot.m_nCarMission = MISSION_CRUISE;
		car->bEngineOn = true;
		car->AutoPilot.m_nCruiseSpeed = max(car->AutoPilot.m_nCruiseSpeed, 6);
		car->AutoPilot.m_nAntiReverseTimer = CTimer::GetTimeInMilliseconds();
		return 0;
	}
	case COMMAND_CAR_SET_IDLE:
	{
		CollectParameters(&m_nIp, 1);
		CVehicle* car = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		assert(car);
		car->AutoPilot.m_nCarMission = MISSION_NONE;
		return 0;
	}
	case COMMAND_GET_CAR_COORDINATES:
	{
		CollectParameters(&m_nIp, 1);
		CVehicle* car = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		assert(car);
		*(CVector*)&ScriptParams[0] = car->GetPosition();
		StoreParameters(&m_nIp, 3);
		return 0;
	}
	case COMMAND_SET_CAR_COORDINATES:
	{
		CollectParameters(&m_nIp, 4);
		CVehicle* car = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		assert(car);
		CVector pos = *(CVector*)&ScriptParams[1];
		if (pos.z <= MAP_Z_LOW_LIMIT)
			pos.z = CWorld::FindGroundZForCoord(pos.x, pos.y);
		pos.z += car->GetDistanceFromCentreOfMassToBaseOfModel();
		car->bIsStatic = false;
		/* Again weird usage of virtual functions. */
		if (car->IsBoat()) {
			car->Teleport(pos);
			CTheScripts::ClearSpaceForMissionEntity(pos, car);
		}
		else {
			car->Teleport(pos);
			CTheScripts::ClearSpaceForMissionEntity(pos, car);
			/* May the following be inlined CCarCtrl function? */
			switch (car->AutoPilot.m_nCarMission) {
			case MISSION_CRUISE:
				CCarCtrl::JoinCarWithRoadSystem(car);
				break;
			case MISSION_RAMPLAYER_FARAWAY:
			case MISSION_RAMPLAYER_CLOSE:
			case MISSION_BLOCKPLAYER_FARAWAY:
			case MISSION_BLOCKPLAYER_CLOSE:
			case MISSION_BLOCKPLAYER_HANDBRAKESTOP:
				CCarCtrl::JoinCarWithRoadSystemGotoCoors(car, FindPlayerCoors(), false);
				break;
			case MISSION_GOTOCOORDS:
			case MISSION_GOTOCOORDS_STRAIGHT:
				CCarCtrl::JoinCarWithRoadSystemGotoCoors(car, car->AutoPilot.m_vecDestinationCoors, false);
				break;
			case MISSION_GOTOCOORDS_ACCURATE:
			case MISSION_GOTO_COORDS_STRAIGHT_ACCURATE:
				CCarCtrl::JoinCarWithRoadSystemGotoCoors(car, car->AutoPilot.m_vecDestinationCoors, false);
				break;
			case MISSION_RAMCAR_FARAWAY:
			case MISSION_RAMCAR_CLOSE:
			case MISSION_BLOCKCAR_FARAWAY:
			case MISSION_BLOCKCAR_CLOSE:
			case MISSION_BLOCKCAR_HANDBRAKESTOP:
				CCarCtrl::JoinCarWithRoadSystemGotoCoors(car, car->AutoPilot.m_pTargetCar->GetPosition(), false);
				break;
			default:
				break;
			}
		}
		return 0;
	}
	case COMMAND_IS_CAR_STILL_ALIVE:
	{
		CollectParameters(&m_nIp, 1);
		CVehicle* car = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		UpdateCompareFlag(car && car->m_status != STATUS_WRECKED && (car->IsBoat() || !car->bIsInWater));
		return 0;
	}
	case COMMAND_SET_CAR_CRUISE_SPEED:
	{
		CollectParameters(&m_nIp, 2);
		CVehicle* car = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		assert(car);
		car->AutoPilot.m_nCruiseSpeed = min(*(float*)&ScriptParams[1], 60.0f * car->pHandling->Transmission.fUnkMaxVelocity);
		return 0;
	}
	case COMMAND_SET_CAR_DRIVING_STYLE:
	{
		CollectParameters(&m_nIp, 2);
		CVehicle* car = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		assert(car);
		car->AutoPilot.m_nDrivingStyle = (eCarDrivingStyle)ScriptParams[1];
		return 0;
	}
	case COMMAND_SET_CAR_MISSION:
	{
		CollectParameters(&m_nIp, 2);
		CVehicle* car = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		assert(car);
		car->AutoPilot.m_nCarMission = (eCarMission)ScriptParams[1];
		car->AutoPilot.m_nAntiReverseTimer = CTimer::GetTimeInMilliseconds();
		car->bEngineOn = true;
		return 0;
	}
	case COMMAND_IS_CAR_IN_AREA_2D:
	{
		CollectParameters(&m_nIp, 6);
		CVehicle* vehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		assert(vehicle);
		float x1 = *(float*)&ScriptParams[1];
		float y1 = *(float*)&ScriptParams[2];
		float x2 = *(float*)&ScriptParams[3];
		float y2 = *(float*)&ScriptParams[4];
		UpdateCompareFlag(vehicle->IsWithinArea(x1, y1, x2, y2));
		if (!ScriptParams[5])
			return 0;
		CTheScripts::HighlightImportantArea((uint32)this + m_nIp, x1, y1, x2, y2, MAP_Z_LOW_LIMIT);
		if (CTheScripts::DbgFlag)
			CTheScripts::DrawDebugSquare(x1, y1, x2, y2);
		return 0;
	}
	case COMMAND_IS_CAR_IN_AREA_3D:
	{
		CollectParameters(&m_nIp, 8);
		CVehicle* vehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		assert(vehicle);
		float x1 = *(float*)&ScriptParams[1];
		float y1 = *(float*)&ScriptParams[2];
		float z1 = *(float*)&ScriptParams[3];
		float x2 = *(float*)&ScriptParams[4];
		float y2 = *(float*)&ScriptParams[5];
		float z2 = *(float*)&ScriptParams[6];
		UpdateCompareFlag(vehicle->IsWithinArea(x1, y1, z1, x2, y2, z2));
		if (!ScriptParams[7])
			return 0;
		CTheScripts::HighlightImportantArea((uint32)this + m_nIp, x1, y1, x2, y2, (z1 + z2) / 2);
		if (CTheScripts::DbgFlag)
			CTheScripts::DrawDebugCube(x1, y1, z1, x2, y2, z2);
		return 0;
	}
	case COMMAND_SPECIAL_0:
	case COMMAND_SPECIAL_1:
	case COMMAND_SPECIAL_2:
	case COMMAND_SPECIAL_3:
	case COMMAND_SPECIAL_4:
	case COMMAND_SPECIAL_5:
	case COMMAND_SPECIAL_6:
	case COMMAND_SPECIAL_7:
		assert(0);
		return 0;
	case COMMAND_PRINT_BIG:
	{
		wchar* key = TheText.Get((char*)&CTheScripts::ScriptSpace[m_nIp]);
		m_nIp += 8;
		CollectParameters(&m_nIp, 2);
		CMessages::AddBigMessage(key, ScriptParams[0], ScriptParams[1] - 1);
		return 0;
	}
	case COMMAND_PRINT:
	{
		wchar* key = TheText.Get((char*)&CTheScripts::ScriptSpace[m_nIp]);
		m_nIp += 8;
		CollectParameters(&m_nIp, 2);
		CMessages::AddMessage(key, ScriptParams[0], ScriptParams[1]);
		return 0;
	}
	case COMMAND_PRINT_NOW:
	{
		wchar* key = TheText.Get((char*)&CTheScripts::ScriptSpace[m_nIp]);
		m_nIp += 8;
		CollectParameters(&m_nIp, 2);
		CMessages::AddMessageJumpQ(key, ScriptParams[0], ScriptParams[1]);
		return 0;
	}
	case COMMAND_PRINT_SOON:
	{
		wchar* key = TheText.Get((char*)&CTheScripts::ScriptSpace[m_nIp]);
		m_nIp += 8;
		CollectParameters(&m_nIp, 2);
		CMessages::AddMessageSoon(key, ScriptParams[0], ScriptParams[1]);
		return 0;
	}
	case COMMAND_CLEAR_PRINTS:
		CMessages::ClearMessages();
		return 0;
	case COMMAND_GET_TIME_OF_DAY:
		ScriptParams[0] = CClock::GetHours();
		ScriptParams[1] = CClock::GetMinutes();
		StoreParameters(&m_nIp, 2);
		return 0;
	case COMMAND_SET_TIME_OF_DAY:
		CollectParameters(&m_nIp, 2);
		CClock::SetGameClock(ScriptParams[0], ScriptParams[1]);
		return 0;
	case COMMAND_GET_MINUTES_TO_TIME_OF_DAY:
		CollectParameters(&m_nIp, 2);
		ScriptParams[0] = CClock::GetGameClockMinutesUntil(ScriptParams[0], ScriptParams[1]);
		StoreParameters(&m_nIp, 1);
		return 0;
	case COMMAND_IS_POINT_ON_SCREEN:
	{
		CollectParameters(&m_nIp, 4);
		CVector pos = *(CVector*)&ScriptParams[0];
		if (pos.z <= -100)
			pos.z = CWorld::FindGroundZForCoord(pos.x, pos.y);
		UpdateCompareFlag(TheCamera.IsSphereVisible(pos, *(float*)&ScriptParams[3]));
	}
	case COMMAND_DEBUG_ON:
		CTheScripts::DbgFlag = true;
		return 0;
	case COMMAND_DEBUG_OFF:
		CTheScripts::DbgFlag = false;
		return 0;
	case COMMAND_RETURN_TRUE:
		UpdateCompareFlag(true);
		return 0;
	case COMMAND_RETURN_FALSE:
		UpdateCompareFlag(false);
		return 0;
	/* Special command only used by compiler. 
	case COMMAND_VAR_INT:
	*/
	default:
		assert(0);
		break;
	}
	return -1;
}

int8 CRunningScript::ProcessCommandsFrom200To299(int32 command)
{ 
	switch (command) {
	/* Special commands. 
	case COMMAND_VAR_FLOAT:
	case COMMAND_LVAR_INT:
	case COMMAND_LVAR_FLOAT:
	case COMMAND_LBRACKET:
	case COMMAND_RBRACKET:
	case COMMAND_REPEAT:
	case COMMAND_ENDREPEAT:
	case COMMAND_IF:
	case COMMAND_IFNOT:
	case COMMAND_ELSE:
	case COMMAND_ENDIF:
	case COMMAND_WHILE:
	case COMMAND_WHILENOT:
	case COMMAND_ENDWHILE:
	*/
	case COMMAND_ANDOR:
		CollectParameters(&m_nIp, 1);
		m_nAndOrState = ScriptParams[0];
		if (m_nAndOrState == ANDOR_NONE){
			m_bCondResult = false; // pointless
		}else if (m_nAndOrState >= ANDS_1 && m_nAndOrState <= ANDS_8){
			m_bCondResult = true;
			m_nAndOrState++;
		}else if (m_nAndOrState >= ORS_1 && m_nAndOrState <= ORS_8){
			m_bCondResult = false;
			m_nAndOrState++;
		}else{
			assert(0 && "COMMAND_ANDOR: invalid ANDOR state");
		}
		return 0;
	case COMMAND_LAUNCH_MISSION:
	{
		CollectParameters(&m_nIp, 1);
		CRunningScript* pNew = CTheScripts::StartNewScript(ScriptParams[0]);
		pNew->m_bIsMissionScript = true;
		return 0;
	}
	case COMMAND_MISSION_HAS_FINISHED:
	{
		if (!m_bIsMissionScript)
			return 0;
		if (strcmp(m_abScriptName, "love3") == 0) /* A Drop in the Ocean */
			CPickups::RemoveAllFloatingPickups();
		CTheScripts::MissionCleanup.Process();
		return 0;
	}
	case COMMAND_STORE_CAR_CHAR_IS_IN:
	{
		CollectParameters(&m_nIp, 1);
		CPed* ped = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		assert(ped);
		CVehicle* pCurrent = nil;
		if (ped->bInVehicle) {
			pCurrent = ped->m_pMyVehicle;
		}
		assert(pCurrent); // GetIndex(0) doesn't look good
		int handle = CPools::GetVehiclePool()->GetIndex(pCurrent);
		if (handle != CTheScripts::StoreVehicleIndex && m_bIsMissionScript){
#ifdef FIX_BUGS
			if (CTheScripts::StoreVehicleIndex != -1)
#endif
			{
				CVehicle* pOld = CPools::GetVehiclePool()->GetAt(CTheScripts::StoreVehicleIndex);
				if (pOld){
					CCarCtrl::RemoveFromInterestingVehicleList(pOld);
					if (pOld->VehicleCreatedBy == MISSION_VEHICLE && CTheScripts::StoreVehicleWasRandom){
						pOld->VehicleCreatedBy = RANDOM_VEHICLE;
						pOld->bIsLocked = false;
						CCarCtrl::NumRandomCars++;
						CCarCtrl::NumMissionCars--;
						CTheScripts::MissionCleanup.RemoveEntityFromList(CTheScripts::StoreVehicleIndex, CLEANUP_CAR);
					}
				}
			}

			CTheScripts::StoreVehicleIndex = handle;
			switch (pCurrent->VehicleCreatedBy){
			case RANDOM_VEHICLE:
				pCurrent->VehicleCreatedBy = MISSION_VEHICLE;
				CCarCtrl::NumMissionCars++;
				CCarCtrl::NumRandomCars--;
				CTheScripts::StoreVehicleWasRandom = true;
				CTheScripts::MissionCleanup.AddEntityToList(CTheScripts::StoreVehicleIndex, CLEANUP_CAR);
				break;
			case PARKED_VEHICLE:
				pCurrent->VehicleCreatedBy = MISSION_VEHICLE;
				CCarCtrl::NumMissionCars++;
				CCarCtrl::NumParkedCars--;
				CTheScripts::StoreVehicleWasRandom = true;
				CTheScripts::MissionCleanup.AddEntityToList(CTheScripts::StoreVehicleIndex, CLEANUP_CAR);
				break;
			case MISSION_VEHICLE:
			case PERMANENT_VEHICLE:
				CTheScripts::StoreVehicleWasRandom = false;
				break;
			default:
				break;
			}
		}
		ScriptParams[0] = CTheScripts::StoreVehicleIndex;
		StoreParameters(&m_nIp, 1);
		return 0;
	}
	case COMMAND_STORE_CAR_PLAYER_IS_IN:
	{
		CollectParameters(&m_nIp, 1);
		CPed* ped = CWorld::Players[ScriptParams[0]].m_pPed;
		assert(ped);
		if (!ped->bInVehicle)
			return 0; // No value written to output variable
		CVehicle* pCurrent = ped->m_pMyVehicle;
		assert(pCurrent); // Here pCurrent shouldn't be NULL anyway
		int handle = CPools::GetVehiclePool()->GetIndex(pCurrent);
		if (handle != CTheScripts::StoreVehicleIndex && m_bIsMissionScript) {
#ifdef FIX_BUGS
			if (CTheScripts::StoreVehicleIndex != -1)
#endif
			{
				CVehicle* pOld = CPools::GetVehiclePool()->GetAt(CTheScripts::StoreVehicleIndex);
				if (pOld){
					CCarCtrl::RemoveFromInterestingVehicleList(pOld);
					if (pOld->VehicleCreatedBy == MISSION_VEHICLE && CTheScripts::StoreVehicleWasRandom){
						pOld->VehicleCreatedBy = RANDOM_VEHICLE;
						pOld->bIsLocked = false;
						CCarCtrl::NumRandomCars++;
						CCarCtrl::NumMissionCars--;
						CTheScripts::MissionCleanup.RemoveEntityFromList(CTheScripts::StoreVehicleIndex, CLEANUP_CAR);
					}
				}
			}

			CTheScripts::StoreVehicleIndex = handle;
			switch (pCurrent->VehicleCreatedBy) {
			case RANDOM_VEHICLE:
				pCurrent->VehicleCreatedBy = MISSION_VEHICLE;
				CCarCtrl::NumMissionCars++;
				CCarCtrl::NumRandomCars--;
				CTheScripts::StoreVehicleWasRandom = true;
				CTheScripts::MissionCleanup.AddEntityToList(CTheScripts::StoreVehicleIndex, CLEANUP_CAR);
				break;
			case PARKED_VEHICLE:
				pCurrent->VehicleCreatedBy = MISSION_VEHICLE;
				CCarCtrl::NumMissionCars++;
				CCarCtrl::NumParkedCars--;
				CTheScripts::StoreVehicleWasRandom = true;
				CTheScripts::MissionCleanup.AddEntityToList(CTheScripts::StoreVehicleIndex, CLEANUP_CAR);
				break;
			case MISSION_VEHICLE:
			case PERMANENT_VEHICLE:
				CTheScripts::StoreVehicleWasRandom = false;
				break;
			default:
				break;
			}
		}
		ScriptParams[0] = CTheScripts::StoreVehicleIndex;
		StoreParameters(&m_nIp, 1);
		return 0;
	}
	case COMMAND_IS_CHAR_IN_CAR:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		CVehicle* pCheckedVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[1]);
		CVehicle* pActualVehicle = pPed->bInVehicle ? pPed->m_pMyVehicle : nil;
		UpdateCompareFlag(pActualVehicle && pActualVehicle == pCheckedVehicle);
		return 0;
	}
	case COMMAND_IS_PLAYER_IN_CAR:
	{
		CollectParameters(&m_nIp, 2);
		CVehicle* pCheckedVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[1]);
		CPed* pPed = CWorld::Players[ScriptParams[0]].m_pPed;
		UpdateCompareFlag(pPed->bInVehicle && pPed->m_pMyVehicle == pCheckedVehicle);
		return 0;
	}
	case COMMAND_IS_CHAR_IN_MODEL:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		CVehicle* pActualVehicle = pPed->bInVehicle ? pPed->m_pMyVehicle : nil;
		UpdateCompareFlag(pActualVehicle && pActualVehicle->GetModelIndex() == ScriptParams[1]);
		return 0;
	}
	case COMMAND_IS_PLAYER_IN_MODEL:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CWorld::Players[ScriptParams[0]].m_pPed;
		UpdateCompareFlag(pPed->bInVehicle && pPed->m_pMyVehicle->GetModelIndex() == ScriptParams[1]);
		return 0;
	}
	case COMMAND_IS_CHAR_IN_ANY_CAR:
	{
		CollectParameters(&m_nIp, 1);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		UpdateCompareFlag(pPed->bInVehicle);
		return 0;
	}
	case COMMAND_IS_PLAYER_IN_ANY_CAR:
	{
		CollectParameters(&m_nIp, 1);
		CPed* pPed = CWorld::Players[ScriptParams[0]].m_pPed;
		UpdateCompareFlag(pPed->bInVehicle);
		return 0;
	}
	case COMMAND_IS_BUTTON_PRESSED:
	{
		CollectParameters(&m_nIp, 2);
		bool value = GetPadState(ScriptParams[0], ScriptParams[1]) != 0;
		if (CGame::playingIntro && ScriptParams[0] == 0 && ScriptParams[1] == 12){ /* pad1, start */
			if (CPad::GetPad(0)->GetLeftMouseJustDown() ||
				CPad::GetPad(0)->GetPadEnterJustDown() ||
				CPad::GetPad(0)->GetCharJustDown(' '))
				value = true;
		}
		UpdateCompareFlag(value);
		return 0;
	}
	case COMMAND_GET_PAD_STATE:
	{
		CollectParameters(&m_nIp, 1);
		ScriptParams[0] = GetPadState(ScriptParams[0], ScriptParams[1]);
		StoreParameters(&m_nIp, 1);
		return 0;
	}
	case COMMAND_LOCATE_PLAYER_ANY_MEANS_2D:
	case COMMAND_LOCATE_PLAYER_ON_FOOT_2D:
	case COMMAND_LOCATE_PLAYER_IN_CAR_2D:
	case COMMAND_LOCATE_STOPPED_PLAYER_ANY_MEANS_2D:
	case COMMAND_LOCATE_STOPPED_PLAYER_ON_FOOT_2D:
	case COMMAND_LOCATE_STOPPED_PLAYER_IN_CAR_2D:
		LocatePlayerCommand(command, &m_nIp);
		return 0;
	case COMMAND_LOCATE_PLAYER_ANY_MEANS_CHAR_2D:
	case COMMAND_LOCATE_PLAYER_ON_FOOT_CHAR_2D:
	case COMMAND_LOCATE_PLAYER_IN_CAR_CHAR_2D:
		LocatePlayerCharCommand(command, &m_nIp);
		return 0;
	case COMMAND_LOCATE_CHAR_ANY_MEANS_2D:
	case COMMAND_LOCATE_CHAR_ON_FOOT_2D:
	case COMMAND_LOCATE_CHAR_IN_CAR_2D:
	case COMMAND_LOCATE_STOPPED_CHAR_ANY_MEANS_2D:
	case COMMAND_LOCATE_STOPPED_CHAR_ON_FOOT_2D:
	case COMMAND_LOCATE_STOPPED_CHAR_IN_CAR_2D:
		LocateCharCommand(command, &m_nIp);
		return 0;
	case COMMAND_LOCATE_CHAR_ANY_MEANS_CHAR_2D:
	case COMMAND_LOCATE_CHAR_ON_FOOT_CHAR_2D:
	case COMMAND_LOCATE_CHAR_IN_CAR_CHAR_2D:
		LocateCharCharCommand(command, &m_nIp);
		return 0;
	case COMMAND_LOCATE_PLAYER_ANY_MEANS_3D:
	case COMMAND_LOCATE_PLAYER_ON_FOOT_3D:
	case COMMAND_LOCATE_PLAYER_IN_CAR_3D:
	case COMMAND_LOCATE_STOPPED_PLAYER_ANY_MEANS_3D:
	case COMMAND_LOCATE_STOPPED_PLAYER_ON_FOOT_3D:
	case COMMAND_LOCATE_STOPPED_PLAYER_IN_CAR_3D:
		LocatePlayerCommand(command, &m_nIp);
		return 0;
	case COMMAND_LOCATE_PLAYER_ANY_MEANS_CHAR_3D:
	case COMMAND_LOCATE_PLAYER_ON_FOOT_CHAR_3D:
	case COMMAND_LOCATE_PLAYER_IN_CAR_CHAR_3D:
		LocatePlayerCharCommand(command, &m_nIp);
		return 0;
	case COMMAND_LOCATE_CHAR_ANY_MEANS_3D:
	case COMMAND_LOCATE_CHAR_ON_FOOT_3D:
	case COMMAND_LOCATE_CHAR_IN_CAR_3D:
	case COMMAND_LOCATE_STOPPED_CHAR_ANY_MEANS_3D:
	case COMMAND_LOCATE_STOPPED_CHAR_ON_FOOT_3D:
	case COMMAND_LOCATE_STOPPED_CHAR_IN_CAR_3D:
		LocateCharCommand(command, &m_nIp);
		return 0;
	case COMMAND_LOCATE_CHAR_ANY_MEANS_CHAR_3D:
	case COMMAND_LOCATE_CHAR_ON_FOOT_CHAR_3D:
	case COMMAND_LOCATE_CHAR_IN_CAR_CHAR_3D:
		LocateCharCharCommand(command, &m_nIp);
		return 0;
	case COMMAND_CREATE_OBJECT:
	{
		CollectParameters(&m_nIp, 4);
		int mi = ScriptParams[0] >= 0 ? ScriptParams[0] : CTheScripts::UsedObjectArray[-ScriptParams[0]].index;
		CObject* pObj = new CObject(mi, false);
		pObj->ObjectCreatedBy = MISSION_OBJECT;
		CVector pos = *(CVector*)&ScriptParams[1];
		if (pos.z <= MAP_Z_LOW_LIMIT)
			pos.z = CWorld::FindGroundZForCoord(pos.x, pos.y);
		pos.z += pObj->GetDistanceFromCentreOfMassToBaseOfModel();
		pObj->GetPosition() = pos;
		pObj->SetOrientation(0.0f, 0.0f, 0.0f);
		pObj->GetMatrix().UpdateRW();
		pObj->UpdateRwFrame();
		CTheScripts::ClearSpaceForMissionEntity(pos, pObj);
		CWorld::Add(pObj);
		ScriptParams[0] = CPools::GetObjectPool()->GetIndex(pObj);
		StoreParameters(&m_nIp, 1);
		if (m_bIsMissionScript)
			CTheScripts::MissionCleanup.AddEntityToList(ScriptParams[0], CLEANUP_OBJECT);
		return 0;
	}
	case COMMAND_DELETE_OBJECT:
	{
		CollectParameters(&m_nIp, 1);
		CObject* pObj = CPools::GetObjectPool()->GetAt(ScriptParams[0]);
		if (pObj){
			CWorld::Remove(pObj);
			CWorld::RemoveReferencesToDeletedObject(pObj);
			delete pObj;
		}
		if (m_bIsMissionScript)
			CTheScripts::MissionCleanup.RemoveEntityFromList(ScriptParams[0], CLEANUP_OBJECT);
		return 0;
	}
	case COMMAND_ADD_SCORE:
		CollectParameters(&m_nIp, 2);
		CWorld::Players[ScriptParams[0]].m_nMoney += ScriptParams[1];
		return 0;
	case COMMAND_IS_SCORE_GREATER:
		CollectParameters(&m_nIp, 2);
		UpdateCompareFlag(CWorld::Players[ScriptParams[0]].m_nMoney > ScriptParams[1]);
		return 0;
	case COMMAND_STORE_SCORE:
		CollectParameters(&m_nIp, 1);
		ScriptParams[0] = CWorld::Players[ScriptParams[0]].m_nMoney;
		StoreParameters(&m_nIp, 1);
		return 0;
	case COMMAND_GIVE_REMOTE_CONTROLLED_CAR_TO_PLAYER:
	{
		CollectParameters(&m_nIp, 5);
		CVector pos = *(CVector*)&ScriptParams[1];
		if (pos.z <= MAP_Z_LOW_LIMIT)
			pos.z = CWorld::FindGroundZForCoord(pos.x, pos.y);
		CRemote::GivePlayerRemoteControlledCar(pos.x, pos.y, pos.z, DEGTORAD(*(float*)&ScriptParams[4]), MI_RCBANDIT);
		return 0;
	}
	case COMMAND_ALTER_WANTED_LEVEL:
		CollectParameters(&m_nIp, 2);
		CWorld::Players[ScriptParams[0]].m_pPed->SetWantedLevel(ScriptParams[1]);
		return 0;
	case COMMAND_ALTER_WANTED_LEVEL_NO_DROP:
		CollectParameters(&m_nIp, 2);
		CWorld::Players[ScriptParams[0]].m_pPed->SetWantedLevelNoDrop(ScriptParams[1]);
		return 0;
	case COMMAND_IS_WANTED_LEVEL_GREATER:
		CollectParameters(&m_nIp, 2);
		UpdateCompareFlag(CWorld::Players[ScriptParams[0]].m_pPed->m_pWanted->m_nWantedLevel > ScriptParams[1]);
		return 0;
	case COMMAND_CLEAR_WANTED_LEVEL:
		CollectParameters(&m_nIp, 1);
		CWorld::Players[ScriptParams[0]].m_pPed->SetWantedLevel(0);
		return 0;
	case COMMAND_SET_DEATHARREST_STATE:
		CollectParameters(&m_nIp, 1);
		m_bDeatharrestEnabled = (ScriptParams[0] == 1);
		return 0;
	case COMMAND_HAS_DEATHARREST_BEEN_EXECUTED:
		UpdateCompareFlag(m_bDeatharrestExecuted);
		return 0;
	case COMMAND_ADD_AMMO_TO_PLAYER:
	{
		CollectParameters(&m_nIp, 3);
		CWorld::Players[ScriptParams[0]].m_pPed->GrantAmmo((eWeaponType)ScriptParams[1], ScriptParams[2]);
		return 0;
	}
	case COMMAND_ADD_AMMO_TO_CHAR:
	{
		CollectParameters(&m_nIp, 3);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		assert(pPed);
		pPed->GrantAmmo((eWeaponType)ScriptParams[1], ScriptParams[2]);
		return 0;
	}
	/* Not implemented
	case COMMAND_ADD_AMMO_TO_CAR:
	case COMMAND_IS_PLAYER_STILL_ALIVE:
	*/
	case COMMAND_IS_PLAYER_DEAD:
		CollectParameters(&m_nIp, 1);
		UpdateCompareFlag(CWorld::Players[ScriptParams[0]].m_WBState == WBSTATE_WASTED);
		return 0;
	case COMMAND_IS_CHAR_DEAD:
	{
		CollectParameters(&m_nIp, 1);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		UpdateCompareFlag(!pPed || pPed->GetPedState() == PED_DIE || pPed->GetPedState() == PED_DEAD);
		return 0;
	}
	case COMMAND_IS_CAR_DEAD:
	{
		CollectParameters(&m_nIp, 1);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		UpdateCompareFlag(!pVehicle || pVehicle->m_status == STATUS_WRECKED || !pVehicle->IsBoat() && pVehicle->bIsInWater);
		return 0;
	}
	case COMMAND_SET_CHAR_THREAT_SEARCH:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		assert(pPed);
		pPed->m_fearFlags |= ScriptParams[1];
		return 0;
	}
	/* Not implemented. 
	case COMMAND_SET_CHAR_THREAT_REACTION:
	*/
	case COMMAND_SET_CHAR_OBJ_NO_OBJ:
	{
		CollectParameters(&m_nIp, 1);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		assert(pPed);
		pPed->bScriptObjectiveCompleted = false;
		pPed->ClearObjective();
		return 0;
	}
	/* Not implemented. 
	case COMMAND_ORDER_DRIVER_OUT_OF_CAR:
	case COMMAND_ORDER_CHAR_TO_DRIVE_CAR:
	case COMMAND_ADD_PATROL_POINT:
	case COMMAND_IS_PLAYER_IN_GANGZONE:
	*/
	case COMMAND_IS_PLAYER_IN_ZONE:
	{
		CollectParameters(&m_nIp, 1);
		CPlayerInfo* pPlayer = &CWorld::Players[ScriptParams[0]];
		char label[12];
		CTheScripts::ReadTextLabelFromScript(&m_nIp, label);
		int zoneToCheck = CTheZones::FindZoneByLabelAndReturnIndex(label);
		if (zoneToCheck != -1)
			m_nIp += 8; /* why only if zone != 1? */
		CVector pos = pPlayer->GetPos();
		CZone* pZone = CTheZones::GetZone(zoneToCheck);
		UpdateCompareFlag(CTheZones::PointLiesWithinZone(pos, pZone));
		return 0;
	}
	case COMMAND_IS_PLAYER_PRESSING_HORN:
		CollectParameters(&m_nIp, 1);
		UpdateCompareFlag(CWorld::Players[ScriptParams[0]].m_pPed->GetPedState() == PED_DRIVING &&
			CPad::GetPad(ScriptParams[0])->GetHorn());
		/* Is it correct that same parameter is used both as index of Players */
		/* and as ID of pad? Pratically this parameter is always 0 anyway of course. */
		return 0;
	case COMMAND_HAS_CHAR_SPOTTED_PLAYER:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		assert(pPed);
		UpdateCompareFlag(pPed->OurPedCanSeeThisOne(CWorld::Players[ScriptParams[1]].m_pPed));
		return 0;
	}
	/* Not implemented.
	case COMMAND_ORDER_CHAR_TO_BACKDOOR:
	case COMMAND_ADD_CHAR_TO_GANG:
	*/
	case COMMAND_IS_CHAR_OBJECTIVE_PASSED:
	{
		CollectParameters(&m_nIp, 1);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		assert(pPed);
		UpdateCompareFlag(pPed->bScriptObjectiveCompleted);
		return 0;
	}
	/* Not implemented. 
	case COMMAND_SET_CHAR_DRIVE_AGGRESSION:
	case COMMAND_SET_CHAR_MAX_DRIVESPEED:
	*/
	case COMMAND_CREATE_CHAR_INSIDE_CAR:
	{
		CollectParameters(&m_nIp, 3);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		assert(pVehicle);
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
			pPed = new CCivilianPed(ScriptParams[1], ScriptParams[2]);
		pPed->CharCreatedBy = MISSION_CHAR;
		pPed->bRespondsToThreats = false;
		pPed->bAllowMedicsToReviveMe = false;
		pPed->GetPosition() = pVehicle->GetPosition();
		pPed->SetOrientation(0.0f, 0.0f, 0.0f);
		pPed->SetPedState(PED_DRIVING);
		CPopulation::ms_nTotalMissionPeds++;
		assert(!pVehicle->pDriver);
		pVehicle->pDriver = pPed;
		pVehicle->pDriver->RegisterReference((CEntity**)&pVehicle->pDriver);
		pPed->m_pMyVehicle = pVehicle;
		pPed->m_pMyVehicle->RegisterReference((CEntity**)&pPed->m_pMyVehicle);
		pPed->bInVehicle = true;
		pVehicle->m_status = STATUS_PHYSICS;
		if (!pVehicle->IsBoat())
			pVehicle->AutoPilot.m_nCarMission = MISSION_CRUISE;
		pVehicle->bEngineOn = true;
		pPed->bUsesCollision = false;
		AnimationId anim = pVehicle->bLowVehicle ? ANIM_CAR_LSIT : ANIM_CAR_SIT;
		pPed->m_pVehicleAnim = CAnimManager::BlendAnimation(pPed->GetClump(), ASSOCGRP_STD, anim, 100.0f);
		pPed->StopNonPartialAnims();
		pPed->m_nZoneLevel = CTheZones::GetLevelFromPosition(pPed->GetPosition());
		CWorld::Add(pPed);
		ScriptParams[0] = CPools::GetPedPool()->GetIndex(pPed);
		StoreParameters(&m_nIp, 1);
		if (m_bIsMissionScript)
			CTheScripts::MissionCleanup.AddEntityToList(ScriptParams[0], CLEANUP_CHAR);
		return 0;
	}
	case COMMAND_WARP_PLAYER_FROM_CAR_TO_COORD:
	{
		CollectParameters(&m_nIp, 4);
		CVector pos = *(CVector*)&ScriptParams[1];
		CPlayerInfo* pPlayer = &CWorld::Players[ScriptParams[0]];
		if (pos.z <= MAP_Z_LOW_LIMIT)
			pos.z = CWorld::FindGroundZForCoord(pos.x, pos.y);
		if (pPlayer->m_pPed->bInVehicle){
			assert(pPlayer->m_pPed->m_pMyVehicle);
			if (pPlayer->m_pPed->m_pMyVehicle->bIsBus)
				pPlayer->m_pPed->bRenderPedInCar = true;
			if (pPlayer->m_pPed->m_pMyVehicle->pDriver == pPlayer->m_pPed){
				pPlayer->m_pPed->m_pMyVehicle->RemoveDriver();
				pPlayer->m_pPed->m_pMyVehicle->m_status = STATUS_ABANDONED;
				pPlayer->m_pPed->m_pMyVehicle->bEngineOn = false;
				pPlayer->m_pPed->m_pMyVehicle->AutoPilot.m_nCruiseSpeed = 0;
			}else{
				pPlayer->m_pPed->m_pMyVehicle->RemovePassenger(pPlayer->m_pPed);
			}
		}
		pPlayer->m_pPed->bInVehicle = false;
		pPlayer->m_pPed->m_pMyVehicle = nil;
		pPlayer->m_pPed->SetPedState(PED_IDLE);
		pPlayer->m_pPed->bUsesCollision = true;
		pPlayer->m_pPed->SetMoveSpeed(0.0f, 0.0f, 0.0f);
		pPlayer->m_pPed->AddWeaponModel(CWeaponInfo::GetWeaponInfo(pPlayer->m_pPed->GetWeapon()->m_eWeaponType)->m_nModelId);
		pPlayer->m_pPed->RemoveInCarAnims();
		if (pPlayer->m_pPed->m_pVehicleAnim)
			pPlayer->m_pPed->m_pVehicleAnim->blendDelta = -1000.0f;
		pPlayer->m_pPed->m_pVehicleAnim = nil;
		pPlayer->m_pPed->SetMoveState(PEDMOVE_NONE);
		CAnimManager::BlendAnimation(pPlayer->m_pPed->GetClump(), pPlayer->m_pPed->m_animGroup, ANIM_IDLE_STANCE, 100.0f);
		pPlayer->m_pPed->RestartNonPartialAnims();
		AudioManager.PlayerJustLeftCar();
		pos.z += pPlayer->m_pPed->GetDistanceFromCentreOfMassToBaseOfModel();
		pPlayer->m_pPed->Teleport(pos);
		CTheScripts::ClearSpaceForMissionEntity(pos, pPlayer->m_pPed);
		return 0;
	}
	/* Not implemented. 
	case COMMAND_MAKE_CHAR_DO_NOTHING:
	*/
	default:
		assert(0);
		break;
	}
	return -1;
}

#if 0
WRAPPER int8 CRunningScript::ProcessCommandsFrom300To399(int32 command) { EAXJMP(0x43ED30); }
#else
int8 CRunningScript::ProcessCommandsFrom300To399(int32 command)
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
		assert(pVehicle);
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
		assert(pVehicle);
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
		assert(pVehicle);
		float x1 = *(float*)&ScriptParams[1];
		float y1 = *(float*)&ScriptParams[2];
		float x2 = *(float*)&ScriptParams[3];
		float y2 = *(float*)&ScriptParams[4];
		UpdateCompareFlag(pVehicle->m_status == STATUS_WRECKED &&
			pVehicle->IsWithinArea(x1, y1, x2, y2));
		if (!ScriptParams[5])
			return 0;
		CTheScripts::HighlightImportantArea((uint32)this + m_nIp, x1, y1, x2, y2, MAP_Z_LOW_LIMIT);
		if (CTheScripts::DbgFlag)
			CTheScripts::DrawDebugSquare(x1, y1, x2, y2);
		return 0;
	}
	case COMMAND_IS_CAR_DEAD_IN_AREA_3D:
	{
		CollectParameters(&m_nIp, 8);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		assert(pVehicle);
		float x1 = *(float*)&ScriptParams[1];
		float y1 = *(float*)&ScriptParams[2];
		float z1 = *(float*)&ScriptParams[3];
		float x2 = *(float*)&ScriptParams[4];
		float y2 = *(float*)&ScriptParams[5];
		float z2 = *(float*)&ScriptParams[6];
		UpdateCompareFlag(pVehicle->m_status == STATUS_WRECKED &&
			pVehicle->IsWithinArea(x1, y1, z1, x2, y2, z2));
		if (!ScriptParams[7])
			return 0;
		CTheScripts::HighlightImportantArea((uint32)this + m_nIp, x1, y1, x2, y2, (z1 + z2) / 2);
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
		}else{
			pCarGen->SwitchOn();
			pCarGen->SetUsesRemaining(ScriptParams[1]);
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
		assert(CTheScripts::ScriptSpace[m_nIp] == ARGUMENT_GLOBALVAR);
		m_nIp++;
		CUserDisplay::OnscnTimer.AddClock(CTheScripts::Read2BytesFromScript(&m_nIp), nil);
		return 0;
	}
	case COMMAND_CLEAR_ONSCREEN_TIMER:
	{
		assert(CTheScripts::ScriptSpace[m_nIp] == ARGUMENT_GLOBALVAR);
		m_nIp++;
		CUserDisplay::OnscnTimer.ClearClock(CTheScripts::Read2BytesFromScript(&m_nIp));
		return 0;
	}
	case COMMAND_DISPLAY_ONSCREEN_COUNTER:
	{
		assert(CTheScripts::ScriptSpace[m_nIp] == ARGUMENT_GLOBALVAR);
		m_nIp++;
		int32 counter = CTheScripts::Read2BytesFromScript(&m_nIp);
		CollectParameters(&m_nIp, 1);
		CUserDisplay::OnscnTimer.AddCounter(counter, ScriptParams[0], nil);
		return 0;
	}
	case COMMAND_CLEAR_ONSCREEN_COUNTER:
	{
		assert(CTheScripts::ScriptSpace[m_nIp] == ARGUMENT_GLOBALVAR);
		m_nIp++;
		CUserDisplay::OnscnTimer.ClearCounter(CTheScripts::Read2BytesFromScript(&m_nIp));
		return 0;
	}
	case COMMAND_SET_ZONE_CAR_INFO:
	{
		char label[12];
		CTheScripts::ReadTextLabelFromScript(&m_nIp, label);
		m_nIp += 8;
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
		assert(pPed);
		char label[12];
		CTheScripts::ReadTextLabelFromScript(&m_nIp, label);
		int zone = CTheZones::FindZoneByLabelAndReturnIndex(label);
		if (zone != -1)
			m_nIp += 8;
		CVector pos = pPed->bInVehicle ? pPed->m_pMyVehicle->GetPosition() : pPed->GetPosition();
		UpdateCompareFlag(CTheZones::PointLiesWithinZone(pos, CTheZones::GetZone(zone)));
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
		m_nIp += 8;
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
		TheCamera.TakeControl(nil, ScriptParams[1], ScriptParams[2], 1);
		return 0;
	}
	case COMMAND_POINT_CAMERA_AT_CAR:
	{
		CollectParameters(&m_nIp, 3);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		assert(pVehicle);
		TheCamera.TakeControl(pVehicle, ScriptParams[1], ScriptParams[2], 1);
		return 0;
	}
	case COMMAND_POINT_CAMERA_AT_CHAR:
	{
		CollectParameters(&m_nIp, 3);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		assert(pPed);
		TheCamera.TakeControl(pPed, ScriptParams[1], ScriptParams[2], 1);
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
		m_nIp += 8;
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
		assert(pVehicle && pVehicle->IsCar());
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
		TheCamera.TakeControlNoEntity(pos, ScriptParams[3], 1);
		return 0;
	}
	case COMMAND_ADD_BLIP_FOR_CAR_OLD:
	{
		CollectParameters(&m_nIp, 3);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		assert(pVehicle);
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
		assert(pPed);
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
		assert(pObject);
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
			// Is assertion required?
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
		assert(pPed);
		float angle = pPed->bInVehicle ? pPed->m_pMyVehicle->GetForward().Heading() : pPed->GetForward().Heading();
		*(float*)&ScriptParams[0] = CGeneral::LimitAngle(RADTODEG(angle));
		StoreParameters(&m_nIp, 1);
		return 0;
	}
	case COMMAND_SET_CHAR_HEADING:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		assert(pPed);
		if (pPed->bInVehicle) {
			// Is assertion required?
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
		assert(pVehicle);
		float angle = pVehicle->GetForward().Heading();
		*(float*)&ScriptParams[0] = CGeneral::LimitAngle(RADTODEG(angle));
		StoreParameters(&m_nIp, 1);
		return 0;
	}
	case COMMAND_SET_CAR_HEADING:
	{
		CollectParameters(&m_nIp, 2);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		assert(pVehicle);
		pVehicle->SetHeading(DEGTORAD(*(float*)&ScriptParams[1]));
		return 0;
	}
	case COMMAND_GET_OBJECT_HEADING:
	{
		CollectParameters(&m_nIp, 1);
		CObject* pObject = CPools::GetObjectPool()->GetAt(ScriptParams[0]);
		assert(pObject);
		float angle = pObject->GetForward().Heading();
		*(float*)&ScriptParams[0] = CGeneral::LimitAngle(RADTODEG(angle));
		StoreParameters(&m_nIp, 1);
		return 0;
	}
	case COMMAND_SET_OBJECT_HEADING:
	{
		CollectParameters(&m_nIp, 2);
		CObject* pObject = CPools::GetObjectPool()->GetAt(ScriptParams[0]);
		assert(pObject);
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
		assert(pObject);
		CPhysical* pEntityToTest = pPed->bInVehicle ? (CPhysical*)pPed->m_pMyVehicle : pPed;
		UpdateCompareFlag(pEntityToTest->GetHasCollidedWith(pObject));
		return 0;
	}
	case COMMAND_IS_CHAR_TOUCHING_OBJECT:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		assert(pPed);
		CObject* pObject = CPools::GetObjectPool()->GetAt(ScriptParams[1]);
		assert(pObject);
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
		CTheScripts::OnAMissionFlag = CTheScripts::Read2BytesFromScript(&++m_nIp);
		return 0;
	case COMMAND_DECLARE_MISSION_FLAG_FOR_CONTACT:
		CollectParameters(&m_nIp, 1);
		CTheScripts::OnAMissionForContactFlag[ScriptParams[0]] = CTheScripts::Read2BytesFromScript(&++m_nIp);
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
		assert(pPed);
		UpdateCompareFlag(pPed->m_fHealth > ScriptParams[1]);
		return 0;
	}
	case COMMAND_IS_CAR_HEALTH_GREATER:
	{
		CollectParameters(&m_nIp, 2);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		assert(pVehicle);
		UpdateCompareFlag(pVehicle->m_fHealth > ScriptParams[1]);
		return 0;
	}
	case COMMAND_ADD_BLIP_FOR_CAR:
	{
		CollectParameters(&m_nIp, 1);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		assert(pVehicle);
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
		assert(pPed);
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
		assert(pObject);
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
		int handle = CRadar::SetCoordBlip(BLIP_COORD, pos, 2, BLIP_DISPLAY_BOTH);
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
		assert(pVehicle);
		UpdateCompareFlag(CTheScripts::UpsideDownCars.HasCarBeenUpsideDownForAWhile(ScriptParams[0]));
		return 0;
	}
	default:
		assert(0);
	}
	return -1;
}
#endif

#if 0
WRAPPER int8 CRunningScript::ProcessCommandsFrom400To499(int32 command) { EAXJMP(0x440CB0); }
#else
int8 CRunningScript::ProcessCommandsFrom400To499(int32 command)
{
	switch (command) {
	case COMMAND_ADD_UPSIDEDOWN_CAR_CHECK:
	{
		CollectParameters(&m_nIp, 1);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		assert(pVehicle);
		CTheScripts::UpsideDownCars.AddCarToCheck(ScriptParams[0]);
		return 0;
	}
	case COMMAND_REMOVE_UPSIDEDOWN_CAR_CHECK:
	{
		CollectParameters(&m_nIp, 1);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		assert(pVehicle);
		CTheScripts::UpsideDownCars.RemoveCarFromCheck(ScriptParams[0]);
		return 0;
	}
	case COMMAND_SET_CHAR_OBJ_WAIT_ON_FOOT:
	{
		CollectParameters(&m_nIp, 1);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		assert(pPed);
		pPed->bScriptObjectiveCompleted = false;
		pPed->SetObjective(OBJECTIVE_IDLE);
		return 0;
	}
	case COMMAND_SET_CHAR_OBJ_FLEE_ON_FOOT_TILL_SAFE:
	{
		CollectParameters(&m_nIp, 1);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		assert(pPed);
		pPed->bScriptObjectiveCompleted = false;
		pPed->SetObjective(OBJECTIVE_FLEE_CHAR_ON_FOOT_TILL_SAFE);
		return 0;
	}
	case COMMAND_SET_CHAR_OBJ_GUARD_SPOT:
	{
		CollectParameters(&m_nIp, 4);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		assert(pPed);
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
		assert(pPed);
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
		float radius = max(pos.x - infX, pos.y - infY);
		pPed->bScriptObjectiveCompleted = false;
		pPed->SetObjective(OBJECTIVE_GUARD_SPOT, pos, radius);
		return 0;
	}
	case COMMAND_SET_CHAR_OBJ_WAIT_IN_CAR:
	{
		CollectParameters(&m_nIp, 1);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		assert(pPed);
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
		assert(pPed);
		pPed->m_nSelectedWepSlot = pPed->GiveWeapon((eWeaponType)ScriptParams[1], ScriptParams[2]);
		return 0;
	}
	case COMMAND_GIVE_WEAPON_TO_CHAR:
	{
		CollectParameters(&m_nIp, 3);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		assert(pPed);
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
			if (CReplay::IsPlayingBack() || CTheScripts::DelayMakingPlayerUnsafeThisTime){
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
				CAnimManager::BlendAnimation((RpClump*)pPlayer->m_pPed->m_rwObject, pPlayer->m_pPed->m_animGroup, ANIM_IDLE_STANCE, 1000.0f);
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
		CPed* pPed = CWorld::Players[ScriptParams[0]].m_pPed;
		for (int i = 0; i < WEAPONTYPE_TOTAL_INVENTORY_WEAPONS; i++){
			if (pPed->m_weapons[i].m_eWeaponType == ScriptParams[1])
				pPed->m_currentWeapon = i;
		}
		return 0;
	}
	case COMMAND_SET_CURRENT_CHAR_WEAPON:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		for (int i = 0; i < WEAPONTYPE_TOTAL_INVENTORY_WEAPONS; i++) {
			if (pPed->m_weapons[i].m_eWeaponType == ScriptParams[1])
				pPed->m_currentWeapon = i;
		}
		return 0;
	}
	/* Not implemented */
	//case COMMAND_SET_CURRENT_CAR_WEAPON:
	case COMMAND_GET_OBJECT_COORDINATES:
	{
		CollectParameters(&m_nIp, 1);
		CObject* pObject = CPools::GetObjectPool()->GetAt(ScriptParams[0]);
		assert(pObject);
		*(CVector*)&ScriptParams[0] = pObject->GetPosition();
		StoreParameters(&m_nIp, 3);
		return 0;
	}
	case COMMAND_SET_OBJECT_COORDINATES:
	{
		CollectParameters(&m_nIp, 4);
		CObject* pObject = CPools::GetObjectPool()->GetAt(ScriptParams[0]);
		assert(pObject);
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
		assert(pPed);
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
		assert(pPed);
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
		assert(pPed);
		ScriptParams[0] = pPed->m_pWanted->m_nWantedLevel;
		StoreParameters(&m_nIp, 1);
		return 0;
	}
	case COMMAND_IS_CAR_STOPPED:
	{
		CollectParameters(&m_nIp, 1);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		assert(pVehicle);
		UpdateCompareFlag(CTheScripts::IsVehicleStopped(pVehicle));
		return 0;
	}
	case COMMAND_MARK_CHAR_AS_NO_LONGER_NEEDED:
	{
		CollectParameters(&m_nIp, 1);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		CTheScripts::CleanUpThisPed(pPed);
		if (m_bIsMissionScript)
			CTheScripts::MissionCleanup.RemoveEntityFromList(ScriptParams[0], CLEANUP_CHAR);
		return 0;
	}
	case COMMAND_MARK_CAR_AS_NO_LONGER_NEEDED:
	{
		CollectParameters(&m_nIp, 1);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		CTheScripts::CleanUpThisVehicle(pVehicle);
		if (m_bIsMissionScript)
			CTheScripts::MissionCleanup.RemoveEntityFromList(ScriptParams[0], CLEANUP_CAR);
		return 0;
	}
	case COMMAND_MARK_OBJECT_AS_NO_LONGER_NEEDED:
	{
		CollectParameters(&m_nIp, 1);
		CObject* pObject = CPools::GetObjectPool()->GetAt(ScriptParams[0]);
		CTheScripts::CleanUpThisObject(pObject);
		if (m_bIsMissionScript)
			CTheScripts::MissionCleanup.RemoveEntityFromList(ScriptParams[0], CLEANUP_OBJECT);
		return 0;
	}
	case COMMAND_DONT_REMOVE_CHAR:
	{
		CollectParameters(&m_nIp, 1);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		assert(pPed);
		CTheScripts::MissionCleanup.RemoveEntityFromList(ScriptParams[0], CLEANUP_CHAR);
		return 0;
	}
	case COMMAND_DONT_REMOVE_CAR:
	{
		CollectParameters(&m_nIp, 1);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		assert(pVehicle);
		CTheScripts::MissionCleanup.RemoveEntityFromList(ScriptParams[0], CLEANUP_CAR);
		return 0;
	}
	case COMMAND_DONT_REMOVE_OBJECT:
	{
		CollectParameters(&m_nIp, 1);
		CObject* pObject = CPools::GetObjectPool()->GetAt(ScriptParams[0]);
		assert(pObject);
		CTheScripts::MissionCleanup.RemoveEntityFromList(ScriptParams[0], CLEANUP_OBJECT);
		return 0;
	}
	case COMMAND_CREATE_CHAR_AS_PASSENGER:
	{
		CollectParameters(&m_nIp, 4);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		assert(pVehicle);
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
			pPed = new CCivilianPed(ScriptParams[1], ScriptParams[2]);
		pPed->CharCreatedBy = MISSION_CHAR;
		pPed->bRespondsToThreats = false;
		pPed->bAllowMedicsToReviveMe = false;
		pPed->GetPosition() = pVehicle->GetPosition();
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
		pVehicle->m_status = STATUS_PHYSICS;
		pPed->bUsesCollision = false;
		AnimationId anim = pVehicle->bLowVehicle ? ANIM_CAR_LSIT : ANIM_CAR_SIT;
		pPed->m_pVehicleAnim = CAnimManager::BlendAnimation(pPed->GetClump(), ASSOCGRP_STD, anim, 100.0f);
		pPed->StopNonPartialAnims();
		pPed->m_nZoneLevel = CTheZones::GetLevelFromPosition(pPed->GetPosition());
		CWorld::Add(pPed);
		ScriptParams[0] = CPools::GetPedPool()->GetIndex(pPed);
		StoreParameters(&m_nIp, 1);
		if (m_bIsMissionScript)
			CTheScripts::MissionCleanup.AddEntityToList(ScriptParams[0], CLEANUP_CHAR);
		return 0;
	}
	case COMMAND_SET_CHAR_OBJ_KILL_CHAR_ON_FOOT:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		assert(pPed);
		CPed* pTarget = CPools::GetPedPool()->GetAt(ScriptParams[1]);
		pPed->bScriptObjectiveCompleted = false;
		pPed->SetObjective(OBJECTIVE_KILL_CHAR_ON_FOOT, pTarget);
		return 0;
	}
	case COMMAND_SET_CHAR_OBJ_KILL_PLAYER_ON_FOOT:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		assert(pPed);
		CPed* pTarget = CWorld::Players[ScriptParams[1]].m_pPed;
		pPed->bScriptObjectiveCompleted = false;
		pPed->SetObjective(OBJECTIVE_KILL_CHAR_ON_FOOT, pTarget);
		return 0;
	}
	case COMMAND_SET_CHAR_OBJ_KILL_CHAR_ANY_MEANS:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		assert(pPed);
		CPed* pTarget = CPools::GetPedPool()->GetAt(ScriptParams[1]);
		pPed->bScriptObjectiveCompleted = false;
		pPed->SetObjective(OBJECTIVE_KILL_CHAR_ANY_MEANS, pTarget);
		return 0;
	}
	case COMMAND_SET_CHAR_OBJ_KILL_PLAYER_ANY_MEANS:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		assert(pPed);
		CPed* pTarget = CWorld::Players[ScriptParams[1]].m_pPed;
		pPed->bScriptObjectiveCompleted = false;
		pPed->SetObjective(OBJECTIVE_KILL_CHAR_ANY_MEANS, pTarget);
		return 0;
	}
	case COMMAND_SET_CHAR_OBJ_FLEE_CHAR_ON_FOOT_TILL_SAFE:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		assert(pPed);
		CPed* pTarget = CPools::GetPedPool()->GetAt(ScriptParams[1]);
		pPed->bScriptObjectiveCompleted = false;
		pPed->SetObjective(OBJECTIVE_FLEE_CHAR_ON_FOOT_TILL_SAFE, pTarget);
		return 0;
	}
	case COMMAND_SET_CHAR_OBJ_FLEE_PLAYER_ON_FOOT_TILL_SAFE:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		assert(pPed);
		CPed* pTarget = CWorld::Players[ScriptParams[1]].m_pPed;
		pPed->bScriptObjectiveCompleted = false;
		pPed->SetObjective(OBJECTIVE_FLEE_CHAR_ON_FOOT_TILL_SAFE, pTarget);
		return 0;
	}
	case COMMAND_SET_CHAR_OBJ_FLEE_CHAR_ON_FOOT_ALWAYS:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		assert(pPed);
		CPed* pTarget = CPools::GetPedPool()->GetAt(ScriptParams[1]);
		pPed->bScriptObjectiveCompleted = false;
		pPed->SetObjective(OBJECTIVE_FLEE_CHAR_ON_FOOT_ALWAYS, pTarget);
		return 0;
	}
	case COMMAND_SET_CHAR_OBJ_FLEE_PLAYER_ON_FOOT_ALWAYS:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		assert(pPed);
		CPed* pTarget = CWorld::Players[ScriptParams[1]].m_pPed;
		pPed->bScriptObjectiveCompleted = false;
		pPed->SetObjective(OBJECTIVE_FLEE_CHAR_ON_FOOT_ALWAYS, pTarget);
		return 0;
	}
	case COMMAND_SET_CHAR_OBJ_GOTO_CHAR_ON_FOOT:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		assert(pPed);
		CPed* pTarget = CPools::GetPedPool()->GetAt(ScriptParams[1]);
		pPed->bScriptObjectiveCompleted = false;
		pPed->SetObjective(OBJECTIVE_GOTO_CHAR_ON_FOOT, pTarget);
		return 0;
	}
	case COMMAND_SET_CHAR_OBJ_GOTO_PLAYER_ON_FOOT:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		assert(pPed);
		CPed* pTarget = CWorld::Players[ScriptParams[1]].m_pPed;
		pPed->bScriptObjectiveCompleted = false;
		pPed->SetObjective(OBJECTIVE_GOTO_CHAR_ON_FOOT, pTarget);
		return 0;
	}
	case COMMAND_SET_CHAR_OBJ_LEAVE_CAR:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		assert(pPed);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[1]);
		pPed->bScriptObjectiveCompleted = false;
		pPed->SetObjective(OBJECTIVE_LEAVE_VEHICLE, pVehicle);
		return 0;
	}
	case COMMAND_SET_CHAR_OBJ_ENTER_CAR_AS_PASSENGER:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		assert(pPed);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[1]);
		pPed->bScriptObjectiveCompleted = false;
		pPed->SetObjective(OBJECTIVE_ENTER_CAR_AS_PASSENGER, pVehicle);
		return 0;
	}
	case COMMAND_SET_CHAR_OBJ_ENTER_CAR_AS_DRIVER:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		assert(pPed);
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
		assert(pPed);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[1]);
		pPed->bScriptObjectiveCompleted = false;
		pPed->SetObjective(OBJECTIVE_DESTROY_CAR, pVehicle);
		return 0;
	}
	case COMMAND_SET_CHAR_OBJ_GOTO_AREA_ON_FOOT:
	{
		CollectParameters(&m_nIp, 5);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		assert(pPed);
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
		float radius = max(pos.x - infX, pos.y - infY);
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
		assert(pPed);
		CPed* pTarget = CPools::GetPedPool()->GetAt(ScriptParams[1]);
		pPed->SetObjective(OBJECTIVE_SET_LEADER, pTarget);
		return 0;
	}
	case COMMAND_SET_PLAYER_AS_LEADER:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		assert(pPed);
		CPed* pTarget = CWorld::Players[ScriptParams[1]].m_pPed;
		pPed->SetObjective(OBJECTIVE_SET_LEADER, pTarget);
		return 0;
	}
	case COMMAND_LEAVE_GROUP:
	{
		CollectParameters(&m_nIp, 1);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		assert(pPed);
		pPed->ClearLeader();
		return 0;
	}
	case COMMAND_SET_CHAR_OBJ_FOLLOW_ROUTE:
	{
		CollectParameters(&m_nIp, 3);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		assert(pPed);
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
		assert(pVehicle);
		ScriptParams[0] = pVehicle->m_nNumPassengers;
		StoreParameters(&m_nIp, 1);
		return 0;
	}
	case COMMAND_GET_MAXIMUM_NUMBER_OF_PASSENGERS:
	{
		CollectParameters(&m_nIp, 1);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		assert(pVehicle);
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
		CollectParameters(&m_nIp, 1);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		assert(pVehicle);
		pVehicle->bIsHeavy = (ScriptParams[1] != 0);
		return 0;
	}
	case COMMAND_CLEAR_CHAR_THREAT_SEARCH:
	{
		CollectParameters(&m_nIp, 1);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		assert(pPed);
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
		assert(pVehicle);
		UpdateCompareFlag(pVehicle->m_nCollisionRecords == 0);
		return 0;
	}
	default:
		assert(0);
	}
	return -1;
}
#endif

#if 0
WRAPPER int8 CRunningScript::ProcessCommandsFrom500To599(int32 command) { EAXJMP(0x4429C0); }
#else
int8 CRunningScript::ProcessCommandsFrom500To599(int32 command)
{
	switch (command) {
	case COMMAND_IS_CAR_UPSIDEDOWN:
	{
		CollectParameters(&m_nIp, 1);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		assert(pVehicle);
		UpdateCompareFlag(pVehicle->GetUp().z <= -0.97f);
		return 0;
	}
	case COMMAND_GET_PLAYER_CHAR:
	{
		CollectParameters(&m_nIp, 1);
		CPlayerPed* pPed = CWorld::Players[ScriptParams[0]].m_pPed;
		assert(pPed);
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
		assert(pPed);
		if (ScriptParams[0]) {
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
		assert(pVehicle);
		pVehicle->m_nDoorLock = (eCarLock)ScriptParams[1];
		return 0;
	}
	case COMMAND_EXPLODE_CAR:
	{
		CollectParameters(&m_nIp, 1);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		assert(pVehicle);
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
		assert(pVehicle);
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
		assert(pPed);
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
		assert(pVehicle);
		assert(pVehicle->m_vehType == VEHICLE_TYPE_CAR);
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
		ScriptParams[0] = CGarages::AddOne(infX, infY, infZ, supX, supY, supZ, ScriptParams[6], 0);
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
		ScriptParams[0] = CGarages::AddOne(infX, infY, infZ, supX, supY, supZ, ScriptParams[6], ScriptParams[7]);
		StoreParameters(&m_nIp, 1);
		return 0;
	}
	case COMMAND_SET_TARGET_CAR_FOR_MISSION_GARAGE:
	{
		CollectParameters(&m_nIp, 2);
		CVehicle* pTarget;
		if (ScriptParams[1] >= 0) {
			pTarget = CPools::GetVehiclePool()->GetAt(ScriptParams[1]);
			assert(pTarget);
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
		CGarages::BombsAreFree = (ScriptParams[0] != 0);
		return 0;
		//case COMMAND_SET_POWERPOINT:
	case COMMAND_SET_ALL_TAXI_LIGHTS:
		CollectParameters(&m_nIp, 1);
		CAutomobile::SetAllTaxiLights(ScriptParams[0] != 0);
		return 0;
	case COMMAND_IS_CAR_ARMED_WITH_ANY_BOMB:
	{
		CollectParameters(&m_nIp, 1);
		CAutomobile* pCar = (CAutomobile*)CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		assert(pCar);
		assert(pCar->m_vehType == VEHICLE_TYPE_CAR);
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
		assert(pPed);
		pPed->m_fHealth = ScriptParams[1];
		return 0;
	}
	case COMMAND_SET_CHAR_HEALTH:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		assert(pPed);
		if (ScriptParams[1]) {
			pPed->m_fHealth = ScriptParams[1];
		}
		else if (pPed->bInVehicle) {
			pPed->SetDead();
			if (!pPed->IsPlayer())
				pPed->FlagToDestroyWhenNextProcessed();
		}
		else {
			pPed->SetDie(ANIM_KO_SHOT_FRONT1, 4.0f, 0.0f);
		}
		return 0;
	}
	case COMMAND_SET_CAR_HEALTH:
	{
		CollectParameters(&m_nIp, 2);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		assert(pVehicle);
		pVehicle->m_fHealth = ScriptParams[1];
		return 0;
	}
	case COMMAND_GET_PLAYER_HEALTH:
	{
		CollectParameters(&m_nIp, 1);
		CPed* pPed = CWorld::Players[ScriptParams[0]].m_pPed;
		assert(pPed);
		ScriptParams[0] = pPed->m_fHealth; // correct cast float to int
		StoreParameters(&m_nIp, 1);
		return 0;
	}
	case COMMAND_GET_CHAR_HEALTH:
	{
		CollectParameters(&m_nIp, 1);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		assert(pPed);
		ScriptParams[0] = pPed->m_fHealth; // correct cast float to int
		StoreParameters(&m_nIp, 1);
		return 0;
	}
	case COMMAND_GET_CAR_HEALTH:
	{
		CollectParameters(&m_nIp, 1);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		assert(pVehicle);
		ScriptParams[0] = pVehicle->m_fHealth; // correct cast float to int
		StoreParameters(&m_nIp, 1);
		return 0;
	}
	case COMMAND_IS_CAR_ARMED_WITH_BOMB:
	{
		CollectParameters(&m_nIp, 2);
		CAutomobile* pCar = (CAutomobile*)CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		assert(pCar);
		assert(pCar->m_vehType == VEHICLE_TYPE_CAR);
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
		assert(pSourcePed);
		CPed* pTargetPed = CPools::GetPedPool()->GetAt(ScriptParams[1]);
		assert(pTargetPed);
		pSourcePed->SetLookFlag(pTargetPed, true);
		pSourcePed->SetLookTimer(60000);
		return 0;
	}
	case COMMAND_CHAR_LOOK_AT_PLAYER_ALWAYS:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pSourcePed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		assert(pSourcePed);
		CPed* pTargetPed = CWorld::Players[ScriptParams[1]].m_pPed;
		assert(pTargetPed);
		pSourcePed->SetLookFlag(pTargetPed, true);
		pSourcePed->SetLookTimer(60000);
		return 0;
	}
	case COMMAND_PLAYER_LOOK_AT_CHAR_ALWAYS:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pSourcePed = CWorld::Players[ScriptParams[0]].m_pPed;
		assert(pSourcePed);
		CPed* pTargetPed = CPools::GetPedPool()->GetAt(ScriptParams[1]);
		assert(pTargetPed);
		pSourcePed->SetLookFlag(pTargetPed, true);
		pSourcePed->SetLookTimer(60000);
		return 0;
	}
	case COMMAND_STOP_CHAR_LOOKING:
	{
		CollectParameters(&m_nIp, 1);
		CPed* pSourcePed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		assert(pSourcePed);
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
		assert(pSourcePed);
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
		assert(pPed);
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
		float radius = max(pos.x - infX, pos.y - infY);
		pPed->bScriptObjectiveCompleted = false;
		pPed->SetObjective(OBJECTIVE_RUN_TO_AREA, pos, radius);
		return 0;
	}
	case COMMAND_SET_CHAR_OBJ_RUN_TO_COORD:
	{
		CollectParameters(&m_nIp, 3);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		assert(pPed);
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
		assert(pPed);
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
		assert(pPed);
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
		strncpy(name, (char*)&CTheScripts::ScriptSpace[m_nIp], 8);
		for (int i = 0; i < 8; i++)
			name[i] = tolower(name[i]);
		CStreaming::RequestSpecialChar(ScriptParams[0] - 1, name, STREAMFLAGS_DEPENDENCY | STREAMFLAGS_SCRIPTOWNED);
		m_nIp += 8;
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
		assert(pVehicle);
		pVehicle->bHasBlip = (ScriptParams[1] != 0);
		return 0;
	}
	case COMMAND_FLASH_CHAR:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		assert(pPed);
		pPed->bHasBlip = (ScriptParams[1] != 0);
		return 0;
	}
	case COMMAND_FLASH_OBJECT:
	{
		CollectParameters(&m_nIp, 2);
		CObject* pObject = CPools::GetObjectPool()->GetAt(ScriptParams[0]);
		assert(pObject);
		pObject->bHasBlip = (ScriptParams[1] != 0);
		return 0;
	}
	case COMMAND_IS_PLAYER_IN_REMOTE_MODE:
		CollectParameters(&m_nIp, 2);
		UpdateCompareFlag(CWorld::Players[ScriptParams[0]].IsPlayerInRemoteMode());
		return 0;
	case COMMAND_ARM_CAR_WITH_BOMB:
	{
		CollectParameters(&m_nIp, 2);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		assert(pVehicle);
		assert(pVehicle->m_vehType == VEHICLE_TYPE_CAR);
		((CAutomobile*)pVehicle)->m_bombType = ScriptParams[1];
		((CAutomobile*)pVehicle)->m_pBombRigger = FindPlayerPed();
		return 0;
	}
	case COMMAND_SET_CHAR_PERSONALITY:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		assert(pPed);
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
		assert(pPed);
		pPed->m_animGroup = (AssocGroupId)ScriptParams[1];
		return 0;
	}
	case COMMAND_SET_ANIM_GROUP_FOR_PLAYER:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CWorld::Players[ScriptParams[0]].m_pPed;
		assert(pPed);
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
		CCoronas::RegisterCorona((uint32)this + m_nIp, ScriptParams[6], ScriptParams[7], ScriptParams[8],
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
		if (CWorld::Players[CWorld::PlayerInFocus].m_WBState != WBSTATE_PLAYING) //TODO: enum
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
	//case COMMAND_SET_COLL_OBJ_NO_OBJ:
	default:
		assert(0);
	}
	return -1;
}
#endif

#if 0
WRAPPER int8 CRunningScript::ProcessCommandsFrom600To699(int32 command) { EAXJMP(0x444B20); }
#else
int8 CRunningScript::ProcessCommandsFrom600To699(int32 command)
{
	switch (command){
	/* Collective commands are not implemented until LCS.
	case COMMAND_SET_COLL_OBJ_WAIT_ON_FOOT:
	case COMMAND_SET_COLL_OBJ_FLEE_ON_FOOT_TILL_SAFE:
	case COMMAND_SET_COLL_OBJ_GUARD_SPOT:
	case COMMAND_SET_COLL_OBJ_GUARD_AREA:
	case COMMAND_SET_COLL_OBJ_WAIT_IN_CAR:
	case COMMAND_SET_COLL_OBJ_KILL_CHAR_ON_FOOT:
	case COMMAND_SET_COLL_OBJ_KILL_PLAYER_ON_FOOT:
	case COMMAND_SET_COLL_OBJ_KILL_CHAR_ANY_MEANS:
	case COMMAND_SET_COLL_OBJ_KILL_PLAYER_ANY_MEANS:
	case COMMAND_SET_COLL_OBJ_FLEE_CHAR_ON_FOOT_TILL_SAFE:
	case COMMAND_SET_COLL_OBJ_FLEE_PLAYER_ON_FOOT_TILL_SAFE:
	case COMMAND_SET_COLL_OBJ_FLEE_CHAR_ON_FOOT_ALWAYS:
	case COMMAND_SET_COLL_OBJ_FLEE_PLAYER_ON_FOOT_ALWAYS:
	case COMMAND_SET_COLL_OBJ_GOTO_CHAR_ON_FOOT:
	case COMMAND_SET_COLL_OBJ_GOTO_PLAYER_ON_FOOT:
	case COMMAND_SET_COLL_OBJ_LEAVE_CAR:
	case COMMAND_SET_COLL_OBJ_ENTER_CAR_AS_PASSENGER:
	case COMMAND_SET_COLL_OBJ_ENTER_CAR_AS_DRIVER:
	case COMMAND_SET_COLL_OBJ_FOLLOW_CAR_IN_CAR:
	case COMMAND_SET_COLL_OBJ_FIRE_AT_OBJECT_FROM_VEHICLE:
	case COMMAND_SET_COLL_OBJ_DESTROY_OBJECT:
	case COMMAND_SET_COLL_OBJ_DESTROY_CAR:
	case COMMAND_SET_COLL_OBJ_GOTO_AREA_ON_FOOT:
	case COMMAND_SET_COLL_OBJ_GOTO_AREA_IN_CAR:
	case COMMAND_SET_COLL_OBJ_FOLLOW_CAR_ON_FOOT_WITH_OFFSET:
	case COMMAND_SET_COLL_OBJ_GUARD_ATTACK:
	case COMMAND_SET_COLL_OBJ_FOLLOW_ROUTE:
	case COMMAND_SET_COLL_OBJ_GOTO_COORD_ON_FOOT:
	case COMMAND_SET_COLL_OBJ_GOTO_COORD_IN_CAR:
	case COMMAND_SET_COLL_OBJ_RUN_TO_AREA:
	case COMMAND_SET_COLL_OBJ_RUN_TO_COORD:
	case COMMAND_ADD_PEDS_IN_AREA_TO_COLL:
	case COMMAND_ADD_PEDS_IN_VEHICLE_TO_COLL:
	case COMMAND_CLEAR_COLL:
	case COMMAND_IS_COLL_IN_CARS:
	case COMMAND_LOCATE_COLL_ANY_MEANS_2D:
	case COMMAND_LOCATE_COLL_ON_FOOT_2D:
	case COMMAND_LOCATE_COLL_IN_CAR_2D:
	case COMMAND_LOCATE_STOPPED_COLL_ANY_MEANS_2D:
	case COMMAND_LOCATE_STOPPED_COLL_ON_FOOT_2D:
	case COMMAND_LOCATE_STOPPED_COLL_IN_CAR_2D:
	case COMMAND_LOCATE_COLL_ANY_MEANS_CHAR_2D:
	case COMMAND_LOCATE_COLL_ON_FOOT_CHAR_2D:
	case COMMAND_LOCATE_COLL_IN_CAR_CHAR_2D:
	case COMMAND_LOCATE_COLL_ANY_MEANS_CAR_2D:
	case COMMAND_LOCATE_COLL_ON_FOOT_CAR_2D:
	case COMMAND_LOCATE_COLL_IN_CAR_CAR_2D:
	case COMMAND_LOCATE_COLL_ANY_MEANS_PLAYER_2D:
	case COMMAND_LOCATE_COLL_ON_FOOT_PLAYER_2D:
	case COMMAND_LOCATE_COLL_IN_CAR_PLAYER_2D:
	case COMMAND_IS_COLL_IN_AREA_2D:
	case COMMAND_IS_COLL_IN_AREA_ON_FOOT_2D:
	case COMMAND_IS_COLL_IN_AREA_IN_CAR_2D:
	case COMMAND_IS_COLL_STOPPED_IN_AREA_2D:
	case COMMAND_IS_COLL_STOPPED_IN_AREA_ON_FOOT_2D:
	case COMMAND_IS_COLL_STOPPED_IN_AREA_IN_CAR_2D:
	case COMMAND_GET_NUMBER_OF_PEDS_IN_COLL:
	*/
	case COMMAND_SET_CHAR_HEED_THREATS:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		assert(pPed);
		pPed->bRespondsToThreats = (ScriptParams[1] != 0);
		return 0;
	}
	case COMMAND_SET_PLAYER_HEED_THREATS:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CWorld::Players[ScriptParams[0]].m_pPed;
		assert(pPed);
		pPed->bRespondsToThreats = (ScriptParams[1] != 0);
		return 0;
	}
	case COMMAND_GET_CONTROLLER_MODE:
		ScriptParams[0] = 0;
		StoreParameters(&m_nIp, 1);
		return 0;
	case COMMAND_SET_CAN_RESPRAY_CAR:
	{
		CollectParameters(&m_nIp, 2);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		assert(pVehicle);
		assert(pVehicle->m_vehType == VEHICLE_TYPE_CAR);
		((CAutomobile*)pVehicle)->bFixedColour = (ScriptParams[1] == 0);
		return 0;
	}
	case COMMAND_IS_TAXI:
	{
		CollectParameters(&m_nIp, 1);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		assert(pVehicle);
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
;		pObj->ObjectCreatedBy = MISSION_OBJECT;
		CVector pos = *(CVector*)&ScriptParams[1];
		if (pos.z <= MAP_Z_LOW_LIMIT)
			pos.z = CWorld::FindGroundZForCoord(pos.x, pos.y);
		pObj->GetPosition() = pos;
		pObj->SetOrientation(0.0f, 0.0f, 0.0f);
		pObj->GetMatrix().UpdateRW();
		pObj->UpdateRwFrame();
		CTheScripts::ClearSpaceForMissionEntity(pos, pObj);
		CWorld::Add(pObj);
		ScriptParams[0] = CPools::GetObjectPool()->GetIndex(pObj);
		StoreParameters(&m_nIp, 1);
		if (m_bIsMissionScript)
			CTheScripts::MissionCleanup.AddEntityToList(ScriptParams[0], CLEANUP_OBJECT);
		return 0;
	}
	case COMMAND_IS_BOAT:
	{
		CollectParameters(&m_nIp, 1);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		assert(pVehicle);
		UpdateCompareFlag(pVehicle->m_vehType == VEHICLE_TYPE_BOAT);
		return 0;
	}
	case COMMAND_SET_CHAR_OBJ_GOTO_AREA_ANY_MEANS:
	{
		CollectParameters(&m_nIp, 5);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		assert(pPed);
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
		float radius = max(pos.x - infX, pos.y - infY);
		pPed->bScriptObjectiveCompleted = false;
		pPed->SetObjective(OBJECTIVE_GOTO_AREA_ANY_MEANS, pos, radius);
		return 0;
	}
	//case COMMAND_SET_COLL_OBJ_GOTO_AREA_ANY_MEANS:
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
		assert(pVehicle);
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
		assert(pPed);
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
		assert(pObject);
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
		assert(pPed);
		pPed->bOnlyDamagedByPlayer = (ScriptParams[1] != 0);
		return 0;
	}
	case COMMAND_SET_CAR_ONLY_DAMAGED_BY_PLAYER:
	{
		CollectParameters(&m_nIp, 2);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		assert(pVehicle);
		pVehicle->bOnlyDamagedByPlayer = (ScriptParams[1] != 0);
		return 0;
	}
	case COMMAND_SET_CHAR_PROOFS:
	{
		CollectParameters(&m_nIp, 6);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		assert(pPed);
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
		assert(pVehicle);
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
		assert(0);
	}
	return -1;
}
#endif

#if 0
WRAPPER int8 CRunningScript::ProcessCommandsFrom700To799(int32 command) { EAXJMP(0x4458A0); }
#else
int8 CRunningScript::ProcessCommandsFrom700To799(int32 command)
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
		assert(pVehicle);
		UpdateCompareFlag(pVehicle->bIsInWater);
		return 0;
	}
	case COMMAND_GET_CLOSEST_CHAR_NODE:
	{
		CollectParameters(&m_nIp, 3);
		CVector pos = *(CVector*)&ScriptParams[0];
		if (pos.z <= MAP_Z_LOW_LIMIT)
			pos.z = CWorld::FindGroundZForCoord(pos.x, pos.y);
		CPathNode* pNode = &ThePaths.m_pathNodes[ThePaths.FindNodeClosestToCoors(pos, 1, 999999.9f)];
		*(CVector*)&ScriptParams[0] = pNode->pos;
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
		*(CVector*)&ScriptParams[0] = pNode->pos;
		StoreParameters(&m_nIp, 3);
		return 0;
	}
	case COMMAND_CAR_GOTO_COORDINATES_ACCURATE:
	{
		CollectParameters(&m_nIp, 4);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		assert(pVehicle);
		CVector pos = *(CVector*)&ScriptParams[1];
		if (pos.z <= MAP_Z_LOW_LIMIT)
			pos.z = CWorld::FindGroundZForCoord(pos.x, pos.y);
		pos.z += pVehicle->GetDistanceFromCentreOfMassToBaseOfModel();
		if (CCarCtrl::JoinCarWithRoadSystemGotoCoors(pVehicle, pos, false))
			pVehicle->AutoPilot.m_nCarMission = MISSION_GOTO_COORDS_STRAIGHT_ACCURATE;
		else
			pVehicle->AutoPilot.m_nCarMission = MISSION_GOTOCOORDS_ACCURATE;
		pVehicle->m_status = STATUS_PHYSICS;
		pVehicle->bEngineOn = true;
		pVehicle->AutoPilot.m_nCruiseSpeed = min(6, pVehicle->AutoPilot.m_nCruiseSpeed);
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
		return 0;
	case COMMAND_CLEAR_NUMBER_OF_POWER_PILLS_CARRIED:
		CPacManPickups::ResetPowerPillsCarriedByPlayer();
		return 0;
	case COMMAND_IS_CAR_ON_SCREEN:
	{
		CollectParameters(&m_nIp, 1);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		assert(pVehicle);
		UpdateCompareFlag(TheCamera.IsSphereVisible(pVehicle->GetBoundCentre(), pVehicle->GetBoundRadius()));
		return 0;
	}
	case COMMAND_IS_CHAR_ON_SCREEN:
	{
		CollectParameters(&m_nIp, 1);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		assert(pPed);
		UpdateCompareFlag(TheCamera.IsSphereVisible(pPed->GetBoundCentre(), pPed->GetBoundRadius()));
		return 0;
	}
	case COMMAND_IS_OBJECT_ON_SCREEN:
	{
		CollectParameters(&m_nIp, 1);
		CObject* pObject = CPools::GetObjectPool()->GetAt(ScriptParams[0]);
		assert(pObject);
		UpdateCompareFlag(TheCamera.IsSphereVisible(pObject->GetBoundCentre(), pObject->GetBoundRadius()));
		return 0;
	}
	case COMMAND_GOSUB_FILE:
	{
		CollectParameters(&m_nIp, 2);
		assert(m_nStackPointer < MAX_STACK_DEPTH);
		m_anStack[m_nStackPointer++] = m_nIp;
		m_nIp = ScriptParams[0];
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
		assert(pVehicle);
		pVehicle->bComedyControls = (ScriptParams[1] != 0);
		return 0;
	}
	case COMMAND_BOAT_GOTO_COORDS:
	{
		CollectParameters(&m_nIp, 4);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		assert(pVehicle);
		assert(pVehicle->m_vehType == VEHICLE_TYPE_BOAT);
		CBoat* pBoat = (CBoat*)pVehicle;
		CVector pos = *(CVector*)&ScriptParams[1];
		if (pos.z <= MAP_Z_LOW_LIMIT)
			CWaterLevel::GetWaterLevel(pos.x, pos.y, pos.z, &pos.z, false);
		pBoat->AutoPilot.m_nCarMission = MISSION_GOTOCOORDS_ASTHECROWSWIMS;
		pBoat->AutoPilot.m_vecDestinationCoors = pos;
		pBoat->m_status = STATUS_PHYSICS;
		pBoat->AutoPilot.m_nCruiseSpeed = max(6, pBoat->AutoPilot.m_nCruiseSpeed);
		pBoat->AutoPilot.m_nAntiReverseTimer = CTimer::GetTimeInMilliseconds();
		return 0;
	}
	case COMMAND_BOAT_STOP:
	{
		CollectParameters(&m_nIp, 1);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		assert(pVehicle);
		assert(pVehicle->m_vehType == VEHICLE_TYPE_BOAT);
		CBoat* pBoat = (CBoat*)pVehicle;
		pBoat->AutoPilot.m_nCarMission = MISSION_NONE;
		pBoat->m_status = STATUS_PHYSICS;
		pBoat->bEngineOn = false;
		pBoat->AutoPilot.m_nCruiseSpeed = 0;
		return 0;
	}
	case COMMAND_IS_PLAYER_SHOOTING_IN_AREA:
	{
		CollectParameters(&m_nIp, 6);
		CPed* pPed = CWorld::Players[ScriptParams[0]].m_pPed;
		assert(pPed);
		float x1 = *(float*)&ScriptParams[1];
		float y1 = *(float*)&ScriptParams[2];
		float x2 = *(float*)&ScriptParams[3];
		float y2 = *(float*)&ScriptParams[4];
		UpdateCompareFlag(pPed->bIsShooting && pPed->IsWithinArea(x1, y1, x2, y2));
		if (!ScriptParams[5])
			return 0;
		CTheScripts::HighlightImportantArea((uint32)this + m_nIp, x1, y1, x2, y2, MAP_Z_LOW_LIMIT);
		if (CTheScripts::DbgFlag)
			CTheScripts::DrawDebugSquare(x1, y1, x2, y2);
		return 0;
	}
	case COMMAND_IS_CHAR_SHOOTING_IN_AREA:
	{
		CollectParameters(&m_nIp, 6);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		assert(pPed);
		float x1 = *(float*)&ScriptParams[1];
		float y1 = *(float*)&ScriptParams[2];
		float x2 = *(float*)&ScriptParams[3];
		float y2 = *(float*)&ScriptParams[4];
		UpdateCompareFlag(pPed->bIsShooting && pPed->IsWithinArea(x1, y1, x2, y2));
		if (!ScriptParams[5])
			return 0;
		CTheScripts::HighlightImportantArea((uint32)this + m_nIp, x1, y1, x2, y2, MAP_Z_LOW_LIMIT);
		if (CTheScripts::DbgFlag)
			CTheScripts::DrawDebugSquare(x1, y1, x2, y2);
		return 0;
	}
	case COMMAND_IS_CURRENT_PLAYER_WEAPON:
	{
		CollectParameters(&m_nIp, 2); 
		CPed* pPed = CWorld::Players[ScriptParams[0]].m_pPed;
		assert(pPed);
		UpdateCompareFlag(ScriptParams[1] == pPed->m_weapons[pPed->m_currentWeapon].m_eWeaponType);
		return 0;
	}
	case COMMAND_IS_CURRENT_CHAR_WEAPON:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		assert(pPed);
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
		assert(pVehicle);
		assert(pVehicle->m_vehType == VEHICLE_TYPE_BOAT);
		CBoat* pBoat = (CBoat*)pVehicle;
		pBoat->AutoPilot.m_nCruiseSpeed = ScriptParams[1];
	}
	case COMMAND_GET_RANDOM_CHAR_IN_AREA:
	{
		CollectParameters(&m_nIp, 4);
		int ped_handle = -1;
		CVector pos = FindPlayerCoors();
		float x1 = *(float*)&ScriptParams[1];
		float y1 = *(float*)&ScriptParams[2];
		float x2 = *(float*)&ScriptParams[3];
		float y2 = *(float*)&ScriptParams[4];
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
				CTheScripts::MissionCleanup.AddEntityToList(ped_handle, CLEANUP_CHAR);
		}
		ScriptParams[0] = ped_handle;
		StoreParameters(&m_nIp, 1);
		return 0;
	}
	case COMMAND_GET_RANDOM_CHAR_IN_ZONE:
	{
		char zone[8];
		strncpy(zone, (const char*)&CTheScripts::ScriptSpace[m_nIp], 8);
		int nZone = CTheZones::FindZoneByLabelAndReturnIndex(zone);
		if (nZone != -1)
			m_nIp += 8;
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
			if (!CTheZones::PointLiesWithinZone(pPed->GetPosition(), pZone))
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
				CTheScripts::MissionCleanup.AddEntityToList(ped_handle, CLEANUP_CHAR);
		}
		ScriptParams[0] = ped_handle;
		StoreParameters(&m_nIp, 1);
		return 0;
	}
	case COMMAND_IS_PLAYER_IN_TAXI:
	{
		CollectParameters(&m_nIp, 1);
		CPed* pPed = CWorld::Players[ScriptParams[0]].m_pPed;
		assert(pPed);
		UpdateCompareFlag(pPed->bInVehicle && pPed->m_pMyVehicle->IsTaxi());
		return 0;
	}
	case COMMAND_IS_PLAYER_SHOOTING:
	{
		CollectParameters(&m_nIp, 1);
		CPed* pPed = CWorld::Players[ScriptParams[0]].m_pPed;
		assert(pPed);
		UpdateCompareFlag(pPed->bIsShooting);
		return 0;
	}
	case COMMAND_IS_CHAR_SHOOTING:
	{
		CollectParameters(&m_nIp, 1);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		assert(pPed);
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
		assert(pPed);
		pPed->m_wepAccuracy = ScriptParams[1];
		return 0;
	}
	case COMMAND_GET_CAR_SPEED:
	{
		CollectParameters(&m_nIp, 1);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		assert(pVehicle);
		*(float*)&ScriptParams[0] = pVehicle->GetSpeed().Magnitude() * GAME_SPEED_TO_METERS_PER_SECOND;
		StoreParameters(&m_nIp, 1);
		return 0;
	}
	case COMMAND_LOAD_CUTSCENE:
	{
		char name[8];
		strncpy(name, (const char*)&CTheScripts::ScriptSpace[m_nIp], 8);
		m_nIp += 8;
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
		char name[8];
		CObject* pObject = CPools::GetObjectPool()->GetAt(ScriptParams[0]);
		assert(pObject);
		strncpy(name, (const char*)&CTheScripts::ScriptSpace[m_nIp], 8);
		m_nIp += 8;
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
		assert(pPed);
		UpdateCompareFlag(ScriptParams[1] == pPed->GetModelIndex());
		return 0;
	}
	case COMMAND_LOAD_SPECIAL_MODEL:
	{
		CollectParameters(&m_nIp, 1);
		char name[8];
		strncpy(name, (const char*)&CTheScripts::ScriptSpace[m_nIp], 8);
		for (int i = 0; i < 8; i++)
			name[i] = tolower(name[i]);
		CStreaming::RequestSpecialModel(ScriptParams[0], name, STREAMFLAGS_DEPENDENCY | STREAMFLAGS_SCRIPTOWNED);
		m_nIp += 8;
		return 0;
	}
	case COMMAND_CREATE_CUTSCENE_HEAD:
	{
		CollectParameters(&m_nIp, 2);
		CObject* pObject = CPools::GetObjectPool()->GetAt(ScriptParams[0]);
		assert(pObject);
		CCutsceneHead* pCutHead = CCutsceneMgr::AddCutsceneHead(pObject, ScriptParams[1]);
		ScriptParams[0] = CPools::GetObjectPool()->GetIndex(pCutHead);
		StoreParameters(&m_nIp, 1);
		return 0;
	}
	case COMMAND_SET_CUTSCENE_HEAD_ANIM:
	{
		CollectParameters(&m_nIp, 1);
		CObject* pCutHead = CPools::GetObjectPool()->GetAt(ScriptParams[0]);
		assert(pCutHead);
		char name[8];
		strncpy(name, (const char*)&CTheScripts::ScriptSpace[m_nIp], 8);
		m_nIp += 8;
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
		CGarages::ChangeGarageType(ScriptParams[0], (eGarageType)ScriptParams[1]);
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
			DEGTORAD(*(float*)&ScriptParams[8]), true, false,
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
		assert(pPed);
		CPed* pTargetPed = CPools::GetPedPool()->GetAt(ScriptParams[1]);
		pPed->bScriptObjectiveCompleted = false;
		pPed->SetObjective(OBJECTIVE_FOLLOW_PED_IN_FORMATION, pTargetPed);
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
		CStats::MaximumJumpDistance = max(CStats::MaximumJumpDistance, *(float*)&ScriptParams[0]);
		return 0;
	case COMMAND_REGISTER_JUMP_HEIGHT:
		CollectParameters(&m_nIp, 1);
		CStats::MaximumJumpHeight = max(CStats::MaximumJumpHeight, *(float*)&ScriptParams[0]);
		return 0;
	case COMMAND_REGISTER_JUMP_FLIPS:
		CollectParameters(&m_nIp, 1);
		CStats::MaximumJumpFlips = max(CStats::MaximumJumpFlips, ScriptParams[0]);
		return 0;
	case COMMAND_REGISTER_JUMP_SPINS:
		CollectParameters(&m_nIp, 1);
		CStats::MaximumJumpSpins = max(CStats::MaximumJumpSpins, ScriptParams[0]);
		return 0;
	case COMMAND_REGISTER_JUMP_STUNT:
		CollectParameters(&m_nIp, 1);
		CStats::BestStuntJump = max(CStats::BestStuntJump, ScriptParams[0]);
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
		char name[8];
		strncpy(name, (const char*)&CTheScripts::ScriptSpace[m_nIp], 8);
		m_nIp += 8;
		strncpy(CStats::LastMissionPassedName, name, 8);
		++CStats::MissionsPassed;
		CStats::CheckPointReachedSuccessfully();
		return 0;
	}
	case COMMAND_SET_CHAR_RUNNING:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		assert(pPed);
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
		assert(pVehicle);
		UpdateCompareFlag(pVehicle->m_currentColour1 == ScriptParams[1]);
		return 0;
	}
	case COMMAND_IS_SECOND_CAR_COLOUR:
	{
		CollectParameters(&m_nIp, 2);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		assert(pVehicle);
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
		assert(pPed);
		assert(pLeader);
		UpdateCompareFlag(pPed->m_leader == pLeader);
		return 0;
	}
	default:
		assert(0);
	}
	return -1;
}
#endif

#if 0
WRAPPER int8 CRunningScript::ProcessCommandsFrom800To899(int32 command) { EAXJMP(0x448240); }
#else
int8 CRunningScript::ProcessCommandsFrom800To899(int32 command)
{
	CMatrix tmp_matrix;
	switch (command) {
	case COMMAND_IS_CHAR_IN_PLAYERS_GROUP:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		CPed* pLeader = CWorld::Players[ScriptParams[1]].m_pPed;
		assert(pPed);
		assert(pLeader);
		UpdateCompareFlag(pPed->m_leader == pLeader);
		return 0;
	}
	case COMMAND_EXPLODE_CHAR_HEAD:
	{
		CollectParameters(&m_nIp, 1);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		assert(pPed);
		if (pPed->m_nPedState == PED_DRIVING) {
			pPed->SetDead();
			if (!pPed->IsPlayer())
				pPed->FlagToDestroyWhenNextProcessed();
		}
		else if (CGame::nastyGame && pPed->IsPedInControl()) {
			RwMatrix tmp_rw;
			CPedIK::GetWorldMatrix(pPed->m_pFrames[PED_HEAD]->frame, &tmp_rw);
			pPed->ApplyHeadShot(WEAPONTYPE_SNIPERRIFLE, tmp_rw.pos, true);
		}
		else {
			pPed->SetDie(ANIM_KO_SHOT_FRONT1, 4.0f, 0.0f);
		}
		return 0;
	}
	case COMMAND_EXPLODE_PLAYER_HEAD:
	{
		CollectParameters(&m_nIp, 1);
		CPed* pPed = CWorld::Players[ScriptParams[0]].m_pPed;
		assert(pPed);
		if (CGame::nastyGame) {
			RwMatrix tmp_rw;
			CPedIK::GetWorldMatrix(pPed->m_pFrames[PED_HEAD]->frame, &tmp_rw);
			pPed->ApplyHeadShot(WEAPONTYPE_SNIPERRIFLE, tmp_rw.pos, true);
		}
		else {
			pPed->SetDie(ANIM_KO_SHOT_FRONT1, 4.0f, 0.0f);
		}
		return 0;
	}
	case COMMAND_ANCHOR_BOAT:
	{
		CollectParameters(&m_nIp, 2);
		CBoat* pBoat = (CBoat*)CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		assert(pBoat && pBoat->m_vehType == VEHICLE_TYPE_BOAT);
		pBoat->m_bIsAnchored = (ScriptParams[1] == 0);
		return 0;
	}
	case COMMAND_SET_ZONE_GROUP:
	{
		char zone[8];
		CTheScripts::ReadTextLabelFromScript(&m_nIp, zone);
		m_nIp += 8;
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
		assert(pVehicle);
		ScriptParams[0] = gFireManager.StartScriptFire(pVehicle->GetPosition(), pVehicle, 0.8f, 1);
		StoreParameters(&m_nIp, 1);
		return 0;
	}
	case COMMAND_START_CHAR_FIRE:
	{
		CollectParameters(&m_nIp, 1);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		assert(pPed);
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
				CTheScripts::MissionCleanup.AddEntityToList(handle, CLEANUP_CAR);
		}
		ScriptParams[0] = handle;
		StoreParameters(&m_nIp, 1);
		return 0;
	}
	case COMMAND_GET_RANDOM_CAR_OF_TYPE_IN_ZONE:
	{
		char zone[8];
		CTheScripts::ReadTextLabelFromScript(&m_nIp, zone);
		int zone_id = CTheZones::FindZoneByLabelAndReturnIndex(zone);
		if (zone_id != -1)
			m_nIp += 8;
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
			if (!CTheZones::PointLiesWithinZone(pVehicle->GetPosition(), pZone))
				continue;
			handle = CPools::GetVehiclePool()->GetIndex(pVehicle);
			pVehicle->VehicleCreatedBy = MISSION_VEHICLE;
			++CCarCtrl::NumMissionCars;
			--CCarCtrl::NumRandomCars;
			if (m_bIsMissionScript)
				CTheScripts::MissionCleanup.AddEntityToList(handle, CLEANUP_CAR);
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
		assert(pVehicle);
		CVehicle* pTarget = CPools::GetVehiclePool()->GetAt(ScriptParams[1]);
		assert(pTarget);
		CCarAI::TellCarToRamOtherCar(pVehicle, pTarget);
		return 0;
	}
	case COMMAND_SET_CAR_BLOCK_CAR:
	{
		CollectParameters(&m_nIp, 2);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		assert(pVehicle);
		CVehicle* pTarget = CPools::GetVehiclePool()->GetAt(ScriptParams[1]);
		assert(pTarget);
		CCarAI::TellCarToBlockOtherCar(pVehicle, pTarget);
		return 0;
	}
	case COMMAND_SET_CHAR_OBJ_CATCH_TRAIN:
	{
		CollectParameters(&m_nIp, 1);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		assert(pPed);
		pPed->bScriptObjectiveCompleted = false;
		pPed->SetObjective(OBJECTIVE_CATCH_TRAIN);
		return 0;
	}
	//case COMMAND_SET_COLL_OBJ_CATCH_TRAIN:
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
		assert(pPed);
		pPed->bPedIsBleeding = (ScriptParams[1] != 0);
		return 0;
	}
	case COMMAND_SET_CAR_FUNNY_SUSPENSION:
	{
		CollectParameters(&m_nIp, 2);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		assert(pVehicle);
		// no action
		return 0;
	}
	case COMMAND_SET_CAR_BIG_WHEELS:
	{
		CollectParameters(&m_nIp, 2);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		assert(pVehicle);
		assert(pVehicle->m_vehType == VEHICLE_TYPE_CAR);
		CAutomobile* pCar = (CAutomobile*)pVehicle;
		pCar->bBigWheels = (ScriptParams[1] != 0);
		return 0;
	}
	case COMMAND_SET_FREE_RESPRAYS:
		CollectParameters(&m_nIp, 1);
		CGarages::RespraysAreFree = (ScriptParams[0] != 0);
		return 0;
	case COMMAND_SET_PLAYER_VISIBLE:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CWorld::Players[ScriptParams[0]].m_pPed;
		assert(pPed);
		pPed->bIsVisible = (ScriptParams[1] != 0);
		return 0;
	}
	case COMMAND_SET_CHAR_VISIBLE:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		assert(pPed);
		pPed->bIsVisible = (ScriptParams[1] != 0);
		return 0;
	}
	case COMMAND_SET_CAR_VISIBLE:
	{
		CollectParameters(&m_nIp, 2);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		assert(pVehicle);
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
			ScriptParams[7], ScriptParams[8], ScriptParams[9], ScriptParams[10], ScriptParams[11]);
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
		DMAudio.PlayRadioAnnouncement(13); //TODO: enum?
		return 0;
	case COMMAND_COMMERCIAL_PASSED:
		CStats::CommercialPassed = true;
		DMAudio.PlayRadioAnnouncement(14); //TODO: enum?
		return 0;
	case COMMAND_SUBURBAN_PASSED:
		CStats::SuburbanPassed = true;
		return 0;
	case COMMAND_ROTATE_OBJECT:
	{
		CollectParameters(&m_nIp, 4);
		CObject* pObject = CPools::GetObjectPool()->GetAt(ScriptParams[0]);
		assert(pObject);
		float heading = LimitAngleOnCircle(
			RADTODEG(Atan2(-pObject->GetForward().x, pObject->GetForward().y)));
		float headingTarget = *(float*)&ScriptParams[1];
		float rotateBy = *(float*)&ScriptParams[2];
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
				min(cp1.x, min(cp2.x, min(cp3.x, cp4.x))),
				min(cp1.y, min(cp2.y, min(cp3.y, cp4.y))),
				max(cp1.x, max(cp2.x, max(cp3.x, cp4.x))),
				max(cp1.y, max(cp2.y, max(cp3.y, cp4.y))),
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
		assert(pObject);
		CVector pos = pObject->GetPosition();
		CVector posTarget = *(CVector*)&ScriptParams[1];
		CVector slideBy = *(CVector*)&ScriptParams[4];
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
			CWorld::FindObjectsIntersectingAngledCollisionBox(pColModel->boundingBox, tmp_matrix, pos,
				min(cp1.x, min(cp2.x, min(cp3.x, cp4.x))),
				min(cp1.y, min(cp2.y, min(cp3.y, cp4.y))),
				max(cp1.x, max(cp2.x, max(cp3.x, cp4.x))),
				max(cp1.y, max(cp2.y, max(cp3.y, cp4.y))),
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
						pPed->m_pMyVehicle->m_status = STATUS_ABANDONED;
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
			CTheScripts::MissionCleanup.RemoveEntityFromList(ScriptParams[0], CLEANUP_CHAR);
		return 0;
	}
	case COMMAND_SET_CHAR_STAY_IN_SAME_PLACE:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		assert(pPed);
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
		assert(pPed);
		char name[8];
		CTheScripts::ReadTextLabelFromScript(&m_nIp, name);
		for (int i = 0; i < 8; i++)
			name[i] = tolower(name[i]);
		int mi = pPed->GetModelIndex();
		pPed->DeleteRwObject();
		if (pPed->IsPlayer())
			mi = 0;
		CStreaming::RequestSpecialModel(mi, name, STREAMFLAGS_DEPENDENCY | STREAMFLAGS_SCRIPTOWNED);
		m_nIp += 8;
		CWorld::Remove(pPed);
		return 0;
	}
	case COMMAND_DRESS_CHAR:
	{
		CollectParameters(&m_nIp, 1);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		assert(pPed);
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
		char zone[8];
		CTheScripts::ReadTextLabelFromScript(&m_nIp, zone);
		int zone_id = CTheZones::FindZoneByLabelAndReturnIndex(zone);
		if (zone_id != -1)
			m_nIp += 8;
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
		assert(pObject);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[1]);
		assert(pVehicle);
		CVector offset = *(CVector*)&ScriptParams[2];
		CPhysical::PlacePhysicalRelativeToOtherPhysical(pVehicle, pObject, offset);
		return 0;
	}
	case COMMAND_MAKE_OBJECT_TARGETTABLE:
	{
		CollectParameters(&m_nIp, 1);
		CObject* pObject = CPools::GetObjectPool()->GetAt(ScriptParams[0]);
		assert(pObject);
		CPlayerPed* pPlayerPed = CWorld::Players[CWorld::PlayerInFocus].m_pPed;
		assert(pPlayerPed);
		pPlayerPed->MakeObjectTargettable(ScriptParams[0]);
		return 0;
	}
	case COMMAND_ADD_ARMOUR_TO_PLAYER:
	{
		CollectParameters(&m_nIp, 2);
		CPlayerPed* pPlayerPed = CWorld::Players[ScriptParams[0]].m_pPed;
		assert(pPlayerPed);
		pPlayerPed->m_fArmour = clamp(pPlayerPed->m_fArmour + ScriptParams[1], 0.0f, 100.0f);
		return 0;
	}
	case COMMAND_ADD_ARMOUR_TO_CHAR:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		assert(pPed);
		pPed->m_fArmour = clamp(pPed->m_fArmour + ScriptParams[1], 0.0f, 100.0f);
		return 0;
	}
	case COMMAND_OPEN_GARAGE:
	{
		CollectParameters(&m_nIp, 1);
		CGarages::Garages[ScriptParams[0]].OpenThisGarage();
		return 0;
	}
	case COMMAND_CLOSE_GARAGE:
	{
		CollectParameters(&m_nIp, 1);
		CGarages::Garages[ScriptParams[0]].CloseThisGarage();
		return 0;
	}
	case COMMAND_WARP_CHAR_FROM_CAR_TO_COORD:
	{
		CollectParameters(&m_nIp, 4);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		assert(pPed);
		CVector pos = *(CVector*)&ScriptParams[1];
		if (pos.z <= MAP_Z_LOW_LIMIT)
			pos.z = CWorld::FindGroundZForCoord(pos.x, pos.y);
		if (pPed->bInVehicle){
			if (pPed->m_pMyVehicle->bIsBus)
				pPed->bRenderPedInCar = true;
			if (pPed->m_pMyVehicle->pDriver == pPed){
				pPed->m_pMyVehicle->RemoveDriver();
				pPed->m_pMyVehicle->m_status = STATUS_ABANDONED;
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
		CAnimManager::BlendAnimation(pPed->GetClump(), pPed->m_animGroup, ANIM_IDLE_STANCE, 100.0f);
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
			CWorld::FindObjectsOfTypeInRangeSectorList(mi, CWorld::GetBigBuildingList(LEVEL_NONE), pos, range, true, &total, 16, apEntities);
		if (total == 0)
			CWorld::FindObjectsOfTypeInRangeSectorList(mi, CWorld::GetBigBuildingList(CTheZones::FindZoneForPoint(pos)), pos, range, true, &total, 16, apEntities);
		CEntity* pClosestEntity = nil;
		float min_dist = 2 * range;
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
		assert(pPed);
		CPed* pTarget = CPools::GetPedPool()->GetAt(ScriptParams[1]);
		assert(pTarget);
		UpdateCompareFlag(pPed->OurPedCanSeeThisOne(pTarget));
		return 0;
	}
	case COMMAND_SET_CHAR_OBJ_HAIL_TAXI:
	{
		CollectParameters(&m_nIp, 1);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		assert(pPed);
		pPed->bScriptObjectiveCompleted = false;
		pPed->SetObjective(OBJECTIVE_HAIL_TAXI);
		return 0;
	}
	case COMMAND_HAS_OBJECT_BEEN_DAMAGED:
	{
		CollectParameters(&m_nIp, 1);
		CObject* pObject = CPools::GetObjectPool()->GetAt(ScriptParams[0]);
		assert(pObject);
		UpdateCompareFlag(pObject->bHasBeenDamaged || !pObject->bIsVisible);
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
		assert(pPed);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[1]);
		assert(pVehicle);
		pPed->SetObjective(OBJECTIVE_ENTER_CAR_AS_DRIVER, pVehicle);
		pPed->WarpPedIntoCar(pVehicle);
		return 0;
	}
	case COMMAND_WARP_CHAR_INTO_CAR:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		assert(pPed);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[1]);
		assert(pVehicle);
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
		assert(pPed);
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
		int pedtype = PEDTYPE_COP;
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
		ped->GetPosition() = pos;
		ped->SetOrientation(0.0f, 0.0f, 0.0f);
		CTheScripts::ClearSpaceForMissionEntity(pos, ped);
		CWorld::Add(ped);
		ped->m_nZoneLevel = CTheZones::GetLevelFromPosition(pos);
		CPopulation::ms_nTotalMissionPeds++;
		ScriptParams[0] = CPools::GetPedPool()->GetIndex(ped);
		StoreParameters(&m_nIp, 1);
		if (m_bIsMissionScript)
			CTheScripts::MissionCleanup.AddEntityToList(ScriptParams[0], CLEANUP_CHAR);
		return 0;
	}
	case COMMAND_SET_CHAR_OBJ_STEAL_ANY_CAR:
	{
		CollectParameters(&m_nIp, 1);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		assert(pPed);
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
	//case COMMAND_SET_COLL_OBJ_STEAL_ANY_CAR:
	case COMMAND_SET_OBJECT_VELOCITY:
	{
		CollectParameters(&m_nIp, 4);
		CObject* pObject = CPools::GetObjectPool()->GetAt(ScriptParams[0]);
		assert(pObject);
		pObject->SetMoveSpeed(*(CVector*)&ScriptParams[1] * METERS_PER_SECOND_TO_GAME_SPEED);
		return 0;
	}
	case COMMAND_SET_OBJECT_COLLISION:
	{
		CollectParameters(&m_nIp, 2);
		CObject* pObject = CPools::GetObjectPool()->GetAt(ScriptParams[0]);
		assert(pObject);
		pObject->bUsesCollision = (ScriptParams[1] != 0);
		return 0;
	}
	case COMMAND_IS_ICECREAM_JINGLE_ON:
	{
		CollectParameters(&m_nIp, 1);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		assert(pVehicle);
		// Adding this check to correspond to command name.
		// All original game scripts always assume that the vehicle is actually Mr. Whoopee,
		// but maybe there are mods that use it as "is horn activated"?
		assert(pVehicle->GetModelIndex() == MI_MRWHOOP);
		UpdateCompareFlag(pVehicle->m_bSirenOrAlarm);
		return 0;
	}
	default:
		assert(0);
	}
	return -1;
}
#endif

#if 1
WRAPPER int8 CRunningScript::ProcessCommandsFrom900To999(int32 command) { EAXJMP(0x44CB80); }
#else
int8 CRunningScript::ProcessCommandsFrom900To999(int32 command)
{
	switch (command){
	case COMMAND_PRINT_STRING_IN_STRING_NOW:
	case COMMAND_PRINT_STRING_IN_STRING_SOON:
	case COMMAND_SET_5_REPEATED_PHONE_MESSAGES:
	case COMMAND_SET_5_PHONE_MESSAGES:
	case COMMAND_SET_6_REPEATED_PHONE_MESSAGES:
	case COMMAND_SET_6_PHONE_MESSAGES:
	case COMMAND_IS_POINT_OBSCURED_BY_A_MISSION_ENTITY:
	case COMMAND_LOAD_ALL_MODELS_NOW:
	case COMMAND_ADD_TO_OBJECT_VELOCITY:
	case COMMAND_DRAW_SPRITE:
	case COMMAND_DRAW_RECT:
	case COMMAND_LOAD_SPRITE:
	case COMMAND_LOAD_TEXTURE_DICTIONARY:
	case COMMAND_REMOVE_TEXTURE_DICTIONARY:
	case COMMAND_SET_OBJECT_DYNAMIC:
	case COMMAND_SET_CHAR_ANIM_SPEED:
	case COMMAND_PLAY_MISSION_PASSED_TUNE:
	case COMMAND_CLEAR_AREA:
	case COMMAND_FREEZE_ONSCREEN_TIMER:
	case COMMAND_SWITCH_CAR_SIREN:
	case COMMAND_SWITCH_PED_ROADS_ON_ANGLED:
	case COMMAND_SWITCH_PED_ROADS_OFF_ANGLED:
	case COMMAND_SWITCH_ROADS_ON_ANGLED:
	case COMMAND_SWITCH_ROADS_OFF_ANGLED:
	case COMMAND_SET_CAR_WATERTIGHT:
	case COMMAND_ADD_MOVING_PARTICLE_EFFECT:
	case COMMAND_SET_CHAR_CANT_BE_DRAGGED_OUT:
	case COMMAND_TURN_CAR_TO_FACE_COORD:
	case COMMAND_IS_CRANE_LIFTING_CAR:
	case COMMAND_DRAW_SPHERE:
	case COMMAND_SET_CAR_STATUS:
	case COMMAND_IS_CHAR_MALE:
	case COMMAND_SCRIPT_NAME:
	case COMMAND_CHANGE_GARAGE_TYPE_WITH_CAR_MODEL:
	case COMMAND_FIND_DRUG_PLANE_COORDINATES:
	case COMMAND_SAVE_INT_TO_DEBUG_FILE:
	case COMMAND_SAVE_FLOAT_TO_DEBUG_FILE:
	case COMMAND_SAVE_NEWLINE_TO_DEBUG_FILE:
	case COMMAND_POLICE_RADIO_MESSAGE:
	case COMMAND_SET_CAR_STRONG:
	case COMMAND_REMOVE_ROUTE:
	case COMMAND_SWITCH_RUBBISH:
	case COMMAND_REMOVE_PARTICLE_EFFECTS_IN_AREA:
	case COMMAND_SWITCH_STREAMING:
	case COMMAND_IS_GARAGE_OPEN:
	case COMMAND_IS_GARAGE_CLOSED:
	case COMMAND_START_CATALINA_HELI:
	case COMMAND_CATALINA_HELI_TAKE_OFF:
	case COMMAND_REMOVE_CATALINA_HELI:
	case COMMAND_HAS_CATALINA_HELI_BEEN_SHOT_DOWN:
	case COMMAND_SWAP_NEAREST_BUILDING_MODEL:
	case COMMAND_SWITCH_WORLD_PROCESSING:
	case COMMAND_REMOVE_ALL_PLAYER_WEAPONS:
	case COMMAND_GRAB_CATALINA_HELI:
	case COMMAND_CLEAR_AREA_OF_CARS:
	case COMMAND_SET_ROTATING_GARAGE_DOOR:
	case COMMAND_ADD_SPHERE:
	case COMMAND_REMOVE_SPHERE:
	case COMMAND_CATALINA_HELI_FLY_AWAY:
	case COMMAND_SET_EVERYONE_IGNORE_PLAYER:
	case COMMAND_STORE_CAR_CHAR_IS_IN_NO_SAVE:
	case COMMAND_STORE_CAR_PLAYER_IS_IN_NO_SAVE:
	case COMMAND_IS_PHONE_DISPLAYING_MESSAGE:
	case COMMAND_DISPLAY_ONSCREEN_TIMER_WITH_STRING:
	case COMMAND_DISPLAY_ONSCREEN_COUNTER_WITH_STRING:
	case COMMAND_CREATE_RANDOM_CAR_FOR_CAR_PARK:
	case COMMAND_IS_COLLISION_IN_MEMORY:
	case COMMAND_SET_WANTED_MULTIPLIER:
	case COMMAND_SET_CAMERA_IN_FRONT_OF_PLAYER:
	case COMMAND_IS_CAR_VISIBLY_DAMAGED:
	case COMMAND_DOES_OBJECT_EXIST:
	case COMMAND_LOAD_SCENE:
	case COMMAND_ADD_STUCK_CAR_CHECK:
	case COMMAND_REMOVE_STUCK_CAR_CHECK:
	case COMMAND_IS_CAR_STUCK:
	case COMMAND_LOAD_MISSION_AUDIO:
	case COMMAND_HAS_MISSION_AUDIO_LOADED:
	case COMMAND_PLAY_MISSION_AUDIO:
	case COMMAND_HAS_MISSION_AUDIO_FINISHED:
	case COMMAND_GET_CLOSEST_CAR_NODE_WITH_HEADING:
	case COMMAND_HAS_IMPORT_GARAGE_SLOT_BEEN_FILLED:
	case COMMAND_CLEAR_THIS_PRINT:
	case COMMAND_CLEAR_THIS_BIG_PRINT:
	case COMMAND_SET_MISSION_AUDIO_POSITION:
	case COMMAND_ACTIVATE_SAVE_MENU:
	case COMMAND_HAS_SAVE_GAME_FINISHED:
	case COMMAND_NO_SPECIAL_CAMERA_FOR_THIS_GARAGE:
	case COMMAND_ADD_BLIP_FOR_PICKUP_OLD:
	case COMMAND_ADD_BLIP_FOR_PICKUP:
	case COMMAND_ADD_SPRITE_BLIP_FOR_PICKUP:
	case COMMAND_SET_PED_DENSITY_MULTIPLIER:
	case COMMAND_FORCE_RANDOM_PED_TYPE:
	case COMMAND_SET_TEXT_DRAW_BEFORE_FADE:
	case COMMAND_GET_COLLECTABLE1S_COLLECTED:
	case COMMAND_REGISTER_EL_BURRO_TIME:
	case COMMAND_SET_SPRITES_DRAW_BEFORE_FADE:
	case COMMAND_SET_TEXT_RIGHT_JUSTIFY:
	case COMMAND_PRINT_HELP:
	case COMMAND_CLEAR_HELP:
	case COMMAND_FLASH_HUD_OBJECT:
	default:
		assert(0);
	}
	return -1;
}
#endif

#if 1
WRAPPER int8 CRunningScript::ProcessCommandsFrom1000To1099(int32 command) { EAXJMP(0x588490); }
#else
int8 CRunningScript::ProcessCommandsFrom1000To1099(int32 command)
{
	switch (command){
	case COMMAND_FLASH_RADAR_BLIP:
	case COMMAND_IS_CHAR_IN_CONTROL:
	case COMMAND_SET_GENERATE_CARS_AROUND_CAMERA:
	case COMMAND_CLEAR_SMALL_PRINTS:
	case COMMAND_HAS_MILITARY_CRANE_COLLECTED_ALL_CARS:
	case COMMAND_SET_UPSIDEDOWN_CAR_NOT_DAMAGED:
	case COMMAND_CAN_PLAYER_START_MISSION:
	case COMMAND_MAKE_PLAYER_SAFE_FOR_CUTSCENE:
	case COMMAND_USE_TEXT_COMMANDS:
	case COMMAND_SET_THREAT_FOR_PED_TYPE:
	case COMMAND_CLEAR_THREAT_FOR_PED_TYPE:
	case COMMAND_GET_CAR_COLOURS:
	case COMMAND_SET_ALL_CARS_CAN_BE_DAMAGED:
	case COMMAND_SET_CAR_CAN_BE_DAMAGED:
	case COMMAND_MAKE_PLAYER_UNSAFE:
	case COMMAND_LOAD_COLLISION:
	case COMMAND_GET_BODY_CAST_HEALTH:
	case COMMAND_SET_CHARS_CHATTING:
	case COMMAND_MAKE_PLAYER_SAFE:
	case COMMAND_SET_CAR_STAYS_IN_CURRENT_LEVEL:
	case COMMAND_SET_CHAR_STAYS_IN_CURRENT_LEVEL:
	case COMMAND_REGISTER_4X4_ONE_TIME:
	case COMMAND_REGISTER_4X4_TWO_TIME:
	case COMMAND_REGISTER_4X4_THREE_TIME:
	case COMMAND_REGISTER_4X4_MAYHEM_TIME:
	case COMMAND_REGISTER_LIFE_SAVED:
	case COMMAND_REGISTER_CRIMINAL_CAUGHT:
	case COMMAND_REGISTER_AMBULANCE_LEVEL:
	case COMMAND_REGISTER_FIRE_EXTINGUISHED:
	case COMMAND_TURN_PHONE_ON:
	case COMMAND_REGISTER_LONGEST_DODO_FLIGHT:
	case COMMAND_REGISTER_DEFUSE_BOMB_TIME:
	case COMMAND_SET_TOTAL_NUMBER_OF_KILL_FRENZIES:
	case COMMAND_BLOW_UP_RC_BUGGY:
	case COMMAND_REMOVE_CAR_FROM_CHASE:
	case COMMAND_IS_FRENCH_GAME:
	case COMMAND_IS_GERMAN_GAME:
	case COMMAND_CLEAR_MISSION_AUDIO:
	case COMMAND_SET_FADE_IN_AFTER_NEXT_ARREST:
	case COMMAND_SET_FADE_IN_AFTER_NEXT_DEATH:
	case COMMAND_SET_GANG_PED_MODEL_PREFERENCE:
	case COMMAND_SET_CHAR_USE_PEDNODE_SEEK:
	case COMMAND_SWITCH_VEHICLE_WEAPONS:
	case COMMAND_SET_GET_OUT_OF_JAIL_FREE:
	case COMMAND_SET_FREE_HEALTH_CARE:
	case COMMAND_IS_CAR_DOOR_CLOSED:
	case COMMAND_LOAD_AND_LAUNCH_MISSION:
	case COMMAND_LOAD_AND_LAUNCH_MISSION_INTERNAL:
	case COMMAND_SET_OBJECT_DRAW_LAST:
	case COMMAND_GET_AMMO_IN_PLAYER_WEAPON:
	case COMMAND_GET_AMMO_IN_CHAR_WEAPON:
	case COMMAND_REGISTER_KILL_FRENZY_PASSED:
	case COMMAND_SET_CHAR_SAY:
	case COMMAND_SET_NEAR_CLIP:
	case COMMAND_SET_RADIO_CHANNEL:
	case COMMAND_OVERRIDE_HOSPITAL_LEVEL:
	case COMMAND_OVERRIDE_POLICE_STATION_LEVEL:
	case COMMAND_FORCE_RAIN:
	case COMMAND_DOES_GARAGE_CONTAIN_CAR:
	case COMMAND_SET_CAR_TRACTION:
	case COMMAND_ARE_MEASUREMENTS_IN_METRES:
	case COMMAND_CONVERT_METRES_TO_FEET:
	case COMMAND_MARK_ROADS_BETWEEN_LEVELS:
	case COMMAND_MARK_PED_ROADS_BETWEEN_LEVELS:
	case COMMAND_SET_CAR_AVOID_LEVEL_TRANSITIONS:
	case COMMAND_SET_CHAR_AVOID_LEVEL_TRANSITIONS:
	case COMMAND_IS_THREAT_FOR_PED_TYPE:
	case COMMAND_CLEAR_AREA_OF_CHARS:
	case COMMAND_SET_TOTAL_NUMBER_OF_MISSIONS:
	case COMMAND_CONVERT_METRES_TO_FEET_INT:
	case COMMAND_REGISTER_FASTEST_TIME:
	case COMMAND_REGISTER_HIGHEST_SCORE:
	case COMMAND_WARP_CHAR_INTO_CAR_AS_PASSENGER:
	case COMMAND_IS_CAR_PASSENGER_SEAT_FREE:
	case COMMAND_GET_CHAR_IN_CAR_PASSENGER_SEAT:
	case COMMAND_SET_CHAR_IS_CHRIS_CRIMINAL:
	case COMMAND_START_CREDITS:
	case COMMAND_STOP_CREDITS:
	case COMMAND_ARE_CREDITS_FINISHED:
	case COMMAND_CREATE_SINGLE_PARTICLE:
	case COMMAND_SET_CHAR_IGNORE_LEVEL_TRANSITIONS:
	case COMMAND_GET_CHASE_CAR:
	case COMMAND_START_BOAT_FOAM_ANIMATION:
	case COMMAND_UPDATE_BOAT_FOAM_ANIMATION:
	case COMMAND_SET_MUSIC_DOES_FADE:
	case COMMAND_SET_INTRO_IS_PLAYING:
	case COMMAND_SET_PLAYER_HOOKER:
	case COMMAND_PLAY_END_OF_GAME_TUNE:
	case COMMAND_STOP_END_OF_GAME_TUNE:
	case COMMAND_GET_CAR_MODEL:
	case COMMAND_IS_PLAYER_SITTING_IN_CAR:
	case COMMAND_IS_PLAYER_SITTING_IN_ANY_CAR:
	case COMMAND_SET_SCRIPT_FIRE_AUDIO:
	case COMMAND_ARE_ANY_CAR_CHEATS_ACTIVATED:
	case COMMAND_SET_CHAR_SUFFERS_CRITICAL_HITS:
	case COMMAND_IS_PLAYER_LIFTING_A_PHONE:
	case COMMAND_IS_CHAR_SITTING_IN_CAR:
	case COMMAND_IS_CHAR_SITTING_IN_ANY_CAR:
	case COMMAND_IS_PLAYER_ON_FOOT:
	case COMMAND_IS_CHAR_ON_FOOT:
	default:
		assert(0);
	}
	return -1;
}
#endif

#if 1
WRAPPER int8 CRunningScript::ProcessCommandsFrom1100To1199(int32 command) { EAXJMP(0x589D00); }
#else
int8 CRunningScript::ProcessCommandsFrom1100To1199(int32 command)
{
	switch (command) {
	case COMMAND_LOAD_COLLISION_WITH_SCREEN:
	case COMMAND_LOAD_SPLASH_SCREEN:
	case COMMAND_SET_CAR_IGNORE_LEVEL_TRANSITIONS:
	case COMMAND_MAKE_CRAIGS_CAR_A_BIT_STRONGER:
	case COMMAND_SET_JAMES_CAR_ON_PATH_TO_PLAYER:
	case COMMAND_LOAD_END_OF_GAME_TUNE:
	case COMMAND_ENABLE_PLAYER_CONTROL_CAMERA:
	case COMMAND_SET_OBJECT_ROTATION:
	case COMMAND_GET_DEBUG_CAMERA_COORDINATES:
	case COMMAND_GET_DEBUG_CAMERA_FRONT_VECTOR:
	case COMMAND_IS_PLAYER_TARGETTING_ANY_CHAR:
	case COMMAND_IS_PLAYER_TARGETTING_CHAR:
	case COMMAND_IS_PLAYER_TARGETTING_OBJECT:
	case COMMAND_TERMINATE_ALL_SCRIPTS_WITH_THIS_NAME:
	case COMMAND_DISPLAY_TEXT_WITH_NUMBER:
	case COMMAND_DISPLAY_TEXT_WITH_2_NUMBERS:
	case COMMAND_FAIL_CURRENT_MISSION:
	case COMMAND_GET_CLOSEST_OBJECT_OF_TYPE:
	case COMMAND_PLACE_OBJECT_RELATIVE_TO_OBJECT:
	case COMMAND_SET_ALL_OCCUPANTS_OF_CAR_LEAVE_CAR:
	case COMMAND_SET_INTERPOLATION_PARAMETERS:
	case COMMAND_GET_CLOSEST_CAR_NODE_WITH_HEADING_TOWARDS_POINT:
	case COMMAND_GET_CLOSEST_CAR_NODE_WITH_HEADING_AWAY_POINT:
	case COMMAND_GET_DEBUG_CAMERA_POINT_AT:
	case COMMAND_ATTACH_CHAR_TO_CAR:
	case COMMAND_DETACH_CHAR_FROM_CAR:
	case COMMAND_SET_CAR_STAY_IN_FAST_LANE:
	case COMMAND_CLEAR_CHAR_LAST_WEAPON_DAMAGE:
	case COMMAND_CLEAR_CAR_LAST_WEAPON_DAMAGE:
	case COMMAND_GET_RANDOM_COP_IN_AREA:
	case COMMAND_GET_RANDOM_COP_IN_ZONE:
	case COMMAND_SET_CHAR_OBJ_FLEE_CAR:
	case COMMAND_GET_DRIVER_OF_CAR:
	case COMMAND_GET_NUMBER_OF_FOLLOWERS:
	case COMMAND_GIVE_REMOTE_CONTROLLED_MODEL_TO_PLAYER:
	case COMMAND_GET_CURRENT_PLAYER_WEAPON:
	case COMMAND_GET_CURRENT_CHAR_WEAPON:
	case COMMAND_LOCATE_CHAR_ANY_MEANS_OBJECT_2D:
	case COMMAND_LOCATE_CHAR_ON_FOOT_OBJECT_2D:
	case COMMAND_LOCATE_CHAR_IN_CAR_OBJECT_2D:
	case COMMAND_LOCATE_CHAR_ANY_MEANS_OBJECT_3D:
	case COMMAND_LOCATE_CHAR_ON_FOOT_OBJECT_3D:
	case COMMAND_LOCATE_CHAR_IN_CAR_OBJECT_3D:
	case COMMAND_SET_CAR_TEMP_ACTION:
	case COMMAND_SET_CAR_HANDBRAKE_TURN_RIGHT:
	case COMMAND_SET_CAR_HANDBRAKE_STOP:
	case COMMAND_IS_CHAR_ON_ANY_BIKE:
	case COMMAND_LOCATE_SNIPER_BULLET_2D:
	case COMMAND_LOCATE_SNIPER_BULLET_3D:
	case COMMAND_GET_NUMBER_OF_SEATS_IN_MODEL:
	case COMMAND_IS_PLAYER_ON_ANY_BIKE:
	case COMMAND_IS_CHAR_LYING_DOWN:
	case COMMAND_CAN_CHAR_SEE_DEAD_CHAR:
	case COMMAND_SET_ENTER_CAR_RANGE_MULTIPLIER:
	default:
		assert(0);
	}
	return -1;
}
#endif

int16 CRunningScript::GetPadState(uint16 pad, uint16 button)
{
	CPad* pPad = CPad::GetPad(pad);
	switch (button){
	case 0: return pPad->NewState.LeftStickX;
	case 1: return pPad->NewState.LeftStickY;
	case 2: return pPad->NewState.RightStickX;
	case 3: return pPad->NewState.RightStickY;
	case 4: return pPad->NewState.LeftShoulder1;
	case 5: return pPad->NewState.LeftShoulder2;
	case 6: return pPad->NewState.RightShoulder1;
	case 7: return pPad->NewState.RightShoulder2;
	case 8: return pPad->NewState.DPadUp;
	case 9: return pPad->NewState.DPadDown;
	case 10: return pPad->NewState.DPadLeft;
	case 11: return pPad->NewState.DPadRight;
	case 12: return pPad->NewState.Start;
	case 13: return pPad->NewState.Select;
	case 14: return pPad->NewState.Square;
	case 15: return pPad->NewState.Triangle;
	case 16: return pPad->NewState.Cross;
	case 17: return pPad->NewState.Circle;
	case 18: return pPad->NewState.LeftShock;
	case 19: return pPad->NewState.RightShock;
	default: break;
	}
	return 0;
}

bool CTheScripts::IsVehicleStopped(CVehicle* pVehicle)
{
	return 0.01f * CTimer::GetTimeStep() >= pVehicle->m_fDistanceTravelled;
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
		int j = 0;
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

WRAPPER void CRunningScript::LocatePlayerCommand(int32, uint32*) { EAXJMP(0x44FE10); }
WRAPPER void CRunningScript::LocatePlayerCharCommand(int32, uint32*) { EAXJMP(0x4501E0); }
WRAPPER void CRunningScript::LocatePlayerCarCommand(int32, uint32*) { EAXJMP(0x450540); }
WRAPPER void CRunningScript::LocateCharCommand(int32, uint32*) { EAXJMP(0x450870); }
WRAPPER void CRunningScript::LocateCharCharCommand(int32, uint32*) { EAXJMP(0x450BF0); }
WRAPPER void CRunningScript::LocateCharCarCommand(int32, uint32*) { EAXJMP(0x450F30); }
WRAPPER void CRunningScript::LocateCharObjectCommand(int32, uint32*) { EAXJMP(0x451260); }
WRAPPER void CRunningScript::LocateCarCommand(int32, uint32*) { EAXJMP(0x451590); }
WRAPPER void CRunningScript::LocateSniperBulletCommand(int32, uint32*) { EAXJMP(0x4518A0); }
WRAPPER void CRunningScript::PlayerInAreaCheckCommand(int32, uint32*) { EAXJMP(0x451A60); }
WRAPPER void CRunningScript::PlayerInAngledAreaCheckCommand(int32, uint32*) { EAXJMP(0x451E50); }
WRAPPER void CRunningScript::CharInAreaCheckCommand(int32, uint32*) { EAXJMP(0x4523B0); }
WRAPPER void CRunningScript::CarInAreaCheckCommand(int32, uint32*) { EAXJMP(0x452750); }

WRAPPER void CTheScripts::DrawScriptSpheres() { EAXJMP(0x44FAC0); }
WRAPPER void CRunningScript::DoDeatharrestCheck() { EAXJMP(0x452A30); }
WRAPPER void CTheScripts::DrawDebugSquare(float, float, float, float) { EAXJMP(0x452D00); }
WRAPPER void CTheScripts::DrawDebugCube(float, float, float, float, float, float) { EAXJMP(0x453100); }
WRAPPER void CTheScripts::ScriptDebugLine3D(float x1, float y1, float z1, float x2, float y2, float z2, int col, int col2) { EAXJMP(0x4534E0); }
WRAPPER void CTheScripts::ClearSpaceForMissionEntity(const CVector&, CEntity*) { EAXJMP(0x454060); }
WRAPPER void CTheScripts::HighlightImportantArea(uint32, float, float, float, float, float) { EAXJMP(0x454320); }
WRAPPER void CTheScripts::CleanUpThisVehicle(CVehicle*) { EAXJMP(0x4548D0); }
WRAPPER void CTheScripts::CleanUpThisPed(CPed*) { EAXJMP(0x4547A0); }
WRAPPER void CTheScripts::CleanUpThisObject(CObject*) { EAXJMP(0x454910); }
WRAPPER void CTheScripts::ReadObjectNamesFromScript() { EAXJMP(0x454960); }
WRAPPER void CTheScripts::UpdateObjectIndices() { EAXJMP(0x454AD0); }
WRAPPER void CTheScripts::ReadMultiScriptFileOffsetsFromScript() { EAXJMP(0x454BC0); }
WRAPPER bool CTheScripts::IsPedStopped(CPed*) { EAXJMP(0x454670); }
WRAPPER bool CTheScripts::IsPlayerStopped(CPlayerInfo*) { EAXJMP(0x4546C0); }

STARTPATCHES
InjectHook(0x437AE0, &CMissionCleanup::Init, PATCH_JUMP);
InjectHook(0x437BA0, &CMissionCleanup::AddEntityToList, PATCH_JUMP);
InjectHook(0x437BD0, &CMissionCleanup::RemoveEntityFromList, PATCH_JUMP);
InjectHook(0x437C10, &CMissionCleanup::Process, PATCH_JUMP);
InjectHook(0x437DC0, &CUpsideDownCarCheck::Init, PATCH_JUMP);
InjectHook(0x437EE0, &CUpsideDownCarCheck::UpdateTimers, PATCH_JUMP);
InjectHook(0x437F80, &CUpsideDownCarCheck::AreAnyCarsUpsideDown, PATCH_JUMP);
InjectHook(0x437FB0, &CUpsideDownCarCheck::AddCarToCheck, PATCH_JUMP);
InjectHook(0x437FE0, &CUpsideDownCarCheck::RemoveCarFromCheck, PATCH_JUMP);
InjectHook(0x438010, &CUpsideDownCarCheck::HasCarBeenUpsideDownForAWhile, PATCH_JUMP);
InjectHook(0x438050, &CStuckCarCheck::Init, PATCH_JUMP);
InjectHook(0x4380A0, &CStuckCarCheck::Process, PATCH_JUMP);
InjectHook(0x4381C0, &CStuckCarCheck::AddCarToCheck, PATCH_JUMP);
InjectHook(0x438240, &CStuckCarCheck::RemoveCarFromCheck, PATCH_JUMP);
InjectHook(0x4382A0, &CStuckCarCheck::HasCarBeenStuckForAWhile, PATCH_JUMP);
InjectHook(0x4382E0, &CRunningScript::CollectParameters, PATCH_JUMP);
InjectHook(0x438460, &CRunningScript::CollectNextParameterWithoutIncreasingPC, PATCH_JUMP);
InjectHook(0x4385A0, &CRunningScript::StoreParameters, PATCH_JUMP);
InjectHook(0x438640, &CRunningScript::GetPointerToScriptVariable, PATCH_JUMP);
InjectHook(0x438790, &CTheScripts::Init, PATCH_JUMP);
InjectHook(0x439000, &CTheScripts::StartNewScript, PATCH_JUMP);
InjectHook(0x439040, &CTheScripts::Process, PATCH_JUMP);
InjectHook(0x439400, &CTheScripts::StartTestScript, PATCH_JUMP);
InjectHook(0x439410, &CTheScripts::IsPlayerOnAMission, PATCH_JUMP);
ENDPATCHES
