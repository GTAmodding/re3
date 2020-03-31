#include <windows.h>
#include "common.h"
#include "patcher.h"
#include "DMAudio.h"
#include "Record.h"
#include "Timer.h"

uint32 &CTimer::m_snTimeInMilliseconds = *(uint32*)0x885B48;
uint32 &CTimer::m_snTimeInMillisecondsPauseMode = *(uint32*)0x5F7614;
uint32 &CTimer::m_snTimeInMillisecondsNonClipped = *(uint32*)0x9412E8;
uint32 &CTimer::m_snPreviousTimeInMilliseconds = *(uint32*)0x8F29E4;
uint32 &CTimer::m_FrameCounter = *(uint32*)0x9412EC;
float &CTimer::ms_fTimeScale = *(float*)0x8F2C20;
float &CTimer::ms_fTimeStep = *(float*)0x8E2CB4;
float &CTimer::ms_fTimeStepNonClipped = *(float*)0x8E2C4C;
bool  &CTimer::m_UserPause = *(bool*)0x95CD7C;
bool  &CTimer::m_CodePause = *(bool*)0x95CDB1;

//UInt32 oldPcTimer;
uint32 &oldPcTimer = *(uint32*)0x9434F4;

//UInt32 suspendPcTimer;
uint32 &suspendPcTimer = *(uint32*)0x62A308;

//UInt32 _nCyclesPerMS = 1;
uint32 &_nCyclesPerMS = *(uint32*)0x5F7610;

//LARGE_INTEGER _oldPerfCounter;
LARGE_INTEGER &_oldPerfCounter = *(LARGE_INTEGER*)0x62A310;

//LARGE_INTEGER perfSuspendCounter;
LARGE_INTEGER &perfSuspendCounter = *(LARGE_INTEGER*)0x62A318;

//UInt32 suspendDepth;
uint32 &suspendDepth = *(uint32*)0x62A320;

void CTimer::Initialise(void)
{
	debug("Initialising CTimer...\n");
	
	ms_fTimeScale = 1.0f;
	ms_fTimeStep = 1.0f;
	suspendDepth = 0;
	m_UserPause = false;
	m_CodePause = false;
	m_snTimeInMillisecondsNonClipped = 0;
	m_snPreviousTimeInMilliseconds = 0;
	m_snTimeInMilliseconds = 1;
	
	LARGE_INTEGER perfFreq;
	if ( QueryPerformanceFrequency(&perfFreq) )
	{
		OutputDebugString("Performance counter available\n");
		_nCyclesPerMS = uint32(perfFreq.QuadPart / 1000);
		QueryPerformanceCounter(&_oldPerfCounter);
	}
	else
	{
		OutputDebugString("Performance counter not available, using millesecond timer\n");
		_nCyclesPerMS = 0;
		oldPcTimer = RsTimer();
	}
	
	m_snTimeInMilliseconds = m_snPreviousTimeInMilliseconds;
	
	m_FrameCounter = 0;
	
	DMAudio.ResetTimers(m_snPreviousTimeInMilliseconds);
	
	debug("CTimer ready\n");
}

void CTimer::Shutdown(void)
{
	;
}

void CTimer::Update(void)
{
	m_snPreviousTimeInMilliseconds = m_snTimeInMilliseconds;
	
	if ( (double)_nCyclesPerMS != 0.0 )
	{
		LARGE_INTEGER pc;
		QueryPerformanceCounter(&pc);
		
		int32 updInCycles = (pc.LowPart - _oldPerfCounter.LowPart); // & 0x7FFFFFFF; pointless
		
		_oldPerfCounter = pc;
		
		float updInCyclesScaled = updInCycles * ms_fTimeScale;
		
		double upd = updInCyclesScaled / (double)_nCyclesPerMS;

		m_snTimeInMillisecondsPauseMode = m_snTimeInMillisecondsPauseMode + upd;
		
		if ( GetIsPaused() )
			ms_fTimeStep = 0.0f;
		else
		{
			m_snTimeInMilliseconds = m_snTimeInMilliseconds + upd;
			m_snTimeInMillisecondsNonClipped = m_snTimeInMillisecondsNonClipped + upd;
			ms_fTimeStep = updInCyclesScaled / (double)_nCyclesPerMS / 20.0f;
		}
	}
	else
	{
		uint32 timer = RsTimer();
		
		uint32 updInMs = timer - oldPcTimer;
		
		double upd = (double)updInMs * ms_fTimeScale;
		
		oldPcTimer = timer;
		
		m_snTimeInMillisecondsPauseMode = m_snTimeInMillisecondsPauseMode + upd;
															 
		if ( GetIsPaused() )
			ms_fTimeStep = 0.0f;
		else
		{
			m_snTimeInMilliseconds = m_snTimeInMilliseconds + upd;
			m_snTimeInMillisecondsNonClipped = m_snTimeInMillisecondsNonClipped + upd;
			ms_fTimeStep = upd / 1000.0f * 50.0f;
		}
	}
	
	if ( ms_fTimeStep < 0.01f && !GetIsPaused() )
		ms_fTimeStep = 0.01f;

	ms_fTimeStepNonClipped = ms_fTimeStep;
	
	if ( CRecordDataForGame::RecordingState != RECORDSTATE_2 )
	{
		ms_fTimeStep = min(3.0f, ms_fTimeStep);

		if ( (m_snTimeInMilliseconds - m_snPreviousTimeInMilliseconds) > 60 )
			m_snTimeInMilliseconds = m_snPreviousTimeInMilliseconds + 60;
	}
  
	if ( CRecordDataForChase::Status == RECORDSTATE_1 )
	{
		ms_fTimeStep = 1.0f;
		m_snTimeInMilliseconds = m_snPreviousTimeInMilliseconds + 16;
	}
  
	m_FrameCounter++;
}

void CTimer::Suspend(void)
{
	if ( ++suspendDepth > 1 )
		return;
	
	if ( (double)_nCyclesPerMS != 0.0 )
		QueryPerformanceCounter(&perfSuspendCounter);
	else
		suspendPcTimer = RsTimer();
}

void CTimer::Resume(void)
{
	if ( --suspendDepth != 0 )
		return;

	if ( (double)_nCyclesPerMS != 0.0 )
	{
		LARGE_INTEGER pc;
		QueryPerformanceCounter(&pc);

		_oldPerfCounter.LowPart += pc.LowPart - perfSuspendCounter.LowPart;
	}
	else
		oldPcTimer += RsTimer() - suspendPcTimer;
}

uint32 CTimer::GetCyclesPerMillisecond(void)
{
	if (_nCyclesPerMS != 0)
		return _nCyclesPerMS;
	else 
		return 1;
}

uint32 CTimer::GetCurrentTimeInCycles(void)
{
	if ( _nCyclesPerMS != 0 )
	{
		LARGE_INTEGER pc;
		QueryPerformanceCounter(&pc);
		return (pc.LowPart - _oldPerfCounter.LowPart); // & 0x7FFFFFFF; pointless
	}
	else
		return RsTimer() - oldPcTimer;
}

bool CTimer::GetIsSlowMotionActive(void)
{
	return ms_fTimeScale < 1.0f;
}

void CTimer::Stop(void)
{
	m_snPreviousTimeInMilliseconds = m_snTimeInMilliseconds;
}

void CTimer::StartUserPause(void)
{
	m_UserPause = true;
}

void CTimer::EndUserPause(void)
{
	m_UserPause = false;
}

uint32 CTimer::GetCyclesPerFrame()
{
	return 20;
}

#if 1
STARTPATCHES	
	InjectHook(0x4ACE60, CTimer::Initialise, PATCH_JUMP);
	InjectHook(0x4ACF60, CTimer::Shutdown, PATCH_JUMP);
	InjectHook(0x4ACF70, CTimer::Update, PATCH_JUMP);
	InjectHook(0x4AD310, CTimer::Suspend, PATCH_JUMP);
	InjectHook(0x4AD370, CTimer::Resume, PATCH_JUMP);
	InjectHook(0x4AD3F0, CTimer::GetCyclesPerMillisecond, PATCH_JUMP);
	InjectHook(0x4AD410, CTimer::GetCurrentTimeInCycles, PATCH_JUMP);
	InjectHook(0x4AD450, CTimer::GetIsSlowMotionActive, PATCH_JUMP);
	InjectHook(0x4AD480, CTimer::Stop, PATCH_JUMP);
	InjectHook(0x4AD490, CTimer::StartUserPause, PATCH_JUMP);
	InjectHook(0x4AD4A0, CTimer::EndUserPause, PATCH_JUMP);
ENDPATCHES
#endif
