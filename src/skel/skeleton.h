#ifndef SKELETON_H
#define SKELETON_H

#include "rwcore.h"

/* Default arena size depending on platform. */
#define rsRESOURCESDEFAULTARENASIZE (1 << 20)

#if (!defined(RsSprintf))
#define RsSprintf rwsprintf
#endif /* (!defined(RsSprintf)) */

#ifdef    __cplusplus
extern "C"
{
#endif                          /* __cplusplus */

#if (!defined(RSASSERT))
#define RSASSERT(_condition) /* No-op */
#endif /* (!defined(RSASSERT)) */

#define RSASSERTISTYPE(_f, _t) \
   RSASSERT( (!(_f)) || ((((const RwObject *)(_f))->type)==(_t)) )

enum RsInputDeviceType
{
	rsKEYBOARD,
	rsMOUSE,
	rsPAD
};
typedef enum RsInputDeviceType RsInputDeviceType;

enum RsEventStatus
{
	rsEVENTERROR,
	rsEVENTPROCESSED,
	rsEVENTNOTPROCESSED
};
typedef enum RsEventStatus RsEventStatus;

enum RsEvent
{
	rsCAMERASIZE,
	rsCOMMANDLINE,
	rsFILELOAD,
	rsINITDEBUG,
	rsINPUTDEVICEATTACH,
	rsLEFTBUTTONDOWN,
	rsLEFTBUTTONUP,
	rsMOUSEMOVE,
	rsMOUSEWHEELMOVE,
	rsPLUGINATTACH,
	rsREGISTERIMAGELOADER,
	rsRIGHTBUTTONDOWN,
	rsRIGHTBUTTONUP,
	_rs_13,
	_rs_14,
	_rs_15,
	_rs_16,
	_rs_17,
	_rs_18,
	_rs_19,
	_rs_20,
	rsRWINITIALIZE,
	rsRWTERMINATE,
	rsSELECTDEVICE,
	rsINITIALIZE,
	rsTERMINATE,
	rsIDLE,
	rsFRONTENDIDLE,
	rsKEYDOWN,
	rsKEYUP,
	rsQUITAPP,
	rsPADBUTTONDOWN,
	rsPADBUTTONUP,
	rsPADANALOGUELEFT,
	rsPADANALOGUELEFTRESET,
	rsPADANALOGUERIGHT,
	rsPADANALOGUERIGHTRESET,
	rsPREINITCOMMANDLINE,
	rsACTIVATE,
};

typedef enum RsEvent RsEvent;

typedef RsEventStatus (*RsInputEventHandler)(RsEvent event, void *param);

typedef struct RsInputDevice RsInputDevice;
struct RsInputDevice
{
	RsInputDeviceType inputDeviceType;
	RwBool used;
	RsInputEventHandler inputEventHandler;
};

typedef struct RsGlobalType RsGlobalType;
struct RsGlobalType
{
	const RwChar *appName;
	RwInt32 width;
	RwInt32 height;
	RwInt32 maximumWidth;
	RwInt32 maximumHeight;
	RwInt32 maxFPS;
	RwBool  quit;

	void   *ps; /* platform specific data */

	RsInputDevice keyboard;
	RsInputDevice mouse;
	RsInputDevice pad;
};

enum RsKeyCodes
{
	rsESC            = 1000,
	
	rsF1             = 1001,
	rsF2             = 1002,
	rsF3             = 1003,
	rsF4             = 1004,
	rsF5             = 1005,
	rsF6             = 1006,
	rsF7             = 1007,
	rsF8             = 1008,
	rsF9             = 1009,
	rsF10            = 1010,
	rsF11            = 1011,
	rsF12            = 1012,
	
	rsINS            = 1013,
	rsDEL            = 1014,
	rsHOME           = 1015,
	rsEND            = 1016,
	rsPGUP           = 1017,
	rsPGDN           = 1018,
	
	rsUP             = 1019,
	rsDOWN           = 1020,
	rsLEFT           = 1021,
	rsRIGHT          = 1022,
	
	rsDIVIDE         = 1023,
	rsTIMES          = 1024,
	rsPLUS           = 1025,
	rsMINUS          = 1026,
	rsPADDEL         = 1027,
	rsPADEND         = 1028,
	rsPADDOWN        = 1029,
	rsPADPGDN        = 1030,
	rsPADLEFT        = 1031,
	rsPAD5           = 1032,
	rsNUMLOCK        = 1033,
	rsPADRIGHT       = 1034,
	rsPADHOME        = 1035,
	rsPADUP          = 1036,
	rsPADPGUP        = 1037,
	rsPADINS         = 1038,
	rsPADENTER       = 1039,
	
	rsSCROLL         = 1040,
	rsPAUSE          = 1041,
	
	rsBACKSP         = 1042,
	rsTAB            = 1043,
	rsCAPSLK         = 1044,
	rsENTER          = 1045,
	rsLSHIFT         = 1046,
	rsRSHIFT         = 1047,
	rsSHIFT          = 1048,
	rsLCTRL          = 1049,
	rsRCTRL          = 1050,
	rsLALT           = 1051,
	rsRALT           = 1052,
	rsLWIN           = 1053,
	rsRWIN           = 1054,
	rsAPPS           = 1055,
	
	rsNULL           = 1056,

	rsMOUSELEFTBUTTON      = 1,
	rsMOUSMIDDLEBUTTON     = 2,
	rsMOUSERIGHTBUTTON     = 3,
	rsMOUSEWHEELUPBUTTON   = 4,
	rsMOUSEWHEELDOWNBUTTON = 5,
	rsMOUSEX1BUTTON        = 6,
	rsMOUSEX2BUTTON        = 7,
};
typedef enum RsKeyCodes RsKeyCodes;

typedef struct RsKeyStatus RsKeyStatus;
struct RsKeyStatus
{
	RwInt32     keyCharCode;
};

typedef struct RsPadButtonStatus RsPadButtonStatus;
struct RsPadButtonStatus
{
	RwInt32     padID;
};

enum RsPadButtons
{
	rsPADNULL       = 0,
	
	rsPADBUTTON1    = 1,
	rsPADBUTTON2    = 2,
	rsPADBUTTON3    = 3,
	rsPADBUTTON4    = 4,
	
	rsPADBUTTON5    = 5,
	rsPADBUTTON6    = 6,
	rsPADBUTTON7    = 7,
	rsPADBUTTON8    = 8,
	
	rsPADSELECT     = 9,
	
	rsPADBUTTONA1   = 10,
	rsPADBUTTONA2   = 11,
	
	rsPADSTART      = 12,
	
	rsPADDPADUP     = 13,
	rsPADDPADRIGHT  = 14,
	rsPADDPADDOWN   = 15,
	rsPADDPADLEFT   = 16,
};
typedef enum RsPadButtons RsPadButtons;


extern RsGlobalType RsGlobal;

extern RsEventStatus AppEventHandler(RsEvent event, void *param);
extern RwBool        AttachInputDevices(void);

extern RsEventStatus RsEventHandler(RsEvent event, void *param);
extern RsEventStatus RsKeyboardEventHandler(RsEvent event, void *param);
extern RsEventStatus RsPadEventHandler(RsEvent event, void *param);

extern RwBool
RsInitialize(void);

extern RwBool
RsRegisterImageLoader(void);

extern RwBool
RsRwInitialize(void *param);

extern RwBool
RsSelectDevice(void);

extern RwBool
RsInputDeviceAttach(RsInputDeviceType inputDevice,
					RsInputEventHandler inputEventHandler);

#ifdef _WIN32
extern RwUInt32    
#else
extern double
#endif             
RsTimer(void);

extern void                     
RsCameraShowRaster(RwCamera *camera);

extern RwBool                     
RsCameraBeginUpdate(RwCamera *camera);

//TODO
//extern void                     
//RsMouseSetVisibility(RwBool visible);

extern RwImage*
RsGrabScreen(RwCamera *camera);

extern void                     
RsMouseSetPos(RwV2d *pos);

extern void                     
RsRwTerminate(void);

extern void                     
RsTerminate(void);

#ifdef    __cplusplus
}
#endif                          /* __cplusplus */

#endif /* SKELETON_H */
