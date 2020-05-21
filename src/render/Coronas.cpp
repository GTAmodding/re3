#include "common.h"

#include "main.h"
#include "General.h"
#include "TxdStore.h"
#include "Camera.h"
#include "Sprite.h"
#include "Timer.h"
#include "World.h"
#include "Weather.h"
#include "Collision.h"
#include "Timecycle.h"
#include "Coronas.h"

struct FlareDef
{
	float position;
	float size;
	int16 red;
	int16 green;
	int16 blue;
	int16 alpha;
	int16 texture;
};

FlareDef SunFlareDef[] = {
	{ -0.5f, 15.0f, 50, 50, 0, 200, 1 },
	{ -1.0f, 10.0f, 50, 20, 0, 200, 2 },
	{ -1.5f, 15.0f, 50, 0, 0, 200, 3 },
	{ -2.5f, 25.0f, 50, 0, 0, 200, 1 },
	{ 0.5f, 12.5f, 40, 40, 25, 200, 1 },
	{ 0.05f, 20.0f, 30, 22, 9, 200, 2 },
	{ 1.3f, 7.5f, 50, 30, 9, 200, 3 },
	{ 0.0f, 0.0f, 255, 255, 255, 255, 0 }
};

FlareDef HeadLightsFlareDef[] = {
	{ -0.5f, 15.5, 70, 70, 70, 200, 1 },
	{ -1.0f, 10.0, 70, 70, 70, 200, 2 },
	{ -1.5f, 5.5f, 50, 50, 50, 200, 3 },
	{ 0.5f, 12.0f, 50, 50, 50, 200, 1 },
	{ 0.05f, 20.0f, 40, 40, 40, 200, 2 },
	{ 1.3f, 8.0f, 60, 60, 60, 200, 3 },
	{ -2.0f, 12.0f, 50, 50, 50, 200, 1 },
	{ -2.3f, 15.0f, 40, 40, 40, 200, 2 },
	{ -3.0f, 16.0f, 40, 40, 40, 200, 3 },
	{ 0.0f, 0.0f, 255, 255, 255, 255, 0 }
};


RwTexture *gpCoronaTexture[9] = { nil, nil, nil, nil, nil, nil, nil, nil, nil };

float CCoronas::LightsMult = 1.0f;
float CCoronas::SunScreenX;
float CCoronas::SunScreenY;
int CCoronas::MoonSize;
bool CCoronas::SunBlockedByClouds;
int CCoronas::bChangeBrightnessImmediately;

CRegisteredCorona CCoronas::aCoronas[NUMCORONAS];

const char aCoronaSpriteNames[][32] = {
	"coronastar",
	"corona",
	"coronamoon",
	"coronareflect",
	"coronaheadlightline",
	"coronahex",
	"coronacircle",
	"coronaringa",
	"streek"
};

void
CCoronas::Init(void)
{
	int i;

	CTxdStore::PushCurrentTxd();
	CTxdStore::SetCurrentTxd(CTxdStore::FindTxdSlot("particle"));

	for(i = 0; i < 9; i++)
		if(gpCoronaTexture[i] == nil)
			gpCoronaTexture[i] = RwTextureRead(aCoronaSpriteNames[i], nil);

	CTxdStore::PopCurrentTxd();

	for(i = 0; i < NUMCORONAS; i++)
		aCoronas[i].id = 0;
}

void
CCoronas::Shutdown(void)
{
	int i;
	for(i = 0; i < 9; i++)
		if(gpCoronaTexture[i]){
			RwTextureDestroy(gpCoronaTexture[i]);
			gpCoronaTexture[i] = nil;
		}
}

void
CCoronas::Update(void)
{
	int i;
	static int LastCamLook = 0;

	LightsMult = Min(LightsMult + 0.03f * CTimer::GetTimeStep(), 1.0f);

	int CamLook = 0;
	if(TheCamera.Cams[TheCamera.ActiveCam].LookingLeft) CamLook |= 1;
	if(TheCamera.Cams[TheCamera.ActiveCam].LookingRight) CamLook |= 2;
	if(TheCamera.Cams[TheCamera.ActiveCam].LookingBehind) CamLook |= 4;
	// BUG?
	if(TheCamera.GetLookDirection() == LOOKING_BEHIND) CamLook |= 8;

	if(LastCamLook != CamLook)
		bChangeBrightnessImmediately = 3;
	else
		bChangeBrightnessImmediately = Max(bChangeBrightnessImmediately-1, 0);
	LastCamLook = CamLook;

	for(i = 0; i < NUMCORONAS; i++)
		if(aCoronas[i].id != 0)
			aCoronas[i].Update();
}

void
CCoronas::RegisterCorona(uint32 id, uint8 red, uint8 green, uint8 blue, uint8 alpha,
	const CVector &coors, float size, float drawDist, RwTexture *tex,
	int8 flareType, uint8 reflection, uint8 LOScheck, uint8 drawStreak, float someAngle,
	bool longDist, float nearDist)
{
	int i;

	if(sq(drawDist) < (TheCamera.GetPosition() - coors).MagnitudeSqr2D())
		return;

	for(i = 0; i < NUMCORONAS; i++)
		if(aCoronas[i].id == id)
			break;

	if(i == NUMCORONAS){
		// add a new one

		// find empty slot
		for(i = 0; i < NUMCORONAS; i++)
			if(aCoronas[i].id == 0)
				break;
		if(i == NUMCORONAS)
			return;		// no space

		aCoronas[i].fadeAlpha = 0;
		aCoronas[i].offScreen = true;
		aCoronas[i].firstUpdate = true;
		aCoronas[i].renderReflection = false;
		aCoronas[i].lastLOScheck = 0;
		aCoronas[i].sightClear = false;
		aCoronas[i].hasValue[0] = false;
		aCoronas[i].hasValue[1] = false;
		aCoronas[i].hasValue[2] = false;
		aCoronas[i].hasValue[3] = false;
		aCoronas[i].hasValue[4] = false;
		aCoronas[i].hasValue[5] = false;

	}else{
		// use existing one

		if(aCoronas[i].fadeAlpha == 0 && alpha == 0){
			// unregister
			aCoronas[i].id = 0;
			return;
		}
	}

	aCoronas[i].id = id;
	aCoronas[i].red = red;
	aCoronas[i].green = green;
	aCoronas[i].blue = blue;
	aCoronas[i].alpha = alpha;
	aCoronas[i].coors = coors;
	aCoronas[i].size = size;
	aCoronas[i].someAngle = someAngle;
	aCoronas[i].registeredThisFrame = true;
	aCoronas[i].drawDist = drawDist;
	aCoronas[i].texture = tex;
	aCoronas[i].flareType = flareType;
	aCoronas[i].reflection = reflection;
	aCoronas[i].LOScheck = LOScheck;
	aCoronas[i].drawStreak = drawStreak;
}

void
CCoronas::RegisterCorona(uint32 id, uint8 red, uint8 green, uint8 blue, uint8 alpha,
	const CVector &coors, float size, float drawDist, uint8 type,
	int8 flareType, uint8 reflection, uint8 LOScheck, uint8 drawStreak, float someAngle,
	bool longDist, float nearDist)
{
	RegisterCorona(id, red, green, blue, alpha, coors, size, drawDist,
		gpCoronaTexture[type], flareType, reflection, LOScheck, drawStreak, someAngle,
		longDist, nearDist);
}

void
CCoronas::UpdateCoronaCoors(uint32 id, const CVector &coors, float drawDist, float someAngle)
{
	int i;

	if(sq(drawDist) < (TheCamera.GetPosition() - coors).MagnitudeSqr2D())
		return;

	for(i = 0; i < NUMCORONAS; i++)
		if(aCoronas[i].id == id)
			break;

	if(i == NUMCORONAS)
		return;

	if(aCoronas[i].fadeAlpha == 0)
		aCoronas[i].id = 0;	// faded out, remove
	else{
		aCoronas[i].coors = coors;
		aCoronas[i].someAngle = someAngle;
	}
}

static RwIm2DVertex vertexbufferX[2];

void
CCoronas::Render(void)
{
	int i, j;
	int screenw, screenh;

	screenw = RwRasterGetWidth(RwCameraGetRaster(Scene.camera));
	screenh = RwRasterGetHeight(RwCameraGetRaster(Scene.camera));

	RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)FALSE);
	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)TRUE);
	RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDONE);
	RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDONE);

	for(i = 0; i < NUMCORONAS; i++){
		for(j = 5; j > 0; j--){
			aCoronas[i].prevX[j] = aCoronas[i].prevX[j-1];
			aCoronas[i].prevY[j] = aCoronas[i].prevY[j-1];
			aCoronas[i].prevRed[j] = aCoronas[i].prevRed[j-1];
			aCoronas[i].prevGreen[j] = aCoronas[i].prevGreen[j-1];
			aCoronas[i].prevBlue[j] = aCoronas[i].prevBlue[j-1];
			aCoronas[i].hasValue[j] = aCoronas[i].hasValue[j-1];
		}
		aCoronas[i].hasValue[0] = false;

		if(aCoronas[i].id == 0 ||
		   aCoronas[i].fadeAlpha == 0 && aCoronas[i].alpha == 0)
			continue;

		CVector spriteCoors;
		float spritew, spriteh;
		if(CSprite::CalcScreenCoors(aCoronas[i].coors, spriteCoors, &spritew, &spriteh, true)){
			aCoronas[i].offScreen = false;

			if(spriteCoors.x < 0.0f || spriteCoors.y < 0.0f ||
			   spriteCoors.x > screenw || spriteCoors.y > screenh){
				aCoronas[i].offScreen = true;
				aCoronas[i].sightClear = false;
			}else{
				if(CTimer::GetTimeInMilliseconds() > aCoronas[i].lastLOScheck + 2000){
					aCoronas[i].lastLOScheck = CTimer::GetTimeInMilliseconds();
					aCoronas[i].sightClear = CWorld::GetIsLineOfSightClear(
						aCoronas[i].coors, TheCamera.Cams[TheCamera.ActiveCam].Source,
						true, true, false, false, false, true, false);
				}
			
				// add new streak point
				if(aCoronas[i].sightClear){
					aCoronas[i].prevX[0] = spriteCoors.x;
					aCoronas[i].prevY[0] = spriteCoors.y;
					aCoronas[i].prevRed[0] = aCoronas[i].red;
					aCoronas[i].prevGreen[0] = aCoronas[i].green;
					aCoronas[i].prevBlue[0] = aCoronas[i].blue;
					aCoronas[i].hasValue[0] = true;
				}
			
				// if distance too big, break streak
				if(aCoronas[i].hasValue[1]){
					if(Abs(aCoronas[i].prevX[0] - aCoronas[i].prevX[1]) > 50.0f ||
					   Abs(aCoronas[i].prevY[0] - aCoronas[i].prevY[1]) > 50.0f)
						aCoronas[i].hasValue[0] = false;
				}
			}


			if(aCoronas[i].fadeAlpha == 0)
				continue;

			if(spriteCoors.z < aCoronas[i].drawDist){
				float recipz = 1.0f/spriteCoors.z;
				float fadeDistance = aCoronas[i].drawDist / 2.0f;
				float distanceFade = spriteCoors.z < fadeDistance ? 1.0f : 1.0f - (spriteCoors.z - fadeDistance)/fadeDistance;
				int totalFade = aCoronas[i].fadeAlpha * distanceFade;

				if(aCoronas[i].LOScheck)
					RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*)FALSE);
				else
					RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*)TRUE);

				// render corona itself
				if(aCoronas[i].texture){
					float fogscale = CWeather::Foggyness*Min(spriteCoors.z, 40.0f)/40.0f + 1.0f;
					if(CCoronas::aCoronas[i].id == SUN_CORE)
						spriteCoors.z = 0.95f * RwCameraGetFarClipPlane(Scene.camera);
					RwRenderStateSet(rwRENDERSTATETEXTURERASTER, RwTextureGetRaster(aCoronas[i].texture));
					spriteCoors.z -= 1.5f;

					if(aCoronas[i].texture == gpCoronaTexture[8]){
						// what's this?
						float f = 1.0f - aCoronas[i].someAngle*2.0f/PI;
						float wscale = 6.0f*sq(sq(sq(f))) + 0.5f;
						float hscale = 0.35f - (wscale - 0.5f) * 0.06f;
						hscale = Max(hscale, 0.15f);

						CSprite::RenderOneXLUSprite(spriteCoors.x, spriteCoors.y, spriteCoors.z,
							spritew * aCoronas[i].size * wscale,
							spriteh * aCoronas[i].size * fogscale * hscale,
							CCoronas::aCoronas[i].red / fogscale,
							CCoronas::aCoronas[i].green / fogscale,
							CCoronas::aCoronas[i].blue / fogscale,
							totalFade,
							recipz,
							255);
					}else{
						CSprite::RenderOneXLUSprite_Rotate_Aspect(
							spriteCoors.x, spriteCoors.y, spriteCoors.z,
							spritew * aCoronas[i].size * fogscale,
							spriteh * aCoronas[i].size * fogscale,
							CCoronas::aCoronas[i].red / fogscale,
							CCoronas::aCoronas[i].green / fogscale,
							CCoronas::aCoronas[i].blue / fogscale,
							totalFade,
							recipz,
							20.0f * recipz,
							255);
					}
				}

				// render flares
				if(aCoronas[i].flareType != FLARE_NONE){
					FlareDef *flare;

					switch(aCoronas[i].flareType){
					case FLARE_SUN: flare = SunFlareDef; break;
					case FLARE_HEADLIGHTS: flare = HeadLightsFlareDef; break;
					default: assert(0);
					}

					for(; flare->texture; flare++){
						RwRenderStateSet(rwRENDERSTATETEXTURERASTER, RwTextureGetRaster(gpCoronaTexture[flare->texture + 4]));
						CSprite::RenderOneXLUSprite(
							(spriteCoors.x - (screenw/2)) * flare->position + (screenw/2),
							(spriteCoors.y - (screenh/2)) * flare->position + (screenh/2),
							spriteCoors.z,
							4.0f*flare->size * spritew/spriteh,
							4.0f*flare->size,
							(flare->red * aCoronas[i].red)>>8,
							(flare->green * aCoronas[i].green)>>8,
							(flare->blue * aCoronas[i].blue)>>8,
							(totalFade * flare->alpha)>>8,
							recipz, 255);
					}
				}
			}else{
				aCoronas[i].offScreen = true;
				aCoronas[i].sightClear = false;
			}
		}
	}

	RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)TRUE);
	RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*)FALSE);
	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)TRUE);
	RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDONE);
	RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDONE);
	RwRenderStateSet(rwRENDERSTATETEXTURERASTER, nil);

	// streaks
	for(i = 0; i < NUMCORONAS; i++){
		if(aCoronas[i].id == 0 || !aCoronas[i].drawStreak)
			continue;

		for(j = 0; j < 5; j++){
			if(!aCoronas[i].hasValue[j] || !aCoronas[i].hasValue[j+1])
				continue;

			int mod1 = (float)(6 - j) / 6 * 128;
			int mod2 = (float)(6 - (j+1)) / 6 * 128;

			RwIm2DVertexSetScreenX(&vertexbufferX[0], aCoronas[i].prevX[j]);
			RwIm2DVertexSetScreenY(&vertexbufferX[0], aCoronas[i].prevY[j]);
			RwIm2DVertexSetIntRGBA(&vertexbufferX[0], aCoronas[i].prevRed[j] * mod1 / 256, aCoronas[i].prevGreen[j] * mod1 / 256, aCoronas[i].prevBlue[j] * mod1 / 256, 255);
			RwIm2DVertexSetScreenX(&vertexbufferX[1], aCoronas[i].prevX[j+1]);
			RwIm2DVertexSetScreenY(&vertexbufferX[1], aCoronas[i].prevY[j+1]);
			RwIm2DVertexSetIntRGBA(&vertexbufferX[1], aCoronas[i].prevRed[j+1] * mod2 / 256, aCoronas[i].prevGreen[j+1] * mod2 / 256, aCoronas[i].prevBlue[j+1] * mod2 / 256, 255);

			// BUG: game doesn't do this
			RwIm2DVertexSetScreenZ(&vertexbufferX[0], RwIm2DGetNearScreenZ());
			RwIm2DVertexSetCameraZ(&vertexbufferX[0], RwCameraGetNearClipPlane(Scene.camera));
			RwIm2DVertexSetRecipCameraZ(&vertexbufferX[0], 1.0f/RwCameraGetNearClipPlane(Scene.camera));
			RwIm2DVertexSetScreenZ(&vertexbufferX[1], RwIm2DGetNearScreenZ());
			RwIm2DVertexSetCameraZ(&vertexbufferX[1], RwCameraGetNearClipPlane(Scene.camera));
			RwIm2DVertexSetRecipCameraZ(&vertexbufferX[1], 1.0f/RwCameraGetNearClipPlane(Scene.camera));

			RwIm2DRenderLine(vertexbufferX, 2, 0, 1);
		}
	}

	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)FALSE);
	RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*)TRUE);
	RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)TRUE);
}

void
CCoronas::RenderReflections(void)
{
	int i;
	CColPoint point;
	CEntity *entity;

	if(CWeather::WetRoads > 0.0f){
		RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void*)FALSE);
		RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)FALSE);
		RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*)FALSE);
		RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)TRUE);
		RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDONE);
		RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDONE);
		RwRenderStateSet(rwRENDERSTATETEXTURERASTER, RwTextureGetRaster(gpCoronaTexture[3]));

		for(i = 0; i < NUMCORONAS; i++){
			if(aCoronas[i].id == 0 ||
			   aCoronas[i].fadeAlpha == 0 && aCoronas[i].alpha == 0)
				continue;

			// check if we want a reflection on this corona
			if(aCoronas[i].renderReflection){
				if(((CTimer::GetFrameCounter() + i) & 0xF) == 0 &&
				   CWorld::ProcessVerticalLine(aCoronas[i].coors, -1000.0f, point, entity, true, false, false, false, true, false, nil))
					aCoronas[i].heightAboveRoad = aCoronas[i].coors.z - point.point.z;
			}else{
				if(CWorld::ProcessVerticalLine(aCoronas[i].coors, -1000.0f, point, entity, true, false, false, false, true, false, nil)){
					aCoronas[i].heightAboveRoad = aCoronas[i].coors.z - point.point.z;
					aCoronas[i].renderReflection = true;
				}
			}

			if(!aCoronas[i].renderReflection)
				continue;

			// Don't draw if reflection is too high
			if(aCoronas[i].heightAboveRoad < 20.0f){
				// don't draw if camera is below road
				if(CCoronas::aCoronas[i].coors.z - aCoronas[i].heightAboveRoad > TheCamera.GetPosition().z)
					continue;

				CVector coors = aCoronas[i].coors;
				coors.z -= 2.0f*aCoronas[i].heightAboveRoad;

				CVector spriteCoors;
				float spritew, spriteh;
				if(CSprite::CalcScreenCoors(coors, spriteCoors, &spritew, &spriteh, true)){
					float drawDist = 0.75f * aCoronas[i].drawDist;
					drawDist = Min(drawDist, 50.0f);
					if(spriteCoors.z < drawDist){
						float fadeDistance = drawDist / 2.0f;
						float distanceFade = spriteCoors.z < fadeDistance ? 1.0f : 1.0f - (spriteCoors.z - fadeDistance)/fadeDistance;
						distanceFade = clamp(distanceFade, 0.0f, 1.0f);
						float recipz = 1.0f/RwCameraGetNearClipPlane(Scene.camera);
						int intensity = (20.0f - aCoronas[i].heightAboveRoad) * 230.0 * distanceFade*CWeather::WetRoads * 0.05f;

						CSprite::RenderBufferedOneXLUSprite(
							spriteCoors.x, spriteCoors.y, RwIm2DGetNearScreenZ(),
							spritew * aCoronas[i].size * 0.75f,
							spriteh * aCoronas[i].size * 2.0f,
							(intensity * CCoronas::aCoronas[i].red)>>8,
							(intensity * CCoronas::aCoronas[i].green)>>8,
							(intensity * CCoronas::aCoronas[i].blue)>>8,
							255,
							recipz,
							255);
					}
				}
			}
		}
		CSprite::FlushSpriteBuffer();

		RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDSRCALPHA);
		RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDINVSRCALPHA);
		RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)FALSE);
		RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)TRUE);
		RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*)TRUE);
	}else{
		for(i = 0; i < NUMCORONAS; i++)
			aCoronas[i].renderReflection = false;
	}
}

void 
CCoronas::DoSunAndMoon(void)
{
	// yeah, moon is done somewhere else....

	CVector sunCoors = CTimeCycle::GetSunPosition();
	sunCoors *= 150.0f;
	sunCoors += TheCamera.GetPosition();

	if(CTimeCycle::GetSunPosition().z > -0.2f){
		float size = ((CGeneral::GetRandomNumber()&0xFF) * 0.005f + 10.0f) * CTimeCycle::GetSunSize();
		RegisterCorona(SUN_CORE,
			CTimeCycle::GetSunCoreRed(), CTimeCycle::GetSunCoreGreen(), CTimeCycle::GetSunCoreBlue(),
			255, sunCoors, size,
			999999.88f, TYPE_STAR, FLARE_NONE, REFLECTION_OFF, LOSCHECK_OFF, STREAK_OFF, 0.0f);

		if(CTimeCycle::GetSunPosition().z > 0.0f)
			RegisterCorona(SUN_CORONA,
				CTimeCycle::GetSunCoronaRed(), CTimeCycle::GetSunCoronaGreen(), CTimeCycle::GetSunCoronaBlue(),
				255, sunCoors, 25.0f * CTimeCycle::GetSunSize(),
				999999.88f, TYPE_STAR, FLARE_SUN, REFLECTION_OFF, LOSCHECK_ON, STREAK_OFF, 0.0f);
	}

	CVector spriteCoors;
	float spritew, spriteh;
	if(CSprite::CalcScreenCoors(sunCoors, spriteCoors, &spritew, &spriteh, true)){
		SunScreenX = spriteCoors.x;
		SunScreenY = spriteCoors.y;
	}else{
		SunScreenX = 1000000.0f;
		SunScreenY = 1000000.0f;
	}
}

void
CRegisteredCorona::Update(void)
{
	if(!registeredThisFrame)
		alpha = 0;

	if(LOScheck &&
	   (CCoronas::SunBlockedByClouds && id == CCoronas::SUN_CORONA ||
	    !CWorld::GetIsLineOfSightClear(coors, TheCamera.GetPosition(), true, false, false, false, false, false))){
		// Corona is blocked, fade out
		fadeAlpha = Max(fadeAlpha - 15.0f*CTimer::GetTimeStep(), 0.0f);
	}else if(offScreen){
		// Same when off screen
		fadeAlpha = Max(fadeAlpha - 15.0f*CTimer::GetTimeStep(), 0.0f);
	}else{
		// Visible
		if(alpha > fadeAlpha){
			// fade in
			fadeAlpha = Min(fadeAlpha + 15.0f*CTimer::GetTimeStep(), alpha);
			if(CCoronas::bChangeBrightnessImmediately)
				fadeAlpha = alpha;
		}else if(alpha < fadeAlpha){
			// too visible, decrease alpha but not below alpha
			fadeAlpha = Max(fadeAlpha - 15.0f*CTimer::GetTimeStep(), alpha);
		}

		// darken scene when the sun is visible
		if(id == CCoronas::SUN_CORONA)
			CCoronas::LightsMult = Max(CCoronas::LightsMult - CTimer::GetTimeStep()*0.06f, 0.6f);
	}

	// remove if invisible
	if(fadeAlpha == 0 && !firstUpdate)
		id = 0;
	firstUpdate = false;
	registeredThisFrame = false;
}
