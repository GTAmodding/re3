#include "common.h"

#include "Restart.h"
#include "Zones.h"
#include "PathFind.h"
#include "SaveBuf.h"

uint8 CRestart::OverrideHospitalLevel;
uint8 CRestart::OverridePoliceStationLevel;
bool CRestart::bFadeInAfterNextArrest;
bool CRestart::bFadeInAfterNextDeath;

bool CRestart::bOverrideRestart;
CVector CRestart::OverridePosition;
float CRestart::OverrideHeading;

CVector CRestart::HospitalRestartPoints[NUM_RESTART_POINTS];
float CRestart::HospitalRestartHeadings[NUM_RESTART_POINTS];
uint16 CRestart::NumberOfHospitalRestarts;

CVector CRestart::PoliceRestartPoints[NUM_RESTART_POINTS];
float CRestart::PoliceRestartHeadings[NUM_RESTART_POINTS];
uint16 CRestart::NumberOfPoliceRestarts;

void
CRestart::Initialise()
{
	OverridePoliceStationLevel = LEVEL_GENERIC;
	OverrideHospitalLevel = LEVEL_GENERIC;
	bFadeInAfterNextArrest = true;
	bFadeInAfterNextDeath = true;
	OverrideHeading = 0.0f;
	OverridePosition = CVector(0.0f, 0.0f, 0.0f);
	bOverrideRestart = false;
	NumberOfPoliceRestarts = 0;
	NumberOfHospitalRestarts = 0;

	for (int i = 0; i < NUM_RESTART_POINTS; i++) {
		HospitalRestartPoints[i] = CVector(0.0f, 0.0f, 0.0f);
		HospitalRestartHeadings[i] = 0.0f;
		PoliceRestartPoints[i] = CVector(0.0f, 0.0f, 0.0f);
		PoliceRestartHeadings[i] = 0.0f;
	}
}

void
CRestart::AddHospitalRestartPoint(const CVector &pos, float heading)
{
	HospitalRestartPoints[NumberOfHospitalRestarts] = pos;
	HospitalRestartHeadings[NumberOfHospitalRestarts++] = heading;
}

void
CRestart::AddPoliceRestartPoint(const CVector &pos, float heading)
{
	PoliceRestartPoints[NumberOfPoliceRestarts] = pos;
	PoliceRestartHeadings[NumberOfPoliceRestarts++] = heading;
}

void
CRestart::OverrideNextRestart(const CVector &pos, float heading)
{
	bOverrideRestart = true;
	OverridePosition = pos;
	OverrideHeading = heading;
}

void
CRestart::CancelOverrideRestart()
{
	bOverrideRestart = false;
}

void
CRestart::FindClosestHospitalRestartPoint(const CVector &pos, CVector *outPos, float *outHeading)
{
	if (bOverrideRestart) {
		*outPos = OverridePosition;
		*outHeading = OverrideHeading;
		CancelOverrideRestart();
		return;
	}

	eLevelName curlevel = CTheZones::GetLevelFromPosition(&pos);
	float fMinDist = SQR(4000.0f);
	int closestPoint = NUM_RESTART_POINTS;

	// find closest point on this level
	for (int i = 0; i < NumberOfHospitalRestarts; i++) {
		if (CTheZones::GetLevelFromPosition(&HospitalRestartPoints[i]) == (OverrideHospitalLevel != LEVEL_GENERIC ? OverrideHospitalLevel : curlevel)) {
			float dist = (pos - HospitalRestartPoints[i]).MagnitudeSqr();
			if (fMinDist >= dist) {
				fMinDist = dist;
				closestPoint = i;
			}
		}
	}

	// if we didn't find anything, find closest point on any level
	if (closestPoint == NUM_RESTART_POINTS) {
		for (int i = 0; i < NumberOfHospitalRestarts; i++) {
			float dist = (pos - HospitalRestartPoints[i]).MagnitudeSqr();
			if (fMinDist >= dist) {
				fMinDist = dist;
				closestPoint = i;
			}
		}
	}

	// if we still didn't find anything, find closest path node
	if (closestPoint == NUM_RESTART_POINTS) {
		*outPos = ThePaths.m_pathNodes[ThePaths.FindNodeClosestToCoors(pos, PATH_PED, 999999.9f)].GetPosition();
		*outHeading = 0.0f;
		printf("Couldn't find a hospital restart zone near the player %f %f %f->%f %f %f\n", pos.x, pos.y, pos.z, outPos->x, outPos->y, outPos->z);
	} else {
		*outPos = HospitalRestartPoints[closestPoint];
		*outHeading = HospitalRestartHeadings[closestPoint];
	}
}

void
CRestart::FindClosestPoliceRestartPoint(const CVector &pos, CVector *outPos, float *outHeading)
{
	if (bOverrideRestart) {
		*outPos = OverridePosition;
		*outHeading = OverrideHeading;
		CancelOverrideRestart();
		return;
	}

	eLevelName curlevel = CTheZones::GetLevelFromPosition(&pos);
	float fMinDist = SQR(4000.0f);
	int closestPoint = NUM_RESTART_POINTS;

	// find closest point on this level
	for (int i = 0; i < NumberOfPoliceRestarts; i++) {
		if (CTheZones::GetLevelFromPosition(&PoliceRestartPoints[i]) == (OverridePoliceStationLevel != LEVEL_GENERIC ? OverridePoliceStationLevel : curlevel)) {
			float dist = (pos - PoliceRestartPoints[i]).MagnitudeSqr();
			if (fMinDist >= dist) {
				fMinDist = dist;
				closestPoint = i;
			}
		}
	}

	// if we didn't find anything, find closest point on any level
	if (closestPoint == NUM_RESTART_POINTS) {
		for (int i = 0; i < NumberOfPoliceRestarts; i++) {
			float dist = (pos - PoliceRestartPoints[i]).MagnitudeSqr();
			if (fMinDist >= dist) {
				fMinDist = dist;
				closestPoint = i;
			}
		}
	}

	// if we still didn't find anything, find closest path node
	if (closestPoint == NUM_RESTART_POINTS) {
		printf("Couldn't find a police restart zone near the player\n");
		*outPos = ThePaths.m_pathNodes[ThePaths.FindNodeClosestToCoors(pos, PATH_PED, 999999.9f)].GetPosition();
		*outHeading = 0.0f;
	} else {
		*outPos = PoliceRestartPoints[closestPoint];
		*outHeading = PoliceRestartHeadings[closestPoint];
	}
}

void
CRestart::LoadAllRestartPoints(uint8 *buf, uint32 size)
{
	Initialise();

INITSAVEBUF
	CheckSaveHeader(buf, 'R','S','T','\0', size - SAVE_HEADER_SIZE);

	for (int i = 0; i < NUM_RESTART_POINTS; i++) {
		ReadSaveBuf(&HospitalRestartPoints[i], buf);
		ReadSaveBuf(&HospitalRestartHeadings[i], buf);
	}

	for (int i = 0; i < NUM_RESTART_POINTS; i++) {
		ReadSaveBuf(&PoliceRestartPoints[i], buf);
		ReadSaveBuf(&PoliceRestartHeadings[i], buf);
	}

	ReadSaveBuf(&NumberOfHospitalRestarts, buf);
	ReadSaveBuf(&NumberOfPoliceRestarts, buf);
	ReadSaveBuf(&bOverrideRestart, buf);

	// skip something unused
	SkipSaveBuf(buf, 3);

	ReadSaveBuf(&OverridePosition, buf);
	ReadSaveBuf(&OverrideHeading, buf);
	ReadSaveBuf(&bFadeInAfterNextDeath, buf);
	ReadSaveBuf(&bFadeInAfterNextArrest, buf);
	ReadSaveBuf(&OverrideHospitalLevel, buf);
	ReadSaveBuf(&OverridePoliceStationLevel, buf);
VALIDATESAVEBUF(size);
}

void
CRestart::SaveAllRestartPoints(uint8 *buf, uint32 *size)
{
	*size = SAVE_HEADER_SIZE
		+ sizeof(HospitalRestartPoints)
		+ sizeof(HospitalRestartHeadings)
		+ sizeof(PoliceRestartPoints)
		+ sizeof(PoliceRestartHeadings)
		+ sizeof(NumberOfHospitalRestarts)
		+ sizeof(NumberOfPoliceRestarts)
		+ sizeof(bOverrideRestart)
		+ sizeof(uint8)
		+ sizeof(uint16)
		+ sizeof(OverridePosition)
		+ sizeof(OverrideHeading)
		+ sizeof(bFadeInAfterNextDeath)
		+ sizeof(bFadeInAfterNextArrest)
		+ sizeof(OverrideHospitalLevel)
		+ sizeof(OverridePoliceStationLevel); // == 292

INITSAVEBUF
	WriteSaveHeader(buf, 'R','S','T','\0', *size - SAVE_HEADER_SIZE);

	for (int i = 0; i < NUM_RESTART_POINTS; i++) {
		WriteSaveBuf(buf, HospitalRestartPoints[i]);
		WriteSaveBuf(buf, HospitalRestartHeadings[i]);
	}

	for (int i = 0; i < NUM_RESTART_POINTS; i++) {
		WriteSaveBuf(buf, PoliceRestartPoints[i]);
		WriteSaveBuf(buf, PoliceRestartHeadings[i]);
	}

	WriteSaveBuf(buf, NumberOfHospitalRestarts);
	WriteSaveBuf(buf, NumberOfPoliceRestarts);
	WriteSaveBuf(buf, bOverrideRestart);

	WriteSaveBuf(buf, (uint8)0);
	WriteSaveBuf(buf, (uint16)0);

	WriteSaveBuf(buf, OverridePosition);
	WriteSaveBuf(buf, OverrideHeading);
	WriteSaveBuf(buf, bFadeInAfterNextDeath);
	WriteSaveBuf(buf, bFadeInAfterNextArrest);
	WriteSaveBuf(buf, OverrideHospitalLevel);
	WriteSaveBuf(buf, OverridePoliceStationLevel);
VALIDATESAVEBUF(*size);
}
