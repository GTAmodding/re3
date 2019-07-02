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

void CDraw::CalculateAspectRatio()
{
	SetScreenMult(DEFAULT_SCALE);

#ifndef HOR_PLUS
	if (FrontEndMenuManager.m_PrefsUseWideScreen)
		ms_fAspectRatio = 16.0f / 9.0f;
	else
		ms_fAspectRatio = 4.0f / 3.0f;
#else
	switch (FrontEndMenuManager.m_PrefsUseWideScreen) {
	case AR_AUTO:
		ms_fAspectRatio = (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT;
		return;
	case AR_3_2:
		ms_fAspectRatio = 3.0f / 2.0f;
		return;
	case AR_4_3:
		ms_fAspectRatio = 4.0f / 3.0f;
		return;
	case AR_5_3:
		ms_fAspectRatio = 5.0f / 3.0f;
		return;
	case AR_5_4:
		ms_fAspectRatio = 5.0f / 4.0f;
		return;
	case AR_15_9:
		ms_fAspectRatio = 15.0f / 9.0f;
		return;
	case AR_16_9:
		ms_fAspectRatio = 16.0f / 9.0f;
		return;
	case AR_16_10:
		ms_fAspectRatio = 16.0f / 10.0f;
		return;
	case AR_64_27:
		ms_fAspectRatio = 64.0f / 27.0f;
		return;
	};

#endif
}

float 
HorToVert(float hfov)
{
	float vfov = atan(tan(DEGTORAD(hfov) / 2) / (4.0 / 3.0)) * 2;
	return RADTODEG(hfov = atan(tan(vfov / 2) * CDraw::GetAspectRatio()) * 2);
}

void
CDraw::SetFOV(float fov)
{
#ifndef HOR_PLUS
	ms_fFOV = fov;
#else
	ms_fFOV = HorToVert(fov);
#endif
}

STARTPATCHES
	InjectHook(0x4FE7B0, CDraw::SetFOV, PATCH_JUMP);

	// Some patches to fix aiming offset on "CCamera::Find3rdPersonCamTargetVector".
	Nop(0x46B618, 2);
	Patch<float>(0x5F0A64, 1.3333334f);
ENDPATCHES
