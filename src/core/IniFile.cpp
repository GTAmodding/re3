#include "common.h"

#include "IniFile.h"

#include "vehicle_enums.h" //needs to be before Vehicle.h

#include "Vehicle.h"

#include "CarCtrl.h"
#include "FileMgr.h"
#include "Population.h"
#include "main.h"

// --MIAMI: file done

float CIniFile::PedNumberMultiplier = 0.6f;
float CIniFile::CarNumberMultiplier = 0.6f;

void CIniFile::LoadIniFile()
{
	CFileMgr::SetDir("");
	int f = CFileMgr::OpenFile("gta3.ini", "r");
	if (f){
		CFileMgr::ReadLine(f, gString, 200);
		sscanf(gString, "%f", &PedNumberMultiplier);
		PedNumberMultiplier = Min(3.0f, Max(0.5f, PedNumberMultiplier));
		CFileMgr::ReadLine(f, gString, 200);
		sscanf(gString, "%f", &CarNumberMultiplier);
		CarNumberMultiplier = Min(3.0f, Max(0.5f, CarNumberMultiplier));
		CFileMgr::CloseFile(f);
	}
	CPopulation::MaxNumberOfPedsInUse = 25.0f * PedNumberMultiplier;
	CPopulation::MaxNumberOfPedsInUseInterior = 40.0f * PedNumberMultiplier;
	CCarCtrl::MaxNumberOfCarsInUse = 12.0f * CarNumberMultiplier;
}