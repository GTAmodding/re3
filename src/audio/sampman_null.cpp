#include "common.h"
#if !defined(AUDIO_OAL) &&  !defined(AUDIO_MSS)
#include "sampman.h"
#include "AudioManager.h"

cSampleManager SampleManager;
bool _bSampmanInitialised = false;

uint32 BankStartOffset[MAX_SAMPLEBANKS];
uint32     nNumMP3s;

cSampleManager::cSampleManager(void)
{
	;
}

cSampleManager::~cSampleManager(void)
{
	
}

void cSampleManager::SetSpeakerConfig(int32 nConfig)
{

}

uint32 cSampleManager::GetMaximumSupportedChannels(void)
{	
	return MAXCHANNELS;
}

uint32 cSampleManager::GetNum3DProvidersAvailable()
{
	return 1;
}

void cSampleManager::SetNum3DProvidersAvailable(uint32 num)
{
	
}

char *cSampleManager::Get3DProviderName(uint8 id)
{
	static char name[64] = "NULL";
	return name;
}

void cSampleManager::Set3DProviderName(uint8 id, char *name)
{
	
}

int8 cSampleManager::GetCurrent3DProviderIndex(void)
{
	return 0;
}

int8 cSampleManager::SetCurrent3DProvider(uint8 nProvider)
{
	return 0;
}

bool
cSampleManager::IsMP3RadioChannelAvailable(void)
{
	return nNumMP3s != 0;
}


void cSampleManager::ReleaseDigitalHandle(void)
{
}

void cSampleManager::ReacquireDigitalHandle(void)
{
}

bool
cSampleManager::Initialise(void)
{
	return true;
}

void
cSampleManager::Terminate(void)
{

}

bool cSampleManager::CheckForAnAudioFileOnCD(void)
{
	return true;
}

char cSampleManager::GetCDAudioDriveLetter(void)
{
	return '\0';
}

void
cSampleManager::UpdateEffectsVolume(void)
{
	
}

void
cSampleManager::SetEffectsMasterVolume(uint8 nVolume)
{
}

void
cSampleManager::SetMusicMasterVolume(uint8 nVolume)
{
}

void
cSampleManager::SetEffectsFadeVolume(uint8 nVolume)
{
}

void
cSampleManager::SetMusicFadeVolume(uint8 nVolume)
{
}

void
cSampleManager::SetMonoMode(uint8 nMode)
{
}

bool
cSampleManager::LoadSampleBank(uint8 nBank)
{
	ASSERT( nBank < MAX_SAMPLEBANKS );
	return false;
}

void
cSampleManager::UnloadSampleBank(uint8 nBank)
{
	ASSERT( nBank < MAX_SAMPLEBANKS );
}

bool
cSampleManager::IsSampleBankLoaded(uint8 nBank)
{
	ASSERT( nBank < MAX_SAMPLEBANKS );
	
	return false;
}

bool
cSampleManager::IsPedCommentLoaded(uint32 nComment)
{
	ASSERT( nComment < TOTAL_AUDIO_SAMPLES );

	return false;
}


int32
cSampleManager::_GetPedCommentSlot(uint32 nComment)
{
	return -1;
}

bool
cSampleManager::LoadPedComment(uint32 nComment)
{
	ASSERT( nComment < TOTAL_AUDIO_SAMPLES );
	return false;
}

int32
cSampleManager::GetBankContainingSound(uint32 offset)
{
	return SAMPLEBANK_INVALID;
}

int32
cSampleManager::GetSampleBaseFrequency(uint32 nSample)
{
	ASSERT( nSample < TOTAL_AUDIO_SAMPLES );
	return 0;
}

int32
cSampleManager::GetSampleLoopStartOffset(uint32 nSample)
{
	ASSERT( nSample < TOTAL_AUDIO_SAMPLES );
	return 0;
}

int32
cSampleManager::GetSampleLoopEndOffset(uint32 nSample)
{
	ASSERT( nSample < TOTAL_AUDIO_SAMPLES );
	return 0;
}

uint32
cSampleManager::GetSampleLength(uint32 nSample)
{
	ASSERT( nSample < TOTAL_AUDIO_SAMPLES );
	return 0;
}

bool cSampleManager::UpdateReverb(void)
{
	return false;
}

void
cSampleManager::SetChannelReverbFlag(uint32 nChannel, uint8 nReverbFlag)
{
	ASSERT( nChannel < MAXCHANNELS+MAX2DCHANNELS );
}

bool
cSampleManager::InitialiseChannel(uint32 nChannel, uint32 nSfx, uint8 nBank)
{
	ASSERT( nChannel < MAXCHANNELS+MAX2DCHANNELS );
	return false;
}

void
cSampleManager::SetChannelEmittingVolume(uint32 nChannel, uint32 nVolume)
{
	ASSERT( nChannel != CHANNEL2D );
	ASSERT( nChannel < MAXCHANNELS+MAX2DCHANNELS );
}

void
cSampleManager::SetChannel3DPosition(uint32 nChannel, float fX, float fY, float fZ)
{
	ASSERT( nChannel != CHANNEL2D );
	ASSERT( nChannel < MAXCHANNELS+MAX2DCHANNELS );
}

void
cSampleManager::SetChannel3DDistances(uint32 nChannel, float fMax, float fMin)
{
	ASSERT( nChannel != CHANNEL2D );
	ASSERT( nChannel < MAXCHANNELS+MAX2DCHANNELS );
}

void
cSampleManager::SetChannelVolume(uint32 nChannel, uint32 nVolume)
{
	ASSERT( nChannel == CHANNEL2D );
	ASSERT( nChannel < MAXCHANNELS+MAX2DCHANNELS );
}

void
cSampleManager::SetChannelPan(uint32 nChannel, uint32 nPan)
{
	ASSERT(nChannel == CHANNEL2D);
	ASSERT( nChannel < MAXCHANNELS+MAX2DCHANNELS );
}

void
cSampleManager::SetChannelFrequency(uint32 nChannel, uint32 nFreq)
{
	ASSERT( nChannel < MAXCHANNELS+MAX2DCHANNELS );
}

void
cSampleManager::SetChannelLoopPoints(uint32 nChannel, uint32 nLoopStart, int32 nLoopEnd)
{
	ASSERT( nChannel < MAXCHANNELS+MAX2DCHANNELS );
}

void
cSampleManager::SetChannelLoopCount(uint32 nChannel, uint32 nLoopCount)
{
	ASSERT( nChannel < MAXCHANNELS+MAX2DCHANNELS );
}

bool
cSampleManager::GetChannelUsedFlag(uint32 nChannel)
{
	ASSERT( nChannel < MAXCHANNELS+MAX2DCHANNELS );

	return false;
}

void
cSampleManager::StartChannel(uint32 nChannel)
{
	ASSERT( nChannel < MAXCHANNELS+MAX2DCHANNELS );
}

void
cSampleManager::StopChannel(uint32 nChannel)
{
	ASSERT( nChannel < MAXCHANNELS+MAX2DCHANNELS );
}

void
cSampleManager::PreloadStreamedFile(uint32 nFile, uint8 nStream)
{
	ASSERT( nStream < MAX_STREAMS );
}

void
cSampleManager::PauseStream(uint8 nPauseFlag, uint8 nStream)
{
	ASSERT( nStream < MAX_STREAMS );
}

void
cSampleManager::StartPreloadedStreamedFile(uint8 nStream)
{
	ASSERT( nStream < MAX_STREAMS );
}

bool
cSampleManager::StartStreamedFile(uint32 nFile, uint32 nPos, uint8 nStream)
{	
	ASSERT( nStream < MAX_STREAMS );
	
	return false;
}

void
cSampleManager::StopStreamedFile(uint8 nStream)
{
	ASSERT( nStream < MAX_STREAMS );
}

int32
cSampleManager::GetStreamedFilePosition(uint8 nStream)
{
	ASSERT( nStream < MAX_STREAMS );
	
	return 0;
}

void
cSampleManager::SetStreamedVolumeAndPan(uint8 nVolume, uint8 nPan, uint8 nEffectFlag, uint8 nStream)
{
	ASSERT( nStream < MAX_STREAMS );
}

int32
cSampleManager::GetStreamedFileLength(uint8 nStream)
{
	ASSERT( nStream < TOTAL_STREAMED_SOUNDS );

	return 1;
}

bool
cSampleManager::IsStreamPlaying(uint8 nStream)
{
	ASSERT( nStream < MAX_STREAMS );

	return false;
}

bool
cSampleManager::InitialiseSampleBanks(void)
{
	
	return true;
}

#endif
