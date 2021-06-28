#pragma once

#ifdef VALIDATE_SAVE_SIZE
extern int32 _saveBufCount;
#define INITSAVEBUF _saveBufCount = 0;
#define VALIDATESAVEBUF(b) assert(_saveBufCount == b);
#else
#define INITSAVEBUF
#define VALIDATESAVEBUF(b)
#endif

inline void
SkipSaveBuf(uint8 *&buf, int32 skip)
{
	buf += skip;
#ifdef VALIDATE_SAVE_SIZE
	_saveBufCount += skip;
#endif
}

inline void
SkipSaveBuf(uint8*& buf, uint32 &length, int32 skip)
{
	buf += skip;
	length += skip;
#ifdef VALIDATE_SAVE_SIZE
	_saveBufCount += skip;
#endif
}

template<typename T>
inline void
ReadSaveBuf(T *out, uint8 *&buf)
{
	*out = *(T *)buf;
	SkipSaveBuf(buf, sizeof(T));
}

template<typename T>
inline void
ReadSaveBuf(T *out, uint8 *&buf, uint32 &length)
{
	*out = *(T *)buf;
	SkipSaveBuf(buf, length, sizeof(T));
}

template<typename T>
inline T *
WriteSaveBuf(uint8 *&buf, const T &value)
{
	T *p = (T*)buf;
	*p = value;
	SkipSaveBuf(buf, sizeof(T));
	return p;
}

template<typename T>
inline T *
WriteSaveBuf(uint8 *&buf, uint32 &length, const T &value)
{
	T *p = (T*)buf;
	*p = value;
	SkipSaveBuf(buf, length, sizeof(T));
	return p;
}


#define SAVE_HEADER_SIZE (4*sizeof(char)+sizeof(uint32))

#define WriteSaveHeader(buf,a,b,c,d,size) \
	WriteSaveBuf(buf, a);\
	WriteSaveBuf(buf, b);\
	WriteSaveBuf(buf, c);\
	WriteSaveBuf(buf, d);\
	WriteSaveBuf<uint32>(buf, size);

#define WriteSaveHeaderWithLength(buf,len,a,b,c,d,size) \
	WriteSaveBuf(buf, len, a);\
	WriteSaveBuf(buf, len, b);\
	WriteSaveBuf(buf, len, c);\
	WriteSaveBuf(buf, len, d);\
	WriteSaveBuf(buf, len, (uint32)(size));

#ifdef VALIDATE_SAVE_SIZE
#define CheckSaveHeader(buf, a, b, c, d, size) do { \
	char _C; uint32 _size;\
	ReadSaveBuf(&_C, buf);\
	assert(_C == a);\
	ReadSaveBuf(&_C, buf);\
	assert(_C == b);\
	ReadSaveBuf(&_C, buf);\
	assert(_C == c);\
	ReadSaveBuf(&_C, buf);\
	assert(_C == d);\
	ReadSaveBuf(&_size, buf);\
	assert(_size == size);\
	} while(0)

#define CheckSaveHeaderWithLength(buf,len,a,b,c,d,size) do { \
	char _C; uint32 _size;\
	ReadSaveBuf(&_C, buf, len);\
	assert(_C == a);\
	ReadSaveBuf(&_C, buf, len);\
	assert(_C == b);\
	ReadSaveBuf(&_C, buf, len);\
	assert(_C == c);\
	ReadSaveBuf(&_C, buf, len);\
	assert(_C == d);\
	ReadSaveBuf(&_size, buf, len);\
	assert(_size == size);\
	} while(0)
#else
#define CheckSaveHeader(buf, a, b, c, d, size) SkipSaveBuf(buf, 8);
#define CheckSaveHeaderWithLength(buf, a, b, c, d, size) SkipSaveBuf(buf, 8);
#endif
