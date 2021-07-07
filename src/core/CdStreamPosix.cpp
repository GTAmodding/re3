#ifndef _WIN32
#include "common.h"
#include "crossplatform.h"
#include <signal.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/statvfs.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <stdarg.h>
#include <limits.h>

#ifdef __linux__
#include <sys/syscall.h>
#endif

#include "CdStream.h"
#include "rwcore.h"
#include "MemoryMgr.h"

#define CDDEBUG(f, ...)   debug ("%s: " f "\n", "cdvd_stream", ## __VA_ARGS__)
#define CDTRACE(f, ...)   printf("%s: " f "\n", "cdvd_stream", ## __VA_ARGS__)

#ifdef FLUSHABLE_STREAMING
bool flushStream[MAX_CDCHANNELS];
#endif

#ifdef USE_UNNAMED_SEM

#define RE3_SEM_OPEN(name, ...) re3_sem_open()
sem_t*
re3_sem_open(void)
{
	sem_t* sem = (sem_t*)malloc(sizeof(sem_t));
	if (sem_init(sem, 0, 1) == -1) {
		sem = SEM_FAILED;
	}

	return sem;
}

#define RE3_SEM_CLOSE(sem, format, ...) re3_sem_close(sem)
void
re3_sem_close(sem_t* sem)
{
	sem_destroy(sem);
	free(sem);
}

#else

#define RE3_SEM_OPEN re3_sem_open
sem_t*
re3_sem_open(const char* format, ...)
{
	char semName[21];
	va_list va;
	va_start(va, format);
	vsprintf(semName, format, va);

	return sem_open(semName, O_CREAT, 0644, 1);
}

#define RE3_SEM_CLOSE re3_sem_close
void
re3_sem_close(sem_t* sem, const char* format, ...)
{
	sem_close(sem);

	char semName[21];
	va_list va;
	va_start(va, format);
	vsprintf(semName, format, va);

	sem_unlink(semName);
}

#endif

struct CdReadInfo
{
	uint32 nSectorOffset;
	uint32 nSectorsToRead;
	void *pBuffer;
	bool bLocked;
	bool bReading;
	int32 nStatus;
#ifdef ONE_THREAD_PER_CHANNEL
	int8 nThreadStatus; // 0: created 1:priority set up 2:abort now
	pthread_t pChannelThread;
	sem_t *pStartSemaphore;
#endif
	sem_t *pDoneSemaphore; // used for CdStreamSync
	int32 hFile;
};

char gCdImageNames[MAX_CDIMAGES+1][64];
int32 gNumImages;
int32 gNumChannels;

int32 gImgFiles[MAX_CDIMAGES]; // -1: error 0:unused otherwise: fd
char *gImgNames[MAX_CDIMAGES];

#ifndef ONE_THREAD_PER_CHANNEL
pthread_t _gCdStreamThread;
sem_t *gCdStreamSema; // released when we have new thing to read(so channel is set)
int8 gCdStreamThreadStatus; // 0: created 1:priority set up 2:abort now
Queue gChannelRequestQ;
bool _gbCdStreamOverlapped;
#endif

CdReadInfo *gpReadInfo;

int32 lastPosnRead;

int _gdwCdStreamFlags;

void *CdStreamThread(void* channelId);

void
CdStreamInitThread(void)
{
	int status;
#ifndef ONE_THREAD_PER_CHANNEL
	gChannelRequestQ.items = (int32 *)calloc(gNumChannels + 1, sizeof(int32));
	gChannelRequestQ.head = 0;
	gChannelRequestQ.tail = 0;
	gChannelRequestQ.size = gNumChannels + 1;
	ASSERT(gChannelRequestQ.items != nil );
	gCdStreamSema = RE3_SEM_OPEN("/semaphore_cd_stream");


	if (gCdStreamSema == SEM_FAILED) {
		CDTRACE("failed to create stream semaphore");
		ASSERT(0);
		return;
	}
#endif

	if ( gNumChannels > 0 )
	{
		for ( int32 i = 0; i < gNumChannels; i++ )
		{
			gpReadInfo[i].pDoneSemaphore = RE3_SEM_OPEN("/semaphore_done%d", i);

			if (gpReadInfo[i].pDoneSemaphore == SEM_FAILED)
			{
				CDTRACE("failed to create sync semaphore");
				ASSERT(0);
				return;
			}

#ifdef ONE_THREAD_PER_CHANNEL
			gpReadInfo[i].pStartSemaphore = RE3_SEM_OPEN("/semaphore_start%d", i);

			if (gpReadInfo[i].pStartSemaphore == SEM_FAILED)
			{
				CDTRACE("failed to create start semaphore");
				ASSERT(0);
				return;
			}
			gpReadInfo[i].nThreadStatus = 0;
			int *channelI = (int*)malloc(sizeof(int));
			*channelI = i;
			status = pthread_create(&gpReadInfo[i].pChannelThread, NULL, CdStreamThread, (void*)channelI);

			if (status == -1)
			{
				CDTRACE("failed to create sync thread");
				ASSERT(0);
				return;
			}
#endif
		}
	}

#ifndef ONE_THREAD_PER_CHANNEL
	debug("Using one streaming thread for all channels\n");
	gCdStreamThreadStatus = 0;
	status = pthread_create(&_gCdStreamThread, NULL, CdStreamThread, nil);

	if (status == -1)
	{
		CDTRACE("failed to create sync thread");
		ASSERT(0);
		return;
	}
#else
	debug("Using separate streaming threads for each channel\n");
#endif
}

void
CdStreamInit(int32 numChannels)
{
	struct statvfs fsInfo;

	if((statvfs("models/gta3.img", &fsInfo)) < 0)
	{
		CDTRACE("can't get filesystem info");
		ASSERT(0);
		return;
	}
#ifdef __linux__
	_gdwCdStreamFlags = O_RDONLY | O_NOATIME;
#else
	_gdwCdStreamFlags = O_RDONLY;
#endif
	// People say it's slower
/*
	if ( fsInfo.f_bsize <= CDSTREAM_SECTOR_SIZE )
	{
		_gdwCdStreamFlags |= O_DIRECT;
		debug("Using no buffered loading for streaming\n");
	}
*/
	void *pBuffer = (void *)RwMallocAlign(CDSTREAM_SECTOR_SIZE, (RwUInt32)fsInfo.f_bsize);
	ASSERT( pBuffer != nil );

	gNumImages = 0;

	gNumChannels = numChannels;
	ASSERT( gNumChannels != 0 );

	gpReadInfo = (CdReadInfo *)calloc(numChannels, sizeof(CdReadInfo));
	ASSERT( gpReadInfo != nil );

	CDDEBUG("read info %p", gpReadInfo);

	CdStreamInitThread();

	ASSERT( pBuffer != nil );
	RwFreeAlign(pBuffer);
}

uint32
GetGTA3ImgSize(void)
{
	ASSERT( gImgFiles[0] > 0 );
	struct stat statbuf;

	char path[PATH_MAX];
	realpath(gImgNames[0], path);
	if (stat(path, &statbuf) == -1) {
		// Try case-insensitivity
		char* real = casepath(gImgNames[0], false);
		if (real)
		{
			realpath(real, path);
			free(real);
			if (stat(path, &statbuf) != -1)
				goto ok;
		}

		CDTRACE("can't get size of gta3.img");
		ASSERT(0);
		return 0;
	}
	ok:
	return (uint32)statbuf.st_size;
}

void
CdStreamShutdown(void)
{
    // Destroying semaphores and free(gpReadInfo) will be done at threads
#ifndef ONE_THREAD_PER_CHANNEL
	gCdStreamThreadStatus = 2;
	sem_post(gCdStreamSema);
	pthread_join(_gCdStreamThread, nil);
#else
	for ( int32 i = 0; i < gNumChannels; i++ ) {
		gpReadInfo[i].nThreadStatus = 2;
		sem_post(gpReadInfo[i].pStartSemaphore);
		pthread_join(gpReadInfo[i].pChannelThread, nil);
	}
#endif
}


int32
CdStreamRead(int32 channel, void *buffer, uint32 offset, uint32 size)
{
	ASSERT( channel < gNumChannels );
	ASSERT( buffer != nil );

	lastPosnRead = size + offset;

	ASSERT( _GET_INDEX(offset) < MAX_CDIMAGES );
	int32 hImage = gImgFiles[_GET_INDEX(offset)];
	ASSERT( hImage > 0 );

	CdReadInfo *pChannel = &gpReadInfo[channel];
	ASSERT( pChannel != nil );

	if ( pChannel->nSectorsToRead != 0 || pChannel->bReading ) {
		if (pChannel->hFile == hImage - 1 && pChannel->nSectorOffset == _GET_OFFSET(offset) && pChannel->nSectorsToRead >= size)
			return STREAM_SUCCESS;
#ifdef FLUSHABLE_STREAMING
		flushStream[channel] = 1;
		CdStreamSync(channel);
#else
		return STREAM_NONE;
#endif
	}

	pChannel->hFile = hImage - 1;
	pChannel->nStatus = STREAM_NONE;
	pChannel->nSectorOffset = _GET_OFFSET(offset);
	pChannel->nSectorsToRead = size;
	pChannel->pBuffer = buffer;
	pChannel->bLocked = 0;

#ifndef ONE_THREAD_PER_CHANNEL
	AddToQueue(&gChannelRequestQ, channel);
	if ( sem_post(gCdStreamSema) != 0 )
		printf("Signal Sema Error\n");
#else
	if ( sem_post(pChannel->pStartSemaphore) != 0 )
		printf("Signal Sema Error\n");
#endif

	return STREAM_SUCCESS;
}

int32
CdStreamGetStatus(int32 channel)
{
	ASSERT( channel < gNumChannels );
	CdReadInfo *pChannel = &gpReadInfo[channel];
	ASSERT( pChannel != nil );

#ifdef ONE_THREAD_PER_CHANNEL
	if (pChannel->nThreadStatus == 2)
		return STREAM_NONE;
#else
	if (gCdStreamThreadStatus == 2)
		return STREAM_NONE;
#endif

	if ( pChannel->bReading )
		return STREAM_READING;

	if ( pChannel->nSectorsToRead != 0 )
		return STREAM_WAITING;

	if ( pChannel->nStatus != STREAM_NONE )
	{
		int32 status = pChannel->nStatus;
		pChannel->nStatus = STREAM_NONE;

		return status;
	}

	return STREAM_NONE;
}

int32
CdStreamGetLastPosn(void)
{
	return lastPosnRead;
}

// wait for channel to finish reading
int32
CdStreamSync(int32 channel)
{
	ASSERT( channel < gNumChannels );
	CdReadInfo *pChannel = &gpReadInfo[channel];
	ASSERT( pChannel != nil );

#ifdef FLUSHABLE_STREAMING
	if (flushStream[channel]) {
		pChannel->nSectorsToRead = 0;
#ifdef ONE_THREAD_PER_CHANNEL
		pthread_kill(pChannel->pChannelThread, SIGUSR1);
		if (pChannel->bReading) {
			pChannel->bLocked = true;
#else
		if (pChannel->bReading) {
			pChannel->bLocked = true;
			pthread_kill(_gCdStreamThread, SIGUSR1);
#endif
			while (pChannel->bLocked)
				sem_wait(pChannel->pDoneSemaphore);
		}
		pChannel->bReading = false;
		flushStream[channel] = false;
		return STREAM_NONE;
	}
#endif

	if ( pChannel->nSectorsToRead != 0 )
	{
		pChannel->bLocked = true;
		while (pChannel->bLocked && pChannel->nSectorsToRead != 0){
			sem_wait(pChannel->pDoneSemaphore);
		}
		pChannel->bLocked = false;
	}

	pChannel->bReading = false;

	return pChannel->nStatus;
}

void
AddToQueue(Queue *queue, int32 item)
{
	ASSERT( queue != nil );
	ASSERT( queue->items != nil );
	queue->items[queue->tail] = item;

	queue->tail = (queue->tail + 1) % queue->size;

	if ( queue->head == queue->tail )
		debug("Queue is full\n");
}

int32
GetFirstInQueue(Queue *queue)
{
	ASSERT( queue != nil );
	if ( queue->head == queue->tail )
		return -1;

	ASSERT( queue->items != nil );
	return queue->items[queue->head];
}

void
RemoveFirstInQueue(Queue *queue)
{
	ASSERT( queue != nil );
	if ( queue->head == queue->tail )
	{
		debug("Queue is empty\n");
		return;
	}

	queue->head = (queue->head + 1) % queue->size;
}

void *CdStreamThread(void *param)
{
	debug("Created cdstream thread\n");

#ifndef ONE_THREAD_PER_CHANNEL
	while (gCdStreamThreadStatus != 2) {
		sem_wait(gCdStreamSema);

		int32 channel = GetFirstInQueue(&gChannelRequestQ);
		
		// spurious wakeup
		if (channel == -1)
			continue;
#else
	int channel = *((int*)param);
	while (gpReadInfo[channel].nThreadStatus != 2){
		sem_wait(gpReadInfo[channel].pStartSemaphore);
#endif

		CdReadInfo *pChannel = &gpReadInfo[channel];
		ASSERT( pChannel != nil );

		// spurious wakeup or we sent interrupt signal for flushing
		if(pChannel->nSectorsToRead == 0)
			continue;

		pChannel->bReading = true;

		// Not standard POSIX :shrug:
#ifdef __linux__
#ifdef ONE_THREAD_PER_CHANNEL
		if (gpReadInfo[channel].nThreadStatus == 0){
			gpReadInfo[channel].nThreadStatus = 1;
#else
		if (gCdStreamThreadStatus == 0){
			gCdStreamThreadStatus = 1;
#endif
			pid_t tid = syscall(SYS_gettid);
			int ret = setpriority(PRIO_PROCESS, tid, getpriority(PRIO_PROCESS, getpid()) + 1);
		}
#endif
		if ( pChannel->nStatus == STREAM_NONE )
		{
			ASSERT(pChannel->hFile >= 0);
			ASSERT(pChannel->pBuffer != nil );

			lseek(pChannel->hFile, (size_t)pChannel->nSectorOffset * (size_t)CDSTREAM_SECTOR_SIZE, SEEK_SET);
			if (read(pChannel->hFile, pChannel->pBuffer, pChannel->nSectorsToRead * CDSTREAM_SECTOR_SIZE) == -1) {
				// pChannel->nSectorsToRead == 0 at this point means we wanted to flush channel
				// STREAM_WAITING is a little hack to make CStreaming not process this data
				pChannel->nStatus = pChannel->nSectorsToRead == 0 ? STREAM_WAITING : STREAM_ERROR;
			} else {
				pChannel->nStatus = STREAM_NONE;
			}
		}

#ifndef ONE_THREAD_PER_CHANNEL
		RemoveFirstInQueue(&gChannelRequestQ);
#endif

		pChannel->nSectorsToRead = 0;
		if ( pChannel->bLocked )
		{
			pChannel->bLocked = 0;
			sem_post(pChannel->pDoneSemaphore);
		}
		pChannel->bReading = false;
	}
	char semName[20];
#ifndef ONE_THREAD_PER_CHANNEL
	for ( int32 i = 0; i < gNumChannels; i++ )
	{
		RE3_SEM_CLOSE(gpReadInfo[i].pDoneSemaphore, "/semaphore_done%d", i);
	}
	RE3_SEM_CLOSE(gCdStreamSema, "/semaphore_cd_stream");
	free(gChannelRequestQ.items);
#else
	RE3_SEM_CLOSE(gpReadInfo[channel].pStartSemaphore, "/semaphore_start%d", channel);

	RE3_SEM_CLOSE(gpReadInfo[channel].pDoneSemaphore, "/semaphore_done%d", channel);
#endif
	if (gpReadInfo)
		free(gpReadInfo);
	gpReadInfo = nil;
	pthread_exit(nil);
}

bool
CdStreamAddImage(char const *path)
{
	ASSERT(path != nil);
	ASSERT(gNumImages < MAX_CDIMAGES);

	gImgFiles[gNumImages] = open(path, _gdwCdStreamFlags);

	// Fix case sensitivity and backslashes.
	if (gImgFiles[gNumImages] == -1) {
		char* real = casepath(path, false);
		if (real)
		{
			gImgFiles[gNumImages] = open(real, _gdwCdStreamFlags);
			free(real);
		}
	}

	if ( gImgFiles[gNumImages] == -1 ) {
		assert(false);
		return false;
	}

	gImgNames[gNumImages] = strdup(path);
	gImgFiles[gNumImages]++; // because -1: error 0: not used

	strcpy(gCdImageNames[gNumImages], path);

	gNumImages++;

	return true;
}

char *
CdStreamGetImageName(int32 cd)
{
	ASSERT(cd < MAX_CDIMAGES);
	if ( gImgFiles[cd] > 0)
		return gCdImageNames[cd];

	return nil;
}

void
CdStreamRemoveImages(void)
{
	for ( int32 i = 0; i < gNumChannels; i++ ) {
#ifdef FLUSHABLE_STREAMING
		flushStream[i] = 1;
#endif
		CdStreamSync(i);
	}

	for ( int32 i = 0; i < gNumImages; i++ )
	{
		close(gImgFiles[i] - 1);
		free(gImgNames[i]);
		gImgFiles[i] = 0;
	}

	gNumImages = 0;
}

int32
CdStreamGetNumImages(void)
{
	return gNumImages;
}
#endif
