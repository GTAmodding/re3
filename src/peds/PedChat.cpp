#include "common.h"
#include "Camera.h"
#include "DMAudio.h"
#include "General.h"
#include "Ped.h"

// Corresponds to ped sounds (from SOUND_PED_DEATH to SOUND_PED_TAXI_CALL)
PedAudioData CommentWaitTime[39] = {
	{500, 800, 500, 2},
	{500, 800, 500, 2},
	{500, 800, 500, 2},
	{500, 800, 500, 2},
	{100, 2, 100, 2},
	{700, 500, 1000, 500},
	{700, 500, 1000, 500},
	{5000, 2000, 15000, 3000},
	{5000, 2000, 15000, 3000},
	{5000, 2000, 15000, 3000},
	{6000, 6000, 6000, 6000},
	{1000, 1000, 2000, 2000},
	{1000, 500, 2000, 1500},
	{1000, 500, 2000, 1500},
	{800, 200, 1000, 500},
	{800, 200, 1000, 500},
	{800, 400, 2000, 1000},
	{800, 400, 2000, 1000},
	{400, 300, 2000, 1000},
	{2000, 1000, 2500, 1500},
	{200, 200, 200, 200},
	{6000, 3000, 5000, 6000},
	{6000, 3000, 9000, 5000},
	{6000, 3000, 9000, 5000},
	{6000, 3000, 9000, 5000},
	{400, 300, 4000, 1000},
	{400, 300, 4000, 1000},
	{400, 300, 4000, 1000},
	{1000, 500, 3000, 1000},
	{1000, 500, 1000, 1000},
	{3000, 2000, 3000, 2000},
	{1000, 500, 3000, 6000},
	{1000, 500, 2000, 4000},
	{1000, 500, 2000, 5000},
	{1000, 500, 3000, 2000},
	{1600, 1000, 2000, 2000},
	{3000, 2000, 5000, 3000},
	{1000, 1000, 1000, 1000},
	{1000, 1000, 5000, 5000},
};

bool
CPed::ServiceTalkingWhenDead(void)
{
	return m_queuedSound == SOUND_PED_DEATH;
}

void
CPed::ServiceTalking(void)
{
	if (bBodyPartJustCameOff && m_bodyPartBleeding == PED_HEAD)
		return;

	if (!CGeneral::faststricmp(CModelInfo::GetModelInfo(GetModelIndex())->GetModelName(), "bomber"))
		m_queuedSound = SOUND_PED_BOMBER;
	else if (m_nPedState == PED_ON_FIRE)
		m_queuedSound = SOUND_PED_BURNING;

	if (m_queuedSound != SOUND_NO_SOUND) {
		if (m_queuedSound == SOUND_PED_DEATH)
			m_soundStart = CTimer::GetTimeInMilliseconds() - 1;

		if (CTimer::GetTimeInMilliseconds() > m_soundStart) {
			DMAudio.PlayOneShot(m_audioEntityId, m_queuedSound, 1.0f);
			m_lastSoundStart = CTimer::GetTimeInMilliseconds();
			m_soundStart =
				CommentWaitTime[m_queuedSound - SOUND_PED_DEATH].m_nFixedDelayTime
				+ CTimer::GetTimeInMilliseconds()
				+ CGeneral::GetRandomNumberInRange(0, CommentWaitTime[m_queuedSound - SOUND_PED_DEATH].m_nOverrideFixedDelayTime);
			m_lastQueuedSound = m_queuedSound;
			m_queuedSound = SOUND_NO_SOUND;
		}
	}
}

void
CPed::Say(uint16 audio)
{
	if (IsPlayer()) {

		// Ofc this part isn't in VC.
		switch (audio) {
			case SOUND_PED_DEATH:
				audio = SOUND_PED_DAMAGE;
				break;
			case SOUND_PED_DAMAGE:
			case SOUND_PED_HIT:
			case SOUND_PED_LAND:
				break;
			case SOUND_PED_BULLET_HIT:
			case SOUND_PED_CAR_JACKED:
			case SOUND_PED_DEFEND:
				audio = SOUND_PED_HIT;
				break;
			default:
				return;
		}
	} else {
		if (TheCamera.GetPosition().z + 3.0f < GetPosition().z)
			return;

		if (TheCamera.m_CameraAverageSpeed > 1.65f) {
#ifdef VC_PED_PORTS
			if (audio != SOUND_PED_DAMAGE && audio != SOUND_PED_HIT && audio != SOUND_PED_LAND)
#endif
				return;

		} else if (TheCamera.m_CameraAverageSpeed > 1.25f) {
			if (audio != SOUND_PED_DEATH &&
#ifdef VC_PED_PORTS
				audio != SOUND_PED_DAMAGE && audio != SOUND_PED_HIT && audio != SOUND_PED_LAND &&
#endif
				audio != SOUND_PED_TAXI_WAIT && audio != SOUND_PED_EVADE)
				return;

		} else if (TheCamera.m_CameraAverageSpeed > 0.9f) {
			switch (audio) {
				case SOUND_PED_DEATH:
#ifdef VC_PED_PORTS
				case SOUND_PED_DAMAGE:
				case SOUND_PED_HIT:
				case SOUND_PED_LAND:
#endif
				case SOUND_PED_BURNING:
				case SOUND_PED_FLEE_SPRINT:
				case SOUND_PED_TAXI_WAIT:
				case SOUND_PED_EVADE:
				case SOUND_PED_ANNOYED_DRIVER:
					break;
				default:
					return;
			}
		}
	}

	if (audio < m_queuedSound) {
		if (audio != m_lastQueuedSound || audio == SOUND_PED_DEATH
			|| CommentWaitTime[audio - SOUND_PED_DEATH].m_nOverrideMaxRandomDelayTime
				+ m_lastSoundStart
				+ (uint32) CGeneral::GetRandomNumberInRange(0, CommentWaitTime[audio - SOUND_PED_DEATH].m_nMaxRandomDelayTime) <= CTimer::GetTimeInMilliseconds()) {
			m_queuedSound = audio;
		}
	}
}