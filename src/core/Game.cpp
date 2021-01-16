#include "common.h"
#include "platform.h"

#include "Game.h"
#include "main.h"
#include "RwHelper.h"
#include "Accident.h"
#include "Antennas.h"
#include "Bridge.h"
#include "CarCtrl.h"
#include "CarGen.h"
#include "CdStream.h"
#include "Clock.h"
#include "Clouds.h"
#include "Collision.h"
#include "Console.h"
#include "Coronas.h"
#include "Cranes.h"
#include "Credits.h"
#include "CutsceneMgr.h"
#include "DMAudio.h"
#include "Darkel.h"
#include "Debug.h"
#include "EventList.h"
#include "FileLoader.h"
#include "FileMgr.h"
#include "Fire.h"
#include "Fluff.h"
#include "Font.h"
#include "Frontend.h"
#include "frontendoption.h"
#include "GameLogic.h"
#include "Garages.h"
#include "GenericGameStorage.h"
#include "Glass.h"
#include "HandlingMgr.h"
#include "Heli.h"
#include "Hud.h"
#include "IniFile.h"
#include "Lights.h"
#include "MBlur.h"
#include "Messages.h"
#include "MemoryCard.h"
#include "Pad.h"
#include "Particle.h"
#include "ParticleObject.h"
#include "PedRoutes.h"
#include "Phones.h"
#include "Pickups.h"
#include "Plane.h"
#include "PlayerSkin.h"
#include "Population.h"
#include "Radar.h"
#include "Record.h"
#include "References.h"
#include "Renderer.h"
#include "Replay.h"
#include "Restart.h"
#include "RoadBlocks.h"
#include "Rubbish.h"
#include "SceneEdit.h"
#include "Script.h"
#include "Shadows.h"
#include "Skidmarks.h"
#include "SpecialFX.h"
#include "Stats.h"
#include "Streaming.h"
#include "SurfaceTable.h"
#include "TempColModels.h"
#include "Timecycle.h"
#include "TrafficLights.h"
#include "Train.h"
#include "TxdStore.h"
#include "User.h"
#include "VisibilityPlugins.h"
#include "WaterCannon.h"
#include "WaterLevel.h"
#include "Weapon.h"
#include "WeaponEffects.h"
#include "Weather.h"
#include "World.h"
#include "ZoneCull.h"
#include "Zones.h"
#include "debugmenu.h"
#include "postfx.h"
#include "custompipes.h"
#include "screendroplets.h"
#include "crossplatform.h"
#include "MemoryHeap.h"
#ifdef USE_TEXTURE_POOL
#include "TexturePools.h"
#endif

eLevelName CGame::currLevel;
bool CGame::bDemoMode = true;
bool CGame::nastyGame = true;
bool CGame::frenchGame;
bool CGame::germanGame;
bool CGame::noProstitutes;
bool CGame::playingIntro;
char CGame::aDatFile[32];
#ifdef MORE_LANGUAGES
bool CGame::russianGame = false;
bool CGame::japaneseGame = false;
#endif

int gameTxdSlot;


bool DoRWStuffStartOfFrame(int16 TopRed, int16 TopGreen, int16 TopBlue, int16 BottomRed, int16 BottomGreen, int16 BottomBlue, int16 Alpha);
void DoRWStuffEndOfFrame(void);
#ifdef PS2_MENU
void MessageScreen(char *msg)
{
	CRect rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	CRGBA color(255, 255, 255, 255);

	DoRWStuffStartOfFrame(50, 50, 50, 0, 0, 0, 255);
	
	CSprite2d::InitPerFrame();
	CFont::InitPerFrame();
	DefinedState();

	CSprite2d *splash = LoadSplash(NULL);
	splash->Draw(rect, color, color, color, color);
#ifdef FIX_BUGS
	splash->DrawRect(CRect(SCREEN_SCALE_X(20.0f), SCREEN_SCALE_Y(110.0f), SCREEN_WIDTH-SCREEN_SCALE_X(20.0f), SCREEN_SCALE_Y(300.0f)), CRGBA(50, 50, 50, 192));
#else
	splash->DrawRect(CRect(20.0f, 110.0f, SCREEN_WIDTH-20.0f, 300.0f), CRGBA(50, 50, 50, 192));
#endif
	CFont::SetFontStyle(FONT_BANK);
	CFont::SetBackgroundOff();
	CFont::SetWrapx(SCREEN_SCALE_FROM_RIGHT(190));
#ifdef FIX_BUGS
	CFont::SetScale(SCREEN_SCALE_X(1.0f), SCREEN_SCALE_Y(1.0f));
#else
	CFont::SetScale(1.0f, 1.0f);
#endif
	CFont::SetCentreOn();
	CFont::SetCentreSize(SCREEN_SCALE_X(DEFAULT_SCREEN_WIDTH - 190)); // 450.0f
	CFont::SetJustifyOff();
	CFont::SetColor(CRGBA(255, 255, 255, 255));
	CFont::SetDropColor(CRGBA(32, 32, 32, 255));
	CFont::SetDropShadowPosition(3);
	CFont::SetBackGroundOnlyTextOff();
	CFont::SetPropOn();
#ifdef FIX_BUGS
	CFont::PrintString(SCREEN_WIDTH/2, SCREEN_SCALE_Y(130.0f), TheText.Get(msg));
#else
	CFont::PrintString(SCREEN_WIDTH/2, 130.0f, TheText.Get(msg));
#endif
	CFont::DrawFonts();
	
	DoRWStuffEndOfFrame();
}
#endif

bool
CGame::InitialiseOnceBeforeRW(void)
{
	CFileMgr::Initialise();
	CdStreamInit(MAX_CDCHANNELS);
	ValidateVersion();
#ifdef EXTENDED_COLOURFILTER
	CPostFX::InitOnce();
#endif
#ifdef CUSTOM_FRONTEND_OPTIONS
	// Not needed here but may be needed in future
	// if (numCustomFrontendOptions == 0 && numCustomFrontendScreens == 0)
	CustomFrontendOptionsPopulate();
#endif
	return true;
}

#ifndef LIBRW
#ifdef PS2_MATFX
void ReplaceMatFxCallback();
#endif // PS2_MATFX
#ifdef PS2_ALPHA_TEST
void ReplaceAtomicPipeCallback();
#endif // PS2_ALPHA_TEST
#endif // !LIBRW

bool
CGame::InitialiseRenderWare(void)
{
#ifdef USE_TEXTURE_POOL
	_TexturePoolsInitialise();
#endif

#if GTA_VERSION > GTA3_PS2_160
	CTxdStore::Initialise();	// in GameInit on ps2
	CVisibilityPlugins::Initialise();	// in plugin attach on ps2
#endif

	//InitialiseScene(Scene);	// PS2 only, only clears Scene.camera

#ifdef GTA_PS2
	RpSkySelectTrueTSClipper(TRUE);
	RpSkySelectTrueTLClipper(TRUE);

	// PS2ManagerApplyDirectionalLightingCB() uploads the GTA lights
	// directly without going through RpWorld and all that
	SetupPS2ManagerDefaultLightingCallback();
	PreAllocateRwObjects();
#endif

	/* Create camera */
	Scene.camera = CameraCreate(SCREEN_WIDTH, SCREEN_HEIGHT, TRUE);
	ASSERT(Scene.camera != nil);
	if (!Scene.camera)
	{
		return (false);
	}
	
	RwCameraSetFarClipPlane(Scene.camera, 2000.0f);	// 250.0f on PS2 but who cares
	RwCameraSetNearClipPlane(Scene.camera, 0.9f);
	
	CameraSize(Scene.camera, nil, DEFAULT_VIEWWINDOW, DEFAULT_ASPECT_RATIO);
	
	/* Create a world */
	RwBBox  bbox;
	
	bbox.sup.x = bbox.sup.y = bbox.sup.z = 10000.0f;
	bbox.inf.x = bbox.inf.y = bbox.inf.z = -10000.0f;

	Scene.world = RpWorldCreate(&bbox);
	ASSERT(Scene.world != nil);
	if (!Scene.world)
	{
		CameraDestroy(Scene.camera);
		Scene.camera = nil;
		return (false);
	}
	
	/* Add the camera to the world */
	RpWorldAddCamera(Scene.world, Scene.camera);
	LightsCreate(Scene.world);

#if GTA_VERSION > GTA3_PS2_160
	CreateDebugFont();	// in GameInit on PS2
#else
	RwImageSetPath("textures");
#endif

#ifdef LIBRW
#ifdef PS2_MATFX
	rw::MatFX::modulateEnvMap = true;
#else
	rw::MatFX::modulateEnvMap = false;
#endif
#else
#ifdef PS2_MATFX
	ReplaceMatFxCallback();
#endif // PS2_MATFX
#ifdef PS2_ALPHA_TEST
	ReplaceAtomicPipeCallback();
#endif // PS2_ALPHA_TEST
#endif // LIBRW


#if GTA_VERSION > GTA3_PS2_160
	// in GameInit on PS2
	PUSH_MEMID(MEMID_TEXTURES);
	CFont::Initialise();
	CHud::Initialise();
	POP_MEMID();
	// TODO: define
	CPlayerSkin::Initialise();
#endif
	
#ifdef EXTENDED_PIPELINES
	CustomPipes::CustomPipeInit();	// need Scene.world for this
#endif
#ifdef SCREEN_DROPLETS
	ScreenDroplets::InitDraw();
#endif

	return (true);
}

// missing altogether on PS2
void CGame::ShutdownRenderWare(void)
{
#ifdef SCREEN_DROPLETS
	ScreenDroplets::Shutdown();
#endif
#ifdef EXTENDED_PIPELINES
	CustomPipes::CustomPipeShutdown();
#endif

	CMBlur::MotionBlurClose();
	DestroySplashScreen();
	CHud::Shutdown();
	CFont::Shutdown();
	
	for ( int32 i = 0; i < NUMPLAYERS; i++ )
		CWorld::Players[i].DeletePlayerSkin();

	// TODO: define
	CPlayerSkin::Shutdown();
	
	DestroyDebugFont();
	
	/* Destroy world */
	LightsDestroy(Scene.world);
	RpWorldRemoveCamera(Scene.world, Scene.camera);
	RpWorldDestroy(Scene.world);
	
	/* destroy camera */
	CameraDestroy(Scene.camera);
	
	Scene.world = nil;
	Scene.camera = nil;
	
	CVisibilityPlugins::Shutdown();
	
#ifdef USE_TEXTURE_POOL
	_TexturePoolsShutdown();
#endif
}

// missing altogether on PS2
bool CGame::InitialiseOnceAfterRW(void)
{
#if GTA_VERSION > GTA3_PS2_160
	TheText.Load();
	DMAudio.Initialise();	// before TheGame() on PS2
	CTimer::Initialise();
	CTempColModels::Initialise();
	mod_HandlingManager.Initialise();
	CSurfaceTable::Initialise("DATA\\SURFACE.DAT");
	CPedStats::Initialise();
	CTimeCycle::Initialise();

#ifndef GTA_PS2
	if ( DMAudio.GetNum3DProvidersAvailable() == 0 )
		FrontEndMenuManager.m_nPrefsAudio3DProviderIndex = -1;

	if ( FrontEndMenuManager.m_nPrefsAudio3DProviderIndex == -99 || FrontEndMenuManager.m_nPrefsAudio3DProviderIndex == -2 ) {
		CMenuManager::m_PrefsSpeakers = 0;
		int32 i;
		for (i = 0; i < DMAudio.GetNum3DProvidersAvailable(); i++) {
			wchar buff[64];

#ifdef AUDIO_OAL
			extern int defaultProvider;
			if (defaultProvider >= 0 && defaultProvider < DMAudio.GetNum3DProvidersAvailable()) 
				break;
#endif
			char *name = DMAudio.Get3DProviderName(i);
			AsciiToUnicode(name, buff);
			char *providername = UnicodeToAscii(buff);
			strupr(providername);
#if defined(AUDIO_MSS)
			if (strcmp(providername, "MILES FAST 2D POSITIONAL AUDIO") == 0)
				break;
#elif defined(AUDIO_OAL)
			if (strcmp(providername, "OPENAL SOFT") == 0)
				break;
#endif
		}

		FrontEndMenuManager.m_nPrefsAudio3DProviderIndex = i;
	}

	DMAudio.SetCurrent3DProvider(FrontEndMenuManager.m_nPrefsAudio3DProviderIndex);
	DMAudio.SetSpeakerConfig(CMenuManager::m_PrefsSpeakers);
	DMAudio.SetDynamicAcousticModelingStatus(CMenuManager::m_PrefsDMA);
	DMAudio.SetMusicMasterVolume(CMenuManager::m_PrefsMusicVolume);
	DMAudio.SetEffectsMasterVolume(CMenuManager::m_PrefsSfxVolume);
	DMAudio.SetEffectsFadeVol(127);
	DMAudio.SetMusicFadeVol(127);
#endif
	CWorld::Players[0].SetPlayerSkin(CMenuManager::m_PrefsSkinFile);
#endif
	return true;
}

// missing altogether on PS2
void
CGame::FinalShutdown(void)
{	
	CTxdStore::Shutdown();
	CPedStats::Shutdown();
	CdStreamShutdown();
}

#if GTA_VERSION <= GTA3_PS2_160
bool CGame::Initialise(void)
#else
bool CGame::Initialise(const char* datFile)
#endif
{
#ifdef GTA_PS2
	// TODO: upload VU0 collision code here
#endif

#if GTA_VERSION > GTA3_PS2_160
	ResetLoadingScreenBar();
	strcpy(aDatFile, datFile);
	CPools::Initialise();	// done in CWorld on PS2
#endif

#ifndef GTA_PS2
	CIniFile::LoadIniFile();
#endif

	currLevel = LEVEL_INDUSTRIAL;

	PUSH_MEMID(MEMID_TEXTURES);
	LoadingScreen("Loading the Game", "Loading generic textures", GetRandomSplashScreen());
	gameTxdSlot = CTxdStore::AddTxdSlot("generic");
	CTxdStore::Create(gameTxdSlot);
	CTxdStore::AddRef(gameTxdSlot);

	LoadingScreen("Loading the Game", "Loading particles", nil);
	int particleTxdSlot = CTxdStore::AddTxdSlot("particle");
	CTxdStore::LoadTxd(particleTxdSlot, "MODELS/PARTICLE.TXD");
	CTxdStore::AddRef(particleTxdSlot);
	CTxdStore::SetCurrentTxd(gameTxdSlot);
	LoadingScreen("Loading the Game", "Setup game variables", nil);
	POP_MEMID();

#ifdef GTA_PS2
	CDma::SyncChannel(0, true);
#endif

	CGameLogic::InitAtStartOfGame();
	CReferences::Init();
	TheCamera.Init();
	TheCamera.SetRwCamera(Scene.camera);
	CDebug::DebugInitTextBuffer();
	ThePaths.Init();
	ThePaths.AllocatePathFindInfoMem(4500);
	CWeather::Init();
	CCullZones::Init();
	CCollision::Init();
#ifdef PS2_MENU	// TODO: is this the right define?
	TheText.Load();
#endif
	CTheZones::Init();
	CUserDisplay::Init();
	CMessages::Init();
#if GTA_VERSION > GTA3_PS2_160
	CMessages::ClearAllMessagesDisplayedByGame();
#endif
	CRecordDataForGame::Init();
	CRestart::Initialise();

	PUSH_MEMID(MEMID_WORLD);
	CWorld::Initialise();
	POP_MEMID();

#if GTA_VERSION <= GTA3_PS2_160
	mod_HandlingManager.Initialise();
	CSurfaceTable::Initialise("DATA\\SURFACE.DAT");
	CTempColModels::Initialise();
#endif

	PUSH_MEMID(MEMID_TEXTURES);
	CParticle::Initialise();
	POP_MEMID();

#if GTA_VERSION <= GTA3_PS2_160
	gStartX = -180.0f;
	gStartY = 180.0f;
	gStartZ = 14.0f;
#endif

	PUSH_MEMID(MEMID_ANIMATION);
	CAnimManager::Initialise();
	CCutsceneMgr::Initialise();
	POP_MEMID();

	PUSH_MEMID(MEMID_CARS);
	CCarCtrl::Init();
	POP_MEMID();

	PUSH_MEMID(MEMID_DEF_MODELS);
#if GTA_VERSION > GTA3_PS2_160
	InitModelIndices();
#endif
	CModelInfo::Initialise();

#if GTA_VERSION > GTA3_PS2_160
	// probably moved before LoadLevel for multiplayer maps?
	CPickups::Init();
	CTheCarGenerators::Init();

	CdStreamAddImage("MODELS\\GTA3.IMG");

	CFileLoader::LoadLevel("DATA\\DEFAULT.DAT");
	CFileLoader::LoadLevel(datFile);
#else
	CPedStats::Initialise();	// InitialiseOnceAfterRW

	CFileLoader::LoadLevel("GTA3.DAT");
#endif

#ifdef EXTENDED_PIPELINES
	// for generic fallback
	CustomPipes::SetTxdFindCallback();
#endif
	CWorld::AddParticles();
	CVehicleModelInfo::LoadVehicleColours();
	CVehicleModelInfo::LoadEnvironmentMaps();
	CTheZones::PostZoneCreation();
	POP_MEMID();

#if GTA_VERSION <= GTA3_PS2_160
	TestModelIndices();
#endif
	LoadingScreen("Loading the Game", "Setup paths", GetRandomSplashScreen());
	ThePaths.PreparePathData();
#if GTA_VERSION > GTA3_PS2_160
	for (int i = 0; i < NUMPLAYERS; i++)
		CWorld::Players[i].Clear();
	CWorld::Players[0].LoadPlayerSkin();
	TestModelIndices();
#endif

	LoadingScreen("Loading the Game", "Setup water", nil);
	CWaterLevel::Initialise("DATA\\WATER.DAT");
#if GTA_VERSION <= GTA3_PS2_160
	CTimeCycle::Initialise();	// InitialiseOnceAfterRW
#else
	TheConsole.Init();
#endif
	CDraw::SetFOV(120.0f);
	CDraw::ms_fLODDistance = 500.0f;

	LoadingScreen("Loading the Game", "Setup streaming", nil);
	CStreaming::Init();
	CStreaming::LoadInitialVehicles();
	CStreaming::LoadInitialPeds();
	CStreaming::RequestBigBuildings(LEVEL_GENERIC);
	CStreaming::LoadAllRequestedModels(false);
#if GTA_VERSION > GTA3_PS2_160
	printf("Streaming uses %zuK of its memory", CStreaming::ms_memoryUsed / 1024); // original modifier was %d
#endif

	LoadingScreen("Loading the Game", "Load animations", GetRandomSplashScreen());
	PUSH_MEMID(MEMID_ANIMATION);
	CAnimManager::LoadAnimFiles();
	POP_MEMID();

	CPed::Initialise();
	CRouteNode::Initialise();
	CEventList::Initialise();
#ifdef SCREEN_DROPLETS
	ScreenDroplets::Initialise();
#endif
	LoadingScreen("Loading the Game", "Find big buildings", nil);
	CRenderer::Init();

	LoadingScreen("Loading the Game", "Setup game variables", nil);
	CRadar::Initialise();
	CRadar::LoadTextures();
	CWeapon::InitialiseWeapons();

	LoadingScreen("Loading the Game", "Setup traffic lights", nil);
	CTrafficLights::ScanForLightsOnMap();
	CRoadBlocks::Init();

	LoadingScreen("Loading the Game", "Setup game variables", nil);
	CPopulation::Initialise();
#if GTA_VERSION <= GTA3_PS2_160
	for (int i = 0; i < NUMPLAYERS; i++)
		CWorld::Players[i].Clear();
//	CWorld::Players[0].LoadPlayerSkin();	// TODO: use a define for this
#endif
	CWorld::PlayerInFocus = 0;
	CCoronas::Init();
	CShadows::Init();
	CWeaponEffects::Init();
	CSkidmarks::Init();
	CAntennas::Init();
	CGlass::Init();
	gPhoneInfo.Initialise();
#ifdef GTA_SCENE_EDIT
	CSceneEdit::Initialise();
#endif

	LoadingScreen("Loading the Game", "Load scripts", nil);
	PUSH_MEMID(MEMID_SCRIPT);
	CTheScripts::Init();
	CGangs::Initialise();
	POP_MEMID();

	LoadingScreen("Loading the Game", "Setup game variables", nil);
#if GTA_VERSION <= GTA3_PS2_160
	CTimer::Initialise();
#endif
	CClock::Initialise(1000);
#if GTA_VERSION <= GTA3_PS2_160
	CTheCarGenerators::Init();
#endif
	CHeli::InitHelis();
	CCranes::InitCranes();
	CMovingThings::Init();
	CDarkel::Init();
	CStats::Init();
#if GTA_VERSION <= GTA3_PS2_160
	CPickups::Init();
#endif
	CPacManPickups::Init();
#if GTA_VERSION <= GTA3_PS2_160
	CGarages::Init();
#endif
	CRubbish::Init();
	CClouds::Init();
#if GTA_VERSION <= GTA3_PS2_160
	CRemote::Init();
#endif
	CSpecialFX::Init();
	CWaterCannons::Init();
	CBridge::Init();
#if GTA_VERSION > GTA3_PS2_160
	CGarages::Init();
#endif

	LoadingScreen("Loading the Game", "Position dynamic objects", nil);
	CWorld::RepositionCertainDynamicObjects();
#if GTA_VERSION <= GTA3_PS2_160
	CCullZones::ResolveVisibilities();
#endif

	LoadingScreen("Loading the Game", "Initialise vehicle paths", nil);
#if GTA_VERSION > GTA3_PS2_160
	CCullZones::ResolveVisibilities();
#endif
	CTrain::InitTrains();
	CPlane::InitPlanes();
	CCredits::Init();
	CRecordDataForChase::Init();
	CReplay::Init();

	LoadingScreen("Loading the Game", "Start script", nil);
#ifdef PS2_MENU
	if ( !TheMemoryCard.m_bWantToLoad )
#endif
	{
		CTheScripts::StartTestScript();
		CTheScripts::Process();
		TheCamera.Process();
	}

	LoadingScreen("Loading the Game", "Load scene", nil);
	CModelInfo::RemoveColModelsFromOtherLevels(currLevel);
	CCollision::ms_collisionInMemory = currLevel;
	for (int i = 0; i < MAX_PADS; i++)
		CPad::GetPad(i)->Clear(true);
	return true;
}

bool CGame::ShutDown(void)
{
	CReplay::FinishPlayback();
	CPlane::Shutdown();
	CTrain::Shutdown();
	CSpecialFX::Shutdown();
#if GTA_VERSION > GTA3_PS2_160
	CGarages::Shutdown();
#endif
	CMovingThings::Shutdown();
	gPhoneInfo.Shutdown();
	CWeapon::ShutdownWeapons();
	CPedType::Shutdown();
	CMBlur::MotionBlurClose();
	
	for (int32 i = 0; i < NUMPLAYERS; i++)
	{
		if ( CWorld::Players[i].m_pPed )
		{
			CWorld::Remove(CWorld::Players[i].m_pPed);
			delete CWorld::Players[i].m_pPed;
			CWorld::Players[i].m_pPed = nil;
		}
		
		CWorld::Players[i].Clear();
	}
	
	CRenderer::Shutdown();
	CWorld::ShutDown();
	DMAudio.DestroyAllGameCreatedEntities();
	CModelInfo::ShutDown();
	CAnimManager::Shutdown();
	CCutsceneMgr::Shutdown();
	CVehicleModelInfo::DeleteVehicleColourTextures();
	CVehicleModelInfo::ShutdownEnvironmentMaps();
	CRadar::Shutdown();
	CStreaming::Shutdown();
	CTxdStore::GameShutdown();
	CCollision::Shutdown();
	CWaterLevel::Shutdown();
	CRubbish::Shutdown();
	CClouds::Shutdown();
	CShadows::Shutdown();
	CCoronas::Shutdown();
	CSkidmarks::Shutdown();
	CWeaponEffects::Shutdown();
	CParticle::Shutdown();
#if GTA_VERSION > GTA3_PS2_160
	CPools::ShutDown();
#endif
	CTxdStore::RemoveTxdSlot(gameTxdSlot);
	CdStreamRemoveImages();
	return true;
}

void CGame::ReInitGameObjectVariables(void)
{
	CGameLogic::InitAtStartOfGame();
#ifdef PS2_MENU
	if ( !TheMemoryCard.m_bWantToLoad )
#endif
	{
		TheCamera.Init();
		TheCamera.SetRwCamera(Scene.camera);
	}
	CDebug::DebugInitTextBuffer();
	CWeather::Init();
	CUserDisplay::Init();
	CMessages::Init();
	CRestart::Initialise();
	CWorld::bDoingCarCollisions = false;
	CHud::ReInitialise();
	CRadar::Initialise();
#if GTA_VERSION <= GTA3_PS2_160
	gStartX = -180.0f;
	gStartY = 180.0f;
	gStartZ = 14.0f;
#endif
	CCarCtrl::ReInit();
	CTimeCycle::Initialise();
	CDraw::SetFOV(120.0f);
	CDraw::ms_fLODDistance = 500.0f;
	CStreaming::RequestBigBuildings(LEVEL_GENERIC);
	CStreaming::LoadAllRequestedModels(false);
	CPed::Initialise();
	CEventList::Initialise();
#ifdef SCREEN_DROPLETS
	ScreenDroplets::Initialise();
#endif
	CWeapon::InitialiseWeapons();
	CPopulation::Initialise();
	
	for (int i = 0; i < NUMPLAYERS; i++)
		CWorld::Players[i].Clear();
	
	CWorld::PlayerInFocus = 0;
#if GTA_VERSION <= GTA3_PS2_160
	CWeaponEffects::Init();
	CSkidmarks::Init();
#endif
	CAntennas::Init();
	CGlass::Init();
	gPhoneInfo.Initialise();

	PUSH_MEMID(MEMID_SCRIPT);
	CTheScripts::Init();
	CGangs::Initialise();
	POP_MEMID();

	CTimer::Initialise();
	CClock::Initialise(1000);
	CTheCarGenerators::Init();
	CHeli::InitHelis();
	CMovingThings::Init();
	CDarkel::Init();
	CStats::Init();
	CPickups::Init();
	CPacManPickups::Init();
	CGarages::Init();
#if GTA_VERSION <= GTA3_PS2_160
	CClouds::Init();
	CRemote::Init();
#endif
	CSpecialFX::Init();
	CWaterCannons::Init();
	CParticle::ReloadConfig();
	CCullZones::ResolveVisibilities();

#ifdef PS2_MENU
	if ( !TheMemoryCard.m_bWantToLoad )
#else
	if ( !FrontEndMenuManager.m_bWantToLoad )
#endif
	{
		CCranes::InitCranes();
		CTheScripts::StartTestScript();
		CTheScripts::Process();
		TheCamera.Process();
		CTrain::InitTrains();
		CPlane::InitPlanes();
	}
	
	for (int32 i = 0; i < MAX_PADS; i++)
		CPad::GetPad(i)->Clear(true);
}

void CGame::ReloadIPLs(void)
{
	CTimer::Stop();
	CWorld::RemoveStaticObjects();
	ThePaths.Init();
	CCullZones::Init();
	CFileLoader::ReloadPaths("GTA3.IDE");
	CFileLoader::LoadScene("INDUST.IPL");
	CFileLoader::LoadScene("COMMER.IPL");
	CFileLoader::LoadScene("SUBURBAN.IPL");
	CFileLoader::LoadScene("CULL.IPL");
	ThePaths.PreparePathData();
	CTrafficLights::ScanForLightsOnMap();
	CRoadBlocks::Init();
	CCranes::InitCranes();
	CGarages::Init();
	CWorld::RepositionCertainDynamicObjects();
	CCullZones::ResolveVisibilities();
	CRenderer::SortBIGBuildings();
	CTimer::Update();
}

void CGame::ShutDownForRestart(void)
{
	CReplay::FinishPlayback();
	CReplay::EmptyReplayBuffer();
	DMAudio.DestroyAllGameCreatedEntities();
	
	for (int i = 0; i < NUMPLAYERS; i++)
		CWorld::Players[i].Clear();

	CGarages::SetAllDoorsBackToOriginalHeight();
	CTheScripts::UndoBuildingSwaps();
	CTheScripts::UndoEntityInvisibilitySettings();
	CWorld::ClearForRestart();
	CTimer::Shutdown();
	CStreaming::FlushRequestList();
	CStreaming::DeleteAllRwObjects();
	CStreaming::RemoveAllUnusedModels();
	CStreaming::ms_disableStreaming = false;
	CRadar::RemoveRadarSections();
	FrontEndMenuManager.UnloadTextures();
	CParticleObject::RemoveAllParticleObjects();
#if GTA_VERSION >= GTA3_PS2_160
	CPedType::Shutdown();
	CSpecialFX::Shutdown();
#endif
	TidyUpMemory(true, false);
}

void CGame::InitialiseWhenRestarting(void)
{
	CRect rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	CRGBA color(255, 255, 255, 255);
	
	CTimer::Initialise();
	CSprite2d::SetRecipNearClip();

#ifdef PS2_MENU
	if ( TheMemoryCard.b_FoundRecentSavedGameWantToLoad == true || TheMemoryCard.m_bWantToLoad == false )
	{
		if ( TheMemoryCard.m_bWantToLoad == true )
			MessageScreen("MCLOAD");  // Loading Data. Please do not remove the Memory Card (PS2) in MEMORY CARD slot 1, reset or switch off the console.
		else
			MessageScreen("RESTART"); // Starting new game
	}
#endif
	
#ifdef PS2_MENU
	TheMemoryCard.b_FoundRecentSavedGameWantToLoad = false;
#else
	b_FoundRecentSavedGameWantToLoad = false;
#endif
	
	TheCamera.Init();
	
#ifdef PS2_MENU
	if ( TheMemoryCard.m_bWantToLoad == true )
	{
		TheMemoryCard.RestoreForStartLoad();
		CStreaming::LoadScene(TheCamera.GetPosition());
	}
#else
	if ( FrontEndMenuManager.m_bWantToLoad == true )
	{
		RestoreForStartLoad();
		CStreaming::LoadScene(TheCamera.GetPosition());
	}
#endif
	
	ReInitGameObjectVariables();
	
#ifdef PS2_MENU
	if ( TheMemoryCard.m_bWantToLoad == true )
	{
		if ( TheMemoryCard.LoadSavedGame() == CMemoryCard::RES_SUCCESS )
		{
			for ( int32 i = 0; i < 35; i++ )
			{
				MessageScreen("FESZ_LS"); // Load Successful.
			}
			
			DMAudio.ResetTimers(CTimer::GetTimeInMilliseconds());
			CTrain::InitTrains();
			CPlane::InitPlanes();
		}
		else
		{
			for ( int32 i = 0; i < 50; i++ )
			{				
				DoRWStuffStartOfFrame(50, 50, 50, 0, 0, 0, 255);
				
				CSprite2d::InitPerFrame();
				CFont::InitPerFrame();
				DefinedState();
				
				CSprite2d *splash = LoadSplash(NULL);
				splash->Draw(rect, color, color, color, color);		
#ifdef FIX_BUGS
				splash->DrawRect(CRect(SCREEN_SCALE_X(20.0f), SCREEN_SCALE_Y(110.0f), SCREEN_SCALE_FROM_RIGHT(20.0f), SCREEN_SCALE_Y(300.0f)), CRGBA(50, 50, 50, 192));
#else
				splash->DrawRect(CRect(20.0f, 110.0f, SCREEN_WIDTH-20.0f, 300.0f), CRGBA(50, 50, 50, 192));
#endif

				CFont::SetBackgroundOff();
#ifdef ASPECT_RATIO_SCALE
				CFont::SetWrapx(SCREEN_SCALE_FROM_RIGHT(160.0f)); // because SCREEN_SCALE_FROM_RIGHT(x) != SCREEN_SCALE_X(640-x)
#else
				CFont::SetWrapx(SCREEN_SCALE_X(480.0f));
#endif
				CFont::SetScale(SCREEN_SCALE_X(1.0f), SCREEN_SCALE_Y(1.0f));
				CFont::SetCentreOn();
				CFont::SetCentreSize(SCREEN_SCALE_X(480.0f));
				CFont::SetJustifyOff();
				CFont::SetColor(CRGBA(255, 255, 255, 255));
				CFont::SetBackGroundOnlyTextOff();
				CFont::SetDropColor(CRGBA(32, 32, 32, 255));
				CFont::SetDropShadowPosition(3);
				CFont::SetPropOn();
#ifdef FIX_BUGS
				CFont::PrintString(SCREEN_WIDTH/2, SCREEN_SCALE_Y(130.0f), TheText.Get("MC_LDFL")); // Load Failed!
				CFont::PrintString(SCREEN_WIDTH/2, SCREEN_SCALE_Y(170.0f), TheText.Get("FES_NOC")); // No Memory Card (PS2) in MEMORY CARD slot 1.
				CFont::PrintString(SCREEN_WIDTH/2, SCREEN_SCALE_Y(240.0f), TheText.Get("MC_NWRE")); // Now Restarting Game.
#else
				CFont::PrintString(SCREEN_WIDTH/2, 130.0f, TheText.Get("MC_LDFL")); // Load Failed!
				CFont::PrintString(SCREEN_WIDTH/2, 170.0f, TheText.Get("FES_NOC")); // No Memory Card (PS2) in MEMORY CARD slot 1.
				CFont::PrintString(SCREEN_WIDTH/2, 240.0f, TheText.Get("MC_NWRE")); // Now Restarting Game.
#endif
				CFont::DrawFonts();
				
				DoRWStuffEndOfFrame();
			}
			
			ShutDownForRestart();
			CTimer::Stop();
			CTimer::Initialise();
			TheMemoryCard.m_bWantToLoad = false;
			ReInitGameObjectVariables();
			currLevel = LEVEL_INDUSTRIAL;
			CCollision::SortOutCollisionAfterLoad();
			
			FrontEndMenuManager.SetSoundLevelsForMusicMenu();
			FrontEndMenuManager.InitialiseMenuContentsAfterLoadingGame();
		}
	}
#else
	if ( FrontEndMenuManager.m_bWantToLoad == true )
	{
		if ( GenericLoad() == true )
		{
			DMAudio.ResetTimers(CTimer::GetTimeInMilliseconds());
			CTrain::InitTrains();
			CPlane::InitPlanes();
		}
		else
		{
			for ( int32 i = 0; i < 50; i++ )
			{
				HandleExit();
				FrontEndMenuManager.MessageScreen("FED_LFL"); // Loading save game has failed. The game will restart now. 
			}
			
			ShutDownForRestart();
			CTimer::Stop();
			CTimer::Initialise();
			FrontEndMenuManager.m_bWantToLoad = false;
			ReInitGameObjectVariables();
			currLevel = LEVEL_INDUSTRIAL;
			CCollision::SortOutCollisionAfterLoad();
		}
	}
#endif
	
	CTimer::Update();
	
	DMAudio.ChangeMusicMode(MUSICMODE_GAME);
}

void CGame::Process(void) 
{
	CPad::UpdatePads();
#ifdef USE_CUSTOM_ALLOCATOR
	ProcessTidyUpMemory();
#endif
	TheCamera.SetMotionBlurAlpha(0);
	if (TheCamera.m_BlurType == MOTION_BLUR_NONE || TheCamera.m_BlurType == MOTION_BLUR_SNIPER || TheCamera.m_BlurType == MOTION_BLUR_LIGHT_SCENE)
		TheCamera.SetMotionBlur(0, 0, 0, 0, MOTION_BLUR_NONE);
#ifdef DEBUGMENU
	DebugMenuProcess();
#endif
	CCutsceneMgr::Update();

	PUSH_MEMID(MEMID_FRONTEND);
	if (!CCutsceneMgr::IsCutsceneProcessing() && !CTimer::GetIsCodePaused())
		FrontEndMenuManager.Process();
	POP_MEMID();

	CStreaming::Update();
	if (!CTimer::GetIsPaused())
	{
		CTheZones::Update();
		CSprite2d::SetRecipNearClip();
		CSprite2d::InitPerFrame();
		CFont::InitPerFrame();
		CRecordDataForGame::SaveOrRetrieveDataForThisFrame();
		CRecordDataForChase::SaveOrRetrieveDataForThisFrame();
		CPad::DoCheats();
		CClock::Update();
		CWeather::Update();

		PUSH_MEMID(MEMID_SCRIPT);
		CTheScripts::Process();
		POP_MEMID();

		CCollision::Update();
		CTrain::UpdateTrains();
		CPlane::UpdatePlanes();
		CHeli::UpdateHelis();
		CDarkel::Update();
		CSkidmarks::Update();
		CAntennas::Update();
		CGlass::Update();
#ifdef GTA_SCENE_EDIT
		CSceneEdit::Update();
#endif
		CEventList::Update();
		CParticle::Update();
		gFireManager.Update();
		CPopulation::Update();
		CWeapon::UpdateWeapons();
		if (!CCutsceneMgr::IsRunning())
			CTheCarGenerators::Process();
		if (!CReplay::IsPlayingBack())
			CCranes::UpdateCranes();
		CClouds::Update();
		CMovingThings::Update();
		CWaterCannons::Update();
		CUserDisplay::Process();
		CReplay::Update();

		PUSH_MEMID(MEMID_WORLD);
		CWorld::Process();
		POP_MEMID();

		gAccidentManager.Update();
		CPacManPickups::Update();
		CPickups::Update();
		CGarages::Update();
		CRubbish::Update();
		CSpecialFX::Update();
		CTimeCycle::Update();
		if (CReplay::ShouldStandardCameraBeProcessed())
			TheCamera.Process();
		CCullZones::Update();
		if (!CReplay::IsPlayingBack())
			CGameLogic::Update();
		CBridge::Update();
		CCoronas::DoSunAndMoon();
		CCoronas::Update();
		CShadows::UpdateStaticShadows();
		CShadows::UpdatePermanentShadows();
		gPhoneInfo.Update();
		if (!CReplay::IsPlayingBack())
		{
			PUSH_MEMID(MEMID_CARS);
			CCarCtrl::GenerateRandomCars();
			CRoadBlocks::GenerateRoadBlocks();
			CCarCtrl::RemoveDistantCars();
			POP_MEMID();
		}
	}
#ifdef GTA_PS2
	CMemCheck::DoTest();
#endif
}

#ifdef USE_CUSTOM_ALLOCATOR

int32 gNumMemMoved;

bool
MoveMem(void **ptr)
{
	if(*ptr){
		gNumMemMoved++;
		void *newPtr = gMainHeap.MoveMemory(*ptr);
		if(*ptr != newPtr){
			*ptr = newPtr;
			return true;
		}
	}
	return false;
}

// Some convenience structs
struct SkyDataPrefix
{
	uint32 pktSize1;
	uint32 data;	// pointer to data as read from TXD
	uint32 pktSize2;
	uint32 unused;
};

struct DMAGIFUpload
{
	uint32 tag1_qwc, tag1_addr;	// dmaref
	uint32 nop1, vif_direct1;

	uint32 giftag[4];
	uint32 gs_bitbltbuf[4];

	uint32 tag2_qwc, tag2_addr;	// dmaref
	uint32 nop2, vif_direct2;
};

// This is very scary. it depends on the exact memory layout of the DMA chains and whatnot
RwTexture *
MoveTextureMemoryCB(RwTexture *texture, void *pData)
{
#ifdef GTA_PS2
	bool *pRet = (bool*)pData;
	RwRaster *raster = RwTextureGetRaster(texture);
	_SkyRasterExt *rasterExt = RASTEREXTFROMRASTER(raster);
	if(raster->originalPixels == nil ||	// the raw data
	   raster->cpPixels == raster->originalPixels ||	// old format, can't handle it
	   rasterExt->dmaRefCount != 0 && rasterExt->dmaClrCount != 0)
		return texture;

	// this is the allocated pointer we will move
	SkyDataPrefix *prefix = (SkyDataPrefix*)raster->originalPixels;
	DMAGIFUpload *uploads = (DMAGIFUpload*)(prefix+1);

	// We have 4qw for each upload,
	// i.e. for each buffer width of mip levels,
	// and the palette if there is one.
	// NB: this code does NOT support mipmaps!
	// so we assume two uploads (pixels and palette)
	//
	// each upload looks like this:
	//    (DMAcnt; NOP; VIF DIRECT(2))
	//     giftag (1, A+D)
	//      GS_BITBLTBUF
	//    (DMAref->pixel data; NOP; VIF DIRECT(5))
	// the DMArefs are what we have to adjust
	uintptr dataDiff, upload1Diff, upload2Diff, pixelDiff, paletteDiff;
	dataDiff = prefix->data - (uintptr)raster->originalPixels;
	upload1Diff = uploads[0].tag2_addr - (uintptr)raster->originalPixels;
	if(raster->palette)
		upload2Diff = uploads[1].tag2_addr - (uintptr)raster->originalPixels;
	pixelDiff = (uintptr)raster->cpPixels - (uintptr)raster->originalPixels;
	if(raster->palette)
		paletteDiff = (uintptr)raster->palette - (uintptr)raster->originalPixels;
	uint8 *newptr = (uint8*)gMainHeap.MoveMemory(raster->originalPixels);
	if(newptr != raster->originalPixels){
		// adjust everything
		prefix->data = (uintptr)newptr + dataDiff;
		uploads[0].tag2_addr = (uintptr)newptr + upload1Diff;
		if(raster->palette)
			uploads[1].tag2_addr = (uintptr)newptr + upload2Diff;
		raster->originalPixels = newptr;
		raster->cpPixels = newptr + pixelDiff;
		if(raster->palette)
			raster->palette = newptr + paletteDiff;

		if(pRet){
			*pRet = true;
			return nil;
		}
	}
#else
	// nothing to do here really, everything should be in videomemory
#endif
	return texture;
}

bool
MoveAtomicMemory(RpAtomic *atomic, bool onlyOne)
{
	RpGeometry *geo = RpAtomicGetGeometry(atomic);

#if THIS_IS_COMPATIBLE_WITH_GTA3_RW31
	if(MoveMem((void**)&geo->triangles) && onlyOne)
		return true;
	if(MoveMem((void**)&geo->matList.materials) && onlyOne)
		return true;
	if(MoveMem((void**)&geo->preLitLum) && onlyOne)
		return true;
	if(MoveMem((void**)&geo->texCoords[0]) && onlyOne)
		return true;
	if(MoveMem((void**)&geo->texCoords[1]) && onlyOne)
		return true;

	// verts and normals of morph target are allocated together
	int vertDiff;
	if(geo->morphTarget->normals)
		vertDiff = geo->morphTarget->normals - geo->morphTarget->verts;
	if(MoveMem((void**)&geo->morphTarget->verts)){
		if(geo->morphTarget->normals)
			geo->morphTarget->normals = geo->morphTarget->verts + vertDiff;
		if(onlyOne)
			return true;
	}

	RpMeshHeader *oldmesh = geo->mesh;
	if(MoveMem((void**)&geo->mesh)){
		// index pointers are allocated together with meshes,
		// have to relocate those too
		RpMesh *mesh = (RpMesh*)(geo->mesh+1);
		uintptr reloc = (uintptr)geo->mesh - (uintptr)oldmesh;
		for(int i = 0; i < geo->mesh->numMeshes; i++)
			mesh[i].indices = (RxVertexIndex*)((uintptr)mesh[i].indices + reloc);
		if(onlyOne)
			return true;
	}
#else
	// we could do something in librw here
#endif
	return false;
}

bool
MoveColModelMemory(CColModel &colModel, bool onlyOne)
{
#if GTA_VERSION >= GTA3_PS2_160
	// hm...should probably only do this if ownsCollisionVolumes
	// but it doesn't exist on PS2...
	if(!colModel.ownsCollisionVolumes)
		return false;
#endif

	if(MoveMem((void**)&colModel.spheres) && onlyOne)
		return true;
	if(MoveMem((void**)&colModel.lines) && onlyOne)
		return true;
	if(MoveMem((void**)&colModel.boxes) && onlyOne)
		return true;
	if(MoveMem((void**)&colModel.vertices) && onlyOne)
		return true;
	if(MoveMem((void**)&colModel.triangles) && onlyOne)
		return true;
	if(MoveMem((void**)&colModel.trianglePlanes) && onlyOne)
		return true;
	return false;
}

RpAtomic*
MoveAtomicMemoryCB(RpAtomic *atomic, void *pData)
{
	bool *pRet = (bool*)pData;
	if(pRet == nil)
		MoveAtomicMemory(atomic, false);
	else if(MoveAtomicMemory(atomic, true)){
		*pRet = true;
		return nil;
	}
	return atomic;
}

bool
TidyUpModelInfo(CBaseModelInfo *modelInfo, bool onlyone)
{
	if(modelInfo->GetColModel() && modelInfo->DoesOwnColModel())
		if(MoveColModelMemory(*modelInfo->GetColModel(), onlyone))
			return true;

	RwObject *rwobj = modelInfo->GetRwObject();
	if(RwObjectGetType(rwobj) == rpATOMIC)
		if(MoveAtomicMemory((RpAtomic*)rwobj, onlyone))
			return true;
	if(RwObjectGetType(rwobj) == rpCLUMP){
		bool ret = false;
		if(onlyone)
			RpClumpForAllAtomics((RpClump*)rwobj, MoveAtomicMemoryCB, &ret);
		else
			RpClumpForAllAtomics((RpClump*)rwobj, MoveAtomicMemoryCB, nil);
		if(ret)
			return true;
	}

	if(modelInfo->GetModelType() == MITYPE_PED && ((CPedModelInfo*)modelInfo)->m_hitColModel)
		if(MoveColModelMemory(*((CPedModelInfo*)modelInfo)->m_hitColModel, onlyone))
			return true;

	return false;
}
#endif

void CGame::DrasticTidyUpMemory(bool flushDraw)
{
#ifdef USE_CUSTOM_ALLOCATOR
	bool removedCol = false;

	TidyUpMemory(true, flushDraw);

	if(gMainHeap.GetLargestFreeBlock() < 200000 && !playingIntro){
		CStreaming::RemoveIslandsNotUsed(LEVEL_INDUSTRIAL);
		CStreaming::RemoveIslandsNotUsed(LEVEL_COMMERCIAL);
		CStreaming::RemoveIslandsNotUsed(LEVEL_SUBURBAN);
		TidyUpMemory(true, flushDraw);
	}

	if(gMainHeap.GetLargestFreeBlock() < 200000 && !playingIntro){
		CModelInfo::RemoveColModelsFromOtherLevels(LEVEL_GENERIC);
		TidyUpMemory(true, flushDraw);
		removedCol = true;
	}

	if(gMainHeap.GetLargestFreeBlock() < 200000 && !playingIntro){
		CStreaming::RemoveBigBuildings(LEVEL_INDUSTRIAL);
		CStreaming::RemoveBigBuildings(LEVEL_COMMERCIAL);
		CStreaming::RemoveBigBuildings(LEVEL_SUBURBAN);
		TidyUpMemory(true, flushDraw);
	}

	if(removedCol){
		// different on PS2
		CFileLoader::LoadCollisionFromDatFile(CCollision::ms_collisionInMemory);
	}

	if(!playingIntro)
		CStreaming::RequestBigBuildings(currLevel);

	CStreaming::LoadAllRequestedModels(true);
#endif
}

void CGame::TidyUpMemory(bool moveTextures, bool flushDraw)
{
#ifdef USE_CUSTOM_ALLOCATOR
	printf("Largest free block before tidy %d\n", gMainHeap.GetLargestFreeBlock());

	if(moveTextures){
		if(flushDraw){
#ifdef GTA_PS2
			for(int i = 0; i < sweMaxFlips+1; i++){
#else
			for(int i = 0; i < 5; i++){	// probably more than needed
#endif
				RwCameraBeginUpdate(Scene.camera);
				RwCameraEndUpdate(Scene.camera);
				RwCameraShowRaster(Scene.camera, nil, 0);
			}
		}
		int fontSlot = CTxdStore::FindTxdSlot("fonts");

		for(int i = 0; i < TXDSTORESIZE; i++){
			if(i == fontSlot ||
			   CTxdStore::GetSlot(i) == nil)
				continue;
			RwTexDictionary *txd = CTxdStore::GetSlot(i)->texDict;
			if(txd)
				RwTexDictionaryForAllTextures(txd, MoveTextureMemoryCB, nil);
		}
	}

	// animations
	for(int i = 0; i < NUMANIMATIONS; i++){
		CAnimBlendHierarchy *anim = CAnimManager::GetAnimation(i);
		if(anim == nil)
			continue;	// cannot happen
		anim->MoveMemory();
	}

	// model info
	for(int i = 0; i < MODELINFOSIZE; i++){
		CBaseModelInfo *mi = CModelInfo::GetModelInfo(i);
		if(mi == nil)
			continue;
		TidyUpModelInfo(mi, false);
	}

	printf("Largest free block after tidy %d\n", gMainHeap.GetLargestFreeBlock());
#endif
}

void CGame::ProcessTidyUpMemory(void)
{
#ifdef USE_CUSTOM_ALLOCATOR
	static int32 modelIndex = 0;
	static int32 animIndex = 0;
	static int32 txdIndex = 0;
	bool txdReturn = false;
	RwTexDictionary *txd = nil;
	gNumMemMoved = 0;

	// model infos
	for(int numCleanedUp = 0; numCleanedUp < 10; numCleanedUp++){
		CBaseModelInfo *mi;
		do{
			mi = CModelInfo::GetModelInfo(modelIndex);
			modelIndex++;
			if(modelIndex >= MODELINFOSIZE)
				modelIndex = 0;
		}while(mi == nil);

		if(TidyUpModelInfo(mi, true))
			return;
	}

	// tex dicts
	for(int numCleanedUp = 0; numCleanedUp < 3; numCleanedUp++){
		if(gNumMemMoved > 80)
			break;

		do{
#ifdef FIX_BUGS
			txd = nil;
#endif
			if(CTxdStore::GetSlot(txdIndex))
				txd = CTxdStore::GetSlot(txdIndex)->texDict;
			txdIndex++;
			if(txdIndex >= TXDSTORESIZE)
				txdIndex = 0;
		}while(txd == nil);

		RwTexDictionaryForAllTextures(txd, MoveTextureMemoryCB, &txdReturn);
		if(txdReturn)
			return;
	}

	// animations
	CAnimBlendHierarchy *anim;
	do{
		anim = CAnimManager::GetAnimation(animIndex);
		animIndex++;
		if(animIndex >= NUMANIMATIONS)
			animIndex = 0;
	}while(anim == nil);	// always != nil
	anim->MoveMemory(true);
#endif
}
