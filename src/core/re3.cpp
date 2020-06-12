#include <csignal>
#define WITHWINDOWS
#include "common.h"
#include "crossplatform.h"
#include "patcher.h"
#include "Renderer.h"
#include "Credits.h"
#include "Camera.h"
#include "Weather.h"
#include "Clock.h"
#include "World.h"
#include "Vehicle.h"
#include "ModelIndices.h"
#include "Streaming.h"
#include "PathFind.h"
#include "Boat.h"
#include "Heli.h"
#include "Automobile.h"
#include "Bike.h"
#include "Ped.h"
#include "Particle.h"
#include "Console.h"
#include "Debug.h"
#include "Hud.h"
#include "SceneEdit.h"
#include "Pad.h"
#include "PlayerPed.h"
#include "Radar.h"
#include "debugmenu.h"
#include "Frontend.h"
#include "Text.h"
#include "WaterLevel.h"
#include "main.h"

#ifndef _WIN32
#include "assert.h"
#include <stdarg.h>
#endif

#include <list>

#ifdef RWLIBS
extern "C" int vsprintf(char* const _Buffer, char const* const _Format, va_list  _ArgList);
#endif


#ifdef USE_PS2_RAND
unsigned long long myrand_seed = 1;
#else
unsigned long int myrand_seed = 1;
#endif

int
myrand(void)
{
#ifdef USE_PS2_RAND
	// Use our own implementation of rand, stolen from PS2
	myrand_seed = 0x5851F42D4C957F2D * myrand_seed + 1;
	return ((myrand_seed >> 32) & 0x7FFFFFFF);
#else
	// or original codewarrior rand
	myrand_seed = myrand_seed * 1103515245 + 12345;
	return((myrand_seed >> 16) & 0x7FFF);
#endif
}

void
mysrand(unsigned int seed)
{
	myrand_seed = seed;
}

#ifdef CUSTOM_FRONTEND_OPTIONS
#include "frontendoption.h"
#include "platform.h"

void ReloadFrontendOptions(void)
{
	RemoveCustomFrontendOptions();
	CustomFrontendOptionsPopulate();
}

#ifdef MORE_LANGUAGES
void LangPolSelect(int8 action)
{
	if (action == FEOPTION_ACTION_SELECT) {
		FrontEndMenuManager.m_PrefsLanguage = LANGUAGE_POLISH;
		FrontEndMenuManager.m_bFrontEnd_ReloadObrTxtGxt = true;
		FrontEndMenuManager.InitialiseChangedLanguageSettings();
		FrontEndMenuManager.SaveSettings();
	}
}

void LangRusSelect(int8 action)
{
	if (action == FEOPTION_ACTION_SELECT) {
		FrontEndMenuManager.m_PrefsLanguage = LANGUAGE_RUSSIAN;
		FrontEndMenuManager.m_bFrontEnd_ReloadObrTxtGxt = true;
		FrontEndMenuManager.InitialiseChangedLanguageSettings();
		FrontEndMenuManager.SaveSettings();
	}
}

void LangJapSelect(int8 action)
{
	if (action == FEOPTION_ACTION_SELECT) {
		FrontEndMenuManager.m_PrefsLanguage = LANGUAGE_JAPANESE;
		FrontEndMenuManager.m_bFrontEnd_ReloadObrTxtGxt = true;
		FrontEndMenuManager.InitialiseChangedLanguageSettings();
		FrontEndMenuManager.SaveSettings();
	}
}
#endif

#ifdef IMPROVED_VIDEOMODE
void ScreenModeChange(int8 displayedValue)
{
	if (displayedValue != FrontEndMenuManager.m_nPrefsWindowed) {
		FrontEndMenuManager.m_nPrefsWindowed = displayedValue;
		_psSelectScreenVM(FrontEndMenuManager.m_nPrefsVideoMode); // apply same resolution
		FrontEndMenuManager.SetHelperText(0);
		FrontEndMenuManager.SaveSettings();
	}
}
#endif

#ifdef FREE_CAM
void ToggleFreeCam(int8 action)
{
	if (action == FEOPTION_ACTION_SELECT) {
		TheCamera.bFreeCam = !TheCamera.bFreeCam;
		FrontEndMenuManager.SaveSettings();
	}
}
#endif

// Reloaded on language change, so you can use hardcoded wchar* and TheText.Get with peace of mind
void
CustomFrontendOptionsPopulate(void)
{
#ifdef MORE_LANGUAGES
	FrontendOptionSetPosition(MENUPAGE_LANGUAGE_SETTINGS);
	FrontendOptionAddDynamic(TheText.Get("FEL_POL"), nil, LangPolSelect, nil);
	FrontendOptionAddDynamic(TheText.Get("FEL_RUS"), nil, LangRusSelect, nil);
	FrontendOptionAddDynamic(TheText.Get("FEL_JAP"), nil, LangJapSelect, nil);
#endif

#ifdef IMPROVED_VIDEOMODE
	static const wchar *screenModes[] = { (wchar*)L"FULLSCREEN", (wchar*)L"WINDOWED" };
	FrontendOptionSetPosition(MENUPAGE_GRAPHICS_SETTINGS, 8);
	FrontendOptionAddSelect(TheText.Get("SCRFOR"), screenModes, 2, (int8*)&FrontEndMenuManager.m_nPrefsWindowed, true, ScreenModeChange, nil);
#endif

#ifdef MENU_MAP
	FrontendOptionSetPosition(MENUPAGE_PAUSE_MENU, 2);
	FrontendOptionAddRedirect(TheText.Get("FEG_MAP"), MENUPAGE_MAP);
#endif

#ifdef FREE_CAM
	static const wchar *text = (wchar*)L"TOGGLE FREE CAM";
	FrontendOptionSetPosition(MENUPAGE_CONTROLLER_PC, 1);
	FrontendOptionAddDynamic(text, nil, ToggleFreeCam, nil);
#endif
}
#endif

#ifdef DEBUGMENU
void WeaponCheat1();
void WeaponCheat2();
void WeaponCheat3();
void HealthCheat();
void VehicleCheat(bool something, int model);
void BlowUpCarsCheat();
void ChangePlayerCheat();
void MayhemCheat();
void EverybodyAttacksPlayerCheat();
void WeaponsForAllCheat();
void FastTimeCheat();
void SlowTimeCheat();
void MoneyCheat();
void ArmourCheat();
void WantedLevelUpCheat();
void WantedLevelDownCheat();
void SunnyWeatherCheat();
void CloudyWeatherCheat();
void RainyWeatherCheat();
void FoggyWeatherCheat();
void FastWeatherCheat();
void OnlyRenderWheelsCheat();
void ChittyChittyBangBangCheat();
void StrongGripCheat();
void NastyLimbsCheat();

DebugMenuEntry *carCol1;
DebugMenuEntry *carCol2;

void
SpawnCar(int id)
{
	CVector playerpos;
	CStreaming::RequestModel(id, 0);
	CStreaming::LoadAllRequestedModels(false);
	if(CStreaming::HasModelLoaded(id)){
		playerpos = FindPlayerCoors();
		int node;
		if(!CModelInfo::IsBoatModel(id)){
			node = ThePaths.FindNodeClosestToCoors(playerpos, 0, 100.0f, false, false);
			if(node < 0)
				return;
		}

		CVehicle *v;
		if(CModelInfo::IsBoatModel(id))
			v = new CBoat(id, RANDOM_VEHICLE);
		if(CModelInfo::IsBikeModel(id))
			v = new CBike(id, RANDOM_VEHICLE);
		else
			v = new CAutomobile(id, RANDOM_VEHICLE);

		v->bHasBeenOwnedByPlayer = true;
		if(carCol1)
			DebugMenuEntrySetAddress(carCol1, &v->m_currentColour1);
		if(carCol2)
			DebugMenuEntrySetAddress(carCol2, &v->m_currentColour2);

		if(CModelInfo::IsBoatModel(id))
			v->SetPosition(TheCamera.GetPosition() + TheCamera.GetForward()*15.0f);
		else
			v->SetPosition(ThePaths.m_pathNodes[node].GetPosition());

		v->GetMatrix().GetPosition().z += 4.0f;
		v->SetOrientation(0.0f, 0.0f, 3.49f);
		v->SetStatus(STATUS_ABANDONED);
		v->m_nDoorLock = CARLOCK_UNLOCKED;
		CWorld::Add(v);
	}
}

static void
FixCar(void)
{
	CVehicle *veh = FindPlayerVehicle();
	if(veh == nil)
		return;
	veh->m_fHealth = 1000.0f;
	if(veh->IsCar()){
		((CAutomobile*)veh)->Damage.SetEngineStatus(0);
		((CAutomobile*)veh)->Fix();
	}else if(veh->IsBike()){
		((CBike*)veh)->Fix();
	}
}

#ifdef MENU_MAP
static void
TeleportToWaypoint(void)
{
	if (FindPlayerVehicle()) {
		if (CRadar::TargetMarkerId != -1)
			FindPlayerVehicle()->Teleport(CRadar::TargetMarkerPos + CVector(0.0f, 0.0f, FindPlayerVehicle()->GetColModel()->boundingSphere.center.z));
	} else
		if(CRadar::TargetMarkerId != -1)
			FindPlayerPed()->Teleport(CRadar::TargetMarkerPos + CVector(0.0f, 0.0f, FEET_OFFSET));
}
#endif

static void
SwitchCarCollision(void)
{
	if (FindPlayerVehicle() && FindPlayerVehicle()->IsCar())
		FindPlayerVehicle()->bUsesCollision = !FindPlayerVehicle()->bUsesCollision;
}

static int engineStatus;
static void
SetEngineStatus(void)
{
	CVehicle *veh = FindPlayerVehicle();
	if(veh == nil)
		return;
	if(!veh->IsCar())
		return;
	((CAutomobile*)veh)->Damage.SetEngineStatus(engineStatus);
}

static void
ToggleComedy(void)
{
	CVehicle *veh = FindPlayerVehicle();
	if(veh == nil)
		return;
	veh->bComedyControls = !veh->bComedyControls;
}

static void
PlaceOnRoad(void)
{
	CVehicle *veh = FindPlayerVehicle();
	if(veh == nil)
		return;

	if(veh->IsCar())
		((CAutomobile*)veh)->PlaceOnRoadProperly();
}

static void
ResetCamStatics(void)
{
	TheCamera.Cams[TheCamera.ActiveCam].ResetStatics = true;
}

static const char *carnames[] = {
	"landstal", "idaho", "stinger", "linerun", "peren", "sentinel", "rio", "firetruk", "trash", "stretch", "manana",
	"infernus", "voodoo", "pony", "mule", "cheetah", "ambulan", "fbicar", "moonbeam", "esperant", "taxi", "washing",
	"bobcat", "mrwhoop", "bfinject", "hunter", "police", "enforcer", "securica", "banshee", "predator", "bus",
	"rhino", "barracks", "cuban", "chopper", "angel", "coach", "cabbie", "stallion", "rumpo", "rcbandit", "romero",
	"packer", "sentxs", "admiral", "squalo", "seaspar", "pizzaboy", "gangbur", "airtrain", "deaddodo", "speeder",
	"reefer", "tropic", "flatbed", "yankee", "caddy", "zebra", "topfun", "skimmer", "pcj600", "faggio", "freeway",
	"rcbaron", "rcraider", "glendale", "oceanic", "sanchez", "sparrow", "patriot", "lovefist", "coastg", "dinghy",
	"hermes", "sabre", "sabretur", "pheonix", "walton", "regina", "comet", "deluxo", "burrito", "spand", "marquis",
	"baggage", "kaufman", "maverick", "vcnmav", "rancher", "fbiranch", "virgo", "greenwoo", "jetmax", "hotring",
	"sandking", "blistac", "polmav", "boxville", "benson", "mesa", "rcgoblin", "hotrina", "hotrinb",
	"bloodra", "bloodrb", "vicechee"
};

static CTweakVar** TweakVarsList;
static int TweakVarsListSize = -1;
static bool bAddTweakVarsNow = false;
static const char *pTweakVarsDefaultPath = NULL;

void CTweakVars::Add(CTweakVar *var)
{
	if(TweakVarsListSize == -1) {
		TweakVarsList = (CTweakVar**)malloc(64 * sizeof(CTweakVar*));
		TweakVarsListSize = 0;
	}
	if(TweakVarsListSize > 63)
		TweakVarsList = (CTweakVar**) realloc(TweakVarsList, (TweakVarsListSize + 1) * sizeof(var));

	TweakVarsList[TweakVarsListSize++] = var;
//	TweakVarsList.push_back(var);
	
	if ( bAddTweakVarsNow )
		var->AddDBG(pTweakVarsDefaultPath);
}

void CTweakVars::AddDBG(const char *path)
{
	pTweakVarsDefaultPath = path;

	for(int i = 0; i < TweakVarsListSize; ++i)
		TweakVarsList[i]->AddDBG(pTweakVarsDefaultPath);
	
	bAddTweakVarsNow = true;
}

void CTweakSwitch::AddDBG(const char *path)
{		
	DebugMenuEntry *e = DebugMenuAddVar(m_pPath == NULL ? path : m_pPath, m_pVarName, (int32_t *)m_pIntVar, m_pFunc, 1, m_nMin, m_nMax, m_aStr);
	DebugMenuEntrySetWrap(e, true);
}
	
void CTweakFunc::AddDBG  (const char *path) { DebugMenuAddCmd     (m_pPath == NULL ? path : m_pPath, m_pVarName, m_pFunc); }
void CTweakBool::AddDBG  (const char *path) { DebugMenuAddVarBool8(m_pPath == NULL ? path : m_pPath, m_pVarName, (int8_t *)m_pBoolVar,  NULL); }
void CTweakInt8::AddDBG  (const char *path) { DebugMenuAddVar     (m_pPath == NULL ? path : m_pPath, m_pVarName, (int8_t *)m_pIntVar,   NULL, m_nStep, m_nLoawerBound, m_nUpperBound, NULL); }
void CTweakUInt8::AddDBG (const char *path) { DebugMenuAddVar     (m_pPath == NULL ? path : m_pPath, m_pVarName, (uint8_t *)m_pIntVar,  NULL, m_nStep, m_nLoawerBound, m_nUpperBound, NULL); }
void CTweakInt16::AddDBG (const char *path) { DebugMenuAddVar     (m_pPath == NULL ? path : m_pPath, m_pVarName, (int16_t *)m_pIntVar,  NULL, m_nStep, m_nLoawerBound, m_nUpperBound, NULL); }
void CTweakUInt16::AddDBG(const char *path) { DebugMenuAddVar     (m_pPath == NULL ? path : m_pPath, m_pVarName, (uint16_t *)m_pIntVar, NULL, m_nStep, m_nLoawerBound, m_nUpperBound, NULL); }
void CTweakInt32::AddDBG (const char *path) { DebugMenuAddVar     (m_pPath == NULL ? path : m_pPath, m_pVarName, (int32_t *)m_pIntVar,  NULL, m_nStep, m_nLoawerBound, m_nUpperBound, NULL); }
void CTweakUInt32::AddDBG(const char *path) { DebugMenuAddVar     (m_pPath == NULL ? path : m_pPath, m_pVarName, (uint32_t *)m_pIntVar, NULL, m_nStep, m_nLoawerBound, m_nUpperBound, NULL); }
void CTweakFloat::AddDBG (const char *path) { DebugMenuAddVar     (m_pPath == NULL ? path : m_pPath, m_pVarName, (float *)m_pIntVar,    NULL, m_nStep, m_nLoawerBound, m_nUpperBound); }

/*
static const char *wt[] = {
			"Sunny", "Cloudy", "Rainy", "Foggy"
		};

SETTWEAKPATH("TEST");		
TWEAKSWITCH(CWeather::NewWeatherType, 0, 3, wt, NULL);
*/

void
DebugMenuPopulate(void)
{
	if(1){
		static const char *weathers[] = {
			"Sunny", "Cloudy", "Rainy", "Foggy", "Extrasunny", "Stormy"
		};
		DebugMenuEntry *e;
		e = DebugMenuAddVar("Time & Weather", "Current Hour", &CClock::GetHoursRef(), nil, 1, 0, 23, nil);
		DebugMenuEntrySetWrap(e, true);
		e = DebugMenuAddVar("Time & Weather", "Current Minute", &CClock::GetMinutesRef(),
			[](){ CWeather::InterpolationValue = CClock::GetMinutes()/60.0f; }, 1, 0, 59, nil);
			DebugMenuEntrySetWrap(e, true);
		e = DebugMenuAddVar("Time & Weather", "Old Weather", (int16*)&CWeather::OldWeatherType, nil, 1, 0, 5, weathers);
		DebugMenuEntrySetWrap(e, true);
		e = DebugMenuAddVar("Time & Weather", "New Weather", (int16*)&CWeather::NewWeatherType, nil, 1, 0, 5, weathers);
		DebugMenuEntrySetWrap(e, true);
		DebugMenuAddVar("Time & Weather", "Wind", (float*)&CWeather::Wind, nil, 0.1f, 0.0f, 1.0f);
		DebugMenuAddVar("Time & Weather", "Time scale", (float*)&CTimer::GetTimeScale(), nil, 0.1f, 0.0f, 10.0f);

		DebugMenuAddCmd("Cheats", "Weapon set 1", WeaponCheat1);
		DebugMenuAddCmd("Cheats", "Weapon set 2", WeaponCheat2);
		DebugMenuAddCmd("Cheats", "Weapon set 3", WeaponCheat3);
		DebugMenuAddCmd("Cheats", "Money", MoneyCheat);
		DebugMenuAddCmd("Cheats", "Health", HealthCheat);
		DebugMenuAddCmd("Cheats", "Wanted level up", WantedLevelUpCheat);
		DebugMenuAddCmd("Cheats", "Wanted level down", WantedLevelDownCheat);
		DebugMenuAddCmd("Cheats", "Tank", []() { VehicleCheat(true, MI_TAXI); });
		DebugMenuAddCmd("Cheats", "Blow up cars", BlowUpCarsCheat);
		DebugMenuAddCmd("Cheats", "Change player", ChangePlayerCheat);
		DebugMenuAddCmd("Cheats", "Mayhem", MayhemCheat);
		DebugMenuAddCmd("Cheats", "Everybody attacks player", EverybodyAttacksPlayerCheat);
		DebugMenuAddCmd("Cheats", "Weapons for all", WeaponsForAllCheat);
		DebugMenuAddCmd("Cheats", "Fast time", FastTimeCheat);
		DebugMenuAddCmd("Cheats", "Slow time", SlowTimeCheat);
		DebugMenuAddCmd("Cheats", "Armour", ArmourCheat);
		DebugMenuAddCmd("Cheats", "Sunny weather", SunnyWeatherCheat);
		DebugMenuAddCmd("Cheats", "Cloudy weather", CloudyWeatherCheat);
		DebugMenuAddCmd("Cheats", "Rainy weather", RainyWeatherCheat);
		DebugMenuAddCmd("Cheats", "Foggy weather", FoggyWeatherCheat);
		DebugMenuAddCmd("Cheats", "Fast weather", FastWeatherCheat);
		DebugMenuAddCmd("Cheats", "Only render wheels", OnlyRenderWheelsCheat);
		DebugMenuAddCmd("Cheats", "Chitty chitty bang bang", ChittyChittyBangBangCheat);
		DebugMenuAddCmd("Cheats", "Strong grip", StrongGripCheat);
		DebugMenuAddCmd("Cheats", "Nasty limbs", NastyLimbsCheat);

		static int spawnCarId = MI_LANDSTAL;
		e = DebugMenuAddVar("Spawn", "Spawn Car ID", &spawnCarId, nil, 1, MI_LANDSTAL, MI_VICECHEE, carnames);
		DebugMenuEntrySetWrap(e, true);
		DebugMenuAddCmd("Spawn", "Spawn Car", [](){
			if(spawnCarId == MI_CHOPPER ||
			   spawnCarId == MI_AIRTRAIN ||
			   spawnCarId == MI_DEADDODO)
				return;
			SpawnCar(spawnCarId);
		});
		static uint8 dummy;
		carCol1 = DebugMenuAddVar("Spawn", "First colour", &dummy, nil, 1, 0, 255, nil);
		carCol2 = DebugMenuAddVar("Spawn", "Second colour", &dummy, nil, 1, 0, 255, nil);
		DebugMenuAddCmd("Spawn", "Spawn Stinger", [](){ SpawnCar(MI_STINGER); });
		DebugMenuAddCmd("Spawn", "Spawn Infernus", [](){ SpawnCar(MI_INFERNUS); });
		DebugMenuAddCmd("Spawn", "Spawn Cheetah", [](){ SpawnCar(MI_CHEETAH); });
		DebugMenuAddCmd("Spawn", "Spawn Phoenix", [](){ SpawnCar(MI_PHEONIX); });
		DebugMenuAddCmd("Spawn", "Spawn Banshee", [](){ SpawnCar(MI_BANSHEE); });
		DebugMenuAddCmd("Spawn", "Spawn Esperanto", [](){ SpawnCar(MI_ESPERANT); });
		DebugMenuAddCmd("Spawn", "Spawn Stallion", [](){ SpawnCar(MI_STALLION); });
		DebugMenuAddCmd("Spawn", "Spawn Admiral", [](){ SpawnCar(MI_ADMIRAL); });
		DebugMenuAddCmd("Spawn", "Spawn Washington", [](){ SpawnCar(MI_WASHING); });
		DebugMenuAddCmd("Spawn", "Spawn Taxi", [](){ SpawnCar(MI_TAXI); });
		DebugMenuAddCmd("Spawn", "Spawn Police", [](){ SpawnCar(MI_POLICE); });
		DebugMenuAddCmd("Spawn", "Spawn Enforcer", [](){ SpawnCar(MI_ENFORCER); });
		DebugMenuAddCmd("Spawn", "Spawn Cuban", [](){ SpawnCar(MI_CUBAN); });
		DebugMenuAddCmd("Spawn", "Spawn Voodoo", [](){ SpawnCar(MI_VOODOO); });
		DebugMenuAddCmd("Spawn", "Spawn BF injection", [](){ SpawnCar(MI_BFINJECT); });
		DebugMenuAddCmd("Spawn", "Spawn Maverick", [](){ SpawnCar(MI_MAVERICK); });
		DebugMenuAddCmd("Spawn", "Spawn VCN Maverick", [](){ SpawnCar(MI_VCNMAV); });
		DebugMenuAddCmd("Spawn", "Spawn Sparrow", [](){ SpawnCar(MI_SPARROW); });
		DebugMenuAddCmd("Spawn", "Spawn Sea Sparrow", [](){ SpawnCar(MI_SEASPAR); });
		DebugMenuAddCmd("Spawn", "Spawn Hunter", [](){ SpawnCar(MI_HUNTER); });
		DebugMenuAddCmd("Spawn", "Spawn Rhino", [](){ SpawnCar(MI_RHINO); });
		DebugMenuAddCmd("Spawn", "Spawn Firetruck", [](){ SpawnCar(MI_FIRETRUCK); });
		DebugMenuAddCmd("Spawn", "Spawn Predator", [](){ SpawnCar(MI_PREDATOR); });
		DebugMenuAddCmd("Spawn", "Spawn PCJ 600", [](){ SpawnCar(MI_PCJ600); });
		DebugMenuAddCmd("Spawn", "Spawn Faggio", [](){ SpawnCar(MI_FAGGIO); });
		DebugMenuAddCmd("Spawn", "Spawn Freeway", [](){ SpawnCar(MI_FREEWAY); });

		DebugMenuAddVarBool8("Render", "Draw hud", &CHud::m_Wants_To_Draw_Hud, nil);
		DebugMenuAddVarBool8("Render", "Backface Culling", &gBackfaceCulling, nil);
#ifdef LIBRW
		DebugMenuAddVarBool8("Render", "PS2 Alpha test Emu", &gPS2alphaTest, nil);
#endif
		DebugMenuAddVarBool8("Render", "Frame limiter", &FrontEndMenuManager.m_PrefsFrameLimiter, nil);
		DebugMenuAddVarBool8("Render", "VSynch", &FrontEndMenuManager.m_PrefsVsync, nil);
		DebugMenuAddVar("Render", "Max FPS", &RsGlobal.maxFPS, nil, 1, 1, 1000, nil);
		DebugMenuAddVarBool8("Render", "Show Ped Paths", &gbShowPedPaths, nil);
		DebugMenuAddVarBool8("Render", "Show Car Paths", &gbShowCarPaths, nil);
		DebugMenuAddVarBool8("Render", "Show Car Path Links", &gbShowCarPathsLinks, nil);
		DebugMenuAddVarBool8("Render", "Show Collision Lines", &gbShowCollisionLines, nil);
		DebugMenuAddVarBool8("Render", "Show Collision Polys", &gbShowCollisionPolys, nil);
		DebugMenuAddVarBool8("Render", "Don't render Buildings", &gbDontRenderBuildings, nil);
		DebugMenuAddVarBool8("Render", "Don't render Big Buildings", &gbDontRenderBigBuildings, nil);
		DebugMenuAddVarBool8("Render", "Don't render Peds", &gbDontRenderPeds, nil);
		DebugMenuAddVarBool8("Render", "Don't render Vehicles", &gbDontRenderVehicles, nil);
		DebugMenuAddVarBool8("Render", "Don't render Objects", &gbDontRenderObjects, nil);
		DebugMenuAddVarBool8("Render", "Don't Render Water", &gbDontRenderWater, nil);

		DebugMenuAddVarBool8("Debug", "pad 1 -> pad 2", &CPad::m_bMapPadOneToPadTwo, nil);
		DebugMenuAddVarBool8("Debug", "Edit on", &CSceneEdit::m_bEditOn, nil);
#ifdef MENU_MAP
		DebugMenuAddCmd("Debug", "Teleport to map waypoint", TeleportToWaypoint);
#endif
		DebugMenuAddCmd("Debug", "Switch car collision", SwitchCarCollision);
		DebugMenuAddVar("Debug", "Engine Status", &engineStatus, nil, 1, 0, 226, nil);
		DebugMenuAddCmd("Debug", "Set Engine Status", SetEngineStatus);
		DebugMenuAddCmd("Debug", "Fix Car", FixCar);
		DebugMenuAddCmd("Debug", "Toggle Comedy Controls", ToggleComedy);
		DebugMenuAddCmd("Debug", "Place Car on Road", PlaceOnRoad);

		DebugMenuAddVarBool8("Debug", "Script Heli On", &CHeli::ScriptHeliOn, nil);

#ifdef CUSTOM_FRONTEND_OPTIONS
		DebugMenuAddCmd("Debug", "Reload custom frontend options", ReloadFrontendOptions);
#endif
		DebugMenuAddCmd("Debug", "Start Credits", CCredits::Start);
		DebugMenuAddCmd("Debug", "Stop Credits", CCredits::Stop);

#ifdef RELOADABLES
		DebugMenuAddCmd("Reload", "HUD.TXD", CHud::ReloadTXD);
#endif
		DebugMenuAddVarBool8("Debug", "Show DebugStuffInRelease", &gbDebugStuffInRelease, nil);
#ifdef TIMEBARS
		DebugMenuAddVarBool8("Debug", "Show Timebars", &gbShowTimebars, nil);
#endif

		extern bool PrintDebugCode;
		extern int16 DebugCamMode;
		DebugMenuAddVarBool8("Cam", "Use mouse Cam", &CCamera::m_bUseMouse3rdPerson, nil);
#ifdef FREE_CAM
		DebugMenuAddVarBool8("Cam", "Free Cam", &CCamera::bFreeCam, nil);
#endif
		DebugMenuAddVarBool8("Cam", "Print Debug Code", &PrintDebugCode, nil);
		DebugMenuAddVar("Cam", "Cam Mode", &DebugCamMode, nil, 1, 0, CCam::MODE_EDITOR, nil);
		DebugMenuAddCmd("Cam", "Normal", []() { DebugCamMode = 0; });
		DebugMenuAddCmd("Cam", "Follow Ped With Bind", []() { DebugCamMode = CCam::MODE_FOLLOW_PED_WITH_BIND; });
		DebugMenuAddCmd("Cam", "Reaction", []() { DebugCamMode = CCam::MODE_REACTION; });
		DebugMenuAddCmd("Cam", "Chris", []() { DebugCamMode = CCam::MODE_CHRIS; });
		DebugMenuAddCmd("Cam", "Reset Statics", ResetCamStatics);

		CTweakVars::AddDBG("Debug");
	}
}
#endif

const int   re3_buffsize = 1024;
static char re3_buff[re3_buffsize];

void re3_assert(const char *expr, const char *filename, unsigned int lineno, const char *func)
{
#ifdef _WIN32
	int nCode;

	strcpy_s(re3_buff, re3_buffsize, "Assertion failed!" );
	strcat_s(re3_buff, re3_buffsize, "\n" );	
	
	strcat_s(re3_buff, re3_buffsize, "File: ");
	strcat_s(re3_buff, re3_buffsize, filename );
	strcat_s(re3_buff, re3_buffsize, "\n" );	

	strcat_s(re3_buff, re3_buffsize, "Line: " );
	_itoa_s( lineno, re3_buff + strlen(re3_buff), re3_buffsize - strlen(re3_buff), 10 );
	strcat_s(re3_buff, re3_buffsize, "\n");
	
	strcat_s(re3_buff, re3_buffsize, "Function: ");
	strcat_s(re3_buff, re3_buffsize, func );
	strcat_s(re3_buff, re3_buffsize, "\n" );	
	
	strcat_s(re3_buff, re3_buffsize, "Expression: ");
	strcat_s(re3_buff, re3_buffsize, expr);
	strcat_s(re3_buff, re3_buffsize, "\n");

	strcat_s(re3_buff, re3_buffsize, "\n" );
	strcat_s(re3_buff, re3_buffsize, "(Press Retry to debug the application)");


	nCode = ::MessageBoxA(nil, re3_buff, "RE3 Assertion Failed!",
		MB_ABORTRETRYIGNORE|MB_ICONHAND|MB_SETFOREGROUND|MB_TASKMODAL);

	if (nCode == IDABORT)
	{
		raise(SIGABRT);
		_exit(3);
	}

	if (nCode == IDRETRY)
	{
		__debugbreak();
		return;
	}

	if (nCode == IDIGNORE)
		return;

	abort();
#else
	// TODO
	printf("\nRE3 ASSERT FAILED\n\tFile: %s\n\tLine: %d\n\tFunction: %s\n\tExpression: %s\n",filename,lineno,func,expr);
	assert(false);
#endif
}

void re3_debug(const char *format, ...)
{
	va_list va;
	va_start(va, format);
#ifdef _WIN32
	vsprintf_s(re3_buff, re3_buffsize, format, va);
#else
	vsprintf(re3_buff, format, va);
#endif
	va_end(va);

	printf("%s", re3_buff);
	CDebug::DebugAddText(re3_buff);
}

void re3_trace(const char *filename, unsigned int lineno, const char *func, const char *format, ...)
{
	char buff[re3_buffsize *2];
	va_list va;
	va_start(va, format);
#ifdef _WIN32
	vsprintf_s(re3_buff, re3_buffsize, format, va);
	va_end(va);
	
	sprintf_s(buff, re3_buffsize * 2, "[%s.%s:%d]: %s", filename, func, lineno, re3_buff);
#else
	vsprintf(re3_buff, format, va);
	va_end(va);
	
	sprintf(buff, "[%s.%s:%d]: %s", filename, func, lineno, re3_buff);
#endif

	OutputDebugString(buff);
}

void re3_usererror(const char *format, ...)
{
	va_list va;
	va_start(va, format);
#ifdef _WIN32
	vsprintf_s(re3_buff, re3_buffsize, format, va);
	va_end(va);
	
	::MessageBoxA(nil, re3_buff, "RE3 Error!",
		MB_OK|MB_ICONHAND|MB_SETFOREGROUND|MB_TASKMODAL);

	raise(SIGABRT);
	_exit(3);
#else
	vsprintf(re3_buff, format, va);
	printf("\nRE3 Error!\n\t%s\n",re3_buff);
	assert(false);
#endif
}

#ifdef VALIDATE_SAVE_SIZE
int32 _saveBufCount;
#endif
