#pragma once

class CGarages
{
public:
	static int32 &BankVansCollected;
	static bool &BombsAreFree;
	static bool &RespraysAreFree;
	static int32 &CarsCollected;
	static int32 &CarTypesCollected;
	static int32 &CrushedCarId;
	static uint32 &LastTimeHelpMessage;
	static int32 &MessageNumberInString;
	static const char *MessageIDString;
	static int32 &MessageNumberInString2;
	static uint32 &MessageStartTime;
	static uint32 &MessageEndTime;
	static uint32 &NumGarages;
	static bool &PlayerInGarage;
	static int32 &PoliceCarsCollected;
	static uint32 &GarageToBeTidied;

public:
	static bool IsModelIndexADoor(uint32 id);
	static void TriggerMessage(const char *text, int16, uint16 time, int16);
	static void PrintMessages(void);
	static bool HasCarBeenCrushed(int32);
	static bool IsPointWithinHideOutGarage(CVector&);
	static bool IsPointWithinAnyGarage(CVector&);
	static void PlayerArrestedOrDied();
	static void Init(void);
	static void Update(void);
};
