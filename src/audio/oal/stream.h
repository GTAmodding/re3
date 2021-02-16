#pragma once

#ifdef AUDIO_OAL
#include <AL/al.h>

#define NUM_STREAMBUFFERS 8

class IDecoder
{
public:
	virtual ~IDecoder() { }
	
	virtual bool   IsOpened() = 0;
	
	virtual uint32 GetSampleSize() = 0;
	virtual uint32 GetSampleCount() = 0;
	virtual uint32 GetSampleRate() = 0;
	virtual uint32 GetChannels() = 0;
	
	uint32 GetAvgSamplesPerSec()
	{
		return GetChannels() * GetSampleRate();
	}
	
	uint32 ms2samples(uint32 ms)
	{
		return float(ms) / 1000.0f * float(GetSampleRate());
	}
	
	uint32 samples2ms(uint32 sm)
	{
		return float(sm) * 1000.0f / float(GetSampleRate());
	}
	
	uint32 GetBufferSamples()
	{
		//return (GetAvgSamplesPerSec() >> 2) - (GetSampleCount() % GetChannels());
		return (GetAvgSamplesPerSec() / 4); // 250ms
	}
	
	uint32 GetBufferSize()
	{
		return GetBufferSamples() * GetSampleSize();
	}
	
	virtual void   Seek(uint32 milliseconds) = 0;
	virtual uint32 Tell() = 0;
	
	uint32 GetLength()
	{
		return float(GetSampleCount()) * 1000.0f / float(GetSampleRate());
	}
	
	virtual uint32 Decode(void *buffer) = 0;
};

class CStream
{
	char     m_aFilename[128];
	ALuint  *m_pAlSources;
	ALuint (&m_alBuffers)[NUM_STREAMBUFFERS];
	
	bool     m_bPaused;
	bool     m_bActive;
	
	void    *m_pBuffer;
	
	bool     m_bReset;
	uint32   m_nVolume;
	uint8    m_nPan;
	uint32   m_nPosBeforeReset;
	int32   m_nLoopCount;
	
	IDecoder *m_pSoundFile;
	
	bool HasSource();
	void SetPosition(int i, float x, float y, float z);
	void SetPitch(float pitch);
	void SetGain(float gain);
	void   Pause();
	void   SetPlay(bool state);
	
	bool   FillBuffer(ALuint *alBuffer);
	int32  FillBuffers();
	void   ClearBuffers();
public:
	static void Initialise();
	static void Terminate();
	
	CStream(char *filename, ALuint *sources, ALuint (&buffers)[NUM_STREAMBUFFERS], uint32 overrideSampleRate = 32000);
	~CStream();
	void   Delete();
	
	bool   IsOpened();
	bool   IsPlaying();
	void   SetPause (bool bPause);
	void   SetVolume(uint32 nVol);
	void   SetPan   (uint8 nPan);
	void   SetPosMS (uint32 nPos); 
	uint32 GetPosMS();
	uint32 GetLengthMS();
	
	bool Setup(bool imSureQueueIsEmpty = false);
	void Start();
	void Stop();
	void Update(void);
	void SetLoopCount(int32);

	
	void ProviderInit();
	void ProviderTerm();
};

#endif
