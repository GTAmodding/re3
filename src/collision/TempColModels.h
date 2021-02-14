#pragma once

#include "ColModel.h"

class CTempColModels
{
public:
	static CColModel ms_colModelPed1;
	static CColModel ms_colModelPed2;
	static CColModel ms_colModelBBox;
	static CColModel ms_colModelBumper1;
	static CColModel ms_colModelWheel1;	
	static CColModel ms_colModelPanel1;
	static CColModel ms_colModelBodyPart2;
	static CColModel ms_colModelBodyPart1;
	static CColModel ms_colModelCutObj[5];
	static CColModel ms_colModelPedGroundHit;
	static CColModel ms_colModelBoot1;
	static CColModel ms_colModelDoor1;
	static CColModel ms_colModelBonnet1;

	static void Initialise(void);
};
