#ifndef PLATFORM_H
#define PLATFORM_H

#include "rwcore.h"
#include "skeleton.h"

#ifdef    __cplusplus
extern "C"
{
#endif                          /* __cplusplus */

#ifdef _WIN32
extern RwUInt32 psTimer(void);
#else
extern double psTimer(void);
#endif

extern RwBool psInitialize(void);
extern void   psTerminate(void);

extern void psCameraShowRaster(RwCamera *camera);
extern RwBool psCameraBeginUpdate(RwCamera *camera);

extern void psMouseSetPos(RwV2d *pos);

extern RwBool psSelectDevice();

extern RwMemoryFunctions *psGetMemoryFunctions(void);

/* install the platform specific file system */
extern RwBool psInstallFileSystem(void);


/* Handle native texture support */
extern RwBool psNativeTextureSupport(void);

extern void _InputTranslateShiftKeyUpDown(RsKeyCodes* rs);

extern void HandleExit();

extern void _psSelectScreenVM(RwInt32 videoMode);

extern void InitialiseLanguage();

extern RwBool _psSetVideoMode(RwInt32 subSystem, RwInt32 videoMode);

extern RwChar** _psGetVideoModeList();

extern RwInt32 _psGetNumVideModes();
#ifdef    __cplusplus
}
#endif                          /* __cplusplus */

#endif /* PLATFORM_H */
