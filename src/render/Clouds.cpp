#include "common.h"

#include "Sprite.h"
#include "Sprite2d.h"
#include "General.h"
#include "Game.h"
#include "Coronas.h"
#include "Camera.h"
#include "TxdStore.h"
#include "Weather.h"
#include "Clock.h"
#include "Timer.h"
#include "Timecycle.h"
#include "Renderer.h"
#include "Clouds.h"

#define SMALLSTRIPHEIGHT 4.0f
#define HORIZSTRIPHEIGHT 48.0f

RwTexture *gpCloudTex[5];

float CClouds::CloudRotation;
uint32 CClouds::IndividualRotation;

float CClouds::ms_cameraRoll;
float CClouds::ms_horizonZ;
float CClouds::ms_HorizonTilt;
CRGBA CClouds::ms_colourTop;
CRGBA CClouds::ms_colourBottom;
CRGBA CClouds::ms_colourBkGrd;

void
CClouds::Init(void)
{
	CTxdStore::PushCurrentTxd();
	CTxdStore::SetCurrentTxd(CTxdStore::FindTxdSlot("particle"));
	gpCloudTex[0] = RwTextureRead("cloud1", nil);
	gpCloudTex[1] = RwTextureRead("cloud2", nil);
	gpCloudTex[2] = RwTextureRead("cloud3", nil);
	gpCloudTex[3] = RwTextureRead("cloudhilit", nil);
	gpCloudTex[4] = RwTextureRead("cloudmasked", nil);
	CTxdStore::PopCurrentTxd();
	CloudRotation = 0.0f;
}

void
CClouds::Shutdown(void)
{
	RwTextureDestroy(gpCloudTex[0]);
	gpCloudTex[0] = nil;
	RwTextureDestroy(gpCloudTex[1]);
	gpCloudTex[1] = nil;
	RwTextureDestroy(gpCloudTex[2]);
	gpCloudTex[2] = nil;
	RwTextureDestroy(gpCloudTex[3]);
	gpCloudTex[3] = nil;
	RwTextureDestroy(gpCloudTex[4]);
	gpCloudTex[4] = nil;
}

void
CClouds::Update(void)
{
	float s = Sin(TheCamera.Orientation - 0.85f);
#ifdef FIX_BUGS
	CloudRotation += CWeather::Wind*s*0.001f*CTimer::GetTimeStepFix();
	IndividualRotation += (CWeather::Wind*CTimer::GetTimeStep()*0.5f + 0.3f*CTimer::GetTimeStepFix()) * 60.0f;
#else
	CloudRotation += CWeather::Wind*s*0.001f;
	IndividualRotation += (CWeather::Wind*CTimer::GetTimeStep()*0.5f + 0.3f) * 60.0f;
#endif
}

void
CClouds::Render(void)
{
	int i;
	float szx, szy;
	RwV3d screenpos;
	RwV3d worldpos;

	if(!CGame::CanSeeOutSideFromCurrArea())
		return;

	CCoronas::SunBlockedByClouds = false;

	RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)FALSE);
	RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*)FALSE);
	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)TRUE);
	RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDONE);
	RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDONE);
	CSprite::InitSpriteBuffer();

	float minute = CClock::GetHours()*60 + CClock::GetMinutes() + CClock::GetSeconds()/60.0f;
	RwV3d campos = *(RwV3d*)&TheCamera.GetPosition();

	float coverage = Max(CWeather::Foggyness, CWeather::Foggyness);

	// Moon
	float moonfadeout = Abs(minute - 180.0f);	// fully visible at 3AM
	if((int)moonfadeout < 180){			// fade in/out 3 hours
		int brightness = (1.0f - coverage) * (180 - (int)moonfadeout);
		RwV3d pos = { 0.0f, -100.0f, 15.0f };
		RwV3dAdd(&worldpos, &campos, &pos);
		if(CSprite::CalcScreenCoors(worldpos, &screenpos, &szx, &szy, false)){
			RwRenderStateSet(rwRENDERSTATETEXTURERASTER, RwTextureGetRaster(gpCoronaTexture[2]));
			szx *= CCoronas::MoonSize*2.0f + 4.0f;
			szy *= CCoronas::MoonSize*2.0f + 4.0f;
			CSprite::RenderOneXLUSprite(screenpos.x, screenpos.y, screenpos.z,
				szx, szy, brightness, brightness, brightness, 255, 1.0f/screenpos.z, 255);
		}
	}

	// The R* logo
	int starintens = 0;
	if(CClock::GetHours() < 22 && CClock::GetHours() > 5)
		starintens = 0;
	else if(CClock::GetHours() > 22 || CClock::GetHours() < 5)
		starintens = 255;
	else if(CClock::GetHours() == 22)
		starintens = 255 * CClock::GetMinutes()/60.0f;
	else if(CClock::GetHours() == 5)
		starintens = 255 * (60 - CClock::GetMinutes())/60.0f;
	if(starintens != 0){
		// R
		static float StarCoorsX[9] = { 0.0f, 0.05f, 0.13f, 0.4f, 0.7f, 0.6f, 0.27f, 0.55f, 0.75f };
		static float StarCoorsY[9] = { 0.0f, 0.45f, 0.9f, 1.0f, 0.85f, 0.52f, 0.48f, 0.35f, 0.2f };
		static float StarSizes[9] = { 1.0f, 1.4f, 0.9f, 1.0f, 0.6f, 1.5f, 1.3f, 1.0f, 0.8f };
		int brightness = (1.0f - coverage) * starintens;
		RwRenderStateSet(rwRENDERSTATETEXTURERASTER, RwTextureGetRaster(gpCoronaTexture[0]));
		for(i = 0; i < 11; i++){
			RwV3d pos = { 100.0f, 0.0f, 10.0f };
			if(i >= 9) pos.x = -pos.x;
			RwV3dAdd(&worldpos, &campos, &pos);
			worldpos.y -= 90.0f*StarCoorsX[i%9];
			worldpos.z += 80.0f*StarCoorsY[i%9];
			if(CSprite::CalcScreenCoors(worldpos, &screenpos, &szx, &szy, false)){
				float sz = 0.8f*StarSizes[i%9];
				CSprite::RenderBufferedOneXLUSprite(screenpos.x, screenpos.y, screenpos.z,
					szx*sz, szy*sz, brightness, brightness, brightness, 255, 1.0f/screenpos.z, 255);
			}
		}
		CSprite::FlushSpriteBuffer();

		// *
		RwRenderStateSet(rwRENDERSTATETEXTURERASTER, RwTextureGetRaster(gpCoronaTexture[0]));
		RwV3d pos = { 100.0f, 0.0f, 10.0f };
		RwV3dAdd(&worldpos, &campos, &pos);
		worldpos.y -= 90.0f;
		if(CSprite::CalcScreenCoors(worldpos, &screenpos, &szx, &szy, false)){
			brightness *= (CGeneral::GetRandomNumber()&127) / 640.0f + 0.5f;
			CSprite::RenderOneXLUSprite(screenpos.x, screenpos.y, screenpos.z,
				szx*5.0f, szy*5.0f, brightness, brightness, brightness, 255, 1.0f/screenpos.z, 255);
		}
	}

	// Low clouds
	static float LowCloudsX[12] = { 1.0f, 0.7f, 0.0f, -0.7f, -1.0f, -0.7f,
		0.0f, 0.7f, 0.8f, -0.8f, 0.4f, -0.4f };
	static float LowCloudsY[12] = { 0.0f, -0.7f, -1.0f, -0.7f, 0.0f, 0.7f,
		1.0f, 0.7f, 0.4f, 0.4f, -0.8f, -0.8f };
	static float LowCloudsZ[12] = { 0.0f, 1.0f, 0.5f, 0.0f, 1.0f, 0.3f,
		0.9f, 0.4f, 1.3f, 1.4f, 1.2f, 1.7f };
	float lowcloudintensity = 1.0f - Max(coverage, CWeather::ExtraSunnyness);
	int r = CTimeCycle::GetLowCloudsRed() * lowcloudintensity;
	int g = CTimeCycle::GetLowCloudsGreen() * lowcloudintensity;
	int b = CTimeCycle::GetLowCloudsBlue() * lowcloudintensity;
	for(int cloudtype = 0; cloudtype < 3; cloudtype++){
		for(i = cloudtype; i < 12; i += 3){
			RwRenderStateSet(rwRENDERSTATETEXTURERASTER, RwTextureGetRaster(gpCloudTex[cloudtype]));
			RwV3d pos = { 800.0f*LowCloudsX[i], 800.0f*LowCloudsY[i], 60.0f*LowCloudsZ[i] };
			worldpos.x = campos.x + pos.x;
			worldpos.y = campos.y + pos.y;
			worldpos.z = 40.0f + pos.z;
			if(CSprite::CalcScreenCoors(worldpos, &screenpos, &szx, &szy, false))
				CSprite::RenderBufferedOneXLUSprite_Rotate_Dimension(screenpos.x, screenpos.y, screenpos.z,
					szx*320.0f, szy*40.0f, r, g, b, 255, 1.0f/screenpos.z, ms_cameraRoll, 255);
		}
		CSprite::FlushSpriteBuffer();
	}

	// Fluffy clouds
	float rot_sin = Sin(CloudRotation);
	float rot_cos = Cos(CloudRotation);
	int fluffyalpha = 160 * (1.0f - Max(CWeather::Foggyness, CWeather::ExtraSunnyness));
	if(fluffyalpha != 0){
		static float CoorsOffsetX[37] = {
			0.0f, 60.0f, 72.0f, 48.0f, 21.0f, 12.0f,
			9.0f, -3.0f, -8.4f, -18.0f, -15.0f, -36.0f,
			-40.0f, -48.0f, -60.0f, -24.0f, 100.0f, 100.0f,
			100.0f, 100.0f, 100.0f, 100.0f, 100.0f, 100.0f,
			100.0f, 100.0f, -30.0f, -20.0f, 10.0f, 30.0f,
			0.0f, -100.0f, -100.0f, -100.0f, -100.0f, -100.0f, -100.0f
		};
		static float CoorsOffsetY[37] = {
			100.0f, 100.0f, 100.0f, 100.0f, 100.0f, 100.0f,
			100.0f, 100.0f, 100.0f, 100.0f, 100.0f, 100.0f,
			100.0f, 100.0f, 100.0f, 100.0f, -30.0f, 10.0f,
			-25.0f, -5.0f, 28.0f, -10.0f, 10.0f, 0.0f,
			15.0f, 40.0f, -100.0f, -100.0f, -100.0f, -100.0f,
			-100.0f, -40.0f, -20.0f, 0.0f, 10.0f, 30.0f, 35.0f
		};
		static float CoorsOffsetZ[37] = {
			2.0f, 1.0f, 0.0f, 0.3f, 0.7f, 1.4f,
			1.7f, 0.24f, 0.7f, 1.3f, 1.6f, 1.0f,
			1.2f, 0.3f, 0.7f, 1.4f, 0.0f, 0.1f,
			0.5f, 0.4f, 0.55f, 0.75f, 1.0f, 1.4f,
			1.7f, 2.0f, 2.0f, 2.3f, 1.9f, 2.4f,
			2.0f, 2.0f, 1.5f, 1.2f, 1.7f, 1.5f, 2.1f
		};
		static bool bCloudOnScreen[37];
		float sundist, hilight;

		RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDSRCALPHA);
		RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDINVSRCALPHA);
		RwRenderStateSet(rwRENDERSTATETEXTURERASTER, RwTextureGetRaster(gpCloudTex[4]));
		for(i = 0; i < 37; i++){
			RwV3d pos = { 2.0f*CoorsOffsetX[i], 2.0f*CoorsOffsetY[i], 40.0f*CoorsOffsetZ[i] + 40.0f };
			worldpos.x = pos.x*rot_cos + pos.y*rot_sin + campos.x;
			worldpos.y = pos.x*rot_sin - pos.y*rot_cos + campos.y;
			worldpos.z = pos.z;

			if(CSprite::CalcScreenCoors(worldpos, &screenpos, &szx, &szy, false)){
				sundist = Sqrt(sq(screenpos.x-CCoronas::SunScreenX) + sq(screenpos.y-CCoronas::SunScreenY));
				int tr = CTimeCycle::GetFluffyCloudsTopRed();
				int tg = CTimeCycle::GetFluffyCloudsTopGreen();
				int tb = CTimeCycle::GetFluffyCloudsTopBlue();
				int br = CTimeCycle::GetFluffyCloudsBottomRed();
				int bg = CTimeCycle::GetFluffyCloudsBottomGreen();
				int bb = CTimeCycle::GetFluffyCloudsBottomBlue();
				int distLimit = (3*SCREEN_WIDTH)/4;
				if(sundist < distLimit){
					hilight = (1.0f - coverage) * (1.0f - sundist/(float)distLimit);
					tr = tr*(1.0f-hilight) + 255*hilight;
					tg = tg*(1.0f-hilight) + 190*hilight;
					tb = tb*(1.0f-hilight) + 190*hilight;
					br = br*(1.0f-hilight) + 255*hilight;
					bg = bg*(1.0f-hilight) + 190*hilight;
					bb = bb*(1.0f-hilight) + 190*hilight;
					if(sundist < SCREEN_WIDTH/10)
						CCoronas::SunBlockedByClouds = true;
				}else
					hilight = 0.0f;
				CSprite::RenderBufferedOneXLUSprite_Rotate_2Colours(screenpos.x, screenpos.y, screenpos.z,
					szx*55.0f, szy*55.0f,
					tr, tg, tb, br, bg, bb, 0.0f, -1.0f,
					1.0f/screenpos.z,
					(uint16)IndividualRotation/65336.0f * 6.28f + ms_cameraRoll,
					fluffyalpha);
				bCloudOnScreen[i] = true;
			}else
				bCloudOnScreen[i] = false;
		}
		CSprite::FlushSpriteBuffer();

		// Highlights
		RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDONE);
		RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDONE);
		RwRenderStateSet(rwRENDERSTATETEXTURERASTER, RwTextureGetRaster(gpCloudTex[3]));

		for(i = 0; i < 37; i++){
			RwV3d pos = { 2.0f*CoorsOffsetX[i], 2.0f*CoorsOffsetY[i], 40.0f*CoorsOffsetZ[i] + 40.0f };
			worldpos.x = campos.x*rot_cos + campos.y*rot_sin + pos.x;
			worldpos.y = campos.x*rot_sin + campos.y*rot_cos + pos.y;
			worldpos.z = pos.z;
			if(bCloudOnScreen[i] && CSprite::CalcScreenCoors(worldpos, &screenpos, &szx, &szy, false)){
				if(sundist < SCREEN_WIDTH/3){
					CSprite::RenderBufferedOneXLUSprite_Rotate_Aspect(screenpos.x, screenpos.y, screenpos.z,
						szx*30.0f, szy*30.0f,
						200*hilight, 0, 0, 255, 1.0f/screenpos.z,
						1.7f - CGeneral::GetATanOfXY(screenpos.x-CCoronas::SunScreenX, screenpos.y-CCoronas::SunScreenY) + CClouds::ms_cameraRoll, 255);
				}
			}
		}
		CSprite::FlushSpriteBuffer();
	}

	// Rainbow
	if(CWeather::Rainbow != 0.0f){
		static uint8 BowRed[6] = { 30, 30, 30, 10, 0, 15 };
		static uint8 BowGreen[6] = { 0, 15, 30, 30, 0, 0 };
		static uint8 BowBlue[6] = { 0, 0, 0, 10, 30, 30 };
		RwRenderStateSet(rwRENDERSTATETEXTURERASTER, RwTextureGetRaster(gpCoronaTexture[0]));
		for(i = 0; i < 6; i++){
			RwV3d pos = { i*1.5f, 100.0f, 5.0f };
			RwV3dAdd(&worldpos, &campos, &pos);
			if(CSprite::CalcScreenCoors(worldpos, &screenpos, &szx, &szy, false))
				CSprite::RenderBufferedOneXLUSprite(screenpos.x, screenpos.y, screenpos.z,
					2.0f*szx, 50.0*szy,
					BowRed[i]*CWeather::Rainbow, BowGreen[i]*CWeather::Rainbow, BowBlue[i]*CWeather::Rainbow,
					255, 1.0f/screenpos.z, 255);

		}
		CSprite::FlushSpriteBuffer();
	}

	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)FALSE);
	RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)TRUE);
	RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*)TRUE);
	RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDSRCALPHA);
	RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDINVSRCALPHA);
}

bool
UseDarkBackground(void)
{
	return TheCamera.GetForward().z < -0.9f || gbShowCollisionPolys;
}

void
CClouds::RenderBackground(int16 topred, int16 topgreen, int16 topblue,
	int16 botred, int16 botgreen, int16 botblue, int16 alpha)
{
	CVector right = CrossProduct(TheCamera.GetUp(), TheCamera.GetForward());
	right.Normalise();
	float c = right.Magnitude2D();
	if(c > 1.0f)
		c = 1.0f;
	ms_cameraRoll = Acos(c);
	if(right.z < 0.0f)
		ms_cameraRoll = -ms_cameraRoll;

	ms_HorizonTilt = SCREEN_WIDTH/2.0f * Tan(ms_cameraRoll);

	if(UseDarkBackground()){
		ms_colourTop.r = 50;
		ms_colourTop.g = 50;
		ms_colourTop.b = 50;
		ms_colourTop.a = 255;
		if(gbShowCollisionPolys){
			if(CTimer::GetFrameCounter() & 1){
				ms_colourTop.r = 0;
				ms_colourTop.g = 0;
				ms_colourTop.b = 0;
			}else{
				ms_colourTop.r = 255;
				ms_colourTop.g = 255;
				ms_colourTop.b = 255;
			}
		}
		ms_colourBottom = ms_colourTop;
		CRect r(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
		CSprite2d::DrawRect(r, ms_colourBottom, ms_colourBottom, ms_colourTop, ms_colourTop);
	}else{
		ms_horizonZ = CSprite::CalcHorizonCoors();

		int fogr = (topred + 2 * botred) / 3;
		int fogg = (topgreen + 2 * botgreen) / 3;
		int fogb = (topblue + 2 * botblue) / 3;

		// Draw top/bottom gradient
		float gradheight = SCREEN_HEIGHT/2.0f;

		ms_colourTop.r = topred;
		ms_colourTop.g = topgreen;
		ms_colourTop.b = topblue;
		ms_colourTop.a = alpha;
		ms_colourBottom.r = botred;
		ms_colourBottom.g = botgreen;
		ms_colourBottom.b = botblue;
		ms_colourBottom.a = alpha;

		float botright = ms_horizonZ - ms_HorizonTilt;
		float botleft = ms_horizonZ + ms_HorizonTilt;
		float topright = botright - gradheight;
		float topleft = botleft - gradheight;

		CSprite2d::DrawAnyRect(0.0f, topleft,  SCREEN_WIDTH, topright,  0.0f, botleft,  SCREEN_WIDTH, botright,
			 ms_colourTop, ms_colourTop, ms_colourBottom, ms_colourBottom);

		// draw the small stripe (whatever it's supposed to be)
		ms_colourTop.r = fogr;
		ms_colourTop.g = fogg;
		ms_colourTop.b = fogb;
		ms_colourTop.a = alpha;
		topright = ms_horizonZ - ms_HorizonTilt;
		topleft = ms_horizonZ + ms_HorizonTilt;
		botright = topright + SMALLSTRIPHEIGHT;
		botleft = topleft + SMALLSTRIPHEIGHT;
		CSprite2d::DrawAnyRect(0.0f, topleft,  SCREEN_WIDTH, topright,  0.0f, botleft,  SCREEN_WIDTH, botright,
			ms_colourTop, ms_colourTop, ms_colourTop, ms_colourTop);

		// Only top
		if(ms_horizonZ + ms_HorizonTilt - gradheight > 0.0f ||
		   ms_horizonZ - ms_HorizonTilt - gradheight > 0.0f){
			ms_colourTop.r = topred;
			ms_colourTop.g = topgreen;
			ms_colourTop.b = topblue;
			ms_colourTop.a = alpha;

			if(ms_horizonZ - Abs(ms_HorizonTilt) - gradheight > SCREEN_HEIGHT){
				// only top is visible
				topleft = 0.0f;
				topright = 0.0f;
				botleft = SCREEN_HEIGHT;
				botright = SCREEN_HEIGHT;
			}else{
				botright = ms_horizonZ - ms_HorizonTilt - gradheight;
				botleft = ms_horizonZ + ms_HorizonTilt - gradheight;
				topright = Min(ms_horizonZ - ms_HorizonTilt - 2*SCREEN_HEIGHT, 0.0f);
				topleft = Min(ms_horizonZ + ms_HorizonTilt - 2*SCREEN_HEIGHT, 0.0f);
			}

			CSprite2d::DrawAnyRect(0.0f, topleft,  SCREEN_WIDTH, topright,  0.0f, botleft,  SCREEN_WIDTH, botright,
				ms_colourTop, ms_colourTop, ms_colourTop, ms_colourTop);
		}

		// Set both to fog colour for RenderHorizon
		ms_colourTop.r = fogr;
		ms_colourTop.g = fogg;
		ms_colourTop.b = fogb;
		ms_colourBottom.r = fogr;
		ms_colourBottom.g = fogg;
		ms_colourBottom.b = fogb;
	}
}

void
CClouds::RenderHorizon(void)
{
	if(UseDarkBackground())
		return;

	ms_colourBottom.a = 230;
	ms_colourTop.a = 80;

	float topright = ms_horizonZ - ms_HorizonTilt;
	float topleft = ms_horizonZ + ms_HorizonTilt;
	float botright = topright + SMALLSTRIPHEIGHT;
	float botleft = topleft + SMALLSTRIPHEIGHT;

	CSprite2d::DrawAnyRect(0.0f, topleft,  SCREEN_WIDTH, topright,  0.0f, botleft,  SCREEN_WIDTH, botright,
		ms_colourTop, ms_colourTop, ms_colourBottom, ms_colourBottom);


	ms_colourBkGrd.r = 128.0f*CTimeCycle::GetAmbientRed();
	ms_colourBkGrd.g = 128.0f*CTimeCycle::GetAmbientGreen();
	ms_colourBkGrd.b = 128.0f*CTimeCycle::GetAmbientBlue();
	ms_colourBkGrd.a = 255;

	float horzstrip = SCREEN_STRETCH_Y(HORIZSTRIPHEIGHT);
	topright = botright;
	topleft = botleft;
	botright = topright + horzstrip;
	botleft = topleft + horzstrip;

	CSprite2d::DrawAnyRect(0.0f, topleft,  SCREEN_WIDTH, topright,  0.0f, botleft,  SCREEN_WIDTH, botright,
		ms_colourBottom, ms_colourBottom, ms_colourBkGrd, ms_colourBkGrd);


	topright = botright;
	topleft = botleft;
	botright = Max(topright, SCREEN_HEIGHT);
	botleft = Max(topleft, SCREEN_HEIGHT);

	CSprite2d::DrawAnyRect(0.0f, topleft,  SCREEN_WIDTH, topright,  0.0f, botleft,  SCREEN_WIDTH, botright,
		ms_colourBkGrd, ms_colourBkGrd, ms_colourBkGrd, ms_colourBkGrd);
}
