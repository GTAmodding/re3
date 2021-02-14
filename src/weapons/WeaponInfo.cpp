#include "common.h"

#include "main.h"
#include "FileMgr.h"
#include "WeaponInfo.h"
#include "AnimManager.h"
#include "AnimBlendAssociation.h"
#include "Weapon.h"

static CWeaponInfo aWeaponInfo[WEAPONTYPE_TOTALWEAPONS];

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
	return &aWeaponInfo[weaponType];
}

void
CWeaponInfo::Initialise(void)
{
	debug("Initialising CWeaponInfo...\n");
	for (int i = 0; i < WEAPONTYPE_TOTALWEAPONS; i++) {
		aWeaponInfo[i].m_eWeaponFire = WEAPON_FIRE_INSTANT_HIT;
		aWeaponInfo[i].m_AnimToPlay = ANIM_STD_PUNCH;
		aWeaponInfo[i].m_Anim2ToPlay = ANIM_STD_NUM;
		aWeaponInfo[i].m_Flags = WEAPONFLAG_USE_GRAVITY | WEAPONFLAG_SLOWS_DOWN | WEAPONFLAG_RAND_SPEED | WEAPONFLAG_EXPANDS | WEAPONFLAG_EXPLODES;
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

	size_t bp, buflen;
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
		for (lp = 0; line[lp] <= ' ' && line[lp] != '\0'; lp++);

		if (line[lp] == '\0' || line[lp] == '#')
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
		animId = ANIM_STD_WALK;
		sscanf(
			&line[lp],
			"%s %s %f %d %d %d %d %f %f %f %f %f %f %f %s %s %f %f %f %f %d %d",
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
			anim2ToPlay,
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

		if (strcmp(anim2ToPlay, "null") != 0) {
			animAssoc = CAnimManager::GetAnimAssociation(ASSOCGRP_STD, anim2ToPlay);
			aWeaponInfo[weaponType].m_Anim2ToPlay = (AnimationId) animAssoc->animId;
		}

		CVector vecFireOffset(fireOffsetX, fireOffsetY, fireOffsetZ);

		aWeaponInfo[weaponType].m_eWeaponFire = FindWeaponFireType(fireType);
		aWeaponInfo[weaponType].m_fRange = range;
		aWeaponInfo[weaponType].m_nFiringRate = firingRate;
		aWeaponInfo[weaponType].m_nReload = reload;
		aWeaponInfo[weaponType].m_nAmountofAmmunition = ammoAmount;
		aWeaponInfo[weaponType].m_nDamage = damage;
		aWeaponInfo[weaponType].m_fSpeed = speed;
		aWeaponInfo[weaponType].m_fRadius = radius;
		aWeaponInfo[weaponType].m_fLifespan = lifeSpan;
		aWeaponInfo[weaponType].m_fSpread = spread;
		aWeaponInfo[weaponType].m_vecFireOffset = vecFireOffset;
		aWeaponInfo[weaponType].m_AnimToPlay = animId;
		aWeaponInfo[weaponType].m_fAnimLoopStart = animLoopStart / 30.0f;
		aWeaponInfo[weaponType].m_fAnimLoopEnd = animLoopEnd / 30.0f;
		aWeaponInfo[weaponType].m_fAnimFrameFire = delayBetweenAnimAndFire / 30.0f;
		aWeaponInfo[weaponType].m_fAnim2FrameFire = delayBetweenAnim2AndFire / 30.0f;
		aWeaponInfo[weaponType].m_nModelId = modelId;
		aWeaponInfo[weaponType].m_Flags = flags;
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
