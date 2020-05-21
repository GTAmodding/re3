#pragma once

enum
{
	COUNTER_DISPLAY_NUMBER,
	COUNTER_DISPLAY_BAR,
};

class COnscreenTimerEntry
{
public:
	uint32 m_nTimerOffset;
	uint32 m_nCounterOffset;
	char m_aTimerText[10];
	char m_aCounterText[10];
	uint16 m_nType;
	char m_bCounterBuffer[42];
	char m_bTimerBuffer[42];
	bool m_bTimerProcessed;
	bool m_bTimerGoingDown;
	bool m_bCounterProcessed;

	void Process();
	bool ProcessForDisplay();

	void ProcessForDisplayClock();
	void ProcessForDisplayCounter();
};

VALIDATE_SIZE(COnscreenTimerEntry, 0x74);

class COnscreenTimer
{
public:
	COnscreenTimerEntry m_sEntries[NUMONSCREENTIMERENTRIES];
	bool m_bProcessed;
	bool m_bDisabled;

	void Init();
	void Process();
	void ProcessForDisplay();

	void ClearCounter(uint32 offset);
	void ClearClock(uint32 offset);

	void AddCounter(uint32 offset, uint16 type, char* text, uint16 pos);
	void AddClock(uint32 offset, char* text, bool bGoingDown);
};

VALIDATE_SIZE(COnscreenTimer, 0x78);
