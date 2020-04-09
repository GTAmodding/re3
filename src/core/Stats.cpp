#include "common.h"
#include "patcher.h"
#include "Stats.h"
#include "Text.h"
#include "World.h"

int32 &CStats::DaysPassed = *(int32*)0x8F2BB8;
int32 &CStats::HeadsPopped = *(int32*)0x8F647C;
int32& CStats::CommercialPassed = *(int32*)0x8F4334;
int32& CStats::IndustrialPassed = *(int32*)0x8E2A68;
int32& CStats::SuburbanPassed = *(int32*)0x8F2740;
int32 &CStats::NumberKillFrenziesPassed = *(int32*)0x8E287C;
int32 &CStats::PeopleKilledByOthers = *(int32*)0x8E2C50;
int32 &CStats::HelisDestroyed = *(int32*)0x8E2A64;
int32(&CStats::PedsKilledOfThisType)[NUM_PEDTYPES] = *(int32(*)[NUM_PEDTYPES]) * (uintptr*)0x880DBC;
int32 &CStats::TimesDied = *(int32*)0x8E2BDC;
int32 &CStats::TimesArrested = *(int32*)0x8E2BEC;
int32 &CStats::KillsSinceLastCheckpoint = *(int32*)0x8F2C8C;
int32& CStats::DistanceTravelledInVehicle = *(int32*)0x940574;
int32& CStats::DistanceTravelledOnFoot = *(int32*)0x941518;
int32 &CStats::ProgressMade = *(int32*)0x8F6224;
int32 &CStats::TotalProgressInGame = *(int32*)0x885B2C;
int32& CStats::CarsExploded = *(int32*)0x941288;
int32& CStats::PeopleKilledByPlayer = *(int32*)0x8F1B7C;
float &CStats::MaximumJumpDistance = *(float*)0x8F2BDC;
float &CStats::MaximumJumpHeight = *(float*)0x940564;
int32 &CStats::MaximumJumpFlips = *(int32*)0x8F2524;
int32 &CStats::MaximumJumpSpins = *(int32*)0x8F29B0;
int32 &CStats::BestStuntJump = *(int32*)0x885B50;
int32 &CStats::NumberOfUniqueJumpsFound = *(int32*)0x885B74;
int32 &CStats::TotalNumberOfUniqueJumps = *(int32*)0x8E2DC0;
int32 &CStats::PassengersDroppedOffWithTaxi = *(int32*)0x940724;
int32 &CStats::MoneyMadeWithTaxi = *(int32*)0x941544;
int32 &CStats::MissionsGiven = *(int32*)0x9430E8;
int32 &CStats::MissionsPassed = *(int32*)0x940768;
char(&CStats::LastMissionPassedName)[8] = *(char(*)[8])*(uintptr*)0x70D828;
int32 &CStats::TotalLegitimateKills = *(int32*)0x8F6004;
int32 &CStats::ElBurroTime = *(int32*)0x8E2A6C;
int32& CStats::Record4x4One = *(int32*)0x940570;
int32& CStats::Record4x4Two = *(int32*)0x94058C;
int32& CStats::Record4x4Three = *(int32*)0x880FA8;
int32& CStats::Record4x4Mayhem = *(int32*)0x885B70;
int32& CStats::LivesSavedWithAmbulance = *(int32*)0x8F57E0;
int32& CStats::CriminalsCaught = *(int32*)0x8F2518;
int32& CStats::HighestLevelAmbulanceMission = *(int32*)0x8F2A04;
int32& CStats::FiresExtinguished = *(int32*)0x8F5FEC;
int32& CStats::LongestFlightInDodo = *(int32*)0x8F5FE4;
int32& CStats::TimeTakenDefuseMission = *(int32*)0x880E24;
int32& CStats::TotalNumberKillFrenzies = *(int32*)0x8E2884;
int32& CStats::TotalNumberMissions = *(int32*)0x8E2820;
int32& CStats::RoundsFiredByPlayer = *(int32*)0x8E2BE8;
int32& CStats::KgsOfExplosivesUsed = *(int32*)0x8F2510;
int32& CStats::InstantHitsFiredByPlayer = *(int32*)0x943070;
int32& CStats::InstantHitsHitByPlayer = *(int32*)0x95CB8C;
int32& CStats::BestTimeBombDefusal = *(int32*)0x880E24;
int32& CStats::mmRain = *(int32*)0x8F2C98;
int32& CStats::CarsCrushed = *(int32*)0x943050;
int32(&CStats::FastestTimes)[CStats::TOTAL_FASTEST_TIMES] = *(int32(*)[CStats::TOTAL_FASTEST_TIMES])*(uintptr*)0x6E9128;
int32(&CStats::HighestScores)[CStats::TOTAL_HIGHEST_SCORES] = *(int32(*)[CStats::TOTAL_HIGHEST_SCORES]) * (uintptr*)0x8622B0;

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
}

void CStats::RegisterFastestTime(int32 index, int32 time)
{
	assert(index >= 0 && index < TOTAL_FASTEST_TIMES);
	if (FastestTimes[index] == 0)
		FastestTimes[index] = time;
	else
		FastestTimes[index] = min(FastestTimes[index], time);
}

void CStats::RegisterHighestScore(int32 index, int32 score)
{
	assert(index >= 0 && index < TOTAL_HIGHEST_SCORES);
	HighestScores[index] = max(HighestScores[index], score);
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
	HighestLevelAmbulanceMission = max(HighestLevelAmbulanceMission, level);
}

void CStats::AnotherFireExtinguished()
{
	++FiresExtinguished;
}

void CStats::RegisterLongestFlightInDodo(int32 time)
{
	LongestFlightInDodo = max(LongestFlightInDodo, time);
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

STARTPATCHES
	InjectHook(0x48C5A3, CStats::Init, PATCH_JUMP); // CGame::ReInitGameObjectVariables
	InjectHook(0x4AB3E0, CStats::SaveStats, PATCH_JUMP);
	InjectHook(0x4AB670, CStats::LoadStats, PATCH_JUMP);
	InjectHook(0x4AB090, CStats::FindCriminalRatingString, PATCH_JUMP);
	InjectHook(0x4AB2A0, CStats::FindCriminalRatingNumber, PATCH_JUMP);
ENDPATCHES