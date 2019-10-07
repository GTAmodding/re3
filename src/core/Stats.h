#pragma once

class CStats
{
public:
	static int32 &DaysPassed;
	static int32 &HeadsPopped;
	static bool& CommercialPassed;
	static bool& IndustrialPassed;
	static int32 &NumberKillFrenziesPassed;
	static int32 &PeopleKilledByOthers;
	static int32 &HelisDestroyed;
	static int32 *PedsKilledOfThisType;	//[NUM_PEDTYPES]
	static int32 &TimesDied;
	static int32 &TimesArrested;
	static int32 &KillsSinceLastCheckpoint;

public:
	static void AnotherKillFrenzyPassed();
	static void CheckPointReachedUnsuccessfully() { KillsSinceLastCheckpoint = 0; };
};