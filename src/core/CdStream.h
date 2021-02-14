#pragma once

#define CDSTREAM_SECTOR_SIZE 2048

#define _GET_INDEX(a) (a >> 24)
#define _GET_OFFSET(a)  (a & 0xFFFFFF)

enum
{
	STREAM_NONE            = uint8( 0),
	STREAM_SUCCESS         = uint8( 1),
	STREAM_READING         = uint8(-1), // 0xFF,
	STREAM_ERROR           = uint8(-2), // 0xFE,
	STREAM_ERROR_NOCD      = uint8(-3), // 0xFD,
	STREAM_ERROR_WRONGCD   = uint8(-4), // 0xFC,
	STREAM_ERROR_OPENCD    = uint8(-5), // 0xFB,
	STREAM_WAITING         = uint8(-6)  // 0xFA,
};

struct Queue
{
	int32 *items;
	int32 head;
	int32 tail;
	int32 size;
};

VALIDATE_SIZE(Queue, 0x10);

void CdStreamInitThread(void);
void CdStreamInit(int32 numChannels);
uint32 GetGTA3ImgSize(void);
void CdStreamShutdown(void);
int32 CdStreamRead(int32 channel, void *buffer, uint32 offset, uint32 size);
int32 CdStreamGetStatus(int32 channel);
int32 CdStreamGetLastPosn(void);
int32 CdStreamSync(int32 channel);
void AddToQueue(Queue *queue, int32 item);
int32 GetFirstInQueue(Queue *queue);
void RemoveFirstInQueue(Queue *queue);
bool CdStreamAddImage(char const *path);
char *CdStreamGetImageName(int32 cd);
void CdStreamRemoveImages(void);
int32 CdStreamGetNumImages(void);

#ifdef FLUSHABLE_STREAMING
extern bool flushStream[MAX_CDCHANNELS];
#endif
