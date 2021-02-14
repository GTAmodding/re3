#pragma once

#include "audio_enums.h"
#include "AudioCollision.h"
#include "PoliceRadio.h"

class tSound
{
public:
	int32 m_nEntityIndex;
	int32 m_nCounter;
	int32 m_nSampleIndex;
	uint8 m_nBankIndex;
	bool m_bIs2D;
	int32 m_nReleasingVolumeModificator;
	uint32 m_nFrequency;
	uint8 m_nVolume;
	float m_fDistance;
	int32 m_nLoopCount;
	int32 m_nLoopStart;
	int32 m_nLoopEnd;
	uint8 m_nEmittingVolume;
	float m_fSpeedMultiplier;
	float m_fSoundIntensity;
	bool m_bReleasingSoundFlag;
	CVector m_vecPos;
	bool m_bReverbFlag;
	uint8 m_nLoopsRemaining;
	bool m_bRequireReflection; // Used for oneshots
	uint8 m_nOffset;
	int32 m_nReleasingVolumeDivider;
	bool m_bIsProcessed;
	bool m_bLoopEnded;
	int32 m_nCalculatedVolume;
	int8 m_nVolumeChange;
};

VALIDATE_SIZE(tSound, 92);

class CPhysical;
class CAutomobile;

class tAudioEntity
{
public:
	eAudioType m_nType;
	void *m_pEntity;
	bool m_bIsUsed;
	uint8 m_bStatus;
	int16 m_awAudioEvent[NUM_AUDIOENTITY_EVENTS];
	float m_afVolume[NUM_AUDIOENTITY_EVENTS];
	uint8 m_AudioEvents;
};

VALIDATE_SIZE(tAudioEntity, 40);

class tPedComment
{
public:
	int32 m_nSampleIndex;
	int32 m_nEntityIndex;
	CVector m_vecPos;
	float m_fDistance;
	uint8 m_bVolume;
	int8 m_nProcess;
};

VALIDATE_SIZE(tPedComment, 28);

class cPedComments
{
public:
	tPedComment m_asPedComments[NUM_PED_COMMENTS_BANKS][NUM_PED_COMMENTS_SLOTS];
	uint8 m_nIndexMap[NUM_PED_COMMENTS_BANKS][NUM_PED_COMMENTS_SLOTS];
	uint8 m_nCommentsInBank[NUM_PED_COMMENTS_BANKS];
	uint8 m_nActiveBank;

	cPedComments()
	{
		for (int i = 0; i < NUM_PED_COMMENTS_SLOTS; i++)
			for (int j = 0; j < NUM_PED_COMMENTS_BANKS; j++) {
				m_asPedComments[j][i].m_nProcess = -1;
				m_nIndexMap[j][i] = NUM_PED_COMMENTS_SLOTS;
			}

		for (int i = 0; i < NUM_PED_COMMENTS_BANKS; i++)
			m_nCommentsInBank[i] = 0;
		m_nActiveBank = 0;
	}
	void Add(tPedComment *com);
	void Process();
};

VALIDATE_SIZE(cPedComments, 1164);

class CEntity;

class cMissionAudio
{
public:
	CVector m_vecPos;
	bool m_bPredefinedProperties;
	int32 m_nSampleIndex;
	uint8 m_nLoadingStatus;
	uint8 m_nPlayStatus;
	bool m_bIsPlaying;
	int32 m_nMissionAudioCounter;
	bool m_bIsPlayed;
};
VALIDATE_SIZE(cMissionAudio, 32);

// name made up
class cAudioScriptObjectManager
{
public:
	int32 m_anScriptObjectEntityIndices[NUM_SCRIPT_MAX_ENTITIES];
	int32 m_nScriptObjectEntityTotal;

	cAudioScriptObjectManager() { m_nScriptObjectEntityTotal = 0; }
	~cAudioScriptObjectManager() { m_nScriptObjectEntityTotal = 0; }
};


class cTransmission;
class CPlane;
class CVehicle;
class CPed;

class cPedParams
{
public:
	bool m_bDistanceCalculated;
	float m_fDistance;
	CPed *m_pPed;

	cPedParams()
	{
		m_bDistanceCalculated = false;
		m_fDistance = 0.0f;
		m_pPed = nil;
	}
};

class cVehicleParams
{
public:
	bool m_bDistanceCalculated;
	float m_fDistance;
	CVehicle *m_pVehicle;
	cTransmission *m_pTransmission;
	int32 m_nIndex;
	float m_fVelocityChange;

	cVehicleParams()
	{
		m_bDistanceCalculated = false;
		m_fDistance = 0.0f;
		m_pVehicle = nil;
		m_pTransmission = nil;
		m_nIndex = 0;
		m_fVelocityChange = 0.0f;
	}
};

VALIDATE_SIZE(cVehicleParams, 0x18);

enum {
	/*
	REFLECTION_YMAX = 0, top
	REFLECTION_YMIN = 1, bottom
	REFLECTION_XMIN = 2, left
	REFLECTION_XMAX = 3, right
	REFLECTION_ZMAX = 4,
	*/

	REFLECTION_TOP = 0,
	REFLECTION_BOTTOM,
	REFLECTION_LEFT,
	REFLECTION_RIGHT,
	REFLECTION_UP,
	MAX_REFLECTIONS,
};

class cAudioManager
{
public:
	bool m_bIsInitialised;
	bool m_bReverb; // unused
	bool m_bFifthFrameFlag;
	uint8 m_nActiveSamples;
	uint8 field_4; // unused
	bool m_bDynamicAcousticModelingStatus;
	float m_fSpeedOfSound;
	bool m_bTimerJustReset;
	int32 m_nTimer;
	tSound m_sQueueSample;
	uint8 m_nActiveSampleQueue;
	tSound m_asSamples[NUM_SOUNDS_SAMPLES_BANKS][NUM_SOUNDS_SAMPLES_SLOTS];
	uint8 m_abSampleQueueIndexTable[NUM_SOUNDS_SAMPLES_BANKS][NUM_SOUNDS_SAMPLES_SLOTS];
	uint8 m_SampleRequestQueuesStatus[NUM_SOUNDS_SAMPLES_BANKS];
	tSound m_asActiveSamples[NUM_SOUNDS_SAMPLES_SLOTS];
	tAudioEntity m_asAudioEntities[NUM_AUDIOENTITIES];
	int32 m_anAudioEntityIndices[NUM_AUDIOENTITIES];
	int32 m_nAudioEntitiesTotal;
	CVector m_avecReflectionsPos[NUM_AUDIO_REFLECTIONS];
	float m_afReflectionsDistances[NUM_AUDIO_REFLECTIONS];
	cAudioScriptObjectManager m_sAudioScriptObjectManager;
	cPedComments m_sPedComments;
	int32 m_nFireAudioEntity;
	int32 m_nWaterCannonEntity;
	int32 m_nPoliceChannelEntity;
	cPoliceRadioQueue m_sPoliceRadioQueue;
	int32 m_nFrontEndEntity;
	int32 m_nCollisionEntity;
	cAudioCollisionManager m_sCollisionManager;
	int32 m_nProjectileEntity;
	int32 m_nBridgeEntity;
	cMissionAudio m_sMissionAudio;
	int32 m_anRandomTable[5];
	uint8 m_nTimeSpent;
	uint8 m_nUserPause;
	uint8 m_nPreviousUserPause;
	uint32 m_FrameCounter;

	cAudioManager();
	~cAudioManager();

	// getters
	uint32 GetFrameCounter() const { return m_FrameCounter; }
	float GetReflectionsDistance(int32 idx) const { return m_afReflectionsDistances[idx]; }
	int32 GetRandomNumber(int32 idx) const { return m_anRandomTable[idx]; }
	int32 GetRandomNumberInRange(int32 idx, int32 low, int32 high) const { return (m_anRandomTable[idx] % (high - low + 1)) + low; }
	bool ShouldDuckMissionAudio() const { return m_sMissionAudio.m_nPlayStatus == 1; }

	// "Should" be in alphabetic order, except "getXTalkSfx"
	void AddDetailsToRequestedOrderList(uint8 sample);
	void AddPlayerCarSample(uint8 emittingVolume, int32 freq, uint32 sample, uint8 bank,
	                        uint8 counter, bool notLooping);
	void AddReflectionsToRequestedQueue();
	void AddReleasingSounds();
	void AddSampleToRequestedQueue();
	void AgeCrimes();

	void CalculateDistance(bool &condition, float dist);
	bool CheckForAnAudioFileOnCD() const;
	void ClearActiveSamples();
	void ClearMissionAudio();
	void ClearRequestedQueue();
	int32 ComputeDopplerEffectedFrequency(uint32 oldFreq, float position1, float position2,
	                                      float speedMultiplier) const;
	int32 ComputePan(float, CVector *);
	uint8 ComputeVolume(uint8 emittingVolume, float soundIntensity, float distance) const;
	int32 CreateEntity(eAudioType type, void *entity);

	void DestroyAllGameCreatedEntities();
	void DestroyEntity(int32 id);
	void DoPoliceRadioCrackle();

	// functions returning talk sfx,
	// order from GetPedCommentSfx
	uint32 GetPlayerTalkSfx(int16 sound);
	uint32 GetCopTalkSfx(int16 sound);
	uint32 GetSwatTalkSfx(int16 sound);
	uint32 GetFBITalkSfx(int16 sound);
	uint32 GetArmyTalkSfx(int16 sound);
	uint32 GetMedicTalkSfx(int16 sound);
	uint32 GetFiremanTalkSfx(int16 sound);
	uint32 GetNormalMaleTalkSfx(int16 sound);
	uint32 GetTaxiDriverTalkSfx(int16 sound);
	uint32 GetPimpTalkSfx(int16 sound);
	uint32 GetMafiaTalkSfx(int16 sound);
	uint32 GetTriadTalkSfx(int16 sound);
	uint32 GetDiabloTalkSfx(int16 sound);
	uint32 GetYakuzaTalkSfx(int16 sound);
	uint32 GetYardieTalkSfx(int16 sound);
	uint32 GetColumbianTalkSfx(int16 sound);
	uint32 GetHoodTalkSfx(int16 sound);
	uint32 GetBlackCriminalTalkSfx(int16 sound);
	uint32 GetWhiteCriminalTalkSfx(int16 sound);
	uint32 GetMaleNo2TalkSfx(int16 sound);
	uint32 GetBlackProjectMaleTalkSfx(int16 sound, int32 model);
	uint32 GetWhiteFatMaleTalkSfx(int16 sound);
	uint32 GetBlackFatMaleTalkSfx(int16 sound);
	uint32 GetBlackCasualFemaleTalkSfx(int16 sound);
	uint32 GetWhiteCasualFemaleTalkSfx(int16 sound);
	uint32 GetFemaleNo3TalkSfx(int16 sound);
	uint32 GetBlackFatFemaleTalkSfx(int16 sound);
	uint32 GetWhiteFatFemaleTalkSfx(int16 sound);
	uint32 GetBlackFemaleProstituteTalkSfx(int16 sound);
	uint32 GetWhiteFemaleProstituteTalkSfx(int16 sound);
	uint32 GetBlackProjectFemaleOldTalkSfx(int16 sound);
	uint32 GetBlackProjectFemaleYoungTalkSfx(int16 sound);
	uint32 GetChinatownMaleOldTalkSfx(int16 sound);
	uint32 GetChinatownMaleYoungTalkSfx(int16 sound);
	uint32 GetChinatownFemaleOldTalkSfx(int16 sound);
	uint32 GetChinatownFemaleYoungTalkSfx(int16 sound);
	uint32 GetLittleItalyMaleTalkSfx(int16 sound);
	uint32 GetLittleItalyFemaleOldTalkSfx(int16 sound);
	uint32 GetLittleItalyFemaleYoungTalkSfx(int16 sound);
	uint32 GetWhiteDockerMaleTalkSfx(int16 sound);
	uint32 GetBlackDockerMaleTalkSfx(int16 sound);
	uint32 GetScumMaleTalkSfx(int16 sound);
	uint32 GetScumFemaleTalkSfx(int16 sound);
	uint32 GetWhiteWorkerMaleTalkSfx(int16 sound);
	uint32 GetBlackWorkerMaleTalkSfx(int16 sound);
	uint32 GetBusinessMaleYoungTalkSfx(int16 sound, int32 model);
	uint32 GetBusinessMaleOldTalkSfx(int16 sound);
	uint32 GetWhiteBusinessFemaleTalkSfx(int16 sound, int32 model);
	uint32 GetBlackBusinessFemaleTalkSfx(int16 sound);
	uint32 GetSupermodelMaleTalkSfx(int16 sound);
	uint32 GetSupermodelFemaleTalkSfx(int16 sound);
	uint32 GetStewardMaleTalkSfx(int16 sound);
	uint32 GetStewardFemaleTalkSfx(int16 sound);
	uint32 GetFanMaleTalkSfx(int16 sound, int32 model);
	uint32 GetFanFemaleTalkSfx(int16 sound);
	uint32 GetHospitalMaleTalkSfx(int16 sound);
	uint32 GetHospitalFemaleTalkSfx(int16 sound);
	uint32 GetWhiteConstructionWorkerTalkSfx(int16 sound);
	uint32 GetBlackConstructionWorkerTalkSfx(int16 sound);
	uint32 GetShopperFemaleTalkSfx(int16 sound, int32 model);
	uint32 GetStudentMaleTalkSfx(int16 sound);
	uint32 GetStudentFemaleTalkSfx(int16 sound);
	uint32 GetCasualMaleOldTalkSfx(int16 sound);

	uint32 GetSpecialCharacterTalkSfx(int32 modelIndex, int32 sound);
	uint32 GetEightTalkSfx(int16 sound);
	uint32 GetFrankieTalkSfx(int16 sound);
	uint32 GetMistyTalkSfx(int16 sound);
	uint32 GetOJGTalkSfx(int16 sound);
	uint32 GetCatatalinaTalkSfx(int16 sound);
	uint32 GetBomberTalkSfx(int16 sound);
	uint32 GetSecurityGuardTalkSfx(int16 sound);
	uint32 GetChunkyTalkSfx(int16 sound);

	uint32 GetGenericMaleTalkSfx(int16 sound);
	uint32 GetGenericFemaleTalkSfx(int16 sound);
	// end of functions returning talk sfx

	void GenerateIntegerRandomNumberTable();
	char *Get3DProviderName(uint8 id) const;
	uint8 GetCDAudioDriveLetter() const;
	int8 GetCurrent3DProviderIndex() const;
	float GetCollisionLoopingRatio(uint32 a, uint32 b, float c) const; // not used
	float GetCollisionOneShotRatio(int32 a, float b) const;
	float GetCollisionRatio(float a, float b, float c, float d) const;
	float GetDistanceSquared(const CVector &v) const;
	int32 GetJumboTaxiFreq() const;
	uint8 GetMissionAudioLoadingStatus() const;
	int8 GetMissionScriptPoliceAudioPlayingStatus() const;
	uint8 GetNum3DProvidersAvailable() const;
	int32 GetPedCommentSfx(CPed *ped, int32 sound);
	void GetPhrase(uint32 &phrase, uint32 &prevPhrase, uint32 sample, uint32 maxOffset) const;
	float GetVehicleDriveWheelSkidValue(uint8 wheel, CAutomobile *automobile,
	                                    cTransmission *transmission, float velocityChange);
	float GetVehicleNonDriveWheelSkidValue(uint8 wheel, CAutomobile *automobile,
	                                       cTransmission *transmission, float velocityChange);

	bool HasAirBrakes(int32 model) const;

	void Initialise();
	void InitialisePoliceRadio();
	void InitialisePoliceRadioZones();
	void InterrogateAudioEntities();
	bool IsAudioInitialised() const;
	bool IsMissionAudioSampleFinished();
	bool IsMP3RadioChannelAvailable() const;

	bool MissionScriptAudioUsesPoliceChannel(int32 soundMission) const;

	void PlayLoadedMissionAudio();
	void PlayOneShot(int32 index, uint16 sound, float vol);
	void PlaySuspectLastSeen(float x, float y, float z);
	void PlayerJustGotInCar() const;
	void PlayerJustLeftCar() const;
	void PostInitialiseGameSpecificSetup();
	void PostTerminateGameSpecificShutdown();
	void PreInitialiseGameSpecificSetup() const;
	void PreloadMissionAudio(Const char *name);
	void PreTerminateGameSpecificShutdown();
	/// processX - main logic of adding new sounds
	void ProcessActiveQueues();
	bool ProcessAirBrakes(cVehicleParams& params);
	void ProcessAirportScriptObject(uint8 sound);
	bool ProcessBoatEngine(cVehicleParams& params);
	bool ProcessBoatMovingOverWater(cVehicleParams& params);
	void ProcessBridge();
	void ProcessBridgeMotor();
	void ProcessBridgeOneShots();
	void ProcessBridgeWarning();
	bool ProcessCarBombTick(cVehicleParams& params);
	void ProcessCesna(cVehicleParams& params);
	void ProcessCinemaScriptObject(uint8 sound);
	void ProcessCrane();
	void ProcessDocksScriptObject(uint8 sound);
	bool ProcessEngineDamage(cVehicleParams& params);
	void ProcessEntity(int32 sound);
	void ProcessExplosions(int32 explosion);
	void ProcessFireHydrant();
	void ProcessFires(int32 entity);
	void ProcessFrontEnd();
	void ProcessGarages();
	bool ProcessHelicopter(cVehicleParams& params);
	void ProcessHomeScriptObject(uint8 sound);
	void ProcessJumbo(cVehicleParams& params);
	void ProcessJumboAccel(CPlane *plane);
	void ProcessJumboDecel(CPlane *plane);
	void ProcessJumboFlying();
	void ProcessJumboLanding(CPlane *plane);
	void ProcessJumboTakeOff(CPlane *plane);
	void ProcessJumboTaxi();
	void ProcessLaunderetteScriptObject(uint8 sound);
	void ProcessLoopingScriptObject(uint8 sound);
	void ProcessMissionAudio();
	void ProcessModelCarEngine(cVehicleParams& params);
	void ProcessOneShotScriptObject(uint8 sound);
	void ProcessPed(CPhysical *ped);
	void ProcessPedHeadphones(cPedParams &params);
	void ProcessPedOneShots(cPedParams &params);
	void ProcessPhysical(int32 id);
	void ProcessPlane(cVehicleParams& params);
	void ProcessPlayersVehicleEngine(cVehicleParams& params, CAutomobile *automobile);
	void ProcessPoliceCellBeatingScriptObject(uint8 sound);
	void ProcessPornCinema(uint8 sound);
	void ProcessProjectiles();
	void ProcessRainOnVehicle(cVehicleParams& params);
	void ProcessReverb() const;
	bool ProcessReverseGear(cVehicleParams& params);
	void ProcessSawMillScriptObject(uint8 sound);
	void ProcessScriptObject(int32 id);
	void ProcessShopScriptObject(uint8 sound);
	void ProcessSpecial();
	bool ProcessTrainNoise(cVehicleParams& params);
	void ProcessVehicle(CVehicle *vehicle);
	bool ProcessVehicleDoors(cVehicleParams& params);
	void ProcessVehicleEngine(cVehicleParams& params);
	void ProcessVehicleHorn(cVehicleParams& params);
	void ProcessVehicleOneShots(cVehicleParams& params);
	bool ProcessVehicleReverseWarning(cVehicleParams& params);
	bool ProcessVehicleRoadNoise(cVehicleParams& params);
	bool ProcessVehicleSirenOrAlarm(cVehicleParams& params);
	bool ProcessVehicleSkidding(cVehicleParams& params);
	void ProcessWaterCannon(int32);
	void ProcessWeather(int32 id);
	bool ProcessWetRoadNoise(cVehicleParams& params);
	void ProcessWorkShopScriptObject(uint8 sound);

	int32 RandomDisplacement(uint32 seed) const;
	void ReacquireDigitalHandle() const;
	void ReleaseDigitalHandle() const;
	void ReportCollision(CEntity *entity1, CEntity *entity2, uint8 surface1, uint8 surface2,
	                     float collisionPower, float intensity2);
	void ReportCrime(eCrimeType crime, const CVector &pos);
	void ResetAudioLogicTimers(uint32 timer);
	void ResetPoliceRadio();
	void ResetTimers(uint32 time);

	void Service();
	void ServiceCollisions();
	void ServicePoliceRadio();
	void ServicePoliceRadioChannel(uint8 wantedLevel);
	void ServiceSoundEffects();
	int8 SetCurrent3DProvider(uint8 which);
	void SetDynamicAcousticModelingStatus(uint8 status);
	void SetEffectsFadeVol(uint8 volume) const;
	void SetEffectsMasterVolume(uint8 volume) const;
	void SetEntityStatus(int32 id, uint8 status);
	uint32 SetLoopingCollisionRequestedSfxFreqAndGetVol(const cAudioCollision &audioCollision);
	void SetMissionAudioLocation(float x, float y, float z);
	void SetMissionScriptPoliceAudio(int32 sfx) const;
	void SetMonoMode(uint8 mono);
	void SetMusicFadeVol(uint8 volume) const;
	void SetMusicMasterVolume(uint8 volume) const;
	void SetSpeakerConfig(int32 conf) const;
	void SetUpLoopingCollisionSound(const cAudioCollision &col, uint8 counter);
	void SetUpOneShotCollisionSound(const cAudioCollision &col);
	bool SetupCrimeReport();
	bool SetupJumboEngineSound(uint8 vol, uint32 freq);
	bool SetupJumboFlySound(uint8 emittingVol);
	bool SetupJumboRumbleSound(uint8 emittingVol);
	bool SetupJumboTaxiSound(uint8 vol);
	bool SetupJumboWhineSound(uint8 emittingVol, uint32 freq);
	void SetupPedComments(cPedParams &params, uint16 sound);
	void SetupSuspectLastSeenReport();

	void Terminate();
	void TranslateEntity(Const CVector *v1, CVector *v2) const;

	void UpdateGasPedalAudio(CAutomobile *automobile);
	void UpdateReflections();
	bool UsesReverseWarning(int32 model) const;
	bool UsesSiren(int32 model) const;
	bool UsesSirenSwitching(int32 model) const;

#ifdef GTA_PC
	// only used in pc
	void AdjustSamplesVolume();
	uint8 ComputeEmittingVolume(uint8 emittingVolume, float intensity, float dist);
#endif
};

#ifdef AUDIO_MSS
static_assert(sizeof(cAudioManager) == 19220, "cAudioManager: error");
#endif

extern cAudioManager AudioManager;
