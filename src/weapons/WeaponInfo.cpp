#include "common.h"
#include "patcher.h"
#include "main.h"
#include "FileMgr.h"
#include "WeaponInfo.h"
#include "AnimManager.h"
#include "AnimBlendAssociation.h"
#include "Weapon.h"

//CWeaponInfo (&CWeaponInfo::ms_apWeaponInfos)[14] = * (CWeaponInfo(*)[14]) * (uintptr*)0x6503EC;
CWeaponInfo CWeaponInfo::ms_apWeaponInfos[WEAPONTYPE_TOTALWEAPONS];

static char ms_aWeaponNames[][32] = {
	"Unarmed",
	"BaseballBat",
	"Colt45",
	"Uzi",
	"Shotgun",
	"AK47",
	"M16",
	"SniperRifle",
	"RocketLauncher",
	"FlameThrower",
	"Molotov",
	"Grenade",
	"Detonator",
	"HeliCannon"
};

CWeaponInfo*
CWeaponInfo::GetWeaponInfo(eWeaponType weaponType) {
	return &CWeaponInfo::ms_apWeaponInfos[weaponType];
}

void
CWeaponInfo::Initialise(void)
{
	debug("Initialising CWeaponInfo...\n");
	for (int i = 0; i < WEAPONTYPE_TOTALWEAPONS; i++) {
		ms_apWeaponInfos[i].m_eWeaponFire = WEAPON_FIRE_INSTANT_HIT;
		ms_apWeaponInfos[i].m_AnimToPlay = ANIM_PUNCH_R;
		ms_apWeaponInfos[i].m_Anim2ToPlay = NUM_ANIMS;
		ms_apWeaponInfos[i].m_bUseGravity = 1;
		ms_apWeaponInfos[i].m_bSlowsDown = 1;
		ms_apWeaponInfos[i].m_bRandSpeed = 1;
		ms_apWeaponInfos[i].m_bExpands = 1;
		ms_apWeaponInfos[i].m_bExplodes = 1;
	}
	debug("Loading weapon data...\n");
	LoadWeaponData();
	debug("CWeaponInfo ready\n");
}

void
CWeaponInfo::LoadWeaponData(void)
{
	float spread, speed, lifeSpan, radius;
	float range, fireOffsetX, fireOffsetY, fireOffsetZ;
	float delayBetweenAnimAndFire, delayBetweenAnim2AndFire, animLoopStart, animLoopEnd;
	int flags, ammoAmount, damage, reload, weaponType;
	int firingRate, modelId;
	char line[256], weaponName[32], fireType[32];
	char animToPlay[32], anim2ToPlay[32];

	CAnimBlendAssociation *animAssoc;
	AnimationId animId;

	int bp, buflen;
	int lp, linelen;
		
	CFileMgr::SetDir("DATA");
	buflen = CFileMgr::LoadFile("WEAPON.DAT", work_buff, sizeof(work_buff), "r");
	CFileMgr::SetDir("");

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
		animId = ANIM_WALK;
		sscanf(
			&line[lp],
			"%s %s %f %d %d %d %d %f %f %f %f %f %f %f %s %s %f %f %f %f %d %d",
			&weaponName,
			&fireType,
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
			&animToPlay,
			&anim2ToPlay,
			&animLoopStart,
			&animLoopEnd,
			&delayBetweenAnimAndFire,
			&delayBetweenAnim2AndFire,
			&modelId,
			&flags);

		if (strncmp(weaponName, "ENDWEAPONDATA", 13) == 0)
			return;

		weaponType = FindWeaponType(weaponName);

		animAssoc = CAnimManager::GetAnimAssociation(ASSOCGRP_STD, animToPlay);
		animId = static_cast<AnimationId>(animAssoc->animId);

		if (strncmp(anim2ToPlay, "null", 4) != 0) {
			animAssoc = CAnimManager::GetAnimAssociation(ASSOCGRP_STD, anim2ToPlay);
			ms_apWeaponInfos[weaponType].m_Anim2ToPlay = (AnimationId) animAssoc->animId;
		}

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
		ms_apWeaponInfos[weaponType].m_AnimToPlay = animId;
		ms_apWeaponInfos[weaponType].m_fAnimLoopStart = animLoopStart / 30.0f;
		ms_apWeaponInfos[weaponType].m_fAnimLoopEnd = animLoopEnd / 30.0f;
		ms_apWeaponInfos[weaponType].m_fAnimFrameFire = delayBetweenAnimAndFire / 30.0f;
		ms_apWeaponInfos[weaponType].m_fAnim2FrameFire = delayBetweenAnim2AndFire / 30.0f;
		ms_apWeaponInfos[weaponType].m_nModelId = modelId;
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
	Error("Unknown weapon fire type, WeaponInfo.cpp");
	return WEAPON_FIRE_INSTANT_HIT;
}

void
CWeaponInfo::Shutdown(void)
{
	debug("Shutting down CWeaponInfo...\n");
	debug("CWeaponInfo shut down\n");
}

STARTPATCHES
	InjectHook(0x564EA0, &CWeaponInfo::Initialise, PATCH_JUMP);
	InjectHook(0x564FD0, &CWeaponInfo::GetWeaponInfo, PATCH_JUMP);
	InjectHook(0x5653E0, &CWeaponInfo::FindWeaponType, PATCH_JUMP);
	InjectHook(0x5653B0, &CWeaponInfo::FindWeaponFireType, PATCH_JUMP);
	InjectHook(0x564FE0, &CWeaponInfo::LoadWeaponData, PATCH_JUMP);
	InjectHook(0x564FB0, &CWeaponInfo::Shutdown, PATCH_JUMP);
ENDPATCHES