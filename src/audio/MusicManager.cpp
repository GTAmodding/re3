#include "common.h"
#include <time.h>
#include "soundlist.h"
#include "MusicManager.h"
#include "AudioManager.h"
#include "ControllerConfig.h"
#include "Camera.h"
#include "Font.h"
#include "Hud.h"
#include "ModelIndices.h"
#include "Replay.h"
#include "Pad.h"
#include "Text.h"
#include "Timer.h"
#include "World.h"
#include "sampman.h"

#if !defined FIX_BUGS && (defined RADIO_SCROLL_TO_PREV_STATION || defined RADIO_OFF_TEXT)
static_assert(false, "RADIO_SCROLL_TO_PREV_STATION and RADIO_OFF_TEXT won't work correctly without FIX_BUGS");
#endif

cMusicManager MusicManager;
int32 gNumRetunePresses;
int32 gRetuneCounter;
bool bHasStarted;

cMusicManager::cMusicManager()
{
	m_bIsInitialised = false;
	m_bDisabled = false;
	m_nMusicMode = MUSICMODE_DISABLED;
	m_nNextTrack = NO_TRACK;
	m_nPlayingTrack = NO_TRACK;
	m_bFrontendTrackFinished = false;
	m_bPlayInFrontend = false;
	m_bSetNextStation = false;
	m_nAnnouncement = NO_TRACK;
	m_bPreviousPlayerInCar = false;
	m_bPlayerInCar = false;
	m_bAnnouncementInProgress = false;
	m_bVerifyAmbienceTrackStartedToPlay = false;
	bHasStarted = false;
}

bool
cMusicManager::PlayerInCar()
{
	if(!FindPlayerVehicle())
		return false;

	int32 State = FindPlayerPed()->m_nPedState;

	if(State == PED_DRAG_FROM_CAR || State == PED_EXIT_CAR || State == PED_ARRESTED)
		return false;

	if (!FindPlayerVehicle())
		return true;

	if (FindPlayerVehicle()->GetStatus() == STATUS_WRECKED)
		return false;

	switch (FindPlayerVehicle()->GetModelIndex()) {
	case MI_FIRETRUCK:
	case MI_AMBULAN:
	case MI_MRWHOOP:
	case MI_PREDATOR:
	case MI_TRAIN:
	case MI_SPEEDER:
	case MI_REEFER:
	case MI_GHOST: return false;
	default: return true;
	}
}

void
cMusicManager::DisplayRadioStationName()
{
	int8 pRetune;
	int8 gStreamedSound;
	int8 gRetuneCounter;
	static wchar *pCurrentStation = nil;
	static uint8 cDisplay = 0;

	if(!CTimer::GetIsPaused() && !TheCamera.m_WideScreenOn && PlayerInCar() &&
	   !CReplay::IsPlayingBack()) {
		if(m_bPlayerInCar && !m_bPreviousPlayerInCar)
			pCurrentStation = nil;

#ifdef FIX_BUGS
		const int curRadio = GetCarTuning();
#else
		const int curRadio = m_nNextTrack;
#endif

#ifdef RADIO_SCROLL_TO_PREV_STATION
		if(gNumRetunePresses < 0) {
			gStreamedSound = curRadio;

			gRetuneCounter = gNumRetunePresses;
			pRetune = gStreamedSound;

			while(gRetuneCounter < 0) {
				if(pRetune == HEAD_RADIO) {
					pRetune = RADIO_OFF;
				} else if(pRetune == RADIO_OFF || pRetune == NUM_RADIOS) {
					pRetune =  SampleManager.IsMP3RadioChannelAvailable() ? USERTRACK : USERTRACK - 1;
				} else
					pRetune--;

				++gRetuneCounter;
			}
		} else
#endif
		if(SampleManager.IsMP3RadioChannelAvailable()) {
			gStreamedSound = curRadio;

			if(gStreamedSound == STREAMED_SOUND_CITY_AMBIENT ||
			   gStreamedSound == STREAMED_SOUND_WATER_AMBIENT) { // which means OFF
				gStreamedSound = NUM_RADIOS;
			} else if(gStreamedSound > STREAMED_SOUND_RADIO_MP3_PLAYER)
				return;

			pRetune = gNumRetunePresses + gStreamedSound;

#ifdef FIX_BUGS
			while(pRetune > NUM_RADIOS)
				pRetune -= (NUM_RADIOS + 1);
#endif
			if(pRetune == NUM_RADIOS) {
				pRetune = RADIO_OFF;
			}
#ifndef FIX_BUGS
			else if(pRetune > NUM_RADIOS) {
				pRetune = pRetune - (NUM_RADIOS + 1);
			}
#endif
		} else {
			gStreamedSound = curRadio;
			pRetune = gNumRetunePresses + gStreamedSound;

			if(pRetune >= USERTRACK) {
				gRetuneCounter = gNumRetunePresses;
				pRetune = curRadio;

				if(gStreamedSound == STREAMED_SOUND_WATER_AMBIENT)
					pRetune = STREAMED_SOUND_CITY_AMBIENT; // which is RADIO_OFF

				while(gRetuneCounter) {
					if(pRetune == RADIO_OFF) {
						pRetune = HEAD_RADIO;
					} else if(pRetune < USERTRACK) {
						pRetune = pRetune + 1;
					}
					if(pRetune == USERTRACK) pRetune = RADIO_OFF;

					--gRetuneCounter;
				}
			}
		}

		wchar *string;

		switch(pRetune) {
		case HEAD_RADIO: string = TheText.Get("FEA_FM0"); break;
		case DOUBLE_CLEF: string = TheText.Get("FEA_FM1"); break;
		case JAH_RADIO: string = TheText.Get("FEA_FM2"); break;
		case RISE_FM: string = TheText.Get("FEA_FM3"); break;
		case LIPS_106: string = TheText.Get("FEA_FM4"); break;
		case GAME_FM: string = TheText.Get("FEA_FM5"); break;
		case MSX_FM: string = TheText.Get("FEA_FM6"); break;
		case FLASHBACK: string = TheText.Get("FEA_FM7"); break;
		case CHATTERBOX: string = TheText.Get("FEA_FM8"); break;
		case USERTRACK:
			if (!SampleManager.IsMP3RadioChannelAvailable())
				return;
			string = TheText.Get("FEA_FM9"); break;
#ifdef RADIO_OFF_TEXT
		case RADIO_OFF: {
			extern wchar WideErrorString[];

			string = TheText.Get("FEA_FMN");
			if(string == WideErrorString) {
				pCurrentStation = nil;
				return;
			}
			break;
		}
#endif
		default: return;
		};

		if(pCurrentStation != string ||
			m_nNextTrack == STREAMED_SOUND_RADIO_MP3_PLAYER && m_nPlayingTrack != STREAMED_SOUND_RADIO_MP3_PLAYER) {
			pCurrentStation = string;
			cDisplay = 60;
		} else {
			if(cDisplay == 0) return;
			cDisplay--;
		}

		CFont::SetJustifyOff();
		CFont::SetBackgroundOff();
		CFont::SetScale(SCREEN_SCALE_X(0.8f), SCREEN_SCALE_Y(1.35f));
		CFont::SetPropOn();
		CFont::SetFontStyle(FONT_HEADING);
		CFont::SetCentreOn();
		// Reminder: Game doesn't have "scaling" at all, it just stretches, and it's team's decision here to not let centered text occupy all the screen.
		//				Disable ASPECT_RATIO_SCALE and it'll go back to default behaviour; stretching.
		CFont::SetCentreSize(SCREEN_SCALE_X(DEFAULT_SCREEN_WIDTH));
		CFont::SetColor(CRGBA(0, 0, 0, 255));
#ifdef FIX_BUGS
		CFont::PrintString(SCREEN_WIDTH / 2 + SCREEN_SCALE_X(2.0f), SCREEN_SCALE_Y(22.0f) + SCREEN_SCALE_Y(2.0f), pCurrentStation);
#else
		CFont::PrintString(SCREEN_WIDTH / 2 + 2.0f, SCREEN_SCALE_Y(22.0f) + 2.0f, pCurrentStation);
#endif

		if(gNumRetunePresses)
			CFont::SetColor(CRGBA(102, 133, 143, 255));
		else
			CFont::SetColor(CRGBA(147, 196, 211, 255));

		CFont::PrintString(SCREEN_WIDTH / 2, SCREEN_SCALE_Y(22.0f), pCurrentStation);
		CFont::DrawFonts();
	}
}

bool
cMusicManager::Initialise()
{
	int pos;

	if (!IsInitialised()) {
		time_t timevalue = time(0);
		if (timevalue == -1) {
			pos = AudioManager.GetRandomNumber(0);
		} else {
			tm *pTm = localtime(&timevalue);
			if (pTm->tm_sec == 0)
				pTm->tm_sec = AudioManager.GetRandomNumber(0);
			if (pTm->tm_min == 0)
				pTm->tm_min = AudioManager.GetRandomNumber(1);
			if (pTm->tm_hour == 0)
				pTm->tm_hour = AudioManager.GetRandomNumber(2);
			if (pTm->tm_mday == 0)
				pTm->tm_mday = AudioManager.GetRandomNumber(3);
			if (pTm->tm_mon == 0)
				pTm->tm_mon = AudioManager.GetRandomNumber(4);
			if (pTm->tm_year == 0)
				pTm->tm_year = AudioManager.GetRandomNumber(3);
			if (pTm->tm_wday == 0)
				pTm->tm_wday = AudioManager.GetRandomNumber(2);
			pos = pTm->tm_yday
				* pTm->tm_wday
				* pTm->tm_year
				* pTm->tm_mon
				* pTm->tm_mday
				* pTm->tm_hour * pTm->tm_hour
				* pTm->tm_min * pTm->tm_min
				* pTm->tm_sec * pTm->tm_sec * pTm->tm_sec * pTm->tm_sec;
		}

		for (int i = 0; i < TOTAL_STREAMED_SOUNDS; i++) {
			m_aTracks[i].m_nLength = SampleManager.GetStreamedFileLength(i);
			m_aTracks[i].m_nPosition = pos * AudioManager.GetRandomNumber(i % 5) % m_aTracks[i].m_nLength;
			m_aTracks[i].m_nLastPosCheckTimer = CTimer::GetTimeInMillisecondsPauseMode();
		}

		m_bResetTimers = false;
		m_nResetTime = 0;
		m_nTimer = m_nLastTrackServiceTime = CTimer::GetTimeInMillisecondsPauseMode();
		m_bDoTrackService = false;
		m_bIgnoreTimeDelay = false;
		m_bRadioSetByScript = false;
		m_nRadioStationScript = HEAD_RADIO;
		m_nRadioPosition = -1;
		m_nRadioInCar = NO_TRACK;
		gNumRetunePresses = 0;
		gRetuneCounter = 0;
		m_bIsInitialised = true;
	}
	return m_bIsInitialised;
}

void
cMusicManager::Terminate()
{
	if (!IsInitialised()) return;

	if (SampleManager.IsStreamPlaying(0)) {
		SampleManager.StopStreamedFile(0);
		m_nNextTrack = NO_TRACK;
		m_nPlayingTrack = NO_TRACK;
	}
	m_bIsInitialised = false;
}

void
cMusicManager::ChangeMusicMode(uint8 mode)
{
	if (!IsInitialised()) return;

	uint8 mode2;
	switch (mode)
	{
	case MUSICMODE_FRONTEND: mode2 = MUSICMODE_FRONTEND; break;
	case MUSICMODE_GAME: mode2 = MUSICMODE_GAME; break;
	case MUSICMODE_CUTSCENE: mode2 = MUSICMODE_CUTSCENE; break;
	case MUSICMODE_DISABLE: mode2 = MUSICMODE_DISABLED; break;
	default: return;
	}

	if (mode2 != m_nMusicMode || mode == MUSICMODE_FRONTEND && mode2 == MUSICMODE_FRONTEND) {
		switch (mode)
		{
		case MUSICMODE_FRONTEND:
		case MUSICMODE_GAME:
		case MUSICMODE_CUTSCENE:
		case MUSICMODE_DISABLED:
			if (SampleManager.IsStreamPlaying(0)) {
				if (m_nNextTrack < TOTAL_STREAMED_SOUNDS) {
					m_aTracks[m_nNextTrack].m_nPosition = SampleManager.GetStreamedFilePosition(0);
					m_aTracks[m_nNextTrack].m_nLastPosCheckTimer = CTimer::GetTimeInMillisecondsPauseMode();
				}
				SampleManager.StopStreamedFile(0);
			}
			m_nNextTrack = NO_TRACK;
			m_nPlayingTrack = NO_TRACK;
			m_bFrontendTrackFinished = false;
			m_bPlayInFrontend = false;
			m_bSetNextStation = false;
			m_bPreviousPlayerInCar = false;
			m_bPlayerInCar = false;
			m_bAnnouncementInProgress = false;
			m_nTimer = m_nLastTrackServiceTime = CTimer::GetTimeInMillisecondsPauseMode();
			m_bDoTrackService = false;
			m_bIgnoreTimeDelay = true;
			m_bVerifyAmbienceTrackStartedToPlay = false;
			m_nMusicMode = mode2;
			break;
		default: return;
		}
	}
}

uint8
cMusicManager::GetRadioInCar(void)
{
	if (!m_bIsInitialised) return HEAD_RADIO;
	if (PlayerInCar()) {
		CVehicle *veh = FindPlayerVehicle();
		if (veh != nil){
			if (UsesPoliceRadio(veh)) {
				if (m_nRadioInCar == NO_TRACK || (CReplay::IsPlayingBack() && AudioManager.m_nUserPause == 0))
					return POLICE_RADIO;
				return m_nRadioInCar;
			} else return veh->m_nRadioStation;
		}
	}

	if (m_nRadioInCar == NO_TRACK || (CReplay::IsPlayingBack() && AudioManager.m_nUserPause == 0))
		return RADIO_OFF;
	return m_nRadioInCar;
}

void
cMusicManager::SetRadioInCar(uint32 station)
{
	if (m_bIsInitialised) {
		if (!PlayerInCar()) {
			m_nRadioInCar = station;
			return;
		}
		CVehicle *veh = FindPlayerVehicle();
		if (veh == nil) return;
		if (UsesPoliceRadio(veh))
			m_nRadioInCar = station;
		else
			veh->m_nRadioStation = station;
	}
}

void
cMusicManager::SetRadioChannelByScript(uint8 station, int32 pos)
{
	if (m_bIsInitialised && station < RADIO_OFF) {
		m_bRadioSetByScript = true;
		m_nRadioStationScript = station;
		m_nRadioPosition = pos == -1 ? -1 : pos % m_aTracks[station].m_nLength;
	}
}


void
cMusicManager::ResetMusicAfterReload()
{
	m_bRadioSetByScript = false;
	m_nRadioStationScript = 0;
	m_nRadioPosition = -1;
	m_nAnnouncement = NO_TRACK;
	m_bAnnouncementInProgress = false;
	m_bSetNextStation = false;
	gRetuneCounter = 0;
	gNumRetunePresses = 0;
}


void
cMusicManager::ResetTimers(int32 time)
{
	m_bResetTimers = true;
	m_nResetTime = time;
}

void
cMusicManager::Service()
{
	if (m_bResetTimers) {
		m_bResetTimers = false;
		m_nLastTrackServiceTime = m_nResetTime;
	}

	if (!m_bIsInitialised || m_bDisabled) return;

	if (m_nMusicMode == MUSICMODE_CUTSCENE) {
		SampleManager.SetStreamedVolumeAndPan(MAX_VOLUME, 63, 1, 0);
		return;
	}

	m_nTimer = CTimer::GetTimeInMillisecondsPauseMode();
	if (m_nTimer > (m_nLastTrackServiceTime + 2000) || m_bIgnoreTimeDelay) {
		m_bIgnoreTimeDelay = false;
		m_bDoTrackService = true;
		m_nLastTrackServiceTime = m_nTimer;
	} else m_bDoTrackService = false;

	if (m_nNextTrack == NO_TRACK && SampleManager.IsStreamPlaying(0))
		SampleManager.StopStreamedFile(0);
	else switch (m_nMusicMode) {
		case MUSICMODE_FRONTEND: ServiceFrontEndMode(); break;
		case MUSICMODE_GAME: ServiceGameMode(); break;
		}
}

void
cMusicManager::ServiceFrontEndMode()
{
	if (m_nNextTrack < TOTAL_STREAMED_SOUNDS) {
		if (m_bFrontendTrackFinished) {
			if (!SampleManager.IsStreamPlaying(0)) {
				switch (m_nNextTrack)
				{
				case STREAMED_SOUND_MISSION_COMPLETED:
					if (!AudioManager.m_nUserPause)
						ChangeMusicMode(MUSICMODE_GAME);
					break;
				case STREAMED_SOUND_GAME_COMPLETED:
					ChangeMusicMode(MUSICMODE_GAME);
					break;
				default:
					break;
				}
				m_nNextTrack = NO_TRACK;
				m_nPlayingTrack = NO_TRACK;
			}
		} else if (bHasStarted) {
			if (!SampleManager.IsStreamPlaying(0))
				SampleManager.StartStreamedFile(m_nNextTrack, 0, 0);
		} else {
			SampleManager.SetStreamedVolumeAndPan(0, 63, 0, 0);
			if (!SampleManager.StartStreamedFile(m_nNextTrack, m_nNextTrack < NUM_RADIOS ? GetTrackStartPos(m_nNextTrack) : 0, 0))
				return;
			SampleManager.SetStreamedVolumeAndPan(100, 63, 0, 0);
			if (m_bPlayInFrontend) bHasStarted = true;
			else m_bFrontendTrackFinished = true;
		}
	}
	if (SampleManager.IsStreamPlaying(0))
		SampleManager.SetStreamedVolumeAndPan((CPad::GetPad(0)->bDisplayNoControllerMessage || CPad::GetPad(0)->bObsoleteControllerMessage) ? 0 : 100, 63, 0, 0);
}

void
cMusicManager::ServiceGameMode()
{
	bool bRadioOff = false;
	static int8 nFramesSinceCutsceneEnded = -1;
	uint8 volume;

	m_bPreviousPlayerInCar = m_bPlayerInCar;
	m_bPlayerInCar = PlayerInCar();
	m_nPlayingTrack = m_nNextTrack;
	if (m_bPlayerInCar) {
		if (FindPlayerPed() != nil
			&& !FindPlayerPed()->DyingOrDead()
			&& !CReplay::IsPlayingBack()
			&& FindPlayerVehicle() != nil
			&& !UsesPoliceRadio(FindPlayerVehicle())) {

				if (CPad::GetPad(0)->ChangeStationJustDown()) {
					gRetuneCounter = 30;
					gNumRetunePresses++;
					AudioManager.PlayOneShot(AudioManager.m_nFrontEndEntity, SOUND_FRONTEND_RADIO_CHANGE, 1.0f);
					// This needs loop, and this is not the right place. Now done elsewhere.
#ifndef FIX_BUGS
					if (SampleManager.IsMP3RadioChannelAvailable()) {
						if (gNumRetunePresses > RADIO_OFF)
							gNumRetunePresses -= RADIO_OFF;
					}
#endif
				}
#ifdef RADIO_SCROLL_TO_PREV_STATION
				else if(CPad::GetPad(0)->GetMouseWheelDownJustDown() || CPad::GetPad(0)->GetMouseWheelUpJustDown()) {
					int scrollNext = ControlsManager.GetControllerKeyAssociatedWithAction(VEHICLE_CHANGE_RADIO_STATION, MOUSE);
					int scrollPrev = scrollNext == rsMOUSEWHEELUPBUTTON ? rsMOUSEWHEELDOWNBUTTON : scrollNext == rsMOUSEWHEELDOWNBUTTON ? rsMOUSEWHEELUPBUTTON : -1;

					if (scrollPrev != -1 && !ControlsManager.IsAnyVehicleActionAssignedToMouseKey(scrollPrev)) {
						gRetuneCounter = 30;
						gNumRetunePresses--;
						AudioManager.PlayOneShot(AudioManager.m_nFrontEndEntity, SOUND_FRONTEND_RADIO_CHANGE, 1.0f);
					}
				}
#endif
		}
	} else {
		nFramesSinceCutsceneEnded = -1;
	}

	if (AudioManager.m_nPreviousUserPause)
		m_bPreviousPlayerInCar = false;
	if (!m_bPlayerInCar) {
		if (m_bPreviousPlayerInCar) {
			if (m_nNextTrack != STREAMED_SOUND_RADIO_POLICE)
				m_nRadioInCar = m_nNextTrack;
		}
		ServiceAmbience();
		return;
	}

	if (m_bPreviousPlayerInCar) {
		if (m_nAnnouncement < TOTAL_STREAMED_SOUNDS
			&& (m_nNextTrack < RADIO_OFF || m_bAnnouncementInProgress)
			&& ServiceAnnouncement())
		{
			if (m_bAnnouncementInProgress) {
				m_bSetNextStation = false;
				return;
			}
			m_nPlayingTrack = m_nNextTrack;
			m_nNextTrack = GetCarTuning();
		}
		if (SampleManager.IsMP3RadioChannelAvailable()
			&& m_nNextTrack != STREAMED_SOUND_RADIO_MP3_PLAYER
			&& ControlsManager.GetIsKeyboardKeyJustDown(rsF9))
		{
			m_nPlayingTrack = m_nNextTrack;
			m_nNextTrack = STREAMED_SOUND_RADIO_MP3_PLAYER;
			if (FindPlayerVehicle() != nil)
				FindPlayerVehicle()->m_nRadioStation = STREAMED_SOUND_RADIO_MP3_PLAYER;
			AudioManager.PlayOneShot(AudioManager.m_nFrontEndEntity, SOUND_FRONTEND_RADIO_CHANGE, 1.0f);
			gRetuneCounter = 0;
			gNumRetunePresses = 0;
			m_bSetNextStation = false;
		}
		// Because when you switch radio back and forth, gNumRetunePresses will be 0 but gRetuneCounter won't.
#ifdef RADIO_SCROLL_TO_PREV_STATION
		if (gRetuneCounter != 0) {
			if (gRetuneCounter > 1) gRetuneCounter--;
			else if (gRetuneCounter == 1) gRetuneCounter = -1;
			else if (gRetuneCounter == -1) {
				m_bSetNextStation = true;
				gRetuneCounter = 0;
			}
		}
#else
		if (gNumRetunePresses) {
			if (gRetuneCounter != 0) gRetuneCounter--;
			else m_bSetNextStation = true;
		}
#endif
		if (gRetuneCounter)
			AudioManager.DoPoliceRadioCrackle();
		if (m_bSetNextStation) {
			m_bSetNextStation = false;
			m_nPlayingTrack = m_nNextTrack;
			m_nNextTrack = GetNextCarTuning();
			if (m_nNextTrack == STREAMED_SOUND_CITY_AMBIENT || m_nNextTrack == STREAMED_SOUND_WATER_AMBIENT)
				bRadioOff = true;

			if (m_nPlayingTrack == STREAMED_SOUND_CITY_AMBIENT || m_nPlayingTrack == STREAMED_SOUND_WATER_AMBIENT)
				AudioManager.PlayOneShot(AudioManager.m_nFrontEndEntity, SOUND_FRONTEND_RADIO_CHANGE, 0.0f);
		}
		if (m_nNextTrack < RADIO_OFF) {
			if (ChangeRadioChannel()) {
				ServiceTrack();
			} else {
				m_bPlayerInCar = false;
				if (FindPlayerVehicle())
					FindPlayerVehicle()->m_nRadioStation = m_nNextTrack;
				m_nNextTrack = NO_TRACK;
			}
			if (CTimer::GetIsSlowMotionActive()) {
				if (TheCamera.pTargetEntity != nil) {
					float DistToTargetSq = (TheCamera.pTargetEntity->GetPosition() - TheCamera.GetPosition()).MagnitudeSqr();
					if (DistToTargetSq >= SQR(55.0f)) {
						SampleManager.SetStreamedVolumeAndPan(0, 63, 0, 0);
					} else if (DistToTargetSq >= SQR(10.0f)) {
						volume = ((45.0f - (Sqrt(DistToTargetSq) - 10.0f)) / 45.0f * 100.0f);
						uint8 pan;
						if (AudioManager.ShouldDuckMissionAudio())
							volume /= 4;
						if (volume > 0) {
							CVector panVec;
							AudioManager.TranslateEntity(&TheCamera.pTargetEntity->GetPosition(), &panVec);
							pan = AudioManager.ComputePan(55.0f, &panVec);
						} else {
							pan = 0;
						}
						if (gRetuneCounter)
							volume /= 4;
						SampleManager.SetStreamedVolumeAndPan(volume, pan, 0, 0);
					} else if (AudioManager.ShouldDuckMissionAudio()) {
						SampleManager.SetStreamedVolumeAndPan(25, 63, 0, 0);
					} else if (gRetuneCounter) {
						SampleManager.SetStreamedVolumeAndPan(25, 63, 0, 0);
					} else {
						SampleManager.SetStreamedVolumeAndPan(100, 63, 0, 0);
					}
				}
			} else if (AudioManager.ShouldDuckMissionAudio()) {
				SampleManager.SetStreamedVolumeAndPan(25, 63, 0, 0);
				nFramesSinceCutsceneEnded = 0;
			} else {
				if (nFramesSinceCutsceneEnded == -1) {
					volume = 100;
				} else if (nFramesSinceCutsceneEnded < 20) {
					nFramesSinceCutsceneEnded++;
					volume = 25;
				} else if (nFramesSinceCutsceneEnded < 40) {
					volume = 3 * (nFramesSinceCutsceneEnded - 20) + 25;
					nFramesSinceCutsceneEnded++;
				} else {
					nFramesSinceCutsceneEnded = -1;
					volume = 100;
				}
				if (gRetuneCounter != 0)
					volume /= 4;
				SampleManager.SetStreamedVolumeAndPan(volume, 63, 0, 0);
			}
			return;
		}
		if (bRadioOff) {
			m_nNextTrack = m_nPlayingTrack;
			if (FindPlayerVehicle() != nil)
				FindPlayerVehicle()->m_nRadioStation = RADIO_OFF;
			AudioManager.PlayOneShot(AudioManager.m_nFrontEndEntity, SOUND_FRONTEND_RADIO_TURN_OFF, 0.0f);
		}
		ServiceAmbience();
		return;
	}
	if (m_bRadioSetByScript) {
		if (UsesPoliceRadio(FindPlayerVehicle())) {
			m_nNextTrack = STREAMED_SOUND_RADIO_POLICE;
		} else {
			m_nNextTrack = m_nRadioStationScript;
			if (FindPlayerVehicle()->m_nRadioStation == m_nNextTrack) {
				m_nPlayingTrack = NO_TRACK;
				SampleManager.SetStreamedVolumeAndPan(0, 63, 0, 0);
				SampleManager.StopStreamedFile(0);
			}
			if (m_nRadioPosition != -1) {
				m_aTracks[m_nNextTrack].m_nPosition = m_nRadioPosition;
				m_aTracks[m_nNextTrack].m_nLastPosCheckTimer = CTimer::GetTimeInMillisecondsPauseMode();
			}
		}
	} else {
		m_nNextTrack = GetCarTuning();
	}
	if (m_nNextTrack >= RADIO_OFF) {
		ServiceAmbience();
		return;
	}
	if (ChangeRadioChannel()) {
		if (m_bRadioSetByScript) {
			m_bRadioSetByScript = false;
			FindPlayerVehicle()->m_nRadioStation = m_nNextTrack;
		}
	} else {
		m_bPlayerInCar = false;
		m_nNextTrack = NO_TRACK;
	}
}

void
cMusicManager::StopFrontEndTrack()
{
	if (IsInitialised() && !m_bDisabled && m_nMusicMode == MUSICMODE_FRONTEND && m_nNextTrack != NO_TRACK) {
		m_aTracks[m_nNextTrack].m_nPosition = SampleManager.GetStreamedFilePosition(0);
		m_aTracks[m_nNextTrack].m_nLastPosCheckTimer = CTimer::GetTimeInMillisecondsPauseMode();
		SampleManager.StopStreamedFile(0);
		m_nPlayingTrack = NO_TRACK;
		m_nNextTrack = NO_TRACK;
	}
}

void
cMusicManager::PlayAnnouncement(uint8 announcement)
{
	if (IsInitialised() && !m_bDisabled && !m_bAnnouncementInProgress)
		m_nAnnouncement = announcement;
}

void
cMusicManager::PlayFrontEndTrack(uint8 track, uint8 bPlayInFrontend)
{
	if (IsInitialised() && !m_bDisabled && track < TOTAL_STREAMED_SOUNDS) {
		if (m_nMusicMode == MUSICMODE_GAME) {
			if (m_nNextTrack != NO_TRACK) {
				if (m_bAnnouncementInProgress) {
					m_nAnnouncement = NO_TRACK;
					m_bAnnouncementInProgress = false;
				}
				m_aTracks[m_nNextTrack].m_nPosition = SampleManager.GetStreamedFilePosition(0);
				m_aTracks[m_nNextTrack].m_nLastPosCheckTimer = CTimer::GetTimeInMillisecondsPauseMode();
			}
			SampleManager.StopStreamedFile(0);
		} else if (m_nMusicMode == MUSICMODE_FRONTEND) {
			if (m_nNextTrack != NO_TRACK) {
				m_aTracks[m_nNextTrack].m_nPosition = SampleManager.GetStreamedFilePosition(0);
				m_aTracks[m_nNextTrack].m_nLastPosCheckTimer = CTimer::GetTimeInMillisecondsPauseMode();
			}
			SampleManager.StopStreamedFile(0);
		}

		m_nPlayingTrack = m_nNextTrack;
		m_nNextTrack = track;
		m_bPlayInFrontend = !!bPlayInFrontend;
		m_bFrontendTrackFinished = false;
		m_bDoTrackService = true;
		bHasStarted = false;
		if (m_nNextTrack < NUM_RADIOS) {
			gRetuneCounter = 0;
			gNumRetunePresses = 0;
		}
	}
}

void
cMusicManager::PreloadCutSceneMusic(uint8 track)
{
	if (IsInitialised() && !m_bDisabled && track < TOTAL_STREAMED_SOUNDS && m_nMusicMode == MUSICMODE_CUTSCENE) {
		AudioManager.ResetPoliceRadio();
		while (SampleManager.IsStreamPlaying(0))
			SampleManager.StopStreamedFile(0);
		SampleManager.PreloadStreamedFile(track, 0);
		SampleManager.SetStreamedVolumeAndPan(MAX_VOLUME, 63, 1, 0);
		m_nNextTrack = track;
	}
}

void
cMusicManager::PlayPreloadedCutSceneMusic(void)
{
	if (IsInitialised() && !m_bDisabled && m_nMusicMode == MUSICMODE_CUTSCENE)
		SampleManager.StartPreloadedStreamedFile(0);
}

void
cMusicManager::StopCutSceneMusic(void)
{
	if (IsInitialised() && !m_bDisabled && m_nMusicMode == MUSICMODE_CUTSCENE) {
		SampleManager.StopStreamedFile(0);
		m_nNextTrack = NO_TRACK;
	}
}

uint32
cMusicManager::GetTrackStartPos(uint8 track)
{
	uint32 pos = m_aTracks[track].m_nPosition;
	if (CTimer::GetTimeInMillisecondsPauseMode() > m_aTracks[track].m_nLastPosCheckTimer)
		pos += Min(CTimer::GetTimeInMillisecondsPauseMode() - m_aTracks[track].m_nLastPosCheckTimer, 90000);
	else
		m_aTracks[track].m_nLastPosCheckTimer = CTimer::GetTimeInMillisecondsPauseMode();

	if (pos > m_aTracks[track].m_nLength)
		pos %= m_aTracks[track].m_nLength;
	return pos;
}


bool
cMusicManager::UsesPoliceRadio(CVehicle *veh)
{
	switch (veh->GetModelIndex())
	{
	case MI_FBICAR:
	case MI_POLICE:
	case MI_ENFORCER:
	case MI_PREDATOR:
	case MI_RHINO:
	case MI_BARRACKS:
		return true;
	}
	return false;
}

void
cMusicManager::ServiceAmbience()
{
	uint8 volume;

	if (m_bAnnouncementInProgress) {
		m_nAnnouncement = NO_TRACK;
		m_bAnnouncementInProgress = false;
	}
	if (m_nNextTrack < RADIO_OFF) {
		if (SampleManager.IsStreamPlaying(0)) {
			m_aTracks[m_nNextTrack].m_nPosition = SampleManager.GetStreamedFilePosition(0);
			m_aTracks[m_nNextTrack].m_nLastPosCheckTimer = CTimer::GetTimeInMillisecondsPauseMode();
			SampleManager.StopStreamedFile(0);
			m_nNextTrack = NO_TRACK;
			return;
		}
		m_nNextTrack = RADIO_OFF;
	}
	if (CWorld::Players[CWorld::PlayerInFocus].m_WBState != WBSTATE_PLAYING && !SampleManager.IsStreamPlaying(0)) {
		m_nNextTrack = NO_TRACK;
		return;
	}

	m_nPlayingTrack = m_nNextTrack;
	m_nNextTrack = TheCamera.DistanceToWater <= 45.0f ? STREAMED_SOUND_WATER_AMBIENT : STREAMED_SOUND_CITY_AMBIENT;

	if (m_nNextTrack == m_nPlayingTrack) {
		ComputeAmbienceVol(false, volume);
		SampleManager.SetStreamedVolumeAndPan(volume, 63, 1, 0);
		if (m_bVerifyAmbienceTrackStartedToPlay) {
			if (SampleManager.IsStreamPlaying(0))
				m_bVerifyAmbienceTrackStartedToPlay = false;
		} else ServiceTrack();
	} else {
		if (m_nPlayingTrack < TOTAL_STREAMED_SOUNDS) {
			m_aTracks[m_nPlayingTrack].m_nPosition = SampleManager.GetStreamedFilePosition(0);
			m_aTracks[m_nPlayingTrack].m_nLastPosCheckTimer = CTimer::GetTimeInMillisecondsPauseMode();
			SampleManager.StopStreamedFile(0);
		}
		uint32 pos = GetTrackStartPos(m_nNextTrack);
		SampleManager.SetStreamedVolumeAndPan(0, 63, 1, 0);
		if (SampleManager.StartStreamedFile(m_nNextTrack, pos, 0)) {
			ComputeAmbienceVol(true, volume);
			SampleManager.SetStreamedVolumeAndPan(volume, 63, 1, 0);
			m_bVerifyAmbienceTrackStartedToPlay = true;
		} else
			m_nNextTrack = NO_TRACK;
	}
}

void
cMusicManager::ComputeAmbienceVol(uint8 reset, uint8 &outVolume)
{
	static float fVol = 0.0f;

	if (reset)
		fVol = 0.0f;
	else if (fVol < 60.0f)
		fVol += 1.0f;

	if (TheCamera.DistanceToWater > 70.0f)
		outVolume = fVol;
	else if (TheCamera.DistanceToWater > 45.0f)
		outVolume = (TheCamera.DistanceToWater - 45.0f) / 25.0f * fVol;
	else if (TheCamera.DistanceToWater > 20.0f)
		outVolume = (45.0f - TheCamera.DistanceToWater) / 25.0f * fVol;
	else
		outVolume = fVol;
}

void
cMusicManager::ServiceTrack()
{
	if (m_bDoTrackService) {
		if (!SampleManager.IsStreamPlaying(0))
			SampleManager.StartStreamedFile(m_nNextTrack, 0, 0);
	}
}

bool
cMusicManager::ServiceAnnouncement()
{
	static int8 cCheck = 0;
	if (m_bAnnouncementInProgress) {
		if (!SampleManager.IsStreamPlaying(0)) {
			m_nAnnouncement = NO_TRACK;
			m_bAnnouncementInProgress = false;
		}
		return true;
	}

	if (++cCheck >= 30) {
		cCheck = 0;
		int pos = SampleManager.GetStreamedFilePosition(0);
		if (SampleManager.IsStreamPlaying(0)) {
			if (m_nNextTrack != NO_TRACK) {
				m_aTracks[m_nNextTrack].m_nPosition = pos;
				m_aTracks[m_nNextTrack].m_nLastPosCheckTimer = CTimer::GetTimeInMillisecondsPauseMode();
				SampleManager.StopStreamedFile(0);
			}
		}

		SampleManager.SetStreamedVolumeAndPan(0, 63, 0, 0);
		if (SampleManager.StartStreamedFile(m_nAnnouncement, 0, 0)) {
			SampleManager.SetStreamedVolumeAndPan(AudioManager.ShouldDuckMissionAudio() ? 25 : 100, 63, 0, 0);
			m_bAnnouncementInProgress = true;
			m_nPlayingTrack = m_nNextTrack;
			m_nNextTrack = m_nAnnouncement;
			return true;
		}

		if (cCheck != 0) cCheck--;
		else cCheck = 30;
		return false;
	}

	return false;
}

uint8
cMusicManager::GetCarTuning()
{
	CVehicle *veh = FindPlayerVehicle();
	if (veh == nil) return RADIO_OFF;
	if (UsesPoliceRadio(veh)) return POLICE_RADIO;
	if (veh->m_nRadioStation == USERTRACK && !SampleManager.IsMP3RadioChannelAvailable())
		veh->m_nRadioStation = AudioManager.GetRandomNumber(2) % USERTRACK;
	return veh->m_nRadioStation;
}

uint8
cMusicManager::GetNextCarTuning()
{
	CVehicle *veh = FindPlayerVehicle();
	if (veh == nil) return RADIO_OFF;
	if (UsesPoliceRadio(veh)) return POLICE_RADIO;
	if (gNumRetunePresses != 0) {
#ifdef RADIO_SCROLL_TO_PREV_STATION
		if (gNumRetunePresses < 0) {
			while (gNumRetunePresses < 0) {
				if(veh->m_nRadioStation == HEAD_RADIO) {
					veh->m_nRadioStation = RADIO_OFF;
				} else if(veh->m_nRadioStation == RADIO_OFF || veh->m_nRadioStation == NUM_RADIOS) {
					veh->m_nRadioStation = SampleManager.IsMP3RadioChannelAvailable() ? USERTRACK : USERTRACK - 1;
				} else
					veh->m_nRadioStation--;

				++gNumRetunePresses;
			}
		} else
#endif
		if (SampleManager.IsMP3RadioChannelAvailable()) {
			if (veh->m_nRadioStation == RADIO_OFF)
				veh->m_nRadioStation = NUM_RADIOS;
			veh->m_nRadioStation += gNumRetunePresses;
#ifdef FIX_BUGS
			while (veh->m_nRadioStation > NUM_RADIOS)
				veh->m_nRadioStation -= (NUM_RADIOS + 1);
#endif
			if (veh->m_nRadioStation == NUM_RADIOS)
				veh->m_nRadioStation = RADIO_OFF;
#ifndef FIX_BUGS
			else if (veh->m_nRadioStation > NUM_RADIOS)
				veh->m_nRadioStation -= (NUM_RADIOS + 1);
#endif
		} else if (gNumRetunePresses + veh->m_nRadioStation >= USERTRACK) {
			while (gNumRetunePresses) {
				if (veh->m_nRadioStation == RADIO_OFF)
					veh->m_nRadioStation = HEAD_RADIO;
				else if (veh->m_nRadioStation < USERTRACK)
					++veh->m_nRadioStation;

				if (veh->m_nRadioStation == USERTRACK)
					veh->m_nRadioStation = RADIO_OFF;
				--gNumRetunePresses;
			}
		} else
			veh->m_nRadioStation += gNumRetunePresses;
		gNumRetunePresses = 0;
	}
	return veh->m_nRadioStation;
}

bool
cMusicManager::ChangeRadioChannel()
{
	if (m_nNextTrack != m_nPlayingTrack) {
		if (m_nPlayingTrack < TOTAL_STREAMED_SOUNDS) {
			m_aTracks[m_nPlayingTrack].m_nPosition = SampleManager.GetStreamedFilePosition(0);
			m_aTracks[m_nPlayingTrack].m_nLastPosCheckTimer = CTimer::GetTimeInMillisecondsPauseMode();
			SampleManager.SetStreamedVolumeAndPan(0, 63, 0, 0);
			SampleManager.StopStreamedFile(0);
		}
		if (SampleManager.IsStreamPlaying(0))
			return false;
		if (!SampleManager.StartStreamedFile(m_nNextTrack, GetTrackStartPos(m_nNextTrack), 0))
			return false;
		SampleManager.SetStreamedVolumeAndPan(AudioManager.ShouldDuckMissionAudio() ? 25 : 100, 63, 0, 0);
	}
	return true;
}
