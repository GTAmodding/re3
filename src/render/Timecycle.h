#pragma once

class CTimeCycle
{
	static int   m_nAmbientRed[NUMHOURS][NUMWEATHERS];
	static int   m_nAmbientGreen[NUMHOURS][NUMWEATHERS];
	static int   m_nAmbientBlue[NUMHOURS][NUMWEATHERS];
	static int   m_nAmbientRed_Obj[NUMHOURS][NUMWEATHERS];
	static int   m_nAmbientGreen_Obj[NUMHOURS][NUMWEATHERS];
	static int   m_nAmbientBlue_Obj[NUMHOURS][NUMWEATHERS];
	static int   m_nAmbientRed_Bl[NUMHOURS][NUMWEATHERS];
	static int   m_nAmbientGreen_Bl[NUMHOURS][NUMWEATHERS];
	static int   m_nAmbientBlue_Bl[NUMHOURS][NUMWEATHERS];
	static int   m_nAmbientRed_Obj_Bl[NUMHOURS][NUMWEATHERS];
	static int   m_nAmbientGreen_Obj_Bl[NUMHOURS][NUMWEATHERS];
	static int   m_nAmbientBlue_Obj_Bl[NUMHOURS][NUMWEATHERS];
	static int   m_nDirectionalRed[NUMHOURS][NUMWEATHERS];
	static int   m_nDirectionalGreen[NUMHOURS][NUMWEATHERS];
	static int   m_nDirectionalBlue[NUMHOURS][NUMWEATHERS];
	static int   m_nSkyTopRed[NUMHOURS][NUMWEATHERS];
	static int   m_nSkyTopGreen[NUMHOURS][NUMWEATHERS];
	static int   m_nSkyTopBlue[NUMHOURS][NUMWEATHERS];
	static int   m_nSkyBottomRed[NUMHOURS][NUMWEATHERS];
	static int   m_nSkyBottomGreen[NUMHOURS][NUMWEATHERS];
	static int   m_nSkyBottomBlue[NUMHOURS][NUMWEATHERS];
	static int   m_nSunCoreRed[NUMHOURS][NUMWEATHERS];
	static int   m_nSunCoreGreen[NUMHOURS][NUMWEATHERS];
	static int   m_nSunCoreBlue[NUMHOURS][NUMWEATHERS];
	static int   m_nSunCoronaRed[NUMHOURS][NUMWEATHERS];
	static int   m_nSunCoronaGreen[NUMHOURS][NUMWEATHERS];
	static int   m_nSunCoronaBlue[NUMHOURS][NUMWEATHERS];
	static float m_fSunSize[NUMHOURS][NUMWEATHERS];
	static float m_fSpriteSize[NUMHOURS][NUMWEATHERS];
	static float m_fSpriteBrightness[NUMHOURS][NUMWEATHERS];
	static short m_nShadowStrength[NUMHOURS][NUMWEATHERS];
	static short m_nLightShadowStrength[NUMHOURS][NUMWEATHERS];
	static short m_nPoleShadowStrength[NUMHOURS][NUMWEATHERS];
	static float m_fFogStart[NUMHOURS][NUMWEATHERS];
	static float m_fFarClip[NUMHOURS][NUMWEATHERS];
	static float m_fLightsOnGroundBrightness[NUMHOURS][NUMWEATHERS];
	static int   m_nLowCloudsRed[NUMHOURS][NUMWEATHERS];
	static int   m_nLowCloudsGreen[NUMHOURS][NUMWEATHERS];
	static int   m_nLowCloudsBlue[NUMHOURS][NUMWEATHERS];
	static int   m_nFluffyCloudsTopRed[NUMHOURS][NUMWEATHERS];
	static int   m_nFluffyCloudsTopGreen[NUMHOURS][NUMWEATHERS];
	static int   m_nFluffyCloudsTopBlue[NUMHOURS][NUMWEATHERS];
	static int   m_nFluffyCloudsBottomRed[NUMHOURS][NUMWEATHERS];
	static int   m_nFluffyCloudsBottomGreen[NUMHOURS][NUMWEATHERS];
	static int   m_nFluffyCloudsBottomBlue[NUMHOURS][NUMWEATHERS];
	static float m_fBlurRed[NUMHOURS][NUMWEATHERS];
	static float m_fBlurGreen[NUMHOURS][NUMWEATHERS];
	static float m_fBlurBlue[NUMHOURS][NUMWEATHERS];
	static float m_fWaterRed[NUMHOURS][NUMWEATHERS];
	static float m_fWaterGreen[NUMHOURS][NUMWEATHERS];
	static float m_fWaterBlue[NUMHOURS][NUMWEATHERS];
	static float m_fWaterAlpha[NUMHOURS][NUMWEATHERS];

	static float m_fCurrentAmbientRed;
	static float m_fCurrentAmbientGreen;
	static float m_fCurrentAmbientBlue;
	static float m_fCurrentAmbientRed_Obj;
	static float m_fCurrentAmbientGreen_Obj;
	static float m_fCurrentAmbientBlue_Obj;
	static float m_fCurrentAmbientRed_Bl;
	static float m_fCurrentAmbientGreen_Bl;
	static float m_fCurrentAmbientBlue_Bl;
	static float m_fCurrentAmbientRed_Obj_Bl;
	static float m_fCurrentAmbientGreen_Obj_Bl;
	static float m_fCurrentAmbientBlue_Obj_Bl;
	static float m_fCurrentDirectionalRed;
	static float m_fCurrentDirectionalGreen;
	static float m_fCurrentDirectionalBlue;
	static int   m_nCurrentSkyTopRed;
	static int   m_nCurrentSkyTopGreen;
	static int   m_nCurrentSkyTopBlue;
	static int   m_nCurrentSkyBottomRed;
	static int   m_nCurrentSkyBottomGreen;
	static int   m_nCurrentSkyBottomBlue;
	static int   m_nCurrentSunCoreRed;
	static int   m_nCurrentSunCoreGreen;
	static int   m_nCurrentSunCoreBlue;
	static int   m_nCurrentSunCoronaRed;
	static int   m_nCurrentSunCoronaGreen;
	static int   m_nCurrentSunCoronaBlue;
	static float m_fCurrentSunSize;
	static float m_fCurrentSpriteSize;
	static float m_fCurrentSpriteBrightness;
	static int   m_nCurrentShadowStrength;
	static int   m_nCurrentLightShadowStrength;
	static int   m_nCurrentPoleShadowStrength;
	static float m_fCurrentFogStart;
	static float m_fCurrentFarClip;
	static float m_fCurrentLightsOnGroundBrightness;
	static int   m_nCurrentLowCloudsRed;
	static int   m_nCurrentLowCloudsGreen;
	static int   m_nCurrentLowCloudsBlue;
	static int   m_nCurrentFluffyCloudsTopRed;
	static int   m_nCurrentFluffyCloudsTopGreen;
	static int   m_nCurrentFluffyCloudsTopBlue;
	static int   m_nCurrentFluffyCloudsBottomRed;
	static int   m_nCurrentFluffyCloudsBottomGreen;
	static int   m_nCurrentFluffyCloudsBottomBlue;
	static float m_fCurrentBlurRed;
	static float m_fCurrentBlurGreen;
	static float m_fCurrentBlurBlue;
	static float m_fCurrentWaterRed;
	static float m_fCurrentWaterGreen;
	static float m_fCurrentWaterBlue;
	static float m_fCurrentWaterAlpha;
	static int   m_nCurrentFogColourRed;
	static int   m_nCurrentFogColourGreen;
	static int   m_nCurrentFogColourBlue;

	static int   m_FogReduction;

public:
	static int   m_CurrentStoredValue;
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
	static float GetAmbientRed_Obj(void) { return m_fCurrentAmbientRed_Obj; }
	static float GetAmbientGreen_Obj(void) { return m_fCurrentAmbientGreen_Obj; }
	static float GetAmbientBlue_Obj(void) { return m_fCurrentAmbientBlue_Obj; }
	static float GetAmbientRed_Bl(void) { return m_fCurrentAmbientRed_Bl; }
	static float GetAmbientGreen_Bl(void) { return m_fCurrentAmbientGreen_Bl; }
	static float GetAmbientBlue_Bl(void) { return m_fCurrentAmbientBlue_Bl; }
	static float GetAmbientRed_Obj_Bl(void) { return m_fCurrentAmbientRed_Obj_Bl; }
	static float GetAmbientGreen_Obj_Bl(void) { return m_fCurrentAmbientGreen_Obj_Bl; }
	static float GetAmbientBlue_Obj_Bl(void) { return m_fCurrentAmbientBlue_Obj_Bl; }
	static float GetDirectionalRed(void) { return m_fCurrentDirectionalRed; }
	static float GetDirectionalGreen(void) { return m_fCurrentDirectionalGreen; }
	static float GetDirectionalBlue(void) { return m_fCurrentDirectionalBlue; }
	static int GetSkyTopRed(void) { return m_nCurrentSkyTopRed; }
	static int GetSkyTopGreen(void) { return m_nCurrentSkyTopGreen; }
	static int GetSkyTopBlue(void) { return m_nCurrentSkyTopBlue; }
	static int GetSkyBottomRed(void) { return m_nCurrentSkyBottomRed; }
	static int GetSkyBottomGreen(void) { return m_nCurrentSkyBottomGreen; }
	static int GetSkyBottomBlue(void) { return m_nCurrentSkyBottomBlue; }
	static int GetSunCoreRed(void) { return m_nCurrentSunCoreRed; }
	static int GetSunCoreGreen(void) { return m_nCurrentSunCoreGreen; }
	static int GetSunCoreBlue(void) { return m_nCurrentSunCoreBlue; }
	static int GetSunCoronaRed(void) { return m_nCurrentSunCoronaRed; }
	static int GetSunCoronaGreen(void) { return m_nCurrentSunCoronaGreen; }
	static int GetSunCoronaBlue(void) { return m_nCurrentSunCoronaBlue; }
	static float GetSunSize(void) { return m_fCurrentSunSize; }
	static float GetSpriteBrightness(void) { return m_fCurrentSpriteBrightness; }
	static float GetSpriteSize(void) { return m_fCurrentSpriteSize; }
	static int GetShadowStrength(void) { return m_nCurrentShadowStrength; }
	static int GetLightShadowStrength(void) { return m_nCurrentLightShadowStrength; }
	static float GetLightOnGroundBrightness(void) { return m_fCurrentLightsOnGroundBrightness; }
	static float GetFarClip(void) { return m_fCurrentFarClip; }
	static float GetFogStart(void) { return m_fCurrentFogStart; }

	static int GetLowCloudsRed(void) { return m_nCurrentLowCloudsRed; }
	static int GetLowCloudsGreen(void) { return m_nCurrentLowCloudsGreen; }
	static int GetLowCloudsBlue(void) { return m_nCurrentLowCloudsBlue; }
	static int GetFluffyCloudsTopRed(void) { return m_nCurrentFluffyCloudsTopRed; }
	static int GetFluffyCloudsTopGreen(void) { return m_nCurrentFluffyCloudsTopGreen; }
	static int GetFluffyCloudsTopBlue(void) { return m_nCurrentFluffyCloudsTopBlue; }
	static int GetFluffyCloudsBottomRed(void) { return m_nCurrentFluffyCloudsBottomRed; }
	static int GetFluffyCloudsBottomGreen(void) { return m_nCurrentFluffyCloudsBottomGreen; }
	static int GetFluffyCloudsBottomBlue(void) { return m_nCurrentFluffyCloudsBottomBlue; }
	static int GetFogRed(void) { return m_nCurrentFogColourRed; }
	static int GetFogGreen(void) { return m_nCurrentFogColourGreen; }
	static int GetFogBlue(void) { return m_nCurrentFogColourBlue; }
	static int GetFogReduction(void) { return m_FogReduction; }

	static int GetWaterRed(void) { return m_fCurrentWaterRed; }
	static int GetWaterGreen(void) { return m_fCurrentWaterGreen; }
	static int GetWaterBlue(void) { return m_fCurrentWaterBlue; }
	static int GetWaterAlpha(void) { return m_fCurrentWaterAlpha; }

	static void Initialise(void);
	static void Update(void);
	static CVector &GetSunPosition(void) { return m_VectorToSun[m_CurrentStoredValue]; }
	static float GetShadowFrontX(void) { return m_fShadowFrontX[m_CurrentStoredValue]; }
	static float GetShadowFrontY(void) { return m_fShadowFrontY[m_CurrentStoredValue]; }
	static float GetShadowSideX(void) { return m_fShadowSideX[m_CurrentStoredValue]; }
	static float GetShadowSideY(void) { return m_fShadowSideY[m_CurrentStoredValue]; }
	static float GetShadowDisplacementX(void) { return m_fShadowDisplacementX[m_CurrentStoredValue]; }
	static float GetShadowDisplacementY(void) { return m_fShadowDisplacementY[m_CurrentStoredValue]; }
};
