﻿#pragma warning( push )
#pragma warning( disable : 4005)
#pragma warning( pop )
#include "common.h"
#include "platform.h"

#include "Game.h"
#include "main.h"
#include "RwHelper.h"
#include "Accident.h"
#include "Antennas.h"
#include "Bridge.h"
#include "Camera.h"
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
#include "Sprite2d.h"
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
	//TODO: stretch_screen
	
	CRect rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	CRGBA color(255, 255, 255, 255);

	DoRWStuffStartOfFrame(50, 50, 50, 0, 0, 0, 255);
	
	CSprite2d::InitPerFrame();
	CFont::InitPerFrame();
	DefinedState();

	CSprite2d *splash = LoadSplash(NULL);
	splash->Draw(rect, color, color, color, color);
	splash->DrawRect(CRect(SCREEN_SCALE_X(20.0f), SCREEN_SCALE_Y(110.0f), SCREEN_SCALE_X(620.0f), SCREEN_SCALE_Y(300.0f)), CRGBA(50, 50, 50, 192));
	
	CFont::SetFontStyle(FONT_BANK);
	CFont::SetBackgroundOff();
	CFont::SetWrapx(SCREEN_SCALE_FROM_RIGHT(190.0f)); // 450.0f
	CFont::SetScale(SCREEN_SCALE_X(1.0f), SCREEN_SCALE_Y(1.0f));
	CFont::SetCentreOn();
	CFont::SetCentreSize(SCREEN_SCALE_X(450.0f)); // 450.0f
	CFont::SetJustifyOff();
	CFont::SetColor(CRGBA(255, 255, 255, 255));
	CFont::SetDropColor(CRGBA(32, 32, 32, 255));
	CFont::SetDropShadowPosition(3);
	CFont::SetPropOn();
	CFont::PrintString(SCREEN_SCALE_X(320.0f), SCREEN_SCALE_Y(130.0f), TheText.Get(msg));
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
	
	CTxdStore::Initialise();
	CVisibilityPlugins::Initialise();
	
	/* Create camera */
	Scene.camera = CameraCreate(RsGlobal.width, RsGlobal.height, TRUE);
	ASSERT(Scene.camera != nil);
	if (!Scene.camera)
	{
		return (false);
	}
	
	RwCameraSetFarClipPlane(Scene.camera, 2000.0f);
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
	
	CreateDebugFont();

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
	
	CFont::Initialise();
	CHud::Initialise();
	CPlayerSkin::Initialise();
	
	return (true);
}

void CGame::ShutdownRenderWare(void)
{
	CMBlur::MotionBlurClose();
	DestroySplashScreen();
	CHud::Shutdown();
	CFont::Shutdown();
	
	for ( int32 i = 0; i < NUMPLAYERS; i++ )
		CWorld::Players[i].DeletePlayerSkin();
	
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

bool CGame::InitialiseOnceAfterRW(void)
{
	TheText.Load();
	DMAudio.Initialise();
	CTimer::Initialise();
	CTempColModels::Initialise();
	mod_HandlingManager.Initialise();
	CSurfaceTable::Initialise("DATA\\SURFACE.DAT");
	CPedStats::Initialise();
	CTimeCycle::Initialise();

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
	CWorld::Players[0].SetPlayerSkin(CMenuManager::m_PrefsSkinFile);

	return true;
}

void
CGame::FinalShutdown(void)
{	
	CTxdStore::Shutdown();
	CPedStats::Shutdown();
	CdStreamShutdown();
}

bool CGame::Initialise(const char* datFile)
{
	ResetLoadingScreenBar();
	strcpy(aDatFile, datFile);
	CPools::Initialise();
	CIniFile::LoadIniFile();
	currLevel = LEVEL_INDUSTRIAL;
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
#ifdef PS2_MENU
	TheText.Load();
#endif
	CTheZones::Init();
	CUserDisplay::Init();
	CMessages::Init();
	CMessages::ClearAllMessagesDisplayedByGame();
	CRecordDataForGame::Init();
	CRestart::Initialise();
	CWorld::Initialise();
	CParticle::Initialise();
#ifdef PS2
	gStartX = -180.0f;
	gStartY = 180.0f;
	gStartZ = 14.0f;
#endif
	CAnimManager::Initialise();
	CCutsceneMgr::Initialise();
	CCarCtrl::Init();
	InitModelIndices();
	CModelInfo::Initialise();
	CPickups::Init();
	CTheCarGenerators::Init();
	CdStreamAddImage("MODELS\\GTA3.IMG");
	CFileLoader::LoadLevel("DATA\\DEFAULT.DAT");
	CFileLoader::LoadLevel(datFile);
#ifdef EXTENDED_PIPELINES
	// for generic fallback
	CustomPipes::SetTxdFindCallback();
#endif
	CWorld::AddParticles();
	CVehicleModelInfo::LoadVehicleColours();
	CVehicleModelInfo::LoadEnvironmentMaps();
	CTheZones::PostZoneCreation();
	LoadingScreen("Loading the Game", "Setup paths", GetRandomSplashScreen());
	ThePaths.PreparePathData();
	for (int i = 0; i < NUMPLAYERS; i++)
		CWorld::Players[i].Clear();
	CWorld::Players[0].LoadPlayerSkin();
	TestModelIndices();
	LoadingScreen("Loading the Game", "Setup water", nil);
	CWaterLevel::Initialise("DATA\\WATER.DAT");
	TheConsole.Init();
	CDraw::SetFOV(120.0f);
	CDraw::ms_fLODDistance = 500.0f;
	LoadingScreen("Loading the Game", "Setup streaming", nil);
	CStreaming::Init();
	CStreaming::LoadInitialVehicles();
	CStreaming::LoadInitialPeds();
	CStreaming::RequestBigBuildings(LEVEL_GENERIC);
	CStreaming::LoadAllRequestedModels(false);
	printf("Streaming uses %zuK of its memory", CStreaming::ms_memoryUsed / 1024); // original modifier was %d
	LoadingScreen("Loading the Game", "Load animations", GetRandomSplashScreen());
	CAnimManager::LoadAnimFiles();
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
	CWorld::PlayerInFocus = 0;
	CCoronas::Init();
	CShadows::Init();
	CWeaponEffects::Init();
	CSkidmarks::Init();
	CAntennas::Init();
	CGlass::Init();
	gPhoneInfo.Initialise();
	CSceneEdit::Initialise();
	LoadingScreen("Loading the Game", "Load scripts", nil);
	CTheScripts::Init();
	CGangs::Initialise();
	LoadingScreen("Loading the Game", "Setup game variables", nil);
	CClock::Initialise(1000);
	CHeli::InitHelis();
	CCranes::InitCranes();
	CMovingThings::Init();
	CDarkel::Init();
	CStats::Init();
	CPacManPickups::Init();
	CRubbish::Init();
	CClouds::Init();
	CSpecialFX::Init();
	CWaterCannons::Init();
	CBridge::Init();
	CGarages::Init();
	LoadingScreen("Loading the Game", "Position dynamic objects", nil);
	CWorld::RepositionCertainDynamicObjects();
	LoadingScreen("Loading the Game", "Initialise vehicle paths", nil);
	CCullZones::ResolveVisibilities();
	CTrain::InitTrains();
	CPlane::InitPlanes();
	CCredits::Init();
	CRecordDataForChase::Init();
	CReplay::Init();
#ifdef PS2_MENU
	if ( !TheMemoryCard.m_bWantToLoad )
	{
#endif
	LoadingScreen("Loading the Game", "Start script", nil);
	CTheScripts::StartTestScript();
	CTheScripts::Process();
	TheCamera.Process();
#ifdef PS2_MENU
	}
#endif
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
#ifndef PS2
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
	CPools::ShutDown();
	CTxdStore::RemoveTxdSlot(gameTxdSlot);
	CdStreamRemoveImages();
	return true;
}

void CGame::ReInitGameObjectVariables(void)
{
	CGameLogic::InitAtStartOfGame();
#ifdef PS2_MENU
	if ( !TheMemoryCard.m_bWantToLoad )
	{
#endif
	TheCamera.Init();
	TheCamera.SetRwCamera(Scene.camera);
#ifdef PS2_MENU
	}
#endif
	CDebug::DebugInitTextBuffer();
	CWeather::Init();
	CUserDisplay::Init();
	CMessages::Init();
	CRestart::Initialise();
	CWorld::bDoingCarCollisions = false;
	CHud::ReInitialise();
	CRadar::Initialise();
#ifdef PS2
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
#ifdef PS2
	CWeaponEffects::Init();
	CSkidmarks::Init();
#endif
	CAntennas::Init();
	CGlass::Init();
	gPhoneInfo.Initialise();
	CTheScripts::Init();
	CGangs::Initialise();
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
#ifdef PS2
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
	//CTimer::Update(1000/30);
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
#ifndef PS2
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
				splash->DrawRect(CRect(SCREEN_SCALE_X(20.0f), SCREEN_SCALE_Y(110.0f), SCREEN_SCALE_X(620.0f), SCREEN_SCALE_Y(300.0f)), CRGBA(50, 50, 50, 192));
				
				//CFont::SetFontStyle(?);
				CFont::SetBackgroundOff();
				CFont::SetWrapx(SCREEN_SCALE_FROM_RIGHT(160.0f)); // 480.0f
				CFont::SetScale(SCREEN_SCALE_X(1.0f), SCREEN_SCALE_Y(1.0f));
				CFont::SetCentreOn();
				CFont::SetCentreSize(SCREEN_SCALE_X(480.0f));
				CFont::SetJustifyOff();
				CFont::SetColor(CRGBA(255, 255, 255, 255));
				CFont::SetBackGroundOnlyTextOff();
				CFont::SetDropColor(CRGBA(32, 32, 32, 255));
				CFont::SetDropShadowPosition(3);
				CFont::SetPropOn();
				CFont::PrintString(SCREEN_SCALE_X(320.0f), SCREEN_SCALE_Y(130.0f), TheText.Get("MC_LDFL")); // Load Failed!
				CFont::PrintString(SCREEN_SCALE_X(320.0f), SCREEN_SCALE_Y(170.0f), TheText.Get("FES_NOC")); // No Memory Card (PS2) in MEMORY CARD slot 1.
				CFont::PrintString(SCREEN_SCALE_X(320.0f), SCREEN_SCALE_Y(240.0f), TheText.Get("MC_NWRE")); // Now Restarting Game.
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
	
	//CTimer::Update(1000/30);
	
	DMAudio.ChangeMusicMode(MUSICMODE_GAME);
}

void CGame::Process(void) 
{
	CPad::UpdatePads();
#ifdef GTA_PS2
	ProcessTidyUpMemory();
#endif
	TheCamera.SetMotionBlurAlpha(0);
	if (TheCamera.m_BlurType == MOTION_BLUR_NONE || TheCamera.m_BlurType == MOTION_BLUR_SNIPER || TheCamera.m_BlurType == MOTION_BLUR_LIGHT_SCENE)
		TheCamera.SetMotionBlur(0, 0, 0, 0, MOTION_BLUR_NONE);
#ifdef DEBUGMENU
	DebugMenuProcess();
#endif
	CCutsceneMgr::Update();
	if (!CCutsceneMgr::IsCutsceneProcessing() && !CTimer::GetIsCodePaused())
		FrontEndMenuManager.Process();
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
		CTheScripts::Process();
		CCollision::Update();
		CTrain::UpdateTrains();
		CPlane::UpdatePlanes();
		CHeli::UpdateHelis();
		CDarkel::Update();
		CSkidmarks::Update();
		CAntennas::Update();
		CGlass::Update();
		CSceneEdit::Update();
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
		CWorld::Process();
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
			CCarCtrl::GenerateRandomCars();
			CRoadBlocks::GenerateRoadBlocks();
			CCarCtrl::RemoveDistantCars();
		}
	}
#ifdef PS2
	CMemCheck::DoTest();
#endif
}

void
CGame::UpdateMatrices()
{
	for(CPtrNode *node = CWorld::GetMovingEntityList().first; node; node = node->next) {
		CPlaceable *movingEnt = (CPlaceable *)node->item;
		if(movingEnt) { movingEnt->m_matrix.SaveOldMatrix(); }
	}
}

void
CGame::InterpolateMatrices()
{
	for(CPtrNode *node = CWorld::GetMovingEntityList().first; node; node = node->next) {
		CPhysical *movingEnt = (CPhysical *)node->item;
		movingEnt->m_matrix.UpdateRWInterPolated();
		movingEnt->UpdateRwFrameInterpolated();
	}
}

void CGame::DrasticTidyUpMemory(bool)
{
#ifdef PS2
	// meow
#endif
}

void CGame::TidyUpMemory(bool, bool)
{
#ifdef PS2
	// meow
#endif
}

void CGame::ProcessTidyUpMemory(void)
{
#ifdef PS2
	// meow
#endif
}
