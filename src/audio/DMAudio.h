#pragma once

#include "audio_enums.h"
#include "soundlist.h"
#include "Crime.h"

#define AEHANDLE_IS_FAILED(h) ((h)<0)
#define AEHANDLE_IS_OK(h)     ((h)>=0)

class cAudioScriptObject;
class CEntity;

class cDMAudio
{
public:
	~cDMAudio()
	{ }

	void Initialise(void);
	void Terminate(void);
	void Service(void);
	
	int32 CreateEntity(eAudioType type, void *UID);
	void DestroyEntity(int32 audioEntity);
	void SetEntityStatus(int32 audioEntity, uint8 status);
	void PlayOneShot(int32 audioEntity, uint16 oneShot, float volume);
	void DestroyAllGameCreatedEntities(void);
	
	void SetEffectsMasterVolume(uint8 volume);
	void SetMusicMasterVolume(uint8 volume);
	void SetEffectsFadeVol(uint8 volume);
	void SetMusicFadeVol(uint8 volume);
	
	uint8 GetNum3DProvidersAvailable(void);
	char *Get3DProviderName(uint8 id);
	
	int8 AutoDetect3DProviders(void);
	
	int8 GetCurrent3DProviderIndex(void);
	int8 SetCurrent3DProvider(uint8 which);
	
	void SetSpeakerConfig(int32 config);
	
	bool IsMP3RadioChannelAvailable(void);
	
	void ReleaseDigitalHandle(void);
	void ReacquireDigitalHandle(void);
	
	void SetDynamicAcousticModelingStatus(uint8 status);
	
	bool CheckForAnAudioFileOnCD(void);
	
	char GetCDAudioDriveLetter(void);
	bool IsAudioInitialised(void);
	
	void ReportCrime(eCrimeType crime, CVector const &pos);
	
	int32 CreateLoopingScriptObject(cAudioScriptObject *scriptObject);
	void DestroyLoopingScriptObject(int32 audioEntity);
	void CreateOneShotScriptObject(cAudioScriptObject *scriptObject);
	
	void PlaySuspectLastSeen(float x, float y, float z);
	
	void ReportCollision(CEntity *entityA, CEntity *entityB, uint8 surfaceTypeA, uint8 surfaceTypeB, float collisionPower, float velocity);
	
	void PlayFrontEndSound(uint16 frontend, uint32 volume);
	void PlayRadioAnnouncement(uint8 announcement);
	void PlayFrontEndTrack(uint8 track, uint8 frontendFlag);
	void StopFrontEndTrack(void);
	
	void ResetTimers(uint32 time);
	
	void ChangeMusicMode(uint8 mode);
	
	void PreloadCutSceneMusic(uint8 track);
	void PlayPreloadedCutSceneMusic(void);
	void StopCutSceneMusic(void);
	
	void PreloadMissionAudio(uint8 slot, Const char *missionAudio);
	uint8 GetMissionAudioLoadingStatus(uint8 slot);
	void SetMissionAudioLocation(uint8 slot, float x, float y, float z);
	void PlayLoadedMissionAudio(uint8 slot);
	bool IsMissionAudioSampleFinished(uint8 slot);
	void ClearMissionAudio(uint8 slot);

	uint8 GetRadioInCar(void);
	void SetRadioInCar(uint32 radio);
	void SetRadioChannel(int8 radio, int32 pos);
};
extern cDMAudio DMAudio;
