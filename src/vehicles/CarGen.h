#pragma once
#include "config.h"

enum {
	CARGEN_MAXACTUALLIMIT = 100
};

class CCarGenerator
{
	int32 m_nModelIndex;
	CVector m_vecPos;
	float m_fAngle;
	int16 m_nColor1;
	int16 m_nColor2;
	uint8 m_bForceSpawn;
	uint8 m_nAlarm;
	uint8 m_nDoorlock;
	int16 m_nMinDelay;
	int16 m_nMaxDelay;
	uint32 m_nTimer;
	int32 m_nVehicleHandle;
	uint16 m_nUsesRemaining;
	bool m_bIsBlocking;
public:
	void SwitchOff();
	void SwitchOn();
	uint32 CalcNextGen();
	void DoInternalProcessing();
	void Process();
	void Setup(float x, float y, float z, float angle, int32 mi, int16 color1, int16 color2, uint8 force, uint8 alarm, uint8 lock, uint16 min_delay, uint16 max_delay);
	bool CheckForBlockage(int32 mi);
	bool CheckIfWithinRangeOfAnyPlayers();
	void SetUsesRemaining(uint16 uses) { m_nUsesRemaining = uses; }
};

class CTheCarGenerators
{
public:
	static uint8 ProcessCounter;
	static uint32 NumOfCarGenerators;
	static CCarGenerator CarGeneratorArray[NUM_CARGENS];
	static uint8 GenerateEvenIfPlayerIsCloseCounter;
	static uint32 CurrentActiveCount;

	static void Process();
	static int32 CreateCarGenerator(float x, float y, float z, float angle, int32 mi, int16 color1, int16 color2, uint8 force, uint8 alarm, uint8 lock, uint16 min_delay, uint16 max_delay);
	static void Init();
	static void SaveAllCarGenerators(uint8 *, uint32 *);
	static void LoadAllCarGenerators(uint8 *, uint32);
};
