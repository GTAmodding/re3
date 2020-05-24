#pragma once

#ifdef AUDIO_OAL
#include "oal/oal_utils.h"
#include <AL/al.h>
#include <AL/alext.h>
#include <AL/efx.h>


class CChannel
{
	ALuint alSource;
	ALuint alFilter;
	ALuint alBuffer;
	float  Pitch, Gain;
	float  Mix;
	int32  Frequency;
	float  Position[3];
	float  Distances[2];
	int32  LoopCount;
	ALint  LoopPoints[2];
	uint32 Sample;
public:
	CChannel();
	void SetDefault();
	void Reset();
	void Init(bool Is2D = false);
	void Term();
	void Start();
	void Stop();
	bool HasSource();
	bool IsUsed();
	void SetPitch(float pitch);
	void SetGain(float gain);
	void SetVolume(int32 vol);
	void SetSampleID(uint32 nSfx);
	void SetFreq(int32 freq);
	void SetCurrentFreq(uint32 freq);
	void SetLoopCount(int32 loopCount); // fake
	void SetLoopPoints(ALint start, ALint end);
	void SetPosition(float x, float y, float z);
	void SetDistances(float max, float min);
	void SetPan(uint32 pan);
	void SetBuffer(ALuint buffer);
	void ClearBuffer();
	void SetReverbMix(ALuint slot, float mix);
	void UpdateReverb(ALuint slot);
};

#endif