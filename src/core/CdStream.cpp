#ifdef _WIN32
#define WITHWINDOWS
#include "common.h"

#include "CdStream.h"
#include "rwcore.h"
#include "RwHelper.h"
#include "MemoryMgr.h"

struct CdReadInfo
{
	uint32 nSectorOffset;
	uint32 nSectorsToRead;
	void *pBuffer;
	char field_C;
	bool bLocked;
	bool bReading;
	int32 nStatus;
	HANDLE pDoneSemaphore; // used for CdStreamSync
	HANDLE hFile;
	OVERLAPPED Overlapped;
};

VALIDATE_SIZE(CdReadInfo, 0x30);

char gCdImageNames[MAX_CDIMAGES+1][64];
int32 gNumImages;
int32 gNumChannels;

HANDLE gImgFiles[MAX_CDIMAGES];

HANDLE _gCdStreamThread;
HANDLE gCdStreamSema; // released when we have new thing to read(so channel is set)
DWORD _gCdStreamThreadId;

CdReadInfo *gpReadInfo;
Queue gChannelRequestQ;

int32 lastPosnRead;

BOOL _gbCdStreamOverlapped;
BOOL _gbCdStreamAsync;
DWORD _gdwCdStreamFlags;

DWORD WINAPI CdStreamThread(LPVOID lpThreadParameter);

void
CdStreamInitThread(void)
{
	SetLastError(0);
	
	if ( gNumChannels > 0 )
	{
		for ( int32 i = 0; i < gNumChannels; i++ )
		{
			gpReadInfo[i].pDoneSemaphore = CreateSemaphore(nil, 0, 2, nil);
			
			if ( gpReadInfo[i].pDoneSemaphore == nil )
			{
				printf("%s: failed to create sync semaphore\n", "cdvd_stream");
				ASSERT(0);
				return;
			}
		}
	}
	
	gChannelRequestQ.items = (int32 *)LocalAlloc(LMEM_ZEROINIT, sizeof(int32) * (gNumChannels + 1));
	gChannelRequestQ.head = 0;
	gChannelRequestQ.tail = 0;
	gChannelRequestQ.size = gNumChannels + 1;
	ASSERT(gChannelRequestQ.items != nil );
	
#ifdef FIX_BUGS
	gCdStreamSema = CreateSemaphore(nil, 0, 5, nil);
#else
	gCdStreamSema = CreateSemaphore(nil, 0, 5, "CdStream");
#endif
	
	if ( gCdStreamSema == nil )
	{
		printf("%s: failed to create stream semaphore\n", "cdvd_stream");
		ASSERT(0);
		return;
	}
	
	_gCdStreamThread = CreateThread(nil, 64*1024/*64KB*/, CdStreamThread, nil, CREATE_SUSPENDED, &_gCdStreamThreadId);
	
	if ( _gCdStreamThread == nil )
	{
		printf("%s: failed to create streaming thread\n", "cdvd_stream");
		ASSERT(0);
		return;
	}

	SetThreadPriority(_gCdStreamThread, GetThreadPriority(GetCurrentThread()) - 1);
		
	ResumeThread(_gCdStreamThread);	
}

void
CdStreamInit(int32 numChannels)
{
	DWORD SectorsPerCluster;
	DWORD BytesPerSector;
	DWORD NumberOfFreeClusters;
	DWORD TotalNumberOfClusters;

	GetDiskFreeSpace(nil, &SectorsPerCluster, &BytesPerSector, &NumberOfFreeClusters, &TotalNumberOfClusters);
	
	_gdwCdStreamFlags = 0;

#ifndef FIX_BUGS // this just slows down streaming
	if ( BytesPerSector <= CDSTREAM_SECTOR_SIZE )
	{
		_gdwCdStreamFlags |= FILE_FLAG_NO_BUFFERING;
		debug("Using no buffered loading for streaming\n");
	}
#endif
	
	_gbCdStreamOverlapped = TRUE;

	_gdwCdStreamFlags |= FILE_FLAG_OVERLAPPED;
	
	_gbCdStreamAsync = FALSE;
	
	void *pBuffer = (void *)RwMallocAlign(CDSTREAM_SECTOR_SIZE, BytesPerSector);
	ASSERT( pBuffer != nil );
	
	SetLastError(0);
	
	gNumImages = 0;
	
	gNumChannels = numChannels;
	
	gpReadInfo = (CdReadInfo *)LocalAlloc(LMEM_ZEROINIT, sizeof(CdReadInfo) * numChannels);
	ASSERT( gpReadInfo != nil );
	
	debug("%s: read info %p\n", "cdvd_stream", gpReadInfo);
	
	CdStreamAddImage("MODELS\\GTA3.IMG");
	
	int32 nStatus = CdStreamRead(0, pBuffer, 0, 1);
	
	CdStreamRemoveImages();
	
	if ( nStatus == STREAM_SUCCESS )
	{
		_gbCdStreamAsync = TRUE;
		
		debug("Using async loading for streaming\n");
	}
	else
	{
		_gdwCdStreamFlags &= ~FILE_FLAG_OVERLAPPED;
		
		_gbCdStreamOverlapped = FALSE;
		
		_gbCdStreamAsync = TRUE;
		
		debug("Using sync loading for streaming\n");
	}
	
	CdStreamInitThread();
	
	ASSERT( pBuffer != nil );
	RwFreeAlign(pBuffer);
}

uint32
GetGTA3ImgSize(void)
{
	ASSERT( gImgFiles[0] != nil );
	return (uint32)GetFileSize(gImgFiles[0], nil);
}

void
CdStreamShutdown(void)
{
	if ( _gbCdStreamAsync )
	{
		LocalFree(gChannelRequestQ.items);
		CloseHandle(gCdStreamSema);
		CloseHandle(_gCdStreamThread);
		
		for ( int32 i = 0; i < gNumChannels; i++ )
			CloseHandle(gpReadInfo[i].pDoneSemaphore);
	}
	
	LocalFree(gpReadInfo);
}



int32
CdStreamRead(int32 channel, void *buffer, uint32 offset, uint32 size)
{
	ASSERT( channel < gNumChannels );
	ASSERT( buffer != nil );
	
	lastPosnRead = size + offset;
	
	ASSERT( _GET_INDEX(offset) < MAX_CDIMAGES );
	HANDLE hImage = gImgFiles[_GET_INDEX(offset)];
	ASSERT( hImage != nil );
	
	
	CdReadInfo *pChannel = &gpReadInfo[channel];
	ASSERT( pChannel != nil );
	
	pChannel->hFile = hImage;
	
	SetLastError(0);
	
	if ( _gbCdStreamAsync )
	{
		if ( pChannel->nSectorsToRead != 0 || pChannel->bReading )
			return STREAM_NONE;
		
		pChannel->nStatus = STREAM_NONE;
		pChannel->nSectorOffset = _GET_OFFSET(offset);
		pChannel->nSectorsToRead = size;
		pChannel->pBuffer = buffer;
		pChannel->bLocked = 0;
		
		AddToQueue(&gChannelRequestQ, channel);
		
		if ( !ReleaseSemaphore(gCdStreamSema, 1, nil) )
			printf("Signal Sema Error\n");
		
		return STREAM_SUCCESS;
	}
	
	if ( _gbCdStreamOverlapped )
	{
		ASSERT( channel < gNumChannels );
		CdReadInfo *pChannel = &gpReadInfo[channel];
		ASSERT( pChannel != nil );
		
		pChannel->Overlapped.Offset = _GET_OFFSET(offset) * CDSTREAM_SECTOR_SIZE;
		
		if ( !ReadFile(hImage, buffer, size * CDSTREAM_SECTOR_SIZE, NULL, &pChannel->Overlapped)
					&& GetLastError() != ERROR_IO_PENDING )
			return STREAM_NONE;
		else
			return STREAM_SUCCESS;
	}

#ifdef BIG_IMG
	LARGE_INTEGER liDistanceToMove;
	liDistanceToMove.QuadPart = _GET_OFFSET(offset);
	liDistanceToMove.QuadPart *= CDSTREAM_SECTOR_SIZE;
	SetFilePointerEx(hImage, liDistanceToMove, nil, FILE_BEGIN);
#else
	SetFilePointer(hImage, _GET_OFFSET(offset) * CDSTREAM_SECTOR_SIZE, nil, FILE_BEGIN);
#endif
	
	DWORD NumberOfBytesRead;
	
	if ( !ReadFile(hImage, buffer, size * CDSTREAM_SECTOR_SIZE, &NumberOfBytesRead, nil) )
		return STREAM_NONE;
	else
		return STREAM_SUCCESS;
}

int32
CdStreamGetStatus(int32 channel)
{
	ASSERT( channel < gNumChannels );
	CdReadInfo *pChannel = &gpReadInfo[channel];
	ASSERT( pChannel != nil );
	
	if ( _gbCdStreamAsync )
	{
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
	
	if ( _gbCdStreamOverlapped )
	{
		ASSERT( pChannel->hFile != nil );
		if ( WaitForSingleObjectEx(pChannel->hFile, 0, TRUE) == WAIT_OBJECT_0 )
			return STREAM_NONE;
		else
			return STREAM_READING;
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
	
	if ( _gbCdStreamAsync )
	{
		if ( pChannel->nSectorsToRead != 0 )
		{
			pChannel->bLocked = true;

			ASSERT( pChannel->pDoneSemaphore != nil );

			// Deadlock fix 1
#ifdef FIX_BUGS
			// This is while loop on Posix streamer, for spurious wakeups
			if (pChannel->bLocked && pChannel->nSectorsToRead != 0){
				WaitForSingleObject(pChannel->pDoneSemaphore, INFINITE);
			}
			pChannel->bLocked = false;
#else
			WaitForSingleObject(pChannel->pDoneSemaphore, INFINITE);
#endif
		}

		pChannel->bReading = false;
		
		return pChannel->nStatus;
	}
	
	DWORD NumberOfBytesTransferred;

	if ( _gbCdStreamOverlapped && pChannel->hFile )
	{
		ASSERT(pChannel->hFile != nil );
		// Beware: This is blocking call (because of last parameter)
		if ( GetOverlappedResult(pChannel->hFile, &pChannel->Overlapped, &NumberOfBytesTransferred, TRUE) )
			return STREAM_NONE;
		else
			return STREAM_ERROR;
	}
	
	return STREAM_NONE;
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

DWORD
WINAPI CdStreamThread(LPVOID lpThreadParameter)
{
	debug("Created cdstream thread\n");
	
	while ( true )
	{
		WaitForSingleObject(gCdStreamSema, INFINITE);
		
		int32 channel = GetFirstInQueue(&gChannelRequestQ);
		ASSERT( channel < gNumChannels );
		
		CdReadInfo *pChannel = &gpReadInfo[channel];
		ASSERT( pChannel != nil );
		
		pChannel->bReading = true;
		
		if ( pChannel->nStatus == STREAM_NONE )
		{
			if ( _gbCdStreamOverlapped )
			{
				pChannel->Overlapped.Offset = pChannel->nSectorOffset * CDSTREAM_SECTOR_SIZE;
				
				ASSERT(pChannel->hFile != nil );
				ASSERT(pChannel->pBuffer != nil );
				
				DWORD NumberOfBytesTransferred;
				
				if ( ReadFile(pChannel->hFile,
						pChannel->pBuffer,
						pChannel->nSectorsToRead * CDSTREAM_SECTOR_SIZE,
						NULL,
						&pChannel->Overlapped) )
				{
					pChannel->nStatus = STREAM_NONE;
				}
				// Beware: This is blocking call (because of last parameter)
				else if ( GetLastError() == ERROR_IO_PENDING
						&& GetOverlappedResult(pChannel->hFile, &pChannel->Overlapped, &NumberOfBytesTransferred, TRUE) )
				{
					pChannel->nStatus = STREAM_NONE;
				}
				else
				{
					pChannel->nStatus = STREAM_ERROR;
				}
			}
			else
			{
				ASSERT(pChannel->hFile != nil );
				ASSERT(pChannel->pBuffer != nil );
				
				SetFilePointer(pChannel->hFile, pChannel->nSectorOffset * CDSTREAM_SECTOR_SIZE, nil, FILE_BEGIN);

				DWORD NumberOfBytesRead;
				if ( ReadFile(pChannel->hFile,
						pChannel->pBuffer,
						pChannel->nSectorsToRead * CDSTREAM_SECTOR_SIZE,
						&NumberOfBytesRead,
						NULL) )
				{
					pChannel->nStatus = STREAM_NONE;
				}
			}
		}
		
		RemoveFirstInQueue(&gChannelRequestQ);
		
		pChannel->nSectorsToRead = 0;
		
		if ( pChannel->bLocked )
		{
			ASSERT( pChannel->pDoneSemaphore != nil );
			// Deadlock fix 2
#ifdef FIX_BUGS
			pChannel->bLocked = 0;
#endif
			ReleaseSemaphore(pChannel->pDoneSemaphore, 1, NULL);
		}
		
		pChannel->bReading = false;
	}
}

bool
CdStreamAddImage(char const *path)
{
	ASSERT(path != nil);
	ASSERT(gNumImages < MAX_CDIMAGES);
	
	SetLastError(0);
	
	gImgFiles[gNumImages] = CreateFile(path,
	                                   GENERIC_READ,
	                                   FILE_SHARE_READ,
	                                   nil,
	                                   OPEN_EXISTING,
	                                   _gdwCdStreamFlags | FILE_FLAG_RANDOM_ACCESS | FILE_ATTRIBUTE_READONLY,
	                                   nil);
	
	ASSERT( gImgFiles[gNumImages] != nil );
	if ( gImgFiles[gNumImages] == NULL )
		return false;
	
	strcpy(gCdImageNames[gNumImages], path);
	
	gNumImages++;
	
	return true;
}

char *
CdStreamGetImageName(int32 cd)
{
	ASSERT(cd < MAX_CDIMAGES);
	if ( gImgFiles[cd] != nil )
		return gCdImageNames[cd];
	
	return nil;
}

void
CdStreamRemoveImages(void)
{	
	for ( int32 i = 0; i < gNumChannels; i++ )
		CdStreamSync(i);
	
	for ( int32 i = 0; i < gNumImages; i++ )
	{
		SetLastError(0);
		
		CloseHandle(gImgFiles[i]);
		gImgFiles[i] = nil;
	}
	
	gNumImages = 0;
}

int32
CdStreamGetNumImages(void)
{	
	return gNumImages;
}
#endif
