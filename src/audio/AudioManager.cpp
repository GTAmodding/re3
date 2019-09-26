#include "common.h"
#include "patcher.h"

#include "AudioManager.h"

#include "Automobile.h"
#include "Boat.h"
#include "Bridge.h"
#include "CProjectileInfo.h"
#include "Camera.h"
#include "DMAudio.h"
#include "Entity.h"
#include "Explosion.h"
#include "Garages.h"
#include "HandlingMgr.h"
#include "Heli.h"
#include "ModelIndices.h"
#include "MusicManager.h"
#include "Pad.h"
#include "Ped.h"
#include "Physical.h"
#include "Placeable.h"
#include "Plane.h"
#include "PlayerPed.h"
#include "Pools.h"
#include "Replay.h"
#include "Stats.h"
#include "SurfaceTable.h"
#include "Train.h"
#include "Transmission.h"
#include "Vehicle.h"
#include "WaterCannon.h"
#include "Weather.h"
#include "Zones.h"
#include "World.h"
#include "ZoneCull.h"
#include "sampman.h"

cAudioManager &AudioManager = *(cAudioManager *)0x880FC0;
uint32 *audioLogicTimers = (uint32 *)0x6508A0;
uint8 &jumboVolOffset = *(uint8 *)0x6508ED;
uint8 &gJumboVolOffsetPercentage = *(uint8 *)0x6508ED;
char &g_nMissionAudioPlayingStatus = *(char *)0x60ED88;
int32 *BankStartOffset = (int32 *)0x6FAB70; //[2]
int32 &g_nMissionAudioSfx = *(int32 *)0x60ED84;
bool &bPlayerJustEnteredCar = *(bool *)0x6508C4;
bool &g_bMissionAudioLoadFailed = *(bool *)0x95CD8E;
uint32 *gMinTimeToNextReport = (uint32 *)0x8E2828;
uint8 &gSpecialSuspectLastSeenReport = *(uint8 *)0x95CD4D;
//uint32 *gOneShotCol = (uint32 *)0x604BD0;
bool *somethingWithHorns = (bool *)0x606AB8;

constexpr int totalAudioEntitiesSlots = 200;
constexpr int maxVolume = 127;
constexpr int policeChannel = 28;
constexpr uint8 panTable[64]{
    0,  3,  8,  12, 16, 19, 22, 24, 26, 28, 30, 31, 33, 34, 36, 37, 39, 40, 41, 42, 44, 45,
    46, 47, 48, 49, 49, 50, 51, 52, 53, 53, 54, 55, 55, 56, 56, 57, 57, 58, 58, 58, 59, 59,
    59, 60, 60, 61, 61, 61, 61, 62, 62, 62, 62, 62, 63, 63, 63, 63, 63, 63, 63, 63};

// TODO: where is this used? Is this the right file?
enum eVehicleModel {
	LANDSTAL,
	IDAHO,
	STINGER,
	LINERUN,
	PEREN,
	SENTINEL,
	PATRIOT,
	FIRETRUK,
	TRASH,
	STRETCH,
	MANANA,
	INFERNUS,
	BLISTA,
	PONY,
	MULE,
	CHEETAH,
	AMBULAN,
	FBICAR,
	MOONBEAM,
	ESPERANT,
	TAXI,
	KURUMA,
	BOBCAT,
	MRWHOOP,
	BFINJECT,
	CORPSE,
	POLICE,
	ENFORCER,
	SECURICA,
	BANSHEE,
	PREDATOR,
	BUS,
	RHINO,
	BARRACKS,
	TRAIN,
	CHOPPER,
	DODO,
	COACH,
	CABBIE,
	STALLION,
	RUMPO,
	RCBANDIT,
	BELLYUP,
	MRWONGS,
	MAFIA,
	YARDIE,
	YAKUZA,
	DIABLOS,
	COLUMB,
	HOODS,
	AIRTRAIN,
	DEADDODO,
	SPEEDER,
	REEFER,
	PANLANT,
	FLATBED,
	YANKEE,
	ESCAPE,
	BORGNINE,
	TOYZ,
	GHOST,
	CAR151,
	CAR152,
	CAR153,
	CAR154,
	CAR155,
	CAR156,
	CAR157,
	CAR158,
	CAR159,
};

void
cPedComments::Add(tPedComment *com)
{
	uint8 index;

	if(nrOfCommentsInBank[activeBank] >= 20u) {
		index = indexMap[activeBank][19];
		if(m_asPedComments[activeBank][index].m_bVolume > com->m_bVolume) return;
	} else {
		index = nrOfCommentsInBank[activeBank]++;
	}

	m_asPedComments[activeBank][index].m_nSampleIndex = com->m_nSampleIndex;
	m_asPedComments[activeBank][index].m_entityIndex = com->m_entityIndex;
	m_asPedComments[activeBank][index].m_vecPos = com->m_vecPos;
	m_asPedComments[activeBank][index].m_fDistance = com->m_fDistance;
	m_asPedComments[activeBank][index].m_bVolume = com->m_bVolume;

	uint32 i = 0;
	if(index != 0) {
		for(i = 0; i < index; i++) {
			if(m_asPedComments[activeBank][indexMap[activeBank][i]].m_bVolume <
			   m_asPedComments[activeBank][index].m_bVolume) {
				break;
			}
		}

		if(i < index)
			memmove(&indexMap[activeBank][i + 1], &indexMap[activeBank][i], 19 - i);
	}

	indexMap[activeBank][i] = index;
}

void
cPedComments::Process()
{
	int sampleIndex;
	uint8 actualUsedBank;
	tPedComment *comment;

	if(!AudioManager.m_bUserPause) {
		if(nrOfCommentsInBank[activeBank]) {
			sampleIndex =
			    m_asPedComments[activeBank][indexMap[activeBank][0]].m_nSampleIndex;
			if(!SampleManager.IsPedCommentLoaded(sampleIndex))
				SampleManager.LoadPedComment(sampleIndex);

			AudioManager.m_sQueueSample.m_nEntityIndex =
			    m_asPedComments[activeBank][indexMap[activeBank][0]].m_entityIndex;
			AudioManager.m_sQueueSample.m_counter = 0;
			AudioManager.m_sQueueSample.m_nSampleIndex = sampleIndex;
			AudioManager.m_sQueueSample.m_bBankIndex = 1;
			AudioManager.m_sQueueSample.field_16 = 3;
			AudioManager.m_sQueueSample.m_bVolume =
			    m_asPedComments[activeBank][indexMap[activeBank][0]].m_bVolume;
			AudioManager.m_sQueueSample.m_fDistance =
			    m_asPedComments[activeBank][indexMap[activeBank][0]].m_fDistance;
			AudioManager.m_sQueueSample.m_nLoopCount = 1;
			AudioManager.m_sQueueSample.m_nLoopStart = 0;
			AudioManager.m_sQueueSample.m_nLoopEnd = -1;
			AudioManager.m_sQueueSample.m_bEmittingVolume = maxVolume;
			AudioManager.m_sQueueSample.field_48 = 3.0f;
			switch(sampleIndex) {
			case AUDIO_SAMPLE_POLICE_HELI_FOUND_PLAYER_1:
			case AUDIO_SAMPLE_POLICE_HELI_FOUND_PLAYER_2:
			case AUDIO_SAMPLE_POLICE_HELI_FOUND_PLAYER_3:
				AudioManager.m_sQueueSample.m_fSoundIntensity = 400.0f;
				break;
			default: AudioManager.m_sQueueSample.m_fSoundIntensity = 50.0f; break;
			}
			AudioManager.m_sQueueSample.field_56 = 1;
			AudioManager.m_sQueueSample.m_vecPos =
			    m_asPedComments[activeBank][indexMap[activeBank][0]].m_vecPos;

			if((sampleIndex - AUDIO_SAMPLE_AMMUNATION_WELCOME_1) > 1 &&
			   sampleIndex != AUDIO_SAMPLE_AMMUNATION_WELCOME_3) {
				AudioManager.m_sQueueSample.m_bReverbFlag = 1;
				AudioManager.m_sQueueSample.m_bRequireReflection = 1;
			} else {
				AudioManager.m_sQueueSample.m_bReverbFlag = 0;
				AudioManager.m_sQueueSample.m_bRequireReflection = 0;
			}

			AudioManager.m_sQueueSample.m_bIsDistant = 0;
			AudioManager.m_sQueueSample.m_nFrequency =
			    SampleManager.GetSampleBaseFrequency(
			        AudioManager.m_sQueueSample.m_nSampleIndex) +
			    AudioManager.RandomDisplacement(750u);
			if(CTimer::GetIsSlowMotionActive())
				AudioManager.m_sQueueSample.m_nFrequency /= 2;
			m_asPedComments[activeBank][indexMap[activeBank][0]].field_25 = -1;
			AudioManager.AddSampleToRequestedQueue();
		}

		// Switch bank
		if(activeBank) {
			actualUsedBank = 1;
			activeBank = 0;
		} else {
			actualUsedBank = 0;
			activeBank = 1;
		}
		comment = m_asPedComments[actualUsedBank];
		for(uint32 i = 0; i < nrOfCommentsInBank[actualUsedBank]; i++) {
			if(m_asPedComments[actualUsedBank][indexMap[actualUsedBank][i]].field_25 >
			   0) {
				--m_asPedComments[actualUsedBank][indexMap[actualUsedBank][i]]
				      .field_25;
				Add(&comment[indexMap[actualUsedBank][i]]);
			}
		}

		for(uint32 i = 0; i < 20; i++) { indexMap[actualUsedBank][i] = 20; }
		nrOfCommentsInBank[actualUsedBank] = 0;
	}
}

void
cAudioCollisionManager::AddCollisionToRequestedQueue()
{
	int32 collisionsIndex;
	int32 i;

	if(m_bCollisionsInQueue >= 10u) {
		collisionsIndex = m_bIndicesTable[9];
		if(m_sQueue.m_fDistance >= m_asCollisions1[collisionsIndex].m_fDistance) return;
	} else {
		collisionsIndex = m_bCollisionsInQueue++;
	}

	m_asCollisions1[collisionsIndex] = m_sQueue;

	i = 0;
	if(collisionsIndex) {
		while(m_asCollisions1[m_bIndicesTable[i]].m_fDistance <=
		      m_asCollisions1[collisionsIndex].m_fDistance) {
			if(++i >= collisionsIndex) {
				m_bIndicesTable[i] = collisionsIndex;
				return;
			}
		}
		memmove(&m_bIndicesTable[i + 1], &m_bIndicesTable[i], 9 - i);
	}
	m_bIndicesTable[i] = collisionsIndex;
}

void
cAudioScriptObject::Reset()
{
	AudioId = 125;
	Posn = {0.0f, 0.0f, 0.0f};
	AudioEntity = -5;
}

void *
cAudioScriptObject::operator new(size_t sz)
{
	return CPools::GetAudioScriptObjectPool()->New();
}
void *
cAudioScriptObject::operator new(size_t sz, int handle)
{
	return CPools::GetAudioScriptObjectPool()->New(handle);
}
void
cAudioScriptObject::operator delete(void *p, size_t sz)
{
	CPools::GetAudioScriptObjectPool()->Delete((cAudioScriptObject *)p);
}
void
cAudioScriptObject::operator delete(void *p, int handle)
{
	CPools::GetAudioScriptObjectPool()->Delete((cAudioScriptObject *)p);
}

void
cAudioManager::AddDetailsToRequestedOrderList(uint8 sample)
{
	uint32 i = 0;
	if(sample != 0) {
		for(; i < sample; i++) {
			if(m_asSamples[m_bActiveSampleQueue]
			              [m_abSampleQueueIndexTable[m_bActiveSampleQueue][i]]
			                  .calculatedVolume >
			   m_asSamples[m_bActiveSampleQueue][sample].calculatedVolume)
				break;
		}
		if(i < sample) {
			memmove(&m_abSampleQueueIndexTable[m_bActiveSampleQueue][i + 1],
			        &m_abSampleQueueIndexTable[m_bActiveSampleQueue][i],
			        m_bActiveSamples - i - 1);
		}
	}
	m_abSampleQueueIndexTable[m_bActiveSampleQueue][i] = sample;
}

void
cAudioManager::AddPlayerCarSample(uint8 emittingVolume, int32 freq, uint32 sample, uint8 unk1,
                                  uint8 counter, bool notLooping)
{
	m_sQueueSample.m_bVolume = ComputeVolume(emittingVolume, 50.f, m_sQueueSample.m_fDistance);
	if(m_sQueueSample.m_bVolume) {
		m_sQueueSample.m_counter = counter;
		m_sQueueSample.m_nSampleIndex = sample;
		m_sQueueSample.m_bBankIndex = 0;
		m_sQueueSample.m_bIsDistant = 0;
		m_sQueueSample.field_16 = 0;
		m_sQueueSample.m_nFrequency = freq;
		if(notLooping) {
			m_sQueueSample.m_nLoopCount = 0;
			m_sQueueSample.field_76 = 8;
		} else {
			m_sQueueSample.m_nLoopCount = 1;
		}
		m_sQueueSample.m_bEmittingVolume = emittingVolume;
		m_sQueueSample.m_nLoopStart =
		    SampleManager.GetSampleLoopStartOffset(m_sQueueSample.m_nSampleIndex);
		m_sQueueSample.m_nLoopEnd =
		    SampleManager.GetSampleLoopEndOffset(m_sQueueSample.m_nSampleIndex);
		m_sQueueSample.field_48 = 6.0f;
		m_sQueueSample.m_fSoundIntensity = 50.0f;
		m_sQueueSample.field_56 = 0;
		m_sQueueSample.m_bReverbFlag = 1;
		m_sQueueSample.m_bRequireReflection = 0;
		AddSampleToRequestedQueue();
	}
}

void
cAudioManager::AddReflectionsToRequestedQueue()
{
	float reflectionDistance;
	int32 noise;
	uint8 emittingVolume = (m_sQueueSample.m_bVolume >> 1) + (m_sQueueSample.m_bVolume >> 3);

	for(uint32 i = 0; i < 5u; i++) {
		reflectionDistance = m_afReflectionsDistances[i];
		if(reflectionDistance > 0.0f && reflectionDistance < 100.f &&
		   reflectionDistance < m_sQueueSample.m_fSoundIntensity) {
			m_sQueueSample.m_bLoopsRemaining = (reflectionDistance * 500.f / 1029.f);
			if(m_sQueueSample.m_bLoopsRemaining > 5u) {
				m_sQueueSample.m_fDistance = m_afReflectionsDistances[i];
				m_sQueueSample.m_bEmittingVolume = emittingVolume;
				m_sQueueSample.m_bVolume =
				    ComputeVolume(emittingVolume, m_sQueueSample.m_fSoundIntensity,
				                  m_sQueueSample.m_fDistance);
				if(m_sQueueSample.m_bVolume > emittingVolume >> 4) {
					m_sQueueSample.m_counter += ((i + 1) << 8);
					if(m_sQueueSample.m_nLoopCount) {
						noise = RandomDisplacement(
						    m_sQueueSample.m_nFrequency >> 5);
						if(noise <= 0)
							m_sQueueSample.m_nFrequency += noise;
						else
							m_sQueueSample.m_nFrequency -= noise;
					}
					m_sQueueSample.field_16 += 20;
					m_sQueueSample.m_vecPos = m_avecReflectionsPos[i];
					AddSampleToRequestedQueue();
				}
			}
		}
	}
}

void
cAudioManager::AddReleasingSounds()
{
	bool toProcess[44];

	for(int32 i = 0; i < m_bSampleRequestQueuesStatus[m_bActiveSampleQueue]; i++) {
		tActiveSample &sample =
		    m_asSamples[!m_bActiveSampleQueue]
		               [m_abSampleQueueIndexTable[!m_bActiveSampleQueue][i]];
		if(!m_asSamples[!m_bActiveSampleQueue]
		               [m_abSampleQueueIndexTable[!m_bActiveSampleQueue][i]]
		                   .m_bLoopEnded) {
			toProcess[i] = 0;
			for(int32 j = 0; j < m_bSampleRequestQueuesStatus[m_bActiveSampleQueue];
			    j++) {
				if(sample.m_nEntityIndex ==
				       m_asSamples[m_bActiveSampleQueue]
				                  [m_abSampleQueueIndexTable[m_bActiveSampleQueue]
				                                            [j]]
				                      .m_nEntityIndex &&
				   sample.m_counter ==
				       m_asSamples[m_bActiveSampleQueue]
				                  [m_abSampleQueueIndexTable[m_bActiveSampleQueue]
				                                            [j]]
				                      .m_counter) {
					toProcess[i] = 1;
					break;
				}
			}
			if(!toProcess[i]) {
				if(sample.m_counter <= 255u || !sample.m_bLoopsRemaining) {
					if(!sample.field_76) continue;
					if(!sample.m_nLoopCount) {
						if(sample.field_88 == -1) {
							sample.field_88 =
							    sample.m_bVolume / sample.field_76;
							if(sample.field_88 <= 0)
								sample.field_88 = 1;
						}
						if(sample.m_bVolume <= sample.field_88) {
							sample.field_76 = 0;
							continue;
						}
						sample.m_bVolume -= sample.field_88;
					}
					--sample.field_76;
					if(field_2) {
						if(sample.field_16 < 20u) ++sample.field_16;
					}
					sample.field_56 = 0;
				}
				memcpy(&m_sQueueSample, &sample, 92);
				AddSampleToRequestedQueue();
			}
		}
	}
}

void
cAudioManager::AddSampleToRequestedQueue()
{
	int32 calculatedVolume;
	uint8 sampleIndex;
	bool bReflections;

	if(m_sQueueSample.m_nSampleIndex < TOTAL_AUDIO_SAMPLES) {
		calculatedVolume = m_sQueueSample.field_16 * (maxVolume - m_sQueueSample.m_bVolume);
		sampleIndex = m_bSampleRequestQueuesStatus[m_bActiveSampleQueue];
		if(sampleIndex >= m_bActiveSamples) {
			sampleIndex =
			    m_abSampleQueueIndexTable[m_bActiveSampleQueue][m_bActiveSamples - 1];
			if(m_asSamples[m_bActiveSampleQueue][sampleIndex].calculatedVolume <=
			   calculatedVolume)
				return;
		} else {
			++m_bSampleRequestQueuesStatus[m_bActiveSampleQueue];
		}
		m_sQueueSample.calculatedVolume = calculatedVolume;
		m_sQueueSample.m_bLoopEnded = 0;
		if(m_sQueueSample.m_bIsDistant) {
			m_sQueueSample.m_bRequireReflection = 0;
			m_sQueueSample.m_bLoopsRemaining = 0;
		}
		if(m_bDynamicAcousticModelingStatus && m_sQueueSample.m_nLoopCount) {
			bReflections = m_sQueueSample.m_bRequireReflection;
		} else {
			bReflections = false;
			m_sQueueSample.m_bLoopsRemaining = 0;
		}
		m_sQueueSample.m_bRequireReflection = 0;

		if(!m_bDynamicAcousticModelingStatus) m_sQueueSample.m_bReverbFlag = 0;

		m_asSamples[m_bActiveSampleQueue][sampleIndex] = m_sQueueSample;

		AddDetailsToRequestedOrderList(sampleIndex);
		if(bReflections) AddReflectionsToRequestedQueue();
	}
}

void
cAudioManager::AgeCrimes()
{
	for(uint8 i = 0; i < 10; i++) {
		if(crimes[i].type) {
			if(++crimes[i].timer > 1500u) crimes[i].type = 0;
		}
	}
}

void
cAudioManager::CalculateDistance(bool *ptr, float dist)
{
	if(*ptr == false) {
		m_sQueueSample.m_fDistance = Sqrt(dist);
		*ptr = true;
	}
}

bool
cAudioManager::CheckForAnAudioFileOnCD() const
{
	return SampleManager.CheckForAnAudioFileOnCD();
}

void
cAudioManager::ClearActiveSamples()
{
	for(int32 i = 0; i < m_bActiveSamples; i++) {
		m_asActiveSamples[i].m_nEntityIndex = -5;
		m_asActiveSamples[i].m_counter = 0;
		m_asActiveSamples[i].m_nSampleIndex = NO_SAMPLE;
		m_asActiveSamples[i].m_bBankIndex = 3;
		m_asActiveSamples[i].m_bIsDistant = 0;
		m_asActiveSamples[i].field_16 = 5;
		m_asActiveSamples[i].m_nFrequency = 0;
		m_asActiveSamples[i].m_bVolume = 0;
		m_asActiveSamples[i].m_bEmittingVolume = 0;
		m_asActiveSamples[i].m_fDistance = 0.0f;
		m_asActiveSamples[i].m_bIsProcessed = 0;
		m_asActiveSamples[i].m_bLoopEnded = 0;
		m_asActiveSamples[i].m_nLoopCount = 1;
		m_asActiveSamples[i].m_nLoopStart = 0;
		m_asActiveSamples[i].m_nLoopEnd = -1;
		m_asActiveSamples[i].field_48 = 0.0f;
		m_asActiveSamples[i].m_fSoundIntensity = 200.0f;
		m_asActiveSamples[i].m_bOffset = 63;
		m_asActiveSamples[i].field_56 = 0;
		m_asActiveSamples[i].calculatedVolume = 0;
		m_asActiveSamples[i].field_76 = 0;
		m_asActiveSamples[i].field_88 = -1;
		m_asActiveSamples[i].m_vecPos = {0.0f, 0.0f, 0.0f};
		m_asActiveSamples[i].m_bReverbFlag = 0;
		m_asActiveSamples[i].m_bLoopsRemaining = 0;
		m_asActiveSamples[i].m_bRequireReflection = 0;
	}
}

void
cAudioManager::ClearMissionAudio()
{
	if(m_bIsInitialised) {
		m_sMissionAudio.m_nSampleIndex = NO_SAMPLE;
		m_sMissionAudio.m_bLoadingStatus = 0;
		m_sMissionAudio.m_bPlayStatus = 0;
		m_sMissionAudio.field_22 = 0;
		m_sMissionAudio.m_bIsPlayed = false;
		m_sMissionAudio.field_12 = 1;
		m_sMissionAudio.field_24 = 0;
	}
}

void
cAudioManager::ClearRequestedQueue()
{
	for(int32 i = 0; i < m_bActiveSamples; i++) {
		m_abSampleQueueIndexTable[m_bActiveSampleQueue][i] = m_bActiveSamples;
	}
	m_bSampleRequestQueuesStatus[m_bActiveSampleQueue] = 0;
}

int32
cAudioManager::ComputeDopplerEffectedFrequency(uint32 oldFreq, float position1, float position2,
                                               float speedMultiplier) const
{
	uint32 newFreq = oldFreq;
	if(!TheCamera.Get_Just_Switched_Status() && speedMultiplier != 0.0f) {
		float dist = position2 - position1;
		if(dist != 0.0f) {
			float speedOfSource = (dist / field_19195) * speedMultiplier;
			if(speedOfSound > Abs(speedOfSource)) {
				if(speedOfSource < 0.0f) {
					speedOfSource = max(speedOfSource, -1.5f);
				} else {
					speedOfSource = min(speedOfSource, 1.5f);
				}
				newFreq = (oldFreq * speedOfSound) / (speedOfSource + speedOfSound);
			}
		}
	}
	return newFreq;
}

int32
cAudioManager::ComputePan(float dist, CVector *vec)
{
	int32 index = min(63, Abs(vec->x / (dist / 64.f)));

	if(vec->x > 0.f) return max(20, 63 - panTable[index]);
	return min(107, panTable[index] + 63);
}

uint32
cAudioManager::ComputeVolume(int emittingVolume, float soundIntensity, float distance) const
{
	float newSoundIntensity;
	if(soundIntensity <= 0.0f) return 0;
	if((soundIntensity * 0.2f) <= distance) {
		newSoundIntensity = soundIntensity * 0.2f;
		emittingVolume =
		    sq((soundIntensity - distance) / (soundIntensity - newSoundIntensity)) *
		    emittingVolume;
	}
	return emittingVolume;
}

int32
cAudioManager::CreateEntity(int32 type, CPhysical *entity)
{
	if(!m_bIsInitialised) return -4;
	if(!entity) return -2;
	if(type >= TOTAL_AUDIO_TYPES) return -1;
	for(uint32 i = 0; i < 200; i++) {
		if(!m_asAudioEntities[i].m_bIsUsed) {
			m_asAudioEntities[i].m_bIsUsed = true;
			m_asAudioEntities[i].m_bStatus = 0;
			m_asAudioEntities[i].m_nType = (eAudioType)type;
			m_asAudioEntities[i].m_pEntity = (void *)entity;
			m_asAudioEntities[i].m_awAudioEvent[0] = SOUND_TOTAL_PED_SOUNDS;
			m_asAudioEntities[i].m_awAudioEvent[1] = SOUND_TOTAL_PED_SOUNDS;
			m_asAudioEntities[i].m_awAudioEvent[2] = SOUND_TOTAL_PED_SOUNDS;
			m_asAudioEntities[i].m_awAudioEvent[3] = SOUND_TOTAL_PED_SOUNDS;
			m_asAudioEntities[i].m_Loops = 0;
			m_anAudioEntityIndices[m_nAudioEntitiesTotal++] = i;
			return i;
		}
	}
	return -3;
}

void
cAudioManager::DestroyAllGameCreatedEntities()
{
	cAudioScriptObject *entity;

	if(m_bIsInitialised) {
		for(uint32 i = 0; i < 200; i++) {
			if(m_asAudioEntities[i].m_bIsUsed) {
				switch(m_asAudioEntities[i].m_nType) {
				case AUDIOTYPE_PHYSICAL:
				case AUDIOTYPE_EXPLOSION:
				case AUDIOTYPE_WEATHER:
				case AUDIOTYPE_CRANE:
				case AUDIOTYPE_GARAGE:
				case AUDIOTYPE_FIREHYDRANT: DestroyEntity(i); break;
				case AUDIOTYPE_SCRIPTOBJECT:
					entity =
					    (cAudioScriptObject *)m_asAudioEntities[i].m_pEntity;
					if(entity) { 
						delete entity; 
						m_asAudioEntities[i].m_pEntity = nil;
					}
					DestroyEntity(i);
					break;
				default: break;
				}
			}
		}
		m_nScriptObjectEntityTotal = 0;
	}
}

void
cAudioManager::DestroyEntity(int32 id)
{
	if(m_bIsInitialised && id >= 0 && id < totalAudioEntitiesSlots &&
	   m_asAudioEntities[id].m_bIsUsed) {
		m_asAudioEntities[id].m_bIsUsed = 0;
		for(int32 i = 0; i < m_nAudioEntitiesTotal; ++i) {
			if(id == m_anAudioEntityIndices[i]) {
				if(i < totalAudioEntitiesSlots - 1)
					memmove(&m_anAudioEntityIndices[i],
					        &m_anAudioEntityIndices[i + 1],
					        4 * (m_nAudioEntitiesTotal - (i + 1)));
				m_anAudioEntityIndices[--m_nAudioEntitiesTotal] =
				    totalAudioEntitiesSlots;
				return;
			}
		}
	}
}

void
cAudioManager::DoJumboVolOffset() const
{
	if(!(m_nTimeOfRecentCrime % (m_anRandomTable[0] % 6u + 3)))
		jumboVolOffset = m_anRandomTable[1] % 60u;
}

void
cAudioManager::DoPoliceRadioCrackle()
{
	m_sQueueSample.m_nEntityIndex = m_nPoliceChannelEntity;
	m_sQueueSample.m_counter = 0;
	m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_POLICE_SCANNER_CRACKLE;
	m_sQueueSample.m_bBankIndex = 0;
	m_sQueueSample.m_bIsDistant = 1;
	m_sQueueSample.field_16 = 10;
	m_sQueueSample.m_nFrequency =
	    SampleManager.GetSampleBaseFrequency(AUDIO_SAMPLE_POLICE_SCANNER_CRACKLE);
	m_sQueueSample.m_bVolume = m_anRandomTable[2] % 20u + 15;
	m_sQueueSample.m_nLoopCount = 0;
	m_sQueueSample.m_bEmittingVolume = m_sQueueSample.m_bVolume;
	m_sQueueSample.m_nLoopStart = SampleManager.GetSampleLoopStartOffset(188);
	m_sQueueSample.m_nLoopEnd = SampleManager.GetSampleLoopEndOffset(188);
	m_sQueueSample.field_56 = 0;
	m_sQueueSample.m_bReverbFlag = 0;
	m_sQueueSample.m_bOffset = 63;
	m_sQueueSample.field_76 = 3;
	m_sQueueSample.m_bRequireReflection = 0;
	AddSampleToRequestedQueue();
}

uint32
cAudioManager::GetPlayerTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_DAMAGE:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_DAMAGE_REACTION_1, 11u);
		break;

	case SOUND_PED_HIT: GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_HIT_REACTION_1, 10u); break;

	case SOUND_PED_LAND: GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_FALL_REACTION_1, 6u); break;

	default: sfx = NO_SAMPLE; break;
	}
	return sfx;
}

uint32
cAudioManager::GetCopTalkSfx(int16 sound)
{
	uint32 sfx;
	PedState pedState;
	static uint32 lastSfx = NO_SAMPLE;

	if(sound == SOUND_PED_ARREST_COP) {
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_POLICE_COP_1_ARREST_1, 6u);
	} else {
		if(sound != SOUND_PED_PURSUIT_COP) { return GetGenericMaleTalkSfx(sound); }

		pedState = FindPlayerPed()->m_nPedState;
		if(pedState == PED_ARRESTED || pedState == PED_DEAD || pedState == PED_DIE)
			return NO_SAMPLE;
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_POLICE_COP_1_PURSUIT_1, 7u);
	}

	return 13 * (m_sQueueSample.m_nEntityIndex % 5) + sfx;
}

uint32
cAudioManager::GetSwatTalkSfx(int16 sound)
{
	uint32 sfx;
	PedState pedState;
	static uint32 lastSfx = NO_SAMPLE;

	if(sound == SOUND_PED_ARREST_SWAT) {
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_POLICE_SWAT_1_PURSUIT_ARREST_1, 6u);
	} else {
		if(sound != SOUND_PED_PURSUIT_SWAT) { return GetGenericMaleTalkSfx(sound); }

		pedState = FindPlayerPed()->m_nPedState;
		if(pedState == PED_ARRESTED || pedState == PED_DEAD || pedState == PED_DIE)
			return NO_SAMPLE;
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_POLICE_SWAT_1_PURSUIT_ARREST_1, 6u);
	}

	return 6 * (m_sQueueSample.m_nEntityIndex % 3) + sfx;
}

uint32
cAudioManager::GetFBITalkSfx(int16 sound)
{
	uint32 sfx;
	PedState pedState;
	static uint32 lastSfx = NO_SAMPLE;

	if(sound == SOUND_PED_ARREST_FBI) {
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_POLICE_FBI_1_PURSUIT_ARREST_1, 6u);
	} else {
		if(sound != SOUND_PED_PURSUIT_FBI) { return GetGenericMaleTalkSfx(sound); }

		pedState = FindPlayerPed()->m_nPedState;
		if(pedState == PED_ARRESTED || pedState == PED_DEAD || pedState == PED_DIE)
			return NO_SAMPLE;
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_POLICE_FBI_1_PURSUIT_ARREST_1, 6u);
	}

	return 6 * (m_sQueueSample.m_nEntityIndex % 3) + sfx;
}

uint32
cAudioManager::GetArmyTalkSfx(int16 sound)
{
	uint32 sfx;
	PedState pedState;
	static uint32 lastSfx = NO_SAMPLE;

	if(sound != SOUND_PED_PURSUIT_ARMY) { return GetGenericMaleTalkSfx(sound); }

	pedState = FindPlayerPed()->m_nPedState;
	if(pedState == PED_ARRESTED || pedState == PED_DEAD || pedState == PED_DIE)
		return NO_SAMPLE;
	GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_POLICE_ARMY_1_PURSUIT_1, 15u);

	return 15 * (m_sQueueSample.m_nEntityIndex % 1) + sfx;
}

uint32
cAudioManager::GetMedicTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_HANDS_COWER:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_MEDIC_1_HANDS_COWER_1, 5u);
		break;
	case SOUND_PED_CAR_JACKED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_MEDIC_1_CAR_JACKED_1, 5u);
		break;
	case SOUND_PED_HEALING:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_MEDIC_1_HEALING_1, 12u);
		break;
	case SOUND_PED_LEAVE_VEHICLE:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_MEDIC_1_LEAVE_VEHICLE_1, 9u);
		break;
	case SOUND_PED_FLEE_RUN:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_MEDIC_1_FLEE_RUN_1, 6u);
		break;
	default: return GetGenericMaleTalkSfx(sound);
	}
	return 37 * (m_sQueueSample.m_nEntityIndex & 1) + sfx;
}

uint32
cAudioManager::GetFiremanTalkSfx(int16 sound)
{
	return GetGenericMaleTalkSfx(sound);
}

uint32
cAudioManager::GetNormalMaleTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_HANDS_COWER:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_NORMAL_MALE_HANDS_COWER_1, 7u);
		break;
	case SOUND_PED_CAR_JACKED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_NORMAL_MALE_CAR_JACKED_1, 7u);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_NORMAL_MALE_EVADE_1, 9u);
		break;
	case SOUND_PED_FLEE_RUN:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_NORMAL_MALE_FLEE_RUN_1, 5u);
		break;
	case SOUND_PED_CAR_COLLISION:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_NORMAL_MALE_CAR_COLLISION_1, 12u);
		break;
	case SOUND_PED_CHAT_SEXY:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_NORMAL_MALE_CHAT_SEXY_1, 8u);
		break;
	case SOUND_PED_CHAT_EVENT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_NORMAL_MALE_CHAT_EVENT_1, 0xAu);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_NORMAL_MALE_CHAT_1, 25u);
		break;
	default: return GetGenericMaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetTaxiDriverTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	if(sound == SOUND_PED_CAR_JACKED) {
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_TAXI_DRIVER_1_CAR_JACKED_1, 7u);
	} else {
		if(sound != SOUND_PED_CAR_COLLISION) return GetGenericMaleTalkSfx(sound);
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_TAXI_DRIVER_1_CAR_COLLISION_1, 6u);
	}
	return 13 * (m_sQueueSample.m_nEntityIndex & 1) + sfx;
}

uint32
cAudioManager::GetPimpTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_HANDS_UP:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_PIMP_HANDS_UP_1, 7u);
		break;
	case SOUND_PED_CAR_JACKED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_PIMP_CAR_JACKED_1, 4u);
		break;
	case SOUND_PED_DEFEND: GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_PIMP_DEFEND_1, 9u); break;
	case SOUND_PED_EVADE: GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_PIMP_EVADE_1, 6u); break;
	case SOUND_PED_CAR_COLLISION:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_PIMP_CAR_COLLISION_1, 5u);
		break;
	case SOUND_PED_CHAT_EVENT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_PIMP_CHAT_EVENT_1, 2u);
		break;
	case SOUND_PED_CHAT: GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_PIMP_CHAT_1, 17u); break;
	default: return GetGenericMaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetMafiaTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_CAR_JACKING:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_MAFIA_1_CAR_JACKING_1, 2u);
		break;
	case SOUND_PED_CAR_JACKED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_MAFIA_1_CAR_JACKED_1, 2u);
		break;
	case SOUND_PED_ATTACK:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_MAFIA_1_ATTACK_1, 5u);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_MAFIA_1_EVADE_1, 5u);
		break;
	case SOUND_PED_CAR_COLLISION:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_MAFIA_1_CAR_COLLISION_1, 6u);
		break;
	case SOUND_PED_CHAT_SEXY:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_MAFIA_1_CHAT_SEXY_1, 3u);
		break;
	case SOUND_PED_CHAT: GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_MAFIA_1_CHAT_1, 7u); break;
	default: return GetGenericMaleTalkSfx(sound);
	}
	return 30 * (m_sQueueSample.m_nEntityIndex % 3) + sfx;
}

uint32
cAudioManager::GetTriadTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_HANDS_UP:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_TRIAD_1_HANDS_UP_1, 3u);
		break;
	case SOUND_PED_CAR_JACKING:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_TRIAD_1_CAR_JACKING_1, 2u);
		break;
	case SOUND_PED_CAR_JACKED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_TRIAD_1_CAR_JACKED_1, 2u);
		break;
	case SOUND_PED_ATTACK:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_TRIAD_1_ATTACK_1, 5u);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_TRIAD_1_EVADE_1, 4u);
		break;
	case SOUND_PED_CAR_COLLISION:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_TRIAD_1_CAR_COLLISION_1, 7u);
		break;
	case SOUND_PED_CHAT_SEXY:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_TRIAD_1_CHAT_SEXY_1, 3u);
		break;
	case SOUND_PED_CHAT: GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_TRIAD_1_CHAT_1, 8u); break;
	default: return GetGenericMaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetDiabloTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_HANDS_UP:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_DIABLO_1_HANDS_UP_1, 4u);
		break;
	case SOUND_PED_HANDS_COWER:
		sound = SOUND_PED_FLEE_SPRINT;
		return GetGenericMaleTalkSfx(sound);
		break;
	case SOUND_PED_CAR_JACKING:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_DIABLO_1_CAR_JACKING_1, 2u);
		break;
	case SOUND_PED_CAR_JACKED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_DIABLO_1_CAR_JACKED_1, 2u);
		break;
	case SOUND_PED_ATTACK:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_DIABLO_1_ATTACK_1, 4u);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_DIABLO_1_EVADE_1, 4u);
		break;
	case SOUND_PED_CAR_COLLISION:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_DIABLO_1_CAR_COLLISION_1, 5u);
		break;
	case SOUND_PED_CHAT_SEXY:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_DIABLO_1_CHAT_SEXY_1, 4u);
		break;
	case SOUND_PED_CHAT: GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_DIABLO_1_CHAT_1, 5u); break;
	default: return GetGenericMaleTalkSfx(sound);
	}
	return 30 * (m_sQueueSample.m_nEntityIndex & 1) + sfx;
}

uint32
cAudioManager::GetYakuzaTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_CAR_JACKING:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_YAKUZA_1_CAR_JACKING_1, 2u);
		break;
	case SOUND_PED_CAR_JACKED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_YAKUZA_1_CAR_JACKED_1, 2u);
		break;
	case SOUND_PED_ATTACK:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_YAKUZA_1_ATTACK_1, 5u);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_YAKUZA_1_EVADE_1, 4u);
		break;
	case SOUND_PED_CAR_COLLISION:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_YAKUZA_1_CAR_COLLISION_1, 6u);
		break;
	case SOUND_PED_CHAT: GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_YAKUZA_1_CHAT_1, 5u); break;
	default: return GetGenericMaleTalkSfx(sound);
	}
	return 24 * (m_sQueueSample.m_nEntityIndex & 1) + sfx;
}

uint32
cAudioManager::GetYardieTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_HANDS_UP: sfx = AUDIO_SAMPLE_PED_YARDIE_1_HANDS_UP_1; break;
	case SOUND_PED_CAR_JACKING:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_YARDIE_1_CAR_JACKING_1, 2u);
		break;
	case SOUND_PED_CAR_JACKED: sfx = AUDIO_SAMPLE_PED_YARDIE_1_CAR_JACKED_1; break;
	case SOUND_PED_ATTACK:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_YARDIE_1_ATTACK_1, 6u);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_YARDIE_1_EVADE_1, 5u);
		break;
	case SOUND_PED_CAR_COLLISION:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_YARDIE_1_CAR_COLLISION_1, 6u);
		break;
	case SOUND_PED_CHAT_SEXY:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_YARDIE_1_CHAT_SEXY_1, 2u);
		break;
	case SOUND_PED_CHAT: GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_YARDIE_1_CHAT_1, 8u); break;
	default: return GetGenericMaleTalkSfx(sound);
	}
	return 31 * (m_sQueueSample.m_nEntityIndex & 1) + sfx;
}

uint32
cAudioManager::GetColumbianTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_CAR_JACKING:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_COLUMB_1_CAR_JACKING_1, 2u);
		break;
	case SOUND_PED_CAR_JACKED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_COLUMB_1_CAR_JACKED_1, 2u);
		break;
	case SOUND_PED_ATTACK:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_COLUMB_1_ATTACK_1, 5u);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_COLUMB_1_EVADE_1, 5u);
		break;
	case SOUND_PED_CAR_COLLISION:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_COLUMB_1_CAR_COLLISION_1, 6u);
		break;
	case SOUND_PED_CHAT_SEXY:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_COLUMB_1_CHAT_SEXY_1, 2u);
		break;
	case SOUND_PED_CHAT: GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_COLUMB_1_CHAT_1, 5u); break;
	default: return GetGenericMaleTalkSfx(sound);
	}
	return 27 * (m_sQueueSample.m_nEntityIndex & 1) + sfx;
}

uint32
cAudioManager::GetHoodTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_HANDS_UP:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_HOODS_1_HANDS_UP_1, 5u);
		break;
	case SOUND_PED_CAR_JACKING:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_HOODS_1_CAR_JACKING_1, 2u);
		break;
	case SOUND_PED_CAR_JACKED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_HOODS_1_CAR_JACKED_1, 2u);
		break;
	case SOUND_PED_ATTACK:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_HOODS_1_ATTACK_1, 6u);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_HOODS_1_EVADE_1, 5u);
		break;
	case SOUND_PED_CAR_COLLISION:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_HOODS_1_CAR_COLLISION_1, 7u);
		break;
	case SOUND_PED_CHAT_SEXY:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_HOODS_1_CHAT_SEXY_1, 2u);
		break;
	case SOUND_PED_CHAT: GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_HOODS_1_CHAT_1, 6u); break;

	default: return GetGenericMaleTalkSfx(sound); break;
	}
	return 35 * (m_sQueueSample.m_nEntityIndex & 1) + sfx;
}

uint32
cAudioManager::GetBlackCriminalTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_HANDS_UP:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_CRIMINAL_1_HANDS_UP_1, 4u);
		break;
	case SOUND_PED_CAR_JACKING: sfx = AUDIO_SAMPLE_PED_BLACK_CRIMINAL_1_CAR_JACKING_1; break;
	case SOUND_PED_MUGGING:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_CRIMINAL_1_MUGGING_1, 2u);
		break;
	case SOUND_PED_ATTACK:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_CRIMINAL_1_ATTACK_1, 5u);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_CRIMINAL_1_EVADE_1, 6u);
		break;
	case SOUND_PED_CAR_COLLISION:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_CRIMINAL_1_CAR_COLLISION_1, 5u);
		break;
	default: return GetGenericMaleTalkSfx(sound); break;
	}
	return sfx;
}

uint32
cAudioManager::GetWhiteCriminalTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_HANDS_UP:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_WHITE_CRIMINAL_1_HANDS_UP_1, 3u);
		break;
	case SOUND_PED_CAR_JACKING: sfx = AUDIO_SAMPLE_PED_WHITE_CRIMINAL_1_CAR_JACKING_1; break;
	case SOUND_PED_MUGGING:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_WHITE_CRIMINAL_1_MUGGING_1, 2u);
		break;
	case SOUND_PED_ATTACK:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_WHITE_CRIMINAL_1_ATTACK_1, 4u);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_WHITE_CRIMINAL_1_EVADE_1, 5u);
		break;
	case SOUND_PED_CAR_COLLISION:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_WHITE_CRIMINAL_1_CAR_COLLISION_1, 4u);
		break;
	default: return GetGenericMaleTalkSfx(sound); break;
	}
	return sfx;
}

uint32
cAudioManager::GetMaleNo2TalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_CAR_JACKED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_MALE_NO_2_CAR_JACKED_1, 3u);
		break;
	case SOUND_PED_ROBBED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_MALE_NO_2_ROBBED_1, 4u);
		break;
	case SOUND_PED_ATTACK:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_MALE_NO_2_ATTACK_1, 4u);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_MALE_NO_2_EVADE_1, 4u);
		break;
	case SOUND_PED_CAR_COLLISION:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_MALE_NO_2_CAR_COLLISION_1, 7u);
		break;
	case SOUND_PED_CHAT_SEXY:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_MALE_NO_2_CHAT_SEXY_1, 5u);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_MALE_NO_2_CHAT_1, 7u);
		break;
	default: return GetGenericMaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetBlackProjectMaleTalkSfx(int16 sound, int32 model)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_HANDS_UP:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_PROJECT_MALE_1_HANDS_UP_1, 3u);
		break;
	case SOUND_PED_CAR_JACKED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_PROJECT_MALE_1_CAR_JACKED_1, 2u);
		break;
	case SOUND_PED_ROBBED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_PROJECT_MALE_1_ROBBED_1, 2u);
		break;
	case SOUND_PED_ATTACK:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_PROJECT_MALE_1_ATTACK_1, 6u);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_PROJECT_MALE_1_EVADE_1, 5u);
		break;
	case SOUND_PED_CAR_COLLISION:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_PROJECT_MALE_1_CAR_COLLISION_1,
		          7u);
		break;
	case SOUND_PED_CHAT_SEXY:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_PROJECT_MALE_1_CHAT_SEXY_1, 3u);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_PROJECT_MALE_1_CHAT_1, 6u);
	default: return GetGenericMaleTalkSfx(sound);
	}

	if(model == MI_P_MAN2) sfx += 34;
	return sfx;
}

uint32
cAudioManager::GetWhiteFatMaleTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_CAR_JACKED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_WHITE_FAT_MALE_1_CAR_JACKED_1, 3u);
		break;
	case SOUND_PED_ROBBED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_WHITE_FAT_MALE_1_ROBBED_1, 3u);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_WHITE_FAT_MALE_1_EVADE_1, 9u);
		break;
	case SOUND_PED_CAR_COLLISION:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_WHITE_FAT_MALE_1_CAR_COLLISION_1, 9u);
		break;
	case SOUND_PED_WAIT_DOUBLEBACK:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_WHITE_FAT_MALE_1_WAIT_DOUBLEBACK_1, 2u);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_WHITE_FAT_MALE_1_CHAT_1, 9u);
	default: return GetGenericMaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetBlackFatMaleTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_CAR_JACKED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_FAT_MALE_1_CAR_JACKED_1, 4u);
		break;
	case SOUND_PED_ROBBED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_FAT_MALE_1_ROBBED_1, 3u);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_FAT_MALE_1_EVADE_1, 7u);
		break;
	case SOUND_PED_CAR_COLLISION:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_FAT_MALE_1_CAR_COLLISION_1, 6u);
		break;
	case SOUND_PED_WAIT_DOUBLEBACK:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_FAT_MALE_1_WAIT_DOUBLEBACK_1, 3u);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_FAT_MALE_1_CHAT_1, 8u);
		break;
	default: return GetGenericMaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetBlackCasualFemaleTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_HANDS_COWER:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_CASUAL_FEMALE_1_HANDS_COWER_1, 2u);
		break;
	case SOUND_PED_CAR_JACKED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_CASUAL_FEMALE_1_CAR_JACKED_1, 2u);
		break;
	case SOUND_PED_ROBBED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_CASUAL_FEMALE_1_ROBBED_1, 3u);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_CASUAL_FEMALE_1_EVADE_1, 6u);
		break;
	case SOUND_PED_FLEE_RUN:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_CASUAL_FEMALE_1_FLEE_RUN_1, 2u);
		break;
	case SOUND_PED_CAR_COLLISION:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_CASUAL_FEMALE_1_CAR_COLLISION_1,
		          7u);
		break;
	case SOUND_PED_CHAT_EVENT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_CASUAL_FEMALE_1_CHAT_EVENT_1, 4u);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_CASUAL_FEMALE_1_CHAT_1, 8u);
		break;
	default: return GetGenericFemaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetWhiteCasualFemaleTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_HANDS_COWER:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_WHITE_CASUAL_FEMALE_1_HANDS_COWER_1, 2u);
		break;
	case SOUND_PED_CAR_JACKED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_WHITE_CASUAL_FEMALE_1_CAR_JACKED_1, 2u);
		break;
	case SOUND_PED_ROBBED: sfx = AUDIO_SAMPLE_PED_WHITE_CASUAL_FEMALE_1_ROBBED_1; break;
	case SOUND_PED_EVADE:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_WHITE_CASUAL_FEMALE_1_EVADE_1, 3u);
		break;
	case SOUND_PED_FLEE_RUN:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_WHITE_CASUAL_FEMALE_1_FLEE_RUN_1, 2u);
		break;
	case SOUND_PED_CAR_COLLISION:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_WHITE_CASUAL_FEMALE_1_CAR_COLLISION_1,
		          8u);
		break;
	case SOUND_PED_CHAT_EVENT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_WHITE_CASUAL_FEMALE_1_CHAT_EVENT_1, 2u);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_WHITE_CASUAL_FEMALE_1_CHAT_1, 4u);
		break;
	default: return GetGenericFemaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetFemaleNo3TalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_HANDS_COWER:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_FEMALE_NO_3_HANDS_COWER_1, 5u);
		break;
	case SOUND_PED_CAR_JACKED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_FEMALE_NO_3_CAR_JACKED_1, 3u);
		break;
	case SOUND_PED_ROBBED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_FEMALE_NO_3_ROBBED_1, 3u);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_FEMALE_NO_3_EVADE_1, 6u);
		break;
	case SOUND_PED_FLEE_RUN:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_FEMALE_NO_3_FLEE_RUN_1, 4u);
		break;
	case SOUND_PED_CAR_COLLISION:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_FEMALE_NO_3_CAR_COLLISION_1, 6u);
		break;
	case SOUND_PED_CHAT_EVENT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_FEMALE_NO_3_CHAT_EVENT_1, 4u);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_FEMALE_NO_3_CHAT_1, 5u);
		break;
	default: return GetGenericFemaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetBlackFatFemaleTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_HANDS_COWER:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_FAT_FEMALE_1_HANDS_COWER_1, 4u);
		break;
	case SOUND_PED_CAR_JACKED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_FAT_FEMALE_1_CAR_JACKED_1, 2u);
		break;
	case SOUND_PED_ROBBED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_FAT_FEMALE_1_ROBBED_1, 2u);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_FAT_FEMALE_1_EVADE_1, 5u);
		break;
	case SOUND_PED_CAR_COLLISION:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_FAT_FEMALE_1_CAR_COLLISION_1, 6u);
		break;
	case SOUND_PED_CHAT_EVENT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_FAT_FEMALE_1_CHAT_EVENT_1, 5u);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_FAT_FEMALE_1_CHAT_1, 7u);
		break;
	default: return GetGenericFemaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetWhiteFatFemaleTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_CAR_JACKED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_WHITE_FAT_FEMALE_1_CAR_JACKED_1, 2u);
		break;
	case SOUND_PED_ROBBED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_WHITE_FAT_FEMALE_1_ROBBED_1, 2u);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_WHITE_FAT_FEMALE_1_EVADE_1, 6u);
		break;
	case SOUND_PED_CAR_COLLISION:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_WHITE_FAT_FEMALE_1_CAR_COLLISION_1, 8u);
		break;
	case SOUND_PED_WAIT_DOUBLEBACK:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_WHITE_FAT_FEMALE_1_WAIT_DOUBLEBACK_1,
		          2u);
		break;
	case SOUND_PED_CHAT_EVENT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_WHITE_FAT_FEMALE_1_CHAT_EVENT_1, 4u);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_WHITE_FAT_FEMALE_1_CHAT_1, 8u);
		break;
	default: return GetGenericFemaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetBlackFemaleProstituteTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_HANDS_UP:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_FEMALE_PROSTITUTE_1_HANDS_UP_1,
		          4u);
		break;
	case SOUND_PED_ROBBED: sfx = AUDIO_SAMPLE_PED_BLACK_FEMALE_PROSTITUTE_1_ROBBED_1; break;
	case SOUND_PED_ATTACK:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_FEMALE_PROSTITUTE_1_ATTACK_1, 4u);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_FEMALE_PROSTITUTE_1_EVADE_1, 3u);
		break;
	case SOUND_PED_CAR_COLLISION:
		GetPhrase(&sfx, &lastSfx,
		          AUDIO_SAMPLE_PED_BLACK_FEMALE_PROSTITUTE_1_CAR_COLLISION_1, 4u);
		break;
	case SOUND_PED_SOLICIT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_FEMALE_PROSTITUTE_1_SOLICIT_1, 8u);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_FEMALE_PROSTITUTE_1_CHAT_1, 4u);
		break;
	default: return GetGenericFemaleTalkSfx(sound);
	}
	return 28 * (m_sQueueSample.m_nEntityIndex & 1) + sfx;
}

uint32
cAudioManager::GetWhiteFemaleProstituteTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_ROBBED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_WHITE_FEMALE_PROSTITUTE_1_ROBBED_1, 2u);
		break;
	case SOUND_PED_ATTACK:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_WHITE_FEMALE_PROSTITUTE_1_ATTACK_1, 4u);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_WHITE_FEMALE_PROSTITUTE_1_EVADE_1, 3u);
		break;
	case SOUND_PED_CAR_COLLISION:
		GetPhrase(&sfx, &lastSfx,
		          AUDIO_SAMPLE_PED_WHITE_FEMALE_PROSTITUTE_1_CAR_COLLISION_1, 4u);
		break;
	case SOUND_PED_SOLICIT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_WHITE_FEMALE_PROSTITUTE_1_SOLICIT_1, 8u);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_WHITE_FEMALE_PROSTITUTE_1_CHAT_1, 4u);
		break;
	default: return GetGenericFemaleTalkSfx(sound);
	}
	return 25 * (m_sQueueSample.m_nEntityIndex & 1) + sfx;
}

uint32
cAudioManager::GetBlackProjectFemaleOldTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_CAR_JACKED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_PROJECT_FEMALE_OLD_1_CAR_JACKED_1,
		          6u);
		break;
	case SOUND_PED_ROBBED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_PROJECT_FEMALE_OLD_1_ROBBED_1, 2u);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_PROJECT_FEMALE_OLD_1_EVADE_1, 10u);
		break;
	case SOUND_PED_FLEE_RUN:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_PROJECT_FEMALE_OLD_1_FLEE_RUN_1,
		          6u);
		break;
	case SOUND_PED_CAR_COLLISION:
		GetPhrase(&sfx, &lastSfx,
		          AUDIO_SAMPLE_PED_BLACK_PROJECT_FEMALE_OLD_1_CAR_COLLISION_1, 7u);
		break;
	case SOUND_PED_CHAT_EVENT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_PROJECT_FEMALE_OLD_1_CHAT_EVENT_1,
		          2u);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_PROJECT_FEMALE_OLD_1_CHAT_1, 10u);
		break;
	default: return GetGenericFemaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetBlackProjectFemaleYoungTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_HANDS_COWER:
		GetPhrase(&sfx, &lastSfx,
		          AUDIO_SAMPLE_PED_BLACK_PROJECT_FEMALE_YOUNG_1_HANDS_COWER_1, 4u);
		break;
	case SOUND_PED_CAR_JACKED:
		sfx = AUDIO_SAMPLE_PED_BLACK_PROJECT_FEMALE_YOUNG_1_CAR_JACKED_1;
		break;
	case SOUND_PED_ROBBED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_PROJECT_FEMALE_YOUNG_1_ROBBED_1,
		          2u);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_PROJECT_FEMALE_YOUNG_1_EVADE_1,
		          5u);
		break;
	case SOUND_PED_CAR_COLLISION:
		GetPhrase(&sfx, &lastSfx,
		          AUDIO_SAMPLE_PED_BLACK_PROJECT_FEMALE_YOUNG_1_CAR_COLLISION_1, 6u);
		break;
	case SOUND_PED_CHAT_EVENT:
		GetPhrase(&sfx, &lastSfx,
		          AUDIO_SAMPLE_PED_BLACK_PROJECT_FEMALE_YOUNG_1_CHAT_EVENT_1, 5u);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_PROJECT_FEMALE_YOUNG_1_CHAT_1, 7u);
		break;
	default: return GetGenericFemaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetChinatownMaleOldTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_HANDS_COWER:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_CHINATOWN_MALE_OLD_HANDS_COWER_1, 3u);
		break;
	case SOUND_PED_CAR_JACKED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_CHINATOWN_MALE_OLD_CAR_JACKED_1, 2u);
		break;
	case SOUND_PED_ATTACK:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_CHINATOWN_MALE_OLD_ATTACK_1, 5u);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_CHINATOWN_MALE_OLD_EVADE_1, 6u);
		break;
	case SOUND_PED_CAR_COLLISION:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_CHINATOWN_MALE_OLD_CAR_COLLISION_1, 6u);
		break;
	case SOUND_PED_CHAT_SEXY:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_CHINATOWN_MALE_OLD_CHAT_SEXY_1, 3u);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_CHINATOWN_MALE_OLD_CHAT_1, 7u);
		break;
	default: return GetGenericMaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetChinatownMaleYoungTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_HANDS_COWER:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_CHINATOWN_MALE_YOUNG_HANDS_COWER_1, 2u);
		break;
	case SOUND_PED_CAR_JACKED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_CHINATOWN_MALE_YOUNG_CAR_JACKED_1, 2u);
		break;
	case SOUND_PED_ATTACK:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_CHINATOWN_MALE_YOUNG_ATTACK_1, 6u);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_CHINATOWN_MALE_YOUNG_EVADE_1, 5u);
		break;
	case SOUND_PED_CAR_COLLISION:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_CHINATOWN_MALE_YOUNG_CAR_COLLISION_1,
		          6u);
		break;
	case SOUND_PED_CHAT_SEXY:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_CHINATOWN_MALE_YOUNG_CHAT_SEXY_1, 3u);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_CHINATOWN_MALE_YOUNG_CHAT_1, 6u);
		break;
	default: return GetGenericMaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetChinatownFemaleOldTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_HANDS_COWER:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_CHINATOWN_FEMALE_OLD_HANDS_COWER_1, 3u);
		break;
	case SOUND_PED_ROBBED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_CHINATOWN_FEMALE_OLD_ROBBED_1, 2u);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_CHINATOWN_FEMALE_OLD_EVADE_1, 5u);
		break;
	case SOUND_PED_CAR_COLLISION:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_CHINATOWN_FEMALE_OLD_CAR_COLLISION_1,
		          5u);
		break;
	case SOUND_PED_CHAT_EVENT: sfx = AUDIO_SAMPLE_PED_CHINATOWN_FEMALE_OLD_CHAT_EVENT_1; break;
	case SOUND_PED_CHAT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_CHINATOWN_FEMALE_OLD_CHAT_1, 6u);
		break;
	default: return GetGenericFemaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetChinatownFemaleYoungTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_CAR_JACKED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_CHINATOWN_FEMALE_YOUNG_CAR_JACKED_1, 2u);
		break;
	case SOUND_PED_ROBBED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_CHINATOWN_FEMALE_YOUNG_ROBBED_1, 2u);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_CHINATOWN_FEMALE_YOUNG_EVADE_1, 6u);
		break;
	case SOUND_PED_CAR_COLLISION:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_CHINATOWN_FEMALE_YOUNG_CAR_COLLISION_1,
		          7u);
		break;
	case SOUND_PED_CHAT_EVENT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_CHINATOWN_FEMALE_YOUNG_CHAT_EVENT_1, 4u);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_CHINATOWN_FEMALE_YOUNG_CHAT_1, 7u);
		break;
	default: return GetGenericFemaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetLittleItalyMaleTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_HANDS_COWER:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_LITTLE_ITALY_MALE_1_HANDS_COWER_1, 3u);
		break;
	case SOUND_PED_CAR_JACKED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_LITTLE_ITALY_MALE_1_CAR_JACKED_1, 2u);
		break;
	case SOUND_PED_ROBBED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_LITTLE_ITALY_MALE_1_ROBBED_1, 2u);
		break;
	case SOUND_PED_ATTACK:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_LITTLE_ITALY_MALE_1_ATTACK_1, 5u);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_LITTLE_ITALY_MALE_1_EVADE_1, 5u);
		break;
	case SOUND_PED_CAR_COLLISION:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_LITTLE_ITALY_MALE_1_CAR_COLLISION_1, 7u);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_LITTLE_ITALY_MALE_1_CHAT_1, 6u);
		break;
	default: return GetGenericMaleTalkSfx(sound);
	}
	return 30 * (m_sQueueSample.m_nEntityIndex & 1) + sfx;
}

uint32
cAudioManager::GetLittleItalyFemaleOldTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_CAR_JACKED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_LITTLE_ITALY_FEMALE_OLD_CAR_JACKED_1,
		          2u);
		break;
	case SOUND_PED_ROBBED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_LITTLE_ITALY_FEMALE_OLD_ROBBED_1, 2u);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_LITTLE_ITALY_FEMALE_OLD_EVADE_1, 6u);
		break;
	case SOUND_PED_CAR_COLLISION:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_LITTLE_ITALY_FEMALE_OLD_CAR_COLLISION_1,
		          7u);
		break;
	case SOUND_PED_CHAT_EVENT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_LITTLE_ITALY_FEMALE_OLD_CHAT_EVENT_1,
		          4u);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_LITTLE_ITALY_FEMALE_OLD_CHAT_1, 7u);
		break;
	default: return GetGenericFemaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetLittleItalyFemaleYoungTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_CAR_JACKED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_LITTLE_ITALY_FEMALE_YOUNG_CAR_JACKED_1,
		          2u);
		break;
	case SOUND_PED_ROBBED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_LITTLE_ITALY_FEMALE_YOUNG_ROBBED_1, 2u);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_LITTLE_ITALY_FEMALE_YOUNG_EVADE_1, 7u);
		break;
	case SOUND_PED_CAR_COLLISION:
		GetPhrase(&sfx, &lastSfx,
		          AUDIO_SAMPLE_PED_LITTLE_ITALY_FEMALE_YOUNG_CAR_COLLISION_1, 6u);
		break;
	case SOUND_PED_CHAT_EVENT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_LITTLE_ITALY_FEMALE_YOUNG_CHAT_EVENT_1,
		          4u);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_LITTLE_ITALY_FEMALE_YOUNG_CHAT_1, 6u);
		break;
	default: return GetGenericFemaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetWhiteDockerMaleTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_HANDS_COWER:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_WHITE_DOCKER_MALE_HANDS_COWER_1, 2u);
		break;
	case SOUND_PED_ATTACK:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_WHITE_DOCKER_MALE_ATTACK_1, 3u);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_WHITE_DOCKER_MALE_EVADE_1, 4u);
		break;
	case SOUND_PED_CAR_COLLISION:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_WHITE_DOCKER_MALE_CAR_COLLISION_1, 4u);
		break;
	case SOUND_PED_CHAT_SEXY:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_WHITE_DOCKER_MALE_CHAT_SEXY_1, 3u);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_WHITE_DOCKER_MALE_CHAT_1, 5u);
		break;
	default: return GetGenericMaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetBlackDockerMaleTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_HANDS_COWER:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_DOCKER_MALE_HANDS_COWER_1, 3u);
		break;
	case SOUND_PED_ATTACK:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_DOCKER_MALE_ATTACK_1, 5u);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_DOCKER_MALE_EVADE_1, 5u);
		break;
	case SOUND_PED_CAR_COLLISION:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_DOCKER_MALE_CAR_COLLISION_1, 6u);
		break;
	case SOUND_PED_CHAT_SEXY:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_DOCKER_MALE_CHAT_SEXY_1, 3u);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_DOCKER_MALE_CHAT_1, 5u);
		break;
	default: return GetGenericMaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetScumMaleTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_HANDS_COWER:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_SCUM_MALE_HANDS_COWER_1, 5u);
		break;
	case SOUND_PED_ROBBED: sfx = AUDIO_SAMPLE_PED_SCUM_MALE_ROBBED_1; break;
	case SOUND_PED_ATTACK:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_SCUM_MALE_ATTACK_1, 0xAu);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_SCUM_MALE_EVADE_1, 5u);
		break;
	case SOUND_PED_CAR_COLLISION:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_SCUM_MALE_CAR_COLLISION_1, 6u);
		break;
	case SOUND_PED_WAIT_DOUBLEBACK:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_SCUM_MALE_WAIT_DOUBLEBACK_1, 3u);
		break;
	case SOUND_PED_CHAT_SEXY:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_SCUM_MALE_CHAT_SEXY_1, 5u);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_SCUM_MALE_CHAT_1, 9u);
		break;
	default: return GetGenericMaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetScumFemaleTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_HANDS_COWER:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_SCUM_FEMALE_HANDS_COWER_1, 4u);
		break;
	case SOUND_PED_ROBBED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_SCUM_FEMALE_ROBBED_1, 2u);
		break;
	case SOUND_PED_ATTACK:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_SCUM_FEMALE_ATTACK_1, 4u);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_SCUM_FEMALE_EVADE_1, 8u);
		break;
	case SOUND_PED_CAR_COLLISION:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_SCUM_FEMALE_CAR_COLLISION_1, 5u);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_SCUM_FEMALE_CHAT_1, 13u);
		break;
	default: return GetGenericFemaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetWhiteWorkerMaleTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_HANDS_COWER:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_WHITE_WORKER_MALE_HANDS_COWER_1, 3u);
		break;
	case SOUND_PED_ATTACK:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_WHITE_WORKER_MALE_ATTACK_1, 3u);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_WHITE_WORKER_MALE_EVADE_1, 4u);
		break;
	case SOUND_PED_CAR_COLLISION:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_WHITE_WORKER_MALE_CAR_COLLISION_1, 6u);
		break;
	case SOUND_PED_CHAT_SEXY:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_WHITE_WORKER_MALE_CHAT_SEXY_1, 2u);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_WHITE_WORKER_MALE_CHAT_1, 6u);
		break;
	default: return GetGenericMaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetBlackWorkerMaleTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_HANDS_COWER:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_WORKER_MALE_HANDS_COWER_1, 4u);
		break;
	case SOUND_PED_ATTACK:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_WORKER_MALE_ATTACK_1, 3u);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_WORKER_MALE_EVADE_1, 3u);
		break;
	case SOUND_PED_CAR_COLLISION:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_WORKER_MALE_CAR_COLLISION_1, 4u);
		break;
	case SOUND_PED_CHAT_SEXY:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_WORKER_MALE_CHAT_SEXY_1, 3u);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_WORKER_MALE_CHAT_1, 4u);

		break;
	default: return GetGenericMaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetBusinessMaleYoungTalkSfx(int16 sound, int32 model)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_HANDS_COWER:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BUSINESS_MALE_YOUNG_1_HANDS_COWER_1, 3u);
		break;
	case SOUND_PED_CAR_JACKED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BUSINESS_MALE_YOUNG_1_CAR_JACKED_1, 2u);
		break;
	case SOUND_PED_ROBBED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BUSINESS_MALE_YOUNG_1_ROBBED_1, 2u);
		break;
	case SOUND_PED_ATTACK:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BUSINESS_MALE_YOUNG_1_ATTACK_1, 4u);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BUSINESS_MALE_YOUNG_1_EVADE_1, 4u);
		break;
	case SOUND_PED_FLEE_RUN:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BUSINESS_MALE_YOUNG_1_FLEE_RUN_1, 5u);
		break;
	case SOUND_PED_CAR_COLLISION:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BUSINESS_MALE_YOUNG_1_CAR_COLLISION_1,
		          6u);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BUSINESS_MALE_YOUNG_1_CHAT_1, 6u);
		break;
	default: return GetGenericMaleTalkSfx(sound);
	}

	if(model == 61) sfx += 32;
	return sfx;
}

uint32
cAudioManager::GetBusinessMaleOldTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_HANDS_COWER:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BUSINESS_MALE_OLD_1_HANDS_COWER_1, 3u);
		break;
	case SOUND_PED_CAR_JACKED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BUSINESS_MALE_OLD_1_CAR_JACKED_1, 2u);
		break;
	case SOUND_PED_ROBBED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BUSINESS_MALE_OLD_1_ROBBED_1, 2u);
		break;
	case SOUND_PED_ATTACK:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BUSINESS_MALE_OLD_1_ATTACK_1, 5u);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BUSINESS_MALE_OLD_1_EVADE_1, 4u);
		break;
	case SOUND_PED_FLEE_RUN:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BUSINESS_MALE_OLD_1_FLEE_RUN_1, 5u);
		break;
	case SOUND_PED_CAR_COLLISION:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BUSINESS_MALE_OLD_1_CAR_COLLISION_1, 5u);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BUSINESS_MALE_OLD_1_CHAT_1, 5u);

		break;
	default: return GetGenericMaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetWhiteBusinessFemaleTalkSfx(int16 sound, int32 model)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_HANDS_COWER:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BUSINESS_WOMAN_1_HANDS_COWER_1, 4u);
		break;
	case SOUND_PED_CAR_JACKED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BUSINESS_WOMAN_1_CAR_JACKED_1, 2u);
		break;
	case SOUND_PED_ROBBED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BUSINESS_WOMAN_1_ROBBED_1, 2u);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BUSINESS_WOMAN_1_EVADE_1, 6u);
		break;
	case SOUND_PED_FLEE_RUN:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BUSINESS_WOMAN_1_FLEE_RUN_1, 4u);
		break;
	case SOUND_PED_CAR_COLLISION:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BUSINESS_WOMAN_1_CAR_COLLISION_1, 5u);
		break;
	case SOUND_PED_CHAT_EVENT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BUSINESS_WOMAN_1_CHAT_EVENT_1, 4u);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BUSINESS_WOMAN_1_CHAT_1, 7u);
		break;
	default: return GetGenericFemaleTalkSfx(sound);
	}

	if(model == MI_B_WOM2) sfx += 34;
	return sfx;
}

uint32
cAudioManager::GetBlackBusinessFemaleTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_HANDS_COWER:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_BUSINESS_FEMALE_HANDS_COWER_1, 5u);
		break;
	case SOUND_PED_CAR_JACKED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_BUSINESS_FEMALE_CAR_JACKED_1, 4u);
		break;
	case SOUND_PED_ROBBED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_BUSINESS_FEMALE_ROBBED_1, 3u);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_BUSINESS_FEMALE_EVADE_1, 6u);
		break;
	case SOUND_PED_FLEE_RUN:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_BUSINESS_FEMALE_FLEE_RUN_1, 6u);
		break;
	case SOUND_PED_CAR_COLLISION:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_BUSINESS_FEMALE_CAR_COLLISION_1,
		          7u);
		break;
	case SOUND_PED_CHAT_EVENT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_BUSINESS_FEMALE_CHAT_EVENT_1, 4u);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_BUSINESS_FEMALE_CHAT_1, 7u);

		break;
	default: return GetGenericFemaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetSupermodelMaleTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_CAR_JACKED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_SUPERMODEL_MALE_CAR_JACKED_1, 2u);
		break;
	case SOUND_PED_ROBBED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_SUPERMODEL_MALE_ROBBED_1, 2u);
		break;
	case SOUND_PED_ATTACK:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_SUPERMODEL_MALE_ATTACK_1, 5u);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_SUPERMODEL_MALE_EVADE_1, 6u);
		break;
	case SOUND_PED_CAR_COLLISION:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_SUPERMODEL_MALE_CAR_COLLISION_1, 6u);
		break;
	case SOUND_PED_CHAT_SEXY:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_SUPERMODEL_MALE_CHAT_SEXY_1, 3u);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_SUPERMODEL_MALE_CHAT_1, 6u);

		break;
	default: return GetGenericMaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetSupermodelFemaleTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_HANDS_COWER:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_SUPERMODEL_FEMALE_HANDS_COWER_1, 4u);
		break;
	case SOUND_PED_ROBBED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_SUPERMODEL_FEMALE_ROBBED_1, 3u);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_SUPERMODEL_FEMALE_EVADE_1, 4u);
		break;
	case SOUND_PED_CAR_COLLISION:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_SUPERMODEL_FEMALE_CAR_COLLISION_1, 7u);
		break;
	case SOUND_PED_CHAT_EVENT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_SUPERMODEL_FEMALE_CHAT_EVENT_1, 5u);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_SUPERMODEL_FEMALE_CHAT_1, 8u);

		break;
	default: return GetGenericFemaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetStewardMaleTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_HANDS_COWER:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_STEWARD_MALE_HANDS_COWER_1, 3u);
		break;
	case SOUND_PED_ATTACK:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_STEWARD_MALE_ATTACK_1, 4u);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_STEWARD_MALE_EVADE_1, 3u);
		break;
	case SOUND_PED_CAR_COLLISION:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_STEWARD_MALE_CAR_COLLISION_1, 5u);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_STEWARD_MALE_CHAT_1, 4u);

		break;
	default: return GetGenericMaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetStewardFemaleTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_HANDS_COWER:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_STEWARD_FEMALE_1_HANDS_COWER_1, 3u);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_STEWARD_FEMALE_1_EVADE_1, 5u);
		break;
	case SOUND_PED_CAR_COLLISION:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_STEWARD_FEMALE_1_CAR_COLLISION_1, 5u);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_STEWARD_FEMALE_1_CHAT_1, 5u);
		break;
	default: return GetGenericFemaleTalkSfx(sound);
	}
	return 18 * (m_sQueueSample.m_nEntityIndex & 1) + sfx;
}

uint32
cAudioManager::GetFanMaleTalkSfx(int16 sound, int32 model)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_ATTACK:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_FAN_MALE_1_ATTACK_1, 3u);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_FAN_MALE_1_EVADE_1, 4u);
		break;
	case SOUND_PED_CAR_COLLISION:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_FAN_MALE_1_CAR_COLLISION_1, 5u);
		break;
	case SOUND_PED_CHAT_EVENT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_FAN_MALE_1_CHAT_EVENT_1, 2u);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_FAN_MALE_1_CHAT_1, 6u);
		break;
	default: return GetGenericMaleTalkSfx(sound);
	}

	if(model == MI_FAN_MAN2) sfx += 20;
	return sfx;
}

uint32
cAudioManager::GetFanFemaleTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_ROBBED: sfx = AUDIO_SAMPLE_PED_FAN_FEMALE_1_ROBBED_1; break;
	case SOUND_PED_EVADE:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_FAN_FEMALE_1_EVADE_1, 4u);
		break;
	case SOUND_PED_CAR_COLLISION:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_FAN_FEMALE_1_CAR_COLLISION_1, 5u);
		break;
	case SOUND_PED_CHAT_EVENT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_FAN_FEMALE_1_CHAT_EVENT_1, 2u);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_FAN_FEMALE_1_CHAT_1, 6u);
		break;
	default: return GetGenericFemaleTalkSfx(sound);
	}
	return 18 * (m_sQueueSample.m_nEntityIndex & 1) + sfx;
}

uint32
cAudioManager::GetHospitalMaleTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_HANDS_COWER:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_HOSPITAL_MALE_HANDS_COWER_1, 4u);
		break;
	case SOUND_PED_ATTACK:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_HOSPITAL_MALE_ATTACK_1, 4u);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_HOSPITAL_MALE_EVADE_1, 4u);
		break;
	case SOUND_PED_CAR_COLLISION:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_HOSPITAL_MALE_CAR_COLLISION_1, 5u);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_HOSPITAL_MALE_CHAT_1, 5u);
		break;
	default: return GetGenericMaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetHospitalFemaleTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_EVADE:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_HOSPITAL_FEMALE_EVADE_1, 5u);
		break;
	case SOUND_PED_CAR_COLLISION:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_HOSPITAL_FEMALE_CAR_COLLISION_1, 6u);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_HOSPITAL_FEMALE_CHAT_1, 6u);

		break;
	default: return GetGenericFemaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetWhiteConstructionWorkerTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_HANDS_COWER:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_WHITE_CONSTRUCTION_WORKER_HANDS_COWER_1,
		          3u);
		break;
	case SOUND_PED_CAR_JACKED:
		sfx = AUDIO_SAMPLE_PED_WHITE_CONSTRUCTION_WORKER_CAR_JACKED_1;
		break;
	case SOUND_PED_ATTACK:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_WHITE_CONSTRUCTION_WORKER_ATTACK_1, 5u);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_WHITE_CONSTRUCTION_WORKER_EVADE_1, 5u);
		break;
	case SOUND_PED_CAR_COLLISION:
		GetPhrase(&sfx, &lastSfx,
		          AUDIO_SAMPLE_PED_WHITE_CONSTRUCTION_WORKER_CAR_COLLISION_1, 4u);
		break;
	case SOUND_PED_CHAT_SEXY:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_WHITE_CONSTRUCTION_WORKER_CHAT_SEXY_1,
		          3u);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_WHITE_CONSTRUCTION_WORKER_CHAT_1, 7u);
		break;
	default: return GetGenericMaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetBlackConstructionWorkerTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_HANDS_COWER:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_CONSTRUCTION_WORKER_HANDS_COWER_1,
		          3u);
		break;
	case SOUND_PED_CAR_JACKED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_CONSTRUCTION_WORKER_CAR_JACKED_1,
		          2u);
		break;
	case SOUND_PED_ATTACK:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_CONSTRUCTION_WORKER_ATTACK_1, 5u);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_CONSTRUCTION_WORKER_EVADE_1, 5u);
		break;
	case SOUND_PED_CAR_COLLISION:
		GetPhrase(&sfx, &lastSfx,
		          AUDIO_SAMPLE_PED_BLACK_CONSTRUCTION_WORKER_CAR_COLLISION_1, 5u);
		break;
	case SOUND_PED_CHAT_SEXY:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_CONSTRUCTION_WORKER_CHAT_SEXY_1,
		          4u);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_CONSTRUCTION_WORKER_CHAT_1, 4u);
		break;
	default: return GetGenericMaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetShopperFemaleTalkSfx(int16 sound, int32 model)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_CAR_JACKED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_SHOPPER_FEMALE_1_CAR_JACKED_1, 2u);
		break;
	case SOUND_PED_ROBBED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_SHOPPER_FEMALE_1_ROBBED_1, 2u);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_SHOPPER_FEMALE_1_EVADE_1, 6u);
		break;
	case SOUND_PED_CAR_COLLISION:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_SHOPPER_FEMALE_1_CAR_COLLISION_1, 7u);
		break;
	case SOUND_PED_CHAT_EVENT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_SHOPPER_FEMALE_1_CHAT_EVENT_1, 4u);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_SHOPPER_FEMALE_1_CHAT_1, 7u);
		break;
	default: return GetGenericFemaleTalkSfx(sound);
	}

	if(model == MI_SHOPPER2) {
		sfx += 28;
	} else if(model == MI_SHOPPER3) {
		sfx += 56;
	}
	return sfx;
}

uint32
cAudioManager::GetStudentMaleTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_HANDS_COWER:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_STUDENT_MALE_HANDS_COWER_1, 2u);
		break;
	case SOUND_PED_ROBBED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_STUDENT_MALE_ROBBED_1, 2u);
		break;
	case SOUND_PED_ATTACK:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_STUDENT_MALE_ATTACK_1, 4u);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_STUDENT_MALE_EVADE_1, 4u);
		break;
	case SOUND_PED_CAR_COLLISION:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_STUDENT_MALE_CAR_COLLISION_1, 4u);
		break;
	case SOUND_PED_CHAT_EVENT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_STUDENT_MALE_CHAT_EVENT_1, 3u);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_STUDENT_MALE_CHAT_1, 5u);

		break;
	default: return GetGenericMaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetStudentFemaleTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_HANDS_COWER:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_STUDENT_FEMALE_HANDS_COWER_1, 4u);
		break;
	case SOUND_PED_ROBBED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_STUDENT_FEMALE_ROBBED_1, 2u);
		break;
	case SOUND_PED_ATTACK:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_STUDENT_FEMALE_ATTACK_1, 4u);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_STUDENT_FEMALE_EVADE_1, 4u);
		break;
	case SOUND_PED_CAR_COLLISION:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_STUDENT_FEMALE_CAR_COLLISION_1, 4u);
		break;
	case SOUND_PED_CHAT_EVENT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_STUDENT_FEMALE_CHAT_EVENT_1, 2u);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_STUDENT_FEMALE_CHAT_1, 4u);

		break;
	default: return GetGenericFemaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetCasualMaleOldTalkSfx(int16 sound)
{
	return GetGenericMaleTalkSfx(sound);
}

uint32
cAudioManager::GetSpecialCharacterTalkSfx(int32 modelIndex, int32 sound)
{
	char *modelName = CModelInfo::GetModelInfo(modelIndex)->GetName();
	if(strcmp(modelName, "eight") == 0 || strcmp(modelName, "eight2") == 0) {
		return GetEightTalkSfx(sound);
	}
	if(strcmp(modelName, "frankie") == 0) { return GetFrankieTalkSfx(sound); }
	if(strcmp(modelName, "misty") == 0) { return GetMistyTalkSfx(sound); }
	if(strcmp(modelName, "ojg") == 0 || strcmp(modelName, "ojg_p") == 0) {
		return GetOJGTalkSfx(sound);
	}
	if(strcmp(modelName, "cat") == 0) { return GetCatatalinaTalkSfx(sound); }
	if(strcmp(modelName, "bomber") == 0) { return GetBomberTalkSfx(sound); }
	if(strcmp(modelName, "s_guard") == 0) { return GetSecurityGuardTalkSfx(sound); }
	if(strcmp(modelName, "chunky") == 0) { return GetChunkyTalkSfx(sound); }
	if(strcmp(modelName, "asuka") == 0) { return GetGenericFemaleTalkSfx(sound); }
	if(strcmp(modelName, "maria") == 0) { return GetGenericFemaleTalkSfx(sound); }

	return GetGenericMaleTalkSfx(sound);
}
uint32
cAudioManager::GetEightTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_HANDS_UP:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_EIGHT_HANDS_UP_1, 2u);
		break;
	case SOUND_PED_ROBBED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_EIGHT_ROBBED_1, 2u);
		break;
	case SOUND_PED_ATTACK:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_EIGHT_ATTACK_1, 6u);
		break;
	case SOUND_PED_EVADE: GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_EIGHT_EVADE_1, 7u); break;
	default: return GetGenericMaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetFrankieTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_HANDS_UP:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_FRANKIE_HANDS_UP_1, 4u);
		break;
	case SOUND_PED_ROBBED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_FRANKIE_ROBBED_1, 2u);
		break;
	case SOUND_PED_ATTACK:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_FRANKIE_ATTACK_1, 6u);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_FRANKIE_EVADE_1, 3u);
		break;
	default: return GetGenericMaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetMistyTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_HANDS_UP:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_MISTY_HANDS_UP_1, 5u);
		break;
	case SOUND_PED_ROBBED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_MISTY_ROBBED_1, 2u);
		break;
	case SOUND_PED_ATTACK:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_MISTY_ATTACK_1, 4u);
		break;
	case SOUND_PED_EVADE: GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_MISTY_EVADE_1, 5u); break;
	case SOUND_PED_TAXI_CALL:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_MISTY_THUMB_LIFT_1, 4u);
		break;
	default: return GetGenericFemaleTalkSfx(sound); break;
	}
	return sfx;
}

uint32
cAudioManager::GetOJGTalkSfx(int16 sound)
{
	return GetGenericMaleTalkSfx(sound);
}

uint32
cAudioManager::GetCatatalinaTalkSfx(int16 sound)
{
	return GetGenericFemaleTalkSfx(sound);
}

uint32
cAudioManager::GetBomberTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	if(sound != SOUND_PED_BOMBER) return GetGenericMaleTalkSfx(sound);

	GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BOMBER_THREAT_1, 7u);
	return sfx;
}

uint32
cAudioManager::GetSecurityGuardTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_HANDS_UP:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_SECURITY_GUARD_HANDS_UP_1, 2u);
		break;
	case SOUND_PED_HANDS_COWER: sfx = AUDIO_SAMPLE_PED_SECURITY_GUARD_HANDS_COWER_1; break;
	case SOUND_PED_CAR_JACKED:
	case SOUND_PED_CAR_COLLISION:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_SECURITY_GUARD_CAR_EVENT_1, 6u);
		break;
	case SOUND_PED_ATTACK:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_SECURITY_GUARD_ATTACK_1, 2u);
		break;
	case SOUND_PED_FLEE_RUN:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_SECURITY_GUARD_CAR_EVENT_1, 12u);
		break;
	default: return GetGenericMaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetChunkyTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	if(sound == SOUND_PED_DEATH) return AUDIO_SAMPLE_PED_CHUNKY_DEATH_1;

	if(sound != SOUND_PED_FLEE_RUN) return GetGenericMaleTalkSfx(sound);

	GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_CHUNKY_FLEE_RUN_1, 5u);
	return sfx;
}

uint32
cAudioManager::GetGenericMaleTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_DEATH: GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_MALE_DEATH_1, 8u); break;
	case SOUND_PED_BULLET_HIT:
	case SOUND_PED_DEFEND:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_INJURED_PED_MALE_OUCH_1, 15u);
		break;
	case SOUND_PED_BURNING:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_MALE_BURNING_1, 8u);
		break;
	case SOUND_PED_FLEE_SPRINT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_MALE_FLEE_SPRINT_1, 6u);
		break;
	default: return NO_SAMPLE;
	}
	return sfx;
}

uint32
cAudioManager::GetGenericFemaleTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_DEATH:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_FEMALE_DEATH_1, 10u);
		break;
	case SOUND_PED_BULLET_HIT:
	case SOUND_PED_DEFEND:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_INJURED_PED_FEMALE_OUCH_1, 11u);
		break;
	case SOUND_PED_BURNING:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_FEMALE_BURNING_1, 9u);
		break;
	case SOUND_PED_FLEE_SPRINT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_FEMALE_FLEE_SPRINT_1, 8u);
		break;
	default: return NO_SAMPLE;
	}
	return sfx;
}

void
cAudioManager::GenerateIntegerRandomNumberTable()
{
	for(int32 i = 0; i < 5; i++) { m_anRandomTable[i] = rand(); }
}

char *
cAudioManager::Get3DProviderName(uint8 id) const
{
	if(!m_bIsInitialised) return 0;
	if(id >= SampleManager.GetNum3DProvidersAvailable()) return 0;
	return SampleManager.Get3DProviderName(id);
}

uint8
cAudioManager::GetCDAudioDriveLetter() const
{
	if(m_bIsInitialised) return SampleManager.GetCDAudioDriveLetter();

	return 0;
}

int8
cAudioManager::GetCurrent3DProviderIndex() const
{
	if(m_bIsInitialised) return SampleManager.GetCurrent3DProviderIndex();

	return -1;
}

float
cAudioManager::GetCollisionLoopingRatio(uint32 a, uint32 b, float c) const
{
	return GetCollisionRatio(c, 0.0f, 0.02f, 0.02f);
}

float
cAudioManager::GetCollisionOneShotRatio(int32 a, float b) const
{
	float result;

	switch(a) {
	case SURFACE_DEFAULT:
	case SURFACE_TARMAC:
	case SURFACE_PAVEMENT:
	case SURFACE_STONE:
	case SURFACE_BOLLARD: result = GetCollisionRatio(b, 10.f, 60.f, 50.f); break;
	case SURFACE_GRASS:
	case SURFACE_LOOSE30: result = GetCollisionRatio(b, 0.f, 2.f, 2.f); break;
	case SURFACE_DIRT: result = GetCollisionRatio(b, 0.f, 2.f, 2.f); break;
	case SURFACE_DIRTTRACK: result = GetCollisionRatio(b, 0.f, 2.f, 2.f); break;
	case SURFACE_METAL6: result = GetCollisionRatio(b, 6.f, 50.f, 44.f); break;
	case SURFACE_GLASS: result = GetCollisionRatio(b, 0.1f, 10.f, 9.9f); break;
	case SURFACE_SCAFFOLD:
	case SURFACE_STEEL: result = GetCollisionRatio(b, 30.f, 130.f, 100.f); break;
	case SURFACE_METAL_DOOR: result = GetCollisionRatio(b, 20.f, 100.f, 80.f); break;
	case SURFACE_BILLBOARD: result = GetCollisionRatio(b, 0.f, 4.f, 4.f); break;
	case SURFACE_METAL_POLE:
	case SURFACE_GATE: result = GetCollisionRatio(b, 1.f, 10.f, 9.f); break;
	case SURFACE_STREET_LIGHT: result = GetCollisionRatio(b, 1.f, 10.f, 9.f); break;
	case SURFACE_METAL14: result = GetCollisionRatio(b, 1.f, 15.f, 14.f); break;
	case SURFACE_METAL15: result = GetCollisionRatio(b, 8.f, 50.f, 42.f); break;
	case SURFACE_METAL_FENCE: result = GetCollisionRatio(b, 0.1f, 10.f, 9.9f); break;
	case SURFACE_FLESH: result = GetCollisionRatio(b, 0.f, 20.f, 20.f); break;
	case SURFACE_SAND: result = GetCollisionRatio(b, 0.f, 10.f, 10.f); break;
	case SURFACE_PUDDLE: result = GetCollisionRatio(b, 0.f, 10.f, 10.f); break;
	case SURFACE_WOOD: result = GetCollisionRatio(b, 1.f, 4.f, 3.f); break;
	case SURFACE_WOOD_BOX: result = GetCollisionRatio(b, 0.1f, 5.f, 4.9f); break;
	case SURFACE_WOOD_PLANK: result = GetCollisionRatio(b, 0.1f, 40.f, 39.9f); break;
	case SURFACE_TIRE:
	case SURFACE_RUBBER29: result = GetCollisionRatio(b, 0.f, 10.f, 10.f); break;
	case SURFACE_HARD24: result = GetCollisionRatio(b, 0.1f, 4.f, 3.9f); break;
	case SURFACE_HEDGE: result = GetCollisionRatio(b, 0.f, 0.5f, 0.5f); break;
	case SURFACE_METAL27: result = GetCollisionRatio(b, 4.f, 40.f, 36.f); break;
	case SURFACE_METAL28: result = GetCollisionRatio(b, 0.f, 5.f, 5.f); break;
	default: result = 0.f; break;
	}

	return result;
}

float
cAudioManager::GetCollisionRatio(float a, float b, float c, float d) const
{
	float e;
	e = a;
	if(a <= b) return 0.0f;
	if(c <= a) e = c;
	return (e - b) / d;
}

float
cAudioManager::GetDistanceSquared(CVector *v) const
{
	const CVector &c = TheCamera.GetPosition();
	return sq(v->x - c.x) + sq(v->y - c.y) + sq((v->z - c.z) * 0.2f);
}

int32
cAudioManager::GetJumboTaxiFreq() const
{
	return (60.833f * m_sQueueSample.m_fDistance) + 22050;
}

bool
cAudioManager::GetMissionAudioLoadingStatus() const
{
	if(m_bIsInitialised) return m_sMissionAudio.m_bLoadingStatus;

	return true;
}

char
cAudioManager::GetMissionScriptPoliceAudioPlayingStatus() const
{
	return g_nMissionAudioPlayingStatus;
}

uint8
cAudioManager::GetNum3DProvidersAvailable() const
{
	if(m_bIsInitialised) return SampleManager.GetNum3DProvidersAvailable();
	return 0;
}

int32
cAudioManager::GetPedCommentSfx(CPed *ped, int32 sound)
{
	if(ped->IsPlayer()) return GetPlayerTalkSfx(sound);

	switch(ped->m_modelIndex) {
	case MI_COP: return GetCopTalkSfx(sound);
	case MI_SWAT: return GetSwatTalkSfx(sound);
	case MI_FBI: return GetFBITalkSfx(sound);
	case MI_ARMY: return GetArmyTalkSfx(sound);
	case MI_MEDIC: return GetMedicTalkSfx(sound);
	case MI_FIREMAN: return GetFiremanTalkSfx(sound);
	case MI_MALE01: return GetNormalMaleTalkSfx(sound);
	case MI_TAXI_D: return GetTaxiDriverTalkSfx(sound);
	case MI_PIMP: return GetPimpTalkSfx(sound);
	case MI_GANG01:
	case MI_GANG02: return GetMafiaTalkSfx(sound);
	case MI_GANG03:
	case MI_GANG04: return GetTriadTalkSfx(sound);
	case MI_GANG05:
	case MI_GANG06: return GetDiabloTalkSfx(sound);
	case MI_GANG07:
	case MI_GANG08: return GetYakuzaTalkSfx(sound);
	case MI_GANG09:
	case MI_GANG10: return GetYardieTalkSfx(sound);
	case MI_GANG11:
	case MI_GANG12: return GetColumbianTalkSfx(sound);
	case MI_GANG13:
	case MI_GANG14: return GetHoodTalkSfx(sound);
	case MI_CRIMINAL01: return GetBlackCriminalTalkSfx(sound);
	case MI_CRIMINAL02: return GetWhiteCriminalTalkSfx(sound);
	case MI_SPECIAL01:
	case MI_SPECIAL02:
	case MI_SPECIAL03:
	case MI_SPECIAL04: return GetSpecialCharacterTalkSfx(ped->m_modelIndex, sound);
	case MI_MALE02: return GetMaleNo2TalkSfx(sound);
	case MI_MALE03:
	case MI_P_MAN1:
	case MI_P_MAN2: return GetBlackProjectMaleTalkSfx(sound, ped->m_modelIndex);
	case MI_FATMALE01: return GetWhiteFatMaleTalkSfx(sound);
	case MI_FATMALE02: return GetBlackFatMaleTalkSfx(sound);
	case MI_FEMALE01: return GetBlackCasualFemaleTalkSfx(sound);
	case MI_FEMALE02:
	case MI_CAS_WOM: return GetWhiteCasualFemaleTalkSfx(sound);
	case MI_FEMALE03: return GetFemaleNo3TalkSfx(sound);
	case MI_FATFEMALE01: return GetBlackFatFemaleTalkSfx(sound);
	case MI_FATFEMALE02: return GetWhiteFatFemaleTalkSfx(sound);
	case MI_PROSTITUTE: return GetBlackFemaleProstituteTalkSfx(sound);
	case MI_PROSTITUTE2: return GetWhiteFemaleProstituteTalkSfx(sound);
	case MI_P_WOM1: return GetBlackProjectFemaleOldTalkSfx(sound);
	case MI_P_WOM2: return GetBlackProjectFemaleYoungTalkSfx(sound);
	case MI_CT_MAN1: return GetChinatownMaleOldTalkSfx(sound);
	case MI_CT_MAN2: return GetChinatownMaleYoungTalkSfx(sound);
	case MI_CT_WOM1: return GetChinatownFemaleOldTalkSfx(sound);
	case MI_CT_WOM2: return GetChinatownFemaleYoungTalkSfx(sound);
	case MI_LI_MAN1:
	case MI_LI_MAN2: return GetLittleItalyMaleTalkSfx(sound);
	case MI_LI_WOM1: return GetLittleItalyFemaleOldTalkSfx(sound);
	case MI_LI_WOM2: return GetLittleItalyFemaleYoungTalkSfx(sound);
	case MI_DOCKER1: return GetWhiteDockerMaleTalkSfx(sound);
	case MI_DOCKER2: return GetBlackDockerMaleTalkSfx(sound);
	case MI_SCUM_MAN: return GetScumMaleTalkSfx(sound);
	case MI_SCUM_WOM: return GetScumFemaleTalkSfx(sound);
	case MI_WORKER1: return GetWhiteWorkerMaleTalkSfx(sound);
	case MI_WORKER2: return GetBlackWorkerMaleTalkSfx(sound);
	case MI_B_MAN1:
	case MI_B_MAN3: return GetBusinessMaleYoungTalkSfx(sound, ped->m_modelIndex);
	case MI_B_MAN2: return GetBusinessMaleOldTalkSfx(sound);
	case MI_B_WOM1:
	case MI_B_WOM2: return GetWhiteBusinessFemaleTalkSfx(sound, ped->m_modelIndex);
	case MI_B_WOM3: return GetBlackBusinessFemaleTalkSfx(sound);
	case MI_MOD_MAN: return GetSupermodelMaleTalkSfx(sound);
	case MI_MOD_WOM: return GetSupermodelFemaleTalkSfx(sound);
	case MI_ST_MAN: return GetStewardMaleTalkSfx(sound);
	case MI_ST_WOM: return GetStewardFemaleTalkSfx(sound);
	case MI_FAN_MAN1:
	case MI_FAN_MAN2: return GetFanMaleTalkSfx(sound, ped->m_modelIndex);
	case MI_FAN_WOM: return GetFanFemaleTalkSfx(sound);
	case MI_HOS_MAN: return GetHospitalMaleTalkSfx(sound);
	case MI_HOS_WOM: return GetHospitalFemaleTalkSfx(sound);
	case MI_CONST1: return GetWhiteConstructionWorkerTalkSfx(sound);
	case MI_CONST2: return GetBlackConstructionWorkerTalkSfx(sound);
	case MI_SHOPPER1:
	case MI_SHOPPER2:
	case MI_SHOPPER3: return GetShopperFemaleTalkSfx(sound, ped->m_modelIndex);
	case MI_STUD_MAN: return GetStudentMaleTalkSfx(sound);
	case MI_STUD_WOM: return GetStudentFemaleTalkSfx(sound);
	case MI_CAS_MAN: return GetCasualMaleOldTalkSfx(sound);
	default: return GetGenericMaleTalkSfx(sound);
	}
}

void
cAudioManager::GetPhrase(uint32 *phrase, uint32 *prevPhrase, uint32 sample, uint32 maxOffset) const
{
	*phrase = sample + m_anRandomTable[m_sQueueSample.m_nEntityIndex & 3] % maxOffset;

	// check if the same sfx like last time, if yes, then try use next one,
	// if exceeded range, then choose first available sample
	if(*phrase == *prevPhrase && ++*phrase >= sample + maxOffset) *phrase = sample;
	*prevPhrase = *phrase;
}

float
cAudioManager::GetVehicleDriveWheelSkidValue(uint8 wheel, CAutomobile *automobile,
                                             cTransmission *transmission, float velocityChange)
{
	int wheelState;
	float relativeVelChange;
	float gasPedalAudio = automobile->m_fGasPedalAudio;
	float modificator;
	float velChange;
	float relativeVel;

	wheelState = automobile->m_aWheelState[wheel];
	if(wheelState == 1 && gasPedalAudio > 0.4f) {
		relativeVelChange = (gasPedalAudio + -0.4f) * 1.25f;

	} else if(wheelState == 2) {
		relativeVelChange = min(1.0f, Abs(velocityChange) / transmission->fMaxVelocity);
	} else if(wheelState == 3) {
		modificator = 0.4f;
		relativeVelChange = gasPedalAudio;
		if(relativeVelChange > 0.4f) {
			relativeVelChange = relativeVelChange - 0.4f;
			modificator = 1.6667f;
		}
		velChange = Abs(velocityChange);
		if(relativeVelChange > 0.4f) relativeVelChange = relativeVelChange * modificator;
		if(velChange > 0.04f) {
			relativeVel = min(1.0f, velChange / transmission->fMaxVelocity);
		} else {
			relativeVel = 0.0f;
		}
		if(relativeVel >= relativeVelChange) relativeVelChange = relativeVel;
	} else {
		relativeVelChange = 0.0f;
	}

	return max(relativeVelChange, min(1.0f, Abs(automobile->m_vecTurnSpeed.z) * 20.0f));
}

float
cAudioManager::GetVehicleNonDriveWheelSkidValue(uint8 wheel, CAutomobile *automobile,
                                                cTransmission *transmission, float velocityChange)
{
	float relativeVelChange;

	if(automobile->m_aWheelState[wheel] == 2) {
		relativeVelChange = min(1.0f, Abs(velocityChange) / transmission->fMaxVelocity);
	} else {
		relativeVelChange = 0.0;
	}

	return max(relativeVelChange, min(1.0f, Abs(automobile->m_vecTurnSpeed.z) * 20.0f));
}

bool
cAudioManager::HasAirBrakes(int32 model) const
{
	return model == LINERUN || model == FIRETRUK || model == TRASH || model == BUS ||
	       model == COACH;
}

void
cAudioManager::Initialise()
{
	if(!m_bIsInitialised) {
		PreInitialiseGameSpecificSetup();
		m_bIsInitialised = SampleManager.Initialise();
		if(m_bIsInitialised) {
			m_bActiveSamples = SampleManager.GetMaximumSupportedChannels();
			if(m_bActiveSamples <= 1u) {
				Terminate();
			} else {
				--m_bActiveSamples;
				PostInitialiseGameSpecificSetup();
				InitialisePoliceRadioZones();
				InitialisePoliceRadio();
				MusicManager.Initialise();
			}
		}
	}
}

void
cAudioManager::InitialisePoliceRadio()
{
	policeChannelTimer = 0;
	policeChannelTimerSeconds = 0;
	policeChannelCounterSeconds = 0;
	for(int32 i = 0; i < 10; i++) { crimes[i].type = 0; }

	SampleManager.SetChannelReverbFlag(28, 0);
	gSpecialSuspectLastSeenReport = 0;
	for(int32 i = 0; i < 18; i++) { gMinTimeToNextReport[i] = m_nTimeOfRecentCrime; }
}

struct tPoliceRadioZone
{
  char m_aName[8];
  uint32 m_nSampleIndex;
  int32 field_12;
};

tPoliceRadioZone* ZoneSfx = (tPoliceRadioZone*)0x880240;
char* SubZo2Label = (char*)0x6E9918;
char* SubZo3Label = (char*)0x6E9870;

void
cAudioManager::InitialisePoliceRadioZones()
{
	for(int32 i = 0; i < 36; i++) { ZoneSfx[i].m_aName[0] = 0; }

	strcpy((char *)ZoneSfx, "HOSPI_2");
	ZoneSfx[0].m_nSampleIndex = AUDIO_SAMPLE_POLICE_SCANNER_ZONE_ROCKFORD;
	strcpy((char *)&ZoneSfx[1], "CONSTRU");
	ZoneSfx[1].m_nSampleIndex = AUDIO_SAMPLE_POLICE_SCANNER_ZONE_FORT_STAUNTON;
	strcpy((char *)&ZoneSfx[2], "STADIUM");
	ZoneSfx[2].m_nSampleIndex = AUDIO_SAMPLE_POLICE_SCANNER_ZONE_ASPATRIA;
	strcpy((char *)&ZoneSfx[3], "YAKUSA");
	ZoneSfx[3].m_nSampleIndex = AUDIO_SAMPLE_POLICE_SCANNER_ZONE_TORRINGTON;
	strcpy((char *)&ZoneSfx[4], "SHOPING");
	ZoneSfx[4].m_nSampleIndex = AUDIO_SAMPLE_POLICE_SCANNER_ZONE_BEDFORD_POINT;
	strcpy((char *)&ZoneSfx[5], "COM_EAS");
	ZoneSfx[5].m_nSampleIndex = AUDIO_SAMPLE_POLICE_SCANNER_ZONE_NEWPORT;
	strcpy((char *)&ZoneSfx[6], "PARK");
	ZoneSfx[6].m_nSampleIndex = AUDIO_SAMPLE_POLICE_SCANNER_ZONE_BELLEVILLE;
	strcpy((char *)&ZoneSfx[7], "UNIVERS");
	ZoneSfx[7].m_nSampleIndex = AUDIO_SAMPLE_POLICE_SCANNER_ZONE_LIBERTY_CAMPUS;
	strcpy((char *)&ZoneSfx[8], "BIG_DAM");
	ZoneSfx[8].m_nSampleIndex = AUDIO_SAMPLE_POLICE_SCANNER_ZONE_COCHRANE;
	strcpy((char *)&ZoneSfx[9], "SUB_IND");
	ZoneSfx[9].m_nSampleIndex = AUDIO_SAMPLE_POLICE_SCANNER_ZONE_PIKE_CREEK;
	strcpy((char *)&ZoneSfx[10], "SWANKS");
	ZoneSfx[10].m_nSampleIndex = AUDIO_SAMPLE_POLICE_SCANNER_ZONE_CEDAR_GROVE;
	strcpy((char *)&ZoneSfx[11], "PROJECT");
	ZoneSfx[11].m_nSampleIndex = AUDIO_SAMPLE_POLICE_SCANNER_ZONE_WICHITA_GARDEN;
	strcpy((char *)&ZoneSfx[12], "AIRPORT");
	ZoneSfx[12].m_nSampleIndex = AUDIO_SAMPLE_POLICE_SCANNER_ZONE_FRANCIS_INTNL;
	strcpy((char *)&ZoneSfx[13], "PORT_W");
	ZoneSfx[13].m_nSampleIndex = AUDIO_SAMPLE_POLICE_SCANNER_ZONE_CALLAHAN_POINT;
	strcpy((char *)&ZoneSfx[14], "PORT_S");
	ZoneSfx[14].m_nSampleIndex = AUDIO_SAMPLE_POLICE_SCANNER_ZONE_ATLANTIC_QUAYS;
	strcpy((char *)&ZoneSfx[15], "PORT_E");
	ZoneSfx[15].m_nSampleIndex = AUDIO_SAMPLE_POLICE_SCANNER_ZONE_PORTLAND_HARBOUR;
	strcpy((char *)&ZoneSfx[16], "PORT_I");
	ZoneSfx[16].m_nSampleIndex = AUDIO_SAMPLE_POLICE_SCANNER_ZONE_TRENTON;
	strcpy((char *)&ZoneSfx[17], "CHINA");
	ZoneSfx[17].m_nSampleIndex = AUDIO_SAMPLE_POLICE_SCANNER_ZONE_CHINATOWN;
	strcpy((char *)&ZoneSfx[18], "REDLIGH");
	ZoneSfx[18].m_nSampleIndex = AUDIO_SAMPLE_POLICE_SCANNER_ZONE_REDLIGHT;
	strcpy((char *)&ZoneSfx[19], "TOWERS");
	ZoneSfx[19].m_nSampleIndex = AUDIO_SAMPLE_POLICE_SCANNER_ZONE_HEPBURN_HEIGHTS;
	strcpy((char *)&ZoneSfx[20], "LITTLEI");
	ZoneSfx[20].m_nSampleIndex = AUDIO_SAMPLE_POLICE_SCANNER_ZONE_ST_MARKS;
	strcpy((char *)&ZoneSfx[21], "HARWOOD");
	ZoneSfx[21].m_nSampleIndex = AUDIO_SAMPLE_POLICE_SCANNER_ZONE_HARWOOD;
	strcpy((char *)&ZoneSfx[22], "EASTBAY");
	ZoneSfx[22].m_nSampleIndex = AUDIO_SAMPLE_POLICE_SCANNER_ZONE_PORTLAND_BEACH;
	strcpy((char *)&ZoneSfx[23], "S_VIEW");
	ZoneSfx[23].m_nSampleIndex = AUDIO_SAMPLE_POLICE_SCANNER_ZONE_PORTLAND_VIEW;
	strcpy((char *)&ZoneSfx[24], "CITYZON");
	ZoneSfx[24].m_nSampleIndex = AUDIO_SAMPLE_POLICE_SCANNER_ZONE_LIBERTY_CITY;
	strcpy((char *)&ZoneSfx[25], "IND_ZON");
	ZoneSfx[25].m_nSampleIndex = AUDIO_SAMPLE_POLICE_SCANNER_ZONE_PORTLAND;
	strcpy((char *)&ZoneSfx[26], "COM_ZON");
	ZoneSfx[26].m_nSampleIndex = AUDIO_SAMPLE_POLICE_SCANNER_ZONE_STAUNTON;
	strcpy((char *)&ZoneSfx[27], "SUB_ZON");
	ZoneSfx[27].m_nSampleIndex = AUDIO_SAMPLE_POLICE_SCANNER_ZONE_SHORESIDE;
	strcpy((char *)&ZoneSfx[28], "SUB_ZO2");
	ZoneSfx[28].m_nSampleIndex = AUDIO_SAMPLE_POLICE_SCANNER_ZONE_SHORESIDE;
	strcpy((char *)&ZoneSfx[29], "SUB_ZO3");
	ZoneSfx[29].m_nSampleIndex = AUDIO_SAMPLE_POLICE_SCANNER_ZONE_SHORESIDE;
	strcpy((char *)&ZoneSfx[30], "A");
	ZoneSfx[30].m_nSampleIndex = AUDIO_SAMPLE_POLICE_SCANNER_ZONE_ROCKFORD;
	strcpy((char *)&ZoneSfx[31], "A");
	ZoneSfx[31].m_nSampleIndex = AUDIO_SAMPLE_POLICE_SCANNER_ZONE_ROCKFORD;
	strcpy((char *)&ZoneSfx[32], "A");
	ZoneSfx[32].m_nSampleIndex = AUDIO_SAMPLE_POLICE_SCANNER_ZONE_ROCKFORD;
	strcpy((char *)&ZoneSfx[33], "A");
	ZoneSfx[33].m_nSampleIndex = AUDIO_SAMPLE_POLICE_SCANNER_ZONE_ROCKFORD;
	strcpy((char *)&ZoneSfx[34], "A");
	ZoneSfx[34].m_nSampleIndex = AUDIO_SAMPLE_POLICE_SCANNER_ZONE_ROCKFORD;
	strcpy(SubZo2Label, "SUB_ZO2");
	strcpy(SubZo3Label, "SUB_ZO3");
}

void
cAudioManager::InterrogateAudioEntities()
{
	for(int32 i = 0; i < m_nAudioEntitiesTotal; i++) {
		ProcessEntity(m_anAudioEntityIndices[i]);
		m_asAudioEntities[m_anAudioEntityIndices[i]].m_Loops = 0;
	}
}

bool
cAudioManager::IsAudioInitialised() const
{
	return m_bIsInitialised;
}

bool
cAudioManager::IsMissionAudioSampleFinished()
{
	if(m_bIsInitialised) return m_sMissionAudio.m_bPlayStatus == 2;

	static int32 cPretendFrame = 1;

	return (cPretendFrame++ & 63) == 0;
}

bool
cAudioManager::IsMP3RadioChannelAvailable() const
{
	if(m_bIsInitialised) return SampleManager.IsMP3RadioChannelAvailable();

	return 0;
}

bool
cAudioManager::MissionScriptAudioUsesPoliceChannel(int32 soundMission) const
{
	switch(soundMission) {
	case STREAMED_SOUND_MISSION_J6_D:
	case STREAMED_SOUND_MISSION_T4_A:
	case STREAMED_SOUND_MISSION_S1_H:
	case STREAMED_SOUND_MISSION_S3_B:
	case STREAMED_SOUND_MISSION_EL3_A:
	case STREAMED_SOUND_MISSION_A3_A:
	case STREAMED_SOUND_MISSION_A5_A:
	case STREAMED_SOUND_MISSION_K1_A:
	case STREAMED_SOUND_MISSION_R1_A:
	case STREAMED_SOUND_MISSION_R5_A:
	case STREAMED_SOUND_MISSION_LO2_A:
	case STREAMED_SOUND_MISSION_LO6_A: return true;
	default: return false;
	}
}

void
cAudioManager::PlayLoadedMissionAudio()
{
	if(m_bIsInitialised && m_sMissionAudio.m_nSampleIndex != NO_SAMPLE &&
	   m_sMissionAudio.m_bLoadingStatus == 1 && !m_sMissionAudio.m_bPlayStatus) {
		m_sMissionAudio.m_bIsPlayed = true;
	}
}

void
cAudioManager::PlayOneShot(int32 index, int16 sound, float vol)
{
	if(m_bIsInitialised) {
		if (index >= 0 && index < totalAudioEntitiesSlots) {
			if (m_asAudioEntities[index].m_bIsUsed) {
				if (sound < SOUND_TOTAL_SOUNDS) {
					if (m_asAudioEntities[index].m_nType ==
						AUDIOTYPE_SCRIPTOBJECT) {
						if (m_nScriptObjectEntityTotal < 40) {
							m_asAudioEntities[index].m_awAudioEvent[0] =
								sound;
							m_asAudioEntities[index].m_Loops = 1;
							m_anScriptObjectEntityIndices
								[m_nScriptObjectEntityTotal++] = index;
						}
					}
					else {
						int32 i = 0;
						while (1) {
							if (i >= m_asAudioEntities[index].m_Loops) {
								if (m_asAudioEntities[index]
									.m_Loops < 4) {
									m_asAudioEntities[index]
										.m_awAudioEvent[i] =
										sound;
									m_asAudioEntities[index]
										.m_afVolume[i] = vol;
									++m_asAudioEntities[index]
										.m_Loops;
								}
								return;
							}
							if (panTable[m_asAudioEntities[index]
								.m_awAudioEvent[i]] >
								panTable[sound])
								break;
							++i;
						}
						if (i < 3) {
							memmove(
								&m_asAudioEntities[index]
								.m_awAudioEvent[i + 1],
								&m_asAudioEntities[index]
								.m_awAudioEvent[i],
								3 - i);
							memmove(&m_asAudioEntities[index]
								.m_afVolume[i + 1],
								&m_asAudioEntities[index]
								.m_afVolume[i],
								3 - i);
						}
						m_asAudioEntities[index].m_awAudioEvent[i] =
							sound;
						m_asAudioEntities[index].m_afVolume[i] = vol;
						if (m_asAudioEntities[index].m_Loops < 4)
							++m_asAudioEntities[index].m_Loops;
					}
				}
			}
		}
	}
}

void
cAudioManager::PlaySuspectLastSeen(float x, float y, float z)
{
		cAudioManager* v4; // ebx
		int16 audioZone; // ax
		unsigned __int8 i; // dl
		CZone* zone; // edi
		int v8; // ecx
		double v10; // st5
		double v11; // st7
		double v12; // st4
		double v13; // st6
		double v14; // st5
		double v15; // st4
		double v16; // st7
		double v17; // st6
		int v18; // eax
		int32 sample; // [esp+8h] [ebp-30h]
		char v20; // [esp+Ch] [ebp-2Ch]
		CVector vec = {x, y, z};

		v4 = this;
		v20 = 0;
		if (this->m_bIsInitialised)
		{
			if (MusicManager.m_nMusicMode != 2 && 60 - policeChannelTimer > 9u)
			{

				audioZone = CTheZones::FindAudioZone(&vec);
				if (audioZone >= 0 && audioZone < 36)
				{
					i = 0;
					zone = &CTheZones::ZoneArray[CTheZones::AudioZoneArray[audioZone]];
					v8 = 0;
					while (strcmp(zone->name, ZoneSfx[v8].m_aName) != 0)
					{
						++i;
						++v8;
						if (i >= 36u)
							goto LABEL_11;
					}
					sample = ZoneSfx[i].m_nSampleIndex;
				LABEL_11:
					if (i < 36u)
					{
						if (v4->policeChannelTimer != 60)
						{
							v4->crimesSamples[v4->policeChannelTimerSeconds] = v4->m_anRandomTable[4] % 3u
								+ AUDIO_SAMPLE_POLICE_SCANNER_SMALL_CRACKLE_1;
							++v4->policeChannelTimer;
							v4->policeChannelTimerSeconds = (v4->policeChannelTimerSeconds + 1) % 60;
						}
						if (v4->policeChannelTimer != 60)
						{
							v4->crimesSamples[v4->policeChannelTimerSeconds] = AUDIO_SAMPLE_POLICE_SCANNER_SUSPECT;
							++v4->policeChannelTimer;
							v4->policeChannelTimerSeconds = (v4->policeChannelTimerSeconds + 1) % 60;
						}
						if (v4->policeChannelTimer != 60)
						{
							v4->crimesSamples[v4->policeChannelTimerSeconds] = AUDIO_SAMPLE_POLICE_SCANNER_LAST_SEEN;
							++v4->policeChannelTimer;
							v4->policeChannelTimerSeconds = (v4->policeChannelTimerSeconds + 1) % 60;
						}
						if (v4->policeChannelTimer != 60)
						{
							v4->crimesSamples[v4->policeChannelTimerSeconds] = AUDIO_SAMPLE_POLICE_SCANNER_IN;
							++v4->policeChannelTimer;
							v4->policeChannelTimerSeconds = (v4->policeChannelTimerSeconds + 1) % 60;
						}
						if (sample == AUDIO_SAMPLE_POLICE_SCANNER_ZONE_SHORESIDE
							&& (strcmp(zone->name, SubZo2Label) == 0
								|| strcmp(zone->name, SubZo3Label) == 0))
						{
							if (v4->policeChannelTimer != 60)
							{
								v4->crimesSamples[v4->policeChannelTimerSeconds] = AUDIO_SAMPLE_POLICE_SCANNER_NORTH;
								++v4->policeChannelTimer;
								v4->policeChannelTimerSeconds = (v4->policeChannelTimerSeconds + 1) % 60;
							}
							if (v4->policeChannelTimer == 60) {
								if (v4->policeChannelTimer != 60)
								{
									v4->crimesSamples[v4->policeChannelTimerSeconds] = sample;
									++v4->policeChannelTimer;
									v4->policeChannelTimerSeconds = (v4->policeChannelTimerSeconds + 1) % 60;
								}
								v18 = -1431655765 * v4->m_anRandomTable[2];
								if (v4->policeChannelTimer != 60)
								{
									v4->crimesSamples[v4->policeChannelTimerSeconds] = v4->m_anRandomTable[2] % 3u
										+ AUDIO_SAMPLE_POLICE_SCANNER_SMALL_CRACKLE_1;
									++v4->policeChannelTimer;
									v4->policeChannelTimerSeconds = (v4->policeChannelTimerSeconds + 1) % 60;
								}
								if (v4->policeChannelTimer != 60)
								{
									v4->crimesSamples[v4->policeChannelTimerSeconds] = TOTAL_AUDIO_SAMPLES;
									++v4->policeChannelTimer;
									v4->policeChannelTimerSeconds = (v4->policeChannelTimerSeconds + 1) % 60;
								}
								gSpecialSuspectLastSeenReport = 1;
								return;
							}
							v4->crimesSamples[v4->policeChannelTimerSeconds] = AUDIO_SAMPLE_POLICE_SCANNER_EAST;
							goto LABEL_45;
						}
						v10 = zone->minx;
						v11 = zone->maxx - v10;
						v12 = zone->miny;
						v13 = zone->maxy - v12;
						v14 = 0.5f * v11 + v10;
						v15 = 0.5f * v13 + v12;
						v16 = 0.25f * v11;
						v17 = 0.25f * v13;
						if (v15 + v17 < vec.y)
						{
							if (v15 - v17 <= vec.y)
								goto LABEL_36;
							if (v4->policeChannelTimer != 60)
							{
								v4->crimesSamples[v4->policeChannelTimerSeconds] = AUDIO_SAMPLE_POLICE_SCANNER_SOUTH;
								++v4->policeChannelTimer;
								v4->policeChannelTimerSeconds = (v4->policeChannelTimerSeconds + 1) % 60;
							}
						}
						else if (v4->policeChannelTimer != 60)
						{
							v4->crimesSamples[v4->policeChannelTimerSeconds] = AUDIO_SAMPLE_POLICE_SCANNER_NORTH;
							++v4->policeChannelTimer;
							v4->policeChannelTimerSeconds = (v4->policeChannelTimerSeconds + 1) % 60;
						}
						v20 = 1;
					LABEL_36:
						if (v14 + v16 < vec.x)
						{
							if (v14 - v16 <= vec.x)
							{
								if (v20 || v4->policeChannelTimer == 60) {
									if (v4->policeChannelTimer != 60)
									{
										v4->crimesSamples[v4->policeChannelTimerSeconds] = sample;
										++v4->policeChannelTimer;
										v4->policeChannelTimerSeconds = (v4->policeChannelTimerSeconds + 1) % 60;
									}
									v18 = -1431655765 * v4->m_anRandomTable[2];
									if (v4->policeChannelTimer != 60)
									{
										v4->crimesSamples[v4->policeChannelTimerSeconds] = v4->m_anRandomTable[2] % 3u
											+ AUDIO_SAMPLE_POLICE_SCANNER_SMALL_CRACKLE_1;
										++v4->policeChannelTimer;
										v4->policeChannelTimerSeconds = (v4->policeChannelTimerSeconds + 1) % 60;
									}
									if (v4->policeChannelTimer != 60)
									{
										v4->crimesSamples[v4->policeChannelTimerSeconds] = TOTAL_AUDIO_SAMPLES;
										++v4->policeChannelTimer;
										v4->policeChannelTimerSeconds = (v4->policeChannelTimerSeconds + 1) % 60;
									}
									gSpecialSuspectLastSeenReport = 1;
									return;
								}
								v4->crimesSamples[v4->policeChannelTimerSeconds] = AUDIO_SAMPLE_POLICE_SCANNER_CENTRAL;
								goto LABEL_45;
							}
							if (v4->policeChannelTimer != 60)
							{
								v4->crimesSamples[v4->policeChannelTimerSeconds] = AUDIO_SAMPLE_POLICE_SCANNER_WEST;
								goto LABEL_45;
							}
						}
						else if (v4->policeChannelTimer != 60)
						{
							v4->crimesSamples[v4->policeChannelTimerSeconds] = AUDIO_SAMPLE_POLICE_SCANNER_EAST;
						LABEL_45:
							++v4->policeChannelTimer;
							v4->policeChannelTimerSeconds = (v4->policeChannelTimerSeconds + 1) % 60;
						}
						if (v4->policeChannelTimer != 60)
						{
							v4->crimesSamples[v4->policeChannelTimerSeconds] = sample;
							++v4->policeChannelTimer;
							v4->policeChannelTimerSeconds = (v4->policeChannelTimerSeconds + 1) % 60;
						}
						v18 = -1431655765 * v4->m_anRandomTable[2];
						if (v4->policeChannelTimer != 60)
						{
							v4->crimesSamples[v4->policeChannelTimerSeconds] = v4->m_anRandomTable[2] % 3u
								+ AUDIO_SAMPLE_POLICE_SCANNER_SMALL_CRACKLE_1;
							++v4->policeChannelTimer;
							v4->policeChannelTimerSeconds = (v4->policeChannelTimerSeconds + 1) % 60;
						}
						if (v4->policeChannelTimer != 60)
						{
							v4->crimesSamples[v4->policeChannelTimerSeconds] = TOTAL_AUDIO_SAMPLES;
							++v4->policeChannelTimer;
							v4->policeChannelTimerSeconds = (v4->policeChannelTimerSeconds + 1) % 60;
						}
						gSpecialSuspectLastSeenReport = 1;
						return;
					}
				}
			}
		}
	
}

void
cAudioManager::PlayerJustGotInCar() const
{
	if(m_bIsInitialised) { bPlayerJustEnteredCar = true; }
}

void
cAudioManager::PlayerJustLeftCar(void) const
{
	// UNUSED: This is a perfectly empty function.
}

void
cAudioManager::PostInitialiseGameSpecificSetup()
{
	m_nFireAudioEntity =
	    CreateEntity(AUDIOTYPE_FIRE,
	                 (CPhysical *)0x8F31D0); // last is addr of firemanager @todo change
	if(m_nFireAudioEntity >= 0) SetEntityStatus(m_nFireAudioEntity, 1);

	m_nCollisionEntity = CreateEntity(AUDIOTYPE_COLLISION, (CPhysical *)1);
	if(m_nCollisionEntity >= 0) SetEntityStatus(m_nCollisionEntity, 1);

	m_nFrontEndEntity = CreateEntity(AUDIOTYPE_FRONTEND, (CPhysical *)1);
	if(m_nFrontEndEntity >= 0) SetEntityStatus(m_nFrontEndEntity, 1);

	m_nProjectileEntity = CreateEntity(AUDIOTYPE_PROJECTILE, (CPhysical *)1);
	if(m_nProjectileEntity >= 0) SetEntityStatus(m_nProjectileEntity, 1);

	m_nWaterCannonEntity = CreateEntity(AUDIOTYPE_WATERCANNON, (CPhysical *)1);
	if(m_nWaterCannonEntity >= 0) SetEntityStatus(m_nWaterCannonEntity, 1);

	m_nPoliceChannelEntity = CreateEntity(AUDIOTYPE_POLICERADIO, (CPhysical *)1);
	if(m_nPoliceChannelEntity >= 0) SetEntityStatus(m_nPoliceChannelEntity, 1);

	m_nBridgeEntity = CreateEntity(AUDIOTYPE_BRIDGE, (CPhysical *)1);
	if(m_nBridgeEntity >= 0) SetEntityStatus(m_nBridgeEntity, 1);

	m_sMissionAudio.m_nSampleIndex = NO_SAMPLE;
	m_sMissionAudio.m_bLoadingStatus = 0;
	m_sMissionAudio.m_bPlayStatus = 0;
	m_sMissionAudio.field_22 = 0;
	m_sMissionAudio.m_bIsPlayed = 0;
	m_sMissionAudio.field_12 = 1;
	m_sMissionAudio.field_24 = 0;
	ResetAudioLogicTimers((int32)CTimer::GetTimeInMilliseconds);
}

void
cAudioManager::PostTerminateGameSpecificShutdown()
{
	;
}

void
cAudioManager::PreInitialiseGameSpecificSetup() const
{
	BankStartOffset[0] = AUDIO_SAMPLE_VEHICLE_HORN_0;
	BankStartOffset[1] = AUDIO_SAMPLE_POLICE_COP_1_ARREST_1;
}

WRAPPER
void
cAudioManager::PreloadMissionAudio(char *)
{
	EAXJMP(0x579550);
}

void
cAudioManager::PreTerminateGameSpecificShutdown()
{
	if(m_nBridgeEntity >= 0) {
		DestroyEntity(m_nBridgeEntity);
		m_nBridgeEntity = -5;
	}
	if(m_nPoliceChannelEntity >= 0) {
		DestroyEntity(m_nPoliceChannelEntity);
		m_nPoliceChannelEntity = -5;
	}
	if(m_nWaterCannonEntity >= 0) {
		DestroyEntity(m_nWaterCannonEntity);
		m_nWaterCannonEntity = -5;
	}
	if(m_nFireAudioEntity >= 0) {
		DestroyEntity(m_nFireAudioEntity);
		m_nFireAudioEntity = -5;
	}
	if(m_nCollisionEntity >= 0) {
		DestroyEntity(m_nCollisionEntity);
		m_nCollisionEntity = -5;
	}
	if(m_nFrontEndEntity >= 0) {
		DestroyEntity(m_nFrontEndEntity);
		m_nFrontEndEntity = -5;
	}
	if(m_nProjectileEntity >= 0) {
		DestroyEntity(m_nProjectileEntity);
		m_nProjectileEntity = -5;
	}
}

WRAPPER
void
cAudioManager::ProcessActiveQueues()
{
	EAXJMP(0x57BA60);
}

bool
cAudioManager::ProcessAirBrakes(cVehicleParams *params)
{
	CAutomobile *automobile;
	uint8 rand;

	if(params->m_fDistance > 900.0f) return 0;
	automobile = (CAutomobile *)params->m_pVehicle;
	if(!automobile->bEngineOn) return 1;

	if((automobile->m_fVelocityChangeForAudio < 0.025f ||
	    params->m_fVelocityChange >= 0.025f) &&
	   (automobile->m_fVelocityChangeForAudio > -0.025f || params->m_fVelocityChange <= 0.025f))
		return 1;

	CalculateDistance((bool *)params, params->m_fDistance);
	rand = m_anRandomTable[0] % 10u + 70;
	m_sQueueSample.m_bVolume = ComputeVolume(rand, 30.0f, m_sQueueSample.m_fDistance);
	if(m_sQueueSample.m_bVolume) {
		m_sQueueSample.m_counter = 13;
		m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_VEHICLE_AIR_BRAKES;
		m_sQueueSample.m_nFrequency =
		    SampleManager.GetSampleBaseFrequency(AUDIO_SAMPLE_VEHICLE_AIR_BRAKES);
		m_sQueueSample.m_nFrequency += RandomDisplacement(m_sQueueSample.m_nFrequency >> 4);
		m_sQueueSample.m_bBankIndex = 0;
		m_sQueueSample.m_bIsDistant = 0;
		m_sQueueSample.field_16 = 10;
		m_sQueueSample.m_nLoopCount = 1;
		m_sQueueSample.m_bEmittingVolume = rand;
		m_sQueueSample.m_nLoopStart = 0;
		m_sQueueSample.m_nLoopEnd = -1;
		m_sQueueSample.field_48 = 0.0f;
		m_sQueueSample.m_fSoundIntensity = 30.0f;
		m_sQueueSample.field_56 = 1;
		m_sQueueSample.m_bReverbFlag = 1;
		m_sQueueSample.m_bRequireReflection = 0;
		AddSampleToRequestedQueue();
	}

	return 1;
}

void
cAudioManager::ProcessAirportScriptObject(uint8 sound)
{
	float dist;
	float maxDist;

	static uint8 counter = 0;

	uint32 time = CTimer::GetTimeInMilliseconds();
	if(time > audioLogicTimers[3]) {
		switch(sound) {
		case SCRIPT_SOUND_AIRPORT_LOOP_S:
			maxDist = 900.f;
			m_sQueueSample.m_fSoundIntensity = 30.0f;
			break;
		case SCRIPT_SOUND_AIRPORT_LOOP_L:
			maxDist = 6400.f;
			m_sQueueSample.m_fSoundIntensity = 80.0f;
			break;
		default: return;
		}
		float distSquared = GetDistanceSquared(&m_sQueueSample.m_vecPos);
		if(distSquared < maxDist) {
			dist = sqrt(distSquared);
			m_sQueueSample.m_fDistance = dist;
			m_sQueueSample.m_bVolume = ComputeVolume(
			    110u, m_sQueueSample.m_fSoundIntensity, m_sQueueSample.m_fDistance);
			if(m_sQueueSample.m_bVolume) {
				m_sQueueSample.m_nSampleIndex =
				    (m_anRandomTable[1] & 3) + AUDIO_SAMPLE_AIRPORT_1;
				m_sQueueSample.m_bBankIndex = 0;
				m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(
				    m_sQueueSample.m_nSampleIndex);
				m_sQueueSample.m_counter = counter++;
				m_sQueueSample.m_bIsDistant = 0;
				m_sQueueSample.m_nLoopCount = 1;
				m_sQueueSample.field_56 = 1;
				m_sQueueSample.field_16 = 3;
				m_sQueueSample.field_48 = 2.0f;
				m_sQueueSample.m_bEmittingVolume = 110;
				m_sQueueSample.m_nLoopStart = 0;
				m_sQueueSample.m_nLoopEnd = -1;
				m_sQueueSample.m_bReverbFlag = 1;
				m_sQueueSample.m_bRequireReflection = 0;
				AddSampleToRequestedQueue();
				audioLogicTimers[3] = time + 10000 + m_anRandomTable[3] % 20000u;
			}
		}
	}
}

bool
cAudioManager::ProcessBoatEngine(cVehicleParams *params)
{
	CBoat *boat;
	float padRelativeAccerate;
	float gasPedal;
	int32 padAccelerate;
	uint8 emittingVol;
	float oneShotVol;

	static uint16 LastAccel = 0;
	static uint8 LastVol = 0;

	if(params->m_fDistance < 2500.f) {
		boat = (CBoat *)params->m_pVehicle;
		if(params->m_nIndex == REEFER) {
			CalculateDistance((bool *)params, params->m_fDistance);
			m_sQueueSample.m_bVolume =
			    ComputeVolume(80u, 50.f, m_sQueueSample.m_fDistance);
			if(m_sQueueSample.m_bVolume) {
				m_sQueueSample.m_counter = 39;
				m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_MOTOR;
				m_sQueueSample.m_nFrequency = 10386;
				m_sQueueSample.m_nFrequency +=
				    (m_sQueueSample.m_nEntityIndex << 16) % 1000;
				m_sQueueSample.m_bBankIndex = 0;
				m_sQueueSample.m_bIsDistant = 0;
				m_sQueueSample.field_16 = 3;
				m_sQueueSample.m_nLoopCount = 0;
				m_sQueueSample.m_bEmittingVolume = 80;
				m_sQueueSample.m_nLoopStart =
				    SampleManager.GetSampleLoopStartOffset(
				        m_sQueueSample.m_nSampleIndex);
				m_sQueueSample.m_nLoopEnd = SampleManager.GetSampleLoopEndOffset(
				    m_sQueueSample.m_nSampleIndex);
				m_sQueueSample.field_48 = 2.0f;
				m_sQueueSample.m_fSoundIntensity = 50.0f;
				m_sQueueSample.field_56 = 0;
				m_sQueueSample.field_76 = 7;
				m_sQueueSample.m_bReverbFlag = 1;
				m_sQueueSample.m_bRequireReflection = 0;
				AddSampleToRequestedQueue();
			}
			if(FindPlayerVehicle() == params->m_pVehicle) {
				if(Pads[0].GetAccelerate() <= Pads[0].GetBrake())
					padAccelerate = Pads[0].GetBrake();
				else
					padAccelerate = Pads[0].GetAccelerate();
				padRelativeAccerate = padAccelerate / 255;
				emittingVol = (100.f * padRelativeAccerate) + 15;
				m_sQueueSample.m_nFrequency = (3000.f * padRelativeAccerate) + 6000;
				if(!boat->m_bIsAnchored)
					m_sQueueSample.m_nFrequency =
					    11 * m_sQueueSample.m_nFrequency / 10u;
			} else {
				gasPedal = Abs(boat->m_fGasPedal);
				if(gasPedal > 0.0f) {
					m_sQueueSample.m_nFrequency = 6000;
					emittingVol = 15;
				} else {
					emittingVol = (100.f * gasPedal) + 15;
					m_sQueueSample.m_nFrequency = (3000.f * gasPedal) + 6000;
					if(!boat->m_bIsAnchored)
						m_sQueueSample.m_nFrequency =
						    11 * m_sQueueSample.m_nFrequency / 10u;
				}
			}
			m_sQueueSample.m_bVolume =
			    ComputeVolume(emittingVol, 50.f, m_sQueueSample.m_fDistance);
			if(!m_sQueueSample.m_bVolume) return 1;
			m_sQueueSample.m_counter = 40;
			m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_BOAT_ENGINE;
			m_sQueueSample.m_nFrequency += (m_sQueueSample.m_nEntityIndex << 16) % 1000;
			m_sQueueSample.m_bBankIndex = 0;
			m_sQueueSample.m_bIsDistant = 0;
			m_sQueueSample.field_16 = 3;
			m_sQueueSample.m_nLoopCount = 0;
			m_sQueueSample.m_bEmittingVolume = emittingVol;
			m_sQueueSample.m_nLoopStart =
			    SampleManager.GetSampleLoopStartOffset(m_sQueueSample.m_nSampleIndex);
			m_sQueueSample.m_nLoopEnd =
			    SampleManager.GetSampleLoopEndOffset(m_sQueueSample.m_nSampleIndex);
			m_sQueueSample.field_48 = 2.0f;
			m_sQueueSample.m_fSoundIntensity = 50.0f;
			m_sQueueSample.field_56 = 0;
			m_sQueueSample.field_76 = 7;
			m_sQueueSample.m_bReverbFlag = 1;
			m_sQueueSample.m_bRequireReflection = 0;
		} else {
			if(FindPlayerVehicle() == params->m_pVehicle) {
				if(Pads[0].GetAccelerate() <= Pads[0].GetBrake())
					padAccelerate = Pads[0].GetBrake();
				else
					padAccelerate = Pads[0].GetAccelerate();
				if(padAccelerate <= 20) {
					emittingVol = 45 - 45 * padAccelerate / 40;
					m_sQueueSample.m_nFrequency = 100 * padAccelerate + 11025;
					m_sQueueSample.m_counter = 39;
					m_sQueueSample.m_nSampleIndex =
					    AUDIO_SAMPLE_BOAT_ENGINE_IDLE;
					if(LastAccel > 20) {
						oneShotVol = LastVol;
						PlayOneShot(m_sQueueSample.m_nEntityIndex, SOUND_17,
						            oneShotVol);
					}
				} else {
					emittingVol = 105 * padAccelerate / 255 + 15;
					m_sQueueSample.m_nFrequency =
					    4000 * padAccelerate / 255 + 8000;
					if(!boat->m_bIsAnchored)
						m_sQueueSample.m_nFrequency =
						    11 * m_sQueueSample.m_nFrequency / 10u;
					m_sQueueSample.m_counter = 40;
					m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_BOAT_ENGINE;
				}
				LastVol = emittingVol;
				LastAccel = padAccelerate;
			} else {
				gasPedal = Abs(boat->m_fGasPedal);
				if(gasPedal > 0.0f) {
					m_sQueueSample.m_nFrequency = 11025;
					emittingVol = 45;
					m_sQueueSample.m_counter = 39;
					m_sQueueSample.m_nSampleIndex =
					    AUDIO_SAMPLE_BOAT_ENGINE_IDLE;
				} else {
					emittingVol = (105.f * gasPedal) + 15;
					m_sQueueSample.m_nFrequency = (4000.f * gasPedal) + 8000;
					if(!boat->m_bIsAnchored)
						m_sQueueSample.m_nFrequency =
						    11 * m_sQueueSample.m_nFrequency / 10u;
					m_sQueueSample.m_counter = 40;
					m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_BOAT_ENGINE;
				}
			}
			CalculateDistance((bool *)params, params->m_fDistance);
			m_sQueueSample.m_bVolume =
			    ComputeVolume(emittingVol, 50.f, m_sQueueSample.m_fDistance);
			if(!m_sQueueSample.m_bVolume) return 1;
			m_sQueueSample.m_nFrequency += (m_sQueueSample.m_nEntityIndex << 16) % 1000;
			m_sQueueSample.m_bBankIndex = 0;
			m_sQueueSample.m_bIsDistant = 0;
			m_sQueueSample.field_16 = 3;
			m_sQueueSample.m_nLoopCount = 0;
			m_sQueueSample.m_bEmittingVolume = emittingVol;
			m_sQueueSample.m_nLoopStart =
			    SampleManager.GetSampleLoopStartOffset(m_sQueueSample.m_nSampleIndex);
			m_sQueueSample.m_nLoopEnd =
			    SampleManager.GetSampleLoopEndOffset(m_sQueueSample.m_nSampleIndex);
			m_sQueueSample.field_48 = 2.0f;
			m_sQueueSample.m_fSoundIntensity = 50.0f;
			m_sQueueSample.field_56 = 0;
			m_sQueueSample.field_76 = 7;
			m_sQueueSample.m_bReverbFlag = 1;
			m_sQueueSample.m_bRequireReflection = 0;
		}
		AddSampleToRequestedQueue();
		return 1;
	}
	return 0;
}

bool
cAudioManager::ProcessBoatMovingOverWater(cVehicleParams *params)
{
	float velocityChange;
	int32 vol;
	float multiplier;

	if(params->m_fDistance > 2500.f) return 0;

	velocityChange = Abs(params->m_fVelocityChange);
	if(velocityChange <= 0.0005f && params->m_pVehicle->GetPosition().y) return 1;

	velocityChange = min(0.75f, velocityChange);
	multiplier = (velocityChange - 0.0005f) * 4 / 3;
	CalculateDistance((bool *)params, params->m_fDistance);
	vol = (30.f * multiplier);
	m_sQueueSample.m_bVolume = ComputeVolume(vol, 50.f, m_sQueueSample.m_fDistance);
	if(m_sQueueSample.m_bVolume) {
		m_sQueueSample.m_counter = 38;
		m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_LOOPING_WATER;
		m_sQueueSample.m_bBankIndex = 0;
		m_sQueueSample.m_bIsDistant = 0;
		m_sQueueSample.field_16 = 3;
		m_sQueueSample.m_nFrequency = (6050.f * multiplier) + 16000;
		m_sQueueSample.m_nLoopCount = 0;
		m_sQueueSample.m_bEmittingVolume = vol;
		m_sQueueSample.m_nLoopStart =
		    SampleManager.GetSampleLoopStartOffset(m_sQueueSample.m_nSampleIndex);
		m_sQueueSample.m_nLoopEnd =
		    SampleManager.GetSampleLoopEndOffset(m_sQueueSample.m_nSampleIndex);
		m_sQueueSample.field_48 = 2.0f;
		m_sQueueSample.m_fSoundIntensity = 50.0f;
		m_sQueueSample.field_56 = 0;
		m_sQueueSample.field_76 = 3;
		m_sQueueSample.m_bReverbFlag = 1;
		m_sQueueSample.m_bRequireReflection = 0;
		AddSampleToRequestedQueue();
	}

	return 1;
}

void
cAudioManager::ProcessBridge()
{
	float dist;
	bool something = false;

	if(CBridge::pLiftRoad) {
		m_sQueueSample.m_vecPos = CBridge::pLiftRoad->GetPosition();
		dist = GetDistanceSquared(&m_sQueueSample.m_vecPos);
		if(dist < 202500.0f) {
			CalculateDistance(&something, dist);
			switch(CBridge::State) {
			case STATE_BRIDGE_LOCKED:
			case STATE_LIFT_PART_IS_UP:
			case STATE_LIFT_PART_ABOUT_TO_MOVE_UP: ProcessBridgeWarning(); break;
			case STATE_LIFT_PART_MOVING_DOWN:
			case STATE_LIFT_PART_MOVING_UP:
				ProcessBridgeWarning();
				ProcessBridgeMotor();
				break;
			default: break;
			}
			ProcessBridgeOneShots();
		}
	}
}

void
cAudioManager::ProcessBridgeMotor()
{
	if(m_sQueueSample.m_fDistance < 400.f) {
		m_sQueueSample.m_bVolume =
		    ComputeVolume(maxVolume, 400.f, m_sQueueSample.m_fDistance);
		if(m_sQueueSample.m_bVolume) {
			m_sQueueSample.m_counter = 1;
			m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_MOTOR;
			m_sQueueSample.m_bBankIndex = 0;
			m_sQueueSample.m_bIsDistant = 0;
			m_sQueueSample.field_16 = 1;
			m_sQueueSample.m_nFrequency = 5500;
			m_sQueueSample.m_nLoopCount = 0;
			m_sQueueSample.m_bEmittingVolume = maxVolume;
			m_sQueueSample.m_nLoopStart =
			    SampleManager.GetSampleLoopStartOffset(m_sQueueSample.m_nSampleIndex);
			m_sQueueSample.m_nLoopEnd =
			    SampleManager.GetSampleLoopEndOffset(m_sQueueSample.m_nSampleIndex);
			m_sQueueSample.field_48 = 2.0f;
			m_sQueueSample.m_fSoundIntensity = 400.0f;
			m_sQueueSample.field_56 = 0;
			m_sQueueSample.field_76 = 3;
			m_sQueueSample.m_bReverbFlag = 0;
			AddSampleToRequestedQueue();
		}
	}
}

void
cAudioManager::ProcessBridgeOneShots()
{
	float maxDist;

	if(CBridge::State != STATE_LIFT_PART_IS_UP ||
	   CBridge::OldState != STATE_LIFT_PART_MOVING_UP) {
		if(CBridge::State != STATE_LIFT_PART_IS_DOWN ||
		   CBridge::OldState != STATE_LIFT_PART_MOVING_DOWN) {
			if(CBridge::State != STATE_LIFT_PART_MOVING_UP ||
			   CBridge::OldState != STATE_LIFT_PART_ABOUT_TO_MOVE_UP) {
				if(CBridge::State != STATE_LIFT_PART_MOVING_DOWN ||
				   CBridge::OldState != STATE_LIFT_PART_IS_UP)
					return;
				maxDist = 400.f;
				m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_COLLISION_METAL_27;
			} else {
				maxDist = 400.f;
				m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_COLLISION_METAL_27;
			}
		} else {
			maxDist = 400.f;
			m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_COLLISION_METAL_27;
		}
	} else {
		maxDist = 400.f;
		m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_COLLISION_METAL_27;
	}
	if(m_sQueueSample.m_fDistance < maxDist) {
		m_sQueueSample.m_bVolume =
		    ComputeVolume(maxVolume, maxDist, m_sQueueSample.m_fDistance);
		if(m_sQueueSample.m_bVolume) {
			m_sQueueSample.m_counter = 2;
			m_sQueueSample.m_bBankIndex = 0;
			m_sQueueSample.m_bIsDistant = 0;
			m_sQueueSample.field_16 = 1;
			m_sQueueSample.m_nFrequency =
			    SampleManager.GetSampleBaseFrequency(m_sQueueSample.m_nSampleIndex);
			m_sQueueSample.m_nLoopCount = 1;
			m_sQueueSample.m_bEmittingVolume = maxVolume;
			m_sQueueSample.m_nLoopStart = 0;
			m_sQueueSample.m_nLoopEnd = -1;
			m_sQueueSample.field_48 = 2.0f;
			m_sQueueSample.m_fSoundIntensity = maxDist;
			m_sQueueSample.field_56 = 1;
			m_sQueueSample.m_bReverbFlag = 0;
			m_sQueueSample.m_bRequireReflection = 0;
			AddSampleToRequestedQueue();
		}
	}
}

void
cAudioManager::ProcessBridgeWarning()
{
	if(CStats::CommercialPassed && m_sQueueSample.m_fDistance < 450.f) {
		m_sQueueSample.m_bVolume = ComputeVolume(100u, 450.f, m_sQueueSample.m_fDistance);
		if(m_sQueueSample.m_bVolume) {
			m_sQueueSample.m_counter = 0;
			m_sQueueSample.m_nSampleIndex = 457;
			m_sQueueSample.m_bBankIndex = 0;
			m_sQueueSample.m_bIsDistant = 0;
			m_sQueueSample.field_16 = 1;
			m_sQueueSample.m_nFrequency =
			    SampleManager.GetSampleBaseFrequency(AUDIO_SAMPLE_BRIDGE_WARNING);
			m_sQueueSample.m_nLoopCount = 0;
			m_sQueueSample.m_bEmittingVolume = 100;
			m_sQueueSample.m_nLoopStart =
			    SampleManager.GetSampleLoopStartOffset(m_sQueueSample.m_nSampleIndex);
			m_sQueueSample.m_nLoopEnd =
			    SampleManager.GetSampleLoopEndOffset(m_sQueueSample.m_nSampleIndex);
			m_sQueueSample.field_48 = 2.0f;
			m_sQueueSample.m_fSoundIntensity = 450.0f;
			m_sQueueSample.field_56 = 0;
			m_sQueueSample.field_76 = 8;
			m_sQueueSample.m_bReverbFlag = 0;
			m_sQueueSample.m_bRequireReflection = 0;
			AddSampleToRequestedQueue();
		}
	}
}

bool
cAudioManager::ProcessCarBombTick(cVehicleParams *params)
{
	CAutomobile *automobile;

	if(params->m_fDistance >= 1600.f) return 0;
	automobile = (CAutomobile *)params->m_pVehicle;
	if(automobile->bEngineOn && automobile->m_bombType == CARBOMB_TIMEDACTIVE) {
		CalculateDistance((bool *)params, params->m_fDistance);
		m_sQueueSample.m_bVolume = ComputeVolume(60u, 40.f, m_sQueueSample.m_fDistance);
		if(m_sQueueSample.m_bVolume) {
			m_sQueueSample.m_counter = 35;
			m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_CAR_BOMB_TICK;
			m_sQueueSample.m_bBankIndex = 0;
			m_sQueueSample.m_bIsDistant = 0;
			m_sQueueSample.field_16 = 0;
			m_sQueueSample.m_nFrequency =
			    SampleManager.GetSampleBaseFrequency(AUDIO_SAMPLE_CAR_BOMB_TICK);
			m_sQueueSample.m_nLoopCount = 0;
			m_sQueueSample.m_bEmittingVolume = 60;
			m_sQueueSample.m_nLoopStart =
			    SampleManager.GetSampleLoopStartOffset(m_sQueueSample.m_nSampleIndex);
			m_sQueueSample.m_nLoopEnd =
			    SampleManager.GetSampleLoopEndOffset(m_sQueueSample.m_nSampleIndex);
			m_sQueueSample.field_48 = 2.0f;
			m_sQueueSample.m_fSoundIntensity = 40.0f;
			m_sQueueSample.field_56 = 0;
			m_sQueueSample.field_76 = 3;
			m_sQueueSample.m_bReverbFlag = 1;
			m_sQueueSample.m_bRequireReflection = 0;
			AddSampleToRequestedQueue();
		}
	}
	return 1;
}

WRAPPER
void
cAudioManager::ProcessCesna(void *)
{
	EAXJMP(0x56ADF0);
}

void
cAudioManager::ProcessCinemaScriptObject(uint8 sound)
{
	uint8 rand;
	float maxDist;

	static uint8 counter = 0;

	uint32 time = CTimer::GetTimeInMilliseconds();
	if(time > audioLogicTimers[4]) {
		switch(sound) {
		case SCRIPT_SOUND_CINEMA_LOOP_S:
			maxDist = 900.f;
			m_sQueueSample.m_fSoundIntensity = 30.0f;
			break;
		case SCRIPT_SOUND_CINEMA_LOOP_L:
			maxDist = 6400.f;
			m_sQueueSample.m_fSoundIntensity = 80.0f;
			break;
		default: return;
		}
		float distSquared = GetDistanceSquared(&m_sQueueSample.m_vecPos);
		if(distSquared < maxDist) {
			m_sQueueSample.m_fDistance = sqrt(distSquared);
			rand = m_anRandomTable[0] % 90u + 30;
			m_sQueueSample.m_bVolume = ComputeVolume(
			    rand, m_sQueueSample.m_fSoundIntensity, m_sQueueSample.m_fDistance);
			if(m_sQueueSample.m_bVolume) {
				m_sQueueSample.m_nSampleIndex = counter % 3 + AUDIO_SAMPLE_CINEMA_1;
				m_sQueueSample.m_bBankIndex = 0;
				m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(
				    m_sQueueSample.m_nSampleIndex);
				m_sQueueSample.m_nFrequency +=
				    RandomDisplacement(m_sQueueSample.m_nFrequency / 4);
				m_sQueueSample.m_counter = counter++;
				m_sQueueSample.m_bIsDistant = 0;
				m_sQueueSample.m_nLoopCount = 1;
				m_sQueueSample.field_56 = 1;
				m_sQueueSample.field_16 = 3;
				m_sQueueSample.field_48 = 2.0f;
				m_sQueueSample.m_bEmittingVolume = rand;
				m_sQueueSample.m_nLoopStart = 0;
				m_sQueueSample.m_nLoopEnd = -1;
				m_sQueueSample.m_bReverbFlag = 1;
				m_sQueueSample.m_bRequireReflection = 0;
				AddSampleToRequestedQueue();
				audioLogicTimers[4] = time + 1000 + m_anRandomTable[3] % 4000u;
			}
		}
	}
}

WRAPPER
void
cAudioManager::ProcessCrane()
{
	EAXJMP(0x578910);
}

void
cAudioManager::ProcessDocksScriptObject(uint8 sound)
{
	uint32 time;
	uint8 rand;
	float distSquared;
	float maxDist;

	static uint32 counter = 0;

	time = CTimer::GetTimeInMilliseconds();
	if(time > audioLogicTimers[5]) {
		switch(sound) {
		case SCRIPT_SOUND_DOCKS_LOOP_S:
			maxDist = 900.f;
			m_sQueueSample.m_fSoundIntensity = 30.0f;
			break;
		case SCRIPT_SOUND_DOCKS_LOOP_L:
			maxDist = 6400.f;
			m_sQueueSample.m_fSoundIntensity = 80.0f;
			break;
		default: return;
		}
		distSquared = GetDistanceSquared(&m_sQueueSample.m_vecPos);
		if(distSquared < maxDist) {
			m_sQueueSample.m_fDistance = sqrt(distSquared);
			rand = m_anRandomTable[0] % 60u + 40;
			m_sQueueSample.m_bVolume = ComputeVolume(
			    rand, m_sQueueSample.m_fSoundIntensity, m_sQueueSample.m_fDistance);
			if(m_sQueueSample.m_bVolume) {
				m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_DOCKS;
				m_sQueueSample.m_bBankIndex = 0;
				m_sQueueSample.m_nFrequency =
				    SampleManager.GetSampleBaseFrequency(AUDIO_SAMPLE_DOCKS);
				m_sQueueSample.m_nFrequency +=
				    RandomDisplacement(m_sQueueSample.m_nFrequency >> 3);
				m_sQueueSample.m_counter = counter++;
				m_sQueueSample.m_bIsDistant = 0;
				m_sQueueSample.m_nLoopCount = 1;
				m_sQueueSample.field_56 = 1;
				m_sQueueSample.field_16 = 3;
				m_sQueueSample.field_48 = 2.0f;
				m_sQueueSample.m_bEmittingVolume = rand;
				m_sQueueSample.m_nLoopStart = 0;
				m_sQueueSample.m_nLoopEnd = -1;
				m_sQueueSample.m_bReverbFlag = 1;
				m_sQueueSample.m_bRequireReflection = 0;
				AddSampleToRequestedQueue();
				audioLogicTimers[5] = time + 10000 + m_anRandomTable[3] % 40000u;
			}
		}
	}
}

bool
cAudioManager::ProcessEngineDamage(cVehicleParams *params)
{
	CAutomobile *veh;
	uint8 engineStatus;
	uint8 emittingVolume;

	if(params->m_fDistance >= 1600.f) return 0;
	veh = (CAutomobile *)params->m_pVehicle;
	if(veh->bEngineOn) {
		engineStatus = veh->Damage.GetEngineStatus();
		if(engineStatus > 250u || engineStatus < 100u) return 1;
		if(engineStatus < 225u) {
			m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_JUMBO_TAXI_SOUND;
			emittingVolume = 6;
			m_sQueueSample.field_16 = 7;
			m_sQueueSample.m_nFrequency = 40000;
		} else {
			emittingVolume = 60;
			m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_FIRE_ENTITY;
			m_sQueueSample.field_16 = 7;
			m_sQueueSample.m_nFrequency =
			    SampleManager.GetSampleBaseFrequency(AUDIO_SAMPLE_FIRE_ENTITY);
		}
		CalculateDistance((bool *)params, params->m_fDistance);
		m_sQueueSample.m_bVolume =
		    ComputeVolume(emittingVolume, 40.f, m_sQueueSample.m_fDistance);
		if(m_sQueueSample.m_bVolume) {
			m_sQueueSample.m_counter = 28;
			m_sQueueSample.m_bBankIndex = 0;
			m_sQueueSample.m_bIsDistant = 0;
			m_sQueueSample.m_nLoopCount = 0;
			m_sQueueSample.m_bEmittingVolume = emittingVolume;
			m_sQueueSample.m_nLoopStart =
			    SampleManager.GetSampleLoopStartOffset(m_sQueueSample.m_nSampleIndex);
			m_sQueueSample.m_nLoopEnd =
			    SampleManager.GetSampleLoopEndOffset(m_sQueueSample.m_nSampleIndex);
			m_sQueueSample.field_48 = 2.0f;
			m_sQueueSample.m_fSoundIntensity = 40.0f;
			m_sQueueSample.field_56 = 0;
			m_sQueueSample.field_76 = 3;
			m_sQueueSample.m_bReverbFlag = 1;
			m_sQueueSample.m_bRequireReflection = 0;
			AddSampleToRequestedQueue();
		}
	}
	return 1;
}

void
cAudioManager::ProcessEntity(int32 id)
{
	if(m_asAudioEntities[id].m_bStatus) {
		m_sQueueSample.m_nEntityIndex = id;
		switch(m_asAudioEntities[id].m_nType) {
		case AUDIOTYPE_PHYSICAL:
			if(!m_bUserPause) {
				m_sQueueSample.m_bReverbFlag = 1;
				ProcessPhysical(id);
			}
			break;
		case AUDIOTYPE_EXPLOSION:
			if(!m_bUserPause) {
				m_sQueueSample.m_bReverbFlag = 1;
				ProcessExplosions(id);
			}
			break;
		case AUDIOTYPE_FIRE:
			if(!m_bUserPause) {
				m_sQueueSample.m_bReverbFlag = 1;
				ProcessFires(id);
			}
			break;
		case AUDIOTYPE_WEATHER:
			if(!m_bUserPause) {
				m_sQueueSample.m_bReverbFlag = 1;
				ProcessWeather(id);
			}
			break;
		case AUDIOTYPE_CRANE:
			if(!m_bUserPause) {
				m_sQueueSample.m_bReverbFlag = 1;
				ProcessCrane();
			}
			break;
		case AUDIOTYPE_SCRIPTOBJECT:
			if(!m_bUserPause) {
				m_sQueueSample.m_bReverbFlag = 1;
				ProcessScriptObject(id);
			}
			break;
		case AUDIOTYPE_BRIDGE:
			if(!m_bUserPause) {
				m_sQueueSample.m_bReverbFlag = 1;
				ProcessBridge();
			}
			break;
		case AUDIOTYPE_FRONTEND:
			m_sQueueSample.m_bReverbFlag = 0;
			ProcessFrontEnd();
			break;
		case AUDIOTYPE_PROJECTILE:
			if(!m_bUserPause) {
				m_sQueueSample.m_bReverbFlag = 1;
				ProcessProjectiles();
			}
			break;
		case AUDIOTYPE_GARAGE:
			if(!m_bUserPause) ProcessGarages();
			break;
		case AUDIOTYPE_FIREHYDRANT:
			if(!m_bUserPause) {
				m_sQueueSample.m_bReverbFlag = 1;
				ProcessFireHydrant();
			}
			break;
		case AUDIOTYPE_WATERCANNON:
			if(!m_bUserPause) {
				m_sQueueSample.m_bReverbFlag = 1;
				ProcessWaterCannon(id);
			}
			break;
		default: return;
		}
	}
}

void
cAudioManager::ProcessExplosions(int32 explosion)
{
	uint8 type;
	CVector *pos;
	float maxDist;
	float distSquared;

	for(uint8 i = 0; i < 48u; i++) {
		if(CExplosion::GetExplosionActiveCounter(i) == 1) {
			CExplosion::ResetExplosionActiveCounter(i);
			type = CExplosion::GetExplosionType(i);
			switch(type) {
			case EXPLOSION_GRENADE:
			case EXPLOSION_ROCKET:
			case EXPLOSION_BARREL:
			case EXPLOSION_TANK_GRENADE:
				maxDist = 160000.f;
				m_sQueueSample.m_fSoundIntensity = 400.0f;
				m_sQueueSample.m_nSampleIndex =
				    AUDIO_SAMPLE_WEAPON_GRENADE_EXPLOSION;
				m_sQueueSample.m_nFrequency = RandomDisplacement(2000u) + 38000;
				m_sQueueSample.field_16 = 0;
				m_sQueueSample.m_bBankIndex = 0;
				break;
			case EXPLOSION_MOLOTOV:
				maxDist = 40000.f;
				m_sQueueSample.m_fSoundIntensity = 200.0f;
				m_sQueueSample.m_nSampleIndex =
				    AUDIO_SAMPLE_WEAPON_MOLOTOV_EXPLOSION;
				m_sQueueSample.m_nFrequency = RandomDisplacement(1000u) + 19000;
				m_sQueueSample.field_16 = 0;
				m_sQueueSample.m_bBankIndex = 0;
				break;
			case EXPLOSION_MINE:
			case EXPLOSION_HELI_BOMB:
				maxDist = 90000.f;
				m_sQueueSample.m_fSoundIntensity = 300.0f;
				m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_WEAPON_ROCKET_SHOT;
				m_sQueueSample.m_nFrequency = RandomDisplacement(1000u) + 12347;
				m_sQueueSample.field_16 = 0;
				m_sQueueSample.m_bBankIndex = 0;
				break;
			default:
				maxDist = 160000.f;
				m_sQueueSample.m_fSoundIntensity = 400.0f;
				m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_EXPLOSION_CAR;
				m_sQueueSample.m_nFrequency = RandomDisplacement(2000u) + 38000;
				if(type == EXPLOSION_HELI)
					m_sQueueSample.m_nFrequency =
					    8 * m_sQueueSample.m_nFrequency / 10u;
				m_sQueueSample.field_16 = 0;
				m_sQueueSample.m_bBankIndex = 0;
				break;
			}
			pos = CExplosion::GetExplosionPosition(i);
			m_sQueueSample.m_vecPos = *pos;
			distSquared = GetDistanceSquared(&m_sQueueSample.m_vecPos);
			if(distSquared < maxDist) {
				m_sQueueSample.m_fDistance = Sqrt(distSquared);
				m_sQueueSample.m_bVolume =
				    ComputeVolume(maxVolume, m_sQueueSample.m_fSoundIntensity,
				                  m_sQueueSample.m_fDistance);
				if(m_sQueueSample.m_bVolume) {
					m_sQueueSample.m_counter = i;
					m_sQueueSample.field_48 = 2.0f;
					m_sQueueSample.m_bIsDistant = 0;
					m_sQueueSample.m_nLoopCount = 1;
					m_sQueueSample.field_56 = 1;
					m_sQueueSample.m_bReverbFlag = 1;
					m_sQueueSample.m_bEmittingVolume = maxVolume;
					m_sQueueSample.m_nLoopStart = 0;
					m_sQueueSample.m_nLoopEnd = -1;
					m_sQueueSample.m_bReverbFlag = 1;
					m_sQueueSample.m_bRequireReflection = 1;
					AddSampleToRequestedQueue();
				}
			}
		}
	}
}

void
cAudioManager::ProcessFireHydrant()
{
	float distSquared;
	bool something = false;

	m_sQueueSample.m_vecPos =
	    *(CVector *)((size_t)m_asAudioEntities[m_sQueueSample.m_nEntityIndex].m_pEntity + 52);
	distSquared = GetDistanceSquared(&m_sQueueSample.m_vecPos);
	if(distSquared < 1225.f) {
		CalculateDistance(&something, distSquared);
		m_sQueueSample.m_bVolume = ComputeVolume(40u, 35.f, m_sQueueSample.m_fDistance);
		if(m_sQueueSample.m_bVolume) {
			m_sQueueSample.m_counter = 0;
			m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_JUMBO_TAXI_SOUND;
			m_sQueueSample.m_bBankIndex = 0;
			m_sQueueSample.m_bIsDistant = 0;
			m_sQueueSample.field_16 = 4;
			m_sQueueSample.m_nFrequency = 15591;
			m_sQueueSample.m_nLoopCount = 0;
			m_sQueueSample.m_bEmittingVolume = 40;
			m_sQueueSample.m_nLoopStart =
			    SampleManager.GetSampleLoopStartOffset(m_sQueueSample.m_nSampleIndex);
			m_sQueueSample.m_nLoopEnd =
			    SampleManager.GetSampleLoopEndOffset(m_sQueueSample.m_nSampleIndex);
			m_sQueueSample.field_48 = 2.0f;
			m_sQueueSample.m_fSoundIntensity = 35.0f;
			m_sQueueSample.field_56 = 0;
			m_sQueueSample.field_76 = 3;
			m_sQueueSample.m_bReverbFlag = 1;
			m_sQueueSample.m_bRequireReflection = 0;
			AddSampleToRequestedQueue();
		}
	}
}

WRAPPER
void
cAudioManager::ProcessFires(int32 entity)
{
	EAXJMP(0x575CD0);
}

void
cAudioManager::ProcessFrontEnd()
{
	bool processed;
	int16 sample;

	static uint32 counter = 0;

	for(uint32 i = 0; i < m_asAudioEntities[m_sQueueSample.m_nEntityIndex].m_Loops; i++) {
		processed = 0;
		switch(
		    m_asAudioEntities[m_sQueueSample.m_nEntityIndex].m_awAudioEvent[i]) {
		case SOUND_WEAPON_SNIPER_SHOT_NO_ZOOM:
			m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_SNIPER_NO_ZOOM;
			break;
		case SOUND_WEAPON_ROCKET_SHOT_NO_ZOOM:
			m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_ROCKET_NO_ZOOM;
			break;
		case SOUND_GARAGE_NO_MONEY:
		case SOUND_GARAGE_BAD_VEHICLE:
		case SOUND_3C:
			m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_PICKUP_FAIL_1;
			processed = 1;
			break;
		case SOUND_GARAGE_OPENING:
		case SOUND_GARAGE_BOMB1_SET:
		case SOUND_GARAGE_BOMB2_SET:
		case SOUND_GARAGE_BOMB3_SET:
		case SOUND_41:
		case SOUND_GARAGE_VEHICLE_DECLINED:
		case SOUND_GARAGE_VEHICLE_ACCEPTED:
		case SOUND_PICKUP_HEALTH:
		case SOUND_4B:
		case SOUND_PICKUP_ADRENALINE:
		case SOUND_PICKUP_ARMOUR:
		case SOUND_EVIDENCE_PICKUP:
		case SOUND_UNLOAD_GOLD:
			m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_PICKUP_SUCCESS_1;
			processed = 1;
			break;
		case SOUND_PICKUP_WEAPON_BOUGHT:
		case SOUND_PICKUP_WEAPON:
			m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_PICKUP_NEUTRAL_1;
			processed = 1;
			break;
		case SOUND_4A:
			m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_PICKUP_FAIL_1;
			processed = 1;
			break;
		case SOUND_PICKUP_BONUS:
		case SOUND_PICKUP_MONEY:
		case SOUND_PICKUP_HIDDEN_PACKAGE:
		case SOUND_PICKUP_PACMAN_PILL:
		case SOUND_PICKUP_PACMAN_PACKAGE:
		case SOUND_PICKUP_FLOAT_PACKAGE:
			m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_PICKUP_SUCCESS_3;
			processed = 1;
			break;
		case SOUND_PAGER: m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_PAGER; break;
		case SOUND_RACE_START_3:
		case SOUND_RACE_START_2:
		case SOUND_RACE_START_1:
		case SOUND_CLOCK_TICK:
			m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_FRONTEND_CLOCK_TICK;
			break;
		case SOUND_RACE_START_GO:
			m_sQueueSample.m_nSampleIndex =
			    AUDIO_SAMPLE_FRONTEND_PART_MISSION_COMPLETED;
			break;
		case SOUND_PART_MISSION_COMPLETE:
			m_sQueueSample.m_nSampleIndex =
			    AUDIO_SAMPLE_FRONTEND_PART_MISSION_COMPLETED;
			break;
		case SOUND_FRONTEND_MENU_STARTING:
			processed = 1;
			m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_FRONTEND_MENU_STARTING_1;
			break;
		case SOUND_FRONTEND_MENU_COMPLETED:
			processed = 1;
			m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_FRONTEND_MENU_COMPLETED_1;
			break;
		case SOUND_FRONTEND_MENU_DENIED:
			processed = 1;
			m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_FRONTEND_MENU_DENIED_1;
			break;
		case SOUND_FRONTEND_MENU_SUCCESS:
			processed = 1;
			m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_FRONTEND_MENU_SUCCESS_1;
			break;
		case SOUND_FRONTEND_EXIT:
			processed = 1;
			m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_FRONTEND_MENU_EXIT_1;
			break;
		case SOUND_9A:
			processed = 1;
			m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_380;
			break;
		case SOUND_9B: m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_382; break;
		case SOUND_FRONTEND_AUDIO_TEST:
			m_sQueueSample.m_nSampleIndex =
			    m_anRandomTable[0] % 3u + AUDIO_SAMPLE_FRONTEND_MENU_AUDIO_TEST_1;
			break;
		case SOUND_FRONTEND_FAIL:
			processed = 1;
			m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_FRONTEND_MENU_FAIL_1;
			break;
		case SOUND_FRONTEND_NO_RADIO:
		case SOUND_FRONTEND_RADIO_CHANGE:
			m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_FRONTEND_GAMEPLAY_FAIL;
			break;
		case SOUND_A0:
			m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_FRONTEND_GAMEPLAY_SUCCESS;
			break;
		default: continue;
		}

		sample =
		    m_asAudioEntities[m_sQueueSample.m_nEntityIndex].m_awAudioEvent[i];
		if(sample == AUDIO_SAMPLE_COLLISION_LOOPING_GRASS) {
			m_sQueueSample.m_nFrequency = 28509;
		} else if(sample == AUDIO_SAMPLE_PICKUP_NEUTRAL_1) {
			if(1.f ==
			   m_asAudioEntities[m_sQueueSample.m_nEntityIndex].m_afVolume[i])
				m_sQueueSample.m_nFrequency = 32000;
			else
				m_sQueueSample.m_nFrequency = 48000;
		} else {
			m_sQueueSample.m_nFrequency =
			    SampleManager.GetSampleBaseFrequency(m_sQueueSample.m_nSampleIndex);
		}
		m_sQueueSample.m_bVolume = 110;
		m_sQueueSample.m_counter = counter++;
		m_sQueueSample.m_nLoopCount = 1;
		m_sQueueSample.field_56 = 1;
		m_sQueueSample.m_bBankIndex = 0;
		m_sQueueSample.field_16 = 0;
		m_sQueueSample.m_bIsDistant = 1;
		m_sQueueSample.m_bEmittingVolume = m_sQueueSample.m_bVolume;
		m_sQueueSample.m_nLoopStart = 0;
		m_sQueueSample.m_nLoopEnd = -1;
		if(processed)
			m_sQueueSample.m_bOffset = m_anRandomTable[0] & 0x1F;
		else
			m_sQueueSample.m_bOffset = 63;
		m_sQueueSample.m_bReverbFlag = 0;
		m_sQueueSample.m_bRequireReflection = 0;
		AddSampleToRequestedQueue();
		if(processed) {
			++m_sQueueSample.m_nSampleIndex;
			m_sQueueSample.m_counter = counter++;
			m_sQueueSample.m_bOffset = maxVolume - m_sQueueSample.m_bOffset;
			AddSampleToRequestedQueue();
		}
	}
}

WRAPPER
void
cAudioManager::ProcessGarages()
{
	EAXJMP(0x578C20);
}

struct tHelicopterSampleData {
	float m_fMaxDistance;
	float m_fBaseDistance;
	char m_bBaseVolume;
	char gap_9[3];
};

tHelicopterSampleData *gHeliSfxRanges = (tHelicopterSampleData *)0x604784;

bool
cAudioManager::ProcessHelicopter(cVehicleParams *params)
{
	CHeli *heli;
	float MaxDist;
	float dist;
	float baseDist;
	int32 emittingVol;

	if(gHeliSfxRanges[0].m_fMaxDistance * gHeliSfxRanges[0].m_fMaxDistance <=
	   params->m_fDistance)
		return 0;

	CalculateDistance((bool *)params, params->m_fDistance);
	heli = (CHeli *)params->m_pVehicle;
	for(uint32 i = 0; i < 3; i++) {
		MaxDist = gHeliSfxRanges[i].m_fMaxDistance;
		dist = m_sQueueSample.m_fDistance;
		if(dist >= MaxDist) return 1;
		baseDist = gHeliSfxRanges[i].m_fBaseDistance;
		if(dist < baseDist)
			emittingVol = (gHeliSfxRanges[i].m_bBaseVolume *
			               ((MaxDist - dist) / (MaxDist - baseDist)));
		else
			emittingVol = gHeliSfxRanges[i].m_bBaseVolume;

		m_sQueueSample.m_bVolume = ComputeVolume(
		    emittingVol, gHeliSfxRanges[i].m_fMaxDistance, m_sQueueSample.m_fDistance);
		if(m_sQueueSample.m_bVolume) {
			m_sQueueSample.m_counter = i + 65;
			m_sQueueSample.m_nSampleIndex = i + AUDIO_SAMPLE_HELI_FAR;
			m_sQueueSample.m_bBankIndex = 0;
			m_sQueueSample.m_bIsDistant = 0;
			m_sQueueSample.field_16 = 0;
			m_sQueueSample.m_nFrequency =
			    1200 * heli->m_nHeliId +
			    SampleManager.GetSampleBaseFrequency(m_sQueueSample.m_nSampleIndex);
			m_sQueueSample.m_nLoopCount = 0;
			m_sQueueSample.m_bEmittingVolume = emittingVol;
			m_sQueueSample.m_nLoopStart =
			    SampleManager.GetSampleLoopStartOffset(m_sQueueSample.m_nSampleIndex);
			m_sQueueSample.m_nLoopEnd =
			    SampleManager.GetSampleLoopEndOffset(m_sQueueSample.m_nSampleIndex);
			m_sQueueSample.field_48 = 6.0f;
			m_sQueueSample.m_fSoundIntensity = gHeliSfxRanges[i].m_fMaxDistance;
			m_sQueueSample.field_56 = 0;
			m_sQueueSample.field_76 = 3;
			m_sQueueSample.m_bReverbFlag = 1;
			m_sQueueSample.m_bRequireReflection = 0;
			AddSampleToRequestedQueue();
		}
	}
	return 1;
}

void
cAudioManager::ProcessHomeScriptObject(uint8 sound)
{
	uint32 time;
	uint8 rand;
	float dist;
	float maxDist;

	static uint8 counter = 0;

	time = CTimer::GetTimeInMilliseconds();
	if(time > audioLogicTimers[6]) {
		switch(sound) {
		case SCRIPT_SOUND_HOME_LOOP_S:
			maxDist = 900.f;
			m_sQueueSample.m_fSoundIntensity = 30.0f;
			break;
		case SCRIPT_SOUND_HOME_LOOP_L:
			maxDist = 6400.f;
			m_sQueueSample.m_fSoundIntensity = 80.0f;
			break;
		default: return;
		}
		dist = GetDistanceSquared(&m_sQueueSample.m_vecPos);
		if(dist < maxDist) {
			m_sQueueSample.m_fDistance = sqrt(dist);
			rand = m_anRandomTable[0] % 30u + 40;
			m_sQueueSample.m_bVolume = ComputeVolume(
			    rand, m_sQueueSample.m_fSoundIntensity, m_sQueueSample.m_fDistance);
			if(m_sQueueSample.m_bVolume) {
				m_sQueueSample.m_nSampleIndex =
				    m_anRandomTable[0] % 5u + AUDIO_SAMPLE_HOME_1;
				m_sQueueSample.m_bBankIndex = 0;
				m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(
				    m_sQueueSample.m_nSampleIndex);
				m_sQueueSample.m_nFrequency +=
				    RandomDisplacement(m_sQueueSample.m_nFrequency / 16);
				m_sQueueSample.m_counter = counter++;
				m_sQueueSample.m_bIsDistant = 0;
				m_sQueueSample.m_nLoopCount = 1;
				m_sQueueSample.field_56 = 1;
				m_sQueueSample.field_16 = 3;
				m_sQueueSample.field_48 = 0.0f;
				m_sQueueSample.m_bEmittingVolume = rand;
				m_sQueueSample.m_nLoopStart = 0;
				m_sQueueSample.m_nLoopEnd = -1;
				m_sQueueSample.m_bReverbFlag = 1;
				m_sQueueSample.m_bRequireReflection = 1;
				AddSampleToRequestedQueue();
				audioLogicTimers[6] = time + 1000 + m_anRandomTable[3] % 4000u;
			}
		}
	}
}

void
cAudioManager::ProcessJumbo(cVehicleParams *params)
{
	CPlane *plane;
	float position;

	if(params->m_fDistance < 193600.0f) {
		CalculateDistance((bool *)params, params->m_fDistance);
		plane = (CPlane *)params->m_pVehicle;
		DoJumboVolOffset();
		position = PlanePathPosition[plane->m_nPlaneId];
		if(position <= TakeOffPoint) {
			if(plane->m_fSpeed <= 0.10334f) {
				ProcessJumboTaxi();
				return;
			}

			ProcessJumboAccel(plane);
		} else if(300.0f + TakeOffPoint >= position) {
			ProcessJumboTakeOff(plane);
		} else if(LandingPoint - 350.0f >= position) {
			ProcessJumboFlying();
		} else {
			if(position > LandingPoint) {
				if(plane->m_fSpeed > 0.10334f) {
					ProcessJumboDecel(plane);
					return;
				}
				ProcessJumboTaxi();
				return;
			}
			ProcessJumboLanding(plane);
		}
	}
}

void
cAudioManager::ProcessJumboAccel(CPlane *plane)
{
	int32 engineFreq;
	int32 vol;
	float whineSoundFreq;
	float modificator;

	if(SetupJumboFlySound(20u)) {
		modificator = (plane->m_fSpeed - 0.10334f) * 1.676f;
		if(modificator > 1.0f) modificator = 1.0f;
		if(SetupJumboRumbleSound(maxVolume * modificator) &&
		   SetupJumboTaxiSound((1.0f - modificator) * 75.f)) {
			if(modificator < 0.2f) {
				whineSoundFreq = modificator * 5.f * 14600.0f + 29500;
				vol = modificator * 5.f * maxVolume;
				engineFreq = modificator * 5.f * 6050.f + 16000;
			} else {
				whineSoundFreq = 44100;
				engineFreq = 22050;
				vol = maxVolume;
			}
			SetupJumboEngineSound(vol, engineFreq);
			SetupJumboWhineSound(18u, whineSoundFreq);
		}
	}
}

void
cAudioManager::ProcessJumboDecel(CPlane *plane)
{
	float modificator;

	if(SetupJumboFlySound(20u) && SetupJumboTaxiSound(75u)) {
		modificator = (plane->m_fSpeed - 0.10334f) * 1.676f;
		if(modificator > 1.0f) modificator = 1.0f;
		SetupJumboEngineSound(maxVolume * modificator, 6050.f * modificator + 16000);
		SetupJumboWhineSound(18u, 29500);
	}
}

void
cAudioManager::ProcessJumboFlying()
{
	if(SetupJumboFlySound(maxVolume)) SetupJumboEngineSound(63u, 22050);
}

void
cAudioManager::ProcessJumboLanding(CPlane *plane)
{
	float modificator = (LandingPoint - PlanePathPosition[plane->m_nPlaneId]) / 350.f;
	if(SetupJumboFlySound(107.f * modificator + 20)) {
		if(SetupJumboTaxiSound(75.f * (1.f - modificator))) {
			SetupJumboEngineSound(maxVolume, 22050);
			SetupJumboWhineSound(18.f * (1.f - modificator),
			                     14600.f * modificator + 29500);
		}
	}
}

void
cAudioManager::ProcessJumboTakeOff(CPlane *plane)
{
	float modificator = (PlanePathPosition[plane->m_nPlaneId] - TakeOffPoint) / 300.f;

	if(SetupJumboFlySound((107.f * modificator) + 20) &&
	   SetupJumboRumbleSound(maxVolume * (1.f - modificator))) {
		if(SetupJumboEngineSound(maxVolume, 22050))
			SetupJumboWhineSound(18.f * (1.f - modificator), 44100);
	}
}

void
cAudioManager::ProcessJumboTaxi()
{
	if(SetupJumboFlySound(20u)) {
		if(SetupJumboTaxiSound(75u)) SetupJumboWhineSound(18u, 29500);
	}
}

void
cAudioManager::ProcessLaunderetteScriptObject(uint8 sound)
{
	float maxDist;

	switch(sound) {
	case SCRIPT_SOUND_LAUNDERETTE_LOOP_S:
	case SCRIPT_SOUND_LAUNDERETTE_LOOP_L:
		maxDist = 900.f;
		m_sQueueSample.m_fSoundIntensity = 30.0f;
		break;
	default: return;
	}
	float distSquared = GetDistanceSquared(&m_sQueueSample.m_vecPos);
	if(distSquared < maxDist) {
		m_sQueueSample.m_fDistance = sqrt(distSquared);
		m_sQueueSample.m_bVolume = ComputeVolume(45u, m_sQueueSample.m_fSoundIntensity,
		                                         m_sQueueSample.m_fDistance);
		if(m_sQueueSample.m_bVolume) {
			m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_LAUNDERETTE_1;
			m_sQueueSample.m_bBankIndex = 0;
			m_sQueueSample.m_nFrequency =
			    SampleManager.GetSampleBaseFrequency(AUDIO_SAMPLE_LAUNDERETTE_1);
			m_sQueueSample.m_counter = 0;
			m_sQueueSample.m_bIsDistant = 0;
			m_sQueueSample.m_nLoopCount = 0;
			m_sQueueSample.field_56 = 0;
			m_sQueueSample.field_16 = 5;
			m_sQueueSample.field_48 = 2.0f;
			m_sQueueSample.m_bEmittingVolume = 45;
			m_sQueueSample.m_nLoopStart =
			    SampleManager.GetSampleLoopStartOffset(m_sQueueSample.m_nSampleIndex);
			m_sQueueSample.m_nLoopEnd =
			    SampleManager.GetSampleLoopEndOffset(m_sQueueSample.m_nSampleIndex);
			m_sQueueSample.m_bReverbFlag = 1;
			m_sQueueSample.m_bRequireReflection = 0;
			AddSampleToRequestedQueue();
		}
		m_sQueueSample.m_bVolume = ComputeVolume(110u, m_sQueueSample.m_fSoundIntensity,
		                                         m_sQueueSample.m_fDistance);
		if(m_sQueueSample.m_bVolume) {
			m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_LAUNDERETTE_2;
			m_sQueueSample.m_bBankIndex = 0;
			m_sQueueSample.m_nFrequency =
			    SampleManager.GetSampleBaseFrequency(AUDIO_SAMPLE_LAUNDERETTE_2);
			m_sQueueSample.m_counter = 1;
			m_sQueueSample.m_bIsDistant = 0;
			m_sQueueSample.m_nLoopCount = 0;
			m_sQueueSample.field_56 = 0;
			m_sQueueSample.field_16 = 3;
			m_sQueueSample.field_48 = 2.0f;
			m_sQueueSample.m_bEmittingVolume = 110;
			m_sQueueSample.m_nLoopStart =
			    SampleManager.GetSampleLoopStartOffset(m_sQueueSample.m_nSampleIndex);
			m_sQueueSample.m_nLoopEnd =
			    SampleManager.GetSampleLoopEndOffset(m_sQueueSample.m_nSampleIndex);
			m_sQueueSample.m_bReverbFlag = 1;
			m_sQueueSample.m_bRequireReflection = 0;
			AddSampleToRequestedQueue();
		}
	}
}

void
cAudioManager::ProcessLoopingScriptObject(uint8 sound)
{
	uint8 emittingVolume;
	float maxDist;
	float distSquared;

	switch(sound) {
	case SCRIPT_SOUND_PARTY_1_LOOP_S:
		maxDist = 900.f;
		m_sQueueSample.m_fSoundIntensity = 30.0f;
		m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_PARTY_1;
		m_sQueueSample.m_bBankIndex = 0;
		emittingVolume = maxVolume;
		m_sQueueSample.m_nFrequency =
		    SampleManager.GetSampleBaseFrequency(AUDIO_SAMPLE_PARTY_1);
		m_sQueueSample.field_16 = 3;
		m_sQueueSample.field_76 = 3;
		m_sQueueSample.field_48 = 2.0f;
		break;
	case SCRIPT_SOUND_PARTY_1_LOOP_L:
		maxDist = 6400.f;
		m_sQueueSample.m_fSoundIntensity = 80.0f;
		m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_PARTY_1;
		m_sQueueSample.m_bBankIndex = 0;
		emittingVolume = maxVolume;
		m_sQueueSample.m_nFrequency =
		    SampleManager.GetSampleBaseFrequency(AUDIO_SAMPLE_PARTY_1);
		m_sQueueSample.field_16 = 3;
		m_sQueueSample.field_76 = 3;
		m_sQueueSample.field_48 = 2.0f;
		break;
	case SCRIPT_SOUND_PARTY_2_LOOP_S:
		maxDist = 900.f;
		m_sQueueSample.m_fSoundIntensity = 30.0f;
		m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_PARTY_2;
		m_sQueueSample.m_bBankIndex = 0;
		emittingVolume = maxVolume;
		m_sQueueSample.m_nFrequency =
		    SampleManager.GetSampleBaseFrequency(AUDIO_SAMPLE_PARTY_2);
		m_sQueueSample.field_16 = 3;
		m_sQueueSample.field_76 = 3;
		m_sQueueSample.field_48 = 2.0f;
		break;
	case SCRIPT_SOUND_PARTY_2_LOOP_L:
		maxDist = 6400.f;
		m_sQueueSample.m_fSoundIntensity = 80.0f;
		m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_PARTY_2;
		m_sQueueSample.m_bBankIndex = 0;
		emittingVolume = maxVolume;
		m_sQueueSample.m_nFrequency =
		    SampleManager.GetSampleBaseFrequency(AUDIO_SAMPLE_PARTY_2);
		m_sQueueSample.field_16 = 3;
		m_sQueueSample.field_76 = 3;
		m_sQueueSample.field_48 = 2.0f;
		break;
	case SCRIPT_SOUND_PARTY_3_LOOP_S:
		maxDist = 900.f;
		m_sQueueSample.m_fSoundIntensity = 30.0f;
		m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_PARTY_3;
		m_sQueueSample.m_bBankIndex = 0;
		emittingVolume = maxVolume;
		m_sQueueSample.m_nFrequency =
		    SampleManager.GetSampleBaseFrequency(AUDIO_SAMPLE_PARTY_3);
		m_sQueueSample.field_16 = 3;
		m_sQueueSample.field_76 = 3;
		m_sQueueSample.field_48 = 2.0f;
		break;
	case SCRIPT_SOUND_PARTY_3_LOOP_L:
		maxDist = 6400.f;
		m_sQueueSample.m_fSoundIntensity = 80.0f;
		m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_PARTY_3;
		m_sQueueSample.m_bBankIndex = 0;
		emittingVolume = maxVolume;
		m_sQueueSample.m_nFrequency =
		    SampleManager.GetSampleBaseFrequency(AUDIO_SAMPLE_PARTY_3);
		m_sQueueSample.field_16 = 3;
		m_sQueueSample.field_76 = 3;
		m_sQueueSample.field_48 = 2.0f;
		break;
	case SCRIPT_SOUND_PARTY_4_LOOP_S:
		maxDist = 900.f;
		m_sQueueSample.m_fSoundIntensity = 30.0f;
		m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_PARTY_4;
		m_sQueueSample.m_bBankIndex = 0;
		emittingVolume = maxVolume;
		m_sQueueSample.m_nFrequency =
		    SampleManager.GetSampleBaseFrequency(AUDIO_SAMPLE_PARTY_4);
		m_sQueueSample.field_16 = 3;
		m_sQueueSample.field_76 = 3;
		m_sQueueSample.field_48 = 2.0f;
		break;
	case SCRIPT_SOUND_PARTY_4_LOOP_L:
		maxDist = 6400.f;
		m_sQueueSample.m_fSoundIntensity = 80.0f;
		m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_PARTY_4;
		m_sQueueSample.m_bBankIndex = 0;
		emittingVolume = maxVolume;
		m_sQueueSample.m_nFrequency =
		    SampleManager.GetSampleBaseFrequency(AUDIO_SAMPLE_PARTY_4);
		m_sQueueSample.field_16 = 3;
		m_sQueueSample.field_76 = 3;
		m_sQueueSample.field_48 = 2.0f;
		break;
	case SCRIPT_SOUND_PARTY_5_LOOP_S:
		maxDist = 900.f;
		m_sQueueSample.m_fSoundIntensity = 30.0f;
		m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_PARTY_5;
		m_sQueueSample.m_bBankIndex = 0;
		emittingVolume = maxVolume;
		m_sQueueSample.m_nFrequency =
		    SampleManager.GetSampleBaseFrequency(AUDIO_SAMPLE_PARTY_5);
		m_sQueueSample.field_16 = 3;
		m_sQueueSample.field_76 = 3;
		m_sQueueSample.field_48 = 2.0f;
		break;
	case SCRIPT_SOUND_PARTY_5_LOOP_L:
		maxDist = 6400.f;
		m_sQueueSample.m_fSoundIntensity = 80.0f;
		m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_PARTY_5;
		m_sQueueSample.m_bBankIndex = 0;
		emittingVolume = maxVolume;
		m_sQueueSample.m_nFrequency =
		    SampleManager.GetSampleBaseFrequency(AUDIO_SAMPLE_PARTY_5);
		m_sQueueSample.field_16 = 3;
		m_sQueueSample.field_76 = 3;
		m_sQueueSample.field_48 = 2.0f;
		break;
	case SCRIPT_SOUND_PARTY_6_LOOP_S:
		maxDist = 900.f;
		m_sQueueSample.m_fSoundIntensity = 30.0f;
		m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_PARTY_6;
		m_sQueueSample.m_bBankIndex = 0;
		emittingVolume = maxVolume;
		m_sQueueSample.m_nFrequency =
		    SampleManager.GetSampleBaseFrequency(AUDIO_SAMPLE_PARTY_6);
		m_sQueueSample.field_16 = 3;
		m_sQueueSample.field_76 = 3;
		m_sQueueSample.field_48 = 2.0f;
		break;
	case SCRIPT_SOUND_PARTY_6_LOOP_L:
		maxDist = 6400.f;
		m_sQueueSample.m_fSoundIntensity = 80.0f;
		m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_PARTY_6;
		m_sQueueSample.m_bBankIndex = 0;
		emittingVolume = maxVolume;
		m_sQueueSample.m_nFrequency =
		    SampleManager.GetSampleBaseFrequency(AUDIO_SAMPLE_PARTY_6);
		m_sQueueSample.field_16 = 3;
		m_sQueueSample.field_76 = 3;
		m_sQueueSample.field_48 = 2.0f;
		break;
	case SCRIPT_SOUND_PARTY_7_LOOP_S:
		maxDist = 900.f;
		m_sQueueSample.m_fSoundIntensity = 30.0f;
		m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_PARTY_7;
		m_sQueueSample.m_bBankIndex = 0;
		emittingVolume = maxVolume;
		m_sQueueSample.m_nFrequency =
		    SampleManager.GetSampleBaseFrequency(AUDIO_SAMPLE_PARTY_7);
		m_sQueueSample.field_16 = 3;
		m_sQueueSample.field_76 = 3;
		m_sQueueSample.field_48 = 2.0f;
		break;
	case SCRIPT_SOUND_PARTY_7_LOOP_L:
		maxDist = 6400.f;
		m_sQueueSample.m_fSoundIntensity = 80.0f;
		m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_PARTY_7;
		m_sQueueSample.m_bBankIndex = 0;
		emittingVolume = maxVolume;
		m_sQueueSample.m_nFrequency =
		    SampleManager.GetSampleBaseFrequency(AUDIO_SAMPLE_PARTY_7);
		m_sQueueSample.field_16 = 3;
		m_sQueueSample.field_76 = 3;
		m_sQueueSample.field_48 = 2.0f;
		break;
	case SCRIPT_SOUND_PARTY_8_LOOP_S:
		maxDist = 900.f;
		m_sQueueSample.m_fSoundIntensity = 30.0f;
		m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_PARTY_8;
		m_sQueueSample.m_bBankIndex = 0;
		emittingVolume = maxVolume;
		m_sQueueSample.m_nFrequency =
		    SampleManager.GetSampleBaseFrequency(AUDIO_SAMPLE_PARTY_8);
		m_sQueueSample.field_16 = 3;
		m_sQueueSample.field_76 = 3;
		m_sQueueSample.field_48 = 2.0f;
		break;
	case SCRIPT_SOUND_PARTY_8_LOOP_L:
		maxDist = 6400.f;
		m_sQueueSample.m_fSoundIntensity = 80.0f;
		m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_PARTY_8;
		m_sQueueSample.m_bBankIndex = 0;
		emittingVolume = maxVolume;
		m_sQueueSample.m_nFrequency =
		    SampleManager.GetSampleBaseFrequency(AUDIO_SAMPLE_PARTY_8);
		m_sQueueSample.field_16 = 3;
		m_sQueueSample.field_76 = 3;
		m_sQueueSample.field_48 = 2.0f;
		break;
	case SCRIPT_SOUND_PARTY_9_LOOP_S:
		maxDist = 900.f;
		m_sQueueSample.m_fSoundIntensity = 30.0f;
		m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_PARTY_9;
		m_sQueueSample.m_bBankIndex = 0;
		emittingVolume = maxVolume;
		m_sQueueSample.m_nFrequency =
		    SampleManager.GetSampleBaseFrequency(AUDIO_SAMPLE_PARTY_9);
		m_sQueueSample.field_16 = 3;
		m_sQueueSample.field_76 = 3;
		m_sQueueSample.field_48 = 2.0f;
		break;
	case SCRIPT_SOUND_PARTY_9_LOOP_L:
		maxDist = 6400.f;
		m_sQueueSample.m_fSoundIntensity = 80.0f;
		m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_PARTY_9;
		m_sQueueSample.m_bBankIndex = 0;
		emittingVolume = maxVolume;
		m_sQueueSample.m_nFrequency =
		    SampleManager.GetSampleBaseFrequency(AUDIO_SAMPLE_PARTY_9);
		m_sQueueSample.field_16 = 3;
		m_sQueueSample.field_76 = 3;
		m_sQueueSample.field_48 = 2.0f;
		break;
	case SCRIPT_SOUND_PARTY_10_LOOP_S:
		maxDist = 900.f;
		m_sQueueSample.m_fSoundIntensity = 30.0f;
		m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_PARTY_10;
		m_sQueueSample.m_bBankIndex = 0;
		emittingVolume = maxVolume;
		m_sQueueSample.m_nFrequency =
		    SampleManager.GetSampleBaseFrequency(AUDIO_SAMPLE_PARTY_10);
		m_sQueueSample.field_16 = 3;
		m_sQueueSample.field_76 = 3;
		m_sQueueSample.field_48 = 2.0f;
		break;
	case SCRIPT_SOUND_PARTY_10_LOOP_L:
		maxDist = 6400.f;
		m_sQueueSample.m_fSoundIntensity = 80.0f;
		m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_PARTY_10;
		m_sQueueSample.m_bBankIndex = 0;
		emittingVolume = maxVolume;
		m_sQueueSample.m_nFrequency =
		    SampleManager.GetSampleBaseFrequency(AUDIO_SAMPLE_PARTY_10);
		m_sQueueSample.field_16 = 3;
		m_sQueueSample.field_76 = 3;
		m_sQueueSample.field_48 = 2.0f;
		break;
	case SCRIPT_SOUND_PARTY_11_LOOP_S:
		maxDist = 900.f;
		m_sQueueSample.m_fSoundIntensity = 30.0f;
		m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_PARTY_11;
		m_sQueueSample.m_bBankIndex = 0;
		emittingVolume = maxVolume;
		m_sQueueSample.m_nFrequency =
		    SampleManager.GetSampleBaseFrequency(AUDIO_SAMPLE_PARTY_11);
		m_sQueueSample.field_16 = 3;
		m_sQueueSample.field_76 = 3;
		m_sQueueSample.field_48 = 2.0f;
		break;
	case SCRIPT_SOUND_PARTY_11_LOOP_L:
		maxDist = 6400.f;
		m_sQueueSample.m_fSoundIntensity = 80.0f;
		m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_PARTY_11;
		m_sQueueSample.m_bBankIndex = 0;
		emittingVolume = maxVolume;
		m_sQueueSample.m_nFrequency =
		    SampleManager.GetSampleBaseFrequency(AUDIO_SAMPLE_PARTY_11);
		m_sQueueSample.field_16 = 3;
		m_sQueueSample.field_76 = 3;
		m_sQueueSample.field_48 = 2.0f;
		break;
	case SCRIPT_SOUND_PARTY_12_LOOP_S:
		maxDist = 900.f;
		m_sQueueSample.m_fSoundIntensity = 30.0f;
		m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_PARTY_12;
		m_sQueueSample.m_bBankIndex = 0;
		emittingVolume = maxVolume;
		m_sQueueSample.m_nFrequency =
		    SampleManager.GetSampleBaseFrequency(AUDIO_SAMPLE_PARTY_12);
		m_sQueueSample.field_16 = 3;
		m_sQueueSample.field_76 = 3;
		m_sQueueSample.field_48 = 2.0f;
		break;
	case SCRIPT_SOUND_PARTY_12_LOOP_L:
		maxDist = 6400.f;
		m_sQueueSample.m_fSoundIntensity = 80.0f;
		m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_PARTY_12;
		m_sQueueSample.m_bBankIndex = 0;
		emittingVolume = maxVolume;
		m_sQueueSample.m_nFrequency =
		    SampleManager.GetSampleBaseFrequency(AUDIO_SAMPLE_PARTY_12);
		m_sQueueSample.field_16 = 3;
		m_sQueueSample.field_76 = 3;
		m_sQueueSample.field_48 = 2.0f;
		break;
	case SCRIPT_SOUND_PARTY_13_LOOP_S:
		maxDist = 900.f;
		m_sQueueSample.m_fSoundIntensity = 30.0f;
		m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_PARTY_13;
		m_sQueueSample.m_bBankIndex = 0;
		emittingVolume = maxVolume;
		m_sQueueSample.m_nFrequency =
		    SampleManager.GetSampleBaseFrequency(AUDIO_SAMPLE_PARTY_13);
		m_sQueueSample.field_16 = 3;
		m_sQueueSample.field_76 = 3;
		m_sQueueSample.field_48 = 2.0f;
		break;
	case SCRIPT_SOUND_PARTY_13_LOOP_L:
		maxDist = 6400.f;
		m_sQueueSample.m_fSoundIntensity = 80.0f;
		m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_PARTY_13;
		m_sQueueSample.m_bBankIndex = 0;
		emittingVolume = maxVolume;
		m_sQueueSample.m_nFrequency =
		    SampleManager.GetSampleBaseFrequency(AUDIO_SAMPLE_PARTY_13);
		m_sQueueSample.field_16 = 3;
		m_sQueueSample.field_76 = 3;
		m_sQueueSample.field_48 = 2.0f;
		break;
	case SCRIPT_SOUND_STRIP_CLUB_LOOP_1_S:
		maxDist = 900.f;
		m_sQueueSample.m_fSoundIntensity = 30.0f;
		m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_STRIP_CLUB_1;
		m_sQueueSample.m_bBankIndex = 0;
		emittingVolume = maxVolume;
		m_sQueueSample.m_nFrequency =
		    SampleManager.GetSampleBaseFrequency(AUDIO_SAMPLE_STRIP_CLUB_1);
		m_sQueueSample.field_16 = 3;
		m_sQueueSample.field_76 = 3;
		m_sQueueSample.field_48 = 2.0f;
		break;
	case SCRIPT_SOUND_STRIP_CLUB_LOOP_1_L:
		maxDist = 6400.f;
		m_sQueueSample.m_fSoundIntensity = 80.0f;
		m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_STRIP_CLUB_1;
		m_sQueueSample.m_bBankIndex = 0;
		emittingVolume = maxVolume;
		m_sQueueSample.m_nFrequency =
		    SampleManager.GetSampleBaseFrequency(AUDIO_SAMPLE_STRIP_CLUB_1);
		m_sQueueSample.field_16 = 3;
		m_sQueueSample.field_76 = 3;
		m_sQueueSample.field_48 = 2.0f;
		break;
	case SCRIPT_SOUND_STRIP_CLUB_LOOP_2_S:
		maxDist = 900.f;
		m_sQueueSample.m_fSoundIntensity = 30.0f;
		m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_STRIP_CLUB_2;
		m_sQueueSample.m_bBankIndex = 0;
		emittingVolume = maxVolume;
		m_sQueueSample.m_nFrequency =
		    SampleManager.GetSampleBaseFrequency(AUDIO_SAMPLE_STRIP_CLUB_2);
		m_sQueueSample.field_16 = 3;
		m_sQueueSample.field_76 = 3;
		m_sQueueSample.field_48 = 2.0f;
		break;
	case SCRIPT_SOUND_STRIP_CLUB_LOOP_2_L:
		maxDist = 6400.f;
		m_sQueueSample.m_fSoundIntensity = 80.0f;
		m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_STRIP_CLUB_2;
		m_sQueueSample.m_bBankIndex = 0;
		emittingVolume = maxVolume;
		m_sQueueSample.m_nFrequency =
		    SampleManager.GetSampleBaseFrequency(AUDIO_SAMPLE_STRIP_CLUB_2);
		m_sQueueSample.field_16 = 3;
		m_sQueueSample.field_76 = 3;
		m_sQueueSample.field_48 = 2.0f;
		break;
	case SCRIPT_SOUND_WORK_SHOP_LOOP_S:
	case SCRIPT_SOUND_WORK_SHOP_LOOP_L: ProcessWorkShopScriptObject(sound); return;
	case SCRIPT_SOUND_SAWMILL_LOOP_S:
	case SCRIPT_SOUND_SAWMILL_LOOP_L: ProcessSawMillScriptObject(sound); return;
	case SCRIPT_SOUND_38:
		maxDist = 900.f;
		m_sQueueSample.m_fSoundIntensity = 30.0f;
		m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_409;
		m_sQueueSample.m_bBankIndex = 0;
		emittingVolume = 110;
		m_sQueueSample.m_nFrequency =
		    SampleManager.GetSampleBaseFrequency(AUDIO_SAMPLE_409);
		m_sQueueSample.field_16 = 6;
		m_sQueueSample.field_76 = 3;
		m_sQueueSample.field_48 = 2.0f;
		break;
	case SCRIPT_SOUND_39:
		maxDist = 6400.f;
		m_sQueueSample.m_fSoundIntensity = 80.0f;
		m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_409;
		m_sQueueSample.m_bBankIndex = 0;
		emittingVolume = 110;
		m_sQueueSample.m_nFrequency =
		    SampleManager.GetSampleBaseFrequency(AUDIO_SAMPLE_409);
		m_sQueueSample.field_16 = 6;
		m_sQueueSample.field_76 = 3;
		m_sQueueSample.field_48 = 2.0f;
		break;
	case SCRIPT_SOUND_LAUNDERETTE_LOOP_S:
	case SCRIPT_SOUND_LAUNDERETTE_LOOP_L: ProcessLaunderetteScriptObject(sound); return;
	case SCRIPT_SOUND_CHINATOWN_RESTAURANT_S:
		maxDist = 900.f;
		m_sQueueSample.m_fSoundIntensity = 30.0f;
		m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_CHINATOWN_RESTAURANT;
		m_sQueueSample.m_bBankIndex = 0;
		emittingVolume = 110;
		m_sQueueSample.m_nFrequency =
		    SampleManager.GetSampleBaseFrequency(AUDIO_SAMPLE_CHINATOWN_RESTAURANT);
		m_sQueueSample.field_16 = 3;
		m_sQueueSample.field_76 = 3;
		m_sQueueSample.field_48 = 2.0f;
		break;
	case SCRIPT_SOUND_CHINATOWN_RESTAURANT_L:
		maxDist = 6400.f;
		m_sQueueSample.m_fSoundIntensity = 80.0f;
		m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_CHINATOWN_RESTAURANT;
		m_sQueueSample.m_bBankIndex = 0;
		emittingVolume = 110;
		m_sQueueSample.m_nFrequency =
		    SampleManager.GetSampleBaseFrequency(AUDIO_SAMPLE_CHINATOWN_RESTAURANT);
		m_sQueueSample.field_16 = 3;
		m_sQueueSample.field_76 = 3;
		m_sQueueSample.field_48 = 2.0f;
		break;
	case SCRIPT_SOUND_CIPRIANI_RESAURANT_S:
		maxDist = 900.f;
		m_sQueueSample.m_fSoundIntensity = 30.0f;
		m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_CIPRIANI_RESTAURANT;
		m_sQueueSample.m_bBankIndex = 0;
		emittingVolume = 110;
		m_sQueueSample.m_nFrequency =
		    SampleManager.GetSampleBaseFrequency(AUDIO_SAMPLE_CIPRIANI_RESTAURANT);
		m_sQueueSample.field_16 = 3;
		m_sQueueSample.field_76 = 3;
		m_sQueueSample.field_48 = 2.0f;
		break;
	case SCRIPT_SOUND_CIPRIANI_RESAURANT_L:
		maxDist = 6400.f;
		m_sQueueSample.m_fSoundIntensity = 80.0f;
		m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_CIPRIANI_RESTAURANT;
		m_sQueueSample.m_bBankIndex = 0;
		emittingVolume = 110;
		m_sQueueSample.m_nFrequency =
		    SampleManager.GetSampleBaseFrequency(AUDIO_SAMPLE_CIPRIANI_RESTAURANT);
		m_sQueueSample.field_16 = 3;
		m_sQueueSample.field_76 = 3;
		m_sQueueSample.field_48 = 2.0f;
		break;
	case SCRIPT_SOUND_46:
		maxDist = 900.f;
		m_sQueueSample.m_fSoundIntensity = 30.0f;
		m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_414;
		m_sQueueSample.m_bBankIndex = 0;
		emittingVolume = 110;
		m_sQueueSample.m_nFrequency =
		    SampleManager.GetSampleBaseFrequency(AUDIO_SAMPLE_414);
		m_sQueueSample.field_16 = 3;
		m_sQueueSample.field_76 = 3;
		m_sQueueSample.field_48 = 2.0f;
		break;
	case SCRIPT_SOUND_47:
		maxDist = 6400.f;
		m_sQueueSample.m_fSoundIntensity = 80.0f;
		m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_414;
		m_sQueueSample.m_bBankIndex = 0;
		emittingVolume = 110;
		m_sQueueSample.m_nFrequency =
		    SampleManager.GetSampleBaseFrequency(AUDIO_SAMPLE_414);
		m_sQueueSample.field_16 = 3;
		m_sQueueSample.field_76 = 3;
		m_sQueueSample.field_48 = 2.0f;
		break;
	case SCRIPT_SOUND_MARCO_BISTRO_S:
		maxDist = 900.f;
		m_sQueueSample.m_fSoundIntensity = 30.0f;
		m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_MARCO_BISTRO;
		m_sQueueSample.m_bBankIndex = 0;
		emittingVolume = 110;
		m_sQueueSample.m_nFrequency =
		    SampleManager.GetSampleBaseFrequency(AUDIO_SAMPLE_MARCO_BISTRO);
		m_sQueueSample.field_16 = 3;
		m_sQueueSample.field_76 = 3;
		m_sQueueSample.field_48 = 2.0f;
		break;
	case SCRIPT_SOUND_MARCO_BISTRO_L:
		maxDist = 6400.f;
		m_sQueueSample.m_fSoundIntensity = 80.0f;
		m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_MARCO_BISTRO;
		m_sQueueSample.m_bBankIndex = 0;
		emittingVolume = 110;
		m_sQueueSample.m_nFrequency =
		    SampleManager.GetSampleBaseFrequency(AUDIO_SAMPLE_MARCO_BISTRO);
		m_sQueueSample.field_16 = 3;
		m_sQueueSample.field_76 = 3;
		m_sQueueSample.field_48 = 2.0f;
		break;
	case SCRIPT_SOUND_AIRPORT_LOOP_S:
	case SCRIPT_SOUND_AIRPORT_LOOP_L: ProcessAirportScriptObject(sound); return;
	case SCRIPT_SOUND_SHOP_LOOP_S:
	case SCRIPT_SOUND_SHOP_LOOP_L: ProcessShopScriptObject(sound); return;
	case SCRIPT_SOUND_CINEMA_LOOP_S:
	case SCRIPT_SOUND_CINEMA_LOOP_L: ProcessCinemaScriptObject(sound); return;
	case SCRIPT_SOUND_DOCKS_LOOP_S:
	case SCRIPT_SOUND_DOCKS_LOOP_L: ProcessDocksScriptObject(sound); return;
	case SCRIPT_SOUND_HOME_LOOP_S:
	case SCRIPT_SOUND_HOME_LOOP_L: ProcessHomeScriptObject(sound); return;
	case SCRIPT_SOUND_FRANKIE_PIANO:
		maxDist = 900.f;
		m_sQueueSample.m_fSoundIntensity = 30.0f;
		m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_FRANKIE_PIANO;
		m_sQueueSample.m_bBankIndex = 0;
		emittingVolume = maxVolume;
		m_sQueueSample.m_nFrequency =
		    SampleManager.GetSampleBaseFrequency(AUDIO_SAMPLE_FRANKIE_PIANO);
		m_sQueueSample.field_16 = 3;
		m_sQueueSample.field_76 = 3;
		m_sQueueSample.field_48 = 2.0f;
		break;
	case SCRIPT_SOUND_PARTY_1_LOOP:
		maxDist = 6400.f;
		m_sQueueSample.m_fSoundIntensity = 80.0f;
		m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_PARTY_1;
		m_sQueueSample.m_bBankIndex = 0;
		emittingVolume = maxVolume;
		m_sQueueSample.m_nFrequency =
		    SampleManager.GetSampleBaseFrequency(AUDIO_SAMPLE_PARTY_1);
		m_sQueueSample.field_16 = 3;
		m_sQueueSample.field_76 = 3;
		m_sQueueSample.field_48 = 2.0f;
		break;
	case SCRIPT_SOUND_PORN_CINEMA_1_S:
	case SCRIPT_SOUND_PORN_CINEMA_1_L:
	case SCRIPT_SOUND_PORN_CINEMA_2_S:
	case SCRIPT_SOUND_PORN_CINEMA_2_L:
	case SCRIPT_SOUND_PORN_CINEMA_3_S:
	case SCRIPT_SOUND_PORN_CINEMA_3_L:
	case SCRIPT_SOUND_MISTY_SEX_S:
	case SCRIPT_SOUND_MISTY_SEX_L: ProcessPornCinema(sound); return;
	case SCRIPT_SOUND_BANK_ALARM_LOOP_S:
		maxDist = 900.f;
		m_sQueueSample.m_fSoundIntensity = 30.0f;
		m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_BANK_ALARM;
		m_sQueueSample.m_bBankIndex = 0;
		emittingVolume = 90;
		m_sQueueSample.m_nFrequency =
		    SampleManager.GetSampleBaseFrequency(AUDIO_SAMPLE_BANK_ALARM);
		m_sQueueSample.field_16 = 2;
		m_sQueueSample.field_76 = 3;
		m_sQueueSample.field_48 = 2.0f;
		break;
	case SCRIPT_SOUND_BANK_ALARM_LOOP_L:
		maxDist = 6400.f;
		m_sQueueSample.m_fSoundIntensity = 80.0f;
		m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_BANK_ALARM;
		m_sQueueSample.m_bBankIndex = 0;
		emittingVolume = 90;
		m_sQueueSample.m_nFrequency =
		    SampleManager.GetSampleBaseFrequency(AUDIO_SAMPLE_BANK_ALARM);
		m_sQueueSample.field_16 = 2;
		m_sQueueSample.field_76 = 3;
		m_sQueueSample.field_48 = 2.0f;
		break;
	case SCRIPT_SOUND_POLICE_BALL_LOOP_S:
		maxDist = 900.f;
		m_sQueueSample.m_fSoundIntensity = 30.0f;
		m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_POLICE_BALL;
		m_sQueueSample.m_bBankIndex = 0;
		emittingVolume = maxVolume;
		m_sQueueSample.m_nFrequency =
		    SampleManager.GetSampleBaseFrequency(AUDIO_SAMPLE_POLICE_BALL);
		m_sQueueSample.field_16 = 2;
		m_sQueueSample.field_76 = 3;
		m_sQueueSample.field_48 = 2.0f;
		break;
	case SCRIPT_SOUND_POLICE_BALL_LOOP_L:
		maxDist = 6400.f;
		m_sQueueSample.m_fSoundIntensity = 80.0f;
		m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_POLICE_BALL;
		m_sQueueSample.m_bBankIndex = 0;
		emittingVolume = maxVolume;
		m_sQueueSample.m_nFrequency =
		    SampleManager.GetSampleBaseFrequency(AUDIO_SAMPLE_POLICE_BALL);
		m_sQueueSample.field_16 = 2;
		m_sQueueSample.field_76 = 3;
		m_sQueueSample.field_48 = 2.0f;
		break;
	case SCRIPT_SOUND_RAVE_LOOP_INDUSTRIAL_S:
		maxDist = 900.f;
		m_sQueueSample.m_fSoundIntensity = 30.0f;
		m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_RAVE_INDUSTRIAL;
		m_sQueueSample.m_bBankIndex = 0;
		emittingVolume = maxVolume;
		m_sQueueSample.m_nFrequency =
		    SampleManager.GetSampleBaseFrequency(AUDIO_SAMPLE_RAVE_INDUSTRIAL);
		m_sQueueSample.field_16 = 3;
		m_sQueueSample.field_76 = 3;
		m_sQueueSample.field_48 = 2.0f;
		break;
	case SCRIPT_SOUND_RAVE_LOOP_INDUSTRIAL_L:
		maxDist = 6400.f;
		m_sQueueSample.m_fSoundIntensity = 80.0f;
		m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_RAVE_INDUSTRIAL;
		m_sQueueSample.m_bBankIndex = 0;
		emittingVolume = maxVolume;
		m_sQueueSample.m_nFrequency =
		    SampleManager.GetSampleBaseFrequency(AUDIO_SAMPLE_RAVE_INDUSTRIAL);
		m_sQueueSample.field_16 = 3;
		m_sQueueSample.field_76 = 3;
		m_sQueueSample.field_48 = 2.0f;
		break;
	case SCRIPT_SOUND_POLICE_CELL_BEATING_LOOP_S:
	case SCRIPT_SOUND_POLICE_CELL_BEATING_LOOP_L:
		ProcessPoliceCellBeatingScriptObject(sound);
		return;
	case SCRIPT_SOUND_RAVE_1_LOOP_S:
	case SCRIPT_SOUND_RAVE_2_LOOP_S:
		maxDist = 900.f;
		m_sQueueSample.m_fSoundIntensity = 30.0f;
		m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_RAVE_1;
		m_sQueueSample.m_bBankIndex = 0;
		emittingVolume = maxVolume;
		m_sQueueSample.m_nFrequency =
		    SampleManager.GetSampleBaseFrequency(m_sQueueSample.m_nSampleIndex);
		m_sQueueSample.field_16 = 3;
		m_sQueueSample.field_76 = 3;
		m_sQueueSample.field_48 = 2.0f;
		break;
	case SCRIPT_SOUND_RAVE_1_LOOP_L:
	case SCRIPT_SOUND_RAVE_2_LOOP_L:
		maxDist = 6400.f;
		m_sQueueSample.m_fSoundIntensity = 80.0f;
		m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_RAVE_1;
		m_sQueueSample.m_bBankIndex = 0;
		emittingVolume = maxVolume;
		m_sQueueSample.m_nFrequency =
		    SampleManager.GetSampleBaseFrequency(m_sQueueSample.m_nSampleIndex);
		m_sQueueSample.field_16 = 3;
		m_sQueueSample.field_76 = 3;
		m_sQueueSample.field_48 = 2.0f;
		break;
	case SCRIPT_SOUND_RAVE_3_LOOP_S:
		maxDist = 900.f;
		m_sQueueSample.m_fSoundIntensity = 30.0f;
		m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_RAVE_2;
		m_sQueueSample.m_bBankIndex = 0;
		emittingVolume = maxVolume;
		m_sQueueSample.m_nFrequency =
		    SampleManager.GetSampleBaseFrequency(AUDIO_SAMPLE_RAVE_2);
		m_sQueueSample.field_16 = 3;
		m_sQueueSample.field_76 = 3;
		m_sQueueSample.field_48 = 2.0f;
		break;
	case SCRIPT_SOUND_RAVE_3_LOOP_L:
		maxDist = 6400.f;
		m_sQueueSample.m_fSoundIntensity = 80.0f;
		m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_RAVE_2;
		m_sQueueSample.m_bBankIndex = 0;
		emittingVolume = maxVolume;
		m_sQueueSample.m_nFrequency =
		    SampleManager.GetSampleBaseFrequency(AUDIO_SAMPLE_RAVE_2);
		m_sQueueSample.field_16 = 3;
		m_sQueueSample.field_76 = 3;
		m_sQueueSample.field_48 = 2.0f;
		break;
	case SCRIPT_SOUND_PRETEND_FIRE_LOOP:
		maxDist = 2500.f;
		m_sQueueSample.m_fSoundIntensity = 50.0f;
		m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_FIRE_ENTITY;
		m_sQueueSample.m_bBankIndex = 0;
		emittingVolume = 80;
		m_sQueueSample.m_nFrequency =
		    SampleManager.GetSampleBaseFrequency(AUDIO_SAMPLE_FIRE_ENTITY);
		m_sQueueSample.field_16 = 8;
		m_sQueueSample.field_76 = 10;
		m_sQueueSample.field_48 = 2.0f;
		break;
	default: return;
	}

	distSquared = GetDistanceSquared(&m_sQueueSample.m_vecPos);
	if(distSquared < maxDist) {
		m_sQueueSample.m_fDistance = Sqrt(distSquared);
		m_sQueueSample.m_bVolume = ComputeVolume(
		    emittingVolume, m_sQueueSample.m_fSoundIntensity, m_sQueueSample.m_fDistance);
		if(m_sQueueSample.m_bVolume) {
			m_sQueueSample.m_counter = 0;
			m_sQueueSample.m_bIsDistant = 0;
			m_sQueueSample.m_nLoopCount = 0;
			m_sQueueSample.field_56 = 0;
			m_sQueueSample.m_bReverbFlag = 1;
			m_sQueueSample.m_bEmittingVolume = emittingVolume;
			m_sQueueSample.m_nLoopStart =
			    SampleManager.GetSampleLoopStartOffset(m_sQueueSample.m_nSampleIndex);
			m_sQueueSample.m_nLoopEnd =
			    SampleManager.GetSampleLoopEndOffset(m_sQueueSample.m_nSampleIndex);
			m_sQueueSample.m_bRequireReflection = 0;
			AddSampleToRequestedQueue();
		}
	}
}

void
cAudioManager::ProcessMissionAudio()
{
	float dist;
	uint8 emittingVol;
	uint8 pan;
	float distSquared;
	CVector vec;

	static uint8 nCheckPlayingDelay = 0;
	static uint8 nFramesUntilFailedLoad = 0;
	static uint8 nFramesForPretendPlaying = 0;

	if(m_bIsInitialised) {
		if(m_sMissionAudio.m_nSampleIndex != 3033) {
			switch(m_sMissionAudio.m_bLoadingStatus) {
			case 0:
				SampleManager.PreloadStreamedFile(m_sMissionAudio.m_nSampleIndex,
				                                  1u);
				m_sMissionAudio.m_bLoadingStatus = 1;
				nFramesUntilFailedLoad = 0;
				return;
			case 1:
				if(!m_sMissionAudio.m_bIsPlayed) return;
				if(g_bMissionAudioLoadFailed) {
					if(m_bTimerJustReset) {
						ClearMissionAudio();
						SampleManager.StopStreamedFile(1u);
						nFramesForPretendPlaying = 0;
						nCheckPlayingDelay = 0;
						nFramesUntilFailedLoad = 0;
					} else if(!m_bUserPause) {
						if(++nFramesForPretendPlaying < 120u) {
							m_sMissionAudio.m_bPlayStatus = 1;
						} else {
							m_sMissionAudio.m_bPlayStatus = 2;
							m_sMissionAudio.m_nSampleIndex = 3033;
						}
					}
				} else {
					if(m_sMissionAudio.m_bPlayStatus) {
						if(m_sMissionAudio.m_bPlayStatus != 1) return;
						if(m_bTimerJustReset) {
							ClearMissionAudio();
							SampleManager.StopStreamedFile(1u);
							return;
						}
						if(MissionScriptAudioUsesPoliceChannel(
						       m_sMissionAudio.m_nSampleIndex)) {
							if(!m_bUserPause) {
								if(nCheckPlayingDelay) {
									--nCheckPlayingDelay;
								} else if(
								    GetMissionScriptPoliceAudioPlayingStatus() ==
								        2 ||
								    --m_sMissionAudio.field_24) {
									m_sMissionAudio
									    .m_bPlayStatus = 2;
									m_sMissionAudio
									    .m_nSampleIndex = 3033;
									SampleManager
									    .StopStreamedFile(1u);
									m_sMissionAudio.field_24 =
									    0;
								}
							}
						} else if(m_sMissionAudio.field_22) {
							if(SampleManager.IsStreamPlaying(1u) ||
							   m_bUserPause || m_bPreviousUserPause) {
								if(m_bUserPause)
									SampleManager.PauseStream(
									    1, 1u);
								else
									SampleManager.PauseStream(
									    0, 1u);
							} else {
								m_sMissionAudio.m_bPlayStatus = 2;
								m_sMissionAudio.m_nSampleIndex =
								    3033;
								SampleManager.StopStreamedFile(1u);
								m_sMissionAudio.field_24 = 0;
							}
						} else {
							if(m_bUserPause) return;
							if(nCheckPlayingDelay--) {
								if(!SampleManager.IsStreamPlaying(
								       1u))
									return;
								nCheckPlayingDelay = 0;
							}
							m_sMissionAudio.field_22 = 1;
						}
					} else {
						if(MissionScriptAudioUsesPoliceChannel(
						       m_sMissionAudio.m_nSampleIndex)) {
							SetMissionScriptPoliceAudio(
							    m_sMissionAudio.m_nSampleIndex);
						} else {
							if(m_bUserPause)
								SampleManager.PauseStream(1, 1u);
							if(m_sMissionAudio.field_12) {
								SampleManager
								    .SetStreamedVolumeAndPan(
								        80u, 63u, 1, 1u);
							} else {
								distSquared = GetDistanceSquared(
								    &m_sMissionAudio.m_vecPos);
								if(distSquared >= 2500.f) {
									emittingVol = 0;
									pan = 63;
								} else {
									dist = Sqrt(distSquared);
									emittingVol = ComputeVolume(
									    80u, 50.0f, dist);
									TranslateEntity(
									    &m_sMissionAudio
									         .m_vecPos,
									    &vec);
									pan =
									    ComputePan(50.f, &vec);
								}
								SampleManager
								    .SetStreamedVolumeAndPan(
								        emittingVol, pan, 1, 1u);
							}
							SampleManager.StartPreloadedStreamedFile(
							    1u);
						}
						m_sMissionAudio.m_bPlayStatus = 1;
						nCheckPlayingDelay = 30;
					}
				}
				break;
			case 2:
				if(++nFramesUntilFailedLoad >= 90u) {
					nFramesForPretendPlaying = 0;
					g_bMissionAudioLoadFailed = 1;
					nFramesUntilFailedLoad = 0;
					m_sMissionAudio.m_bLoadingStatus = 1;
				}
				return;
			default: return;
			}
		}
	}
}

void
cAudioManager::ProcessModelCarEngine(cVehicleParams *params)
{
	cAudioManager *v2;
	CAutomobile *automobile;
	float allowedVelocity;
	int32 emittingVol;
	float velocityChange;

	v2 = this;
	if(params->m_fDistance < 900.f) {
		automobile = (CAutomobile *)params->m_pVehicle;
		if(automobile->bEngineOn) {
			if(automobile->m_nWheelsOnGround) {
				velocityChange = Abs(params->m_fVelocityChange);
			} else {
				if(automobile->m_nDriveWheelsOnGround)
					automobile->m_fGasPedalAudio =
					    automobile->m_fGasPedalAudio * 0.4f;
				velocityChange = automobile->m_fGasPedalAudio *
				                 params->m_pTransmission->fMaxVelocity;
			}
			if(velocityChange > 0.001f) {
				allowedVelocity = 0.5f * params->m_pTransmission->fMaxVelocity;
				if(velocityChange < allowedVelocity)
					emittingVol = (90.f * velocityChange / allowedVelocity);
				else
					emittingVol = 90;
				if(emittingVol) {
					CalculateDistance((bool *)params, params->m_fDistance);
					m_sQueueSample.m_bVolume = ComputeVolume(
					    emittingVol, 30.f, m_sQueueSample.m_fDistance);
					if(m_sQueueSample.m_bVolume) {
						m_sQueueSample.m_counter = 2;
						m_sQueueSample.m_nSampleIndex =
						    AUDIO_SAMPLE_MODEL_VEHICLE_ACCELERATING;
						m_sQueueSample.m_bBankIndex = 0;
						m_sQueueSample.m_bIsDistant = 0;
						m_sQueueSample.field_16 = 1;
						m_sQueueSample.m_nFrequency =
						    (11025.f * velocityChange /
						         params->m_pTransmission->fMaxVelocity +
						     11025.f);
						m_sQueueSample.m_nLoopCount = 0;
						m_sQueueSample.m_bEmittingVolume = emittingVol;
						m_sQueueSample.m_nLoopStart =
						    SampleManager.GetSampleLoopStartOffset(
						        m_sQueueSample.m_nSampleIndex);
						m_sQueueSample.m_nLoopEnd =
						    SampleManager.GetSampleLoopEndOffset(
						        m_sQueueSample.m_nSampleIndex);
						m_sQueueSample.field_48 = 3.0f;
						m_sQueueSample.m_fSoundIntensity = 30.0f;
						m_sQueueSample.field_56 = 0;
						m_sQueueSample.field_76 = 3;
						m_sQueueSample.m_bReverbFlag = 1;
						m_sQueueSample.m_bRequireReflection = 0;
						AddSampleToRequestedQueue();
					}
				}
			}
		}
	}
}

void
cAudioManager::ProcessOneShotScriptObject(uint8 sound)
{
	CPlayerPed *playerPed;
	uint8 emittingVolume;
	float maxDist;
	float distSquared;

	cPedParams male;
	cPedParams female;

	static uint8 iSound = 0;

	switch(sound) {
	case SCRIPT_SOUND_INJURED_PED_MALE_OUCH_S:
	case SCRIPT_SOUND_INJURED_PED_MALE_OUCH_L:
		male.m_pPed = nil;
		male.m_bDistanceCalculated = 0;
		male.m_fDistance = GetDistanceSquared(&m_sQueueSample.m_vecPos);
		SetupPedComments(&male, SOUND_INJURED_PED_MALE_OUCH);
		return;
	case SCRIPT_SOUND_INJURED_PED_FEMALE_OUCH_S:
	case SCRIPT_SOUND_INJURED_PED_FEMALE_OUCH_L:
		female.m_pPed = nil;
		female.m_bDistanceCalculated = 0;
		female.m_fDistance = GetDistanceSquared(&m_sQueueSample.m_vecPos);
		SetupPedComments(&female, SOUND_INJURED_PED_FEMALE);
		return;
	case SCRIPT_SOUND_GATE_START_CLUNK:
	case SCRIPT_SOUND_GATE_STOP_CLUNK:
		maxDist = 1600.f;
		m_sQueueSample.m_fSoundIntensity = 40.0f;
		m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_COLLISION_GATE;
		m_sQueueSample.m_bBankIndex = 0;
		if(sound == SCRIPT_SOUND_GATE_START_CLUNK)
			m_sQueueSample.m_nFrequency = 10600;
		else
			m_sQueueSample.m_nFrequency = 9000;
		m_sQueueSample.field_16 = 1;
		m_sQueueSample.field_48 = 0.0f;
		m_sQueueSample.m_bIsDistant = 0;
		m_sQueueSample.m_bRequireReflection = 1;
		emittingVolume = RandomDisplacement(10u) + 50;
		break;
	case SCRIPT_SOUND_BULLET_HIT_GROUND_1:
	case SCRIPT_SOUND_BULLET_HIT_GROUND_2:
	case SCRIPT_SOUND_BULLET_HIT_GROUND_3:
		maxDist = 2500.f;
		m_sQueueSample.m_fSoundIntensity = 50.0f;
		m_sQueueSample.m_nSampleIndex =
		    m_anRandomTable[iSound % 5] % 3u + AUDIO_SAMPLE_BULLET_HIT_GROUND_1;
		m_sQueueSample.m_bBankIndex = 0;
		m_sQueueSample.m_nFrequency =
		    SampleManager.GetSampleBaseFrequency(m_sQueueSample.m_nSampleIndex);
		m_sQueueSample.m_nFrequency += RandomDisplacement(m_sQueueSample.m_nFrequency >> 5);
		m_sQueueSample.field_16 = 9;
		m_sQueueSample.field_48 = 0.0f;
		m_sQueueSample.m_bIsDistant = 0;
		emittingVolume = m_anRandomTable[2] % 20 + 90;
		break;
	case SCRIPT_SOUND_110:
	case SCRIPT_SOUND_111:
		if(SampleManager.IsSampleBankLoaded(0) != 1) return;
		maxDist = 6400.f;
		m_sQueueSample.m_fSoundIntensity = 80.0f;
		m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_389;
		m_sQueueSample.m_bBankIndex = 0;
		emittingVolume = maxVolume;
		m_sQueueSample.m_nFrequency =
		    SampleManager.GetSampleBaseFrequency(AUDIO_SAMPLE_389);
		m_sQueueSample.field_16 = 0;
		m_sQueueSample.field_48 = 2.0f;
		m_sQueueSample.m_bIsDistant = 0;
		break;
	case SCRIPT_SOUND_PAYPHONE_RINGING:
		maxDist = 6400.f;
		m_sQueueSample.m_fSoundIntensity = 80.0f;
		m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_PAYPHONE_RINGING;
		m_sQueueSample.m_bBankIndex = 0;
		emittingVolume = 80;
		m_sQueueSample.m_nFrequency =
		    SampleManager.GetSampleBaseFrequency(AUDIO_SAMPLE_PAYPHONE_RINGING);
		m_sQueueSample.field_16 = 1;
		m_sQueueSample.field_48 = 2.0f;
		m_sQueueSample.m_bIsDistant = 0;
		m_sQueueSample.m_bRequireReflection = 0;
		break;
	case SCRIPT_SOUND_GLASS_BREAK_L:
		maxDist = 3600.f;
		m_sQueueSample.m_fSoundIntensity = 60.0f;
		m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_GLASS_GENERIC_BREAK;
		m_sQueueSample.m_bBankIndex = 0;
		emittingVolume = 70;
		m_sQueueSample.m_nFrequency =
		    SampleManager.GetSampleBaseFrequency(AUDIO_SAMPLE_GLASS_GENERIC_BREAK);
		m_sQueueSample.field_16 = 3;
		m_sQueueSample.field_48 = 0.0f;
		m_sQueueSample.m_bIsDistant = 0;
		break;
	case SCRIPT_SOUND_GLASS_BREAK_S:
		maxDist = 3600.f;
		m_sQueueSample.m_fSoundIntensity = 60.0f;
		m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_GLASS_GENERIC_BREAK;
		m_sQueueSample.m_bBankIndex = 0;
		emittingVolume = 60;
		m_sQueueSample.m_nFrequency =
		    SampleManager.GetSampleBaseFrequency(AUDIO_SAMPLE_GLASS_GENERIC_BREAK);
		m_sQueueSample.field_16 = 3;
		m_sQueueSample.field_48 = 0.0f;
		m_sQueueSample.m_bIsDistant = 0;
		break;
	case SCRIPT_SOUND_GLASS_CRACK:
		maxDist = 3600.f;
		m_sQueueSample.m_fSoundIntensity = 60.0f;
		m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_GLASS_WINDSHIELD_CRACK;
		m_sQueueSample.m_bBankIndex = 0;
		emittingVolume = 70;
		m_sQueueSample.m_nFrequency =
		    SampleManager.GetSampleBaseFrequency(AUDIO_SAMPLE_GLASS_WINDSHIELD_CRACK);
		m_sQueueSample.field_16 = 3;
		m_sQueueSample.field_48 = 0.0f;
		m_sQueueSample.m_bIsDistant = 0;
		m_sQueueSample.m_bRequireReflection = 1;
		break;
	case SCRIPT_SOUND_GLASS_LIGHT_BREAK:
		maxDist = 3025.f;
		m_sQueueSample.m_fSoundIntensity = 55.0f;
		m_sQueueSample.m_nSampleIndex =
		    (m_anRandomTable[4] & 3) + AUDIO_SAMPLE_GLASS_LIGHT_BREAK_1;
		m_sQueueSample.m_bBankIndex = 0;
		m_sQueueSample.m_nFrequency = RandomDisplacement(2000u) + 19000;
		m_sQueueSample.field_16 = 9;
		m_sQueueSample.field_48 = 0.0f;
		m_sQueueSample.m_bIsDistant = 0;
		emittingVolume = RandomDisplacement(11u) + 25;
		break;
	case SCRIPT_SOUND_BOX_DESTROYED_1:
		maxDist = 3600.f;
		m_sQueueSample.m_fSoundIntensity = 60.0f;
		m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_BOX_DESTROYED_1;
		m_sQueueSample.m_bBankIndex = 0;
		m_sQueueSample.m_nFrequency = RandomDisplacement(1500u) + 18600;
		m_sQueueSample.field_16 = 3;
		m_sQueueSample.field_48 = 0.0f;
		m_sQueueSample.m_bIsDistant = 0;
		m_sQueueSample.m_bRequireReflection = 1;
		emittingVolume = m_anRandomTable[2] % 20 + 80;
		break;
	case SCRIPT_SOUND_BOX_DESTROYED_2:
		maxDist = 3600.f;
		m_sQueueSample.m_fSoundIntensity = 60.0f;
		m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_BOX_DESTROYED_2;
		m_sQueueSample.m_bBankIndex = 0;
		m_sQueueSample.m_nFrequency = RandomDisplacement(1500u) + 18600;
		m_sQueueSample.field_16 = 3;
		m_sQueueSample.field_48 = 0.0f;
		m_sQueueSample.m_bIsDistant = 0;
		m_sQueueSample.m_bRequireReflection = 1;
		emittingVolume = m_anRandomTable[2] % 20 + 80;
		break;
	case SCRIPT_SOUND_METAL_COLLISION:
		maxDist = 3600.f;
		m_sQueueSample.m_fSoundIntensity = 60.0f;
		m_sQueueSample.m_nSampleIndex =
		    m_anRandomTable[3] % 5u + AUDIO_SAMPLE_COLLISION_METAL;
		m_sQueueSample.m_bBankIndex = 0;
		m_sQueueSample.m_nFrequency =
		    SampleManager.GetSampleBaseFrequency(m_sQueueSample.m_nSampleIndex);
		m_sQueueSample.m_nFrequency += RandomDisplacement(m_sQueueSample.m_nFrequency >> 4);
		m_sQueueSample.field_16 = 3;
		m_sQueueSample.field_48 = 0.0f;
		m_sQueueSample.m_bIsDistant = 0;
		m_sQueueSample.m_bRequireReflection = 1;
		emittingVolume = m_anRandomTable[2] % 30 + 70;
		break;
	case SCRIPT_SOUND_TIRE_COLLISION:
		maxDist = 3600.f;
		m_sQueueSample.m_fSoundIntensity = 60.0f;
		m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_COLLISION_RUBBER;
		m_sQueueSample.m_bBankIndex = 0;
		m_sQueueSample.m_nFrequency =
		    SampleManager.GetSampleBaseFrequency(m_sQueueSample.m_nSampleIndex);
		m_sQueueSample.m_nFrequency += RandomDisplacement(m_sQueueSample.m_nFrequency >> 4);
		m_sQueueSample.field_16 = 3;
		m_sQueueSample.field_48 = 0.0f;
		m_sQueueSample.m_bIsDistant = 0;
		m_sQueueSample.m_bRequireReflection = 1;
		emittingVolume = m_anRandomTable[2] % 30 + 60;
		break;
	case SCRIPT_SOUND_GUNSHELL_DROP:
		playerPed = FindPlayerPed();
		if(playerPed) {
			switch(playerPed->m_nSurfaceTouched) {
			case SURFACE_GRASS:
			case SURFACE_DIRT:
			case SURFACE_DIRTTRACK:
			case SURFACE_SCAFFOLD:
			case SURFACE_FLESH:
			case SURFACE_SAND:
			case SURFACE_TIRE:
			case SURFACE_HEDGE:
				m_sQueueSample.m_nSampleIndex =
				    AUDIO_SAMPLE_BULLET_GUNSHELL_SOFT_DROP;
				m_sQueueSample.m_nFrequency = RandomDisplacement(500u) + 11000;
				m_sQueueSample.field_16 = 18;
				maxDist = 400.f;
				m_sQueueSample.m_fSoundIntensity = 20.0f;
				m_sQueueSample.m_bBankIndex = 0;
				m_sQueueSample.field_48 = 0.0f;
				m_sQueueSample.m_bIsDistant = 0;
				emittingVolume = m_anRandomTable[2] % 20 + 30;
				distSquared = GetDistanceSquared(&m_sQueueSample.m_vecPos);
				if(distSquared < maxDist) {
					m_sQueueSample.m_fDistance = Sqrt(distSquared);
					m_sQueueSample.m_bVolume = ComputeVolume(
					    emittingVolume, m_sQueueSample.m_fSoundIntensity,
					    m_sQueueSample.m_fDistance);
					if(m_sQueueSample.m_bVolume) {
						m_sQueueSample.m_counter = iSound++;
						m_sQueueSample.m_nLoopCount = 1;
						m_sQueueSample.field_56 = 1;
						m_sQueueSample.m_bEmittingVolume = emittingVolume;
						m_sQueueSample.m_nLoopStart = 0;
						m_sQueueSample.m_nLoopEnd = -1;
						m_sQueueSample.m_bReverbFlag = 1;
						AddSampleToRequestedQueue();
					}
				}
				return;
			case SURFACE_PUDDLE: return;
			default: break;
			}
		}
		m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_BULLET_GUNSHELL_HARD_DROP;
		m_sQueueSample.m_nFrequency = RandomDisplacement(750u) + 18000;
		m_sQueueSample.field_16 = 15;
		maxDist = 400.f;
		m_sQueueSample.m_fSoundIntensity = 20.0f;
		m_sQueueSample.m_bBankIndex = 0;
		m_sQueueSample.field_48 = 0.0f;
		m_sQueueSample.m_bIsDistant = 0;
		emittingVolume = m_anRandomTable[2] % 20 + 30;
		break;
	case SCRIPT_SOUND_GUNSHELL_DROP_SOFT:
		m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_BULLET_GUNSHELL_SOFT_DROP;
		m_sQueueSample.m_nFrequency = RandomDisplacement(500u) + 11000;
		m_sQueueSample.field_16 = 18;
		maxDist = 400.f;
		m_sQueueSample.m_fSoundIntensity = 20.0f;
		m_sQueueSample.m_bBankIndex = 0;
		m_sQueueSample.field_48 = 0.0f;
		m_sQueueSample.m_bIsDistant = 0;
		emittingVolume = m_anRandomTable[2] % 20 + 30;
		break;
	default: return;
	}

	distSquared = GetDistanceSquared(&m_sQueueSample.m_vecPos);
	if(distSquared < maxDist) {
		m_sQueueSample.m_fDistance = Sqrt(distSquared);
		m_sQueueSample.m_bVolume = ComputeVolume(
		    emittingVolume, m_sQueueSample.m_fSoundIntensity, m_sQueueSample.m_fDistance);
		if(m_sQueueSample.m_bVolume) {
			m_sQueueSample.m_counter = iSound++;
			m_sQueueSample.m_nLoopCount = 1;
			m_sQueueSample.field_56 = 1;
			m_sQueueSample.m_bEmittingVolume = emittingVolume;
			m_sQueueSample.m_nLoopStart = 0;
			m_sQueueSample.m_nLoopEnd = -1;
			m_sQueueSample.m_bReverbFlag = 1;
			AddSampleToRequestedQueue();
		}
	}
}

void
cAudioManager::ProcessPed(CPhysical *ped)
{
	cPedParams params;

	params.m_pPed = 0;
	params.m_bDistanceCalculated = 0;
	params.m_fDistance = 0.0f;

	m_sQueueSample.m_vecPos = ped->GetPosition();

	params.m_bDistanceCalculated = 0;
	params.m_pPed = (CPed *)ped;
	params.m_fDistance = GetDistanceSquared(&m_sQueueSample.m_vecPos);
	if(ped->m_modelIndex == MI_FATMALE02) ProcessPedHeadphones(&params);
	ProcessPedOneShots(&params);
}

void
cAudioManager::ProcessPedHeadphones(cPedParams *params)
{
	CPed *ped;
	CVehicle *veh;
	uint8 emittingVol;

	if(params->m_fDistance < 49.f) {
		ped = params->m_pPed;
		if(!ped->bIsAimingGun || ped->m_bodyPartBleeding != 2) {
			CalculateDistance((bool *)params, params->m_fDistance);
			if(ped->bInVehicle && ped->m_nPedState == PED_DRIVING) {
				emittingVol = 10;
				veh = ped->m_pMyVehicle;
				if(veh && veh->m_type == 0) {
					for(int32 i = 2; i < 6; i++) {
						if(!veh->IsDoorClosed((eDoors)i) ||
						   veh->IsDoorMissing((eDoors)i)) {
							emittingVol = 42;
							break;
						}
					}
				}
			} else {
				emittingVol = 42;
			}

			m_sQueueSample.m_bVolume =
			    ComputeVolume(emittingVol, 7.f, m_sQueueSample.m_fDistance);
			if(m_sQueueSample.m_bVolume) {
				m_sQueueSample.m_counter = 64;
				m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_PED_HEADPHONES;
				m_sQueueSample.m_bBankIndex = 0;
				m_sQueueSample.m_bIsDistant = 0;
				m_sQueueSample.field_16 = 5;
				m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(
				    AUDIO_SAMPLE_PED_HEADPHONES);
				m_sQueueSample.m_nLoopCount = 0;
				m_sQueueSample.m_bEmittingVolume = emittingVol;
				m_sQueueSample.m_nLoopStart =
				    SampleManager.GetSampleLoopStartOffset(
				        m_sQueueSample.m_nSampleIndex);
				m_sQueueSample.m_nLoopEnd = SampleManager.GetSampleLoopEndOffset(
				    m_sQueueSample.m_nSampleIndex);
				m_sQueueSample.field_48 = 4.0f;
				m_sQueueSample.m_fSoundIntensity = 7.0f;
				m_sQueueSample.field_56 = 0;
				m_sQueueSample.field_76 = 5;
				m_sQueueSample.m_bReverbFlag = 1;
				m_sQueueSample.m_bRequireReflection = 0;
				AddSampleToRequestedQueue();
			}
			return;
		}
	}
}

#if 1
WRAPPER
void
cAudioManager::ProcessPedOneShots(cPedParams *params)
{
	EAXJMP(0x56F650);
}
#else
void
cAudioManager::ProcessPedOneShots(cPedParams *params)
{
	uint8 emittingVol;
	int32 sampleIndex;

	CPed *ped = params->m_pPed;

	char processed;
	int16 sound;
	char noReflection;
	CWeapon *weapon;
	float maxDist;

	static uint8 iSound = 21;

	weapon = nil;
	for(uint32 i = 0; i < m_asAudioEntities[m_sQueueSample.m_nEntityIndex].m_Loops; i++) {
		noReflection = 0;
		processed = 0;
		m_sQueueSample.m_bRequireReflection = 0;
		sound = m_asAudioEntities[m_sQueueSample.m_nEntityIndex].m_awAudioEvent[i];
		switch(sound) {
		case SOUND_STEP_START:
		case SOUND_STEP_END:
			if(!params->m_pPed->bIsLooking) {
				emittingVol = m_anRandomTable[3] % 15u + 45;
				if(FindPlayerPed() !=
				   m_asAudioEntities[m_sQueueSample.m_nEntityIndex].m_pEntity)
					emittingVol >>= 1;
				maxDist = 400.f;
				switch(params->m_pPed->m_nSurfaceTouched) {
				case SURFACE_GRASS:
					sampleIndex =
					    m_anRandomTable[1] % 5u + AUDIO_SAMPLE_PED_FALL_GRASS_1;
					break;
				case SURFACE_DIRT:
				case SURFACE_DIRTTRACK:
					sampleIndex =
					    m_anRandomTable[4] % 5u + AUDIO_SAMPLE_PED_FALL_DIRT_1;
					break;
				case SURFACE_METAL6:
				case SURFACE_METAL_DOOR:
				case SURFACE_BILLBOARD:
				case SURFACE_STEEL:
				case SURFACE_METAL_POLE:
				case SURFACE_STREET_LIGHT:
				case SURFACE_METAL14:
				case SURFACE_METAL15:
				case SURFACE_METAL_FENCE:
				case SURFACE_METAL27:
				case SURFACE_METAL28:
					sampleIndex =
					    m_anRandomTable[0] % 5u + AUDIO_SAMPLE_PED_FALL_METAL_1;
					break;
				case SURFACE_SAND:
					sampleIndex =
					    (m_anRandomTable[4] & 3) + AUDIO_SAMPLE_PED_FALL_SAND_1;
					break;
				case SURFACE_PUDDLE:
					sampleIndex = (m_anRandomTable[3] & 3) +
					              AUDIO_SAMPLE_PED_FALL_IN_WATER_1;
					break;
				case SURFACE_WOOD:
				case SURFACE_WOOD_BOX:
				case SURFACE_WOOD_PLANK:
					sampleIndex =
					    m_anRandomTable[2] % 5u + AUDIO_SAMPLE_PED_FALL_WOOD_1;
					break;
				case SURFACE_HEDGE:
					sampleIndex =
					    m_anRandomTable[2] % 5u + AUDIO_SAMPLE_COLLISION_HEDGE;
					break;
				default:
					sampleIndex = m_anRandomTable[2] % 5u +
					              AUDIO_SAMPLE_PED_FALL_PAVEMENT_1;
					break;
				}
				m_sQueueSample.m_nSampleIndex = sampleIndex;
				m_sQueueSample.m_bBankIndex = 0;
				m_sQueueSample.m_counter =
				    m_asAudioEntities[m_sQueueSample.m_nEntityIndex]
				        .m_awAudioEvent[i] -
				    28;
				m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(
				    m_sQueueSample.m_nSampleIndex);
				m_sQueueSample.m_nFrequency +=
				    RandomDisplacement(m_sQueueSample.m_nFrequency / 17u);
				switch(params->m_pPed->m_nMoveState) {
				case 2:
					emittingVol >>= 2;
					m_sQueueSample.m_nFrequency =
					    9 * m_sQueueSample.m_nFrequency / 10u;
					break;
				case 3:
					emittingVol >>= 1;
					m_sQueueSample.m_nFrequency =
					    11 * m_sQueueSample.m_nFrequency / 10u;
					break;
				case 4:
					m_sQueueSample.m_nFrequency =
					    12 * m_sQueueSample.m_nFrequency / 10u;
					break;
				default: break;
				}
				m_sQueueSample.field_16 = 5;
				m_sQueueSample.field_48 = 0.0f;
				m_sQueueSample.m_fSoundIntensity = 20.0f;
				m_sQueueSample.m_nLoopCount = 1;
				m_sQueueSample.m_nLoopStart = 0;
				m_sQueueSample.m_nLoopEnd = -1;
				m_sQueueSample.m_bEmittingVolume = emittingVol;
				m_sQueueSample.m_bIsDistant = 0;
				m_sQueueSample.field_56 = 1;
				m_sQueueSample.m_bRequireReflection = 1;
			}
			break;
		case SOUND_FALL_LAND:
		case SOUND_FALL_COLLAPSE:
			if(!ped->bIsLooking) {
				maxDist = 900.f;
				emittingVol = m_anRandomTable[3] % 20 + 80;
				if(ped->m_nSurfaceTouched == SURFACE_PUDDLE) {
					m_sQueueSample.m_nSampleIndex =
					    (m_anRandomTable[3] & 3) +
					    AUDIO_SAMPLE_PED_FALL_IN_WATER_1;
				} else if(sound == SOUND_FALL_LAND) {
					m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_PED_FALL_LAND;
				} else {
					m_sQueueSample.m_nSampleIndex =
					    AUDIO_SAMPLE_PED_FALL_COLLAPSE;
				}
				m_sQueueSample.m_bBankIndex = 0;
				m_sQueueSample.m_counter = 1;
				m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(
				    m_sQueueSample.m_nSampleIndex);
				m_sQueueSample.m_nFrequency +=
				    RandomDisplacement(m_sQueueSample.m_nFrequency / 17u);
				m_sQueueSample.field_16 = 2;
				m_sQueueSample.field_48 = 0.0f;
				m_sQueueSample.m_fSoundIntensity = 30.0f;
				m_sQueueSample.m_nLoopCount = 1;
				m_sQueueSample.m_nLoopStart = 0;
				m_sQueueSample.m_nLoopEnd = -1;
				m_sQueueSample.m_bEmittingVolume = emittingVol;
				m_sQueueSample.m_bIsDistant = 0;
				m_sQueueSample.field_56 = 1;
				m_sQueueSample.m_bRequireReflection = 1;
				break;
			}
			break;
		case SOUND_FIGHT_PUNCH_33:
			m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_PED_HIT_1;
			m_sQueueSample.m_nFrequency = 18000;
			m_sQueueSample.m_bBankIndex = 0;
			m_sQueueSample.m_counter = iSound;
			processed = 1;
			++iSound;
			m_sQueueSample.field_16 = 3;
			m_sQueueSample.field_48 = 0.0f;
			m_sQueueSample.m_fSoundIntensity = 30.0f;
			maxDist = 900.f;
			m_sQueueSample.m_nLoopCount = 1;
			m_sQueueSample.m_nLoopStart = 0;
			emittingVol = m_anRandomTable[3] % 26 + 100;
			m_sQueueSample.m_nLoopEnd = -1;
			m_sQueueSample.m_bEmittingVolume = emittingVol;
			m_sQueueSample.m_bIsDistant = 0;
			m_sQueueSample.field_56 = 1;
			m_sQueueSample.m_bRequireReflection = 1;
			break;
		case SOUND_FIGHT_KICK_34:
			m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_PED_HIT_1;
			m_sQueueSample.m_nFrequency = 16500;
			m_sQueueSample.m_bBankIndex = 0;
			m_sQueueSample.m_counter = iSound;
			processed = 1;
			++iSound;
			m_sQueueSample.field_16 = 3;
			m_sQueueSample.field_48 = 0.0f;
			m_sQueueSample.m_fSoundIntensity = 30.0f;
			maxDist = 900.f;
			m_sQueueSample.m_nLoopCount = 1;
			m_sQueueSample.m_nLoopStart = 0;
			emittingVol = m_anRandomTable[3] % 26 + 100;
			m_sQueueSample.m_nLoopEnd = -1;
			m_sQueueSample.m_bEmittingVolume = emittingVol;
			m_sQueueSample.m_bIsDistant = 0;
			m_sQueueSample.field_56 = 1;
			m_sQueueSample.m_bRequireReflection = 1;
			break;
		case SOUND_FIGHT_HEADBUTT_35:
			m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_PED_HIT_1;
			m_sQueueSample.m_nFrequency = 20000;
			m_sQueueSample.m_bBankIndex = 0;
			m_sQueueSample.m_counter = iSound;
			processed = 1;
			++iSound;
			m_sQueueSample.field_16 = 3;
			m_sQueueSample.field_48 = 0.0f;
			m_sQueueSample.m_fSoundIntensity = 30.0f;
			maxDist = 900.f;
			m_sQueueSample.m_nLoopCount = 1;
			m_sQueueSample.m_nLoopStart = 0;
			emittingVol = m_anRandomTable[3] % 26 + 100;
			m_sQueueSample.m_nLoopEnd = -1;
			m_sQueueSample.m_bEmittingVolume = emittingVol;
			m_sQueueSample.m_bIsDistant = 0;
			m_sQueueSample.field_56 = 1;
			m_sQueueSample.m_bRequireReflection = 1;
			break;
		case SOUND_FIGHT_PUNCH_36:
			m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_PED_HIT_2;
			m_sQueueSample.m_nFrequency = 18000;
			m_sQueueSample.m_bBankIndex = 0;
			m_sQueueSample.m_counter = iSound;
			processed = 1;
			++iSound;
			m_sQueueSample.field_16 = 3;
			m_sQueueSample.field_48 = 0.0f;
			m_sQueueSample.m_fSoundIntensity = 30.0f;
			maxDist = 900.f;
			m_sQueueSample.m_nLoopCount = 1;
			m_sQueueSample.m_nLoopStart = 0;
			emittingVol = m_anRandomTable[3] % 26 + 100;
			m_sQueueSample.m_nLoopEnd = -1;
			m_sQueueSample.m_bEmittingVolume = emittingVol;
			m_sQueueSample.m_bIsDistant = 0;
			m_sQueueSample.field_56 = 1;
			m_sQueueSample.m_bRequireReflection = 1;
			break;
		case SOUND_FIGHT_PUNCH_37:
			m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_PED_HIT_2;
			m_sQueueSample.m_nFrequency = 16500;
			m_sQueueSample.m_bBankIndex = 0;
			m_sQueueSample.m_counter = iSound;
			processed = 1;
			++iSound;
			m_sQueueSample.field_16 = 3;
			m_sQueueSample.field_48 = 0.0f;
			m_sQueueSample.m_fSoundIntensity = 30.0f;
			maxDist = 900.f;
			m_sQueueSample.m_nLoopCount = 1;
			m_sQueueSample.m_nLoopStart = 0;
			emittingVol = m_anRandomTable[3] % 26 + 100;
			m_sQueueSample.m_nLoopEnd = -1;
			m_sQueueSample.m_bEmittingVolume = emittingVol;
			m_sQueueSample.m_bIsDistant = 0;
			m_sQueueSample.field_56 = 1;
			m_sQueueSample.m_bRequireReflection = 1;
			break;
		case SOUND_FIGHT_CLOSE_PUNCH_38:
			m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_PED_HIT_2;
			m_sQueueSample.m_nFrequency = 20000;
			m_sQueueSample.m_bBankIndex = 0;
			m_sQueueSample.m_counter = iSound;
			processed = 1;
			++iSound;
			m_sQueueSample.field_16 = 3;
			m_sQueueSample.field_48 = 0.0f;
			m_sQueueSample.m_fSoundIntensity = 30.0f;
			maxDist = 900.f;
			m_sQueueSample.m_nLoopCount = 1;
			m_sQueueSample.m_nLoopStart = 0;
			emittingVol = m_anRandomTable[3] % 26 + 100;
			m_sQueueSample.m_nLoopEnd = -1;
			m_sQueueSample.m_bEmittingVolume = emittingVol;
			m_sQueueSample.m_bIsDistant = 0;
			m_sQueueSample.field_56 = 1;
			m_sQueueSample.m_bRequireReflection = 1;
			break;
		case SOUND_FIGHT_PUNCH_39:
			m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_PED_HIT_3;
			m_sQueueSample.m_nFrequency = 18000;
			m_sQueueSample.m_bBankIndex = 0;
			m_sQueueSample.m_counter = iSound;
			processed = 1;
			++iSound;
			m_sQueueSample.field_16 = 3;
			m_sQueueSample.field_48 = 0.0f;
			m_sQueueSample.m_fSoundIntensity = 30.0f;
			maxDist = 900.f;
			m_sQueueSample.m_nLoopCount = 1;
			m_sQueueSample.m_nLoopStart = 0;
			emittingVol = m_anRandomTable[3] % 26 + 100;
			m_sQueueSample.m_nLoopEnd = -1;
			m_sQueueSample.m_bEmittingVolume = emittingVol;
			m_sQueueSample.m_bIsDistant = 0;
			m_sQueueSample.field_56 = 1;
			m_sQueueSample.m_bRequireReflection = 1;
			break;
		case SOUND_FIGHT_PUNCH_OR_KICK_BELOW_40:
			m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_PED_HIT_3;
			m_sQueueSample.m_nFrequency = 16500;
			m_sQueueSample.m_bBankIndex = 0;
			m_sQueueSample.m_counter = iSound;
			processed = 1;
			++iSound;
			m_sQueueSample.field_16 = 3;
			m_sQueueSample.field_48 = 0.0f;
			m_sQueueSample.m_fSoundIntensity = 30.0f;
			maxDist = 900.f;
			m_sQueueSample.m_nLoopCount = 1;
			m_sQueueSample.m_nLoopStart = 0;
			emittingVol = m_anRandomTable[3] % 26 + 100;
			m_sQueueSample.m_nLoopEnd = -1;
			m_sQueueSample.m_bEmittingVolume = emittingVol;
			m_sQueueSample.m_bIsDistant = 0;
			m_sQueueSample.field_56 = 1;
			m_sQueueSample.m_bRequireReflection = 1;
			break;
		case SOUND_FIGHT_PUNCH_41:
			m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_PED_HIT_3;
			m_sQueueSample.m_nFrequency = 20000;
			m_sQueueSample.m_bBankIndex = 0;
			m_sQueueSample.m_counter = iSound;
			processed = 1;
			++iSound;
			m_sQueueSample.field_16 = 3;
			m_sQueueSample.field_48 = 0.0f;
			m_sQueueSample.m_fSoundIntensity = 30.0f;
			maxDist = 900.f;
			m_sQueueSample.m_nLoopCount = 1;
			m_sQueueSample.m_nLoopStart = 0;
			emittingVol = m_anRandomTable[3] % 26 + 100;
			m_sQueueSample.m_nLoopEnd = -1;
			m_sQueueSample.m_bEmittingVolume = emittingVol;
			m_sQueueSample.m_bIsDistant = 0;
			m_sQueueSample.field_56 = 1;
			m_sQueueSample.m_bRequireReflection = 1;
			break;
		case SOUND_FIGHT_PUNCH_FROM_BEHIND_42:
			m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_PED_HIT_4;
			m_sQueueSample.m_nFrequency = 18000;
			m_sQueueSample.m_bBankIndex = 0;
			m_sQueueSample.m_counter = iSound;
			processed = 1;
			++iSound;
			m_sQueueSample.field_16 = 3;
			m_sQueueSample.field_48 = 0.0f;
			m_sQueueSample.m_fSoundIntensity = 30.0f;
			maxDist = 900.f;
			m_sQueueSample.m_nLoopCount = 1;
			m_sQueueSample.m_nLoopStart = 0;
			emittingVol = m_anRandomTable[3] % 26 + 100;
			m_sQueueSample.m_nLoopEnd = -1;
			m_sQueueSample.m_bEmittingVolume = emittingVol;
			m_sQueueSample.m_bIsDistant = 0;
			m_sQueueSample.field_56 = 1;
			m_sQueueSample.m_bRequireReflection = 1;
			break;
		case SOUND_FIGHT_KNEE_OR_KICK_43:
			m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_PED_HIT_4;
			m_sQueueSample.m_nFrequency = 16500;
			m_sQueueSample.m_bBankIndex = 0;
			m_sQueueSample.m_counter = iSound;
			processed = 1;
			++iSound;
			m_sQueueSample.field_16 = 3;
			m_sQueueSample.field_48 = 0.0f;
			m_sQueueSample.m_fSoundIntensity = 30.0f;
			maxDist = 900.f;
			m_sQueueSample.m_nLoopCount = 1;
			m_sQueueSample.m_nLoopStart = 0;
			emittingVol = m_anRandomTable[3] % 26 + 100;
			m_sQueueSample.m_nLoopEnd = -1;
			m_sQueueSample.m_bEmittingVolume = emittingVol;
			m_sQueueSample.m_bIsDistant = 0;
			m_sQueueSample.field_56 = 1;
			m_sQueueSample.m_bRequireReflection = 1;
			break;
		case SOUND_FIGHT_KICK_44:
			m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_PED_HIT_4;
			m_sQueueSample.m_nFrequency = 20000;
			m_sQueueSample.m_bBankIndex = 0;
			m_sQueueSample.m_counter = iSound;
			processed = 1;
			++iSound;
			m_sQueueSample.field_16 = 3;
			m_sQueueSample.field_48 = 0.0f;
			m_sQueueSample.m_fSoundIntensity = 30.0f;
			maxDist = 900.f;
			m_sQueueSample.m_nLoopCount = 1;
			m_sQueueSample.m_nLoopStart = 0;
			emittingVol = m_anRandomTable[3] % 26 + 100;
			m_sQueueSample.m_nLoopEnd = -1;
			m_sQueueSample.m_bEmittingVolume = emittingVol;
			m_sQueueSample.m_bIsDistant = 0;
			m_sQueueSample.field_56 = 1;
			m_sQueueSample.m_bRequireReflection = 1;
			break;
		case SOUND_WEAPON_BAT_ATTACK:
			m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_PED_HIT_BY_BAT;
			m_sQueueSample.m_bBankIndex = 0;
			m_sQueueSample.m_counter = iSound++;
			processed = 1;
			m_sQueueSample.m_nFrequency = RandomDisplacement(2000u) + 22000;
			m_sQueueSample.field_16 = 3;
			m_sQueueSample.field_48 = 0.0f;
			m_sQueueSample.m_fSoundIntensity = 30.0f;
			maxDist = 900.f;
			m_sQueueSample.m_nLoopCount = 1;
			m_sQueueSample.m_nLoopStart = 0;
			m_sQueueSample.m_nLoopEnd = -1;
			emittingVol = m_anRandomTable[2] % 20 + 100;
			m_sQueueSample.m_bEmittingVolume = emittingVol;
			m_sQueueSample.m_bIsDistant = 0;
			m_sQueueSample.field_56 = 1;
			if(m_bDynamicAcousticModelingStatus)
				m_sQueueSample.m_bRequireReflection = 1;
			else
				noReflection = 1;
			break;
		case SOUND_WEAPON_SHOT_FIRED:
			weapon = &ped->m_weapons[ped->m_currentWeapon];
			switch(weapon->m_eWeaponType) {
			case WEAPONTYPE_COLT45:
				m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_WEAPON_PISTOL_SHOT;
				m_sQueueSample.m_bBankIndex = 0;
				m_sQueueSample.m_counter = iSound++;
				processed = 1;
				m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(
				    AUDIO_SAMPLE_WEAPON_PISTOL_SHOT);
				m_sQueueSample.m_nFrequency +=
				    RandomDisplacement(m_sQueueSample.m_nFrequency >> 5);
				m_sQueueSample.field_16 = 3;
				m_sQueueSample.field_48 = 0.0f;
				m_sQueueSample.m_fSoundIntensity = 50.0f;
				maxDist = 2500.f;
				m_sQueueSample.m_nLoopCount = 1;
				m_sQueueSample.m_nLoopStart = 0;
				m_sQueueSample.m_nLoopEnd = -1;
				emittingVol = m_anRandomTable[1] % 10 + 90;
				m_sQueueSample.m_bEmittingVolume = emittingVol;
				m_sQueueSample.m_bIsDistant = 0;
				m_sQueueSample.field_56 = 1;
				if(m_bDynamicAcousticModelingStatus)
					m_sQueueSample.m_bRequireReflection = 1;
				else
					noReflection = 1;
				break;
			case WEAPONTYPE_UZI:
				m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_WEAPON_UZI_SHOT;
				m_sQueueSample.m_bBankIndex = 0;
				m_sQueueSample.m_counter = iSound++;
				processed = 1;
				m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(
				    AUDIO_SAMPLE_WEAPON_UZI_SHOT);
				m_sQueueSample.m_nFrequency +=
				    RandomDisplacement(m_sQueueSample.m_nFrequency >> 5);
				m_sQueueSample.field_16 = 3;
				m_sQueueSample.field_48 = 0.0f;
				m_sQueueSample.m_fSoundIntensity = 80.0f;
				maxDist = 6400.f;
				m_sQueueSample.m_nLoopCount = 1;
				m_sQueueSample.m_nLoopStart = 0;
				emittingVol = m_anRandomTable[3] % 15 + 70;
				m_sQueueSample.m_nLoopEnd = -1;
				m_sQueueSample.m_bEmittingVolume = emittingVol;
				m_sQueueSample.m_bIsDistant = 0;
				m_sQueueSample.field_56 = 1;
				break;
			case WEAPONTYPE_SHOTGUN:
				m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_WEAPON_SHOTGUN_SHOT;
				m_sQueueSample.m_bBankIndex = 0;
				m_sQueueSample.m_counter = iSound++;
				processed = 1;
				m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(
				    AUDIO_SAMPLE_WEAPON_SHOTGUN_SHOT);
				m_sQueueSample.m_nFrequency +=
				    RandomDisplacement(m_sQueueSample.m_nFrequency >> 5);
				m_sQueueSample.field_16 = 3;
				m_sQueueSample.field_48 = 0.0f;
				m_sQueueSample.m_fSoundIntensity = 60.0f;
				maxDist = 3600.f;
				m_sQueueSample.m_nLoopCount = 1;
				m_sQueueSample.m_nLoopStart = 0;
				m_sQueueSample.m_nLoopEnd = -1;
				emittingVol = m_anRandomTable[2] % 10 + 100;
				m_sQueueSample.m_bEmittingVolume = emittingVol;
				m_sQueueSample.m_bIsDistant = 0;
				m_sQueueSample.field_56 = 1;
				if(m_bDynamicAcousticModelingStatus)
					m_sQueueSample.m_bRequireReflection = 1;
				else
					noReflection = 1;
				break;
			case WEAPONTYPE_AK47:
				m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_WEAPON_CHAINGUN_SHOT;
				m_sQueueSample.m_bBankIndex = 0;
				m_sQueueSample.m_counter = iSound++;
				processed = 1;
				m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(
				    AUDIO_SAMPLE_WEAPON_CHAINGUN_SHOT);
				m_sQueueSample.m_nFrequency +=
				    RandomDisplacement(m_sQueueSample.m_nFrequency >> 5);
				m_sQueueSample.field_16 = 3;
				m_sQueueSample.field_48 = 0.0f;
				m_sQueueSample.m_fSoundIntensity = 80.0f;
				maxDist = 6400.f;
				m_sQueueSample.m_nLoopCount = 1;
				m_sQueueSample.m_nLoopStart = 0;
				emittingVol = m_anRandomTable[1] % 15 + 70;
				m_sQueueSample.m_nLoopEnd = -1;
				m_sQueueSample.m_bEmittingVolume = emittingVol;
				m_sQueueSample.m_bIsDistant = 0;
				m_sQueueSample.field_56 = 1;
				break;
			case WEAPONTYPE_M16:
				m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_WEAPON_M16_SHOT;
				m_sQueueSample.m_bBankIndex = 0;
				m_sQueueSample.m_counter = iSound++;
				processed = 1;
				m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(
				    AUDIO_SAMPLE_WEAPON_M16_SHOT);
				m_sQueueSample.m_nFrequency +=
				    RandomDisplacement(m_sQueueSample.m_nFrequency >> 5);
				m_sQueueSample.field_16 = 3;
				m_sQueueSample.field_48 = 0.0f;
				m_sQueueSample.m_fSoundIntensity = 80.0f;
				maxDist = 6400.f;
				m_sQueueSample.m_nLoopCount = 1;
				m_sQueueSample.m_nLoopStart = 0;
				emittingVol = m_anRandomTable[4] % 15 + 70;
				m_sQueueSample.m_nLoopEnd = -1;
				m_sQueueSample.m_bEmittingVolume = emittingVol;
				m_sQueueSample.m_bIsDistant = 0;
				m_sQueueSample.field_56 = 1;
				break;
			case WEAPONTYPE_SNIPERRIFLE:
				m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_WEAPON_SNIPER_SHOT;
				m_sQueueSample.m_bBankIndex = 0;
				m_sQueueSample.m_counter = iSound++;
				processed = 1;
				m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(
				    AUDIO_SAMPLE_WEAPON_SNIPER_SHOT);
				m_sQueueSample.m_nFrequency +=
				    RandomDisplacement(m_sQueueSample.m_nFrequency >> 5);
				m_sQueueSample.field_16 = 3;
				m_sQueueSample.field_48 = 0.0f;
				m_sQueueSample.m_fSoundIntensity = 60.0f;
				maxDist = 3600.f;
				m_sQueueSample.m_nLoopCount = 1;
				m_sQueueSample.m_nLoopStart = 0;
				m_sQueueSample.m_nLoopEnd = -1;
				emittingVol = m_anRandomTable[4] % 10 + 110;
				m_sQueueSample.m_bEmittingVolume = emittingVol;
				m_sQueueSample.m_bIsDistant = 0;
				m_sQueueSample.field_56 = 1;
				if(m_bDynamicAcousticModelingStatus)
					m_sQueueSample.m_bRequireReflection = 1;
				else
					noReflection = 1;
				break;
			case WEAPONTYPE_ROCKETLAUNCHER:
				m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_WEAPON_ROCKET_SHOT;
				m_sQueueSample.m_bBankIndex = 0;
				m_sQueueSample.m_counter = iSound++;
				processed = 1;
				m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(
				    AUDIO_SAMPLE_WEAPON_ROCKET_SHOT);
				m_sQueueSample.m_nFrequency +=
				    RandomDisplacement(m_sQueueSample.m_nFrequency >> 5);
				m_sQueueSample.field_16 = 1;
				m_sQueueSample.field_48 = 0.0f;
				m_sQueueSample.m_fSoundIntensity = 90.0f;
				maxDist = 8100.f;
				m_sQueueSample.m_nLoopCount = 1;
				m_sQueueSample.m_nLoopStart = 0;
				m_sQueueSample.m_nLoopEnd = -1;
				emittingVol = m_anRandomTable[0] % 20 + 80;
				m_sQueueSample.m_bEmittingVolume = emittingVol;
				m_sQueueSample.m_bIsDistant = 0;
				m_sQueueSample.field_56 = 1;
				if(m_bDynamicAcousticModelingStatus)
					m_sQueueSample.m_bRequireReflection = 1;
				else
					noReflection = 1;
				break;
			case WEAPONTYPE_FLAMETHROWER:
				m_sQueueSample.m_nSampleIndex =
				    AUDIO_SAMPLE_WEAPON_FLAMETHROWER_SHOT;
				m_sQueueSample.m_bBankIndex = 0;
				m_sQueueSample.m_counter = 9;
				emittingVol = 90;
				m_sQueueSample.m_nFrequency =
				    (10 * m_sQueueSample.m_nEntityIndex & 2047) +
				    SampleManager.GetSampleBaseFrequency(
				        AUDIO_SAMPLE_WEAPON_FLAMETHROWER_SHOT);
				m_sQueueSample.field_16 = 3;
				m_sQueueSample.field_48 = 4.0f;
				m_sQueueSample.m_fSoundIntensity = 60.0f;
				maxDist = 3600.f;
				m_sQueueSample.m_nLoopCount = 0;
				m_sQueueSample.m_nLoopStart =
				    SampleManager.GetSampleLoopStartOffset(
				        m_sQueueSample.m_nSampleIndex);
				m_sQueueSample.m_nLoopEnd = SampleManager.GetSampleLoopEndOffset(
				    m_sQueueSample.m_nSampleIndex);
				m_sQueueSample.m_bEmittingVolume = 90;
				m_sQueueSample.m_bIsDistant = 0;
				m_sQueueSample.field_56 = 0;
				m_sQueueSample.field_76 = 6;
				if(m_bDynamicAcousticModelingStatus)
					m_sQueueSample.m_bRequireReflection = 1;
				else
					noReflection = 1;
				break;
			default: continue;
			}

			break;
		case SOUND_WEAPON_RELOAD:
			weapon = &ped->m_weapons[ped->m_currentWeapon];
			switch(weapon->m_eWeaponType) {
			case WEAPONTYPE_COLT45:
				m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_WEAPON_PISTOL_RELOAD;
				emittingVol = 75;
				m_sQueueSample.m_counter = iSound++;
				processed = 1;
				m_sQueueSample.m_nFrequency =
				    SampleManager.GetSampleBaseFrequency(
				        AUDIO_SAMPLE_WEAPON_PISTOL_RELOAD) +
				    RandomDisplacement(300u);
				m_sQueueSample.m_nFrequency += RandomDisplacement(300u);
				m_sQueueSample.m_bBankIndex = 0;
				m_sQueueSample.field_16 = 5;
				m_sQueueSample.field_48 = 0.0f;
				m_sQueueSample.m_fSoundIntensity = 30.0f;
				maxDist = 900.f;
				m_sQueueSample.m_nLoopCount = 1;
				m_sQueueSample.m_nLoopStart = 0;
				m_sQueueSample.m_nLoopEnd = -1;
				m_sQueueSample.m_bEmittingVolume = 75;
				m_sQueueSample.m_bIsDistant = 0;
				m_sQueueSample.field_56 = 1;
				m_sQueueSample.m_bRequireReflection = 1;
				break;
			case WEAPONTYPE_UZI:
				m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_WEAPON_UZI_RELOAD;
				emittingVol = 75;
				m_sQueueSample.m_counter = iSound++;
				processed = 1;
				m_sQueueSample.m_nFrequency = 39243;
				m_sQueueSample.m_nFrequency += RandomDisplacement(300u);
				m_sQueueSample.m_bBankIndex = 0;
				m_sQueueSample.field_16 = 5;
				m_sQueueSample.field_48 = 0.0f;
				m_sQueueSample.m_fSoundIntensity = 30.0f;
				maxDist = 900.f;
				m_sQueueSample.m_nLoopCount = 1;
				m_sQueueSample.m_nLoopStart = 0;
				m_sQueueSample.m_nLoopEnd = -1;
				m_sQueueSample.m_bEmittingVolume = 75;
				m_sQueueSample.m_bIsDistant = 0;
				m_sQueueSample.field_56 = 1;
				m_sQueueSample.m_bRequireReflection = 1;
				break;
			case WEAPONTYPE_SHOTGUN:
				m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_WEAPON_CHAINGUN_RELOAD;
				emittingVol = 75;
				m_sQueueSample.m_counter = iSound++;
				processed = 1;
				m_sQueueSample.m_nFrequency = 30290;
				m_sQueueSample.m_nFrequency += RandomDisplacement(300u);
				m_sQueueSample.m_bBankIndex = 0;
				m_sQueueSample.field_16 = 5;
				m_sQueueSample.field_48 = 0.0f;
				m_sQueueSample.m_fSoundIntensity = 30.0f;
				maxDist = 900.f;
				m_sQueueSample.m_nLoopCount = 1;
				m_sQueueSample.m_nLoopStart = 0;
				m_sQueueSample.m_nLoopEnd = -1;
				m_sQueueSample.m_bEmittingVolume = 75;
				m_sQueueSample.m_bIsDistant = 0;
				m_sQueueSample.field_56 = 1;
				m_sQueueSample.m_bRequireReflection = 1;
				break;
			case WEAPONTYPE_AK47:
				m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_WEAPON_CHAINGUN_RELOAD;
				emittingVol = 75;
				m_sQueueSample.m_counter = iSound++;
				processed = 1;
				m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(
				    AUDIO_SAMPLE_WEAPON_CHAINGUN_RELOAD);
				m_sQueueSample.m_nFrequency += RandomDisplacement(300u);
				m_sQueueSample.m_bBankIndex = 0;
				m_sQueueSample.field_16 = 5;
				m_sQueueSample.field_48 = 0.0f;
				m_sQueueSample.m_fSoundIntensity = 30.0f;
				maxDist = 900.f;
				m_sQueueSample.m_nLoopCount = 1;
				m_sQueueSample.m_nLoopStart = 0;
				m_sQueueSample.m_nLoopEnd = -1;
				m_sQueueSample.m_bEmittingVolume = 75;
				m_sQueueSample.m_bIsDistant = 0;
				m_sQueueSample.field_56 = 1;
				m_sQueueSample.m_bRequireReflection = 1;
				break;
			case WEAPONTYPE_M16:
				m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_WEAPON_UZI_RELOAD;
				emittingVol = 75;
				m_sQueueSample.m_counter = iSound++;
				processed = 1;
				m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(
				    AUDIO_SAMPLE_WEAPON_UZI_RELOAD);
				m_sQueueSample.m_nFrequency += RandomDisplacement(300u);
				m_sQueueSample.m_bBankIndex = 0;
				m_sQueueSample.field_16 = 5;
				m_sQueueSample.field_48 = 0.0f;
				m_sQueueSample.m_fSoundIntensity = 30.0f;
				maxDist = 900.f;
				m_sQueueSample.m_nLoopCount = 1;
				m_sQueueSample.m_nLoopStart = 0;
				m_sQueueSample.m_nLoopEnd = -1;
				m_sQueueSample.m_bEmittingVolume = 75;
				m_sQueueSample.m_bIsDistant = 0;
				m_sQueueSample.field_56 = 1;
				m_sQueueSample.m_bRequireReflection = 1;
				break;
			case WEAPONTYPE_SNIPERRIFLE:
				m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_WEAPON_SNIPER_RELOAD;
				emittingVol = 75;
				m_sQueueSample.m_counter = iSound++;
				processed = 1;
				m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(
				    AUDIO_SAMPLE_WEAPON_SNIPER_RELOAD);
				m_sQueueSample.m_nFrequency += RandomDisplacement(300u);
				m_sQueueSample.m_bBankIndex = 0;
				m_sQueueSample.field_16 = 5;
				m_sQueueSample.field_48 = 0.0f;
				m_sQueueSample.m_fSoundIntensity = 30.0f;
				maxDist = 900.f;
				m_sQueueSample.m_nLoopCount = 1;
				m_sQueueSample.m_nLoopStart = 0;
				m_sQueueSample.m_nLoopEnd = -1;
				m_sQueueSample.m_bEmittingVolume = 75;
				m_sQueueSample.m_bIsDistant = 0;
				m_sQueueSample.field_56 = 1;
				m_sQueueSample.m_bRequireReflection = 1;
				break;
			case WEAPONTYPE_ROCKETLAUNCHER:
				m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_WEAPON_ROCKET_RELOAD;
				emittingVol = 75;
				m_sQueueSample.m_counter = iSound++;
				processed = 1;
				m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(
				    AUDIO_SAMPLE_WEAPON_ROCKET_RELOAD);
				m_sQueueSample.m_nFrequency += RandomDisplacement(300u);
				m_sQueueSample.m_bBankIndex = 0;
				m_sQueueSample.field_16 = 5;
				m_sQueueSample.field_48 = 0.0f;
				m_sQueueSample.m_fSoundIntensity = 30.0f;
				maxDist = 900.f;
				m_sQueueSample.m_nLoopCount = 1;
				m_sQueueSample.m_nLoopStart = 0;
				m_sQueueSample.m_nLoopEnd = -1;
				m_sQueueSample.m_bEmittingVolume = 75;
				m_sQueueSample.m_bIsDistant = 0;
				m_sQueueSample.field_56 = 1;
				m_sQueueSample.m_bRequireReflection = 1;
				break;
			default: continue;
			}
			break;
		case SOUND_WEAPON_AK47_BULLET_ECHO:
		case SOUND_WEAPON_UZI_BULLET_ECHO:
		case SOUND_WEAPON_M16_BULLET_ECHO:
			m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_75;
			m_sQueueSample.m_bBankIndex = 0;
			m_sQueueSample.m_counter = iSound++;
			processed = 1;
			m_sQueueSample.m_nFrequency =
			    SampleManager.GetSampleBaseFrequency(AUDIO_SAMPLE_75);
			m_sQueueSample.m_nFrequency +=
			    RandomDisplacement(m_sQueueSample.m_nFrequency >> 4);
			m_sQueueSample.field_16 = 3;
			m_sQueueSample.field_48 = 0.0f;
			m_sQueueSample.m_fSoundIntensity = 80.0f;
			maxDist = 6400.f;
			m_sQueueSample.m_nLoopCount = 1;
			m_sQueueSample.m_nLoopStart = 0;
			m_sQueueSample.m_nLoopEnd = -1;
			emittingVol = m_anRandomTable[4] % 10 + 40;
			m_sQueueSample.m_bEmittingVolume = emittingVol;
			m_sQueueSample.m_bIsDistant = 0;
			m_sQueueSample.field_56 = 1;
			if(m_bDynamicAcousticModelingStatus)
				m_sQueueSample.m_bRequireReflection = 1;
			else
				noReflection = 1;
			break;
		case SOUND_WEAPON_FLAMETHROWER_FIRE:
			m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_WEAPON_FLAMETHROWER_FIRE;
			m_sQueueSample.m_bBankIndex = 0;
			m_sQueueSample.m_counter = iSound++;
			m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(
			    AUDIO_SAMPLE_WEAPON_FLAMETHROWER_FIRE);
			m_sQueueSample.m_nFrequency +=
			    RandomDisplacement(m_sQueueSample.m_nFrequency >> 4);
			m_sQueueSample.field_16 = 3;
			m_sQueueSample.field_48 = 4.0f;
			m_sQueueSample.m_fSoundIntensity = 60.0f;
			maxDist = 3600.f;
			m_sQueueSample.m_nLoopCount = 1;
			m_sQueueSample.m_nLoopStart = 0;
			m_sQueueSample.m_nLoopEnd = -1;
			emittingVol = 70;
			m_sQueueSample.m_bEmittingVolume = 70;
			m_sQueueSample.m_bIsDistant = 0;
			m_sQueueSample.field_56 = 1;
			break;
		case SOUND_WEAPON_HIT_PED:
			m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_BULLET_HIT_PED;
			m_sQueueSample.m_bBankIndex = 0;
			m_sQueueSample.m_counter = iSound++;
			processed = 1;
			m_sQueueSample.m_nFrequency =
			    SampleManager.GetSampleBaseFrequency(AUDIO_SAMPLE_BULLET_HIT_PED);
			m_sQueueSample.m_nFrequency +=
			    RandomDisplacement(m_sQueueSample.m_nFrequency >> 3);
			m_sQueueSample.field_16 = 7;
			m_sQueueSample.field_48 = 0.0f;
			m_sQueueSample.m_fSoundIntensity = 30.0f;
			maxDist = 900.f;
			m_sQueueSample.m_nLoopCount = 1;
			m_sQueueSample.m_nLoopStart = 0;
			emittingVol = m_anRandomTable[0] % 20 + 90;
			m_sQueueSample.m_nLoopEnd = -1;
			m_sQueueSample.m_bEmittingVolume = emittingVol;
			m_sQueueSample.m_bIsDistant = 0;
			m_sQueueSample.field_56 = 1;
			break;
		case SOUND_SPLASH:
			m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_COLLISION_WATER;
			m_sQueueSample.m_bBankIndex = 0;
			m_sQueueSample.m_counter = iSound++;
			processed = 1;
			m_sQueueSample.m_nFrequency = RandomDisplacement(1400u) + 20000;
			m_sQueueSample.field_16 = 1;
			m_sQueueSample.field_48 = 0.0f;
			m_sQueueSample.m_fSoundIntensity = 40.0f;
			maxDist = 1600.f;
			m_sQueueSample.m_nLoopCount = 1;
			m_sQueueSample.m_nLoopStart = 0;
			emittingVol = m_anRandomTable[2] % 30 + 70;
			m_sQueueSample.m_nLoopEnd = -1;
			m_sQueueSample.m_bEmittingVolume = emittingVol;
			m_sQueueSample.m_bIsDistant = 0;
			m_sQueueSample.field_56 = 1;
			m_sQueueSample.m_bRequireReflection = 1;
			break;
		default:
			SetupPedComments(params, sound);
			continue;

			if(processed && iSound > 60u) iSound = 21;
			if(params->m_fDistance < maxDist) {
				CalculateDistance((bool *)params, params->m_fDistance);
				m_sQueueSample.m_bVolume =
				    ComputeVolume(emittingVol, m_sQueueSample.m_fSoundIntensity,
				                  m_sQueueSample.m_fDistance);
				if(m_sQueueSample.m_bVolume) {
					if(noReflection) {
						if(0.2f * m_sQueueSample.m_fSoundIntensity >
						   m_sQueueSample.m_fDistance) {
							noReflection = 0;
						} else {
							m_sQueueSample.m_bIsDistant = 1;
							m_sQueueSample.m_bOffset = 0;
						}
					}
					m_sQueueSample.m_bReverbFlag = 1;
					AddSampleToRequestedQueue();
					if(noReflection) {
						m_sQueueSample.m_bOffset = 127;
						++m_sQueueSample.m_nSampleIndex;
						if(m_asAudioEntities[m_sQueueSample.m_nEntityIndex]
						           .m_awAudioEvent[i] != 47 ||
						   weapon->m_eWeaponType !=
						       WEAPONTYPE_FLAMETHROWER) {
							m_sQueueSample.m_counter = iSound++;
							if(iSound > 60u) iSound = 21;
						} else {
							++m_sQueueSample.m_counter;
						}
						AddSampleToRequestedQueue();
					}
				}
			}
		}
	}
}
#endif

void
cAudioManager::ProcessPhysical(int32 id)
{
	CPhysical *entity = (CPhysical *)m_asAudioEntities[id].m_pEntity;
	if(entity) {
		switch(entity->m_type & 7) {
		case ENTITY_TYPE_VEHICLE:
			ProcessVehicle((CVehicle *)m_asAudioEntities[id].m_pEntity);
			break;
		case ENTITY_TYPE_PED:
			ProcessPed((CPhysical *)m_asAudioEntities[id].m_pEntity);
			break;
		default: return;
		}
	}
}

void
cAudioManager::ProcessPlane(cVehicleParams *params)
{
	switch(params->m_nIndex) {
	case AIRTRAIN: ProcessJumbo(params); break;
	case DEADDODO: ProcessCesna(params); break;
	default: debug("Plane Model Id is %d\n, ", params->m_pVehicle->m_modelIndex); break;
	}
}

struct tVehicleSampleData {
	eAudioSamples m_nAccelerationSampleIndex;
	char m_bEngineSoundType;
	char gap_5[3];
	eAudioSamples m_nHornSample;
	int32 m_nHornFrequency;
	char m_nSirenOrAlarmSample;
	int m_nSirenOrAlarmFrequency;
	char m_bDoorType;
	char gap_25[3];
};

int32 *CSWTCH_554 = (int32 *)0x606A50;
tVehicleSampleData *CarSounds = (tVehicleSampleData *)0x606204;

void
cAudioManager::ProcessPlayersVehicleEngine(cVehicleParams *params, CAutomobile *automobile)
{
	cTransmission *transmission;
	float velocityChange;
	float relativeVelocityChange;
	float accelerationMultipler;
	uint8 wheelInUseCounter;
	uint8 i;
	float time;
	int baseFreq;
	uint8 vol;
	int gearNr;
	int32 freq;

	int freqModifier;
	int soundOffset;
	uint8 engineSoundType;
	int32 accelerateState;
	bool channelUsed;
	char lostTraction;
	bool processedAccelSampleStopped;
	uint8 currentGear;
	float gasPedalAudio;
	CVector pos;

	static int16 LastAccel = 0;
	static int16 LastBrake = 0;
	static uint8 CurrentPretendGear = 1;
	static bool bLostTractionLastFrame = 0;
	static bool bHandbrakeOnLastFrame = 0;
	static int32 nCruising = 0;
	static bool bAccelSampleStopped = 1;

	lostTraction = 0;
	processedAccelSampleStopped = 0;
	if(bPlayerJustEnteredCar) {
		bAccelSampleStopped = 1;
		bPlayerJustEnteredCar = 0;
		nCruising = 0;
		LastAccel = 0;
		bLostTractionLastFrame = 0;
		LastBrake = 0;
		bHandbrakeOnLastFrame = 0;
		CurrentPretendGear = 1;
	}
	if(CReplay::IsPlayingBack()) {
		accelerateState = 255.f * max(0.0f, min(1.0f, automobile->m_fGasPedal));
	} else {
		accelerateState = Pads->GetAccelerate();
	}
	channelUsed = SampleManager.GetChannelUsedFlag(m_bActiveSamples);
	transmission = params->m_pTransmission;
	velocityChange = params->m_fVelocityChange;
	relativeVelocityChange = 2.0f * velocityChange / transmission->fMaxVelocity;

	accelerationMultipler = 0.0f;

	if(relativeVelocityChange > 1.0f) accelerationMultipler = relativeVelocityChange;

	gasPedalAudio = accelerationMultipler;
	currentGear = params->m_pVehicle->m_nCurrentGear;
	if(transmission->nDriveType == '4') {
		wheelInUseCounter = 0;
		i = 0;
		do {
			if(automobile->m_aWheelState[i]) ++wheelInUseCounter;
			++i;
		} while(i < 4u);
		if(wheelInUseCounter > 2u) lostTraction = 1;
	} else if(transmission->nDriveType == 'F') {
		if((automobile->m_aWheelState[0] || automobile->m_aWheelState[2]) &&
		   (automobile->m_aWheelState[1] || automobile->m_aWheelState[3])) {
			lostTraction = 1;
		}
	} else if(transmission->nDriveType == 'R' &&
	          (automobile->m_aWheelState[1] || automobile->m_aWheelState[3])) {
		lostTraction = 1;
	}
	if(0.0f != velocityChange) {
		time = params->m_pVehicle->m_vecMoveSpeed.z / velocityChange;
		if(time <= 0.0f) {
			freqModifier = max(-0.2f, time) * -15000.f;
		} else {
			freqModifier = -(min(0.2f, time) * 15000.f);
		}
		if(params->m_fVelocityChange < -0.001f) freqModifier = -freqModifier;
	} else {
		freqModifier = 0;
	}
	engineSoundType = CarSounds[params->m_nIndex].m_bEngineSoundType;
	soundOffset = 3 * (engineSoundType - 1);
	if(accelerateState <= 0) {
		if(params->m_fVelocityChange < -0.001f) {
			if(channelUsed) {
				SampleManager.StopChannel(m_bActiveSamples);
				bAccelSampleStopped = 1;
			}
			if(!automobile->m_nWheelsOnGround || automobile->bIsHandbrakeOn ||
			   lostTraction) {
				gasPedalAudio = automobile->m_fGasPedalAudio;
			} else {
				gasPedalAudio =
				    min(1.0f, params->m_fVelocityChange /
				                  params->m_pTransmission->fMaxReverseVelocity);
			}
			gasPedalAudio = max(0.0f, gasPedalAudio);
			automobile->m_fGasPedalAudio = gasPedalAudio;
		} else if(LastAccel > 0) {
			if(channelUsed) {
				SampleManager.StopChannel(m_bActiveSamples);
				bAccelSampleStopped = 1;
			}
			nCruising = 0;
			if(!automobile->m_nWheelsOnGround || automobile->bIsHandbrakeOn ||
			   lostTraction ||
			   params->m_fVelocityChange >= 0.01f &&
			       automobile->m_fGasPedalAudio > 0.2f) {
				automobile->m_fGasPedalAudio = automobile->m_fGasPedalAudio * 0.6f;
				gasPedalAudio = automobile->m_fGasPedalAudio;
			}
			if(gasPedalAudio > 0.05f) {
				freq = (5000.f * (gasPedalAudio - 0.05f) * 20.f / 19) + 19000;
				if(engineSoundType == 6) freq >>= 1;
				AddPlayerCarSample(
				    (25.f * (gasPedalAudio - 0.05f) * 20.f / 19) + 40, freq,
				    (soundOffset + AUDIO_SAMPLE_VEHICLE_ENGINE_STOPPING_GENERIC),
				    engineSoundType, 63u, 0);
			}
		}
		freq = (10000.f * gasPedalAudio) + 22050;
		if(engineSoundType == 6) freq >>= 1;
		AddPlayerCarSample(110 - (40.f * gasPedalAudio), freq,
		                   (engineSoundType + AUDIO_SAMPLE_VEHICLE_ENGINE_IDLE_NONE), 0,
		                   52u, 1);

		CurrentPretendGear = max(1, currentGear);
		LastAccel = accelerateState;

		bHandbrakeOnLastFrame = automobile->bIsHandbrakeOn;
		bLostTractionLastFrame = lostTraction;
		return;
	}
	if(!nCruising) {
		if(accelerateState < 150 || !automobile->m_nWheelsOnGround ||
		   automobile->bIsHandbrakeOn || lostTraction ||
		   currentGear < 2u &&
		       velocityChange - automobile->m_fVelocityChangeForAudio >= 0.01f) {
			if(!automobile->m_nWheelsOnGround || automobile->bIsHandbrakeOn ||
			   lostTraction) {
				if(!automobile->m_nWheelsOnGround &&
				       automobile->m_nDriveWheelsOnGround ||
				   (automobile->bIsHandbrakeOn && !bHandbrakeOnLastFrame ||
				    lostTraction && !bLostTractionLastFrame) &&
				       automobile->m_nWheelsOnGround) {
					automobile->m_fGasPedalAudio =
					    automobile->m_fGasPedalAudio * 0.6f;
				}
				freqModifier = 0;
				baseFreq = (15000.f * automobile->m_fGasPedalAudio) + 14000;
				vol = (25.0f * automobile->m_fGasPedalAudio) + 60;
			} else {
				baseFreq = (8000.f * accelerationMultipler) + 16000;
				vol = (25.0f * accelerationMultipler) + 60;
				automobile->m_fGasPedalAudio = accelerationMultipler;
			}
			freq = freqModifier + baseFreq;
			if(engineSoundType == 6) freq >>= 1;
			if(channelUsed) {
				SampleManager.StopChannel(m_bActiveSamples);
				bAccelSampleStopped = 1;
			}
			AddPlayerCarSample(
			    vol, freq, (engineSoundType + AUDIO_SAMPLE_PAYPHONE_RINGING), 0, 2u, 1);
			LastAccel = accelerateState;

			bHandbrakeOnLastFrame = automobile->bIsHandbrakeOn;
			bLostTractionLastFrame = lostTraction;
			return;
		}
		TranslateEntity(&m_sQueueSample.m_vecPos, &pos);
		if(bAccelSampleStopped) {
			if(CurrentPretendGear != 1 || currentGear != 2) {
				gearNr = currentGear - 1;
				if(gearNr < 1) gearNr = 1;
				CurrentPretendGear = gearNr;
			}
			processedAccelSampleStopped = 1;
			bAccelSampleStopped = 0;
		}
		if(channelUsed) {
			SampleManager.SetChannelEmittingVolume(m_bActiveSamples, 85u);
			SampleManager.SetChannel3DPosition(m_bActiveSamples, pos.x, pos.y, pos.z);
			SampleManager.SetChannel3DDistances(m_bActiveSamples, 50.f, 12.5f);
			if(engineSoundType == 6)
				freq = (CSWTCH_554[CurrentPretendGear] + freqModifier + 22050) >> 1;
			else
				freq = CSWTCH_554[CurrentPretendGear] + freqModifier + 22050;
			SampleManager.SetChannelFrequency(m_bActiveSamples, freq);
			if(!channelUsed) {
				SampleManager.SetChannelReverbFlag(
				    m_bActiveSamples, m_bDynamicAcousticModelingStatus != 0);
				SampleManager.StartChannel(m_bActiveSamples);
			}
			LastAccel = accelerateState;

			bHandbrakeOnLastFrame = automobile->bIsHandbrakeOn;
			bLostTractionLastFrame = lostTraction;
			return;
		}
		if(processedAccelSampleStopped) {
			if(!SampleManager.InitialiseChannel(m_bActiveSamples, soundOffset + 345, 0))
				return;
			SampleManager.SetChannelLoopCount(m_bActiveSamples, 1);
			SampleManager.SetChannelLoopPoints(m_bActiveSamples, 0, -1);
			SampleManager.SetChannelEmittingVolume(m_bActiveSamples, 85u);
			SampleManager.SetChannel3DPosition(m_bActiveSamples, pos.x, pos.y, pos.z);
			SampleManager.SetChannel3DDistances(m_bActiveSamples, 50.f, 12.5f);
			if(engineSoundType == 6)
				freq = (CSWTCH_554[CurrentPretendGear] + freqModifier + 22050) >> 1;
			else
				freq = CSWTCH_554[CurrentPretendGear] + freqModifier + 22050;
			SampleManager.SetChannelFrequency(m_bActiveSamples, freq);
			if(!channelUsed) {
				SampleManager.SetChannelReverbFlag(
				    m_bActiveSamples, m_bDynamicAcousticModelingStatus != 0);
				SampleManager.StartChannel(m_bActiveSamples);
			}
			LastAccel = accelerateState;

			bHandbrakeOnLastFrame = automobile->bIsHandbrakeOn;
			bLostTractionLastFrame = lostTraction;
			return;
		}
		if(CurrentPretendGear < params->m_pTransmission->nNumberOfGears - 1) {
			++CurrentPretendGear;
			if(!SampleManager.InitialiseChannel(m_bActiveSamples, soundOffset + 345, 0))
				return;
			SampleManager.SetChannelLoopCount(m_bActiveSamples, 1);
			SampleManager.SetChannelLoopPoints(m_bActiveSamples, 0, -1);
			SampleManager.SetChannelEmittingVolume(m_bActiveSamples, 85u);
			SampleManager.SetChannel3DPosition(m_bActiveSamples, pos.x, pos.y, pos.z);
			SampleManager.SetChannel3DDistances(m_bActiveSamples, 50.f, 12.5f);
			freq = CSWTCH_554[CurrentPretendGear] + freqModifier + 22050;

			if(engineSoundType == 6) freq >>= 1;

			SampleManager.SetChannelFrequency(m_bActiveSamples, freq);
			if(!channelUsed) {
				SampleManager.SetChannelReverbFlag(
				    m_bActiveSamples, m_bDynamicAcousticModelingStatus != 0);
				SampleManager.StartChannel(m_bActiveSamples);
			}
			LastAccel = accelerateState;

			bHandbrakeOnLastFrame = automobile->bIsHandbrakeOn;
			bLostTractionLastFrame = lostTraction;
			return;
		}
		nCruising = 1;
	}
	bAccelSampleStopped = 1;
	if(accelerateState < 150 || !automobile->m_nWheelsOnGround || automobile->bIsHandbrakeOn ||
	   lostTraction || currentGear < params->m_pTransmission->nNumberOfGears - 1) {
		nCruising = 0;
	} else {
		if(accelerateState >= 220 &&
		   0.001f + params->m_fVelocityChange < automobile->m_fVelocityChangeForAudio) {
			if(nCruising < 800) ++nCruising;
		} else if(nCruising > 3) {
			--nCruising;
		}
		freq = 27 * nCruising + freqModifier + 22050;
		if(engineSoundType == 6) freq >>= 1;
		AddPlayerCarSample(85u, freq,
		                   (soundOffset + AUDIO_SAMPLE_VEHICLE_ENGINE_CHANGE_GEAR_GENERIC),
		                   engineSoundType, 64u, 1);
	}
	LastAccel = accelerateState;

	bHandbrakeOnLastFrame = automobile->bIsHandbrakeOn;
	bLostTractionLastFrame = lostTraction;
}

void
cAudioManager::ProcessPoliceCellBeatingScriptObject(uint8 sound)
{
	uint32 time = CTimer::GetTimeInMilliseconds();
	int32 sampleIndex;
	uint8 emittingVol;
	float distSquared;
	float maxDist;
	cPedParams params;

	static uint8 counter = 0;

	if(time > audioLogicTimers[7]) {
		switch(sound) {
		case SCRIPT_SOUND_POLICE_CELL_BEATING_LOOP_S:
			maxDist = 900.f;
			m_sQueueSample.m_fSoundIntensity = 30.0f;
			break;
		case SCRIPT_SOUND_POLICE_CELL_BEATING_LOOP_L:
			maxDist = 6400.f;
			m_sQueueSample.m_fSoundIntensity = 80.0f;
			break;
		default: return;
		}
		distSquared = GetDistanceSquared(&m_sQueueSample.m_vecPos);
		if(distSquared < maxDist) {
			m_sQueueSample.m_fDistance = sqrt(distSquared);
			if(m_nTimeOfRecentCrime & 1)
				sampleIndex = (m_anRandomTable[1] & 3) + AUDIO_SAMPLE_PED_HIT_1;
			else
				sampleIndex =
				    (m_anRandomTable[3] & 1) + AUDIO_SAMPLE_PED_HIT_BY_BAT;
			m_sQueueSample.m_nSampleIndex = sampleIndex;
			emittingVol = m_anRandomTable[0] % 50u + 55;
			m_sQueueSample.m_bVolume =
			    ComputeVolume(emittingVol, m_sQueueSample.m_fSoundIntensity,
			                  m_sQueueSample.m_fDistance);
			if(m_sQueueSample.m_bVolume) {
				m_sQueueSample.m_bBankIndex = 0;
				m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(
				    m_sQueueSample.m_nSampleIndex);
				m_sQueueSample.m_nFrequency +=
				    RandomDisplacement(m_sQueueSample.m_nFrequency / 16);
				m_sQueueSample.m_counter = counter++;
				m_sQueueSample.m_bIsDistant = 0;
				m_sQueueSample.m_nLoopCount = 1;
				m_sQueueSample.field_56 = 1;
				m_sQueueSample.field_16 = 3;
				m_sQueueSample.field_48 = 0.0f;
				m_sQueueSample.m_bEmittingVolume = emittingVol;
				m_sQueueSample.m_nLoopStart = 0;
				m_sQueueSample.m_nLoopEnd = -1;
				m_sQueueSample.m_bReverbFlag = 1;
				m_sQueueSample.m_bRequireReflection = 0;
				AddSampleToRequestedQueue();
				params.m_bDistanceCalculated = 1;
				params.m_fDistance = distSquared;
				params.m_pPed = 0;
				SetupPedComments(&params, SOUND_8A);
			}
			audioLogicTimers[7] = time + 500 + m_anRandomTable[3] % 1500u;
		}
	}
}

void
cAudioManager::ProcessPornCinema(uint8 sound)
{

	eAudioSamples sample;
	uint32 time;
	int32 rand;
	float distSquared;
	float maxDist;

	switch(sound) {
	case SCRIPT_SOUND_PORN_CINEMA_1_S:
	case SCRIPT_SOUND_MISTY_SEX_S:
		m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_PORN_CINEMA_1_BACKGROUND_1;
		m_sQueueSample.m_bBankIndex = 0;
		maxDist = 400.f;
		sample = AUDIO_SAMPLE_PORN_CINEMA_1_SEX_1;
		m_sQueueSample.m_fSoundIntensity = 20.0f;
		break;
	case SCRIPT_SOUND_PORN_CINEMA_1_L:
	case SCRIPT_SOUND_MISTY_SEX_L:
		m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_PORN_CINEMA_1_BACKGROUND_1;
		m_sQueueSample.m_bBankIndex = 0;
		maxDist = 6400.f;
		sample = AUDIO_SAMPLE_PORN_CINEMA_1_SEX_1;
		m_sQueueSample.m_fSoundIntensity = 80.0f;
		break;
	case SCRIPT_SOUND_PORN_CINEMA_2_S:
		m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_PORN_CINEMA_2_BACKGROUND_2;
		m_sQueueSample.m_bBankIndex = 0;
		maxDist = 400.f;
		sample = AUDIO_SAMPLE_PORN_CINEMA_2_SEX_1;
		m_sQueueSample.m_fSoundIntensity = 20.0f;
		break;
	case SCRIPT_SOUND_PORN_CINEMA_2_L:
		m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_PORN_CINEMA_2_BACKGROUND_2;
		m_sQueueSample.m_bBankIndex = 0;
		maxDist = 6400.f;
		sample = AUDIO_SAMPLE_PORN_CINEMA_2_SEX_1;
		m_sQueueSample.m_fSoundIntensity = 80.0f;
		break;
	case SCRIPT_SOUND_PORN_CINEMA_3_S:
		m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_PORN_CINEMA_3_BACKGROUND_3;
		m_sQueueSample.m_bBankIndex = 0;
		maxDist = 400.f;
		m_sQueueSample.m_fSoundIntensity = 20.0f;
		sample = AUDIO_SAMPLE_PORN_CINEMA_3_SEX_1;
		break;
	case SCRIPT_SOUND_PORN_CINEMA_3_L:
		m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_PORN_CINEMA_3_BACKGROUND_3;
		m_sQueueSample.m_bBankIndex = 0;
		maxDist = 6400.f;
		m_sQueueSample.m_fSoundIntensity = 80.0f;
		sample = AUDIO_SAMPLE_PORN_CINEMA_3_SEX_1;
		break;
	default: return;
	}
	distSquared = GetDistanceSquared(&m_sQueueSample.m_vecPos);
	if(distSquared < maxDist) {
		m_sQueueSample.m_fDistance = sqrt(distSquared);
		if(sound != SCRIPT_SOUND_MISTY_SEX_S && sound != SCRIPT_SOUND_MISTY_SEX_L) {
			m_sQueueSample.m_bVolume =
			    ComputeVolume(maxVolume, m_sQueueSample.m_fSoundIntensity,
			                  m_sQueueSample.m_fDistance);
			if(m_sQueueSample.m_bVolume) {
				m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(
				    m_sQueueSample.m_nSampleIndex);
				m_sQueueSample.m_counter = 0;
				m_sQueueSample.m_bIsDistant = 0;
				m_sQueueSample.m_nLoopCount = 0;
				m_sQueueSample.field_56 = 0;
				m_sQueueSample.field_16 = 3;
				m_sQueueSample.field_48 = 2.0f;
				m_sQueueSample.m_bEmittingVolume = maxVolume;
				m_sQueueSample.m_nLoopStart =
				    SampleManager.GetSampleLoopStartOffset(
				        m_sQueueSample.m_nSampleIndex);
				m_sQueueSample.m_nLoopEnd = SampleManager.GetSampleLoopEndOffset(
				    m_sQueueSample.m_nSampleIndex);
				m_sQueueSample.m_bReverbFlag = 1;
				m_sQueueSample.m_bRequireReflection = 0;
				AddSampleToRequestedQueue();
			}
		}

		time = CTimer::GetTimeInMilliseconds();
		if(time > audioLogicTimers[0]) {
			m_sQueueSample.m_bVolume = ComputeVolume(
			    90u, m_sQueueSample.m_fSoundIntensity, m_sQueueSample.m_fDistance);
			if(m_sQueueSample.m_bVolume) {
				rand = m_anRandomTable[1] & 1;
				m_sQueueSample.m_nSampleIndex = rand + sample;
				m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(
				    m_sQueueSample.m_nSampleIndex);
				m_sQueueSample.m_nFrequency +=
				    RandomDisplacement(m_sQueueSample.m_nFrequency >> 4);
				m_sQueueSample.m_counter = rand + 1;
				m_sQueueSample.m_bIsDistant = 0;
				m_sQueueSample.m_nLoopCount = 1;
				m_sQueueSample.field_56 = 1;
				m_sQueueSample.field_16 = 6;
				m_sQueueSample.field_48 = 0.0f;
				m_sQueueSample.m_nLoopStart = 0;
				m_sQueueSample.m_nLoopEnd = -1;
				m_sQueueSample.m_bReverbFlag = 1;
				m_sQueueSample.m_bRequireReflection = 0;
				AddSampleToRequestedQueue();
				audioLogicTimers[0] = time + 2000 + m_anRandomTable[3] % 6000u;
			}
		}
	}
}

void
cAudioManager::ProcessProjectiles()
{
	uint8 emittingVol;
	float maxDist;

	for(int32 i = 0; i < 32; i++) {
		if(CProjectileInfo::GetProjectileInfo(i)->m_bInUse) {
			switch(CProjectileInfo::GetProjectileInfo(i)->m_eWeaponType) {
			case WEAPONTYPE_ROCKETLAUNCHER:
				emittingVol = maxVolume;
				maxDist = 8100.f;
				m_sQueueSample.m_fSoundIntensity = 90.0;
				m_sQueueSample.m_nSampleIndex = 81;
				m_sQueueSample.m_bBankIndex = 0;
				m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(
				    AUDIO_SAMPLE_WEAPON_ROCKET_PROCESS);
				m_sQueueSample.field_16 = 3;
				break;
			case WEAPONTYPE_MOLOTOV:
				emittingVol = 50;
				maxDist = 900.f;
				m_sQueueSample.m_fSoundIntensity = 30.0;
				m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_FIRE_PED;
				m_sQueueSample.m_bBankIndex = 0;
				m_sQueueSample.m_nFrequency =
				    32 *
				    SampleManager.GetSampleBaseFrequency(AUDIO_SAMPLE_FIRE_PED) /
				    25;
				m_sQueueSample.field_16 = 7;
				break;
			default: return;
			}
			m_sQueueSample.field_48 = 4.0;
			m_sQueueSample.field_76 = 3;
			m_sQueueSample.m_vecPos =
			    CProjectileInfo::ms_apProjectile[i].GetPosition();
			float distSquared = GetDistanceSquared(&m_sQueueSample.m_vecPos);
			if(distSquared < maxDist) {
				m_sQueueSample.m_fDistance = Sqrt(distSquared);
				m_sQueueSample.m_bVolume =
				    ComputeVolume(emittingVol, m_sQueueSample.m_fSoundIntensity,
				                  m_sQueueSample.m_fDistance);
				if(m_sQueueSample.m_bVolume) {
					m_sQueueSample.m_counter = i;
					m_sQueueSample.m_bIsDistant = 0;
					m_sQueueSample.m_nLoopCount = 0;
					m_sQueueSample.m_bEmittingVolume = emittingVol;
					m_sQueueSample.m_nLoopStart =
					    SampleManager.GetSampleLoopStartOffset(
					        m_sQueueSample.m_nSampleIndex);
					m_sQueueSample.m_nLoopEnd =
					    SampleManager.GetSampleLoopEndOffset(
					        m_sQueueSample.m_nSampleIndex);
					m_sQueueSample.field_56 = 0;
					m_sQueueSample.m_bReverbFlag = 1;
					m_sQueueSample.m_bRequireReflection = 0;
					AddSampleToRequestedQueue();
				}
			}
		}
	}
}

void
cAudioManager::ProcessRainOnVehicle(cVehicleParams *params)
{
	float emittingVol;
	CVehicle *veh;

	if(params->m_fDistance < 484.f && CWeather::Rain > 0.01f &&
	   (!CCullZones::CamNoRain() || !CCullZones::PlayerNoRain())) {
		++params->m_pVehicle->m_bRainAudioCounter;
		veh = params->m_pVehicle;
		if(veh->m_bRainAudioCounter >= 2u) {
			veh->m_bRainAudioCounter = 0;
			CalculateDistance((bool *)params, params->m_fDistance);
			emittingVol = 30.f * CWeather::Rain;
			m_sQueueSample.m_bVolume =
			    ComputeVolume(emittingVol, 22.f, m_sQueueSample.m_fDistance);
			if(m_sQueueSample.m_bVolume) {
				m_sQueueSample.m_counter = veh->m_bRainSamplesCounter++;
				veh = params->m_pVehicle;
				if(veh->m_bRainSamplesCounter > 4u) veh->m_bRainSamplesCounter = 68;
				m_sQueueSample.m_nSampleIndex =
				    (m_anRandomTable[1] & 3) + AUDIO_SAMPLE_RAIN_ON_VEHICLE_1;
				m_sQueueSample.m_bBankIndex = 0;
				m_sQueueSample.m_bIsDistant = 0;
				m_sQueueSample.field_16 = 9;
				m_sQueueSample.m_nFrequency = m_anRandomTable[1] % 4000u + 28000;
				m_sQueueSample.m_nLoopCount = 1;
				m_sQueueSample.m_bEmittingVolume = (uint8)emittingVol;
				m_sQueueSample.m_nLoopStart = 0;
				m_sQueueSample.m_nLoopEnd = -1;
				m_sQueueSample.field_48 = 0.0f;
				m_sQueueSample.m_fSoundIntensity = 22.0f;
				m_sQueueSample.field_56 = 1;
				m_sQueueSample.m_bReverbFlag = 0;
				m_sQueueSample.m_bRequireReflection = 0;
				AddSampleToRequestedQueue();
			}
		}
	}
}

void
cAudioManager::ProcessReverb() const
{
	if(SampleManager.UpdateReverb() && m_bDynamicAcousticModelingStatus) {
		for(uint32 i = 0; i < 28; i++) {
			if(m_asActiveSamples[i].m_bReverbFlag)
				SampleManager.SetChannelReverbFlag(i, 1);
		}
	}
}

bool
cAudioManager::ProcessReverseGear(cVehicleParams *params)
{
	CVehicle *veh;
	CAutomobile *automobile;
	int32 emittingVol;
	float modificator;

	if(params->m_fDistance >= 900.f) return 0;
	veh = params->m_pVehicle;
	if(veh->bEngineOn && (veh->m_fGasPedal < 0.0f || !veh->m_nCurrentGear)) {
		CalculateDistance((bool *)params, params->m_fDistance);
		automobile = (CAutomobile *)params->m_pVehicle;
		if(automobile->m_nWheelsOnGround) {
			modificator = params->m_fVelocityChange /
			              params->m_pTransmission->fMaxReverseVelocity;
		} else {
			if(automobile->m_nDriveWheelsOnGround)
				automobile->m_fGasPedalAudio = automobile->m_fGasPedalAudio * 0.4f;
			modificator = automobile->m_fGasPedalAudio;
		}
		modificator = Abs(modificator);
		emittingVol = (24.f * modificator);
		m_sQueueSample.m_bVolume =
		    ComputeVolume(emittingVol, 30.f, m_sQueueSample.m_fDistance);
		if(m_sQueueSample.m_bVolume) {
			if(params->m_pVehicle->m_fGasPedal >= 0.0f) {
				m_sQueueSample.m_counter = 62;
				m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_REVERSE_GEAR_CONSTANT;
			} else {
				m_sQueueSample.m_counter = 61;
				m_sQueueSample.m_nSampleIndex =
				    AUDIO_SAMPLE_REVERSE_GEAR_ACCELERATING;
			}
			m_sQueueSample.m_bBankIndex = 0;
			m_sQueueSample.m_bIsDistant = 0;
			m_sQueueSample.field_16 = 3;
			m_sQueueSample.m_nFrequency = (6000.f * modificator) + 7000;
			m_sQueueSample.m_nLoopCount = 0;
			m_sQueueSample.m_bEmittingVolume = emittingVol;
			m_sQueueSample.m_nLoopStart =
			    SampleManager.GetSampleLoopStartOffset(m_sQueueSample.m_nSampleIndex);
			m_sQueueSample.m_nLoopEnd =
			    SampleManager.GetSampleLoopEndOffset(m_sQueueSample.m_nSampleIndex);
			m_sQueueSample.field_48 = 3.0f;
			m_sQueueSample.m_fSoundIntensity = 30.0f;
			m_sQueueSample.field_56 = 0;
			m_sQueueSample.field_76 = 5;
			m_sQueueSample.m_bReverbFlag = 1;
			m_sQueueSample.m_bRequireReflection = 0;
			AddSampleToRequestedQueue();
		}
	}
	return 1;
}

void
cAudioManager::ProcessSawMillScriptObject(uint8 sound)
{
	uint32 time;
	float distSquared;
	float maxDist;

	switch(sound) {
	case SCRIPT_SOUND_SAWMILL_LOOP_S:
	case SCRIPT_SOUND_SAWMILL_LOOP_L:
		maxDist = 900.f;
		m_sQueueSample.m_fSoundIntensity = 30.0f;
		break;
	default: return;
	}
	distSquared = GetDistanceSquared(&m_sQueueSample.m_vecPos);
	if(distSquared < maxDist) {
		m_sQueueSample.m_fDistance = sqrt(distSquared);
		m_sQueueSample.m_bVolume = ComputeVolume(30u, m_sQueueSample.m_fSoundIntensity,
		                                         m_sQueueSample.m_fDistance);
		if(m_sQueueSample.m_bVolume) {
			m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_SAWMILL_1;
			m_sQueueSample.m_bBankIndex = 0;
			m_sQueueSample.m_nFrequency =
			    SampleManager.GetSampleBaseFrequency(AUDIO_SAMPLE_SAWMILL_1);
			m_sQueueSample.m_counter = 0;
			m_sQueueSample.m_bIsDistant = 0;
			m_sQueueSample.m_nLoopCount = 0;
			m_sQueueSample.field_56 = 0;
			m_sQueueSample.field_16 = 5;
			m_sQueueSample.field_48 = 2.0f;
			m_sQueueSample.m_bEmittingVolume = 30;
			m_sQueueSample.m_nLoopStart =
			    SampleManager.GetSampleLoopStartOffset(m_sQueueSample.m_nSampleIndex);
			m_sQueueSample.m_nLoopEnd =
			    SampleManager.GetSampleLoopEndOffset(m_sQueueSample.m_nSampleIndex);
			m_sQueueSample.m_bReverbFlag = 1;
			m_sQueueSample.m_bRequireReflection = 0;
			AddSampleToRequestedQueue();
		}
		time = CTimer::GetTimeInMilliseconds();
		if(time > audioLogicTimers[1]) {
			m_sQueueSample.m_bVolume = ComputeVolume(
			    70u, m_sQueueSample.m_fSoundIntensity, m_sQueueSample.m_fDistance);
			if(m_sQueueSample.m_bVolume) {
				m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_SAWMILL_2;
				m_sQueueSample.m_bBankIndex = 0;
				m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(
				    m_sQueueSample.m_nSampleIndex);
				m_sQueueSample.m_counter = 1;
				m_sQueueSample.m_bIsDistant = 0;
				m_sQueueSample.m_nLoopCount = 1;
				m_sQueueSample.field_56 = 1;
				m_sQueueSample.field_16 = 3;
				m_sQueueSample.field_48 = 2.0f;
				m_sQueueSample.m_nLoopStart = 0;
				m_sQueueSample.m_nLoopEnd = -1;
				m_sQueueSample.m_bReverbFlag = 1;
				m_sQueueSample.m_bRequireReflection = 0;
				AddSampleToRequestedQueue();
				audioLogicTimers[1] = time + 2000 + m_anRandomTable[3] % 4000u;
			}
		}
	}
}

void
cAudioManager::ProcessScriptObject(int32 id)
{
	cAudioScriptObject *entity = (cAudioScriptObject *)m_asAudioEntities[id].m_pEntity;
	if(entity) {
		m_sQueueSample.m_vecPos = entity->Posn;
		if(m_asAudioEntities[id].m_Loops == 1)
			ProcessOneShotScriptObject(m_asAudioEntities[id].m_awAudioEvent[0]);
		else
			ProcessLoopingScriptObject(entity->AudioId);
	}
}

void
cAudioManager::ProcessShopScriptObject(uint8 sound)
{
	uint32 time;
	int32 rand;
	float distSquared;
	float maxDist;

	switch(sound) {
	case SCRIPT_SOUND_SHOP_LOOP_S:
	case SCRIPT_SOUND_SHOP_LOOP_L:
		maxDist = 900.f;
		m_sQueueSample.m_fSoundIntensity = 30.0f;
		break;
	default: return;
	}
	distSquared = GetDistanceSquared(&m_sQueueSample.m_vecPos);
	if(distSquared < maxDist) {
		m_sQueueSample.m_fDistance = sqrt(distSquared);
		m_sQueueSample.m_bVolume = ComputeVolume(30u, m_sQueueSample.m_fSoundIntensity,
		                                         m_sQueueSample.m_fDistance);
		if(m_sQueueSample.m_bVolume) {
			m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_SHOP_1;
			m_sQueueSample.m_bBankIndex = 0;
			m_sQueueSample.m_nFrequency =
			    SampleManager.GetSampleBaseFrequency(AUDIO_SAMPLE_SHOP_1);
			m_sQueueSample.m_counter = 0;
			m_sQueueSample.m_bIsDistant = 0;
			m_sQueueSample.m_nLoopCount = 0;
			m_sQueueSample.field_56 = 0;
			m_sQueueSample.field_16 = 5;
			m_sQueueSample.field_48 = 2.0f;
			m_sQueueSample.m_bEmittingVolume = 30;
			m_sQueueSample.m_nLoopStart =
			    SampleManager.GetSampleLoopStartOffset(m_sQueueSample.m_nSampleIndex);
			m_sQueueSample.m_nLoopEnd =
			    SampleManager.GetSampleLoopEndOffset(m_sQueueSample.m_nSampleIndex);
			m_sQueueSample.m_bReverbFlag = 1;
			m_sQueueSample.m_bRequireReflection = 0;
			AddSampleToRequestedQueue();
		}
		time = CTimer::GetTimeInMilliseconds();
		if(time > audioLogicTimers[2]) {
			m_sQueueSample.m_bVolume = ComputeVolume(
			    70u, m_sQueueSample.m_fSoundIntensity, m_sQueueSample.m_fDistance);
			if(m_sQueueSample.m_bVolume) {
				rand = m_anRandomTable[1] & 1;
				m_sQueueSample.m_nSampleIndex = rand + AUDIO_SAMPLE_SHOP_2;
				m_sQueueSample.m_bBankIndex = 0;
				m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(
				    m_sQueueSample.m_nSampleIndex);
				m_sQueueSample.m_counter = rand + 1;
				m_sQueueSample.m_bIsDistant = 0;
				m_sQueueSample.m_nLoopCount = 1;
				m_sQueueSample.field_56 = 1;
				m_sQueueSample.field_16 = 3;
				m_sQueueSample.field_48 = 2.0f;
				m_sQueueSample.m_bEmittingVolume = 70;
				m_sQueueSample.m_nLoopStart = 0;
				m_sQueueSample.m_nLoopEnd = -1;
				m_sQueueSample.m_bReverbFlag = 1;
				m_sQueueSample.m_bRequireReflection = 0;
				AddSampleToRequestedQueue();
				audioLogicTimers[2] = time + 3000 + m_anRandomTable[3] % 7000u;
			}
		}
	}
}

void
cAudioManager::ProcessSpecial()
{
	if(m_bUserPause) {
		if(!m_bPreviousUserPause) {
			MusicManager.ChangeMusicMode(0);
			SampleManager.SetEffectsFadeVolume(maxVolume);
			SampleManager.SetMusicFadeVolume(maxVolume);
		}
	} else {
		if(m_bPreviousUserPause) {
			MusicManager.StopFrontEndTrack();
			MusicManager.ChangeMusicMode(1u);
		}
		CPlayerPed *playerPed = FindPlayerPed();
		if(playerPed) {
			const PedState &state = playerPed->m_nPedState;
			if(state != PED_ENTER_CAR && state != PED_STEAL_CAR &&
			   !playerPed->bInVehicle)
				SampleManager.StopChannel(m_bActiveSamples);
		}
	}
}

bool
cAudioManager::ProcessTrainNoise(cVehicleParams *params)
{
	CTrain *train;
	uint8 emittingVol;
	float speedMultipler;

	if(params->m_fDistance >= 90000.f) return 0;

	if(params->m_fVelocityChange > 0.0f) {
		CalculateDistance((bool *)params, params->m_fDistance);
		train = (CTrain *)params->m_pVehicle;
		speedMultipler = min(1.0f, train->m_fSpeed * 250.f / 51.f);
		emittingVol = (75.f * speedMultipler);
		if(train->m_fWagonPosition == 0.0f) {
			m_sQueueSample.m_bVolume =
			    ComputeVolume(emittingVol, 300.f, m_sQueueSample.m_fDistance);
			if(m_sQueueSample.m_bVolume) {
				m_sQueueSample.m_counter = 32;
				m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_TRAIN_FAR;
				m_sQueueSample.m_bBankIndex = 0;
				m_sQueueSample.m_bIsDistant = 0;
				m_sQueueSample.field_16 = 2;
				m_sQueueSample.m_nFrequency =
				    SampleManager.GetSampleBaseFrequency(AUDIO_SAMPLE_TRAIN_FAR);
				m_sQueueSample.m_nLoopCount = 0;
				m_sQueueSample.m_bEmittingVolume = emittingVol;
				m_sQueueSample.m_nLoopStart =
				    SampleManager.GetSampleLoopStartOffset(
				        m_sQueueSample.m_nSampleIndex);
				m_sQueueSample.m_nLoopEnd = SampleManager.GetSampleLoopEndOffset(
				    m_sQueueSample.m_nSampleIndex);
				m_sQueueSample.field_48 = 3.0f;
				m_sQueueSample.m_fSoundIntensity = 300.0f;
				m_sQueueSample.field_56 = 0;
				m_sQueueSample.field_76 = 3;
				m_sQueueSample.m_bReverbFlag = 1;
				m_sQueueSample.m_bRequireReflection = 0;
				AddSampleToRequestedQueue();
			}
		}
		if(params->m_fDistance < 4900.f) {
			m_sQueueSample.m_bVolume =
			    ComputeVolume(emittingVol, 70.f, m_sQueueSample.m_fDistance);
			if(m_sQueueSample.m_bVolume) {
				m_sQueueSample.m_counter = 33;
				m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_TRAIN_CLOSE;
				m_sQueueSample.m_bBankIndex = 0;
				m_sQueueSample.m_bIsDistant = 0;
				m_sQueueSample.field_16 = 5;
				m_sQueueSample.m_nFrequency =
				    SampleManager.GetSampleBaseFrequency(AUDIO_SAMPLE_TRAIN_CLOSE) +
				    100 * m_sQueueSample.m_nEntityIndex % 987;
				m_sQueueSample.m_nLoopCount = 0;
				m_sQueueSample.m_bEmittingVolume = emittingVol;
				m_sQueueSample.m_nLoopStart = SampleManager.GetSampleLoopStartOffset(m_sQueueSample.m_nSampleIndex);
				m_sQueueSample.m_nLoopEnd = SampleManager.GetSampleLoopEndOffset(m_sQueueSample.m_nSampleIndex);
				m_sQueueSample.field_48 = 6.0f;
				m_sQueueSample.m_fSoundIntensity = 70.0f;
				m_sQueueSample.field_56 = 0;
				m_sQueueSample.field_76 = 3;
				m_sQueueSample.m_bReverbFlag = 1;
				m_sQueueSample.m_bRequireReflection = 0;
				AddSampleToRequestedQueue();
			}
		}
	}
	return 1;
}


void
cAudioManager::ProcessVehicle(CVehicle *veh)
{
	tHandlingData *handling = veh->pHandling;
	float velChange;
	cVehicleParams params;
	m_sQueueSample.m_vecPos = veh->GetPosition();

	params.m_bDistancECalculated = 0;
	params.m_fDistance = GetDistanceSquared(&m_sQueueSample.m_vecPos);
	params.m_pVehicle = veh;
	params.m_pTransmission = nil;
	params.m_nIndex = 0;
	params.m_fVelocityChange = 0.0;

	if(handling) params.m_pTransmission = &handling->Transmission;

	params.m_nIndex = veh->m_modelIndex - 90;
	if(params.m_pVehicle->m_status == STATUS_SIMPLE)
		velChange = params.m_pVehicle->AutoPilot.m_fMaxTrafficSpeed * 0.02f;
	else
		velChange =
		    DotProduct(params.m_pVehicle->m_vecMoveSpeed, params.m_pVehicle->GetForward());
	params.m_fVelocityChange = velChange;
	switch(params.m_pVehicle->m_vehType) {
	case VEHICLE_TYPE_AUTOMOBILE:
		UpdateGasPedalAudio((CAutomobile *)veh);
		if(params.m_nIndex == RCBANDIT) {
			ProcessModelCarEngine(&params);
			ProcessVehicleOneShots(&params);
				((CAutomobile *)veh)->m_fVelocityChangeForAudio =
				    params.m_fVelocityChange;
			ProcessRainOnVehicle(&params);
			break;
		}
		if(params.m_nIndex == DODO) {
			if(!ProcessVehicleRoadNoise(&params)) {
				ProcessVehicleOneShots(&params);
				((CAutomobile *)veh)->m_fVelocityChangeForAudio =
				    params.m_fVelocityChange;
				ProcessRainOnVehicle(&params);
				break;
			}
			if(CWeather::WetRoads > 0.f) ProcessWetRoadNoise(&params);
			ProcessVehicleSkidding(&params);
		} else {
			if(!ProcessVehicleRoadNoise(&params)) {
				ProcessVehicleOneShots(&params);
				((CAutomobile *)veh)->m_fVelocityChangeForAudio =
				    params.m_fVelocityChange;
				ProcessRainOnVehicle(&params);
				break;
			}
			ProcessReverseGear(&params);
			if(CWeather::WetRoads > 0.f) ProcessWetRoadNoise(&params);
			ProcessVehicleSkidding(&params);
			ProcessVehicleHorn(&params);
			ProcessVehicleSirenOrAlarm(&params);
			if(UsesReverseWarning(params.m_nIndex))
				ProcessVehicleReverseWarning(&params);
			if(HasAirBrakes(params.m_nIndex)) ProcessAirBrakes(&params);
		}
		ProcessCarBombTick(&params);
		ProcessVehicleEngine(&params);
		ProcessEngineDamage(&params);
		ProcessVehicleDoors(&params);
		ProcessVehicleOneShots(&params);
		((CAutomobile *)veh)->m_fVelocityChangeForAudio = params.m_fVelocityChange;
		ProcessRainOnVehicle(&params);
		break;
	case VEHICLE_TYPE_BOAT:
		ProcessBoatEngine(&params);
		ProcessBoatMovingOverWater(&params);
		ProcessVehicleOneShots(&params);
		ProcessRainOnVehicle(&params);
		break;
	case VEHICLE_TYPE_TRAIN:
		ProcessTrainNoise(&params);
		ProcessVehicleOneShots(&params);
		ProcessRainOnVehicle(&params);
		break;
	case VEHICLE_TYPE_HELI:
		ProcessHelicopter(&params);
		ProcessVehicleOneShots(&params);
		ProcessRainOnVehicle(&params);
		break;
	case VEHICLE_TYPE_PLANE:
		ProcessPlane(&params);
		ProcessVehicleOneShots(&params);
		ProcessRainOnVehicle(&params);
		break;
	default: ProcessRainOnVehicle(&params); break;
	}
}

bool
cAudioManager::ProcessVehicleDoors(cVehicleParams *params)
{
	CAutomobile *automobile;
	int8 doorState;
	int32 emittingVol;
	float velocity;

	if(params->m_fDistance >= 1600.f) return 0;

	automobile = (CAutomobile *)params->m_pVehicle;
	CalculateDistance((bool *)params, params->m_fDistance);
	for(int32 i = 0; i < 6; i++) {
		if(automobile->Damage.GetDoorStatus(i) == 2) {
			doorState = automobile->Doors[i].m_nDoorState;
			if(doorState == 1 || doorState == 2) {
				velocity = min(0.3f, Abs(automobile->Doors[i].m_fAngVel));
				if(velocity > 0.0035f) {
					emittingVol = (100.f * velocity * 10.f / 3.f);
					m_sQueueSample.m_bVolume = ComputeVolume(
					    emittingVol, 40.f, m_sQueueSample.m_fDistance);
					if(m_sQueueSample.m_bVolume) {
						m_sQueueSample.m_counter = i + 6;
						m_sQueueSample.m_nSampleIndex =
						    m_anRandomTable[1] % 6u +
						    AUDIO_SAMPLE_CAR_DOOR_MOVEMENT_1;
						m_sQueueSample.m_nFrequency =
						    SampleManager.GetSampleBaseFrequency(
						        m_sQueueSample.m_nSampleIndex) +
						    RandomDisplacement(1000u);
						m_sQueueSample.m_bBankIndex = 0;
						m_sQueueSample.m_bIsDistant = 0;
						m_sQueueSample.field_16 = 10;
						m_sQueueSample.m_nLoopCount = 1;
						m_sQueueSample.m_bEmittingVolume = emittingVol;
						m_sQueueSample.m_nLoopStart = 0;
						m_sQueueSample.m_nLoopEnd = -1;
						m_sQueueSample.field_48 = 1.0f;
						m_sQueueSample.m_fSoundIntensity = 40.0f;
						m_sQueueSample.field_56 = 1;
						m_sQueueSample.m_bReverbFlag = 1;
						m_sQueueSample.m_bRequireReflection = 1;
						AddSampleToRequestedQueue();
					}
				}
			}
		}
	}
    return 1;
}

WRAPPER
bool cAudioManager::ProcessVehicleEngine(cVehicleParams *params)
{
	EAXJMP(0x56A610);
}

void
cAudioManager::ProcessVehicleHorn(cVehicleParams *params)
{
	CAutomobile *automobile;

	if(params->m_fDistance < 1600.f) {
		automobile = (CAutomobile *)params->m_pVehicle;
		if((!automobile->m_bSirenOrAlarm || !UsesSirenSwitching(params->m_nIndex)) &&
		   automobile->m_modelIndex != MI_MRWHOOP) {
			if(automobile->m_nCarHornTimer) {
				if(!params->m_pVehicle->m_status) {
					CalculateDistance((bool *)params, params->m_fDistance);
					m_sQueueSample.m_bVolume =
					    ComputeVolume(80, 40.f, m_sQueueSample.m_fDistance);
					if(m_sQueueSample.m_bVolume) {
						m_sQueueSample.m_counter = 4;
						m_sQueueSample.m_nSampleIndex =
						    CarSounds[params->m_nIndex].m_nHornSample;
						m_sQueueSample.m_bBankIndex = 0;
						m_sQueueSample.m_bIsDistant = 0;
						m_sQueueSample.field_16 = 2;
						m_sQueueSample.m_nFrequency =
						    CarSounds[params->m_nIndex].m_nHornFrequency;
						m_sQueueSample.m_nLoopCount = 0;
						m_sQueueSample.m_bEmittingVolume = 80;
						m_sQueueSample.m_nLoopStart =
						    SampleManager.GetSampleLoopStartOffset(
						        m_sQueueSample.m_nSampleIndex);
						m_sQueueSample.m_nLoopEnd =
						    SampleManager.GetSampleLoopEndOffset(
						        m_sQueueSample.m_nSampleIndex);
						m_sQueueSample.field_48 = 5.0;
						m_sQueueSample.m_fSoundIntensity = 40.0;
						m_sQueueSample.field_56 = 0;
						m_sQueueSample.field_76 = 3;
						m_sQueueSample.m_bReverbFlag = 1;
						m_sQueueSample.m_bRequireReflection = 0;
						AddSampleToRequestedQueue();
					}
					return;
				}
				if(automobile->m_nCarHornTimer > 44)
					automobile->m_nCarHornTimer = 44;
				if(automobile->m_nCarHornTimer == 44)
					automobile->field_22D =
					    (LOBYTE(m_nTimeOfRecentCrime) +
					     LOBYTE(m_sQueueSample.m_nEntityIndex)) &
					    7;
				if(somethingWithHorns[44 * automobile->field_22D + 44 -
				                      automobile->m_nCarHornTimer]) {
					CalculateDistance((bool *)params, params->m_fDistance);
					m_sQueueSample.m_bVolume =
					    ComputeVolume(80, 40.f, m_sQueueSample.m_fDistance);
					if(m_sQueueSample.m_bVolume) {
						m_sQueueSample.m_counter = 4;
						m_sQueueSample.m_nSampleIndex =
						    CarSounds[params->m_nIndex].m_nHornSample;
						m_sQueueSample.m_bBankIndex = 0;
						m_sQueueSample.m_bIsDistant = 0;
						m_sQueueSample.field_16 = 2;
						m_sQueueSample.m_nFrequency =
						    CarSounds[params->m_nIndex].m_nHornFrequency;
						m_sQueueSample.m_nLoopCount = 0;
						m_sQueueSample.m_bEmittingVolume = 80;
						m_sQueueSample.m_nLoopStart =
						    SampleManager.GetSampleLoopStartOffset(
						        m_sQueueSample.m_nSampleIndex);
						m_sQueueSample.m_nLoopEnd =
						    SampleManager.GetSampleLoopEndOffset(
						        m_sQueueSample.m_nSampleIndex);
						m_sQueueSample.field_48 = 5.0;
						m_sQueueSample.m_fSoundIntensity = 40.0;
						m_sQueueSample.field_56 = 0;
						m_sQueueSample.field_76 = 3;
						m_sQueueSample.m_bReverbFlag = 1;
						m_sQueueSample.m_bRequireReflection = 0;
						AddSampleToRequestedQueue();
					}
				}
			}
		}
	}
}

WRAPPER
void cAudioManager::ProcessVehicleOneShots(void*)
{
	EAXJMP(0x56CD40);
}

bool
cAudioManager::ProcessVehicleReverseWarning(cVehicleParams *params)
{
	CVehicle *veh = params->m_pVehicle;

	if(params->m_fDistance >= 2500.f) return 0;

	if(veh->bEngineOn && veh->m_fGasPedal < 0.0f) {
		CalculateDistance((bool *)params, params->m_fDistance);
		m_sQueueSample.m_bVolume = ComputeVolume(60u, 50.f, m_sQueueSample.m_fDistance);
		if(m_sQueueSample.m_bVolume) {
			m_sQueueSample.m_counter = 12;
			m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_VEHICLE_REVERSE_WARNING;
			m_sQueueSample.m_bBankIndex = 0;
			m_sQueueSample.m_bIsDistant = 0;
			m_sQueueSample.field_16 = 2;
			m_sQueueSample.m_nFrequency =
			    (100 * m_sQueueSample.m_nEntityIndex & 0x3FF) +
			    SampleManager.GetSampleBaseFrequency(
			        AUDIO_SAMPLE_VEHICLE_REVERSE_WARNING);
			m_sQueueSample.m_nLoopCount = 0;
			m_sQueueSample.m_bEmittingVolume = 60;
			m_sQueueSample.m_nLoopStart =
			    SampleManager.GetSampleLoopStartOffset(m_sQueueSample.m_nSampleIndex);
			m_sQueueSample.m_nLoopEnd =
			    SampleManager.GetSampleLoopEndOffset(m_sQueueSample.m_nSampleIndex);
			m_sQueueSample.field_48 = 3.0f;
			m_sQueueSample.m_fSoundIntensity = 50.0f;
			m_sQueueSample.field_56 = 0;
			m_sQueueSample.field_76 = 3;
			m_sQueueSample.m_bReverbFlag = 1;
			m_sQueueSample.m_bRequireReflection = 0;
			AddSampleToRequestedQueue();
		}
	}
	return 1;
}

bool
cAudioManager::ProcessVehicleRoadNoise(cVehicleParams *params)
{
	int32 emittingVol;
	uint32 freq;
	float modificator;
	int sampleFreq;
	float velocity;

	if(params->m_fDistance >= 9025.f) return 0;
	if(params->m_pTransmission) {
		if(params->m_pVehicle->m_vecMoveSpeed.z) {
			velocity = Abs(params->m_fVelocityChange);
			if(velocity > 0.0f) {
				CalculateDistance((bool *)params, params->m_fDistance);
				emittingVol =
				    30.f *
				    min(1.f,
				        velocity / (0.5f * params->m_pTransmission->fMaxVelocity));
				m_sQueueSample.m_bVolume =
				    ComputeVolume(emittingVol, 95.f, m_sQueueSample.m_fDistance);
				if(m_sQueueSample.m_bVolume) {
					m_sQueueSample.m_counter = 0;
					m_sQueueSample.m_bBankIndex = 0;
					m_sQueueSample.m_bIsDistant = 0;
					m_sQueueSample.field_16 = 3;
					if(params->m_pVehicle->m_nSurfaceTouched ==
					   SURFACE_PUDDLE) {
						m_sQueueSample.m_nSampleIndex =
						    AUDIO_SAMPLE_LOOPING_WATER;
						freq = 6050 * emittingVol / 30 + 16000;
					} else {
						m_sQueueSample.m_nSampleIndex =
						    AUDIO_SAMPLE_VEHICLE_ROAD_NOISE;
						modificator =
						    m_sQueueSample.m_fDistance * 1.f / 95.f * 0.5f;
						sampleFreq = SampleManager.GetSampleBaseFrequency(
						    AUDIO_SAMPLE_VEHICLE_ROAD_NOISE);
						freq = (sampleFreq * modificator) +
						       ((3 * sampleFreq) >> 2);
					}
					m_sQueueSample.m_nFrequency = freq;
					m_sQueueSample.m_nLoopCount = 0;
					m_sQueueSample.m_bEmittingVolume = emittingVol;
					m_sQueueSample.m_nLoopStart =
					    SampleManager.GetSampleLoopStartOffset(
					        m_sQueueSample.m_nSampleIndex);
					m_sQueueSample.m_nLoopEnd =
					    SampleManager.GetSampleLoopEndOffset(
					        m_sQueueSample.m_nSampleIndex);
					m_sQueueSample.field_48 = 6.0f;
					m_sQueueSample.m_fSoundIntensity = 95.0f;
					m_sQueueSample.field_56 = 0;
					m_sQueueSample.field_76 = 4;
					m_sQueueSample.m_bReverbFlag = 1;
					m_sQueueSample.m_bRequireReflection = 0;
					AddSampleToRequestedQueue();
				}
			}
		}
	}
	return 1;
}

WRAPPER
void cAudioManager::ProcessVehicleSirenOrAlarm(void*)
{
	EAXJMP(0x56C420);
}

void
cAudioManager::ProcessVehicleSkidding(cVehicleParams *params)
{
	CAutomobile *automobile;
	cTransmission *transmission;
	int32 emittingVol;
	float newSkidVal = 0.f;
	float skidVal = 0.f;

	if(params->m_fDistance >= 1600.f) return;
	automobile = (CAutomobile *)params->m_pVehicle;
	if(!automobile->m_nWheelsOnGround) return;
	CalculateDistance((bool *)params, params->m_fDistance);
	for(int32 i = 0; i < 4; i++) {
		if(!automobile->m_aWheelState[i] || automobile->Damage.GetWheelStatus(i) == 2)
			continue;
		transmission = params->m_pTransmission;
		if(transmission->nDriveType == '4') {
			newSkidVal = GetVehicleDriveWheelSkidValue(i, automobile, transmission,
			                                           params->m_fVelocityChange);
			if(newSkidVal > skidVal) skidVal = newSkidVal;
			continue;
		}
		if(transmission->nDriveType != 'F') {
			if(transmission->nDriveType != 'R') {
				if(newSkidVal > skidVal) skidVal = newSkidVal;
				continue;
			}
			if(i != 1 && i != 3) {
				newSkidVal = GetVehicleNonDriveWheelSkidValue(
				    i, automobile, transmission, params->m_fVelocityChange);
				if(newSkidVal > skidVal) skidVal = newSkidVal;
				continue;
			}
			newSkidVal = GetVehicleDriveWheelSkidValue(i, automobile, transmission,
			                                           params->m_fVelocityChange);
			if(newSkidVal > skidVal) skidVal = newSkidVal;
			continue;
		}
		if(!i || i == 2) {
			newSkidVal = GetVehicleDriveWheelSkidValue(i, automobile, transmission,
			                                           params->m_fVelocityChange);
			if(newSkidVal > skidVal) skidVal = newSkidVal;
			continue;
		}
		newSkidVal = GetVehicleNonDriveWheelSkidValue(i, automobile, transmission,
		                                              params->m_fVelocityChange);
		if(newSkidVal > skidVal) skidVal = newSkidVal;
	}

	if(skidVal > 0.0f) {
		emittingVol = 50.f * skidVal;
		m_sQueueSample.m_bVolume =
		    ComputeVolume(emittingVol, 40.f, m_sQueueSample.m_fDistance);
		if(m_sQueueSample.m_bVolume) {
			m_sQueueSample.m_counter = 3;
			switch(params->m_pVehicle->m_nSurfaceTouched) {
			case SURFACE_GRASS:
			case SURFACE_HEDGE:
				m_sQueueSample.m_nSampleIndex =
				    AUDIO_SAMPLE_COLLISION_LOOPING_GRASS;
				emittingVol /= 4;
				m_sQueueSample.m_nFrequency =
				    (signed __int64)(13000.f * skidVal + 35000.f);
				m_sQueueSample.m_bVolume /= 4;
				break;
			case SURFACE_DIRT:
			case SURFACE_DIRTTRACK:
			case SURFACE_SAND:
			case SURFACE_PUDDLE:
				m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_COLLISION_LOOPING_SOFT;
				m_sQueueSample.m_nFrequency = 6000.f * skidVal + 10000.f;
				break;

			default:
				m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_VEHICLE_SKIDDING;
				m_sQueueSample.m_nFrequency = 5000.f * skidVal + 11000.f;
				break;
			}

			m_sQueueSample.m_bBankIndex = 0;
			m_sQueueSample.m_bIsDistant = 0;
			m_sQueueSample.field_16 = 8;
			m_sQueueSample.m_nLoopCount = 0;
			m_sQueueSample.m_bEmittingVolume = emittingVol;
			m_sQueueSample.m_nLoopStart =
			    SampleManager.GetSampleLoopStartOffset(m_sQueueSample.m_nSampleIndex);
			m_sQueueSample.m_nLoopEnd =
			    SampleManager.GetSampleLoopEndOffset(m_sQueueSample.m_nSampleIndex);
			m_sQueueSample.field_48 = 3.0;
			m_sQueueSample.m_fSoundIntensity = 40.0;
			m_sQueueSample.field_56 = 0;
			m_sQueueSample.field_76 = 3;
			m_sQueueSample.m_bReverbFlag = 1;
			m_sQueueSample.m_bRequireReflection = 0;
			AddSampleToRequestedQueue();
		}
	}
}

void cAudioManager::ProcessWaterCannon(int32)
{
	for(int32 i = 0; i < 3u; i++) {
		if(aCannons[i].m_nId) {
			m_sQueueSample.m_vecPos = aCannons[0].m_avecPos[aCannons[i].m_wIndex];
			float distSquared = GetDistanceSquared(&m_sQueueSample.m_vecPos);
			if(distSquared < 900.f) {
				m_sQueueSample.m_fDistance = Sqrt(distSquared);
				m_sQueueSample.m_bVolume =
				    ComputeVolume(50, m_sQueueSample.m_fSoundIntensity,
				                  m_sQueueSample.m_fDistance);
				if(m_sQueueSample.m_bVolume) {
					m_sQueueSample.m_fSoundIntensity = 900.0f;
					m_sQueueSample.m_nSampleIndex =
					    AUDIO_SAMPLE_JUMBO_TAXI_SOUND;
					m_sQueueSample.m_bBankIndex = 0;
					m_sQueueSample.m_nFrequency = 15591;
					m_sQueueSample.field_16 = 5;
					m_sQueueSample.m_counter = i;
					m_sQueueSample.field_48 = 2.0f;
					m_sQueueSample.field_76 = 8;
					m_sQueueSample.m_bIsDistant = 0;
					m_sQueueSample.m_nLoopCount = 0;
					m_sQueueSample.field_56 = 0;
					m_sQueueSample.m_bEmittingVolume = 50;
					m_sQueueSample.m_nLoopStart =
					    SampleManager.GetSampleLoopStartOffset(
					        m_sQueueSample.m_nSampleIndex);
					m_sQueueSample.m_nLoopEnd =
					    SampleManager.GetSampleLoopEndOffset(
					        m_sQueueSample.m_nSampleIndex);
					m_sQueueSample.m_bReverbFlag = 1;
					m_sQueueSample.m_bRequireReflection = 0;
					AddSampleToRequestedQueue();
				}
			}
		}
	}
}

void
cAudioManager::ProcessWeather(int32 id)
{
	uint8 vol;
	static uint8 counter = 0;

	if(m_asAudioEntities[id].m_Loops &&
	   m_asAudioEntities[id].m_awAudioEvent[0] == SOUND_LIGHTNING) {
		if(m_asAudioEntities[id].m_afVolume[0] >= 10.f) {
			m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_EXPLOSION_CAR;
			m_sQueueSample.m_bBankIndex = 0;
			m_sQueueSample.m_nFrequency = RandomDisplacement(500u) + 4000;
			vol = (m_asAudioEntities[id].m_afVolume[0] - 10.f) + 40;
		} else {
			m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_WEAPON_GRENADE_EXPLOSION;
			m_sQueueSample.m_bBankIndex = 0;
			m_sQueueSample.m_nFrequency = RandomDisplacement(500u) + 4000;
			vol = (m_asAudioEntities[id].m_afVolume[0]) + 35;
		}
		m_sQueueSample.m_bVolume = vol;
		if(TheCamera.SoundDistUp < 20.f) m_sQueueSample.m_bVolume >>= 1;
		if(counter == 4) counter = 0;
		m_sQueueSample.m_counter = counter++;
		m_sQueueSample.field_16 = 0;
		m_sQueueSample.m_bOffset = (m_anRandomTable[2] & 15) + 55;
		m_sQueueSample.m_bIsDistant = 1;
		m_sQueueSample.m_nLoopCount = 1;
		m_sQueueSample.field_56 = 1;
		m_sQueueSample.m_bEmittingVolume = m_sQueueSample.m_bVolume;
		m_sQueueSample.m_nLoopStart = 0;
		m_sQueueSample.m_nLoopEnd = -1;
		m_sQueueSample.m_bReverbFlag = 0;
		m_sQueueSample.m_bRequireReflection = 0;
		AddSampleToRequestedQueue();
	}
	if(CWeather::Rain > 0.0f && (!CCullZones::CamNoRain() || !CCullZones::PlayerNoRain())) {
		m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_COLLISION_LOOPING_GRASS;
		m_sQueueSample.m_nFrequency =
		    SampleManager.GetSampleBaseFrequency(AUDIO_SAMPLE_COLLISION_LOOPING_GRASS);
		m_sQueueSample.m_bVolume = (uint8)(25.f * CWeather::Rain);
		m_sQueueSample.m_counter = 4;
		m_sQueueSample.m_bBankIndex = 0;
		m_sQueueSample.field_16 = 0;
		m_sQueueSample.m_bOffset = 63;
		m_sQueueSample.m_bIsDistant = 1;
		m_sQueueSample.m_nLoopCount = 0;
		m_sQueueSample.field_56 = 0;
		m_sQueueSample.field_76 = 30;
		m_sQueueSample.m_bReverbFlag = 0;
		m_sQueueSample.m_bEmittingVolume = m_sQueueSample.m_bVolume;
		m_sQueueSample.m_nLoopStart =
		    SampleManager.GetSampleLoopStartOffset(m_sQueueSample.m_nSampleIndex);
		m_sQueueSample.m_nLoopEnd =
		    SampleManager.GetSampleLoopEndOffset(m_sQueueSample.m_nSampleIndex);
		m_sQueueSample.m_bRequireReflection = 0;
		AddSampleToRequestedQueue();
	}
}

bool
cAudioManager::ProcessWetRoadNoise(cVehicleParams *params)
{
	float relativeVelocity;
	int32 emittingVol;
	float modificator;
	int freq;
	float velChange;

	if(params->m_fDistance >= 900.f) return 0;
	if(params->m_pTransmission) {
		if(params->m_pVehicle->m_vecMoveSpeed.z) {
			velChange = Abs(params->m_fVelocityChange);
			if(velChange > 0.f) {
				CalculateDistance((bool *)params, params->m_fDistance);
				relativeVelocity =
				    min(1.0f,
				        velChange / (0.5f * params->m_pTransmission->fMaxVelocity));
				emittingVol = 23.0f * relativeVelocity * CWeather::WetRoads;
				m_sQueueSample.m_bVolume =
				    ComputeVolume(emittingVol, 30.f, m_sQueueSample.m_fDistance);
				if(m_sQueueSample.m_bVolume) {
					m_sQueueSample.m_counter = 1;
					m_sQueueSample.m_nSampleIndex =
					    AUDIO_SAMPLE_VEHICLE_ROAD_NOISE;
					m_sQueueSample.m_bBankIndex = 0;
					m_sQueueSample.m_bIsDistant = 0;
					m_sQueueSample.field_16 = 3;
					modificator = m_sQueueSample.m_fDistance * 1.f / 3.f * 0.5f;
					freq = SampleManager.GetSampleBaseFrequency(
					    AUDIO_SAMPLE_VEHICLE_ROAD_NOISE);
					m_sQueueSample.m_nFrequency = freq + freq * modificator;
					m_sQueueSample.m_nLoopCount = 0;
					m_sQueueSample.m_bEmittingVolume = emittingVol;
					m_sQueueSample.m_nLoopStart =
					    SampleManager.GetSampleLoopStartOffset(
					        m_sQueueSample.m_nSampleIndex);
					m_sQueueSample.m_nLoopEnd =
					    SampleManager.GetSampleLoopEndOffset(
					        m_sQueueSample.m_nSampleIndex);
					m_sQueueSample.field_48 = 6.0f;
					m_sQueueSample.m_fSoundIntensity = 30.0f;
					m_sQueueSample.field_56 = 0;
					m_sQueueSample.field_76 = 4;
					m_sQueueSample.m_bReverbFlag = 1;
					m_sQueueSample.m_bRequireReflection = 0;
					AddSampleToRequestedQueue();
				}
			}
		}
	}
	return 1;
}

void
cAudioManager::ProcessWorkShopScriptObject(uint8 sound)
{
	float distSquared;
	float maxDist;

	switch(sound) {
	case SCRIPT_SOUND_WORK_SHOP_LOOP_S:
	case SCRIPT_SOUND_WORK_SHOP_LOOP_L:
		maxDist = 400.f;
		m_sQueueSample.m_fSoundIntensity = 20.0f;
		break;
	default: return;
	}
	distSquared = GetDistanceSquared(&m_sQueueSample.m_vecPos);
	if(distSquared < maxDist) {
		m_sQueueSample.m_fDistance = sqrt(distSquared);
		m_sQueueSample.m_bVolume = ComputeVolume(30u, m_sQueueSample.m_fSoundIntensity,
		                                         m_sQueueSample.m_fDistance);
		if(m_sQueueSample.m_bVolume) {
			m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_WORK_SHOP;
			m_sQueueSample.m_bBankIndex = 0;
			m_sQueueSample.m_nFrequency =
			    SampleManager.GetSampleBaseFrequency(AUDIO_SAMPLE_WORK_SHOP);
			m_sQueueSample.m_counter = 0;
			m_sQueueSample.m_bIsDistant = 0;
			m_sQueueSample.m_nLoopCount = 0;
			m_sQueueSample.field_56 = 0;
			m_sQueueSample.field_16 = 5;
			m_sQueueSample.field_48 = 2.0f;
			m_sQueueSample.m_bEmittingVolume = 30;
			m_sQueueSample.m_nLoopStart =
			    SampleManager.GetSampleLoopStartOffset(m_sQueueSample.m_nSampleIndex);
			m_sQueueSample.m_nLoopEnd =
			    SampleManager.GetSampleLoopEndOffset(m_sQueueSample.m_nSampleIndex);
			m_sQueueSample.m_bReverbFlag = 1;
			m_sQueueSample.m_bRequireReflection = 0;
			AddSampleToRequestedQueue();
		}
	}
}

int32
cAudioManager::RandomDisplacement(uint32 seed) const
{
	int32 value;

	static bool bIsEven = true;
	static uint32 base = 0;

	if(!seed) return 0;

	value = m_anRandomTable[(base + seed) % 5] % seed;
	base += value;

	if(value % 2) { bIsEven = !bIsEven; }
	if(!bIsEven) value = -value;
	return value;
}

void
cAudioManager::ReacquireDigitalHandle() const
{
	if(m_bIsInitialised) { SampleManager.ReacquireDigitalHandle(); }
}

void
cAudioManager::ReleaseDigitalHandle() const
{
	if(m_bIsInitialised) { SampleManager.ReleaseDigitalHandle(); }
}

void
cAudioManager::ReportCollision(CEntity *entity1, CEntity *entity2, uint8 surface1, uint8 surface2,
                               float collisionPower, float velocity)
{
	float dist;
	CVector v1;
	CVector v2;

	if(m_bIsInitialised && m_nCollisionEntity >= 0 && !m_bUserPause &&
	   (velocity >= 0.0016f || collisionPower >= 0.01f)) {
		if((entity1->m_status & 7) == ENTITY_TYPE_BUILDING) {
			v1 = v2 = entity2->GetPosition();
		} else if((entity2->m_status & 7) == ENTITY_TYPE_BUILDING) {
			v1 = v2 = entity1->GetPosition();
		} else {
			v1 = entity1->GetPosition();
			v2 = entity2->GetPosition();
		}
		CVector pos = (v1 + v2) * 0.5f;
		dist = GetDistanceSquared(&pos);
		if(dist < 3600.f) {
			m_sCollisionManager.m_sQueue.m_pEntity1 = entity1;
			m_sCollisionManager.m_sQueue.m_pEntity2 = entity2;
			m_sCollisionManager.m_sQueue.m_bSurface1 = surface1;
			m_sCollisionManager.m_sQueue.m_bSurface2 = surface2;
			m_sCollisionManager.m_sQueue.m_fIntensity1 = collisionPower;
			m_sCollisionManager.m_sQueue.m_fIntensity2 = velocity;
			m_sCollisionManager.m_sQueue.m_vecPosition = pos;
			m_sCollisionManager.m_sQueue.m_fDistance = dist;
			m_sCollisionManager.AddCollisionToRequestedQueue();
		}
	}
}

void
cAudioManager::ReportCrime(int32 type, const CVector *pos)
{
	int32 lastCrime = 10;
	if(m_bIsInitialised && MusicManager.m_nMusicMode != 2 &&
	   FindPlayerPed()->m_pWanted->m_nWantedLevel > 0 && (type > 0 || type < 17) &&
	   m_nTimeOfRecentCrime >= gMinTimeToNextReport[type]) {
		for(int32 i = 0; i < 10; i++) {
			if(crimes[i].type) {
				if(crimes[i].type == type) {
					crimes[i].position = *pos;
					crimes[i].timer = 0;
					return;
				}
			} else {
				lastCrime = i;
			}
		}

		if(lastCrime < 10) {
			crimes[lastCrime].type = type;
			crimes[lastCrime].position = *pos;
			crimes[lastCrime].timer = 0;
			gMinTimeToNextReport[type] = m_nTimeOfRecentCrime + 500;
		}
	}
}

void
cAudioManager::ResetAudioLogicTimers(int32 timer)
{
	audioLogicTimers[0] = timer;
	audioLogicTimers[8] = timer;
	audioLogicTimers[1] = timer;
	audioLogicTimers[7] = timer;
	audioLogicTimers[2] = timer;
	audioLogicTimers[6] = timer;
	audioLogicTimers[3] = timer;
	audioLogicTimers[5] = timer;
	audioLogicTimers[4] = timer;
	for(int32 i = 0; i < m_nAudioEntitiesTotal; i++) {
		if(m_asAudioEntities[m_anAudioEntityIndices[i]].m_nType == AUDIOTYPE_PHYSICAL) {
			CPed *ped = (CPed *)m_asAudioEntities[m_anAudioEntityIndices[i]].m_pEntity;
			if(ped->IsPed()) {
				ped->m_lastSoundStart = timer;
				ped->m_soundStart = timer + m_anRandomTable[0] % 3000u;
			}
		}
	}
	ClearMissionAudio();
	SampleManager.StopChannel(policeChannel);
}

void
cAudioManager::ResetPoliceRadio()
{
	if(m_bIsInitialised) {
		if(SampleManager.GetChannelUsedFlag(policeChannel))
			SampleManager.StopChannel(policeChannel);
		InitialisePoliceRadio();
	}
}

void
cAudioManager::ResetTimers(uint32 time)
{
	if(m_bIsInitialised) {
		m_bTimerJustReset = 1;
		m_nTimer = time;
		ClearRequestedQueue();
		if(m_bActiveSampleQueue) {
			m_bActiveSampleQueue = 0;
			ClearRequestedQueue();
			m_bActiveSampleQueue = 1;
		} else {
			m_bActiveSampleQueue = 1;
			ClearRequestedQueue();
			m_bActiveSampleQueue = 0;
		}
		ClearActiveSamples();
		ClearMissionAudio();
		SampleManager.StopChannel(policeChannel);
		SampleManager.SetEffectsFadeVolume(0);
		SampleManager.SetMusicFadeVolume(0);
		MusicManager.ResetMusicAfterReload();
	}
}

void
cAudioManager::Service()
{
	GenerateIntegerRandomNumberTable();
	if(m_bTimerJustReset) {
		ResetAudioLogicTimers(m_nTimer);
		MusicManager.ResetTimers(m_nTimer);
		m_bTimerJustReset = 0;
	}
	if(m_bIsInitialised) {
		m_bPreviousUserPause = m_bUserPause;
		m_bUserPause = CTimer::GetIsUserPaused();
		UpdateReflections();
		ServiceSoundEffects();
		MusicManager.Service();
	}
}

WRAPPER
void
cAudioManager::ServiceCollisions()
{
	EAXJMP(0x5686D0);
}

void
cAudioManager::ServicePoliceRadio()
{
	int32 wantedLevel = 0; // bug?;
	static uint32 nLastSeen = 300;

	if(m_bIsInitialised) {
		if(!m_bUserPause) {
			bool crimeReport = SetupCrimeReport();
			wantedLevel = FindPlayerPed()->m_pWanted->m_nWantedLevel;
			if(!crimeReport) {
				if(wantedLevel) {
					if(nLastSeen) {
						--nLastSeen;
					} else {
						nLastSeen = m_anRandomTable[1] % 1000u + 2000;
						SetupSuspectLastSeenReport();
					}
				}
			}
		}
		ServicePoliceRadioChannel(wantedLevel);
	}
}

void
cAudioManager::ServicePoliceRadioChannel(int32 wantedLevel)
{
	bool processed = false;
	uint32 sample;
	int32 freq;

	static int cWait = 0;
	static bool bChannelOpen = 0;
	static uint8 bMissionAudioPhysicalPlayingStatus = 0;
	static int32 PoliceChannelFreq = 5500;

	if(m_bIsInitialised) {
		if(m_bUserPause) {
			if(SampleManager.GetChannelUsedFlag(policeChannel))
				SampleManager.StopChannel(policeChannel);
			if(g_nMissionAudioSfx != TOTAL_AUDIO_SAMPLES &&
			   bMissionAudioPhysicalPlayingStatus == 1 &&
			   SampleManager.IsStreamPlaying(1u)) {
				SampleManager.PauseStream(1, 1u);
			}
		} else {
			if(m_bPreviousUserPause && g_nMissionAudioSfx != TOTAL_AUDIO_SAMPLES &&
			   bMissionAudioPhysicalPlayingStatus == 1) {
				SampleManager.PauseStream(0, 1u);
			}
			if(!policeChannelTimer) bChannelOpen = 0;
			if(cWait) {
				--cWait;
				return;
			}
			if(g_nMissionAudioSfx != TOTAL_AUDIO_SAMPLES && !bChannelOpen) {
				if(g_nMissionAudioPlayingStatus) {
					if(g_nMissionAudioPlayingStatus == 1 &&
					   !bMissionAudioPhysicalPlayingStatus &&
					   SampleManager.IsStreamPlaying(1u)) {
						bMissionAudioPhysicalPlayingStatus = 1;
					}
					if(bMissionAudioPhysicalPlayingStatus == 1) {
						if(SampleManager.IsStreamPlaying(1u)) {
							DoPoliceRadioCrackle();
						} else {
							bMissionAudioPhysicalPlayingStatus = 2;
							g_nMissionAudioPlayingStatus = 2;
							g_nMissionAudioSfx = TOTAL_AUDIO_SAMPLES;
							cWait = 30;
						}
						return;
					}
				} else if(!SampleManager.GetChannelUsedFlag(policeChannel)) {
					SampleManager.PreloadStreamedFile(g_nMissionAudioSfx, 1u);
					SampleManager.SetStreamedVolumeAndPan(maxVolume, 63u, 1,
					                                      1u);
					SampleManager.StartPreloadedStreamedFile(1u);
					g_nMissionAudioPlayingStatus = 1;
					bMissionAudioPhysicalPlayingStatus = 0;
					return;
				}
			}
			if(bChannelOpen) DoPoliceRadioCrackle();
			if((g_nMissionAudioSfx == TOTAL_AUDIO_SAMPLES ||
			    g_nMissionAudioPlayingStatus != 1) &&
			   !SampleManager.GetChannelUsedFlag(policeChannel) && policeChannelTimer) {
				if(policeChannelTimer) {
					sample = crimesSamples[policeChannelCounterSeconds];
					--policeChannelTimer;
					policeChannelCounterSeconds =
					    (policeChannelCounterSeconds + 1) % 60;
				} else {
					sample = TOTAL_AUDIO_SAMPLES;
				}
				if(!wantedLevel) {
					if(gSpecialSuspectLastSeenReport) {
						gSpecialSuspectLastSeenReport = 0;
					} else if((unsigned int)(sample - 212) <= 2 ||
					          sample == TOTAL_AUDIO_SAMPLES) {
						bChannelOpen = 0;
						processed = 1;
					}
				}
				if(sample == TOTAL_AUDIO_SAMPLES) {
					if(!processed) cWait = 30;
				} else {
					SampleManager.InitialiseChannel(policeChannel, sample, 0);
					switch(sample) {
					case AUDIO_SAMPLE_POLICE_SCANNER_SMALL_CRACKLE_1:
					case AUDIO_SAMPLE_POLICE_SCANNER_SMALL_CRACKLE_2:
					case AUDIO_SAMPLE_POLICE_SCANNER_SMALL_CRACKLE_3:
						freq = m_anRandomTable[4] % 2000u + 10025;
						bChannelOpen = bChannelOpen == 0;
						break;
					default:
						freq = SampleManager.GetSampleBaseFrequency(sample);
						break;
					}
					PoliceChannelFreq = freq;
					SampleManager.SetChannelFrequency(policeChannel, freq);
					SampleManager.SetChannelVolume(policeChannel, 100u);
					SampleManager.SetChannelPan(policeChannel, 63);
					SampleManager.SetChannelLoopCount(policeChannel, 1);
					SampleManager.SetChannelLoopPoints(policeChannel, 0, -1);
					SampleManager.StartChannel(policeChannel);
				}
				if(processed) ResetPoliceRadio();
			}
		}
	}
}

void
cAudioManager::ServiceSoundEffects()
{
	uint32 timeOfRecentCrime;
	cAudioScriptObject *object;

	timeOfRecentCrime = m_nTimeOfRecentCrime;
	++m_nTimeOfRecentCrime;
	if(timeOfRecentCrime % 5)
		field_2 = 0;
	else
		field_2 = 1;
	if(m_bUserPause && !m_bPreviousUserPause) {
		for(int32 i = 0; i < 29; i++) SampleManager.StopChannel(i);

		ClearRequestedQueue();
		if(m_bActiveSampleQueue) {
			m_bActiveSampleQueue = 0;
			ClearRequestedQueue();
			m_bActiveSampleQueue = 1;
		} else {
			m_bActiveSampleQueue = 1;
			ClearRequestedQueue();
			m_bActiveSampleQueue = 0;
		}
		ClearActiveSamples();
	}
	m_bActiveSampleQueue = m_bActiveSampleQueue != 1;
	ProcessReverb();
	ProcessSpecial();
	ClearRequestedQueue();
	InterrogateAudioEntities();
	m_sPedComments.Process();
	ServicePoliceRadio();
	ServiceCollisions();
	AddReleasingSounds();
	ProcessMissionAudio();
	sub_57C2B0();
	ProcessActiveQueues();
	for(int32 i = 0; i < m_nScriptObjectEntityTotal; ++i) {
		object = (cAudioScriptObject *)m_asAudioEntities[m_anScriptObjectEntityIndices[i]]
		             .m_pEntity;
		if(object) { delete object; }
		DestroyEntity(m_anScriptObjectEntityIndices[i]);
	}
	m_nScriptObjectEntityTotal = 0;
}

int8
cAudioManager::SetCurrent3DProvider(uint8 which)
{
	if(!m_bIsInitialised) return -1;
	for(uint8 i = 0; i < m_bActiveSamples + 1; ++i) SampleManager.StopChannel(i);
	ClearRequestedQueue();
	if(m_bActiveSampleQueue)
		m_bActiveSampleQueue = 0;
	else
		m_bActiveSampleQueue = 1;
	ClearRequestedQueue();
	ClearActiveSamples();
	int8 current = SampleManager.SetCurrent3DProvider(which);
	if(current > 0) {
		m_bActiveSamples = SampleManager.GetMaximumSupportedChannels();
		if(m_bActiveSamples > 1u) --m_bActiveSamples;
	}
	return current;
}

void
cAudioManager::SetDynamicAcousticModelingStatus(bool status)
{
	m_bDynamicAcousticModelingStatus = status;
}

void
cAudioManager::SetEffectsFadeVolume(uint8 volume) const
{
	SampleManager.SetEffectsFadeVolume(volume);
}

void
cAudioManager::SetEffectsMasterVolume(uint8 volume) const
{
	SampleManager.SetEffectsMasterVolume(volume);
}

void
cAudioManager::SetEntityStatus(int32 id, bool status)
{
	if(m_bIsInitialised && id >= 0 && id < totalAudioEntitiesSlots &&
	   m_asAudioEntities[id].m_bIsUsed) {
		m_asAudioEntities[id].m_bStatus = status;
	}
}

uint32
cAudioManager::SetLoopingCollisionRequestedSfxFreqAndGetVol(cAudioCollision *audioCollision)
{
	uint8 surface1 = audioCollision->m_bSurface1;
	uint8 surface2 = audioCollision->m_bSurface2;
	int32 vol;
	float ratio;

	if(surface1 != SURFACE_GRASS && surface2 != SURFACE_GRASS && surface1 != SURFACE_HEDGE &&
	   surface2 != SURFACE_HEDGE) {
		if(surface1 != SURFACE_PUDDLE && surface2 != SURFACE_PUDDLE) {
			if(surface1 != SURFACE_DIRT && surface2 != SURFACE_DIRT &&
			   surface1 != SURFACE_DIRTTRACK && surface2 != SURFACE_DIRTTRACK &&
			   surface1 != SURFACE_SAND && surface2 != SURFACE_SAND) {
				if(surface1 == SURFACE_FLESH || surface2 == SURFACE_FLESH) {
					return 0;
				}
				ratio = GetCollisionRatio(audioCollision->m_fIntensity2, 0.0001f,
				                          0.09f, 0.0899f);
				m_sQueueSample.m_nSampleIndex =
				    AUDIO_SAMPLE_COLLISION_LOOPING_GENERIC;
				m_sQueueSample.m_nFrequency = 10000.f * ratio + 10000;
				vol = 40.f * ratio;
			} else {
				ratio = GetCollisionRatio(audioCollision->m_fIntensity2, 0.0001f,
				                          0.09f, 0.0899f);
				m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_COLLISION_LOOPING_SOFT;
				m_sQueueSample.m_nFrequency = 6000.f * ratio + 10000;
				vol = 50.f * ratio;
			}
		} else {
			ratio = GetCollisionRatio(audioCollision->m_fIntensity2, 0.0001f, 0.09f,
			                          0.0899f);
			m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_LOOPING_WATER;
			m_sQueueSample.m_nFrequency = 6050.f * ratio + 16000;
			vol = 30.f * ratio;
		}
	} else {
		ratio = GetCollisionRatio(audioCollision->m_fIntensity2, 0.0001f, 0.09f, 0.0899f);
		m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_COLLISION_LOOPING_GRASS;
		m_sQueueSample.m_nFrequency = 13000.f * ratio + 35000;
		vol = 50.f * ratio;
	}
	if(audioCollision->m_nBaseVolume < 2) vol = audioCollision->m_nBaseVolume * vol / 2;
	return vol;
}

void
cAudioManager::SetMissionAudioLocation(float x, float y, float z)
{
	if(m_bIsInitialised) {
		m_sMissionAudio.field_12 = 0;
		m_sMissionAudio.m_vecPos.x = x;
		m_sMissionAudio.m_vecPos.y = y;
		m_sMissionAudio.m_vecPos.z = z;
	}
}

void
cAudioManager::SetMissionScriptPoliceAudio(int32 sfx) const
{
	if(m_bIsInitialised) {
		if(g_nMissionAudioPlayingStatus != 1) {
			g_nMissionAudioPlayingStatus = 0;
			g_nMissionAudioSfx = sfx;
		}
	}
}

void
cAudioManager::SetMusicFadeVolume(uint8 volume) const
{
	SampleManager.SetMusicFadeVolume(volume);
}

void
cAudioManager::SetMusicMasterVolume(uint8 volume) const
{
	SampleManager.SetMusicMasterVolume(volume);
}

void
cAudioManager::SetSpeakerConfig(int32 conf) const
{
	SampleManager.SetSpeakerConfig(conf);
}

void
cAudioManager::SetUpLoopingCollisionSound(cAudioCollision *col, uint8 counter)
{
	if(col->m_fIntensity2 > 0.0016f) {
		uint8 emittingVol = SetLoopingCollisionRequestedSfxFreqAndGetVol(col);
		if(emittingVol) {
			m_sQueueSample.m_fDistance = Sqrt(col->m_fDistance);
			m_sQueueSample.m_bVolume =
			    ComputeVolume(emittingVol, 60.f, m_sQueueSample.m_fDistance);
			if(m_sQueueSample.m_bVolume) {
				m_sQueueSample.m_counter = counter;
				m_sQueueSample.m_vecPos = col->m_vecPosition;
				m_sQueueSample.m_bBankIndex = 0;
				m_sQueueSample.m_bIsDistant = 0;
				m_sQueueSample.field_16 = 7;
				m_sQueueSample.m_nLoopCount = 0;
				m_sQueueSample.m_bEmittingVolume = emittingVol;
				m_sQueueSample.m_nLoopStart =
				    SampleManager.GetSampleLoopStartOffset(
				        m_sQueueSample.m_nSampleIndex);
				m_sQueueSample.m_nLoopEnd = SampleManager.GetSampleLoopEndOffset(
				    m_sQueueSample.m_nSampleIndex);
				m_sQueueSample.field_48 = 4.0f;
				m_sQueueSample.m_fSoundIntensity = 60.0f;
				m_sQueueSample.field_56 = 0;
				m_sQueueSample.field_76 = 5;
				m_sQueueSample.m_bReverbFlag = 1;
				m_sQueueSample.m_bRequireReflection = 0;
				AddSampleToRequestedQueue();
			}
		}
	}
}

void
cAudioManager::SetUpOneShotCollisionSound(cAudioCollision *col)
{
	static constexpr int32 gOneShotCol[] = {
	    AUDIO_SAMPLE_COLLISION_PAVEMENT,   AUDIO_SAMPLE_COLLISION_PAVEMENT,
	    AUDIO_SAMPLE_COLLISION_MUFFLED,    AUDIO_SAMPLE_COLLISION_DIRT,
	    AUDIO_SAMPLE_COLLISION_MUD,        AUDIO_SAMPLE_COLLISION_PAVEMENT,
	    AUDIO_SAMPLE_COLLISION_METAL,      AUDIO_SAMPLE_COLLISION_MUFFLED,
	    AUDIO_SAMPLE_COLLISION_HARD,       AUDIO_SAMPLE_COLLISION_METAL_DOOR,
	    AUDIO_SAMPLE_CAR_DOOR_MOVEMENT_1,  AUDIO_SAMPLE_COLLISION_METAL_11,
	    AUDIO_SAMPLE_COLLISION_HARD,       AUDIO_SAMPLE_COLLISION_METAL_13,
	    AUDIO_SAMPLE_COLLISION_METAL_14,   AUDIO_SAMPLE_COLLISION_METAL_14,
	    AUDIO_SAMPLE_COLLISION_FENCE,      AUDIO_SAMPLE_COLLISION_FLESH,
	    AUDIO_SAMPLE_COLLISION_SAND,       AUDIO_SAMPLE_COLLISION_WATER,
	    AUDIO_SAMPLE_COLLISION_WOOD,       AUDIO_SAMPLE_COLLISION_WOOD_BOX,
	    AUDIO_SAMPLE_COLLISION_WOOD_PLANK, AUDIO_SAMPLE_COLLISION_MUFFLED,
	    AUDIO_SAMPLE_COLLISION_MUFFLED,    AUDIO_SAMPLE_COLLISION_HEDGE,
	    AUDIO_SAMPLE_COLLISION_PAVEMENT,   AUDIO_SAMPLE_COLLISION_METAL_27,
	    AUDIO_SAMPLE_COLLISION_METAL_28,   AUDIO_SAMPLE_COLLISION_RUBBER,
	    AUDIO_SAMPLE_COLLISION_LOOSE,      AUDIO_SAMPLE_COLLISION_PAVEMENT,
	    AUDIO_SAMPLE_COLLISION_GATE};

	int16 s1;
	int16 s2;

	int32 emittingVol;
	float ratio;

	static uint16 counter = 28;

	for(int32 i = 0; i < 2; i++) {
		if(i) {
			s1 = col->m_bSurface2;
			s2 = col->m_bSurface1;
		} else {
			s1 = col->m_bSurface1;
			s2 = col->m_bSurface2;
		}
		ratio = GetCollisionOneShotRatio(s1, col->m_fIntensity1);
		if(s1 == SURFACE_METAL6 && s2 == SURFACE_FLESH) ratio = 0.25f * ratio;
		if(s1 == SURFACE_METAL6 && ratio < 0.6f) {
			s1 = SURFACE_BILLBOARD;
			ratio = min(1.f, 2.f * ratio);
		}
		emittingVol = 40.f * ratio;
		if(emittingVol) {
			m_sQueueSample.m_fDistance = Sqrt(col->m_fDistance);
			m_sQueueSample.m_bVolume =
			    ComputeVolume(emittingVol, 60.f, m_sQueueSample.m_fDistance);
			if(m_sQueueSample.m_bVolume) {
				m_sQueueSample.m_nSampleIndex = gOneShotCol[s1];
				switch(m_sQueueSample.m_nSampleIndex) {
				case AUDIO_SAMPLE_COLLISION_PAVEMENT:
					m_sQueueSample.m_nSampleIndex += m_anRandomTable[3] % 5u;
					break;
				case AUDIO_SAMPLE_CAR_DOOR_MOVEMENT_1:
					m_sQueueSample.m_nSampleIndex += m_anRandomTable[0] % 6u;
					break;
				case AUDIO_SAMPLE_COLLISION_METAL_13:
					m_sQueueSample.m_nSampleIndex += m_anRandomTable[1] & 1;
					break;
				case AUDIO_SAMPLE_COLLISION_FENCE:
					m_sQueueSample.m_nSampleIndex += m_anRandomTable[3] & 3;
					break;
				case AUDIO_SAMPLE_COLLISION_FLESH:
					m_sQueueSample.m_nSampleIndex += m_anRandomTable[4] % 5u;
					break;
				case AUDIO_SAMPLE_COLLISION_WOOD:
					m_sQueueSample.m_nSampleIndex += m_anRandomTable[4] & 3;
					break;
				case AUDIO_SAMPLE_COLLISION_WOOD_BOX:
					m_sQueueSample.m_nSampleIndex += m_anRandomTable[1] & 3;
					break;
				case AUDIO_SAMPLE_COLLISION_HEDGE:
					m_sQueueSample.m_nSampleIndex += m_anRandomTable[2] % 5u;
					break;
				case AUDIO_SAMPLE_COLLISION_METAL_28:
					m_sQueueSample.m_nSampleIndex += m_anRandomTable[2] % 3u;
					break;
				case AUDIO_SAMPLE_COLLISION_METAL:
					m_sQueueSample.m_nSampleIndex += m_anRandomTable[1] % 5u;
					break;
				case AUDIO_SAMPLE_COLLISION_LOOSE:
					m_sQueueSample.m_nSampleIndex += m_anRandomTable[3] & 1;
					break;
				default: break;
				}
				switch(s1) {
				case SURFACE_GLASS: m_sQueueSample.m_nFrequency = 13500; break;
				case SURFACE_METAL15: m_sQueueSample.m_nFrequency = 8819; break;
				case SURFACE_PUDDLE:
					m_sQueueSample.m_nFrequency =
					    2 * SampleManager.GetSampleBaseFrequency(
					            m_sQueueSample.m_nSampleIndex);
					break;
				case SURFACE_TIRE: m_sQueueSample.m_nFrequency = 6000; break;
				case SURFACE_HARD24: m_sQueueSample.m_nFrequency = 8000; break;
				default:
					m_sQueueSample.m_nFrequency =
					    SampleManager.GetSampleBaseFrequency(
					        m_sQueueSample.m_nSampleIndex);
					break;
				}
				m_sQueueSample.m_nFrequency +=
				    RandomDisplacement(m_sQueueSample.m_nFrequency / 16);
				m_sQueueSample.m_counter = counter++;
				if(counter >= 255u) counter = 28;
				m_sQueueSample.m_vecPos = col->m_vecPosition;
				m_sQueueSample.m_bBankIndex = 0;
				m_sQueueSample.m_bIsDistant = 0;
				m_sQueueSample.field_16 = 11;
				m_sQueueSample.m_nLoopCount = 1;
				m_sQueueSample.m_bEmittingVolume = emittingVol;
				m_sQueueSample.m_nLoopStart = 0;
				m_sQueueSample.m_nLoopEnd = -1;
				m_sQueueSample.field_48 = 4.0;
				m_sQueueSample.m_fSoundIntensity = 60.0;
				m_sQueueSample.field_56 = 1;
				m_sQueueSample.m_bReverbFlag = 1;
				m_sQueueSample.m_bRequireReflection = 0;
				AddSampleToRequestedQueue();
			}
		}
	}
}

bool
cAudioManager::SetupCrimeReport()
{

		cAudioManager* v1; // ebx
		unsigned __int8 v3; // dl
		int v4; // eax
		__int16 v5; // ax
		unsigned __int8 v6; // dl
		CZone* v7; // edi
		int v8; // ecx
		int v9; // eax
		int v10; // eax
		int32 v11; // edx
		double v12; // st5
		double v13; // st7
		double v14; // st4
		double v15; // st6
		double v16; // st5
		double v17; // st4
		double v18; // st7
		double v19; // st6
		double v20; // st3
		double v21; // st6
		int v22; // eax
		int v23; // [esp+4h] [ebp-24h]
		int32 v24; // [esp+8h] [ebp-20h]
		char v25; // [esp+Ch] [ebp-1Ch]

		v1 = this;
		v25 = 0;
		if (MusicManager.m_nMusicMode == 2)
			return 0;
		if ((unsigned __int8)(60 - this->policeChannelTimer) <= 9u)
			goto LABEL_65;
		v3 = 0;
		v4 = 0;
		do
		{
			if (this->crimes[v4].type)
				break;
			++v3;
			++v4;
		} while (v3 < 10u);
		if (v3 == 10)
			return 0;
		v23 = v3;
		v5 = CTheZones::FindAudioZone(&this->crimes[v23].position);
		if (v5 >= 0 && v5 < 36)
		{
			v6 = 0;
			v7 = &CTheZones::ZoneArray[CTheZones::AudioZoneArray[v5]];
			v8 = 0;
			while (strcmp(v7->name, ZoneSfx[v8].m_aName) != 0)
			{
				++v6;
				++v8;
				if (v6 >= 36u)
					goto LABEL_16;
			}
			v24 = ZoneSfx[v6].m_nSampleIndex;
		LABEL_16:
			if (v6 < 36u)
			{
				v9 = -1431655765 * v1->m_anRandomTable[4];
				if (v1->policeChannelTimer != 60)
				{
					v1->crimesSamples[v1->policeChannelTimerSeconds] = v1->m_anRandomTable[4] % 3u
						+ AUDIO_SAMPLE_POLICE_SCANNER_SMALL_CRACKLE_1;
					++v1->policeChannelTimer;
					v1->policeChannelTimerSeconds = (v1->policeChannelTimerSeconds + 1) % 60;
				}
				v10 = -1431655765 * v1->m_anRandomTable[0];
				if (v1->policeChannelTimer != 60)
				{
					v1->crimesSamples[v1->policeChannelTimerSeconds] = v1->m_anRandomTable[0] % 3u
						+ AUDIO_SAMPLE_POLICE_SCANNER_WE_GOT_1;
					++v1->policeChannelTimer;
					v1->policeChannelTimerSeconds = (v1->policeChannelTimerSeconds + 1) % 60;
				}
				if (v1->policeChannelTimer != 60)
				{
					v1->crimesSamples[v1->policeChannelTimerSeconds] = (v1->m_anRandomTable[1] & 1)
						+ AUDIO_SAMPLE_POLICE_SCANNER_TEN_1;
					++v1->policeChannelTimer;
					v1->policeChannelTimerSeconds = (v1->policeChannelTimerSeconds + 1) % 60;
				}
				v11 = v1->crimes[v23].type;
				switch (v1->crimes[v23].type)
				{
				case CRIME_PED_BURNED:
					v1->crimes[v23].type = 2;
					break;
				case CRIME_COP_BURNED:
					v1->crimes[v23].type = 3;
					break;
				case CRIME_VEHICLE_BURNED:
					v1->crimes[v23].type = 6;
					break;
				case CRIME_DESTROYED_CESSNA:
					v1->crimes[v23].type = 12;
					break;
				default:
					break;
				}
				if (v1->policeChannelTimer != 60)
				{
					v1->crimesSamples[v1->policeChannelTimerSeconds] = v1->crimes[v23].type + AUDIO_SAMPLE_POLICE_SCANNER_TEN_2;
					++v1->policeChannelTimer;
					v1->policeChannelTimerSeconds = (v1->policeChannelTimerSeconds + 1) % 60;
				}
				if (v1->policeChannelTimer != 60)
				{
					v1->crimesSamples[v1->policeChannelTimerSeconds] = AUDIO_SAMPLE_POLICE_SCANNER_IN;
					++v1->policeChannelTimer;
					v1->policeChannelTimerSeconds = (v1->policeChannelTimerSeconds + 1) % 60;
				}
				if (v24 == AUDIO_SAMPLE_POLICE_SCANNER_ZONE_SHORESIDE
					&& (strcmp(v7->name, SubZo2Label) == 0
						|| strcmp(v7->name, SubZo3Label) ==0))
				{
					if (v1->policeChannelTimer != 60)
					{
						v1->crimesSamples[v1->policeChannelTimerSeconds] = AUDIO_SAMPLE_POLICE_SCANNER_NORTH;
						++v1->policeChannelTimer;
						v1->policeChannelTimerSeconds = (v1->policeChannelTimerSeconds + 1) % 60;
					}
					if (v1->policeChannelTimer == 60)
						goto LABEL_58;
					v1->crimesSamples[v1->policeChannelTimerSeconds] = AUDIO_SAMPLE_POLICE_SCANNER_EAST;
					goto LABEL_57;
				}
				v12 = v7->minx;
				v13 = v7->maxx - v12;
				v14 = v7->miny;
				v15 = v7->maxy - v14;
				v16 = 0.5f * v13 + v12;
				v17 = 0.5f * v15 + v14;
				v18 = 0.25f * v13;
				v19 = 0.25f * v15;
				v20 = v1->crimes[v23].position.y;
				if (v17 + v19 < v20)
				{
					if (v17 - v19 <= v20)
						goto LABEL_48;
					if (v1->policeChannelTimer != 60)
					{
						v1->crimesSamples[v1->policeChannelTimerSeconds] = AUDIO_SAMPLE_POLICE_SCANNER_SOUTH;
						++v1->policeChannelTimer;
						v1->policeChannelTimerSeconds = (v1->policeChannelTimerSeconds + 1) % 60;
					}
				}
				else if (v1->policeChannelTimer != 60)
				{
					v1->crimesSamples[v1->policeChannelTimerSeconds] = AUDIO_SAMPLE_POLICE_SCANNER_NORTH;
					++v1->policeChannelTimer;
					v1->policeChannelTimerSeconds = (v1->policeChannelTimerSeconds + 1) % 60;
				}
				v25 = 1;
			LABEL_48:
				v21 = v1->crimes[v23].position.x;
				if (v16 + v18 < v21)
				{
					if (v16 - v18 <= v21)
					{
						if (v25 || v1->policeChannelTimer == 60)
							goto LABEL_58;
						v1->crimesSamples[v1->policeChannelTimerSeconds] = AUDIO_SAMPLE_POLICE_SCANNER_CENTRAL;
						goto LABEL_57;
					}
					if (v1->policeChannelTimer != 60)
					{
						v1->crimesSamples[v1->policeChannelTimerSeconds] = AUDIO_SAMPLE_POLICE_SCANNER_WEST;
						goto LABEL_57;
					}
				}
				else if (v1->policeChannelTimer != 60)
				{
					v1->crimesSamples[v1->policeChannelTimerSeconds] = AUDIO_SAMPLE_POLICE_SCANNER_EAST;
				LABEL_57:
					++v1->policeChannelTimer;
					v1->policeChannelTimerSeconds = (v1->policeChannelTimerSeconds + 1) % 60;
					goto LABEL_58;
				}
			LABEL_58:
				if (v1->policeChannelTimer != 60)
				{
					v1->crimesSamples[v1->policeChannelTimerSeconds] = v24;
					++v1->policeChannelTimer;
					v1->policeChannelTimerSeconds = (v1->policeChannelTimerSeconds + 1) % 60;
				}
				v22 = -1431655765 * v1->m_anRandomTable[2];
				if (v1->policeChannelTimer != 60)
				{
					v1->crimesSamples[v1->policeChannelTimerSeconds] = v1->m_anRandomTable[2] % 3u
						+ AUDIO_SAMPLE_POLICE_SCANNER_SMALL_CRACKLE_1;
					++v1->policeChannelTimer;
					v1->policeChannelTimerSeconds = (v1->policeChannelTimerSeconds + 1) % 60;
				}
				if (v1->policeChannelTimer != 60)
				{
					v1->crimesSamples[v1->policeChannelTimerSeconds] = TOTAL_AUDIO_SAMPLES;
					++v1->policeChannelTimer;
					v1->policeChannelTimerSeconds = (v1->policeChannelTimerSeconds + 1) % 60;
				}
				goto LABEL_64;
			}
		}
	LABEL_64:
		v1->crimes[v23].type = 0;
	LABEL_65:
		AgeCrimes();
		return 1;
	
}

WRAPPER
bool cAudioManager::SetupJumboEngineSound(uint8, int32) { EAXJMP(0x56F140) }

bool
cAudioManager::SetupJumboFlySound(uint8 emittingVol)
{
	if(m_sQueueSample.m_fDistance >= 440.0f) return 0;

	int32 vol = ComputeVolume(emittingVol, 440.0f, m_sQueueSample.m_fDistance);
	m_sQueueSample.m_bVolume = vol;
	if(m_sQueueSample.m_bVolume) {
		m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_JUMBO_FLY_SOUND;
		m_sQueueSample.m_counter = 0;
		m_sQueueSample.m_bBankIndex = 0;
		m_sQueueSample.m_bIsDistant = 0;
		m_sQueueSample.field_16 = 1;
		m_sQueueSample.m_bEmittingVolume = emittingVol;
		m_sQueueSample.m_nLoopCount = 0;
		m_sQueueSample.m_nFrequency =
		    SampleManager.GetSampleBaseFrequency(AUDIO_SAMPLE_JUMBO_FLY_SOUND);
		m_sQueueSample.m_nLoopStart =
		    SampleManager.GetSampleLoopStartOffset(m_sQueueSample.m_nSampleIndex);
		m_sQueueSample.m_fSoundIntensity = 440.0f;
		m_sQueueSample.field_56 = 0;
		m_sQueueSample.field_48 = 4.0f;
		m_sQueueSample.m_bReverbFlag = 1;
		m_sQueueSample.field_76 = 5;
		m_sQueueSample.m_nLoopEnd =
		    SampleManager.GetSampleLoopEndOffset(m_sQueueSample.m_nSampleIndex);
		AddSampleToRequestedQueue();
	}
	return 1;
}

bool
cAudioManager::SetupJumboRumbleSound(uint8 emittingVol)
{
	if(m_sQueueSample.m_fDistance >= 240.f) return 0;

	m_sQueueSample.m_bVolume = ComputeVolume(emittingVol, 240.f, m_sQueueSample.m_fDistance);

	if(m_sQueueSample.m_bVolume) {
		m_sQueueSample.m_counter = 5;
		m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_JUMBO_RUMBLE_SOUND;
		m_sQueueSample.m_bBankIndex = 0;
		m_sQueueSample.m_bIsDistant = 1;
		m_sQueueSample.field_16 = 1;
		m_sQueueSample.m_nFrequency =
		    SampleManager.GetSampleBaseFrequency(AUDIO_SAMPLE_JUMBO_RUMBLE_SOUND);
		m_sQueueSample.m_nLoopCount = 0;
		m_sQueueSample.m_bEmittingVolume = emittingVol;
		m_sQueueSample.m_nLoopStart =
		    SampleManager.GetSampleLoopStartOffset(m_sQueueSample.m_nSampleIndex);
		m_sQueueSample.m_nLoopEnd =
		    SampleManager.GetSampleLoopEndOffset(m_sQueueSample.m_nSampleIndex);
		m_sQueueSample.field_48 = 4.0f;
		m_sQueueSample.m_fSoundIntensity = 240.0f;
		m_sQueueSample.field_56 = 0;
		m_sQueueSample.field_76 = 12;
		m_sQueueSample.m_bOffset = 0;
		m_sQueueSample.m_bReverbFlag = 1;
		m_sQueueSample.m_bRequireReflection = 0;
		AddSampleToRequestedQueue();
		m_sQueueSample.m_counter = 6;
		m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_JUMBO_RUMBLE_SOUND;
		m_sQueueSample.m_nFrequency += 200;
		m_sQueueSample.m_bOffset = maxVolume;
		AddSampleToRequestedQueue();
	}
	return 1;
}

bool
cAudioManager::SetupJumboTaxiSound(uint8 vol)
{
	if(m_sQueueSample.m_fDistance >= 180.f) return 0;

	uint8 emittingVol = (vol + (vol * m_sQueueSample.m_fDistance / 180)) / 2;

	if(m_sQueueSample.m_fDistance / 180 < 0.7f)
		emittingVol -= emittingVol * gJumboVolOffsetPercentage / 100;
	m_sQueueSample.m_bVolume = ComputeVolume(emittingVol, 180.f, m_sQueueSample.m_fDistance);

	if(m_sQueueSample.m_bVolume) {
		m_sQueueSample.m_counter = 1;
		m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_JUMBO_TAXI_SOUND;
		m_sQueueSample.m_bBankIndex = 0;
		m_sQueueSample.m_bIsDistant = 0;
		m_sQueueSample.field_16 = 1;
		m_sQueueSample.m_nFrequency = GetJumboTaxiFreq();
		m_sQueueSample.m_nLoopCount = 0;
		m_sQueueSample.m_bEmittingVolume = emittingVol;
		m_sQueueSample.m_nLoopStart =
		    SampleManager.GetSampleLoopStartOffset(m_sQueueSample.m_nSampleIndex);
		m_sQueueSample.m_nLoopEnd =
		    SampleManager.GetSampleLoopEndOffset(m_sQueueSample.m_nSampleIndex);
		m_sQueueSample.field_48 = 4.0f;
		m_sQueueSample.m_fSoundIntensity = 180.0f;
		m_sQueueSample.field_56 = 0;
		m_sQueueSample.field_76 = 4;
		m_sQueueSample.m_bReverbFlag = 1;
		m_sQueueSample.m_bRequireReflection = 0;
		AddSampleToRequestedQueue();
	}
	return 1;
}

bool
cAudioManager::SetupJumboWhineSound(uint8 emittingVol, int32 freq)
{
	if(m_sQueueSample.m_fDistance >= 170.f) return 0;

	m_sQueueSample.m_bVolume = ComputeVolume(emittingVol, 170.f, m_sQueueSample.m_fDistance);

	if(m_sQueueSample.m_bVolume) {
		m_sQueueSample.m_counter = 2;
		m_sQueueSample.m_nSampleIndex = AUDIO_SAMPLE_JUMBO_WHINE_SOUND;
		m_sQueueSample.m_bBankIndex = 0;
		m_sQueueSample.m_bIsDistant = 0;
		m_sQueueSample.field_16 = 1;
		m_sQueueSample.m_nFrequency = freq;
		m_sQueueSample.m_nLoopCount = 0;
		m_sQueueSample.m_bEmittingVolume = emittingVol;
		m_sQueueSample.m_nLoopStart =
		    SampleManager.GetSampleLoopStartOffset(m_sQueueSample.m_nSampleIndex);
		m_sQueueSample.m_nLoopEnd =
		    SampleManager.GetSampleLoopEndOffset(m_sQueueSample.m_nSampleIndex);
		m_sQueueSample.field_48 = 4.0f;
		m_sQueueSample.m_fSoundIntensity = 170.0f;
		m_sQueueSample.field_56 = 0;
		m_sQueueSample.field_76 = 4;
		m_sQueueSample.m_bReverbFlag = 1;
		m_sQueueSample.m_bRequireReflection = 0;
		AddSampleToRequestedQueue();
	}
	return 1;
}

void
cAudioManager::SetupPedComments(cPedParams *params, uint32 sound)
{
	CPed *ped = params->m_pPed;
	uint8 emittingVol;
	float maxDist;
	float soundIntensity;
	tPedComment pedComment;

	if(ped) {
		switch(sound) {
		case SOUND_AMMUNATION_WELCOME_1:
			pedComment.m_nSampleIndex = AUDIO_SAMPLE_AMMUNATION_WELCOME_1;
			break;
		case SOUND_AMMUNATION_WELCOME_2:
			pedComment.m_nSampleIndex = AUDIO_SAMPLE_AMMUNATION_WELCOME_2;
			break;
		case SOUND_AMMUNATION_WELCOME_3:
			pedComment.m_nSampleIndex = AUDIO_SAMPLE_AMMUNATION_WELCOME_3;
			break;
		default:
			pedComment.m_nSampleIndex = GetPedCommentSfx(ped, sound);
			if(pedComment.m_nSampleIndex == NO_SAMPLE) return;
			break;
		}

		maxDist = 2500.f;
		soundIntensity = 50.f;

		if(params->m_fDistance < maxDist) {
			CalculateDistance((bool *)params, params->m_fDistance);
			if(sound != SOUND_PAGER) {
				switch(sound) {
				case SOUND_AMMUNATION_WELCOME_1:
				case SOUND_AMMUNATION_WELCOME_2:
				case SOUND_AMMUNATION_WELCOME_3: emittingVol = maxVolume; break;
				default:
					if(CWorld::GetIsLineOfSightClear(
					       TheCamera.GetGameCamPosition(),
					       m_sQueueSample.m_vecPos, 1, 0, 0, 0, 0, 0, 0)) {
						emittingVol = maxVolume;
					} else {
						emittingVol = 31;
					}
					break;
				}
				m_sQueueSample.m_bVolume = ComputeVolume(
				    emittingVol, soundIntensity, m_sQueueSample.m_fDistance);
				pedComment.field_25 = 10;
				if(m_sQueueSample.m_bVolume) {
					pedComment.m_entityIndex = m_sQueueSample.m_nEntityIndex;
					pedComment.m_vecPos = m_sQueueSample.m_vecPos;
					pedComment.m_fDistance = m_sQueueSample.m_fDistance;
					pedComment.m_bVolume = m_sQueueSample.m_bVolume;
					m_sPedComments.Add(&pedComment);
				}
			}
		}
	} else {
		switch(sound) {
		case SOUND_PED_HELI_PLAYER_FOUND:
			maxDist = 160000.f;
			soundIntensity = 400.f;
			pedComment.m_nSampleIndex =
			    m_anRandomTable[m_sQueueSample.m_nEntityIndex & 3] % 29u +
			    AUDIO_SAMPLE_POLICE_HELI_FOUND_PLAYER_1;
			break;
		case SOUND_PED_BODYCAST_HIT:
			if(CTimer::GetTimeInMilliseconds() <= audioLogicTimers[8]) return;
			maxDist = 2500.f;
			soundIntensity = 50.f;
			audioLogicTimers[8] = CTimer::GetTimeInMilliseconds() + 500;
			pedComment.m_nSampleIndex =
			    (m_anRandomTable[m_sQueueSample.m_nEntityIndex & 3] & 3) +
			    AUDIO_SAMPLE_PED_BODYCAST_HIT_1;
			break;
		case SOUND_INJURED_PED_MALE_OUCH:
		case SOUND_8A:
			maxDist = 2500.f;
			soundIntensity = 50.f;
			pedComment.m_nSampleIndex =
			    m_anRandomTable[m_sQueueSample.m_nEntityIndex & 3] % 15u +
			    AUDIO_SAMPLE_INJURED_PED_MALE_OUCH_1;
			break;
		case SOUND_INJURED_PED_FEMALE:
			maxDist = 2500.f;
			soundIntensity = 50.f;
			pedComment.m_nSampleIndex =
			    m_anRandomTable[m_sQueueSample.m_nEntityIndex & 3] % 11u +
			    AUDIO_SAMPLE_INJURED_PED_FEMALE_OUCH_1;
			break;
		default: return;
		}

		if(params->m_fDistance < maxDist) {
			CalculateDistance((bool *)params, params->m_fDistance);
			if(sound != SOUND_PAGER) {
				switch(sound) {
				case SOUND_AMMUNATION_WELCOME_1:
				case SOUND_AMMUNATION_WELCOME_2:
				case SOUND_AMMUNATION_WELCOME_3: emittingVol = maxVolume; break;
				default:
					if(CWorld::GetIsLineOfSightClear(
					       TheCamera.GetGameCamPosition(),
					       m_sQueueSample.m_vecPos, 1, 0, 0, 0, 0, 0, 0)) {
						emittingVol = maxVolume;
					} else {
						emittingVol = 31;
					}
					break;
				}
				m_sQueueSample.m_bVolume = ComputeVolume(
				    emittingVol, soundIntensity, m_sQueueSample.m_fDistance);
				pedComment.field_25 = 10;
				if(m_sQueueSample.m_bVolume) {
					pedComment.m_entityIndex = m_sQueueSample.m_nEntityIndex;
					pedComment.m_vecPos = m_sQueueSample.m_vecPos;
					pedComment.m_fDistance = m_sQueueSample.m_fDistance;
					pedComment.m_bVolume = m_sQueueSample.m_bVolume;
					m_sPedComments.Add(&pedComment);
				}
			}
		}
	}
}

struct ColorSoundForScanner
{
	eAudioSamples m_eColor1;
	eAudioSamples m_eColor2;
	eAudioSamples m_eColor3;
};

void
cAudioManager::SetupSuspectLastSeenReport()
{
		cAudioManager* v1; // ebp
		CAutomobile* v2; // eax
		uint8 v3; // bl
		unsigned __int8 v4; // dl
		int v5; // edx
		int main_color; // edi
		int v7; // edx
		eAudioSamples v8; // esi
		int v9; // eax
		int v10; // eax
		uint8 v11; // cl
		int v12; // eax
		int v13; // eax
		int color_pre_modifier; // [esp+8h] [ebp-18h]
		int color_post_modifier; // [esp+Ch] [ebp-14h]

		constexpr int32 colors[] =
{
  3032,
  248,
  3032,
  3032,
  249,
  3032,
  3032,
  250,
  3032,
  3032,
  251,
  3032,
  258,
  250,
  3032,
  3032,
  252,
  3032,
  3032,
  253,
  3032,
  260,
  250,
  3032,
  259,
  250,
  254,
  259,
  3032,
  3032,
  258,
  3032,
  3032,
  258,
  3032,
  3032,
  258,
  251,
  3032,
  3032,
  251,
  3032,
  3032,
  251,
  3032,
  3032,
  251,
  3032,
  3032,
  251,
  3032,
  3032,
  251,
  3032,
  3032,
  251,
  3032,
  259,
  3032,
  3032,
  258,
  3032,
  3032,
  258,
  3032,
  3032,
  258,
  3032,
  3032,
  3032,
  255,
  3032,
  3032,
  255,
  3032,
  3032,
  255,
  3032,
  3032,
  255,
  3032,
  3032,
  255,
  3032,
  3032,
  255,
  3032,
  259,
  3032,
  3032,
  258,
  3032,
  3032,
  258,
  3032,
  3032,
  258,
  3032,
  3032,
  3032,
  253,
  3032,
  3032,
  253,
  3032,
  3032,
  253,
  3032,
  3032,
  253,
  3032,
  3032,
  253,
  3032,
  3032,
  253,
  3032,
  259,
  3032,
  3032,
  258,
  3032,
  3032,
  258,
  3032,
  3032,
  258,
  3032,
  3032,
  3032,
  256,
  3032,
  3032,
  256,
  3032,
  3032,
  256,
  3032,
  3032,
  256,
  3032,
  3032,
  256,
  3032,
  3032,
  256,
  3032,
  259,
  3032,
  3032,
  258,
  3032,
  3032,
  258,
  3032,
  3032,
  258,
  3032,
  3032,
  3032,
  250,
  3032,
  3032,
  250,
  3032,
  3032,
  250,
  3032,
  3032,
  250,
  3032,
  3032,
  250,
  3032,
  3032,
  250,
  3032,
  259,
  3032,
  3032,
  258,
  3032,
  3032,
  258,
  3032,
  3032,
  258,
  3032,
  3032,
  3032,
  252,
  3032,
  3032,
  252,
  3032,
  3032,
  252,
  3032,
  3032,
  252,
  3032,
  3032,
  252,
  3032,
  3032,
  252,
  3032,
  259,
  3032,
  3032,
  258,
  3032,
  3032,
  258,
  3032,
  3032,
  258,
  3032,
  3032,
  3032,
  257,
  3032,
  3032,
  257,
  3032,
  3032,
  257,
  3032,
  3032,
  257,
  3032,
  3032,
  257,
  3032,
  3032,
  257,
  3032,
  259,
  3032,
  3032,
  259,
  3032,
  3032,
  259,
  3032,
  3032,
  259,
  3032,
  3032,
  259,
  3032,
  3032,
  259,
  3032,
  3032,
  259,
  3032,
  3032,
  259,
  3032,
  3032,
  259,
  3032,
  3032,
  259,
  3032,
  3032,
  259,
  3032,
  3032,
  258,
  3032,
  3032,
  258,
  3032,
  3032,
  258,
  3032,
  3032,
  258,
  3032,
  3032,
  258,
  3032,
  3032
};

		v1 = this;
		if (MusicManager.m_nMusicMode != 2)
		{
			v2 = (CAutomobile*)FindPlayerVehicle();
			if (v2)
			{
				v3 = v1->policeChannelTimer;
				if (60 - v3 > 9u)
				{
					v4 = v2->m_currentColour1;
					if (v4 >= 95u)
					{
						debug("\n *** UNKNOWN CAR COLOUR %d *** ", v4);
					}
					else
					{
						v5 = 3 * v4;
						main_color = colors[v5 + 1]; //todo refactor struct
						color_pre_modifier = colors[v5];
						color_post_modifier = colors[v5 + 2];
						v7 = v2->m_modelIndex;
						switch (v7)
						{
						case MI_LANDSTAL:
						case MI_BLISTA:
							v8 = AUDIO_SAMPLE_POLICE_SCANNER_CAR_CRUISER;
							goto LABEL_28;
						case MI_IDAHO:
						case MI_STALLION:
							v8 = AUDIO_SAMPLE_POLICE_SCANNER_CAR_CONVERTIBLE;
							goto LABEL_28;
						case MI_STINGER:
						case MI_INFERNUS:
						case MI_CHEETAH:
						case MI_BANSHEE:
							v8 = AUDIO_SAMPLE_POLICE_SCANNER_CAR_SPORTS_CAR;
							goto LABEL_28;
						case MI_PEREN:
						case MI_SENTINEL:
						case MI_FBICAR:
							v8 = AUDIO_SAMPLE_POLICE_SCANNER_CAR_ECONOMY;
							goto LABEL_28;
						case MI_PATRIOT:
						case MI_BOBCAT:
							v8 = AUDIO_SAMPLE_POLICE_SCANNER_CAR_PICKUP;
							goto LABEL_28;
						case MI_FIRETRUCK:
							v8 = AUDIO_SAMPLE_POLICE_SCANNER_CAR_FIRETRUCK;
							goto LABEL_28;
						case MI_TRASH:
						case MI_BARRACKS:
							v8 = AUDIO_SAMPLE_POLICE_SCANNER_CAR_TRUCK;
							goto LABEL_28;
						case MI_STRETCH:
							v8 = AUDIO_SAMPLE_POLICE_SCANNER_CAR_LIMO;
							goto LABEL_28;
						case MI_MANANA:
						case MI_ESPERANT:
							v8 = AUDIO_SAMPLE_POLICE_SCANNER_CAR_COUPE;
							goto LABEL_28;
						case MI_PONY:
						case MI_MULE:
						case MI_MOONBEAM:
						case MI_ENFORCER:
						case MI_SECURICA:
						case MI_RUMPO:
							v8 = AUDIO_SAMPLE_POLICE_SCANNER_CAR_VAN;
							goto LABEL_28;
						case MI_AMBULAN:
							v8 = AUDIO_SAMPLE_POLICE_SCANNER_CAR_AMBULANCE;
							goto LABEL_28;
						case MI_TAXI:
						case MI_CABBIE:
						case MI_BORGNINE:
							v8 = AUDIO_SAMPLE_POLICE_SCANNER_CAR_TAXI;
							goto LABEL_28;
						case MI_MRWHOOP:
							v8 = AUDIO_SAMPLE_POLICE_SCANNER_CAR_ICE_CREAM_TRUCK;
							goto LABEL_28;
						case MI_BFINJECT:
							v8 = AUDIO_SAMPLE_POLICE_SCANNER_CAR_BUGGY;
							goto LABEL_28;
						case MI_POLICE:
							v8 = AUDIO_SAMPLE_POLICE_SCANNER_CAR_POLICE_CAR;
							goto LABEL_28;
						case MI_PREDATOR:
							v8 = AUDIO_SAMPLE_POLICE_SCANNER_CAR_BOAT;
							goto LABEL_28;
						case MI_BUS:
						case MI_COACH:
							v8 = AUDIO_SAMPLE_POLICE_SCANNER_CAR_BUS;
							goto LABEL_28;
						case MI_RHINO:
							v8 = AUDIO_SAMPLE_POLICE_SCANNER_CAR_TANK;
							main_color = TOTAL_AUDIO_SAMPLES;
							color_post_modifier = TOTAL_AUDIO_SAMPLES;
							goto LABEL_28;
						case MI_TRAIN:
							v8 = AUDIO_SAMPLE_POLICE_SCANNER_CAR_SUBWAY_CAR;
							main_color = TOTAL_AUDIO_SAMPLES;
							color_post_modifier = TOTAL_AUDIO_SAMPLES;
						LABEL_28:
							v9 = -1431655765 * v1->m_anRandomTable[4];
							if (v3 != 60)
							{
								v1->crimesSamples[v1->policeChannelTimerSeconds] = v1->m_anRandomTable[4] % 3u
									+ AUDIO_SAMPLE_POLICE_SCANNER_SMALL_CRACKLE_1;
								++v1->policeChannelTimer;
								v1->policeChannelTimerSeconds = (v1->policeChannelTimerSeconds + 1) % 60;
							}
							if (v1->policeChannelTimer != 60)
							{
								v1->crimesSamples[v1->policeChannelTimerSeconds] = AUDIO_SAMPLE_POLICE_SCANNER_SUSPECT;
								++v1->policeChannelTimer;
								v1->policeChannelTimerSeconds = (v1->policeChannelTimerSeconds + 1) % 60;
							}
							if (v1->m_anRandomTable[3] & 1 && v1->policeChannelTimer != 60)
							{
								v1->crimesSamples[v1->policeChannelTimerSeconds] = AUDIO_SAMPLE_POLICE_SCANNER_LAST_SEEN;
								++v1->policeChannelTimer;
								v1->policeChannelTimerSeconds = (v1->policeChannelTimerSeconds + 1) % 60;
							}
							if (main_color == AUDIO_SAMPLE_POLICE_SCANNER_COLOR_ORANGE)
							{
								if (v1->policeChannelTimer != 60)
								{
									v1->crimesSamples[v1->policeChannelTimerSeconds] = AUDIO_SAMPLE_POLICE_SCANNER_IN_AN;
									++v1->policeChannelTimer;
									v1->policeChannelTimerSeconds = (v1->policeChannelTimerSeconds + 1) % 60;
								}
							}
							else if (v1->policeChannelTimer != 60)
							{
								v1->crimesSamples[v1->policeChannelTimerSeconds] = AUDIO_SAMPLE_POLICE_SCANNER_IN_A;
								++v1->policeChannelTimer;
								v1->policeChannelTimerSeconds = (v1->policeChannelTimerSeconds + 1) % 60;
							}
							if (color_pre_modifier != 3032 && v1->policeChannelTimer != 60)
							{
								v1->crimesSamples[v1->policeChannelTimerSeconds] = color_pre_modifier;
								++v1->policeChannelTimer;
								v1->policeChannelTimerSeconds = (v1->policeChannelTimerSeconds + 1) % 60;
							}
							if (main_color != 3032 && v1->policeChannelTimer != 60)
							{
								v1->crimesSamples[v1->policeChannelTimerSeconds] = main_color;
								++v1->policeChannelTimer;
								v1->policeChannelTimerSeconds = (v1->policeChannelTimerSeconds + 1) % 60;
							}
							if (color_post_modifier != 3032 && v1->policeChannelTimer != 60)
							{
								v1->crimesSamples[v1->policeChannelTimerSeconds] = color_post_modifier;
								++v1->policeChannelTimer;
								v1->policeChannelTimerSeconds = (v1->policeChannelTimerSeconds + 1) % 60;
							}
							if (v1->policeChannelTimer != 60)
							{
								v1->crimesSamples[v1->policeChannelTimerSeconds] = v8;
								++v1->policeChannelTimer;
								v1->policeChannelTimerSeconds = (v1->policeChannelTimerSeconds + 1) % 60;
							}
							v10 = -1431655765 * v1->m_anRandomTable[0];
							if (v1->policeChannelTimer != 60)
							{
								v1->crimesSamples[v1->policeChannelTimerSeconds] = v1->m_anRandomTable[0] % 3u
									+ AUDIO_SAMPLE_POLICE_SCANNER_SMALL_CRACKLE_1;
								++v1->policeChannelTimer;
								v1->policeChannelTimerSeconds = (v1->policeChannelTimerSeconds + 1) % 60;
							}
							if (v1->policeChannelTimer != 60)
							{
								v1->crimesSamples[v1->policeChannelTimerSeconds] = TOTAL_AUDIO_SAMPLES;
								++v1->policeChannelTimer;
								v1->policeChannelTimerSeconds = (v1->policeChannelTimerSeconds + 1) % 60;
							}
							break;
						default:
							debug("\n *** UNKNOWN CAR MODEL INDEX %d *** ", v7);
							break;
						}
					}
				}
			}
			else
			{
				v11 = v1->policeChannelTimer;
				if (60 - v11 > 4u)
				{
					v12 = -1431655765 * v1->m_anRandomTable[4];
					if (v11 != 60)
					{
						v1->crimesSamples[v1->policeChannelTimerSeconds] = v1->m_anRandomTable[4] % 3u
							+ AUDIO_SAMPLE_POLICE_SCANNER_SMALL_CRACKLE_1;
						++v1->policeChannelTimer;
						v1->policeChannelTimerSeconds = (v1->policeChannelTimerSeconds + 1) % 60;
					}
					if (v1->policeChannelTimer != 60)
					{
						v1->crimesSamples[v1->policeChannelTimerSeconds] = AUDIO_SAMPLE_POLICE_SCANNER_SUSPECT;
						++v1->policeChannelTimer;
						v1->policeChannelTimerSeconds = (v1->policeChannelTimerSeconds + 1) % 60;
					}
					if (v1->policeChannelTimer != 60)
					{
						v1->crimesSamples[v1->policeChannelTimerSeconds] = AUDIO_SAMPLE_POLICE_SCANNER_ON_FOOT;
						++v1->policeChannelTimer;
						v1->policeChannelTimerSeconds = (v1->policeChannelTimerSeconds + 1) % 60;
					}
					v13 = -1431655765 * v1->m_anRandomTable[0];
					if (v1->policeChannelTimer != 60)
					{
						v1->crimesSamples[v1->policeChannelTimerSeconds] = v1->m_anRandomTable[0] % 3u
							+ AUDIO_SAMPLE_POLICE_SCANNER_SMALL_CRACKLE_1;
						++v1->policeChannelTimer;
						v1->policeChannelTimerSeconds = (v1->policeChannelTimerSeconds + 1) % 60;
					}
					if (v1->policeChannelTimer != 60)
					{
						v1->crimesSamples[v1->policeChannelTimerSeconds] = TOTAL_AUDIO_SAMPLES;
						++v1->policeChannelTimer;
						v1->policeChannelTimerSeconds = (v1->policeChannelTimerSeconds + 1) % 60;
					}
				}
			}
		}
	
}

void
cAudioManager::Terminate()
{
	if(m_bIsInitialised) {
		MusicManager.Terminate();

		for(uint32 i = 0; i < totalAudioEntitiesSlots; i++) {
			m_asAudioEntities[i].m_bIsUsed = 0;
			m_anAudioEntityIndices[i] = 200;
		}

		m_nAudioEntitiesTotal = 0;
		m_nScriptObjectEntityTotal = 0;
		PreTerminateGameSpecificShutdown();

		for(uint32 i = 0; i < 2; i++) {
			if(SampleManager.IsSampleBankLoaded(i)) SampleManager.UnloadSampleBank(i);
		}

		SampleManager.Terminate();

		m_bIsInitialised = 0;
		PostTerminateGameSpecificShutdown();
	}
}

void
cAudioManager::TranslateEntity(CVector *v1, CVector *v2) const
{
	const RwMatrix &cM = TheCamera.GetMatrix().m_matrix;
	const CVector &cV = TheCamera.GetPosition();

	float a = v1->z - cV.z;
	float b = v1->y - cV.y;
	float c = v1->x - cV.x;

	v2->x = cM.right.y * b + cM.right.x * c + cM.right.z * a;
	v2->y = cM.up.y * b + cM.up.x * c + cM.up.z * a;
	v2->z = cM.at.y * b + cM.at.x * c + cM.at.z * a;
}

void
cAudioManager::UpdateGasPedalAudio(CAutomobile *automobile)
{
	float newGasPedalAudio;

	float gasPedal = Abs(automobile->m_fGasPedal);
	float gasPedalAudio = automobile->m_fGasPedalAudio;

	if(gasPedalAudio < gasPedal) {
		newGasPedalAudio = gasPedalAudio + 0.09f;
		if(gasPedal <= newGasPedalAudio) newGasPedalAudio = gasPedal;
	} else {
		newGasPedalAudio = gasPedalAudio - 0.07f;
		if(gasPedal >= newGasPedalAudio) newGasPedalAudio = gasPedal;
	}
	automobile->m_fGasPedalAudio = newGasPedalAudio;
}

WRAPPER
void
cAudioManager::UpdateReflections()
{
	EAXJMP(0x57B470)
}

bool
cAudioManager::UsesReverseWarning(int32 model) const
{
	return model == LINERUN || model == FIRETRUK || model == TRASH || model == BUS ||
	       model == COACH;
}

bool
cAudioManager::UsesSiren(int32 model) const
{
	switch(model) {
	case FIRETRUK:
	case AMBULAN:
	case FBICAR:
	case POLICE:
	case ENFORCER:
	case PREDATOR: return true;
	default: return false;
	}
}

bool
cAudioManager::UsesSirenSwitching(int32 model) const
{
	switch(model) {
	case AMBULAN:
	case POLICE:
	case ENFORCER:
	case PREDATOR: return true;
	default: return false;
	}
}

WRAPPER
void
cAudioManager::sub_57C2B0()
{
	EAXJMP(0x57C2B0);
}

int32
cAudioManager::sub_57C320(uint8 a1, float a2, float a3)
{
	float v4;
	float v5;
	int32 v6;

	v4 = 0.25f * a2;
	v5 = a2 - v4;
	if(a3 <= v5)
		v6 = a1;
	else
		v6 = ((v4 - (a3 - v5)) * (float)a1 / v4);
	return v6;
}

STARTPATCHES
InjectHook(0x57B210, &cAudioManager::AddDetailsToRequestedOrderList, PATCH_JUMP);
InjectHook(0x56AD30, &cAudioManager::AddPlayerCarSample, PATCH_JUMP);
InjectHook(0x57B300, &cAudioManager::AddReflectionsToRequestedQueue, PATCH_JUMP);
InjectHook(0x57B8D0, &cAudioManager::AddReleasingSounds, PATCH_JUMP);
InjectHook(0x57B070, &cAudioManager::AddSampleToRequestedQueue, PATCH_JUMP);
InjectHook(0x580AF0, &cAudioManager::AgeCrimes, PATCH_JUMP);
InjectHook(0x5697A0, &cAudioManager::CalculateDistance, PATCH_JUMP);
InjectHook(0x57AA10, &cAudioManager::CheckForAnAudioFileOnCD, PATCH_JUMP);
InjectHook(0x57C160, &cAudioManager::ClearActiveSamples, PATCH_JUMP);
InjectHook(0x5796A0, &cAudioManager::ClearMissionAudio, PATCH_JUMP);
InjectHook(0x57C120, &cAudioManager::ClearRequestedQueue, PATCH_JUMP);
InjectHook(0x57AE00, &cAudioManager::ComputeDopplerEffectedFrequency, PATCH_JUMP);
InjectHook(0x57AD20, &cAudioManager::ComputePan, PATCH_JUMP);
InjectHook(0x57ABB0, &cAudioManager::ComputeVolume, PATCH_JUMP);
InjectHook(0x57A310, &cAudioManager::CreateEntity, PATCH_JUMP);
InjectHook(0x57A830, &cAudioManager::DestroyAllGameCreatedEntities, PATCH_JUMP);
InjectHook(0x57A400, &cAudioManager::DestroyEntity, PATCH_JUMP);
InjectHook(0x57F060, &cAudioManager::DoPoliceRadioCrackle, PATCH_JUMP);
InjectHook(0x57C290, &cAudioManager::GenerateIntegerRandomNumberTable, PATCH_JUMP);
InjectHook(0x57A8C0, &cAudioManager::Get3DProviderName, PATCH_JUMP);
InjectHook(0x571110, &cAudioManager::GetArmyTalkSfx, PATCH_JUMP);
InjectHook(0x573AB0, &cAudioManager::GetBlackBusinessFemaleTalkSfx, PATCH_JUMP);
InjectHook(0x572050, &cAudioManager::GetBlackCasualFemaleTalkSfx, PATCH_JUMP);
InjectHook(0x574380, &cAudioManager::GetBlackConstructionWorkerTalkSfx, PATCH_JUMP);
InjectHook(0x571D80, &cAudioManager::GetBlackCriminalTalkSfx, PATCH_JUMP);
InjectHook(0x5735E0, &cAudioManager::GetBlackDockerMaleTalkSfx, PATCH_JUMP);
InjectHook(0x5724D0, &cAudioManager::GetBlackFatFemaleTalkSfx, PATCH_JUMP);
InjectHook(0x5726C0, &cAudioManager::GetBlackFatMaleTalkSfx, PATCH_JUMP);
InjectHook(0x5728B0, &cAudioManager::GetBlackFemaleProstituteTalkSfx, PATCH_JUMP);
InjectHook(0x572C20, &cAudioManager::GetBlackProjectFemaleOldTalkSfx, PATCH_JUMP);
InjectHook(0x572D20, &cAudioManager::GetBlackProjectFemaleYoungTalkSfx, PATCH_JUMP);
InjectHook(0x572AF0, &cAudioManager::GetBlackProjectMaleTalkSfx, PATCH_JUMP);
InjectHook(0x5739C0, &cAudioManager::GetBlackWorkerMaleTalkSfx, PATCH_JUMP);
InjectHook(0x574FF0, &cAudioManager::GetBomberTalkSfx, PATCH_JUMP);
InjectHook(0x5712C0, &cAudioManager::GetBusinessMaleOldTalkSfx, PATCH_JUMP);
InjectHook(0x5713E0, &cAudioManager::GetBusinessMaleYoungTalkSfx, PATCH_JUMP);
InjectHook(0x572040, &cAudioManager::GetCasualMaleOldTalkSfx, PATCH_JUMP);
InjectHook(0x574FE0, &cAudioManager::GetCatatalinaTalkSfx, PATCH_JUMP);
InjectHook(0x57AA30, &cAudioManager::GetCDAudioDriveLetter, PATCH_JUMP);
InjectHook(0x573010, &cAudioManager::GetChinatownFemaleOldTalkSfx, PATCH_JUMP);
InjectHook(0x5730F0, &cAudioManager::GetChinatownFemaleYoungTalkSfx, PATCH_JUMP);
InjectHook(0x572E10, &cAudioManager::GetChinatownMaleOldTalkSfx, PATCH_JUMP);
InjectHook(0x572F10, &cAudioManager::GetChinatownMaleYoungTalkSfx, PATCH_JUMP);
InjectHook(0x575120, &cAudioManager::GetChunkyTalkSfx, PATCH_JUMP);
InjectHook(0x571B00, &cAudioManager::GetColumbianTalkSfx, PATCH_JUMP);
InjectHook(0x570EA0, &cAudioManager::GetCopTalkSfx, PATCH_JUMP);
InjectHook(0x57A8F0, &cAudioManager::GetCurrent3DProviderIndex, PATCH_JUMP);
InjectHook(0x569060, &cAudioManager::GetCollisionOneShotRatio, PATCH_JUMP);
InjectHook(0x5693B0, &cAudioManager::GetCollisionRatio, PATCH_JUMP);
InjectHook(0x571770, &cAudioManager::GetDiabloTalkSfx, PATCH_JUMP);
InjectHook(0x569750, &cAudioManager::GetDistanceSquared, PATCH_JUMP);
InjectHook(0x574DA0, &cAudioManager::GetEightTalkSfx, PATCH_JUMP);
InjectHook(0x574040, &cAudioManager::GetFanFemaleTalkSfx, PATCH_JUMP);
InjectHook(0x573F60, &cAudioManager::GetFanMaleTalkSfx, PATCH_JUMP);
InjectHook(0x571040, &cAudioManager::GetFBITalkSfx, PATCH_JUMP);
InjectHook(0x572280, &cAudioManager::GetFemaleNo3TalkSfx, PATCH_JUMP);
InjectHook(0x5712B0, &cAudioManager::GetFiremanTalkSfx, PATCH_JUMP);
InjectHook(0x574E50, &cAudioManager::GetFrankieTalkSfx, PATCH_JUMP);
InjectHook(0x575510, &cAudioManager::GetGenericFemaleTalkSfx, PATCH_JUMP);
InjectHook(0x575460, &cAudioManager::GetGenericMaleTalkSfx, PATCH_JUMP);
InjectHook(0x571C30, &cAudioManager::GetHoodTalkSfx, PATCH_JUMP);
InjectHook(0x5741F0, &cAudioManager::GetHospitalFemaleTalkSfx, PATCH_JUMP);
InjectHook(0x574120, &cAudioManager::GetHospitalMaleTalkSfx, PATCH_JUMP);
InjectHook(0x56F410, &cAudioManager::GetJumboTaxiFreq, PATCH_JUMP);
InjectHook(0x573310, &cAudioManager::GetLittleItalyFemaleOldTalkSfx, PATCH_JUMP);
InjectHook(0x573400, &cAudioManager::GetLittleItalyFemaleYoungTalkSfx, PATCH_JUMP);
InjectHook(0x5731E0, &cAudioManager::GetLittleItalyMaleTalkSfx, PATCH_JUMP);
InjectHook(0x571510, &cAudioManager::GetMafiaTalkSfx, PATCH_JUMP);
InjectHook(0x571F40, &cAudioManager::GetMaleNo2TalkSfx, PATCH_JUMP);
InjectHook(0x5711C0, &cAudioManager::GetMedicTalkSfx, PATCH_JUMP);
InjectHook(0x5795D0, &cAudioManager::GetMissionAudioLoadingStatus, PATCH_JUMP);
InjectHook(0x57F050, &cAudioManager::GetMissionScriptPoliceAudioPlayingStatus, PATCH_JUMP);
InjectHook(0x574F00, &cAudioManager::GetMistyTalkSfx, PATCH_JUMP);
InjectHook(0x575340, &cAudioManager::GetNormalMaleTalkSfx, PATCH_JUMP);
InjectHook(0x57A8A0, &cAudioManager::GetNum3DProvidersAvailable, PATCH_JUMP);
InjectHook(0x574FD0, &cAudioManager::GetOJGTalkSfx, PATCH_JUMP);
InjectHook(0x570960, &cAudioManager::GetPedCommentSfx, PATCH_JUMP);
InjectHook(0x570DB0, &cAudioManager::GetPhrase, PATCH_JUMP);
InjectHook(0x56BF80, &cAudioManager::GetVehicleDriveWheelSkidValue, PATCH_JUMP);
InjectHook(0x56C120, &cAudioManager::GetVehicleNonDriveWheelSkidValue, PATCH_JUMP);
InjectHook(0x575240, &cAudioManager::GetPimpTalkSfx, PATCH_JUMP);
InjectHook(0x570E00, &cAudioManager::GetPlayerTalkSfx, PATCH_JUMP);
InjectHook(0x5737E0, &cAudioManager::GetScumFemaleTalkSfx, PATCH_JUMP);
InjectHook(0x5736D0, &cAudioManager::GetScumMaleTalkSfx, PATCH_JUMP);
InjectHook(0x575060, &cAudioManager::GetSecurityGuardTalkSfx, PATCH_JUMP);
InjectHook(0x574480, &cAudioManager::GetShopperFemaleTalkSfx, PATCH_JUMP);
InjectHook(0x574790, &cAudioManager::GetSpecialCharacterTalkSfx, PATCH_JUMP);
InjectHook(0x573E90, &cAudioManager::GetStewardFemaleTalkSfx, PATCH_JUMP);
InjectHook(0x573DC0, &cAudioManager::GetStewardMaleTalkSfx, PATCH_JUMP);
InjectHook(0x574690, &cAudioManager::GetStudentFemaleTalkSfx, PATCH_JUMP);
InjectHook(0x574590, &cAudioManager::GetStudentMaleTalkSfx, PATCH_JUMP);
InjectHook(0x573CD0, &cAudioManager::GetSupermodelFemaleTalkSfx, PATCH_JUMP);
InjectHook(0x573BD0, &cAudioManager::GetSupermodelMaleTalkSfx, PATCH_JUMP);
InjectHook(0x570F80, &cAudioManager::GetSwatTalkSfx, PATCH_JUMP);
InjectHook(0x575190, &cAudioManager::GetTaxiDriverTalkSfx, PATCH_JUMP);
InjectHook(0x571650, &cAudioManager::GetTriadTalkSfx, PATCH_JUMP);
InjectHook(0x5723A0, &cAudioManager::GetWhiteBusinessFemaleTalkSfx, PATCH_JUMP);
InjectHook(0x572170, &cAudioManager::GetWhiteCasualFemaleTalkSfx, PATCH_JUMP);
InjectHook(0x574290, &cAudioManager::GetWhiteConstructionWorkerTalkSfx, PATCH_JUMP);
InjectHook(0x571E60, &cAudioManager::GetWhiteCriminalTalkSfx, PATCH_JUMP);
InjectHook(0x5734F0, &cAudioManager::GetWhiteDockerMaleTalkSfx, PATCH_JUMP);
InjectHook(0x5727B0, &cAudioManager::GetWhiteFatFemaleTalkSfx, PATCH_JUMP);
InjectHook(0x5725D0, &cAudioManager::GetWhiteFatMaleTalkSfx, PATCH_JUMP);
InjectHook(0x5729D0, &cAudioManager::GetWhiteFemaleProstituteTalkSfx, PATCH_JUMP);
InjectHook(0x5738D0, &cAudioManager::GetWhiteWorkerMaleTalkSfx, PATCH_JUMP);
InjectHook(0x5718D0, &cAudioManager::GetYakuzaTalkSfx, PATCH_JUMP);
InjectHook(0x5719E0, &cAudioManager::GetYardieTalkSfx, PATCH_JUMP);
InjectHook(0x56CAB0, &cAudioManager::HasAirBrakes, PATCH_JUMP);
InjectHook(0x57A0E0, &cAudioManager::Initialise, PATCH_JUMP);
InjectHook(0x57EEC0, &cAudioManager::InitialisePoliceRadio, PATCH_JUMP);
InjectHook(0x57EAC0, &cAudioManager::InitialisePoliceRadioZones, PATCH_JUMP);
InjectHook(0x57B030, &cAudioManager::InterrogateAudioEntities, PATCH_JUMP);
InjectHook(0x57AA50, &cAudioManager::IsAudioInitialised, PATCH_JUMP);
InjectHook(0x579650, &cAudioManager::IsMissionAudioSampleFinished, PATCH_JUMP);
InjectHook(0x57A9C0, &cAudioManager::IsMP3RadioChannelAvailable, PATCH_JUMP);
InjectHook(0x579520, &cAudioManager::MissionScriptAudioUsesPoliceChannel, PATCH_JUMP);
InjectHook(0x56AD10, &cAudioManager::PlayerJustGotInCar, PATCH_JUMP);
InjectHook(0x56AD20, &cAudioManager::PlayerJustLeftCar, PATCH_JUMP);
InjectHook(0x579620, &cAudioManager::PlayLoadedMissionAudio, PATCH_JUMP);
InjectHook(0x57A500, &cAudioManager::PlayOneShot, PATCH_JUMP);
InjectHook(0x569420, &cAudioManager::PostInitialiseGameSpecificSetup, PATCH_JUMP);
InjectHook(0x569640, &cAudioManager::PostTerminateGameSpecificShutdown, PATCH_JUMP);
InjectHook(0x569400, &cAudioManager::PreInitialiseGameSpecificSetup, PATCH_JUMP);
InjectHook(0x569570, &cAudioManager::PreTerminateGameSpecificShutdown, PATCH_JUMP);
// InjectHook(0x57BA60, &cAudioManager::ProcessActiveQueues, PATCH_JUMP);
InjectHook(0x56C940, &cAudioManager::ProcessAirBrakes, PATCH_JUMP);
InjectHook(0x577B30, &cAudioManager::ProcessAirportScriptObject, PATCH_JUMP);
InjectHook(0x56DE80, &cAudioManager::ProcessBoatEngine, PATCH_JUMP);
InjectHook(0x56E500, &cAudioManager::ProcessBoatMovingOverWater, PATCH_JUMP);
InjectHook(0x5790D0, &cAudioManager::ProcessBridge, PATCH_JUMP);
InjectHook(0x579250, &cAudioManager::ProcessBridgeMotor, PATCH_JUMP);
InjectHook(0x579310, &cAudioManager::ProcessBridgeOneShots, PATCH_JUMP);
InjectHook(0x579170, &cAudioManager::ProcessBridgeWarning, PATCH_JUMP);
InjectHook(0x56CC20, &cAudioManager::ProcessCarBombTick, PATCH_JUMP);
InjectHook(0x577CA0, &cAudioManager::ProcessCinemaScriptObject, PATCH_JUMP);
InjectHook(0x577E50, &cAudioManager::ProcessDocksScriptObject, PATCH_JUMP);
InjectHook(0x56CAF0, &cAudioManager::ProcessEngineDamage, PATCH_JUMP);
InjectHook(0x569870, &cAudioManager::ProcessEntity, PATCH_JUMP);
InjectHook(0x575AC0, &cAudioManager::ProcessExplosions, PATCH_JUMP);
InjectHook(0x578FD0, &cAudioManager::ProcessFireHydrant, PATCH_JUMP);
InjectHook(0x5785E0, &cAudioManager::ProcessFrontEnd, PATCH_JUMP);
InjectHook(0x56E6A0, &cAudioManager::ProcessHelicopter, PATCH_JUMP);
InjectHook(0x577FE0, &cAudioManager::ProcessHomeScriptObject, PATCH_JUMP);
InjectHook(0x56E8F0, &cAudioManager::ProcessJumbo, PATCH_JUMP);
InjectHook(0x56EA40, &cAudioManager::ProcessJumboAccel, PATCH_JUMP);
InjectHook(0x56EE40, &cAudioManager::ProcessJumboDecel, PATCH_JUMP);
InjectHook(0x56ECF0, &cAudioManager::ProcessJumboFlying, PATCH_JUMP);
InjectHook(0x56ED10, &cAudioManager::ProcessJumboLanding, PATCH_JUMP);
InjectHook(0x56EC00, &cAudioManager::ProcessJumboTakeOff, PATCH_JUMP);
InjectHook(0x56EA10, &cAudioManager::ProcessJumboTaxi, PATCH_JUMP);
InjectHook(0x5777E0, &cAudioManager::ProcessLaunderetteScriptObject, PATCH_JUMP);
InjectHook(0x576770, &cAudioManager::ProcessLoopingScriptObject, PATCH_JUMP);
InjectHook(0x5796E0, &cAudioManager::ProcessMissionAudio, PATCH_JUMP);
InjectHook(0x56A050, &cAudioManager::ProcessModelCarEngine, PATCH_JUMP);
InjectHook(0x5760C0, &cAudioManager::ProcessOneShotScriptObject, PATCH_JUMP);
InjectHook(0x56F450, &cAudioManager::ProcessPed, PATCH_JUMP);
InjectHook(0x56F4D0, &cAudioManager::ProcessPedHeadphones, PATCH_JUMP);
// InjectHook(0x56F650, &cAudioManager::ProcessPedOneShots, PATCH_JUMP);
InjectHook(0x5699C0, &cAudioManager::ProcessPhysical, PATCH_JUMP);
InjectHook(0x56E860, &cAudioManager::ProcessPlane, PATCH_JUMP);
InjectHook(0x56B0D0, &cAudioManager::ProcessPlayersVehicleEngine, PATCH_JUMP);
InjectHook(0x578190, &cAudioManager::ProcessPoliceCellBeatingScriptObject, PATCH_JUMP);
InjectHook(0x577280, &cAudioManager::ProcessPornCinema, PATCH_JUMP);
InjectHook(0x578A80, &cAudioManager::ProcessProjectiles, PATCH_JUMP);
InjectHook(0x569CC0, &cAudioManager::ProcessRainOnVehicle, PATCH_JUMP);
InjectHook(0x569700, &cAudioManager::ProcessReverb, PATCH_JUMP);
InjectHook(0x569E50, &cAudioManager::ProcessReverseGear, PATCH_JUMP);
InjectHook(0x577630, &cAudioManager::ProcessSawMillScriptObject, PATCH_JUMP);
InjectHook(0x576070, &cAudioManager::ProcessScriptObject, PATCH_JUMP);
InjectHook(0x577970, &cAudioManager::ProcessShopScriptObject, PATCH_JUMP);
InjectHook(0x5697D0, &cAudioManager::ProcessSpecial, PATCH_JUMP);
InjectHook(0x56DBF0, &cAudioManager::ProcessTrainNoise, PATCH_JUMP);
InjectHook(0x569A00, &cAudioManager::ProcessVehicle, PATCH_JUMP);
InjectHook(0x56C770, &cAudioManager::ProcessVehicleDoors, PATCH_JUMP);
InjectHook(0x56C200, &cAudioManager::ProcessVehicleHorn, PATCH_JUMP);
InjectHook(0x56C640, &cAudioManager::ProcessVehicleReverseWarning, PATCH_JUMP);
InjectHook(0x56A230, &cAudioManager::ProcessVehicleRoadNoise, PATCH_JUMP);
InjectHook(0x56BCB0, &cAudioManager::ProcessVehicleSkidding, PATCH_JUMP);
InjectHook(0x575F30, &cAudioManager::ProcessWaterCannon, PATCH_JUMP);
InjectHook(0x578370, &cAudioManager::ProcessWeather, PATCH_JUMP);
InjectHook(0x56A440, &cAudioManager::ProcessWetRoadNoise, PATCH_JUMP);
InjectHook(0x577530, &cAudioManager::ProcessWorkShopScriptObject, PATCH_JUMP);
InjectHook(0x57AF90, &cAudioManager::RandomDisplacement, PATCH_JUMP);
InjectHook(0x57A9F0, &cAudioManager::ReacquireDigitalHandle, PATCH_JUMP);
InjectHook(0x57A9E0, &cAudioManager::ReleaseDigitalHandle, PATCH_JUMP);
InjectHook(0x568410, &cAudioManager::ReportCollision, PATCH_JUMP);
InjectHook(0x5803D0, &cAudioManager::ReportCrime, PATCH_JUMP);
InjectHook(0x569650, &cAudioManager::ResetAudioLogicTimers, PATCH_JUMP);
InjectHook(0x57EFF0, &cAudioManager::ResetPoliceRadio, PATCH_JUMP);
InjectHook(0x57A7B0, &cAudioManager::ResetTimers, PATCH_JUMP);
InjectHook(0x57A2A0, &cAudioManager::Service, PATCH_JUMP);
InjectHook(0x57F110, &cAudioManager::ServicePoliceRadio, PATCH_JUMP);
InjectHook(0x57F1B0, &cAudioManager::ServicePoliceRadioChannel, PATCH_JUMP);
InjectHook(0x57AA60, &cAudioManager::ServiceSoundEffects, PATCH_JUMP);
InjectHook(0x57A910, &cAudioManager::SetCurrent3DProvider, PATCH_JUMP);
InjectHook(0x57AA00, &cAudioManager::SetDynamicAcousticModelingStatus, PATCH_JUMP);
InjectHook(0x57A770, &cAudioManager::SetEffectsFadeVolume, PATCH_JUMP);
InjectHook(0x57A730, &cAudioManager::SetEffectsMasterVolume, PATCH_JUMP);
InjectHook(0x57A4C0, &cAudioManager::SetEntityStatus, PATCH_JUMP);
InjectHook(0x568E20, &cAudioManager::SetLoopingCollisionRequestedSfxFreqAndGetVol, PATCH_JUMP);
InjectHook(0x5795F0, &cAudioManager::SetMissionAudioLocation, PATCH_JUMP);
InjectHook(0x57F020, &cAudioManager::SetMissionScriptPoliceAudio, PATCH_JUMP);
InjectHook(0x57A790, &cAudioManager::SetMusicFadeVolume, PATCH_JUMP);
InjectHook(0x57A750, &cAudioManager::SetMusicMasterVolume, PATCH_JUMP);
InjectHook(0x57A9A0, &cAudioManager::SetSpeakerConfig, PATCH_JUMP);
InjectHook(0x568D30, &cAudioManager::SetUpLoopingCollisionSound, PATCH_JUMP);
InjectHook(0x5689D0, &cAudioManager::SetUpOneShotCollisionSound, PATCH_JUMP);
InjectHook(0x56F230, &cAudioManager::SetupJumboFlySound, PATCH_JUMP);
InjectHook(0x56F310, &cAudioManager::SetupJumboRumbleSound, PATCH_JUMP);
InjectHook(0x56EF20, &cAudioManager::SetupJumboTaxiSound, PATCH_JUMP);
InjectHook(0x56F070, &cAudioManager::SetupJumboWhineSound, PATCH_JUMP);
InjectHook(0x570690, &cAudioManager::SetupPedComments, PATCH_JUMP);
InjectHook(0x57A150, &cAudioManager::Terminate, PATCH_JUMP);
InjectHook(0x57AC60, &cAudioManager::TranslateEntity, PATCH_JUMP);
InjectHook(0x56AC80, &cAudioManager::UpdateGasPedalAudio, PATCH_JUMP);
InjectHook(0x56C600, &cAudioManager::UsesReverseWarning, PATCH_JUMP);
InjectHook(0x56C3C0, &cAudioManager::UsesSiren, PATCH_JUMP);
InjectHook(0x56C3F0, &cAudioManager::UsesSirenSwitching, PATCH_JUMP);
InjectHook(0x57C320, &cAudioManager::sub_57C320, PATCH_JUMP);
InjectHook(0x5755C0, &cPedComments::Add, PATCH_JUMP);
InjectHook(0x575730, &cPedComments::Process, PATCH_JUMP);
InjectHook(0x5685E0, &cAudioCollisionManager::AddCollisionToRequestedQueue, PATCH_JUMP);
InjectHook(0x57C430, &cAudioScriptObject::Reset, PATCH_JUMP);
ENDPATCHES
