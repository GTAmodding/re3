#pragma once

class CAutomobile;
class CVehicle;
class CControllerState;

class CCarStateEachFrame
{
public:
	int16 velX;
	int16 velY;
	int16 velZ;
	int8 rightX;
	int8 rightY;
	int8 rightZ;
	int8 forwardX;
	int8 forwardY;
	int8 forwardZ;
	int8 wheel;
	int8 gas;
	int8 brake;
	bool handbrake;
	CVector pos;
};

extern char gString[256];

class CRecordDataForChase
{
	enum {
		NUM_CHASE_CARS = 20
	};
	enum {
		STATE_NONE = 0,
		STATE_RECORD = 1,
		STATE_PLAYBACK_INIT = 2,
		STATE_PLAYBACK = 3,
		STATE_PLAYBACK_BEFORE_RECORDING = 4
	};
	static uint8 Status;
	static int PositionChanges;
	static uint8 CurrentCar;
	static CAutomobile*pChaseCars[NUM_CHASE_CARS];
	static float AnimTime;
	static uint32 AnimStartTime;
	static CCarStateEachFrame* pBaseMemForCar[NUM_CHASE_CARS];
	static float TimeMultiplier;
	static int FId2;
public:

	static bool IsRecording(void) { return Status == STATE_RECORD; }

	static void Init(void);
	static void SaveOrRetrieveDataForThisFrame(void);
	static void SaveOrRetrieveCarPositions(void);
	static void StoreInfoForCar(CAutomobile*, CCarStateEachFrame*);
	static void RestoreInfoForMatrix(CMatrix&, CCarStateEachFrame*);
	static void RestoreInfoForCar(CAutomobile*, CCarStateEachFrame*, bool);
	static void ProcessControlCars(void);
	static bool ShouldThisPadBeLeftAlone(uint8 pad);
	static void GiveUsACar(int32, CVector, float, CAutomobile**, uint8, uint8);
	static void StartChaseScene(float);
	static void CleanUpChaseScene(void);
	static void SetUpCarsForChaseScene(void);
	static void CleanUpCarsForChaseScene(void);
	static void RemoveCarFromChase(int32);
	static CVehicle* TurnChaseCarIntoScriptCar(int32);

};

struct tGameBuffer
{
	float m_fTimeStep;
	uint32 m_nTimeInMilliseconds;
	uint8 m_nSizeOfPads[2];
	uint16 m_nChecksum;
	uint8 m_ControllerBuffer[116];
};

class CRecordDataForGame
{
	enum {
		STATE_NONE = 0,
		STATE_RECORD = 1,
		STATE_PLAYBACK = 2,
	};
	static uint16 RecordingState;
	static uint8* pDataBuffer;
	static uint8* pDataBufferPointer;
	static int FId;
	static tGameBuffer pDataBufferForFrame;

public:
	static bool IsRecording() { return RecordingState == STATE_RECORD; }
	static bool IsPlayingBack() { return RecordingState == STATE_PLAYBACK; }

	static void SaveOrRetrieveDataForThisFrame(void);
	static void Init(void);

private:
	static uint16 CalcGameChecksum(void);
	static uint8* PackCurrentPadValues(uint8*, CControllerState*, CControllerState*);
	static uint8* UnPackCurrentPadValues(uint8*, uint8, CControllerState*);
};
