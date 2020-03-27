#include "common.h"
#include "patcher.h"
#include "main.h"
#include "Draw.h"
#include "World.h"
#include "Vehicle.h"
#include "Ped.h"
#include "PlayerPed.h"
#include "Pad.h"
#include "General.h"
#include "ZoneCull.h"
#include "SurfaceTable.h"
#include "MBlur.h"
#include "Camera.h"

CCamera &TheCamera = *(CCamera*)0x6FACF8;
bool &CCamera::m_bUseMouse3rdPerson = *(bool *)0x5F03D8;

WRAPPER void CCamera::CamShake(float strength, float x, float y, float z) { EAXJMP(0x46B200); }
WRAPPER void CCamera::DrawBordersForWideScreen(void) { EAXJMP(0x46B430); }
WRAPPER void CCamera::CalculateDerivedValues(void) { EAXJMP(0x46EEA0); }
WRAPPER void CCamera::Restore(void) { EAXJMP(0x46F990); }
WRAPPER void CamShakeNoPos(CCamera*, float) { EAXJMP(0x46B100); }
WRAPPER void CCamera::TakeControl(CEntity*, int16, int16, int32) { EAXJMP(0x471500); }
WRAPPER void CCamera::TakeControlNoEntity(const CVector&, int16, int32) { EAXJMP(0x4715B0); }
WRAPPER void CCamera::Init(void) { EAXJMP(0x46BAD0); }
WRAPPER void CCamera::Process(void) { EAXJMP(0x46D3F0); }
WRAPPER void CCamera::LoadPathSplines(int file) { EAXJMP(0x46D1D0); }
WRAPPER void CCamera::RestoreWithJumpCut(void) { EAXJMP(0x46FAE0); };
WRAPPER void CCamera::SetPercentAlongCutScene(float) { EAXJMP(0x46FE20); };
WRAPPER void CCamera::SetParametersForScriptInterpolation(float, float, int32) { EAXJMP(0x46FDE0); }

bool
CCamera::GetFading()
{
	return m_bFading;
}

int
CCamera::GetFadingDirection()
{
	if(m_bFading)
		return m_iFadingDirection == FADE_IN ? FADE_IN : FADE_OUT;
	else
		return FADE_NONE;
}

bool
CCamera::IsSphereVisible(const CVector &center, float radius, const CMatrix *mat)
{
	RwV3d c;
	c = *(RwV3d*)&center;
	RwV3dTransformPoints(&c, &c, 1, &mat->m_matrix);
	if(c.y + radius < CDraw::GetNearClipZ()) return false;
	if(c.y - radius > CDraw::GetFarClipZ()) return false;
	if(c.x*m_vecFrustumNormals[0].x + c.y*m_vecFrustumNormals[0].y > radius) return false;
	if(c.x*m_vecFrustumNormals[1].x + c.y*m_vecFrustumNormals[1].y > radius) return false;
	if(c.y*m_vecFrustumNormals[2].y + c.z*m_vecFrustumNormals[2].z > radius) return false;
	if(c.y*m_vecFrustumNormals[3].y + c.z*m_vecFrustumNormals[3].z > radius) return false;
	return true;
}

bool
CCamera::IsSphereVisible(const CVector &center, float radius)
{
	CMatrix mat = m_cameraMatrix;
	return IsSphereVisible(center, radius, &mat);
}

bool
CCamera::IsPointVisible(const CVector &center, const CMatrix *mat)
{
	RwV3d c;
	c = *(RwV3d*)&center;
	RwV3dTransformPoints(&c, &c, 1, &mat->m_matrix);
	if(c.y < CDraw::GetNearClipZ()) return false;
	if(c.y > CDraw::GetFarClipZ()) return false;
	if(c.x*m_vecFrustumNormals[0].x + c.y*m_vecFrustumNormals[0].y > 0.0f) return false;
	if(c.x*m_vecFrustumNormals[1].x + c.y*m_vecFrustumNormals[1].y > 0.0f) return false;
	if(c.y*m_vecFrustumNormals[2].y + c.z*m_vecFrustumNormals[2].z > 0.0f) return false;
	if(c.y*m_vecFrustumNormals[3].y + c.z*m_vecFrustumNormals[3].z > 0.0f) return false;
	return true;
}

bool
CCamera::IsBoxVisible(RwV3d *box, const CMatrix *mat)
{
	int i;
	int frustumTests[6] = { 0 };
	RwV3dTransformPoints(box, box, 8, &mat->m_matrix);

	for(i = 0; i < 8; i++){
		if(box[i].y < CDraw::GetNearClipZ()) frustumTests[0]++;
		if(box[i].y > CDraw::GetFarClipZ()) frustumTests[1]++;
		if(box[i].x*m_vecFrustumNormals[0].x + box[i].y*m_vecFrustumNormals[0].y > 0.0f) frustumTests[2]++;
		if(box[i].x*m_vecFrustumNormals[1].x + box[i].y*m_vecFrustumNormals[1].y > 0.0f) frustumTests[3]++;
//	Why not test z?
//		if(box[i].y*m_vecFrustumNormals[2].y + box[i].z*m_vecFrustumNormals[2].z > 0.0f) frustumTests[4]++;
//		if(box[i].y*m_vecFrustumNormals[3].y + box[i].z*m_vecFrustumNormals[3].z > 0.0f) frustumTests[5]++;
	}
	for(i = 0; i < 6; i++)
		if(frustumTests[i] == 8)
			return false;		// Box is completely outside of one plane
	return true;
}

int
CCamera::GetLookDirection(void)
{
	if(Cams[ActiveCam].Mode == CCam::MODE_CAM_ON_A_STRING ||
	   Cams[ActiveCam].Mode == CCam::MODE_1STPERSON ||
	   Cams[ActiveCam].Mode == CCam::MODE_BEHINDBOAT ||
	   Cams[ActiveCam].Mode == CCam::MODE_FOLLOWPED)
		return Cams[ActiveCam].DirectionWasLooking;
	return LOOKING_FORWARD;;
}

bool
CCamera::GetLookingForwardFirstPerson()
{
	return Cams[ActiveCam].Mode == CCam::MODE_1STPERSON &&
		Cams[ActiveCam].DirectionWasLooking == LOOKING_FORWARD;
}


WRAPPER void CCamera::Fade(float timeout, int16 direction) { EAXJMP(0x46B3A0); }
WRAPPER void CCamera::ProcessFade(void) { EAXJMP(0x46F080); }
WRAPPER void CCamera::ProcessMusicFade(void) { EAXJMP(0x46F1E0); }

int
CCamera::GetScreenFadeStatus(void)
{
	if(m_fFLOATingFade == 0.0f)
		return FADE_0;
	if(m_fFLOATingFade == 255.0f)
		return FADE_2;
	return FADE_1;
}

void
CCamera::SetFadeColour(uint8 r, uint8 g, uint8 b)
{
	m_FadeTargetIsSplashScreen = r == 0 && g == 0 && b == 0;
	CDraw::FadeRed = r;
	CDraw::FadeGreen = g;
	CDraw::FadeBlue = b;
}

void
CCamera::SetMotionBlur(int r, int g, int b, int a, int type)
{
	m_BlurRed = r;
	m_BlurGreen = g;
	m_BlurBlue = b;
	m_motionBlur = a;
	m_BlurType = type;
}

void
CCamera::SetMotionBlurAlpha(int a)
{
	m_imotionBlurAddAlpha = a;
}

void
CCamera::SetNearClipScript(float clip)
{
	m_fNearClipScript = clip;
	m_bUseNearClipScript = true;
}

void
CCamera::RenderMotionBlur(void)
{
	if(m_BlurType == 0)
		return;

	CMBlur::MotionBlurRender(m_pRwCamera,
		m_BlurRed, m_BlurGreen, m_BlurBlue,
		m_motionBlur, m_BlurType, m_imotionBlurAddAlpha);
}

void
CCamera::ClearPlayerWeaponMode()
{
	PlayerWeaponMode.Mode = 0;
	PlayerWeaponMode.MaxZoom = 1;
	PlayerWeaponMode.MinZoom = -1;
	PlayerWeaponMode.Duration = 0.0f;
}

float
CCamera::Find3rdPersonQuickAimPitch(void)
{
	float clampedFrontZ = clamp(Cams[ActiveCam].Front.z, -1.0f, 1.0f);

	// float rot = atan2(clampedFrontZ, sqrt(1.0f - sq(clampedFrontZ)));
	float rot = Asin(clampedFrontZ);

	return -(DEGTORAD(((0.5f - m_f3rdPersonCHairMultY) * 1.8f * 0.5f * Cams[ActiveCam].FOV)) + rot);
}

void
CCamera::SetCamCutSceneOffSet(const CVector &pos)
{
	m_vecCutSceneOffset = pos;
};

void
CCamera::TakeControlWithSpline(short nSwitch)
{
	m_iModeToGoTo = CCam::MODE_FLYBY;
	m_bLookingAtPlayer = false;
	m_bLookingAtVector = false;
	m_bcutsceneFinished = false;
	m_iTypeOfSwitch = nSwitch;
	m_bStartInterScript = true;

	//FindPlayerPed(); // unused
};

void CCamera::SetCameraDirectlyInFrontForFollowPed_CamOnAString()
{
	m_bCamDirectlyInFront = true;
	CPlayerPed *player = FindPlayerPed();
	if (player)
		m_PedOrientForBehindOrInFront = CGeneral::GetATanOfXY(player->GetForward().x, player->GetForward().y);
}

void CCamera::SetCameraDirectlyBehindForFollowPed_CamOnAString()
{
	m_bCamDirectlyBehind = true;
	CPlayerPed *player = FindPlayerPed();
	if (player)
		m_PedOrientForBehindOrInFront = CGeneral::GetATanOfXY(player->GetForward().x, player->GetForward().y);
}

void
CCamera::SetWideScreenOn(void)
{
	m_WideScreenOn = true;
}

void
CCamera::SetWideScreenOff(void)
{
	m_bWantsToSwitchWidescreenOff = m_WideScreenOn;
}

void
CCamera::SetNewPlayerWeaponMode(int16 mode, int16 minZoom, int16 maxZoom)
{
	PlayerWeaponMode.Mode = mode;
	PlayerWeaponMode.MaxZoom = maxZoom;
	PlayerWeaponMode.MinZoom = minZoom;
	PlayerWeaponMode.Duration = 0.0f;
}

void
CCamera::UpdateAimingCoors(CVector const &coors)
{
	m_cvecAimingTargetCoors = coors;
}

void
CCamera::SetCamPositionForFixedMode(const CVector &Source, const CVector &UpOffSet)
{
	m_vecFixedModeSource = Source;
	m_vecFixedModeUpOffSet = UpOffSet;
}

void
CCamera::SetRwCamera(RwCamera *cam)
{
	m_pRwCamera = cam;
	m_viewMatrix.Attach(&m_pRwCamera->viewMatrix, false);
	CMBlur::MotionBlurOpen(m_pRwCamera);
}

uint32
CCamera::GetCutSceneFinishTime(void)
{
	int cam = ActiveCam;
	if (Cams[cam].Mode == CCam::MODE_FLYBY)
		return Cams[cam].m_uiFinishTime;
	cam = (cam + 1) % 2;
	if (Cams[cam].Mode == CCam::MODE_FLYBY)
		return Cams[cam].m_uiFinishTime;

	return 0;
}

void
CCamera::FinishCutscene(void)
{
	SetPercentAlongCutScene(100.0f);
	m_fPositionAlongSpline = 1.0f;
	m_bcutsceneFinished = true;
}

void
CCamera::SetZoomValueFollowPedScript(int16 mode)
{
	switch (mode) {
	case 0: m_fPedZoomValueScript = 0.25f; break;
	case 1: m_fPedZoomValueScript = 1.5f; break;
	case 2: m_fPedZoomValueScript = 2.9f; break;
	default: m_fPedZoomValueScript = m_fPedZoomValueScript; break;
	}

	m_bUseScriptZoomValuePed = true;
}

void
CCamera::SetZoomValueCamStringScript(int16 mode)
{
	switch (mode) {
	case 0: m_fCarZoomValueScript = 0.05f; break;
	case 1: m_fCarZoomValueScript = 1.9f; break;
	case 2: m_fCarZoomValueScript = 3.9f; break;
	default: m_fCarZoomValueScript = m_fCarZoomValueScript; break;
	}

	m_bUseScriptZoomValueCar = true;
}

STARTPATCHES
	InjectHook(0x42C760, (bool (CCamera::*)(const CVector &center, float radius, const CMatrix *mat))&CCamera::IsSphereVisible, PATCH_JUMP);
	InjectHook(0x46FD00, &CCamera::SetFadeColour, PATCH_JUMP);

	InjectHook(0x46FD40, &CCamera::SetMotionBlur, PATCH_JUMP);
	InjectHook(0x46FD80, &CCamera::SetMotionBlurAlpha, PATCH_JUMP);
	InjectHook(0x46F940, &CCamera::RenderMotionBlur, PATCH_JUMP);

	InjectHook(0x46FC90, &CCamera::SetCameraDirectlyInFrontForFollowPed_CamOnAString, PATCH_JUMP);

	InjectHook(0x46FF00, &CCamera::SetWideScreenOn, PATCH_JUMP);
	InjectHook(0x46FF10, &CCamera::SetWideScreenOff, PATCH_JUMP);

	InjectHook(0x46FCC0, &CCamera::SetCamPositionForFixedMode, PATCH_JUMP);
	InjectHook(0x46FEC0, &CCamera::SetRwCamera, PATCH_JUMP);
	InjectHook(0x46B920, &CCamera::GetCutSceneFinishTime, PATCH_JUMP);
	InjectHook(0x46B560, &CCamera::FinishCutscene, PATCH_JUMP);
	InjectHook(0x46FF30, &CCamera::SetZoomValueFollowPedScript, PATCH_JUMP);
	InjectHook(0x46FF90, &CCamera::SetZoomValueCamStringScript, PATCH_JUMP);
ENDPATCHES
