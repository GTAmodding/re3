#pragma once

#include "Wanted.h"

struct cAMCrime {
	int32 type;
	CVector position;
	uint16 timer;

	cAMCrime();
};

VALIDATE_SIZE(cAMCrime, 20);

class cPoliceRadioQueue
{
public:
	int32 crimesSamples[60];
	uint8 policeChannelTimer;
	uint8 policeChannelTimerSeconds;
	uint8 policeChannelCounterSeconds;
	cAMCrime crimes[10];

	cPoliceRadioQueue();

	void Add(uint32 sample);
};

VALIDATE_SIZE(cPoliceRadioQueue, 444);
