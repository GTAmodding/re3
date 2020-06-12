#pragma once
#include "Ped.h"

enum eCopType
{
	COP_STREET = 0,
	COP_FBI = 1,
	COP_SWAT = 2,
	COP_ARMY = 3,
	COP_MIAMIVICE = 5
};

class CCopPed : public CPed
{
public:
	int16 m_nRoadblockNode;
	float m_fDistanceToTarget;
	bool m_bIsInPursuit;
	bool m_bIsDisabledCop;
	int8 field_5FE;
	int8 field_5FF;
	bool m_bBeatingSuspect;
	bool m_bStopAndShootDisabledZone;
	bool field_601; // set when police dragging player from car
	bool m_bZoneDisabled;
	float m_fAbseilPos;
	eCopType m_nCopType;
	int8 field_628;

	CCopPed(eCopType, int32 modifier = 0);
	~CCopPed();

	void ClearPursuit(void);
	void ProcessControl(void);
	void SetArrestPlayer(CPed*);
	void SetPursuit(bool);
	void ArrestPlayer(void);
	void ScanForCrimes(void);
	void CopAI(void);
};

#ifndef PED_SKIN
VALIDATE_SIZE(CCopPed, 0x558);
#endif
