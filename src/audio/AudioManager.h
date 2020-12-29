#pragma once

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
	uint8 m_bStatus;
	int16 m_awAudioEvent[NUM_AUDIOENTITY_EVENTS];
	float m_afVolume[NUM_AUDIOENTITY_EVENTS];
	uint8 m_AudioEvents;
};

VALIDATE_SIZE(tAudioEntity, 40);

class tPedComment
{
public:
	uint32 m_nSampleIndex;
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
#ifdef GTA_PC
	bool m_bDelay;
	uint32 m_nDelayTimer;
#endif

	cPedComments();
	void Add(tPedComment *com); // done
	void Process();             // done
};

VALIDATE_SIZE(cPedComments, 1168);

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
VALIDATE_SIZE(cMissionAudio, 56);

// name made up
class cAudioScriptObjectManager
{
public:
	int32 m_anScriptObjectEntityIndices[NUM_SCRIPT_MAX_ENTITIES];
	int32 m_nScriptObjectEntityTotal;

	cAudioScriptObjectManager();
	~cAudioScriptObjectManager();
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

	cPedParams();
};

class cVehicleParams
{
public:
	int32 m_VehicleType;
	bool m_bDistanceCalculated;
	float m_fDistance;
	CVehicle *m_pVehicle;
	cTransmission *m_pTransmission;
	int32 m_nIndex;
	float m_fVelocityChange;

	cVehicleParams();
};

VALIDATE_SIZE(cVehicleParams, 28);

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
	uint8 m_bReverb; // unused
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
	uint32 GetFrameCounter() const;                // done
	float GetReflectionsDistance(int32 idx) const; // done
	int32 GetRandomNumber(int32 idx) const;
	int32 GetRandomNumberInRange(int32 idx, int32 low, int32 high) const;
	bool IsMissionAudioSamplePlaying(uint8 slot) const;
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
	void ClearMissionAudio(uint8 slot);                                                                                    // done (inlined in vc)
	void ClearRequestedQueue();                                                                                            // done (inlined in vc)
	uint32 ComputeDopplerEffectedFrequency(uint32 oldFreq, float position1, float position2, float speedMultiplier) const; // done
	int32 ComputePan(float, CVector *);                                                                                    // done
	uint8 ComputeVolume(uint8 emittingVolume, float soundIntensity, float distance) const;                                 // done
	int32 CreateEntity(eAudioType type, void *entity);                                                                     // done

	void DestroyAllGameCreatedEntities();                                                                                  // done
	void DestroyEntity(int32 id);                                                                                          // done (inlined in vc)
	void DoPoliceRadioCrackle();                                                                                           // done

	// functions returning talk sfx,
	// order from GetPedCommentSfx
	uint32 GetPlayerTalkSfx(CPed *ped, int16 sound);
	uint32 GetCopTalkSfx(CPed *ped, int16 sound);
	uint32 GetSwatTalkSfx(CPed *ped, int16 sound);
	uint32 GetFBITalkSfx(CPed *ped, int16 sound);
	uint32 GetArmyTalkSfx(CPed *ped, int16 sound);
	uint32 GetMedicTalkSfx(CPed *ped, int16 sound);
	uint32 GetFiremanTalkSfx(CPed *ped, int16 sound);
	uint32 GetDefaultTalkSfx(CPed *ped, int16 sound);
	uint32 GetHFYSTTalkSfx(CPed *ped, int16 sound);
	uint32 GetHFOSTTalkSfx(CPed *ped, int16 sound);
	uint32 GetHMYSTTalkSfx(CPed *ped, int16 sound);
	uint32 GetHMOSTTalkSfx(CPed *ped, int16 sound);
	uint32 GetHFYRITalkSfx(CPed *ped, int16 sound);
	uint32 GetHFORITalkSfx(CPed *ped, int16 sound); 
	uint32 GetHMYRITalkSfx(CPed *ped, int16 sound); 
	uint32 GetHMORITalkSfx(CPed *ped, int16 sound);
	uint32 GetHFYBETalkSfx(CPed *ped, int16 sound);
	uint32 GetHFOBETalkSfx(CPed *ped, int16 sound);
	uint32 GetHMYBETalkSfx(CPed *ped, int16 sound);
	uint32 GetHMOBETalkSfx(CPed *ped, int16 sound);
	uint32 GetHFYBUTalkSfx(CPed *ped, int16 sound);
	uint32 GetHFYMDTalkSfx(CPed *ped, int16 sound);
	uint32 GetHFYCGTalkSfx(CPed *ped, int16 sound);
	uint32 GetHFYPRTalkSfx(CPed *ped, int16 sound);
	uint32 GetHFOTRTalkSfx(CPed *ped, int16 sound);
	uint32 GetHMOTRTalkSfx(CPed *ped, int16 sound);
	uint32 GetHMYAPTalkSfx(CPed *ped, int16 sound);
	uint32 GetHMOCATalkSfx(CPed *ped, int16 sound);
	uint32 GetBMODKTalkSfx(CPed *ped, int16 sound);
	uint32 GetBMYCRTalkSfx(CPed *ped, int16 sound);
	uint32 GetBFYSTTalkSfx(CPed *ped, int16 sound);
	uint32 GetBFOSTTalkSfx(CPed *ped, int16 sound);
	uint32 GetBMYSTTalkSfx(CPed *ped, int16 sound);
	uint32 GetBMOSTTalkSfx(CPed *ped, int16 sound);
	uint32 GetBFYRITalkSfx(CPed *ped, int16 sound);
	uint32 GetBFORITalkSfx(CPed *ped, int16 sound);
	uint32 GetBMYRITalkSfx(CPed *ped, int16 sound);
	uint32 GetBFYBETalkSfx(CPed *ped, int16 sound);
	uint32 GetBMYBETalkSfx(CPed *ped, int16 sound);
	uint32 GetBFOBETalkSfx(CPed *ped, int16 sound);
	uint32 GetBMOBETalkSfx(CPed *ped, int16 sound);
	uint32 GetBMYBUTalkSfx(CPed *ped, int16 sound);
	uint32 GetBFYPRTalkSfx(CPed *ped, int16 sound);
	uint32 GetBFOTRTalkSfx(CPed *ped, int16 sound);
	uint32 GetBMOTRTalkSfx(CPed *ped, int16 sound);
	uint32 GetBMYPITalkSfx(CPed *ped, int16 sound);
	uint32 GetBMYBBTalkSfx(CPed *ped, int16 sound);
	uint32 GetWMYCRTalkSfx(CPed *ped, int16 sound);
	uint32 GetWFYSTTalkSfx(CPed *ped, int16 sound);
	uint32 GetWFOSTTalkSfx(CPed *ped, int16 sound);
	uint32 GetWMYSTTalkSfx(CPed *ped, int16 sound);
	uint32 GetWMOSTTalkSfx(CPed *ped, int16 sound);
	uint32 GetWFYRITalkSfx(CPed *ped, int16 sound);
	uint32 GetWFORITalkSfx(CPed *ped, int16 sound);
	uint32 GetWMYRITalkSfx(CPed *ped, int16 sound);
	uint32 GetWMORITalkSfx(CPed *ped, int16 sound);
	uint32 GetWFYBETalkSfx(CPed *ped, int16 sound);
	uint32 GetWMYBETalkSfx(CPed *ped, int16 sound);
	uint32 GetWFOBETalkSfx(CPed *ped, int16 sound);
	uint32 GetWMOBETalkSfx(CPed *ped, int16 sound);
	uint32 GetWMYCWTalkSfx(CPed *ped, int16 sound);
	uint32 GetWMYGOTalkSfx(CPed *ped, int16 sound);
	uint32 GetWFOGOTalkSfx(CPed *ped, int16 sound);
	uint32 GetWMOGOTalkSfx(CPed *ped, int16 sound);
	uint32 GetWFYLGTalkSfx(CPed *ped, int16 sound);
	uint32 GetWMYLGTalkSfx(CPed *ped, int16 sound);
	uint32 GetWFYBUTalkSfx(CPed *ped, int16 sound);
	uint32 GetWMYBUTalkSfx(CPed *ped, int16 sound);
	uint32 GetWMOBUTalkSfx(CPed *ped, int16 sound);
	uint32 GetWFYPRTalkSfx(CPed *ped, int16 sound);
	uint32 GetWFOTRTalkSfx(CPed *ped, int16 sound);
	uint32 GetWMOTRTalkSfx(CPed *ped, int16 sound);
	uint32 GetWMYPITalkSfx(CPed *ped, int16 sound);
	uint32 GetWMOCATalkSfx(CPed *ped, int16 sound);
	uint32 GetWFYJGTalkSfx(CPed *ped, int16 sound);
	uint32 GetWMYJGTalkSfx(CPed *ped, int16 sound);
	uint32 GetWFYSKTalkSfx(CPed *ped, int16 sound);
	uint32 GetWMYSKTalkSfx(CPed *ped, int16 sound);
	uint32 GetWFYSHTalkSfx(CPed *ped, int16 sound);
	uint32 GetWFOSHTalkSfx(CPed *ped, int16 sound);
	uint32 GetJFOTOTalkSfx(CPed *ped, int16 sound);
	uint32 GetJMOTOTalkSfx(CPed *ped, int16 sound);
	uint32 GetCBTalkSfx(CPed *ped, int16 sound);
	uint32 GetHNTalkSfx(CPed *ped, int16 sound);
	uint32 GetSGTalkSfx(CPed *ped, int16 sound);
	uint32 GetCLTalkSfx(CPed *ped, int16 sound);
	uint32 GetGDTalkSfx(CPed *ped, int16 sound);
	uint32 GetBKTalkSfx(CPed *ped, int16 sound);
	uint32 GetPGTalkSfx(CPed *ped, int16 sound);
	uint32 GetVICETalkSfx(CPed *ped, int16 sound, int16 model);
	uint32 GetWFYG1TalkSfx(CPed *ped, int16 sound);
	uint32 GetWFYG2TalkSfx(CPed *ped, int16 sound);

	uint32 GetGenericMaleTalkSfx(CPed *ped, int16 sound);   // todo names (inlined in vc)
	uint32 GetGenericFemaleTalkSfx(CPed *ped, int16 sound); // todo names (inlined in vc)
	// end of functions returning talk sfx

	void GenerateIntegerRandomNumberTable();                                                   // done
	char *Get3DProviderName(uint8 id) const;                                                   // done
	uint8 GetCDAudioDriveLetter() const;                                                       // done
	int8 GetCurrent3DProviderIndex() const;                                                    // done
	int8 AutoDetect3DProviders() const;                                                        // done
	float GetCollisionLoopingRatio(uint32 a, uint32 b, float c) const;                         // not used
	float GetCollisionOneShotRatio(int32 a, float b) const;                                    // done
	float GetCollisionRatio(float a, float b, float c, float d) const;                         // done (inlined in vc)
	float GetDistanceSquared(const CVector &v) const;                                          // done (inlined in vc)
	int32 GetJumboTaxiFreq() const;                                                            // done (inlined in vc)
	uint8 GetMissionAudioLoadingStatus(uint8 slot) const;                                      // done
	int8 GetMissionScriptPoliceAudioPlayingStatus() const;                                     // done
	uint8 GetNum3DProvidersAvailable() const;                                                  // done
	uint32 GetPedCommentSfx(CPed *ped, int32 sound);                                           // done
	void GetPhrase(uint32 &phrase, uint32 &prevPhrase, uint32 sample, uint32 maxOffset) const; // done
	float GetVehicleDriveWheelSkidValue(CVehicle *veh, tWheelState wheelState, float gasPedalAudio, cTransmission *transmission,
	                                    float velocityChange);                                                                        // done
	float GetVehicleNonDriveWheelSkidValue(CVehicle *veh, tWheelState wheelState, cTransmission *transmission, float velocityChange); // done

	bool HasAirBrakes(int32 model) const; // done

	void Initialise();                             // done
	void InitialisePoliceRadio();                  // done
	void InitialisePoliceRadioZones();             // done
	void InterrogateAudioEntities();               // done (inlined)
	bool IsAudioInitialised() const;               // done
	bool IsMissionAudioSampleFinished(uint8 slot); // done
	bool IsMP3RadioChannelAvailable() const;       // done

	bool MissionScriptAudioUsesPoliceChannel(int32 soundMission) const; //done

	void PlayLoadedMissionAudio(uint8 slot);                // done
	void PlayOneShot(int32 index, uint16 sound, float vol); // done
	void PlaySuspectLastSeen(float x, float y, float z);    // done
	void PlayerJustGotInCar() const;                        // done
	void PlayerJustLeftCar() const;                         // done
	void PostInitialiseGameSpecificSetup();                 // done
	void PostTerminateGameSpecificShutdown();               // done
	void PreInitialiseGameSpecificSetup() const;            // done
	void PreloadMissionAudio(uint8 slot, Const char *name); // done
	void PreTerminateGameSpecificShutdown();                // done
	/// processX - main logic of adding new sounds
	void ProcessActiveQueues();                              // done
	bool ProcessAirBrakes(cVehicleParams& params);           // done
	bool ProcessBoatEngine(cVehicleParams& params);          
	bool ProcessBoatMovingOverWater(cVehicleParams& params); //done 
#ifdef GTA_BRIDGE
	void ProcessBridge();         // done(bcs not exists in VC)
	void ProcessBridgeMotor();    // done(bcs not exists in VC)
	void ProcessBridgeOneShots(); // done(bcs not exists in VC)
	void ProcessBridgeWarning();  // done(bcs not exists in VC)
#endif
	bool ProcessCarBombTick(cVehicleParams& params);                         // done
	void ProcessCarHeli(cVehicleParams& params);                             // done
	void ProcessCesna(cVehicleParams& params);                               // done
	//void ProcessCrane();                                                   // done(bcs not exists in VC)
	bool ProcessEngineDamage(cVehicleParams& params);                        // done
	void ProcessEntity(int32 sound);                                         // done
	void ProcessExplosions(int32 explosion);                                 // done
	void ProcessFireHydrant();                                               // done
	void ProcessFires(int32 entity);                                         // done
	void ProcessFrontEnd();                                                  // done
	void ProcessGarages();                                                   // done
	void ProcessJumbo(cVehicleParams& params);                               // done
	void ProcessJumboAccel(CPlane *plane);                                   // done
	void ProcessJumboDecel(CPlane *plane);                                   // done
	void ProcessJumboFlying();                                               // done
	void ProcessJumboLanding(CPlane *plane);                                 // done
	void ProcessJumboTakeOff(CPlane *plane);                               	 // done
	void ProcessJumboTaxi();                                                 // done
	void ProcessLoopingScriptObject(uint8 sound);                            // done
	void ProcessMissionAudio();                                              // done
	void ProcessMissionAudioSlot(uint8 slot);                                // done
	void ProcessModelHeliVehicle(cVehicleParams& params);                    // done
	void ProcessModelVehicle(cVehicleParams& params);                        // done
	void ProcessOneShotScriptObject(uint8 sound);                            // 
	void ProcessPed(CPhysical *ped);                                         // done
	void ProcessPedOneShots(cPedParams &params);                             // 
	void ProcessPhysical(int32 id);                                          // done
	void ProcessPlane(cVehicleParams& params);                               // done
	void ProcessPlayerMood();                                                // done
	void ProcessPlayersVehicleEngine(cVehicleParams& params, CVehicle* veh); // done
	void ProcessProjectiles();                                               // done
	void ProcessRainOnVehicle(cVehicleParams& params);                       // done
	void ProcessReverb() const;                                              // done
	bool ProcessReverseGear(cVehicleParams& params);                         // done
	void ProcessScriptObject(int32 id);                                      // done
	void ProcessSpecial();                                                   // done
#ifdef GTA_TRAIN
	bool ProcessTrainNoise(cVehicleParams *params); //done(bcs not exists in VC)
#endif
	void ProcessVehicle(CVehicle *vehicle);                    // done
	bool ProcessVehicleDoors(cVehicleParams &params);          // done
	void ProcessVehicleEngine(cVehicleParams &params);         // done
	void ProcessVehicleFlatTyre(cVehicleParams &params);       // done
	bool ProcessVehicleHorn(cVehicleParams &params);           // done
	void ProcessVehicleOneShots(cVehicleParams &params);       // done
	bool ProcessVehicleReverseWarning(cVehicleParams &params); // done
	bool ProcessVehicleRoadNoise(cVehicleParams &params);      // done
	bool ProcessVehicleSirenOrAlarm(cVehicleParams &params);   // done
	bool ProcessVehicleSkidding(cVehicleParams &params);       // done
	void ProcessWaterCannon(int32);                            // done
	void ProcessWeather(int32 id);                             // done
	bool ProcessWetRoadNoise(cVehicleParams& params);          // done
	void ProcessEscalators();                                  // done
	void ProcessExtraSounds();                                 // done

	int32 RandomDisplacement(uint32 seed) const;                                                                                      // done
	void ReacquireDigitalHandle() const;                                                                                              // done
	void ReleaseDigitalHandle() const;                                                                                                // done
	void ReportCollision(CEntity *entity1, CEntity *entity2, uint8 surface1, uint8 surface2, float collisionPower, float intensity2); // done
	void ReportCrime(eCrimeType crime, const CVector &pos);                                                                           // done
	void ResetAudioLogicTimers(uint32 timer);                                                                                         // done
	void ResetPoliceRadio();                                                                                                          // done
	void ResetTimers(uint32 time);                                                                                                    // done

	void Service();                                                                             // done
	void ServiceCollisions();                                                                   // done
	void ServicePoliceRadio();                                                                  // done
	void ServicePoliceRadioChannel(uint8 wantedLevel);                                          // done
	void ServiceSoundEffects();                                                                 // done
	int8 SetCurrent3DProvider(uint8 which);                                                     // done
	void SetDynamicAcousticModelingStatus(uint8 status);                                        // done
	void SetEffectsFadeVol(uint8 volume) const;                                                 // done
	void SetEffectsMasterVolume(uint8 volume) const;                                            // done
	void SetMP3BoostVolume(uint8 volume) const;                                                 // done
	void SetEntityStatus(int32 id, uint8 status);                                               // done
	uint32 SetLoopingCollisionRequestedSfxFreqAndGetVol(const cAudioCollision &audioCollision); // done
	void SetMissionAudioLocation(uint8 slot, float x, float y, float z);                        // done
	void SetMissionScriptPoliceAudio(int32 sfx) const;                                          // inlined and optimized
	void SetMonoMode(uint8 mono);                                                               // done
	void SetMusicFadeVol(uint8 volume) const;                                                   // done
	void SetMusicMasterVolume(uint8 volume) const;                                              // done
	void SetSpeakerConfig(int32 conf) const;                                                    // done
	void SetUpLoopingCollisionSound(const cAudioCollision &col, uint8 counter);                 // done
	void SetUpOneShotCollisionSound(const cAudioCollision &col);                                // done
	bool SetupCrimeReport();                                                                    // done
	bool SetupJumboEngineSound(uint8 vol, uint32 freq);                                         // done
	bool SetupJumboFlySound(uint8 emittingVol);                                                 // done
	bool SetupJumboRumbleSound(uint8 emittingVol);                                              // done
	bool SetupJumboTaxiSound(uint8 vol);                                                        // done
	bool SetupJumboWhineSound(uint8 emittingVol, uint32 freq);                                  // done
	void SetupPedComments(cPedParams &params, uint16 sound);                                    // done
	void SetupSuspectLastSeenReport();

	void Terminate();                                           // done
	void TranslateEntity(Const CVector *v1, CVector *v2) const; // done

	void UpdateGasPedalAudio(CVehicle *veh, int vehType);  // done
	void UpdateReflections();                              // done
	bool UsesReverseWarning(int32 model) const;            // done
	bool UsesSiren(cVehicleParams &params) const;          // done
	bool UsesSirenSwitching(cVehicleParams &params) const; // done

	CVehicle *FindVehicleOfPlayer();                   // done
	void SetPedTalkingStatus(CPed *ped, uint8 status); // done
	void SetPlayersMood(uint8 mood, uint32 time);      // done

	float Sqrt(float v) const;

#ifdef GTA_PC
	// only used in pc
	void AdjustSamplesVolume();                                                     // done (inlined)
	uint8 ComputeEmittingVolume(uint8 emittingVolume, float intensity, float dist); // done (inlined)
#endif
};

#ifdef AUDIO_MSS
static_assert(sizeof(cAudioManager) == 0x5558, "cAudioManager: error");
#endif

extern cAudioManager AudioManager;
