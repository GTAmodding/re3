#pragma once

// fake mss.h header for use with re3, to make using mss32.dll possible
// gta3 uses miles 6.1a
// check https://github.com/withmorten/re3mss for more info

#include <stdint.h>

typedef char C8;
typedef uint8_t U8;
typedef int8_t S8;
typedef int16_t S16;
typedef uint16_t U16;
typedef int32_t S32;
typedef uint32_t U32;
typedef float F32;
typedef double F64;

typedef void *HSTREAM;
typedef U32 HPROVIDER;
typedef void *H3DPOBJECT;
typedef H3DPOBJECT H3DSAMPLE;
typedef void *HSAMPLE;
typedef void *HDIGDRIVER;

typedef U32 HPROENUM;
#define HPROENUM_FIRST 0

typedef S32 M3DRESULT;

#define M3D_NOERR 0

enum { ENVIRONMENT_CAVE = 8 };

#define AIL_3D_2_SPEAKER 0
#define AIL_3D_HEADPHONE 1
#define AIL_3D_4_SPEAKER 3

#define DIG_MIXER_CHANNELS 1

#define DIG_F_MONO_16 1
#define DIG_PCM_SIGN 1

#define SMP_PLAYING 4

typedef struct _AILSOUNDINFO
{
	S32 format;
	void const *data_ptr;
	U32 data_len;
	U32 rate;
	S32 bits;
	S32 channels;
	U32 samples;
	U32 block_size;
	void const *initial_ptr;
} AILSOUNDINFO;

typedef U32 (WINAPI *AIL_file_open_callback)(char const * Filename, U32 * FileHandle);

typedef void (WINAPI *AIL_file_close_callback)(U32 FileHandle);

#define AIL_FILE_SEEK_BEGIN 0
#define AIL_FILE_SEEK_CURRENT 1
#define AIL_FILE_SEEK_END 2

typedef S32(WINAPI *AIL_file_seek_callback)(U32 FileHandle, S32 Offset, U32 Type);

typedef U32(WINAPI *AIL_file_read_callback)(U32 FileHandle, void* Buffer, U32 Bytes);

#ifdef RE3MSS_EXPORTS
#define RE3MSS_EXPORT __declspec(dllexport)
#else
#define RE3MSS_EXPORT __declspec(dllimport)
#endif

#ifdef __cplusplus
extern "C" {
#endif

RE3MSS_EXPORT S32 WINAPI AIL_enumerate_3D_providers(HPROENUM *next, HPROVIDER *dest, C8 **name);
RE3MSS_EXPORT void WINAPI AIL_release_3D_sample_handle(H3DSAMPLE S);
RE3MSS_EXPORT void WINAPI AIL_close_3D_provider(HPROVIDER lib);
RE3MSS_EXPORT void WINAPI AIL_set_3D_provider_preference(HPROVIDER lib, C8 const *name, void const *val);
RE3MSS_EXPORT M3DRESULT WINAPI AIL_open_3D_provider(HPROVIDER lib);
RE3MSS_EXPORT C8 *WINAPI AIL_last_error(void);
RE3MSS_EXPORT S32 WINAPI AIL_3D_room_type(HPROVIDER lib);
RE3MSS_EXPORT void WINAPI AIL_set_3D_room_type(HPROVIDER lib, S32 room_type);
RE3MSS_EXPORT void WINAPI AIL_3D_provider_attribute(HPROVIDER lib, C8 const *name, void *val);
RE3MSS_EXPORT H3DSAMPLE WINAPI AIL_allocate_3D_sample_handle(HPROVIDER lib);
RE3MSS_EXPORT void WINAPI AIL_set_3D_sample_effects_level(H3DSAMPLE S, F32 effects_level);
RE3MSS_EXPORT void WINAPI AIL_set_3D_speaker_type(HPROVIDER lib, S32 speaker_type);
RE3MSS_EXPORT HSTREAM WINAPI AIL_open_stream(HDIGDRIVER dig, C8 const *filename, S32 stream_mem);
RE3MSS_EXPORT void WINAPI AIL_stream_ms_position(HSTREAM S, S32 *total_milliseconds, S32 *current_milliseconds);
RE3MSS_EXPORT void WINAPI AIL_close_stream(HSTREAM stream);
RE3MSS_EXPORT S32 WINAPI AIL_digital_handle_release(HDIGDRIVER drvr);
RE3MSS_EXPORT S32 WINAPI AIL_digital_handle_reacquire(HDIGDRIVER drvr);
RE3MSS_EXPORT C8 *WINAPI AIL_set_redist_directory(C8 const *dir);
RE3MSS_EXPORT S32 WINAPI AIL_startup(void);
RE3MSS_EXPORT S32 WINAPI AIL_set_preference(U32 number, S32 value);
RE3MSS_EXPORT HDIGDRIVER WINAPI AIL_open_digital_driver(U32 frequency, S32 bits, S32 channel, U32 flags);
RE3MSS_EXPORT void *WINAPI AIL_mem_alloc_lock(U32 size);
RE3MSS_EXPORT HSAMPLE WINAPI AIL_allocate_sample_handle(HDIGDRIVER dig);
RE3MSS_EXPORT void WINAPI AIL_init_sample(HSAMPLE S);
RE3MSS_EXPORT void WINAPI AIL_set_sample_type(HSAMPLE S, S32 format, U32 flags);
RE3MSS_EXPORT void WINAPI AIL_pause_stream(HSTREAM stream, S32 onoff);
RE3MSS_EXPORT void WINAPI AIL_release_sample_handle(HSAMPLE S);
RE3MSS_EXPORT void WINAPI AIL_mem_free_lock(void *ptr);
RE3MSS_EXPORT void WINAPI AIL_close_digital_driver(HDIGDRIVER dig);
RE3MSS_EXPORT void WINAPI AIL_shutdown(void);
RE3MSS_EXPORT void WINAPI AIL_set_3D_sample_volume(H3DSAMPLE S, S32 volume);
RE3MSS_EXPORT void WINAPI AIL_set_sample_volume(HSAMPLE S, S32 volume);
RE3MSS_EXPORT void WINAPI AIL_set_sample_address(HSAMPLE S, void const *start, U32 len);
RE3MSS_EXPORT S32 WINAPI AIL_set_3D_sample_info(H3DSAMPLE S, AILSOUNDINFO const *info);
RE3MSS_EXPORT void WINAPI AIL_set_3D_position(H3DPOBJECT obj, F32 X, F32 Y, F32 Z);
RE3MSS_EXPORT void WINAPI AIL_set_3D_sample_distances(H3DSAMPLE S, F32 max_dist, F32 min_dist);
RE3MSS_EXPORT void WINAPI AIL_set_sample_pan(HSAMPLE S, S32 pan);
RE3MSS_EXPORT void WINAPI AIL_set_sample_playback_rate(HSAMPLE S, S32 playback_rate);
RE3MSS_EXPORT void WINAPI AIL_set_3D_sample_playback_rate(H3DSAMPLE S, S32 playback_rate);
RE3MSS_EXPORT void WINAPI AIL_set_sample_loop_block(HSAMPLE S, S32 loop_start_offset, S32 loop_end_offset);
RE3MSS_EXPORT void WINAPI AIL_set_3D_sample_loop_block(H3DSAMPLE S, S32 loop_start_offset, S32 loop_end_offset);
RE3MSS_EXPORT void WINAPI AIL_set_sample_loop_count(HSAMPLE S, S32 loop_count);
RE3MSS_EXPORT void WINAPI AIL_set_3D_sample_loop_count(H3DSAMPLE S, S32 loops);
RE3MSS_EXPORT U32 WINAPI AIL_sample_status(HSAMPLE S);
RE3MSS_EXPORT U32 WINAPI AIL_3D_sample_status(H3DSAMPLE S);
RE3MSS_EXPORT void WINAPI AIL_start_sample(HSAMPLE S);
RE3MSS_EXPORT void WINAPI AIL_start_3D_sample(H3DSAMPLE S);
RE3MSS_EXPORT void WINAPI AIL_end_sample(HSAMPLE S);
RE3MSS_EXPORT void WINAPI AIL_end_3D_sample(H3DSAMPLE S);
RE3MSS_EXPORT void WINAPI AIL_set_stream_loop_count(HSTREAM stream, S32 count);
RE3MSS_EXPORT S32 WINAPI AIL_service_stream(HSTREAM stream, S32 fillup);
RE3MSS_EXPORT void WINAPI AIL_start_stream(HSTREAM stream);
RE3MSS_EXPORT void WINAPI AIL_set_stream_ms_position(HSTREAM S, S32 milliseconds);
RE3MSS_EXPORT void WINAPI AIL_set_stream_volume(HSTREAM stream, S32 volume);
RE3MSS_EXPORT void WINAPI AIL_set_stream_pan(HSTREAM stream, S32 pan);
RE3MSS_EXPORT S32 WINAPI AIL_stream_status(HSTREAM stream);
RE3MSS_EXPORT void WINAPI AIL_set_file_callbacks(AIL_file_open_callback opencb, AIL_file_close_callback closecb, AIL_file_seek_callback seekcb, AIL_file_read_callback readcb);

#ifdef __cplusplus
}
#endif
