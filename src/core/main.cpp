#include "common.h"
#include "patcher.h"
#include "main.h"
#include "General.h"
#include "RwHelper.h"
#include "Clouds.h"
#include "Draw.h"
#include "Sprite2d.h"
#include "Renderer.h"
#include "Coronas.h"
#include "WaterLevel.h"
#include "Weather.h"
#include "Glass.h"
#include "WaterCannon.h"
#include "SpecialFX.h"
#include "Shadows.h"
#include "Skidmarks.h"
#include "Antennas.h"
#include "Rubbish.h"
#include "Particle.h"
#include "Pickups.h"
#include "WeaponEffects.h"
#include "PointLights.h"
#include "Fluff.h"
#include "Replay.h"
#include "Camera.h"
#include "World.h"
#include "Ped.h"
#include "Font.h"
#include "Pad.h"
#include "Hud.h"
#include "User.h"
#include "Messages.h"
#include "Darkel.h"
#include "Garages.h"
#include "MusicManager.h"
#include "VisibilityPlugins.h"
#include "NodeName.h"
#include "DMAudio.h"
#include "CutsceneMgr.h"
#include "Lights.h"
#include "Credits.h"
#include "ZoneCull.h"
#include "Timecycle.h"
#include "TxdStore.h"
#include "FileMgr.h"
#include "Text.h"
#include "RpAnimBlend.h"
#include "Frontend.h"
#include "AnimViewer.h"
#include "Script.h"
#include "Debug.h"
#include "Console.h"
#include "timebars.h"

#define DEFAULT_VIEWWINDOW (Tan(DEGTORAD(CDraw::GetFOV() * 0.5f)))


GlobalScene &Scene = *(GlobalScene*)0x726768;

uint8 work_buff[55000];
//char gString[256];
//char gString2[512];
//wchar gUString[256];
//wchar gUString2[256];
char *gString = (char*)0x711B40;
char *gString2 = (char*)0x878A40;
wchar *gUString = (wchar*)0x74B018;
wchar *gUString2 = (wchar*)0x6EDD70;

bool &b_FoundRecentSavedGameWantToLoad = *(bool*)0x95CDA8;


char version_name[64];

float FramesPerSecond = 30.0f;

bool gbPrintShite = false;
bool &gbModelViewer = *(bool*)0x95CD93;

bool DoRWStuffStartOfFrame_Horizon(int16 TopRed, int16 TopGreen, int16 TopBlue, int16 BottomRed, int16 BottomGreen, int16 BottomBlue, int16 Alpha);
bool DoRWStuffStartOfFrame(int16 TopRed, int16 TopGreen, int16 TopBlue, int16 BottomRed, int16 BottomGreen, int16 BottomBlue, int16 Alpha);
void DoRWStuffEndOfFrame(void);

void RenderScene(void);
void RenderDebugShit(void);
void RenderEffects(void);
void Render2dStuff(void);
void RenderMenus(void);
void DoFade(void);
void Render2dStuffAfterFade(void);

CSprite2d *LoadSplash(const char *name);


extern void (*DebugMenuProcess)(void);
extern void (*DebugMenuRender)(void);
void DebugMenuInit(void);
void DebugMenuPopulate(void);

void PrintGameVersion();

RwRGBA gColourTop;

void
InitialiseGame(void)
{
	LoadingScreen(nil, nil, "loadsc0");
	CGame::Initialise("DATA\\GTA3.DAT");
}

#ifndef MASTER
void
TheModelViewer(void)
{
#ifdef ASPECT_RATIO_SCALE
	CDraw::SetAspectRatio(CDraw::FindAspectRatio());
#endif
	CAnimViewer::Update();
	CTimer::Update();
	SetLightsWithTimeOfDayColour(Scene.world);
	CRenderer::ConstructRenderList();
	DoRWStuffStartOfFrame(CTimeCycle::GetSkyTopRed(), CTimeCycle::GetSkyTopGreen(), CTimeCycle::GetSkyTopBlue(),
		CTimeCycle::GetSkyBottomRed(), CTimeCycle::GetSkyBottomGreen(), CTimeCycle::GetSkyBottomBlue(),
		255);

	CSprite2d::InitPerFrame();
	CFont::InitPerFrame();
	DefinedState();
	CVisibilityPlugins::InitAlphaEntityList();
	CAnimViewer::Render();
	Render2dStuff();
	DoRWStuffEndOfFrame();
}
#endif

void
Idle(void *arg)
{
#ifdef ASPECT_RATIO_SCALE
	CDraw::SetAspectRatio(CDraw::FindAspectRatio());
#endif

	CTimer::Update();

#ifdef TIMEBARS
	tbInit();
#endif

	CSprite2d::InitPerFrame();
	CFont::InitPerFrame();

	// We're basically merging FrontendIdle and Idle (just like TheGame on PS2)
#ifdef PS2_SAVE_DIALOG
	// Only exists on PC FrontendIdle, probably some PS2 bug fix
	if (FrontEndMenuManager.m_bMenuActive)
		CSprite2d::SetRecipNearClip();
	
	if (FrontEndMenuManager.m_bGameNotLoaded) {
		CPad::UpdatePads();
		FrontEndMenuManager.Process();
	} else {
		CPointLights::InitPerFrame();
#ifdef TIMEBARS
		tbStartTimer(0, "CGame::Process");
#endif
		CGame::Process();
#ifdef TIMEBARS
		tbEndTimer("CGame::Process");
		tbStartTimer(0, "DMAudio.Service");
#endif
		DMAudio.Service();

#ifdef TIMEBARS
		tbEndTimer("DMAudio.Service");
#endif
	}

	if (RsGlobal.quit)
		return;
#else
	CPointLights::InitPerFrame();
#ifdef TIMEBARS
	tbStartTimer(0, "CGame::Process");
#endif
	CGame::Process();
#ifdef TIMEBARS
	tbEndTimer("CGame::Process");
	tbStartTimer(0, "DMAudio.Service");
#endif

	DMAudio.Service();

#ifdef TIMEBARS
	tbEndTimer("DMAudio.Service");
#endif
#endif

	if(CGame::bDemoMode && CTimer::GetTimeInMilliseconds() > (3*60 + 30)*1000 && !CCutsceneMgr::IsCutsceneProcessing()){
		FrontEndMenuManager.m_bStartGameLoading = true;
		FrontEndMenuManager.m_bLoadingSavedGame = false;
		return;
	}

	if(FrontEndMenuManager.m_bStartGameLoading || b_FoundRecentSavedGameWantToLoad)
		return;

	SetLightsWithTimeOfDayColour(Scene.world);

	if(arg == nil)
		return;

	if((!FrontEndMenuManager.m_bMenuActive || FrontEndMenuManager.m_bRenderGameInMenu) &&
	   TheCamera.GetScreenFadeStatus() != FADE_2){
#ifdef GTA_PC
		if (!FrontEndMenuManager.m_bRenderGameInMenu) {
			// This is from SA, but it's nice for windowed mode
			RwV2d pos;
			pos.x = SCREEN_WIDTH / 2.0f;
			pos.y = SCREEN_HEIGHT / 2.0f;
			RsMouseSetPos(&pos);
		}
#endif
#ifdef TIMEBARS
		tbStartTimer(0, "CnstrRenderList");
#endif
		CRenderer::ConstructRenderList();
#ifdef TIMEBARS
		tbEndTimer("CnstrRenderList");
		tbStartTimer(0, "PreRender");
#endif
		CRenderer::PreRender();
#ifdef TIMEBARS
		tbEndTimer("PreRender");
#endif

		if(CWeather::LightningFlash && !CCullZones::CamNoRain()){
			if(!DoRWStuffStartOfFrame_Horizon(255, 255, 255, 255, 255, 255, 255))
				return;
		}else{
			if(!DoRWStuffStartOfFrame_Horizon(CTimeCycle::GetSkyTopRed(), CTimeCycle::GetSkyTopGreen(), CTimeCycle::GetSkyTopBlue(),
						CTimeCycle::GetSkyBottomRed(), CTimeCycle::GetSkyBottomGreen(), CTimeCycle::GetSkyBottomBlue(),
						255))
				return;
		}

		DefinedState();

		// BUG. This has to be done BEFORE RwCameraBeginUpdate
		RwCameraSetFarClipPlane(Scene.camera, CTimeCycle::GetFarClip());
		RwCameraSetFogDistance(Scene.camera, CTimeCycle::GetFogStart());

#ifdef TIMEBARS
		tbStartTimer(0, "RenderScene");
#endif
		RenderScene();
#ifdef TIMEBARS
		tbEndTimer("RenderScene");
#endif
		RenderDebugShit();
		RenderEffects();

#ifdef TIMEBARS
		tbStartTimer(0, "RenderMotionBlur");
#endif
		if((TheCamera.m_BlurType == MBLUR_NONE || TheCamera.m_BlurType == MBLUR_NORMAL) &&
		   TheCamera.m_ScreenReductionPercentage > 0.0f)
		        TheCamera.SetMotionBlurAlpha(150);
		TheCamera.RenderMotionBlur();
#ifdef TIMEBARS
		tbEndTimer("RenderMotionBlur");
		tbStartTimer(0, "Render2dStuff");
#endif
		Render2dStuff();
#ifdef TIMEBARS
		tbEndTimer("Render2dStuff");
#endif
	}else{
		float viewWindow = DEFAULT_VIEWWINDOW;
#ifdef ASPECT_RATIO_SCALE
		CameraSize(Scene.camera, nil, viewWindow, SCREEN_ASPECT_RATIO);
#else
		CameraSize(Scene.camera, nil, viewWindow, DEFAULT_ASPECT_RATIO);
#endif
		CVisibilityPlugins::SetRenderWareCamera(Scene.camera);
		RwCameraClear(Scene.camera, &gColourTop, rwCAMERACLEARZ);
		if(!RsCameraBeginUpdate(Scene.camera))
			return;
	}

#ifdef PS2_SAVE_DIALOG
	if (FrontEndMenuManager.m_bMenuActive)
		DefinedState();
#endif
#ifdef TIMEBARS
	tbStartTimer(0, "RenderMenus");
#endif
	RenderMenus();
#ifdef TIMEBARS
	tbEndTimer("RenderMenus");
	tbStartTimer(0, "DoFade");
#endif
	DoFade();
#ifdef TIMEBARS
	tbEndTimer("DoFade");
	tbStartTimer(0, "Render2dStuff-Fade");
#endif
	Render2dStuffAfterFade();
#ifdef TIMEBARS
	tbEndTimer("Render2dStuff-Fade");
#endif
	CCredits::Render();

#ifdef TIMEBARS
	tbDisplay();
#endif

	DoRWStuffEndOfFrame();

//	if(g_SlowMode) 
//		ProcessSlowMode();
}

void
FrontendIdle(void)
{
#ifdef ASPECT_RATIO_SCALE
	CDraw::SetAspectRatio(CDraw::FindAspectRatio());
#endif

	CTimer::Update();
	CSprite2d::SetRecipNearClip(); // this should be on InitialiseRenderWare according to PS2 asm. seems like a bug fix
	CSprite2d::InitPerFrame();
	CFont::InitPerFrame();
	CPad::UpdatePads();
	FrontEndMenuManager.Process();

	if(RsGlobal.quit)
		return;

	float viewWindow = DEFAULT_VIEWWINDOW;
#ifdef ASPECT_RATIO_SCALE
	CameraSize(Scene.camera, nil, viewWindow, SCREEN_ASPECT_RATIO);
#else
	CameraSize(Scene.camera, nil, viewWindow, DEFAULT_ASPECT_RATIO);
#endif
	CVisibilityPlugins::SetRenderWareCamera(Scene.camera);
	RwCameraClear(Scene.camera, &gColourTop, rwCAMERACLEARZ);
	if(!RsCameraBeginUpdate(Scene.camera))
		return;

	DefinedState(); // seems redundant, but breaks resolution change.
	RenderMenus();
	DoFade();
	Render2dStuffAfterFade();
//	CFont::DrawFonts(); // redundant
	DoRWStuffEndOfFrame();
}

bool
DoRWStuffStartOfFrame(int16 TopRed, int16 TopGreen, int16 TopBlue, int16 BottomRed, int16 BottomGreen, int16 BottomBlue, int16 Alpha)
{
	CRGBA TopColor(TopRed, TopGreen, TopBlue, Alpha);
	CRGBA BottomColor(BottomRed, BottomGreen, BottomBlue, Alpha);

	CameraSize(Scene.camera, nil, DEFAULT_VIEWWINDOW, SCREEN_ASPECT_RATIO);
	CVisibilityPlugins::SetRenderWareCamera(Scene.camera);
	RwCameraClear(Scene.camera, &gColourTop, rwCAMERACLEARZ);

	if(!RsCameraBeginUpdate(Scene.camera))
		return false;

	CSprite2d::InitPerFrame();

	if(Alpha != 0)
		CSprite2d::DrawRect(CRect(0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT), BottomColor, BottomColor, TopColor, TopColor);

	return true;
}

bool
DoRWStuffStartOfFrame_Horizon(int16 TopRed, int16 TopGreen, int16 TopBlue, int16 BottomRed, int16 BottomGreen, int16 BottomBlue, int16 Alpha)
{
	CameraSize(Scene.camera, nil, DEFAULT_VIEWWINDOW, SCREEN_ASPECT_RATIO);
	CVisibilityPlugins::SetRenderWareCamera(Scene.camera);
	RwCameraClear(Scene.camera, &gColourTop, rwCAMERACLEARZ);

	if(!RsCameraBeginUpdate(Scene.camera))
		return false;

	TheCamera.m_viewMatrix.Update();
	CClouds::RenderBackground(TopRed, TopGreen, TopBlue, BottomRed, BottomGreen, BottomBlue, Alpha);

	return true;
}

void
DoRWStuffEndOfFrame(void)
{
	CDebug::DisplayScreenStrings();	// custom
	CDebug::DebugDisplayTextBuffer();
	FlushObrsPrintfs();
	RwCameraEndUpdate(Scene.camera);
	RsCameraShowRaster(Scene.camera);
}


// This is certainly a very useful function
void
DoRWRenderHorizon(void)
{
	CClouds::RenderHorizon();
}

void
RenderScene(void)
{
	CClouds::Render();
	DoRWRenderHorizon();
	CRenderer::RenderRoads();
	CCoronas::RenderReflections();
	RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void*)TRUE);
	CRenderer::RenderEverythingBarRoads();
	CRenderer::RenderBoats();
	DefinedState();
	CWaterLevel::RenderWater();
	CRenderer::RenderFadingInEntities();
	CRenderer::RenderVehiclesButNotBoats();
	CWeather::RenderRainStreaks();
}

void
RenderDebugShit(void)
{
	CTheScripts::RenderTheScriptDebugLines();
	if(gbShowCollisionLines)
		CRenderer::RenderCollisionLines();
}

void
RenderEffects(void)
{
	CGlass::Render();
	CWaterCannons::Render();
	CSpecialFX::Render();
	CShadows::RenderStaticShadows();
	CShadows::RenderStoredShadows();
	CSkidmarks::Render();
	CAntennas::Render();
	CRubbish::Render();
	CCoronas::Render();
	CParticle::Render();
	CPacManPickups::Render();
	CWeaponEffects::Render();
	CPointLights::RenderFogEffect();
	CMovingThings::Render();
	CRenderer::RenderFirstPersonVehicle();
}

void
Render2dStuff(void)
{
	RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*)FALSE);
	RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)FALSE);
	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)TRUE);
	RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDSRCALPHA);
	RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDINVSRCALPHA);
	RwRenderStateSet(rwRENDERSTATECULLMODE, (void*)rwCULLMODECULLNONE);

	CReplay::Display();
	CPickups::RenderPickUpText();

	if(TheCamera.m_WideScreenOn)
		TheCamera.DrawBordersForWideScreen();

	CPed *player = FindPlayerPed();
	int weaponType = 0;
	if(player)
		weaponType = player->GetWeapon()->m_eWeaponType;

	bool firstPersonWeapon = false;
	int cammode = TheCamera.Cams[TheCamera.ActiveCam].Mode;
	if(cammode == CCam::MODE_SNIPER ||
	   cammode == CCam::MODE_SNIPER_RUNABOUT ||
	   cammode == CCam::MODE_ROCKETLAUNCHER ||
	   cammode == CCam::MODE_ROCKETLAUNCHER_RUNABOUT)
		firstPersonWeapon = true;

	// Draw black border for sniper and rocket launcher
	if((weaponType == WEAPONTYPE_SNIPERRIFLE || weaponType == WEAPONTYPE_ROCKETLAUNCHER) && firstPersonWeapon){
		CRGBA black(0, 0, 0, 255);

		// top and bottom strips
		if (weaponType == WEAPONTYPE_ROCKETLAUNCHER) {
			CSprite2d::DrawRect(CRect(0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT / 2 - SCREEN_SCALE_Y(180)), black);
			CSprite2d::DrawRect(CRect(0.0f, SCREEN_HEIGHT / 2 + SCREEN_SCALE_Y(170), SCREEN_WIDTH, SCREEN_HEIGHT), black);
		}
		else {
			CSprite2d::DrawRect(CRect(0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT / 2 - SCREEN_SCALE_Y(210)), black);
			CSprite2d::DrawRect(CRect(0.0f, SCREEN_HEIGHT / 2 + SCREEN_SCALE_Y(210), SCREEN_WIDTH, SCREEN_HEIGHT), black);
		}
		CSprite2d::DrawRect(CRect(0.0f, 0.0f, SCREEN_WIDTH / 2 - SCREEN_SCALE_X(210), SCREEN_HEIGHT), black);
		CSprite2d::DrawRect(CRect(SCREEN_WIDTH / 2 + SCREEN_SCALE_X(210), 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT), black);
	}

	MusicManager.DisplayRadioStationName();
	TheConsole.Display();
/*
	if(CSceneEdit::m_bEditOn)
		CSceneEdit::Draw();
	else
*/
		CHud::Draw();
	CUserDisplay::OnscnTimer.ProcessForDisplay();
	CMessages::Display();
	CDarkel::DrawMessages();
	CGarages::PrintMessages();
	CPad::PrintErrorMessage();
	CFont::DrawFonts();

	DebugMenuRender();
}

void
RenderMenus(void)
{
	if(FrontEndMenuManager.m_bMenuActive)
		FrontEndMenuManager.DrawFrontEnd();
}

bool &JustLoadedDontFadeInYet = *(bool*)0x95CDB4;
bool &StillToFadeOut = *(bool*)0x95CD99;
uint32 &TimeStartedCountingForFade = *(uint32*)0x9430EC;
uint32 &TimeToStayFadedBeforeFadeOut = *(uint32*)0x611564;

void
DoFade(void)
{
	if(CTimer::GetIsPaused())
		return;

	if(JustLoadedDontFadeInYet){
		JustLoadedDontFadeInYet = false;
		TimeStartedCountingForFade = CTimer::GetTimeInMilliseconds();
	}

	if(StillToFadeOut){
		if(CTimer::GetTimeInMilliseconds() - TimeStartedCountingForFade > TimeToStayFadedBeforeFadeOut){
			StillToFadeOut = false;
			TheCamera.Fade(3.0f, FADE_IN);
			TheCamera.ProcessFade();
			TheCamera.ProcessMusicFade();
		}else{
			TheCamera.SetFadeColour(0, 0, 0);
			TheCamera.Fade(0.0f, FADE_OUT);
			TheCamera.ProcessFade();
		}
	}

	if(CDraw::FadeValue != 0 || CMenuManager::m_PrefsBrightness < 256){
		CSprite2d *splash = LoadSplash(nil);

		CRGBA fadeColor;
		CRect rect;
		int fadeValue = CDraw::FadeValue;
		float brightness = min(CMenuManager::m_PrefsBrightness, 256);
		if(brightness <= 50)
			brightness = 50;
		if(FrontEndMenuManager.m_bMenuActive)
			brightness = 256;

		if(TheCamera.m_FadeTargetIsSplashScreen)
			fadeValue = 0;

		float fade = fadeValue + 256 - brightness;
		if(fade == 0){
			fadeColor.r = 0;
			fadeColor.g = 0;
			fadeColor.b = 0;
			fadeColor.a = 0;
		}else{
			fadeColor.r = fadeValue * CDraw::FadeRed / fade;
			fadeColor.g = fadeValue * CDraw::FadeGreen / fade;
			fadeColor.b = fadeValue * CDraw::FadeBlue / fade;
			int alpha = 255 - brightness*(256 - fadeValue)/256;
			if(alpha < 0)
				alpha = 0;
			fadeColor.a = alpha;
		}

		if(TheCamera.m_WideScreenOn){
			// what's this?
			float y = SCREEN_HEIGHT/2 * TheCamera.m_ScreenReductionPercentage/100.0f;
			rect.left = 0.0f;
			rect.right = SCREEN_WIDTH;
			rect.top = y - 8.0f;
			rect.bottom = SCREEN_HEIGHT - y - 8.0f;
		}else{
			rect.left = 0.0f;
			rect.right = SCREEN_WIDTH;
			rect.top = 0.0f;
			rect.bottom = SCREEN_HEIGHT;
		}
		CSprite2d::DrawRect(rect, fadeColor);

		if(CDraw::FadeValue != 0 && TheCamera.m_FadeTargetIsSplashScreen){
			fadeColor.r = 255;
			fadeColor.g = 255;
			fadeColor.b = 255;
			fadeColor.a = CDraw::FadeValue;
			splash->Draw(CRect(0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT), fadeColor, fadeColor, fadeColor, fadeColor);
		}
	}
}

float FramesPerSecondCounter;
int32 FrameSamples;

struct tZonePrint
{
  char name[12];
  CRect rect;
};

tZonePrint ZonePrint[] =
{
	{ "suburban", CRect(-1639.4f,  1014.3f, -226.23f, -1347.9f) },
	{ "comntop",  CRect(-223.52f,  203.62f,  616.79f, -413.6f)  },
	{ "comnbtm",  CRect(-227.24f, -413.6f,   620.51f, -911.84f) },
	{ "comse",    CRect( 200.35f, -911.84f,  620.51f, -1737.3f) },
	{ "comsw",    CRect(-223.52f, -911.84f,  200.35f, -1737.3f) },
	{ "industsw", CRect( 744.05f, -473.0f,   1067.5f, -1331.5f) },
	{ "industne", CRect( 1067.5f,  282.19f,  1915.3f, -473.0f)  },
	{ "industnw", CRect( 744.05f,  324.95f,  1067.5f, -473.0f)  },
	{ "industse", CRect( 1070.3f, -473.0f,   1918.1f, -1331.5f) },
	{ "no zone",  CRect( 0.0f,     0.0f,     0.0f,    0.0f)     }
};

#ifndef MASTER
void
DisplayGameDebugText()
{
	static bool bDisplayPosn = false;
	static bool bDisplayRate = false;

	{
		SETTWEAKPATH("GameDebugText");
		TWEAKBOOL(bDisplayPosn);
		TWEAKBOOL(bDisplayRate);
	}


	char str[200];
	wchar ustr[200];
	wchar ver[200];
	
	AsciiToUnicode(version_name, ver);

	CFont::SetPropOn();
	CFont::SetBackgroundOff();
	CFont::SetFontStyle(FONT_BANK);
	CFont::SetScale(SCREEN_STRETCH_X(0.5f), SCREEN_STRETCH_Y(0.5f));
	CFont::SetCentreOff();
	CFont::SetRightJustifyOff();
	CFont::SetWrapx(SCREEN_WIDTH);
	CFont::SetJustifyOff();
	CFont::SetBackGroundOnlyTextOff();
	CFont::SetColor(CRGBA(255, 108, 0, 255));
	CFont::PrintString(10.0f, 10.0f, ver);

	FrameSamples++;
	FramesPerSecondCounter += 1000.0f / (CTimer::GetTimeStepNonClippedInSeconds() * 1000.0f);	
	FramesPerSecond = FramesPerSecondCounter / FrameSamples;
	
	if ( FrameSamples > 30 )
	{
		FramesPerSecondCounter = 0.0f;
		FrameSamples = 0;
	}
  
	if ( !TheCamera.WorldViewerBeingUsed 
		&& CPad::GetPad(1)->GetSquare() 
		&& CPad::GetPad(1)->GetTriangle()
		&& CPad::GetPad(1)->GetLeftShoulder2JustDown() )
	{
		bDisplayPosn = !bDisplayPosn;
	}

	if ( CPad::GetPad(1)->GetSquare()
		&& CPad::GetPad(1)->GetTriangle()
		&& CPad::GetPad(1)->GetRightShoulder2JustDown() )
	{
		bDisplayRate = !bDisplayRate;
	}
	
	if ( bDisplayPosn || bDisplayRate )
	{
		CVector pos = FindPlayerCoors();
		int32 ZoneId = ARRAY_SIZE(ZonePrint)-1; // no zone
		
		for ( int32 i = 0; i < ARRAY_SIZE(ZonePrint)-1; i++ )
		{
			if ( pos.x > ZonePrint[i].rect.left
				&& pos.x < ZonePrint[i].rect.right
				&& pos.y > ZonePrint[i].rect.bottom
				&& pos.y < ZonePrint[i].rect.top )
			{
				ZoneId = i;
			}
		}

		//NOTE: fps should be 30, but its 29 due to different fp2int conversion 
		if ( bDisplayRate )
			sprintf(str, "X:%5.1f, Y:%5.1f, Z:%5.1f, F-%d, %s", pos.x, pos.y, pos.z, (int32)FramesPerSecond, ZonePrint[ZoneId].name);
		else
			sprintf(str, "X:%5.1f, Y:%5.1f, Z:%5.1f, %s", pos.x, pos.y, pos.z, ZonePrint[ZoneId].name);
		
		AsciiToUnicode(str, ustr);
		
		CFont::SetPropOff();
		CFont::SetBackgroundOff();
		CFont::SetScale(0.7f, 1.5f);
		CFont::SetCentreOff();
		CFont::SetRightJustifyOff();
		CFont::SetJustifyOff();
		CFont::SetBackGroundOnlyTextOff();
		CFont::SetWrapx(640.0f);
		CFont::SetFontStyle(FONT_HEADING);
		
		CFont::SetColor(CRGBA(0, 0, 0, 255));
		CFont::PrintString(42.0f, 42.0f, ustr);
		
		CFont::SetColor(CRGBA(255, 108, 0, 255));
		CFont::PrintString(40.0f, 40.0f, ustr);
	}
}
#endif

void
Render2dStuffAfterFade(void)
{
#ifndef MASTER
	DisplayGameDebugText();
	//PrintGameVersion();
#endif

	CHud::DrawAfterFade();
	CFont::DrawFonts();
}

CSprite2d splash;
int splashTxdId = -1;

CSprite2d*
LoadSplash(const char *name)
{
	RwTexDictionary *txd;
	char filename[140];
	RwTexture *tex = nil;

	if(name == nil)
		return &splash;
	if(splashTxdId == -1)
		splashTxdId = CTxdStore::AddTxdSlot("splash");

	txd = CTxdStore::GetSlot(splashTxdId)->texDict;
	if(txd)
		tex = RwTexDictionaryFindNamedTexture(txd, name);
	// if texture is found, splash was already set up below

	if(tex == nil){
		CFileMgr::SetDir("TXD\\");
		sprintf(filename, "%s.txd", name);
		if(splash.m_pTexture)
			splash.Delete();
		if(txd)
			CTxdStore::RemoveTxd(splashTxdId);
		CTxdStore::LoadTxd(splashTxdId, filename);
		CTxdStore::AddRef(splashTxdId);
		CTxdStore::PushCurrentTxd();
		CTxdStore::SetCurrentTxd(splashTxdId);
		splash.SetTexture(name);
		CTxdStore::PopCurrentTxd();
		CFileMgr::SetDir("");
	}

	return &splash;
}

void
DestroySplashScreen(void)
{
	splash.Delete();
	if(splashTxdId != -1)
		CTxdStore::RemoveTxdSlot(splashTxdId);
	splashTxdId = -1;
}

float NumberOfChunksLoaded;
#define TOTALNUMCHUNKS 73.0f

void
ResetLoadingScreenBar()
{
	NumberOfChunksLoaded = 0.0f;
}

// TODO: compare with PS2
void
LoadingScreen(const char *str1, const char *str2, const char *splashscreen)
{
	CSprite2d *splash;

#ifndef RANDOMSPLASH
	if(CGame::frenchGame || CGame::germanGame || !CGame::nastyGame)
		splashscreen = "mainsc2";
	else
		splashscreen = "mainsc1";
#endif

	splash = LoadSplash(splashscreen);

	if(RsGlobal.quit)
		return;

	if(DoRWStuffStartOfFrame(0, 0, 0, 0, 0, 0, 255)){
		CSprite2d::SetRecipNearClip();
		CSprite2d::InitPerFrame();
		CFont::InitPerFrame();
		DefinedState();
		RwRenderStateSet(rwRENDERSTATETEXTUREADDRESS, (void*)rwTEXTUREADDRESSCLAMP);
		splash->Draw(CRect(0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT), CRGBA(255, 255, 255, 255));

		if(str1){
			NumberOfChunksLoaded += 1;

			float hpos = SCREEN_SCALE_X(40);
			float length = SCREEN_WIDTH - SCREEN_SCALE_X(100);
			float vpos = SCREEN_HEIGHT - SCREEN_SCALE_Y(13);
			float height = SCREEN_SCALE_Y(7);
			CSprite2d::DrawRect(CRect(hpos, vpos, hpos + length, vpos + height), CRGBA(40, 53, 68, 255));

			length *= NumberOfChunksLoaded/TOTALNUMCHUNKS;
			CSprite2d::DrawRect(CRect(hpos, vpos, hpos + length, vpos + height), CRGBA(81, 106, 137, 255));

			// this is done by the game but is unused
			CFont::SetScale(SCREEN_SCALE_X(2), SCREEN_SCALE_Y(2));
			CFont::SetPropOn();
			CFont::SetRightJustifyOn();
			CFont::SetFontStyle(FONT_HEADING);

#ifdef CHATTYSPLASH
			// my attempt
			static wchar tmpstr[80];
			float yscale = SCREEN_SCALE_Y(0.9f);
			vpos -= 45*yscale;
			CFont::SetScale(SCREEN_SCALE_X(0.75f), yscale);
			CFont::SetPropOn();
			CFont::SetRightJustifyOff();
			CFont::SetFontStyle(FONT_BANK);
			CFont::SetColor(CRGBA(255, 255, 255, 255));
			AsciiToUnicode(str1, tmpstr);
			CFont::PrintString(hpos, vpos, tmpstr);
			vpos += 22*yscale;
			AsciiToUnicode(str2, tmpstr);
			CFont::PrintString(hpos, vpos, tmpstr);
#endif
		}

		CFont::DrawFonts();
 		DoRWStuffEndOfFrame();
	}
}

void
LoadingIslandScreen(const char *levelName)
{
	CSprite2d *splash;
	wchar *name;
	char str[100];
	wchar wstr[80];
	CRGBA col;

	splash = LoadSplash(nil);
	name = TheText.Get(levelName);
	if(!DoRWStuffStartOfFrame(0, 0, 0, 0, 0, 0, 255))
		return;

	CSprite2d::SetRecipNearClip();
	CSprite2d::InitPerFrame();
	CFont::InitPerFrame();
	DefinedState();
	col = CRGBA(255, 255, 255, 255);
	splash->Draw(CRect(0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT), col, col, col, col);
	CFont::SetBackgroundOff();
	CFont::SetScale(1.5f, 1.5f);
	CFont::SetPropOn();
	CFont::SetRightJustifyOn();
	CFont::SetRightJustifyWrap(150.0f);
	CFont::SetFontStyle(FONT_HEADING);
	sprintf(str, "WELCOME TO");
	AsciiToUnicode(str, wstr);
	CFont::SetDropColor(CRGBA(0, 0, 0, 255));
	CFont::SetDropShadowPosition(3);
	CFont::SetColor(CRGBA(243, 237, 71, 255));
	CFont::SetScale(SCREEN_STRETCH_X(1.2f), SCREEN_STRETCH_Y(1.2f));
	CFont::PrintString(SCREEN_WIDTH - 20, SCREEN_STRETCH_FROM_BOTTOM(110.0f), TheText.Get("WELCOME"));
	TextCopy(wstr, name);
	TheText.UpperCase(wstr);
	CFont::SetColor(CRGBA(243, 237, 71, 255));
	CFont::SetScale(SCREEN_STRETCH_X(1.2f), SCREEN_STRETCH_Y(1.2f));
	CFont::PrintString(SCREEN_WIDTH-20, SCREEN_STRETCH_FROM_BOTTOM(80.0f), wstr);
	CFont::DrawFonts();
	DoRWStuffEndOfFrame();
}

char*
GetLevelSplashScreen(int level)
{
	static char *splashScreens[4] = {
		nil,
		"splash1",
		"splash2",
		"splash3",
	};

	return splashScreens[level];
}

char*
GetRandomSplashScreen(void)
{
	int index;
	static int index2 = 0;
	static char splashName[128];
	static int splashIndex[24] = {
		25, 22, 4, 13,
		1, 21, 14, 16,
		10, 12, 5, 9,
		11, 18, 3, 2,
		19, 23, 7, 17,
		15, 6, 8, 20
	};

	index = splashIndex[4*index2 + CGeneral::GetRandomNumberInRange(0, 3)];
	index2++;
	if(index2 == 6)
		index2 = 0;
	sprintf(splashName, "loadsc%d", index);
	return splashName;
}

#include "rwcore.h"
#include "rpworld.h"
#include "rpmatfx.h"
#include "rpskin.h"
#include "rphanim.h"
#include "rtbmp.h"

_TODO("temp, move this includes out of here")

static RwBool 
PluginAttach(void)
{
	if( !RpWorldPluginAttach() )
	{
		printf("Couldn't attach world plugin\n");
		
		return FALSE;
	}
	
	if( !RpSkinPluginAttach() )
	{
		printf("Couldn't attach RpSkin plugin\n");
		
		return FALSE;
	}
	
	if( !RpHAnimPluginAttach() )
	{
		printf("Couldn't attach RpHAnim plugin\n");
		
		return FALSE;
	}
	
	if( !NodeNamePluginAttach() )
	{
		printf("Couldn't attach node name plugin\n");
		
		return FALSE;
	}
	
	if( !CVisibilityPlugins::PluginAttach() )
	{
		printf("Couldn't attach visibility plugins\n");
		
		return FALSE;
	}
	
	if( !RpAnimBlendPluginAttach() )
	{
		printf("Couldn't attach RpAnimBlend plugin\n");
		
		return FALSE;
	}
	
	if( !RpMatFXPluginAttach() )
	{
		printf("Couldn't attach RpMatFX plugin\n");
		
		return FALSE;
	}

	return TRUE;
}

static RwBool 
Initialise3D(void *param)
{
	if (RsRwInitialise(param))
	{
		//
		DebugMenuInit();
		DebugMenuPopulate();
		//

		return CGame::InitialiseRenderWare();
	}

	return (FALSE);
}


static void 
Terminate3D(void)
{
	CGame::ShutdownRenderWare();
	
	RsRwTerminate();

	return;
}

RsEventStatus
AppEventHandler(RsEvent event, void *param)
{
	switch( event )
	{
		case rsINITIALISE:
		{
			CGame::InitialiseOnceBeforeRW();
			return RsInitialise() ? rsEVENTPROCESSED : rsEVENTERROR;
		}

		case rsCAMERASIZE:
		{
											
			CameraSize(Scene.camera, (RwRect *)param,
				DEFAULT_VIEWWINDOW, DEFAULT_ASPECT_RATIO);
			
			return rsEVENTPROCESSED;
		}

		case rsRWINITIALISE:
		{
			return Initialise3D(param) ? rsEVENTPROCESSED : rsEVENTERROR;
		}

		case rsRWTERMINATE:
		{
			Terminate3D();

			return rsEVENTPROCESSED;
		}

		case rsTERMINATE:
		{
			CGame::FinalShutdown();

			return rsEVENTPROCESSED;
		}

		case rsPLUGINATTACH:
		{
			return PluginAttach() ? rsEVENTPROCESSED : rsEVENTERROR;
		}

		case rsINPUTDEVICEATTACH:
		{
			AttachInputDevices();

			return rsEVENTPROCESSED;
		}

		case rsIDLE:
		{
			Idle(param);

			return rsEVENTPROCESSED;
		}

		case rsFRONTENDIDLE:
		{
#ifdef PS2_SAVE_DIALOG
			Idle((void*)1);
#else
			FrontendIdle();
#endif

			return rsEVENTPROCESSED;
		}

		case rsACTIVATE:
		{
			param ? DMAudio.ReacquireDigitalHandle() : DMAudio.ReleaseDigitalHandle();

			return rsEVENTPROCESSED;
		}

#ifndef MASTER
		case rsANIMVIEWER:
		{
			TheModelViewer();

			return rsEVENTPROCESSED;
		}
#endif

		default:
		{
			return rsEVENTNOTPROCESSED;
		}
	}
}

void PrintGameVersion()
{
	CFont::SetPropOn();
	CFont::SetBackgroundOff();
	CFont::SetScale(SCREEN_SCALE_X(0.7f), SCREEN_SCALE_Y(0.5f));
	CFont::SetCentreOff();
	CFont::SetRightJustifyOff();
	CFont::SetBackGroundOnlyTextOff();
	CFont::SetFontStyle(FONT_BANK);
	CFont::SetWrapx(SCREEN_WIDTH);
	CFont::SetDropShadowPosition(0);
	CFont::SetDropColor(CRGBA(0, 0, 0, 255));
	CFont::SetColor(CRGBA(235, 170, 50, 255));

	strcpy(gString, "RE3");
	AsciiToUnicode(gString, gUString);
	CFont::PrintString(SCREEN_SCALE_X(10.5f), SCREEN_SCALE_Y(8.0f), gUString);
}

void
ValidateVersion()
{
	int32 file = CFileMgr::OpenFile("models\\coll\\peds.col", "rb");
	char buff[128];

	if ( file != -1 )
	{
		CFileMgr::Seek(file, 100, SEEK_SET);
		
		for ( int i = 0; i < 128; i++ )
		{
			CFileMgr::Read(file, &buff[i], sizeof(char));
			buff[i] -= 23;
			if ( buff[i] == '\0' )
				break;
			CFileMgr::Seek(file, 99, SEEK_CUR);
		}
		
		if ( !strncmp(buff, "grandtheftauto3", 15) )
		{
			strncpy(version_name, &buff[15], 64);
			CFileMgr::CloseFile(file);
			return;
		}
	}

	LoadingScreen("Invalid version", NULL, NULL);
	
	while(true)
	{
		;
	}
}

STARTPATCHES
	InjectHook(0x48E480, Idle, PATCH_JUMP);
	InjectHook(0x48E700, FrontendIdle, PATCH_JUMP);

	InjectHook(0x48CF10, DoRWStuffStartOfFrame, PATCH_JUMP);
	InjectHook(0x48D040, DoRWStuffStartOfFrame_Horizon, PATCH_JUMP);
	InjectHook(0x48E030, RenderScene, PATCH_JUMP);
	InjectHook(0x48E080, RenderDebugShit, PATCH_JUMP);
	InjectHook(0x48E090, RenderEffects, PATCH_JUMP);
	InjectHook(0x48E0E0, Render2dStuff, PATCH_JUMP);
	InjectHook(0x48E450, RenderMenus, PATCH_JUMP);
	InjectHook(0x48D120, DoFade, PATCH_JUMP);
	InjectHook(0x48E470, Render2dStuffAfterFade, PATCH_JUMP);

	InjectHook(0x48D550, LoadSplash, PATCH_JUMP);
	InjectHook(0x48D670, DestroySplashScreen, PATCH_JUMP);
	InjectHook(0x48D770, LoadingScreen, PATCH_JUMP);
	InjectHook(0x48D760, ResetLoadingScreenBar, PATCH_JUMP);
	
	InjectHook(0x48D470, PluginAttach, PATCH_JUMP);
	InjectHook(0x48D520, Initialise3D, PATCH_JUMP);
	InjectHook(0x48D540, Terminate3D, PATCH_JUMP);
	InjectHook(0x48E800, AppEventHandler, PATCH_JUMP);
ENDPATCHES
