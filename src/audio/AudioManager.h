#pragma once

#include "audio_enums.h"
#include "AudioCollision.h"
#include "PoliceRadio.h"
#include "VehicleModelInfo.h"

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
	uint8 field_4C;
	int32 m_nReleasingVolumeDivider;
	bool m_bIsProcessed;
	bool m_bLoopEnded;
	int32 m_nCalculatedVolume;
	int8 m_nVolumeChange;
};

VALIDATE_SIZE(tSound, 96);

class CPhysical;
class CAutomobile;

class tAudioEntity
{
public:
	eAudioType m_nType;
	void *m_pEntity;
	bool m_bIsUsed;
	uint8 m_nStatus;
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
	bool m_bDelay;
	uint32 m_nDelayTimer;

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
	void Add(tPedComment *com); // done
	void Process();             // done
};

VALIDATE_SIZE(cPedComments, 1164);

class CEntity;

#define MISSION_AUDIO_SLOTS (2)

// So instead of doing cMissionAudio [2] they've added [2] to every field of the struct...
// Only someone with a VERY EXTRAORDINARY mind could have come up with that
class cMissionAudio
{
public:
	CVector m_vecPos[MISSION_AUDIO_SLOTS];
	bool m_bPredefinedProperties[MISSION_AUDIO_SLOTS];
	int32 m_nSampleIndex[MISSION_AUDIO_SLOTS];
	uint8 m_nLoadingStatus[MISSION_AUDIO_SLOTS];
	uint8 m_nPlayStatus[MISSION_AUDIO_SLOTS];
	bool m_bIsPlaying[MISSION_AUDIO_SLOTS];
	int32 m_nMissionAudioCounter[MISSION_AUDIO_SLOTS];
	bool m_bIsPlayed[MISSION_AUDIO_SLOTS];
	bool m_bIsMobile[MISSION_AUDIO_SLOTS];
};
VALIDATE_SIZE(cMissionAudio, 0x38);

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
};

class cVehicleParams
{
public:
	eVehicleType m_VehicleType;
	bool m_bDistanceCalculated;
	float m_fDistance;
	CVehicle *m_pVehicle;
	cTransmission *m_pTransmission;
	int32 m_nIndex;
	float m_fVelocityChange;
};

VALIDATE_SIZE(cVehicleParams, 0x18);

enum {
	REFLECTION_NORTH = 0,
	REFLECTION_SOUTH,
	REFLECTION_WEST,
	REFLECTION_EAST,
	REFLECTION_CEIL_NORTH,
	REFLECTION_CEIL_SOUTH,
	REFLECTION_CEIL_WEST,
	REFLECTION_CEIL_EAST,
	MAX_REFLECTIONS,
};

class cAudioManager
{
public:
	bool m_bIsInitialised;
	uint8 field_1; // unused
	bool m_bFifthFrameFlag;
	uint8 m_nActiveSamples;
	uint8 field_4; // unused
	bool m_bDynamicAcousticModelingStatus;
	int8 field_6;
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

	// miami
	uint8 m_bIsPlayerShutUp;
	uint8 m_nPlayerMood;
	uint32 m_nPlayerMoodTimer;
	uint8 field_rest[4];
	bool m_bGenericSfx;

	cPedComments m_sPedComments;
	int32 m_nFireAudioEntity;
	int32 m_nWaterCannonEntity;
	int32 m_nPoliceChannelEntity;
	cPoliceRadioQueue m_sPoliceRadioQueue;
	int32 m_nFrontEndEntity;
	int32 m_nCollisionEntity;
	cAudioCollisionManager m_sCollisionManager;
	int32 m_nProjectileEntity;
#ifdef GTA_BRIDGE
	int32 m_nBridgeEntity;
#endif
	int32 m_nEscalatorEntity;
	int32 m_nExtraSoundsEntity;
	cMissionAudio m_sMissionAudio;
	uint8 field_5538; // something related to phone dialogues
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
	bool IsMissionAudioSamplePlaying(uint8 slot) const; // { return m_sMissionAudio.m_nPlayStatus == 1; }
	bool ShouldDuckMissionAudio(uint8 slot) const;

	// "Should" be in alphabetic order, except "getXTalkSfx"
	void AddDetailsToRequestedOrderList(uint8 sample);                                                                    // done (inlined in vc)
	void AddPlayerCarSample(uint8 emittingVolume, int32 freq, uint32 sample, uint8 bank, uint8 counter, bool notLooping); // done
	void AddReflectionsToRequestedQueue();                                                                                // done
	void AddReleasingSounds();                                                                                            // done
	void AddSampleToRequestedQueue();                                                                                     // done
	void AgeCrimes();                                                                                                     // done (inlined in vc)

	void CalculateDistance(bool &condition, float dist);                                                                   // done
	bool CheckForAnAudioFileOnCD() const;                                                                                  // done
	void ClearActiveSamples();                                                                                             // done
	void ClearMissionAudio(uint8 slot);                                                                                    // done
	void ClearRequestedQueue();                                                                                            // done (inlined in vc)
	uint32 ComputeDopplerEffectedFrequency(uint32 oldFreq, float position1, float position2, float speedMultiplier) const; // done
	int32 ComputePan(float, CVector *);                                                                                    // done
	uint8 ComputeVolume(uint8 emittingVolume, float soundIntensity, float distance) const;                                 // done
	int32 CreateEntity(eAudioType type, void *entity);                                                                     // done

	void DestroyAllGameCreatedEntities();                                                                                  // done ? I don't seed pEntity = nil;
	void DestroyEntity(int32 id);                                                                                          // done (inlined in vc) ? I not seen id checks
	void DoPoliceRadioCrackle();                                                                                           // done

	// functions returning talk sfx,
	// order from GetPedCommentSfx
	int32 GetPlayerTalkSfx(CPed *ped, int16 sound);
	int32 GetCopTalkSfx(CPed *ped, int16 sound);
	int32 GetSwatTalkSfx(CPed *ped, int16 sound);
	int32 GetFBITalkSfx(CPed *ped, int16 sound);
	int32 GetArmyTalkSfx(CPed *ped, int16 sound);
	int32 GetMedicTalkSfx(CPed *ped, int16 sound);
	int32 GetFiremanTalkSfx(CPed *ped, int16 sound);
	int32 GetDefaultTalkSfx(CPed *ped, int16 sound);
	int32 GetHFYSTTalkSfx(CPed *ped, int16 sound);
	int32 GetHFOSTTalkSfx(CPed *ped, int16 sound);
	int32 GetHMYSTTalkSfx(CPed *ped, int16 sound);
	int32 GetHMOSTTalkSfx(CPed *ped, int16 sound);
	int32 GetHFYRITalkSfx(CPed *ped, int16 sound);
	int32 GetHFORITalkSfx(CPed *ped, int16 sound); 
	int32 GetHMYRITalkSfx(CPed *ped, int16 sound); 
	int32 GetHMORITalkSfx(CPed *ped, int16 sound);
	int32 GetHFYBETalkSfx(CPed *ped, int16 sound);
	int32 GetHFOBETalkSfx(CPed *ped, int16 sound);
	int32 GetHMYBETalkSfx(CPed *ped, int16 sound);
	int32 GetHMOBETalkSfx(CPed *ped, int16 sound);
	int32 GetHFYBUTalkSfx(CPed *ped, int16 sound);
	int32 GetHFYMDTalkSfx(CPed *ped, int16 sound);
	int32 GetHFYCGTalkSfx(CPed *ped, int16 sound);
	int32 GetHFYPRTalkSfx(CPed *ped, int16 sound);
	int32 GetHFOTRTalkSfx(CPed *ped, int16 sound);
	int32 GetHMOTRTalkSfx(CPed *ped, int16 sound);
	int32 GetHMYAPTalkSfx(CPed *ped, int16 sound);
	int32 GetHMOCATalkSfx(CPed *ped, int16 sound);
	int32 GetBMODKTalkSfx(CPed *ped, int16 sound);
	int32 GetBMYCRTalkSfx(CPed *ped, int16 sound);
	int32 GetBFYSTTalkSfx(CPed *ped, int16 sound);
	int32 GetBFOSTTalkSfx(CPed *ped, int16 sound);
	int32 GetBMYSTTalkSfx(CPed *ped, int16 sound);
	int32 GetBMOSTTalkSfx(CPed *ped, int16 sound);
	int32 GetBFYRITalkSfx(CPed *ped, int16 sound);
	int32 GetBFORITalkSfx(CPed *ped, int16 sound);
	int32 GetBMYRITalkSfx(CPed *ped, int16 sound);
	int32 GetBFYBETalkSfx(CPed *ped, int16 sound);
	int32 GetBMYBETalkSfx(CPed *ped, int16 sound);
	int32 GetBFOBETalkSfx(CPed *ped, int16 sound);
	int32 GetBMOBETalkSfx(CPed *ped, int16 sound);
	int32 GetBMYBUTalkSfx(CPed *ped, int16 sound);
	int32 GetBFYPRTalkSfx(CPed *ped, int16 sound);
	int32 GetBFOTRTalkSfx(CPed *ped, int16 sound);
	int32 GetBMOTRTalkSfx(CPed *ped, int16 sound);
	int32 GetBMYPITalkSfx(CPed *ped, int16 sound);
	int32 GetBMYBBTalkSfx(CPed *ped, int16 sound);
	int32 GetWMYCRTalkSfx(CPed *ped, int16 sound);
	int32 GetWFYSTTalkSfx(CPed *ped, int16 sound);
	int32 GetWFOSTTalkSfx(CPed *ped, int16 sound);
	int32 GetWMYSTTalkSfx(CPed *ped, int16 sound);
	int32 GetWMOSTTalkSfx(CPed *ped, int16 sound);
	int32 GetWFYRITalkSfx(CPed *ped, int16 sound);
	int32 GetWFORITalkSfx(CPed *ped, int16 sound);
	int32 GetWMYRITalkSfx(CPed *ped, int16 sound);
	int32 GetWMORITalkSfx(CPed *ped, int16 sound);
	int32 GetWFYBETalkSfx(CPed *ped, int16 sound);
	int32 GetWMYBETalkSfx(CPed *ped, int16 sound);
	int32 GetWFOBETalkSfx(CPed *ped, int16 sound);
	int32 GetWMOBETalkSfx(CPed *ped, int16 sound);
	int32 GetWMYCWTalkSfx(CPed *ped, int16 sound);
	int32 GetWMYGOTalkSfx(CPed *ped, int16 sound);
	int32 GetWFOGOTalkSfx(CPed *ped, int16 sound);
	int32 GetWMOGOTalkSfx(CPed *ped, int16 sound);
	int32 GetWFYLGTalkSfx(CPed *ped, int16 sound);
	int32 GetWMYLGTalkSfx(CPed *ped, int16 sound);
	int32 GetWFYBUTalkSfx(CPed *ped, int16 sound);
	int32 GetWMYBUTalkSfx(CPed *ped, int16 sound);
	int32 GetWMOBUTalkSfx(CPed *ped, int16 sound);
	int32 GetWFYPRTalkSfx(CPed *ped, int16 sound);
	int32 GetWFOTRTalkSfx(CPed *ped, int16 sound);
	int32 GetWMOTRTalkSfx(CPed *ped, int16 sound);
	int32 GetWMYPITalkSfx(CPed *ped, int16 sound);
	int32 GetWMOCATalkSfx(CPed *ped, int16 sound);
	int32 GetWFYJGTalkSfx(CPed *ped, int16 sound);
	int32 GetWMYJGTalkSfx(CPed *ped, int16 sound);
	int32 GetWFYSKTalkSfx(CPed *ped, int16 sound);
	int32 GetWMYSKTalkSfx(CPed *ped, int16 sound);
	int32 GetWFYSHTalkSfx(CPed *ped, int16 sound);
	int32 GetWFOSHTalkSfx(CPed *ped, int16 sound);
	int32 GetJFOTOTalkSfx(CPed *ped, int16 sound);
	int32 GetJMOTOTalkSfx(CPed *ped, int16 sound);
	int32 GetCBTalkSfx(CPed *ped, int16 sound);
	int32 GetHNTalkSfx(CPed *ped, int16 sound);
	int32 GetSGTalkSfx(CPed *ped, int16 sound);
	int32 GetCLTalkSfx(CPed *ped, int16 sound);
	int32 GetGDTalkSfx(CPed *ped, int16 sound);
	int32 GetBKTalkSfx(CPed *ped, int16 sound);
	int32 GetPGTalkSfx(CPed *ped, int16 sound);
	int32 GetVICETalkSfx(CPed *ped, int16 sound, int16 model);
	int32 GetWFYG1TalkSfx(CPed *ped, int16 sound);
	int32 GetWFYG2TalkSfx(CPed *ped, int16 sound);

	int32 GetGenericMaleTalkSfx(CPed *ped, int16 sound);   // todo names (inlined in vc)
	int32 GetGenericFemaleTalkSfx(CPed *ped, int16 sound); // todo names (inlined in vc)
	// end of functions returning talk sfx

	void GenerateIntegerRandomNumberTable();
	char *Get3DProviderName(uint8 id) const;
	uint8 GetCDAudioDriveLetter() const;
	int8 GetCurrent3DProviderIndex() const;
	int8 AutoDetect3DProviders() const;                                // done
	float GetCollisionLoopingRatio(uint32 a, uint32 b, float c) const; // not used
	float GetCollisionOneShotRatio(int32 a, float b) const;
	float GetCollisionRatio(float a, float b, float c, float d) const;
	float GetDistanceSquared(const CVector &v) const; // done (inlined in vc)
	int32 GetJumboTaxiFreq() const;
	uint8 GetMissionAudioLoadingStatus(uint8 slot) const; // done
	int8 GetMissionScriptPoliceAudioPlayingStatus() const;
	uint8 GetNum3DProvidersAvailable() const; // done
	int32 GetPedCommentSfx(CPed *ped, int32 sound);
	void GetPhrase(int32 *phrase, int32 *prevPhrase, int32 sample, int32 maxOffset) const;
	float GetVehicleDriveWheelSkidValue(uint8 wheel, CAutomobile *automobile, cTransmission *transmission, float velocityChange);
	float GetVehicleNonDriveWheelSkidValue(uint8 wheel, CAutomobile *automobile, cTransmission *transmission, float velocityChange);

	bool HasAirBrakes(int32 model) const; // done

	void Initialise(); // done
	void InitialisePoliceRadio();
	void InitialisePoliceRadioZones();
	void InterrogateAudioEntities(); // done
	bool IsAudioInitialised() const;
	bool IsMissionAudioSampleFinished(uint8 slot);
	bool IsMP3RadioChannelAvailable() const; // done

	bool MissionScriptAudioUsesPoliceChannel(int32 soundMission) const;

	void PlayLoadedMissionAudio(uint8 slot);                // done
	void PlayOneShot(int32 index, int16 sound, float vol);  // done
	void PlaySuspectLastSeen(float x, float y, float z);    //
	void PlayerJustGotInCar() const;                        // done
	void PlayerJustLeftCar() const;                         // done
	void PostInitialiseGameSpecificSetup();                 //
	void PostTerminateGameSpecificShutdown();               // done
	void PreInitialiseGameSpecificSetup() const;            // done
	void PreloadMissionAudio(uint8 slot, Const char *name); // done
	void PreTerminateGameSpecificShutdown();                // done
	/// processX - main logic of adding new sounds
	void ProcessActiveQueues(); //done
	bool ProcessAirBrakes(cVehicleParams *params);
	bool ProcessBoatEngine(cVehicleParams *params);
	bool ProcessBoatMovingOverWater(cVehicleParams *params);
#ifdef GTA_BRIDGE
	void ProcessBridge();
	void ProcessBridgeMotor();
	void ProcessBridgeOneShots();
	void ProcessBridgeWarning();
#endif
	bool ProcessCarBombTick(cVehicleParams *params);                         // done
	void ProcessCesna(cVehicleParams *params);                               // 
	//void ProcessCrane();                                                   // 
	bool ProcessEngineDamage(cVehicleParams *params);                        // done
	void ProcessEntity(int32 sound);                                         // done
	void ProcessExplosions(int32 explosion);                                 // done
	void ProcessFireHydrant();                                               // done
	void ProcessFires(int32 entity);                                         // 
	void ProcessFrontEnd();                                                  // done
	void ProcessGarages();                                                   // 
	void ProcessCarHeli(cVehicleParams* params);                             // done
	void ProcessVehicleFlatTyre(cVehicleParams* params);                     // done
	void ProcessJumbo(cVehicleParams *);                                     // 
	void ProcessJumboAccel(CPlane *plane);                                   // 
	void ProcessJumboDecel(CPlane *plane);                                   // 
	void ProcessJumboFlying();                                               // 
	void ProcessJumboLanding(CPlane *plane);                                 // 
	void ProcessJumboTakeOff(CPlane *plane);                               	 // 
	void ProcessJumboTaxi();                                                 // 
	void ProcessLoopingScriptObject(uint8 sound);                            // 
	void ProcessMissionAudio();                                              // 
	void ProcessMissionAudioSlot(uint8 slot);                                // 
	void ProcessModelCarEngine(cVehicleParams *params);                      // 
	void ProcessOneShotScriptObject(uint8 sound);                            // 
	void ProcessPed(CPhysical *ped);                                         // 
	void ProcessPedOneShots(cPedParams *params);                             // 
	void ProcessPhysical(int32 id);                                          // done
	void ProcessPlane(cVehicleParams *params);                               // done
	void ProcessPlayersVehicleEngine(cVehicleParams *params, CVehicle* veh); // done
	void ProcessProjectiles();                                               // 
	void ProcessRainOnVehicle(cVehicleParams *params);                       // 
	void ProcessReverb() const;                                              // 
	bool ProcessReverseGear(cVehicleParams *params);                         // done
	void ProcessScriptObject(int32 id);                                      // done
	void ProcessSpecial();
#ifdef GTA_TRAIN
	bool ProcessTrainNoise(cVehicleParams *params);
#endif
	void ProcessVehicle(CVehicle *vehicle); //done, but need add model functions
	bool ProcessVehicleDoors(cVehicleParams *params); //done
	void ProcessVehicleEngine(cVehicleParams *params); //done
	void UpdateGasPedalAudio(CVehicle* veh, int vehType); //done
	void ProcessVehicleHorn(cVehicleParams *params);
	void ProcessVehicleOneShots(cVehicleParams *params);
	bool ProcessVehicleReverseWarning(cVehicleParams *params);
	bool ProcessVehicleRoadNoise(cVehicleParams *params);
	bool ProcessVehicleSirenOrAlarm(cVehicleParams *params);
	bool ProcessVehicleSkidding(cVehicleParams *params);
	void ProcessWaterCannon(int32);
	void ProcessWeather(int32 id); //done
	bool ProcessWetRoadNoise(cVehicleParams *params);
	void ProcessEscalators(); //done
	void ProcessExtraSounds(); //done

	int32 RandomDisplacement(uint32 seed) const;
	void ReacquireDigitalHandle() const;                                                                                              // done
	void ReleaseDigitalHandle() const;                                                                                                // done
	void ReportCollision(CEntity *entity1, CEntity *entity2, uint8 surface1, uint8 surface2, float collisionPower, float intensity2); // done
	void ReportCrime(eCrimeType crime, const CVector &pos);                                                                                // done
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
	void SetMP3BoostVolume(uint8 volume) const;
	void SetEntityStatus(int32 id, uint8 status); //done
	uint32 SetLoopingCollisionRequestedSfxFreqAndGetVol(const cAudioCollision &audioCollision);
	void SetMissionAudioLocation(uint8 slot, float x, float y, float z);
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
	void SetupPedComments(cPedParams *params, uint32 sound); // done
	void SetupSuspectLastSeenReport();

	void Terminate();
	void TranslateEntity(Const CVector *v1, CVector *v2) const;

	void UpdateReflections();
	bool UsesReverseWarning(int32 model) const; //done
	bool UsesSiren(int32 model) const;
	bool UsesSirenSwitching(int32 model) const;

	CVehicle *FindVehicleOfPlayer(); //done
	void SetPedTalkingStatus(CPed *ped, uint8 status);
	void SetPlayersMood(uint8 mood, uint32 time);

#ifdef GTA_PC
	// only used in pc
	void AdjustSamplesVolume();
	uint8 ComputeEmittingVolume(uint8 emittingVolume, float intensity, float dist);
#endif
};

#ifdef AUDIO_MSS
static_assert(sizeof(cAudioManager) == 0x5558, "cAudioManager: error");
#endif

extern cAudioManager AudioManager;
