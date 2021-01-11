#include "common.h"

#ifdef AUDIO_OAL
#include "stream.h"
#include "sampman.h"

#if defined _MSC_VER && !defined RE3_NO_AUTOLINK
#ifdef AUDIO_OAL_USE_SNDFILE
#pragma comment( lib, "libsndfile-1.lib" )
#endif
#ifdef AUDIO_OAL_USE_MPG123
#pragma comment( lib, "libmpg123-0.lib" )
#endif
#endif
#ifdef AUDIO_OAL_USE_SNDFILE
#include <sndfile.h>
#endif
#ifdef AUDIO_OAL_USE_MPG123
#include <mpg123.h>
#endif
#ifdef AUDIO_OAL_USE_OPUS
#include <opusfile.h>
#endif

#ifndef _WIN32
#include "crossplatform.h"
#endif

/*
As we ran onto an issue of having different volume levels for mono streams
and stereo streams we are now handling all the stereo panning ourselves.
Each stream now has two sources - one panned to the left and one to the right,
and uses two separate buffers to store data for each individual channel.
For that we also have to reshuffle all decoded PCM stereo data from LRLRLRLR to
LLLLRRRR (handled by CSortStereoBuffer).
*/

class CSortStereoBuffer
{
	uint16* PcmBuf;
	size_t BufSize;
public:
	CSortStereoBuffer() : PcmBuf(nil), BufSize(0) {}
	~CSortStereoBuffer()
	{
		if (PcmBuf)
			free(PcmBuf);
	}

	uint16* GetBuffer(size_t size)
	{
		if (size == 0) return nil;
		if (!PcmBuf)
		{
			BufSize = size;
			PcmBuf = (uint16*)malloc(BufSize);
		}
		else if (BufSize < size)
		{
			BufSize = size;
			PcmBuf = (uint16*)realloc(PcmBuf, size);
		}
		return PcmBuf;
	}

	void SortStereo(void* buf, size_t size)
	{
		uint16* InBuf = (uint16*)buf;
		uint16* OutBuf = GetBuffer(size);

		if (!OutBuf) return;

		size_t rightStart = size / 4;
		for (size_t i = 0; i < size / 4; i++)
		{
			OutBuf[i] = InBuf[i*2];
			OutBuf[i+rightStart] = InBuf[i*2+1];
		}

		memcpy(InBuf, OutBuf, size);
	}

};

CSortStereoBuffer SortStereoBuffer;

class CImaADPCMDecoder
{
	const uint16 StepTable[89] = {
		7, 8, 9, 10, 11, 12, 13, 14,
		16, 17, 19, 21, 23, 25, 28, 31,
		34, 37, 41, 45, 50, 55, 60, 66,
		73, 80, 88, 97, 107, 118, 130, 143,
		157, 173, 190, 209, 230, 253, 279, 307,
		337, 371, 408, 449, 494, 544, 598, 658,
		724, 796, 876, 963, 1060, 1166, 1282, 1411,
		1552, 1707, 1878, 2066, 2272, 2499, 2749, 3024,
		3327, 3660, 4026, 4428, 4871, 5358, 5894, 6484,
		7132, 7845, 8630, 9493, 10442, 11487, 12635, 13899,
		15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794,
		32767
	};

	int16 Sample, StepIndex;

public:
	CImaADPCMDecoder()
	{
		Init(0, 0);
	}

	void Init(int16 _Sample, int16 _StepIndex)
	{
		Sample = _Sample;
		StepIndex = _StepIndex;
	}

	void Decode(uint8 *inbuf, int16 *_outbuf, size_t size)
	{
		int16* outbuf = _outbuf;
		for (size_t i = 0; i < size; i++)
		{
			*(outbuf++) = DecodeSample(inbuf[i] & 0xF);
			*(outbuf++) = DecodeSample(inbuf[i] >> 4);
		}
	}

	int16 DecodeSample(uint8 adpcm)
	{
		uint16 step = StepTable[StepIndex];

		if (adpcm & 4)
			StepIndex += ((adpcm & 3) + 1) * 2;
		else
			StepIndex--;

		StepIndex = clamp(StepIndex, 0, 88);

		int delta = step >> 3;
		if (adpcm & 1) delta += step >> 2;
		if (adpcm & 2) delta += step >> 1;
		if (adpcm & 4) delta += step;
		if (adpcm & 8) delta = -delta;

		int newSample = Sample + delta;
		Sample = clamp(newSample, -32768, 32767);
		return Sample;
	}
};

class CWavFile : public IDecoder
{
	enum
	{
		WAVEFMT_PCM = 1,
		WAVEFMT_IMA_ADPCM = 0x11,
		WAVEFMT_XBOX_ADPCM = 0x69,
	};

	struct tDataHeader
	{
		uint32 ID;
		uint32 Size;
	};

	struct tFormatHeader
	{
		uint16 AudioFormat;
		uint16 NumChannels;
		uint32 SampleRate;
		uint32 ByteRate;
		uint16 BlockAlign;
		uint16 BitsPerSample;
		uint16 extra[2]; // adpcm only

		tFormatHeader() { memset(this, 0, sizeof(*this)); }
	};

	FILE *m_pFile;
	bool  m_bIsOpen;

	tFormatHeader m_FormatHeader;

	uint32 m_DataStartOffset; // TODO: 64 bit?
	uint32 m_nSampleCount;
	uint32 m_nSamplesPerBlock;

	// ADPCM things
	uint8            *m_pAdpcmBuffer;
	int16           **m_ppPcmBuffers;
	CImaADPCMDecoder *m_pAdpcmDecoders;

	void Close()
	{
		if (m_pFile) {
			fclose(m_pFile);
			m_pFile = nil;
		}
		delete[] m_pAdpcmBuffer;
		delete[] m_ppPcmBuffers;
		delete[] m_pAdpcmDecoders;
	}

	uint32 GetCurrentSample() const
	{
		// TODO: 64 bit?
		uint32 FilePos = ftell(m_pFile);
		if (FilePos <= m_DataStartOffset)
			return 0;
		return (FilePos - m_DataStartOffset) / m_FormatHeader.BlockAlign * m_nSamplesPerBlock;
	}

public:
	CWavFile(const char* path) : m_bIsOpen(false), m_DataStartOffset(0), m_nSampleCount(0), m_nSamplesPerBlock(0), m_pAdpcmBuffer(nil), m_ppPcmBuffers(nil), m_pAdpcmDecoders(nil)
	{
		m_pFile = fopen(path, "rb");
		if (!m_pFile) return;

#define CLOSE_ON_ERROR(op)\
			if (op) { \
				Close(); \
				return; \
			}

		tDataHeader DataHeader;

		CLOSE_ON_ERROR(fread(&DataHeader, sizeof(DataHeader), 1, m_pFile) == 0);
		CLOSE_ON_ERROR(DataHeader.ID != 'FFIR');

		// TODO? validate filesizes

		int WAVE;
		CLOSE_ON_ERROR(fread(&WAVE, 4, 1, m_pFile) == 0);
		CLOSE_ON_ERROR(WAVE != 'EVAW')
		CLOSE_ON_ERROR(fread(&DataHeader, sizeof(DataHeader), 1, m_pFile) == 0);
		CLOSE_ON_ERROR(DataHeader.ID != ' tmf');

		CLOSE_ON_ERROR(fread(&m_FormatHeader, Min(DataHeader.Size, sizeof(tFormatHeader)), 1, m_pFile) == 0);
		CLOSE_ON_ERROR(DataHeader.Size > sizeof(tFormatHeader));

		switch (m_FormatHeader.AudioFormat)
		{
		case WAVEFMT_XBOX_ADPCM:
			m_FormatHeader.AudioFormat = WAVEFMT_IMA_ADPCM;
		case WAVEFMT_IMA_ADPCM:
			m_nSamplesPerBlock = (m_FormatHeader.BlockAlign / m_FormatHeader.NumChannels - 4) * 2 + 1;
			m_pAdpcmBuffer = new uint8[m_FormatHeader.BlockAlign];
			m_ppPcmBuffers = new int16*[m_FormatHeader.NumChannels];
			m_pAdpcmDecoders = new CImaADPCMDecoder[m_FormatHeader.NumChannels];
			break;
		case WAVEFMT_PCM:
			m_nSamplesPerBlock = 1;
			if (m_FormatHeader.BitsPerSample != 16)
			{
				debug("Unsupported PCM (%d bits), only signed 16-bit is supported (%s)\n", m_FormatHeader.BitsPerSample, path);
				Close();
				return;
			}
			break;
		default:
			debug("Unsupported wav format 0x%x (%s)\n", m_FormatHeader.AudioFormat, path);
			Close();
			return;
		}

		while (true) {
			CLOSE_ON_ERROR(fread(&DataHeader, sizeof(DataHeader), 1, m_pFile) == 0);
			if (DataHeader.ID == 'atad')
				break;
			fseek(m_pFile, DataHeader.Size, SEEK_CUR);
			// TODO? validate data size
			// maybe check if there no extreme custom headers that might break this
		}
		
		m_DataStartOffset = ftell(m_pFile);
		m_nSampleCount = DataHeader.Size / m_FormatHeader.BlockAlign * m_nSamplesPerBlock;

		m_bIsOpen = true;
#undef CLOSE_ON_ERROR
	}

	~CWavFile()
	{
		Close();
	}

	bool IsOpened()
	{
		return m_bIsOpen;
	}

	uint32 GetSampleSize()
	{
		return sizeof(uint16);
	}

	uint32 GetSampleCount()
	{
		return m_nSampleCount;
	}

	uint32 GetSampleRate()
	{
		return m_FormatHeader.SampleRate;
	}

	uint32 GetChannels()
	{
		return m_FormatHeader.NumChannels;
	}

	void Seek(uint32 milliseconds)
	{
		if (!IsOpened()) return;
		fseek(m_pFile, m_DataStartOffset + ms2samples(milliseconds) / m_nSamplesPerBlock * m_FormatHeader.BlockAlign, SEEK_SET);
	}

	uint32 Tell()
	{
		if (!IsOpened()) return 0;
		return samples2ms(GetCurrentSample());
	}

#define SAMPLES_IN_LINE (8)

	uint32 Decode(void* buffer)
	{
		if (!IsOpened()) return 0;
		
		if (m_FormatHeader.AudioFormat == WAVEFMT_PCM)
		{
			// just read the file and sort the samples
			uint32 size = fread(buffer, 1, GetBufferSize(), m_pFile);
			if (m_FormatHeader.NumChannels == 2)
				SortStereoBuffer.SortStereo(buffer, size);
			return size;
		}
		else if (m_FormatHeader.AudioFormat == WAVEFMT_IMA_ADPCM)
		{
			// trim the buffer size if we're at the end of our file
			uint32 nMaxSamples = GetBufferSamples() / m_FormatHeader.NumChannels;
			uint32 nSamplesLeft = m_nSampleCount - GetCurrentSample();
			nMaxSamples = Min(nMaxSamples, nSamplesLeft);

			// align sample count to our block
			nMaxSamples = nMaxSamples / m_nSamplesPerBlock * m_nSamplesPerBlock;

			// count the size of output buffer
			uint32 OutBufSizePerChannel = nMaxSamples * GetSampleSize();
			uint32 OutBufSize = OutBufSizePerChannel * m_FormatHeader.NumChannels;

			// calculate the pointers to individual channel buffers
			for (uint32 i = 0; i < m_FormatHeader.NumChannels; i++)
				m_ppPcmBuffers[i] = (int16*)((int8*)buffer + OutBufSizePerChannel * i);

			uint32 samplesRead = 0;
			while (samplesRead < nMaxSamples)
			{
				// read the file
				uint8 *pAdpcmBuf = m_pAdpcmBuffer;
				if (fread(m_pAdpcmBuffer, 1, m_FormatHeader.BlockAlign, m_pFile) == 0)
					return 0;

				// get the first sample in adpcm block and initialise the decoder(s)
				for (uint32 i = 0; i < m_FormatHeader.NumChannels; i++)
				{
					int16 Sample = *(int16*)pAdpcmBuf;
					pAdpcmBuf += sizeof(int16);
					int16 Step = *(int16*)pAdpcmBuf;
					pAdpcmBuf += sizeof(int16);
					m_pAdpcmDecoders[i].Init(Sample, Step);
					*(m_ppPcmBuffers[i]) = Sample;
					m_ppPcmBuffers[i]++;
				}
				samplesRead++;

				// decode the rest of the block
				for (uint32 s = 1; s < m_nSamplesPerBlock; s += SAMPLES_IN_LINE)
				{
					for (uint32 i = 0; i < m_FormatHeader.NumChannels; i++)
					{
						m_pAdpcmDecoders[i].Decode(pAdpcmBuf, m_ppPcmBuffers[i], SAMPLES_IN_LINE / 2);
						pAdpcmBuf += SAMPLES_IN_LINE / 2;
						m_ppPcmBuffers[i] += SAMPLES_IN_LINE;
					}
					samplesRead += SAMPLES_IN_LINE;
				}
			}
			return OutBufSize;
		}
		return 0;
	}
};

#ifdef AUDIO_OAL_USE_SNDFILE
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

		size_t size = sf_read_short(m_pfSound, (short*)buffer, GetBufferSamples()) * GetSampleSize();
		if (GetChannels()==2)
			SortStereoBuffer.SortStereo(buffer, size);
		return size;
	}
};
#endif

#ifdef AUDIO_OAL_USE_MPG123
// fuzzy seek eliminates stutter when playing ADF but spams errors a lot (nothing breaks though)
#define MP3_USE_FUZZY_SEEK

class CMP3File : public IDecoder
{
	mpg123_handle *m_pMH;
	bool m_bOpened;
	uint32 m_nRate;
	uint32 m_nChannels;
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
#ifdef MP3_USE_FUZZY_SEEK
			mpg123_param(m_pMH, MPG123_FLAGS, MPG123_FUZZY | MPG123_SEEKBUFFER | MPG123_GAPLESS | MPG123_QUIET, 0.0);
#endif
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
		mpg123_seek(m_pMH, ms2samples(milliseconds), SEEK_SET);
	}
	
	uint32 Tell()
	{
		if ( !IsOpened() ) return 0;
		return samples2ms(mpg123_tell(m_pMH));
	}
	
	uint32 Decode(void *buffer)
	{
		if ( !IsOpened() ) return 0;
		
		size_t size;
		int err = mpg123_read(m_pMH, (unsigned char *)buffer, GetBufferSize(), &size);
#if defined(__LP64__) || defined(_WIN64)
		assert("We can't handle audio files more then 2 GB yet :shrug:" && (size < UINT32_MAX));
#endif
		if (err != MPG123_OK && err != MPG123_DONE) return 0;
		if (GetChannels() == 2)
			SortStereoBuffer.SortStereo(buffer, size);
		return (uint32)size;
	}
};

#endif
#define VAG_LINE_SIZE (0x10)
#define VAG_SAMPLES_IN_LINE (28)

class CVagDecoder
{
	const double f[5][2] = { { 0.0, 0.0 },
					{  60.0 / 64.0,  0.0 },
					{  115.0 / 64.0, -52.0 / 64.0 },
					{  98.0 / 64.0, -55.0 / 64.0 },
					{  122.0 / 64.0, -60.0 / 64.0 } };

	double s_1;
	double s_2;
public:
	CVagDecoder()
	{
		ResetState();
	}

	void ResetState()
	{
		s_1 = s_2 = 0.0;
	}

	static short quantize(double sample)
	{
		int a = int(sample + 0.5);
		return short(clamp(a, -32768, 32767));
	}

	void Decode(void* _inbuf, int16* _outbuf, size_t size)
	{
		uint8* inbuf = (uint8*)_inbuf;
		int16* outbuf = _outbuf;
		size &= ~(VAG_LINE_SIZE - 1);

		while (size > 0) {
			double samples[VAG_SAMPLES_IN_LINE];

			int predict_nr, shift_factor, flags;
			predict_nr = *(inbuf++);
			shift_factor = predict_nr & 0xf;
			predict_nr >>= 4;
			flags = *(inbuf++);
			if (flags == 7) // TODO: ignore?
				break;
			for (int i = 0; i < VAG_SAMPLES_IN_LINE; i += 2) {
				int d = *(inbuf++);
				int16 s = int16((d & 0xf) << 12);
				samples[i] = (double)(s >> shift_factor);
				s = int16((d & 0xf0) << 8);
				samples[i + 1] = (double)(s >> shift_factor);
			}

			for (int i = 0; i < VAG_SAMPLES_IN_LINE; i++) {
				samples[i] = samples[i] + s_1 * f[predict_nr][0] + s_2 * f[predict_nr][1];
				s_2 = s_1;
				s_1 = samples[i];
				*(outbuf++) = quantize(samples[i] + 0.5);
			}
			size -= VAG_LINE_SIZE;
		}
	}
};

#define VB_BLOCK_SIZE (0x2000)
#define NUM_VAG_LINES_IN_BLOCK (VB_BLOCK_SIZE / VAG_LINE_SIZE)
#define NUM_VAG_SAMPLES_IN_BLOCK (NUM_VAG_LINES_IN_BLOCK * VAG_SAMPLES_IN_LINE)

class CVbFile : public IDecoder
{
	FILE        *m_pFile;
	CVagDecoder *m_pVagDecoders;

	size_t  m_FileSize;
	size_t  m_nNumberOfBlocks;

	uint32  m_nSampleRate;
	uint8   m_nChannels;
	bool    m_bBlockRead;
	uint16  m_LineInBlock;
	size_t  m_CurrentBlock;

	uint8 **m_ppVagBuffers; // buffers that cache actual ADPCM file data
	int16 **m_ppPcmBuffers;

	void ReadBlock(int32 block = -1)
	{
		// just read next block if -1
		if (block != -1)
			fseek(m_pFile, block * m_nChannels * VB_BLOCK_SIZE, SEEK_SET);

		for (int i = 0; i < m_nChannels; i++)
			fread(m_ppVagBuffers[i], VB_BLOCK_SIZE, 1, m_pFile);
		m_bBlockRead = true;
	}

public:
	CVbFile(const char* path, uint32 nSampleRate = 32000, uint8 nChannels = 2) : m_nSampleRate(nSampleRate), m_nChannels(nChannels), m_pVagDecoders(nil), m_ppVagBuffers(nil), m_ppPcmBuffers(nil),
		m_FileSize(0), m_nNumberOfBlocks(0), m_bBlockRead(false), m_LineInBlock(0), m_CurrentBlock(0)
	{
		m_pFile = fopen(path, "rb");
		if (!m_pFile) return;

		fseek(m_pFile, 0, SEEK_END);
		m_FileSize = ftell(m_pFile);
		fseek(m_pFile, 0, SEEK_SET);

		m_nNumberOfBlocks = m_FileSize / (nChannels * VB_BLOCK_SIZE);
		m_pVagDecoders = new CVagDecoder[nChannels];
		m_ppVagBuffers = new uint8*[nChannels];
		m_ppPcmBuffers = new int16*[nChannels];
		for (uint8 i = 0; i < nChannels; i++)
			m_ppVagBuffers[i] = new uint8[VB_BLOCK_SIZE];
	}

	~CVbFile()
	{
		if (m_pFile)
		{
			fclose(m_pFile);

			delete[] m_pVagDecoders;
			for (int i = 0; i < m_nChannels; i++)
				delete[] m_ppVagBuffers[i];
			delete[] m_ppVagBuffers;
			delete[] m_ppPcmBuffers;
		}
	}

	bool IsOpened()
	{
		return m_pFile != nil;
	}

	uint32 GetSampleSize()
	{
		return sizeof(uint16);
	}

	uint32 GetSampleCount()
	{
		if (!IsOpened()) return 0;
		return m_nNumberOfBlocks * NUM_VAG_LINES_IN_BLOCK * VAG_SAMPLES_IN_LINE;
	}

	uint32 GetSampleRate()
	{
		return m_nSampleRate;
	}

	uint32 GetChannels()
	{
		return m_nChannels;
	}

	void Seek(uint32 milliseconds)
	{
		if (!IsOpened()) return;
		uint32 samples = ms2samples(milliseconds);

		// find the block of our sample
		uint32 block = samples / NUM_VAG_SAMPLES_IN_BLOCK;
		if (block > m_nNumberOfBlocks)
		{
			samples = 0;
			block = 0;
		}
		if (block != m_CurrentBlock)
			m_bBlockRead = false;

		// find a line of our sample within our block
		uint32 remainingSamples = samples - block * NUM_VAG_SAMPLES_IN_BLOCK;
		uint32 newLine = remainingSamples / VAG_SAMPLES_IN_LINE / VAG_LINE_SIZE;

		if (m_CurrentBlock != block || m_LineInBlock != newLine)
		{
			m_CurrentBlock = block;
			m_LineInBlock = newLine;
			for (uint32 i = 0; i < GetChannels(); i++)
				m_pVagDecoders[i].ResetState();
		}

	}

	uint32 Tell()
	{
		if (!IsOpened()) return 0;
		uint32 pos = (m_CurrentBlock * NUM_VAG_LINES_IN_BLOCK + m_LineInBlock) * VAG_SAMPLES_IN_LINE;
		return samples2ms(pos);
	}

	uint32 Decode(void* buffer)
	{
		if (!IsOpened()) return 0;

		if (m_CurrentBlock >= m_nNumberOfBlocks) return 0;

		// cache current ADPCM block
		if (!m_bBlockRead)
			ReadBlock(m_CurrentBlock);

		// trim the buffer size if we're at the end of our file
		int numberOfRequiredLines = GetBufferSamples() / m_nChannels / VAG_SAMPLES_IN_LINE;
		int numberOfRemainingLines = (m_nNumberOfBlocks - m_CurrentBlock) * NUM_VAG_LINES_IN_BLOCK - m_LineInBlock;
		int bufSizePerChannel = Min(numberOfRequiredLines, numberOfRemainingLines) * VAG_SAMPLES_IN_LINE * GetSampleSize();

		// calculate the pointers to individual channel buffers
		for (uint32 i = 0; i < m_nChannels; i++)
			m_ppPcmBuffers[i] = (int16*)((int8*)buffer + bufSizePerChannel * i);

		int size = 0;
		while (size < bufSizePerChannel)
		{
			// decode the VAG lines
			for (uint32 i = 0; i < m_nChannels; i++)
			{
				m_pVagDecoders[i].Decode(m_ppVagBuffers[i] + m_LineInBlock * VAG_LINE_SIZE, m_ppPcmBuffers[i], VAG_LINE_SIZE);
				m_ppPcmBuffers[i] += VAG_SAMPLES_IN_LINE;
			}
			size += VAG_SAMPLES_IN_LINE * GetSampleSize();
			m_LineInBlock++;

			// block is over, read the next block
			if (m_LineInBlock >= NUM_VAG_LINES_IN_BLOCK)
			{
				m_CurrentBlock++;
				if (m_CurrentBlock >= m_nNumberOfBlocks) // end of file
					break;
				m_LineInBlock = 0;
				ReadBlock();
			}
		}

		return bufSizePerChannel * m_nChannels;
	}
};
#ifdef AUDIO_OAL_USE_OPUS
class COpusFile : public IDecoder
{
	OggOpusFile *m_FileH;
	bool m_bOpened;
	uint32 m_nRate;
	uint32 m_nChannels;
public:
	COpusFile(const char *path) : m_FileH(nil),
		m_bOpened(false),
		m_nRate(0),
		m_nChannels(0)
	{
		int ret;
		m_FileH = op_open_file(path, &ret);

		if (m_FileH) {
			m_nChannels = op_head(m_FileH, 0)->channel_count;
			m_nRate = 48000;
			const OpusTags *tags = op_tags(m_FileH, 0);
			for (int i = 0; i < tags->comments; i++) {
				if (strncmp(tags->user_comments[i], "SAMPLERATE", sizeof("SAMPLERATE")-1) == 0)
				{
					sscanf(tags->user_comments[i], "SAMPLERATE=%i", &m_nRate);
					break;
				}
			}
			
			m_bOpened = true;
		}
	}
	
	~COpusFile()
	{
		if (m_FileH)
		{
			op_free(m_FileH);
			m_FileH = nil;
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
		return op_pcm_total(m_FileH, 0);
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
		op_pcm_seek(m_FileH, ms2samples(milliseconds) / GetChannels());
	}
	
	uint32 Tell()
	{
		if ( !IsOpened() ) return 0;
		return samples2ms(op_pcm_tell(m_FileH) * GetChannels());
	}
	
	uint32 Decode(void *buffer)
	{
		if ( !IsOpened() ) return 0;

		int size = op_read(m_FileH, (opus_int16 *)buffer, GetBufferSamples(), NULL);

		if (size < 0)
			return 0;

		if (GetChannels() == 2)
			SortStereoBuffer.SortStereo(buffer, size * m_nChannels * GetSampleSize());

		return size * m_nChannels * GetSampleSize();
	}
};
#endif

void CStream::Initialise()
{
#ifdef AUDIO_OAL_USE_MPG123
	mpg123_init();
#endif
}

void CStream::Terminate()
{
#ifdef AUDIO_OAL_USE_MPG123
	mpg123_exit();
#endif
}

CStream::CStream(char *filename, ALuint *sources, ALuint (&buffers)[NUM_STREAMBUFFERS], uint32 overrideSampleRate) :
	m_pAlSources(sources),
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
	char *real = casepath(filename);
	if (real) {
		strcpy(m_aFilename, real);
		free(real);
	} else {
#else
	{
#endif
		strcpy(m_aFilename, filename);
	}
		
	DEV("Stream %s\n", m_aFilename);

	if (!strcasecmp(&m_aFilename[strlen(m_aFilename) - strlen(".wav")], ".wav"))
#ifdef AUDIO_OAL_USE_SNDFILE
		m_pSoundFile = new CSndFile(m_aFilename);
#else
		m_pSoundFile = new CWavFile(m_aFilename);
#endif
#ifdef AUDIO_OAL_USE_MPG123
	else if (!strcasecmp(&m_aFilename[strlen(m_aFilename) - strlen(".mp3")], ".mp3"))
		m_pSoundFile = new CMP3File(m_aFilename);
#endif
	else if (!strcasecmp(&m_aFilename[strlen(m_aFilename) - strlen(".vb")], ".VB"))
		m_pSoundFile = new CVbFile(m_aFilename, overrideSampleRate);
#ifdef AUDIO_OAL_USE_OPUS
	else if (!strcasecmp(&m_aFilename[strlen(m_aFilename) - strlen(".opus")], ".opus"))
		m_pSoundFile = new COpusFile(m_aFilename);
#endif
	else 
		m_pSoundFile = nil;

	if ( IsOpened() )
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
	return (m_pAlSources[0] != AL_NONE) && (m_pAlSources[1] != AL_NONE);
}

bool CStream::IsOpened()
{
	return m_pSoundFile && m_pSoundFile->IsOpened();
}

bool CStream::IsPlaying()
{
	if ( !HasSource() || !IsOpened() ) return false;
	
	if ( !m_bPaused )
	{
		ALint sourceState[2];
		alGetSourcei(m_pAlSources[0], AL_SOURCE_STATE, &sourceState[0]);
		alGetSourcei(m_pAlSources[1], AL_SOURCE_STATE, &sourceState[1]);
		if ( m_bActive || sourceState[0] == AL_PLAYING || sourceState[1] == AL_PLAYING)
			return true;
	}
	
	return false;
}

void CStream::Pause()
{
	if ( !HasSource() ) return;
	ALint sourceState = AL_PAUSED;
	alGetSourcei(m_pAlSources[0], AL_SOURCE_STATE, &sourceState);
	if (sourceState != AL_PAUSED)
		alSourcePause(m_pAlSources[0]);
	alGetSourcei(m_pAlSources[1], AL_SOURCE_STATE, &sourceState);
	if (sourceState != AL_PAUSED)
		alSourcePause(m_pAlSources[1]);
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
	alSourcef(m_pAlSources[0], AL_PITCH, pitch);
	alSourcef(m_pAlSources[1], AL_PITCH, pitch);
}

void CStream::SetGain(float gain)
{
	if ( !HasSource() ) return;
	alSourcef(m_pAlSources[0], AL_GAIN, gain);
	alSourcef(m_pAlSources[1], AL_GAIN, gain);
}

void CStream::SetPosition(int i, float x, float y, float z)
{
	if ( !HasSource() ) return;
	alSource3f(m_pAlSources[i], AL_POSITION, x, y, z);
}

void CStream::SetVolume(uint32 nVol)
{
	m_nVolume = nVol;
	SetGain(ALfloat(nVol) / MAX_VOLUME);
}

void CStream::SetPan(uint8 nPan)
{
	m_nPan = clamp((int8)nPan - 63, 0, 63);
	SetPosition(0, (m_nPan - 63) / 64.0f, 0.0f, Sqrt(1.0f - SQR((m_nPan - 63) / 64.0f)));

	m_nPan = clamp((int8)nPan + 64, 64, 127);
	SetPosition(1, (m_nPan - 63) / 64.0f, 0.0f, Sqrt(1.0f - SQR((m_nPan - 63) / 64.0f)));

	m_nPan = nPan;
}

void CStream::SetPosMS(uint32 nPos)
{
	if ( !IsOpened() ) return;
	m_pSoundFile->Seek(nPos);
	ClearBuffers();
}

uint32 CStream::GetPosMS()
{
	if ( !HasSource() ) return 0;
	if ( !IsOpened() ) return 0;
	
	ALint offset;
	//alGetSourcei(m_alSource, AL_SAMPLE_OFFSET, &offset);
	alGetSourcei(m_pAlSources[0], AL_BYTE_OFFSET, &offset);

	return m_pSoundFile->Tell()
		- m_pSoundFile->samples2ms(m_pSoundFile->GetBufferSamples() * (NUM_STREAMBUFFERS/2-1)) / m_pSoundFile->GetChannels()
		+ m_pSoundFile->samples2ms(offset/m_pSoundFile->GetSampleSize()) / m_pSoundFile->GetChannels();
}

uint32 CStream::GetLengthMS()
{
	if ( !IsOpened() ) return 0;
	return m_pSoundFile->GetLength();
}

bool CStream::FillBuffer(ALuint *alBuffer)
{
	if ( !HasSource() )
		return false;
	if ( !IsOpened() )
		return false;
	if ( !(alBuffer[0] != AL_NONE && alIsBuffer(alBuffer[0])) )
		return false;
	if ( !(alBuffer[1] != AL_NONE && alIsBuffer(alBuffer[1])) )
		return false;
	
	uint32 size = m_pSoundFile->Decode(m_pBuffer);
	if( size == 0 )
		return false;

	uint32 channelSize = size / m_pSoundFile->GetChannels();
	
	alBufferData(alBuffer[0], AL_FORMAT_MONO16, m_pBuffer, channelSize, m_pSoundFile->GetSampleRate());
	// TODO: use just one buffer if we play mono
	if (m_pSoundFile->GetChannels() == 1)
		alBufferData(alBuffer[1], AL_FORMAT_MONO16, m_pBuffer, channelSize, m_pSoundFile->GetSampleRate());
	else
		alBufferData(alBuffer[1], AL_FORMAT_MONO16, (uint8*)m_pBuffer + channelSize, channelSize, m_pSoundFile->GetSampleRate());
	return true;
}

int32 CStream::FillBuffers()
{
	int32 i = 0;
	for ( i = 0; i < NUM_STREAMBUFFERS/2; i++ )
	{
		if ( !FillBuffer(&m_alBuffers[i*2]) )
			break;
		alSourceQueueBuffers(m_pAlSources[0], 1, &m_alBuffers[i*2]);
		alSourceQueueBuffers(m_pAlSources[1], 1, &m_alBuffers[i*2+1]);
	}
	
	return i;
}

void CStream::ClearBuffers()
{
	if ( !HasSource() ) return;

	ALint buffersQueued[2];
	alGetSourcei(m_pAlSources[0], AL_BUFFERS_QUEUED, &buffersQueued[0]);
	alGetSourcei(m_pAlSources[1], AL_BUFFERS_QUEUED, &buffersQueued[1]);

	ALuint value;
	while (buffersQueued[0]--)
		alSourceUnqueueBuffers(m_pAlSources[0], 1, &value);
	while (buffersQueued[1]--)
		alSourceUnqueueBuffers(m_pAlSources[1], 1, &value);
}

bool CStream::Setup()
{
	if ( IsOpened() )
	{
		m_pSoundFile->Seek(0);
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
		alGetSourcei(m_pAlSources[0], AL_SOURCE_STATE, &sourceState);
		if (sourceState != AL_PLAYING )
			alSourcePlay(m_pAlSources[0]);

		sourceState = AL_PLAYING;
		alGetSourcei(m_pAlSources[1], AL_SOURCE_STATE, &sourceState);
		if (sourceState != AL_PLAYING)
			alSourcePlay(m_pAlSources[1]);

		m_bActive = true;
	}
	else
	{
		ALint sourceState = AL_STOPPED;
		alGetSourcei(m_pAlSources[0], AL_SOURCE_STATE, &sourceState);
		if (sourceState != AL_STOPPED)
			alSourceStop(m_pAlSources[0]);

		sourceState = AL_STOPPED;
		alGetSourcei(m_pAlSources[1], AL_SOURCE_STATE, &sourceState);
		if (sourceState != AL_STOPPED)
			alSourceStop(m_pAlSources[1]);

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
		ALint sourceState[2];
		ALint buffersProcessed[2] = { 0, 0 };
		
		// Relying a lot on left buffer states in here

		do
		{
			//alSourcef(m_pAlSources[0], AL_ROLLOFF_FACTOR, 0.0f);
			alGetSourcei(m_pAlSources[0], AL_SOURCE_STATE, &sourceState[0]);
			alGetSourcei(m_pAlSources[0], AL_BUFFERS_PROCESSED, &buffersProcessed[0]);
			//alSourcef(m_pAlSources[1], AL_ROLLOFF_FACTOR, 0.0f);
			alGetSourcei(m_pAlSources[1], AL_SOURCE_STATE, &sourceState[1]);
			alGetSourcei(m_pAlSources[1], AL_BUFFERS_PROCESSED, &buffersProcessed[1]);
		} while (buffersProcessed[0] != buffersProcessed[1]);
		
		ALint looping = AL_FALSE;
		alGetSourcei(m_pAlSources[0], AL_LOOPING, &looping);
		
		if ( looping == AL_TRUE )
		{
			TRACE("stream set looping");
			alSourcei(m_pAlSources[0], AL_LOOPING, AL_TRUE);
			alSourcei(m_pAlSources[1], AL_LOOPING, AL_TRUE);
		}

		assert(buffersProcessed[0] == buffersProcessed[1]);
		
		while( buffersProcessed[0]-- )
		{
			ALuint buffer[2];
			
			alSourceUnqueueBuffers(m_pAlSources[0], 1, &buffer[0]);
			alSourceUnqueueBuffers(m_pAlSources[1], 1, &buffer[1]);
			
			if (m_bActive && FillBuffer(buffer))
			{
				alSourceQueueBuffers(m_pAlSources[0], 1, &buffer[0]);
				alSourceQueueBuffers(m_pAlSources[1], 1, &buffer[1]);
			}
		}
		
		if ( sourceState[0] != AL_PLAYING )
		{
			alGetSourcei(m_pAlSources[0], AL_BUFFERS_PROCESSED, &buffersProcessed[0]);
			SetPlay(buffersProcessed[0]!=0);
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
