#include "common.h"
//#define JUICY_OAL

#ifdef AUDIO_OAL
#include "sampman.h"

#include <time.h>

#include "eax.h"
#include "eax-util.h"

#ifdef _WIN32
#include <io.h>
#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>
#include <AL/efx.h>
#include <AL/efx-presets.h>
#endif

#include "oal/oal_utils.h"
#include "oal/aldlist.h"
#include "oal/channel.h"
#include "oal/stream.h"

#include "AudioManager.h"
#include "MusicManager.h"
#include "Frontend.h"
#include "Timer.h"

//TODO: fix eax3 reverb
//TODO: max channals
//TODO: loop count
//TODO: mp3 player

#ifdef _WIN32
#pragma comment( lib, "OpenAL32.lib" )
#endif

cSampleManager SampleManager;
bool _bSampmanInitialised = false;

uint32 BankStartOffset[MAX_SAMPLEBANKS];

int           prevprovider=-1;
int           curprovider=-1;
int           usingEAX=0;
int           usingEAX3=0;
//int         speaker_type=0;
ALCdevice    *ALDevice = NULL;
ALCcontext   *ALContext = NULL;
unsigned int _maxSamples;
float        _fPrevEaxRatioDestination;
bool         _usingEFX;
float        _fEffectsLevel;
ALuint       ALEffect = AL_EFFECT_NULL;
ALuint       ALEffectSlot = AL_EFFECTSLOT_NULL;
struct
{
	char id[256];
	char name[256];
	int sources;
}providers[MAXPROVIDERS];

int defaultProvider;


char SampleBankDescFilename[] = "audio/sfx.SDT";
char SampleBankDataFilename[] = "audio/sfx.RAW";

FILE *fpSampleDescHandle;
FILE *fpSampleDataHandle;
bool  bSampleBankLoaded            [MAX_SAMPLEBANKS];
int32 nSampleBankDiscStartOffset   [MAX_SAMPLEBANKS];
int32 nSampleBankSize              [MAX_SAMPLEBANKS];
uintptr nSampleBankMemoryStartAddress[MAX_SAMPLEBANKS];
int32 _nSampleDataEndOffset;

int32 nPedSlotSfx    [MAX_PEDSFX];
int32 nPedSlotSfxAddr[MAX_PEDSFX];
uint8 nCurrentPedSlot;

ALuint pedBuffers[MAX_PEDSFX];

CChannel aChannel[MAXCHANNELS+MAX2DCHANNELS];
uint8 nChannelVolume[MAXCHANNELS+MAX2DCHANNELS];

uint32 nStreamLength[TOTAL_STREAMED_SOUNDS];
ALuint ALStreamSources[MAX_STREAMS];
ALuint ALStreamBuffers[MAX_STREAMS][NUM_STREAMBUFFERS];
struct
{
	ALuint buffer;
	ALuint timer;

	bool IsEmpty() { return timer == 0; }
	void Set(ALuint buf) { buffer = buf; }
	void Wait() { timer  = 10000; }
	void Init()
	{
		buffer = 0;
		timer  = 0;
	}
	void Term()
	{
		if ( buffer != 0 && alIsBuffer(buffer) )
			alDeleteBuffers(1, &buffer);
		timer = 0;
	}
	void Update()
	{
		if ( !(timer > 0) ) return;
		timer -= ALuint(CTimer::GetTimeStepInMilliseconds());
		if ( timer > 0 ) return;
		if ( buffer != 0 && alIsBuffer(buffer) )
		{
			alDeleteBuffers(1, &buffer);
			timer = ( alGetError() == AL_NO_ERROR ) ? 0 : 10000;
		}
	}
}ALBuffers[SAMPLEBANK_MAX];

uint32     nNumMP3s;
CStream    *aStream[MAX_STREAMS];
uint8      nStreamPan   [MAX_STREAMS];
uint8      nStreamVolume[MAX_STREAMS];

///////////////////////////////////////////////////////////////
//	Env		Size	Diffus	Room	RoomHF	RoomLF	DecTm	DcHF	DcLF	Refl	RefDel	Ref Pan				Revb	RevDel		Rev Pan				EchTm	EchDp	ModTm	ModDp	AirAbs	HFRef		LFRef	RRlOff	FLAGS
EAXLISTENERPROPERTIES StartEAX3 =
	{26,	1.7f,	0.8f,	-1000,	-1000,	-100,	4.42f,	0.14f,	1.00f,	429,	0.014f,	0.00f,0.00f,0.00f,	1023,	0.021f,		0.00f,0.00f,0.00f,	0.250f,	0.000f,	0.250f,	0.000f,	-5.0f,	2727.1f,	250.0f,	0.00f,	0x3f };

EAXLISTENERPROPERTIES FinishEAX3 =
	{26,	100.0f,	1.0f,	0,		-1000,	-2200,	20.0f,	1.39f,	1.00f,	1000,	0.069f,	0.00f,0.00f,0.00f,	400,	0.100f,		0.00f,0.00f,0.00f,	0.250f,	1.000f,	3.982f,	0.000f,	-18.0f,	3530.8f,	417.9f,	6.70f,	0x3f };

EAXLISTENERPROPERTIES EAX3Params;


bool IsFXSupported()
{
	return usingEAX || usingEAX3 || _usingEFX;
}

void EAX_SetAll(const EAXLISTENERPROPERTIES *allparameters)
{
	if ( usingEAX || usingEAX3 )
		EAX3_Set(ALEffect, allparameters);
	else
		EFX_Set(ALEffect, allparameters);
}

static void
add_providers()
{
	SampleManager.SetNum3DProvidersAvailable(0);
	
	ALDeviceList *pDeviceList = NULL;
	pDeviceList = new ALDeviceList();

	if ((pDeviceList) && (pDeviceList->GetNumDevices()))
	{
		const int devNumber = Min(pDeviceList->GetNumDevices(), MAXPROVIDERS);
		int n = 0;
		
		for (int i = 0; i < devNumber; i++) 
		{
			if ( n < MAXPROVIDERS )
			{
				strcpy(providers[n].id, pDeviceList->GetDeviceName(i));
				strncpy(providers[n].name, pDeviceList->GetDeviceName(i), sizeof(providers[n].name));
				providers[n].sources = pDeviceList->GetMaxNumSources(i);
				SampleManager.Set3DProviderName(n, providers[n].name);
				n++;
			}
			
			if ( alGetEnumValue("AL_EFFECT_EAXREVERB") != 0
				|| pDeviceList->IsExtensionSupported(i, ADEXT_EAX2)
				|| pDeviceList->IsExtensionSupported(i, ADEXT_EAX3) 
				|| pDeviceList->IsExtensionSupported(i, ADEXT_EAX4)
				|| pDeviceList->IsExtensionSupported(i, ADEXT_EAX5) )
			{
				if ( n < MAXPROVIDERS )
				{
					strcpy(providers[n].id, pDeviceList->GetDeviceName(i));
					strncpy(providers[n].name, pDeviceList->GetDeviceName(i), sizeof(providers[n].name));
					strcat(providers[n].name, " EAX");
					providers[n].sources = pDeviceList->GetMaxNumSources(i);
					SampleManager.Set3DProviderName(n, providers[n].name);
					n++;
				}
				
				if ( n < MAXPROVIDERS )
				{
					strcpy(providers[n].id, pDeviceList->GetDeviceName(i));
					strncpy(providers[n].name, pDeviceList->GetDeviceName(i), sizeof(providers[n].name));
					strcat(providers[n].name, " EAX3");
					providers[n].sources = pDeviceList->GetMaxNumSources(i);
					SampleManager.Set3DProviderName(n, providers[n].name);
					n++;
				}
			}
		}
		SampleManager.SetNum3DProvidersAvailable(n);
	
		for(int j=n;j<MAXPROVIDERS;j++)
			SampleManager.Set3DProviderName(j, NULL);
		
		defaultProvider = pDeviceList->GetDefaultDevice();
		if ( defaultProvider > MAXPROVIDERS )
			defaultProvider = 0;
	}
	
	delete pDeviceList;
}

static void
release_existing()
{
	for ( int32 i = 0; i < MAXCHANNELS; i++ )
		aChannel[i].Term();
	aChannel[CHANNEL2D].Term();
	
	if ( IsFXSupported() )
	{
		if ( alIsEffect(ALEffect) )
		{
			alEffecti(ALEffect, AL_EFFECT_TYPE, AL_EFFECT_NULL);
			alDeleteEffects(1, &ALEffect);
			ALEffect = AL_EFFECT_NULL;
		}
		
		if (alIsAuxiliaryEffectSlot(ALEffectSlot))
		{
			alAuxiliaryEffectSloti(ALEffectSlot, AL_EFFECTSLOT_EFFECT, AL_EFFECT_NULL);
			
			alDeleteAuxiliaryEffectSlots(1, &ALEffectSlot);
			ALEffectSlot = AL_EFFECTSLOT_NULL;
		}
	}
	
	for ( int32 i = 0; i < MAX_STREAMS; i++ )
	{
		CStream *stream = aStream[i];
		if (stream)
			stream->ProviderTerm();
		
		alDeleteSources(1, &ALStreamSources[i]);
		alDeleteBuffers(NUM_STREAMBUFFERS, ALStreamBuffers[i]);
	}
	
	alDeleteBuffers(MAX_PEDSFX, pedBuffers);
	
	for ( int32 i = 0; i < SAMPLEBANK_MAX; i++ )
	{
		ALBuffers[i].Term();
	}
	
	if ( ALContext )
	{
		alcMakeContextCurrent(NULL);
		alcSuspendContext(ALContext);
		alcDestroyContext(ALContext);
	}
	if ( ALDevice )
		alcCloseDevice(ALDevice);
	
	ALDevice = NULL;
	ALContext = NULL;
	
	_fPrevEaxRatioDestination = 0.0f;
	_usingEFX                 = false;
	_fEffectsLevel            = 0.0f;
	
	DEV("release_existing()\n");
}

static bool
set_new_provider(int index)
{
	if ( curprovider == index )
		return true;
	
	curprovider = index;
	
	release_existing();
	
	if ( curprovider != -1 )
	{
		DEV("set_new_provider()\n");
		
		//TODO:
		_maxSamples = MAXCHANNELS;
		
		ALCint attr[] = {ALC_FREQUENCY,MAX_FREQ,0};
		
		ALDevice  = alcOpenDevice(providers[index].id);
		ASSERT(ALDevice != NULL);
		
		ALContext = alcCreateContext(ALDevice, attr);
		ASSERT(ALContext != NULL);
		
		alcMakeContextCurrent(ALContext);
	
		const char* ext=(const char*)alGetString(AL_EXTENSIONS);
		ASSERT(strstr(ext,"AL_SOFT_loop_points")!=NULL);
		if ( strstr(ext,"AL_SOFT_loop_points")==NULL )
		{
			curprovider=-1;
			release_existing();
			return false;
		}
		
		alListenerf (AL_GAIN,     1.0f);
		alListener3f(AL_POSITION, 0.0f, 0.0f, 0.0f);
		alListener3f(AL_VELOCITY, 0.0f, 0.0f, 0.0f);
		ALfloat orientation[6] = { 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f };
		alListenerfv(AL_ORIENTATION, orientation);
		
		alDistanceModel(AL_INVERSE_DISTANCE_CLAMPED);
		
		if ( alcIsExtensionPresent(ALDevice, (ALCchar*)ALC_EXT_EFX_NAME) )
		{
			alGenAuxiliaryEffectSlots(1, &ALEffectSlot);
			alGenEffects(1, &ALEffect);
		}
		
		for ( int32 i = 0; i < MAX_STREAMS; i++ )
		{
			alGenSources(1, &ALStreamSources[i]);
			alGenBuffers(NUM_STREAMBUFFERS, ALStreamBuffers[i]);
			
			CStream *stream = aStream[i];
			if (stream)
				stream->ProviderInit();
		}
		
		for ( int32 i = 0; i < SAMPLEBANK_MAX; i++ )
		{
			ALBuffers[i].Init();
		}
		
		alGenBuffers(MAX_PEDSFX,    pedBuffers);
		
		usingEAX = 0;
		usingEAX3 = 0;
		_usingEFX = false;
		
		if ( !strcmp(&providers[index].name[strlen(providers[index].name) - strlen(" EAX3")], " EAX3") 
				&& alcIsExtensionPresent(ALDevice, (ALCchar*)ALC_EXT_EFX_NAME) )
		{
			EAX_SetAll(&FinishEAX3);
			
			usingEAX = 1;
			usingEAX3 = 1;

			DEV("EAX3\n");
		}
		else if ( alcIsExtensionPresent(ALDevice, (ALCchar*)ALC_EXT_EFX_NAME) )
		{
			EAX_SetAll(&EAX30_ORIGINAL_PRESETS[EAX_ENVIRONMENT_CAVE]);
			
			if ( !strcmp(&providers[index].name[strlen(providers[index].name) - strlen(" EAX")], " EAX"))
			{
				usingEAX = 1;
				DEV("EAX1\n");
			}
			else
			{
				_usingEFX = true;
				DEV("EFX\n");
			}
		}
		
		//SampleManager.SetSpeakerConfig(speaker_type);
				
		for ( int32 i = 0; i < MAXCHANNELS; i++ )
			aChannel[i].Init();
		aChannel[CHANNEL2D].Init(true);
		
		if ( IsFXSupported() )
		{
			/**/
			alAuxiliaryEffectSloti(ALEffectSlot, AL_EFFECTSLOT_EFFECT, ALEffect);
			/**/
			
			for ( int32 i = 0; i < MAXCHANNELS; i++ )
				aChannel[i].SetReverbMix(ALEffectSlot, 0.0f);
		}
		
		return true;
	}
	
	return false;
}

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
	if ( _maxSamples > MAXCHANNELS )
		return MAXCHANNELS;
	
	return _maxSamples;
}

uint32 cSampleManager::GetNum3DProvidersAvailable()
{
	return m_nNumberOfProviders;
}

void cSampleManager::SetNum3DProvidersAvailable(uint32 num)
{
	m_nNumberOfProviders = num;
}

char *cSampleManager::Get3DProviderName(uint8 id)
{
	return m_aAudioProviders[id];
}

void cSampleManager::Set3DProviderName(uint8 id, char *name)
{
	m_aAudioProviders[id] = name;
}

int8 cSampleManager::GetCurrent3DProviderIndex(void)
{
	return curprovider;
}

int8 cSampleManager::SetCurrent3DProvider(uint8 nProvider)
{
	ASSERT( nProvider < m_nNumberOfProviders );
	int savedprovider = curprovider;
	
	if ( nProvider < m_nNumberOfProviders )
	{
		if ( set_new_provider(nProvider) )
			return curprovider;
		else if ( savedprovider != -1 && savedprovider < m_nNumberOfProviders && set_new_provider(savedprovider) )
			return curprovider;
		else
			return -1;
	}
	else
		return curprovider;
}

bool
cSampleManager::IsMP3RadioChannelAvailable(void)
{
	return nNumMP3s != 0;
}


void cSampleManager::ReleaseDigitalHandle(void)
{
	if ( ALDevice )
	{
		prevprovider = curprovider;
		release_existing();
		curprovider = -1;
	}
}

void cSampleManager::ReacquireDigitalHandle(void)
{
	if ( ALDevice )
	{
		if ( prevprovider != -1 )
			set_new_provider(prevprovider);
	}
}

bool
cSampleManager::Initialise(void)
{
	if ( _bSampmanInitialised )
		return true;

	EFXInit();
	CStream::Initialise();

	{
		for ( int32 i = 0; i < TOTAL_AUDIO_SAMPLES; i++ )
		{
			m_aSamples[i].nOffset    = 0;
			m_aSamples[i].nSize      = 0;
			m_aSamples[i].nFrequency = MAX_FREQ;
			m_aSamples[i].nLoopStart = 0;
			m_aSamples[i].nLoopEnd   = -1;
		}
		
		m_nEffectsVolume     = MAX_VOLUME;
		m_nMusicVolume       = MAX_VOLUME;
		m_nEffectsFadeVolume = MAX_VOLUME;
		m_nMusicFadeVolume   = MAX_VOLUME;
	
		m_nMonoMode = 0;
	}
	
	{
		curprovider = -1;
		prevprovider = -1;
			
		_usingEFX = false;
		usingEAX =0;
		usingEAX3=0;
			
		_fEffectsLevel = 0.0f;
			
		_maxSamples = 0;
		
		ALDevice = NULL;
		ALContext = NULL;
	}
	
	{
		fpSampleDescHandle = NULL;
		fpSampleDataHandle = NULL;
		
		for ( int32 i = 0; i < MAX_SAMPLEBANKS; i++ )
		{
			bSampleBankLoaded[i]             = false;
			nSampleBankDiscStartOffset[i]    = 0;
			nSampleBankSize[i]               = 0;
			nSampleBankMemoryStartAddress[i] = 0;
		}
	}
	
	{
		for ( int32 i = 0; i < MAX_PEDSFX; i++ )
		{
			nPedSlotSfx[i]     = NO_SAMPLE;
			nPedSlotSfxAddr[i] = 0;
		}
		
		nCurrentPedSlot = 0;
	}
	
	{
		for ( int32 i = 0; i < MAXCHANNELS+MAX2DCHANNELS; i++ )
			nChannelVolume[i] = 0;
	}
	
	{
		for ( int32 i = 0; i < MAX_STREAMS; i++ )
		{
			aStream[i]       = NULL;
			nStreamVolume[i] = 100;
			nStreamPan[i]    = 63;
		}
		
		for ( int32 i = 0; i < TOTAL_STREAMED_SOUNDS; i++ )
			nStreamLength[i] = 0;
	}
	
	{
		add_providers();
		
		if ( !InitialiseSampleBanks() )
		{
			Terminate();
			return false;
		}
		
		nSampleBankMemoryStartAddress[SAMPLEBANK_MAIN] = (uintptr)malloc(nSampleBankSize[SAMPLEBANK_MAIN]);
		ASSERT(nSampleBankMemoryStartAddress[SAMPLEBANK_MAIN] != NULL);
		
		if ( nSampleBankMemoryStartAddress[SAMPLEBANK_MAIN] == NULL )
		{
			Terminate();
			return false;
		}
		
		nSampleBankMemoryStartAddress[SAMPLEBANK_PED] = (uintptr)malloc(PED_BLOCKSIZE*MAX_PEDSFX);
		ASSERT(nSampleBankMemoryStartAddress[SAMPLEBANK_PED] != NULL);
	}
	
	
	{
		_bSampmanInitialised = true;
		
		if ( 0 >= defaultProvider && defaultProvider < m_nNumberOfProviders )
		{
			set_new_provider(defaultProvider);
		}
		else
		{
			Terminate();
			return false;
		}
	}
	
	{
	
		for ( int32 i = 0; i < TOTAL_STREAMED_SOUNDS; i++ )
		{	
			aStream[0] = new CStream(StreamedNameTable[i], ALStreamSources[0], ALStreamBuffers[0]);
			
			if ( aStream[0] && aStream[0]->IsOpened() )
			{
				uint32 tatalms = aStream[0]->GetLengthMS();
				delete aStream[0];
				aStream[0] = NULL;
				
				nStreamLength[i] = tatalms;
			}
			else
				USERERROR("Can't open '%s'\n", StreamedNameTable[i]);
		}
	}
		
	LoadSampleBank(SAMPLEBANK_MAIN);
	
	return true;
}

void
cSampleManager::Terminate(void)
{
	release_existing();

	for (int32 i = 0; i < MAX_STREAMS; i++)
	{
		CStream *stream = aStream[i];
		if (stream)
		{
			delete stream;
			aStream[i] = NULL;
		}
	}

	CStream::Terminate();
	
	if ( nSampleBankMemoryStartAddress[SAMPLEBANK_MAIN] != 0 )
	{
		free((void *)nSampleBankMemoryStartAddress[SAMPLEBANK_MAIN]);
		nSampleBankMemoryStartAddress[SAMPLEBANK_MAIN] = 0;
	}

	if ( nSampleBankMemoryStartAddress[SAMPLEBANK_PED] != 0 )
	{
		free((void *)nSampleBankMemoryStartAddress[SAMPLEBANK_PED]);
		nSampleBankMemoryStartAddress[SAMPLEBANK_PED] = 0;
	}
	
	_bSampmanInitialised = false;
}

void
cSampleManager::UpdateSoundBuffers(void)
{	
	for ( int32 i = 0; i < SAMPLEBANK_MAX; i++ )
	{
		ALBuffers[i].Update();
	}
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
	if ( _bSampmanInitialised )
	{
		for ( int32 i = 0; i < MAXCHANNELS+MAX2DCHANNELS; i++ )
		{
			if ( GetChannelUsedFlag(i) )
			{
				if ( nChannelVolume[i] != 0 )
					aChannel[i].SetVolume(m_nEffectsFadeVolume*nChannelVolume[i]*m_nEffectsVolume >> 14);
			}
		}
	}
}

void
cSampleManager::SetEffectsMasterVolume(uint8 nVolume)
{
	m_nEffectsVolume = nVolume;
	UpdateEffectsVolume();
}

void
cSampleManager::SetMusicMasterVolume(uint8 nVolume)
{
	m_nMusicVolume = nVolume;
}

void
cSampleManager::SetEffectsFadeVolume(uint8 nVolume)
{
	m_nEffectsFadeVolume = nVolume;
	UpdateEffectsVolume();
}

void
cSampleManager::SetMusicFadeVolume(uint8 nVolume)
{
	m_nMusicFadeVolume = nVolume;
}

void
cSampleManager::SetMonoMode(uint8 nMode)
{
	m_nMonoMode = nMode;
}

bool
cSampleManager::LoadSampleBank(uint8 nBank)
{
	ASSERT( nBank < MAX_SAMPLEBANKS );
	
	if ( CTimer::GetIsCodePaused() )
		return false;
	
	if ( MusicManager.IsInitialised()
		&& MusicManager.GetMusicMode() == MUSICMODE_CUTSCENE
		&& nBank != SAMPLEBANK_MAIN )
	{
		return false;
	}
	
	if ( fseek(fpSampleDataHandle, nSampleBankDiscStartOffset[nBank], SEEK_SET) != 0 )
		return false;
	
	if ( fread((void *)nSampleBankMemoryStartAddress[nBank], 1, nSampleBankSize[nBank], fpSampleDataHandle) != nSampleBankSize[nBank] )
		return false;
	
	bSampleBankLoaded[nBank] = true;
	
	return true;
}

void
cSampleManager::UnloadSampleBank(uint8 nBank)
{
	ASSERT( nBank < MAX_SAMPLEBANKS );
	
	bSampleBankLoaded[nBank] = false;
}

bool
cSampleManager::IsSampleBankLoaded(uint8 nBank)
{
	ASSERT( nBank < MAX_SAMPLEBANKS );
	
	return bSampleBankLoaded[nBank];
}

bool
cSampleManager::IsPedCommentLoaded(uint32 nComment)
{
	ASSERT( nComment < TOTAL_AUDIO_SAMPLES );
	
	int8 slot;

	for ( int32 i = 0; i < _TODOCONST(3); i++ )
	{
		slot = nCurrentPedSlot - i - 1;
#ifdef FIX_BUGS
		if (slot < 0)
			slot += ARRAY_SIZE(nPedSlotSfx);
#endif
		if ( nComment == nPedSlotSfx[slot] )
			return true;
	}
	
	return false;
}


int32
cSampleManager::_GetPedCommentSlot(uint32 nComment)
{
	int8 slot;

	for (int32 i = 0; i < _TODOCONST(3); i++)
	{
		slot = nCurrentPedSlot - i - 1;
#ifdef FIX_BUGS
		if (slot < 0)
			slot += ARRAY_SIZE(nPedSlotSfx);
#endif
		if (nComment == nPedSlotSfx[slot])
			return slot;
	}

	return -1;
}

bool
cSampleManager::LoadPedComment(uint32 nComment)
{
	ASSERT( nComment < TOTAL_AUDIO_SAMPLES );
	
	if ( CTimer::GetIsCodePaused() )
		return false;
	
	// no talking peds during cutsenes or the game end
	if ( MusicManager.IsInitialised() )
	{
		switch ( MusicManager.GetMusicMode() )
		{
			case MUSICMODE_CUTSCENE:
			{
				return false;

				break;
			}
			
			case MUSICMODE_FRONTEND:
			{
				if ( MusicManager.GetCurrentTrack() == STREAMED_SOUND_CUTSCENE_FINALE)
					return false;

				break;
			}
		}
	}
	
	if ( fseek(fpSampleDataHandle, m_aSamples[nComment].nOffset, SEEK_SET) != 0 )
		return false;
	
	if ( fread((void *)(nSampleBankMemoryStartAddress[SAMPLEBANK_PED] + PED_BLOCKSIZE*nCurrentPedSlot), 1, m_aSamples[nComment].nSize, fpSampleDataHandle) != m_aSamples[nComment].nSize )
		return false;
	
	nPedSlotSfx[nCurrentPedSlot] = nComment;
	
	alBufferData(pedBuffers[nCurrentPedSlot],
		AL_FORMAT_MONO16,
		(void *)(nSampleBankMemoryStartAddress[SAMPLEBANK_PED] + PED_BLOCKSIZE*nCurrentPedSlot),
		m_aSamples[nComment].nSize,
		m_aSamples[nComment].nFrequency);
	
	if ( ++nCurrentPedSlot >= MAX_PEDSFX )
		nCurrentPedSlot = 0;
	
	return true;
}

int32
cSampleManager::GetBankContainingSound(uint32 offset)
{
	if ( offset >= BankStartOffset[SAMPLEBANK_PED] )
		return SAMPLEBANK_PED;
	
	if ( offset >= BankStartOffset[SAMPLEBANK_MAIN] )
		return SAMPLEBANK_MAIN;
	
	return SAMPLEBANK_INVALID;
}

int32
cSampleManager::GetSampleBaseFrequency(uint32 nSample)
{
	ASSERT( nSample < TOTAL_AUDIO_SAMPLES );
	return m_aSamples[nSample].nFrequency;
}

int32
cSampleManager::GetSampleLoopStartOffset(uint32 nSample)
{
	ASSERT( nSample < TOTAL_AUDIO_SAMPLES );
	return m_aSamples[nSample].nLoopStart;
}

int32
cSampleManager::GetSampleLoopEndOffset(uint32 nSample)
{
	ASSERT( nSample < TOTAL_AUDIO_SAMPLES );
	return m_aSamples[nSample].nLoopEnd;
}

uint32
cSampleManager::GetSampleLength(uint32 nSample)
{
	ASSERT( nSample < TOTAL_AUDIO_SAMPLES );
	return m_aSamples[nSample].nSize / sizeof(uint16);
}

bool cSampleManager::UpdateReverb(void)
{
	if ( !usingEAX && !_usingEFX )
		return false;

	if ( AudioManager.GetFrameCounter() & 15 )
		return false;
			
	float y = AudioManager.GetReflectionsDistance(REFLECTION_TOP)  + AudioManager.GetReflectionsDistance(REFLECTION_BOTTOM);
	float x = AudioManager.GetReflectionsDistance(REFLECTION_LEFT) + AudioManager.GetReflectionsDistance(REFLECTION_RIGHT);
	float z = AudioManager.GetReflectionsDistance(REFLECTION_UP);
	
	float normy = norm(y, 5.0f, 40.0f);
	float normx = norm(x, 5.0f, 40.0f);
	float normz = norm(z, 5.0f, 40.0f);
	
	#define ZR(v, a, b) (((v)==0)?(a):(b))
	#define CALCRATIO(x,y,z,min,max,val) (ZR(y, ZR(x, ZR(z, min, max), min), ZR(x, ZR(z, min, max), ZR(z, min, val))))
	
	float fRatio = CALCRATIO(normx, normy, normz, 0.3f, 0.5f, (normy+normx+normz)/3.0f);
	
	#undef CALCRATIO
	#undef ZE
	
	fRatio = clamp(fRatio, usingEAX3==1 ? 0.0f : 0.30f, 1.0f);
	
	if ( fRatio == _fPrevEaxRatioDestination )
		return false;
	
#ifdef JUICY_OAL
	if ( usingEAX3 || _usingEFX )
#else
	if ( usingEAX3 )
#endif
	{
		if ( EAX3ListenerInterpolate(&StartEAX3, &FinishEAX3, fRatio, &EAX3Params, false) )
		{
			EAX_SetAll(&EAX3Params);
			
			/*
			if ( IsFXSupported() )
			{
				alAuxiliaryEffectSloti(ALEffectSlot, AL_EFFECTSLOT_EFFECT, ALEffect);
			
				for ( int32 i = 0; i < MAXCHANNELS; i++ )
					aChannel[i].UpdateReverb(ALEffectSlot);
			}
			*/
			
			_fEffectsLevel = 1.0f - fRatio * 0.5f;
		}
	}
	else
	{
		if ( _usingEFX )
			_fEffectsLevel = (1.0f - fRatio) * 0.4f;
		else
			_fEffectsLevel = (1.0f - fRatio) * 0.7f;
	}

	_fPrevEaxRatioDestination = fRatio;
	
	return true;
}

void
cSampleManager::SetChannelReverbFlag(uint32 nChannel, uint8 nReverbFlag)
{
	ASSERT( nChannel < MAXCHANNELS+MAX2DCHANNELS );
	
	if ( usingEAX || _usingEFX )
	{
		if ( IsFXSupported() )
		{
			alAuxiliaryEffectSloti(ALEffectSlot, AL_EFFECTSLOT_EFFECT, ALEffect);
			
			if ( nReverbFlag != 0 )
				aChannel[nChannel].SetReverbMix(ALEffectSlot, _fEffectsLevel);
			else
				aChannel[nChannel].SetReverbMix(ALEffectSlot, 0.0f);
		}
	}
}

bool
cSampleManager::InitialiseChannel(uint32 nChannel, uint32 nSfx, uint8 nBank)
{
	ASSERT( nChannel < MAXCHANNELS+MAX2DCHANNELS );
	
	ALuint buffer;
	
	if ( nSfx < SAMPLEBANK_MAX )
	{
		if ( !IsSampleBankLoaded(nBank) )
			return false;
		
		uintptr addr = nSampleBankMemoryStartAddress[nBank] + m_aSamples[nSfx].nOffset - m_aSamples[BankStartOffset[nBank]].nOffset;
	
		if ( ALBuffers[nSfx].IsEmpty() )
		{
			ALuint buf;
			alGenBuffers(1, &buf);
			alBufferData(buf, AL_FORMAT_MONO16, (void *)addr, m_aSamples[nSfx].nSize, m_aSamples[nSfx].nFrequency);
			ALBuffers[nSfx].Set(buf);
		}
		ALBuffers[nSfx].Wait();
		
		buffer = ALBuffers[nSfx].buffer;
	}
	else
	{
		if ( !IsPedCommentLoaded(nSfx) )
			return false;
		
		int32 slot = _GetPedCommentSlot(nSfx);
		
		buffer = pedBuffers[slot];
	}
	
	if ( buffer == 0 )
	{
		TRACE("No buffer to play id %d", nSfx);
		return false;
	}
	
	if ( GetChannelUsedFlag(nChannel) )
	{
		TRACE("Stopping channel %d - really!!!", nChannel);
		StopChannel(nChannel);
	}
	
	aChannel[nChannel].Reset();
	if ( aChannel[nChannel].HasSource() )
	{	
		aChannel[nChannel].SetSampleID     (nSfx);
		aChannel[nChannel].SetFreq         (m_aSamples[nSfx].nFrequency);
		aChannel[nChannel].SetLoopPoints   (0, -1);
		aChannel[nChannel].SetBuffer       (buffer);
		aChannel[nChannel].SetPitch        (1.0f);
		return true;
	}
	
	return false;
}

void
cSampleManager::SetChannelEmittingVolume(uint32 nChannel, uint32 nVolume)
{
	ASSERT( nChannel != CHANNEL2D );
	ASSERT( nChannel < MAXCHANNELS+MAX2DCHANNELS );
	
	uint32 vol = nVolume;
	if ( vol > MAX_VOLUME ) vol = MAX_VOLUME;
	
	nChannelVolume[nChannel] = vol;
	
	if (   MusicManager.GetMusicMode()    == MUSICMODE_CUTSCENE
		&& MusicManager.GetCurrentTrack() != STREAMED_SOUND_CUTSCENE_FINALE )
	{
		nChannelVolume[nChannel] = vol / 4;
	}

	// no idea, does this one looks like a bug or it's SetChannelVolume ?
	aChannel[nChannel].SetVolume(m_nEffectsFadeVolume*nChannelVolume[nChannel]*m_nEffectsVolume >> 14);
}

void
cSampleManager::SetChannel3DPosition(uint32 nChannel, float fX, float fY, float fZ)
{
	ASSERT( nChannel != CHANNEL2D );
	ASSERT( nChannel < MAXCHANNELS+MAX2DCHANNELS );
	
	aChannel[nChannel].SetPosition(-fX, fY, fZ);
}

void
cSampleManager::SetChannel3DDistances(uint32 nChannel, float fMax, float fMin)
{
	ASSERT( nChannel != CHANNEL2D );
	ASSERT( nChannel < MAXCHANNELS+MAX2DCHANNELS );
	aChannel[nChannel].SetDistances(fMax, fMin);
}

void
cSampleManager::SetChannelVolume(uint32 nChannel, uint32 nVolume)
{
	ASSERT( nChannel == CHANNEL2D );
	ASSERT( nChannel < MAXCHANNELS+MAX2DCHANNELS );
	
	if ( nChannel == CHANNEL2D )
	{
		uint32 vol = nVolume;
		if ( vol > MAX_VOLUME ) vol = MAX_VOLUME;
		
		nChannelVolume[nChannel] = vol;
		
		if (   MusicManager.GetMusicMode()    == MUSICMODE_CUTSCENE
			&& MusicManager.GetCurrentTrack() != STREAMED_SOUND_CUTSCENE_FINALE )
		{
			nChannelVolume[nChannel] = vol / 4;
		}
			
		aChannel[nChannel].SetVolume(m_nEffectsFadeVolume*vol*m_nEffectsVolume >> 14);
	}
}

void
cSampleManager::SetChannelPan(uint32 nChannel, uint32 nPan)
{
	ASSERT(nChannel == CHANNEL2D);
	ASSERT( nChannel < MAXCHANNELS+MAX2DCHANNELS );
	
	if ( nChannel == CHANNEL2D )
	{
		aChannel[nChannel].SetPan(nPan);
	}
}

void
cSampleManager::SetChannelFrequency(uint32 nChannel, uint32 nFreq)
{
	ASSERT( nChannel < MAXCHANNELS+MAX2DCHANNELS );
	
	aChannel[nChannel].SetCurrentFreq(nFreq);
}

void
cSampleManager::SetChannelLoopPoints(uint32 nChannel, uint32 nLoopStart, int32 nLoopEnd)
{
	ASSERT( nChannel < MAXCHANNELS+MAX2DCHANNELS );
	
	aChannel[nChannel].SetLoopPoints(nLoopStart / (DIGITALBITS / 8), nLoopEnd / (DIGITALBITS / 8));
}

void
cSampleManager::SetChannelLoopCount(uint32 nChannel, uint32 nLoopCount)
{
	ASSERT( nChannel < MAXCHANNELS+MAX2DCHANNELS );
	
	aChannel[nChannel].SetLoopCount(nLoopCount);
}

bool
cSampleManager::GetChannelUsedFlag(uint32 nChannel)
{
	ASSERT( nChannel < MAXCHANNELS+MAX2DCHANNELS );
	
	return aChannel[nChannel].IsUsed();
}

void
cSampleManager::StartChannel(uint32 nChannel)
{
	ASSERT( nChannel < MAXCHANNELS+MAX2DCHANNELS );
	
	aChannel[nChannel].Start();
}

void
cSampleManager::StopChannel(uint32 nChannel)
{
	ASSERT( nChannel < MAXCHANNELS+MAX2DCHANNELS );
	
	aChannel[nChannel].Stop();
}

void
cSampleManager::PreloadStreamedFile(uint8 nFile, uint8 nStream)
{
	char filename[256];
	
	ASSERT( nStream < MAX_STREAMS );

	if ( nFile < TOTAL_STREAMED_SOUNDS )
	{
		if ( aStream[nStream] )
		{
			delete aStream[nStream];
			aStream[nStream] = NULL;
		}
		
		strcpy(filename, StreamedNameTable[nFile]);
		
		CStream *stream = new CStream(filename, ALStreamSources[nStream], ALStreamBuffers[nStream]);
		ASSERT(stream != NULL);
		
		aStream[nStream] = stream;
		if ( !stream->IsOpened() )
		{
			delete stream;
			aStream[nStream] = NULL;
		}
	}
}

void
cSampleManager::PauseStream(uint8 nPauseFlag, uint8 nStream)
{
	ASSERT( nStream < MAX_STREAMS );
	
	CStream *stream = aStream[nStream];
	
	if ( stream )
	{
		stream->SetPause(nPauseFlag != 0);
	}
}

void
cSampleManager::StartPreloadedStreamedFile(uint8 nStream)
{
	ASSERT( nStream < MAX_STREAMS );
	
	CStream *stream = aStream[nStream];
	
	if ( stream )
	{
		if ( stream->Setup() )
		{
			stream->Start();
		}
	}
}

bool
cSampleManager::StartStreamedFile(uint8 nFile, uint32 nPos, uint8 nStream)
{
	char filename[256];
	
	ASSERT( nStream < MAX_STREAMS );
	
	if ( nFile < TOTAL_STREAMED_SOUNDS )
	{
		if ( aStream[nStream] )
		{
			delete aStream[nStream];
			aStream[nStream] = NULL;
		}
		
		strcpy(filename, StreamedNameTable[nFile]);
		
		CStream *stream = new CStream(filename, ALStreamSources[nStream], ALStreamBuffers[nStream]);
		ASSERT(stream != NULL);

		aStream[nStream] = stream;
		
		if ( stream->IsOpened() )
		{
			nStreamLength[nFile] = stream->GetLengthMS();
			if ( stream->Setup() )
			{
				if ( nPos != 0 )
					stream->SetPosMS(nPos);
				
				stream->Start();
			}
			
			return true;
		}
		else
		{
			delete stream;
			aStream[nStream] = NULL;
		}
	}
	
	return false;
}

void
cSampleManager::StopStreamedFile(uint8 nStream)
{
	ASSERT( nStream < MAX_STREAMS );

	CStream *stream = aStream[nStream];
	
	if ( stream )
	{
		delete stream;
		aStream[nStream] = NULL;
	}
}

int32
cSampleManager::GetStreamedFilePosition(uint8 nStream)
{
	ASSERT( nStream < MAX_STREAMS );
	
	CStream *stream = aStream[nStream];
	
	if ( stream )
	{
		return stream->GetPosMS();
	}
	
	return 0;
}

void
cSampleManager::SetStreamedVolumeAndPan(uint8 nVolume, uint8 nPan, uint8 nEffectFlag, uint8 nStream)
{
	ASSERT( nStream < MAX_STREAMS );
	
	if ( nVolume > MAX_VOLUME )
		nVolume = MAX_VOLUME;
	
	if ( nPan > MAX_VOLUME )
		nPan = MAX_VOLUME;
		
	nStreamVolume[nStream] = nVolume;
	nStreamPan   [nStream] = nPan;
	
	CStream *stream = aStream[nStream];
	
	if ( stream )
	{
		if ( nEffectFlag )
			stream->SetVolume(m_nEffectsFadeVolume*nVolume*m_nEffectsVolume >> 14);
		else
			stream->SetVolume(m_nMusicFadeVolume*nVolume*m_nMusicVolume >> 14);
		
		stream->SetPan(nPan);
	}
}

int32
cSampleManager::GetStreamedFileLength(uint8 nStream)
{
	ASSERT( nStream < TOTAL_STREAMED_SOUNDS );

	return nStreamLength[nStream];
}

bool
cSampleManager::IsStreamPlaying(uint8 nStream)
{
	ASSERT( nStream < MAX_STREAMS );
	
	CStream *stream = aStream[nStream];
	
	if ( stream )
	{
		if ( stream->IsPlaying() )
			return true;
	}
	
	return false;
}

void
cSampleManager::Service(void)
{
	for ( int32 i = 0; i < MAX_STREAMS; i++ )
	{
		CStream *stream = aStream[i];
		
		if ( stream )
			stream->Update();
	}
	
	UpdateSoundBuffers();
}

bool
cSampleManager::InitialiseSampleBanks(void)
{
	int32 nBank = SAMPLEBANK_MAIN;
	
	fpSampleDescHandle = fopen(SampleBankDescFilename, "rb");
	if ( fpSampleDescHandle == NULL )
		return false;
	
	fpSampleDataHandle = fopen(SampleBankDataFilename, "rb");
	if ( fpSampleDataHandle == NULL )
	{
		fclose(fpSampleDescHandle);
		fpSampleDescHandle = NULL;
		
		return false;
	}
	
	fseek(fpSampleDataHandle, 0, SEEK_END);
	int32 _nSampleDataEndOffset = ftell(fpSampleDataHandle);
	rewind(fpSampleDataHandle);
	
	fread(m_aSamples, sizeof(tSample), TOTAL_AUDIO_SAMPLES, fpSampleDescHandle);
	
	fclose(fpSampleDescHandle);
	fpSampleDescHandle = NULL;
	
	for ( int32 i = 0; i < TOTAL_AUDIO_SAMPLES; i++ )
	{
#ifdef FIX_BUGS
		if (nBank >= MAX_SAMPLEBANKS) break;
#endif
		if ( BankStartOffset[nBank] == BankStartOffset[SAMPLEBANK_MAIN] + i )
		{
			nSampleBankDiscStartOffset[nBank] = m_aSamples[i].nOffset;
			nBank++;
		}
	}

	nSampleBankSize[SAMPLEBANK_MAIN] = nSampleBankDiscStartOffset[SAMPLEBANK_PED] - nSampleBankDiscStartOffset[SAMPLEBANK_MAIN];
	nSampleBankSize[SAMPLEBANK_PED]  = _nSampleDataEndOffset                      - nSampleBankDiscStartOffset[SAMPLEBANK_PED];
	
	return true;
}

#endif
