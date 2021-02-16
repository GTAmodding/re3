#include "common.h"

#ifdef AUDIO_OAL
#include "channel.h"
#include "sampman.h"

#ifndef _WIN32
#include <float.h>
#endif

extern bool IsFXSupported();

ALuint alSources[MAXCHANNELS+MAX2DCHANNELS];
ALuint alFilters[MAXCHANNELS+MAX2DCHANNELS];
ALuint alBuffers[MAXCHANNELS+MAX2DCHANNELS];
bool bChannelsCreated = false;

int32 CChannel::channelsThatNeedService = 0;

void
CChannel::InitChannels()
{
	alGenSources(MAXCHANNELS+MAX2DCHANNELS, alSources);
	alGenBuffers(MAXCHANNELS+MAX2DCHANNELS, alBuffers);
	if (IsFXSupported())
		alGenFilters(MAXCHANNELS + MAX2DCHANNELS, alFilters);
	bChannelsCreated = true;
}

void
CChannel::DestroyChannels()
{
	if (bChannelsCreated) 
	{
		alDeleteSources(MAXCHANNELS + MAX2DCHANNELS, alSources);
		memset(alSources, 0, sizeof(alSources));
		alDeleteBuffers(MAXCHANNELS + MAX2DCHANNELS, alBuffers);
		memset(alBuffers, 0, sizeof(alBuffers));
		if (IsFXSupported())
		{
			alDeleteFilters(MAXCHANNELS + MAX2DCHANNELS, alFilters);
			memset(alFilters, 0, sizeof(alFilters));
		}
		bChannelsCreated = false;
	}
}


CChannel::CChannel()
{
	Data = nil;
	DataSize = 0;
	SetDefault();
}

void CChannel::SetDefault()
{
	Pitch = 1.0f;
	Gain = 1.0f;
	Mix = 0.0f;
		
	Position[0] = 0.0f; Position[1] = 0.0f; Position[2] = 0.0f;
	Distances[0] = 0.0f; Distances[1] = FLT_MAX;

	LoopCount = 1;
	LastProcessedOffset = UINT32_MAX;
	LoopPoints[0] = 0; LoopPoints[1] = -1;
	
	Frequency = MAX_FREQ;
}

void CChannel::Reset()
{
	// Here is safe because ctor don't call this
	if (LoopCount > 1)
		channelsThatNeedService--;

	ClearBuffer();
	SetDefault();
}

void CChannel::Init(uint32 _id, bool Is2D)
{
	id = _id;
	if ( HasSource() )
	{
		alSourcei(alSources[id], AL_SOURCE_RELATIVE, AL_TRUE);
		if ( IsFXSupported() )
			alSource3i(alSources[id], AL_AUXILIARY_SEND_FILTER, AL_EFFECTSLOT_NULL, 0, AL_FILTER_NULL);
		
		if ( Is2D )
		{
			alSource3f(alSources[id], AL_POSITION, 0.0f, 0.0f, 0.0f);
			alSourcef(alSources[id], AL_GAIN, 1.0f);
		}
	}
}

void CChannel::Term()
{
	Stop();
	if ( HasSource() )
	{
		if ( IsFXSupported() )
		{
			alSource3i(alSources[id], AL_AUXILIARY_SEND_FILTER, AL_EFFECTSLOT_NULL, 0, AL_FILTER_NULL);
		}
	}
}

void CChannel::Start()
{
	if ( !HasSource() ) return;
	if ( !Data ) return;

	alBufferData(alBuffers[id], AL_FORMAT_MONO16, Data, DataSize, Frequency);
	if ( LoopPoints[0] != 0 && LoopPoints[0] != -1 )
		alBufferiv(alBuffers[id], AL_LOOP_POINTS_SOFT, LoopPoints);
	alSourcei(alSources[id], AL_BUFFER, alBuffers[id]);
	alSourcePlay(alSources[id]);
}

void CChannel::Stop()
{
	if ( HasSource() )
		alSourceStop(alSources[id]);
	
	Reset();
}

bool CChannel::HasSource()
{
	return alSources[id] != AL_NONE;
}
	
bool CChannel::IsUsed()
{
	if ( HasSource() )
	{
		ALint sourceState;
		alGetSourcei(alSources[id], AL_SOURCE_STATE, &sourceState);
		return sourceState == AL_PLAYING;
	}
	return false;
}

void CChannel::SetPitch(float pitch)
{
	if ( !HasSource() ) return;
	alSourcef(alSources[id], AL_PITCH, pitch);
}

void CChannel::SetGain(float gain)
{
	if ( !HasSource() ) return;
	alSourcef(alSources[id], AL_GAIN, gain);
}
	
void CChannel::SetVolume(int32 vol)
{
	SetGain(ALfloat(vol) / MAX_VOLUME);
}

void CChannel::SetSampleData(void *_data, size_t _DataSize, int32 freq)
{
	Data = _data;
	DataSize = _DataSize;
	Frequency = freq;
}
	
void CChannel::SetCurrentFreq(uint32 freq)
{
	SetPitch(ALfloat(freq) / Frequency);
}

void CChannel::SetLoopCount(int32 count)
{
	if ( !HasSource() ) return;

	// 0: loop indefinitely, 1: play one time, 2: play two times etc...
	// only > 1 needs manual processing

	if (LoopCount > 1 && count < 2)
		channelsThatNeedService--;
	else if (LoopCount < 2 && count > 1)
		channelsThatNeedService++;

	alSourcei(alSources[id], AL_LOOPING, count == 1 ? AL_FALSE : AL_TRUE);
	LoopCount = count;
}

bool CChannel::Update()
{
	if (!HasSource()) return false;
	if (LoopCount < 2) return false;

	ALint state;
	alGetSourcei(alSources[id], AL_SOURCE_STATE, &state);
	if (state == AL_STOPPED) {
		debug("Looping channels(%d in this case) shouldn't report AL_STOPPED, but nvm\n", id);
		SetLoopCount(1);
		return true;
	}

	assert(channelsThatNeedService > 0 && "Ref counting is broken");

	ALint offset;
	alGetSourcei(alSources[id], AL_SAMPLE_OFFSET, &offset);

	// Rewound
	if (offset < LastProcessedOffset) {
		LoopCount--;
		if (LoopCount == 1) {
			// Playing last tune...
			channelsThatNeedService--;
			alSourcei(alSources[id], AL_LOOPING, AL_FALSE);
		}
	}
	LastProcessedOffset = offset;
	return true;
}

void CChannel::SetLoopPoints(ALint start, ALint end)
{
	LoopPoints[0] = start;
	LoopPoints[1] = end;
}
	
void CChannel::SetPosition(float x, float y, float z)
{
	if ( !HasSource() ) return;
	alSource3f(alSources[id], AL_POSITION, x, y, z);
}
	
void CChannel::SetDistances(float max, float min)
{
	if ( !HasSource() ) return;
	alSourcef   (alSources[id], AL_MAX_DISTANCE,       max);
	alSourcef   (alSources[id], AL_REFERENCE_DISTANCE, min);
	alSourcef   (alSources[id], AL_MAX_GAIN, 1.0f);
	alSourcef   (alSources[id], AL_ROLLOFF_FACTOR, 1.0f);
}
	
void CChannel::SetPan(int32 pan)
{
	SetPosition((pan-63)/64.0f, 0.0f, Sqrt(1.0f-SQR((pan-63)/64.0f)));
}

void CChannel::ClearBuffer()
{
	if ( !HasSource() ) return;
	alSourcei(alSources[id], AL_LOOPING, AL_FALSE);
	alSourcei(alSources[id], AL_BUFFER, AL_NONE);
	Data = nil;
	DataSize = 0;
}

void CChannel::SetReverbMix(ALuint slot, float mix)
{
	if ( !IsFXSupported() ) return;
	if ( !HasSource() ) return;
	if ( alFilters[id] == AL_FILTER_NULL ) return;
	
	Mix = mix;
	EAX3_SetReverbMix(alFilters[id], mix);
	alSource3i(alSources[id], AL_AUXILIARY_SEND_FILTER, slot, 0, alFilters[id]);
}

void CChannel::UpdateReverb(ALuint slot)
{
	if ( !IsFXSupported() ) return;
	if ( !HasSource() ) return;
	if ( alFilters[id] == AL_FILTER_NULL ) return;
	EAX3_SetReverbMix(alFilters[id], Mix);
	alSource3i(alSources[id], AL_AUXILIARY_SEND_FILTER, slot, 0, alFilters[id]);
}

#endif
