#include "common.h"
#ifndef MASTER
#include "Font.h"
#include "Frontend.h"
#include "Timer.h"
#include "Text.h"

#define MAX_TIMERS (50)
#define MAX_MS_COLLECTED (40)

// enables frame time output
#define FRAMETIME

struct sTimeBar
{
	char name[20];
	float startTime;
	float endTime;
	int32 unk;
};

struct
{
	sTimeBar Timers[MAX_TIMERS];
	uint32 count;
} TimerBar;
float MaxTimes[MAX_TIMERS];
float MaxFrameTime;

uint32 curMS;
uint32 msCollected[MAX_MS_COLLECTED];
#ifdef FRAMETIME
float FrameInitTime;
#endif

void tbInit()
{
	TimerBar.count = 0;
	uint32 i = CTimer::GetFrameCounter() & 0x7F;
	if (i == 0) {
		do
			MaxTimes[i++] = 0.0f;
		while (i != MAX_TIMERS);
#ifdef FRAMETIME
		MaxFrameTime = 0.0f;
#endif
	}
#ifdef FRAMETIME
	FrameInitTime = (float)CTimer::GetCurrentTimeInCycles() / (float)CTimer::GetCyclesPerFrame();
#endif
}

void tbStartTimer(int32 unk, char *name)
{
	strcpy(TimerBar.Timers[TimerBar.count].name, name);
	TimerBar.Timers[TimerBar.count].unk = unk;
	TimerBar.Timers[TimerBar.count].startTime = (float)CTimer::GetCurrentTimeInCycles() / (float)CTimer::GetCyclesPerFrame();
	TimerBar.count++;
}

void tbEndTimer(char* name)
{
	uint32 n = 1500;
	for (uint32 i = 0; i < TimerBar.count; i++) {
		if (strcmp(name, TimerBar.Timers[i].name) == 0)
			n = i;
	}
	assert(n != 1500);
	TimerBar.Timers[n].endTime = (float)CTimer::GetCurrentTimeInCycles() / (float)CTimer::GetCyclesPerFrame();
}

float Diag_GetFPS()
{
	return 39000.0f / (msCollected[(curMS - 1) % MAX_MS_COLLECTED] - msCollected[curMS % MAX_MS_COLLECTED]);
}

void tbDisplay()
{
	char temp[200];
	wchar wtemp[200];

#ifdef FRAMETIME
	float FrameEndTime = (float)CTimer::GetCurrentTimeInCycles() / (float)CTimer::GetCyclesPerFrame();
#endif

	msCollected[(curMS++) % MAX_MS_COLLECTED] = RsTimer();
	CFont::SetBackgroundOff();
	CFont::SetBackgroundColor(CRGBA(0, 0, 0, 128));
	CFont::SetScale(0.48f, 1.12f);
	CFont::SetCentreOff();
	CFont::SetJustifyOff();
	CFont::SetWrapx(640.0f);
	CFont::SetRightJustifyOff();
	CFont::SetPropOn();
	CFont::SetFontStyle(FONT_STANDARD);
	sprintf(temp, "FPS: %.2f", Diag_GetFPS());
	AsciiToUnicode(temp, wtemp);
	CFont::SetColor(CRGBA(255, 255, 255, 255));
	if (!CMenuManager::m_PrefsMarketing || !CMenuManager::m_PrefsDisableTutorials) {
		CFont::PrintString(RsGlobal.maximumWidth * (4.0f / DEFAULT_SCREEN_WIDTH), RsGlobal.maximumHeight * (4.0f / DEFAULT_SCREEN_HEIGHT), wtemp);

#ifndef FINAL
		// Timers output (my own implementation)
		for (uint32 i = 0; i < TimerBar.count; i++) {
			MaxTimes[i] = Max(MaxTimes[i], TimerBar.Timers[i].endTime - TimerBar.Timers[i].startTime);
			sprintf(temp, "%s: %.2f", &TimerBar.Timers[i].name[0], MaxTimes[i]);
			AsciiToUnicode(temp, wtemp);
			CFont::PrintString(RsGlobal.maximumWidth * (4.0f / DEFAULT_SCREEN_WIDTH), RsGlobal.maximumHeight * ((8.0f * (i + 2)) / DEFAULT_SCREEN_HEIGHT), wtemp);
		}

#ifdef FRAMETIME
		MaxFrameTime = Max(MaxFrameTime, FrameEndTime - FrameInitTime);
		sprintf(temp, "Frame Time: %.2f", MaxFrameTime);
		AsciiToUnicode(temp, wtemp);

		CFont::PrintString(RsGlobal.maximumWidth * (4.0f / DEFAULT_SCREEN_WIDTH), RsGlobal.maximumHeight * ((8.0f * (TimerBar.count + 4)) / DEFAULT_SCREEN_HEIGHT), wtemp);
#endif // FRAMETIME
#endif // !FINAL
	}
}
#endif // !MASTER