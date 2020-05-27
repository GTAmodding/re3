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
	uint32 m_nCurrentStreamedSound;
	uint32 m_nPreviousStreamedSound;
	bool m_bFrontendTrackFinished;
	bool m_bPlayInFrontend;
	bool m_bSetNextStation;
	uint32 m_nAnnouncement;
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
	bool m_bDontServiceAmbienceTrack;
	bool m_bRadioSetByScript;
	uint32 m_nRadioStation;
	int32 m_nRadioPosition;
	uint32 m_nRadioInCar;

public:
	cMusicManager();
	bool IsInitialised() { return m_bIsInitialised; }
	uint32 GetMusicMode() { return m_nMusicMode; }
	uint8 GetCurrentTrack() { return m_nCurrentStreamedSound; }

	bool Initialise();
	void Terminate();

	void ChangeMusicMode(uint8 mode);
	void StopFrontEndTrack();

	bool PlayerInCar();
	void DisplayRadioStationName();

	void PlayAnnouncement(uint32);
	void PlayFrontEndTrack(uint32, uint8);
	void PreloadCutSceneMusic(uint32);
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

	uint32 GetCarTuning();
	uint32 GetNextCarTuning();
	bool ChangeRadioChannel();
};

VALIDATE_SIZE(cMusicManager, 0x95C);

extern cMusicManager MusicManager;
