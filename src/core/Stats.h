#pragma once

class CStats
{
public:
	static int32 &DaysPassed;
	static int32 &HeadsPopped;
	static bool& CommercialPassed;
	static bool& IndustrialPassed;
	static bool& SuburbanPassed;
	static int32 &NumberKillFrenziesPassed;
	static int32 &PeopleKilledByOthers;
	static int32 &HelisDestroyed;
	static int32 *PedsKilledOfThisType;	//[NUM_PEDTYPES]
	static int32 &TimesDied;
	static int32 &TimesArrested;
	static int32 &KillsSinceLastCheckpoint;
	static int32 &DistanceTravelledInVehicle;
	static int32 &DistanceTravelledOnFoot;
	static int32 &ProgressMade;
	static int32 &TotalProgressInGame;
	static float &MaximumJumpDistance;
	static float &MaximumJumpHeight;
	static int32 &MaximumJumpFlips;
	static int32 &MaximumJumpSpins;
	static int32 &BestStuntJump;
	static int32 &NumberOfUniqueJumpsFound;
	static int32 &TotalNumberOfUniqueJumps;
	static int32 &PassengersDroppedOffWithTaxi;
	static int32 &MoneyMadeWithTaxi;
	static int32 &MissionsGiven;
	static int32 &MissionsPassed;
	static char (&LastMissionPassedName)[8];
	static int32 &TotalLegitimateKills;

public:
	static void AnotherKillFrenzyPassed();
	static void CheckPointReachedUnsuccessfully() { KillsSinceLastCheckpoint = 0; };
	static void CheckPointReachedSuccessfully() { TotalLegitimateKills += KillsSinceLastCheckpoint; KillsSinceLastCheckpoint = 0; };
};