#pragma once

#include "Pools.h"
#include "World.h"

#ifdef FIX_BUGS
#ifndef DONT_FIX_REPLAY_BUGS
#define FIX_REPLAY_BUGS
#endif
#endif

class CVehicle;
struct CReference;

struct CAddressInReplayBuffer
{
	uint32 m_nOffset;
	uint8 *m_pBase;
	uint8 m_bSlot;
};

struct CStoredAnimationState
{
	uint8 animId;
	uint8 time;
	uint8 speed;
	uint8 secAnimId;
	uint8 secTime;
	uint8 secSpeed;
	uint8 blendAmount;
	uint8 partAnimId;
	uint8 partAnimTime;
	uint8 partAnimSpeed;
	uint8 partBlendAmount;
};

enum {
	NUM_MAIN_ANIMS_IN_REPLAY = 3,
	NUM_PARTIAL_ANIMS_IN_REPLAY = 6
};

struct CStoredDetailedAnimationState
{
	uint8 aAnimId[NUM_MAIN_ANIMS_IN_REPLAY];
	uint8 aCurTime[NUM_MAIN_ANIMS_IN_REPLAY];
	uint8 aSpeed[NUM_MAIN_ANIMS_IN_REPLAY];
	uint8 aBlendAmount[NUM_MAIN_ANIMS_IN_REPLAY];
#ifdef FIX_REPLAY_BUGS
	int8 aBlendDelta[NUM_MAIN_ANIMS_IN_REPLAY];
#endif
	uint8 aFunctionCallbackID[NUM_MAIN_ANIMS_IN_REPLAY];
	uint16 aFlags[NUM_MAIN_ANIMS_IN_REPLAY];
	uint8 aAnimId2[NUM_PARTIAL_ANIMS_IN_REPLAY];
	uint8 aCurTime2[NUM_PARTIAL_ANIMS_IN_REPLAY];
	uint8 aSpeed2[NUM_PARTIAL_ANIMS_IN_REPLAY];
	uint8 aBlendAmount2[NUM_PARTIAL_ANIMS_IN_REPLAY];
#ifdef FIX_REPLAY_BUGS
	int8 aBlendDelta2[NUM_PARTIAL_ANIMS_IN_REPLAY];
#endif
	uint8 aFunctionCallbackID2[NUM_PARTIAL_ANIMS_IN_REPLAY];
	uint16 aFlags2[NUM_PARTIAL_ANIMS_IN_REPLAY];
};

#ifdef GTA_REPLAY
#define REPLAY_STUB
#else
#define REPLAY_STUB {}
#endif

class CReplay
{
	enum {
		MODE_RECORD = 0,
		MODE_PLAYBACK = 1
	};

	enum {
		REPLAYCAMMODE_ASSTORED = 0,
		REPLAYCAMMODE_TOPDOWN = 1,
		REPLAYCAMMODE_FIXED = 2
	};

	enum {
		REPLAYPACKET_END = 0,
		REPLAYPACKET_VEHICLE = 1,
		REPLAYPACKET_PED_HEADER = 2,
		REPLAYPACKET_PED_UPDATE = 3,
		REPLAYPACKET_GENERAL = 4,
		REPLAYPACKET_CLOCK = 5,
		REPLAYPACKET_WEATHER = 6,
		REPLAYPACKET_ENDOFFRAME = 7,
		REPLAYPACKET_TIMER = 8,
		REPLAYPACKET_BULLET_TRACES = 9
	};

	enum {
		REPLAYBUFFER_UNUSED = 0,
		REPLAYBUFFER_PLAYBACK = 1,
		REPLAYBUFFER_RECORD = 2
	};

	enum {
		NUM_REPLAYBUFFERS = 8,
		REPLAYBUFFERSIZE = 100000
	};


	struct tGeneralPacket
	{
		uint8 type;
		bool in_rcvehicle;
		CMatrix camera_pos;
		CVector player_pos;
	};

	VALIDATE_SIZE(tGeneralPacket, 88);

	struct tClockPacket
	{
		uint8 type;
		uint8 hours;
		uint8 minutes;
	private:
		uint8 __align;
	};
	VALIDATE_SIZE(tClockPacket, 4);

	struct tWeatherPacket
	{
		uint8 type;
		uint8 old_weather;
		uint8 new_weather;
		float interpolation;
	};
	VALIDATE_SIZE(tWeatherPacket, 8);

	struct tTimerPacket
	{
		uint8 type;
		uint32 timer;
	};
	VALIDATE_SIZE(tTimerPacket, 8);

	struct tPedHeaderPacket
	{
		uint8 type;
		uint8 index;
		uint16 mi;
		uint8 pedtype;
	private:
		uint8 __align[3];
	};
	VALIDATE_SIZE(tPedHeaderPacket, 8);

	struct tBulletTracePacket
	{
		uint8 type;
		uint8 frames;
		uint8 lifetime;
		uint8 index;
		CVector inf;
		CVector sup;
	};
	VALIDATE_SIZE(tBulletTracePacket, 28);

	struct tEndOfFramePacket
	{
		uint8 type;
	private:
		uint8 __align[3];
	};
	VALIDATE_SIZE(tEndOfFramePacket, 4);

	struct tPedUpdatePacket
	{
		uint8 type;
		uint8 index;
		int8 heading;
		int8 vehicle_index;
		CStoredAnimationState anim_state;
		CCompressedMatrixNotAligned matrix;
		int8 assoc_group_id;
		uint8 weapon_model;
	};
	VALIDATE_SIZE(tPedUpdatePacket, 40);

	struct tVehicleUpdatePacket
	{
		uint8 type;
		uint8 index;
		uint8 health;
		uint8 acceleration;
		CCompressedMatrixNotAligned matrix;
		int8 door_angles[2];
		uint16 mi;
		uint32 panels;
		int8 velocityX;
		int8 velocityY;
		int8 velocityZ;
		union {
			int8 car_gun;
			int8 wheel_state;
		};
		uint8 wheel_susp_dist[4];
		uint8 wheel_rotation[4];
		uint8 door_status;
		uint8 primary_color;
		uint8 secondary_color;
	};
	VALIDATE_SIZE(tVehicleUpdatePacket, 48);

private:
	static uint8 Mode;
	static CAddressInReplayBuffer Record;
	static CAddressInReplayBuffer Playback;
	static uint8* pBuf0;
	static CAutomobile* pBuf1;
	static uint8* pBuf2;
	static CPlayerPed* pBuf3;
	static uint8* pBuf4;
	static CCutsceneHead* pBuf5;
	static uint8* pBuf6;
	static CPtrNode* pBuf7;
	static uint8* pBuf8;
	static CEntryInfoNode* pBuf9;
	static uint8* pBuf10;
	static CDummyPed* pBuf11;
	static uint8* pRadarBlips;
	static uint8* pStoredCam;
	static uint8* pWorld1;
	static CReference* pEmptyReferences;
	static CStoredDetailedAnimationState* pPedAnims;
	static uint8* pPickups;
	static uint8* pReferences;
	static uint8 BufferStatus[NUM_REPLAYBUFFERS];
	static uint8 Buffers[NUM_REPLAYBUFFERS][REPLAYBUFFERSIZE];
	static bool bPlayingBackFromFile;
	static bool bReplayEnabled;
	static uint32 SlowMotion;
	static uint32 FramesActiveLookAroundCam;
	static bool bDoLoadSceneWhenDone;
	static CPtrNode* WorldPtrList;
	static CPtrNode* BigBuildingPtrList;
	static CWanted PlayerWanted;
	static CPlayerInfo PlayerInfo;
	static uint32 Time1;
	static uint32 Time2;
	static uint32 Time3;
	static uint32 Time4;
	static uint32 Frame;
	static uint8 ClockHours;
	static uint8 ClockMinutes;
	static uint16 OldWeatherType;
	static uint16 NewWeatherType;
	static float WeatherInterpolationValue;
	static float TimeStepNonClipped;
	static float TimeStep;
	static float TimeScale;
	static float CameraFixedX;
	static float CameraFixedY;
	static float CameraFixedZ;
	static int32 OldRadioStation;
	static int8 CameraMode;
	static bool bAllowLookAroundCam;
	static float LoadSceneX;
	static float LoadSceneY;
	static float LoadSceneZ;
	static float CameraFocusX;
	static float CameraFocusY;
	static float CameraFocusZ;
	static bool bPlayerInRCBuggy;
	static float fDistanceLookAroundCam;
	static float fAlphaAngleLookAroundCam;
	static float fBetaAngleLookAroundCam;
#ifdef FIX_BUGS
	static uint8* pGarages;
	static CFire* FireArray;
	static uint32 NumOfFires;
	static uint8* paProjectileInfo;
	static uint8* paProjectiles;
	static int nHandleOfPlayerPed[NUMPLAYERS];
#endif

public:
	static void Init(void) REPLAY_STUB;
	static void DisableReplays(void) REPLAY_STUB;
	static void EnableReplays(void) REPLAY_STUB;
	static void Update(void) REPLAY_STUB;
	static void FinishPlayback(void) REPLAY_STUB;
	static void EmptyReplayBuffer(void) REPLAY_STUB;
	static void Display(void) REPLAY_STUB;
	static void TriggerPlayback(uint8 cam_mode, float cam_x, float cam_y, float cam_z, bool load_scene) REPLAY_STUB;
	static void StreamAllNecessaryCarsAndPeds(void) REPLAY_STUB;

#ifndef GTA_REPLAY
	static bool ShouldStandardCameraBeProcessed(void) { return true; }
	static bool IsPlayingBack() { return false; }
	static bool IsPlayingBackFromFile() { return false; }
#else
	static bool ShouldStandardCameraBeProcessed(void);
	static bool IsPlayingBack() { return Mode == MODE_PLAYBACK; }
	static bool IsPlayingBackFromFile() { return bPlayingBackFromFile; }
private:
	static void RecordThisFrame(void);
	static void StorePedUpdate(CPed *ped, int id);
	static void StorePedAnimation(CPed *ped, CStoredAnimationState *state);
	static void StoreDetailedPedAnimation(CPed *ped, CStoredDetailedAnimationState *state);
	static void ProcessPedUpdate(CPed *ped, float interpolation, CAddressInReplayBuffer *buffer);
	static void RetrievePedAnimation(CPed *ped, CStoredAnimationState *state);
	static void RetrieveDetailedPedAnimation(CPed *ped, CStoredDetailedAnimationState *state);
	static void PlaybackThisFrame(void);
	static void TriggerPlaybackLastCoupleOfSeconds(uint32, uint8, float, float, float, uint32);
	static bool FastForwardToTime(uint32);
	static void StoreCarUpdate(CVehicle *vehicle, int id);
	static void ProcessCarUpdate(CVehicle *vehicle, float interpolation, CAddressInReplayBuffer *buffer);
	static bool PlayBackThisFrameInterpolation(CAddressInReplayBuffer *buffer, float interpolation, uint32 *pTimer);
	static void ProcessReplayCamera(void);
	static void StoreStuffInMem(void);
	static void RestoreStuffFromMem(void);
	static void EmptyPedsAndVehiclePools(void);
	static void EmptyAllPools(void);
	static void MarkEverythingAsNew(void);
	static void SaveReplayToHD(void);
	static void PlayReplayFromHD(void); // out of class in III PC and later because of SecuROM
	static void FindFirstFocusCoordinate(CVector *coord);
	static void ProcessLookAroundCam(void);
	static size_t FindSizeOfPacket(uint8);
#endif
};
