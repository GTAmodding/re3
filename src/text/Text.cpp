#include "common.h"

#include "FileMgr.h"
#ifdef MORE_LANGUAGES
#include "Game.h"
#endif
#include "Frontend.h"
#include "Messages.h"
#include "Text.h"
#include "Timer.h"

static wchar WideErrorString[25];

CText TheText;

//--MIAMI: DONE
CText::CText(void)
{
	encoding = 'e';
	bHasMissionTextOffsets = false;
	bIsMissionTextLoaded = false;
	memset(szMissionTableName, 0, sizeof(szMissionTableName));
	memset(WideErrorString, 0, sizeof(WideErrorString));
}

//--MIAMI: DONE
void
CText::Load(void)
{
	char filename[32];
	uint32 offset;
	int file;
	bool tkey_loaded = false, tdat_loaded = false;
	ChunkHeader m_ChunkHeader;

	bIsMissionTextLoaded = false;
	bHasMissionTextOffsets = false;

	Unload();

	CFileMgr::SetDir("TEXT");
	switch(FrontEndMenuManager.m_PrefsLanguage){
	case LANGUAGE_AMERICAN:
		sprintf(filename, "AMERICAN.GXT");
		break;
	case LANGUAGE_FRENCH:
		sprintf(filename, "FRENCH.GXT");
		break;
	case LANGUAGE_GERMAN:
		sprintf(filename, "GERMAN.GXT");
		break;
	case LANGUAGE_ITALIAN:
		sprintf(filename, "ITALIAN.GXT");
		break;
	case LANGUAGE_SPANISH:
		sprintf(filename, "SPANISH.GXT");
		break;
#ifdef MORE_LANGUAGES
	case LANGUAGE_POLISH:
		sprintf(filename, "POLISH.GXT");
		break;
	case LANGUAGE_RUSSIAN:
		sprintf(filename, "RUSSIAN.GXT");
		break;
	case LANGUAGE_JAPANESE:
		sprintf(filename, "JAPANESE.GXT");
		break;
#endif
	}

	file = CFileMgr::OpenFile(filename, "rb");

	offset = 0;
	while (!tkey_loaded || !tdat_loaded) {
		ReadChunkHeader(&m_ChunkHeader, file, &offset);
		if (m_ChunkHeader.size != 0) {
			if (strncmp(m_ChunkHeader.magic, "TABL", 4) == 0) {
				MissionTextOffsets.Load(m_ChunkHeader.size, file, &offset, 0x58000);
				bHasMissionTextOffsets = true;
			} else if (strncmp(m_ChunkHeader.magic, "TKEY", 4) == 0) {
				this->keyArray.Load(m_ChunkHeader.size, file, &offset);
				tkey_loaded = true;
			} else if (strncmp(m_ChunkHeader.magic, "TDAT", 4) == 0) {
				this->data.Load(m_ChunkHeader.size, file, &offset);
				tdat_loaded = true;
			} else {
				CFileMgr::Seek(file, m_ChunkHeader.size, SEEK_CUR);
				offset += m_ChunkHeader.size;
			}
		}
	}

	keyArray.Update(data.chars);
	CFileMgr::CloseFile(file);
	CFileMgr::SetDir("");
}

//--MIAMI: DONE
void
CText::Unload(void)
{
	CMessages::ClearAllMessagesDisplayedByGame();
	keyArray.Unload();
	data.Unload();
	mission_keyArray.Unload();
	mission_data.Unload();
	bIsMissionTextLoaded = false;
	memset(szMissionTableName, 0, sizeof(szMissionTableName));
}

//--MIAMI: DONE
wchar*
CText::Get(const char *key)
{
	uint8 result = false;
#ifdef FIX_BUGS
	wchar *outstr = keyArray.Search(key, data.chars, &result);
#else
	wchar *outstr = keyArray.Search(key, &result);
#endif

	if (!result && bHasMissionTextOffsets && bIsMissionTextLoaded)
#ifdef FIX_BUGS
		outstr = mission_keyArray.Search(key, mission_data.chars, &result);
#else
		outstr = mission_keyArray.Search(key, &result);
#endif
	return outstr;
}

wchar UpperCaseTable[128] = {
	128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138,
	139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149,
	150, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137,
	138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148,
	149, 173, 173, 175, 176, 177, 178, 179, 180, 181, 182,
	183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193,
	194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204,
	205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215,
	216, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226,
	227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237,
	238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248,
	249, 250, 251, 252, 253, 254, 255
};

wchar FrenchUpperCaseTable[128] = {
	128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138,
	139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149,
	150, 65, 65, 65, 65, 132, 133, 69, 69, 69, 69, 73, 73,
	73, 73, 79, 79, 79, 79, 85, 85, 85, 85, 173, 173, 175,
	176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186,
	187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197,
	198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208,
	209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219,
	220, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230,
	231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241,
	242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252,
	253, 254, 255
};

//--MIAMI: TODO (check tables)
wchar
CText::GetUpperCase(wchar c)
{
	switch (encoding)
	{
	case 'e':
		if (c >= 'a' && c <= 'z')
			return c - 32;
		break;
	case 'f':
		if (c >= 'a' && c <= 'z')
			return c - 32;

		if (c >= 128 && c <= 255)
			return FrenchUpperCaseTable[c-128];
		break;
	case 'g':
	case 'i':
	case 's':
		if (c >= 'a' && c <= 'z')
			return c - 32;

		if (c >= 128 && c <= 255)
			return UpperCaseTable[c-128];
		break;
	default:
		break;
	}
	return c;
}

//--MIAMI: DONE
void
CText::UpperCase(wchar *s)
{
	while(*s){
		*s = GetUpperCase(*s);
		s++;
	}
}

//--MIAMI: DONE
void
CText::GetNameOfLoadedMissionText(char *outName)
{
	strcpy(outName, szMissionTableName);
}

//--MIAMI: DONE
void
CText::ReadChunkHeader(ChunkHeader *buf, int32 file, uint32 *offset)
{
	// original code loops 8 times to read 1 byte with CFileMgr::Read, that's retarded
	CFileMgr::Read(file, (char*)buf, sizeof(ChunkHeader));
	*offset += sizeof(ChunkHeader);
}

//--MIAMI: DONE
void
CText::LoadMissionText(char *MissionTableName)
{
	char filename[32];

	mission_keyArray.Unload();
	mission_data.Unload();

	bool search_result = false;
	int missionTableId = 0;

	for (missionTableId = 0; missionTableId < MissionTextOffsets.size; missionTableId++) {
		if (strncmp(MissionTextOffsets.data[missionTableId].szMissionName, MissionTableName, strlen(MissionTextOffsets.data[missionTableId].szMissionName)) == 0) {
			search_result = true;
			break;
		}
	}

	if (!search_result) {
		printf("CText::LoadMissionText - couldn't find %s", MissionTableName);
		return;
	}

	CFileMgr::SetDir("TEXT");
	switch (FrontEndMenuManager.m_PrefsLanguage) {
	case LANGUAGE_AMERICAN:
		sprintf(filename, "AMERICAN.GXT");
		break;
	case LANGUAGE_FRENCH:
		sprintf(filename, "FRENCH.GXT");
		break;
	case LANGUAGE_GERMAN:
		sprintf(filename, "GERMAN.GXT");
		break;
	case LANGUAGE_ITALIAN:
		sprintf(filename, "ITALIAN.GXT");
		break;
	case LANGUAGE_SPANISH:
		sprintf(filename, "SPANISH.GXT");
		break;
#ifdef MORE_LANGUAGES
	case LANGUAGE_POLISH:
		sprintf(filename, "POLISH.GXT");
		break;
	case LANGUAGE_RUSSIAN:
		sprintf(filename, "RUSSIAN.GXT");
		break;
	case LANGUAGE_JAPANESE:
		sprintf(filename, "JAPANESE.GXT");
		break;
#endif
	}
	CTimer::Suspend();
	int file = CFileMgr::OpenFile(filename, "rb");
	CFileMgr::Seek(file, MissionTextOffsets.data[missionTableId].offset, SEEK_SET);

	char TableCheck[8];
	CFileMgr::Read(file, TableCheck, 8);
	if (strncmp(TableCheck, MissionTableName, 8) != 0)
		printf("CText::LoadMissionText - expected to find %s in the text file", MissionTableName);

	bool tkey_loaded = false, tdat_loaded = false;
	ChunkHeader m_ChunkHeader;
	while (!tkey_loaded || !tdat_loaded) {
		uint32 bytes_read = 0;
		ReadChunkHeader(&m_ChunkHeader, file, &bytes_read);
		if (m_ChunkHeader.size != 0) {
			if (strncmp(m_ChunkHeader.magic, "TKEY", 4) == 0) {
				uint32 bytes_read = 0;
				mission_keyArray.Load(m_ChunkHeader.size, file, &bytes_read);
				tkey_loaded = true;
			} else if (strncmp(m_ChunkHeader.magic, "TDAT", 4) == 0) {
				uint32 bytes_read = 0;
				mission_data.Load(m_ChunkHeader.size, file, &bytes_read);
				tdat_loaded = true;
			} else
				CFileMgr::Seek(file, m_ChunkHeader.size, SEEK_CUR);
		}
	}

	mission_keyArray.Update(mission_data.chars);
	CFileMgr::CloseFile(file);
	CTimer::Resume();
	CFileMgr::SetDir("");
	strcpy(szMissionTableName, MissionTableName);
	bIsMissionTextLoaded = true;
}


//--MIAMI: DONE
void
CKeyArray::Load(uint32 length, int file, uint32 *offset)
{
	char *rawbytes;

	numEntries = length / sizeof(CKeyEntry);
	entries = new CKeyEntry[numEntries];
	rawbytes = (char*)entries;

#if DUMB
	for (uint32 i = 0; i < length; i++)
		CFileMgr::Read(file, &rawbytes[i], 1);
#else
	CFileMgr::Read(file, rawbytes, length);
#endif
}

//--MIAMI: DONE
void
CKeyArray::Unload(void)
{
	delete[] entries;
	entries = nil;
	numEntries = 0;
}

//--MIAMI: DONE
void
CKeyArray::Update(wchar *chars)
{
#ifndef FIX_BUGS
	int i;
	for(i = 0; i < numEntries; i++)
		entries[i].value = (wchar*)((uint8*)chars + (uintptr)entries[i].value);
#endif
}

//--MIAMI: DONE
CKeyEntry*
CKeyArray::BinarySearch(const char *key, CKeyEntry *entries, int16 low, int16 high)
{
	int mid;
	int diff;

	if(low > high)
		return nil;

	mid = (low + high)/2;
	diff = strcmp(key, entries[mid].key);
	if(diff == 0)
		return &entries[mid];
	if(diff < 0)
		return BinarySearch(key, entries, low, mid-1);
	if(diff > 0)
		return BinarySearch(key, entries, mid+1, high);
	return nil;
}

//--MIAMI: DONE
wchar*
#ifdef FIX_BUGS
CKeyArray::Search(const char *key, wchar *data, uint8 *result)
#else
CKeyArray::Search(const char *key, uint8 *result)
#endif
{
	CKeyEntry *found;
	char errstr[25];
	int i;

#ifdef FIX_BUGS
	found = BinarySearch(key, entries, 0, numEntries-1);
	if (found) {
		*result = true;
		return (wchar*)((uint8*)data + found->valueOffset);
	}
#else
	found = BinarySearch(key, entries, 0, numEntries-1);
	if (found) {
		*result = true;
		return found->value;
	}
#endif
	*result = false;
#ifdef MASTER
	sprintf(errstr, "%");
#else
	sprintf(errstr, "%s missing", key);
#endif // MASTER
	for(i = 0; i < 25; i++)
		WideErrorString[i] = errstr[i];
	return WideErrorString;
}

//--MIAMI: DONE
void
CData::Load(uint32 length, int file, uint32 *offset)
{
	char *rawbytes;

	numChars = length / sizeof(wchar);
	chars = new wchar[numChars];
	rawbytes = (char*)chars;

#if DUMB
	for(uint32 i = 0; i < length; i++)
		CFileMgr::Read(file, &rawbytes[i], 1);
#else
	CFileMgr::Read(file, rawbytes, length);
#endif
}

//--MIAMI: DONE
void
CData::Unload(void)
{
	delete[] chars;
	chars = nil;
	numChars = 0;
}

//--MIAMI: DONE
void
CMissionTextOffsets::Load(uint32 table_size, int file, uint32 *offset, int)
{
	// not exact VC code but smaller and better :P
	size = table_size / sizeof(CMissionTextOffsets::Entry);
	CFileMgr::Read(file, (char*)data, sizeof(CMissionTextOffsets::Entry) * size);
	*offset += sizeof(CMissionTextOffsets::Entry) * size;
}

void
AsciiToUnicode(const char *src, wchar *dst)
{
	while((*dst++ = (unsigned char)*src++) != '\0');
}

char*
UnicodeToAscii(wchar *src)
{
	static char aStr[256];
	int len;
	for(len = 0; *src != '\0' && len < 256-1; len++, src++)
#ifdef MORE_LANGUAGES
		if(*src < 128 || ((CGame::russianGame || CGame::japaneseGame) && *src < 256))
#else
		if(*src < 128)
#endif
			aStr[len] = *src;
		else
			aStr[len] = '#';
	aStr[len] = '\0';
	return aStr;
}

char*
UnicodeToAsciiForSaveLoad(wchar *src)
{
	static char aStr[256];
	int len;
	for(len = 0; *src != '\0' && len < 256-1; len++, src++)
		if(*src < 256)
			aStr[len] = *src;
		else
			aStr[len] = '#';
	aStr[len] = '\0';
	return aStr;
}

void
UnicodeStrcpy(wchar *dst, const wchar *src)
{
	while((*dst++ = *src++) != '\0');
}

int
UnicodeStrlen(const wchar *str)
{
	int len;
	for(len = 0; *str != '\0'; len++, str++);
	return len;
}

void
TextCopy(wchar *dst, const wchar *src)
{
	while((*dst++ = *src++) != '\0');
}