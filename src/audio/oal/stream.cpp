#include "common.h"

#ifdef AUDIO_OAL
#include "stream.h"
#include "sampman.h"

#ifdef _WIN32
typedef long ssize_t;
#pragma comment( lib, "libsndfile-1.lib" )
#pragma comment( lib, "libmpg123.lib" )
#else
#include "crossplatform.h"
#endif
#include <sndfile.h>
#include <mpg123.h>

class CSndFile : public IDecoder
{
	SNDFILE *m_pfSound;
	SF_INFO m_soundInfo;
public:
	CSndFile(const char *path) :
		m_pfSound(nil)
	{
		memset(&m_soundInfo, 0, sizeof(m_soundInfo));
		m_pfSound = sf_open(path, SFM_READ, &m_soundInfo);
	}
	
	~CSndFile()
	{
		if ( m_pfSound )
		{
			sf_close(m_pfSound);
			m_pfSound = nil;
		}
	}
	
	bool IsOpened()
	{
		return m_pfSound != nil;
	}
	
	uint32 GetSampleSize()
	{
		return sizeof(uint16);
	}
	
	uint32 GetSampleCount()
	{
		return m_soundInfo.frames;
	}
	
	uint32 GetSampleRate()
	{
		return m_soundInfo.samplerate;
	}
	
	uint32 GetChannels()
	{
		return m_soundInfo.channels;
	}
	
	void Seek(uint32 milliseconds)
	{
		if ( !IsOpened() ) return;
		sf_seek(m_pfSound, ms2samples(milliseconds), SF_SEEK_SET);
	}
	
	uint32 Tell()
	{
		if ( !IsOpened() ) return 0;
		return samples2ms(sf_seek(m_pfSound, 0, SF_SEEK_CUR));
	}
	
	uint32 Decode(void *buffer)
	{
		if ( !IsOpened() ) return 0;
		return sf_read_short(m_pfSound, (short *)buffer, GetBufferSamples()) * GetSampleSize();
	}
};

class CMP3File : public IDecoder
{
protected:
	mpg123_handle *m_pMH;
	bool m_bOpened;
	uint32 m_nRate;
	uint32 m_nChannels;
	
	CMP3File() :
		m_pMH(nil),
		m_bOpened(false),
		m_nRate(0),
		m_nChannels(0) {}
public:
	CMP3File(const char *path) :
		m_pMH(nil),
		m_bOpened(false),
		m_nRate(0),
		m_nChannels(0)
	{
		m_pMH = mpg123_new(nil, nil);
		if ( m_pMH )
		{
			long rate = 0;
			int channels = 0;
			int encoding = 0;
			
			m_bOpened = mpg123_open(m_pMH, path) == MPG123_OK
				&& mpg123_getformat(m_pMH, &rate, &channels, &encoding) == MPG123_OK;
			m_nRate = rate;
			m_nChannels = channels;
			
			if ( IsOpened() )
			{
				mpg123_format_none(m_pMH);
				mpg123_format(m_pMH, rate, channels, encoding);
			}
		}
	}
	
	~CMP3File()
	{
		if ( m_pMH )
		{
			mpg123_close(m_pMH);
			mpg123_delete(m_pMH);
			m_pMH = nil;
		}
	}
	
	bool IsOpened()
	{
		return m_bOpened;
	}
	
	uint32 GetSampleSize()
	{
		return sizeof(uint16);
	}
	
	uint32 GetSampleCount()
	{
		if ( !IsOpened() ) return 0;
		return mpg123_length(m_pMH);
	}
	
	uint32 GetSampleRate()
	{
		return m_nRate;
	}
	
	uint32 GetChannels()
	{
		return m_nChannels;
	}
	
	void Seek(uint32 milliseconds)
	{
		if ( !IsOpened() ) return;
		mpg123_seek(m_pMH, ms2samples(milliseconds)*GetSampleSize(), SEEK_SET);
	}
	
	uint32 Tell()
	{
		if ( !IsOpened() ) return 0;
		return samples2ms(mpg123_tell(m_pMH)/GetSampleSize());
	}
	
	uint32 Decode(void *buffer)
	{
		if ( !IsOpened() ) return 0;
		
		size_t size;
		int err = mpg123_read(m_pMH, (unsigned char *)buffer, GetBufferSize(), &size);
		if (err != MPG123_OK && err != MPG123_DONE) return 0;
		return size;
	}
};

class CADFFile : public CMP3File
{
	static ssize_t r_read(void* fh, void* buf, size_t size)
	{
		size_t bytesRead = fread(buf, 1, size, (FILE*)fh);
		uint8* _buf = (uint8*)buf;
		for (int i = 0; i < size; i++)
			_buf[i] ^= 0x22;
		return bytesRead;
	}
	static off_t r_seek(void* fh, off_t pos, int seekType)
	{
		fseek((FILE*)fh, pos, seekType);
		return ftell((FILE*)fh);
	}
	static void r_close(void* fh)
	{
		fclose((FILE*)fh);
	}
public:
	CADFFile(const char* path)
	{
		m_pMH = mpg123_new(nil, nil);
		if (m_pMH)
		{
			long rate = 0;
			int channels = 0;
			int encoding = 0;

			FILE* f = fopen(path, "rb");

			m_bOpened = mpg123_replace_reader_handle(m_pMH, r_read, r_seek, r_close) == MPG123_OK
				&& mpg123_open_handle(m_pMH, f) == MPG123_OK &&  mpg123_getformat(m_pMH, &rate, &channels, &encoding) == MPG123_OK;
			m_nRate = rate;
			m_nChannels = channels;

			if (IsOpened())
			{
				mpg123_format_none(m_pMH);
				mpg123_format(m_pMH, rate, channels, encoding);
			}
		}
	}
};

void CStream::Initialise()
{
	mpg123_init();
}

void CStream::Terminate()
{
	mpg123_exit();
}

CStream::CStream(char *filename, ALuint &source, ALuint (&buffers)[NUM_STREAMBUFFERS]) :
	m_alSource(source),
	m_alBuffers(buffers),
	m_pBuffer(nil),
	m_bPaused(false),
	m_bActive(false),
	m_pSoundFile(nil),
	m_bReset(false),
	m_nVolume(0),
	m_nPan(0),
	m_nPosBeforeReset(0)
	
{
// Be case-insensitive on linux (from https://github.com/OneSadCookie/fcaseopen/)
#if !defined(_WIN32)
	FILE *test = fopen(filename, "r");
	if (!test) {
		char *r = (char*)alloca(strlen(filename) + 2);
		if (casepath(filename, r))
		{
		    strcpy(m_aFilename, r);
		}
	} else {
		fclose(test);
#else
	{
#endif
		strcpy(m_aFilename, filename);
	}
		
	DEV("Stream %s\n", m_aFilename);

	if (!strcasecmp(&m_aFilename[strlen(m_aFilename) - strlen(".mp3")], ".mp3"))
		m_pSoundFile = new CMP3File(m_aFilename);
	else if (!strcasecmp(&m_aFilename[strlen(m_aFilename) - strlen(".wav")], ".wav"))
		m_pSoundFile = new CSndFile(m_aFilename);
	else if (!strcasecmp(&m_aFilename[strlen(m_aFilename) - strlen(".adf")], ".adf"))
		m_pSoundFile = new CADFFile(m_aFilename);
	else 
		m_pSoundFile = nil;
	ASSERT(m_pSoundFile != nil);
	if (m_pSoundFile && m_pSoundFile->IsOpened() )
	{
		m_pBuffer            = malloc(m_pSoundFile->GetBufferSize());
		ASSERT(m_pBuffer!=nil);
		
		DEV("AvgSamplesPerSec: %d\n", m_pSoundFile->GetAvgSamplesPerSec());
		DEV("SampleCount: %d\n",      m_pSoundFile->GetSampleCount());
		DEV("SampleRate: %d\n",       m_pSoundFile->GetSampleRate());
		DEV("Channels: %d\n",         m_pSoundFile->GetChannels());
		DEV("Buffer Samples: %d\n",   m_pSoundFile->GetBufferSamples());
		DEV("Buffer sec: %f\n",       (float(m_pSoundFile->GetBufferSamples()) / float(m_pSoundFile->GetChannels())/ float(m_pSoundFile->GetSampleRate())));
		DEV("Length MS: %02d:%02d\n", (m_pSoundFile->GetLength() / 1000) / 60, (m_pSoundFile->GetLength() / 1000) % 60);
		
		return;
	}
}

CStream::~CStream()
{
	Delete();
}

void CStream::Delete()
{
	Stop();
	ClearBuffers();
	
	if ( m_pSoundFile )
	{
		delete m_pSoundFile;
		m_pSoundFile = nil;
	}
	
	if ( m_pBuffer )
	{
		free(m_pBuffer);
		m_pBuffer = nil;
	}
}

bool CStream::HasSource()
{
	return m_alSource != AL_NONE;
}

bool CStream::IsOpened()
{
	return m_pSoundFile->IsOpened();
}

bool CStream::IsPlaying()
{
	if ( !HasSource() || !IsOpened() ) return false;
	
	if ( m_pSoundFile->IsOpened() && !m_bPaused )
	{
		ALint sourceState;
		alGetSourcei(m_alSource, AL_SOURCE_STATE, &sourceState);
		if ( m_bActive || sourceState == AL_PLAYING )
			return true;
	}
	
	return false;
}

void CStream::Pause()
{
	if ( !HasSource() ) return;
	ALint sourceState = AL_PAUSED;
	alGetSourcei(m_alSource, AL_SOURCE_STATE, &sourceState);
	if (sourceState != AL_PAUSED )
		alSourcePause(m_alSource);
}

void CStream::SetPause(bool bPause)
{
	if ( !HasSource() ) return;
	if ( bPause )
	{
		Pause();
		m_bPaused = true;
	}
	else
	{
		if (m_bPaused)
			SetPlay(true);
		m_bPaused = false;
	}
}

void CStream::SetPitch(float pitch)
{
	if ( !HasSource() ) return;
	alSourcef(m_alSource, AL_PITCH, pitch);
}

void CStream::SetGain(float gain)
{
	if ( !HasSource() ) return;
	alSourcef(m_alSource, AL_GAIN, gain);
}

void CStream::SetPosition(float x, float y, float z)
{
	if ( !HasSource() ) return;
	alSource3f(m_alSource, AL_POSITION, x, y, z);
}

void CStream::SetVolume(uint32 nVol)
{
	m_nVolume = nVol;
	SetGain(ALfloat(nVol) / MAX_VOLUME);
}

void CStream::SetPan(uint8 nPan)
{
	m_nPan = nPan;
	SetPosition((nPan - 63)/64.0f, 0.0f, Sqrt(1.0f-SQR((nPan-63)/64.0f)));
}

void CStream::SetPosMS(uint32 nPos)
{
	if ( !m_pSoundFile->IsOpened() ) return;
	m_pSoundFile->Seek(nPos);
	ClearBuffers();
}

uint32 CStream::GetPosMS()
{
	if ( !HasSource() ) return 0;
	if ( !m_pSoundFile->IsOpened() ) return 0;
	
	ALint offset;
	//alGetSourcei(m_alSource, AL_SAMPLE_OFFSET, &offset);
	alGetSourcei(m_alSource, AL_BYTE_OFFSET, &offset);

	return m_pSoundFile->Tell()
		- m_pSoundFile->samples2ms(m_pSoundFile->GetBufferSamples() * (NUM_STREAMBUFFERS-1))
		+ m_pSoundFile->samples2ms(offset/m_pSoundFile->GetSampleSize());
}

uint32 CStream::GetLengthMS()
{
	if ( !m_pSoundFile->IsOpened() ) return 0;
	return m_pSoundFile->GetLength();
}

bool CStream::FillBuffer(ALuint alBuffer)
{
	if ( !HasSource() )
		return false;
	if ( !m_pSoundFile->IsOpened() )
		return false;
	if ( !(alBuffer != AL_NONE && alIsBuffer(alBuffer)) )
		return false;
	
	uint32 size = m_pSoundFile->Decode(m_pBuffer);
	if( size == 0 )
		return false;
	
	alBufferData(alBuffer, m_pSoundFile->GetChannels() == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16,
		m_pBuffer, size, m_pSoundFile->GetSampleRate());
	
	return true;
}

int32 CStream::FillBuffers()
{
	int32 i = 0;
	for ( i = 0; i < NUM_STREAMBUFFERS; i++ )
	{
		if ( !FillBuffer(m_alBuffers[i]) )
			break;
		alSourceQueueBuffers(m_alSource, 1, &m_alBuffers[i]);
	}
	
	return i;
}

void CStream::ClearBuffers()
{
	if ( !HasSource() ) return;
	
	ALint buffersQueued;
	alGetSourcei(m_alSource, AL_BUFFERS_QUEUED, &buffersQueued);

	ALuint value;
	while (buffersQueued--)
		alSourceUnqueueBuffers(m_alSource, 1, &value);
}

bool CStream::Setup()
{
	if ( m_pSoundFile->IsOpened() )
	{
		m_pSoundFile->Seek(0);
		alSourcei(m_alSource, AL_SOURCE_RELATIVE, AL_TRUE);
		//SetPosition(0.0f, 0.0f, 0.0f);
		SetPitch(1.0f);
		//SetPan(m_nPan);
		//SetVolume(100);
	}
	
	return IsOpened();
}

void CStream::SetPlay(bool state)
{
	if ( !HasSource() ) return;
	if ( state )
	{
		ALint sourceState = AL_PLAYING;
		alGetSourcei(m_alSource, AL_SOURCE_STATE, &sourceState);
		if (sourceState != AL_PLAYING )
			alSourcePlay(m_alSource);
		m_bActive = true;
	}
	else
	{
		ALint sourceState = AL_STOPPED;
		alGetSourcei(m_alSource, AL_SOURCE_STATE, &sourceState);
		if (sourceState != AL_STOPPED )
			alSourceStop(m_alSource);
		m_bActive = false;
	}
}

void CStream::Start()
{
	if ( !HasSource() ) return;
	if ( FillBuffers() != 0 )
		SetPlay(true);
}

void CStream::Stop()
{
	if ( !HasSource() ) return;
	SetPlay(false);
}

void CStream::Update()
{
	if ( !IsOpened() )
		return;
	
	if ( !HasSource() )
		return;
	
	if ( m_bReset )
		return;
	
	if ( !m_bPaused )
	{
		ALint sourceState;
		ALint buffersProcessed = 0;
		
		alGetSourcei(m_alSource, AL_SOURCE_STATE, &sourceState);
		alGetSourcei(m_alSource, AL_BUFFERS_PROCESSED, &buffersProcessed);
		
		ALint looping = AL_FALSE;
		alGetSourcei(m_alSource, AL_LOOPING, &looping);
		
		if ( looping == AL_TRUE )
		{
			TRACE("stream set looping");
			alSourcei(m_alSource, AL_LOOPING, AL_TRUE);
		}
		
		while( buffersProcessed-- )
		{
			ALuint buffer;
			
			alSourceUnqueueBuffers(m_alSource, 1, &buffer);
			
			if ( m_bActive && FillBuffer(buffer) )
				alSourceQueueBuffers(m_alSource, 1, &buffer);
		}
		
		if ( sourceState != AL_PLAYING )
		{
			alGetSourcei(m_alSource, AL_BUFFERS_PROCESSED, &buffersProcessed);
			SetPlay(buffersProcessed!=0);
		}
	}
}

void CStream::ProviderInit()
{
	if ( m_bReset )
	{
		if ( Setup() )
		{
			SetPan(m_nPan);
			SetVolume(m_nVolume);
			SetPosMS(m_nPosBeforeReset);
			if (m_bActive)
				FillBuffers();
			SetPlay(m_bActive);
			if ( m_bPaused )
				Pause();
		}
	
		m_bReset = false;
	}
}

void CStream::ProviderTerm()
{
	m_bReset = true;
	m_nPosBeforeReset = GetPosMS();
	
	ClearBuffers();
}
	
#endif
