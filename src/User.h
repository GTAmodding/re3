#pragma once

class COnscreenTimerEntry {
public:
	uint32 m_nTimerOffset;
	uint32 m_nCounterOffset;
	uint8 m_aTimerText[10];
	uint8 m_aCounterText[10];
	uint16 m_nType;
	char m_bCounterBuffer[42];
	char m_bTimerBuffer[42];
	bool m_bTimerProcessed;
	bool m_bCounterProcessed;
};

class COnscreenTimer
{
public:
	COnscreenTimerEntry m_sEntries[1];
	char m_bProcessed;
	char m_bDisabled;
	char field_119[2];

	void ProcessForDisplay(void);
};

class CPlaceName
{
};

class CCurrentVehicle
{
};

class CPager
{
};

class CUserDisplay
{
public:
	static CPlaceName &PlaceName;
	static COnscreenTimer &OnscnTimer;
	static CPager &Pager;
	static CCurrentVehicle &CurrentVehicle;
};
