#pragma once
#include "Placeable.h"

class CEntity;
class CPed;
class CAutomobile;

extern int16 &DebugCamMode;

#define NUMBER_OF_VECTORS_FOR_AVERAGE 2

struct CCam
{
	enum
	{
		MODE_NONE = 0,
		MODE_TOPDOWN,
		MODE_GTACLASSIC,
		MODE_BEHINDCAR,
		MODE_FOLLOWPED,
		MODE_AIMING,
		MODE_DEBUG,
		MODE_SNIPER,
		MODE_ROCKETLAUNCHER,
		MODE_MODELVIEW,
		MODE_BILL,
		MODE_SYPHON,
		MODE_CIRCLE,
		MODE_CHEESYZOOM,
		MODE_WHEELCAM,
		MODE_FIXED,
		MODE_1STPERSON,
		MODE_FLYBY,
		MODE_CAM_ON_A_STRING,
		MODE_REACTION,
		MODE_FOLLOW_PED_WITH_BIND,
		MODE_CHRIS,
		MODE_BEHINDBOAT,
		MODE_PLAYER_FALLEN_WATER,
		MODE_CAM_ON_TRAIN_ROOF,
		MODE_CAM_RUNNING_SIDE_TRAIN,
		MODE_BLOOD_ON_THE_TRACKS,
		MODE_IM_THE_PASSENGER_WOOWOO,
		MODE_SYPHON_CRIM_IN_FRONT,
		MODE_PED_DEAD_BABY,
		MODE_PILLOWS_PAPS,
		MODE_LOOK_AT_CARS,
		MODE_ARRESTCAM_ONE,
		MODE_ARRESTCAM_TWO,
		MODE_M16_1STPERSON,
		MODE_SPECIAL_FIXED_FOR_SYPHON,
		MODE_FIGHT_CAM,
		MODE_TOP_DOWN_PED,
		MODE_SNIPER_RUNABOUT,
		MODE_ROCKETLAUNCHER_RUNABOUT,
		MODE_1STPERSON_RUNABOUT,
		MODE_M16_1STPERSON_RUNABOUT,
		MODE_FIGHT_CAM_RUNABOUT,
		MODE_EDITOR,
		MODE_HELICANNON_1STPERSON, // vice city leftover
	};

	bool    bBelowMinDist; //used for follow ped mode
	bool    bBehindPlayerDesired; //used for follow ped mode
	bool    m_bCamLookingAtVector;
	bool    m_bCollisionChecksOn;
	bool    m_bFixingBeta; //used for camera on a string
	bool    m_bTheHeightFixerVehicleIsATrain;
	bool    LookBehindCamWasInFront;
	bool    LookingBehind;
	bool    LookingLeft;
	bool    LookingRight;
	bool    ResetStatics; //for interpolation type stuff to work
	bool    Rotating;

	int16   Mode;                   // CameraMode
	uint32  m_uiFinishTime;

	int     m_iDoCollisionChecksOnFrameNum; 
	int     m_iDoCollisionCheckEveryNumOfFrames;
	int     m_iFrameNumWereAt;
	int     m_iRunningVectorArrayPos;
	int     m_iRunningVectorCounter;
	int     DirectionWasLooking;

	float   f_max_role_angle; //=DEGTORAD(5.0f);    
	float   f_Roll; //used for adding a slight roll to the camera in the
	float	f_rollSpeed;
	float   m_fSyphonModeTargetZOffSet;
	float	m_fRoadOffSet;
	float   m_fAmountFractionObscured;
	float   m_fAlphaSpeedOverOneFrame;
	float   m_fBetaSpeedOverOneFrame;
	float   m_fBufferedTargetBeta;
	float   m_fBufferedTargetOrientation;
	float   m_fBufferedTargetOrientationSpeed;
	float   m_fCamBufferedHeight;
	float   m_fCamBufferedHeightSpeed;
	float   m_fCloseInPedHeightOffset;
	float   m_fCloseInPedHeightOffsetSpeed;
	float   m_fCloseInCarHeightOffset;
	float   m_fCloseInCarHeightOffsetSpeed;
	float   m_fDimensionOfHighestNearCar;       
	float   m_fDistanceBeforeChanges;
	float   m_fFovSpeedOverOneFrame;
	float   m_fMinDistAwayFromCamWhenInterPolating;
	float   m_fPedBetweenCameraHeightOffset;
	float   m_fPlayerInFrontSyphonAngleOffSet;
	float   m_fRadiusForDead;
	float   m_fRealGroundDist; //used for follow ped mode
	float   m_fTargetBeta;
	float   m_fTimeElapsedFloat;  
  
	float   m_fTransitionBeta;
	float   m_fTrueBeta;
	float   m_fTrueAlpha;
	float   m_fInitialPlayerOrientation; //used for first person

	float   Alpha;
	float   AlphaSpeed;
	float   FOV;
	float   FOVSpeed;
	float   Beta;
	float   BetaSpeed;
	float   Distance;
	float   DistanceSpeed;
	float   CA_MIN_DISTANCE;
	float   CA_MAX_DISTANCE;
	float   SpeedVar;

	CVector m_cvecSourceSpeedOverOneFrame;
	CVector m_cvecTargetSpeedOverOneFrame;
	CVector m_cvecUpOverOneFrame;

	CVector m_cvecTargetCoorsForFudgeInter;
	CVector m_cvecCamFixedModeVector;
	CVector m_cvecCamFixedModeSource;
	CVector m_cvecCamFixedModeUpOffSet;
	CVector m_vecLastAboveWaterCamPosition;  //helper for when the player has gone under the water
	CVector m_vecBufferedPlayerBodyOffset;

	// The three vectors that determine this camera for this frame
	CVector Front;                                                  // Direction of looking in
	CVector Source;                                                 // Coors in world space
	CVector SourceBeforeLookBehind;
	CVector Up;                                                     // Just that
	CVector m_arrPreviousVectors[NUMBER_OF_VECTORS_FOR_AVERAGE];    // used to average stuff
	CEntity *CamTargetEntity;

	float       m_fCameraDistance;
	float       m_fIdealAlpha;
	float       m_fPlayerVelocity;
	CAutomobile  *m_pLastCarEntered; // So interpolation works
	CPed         *m_pLastPedLookedAt;// So interpolation works 
	bool        m_bFirstPersonRunAboutActive;


	CCam(void) { Init(); }
	void Init(void);
	void Process(void);
	void ProcessSpecialHeightRoutines(void);
	void GetVectorsReadyForRW(void);
	CVector DoAverageOnVector(const CVector &vec);
	float GetPedBetaAngleForClearView(const CVector &Target, float Dist, float BetaOffset, bool checkBuildings, bool checkVehicles, bool checkPeds, bool checkObjects, bool checkDummies);
	void WorkOutCamHeightWeeCar(CVector &TargetCoors, float TargetOrientation);
	void WorkOutCamHeight(const CVector &TargetCoors, float TargetOrientation, float TargetHeight);
	bool RotCamIfInFrontCar(CVector &TargetCoors, float TargetOrientation);
	bool FixCamIfObscured(CVector &TargetCoors, float TargetHeight, float TargetOrientation);
	void Cam_On_A_String_Unobscured(const CVector &TargetCoors, float BaseDist);
	void FixCamWhenObscuredByVehicle(const CVector &TargetCoors);
	void LookBehind(void);
	void LookLeft(void);
	void LookRight(void);
	void ClipIfPedInFrontOfPlayer(void);
	void KeepTrackOfTheSpeed(const CVector &source, const CVector &target, const CVector &up, const float &alpha, const float &beta, const float &fov);
	bool Using3rdPersonMouseCam(void);
	bool GetWeaponFirstPersonOn(void);
	bool IsTargetInWater(const CVector &CamCoors);
	void AvoidWallsTopDownPed(const CVector &TargetCoors, const CVector &Offset, float *Adjuster, float *AdjusterSpeed, float yDistLimit);
	void PrintMode(void);

	void Process_Debug(const CVector&, float, float, float);
	void Process_Editor(const CVector&, float, float, float);
	void Process_ModelView(const CVector &CameraTarget, float, float, float);
	void Process_FollowPed(const CVector &CameraTarget, float TargetOrientation, float, float);
	void Process_FollowPedWithMouse(const CVector &CameraTarget, float TargetOrientation, float, float);
	void Process_BehindCar(const CVector &CameraTarget, float TargetOrientation, float, float);
	void Process_Cam_On_A_String(const CVector &CameraTarget, float TargetOrientation, float, float);
	void Process_TopDown(const CVector &CameraTarget, float TargetOrientation, float SpeedVar, float TargetSpeedVar);
	void Process_TopDownPed(const CVector &CameraTarget, float TargetOrientation, float, float);
	void Process_Rocket(const CVector &CameraTarget, float, float, float);
	void Process_M16_1stPerson(const CVector &CameraTarget, float, float, float);
	void Process_1stPerson(const CVector &CameraTarget, float, float, float);
	void Process_1rstPersonPedOnPC(const CVector &CameraTarget, float TargetOrientation, float, float);
	void Process_Sniper(const CVector &CameraTarget, float, float, float);
	void Process_Syphon(const CVector &CameraTarget, float, float, float);
	void Process_Syphon_Crim_In_Front(const CVector &CameraTarget, float, float, float);
	void Process_BehindBoat(const CVector &CameraTarget, float TargetOrientation, float, float);
	void Process_Fight_Cam(const CVector &CameraTarget, float TargetOrientation, float, float);
	void Process_FlyBy(const CVector&, float, float, float);
	void Process_WheelCam(const CVector&, float, float, float);
	void Process_Fixed(const CVector &CameraTarget, float, float, float);
	void Process_Player_Fallen_Water(const CVector &CameraTarget, float TargetOrientation, float, float);
	void Process_Circle(const CVector &CameraTarget, float, float, float);
	void Process_SpecialFixedForSyphon(const CVector &CameraTarget, float, float, float);
	void ProcessPedsDeadBaby(void);
	bool ProcessArrestCamOne(void);
	bool ProcessArrestCamTwo(void);

	/* Some of the unused PS2 cams */
	void Process_Chris_With_Binding_PlusRotation(const CVector &CameraTarget, float, float, float);
	void Process_ReactionCam(const CVector &CameraTarget, float TargetOrientation, float, float);
	void Process_FollowPed_WithBinding(const CVector &CameraTarget, float TargetOrientation, float, float);
	// TODO:
	// CCam::Process_CushyPillows_Arse
	// CCam::Process_Look_At_Cars
	// CCam::Process_CheesyZoom
	// CCam::Process_Aiming
	// CCam::Process_Bill	// same as BehindCar due to unused variables
	// CCam::Process_Im_The_Passenger_Woo_Woo
	// CCam::Process_Blood_On_The_Tracks
	// CCam::Process_Cam_Running_Side_Train
	// CCam::Process_Cam_On_Train_Roof
};
static_assert(sizeof(CCam) == 0x1A4, "CCam: wrong size");
static_assert(offsetof(CCam, Alpha) == 0xA8, "CCam: error");
static_assert(offsetof(CCam, Front) == 0x140, "CCam: error");

struct CCamPathSplines
{
	float m_arr_PathData[800];
};

struct CTrainCamNode
{
	CVector m_cvecCamPosition;
	CVector m_cvecPointToLookAt;
	CVector m_cvecMinPointInRange;
	CVector m_cvecMaxPointInRange;
	float m_fDesiredFOV;
	float m_fNearClip;
};

struct CQueuedMode
{
	int16 Mode;
	float Duration;
	int16 MinZoom;
	int16 MaxZoom;
};

enum
{
	LOOKING_BEHIND,
	LOOKING_LEFT,
	LOOKING_RIGHT,
	LOOKING_FORWARD,
};

enum
{
	// TODO: figure out
	FADE_0,
	FADE_1,	// mid fade
	FADE_2,

	FADE_OUT = 0,
	FADE_IN,
	FADE_NONE
};

enum
{
	MBLUR_NONE,
	MBLUR_SNIPER,
	MBLUR_NORMAL,
	MBLUR_INTRO1,		// green camera
	MBLUR_INTRO2,		// unused
	MBLUR_INTRO3,		// bank scene
	MBLUR_INTRO4,		// jail break scene
	MBLUR_INTRO5,		// explosion
	MBLUR_INTRO6,		// player shot
	MBLUR_UNUSED,		// pinkish
};

enum
{
	NONE = 0,
	INTERPOLATION,
	JUMP_CUT
};

enum
{
	CAM_CONTROLLER_0,
	CAM_CONTROLLER_1,
	CAM_CONTROLLER_2
};

struct CCamera : public CPlaceable
{
	bool m_bAboveGroundTrainNodesLoaded;
	bool m_bBelowGroundTrainNodesLoaded;
	bool m_bCamDirectlyBehind;
	bool m_bCamDirectlyInFront;
	bool m_bCameraJustRestored;
	bool m_bcutsceneFinished;
	bool m_bCullZoneChecksOn;
	bool m_bFirstPersonBeingUsed;
	bool m_bJustJumpedOutOf1stPersonBecauseOfTarget;
	bool m_bIdleOn;
	bool m_bInATunnelAndABigVehicle;
	bool m_bInitialNodeFound;
	bool m_bInitialNoNodeStaticsSet;
	bool m_bIgnoreFadingStuffForMusic;
	bool m_bPlayerIsInGarage;
	bool m_bJustCameOutOfGarage;
	bool m_bJustInitalised;
	bool m_bJust_Switched;
	bool m_bLookingAtPlayer;
	bool m_bLookingAtVector;
	bool m_bMoveCamToAvoidGeom;
	bool m_bObbeCinematicPedCamOn;
	bool m_bObbeCinematicCarCamOn;
	bool m_bRestoreByJumpCut;
	bool m_bUseNearClipScript;
	bool m_bStartInterScript;
	bool m_bStartingSpline;
	bool m_bTargetJustBeenOnTrain;
	bool m_bTargetJustCameOffTrain;
	bool m_bUseSpecialFovTrain;
	bool m_bUseTransitionBeta;
	bool m_bUseScriptZoomValuePed;
	bool m_bUseScriptZoomValueCar;
	bool m_bWaitForInterpolToFinish;
	bool m_bItsOkToLookJustAtThePlayer;
	bool m_bWantsToSwitchWidescreenOff;
	bool m_WideScreenOn;
	bool m_1rstPersonRunCloseToAWall;
	bool m_bHeadBob;
	bool m_bFailedCullZoneTestPreviously;

bool m_FadeTargetIsSplashScreen;

	bool WorldViewerBeingUsed;
	uint8 ActiveCam;
	uint32 m_uiCamShakeStart;
	uint32 m_uiFirstPersonCamLastInputTime;
// where are those?
//bool m_bVehicleSuspenHigh;
//bool m_bEnable1rstPersonCamCntrlsScript;
//bool m_bAllow1rstPersonWeaponsCamera;

	uint32 m_uiLongestTimeInMill;
	uint32 m_uiNumberOfTrainCamNodes;
	uint8   m_uiTransitionJUSTStarted;
	uint8   m_uiTransitionState;        // 0:one mode 1:transition

	uint32 m_uiTimeLastChange;
	uint32 m_uiTimeWeEnteredIdle;
	uint32 m_uiTimeTransitionStart;
	uint32 m_uiTransitionDuration;
	int m_BlurBlue;
	int m_BlurGreen;
	int m_BlurRed;
	int m_BlurType;

uint32    unknown;
	int m_iWorkOutSpeedThisNumFrames;
	int m_iNumFramesSoFar;


	int m_iCurrentTrainCamNode;
	int m_motionBlur;
	int m_imotionBlurAddAlpha;
	int m_iCheckCullZoneThisNumFrames;
	int m_iZoneCullFrameNumWereAt;
	int WhoIsInControlOfTheCamera;

	float CamFrontXNorm;
	float CamFrontYNorm;
	float CarZoomIndicator;
	float CarZoomValue;
	float CarZoomValueSmooth;

	float DistanceToWater;
	float FOVDuringInter;
	float LODDistMultiplier;
	float GenerationDistMultiplier;
	float m_fAlphaSpeedAtStartInter;
	float m_fAlphaWhenInterPol;
	float m_fAlphaDuringInterPol;
	float m_fBetaDuringInterPol;
	float m_fBetaSpeedAtStartInter;
	float m_fBetaWhenInterPol;
	float m_fFOVWhenInterPol;
	float m_fFOVSpeedAtStartInter;
	float m_fStartingBetaForInterPol;
	float m_fStartingAlphaForInterPol;
	float m_PedOrientForBehindOrInFront;
	float m_CameraAverageSpeed;
	float m_CameraSpeedSoFar;
	float m_fCamShakeForce;
	float m_fCarZoomValueScript;
	float m_fFovForTrain;
	float m_fFOV_Wide_Screen;
	float m_fNearClipScript;
	float m_fOldBetaDiff;
	float m_fPedZoomValue;

  float m_fPedZoomValueScript;
  float m_fPedZoomValueSmooth;
  float m_fPositionAlongSpline;
  float m_ScreenReductionPercentage;
  float m_ScreenReductionSpeed;
  float m_AlphaForPlayerAnim1rstPerson;
  float Orientation;
  float PedZoomIndicator;
  float PlayerExhaustion;
  float SoundDistUp, SoundDistLeft, SoundDistRight;
  float SoundDistUpAsRead, SoundDistLeftAsRead, SoundDistRightAsRead;
  float SoundDistUpAsReadOld, SoundDistLeftAsReadOld, SoundDistRightAsReadOld;
  float m_fWideScreenReductionAmount;
  float m_fStartingFOVForInterPol;

	// not static yet
	float m_fMouseAccelHorzntl;// acceleration multiplier for 1st person controls
	float m_fMouseAccelVertical;// acceleration multiplier for 1st person controls
	float m_f3rdPersonCHairMultX;
	float m_f3rdPersonCHairMultY;


	CCam Cams[3];
	void *pToGarageWeAreIn;
	void *pToGarageWeAreInForHackAvoidFirstPerson;
	CQueuedMode m_PlayerMode;
	CQueuedMode PlayerWeaponMode;
	CVector m_PreviousCameraPosition;
	CVector m_RealPreviousCameraPosition;
	CVector m_cvecAimingTargetCoors;
	CVector m_vecFixedModeVector;
	CVector m_vecFixedModeSource;
	CVector m_vecFixedModeUpOffSet;
	CVector m_vecCutSceneOffset;

  // one of those has to go
  CVector m_cvecStartingSourceForInterPol;
  CVector m_cvecStartingTargetForInterPol;
  CVector m_cvecStartingUpForInterPol;
  CVector m_cvecSourceSpeedAtStartInter;
  CVector m_cvecTargetSpeedAtStartInter;
  CVector m_cvecUpSpeedAtStartInter;
  CVector m_vecSourceWhenInterPol;
  CVector m_vecTargetWhenInterPol;
  CVector m_vecUpWhenInterPol;
  //CVector m_vecClearGeometryVec;

	CVector m_vecGameCamPos;
	CVector SourceDuringInter;
	CVector TargetDuringInter;
	CVector UpDuringInter;
	RwCamera *m_pRwCamera;
	CEntity *pTargetEntity;
	CCamPathSplines m_arrPathArray[4];
	CTrainCamNode m_arrTrainCamNode[800];
	CMatrix m_cameraMatrix;
	bool m_bGarageFixedCamPositionSet;
	bool m_vecDoingSpecialInterPolation;
	bool m_bScriptParametersSetForInterPol;
	bool m_bFading;
	bool m_bMusicFading;
	CMatrix m_viewMatrix;
	CVector m_vecFrustumNormals[4];
	CVector m_vecOldSourceForInter;
	CVector m_vecOldFrontForInter;
	CVector m_vecOldUpForInter;

	float m_vecOldFOVForInter;
	float m_fFLOATingFade;
	float m_fFLOATingFadeMusic;
	float m_fTimeToFadeOut;
	float m_fTimeToFadeMusic;
	float m_fFractionInterToStopMovingTarget;
	float m_fFractionInterToStopCatchUpTarget;
	float m_fGaitSwayBuffer;
	float m_fScriptPercentageInterToStopMoving;
	float m_fScriptPercentageInterToCatchUp;

uint32  m_fScriptTimeForInterPolation;


	int16   m_iFadingDirection;
	int     m_iModeObbeCamIsInForCar;
	int16   m_iModeToGoTo;
	int16   m_iMusicFadingDirection;
	int16   m_iTypeOfSwitch;

	uint32 m_uiFadeTimeStarted;
	uint32 m_uiFadeTimeStartedMusic;

	static bool &m_bUseMouse3rdPerson;

	bool Get_Just_Switched_Status() { return m_bJust_Switched; }
	inline const CMatrix& GetCameraMatrix(void) { return m_cameraMatrix; }
	CVector &GetGameCamPosition(void) { return m_vecGameCamPos; }
	float GetPositionAlongSpline(void) { return m_fPositionAlongSpline; }
	bool IsPointVisible(const CVector &center, const CMatrix *mat);
	bool IsSphereVisible(const CVector &center, float radius, const CMatrix *mat);
	bool IsSphereVisible(const CVector &center, float radius);
	bool IsBoxVisible(RwV3d *box, const CMatrix *mat);
	int GetLookDirection(void);
	bool GetLookingForwardFirstPerson(void);

	void Fade(float timeout, int16 direction);
	int GetScreenFadeStatus(void);
	void ProcessFade(void);
	void ProcessMusicFade(void);
	void SetFadeColour(uint8 r, uint8 g, uint8 b);

	void CamShake(float strength, float x, float y, float z);

	void SetMotionBlur(int r, int g, int b, int a, int type);
	void SetMotionBlurAlpha(int a);
	void RenderMotionBlur(void);
	void ClearPlayerWeaponMode();
	void CalculateDerivedValues(void);

	void DrawBordersForWideScreen(void);
	void Restore(void);
	void SetWideScreenOn(void);
	void SetWideScreenOff(void);
	void SetNearClipScript(float);

	float Find3rdPersonQuickAimPitch(void);

	void TakeControl(CEntity*, int16, int16, int32);
	void TakeControlNoEntity(const CVector&, int16, int32);
	void SetCamPositionForFixedMode(const CVector&, const CVector&);
	bool GetFading();
	int GetFadingDirection();

	void Init();
	void SetRwCamera(RwCamera*);
	void Process();

	void LoadPathSplines(int file);
	uint32 GetCutSceneFinishTime(void);
	void FinishCutscene(void);

	void SetCamCutSceneOffSet(const CVector&);
	void TakeControlWithSpline(short);
	void RestoreWithJumpCut(void);
	void SetCameraDirectlyInFrontForFollowPed_CamOnAString(void);
	void SetCameraDirectlyBehindForFollowPed_CamOnAString(void);
	void SetZoomValueFollowPedScript(int16);
	void SetZoomValueCamStringScript(int16);
	void SetNewPlayerWeaponMode(int16, int16, int16);
	void UpdateAimingCoors(CVector const &);

	void SetPercentAlongCutScene(float);
	void SetParametersForScriptInterpolation(float, float, int32);

	void dtor(void) { this->CCamera::~CCamera(); }
};
static_assert(offsetof(CCamera, DistanceToWater) == 0xe4, "CCamera: error");
static_assert(offsetof(CCamera, m_WideScreenOn) == 0x70, "CCamera: error");
static_assert(offsetof(CCamera, WorldViewerBeingUsed) == 0x75, "CCamera: error");
static_assert(offsetof(CCamera, m_uiNumberOfTrainCamNodes) == 0x84, "CCamera: error");
static_assert(offsetof(CCamera, m_uiTransitionState) == 0x89, "CCamera: error");
static_assert(offsetof(CCamera, m_uiTimeTransitionStart) == 0x94, "CCamera: error");
static_assert(offsetof(CCamera, m_BlurBlue) == 0x9C, "CCamera: error");
static_assert(offsetof(CCamera, Cams) == 0x1A4, "CCamera: error");
static_assert(offsetof(CCamera, pToGarageWeAreIn) == 0x690, "CCamera: error");
static_assert(offsetof(CCamera, m_PreviousCameraPosition) == 0x6B0, "CCamera: error");
static_assert(offsetof(CCamera, m_vecCutSceneOffset) == 0x6F8, "CCamera: error");
static_assert(offsetof(CCamera, m_arrPathArray) == 0x7a8, "CCamera: error");
static_assert(sizeof(CCamera) == 0xE9D8, "CCamera: wrong size");

extern CCamera &TheCamera;

void CamShakeNoPos(CCamera*, float);
