#pragma warning( push )
#pragma warning( disable : 4005)
#if defined RW_D3D9 || defined RWLIBS
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#endif
#pragma warning( pop )

#include "common.h"
#include "crossplatform.h"
#include "platform.h"
#ifdef XINPUT
#include <xinput.h>
#if !defined(PSAPI_VERSION) || (PSAPI_VERSION > 1)
#pragma comment( lib, "Xinput9_1_0.lib" )
#else
#pragma comment( lib, "Xinput.lib" )
#endif
#endif

#include "vehicle_enums.h" //needs to be before Vehicle.h

#include "Vehicle.h"

#include "Camera.h"
#include "CarCtrl.h"
#include "ControllerConfig.h"
#include "CutsceneMgr.h"
#include "Fluff.h"
#include "Font.h"
#include "Frontend.h"
#include "Game.h"
#include "Gangs.h"
#include "General.h"
#include "Hud.h"
#include "Pad.h"
#include "PathFind.h"
#include "Ped.h"
#include "Population.h"
#include "Record.h"
#include "Replay.h"
#include "Stats.h"
#include "Streaming.h"
#include "Text.h"
#include "Timer.h"
#include "TrafficLights.h"
#include "Wanted.h"
#include "WaterLevel.h"
#include "Weather.h"
#include "World.h"
#include "platform.h"

#ifdef GTA_PS2
#include "eetypes.h"
#include "libpad.h"
#endif

// --MIAMI: file done except Mobile(see TODOs) and PS2 stuff

CPad Pads[MAX_PADS];
#ifdef GTA_PS2
u_long128 pad_dma_buf[scePadDmaBufferMax] __attribute__((aligned(64)));
u_long128 pad2_dma_buf[scePadDmaBufferMax] __attribute__((aligned(64)));
#endif

CMousePointerStateHelper MousePointerStateHelper;

bool CPad::bDisplayNoControllerMessage;
bool CPad::bObsoleteControllerMessage;
bool CPad::bOldDisplayNoControllerMessage;
bool CPad::m_bMapPadOneToPadTwo;
bool CPad::m_bDebugCamPCOn;
bool CPad::bHasPlayerCheated;
bool CPad::bInvertLook4Pad;
#ifdef GTA_PS2
unsigned char act_direct[6];
unsigned char act_align[6];
#endif

CKeyboardState CPad::OldKeyState;
CKeyboardState CPad::NewKeyState;
CKeyboardState CPad::TempKeyState;

char CPad::KeyBoardCheatString[30];

CMouseControllerState CPad::OldMouseControllerState;
CMouseControllerState CPad::NewMouseControllerState;
CMouseControllerState CPad::PCTempMouseControllerState;

#ifdef DETECT_PAD_INPUT_SWITCH
bool CPad::IsAffectedByController = false;
#endif

_TODO("gbFastTime");
extern bool gbFastTime;

#ifdef WALLCLIMB_CHEAT
extern bool gGravityCheat;
#endif

void SpecialCarCheats()
{
	if ( !CVehicle::bCheat9 )
	{
		((CVehicleModelInfo*)CModelInfo::GetModelInfo(MI_INFERNUS))->m_wheelScale *= 1.3f;
		((CVehicleModelInfo*)CModelInfo::GetModelInfo(MI_CHEETAH))->m_wheelScale *= 1.3f;
		((CVehicleModelInfo*)CModelInfo::GetModelInfo(MI_PHEONIX))->m_wheelScale *= 1.3f;
		((CVehicleModelInfo*)CModelInfo::GetModelInfo(MI_DELUXO))->m_wheelScale *= 1.3f;
		mod_HandlingManager.GetHandlingData(HANDLING_LANDSTAL)->Transmission.fEngineAcceleration *= 2.0f;
		mod_HandlingManager.GetHandlingData(HANDLING_PATRIOT)->Transmission.fEngineAcceleration *= 2.0f;
		mod_HandlingManager.GetHandlingData(HANDLING_BOBCAT)->Transmission.fEngineAcceleration *= 2.0f;
		mod_HandlingManager.GetHandlingData(HANDLING_BFINJECT)->Transmission.fEngineAcceleration *= 2.0f;
		mod_HandlingManager.GetHandlingData(HANDLING_RANCHER)->Transmission.fEngineAcceleration *= 2.0f;
		mod_HandlingManager.GetHandlingData(HANDLING_DESPERAD)->Transmission.fEngineAcceleration *= 2.0f;
		mod_HandlingManager.GetHandlingData(HANDLING_DELUXO)->Transmission.fEngineAcceleration *= 2.0f;

		mod_HandlingManager.GetHandlingData(HANDLING_BAGGAGE)->Transmission.fEngineAcceleration *= 2.0f;
		mod_HandlingManager.GetHandlingData(HANDLING_BAGGAGE)->Transmission.fMaxVelocity *= 2.0f;
		mod_HandlingManager.GetHandlingData(HANDLING_BAGGAGE)->Transmission.InitGearRatios();

		mod_HandlingManager.GetHandlingData(HANDLING_GOLFCART)->Transmission.fEngineAcceleration *= 2.0f;
		mod_HandlingManager.GetHandlingData(HANDLING_GOLFCART)->Transmission.fMaxVelocity *= 2.0f;
		mod_HandlingManager.GetHandlingData(HANDLING_GOLFCART)->Transmission.InitGearRatios();

		mod_HandlingManager.GetHandlingData(HANDLING_STINGER)->fCollisionDamageMultiplier *= 0.25f;
		mod_HandlingManager.GetHandlingData(HANDLING_STINGER)->fMass *= 2.5f;
		mod_HandlingManager.GetHandlingData(HANDLING_STINGER)->fTurnMass *= 4.0f;

		mod_HandlingManager.GetHandlingData(HANDLING_BANSHEE)->fCollisionDamageMultiplier *= 0.25f;
		mod_HandlingManager.GetHandlingData(HANDLING_BANSHEE)->fMass *= 2.5f;
		mod_HandlingManager.GetHandlingData(HANDLING_BANSHEE)->fTurnMass *= 4.0f;

		mod_HandlingManager.GetHandlingData(HANDLING_SABRETUR)->fCollisionDamageMultiplier *= 0.25f;
		mod_HandlingManager.GetHandlingData(HANDLING_SABRETUR)->fMass *= 2.5f;
		mod_HandlingManager.GetHandlingData(HANDLING_SABRETUR)->fTurnMass *= 4.0f;

		mod_HandlingManager.GetHandlingData(HANDLING_COMET)->fCollisionDamageMultiplier *= 0.25f;
		mod_HandlingManager.GetHandlingData(HANDLING_COMET)->fMass *= 2.5f;
		mod_HandlingManager.GetHandlingData(HANDLING_COMET)->fTurnMass *= 4.0f;

		mod_HandlingManager.GetHandlingData(HANDLING_DELUXO)->fCollisionDamageMultiplier *= 0.25f;
		mod_HandlingManager.GetHandlingData(HANDLING_DELUXO)->fMass *= 2.5f;
		mod_HandlingManager.GetHandlingData(HANDLING_DELUXO)->fTurnMass *= 4.0f;
		CHud::SetHelpMessage(TheText.Get("CHEAT1"), true);
		CVehicle::bCheat9 = true;
		CPad::bHasPlayerCheated = true;
	}
}

void PickUpChicksCheat()
{
	if ( FindPlayerVehicle() && (FindPlayerVehicle()->IsCar() || FindPlayerVehicle()->IsBike()) )
	{
		CVehicle *vehicle = FindPlayerVehicle();
		if ( FindPlayerVehicle()->m_vehType == 5 )
		{
			if ( vehicle->pPassengers[0] )
				vehicle->pPassengers[0]->SetObjective(OBJECTIVE_LEAVE_CAR, vehicle);
		}
		CPed* someoneElse = (CPed*)CWorld::TestSphereAgainstWorld(vehicle->GetPosition(), 6.0f, FindPlayerPed(), false, false, true, false, false, false);
		if ( someoneElse && someoneElse->m_nPedState != PED_DRIVING )
		{
			CHud::SetHelpMessage(TheText.Get("CHEAT1"), true);
			someoneElse->SetObjective(OBJECTIVE_ENTER_CAR_AS_PASSENGER, vehicle);
		}
	}
}

void WeaponCheat1()
{
	CHud::SetHelpMessage(TheText.Get("CHEAT2"), true);

	CStreaming::RequestModel(MI_BRASS_KNUCKLES, STREAMFLAGS_DONT_REMOVE);
	CStreaming::RequestModel(MI_BASEBALL_BAT, STREAMFLAGS_DONT_REMOVE);
	CStreaming::RequestModel(MI_MOLOTOV, STREAMFLAGS_DONT_REMOVE);
	CStreaming::RequestModel(MI_COLT45, STREAMFLAGS_DONT_REMOVE);
	CStreaming::RequestModel(MI_SHOTGUN, STREAMFLAGS_DONT_REMOVE);
	CStreaming::RequestModel(MI_TEC9, STREAMFLAGS_DONT_REMOVE);
	CStreaming::RequestModel(MI_RUGER, STREAMFLAGS_DONT_REMOVE);
	CStreaming::RequestModel(MI_SNIPERRIFLE, STREAMFLAGS_DONT_REMOVE);
	CStreaming::RequestModel(MI_FLAMETHROWER, STREAMFLAGS_DONT_REMOVE);
	CStreaming::LoadAllRequestedModels(false);

	FindPlayerPed()->GiveWeapon(WEAPONTYPE_BRASSKNUCKLE, 1);
	FindPlayerPed()->GiveWeapon(WEAPONTYPE_BASEBALLBAT, 1);
	FindPlayerPed()->GiveWeapon(WEAPONTYPE_MOLOTOV, 10);
	FindPlayerPed()->GiveWeapon(WEAPONTYPE_COLT45, 100);
	FindPlayerPed()->GiveWeapon(WEAPONTYPE_SHOTGUN, 50);
	FindPlayerPed()->GiveWeapon(WEAPONTYPE_TEC9, 150);
	FindPlayerPed()->GiveWeapon(WEAPONTYPE_RUGER, 120);
	FindPlayerPed()->GiveWeapon(WEAPONTYPE_SNIPERRIFLE, 25);
	FindPlayerPed()->GiveWeapon(WEAPONTYPE_FLAMETHROWER, 200);

	CStreaming::SetModelIsDeletable(MI_BRASS_KNUCKLES);
	CStreaming::SetModelIsDeletable(MI_BASEBALL_BAT);
	CStreaming::SetModelIsDeletable(MI_MOLOTOV);
	CStreaming::SetModelIsDeletable(MI_COLT45);
	CStreaming::SetModelIsDeletable(MI_SHOTGUN);
	CStreaming::SetModelIsDeletable(MI_TEC9);
	CStreaming::SetModelIsDeletable(MI_RUGER);
	CStreaming::SetModelIsDeletable(MI_SNIPERRIFLE);
	CStreaming::SetModelIsDeletable(MI_FLAMETHROWER);
#ifdef MOBILE_IMPROVEMENTS
	if (FindPlayerVehicle()) {
		FindPlayerPed()->RemoveWeaponWhenEnteringVehicle();
	}
#endif
}

void WeaponCheat2()
{
	CHud::SetHelpMessage(TheText.Get("CHEAT2"), true);

	CStreaming::RequestModel(MI_KATANA, STREAMFLAGS_DONT_REMOVE);
	CStreaming::RequestModel(MI_GRENADE, STREAMFLAGS_DONT_REMOVE);
	CStreaming::RequestModel(MI_BOMB, STREAMFLAGS_DONT_REMOVE);
	CStreaming::RequestModel(MI_PYTHON, STREAMFLAGS_DONT_REMOVE);
	CStreaming::RequestModel(MI_STUBBY_SHOTGUN, STREAMFLAGS_DONT_REMOVE);
	CStreaming::RequestModel(MI_SILENCEDINGRAM, STREAMFLAGS_DONT_REMOVE);
	CStreaming::RequestModel(MI_M4, STREAMFLAGS_DONT_REMOVE);
	CStreaming::RequestModel(MI_LASERSCOPE, STREAMFLAGS_DONT_REMOVE);
	CStreaming::RequestModel(MI_ROCKETLAUNCHER, STREAMFLAGS_DONT_REMOVE);
	CStreaming::LoadAllRequestedModels(false);

#ifdef FIX_BUGS
	FindPlayerPed()->GiveWeapon(WEAPONTYPE_KATANA, 1);
#else
	FindPlayerPed()->GiveWeapon(WEAPONTYPE_KATANA, 0);
#endif
	FindPlayerPed()->GiveWeapon(WEAPONTYPE_DETONATOR_GRENADE, 10);
	FindPlayerPed()->GiveWeapon(WEAPONTYPE_PYTHON, 40);
	FindPlayerPed()->GiveWeapon(WEAPONTYPE_STUBBY_SHOTGUN, 25);
	FindPlayerPed()->GiveWeapon(WEAPONTYPE_SILENCED_INGRAM, 100);
	FindPlayerPed()->GiveWeapon(WEAPONTYPE_M4, 150);
	FindPlayerPed()->GiveWeapon(WEAPONTYPE_LASERSCOPE, 21);
	FindPlayerPed()->GiveWeapon(WEAPONTYPE_ROCKETLAUNCHER, 5);

	CStreaming::SetModelIsDeletable(MI_KATANA);
	CStreaming::SetModelIsDeletable(MI_GRENADE);
	CStreaming::SetModelIsDeletable(MI_BOMB);
	CStreaming::SetModelIsDeletable(MI_PYTHON);
	CStreaming::SetModelIsDeletable(MI_STUBBY_SHOTGUN);
	CStreaming::SetModelIsDeletable(MI_SILENCEDINGRAM);
	CStreaming::SetModelIsDeletable(MI_M4);
	CStreaming::SetModelIsDeletable(MI_LASERSCOPE);
	CStreaming::SetModelIsDeletable(MI_ROCKETLAUNCHER);
#ifdef MOBILE_IMPROVEMENTS
	if (FindPlayerVehicle()) {
		FindPlayerPed()->RemoveWeaponWhenEnteringVehicle();
	}
#endif
}

void WeaponCheat3()
{
	CHud::SetHelpMessage(TheText.Get("CHEAT2"), true);

	CStreaming::RequestModel(MI_CHAINSAW, STREAMFLAGS_DONT_REMOVE);
	CStreaming::RequestModel(MI_GRENADE, STREAMFLAGS_DONT_REMOVE);
	CStreaming::RequestModel(MI_PYTHON, STREAMFLAGS_DONT_REMOVE);
	CStreaming::RequestModel(MI_SPAS12_SHOTGUN, STREAMFLAGS_DONT_REMOVE);
	CStreaming::RequestModel(MI_MP5, STREAMFLAGS_DONT_REMOVE);
	CStreaming::RequestModel(MI_M4, STREAMFLAGS_DONT_REMOVE);
	CStreaming::RequestModel(MI_LASERSCOPE, STREAMFLAGS_DONT_REMOVE);
	CStreaming::RequestModel(MI_MINIGUN, STREAMFLAGS_DONT_REMOVE);
	CStreaming::RequestModel(MI_MINIGUN2, STREAMFLAGS_DONT_REMOVE);
	CStreaming::LoadAllRequestedModels(false);

#ifdef FIX_BUGS
	FindPlayerPed()->GiveWeapon(WEAPONTYPE_CHAINSAW, 1);
#else
	FindPlayerPed()->GiveWeapon(WEAPONTYPE_CHAINSAW, 0);
#endif
	FindPlayerPed()->GiveWeapon(WEAPONTYPE_GRENADE, 10);
	FindPlayerPed()->GiveWeapon(WEAPONTYPE_PYTHON, 40);
	FindPlayerPed()->GiveWeapon(WEAPONTYPE_SPAS12_SHOTGUN, 30);
	FindPlayerPed()->GiveWeapon(WEAPONTYPE_MP5, 100);
	FindPlayerPed()->GiveWeapon(WEAPONTYPE_M4, 150);
	FindPlayerPed()->GiveWeapon(WEAPONTYPE_LASERSCOPE, 21);
	FindPlayerPed()->GiveWeapon(WEAPONTYPE_MINIGUN, 500);

	CStreaming::SetModelIsDeletable(MI_CHAINSAW);
	CStreaming::SetModelIsDeletable(MI_GRENADE);
	CStreaming::SetModelIsDeletable(MI_PYTHON);
	CStreaming::SetModelIsDeletable(MI_SPAS12_SHOTGUN);
	CStreaming::SetModelIsDeletable(MI_MP5);
	CStreaming::SetModelIsDeletable(MI_M4);
	CStreaming::SetModelIsDeletable(MI_LASERSCOPE);
	CStreaming::SetModelIsDeletable(MI_MINIGUN);
	CStreaming::SetModelIsDeletable(MI_MINIGUN2);

#ifdef MOBILE_IMPROVEMENTS
	if (FindPlayerVehicle()) {
		FindPlayerPed()->RemoveWeaponWhenEnteringVehicle();
	}
#endif
}

void HealthCheat()
{
	CHud::SetHelpMessage(TheText.Get("CHEAT3"), true);
	FindPlayerPed()->m_fHealth = CWorld::Players[0].m_nMaxHealth;
	if (FindPlayerVehicle()) {
		FindPlayerVehicle()->m_fHealth = 1000.0f;
		if (FindPlayerVehicle()->m_vehType == VEHICLE_TYPE_CAR) {
			((CAutomobile*)FindPlayerVehicle())->Damage.SetEngineStatus(0);
			for (int32 i = 0; i < 4; i++)
				((CAutomobile*)FindPlayerVehicle())->Damage.SetWheelStatus(i, WHEEL_STATUS_OK);
		}
	}
}

// TODO(Miami): this is HELLA different on mobile, although it mostly has debug oriented things like player exiting it's current car and enters spawned one etc.
void VehicleCheat(int model)
{
	CHud::SetHelpMessage(TheText.Get("CHEAT1"), true);
	CStreaming::RequestModel(model, STREAMFLAGS_DONT_REMOVE);
	CStreaming::LoadAllRequestedModels(false);
	if (CStreaming::ms_aInfoForModel[model].m_loadState == STREAMSTATE_LOADED) {
		CHud::SetHelpMessage(TheText.Get("CHEAT1"), true);

		if (!(CStreaming::ms_aInfoForModel[model].m_loadState & STREAMFLAGS_DONT_REMOVE)) {
			CStreaming::SetModelIsDeletable(model);
			CStreaming::SetModelTxdIsDeletable(model);
		}

		int32 node = ThePaths.FindNodeClosestToCoors(FindPlayerCoors(), PATH_CAR, 100.0f);
		if (node < 0) return;

#ifdef FIX_BUGS
		CAutomobile* vehicle = new CAutomobile(model, RANDOM_VEHICLE);
#else
		CAutomobile* vehicle = new CAutomobile(model, MISSION_VEHICLE);
#endif
		if (vehicle != nil) {
			CVector pos = ThePaths.m_pathNodes[node].GetPosition();
			pos.z += 4.0f;
			vehicle->SetPosition(pos);
			vehicle->SetOrientation(0.0f, 0.0f, DEGTORAD(200.0f));

			vehicle->SetStatus(STATUS_ABANDONED);
			vehicle->m_nDoorLock = CARLOCK_UNLOCKED;
			CWorld::Add(vehicle);
		}
	}
	CStats::CheatedCount += 1000;
	CPad::bHasPlayerCheated = true;
}


void BlowUpCarsCheat()
{
	CHud::SetHelpMessage(TheText.Get("CHEAT1"), true);

	int i = CPools::GetVehiclePool()->GetSize();
	while (i-- > 0) {
		if (CVehicle *veh = CPools::GetVehiclePool()->GetSlot(i))
			veh->BlowUpCar(nil);
	}
}

void ChangePlayerCheat()
{
	// I don't know wtf is going on in here...
	int modelId, anotherModelId;

	if (FindPlayerPed()->IsPedInControl() && CModelInfo::GetModelInfo("player", nil)) {
		CHud::SetHelpMessage(TheText.Get("CHEAT1"), true);
		CPlayerPed *ped = FindPlayerPed();
		AssocGroupId AnimGrp = ped->m_animGroup;
		do
		{
			do {
				modelId = CGeneral::GetRandomNumberInRange(0, MI_PGA);
				anotherModelId = modelId+1;
			} while (!CModelInfo::GetModelInfo(anotherModelId));
		} while (anotherModelId >= MI_SPECIAL01 && anotherModelId <= MI_SPECIAL04 || modelId == MI_TAXI_D);

		uint8 flags = CStreaming::ms_aInfoForModel[anotherModelId].m_flags;
		ped->DeleteRwObject();
		CStreaming::RequestModel(anotherModelId, STREAMFLAGS_DEPENDENCY| STREAMFLAGS_DONT_REMOVE);
		CStreaming::LoadAllRequestedModels(false);
		ped->m_modelIndex = -1;
		ped->SetModelIndex(anotherModelId);
		ped->m_animGroup = AnimGrp;
		if (modelId != -1) {
			if (!(flags & STREAMFLAGS_DONT_REMOVE))
				CStreaming::SetModelIsDeletable(anotherModelId);
		}
	}
}

void ChangePlayerModel(const char* name) {
	if (!FindPlayerVehicle()) {
		FindPlayerPed()->Undress(name);
		CStreaming::LoadAllRequestedModels(0);
		FindPlayerPed()->Dress();
	}
}

void MayhemCheat()
{
	CHud::SetHelpMessage(TheText.Get("CHEAT1"), true);
	for (int i = PEDTYPE_CIVMALE; i < PEDTYPE_SPECIAL; i++)
		CPedType::SetThreats(i, PED_FLAG_PLAYER1 | PED_FLAG_PLAYER2 | PED_FLAG_PLAYER3 | PED_FLAG_PLAYER4 |
			PED_FLAG_CIVMALE | PED_FLAG_CIVFEMALE | PED_FLAG_COP | PED_FLAG_GANG1 |
			PED_FLAG_GANG2 | PED_FLAG_GANG3 | PED_FLAG_GANG4 | PED_FLAG_GANG5 |
			PED_FLAG_GANG6 | PED_FLAG_GANG7 | PED_FLAG_GANG8 | PED_FLAG_GANG9 |
			PED_FLAG_EMERGENCY | PED_FLAG_PROSTITUTE | PED_FLAG_CRIMINAL | PED_FLAG_SPECIAL );
	CStats::CheatedCount += 1000;
	CPad::bHasPlayerCheated = true;
}

void EverybodyAttacksPlayerCheat()
{
	CHud::SetHelpMessage(TheText.Get("CHEAT1"), true);
	for (int i = PEDTYPE_CIVMALE; i < PEDTYPE_SPECIAL; i++)
		CPedType::AddThreat(i, PED_FLAG_PLAYER1);

	CStats::CheatedCount += 1000;
	CPad::bHasPlayerCheated = true;
}

void WeaponsForAllCheat()
{
	CHud::SetHelpMessage(TheText.Get("CHEAT1"), true);
	CPopulation::ms_bGivePedsWeapons = !CPopulation::ms_bGivePedsWeapons;
	CStats::CheatedCount += 1000;
	CPad::bHasPlayerCheated = true;
}

void FastTimeCheat()
{
	CHud::SetHelpMessage(TheText.Get("CHEAT1"), true);
	if (CTimer::GetTimeScale() < 4.0f)
		CTimer::SetTimeScale(CTimer::GetTimeScale() * 2.0f);
}

void SlowTimeCheat()
{
	CHud::SetHelpMessage(TheText.Get("CHEAT1"), true);
	if (CTimer::GetTimeScale() > 0.25f)
		CTimer::SetTimeScale(CTimer::GetTimeScale() * 0.5f);
}

void MoneyCheat()
{
	CWorld::Players[CWorld::PlayerInFocus].m_nMoney += 250000;
	CHud::SetHelpMessage(TheText.Get("CHEAT6"), true);
}

void ArmourCheat()
{
	CHud::SetHelpMessage(TheText.Get("CHEAT4"), true);
	FindPlayerPed()->m_fArmour = CWorld::Players[0].m_nMaxArmour;
}

void WantedLevelUpCheat()
{
	CHud::SetHelpMessage(TheText.Get("CHEAT5"), true);
	FindPlayerPed()->m_pWanted->CheatWantedLevel(Min(FindPlayerPed()->m_pWanted->m_nWantedLevel + 2, 6));
}

void WantedLevelDownCheat()
{
	CHud::SetHelpMessage(TheText.Get("CHEAT5"), true);
	FindPlayerPed()->m_pWanted->CheatWantedLevel(0);
}

void SunnyWeatherCheat()
{
	CHud::SetHelpMessage(TheText.Get("CHEAT7"), true);
	CWeather::ForceWeatherNow(WEATHER_SUNNY);
}

void ExtraSunnyWeatherCheat()
{
	CHud::SetHelpMessage(TheText.Get("CHEAT7"), true);
	CWeather::ForceWeatherNow(WEATHER_EXTRA_SUNNY);
}

void CloudyWeatherCheat()
{
	CHud::SetHelpMessage(TheText.Get("CHEAT7"), true);
	CWeather::ForceWeatherNow(WEATHER_CLOUDY);
}

void RainyWeatherCheat()
{
	CHud::SetHelpMessage(TheText.Get("CHEAT7"), true);
	CWeather::ForceWeatherNow(WEATHER_RAINY);
}

void FoggyWeatherCheat()
{
	CHud::SetHelpMessage(TheText.Get("CHEAT7"), true);
	CWeather::ForceWeatherNow(WEATHER_FOGGY);
}

void FastWeatherCheat()
{
	CHud::SetHelpMessage(TheText.Get("CHEAT1"), true);
	gbFastTime = !gbFastTime;
}

void OnlyRenderWheelsCheat()
{
	CHud::SetHelpMessage(TheText.Get("CHEAT1"), true);
	CVehicle::bWheelsOnlyCheat = !CVehicle::bWheelsOnlyCheat;
	CStats::CheatedCount += 1000;
	CPad::bHasPlayerCheated = true;
}

void ChittyChittyBangBangCheat()
{
#ifdef BETTER_ALLCARSAREDODO_CHEAT
	CHud::SetHelpMessage(TheText.Get(!CVehicle::bAllDodosCheat ? "CHEAT1" : "CHEATOF"), true);
#else
	CHud::SetHelpMessage(TheText.Get("CHEAT1"), true);
#endif
	CVehicle::bAllDodosCheat = !CVehicle::bAllDodosCheat;
	CStats::CheatedCount += 1000;
	CPad::bHasPlayerCheated = true;
}

void StrongGripCheat()
{
	CHud::SetHelpMessage(TheText.Get("CHEAT1"), true);
	CVehicle::bCheat3 = !CVehicle::bCheat3;
	CStats::CheatedCount += 1000;
	CPad::bHasPlayerCheated = true;
}

void FannyMagnetCheat()
{
	CHud::SetHelpMessage(TheText.Get("CHEAT1"), true);
	CPed::bFannyMagnetCheat = !CPed::bFannyMagnetCheat;
	CPad::bHasPlayerCheated = true;
}

void BlackCarsCheat()
{
	CHud::SetHelpMessage(TheText.Get("CHEAT1"), true);
	gbBlackCars = true;
	gbPinkCars = false;
}

void PinkCarsCheat()
{
	CHud::SetHelpMessage(TheText.Get("CHEAT1"), true);
	gbBlackCars = false;
	gbPinkCars = true;
}

void TrafficLightsCheat()
{
	CHud::SetHelpMessage(TheText.Get("CHEAT1"), true);
	CTrafficLights::bGreenLightsCheat = true;
}

void MadCarsCheat()
{
	CHud::SetHelpMessage(TheText.Get("CHEAT1"), true);
	CCarCtrl::bMadDriversCheat = true;
}

void NoSeaBedCheat(void)
{
	CHud::SetHelpMessage(TheText.Get("CHEAT1"), true);
	CWaterLevel::m_bRenderSeaBed = !CWaterLevel::m_bRenderSeaBed;
}

void RenderWaterLayersCheat(void)
{
	CHud::SetHelpMessage(TheText.Get("CHEAT1"), true);
	if ( ++CWaterLevel::m_nRenderWaterLayers > 5 )
			CWaterLevel::m_nRenderWaterLayers = 0;
}

void BackToTheFuture(void)
{
	CHud::SetHelpMessage(TheText.Get("CHEAT1"), true);
	CVehicle::bHoverCheat = !CVehicle::bHoverCheat;
	CPad::bHasPlayerCheated = true;
}

void SuicideCheat(void) {
	CHud::SetHelpMessage(TheText.Get("CHEAT1"), true);
	FindPlayerPed()->InflictDamage(nil, WEAPONTYPE_UNARMED, 1000.0f, PEDPIECE_TORSO, 0);
}

void DoChicksWithGunsCheat(void) {
	CHud::SetHelpMessage(TheText.Get("CHEAT1"), true);
	CStreaming::SetModelIsDeletable(CGangs::GetGangPedModel1(GANG_PLAYER));
	CStreaming::SetModelIsDeletable(CGangs::GetGangPedModel2(GANG_PLAYER));
	CStreaming::SetModelTxdIsDeletable(CGangs::GetGangPedModel1(GANG_PLAYER));
	CStreaming::SetModelTxdIsDeletable(CGangs::GetGangPedModel2(GANG_PLAYER));
	CStreaming::RemoveCurrentZonesModels();
	CGangs::SetGangPedModels(GANG_PLAYER, MI_HFYBE, MI_WFYBE);
	CGangs::SetGangWeapons(GANG_PLAYER, WEAPONTYPE_M4, WEAPONTYPE_M4);
	CStats::CheatedCount += 1000;
	CPad::bHasPlayerCheated = true;
}

//////////////////////////////////////////////////////////////////////////

#ifdef KANGAROO_CHEAT
void KangarooCheat()
{
	wchar *string;
	CPed *playerPed = FindPlayerPed();
	int m_fMass;

	if (playerPed->m_ped_flagI80) {
		string = TheText.Get("CHEATOF");
		m_fMass = 70.0f;
	} else {
		string = TheText.Get("CHEAT1");
		m_fMass = 15.0f;
	}
	CHud::SetHelpMessage(string, true);
	playerPed->m_ped_flagI80 = !playerPed->m_ped_flagI80;

	playerPed->m_fMass = m_fMass;
	playerPed->m_fAirResistance = 0.4f / m_fMass;
}
#endif

#ifdef RESTORE_ALLCARSHELI_CHEAT
void AllCarsHeliCheat(void)
{
	wchar* string;
	if (bAllCarCheat) {
		string = TheText.Get("CHEATOF");
		bAllCarCheat = false;
	}
	else {
		string = TheText.Get("CHEAT1");
		bAllCarCheat = true;
	}
	CHud::SetHelpMessage(string, true);
}
#endif

#ifdef WALLCLIMB_CHEAT
void WallClimbingCheat(void)
{
	wchar* string;
	if (gGravityCheat) {
		string = TheText.Get("CHEATOF");
		gGravityCheat = false;
	}
	else {
		string = TheText.Get("CHEAT1");
		gGravityCheat = true;
	}
	CHud::SetHelpMessage(string, true);
}
#endif

void FlyingFishCheat(void)
{
	CHud::SetHelpMessage(TheText.Get("CHEAT1"), true);
	CVehicle::bCheat8 = !CVehicle::bCheat8;
	CPad::bHasPlayerCheated = true;
}

void DoShowChaseStatCheat(void) {
	CHud::SetHelpMessage(TheText.Get("CHEAT1"), true);
	CStats::ShowChaseStatOnScreen = 1;
}

bool
CControllerState::CheckForInput(void)
{
	return !!RightStickX || !!RightStickY || !!LeftStickX || !!LeftStickY
		|| !!DPadUp || !!DPadDown || !!DPadLeft || !!DPadRight
		|| !!Triangle || !!Cross || !!Circle || !!Square
		|| !!Start || !!Select
		|| !!LeftShoulder1 || !!LeftShoulder2 || !!RightShoulder1 || !!RightShoulder2
		|| !!LeftShock || !!RightShock;
}

void
CControllerState::Clear(void)
{
	LeftStickX = LeftStickY = RightStickX = RightStickY = 0;
	LeftShoulder1 = LeftShoulder2 = RightShoulder1 = RightShoulder2 = 0;
	DPadUp = DPadDown = DPadLeft = DPadRight = 0;
	Start = Select = 0;
	Square = Triangle = Cross = Circle = 0;
	LeftShock = RightShock = 0;
	NetworkTalk = 0;
}

void CKeyboardState::Clear()
{
	for ( int32 i = 0; i < ARRAY_SIZE(F); i++	)
		F[i] = 0;

	for ( int32 i = 0; i < ARRAY_SIZE(VK_KEYS); i++	 )
		VK_KEYS[i] = 0;

	ESC = INS = DEL = HOME = END = PGUP = PGDN = 0;

	UP = DOWN = LEFT = RIGHT = 0;

	NUMLOCK = 0;

	DIV = MUL = SUB = ADD = 0;

	DECIMAL = NUM1 = NUM2 = NUM3 = NUM4 = 0;

	NUM5 = NUM6 = NUM7 = NUM8 = 0;

	NUM9 = NUM0 = SCROLLLOCK = PAUSE = 0;

	BACKSP = TAB = CAPSLOCK = EXTENTER = 0;

	LSHIFT = SHIFT = RSHIFT = LCTRL = RCTRL = LALT = RALT = 0;

	LWIN = RWIN = APPS = 0;
}

#ifdef GTA_PS2_STUFF
void CPad::Initialise(void)
{
#ifdef GTA_PS2
	scePadInit(0);

	scePadPortOpen(0, 0, pad_dma_buf );
	scePadPortOpen(1, 0, pad2_dma_buf );
#endif
	for (int i = 0; i < MAX_PADS; i++)
	{
		CPad::GetPad(i)->Clear(true);
		CPad::GetPad(i)->Mode = 0;
	}

	bObsoleteControllerMessage	   = false;
	bOldDisplayNoControllerMessage = false;
	bDisplayNoControllerMessage	   = false;
	m_bMapPadOneToPadTwo = false;
	m_bDebugCamPCOn = false;
}
#endif

void CPad::Clear(bool bResetPlayerControls)
{
	NewState.Clear();
	OldState.Clear();

	PCTempKeyState.Clear();
	PCTempJoyState.Clear();
	PCTempMouseState.Clear();

	NewKeyState.Clear();
	OldKeyState.Clear();
	TempKeyState.Clear();

	NewMouseControllerState.Clear();
	OldMouseControllerState.Clear();
	PCTempMouseControllerState.Clear();

	Phase = 0;
	ShakeFreq = 0;
	ShakeDur = 0;

	for (int32 i = 0; i < DRUNK_STEERING_BUFFER_SIZE; i++)
		SteeringLeftRightBuffer[i] = 0;

	DrunkDrivingBufferUsed = 0;

	if ( bResetPlayerControls )
		DisablePlayerControls = PLAYERCONTROL_ENABLED;

	JustOutOfFrontend = 0;
	bApplyBrakes = false;

	for ( int32 i = 0; i < HORNHISTORY_SIZE; i++ )
		bHornHistory[i] = false;

	iCurrHornHistory = 0;

	for ( int32 i = 0; i < ARRAY_SIZE(CheatString); i++ )
		CheatString[i] = ' ';

	LastTimeTouched = CTimer::GetTimeInMilliseconds();
	AverageWeapon = 0;
	AverageEntries = 0;
}

uint32 CPad::InputHowLongAgo()
{
	return CTimer::GetTimeInMilliseconds() - LastTimeTouched;
}

void CPad::ClearMouseHistory()
{
	PCTempMouseControllerState.Clear();
	NewMouseControllerState.Clear();
	OldMouseControllerState.Clear();
}

// unused
void CPad::ClearKeyBoardHistory()
{
	NewKeyState.Clear();
	OldKeyState.Clear();
	TempKeyState.Clear();
}

CMouseControllerState::CMouseControllerState()
{
	LMB = 0;
	RMB = 0;
	MMB = 0;
	WHEELUP = 0;
	WHEELDN = 0;
	MXB1 = 0;
	MXB2 = 0;

	x = 0.0f;
	y = 0.0f;
}

void CMouseControllerState::Clear()
{
	LMB = 0;
	RMB = 0;
	MMB = 0;
	WHEELUP = 0;
	WHEELDN = 0;
	MXB1 = 0;
	MXB2 = 0;
}

CMouseControllerState CMousePointerStateHelper::GetMouseSetUp()
{
	CMouseControllerState state;

#if defined RW_D3D9 || defined RWLIBS
	if ( PSGLOBAL(mouse) == nil )
		_InputInitialiseMouse(!FrontEndMenuManager.m_bMenuActive && _InputMouseNeedsExclusive());

	if ( PSGLOBAL(mouse) != nil )
	{
		DIDEVCAPS devCaps;
		devCaps.dwSize = sizeof(DIDEVCAPS);

		PSGLOBAL(mouse)->GetCapabilities(&devCaps);
		switch ( devCaps.dwButtons )
		{
			case 3:
			case 4:
			case 5:
			case 6:
			case 7:
			case 8:
				state.MMB = true;

			case 2:
				state.RMB = true;

			case 1:
				state.LMB = true;
		}

		if ( devCaps.dwAxes == 3 )
		{
			state.WHEELDN = true;
			state.WHEELUP = true;
		}
	}
#else
	// It seems there is no way to get number of buttons on mouse, so assign all buttons if we have mouse.
	double xpos = 1.0f, ypos;
	glfwGetCursorPos(PSGLOBAL(window), &xpos, &ypos);

	if (xpos != 0.f) {
		state.MMB = true;
		state.RMB = true;
		state.LMB = true;
		state.WHEELDN = true;
		state.WHEELUP = true;
	}
#endif

	return state;
}

void CPad::UpdateMouse()
{
#if defined RW_D3D9 || defined RWLIBS
	if ( IsForegroundApp() )
	{
		if ( PSGLOBAL(mouse) == nil )
			_InputInitialiseMouse(!FrontEndMenuManager.m_bMenuActive && _InputMouseNeedsExclusive());

		DIMOUSESTATE2 state;

		if ( PSGLOBAL(mouse) != nil && SUCCEEDED(_InputGetMouseState(&state)) )
		{
			int32 signX = 1;
			int32 signy = 1;

			if ( !FrontEndMenuManager.m_bMenuActive )
			{
				if ( MousePointerStateHelper.bInvertVertically )
					signy = -1;
				if ( MousePointerStateHelper.bInvertHorizontally )
					signX = -1;
			}

			PCTempMouseControllerState.Clear();

			PCTempMouseControllerState.x = (float)(signX * state.lX);
			PCTempMouseControllerState.y = (float)(signy * state.lY);
			PCTempMouseControllerState.LMB = state.rgbButtons[0] & 128;
			PCTempMouseControllerState.RMB = state.rgbButtons[1] & 128;
			PCTempMouseControllerState.MMB = state.rgbButtons[2] & 128;
			PCTempMouseControllerState.MXB1 = state.rgbButtons[3] & 128;
			PCTempMouseControllerState.MXB2 = state.rgbButtons[4] & 128;

			if ( state.lZ > 0 )
				PCTempMouseControllerState.WHEELUP = 1;
			else if ( state.lZ < 0 )
				PCTempMouseControllerState.WHEELDN = 1;

			OldMouseControllerState = NewMouseControllerState;
			NewMouseControllerState = PCTempMouseControllerState;
		}
	}
#else
	if ( IsForegroundApp() && PSGLOBAL(cursorIsInWindow) )
	{
		double xpos = 1.0f, ypos;
		glfwGetCursorPos(PSGLOBAL(window), &xpos, &ypos);
		if (xpos == 0.f)
			return;

		int32 signX = 1;
		int32 signy = 1;

		if (!FrontEndMenuManager.m_bMenuActive)
		{
			if (MousePointerStateHelper.bInvertVertically)
				signy = -1;
			if (MousePointerStateHelper.bInvertHorizontally)
				signX = -1;
		}

		PCTempMouseControllerState.Clear();

		PCTempMouseControllerState.x = (float)(signX * (xpos - PSGLOBAL(lastMousePos.x)));
		PCTempMouseControllerState.y = (float)(signy * (ypos - PSGLOBAL(lastMousePos.y)));
		PCTempMouseControllerState.LMB = glfwGetMouseButton(PSGLOBAL(window), GLFW_MOUSE_BUTTON_LEFT);
		PCTempMouseControllerState.RMB = glfwGetMouseButton(PSGLOBAL(window), GLFW_MOUSE_BUTTON_RIGHT);
		PCTempMouseControllerState.MMB = glfwGetMouseButton(PSGLOBAL(window), GLFW_MOUSE_BUTTON_MIDDLE);
		PCTempMouseControllerState.MXB1 = glfwGetMouseButton(PSGLOBAL(window), GLFW_MOUSE_BUTTON_4);
		PCTempMouseControllerState.MXB2 = glfwGetMouseButton(PSGLOBAL(window), GLFW_MOUSE_BUTTON_5);

		if (PSGLOBAL(mouseWheel) > 0)
			PCTempMouseControllerState.WHEELUP = 1;
		else if (PSGLOBAL(mouseWheel) < 0)
			PCTempMouseControllerState.WHEELDN = 1;

		PSGLOBAL(lastMousePos.x) = xpos;
		PSGLOBAL(lastMousePos.y) = ypos;
		PSGLOBAL(mouseWheel) = 0.0f;

		OldMouseControllerState = NewMouseControllerState;
		NewMouseControllerState = PCTempMouseControllerState;
	}
#endif
}

CControllerState CPad::ReconcileTwoControllersInput(CControllerState const &State1, CControllerState const &State2)
{
	static CControllerState ReconState;

	ReconState.Clear();

#define _RECONCILE_BUTTON(button) \
	{ if ( State1.button || State2.button ) ReconState.button = 255; }

#define _RECONCILE_AXIS_POSITIVE(axis) \
	{ if ( State1.axis >= 0 && State2.axis >= 0 ) ReconState.axis = Max(State1.axis, State2.axis); }

#define _RECONCILE_AXIS_NEGATIVE(axis) \
	{ if ( State1.axis <= 0 && State2.axis <= 0 ) ReconState.axis = Min(State1.axis, State2.axis); }

#define _RECONCILE_AXIS(axis) \
	{ _RECONCILE_AXIS_POSITIVE(axis); _RECONCILE_AXIS_NEGATIVE(axis); }

#define _FIX_AXIS_DIR(axis) \
	{ if ( State1.axis > 0 && State2.axis < 0 || State1.axis < 0 && State2.axis > 0 ) ReconState.axis = 0; }

#define _FIX_RECON_DIR(pos, neg, axis) \
	{ if ( (ReconState.pos || ReconState.axis < 0) && (ReconState.neg || ReconState.axis > 0) ) { ReconState.pos = 0; ReconState.neg = 0; ReconState.axis = 0; } }

	_RECONCILE_BUTTON(LeftShoulder1);
	_RECONCILE_BUTTON(LeftShoulder2);
	_RECONCILE_BUTTON(RightShoulder1);
	_RECONCILE_BUTTON(RightShoulder2);
	_RECONCILE_BUTTON(Start);
	_RECONCILE_BUTTON(Select);
	_RECONCILE_BUTTON(Square);
	_RECONCILE_BUTTON(Triangle);
	_RECONCILE_BUTTON(Cross);
	_RECONCILE_BUTTON(Circle);
	_RECONCILE_BUTTON(LeftShock);
	_RECONCILE_BUTTON(RightShock);
	_RECONCILE_BUTTON(NetworkTalk);
	_RECONCILE_AXIS(LeftStickX);
	_RECONCILE_AXIS(LeftStickY);
	_FIX_AXIS_DIR(LeftStickX);
	_FIX_AXIS_DIR(LeftStickY);
	_RECONCILE_AXIS(RightStickX);
	_RECONCILE_AXIS(RightStickY);
	_FIX_AXIS_DIR(RightStickX);
	_FIX_AXIS_DIR(RightStickY);
	_RECONCILE_BUTTON(DPadUp);
	_RECONCILE_BUTTON(DPadDown);
	_RECONCILE_BUTTON(DPadLeft);
	_RECONCILE_BUTTON(DPadRight);
	_FIX_RECON_DIR(DPadUp, DPadDown, LeftStickY);
	_FIX_RECON_DIR(DPadLeft, DPadRight, LeftStickX);

	return ReconState;

#undef _RECONCILE_BUTTON
#undef _RECONCILE_AXIS_POSITIVE
#undef _RECONCILE_AXIS_NEGATIVE
#undef _RECONCILE_AXIS
#undef _FIX_AXIS_DIR
#undef _FIX_RECON_DIR
}

void CPad::StartShake(int16 nDur, uint8 nFreq)
{
	if ( !FrontEndMenuManager.m_PrefsUseVibration )
		return;

	if ( CCutsceneMgr::IsRunning() || CGame::playingIntro )
		return;

	if ( nFreq == 0 )
	{
		ShakeDur = 0;
		ShakeFreq = 0;
		return;
	}

	if ( nDur > ShakeDur )
	{
		ShakeDur = nDur;
		ShakeFreq = nFreq;
	}
}

void CPad::StartShake_Distance(int16 nDur, uint8 nFreq, float fX, float fY, float fZ)
{
	if ( !FrontEndMenuManager.m_PrefsUseVibration )
		return;

	if ( CCutsceneMgr::IsRunning() || CGame::playingIntro )
		return;

	float fDist = ( TheCamera.GetPosition() - CVector(fX, fY, fZ) ).Magnitude();

	if ( fDist < 70.0f )
	{
		if ( nFreq == 0 )
		{
			ShakeDur = 0;
			ShakeFreq = 0;
			return;
		}

		if ( nDur > ShakeDur )
		{
			ShakeDur = nDur;
			ShakeFreq = nFreq;
		}
	}
}

void CPad::StartShake_Train(float fX, float fY)
{
	if ( !FrontEndMenuManager.m_PrefsUseVibration )
		return;

	if ( CCutsceneMgr::IsRunning() || CGame::playingIntro )
		return;

	if (FindPlayerVehicle() != nil && FindPlayerVehicle()->IsTrain() )
		return;

	float fDist = ( TheCamera.GetPosition() - CVector(fX, fY, 0.0f) ).Magnitude2D();

	if ( fDist < 70.0f )
	{
		int32 freq = (int32)((70.0f - fDist) * 70.0f / 70.0f + 30.0f);

		if ( ShakeDur < 100 )
		{
			ShakeDur = 100;
			ShakeFreq = freq;
		}
	}
}

#ifdef GTA_PS2_STUFF
void CPad::AddToCheatString(char c)
{
	for ( int32 i = ARRAY_SIZE(CheatString) - 2; i >= 0; i-- )
		CheatString[i + 1] = CheatString[i];

	CheatString[0] = c;

#define _CHEATCMP(str)	strncmp(str, CheatString, sizeof(str)-1)
	// "4414LDRULDRU"	-	R2 R2 L1 R2 LEFT DOWN RIGHT UP LEFT DOWN RIGHT UP
	if ( !_CHEATCMP("URDLURDL4144") )
		WeaponCheat1();

	// "4411LDRULDRU"	-	R2 R2 L1 L1 LEFT DOWN RIGHT UP LEFT DOWN RIGHT UP
	else if ( !_CHEATCMP("URDLURDL1144") )
		MoneyCheat();

	// "4412LDRULDRU"	-	R2 R2 L1 L2 LEFT DOWN RIGHT UP LEFT DOWN RIGHT UP
	else if ( !_CHEATCMP("URDLURDL2144") )
		ArmourCheat();

	// "4413LDRULDRU"	-	R2 R2 L1 R1 LEFT DOWN RIGHT UP LEFT DOWN RIGHT UP
	else if ( !_CHEATCMP("URDLURDL3144") )
		HealthCheat();

	// "4414LRLRLR"		-	R2 R2 L1 R2 LEFT RIGHT LEFT RIGHT LEFT RIGHT
	else if ( !_CHEATCMP("RLRLRL4144") )
		WantedLevelUpCheat();

	// "4414UDUDUD"		-	R2 R2 L1 R2 UP DOWN UP DOWN UP DOWN
	else if ( !_CHEATCMP("DUDUDU4144") )
		WantedLevelDownCheat();

	// "1234432T"		-	L1 L2 R1 R2 R2 R1 L2 TRIANGLE
	else if ( !_CHEATCMP("T2344321") )
		SunnyWeatherCheat();

	// "1234432S"		-	L1 L2 R1 R2 R2 R1 L2 SQUARE
	else if ( !_CHEATCMP("S2344321") )
		CloudyWeatherCheat();

	// "1234432C"		-	L1 L2 R1 R2 R2 R1 L2 CIRCLE
	else if ( !_CHEATCMP("C2344321") )
		RainyWeatherCheat();

	// "1234432X"		-	L1 L2 R1 R2 R2 R1 L2 CROSS
	else if ( !_CHEATCMP("X2344321") )
		FoggyWeatherCheat();

	// "CCCCCC321TCT"	-	CIRCLE CIRCLE CIRCLE CIRCLE CIRCLE CIRCLE R1 L2 L1 TRIANGLE CIRCLE TRIANGLE
	else if ( !_CHEATCMP("TCT123CCCCCC") )
		VehicleCheat(MI_RHINO);

	// "CCCSSSSS1TCT"	-	CIRCLE CIRCLE CIRCLE SQUARE SQUARE SQUARE SQUARE SQUARE L1 TRIANGLE CIRCLE TRIANGLE
	else if ( !_CHEATCMP("TCT1SSSSSCCC") )
		FastWeatherCheat();

	// "241324TSCT21"	-	L2 R2 L1 R1 L2 R2 TRIANGLE SQUARE CIRCLE TRIANGLE L2 L1
	else if ( !_CHEATCMP("12TCST423142") )
		BlowUpCarsCheat();

	// "RDLU12ULDR"		-	RIGHT DOWN LEFT UP L1 L2 UP LEFT DOWN RIGHT
	else if ( !_CHEATCMP("RDLU21ULDR") )
		ChangePlayerCheat();

	// "DULUX3421"		-	DOWN UP LEFT UP CROSS R1 R2 L2 L1
	else if ( !_CHEATCMP("1243XULUD") )
		MayhemCheat();

	// "DULUX3412"		-	DOWN UP LEFT UP CROSS R1 R2 L1 L2
	else if ( !_CHEATCMP("2143XULUD") )
		EverybodyAttacksPlayerCheat();

	// "43TX21UD"		-	R2 R1 TRIANGLE CROSS L2 L1 UP DOWN
	else if ( !_CHEATCMP("DU12XT34") )
		WeaponsForAllCheat();

	// "TURDS12"		-	TRIANGLE UP RIGHT DOWN SQUARE L1 L2
	else if ( !_CHEATCMP("21SDRUT") )
		FastTimeCheat();

	// "TURDS34"		-	TRIANGLE UP RIGHT DOWN SQUARE R1 R2
	else if ( !_CHEATCMP("43SDRUT") )
		SlowTimeCheat();

	// "11S4T1T"		-	L1 L1 SQUARE R2 TRIANGLE L1 TRIANGLE
	else if ( !_CHEATCMP("T1T4S11") )
		OnlyRenderWheelsCheat();

	// "R4C32D13"		-	RIGHT R2 CIRCLE R1 L2 DOWN L1 R1
	else if ( !_CHEATCMP("31D23C4R") )
		ChittyChittyBangBangCheat();

	// "3141L33T"		-	R1 L1 R2 L1 LEFT R1 R1 TRIANGLE
	else if ( !_CHEATCMP("T33L1413") )
		StrongGripCheat();

#undef _CHEATCMP
}
#endif

int Cheat_strncmp(char* sourceStr, char* origCheatStr)
{
#define ccmp(n) if((uint8)sourceStr[i] != (uint8)origCheatStr[i] - n) return 1;
	int i = 0;
	while(origCheatStr[i])
	{
		switch(i)
		{
		case 0: ccmp(3); break;
		case 1: ccmp(5); break;
		case 2: ccmp(7); break;
		case 3: ccmp(1); break;
		case 4: ccmp(13); break;
		case 5: ccmp(27); break;
		case 6: ccmp(3); break;
		case 7: ccmp(7); break;
		case 8: ccmp(1); break;
		case 9: ccmp(11); break;
		case 10: ccmp(13); break;
		case 11: ccmp(8); break;
		case 12: ccmp(7); break;
		case 13: ccmp(32); break;
		case 14: ccmp(13); break;
		case 15: ccmp(6); break;
		case 16: ccmp(28); break;
		case 17: ccmp(19); break;
		case 18: ccmp(10); break;
		case 19: ccmp(3); break;
		case 20: ccmp(3); break;
		case 21: ccmp(5); break;
		case 22: ccmp(7); break;
		case 23: ccmp(1); break;
		case 24: ccmp(13); break;
		case 25: ccmp(27); break;
		case 26: ccmp(3); break;
		case 27: ccmp(7); break;
		default: return 1;
		}
		i++;
	}
	return 0;
#undef ccmp
}

// TODO(Miami): Mobile has changed some of the cheats to include debugging things
void CPad::AddToPCCheatString(char c)
{
	for (int32 i = ARRAY_SIZE(KeyBoardCheatString) - 2; i >= 0; i--)
		KeyBoardCheatString[i + 1] = KeyBoardCheatString[i];

	KeyBoardCheatString[0] = c;

#define _CHEATCMP(str) strncmp(str, KeyBoardCheatString, sizeof(str)-1)

	// "THUGSTOOLS"
	if (!Cheat_strncmp(KeyBoardCheatString, "VQVPanJ\\I_")) {
		KeyBoardCheatString[0] = ' ';
		WeaponCheat1();
	}
	// "PROFESSIONALTOOLS"
	else if (!Cheat_strncmp(KeyBoardCheatString, "VQVPagDUPT`[Lf\\Xl")) {
		KeyBoardCheatString[0] = ' ';
		WeaponCheat2();
	}
	// "NUTTERTOOLS"
	else if (!Cheat_strncmp(KeyBoardCheatString, "VQVPamH[U`[")) {
		KeyBoardCheatString[0] = ' ';
		WeaponCheat3();
	}
	// "PRECIOUSPROTECTION"
	else if (!Cheat_strncmp(KeyBoardCheatString, "QTPUP`WVS[`]ViPKnc")) {
		KeyBoardCheatString[0] = ' ';
		ArmourCheat();
	}
	// "ASPIRINE"
	else if (!Cheat_strncmp(KeyBoardCheatString, "HSPSVkVH")) {
		KeyBoardCheatString[0] = ' ';
		HealthCheat();
	}
	// "YOUWONTTAKEMEALIVE"
	else if (!Cheat_strncmp(KeyBoardCheatString, "H[PMN`PLLLa\\Uod[kl")) {
		KeyBoardCheatString[0] = ' ';
		WantedLevelUpCheat();
	}
	// "LEAVEMEALONE"
	else if (!Cheat_strncmp(KeyBoardCheatString, "HSVMN`PLWLRT")) {
		KeyBoardCheatString[0] = ' ';
		WantedLevelDownCheat();
	}
	// "APLEASANTDAY"
	else if (!Cheat_strncmp(KeyBoardCheatString, "\\FKU[\\VHFW]I")) {
		KeyBoardCheatString[0] = ' ';
		SunnyWeatherCheat();
	}
	// "ALOVELYDAY"
	else if (!Cheat_strncmp(KeyBoardCheatString, "\\FKZY`YVML")) {
		KeyBoardCheatString[0] = ' ';
		ExtraSunnyWeatherCheat();
	}
	// "ABITDRIEG"
	else if (!Cheat_strncmp(KeyBoardCheatString, "JJPSQoLIB")) {
		KeyBoardCheatString[0] = ' ';
		CloudyWeatherCheat();
	}
	// "CATSANDDOGS"
	else if (!Cheat_strncmp(KeyBoardCheatString, "VLVEQiDZULP")) {
		KeyBoardCheatString[0] = ' ';
		RainyWeatherCheat();
	}
	// "CANTSEEATHING"
	else if (!Cheat_strncmp(KeyBoardCheatString, "JSPIa\\HLT_[IJ")) {
		KeyBoardCheatString[0] = ' ';
		FoggyWeatherCheat();
	}
	// "PANZER"
	else if (!Cheat_strncmp(KeyBoardCheatString, "UJaONk")) {
		KeyBoardCheatString[0] = ' ';
		VehicleCheat(MI_RHINO);
	}
	// "LIFEISPASSINGMEBY"
	else if (!Cheat_strncmp(KeyBoardCheatString, "\\GLNTiLZTL][PeSOh")) {
		KeyBoardCheatString[0] = ' ';
		FastWeatherCheat();
	}
	// "BIGBANG"
	else if (!Cheat_strncmp(KeyBoardCheatString, "JSHCTdE")) {
		KeyBoardCheatString[0] = ' ';
		BlowUpCarsCheat();
	}
	// "STILLLIKEDRESSINGUP"
	else if (!Cheat_strncmp(KeyBoardCheatString, "SZNOVnVLSORSPlYReg]")) {
		KeyBoardCheatString[0] = ' ';
		ChangePlayerCheat();
	}
	// "FIGHTFIGHTFIGHT"
	else if (!Cheat_strncmp(KeyBoardCheatString, "WMNJSoKNJQaPNiS")) {
		KeyBoardCheatString[0] = ' ';
		MayhemCheat();
	}
	// "NOBODYLIKESME"
	else if (!Cheat_strncmp(KeyBoardCheatString, "HRZFXdO`EZOWU")) {
		KeyBoardCheatString[0] = ' ';
		EverybodyAttacksPlayerCheat();
	}
	// "OURGODGIVENRIGHTTOBEARARMS"
	else if (!Cheat_strncmp(KeyBoardCheatString, "VRYB_\\HIP_aPNi_TaiSJGTNSbj")) {
		KeyBoardCheatString[0] = ' ';
		WeaponsForAllCheat();
	}
	// "ONSPEED"
	else if (!Cheat_strncmp(KeyBoardCheatString, "GJLQ`iR")) {
		KeyBoardCheatString[0] = ' ';
		FastTimeCheat();
	}
	// "BOOOOOORING"
	else if (!Cheat_strncmp(KeyBoardCheatString, "JSPS\\jRVPZO")) {
		KeyBoardCheatString[0] = ' ';
		SlowTimeCheat();
	}
	// "WHEELSAREALLINEED"
	else if (!Cheat_strncmp(KeyBoardCheatString, "GJLOVgOHF]N[SeRNs")) {
		KeyBoardCheatString[0] = ' ';
		OnlyRenderWheelsCheat();
	}
	//COMEFLYWITHME
	else if (!Cheat_strncmp(KeyBoardCheatString, "HROUVr\\SGPZWJ")) {
		KeyBoardCheatString[0] = ' ';
		ChittyChittyBangBangCheat();
	}
	// "GRIPISEVERYTHING"
	else if (!Cheat_strncmp(KeyBoardCheatString, "JSPIatULWP`QWi_M")) {
		KeyBoardCheatString[0] = ' ';
		StrongGripCheat();
	}
	// "CHASESTAT"
	else if (!Cheat_strncmp(KeyBoardCheatString, "WF[TRnDOD")) {
		KeyBoardCheatString[0] = ' ';
		DoShowChaseStatCheat();
	}
	// "CHICKSWITHGUNS"
	else if (!Cheat_strncmp(KeyBoardCheatString, "VS\\HUoL^TVPQOc")) {
		KeyBoardCheatString[0] = ' ';
		DoChicksWithGunsCheat();
	}
	// "ICANTTAKEITANYMORE"
	else if (!Cheat_strncmp(KeyBoardCheatString, "HWVNfiD[JPXI[t[G_\\")) {
		KeyBoardCheatString[0] = ' ';
		SuicideCheat();
	}
	// "GREENLIGHT"
	else if (!Cheat_strncmp(KeyBoardCheatString, "WMNJYiHLSR")) {
		KeyBoardCheatString[0] = ' ';
		TrafficLightsCheat();
	}
	// "MIAMITRAFFIC"
	else if (!Cheat_strncmp(KeyBoardCheatString, "FNMGNmWPNLVU")) {
		KeyBoardCheatString[0] = ' ';
		MadCarsCheat();
	}
	// "AHAIRDRESSERSCAR"
	else if (!Cheat_strncmp(KeyBoardCheatString, "UFJT_`VZF]QZPaUG")) {
		KeyBoardCheatString[0] = ' ';
		PinkCarsCheat();
	}
	// "IWANTITPAINTEDBLACK"
	else if (!Cheat_strncmp(KeyBoardCheatString, "NHHMO_H[OTNX[iaT]jS")) {
		KeyBoardCheatString[0] = ' ';
		BlackCarsCheat();
	}
	// "TRAVELINSTYLE"
	else if (!Cheat_strncmp(KeyBoardCheatString, "HQ`U`iLSFaNZ[")) {
		KeyBoardCheatString[0] = ' ';
		VehicleCheat(MI_BLOODRA);
	}
	// "THELASTRIDE"
	else if (!Cheat_strncmp(KeyBoardCheatString, "HIPSanDSFSa")) {
		KeyBoardCheatString[0] = ' ';
		VehicleCheat(MI_ROMERO);
	}
	// "ROCKANDROLLCAR"
	else if (!Cheat_strncmp(KeyBoardCheatString, "UFJMYjUKOLXKVr")) {
		KeyBoardCheatString[0] = ' ';
		VehicleCheat(MI_LOVEFIST);
	}
	// "RUBBISHCAR"
	else if (!Cheat_strncmp(KeyBoardCheatString, "UFJI`dEIV]")) {
		KeyBoardCheatString[0] = ' ';
		VehicleCheat(MI_TRASH);
	}
	// "GETTHEREQUICKLY"
	else if (!Cheat_strncmp(KeyBoardCheatString, "\\QRDVpTLSPU\\[eT")) {
		KeyBoardCheatString[0] = ' ';
		VehicleCheat(MI_BLOODRB);
	}
	// "GETTHEREFAST"
	else if (!Cheat_strncmp(KeyBoardCheatString, "WXHGRmHOU_RO")) {
		KeyBoardCheatString[0] = ' ';
		VehicleCheat(MI_SABRETUR);
	}
	// "BETTERTHANWALKING"
	else if (!Cheat_strncmp(KeyBoardCheatString, "JSPLY\\ZUBSaZLtaK^")) {
		KeyBoardCheatString[0] = ' ';
		VehicleCheat(MI_CADDY);
	}
	// "GETTHEREFASTINDEED"
	else if (!Cheat_strncmp(KeyBoardCheatString, "GJLE[dWZBQfZLvRXa[^WHL")) {
		KeyBoardCheatString[0] = ' ';
		VehicleCheat(MI_HOTRINA);
	}
	// "GETTHEREAMAZINGLYFAST"
	else if (!Cheat_strncmp(KeyBoardCheatString, "WXHGfgJUJeNUHe_Kdg^HJ")) {
		KeyBoardCheatString[0] = ' ';
		VehicleCheat(MI_HOTRINB);
	}
	// LOOKLIKELANCE
	else if (!Cheat_strncmp(KeyBoardCheatString, "HHUBY`NPMV\\WS")) {
		KeyBoardCheatString[0] = ' ';
		ChangePlayerModel("igbuddy");
	}
	// IWANTBIGTITS
	else if (!Cheat_strncmp(KeyBoardCheatString, "VYPUTdE[OLdQ")) {
		KeyBoardCheatString[0] = ' ';
		ChangePlayerModel("igcandy");
	}
	// MYSONISALAWYER
	else if (!Cheat_strncmp(KeyBoardCheatString, "UJ`XNgDZJY\\[`m")) {
		KeyBoardCheatString[0] = ' ';
		ChangePlayerModel("igken");
	}
	// ILOOKLIKEHILARY
	else if (!Cheat_strncmp(KeyBoardCheatString, "\\WHMVcHRJWXWVlV")) {
		KeyBoardCheatString[0] = ' ';
		ChangePlayerModel("ighlary");
	}
	// ROCKANDROLLMAN
	else if (!Cheat_strncmp(KeyBoardCheatString, "QFTMYjUKOLXKVr")) {
		KeyBoardCheatString[0] = ' ';
		ChangePlayerModel("igjezz");
	}
	// ONEARMEDBANDIT
	else if (!Cheat_strncmp(KeyBoardCheatString, "WNKON]GLN]NMUo")) {
		KeyBoardCheatString[0] = ' ';
		ChangePlayerModel("igphil");
	}
	// IDONTHAVETHEMONEYSONNY
	else if (!Cheat_strncmp(KeyBoardCheatString, "\\SUP`tHUPXRP[ecGdgXRGN")) {
		KeyBoardCheatString[0] = ' ';
		ChangePlayerModel("igsonny");
	}
	// FOXYLITTLETHING
	else if (!Cheat_strncmp(KeyBoardCheatString, "JSPIa`O[UTYa_oS")) {
		KeyBoardCheatString[0] = ' ';
		ChangePlayerModel("igmerc");
	}
	// WELOVEOURDICK
	else if (!Cheat_strncmp(KeyBoardCheatString, "NHPE_pRLWZYM^")) {
		KeyBoardCheatString[0] = ' ';
		ChangePlayerModel("igdick");
	}
	// CHEATSHAVEBEENCRACKED
	else if (!Cheat_strncmp(KeyBoardCheatString, "GJRDNmFUFPOM]aUYpTOKF")) {
		KeyBoardCheatString[0] = ' ';
		ChangePlayerModel("igdiaz");
	}
	// DEEPFRIEDMARSBARS
	else if (!Cheat_strncmp(KeyBoardCheatString, "VWHC`mDTEPVZMpRK")) {
		KeyBoardCheatString[0] = ' ';
		gfTommyFatness = 0.26f;
	}
	// PROGRAMMER
	else if (!Cheat_strncmp(KeyBoardCheatString, "UJTNNmJVS[")) {
		KeyBoardCheatString[0] = ' ';
		gfTommyFatness = -0.3f;
	}
	// SEAWAYS
	else if (!Cheat_strncmp(KeyBoardCheatString, "V^HXN`V")) {
		KeyBoardCheatString[0] = ' ';
		BackToTheFuture();
	}
	// LOADSOFLITTLETHINGS
	else if (!Cheat_strncmp(KeyBoardCheatString, "VLUJUoHSU_VTMo`J]bV")) {
		KeyBoardCheatString[0] = ' ';
		SpecialCarCheats();
	}
	// HOPINGIRL
	else if (!Cheat_strncmp(KeyBoardCheatString, "OWPH[dSVI")) {
		KeyBoardCheatString[0] = ' ';
		PickUpChicksCheat();
	}
	//CERTAINDEATH
	else if (!Cheat_strncmp(KeyBoardCheatString, "KYHFQiLHU]RK")) {
		KeyBoardCheatString[0] = ' ';
		CSmokeTrails::CigOn = !CSmokeTrails::CigOn;
	}
	//AIRSHIP
	else if (!Cheat_strncmp(KeyBoardCheatString, "SNOT_dD")) {
		KeyBoardCheatString[0] = ' ';
		FlyingFishCheat();
	}
	//FANNYMAGNET
	else if (!Cheat_strncmp(KeyBoardCheatString, "WJUHNh\\UOLS")) {
		KeyBoardCheatString[0] = ' ';
		FannyMagnetCheat();
	}

#ifdef KANGAROO_CHEAT
	// "KANGAROO"
	if (!_CHEATCMP("OORAGNAK"))
		KangarooCheat();
#endif

#ifndef MASTER
	// "PEDDEBUG"
	if (!_CHEATCMP("GUBEDDEP"))
		CPed::SwitchDebugDisplay();
#endif

#ifdef RESTORE_ALLCARSHELI_CHEAT
	// "CARSAREHELI"
	if (!_CHEATCMP("ILEHERASRAC"))
		AllCarsHeliCheat();
#endif

#ifdef WALLCLIMB_CHEAT
	// "SPIDERCAR"
	if (!_CHEATCMP("RACREDIPS"))
		WallClimbingCheat();
#endif

#if !defined(PC_WATER) && defined(WATER_CHEATS)
	// SEABEDCHEAT
	if (!_CHEATCMP("TAEHCDEBAESON"))
		NoSeaBedCheat();

	// WATERLAYERSCHEAT
	if (!_CHEATCMP("TAEHCSREYALRETAW"))
		RenderWaterLayersCheat();
#endif

#undef _CHEATCMP
}

#ifdef XINPUT
void CPad::AffectFromXinput(uint32 pad)
{
	XINPUT_STATE xstate;
	memset(&xstate, 0, sizeof(XINPUT_STATE));
	if (XInputGetState(pad, &xstate) == ERROR_SUCCESS)
	{
		PCTempJoyState.Circle = (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_B) ? 255 : 0;
		PCTempJoyState.Cross = (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_A) ? 255 : 0;
		PCTempJoyState.Square = (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_X) ? 255 : 0;
		PCTempJoyState.Triangle = (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_Y) ? 255 : 0;
		PCTempJoyState.DPadDown = (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN) ? 255 : 0;
		PCTempJoyState.DPadLeft = (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT) ? 255 : 0;
		PCTempJoyState.DPadRight = (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) ? 255 : 0;
		PCTempJoyState.DPadUp = (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP) ? 255 : 0;
		PCTempJoyState.LeftShock = (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB) ? 255 : 0;
		PCTempJoyState.LeftShoulder1 = (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER) ? 255 : 0;
		PCTempJoyState.LeftShoulder2 = xstate.Gamepad.bLeftTrigger;
		PCTempJoyState.RightShock = (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB) ? 255 : 0;
		PCTempJoyState.RightShoulder1 = (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER) ? 255 : 0;
		PCTempJoyState.RightShoulder2 = xstate.Gamepad.bRightTrigger;

		PCTempJoyState.Select = (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_BACK) ? 255 : 0;
#ifdef REGISTER_START_BUTTON
		PCTempJoyState.Start = (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_START) ? 255 : 0;
#endif
		float lx = (float)xstate.Gamepad.sThumbLX / (float)0x7FFF;
		float ly = (float)xstate.Gamepad.sThumbLY / (float)0x7FFF;
		float rx = (float)xstate.Gamepad.sThumbRX / (float)0x7FFF;
		float ry = (float)xstate.Gamepad.sThumbRY / (float)0x7FFF;

		if (Abs(lx) > 0.3f || Abs(ly) > 0.3f) {
			PCTempJoyState.LeftStickX = (int32)(lx * 128.0f);
			PCTempJoyState.LeftStickY = (int32)(-ly * 128.0f);
		}

		if (Abs(rx) > 0.3f || Abs(ry) > 0.3f) {
			PCTempJoyState.RightStickX = (int32)(rx * 128.0f);
			PCTempJoyState.RightStickY = (int32)(-ry * 128.0f);
		}

		XINPUT_VIBRATION VibrationState;

		memset(&VibrationState, 0, sizeof(XINPUT_VIBRATION));

		uint16 iLeftMotor = (uint16)((float)ShakeFreq / 255.0f * (float)0xffff);
		uint16 iRightMotor = (uint16)((float)ShakeFreq / 255.0f * (float)0xffff);

		if (ShakeDur < CTimer::GetTimeStepInMilliseconds())
			ShakeDur = 0;
		else
			ShakeDur -= CTimer::GetTimeStepInMilliseconds();
		if (ShakeDur == 0) ShakeFreq = 0;

		VibrationState.wLeftMotorSpeed = iLeftMotor;
		VibrationState.wRightMotorSpeed = iRightMotor;

		XInputSetState(pad, &VibrationState);
	}
}
#endif

void CPad::UpdatePads(void)
{
	bool bUpdate = true;

	GetPad(0)->UpdateMouse();
#ifdef XINPUT
	GetPad(0)->AffectFromXinput(m_bMapPadOneToPadTwo ? 1 : 0);
	GetPad(1)->AffectFromXinput(m_bMapPadOneToPadTwo ? 0 : 1);
#else
	CapturePad(0);
#endif

	// Improve keyboard input latency part 1
#ifdef FIX_BUGS
	OldKeyState = NewKeyState;
	NewKeyState = TempKeyState;
#endif

#ifdef DETECT_PAD_INPUT_SWITCH
	if (GetPad(0)->PCTempJoyState.CheckForInput())
		IsAffectedByController = true;
	else {
#endif
		ControlsManager.ClearSimButtonPressCheckers();
		ControlsManager.AffectPadFromKeyBoard();
		ControlsManager.AffectPadFromMouse();

#ifdef DETECT_PAD_INPUT_SWITCH
	}
	if (IsAffectedByController && (GetPad(0)->PCTempKeyState.CheckForInput() || GetPad(0)->PCTempMouseState.CheckForInput()))
		IsAffectedByController = false;
#endif

	if ( CReplay::IsPlayingBackFromFile() && !FrontEndMenuManager.m_bMenuActive )
		bUpdate = false;

	if ( bUpdate )
	{
		GetPad(0)->Update(0);
		// GetPad(1)->Update(0); // not in VC
	}

#if defined(MASTER) && !defined(XINPUT)
	GetPad(1)->NewState.Clear();
	GetPad(1)->OldState.Clear();
#endif

	// Improve keyboard input latency part 2
#ifndef FIX_BUGS
	OldKeyState = NewKeyState;
	NewKeyState = TempKeyState;
#endif
}

void CPad::ProcessPCSpecificStuff(void)
{
	;
}

void CPad::Update(int16 pad)
{
	OldState = NewState;

#ifdef GTA_PS2
	bObsoleteControllerMessage = false;

	//int iPressureBtn;
	int id;
	int ext_id=0;
	int state;
	int rterm_id = 0;
	unsigned short paddata, tpad;
	unsigned char rdata[32];

	state = scePadGetState(pad, 0);

	switch(Phase)
	{
	case 0:
		if (state != scePadStateStable && state != scePadStateFindCTP1)
			break;
		id = scePadInfoMode(pad, 0, InfoModeCurID, 0);
		if (id==0) break;

		ext_id = scePadInfoMode(pad, 0, InfoModeCurExID, 0);
		if (ext_id>0) id = ext_id;

		switch(id)
		{
		case 4: // Digital controller
			Phase = 40; // Try for analog(dualshock)
			break;
		case 7: // Dualshock2 controller
			Phase = 50;
			break;
		default:
			Phase = 99;
			break;
		}
		break;

		// Analog Controller (old dualshock)
	case 40: // Analog Contoller check valid (otherwise fail phase)
		if (scePadInfoMode(pad, 0, InfoModeIdTable, -1)==0)
		{
			Phase = 99;
			break;
		}
		Phase++;

	case 41: // Analog controller: Request Lock analog mode (asynchronous)
		if (scePadSetMainMode(pad, 0, 1, 3)==1) Phase++;
		break;

	case 42: // Analog controller: Check state of previous request
		if (scePadGetReqState(pad, 0)==scePadReqStateFaild)
		{
			Phase--;
		}

		if (scePadGetReqState(pad, 0)==scePadReqStateComplete)
		{
			// Lock mode complete
			Phase=0; // Accept normal dualshock
		}
		break;

		// DualShock 2 Controller
	case 50: // Analog Contoller check valid (otherwise fail phase)
		if (scePadInfoMode(pad, 0, InfoModeIdTable, -1)==0)
		{
			Phase = 99;
			break;
		}
		Phase++;

	case 51: // Analog controller: Request Lock analog mode (asynchronous)
		if (scePadSetMainMode(pad, 0, 1, 3)==1) Phase++;
		break;

	case 52: // Analog controller: Check state of previous request
		if (scePadGetReqState(pad, 0)==scePadReqStateFaild)
		{
			Phase--;
		}

		if (scePadGetReqState(pad, 0)==scePadReqStateComplete)
		{
			// Lock mode complete
			Phase=0; // Accept normal dualshock
		}
		break;

	case 70: // DualShock 2 check pressure sensitive possible
		if (scePadInfoPressMode(pad, 0)==1)
		{
			Phase = 76;
			break;
		}
		Phase = 99;
		break;

	case 76: // DualShock2 enable pressure sensitive mode (asynchronous function)
		if (scePadEnterPressMode(pad, 0)==1) Phase++;
		break;

	case 77: // Dualshock2 check status of request pressure sensitive mode
		if (scePadGetReqState(pad, 0)==scePadReqStateFaild) Phase--;
		if (scePadGetReqState(pad, 0)==scePadReqStateComplete)
		{
			Phase=80;
		}
		break;

		// DualShock 2 Controller
	case 80: // Set motors
		if (scePadInfoAct(pad, 0, -1, 0)==0)
		{
			Phase = 99;
		}

		act_align[0] = 0; // Offset 0 for motor0
		act_align[1] = 1; // Offset 1 for motor1

		act_align[2] = 0xff;
		act_align[3] = 0xff;
		act_align[4] = 0xff;
		act_align[5] = 0xff;

		// Asynchronous function
		if (scePadSetActAlign(pad, 0, act_align)==0) break;
		Phase++;
		break;


	case 81:
		if ( scePadGetState(pad, 0) != scePadStateExecCmd )
		{
			Phase = 99;
		}

		break;

	default:
		if ( state == scePadStateError ) break;

		if ( state == scePadStateStable || state == scePadStateFindCTP1 )
		{
			if ( ShakeDur )
			{
				ShakeDur = Max(ShakeDur - (int32)CTimer::GetTimeStepInMilliseconds(), 0);

				if ( ShakeDur == 0 )
				{
					act_direct[0] = 0;
					act_direct[1] = 0;
					scePadSetActDirect(pad, 0, act_direct);
				}
				else
				{
					act_direct[0] = 0;
					act_direct[1] = (unsigned char) ShakeFreq;
					scePadSetActDirect(pad, 0, act_direct);
				}
			}

			if (scePadRead( pad, 0, rdata )==0)
			{
				NewState.Clear();
				break;
			}

			if ((rdata[0] == 0))
			{
				paddata = (unsigned short) ( 0xffff ^ ((rdata[2]<<8)|rdata[3]) );
				rterm_id = (rdata[1]);

				if ( (rterm_id>>4) == 7 ) // DUALSHOCK
				{
					if (!CRecordDataForGame::IsPlayingBack() && !CRecordDataForChase::ShouldThisPadBeLeftAlone(pad))
					{
						tpad = paddata;

						NewState.DPadUp			= ( tpad & SCE_PADLup )	   ? 255 : 0;
						NewState.DPadDown		= ( tpad & SCE_PADLdown )  ? 255 : 0;
						NewState.DPadLeft		= ( tpad & SCE_PADLleft )  ? 255 : 0;
						NewState.DPadRight		= ( tpad & SCE_PADLright ) ? 255 : 0;
						NewState.Triangle		= ( tpad & SCE_PADRup )	   ? 255 : 0;
						NewState.Cross			= ( tpad & SCE_PADRdown )  ? 255 : 0;
						NewState.Square			= ( tpad & SCE_PADRleft )  ? 255 : 0;
						NewState.Circle			= ( tpad & SCE_PADRright ) ? 255 : 0;
						NewState.Start			= ( tpad & SCE_PADstart )  ? 255 : 0;
						NewState.Select			= ( tpad & SCE_PADselect ) ? 255 : 0;
						NewState.LeftShoulder1	= ( tpad & SCE_PADL1 )	   ? 255 : 0;
						NewState.LeftShoulder2	= ( tpad & SCE_PADL2 )	   ? 255 : 0;
						NewState.RightShoulder1 = ( tpad & SCE_PADR1 )	   ? 255 : 0;
						NewState.RightShoulder2 = ( tpad & SCE_PADR2 )	   ? 255 : 0;
						NewState.LeftShock		= ( tpad & SCE_PADi )	   ? 255 : 0;
						NewState.RightShock		= ( tpad & SCE_PADj )	   ? 255 : 0;
						NewState.RightStickX	= (short)rdata[4];
						NewState.RightStickY	= (short)rdata[5];
						NewState.LeftStickX		= (short)rdata[6];
						NewState.LeftStickY		= (short)rdata[7];

						#define CLAMP_AXIS(x) (((x) < 43 && (x) >= -42) ? 0 : (((x) > 0) ? (Max((x)-42, 0)*127/85) : Min((x)+42, 0)*127/85))
						#define FIX_AXIS(x) CLAMP_AXIS((x)-128)

						NewState.RightStickX = FIX_AXIS(NewState.RightStickX);
						NewState.RightStickY = FIX_AXIS(NewState.RightStickY);
						NewState.LeftStickX	 = FIX_AXIS(NewState.LeftStickX);
						NewState.LeftStickY	 = FIX_AXIS(NewState.LeftStickY);

						#undef FIX_AXIS
						#undef CLAMP_AXIS
					}
				}
				else if ( (rterm_id>>4) == 4 ) // Controller (digital)
				{
					if ( pad == 0 )
						bObsoleteControllerMessage = true;
					NewState.Clear();
				}

				if ( NewState.IsAnyButtonPressed() )
					LastTimeTouched = CTimer::GetTimeInMilliseconds();

				break;
			}

			if ( ++iCurrHornHistory >= HORNHISTORY_SIZE )
				iCurrHornHistory = 0;

			bHornHistory[iCurrHornHistory] = GetHorn();
			NewState.Clear();
			return;
		}
		break;
	}

	if ( pad == 0 )
	{
		bOldDisplayNoControllerMessage = bDisplayNoControllerMessage;
		if ( state == scePadStateDiscon )
		{
			bDisplayNoControllerMessage = true;
			Phase = 0;
		}
		else
			bDisplayNoControllerMessage = false;
	}

	if ( ++iCurrHornHistory >= HORNHISTORY_SIZE )
		iCurrHornHistory = 0;

	bHornHistory[iCurrHornHistory] = GetHorn();

	if ( !bDisplayNoControllerMessage )
		CGame::bDemoMode = false;
#endif

#if (defined GTA_PS2 || defined FIX_BUGS)
	if (!CRecordDataForGame::IsPlayingBack() && !CRecordDataForChase::ShouldThisPadBeLeftAlone(pad))
#endif
	{
		NewState = ReconcileTwoControllersInput(PCTempKeyState, PCTempJoyState);
		NewState = ReconcileTwoControllersInput(PCTempMouseState, NewState);
	}

	PCTempJoyState.Clear();
	PCTempKeyState.Clear();
	PCTempMouseState.Clear();

	ProcessPCSpecificStuff();

	if (NewState.CheckForInput())
		LastTimeTouched = CTimer::GetTimeInMilliseconds();
	
	if ( ++iCurrHornHistory >= HORNHISTORY_SIZE )
		iCurrHornHistory = 0;

	bHornHistory[iCurrHornHistory] = GetHorn();

	for (int32 i = DRUNK_STEERING_BUFFER_SIZE - 2; i >= 0; i--) {
		SteeringLeftRightBuffer[i + 1] = SteeringLeftRightBuffer[i];
	}

	if ( !bDisplayNoControllerMessage )
		CGame::bDemoMode = false;

	if ( JustOutOfFrontend != 0 )
		--JustOutOfFrontend;
}

void CPad::DoCheats(void)
{
#ifdef DETECT_PAD_INPUT_SWITCH
	if (IsAffectedByController)
#endif
		GetPad(0)->DoCheats(0);
}

void CPad::DoCheats(int16 unk)
{
#ifdef GTA_PS2_STUFF
	if ( GetTriangleJustDown() )
		AddToCheatString('T');

	if ( GetCircleJustDown() )
		AddToCheatString('C');

	if ( GetCrossJustDown() )
		AddToCheatString('X');

	if ( GetSquareJustDown() )
		AddToCheatString('S');

	if ( GetDPadUpJustDown() )
		AddToCheatString('U');

	if ( GetDPadDownJustDown() )
		AddToCheatString('D');

	if ( GetDPadLeftJustDown() )
		AddToCheatString('L');

	if ( GetDPadRightJustDown() )
		AddToCheatString('R');

	if ( GetLeftShoulder1JustDown() )
		AddToCheatString('1');

	if ( GetLeftShoulder2JustDown() )
		AddToCheatString('2');

	if ( GetRightShoulder1JustDown() )
		AddToCheatString('3');

	if ( GetRightShoulder2JustDown() )
		AddToCheatString('4');
#endif
}

void CPad::StopPadsShaking(void)
{
	GetPad(0)->StopShaking(0);
}

void CPad::StopShaking(int16 pad)
{
#ifdef GTA_PS2_STUFF
	ShakeFreq = 0;
	ShakeDur = 0;

#ifdef GTA_PS2
	if ( Phase == 99 )
	{
		act_direct[0] = 0;
		act_direct[1] = 0;
		scePadSetActDirect(pad, 0, act_direct);
	}
#endif

#endif
}

CPad *CPad::GetPad(int32 pad)
{
	return &Pads[pad];
}
#ifdef DETECT_PAD_INPUT_SWITCH
#define CURMODE (IsAffectedByController ? Mode : 0)
#else
#define CURMODE (Mode)
#endif

int16 CPad::GetSteeringLeftRight(void)
{
	if ( ArePlayerControlsDisabled() )
		return 0;

	int16 value;
	switch (CURMODE)
	{
		case 0:
		case 2:
		{
			int16 axis = NewState.LeftStickX;
			int16 dpad = (NewState.DPadRight - NewState.DPadLeft) / 2;

			if ( Abs(axis) > Abs(dpad) )
				value = axis;
			else
				value = dpad;

			SteeringLeftRightBuffer[0] = value;
			value = SteeringLeftRightBuffer[DrunkDrivingBufferUsed];

			break;
		}

		case 1:
		case 3:
		{
			SteeringLeftRightBuffer[0] = NewState.LeftStickX;
			value = SteeringLeftRightBuffer[DrunkDrivingBufferUsed];
			break;
		}
		default:
		{
			value = 0;
			break;
		}
	}

	return value;
}

int16 CPad::GetSteeringUpDown(void)
{
	if ( ArePlayerControlsDisabled() )
		return 0;

	switch (CURMODE)
	{
		case 0:
		case 2:
		{
			int16 axis = NewState.LeftStickY;
			int16 dpad = (NewState.DPadDown - NewState.DPadUp) / 2;

			if ( Abs(axis) > Abs(dpad) )
				return axis;
			else
				return dpad;

			break;
		}

		case 1:
		case 3:
		{
			return NewState.LeftStickY;

			break;
		}
	}

	return 0;
}

int16 CPad::GetCarGunUpDown(void)
{
	if ( ArePlayerControlsDisabled() )
		return 0;

	switch (CURMODE)
	{
		case 0:
		case 1:
		case 2:
		{
			return NewState.RightStickY;

			break;
		}

		case 3:
		{
			return (NewState.DPadUp - NewState.DPadDown) / 2;

			break;
		}
	}

	return 0;
}

int16 CPad::GetCarGunLeftRight(void)
{
	if ( ArePlayerControlsDisabled() )
		return 0;

	switch (CURMODE)
	{
		case 0:
		case 1:
		case 2:
		{
			return NewState.RightStickX;

			break;
		}

		case 3:
		{
			return (NewState.DPadRight - NewState.DPadLeft) / 2;

			break;
		}
	}

	return 0;
}

int16 CPad::GetPedWalkLeftRight(void)
{
	if ( ArePlayerControlsDisabled() )
		return 0;

	switch (CURMODE)
	{
		case 0:
		case 2:
		{
			int16 axis = NewState.LeftStickX;
			int16 dpad = (NewState.DPadRight - NewState.DPadLeft) / 2;

			if ( Abs(axis) > Abs(dpad) )
				return axis;
			else
				return dpad;

			break;
		}

		case 1:
		case 3:
		{
			return NewState.LeftStickX;

			break;
		}
	}

	return 0;
}

int16 CPad::GetPedWalkUpDown(void)
{
	if ( ArePlayerControlsDisabled() )
		return 0;

	switch (CURMODE)
	{
		case 0:
		case 2:
		{
			int16 axis = NewState.LeftStickY;
			int16 dpad = (NewState.DPadDown - NewState.DPadUp) / 2;

			if ( Abs(axis) > Abs(dpad) )
				return axis;
			else
				return dpad;

			break;
		}

		case 1:
		case 3:
		{
			return NewState.LeftStickY;

			break;
		}
	}

	return 0;
}

int16 CPad::GetAnalogueUpDown(void)
{
	switch (CURMODE)
	{
		case 0:
		case 2:
		{
			int16 axis = NewState.LeftStickY;
			int16 dpad = (NewState.DPadDown - NewState.DPadUp) / 2;

			if ( Abs(axis) > Abs(dpad) )
				return axis;
			else
				return dpad;

			break;
		}

		case 1:
		case 3:
		{
			return NewState.LeftStickY;

			break;
		}
	}

	return 0;
}

int16 CPad::GetAnalogueLeftRight(void)
{
	switch (CURMODE)
	{
		case 0:
		case 2:
		{
			int16 axis = NewState.LeftStickX;
			int16 dpad = (NewState.DPadRight - NewState.DPadLeft) / 2;

			if ( Abs(axis) > Abs(dpad) )
				return axis;
			else
				return dpad;

			break;
		}

		case 1:
		case 3:
		{
			return NewState.LeftStickX;

			break;
		}
	}

	return 0;
}

bool CPad::GetLookLeft(void)
{
	if ( ArePlayerControlsDisabled() )
		return false;

	return !!(NewState.LeftShoulder2 && !NewState.RightShoulder2);
}

bool CPad::GetLookRight(void)
{
	if ( ArePlayerControlsDisabled() )
		return false;

	return !!(NewState.RightShoulder2 && !NewState.LeftShoulder2);
}


bool CPad::GetLookBehindForCar(void)
{
	if ( ArePlayerControlsDisabled() )
		return false;

	return !!(NewState.RightShoulder2 && NewState.LeftShoulder2);
}

bool CPad::GetLookBehindForPed(void)
{
	if ( ArePlayerControlsDisabled() )
		return false;

	return !!NewState.RightShock;
}

bool CPad::GetHorn(void)
{
	if ( ArePlayerControlsDisabled() )
		return false;

	switch (CURMODE)
	{
		case 0:
		{
			return !!NewState.LeftShock;

			break;
		}

		case 1:
		{
			return !!NewState.LeftShoulder1;

			break;
		}

		case 2:
		{
			return !!NewState.RightShoulder1;

			break;
		}

		case 3:
		{
			return !!NewState.LeftShock;

			break;
		}
	}

	return false;
}

bool CPad::HornJustDown(void)
{
	if ( ArePlayerControlsDisabled() )
		return false;

	switch (CURMODE)
	{
		case 0:
		{
			return !!(NewState.LeftShock && !OldState.LeftShock);

			break;
		}

		case 1:
		{
			return !!(NewState.LeftShoulder1 && !OldState.LeftShoulder1);

			break;
		}

		case 2:
		{
			return !!(NewState.RightShoulder1 && !OldState.RightShoulder1);

			break;
		}

		case 3:
		{
			return !!(NewState.LeftShock && !OldState.LeftShock);

			break;
		}
	}

	return false;
}

bool CPad::GetCarGunFired(void)
{
	if ( ArePlayerControlsDisabled() )
		return false;

	switch (CURMODE)
	{
		case 0:
		case 1:
		case 2:
		{
			return !!NewState.Circle;

			break;
		}

		case 3:
		{
			return !!NewState.RightShoulder1;

			break;
		}
	}

	return false;
}

bool CPad::CarGunJustDown(void)
{
	if ( ArePlayerControlsDisabled() )
		return false;

	switch (CURMODE)
	{
		case 0:
		case 1:
		case 2:
		{
			return !!(NewState.Circle && !OldState.Circle);

			break;
		}

		case 3:
		{
			return !!(NewState.RightShoulder1 && !OldState.RightShoulder1);

			break;
		}
	}

	return false;
}

int16 CPad::GetHandBrake(void)
{
	if ( ArePlayerControlsDisabled() )
		return 0;

	switch (CURMODE)
	{
		case 0:
		case 1:
		{
			return NewState.RightShoulder1;

			break;
		}

		case 2:
		{
			return NewState.Triangle;

			break;
		}

		case 3:
		{
			return NewState.LeftShoulder1;

			break;
		}
	}

	return 0;
}

int16 CPad::GetBrake(void)
{
	if ( ArePlayerControlsDisabled() )
		return 0;

	switch (CURMODE)
	{
		case 0:
		case 2:
		{
			return NewState.Square;

			break;
		}

		case 1:
		{
			return NewState.Square;

			break;
		}

		case 3:
		{
			int16 axis = 2 * NewState.RightStickY;

			if ( axis < 0 )
				return 0;
			else
				return axis;

			break;
		}
	}

	return 0;
}

bool CPad::GetExitVehicle(void)
{
	if ( ArePlayerControlsDisabled() )
		return false;


	if ( JustOutOfFrontend != 0 )
		return false;

	switch (CURMODE)
	{
		case 0:
		case 1:
		case 3:
		{
			return !!NewState.Triangle;

			break;
		}

		case 2:
		{
			return !!NewState.LeftShoulder1;

			break;
		}
	}

	return false;
}

bool CPad::ExitVehicleJustDown(void)
{
	if ( ArePlayerControlsDisabled() )
		return false;

	if ( JustOutOfFrontend != 0 )
		return false;

	switch (CURMODE)
	{
		case 0:
		case 1:
		case 3:
		{
			return !!(NewState.Triangle && !OldState.Triangle);

			break;
		}

		case 2:
		{
			return !!(NewState.LeftShoulder1 && !OldState.LeftShoulder1);

			break;
		}
	}

	return false;
}

int32 CPad::GetWeapon(void)
{
	if ( ArePlayerControlsDisabled() )
		return false;

	switch (CURMODE)
	{
		case 0:
		case 1:
		{
			return NewState.Circle;

			break;
		}

		case 2:
		{
			return NewState.Cross;

			break;
		}

		case 3:
		{
			return NewState.RightShoulder1;

			break;
		}
	}

	return false;
}

bool CPad::WeaponJustDown(void)
{
	if ( ArePlayerControlsDisabled() )
		return false;

	switch (CURMODE)
	{
		case 0:
		case 1:
		{
			return !!(NewState.Circle && !OldState.Circle);

			break;
		}

		case 2:
		{
			return !!(NewState.Cross && !OldState.Cross);

			break;
		}

		case 3:
		{
			return !!(NewState.RightShoulder1 && !OldState.RightShoulder1);

			break;
		}
	}

	return false;
}

int16 CPad::GetAccelerate(void)
{
	if ( ArePlayerControlsDisabled() )
		return 0;

	switch (CURMODE)
	{
		case 0:
		case 2:
		{
			return NewState.Cross;

			break;
		}

		case 1:
		{
			return NewState.Cross;

			break;
		}

		case 3:
		{
			int16 axis = -2 * NewState.RightStickY;

			if ( axis < 0 )
				return 0;
			else
				return axis;

			break;
		}
	}

	return 0;
}

bool CPad::CycleCameraModeJustDown(void)
{
	bool result;
	switch (CURMODE)
	{
		case 0:
		case 2:
		case 3:
		{
			result = !!(NewState.Select && !OldState.Select);

			break;
		}

		case 1:
		{
			result = !!(NewState.DPadUp && !OldState.DPadUp);

			break;
		}
		default:
		{
			result = false;
			break;
		}
	}

	if (!result)
	{
		switch (CURMODE)
		{
			case 1:
			{
				result = !!(NewState.DPadDown && !OldState.DPadDown);
				break;
			}
			default:
			{
				result = false;
				break;
			}
		}
	}

	return result;
}

bool CPad::CycleCameraModeUpJustDown(void)
{
	switch (CURMODE)
	{
		case 0:
		case 2:
		case 3:
		{
			return !!(NewState.Select && !OldState.Select);

			break;
		}

		case 1:
		{
			return !!(NewState.DPadUp && !OldState.DPadUp);

			break;
		}
	}

	return false;
}

bool CPad::CycleCameraModeDownJustDown(void)
{
	switch (CURMODE)
	{
		case 0:
		case 2:
		case 3:
		{
			return false;

			break;
		}

		case 1:
		{
			return !!(NewState.DPadDown && !OldState.DPadDown);

			break;
		}
	}

	return false;
}

bool CPad::ChangeStationJustDown(void)
{
	if ( ArePlayerControlsDisabled() )
		return false;

	switch (CURMODE)
	{
		case 0:
		{
			return !!(NewState.LeftShoulder1 && !OldState.LeftShoulder1);

			break;
		}

		case 1:
		{
			return !!(NewState.Select && !OldState.Select);

			break;
		}

		case 2:
		{
			return !!(NewState.LeftShock && !OldState.LeftShock);

			break;
		}

		case 3:
		{
			return !!(NewState.Circle && !OldState.Circle);

			break;
		}
	}

	return false;
}

bool CPad::CycleWeaponLeftJustDown(void)
{
	if ( ArePlayerControlsDisabled() )
		return false;

	return !!(NewState.LeftShoulder2 && !OldState.LeftShoulder2);
}

bool CPad::CycleWeaponRightJustDown(void)
{
	if ( ArePlayerControlsDisabled() )
		return false;

	return !!(NewState.RightShoulder2 && !OldState.RightShoulder2);
}

bool CPad::GetTarget(void)
{
	if ( ArePlayerControlsDisabled() )
		return false;

	switch (CURMODE)
	{
		case 0:
		case 1:
		case 2:
		{
			return !!NewState.RightShoulder1;

			break;
		}

		case 3:
		{
			return !!NewState.LeftShoulder1;

			break;
		}
	}

	return false;
}

bool CPad::TargetJustDown(void)
{
	if ( ArePlayerControlsDisabled() )
		return false;

	switch (CURMODE)
	{
		case 0:
		case 1:
		case 2:
		{
			return !!(NewState.RightShoulder1 && !OldState.RightShoulder1);

			break;
		}

		case 3:
		{
			return !!(NewState.LeftShoulder1 && !OldState.LeftShoulder1);

			break;
		}
	}

	return false;
}

bool CPad::CollectPickupJustDown(void)
{
	if ( ArePlayerControlsDisabled() )
		return false;

	switch (CURMODE)
	{
		case 0:
		case 1:
		{
			return !!(NewState.LeftShoulder1 && !OldState.LeftShoulder1);

			break;
		}
		case 2:
		{
			return !!(NewState.Triangle && !OldState.Triangle);

			break;
		}

		case 3:
		{
			return !!(NewState.Circle && !OldState.Circle);

			break;
		}
	}

	return false;
}

bool CPad::DuckJustDown(void)
{
	if (ArePlayerControlsDisabled())
		return false;

	return !!(NewState.LeftShock && !OldState.LeftShock);
}

bool CPad::JumpJustDown(void)
{
	if ( ArePlayerControlsDisabled() )
		return false;

	return !!(NewState.Square && !OldState.Square);
}

bool CPad::GetSprint(void)
{
	if ( ArePlayerControlsDisabled() )
		return false;

	switch (CURMODE)
	{
		case 0:
		case 1:
		case 3:
		{
			return !!NewState.Cross;

			break;
		}

		case 2:
		{
			return !!NewState.Circle;

			break;
		}
	}

	return false;
}

bool CPad::ShiftTargetLeftJustDown(void)
{
	if ( ArePlayerControlsDisabled() )
		return false;

	return !!(NewState.LeftShoulder2 && !OldState.LeftShoulder2);
}

bool CPad::ShiftTargetRightJustDown(void)
{
	if ( ArePlayerControlsDisabled() )
		return false;

	return !!(NewState.LeftShoulder1 && !OldState.LeftShoulder1) || !!(NewState.RightShoulder2 && !OldState.RightShoulder2);
}

bool CPad::GetAnaloguePadUp(void)
{
	static int16 oldfStickY = 0;

	int16 leftStickY = CPad::GetPad(0)->GetLeftStickY();

	if ( leftStickY < -15 && oldfStickY >= -5 )
	{
		oldfStickY = leftStickY;
		return true;
	}
	else
	{
		oldfStickY = leftStickY;
		return false;
	}
}

bool CPad::GetAnaloguePadDown(void)
{
	static int16 oldfStickY = 0;

	int16 leftStickY = CPad::GetPad(0)->GetLeftStickY();

	if ( leftStickY > 15 && oldfStickY <= 5 )
	{
		oldfStickY = leftStickY;
		return true;
	}
	else
	{
		oldfStickY = leftStickY;
		return false;
	}
}

bool CPad::GetAnaloguePadLeft(void)
{
	static int16 oldfStickX = 0;

	int16 leftStickX = CPad::GetPad(0)->GetLeftStickX();

	if ( leftStickX < -15 && oldfStickX >= -5 )
	{
		oldfStickX = leftStickX;
		return true;
	}
	else
	{
		oldfStickX = leftStickX;
		return false;
	}
}

bool CPad::GetAnaloguePadRight(void)
{
	static int16 oldfStickX = 0;

	int16 leftStickX = CPad::GetPad(0)->GetLeftStickX();

	if ( leftStickX > 15 && oldfStickX <= 5 )
	{
		oldfStickX = leftStickX;
		return true;
	}
	else
	{
		oldfStickX = leftStickX;
		return false;
	}
}

bool CPad::GetAnaloguePadLeftJustUp(void)
{
	static int16 oldfStickX = 0;

	int16 X = GetPad(0)->GetPedWalkLeftRight();

	if ( X == 0 && oldfStickX < 0 )
	{
		oldfStickX = 0;

		return true;
	}
	else
	{
		oldfStickX = X;

		return false;
	}
}

bool CPad::GetAnaloguePadRightJustUp(void)
{
	static int16 oldfStickX = 0;

	int16 X = GetPad(0)->GetPedWalkLeftRight();

	if ( X == 0 && oldfStickX > 0 )
	{
		oldfStickX = 0;

		return true;
	}
	else
	{
		oldfStickX = X;

		return false;
	}
}

bool CPad::ForceCameraBehindPlayer(void)
{
	if ( ArePlayerControlsDisabled() )
		return false;

	switch (CURMODE)
	{
		case 0:
		case 1:
		{
			return !!NewState.LeftShoulder1;

			break;
		}

		case 2:
		{
			return !!NewState.Triangle;

			break;
		}

		case 3:
		{
			return !!NewState.Circle;

			break;
		}
	}

	return false;
}

bool CPad::SniperZoomIn(void)
{
	if ( ArePlayerControlsDisabled() )
		return false;

	switch (CURMODE)
	{
		case 0:
		case 1:
		case 3:
		{
			return !!NewState.Square;

			break;
		}

		case 2:
		{
			return !!NewState.Triangle;

			break;
		}
	}

	return false;
}

bool CPad::SniperZoomOut(void)
{
	if ( ArePlayerControlsDisabled() )
		return false;

	switch (CURMODE)
	{
		case 0:
		case 1:
		case 3:
		{
			return !!NewState.Cross;

			break;
		}

		case 2:
		{
			return !!NewState.Square;

			break;
		}
	}

	return false;
}

#undef CURMODE

int16 CPad::SniperModeLookLeftRight(void)
{
	int16 axis = NewState.LeftStickX;
	int16 dpad = (NewState.DPadRight - NewState.DPadLeft) / 2;

	if ( Abs(axis) > Abs(dpad) ) {
		if ( Abs(axis) > 35.0f ) {
		  return (axis > 0.f ? axis - 35.f : axis + 35.f) * (128.f / (128 - 35));
		} else {
		  return 0;
		}
	} else
		return dpad;
}

int16 CPad::SniperModeLookUpDown(void)
{
	int16 axis = NewState.LeftStickY;
	int16 dpad;

#ifdef FIX_BUGS
	axis = -axis;
#endif
	if (CPad::bInvertLook4Pad) {
		axis = -axis;
		dpad = (NewState.DPadDown - NewState.DPadUp) / 2;
	} else {
		dpad = (NewState.DPadUp - NewState.DPadDown) / 2;
	}

	if ( Abs(axis) > Abs(dpad) ) {
	    if ( Abs(axis) > 35.0f ) {
	      return (axis > 0.f ? axis - 35.f : axis + 35.f) * (128.f / (128 - 35));
	    } else {
	      return 0;
	    }
	} else
		return dpad;
}

int16 CPad::LookAroundLeftRight(void)
{
	float axis = GetPad(0)->NewState.RightStickX;

	if ( Abs(axis) > 85 && !GetLookBehindForPed() )
		return (int16) ( (axis + ( ( axis > 0 ) ? -85 : 85) )
							* (127.0f / 32.0f) ); // 3.96875f

	else if ( TheCamera.Cams[0].Using3rdPersonMouseCam() && Abs(axis) > 10 )
		return (int16) ( (axis + ( ( axis > 0 ) ? -10 : 10) )
							* (127.0f / 64.0f) ); // 1.984375f

	return 0;
}

int16 CPad::LookAroundUpDown(void)
{
	int16 axis = GetPad(0)->NewState.RightStickY;
#ifdef FIX_BUGS
	axis = -axis;
#endif
	if (CPad::bInvertLook4Pad)
		axis = -axis;

	if ( Abs(axis) > 85 && !GetLookBehindForPed() )
		return (int16) ( (axis + ( ( axis > 0 ) ? -85 : 85) )
							* (127.0f / 32.0f) ); // 3.96875f

	else if ( TheCamera.Cams[0].Using3rdPersonMouseCam() && Abs(axis) > 40 )
		return (int16) ( (axis + ( ( axis > 0 ) ? -40 : 40) )
							* (127.0f / 64.0f) ); // 1.984375f

	return 0;
}

void CPad::ResetAverageWeapon(void)
{
	AverageWeapon = GetWeapon();
	AverageEntries = 1;
}

void CPad::PrintErrorMessage(void)
{
	if (TheCamera.m_WideScreenOn)
		return;

	if ( bDisplayNoControllerMessage && !CGame::playingIntro && !FrontEndMenuManager.m_bMenuActive )
	{
		CSprite2d::DrawRect(CRect(SCREEN_STRETCH_X(20.0f), SCREEN_SCALE_FROM_BOTTOM(130.0f), SCREEN_STRETCH_FROM_RIGHT(20.0f), SCREEN_SCALE_Y(140.0f)), CRGBA(50, 50, 50, 210));
		CFont::SetScale(SCREEN_SCALE_X(0.85f), SCREEN_SCALE_Y(1.0f));
		CFont::SetJustifyOff();
		CFont::SetBackgroundOff();
		CFont::SetCentreSize(SCREEN_SCALE_X(590.0f));
		CFont::SetCentreOn();
		CFont::SetPropOn();
		CFont::SetColor(CRGBA(255, 255, 200, 200));
		CFont::SetFontStyle(FONT_STANDARD);
		CFont::PrintString
		(
			SCREEN_WIDTH  / 2,
			SCREEN_HEIGHT / 2 - SCREEN_SCALE_Y(40.0f),
			TheText.Get("NOCONT") // Please reconnect an analog controller (DUALSHOCK@) or analog controller (DUALSHOCK@2). to controller port 1 to continue
		);
	}
	else if ( bObsoleteControllerMessage )
	{
		CSprite2d::DrawRect(CRect(SCREEN_STRETCH_X(20.0f), SCREEN_SCALE_FROM_BOTTOM(130.0f), SCREEN_STRETCH_FROM_RIGHT(20.0f), SCREEN_SCALE_Y(140.0f)), CRGBA(50, 50, 50, 210));
		CFont::SetScale(SCREEN_SCALE_X(0.85f), SCREEN_SCALE_Y(1.0f));
		CFont::SetJustifyOff();
		CFont::SetBackgroundOff();
		CFont::SetCentreSize(SCREEN_SCALE_X(590.0f));
		CFont::SetCentreOn();
		CFont::SetPropOn();
		CFont::SetColor(CRGBA(255, 255, 200, 200));
		CFont::SetFontStyle(FONT_STANDARD);
		CFont::PrintString
		(
			SCREEN_WIDTH  / 2,
			SCREEN_HEIGHT / 2 - SCREEN_SCALE_Y(40.0f),
			TheText.Get("WRCONT") // The controller connected to controller port 1 is an unsupported controller. Grand Theft Auto III requires an analog controller (DUALSHOCK@) or analog controller (DUALSHOCK@2).
		);
	}

}

void LittleTest(void)
{
	static int32 Cunt = 0;

	Cunt++; // ???
}

void CPad::ResetCheats(void)
{
	CWeather::ReleaseWeather();

	gbFastTime = false;
	CPopulation::ms_bGivePedsWeapons = false;
	CTimer::SetTimeScale(1.0f);

	CVehicle::bWheelsOnlyCheat = false;
	CVehicle::bAllDodosCheat = false;
	CVehicle::bCheat3 = false;
	CVehicle::bCheat4 = false;
	CVehicle::bCheat5 = false;
	CVehicle::bAllTaxisHaveNitro = false;
	CVehicle::bHoverCheat = false;
	CVehicle::bCheat8 = false;
	CVehicle::bCheat9 = false;
	CVehicle::bCheat10 = false;
#ifdef RESTORE_ALLCARSHELI_CHEAT
	bAllCarCheat = false;
#endif
	gbBlackCars = false;
	gbPinkCars = false;

	CCarCtrl::bMadDriversCheat = false;
	CTrafficLights::bGreenLightsCheat = false;
	CStats::ShowChaseStatOnScreen = 0;
	CPed::bNastyLimbsCheat = false;
	CPed::bFannyMagnetCheat = false;
	CPed::bPedCheat3 = false;

}

char *CPad::EditString(char *pStr, int32 nSize)
{
	int32 pos = (int32)strlen(pStr);

	// letters
	for ( int32 i = 0; i < ('Z' - 'A' + 1); i++ )
	{
		if ( GetPad(0)->GetCharJustDown(i + 'A') && pos < nSize - 1 )
		{
			pStr[pos++] = i + 'A';
			pStr[pos] = '\0';
		}

		if ( GetPad(0)->GetCharJustDown(i + 'a') && pos < nSize - 1 )
		{
			pStr[pos++] = i + 'a';
			pStr[pos] = '\0';
		}
	}

	// numbers
	for ( int32 i = 0; i < ('9' - '0' + 1); i++ )
	{
		if ( GetPad(0)->GetCharJustDown(i + '0') && pos < nSize - 1 )
		{
			pStr[pos++] = i + '0';
			pStr[pos] = '\0';
		}
	}

	// space
	if ( GetPad(0)->GetCharJustDown(' ') && pos < nSize - 1 )
	{
		pStr[pos++] = ' ';
		pStr[pos] = '\0';
	}


	// del
	if ( GetPad(0)->GetDeleteJustDown() || GetPad(0)->GetBackspaceJustDown() )
	{
		if ( pos > 0 )
			pStr[pos - 1] = '\0';
	}

	// extenter/up/down
	if ( GetPad(0)->GetReturnJustDown() || GetPad(0)->GetUpJustDown() || GetPad(0)->GetDownJustDown() )
		return nil;

	return pStr;
}

int32 *CPad::EditCodesForControls(int32 *pRsKeys, int32 nSize)
{
	*pRsKeys = rsNULL;

	for ( int32 i = 0; i < 255; i++ )
	{
		if ( GetPad(0)->GetCharJustDown(i) )
			*pRsKeys = i;
	}

	for ( int32 i = 0; i < 12; i++ )
	{
		if ( GetPad(0)->GetFJustDown(i) )
			*pRsKeys = i + rsF1;
	}

	if ( GetPad(0)->GetEscapeJustDown() )
		*pRsKeys = rsESC;

	if ( GetPad(0)->GetInsertJustDown() )
		*pRsKeys = rsINS;

	if ( GetPad(0)->GetDeleteJustDown() )
		*pRsKeys = rsDEL;

	if ( GetPad(0)->GetHomeJustDown() )
		*pRsKeys = rsHOME;

	if ( GetPad(0)->GetEndJustDown() )
		*pRsKeys = rsEND;

	if ( GetPad(0)->GetPageUpJustDown() )
		*pRsKeys = rsPGUP;

	if ( GetPad(0)->GetPageDownJustDown() )
		*pRsKeys = rsPGDN;

	if ( GetPad(0)->GetUpJustDown() )
		*pRsKeys = rsUP;

	if ( GetPad(0)->GetDownJustDown() )
		*pRsKeys = rsDOWN;

	if ( GetPad(0)->GetLeftJustDown() )
		*pRsKeys = rsLEFT;

	if ( GetPad(0)->GetRightJustDown() )
		*pRsKeys = rsRIGHT;

	if ( GetPad(0)->GetScrollLockJustDown() )
		*pRsKeys = rsSCROLL;

	if ( GetPad(0)->GetPauseJustDown() )
		*pRsKeys = rsPAUSE;

	if ( GetPad(0)->GetNumLockJustDown() )
		*pRsKeys = rsNUMLOCK;

	if ( GetPad(0)->GetDivideJustDown() )
		*pRsKeys = rsDIVIDE;

	if ( GetPad(0)->GetTimesJustDown() )
		*pRsKeys = rsTIMES;

	if ( GetPad(0)->GetMinusJustDown() )
		*pRsKeys = rsMINUS;

	if ( GetPad(0)->GetPlusJustDown() )
		*pRsKeys = rsPLUS;

	if ( GetPad(0)->GetPadEnterJustDown() )
		*pRsKeys = rsPADENTER;

	if ( GetPad(0)->GetPadDelJustDown() )
		*pRsKeys = rsPADDEL;

	if ( GetPad(0)->GetPad1JustDown() )
		*pRsKeys = rsPADEND;

	if ( GetPad(0)->GetPad2JustDown() )
		*pRsKeys = rsPADDOWN;

	if ( GetPad(0)->GetPad3JustDown() )
		*pRsKeys = rsPADPGDN;

	if ( GetPad(0)->GetPad4JustDown() )
		*pRsKeys = rsPADLEFT;

	if ( GetPad(0)->GetPad5JustDown() )
		*pRsKeys = rsPAD5;

	if ( GetPad(0)->GetPad6JustDown() )
		*pRsKeys = rsPADRIGHT;

	if ( GetPad(0)->GetPad7JustDown() )
		*pRsKeys = rsPADHOME;

	if ( GetPad(0)->GetPad8JustDown() )
		*pRsKeys = rsPADUP;

	if ( GetPad(0)->GetPad9JustDown() )
		*pRsKeys = rsPADPGUP;

	if ( GetPad(0)->GetPad0JustDown() )
		*pRsKeys = rsPADINS;

	if ( GetPad(0)->GetBackspaceJustDown() )
		*pRsKeys = rsBACKSP;

	if ( GetPad(0)->GetTabJustDown() )
		*pRsKeys = rsTAB;

	if ( GetPad(0)->GetCapsLockJustDown() )
		*pRsKeys = rsCAPSLK;

	if ( GetPad(0)->GetReturnJustDown() )
		*pRsKeys = rsENTER;

	if ( GetPad(0)->GetLeftShiftJustDown() )
		*pRsKeys = rsLSHIFT;

	if ( GetPad(0)->GetShiftJustDown() )
		*pRsKeys = rsSHIFT;

	if ( GetPad(0)->GetRightShiftJustDown() )
		*pRsKeys = rsRSHIFT;

	if ( GetPad(0)->GetLeftCtrlJustDown() )
		*pRsKeys = rsLCTRL;

	if ( GetPad(0)->GetRightCtrlJustDown() )
		*pRsKeys = rsRCTRL;

	if ( GetPad(0)->GetLeftAltJustDown() )
		*pRsKeys = rsLALT;

	if ( GetPad(0)->GetRightAltJustDown() )
		*pRsKeys = rsRALT;

	if ( GetPad(0)->GetLeftWinJustDown() )
		*pRsKeys = rsLWIN;

	if ( GetPad(0)->GetRightWinJustDown() )
		*pRsKeys = rsRWIN;

	if ( GetPad(0)->GetAppsJustDown() )
		*pRsKeys = rsAPPS;

	return pRsKeys;
}

void CPad::FixPadsAfterSave(void)
{
	UpdatePads();
	if ( bObsoleteControllerMessage )
	{
		bObsoleteControllerMessage = false;
		GetPad(0)->Phase = 0;
	}
}
