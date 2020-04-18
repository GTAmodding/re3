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

int   CTimeCycle::m_nAmbientRed[NUMHOURS][NUMWEATHERS];
int   CTimeCycle::m_nAmbientGreen[NUMHOURS][NUMWEATHERS];
int   CTimeCycle::m_nAmbientBlue[NUMHOURS][NUMWEATHERS];
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
short CTimeCycle::m_nTreeShadowStrength[NUMHOURS][NUMWEATHERS];
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
float CTimeCycle::m_fBlurAlpha[NUMHOURS][NUMWEATHERS];

float CTimeCycle::m_fCurrentAmbientRed;
float CTimeCycle::m_fCurrentAmbientGreen;
float CTimeCycle::m_fCurrentAmbientBlue;
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
int   CTimeCycle::m_nCurrentTreeShadowStrength;
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
float CTimeCycle::m_fCurrentBlurAlpha;
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
	int dirR, dirG, dirB;
	int skyTopR, skyTopG, skyTopB;
	int skyBotR, skyBotG, skyBotB;
	int sunCoreR, sunCoreG, sunCoreB;
	int sunCoronaR, sunCoronaG, sunCoronaB;
	float sunSz, sprSz, sprBght;
	int shad, lightShad, treeShad;
	float farClp, fogSt, lightGnd;
	int cloudR, cloudG, cloudB;
	int fluffyTopR, fluffyTopG, fluffyTopB;
	int fluffyBotR, fluffyBotG, fluffyBotB;
	float blurR, blurG, blurB, blurA;

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
			while(work_buff[bi] != '\n')
				line[li++] = work_buff[bi++];
			line[li] = '\0';
			bi++;

			sscanf(line, "%d %d %d  %d %d %d  %d %d %d  %d %d %d "
			             "%d %d %d  %d %d %d  %f %f %f %d %d %d %f %f %f "
			             "%d %d %d  %d %d %d  %d %d %d  %f %f %f %f",
				&ambR, &ambG, &ambB,
				 &dirR, &dirG, &dirB,
				 &skyTopR, &skyTopG, &skyTopB,
				 &skyBotR, &skyBotG, &skyBotB,
				&sunCoreR, &sunCoreG, &sunCoreB,
				 &sunCoronaR, &sunCoronaG, &sunCoronaB,
				 &sunSz, &sprSz, &sprBght,
				 &shad, &lightShad, &treeShad,
				 &farClp, &fogSt, &lightGnd,
				&cloudR, &cloudG, &cloudB,
				 &fluffyTopR, &fluffyTopG, &fluffyTopB,
				 &fluffyBotR, &fluffyBotG, &fluffyBotB,
				 &blurR, &blurG, &blurB, &blurA);

			m_nAmbientRed[h][w] = ambR;
			m_nAmbientGreen[h][w] = ambG;
			m_nAmbientBlue[h][w] = ambB;
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
			m_nTreeShadowStrength[h][w] = treeShad;
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
			m_fBlurAlpha[h][w] = blurA;
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
	float timeInterp = CClock::GetMinutes()/60.0f;
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
	m_fCurrentAmbientRed /= 255.0f;
	m_fCurrentAmbientGreen /= 255.0f;
	m_fCurrentAmbientBlue /= 255.0f;

	m_fCurrentDirectionalRed = INTERP(m_nDirectionalRed);
	m_fCurrentDirectionalGreen = INTERP(m_nDirectionalGreen);
	m_fCurrentDirectionalBlue = INTERP(m_nDirectionalBlue);
	m_fCurrentDirectionalRed /= 255.0f;
	m_fCurrentDirectionalGreen /= 255.0f;
	m_fCurrentDirectionalBlue /= 255.0f;

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
	m_nCurrentTreeShadowStrength = INTERP(m_nTreeShadowStrength);
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
	m_fCurrentBlurAlpha = INTERP(m_fBlurAlpha);

	if(TheCamera.m_BlurType == MBLUR_NONE || TheCamera.m_BlurType == MBLUR_NORMAL)
		TheCamera.SetMotionBlur(m_fCurrentBlurRed, m_fCurrentBlurGreen, m_fCurrentBlurBlue, m_fCurrentBlurAlpha, MBLUR_NORMAL);

	if(m_FogReduction != 0)
		m_fCurrentFarClip = max(m_fCurrentFarClip, m_FogReduction/64.0f * 650.0f);
	m_nCurrentFogColourRed = (m_nCurrentSkyTopRed + 2*m_nCurrentSkyBottomRed) / 3;
	m_nCurrentFogColourGreen = (m_nCurrentSkyTopGreen + 2*m_nCurrentSkyBottomGreen) / 3;
	m_nCurrentFogColourBlue = (m_nCurrentSkyTopBlue + 2*m_nCurrentSkyBottomBlue) / 3;

	m_CurrentStoredValue = (m_CurrentStoredValue+1)&0xF;

	float sunAngle = 2*PI*(CClock::GetMinutes() + CClock::GetHours()*60)/(24*60);
	CVector &sunPos = GetSunPosition();
	sunPos.x = Sin(sunAngle);
	sunPos.y = 1.0f;
	sunPos.z = 0.2f - Cos(sunAngle);
	sunPos.Normalise();

	CShadows::CalcPedShadowValues(sunPos, 
		&m_fShadowFrontX[m_CurrentStoredValue], &m_fShadowFrontY[m_CurrentStoredValue],
		&m_fShadowSideX[m_CurrentStoredValue], &m_fShadowSideY[m_CurrentStoredValue],
		&m_fShadowDisplacementX[m_CurrentStoredValue], &m_fShadowDisplacementY[m_CurrentStoredValue]);

	if(TheCamera.GetForward().z < -0.9f ||
	   !CWeather::bScriptsForceRain && (CCullZones::PlayerNoRain() || CCullZones::CamNoRain() || CCutsceneMgr::IsRunning()))
		m_FogReduction = min(m_FogReduction+1, 64);
	else
		m_FogReduction = max(m_FogReduction-1, 0);
}

#ifdef WEATHER_EDIT
#define NUMHOURS 24
#define NUMWEATHERS 4

static const char *header = "// Amb     Dir          Sky top			Sky bot		SunCore 		SunCorona   SunSz  SprSz	SprBght Shdw LightShd  TreeShd FarClp  FogSt	LightOnGround LowCloudsRGB TopCloudRGB BottomCloudRGB		BlurRGBA\n";
static const char *weatherNames[] = {"SUNNY", "CLOUDY", "RAINY", "FOGGY"};
static const char *timeNames[] = {"Midnight", "1AM", "2AM", "3AM", "4AM", "5AM", "6AM", "7AM", "8AM", "9AM", "10AM", "11AM", "Midday",
						   "1PM", "2PM", "3PM", "4PM", "5PM", "6PM", "7PM", "8PM", "9PM", "10PM", "11PM"};

bool CTimeCycle::Save()
{
	FILE *f;
	CFileMgr::SetDir("");
	if (f = fopen("DATA\\timecyc.dat", "w"), f == nil)
		return false;
	for (int w = 0; w < NUMWEATHERS; w++)
	{
		fprintf(f, "//\n/////////////////////////////////////////// %s\n//\n", weatherNames[w]);
		for (int h = 0; h < 24; h++)
		{
			if ((h % 12) == 0)
				fprintf(f, header);
			fprintf(f, "// %s\n", timeNames[h]);
			fprintf(f,
					"%d %d %d\t%d %d %d\t%d %d %d\t%d %d %d\t"
					"%d %d %d\t%d %d %d\t%.1f\t%.1f\t%.1f\t%d\t%d\t%d\t%.1f\t%.1f\t%.1f\t"
					"%d %d %d\t%d %d %d\t%d %d %d\t%d %d %d %d\n",
					CTimeCycle::m_nAmbientRed[h][w],
					CTimeCycle::m_nAmbientGreen[h][w],
					CTimeCycle::m_nAmbientBlue[h][w],
					CTimeCycle::m_nDirectionalRed[h][w],
					CTimeCycle::m_nDirectionalGreen[h][w],
					CTimeCycle::m_nDirectionalBlue[h][w],

					CTimeCycle::m_nSkyTopRed[h][w],
					CTimeCycle::m_nSkyTopGreen[h][w],
					CTimeCycle::m_nSkyTopBlue[h][w],
					CTimeCycle::m_nSkyBottomRed[h][w],
					CTimeCycle::m_nSkyBottomGreen[h][w],
					CTimeCycle::m_nSkyBottomBlue[h][w],
					CTimeCycle::m_nSunCoreRed[h][w],
					CTimeCycle::m_nSunCoreGreen[h][w],
					CTimeCycle::m_nSunCoreBlue[h][w],
					CTimeCycle::m_nSunCoronaRed[h][w],
					CTimeCycle::m_nSunCoronaGreen[h][w],
					CTimeCycle::m_nSunCoronaBlue[h][w],
					CTimeCycle::m_fSunSize[h][w],
					CTimeCycle::m_fSpriteSize[h][w],
					CTimeCycle::m_fSpriteBrightness[h][w],
					CTimeCycle::m_nShadowStrength[h][w],
					CTimeCycle::m_nLightShadowStrength[h][w],
					CTimeCycle::m_nTreeShadowStrength[h][w],
					CTimeCycle::m_fFarClip[h][w],
					CTimeCycle::m_fFogStart[h][w],
					CTimeCycle::m_fLightsOnGroundBrightness[h][w],
					CTimeCycle::m_nLowCloudsRed[h][w],
					CTimeCycle::m_nLowCloudsGreen[h][w],
					CTimeCycle::m_nLowCloudsBlue[h][w],
					CTimeCycle::m_nFluffyCloudsTopRed[h][w],
					CTimeCycle::m_nFluffyCloudsTopGreen[h][w],
					CTimeCycle::m_nFluffyCloudsTopBlue[h][w],
					CTimeCycle::m_nFluffyCloudsBottomRed[h][w],
					CTimeCycle::m_nFluffyCloudsBottomGreen[h][w],
					CTimeCycle::m_nFluffyCloudsBottomBlue[h][w],
					(int)CTimeCycle::m_fBlurRed[h][w],
					(int)CTimeCycle::m_fBlurGreen[h][w],
					(int)CTimeCycle::m_fBlurBlue[h][w],
					(int)CTimeCycle::m_fBlurAlpha[h][w]);
		}
	}
	fclose(f);
	return true;
}
#endif
