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

const float DefaultFOV = 70.0f;	// beta: 80.0f

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


/*
 *
 * CCam
 *
 */


// MaxSpeed is a limit of how fast the value is allowed to change. 1.0 = to Target in up to 1ms
// Acceleration is how fast the speed will change to MaxSpeed. 1.0 = to MaxSpeed in 1ms
void
WellBufferMe(float Target, float *CurrentValue, float *CurrentSpeed, float MaxSpeed, float Acceleration, bool IsAngle)
{
	float Delta = Target - *CurrentValue;

	if(IsAngle){
		while(Delta >= PI) Delta -= 2*PI;
		while(Delta < -PI) Delta += 2*PI;
	}

	float TargetSpeed = Delta * MaxSpeed;
	// Add or subtract absolute depending on sign, genius!
//	if(TargetSpeed - *CurrentSpeed > 0.0f)
//		*CurrentSpeed += Acceleration * Abs(TargetSpeed - *CurrentSpeed) * CTimer::GetTimeStep();
//	else
//		*CurrentSpeed -= Acceleration * Abs(TargetSpeed - *CurrentSpeed) * CTimer::GetTimeStep();
	// this is simpler:
	*CurrentSpeed += Acceleration * (TargetSpeed - *CurrentSpeed) * CTimer::GetTimeStep();

	// Clamp speed if we overshot
	if(TargetSpeed < 0.0f && *CurrentSpeed < TargetSpeed)
		*CurrentSpeed = TargetSpeed;
	else if(TargetSpeed > 0.0f && *CurrentSpeed > TargetSpeed)
		*CurrentSpeed = TargetSpeed;

	*CurrentValue += *CurrentSpeed * min(10.0f, CTimer::GetTimeStep());
}

void
CCam::GetVectorsReadyForRW(void)
{
	CVector right;
	Up = CVector(0.0f, 0.0f, 1.0f);
	Front.Normalise();
	if(Front.x == 0.0f && Front.y == 0.0f){
		Front.x = 0.0001f;
		Front.y = 0.0001f;
	}
	right = CrossProduct(Front, Up);
	right.Normalise();
	Up = CrossProduct(right, Front);
}

// This code is really bad. wtf R*?
CVector
CCam::DoAverageOnVector(const CVector &vec)
{
	int i;
	CVector Average = { 0.0f, 0.0f, 0.0f };

	if(ResetStatics){
		m_iRunningVectorArrayPos = 0;
		m_iRunningVectorCounter = 1;
	}

	// TODO: make this work with NUMBER_OF_VECTORS_FOR_AVERAGE != 2
	if(m_iRunningVectorCounter == 3){
		m_arrPreviousVectors[0] = m_arrPreviousVectors[1];
		m_arrPreviousVectors[1] = vec;
	}else
		m_arrPreviousVectors[m_iRunningVectorArrayPos] = vec;

	for(i = 0; i <= m_iRunningVectorArrayPos; i++)
		Average += m_arrPreviousVectors[i];
	Average /= i;

	m_iRunningVectorArrayPos++;
	m_iRunningVectorCounter++;
	if(m_iRunningVectorArrayPos >= NUMBER_OF_VECTORS_FOR_AVERAGE)
		m_iRunningVectorArrayPos = NUMBER_OF_VECTORS_FOR_AVERAGE-1;
	if(m_iRunningVectorCounter > NUMBER_OF_VECTORS_FOR_AVERAGE+1)
		m_iRunningVectorCounter = NUMBER_OF_VECTORS_FOR_AVERAGE+1;

	return Average;
}

// Rotate Beta in direction opposite of BetaOffset in 5 deg. steps.
// Return the first angle for which Beta + BetaOffset + Angle has a clear view.
// i.e. BetaOffset is a safe zone so that Beta + Angle is really clear.
// If BetaOffset == 0, try both directions.
float
CCam::GetPedBetaAngleForClearView(const CVector &Target, float Dist, float BetaOffset, bool checkBuildings, bool checkVehicles, bool checkPeds, bool checkObjects, bool checkDummies)
{
	CColPoint point;
	CEntity *ent = nil;
	CVector ToSource;
	float a;

	// This would be so much nicer if we just got the step variable before the loop...R*

	for(a = 0.0f; a <= PI; a += DEGTORAD(5.0f)){
		if(BetaOffset <= 0.0f){
			ToSource = CVector(Cos(Beta + BetaOffset + a), Sin(Beta + BetaOffset + a), 0.0f)*Dist;
			if(!CWorld::ProcessLineOfSight(Target, Target + ToSource,
						point, ent, checkBuildings, checkVehicles, checkPeds,
						checkObjects, checkDummies, true, true))
				return a;
		}
		if(BetaOffset >= 0.0f){
			ToSource = CVector(Cos(Beta + BetaOffset - a), Sin(Beta + BetaOffset - a), 0.0f)*Dist;
			if(!CWorld::ProcessLineOfSight(Target, Target + ToSource,
						point, ent, checkBuildings, checkVehicles, checkPeds,
						checkObjects, checkDummies, true, true))
				return -a;
		}
	}
	return 0.0f;
}

static float DefaultAcceleration = 0.045f;
static float DefaultMaxStep = 0.15f;

void
CCam::Process_FollowPed(const CVector &CameraTarget, float TargetOrientation, float, float)
{
	const float GroundDist = 1.85f;

	CVector TargetCoors, Dist, IdealSource;
	float Length = 0.0f;
	float LateralLeft = 0.0f;
	float LateralRight = 0.0f;
	float Center = 0.0f;
	static bool PreviouslyObscured;
	static bool PickedASide;
	static float FixedTargetOrientation = 0.0f;
	float AngleToGoTo = 0.0f;
	float BetaOffsetAvoidBuildings = 0.45f;		// ~25 deg
	float BetaOffsetGoingBehind = 0.45f;
	bool GoingBehind = false;
	bool Obscured = false;
	bool BuildingCheckObscured = false;
	bool HackPlayerOnStoppingTrain = false;
	static int TimeIndicatedWantedToGoDown = 0;
	static bool StartedCountingForGoDown = false;
	float DeltaBeta;

	m_bFixingBeta = false;
	bBelowMinDist = false;
	bBehindPlayerDesired = false;

	assert(CamTargetEntity->IsPed());

	// CenterDist should be > LateralDist because we don't have an angle for safety in this case
	float CenterDist, LateralDist;
	float AngleToGoToSpeed;
	if(m_fCloseInPedHeightOffsetSpeed > 0.00001f){
		LateralDist = 0.55f;
		CenterDist = 1.25f;
		BetaOffsetAvoidBuildings = 0.9f;	// ~50 deg
		BetaOffsetGoingBehind = 0.9f;
		AngleToGoToSpeed = 0.88254666f;
	}else{
		LateralDist = 0.8f;
		CenterDist = 1.35f;
		if(TheCamera.PedZoomIndicator == 1.0f || TheCamera.PedZoomIndicator == 4.0f){
			LateralDist = 1.25f;
			CenterDist = 1.6f;
		}
		AngleToGoToSpeed = 0.43254671f;
	}

	FOV = DefaultFOV;

	if(ResetStatics){
		Rotating = false;
		m_bCollisionChecksOn = true;
		FixedTargetOrientation = 0.0f;
		PreviouslyObscured = false;
		PickedASide = false;
		StartedCountingForGoDown = false;
		AngleToGoTo = 0.0f;
		// unused LastAngleWithNoPickedASide
	}


	TargetCoors = CameraTarget;
	IdealSource = Source;
	TargetCoors.z += m_fSyphonModeTargetZOffSet;

	CVector TempTargetCoors;
	TempTargetCoors = DoAverageOnVector(TargetCoors);
	TargetCoors = TempTargetCoors;
	// Add this unknown offset, but later it's removed again
	TargetCoors.z += m_fUnknownZOffSet;

	Dist.x = IdealSource.x - TargetCoors.x;
	Dist.y = IdealSource.y - TargetCoors.y;
	Length = Dist.Magnitude2D();

	// Cam on a string. With a fixed distance. Zoom in/out is done later.
	if(Length != 0.0f)
		IdealSource = TargetCoors + CVector(Dist.x, Dist.y, 0.0f)/Length * GroundDist;
	else
		IdealSource = TargetCoors + CVector(1.0f, 1.0f, 0.0f);

	// TODO: what's transition beta?
	if(TheCamera.m_bUseTransitionBeta && ResetStatics){
		CVector VecDistance;
		IdealSource.x = TargetCoors.x + GroundDist*Cos(m_fTransitionBeta);
		IdealSource.y = TargetCoors.y + GroundDist*Sin(m_fTransitionBeta);
		Beta = CGeneral::GetATanOfXY(IdealSource.x - TargetCoors.x, IdealSource.y - TargetCoors.y);
	}else
		Beta = CGeneral::GetATanOfXY(Source.x - TargetCoors.x, Source.y - TargetCoors.y);

	if(TheCamera.m_bCamDirectlyBehind){
		 m_bCollisionChecksOn = true;
		 Beta = TargetOrientation + PI;
	}

	if(FindPlayerVehicle())
		if(FindPlayerVehicle()->m_vehType == VEHICLE_TYPE_TRAIN)
			HackPlayerOnStoppingTrain = true;

	if(TheCamera.m_bCamDirectlyInFront){
		 m_bCollisionChecksOn = true;
		 Beta = TargetOrientation;
	}

	while(Beta >= PI) Beta -= 2.0f * PI;
	while(Beta < -PI) Beta += 2.0f * PI;

	// BUG? is this ever used?
	// The values seem to be roughly m_fPedZoomValueSmooth + 1.85
	if(ResetStatics){
		if(TheCamera.PedZoomIndicator == 1.0f) m_fRealGroundDist = 2.090556f;
		if(TheCamera.PedZoomIndicator == 2.0f) m_fRealGroundDist = 3.34973f;
		if(TheCamera.PedZoomIndicator == 3.0f) m_fRealGroundDist = 4.704914f;
		if(TheCamera.PedZoomIndicator == 4.0f) m_fRealGroundDist = 2.090556f;
	}
	// And what is this? It's only used for collision and rotation it seems
	float RealGroundDist;
	if(TheCamera.PedZoomIndicator == 1.0f) RealGroundDist = 2.090556f;
	if(TheCamera.PedZoomIndicator == 2.0f) RealGroundDist = 3.34973f;
	if(TheCamera.PedZoomIndicator == 3.0f) RealGroundDist = 4.704914f;
	if(TheCamera.PedZoomIndicator == 4.0f) RealGroundDist = 2.090556f;
	if(m_fCloseInPedHeightOffset >  0.00001f)
		RealGroundDist = 1.7016f;


	bool Shooting = false;
	CPed *ped = (CPed*)CamTargetEntity;
	if(ped->GetWeapon()->m_eWeaponType != WEAPONTYPE_UNARMED)
		if(CPad::GetPad(0)->GetWeapon())
			Shooting = true;
	if(ped->GetWeapon()->m_eWeaponType == WEAPONTYPE_DETONATOR ||
	   ped->GetWeapon()->m_eWeaponType == WEAPONTYPE_BASEBALLBAT)
		Shooting = false;


	if(m_fCloseInPedHeightOffset > 0.00001f)
		TargetCoors.z -= m_fUnknownZOffSet;

	// Figure out if and where we want to rotate

	if(CPad::GetPad(0)->ForceCameraBehindPlayer() || Shooting){

		// Center cam behind player

		GoingBehind = true;
		m_bCollisionChecksOn = true;
		float OriginalBeta = Beta;
		// Set Beta behind player
		Beta = TargetOrientation + PI;
		TargetCoors.z -= 0.1f;

		AngleToGoTo = GetPedBetaAngleForClearView(TargetCoors, CenterDist * RealGroundDist, 0.0f, true, false, false, true, false);
		if(AngleToGoTo != 0.0f){
			if(AngleToGoTo < 0.0f)
				AngleToGoTo -= AngleToGoToSpeed;
			else
				AngleToGoTo += AngleToGoToSpeed;
		}else{
			float LateralLeft = GetPedBetaAngleForClearView(TargetCoors, LateralDist * RealGroundDist, BetaOffsetGoingBehind, true, false, false, true, false);
			float LateralRight = GetPedBetaAngleForClearView(TargetCoors, LateralDist * RealGroundDist, -BetaOffsetGoingBehind, true, false, false, true, false);
			if(LateralLeft == 0.0f && LateralRight != 0.0f)
				AngleToGoTo += LateralRight;
			else if(LateralLeft != 0.0f && LateralRight == 0.0f)
				AngleToGoTo += LateralLeft;
		}

		TargetCoors.z += 0.1f;
		Beta = OriginalBeta;

		if(PickedASide){
			if(AngleToGoTo == 0.0f)
				FixedTargetOrientation = TargetOrientation + PI;
			Rotating = true;
		}else{
			FixedTargetOrientation = TargetOrientation + PI + AngleToGoTo;
			Rotating = true;
			PickedASide = true;
		}
	}else{

		// Rotate cam to avoid clipping into buildings

		TargetCoors.z -= 0.1f;

		Center = GetPedBetaAngleForClearView(TargetCoors, CenterDist * RealGroundDist, 0.0f, true, false, false, true, false);
		if(m_bCollisionChecksOn || PreviouslyObscured || Center != 0.0f || m_fCloseInPedHeightOffset > 0.00001f){
			if(Center != 0.0f){
				AngleToGoTo = Center;
			}else{
				LateralLeft = GetPedBetaAngleForClearView(TargetCoors, LateralDist * RealGroundDist, BetaOffsetAvoidBuildings, true, false, false, true, false);
				LateralRight = GetPedBetaAngleForClearView(TargetCoors, LateralDist * RealGroundDist, -BetaOffsetAvoidBuildings, true, false, false, true, false);
				if(LateralLeft == 0.0f && LateralRight != 0.0f){
					AngleToGoTo += LateralRight;
					if(m_fCloseInPedHeightOffset > 0.0f)
						RwCameraSetNearClipPlane(Scene.camera, 0.7f);
				}else if(LateralLeft != 0.0f && LateralRight == 0.0f){
					AngleToGoTo += LateralLeft;
					if(m_fCloseInPedHeightOffset > 0.0f)
						RwCameraSetNearClipPlane(Scene.camera, 0.7f);
				}
			}
			if(LateralLeft != 0.0f || LateralRight != 0.0f || Center != 0.0f)
				BuildingCheckObscured = true;
		}

		TargetCoors.z += 0.1f;
	}

	if(m_fCloseInPedHeightOffset > 0.00001f)
		TargetCoors.z += m_fUnknownZOffSet;


	// Have to fix to avoid collision

	if(AngleToGoTo != 0.0f){
		Obscured = true;
		Rotating = true;
		if(CPad::GetPad(0)->ForceCameraBehindPlayer() || Shooting){
			if(!PickedASide)
				FixedTargetOrientation = Beta + AngleToGoTo;	// can this even happen?
		}else
			FixedTargetOrientation = Beta + AngleToGoTo;

		// This calculation is only really used to figure out how fast to rotate out of collision

		m_fAmountFractionObscured = 1.0f;
		CVector PlayerPos = FindPlayerPed()->GetPosition();
		float RotationDist = (AngleToGoTo == Center ? CenterDist : LateralDist) * RealGroundDist;
		// What's going on here? - AngleToGoTo?
		CVector RotatedSource = PlayerPos + CVector(Cos(Beta - AngleToGoTo), Sin(Beta - AngleToGoTo), 0.0f) * RotationDist;

		CColPoint colpoint;
		CEntity *entity;
		if(CWorld::ProcessLineOfSight(PlayerPos, RotatedSource, colpoint, entity, true, false, false, true, false, false, false)){
			if((PlayerPos - RotatedSource).Magnitude() != 0.0f)
				m_fAmountFractionObscured = (PlayerPos - colpoint.point).Magnitude() / (PlayerPos - RotatedSource).Magnitude();
			else
				m_fAmountFractionObscured = 1.0f;
		}
	}
	if(m_fAmountFractionObscured < 0.0f) m_fAmountFractionObscured = 0.0f;
	if(m_fAmountFractionObscured > 1.0f) m_fAmountFractionObscured = 1.0f;



	// Figure out speed values for Beta rotation

	float Acceleration, MaxSpeed;
	static float AccelerationMult = 0.35f;
	static float MaxSpeedMult = 0.85f;
	static float AccelerationMultClose = 0.7f;
	static float MaxSpeedMultClose = 1.6f;
	float BaseAcceleration = 0.025f;
	float BaseMaxSpeed = 0.09f;
	if(m_fCloseInPedHeightOffset > 0.00001f){
		if(AngleToGoTo == 0.0f){
			BaseAcceleration = 0.022f;
			BaseMaxSpeed = 0.04f;
		}else{
			BaseAcceleration = DefaultAcceleration;
			BaseMaxSpeed = DefaultMaxStep;
		}
	}
	if(AngleToGoTo == 0.0f){
		Acceleration = BaseAcceleration;
		MaxSpeed = BaseMaxSpeed;
	}else if(CPad::GetPad(0)->ForceCameraBehindPlayer() && !Shooting){
		Acceleration = 0.051f;
		MaxSpeed = 0.18f;
	}else if(m_fCloseInPedHeightOffset > 0.00001f){
		Acceleration = BaseAcceleration + AccelerationMultClose*sq(m_fAmountFractionObscured - 1.05f);
		MaxSpeed = BaseMaxSpeed + MaxSpeedMultClose*sq(m_fAmountFractionObscured - 1.05f);
	}else{
		Acceleration = DefaultAcceleration + AccelerationMult*sq(m_fAmountFractionObscured - 1.05f);
		MaxSpeed = DefaultMaxStep + MaxSpeedMult*sq(m_fAmountFractionObscured - 1.05f);
	}
	static float AccelerationLimit = 0.3f;
	static float MaxSpeedLimit = 0.65f;
	if(Acceleration > AccelerationLimit) Acceleration = AccelerationLimit;
	if(MaxSpeed > MaxSpeedLimit) MaxSpeed = MaxSpeedLimit;


	int MoveState = ((CPed*)CamTargetEntity)->m_nMoveState;
	if(MoveState != PEDMOVE_NONE && MoveState != PEDMOVE_STILL &&
	   !CPad::GetPad(0)->ForceCameraBehindPlayer() && !Obscured && !Shooting){
		Rotating = false;
		BetaSpeed = 0.0f;
	}

	// Now do the Beta rotation

	float Distance = (IdealSource - TargetCoors).Magnitude2D();
	m_fDistanceBeforeChanges = Distance;

	if(Rotating){
		m_bFixingBeta = true;

		while(FixedTargetOrientation >= PI) FixedTargetOrientation -= 2*PI;
		while(FixedTargetOrientation < -PI) FixedTargetOrientation += 2*PI;

		while(Beta >= PI) Beta -= 2*PI;
		while(Beta < -PI) Beta += 2*PI;


/*
		// This is inlined WellBufferMe
		DeltaBeta = FixedTargetOrientation - Beta;
		while(DeltaBeta >= PI) DeltaBeta -= 2*PI;
		while(DeltaBeta < -PI) DeltaBeta += 2*PI;

		float ReqSpeed = DeltaBeta * MaxSpeed;
		// Add or subtract absolute depending on sign, genius!
		if(ReqSpeed - BetaSpeed > 0.0f)
			BetaSpeed += SpeedStep * Abs(ReqSpeed - BetaSpeed) * CTimer::GetTimeStep();
		else
			BetaSpeed -= SpeedStep * Abs(ReqSpeed - BetaSpeed) * CTimer::GetTimeStep();
		// this would be simpler:
		// BetaSpeed += SpeedStep * (ReqSpeed - BetaSpeed) * CTimer::ms_fTimeStep;

		if(ReqSpeed < 0.0f && BetaSpeed < ReqSpeed)
			BetaSpeed = ReqSpeed;
		else if(ReqSpeed > 0.0f && BetaSpeed > ReqSpeed)
			BetaSpeed = ReqSpeed;

		Beta += BetaSpeed * min(10.0f, CTimer::GetTimeStep());
*/
		WellBufferMe(FixedTargetOrientation, &Beta, &BetaSpeed, MaxSpeed, Acceleration, true);

		if(ResetStatics){
			Beta = FixedTargetOrientation;
			BetaSpeed = 0.0f;
		}

		Source.x = TargetCoors.x + Distance * Cos(Beta);
		Source.y = TargetCoors.y + Distance * Sin(Beta);

		// Check if we can stop rotating
		DeltaBeta = FixedTargetOrientation - Beta;
		while(DeltaBeta >= PI) DeltaBeta -= 2*PI;
		while(DeltaBeta < -PI) DeltaBeta += 2*PI;
		if(Abs(DeltaBeta) < DEGTORAD(1.0f) && !bBehindPlayerDesired){
			// Stop rotation
			PickedASide = false;
			Rotating = false;
			BetaSpeed = 0.0f;
		}
	}


	if(TheCamera.m_bCamDirectlyBehind || TheCamera.m_bCamDirectlyInFront ||
	   HackPlayerOnStoppingTrain || Rotating){
		if(TheCamera.m_bCamDirectlyBehind){
			Beta = TargetOrientation + PI;
			Source.x = TargetCoors.x + Distance * Cos(Beta);
			Source.y = TargetCoors.y + Distance * Sin(Beta);
		}
		if(TheCamera.m_bCamDirectlyInFront){
			Beta = TargetOrientation;
			Source.x = TargetCoors.x + Distance * Cos(Beta);
			Source.y = TargetCoors.y + Distance * Sin(Beta);
		}
		if(HackPlayerOnStoppingTrain){
			Beta = TargetOrientation + PI;
			Source.x = TargetCoors.x + Distance * Cos(Beta);
			Source.y = TargetCoors.y + Distance * Sin(Beta);
			m_fDimensionOfHighestNearCar = 0.0f;
			m_fCamBufferedHeight = 0.0f;
			m_fCamBufferedHeightSpeed = 0.0f;
		}
		// Beta and Source already set in the rotation code
	}else{
		Source = IdealSource;
		BetaSpeed = 0.0f;
	}

	// Subtract m_fUnknownZOffSet from both?
	TargetCoors.z -= m_fUnknownZOffSet;
	Source.z = IdealSource.z - m_fUnknownZOffSet;

	// Apply zoom now
	// m_fPedZoomValueSmooth makes the cam go down the further out it is
	//  0.25 ->  0.20 for nearest dist
	//  1.50 -> -0.05 for mid dist
	//  2.90 -> -0.33 for far dist
	Source.z += (2.5f - TheCamera.m_fPedZoomValueSmooth)*0.2f - 0.25f;
	// Zoom out camera
	Front = TargetCoors - Source;
	Front.Normalise();
	Source -= Front * TheCamera.m_fPedZoomValueSmooth;
	// and then we move up again
	//  -0.375
	//   0.25
	//   0.95
	Source.z += (TheCamera.m_fPedZoomValueSmooth - 1.0f)*0.5f + m_fCloseInPedHeightOffset;


	// Process height offset to avoid peds and cars

	float TargetZOffSet = m_fUnknownZOffSet + m_fDimensionOfHighestNearCar;
	TargetZOffSet = max(TargetZOffSet, m_fPedBetweenCameraHeightOffset);
	float TargetHeight = CameraTarget.z + TargetZOffSet - Source.z;

	if(TargetHeight > m_fCamBufferedHeight){
		// Have to go up
		if(TargetZOffSet == m_fPedBetweenCameraHeightOffset && TargetZOffSet > m_fCamBufferedHeight)
			WellBufferMe(TargetHeight, &m_fCamBufferedHeight, &m_fCamBufferedHeightSpeed, 0.2f, 0.04f, false);
		else if(TargetZOffSet == m_fUnknownZOffSet && TargetZOffSet > m_fCamBufferedHeight){
			// TODO: figure this out
			bool foo = false;
			switch(((CPhysical*)CamTargetEntity)->m_nSurfaceTouched)
			case SURFACE_GRASS:
			case SURFACE_DIRT:
			case SURFACE_PAVEMENT:
			case SURFACE_STEEL:
			case SURFACE_TIRE:
			case SURFACE_STONE:
				foo = true;
			if(foo)
				WellBufferMe(TargetHeight, &m_fCamBufferedHeight, &m_fCamBufferedHeightSpeed, 0.4f, 0.05f, false);
			else
				WellBufferMe(TargetHeight, &m_fCamBufferedHeight, &m_fCamBufferedHeightSpeed, 0.2f, 0.025f, false);
		}else
			WellBufferMe(TargetHeight, &m_fCamBufferedHeight, &m_fCamBufferedHeightSpeed, 0.2f, 0.025f, false);
		StartedCountingForGoDown = false;
	}else{
		// Have to go down
		if(StartedCountingForGoDown){
			if(CTimer::GetTimeInMilliseconds() != TimeIndicatedWantedToGoDown){
				if(TargetHeight > 0.0f)
					WellBufferMe(TargetHeight, &m_fCamBufferedHeight, &m_fCamBufferedHeightSpeed, 0.2f, 0.01f, false);
				else
					WellBufferMe(0.0f, &m_fCamBufferedHeight, &m_fCamBufferedHeightSpeed, 0.2f, 0.01f, false);
			}
		}else{
			StartedCountingForGoDown = true;
			TimeIndicatedWantedToGoDown = CTimer::GetTimeInMilliseconds();
		}
	}

	Source.z += m_fCamBufferedHeight;


	// Clip Source if necessary

	bool ClipSource = m_fCloseInPedHeightOffset > 0.00001f && m_fCamBufferedHeight > 0.001f;
	if(GoingBehind || ResetStatics || ClipSource){
		CColPoint colpoint;
		CEntity *entity;
		if(CWorld::ProcessLineOfSight(TargetCoors, Source, colpoint, entity, true, false, false, true, false, true, true)){
			Source = colpoint.point;
			if((TargetCoors - Source).Magnitude2D() < 1.0f)
				RwCameraSetNearClipPlane(Scene.camera, 0.05f);
		}
	}

	TargetCoors.z += min(1.0f, m_fCamBufferedHeight/2.0f);
	m_cvecTargetCoorsForFudgeInter = TargetCoors;

	Front = TargetCoors - Source;
	m_fRealGroundDist = Front.Magnitude2D();
	m_fMinDistAwayFromCamWhenInterPolating = m_fRealGroundDist;	
	Front.Normalise();
	GetVectorsReadyForRW();
	TheCamera.m_bCamDirectlyBehind = false;
	TheCamera.m_bCamDirectlyInFront = false;
	PreviouslyObscured = BuildingCheckObscured;

	ResetStatics = false;
}

void
CCam::Process_BehindCar(const CVector &CameraTarget, float TargetOrientation, float, float)
{
	FOV = DefaultFOV;

	if(!CamTargetEntity->IsVehicle())
		return;

	CVector TargetCoors = CameraTarget;
	TargetCoors.z -= 0.2f;
	CA_MAX_DISTANCE = 9.95f;
	CA_MIN_DISTANCE = 8.5f;

	CVector Dist = Source - TargetCoors;
	float Length = Dist.Magnitude2D();
	m_fDistanceBeforeChanges = Length;
	if(Length < 0.002f)
		Length = 0.002f;
	Beta = CGeneral::GetATanOfXY(TargetCoors.x - Source.x, TargetCoors.y - Source.y);
	if(Length > CA_MAX_DISTANCE){
		Source.x = TargetCoors.x + Dist.x/Length * CA_MAX_DISTANCE;
		Source.y = TargetCoors.y + Dist.y/Length * CA_MAX_DISTANCE;
	}else if(Length < CA_MIN_DISTANCE){
		Source.x = TargetCoors.x + Dist.x/Length * CA_MIN_DISTANCE;
		Source.y = TargetCoors.y + Dist.y/Length * CA_MIN_DISTANCE;
	}
	TargetCoors.z += 0.8f;

	WorkOutCamHeightWeeCar(TargetCoors, TargetOrientation);
	RotCamIfInFrontCar(TargetCoors, TargetOrientation);
	FixCamIfObscured(TargetCoors, 1.2f, TargetOrientation);

	Front = TargetCoors - Source;
	m_cvecTargetCoorsForFudgeInter = TargetCoors;
	ResetStatics = false;
	GetVectorsReadyForRW();
}

void
CCam::WorkOutCamHeightWeeCar(CVector &TargetCoors, float TargetOrientation)
{
	CColPoint colpoint;
	CEntity *ent;
	float TargetZOffSet = 0.0f;
	static bool PreviouslyFailedRoadHeightCheck = false;
	static float RoadHeightFix = 0.0f;
	static float RoadHeightFixSpeed = 0.0f;

	if(ResetStatics){
		RoadHeightFix = 0.0f;
		RoadHeightFixSpeed = 0.0f;
		Alpha = DEGTORAD(25.0f);
		AlphaSpeed = 0.0f;
	}
	float AlphaTarget = DEGTORAD(25.0f);
	if(CCullZones::CamNoRain() || CCullZones::PlayerNoRain())
		AlphaTarget = DEGTORAD(14.0f);
	WellBufferMe(AlphaTarget, &Alpha, &AlphaSpeed, 0.1f, 0.05f, true);
	Source.z = TargetCoors.z + CA_MAX_DISTANCE*Sin(Alpha);

	if(FindPlayerVehicle()){
		m_fUnknownZOffSet = 0.0f;
		bool FoundRoad = false;
		bool FoundRoof = false;
		float RoadZ = 0.0f;
		float RoofZ = 0.0f;

		if(CWorld::ProcessVerticalLine(Source, -1000.0f, colpoint, ent, true, false, false, false, false, false, nil) &&
		   ent->IsBuilding()){
			FoundRoad = true;
			RoadZ = colpoint.point.z;
		}

		if(FoundRoad){
			if(Source.z - RoadZ < 0.9f){
				PreviouslyFailedRoadHeightCheck = true;
				TargetZOffSet = RoadZ + 0.9f - Source.z;
			}else{
				if(m_bCollisionChecksOn)
					PreviouslyFailedRoadHeightCheck = false;
				else
					TargetZOffSet = 0.0f;
			}
		}else{
			if(CWorld::ProcessVerticalLine(Source, 1000.0f, colpoint, ent, true, false, false, false, false, false, nil) &&
			   ent->IsBuilding()){
				FoundRoof = true;
				RoofZ = colpoint.point.z;
			}
			if(FoundRoof){
				if(Source.z - RoofZ < 0.9f){
					PreviouslyFailedRoadHeightCheck = true;
					TargetZOffSet = RoofZ + 0.9f - Source.z;
				}else{
					if(m_bCollisionChecksOn)
						PreviouslyFailedRoadHeightCheck = false;
					else
						TargetZOffSet = 0.0f;
				}
			}
		}
	}

	if(TargetZOffSet > RoadHeightFix)
		RoadHeightFix = TargetZOffSet;
	else
		WellBufferMe(TargetZOffSet, &RoadHeightFix, &RoadHeightFixSpeed, 0.27f, 0.1f, false);

	if((colpoint.surfaceB == SURFACE_DEFAULT || colpoint.surfaceB >= SURFACE_METAL6) &&
	   colpoint.surfaceB != SURFACE_STEEL && colpoint.surfaceB != SURFACE_STONE &&
	   RoadHeightFix > 1.4f)
		RoadHeightFix = 1.4f;

	Source.z += RoadHeightFix;
}

void
CCam::WorkOutCamHeight(const CVector &TargetCoors, float TargetOrientation, float TargetHeight)
{
	static float LastTargetAlphaWithCollisionOn = 0.0f;
	static float LastTopAlphaSpeed = 0.0f;
	static float LastAlphaSpeedStep = 0.0f;
	static bool PreviousNearCheckNearClipSmall = false;

	bool CamClear = true;
	float ModeAlpha = 0.0f;

	if(ResetStatics){
		LastTargetAlphaWithCollisionOn = 0.0f;
		LastTopAlphaSpeed = 0.0f;
		LastAlphaSpeedStep = 0.0f;
		PreviousNearCheckNearClipSmall = false;
	}

	float TopAlphaSpeed = 0.15f;
	float AlphaSpeedStep = 0.015f;

	float zoomvalue = TheCamera.CarZoomValueSmooth;
	if(zoomvalue < 0.1f)
		zoomvalue = 0.1f;
	if(TheCamera.CarZoomIndicator == 1.0f)
		ModeAlpha = CGeneral::GetATanOfXY(23.0f, zoomvalue);	// near
	else if(TheCamera.CarZoomIndicator == 2.0f)
		ModeAlpha = CGeneral::GetATanOfXY(10.8f, zoomvalue);	// mid
	else if(TheCamera.CarZoomIndicator == 3.0f)
		ModeAlpha = CGeneral::GetATanOfXY(7.0f, zoomvalue);	// far


	float Length = (Source - TargetCoors).Magnitude2D();
	if(m_bCollisionChecksOn){	// there's another variable (on PC) but it's uninitialised
		CVector Forward = CamTargetEntity->GetForward();
		float CarAlpha = CGeneral::GetATanOfXY(Forward.Magnitude2D(), Forward.z);
		// this shouldn't be necessary....
		while(CarAlpha >= PI) CarAlpha -= 2*PI;
		while(CarAlpha < -PI) CarAlpha += 2*PI;

		while(Beta >= PI) Beta -= 2*PI;
		while(Beta < -PI) Beta += 2*PI;

		float deltaBeta = Beta - TargetOrientation;
		while(deltaBeta >= PI) deltaBeta -= 2*PI;
		while(deltaBeta < -PI) deltaBeta += 2*PI;

		float BehindCarNess = Cos(deltaBeta);	// 1 if behind car, 0 if side, -1 if in front
		CarAlpha = -CarAlpha * BehindCarNess;
		if(CarAlpha < -0.01f)
			CarAlpha = -0.01f;

		float DeltaAlpha = CarAlpha - Alpha;
		while(DeltaAlpha >= PI) DeltaAlpha -= 2*PI;
		while(DeltaAlpha < -PI) DeltaAlpha += 2*PI;
		// What's this?? wouldn't it make more sense to clamp?
		float AngleLimit = DEGTORAD(1.8f);
		if(DeltaAlpha < -AngleLimit)
			DeltaAlpha += AngleLimit;
		else if(DeltaAlpha > AngleLimit)
			DeltaAlpha -= AngleLimit;
		else
			DeltaAlpha = 0.0f;

		// Now the collision

		float TargetAlpha = 0.0f;
		bool FoundRoofCenter = false;
		bool FoundRoofSide1 = false;
		bool FoundRoofSide2 = false;
		bool FoundCamRoof = false;
		bool FoundCamGround = false;
		float CamRoof = 0.0f;
		float CarBottom = TargetCoors.z - TargetHeight/2.0f;

		// Check car center
		float CarRoof = CWorld::FindRoofZFor3DCoord(TargetCoors.x, TargetCoors.y, CarBottom, &FoundRoofCenter);

		// Check sides of the car
		Forward = CamTargetEntity->GetForward();	// we actually still have that...
		Forward.Normalise();	// shouldn't be necessary
		float CarSideAngle = CGeneral::GetATanOfXY(Forward.x, Forward.y) + PI/2.0f;
		float SideX = 2.5f * Cos(CarSideAngle);
		float SideY = 2.5f * Sin(CarSideAngle);
		CWorld::FindRoofZFor3DCoord(TargetCoors.x + SideX, TargetCoors.y + SideY, CarBottom, &FoundRoofSide1);
		CWorld::FindRoofZFor3DCoord(TargetCoors.x - SideX, TargetCoors.y - SideY, CarBottom, &FoundRoofSide2);

		// Now find out at what height we'd like to place the camera
		float CamGround = CWorld::FindGroundZFor3DCoord(Source.x, Source.y, TargetCoors.z + Length*Sin(Alpha + ModeAlpha) + m_fCloseInCarHeightOffset, &FoundCamGround);
		float CamTargetZ = 0.0f;
		if(FoundCamGround){
			// This is the normal case
			CamRoof = CWorld::FindRoofZFor3DCoord(Source.x, Source.y, CamGround + TargetHeight, &FoundCamRoof);
			CamTargetZ = CamGround + TargetHeight*1.5f + 0.1f;
		}else{
			FoundCamRoof = false;
			CamTargetZ = TargetCoors.z;
		}

		if(FoundRoofCenter && !FoundCamRoof && (FoundRoofSide1 || FoundRoofSide2)){
			// Car is under something but camera isn't
			// This seems weird...
			TargetAlpha = CGeneral::GetATanOfXY(CA_MAX_DISTANCE, CarRoof - CamTargetZ - 1.5f);
			CamClear = false;
		}
		if(FoundCamRoof){
			// Camera is under something
			float roof = FoundRoofCenter ? min(CamRoof, CarRoof) : CamRoof;
			// Same weirdness again?
			TargetAlpha = CGeneral::GetATanOfXY(CA_MAX_DISTANCE, roof - CamTargetZ - 1.5f);
			CamClear = false;
		}
		while(TargetAlpha >= PI) TargetAlpha -= 2*PI;
		while(TargetAlpha < -PI) TargetAlpha += 2*PI;
		if(TargetAlpha < DEGTORAD(-7.0f))
			TargetAlpha = DEGTORAD(-7.0f);

		// huh?
		if(TargetAlpha > ModeAlpha)
			CamClear = true;
		// Camera is contrained by collision in some way
		PreviousNearCheckNearClipSmall = false;
		if(!CamClear){
			PreviousNearCheckNearClipSmall = true;
			RwCameraSetNearClipPlane(Scene.camera, 0.9f);

			DeltaAlpha = TargetAlpha - (Alpha + ModeAlpha);
			while(DeltaAlpha >= PI) DeltaAlpha -= 2*PI;
			while(DeltaAlpha < -PI) DeltaAlpha += 2*PI;

			TopAlphaSpeed = 0.3f;
			AlphaSpeedStep = 0.03f;
		}

		// Now do things if CamClear...but what is that anyway?
		float CamZ = TargetCoors.z + Length*Sin(Alpha + DeltaAlpha + ModeAlpha) + m_fCloseInCarHeightOffset;
		bool FoundGround, FoundRoof;
		float CamGround2 = CWorld::FindGroundZFor3DCoord(Source.x, Source.y, CamZ, &FoundGround);
		if(FoundGround){
			if(CamClear)
				if(CamZ - CamGround2 < 1.5f){
					PreviousNearCheckNearClipSmall = true;
					RwCameraSetNearClipPlane(Scene.camera, 0.9f);

					float a;
					if(Length == 0.0f || CamGround2 + 1.5f - TargetCoors.z == 0.0f)
						a = Alpha;
					else
						a = CGeneral::GetATanOfXY(Length, CamGround2 + 1.5f - TargetCoors.z);
					while(a > PI) a -= 2*PI;
					while(a < -PI) a += 2*PI;
					DeltaAlpha = a - Alpha;
				}
		}else{
			if(CamClear){
				float CamRoof2 = CWorld::FindRoofZFor3DCoord(Source.x, Source.y, CamZ, &FoundRoof);
				if(FoundRoof && CamZ - CamRoof2 < 1.5f){
					PreviousNearCheckNearClipSmall = true;
					RwCameraSetNearClipPlane(Scene.camera, 0.9f);

					if(CamRoof2 > TargetCoors.z + 3.5f)
						CamRoof2 = TargetCoors.z + 3.5f;

					float a;
					if(Length == 0.0f || CamRoof2 + 1.5f - TargetCoors.z == 0.0f)
						a = Alpha;
					else
						a = CGeneral::GetATanOfXY(Length, CamRoof2 + 1.5f - TargetCoors.z);
					while(a > PI) a -= 2*PI;
					while(a < -PI) a += 2*PI;
					DeltaAlpha = a - Alpha;
				}
			}
		}

		LastTargetAlphaWithCollisionOn = DeltaAlpha + Alpha;
		LastTopAlphaSpeed = TopAlphaSpeed;
		LastAlphaSpeedStep = AlphaSpeedStep;
	}else{
		if(PreviousNearCheckNearClipSmall)
			RwCameraSetNearClipPlane(Scene.camera, 0.9f);
	}

	WellBufferMe(LastTargetAlphaWithCollisionOn, &Alpha, &AlphaSpeed, LastTopAlphaSpeed, LastAlphaSpeedStep, true);

	Source.z = TargetCoors.z + Sin(Alpha + ModeAlpha)*Length + m_fCloseInCarHeightOffset;
}

// Rotate cam behind the car when the car is moving forward
bool
CCam::RotCamIfInFrontCar(CVector &TargetCoors, float TargetOrientation)
{
	bool MovingForward = false;
	CPhysical *phys = (CPhysical*)CamTargetEntity;

	float ForwardSpeed = DotProduct(phys->GetForward(), phys->GetSpeed(CVector(0.0f, 0.0f, 0.0f)));
	if(ForwardSpeed > 0.02f)
		MovingForward = true;

	float Dist = (Source - TargetCoors).Magnitude2D();

	float DeltaBeta = TargetOrientation - Beta;
	while(DeltaBeta >= PI) DeltaBeta -= 2*PI;
	while(DeltaBeta < -PI) DeltaBeta += 2*PI;

	if(Abs(DeltaBeta) > DEGTORAD(20.0f) && MovingForward && TheCamera.m_uiTransitionState == 0)
		m_bFixingBeta = true;

	CPad *pad = CPad::GetPad(0);
	if(!(pad->GetLookBehindForCar() || pad->GetLookBehindForPed() || pad->GetLookLeft() || pad->GetLookRight()))
		if(DirectionWasLooking != LOOKING_FORWARD)
			TheCamera.m_bCamDirectlyBehind = true;

	if(!m_bFixingBeta && !TheCamera.m_bUseTransitionBeta && !TheCamera.m_bCamDirectlyBehind && !TheCamera.m_bCamDirectlyInFront)
		return false;

	bool SetBeta = false;
	if(TheCamera.m_bCamDirectlyBehind || TheCamera.m_bCamDirectlyInFront || TheCamera.m_bUseTransitionBeta)
		if(&TheCamera.Cams[TheCamera.ActiveCam] == this)
			SetBeta = true;

	if(m_bFixingBeta || SetBeta){
		WellBufferMe(TargetOrientation, &Beta, &BetaSpeed, 0.15f, 0.007f, true);

		if(TheCamera.m_bCamDirectlyBehind && &TheCamera.Cams[TheCamera.ActiveCam] == this)
			Beta = TargetOrientation;
		if(TheCamera.m_bCamDirectlyInFront && &TheCamera.Cams[TheCamera.ActiveCam] == this)
			Beta = TargetOrientation + PI;
		if(TheCamera.m_bUseTransitionBeta && &TheCamera.Cams[TheCamera.ActiveCam] == this)
			Beta = m_fTransitionBeta;

		Source.x = TargetCoors.x - Cos(Beta)*Dist;
		Source.y = TargetCoors.y - Sin(Beta)*Dist;

		// Check if we're done
		DeltaBeta = TargetOrientation - Beta;
		while(DeltaBeta >= PI) DeltaBeta -= 2*PI;
		while(DeltaBeta < -PI) DeltaBeta += 2*PI;
		if(Abs(DeltaBeta) < DEGTORAD(2.0f))
			m_bFixingBeta = false;
	}
	TheCamera.m_bCamDirectlyBehind = false;
	TheCamera.m_bCamDirectlyInFront = false;
	return true;
}

// Move the cam to avoid clipping through buildings
bool
CCam::FixCamIfObscured(CVector &TargetCoors, float TargetHeight, float TargetOrientation)
{
	CVector Target = TargetCoors;
	bool UseEntityPos = false;
	CVector EntityPos;
	static CColPoint colPoint;
	static bool LastObscured = false;

	if(Mode == MODE_BEHINDCAR)
		Target.z += TargetHeight/2.0f;
	if(Mode == MODE_CAM_ON_A_STRING){
		UseEntityPos = true;
		Target.z += TargetHeight/2.0f;
		EntityPos = CamTargetEntity->GetPosition();
	}

	CVector TempSource = Source;

	bool Obscured1 = false;
	bool Obscured2 = false;
	bool Fix1 = false;
	float Dist1 = 0.0f;
	float Dist2 = 0.0f;
	CEntity *ent;
	if(m_bCollisionChecksOn || LastObscured){
		Obscured1 = CWorld::ProcessLineOfSight(Target, TempSource, colPoint, ent, true, false, false, true, false, true, true);
		if(Obscured1){
			Dist1 = (Target - colPoint.point).Magnitude2D();
			Fix1 = true;
			if(UseEntityPos)
				Obscured1 = CWorld::ProcessLineOfSight(EntityPos, TempSource, colPoint, ent, true, false, false, true, false, true, true);
		}else if(m_bFixingBeta){
			float d = (TempSource - Target).Magnitude();
			TempSource.x = Target.x - d*Cos(TargetOrientation);
			TempSource.y = Target.y - d*Sin(TargetOrientation);

			// same check again
			Obscured2 = CWorld::ProcessLineOfSight(Target, TempSource, colPoint, ent, true, false, false, true, false, true, true);
			if(Obscured2){
				Dist2 = (Target - colPoint.point).Magnitude2D();
				if(UseEntityPos)
					Obscured2 = CWorld::ProcessLineOfSight(EntityPos, TempSource, colPoint, ent, true, false, false, true, false, true, true);
			}
		}
		LastObscured = Obscured1 || Obscured2;
	}

	// nothing to do
	if(!LastObscured)
		return false;

	if(Fix1){
		Source.x = Target.x - Cos(Beta)*Dist1;
		Source.y = Target.y - Sin(Beta)*Dist1;
		if(Mode == MODE_BEHINDCAR)
			Source = colPoint.point;
	}else{
		WellBufferMe(Dist2, &m_fDistanceBeforeChanges, &DistanceSpeed, 0.2f, 0.025f, false);
		Source.x = Target.x - Cos(Beta)*m_fDistanceBeforeChanges;
		Source.y = Target.y - Sin(Beta)*m_fDistanceBeforeChanges;
	}

	if(ResetStatics){
		m_fDistanceBeforeChanges = (Source - Target).Magnitude2D();
		DistanceSpeed = 0.0f;
		Source.x = colPoint.point.x;
		Source.y = colPoint.point.y;
	}
	return true;
}

void
CCam::Process_Cam_On_A_String(const CVector &CameraTarget, float TargetOrientation, float, float)
{
	if(!CamTargetEntity->IsVehicle())
		return;

	FOV = DefaultFOV;

	if(ResetStatics){
		AlphaSpeed = 0.0f;
		if(TheCamera.m_bIdleOn)
			TheCamera.m_uiTimeWeEnteredIdle = CTimer::GetTimeInMilliseconds();
	}

	CBaseModelInfo *mi = CModelInfo::GetModelInfo(CamTargetEntity->GetModelIndex());
	CVector Dimensions = mi->GetColModel()->boundingBox.max - mi->GetColModel()->boundingBox.min;
	float BaseDist = Dimensions.Magnitude2D();

	CVector TargetCoors = CameraTarget;
	TargetCoors.z += Dimensions.z - 0.1f;	// final
	Beta = CGeneral::GetATanOfXY(TargetCoors.x - Source.x, TargetCoors.y - Source.y);
	while(Alpha >= PI) Alpha -= 2*PI;
	while(Alpha < -PI) Alpha += 2*PI;
	while(Beta >= PI) Beta -= 2*PI;
	while(Beta < -PI) Beta += 2*PI;

	m_fDistanceBeforeChanges = (Source - TargetCoors).Magnitude2D();

	Cam_On_A_String_Unobscured(TargetCoors, BaseDist);
	WorkOutCamHeight(TargetCoors, TargetOrientation, Dimensions.z);
	RotCamIfInFrontCar(TargetCoors, TargetOrientation);
	FixCamIfObscured(TargetCoors, Dimensions.z, TargetOrientation);
	FixCamWhenObscuredByVehicle(TargetCoors);

	m_cvecTargetCoorsForFudgeInter = TargetCoors;
	Front = TargetCoors - Source;
	Front.Normalise();
	GetVectorsReadyForRW();
	ResetStatics = false;
}

// Basic Cam on a string algorithm
void
CCam::Cam_On_A_String_Unobscured(const CVector &TargetCoors, float BaseDist)
{
	CA_MAX_DISTANCE = BaseDist + 0.1f + TheCamera.CarZoomValueSmooth;
	CA_MIN_DISTANCE = min(BaseDist*0.6f, 3.5f);

	CVector Dist = Source - TargetCoors;

	if(ResetStatics)
		Source = TargetCoors + Dist*(CA_MAX_DISTANCE + 1.0f);

	float Length = Dist.Magnitude2D();
	if(Length < 0.001f){
		// This probably shouldn't happen. reset view
		CVector Forward = CamTargetEntity->GetForward();
		Forward.z = 0.0f;
		Forward.Normalise();
		Source = TargetCoors - Forward*CA_MAX_DISTANCE;
		Dist = Source - TargetCoors;
		Length = Dist.Magnitude2D();
	}

	if(Length > CA_MAX_DISTANCE){
		Source.x = TargetCoors.x + Dist.x/Length * CA_MAX_DISTANCE;
		Source.y = TargetCoors.y + Dist.y/Length * CA_MAX_DISTANCE;
	}else if(Length < CA_MIN_DISTANCE){
		Source.x = TargetCoors.x + Dist.x/Length * CA_MIN_DISTANCE;
		Source.y = TargetCoors.y + Dist.y/Length * CA_MIN_DISTANCE;
	}
}

void
CCam::FixCamWhenObscuredByVehicle(const CVector &TargetCoors)
{
	// BUG? is this never reset
	static float HeightFixerCarsObscuring = 0.0f;
	static float HeightFixerCarsObscuringSpeed = 0.0f;
	CColPoint colPoint;
	CEntity *entity;

	float HeightTarget = 0.0f;
	if(CWorld::ProcessLineOfSight(TargetCoors, Source, colPoint, entity, false, true, false, false, false, false, false)){
		CBaseModelInfo *mi = CModelInfo::GetModelInfo(entity->GetModelIndex());
		HeightTarget = mi->GetColModel()->boundingBox.max.z + 1.0f + TargetCoors.z - Source.z;
		if(HeightTarget < 0.0f)
			HeightTarget = 0.0f;
	}
	WellBufferMe(HeightTarget, &HeightFixerCarsObscuring, &HeightFixerCarsObscuringSpeed, 0.2f, 0.025f, false);
	Source.z += HeightFixerCarsObscuring;
}

bool
CCam::Using3rdPersonMouseCam() 
{
	return CCamera::m_bUseMouse3rdPerson &&
		(Mode == MODE_FOLLOWPED ||
			TheCamera.m_bPlayerIsInGarage &&
			FindPlayerPed() && FindPlayerPed()->m_nPedState != PED_DRIVING &&
			Mode != MODE_TOPDOWN && this->CamTargetEntity == FindPlayerPed());
}

bool
CCam::GetWeaponFirstPersonOn()
{
	CEntity *target = this->CamTargetEntity;
	if (target && target->IsPed())
		return ((CPed*)target)->GetWeapon()->m_bAddRotOffset;
	
	return false;
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


	InjectHook(0x456F40, WellBufferMe, PATCH_JUMP);
	InjectHook(0x4582F0, &CCam::GetVectorsReadyForRW, PATCH_JUMP);
	InjectHook(0x457710, &CCam::DoAverageOnVector, PATCH_JUMP);
	InjectHook(0x458060, &CCam::GetPedBetaAngleForClearView, PATCH_JUMP);
	InjectHook(0x457210, &CCam::Cam_On_A_String_Unobscured, PATCH_JUMP);
	InjectHook(0x457A80, &CCam::FixCamWhenObscuredByVehicle, PATCH_JUMP);
	InjectHook(0x457B90, &CCam::FixCamIfObscured, PATCH_JUMP);
	InjectHook(0x465DA0, &CCam::RotCamIfInFrontCar, PATCH_JUMP);
	InjectHook(0x4662D0, &CCam::WorkOutCamHeightWeeCar, PATCH_JUMP);
	InjectHook(0x466650, &CCam::WorkOutCamHeight, PATCH_JUMP);

	InjectHook(0x45E3A0, &CCam::Process_FollowPed, PATCH_JUMP);
	InjectHook(0x45BE60, &CCam::Process_BehindCar, PATCH_JUMP);
	InjectHook(0x45C090, &CCam::Process_Cam_On_A_String, PATCH_JUMP);

	InjectHook(0x473250, &CCamera::dtor, PATCH_JUMP);
ENDPATCHES
