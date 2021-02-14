#include "common.h"

#include "Timer.h"
#include "HandlingMgr.h"
#include "Transmission.h"

void
cTransmission::InitGearRatios(void)
{
	static tGear *pGearRatio0 = nil;
	static tGear *pGearRatio1 = nil;
	int i;
	float velocityDiff;

	memset(Gears, 0, sizeof(Gears));

	for(i = 1; i <= nNumberOfGears; i++){
		pGearRatio0 = &Gears[i-1];
		pGearRatio1 = &Gears[i];

		pGearRatio1->fMaxVelocity = (float)i / nNumberOfGears * fMaxVelocity;

		velocityDiff = pGearRatio1->fMaxVelocity - pGearRatio0->fMaxVelocity;

		if(i >= nNumberOfGears){
			pGearRatio1->fShiftUpVelocity = fMaxVelocity;
		}else{
			Gears[i+1].fShiftDownVelocity = velocityDiff*0.42f + pGearRatio0->fMaxVelocity;
			pGearRatio1->fShiftUpVelocity = velocityDiff*0.6667f + pGearRatio0->fMaxVelocity;
		}
	}

	// Reverse gear
	Gears[0].fMaxVelocity = fMaxReverseVelocity;
	Gears[0].fShiftUpVelocity = -0.01f;
	Gears[0].fShiftDownVelocity = fMaxReverseVelocity;

	Gears[1].fShiftDownVelocity = -0.01f;
}

void
cTransmission::CalculateGearForSimpleCar(float speed, uint8 &gear)
{
	static tGear *pGearRatio;

	pGearRatio = &Gears[gear];
	fCurVelocity = speed;
	if(speed > pGearRatio->fShiftUpVelocity)
		gear++;
	else if(speed < pGearRatio->fShiftDownVelocity){
		if(gear - 1 < 0)
			gear = 0;
		else
			gear--;
	}
}

float
cTransmission::CalculateDriveAcceleration(const float &gasPedal, uint8 &gear, float &time, const float &velocity, bool cheat)
{
	static float fAcceleration = 0.0f;
	static float fVelocity;
	static float fCheat;
	static tGear *pGearRatio;

	fVelocity = velocity;
	if(fVelocity < fMaxReverseVelocity){
		fVelocity = fMaxReverseVelocity;
		return 0.0f;
	}
	if(fVelocity > fMaxVelocity){
		fVelocity = fMaxVelocity;
		return 0.0f;
	}
	fCurVelocity = fVelocity;

	assert(gear <= nNumberOfGears);

	pGearRatio = &Gears[gear];
	if(fVelocity > pGearRatio->fShiftUpVelocity){
		if(gear != 0 || gasPedal > 0.0f){
			gear++;
			time = 0.0f;
			return CalculateDriveAcceleration(gasPedal, gear, time, fVelocity, false);
		}
	}else if(fVelocity < pGearRatio->fShiftDownVelocity && gear != 0){
		if(gear != 1 || gasPedal < 0.0f){
			gear--;
			time = 0.0f;
			return CalculateDriveAcceleration(gasPedal, gear, time, fVelocity, false);
		}
	}

	if(time > 0.0f){
		// changing gears currently, can't accelerate
		fAcceleration = 0.0f;
		time -= CTimer::GetTimeStepInSeconds();
	}else{
		float speedMul, accelMul;

		if(gear < 1){
			// going reverse
			accelMul = (Flags & HANDLING_2G_BOOST) ? 2.0f : 1.0f;
			speedMul = -1.0f;
		}else if(nNumberOfGears == 1){
			accelMul = 1.0f;
			speedMul = 1.0f;
		}else{
			// BUG or not? this is 1.0 normally but 0.0 in the highest gear
			float f = 1.0f - (gear-1)/(nNumberOfGears-1);
			speedMul = 3.0f*sq(f) + 1.0f;
			// This is pretty ugly, could be written more clearly
			if(Flags & HANDLING_2G_BOOST){
				if(gear == 1)
					accelMul = (Flags & HANDLING_1G_BOOST) ? 3.0f : 2.0f;
				else if(gear == 2)
					accelMul = 1.3f;
				else
					accelMul = 1.0f;
			}else if(Flags & HANDLING_1G_BOOST && gear == 1){
				accelMul = 3.0f;
			}else
				accelMul = 1.0f;
		}

		if(cheat)
			fCheat = 1.2f;
		else
			fCheat = 1.0f;
		float targetVelocity = Gears[gear].fMaxVelocity*speedMul*fCheat;
		float accel = (targetVelocity - fVelocity) * (fEngineAcceleration*accelMul) / Abs(targetVelocity);
		if(Abs(fVelocity) < Abs(Gears[gear].fMaxVelocity*fCheat))
			fAcceleration = gasPedal * accel * CTimer::GetTimeStep();
		else
			fAcceleration = 0.0f;
	}
	return fAcceleration;
}
