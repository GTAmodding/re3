#pragma once

class CTimeCycle
{
	static int32 m_nAmbientRed[NUMHOURS][NUMWEATHERS];
	static int32 m_nAmbientGreen[NUMHOURS][NUMWEATHERS];
	static int32 m_nAmbientBlue[NUMHOURS][NUMWEATHERS];
	static int32 m_nDirectionalRed[NUMHOURS][NUMWEATHERS];
	static int32 m_nDirectionalGreen[NUMHOURS][NUMWEATHERS];
	static int32 m_nDirectionalBlue[NUMHOURS][NUMWEATHERS];
	static int32 m_nSkyTopRed[NUMHOURS][NUMWEATHERS];
	static int32 m_nSkyTopGreen[NUMHOURS][NUMWEATHERS];
	static int32 m_nSkyTopBlue[NUMHOURS][NUMWEATHERS];
	static int32 m_nSkyBottomRed[NUMHOURS][NUMWEATHERS];
	static int32 m_nSkyBottomGreen[NUMHOURS][NUMWEATHERS];
	static int32 m_nSkyBottomBlue[NUMHOURS][NUMWEATHERS];
	static int32 m_nSunCoreRed[NUMHOURS][NUMWEATHERS];
	static int32 m_nSunCoreGreen[NUMHOURS][NUMWEATHERS];
	static int32 m_nSunCoreBlue[NUMHOURS][NUMWEATHERS];
	static int32 m_nSunCoronaRed[NUMHOURS][NUMWEATHERS];
	static int32 m_nSunCoronaGreen[NUMHOURS][NUMWEATHERS];
	static int32 m_nSunCoronaBlue[NUMHOURS][NUMWEATHERS];
	static float m_fSunSize[NUMHOURS][NUMWEATHERS];
	static float m_fSpriteSize[NUMHOURS][NUMWEATHERS];
	static float m_fSpriteBrightness[NUMHOURS][NUMWEATHERS];
	static int16 m_nShadowStrength[NUMHOURS][NUMWEATHERS];
	static int16 m_nLightShadowStrength[NUMHOURS][NUMWEATHERS];
	static int16 m_nTreeShadowStrength[NUMHOURS][NUMWEATHERS];
	static float m_fFogStart[NUMHOURS][NUMWEATHERS];
	static float m_fFarClip[NUMHOURS][NUMWEATHERS];
	static float m_fLightsOnGroundBrightness[NUMHOURS][NUMWEATHERS];
	static int32 m_nLowCloudsRed[NUMHOURS][NUMWEATHERS];
	static int32 m_nLowCloudsGreen[NUMHOURS][NUMWEATHERS];
	static int32 m_nLowCloudsBlue[NUMHOURS][NUMWEATHERS];
	static int32 m_nFluffyCloudsTopRed[NUMHOURS][NUMWEATHERS];
	static int32 m_nFluffyCloudsTopGreen[NUMHOURS][NUMWEATHERS];
	static int32 m_nFluffyCloudsTopBlue[NUMHOURS][NUMWEATHERS];
	static int32 m_nFluffyCloudsBottomRed[NUMHOURS][NUMWEATHERS];
	static int32 m_nFluffyCloudsBottomGreen[NUMHOURS][NUMWEATHERS];
	static int32 m_nFluffyCloudsBottomBlue[NUMHOURS][NUMWEATHERS];
	static float m_fBlurRed[NUMHOURS][NUMWEATHERS];
	static float m_fBlurGreen[NUMHOURS][NUMWEATHERS];
	static float m_fBlurBlue[NUMHOURS][NUMWEATHERS];
	static float m_fBlurAlpha[NUMHOURS][NUMWEATHERS];

	static float m_fCurrentAmbientRed;
	static float m_fCurrentAmbientGreen;
	static float m_fCurrentAmbientBlue;
	static float m_fCurrentDirectionalRed;
	static float m_fCurrentDirectionalGreen;
	static float m_fCurrentDirectionalBlue;
	static int32 m_nCurrentSkyTopRed;
	static int32 m_nCurrentSkyTopGreen;
	static int32 m_nCurrentSkyTopBlue;
	static int32 m_nCurrentSkyBottomRed;
	static int32 m_nCurrentSkyBottomGreen;
	static int32 m_nCurrentSkyBottomBlue;
	static int32 m_nCurrentSunCoreRed;
	static int32 m_nCurrentSunCoreGreen;
	static int32 m_nCurrentSunCoreBlue;
	static int32 m_nCurrentSunCoronaRed;
	static int32 m_nCurrentSunCoronaGreen;
	static int32 m_nCurrentSunCoronaBlue;
	static float m_fCurrentSunSize;
	static float m_fCurrentSpriteSize;
	static float m_fCurrentSpriteBrightness;
	static int32 m_nCurrentShadowStrength;
	static int32 m_nCurrentLightShadowStrength;
	static int32 m_nCurrentTreeShadowStrength;
	static float m_fCurrentFogStart;
	static float m_fCurrentFarClip;
	static float m_fCurrentLightsOnGroundBrightness;
	static int32 m_nCurrentLowCloudsRed;
	static int32 m_nCurrentLowCloudsGreen;
	static int32 m_nCurrentLowCloudsBlue;
	static int32 m_nCurrentFluffyCloudsTopRed;
	static int32 m_nCurrentFluffyCloudsTopGreen;
	static int32 m_nCurrentFluffyCloudsTopBlue;
	static int32 m_nCurrentFluffyCloudsBottomRed;
	static int32 m_nCurrentFluffyCloudsBottomGreen;
	static int32 m_nCurrentFluffyCloudsBottomBlue;
	static float m_fCurrentBlurRed;
	static float m_fCurrentBlurGreen;
	static float m_fCurrentBlurBlue;
	static float m_fCurrentBlurAlpha;
	static int32 m_nCurrentFogColourRed;
	static int32 m_nCurrentFogColourGreen;
	static int32 m_nCurrentFogColourBlue;

	static int32 m_FogReduction;

public:
	static int32 m_CurrentStoredValue;
	static CVector m_VectorToSun[16];
	static float m_fShadowFrontX[16];
	static float m_fShadowFrontY[16];
	static float m_fShadowSideX[16];
	static float m_fShadowSideY[16];
	static float m_fShadowDisplacementX[16];
	static float m_fShadowDisplacementY[16];

	static float GetAmbientRed(void) { return m_fCurrentAmbientRed; }
	static float GetAmbientGreen(void) { return m_fCurrentAmbientGreen; }
	static float GetAmbientBlue(void) { return m_fCurrentAmbientBlue; }
	static float GetDirectionalRed(void) { return m_fCurrentDirectionalRed; }
	static float GetDirectionalGreen(void) { return m_fCurrentDirectionalGreen; }
	static float GetDirectionalBlue(void) { return m_fCurrentDirectionalBlue; }
	static int32 GetSkyTopRed(void) { return m_nCurrentSkyTopRed; }
	static int32 GetSkyTopGreen(void) { return m_nCurrentSkyTopGreen; }
	static int32 GetSkyTopBlue(void) { return m_nCurrentSkyTopBlue; }
	static int32 GetSkyBottomRed(void) { return m_nCurrentSkyBottomRed; }
	static int32 GetSkyBottomGreen(void) { return m_nCurrentSkyBottomGreen; }
	static int32 GetSkyBottomBlue(void) { return m_nCurrentSkyBottomBlue; }
	static int32 GetSunCoreRed(void) { return m_nCurrentSunCoreRed; }
	static int32 GetSunCoreGreen(void) { return m_nCurrentSunCoreGreen; }
	static int32 GetSunCoreBlue(void) { return m_nCurrentSunCoreBlue; }
	static int32 GetSunCoronaRed(void) { return m_nCurrentSunCoronaRed; }
	static int32 GetSunCoronaGreen(void) { return m_nCurrentSunCoronaGreen; }
	static int32 GetSunCoronaBlue(void) { return m_nCurrentSunCoronaBlue; }
	static float GetSunSize(void) { return m_fCurrentSunSize; }
	static float GetSpriteBrightness(void) { return m_fCurrentSpriteBrightness; }
	static float GetSpriteSize(void) { return m_fCurrentSpriteSize; }
	static int32 GetShadowStrength(void) { return m_nCurrentShadowStrength; }
	static int32 GetLightShadowStrength(void) { return m_nCurrentLightShadowStrength; }
	static float GetLightOnGroundBrightness(void) { return m_fCurrentLightsOnGroundBrightness; }
	static float GetFarClip(void) { return m_fCurrentFarClip; }
	static float GetFogStart(void) { return m_fCurrentFogStart; }

	static int32 GetLowCloudsRed(void) { return m_nCurrentLowCloudsRed; }
	static int32 GetLowCloudsGreen(void) { return m_nCurrentLowCloudsGreen; }
	static int32 GetLowCloudsBlue(void) { return m_nCurrentLowCloudsBlue; }
	static int32 GetFluffyCloudsTopRed(void) { return m_nCurrentFluffyCloudsTopRed; }
	static int32 GetFluffyCloudsTopGreen(void) { return m_nCurrentFluffyCloudsTopGreen; }
	static int32 GetFluffyCloudsTopBlue(void) { return m_nCurrentFluffyCloudsTopBlue; }
	static int32 GetFluffyCloudsBottomRed(void) { return m_nCurrentFluffyCloudsBottomRed; }
	static int32 GetFluffyCloudsBottomGreen(void) { return m_nCurrentFluffyCloudsBottomGreen; }
	static int32 GetFluffyCloudsBottomBlue(void) { return m_nCurrentFluffyCloudsBottomBlue; }
	static int32 GetFogRed(void) { return m_nCurrentFogColourRed; }
	static int32 GetFogGreen(void) { return m_nCurrentFogColourGreen; }
	static int32 GetFogBlue(void) { return m_nCurrentFogColourBlue; }
	static int32 GetFogReduction(void) { return m_FogReduction; }

	static void Initialise(void);
	static void Update(void);
	static CVector &GetSunDirection(void) { return m_VectorToSun[m_CurrentStoredValue]; }
	static float GetShadowFrontX(void) { return m_fShadowFrontX[m_CurrentStoredValue]; }
	static float GetShadowFrontY(void) { return m_fShadowFrontY[m_CurrentStoredValue]; }
	static float GetShadowSideX(void) { return m_fShadowSideX[m_CurrentStoredValue]; }
	static float GetShadowSideY(void) { return m_fShadowSideY[m_CurrentStoredValue]; }
	static float GetShadowDisplacementX(void) { return m_fShadowDisplacementX[m_CurrentStoredValue]; }
	static float GetShadowDisplacementY(void) { return m_fShadowDisplacementY[m_CurrentStoredValue]; }
};
