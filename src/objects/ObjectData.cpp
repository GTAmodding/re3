#include "common.h"
#include "patcher.h"
#include "main.h"
#include "ModelInfo.h"
#include "Object.h"
#include "FileMgr.h"
#include "ObjectData.h"

CObjectInfo CObjectData::ms_aObjectInfo[NUMOBJECTINFO];

// Another ugly file reader
void
CObjectData::Initialise(const char *filename)
{
	char *p, *lp;
	char line[1024], name[256];
	int id;
	float percentSubmerged;
	int damageEffect, responseCase, camAvoid;
	CBaseModelInfo *mi;

	CFileMgr::SetDir("");
	CFileMgr::LoadFile(filename, work_buff, sizeof(work_buff), "r");

	id = 0;
	p = (char*)work_buff;
	while(*p != '*'){
		// skip over white space and comments
		while(*p == ' ' || *p == '\n' || *p == '\r' || *p == ';')
			if(*p == ';')
				while(*p != '\n' && *p != '*')
					p++;
			else
				p++;

		if(*p == '*')
			break;

		// read one line
		lp = line;
		while(*p != '\n' && *p != '*'){
			*lp++ = *p == ',' ? ' ' : *p;
			p++;
		}
		if(*p == '\n')
			p++;
		*lp = '\0';	// FIX: game wrote '\n' here

		assert(id < NUMOBJECTINFO);
		sscanf(line, "%s %f %f %f %f %f %f %f %d %d %d", name,
			&ms_aObjectInfo[id].m_fMass,
			&ms_aObjectInfo[id].m_fTurnMass,
			&ms_aObjectInfo[id].m_fAirResistance,
			&ms_aObjectInfo[id].m_fElasticity,
			&percentSubmerged,
			&ms_aObjectInfo[id].m_fUprootLimit,
			&ms_aObjectInfo[id].m_fCollisionDamageMultiplier,
			&damageEffect, &responseCase, &camAvoid);

		ms_aObjectInfo[id].m_fBuoyancy = 100.0f/percentSubmerged * GRAVITY *ms_aObjectInfo[id].m_fMass;
		ms_aObjectInfo[id].m_nCollisionDamageEffect = damageEffect;
		ms_aObjectInfo[id].m_nSpecialCollisionResponseCases = responseCase;
		ms_aObjectInfo[id].m_bCameraToAvoidThisObject = camAvoid;

		mi = CModelInfo::GetModelInfo(name, nil);
		if(mi)
			mi->SetObjectID(id++);
		else
			debug("CObjectData: Cannot find object %s\n", name);
	}
}

void
CObjectData::SetObjectData(int32 modelId, CObject &object)
{
	CObjectInfo *objinfo;

	if(CModelInfo::GetModelInfo(modelId)->GetObjectID() == -1)
		return;

	objinfo = &ms_aObjectInfo[CModelInfo::GetModelInfo(modelId)->GetObjectID()];

	object.m_fMass = objinfo->m_fMass;
	object.m_fTurnMass = objinfo->m_fTurnMass;
	object.m_fAirResistance = objinfo->m_fAirResistance;
	object.m_fElasticity = objinfo->m_fElasticity;
	object.m_fBuoyancy = objinfo->m_fBuoyancy;
	object.m_fUprootLimit = objinfo->m_fUprootLimit;
	object.m_fCollisionDamageMultiplier = objinfo->m_fCollisionDamageMultiplier;
	object.m_nCollisionDamageEffect = objinfo->m_nCollisionDamageEffect;
	object.m_nSpecialCollisionResponseCases = objinfo->m_nSpecialCollisionResponseCases;
	object.m_bCameraToAvoidThisObject = objinfo->m_bCameraToAvoidThisObject;
	if(object.m_fMass >= 99998.0f){
		object.bInfiniteMass = true;
		object.bAffectedByGravity = false;
		object.bExplosionProof = true;
	}
}

STARTPATCHES
	InjectHook(0x4BC0E0, CObjectData::Initialise, PATCH_JUMP);
	InjectHook(0x4BC270, CObjectData::SetObjectData, PATCH_JUMP);
ENDPATCHES
