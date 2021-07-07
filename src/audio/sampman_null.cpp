#include "common.h"
#if !defined(AUDIO_OAL) &&  !defined(AUDIO_MSS)
#include "sampman.h"
#include "AudioManager.h"

cSampleManager SampleManager;
bool8 _bSampmanInitialised = FALSE;

uint32 BankStartOffset[MAX_SFX_BANKS];
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

bool8
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

bool8
cSampleManager::Initialise(void)
{
	return TRUE;
}

void
cSampleManager::Terminate(void)
{

}

bool8 cSampleManager::CheckForAnAudioFileOnCD(void)
{
	return TRUE;
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
cSampleManager::SetMP3BoostVolume(uint8 nVolume)
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
cSampleManager::SetMonoMode(bool8 nMode)
{
}

bool8
cSampleManager::LoadSampleBank(uint8 nBank)
{
	ASSERT( nBank < MAX_SFX_BANKS );
	return FALSE;
}

void
cSampleManager::UnloadSampleBank(uint8 nBank)
{
	ASSERT( nBank < MAX_SFX_BANKS );
}

bool8
cSampleManager::IsSampleBankLoaded(uint8 nBank)
{
	ASSERT( nBank < MAX_SFX_BANKS );
	
	return FALSE;
}

bool8
cSampleManager::IsPedCommentLoaded(uint32 nComment)
{
	ASSERT( nComment < TOTAL_AUDIO_SAMPLES );

	return FALSE;
}


int32
cSampleManager::_GetPedCommentSlot(uint32 nComment)
{
	return -1;
}

bool8
cSampleManager::LoadPedComment(uint32 nComment)
{
	ASSERT( nComment < TOTAL_AUDIO_SAMPLES );
	return FALSE;
}

int32
cSampleManager::GetBankContainingSound(uint32 offset)
{
	return INVALID_SFX_BANK;
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

bool8 cSampleManager::UpdateReverb(void)
{
	return FALSE;
}

void
cSampleManager::SetChannelReverbFlag(uint32 nChannel, bool8 nReverbFlag)
{
	ASSERT( nChannel < MAXCHANNELS+MAX2DCHANNELS );
}

bool8
cSampleManager::InitialiseChannel(uint32 nChannel, uint32 nSfx, uint8 nBank)
{
	ASSERT( nChannel < MAXCHANNELS+MAX2DCHANNELS );
	return FALSE;
}

void
cSampleManager::SetChannelEmittingVolume(uint32 nChannel, uint32 nVolume)
{
	ASSERT( nChannel < MAXCHANNELS );
	ASSERT( nChannel < MAXCHANNELS+MAX2DCHANNELS );
}

void
cSampleManager::SetChannel3DPosition(uint32 nChannel, float fX, float fY, float fZ)
{
	ASSERT( nChannel < MAXCHANNELS );
	ASSERT( nChannel < MAXCHANNELS+MAX2DCHANNELS );
}

void
cSampleManager::SetChannel3DDistances(uint32 nChannel, float fMax, float fMin)
{
	ASSERT( nChannel < MAXCHANNELS );
	ASSERT( nChannel < MAXCHANNELS+MAX2DCHANNELS );
}

void
cSampleManager::SetChannelVolume(uint32 nChannel, uint32 nVolume)
{
	ASSERT( nChannel >= MAXCHANNELS );
	ASSERT( nChannel < MAXCHANNELS+MAX2DCHANNELS );
}

void
cSampleManager::SetChannelPan(uint32 nChannel, uint32 nPan)
{
	ASSERT( nChannel >= MAXCHANNELS );
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

bool8
cSampleManager::GetChannelUsedFlag(uint32 nChannel)
{
	ASSERT( nChannel < MAXCHANNELS+MAX2DCHANNELS );

	return FALSE;
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
cSampleManager::PauseStream(bool8 nPauseFlag, uint8 nStream)
{
	ASSERT( nStream < MAX_STREAMS );
}

void
cSampleManager::StartPreloadedStreamedFile(uint8 nStream)
{
	ASSERT( nStream < MAX_STREAMS );
}

bool8
cSampleManager::StartStreamedFile(uint32 nFile, uint32 nPos, uint8 nStream)
{	
	ASSERT( nStream < MAX_STREAMS );
	
	return FALSE;
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
cSampleManager::SetStreamedVolumeAndPan(uint8 nVolume, uint8 nPan, bool8 nEffectFlag, uint8 nStream)
{
	ASSERT( nStream < MAX_STREAMS );
}

int32
cSampleManager::GetStreamedFileLength(uint8 nStream)
{
	ASSERT( nStream < TOTAL_STREAMED_SOUNDS );

	return 1;
}

bool8
cSampleManager::IsStreamPlaying(uint8 nStream)
{
	ASSERT( nStream < MAX_STREAMS );

	return FALSE;
}

bool8
cSampleManager::InitialiseSampleBanks(void)
{
	
	return TRUE;
}

void
cSampleManager::SetStreamedFileLoopFlag(bool8 nLoopFlag, uint8 nChannel)
{
}

int8 cSampleManager::AutoDetect3DProviders()
{
	return -1;
}

#endif
