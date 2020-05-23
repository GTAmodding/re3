#include "common.h"
#include <rwcore.h>
#include <rpworld.h>

#include "Lights.h"
#include "Timecycle.h"
#include "Coronas.h"
#include "Weather.h"
#include "ZoneCull.h"
#include "Frontend.h"
#include "MBlur.h"

RpLight *pAmbient;
RpLight *pDirect;
RpLight *pExtraDirectionals[4] = { nil };
int LightStrengths[4];
int NumExtraDirLightsInWorld;

RwRGBAReal AmbientLightColourForFrame;
RwRGBAReal AmbientLightColourForFrame_PedsCarsAndObjects;
RwRGBAReal DirectionalLightColourForFrame;

RwRGBAReal AmbientLightColour;
RwRGBAReal DirectionalLightColour;

//--MIAMI: done
void
SetLightsWithTimeOfDayColour(RpWorld *)
{
	CVector vec1, vec2, vecsun;
	RwMatrix mat;

	if(pAmbient){
		if(CMBlur::BlurOn){
			AmbientLightColourForFrame.red = CTimeCycle::GetAmbientRed_Bl() * CCoronas::LightsMult;
			AmbientLightColourForFrame.green = CTimeCycle::GetAmbientGreen_Bl() * CCoronas::LightsMult;
			AmbientLightColourForFrame.blue = CTimeCycle::GetAmbientBlue_Bl() * CCoronas::LightsMult;
		}else{
			AmbientLightColourForFrame.red = CTimeCycle::GetAmbientRed() * CCoronas::LightsMult;
			AmbientLightColourForFrame.green = CTimeCycle::GetAmbientGreen() * CCoronas::LightsMult;
			AmbientLightColourForFrame.blue = CTimeCycle::GetAmbientBlue() * CCoronas::LightsMult;
		}

		if(CMBlur::BlurOn){
			AmbientLightColourForFrame_PedsCarsAndObjects.red = CTimeCycle::GetAmbientRed_Obj_Bl() * CCoronas::LightsMult;
			AmbientLightColourForFrame_PedsCarsAndObjects.green = CTimeCycle::GetAmbientGreen_Obj_Bl() * CCoronas::LightsMult;
			AmbientLightColourForFrame_PedsCarsAndObjects.blue = CTimeCycle::GetAmbientBlue_Obj_Bl() * CCoronas::LightsMult;
		}else{
			AmbientLightColourForFrame_PedsCarsAndObjects.red = CTimeCycle::GetAmbientRed_Obj() * CCoronas::LightsMult;
			AmbientLightColourForFrame_PedsCarsAndObjects.green = CTimeCycle::GetAmbientGreen_Obj() * CCoronas::LightsMult;
			AmbientLightColourForFrame_PedsCarsAndObjects.blue = CTimeCycle::GetAmbientBlue_Obj() * CCoronas::LightsMult;
		}

		if(CWeather::LightningFlash && !CCullZones::CamNoRain()){
			AmbientLightColourForFrame.red = 1.0f;
			AmbientLightColourForFrame.green = 1.0f;
			AmbientLightColourForFrame.blue = 1.0f;

			AmbientLightColourForFrame_PedsCarsAndObjects.red = 1.0f;
			AmbientLightColourForFrame_PedsCarsAndObjects.green = 1.0f;
			AmbientLightColourForFrame_PedsCarsAndObjects.blue = 1.0f;
		}
		RpLightSetColor(pAmbient, &AmbientLightColourForFrame);
	}

	if(pDirect){
		DirectionalLightColourForFrame.red = CTimeCycle::GetDirectionalRed() * CCoronas::LightsMult;
		DirectionalLightColourForFrame.green = CTimeCycle::GetDirectionalGreen() * CCoronas::LightsMult;
		DirectionalLightColourForFrame.blue = CTimeCycle::GetDirectionalBlue() * CCoronas::LightsMult;
		RpLightSetColor(pDirect, &DirectionalLightColourForFrame);

		vecsun = CTimeCycle::m_VectorToSun[CTimeCycle::m_CurrentStoredValue];
		vec1 = CVector(0.0f, 0.0f, 1.0f);
		vec2 = CrossProduct(vec1, vecsun);
		vec2.Normalise();
		vec1 = CrossProduct(vec2, vecsun);
		mat.at.x = -vecsun.x;
		mat.at.y = -vecsun.y;
		mat.at.z = -vecsun.z;
		mat.right.x = vec1.x;
		mat.right.y = vec1.y;
		mat.right.z = vec1.z;
		mat.up.x = vec2.x;
		mat.up.y = vec2.y;
		mat.up.z = vec2.z;
		RwFrameTransform(RpLightGetFrame(pDirect), &mat, rwCOMBINEREPLACE);
	}

	if(FrontEndMenuManager.m_PrefsBrightness > 256){
		float f1 = 2.0f * (FrontEndMenuManager.m_PrefsBrightness/256.0f - 1.0f) * 0.6f + 1.0f;
		float f2 = 3.0f * (FrontEndMenuManager.m_PrefsBrightness/256.0f - 1.0f) * 0.6f + 1.0f;

		AmbientLightColourForFrame.red = Min(1.0f, AmbientLightColourForFrame.red * f2);
		AmbientLightColourForFrame.green = Min(1.0f, AmbientLightColourForFrame.green * f2);
		AmbientLightColourForFrame.blue = Min(1.0f, AmbientLightColourForFrame.blue * f2);
		AmbientLightColourForFrame_PedsCarsAndObjects.red = Min(1.0f, AmbientLightColourForFrame_PedsCarsAndObjects.red * f1);
		AmbientLightColourForFrame_PedsCarsAndObjects.green = Min(1.0f, AmbientLightColourForFrame_PedsCarsAndObjects.green * f1);
		AmbientLightColourForFrame_PedsCarsAndObjects.blue = Min(1.0f, AmbientLightColourForFrame_PedsCarsAndObjects.blue * f1);
#ifdef FIX_BUGS
		DirectionalLightColourForFrame.red = Min(1.0f, DirectionalLightColourForFrame.red * f1);
		DirectionalLightColourForFrame.green = Min(1.0f, DirectionalLightColourForFrame.green * f1);
		DirectionalLightColourForFrame.blue = Min(1.0f, DirectionalLightColourForFrame.blue * f1);
#else
		DirectionalLightColourForFrame.red = Min(1.0f, AmbientLightColourForFrame.red * f1);
		DirectionalLightColourForFrame.green = Min(1.0f, AmbientLightColourForFrame.green * f1);
		DirectionalLightColourForFrame.blue = Min(1.0f, AmbientLightColourForFrame.blue * f1);
#endif
	}
}

RpWorld*
LightsCreate(RpWorld *world)
{
	int i;
	RwRGBAReal color;
	RwFrame *frame;

	if(world == nil)
		return nil;

	pAmbient = RpLightCreate(rpLIGHTAMBIENT);
	RpLightSetFlags(pAmbient, rpLIGHTLIGHTATOMICS);
	color.red = 0.25f;
	color.green = 0.25f;
	color.blue = 0.2f;
	RpLightSetColor(pAmbient, &color);

	pDirect = RpLightCreate(rpLIGHTDIRECTIONAL);
	RpLightSetFlags(pDirect, rpLIGHTLIGHTATOMICS);
	color.red = 1.0f;
	color.green = 0.85f;
	color.blue = 0.45f;
	RpLightSetColor(pDirect, &color);
	RpLightSetRadius(pDirect, 2.0f);
	frame = RwFrameCreate();
	RpLightSetFrame(pDirect, frame);
	RwV3d axis = { 1.0f, 1.0f, 0.0f };
	RwFrameRotate(frame, &axis, 160.0f, rwCOMBINEPRECONCAT);

	RpWorldAddLight(world, pAmbient);
	RpWorldAddLight(world, pDirect);

	for(i = 0; i < NUMEXTRADIRECTIONALS; i++){
		pExtraDirectionals[i] = RpLightCreate(rpLIGHTDIRECTIONAL);
		RpLightSetFlags(pExtraDirectionals[i], 0);
		color.red = 1.0f;
		color.green = 0.5f;
		color.blue = 0.0f;
		RpLightSetColor(pExtraDirectionals[i], &color);
		RpLightSetRadius(pExtraDirectionals[i], 2.0f);
		frame = RwFrameCreate();
		RpLightSetFrame(pExtraDirectionals[i], frame);
		RpWorldAddLight(world, pExtraDirectionals[i]);
	}

	return world;
}

void
LightsDestroy(RpWorld *world)
{
	int i;

	if(world == nil)
		return;

	if(pAmbient){
		RpWorldRemoveLight(world, pAmbient);
		RpLightDestroy(pAmbient);
		pAmbient = nil;
	}

	if(pDirect){
		RpWorldRemoveLight(world, pDirect);
		RwFrameDestroy(RpLightGetFrame(pDirect));
		RpLightDestroy(pDirect);
		pDirect = nil;
	}

	for(i = 0; i < NUMEXTRADIRECTIONALS; i++)
		if(pExtraDirectionals[i]){
			RpWorldRemoveLight(world, pExtraDirectionals[i]);
			RwFrameDestroy(RpLightGetFrame(pExtraDirectionals[i]));
			RpLightDestroy(pExtraDirectionals[i]);
			pExtraDirectionals[i] = nil;
		}
}

void
WorldReplaceNormalLightsWithScorched(RpWorld *world, float l)
{
	RwRGBAReal color;
	color.red = l;
	color.green = l;
	color.blue = l;
	RpLightSetColor(pAmbient, &color);
	RpLightSetFlags(pDirect, 0);
}

void
WorldReplaceScorchedLightsWithNormal(RpWorld *world)
{
	RpLightSetColor(pAmbient, &AmbientLightColourForFrame);
	RpLightSetFlags(pDirect, rpLIGHTLIGHTATOMICS);
}

void
AddAnExtraDirectionalLight(RpWorld *world, float dirx, float diry, float dirz, float red, float green, float blue)
{
	float strength;
	int weakest;
	int i, n;
	RwRGBAReal color;
	RwV3d *dir;

	strength = Max(Max(red, green), blue);
	n = -1;
	if(NumExtraDirLightsInWorld < NUMEXTRADIRECTIONALS)
		n = NumExtraDirLightsInWorld;
	else{
		weakest = strength;
		for(i = 0; i < NUMEXTRADIRECTIONALS; i++)
			if(LightStrengths[i] < weakest){
				weakest = LightStrengths[i];
				n = i;
			}
	}

	if(n < 0)
		return;

	color.red = red;
	color.green = green;
	color.blue = blue;
	RpLightSetColor(pExtraDirectionals[n], &color);
	dir = RwMatrixGetAt(RwFrameGetMatrix(RpLightGetFrame(pExtraDirectionals[n])));
	dir->x = -dirx;
	dir->y = -diry;
	dir->z = -dirz;
	RwMatrixUpdate(RwFrameGetMatrix(RpLightGetFrame(pExtraDirectionals[n])));
	RwFrameUpdateObjects(RpLightGetFrame(pExtraDirectionals[n]));
	RpLightSetFlags(pExtraDirectionals[n], rpLIGHTLIGHTATOMICS);
	LightStrengths[n] = strength;
	NumExtraDirLightsInWorld = Min(NumExtraDirLightsInWorld+1, NUMEXTRADIRECTIONALS);
}

void
RemoveExtraDirectionalLights(RpWorld *world)
{
	int i;
	for(i = 0; i < NumExtraDirLightsInWorld; i++)
		RpLightSetFlags(pExtraDirectionals[i], 0);
	NumExtraDirLightsInWorld = 0;
}

void
SetAmbientAndDirectionalColours(float f)
{
	AmbientLightColour.red = AmbientLightColourForFrame.red * f;
	AmbientLightColour.green = AmbientLightColourForFrame.green * f;
	AmbientLightColour.blue = AmbientLightColourForFrame.blue * f;

	DirectionalLightColour.red = DirectionalLightColourForFrame.red * f;
	DirectionalLightColour.green = DirectionalLightColourForFrame.green * f;
	DirectionalLightColour.blue = DirectionalLightColourForFrame.blue * f;

	RpLightSetColor(pAmbient, &AmbientLightColour);
	RpLightSetColor(pDirect, &DirectionalLightColour);
}

void
SetBrightMarkerColours(float f)
{
	AmbientLightColour.red = 0.6f;
	AmbientLightColour.green = 0.6f;
	AmbientLightColour.blue = 0.6f;

	DirectionalLightColour.red = (1.0f - DirectionalLightColourForFrame.red) * 0.4f + DirectionalLightColourForFrame.red;
	DirectionalLightColour.green = (1.0f - DirectionalLightColourForFrame.green) * 0.4f + DirectionalLightColourForFrame.green;
	DirectionalLightColour.blue = (1.0f - DirectionalLightColourForFrame.blue) * 0.4f + DirectionalLightColourForFrame.blue;

	RpLightSetColor(pAmbient, &AmbientLightColour);
	RpLightSetColor(pDirect, &DirectionalLightColour);
}

void
ReSetAmbientAndDirectionalColours(void)
{
	RpLightSetColor(pAmbient, &AmbientLightColourForFrame);
	RpLightSetColor(pDirect, &DirectionalLightColourForFrame);
}

void
DeActivateDirectional(void)
{
	RpLightSetFlags(pDirect, 0);
}

void
ActivateDirectional(void)
{
	RpLightSetFlags(pDirect, rpLIGHTLIGHTATOMICS);
}

void
SetAmbientColours(void)
{
	RpLightSetColor(pAmbient, &AmbientLightColourForFrame);
}

void
SetAmbientColoursForPedsCarsAndObjects(void)
{
	RpLightSetColor(pAmbient, &AmbientLightColourForFrame_PedsCarsAndObjects);
}

uint8 IndicateR[] = { 0, 255, 0, 0, 255, 255, 0 };
uint8 IndicateG[] = { 0, 0, 255, 0, 255, 0, 255 };
uint8 IndicateB[] = { 0, 0, 0, 255, 0, 255, 255 };

void
SetAmbientColoursToIndicateRoadGroup(int i)
{
	AmbientLightColour.red   = IndicateR[i%7]/255.0f;
	AmbientLightColour.green = IndicateG[i%7]/255.0f;
	AmbientLightColour.blue  = IndicateB[i%7]/255.0f;
	RpLightSetColor(pAmbient, &AmbientLightColour);
}

void
SetAmbientColours(RwRGBAReal *color)
{
	RpLightSetColor(pAmbient, color);
}
