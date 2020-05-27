#include "common.h"

#include "main.h"
#include "FileMgr.h"
#include "WeaponInfo.h"
#include "AnimManager.h"
#include "AnimBlendAssociation.h"
#include "Weapon.h"
#include "ModelInfo.h"
#include "ModelIndices.h"

// Yeah...
int32 CWeaponInfo::ms_aMaxAmmoForWeapon[WEAPONTYPE_TOTALWEAPONS] = {
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1
};

CWeaponInfo CWeaponInfo::ms_apWeaponInfos[WEAPONTYPE_TOTALWEAPONS];

// --MIAMI: Todo
static char ms_aWeaponNames[][32] = {
	"Unarmed",
	"BrassKnuckle",
	"ScrewDriver",
	"GolfClub",
	"NightStick",
	"Knife",
	"BaseballBat",
	"Hammer",
	"Cleaver",
	"Machete",
	"Katana",
	"Chainsaw",
	"Grenade",
	"DetonateGrenade",
	"TearGas",
	"Molotov",
	"Rocket",
	"Colt45",
	"Python",
	"Shotgun",
	"Spas12Shotgun",
	"StubbyShotgun",
	"Tec9",
	"Uzi",
	"SilencedIngram",
	"Mp5",
	"m4",
	"Ruger",
	"SniperRifle",
	"LaserScope",
	"RocketLauncher",
	"FlameThrower",
	"M60",
	"Minigun",
	"Detonator",
	"HeliCannon",
	"Camera",
};

CWeaponInfo*
CWeaponInfo::GetWeaponInfo(eWeaponType weaponType)
{
	return &CWeaponInfo::ms_apWeaponInfos[weaponType];
}

// --MIAMI: done except WEAPONTYPE_TOTALWEAPONS value
void
CWeaponInfo::Initialise(void)
{
	debug("Initialising CWeaponInfo...\n");
	for (int i = 0; i < WEAPONTYPE_TOTALWEAPONS; i++) {
		ms_apWeaponInfos[i].m_eWeaponFire = WEAPON_FIRE_INSTANT_HIT;
		ms_apWeaponInfos[i].m_fRange = 0.0f;
		ms_apWeaponInfos[i].m_nFiringRate = 0;
		ms_apWeaponInfos[i].m_nReload = 0;
		ms_apWeaponInfos[i].m_nAmountofAmmunition = 0;
		ms_apWeaponInfos[i].m_nDamage = 0;
		ms_apWeaponInfos[i].m_fSpeed = 0.0f;
		ms_apWeaponInfos[i].m_fRadius = 0.0f;
		ms_apWeaponInfos[i].m_fLifespan = 0.0f;
		ms_apWeaponInfos[i].m_fSpread = 0.0f;
		ms_apWeaponInfos[i].m_vecFireOffset = CVector(0.0f, 0.0f, 0.0f);
		ms_apWeaponInfos[i].m_AnimToPlay = ASSOCGRP_UNARMED;
		ms_apWeaponInfos[i].m_fAnimLoopStart = 0.0f;
		ms_apWeaponInfos[i].m_fAnimLoopEnd = 0.0f;
		ms_apWeaponInfos[i].m_fAnimFrameFire = 0.0f;
		ms_apWeaponInfos[i].m_fAnim2LoopStart = 0.0f;
		ms_apWeaponInfos[i].m_fAnim2LoopEnd = 0.0f;
		ms_apWeaponInfos[i].m_fAnim2FrameFire = 0.0f;
		ms_apWeaponInfos[i].m_fAnimBreakout = 0.0f;
		ms_apWeaponInfos[i].m_bUseGravity = 1;
		ms_apWeaponInfos[i].m_bSlowsDown = 1;
		ms_apWeaponInfos[i].m_bRandSpeed = 1;
		ms_apWeaponInfos[i].m_bExpands = 1;
		ms_apWeaponInfos[i].m_bExplodes = 1;
		ms_apWeaponInfos[i].m_nWeaponSlot = 0;
	}
	debug("Loading weapon data...\n");
	LoadWeaponData();
	debug("CWeaponInfo ready\n");
}

// --MIAMI: Done, commented parts wait for weapons port
void
CWeaponInfo::LoadWeaponData(void)
{
	float spread, speed, lifeSpan, radius;
	float range, fireOffsetX, fireOffsetY, fireOffsetZ;
	float anim2LoopStart, anim2LoopEnd, delayBetweenAnim2AndFire, animBreakout;
	float delayBetweenAnimAndFire, animLoopStart, animLoopEnd;
	int flags, ammoAmount, damage, reload, weaponType;
	int firingRate, modelId, modelId2, weaponSlot;
	char line[256], weaponName[32], fireType[32];
	char animToPlay[32];

	int bp, buflen;
	int lp, linelen;
		
	CFileMgr::SetDir("DATA");
	buflen = CFileMgr::LoadFile("WEAPON.DAT", work_buff, sizeof(work_buff), "r");

	for (bp = 0; bp < buflen; ) {
		// read file line by line
		for (linelen = 0; work_buff[bp] != '\n' && bp < buflen; bp++) {
			line[linelen++] = work_buff[bp];
		}
		bp++;
		line[linelen] = '\0';

		// skip white space
		for (lp = 0; line[lp] <= ' '; lp++);

		if (lp >= linelen ||		// FIX: game uses == here, but this is safer if we have empty lines
			line[lp] == '#')
			continue;

		spread = 0.0f;
		flags = 0;
		speed = 0.0f;
		ammoAmount = 0;
		lifeSpan = 0.0f;
		radius = 0.0f;
		range = 0.0f;
		damage = 0;
		reload = 0;
		firingRate = 0;
		fireOffsetX = 0.0f;
		weaponName[0] = '\0';
		fireType[0] = '\0';
		fireOffsetY = 0.0f;
		fireOffsetZ = 0.0f;
		sscanf(
			&line[lp],
			"%s %s %f %d %d %d %d %f %f %f %f %f %f %f %s %f %f %f %f %f %f %f %d %d %x %d",
			weaponName,
			fireType,
			&range,
			&firingRate,
			&reload,
			&ammoAmount,
			&damage,
			&speed,
			&radius,
			&lifeSpan,
			&spread,
			&fireOffsetX,
			&fireOffsetY,
			&fireOffsetZ,
			animToPlay,
			&animLoopStart,
			&animLoopEnd,
			&delayBetweenAnimAndFire,
			&anim2LoopStart,
			&anim2LoopEnd,
			&delayBetweenAnim2AndFire,
			&animBreakout,
			&modelId,
			&modelId2,
			&flags,
			&weaponSlot);

		if (strncmp(weaponName, "ENDWEAPONDATA", 13) == 0)
			return;

		weaponType = FindWeaponType(weaponName);

		CVector vecFireOffset(fireOffsetX, fireOffsetY, fireOffsetZ);

		ms_apWeaponInfos[weaponType].m_eWeaponFire = FindWeaponFireType(fireType);
		ms_apWeaponInfos[weaponType].m_fRange = range;
		ms_apWeaponInfos[weaponType].m_nFiringRate = firingRate;
		ms_apWeaponInfos[weaponType].m_nReload = reload;
		ms_apWeaponInfos[weaponType].m_nAmountofAmmunition = ammoAmount;
		ms_apWeaponInfos[weaponType].m_nDamage = damage;
		ms_apWeaponInfos[weaponType].m_fSpeed = speed;
		ms_apWeaponInfos[weaponType].m_fRadius = radius;
		ms_apWeaponInfos[weaponType].m_fLifespan = lifeSpan;
		ms_apWeaponInfos[weaponType].m_fSpread = spread;
		ms_apWeaponInfos[weaponType].m_vecFireOffset = vecFireOffset;
		ms_apWeaponInfos[weaponType].m_fAnimLoopStart = animLoopStart / 30.0f;
		ms_apWeaponInfos[weaponType].m_fAnimLoopEnd = animLoopEnd / 30.0f;
		ms_apWeaponInfos[weaponType].m_fAnim2LoopStart = anim2LoopStart / 30.0f;
		ms_apWeaponInfos[weaponType].m_fAnim2LoopEnd = anim2LoopEnd / 30.0f;
		ms_apWeaponInfos[weaponType].m_fAnimFrameFire = delayBetweenAnimAndFire / 30.0f;
		ms_apWeaponInfos[weaponType].m_fAnim2FrameFire = delayBetweenAnim2AndFire / 30.0f;
		ms_apWeaponInfos[weaponType].m_fAnimBreakout = animBreakout / 30.0f;
		ms_apWeaponInfos[weaponType].m_nModelId = modelId;
		ms_apWeaponInfos[weaponType].m_nModel2Id = modelId2;

		ms_apWeaponInfos[weaponType].m_bUseGravity = flags;
		ms_apWeaponInfos[weaponType].m_bSlowsDown = flags >> 1;
		ms_apWeaponInfos[weaponType].m_bDissipates = flags >> 2;
		ms_apWeaponInfos[weaponType].m_bRandSpeed = flags >> 3;
		ms_apWeaponInfos[weaponType].m_bExpands = flags >> 4;
		ms_apWeaponInfos[weaponType].m_bExplodes = flags >> 5;
		ms_apWeaponInfos[weaponType].m_bCanAim = flags >> 6;
		ms_apWeaponInfos[weaponType].m_bCanAimWithArm = flags >> 7;
		ms_apWeaponInfos[weaponType].m_b1stPerson = flags >> 8;
		ms_apWeaponInfos[weaponType].m_bHeavy = flags >> 9;
		ms_apWeaponInfos[weaponType].m_bThrow = flags >> 10;
		ms_apWeaponInfos[weaponType].m_bReloadLoop2Start = flags >> 11;
		ms_apWeaponInfos[weaponType].m_bUse2nd = flags >> 12;
		ms_apWeaponInfos[weaponType].m_bGround2nd = flags >> 13;
		ms_apWeaponInfos[weaponType].m_bFinish3rd = flags >> 14;
		ms_apWeaponInfos[weaponType].m_bReload = flags >> 15;
		ms_apWeaponInfos[weaponType].m_bFightMode = flags >> 16;
		ms_apWeaponInfos[weaponType].m_bCrouchFire = flags >> 17;
		ms_apWeaponInfos[weaponType].m_bCop3rd = flags >> 18;
		ms_apWeaponInfos[weaponType].m_bGround3rd = flags >> 19;
		ms_apWeaponInfos[weaponType].m_bPartialAttack = flags >> 20;
		ms_apWeaponInfos[weaponType].m_bAnimDetonate = flags >> 21;

		ms_apWeaponInfos[weaponType].m_nWeaponSlot = weaponSlot;

		if (animLoopEnd < 98.0f && weaponType != WEAPONTYPE_FLAMETHROWER && !CWeapon::IsShotgun(weaponType))
			ms_apWeaponInfos[weaponType].m_nFiringRate = ((ms_apWeaponInfos[weaponType].m_fAnimLoopEnd - ms_apWeaponInfos[weaponType].m_fAnimLoopStart) * 900.0f);

		if (weaponType == WEAPONTYPE_DETONATOR || weaponType == WEAPONTYPE_HELICANNON)
			modelId = -1;
		else if (weaponType == WEAPONTYPE_DETONATOR_GRENADE)
			modelId = MI_BOMB;

		if (modelId != -1)
			((CWeaponModelInfo*)CModelInfo::GetModelInfo(modelId))->SetWeaponInfo(weaponType);

		for (int i = 0; i < NUM_ANIM_ASSOC_GROUPS; i++) {
			if (!strcmp(animToPlay, CAnimManager::GetAnimGroupName((AssocGroupId)i))) {
				ms_apWeaponInfos[weaponType].m_AnimToPlay = (AssocGroupId)i;
				break;
			}
		}
	}
}

eWeaponType
CWeaponInfo::FindWeaponType(char *name)
{
	for (int i = 0; i < WEAPONTYPE_TOTALWEAPONS; i++) {
		if (strcmp(ms_aWeaponNames[i], name) == 0) {
			return static_cast<eWeaponType>(i);
		}
	}
	return WEAPONTYPE_UNARMED;
}

eWeaponFire
CWeaponInfo::FindWeaponFireType(char *name)
{
	if (strcmp(name, "MELEE") == 0) return WEAPON_FIRE_MELEE;
	if (strcmp(name, "INSTANT_HIT") == 0) return WEAPON_FIRE_INSTANT_HIT;
	if (strcmp(name, "PROJECTILE") == 0) return WEAPON_FIRE_PROJECTILE;
	if (strcmp(name, "AREA_EFFECT") == 0) return WEAPON_FIRE_AREA_EFFECT;
	if (strcmp(name, "CAMERA") == 0) return WEAPON_FIRE_CAMERA;
	Error("Unknown weapon fire type, WeaponInfo.cpp");
	return WEAPON_FIRE_INSTANT_HIT;
}

void
CWeaponInfo::Shutdown(void)
{
	debug("Shutting down CWeaponInfo...\n");
	debug("CWeaponInfo shut down\n");
}
