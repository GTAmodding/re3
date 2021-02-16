//#define JUICY_OAL

#ifdef AUDIO_OAL
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

// for user MP3s
#include <direct.h>
#include <shlobj.h>
#include <shlguid.h>
#else
#define _getcwd getcwd
#endif

#if defined _MSC_VER && !defined CMAKE_NO_AUTOLINK
#pragma comment( lib, "OpenAL32.lib" )
#endif

#include "common.h"
#include "crossplatform.h"

#include "sampman.h"

#include "oal/oal_utils.h"
#include "oal/aldlist.h"
#include "oal/channel.h"
#include "oal/stream.h"

#include "AudioManager.h"
#include "MusicManager.h"
#include "Frontend.h"
#include "Timer.h"
#ifdef AUDIO_OAL_USE_OPUS
#include <opusfile.h>
#endif

//TODO: fix eax3 reverb
//TODO: max channels

cSampleManager SampleManager;
bool _bSampmanInitialised = false;

uint32 BankStartOffset[MAX_SFX_BANKS];

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
#ifdef OPUS_SFX
OggOpusFile *fpSampleDataHandle;
#else
FILE *fpSampleDataHandle;
#endif
bool  bSampleBankLoaded            [MAX_SFX_BANKS];
int32 nSampleBankDiscStartOffset   [MAX_SFX_BANKS];
int32 nSampleBankSize              [MAX_SFX_BANKS];
uintptr nSampleBankMemoryStartAddress[MAX_SFX_BANKS];
int32 _nSampleDataEndOffset;

int32 nPedSlotSfx    [MAX_PEDSFX];
int32 nPedSlotSfxAddr[MAX_PEDSFX];
uint8 nCurrentPedSlot;

CChannel aChannel[MAXCHANNELS+MAX2DCHANNELS];
uint8 nChannelVolume[MAXCHANNELS+MAX2DCHANNELS];

uint32 nStreamLength[TOTAL_STREAMED_SOUNDS];
ALuint ALStreamSources[MAX_STREAMS][2];
ALuint ALStreamBuffers[MAX_STREAMS][NUM_STREAMBUFFERS];

struct tMP3Entry
{
	char aFilename[MAX_PATH];

	uint32 nTrackLength;
	uint32 nTrackStreamPos;

	tMP3Entry* pNext;
	char* pLinkPath;
};

uint32 nNumMP3s;
tMP3Entry* _pMP3List;
char _mp3DirectoryPath[MAX_PATH]; 
CStream    *aStream[MAX_STREAMS];
uint8      nStreamPan   [MAX_STREAMS];
uint8      nStreamVolume[MAX_STREAMS];
uint32 _CurMP3Index;
int32 _CurMP3Pos;
bool _bIsMp3Active;
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
		
		alDeleteBuffers(NUM_STREAMBUFFERS, ALStreamBuffers[i]);
	}
	alDeleteSources(MAX_STREAMS*2, ALStreamSources[0]);
	
	CChannel::DestroyChannels();
	
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
		
		ALCint attr[] = {ALC_FREQUENCY,MAX_FREQ,
						ALC_MONO_SOURCES, MAX_STREAMS * 2 + MAXCHANNELS,
						0,
						};
		
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

		alGenSources(MAX_STREAMS*2, ALStreamSources[0]);
		for ( int32 i = 0; i < MAX_STREAMS; i++ )
		{
			alGenBuffers(NUM_STREAMBUFFERS, ALStreamBuffers[i]); 
			alSourcei(ALStreamSources[i][0], AL_SOURCE_RELATIVE, AL_TRUE);
			alSource3f(ALStreamSources[i][0], AL_POSITION, 0.0f, 0.0f, 0.0f);
			alSourcef(ALStreamSources[i][0], AL_GAIN, 1.0f);
			alSourcei(ALStreamSources[i][1], AL_SOURCE_RELATIVE, AL_TRUE);
			alSource3f(ALStreamSources[i][1], AL_POSITION, 0.0f, 0.0f, 0.0f);
			alSourcef(ALStreamSources[i][1], AL_GAIN, 1.0f);
			
			CStream *stream = aStream[i];
			if (stream)
				stream->ProviderInit();
		}
		
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
		
		CChannel::InitChannels();

		for ( int32 i = 0; i < MAXCHANNELS; i++ )
			aChannel[i].Init(i);
		aChannel[CHANNEL2D].Init(CHANNEL2D, true);
		
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

static bool
IsThisTrackAt16KHz(uint32 track)
{
	return track == STREAMED_SOUND_RADIO_CHAT;
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
	int savedprovider = curprovider;

	nProvider = clamp(nProvider, 0, m_nNumberOfProviders - 1);

	if ( set_new_provider(nProvider) )
		return curprovider;
	else if ( savedprovider != -1 && savedprovider < m_nNumberOfProviders && set_new_provider(savedprovider) )
		return curprovider;
	else
		return curprovider;
}

static bool
_ResolveLink(char const *path, char *out)
{
#ifdef _WIN32
	size_t len = strlen(path);
	if (len < 4 || strcmp(&path[len - 4], ".lnk") != 0)
		return false;
		
	IShellLink* psl;
	WIN32_FIND_DATA fd;
	char filepath[MAX_PATH];
	
	CoInitialize(NULL);
									   
	if (SUCCEEDED( CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID*)&psl ) ))
	{
		IPersistFile *ppf;

		if (SUCCEEDED(psl->QueryInterface(IID_IPersistFile, (LPVOID*)&ppf)))
		{
			WCHAR wpath[MAX_PATH];
			
			MultiByteToWideChar(CP_ACP, 0, path, -1, wpath, MAX_PATH);
			
			if (SUCCEEDED(ppf->Load(wpath, STGM_READ)))
			{
				/* Resolve the link */
				if (SUCCEEDED(psl->Resolve(NULL, SLR_ANY_MATCH|SLR_NO_UI|SLR_NOSEARCH)))
				{
					strcpy(filepath, path);
					
					if (SUCCEEDED(psl->GetPath(filepath, MAX_PATH, &fd, SLGP_UNCPRIORITY)))
					{
						OutputDebugString(fd.cFileName);
						
						strcpy(out, filepath);
						// FIX: Release the objects. Taken from SA.
#ifdef FIX_BUGS
						ppf->Release();
						psl->Release();
#endif
						return true;
					}
				}
			}
			
			ppf->Release();
		}
		psl->Release();
	}
	
	return false;
#else
	struct stat sb;

	if (lstat(path, &sb) == -1) {
		perror("lstat: ");
		return false;
	}

	if (S_ISLNK(sb.st_mode)) {
		char* linkname = (char*)alloca(sb.st_size + 1);
		if (linkname == NULL) {
			fprintf(stderr, "insufficient memory\n");
			return false;
		}

		if (readlink(path, linkname, sb.st_size + 1) < 0) {
			perror("readlink: ");
			return false;
		}
		linkname[sb.st_size] = '\0';
		strcpy(out, linkname);
		return true;
	} else {
		return false;
	}
#endif
}

static void
_FindMP3s(void)
{
	tMP3Entry *pList;
	bool bShortcut;	
	bool bInitFirstEntry;	
	HANDLE hFind;
	char path[MAX_PATH];
	char filepath[MAX_PATH*2];
	int total_ms;
	WIN32_FIND_DATA fd;
	
	if (getcwd(_mp3DirectoryPath, MAX_PATH) == NULL) {
		perror("getcwd: ");
		return;
	}
	
	OutputDebugString("Finding MP3s...");
	strcpy(path, _mp3DirectoryPath);
	strcat(path, "\\MP3\\");
	
	strcpy(_mp3DirectoryPath, path);
	OutputDebugString(_mp3DirectoryPath);
	
	strcat(path, "*");
	
	hFind = FindFirstFile(path, &fd);
	
	if ( hFind == INVALID_HANDLE_VALUE ) 
	{
		return;
	}
	
	strcpy(filepath, _mp3DirectoryPath);
	strcat(filepath, fd.cFileName);
	
	size_t filepathlen = strlen(filepath);
	
	if ( filepathlen <= 0)
	{
		FindClose(hFind);
		return;
	}

	if ( _ResolveLink(filepath, filepath) )
	{
		OutputDebugString("Resolving Link");
		OutputDebugString(filepath);
		bShortcut = true;
	} else
		bShortcut = false;
	
	aStream[0] = new CStream(filepath, ALStreamSources[0], ALStreamBuffers[0]);

	if (aStream[0] && aStream[0]->IsOpened())
	{
		total_ms = aStream[0]->GetLengthMS();
		delete aStream[0];
		aStream[0] = NULL;

		OutputDebugString(fd.cFileName);
		
		_pMP3List = new tMP3Entry;
		
		if ( _pMP3List == NULL )
		{
			FindClose(hFind);
			return;
		}
		
		nNumMP3s = 1;
		
		strcpy(_pMP3List->aFilename, fd.cFileName);
		
		_pMP3List->nTrackLength = total_ms;
		
		_pMP3List->pNext = NULL;
		
		pList = _pMP3List;
		
		if ( bShortcut )
		{
			_pMP3List->pLinkPath = new char[MAX_PATH*2];
			strcpy(_pMP3List->pLinkPath, filepath);
		}
		else
		{
			_pMP3List->pLinkPath = NULL;
		}

		bInitFirstEntry = false;
	}
	else
	{
		strcat(filepath, " - NOT A VALID MP3");
		
		OutputDebugString(filepath);

		bInitFirstEntry = true;
	}
	
	while ( true )
	{
		if ( !FindNextFile(hFind, &fd) )
			break;
		
		if ( bInitFirstEntry )
		{
			strcpy(filepath, _mp3DirectoryPath);
			strcat(filepath, fd.cFileName);
			
			size_t filepathlen = strlen(filepath);

			if ( filepathlen > 0 )
			{
				if ( _ResolveLink(filepath, filepath) )
				{
					OutputDebugString("Resolving Link");
					OutputDebugString(filepath);
					bShortcut = true;
				} else {
					bShortcut = false;
					if (filepathlen > MAX_PATH) {
						continue;
					}
				}
				aStream[0] = new CStream(filepath, ALStreamSources[0], ALStreamBuffers[0]);

				if (aStream[0] && aStream[0]->IsOpened())
				{
					total_ms = aStream[0]->GetLengthMS();
					delete aStream[0];
					aStream[0] = NULL;
					
					OutputDebugString(fd.cFileName);
					
					_pMP3List = new tMP3Entry;
					
					if ( _pMP3List  == NULL)
						break;
					
					nNumMP3s = 1;
					
					strcpy(_pMP3List->aFilename, fd.cFileName);
					
					_pMP3List->nTrackLength = total_ms;
					_pMP3List->pNext = NULL;
					
					if ( bShortcut )
					{
						_pMP3List->pLinkPath = new char [MAX_PATH*2];
						strcpy(_pMP3List->pLinkPath, filepath);
					}
					else
					{
						_pMP3List->pLinkPath = NULL;
					}
					
					pList = _pMP3List;

					bInitFirstEntry = false;
				}
				else
				{
					strcat(filepath, " - NOT A VALID MP3");
					OutputDebugString(filepath);
				}
			}
		}
		else
		{
			strcpy(filepath, _mp3DirectoryPath);
			strcat(filepath, fd.cFileName);
			
			size_t filepathlen = strlen(filepath);
			
			if ( filepathlen > 0 )
			{
				if ( _ResolveLink(filepath, filepath) )
				{
					OutputDebugString("Resolving Link");
					OutputDebugString(filepath);
					bShortcut = true;
				} else
					bShortcut = false;
				
				aStream[0] = new CStream(filepath, ALStreamSources[0], ALStreamBuffers[0]);

				if (aStream[0] && aStream[0]->IsOpened())
				{
					total_ms = aStream[0]->GetLengthMS();
					delete aStream[0];
					aStream[0] = NULL;

					OutputDebugString(fd.cFileName);
					
					pList->pNext = new tMP3Entry;
					
					tMP3Entry *e = pList->pNext;
					
					if ( e == NULL )
						break;
					
					pList = pList->pNext;
					
					strcpy(e->aFilename, fd.cFileName);
					e->nTrackLength = total_ms;
					e->pNext = NULL;
					
					if ( bShortcut )
					{
						e->pLinkPath = new char [MAX_PATH*2];
						strcpy(e->pLinkPath, filepath);
					}
					else
					{
						e->pLinkPath = NULL;
					}
					
					nNumMP3s++;
					
					OutputDebugString(fd.cFileName);
				}
				else
				{
					strcat(filepath, " - NOT A VALID MP3");
					OutputDebugString(filepath);
				}
			}
		}
	}

	FindClose(hFind);
}

static void
_DeleteMP3Entries(void)
{
	tMP3Entry *e = _pMP3List;

	while ( e != NULL )
	{
		tMP3Entry *next = e->pNext;
		
		if ( next == NULL )
			next = NULL;
		
		if ( e->pLinkPath != NULL )
		{
#ifndef FIX_BUGS
			delete   e->pLinkPath; // BUG: should be delete []
#else
			delete[] e->pLinkPath;
#endif
			e->pLinkPath = NULL;
		}
		
		delete e;
		
		if ( next )
			e = next;
		else
			e = NULL;
		
		nNumMP3s--;
	}
	
	
	if ( nNumMP3s != 0 )
	{
		OutputDebugString("Not all MP3 entries were deleted");
		nNumMP3s = 0;
	}
	
	_pMP3List = NULL;
}

static tMP3Entry *
_GetMP3EntryByIndex(uint32 idx)
{
	uint32 n = ( idx < nNumMP3s ) ? idx : 0;
	
	if ( _pMP3List != NULL )
	{
		tMP3Entry *e = _pMP3List;
		
		for ( uint32 i = 0; i < n; i++ )
			e = e->pNext;
		
		return e;
			
	}
	
	return NULL;
}

static inline bool
_GetMP3PosFromStreamPos(uint32 *pPosition, tMP3Entry **pEntry)
{
	_CurMP3Index = 0;
	
	for ( *pEntry = _pMP3List; *pEntry != NULL; *pEntry = (*pEntry)->pNext )
	{
		if (   *pPosition >= (*pEntry)->nTrackStreamPos
			&& *pPosition <  (*pEntry)->nTrackLength + (*pEntry)->nTrackStreamPos )
		{
			*pPosition -= (*pEntry)->nTrackStreamPos;
			_CurMP3Pos = *pPosition;
			
			return true;
		}
		
		_CurMP3Index++;
	}
				
	*pPosition = 0;
	*pEntry = _pMP3List;
	_CurMP3Pos = 0;
	_CurMP3Index = 0;
	
	return false;
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
		
		for ( int32 i = 0; i < MAX_SFX_BANKS; i++ )
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
		for ( int32 i = 0; i < TOTAL_STREAMED_SOUNDS; i++ )
			nStreamLength[i] = 0;
	}
	
		add_providers();

#ifdef AUDIO_CACHE
	FILE *cacheFile = fcaseopen("audio\\sound.cache", "rb");
	if (cacheFile) {
		debug("Loadind audio cache (If game crashes around here, then your cache is corrupted, remove audio/sound.cache)\n");
		fread(nStreamLength, sizeof(uint32), TOTAL_STREAMED_SOUNDS, cacheFile);
		fclose(cacheFile);
	} else
	{
		debug("Cannot load audio cache\n");
#endif

		for(int32 i = 0; i < TOTAL_STREAMED_SOUNDS; i++) {
			aStream[0] = new CStream(StreamedNameTable[i], ALStreamSources[0], ALStreamBuffers[0], IsThisTrackAt16KHz(i) ? 16000 : 32000);

			if(aStream[0] && aStream[0]->IsOpened()) {
				uint32 tatalms = aStream[0]->GetLengthMS();
				delete aStream[0];
				aStream[0] = NULL;

				nStreamLength[i] = tatalms;
			} else
				USERERROR("Can't open '%s'\n", StreamedNameTable[i]);
		}
#ifdef AUDIO_CACHE
		cacheFile = fcaseopen("audio\\sound.cache", "wb");
		if(cacheFile) {
			debug("Saving audio cache\n");
			fwrite(nStreamLength, sizeof(uint32), TOTAL_STREAMED_SOUNDS, cacheFile);
			fclose(cacheFile);
		} else {
			debug("Cannot save audio cache\n");
		}
	}
#endif

	{
		if ( !InitialiseSampleBanks() )
		{
			Terminate();
			return false;
		}
		
		nSampleBankMemoryStartAddress[SFX_BANK_0] = (uintptr)malloc(nSampleBankSize[SFX_BANK_0]);
		ASSERT(nSampleBankMemoryStartAddress[SFX_BANK_0] != 0);
		
		if ( nSampleBankMemoryStartAddress[SFX_BANK_0] == 0 )
		{
			Terminate();
			return false;
		}
		
		nSampleBankMemoryStartAddress[SFX_BANK_PED_COMMENTS] = (uintptr)malloc(PED_BLOCKSIZE*MAX_PEDSFX);
		ASSERT(nSampleBankMemoryStartAddress[SFX_BANK_PED_COMMENTS] != 0);
	
		LoadSampleBank(SFX_BANK_0);
	}
	
	{
		for ( int32 i = 0; i < MAX_STREAMS; i++ )
		{
			aStream[i]       = NULL;
			nStreamVolume[i] = 100;
			nStreamPan[i]    = 63;
		}
	}
	
	{
		_bSampmanInitialised = true;
		
		if ( defaultProvider >= 0 && defaultProvider < m_nNumberOfProviders )
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
		nNumMP3s = 0;
		
		_pMP3List = NULL;
		
		_FindMP3s();
		
		if ( nNumMP3s != 0 )
		{
			nStreamLength[STREAMED_SOUND_RADIO_MP3_PLAYER] = 0;
			
			for ( tMP3Entry *e = _pMP3List; e != NULL; e = e->pNext )
			{
				e->nTrackStreamPos = nStreamLength[STREAMED_SOUND_RADIO_MP3_PLAYER];
				nStreamLength[STREAMED_SOUND_RADIO_MP3_PLAYER] += e->nTrackLength;
			}
			
			time_t t = time(NULL);
			tm *localtm;
			bool bUseRandomTable;
			
			if ( t == -1 )
				bUseRandomTable = true;
			else
			{
				bUseRandomTable = false;
				localtm = localtime(&t);
			}
			
			int32 randval;
			if ( bUseRandomTable )
				randval = AudioManager.GetRandomNumber(1);
			else
				randval = localtm->tm_sec * localtm->tm_min;
			
			_CurMP3Index = randval % nNumMP3s;
			
			tMP3Entry *randmp3 = _pMP3List;
			for ( int32 i = randval % nNumMP3s; i > 0; --i)
				randmp3 = randmp3->pNext;
			
			if ( bUseRandomTable )
				_CurMP3Pos = AudioManager.GetRandomNumber(0)     % randmp3->nTrackLength;
			else
			{
				if ( localtm->tm_sec > 0 )
				{
					int32 s = localtm->tm_sec;
					_CurMP3Pos = s*s*s*s*s*s*s*s                 % randmp3->nTrackLength;
				}
				else
					_CurMP3Pos = AudioManager.GetRandomNumber(0) % randmp3->nTrackLength;
			}
		}
		else
			_CurMP3Pos = 0;
		
		_bIsMp3Active = false;
	}
	
	return true;
}

void
cSampleManager::Terminate(void)
{
	for (int32 i = 0; i < MAX_STREAMS; i++)
	{
		CStream *stream = aStream[i];
		if (stream)
		{
			delete stream;
			aStream[i] = NULL;
		}
	}

	release_existing();

	_DeleteMP3Entries();

	CStream::Terminate();

	if ( nSampleBankMemoryStartAddress[SFX_BANK_0] != 0 )
	{
		free((void *)nSampleBankMemoryStartAddress[SFX_BANK_0]);
		nSampleBankMemoryStartAddress[SFX_BANK_0] = 0;
	}

	if ( nSampleBankMemoryStartAddress[SFX_BANK_PED_COMMENTS] != 0 )
	{
		free((void *)nSampleBankMemoryStartAddress[SFX_BANK_PED_COMMENTS]);
		nSampleBankMemoryStartAddress[SFX_BANK_PED_COMMENTS] = 0;
	}
	
	_bSampmanInitialised = false;
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
	ASSERT( nBank < MAX_SFX_BANKS);
	
	if ( CTimer::GetIsCodePaused() )
		return false;
	
	if ( MusicManager.IsInitialised()
		&& MusicManager.GetMusicMode() == MUSICMODE_CUTSCENE
		&& nBank != SFX_BANK_0 )
	{
		return false;
	}
	
#ifdef OPUS_SFX
	int samplesRead = 0;
	int samplesSize = nSampleBankSize[nBank] / 2;
	op_pcm_seek(fpSampleDataHandle, 0);
	while (samplesSize > 0) {
		int size = op_read(fpSampleDataHandle, (opus_int16 *)(nSampleBankMemoryStartAddress[nBank] + samplesRead), samplesSize, NULL);
		if (size <= 0) {
			// huh?
			//assert(0);
			break;
		}
		samplesRead += size*2;
		samplesSize -= size;
	}
#else
	if ( fseek(fpSampleDataHandle, nSampleBankDiscStartOffset[nBank], SEEK_SET) != 0 )
		return false;
	
	if ( fread((void *)nSampleBankMemoryStartAddress[nBank], 1, nSampleBankSize[nBank], fpSampleDataHandle) != nSampleBankSize[nBank] )
		return false;
#endif
	bSampleBankLoaded[nBank] = true;
	
	return true;
}

void
cSampleManager::UnloadSampleBank(uint8 nBank)
{
	ASSERT( nBank < MAX_SFX_BANKS);
	
	bSampleBankLoaded[nBank] = false;
}

bool
cSampleManager::IsSampleBankLoaded(uint8 nBank)
{
	ASSERT( nBank < MAX_SFX_BANKS);
	
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
				if ( MusicManager.GetNextTrack() == STREAMED_SOUND_GAME_COMPLETED )
					return false;

				break;
			}
		}
	}

#ifdef OPUS_SFX
	int samplesRead = 0;
	int samplesSize = m_aSamples[nComment].nSize / 2;
	op_pcm_seek(fpSampleDataHandle, m_aSamples[nComment].nOffset / 2);
	while (samplesSize > 0) {
		int size = op_read(fpSampleDataHandle, (opus_int16 *)(nSampleBankMemoryStartAddress[SFX_BANK_PED_COMMENTS] + PED_BLOCKSIZE * nCurrentPedSlot + samplesRead),
		                   samplesSize, NULL);
		if (size <= 0) {
			return false;
		}
		samplesRead += size * 2;
		samplesSize -= size;
	}
#else
	if ( fseek(fpSampleDataHandle, m_aSamples[nComment].nOffset, SEEK_SET) != 0 )
		return false;
	
	if ( fread((void *)(nSampleBankMemoryStartAddress[SFX_BANK_PED_COMMENTS] + PED_BLOCKSIZE*nCurrentPedSlot), 1, m_aSamples[nComment].nSize, fpSampleDataHandle) != m_aSamples[nComment].nSize )
		return false;

#endif
	nPedSlotSfx[nCurrentPedSlot] = nComment;
		
	if ( ++nCurrentPedSlot >= MAX_PEDSFX )
		nCurrentPedSlot = 0;
	
	return true;
}

int32
cSampleManager::GetBankContainingSound(uint32 offset)
{
	if ( offset >= BankStartOffset[SFX_BANK_PED_COMMENTS] )
		return SFX_BANK_PED_COMMENTS;
	
	if ( offset >= BankStartOffset[SFX_BANK_0] )
		return SFX_BANK_0;
	
	return INVALID_SFX_BANK;
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
	
	uintptr addr;
	
	if ( nSfx < SAMPLEBANK_MAX )
	{
		if ( !IsSampleBankLoaded(nBank) )
			return false;
		
		addr = nSampleBankMemoryStartAddress[nBank] + m_aSamples[nSfx].nOffset - m_aSamples[BankStartOffset[nBank]].nOffset;
	}
	else
	{
		if ( !IsPedCommentLoaded(nSfx) )
			return false;
		
		int32 slot = _GetPedCommentSlot(nSfx);
		addr = (nSampleBankMemoryStartAddress[SFX_BANK_PED_COMMENTS] + PED_BLOCKSIZE * slot);
	}
	
	if ( GetChannelUsedFlag(nChannel) )
	{
		TRACE("Stopping channel %d - really!!!", nChannel);
		StopChannel(nChannel);
	}
	
	aChannel[nChannel].Reset();
	if ( aChannel[nChannel].HasSource() )
	{	
		aChannel[nChannel].SetSampleData   ((void*)addr, m_aSamples[nSfx].nSize, m_aSamples[nSfx].nFrequency);
		aChannel[nChannel].SetLoopPoints   (0, -1);
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
	
	// reduce channel volume when JB.MP3 or S4_BDBD.MP3 playing
	if (   MusicManager.GetMusicMode()    == MUSICMODE_CUTSCENE
		&& MusicManager.GetNextTrack() != STREAMED_SOUND_NEWS_INTRO
		&& MusicManager.GetNextTrack() != STREAMED_SOUND_CUTSCENE_SAL4_BDBD )
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
		
		// reduce the volume for JB.MP3 and S4_BDBD.MP3
		if (   MusicManager.GetMusicMode()    == MUSICMODE_CUTSCENE
			&& MusicManager.GetNextTrack() != STREAMED_SOUND_NEWS_INTRO
			&& MusicManager.GetNextTrack() != STREAMED_SOUND_CUTSCENE_SAL4_BDBD )
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
	char filename[MAX_PATH];
	
	ASSERT( nStream < MAX_STREAMS );

	if ( nFile < TOTAL_STREAMED_SOUNDS )
	{
		if ( aStream[nStream] )
		{
			delete aStream[nStream];
			aStream[nStream] = NULL;
		}
		
		strcpy(filename, StreamedNameTable[nFile]);
		
		CStream *stream = new CStream(filename, ALStreamSources[nStream], ALStreamBuffers[nStream], IsThisTrackAt16KHz(nFile) ? 16000 : 32000);
		ASSERT(stream != NULL);
		
		aStream[nStream] = stream;
		if ( !stream->Setup() )
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
		if ( stream->IsOpened() )
		{
			stream->Start();
		}
	}
}

bool
cSampleManager::StartStreamedFile(uint8 nFile, uint32 nPos, uint8 nStream)
{
	uint32 position = nPos;
	char filename[256];
	
	ASSERT( nStream < MAX_STREAMS );
	
	if ( nFile < TOTAL_STREAMED_SOUNDS )
	{
		if ( aStream[nStream] )
		{
			delete aStream[nStream];
			aStream[nStream] = NULL;
		}
		
		if ( nFile == STREAMED_SOUND_RADIO_MP3_PLAYER )
		{
			uint32 i = 0;
			do {
				if(i != 0 || _bIsMp3Active) {
					if(++_CurMP3Index >= nNumMP3s) _CurMP3Index = 0;

					_CurMP3Pos = 0;

					tMP3Entry *mp3 = _GetMP3EntryByIndex(_CurMP3Index);

					if(mp3) {
						mp3 = _pMP3List;
						if(mp3 == NULL) {
							_bIsMp3Active = false;
							nFile = 0;
							strcat(filename, StreamedNameTable[nFile]);

							CStream* stream = new CStream(filename, ALStreamSources[nStream], ALStreamBuffers[nStream], IsThisTrackAt16KHz(nFile) ? 16000 : 32000);
							ASSERT(stream != NULL);

							aStream[nStream] = stream;

							if (stream->Setup()) {
								if (position != 0)
									stream->SetPosMS(position);

								stream->Start();

								return true;
							} else {
								delete stream;
								aStream[nStream] = NULL;
							}

							return false;
						}
					}

					if (mp3->pLinkPath != NULL)
						aStream[nStream] = new CStream(mp3->pLinkPath, ALStreamSources[nStream], ALStreamBuffers[nStream], IsThisTrackAt16KHz(nFile) ? 16000 : 32000);
					else {
						strcpy(filename, _mp3DirectoryPath);
						strcat(filename, mp3->aFilename);

						aStream[nStream] = new CStream(filename, ALStreamSources[nStream], ALStreamBuffers[nStream], IsThisTrackAt16KHz(nFile) ? 16000 : 32000);
					}

					if (aStream[nStream]->Setup()) {
						aStream[nStream]->Start();

						return true;
					} else {
						delete aStream[nStream];
						aStream[nStream] = NULL;
					}

					_bIsMp3Active = false;
					continue;
				}
				if ( nPos > nStreamLength[STREAMED_SOUND_RADIO_MP3_PLAYER] )
					position = 0;
				
				tMP3Entry *e;
				if ( !_GetMP3PosFromStreamPos(&position, &e) )
				{
					if ( e == NULL )
					{
						nFile = 0;
						strcat(filename, StreamedNameTable[nFile]);
						CStream* stream = new CStream(filename, ALStreamSources[nStream], ALStreamBuffers[nStream], IsThisTrackAt16KHz(nFile) ? 16000 : 32000);
						ASSERT(stream != NULL);

						aStream[nStream] = stream;

						if (stream->Setup()) {
							if (position != 0)
								stream->SetPosMS(position);

							stream->Start();

							return true;
						} else {
							delete stream;
							aStream[nStream] = NULL;
						}

						return false;
					}
				}

				if (e->pLinkPath != NULL)
					aStream[nStream] = new CStream(e->pLinkPath, ALStreamSources[nStream], ALStreamBuffers[nStream], IsThisTrackAt16KHz(nFile) ? 16000 : 32000);
				else {
					strcpy(filename, _mp3DirectoryPath);
					strcat(filename, e->aFilename);

					aStream[nStream] = new CStream(filename, ALStreamSources[nStream], ALStreamBuffers[nStream]);
				}

				if (aStream[nStream]->Setup()) {
					if (position != 0)
						aStream[nStream]->SetPosMS(position);

					aStream[nStream]->Start();

					_bIsMp3Active = true;
					return true;
				} else {
					delete aStream[nStream];
					aStream[nStream] = NULL;
				}
				
				_bIsMp3Active = false;

			} while(++i < nNumMP3s);

			position = 0;
			nFile = 0;
		}

		strcpy(filename, StreamedNameTable[nFile]);
		
		CStream *stream = new CStream(filename, ALStreamSources[nStream], ALStreamBuffers[nStream], IsThisTrackAt16KHz(nFile) ? 16000 : 32000);
		ASSERT(stream != NULL);

		aStream[nStream] = stream;
		
		if ( stream->Setup() ) {
			if (position != 0)
				stream->SetPosMS(position);	

			stream->Start();
			
			return true;
		} else {
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

		if ( nStream == 0 )
			_bIsMp3Active = false;
	}
}

int32
cSampleManager::GetStreamedFilePosition(uint8 nStream)
{
	ASSERT( nStream < MAX_STREAMS );
	
	CStream *stream = aStream[nStream];
	
	if ( stream )
	{
		if ( _bIsMp3Active )
		{
			tMP3Entry *mp3 = _GetMP3EntryByIndex(_CurMP3Index);
			
			if ( mp3 != NULL )
			{
				return stream->GetPosMS() + mp3->nTrackStreamPos;
			}
			else
				return 0;
		}
		else
		{
			return stream->GetPosMS();
		}
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
	int refCount = CChannel::channelsThatNeedService;
	for ( int32 i = 0; refCount && i < MAXCHANNELS+MAX2DCHANNELS; i++ )
	{
		if ( aChannel[i].Update() )
			refCount--;
	}
}

bool
cSampleManager::InitialiseSampleBanks(void)
{
	int32 nBank = SFX_BANK_0;
	
	fpSampleDescHandle = fcaseopen(SampleBankDescFilename, "rb");
	if ( fpSampleDescHandle == NULL )
		return false;
#ifndef OPUS_SFX
	fpSampleDataHandle = fcaseopen(SampleBankDataFilename, "rb");
	if ( fpSampleDataHandle == NULL )
	{
		fclose(fpSampleDescHandle);
		fpSampleDescHandle = NULL;
		
		return false;
	}
	
	fseek(fpSampleDataHandle, 0, SEEK_END);
	int32 _nSampleDataEndOffset = ftell(fpSampleDataHandle);
	rewind(fpSampleDataHandle);
#else
	int e;
	fpSampleDataHandle = op_open_file(SampleBankDataFilename, &e);
#endif
	fread(m_aSamples, sizeof(tSample), TOTAL_AUDIO_SAMPLES, fpSampleDescHandle);
#ifdef OPUS_SFX
	int32 _nSampleDataEndOffset = m_aSamples[TOTAL_AUDIO_SAMPLES - 1].nOffset + m_aSamples[TOTAL_AUDIO_SAMPLES - 1].nSize;
#endif
	fclose(fpSampleDescHandle);
	fpSampleDescHandle = NULL;
	
	for ( int32 i = 0; i < TOTAL_AUDIO_SAMPLES; i++ )
	{
#ifdef FIX_BUGS
		if (nBank >= MAX_SFX_BANKS) break;
#endif
		if ( BankStartOffset[nBank] == BankStartOffset[SFX_BANK_0] + i )
		{
			nSampleBankDiscStartOffset[nBank] = m_aSamples[i].nOffset;
			nBank++;
		}
	}

	nSampleBankSize[SFX_BANK_0] = nSampleBankDiscStartOffset[SFX_BANK_PED_COMMENTS] - nSampleBankDiscStartOffset[SFX_BANK_0];
	nSampleBankSize[SFX_BANK_PED_COMMENTS]  = _nSampleDataEndOffset                      - nSampleBankDiscStartOffset[SFX_BANK_PED_COMMENTS];

	return true;
}
#endif
