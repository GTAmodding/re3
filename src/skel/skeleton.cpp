#include "common.h"


#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include <string.h>

#include "rwcore.h"

#include "skeleton.h"
#include "platform.h"
#include "main.h"
#include "MemoryHeap.h"

static RwBool               DefaultVideoMode = TRUE;

RsGlobalType                RsGlobal;

#ifdef _WIN32
RwUInt32    
#else
double
#endif
RsTimer(void)
{
	return psTimer();
}


/*
 *****************************************************************************
 */
void
RsCameraShowRaster(RwCamera * camera)
{
	psCameraShowRaster(camera);

	return;
}

/*
 *****************************************************************************
 */
RwBool
RsCameraBeginUpdate(RwCamera * camera)
{
	return psCameraBeginUpdate(camera);
}

/*
 *****************************************************************************
 */
RwImage*
RsGrabScreen(RwCamera *camera)
{
	return psGrabScreen(camera);
}

/*
 *****************************************************************************
 */
RwBool
RsRegisterImageLoader(void)
{
	return TRUE;
}

/*
 *****************************************************************************
 */
static              RwBool
RsSetDebug(void)
{
	return TRUE;
}

/*
 *****************************************************************************
 */
void
RsMouseSetPos(RwV2d * pos)
{
	psMouseSetPos(pos);

	return;
}

/*
 *****************************************************************************
 */
RwBool
RsSelectDevice(void)
{
	return psSelectDevice();
}

/*
 *****************************************************************************
 */
RwBool
RsInputDeviceAttach(RsInputDeviceType inputDevice,
					RsInputEventHandler inputEventHandler)
{
	switch (inputDevice)
	{
		case rsKEYBOARD:
			{
				RsGlobal.keyboard.inputEventHandler = inputEventHandler;
				RsGlobal.keyboard.used = TRUE;
				break;
			}
		case rsMOUSE:
			{
				RsGlobal.mouse.inputEventHandler = inputEventHandler;
				RsGlobal.mouse.used = TRUE;
				break;
			}
		case rsPAD:
			{
				RsGlobal.pad.inputEventHandler = inputEventHandler;
				RsGlobal.pad.used = TRUE;
				break;
			}
		default:
			{
				return FALSE;
			}
	}

	return TRUE;
}


/*
 *****************************************************************************
 */
static RwBool
rsCommandLine(RwChar *arg)
{
	RsEventHandler(rsFILELOAD, arg);

	return TRUE;
}


/*
 *****************************************************************************
 */
static RwBool
rsPreInitCommandLine(RwChar *arg)
{
	if( !strcmp(arg, RWSTRING("-vms")) )
	{
		DefaultVideoMode = FALSE;

		return TRUE;
	}
#ifndef MASTER
	if (!strcmp(arg, RWSTRING("-animviewer")))
	{
		gbModelViewer = TRUE;

		return TRUE;
	}
#endif
	return FALSE;
}

/*
 *****************************************************************************
 */
RsEventStatus
RsKeyboardEventHandler(RsEvent event, void *param)
{
	if (RsGlobal.keyboard.used)
	{
		return RsGlobal.keyboard.inputEventHandler(event, param);
	}

	return rsEVENTNOTPROCESSED;
}

/*
 *****************************************************************************
 */
RsEventStatus
RsPadEventHandler(RsEvent event, void *param)
{
	if (RsGlobal.pad.used)
	{
		return RsGlobal.pad.inputEventHandler(event, param);
	}

	return rsEVENTNOTPROCESSED;
}

/*
 *****************************************************************************
 */
RsEventStatus
RsEventHandler(RsEvent event, void *param)
{
	RsEventStatus       result;
	RsEventStatus       es;
  
	/*
	 * Give the application an opportunity to override any events...
	 */
	es = AppEventHandler(event, param);

	/*
	 * We never allow the app to replace the quit behaviour,
	 * only to intercept...
	 */
	if (event == rsQUITAPP)
	{
		/*
		 * Set the flag which causes the event loop to exit...
		 */
		RsGlobal.quit = TRUE;
	}

	if (es == rsEVENTNOTPROCESSED)
	{
		switch (event)
		{
			case rsSELECTDEVICE:
				result =
					(RsSelectDevice()? rsEVENTPROCESSED : rsEVENTERROR);
				break;

			case rsCOMMANDLINE:
				result = (rsCommandLine((RwChar *) param) ?
						  rsEVENTPROCESSED : rsEVENTERROR);
				break;
			case rsPREINITCOMMANDLINE:
				result = (rsPreInitCommandLine((RwChar *) param) ?
						  rsEVENTPROCESSED : rsEVENTERROR);
				break;
			case rsINITDEBUG:
				result =
					(RsSetDebug()? rsEVENTPROCESSED : rsEVENTERROR);
				break;

			case rsREGISTERIMAGELOADER:
				result = (RsRegisterImageLoader()?
						  rsEVENTPROCESSED : rsEVENTERROR);
				break;

			case rsRWTERMINATE:
				RsRwTerminate();
				result = (rsEVENTPROCESSED);
				break;

			case rsRWINITIALIZE:
				result = (RsRwInitialize(param) ?
						  rsEVENTPROCESSED : rsEVENTERROR);
				break;

			case rsTERMINATE:
				RsTerminate();
				result = (rsEVENTPROCESSED);
				break;

			case rsINITIALIZE:
				result =
					(RsInitialize()? rsEVENTPROCESSED : rsEVENTERROR);
				break;

			default:
				result = (es);
				break;

		}
	}
	else
	{
		result = (es);
	}

	return result;
}

/*
 *****************************************************************************
 */
void
RsRwTerminate(void)
{
	/* Close RenderWare */

	RwEngineStop();
	RwEngineClose();
	RwEngineTerm();

	return;
}

/*
 *****************************************************************************
 */
RwBool
RsRwInitialize(void *displayID)
{
	RwEngineOpenParams  openParams;

	PUSH_MEMID(MEMID_RENDER);	// NB: not popped on failed return

	/*
	 * Start RenderWare...
	 */
	 
	if (!RwEngineInit(psGetMemoryFunctions(), 0, rsRESOURCESDEFAULTARENASIZE))
	{
		return (FALSE);
	}

	/*
	 * Install any platform specific file systems...
	 */
	psInstallFileSystem();
	
	/*
	 * Initialize debug message handling...
	 */
	RsEventHandler(rsINITDEBUG, nil);

	/*
	 * Attach all plugins...
	 */
	if (RsEventHandler(rsPLUGINATTACH, nil) == rsEVENTERROR)
	{
		return (FALSE);
	}

	/*
	 * Attach input devices...
	 */
	if (RsEventHandler(rsINPUTDEVICEATTACH, nil) == rsEVENTERROR)
	{
		return (FALSE);
	}
	
	openParams.displayID = displayID;

	if (!RwEngineOpen(&openParams))
	{
		RwEngineTerm();
		return (FALSE);
	}
	
	if (RsEventHandler(rsSELECTDEVICE, displayID) == rsEVENTERROR)
	{
		RwEngineClose();
		RwEngineTerm();
		return (FALSE);
	}
	
	if (!RwEngineStart())
	{
		RwEngineClose();
		RwEngineTerm();
		return (FALSE);
	}

	/*
	 * Register loaders for an image with a particular file extension...
	 */
	RsEventHandler(rsREGISTERIMAGELOADER, nil);

	psNativeTextureSupport();

	RwTextureSetMipmapping(FALSE);
	RwTextureSetAutoMipmapping(FALSE);

	POP_MEMID();

	return TRUE;
}

/*
 *****************************************************************************
 */
void
RsTerminate(void)
{
	psTerminate();

	return;
}

/*
 *****************************************************************************
 */
RwBool
RsInitialize(void)
{
	/*
	 * Initialize Platform independent data...
	 */
	RwBool              result;

	RsGlobal.appName = RWSTRING("GTA3");
	RsGlobal.maximumWidth = DEFAULT_SCREEN_WIDTH;
	RsGlobal.maximumHeight = DEFAULT_SCREEN_HEIGHT;
	RsGlobal.width = DEFAULT_SCREEN_WIDTH;
	RsGlobal.height = DEFAULT_SCREEN_HEIGHT;
	
	RsGlobal.maxFPS = 30;
	 
	RsGlobal.quit = FALSE;

	/* setup the keyboard */
	RsGlobal.keyboard.inputDeviceType = rsKEYBOARD;
	RsGlobal.keyboard.inputEventHandler = nil;
	RsGlobal.keyboard.used = FALSE;

	/* setup the mouse */
	RsGlobal.mouse.inputDeviceType = rsMOUSE;
	RsGlobal.mouse.inputEventHandler = nil;
	RsGlobal.mouse.used = FALSE;

	/* setup the pad */
	RsGlobal.pad.inputDeviceType = rsPAD;
	RsGlobal.pad.inputEventHandler = nil;
	RsGlobal.pad.used = FALSE;

	result = psInitialize();

	return result;
}
