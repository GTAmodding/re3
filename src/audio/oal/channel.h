#pragma once

#ifdef AUDIO_OAL
#include "oal/oal_utils.h"
#include <AL/al.h>
#include <AL/alext.h>
#include <AL/efx.h>


class CChannel
{
	uint32 id;
	float  Pitch, Gain;
	float  Mix;
	void  *Data;
	size_t DataSize;
	int32  Frequency;
	float  Position[3];
	float  Distances[2];
	int32  LoopCount;
	ALint  LoopPoints[2];
	ALint  LastProcessedOffset;
public:
	static int32 channelsThatNeedService;

	static void InitChannels();
	static void DestroyChannels();

	CChannel();
	void SetDefault();
	void Reset();
	void Init(uint32 _id, bool Is2D = false);
	void Term();
	void Start();
	void Stop();
	bool HasSource();
	bool IsUsed();
	void SetPitch(float pitch);
	void SetGain(float gain);
	void SetVolume(int32 vol);
	void SetSampleData(void *_data, size_t _DataSize, int32 freq);
	void SetCurrentFreq(uint32 freq);
	void SetLoopCount(int32 count);
	void SetLoopPoints(ALint start, ALint end);
	void SetPosition(float x, float y, float z);
	void SetDistances(float max, float min);
	void SetPan(int32 pan);
	void ClearBuffer();
	void SetReverbMix(ALuint slot, float mix);
	void UpdateReverb(ALuint slot);
	bool Update();
};

#endif