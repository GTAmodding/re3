#include "common.h"
#include "patcher.h"
#include "Game.h"
#include "main.h"
#include "AccidentManager.h"
#include "Antennas.h"
#include "Bridge.h"
#include "Camera.h"
#include "CarCtrl.h"
#include "CarGen.h"
#include "CdStream.h"
#include "Clock.h"
#include "Clouds.h"
#include "Collision.h"
#include "Coronas.h"
#include "Cranes.h"
#include "CutsceneMgr.h"
#include "Darkel.h"
#include "EventList.h"
#include "FileLoader.h"
#include "FileMgr.h"
#include "Fire.h"
#include "Fluff.h"
#include "Font.h"
#include "Frontend.h"
#include "GameLogic.h"
#include "Garages.h"
#include "Glass.h"
#include "Heli.h"
#include "Pad.h"
#include "Particle.h"
#include "Phones.h"
#include "Pickups.h"
#include "Plane.h"
#include "Population.h"
#include "Record.h"
#include "Renderer.h"
#include "Replay.h"
#include "RoadBlocks.h"
#include "Rubbish.h"
#include "SceneEdit.h"
#include "Script.h"
#include "Shadows.h"
#include "Skidmarks.h"
#include "SpecialFX.h"
#include "Sprite2d.h"
#include "Streaming.h"
#include "TimeCycle.h"
#include "TrafficLights.h"
#include "Train.h"
#include "User.h"
#include "WaterCannon.h"
#include "Weapon.h"
#include "Weather.h"
#include "World.h"
#include "ZoneCull.h"
#include "Zones.h"

eLevelName &CGame::currLevel = *(eLevelName*)0x941514;
bool &CGame::bDemoMode = *(bool*)0x5F4DD0;
bool &CGame::nastyGame = *(bool*)0x5F4DD4;
bool &CGame::frenchGame = *(bool*)0x95CDCB;
bool &CGame::germanGame = *(bool*)0x95CD1E;
bool &CGame::noProstitutes = *(bool*)0x95CDCF;
bool &CGame::playingIntro = *(bool*)0x95CDC2;
char *CGame::aDatFile = (char*)0x773A48;


bool
CGame::InitialiseOnceBeforeRW(void)
{
	CFileMgr::Initialise();
	CdStreamInit(MAX_CDCHANNELS);
	ValidateVersion();
	return true;
}

WRAPPER void CGame::Initialise(const char *datFile) { EAXJMP(0x48BED0); }
#if 0
WRAPPER void CGame::Process(void) { EAXJMP(0x48C850); }
#else
void CGame::Process(void) 
{
	CPad::UpdatePads();
	TheCamera.SetMotionBlurAlpha(0);
	if (TheCamera.m_BlurType == 0 || TheCamera.m_BlurType == 1 || TheCamera.m_BlurType == 2)
		TheCamera.SetMotionBlur(0, 0, 0, 0, 0);
	CCutsceneMgr::Update();
	if (!CCutsceneMgr::ms_cutsceneProcessing && !CTimer::GetIsCodePaused())
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
}
#endif

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

WRAPPER bool CGame::InitialiseRenderWare(void) { EAXJMP(0x48BBA0); }
WRAPPER void CGame::ShutdownRenderWare(void) { EAXJMP(0x48BCB0); }
WRAPPER void CGame::FinalShutdown(void) { EAXJMP(0x48BEC0); }
WRAPPER void CGame::ShutDown(void) { EAXJMP(0x48C3A0); }
WRAPPER void CGame::ShutDownForRestart(void) { EAXJMP(0x48C6B0); }
WRAPPER void CGame::InitialiseWhenRestarting(void) { EAXJMP(0x48C740); }
WRAPPER bool CGame::InitialiseOnceAfterRW(void) { EAXJMP(0x48BD50); }

STARTPATCHES
InjectHook(0x48C850, CGame::Process, PATCH_JUMP);
ENDPATCHES
