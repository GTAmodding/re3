#pragma once

#include "Transmission.h"

enum tVehicleType
{
	HANDLING_LANDSTAL,
	HANDLING_IDAHO,
	HANDLING_STINGER,
	HANDLING_LINERUN,
	HANDLING_PEREN,
	HANDLING_SENTINEL,
	HANDLING_PATRIOT,
	HANDLING_FIRETRUK,
	HANDLING_TRASH,
	HANDLING_STRETCH,
	HANDLING_MANANA,
	HANDLING_INFERNUS,
	HANDLING_BLISTA,
	HANDLING_PONY,
	HANDLING_MULE,
	HANDLING_CHEETAH,
	HANDLING_AMBULAN,
	HANDLING_FBICAR,
	HANDLING_MOONBEAM,
	HANDLING_ESPERANT,
	HANDLING_TAXI,
	HANDLING_KURUMA,
	HANDLING_BOBCAT,
	HANDLING_MRWHOOP,
	HANDLING_BFINJECT,
	HANDLING_POLICE,
	HANDLING_ENFORCER,
	HANDLING_SECURICA,
	HANDLING_BANSHEE,
	HANDLING_PREDATOR,
	HANDLING_BUS,
	HANDLING_RHINO,
	HANDLING_BARRACKS,
	HANDLING_TRAIN,
	HANDLING_HELI,
	HANDLING_DODO,
	HANDLING_COACH,
	HANDLING_CABBIE,
	HANDLING_STALLION,
	HANDLING_RUMPO,
	HANDLING_RCBANDIT,
	HANDLING_BELLYUP,
	HANDLING_MRWONGS,
	HANDLING_MAFIA,
	HANDLING_YARDIE,
	HANDLING_YAKUZA,
	HANDLING_DIABLOS,
	HANDLING_COLUMB,
	HANDLING_HOODS,
	HANDLING_AIRTRAIN,
	HANDLING_DEADDODO,
	HANDLING_SPEEDER,
	HANDLING_REEFER,
	HANDLING_PANLANT,
	HANDLING_FLATBED,
	HANDLING_YANKEE,
	HANDLING_BORGNINE,

	NUMHANDLINGS
};

enum tField // most likely a handling field enum, never used so :shrug:
{

};

enum
{
	HANDLING_1G_BOOST = 1,
	HANDLING_2G_BOOST = 2,
	HANDLING_REV_BONNET = 4,
	HANDLING_HANGING_BOOT = 8,
	HANDLING_NO_DOORS = 0x10,
	HANDLING_IS_VAN = 0x20,
	HANDLING_IS_BUS = 0x40,
	HANDLING_IS_LOW = 0x80,
	HANDLING_DBL_EXHAUST = 0x100,
	HANDLING_TAILGATE_BOOT = 0x200,
	HANDLING_NOSWING_BOOT = 0x400,
	HANDLING_NONPLAYER_STABILISER = 0x800,
	HANDLING_NEUTRALHANDLING = 0x1000,
	HANDLING_HAS_NO_ROOF = 0x2000,
	HANDLING_IS_BIG = 0x4000,
	HANDLING_HALOGEN_LIGHTS = 0x8000,
};

struct tHandlingData
{
	tVehicleType nIdentifier;
	float fMass;
	float fInvMass;
	float fTurnMass;
	CVector Dimension;
	CVector CentreOfMass;
	int8 nPercentSubmerged;
	float fBuoyancy;
	float fTractionMultiplier;
	cTransmission Transmission;
	float fBrakeDeceleration;
	float fBrakeBias;
	int8 bABS;
	float fSteeringLock;
	float fTractionLoss;
	float fTractionBias;
	float fUnused;
	float fSuspensionForceLevel;
	float fSuspensionDampingLevel;
	float fSuspensionUpperLimit;
	float fSuspensionLowerLimit;
	float fSuspensionBias;
	float fCollisionDamageMultiplier;
	uint32 Flags;
	float fSeatOffsetDistance;
	int32 nMonetaryValue;
	int8 FrontLights;
	int8 RearLights;
};
VALIDATE_SIZE(tHandlingData, 0xD8);

class CVehicle;

class cHandlingDataMgr
{
	float field_0;	// unused it seems
public:
	float fWheelFriction;	// wheel related
private:
	float field_8;	//
	float field_C;	// unused it seems
	float field_10;	//
	tHandlingData HandlingData[NUMHANDLINGS];
	uint32 field_302C;	// unused it seems

public:
	cHandlingDataMgr(void);
	void Initialise(void);
	void LoadHandlingData(void);
	int FindExactWord(const char *word, const char *words, int wordLen, int numWords);
	void ConvertDataToWorldUnits(tHandlingData *handling);
	void ConvertDataToGameUnits(tHandlingData *handling);
	void RangeCheck(tHandlingData *handling);
	void ModifyHandlingValue(CVehicle *, const tVehicleType &, const tField &, const bool &);
	void DisplayHandlingData(CVehicle *, tHandlingData *, uint8, bool);
	int32 GetHandlingId(const char *name);
	tHandlingData *GetHandlingData(tVehicleType id) { return &HandlingData[id]; }
	bool HasRearWheelDrive(tVehicleType id) { return HandlingData[id].Transmission.nDriveType != 'F'; }
	bool HasFrontWheelDrive(tVehicleType id) { return HandlingData[id].Transmission.nDriveType != 'R'; }
};
VALIDATE_SIZE(cHandlingDataMgr, 0x3030);
extern cHandlingDataMgr mod_HandlingManager;
