#include "common.h"

#include "main.h"
#include "Sprite.h"
#include "Sprite2d.h"
#include "General.h"
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
CRGBA CClouds::ms_colourTop;
CRGBA CClouds::ms_colourBottom;

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
#if GTA_VERSION >= GTA3_PC_11
	gpCloudTex[0] = nil;
#endif
	RwTextureDestroy(gpCloudTex[1]);
#if GTA_VERSION >= GTA3_PC_11
	gpCloudTex[1] = nil;
#endif
	RwTextureDestroy(gpCloudTex[2]);
#if GTA_VERSION >= GTA3_PC_11
	gpCloudTex[2] = nil;
#endif
	RwTextureDestroy(gpCloudTex[3]);
#if GTA_VERSION >= GTA3_PC_11
	gpCloudTex[3] = nil;
#endif
	RwTextureDestroy(gpCloudTex[4]);
#if GTA_VERSION >= GTA3_PC_11
	gpCloudTex[4] = nil;
#endif
}

void
CClouds::Update(void)
{
	float s = Sin(TheCamera.Orientation - 0.85f);
#ifdef FIX_BUGS
	CloudRotation += CWeather::Wind*s*0.0025f*CTimer::GetTimeStepFix();
	IndividualRotation += (CWeather::Wind*CTimer::GetTimeStep() + 0.3f*CTimer::GetTimeStepFix()) * 60.0f;
#else
	CloudRotation += CWeather::Wind*s*0.0025f;
	IndividualRotation += (CWeather::Wind*CTimer::GetTimeStep() + 0.3f) * 60.0f;
#endif
}

float StarCoorsX[9] = { 0.0f, 0.05f, 0.12f, 0.5f, 0.8f, 0.6f, 0.27f, 0.55f, 0.75f };
float StarCoorsY[9] = { 0.0f, 0.45f, 0.9f, 1.0f, 0.85f, 0.52f, 0.48f, 0.35f, 0.2f };
float StarSizes[9] = { 1.0f, 1.4f, 0.9f, 1.0f, 0.6f, 1.5f, 1.3f, 1.0f, 0.8f };

float LowCloudsX[12] = { 1.0f, 0.7f, 0.0f, -0.7f, -1.0f, -0.7f, 0.0f, 0.7f, 0.8f, -0.8f, 0.4f, -0.4f };
float LowCloudsY[12] = { 0.0f, -0.7f, -1.0f, -0.7f, 0.0f, 0.7f, 1.0f, 0.7f, 0.4f, 0.4f, -0.8f, -0.8f };
float LowCloudsZ[12] = { 0.0f, 1.0f, 0.5f, 0.0f, 1.0f, 0.3f, 0.9f, 0.4f, 1.3f, 1.4f, 1.2f, 1.7f };

float CoorsOffsetX[37] = {
	0.0f, 60.0f, 72.0f, 48.0f, 21.0f, 12.0f,
	9.0f, -3.0f, -8.4f, -18.0f, -15.0f, -36.0f,
	-40.0f, -48.0f, -60.0f, -24.0f, 100.0f, 100.0f,
	100.0f, 100.0f, 100.0f, 100.0f, 100.0f, 100.0f,
	100.0f, 100.0f, -30.0f, -20.0f, 10.0f, 30.0f,
	0.0f, -100.0f, -100.0f, -100.0f, -100.0f, -100.0f, -100.0f
};
float CoorsOffsetY[37] = {
	100.0f, 100.0f, 100.0f, 100.0f, 100.0f, 100.0f,
	100.0f, 100.0f, 100.0f, 100.0f, 100.0f, 100.0f,
	100.0f, 100.0f, 100.0f, 100.0f, -30.0f, 10.0f,
	-25.0f, -5.0f, 28.0f, -10.0f, 10.0f, 0.0f,
	15.0f, 40.0f, -100.0f, -100.0f, -100.0f, -100.0f,
	-100.0f, -40.0f, -20.0f, 0.0f, 10.0f, 30.0f, 35.0f
};
float CoorsOffsetZ[37] = {
	2.0f, 1.0f, 0.0f, 0.3f, 0.7f, 1.4f,
	1.7f, 0.24f, 0.7f, 1.3f, 1.6f, 1.0f,
	1.2f, 0.3f, 0.7f, 1.4f, 0.0f, 0.1f,
	0.5f, 0.4f, 0.55f, 0.75f, 1.0f, 1.4f,
	1.7f, 2.0f, 2.0f, 2.3f, 1.9f, 2.4f,
	2.0f, 2.0f, 1.5f, 1.2f, 1.7f, 1.5f, 2.1f
};

uint8 BowRed[6] = { 30, 30, 30, 10, 0, 15 };
uint8 BowGreen[6] = { 0, 15, 30, 30, 0, 0 };
uint8 BowBlue[6] = { 0, 0, 0, 10, 30, 30 };

void
CClouds::Render(void)
{
	int i;
	float szx, szy;
	RwV3d screenpos;
	RwV3d worldpos;

	PUSH_RENDERGROUP("CClouds::Render");

	CCoronas::SunBlockedByClouds = false;

	RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)FALSE);
	RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*)FALSE);
	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)TRUE);
	RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDONE);
	RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDONE);
	CSprite::InitSpriteBuffer();

	int minute = CClock::GetHours()*60 + CClock::GetMinutes();
	RwV3d campos = TheCamera.GetPosition();

	// Moon
	int moonfadeout = Abs(minute - 180);	// fully visible at 3AM
	if(moonfadeout < 180){			// fade in/out 3 hours
		float coverage = Max(CWeather::Foggyness, CWeather::CloudCoverage);
		int brightness = (1.0f - coverage) * (180 - moonfadeout);
		RwV3d pos = { 0.0f, -100.0f, 15.0f };
		RwV3dAdd(&worldpos, &campos, &pos);
		if(CSprite::CalcScreenCoors(worldpos, &screenpos, &szx, &szy, false)){
			RwRenderStateSet(rwRENDERSTATETEXTURERASTER, RwTextureGetRaster(gpCoronaTexture[2]));
			if(CCoronas::bSmallMoon){
				szx *= 4.0f;
				szy *= 4.0f;
			}else{
				szx *= 10.0f;
				szy *= 10.0f;
			}
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
		float coverage = Max(CWeather::Foggyness, CWeather::CloudCoverage);
		int brightness = (1.0f - coverage) * starintens;

		// R
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
	float lowcloudintensity = 1.0f - Max(CWeather::Foggyness, CWeather::CloudCoverage);
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
	int fluffyalpha = 160 * (1.0f - CWeather::Foggyness);
	if(fluffyalpha != 0){
		static bool bCloudOnScreen[37];
		float hilight;

		RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDSRCALPHA);
		RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDINVSRCALPHA);
		RwRenderStateSet(rwRENDERSTATETEXTURERASTER, RwTextureGetRaster(gpCloudTex[4]));
		for(i = 0; i < 37; i++){
			RwV3d pos = { 2.0f*CoorsOffsetX[i], 2.0f*CoorsOffsetY[i], 40.0f*CoorsOffsetZ[i] + 40.0f };
			worldpos.x = pos.x*rot_cos + pos.y*rot_sin + campos.x;
			worldpos.y = pos.x*rot_sin - pos.y*rot_cos + campos.y;
			worldpos.z = pos.z;

			if(CSprite::CalcScreenCoors(worldpos, &screenpos, &szx, &szy, false)){
				float sundist = Sqrt(sq(screenpos.x-CCoronas::SunScreenX) + sq(screenpos.y-CCoronas::SunScreenY));
				int tr = CTimeCycle::GetFluffyCloudsTopRed();
				int tg = CTimeCycle::GetFluffyCloudsTopGreen();
				int tb = CTimeCycle::GetFluffyCloudsTopBlue();
				int br = CTimeCycle::GetFluffyCloudsBottomRed();
				int bg = CTimeCycle::GetFluffyCloudsBottomGreen();
				int bb = CTimeCycle::GetFluffyCloudsBottomBlue();
				if(sundist < SCREEN_WIDTH/2){
					hilight = (1.0f - Max(CWeather::Foggyness, CWeather::CloudCoverage)) * (1.0f - sundist/(SCREEN_WIDTH/2));
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
			worldpos.x = pos.x*rot_cos + pos.y*rot_sin + campos.x;
			worldpos.y = pos.x*rot_sin - pos.y*rot_cos + campos.y;
			worldpos.z = pos.z;
			if(bCloudOnScreen[i] && CSprite::CalcScreenCoors(worldpos, &screenpos, &szx, &szy, false)){
				// BUG: this is stupid....would have to do this for each cloud individually
				if(hilight > 0.0f){
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

	POP_RENDERGROUP();
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
	PUSH_RENDERGROUP("CClouds::RenderBackground");

	CVector left = TheCamera.GetRight();
	float c = left.Magnitude2D();
	if(c > 1.0f)
		c = 1.0f;
	ms_cameraRoll = Acos(c);
	if(left.z < 0.0f)
		ms_cameraRoll = -ms_cameraRoll;

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

		// Draw top/bottom gradient
		float gradheight = SCREEN_HEIGHT/2.0f;
		float topedge = ms_horizonZ - gradheight;
		float botpos, toppos;
		if(ms_horizonZ > 0.0f && topedge < SCREEN_HEIGHT){
			ms_colourTop.r = topred;
			ms_colourTop.g = topgreen;
			ms_colourTop.b = topblue;
			ms_colourTop.a = alpha;
			ms_colourBottom.r = botred;
			ms_colourBottom.g = botgreen;
			ms_colourBottom.b = botblue;
			ms_colourBottom.a = alpha;

			if(ms_horizonZ < SCREEN_HEIGHT)
				botpos = ms_horizonZ;
			else{
				float f = (ms_horizonZ - SCREEN_HEIGHT)/gradheight;
				ms_colourBottom.r = topred*f + (1.0f-f)*botred;
				ms_colourBottom.g = topgreen*f + (1.0f-f)*botgreen;
				ms_colourBottom.b = topblue*f + (1.0f-f)*botblue;
				botpos = SCREEN_HEIGHT;
			}
			if(topedge >= 0.0f)
				toppos = topedge;
			else{
				float f = (0.0f - topedge)/gradheight;
				ms_colourTop.r = botred*f + (1.0f-f)*topred;
				ms_colourTop.g = botgreen*f + (1.0f-f)*topgreen;
				ms_colourTop.b = botblue*f + (1.0f-f)*topblue;
				toppos = 0.0f;
			}
			CSprite2d::DrawRect(CRect(0, toppos, SCREEN_WIDTH, botpos),
				ms_colourBottom, ms_colourBottom, ms_colourTop, ms_colourTop);
		}

		// draw the small stripe (whatever it's supposed to be)
		if(ms_horizonZ > -SMALLSTRIPHEIGHT && ms_horizonZ < SCREEN_HEIGHT){
			// Same colour as fog
			ms_colourTop.r = (topred + 2 * botred) / 3;
			ms_colourTop.g = (topgreen + 2 * botgreen) / 3;
			ms_colourTop.b = (topblue + 2 * botblue) / 3;
			CSprite2d::DrawRect(CRect(0, ms_horizonZ, SCREEN_WIDTH, ms_horizonZ+SMALLSTRIPHEIGHT),
				ms_colourTop, ms_colourTop, ms_colourTop, ms_colourTop);
		}

		// Only top
		if(topedge > 0.0f){
			ms_colourTop.r = topred;
			ms_colourTop.g = topgreen;
			ms_colourTop.b = topblue;
			ms_colourTop.a = alpha;
			ms_colourBottom.r = topred;
			ms_colourBottom.g = topgreen;
			ms_colourBottom.b = topblue;
			ms_colourBottom.a = alpha;

			botpos = Min(SCREEN_HEIGHT, topedge);
			CSprite2d::DrawRect(CRect(0, 0, SCREEN_WIDTH, botpos),
				ms_colourBottom, ms_colourBottom, ms_colourTop, ms_colourTop);
		}

		// Set both to fog colour for RenderHorizon
		ms_colourTop.r = (topred + 2 * botred) / 3;
		ms_colourTop.g = (topgreen + 2 * botgreen) / 3;
		ms_colourTop.b = (topblue + 2 * botblue) / 3;
		ms_colourBottom.r = (topred + 2 * botred) / 3;
		ms_colourBottom.g = (topgreen + 2 * botgreen) / 3;
		ms_colourBottom.b = (topblue + 2 * botblue) / 3;
	}

	POP_RENDERGROUP();
}

void
CClouds::RenderHorizon(void)
{
	if(UseDarkBackground())
		return;

	ms_colourBottom.a = 230;
	ms_colourTop.a = 80;

	if(ms_horizonZ > SCREEN_HEIGHT)
		return;

	PUSH_RENDERGROUP("CClouds::RenderHorizon");

	float z1 = Min(ms_horizonZ + SMALLSTRIPHEIGHT, SCREEN_HEIGHT);
	CSprite2d::DrawRectXLU(CRect(0, ms_horizonZ, SCREEN_WIDTH, z1),
		ms_colourBottom, ms_colourBottom, ms_colourTop, ms_colourTop);

	// This is just weird
	float a = SCREEN_HEIGHT/400.0f * HORIZSTRIPHEIGHT +
		SCREEN_HEIGHT/300.0f * Max(TheCamera.GetPosition().z, 0.0f);
	float b = TheCamera.GetUp().z < 0.0f ?
		SCREEN_HEIGHT :
		SCREEN_HEIGHT * Abs(TheCamera.GetRight().z);
	float z2 = z1 + (a + b)*TheCamera.LODDistMultiplier;
	z2 = Min(z2, SCREEN_HEIGHT);
	CSprite2d::DrawRect(CRect(0, z1, SCREEN_WIDTH, z2),
		ms_colourBottom, ms_colourBottom, ms_colourTop, ms_colourTop);

	POP_RENDERGROUP();
}
