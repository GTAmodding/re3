#include "common.h"
#include "Stats.h"

int32 &CStats::DaysPassed = *(int32*)0x8F2BB8;
int32 &CStats::HeadsPopped = *(int32*)0x8F647C;
bool& CStats::CommercialPassed = *(bool*)0x8F4334;
bool& CStats::IndustrialPassed = *(bool*)0x8E2A68;
bool& CStats::SuburbanPassed = *(bool*)0x8F2740;
int32 &CStats::NumberKillFrenziesPassed = *(int32*)0x8E287C;
int32 &CStats::PeopleKilledByOthers = *(int32*)0x8E2C50;
int32 &CStats::HelisDestroyed = *(int32*)0x8E2A64;
int32 *CStats::PedsKilledOfThisType = (int32*)0x880DBC;
int32 &CStats::TimesDied = *(int32*)0x8E2BDC;
int32 &CStats::TimesArrested = *(int32*)0x8E2BEC;
int32 &CStats::KillsSinceLastCheckpoint = *(int32*)0x8F2C8C;
int32& CStats::DistanceTravelledInVehicle = *(int32*)0x940574;
int32& CStats::DistanceTravelledOnFoot = *(int32*)0x941518;
int32 &CStats::ProgressMade = *(int32*)0x8F6224;
int32 &CStats::TotalProgressInGame = *(int32*)0x885B2C;
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

void CStats::AnotherKillFrenzyPassed()
{
	++NumberKillFrenziesPassed;
}
