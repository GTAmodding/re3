#include "common.h"
#include "patcher.h"
#include "DMAudio.h"
#include "MusicManager.h"
#include "AudioManager.h"
#include "sampman.h"

cDMAudio &DMAudio = *(cDMAudio*)0x95CDBE;

void
cDMAudio::Initialise(void)
{
	AudioManager.Initialise();
}

void
cDMAudio::Terminate(void)
{
	AudioManager.Terminate();
}

void
cDMAudio::Service(void)
{
	AudioManager.Service();
}

int32
cDMAudio::CreateEntity(int32 type, void *UID)
{
	return AudioManager.CreateEntity(type, (CPhysical *)UID);
}

void
cDMAudio::DestroyEntity(int32 audioEntity)
{
	AudioManager.DestroyEntity(audioEntity);
}

void
cDMAudio::SetEntityStatus(int32 audioEntity, uint8 status)
{
	AudioManager.SetEntityStatus(audioEntity, status);
}

void
cDMAudio::PlayOneShot(int32 audioEntity, uint16 oneShot, float volume)
{
	AudioManager.PlayOneShot(audioEntity, oneShot, volume);
}

void
cDMAudio::DestroyAllGameCreatedEntities(void)
{
	AudioManager.DestroyAllGameCreatedEntities();
}

void
cDMAudio::SetEffectsMasterVolume(uint8 volume)
{
	uint8 vol = volume;
	if ( vol > MAX_VOLUME ) vol = MAX_VOLUME;
	
	AudioManager.SetEffectsMasterVolume(vol);
}

void
cDMAudio::SetMusicMasterVolume(uint8 volume)
{
	uint8 vol = volume;
	if ( vol > MAX_VOLUME ) vol = MAX_VOLUME;
	
	AudioManager.SetMusicMasterVolume(vol);
}

void
cDMAudio::SetEffectsFadeVol(uint8 volume)
{
	uint8 vol = volume;
	if ( vol > MAX_VOLUME ) vol = MAX_VOLUME;
	
	AudioManager.SetEffectsFadeVolume(vol);
}

void
cDMAudio::SetMusicFadeVol(uint8 volume)
{
	uint8 vol = volume;
	if ( vol > MAX_VOLUME ) vol = MAX_VOLUME;
	
	AudioManager.SetMusicFadeVolume(vol);
}

uint8
cDMAudio::GetNum3DProvidersAvailable(void)
{
	return AudioManager.GetNum3DProvidersAvailable();
}

char *
cDMAudio::Get3DProviderName(uint8 id)
{
	return AudioManager.Get3DProviderName(id);
}

int8
cDMAudio::GetCurrent3DProviderIndex(void)
{
	return AudioManager.GetCurrent3DProviderIndex();
}

int8
cDMAudio::SetCurrent3DProvider(uint8 which)
{
	return AudioManager.SetCurrent3DProvider(which);
}

void
cDMAudio::SetSpeakerConfig(int32 config)
{
	AudioManager.SetSpeakerConfig(config);
}

bool
cDMAudio::IsMP3RadioChannelAvailable(void)
{
	return AudioManager.IsMP3RadioChannelAvailable();
}

void
cDMAudio::ReleaseDigitalHandle(void)
{
	AudioManager.ReleaseDigitalHandle();
}

void
cDMAudio::ReacquireDigitalHandle(void)
{
	AudioManager.ReacquireDigitalHandle();
}

void
cDMAudio::SetDynamicAcousticModelingStatus(uint8 status)
{
	AudioManager.SetDynamicAcousticModelingStatus(status);
}

bool
cDMAudio::CheckForAnAudioFileOnCD(void)
{
	return AudioManager.CheckForAnAudioFileOnCD();
}

char
cDMAudio::GetCDAudioDriveLetter(void)
{
	return AudioManager.GetCDAudioDriveLetter();
}

bool
cDMAudio::IsAudioInitialised(void)
{
	return AudioManager.IsAudioInitialised();
}

void
cDMAudio::ReportCrime(eCrimeType crime, const CVector &pos)
{
	AudioManager.ReportCrime(crime, &pos);
}

int32
cDMAudio::CreateLoopingScriptObject(cAudioScriptObject *scriptObject)
{
	int32 audioEntity = AudioManager.CreateEntity(AUDIOTYPE_SCRIPTOBJECT, (CPhysical *)scriptObject);

	if ( AEHANDLE_IS_OK(audioEntity) )
		AudioManager.SetEntityStatus(audioEntity, true);
	
	return audioEntity;
}

void
cDMAudio::DestroyLoopingScriptObject(int32 audioEntity)
{
	AudioManager.DestroyEntity(audioEntity);
}

void
cDMAudio::CreateOneShotScriptObject(cAudioScriptObject *scriptObject)
{
	int32 audioEntity = AudioManager.CreateEntity(AUDIOTYPE_SCRIPTOBJECT, (CPhysical *)scriptObject);

	if ( AEHANDLE_IS_OK(audioEntity) )
	{
		AudioManager.SetEntityStatus(audioEntity, true);
		AudioManager.PlayOneShot(audioEntity, scriptObject->AudioId, 0.0f);
	}
}

void
cDMAudio::PlaySuspectLastSeen(float x, float y, float z)
{
	AudioManager.PlaySuspectLastSeen(x, y, z);
}

void
cDMAudio::ReportCollision(CEntity *entityA, CEntity *entityB, uint8 surfaceTypeA, uint8 surfaceTypeB, float collisionPower, float velocity)
{
	AudioManager.ReportCollision(entityA, entityB, surfaceTypeA, surfaceTypeB, collisionPower, velocity);
}

void
cDMAudio::PlayFrontEndSound(uint16 frontend, uint32 volume)
{
	AudioManager.PlayOneShot(AudioManager.m_nFrontEndEntity, frontend, (float)volume);
}

void
cDMAudio::PlayRadioAnnouncement(uint8 announcement)
{
	MusicManager.PlayAnnouncement(announcement);
}

void
cDMAudio::PlayFrontEndTrack(uint8 track, uint8 frontendFlag)
{
	MusicManager.PlayFrontEndTrack(track, frontendFlag);
}

void
cDMAudio::StopFrontEndTrack(void)
{
	MusicManager.StopFrontEndTrack();
}

void
cDMAudio::ResetTimers(uint32 time)
{
	AudioManager.ResetTimers(time);
}

void
cDMAudio::ChangeMusicMode(uint8 mode)
{
	MusicManager.ChangeMusicMode(mode);
}

void
cDMAudio::PreloadCutSceneMusic(uint8 track)
{
	MusicManager.PreloadCutSceneMusic(track);
}

void
cDMAudio::PlayPreloadedCutSceneMusic(void)
{
	MusicManager.PlayPreloadedCutSceneMusic();
}

void
cDMAudio::StopCutSceneMusic(void)
{
	MusicManager.StopCutSceneMusic();
}

void
cDMAudio::PreloadMissionAudio(char *missionAudio)
{
	AudioManager.PreloadMissionAudio(missionAudio);
}

uint8
cDMAudio::GetMissionAudioLoadingStatus(void)
{
	return AudioManager.GetMissionAudioLoadingStatus();
}

void
cDMAudio::SetMissionAudioLocation(float x, float y, float z)
{
	AudioManager.SetMissionAudioLocation(x, y, z);
}

void
cDMAudio::PlayLoadedMissionAudio(void)
{
	AudioManager.PlayLoadedMissionAudio();
}

bool
cDMAudio::IsMissionAudioSampleFinished(void)
{
	return AudioManager.IsMissionAudioSampleFinished();
}

void
cDMAudio::ClearMissionAudio(void)
{
	AudioManager.ClearMissionAudio();
}

int32
cDMAudio::GetRadioInCar(void)
{
	return MusicManager.GetRadioInCar();
}

void
cDMAudio::SetRadioInCar(uint32 radio)
{
	MusicManager.SetRadioInCar(radio);
}

void
cDMAudio::SetRadioChannel(int8 radio, int32 pos)
{
	MusicManager.SetRadioChannelByScript(radio, pos);
}

STARTPATCHES
	InjectHook(0x57C760, &cDMAudio::Initialise, PATCH_JUMP);
	InjectHook(0x57C780, &cDMAudio::Terminate, PATCH_JUMP);
	InjectHook(0x57C7A0, &cDMAudio::Service, PATCH_JUMP);
	InjectHook(0x57C7C0, &cDMAudio::CreateEntity, PATCH_JUMP);
	InjectHook(0x57C7F0, &cDMAudio::DestroyEntity, PATCH_JUMP);
	InjectHook(0x57C810, &cDMAudio::SetEntityStatus, PATCH_JUMP);
	InjectHook(0x57C840, &cDMAudio::PlayOneShot, PATCH_JUMP);
	InjectHook(0x57C870, &cDMAudio::DestroyAllGameCreatedEntities, PATCH_JUMP);
	InjectHook(0x57C890, &cDMAudio::SetEffectsMasterVolume, PATCH_JUMP);
	InjectHook(0x57C8C0, &cDMAudio::SetMusicMasterVolume, PATCH_JUMP);
	InjectHook(0x57C8F0, &cDMAudio::SetEffectsFadeVol, PATCH_JUMP);
	InjectHook(0x57C920, &cDMAudio::SetMusicFadeVol, PATCH_JUMP);
	InjectHook(0x57C950, &cDMAudio::GetNum3DProvidersAvailable, PATCH_JUMP);
	InjectHook(0x57C970, &cDMAudio::Get3DProviderName, PATCH_JUMP);
	InjectHook(0x57C990, &cDMAudio::GetCurrent3DProviderIndex, PATCH_JUMP);
	InjectHook(0x57C9B0, &cDMAudio::SetCurrent3DProvider, PATCH_JUMP);
	InjectHook(0x57C9D0, &cDMAudio::SetSpeakerConfig, PATCH_JUMP);
	InjectHook(0x57C9F0, &cDMAudio::IsMP3RadioChannelAvailable, PATCH_JUMP);
	InjectHook(0x57CA10, &cDMAudio::ReleaseDigitalHandle, PATCH_JUMP);
	InjectHook(0x57CA30, &cDMAudio::ReacquireDigitalHandle, PATCH_JUMP);
	InjectHook(0x57CA50, &cDMAudio::SetDynamicAcousticModelingStatus, PATCH_JUMP);
	InjectHook(0x57CA70, &cDMAudio::CheckForAnAudioFileOnCD, PATCH_JUMP);
	InjectHook(0x57CA90, &cDMAudio::GetCDAudioDriveLetter, PATCH_JUMP);
	InjectHook(0x57CAB0, &cDMAudio::IsAudioInitialised, PATCH_JUMP);
	InjectHook(0x57CAD0, &cDMAudio::ReportCrime, PATCH_JUMP);
	InjectHook(0x57CB00, &cDMAudio::CreateLoopingScriptObject, PATCH_JUMP);
	InjectHook(0x57CB40, &cDMAudio::DestroyLoopingScriptObject, PATCH_JUMP);
	InjectHook(0x57CB60, &cDMAudio::CreateOneShotScriptObject, PATCH_JUMP);
	InjectHook(0x57CBB0, &cDMAudio::PlaySuspectLastSeen, PATCH_JUMP);
	InjectHook(0x57CBE0, &cDMAudio::ReportCollision, PATCH_JUMP);
	InjectHook(0x57CC20, &cDMAudio::PlayFrontEndSound, PATCH_JUMP);
	InjectHook(0x57CC60, &cDMAudio::PlayRadioAnnouncement, PATCH_JUMP);
	InjectHook(0x57CC80, &cDMAudio::PlayFrontEndTrack, PATCH_JUMP);
	InjectHook(0x57CCB0, &cDMAudio::StopFrontEndTrack, PATCH_JUMP);
	InjectHook(0x57CCD0, &cDMAudio::ResetTimers, PATCH_JUMP);
	InjectHook(0x57CCF0, &cDMAudio::ChangeMusicMode, PATCH_JUMP);
	InjectHook(0x57CD10, &cDMAudio::PreloadCutSceneMusic, PATCH_JUMP);
	InjectHook(0x57CD30, &cDMAudio::PlayPreloadedCutSceneMusic, PATCH_JUMP);
	InjectHook(0x57CD50, &cDMAudio::StopCutSceneMusic, PATCH_JUMP);
	InjectHook(0x57CD70, &cDMAudio::PreloadMissionAudio, PATCH_JUMP);
	InjectHook(0x57CD90, &cDMAudio::GetMissionAudioLoadingStatus, PATCH_JUMP);
	InjectHook(0x57CDB0, &cDMAudio::SetMissionAudioLocation, PATCH_JUMP);
	InjectHook(0x57CDE0, &cDMAudio::PlayLoadedMissionAudio, PATCH_JUMP);
	InjectHook(0x57CE00, &cDMAudio::IsMissionAudioSampleFinished, PATCH_JUMP);
	InjectHook(0x57CE20, &cDMAudio::ClearMissionAudio, PATCH_JUMP);
	InjectHook(0x57CE40, &cDMAudio::GetRadioInCar, PATCH_JUMP);
	InjectHook(0x57CE60, &cDMAudio::SetRadioInCar, PATCH_JUMP);
	InjectHook(0x57CE80, &cDMAudio::SetRadioChannel, PATCH_JUMP);
	
	//InjectHook(0x57CEB0, `global constructor keyed to'dmaudio.cpp, PATCH_JUMP);
	//InjectHook(0x57CED0, cDMAudio::~cDMAudio, PATCH_JUMP);
ENDPATCHES
