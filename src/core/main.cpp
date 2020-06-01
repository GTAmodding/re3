#include "common.h"
#include "rpmatfx.h"
#include "rphanim.h"
#include "rpskin.h"

#include "main.h"
#include "CdStream.h"
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
#include "PathFind.h"
#include "Debug.h"
#include "Console.h"
#include "timebars.h"
#include "GenericGameStorage.h"
#include "SceneEdit.h"
#include "debugmenu.h"

GlobalScene Scene;

uint8 work_buff[55000];
char gString[256];
char gString2[512];
wchar gUString[256];
wchar gUString2[256];

float FramesPerSecond = 30.0f;

bool gbPrintShite = false;
bool gbModelViewer;

int32 frameCount;

RwRGBA gColourTop;

bool gameAlreadyInitialised;

float NumberOfChunksLoaded;
#define TOTALNUMCHUNKS 95.0f

bool g_SlowMode = false;
char version_name[64];


void GameInit(void);
void SystemInit(void);
void TheGame(void);

#ifdef DEBUGMENU
void DebugMenuPopulate(void);
#endif


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

bool
DoRWStuffStartOfFrame(int16 TopRed, int16 TopGreen, int16 TopBlue, int16 BottomRed, int16 BottomGreen, int16 BottomBlue, int16 Alpha)
{
	CRGBA TopColor(TopRed, TopGreen, TopBlue, Alpha);
	CRGBA BottomColor(BottomRed, BottomGreen, BottomBlue, Alpha);

	CDraw::CalculateAspectRatio();
	CameraSize(Scene.camera, nil, SCREEN_VIEWWINDOW, SCREEN_ASPECT_RATIO);
	CVisibilityPlugins::SetRenderWareCamera(Scene.camera);
	RwCameraClear(Scene.camera, &TopColor.rwRGBA, rwCAMERACLEARZ);

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
	CDraw::CalculateAspectRatio();
	CameraSize(Scene.camera, nil, SCREEN_VIEWWINDOW, SCREEN_ASPECT_RATIO);
	CVisibilityPlugins::SetRenderWareCamera(Scene.camera);
	RwCameraClear(Scene.camera, &gColourTop, rwCAMERACLEARZ);

	if(!RsCameraBeginUpdate(Scene.camera))
		return false;

	TheCamera.m_viewMatrix.Update();
	CClouds::RenderBackground(TopRed, TopGreen, TopBlue, BottomRed, BottomGreen, BottomBlue, Alpha);

	return true;
}

// This is certainly a very useful function
void
DoRWRenderHorizon(void)
{
	CClouds::RenderHorizon();
}

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

	if(CDraw::FadeValue != 0 || FrontEndMenuManager.m_PrefsBrightness < 256){
		CSprite2d *splash = LoadSplash(nil);

		CRGBA fadeColor;
		CRect rect;
		int fadeValue = CDraw::FadeValue;
		float brightness = Min(FrontEndMenuManager.m_PrefsBrightness, 256);
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
#ifdef FIX_BUGS
			rect.top = y - SCREEN_SCALE_Y(8.0f);
			rect.bottom = SCREEN_HEIGHT - y - SCREEN_SCALE_Y(8.0f);
#else
			rect.top = y - 8.0f;
			rect.bottom = SCREEN_HEIGHT - y - 8.0f;
#endif // FIX_BUGS
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

void
DoRWStuffEndOfFrame(void)
{
	CDebug::DisplayScreenStrings();	// custom
	CDebug::DebugDisplayTextBuffer();
	FlushObrsPrintfs();
	RwCameraEndUpdate(Scene.camera);
	RsCameraShowRaster(Scene.camera);
}

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
	if (RsRwInitialize(param))
	{
#ifdef DEBUGMENU
		DebugMenuInit();
		DebugMenuPopulate();
#endif // !DEBUGMENU
		return CGame::InitialiseRenderWare();
	}

	return (FALSE);
}

static void 
Terminate3D(void)
{
	CGame::ShutdownRenderWare();
#ifdef DEBUGMENU
	DebugMenuShutdown();
#endif // !DEBUGMENU
	
	RsRwTerminate();

	return;
}

CSprite2d splash;
int splashTxdId = -1;

//--MIAMI: done
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

//--MIAMI: done
Const char*
GetRandomSplashScreen(void)
{
	int index;
	static int index2 = 0;
	static char splashName[128];
	static int splashIndex[12] = {
		1, 2,
		3, 4,
		5, 11,
		6, 8,
		9, 10,
		7, 12
	};

	index = splashIndex[2*index2 + CGeneral::GetRandomNumberInRange(0, 2)];
	index2++;
	if(index2 == 6)
		index2 = 0;
	sprintf(splashName, "loadsc%d", index);
	return splashName;
}

Const char*
GetLevelSplashScreen(int level)
{
	static Const char *splashScreens[4] = {
		nil,
		"splash1",
		"splash2",
		"splash3",
	};

	return splashScreens[level];
}

void
ResetLoadingScreenBar()
{
	NumberOfChunksLoaded = 0.0f;
}

//--MIAMI: done
void
LoadingScreen(const char *str1, const char *str2, const char *splashscreen)
{
	CSprite2d *splash;

#ifndef RANDOMSPLASH
	splashscreen = "LOADSC0";
#endif

	splash = LoadSplash(splashscreen);

#ifndef GTA_PS2
	if(RsGlobal.quit)
		return;
#endif

	if(DoRWStuffStartOfFrame(0, 0, 0, 0, 0, 0, 255)){
		CSprite2d::SetRecipNearClip();
		CSprite2d::InitPerFrame();
		CFont::InitPerFrame();
		DefinedState();
		RwRenderStateSet(rwRENDERSTATETEXTUREADDRESS, (void*)rwTEXTUREADDRESSCLAMP);
		splash->Draw(CRect(0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT), CRGBA(255, 255, 255, 255));

		if(str1){
			NumberOfChunksLoaded += 1;

#ifndef RANDOMSPLASH
			float hpos = SCREEN_SCALE_X(40);
			float length = SCREEN_WIDTH - SCREEN_SCALE_X(80);
			float top = SCREEN_HEIGHT - SCREEN_SCALE_Y(14);
			float bottom = top + SCREEN_SCALE_Y(5);
#else
			float hpos = SCREEN_STRETCH_X(40);
			float length = SCREEN_STRETCH_X(440);
			// this is rather weird
			float top = SCREEN_STRETCH_Y(407.4f - 7.0f/3.0f);
			float bottom = SCREEN_STRETCH_Y(407.4f + 7.0f/3.0f);
#endif

			CSprite2d::DrawRect(CRect(hpos-1.0f, top-1.0f, hpos+length+1.0f, bottom+1.0f), CRGBA(40, 53, 68, 255));

			CSprite2d::DrawRect(CRect(hpos, top, hpos+length, bottom), CRGBA(155, 50, 125, 255));

			length *= NumberOfChunksLoaded/TOTALNUMCHUNKS;
			CSprite2d::DrawRect(CRect(hpos, top, hpos+length, bottom), CRGBA(255, 150, 225, 255));

			// this is done by the game but is unused
			CFont::SetBackgroundOff();
			CFont::SetScale(SCREEN_SCALE_X(2), SCREEN_SCALE_Y(2));
			CFont::SetPropOn();
			CFont::SetRightJustifyOn();
			CFont::SetDropShadowPosition(1);
			CFont::SetDropColor(CRGBA(0, 0, 0, 255));
			CFont::SetFontStyle(FONT_HEADING);

#ifdef CHATTYSPLASH
			// my attempt
			static wchar tmpstr[80];
			float yscale = SCREEN_SCALE_Y(0.9f);
			top -= 45*yscale;
			CFont::SetScale(SCREEN_SCALE_X(0.75f), yscale);
			CFont::SetPropOn();
			CFont::SetRightJustifyOff();
			CFont::SetFontStyle(FONT_BANK);
			CFont::SetColor(CRGBA(255, 255, 255, 255));
			AsciiToUnicode(str1, tmpstr);
			CFont::PrintString(hpos, top, tmpstr);
			top += 22*yscale;
			AsciiToUnicode(str2, tmpstr);
			CFont::PrintString(hpos, top, tmpstr);
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

void
ProcessSlowMode(void)
{  
	int16 lX = CPad::GetPad(0)->NewState.LeftStickX;
	int16 lY = CPad::GetPad(0)->NewState.LeftStickY;
	int16 rX = CPad::GetPad(0)->NewState.RightStickX;
	int16 rY = CPad::GetPad(0)->NewState.RightStickY;
	int16 L1 = CPad::GetPad(0)->NewState.LeftShoulder1;
	int16 L2 = CPad::GetPad(0)->NewState.LeftShoulder2;
	int16 R1 = CPad::GetPad(0)->NewState.RightShoulder1;
	int16 R2 = CPad::GetPad(0)->NewState.RightShoulder2;
	int16 up = CPad::GetPad(0)->NewState.DPadUp;
	int16 down = CPad::GetPad(0)->NewState.DPadDown;
	int16 left = CPad::GetPad(0)->NewState.DPadLeft;
	int16 right = CPad::GetPad(0)->NewState.DPadRight;
	int16 start = CPad::GetPad(0)->NewState.Start;
	int16 select = CPad::GetPad(0)->NewState.Select;
	int16 square = CPad::GetPad(0)->NewState.Square;
	int16 triangle = CPad::GetPad(0)->NewState.Triangle;
	int16 cross = CPad::GetPad(0)->NewState.Cross;
	int16 circle = CPad::GetPad(0)->NewState.Circle;
	int16 L3 = CPad::GetPad(0)->NewState.LeftShock;
	int16 R3 = CPad::GetPad(0)->NewState.RightShock;
	int16 networktalk = CPad::GetPad(0)->NewState.NetworkTalk;
	int16 stop = true;
	
	do
	{
		if ( CPad::GetPad(1)->GetLeftShoulder1JustDown() || CPad::GetPad(1)->GetRightShoulder1() )
			break;
		
		if ( stop )
		{
			CTimer::Stop();
			stop = false;
		}
		
		CPad::UpdatePads();
		
		RwCameraBeginUpdate(Scene.camera);
		RwCameraEndUpdate(Scene.camera);
		
		if ( CPad::GetPad(1)->GetLeftShoulder1JustDown() || CPad::GetPad(1)->GetRightShoulder1() )
			break;
	
	} while (!CPad::GetPad(1)->GetRightShoulder1());
	
	
	CPad::GetPad(0)->OldState.LeftStickX = lX;
	CPad::GetPad(0)->OldState.LeftStickY = lY;
	CPad::GetPad(0)->OldState.RightStickX = rX;
	CPad::GetPad(0)->OldState.RightStickY = rY;
	CPad::GetPad(0)->OldState.LeftShoulder1 = L1;
	CPad::GetPad(0)->OldState.LeftShoulder2 = L2;
	CPad::GetPad(0)->OldState.RightShoulder1 = R1;
	CPad::GetPad(0)->OldState.RightShoulder2 = R2;
	CPad::GetPad(0)->OldState.DPadUp = up;
	CPad::GetPad(0)->OldState.DPadDown = down;
	CPad::GetPad(0)->OldState.DPadLeft = left;
	CPad::GetPad(0)->OldState.DPadRight = right;
	CPad::GetPad(0)->OldState.Start = start;
	CPad::GetPad(0)->OldState.Select = select;
	CPad::GetPad(0)->OldState.Square = square;
	CPad::GetPad(0)->OldState.Triangle = triangle;
	CPad::GetPad(0)->OldState.Cross = cross;
	CPad::GetPad(0)->OldState.Circle = circle;
	CPad::GetPad(0)->OldState.LeftShock = L3;
	CPad::GetPad(0)->OldState.RightShock = R3;
	CPad::GetPad(0)->OldState.NetworkTalk = networktalk;
	CPad::GetPad(0)->NewState.LeftStickX = lX;
	CPad::GetPad(0)->NewState.LeftStickY = lY;
	CPad::GetPad(0)->NewState.RightStickX = rX;
	CPad::GetPad(0)->NewState.RightStickY = rY;
	CPad::GetPad(0)->NewState.LeftShoulder1 = L1;
	CPad::GetPad(0)->NewState.LeftShoulder2 = L2;
	CPad::GetPad(0)->NewState.RightShoulder1 = R1;
	CPad::GetPad(0)->NewState.RightShoulder2 = R2;
	CPad::GetPad(0)->NewState.DPadUp = up;
	CPad::GetPad(0)->NewState.DPadDown = down;
	CPad::GetPad(0)->NewState.DPadLeft = left;
	CPad::GetPad(0)->NewState.DPadRight = right;
	CPad::GetPad(0)->NewState.Start = start;
	CPad::GetPad(0)->NewState.Select = select;
	CPad::GetPad(0)->NewState.Square = square;
	CPad::GetPad(0)->NewState.Triangle = triangle;
	CPad::GetPad(0)->NewState.Cross = cross;
	CPad::GetPad(0)->NewState.Circle = circle;
	CPad::GetPad(0)->NewState.LeftShock = L3;
	CPad::GetPad(0)->NewState.RightShock = R3;
	CPad::GetPad(0)->NewState.NetworkTalk = networktalk;
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
	static bool bDisplayCheatStr = false;

	{
		SETTWEAKPATH("GameDebugText");
		TWEAKBOOL(bDisplayPosn);
		TWEAKBOOL(bDisplayRate);
		TWEAKBOOL(bDisplayCheatStr);
	}


	char str[200];
	wchar ustr[200];
	wchar ver[200];
	
	AsciiToUnicode(version_name, ver);

	CFont::SetPropOn();
	CFont::SetBackgroundOff();
	CFont::SetFontStyle(FONT_BANK);
	CFont::SetScale(SCREEN_SCALE_X(0.5f), SCREEN_SCALE_Y(0.5f));
	CFont::SetCentreOff();
	CFont::SetRightJustifyOff();
	CFont::SetWrapx(SCREEN_WIDTH);
	CFont::SetJustifyOff();
	CFont::SetBackGroundOnlyTextOff();
	CFont::SetColor(CRGBA(255, 108, 0, 255));
	CFont::PrintString(SCREEN_SCALE_X(10.0f), SCREEN_SCALE_Y(10.0f), ver);

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
		
		// Let's not scale those numbers, they look better that way :eyes:
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

	if (bDisplayCheatStr)
	{
		sprintf(str, "%s", CPad::KeyBoardCheatString);
		AsciiToUnicode(str, ustr);

		CFont::SetPropOff();
		CFont::SetBackgroundOff();
		CFont::SetScale(0.7f, 1.5f);
		CFont::SetCentreOn();
		CFont::SetBackGroundOnlyTextOff();
		CFont::SetWrapx(640.0f);
		CFont::SetFontStyle(FONT_HEADING);

		CFont::SetColor(CRGBA(0, 0, 0, 255));
		CFont::PrintString(SCREEN_SCALE_X(DEFAULT_SCREEN_WIDTH * 0.5f)+2.f, SCREEN_SCALE_FROM_BOTTOM(20.0f)+2.f, ustr);

		CFont::SetColor(CRGBA(255, 150, 225, 255));
		CFont::PrintString(SCREEN_SCALE_X(DEFAULT_SCREEN_WIDTH * 0.5f), SCREEN_SCALE_FROM_BOTTOM(20.0f), ustr);
	}
}
#endif

void
RenderScene(void)
{
	CClouds::Render();
	DoRWRenderHorizon();
	CRenderer::RenderRoads();
	CCoronas::RenderReflections();
	CRenderer::RenderEverythingBarRoads();
	RwRenderStateSet(rwRENDERSTATECULLMODE, (void*)rwCULLMODECULLNONE);
	CWaterLevel::RenderWater();
	CRenderer::RenderBoats();
	CRenderer::RenderFadingInUnderwaterEntities();
	RwRenderStateSet(rwRENDERSTATECULLMODE, (void*)rwCULLMODECULLNONE);
	CWaterLevel::RenderTransparentWater();
	CRenderer::RenderFadingInEntities();
	RwRenderStateSet(rwRENDERSTATECULLMODE, (void*)rwCULLMODECULLNONE);
	CWeather::RenderRainStreaks();
	// CCoronas::RenderSunReflection
}

void
RenderDebugShit(void)
{
	CTheScripts::RenderTheScriptDebugLines();
#ifndef FINAL
	if(gbShowCollisionLines)
		CRenderer::RenderCollisionLines();
	ThePaths.DisplayPathData();
	CDebug::DrawLines();
	DefinedState();
#endif
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
	if(CSceneEdit::m_bEditOn)
		CSceneEdit::Draw();
	else
		CHud::Draw();
	CUserDisplay::OnscnTimer.ProcessForDisplay();
	CMessages::Display();
	CDarkel::DrawMessages();
	CGarages::PrintMessages();
	CPad::PrintErrorMessage();
	CFont::DrawFonts();

#ifdef DEBUGMENU
	DebugMenuRender();
#endif
}

void
RenderMenus(void)
{
#ifdef PS2
	if (FrontEndMenuManager.m_bWantToDraw)
	{
		gMainHeap.PushMemId(_TODOCONST(17));
		FrontEndMenuManager.DrawFrontEnd();
		gMainHeap.PopMemId();
	}
#else
	if(FrontEndMenuManager.m_bMenuActive)
		FrontEndMenuManager.DrawFrontEnd();
#endif
}

void
Render2dStuffAfterFade(void)
{
#ifndef MASTER
	DisplayGameDebugText();
#endif

	CHud::DrawAfterFade();
	CFont::DrawFonts();
	CCredits::Render();
}

void
Idle(void *arg)
{
	CTimer::Update();

#ifdef TIMEBARS
	tbInit();
#endif

	CSprite2d::InitPerFrame();
	CFont::InitPerFrame();

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

	if(CGame::bDemoMode && CTimer::GetTimeInMilliseconds() > (3*60 + 30)*1000 && !CCutsceneMgr::IsCutsceneProcessing()){
		FrontEndMenuManager.m_bWantToRestart = true;
		FrontEndMenuManager.m_bWantToLoad = false;
		return;
	}

	if(FrontEndMenuManager.m_bWantToRestart || b_FoundRecentSavedGameWantToLoad)
		return;

	SetLightsWithTimeOfDayColour(Scene.world);

	if(arg == nil)
		return;

	// m_bRenderGameInMenu is there in III PS2 but I don't know about VC PS2.
	if((!FrontEndMenuManager.m_bMenuActive/* || FrontEndMenuManager.m_bRenderGameInMenu*/) &&
	   TheCamera.GetScreenFadeStatus() != FADE_2)
	{
#ifdef GTA_PC
			// This is from SA, but it's nice for windowed mode
			RwV2d pos;
			pos.x = SCREEN_WIDTH / 2.0f;
			pos.y = SCREEN_HEIGHT / 2.0f;
			RsMouseSetPos(&pos);
#endif
#ifdef TIMEBARS
		tbStartTimer(0, "CnstrRenderList");
#endif
#ifdef PC_WATER
		CWaterLevel::PreCalcWaterGeometry();
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
		CDraw::CalculateAspectRatio();
#ifdef ASPECT_RATIO_SCALE
		CameraSize(Scene.camera, nil, SCREEN_VIEWWINDOW, SCREEN_ASPECT_RATIO);
#else
		CameraSize(Scene.camera, nil, SCREEN_VIEWWINDOW, DEFAULT_ASPECT_RATIO);
#endif
		CVisibilityPlugins::SetRenderWareCamera(Scene.camera);
		RwCameraClear(Scene.camera, &gColourTop, rwCAMERACLEARZ);
		if(!RsCameraBeginUpdate(Scene.camera))
			return;
	}

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
	// CCredits::Render(); // They added it to function above and also forgot it here

#ifdef TIMEBARS
	tbDisplay();
#endif

	DoRWStuffEndOfFrame();

	if(g_SlowMode) 
		ProcessSlowMode();
}

void
FrontendIdle(void)
{
	CDraw::CalculateAspectRatio();
	CTimer::Update();
	CSprite2d::SetRecipNearClip(); // this should be on InitialiseRenderWare according to PS2 asm. seems like a bug fix
	CSprite2d::InitPerFrame();
	CFont::InitPerFrame();
	CPad::UpdatePads();
	FrontEndMenuManager.Process();

	if(RsGlobal.quit)
		return;

	CameraSize(Scene.camera, nil, SCREEN_VIEWWINDOW, SCREEN_ASPECT_RATIO);
	CVisibilityPlugins::SetRenderWareCamera(Scene.camera);
	RwCameraClear(Scene.camera, &gColourTop, rwCAMERACLEARZ);
	if(!RsCameraBeginUpdate(Scene.camera))
		return;

	DefinedState(); // seems redundant, but breaks resolution change.
	RenderMenus();
	DoFade();
	Render2dStuffAfterFade();
	CFont::DrawFonts();
	DoRWStuffEndOfFrame();
}

void
InitialiseGame(void)
{
	LoadingScreen(nil, nil, "loadsc0");
	CGame::Initialise("DATA\\GTA_VC.DAT");
}

RsEventStatus
AppEventHandler(RsEvent event, void *param)
{
	switch( event )
	{
		case rsINITIALIZE:
		{
			CGame::InitialiseOnceBeforeRW();
			return RsInitialize() ? rsEVENTPROCESSED : rsEVENTERROR;
		}

		case rsCAMERASIZE:
		{
											
			CameraSize(Scene.camera, (RwRect *)param,
				SCREEN_VIEWWINDOW, DEFAULT_ASPECT_RATIO);
			
			return rsEVENTPROCESSED;
		}

		case rsRWINITIALIZE:
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
			FrontendIdle();

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

#ifndef MASTER
void
TheModelViewer(void)
{
#if (defined(GTA_PS2) || defined(GTA_XBOX))
	//TODO
#else

	CDraw::CalculateAspectRatio();
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
#endif
}
#endif

void TheGame(void)
{
	printf("Into TheGame!!!\n");

#ifdef GTA_PS2
	gMainHeap.PushMemId(_TODOCONST(1));
#endif

	CTimer::Initialise();

#ifdef GTA_PS2
	CGame::Initialise();
#else
	CGame::Initialise("DATA\\GTA3.DAT");
#endif

	Const char *splash = GetRandomSplashScreen(); // inlined here

	LoadingScreen("Starting Game", NULL, splash);

#ifdef GTA_PS2
	if (   TheMemoryCard.CheckCardInserted(_TODOCONST(0)) == _TODOCONST(26)
		&& TheMemoryCard.ChangeDirectory(_TODOCONST(0), TheMemoryCard.field154)
		&& TheMemoryCard.FindMostRecentFileName(_TODOCONST(0), TheMemoryCard.field37) == 1
		&& TheMemoryCard.CheckDataNotCorrupt(TheMemoryCard.field37))
	{
		strcpy(TheMemoryCard.LoadFileName, TheMemoryCard.field37);
		TheMemoryCard.b_FoundRecentSavedGameWantToLoad = true;

		if (FrontEndMenuManager.m_PrefsLanguage != TheMemoryCard.GetLanguageToLoad())
		{
			FrontEndMenuManager.m_PrefsLanguage = TheMemoryCard.GetLanguageToLoad();
			TheText.Unload();
			TheText.Load();
		}

		CGame::currLevel = TheMemoryCard.GetLevelToLoad();
	}
#else
	//TODO
#endif

	while (true)
	{
#ifdef PS2
		if (TheMemoryCard.m_bWantToLoad)
#else
		if (FrontEndMenuManager.m_bWantToLoad)
#endif
		{
			Const char *splash1 = GetLevelSplashScreen(CGame::currLevel);
			LoadSplash(splash1);
		}

#ifdef PS2
		TheMemoryCard.m_bWantToLoad = false;
#else
		FrontEndMenuManager.m_bWantToLoad = false;
#endif

		CTimer::Update();

#ifdef PS2
		while (!(FrontEndMenuManager.m_bWantToRestart || TheMemoryCard.b_FoundRecentSavedGameWantToLoad))
#else
		while (!(FrontEndMenuManager.m_bWantToRestart || b_FoundRecentSavedGameWantToLoad))
#endif
		{
			CSprite2d::InitPerFrame();
			CFont::InitPerFrame();

#ifdef GTA_PS2
			gMainHeap.PushMemId(_TODOCONST(12));
#endif
			CPointLights::NumLights = 0;
			CGame::Process();
#ifdef GTA_PS2
			gMainHeap.PopMemId();
#endif

			DMAudio.Service();

			if (CGame::bDemoMode && CTimer::GetTimeInMilliseconds() > (3*60 + 30)*1000 && !CCutsceneMgr::IsCutsceneProcessing())
			{
#ifdef PS2
				TheMemoryCard.m_bWantToLoad = false;
#else
				FrontEndMenuManager.m_bWantToLoad = false;
#endif
				FrontEndMenuManager.m_bWantToRestart = true;
				break;
			}

#ifdef PS2
			if (FrontEndMenuManager.m_bWantToRestart || TheMemoryCard.b_FoundRecentSavedGameWantToLoad)
#else
			if (FrontEndMenuManager.m_bWantToRestart || b_FoundRecentSavedGameWantToLoad)
#endif
				break;

			SetLightsWithTimeOfDayColour(Scene.world);
#ifdef GTA_PS2
			gMainHeap.PushMemId(_TODOCONST(15));
#endif

			// m_bRenderGameInMenu is there in III PS2 but I don't know about VC PS2.
			if (!FrontEndMenuManager.m_bMenuActive || /*FrontEndMenuManager.m_bRenderGameInMenu == true && */TheCamera.GetScreenFadeStatus() != FADE_2 )
			{
#ifdef GTA_PS2
				gMainHeap.PushMemId(_TODOCONST(11));
#endif
				CRenderer::ConstructRenderList();
				CRenderer::PreRender();
#ifdef GTA_PS2
				gMainHeap.PopMemId();
#endif

				if (CWeather::LightningFlash && !CCullZones::CamNoRain())
					DoRWStuffStartOfFrame_Horizon(255, 255, 255, 255, 255, 255, 255);
				else
					DoRWStuffStartOfFrame_Horizon(CTimeCycle::GetSkyTopRed(), CTimeCycle::GetSkyTopGreen(), CTimeCycle::GetSkyTopBlue(), CTimeCycle::GetSkyBottomRed(), CTimeCycle::GetSkyBottomGreen(), CTimeCycle::GetSkyBottomBlue(), 255);

				DefinedState();
				RwCameraSetFarClipPlane(Scene.camera, CTimeCycle::GetFarClip());
				RwCameraSetFogDistance(Scene.camera, CTimeCycle::GetFogStart());

				RenderScene();
				RenderDebugShit();
				RenderEffects();

				if ((TheCamera.m_BlurType == MBLUR_NONE || TheCamera.m_BlurType == MBLUR_NORMAL) && TheCamera.m_ScreenReductionPercentage > 0.0f)
					TheCamera.SetMotionBlurAlpha(150);
				TheCamera.RenderMotionBlur();

				Render2dStuff();
			}
			else
			{
				CameraSize(Scene.camera, NULL, SCREEN_VIEWWINDOW, SCREEN_ASPECT_RATIO);
				CVisibilityPlugins::SetRenderWareCamera(Scene.camera);
				RwCameraClear(Scene.camera, &gColourTop, rwCAMERACLEARZ);
				if (!RsCameraBeginUpdate(Scene.camera))
					break;
			}

			RenderMenus();

#ifdef PS2
			if (TheMemoryCard.m_bWantToLoad)
#else
			if (FrontEndMenuManager.m_bWantToLoad)
#endif
			{
#ifdef GTA_PS2
				gMainHeap.PopMemId();
#endif
				break;
			}

			DoFade();
			Render2dStuffAfterFade();
			CCredits::Render();

			DoRWStuffEndOfFrame();

			while (frameCount < 2)
				;

			frameCount = 0;

			CTimer::Update();

#ifdef GTA_PS2
			gMainHeap.PopMemId();
#endif

			if (g_SlowMode)
				ProcessSlowMode();
		}

		CPad::ResetCheats();
		CPad::StopPadsShaking();
		DMAudio.ChangeMusicMode(MUSICMODE_DISABLE);
		CGame::ShutDownForRestart();
		CTimer::Stop();

#ifdef PS2
		if (FrontEndMenuManager.m_bWantToRestart || TheMemoryCard.b_FoundRecentSavedGameWantToLoad)
#else
		if (FrontEndMenuManager.m_bWantToRestart || b_FoundRecentSavedGameWantToLoad)
#endif
		{
#ifdef PS2
			if (TheMemoryCard.b_FoundRecentSavedGameWantToLoad)
#else
			if (b_FoundRecentSavedGameWantToLoad)
#endif
			{
				FrontEndMenuManager.m_bWantToRestart = true;
#ifdef PS2
				TheMemoryCard.m_bWantToLoad = true;
#else
				FrontEndMenuManager.m_bWantToLoad = true;
#endif
			}

			CGame::InitialiseWhenRestarting();
			DMAudio.ChangeMusicMode(MUSICMODE_GAME);
			FrontEndMenuManager.m_bWantToRestart = false;

			continue;
		}

		break;
	}

	DMAudio.Terminate();
}


void SystemInit()
{
#ifdef __MWERKS__
	mwInit();
#endif
	
#ifdef GTA_PS2
	InitMemoryMgr();
#endif
	
#ifdef GTA_PS2
	CFileMgr::InitCdSystem();
	
	char path[256];
	
	sprintf(path, "cdrom0:\\%s%s;1", "SYSTEM\\", "IOPRP23.IMG");
	
	sceSifInitRpc(0);
	
	while ( !sceSifRebootIop(path) )
		;
	while( !sceSifSyncIop() )
		;
	
	sceSifInitRpc(0);
	
	CFileMgr::InitCdSystem();
	
	sceFsReset();
#endif

	CFileMgr::Initialise();
	
#ifdef GTA_PS2
	CFileMgr::InitCd();
	
	Char modulepath[256];
	
	strcpy(modulepath, "cdrom0:\\");
	strcat(modulepath, "SYSTEM\\");
	strcat(modulepath, "SIO2MAN.IRX");
	LoadModule(modulepath);
	
	strcpy(modulepath, "cdrom0:\\");
	strcat(modulepath, "SYSTEM\\");
	strcat(modulepath, "PADMAN.IRX");
	LoadModule(modulepath);
	
	strcpy(modulepath, "cdrom0:\\");
	strcat(modulepath, "SYSTEM\\");
	strcat(modulepath, "LIBSD.IRX");
	LoadModule(modulepath);
	
	strcpy(modulepath, "cdrom0:\\");
	strcat(modulepath, "SYSTEM\\");
	strcat(modulepath, "SDRDRV.IRX");
	LoadModule(modulepath);
	
	strcpy(modulepath, "cdrom0:\\");
	strcat(modulepath, "SYSTEM\\");
	strcat(modulepath, "MCMAN.IRX");
	LoadModule(modulepath);
	
	strcpy(modulepath, "cdrom0:\\");
	strcat(modulepath, "SYSTEM\\");
	strcat(modulepath, "MCSERV.IRX");
	LoadModule(modulepath);
#endif
	

#ifdef GTA_PS2
	ThreadParam param;
	
	param.entry = &IdleThread;
	param.stack = idleThreadStack;
	param.stackSize = 2048;
	param.initPriority = 127;
	param.gpReg = &_gp;
	
	int thread = CreateThread(&param);
	StartThread(thread, NULL);
#else
	//
#endif
	
#ifdef GTA_PS2_STUFF
	CPad::Initialise();
#endif
	CPad::GetPad(0)->Mode = 0;
	
	CGame::frenchGame = false;
	CGame::germanGame = false;
	CGame::nastyGame = true;
	FrontEndMenuManager.m_PrefsAllowNastyGame = true;
	
#ifdef GTA_PS2
	int32 lang = sceScfGetLanguage();
	if ( lang  == SCE_ITALIAN_LANGUAGE )
		FrontEndMenuManager.m_PrefsLanguage = LANGUAGE_ITALIAN;
	else if ( lang  == SCE_SPANISH_LANGUAGE )
		FrontEndMenuManager.m_PrefsLanguage = LANGUAGE_SPANISH;
	else if ( lang  == SCE_GERMAN_LANGUAGE )
	{
		CGame::germanGame = true;
		CGame::nastyGame = false;
		FrontEndMenuManager.m_PrefsAllowNastyGame = false;
		FrontEndMenuManager.m_PrefsLanguage = LANGUAGE_GERMAN;
	}
	else if ( lang  == SCE_FRENCH_LANGUAGE )
	{
		CGame::frenchGame = true;
		CGame::nastyGame = false;
		FrontEndMenuManager.m_PrefsAllowNastyGame = false;
		FrontEndMenuManager.m_PrefsLanguage = LANGUAGE_FRENCH;
	}
	else
		FrontEndMenuManager.m_PrefsLanguage = LANGUAGE_AMERICAN;
	
	FrontEndMenuManager.InitialiseMenuContentsAfterLoadingGame();
#else
	//
#endif
	
#ifdef PS2
	TheMemoryCard.Init();
#endif
}

void GameInit()
{
	if ( !gameAlreadyInitialised )
	{
#ifdef GTA_PS2
		char path[256];
		
		strcpy(path, "cdrom0:\\");
		strcat(path, "SYSTEM\\");
		strcat(path, "CDSTREAM.IRX");
		LoadModule(path);
		
		strcpy(path, "cdrom0:\\");
		strcat(path, "SYSTEM\\");
		strcat(path, "SAMPMAN.IRX");
		LoadModule(path);
		
		strcpy(path, "cdrom0:\\");
		strcat(path, "SYSTEM\\");
		strcat(path, "MUSICSTR.IRX");
		LoadModule(path);
#endif
		CdStreamInit(MAX_CDCHANNELS);
		
#ifdef PS2
		Initialise3D(); //no params
#else
		//TODO
#endif
		
#ifdef GTA_PS2
		char *files[] =
		{
			"\\ANIM\\CUTS.IMG;1",
			"\\ANIM\\CUTS.DIR;1",
			"\\ANIM\\PED.IFP;1",
			"\\MODELS\\FRONTEND.TXD;1",
			"\\MODELS\\FONTS.TXD;1",
			"\\MODELS\\HUD.TXD;1",
			"\\MODELS\\PARTICLE.TXD;1",
			"\\MODELS\\MISC.TXD;1",
			"\\MODELS\\GENERIC.TXD;1",
			"\\MODELS\\GTA3.DIR;1",
			"\\TEXT\\ENGLISH.GXT;1",
			"\\TEXT\\FRENCH.GXT;1",
			"\\TEXT\\GERMAN.GXT;1",
			"\\TEXT\\ITALIAN.GXT;1",
			"\\TEXT\\SPANISH.GXT;1",
			"\\TXD\\LOADSC0.TXD;1",
			"\\TXD\\LOADSC1.TXD;1",
			"\\TXD\\LOADSC2.TXD;1",
			"\\TXD\\LOADSC3.TXD;1",
			"\\TXD\\LOADSC4.TXD;1",
			"\\TXD\\LOADSC5.TXD;1",
			"\\TXD\\LOADSC6.TXD;1",
			"\\TXD\\LOADSC7.TXD;1",
			"\\TXD\\LOADSC8.TXD;1",
			"\\TXD\\LOADSC9.TXD;1",
			"\\TXD\\LOADSC10.TXD;1",
			"\\TXD\\LOADSC11.TXD;1",
			"\\TXD\\LOADSC12.TXD;1",
			"\\TXD\\LOADSC13.TXD;1",
			"\\TXD\\LOADSC14.TXD;1",
			"\\TXD\\LOADSC15.TXD;1",
			"\\TXD\\LOADSC16.TXD;1",
			"\\TXD\\LOADSC17.TXD;1",
			"\\TXD\\LOADSC18.TXD;1",
			"\\TXD\\LOADSC19.TXD;1",
			"\\TXD\\LOADSC20.TXD;1",
			"\\TXD\\LOADSC21.TXD;1",
			"\\TXD\\LOADSC22.TXD;1",
			"\\TXD\\LOADSC23.TXD;1",
			"\\TXD\\LOADSC24.TXD;1",
			"\\TXD\\LOADSC25.TXD;1",
			"\\TXD\\NEWS.TXD;1",
			"\\MODELS\\COLL\\GENERIC.COL;1",
			"\\MODELS\\COLL\\INDUST.COL;1",
			"\\MODELS\\COLL\\COMMER.COL;1",
			"\\MODELS\\COLL\\SUBURB.COL;1",
			"\\MODELS\\COLL\\WEAPONS.COL;1",
			"\\MODELS\\COLL\\VEHICLES.COL;1",
			"\\MODELS\\COLL\\PEDS.COL;1",
			"\\MODELS\\GENERIC\\AIR_VLO.DFF;1",
			"\\MODELS\\GENERIC\\WEAPONS.DFF;1",
			"\\MODELS\\GENERIC\\WHEELS.DFF;1",
			"\\MODELS\\GENERIC\\LOPLYGUY.DFF;1",
			"\\MODELS\\GENERIC\\ARROW.DFF;1",
			"\\MODELS\\GENERIC\\ZONECYLB.DFF;1",
			"\\DATA\\MAPS\\COMNTOP.IPL;1",
			"\\DATA\\MAPS\\COMNBTM.IPL;1",
			"\\DATA\\MAPS\\COMSE.IPL;1",
			"\\DATA\\MAPS\\COMSW.IPL;1",
			"\\DATA\\MAPS\\CULL.IPL;1",
			"\\DATA\\MAPS\\INDUSTNE.IPL;1",
			"\\DATA\\MAPS\\INDUSTNW.IPL;1",
			"\\DATA\\MAPS\\INDUSTSE.IPL;1",
			"\\DATA\\MAPS\\INDUSTSW.IPL;1",
			"\\DATA\\MAPS\\SUBURBNE.IPL;1",
			"\\DATA\\MAPS\\SUBURBSW.IPL;1",
			"\\DATA\\MAPS\\OVERVIEW.IPL;1",
			"\\DATA\\MAPS\\PROPS.IPL;1",
			"\\DATA\\MAPS\\GTA3.IDE;1",
			"\\DATA\\PATHS\\FLIGHT.DAT;1",
			"\\DATA\\PATHS\\FLIGHT2.DAT;1",
			"\\DATA\\PATHS\\FLIGHT3.DAT;1",
			"\\DATA\\PATHS\\FLIGHT4.DAT;1",
			"\\DATA\\PATHS\\TRACKS.DAT;1",
			"\\DATA\\PATHS\\TRACKS2.DAT;1",
			"\\DATA\\PATHS\\CHASE0.DAT;1",
			"\\DATA\\PATHS\\CHASE1.DAT;1",
			"\\DATA\\PATHS\\CHASE2.DAT;1",
			"\\DATA\\PATHS\\CHASE3.DAT;1",
			"\\DATA\\PATHS\\CHASE4.DAT;1",
			"\\DATA\\PATHS\\CHASE5.DAT;1",
			"\\DATA\\PATHS\\CHASE6.DAT;1",
			"\\DATA\\PATHS\\CHASE7.DAT;1",
			"\\DATA\\PATHS\\CHASE10.DAT;1",
			"\\DATA\\PATHS\\CHASE11.DAT;1",
			"\\DATA\\PATHS\\CHASE14.DAT;1",
			"\\DATA\\PATHS\\CHASE16.DAT;1",
			"\\DATA\\PATHS\\CHASE18.DAT;1",
			"\\DATA\\PATHS\\CHASE19.DAT;1"
		};
		
		for ( int32 i = 0; i < ARRAY_SIZE(files); i++ )
			SkyRegisterFileOnCd([i]);
#endif
		
		CreateDebugFont();
		
#ifdef GTA_PS2
		AddIntcHandler(_TODOCONST(2), VBlankCounter, 0);
#endif
		
		CameraSize(Scene.camera, NULL, DEFAULT_VIEWWINDOW, DEFAULT_ASPECT_RATIO);
		
		CSprite2d::SetRecipNearClip();
		CTxdStore::Initialise();
#ifdef GTA_PS2
		gMainHeap.PushMemId(_TODOCONST(9));
#endif
		CFont::Initialise();
		CHud::Initialise();
#ifdef GTA_PS2
		gMainHeap.PopMemId();
#endif

		ValidateVersion();
		
#ifdef GTA_PS2
		sceCdCLOCK rtc;
		sceCdReadClock(&rtc);
		uint32 seed = rtc.minute + rtc.day;
		uint32 seed2 = (seed << 4)-seed;
		uint32 seed3 = (seed2 << 4)-seed2;
		srand ((seed3<<4)+rtc.second);
#else
		//TODO: mysrand();
#endif
		
		gameAlreadyInitialised = true;
	}
}

// Not used anyway. PS2 main() port
#ifdef _WIN32
int
main(int argc, char *argv[])
{
#ifdef __MWERKS__
	mwInit(); // metrowerks initialisation
#endif

	SystemInit();
	
#ifdef PS2
	int32 state = TheMemoryCard.CheckCardStateAtGameStartUp(_TODOCONST(0));
		
	if ( state == _TODOCONST(2) || state == _TODOCONST(1) && state != _TODOCONST(3) && state != _TODOCONST(0) )
	{
		GameInit();
		
		TheText.Unload();
		TheText.Load();
		
		CFont::Initialise();
		
		FrontEndMenuManager.DrawMemoryCardStartUpMenus();
	}
#endif
	
#ifdef GTA_PS2
	{
		if (gameAlreadyInitialised)
			RpSkySuspend();

		InitMPEGPlayer();

		PlayMPEG("cdrom0:\\MOVIES\\DMAPAL.PSS;1", false);

		if (CGame::frenchGame || CGame::germanGame)
			PlayMPEG("cdrom0:\\MOVIES\\INTROPAF.PSS;1", true);
		else
			PlayMPEG("cdrom0:\\MOVIES\\INTROPAL.PSS;1", true);

		ShutdownMPEGPlayer();

		if ( gameAlreadyInitialised )
			RpSkyResume();
	}
#else
	//TODO
#endif

	GameInit();

	if ( CGame::frenchGame || CGame::germanGame )
		LoadingScreen(NULL, version_name, "loadsc24");
	else
		LoadingScreen(NULL, version_name, "loadsc0");
	
	DMAudio.Initialise();
	
	TheGame();
	
	CGame::ShutDown();
	
	RwEngineStop();
	RwEngineClose();
	RwEngineTerm();
	
#ifdef __MWERKS__
	mwExit(); // metrowerks shutdown
#endif
	
	return 0;
}
#endif
