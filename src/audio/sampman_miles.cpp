#define WITHWINDOWS
#include "common.h"

#ifdef AUDIO_MSS
#include <shlobj.h>
#include <shlguid.h>

#include <time.h>

#include "eax.h"
#include "eax-util.h"
#include "mss.h"

#include "sampman.h"
#include "AudioManager.h"
#include "MusicManager.h"
#include "Frontend.h"
#include "Timer.h"
#include "crossplatform.h"

#pragma comment( lib, "mss32.lib" )

cSampleManager SampleManager;
uint32 BankStartOffset[MAX_SFX_BANKS];
///////////////////////////////////////////////////////////////

char SampleBankDescFilename[] = "AUDIO\\SFX.SDT";
char SampleBankDataFilename[] = "AUDIO\\SFX.RAW";

FILE *fpSampleDescHandle;
FILE *fpSampleDataHandle;
bool8 bSampleBankLoaded            [MAX_SFX_BANKS];
int32 nSampleBankDiscStartOffset   [MAX_SFX_BANKS];
int32 nSampleBankSize              [MAX_SFX_BANKS];
int32 nSampleBankMemoryStartAddress[MAX_SFX_BANKS];
int32 _nSampleDataEndOffset;

int32 nPedSlotSfx    [MAX_PEDSFX];
int32 nPedSlotSfxAddr[MAX_PEDSFX];
uint8 nCurrentPedSlot;

uint8 nChannelVolume[MAXCHANNELS+MAX2DCHANNELS];

uint32 nStreamLength[TOTAL_STREAMED_SOUNDS];

///////////////////////////////////////////////////////////////
struct tMP3Entry
{
	char aFilename[MAX_PATH];
	
	uint32 nTrackLength;
	uint32 nTrackStreamPos;
	
	tMP3Entry *pNext;
	char *pLinkPath;
};

uint32 nNumMP3s;
tMP3Entry *_pMP3List;
char _mp3DirectoryPath[MAX_PATH];
HSTREAM mp3Stream [MAX_STREAMS];
int8 nStreamPan   [MAX_STREAMS];
int8 nStreamVolume[MAX_STREAMS];
bool8 nStreamLoopedFlag[MAX_STREAMS];
uint32 _CurMP3Index;
int32 _CurMP3Pos;
bool8 _bIsMp3Active;
///////////////////////////////////////////////////////////////


bool8 _bSampmanInitialised = FALSE;

//
// Miscellaneous globals / defines

//	Env		Size	Diffus	Room	RoomHF	RoomLF	DecTm	DcHF	DcLF	Refl	RefDel	Ref Pan				Revb	RevDel		Rev Pan				EchTm	EchDp	ModTm	ModDp	AirAbs	HFRef		LFRef	RRlOff	FLAGS

EAXLISTENERPROPERTIES StartEAX3 =
	{26,	1.7f,	0.8f,	-1000,	-1000,	-100,	4.42f,	0.14f,	1.00f,	429,	0.014f,	0.00f,0.00f,0.00f,	1023,	0.021f,		0.00f,0.00f,0.00f,	0.250f,	0.000f,	0.250f,	0.000f,	-5.0f,	2727.1f,	250.0f,	0.00f,	0x3f };

EAXLISTENERPROPERTIES FinishEAX3 =
	{26,	100.0f,	1.0f,	0,		-1000,	-2200,	20.0f,	1.39f,	1.00f,	1000,	0.069f,	0.00f,0.00f,0.00f,	400,	0.100f,		0.00f,0.00f,0.00f,	0.250f,	1.000f,	3.982f,	0.000f,	-18.0f,	3530.8f,	417.9f,	6.70f,	0x3f };

EAXLISTENERPROPERTIES EAX3Params;

S32         prevprovider=-1;
S32         curprovider=-1;
S32         usingEAX=0;
S32         usingEAX3=0;
HPROVIDER   opened_provider=0;
H3DSAMPLE   opened_samples[MAXCHANNELS] = {0};
HSAMPLE     opened_2dsamples[MAX2DCHANNELS] = {0};
HDIGDRIVER  DIG;
S32         speaker_type=0;

U32 _maxSamples;
float _fPrevEaxRatioDestination;
bool8 _usingMilesFast2D;
float _fEffectsLevel;


struct
{
	HPROVIDER id;
	char name[80];
}providers[MAXPROVIDERS];

typedef struct provider_stuff
{
  char* name;
  HPROVIDER id;
} provider_stuff;


static int __cdecl comp(const provider_stuff*s1,const provider_stuff*s2)
{
  return( _stricmp(s1->name,s2->name) );
}

static void
add_providers()
{
	provider_stuff pi[MAXPROVIDERS];
	U32   n,i,j;
	
	SampleManager.SetNum3DProvidersAvailable(0);
	
	HPROENUM next = HPROENUM_FIRST;
	
	n=0;
	while (AIL_enumerate_3D_providers(&next, &pi[n].id, &pi[n].name) && (n<MAXPROVIDERS))
	{
		++n;
	}
	
	qsort(pi,n,sizeof(pi[0]), (int(__cdecl*)(void const*, void const*))comp);
	
	for(i=0;i<n;i++)
	{
		providers[i].id=pi[i].id;
		strcpy(providers[i].name, pi[i].name);
		SampleManager.Set3DProviderName(i, providers[i].name);
	}
	
	SampleManager.SetNum3DProvidersAvailable(n);
	
	for(j=n;j<MAXPROVIDERS;j++)
		SampleManager.Set3DProviderName(j, NULL);
}

static void
release_existing()
{
	for ( U32 i = 0; i < _maxSamples; i++ )
	{
		if ( opened_samples[i] )
		{
			AIL_release_3D_sample_handle(opened_samples[i]);
			opened_samples[i] = NULL;
		}
	}

	if ( opened_provider )
	{
		AIL_close_3D_provider(opened_provider);
		opened_provider = NULL;
	}

	_fPrevEaxRatioDestination = 0.0f;
	_usingMilesFast2D = FALSE;
	_fEffectsLevel = 0.0f;
}

static bool8
set_new_provider(S32 index)
{
	DWORD result;
	
	if ( curprovider == index )
		return TRUE;

	//close the already opened provider
	curprovider = index;
	
	release_existing();
	
	if ( curprovider != -1 )
	{
		if ( !strcmp(providers[index].name, "Dolby Surround") )
			_maxSamples = MAXCHANNELS_SURROUND;
		else
			_maxSamples = MAXCHANNELS;
		
		AIL_set_3D_provider_preference(providers[index].id, "Maximum supported samples", &_maxSamples);
		
		//load the new provider
		result = AIL_open_3D_provider(providers[index].id);
		
		if (result != M3D_NOERR) 
		{
			curprovider=-1;
			
			OutputDebugStringA(AIL_last_error());
			
			release_existing();
			
			return FALSE;
		}
		else
		{
			opened_provider=providers[index].id;
			
			//see if we're running under an EAX compatible provider
			
			if ( !strcmp(providers[index].name, "Creative Labs EAX 3 (TM)") )
			{
				usingEAX = 1;
				usingEAX3 = 1;
			}
			else
			{
				usingEAX3 = 0;

				result=AIL_3D_room_type(opened_provider);
				usingEAX=(((S32)result)!=-1)?1:0; // will be something other than -1 on EAX				
			}
			
			if ( usingEAX3 )
			{
				OutputDebugString("DOING SPECIAL EAX 3 STUFF!");
				AIL_set_3D_provider_preference(opened_provider, "EAX all parameters", &FinishEAX3);
			}
			else if ( usingEAX )
			{
				AIL_set_3D_room_type(opened_provider, ENVIRONMENT_CAVE);
				
				if ( !strcmp(providers[index].name, "Miles Fast 2D Positional Audio") )
					_usingMilesFast2D = TRUE;
			}
			
			AIL_3D_provider_attribute(opened_provider, "Maximum supported samples", &_maxSamples);
			
			if ( _maxSamples > MAXCHANNELS )
				_maxSamples = MAXCHANNELS;
			
			SampleManager.SetSpeakerConfig(speaker_type);
			
			//obtain a 3D sample handles
			for ( U32 i = 0; i < _maxSamples; ++i )
			{
				opened_samples[i] = AIL_allocate_3D_sample_handle(opened_provider);
				if ( opened_samples[i] != NULL )
					AIL_set_3D_sample_effects_level(opened_samples[i], 0.0f);
			}
			
			return TRUE;
		}	
	}
	
	return FALSE;
}

U32 RadioHandlers[9];

U32 WINAPI vfs_open_callback(char const* Filename, U32* FileHandle)
{
	*FileHandle = (U32)fopen(Filename, "rb");

	// couldn't they just use stricmp once? and strlen? this is very inefficient
	if ((strcmp(Filename + strlen(Filename) - 4, ".adf") == 0) || (strcmp(Filename + strlen(Filename) - 4, ".ADF") == 0)) {
		for (int i = 0; i < ARRAY_SIZE(RadioHandlers); i++) {
			if (RadioHandlers[i] == NULL) {
				RadioHandlers[i] = *FileHandle;
				break;
			}
		}
		strcpy((char*)Filename + strlen(Filename) - 4, ".mp3");
	}
	return *FileHandle;
}

void WINAPI vfs_close_callback(U32 FileHandle)
{
	for (int i = 0; i < ARRAY_SIZE(RadioHandlers); i++) {
		if (RadioHandlers[i] == FileHandle) {
			RadioHandlers[i] = NULL;
			break;
		}
	}
	fclose((FILE*)FileHandle);
}

S32 WINAPI vfs_seek_callback(U32 FileHandle, S32 Offset, U32 Type)
{
	fseek((FILE*)FileHandle, Offset, Type);
	return ftell((FILE*)FileHandle);
}

U32 WINAPI vfs_read_callback(U32 FileHandle, void* Buffer, U32 Bytes)
{
	fread(Buffer, Bytes, 1, (FILE*)FileHandle);
	uint8* _Buffer = (uint8*)Buffer;

	for (int i = 0; i < ARRAY_SIZE(RadioHandlers); i++) {
		if (FileHandle == RadioHandlers[i]) {
			for (U32 k = 0; k < Bytes; k++)
				_Buffer[k] ^= 0x22;
			break;
		}
	}
	return Bytes;
}

cSampleManager::cSampleManager(void) : 
	m_nNumberOfProviders(0)
{
	;

	AIL_set_file_callbacks(vfs_open_callback, vfs_close_callback, vfs_seek_callback, vfs_read_callback);
}

cSampleManager::~cSampleManager(void)
{
	
}

void
cSampleManager::SetSpeakerConfig(int32 which)
{
	switch ( which )
	{
		case 1:
			speaker_type=AIL_3D_2_SPEAKER;
			break;
		
		case 2:
			speaker_type=AIL_3D_HEADPHONE;
			break;
		
		case 3:
			speaker_type=AIL_3D_4_SPEAKER;
			break;
			
		default:
			return;
			break;
	}
	
	if (opened_provider)
		AIL_set_3D_speaker_type(opened_provider, speaker_type);
}

uint32
cSampleManager::GetMaximumSupportedChannels(void)
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

int8
cSampleManager::GetCurrent3DProviderIndex(void)
{
	return curprovider;
}

int8
cSampleManager::SetCurrent3DProvider(uint8 nProvider)
{
	S32 savedprovider = curprovider;
	
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

int8
cSampleManager::AutoDetect3DProviders()
{
	if (!AudioManager.IsAudioInitialised())
		return -1;

	int eax = -1, eax2 = -1, eax3 = -1, ds3dh = -1, ds3ds = -1;

	for (uint32 i = 0; i < GetNum3DProvidersAvailable(); i++)
	{
		char* providername = Get3DProviderName(i);

		if (!strcasecmp(providername, "CREATIVE LABS EAX (TM)")) {
			AudioManager.SetCurrent3DProvider(i);
			if (GetCurrent3DProviderIndex() == i)
				eax = i;
		}

		if (!strcasecmp(providername, "CREATIVE LABS EAX 2 (TM)")) {
			AudioManager.SetCurrent3DProvider(i);
			if (GetCurrent3DProviderIndex() == i)
				eax2 = i;
		}

		if (!strcasecmp(providername, "CREATIVE LABS EAX 3 (TM)")) {
			AudioManager.SetCurrent3DProvider(i);
			if (GetCurrent3DProviderIndex() == i) {
				eax3 = i;
			}
		}

		if (!strcasecmp(providername, "DIRECTSOUND3D HARDWARE SUPPORT")) {
			AudioManager.SetCurrent3DProvider(i);
			if (GetCurrent3DProviderIndex() == i)
				ds3dh = i;
		}

		if (!strcasecmp(providername, "DIRECTSOUND3D SOFTWARE EMULATION")) {
			AudioManager.SetCurrent3DProvider(i);
			if (GetCurrent3DProviderIndex() == i)
				ds3ds = i;
		}
	}

	if (eax3 != -1)
		return eax3;
	if (eax2 != -1)
		return eax2;
	if (eax != -1)
		return eax;
	if (ds3dh != -1)
		return ds3dh;
	if (ds3ds != -1)
		return ds3ds;
	return -1;
}

static bool8
_ResolveLink(char const *path, char *out)
{
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
						return TRUE;
					}
				}
			}
			
			ppf->Release();
		}
		psl->Release();
	}
	
	return FALSE;
}

static void
_FindMP3s(void)
{
	tMP3Entry *pList;
	bool8 bShortcut;	
	bool8 bInitFirstEntry;	
	HANDLE hFind;
	char path[MAX_PATH];
	char filepath[MAX_PATH*2];
	S32 total_ms;
	WIN32_FIND_DATA fd;
	
	
	if ( GetCurrentDirectory(MAX_PATH, _mp3DirectoryPath) == 0 )
	{
		GetLastError();
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
		GetLastError();
		return;
	}
	
	strcpy(filepath, _mp3DirectoryPath);
	strcat(filepath, fd.cFileName);
	
	int32 filepathlen = strlen(filepath);
	
	if ( filepathlen <= 0)
	{
		FindClose(hFind);
		return;
	}

	if ( filepathlen > 4 )
	{
		if ( !strcmp(&filepath[filepathlen - 4], ".lnk") )
		{
			if ( _ResolveLink(filepath, filepath) )
			{
				OutputDebugString("Resolving Link");
				OutputDebugString(filepath);
			}
			
			bShortcut = TRUE;
		}
		else
			bShortcut = FALSE;
	}
	
	mp3Stream[0] = AIL_open_stream(DIG, filepath, 0);
	if ( mp3Stream[0] )
	{
		AIL_stream_ms_position(mp3Stream[0], &total_ms, NULL);
		
		AIL_close_stream(mp3Stream[0]);
		mp3Stream[0] = NULL;
		
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
		bInitFirstEntry = FALSE;
	}
	else
	{
		strcat(filepath, " - NOT A VALID MP3");
		
		OutputDebugString(filepath);

		bInitFirstEntry = TRUE;
	}
	
	while ( TRUE )
	{
		if ( !FindNextFile(hFind, &fd) )
			break;
		
		if ( bInitFirstEntry )
		{
			strcpy(filepath, _mp3DirectoryPath);
			strcat(filepath, fd.cFileName);
			
			int32 filepathlen = strlen(filepath);
			
			if ( filepathlen > 0 )
			{
				if ( filepathlen > 4 )
				{
					if ( !strcmp(&filepath[filepathlen - 4], ".lnk") )
					{
						if ( _ResolveLink(filepath, filepath) )
						{
							OutputDebugString("Resolving Link");
							OutputDebugString(filepath);
						}
						
						bShortcut = TRUE;
					}
					else
					{
						bShortcut = FALSE;
						
						if ( filepathlen > MAX_PATH )
						{
							continue;
						}
					}
				}
				
				mp3Stream[0] = AIL_open_stream(DIG, filepath, 0);
				if ( mp3Stream[0] )
				{
					AIL_stream_ms_position(mp3Stream[0], &total_ms, NULL);
					
					AIL_close_stream(mp3Stream[0]);
					mp3Stream[0] = NULL;
					
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

					bInitFirstEntry = FALSE;
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
			
			int32 filepathlen = strlen(filepath);
			
			if ( filepathlen > 0 )
			{
				if ( filepathlen > 4 )
				{
					if ( !strcmp(&filepath[filepathlen - 4], ".lnk") )
					{
						if ( _ResolveLink(filepath, filepath) )
						{
							OutputDebugString("Resolving Link");
							OutputDebugString(filepath);
						}
						
						bShortcut = TRUE;
					}
					else
					{
						bShortcut = FALSE;
					}
				}
				
				mp3Stream[0] = AIL_open_stream(DIG, filepath, 0);
				if ( mp3Stream[0] )
				{
					AIL_stream_ms_position(mp3Stream[0], &total_ms, NULL);
					
					AIL_close_stream(mp3Stream[0]);
					mp3Stream[0] = NULL;
					
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

static inline bool8
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
			
			return TRUE;
		}
		
		_CurMP3Index++;
	}
				
	*pPosition = 0;
	*pEntry = _pMP3List;
	_CurMP3Pos = 0;
	_CurMP3Index = 0;
	
	return FALSE;
}

bool8
cSampleManager::IsMP3RadioChannelAvailable(void)
{
	return nNumMP3s != 0;
}

void
cSampleManager::ReleaseDigitalHandle(void)
{
	if ( DIG )
	{
		prevprovider = curprovider;
		release_existing();
		curprovider = -1;
		AIL_digital_handle_release(DIG);
	}
}

void
cSampleManager::ReacquireDigitalHandle(void)
{
	if ( DIG )
	{
		AIL_digital_handle_reacquire(DIG);
		if ( prevprovider != -1 )
			set_new_provider(prevprovider);
	}
}

bool8
cSampleManager::Initialise(void)
{
	TRACE("start");
	
	if ( _bSampmanInitialised )
		return TRUE;

	{
		for ( int32 i = 0; i < TOTAL_AUDIO_SAMPLES; i++ )
		{
			m_aSamples[i].nOffset    = 0;
			m_aSamples[i].nSize      = 0;
			m_aSamples[i].nFrequency = 22050;
			m_aSamples[i].nLoopStart = 0;
			m_aSamples[i].nLoopEnd   = -1;
		}
		
		m_nEffectsVolume     = MAX_VOLUME;
		m_nMusicVolume       = MAX_VOLUME;
		m_nEffectsFadeVolume = MAX_VOLUME;
		m_nMusicFadeVolume   = MAX_VOLUME;
		
		m_nMonoMode = 0;
	}
	
	// miles 
	TRACE("MILES");
	{
		curprovider = -1;
		prevprovider = -1;
		
		_usingMilesFast2D = FALSE;
		usingEAX=0;
		usingEAX3=0;
		
		_fEffectsLevel = 0.0f;
		
		_maxSamples = 0;
	
		opened_provider = NULL;
		DIG = NULL;
		
		for ( int32 i = 0; i < MAXCHANNELS; i++ )
			opened_samples[i] = NULL;
	}
	
	// banks
	TRACE("banks");
	{
		fpSampleDescHandle = NULL;
		fpSampleDataHandle = NULL;
		
		_nSampleDataEndOffset = 0;
		
		for ( int32 i = 0; i < MAX_SFX_BANKS; i++ )
		{
			bSampleBankLoaded[i]             = FALSE;
			nSampleBankDiscStartOffset[i]    = 0;
			nSampleBankSize[i]               = 0;
			nSampleBankMemoryStartAddress[i] = 0;
		}
	}
	
	// pedsfx
	TRACE("pedsfx");
	{
		for ( int32 i = 0; i < MAX_PEDSFX; i++ )
		{
			nPedSlotSfx[i]     = NO_SAMPLE;
			nPedSlotSfxAddr[i] = 0;
		}
		
		nCurrentPedSlot = 0;
	}
	
	// channel volume
	TRACE("vol");
	{
		for ( int32 i = 0; i < MAXCHANNELS+MAX2DCHANNELS; i++ )
			nChannelVolume[i] = 0;
	}
	
	TRACE("mss");
	{
		AIL_set_redist_directory( "mss" );
		
		AIL_startup();
		
		AIL_set_preference(DIG_MIXER_CHANNELS, MAX_DIGITAL_MIXER_CHANNELS);
		
		DIG = AIL_open_digital_driver(DIGITALRATE, DIGITALBITS, DIGITALCHANNELS, 0);		
		
	}
	
#ifdef AUDIO_CACHE
	TRACE("cache");
	FILE *cacheFile = fcaseopen("audio\\sound.cache", "rb");
	bool8 CreateCache = FALSE;
	if (cacheFile) {
		fread(nStreamLength, sizeof(uint32), TOTAL_STREAMED_SOUNDS, cacheFile);
		fclose(cacheFile);
	}else
		CreateCache = TRUE;
#endif
	
	char filepath[MAX_PATH];
	bool8 bFileNotFound;
	S32 tatalms;

	TRACE("cdrom");
	{
		m_bInitialised = FALSE;

		
		while (TRUE)
		{

			// Find path of WAVs (originally in HDD)
			int32 drive = 'C';
			
#ifndef NO_CDCHECK
			do
			{
				char latter[2];
				
				latter[0] = drive;
				latter[1] = '\0';
				
				strcpy(m_szCDRomRootPath, latter);
				strcat(m_szCDRomRootPath, ":\\");
				
				if ( GetDriveType(m_szCDRomRootPath) == DRIVE_CDROM )
				{
					strcpy(filepath, m_szCDRomRootPath);
					strcat(filepath, StreamedNameTable[0]);
					
					FILE *f = fopen(filepath, "rb");
					
					if ( f )
					{
						fclose(f);
						strcpy(m_MiscomPath, m_szCDRomRootPath);
						break;
					}
				}

			} while ( ++drive <= 'Z' );
#else
			m_MiscomPath[0] = '\0';
#endif

			if ( DIG == NULL )
			{
				OutputDebugString(AIL_last_error());
				Terminate();
				return FALSE;
			}
			
			add_providers();

			m_szCDRomRootPath[0] = '\0';

			strcpy(m_WavFilesPath, m_szCDRomRootPath);

#ifdef AUDIO_CACHE
			if ( CreateCache )
#endif
			for ( int32 i = STREAMED_SOUND_MISSION_MOBR1; i < TOTAL_STREAMED_SOUNDS; i++ )
			{
#ifdef PS2_AUDIO_PATHS
				strcpy(filepath, m_szCDRomRootPath);
				strcat(filepath, PS2StreamedNameTable[i]);

				mp3Stream[0] = AIL_open_stream(DIG, filepath, 0);

				if ( !mp3Stream[0] )
#endif
				{
					strcpy(filepath, m_szCDRomRootPath);
					strcat(filepath, StreamedNameTable[i]);

					mp3Stream[0] = AIL_open_stream(DIG, filepath, 0);
				}
				
				if ( mp3Stream[0] )
				{
					AIL_stream_ms_position(mp3Stream[0], &tatalms, NULL);
					
					AIL_close_stream(mp3Stream[0]);
					mp3Stream[0] = NULL;
					
					nStreamLength[i] = tatalms;
				}
				else
				{
					m_bInitialised = FALSE;
					Terminate();
					return FALSE;
				}
			}

			// Find path of MP3s (originally in CD-Rom)
			// if NO_CDCHECK is NOT defined but AUDIO_CACHE is defined, we still need to find MP3s' path, but will exit after the first file
#ifndef NO_CDCHECK
			int32 drive = 'C';
			do
			{
				latter[0] = drive;
				latter[1] = '\0';
				
				strcpy(m_szCDRomRootPath, latter);
				strcat(m_szCDRomRootPath, ":");
				strcat(m_MP3FilesPath, m_szCDRomRootPath);
#else
			m_MP3FilesPath[0] = '\0';
			{
#endif

				for (int32 i = 0; i < STREAMED_SOUND_MISSION_MOBR1; i++)
				{
#ifdef PS2_AUDIO_PATHS
					strcpy(filepath, m_MP3FilesPath);
					strcat(filepath, PS2StreamedNameTable[i]);

					mp3Stream[0] = AIL_open_stream(DIG, filepath, 0);

					if ( !mp3Stream[0] )
#endif
					{
						strcpy(filepath, m_MP3FilesPath);
						strcat(filepath, StreamedNameTable[i]);

						mp3Stream[0] = AIL_open_stream(DIG, filepath, 0);
					}

					if (mp3Stream[0])
					{
						AIL_stream_ms_position(mp3Stream[0], &tatalms, NULL);

						AIL_close_stream(mp3Stream[0]);
						mp3Stream[0] = NULL;

						bFileNotFound = FALSE;
#ifdef AUDIO_CACHE
						if (!CreateCache)
							break;
						else
#endif
							nStreamLength[i] = tatalms;

					}
					else
					{
						bFileNotFound = TRUE;
						break;
					}
				}

#ifndef NO_CDCHECK
				if (!bFileNotFound) // otherwise try next drive
					break;

			}
			while (++drive <= 'Z');
#else
			}
#endif

			if ( !bFileNotFound ) {

#ifdef AUDIO_CACHE
			if ( CreateCache )
#endif
				for ( int32 i = STREAMED_SOUND_MISSION_COMPLETED4; i < STREAMED_SOUND_MISSION_PAGER; i++ )
				{
#ifdef PS2_AUDIO_PATHS
					strcpy(filepath, m_MiscomPath);
					strcat(filepath, PS2StreamedNameTable[i]);

					mp3Stream[0] = AIL_open_stream(DIG, filepath, 0);

					if ( !mp3Stream[0] )
#endif
					{
						strcpy(filepath, m_MiscomPath);
						strcat(filepath, StreamedNameTable[i]);
					
						mp3Stream[0] = AIL_open_stream(DIG, filepath, 0);
					}
					
					if ( mp3Stream[0] )
					{
						AIL_stream_ms_position(mp3Stream[0], &tatalms, NULL);
						
						AIL_close_stream(mp3Stream[0]);
						mp3Stream[0] = NULL;
						
						nStreamLength[i] = tatalms;
						bFileNotFound = FALSE;
					}
					else
					{
						bFileNotFound = TRUE;
						break;
					}
				}
			}
			
			m_bInitialised = !bFileNotFound;

			if ( !m_bInitialised )
			{
#if !defined(GTA3_STEAM_PATCH) && !defined(NO_CDCHECK)
				FrontEndMenuManager.WaitForUserCD();
				if ( FrontEndMenuManager.m_bQuitGameNoCD )
				{
					Terminate();
					return FALSE;
				}
				continue;
#else
				m_bInitialised = TRUE;
#endif
			}
			
			break;
		}
	}

#ifdef AUDIO_CACHE
	if (CreateCache) {
		cacheFile = fcaseopen("audio\\sound.cache", "wb");
		fwrite(nStreamLength, sizeof(uint32), TOTAL_STREAMED_SOUNDS, cacheFile);
		fclose(cacheFile);
	}
#endif

	if ( !InitialiseSampleBanks() )
	{
		Terminate();
		return FALSE;
	}
	
	nSampleBankMemoryStartAddress[SFX_BANK_0] = (int32)AIL_mem_alloc_lock(nSampleBankSize[SFX_BANK_0]);
	if ( !nSampleBankMemoryStartAddress[SFX_BANK_0] )
	{
		Terminate();
		return FALSE;
	}

	nSampleBankMemoryStartAddress[SFX_BANK_PED_COMMENTS] = (int32)AIL_mem_alloc_lock(PED_BLOCKSIZE*MAX_PEDSFX);

	LoadSampleBank(SFX_BANK_0);

	TRACE("stream");
	{
		for ( int32 i = 0; i < MAX_STREAMS; i++ )
		{
			mp3Stream    [i] = NULL;
			nStreamPan   [i] = 63;
			nStreamVolume[i] = 100;
		}
	}
	
	for ( int32 i = 0; i < MAX2DCHANNELS; i++ )
	{
		opened_2dsamples[i] = AIL_allocate_sample_handle(DIG);
		if ( opened_2dsamples[i] )
		{
			AIL_init_sample(opened_2dsamples[i]);
			AIL_set_sample_type(opened_2dsamples[i], DIG_F_MONO_16, DIG_PCM_SIGN);
		}
	}
	
	TRACE("providerset");
	{
		_bSampmanInitialised = TRUE;
		
		U32 n = 0;
		
		while ( n < m_nNumberOfProviders )
		{
			if ( !strcmp(strupr(providers[n].name), "DIRECTSOUND3D SOFTWARE EMULATION") )
			{
				set_new_provider(n);
				break;
			}
			n++;
		}
		
		if ( n == m_nNumberOfProviders )
		{
			Terminate();
			return FALSE;
		}
	}
	
	// mp3
	TRACE("mp3");
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
			bool8 bUseRandomTable;
			
			if ( t == -1 )
				bUseRandomTable = TRUE;
			else
			{
				bUseRandomTable = FALSE;
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
		
		_bIsMp3Active = FALSE;
	}
	
	TRACE("end");
	
	return TRUE;
}

void
cSampleManager::Terminate(void)
{
	for ( int32 i = 0; i < MAX_STREAMS; i++ )
	{
		if ( mp3Stream[i] )
		{
			AIL_pause_stream(mp3Stream[i], 1);
			AIL_close_stream(mp3Stream[i]);
			mp3Stream[i] = NULL;
		}
	}
	
	for ( int32 i = 0; i < MAX2DCHANNELS; i++ )
	{
		if ( opened_2dsamples[i] )
		{
			AIL_release_sample_handle(opened_2dsamples[i]);
			opened_2dsamples[i] = NULL;
		}
	}
	
	release_existing();
	
	_DeleteMP3Entries();
	
	if ( nSampleBankMemoryStartAddress[SFX_BANK_0] != 0 )
	{
		AIL_mem_free_lock((void *)nSampleBankMemoryStartAddress[SFX_BANK_0]);
		nSampleBankMemoryStartAddress[SFX_BANK_0] = 0;
	}

	if ( nSampleBankMemoryStartAddress[SFX_BANK_PED_COMMENTS] != 0 )
	{
		AIL_mem_free_lock((void *)nSampleBankMemoryStartAddress[SFX_BANK_PED_COMMENTS]);
		nSampleBankMemoryStartAddress[SFX_BANK_PED_COMMENTS] = 0;
	}
	
	if ( DIG )
	{
		AIL_close_digital_driver(DIG);
		DIG = NULL;
	}
	
	AIL_shutdown();
	
	_bSampmanInitialised = FALSE;
}

bool8
cSampleManager::CheckForAnAudioFileOnCD(void)
{
#if !defined(NO_CDCHECK) // TODO: check steam, probably GTAVC_STEAM_PATCH needs to be added
	char filepath[MAX_PATH];
	
	strcpy(filepath, m_MiscomPath);
	strcat(filepath, StreamedNameTable[STREAMED_SOUND_MISSION_COMPLETED4]);

	FILE *f = fopen(filepath, "rb");

	if ( f )
	{
		fclose(f);
		DMAudio.SetMusicMasterVolume(FrontEndMenuManager.m_PrefsMusicVolume);
		DMAudio.SetEffectsMasterVolume(FrontEndMenuManager.m_PrefsSfxVolume);
		DMAudio.Service();

		return TRUE;
	}

	DMAudio.SetMusicMasterVolume(0);
	DMAudio.SetEffectsMasterVolume(0);
	DMAudio.Service();

	return FALSE;
	
#else
	return TRUE;
#endif // #if !defined(NO_CDCHECK)
}

char
cSampleManager::GetCDAudioDriveLetter(void)
{
	if ( strlen(m_MiscomPath) != 0 )
		return m_MiscomPath[0];
	else
		return '\0';
}

void
cSampleManager::UpdateEffectsVolume(void) //[Y], cSampleManager::UpdateSoundBuffers ?
{
	if ( _bSampmanInitialised )
	{
		for ( int32 i = 0; i < MAXCHANNELS+MAX2DCHANNELS; i++ )
		{
			if ( i < MAXCHANNELS )
			{
				if ( opened_samples[i] && GetChannelUsedFlag(i) )
				{
					if ( nChannelVolume[i] )
					{
						AIL_set_3D_sample_volume(opened_samples[i],
								m_nEffectsFadeVolume * nChannelVolume[i] * m_nEffectsVolume >> 14);
					}
				}
			}
			else
			{
				if ( opened_2dsamples[i - MAXCHANNELS] )
				{
					if ( GetChannelUsedFlag(i - MAXCHANNELS) )
					{
						if ( nChannelVolume[i - MAXCHANNELS] )
						{
							AIL_set_sample_volume(opened_2dsamples[i - MAXCHANNELS],
									m_nEffectsFadeVolume * nChannelVolume[i - MAXCHANNELS] * m_nEffectsVolume >> 14);
						}
					}
				}
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
cSampleManager::SetMP3BoostVolume(uint8 nVolume)
{
	m_nMP3BoostVolume = nVolume;
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
cSampleManager::SetMonoMode(bool8 nMode)
{
	m_nMonoMode = nMode;
}

bool8
cSampleManager::LoadSampleBank(uint8 nBank)
{
	if ( CTimer::GetIsCodePaused() )
		return FALSE;
	
	if ( MusicManager.IsInitialised()
		&& MusicManager.GetMusicMode() == MUSICMODE_CUTSCENE
		&& nBank != SFX_BANK_0 )
	{
		return FALSE;
	}
	
	if ( fseek(fpSampleDataHandle, nSampleBankDiscStartOffset[nBank], SEEK_SET) != 0 )
		return FALSE;
	
	if ( fread((void *)nSampleBankMemoryStartAddress[nBank], 1, nSampleBankSize[nBank],fpSampleDataHandle) != nSampleBankSize[nBank] )
		return FALSE;
	
	bSampleBankLoaded[nBank] = TRUE;
	
	return TRUE;
}

void
cSampleManager::UnloadSampleBank(uint8 nBank)
{
	bSampleBankLoaded[nBank] = FALSE;
}

bool8
cSampleManager::IsSampleBankLoaded(uint8 nBank)
{
	return bSampleBankLoaded[nBank];
}

bool8
cSampleManager::IsPedCommentLoaded(uint32 nComment)
{
	int8 slot;

	for ( int32 i = 0; i < _TODOCONST(3); i++ )
	{
		slot = nCurrentPedSlot - i - 1;
#ifdef FIX_BUGS
		if (slot < 0)
			slot += ARRAY_SIZE(nPedSlotSfx);
#endif
		if ( nComment == nPedSlotSfx[slot] )
			return TRUE;
	}
	
	return FALSE;
}

int32
cSampleManager::_GetPedCommentSlot(uint32 nComment)
{
	int8 slot;

	for ( int32 i = 0; i < _TODOCONST(3); i++ )
	{
		slot = nCurrentPedSlot - i - 1;
#ifdef FIX_BUGS
		if (slot < 0)
			slot += ARRAY_SIZE(nPedSlotSfx);
#endif
		if ( nComment == nPedSlotSfx[slot] )
			return slot;
	}
	
	return -1;
}

bool8
cSampleManager::LoadPedComment(uint32 nComment)
{
	if ( CTimer::GetIsCodePaused() )
		return FALSE;
	
	// no talking peds during cutsenes or the game end
	if ( MusicManager.IsInitialised() )
	{
		switch ( MusicManager.GetMusicMode() )
		{
			case MUSICMODE_CUTSCENE:
			{
				return FALSE;

				break;
			}
		}
	}
	
	if ( fseek(fpSampleDataHandle, m_aSamples[nComment].nOffset, SEEK_SET) != 0 )
		return FALSE;
	
	if ( fread((void *)(nSampleBankMemoryStartAddress[SFX_BANK_PED_COMMENTS] + PED_BLOCKSIZE*nCurrentPedSlot), 1, m_aSamples[nComment].nSize, fpSampleDataHandle) != m_aSamples[nComment].nSize )
		return FALSE;
	
	nPedSlotSfxAddr[nCurrentPedSlot] = nSampleBankMemoryStartAddress[SFX_BANK_PED_COMMENTS] + PED_BLOCKSIZE*nCurrentPedSlot;
	nPedSlotSfx    [nCurrentPedSlot] = nComment;
	
	if ( ++nCurrentPedSlot >= MAX_PEDSFX )
		nCurrentPedSlot = 0;
	
	return TRUE;
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
	return m_aSamples[nSample].nFrequency;
}

int32
cSampleManager::GetSampleLoopStartOffset(uint32 nSample)
{
	return m_aSamples[nSample].nLoopStart;
}

int32
cSampleManager::GetSampleLoopEndOffset(uint32 nSample)
{
	return m_aSamples[nSample].nLoopEnd;
}

uint32
cSampleManager::GetSampleLength(uint32 nSample)
{
	return m_aSamples[nSample].nSize >> 1;
}

bool8
cSampleManager::UpdateReverb(void)
{
	if ( !usingEAX )
		return FALSE;
	
	if ( AudioManager.GetFrameCounter() & 15 )
		return FALSE;

	float fRatio = 0.0f;

#define MIN_DIST 0.5f
#define CALCULATE_RATIO(value, maxDist, maxRatio) (value > MIN_DIST && value < maxDist ? value / maxDist * maxRatio : 0)

	fRatio += CALCULATE_RATIO(AudioManager.GetReflectionsDistance(REFLECTION_CEIL_NORTH), 10.0f, 1/2.f);
	fRatio += CALCULATE_RATIO(AudioManager.GetReflectionsDistance(REFLECTION_CEIL_SOUTH), 10.0f, 1/2.f);
	fRatio += CALCULATE_RATIO(AudioManager.GetReflectionsDistance(REFLECTION_CEIL_WEST), 10.0f, 1/2.f);
	fRatio += CALCULATE_RATIO(AudioManager.GetReflectionsDistance(REFLECTION_CEIL_EAST), 10.0f, 1/2.f);

	fRatio += CALCULATE_RATIO((AudioManager.GetReflectionsDistance(REFLECTION_NORTH) + AudioManager.GetReflectionsDistance(REFLECTION_SOUTH)) / 2.f, 4.0f, 1/3.f);
	fRatio += CALCULATE_RATIO((AudioManager.GetReflectionsDistance(REFLECTION_WEST) + AudioManager.GetReflectionsDistance(REFLECTION_EAST)) / 2.f, 4.0f, 1/3.f);

#undef CALCULATE_RATIO
#undef MIN_DIST
	
	fRatio = Clamp(fRatio, 0.0f, 0.6f);
	
	if ( fRatio == _fPrevEaxRatioDestination )
		return FALSE;
	
	if ( usingEAX3 )
	{
		fRatio = Min(fRatio * 1.67f, 1.0f);
		if ( EAX3ListenerInterpolate(&StartEAX3, &FinishEAX3, fRatio, &EAX3Params, false) )
		{
			AIL_set_3D_provider_preference(opened_provider, "EAX all parameters", &EAX3Params);
			_fEffectsLevel = fRatio * 0.75f;
		}
	}
	else
	{
		if ( _usingMilesFast2D )
			_fEffectsLevel = fRatio * 0.8f;
		else
			_fEffectsLevel = fRatio * 0.22f;
	}
	_fEffectsLevel = Min(_fEffectsLevel, 1.0f);

	_fPrevEaxRatioDestination = fRatio;
	
	return TRUE;
}

void
cSampleManager::SetChannelReverbFlag(uint32 nChannel, bool8 nReverbFlag)
{
	bool8 b2d = FALSE;
	
	switch ( nChannel )
	{
		case CHANNEL_POLICE_RADIO:
		{
			b2d = TRUE;
			break;
		}
	}
	
	if ( usingEAX )
	{
		if ( nReverbFlag != FALSE )
		{
			if ( !b2d )
				AIL_set_3D_sample_effects_level(opened_samples[nChannel], _fEffectsLevel);
		}
		else
		{
			if ( !b2d )
				AIL_set_3D_sample_effects_level(opened_samples[nChannel], 0.0f);
		}
	}
}

bool8
cSampleManager::InitialiseChannel(uint32 nChannel, uint32 nSfx, uint8 nBank)
{
	bool8 b2d = FALSE;

	switch ( nChannel )
	{
		case CHANNEL_POLICE_RADIO:
		{
			b2d = TRUE;
			break;
		}
	}
	
	int32 addr;
	
	if ( nSfx < SAMPLEBANK_MAX )
	{
		if ( !IsSampleBankLoaded(nBank) )
			return FALSE;
		
		addr = nSampleBankMemoryStartAddress[nBank] + m_aSamples[nSfx].nOffset - m_aSamples[BankStartOffset[nBank]].nOffset;
	}
	else
	{	
		if ( !IsPedCommentLoaded(nSfx) )
			return FALSE;
		
		int32 slot = _GetPedCommentSlot(nSfx);
		
		addr = nPedSlotSfxAddr[slot];
	}
	
	if ( b2d )
	{
		if ( opened_2dsamples[nChannel - MAXCHANNELS] )
		{
			AIL_set_sample_address(opened_2dsamples[nChannel - MAXCHANNELS], (void *)addr, m_aSamples[nSfx].nSize);
			return TRUE;
		}
		else
			return FALSE;
	}
	else
	{
		AILSOUNDINFO info;
		
		info.format   = WAVE_FORMAT_PCM;
		info.data_ptr = (void *)addr;
		info.channels = 1;
		info.data_len = m_aSamples[nSfx].nSize;
		info.rate     = m_aSamples[nSfx].nFrequency;
		info.bits     = 16;
	
		if ( AIL_set_3D_sample_info(opened_samples[nChannel], &info) == 0 )
		{
			OutputDebugString(AIL_last_error());
			return FALSE;
		}
		
		return TRUE;
	}
}

void
cSampleManager::SetChannelEmittingVolume(uint32 nChannel, uint32 nVolume)
{
	uint32 vol = nVolume;
	if ( vol > MAX_VOLUME ) vol = MAX_VOLUME;
	
	nChannelVolume[nChannel] = vol;
	
	// increase the volume for JB.MP3 and S4_BDBD.MP3
	if (MusicManager.GetMusicMode() == MUSICMODE_CUTSCENE ) {
		if (MusicManager.GetCurrentTrack() == STREAMED_SOUND_CUTSCENE_FINALE)
			nChannelVolume[nChannel] = 0;
		else
			nChannelVolume[nChannel] >>= 2;
	}

	if ( opened_samples[nChannel] )
		AIL_set_3D_sample_volume(opened_samples[nChannel], m_nEffectsFadeVolume*nChannelVolume[nChannel]*m_nEffectsVolume >> 14);

}

void
cSampleManager::SetChannel3DPosition(uint32 nChannel, float fX, float fY, float fZ)
{
	if ( opened_samples[nChannel] )
		AIL_set_3D_position(opened_samples[nChannel], -fX, fY, fZ);
}

void
cSampleManager::SetChannel3DDistances(uint32 nChannel, float fMax, float fMin)
{
	if ( opened_samples[nChannel] )
		AIL_set_3D_sample_distances(opened_samples[nChannel], fMax, fMin);
}

void
cSampleManager::SetChannelVolume(uint32 nChannel, uint32 nVolume)
{
	uint32 vol = nVolume;
	if ( vol > MAX_VOLUME ) vol = MAX_VOLUME;
	
	switch ( nChannel )
	{
		case CHANNEL_POLICE_RADIO:
		{
			nChannelVolume[nChannel] = vol;
			
			// increase the volume for JB.MP3 and S4_BDBD.MP3
			if (   MusicManager.GetMusicMode()    == MUSICMODE_CUTSCENE
				&& MusicManager.GetCurrentTrack() != STREAMED_SOUND_CUTSCENE_FINALE )
			{
				nChannelVolume[nChannel] >>= 2;
			}

			if ( opened_2dsamples[nChannel - MAXCHANNELS] )
			{
				AIL_set_sample_volume(opened_2dsamples[nChannel - MAXCHANNELS],
						m_nEffectsFadeVolume*vol*m_nEffectsVolume >> 14);
			}
			
			break;
		}
	}
}

void
cSampleManager::SetChannelPan(uint32 nChannel, uint32 nPan)
{
	switch ( nChannel )
	{
		case CHANNEL_POLICE_RADIO:
		{
#ifndef FIX_BUGS
			if ( opened_samples[nChannel - MAXCHANNELS] ) // BUG
#else
			if ( opened_2dsamples[nChannel - MAXCHANNELS] )
#endif
				AIL_set_sample_pan(opened_2dsamples[nChannel - MAXCHANNELS], nPan);

			break;
		}
	}
}

void
cSampleManager::SetChannelFrequency(uint32 nChannel, uint32 nFreq)
{
	bool8 b2d = FALSE;

	switch ( nChannel )
	{
		case CHANNEL_POLICE_RADIO:
		{
			b2d = TRUE;
			break;
		}
	}

	if ( b2d )
	{
		if ( opened_2dsamples[nChannel - MAXCHANNELS] )
			AIL_set_sample_playback_rate(opened_2dsamples[nChannel - MAXCHANNELS], nFreq);
	}
	else
	{
		if ( opened_samples[nChannel] )
			AIL_set_3D_sample_playback_rate(opened_samples[nChannel], nFreq);
	}
}

void
cSampleManager::SetChannelLoopPoints(uint32 nChannel, uint32 nLoopStart, int32 nLoopEnd)
{
	bool8 b2d = FALSE;

	switch ( nChannel )
	{
		case CHANNEL_POLICE_RADIO:
		{
			b2d = TRUE;
			break;
		}
	}
	
	if ( b2d )
	{
		if ( opened_2dsamples[nChannel - MAXCHANNELS] )
			AIL_set_sample_loop_block(opened_2dsamples[nChannel - MAXCHANNELS], nLoopStart, nLoopEnd);
	}
	else
	{
		if ( opened_samples[nChannel] )
			AIL_set_3D_sample_loop_block(opened_samples[nChannel], nLoopStart, nLoopEnd);
	}
}

void
cSampleManager::SetChannelLoopCount(uint32 nChannel, uint32 nLoopCount)
{
	bool8 b2d = FALSE;

	switch ( nChannel )
	{
		case CHANNEL_POLICE_RADIO:
		{
			b2d = TRUE;
			break;
		}
	}
	
	if ( b2d )
	{
		if ( opened_2dsamples[nChannel - MAXCHANNELS] )
			AIL_set_sample_loop_count(opened_2dsamples[nChannel - MAXCHANNELS], nLoopCount);
	}
	else
	{
		if ( opened_samples[nChannel] )
			AIL_set_3D_sample_loop_count(opened_samples[nChannel], nLoopCount);
	}
}

bool8
cSampleManager::GetChannelUsedFlag(uint32 nChannel)
{
	bool8 b2d = FALSE;

	switch ( nChannel )
	{
		case CHANNEL_POLICE_RADIO:
		{
			b2d = TRUE;
			break;
		}
	}
	
	if ( b2d )
	{
		if ( opened_2dsamples[nChannel - MAXCHANNELS] )
			return AIL_sample_status(opened_2dsamples[nChannel - MAXCHANNELS]) == SMP_PLAYING;
		else
			return FALSE;
	}
	else
	{
		if ( opened_samples[nChannel] )
			return AIL_3D_sample_status(opened_samples[nChannel]) == SMP_PLAYING;
		else
			return FALSE;
	}
	
}

void
cSampleManager::StartChannel(uint32 nChannel)
{
	bool8 b2d = FALSE;

	switch ( nChannel )
	{
		case CHANNEL_POLICE_RADIO:
		{
			b2d = TRUE;
			break;
		}
	}

	if ( b2d )
	{
		if ( opened_2dsamples[nChannel - MAXCHANNELS] )
			AIL_start_sample(opened_2dsamples[nChannel - MAXCHANNELS]);
	}
	else
	{
		if ( opened_samples[nChannel] )
			AIL_start_3D_sample(opened_samples[nChannel]);
	}
}

void
cSampleManager::StopChannel(uint32 nChannel)
{
	bool8 b2d = FALSE;

	switch ( nChannel )
	{
		case CHANNEL_POLICE_RADIO:
		{
			b2d = TRUE;
			break;
		}
	}
	
	if ( b2d )
	{
		if ( opened_2dsamples[nChannel - MAXCHANNELS] )
			AIL_end_sample(opened_2dsamples[nChannel - MAXCHANNELS]);
	}
	else
	{
		if ( opened_samples[nChannel] )
		{
			if ( AIL_3D_sample_status(opened_samples[nChannel]) == SMP_PLAYING )
				AIL_end_3D_sample(opened_samples[nChannel]);
		}
	}
}

void
cSampleManager::PreloadStreamedFile(uint32 nFile, uint8 nStream)
{
	if ( m_bInitialised  )
	{
		if ( nFile < TOTAL_STREAMED_SOUNDS )
		{
			if ( mp3Stream[nStream] )
			{
				AIL_pause_stream(mp3Stream[nStream], 1);
				AIL_close_stream(mp3Stream[nStream]);
			}
			
			char filepath[MAX_PATH];
#ifdef PS2_AUDIO_PATHS
			strcpy(filepath, nFile < STREAMED_SOUND_MISSION_COMPLETED4 ? m_MP3FilesPath : (nFile < STREAMED_SOUND_MISSION_MOBR1 ? m_MiscomPath : m_WavFilesPath));
			strcat(filepath, PS2StreamedNameTable[nFile]);

			mp3Stream[nStream] = AIL_open_stream(DIG, filepath, 0);

			if ( !mp3Stream[nStream] )
#endif
			{
				strcpy(filepath, nFile < STREAMED_SOUND_MISSION_COMPLETED4 ? m_MP3FilesPath : (nFile < STREAMED_SOUND_MISSION_MOBR1 ? m_MiscomPath : m_WavFilesPath));
				strcat(filepath, StreamedNameTable[nFile]);
			
				mp3Stream[nStream] = AIL_open_stream(DIG, filepath, 0);
			}
	
			if ( mp3Stream[nStream] )
			{
				AIL_set_stream_loop_count(mp3Stream[nStream], 1);
				AIL_service_stream(mp3Stream[nStream], 1);
			}
			else
				OutputDebugString(AIL_last_error());
		}
	}
}

void
cSampleManager::PauseStream(bool8 nPauseFlag, uint8 nStream)
{
	if ( m_bInitialised )
	{
		if ( mp3Stream[nStream] )
			AIL_pause_stream(mp3Stream[nStream], nPauseFlag != FALSE);
	}
}

void
cSampleManager::StartPreloadedStreamedFile(uint8 nStream)
{
	if ( m_bInitialised )
	{
		if ( mp3Stream[nStream] )
			AIL_start_stream(mp3Stream[nStream]);
	}
}

bool8
cSampleManager::StartStreamedFile(uint32 nFile, uint32 nPos, uint8 nStream)
{
	int i = 0;
	uint32 position = nPos;
	char filename[MAX_PATH];
	
	if ( !m_bInitialised || nFile >= TOTAL_STREAMED_SOUNDS )
		return FALSE;

	if ( mp3Stream[nStream] )
	{
		AIL_pause_stream(mp3Stream[nStream], 1);
		AIL_close_stream(mp3Stream[nStream]);
	}
	if ( nFile == STREAMED_SOUND_RADIO_MP3_PLAYER )
	{
		do
		{
			// Just switched to MP3 player
			if ( !_bIsMp3Active && i == 0 )
			{
				if ( nPos > nStreamLength[STREAMED_SOUND_RADIO_MP3_PLAYER] )
					position = 0;
				tMP3Entry *e = _pMP3List;

				// Try to continue from previous song, if already started
				if(!_GetMP3PosFromStreamPos(&position, &e) && !e) {
					nFile = 0;
#ifdef PS2_AUDIO_PATHS
					strcpy(filename, m_MiscomPath);
					strcat(filename, PS2StreamedNameTable[nFile]);

					mp3Stream[nStream] = AIL_open_stream(DIG, filename, 0);

					if ( !mp3Stream[nStream] )
#endif
					{
						strcpy(filename, m_MiscomPath);
						strcat(filename, StreamedNameTable[nFile]);
						mp3Stream[nStream] =
						    AIL_open_stream(DIG, filename, 0);
					}
					if(mp3Stream[nStream]) {
						AIL_set_stream_loop_count(mp3Stream[nStream], nStreamLoopedFlag[nStream] ? 0 : 1);
						nStreamLoopedFlag[nStream] = TRUE;
						AIL_set_stream_ms_position(mp3Stream[nStream], position);
						AIL_pause_stream(mp3Stream[nStream], 0);
						return TRUE;
					}
					return FALSE;

				} else {
					if ( e->pLinkPath != NULL )
						mp3Stream[nStream] = AIL_open_stream(DIG, e->pLinkPath, 0);
					else {
						strcpy(filename, _mp3DirectoryPath);
						strcat(filename, e->aFilename);
					
						mp3Stream[nStream] = AIL_open_stream(DIG, filename, 0);
					}
										
					if ( mp3Stream[nStream] ) {
						AIL_set_stream_loop_count(mp3Stream[nStream], 1);
						AIL_set_stream_ms_position(mp3Stream[nStream], position);
						AIL_pause_stream(mp3Stream[nStream], 0);
						
						_bIsMp3Active = TRUE;
				
						return TRUE;
					}
					// fall through, start playing from another song
				}
			} else {
				if(++_CurMP3Index >= nNumMP3s) _CurMP3Index = 0;

				_CurMP3Pos = 0;

				tMP3Entry *mp3 = _GetMP3EntryByIndex(_CurMP3Index);
				if ( !mp3 )
				{
					mp3 = _pMP3List;
					if ( !_pMP3List )
					{
						nFile = 0;
						_bIsMp3Active = 0;
#ifdef PS2_AUDIO_PATHS
						strcpy(filename, m_MiscomPath);
						strcat(filename, PS2StreamedNameTable[nFile]);

						mp3Stream[nStream] = AIL_open_stream(DIG, filename, 0);

						if ( !mp3Stream[nStream] )
#endif
						{
							strcpy(filename, m_MiscomPath);
							strcat(filename, StreamedNameTable[nFile]);
							mp3Stream[nStream] =
							    AIL_open_stream(DIG, filename, 0);
						}
						if(mp3Stream[nStream]) {
							AIL_set_stream_loop_count(
							    mp3Stream[nStream], nStreamLoopedFlag[nStream] ? 0 : 1);
							nStreamLoopedFlag[nStream] = TRUE;
							AIL_set_stream_ms_position(
							    mp3Stream[nStream], position);
							AIL_pause_stream(mp3Stream[nStream],
							                 0);
							return TRUE;
						}
						return FALSE;
					}
				}
				if(mp3->pLinkPath != NULL)
					mp3Stream[nStream] = AIL_open_stream(DIG, mp3->pLinkPath, 0);
				else {
					strcpy(filename, _mp3DirectoryPath);
					strcat(filename, mp3->aFilename);

					mp3Stream[nStream] =
					    AIL_open_stream(DIG, filename, 0);
				}

				if(mp3Stream[nStream]) {
					AIL_set_stream_loop_count(mp3Stream[nStream], 1);
					AIL_set_stream_ms_position(mp3Stream[nStream], 0);
					AIL_pause_stream(mp3Stream[nStream], 0);
#ifdef FIX_BUGS
					_bIsMp3Active = TRUE;
#endif
					return TRUE;
				}

			}
			_bIsMp3Active = 0;
		}
		while ( ++i < nNumMP3s );
		position = 0;
		nFile = 0;
	}
#ifdef PS2_AUDIO_PATHS
	strcpy(filename, m_MiscomPath);
	strcat(filename, PS2StreamedNameTable[nFile]);

	mp3Stream[nStream] = AIL_open_stream(DIG, filename, 0);

	if ( !mp3Stream[nStream] )
#endif
	{
		strcpy(filename, m_MiscomPath);
		strcat(filename, StreamedNameTable[nFile]);
		mp3Stream[nStream] = AIL_open_stream(DIG, filename, 0);
	}

	if ( mp3Stream[nStream] )
	{
		AIL_set_stream_loop_count(mp3Stream[nStream], nStreamLoopedFlag[nStream] ? 0 : 1);
		nStreamLoopedFlag[nStream] = TRUE;
		AIL_set_stream_ms_position(mp3Stream[nStream], position);
		AIL_pause_stream(mp3Stream[nStream], 0);
		return TRUE;
	}
	return FALSE;
}

void
cSampleManager::StopStreamedFile(uint8 nStream)
{
	if ( m_bInitialised )
	{
		if ( mp3Stream[nStream] )
		{
			AIL_pause_stream(mp3Stream[nStream], 1);
			
			AIL_close_stream(mp3Stream[nStream]);
			mp3Stream[nStream] = NULL;
			
			if ( nStream == 0 )
				_bIsMp3Active = FALSE;
		}
	}
}

int32
cSampleManager::GetStreamedFilePosition(uint8 nStream)
{
	S32 currentms;
	
	if ( m_bInitialised )
	{
		if ( mp3Stream[nStream] )
		{
			if ( _bIsMp3Active )
			{
				tMP3Entry *mp3 = _GetMP3EntryByIndex(_CurMP3Index);
				
				if ( mp3 != NULL )
				{
					AIL_stream_ms_position(mp3Stream[nStream], NULL, &currentms);
					return currentms + mp3->nTrackStreamPos;
				}
				else
					return 0;
			}
			else
			{
				AIL_stream_ms_position(mp3Stream[nStream], NULL, &currentms);
				return currentms;
			}
		}
	}
	
	return 0;
}

void
cSampleManager::SetStreamedVolumeAndPan(uint8 nVolume, uint8 nPan, bool8 nEffectFlag, uint8 nStream)
{
	uint8 vol = nVolume;
	float boostMult = 0.0f;
	
	if ( m_bInitialised )
	{
		if ( vol > MAX_VOLUME ) vol = MAX_VOLUME;
		
		if ( MusicManager.GetRadioInCar() == USERTRACK && !MusicManager.CheckForMusicInterruptions() )
			boostMult = m_nMP3BoostVolume / 64.f;
		
		nStreamVolume[nStream] = vol;
		nStreamPan[nStream]    = nPan;
		
		if ( mp3Stream[nStream] )
		{
			if ( nEffectFlag )
			{
				if ( nStream == 1 || nStream == 2 )
					AIL_set_stream_volume(mp3Stream[nStream], 128*vol*m_nEffectsVolume >> 14);
				else
					AIL_set_stream_volume(mp3Stream[nStream], m_nEffectsFadeVolume*vol*m_nEffectsVolume >> 14);
			}
			else
				AIL_set_stream_volume(mp3Stream[nStream], (m_nMusicFadeVolume*vol*(uint32)(m_nMusicVolume * boostMult + m_nMusicVolume)) >> 14);
			
			AIL_set_stream_pan(mp3Stream[nStream], nPan);
		}
	}
}

int32
cSampleManager::GetStreamedFileLength(uint8 nStream)
{
	if ( m_bInitialised )
		return nStreamLength[nStream];
	
	return 0;
}

bool8
cSampleManager::IsStreamPlaying(uint8 nStream)
{
	if ( m_bInitialised )
	{
		if ( mp3Stream[nStream] )
		{
			if ( AIL_stream_status(mp3Stream[nStream]) == SMP_PLAYING )
				return TRUE;
			else
				return FALSE;
		}
	}
	
	return FALSE;
}

bool8
cSampleManager::InitialiseSampleBanks(void)
{
	int32 nBank = SFX_BANK_0;
	
	fpSampleDescHandle = fopen(SampleBankDescFilename, "rb");
	if ( fpSampleDescHandle == NULL )
		return FALSE;
	
	fpSampleDataHandle = fopen(SampleBankDataFilename, "rb");
	if ( fpSampleDataHandle == NULL )
	{
		fclose(fpSampleDescHandle);
		fpSampleDescHandle = NULL;
		
		return FALSE;
	}
	
	fseek(fpSampleDataHandle, 0, SEEK_END);
	_nSampleDataEndOffset = ftell(fpSampleDataHandle);
	rewind(fpSampleDataHandle);
	
	fread(m_aSamples, sizeof(tSample), TOTAL_AUDIO_SAMPLES, fpSampleDescHandle);
	
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
	nSampleBankSize[SFX_BANK_PED_COMMENTS] = _nSampleDataEndOffset                       - nSampleBankDiscStartOffset[SFX_BANK_PED_COMMENTS];
	
	return TRUE;
}


void
cSampleManager::SetStreamedFileLoopFlag(bool8 nLoopFlag, uint8 nChannel)
{
	if (m_bInitialised)
		nStreamLoopedFlag[nChannel] = nLoopFlag;
}

#endif
