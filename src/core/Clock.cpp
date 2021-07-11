#include "common.h"

#include "Timer.h"
#include "Pad.h"
#include "Clock.h"
#include "Stats.h"

_TODO("gbFastTime");
bool gbFastTime;

uint8  CClock::ms_nGameClockHours;
uint8  CClock::ms_nGameClockMinutes;
uint16 CClock::ms_nGameClockSeconds;
uint8  CClock::ms_Stored_nGameClockHours;
uint8  CClock::ms_Stored_nGameClockMinutes;
uint16 CClock::ms_Stored_nGameClockSeconds;
uint32 CClock::ms_nMillisecondsPerGameMinute;
uint32  CClock::ms_nLastClockTick;
bool   CClock::ms_bClockHasBeenStored;

void
CClock::Initialise(uint32 scale)
{
	debug("Initialising CClock...\n");
	ms_nGameClockHours = 12;
	ms_nGameClockMinutes = 0;
	ms_nGameClockSeconds = 0;
	ms_nMillisecondsPerGameMinute = scale;
	ms_nLastClockTick = CTimer::GetTimeInMilliseconds();
	ms_bClockHasBeenStored = false;
	debug("CClock ready\n");
}

void
CClock::Update(void)
{
	if(CPad::GetPad(1)->GetRightShoulder1())
	{
		ms_nGameClockMinutes += 8;
		ms_nLastClockTick = CTimer::GetTimeInMilliseconds();
		
		if(ms_nGameClockMinutes >= 60)
		{
			ms_nGameClockHours++;
			ms_nGameClockMinutes = 0;
			if(ms_nGameClockHours >= 24)
				ms_nGameClockHours = 0;
		}
		
	}
	else if(CTimer::GetTimeInMilliseconds() - ms_nLastClockTick > ms_nMillisecondsPerGameMinute || gbFastTime)
	{
		ms_nGameClockMinutes++;
		ms_nLastClockTick += ms_nMillisecondsPerGameMinute;
		
		if ( gbFastTime )
			ms_nLastClockTick = CTimer::GetTimeInMilliseconds();
		
		if(ms_nGameClockMinutes >= 60)
		{
			ms_nGameClockHours++;
			ms_nGameClockMinutes = 0;
			if(ms_nGameClockHours >= 24)
			{
				CStats::DaysPassed++;
				ms_nGameClockHours = 0;
			}
		}
	}
	ms_nGameClockSeconds = 60 * (CTimer::GetTimeInMilliseconds() - ms_nLastClockTick) / ms_nMillisecondsPerGameMinute;
}

void
CClock::SetGameClock(uint8 h, uint8 m)
{
	while (m >= 60) {
		m -= 60;
		h++;
	}
	ms_nGameClockMinutes = m;
	while (h >= 24)
		h -= 24;
	ms_nGameClockHours = h;
	ms_nGameClockSeconds = 0;
	ms_nLastClockTick = CTimer::GetTimeInMilliseconds();
}

int32
CClock::GetGameClockMinutesUntil(uint8 h, uint8 m)
{
	int32 now, then;
	now = ms_nGameClockHours*60 + ms_nGameClockMinutes;
	then = h*60 + m;
	if(then < now)
		then += 24*60;
	return then-now;
}

bool
CClock::GetIsTimeInRange(uint8 h1, uint8 h2)
{
	if(h1 > h2)
		return ms_nGameClockHours >= h1 || ms_nGameClockHours < h2;
	else
		return ms_nGameClockHours >= h1 && ms_nGameClockHours < h2;
}

void
CClock::StoreClock(void)
{
	ms_Stored_nGameClockHours = ms_nGameClockHours;
	ms_Stored_nGameClockMinutes = ms_nGameClockMinutes;
	ms_Stored_nGameClockSeconds = ms_nGameClockSeconds;
	ms_bClockHasBeenStored = true;
}

void
CClock::RestoreClock(void)
{
	ms_nGameClockHours = ms_Stored_nGameClockHours;
	ms_nGameClockMinutes = ms_Stored_nGameClockMinutes;
	ms_nGameClockSeconds = ms_Stored_nGameClockSeconds;
}
