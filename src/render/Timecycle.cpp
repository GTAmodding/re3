#include "common.h"

#include "main.h"
#include "Clock.h"
#include "Weather.h"
#include "Camera.h"
#include "Shadows.h"
#include "ZoneCull.h"
#include "CutsceneMgr.h"
#include "FileMgr.h"
#include "Timecycle.h"

// TODO(MIAMI): change some of the types here

int   CTimeCycle::m_nAmbientRed[NUMHOURS][NUMWEATHERS];
int   CTimeCycle::m_nAmbientGreen[NUMHOURS][NUMWEATHERS];
int   CTimeCycle::m_nAmbientBlue[NUMHOURS][NUMWEATHERS];
int   CTimeCycle::m_nAmbientRed_Obj[NUMHOURS][NUMWEATHERS];
int   CTimeCycle::m_nAmbientGreen_Obj[NUMHOURS][NUMWEATHERS];
int   CTimeCycle::m_nAmbientBlue_Obj[NUMHOURS][NUMWEATHERS];
int   CTimeCycle::m_nAmbientRed_Bl[NUMHOURS][NUMWEATHERS];
int   CTimeCycle::m_nAmbientGreen_Bl[NUMHOURS][NUMWEATHERS];
int   CTimeCycle::m_nAmbientBlue_Bl[NUMHOURS][NUMWEATHERS];
int   CTimeCycle::m_nAmbientRed_Obj_Bl[NUMHOURS][NUMWEATHERS];
int   CTimeCycle::m_nAmbientGreen_Obj_Bl[NUMHOURS][NUMWEATHERS];
int   CTimeCycle::m_nAmbientBlue_Obj_Bl[NUMHOURS][NUMWEATHERS];
int   CTimeCycle::m_nDirectionalRed[NUMHOURS][NUMWEATHERS];
int   CTimeCycle::m_nDirectionalGreen[NUMHOURS][NUMWEATHERS];
int   CTimeCycle::m_nDirectionalBlue[NUMHOURS][NUMWEATHERS];
int   CTimeCycle::m_nSkyTopRed[NUMHOURS][NUMWEATHERS];
int   CTimeCycle::m_nSkyTopGreen[NUMHOURS][NUMWEATHERS];
int   CTimeCycle::m_nSkyTopBlue[NUMHOURS][NUMWEATHERS];
int   CTimeCycle::m_nSkyBottomRed[NUMHOURS][NUMWEATHERS];
int   CTimeCycle::m_nSkyBottomGreen[NUMHOURS][NUMWEATHERS];
int   CTimeCycle::m_nSkyBottomBlue[NUMHOURS][NUMWEATHERS];
int   CTimeCycle::m_nSunCoreRed[NUMHOURS][NUMWEATHERS];
int   CTimeCycle::m_nSunCoreGreen[NUMHOURS][NUMWEATHERS];
int   CTimeCycle::m_nSunCoreBlue[NUMHOURS][NUMWEATHERS];
int   CTimeCycle::m_nSunCoronaRed[NUMHOURS][NUMWEATHERS];
int   CTimeCycle::m_nSunCoronaGreen[NUMHOURS][NUMWEATHERS];
int   CTimeCycle::m_nSunCoronaBlue[NUMHOURS][NUMWEATHERS];
float CTimeCycle::m_fSunSize[NUMHOURS][NUMWEATHERS];
float CTimeCycle::m_fSpriteSize[NUMHOURS][NUMWEATHERS];
float CTimeCycle::m_fSpriteBrightness[NUMHOURS][NUMWEATHERS];
short CTimeCycle::m_nShadowStrength[NUMHOURS][NUMWEATHERS];
short CTimeCycle::m_nLightShadowStrength[NUMHOURS][NUMWEATHERS];
short CTimeCycle::m_nPoleShadowStrength[NUMHOURS][NUMWEATHERS];
float CTimeCycle::m_fFogStart[NUMHOURS][NUMWEATHERS];
float CTimeCycle::m_fFarClip[NUMHOURS][NUMWEATHERS];
float CTimeCycle::m_fLightsOnGroundBrightness[NUMHOURS][NUMWEATHERS];
int   CTimeCycle::m_nLowCloudsRed[NUMHOURS][NUMWEATHERS];
int   CTimeCycle::m_nLowCloudsGreen[NUMHOURS][NUMWEATHERS];
int   CTimeCycle::m_nLowCloudsBlue[NUMHOURS][NUMWEATHERS];
int   CTimeCycle::m_nFluffyCloudsTopRed[NUMHOURS][NUMWEATHERS];
int   CTimeCycle::m_nFluffyCloudsTopGreen[NUMHOURS][NUMWEATHERS];
int   CTimeCycle::m_nFluffyCloudsTopBlue[NUMHOURS][NUMWEATHERS];
int   CTimeCycle::m_nFluffyCloudsBottomRed[NUMHOURS][NUMWEATHERS];
int   CTimeCycle::m_nFluffyCloudsBottomGreen[NUMHOURS][NUMWEATHERS];
int   CTimeCycle::m_nFluffyCloudsBottomBlue[NUMHOURS][NUMWEATHERS];
float CTimeCycle::m_fBlurRed[NUMHOURS][NUMWEATHERS];
float CTimeCycle::m_fBlurGreen[NUMHOURS][NUMWEATHERS];
float CTimeCycle::m_fBlurBlue[NUMHOURS][NUMWEATHERS];
float CTimeCycle::m_fWaterRed[NUMHOURS][NUMWEATHERS];
float CTimeCycle::m_fWaterGreen[NUMHOURS][NUMWEATHERS];
float CTimeCycle::m_fWaterBlue[NUMHOURS][NUMWEATHERS];
float CTimeCycle::m_fWaterAlpha[NUMHOURS][NUMWEATHERS];


float CTimeCycle::m_fCurrentAmbientRed;
float CTimeCycle::m_fCurrentAmbientGreen;
float CTimeCycle::m_fCurrentAmbientBlue;
float CTimeCycle::m_fCurrentAmbientRed_Obj;
float CTimeCycle::m_fCurrentAmbientGreen_Obj;
float CTimeCycle::m_fCurrentAmbientBlue_Obj;
float CTimeCycle::m_fCurrentAmbientRed_Bl;
float CTimeCycle::m_fCurrentAmbientGreen_Bl;
float CTimeCycle::m_fCurrentAmbientBlue_Bl;
float CTimeCycle::m_fCurrentAmbientRed_Obj_Bl;
float CTimeCycle::m_fCurrentAmbientGreen_Obj_Bl;
float CTimeCycle::m_fCurrentAmbientBlue_Obj_Bl;
float CTimeCycle::m_fCurrentDirectionalRed;
float CTimeCycle::m_fCurrentDirectionalGreen;
float CTimeCycle::m_fCurrentDirectionalBlue;
int   CTimeCycle::m_nCurrentSkyTopRed;
int   CTimeCycle::m_nCurrentSkyTopGreen;
int   CTimeCycle::m_nCurrentSkyTopBlue;
int   CTimeCycle::m_nCurrentSkyBottomRed;
int   CTimeCycle::m_nCurrentSkyBottomGreen;
int   CTimeCycle::m_nCurrentSkyBottomBlue;
int   CTimeCycle::m_nCurrentSunCoreRed;
int   CTimeCycle::m_nCurrentSunCoreGreen;
int   CTimeCycle::m_nCurrentSunCoreBlue;
int   CTimeCycle::m_nCurrentSunCoronaRed;
int   CTimeCycle::m_nCurrentSunCoronaGreen;
int   CTimeCycle::m_nCurrentSunCoronaBlue;
float CTimeCycle::m_fCurrentSunSize;
float CTimeCycle::m_fCurrentSpriteSize;
float CTimeCycle::m_fCurrentSpriteBrightness;
int   CTimeCycle::m_nCurrentShadowStrength;
int   CTimeCycle::m_nCurrentLightShadowStrength;
int   CTimeCycle::m_nCurrentPoleShadowStrength;
float CTimeCycle::m_fCurrentFogStart;
float CTimeCycle::m_fCurrentFarClip;
float CTimeCycle::m_fCurrentLightsOnGroundBrightness;
int   CTimeCycle::m_nCurrentLowCloudsRed;
int   CTimeCycle::m_nCurrentLowCloudsGreen;
int   CTimeCycle::m_nCurrentLowCloudsBlue;
int   CTimeCycle::m_nCurrentFluffyCloudsTopRed;
int   CTimeCycle::m_nCurrentFluffyCloudsTopGreen;
int   CTimeCycle::m_nCurrentFluffyCloudsTopBlue;
int   CTimeCycle::m_nCurrentFluffyCloudsBottomRed;
int   CTimeCycle::m_nCurrentFluffyCloudsBottomGreen;
int   CTimeCycle::m_nCurrentFluffyCloudsBottomBlue;
float CTimeCycle::m_fCurrentBlurRed;
float CTimeCycle::m_fCurrentBlurGreen;
float CTimeCycle::m_fCurrentBlurBlue;
float CTimeCycle::m_fCurrentWaterRed;
float CTimeCycle::m_fCurrentWaterGreen;
float CTimeCycle::m_fCurrentWaterBlue;
float CTimeCycle::m_fCurrentWaterAlpha;
int   CTimeCycle::m_nCurrentFogColourRed;
int   CTimeCycle::m_nCurrentFogColourGreen;
int   CTimeCycle::m_nCurrentFogColourBlue;

int   CTimeCycle::m_FogReduction;

int   CTimeCycle::m_CurrentStoredValue;
CVector CTimeCycle::m_VectorToSun[16];
float CTimeCycle::m_fShadowFrontX[16];
float CTimeCycle::m_fShadowFrontY[16];
float CTimeCycle::m_fShadowSideX[16];
float CTimeCycle::m_fShadowSideY[16];
float CTimeCycle::m_fShadowDisplacementX[16];
float CTimeCycle::m_fShadowDisplacementY[16];


void
CTimeCycle::Initialise(void)
{
	int w, h;
	int li, bi;
	char line[1040];

	int ambR, ambG, ambB;
	int ambobjR, ambobjG, ambobjB;
	int ambblR, ambblG, ambblB;
	int ambobjblR, ambobjblG, ambobjblB;
	int dirR, dirG, dirB;
	int skyTopR, skyTopG, skyTopB;
	int skyBotR, skyBotG, skyBotB;
	int sunCoreR, sunCoreG, sunCoreB;
	int sunCoronaR, sunCoronaG, sunCoronaB;
	float sunSz, sprSz, sprBght;
	int shad, lightShad, poleShad;
	float farClp, fogSt, lightGnd;
	int cloudR, cloudG, cloudB;
	int fluffyTopR, fluffyTopG, fluffyTopB;
	int fluffyBotR, fluffyBotG, fluffyBotB;
	float blurR, blurG, blurB;
	float waterR, waterG, waterB, waterA;

	debug("Intialising CTimeCycle...\n");

	CFileMgr::SetDir("DATA");
	CFileMgr::LoadFile("TIMECYC.DAT", work_buff, sizeof(work_buff), "rb");
	CFileMgr::SetDir("");

	line[0] = '\0';
	bi = 0;
	for(w = 0; w < NUMWEATHERS; w++)
		for(h = 0; h < NUMHOURS; h++){
			li = 0;
			while(work_buff[bi] == '/'){
				while(work_buff[bi] != '\n')
					bi++;
				bi++;
			}
			while(work_buff[bi] != '\n'
#ifdef FIX_BUGS
			&& work_buff[bi] != '\0'
#endif
			)
				line[li++] = work_buff[bi++];
			line[li] = '\0';
			bi++;

			sscanf(line, "%d %d %d  %d %d %d  %d %d %d  %d %d %d "
			             "%d %d %d  %d %d %d  %d %d %d "
			             "%d %d %d  %d %d %d  %f %f %f %d %d %d %f %f %f "
			             "%d %d %d  %d %d %d  %d %d %d  %f %f %f  %f %f %f %f",
				&ambR, &ambG, &ambB,
				 &ambobjR, &ambobjG, &ambobjB,
				 &ambblR, &ambblG, &ambblB,
				 &ambobjblR, &ambobjblG, &ambobjblB,
				&dirR, &dirG, &dirB,
				 &skyTopR, &skyTopG, &skyTopB,
				 &skyBotR, &skyBotG, &skyBotB,
				&sunCoreR, &sunCoreG, &sunCoreB,
				 &sunCoronaR, &sunCoronaG, &sunCoronaB,
				 &sunSz, &sprSz, &sprBght,
				 &shad, &lightShad, &poleShad,
				 &farClp, &fogSt, &lightGnd,
				&cloudR, &cloudG, &cloudB,
				 &fluffyTopR, &fluffyTopG, &fluffyTopB,
				 &fluffyBotR, &fluffyBotG, &fluffyBotB,
				 &blurR, &blurG, &blurB,
				 &waterR, &waterG, &waterB, &waterA);

			m_nAmbientRed[h][w] = ambR;
			m_nAmbientGreen[h][w] = ambG;
			m_nAmbientBlue[h][w] = ambB;
			m_nAmbientRed_Obj[h][w] = ambobjR;
			m_nAmbientGreen_Obj[h][w] = ambobjG;
			m_nAmbientBlue_Obj[h][w] = ambobjB;
			m_nAmbientRed_Bl[h][w] = ambblR;
			m_nAmbientGreen_Bl[h][w] = ambblG;
			m_nAmbientBlue_Bl[h][w] = ambblB;
			m_nAmbientRed_Obj_Bl[h][w] = ambobjblR;
			m_nAmbientGreen_Obj_Bl[h][w] = ambobjblG;
			m_nAmbientBlue_Obj_Bl[h][w] = ambobjblB;
			m_nDirectionalRed[h][w] = dirR;
			m_nDirectionalGreen[h][w] = dirG;
			m_nDirectionalBlue[h][w] = dirB;
			m_nSkyTopRed[h][w] = skyTopR;
			m_nSkyTopGreen[h][w] = skyTopG;
			m_nSkyTopBlue[h][w] = skyTopB;
			m_nSkyBottomRed[h][w] = skyBotR;
			m_nSkyBottomGreen[h][w] = skyBotG;
			m_nSkyBottomBlue[h][w] = skyBotB;
			m_nSunCoreRed[h][w] = sunCoreR;
			m_nSunCoreGreen[h][w] = sunCoreG;
			m_nSunCoreBlue[h][w] = sunCoreB;
			m_nSunCoronaRed[h][w] = sunCoronaR;
			m_nSunCoronaGreen[h][w] = sunCoronaG;
			m_nSunCoronaBlue[h][w] = sunCoronaB;
			m_fSunSize[h][w] = sunSz;
			m_fSpriteSize[h][w] = sprSz;
			m_fSpriteBrightness[h][w] = sprBght;
			m_nShadowStrength[h][w] = shad;
			m_nLightShadowStrength[h][w] = lightShad;
			m_nPoleShadowStrength[h][w] = poleShad;
			m_fFarClip[h][w] = farClp;
			m_fFogStart[h][w] = fogSt;
			m_fLightsOnGroundBrightness[h][w] = lightGnd;
			m_nLowCloudsRed[h][w] = cloudR;
			m_nLowCloudsGreen[h][w] = cloudG;
			m_nLowCloudsBlue[h][w] = cloudB;
			m_nFluffyCloudsTopRed[h][w] = fluffyTopR;
			m_nFluffyCloudsTopGreen[h][w] = fluffyTopG;
			m_nFluffyCloudsTopBlue[h][w] = fluffyTopB;
			m_nFluffyCloudsBottomRed[h][w] = fluffyBotR;
			m_nFluffyCloudsBottomGreen[h][w] = fluffyBotG;
			m_nFluffyCloudsBottomBlue[h][w] = fluffyBotB;
			m_fBlurRed[h][w] = blurR;
			m_fBlurGreen[h][w] = blurG;
			m_fBlurBlue[h][w] = blurB;
			m_fWaterRed[h][w] = waterR;
			m_fWaterGreen[h][w] = waterG;
			m_fWaterBlue[h][w] = waterB;
			m_fWaterAlpha[h][w] = waterA;
		}

	m_FogReduction = 0;

	debug("CTimeCycle ready\n");
}

void
CTimeCycle::Update(void)
{
	int h1 = CClock::GetHours();
	int h2 = (h1+1)%24;
	int w1 = CWeather::OldWeatherType;
	int w2 = CWeather::NewWeatherType;
	float timeInterp = (CClock::GetMinutes() + CClock::GetSeconds()/60.0f)/60.0f;
	// coefficients for a bilinear interpolation
	float c0 = (1.0f-timeInterp) * (1.0f-CWeather::InterpolationValue);
	float c1 = timeInterp * (1.0f-CWeather::InterpolationValue);
	float c2 = (1.0f-timeInterp) * CWeather::InterpolationValue;
	float c3 = timeInterp * CWeather::InterpolationValue;

#define INTERP(v) v[h1][w1]*c0 + v[h2][w1]*c1 + v[h1][w2]*c2 + v[h2][w2]*c3;

	m_nCurrentSkyTopRed = INTERP(m_nSkyTopRed);
	m_nCurrentSkyTopGreen = INTERP(m_nSkyTopGreen);
	m_nCurrentSkyTopBlue = INTERP(m_nSkyTopBlue);

	m_nCurrentSkyBottomRed = INTERP(m_nSkyBottomRed);
	m_nCurrentSkyBottomGreen = INTERP(m_nSkyBottomGreen);
	m_nCurrentSkyBottomBlue = INTERP(m_nSkyBottomBlue);

	m_fCurrentAmbientRed = INTERP(m_nAmbientRed);
	m_fCurrentAmbientGreen = INTERP(m_nAmbientGreen);
	m_fCurrentAmbientBlue = INTERP(m_nAmbientBlue);

	m_fCurrentAmbientRed_Obj = INTERP(m_nAmbientRed_Obj);
	m_fCurrentAmbientGreen_Obj = INTERP(m_nAmbientGreen_Obj);
	m_fCurrentAmbientBlue_Obj = INTERP(m_nAmbientBlue_Obj);

	m_fCurrentAmbientRed_Bl = INTERP(m_nAmbientRed_Bl);
	m_fCurrentAmbientGreen_Bl = INTERP(m_nAmbientGreen_Bl);
	m_fCurrentAmbientBlue_Bl = INTERP(m_nAmbientBlue_Bl);

	m_fCurrentAmbientRed_Obj_Bl = INTERP(m_nAmbientRed_Obj_Bl);
	m_fCurrentAmbientGreen_Obj_Bl = INTERP(m_nAmbientGreen_Obj_Bl);
	m_fCurrentAmbientBlue_Obj_Bl = INTERP(m_nAmbientBlue_Obj_Bl);

	m_fCurrentDirectionalRed = INTERP(m_nDirectionalRed);
	m_fCurrentDirectionalGreen = INTERP(m_nDirectionalGreen);
	m_fCurrentDirectionalBlue = INTERP(m_nDirectionalBlue);

	m_nCurrentSunCoreRed = INTERP(m_nSunCoreRed);
	m_nCurrentSunCoreGreen = INTERP(m_nSunCoreGreen);
	m_nCurrentSunCoreBlue = INTERP(m_nSunCoreBlue);

	m_nCurrentSunCoronaRed = INTERP(m_nSunCoronaRed);
	m_nCurrentSunCoronaGreen = INTERP(m_nSunCoronaGreen);
	m_nCurrentSunCoronaBlue = INTERP(m_nSunCoronaBlue);

	m_fCurrentSunSize = INTERP(m_fSunSize);
	m_fCurrentSpriteSize = INTERP(m_fSpriteSize);
	m_fCurrentSpriteBrightness = INTERP(m_fSpriteBrightness);
	m_nCurrentShadowStrength = INTERP(m_nShadowStrength);
	m_nCurrentLightShadowStrength = INTERP(m_nLightShadowStrength);
	m_nCurrentPoleShadowStrength = INTERP(m_nPoleShadowStrength);
	m_fCurrentFarClip = INTERP(m_fFarClip);
	m_fCurrentFogStart = INTERP(m_fFogStart);
	m_fCurrentLightsOnGroundBrightness = INTERP(m_fLightsOnGroundBrightness);

	m_nCurrentLowCloudsRed = INTERP(m_nLowCloudsRed);
	m_nCurrentLowCloudsGreen = INTERP(m_nLowCloudsGreen);
	m_nCurrentLowCloudsBlue = INTERP(m_nLowCloudsBlue);

	m_nCurrentFluffyCloudsTopRed = INTERP(m_nFluffyCloudsTopRed);
	m_nCurrentFluffyCloudsTopGreen = INTERP(m_nFluffyCloudsTopGreen);
	m_nCurrentFluffyCloudsTopBlue = INTERP(m_nFluffyCloudsTopBlue);

	m_nCurrentFluffyCloudsBottomRed = INTERP(m_nFluffyCloudsBottomRed);
	m_nCurrentFluffyCloudsBottomGreen = INTERP(m_nFluffyCloudsBottomGreen);
	m_nCurrentFluffyCloudsBottomBlue = INTERP(m_nFluffyCloudsBottomBlue);

	m_fCurrentBlurRed = INTERP(m_fBlurRed);
	m_fCurrentBlurGreen = INTERP(m_fBlurGreen);
	m_fCurrentBlurBlue = INTERP(m_fBlurBlue);

	m_fCurrentWaterRed = INTERP(m_fWaterRed);
	m_fCurrentWaterGreen = INTERP(m_fWaterGreen);
	m_fCurrentWaterBlue = INTERP(m_fWaterBlue);
	m_fCurrentWaterAlpha = INTERP(m_fWaterAlpha);

	if(m_FogReduction != 0)
		m_fCurrentFarClip = Max(m_fCurrentFarClip, m_FogReduction/64.0f * 650.0f);

	m_CurrentStoredValue = (m_CurrentStoredValue+1)&0xF;

	float sunAngle = 2*PI*(CClock::GetSeconds()/60.0f + CClock::GetMinutes() + CClock::GetHours()*60)/(24*60);
	CVector &sunPos = GetSunDirection();
	sunPos.x = Sin(sunAngle);
	sunPos.y = 1.0f;
	sunPos.z = 0.2f - Cos(sunAngle);
	sunPos.Normalise();

	// TODO(MIAMI): extra colours

	if(TheCamera.m_BlurType == MBLUR_NONE || TheCamera.m_BlurType == MBLUR_NORMAL)
		TheCamera.SetMotionBlur(m_fCurrentBlurRed, m_fCurrentBlurGreen, m_fCurrentBlurBlue, 5, MBLUR_NORMAL);

	m_nCurrentFogColourRed = (m_nCurrentSkyTopRed + 2*m_nCurrentSkyBottomRed) / 3;
	m_nCurrentFogColourGreen = (m_nCurrentSkyTopGreen + 2*m_nCurrentSkyBottomGreen) / 3;
	m_nCurrentFogColourBlue = (m_nCurrentSkyTopBlue + 2*m_nCurrentSkyBottomBlue) / 3;

	m_fCurrentAmbientRed /= 255.0f;
	m_fCurrentAmbientGreen /= 255.0f;
	m_fCurrentAmbientBlue /= 255.0f;
	m_fCurrentAmbientRed_Obj /= 255.0f;
	m_fCurrentAmbientGreen_Obj /= 255.0f;
	m_fCurrentAmbientBlue_Obj /= 255.0f;
	m_fCurrentAmbientRed_Bl /= 255.0f;
	m_fCurrentAmbientGreen_Bl /= 255.0f;
	m_fCurrentAmbientBlue_Bl /= 255.0f;
	m_fCurrentAmbientRed_Obj_Bl /= 255.0f;
	m_fCurrentAmbientGreen_Obj_Bl /= 255.0f;
	m_fCurrentAmbientBlue_Obj_Bl /= 255.0f;
	m_fCurrentDirectionalRed /= 255.0f;
	m_fCurrentDirectionalGreen /= 255.0f;
	m_fCurrentDirectionalBlue /= 255.0f;

	CShadows::CalcPedShadowValues(sunPos, 
		&m_fShadowFrontX[m_CurrentStoredValue], &m_fShadowFrontY[m_CurrentStoredValue],
		&m_fShadowSideX[m_CurrentStoredValue], &m_fShadowSideY[m_CurrentStoredValue],
		&m_fShadowDisplacementX[m_CurrentStoredValue], &m_fShadowDisplacementY[m_CurrentStoredValue]);

	if(TheCamera.GetForward().z < -0.9f ||
	   !CWeather::bScriptsForceRain && (CCullZones::PlayerNoRain() || CCullZones::CamNoRain() || CCutsceneMgr::IsRunning()))
		m_FogReduction = Min(m_FogReduction+1, 64);
	else
		m_FogReduction = Max(m_FogReduction-1, 0);
}
