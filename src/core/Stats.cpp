#include "common.h"

#include "Stats.h"
#include "Text.h"
#include "World.h"

//TODO
int32 CStats::SeagullsKilled;

int32 CStats::DaysPassed;
int32 CStats::HeadsPopped;
int32 CStats::CommercialPassed;
int32 CStats::IndustrialPassed;
int32 CStats::SuburbanPassed;
int32 CStats::NumberKillFrenziesPassed;
int32 CStats::PeopleKilledByOthers;
int32 CStats::HelisDestroyed;
int32 CStats::PedsKilledOfThisType[NUM_PEDTYPES];
int32 CStats::TimesDied;
int32 CStats::TimesArrested;
int32 CStats::KillsSinceLastCheckpoint;
int32 CStats::DistanceTravelledInVehicle;
int32 CStats::DistanceTravelledOnFoot;
int32 CStats::ProgressMade;
int32 CStats::TotalProgressInGame;
int32 CStats::CarsExploded;
int32 CStats::PeopleKilledByPlayer;
float CStats::MaximumJumpDistance;
float CStats::MaximumJumpHeight;
int32 CStats::MaximumJumpFlips;
int32 CStats::MaximumJumpSpins;
int32 CStats::BestStuntJump;
int32 CStats::NumberOfUniqueJumpsFound;
int32 CStats::TotalNumberOfUniqueJumps;
int32 CStats::PassengersDroppedOffWithTaxi;
int32 CStats::MoneyMadeWithTaxi;
int32 CStats::MissionsGiven;
int32 CStats::MissionsPassed;
char CStats::LastMissionPassedName[8];
int32 CStats::TotalLegitimateKills;
int32 CStats::ElBurroTime;
int32 CStats::Record4x4One;
int32 CStats::Record4x4Two;
int32 CStats::Record4x4Three;
int32 CStats::Record4x4Mayhem;
int32 CStats::LivesSavedWithAmbulance;
int32 CStats::CriminalsCaught;
int32 CStats::HighestLevelAmbulanceMission;
int32 CStats::FiresExtinguished;
int32 CStats::LongestFlightInDodo;
int32 CStats::TimeTakenDefuseMission;
int32 CStats::TotalNumberKillFrenzies;
int32 CStats::TotalNumberMissions;
int32 CStats::RoundsFiredByPlayer;
int32 CStats::KgsOfExplosivesUsed;
int32 CStats::InstantHitsFiredByPlayer;
int32 CStats::InstantHitsHitByPlayer;
int32 CStats::BestTimeBombDefusal;
int32 CStats::mmRain;
int32 CStats::CarsCrushed;
int32 CStats::FastestTimes[CStats::TOTAL_FASTEST_TIMES];
int32 CStats::HighestScores[CStats::TOTAL_HIGHEST_SCORES];
int32 CStats::PropertyDestroyed;

int32 CStats::Sprayings;
float CStats::AutoPaintingBudget;
int32 CStats::NoMoreHurricanes;
float CStats::FashionBudget;
int32 CStats::SafeHouseVisits;

void CStats::Init()
{
	PeopleKilledByOthers = 0;
	PeopleKilledByPlayer = 0;
	RoundsFiredByPlayer = 0;
	CarsExploded = 0;
	HelisDestroyed = 0;
	ProgressMade = 0;
	KgsOfExplosivesUsed = 0;
	InstantHitsFiredByPlayer = 0;
	InstantHitsHitByPlayer = 0;
	CarsCrushed = 0;
	HeadsPopped = 0;
	TimesArrested = 0;
	TimesDied = 0;
	DaysPassed = 0;
	NumberOfUniqueJumpsFound = 0;
	mmRain = 0;
	MaximumJumpFlips = 0;
	MaximumJumpSpins = 0;
	MaximumJumpDistance = 0;
	MaximumJumpHeight = 0;
	BestStuntJump = 0;
	TotalNumberOfUniqueJumps = 0;
	Record4x4One = 0;
	LongestFlightInDodo = 0;
	Record4x4Two = 0;
	PassengersDroppedOffWithTaxi = 0;
	Record4x4Three = 0;
	MoneyMadeWithTaxi = 0;
	Record4x4Mayhem = 0;
	LivesSavedWithAmbulance = 0;
	ElBurroTime = 0;
	CriminalsCaught = 0;
	MissionsGiven = 0;
	HighestLevelAmbulanceMission = 0;
	MissionsPassed = 0;
	FiresExtinguished = 0;
	DistanceTravelledOnFoot = 0;
	TimeTakenDefuseMission = 0;
	NumberKillFrenziesPassed = 0;
	DistanceTravelledInVehicle = 0;
	TotalNumberKillFrenzies = 0;
	TotalNumberMissions = 0;
	KillsSinceLastCheckpoint = 0;
	TotalLegitimateKills = 0;
	for (int i = 0; i < TOTAL_FASTEST_TIMES; i++)
		FastestTimes[i] = 0;
	for (int i = 0; i < TOTAL_HIGHEST_SCORES; i++)
		HighestScores[i] = 0;
	for (int i = 0; i < NUM_PEDTYPES; i++)
		PedsKilledOfThisType[i] = 0;
	IndustrialPassed = 0;
	CommercialPassed = 0;
	SuburbanPassed = 0;

	Sprayings = 0;
	AutoPaintingBudget = 0.0f;
	NoMoreHurricanes = 0;
	SafeHouseVisits = 0;
}

void CStats::RegisterFastestTime(int32 index, int32 time)
{
	assert(index >= 0 && index < TOTAL_FASTEST_TIMES);
	if (FastestTimes[index] == 0)
		FastestTimes[index] = time;
	else
		FastestTimes[index] = Min(FastestTimes[index], time);
}

void CStats::RegisterHighestScore(int32 index, int32 score)
{
	assert(index >= 0 && index < TOTAL_HIGHEST_SCORES);
	HighestScores[index] = Max(HighestScores[index], score);
}

void CStats::RegisterElBurroTime(int32 time)
{
	ElBurroTime = (ElBurroTime && ElBurroTime < time) ? ElBurroTime : time;
}

void CStats::Register4x4OneTime(int32 time)
{
	Record4x4One = (Record4x4One && Record4x4One < time) ? Record4x4One : time;
}

void CStats::Register4x4TwoTime(int32 time)
{
	Record4x4Two = (Record4x4Two && Record4x4Two < time) ? Record4x4Two : time;
}

void CStats::Register4x4ThreeTime(int32 time)
{
	Record4x4Three = (Record4x4Three && Record4x4Three < time) ? Record4x4Three : time;
}

void CStats::Register4x4MayhemTime(int32 time)
{
	Record4x4Mayhem = (Record4x4Mayhem && Record4x4Mayhem < time) ? Record4x4Mayhem : time;
}

void CStats::AnotherLifeSavedWithAmbulance()
{
	++LivesSavedWithAmbulance;
}

void CStats::AnotherCriminalCaught()
{
	++CriminalsCaught;
}

void CStats::RegisterLevelAmbulanceMission(int32 level)
{
	HighestLevelAmbulanceMission = Max(HighestLevelAmbulanceMission, level);
}

void CStats::AnotherFireExtinguished()
{
	++FiresExtinguished;
}

void CStats::RegisterLongestFlightInDodo(int32 time)
{
	LongestFlightInDodo = Max(LongestFlightInDodo, time);
}

void CStats::RegisterTimeTakenDefuseMission(int32 time)
{
	TimeTakenDefuseMission = (TimeTakenDefuseMission && TimeTakenDefuseMission < time) ? TimeTakenDefuseMission : time;
}

void CStats::AnotherKillFrenzyPassed()
{
	++NumberKillFrenziesPassed;
}

void CStats::SetTotalNumberKillFrenzies(int32 total)
{
	TotalNumberKillFrenzies = total;
}

void CStats::SetTotalNumberMissions(int32 total)
{
	TotalNumberMissions = total;
}

wchar *CStats::FindCriminalRatingString()
{
	int rating = FindCriminalRatingNumber();

	if (rating < 10) return TheText.Get("RATNG1");
	if (rating < 25) return TheText.Get("RATNG2");
	if (rating < 70) return TheText.Get("RATNG3");
	if (rating < 150) return TheText.Get("RATNG4");
	if (rating < 250) return TheText.Get("RATNG5");
	if (rating < 450) return TheText.Get("RATNG6");
	if (rating < 700) return TheText.Get("RATNG7");
	if (rating < 1000) return TheText.Get("RATNG8");
	if (rating < 1400) return TheText.Get("RATNG9");
	if (rating < 1900) return TheText.Get("RATNG10");
	if (rating < 2500) return TheText.Get("RATNG11");
	if (rating < 3200) return TheText.Get("RATNG12");
	if (rating < 4000) return TheText.Get("RATNG13");
	if (rating < 5000) return TheText.Get("RATNG14");
	return TheText.Get("RATNG15");
}

int32 CStats::FindCriminalRatingNumber()
{
	int32 rating;

	rating = FiresExtinguished + 10 * HighestLevelAmbulanceMission + CriminalsCaught + LivesSavedWithAmbulance
		+ 30 * HelisDestroyed + TotalLegitimateKills - 3 * TimesArrested - 3 * TimesDied
		+ CWorld::Players[CWorld::PlayerInFocus].m_nMoney / 5000;
	if (rating <= 0) rating = 0;

	if (InstantHitsFiredByPlayer > 100)
		rating += (float)CStats::InstantHitsHitByPlayer / (float)CStats::InstantHitsFiredByPlayer * 500.0f;
	if (TotalProgressInGame)
		rating += (float)CStats::ProgressMade / (float)CStats::TotalProgressInGame * 1000.0f;
	if (!IndustrialPassed && rating >= 3521)
		rating = 3521;
	if (!CommercialPassed && rating >= 4552)
		rating = 4552;
	return rating;
}

float CStats::GetPercentageProgress()
{
	float percentCompleted = (CStats::TotalProgressInGame == 0 ? 0 :
		CStats::ProgressMade * 100.0f / (CGame::nastyGame ? CStats::TotalProgressInGame : CStats::TotalProgressInGame - 1.0f));

	return Min(percentCompleted, 100.0f);
}

void CStats::MoneySpentOnFashion(int32 money)
{
	FashionBudget += money;
}

void CStats::SaveStats(uint8 *buf, uint32 *size)
{
	CheckPointReachedSuccessfully();
	uint8 *buf_start = buf;
	*size = sizeof(PeopleKilledByPlayer) +
		sizeof(PeopleKilledByOthers) +
		sizeof(CarsExploded) +
		sizeof(RoundsFiredByPlayer) +
		sizeof(PedsKilledOfThisType) +
		sizeof(HelisDestroyed) +
		sizeof(ProgressMade) +
		sizeof(TotalProgressInGame) +
		sizeof(KgsOfExplosivesUsed) +
		sizeof(InstantHitsFiredByPlayer) +
		sizeof(InstantHitsHitByPlayer) +
		sizeof(CarsCrushed) +
		sizeof(HeadsPopped) +
		sizeof(TimesArrested) +
		sizeof(TimesDied) +
		sizeof(DaysPassed) +
		sizeof(mmRain) +
		sizeof(MaximumJumpDistance) +
		sizeof(MaximumJumpHeight) +
		sizeof(MaximumJumpFlips) +
		sizeof(MaximumJumpSpins) +
		sizeof(BestStuntJump) +
		sizeof(NumberOfUniqueJumpsFound) +
		sizeof(TotalNumberOfUniqueJumps) +
		sizeof(MissionsGiven) +
		sizeof(MissionsPassed) +
		sizeof(PassengersDroppedOffWithTaxi) +
		sizeof(MoneyMadeWithTaxi) +
		sizeof(IndustrialPassed) +
		sizeof(CommercialPassed) +
		sizeof(SuburbanPassed) +
		sizeof(ElBurroTime) +
		sizeof(DistanceTravelledOnFoot) +
		sizeof(DistanceTravelledInVehicle) +
		sizeof(Record4x4One) +
		sizeof(Record4x4Two) +
		sizeof(Record4x4Three) +
		sizeof(Record4x4Mayhem) +
		sizeof(LivesSavedWithAmbulance) +
		sizeof(CriminalsCaught) +
		sizeof(HighestLevelAmbulanceMission) +
		sizeof(FiresExtinguished) +
		sizeof(LongestFlightInDodo) +
		sizeof(TimeTakenDefuseMission) +
		sizeof(NumberKillFrenziesPassed) +
		sizeof(TotalNumberKillFrenzies) +
		sizeof(TotalNumberMissions) +
		sizeof(FastestTimes) +
		sizeof(HighestScores) +
		sizeof(KillsSinceLastCheckpoint) +
		sizeof(TotalLegitimateKills) +
		sizeof(LastMissionPassedName);

#define CopyToBuf(buf, data) memcpy(buf, &data, sizeof(data)); buf += sizeof(data);
	CopyToBuf(buf, PeopleKilledByPlayer);
	CopyToBuf(buf, PeopleKilledByOthers);
	CopyToBuf(buf, CarsExploded);
	CopyToBuf(buf, RoundsFiredByPlayer);
	CopyToBuf(buf, PedsKilledOfThisType);
	CopyToBuf(buf, HelisDestroyed);
	CopyToBuf(buf, ProgressMade);
	CopyToBuf(buf, TotalProgressInGame);
	CopyToBuf(buf, KgsOfExplosivesUsed);
	CopyToBuf(buf, InstantHitsFiredByPlayer);
	CopyToBuf(buf, InstantHitsHitByPlayer);
	CopyToBuf(buf, CarsCrushed);
	CopyToBuf(buf, HeadsPopped);
	CopyToBuf(buf, TimesArrested);
	CopyToBuf(buf, TimesDied);
	CopyToBuf(buf, DaysPassed);
	CopyToBuf(buf, mmRain);
	CopyToBuf(buf, MaximumJumpDistance);
	CopyToBuf(buf, MaximumJumpHeight);
	CopyToBuf(buf, MaximumJumpFlips);
	CopyToBuf(buf, MaximumJumpSpins);
	CopyToBuf(buf, BestStuntJump);
	CopyToBuf(buf, NumberOfUniqueJumpsFound);
	CopyToBuf(buf, TotalNumberOfUniqueJumps);
	CopyToBuf(buf, MissionsGiven);
	CopyToBuf(buf, MissionsPassed);
	CopyToBuf(buf, PassengersDroppedOffWithTaxi);
	CopyToBuf(buf, MoneyMadeWithTaxi);
	CopyToBuf(buf, IndustrialPassed);
	CopyToBuf(buf, CommercialPassed);
	CopyToBuf(buf, SuburbanPassed);
	CopyToBuf(buf, ElBurroTime);
	CopyToBuf(buf, DistanceTravelledOnFoot);
	CopyToBuf(buf, DistanceTravelledInVehicle);
	CopyToBuf(buf, Record4x4One);
	CopyToBuf(buf, Record4x4Two);
	CopyToBuf(buf, Record4x4Three);
	CopyToBuf(buf, Record4x4Mayhem);
	CopyToBuf(buf, LivesSavedWithAmbulance);
	CopyToBuf(buf, CriminalsCaught);
	CopyToBuf(buf, HighestLevelAmbulanceMission);
	CopyToBuf(buf, FiresExtinguished);
	CopyToBuf(buf, LongestFlightInDodo);
	CopyToBuf(buf, TimeTakenDefuseMission);
	CopyToBuf(buf, NumberKillFrenziesPassed);
	CopyToBuf(buf, TotalNumberKillFrenzies);
	CopyToBuf(buf, TotalNumberMissions);
	CopyToBuf(buf, FastestTimes);
	CopyToBuf(buf, HighestScores);
	CopyToBuf(buf, KillsSinceLastCheckpoint);
	CopyToBuf(buf, TotalLegitimateKills);
	CopyToBuf(buf, LastMissionPassedName);

	assert(buf - buf_start == *size);
#undef CopyToBuf
}

void CStats::LoadStats(uint8 *buf, uint32 size)
{
	uint8* buf_start = buf;

#define CopyFromBuf(buf, data) memcpy(&data, buf, sizeof(data)); buf += sizeof(data);

	CopyFromBuf(buf, PeopleKilledByPlayer);
	CopyFromBuf(buf, PeopleKilledByOthers);
	CopyFromBuf(buf, CarsExploded);
	CopyFromBuf(buf, RoundsFiredByPlayer);
	CopyFromBuf(buf, PedsKilledOfThisType);
	CopyFromBuf(buf, HelisDestroyed);
	CopyFromBuf(buf, ProgressMade);
	CopyFromBuf(buf, TotalProgressInGame);
	CopyFromBuf(buf, KgsOfExplosivesUsed);
	CopyFromBuf(buf, InstantHitsFiredByPlayer);
	CopyFromBuf(buf, InstantHitsHitByPlayer);
	CopyFromBuf(buf, CarsCrushed);
	CopyFromBuf(buf, HeadsPopped);
	CopyFromBuf(buf, TimesArrested);
	CopyFromBuf(buf, TimesDied);
	CopyFromBuf(buf, DaysPassed);
	CopyFromBuf(buf, mmRain);
	CopyFromBuf(buf, MaximumJumpDistance);
	CopyFromBuf(buf, MaximumJumpHeight);
	CopyFromBuf(buf, MaximumJumpFlips);
	CopyFromBuf(buf, MaximumJumpSpins);
	CopyFromBuf(buf, BestStuntJump);
	CopyFromBuf(buf, NumberOfUniqueJumpsFound);
	CopyFromBuf(buf, TotalNumberOfUniqueJumps);
	CopyFromBuf(buf, MissionsGiven);
	CopyFromBuf(buf, MissionsPassed);
	CopyFromBuf(buf, PassengersDroppedOffWithTaxi);
	CopyFromBuf(buf, MoneyMadeWithTaxi);
	CopyFromBuf(buf, IndustrialPassed);
	CopyFromBuf(buf, CommercialPassed);
	CopyFromBuf(buf, SuburbanPassed);
	CopyFromBuf(buf, ElBurroTime);
	CopyFromBuf(buf, DistanceTravelledOnFoot);
	CopyFromBuf(buf, DistanceTravelledInVehicle);
	CopyFromBuf(buf, Record4x4One);
	CopyFromBuf(buf, Record4x4Two);
	CopyFromBuf(buf, Record4x4Three);
	CopyFromBuf(buf, Record4x4Mayhem);
	CopyFromBuf(buf, LivesSavedWithAmbulance);
	CopyFromBuf(buf, CriminalsCaught);
	CopyFromBuf(buf, HighestLevelAmbulanceMission);
	CopyFromBuf(buf, FiresExtinguished);
	CopyFromBuf(buf, LongestFlightInDodo);
	CopyFromBuf(buf, TimeTakenDefuseMission);
	CopyFromBuf(buf, NumberKillFrenziesPassed);
	CopyFromBuf(buf, TotalNumberKillFrenzies);
	CopyFromBuf(buf, TotalNumberMissions);
	CopyFromBuf(buf, FastestTimes);
	CopyFromBuf(buf, HighestScores);
	CopyFromBuf(buf, KillsSinceLastCheckpoint);
	CopyFromBuf(buf, TotalLegitimateKills);
	CopyFromBuf(buf, LastMissionPassedName);

	assert(buf - buf_start == size);
#undef CopyFromBuf
}
