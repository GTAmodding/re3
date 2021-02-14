#include "common.h"

#include "AudioManager.h"
#include "audio_enums.h"

#include "AudioScriptObject.h"
#include "MusicManager.h"
#include "Timer.h"
#include "DMAudio.h"
#include "sampman.h"
#include "Camera.h"
#include "World.h"

cAudioManager AudioManager;

const int channels = ARRAY_SIZE(AudioManager.m_asActiveSamples);
const int policeChannel = channels + 1;
const int allChannels = channels + 2;

#define SPEED_OF_SOUND 343.f
#define TIME_SPENT 50

cAudioManager::cAudioManager()
{
	m_bIsInitialised = false;
	m_bReverb = true;
	m_fSpeedOfSound = SPEED_OF_SOUND / TIME_SPENT;
	m_nTimeSpent = TIME_SPENT;
	m_nActiveSamples = NUM_SOUNDS_SAMPLES_SLOTS;
	m_nActiveSampleQueue = 1;
	ClearRequestedQueue();
	m_nActiveSampleQueue = 0;
	ClearRequestedQueue();
	ClearActiveSamples();
	GenerateIntegerRandomNumberTable();
	field_4 = 0;
	m_bDynamicAcousticModelingStatus = true;

	for (int i = 0; i < NUM_AUDIOENTITIES; i++) {
		m_asAudioEntities[i].m_bIsUsed = false;
		m_anAudioEntityIndices[i] = NUM_AUDIOENTITIES;
	}
	m_nAudioEntitiesTotal = 0;
	m_FrameCounter = 0;
	m_bFifthFrameFlag = false;
	m_bTimerJustReset = false;
	m_nTimer = 0;
}

cAudioManager::~cAudioManager()
{
	if (m_bIsInitialised)
		Terminate();
}

void
cAudioManager::Initialise()
{
	if (!m_bIsInitialised) {
		PreInitialiseGameSpecificSetup();
		m_bIsInitialised = SampleManager.Initialise();
		if (m_bIsInitialised) {
			m_nActiveSamples = SampleManager.GetMaximumSupportedChannels();
			if (m_nActiveSamples <= 1) {
				Terminate();
			} else {
				--m_nActiveSamples;
				PostInitialiseGameSpecificSetup();
				InitialisePoliceRadioZones();
				InitialisePoliceRadio();
				MusicManager.Initialise();
			}
		}
	}
}

void
cAudioManager::Terminate()
{
	if (m_bIsInitialised) {
		MusicManager.Terminate();

		for (uint32 i = 0; i < NUM_AUDIOENTITIES; i++) {
			m_asAudioEntities[i].m_bIsUsed = false;
			m_anAudioEntityIndices[i] = ARRAY_SIZE(m_anAudioEntityIndices);
		}

		m_nAudioEntitiesTotal = 0;
		m_sAudioScriptObjectManager.m_nScriptObjectEntityTotal = 0;
		PreTerminateGameSpecificShutdown();

		for (uint32 i = 0; i < MAX_SFX_BANKS; i++) {
			if (SampleManager.IsSampleBankLoaded(i))
				SampleManager.UnloadSampleBank(i);
		}

		SampleManager.Terminate();

		m_bIsInitialised = false;
		PostTerminateGameSpecificShutdown();
	}
}

void
cAudioManager::Service()
{
	GenerateIntegerRandomNumberTable();
	if (m_bTimerJustReset) {
		ResetAudioLogicTimers(m_nTimer);
		MusicManager.ResetTimers(m_nTimer);
		m_bTimerJustReset = false;
	}
	if (m_bIsInitialised) {
		m_nPreviousUserPause = m_nUserPause;
		m_nUserPause = CTimer::GetIsUserPaused();
		UpdateReflections();
		ServiceSoundEffects();
		MusicManager.Service();
	}
}

int32
cAudioManager::CreateEntity(eAudioType type, void *entity)
{
	if (!m_bIsInitialised)
		return AEHANDLE_ERROR_NOAUDIOSYS;
	if (!entity)
		return AEHANDLE_ERROR_NOENTITY;
	if (type >= TOTAL_AUDIO_TYPES)
		return AEHANDLE_ERROR_BADAUDIOTYPE;
	for (uint32 i = 0; i < ARRAY_SIZE(m_asAudioEntities); i++) {
		if (!m_asAudioEntities[i].m_bIsUsed) {
			m_asAudioEntities[i].m_bIsUsed = true;
			m_asAudioEntities[i].m_bStatus = false;
			m_asAudioEntities[i].m_nType = type;
			m_asAudioEntities[i].m_pEntity = entity;
			m_asAudioEntities[i].m_awAudioEvent[0] = SOUND_NO_SOUND;
			m_asAudioEntities[i].m_awAudioEvent[1] = SOUND_NO_SOUND;
			m_asAudioEntities[i].m_awAudioEvent[2] = SOUND_NO_SOUND;
			m_asAudioEntities[i].m_awAudioEvent[3] = SOUND_NO_SOUND;
			m_asAudioEntities[i].m_AudioEvents = 0;
			m_anAudioEntityIndices[m_nAudioEntitiesTotal++] = i;
			return i;
		}
	}
	return AEHANDLE_ERROR_NOFREESLOT;
}

void
cAudioManager::DestroyEntity(int32 id)
{
	if (m_bIsInitialised && id >= 0 && id < NUM_AUDIOENTITIES && m_asAudioEntities[id].m_bIsUsed) {
		m_asAudioEntities[id].m_bIsUsed = false;
		for (int32 i = 0; i < m_nAudioEntitiesTotal; ++i) {
			if (id == m_anAudioEntityIndices[i]) {
				if (i < NUM_AUDIOENTITIES - 1)
					memmove(&m_anAudioEntityIndices[i], &m_anAudioEntityIndices[i + 1], NUM_AUDIOENTITY_EVENTS * (m_nAudioEntitiesTotal - (i + 1)));
				m_anAudioEntityIndices[--m_nAudioEntitiesTotal] = NUM_AUDIOENTITIES;
				return;
			}
		}
	}
}

void
cAudioManager::SetEntityStatus(int32 id, uint8 status)
{
	if (m_bIsInitialised && id >= 0 && id < NUM_AUDIOENTITIES && m_asAudioEntities[id].m_bIsUsed)
		m_asAudioEntities[id].m_bStatus = status;
}

void
cAudioManager::PlayOneShot(int32 index, uint16 sound, float vol)
{
	static const uint8 OneShotPriority[] = {3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 5, 5, 5, 3, 5, 2, 2, 1, 1, 3, 1, 3, 3, 1, 1, 1, 4, 4, 3, 1, 1,
	                                        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 6, 1, 1, 3, 2, 2, 2, 2, 0, 0, 6, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	                                        1, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 3, 1, 1, 1, 9,
	                                        2, 2, 0, 0, 0, 0, 3, 3, 5, 1, 1, 1, 1, 3, 4, 7, 6, 6, 6, 6, 1, 3, 4, 3, 4, 2, 1, 3, 5, 4, 6, 6, 1, 3,
	                                        1, 1, 1, 0, 0, 0, 0, 0, 0, 3, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

	if (m_bIsInitialised) {
		if (index >= 0 && index < NUM_AUDIOENTITIES) {
			tAudioEntity &entity = m_asAudioEntities[index];
			if (entity.m_bIsUsed) {
				if (sound < SOUND_TOTAL_SOUNDS) {
					if (entity.m_nType == AUDIOTYPE_SCRIPTOBJECT) {
						if (m_sAudioScriptObjectManager.m_nScriptObjectEntityTotal < ARRAY_SIZE(m_sAudioScriptObjectManager.m_anScriptObjectEntityIndices)) {
							entity.m_awAudioEvent[0] = sound;
							entity.m_AudioEvents = 1;
							m_sAudioScriptObjectManager.m_anScriptObjectEntityIndices[m_sAudioScriptObjectManager.m_nScriptObjectEntityTotal++] = index;
						}
					} else {
						int32 i = 0;
						while (true) {
							if (i >= entity.m_AudioEvents) {
								if (entity.m_AudioEvents < ARRAY_SIZE(entity.m_awAudioEvent)) {
									entity.m_awAudioEvent[i] = sound;
									entity.m_afVolume[i] = vol;
									++entity.m_AudioEvents;
								}
								return;
							}
							if (OneShotPriority[entity.m_awAudioEvent[i]] > OneShotPriority[sound])
								break;
							++i;
						}
						if (i < NUM_AUDIOENTITY_EVENTS - 1) {
							memmove(&entity.m_awAudioEvent[i + 1], &entity.m_awAudioEvent[i], (NUM_AUDIOENTITY_EVENTS - 1 - i) * NUM_AUDIOENTITY_EVENTS / 2);
							memmove(&entity.m_afVolume[i + 1], &entity.m_afVolume[i], (NUM_AUDIOENTITY_EVENTS - 1 - i) * NUM_AUDIOENTITY_EVENTS);
						}
						entity.m_awAudioEvent[i] = sound;
						entity.m_afVolume[i] = vol;
						if (entity.m_AudioEvents < ARRAY_SIZE(entity.m_awAudioEvent))
							++entity.m_AudioEvents;
					}
				}
			}
		}
	}
}

void
cAudioManager::SetEffectsMasterVolume(uint8 volume) const
{
	SampleManager.SetEffectsMasterVolume(volume);
}

void
cAudioManager::SetMusicMasterVolume(uint8 volume) const
{
	SampleManager.SetMusicMasterVolume(volume);
}

void
cAudioManager::SetEffectsFadeVol(uint8 volume) const
{
	SampleManager.SetEffectsFadeVolume(volume);
}

void
cAudioManager::SetMonoMode(uint8 mono)
{
	SampleManager.SetMonoMode(mono);
}

void
cAudioManager::SetMusicFadeVol(uint8 volume) const
{
	SampleManager.SetMusicFadeVolume(volume);
}

void
cAudioManager::ResetTimers(uint32 time)
{
	if (m_bIsInitialised) {
		m_bTimerJustReset = true;
		m_nTimer = time;
		ClearRequestedQueue();
		if (m_nActiveSampleQueue) {
			m_nActiveSampleQueue = 0;
			ClearRequestedQueue();
			m_nActiveSampleQueue = 1;
		} else {
			m_nActiveSampleQueue = 1;
			ClearRequestedQueue();
			m_nActiveSampleQueue = 0;
		}
		ClearActiveSamples();
		ClearMissionAudio();
		SampleManager.StopChannel(policeChannel);
		SampleManager.SetEffectsFadeVolume(0);
		SampleManager.SetMusicFadeVolume(0);
		MusicManager.ResetMusicAfterReload();
#ifdef AUDIO_OAL
		SampleManager.Service();
#endif
	}
}

void
cAudioManager::DestroyAllGameCreatedEntities()
{
	cAudioScriptObject *entity;

	if (m_bIsInitialised) {
		for (uint32 i = 0; i < ARRAY_SIZE(m_asAudioEntities); i++) {
			if (m_asAudioEntities[i].m_bIsUsed) {
				switch (m_asAudioEntities[i].m_nType) {
				case AUDIOTYPE_PHYSICAL:
				case AUDIOTYPE_EXPLOSION:
				case AUDIOTYPE_WEATHER:
				case AUDIOTYPE_CRANE:
				case AUDIOTYPE_GARAGE:
				case AUDIOTYPE_FIREHYDRANT:
					DestroyEntity(i);
					break;
				case AUDIOTYPE_SCRIPTOBJECT:
					entity = (cAudioScriptObject *)m_asAudioEntities[i].m_pEntity;
					if (entity) {
						delete entity;
						m_asAudioEntities[i].m_pEntity = nil;
					}
					DestroyEntity(i);
					break;
				default:
					break;
				}
			}
		}
		m_sAudioScriptObjectManager.m_nScriptObjectEntityTotal = 0;
	}
}

uint8
cAudioManager::GetNum3DProvidersAvailable() const
{
	if (m_bIsInitialised)
		return SampleManager.GetNum3DProvidersAvailable();
	return 0;
}

char *
cAudioManager::Get3DProviderName(uint8 id) const
{
	if (!m_bIsInitialised)
		return nil;
#ifdef AUDIO_OAL
	id = clamp(id, 0, SampleManager.GetNum3DProvidersAvailable() - 1);
#else
	// We don't want that either since it will crash the game, but skipping for now
	if (id >= SampleManager.GetNum3DProvidersAvailable())
		return nil;
#endif
	return SampleManager.Get3DProviderName(id);
}

int8
cAudioManager::GetCurrent3DProviderIndex() const
{
	if (m_bIsInitialised)
		return SampleManager.GetCurrent3DProviderIndex();

	return -1;
}

int8
cAudioManager::SetCurrent3DProvider(uint8 which)
{
	if (!m_bIsInitialised)
		return -1;
	for (uint8 i = 0; i < m_nActiveSamples + 1; ++i)
		SampleManager.StopChannel(i);
	ClearRequestedQueue();
	if (m_nActiveSampleQueue == 0)
		m_nActiveSampleQueue = 1;
	else
		m_nActiveSampleQueue = 0;
	ClearRequestedQueue();
	ClearActiveSamples();
	int8 current = SampleManager.SetCurrent3DProvider(which);
	if (current > 0) {
		m_nActiveSamples = SampleManager.GetMaximumSupportedChannels();
		if (m_nActiveSamples > 1)
			--m_nActiveSamples;
	}
	return current;
}

void
cAudioManager::SetSpeakerConfig(int32 conf) const
{
	SampleManager.SetSpeakerConfig(conf);
}

bool
cAudioManager::IsMP3RadioChannelAvailable() const
{
	if (m_bIsInitialised)
		return SampleManager.IsMP3RadioChannelAvailable();

	return false;
}

void
cAudioManager::ReleaseDigitalHandle() const
{
	if (m_bIsInitialised) {
		SampleManager.ReleaseDigitalHandle();
	}
}

void
cAudioManager::ReacquireDigitalHandle() const
{
	if (m_bIsInitialised) {
		SampleManager.ReacquireDigitalHandle();
	}
}

void
cAudioManager::SetDynamicAcousticModelingStatus(uint8 status)
{
	m_bDynamicAcousticModelingStatus = status!=0;
}

bool
cAudioManager::CheckForAnAudioFileOnCD() const
{
	return SampleManager.CheckForAnAudioFileOnCD();
}

uint8
cAudioManager::GetCDAudioDriveLetter() const
{
	if (m_bIsInitialised)
		return SampleManager.GetCDAudioDriveLetter();

	return 0;
}

bool
cAudioManager::IsAudioInitialised() const
{
	return m_bIsInitialised;
}

void
cAudioManager::ServiceSoundEffects()
{
	m_bFifthFrameFlag = (m_FrameCounter++ % 5) == 0;
	if (m_nUserPause && !m_nPreviousUserPause) {
		for (int32 i = 0; i < allChannels; i++)
			SampleManager.StopChannel(i);

		ClearRequestedQueue();
		if (m_nActiveSampleQueue) {
			m_nActiveSampleQueue = 0;
			ClearRequestedQueue();
			m_nActiveSampleQueue = 1;
		} else {
			m_nActiveSampleQueue = 1;
			ClearRequestedQueue();
			m_nActiveSampleQueue = 0;
		}
		ClearActiveSamples();
	}
	m_nActiveSampleQueue = m_nActiveSampleQueue == 1 ? 0 : 1;
	ProcessReverb();
	ProcessSpecial();
	ClearRequestedQueue();
	InterrogateAudioEntities();
	m_sPedComments.Process();
	ServicePoliceRadio();
	ServiceCollisions();
	AddReleasingSounds();
	ProcessMissionAudio();
#ifdef GTA_PC
	AdjustSamplesVolume();
#endif
	ProcessActiveQueues();
#ifdef AUDIO_OAL
	SampleManager.Service();
#endif
	for (int32 i = 0; i < m_sAudioScriptObjectManager.m_nScriptObjectEntityTotal; ++i) {
		cAudioScriptObject *object = (cAudioScriptObject *)m_asAudioEntities[m_sAudioScriptObjectManager.m_anScriptObjectEntityIndices[i]].m_pEntity;
		delete object;
		m_asAudioEntities[m_sAudioScriptObjectManager.m_anScriptObjectEntityIndices[i]].m_pEntity = nil;
		DestroyEntity(m_sAudioScriptObjectManager.m_anScriptObjectEntityIndices[i]);
	}
	m_sAudioScriptObjectManager.m_nScriptObjectEntityTotal = 0;
}

uint8
cAudioManager::ComputeVolume(uint8 emittingVolume, float soundIntensity, float distance) const
{
	float newSoundIntensity;
	if (soundIntensity <= 0.0f)
		return 0;
	newSoundIntensity = soundIntensity / 5.0f;
	if (newSoundIntensity <= distance)
		emittingVolume = sq((soundIntensity - newSoundIntensity - (distance - newSoundIntensity)) / (soundIntensity - newSoundIntensity)) * emittingVolume;
	return emittingVolume;
}

void
cAudioManager::TranslateEntity(Const CVector *in, CVector *out) const
{
	*out = MultiplyInverse(TheCamera.GetMatrix(), *in);
}

int32
cAudioManager::ComputePan(float dist, CVector *vec)
{
	const uint8 PanTable[64] = {0,  3,  8,  12, 16, 19, 22, 24, 26, 28, 30, 31, 33, 34, 36, 37, 39, 40, 41, 42, 44, 45, 46, 47, 48, 49, 49, 50, 51, 52, 53, 53,
	                            54, 55, 55, 56, 56, 57, 57, 58, 58, 58, 59, 59, 59, 60, 60, 61, 61, 61, 61, 62, 62, 62, 62, 62, 63, 63, 63, 63, 63, 63, 63, 63};

	int32 index = Min(63, Abs(vec->x / (dist / 64.f)));

	if (vec->x > 0.f)
		return Max(20, 63 - PanTable[index]);
	return Min(107, PanTable[index] + 63);
}

int32
cAudioManager::ComputeDopplerEffectedFrequency(uint32 oldFreq, float position1, float position2, float speedMultiplier) const
{
	uint32 newFreq = oldFreq;
	if (!TheCamera.Get_Just_Switched_Status() && speedMultiplier != 0.0f) {
		float dist = position2 - position1;
		if (dist != 0.0f) {
			float speedOfSource = (dist / m_nTimeSpent) * speedMultiplier;
			if (m_fSpeedOfSound > Abs(speedOfSource)) {
				if (speedOfSource < 0.0f) {
					speedOfSource = Max(speedOfSource, -1.5f);
				} else {
					speedOfSource = Min(speedOfSource, 1.5f);
				}
				newFreq = (oldFreq * m_fSpeedOfSound) / (speedOfSource + m_fSpeedOfSound);
			}
		}
	}
	return newFreq;
}

int32
cAudioManager::RandomDisplacement(uint32 seed) const
{
	int32 value;

	static bool bPos = true;
	static uint32 Adjustment = 0;

	if (!seed)
		return 0;

	value = m_anRandomTable[(Adjustment + seed) % 5] % seed;
	Adjustment += value;

	if (value % 2) {
		bPos = !bPos;
	}
	if (!bPos)
		value = -value;
	return value;
}

void
cAudioManager::InterrogateAudioEntities()
{
	for (int32 i = 0; i < m_nAudioEntitiesTotal; i++) {
		ProcessEntity(m_anAudioEntityIndices[i]);
		m_asAudioEntities[m_anAudioEntityIndices[i]].m_AudioEvents = 0;
	}
}

void
cAudioManager::AddSampleToRequestedQueue()
{
	int32 calculatedVolume;
	uint8 sampleIndex;
	bool bReflections;

	if (m_sQueueSample.m_nSampleIndex < TOTAL_AUDIO_SAMPLES) {
		calculatedVolume = m_sQueueSample.m_nReleasingVolumeModificator * (MAX_VOLUME - m_sQueueSample.m_nVolume);
		sampleIndex = m_SampleRequestQueuesStatus[m_nActiveSampleQueue];
		if (sampleIndex >= m_nActiveSamples) {
			sampleIndex = m_abSampleQueueIndexTable[m_nActiveSampleQueue][m_nActiveSamples - 1];
			if (m_asSamples[m_nActiveSampleQueue][sampleIndex].m_nCalculatedVolume <= calculatedVolume)
				return;
		} else {
			++m_SampleRequestQueuesStatus[m_nActiveSampleQueue];
		}
		m_sQueueSample.m_nCalculatedVolume = calculatedVolume;
		m_sQueueSample.m_bLoopEnded = false;
		if (m_sQueueSample.m_bIs2D) {
			m_sQueueSample.m_bRequireReflection = false;
			m_sQueueSample.m_nLoopsRemaining = 0;
		}
		if (m_bDynamicAcousticModelingStatus && m_sQueueSample.m_nLoopCount) {
			bReflections = m_sQueueSample.m_bRequireReflection;
		} else {
			bReflections = false;
			m_sQueueSample.m_nLoopsRemaining = 0;
		}
		m_sQueueSample.m_bRequireReflection = false;

		if (!m_bDynamicAcousticModelingStatus)
			m_sQueueSample.m_bReverbFlag = false;

		m_asSamples[m_nActiveSampleQueue][sampleIndex] = m_sQueueSample;

		AddDetailsToRequestedOrderList(sampleIndex);
		if (bReflections)
			AddReflectionsToRequestedQueue();
	}
}
void
cAudioManager::AddDetailsToRequestedOrderList(uint8 sample)
{
	uint32 i = 0;
	if (sample != 0) {
		for (; i < sample; i++) {
			if (m_asSamples[m_nActiveSampleQueue][m_abSampleQueueIndexTable[m_nActiveSampleQueue][i]].m_nCalculatedVolume >
			    m_asSamples[m_nActiveSampleQueue][sample].m_nCalculatedVolume)
				break;
		}
		if (i < sample) {
			memmove(&m_abSampleQueueIndexTable[m_nActiveSampleQueue][i + 1], &m_abSampleQueueIndexTable[m_nActiveSampleQueue][i], m_nActiveSamples - i - 1);
		}
	}
	m_abSampleQueueIndexTable[m_nActiveSampleQueue][i] = sample;
}

void
cAudioManager::AddReflectionsToRequestedQueue()
{
	float reflectionDistance;
	int32 noise;
	uint8 emittingVolume = (m_sQueueSample.m_nVolume / 2) + (m_sQueueSample.m_nVolume / 8);

	for (uint32 i = 0; i < ARRAY_SIZE(m_afReflectionsDistances); i++) {
		reflectionDistance = m_afReflectionsDistances[i];
		if (reflectionDistance > 0.0f && reflectionDistance < 100.f && reflectionDistance < m_sQueueSample.m_fSoundIntensity) {
			m_sQueueSample.m_nLoopsRemaining = (reflectionDistance * 500.f / 1029.f);
			if (m_sQueueSample.m_nLoopsRemaining > 5) {
				m_sQueueSample.m_fDistance = m_afReflectionsDistances[i];
				m_sQueueSample.m_nEmittingVolume = emittingVolume;
				m_sQueueSample.m_nVolume = ComputeVolume(emittingVolume, m_sQueueSample.m_fSoundIntensity, m_sQueueSample.m_fDistance);
				if (m_sQueueSample.m_nVolume > emittingVolume / 16) {
					m_sQueueSample.m_nCounter += (i + 1) * 256;
					if (m_sQueueSample.m_nLoopCount) {
						noise = RandomDisplacement(m_sQueueSample.m_nFrequency / 32);
						if (noise <= 0)
							m_sQueueSample.m_nFrequency += noise;
						else
							m_sQueueSample.m_nFrequency -= noise;
					}
					m_sQueueSample.m_nReleasingVolumeModificator += 20;
					m_sQueueSample.m_vecPos = m_avecReflectionsPos[i];
					AddSampleToRequestedQueue();
				}
			}
		}
	}
}

void
cAudioManager::UpdateReflections()
{
	const CVector &camPos = TheCamera.GetPosition();
	CColPoint colpoint;
	CEntity *ent;

	if (m_FrameCounter % 8 == 0) {
		m_avecReflectionsPos[0] = camPos;
		m_avecReflectionsPos[0].y += 50.f;
		if (CWorld::ProcessLineOfSight(camPos, m_avecReflectionsPos[0], colpoint, ent, true, false, false, true, false, true, true))
			m_afReflectionsDistances[0] = Distance(camPos, colpoint.point);
		else
			m_afReflectionsDistances[0] = 50.0f;
	} else if ((m_FrameCounter + 1) % 8 == 0) {
		m_avecReflectionsPos[1] = camPos;
		m_avecReflectionsPos[1].y -= 50.0f;
		if (CWorld::ProcessLineOfSight(camPos, m_avecReflectionsPos[1], colpoint, ent, true, false, false, true, false, true, true))
			m_afReflectionsDistances[1] = Distance(camPos, colpoint.point);
		else
			m_afReflectionsDistances[1] = 50.0f;
	} else if ((m_FrameCounter + 2) % 8 == 0) {
		m_avecReflectionsPos[2] = camPos;
		m_avecReflectionsPos[2].x -= 50.0f;
		if (CWorld::ProcessLineOfSight(camPos, m_avecReflectionsPos[2], colpoint, ent, true, false, false, true, false, true, true))
			m_afReflectionsDistances[2] = Distance(camPos, colpoint.point);
		else
			m_afReflectionsDistances[2] = 50.0f;
	} else if ((m_FrameCounter + 3) % 8 == 0) {
		m_avecReflectionsPos[3] = camPos;
		m_avecReflectionsPos[3].x += 50.0f;
		if (CWorld::ProcessLineOfSight(camPos, m_avecReflectionsPos[3], colpoint, ent, true, false, false, true, false, true, true))
			m_afReflectionsDistances[3] = Distance(camPos, colpoint.point);
		else
			m_afReflectionsDistances[3] = 50.0f;
	} else if ((m_FrameCounter + 4) % 8 == 0) {
		m_avecReflectionsPos[4] = camPos;
		m_avecReflectionsPos[4].z += 50.0f;
		if (CWorld::ProcessVerticalLine(camPos, m_avecReflectionsPos[4].z, colpoint, ent, true, false, false, false, true, false, nil))
			m_afReflectionsDistances[4] = colpoint.point.z - camPos.z;
		else
			m_afReflectionsDistances[4] = 50.0f;
	}
}

void
cAudioManager::AddReleasingSounds()
{
	bool toProcess[44]; // why not 27?

	int8 queue = m_nActiveSampleQueue == 0 ? 1 : 0;

	for (int32 i = 0; i < m_SampleRequestQueuesStatus[queue]; i++) {
		tSound &sample = m_asSamples[queue][m_abSampleQueueIndexTable[queue][i]];
		if (sample.m_bLoopEnded)
			continue;

		toProcess[i] = false;
		for (int32 j = 0; j < m_SampleRequestQueuesStatus[m_nActiveSampleQueue]; j++) {
			if (sample.m_nEntityIndex == m_asSamples[m_nActiveSampleQueue][m_abSampleQueueIndexTable[m_nActiveSampleQueue][j]].m_nEntityIndex &&
			    sample.m_nCounter == m_asSamples[m_nActiveSampleQueue][m_abSampleQueueIndexTable[m_nActiveSampleQueue][j]].m_nCounter) {
				toProcess[i] = true;
				break;
			}
		}
		if (!toProcess[i]) {
			if (sample.m_nCounter <= 255 || !sample.m_nLoopsRemaining) {
				if (!sample.m_nReleasingVolumeDivider)
					continue;
				if (!sample.m_nLoopCount) {
					if (sample.m_nVolumeChange == -1) {
						sample.m_nVolumeChange = sample.m_nVolume / sample.m_nReleasingVolumeDivider;
						if (sample.m_nVolumeChange <= 0)
							sample.m_nVolumeChange = 1;
					}
					if (sample.m_nVolume <= sample.m_nVolumeChange) {
						sample.m_nReleasingVolumeDivider = 0;
						continue;
					}
					sample.m_nVolume -= sample.m_nVolumeChange;
				}
				--sample.m_nReleasingVolumeDivider;
				if (m_bFifthFrameFlag) {
					if (sample.m_nReleasingVolumeModificator < 20)
						++sample.m_nReleasingVolumeModificator;
				}
				sample.m_bReleasingSoundFlag = 0;
			}
			memcpy(&m_sQueueSample, &sample, sizeof(tSound));
			AddSampleToRequestedQueue();
		}
	}
}

void
cAudioManager::ProcessActiveQueues()
{
	bool flag;
	float position2;
	float position1;

	uint32 v28;
	uint32 v29;

	float x;
	float usedX;
	float usedY;
	float usedZ;

	uint8 vol;
	uint8 emittingVol;
	CVector position;

	for (int32 i = 0; i < m_nActiveSamples; i++) {
		m_asSamples[m_nActiveSampleQueue][i].m_bIsProcessed = false;
		m_asActiveSamples[i].m_bIsProcessed = false;
	}

	for (int32 i = 0; i < m_SampleRequestQueuesStatus[m_nActiveSampleQueue]; ++i) {
		tSound &sample = m_asSamples[m_nActiveSampleQueue][m_abSampleQueueIndexTable[m_nActiveSampleQueue][i]];
		if (sample.m_nSampleIndex != NO_SAMPLE) {
			for (int32 j = 0; j < m_nActiveSamples; ++j) {
				if (sample.m_nEntityIndex == m_asActiveSamples[j].m_nEntityIndex && sample.m_nCounter == m_asActiveSamples[j].m_nCounter &&
				    sample.m_nSampleIndex == m_asActiveSamples[j].m_nSampleIndex) {
					if (sample.m_nLoopCount) {
						if (m_FrameCounter & 1) {
							flag = !!(j & 1);
						} else {
							flag = !(j & 1);
						}
						if (flag && !SampleManager.GetChannelUsedFlag(j)) {
							sample.m_bLoopEnded = true;
							m_asActiveSamples[j].m_bLoopEnded = true;
							m_asActiveSamples[j].m_nSampleIndex = NO_SAMPLE;
							m_asActiveSamples[j].m_nEntityIndex = AEHANDLE_NONE;
							continue;
						}
					}
					sample.m_bIsProcessed = true;
					m_asActiveSamples[j].m_bIsProcessed = true;
					sample.m_nVolumeChange = -1;
					if (!sample.m_bReleasingSoundFlag) {
						if (sample.m_bIs2D) {
							if (field_4) {
								emittingVol = 2 * Min(63, sample.m_nEmittingVolume);
							} else {
								emittingVol = sample.m_nEmittingVolume;
							}
							SampleManager.SetChannelFrequency(j, sample.m_nFrequency);
							SampleManager.SetChannelEmittingVolume(j, emittingVol);
						} else {
							m_asActiveSamples[j].m_fDistance = sample.m_fDistance;
							position2 = sample.m_fDistance;
							position1 = m_asActiveSamples[j].m_fDistance;
							sample.m_nFrequency = ComputeDopplerEffectedFrequency(sample.m_nFrequency, position1, position2, sample.m_fSpeedMultiplier);
							if (sample.m_nFrequency != m_asActiveSamples[j].m_nFrequency) {
								int32 freq;
								if (sample.m_nFrequency <= m_asActiveSamples[j].m_nFrequency) {
#ifdef FIX_BUGS
									freq = Max((int32)sample.m_nFrequency, (int32)m_asActiveSamples[j].m_nFrequency - 6000);
#else
									freq = Max((int32)sample.m_nFrequency, int32(m_asActiveSamples[j].m_nFrequency - 6000));
#endif
								} else {
									freq = Min(sample.m_nFrequency, m_asActiveSamples[j].m_nFrequency + 6000);
								}
								m_asActiveSamples[j].m_nFrequency = freq;
								SampleManager.SetChannelFrequency(j, freq);
							}

							if (sample.m_nEmittingVolume != m_asActiveSamples[j].m_nEmittingVolume) {
								if (sample.m_nEmittingVolume <= m_asActiveSamples[j].m_nEmittingVolume) {
									vol = Max(m_asActiveSamples[j].m_nEmittingVolume - 10, sample.m_nEmittingVolume);
								} else {
									vol = Min(m_asActiveSamples[j].m_nEmittingVolume + 10, sample.m_nEmittingVolume);
								}

								uint8 emittingVol;
								if (field_4) {
									emittingVol = 2 * Min(63, vol);
								} else {
									emittingVol = vol;
								}
								SampleManager.SetChannelEmittingVolume(j, emittingVol);
								m_asActiveSamples[j].m_nEmittingVolume = vol;
							}
							TranslateEntity(&sample.m_vecPos, &position);
							SampleManager.SetChannel3DPosition(j, position.x, position.y, position.z);
							SampleManager.SetChannel3DDistances(j, sample.m_fSoundIntensity, 0.25f * sample.m_fSoundIntensity);
						}
						SampleManager.SetChannelReverbFlag(j, sample.m_bReverbFlag);
						break;
					}
					sample.m_bIsProcessed = false;
					m_asActiveSamples[j].m_bIsProcessed = false;
				}
			}
		}
	}
	for (int32 i = 0; i < m_nActiveSamples; i++) {
		if (m_asActiveSamples[i].m_nSampleIndex != NO_SAMPLE && !m_asActiveSamples[i].m_bIsProcessed) {
			SampleManager.StopChannel(i);
			m_asActiveSamples[i].m_nSampleIndex = NO_SAMPLE;
			m_asActiveSamples[i].m_nEntityIndex = AEHANDLE_NONE;
		}
	}
	for (uint8 i = 0; i < m_SampleRequestQueuesStatus[m_nActiveSampleQueue]; ++i) {
		tSound &sample = m_asSamples[m_nActiveSampleQueue][m_abSampleQueueIndexTable[m_nActiveSampleQueue][i]];
		if (!sample.m_bIsProcessed && !sample.m_bLoopEnded && m_asAudioEntities[sample.m_nEntityIndex].m_bIsUsed && sample.m_nSampleIndex < NO_SAMPLE) {
			if (sample.m_nCounter > 255 && sample.m_nLoopCount && sample.m_nLoopsRemaining) {
				--sample.m_nLoopsRemaining;
				sample.m_nReleasingVolumeDivider = 1;
			} else {
				for (uint8 j = 0; j < m_nActiveSamples; ++j) {
					if (!m_asActiveSamples[j].m_bIsProcessed) {
						if (sample.m_nLoopCount) {
							v28 = sample.m_nFrequency / m_nTimeSpent;
							v29 = sample.m_nLoopCount * SampleManager.GetSampleLength(sample.m_nSampleIndex);
							if (v28 == 0)
								continue;
							sample.m_nReleasingVolumeDivider = v29 / v28 + 1;
						}
						memcpy(&m_asActiveSamples[j], &sample, sizeof(tSound));
						if (!m_asActiveSamples[j].m_bIs2D)
							TranslateEntity(&m_asActiveSamples[j].m_vecPos, &position);
						if (field_4) {
							emittingVol = 2 * Min(63, m_asActiveSamples[j].m_nEmittingVolume);
						} else {
							emittingVol = m_asActiveSamples[j].m_nEmittingVolume;
						}
						if (SampleManager.InitialiseChannel(j, m_asActiveSamples[j].m_nSampleIndex, m_asActiveSamples[j].m_nBankIndex)) {
							SampleManager.SetChannelFrequency(j, m_asActiveSamples[j].m_nFrequency);
							SampleManager.SetChannelEmittingVolume(j, emittingVol);
							SampleManager.SetChannelLoopPoints(j, m_asActiveSamples[j].m_nLoopStart, m_asActiveSamples[j].m_nLoopEnd);
							SampleManager.SetChannelLoopCount(j, m_asActiveSamples[j].m_nLoopCount);
							SampleManager.SetChannelReverbFlag(j, m_asActiveSamples[j].m_bReverbFlag);
							if (m_asActiveSamples[j].m_bIs2D) {
								uint8 offset = m_asActiveSamples[j].m_nOffset;
								if (offset == 63) {
									x = 0.f;
								} else if (offset >= 63) {
									x = (offset - 63) * 1000.f / 63;
								} else {
									x = -(63 - offset) * 1000.f / 63;
								}
								usedX = x;
								usedY = 0.f;
								usedZ = 0.f;
								m_asActiveSamples[j].m_fSoundIntensity = 100000.0f;
							} else {
								usedX = position.x;
								usedY = position.y;
								usedZ = position.z;
							}
							SampleManager.SetChannel3DPosition(j, usedX, usedY, usedZ);
							SampleManager.SetChannel3DDistances(j, m_asActiveSamples[j].m_fSoundIntensity, 0.25f * m_asActiveSamples[j].m_fSoundIntensity);
							SampleManager.StartChannel(j);
						}
						m_asActiveSamples[j].m_bIsProcessed = true;
						sample.m_bIsProcessed = true;
						sample.m_nVolumeChange = -1;
						break;
					}
				}
			}
		}
	}
}

void
cAudioManager::ClearRequestedQueue()
{
	for (int32 i = 0; i < m_nActiveSamples; i++) {
		m_abSampleQueueIndexTable[m_nActiveSampleQueue][i] = m_nActiveSamples;
	}
	m_SampleRequestQueuesStatus[m_nActiveSampleQueue] = 0;
}

void
cAudioManager::ClearActiveSamples()
{
	for (int32 i = 0; i < m_nActiveSamples; i++) {
		m_asActiveSamples[i].m_nEntityIndex = AEHANDLE_NONE;
		m_asActiveSamples[i].m_nCounter = 0;
		m_asActiveSamples[i].m_nSampleIndex = NO_SAMPLE;
		m_asActiveSamples[i].m_nBankIndex = INVALID_SFX_BANK;
		m_asActiveSamples[i].m_bIs2D = false;
		m_asActiveSamples[i].m_nReleasingVolumeModificator = 5;
		m_asActiveSamples[i].m_nFrequency = 0;
		m_asActiveSamples[i].m_nVolume = 0;
		m_asActiveSamples[i].m_nEmittingVolume = 0;
		m_asActiveSamples[i].m_fDistance = 0.0f;
		m_asActiveSamples[i].m_bIsProcessed = false;
		m_asActiveSamples[i].m_bLoopEnded = false;
		m_asActiveSamples[i].m_nLoopCount = 1;
		m_asActiveSamples[i].m_nLoopStart = 0;
		m_asActiveSamples[i].m_nLoopEnd = -1;
		m_asActiveSamples[i].m_fSpeedMultiplier = 0.0f;
		m_asActiveSamples[i].m_fSoundIntensity = 200.0f;
		m_asActiveSamples[i].m_nOffset = 63;
		m_asActiveSamples[i].m_bReleasingSoundFlag = false;
		m_asActiveSamples[i].m_nCalculatedVolume = 0;
		m_asActiveSamples[i].m_nReleasingVolumeDivider = 0;
		m_asActiveSamples[i].m_nVolumeChange = -1;
		m_asActiveSamples[i].m_vecPos = CVector(0.0f, 0.0f, 0.0f);
		m_asActiveSamples[i].m_bReverbFlag = false;
		m_asActiveSamples[i].m_nLoopsRemaining = 0;
		m_asActiveSamples[i].m_bRequireReflection = false;
	}
}

void
cAudioManager::GenerateIntegerRandomNumberTable()
{
	for (int32 i = 0; i < ARRAY_SIZE(m_anRandomTable); i++) {
		m_anRandomTable[i] = myrand();
	}
}

#ifdef GTA_PC
void
cAudioManager::AdjustSamplesVolume()
{
	for (int i = 0; i < m_SampleRequestQueuesStatus[m_nActiveSampleQueue]; i++) {
		tSound *pSample = &m_asSamples[m_nActiveSampleQueue][m_abSampleQueueIndexTable[m_nActiveSampleQueue][i] + 1];

		if (!pSample->m_bIs2D)
			pSample->m_nEmittingVolume = ComputeEmittingVolume(pSample->m_nEmittingVolume, pSample->m_fSoundIntensity, pSample->m_fDistance);
	}
}

uint8
cAudioManager::ComputeEmittingVolume(uint8 emittingVolume, float intensity, float dist)
{
	float quatIntensity = intensity / 4.0f;
	float diffIntensity = intensity - quatIntensity;
	if (dist > diffIntensity)
		return (quatIntensity - (dist - diffIntensity)) * (float)emittingVolume / quatIntensity;
	return emittingVolume;
}
#endif