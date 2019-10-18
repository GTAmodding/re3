#pragma once

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
	bool m_bCounterProcessed;

	void Process();
	bool ProcessForDisplay();

	int ProcessForDisplayTimer();
	int ProcessForDisplayCounter();
};

static_assert(sizeof(COnscreenTimerEntry) == 0x74, "COnscreenTimerEntry: error");

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

	void AddCounter(uint32 offset, uint16 type, char* text);
	void AddClock(uint32 offset, char* text);
};

static_assert(sizeof(COnscreenTimer) == 0x78, "COnscreenTimer: error");

class CPlaceName
{
};

class CCurrentVehicle
{
};

class CPager
{
public:
	void AddMessage(wchar*, uint16, uint16, uint16);
};

class CUserDisplay
{
public:
	static CPlaceName &PlaceName;
	static COnscreenTimer &OnscnTimer;
	static CPager &Pager;
	static CCurrentVehicle &CurrentVehicle;

	static void Process(void);
};
