#include "common.h"

#ifdef AUDIO_OAL
#include "channel.h"
#include "sampman.h"

#ifndef _WIN32
#include <float.h>
#endif

extern bool IsFXSupported();

CChannel::CChannel()
{
	alSource  = AL_NONE;
	alFilter  = AL_FILTER_NULL;
	SetDefault();
}

void CChannel::SetDefault()
{
	alBuffer = AL_NONE;

	Pitch = 1.0f;
	Gain = 1.0f;
	Mix = 0.0f;
		
	Position[0] = 0.0f; Position[1] = 0.0f; Position[2] = 0.0f;
	Distances[0] = 0.0f; Distances[1] = FLT_MAX;
	LoopCount  = 1;
	LoopPoints[0] = 0; LoopPoints[1] = -1;
	
	Frequency = MAX_FREQ;
}

void CChannel::Reset()
{
	ClearBuffer();
	SetDefault();
}

void CChannel::Init(bool Is2D)
{
	ASSERT(!HasSource());
	alGenSources(1, &alSource);
	if ( HasSource() )
	{
		alSourcei(alSource, AL_SOURCE_RELATIVE, AL_TRUE);
		if ( IsFXSupported() )
			alSource3i(alSource, AL_AUXILIARY_SEND_FILTER, AL_EFFECTSLOT_NULL, 0, AL_FILTER_NULL);
		
		if ( Is2D )
		{
			alSource3f(alSource, AL_POSITION, 0.0f, 0.0f, 0.0f);
			alSourcef (alSource, AL_GAIN, 1.0f);
		}
		else
		{
			if ( IsFXSupported() )
				alGenFilters(1,&alFilter);
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
			alSource3i(alSource, AL_AUXILIARY_SEND_FILTER, AL_EFFECTSLOT_NULL, 0, AL_FILTER_NULL);
			
			if(alFilter != AL_FILTER_NULL)
				alDeleteFilters(1,&alFilter);
		}

		alDeleteSources(1, &alSource);
	}
	alSource = AL_NONE;
	alFilter = AL_FILTER_NULL;
}

void CChannel::Start()
{
	if ( !HasSource() ) return;
	
	if ( LoopPoints[0] != 0 && LoopPoints[0] != -1 )
		alBufferiv(alBuffer, AL_LOOP_POINTS_SOFT, LoopPoints);
	alSourcei   (alSource, AL_BUFFER, alBuffer);
	alSourcePlay(alSource);
}

void CChannel::Stop()
{
	if ( HasSource() )
		alSourceStop(alSource);
	
	Reset();
}

bool CChannel::HasSource()
{
	return alSource != AL_NONE;
}
	
bool CChannel::IsUsed()
{
	if ( HasSource() )
	{
		ALint sourceState;
		alGetSourcei(alSource, AL_SOURCE_STATE, &sourceState);
		return sourceState == AL_PLAYING;
	}
	return false;
}

void CChannel::SetPitch(float pitch)
{
	if ( !HasSource() ) return;
	alSourcef(alSource, AL_PITCH, pitch);
}

void CChannel::SetGain(float gain)
{
	if ( !HasSource() ) return;
	alSourcef(alSource, AL_GAIN, gain);
}
	
void CChannel::SetVolume(int32 vol)
{
	SetGain(ALfloat(vol) / MAX_VOLUME);
}
	
void CChannel::SetSampleID(uint32 nSfx)
{
	Sample = nSfx;
}
	
void CChannel::SetFreq(int32 freq)
{
	Frequency = freq;
}
	
void CChannel::SetCurrentFreq(uint32 freq)
{
	SetPitch(ALfloat(freq) / Frequency);
}

void CChannel::SetLoopCount(int32 loopCount) // fake. TODO:
{
	if ( !HasSource() ) return;
	alSourcei(alSource, AL_LOOPING, loopCount == 1 ? AL_FALSE : AL_TRUE);
}

void CChannel::SetLoopPoints(ALint start, ALint end)
{
	LoopPoints[0] = start;
	LoopPoints[1] = end;
}
	
void CChannel::SetPosition(float x, float y, float z)
{
	if ( !HasSource() ) return;
	alSource3f(alSource, AL_POSITION, x, y, z);
}
	
void CChannel::SetDistances(float max, float min)
{
	if ( !HasSource() ) return;
	alSourcef   (alSource, AL_MAX_DISTANCE,       max);
	alSourcef   (alSource, AL_REFERENCE_DISTANCE, min);
	alSourcef   (alSource, AL_MAX_GAIN, 1.0f);
	alSourcef   (alSource, AL_ROLLOFF_FACTOR, 1.0f);
}
	
void CChannel::SetPan(uint32 pan)
{
	SetPosition((pan-63)/64.0f, 0.0f, Sqrt(1.0f-SQR((pan-63)/64.0f)));
}

void CChannel::SetBuffer(ALuint buffer)
{
	alBuffer = buffer;
}

void CChannel::ClearBuffer()
{
	if ( !HasSource() ) return;
	SetBuffer(AL_NONE);
	alSourcei(alSource, AL_BUFFER, AL_NONE);
}

void CChannel::SetReverbMix(ALuint slot, float mix)
{
	if ( !IsFXSupported() ) return;
	if ( !HasSource() ) return;
	if ( alFilter == AL_FILTER_NULL ) return;
	
	Mix = mix;
	EAX3_SetReverbMix(alFilter, mix);
	alSource3i(alSource, AL_AUXILIARY_SEND_FILTER, slot, 0, alFilter);
}

void CChannel::UpdateReverb(ALuint slot)
{
	if ( !IsFXSupported() ) return;
	if ( !HasSource() ) return;
	if ( alFilter == AL_FILTER_NULL ) return;
	EAX3_SetReverbMix(alFilter, Mix);
	alSource3i(alSource, AL_AUXILIARY_SEND_FILTER, slot, 0, alFilter);
}

#endif
