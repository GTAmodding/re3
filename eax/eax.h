/*******************************************************************\
*																	*
*  EAX.H - Environmental Audio Extensions version 3.0				*
*          for OpenAL and DirectSound3D								*
*																	*
********************************************************************/

#ifndef EAX_H_INCLUDED
#define EAX_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#ifndef AUDIO_OAL
	#include <dsound.h>

	/*
	 * EAX Wrapper Interface (using Direct X 7) {4FF53B81-1CE0-11d3-AAB8-00A0C95949D5}
	 */
	DEFINE_GUID(CLSID_EAXDirectSound, 
    	0x4ff53b81, 
    	0x1ce0, 
    	0x11d3,
    	0xaa, 0xb8, 0x0, 0xa0, 0xc9, 0x59, 0x49, 0xd5);
    	
   /*
	* EAX Wrapper Interface (using Direct X 8) {CA503B60-B176-11d4-A094-D0C0BF3A560C}
	*/
	DEFINE_GUID(CLSID_EAXDirectSound8, 
		0xca503b60,
		0xb176,
		0x11d4,
		0xa0, 0x94, 0xd0, 0xc0, 0xbf, 0x3a, 0x56, 0xc);

	

#ifdef DIRECTSOUND_VERSION		
#if DIRECTSOUND_VERSION >= 0x0800
	__declspec(dllimport) HRESULT WINAPI EAXDirectSoundCreate8(GUID*, LPDIRECTSOUND8*, IUnknown FAR *);
	typedef HRESULT (FAR PASCAL *LPEAXDIRECTSOUNDCREATE8)(GUID*, LPDIRECTSOUND8*, IUnknown FAR*);
#endif
#endif
	
	__declspec(dllimport) HRESULT WINAPI EAXDirectSoundCreate(GUID*, LPDIRECTSOUND*, IUnknown FAR *);
	typedef HRESULT (FAR PASCAL *LPEAXDIRECTSOUNDCREATE)(GUID*, LPDIRECTSOUND*, IUnknown FAR*);

	__declspec(dllimport) void CDECL GetCurrentVersion(LPDWORD major, LPDWORD minor);
    typedef void (CDECL *LPGETCURRENTVERSION)(LPDWORD major, LPDWORD minor);


#else // AUDIO_OAL
	#include <AL/al.h>
	#include <string.h>

	#ifndef GUID_DEFINED
		#define GUID_DEFINED
		typedef struct _GUID
		{
			unsigned long Data1;
			unsigned short Data2;
			unsigned short Data3;
			unsigned char Data4[8];
		} GUID;
	#endif // !GUID_DEFINED

	#ifndef DEFINE_GUID
		#ifndef INITGUID
			#define DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
					extern const GUID /*FAR*/ name
		#else
			#define DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
					extern const GUID name = { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }
		#endif // INITGUID
	#endif // DEFINE_GUID
	

	/*
	 * EAX OpenAL Extension
	 */
	typedef ALenum (*EAXSet)(const GUID*, ALuint, ALuint, ALvoid*, ALuint);
	typedef ALenum (*EAXGet)(const GUID*, ALuint, ALuint, ALvoid*, ALuint);
#endif

#pragma pack(push, 4)

/*
 * EAX 3.0 listener property set {A8FA6880-B476-11d3-BDB9-00C0F02DDF87}
 */
DEFINE_GUID(DSPROPSETID_EAX30_ListenerProperties, 
	0xa8fa6882, 
	0xb476, 
	0x11d3, 
	0xbd, 0xb9, 0x00, 0xc0, 0xf0, 0x2d, 0xdf, 0x87);

// For compatibility with future EAX versions:
#define DSPROPSETID_EAX_ListenerProperties DSPROPSETID_EAX30_ListenerProperties

typedef enum
{
    DSPROPERTY_EAXLISTENER_NONE,
    DSPROPERTY_EAXLISTENER_ALLPARAMETERS,
    DSPROPERTY_EAXLISTENER_ENVIRONMENT,
    DSPROPERTY_EAXLISTENER_ENVIRONMENTSIZE,
    DSPROPERTY_EAXLISTENER_ENVIRONMENTDIFFUSION,
    DSPROPERTY_EAXLISTENER_ROOM,
    DSPROPERTY_EAXLISTENER_ROOMHF,
    DSPROPERTY_EAXLISTENER_ROOMLF,
    DSPROPERTY_EAXLISTENER_DECAYTIME,
    DSPROPERTY_EAXLISTENER_DECAYHFRATIO,
    DSPROPERTY_EAXLISTENER_DECAYLFRATIO,
    DSPROPERTY_EAXLISTENER_REFLECTIONS,
    DSPROPERTY_EAXLISTENER_REFLECTIONSDELAY,
    DSPROPERTY_EAXLISTENER_REFLECTIONSPAN,
    DSPROPERTY_EAXLISTENER_REVERB,
    DSPROPERTY_EAXLISTENER_REVERBDELAY,
    DSPROPERTY_EAXLISTENER_REVERBPAN,
    DSPROPERTY_EAXLISTENER_ECHOTIME,
    DSPROPERTY_EAXLISTENER_ECHODEPTH,
    DSPROPERTY_EAXLISTENER_MODULATIONTIME,
    DSPROPERTY_EAXLISTENER_MODULATIONDEPTH,
    DSPROPERTY_EAXLISTENER_AIRABSORPTIONHF,
    DSPROPERTY_EAXLISTENER_HFREFERENCE,
    DSPROPERTY_EAXLISTENER_LFREFERENCE,
    DSPROPERTY_EAXLISTENER_ROOMROLLOFFFACTOR,
    DSPROPERTY_EAXLISTENER_FLAGS
} DSPROPERTY_EAX_LISTENERPROPERTY;

// OR these flags with property id
#define DSPROPERTY_EAXLISTENER_IMMEDIATE 0x00000000 // changes take effect immediately
#define DSPROPERTY_EAXLISTENER_DEFERRED  0x80000000 // changes take effect later
#define DSPROPERTY_EAXLISTENER_COMMITDEFERREDSETTINGS (DSPROPERTY_EAXLISTENER_NONE | \
                                                       DSPROPERTY_EAXLISTENER_IMMEDIATE)

typedef struct _EAXVECTOR {
	float x;
	float y;
	float z;
} EAXVECTOR;

// Use this structure for DSPROPERTY_EAXLISTENER_ALLPARAMETERS
// - all levels are hundredths of decibels
// - all times and delays are in seconds
//
// NOTE: This structure may change in future EAX versions.
//       It is recommended to initialize fields by name:
//              myListener.lRoom = -1000;
//              myListener.lRoomHF = -100;
//              ...
//              myListener.dwFlags = myFlags /* see EAXLISTENERFLAGS below */ ;
//       instead of:
//              myListener = { -1000, -100, ... , 0x00000009 };
//       If you want to save and load presets in binary form, you 
//       should define your own structure to insure future compatibility.
//
typedef struct _EAXLISTENERPROPERTIES
{
    unsigned long ulEnvironment;   // sets all listener properties
    float flEnvironmentSize;       // environment size in meters
    float flEnvironmentDiffusion;  // environment diffusion
    long lRoom;                    // room effect level (at mid frequencies)
    long lRoomHF;                  // relative room effect level at high frequencies
    long lRoomLF;                  // relative room effect level at low frequencies  
    float flDecayTime;             // reverberation decay time at mid frequencies
    float flDecayHFRatio;          // high-frequency to mid-frequency decay time ratio
    float flDecayLFRatio;          // low-frequency to mid-frequency decay time ratio   
    long lReflections;             // early reflections level relative to room effect
    float flReflectionsDelay;      // initial reflection delay time
    EAXVECTOR vReflectionsPan;     // early reflections panning vector
    long lReverb;                  // late reverberation level relative to room effect
    float flReverbDelay;           // late reverberation delay time relative to initial reflection
    EAXVECTOR vReverbPan;          // late reverberation panning vector
    float flEchoTime;              // echo time
    float flEchoDepth;             // echo depth
    float flModulationTime;        // modulation time
    float flModulationDepth;       // modulation depth
    float flAirAbsorptionHF;       // change in level per meter at high frequencies
    float flHFReference;           // reference high frequency
    float flLFReference;           // reference low frequency 
    float flRoomRolloffFactor;     // like DS3D flRolloffFactor but for room effect
    unsigned long ulFlags;         // modifies the behavior of properties
} EAXLISTENERPROPERTIES, *LPEAXLISTENERPROPERTIES;

// used by DSPROPERTY_EAXLISTENER_ENVIRONMENT
enum
{
    EAX_ENVIRONMENT_GENERIC,
    EAX_ENVIRONMENT_PADDEDCELL,
    EAX_ENVIRONMENT_ROOM,
    EAX_ENVIRONMENT_BATHROOM,
    EAX_ENVIRONMENT_LIVINGROOM,
    EAX_ENVIRONMENT_STONEROOM,
    EAX_ENVIRONMENT_AUDITORIUM,
    EAX_ENVIRONMENT_CONCERTHALL,
    EAX_ENVIRONMENT_CAVE,
    EAX_ENVIRONMENT_ARENA,
    EAX_ENVIRONMENT_HANGAR,
    EAX_ENVIRONMENT_CARPETEDHALLWAY,
    EAX_ENVIRONMENT_HALLWAY,
    EAX_ENVIRONMENT_STONECORRIDOR,
    EAX_ENVIRONMENT_ALLEY,
    EAX_ENVIRONMENT_FOREST,
    EAX_ENVIRONMENT_CITY,
    EAX_ENVIRONMENT_MOUNTAINS,
    EAX_ENVIRONMENT_QUARRY,
    EAX_ENVIRONMENT_PLAIN,
    EAX_ENVIRONMENT_PARKINGLOT,
    EAX_ENVIRONMENT_SEWERPIPE,
    EAX_ENVIRONMENT_UNDERWATER,
    EAX_ENVIRONMENT_DRUGGED,
    EAX_ENVIRONMENT_DIZZY,
    EAX_ENVIRONMENT_PSYCHOTIC,

    EAX_ENVIRONMENT_UNDEFINED,

    EAX_ENVIRONMENT_COUNT
};

// Used by DSPROPERTY_EAXLISTENER_FLAGS
//
// Note: The number and order of flags may change in future EAX versions.
//       It is recommended to use the flag defines as follows:
//              myFlags = EAXLISTENERFLAGS_DECAYTIMESCALE | EAXLISTENERFLAGS_REVERBSCALE;
//       instead of:
//              myFlags = 0x00000009;
//
// These flags determine what properties are affected by environment size.
#define EAXLISTENERFLAGS_DECAYTIMESCALE        0x00000001 // reverberation decay time
#define EAXLISTENERFLAGS_REFLECTIONSSCALE      0x00000002 // reflection level
#define EAXLISTENERFLAGS_REFLECTIONSDELAYSCALE 0x00000004 // initial reflection delay time
#define EAXLISTENERFLAGS_REVERBSCALE           0x00000008 // reflections level
#define EAXLISTENERFLAGS_REVERBDELAYSCALE      0x00000010 // late reverberation delay time
#define EAXLISTENERFLAGS_ECHOTIMESCALE         0x00000040 // echo time
#define EAXLISTENERFLAGS_MODULATIONTIMESCALE   0x00000080 // modulation time

// This flag limits high-frequency decay time according to air absorption.
#define EAXLISTENERFLAGS_DECAYHFLIMIT          0x00000020
 
#define EAXLISTENERFLAGS_RESERVED              0xFFFFFF00 // reserved future use

// Property ranges and defaults:

#define EAXLISTENER_MINENVIRONMENT                0
#define EAXLISTENER_MAXENVIRONMENT                (EAX_ENVIRONMENT_COUNT-1)
#define EAXLISTENER_DEFAULTENVIRONMENT            EAX_ENVIRONMENT_GENERIC

#define EAXLISTENER_MINENVIRONMENTSIZE            1.0f
#define EAXLISTENER_MAXENVIRONMENTSIZE            100.0f
#define EAXLISTENER_DEFAULTENVIRONMENTSIZE        7.5f

#define EAXLISTENER_MINENVIRONMENTDIFFUSION       0.0f
#define EAXLISTENER_MAXENVIRONMENTDIFFUSION       1.0f
#define EAXLISTENER_DEFAULTENVIRONMENTDIFFUSION   1.0f

#define EAXLISTENER_MINROOM                       (-10000)
#define EAXLISTENER_MAXROOM                       0
#define EAXLISTENER_DEFAULTROOM                   (-1000)

#define EAXLISTENER_MINROOMHF                     (-10000)
#define EAXLISTENER_MAXROOMHF                     0
#define EAXLISTENER_DEFAULTROOMHF                 (-100)

#define EAXLISTENER_MINROOMLF                     (-10000)
#define EAXLISTENER_MAXROOMLF                     0
#define EAXLISTENER_DEFAULTROOMLF                 0

#define EAXLISTENER_MINDECAYTIME                  0.1f
#define EAXLISTENER_MAXDECAYTIME                  20.0f
#define EAXLISTENER_DEFAULTDECAYTIME              1.49f

#define EAXLISTENER_MINDECAYHFRATIO               0.1f
#define EAXLISTENER_MAXDECAYHFRATIO               2.0f
#define EAXLISTENER_DEFAULTDECAYHFRATIO           0.83f

#define EAXLISTENER_MINDECAYLFRATIO               0.1f
#define EAXLISTENER_MAXDECAYLFRATIO               2.0f
#define EAXLISTENER_DEFAULTDECAYLFRATIO           1.00f

#define EAXLISTENER_MINREFLECTIONS                (-10000)
#define EAXLISTENER_MAXREFLECTIONS                1000
#define EAXLISTENER_DEFAULTREFLECTIONS            (-2602)

#define EAXLISTENER_MINREFLECTIONSDELAY           0.0f
#define EAXLISTENER_MAXREFLECTIONSDELAY           0.3f
#define EAXLISTENER_DEFAULTREFLECTIONSDELAY       0.007f

#define EAXLISTENER_MINREVERB                     (-10000)
#define EAXLISTENER_MAXREVERB                     2000
#define EAXLISTENER_DEFAULTREVERB                 200

#define EAXLISTENER_MINREVERBDELAY                0.0f
#define EAXLISTENER_MAXREVERBDELAY                0.1f
#define EAXLISTENER_DEFAULTREVERBDELAY            0.011f

#define EAXLISTENER_MINECHOTIME                   0.075f
#define EAXLISTENER_MAXECHOTIME	                  0.25f
#define EAXLISTENER_DEFAULTECHOTIME               0.25f

#define EAXLISTENER_MINECHODEPTH                  0.0f
#define EAXLISTENER_MAXECHODEPTH                  1.0f
#define EAXLISTENER_DEFAULTECHODEPTH              0.0f

#define EAXLISTENER_MINMODULATIONTIME             0.04f
#define EAXLISTENER_MAXMODULATIONTIME             4.0f
#define EAXLISTENER_DEFAULTMODULATIONTIME         0.25f

#define EAXLISTENER_MINMODULATIONDEPTH            0.0f
#define EAXLISTENER_MAXMODULATIONDEPTH            1.0f
#define EAXLISTENER_DEFAULTMODULATIONDEPTH        0.0f

#define EAXLISTENER_MINAIRABSORPTIONHF            (-100.0f)
#define EAXLISTENER_MAXAIRABSORPTIONHF            0.0f
#define EAXLISTENER_DEFAULTAIRABSORPTIONHF        (-5.0f)

#define EAXLISTENER_MINHFREFERENCE                1000.0f
#define EAXLISTENER_MAXHFREFERENCE                20000.0f
#define EAXLISTENER_DEFAULTHFREFERENCE            5000.0f

#define EAXLISTENER_MINLFREFERENCE                20.0f
#define EAXLISTENER_MAXLFREFERENCE                1000.0f
#define EAXLISTENER_DEFAULTLFREFERENCE            250.0f

#define EAXLISTENER_MINROOMROLLOFFFACTOR          0.0f
#define EAXLISTENER_MAXROOMROLLOFFFACTOR          10.0f
#define EAXLISTENER_DEFAULTROOMROLLOFFFACTOR      0.0f

#define EAXLISTENER_DEFAULTFLAGS                  (EAXLISTENERFLAGS_DECAYTIMESCALE |        \
                                                   EAXLISTENERFLAGS_REFLECTIONSSCALE |      \
                                                   EAXLISTENERFLAGS_REFLECTIONSDELAYSCALE | \
                                                   EAXLISTENERFLAGS_REVERBSCALE |           \
                                                   EAXLISTENERFLAGS_REVERBDELAYSCALE |      \
                                                   EAXLISTENERFLAGS_DECAYHFLIMIT)



/*
* EAX 3.0 buffer property set {A8FA6881-B476-11d3-BDB9-00C0F02DDF87}
*/
DEFINE_GUID(DSPROPSETID_EAX30_BufferProperties, 
	0xa8fa6881, 
	0xb476, 
	0x11d3, 
	0xbd, 0xb9, 0x0, 0xc0, 0xf0, 0x2d, 0xdf, 0x87);

// For compatibility with future EAX versions:
#define DSPROPSETID_EAX_BufferProperties DSPROPSETID_EAX30_BufferProperties
#define DSPROPSETID_EAX_SourceProperties DSPROPSETID_EAX30_BufferProperties

typedef enum
{
    DSPROPERTY_EAXBUFFER_NONE,
    DSPROPERTY_EAXBUFFER_ALLPARAMETERS,
    DSPROPERTY_EAXBUFFER_OBSTRUCTIONPARAMETERS,
    DSPROPERTY_EAXBUFFER_OCCLUSIONPARAMETERS,
    DSPROPERTY_EAXBUFFER_EXCLUSIONPARAMETERS,
    DSPROPERTY_EAXBUFFER_DIRECT,
    DSPROPERTY_EAXBUFFER_DIRECTHF,
    DSPROPERTY_EAXBUFFER_ROOM,
    DSPROPERTY_EAXBUFFER_ROOMHF,
    DSPROPERTY_EAXBUFFER_OBSTRUCTION,
    DSPROPERTY_EAXBUFFER_OBSTRUCTIONLFRATIO,
    DSPROPERTY_EAXBUFFER_OCCLUSION, 
    DSPROPERTY_EAXBUFFER_OCCLUSIONLFRATIO,
    DSPROPERTY_EAXBUFFER_OCCLUSIONROOMRATIO,
    DSPROPERTY_EAXBUFFER_OCCLUSIONDIRECTRATIO,
    DSPROPERTY_EAXBUFFER_EXCLUSION, 
    DSPROPERTY_EAXBUFFER_EXCLUSIONLFRATIO,
    DSPROPERTY_EAXBUFFER_OUTSIDEVOLUMEHF, 
    DSPROPERTY_EAXBUFFER_DOPPLERFACTOR, 
    DSPROPERTY_EAXBUFFER_ROLLOFFFACTOR, 
    DSPROPERTY_EAXBUFFER_ROOMROLLOFFFACTOR,
    DSPROPERTY_EAXBUFFER_AIRABSORPTIONFACTOR,
    DSPROPERTY_EAXBUFFER_FLAGS
} DSPROPERTY_EAX_BUFFERPROPERTY;    

// OR these flags with property id
#define DSPROPERTY_EAXBUFFER_IMMEDIATE 0x00000000 // changes take effect immediately
#define DSPROPERTY_EAXBUFFER_DEFERRED  0x80000000 // changes take effect later
#define DSPROPERTY_EAXBUFFER_COMMITDEFERREDSETTINGS (DSPROPERTY_EAXBUFFER_NONE | \
                                                     DSPROPERTY_EAXBUFFER_IMMEDIATE)

// Use this structure for DSPROPERTY_EAXBUFFER_ALLPARAMETERS
// - all levels are hundredths of decibels
// - all delays are in seconds
//
// NOTE: This structure may change in future EAX versions.
//       It is recommended to initialize fields by name:
//              myBuffer.lDirect = 0;
//              myBuffer.lDirectHF = -200;
//              ...
//              myBuffer.dwFlags = myFlags /* see EAXBUFFERFLAGS below */ ;
//       instead of:
//              myBuffer = { 0, -200, ... , 0x00000003 };
//
typedef struct _EAXBUFFERPROPERTIES
{
    long lDirect;                 // direct path level (at low and mid frequencies)
    long lDirectHF;               // relative direct path level at high frequencies
    long lRoom;                   // room effect level (at low and mid frequencies)
    long lRoomHF;                 // relative room effect level at high frequencies
    long lObstruction;            // main obstruction control (attenuation at high frequencies) 
    float flObstructionLFRatio;   // obstruction low-frequency level re. main control
    long lOcclusion;              // main occlusion control (attenuation at high frequencies)
    float flOcclusionLFRatio;     // occlusion low-frequency level re. main control
    float flOcclusionRoomRatio;   // relative occlusion control for room effect
    float flOcclusionDirectRatio; // relative occlusion control for direct path
    long lExclusion;              // main exlusion control (attenuation at high frequencies)
    float flExclusionLFRatio;     // exclusion low-frequency level re. main control
    long lOutsideVolumeHF;        // outside sound cone level at high frequencies
    float flDopplerFactor;        // like DS3D flDopplerFactor but per source
    float flRolloffFactor;        // like DS3D flRolloffFactor but per source
    float flRoomRolloffFactor;    // like DS3D flRolloffFactor but for room effect
    float flAirAbsorptionFactor;  // multiplies DSPROPERTY_EAXLISTENER_AIRABSORPTIONHF
    unsigned long ulFlags;        // modifies the behavior of properties
} EAXBUFFERPROPERTIES, *LPEAXBUFFERPROPERTIES;

// Use this structure for DSPROPERTY_EAXBUFFER_OBSTRUCTION,
typedef struct _EAXOBSTRUCTIONPROPERTIES
{
    long lObstruction;
    float flObstructionLFRatio;
} EAXOBSTRUCTIONPROPERTIES, *LPEAXOBSTRUCTIONPROPERTIES;

// Use this structure for DSPROPERTY_EAXBUFFER_OCCLUSION
typedef struct _EAXOCCLUSIONPROPERTIES
{
    long lOcclusion;
    float flOcclusionLFRatio;
    float flOcclusionRoomRatio;
    float flOcclusionDirectRatio;
} EAXOCCLUSIONPROPERTIES, *LPEAXOCCLUSIONPROPERTIES;

// Use this structure for DSPROPERTY_EAXBUFFER_EXCLUSION
typedef struct _EAXEXCLUSIONPROPERTIES
{
    long lExclusion;
    float flExclusionLFRatio;
} EAXEXCLUSIONPROPERTIES, *LPEAXEXCLUSIONPROPERTIES;

// Used by DSPROPERTY_EAXBUFFER_FLAGS
//    TRUE:    value is computed automatically - property is an offset
//    FALSE:   value is used directly
//
// Note: The number and order of flags may change in future EAX versions.
//       To insure future compatibility, use flag defines as follows:
//              myFlags = EAXBUFFERFLAGS_DIRECTHFAUTO | EAXBUFFERFLAGS_ROOMAUTO;
//       instead of:
//              myFlags = 0x00000003;
//
#define EAXBUFFERFLAGS_DIRECTHFAUTO          0x00000001 // affects DSPROPERTY_EAXBUFFER_DIRECTHF
#define EAXBUFFERFLAGS_ROOMAUTO              0x00000002 // affects DSPROPERTY_EAXBUFFER_ROOM
#define EAXBUFFERFLAGS_ROOMHFAUTO            0x00000004 // affects DSPROPERTY_EAXBUFFER_ROOMHF

#define EAXBUFFERFLAGS_RESERVED              0xFFFFFFF8 // reserved future use

// Property ranges and defaults:

#define EAXBUFFER_MINDIRECT                    (-10000)
#define EAXBUFFER_MAXDIRECT                    1000
#define EAXBUFFER_DEFAULTDIRECT                0

#define EAXBUFFER_MINDIRECTHF                  (-10000)
#define EAXBUFFER_MAXDIRECTHF                  0
#define EAXBUFFER_DEFAULTDIRECTHF              0

#define EAXBUFFER_MINROOM                      (-10000)
#define EAXBUFFER_MAXROOM                      1000
#define EAXBUFFER_DEFAULTROOM                  0

#define EAXBUFFER_MINROOMHF                    (-10000)
#define EAXBUFFER_MAXROOMHF                    0
#define EAXBUFFER_DEFAULTROOMHF                0

#define EAXBUFFER_MINOBSTRUCTION               (-10000)
#define EAXBUFFER_MAXOBSTRUCTION               0
#define EAXBUFFER_DEFAULTOBSTRUCTION           0

#define EAXBUFFER_MINOBSTRUCTIONLFRATIO        0.0f
#define EAXBUFFER_MAXOBSTRUCTIONLFRATIO        1.0f
#define EAXBUFFER_DEFAULTOBSTRUCTIONLFRATIO    0.0f

#define EAXBUFFER_MINOCCLUSION                 (-10000)
#define EAXBUFFER_MAXOCCLUSION                 0
#define EAXBUFFER_DEFAULTOCCLUSION             0

#define EAXBUFFER_MINOCCLUSIONLFRATIO          0.0f
#define EAXBUFFER_MAXOCCLUSIONLFRATIO          1.0f
#define EAXBUFFER_DEFAULTOCCLUSIONLFRATIO      0.25f

#define EAXBUFFER_MINOCCLUSIONROOMRATIO        0.0f
#define EAXBUFFER_MAXOCCLUSIONROOMRATIO        10.0f
#define EAXBUFFER_DEFAULTOCCLUSIONROOMRATIO    1.5f

#define EAXBUFFER_MINOCCLUSIONDIRECTRATIO      0.0f
#define EAXBUFFER_MAXOCCLUSIONDIRECTRATIO      10.0f
#define EAXBUFFER_DEFAULTOCCLUSIONDIRECTRATIO  1.0f

#define EAXBUFFER_MINEXCLUSION                 (-10000)
#define EAXBUFFER_MAXEXCLUSION                 0
#define EAXBUFFER_DEFAULTEXCLUSION             0

#define EAXBUFFER_MINEXCLUSIONLFRATIO          0.0f
#define EAXBUFFER_MAXEXCLUSIONLFRATIO          1.0f
#define EAXBUFFER_DEFAULTEXCLUSIONLFRATIO      1.0f

#define EAXBUFFER_MINOUTSIDEVOLUMEHF           (-10000)
#define EAXBUFFER_MAXOUTSIDEVOLUMEHF           0
#define EAXBUFFER_DEFAULTOUTSIDEVOLUMEHF       0

#define EAXBUFFER_MINDOPPLERFACTOR             0.0f
#define EAXBUFFER_MAXDOPPLERFACTOR             10.f
#define EAXBUFFER_DEFAULTDOPPLERFACTOR         0.0f

#define EAXBUFFER_MINROLLOFFFACTOR             0.0f
#define EAXBUFFER_MAXROLLOFFFACTOR             10.f
#define EAXBUFFER_DEFAULTROLLOFFFACTOR         0.0f

#define EAXBUFFER_MINROOMROLLOFFFACTOR         0.0f
#define EAXBUFFER_MAXROOMROLLOFFFACTOR         10.f
#define EAXBUFFER_DEFAULTROOMROLLOFFFACTOR     0.0f

#define EAXBUFFER_MINAIRABSORPTIONFACTOR       0.0f
#define EAXBUFFER_MAXAIRABSORPTIONFACTOR       10.0f
#define EAXBUFFER_DEFAULTAIRABSORPTIONFACTOR   1.0f

#define EAXBUFFER_DEFAULTFLAGS                 (EAXBUFFERFLAGS_DIRECTHFAUTO |       \
                                                EAXBUFFERFLAGS_ROOMAUTO |           \
                                                EAXBUFFERFLAGS_ROOMHFAUTO )

#pragma pack(pop)

#ifdef __cplusplus
}
#endif // __cplusplus

#endif
