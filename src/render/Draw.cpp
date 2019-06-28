#include "common.h"
#include "patcher.h"
#include "Draw.h"
#include "Frontend.h"
#include "Camera.h"

float CDraw::ms_fAspectRatio;
float CDraw::ms_fScreenMultiplier;

float &CDraw::ms_fNearClipZ = *(float*)0x8E2DC4;
float &CDraw::ms_fFarClipZ = *(float*)0x9434F0;
float &CDraw::ms_fFOV = *(float*)0x5FBC6C;

uint8 &CDraw::FadeValue = *(uint8*)0x95CD68;
uint8 &CDraw::FadeRed = *(uint8*)0x95CD90;
uint8 &CDraw::FadeGreen = *(uint8*)0x95CD71;
uint8 &CDraw::FadeBlue = *(uint8*)0x95CD53;

void
CDraw::CalculateAspectRatio()
{
	SetScreenMult(DEFAULT_SCALE);

	if(FrontEndMenuManager.m_PrefsUseWideScreen)	
		ms_fAspectRatio = 16.0f/9.0f;
	else
		ms_fAspectRatio = 4.0f/3.0f;
}

static float hFov2vFov(float hfov)
{
	float w = SCREENW;
	float h = SCREENH;

	// => tan(hFOV/2) = tan(vFOV/2)*aspectRatio
	// => tan(vFOV/2) = tan(hFOV/2)/aspectRatio
	float ar1 = 4.0/3.0;
	float ar2 = w/h;
	hfov = DEGTORAD(hfov);
	float vfov = atan(tan(hfov/2) / ar1) *2;
	hfov = atan(tan(vfov/2) * ar2) *2;
	return RADTODEG(hfov);
}

void
CDraw::SetFOV(float fov)
{
// TODO: fix FOV here or somewhere else?
//	ms_fFOV = hFov2vFov(fov);
	ms_fFOV = fov;
}

STARTPATCHES
	InjectHook(0x4FE7B0, CDraw::SetFOV, PATCH_JUMP);

	Nop(0x46B618, 2);
	Patch<float>(0x5F0A64, 1.3333334f);
ENDPATCHES
