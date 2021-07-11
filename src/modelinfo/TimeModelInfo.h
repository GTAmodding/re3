#pragma once

#include "SimpleModelInfo.h"

class CTimeModelInfo : public CSimpleModelInfo
{
	int32 m_timeOn;
	int32 m_timeOff;
	int32 m_otherTimeModelID;
public:
	CTimeModelInfo(void) : CSimpleModelInfo(MITYPE_TIME) { m_otherTimeModelID = -1; }

	int32 GetTimeOn(void) { return m_timeOn; }
	int32 GetTimeOff(void) { return m_timeOff; }
	void SetTimes(int32 on, int32 off) { m_timeOn = on; m_timeOff = off; }
	int32 GetOtherTimeModel(void) { return m_otherTimeModelID; }
	void SetOtherTimeModel(int32 other) { m_otherTimeModelID = other; }
	CTimeModelInfo *FindOtherTimeModel(void);
};
//static_assert(sizeof(CTimeModelInfo) == 0x58, "CTimeModelInfo: error");
