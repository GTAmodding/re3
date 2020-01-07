#pragma once

enum {
	RECORDSTATE_0,
	RECORDSTATE_1,
	RECORDSTATE_2,
};

class CRecordDataForChase
{
public:
	static uint8 &Status;

	static void SaveOrRetrieveDataForThisFrame(void);
	static void ProcessControlCars(void);
	static void SaveOrRetrieveCarPositions(void);
	static void StartChaseScene(float);
	static void CleanUpChaseScene();
};


class CRecordDataForGame
{
public:
	static uint16 &RecordingState;

	static void SaveOrRetrieveDataForThisFrame(void);
};
