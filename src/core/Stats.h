#pragma once

#include "PedType.h"

class CStats
{
public:
	enum {
		TOTAL_FASTEST_TIMES = 16,
		TOTAL_HIGHEST_SCORES = 16
	};
	static int32 DaysPassed;
	static int32 HeadsPopped;
	static int32 CommercialPassed;
	static int32 IndustrialPassed;
	static int32 SuburbanPassed;
	static int32 NumberKillFrenziesPassed;
	static int32 PeopleKilledByOthers;
	static int32 HelisDestroyed;
	static int32 PedsKilledOfThisType[NUM_PEDTYPES];
	static int32 TimesDied;
	static int32 TimesArrested;
	static int32 KillsSinceLastCheckpoint;
	static float DistanceTravelledInVehicle;
	static float DistanceTravelledOnFoot;
	static int32 CarsExploded;
	static int32 PeopleKilledByPlayer;
	static int32 ProgressMade;
	static int32 TotalProgressInGame;
	static float MaximumJumpDistance;
	static float MaximumJumpHeight;
	static int32 MaximumJumpFlips;
	static int32 MaximumJumpSpins;
	static int32 BestStuntJump;
	static int32 NumberOfUniqueJumpsFound;
	static int32 TotalNumberOfUniqueJumps;
	static int32 PassengersDroppedOffWithTaxi;
	static int32 MoneyMadeWithTaxi;
	static int32 MissionsGiven;
	static int32 MissionsPassed;
	static char  LastMissionPassedName[8];
	static int32 TotalLegitimateKills;
	static int32 ElBurroTime;
	static int32 Record4x4One;
	static int32 Record4x4Two;
	static int32 Record4x4Three;
	static int32 Record4x4Mayhem;
	static int32 LivesSavedWithAmbulance;
	static int32 CriminalsCaught;
	static int32 HighestLevelAmbulanceMission;
	static int32 FiresExtinguished;
	static int32 LongestFlightInDodo;
	static int32 TimeTakenDefuseMission;
	static int32 TotalNumberKillFrenzies;
	static int32 TotalNumberMissions;
	static int32 RoundsFiredByPlayer;
	static int32 KgsOfExplosivesUsed;
	static int32 InstantHitsFiredByPlayer;
	static int32 InstantHitsHitByPlayer;
	static int32 BestTimeBombDefusal;
	static int32 mmRain;
	static int32 CarsCrushed;
	static int32 FastestTimes[TOTAL_FASTEST_TIMES];
	static int32 HighestScores[TOTAL_HIGHEST_SCORES];

public:
	static void Init(void);
	static void RegisterFastestTime(int32, int32);
	static void RegisterHighestScore(int32, int32);
	static void RegisterElBurroTime(int32);
	static void Register4x4OneTime(int32);
	static void Register4x4TwoTime(int32);
	static void Register4x4ThreeTime(int32);
	static void Register4x4MayhemTime(int32);
	static void AnotherLifeSavedWithAmbulance();
	static void AnotherCriminalCaught();
	static void RegisterLevelAmbulanceMission(int32);
	static void AnotherFireExtinguished();
	static wchar *FindCriminalRatingString();
	static void RegisterLongestFlightInDodo(int32);
	static void RegisterTimeTakenDefuseMission(int32);
	static void AnotherKillFrenzyPassed();
	static void SetTotalNumberKillFrenzies(int32);
	static void SetTotalNumberMissions(int32);
	static void CheckPointReachedSuccessfully() { TotalLegitimateKills += KillsSinceLastCheckpoint; KillsSinceLastCheckpoint = 0; };
	static void CheckPointReachedUnsuccessfully() { KillsSinceLastCheckpoint = 0; };
	static int32 FindCriminalRatingNumber();
	static void SaveStats(uint8 *buf, uint32 *size);
	static void LoadStats(uint8 *buf, uint32 size);
};
