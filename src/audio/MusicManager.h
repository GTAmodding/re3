#pragma once

#include "audio_enums.h"

class tStreamedSample
{
public:
	uint32 m_nLength;
	uint32 m_nPosition;
	uint32 m_nLastPosCheckTimer;
};

class CVehicle;

class cMusicManager
{
public:
	bool m_bIsInitialised;
	bool m_bDisabled;
	uint8 m_nMusicMode;
	uint8 m_nNextTrack;
	uint8 m_nPlayingTrack;
	bool m_bFrontendTrackFinished;
	bool m_bPlayInFrontend;
	bool m_bSetNextStation;
	uint8 m_nAnnouncement;
	bool m_bPreviousPlayerInCar;
	bool m_bPlayerInCar;
	bool m_bAnnouncementInProgress;
	tStreamedSample m_aTracks[TOTAL_STREAMED_SOUNDS];
	bool m_bResetTimers;
	uint32 m_nResetTime;
	uint32 m_nLastTrackServiceTime;
	uint32 m_nTimer;
	bool m_bDoTrackService;
	bool m_bIgnoreTimeDelay;
	bool m_bVerifyAmbienceTrackStartedToPlay;
	bool m_bRadioSetByScript;
	uint8 m_nRadioStationScript;
	int32 m_nRadioPosition;
	uint8 m_nRadioInCar;

public:
	cMusicManager();
	bool IsInitialised() { return m_bIsInitialised; }
	uint32 GetMusicMode() { return m_nMusicMode; }
	uint8 GetNextTrack() { return m_nNextTrack; }

	bool Initialise();
	void Terminate();

	void ChangeMusicMode(uint8 mode);
	void StopFrontEndTrack();

	bool PlayerInCar();
	void DisplayRadioStationName();

	void PlayAnnouncement(uint8);
	void PlayFrontEndTrack(uint8, uint8);
	void PreloadCutSceneMusic(uint8);
	void PlayPreloadedCutSceneMusic(void);
	void StopCutSceneMusic(void);
	uint8 GetRadioInCar(void);
	void SetRadioInCar(uint32);
	void SetRadioChannelByScript(uint8, int32);

	void ResetMusicAfterReload();

	void ResetTimers(int32);
	void Service();
	void ServiceFrontEndMode();
	void ServiceGameMode();
	void ServiceAmbience();
	void ServiceTrack();

	bool UsesPoliceRadio(CVehicle *veh);
	uint32 GetTrackStartPos(uint8);

	void ComputeAmbienceVol(uint8 reset, uint8& outVolume);
	bool ServiceAnnouncement();

	uint8 GetCarTuning();
	uint8 GetNextCarTuning();
	bool ChangeRadioChannel();
};

VALIDATE_SIZE(cMusicManager, 0x95C);

extern cMusicManager MusicManager;
