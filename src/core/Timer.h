#pragma once

#ifdef FIX_HIGH_FPS_BUGS_ON_FRONTEND
#define PauseModeTime double
#else
#define PauseModeTime uint32
#endif

class CTimer
{

	static uint32 m_snTimeInMilliseconds;
	static PauseModeTime m_snTimeInMillisecondsPauseMode;
	static uint32 m_snTimeInMillisecondsNonClipped;
	static uint32 m_snPreviousTimeInMilliseconds;
	static uint32 m_FrameCounter;
	static float ms_fTimeScale;
	static float ms_fTimeStep;
	static float ms_fTimeStepNonClipped;
public:
	static bool  m_UserPause;
	static bool  m_CodePause;

	static const float &GetTimeStep(void) { return ms_fTimeStep; }
	static void SetTimeStep(float ts) { ms_fTimeStep = ts; }
	static float GetTimeStepInSeconds() { return ms_fTimeStep / 50.0f; }
	static uint32 GetTimeStepInMilliseconds() { return ms_fTimeStep / 50.0f * 1000.0f; }
	static const float &GetTimeStepNonClipped(void) { return ms_fTimeStepNonClipped; }
	static float GetTimeStepNonClippedInSeconds(void) { return ms_fTimeStepNonClipped / 50.0f; }
	static uint32 GetTimeStepNonClippedInMilliseconds(void) { return ms_fTimeStepNonClipped / 50.0f * 1000.0f; }
	static void SetTimeStepNonClipped(float ts) { ms_fTimeStepNonClipped = ts; }
	static const uint32 &GetFrameCounter(void) { return m_FrameCounter; }
	static void SetFrameCounter(uint32 fc) { m_FrameCounter = fc; }
	static const uint32 &GetTimeInMilliseconds(void) { return m_snTimeInMilliseconds; }
	static void SetTimeInMilliseconds(uint32 t) { m_snTimeInMilliseconds = t; }
	static uint32 GetTimeInMillisecondsNonClipped(void) { return m_snTimeInMillisecondsNonClipped; }
	static void SetTimeInMillisecondsNonClipped(uint32 t) { m_snTimeInMillisecondsNonClipped = t; }
	static PauseModeTime GetTimeInMillisecondsPauseMode(void) { return m_snTimeInMillisecondsPauseMode; }
	static void SetTimeInMillisecondsPauseMode(uint32 t) { m_snTimeInMillisecondsPauseMode = t; }
	static uint32 GetPreviousTimeInMilliseconds(void) { return m_snPreviousTimeInMilliseconds; }
	static void SetPreviousTimeInMilliseconds(uint32 t) { m_snPreviousTimeInMilliseconds = t; }
	static const float &GetTimeScale(void) { return ms_fTimeScale; }
	static void SetTimeScale(float ts) { ms_fTimeScale = ts; }
	static uint32 GetCyclesPerFrame();

	static bool GetIsPaused() { return m_UserPause || m_CodePause; }
	static bool GetIsUserPaused() { return m_UserPause; }
	static bool GetIsCodePaused() { return m_CodePause; }
	static void SetCodePause(bool pause) { m_CodePause = pause; }
	
	static void Initialise(void);
	static void Shutdown(void);
	static void Update(void);
	static void Suspend(void);
	static void Resume(void);
	static uint32 GetCyclesPerMillisecond(void);
	static uint32 GetCurrentTimeInCycles(void);
	static bool GetIsSlowMotionActive(void);
	static void Stop(void);
	static void StartUserPause(void);
	static void EndUserPause(void);

	friend bool GenericLoad(void);
	friend bool GenericSave(int file);

#ifdef FIX_BUGS
	static float GetDefaultTimeStep(void) { return 50.0f / 30.0f; }
	static float GetTimeStepFix(void) { return GetTimeStep() / GetDefaultTimeStep(); }
#endif
};

#ifdef FIX_HIGH_FPS_BUGS_ON_FRONTEND
extern double frameTime;
#endif
