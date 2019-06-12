#pragma once

class CEntity;

class cDMAudio
{
public:
	void Terminate(void);
	void ReleaseDigitalHandle(void);
	void ReacquireDigitalHandle(void);
	void Service(void);
	void ReportCollision(CEntity *A, CEntity *B, uint8 surfA, uint8 surfB, float impulse, float speed);
	void ResetTimers(UInt32 timerval); 
	Bool IsAudioInitialised(void);
	Char GetCDAudioDriveLetter(void);
	Bool CheckForAnAudioFileOnCD(void);
	void ChangeMusicMode(UInt8 mode);
	void PlayFrontEndSound(int16 m_dwID, float fVolume);
};
extern cDMAudio &DMAudio;
