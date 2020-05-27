#include "common.h"

#include "DMAudio.h"

#include "AudioManager.h"

#include "AudioSamples.h"
#include "MusicManager.h"
#include "PlayerPed.h"
#include "PoliceRadio.h"
#include "Replay.h"
#include "Vehicle.h"
#include "World.h"
#include "Zones.h"
#include "sampman.h"

const int channels = ARRAY_SIZE(cAudioManager::m_asActiveSamples);
const int policeChannel = channels + 1;

struct tPoliceRadioZone {
	char m_aName[8];
	uint32 m_nSampleIndex;
	int32 field_12;
};

tPoliceRadioZone ZoneSfx[NUMAUDIOZONES];

int32 g_nMissionAudioSfx = TOTAL_AUDIO_SAMPLES;
int8 g_nMissionAudioPlayingStatus = 2;
uint8 gSpecialSuspectLastSeenReport;
uint32 gMinTimeToNextReport[NUM_CRIME_TYPES];

void
cAudioManager::InitialisePoliceRadioZones()
{
	for (int32 i = 0; i < NUMAUDIOZONES; i++)
		memset(ZoneSfx[i].m_aName, 0, 8);

#define SETZONESFX(i, name, sample) \
	strcpy(ZoneSfx[i].m_aName, name); \
	ZoneSfx[i].m_nSampleIndex = sample;

	SETZONESFX(0, "VICE_C", SFX_POLICE_RADIO_VICE_CITY);
	SETZONESFX(1, "IND_ZON", SFX_POLICE_RADIO_VICE_CITY_BEACH);
	SETZONESFX(2, "COM_ZON", SFX_POLICE_RADIO_VICE_CITY_MAINLAND);
	SETZONESFX(3, "BEACH1", SFX_POLICE_RADIO_OCEAN_BEACH);
	SETZONESFX(4, "BEACH2", SFX_POLICE_RADIO_WASHINGTON_BEACH);
	SETZONESFX(5, "BEACH3", SFX_POLICE_RADIO_VICE_POINT);
	SETZONESFX(6, "GOLFC", SFX_POLICE_RADIO_LEAF_LINKS);
	SETZONESFX(7, "STARI", SFX_POLICE_RADIO_STRAFISH_ISLAND);
	SETZONESFX(8, "DOCKS", SFX_POLICE_RADIO_VICE_PORT);
	SETZONESFX(9, "HAVANA", SFX_POLICE_RADIO_LITTLE_HAVANA);
	SETZONESFX(10, "HAITI", SFX_POLICE_RADIO_LITTLE_HAITI);
	SETZONESFX(11, "PORNI", SFX_POLICE_RADIO_PRAWN_ISLAND);
	SETZONESFX(12, "DTOWN", SFX_POLICE_RADIO_DOWNTOWN);
	SETZONESFX(13, "A_PORT", SFX_POLICE_RADIO_ESCOBAR_INTERNATIONAL);

#undef SETZONESFX
}

void
cAudioManager::InitialisePoliceRadio()
{
	m_sPoliceRadioQueue.policeChannelTimer = 0;
	m_sPoliceRadioQueue.policeChannelTimerSeconds = 0;
	m_sPoliceRadioQueue.policeChannelCounterSeconds = 0;
	for (int32 i = 0; i < ARRAY_SIZE(m_sPoliceRadioQueue.crimes); i++)
		m_sPoliceRadioQueue.crimes[i].type = 0;

	SampleManager.SetChannelReverbFlag(policeChannel, 0);
	gSpecialSuspectLastSeenReport = false;
	for (int32 i = 0; i < ARRAY_SIZE(gMinTimeToNextReport); i++)
		gMinTimeToNextReport[i] = m_FrameCounter;
}

void
cAudioManager::ResetPoliceRadio()
{
	if (!m_bIsInitialised) return;
	if (SampleManager.GetChannelUsedFlag(policeChannel)) SampleManager.StopChannel(policeChannel);
	InitialisePoliceRadio();
}

void
cAudioManager::SetMissionScriptPoliceAudio(int32 sfx) const
{
	if (!m_bIsInitialised) return;
	if (g_nMissionAudioPlayingStatus != 1) {
		g_nMissionAudioPlayingStatus = 0;
		g_nMissionAudioSfx = sfx;
	}
}

int8
cAudioManager::GetMissionScriptPoliceAudioPlayingStatus() const
{
	return g_nMissionAudioPlayingStatus;
}

void
cAudioManager::DoPoliceRadioCrackle()
{
	m_sQueueSample.m_nEntityIndex = m_nPoliceChannelEntity;
	m_sQueueSample.m_nCounter = 0;
	m_sQueueSample.m_nSampleIndex = SFX_POLICE_RADIO_CRACKLE;
	m_sQueueSample.m_nBankIndex = SAMPLEBANK_MAIN;
	m_sQueueSample.m_bIs2D = true;
	m_sQueueSample.m_nReleasingVolumeModificator = 10;
	m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_POLICE_RADIO_CRACKLE);
	m_sQueueSample.m_nVolume = m_anRandomTable[2] % 20 + 15;
	m_sQueueSample.m_nLoopCount = 0;
	m_sQueueSample.m_nEmittingVolume = m_sQueueSample.m_nVolume;
	m_sQueueSample.m_nLoopStart = SampleManager.GetSampleLoopStartOffset(SFX_POLICE_RADIO_CRACKLE);
	m_sQueueSample.m_nLoopEnd = SampleManager.GetSampleLoopEndOffset(SFX_POLICE_RADIO_CRACKLE);
	m_sQueueSample.m_bReleasingSoundFlag = false;
	m_sQueueSample.m_bReverbFlag = false;
	m_sQueueSample.m_nOffset = 63;
	m_sQueueSample.m_nReleasingVolumeDivider = 3;
	m_sQueueSample.m_bRequireReflection = false;
	AddSampleToRequestedQueue();
}

void
cAudioManager::ServicePoliceRadio()
{
	int32 wantedLevel = 0; // uninitialized variable
	static uint32 nLastSeen = 300;

	if(!m_bIsInitialised) return;

	if(!m_nUserPause) {
		bool crimeReport = SetupCrimeReport();
#ifdef FIX_BUGS // Crash at 0x5fe6ef
		if(CReplay::IsPlayingBack() || !FindPlayerPed() || !FindPlayerPed()->m_pWanted)
			return;
#endif
		wantedLevel = FindPlayerPed()->m_pWanted->m_nWantedLevel;
		if(!crimeReport) {
			if(wantedLevel) {
				if(nLastSeen) {
					--nLastSeen;
				} else {
					nLastSeen = m_anRandomTable[1] % 1000 + 2000;
					SetupSuspectLastSeenReport();
				}
			}
		}
	}
	ServicePoliceRadioChannel(wantedLevel);
}

void
cAudioManager::ServicePoliceRadioChannel(int32 wantedLevel)
{
	bool processed = false;
	uint32 sample;
	int32 freq;

	static int cWait = 0;
	static bool bChannelOpen = false;
	static uint8 bMissionAudioPhysicalPlayingStatus = 0;
	static int32 PoliceChannelFreq = 5500;

	if (!m_bIsInitialised) return;

	if (m_nUserPause) {
		if (SampleManager.GetChannelUsedFlag(policeChannel)) SampleManager.StopChannel(policeChannel);
		if (g_nMissionAudioSfx != TOTAL_AUDIO_SAMPLES && bMissionAudioPhysicalPlayingStatus == 1 &&
			SampleManager.IsStreamPlaying(1)) {
			SampleManager.PauseStream(1, 1);
		}
	} else {
		if (m_nPreviousUserPause && g_nMissionAudioSfx != TOTAL_AUDIO_SAMPLES &&
			bMissionAudioPhysicalPlayingStatus == 1) {
			SampleManager.PauseStream(0, 1);
		}
		if (m_sPoliceRadioQueue.policeChannelTimer == 0) bChannelOpen = false;
		if (cWait) {
			--cWait;
			return;
		}
		if (g_nMissionAudioSfx != TOTAL_AUDIO_SAMPLES && !bChannelOpen) {
			if (g_nMissionAudioPlayingStatus) {
				if (g_nMissionAudioPlayingStatus == 1 && !bMissionAudioPhysicalPlayingStatus &&
					SampleManager.IsStreamPlaying(1)) {
					bMissionAudioPhysicalPlayingStatus = 1;
				}
				if (bMissionAudioPhysicalPlayingStatus == 1) {
					if (SampleManager.IsStreamPlaying(1)) {
						DoPoliceRadioCrackle();
					} else {
						bMissionAudioPhysicalPlayingStatus = 2;
						g_nMissionAudioPlayingStatus = 2;
						g_nMissionAudioSfx = TOTAL_AUDIO_SAMPLES;
						cWait = 30;
					}
					return;
				}
			} else if (!SampleManager.GetChannelUsedFlag(policeChannel)) {
				SampleManager.PreloadStreamedFile(g_nMissionAudioSfx, 1);
				SampleManager.SetStreamedVolumeAndPan(MAX_VOLUME, 63, 1, 1);
				SampleManager.StartPreloadedStreamedFile(1);
				g_nMissionAudioPlayingStatus = 1;
				bMissionAudioPhysicalPlayingStatus = 0;
				return;
			}
		}
		if (bChannelOpen) DoPoliceRadioCrackle();
		if ((g_nMissionAudioSfx == TOTAL_AUDIO_SAMPLES || g_nMissionAudioPlayingStatus != 1) &&
			!SampleManager.GetChannelUsedFlag(policeChannel) && m_sPoliceRadioQueue.policeChannelTimer) {
			if (m_sPoliceRadioQueue.policeChannelTimer) {
				sample = m_sPoliceRadioQueue.crimesSamples[m_sPoliceRadioQueue.policeChannelCounterSeconds];
				m_sPoliceRadioQueue.policeChannelTimer--;
				m_sPoliceRadioQueue.policeChannelCounterSeconds = (m_sPoliceRadioQueue.policeChannelCounterSeconds + 1) % 60;
			} else {
				sample = TOTAL_AUDIO_SAMPLES;
			}
			if (!wantedLevel) {
				if (gSpecialSuspectLastSeenReport) {
					gSpecialSuspectLastSeenReport = 0;
				} else if (sample == SFX_POLICE_RADIO_MESSAGE_NOISE_1 || sample == TOTAL_AUDIO_SAMPLES) {
					bChannelOpen = false;
					processed = true;
				}
			}
			if (sample == TOTAL_AUDIO_SAMPLES) {
				if (!processed) cWait = 30;
			} else {
				SampleManager.InitialiseChannel(policeChannel, sample, 0);
				switch (sample) {
				case SFX_POLICE_RADIO_MESSAGE_NOISE_1:
					freq = m_anRandomTable[4] % 2000 + 10025;
					bChannelOpen = bChannelOpen == false;
					break;
				default: freq = SampleManager.GetSampleBaseFrequency(sample); break;
				}
				PoliceChannelFreq = freq;
				SampleManager.SetChannelFrequency(policeChannel, freq);
				SampleManager.SetChannelVolume(policeChannel, 100);
				SampleManager.SetChannelPan(policeChannel, 63);
				SampleManager.SetChannelLoopCount(policeChannel, 1);
				SampleManager.SetChannelLoopPoints(policeChannel, 0, -1);
				SampleManager.StartChannel(policeChannel);
			}
			if (processed) ResetPoliceRadio();
		}
	}
}

bool
cAudioManager::SetupCrimeReport()
{
	int16 audioZoneId;
	CZone *zone;
	float rangeX;
	float rangeY;
	float halfX;
	float halfY;
	float quarterX;
	float quarterY;
	int i;
	int32 sampleIndex;
	bool processed = false;

	if (MusicManager.m_nMusicMode == MUSICMODE_CUTSCENE) return false;

	if (60 - m_sPoliceRadioQueue.policeChannelTimer <= 9) {
		AgeCrimes();
		return true;
	}

	for (i = 0; i < ARRAY_SIZE(m_sPoliceRadioQueue.crimes); i++) {
		if (m_sPoliceRadioQueue.crimes[i].type != CRIME_NONE)
			break;
	}

	if (i == ARRAY_SIZE(m_sPoliceRadioQueue.crimes)) return false;
	audioZoneId = CTheZones::FindAudioZone(&m_sPoliceRadioQueue.crimes[i].position);
	if (audioZoneId >= 0 && audioZoneId < NUMAUDIOZONES) {
		zone = CTheZones::GetAudioZone(audioZoneId);
		for (int j = 0; j < NUMAUDIOZONES; j++) {
			if (strcmp(zone->name, ZoneSfx[j].m_aName) == 0) {
				sampleIndex = ZoneSfx[j].m_nSampleIndex;
				m_sPoliceRadioQueue.Add(m_anRandomTable[4] % 3 + SFX_POLICE_RADIO_MESSAGE_NOISE_1);
				m_sPoliceRadioQueue.Add(m_anRandomTable[0] % 3 + SFX_WEVE_GOT);
				m_sPoliceRadioQueue.Add(SFX_A_10);
				switch (m_sPoliceRadioQueue.crimes[i].type) {
				case CRIME_PED_BURNED: m_sPoliceRadioQueue.crimes[i].type = CRIME_HIT_PED; break;
				case CRIME_COP_BURNED: m_sPoliceRadioQueue.crimes[i].type = CRIME_HIT_COP; break;
				case CRIME_VEHICLE_BURNED: m_sPoliceRadioQueue.crimes[i].type = CRIME_STEAL_CAR; break;
				case CRIME_DESTROYED_CESSNA: m_sPoliceRadioQueue.crimes[i].type = CRIME_SHOOT_HELI; break;
				default: break;
				}
				m_sPoliceRadioQueue.Add(m_sPoliceRadioQueue.crimes[i].type + SFX_CRIME_1 - 1);
				m_sPoliceRadioQueue.Add(SFX_IN);
				rangeX = zone->maxx - zone->minx;
				rangeY = zone->maxy - zone->miny;
				halfX = 0.5f * rangeX + zone->minx;
				halfY = 0.5f * rangeY + zone->miny;
				quarterX = 0.25f * rangeX;
				quarterY = 0.25f * rangeY;

				if (m_sPoliceRadioQueue.crimes[i].position.y > halfY + quarterY) {
					m_sPoliceRadioQueue.Add(SFX_NORTH);
					processed = true;
				} else if (m_sPoliceRadioQueue.crimes[i].position.y < halfY - quarterY) {
					m_sPoliceRadioQueue.Add(SFX_SOUTH);
					processed = true;
				}

				if (m_sPoliceRadioQueue.crimes[i].position.x > halfX + quarterX)
					m_sPoliceRadioQueue.Add(SFX_EAST);
				else if (m_sPoliceRadioQueue.crimes[i].position.x < halfX - quarterX)
					m_sPoliceRadioQueue.Add(SFX_WEST);
				else if (!processed)
					m_sPoliceRadioQueue.Add(SFX_CENTRAL);

				m_sPoliceRadioQueue.Add(sampleIndex);
				m_sPoliceRadioQueue.Add(m_anRandomTable[2] % 3 + SFX_POLICE_RADIO_MESSAGE_NOISE_1);
				m_sPoliceRadioQueue.Add(TOTAL_AUDIO_SAMPLES);
				break;
			}
		}
	}
	m_sPoliceRadioQueue.crimes[i].type = CRIME_NONE;
	AgeCrimes();
	return true;
}

void
cAudioManager::SetupSuspectLastSeenReport()
{
	CVehicle *veh;
	uint8 color1;
	int32 main_color;
	int32 sample;

	int32 color_pre_modifier;
	int32 color_post_modifier;

	const int32 gCarColourTable[][3] = {
		{TOTAL_AUDIO_SAMPLES, SFX_POLICE_RADIO_BLACK, TOTAL_AUDIO_SAMPLES},
		{TOTAL_AUDIO_SAMPLES, SFX_POLICE_RADIO_WHITE, TOTAL_AUDIO_SAMPLES},
		{TOTAL_AUDIO_SAMPLES, SFX_POLICE_RADIO_BLUE, TOTAL_AUDIO_SAMPLES},
		{TOTAL_AUDIO_SAMPLES, SFX_POLICE_RADIO_RED, TOTAL_AUDIO_SAMPLES},
		{SFX_POLICE_RADIO_DARK, SFX_POLICE_RADIO_BLUE, TOTAL_AUDIO_SAMPLES},
		{TOTAL_AUDIO_SAMPLES, SFX_POLICE_RADIO_PURPLE, TOTAL_AUDIO_SAMPLES},
		{TOTAL_AUDIO_SAMPLES, SFX_POLICE_RADIO_YELLOW, TOTAL_AUDIO_SAMPLES},
		{SFX_POLICE_RADIO_BRIGHT, SFX_POLICE_RADIO_BLUE, TOTAL_AUDIO_SAMPLES},
		{SFX_POLICE_RADIO_LIGHT, SFX_POLICE_RADIO_BLUE, SFX_POLICE_RADIO_GREY},
#ifdef FIX_BUGS
		{SFX_POLICE_RADIO_LIGHT, SFX_POLICE_RADIO_RED, TOTAL_AUDIO_SAMPLES},
		{SFX_POLICE_RADIO_DARK, SFX_POLICE_RADIO_RED, TOTAL_AUDIO_SAMPLES},
		{SFX_POLICE_RADIO_DARK, SFX_POLICE_RADIO_RED, TOTAL_AUDIO_SAMPLES},
#else
		{SFX_POLICE_RADIO_LIGHT, TOTAL_AUDIO_SAMPLES, TOTAL_AUDIO_SAMPLES},
		{SFX_POLICE_RADIO_DARK, TOTAL_AUDIO_SAMPLES, TOTAL_AUDIO_SAMPLES},
		{SFX_POLICE_RADIO_DARK, TOTAL_AUDIO_SAMPLES, TOTAL_AUDIO_SAMPLES},
#endif
		{SFX_POLICE_RADIO_DARK, SFX_POLICE_RADIO_RED, TOTAL_AUDIO_SAMPLES},
		{TOTAL_AUDIO_SAMPLES, SFX_POLICE_RADIO_RED, TOTAL_AUDIO_SAMPLES},
		{TOTAL_AUDIO_SAMPLES, SFX_POLICE_RADIO_RED, TOTAL_AUDIO_SAMPLES},
		{TOTAL_AUDIO_SAMPLES, SFX_POLICE_RADIO_RED, TOTAL_AUDIO_SAMPLES},
		{TOTAL_AUDIO_SAMPLES, SFX_POLICE_RADIO_RED, TOTAL_AUDIO_SAMPLES},
		{TOTAL_AUDIO_SAMPLES, SFX_POLICE_RADIO_RED, TOTAL_AUDIO_SAMPLES},
		{TOTAL_AUDIO_SAMPLES, SFX_POLICE_RADIO_RED, TOTAL_AUDIO_SAMPLES},
#ifdef FIX_BUGS
		{SFX_POLICE_RADIO_LIGHT, SFX_POLICE_RADIO_RED, TOTAL_AUDIO_SAMPLES},
		{SFX_POLICE_RADIO_DARK, SFX_POLICE_RADIO_ORANGE, TOTAL_AUDIO_SAMPLES},
		{SFX_POLICE_RADIO_DARK, SFX_POLICE_RADIO_ORANGE, TOTAL_AUDIO_SAMPLES},
		{SFX_POLICE_RADIO_DARK, SFX_POLICE_RADIO_ORANGE, TOTAL_AUDIO_SAMPLES},
#else
		{SFX_POLICE_RADIO_LIGHT, TOTAL_AUDIO_SAMPLES, TOTAL_AUDIO_SAMPLES},
		{SFX_POLICE_RADIO_DARK, TOTAL_AUDIO_SAMPLES, TOTAL_AUDIO_SAMPLES},
		{SFX_POLICE_RADIO_DARK, TOTAL_AUDIO_SAMPLES, TOTAL_AUDIO_SAMPLES},
		{SFX_POLICE_RADIO_DARK, TOTAL_AUDIO_SAMPLES, TOTAL_AUDIO_SAMPLES},
#endif
		{TOTAL_AUDIO_SAMPLES, SFX_POLICE_RADIO_ORANGE, TOTAL_AUDIO_SAMPLES},
		{TOTAL_AUDIO_SAMPLES, SFX_POLICE_RADIO_ORANGE, TOTAL_AUDIO_SAMPLES},
		{TOTAL_AUDIO_SAMPLES, SFX_POLICE_RADIO_ORANGE, TOTAL_AUDIO_SAMPLES},
		{TOTAL_AUDIO_SAMPLES, SFX_POLICE_RADIO_ORANGE, TOTAL_AUDIO_SAMPLES},
		{TOTAL_AUDIO_SAMPLES, SFX_POLICE_RADIO_ORANGE, TOTAL_AUDIO_SAMPLES},
		{TOTAL_AUDIO_SAMPLES, SFX_POLICE_RADIO_ORANGE, TOTAL_AUDIO_SAMPLES},
#ifdef FIX_BUGS
		{SFX_POLICE_RADIO_LIGHT, SFX_POLICE_RADIO_ORANGE, TOTAL_AUDIO_SAMPLES},
		{SFX_POLICE_RADIO_DARK, SFX_POLICE_RADIO_YELLOW, TOTAL_AUDIO_SAMPLES},
		{SFX_POLICE_RADIO_DARK, SFX_POLICE_RADIO_YELLOW, TOTAL_AUDIO_SAMPLES},
		{SFX_POLICE_RADIO_DARK, SFX_POLICE_RADIO_YELLOW, TOTAL_AUDIO_SAMPLES},
#else
		{SFX_POLICE_RADIO_LIGHT, TOTAL_AUDIO_SAMPLES, TOTAL_AUDIO_SAMPLES},
		{SFX_POLICE_RADIO_DARK, TOTAL_AUDIO_SAMPLES, TOTAL_AUDIO_SAMPLES},
		{SFX_POLICE_RADIO_DARK, TOTAL_AUDIO_SAMPLES, TOTAL_AUDIO_SAMPLES},
		{SFX_POLICE_RADIO_DARK, TOTAL_AUDIO_SAMPLES, TOTAL_AUDIO_SAMPLES},
#endif
		{TOTAL_AUDIO_SAMPLES, SFX_POLICE_RADIO_YELLOW, TOTAL_AUDIO_SAMPLES},
		{TOTAL_AUDIO_SAMPLES, SFX_POLICE_RADIO_YELLOW, TOTAL_AUDIO_SAMPLES},
		{TOTAL_AUDIO_SAMPLES, SFX_POLICE_RADIO_YELLOW, TOTAL_AUDIO_SAMPLES},
		{TOTAL_AUDIO_SAMPLES, SFX_POLICE_RADIO_YELLOW, TOTAL_AUDIO_SAMPLES},
		{TOTAL_AUDIO_SAMPLES, SFX_POLICE_RADIO_YELLOW, TOTAL_AUDIO_SAMPLES},
		{TOTAL_AUDIO_SAMPLES, SFX_POLICE_RADIO_YELLOW, TOTAL_AUDIO_SAMPLES},
#ifdef FIX_BUGS
		{SFX_POLICE_RADIO_LIGHT, SFX_POLICE_RADIO_YELLOW, TOTAL_AUDIO_SAMPLES},
		{SFX_POLICE_RADIO_DARK, SFX_POLICE_RADIO_GREEN, TOTAL_AUDIO_SAMPLES},
		{SFX_POLICE_RADIO_DARK, SFX_POLICE_RADIO_GREEN, TOTAL_AUDIO_SAMPLES},
		{SFX_POLICE_RADIO_DARK, SFX_POLICE_RADIO_GREEN, TOTAL_AUDIO_SAMPLES},
#else
		{SFX_POLICE_RADIO_LIGHT, TOTAL_AUDIO_SAMPLES, TOTAL_AUDIO_SAMPLES},
		{SFX_POLICE_RADIO_DARK, TOTAL_AUDIO_SAMPLES, TOTAL_AUDIO_SAMPLES},
		{SFX_POLICE_RADIO_DARK, TOTAL_AUDIO_SAMPLES, TOTAL_AUDIO_SAMPLES},
		{SFX_POLICE_RADIO_DARK, TOTAL_AUDIO_SAMPLES, TOTAL_AUDIO_SAMPLES},
#endif
		{TOTAL_AUDIO_SAMPLES, SFX_POLICE_RADIO_GREEN, TOTAL_AUDIO_SAMPLES},
		{TOTAL_AUDIO_SAMPLES, SFX_POLICE_RADIO_GREEN, TOTAL_AUDIO_SAMPLES},
		{TOTAL_AUDIO_SAMPLES, SFX_POLICE_RADIO_GREEN, TOTAL_AUDIO_SAMPLES},
		{TOTAL_AUDIO_SAMPLES, SFX_POLICE_RADIO_GREEN, TOTAL_AUDIO_SAMPLES},
		{TOTAL_AUDIO_SAMPLES, SFX_POLICE_RADIO_GREEN, TOTAL_AUDIO_SAMPLES},
		{TOTAL_AUDIO_SAMPLES, SFX_POLICE_RADIO_GREEN, TOTAL_AUDIO_SAMPLES},
#ifdef FIX_BUGS
		{SFX_POLICE_RADIO_LIGHT, SFX_POLICE_RADIO_GREEN, TOTAL_AUDIO_SAMPLES},
		{SFX_POLICE_RADIO_DARK, SFX_POLICE_RADIO_BLUE, TOTAL_AUDIO_SAMPLES},
		{SFX_POLICE_RADIO_DARK, SFX_POLICE_RADIO_BLUE, TOTAL_AUDIO_SAMPLES},
		{SFX_POLICE_RADIO_DARK, SFX_POLICE_RADIO_BLUE, TOTAL_AUDIO_SAMPLES},
#else
		{SFX_POLICE_RADIO_LIGHT, TOTAL_AUDIO_SAMPLES, TOTAL_AUDIO_SAMPLES},
		{SFX_POLICE_RADIO_DARK, TOTAL_AUDIO_SAMPLES, TOTAL_AUDIO_SAMPLES},
		{SFX_POLICE_RADIO_DARK, TOTAL_AUDIO_SAMPLES, TOTAL_AUDIO_SAMPLES},
		{SFX_POLICE_RADIO_DARK, TOTAL_AUDIO_SAMPLES, TOTAL_AUDIO_SAMPLES},
#endif
		{TOTAL_AUDIO_SAMPLES, SFX_POLICE_RADIO_BLUE, TOTAL_AUDIO_SAMPLES},
		{TOTAL_AUDIO_SAMPLES, SFX_POLICE_RADIO_BLUE, TOTAL_AUDIO_SAMPLES},
		{TOTAL_AUDIO_SAMPLES, SFX_POLICE_RADIO_BLUE, TOTAL_AUDIO_SAMPLES},
		{TOTAL_AUDIO_SAMPLES, SFX_POLICE_RADIO_BLUE, TOTAL_AUDIO_SAMPLES},
		{TOTAL_AUDIO_SAMPLES, SFX_POLICE_RADIO_BLUE, TOTAL_AUDIO_SAMPLES},
		{TOTAL_AUDIO_SAMPLES, SFX_POLICE_RADIO_BLUE, TOTAL_AUDIO_SAMPLES},
#ifdef FIX_BUGS
		{SFX_POLICE_RADIO_LIGHT, SFX_POLICE_RADIO_BLUE, TOTAL_AUDIO_SAMPLES},
		{SFX_POLICE_RADIO_DARK, SFX_POLICE_RADIO_PURPLE, TOTAL_AUDIO_SAMPLES},
		{SFX_POLICE_RADIO_DARK, SFX_POLICE_RADIO_PURPLE, SFX_POLICE_RADIO_BLUE},
		{SFX_POLICE_RADIO_DARK, SFX_POLICE_RADIO_PURPLE, TOTAL_AUDIO_SAMPLES},
#else
		{SFX_POLICE_RADIO_LIGHT, TOTAL_AUDIO_SAMPLES, TOTAL_AUDIO_SAMPLES},
		{SFX_POLICE_RADIO_DARK, TOTAL_AUDIO_SAMPLES, TOTAL_AUDIO_SAMPLES},
		{SFX_POLICE_RADIO_DARK, TOTAL_AUDIO_SAMPLES, TOTAL_AUDIO_SAMPLES},
		{SFX_POLICE_RADIO_DARK, TOTAL_AUDIO_SAMPLES, TOTAL_AUDIO_SAMPLES},
#endif
		{TOTAL_AUDIO_SAMPLES, SFX_POLICE_RADIO_PURPLE, TOTAL_AUDIO_SAMPLES},
		{TOTAL_AUDIO_SAMPLES, SFX_POLICE_RADIO_PURPLE, TOTAL_AUDIO_SAMPLES},
#ifdef FIX_BUGS
		{TOTAL_AUDIO_SAMPLES, SFX_POLICE_RADIO_PURPLE, SFX_POLICE_RADIO_GREY},
#else
		{TOTAL_AUDIO_SAMPLES, SFX_POLICE_RADIO_PURPLE, TOTAL_AUDIO_SAMPLES},
#endif
		{TOTAL_AUDIO_SAMPLES, SFX_POLICE_RADIO_PURPLE, TOTAL_AUDIO_SAMPLES},
		{TOTAL_AUDIO_SAMPLES, SFX_POLICE_RADIO_PURPLE, TOTAL_AUDIO_SAMPLES},
		{TOTAL_AUDIO_SAMPLES, SFX_POLICE_RADIO_PURPLE, TOTAL_AUDIO_SAMPLES},
#ifdef FIX_BUGS
		{SFX_POLICE_RADIO_LIGHT, SFX_POLICE_RADIO_PURPLE, TOTAL_AUDIO_SAMPLES},
		{SFX_POLICE_RADIO_DARK, SFX_POLICE_RADIO_SILVER, TOTAL_AUDIO_SAMPLES},
		{SFX_POLICE_RADIO_DARK, SFX_POLICE_RADIO_SILVER, TOTAL_AUDIO_SAMPLES},
		{SFX_POLICE_RADIO_DARK, SFX_POLICE_RADIO_SILVER, TOTAL_AUDIO_SAMPLES},
#else
		{SFX_POLICE_RADIO_LIGHT, TOTAL_AUDIO_SAMPLES, TOTAL_AUDIO_SAMPLES},
		{SFX_POLICE_RADIO_DARK, TOTAL_AUDIO_SAMPLES, TOTAL_AUDIO_SAMPLES},
		{SFX_POLICE_RADIO_DARK, TOTAL_AUDIO_SAMPLES, TOTAL_AUDIO_SAMPLES},
		{SFX_POLICE_RADIO_DARK, TOTAL_AUDIO_SAMPLES, TOTAL_AUDIO_SAMPLES},
#endif
		{TOTAL_AUDIO_SAMPLES, SFX_POLICE_RADIO_SILVER, TOTAL_AUDIO_SAMPLES},
		{TOTAL_AUDIO_SAMPLES, SFX_POLICE_RADIO_SILVER, TOTAL_AUDIO_SAMPLES},
		{TOTAL_AUDIO_SAMPLES, SFX_POLICE_RADIO_SILVER, TOTAL_AUDIO_SAMPLES},
		{TOTAL_AUDIO_SAMPLES, SFX_POLICE_RADIO_SILVER, TOTAL_AUDIO_SAMPLES},
		{TOTAL_AUDIO_SAMPLES, SFX_POLICE_RADIO_SILVER, TOTAL_AUDIO_SAMPLES},
#ifdef FIX_BUGS
		{SFX_POLICE_RADIO_LIGHT, SFX_POLICE_RADIO_SILVER, TOTAL_AUDIO_SAMPLES},
#else
		{TOTAL_AUDIO_SAMPLES, SFX_POLICE_RADIO_SILVER, TOTAL_AUDIO_SAMPLES},
#endif
		{SFX_POLICE_RADIO_LIGHT, TOTAL_AUDIO_SAMPLES, TOTAL_AUDIO_SAMPLES},
		{SFX_POLICE_RADIO_LIGHT, TOTAL_AUDIO_SAMPLES, TOTAL_AUDIO_SAMPLES},
		{SFX_POLICE_RADIO_LIGHT, TOTAL_AUDIO_SAMPLES, TOTAL_AUDIO_SAMPLES},
		{SFX_POLICE_RADIO_LIGHT, TOTAL_AUDIO_SAMPLES, TOTAL_AUDIO_SAMPLES},
		{SFX_POLICE_RADIO_LIGHT, TOTAL_AUDIO_SAMPLES, TOTAL_AUDIO_SAMPLES},
		{SFX_POLICE_RADIO_LIGHT, TOTAL_AUDIO_SAMPLES, TOTAL_AUDIO_SAMPLES},
		{SFX_POLICE_RADIO_LIGHT, TOTAL_AUDIO_SAMPLES, TOTAL_AUDIO_SAMPLES},
		{SFX_POLICE_RADIO_LIGHT, TOTAL_AUDIO_SAMPLES, TOTAL_AUDIO_SAMPLES},
		{SFX_POLICE_RADIO_LIGHT, TOTAL_AUDIO_SAMPLES, TOTAL_AUDIO_SAMPLES},
		{SFX_POLICE_RADIO_LIGHT, TOTAL_AUDIO_SAMPLES, TOTAL_AUDIO_SAMPLES},
		{SFX_POLICE_RADIO_LIGHT, TOTAL_AUDIO_SAMPLES, TOTAL_AUDIO_SAMPLES},
		{SFX_POLICE_RADIO_DARK, TOTAL_AUDIO_SAMPLES, TOTAL_AUDIO_SAMPLES},
		{SFX_POLICE_RADIO_DARK, TOTAL_AUDIO_SAMPLES, TOTAL_AUDIO_SAMPLES},
		{SFX_POLICE_RADIO_DARK, TOTAL_AUDIO_SAMPLES, TOTAL_AUDIO_SAMPLES},
		{SFX_POLICE_RADIO_DARK, TOTAL_AUDIO_SAMPLES, TOTAL_AUDIO_SAMPLES},
		{SFX_POLICE_RADIO_DARK, TOTAL_AUDIO_SAMPLES, TOTAL_AUDIO_SAMPLES}
	};

	if (MusicManager.m_nMusicMode != MUSICMODE_CUTSCENE) {
		veh = FindPlayerVehicle();
		if (veh != nil) {
			if (60 - m_sPoliceRadioQueue.policeChannelTimer > 9) {
				color1 = veh->m_currentColour1;
				if (color1 >= ARRAY_SIZE(gCarColourTable)) {
					debug("\n *** UNKNOWN CAR COLOUR %d *** ", color1);
				} else {
					main_color = gCarColourTable[color1][1];
					color_pre_modifier = gCarColourTable[color1][0];
					color_post_modifier = gCarColourTable[color1][2];
					switch (veh->GetModelIndex()) {
// TODO(MIAMI): just making this compile
#ifdef FIX_BUGS
	//				case MI_COLUMB:
	//					main_color = SFX_POLICE_RADIO_BLUE;
	//					color_pre_modifier = color_post_modifier = TOTAL_AUDIO_SAMPLES;
#endif
					case MI_LANDSTAL:
	//				case MI_BLISTA: 
						sample = SFX_POLICE_RADIO_CRUISER; break;
#ifdef FIX_BUGS
	//				case MI_YARDIE:
	//					color_pre_modifier = TOTAL_AUDIO_SAMPLES;
	//					main_color = SFX_POLICE_RADIO_RED;
	//					color_post_modifier = SFX_POLICE_RADIO_YELLOW;
	//					sample = SFX_POLICE_RADIO_CONVERTIBLE; break;
	//				case MI_DIABLOS:
	//					main_color = SFX_POLICE_RADIO_BLACK;
#endif
					case MI_IDAHO:
	//				case MI_STALLION: sample = SFX_POLICE_RADIO_CONVERTIBLE; break;
#ifdef FIX_BUGS
	//				case MI_YAKUZA:
	//					color_pre_modifier = TOTAL_AUDIO_SAMPLES;
	//					main_color = SFX_POLICE_RADIO_SILVER;
	//					color_post_modifier = SFX_POLICE_RADIO_RED;
#endif
					case MI_STINGER:
					case MI_INFERNUS:
					case MI_CHEETAH:
					case MI_BANSHEE: sample = SFX_POLICE_RADIO_SPORTS_CAR; break;
#ifdef FIX_BUGS
	//				case MI_MAFIA:
	//					color_pre_modifier = color_post_modifier = TOTAL_AUDIO_SAMPLES;
	//					main_color = SFX_POLICE_RADIO_GREY;
					case MI_WASHING:
#endif
					case MI_PEREN:
					case MI_SENTINEL:
	//				case MI_FBICAR: sample = SFX_POLICE_RADIO_SALOON; break;
					case MI_PATRIOT:
					case MI_BOBCAT: sample = SFX_POLICE_RADIO_PICKUP; break;
					case MI_FIRETRUCK: sample = SFX_POLICE_RADIO_FIRE_TRUCK; break;
#ifdef FIX_BUGS
					case MI_LINERUN:
					case MI_FLATBED:
#endif
					case MI_TRASH:
					case MI_BARRACKS: sample = SFX_POLICE_RADIO_TRUCK; break;
	//				case MI_STRETCH: sample = SFX_POLICE_RADIO_LIMO; break;
					case MI_MANANA:
					case MI_ESPERANT: sample = SFX_POLICE_RADIO_2_DOOR; break;
#ifdef FIX_BUGS
	//				case MI_HOODS:
	//					color_pre_modifier = TOTAL_AUDIO_SAMPLES;
	//					main_color = SFX_POLICE_RADIO_BLUE;
	//					color_post_modifier = SFX_POLICE_RADIO_GREEN;
	//				case MI_BELLYUP:
					case MI_YANKEE:
					case MI_TOPFUN:
	//				case MI_MRWONGS:
	//				case MI_PANLANT:
#endif
					case MI_PONY:
					case MI_MULE:
					case MI_MOONBEAM:
					case MI_ENFORCER:
					case MI_SECURICA:
					case MI_RUMPO: sample = SFX_POLICE_RADIO_VAN; break;
					case MI_AMBULAN: sample = SFX_POLICE_RADIO_AMBULANCE; break;
					case MI_TAXI:
					case MI_CABBIE:
					case MI_ZEBRA: sample = SFX_POLICE_RADIO_TAXI; break;
					case MI_KAUFMAN: sample = SFX_POLICE_RADIO_TAXI; break;
					case MI_MRWHOOP:
						sample = SFX_POLICE_RADIO_ICE_CREAM_VAN;
						break;
					case MI_BFINJECT: sample = SFX_POLICE_RADIO_BUGGY; break;
					case MI_POLICE: sample = SFX_POLICE_RADIO_POLICE_CAR; break;
#ifdef FIX_BUGS
					case MI_SPEEDER:
					case MI_REEFER:
	//				case MI_GHOST:
#endif
					case MI_PREDATOR: sample = SFX_POLICE_RADIO_BOAT; break;
					case MI_BUS:
					case MI_COACH: sample = SFX_POLICE_RADIO_BUS; break;
					case MI_RHINO:
						sample = SFX_POLICE_RADIO_TANK;
						main_color = TOTAL_AUDIO_SAMPLES;
						color_post_modifier = TOTAL_AUDIO_SAMPLES;
						break;
	//				case MI_TRAIN:
	//					sample = SFX_POLICE_RADIO_SUBWAY_CAR;
	//					main_color = TOTAL_AUDIO_SAMPLES;
	//					color_post_modifier = TOTAL_AUDIO_SAMPLES;

	//					break;
					default:
						debug("\n *** UNKNOWN CAR MODEL INDEX %d *** ", veh->GetModelIndex());
						return;
					}
					m_sPoliceRadioQueue.Add(m_anRandomTable[4] % 3 + SFX_POLICE_RADIO_MESSAGE_NOISE_1);
					m_sPoliceRadioQueue.Add(SFX_POLICE_RADIO_SUSPECT);
					if (m_anRandomTable[3] % 2) 
						m_sPoliceRadioQueue.Add(SFX_POLICE_RADIO_LAST_SEEN);
					m_sPoliceRadioQueue.Add(SFX_POLICE_RADIO_IN_A);
					if (color_pre_modifier != TOTAL_AUDIO_SAMPLES)
						m_sPoliceRadioQueue.Add(color_pre_modifier);
					if (main_color != TOTAL_AUDIO_SAMPLES)
						m_sPoliceRadioQueue.Add(main_color);
					if (color_post_modifier != TOTAL_AUDIO_SAMPLES)
						m_sPoliceRadioQueue.Add(color_post_modifier);
					m_sPoliceRadioQueue.Add(sample);
					m_sPoliceRadioQueue.Add(m_anRandomTable[0] % 3 + SFX_POLICE_RADIO_MESSAGE_NOISE_1);
					m_sPoliceRadioQueue.Add(TOTAL_AUDIO_SAMPLES);
				}
			}
		} else if (60 - m_sPoliceRadioQueue.policeChannelTimer > 4) {
			m_sPoliceRadioQueue.Add(SFX_POLICE_RADIO_MESSAGE_NOISE_1);
			m_sPoliceRadioQueue.Add(SFX_POLICE_RADIO_SUSPECT);
			m_sPoliceRadioQueue.Add(SFX_POLICE_RADIO_ON_FOOT);
			m_sPoliceRadioQueue.Add(m_anRandomTable[0] % 3 + SFX_POLICE_RADIO_MESSAGE_NOISE_1);
			m_sPoliceRadioQueue.Add(TOTAL_AUDIO_SAMPLES);
		}
	}
}



void
cAudioManager::ReportCrime(int32 type, const CVector *pos)
{
	int32 lastCrime = ARRAY_SIZE(m_sPoliceRadioQueue.crimes);
	if (m_bIsInitialised && MusicManager.m_nMusicMode != MUSICMODE_CUTSCENE && FindPlayerPed()->m_pWanted->m_nWantedLevel > 0 &&
		(type > CRIME_NONE || type < NUM_CRIME_TYPES) && m_FrameCounter >= gMinTimeToNextReport[type]) {
		for (int32 i = 0; i < ARRAY_SIZE(m_sPoliceRadioQueue.crimes); i++) {
			if (m_sPoliceRadioQueue.crimes[i].type) {
				if (m_sPoliceRadioQueue.crimes[i].type == type) {
					m_sPoliceRadioQueue.crimes[i].position = *pos;
					m_sPoliceRadioQueue.crimes[i].timer = 0;
					return;
				}
			} else {
				lastCrime = i;
			}
		}

		if (lastCrime < ARRAY_SIZE(m_sPoliceRadioQueue.crimes)) {
			m_sPoliceRadioQueue.crimes[lastCrime].type = type;
			m_sPoliceRadioQueue.crimes[lastCrime].position = *pos;
			m_sPoliceRadioQueue.crimes[lastCrime].timer = 0;
			gMinTimeToNextReport[type] = m_FrameCounter + 500;
		}
	}
}

void
cAudioManager::PlaySuspectLastSeen(float x, float y, float z)
{
	int16 audioZone;
	CZone *zone;
	float rangeX;
	float rangeY;
	float halfX;
	float halfY;
	float quarterX;
	float quarterY;
	int32 sample;
	bool processed = false;
	CVector vec = CVector(x, y, z);

	if (!m_bIsInitialised) return;

	if (MusicManager.m_nMusicMode != MUSICMODE_CUTSCENE && 60 - m_sPoliceRadioQueue.policeChannelTimer > 9) {
		audioZone = CTheZones::FindAudioZone(&vec);
		if (audioZone >= 0 && audioZone < NUMAUDIOZONES) {
			zone = CTheZones::GetAudioZone(audioZone);
			for (int i = 0; i < NUMAUDIOZONES; i++) {
				if (strcmp(zone->name, ZoneSfx[i].m_aName) == 0) {
					sample = ZoneSfx[i].m_nSampleIndex;
					m_sPoliceRadioQueue.Add(m_anRandomTable[4] % 3 + SFX_POLICE_RADIO_MESSAGE_NOISE_1);
					m_sPoliceRadioQueue.Add(SFX_POLICE_RADIO_SUSPECT);
					m_sPoliceRadioQueue.Add(SFX_POLICE_RADIO_LAST_SEEN);
					m_sPoliceRadioQueue.Add(SFX_IN);
					rangeX = zone->maxx - zone->minx;
					rangeY = zone->maxy - zone->miny;
					halfX = 0.5f * rangeX + zone->minx;
					halfY = 0.5f * rangeY + zone->miny;
					quarterX = 0.25f * rangeX;
					quarterY = 0.25f * rangeY;

					if (vec.y > halfY + quarterY) {
						m_sPoliceRadioQueue.Add(SFX_NORTH);
						processed = true;
					} else if (vec.y < halfY - quarterY) {
						m_sPoliceRadioQueue.Add(SFX_SOUTH);
						processed = true;
					}

					if (vec.x > halfX + quarterX)
						m_sPoliceRadioQueue.Add(SFX_EAST);
					else if (vec.x < halfX - quarterX)
						m_sPoliceRadioQueue.Add(SFX_WEST);
					else if (!processed)
						m_sPoliceRadioQueue.Add(SFX_CENTRAL);
					m_sPoliceRadioQueue.Add(sample);
					m_sPoliceRadioQueue.Add(m_anRandomTable[2] % 3 + SFX_POLICE_RADIO_MESSAGE_NOISE_1);
					m_sPoliceRadioQueue.Add(TOTAL_AUDIO_SAMPLES);
					gSpecialSuspectLastSeenReport = true;
					break;
				}
			}
		}
	}
}

void
cAudioManager::AgeCrimes()
{
	for (uint8 i = 0; i < ARRAY_SIZE(m_sPoliceRadioQueue.crimes); i++) {
		if (m_sPoliceRadioQueue.crimes[i].type != CRIME_NONE) {
			if (++m_sPoliceRadioQueue.crimes[i].timer > 1500) m_sPoliceRadioQueue.crimes[i].type = CRIME_NONE;
		}
	}
}
