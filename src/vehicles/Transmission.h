#pragma once

struct tGear
{
	float fMaxVelocity;
	float fShiftUpVelocity;
	float fShiftDownVelocity;
};

class cTransmission
{
public:
	// Gear 0 is reverse, 1-5 are forward
	tGear Gears[6];
	char nDriveType;
	char nEngineType;
	int8 nNumberOfGears;
	uint8 Flags;
	float fEngineAcceleration;
	float fMaxVelocity;
	float fMaxCruiseVelocity;
	float fMaxReverseVelocity;
	float fCurVelocity;

	void InitGearRatios(void);
	void CalculateGearForSimpleCar(float speed, uint8 &gear);
	float CalculateDriveAcceleration(const float &gasPedal, uint8 &gear, float &time, const float &velocity, bool cheat);
};
