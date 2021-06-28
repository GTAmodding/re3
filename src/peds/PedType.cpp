#include "common.h"

#include "General.h"
#include "FileMgr.h"
#include "PedType.h"
#include "SaveBuf.h"

CPedType *CPedType::ms_apPedType[NUM_PEDTYPES];
CPedStats *CPedStats::ms_apPedStats[NUM_PEDSTATS];

void
CPedType::Initialise(void)
{
	int i;

	debug("Initialising CPedType...\n");
	for(i = 0; i < NUM_PEDTYPES; i++){
		ms_apPedType[i] = new CPedType;
		ms_apPedType[i]->m_flag = PED_FLAG_PLAYER1;
		ms_apPedType[i]->unknown1 = 0.0f;
		ms_apPedType[i]->unknown2 = 0.0f;
		// unknown3 not initialized
		ms_apPedType[i]->unknown4 = 0.0f;
		ms_apPedType[i]->unknown5 = 0.0f;
		ms_apPedType[i]->m_threats = 0;
		ms_apPedType[i]->m_avoid = 0;
	}
	debug("Loading ped data...\n");
	LoadPedData();
	debug("CPedType ready\n");
}

void
CPedType::Shutdown(void)
{
	int i;
	debug("Shutting down CPedType...\n");
	for(i = 0; i < NUM_PEDTYPES; i++)
		delete ms_apPedType[i];
	debug("CPedType shut down\n");
}

void
CPedType::LoadPedData(void)
{
	char *buf;
	char line[256];
	char word[32];
	ssize_t bp, buflen;
	int lp, linelen;
	int type;
	uint32 flags;
	float f1, f2, f3, f4, f5;

	type = NUM_PEDTYPES;
	buf = new char[16 * 1024];

	CFileMgr::SetDir("DATA");
	buflen = CFileMgr::LoadFile("PED.DAT", (uint8*)buf, 16 * 1024, "r");
	CFileMgr::SetDir("");

	for(bp = 0; bp < buflen; ){
		// read file line by line
		for(linelen = 0; buf[bp] != '\n' && bp < buflen; bp++){
			if(buf[bp] == '\r' || buf[bp] == ',' || buf[bp] == '\t')
				line[linelen++] = ' ';
			else
				line[linelen++] = buf[bp];
		}
		bp++;
		line[linelen] = '\0';

		// skip white space
		for(lp = 0; line[lp] <= ' '; lp++);

		if(lp >= linelen ||		// FIX: game uses == here, but this is safer if we have empty lines
		   line[lp] == '#')
			continue;

		// Game uses just "line" here since sscanf already trims whitespace, but this is safer
		sscanf(&line[lp], "%s", word);

		if(strcmp(word, "Threat") == 0){
			flags = 0;
			lp += 7;
			while(sscanf(&line[lp], "%s", word) == 1 && lp <= linelen){
				flags |= FindPedFlag(word);
				// skip word
				while(line[lp] != ' ' && line[lp] != '\n' && line[lp] != '\0')
					lp++;
				// skip white space
				while(line[lp] == ' ')
					lp++;
			}
			ms_apPedType[type]->m_threats = flags;
		}else if(strcmp(word, "Avoid") == 0){
			flags = 0;
			lp += 6;
			while(sscanf(&line[lp], "%s", word) == 1 && lp <= linelen){
				flags |= FindPedFlag(word);
				// skip word
				while(line[lp] != ' ' && line[lp] != '\n' && line[lp] != '\0')
					lp++;
				// skip white space
				while(line[lp] == ' ')
					lp++;
			}
			ms_apPedType[type]->m_avoid = flags;
		}else{
			sscanf(line, "%s %f %f %f %f %f", word, &f1, &f2, &f3, &f4, &f5);
			type = FindPedType(word);
			ms_apPedType[type]->m_flag = FindPedFlag(word);
			// unknown values
			ms_apPedType[type]->unknown1 = f1 / 50.0f;
			ms_apPedType[type]->unknown2 = f2 / 50.0f;
			ms_apPedType[type]->unknown3 = f3 / 50.0f;
			ms_apPedType[type]->unknown4 = f4;
			ms_apPedType[type]->unknown5 = f5;

		}
	}

	delete[] buf;
}

ePedType
CPedType::FindPedType(char *type)
{
	if(strcmp(type, "PLAYER1") == 0) return PEDTYPE_PLAYER1;
	if(strcmp(type, "PLAYER2") == 0) return PEDTYPE_PLAYER2;
	if(strcmp(type, "PLAYER3") == 0) return PEDTYPE_PLAYER3;
	if(strcmp(type, "PLAYER4") == 0) return PEDTYPE_PLAYER4;
	if(strcmp(type, "CIVMALE") == 0) return PEDTYPE_CIVMALE;
	if(strcmp(type, "CIVFEMALE") == 0) return PEDTYPE_CIVFEMALE;
	if(strcmp(type, "COP") == 0) return PEDTYPE_COP;
	if(strcmp(type, "GANG1") == 0) return PEDTYPE_GANG1;
	if(strcmp(type, "GANG2") == 0) return PEDTYPE_GANG2;
	if(strcmp(type, "GANG3") == 0) return PEDTYPE_GANG3;
	if(strcmp(type, "GANG4") == 0) return PEDTYPE_GANG4;
	if(strcmp(type, "GANG5") == 0) return PEDTYPE_GANG5;
	if(strcmp(type, "GANG6") == 0) return PEDTYPE_GANG6;
	if(strcmp(type, "GANG7") == 0) return PEDTYPE_GANG7;
	if(strcmp(type, "GANG8") == 0) return PEDTYPE_GANG8;
	if(strcmp(type, "GANG9") == 0) return PEDTYPE_GANG9;
	if(strcmp(type, "EMERGENCY") == 0) return PEDTYPE_EMERGENCY;
	if(strcmp(type, "FIREMAN") == 0) return PEDTYPE_FIREMAN;
	if(strcmp(type, "CRIMINAL") == 0) return PEDTYPE_CRIMINAL;
	if(strcmp(type, "SPECIAL") == 0) return PEDTYPE_SPECIAL;
	if(strcmp(type, "PROSTITUTE") == 0) return PEDTYPE_PROSTITUTE;
	Error("Unknown ped type, Pedtype.cpp");
	return NUM_PEDTYPES;
}

uint32
CPedType::FindPedFlag(char *type)
{
	if(strcmp(type, "PLAYER1") == 0) return PED_FLAG_PLAYER1;
	if(strcmp(type, "PLAYER2") == 0) return PED_FLAG_PLAYER2;
	if(strcmp(type, "PLAYER3") == 0) return PED_FLAG_PLAYER3;
	if(strcmp(type, "PLAYER4") == 0) return PED_FLAG_PLAYER4;
	if(strcmp(type, "CIVMALE") == 0) return PED_FLAG_CIVMALE;
	if(strcmp(type, "CIVFEMALE") == 0) return PED_FLAG_CIVFEMALE;
	if(strcmp(type, "COP") == 0) return PED_FLAG_COP;
	if(strcmp(type, "GANG1") == 0) return PED_FLAG_GANG1;
	if(strcmp(type, "GANG2") == 0) return PED_FLAG_GANG2;
	if(strcmp(type, "GANG3") == 0) return PED_FLAG_GANG3;
	if(strcmp(type, "GANG4") == 0) return PED_FLAG_GANG4;
	if(strcmp(type, "GANG5") == 0) return PED_FLAG_GANG5;
	if(strcmp(type, "GANG6") == 0) return PED_FLAG_GANG6;
	if(strcmp(type, "GANG7") == 0) return PED_FLAG_GANG7;
	if(strcmp(type, "GANG8") == 0) return PED_FLAG_GANG8;
	if(strcmp(type, "GANG9") == 0) return PED_FLAG_GANG9;
	if(strcmp(type, "EMERGENCY") == 0) return PED_FLAG_EMERGENCY;
	if(strcmp(type, "FIREMAN") == 0) return PED_FLAG_FIREMAN;
	if(strcmp(type, "CRIMINAL") == 0) return PED_FLAG_CRIMINAL;
	if(strcmp(type, "SPECIAL") == 0) return PED_FLAG_SPECIAL;
	if(strcmp(type, "GUN") == 0) return PED_FLAG_GUN;
	if(strcmp(type, "COP_CAR") == 0) return PED_FLAG_COP_CAR;
	if(strcmp(type, "FAST_CAR") == 0) return PED_FLAG_FAST_CAR;
	if(strcmp(type, "EXPLOSION") == 0) return PED_FLAG_EXPLOSION;
	if(strcmp(type, "PROSTITUTE") == 0) return PED_FLAG_PROSTITUTE;
	if(strcmp(type, "DEADPEDS") == 0) return PED_FLAG_DEADPEDS;
	return 0;
}

void
CPedType::Save(uint8 *buf, uint32 *size)
{
	*size = sizeof(CPedType) * NUM_PEDTYPES + SAVE_HEADER_SIZE;
INITSAVEBUF
	WriteSaveHeader(buf, 'P','T','P','\0', *size - SAVE_HEADER_SIZE);
	for(int i = 0; i < NUM_PEDTYPES; i++)
		WriteSaveBuf(buf, *ms_apPedType[i]);
VALIDATESAVEBUF(*size)
}

void
CPedType::Load(uint8 *buf, uint32 size)
{
INITSAVEBUF
	// original: SkipSaveBuf(buf, SAVE_HEADER_SIZE);
	CheckSaveHeader(buf, 'P', 'T', 'P', '\0', size - SAVE_HEADER_SIZE);

	for(int i = 0; i < NUM_PEDTYPES; i++)
		ReadSaveBuf(ms_apPedType[i], buf);
VALIDATESAVEBUF(size)
}

void
CPedStats::Initialise(void)
{
	int i;

	debug("Initialising CPedStats...\n");
	for(i = 0; i < NUM_PEDSTATS; i++){
		ms_apPedStats[i] = new CPedStats;
		ms_apPedStats[i]->m_type = PEDSTAT_PLAYER;
		ms_apPedStats[i]->m_name[8] = 'R';	// WHAT?
		ms_apPedStats[i]->m_fleeDistance = 20.0f;
		ms_apPedStats[i]->m_headingChangeRate = 15.0f;
		ms_apPedStats[i]->m_fear = 50;
		ms_apPedStats[i]->m_temper = 50;
		ms_apPedStats[i]->m_lawfulness = 50;
		ms_apPedStats[i]->m_sexiness = 50;
		ms_apPedStats[i]->m_attackStrength = 1.0f;
		ms_apPedStats[i]->m_defendWeakness = 1.0f;
		ms_apPedStats[i]->m_flags = 0;
	}
	debug("Loading pedstats data...\n");
	CPedStats::LoadPedStats();
	debug("CPedStats ready\n");
}

void
CPedStats::Shutdown(void)
{
	int i;
	debug("Shutting down CPedStats...\n");
	for(i = 0; i < NUM_PEDSTATS; i++)
		delete ms_apPedStats[i];
	debug("CPedStats shut down\n");
}

void
CPedStats::LoadPedStats(void)
{
	char *buf;
	char line[256];
	char name[32];
	ssize_t bp, buflen;
	int lp, linelen;
	int type;
	float fleeDist, headingChangeRate, attackStrength, defendWeakness;
	int fear, temper, lawfullness, sexiness, flags;

	type = 0;
	buf = new char[16 * 1024];

	CFileMgr::SetDir("DATA");
	buflen = CFileMgr::LoadFile("PEDSTATS.DAT", (uint8*)buf, 16 * 1024, "r");
	CFileMgr::SetDir("");

	for(bp = 0; bp < buflen; ){
		// read file line by line
		for(linelen = 0; buf[bp] != '\n' && bp < buflen; bp++){
			if(buf[bp] == '\r' || buf[bp] == ',' || buf[bp] == '\t')
				line[linelen++] = ' ';
			else
				line[linelen++] = buf[bp];
		}
		bp++;
		line[linelen] = '\0';

		// skip white space
		for(lp = 0; line[lp] <= ' '; lp++);

		if(lp >= linelen ||		// FIX: game uses == here, but this is safer if we have empty lines
		   line[lp] == '#')
			continue;

		sscanf(&line[lp], "%s %f %f %d %d %d %d %f %f %d",
			name,
			&fleeDist,
			&headingChangeRate,
			&fear,
			&temper,
			&lawfullness,
			&sexiness,
			&attackStrength,
			&defendWeakness,
			&flags);
		ms_apPedStats[type]->m_type = (ePedStats)type;
		strncpy(ms_apPedStats[type]->m_name, name, 24);	// FIX: game uses strcpy
		ms_apPedStats[type]->m_fleeDistance = fleeDist;
		ms_apPedStats[type]->m_headingChangeRate = headingChangeRate;
		ms_apPedStats[type]->m_fear = fear;
		ms_apPedStats[type]->m_temper = temper;
		ms_apPedStats[type]->m_lawfulness = lawfullness;
		ms_apPedStats[type]->m_sexiness = sexiness;
		ms_apPedStats[type]->m_attackStrength = attackStrength;
		ms_apPedStats[type]->m_defendWeakness = defendWeakness;
		ms_apPedStats[type]->m_flags = flags;
		type++;
	}

	delete[] buf;
}

ePedStats
CPedStats::GetPedStatType(char *name)
{
	for(uint16 type = 0; type < NUM_PEDSTATS; type++)
		if(!CGeneral::faststrcmp(ms_apPedStats[type]->m_name, name))
			return (ePedStats) type;

	return NUM_PEDSTATS;
}
