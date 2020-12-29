#include <csignal>
#define WITHWINDOWS
#include "common.h"
#include "crossplatform.h"
#include "vehicle_enums.h" //needs to be before Vehicle.h

#include "Automobile.h"
#include "Bike.h"
#include "Boat.h"
#include "Camera.h"
#include "Clock.h"
#include "Console.h"
#include "Credits.h"
#include "Debug.h"
#include "FileMgr.h"
#include "Frontend.h"
#include "Heli.h"
#include "Hud.h"
#include "MBlur.h"
#include "MemoryHeap.h"
#include "ModelIndices.h"
#include "Occlusion.h"
#include "Pad.h"
#include "PlayerPed.h"
#include "Radar.h"
#include "Renderer.h"
#include "SceneEdit.h"
#include "Script.h"
#include "Streaming.h"
#include "Timecycle.h"
#include "Vehicle.h"
#include "WaterLevel.h"
#include "Weather.h"
#include "World.h"
#include "custompipes.h"
#include "debugmenu.h"
#include "main.h"
#include "postfx.h"

#ifdef DONT_TRUST_RECOGNIZED_JOYSTICKS
#include "ControllerConfig.h"
#endif

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

void
CustomFrontendOptionsPopulate(void)
{
	// Moved to an array in MenuScreensCustom.cpp, but APIs are still available. see frontendoption.h

	// These work only if we have neo folder, so they're dynamically added
#ifdef EXTENDED_PIPELINES
	const char *vehPipelineNames[] = { "FED_MFX", "FED_NEO" };
	const char *off_on[] = { "FEM_OFF", "FEM_ON" };
	int fd = CFileMgr::OpenFile("neo/neo.txd","r");
	if (fd) {
#ifdef GRAPHICS_MENU_OPTIONS
		FrontendOptionSetCursor(MENUPAGE_GRAPHICS_SETTINGS, -3, false);
		FrontendOptionAddSelect("FED_VPL", 0, 0, MENUALIGN_LEFT, vehPipelineNames, ARRAY_SIZE(vehPipelineNames), (int8*)&CustomPipes::VehiclePipeSwitch, false, nil, "VehiclePipeline");
		FrontendOptionAddSelect("FED_PRM", 0, 0, MENUALIGN_LEFT, off_on, 2, (int8*)&CustomPipes::RimlightEnable, false, nil, "NeoRimLight");
		FrontendOptionAddSelect("FED_WLM", 0, 0, MENUALIGN_LEFT, off_on, 2, (int8*)&CustomPipes::LightmapEnable, false, nil, "NeoLightMaps");
		FrontendOptionAddSelect("FED_RGL", 0, 0, MENUALIGN_LEFT, off_on, 2, (int8*)&CustomPipes::GlossEnable, false, nil, "NeoRoadGloss");
#else
		FrontendOptionSetCursor(MENUPAGE_DISPLAY_SETTINGS, -3, false);
		FrontendOptionAddSelect("FED_VPL", 0, 0, MENUALIGN_LEFT, vehPipelineNames, ARRAY_SIZE(vehPipelineNames), (int8*)&CustomPipes::VehiclePipeSwitch, false, nil, "VehiclePipeline");
		FrontendOptionAddSelect("FED_PRM", 0, 0, MENUALIGN_LEFT, off_on, 2, (int8*)&CustomPipes::RimlightEnable, false, nil, "NeoRimLight");
		FrontendOptionAddSelect("FED_WLM", 0, 0, MENUALIGN_LEFT, off_on, 2, (int8*)&CustomPipes::LightmapEnable, false, nil, "NeoLightMaps");
		FrontendOptionAddSelect("FED_RGL", 0, 0, MENUALIGN_LEFT, off_on, 2, (int8*)&CustomPipes::GlossEnable, false, nil, "NeoRoadGloss");
#endif
		CFileMgr::CloseFile(fd);
	}
#endif

}
#endif

#ifdef LOAD_INI_SETTINGS
#include "ini_parser.hpp"

linb::ini cfg;
int CheckAndReadIniInt(const char *cat, const char *key, int original)
{
	std::string strval = cfg.get(cat, key, "");
	const char *value = strval.c_str();
	if (value && value[0] != '\0')
		return atoi(value);

	return original;
}

float CheckAndReadIniFloat(const char *cat, const char *key, float original)
{
	std::string strval = cfg.get(cat, key, "");
	const char *value = strval.c_str();
	if (value && value[0] != '\0')
		return atof(value);

	return original;
}

void CheckAndSaveIniInt(const char *cat, const char *key, int val, bool &changed)
{
	char temp[10];
	if (atoi(cfg.get(cat, key, "xxx").c_str()) != val) { // if .ini doesn't have our key, compare with xxx and forcefully add it
		changed = true;
		sprintf(temp, "%u", val);
		cfg.set(cat, key, temp);
	}
}

void CheckAndSaveIniFloat(const char *cat, const char *key, float val, bool &changed)
{
	char temp[10];
	if (atof(cfg.get(cat, key, "xxx").c_str()) != val) { // if .ini doesn't have our key, compare with xxx and forcefully add it
		changed = true;
		sprintf(temp, "%f", val);
		cfg.set(cat, key, temp);
	}
}

void LoadINISettings()
{
	cfg.load_file("reVC.ini");

#ifdef DONT_TRUST_RECOGNIZED_JOYSTICKS
	// Written by assuming the codes below will run after _InputInitialiseJoys().
	strcpy(gSelectedJoystickName, cfg.get("DetectJoystick", "JoystickName", "").c_str());
	
	if(gSelectedJoystickName[0] != '\0') {
		for (int i = 0; i <= GLFW_JOYSTICK_LAST; i++) {
			if (glfwJoystickPresent(i) && strncmp(gSelectedJoystickName, glfwGetJoystickName(i), strlen(gSelectedJoystickName)) == 0) {
				if (PSGLOBAL(joy1id) != -1) {
					PSGLOBAL(joy2id) = PSGLOBAL(joy1id);
				}
				PSGLOBAL(joy1id) = i;
				int count;
				glfwGetJoystickButtons(PSGLOBAL(joy1id), &count);
				
				// We need to init and reload bindings, because;
				//	1-joypad button number may differ with saved/prvly connected one
				//	2-bindings are not init'ed if there is no joypad at the start
				ControlsManager.InitDefaultControlConfigJoyPad(count);
				CFileMgr::SetDirMyDocuments();
				int32 gta3set = CFileMgr::OpenFile("gta3.set", "r");
				if (gta3set) {
					ControlsManager.LoadSettings(gta3set);
					CFileMgr::CloseFile(gta3set);
				}
				CFileMgr::SetDir("");
				break;
			}
		}
	}
#endif

#ifdef CUSTOM_FRONTEND_OPTIONS
	for (int i = 0; i < MENUPAGES; i++) {
		for (int j = 0; j < NUM_MENUROWS; j++) {
			CMenuScreenCustom::CMenuEntry &option = aScreens[i].m_aEntries[j];
			if (option.m_Action == MENUACTION_NOTHING)
				break;
				
			// CFO check
			if (option.m_Action < MENUACTION_NOTHING && option.m_CFO->save) {
				// CFO only supports saving uint8 right now
				*option.m_CFO->value = CheckAndReadIniInt("FrontendOptions", option.m_CFO->save, *option.m_CFO->value);
				if (option.m_Action == MENUACTION_CFO_SELECT) {
					option.m_CFOSelect->lastSavedValue = option.m_CFOSelect->displayedValue = *option.m_CFO->value;
				}
			}
		}
	}
#endif

#ifdef EXTENDED_COLOURFILTER
	CPostFX::Intensity = CheckAndReadIniFloat("CustomPipesValues", "PostFXIntensity", CPostFX::Intensity);
#endif
#ifdef EXTENDED_PIPELINES
	CustomPipes::VehicleShininess = CheckAndReadIniFloat("CustomPipesValues", "NeoVehicleShininess", CustomPipes::VehicleShininess);
	CustomPipes::VehicleSpecularity = CheckAndReadIniFloat("CustomPipesValues", "NeoVehicleSpecularity", CustomPipes::VehicleSpecularity);
	CustomPipes::RimlightMult = CheckAndReadIniFloat("CustomPipesValues", "RimlightMult", CustomPipes::RimlightMult);
	CustomPipes::LightmapMult = CheckAndReadIniFloat("CustomPipesValues", "LightmapMult", CustomPipes::LightmapMult);
	CustomPipes::GlossMult = CheckAndReadIniFloat("CustomPipesValues", "GlossMult", CustomPipes::GlossMult);
#endif
	gBackfaceCulling = CheckAndReadIniInt("Rendering", "BackfaceCulling", gBackfaceCulling);
}

void SaveINISettings()
{
	bool changed = false;
	char temp[4];

#ifdef DONT_TRUST_RECOGNIZED_JOYSTICKS
	if (strncmp(cfg.get("DetectJoystick", "JoystickName", "").c_str(), gSelectedJoystickName, strlen(gSelectedJoystickName)) != 0) {
		changed = true;
		cfg.set("DetectJoystick", "JoystickName", gSelectedJoystickName);
	}
#endif
#ifdef CUSTOM_FRONTEND_OPTIONS
	for (int i = 0; i < MENUPAGES; i++) {
		for (int j = 0; j < NUM_MENUROWS; j++) {
			CMenuScreenCustom::CMenuEntry &option = aScreens[i].m_aEntries[j];
			if (option.m_Action == MENUACTION_NOTHING)
				break;
				
			if (option.m_Action < MENUACTION_NOTHING && option.m_CFO->save) {
				// Beware: CFO only supports saving uint8 right now
				CheckAndSaveIniInt("FrontendOptions", option.m_CFO->save, *option.m_CFO->value, changed);
			}
		}
	}
#endif

#ifdef EXTENDED_COLOURFILTER
	CheckAndSaveIniFloat("CustomPipesValues", "PostFXIntensity", CPostFX::Intensity, changed);
#endif
#ifdef EXTENDED_PIPELINES
	CheckAndSaveIniFloat("CustomPipesValues", "NeoVehicleShininess", CustomPipes::VehicleShininess, changed);
	CheckAndSaveIniFloat("CustomPipesValues", "NeoVehicleSpecularity", CustomPipes::VehicleSpecularity, changed);
	CheckAndSaveIniFloat("CustomPipesValues", "RimlightMult", CustomPipes::RimlightMult, changed);
	CheckAndSaveIniFloat("CustomPipesValues", "LightmapMult", CustomPipes::LightmapMult, changed);
	CheckAndSaveIniFloat("CustomPipesValues", "GlossMult", CustomPipes::GlossMult, changed);
#endif
	CheckAndSaveIniInt("Rendering", "BackfaceCulling", gBackfaceCulling, changed);

	if (changed)
		cfg.write_file("reVC.ini");
}

#endif

#ifdef DEBUGMENU
void WeaponCheat1();
void WeaponCheat2();
void WeaponCheat3();
void HealthCheat();
void VehicleCheat(int model);
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
void SpecialCarCheats();
void PickUpChicksCheat();

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
		else if(CModelInfo::IsBikeModel(id))
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

#ifdef MAP_ENHANCEMENTS
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

#ifdef MISSION_SWITCHER
int8 nextMissionToSwitch = 0;
static void
SwitchToMission(void)
{
	CTheScripts::SwitchToMission(nextMissionToSwitch);
}
#endif

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
		TweakVarsList = (CTweakVar**) realloc(TweakVarsList, (TweakVarsListSize + 1) * sizeof(*var));

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
switchWeather(void)
{
	CWeather::StreamAfterRainTimer = 0;
}

void
DebugMenuPopulate(void)
{
	if(1){
		static const char *weathers[] = {
			"Sunny", "Cloudy", "Rainy", "Foggy", "Extrasunny", "Stormy"
		};
		static const char *extracols[] = {
			"1 - Malibu club",
			"2 - Strib club",
			"3 - Hotel",
			"4 - Bank",
			"5 - Police HQ",
			"6 - Mall",
			"7 - Rifle Range",
			"8 - Mansion",
			"9 - Dirt ring",
			"10 - Blood ring",
			"11 - Hot ring",
			"12 - Concert hall",
			"13 - Auntie Poulets",
			"14 - Intro at docks",
			"15 - Biker bar",
			"16 - Intro cafe",
			"17 - Studio",
			"18", "19", "20", "21", "22", "23", "24"
		};
		DebugMenuEntry *e;
		e = DebugMenuAddVar("Time & Weather", "Current Hour", &CClock::GetHoursRef(), nil, 1, 0, 23, nil);
		DebugMenuEntrySetWrap(e, true);
		e = DebugMenuAddVar("Time & Weather", "Current Minute", &CClock::GetMinutesRef(),
			[](){ CWeather::InterpolationValue = CClock::GetMinutes()/60.0f; }, 1, 0, 59, nil);
			DebugMenuEntrySetWrap(e, true);
		e = DebugMenuAddVar("Time & Weather", "Old Weather", (int16*)&CWeather::OldWeatherType, switchWeather, 1, 0, 5, weathers);
		DebugMenuEntrySetWrap(e, true);
		e = DebugMenuAddVar("Time & Weather", "New Weather", (int16*)&CWeather::NewWeatherType, switchWeather, 1, 0, 5, weathers);
		DebugMenuEntrySetWrap(e, true);
		DebugMenuAddVarBool32("Time & Weather", "Extracolours On", &CTimeCycle::m_bExtraColourOn, nil);
		DebugMenuAddVar("Time & Weather", "Extracolour", &CTimeCycle::m_ExtraColour, nil, 1, 0, 23, extracols);
		DebugMenuAddVar("Time & Weather", "Time scale", (float*)&CTimer::GetTimeScale(), nil, 0.1f, 0.0f, 10.0f);

		DebugMenuAddCmd("Cheats", "Weapon set 1", WeaponCheat1);
		DebugMenuAddCmd("Cheats", "Weapon set 2", WeaponCheat2);
		DebugMenuAddCmd("Cheats", "Weapon set 3", WeaponCheat3);
		DebugMenuAddCmd("Cheats", "Money", MoneyCheat);
		DebugMenuAddCmd("Cheats", "Health", HealthCheat);
		DebugMenuAddCmd("Cheats", "Wanted level up", WantedLevelUpCheat);
		DebugMenuAddCmd("Cheats", "Wanted level down", WantedLevelDownCheat);
		DebugMenuAddCmd("Cheats", "Tank", []() { VehicleCheat(MI_TAXI); });
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
		DebugMenuAddCmd("Cheats", "Special car", SpecialCarCheats);
		DebugMenuAddCmd("Cheats", "Pickup chicks", PickUpChicksCheat);

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
		DebugMenuAddCmd("Spawn", "Spawn Squalo", [](){ SpawnCar(MI_SQUALO); });
		DebugMenuAddCmd("Spawn", "Spawn Skimmer", [](){ SpawnCar(MI_SKIMMER); });

		DebugMenuAddVarBool8("Render", "Draw hud", &CHud::m_Wants_To_Draw_Hud, nil);
		DebugMenuAddVarBool8("Render", "Backface Culling", &gBackfaceCulling, nil);
		DebugMenuAddVarBool8("Render", "PS2 Alpha test Emu", &gPS2alphaTest, nil);
		DebugMenuAddVarBool8("Render", "Frame limiter", &FrontEndMenuManager.m_PrefsFrameLimiter, nil);
		DebugMenuAddVarBool8("Render", "VSynch", &FrontEndMenuManager.m_PrefsVsync, nil);
		DebugMenuAddVar("Render", "Max FPS", &RsGlobal.maxFPS, nil, 1, 1, 1000, nil);
#ifdef NEW_RENDERER
		DebugMenuAddVarBool8("Render", "new renderer", &gbNewRenderer, nil);
extern bool gbRenderRoads;
extern bool gbRenderEverythingBarRoads;
extern bool gbRenderFadingInUnderwaterEntities;
extern bool gbRenderFadingInEntities;
extern bool gbRenderWater;
extern bool gbRenderBoats;
extern bool gbRenderVehicles;
extern bool gbRenderWorld0;
extern bool gbRenderWorld1;
extern bool gbRenderWorld2;
		DebugMenuAddVarBool8("Render", "gbRenderRoads", &gbRenderRoads, nil);
		DebugMenuAddVarBool8("Render", "gbRenderEverythingBarRoads", &gbRenderEverythingBarRoads, nil);
		DebugMenuAddVarBool8("Render", "gbRenderFadingInUnderwaterEntities", &gbRenderFadingInUnderwaterEntities, nil);
		DebugMenuAddVarBool8("Render", "gbRenderFadingInEntities", &gbRenderFadingInEntities, nil);
		DebugMenuAddVarBool8("Render", "gbRenderWater", &gbRenderWater, nil);
		DebugMenuAddVarBool8("Render", "gbRenderBoats", &gbRenderBoats, nil);
		DebugMenuAddVarBool8("Render", "gbRenderVehicles", &gbRenderVehicles, nil);
		DebugMenuAddVarBool8("Render", "gbRenderWorld0", &gbRenderWorld0, nil);
		DebugMenuAddVarBool8("Render", "gbRenderWorld1", &gbRenderWorld1, nil);
		DebugMenuAddVarBool8("Render", "gbRenderWorld2", &gbRenderWorld2, nil);
#endif

#ifdef EXTENDED_COLOURFILTER
		static const char *filternames[] = { "None", "Simple", "Normal", "Mobile" };
		e = DebugMenuAddVar("Render", "Colourfilter", &CPostFX::EffectSwitch, nil, 1, CPostFX::POSTFX_OFF, CPostFX::POSTFX_MOBILE, filternames);
		DebugMenuEntrySetWrap(e, true);
		DebugMenuAddVar("Render", "Intensity", &CPostFX::Intensity, nil, 0.05f, 0, 10.0f);
		DebugMenuAddVarBool8("Render", "Blur", &CPostFX::BlurOn, nil);
		DebugMenuAddVarBool8("Render", "Motion Blur", &CPostFX::MotionBlurOn, nil);
#endif
		DebugMenuAddVar("Render", "Drunkness", &CMBlur::Drunkness, nil, 0.05f, 0, 1.0f);
#ifndef MASTER
		DebugMenuAddVarBool8("Render", "Occlusion debug", &bDisplayOccDebugStuff, nil);
#endif
#ifdef EXTENDED_PIPELINES
		static const char *vehpipenames[] = { "MatFX", "Neo" };
		e = DebugMenuAddVar("Render", "Vehicle Pipeline", &CustomPipes::VehiclePipeSwitch, nil,
			1, CustomPipes::VEHICLEPIPE_MATFX, CustomPipes::VEHICLEPIPE_NEO, vehpipenames);
		DebugMenuEntrySetWrap(e, true);
		DebugMenuAddVar("Render", "Neo Vehicle Shininess", &CustomPipes::VehicleShininess, nil, 0.1f, 0, 1.0f);
		DebugMenuAddVar("Render", "Neo Vehicle Specularity", &CustomPipes::VehicleSpecularity, nil, 0.1f, 0, 1.0f);
		DebugMenuAddVarBool8("Render", "Neo Ped Rim light enable", &CustomPipes::RimlightEnable, nil);
		DebugMenuAddVar("Render", "Mult", &CustomPipes::RimlightMult, nil, 0.1f, 0, 1.0f);
		DebugMenuAddVarBool8("Render", "Neo World Lightmaps enable", &CustomPipes::LightmapEnable, nil);
		DebugMenuAddVar("Render", "Mult", &CustomPipes::LightmapMult, nil, 0.1f, 0, 1.0f);
		DebugMenuAddVarBool8("Render", "Neo Road Gloss enable", &CustomPipes::GlossEnable, nil);
		DebugMenuAddVar("Render", "Mult", &CustomPipes::GlossMult, nil, 0.1f, 0, 1.0f);
#endif
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

#ifndef FINAL
		DebugMenuAddVarBool8("Debug", "Print Memory Usage", &gbPrintMemoryUsage, nil);
#ifdef USE_CUSTOM_ALLOCATOR
		DebugMenuAddCmd("Debug", "Parse Heap", ParseHeap);
#endif
#endif

		DebugMenuAddVarBool8("Debug", "pad 1 -> pad 2", &CPad::m_bMapPadOneToPadTwo, nil);
#ifdef GTA_SCENE_EDIT
		DebugMenuAddVarBool8("Debug", "Edit on", &CSceneEdit::m_bEditOn, nil);
#endif
#ifdef MAP_ENHANCEMENTS
		DebugMenuAddCmd("Debug", "Teleport to map waypoint", TeleportToWaypoint);
#endif
		DebugMenuAddCmd("Debug", "Switch car collision", SwitchCarCollision);
		DebugMenuAddVar("Debug", "Engine Status", &engineStatus, nil, 1, 0, 226, nil);
		DebugMenuAddCmd("Debug", "Set Engine Status", SetEngineStatus);
		DebugMenuAddCmd("Debug", "Fix Car", FixCar);
		DebugMenuAddCmd("Debug", "Toggle Comedy Controls", ToggleComedy);
		DebugMenuAddCmd("Debug", "Place Car on Road", PlaceOnRoad);

		DebugMenuAddVarBool8("Debug", "Script Heli On", &CHeli::ScriptHeliOn, nil);

		DebugMenuAddCmd("Debug", "Start Credits", CCredits::Start);
		DebugMenuAddCmd("Debug", "Stop Credits", CCredits::Stop);

#ifdef RELOADABLES
		DebugMenuAddCmd("Reload", "HUD.TXD", CHud::ReloadTXD);
#endif
		DebugMenuAddVarBool8("Debug", "Show DebugStuffInRelease", &gbDebugStuffInRelease, nil);
#ifdef TIMEBARS
		DebugMenuAddVarBool8("Debug", "Show Timebars", &gbShowTimebars, nil);
#endif
#ifdef MISSION_SWITCHER
		DebugMenuEntry *missionEntry;
		static const char* missions[] = {
			"Initial", "Intro", "An Old Friend", "The Party", "Back Alley Brawl", "Jury Fury", "Riot",
			"Treacherous Swine", "Mall Shootout", "Guardian Angels", "Sir, Yes Sir!", "All Hands On Deck!",
			"The Chase", "Phnom Penh '86", "The Fastest Boat", "Supply & Demand", "Rub Out", "Death Row",
			"Four Iron", "Demolition Man", "Two Bit Hit", "No Escape?", "The Shootist", "The Driver",
			"The Job", "Gun Runner", "Boomshine Saigon", "Recruitment Drive", "Dildo Dodo", "Martha's Mug Shot",
			"G-spotlight", "Shakedown", "Bar Brawl", "Cop Land", "Spilling the Beans", "Hit the Courier",
			"Printworks Buy", "Sunshine Autos", "Interglobal Films Buy", "Cherry Popper Icecreams Buy",
			"Kaufman Cabs Buy", "Malibu Club Buy", "The Boatyard Buy", "Pole Position Club Buy", "El Swanko Casa Buy",
			"Links View Apartment Buy", "Hyman Condo Buy", "Ocean Heighs Aprt. Buy", "1102 Washington Street Buy",
			"Vice Point Buy", "Skumole Shack Buy", "Cap the Collector", "Keep your Friends Close...",
			"Alloy Wheels of Steel", "Messing with the Man", "Hog Tied", "Stunt Boat Challenge", "Cannon Fodder",
			"Naval Engagement", "Trojan Voodoo", "Juju Scramble", "Bombs Away!", "Dirty Lickin's", "Love Juice",
			"Psycho Killer", "Publicity Tour", "Weapon Range", "Road Kill", "Waste the Wife", "Autocide",
			"Check Out at the Check In", "Loose Ends", "V.I.P.", "Friendly Rivalry", "Cabmaggedon", "TAXI DRIVER",
			"PARAMEDIC", "FIREFIGHTER", "VIGILANTE", "HOTRING", "BLOODRING", "DIRTRING", "Sunshine Autos Races",
			"Distribution", "Downtown Chopper Checkpoint", "Ocean Beach Chopper Checkpoint", "Vice Point Chopper Checkpoint",
			"Little Haiti Chopper Checkpoint", "Trial by Dirt", "Test Track", "PCJ Playground", "Cone Crazy",
			"PIZZA BOY", "RC Raider Pickup", "RC Bandit Race", "RC Baron Race", "Checkpoint Charlie"
		};

		missionEntry = DebugMenuAddVar("Debug", "Select mission", &nextMissionToSwitch, nil, 1, 0, 96, missions);
		DebugMenuEntrySetWrap(missionEntry, true);
		DebugMenuAddCmd("Debug", "Start selected mission ", SwitchToMission);
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
