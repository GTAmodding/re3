#define WITHWINDOWS
#include "common.h"
#ifdef PS2_MENU
#include "crossplatform.h"
#include "MemoryCard.h"
#include "main.h"
#include "DMAudio.h"
#include "AudioScriptObject.h"
#include "Camera.h"
#include "CarGen.h"
#include "Cranes.h"
#include "Clock.h"
#include "MBlur.h"
#include "Date.h"
#include "Font.h"
#include "FileMgr.h"
#include "Game.h"
#include "GameLogic.h"
#include "Gangs.h"
#include "Garages.h"
#include "GenericGameStorage.h"
#include "Pad.h"
#include "Particle.h"
#include "ParticleObject.h"
#include "PathFind.h"
#include "PCSave.h"
#include "Phones.h"
#include "Pickups.h"
#include "PlayerPed.h"
#include "ProjectileInfo.h"
#include "Pools.h"
#include "Radar.h"
#include "Restart.h"
#include "Script.h"
#include "Stats.h"
#include "Streaming.h"
#include "Sprite2d.h"
#include "Timer.h"
#include "TimeStep.h"
#include "Weather.h"
#include "World.h"
#include "Zones.h"
#include "Frontend_PS2.h"

CMemoryCard TheMemoryCard;

char icon_one[16] = "slime1.ico";
char icon_two[16] = "slime2.ico";
char icon_three[16] = "slime3.ico";
char HostFileLocationOfIcons[64] = "icons\\";
char TheGameRootDirectory[64] = "/BESLES-50330GTA30000";

#define ReadDataFromBufferPointer(buf, to) memcpy(&to, buf, sizeof(to)); buf += align4bytes(sizeof(to));
#define WriteDataToBufferPointer(buf, from) memcpy(buf, &from, sizeof(from)); buf += align4bytes(sizeof(from));
	
static int
align4bytes(int32 size)
{
	return (size + 3) & 0xFFFFFFFC;
}

unsigned short ascii_table[3][2] =
{
	{ 0x824F, 0x30 }, /* 0-9  */
	{ 0x8260, 0x41 }, /* A-Z  */
	{ 0x8281, 0x61 }, /* a-z  */
};

unsigned short ascii_special[33][2] =
{
	{0x8140, 0x20}, /* " " */
	{0x8149, 0x21}, /* "!" */
	{0x8168, 0x22}, /* """ */
	{0x8194, 0x23}, /* "#" */
	{0x8190, 0x24}, /* "$" */
	{0x8193, 0x25}, /* "%" */
	{0x8195, 0x26}, /* "&" */
	{0x8166, 0x27}, /* "'" */
	{0x8169, 0x28}, /* "(" */
	{0x816A, 0x29}, /* ")" */
	{0x8196, 0x2A}, /* "*" */
	{0x817B, 0x2B}, /* "+" */
	{0x8143, 0x2C}, /* "," */
	{0x817C, 0x2D}, /* "-" */
	{0x8144, 0x2E}, /* "." */
	{0x815E, 0x2F}, /* "/" */
	{0x8146, 0x3A}, /* ":" */
	{0x8147, 0x3B}, /* ";" */
	{0x8171, 0x3C}, /* "<" */
	{0x8181, 0x3D}, /* "=" */
	{0x8172, 0x3E}, /* ">" */
	{0x8148, 0x3F}, /* "?" */
	{0x8197, 0x40}, /* "@" */
	{0x816D, 0x5B}, /* "[" */
	{0x818F, 0x5C}, /* "\" */
	{0x816E, 0x5D}, /* "]" */
	{0x814F, 0x5E}, /* "^" */
	{0x8151, 0x5F}, /* "_" */
	{0x8165, 0x60}, /* "`" */
	{0x816F, 0x7B}, /* "{" */
	{0x8162, 0x7C}, /* "|" */
	{0x8170, 0x7D}, /* "}" */
	{0x8150, 0x7E}, /* "~" */
};

unsigned short
Ascii2Sjis(unsigned char ascii_code)
{
	unsigned short sjis_code = 0; 
	unsigned char stmp; 
	unsigned char stmp2 = 0; 

	if ((ascii_code >= 0x20) && (ascii_code <= 0x2f)) 
		stmp2 = 1; 
	else 
	if ((ascii_code >= 0x30) && (ascii_code <= 0x39)) 
		stmp = 0; 
	else 
	if ((ascii_code >= 0x3a) && (ascii_code <= 0x40)) 
		stmp2 = 11; 
	else 
	if ((ascii_code >= 0x41) && (ascii_code <= 0x5a)) 
		stmp = 1; 
	else 
	if ((ascii_code >= 0x5b) && (ascii_code <= 0x60)) 
		stmp2 = 37; 
	else 
	if ((ascii_code >= 0x61) && (ascii_code <= 0x7a)) 
		stmp = 2; 
	else 
	if ((ascii_code >= 0x7b) && (ascii_code <= 0x7e)) 
		stmp2 = 63; 
	else { 
		printf("bad ASCII code 0x%x\n", ascii_code); 
		return(0); 
	} 
 
	if (stmp2) 
		sjis_code = ascii_special[ascii_code - 0x20 - (stmp2 - 1)][0]; 
	else 
		sjis_code = ascii_table[stmp][0] + ascii_code - ascii_table[stmp][1]; 
 
	return(sjis_code);
}

#if defined(GTA_PC)

extern "C"
{
	extern void HandleExit();
}
			
char CardCurDir[MAX_CARDS][260] = { "", "" };
char PCCardsPath[260];
char PCCardDir[MAX_CARDS][12] = { "memcard1", "memcard2" };

const char* _psGetUserFilesFolder();
void _psCreateFolder(LPCSTR path);

void
PCMCInit()
{
	sprintf(PCCardsPath, "%s", _psGetUserFilesFolder());
	
	char path[512];
	
	sprintf(path, "%s\\%s", PCCardsPath, PCCardDir[CARD_ONE]);
	_psCreateFolder(path);
	
	sprintf(path, "%s\\%s", PCCardsPath, PCCardDir[CARD_TWO]);
	_psCreateFolder(path);
}
#endif

CMemoryCardInfo::CMemoryCardInfo(void)
{
	type = 0;
	free = 0;
	format = 0;
	
	for ( int32 i = 0; i < sizeof(dir); i++ )
		dir[i] = '\0';
	
	strncpy(dir, TheGameRootDirectory, sizeof(dir) - 1);
}

int32
CMemoryCard::Init(void)
{
#if defined(PS2)
	if ( sceMcInit() == sceMcIniSucceed )
	{
		printf("Memory card initialsed\n");
		return RES_SUCCESS;
	}
	
	printf("Memory Card not being initialised\n");
	return RES_FAILED;
#else
	PCMCInit();
	printf("Memory card initialsed\n");
	return RES_SUCCESS;
#endif
}

CMemoryCard::CMemoryCard(void)
{
	_unk0 = 0;
	CurrentCard = CARD_ONE;
	Cards[CARD_ONE].port = 0;
	Cards[CARD_TWO].port = 1;
	
	for ( int32 i = 0; i < sizeof(_unkName3); i++ )
		_unkName3[i] = '\0';
	
	m_bWantToLoad = false;
	_bunk2 = false;
	_bunk7 = false;
	JustLoadedDontFadeInYet = false;
	StillToFadeOut = false;
	TimeStartedCountingForFade = 0;
	TimeToStayFadedBeforeFadeOut = 1750;
	b_FoundRecentSavedGameWantToLoad = false;
	
	char date[64];
	char time[64];
	char day[8];
	char month[8];
	char year[8];
	char hour[8];
	char minute[8];
	char second[8];
	
	strncpy(date, "Oct  7 2001", 62);
	strncpy(time, "15:48:32",    62);
	
	strncpy(month, date, 3);
	month[3] = '\0';
	
	strncpy(day, &date[4], 2);
	day[2] = '\0';
	
	strncpy(year, &date[7], 4);
	year[4] = '\0';
	
	strncpy(hour, time, 2);
	hour[2] = '\0';
	
	strncpy(minute, &time[3], 2);
	minute[2] = '\0';
	
	strncpy(second, &time[6], 2);
	second[2] = '\0';
	
	
	#define _CMP(m) strncmp(month, m, sizeof(m)-1)
	
	if ( !_CMP("Jan") ) CompileDateAndTime.m_nMonth = 1;
	else
	if ( !_CMP("Feb") ) CompileDateAndTime.m_nMonth = 2;
	else
	if ( !_CMP("Mar") ) CompileDateAndTime.m_nMonth = 3;
	else
	if ( !_CMP("Apr") ) CompileDateAndTime.m_nMonth = 4;
	else
	if ( !_CMP("May") ) CompileDateAndTime.m_nMonth = 5;
	else
	if ( !_CMP("Jun") ) CompileDateAndTime.m_nMonth = 6;
	else
	if ( !_CMP("Jul") ) CompileDateAndTime.m_nMonth = 7;
	else
	if ( !_CMP("Aug") ) CompileDateAndTime.m_nMonth = 8;
	else
	if ( !_CMP("Oct") ) CompileDateAndTime.m_nMonth = 9;  // BUG: oct and sep is swapped here
	else
	if ( !_CMP("Sep") ) CompileDateAndTime.m_nMonth = 10;
	else
	if ( !_CMP("Nov") ) CompileDateAndTime.m_nMonth = 11;
	else
	if ( !_CMP("Dec") ) CompileDateAndTime.m_nMonth = 12;
	
	#undef _CMP
	
	CompileDateAndTime.m_nDay    = atoi(day);
	CompileDateAndTime.m_nYear   = atoi(year);
	CompileDateAndTime.m_nHour   = atoi(hour);
	CompileDateAndTime.m_nMinute = atoi(minute);
	CompileDateAndTime.m_nSecond = atoi(second);
}

int32
CMemoryCard::RestoreForStartLoad(void)
{
	uint8 buf[30];
	
	int32 file = OpenMemCardFileForReading(CurrentCard, LoadFileName);
	if ( nError != NO_ERR_SUCCESS )
		return RES_FAILED;
	
	ReadFromMemCard(file, buf, sizeof(buf) - 1);
	if ( nError != NO_ERR_SUCCESS )
		return RES_FAILED;
	
	uint8 *pBuf = buf + sizeof(uint32) + sizeof(uint32);
	ReadDataFromBufferPointer(pBuf, CGame::currLevel);
	ReadDataFromBufferPointer(pBuf, TheCamera.GetMatrix().GetPosition().x);
	ReadDataFromBufferPointer(pBuf, TheCamera.GetMatrix().GetPosition().y);
	ReadDataFromBufferPointer(pBuf, TheCamera.GetMatrix().GetPosition().z);
	
	if ( CGame::currLevel != LEVEL_INDUSTRIAL )
		CStreaming::RemoveBigBuildings(LEVEL_INDUSTRIAL);
	
	if ( CGame::currLevel != LEVEL_COMMERCIAL )
		CStreaming::RemoveBigBuildings(LEVEL_COMMERCIAL);
	
	if ( CGame::currLevel != LEVEL_SUBURBAN )
		CStreaming::RemoveBigBuildings(LEVEL_SUBURBAN);
	
	CStreaming::RemoveIslandsNotUsed(CGame::currLevel);
	CCollision::SortOutCollisionAfterLoad();
	CStreaming::RequestBigBuildings(CGame::currLevel);
	CStreaming::LoadAllRequestedModels(false);
	CStreaming::HaveAllBigBuildingsLoaded(CGame::currLevel);
	CGame::TidyUpMemory(true, false);
	
	CloseMemCardFile(file);
	
	if ( nError != NO_ERR_SUCCESS )
		return RES_FAILED;
	
	return RES_SUCCESS;
}

int32
CMemoryCard::LoadSavedGame(void)
{
	CheckSum = 0;
	CDate date;
	
	int32 saveSize = 0;
	uint32 size = 0;
	
	int32 oldLang = CMenuManager::m_PrefsLanguage;
	
	CPad::ResetCheats();
	
	ChangeDirectory(CurrentCard, Cards[CurrentCard].dir);
	
	if ( nError != NO_ERR_SUCCESS )
		return RES_FAILED;
	
	int32 file = OpenMemCardFileForReading(CurrentCard, LoadFileName);
	
	if ( nError != NO_ERR_SUCCESS )
		return RES_FAILED;
	
	
	#define LoadSaveDataBlock()\
		do {\
			ReadFromMemCard(file, &size, sizeof(size)); \
			if ( nError != NO_ERR_SUCCESS ) return RES_FAILED; \
			size = align4bytes(size); \
			ReadFromMemCard(file, work_buff, size); \
			if ( nError != NO_ERR_SUCCESS ) return RES_FAILED; \
			buf = work_buff; \
		} while (0)
	
	uint8 *buf;
	
	LoadSaveDataBlock();
	
	ReadDataFromBufferPointer(buf, saveSize);
	ReadDataFromBufferPointer(buf, CGame::currLevel);
	ReadDataFromBufferPointer(buf, TheCamera.GetMatrix().GetPosition().x);
	ReadDataFromBufferPointer(buf, TheCamera.GetMatrix().GetPosition().y);
	ReadDataFromBufferPointer(buf, TheCamera.GetMatrix().GetPosition().z);
	ReadDataFromBufferPointer(buf, CClock::ms_nMillisecondsPerGameMinute);
	ReadDataFromBufferPointer(buf, CClock::ms_nLastClockTick);
	ReadDataFromBufferPointer(buf, CClock::ms_nGameClockHours);
	ReadDataFromBufferPointer(buf, CClock::ms_nGameClockMinutes);
	ReadDataFromBufferPointer(buf, CPad::GetPad(0)->Mode);
	ReadDataFromBufferPointer(buf, CTimer::m_snTimeInMilliseconds);
	ReadDataFromBufferPointer(buf, CTimer::ms_fTimeScale);
	ReadDataFromBufferPointer(buf, CTimer::ms_fTimeStep);
	ReadDataFromBufferPointer(buf, CTimer::ms_fTimeStepNonClipped);
	ReadDataFromBufferPointer(buf, CTimer::m_FrameCounter);
	ReadDataFromBufferPointer(buf, CTimeStep::ms_fTimeStep);
	ReadDataFromBufferPointer(buf, CTimeStep::ms_fFramesPerUpdate);
	ReadDataFromBufferPointer(buf, CTimeStep::ms_fTimeScale);
	ReadDataFromBufferPointer(buf, CWeather::OldWeatherType);
	ReadDataFromBufferPointer(buf, CWeather::NewWeatherType);
	ReadDataFromBufferPointer(buf, CWeather::ForcedWeatherType);
	ReadDataFromBufferPointer(buf, CWeather::InterpolationValue);
	ReadDataFromBufferPointer(buf, CMenuManager::m_PrefsMusicVolume);
	ReadDataFromBufferPointer(buf, CMenuManager::m_PrefsSfxVolume);
	ReadDataFromBufferPointer(buf, CMenuManager::m_PrefsControllerConfig);
	ReadDataFromBufferPointer(buf, CMenuManager::m_PrefsUseVibration);
	ReadDataFromBufferPointer(buf, CMenuManager::m_PrefsStereoMono);
	ReadDataFromBufferPointer(buf, CMenuManager::m_PrefsRadioStation);
	ReadDataFromBufferPointer(buf, CMenuManager::m_PrefsBrightness);
	ReadDataFromBufferPointer(buf, CMenuManager::m_PrefsShowTrails);
	ReadDataFromBufferPointer(buf, CMenuManager::m_PrefsShowSubtitles);
	ReadDataFromBufferPointer(buf, CMenuManager::m_PrefsLanguage);
	ReadDataFromBufferPointer(buf, CMenuManager::m_PrefsUseWideScreen);
	ReadDataFromBufferPointer(buf, CPad::GetPad(0)->Mode);
#ifdef PS2
	ReadDataFromBufferPointer(buf, BlurOn);
#else
	ReadDataFromBufferPointer(buf, CMBlur::BlurOn);
#endif
	ReadDataFromBufferPointer(buf, date.m_nSecond);
	ReadDataFromBufferPointer(buf, date.m_nMinute);
	ReadDataFromBufferPointer(buf, date.m_nHour);
	ReadDataFromBufferPointer(buf, date.m_nDay);
	ReadDataFromBufferPointer(buf, date.m_nMonth);
	ReadDataFromBufferPointer(buf, date.m_nYear);
	ReadDataFromBufferPointer(buf, CWeather::WeatherTypeInList);
	ReadDataFromBufferPointer(buf, TheCamera.CarZoomIndicator);
	ReadDataFromBufferPointer(buf, TheCamera.PedZoomIndicator);
	
	if ( date > CompileDateAndTime )
		;
	else
	if ( date < CompileDateAndTime )
		;
	
	#define ReadDataFromBlock(load_func)\
		do {\
			ReadDataFromBufferPointer(buf, size);\
			load_func(buf, size);\
			size = align4bytes(size);\
			buf += size;\
		} while (0)
	
	

	printf("Loading Scripts \n");
	ReadDataFromBlock(CTheScripts::LoadAllScripts);
	
	printf("Loading PedPool \n");
	ReadDataFromBlock(CPools::LoadPedPool);
	
	printf("Loading Garages \n");
	ReadDataFromBlock(CGarages::Load);
	
	printf("Loading Vehicles \n");
	ReadDataFromBlock(CPools::LoadVehiclePool);
	
	LoadSaveDataBlock();
	
	CProjectileInfo::RemoveAllProjectiles();
	CObject::DeleteAllTempObjects();
	
	printf("Loading Objects \n");
	ReadDataFromBlock(CPools::LoadObjectPool);
	
	printf("Loading Paths \n");
	ReadDataFromBlock(ThePaths.Load);
	
	printf("Loading Cranes \n");
	ReadDataFromBlock(CCranes::Load);
	
	LoadSaveDataBlock();
	
	printf("Loading Pickups \n");
	ReadDataFromBlock(CPickups::Load);
	
	printf("Loading Phoneinfo \n");
	ReadDataFromBlock(gPhoneInfo.Load);
	
	printf("Loading Restart \n");
	ReadDataFromBlock(CRestart::LoadAllRestartPoints);
	
	printf("Loading Radar Blips \n");
	ReadDataFromBlock(CRadar::LoadAllRadarBlips);
	
	printf("Loading Zones \n");
	ReadDataFromBlock(CTheZones::LoadAllZones);
	
	printf("Loading Gang Data \n");
	ReadDataFromBlock(CGangs::LoadAllGangData);
	
	printf("Loading Car Generators \n");
	ReadDataFromBlock(CTheCarGenerators::LoadAllCarGenerators);
	
	printf("Loading Particles \n");
	ReadDataFromBlock(CParticleObject::LoadParticle);
	
	printf("Loading AudioScript Objects \n");
	ReadDataFromBlock(cAudioScriptObject::LoadAllAudioScriptObjects);
	
	printf("Loading Player Info \n");
	ReadDataFromBlock(CWorld::Players[CWorld::PlayerInFocus].LoadPlayerInfo);
	
	printf("Loading Stats \n");
	ReadDataFromBlock(CStats::LoadStats);
	
	printf("Loading Streaming Stuff \n");
	ReadDataFromBlock(CStreaming::MemoryCardLoad);
	
	printf("Loading PedType Stuff \n");
	ReadDataFromBlock(CPedType::Load);
	
	#undef LoadSaveDataBlock
	#undef ReadDataFromBlock
	
	FrontEndMenuManager.SetSoundLevelsForMusicMenu();
	FrontEndMenuManager.InitialiseMenuContentsAfterLoadingGame();
	
	CloseMemCardFile(file);
	
	if ( nError != NO_ERR_SUCCESS )
		return RES_FAILED;
	
	if ( oldLang != CMenuManager::m_PrefsLanguage )
	{
		TheText.Unload();
		TheText.Load();
	}
	
	JustLoadedDontFadeInYet = true;
	StillToFadeOut = true;
	
	CTheScripts::Process();
	
	printf("Game sucessfully loaded \n");
	
	return RES_SUCCESS;
}

int32
CMemoryCard::CheckCardInserted(int32 cardID)
{
#if defined(PS2)
	int cmd  = sceMcFuncNoCardInfo;
	int type = sceMcTypeNoCard;
	
	CTimer::Stop();
	
	while ( sceMcGetInfo(Cards[cardID].port, 0, &type, 0, 0) != sceMcResSucceed )
		;
	
	int result;
	sceMcSync(0, &cmd, &result);
	
	if ( type == sceMcTypePS2 )
	{
		if ( result == sceMcResChangedCard || result == sceMcResSucceed )
		{
			nError = NO_ERR_SUCCESS;
			return nError;
		}
		else if ( result == sceMcResNoFormat )
		{
			nError = ERR_NOFORMAT;
			return nError;
		}
	}
	
	printf("Memory card %i not present\n", cardID);
	
	nError = ERR_NONE;
	return nError;
#else
	nError = NO_ERR_SUCCESS;
	return nError;
#endif
}

int32
CMemoryCard::PopulateCardFlags(int32 cardID, bool bSlotFlag, bool bTypeFlag, bool bFreeFlag, bool bFormatFlag)
{
#if defined(PS2)
	int cmd = sceMcFuncNoCardInfo;
	int type = sceMcTypeNoCard;
	int free = 0;
	int format = 0;
	
	CTimer::Stop();
	
	Cards[cardID].type = 0;
	Cards[cardID].free = 0;
	Cards[cardID].format = 0;
	
	while ( sceMcGetInfo(Cards[cardID].port, 0, &type, &free, &format) != sceMcResSucceed )
		;
	
	int result;
	sceMcSync(0, &cmd, &result);
	
	if ( type == sceMcTypePS2 )
	{
		if ( result == sceMcResChangedCard || result == sceMcResSucceed )
		{
			if ( bSlotFlag )
				Cards[cardID].slot = 0;
			
			//if ( bTypeFlag )
				Cards[cardID].type = type;
			
			if ( bFreeFlag )
				Cards[cardID].free = free;
			
			if ( bFormatFlag )
				Cards[cardID].format = format;
			
			printf("Memory card %i present\n", cardID);
			
			nError = NO_ERR_SUCCESS;
			return nError;
		}
		else if ( result == sceMcResNoFormat )
		{
			nError = ERR_NOFORMAT;
			return nError;
		}
	}
	
	printf("Memory card %i not present\n", cardID);
	
	nError = ERR_NONE;
	return nError;
#else
	CTimer::Stop();
	
	Cards[cardID].type = 0;
	Cards[cardID].free = 0;
	Cards[cardID].format = 0;
	
	if ( bSlotFlag )
		Cards[cardID].slot = 0;
			
	//if ( bTypeFlag )
		Cards[cardID].type = 0;
			
	if ( bFreeFlag )
		Cards[cardID].free = 1024 * 1024 * 4;
			
	if ( bFormatFlag )
		Cards[cardID].format = 0;
	
	printf("Memory card %i present\n", cardID);
	
	nError = NO_ERR_SUCCESS;
	return nError;
#endif
}

int32
CMemoryCard::FormatCard(int32 cardID)
{
	CTimer::Stop();
	
#if defined(PS2)
	int cmd = sceMcFuncNoFormat;
	
	int32 r = CheckCardInserted(cardID);
	if ( r == NO_ERR_SUCCESS )
	{
		while ( sceMcFormat(Cards[cardID].port, 0) != sceMcResSucceed )
			;
		
		int result;
		sceMcSync(0, &cmd, &result);
		
		if ( result < sceMcResSucceed )
		{
			printf("Memory card %i could not be formatted\n", cardID);
			
			nError = ERR_FORMATFAILED;
			return nError;
		}
		
		printf("Memory card %i present and formatted\n", cardID);
			
		nError = NO_ERR_SUCCESS;
		return nError;
	}

	return r;
#else
	printf("Memory card %i present and formatted\n", cardID);

	nError = NO_ERR_SUCCESS;
	return nError;
#endif
}

int32
CMemoryCard::PopulateFileTable(int32 cardID)
{
	CTimer::Stop();
	
#if defined (PS2)
	int cmd = sceMcFuncNoGetDir;
	
	ClearFileTableBuffer(cardID);
	
	while ( sceMcGetDir(Cards[cardID].port, 0, "*", 0, ARRAY_SIZE(Cards[cardID].table), Cards[cardID].table) != sceMcResSucceed )
		;
		
	int result;
	sceMcSync(0, &cmd, &result);
	
	if ( result >= sceMcResSucceed )
	{
		printf("Memory card %i present PopulateFileTables function successfull \n", cardID);
		
		nError = NO_ERR_SUCCESS;
		return nError;
	}
	
	if ( result == sceMcResNoFormat )
	{
		printf("Memory card %i PopulateFileTables function successfull. MemoryCard not Formatted  \n", cardID);
		
		nError = ERR_NOFORMAT;
		return nError;
	}
	
	if ( result == sceMcResNoEntry )
	{
		printf("Memory card %i  PopulateFileTables function unsuccessfull. Path does not exist  \n", cardID);
		
		nError = ERR_FILETABLENOENTRY;
		return nError;
	}
	
	printf("Memory card %i not Present\n", cardID);
		
	nError = ERR_NONE;
	return nError;
#else
	ClearFileTableBuffer(cardID);

	char path[512];
	sprintf(path, "%s\\%s\\%s\\*", PCCardsPath, PCCardDir[cardID], CardCurDir[cardID]);
	
	memset(Cards[cardID].table, 0, sizeof(Cards[cardID].table));
	WIN32_FIND_DATA fd; HANDLE hFind; int32 num = 0;
	if ( (hFind = FindFirstFile(path, &fd)) == INVALID_HANDLE_VALUE )
	{
		printf("Memory card %i not Present\n", cardID);
		nError = ERR_NONE;
		return nError;
	}
	do
	{
		SYSTEMTIME st;
		FileTimeToSystemTime(&fd.ftCreationTime, &st);
		Cards[cardID].table[num]._Create.Sec = st.wSecond;Cards[cardID].table[num]._Create.Min = st.wMinute;Cards[cardID].table[num]._Create.Hour = st.wHour;Cards[cardID].table[num]._Create.Day = st.wDay;Cards[cardID].table[num]._Create.Month = st.wMonth;Cards[cardID].table[num]._Create.Year = st.wYear;
		FileTimeToSystemTime(&fd.ftLastWriteTime, &st);
		Cards[cardID].table[num]._Modify.Sec = st.wSecond;Cards[cardID].table[num]._Modify.Min = st.wMinute;Cards[cardID].table[num]._Modify.Hour = st.wHour;Cards[cardID].table[num]._Modify.Day = st.wDay;Cards[cardID].table[num]._Modify.Month = st.wMonth;Cards[cardID].table[num]._Modify.Year = st.wYear;
		Cards[cardID].table[num].FileSizeByte = fd.nFileSizeLow;
		strncpy((char *)Cards[cardID].table[num].EntryName, fd.cFileName, sizeof(Cards[cardID].table[num].EntryName) - 1);
		num++;
	} while( FindNextFile(hFind, &fd) && num < ARRAY_SIZE(Cards[cardID].table) );
	FindClose(hFind);
	
	//todo errors
	
	printf("Memory card %i present PopulateFileTables function successfull \n", cardID);

	nError = NO_ERR_SUCCESS;
	return nError;
#endif
}

int32
CMemoryCard::CreateRootDirectory(int32 cardID)
{
	CTimer::Stop();
#if defined(PS2)
	int cmd = sceMcFuncNoMkdir;
	
	while ( sceMcMkdir(Cards[cardID].port, 0, Cards[cardID].dir) != sceMcResSucceed )
		;
	
	int result;
	sceMcSync(0, &cmd, &result);
	
	if ( result == sceMcResSucceed )
	{
		printf("Memory card %i present. RootDirectory Created\n", cardID);
		
		nError = NO_ERR_SUCCESS;
		return nError;
	}
	
	if ( result == sceMcResNoFormat )
	{
		printf("Memory card %i RootDirectory not created card unformatted\n", cardID);
		
		nError = ERR_NOFORMAT;
		return nError;
	}
	
	if ( result == sceMcResFullDevice )
	{
		printf("Memory card %i RootDirectory not created due to insufficient memory card capacity\n", cardID);
		
		nError = ERR_DIRFULLDEVICE;
		return nError;
	}
	
	if ( result == sceMcResNoEntry )
	{
		printf("Memory card %i RootDirectory not created due to problem with pathname\n", cardID);
		
		nError = ERR_DIRBADENTRY;
		return nError;
	}
	
	printf("Memory card %i not present so RootDirectory not created \n", cardID);
		
	nError = ERR_NONE;
	return nError;
#else
	char path[512];
	sprintf(path, "%s\\%s\\%s", PCCardsPath, PCCardDir[cardID], Cards[cardID].dir);
	_psCreateFolder(path);
	
	printf("Memory card %i present. RootDirectory Created\n", cardID);

	nError = NO_ERR_SUCCESS;
	return nError;	
#endif
}

int32
CMemoryCard::ChangeDirectory(int32 cardID, char *dir)
{
	CTimer::Stop();
	
#if defined(PS2)
	int cmd = sceMcFuncNoChDir;
	
	while ( sceMcChdir(Cards[cardID].port, 0, dir, 0) != sceMcResSucceed )
		;
	
	int result;
	sceMcSync(0, &cmd, &result);
	
	if ( result == sceMcResSucceed )
	{
		printf("Memory Card %i. Changed to the directory %s \n", cardID, dir);
		
		nError = NO_ERR_SUCCESS;
		return nError;
	}
	
	if ( result == sceMcResNoFormat )
	{
		printf("Memory card %i. Couldn't change to the directory %s. MemoryCard not Formatted  \n", cardID, dir);
		
		nError = ERR_NOFORMAT;
		return nError;
	}
	
	if ( result == sceMcResNoEntry )
	{
		printf("Memory card %i  Couldn't change to the directory %s. Path does not exist  \n", cardID, dir);
		
		nError = ERR_DIRNOENTRY;
		return nError;
	}
	
	printf("Memory card %i not Present. So could not change to directory %s.\n", cardID, dir);
	
	nError = ERR_NONE;
	return nError;
#else
	
	if ( !strcmp(dir, "/" ) )
	{
		strncpy(CardCurDir[cardID], dir, sizeof(CardCurDir[cardID]) - 1);
		printf("Memory Card %i. Changed to the directory %s \n", cardID, dir);
		nError = NO_ERR_SUCCESS;
		return nError;
	}

	char path[512];
	sprintf(path, "%s\\%s\\%s", PCCardsPath, PCCardDir[cardID], dir);
	
	WIN32_FIND_DATA fd; HANDLE hFind;
	if ( (hFind = FindFirstFile(path, &fd)) == INVALID_HANDLE_VALUE )
	{
		printf("Memory card %i  Couldn't change to the directory %s. Path does not exist  \n", cardID, dir);
		
		nError = ERR_DIRNOENTRY;
		return nError;
	}

	FindClose(hFind);

	strncpy(CardCurDir[cardID], dir, sizeof(CardCurDir[cardID]) - 1);
	printf("Memory Card %i. Changed to the directory %s \n", cardID, dir);
	nError = NO_ERR_SUCCESS;
	return nError;
#endif
}

int32
CMemoryCard::CreateIconFiles(int32 cardID, char *icon_one, char *icon_two, char *icon_three)
{
#if defined(PS2)
	sceMcIconSys icon;
	static sceVu0IVECTOR bgcolor[4] = {
		{ 0x80,    0,    0, 0 },
		{    0, 0x80,    0, 0 },
		{    0,    0, 0x80, 0 },
		{ 0x80, 0x80, 0x80, 0 },
	};
	static sceVu0FVECTOR lightdir[3] = {
		{ 0.5, 0.5, 0.5, 0.0 },
		{ 0.0,-0.4,-0.1, 0.0 },
		{-0.5,-0.5, 0.5, 0.0 },
	};
	static sceVu0FVECTOR lightcol[3] = {
		{ 0.48, 0.48, 0.03, 0.00 },
		{ 0.50, 0.33, 0.20, 0.00 },
		{ 0.14, 0.14, 0.38, 0.00 },
	};
	static sceVu0FVECTOR ambient = { 0.50, 0.50, 0.50, 0.00 };
	char head[8]  = "PS2D";
	char title[8] = "GTA3";
	
	memset(&icon, 0, sizeof(icon));
	
	memcpy(icon.BgColor,    bgcolor,  sizeof(bgcolor));
	memcpy(icon.LightDir,   lightdir, sizeof(lightdir));
	memcpy(icon.LightColor, lightcol, sizeof(lightcol));
	memcpy(icon.Ambient,    ambient,  sizeof(ambient));
	
	icon.OffsLF = 24;
	icon.TransRate = 0x60;
	
	unsigned short *titleName = (unsigned short *)icon.TitleName;
	
	uint32 titlec = 0;
	while ( titlec < strlen(title) )
	{
		unsigned short sjis = Ascii2Sjis(title[titlec]);
		titleName[titlec] = (sjis << 8) | (sjis >> 8);
		titlec++;
	}
	
	titleName[titlec] = L'\0';

	char icon1[80];
	char icon2[80];
	char icon3[80];
	
	strncpy(icon1, icon_one,   sizeof(icon1) - 1);
	strncpy(icon2, icon_two,   sizeof(icon2) - 1);
	strncpy(icon3, icon_three, sizeof(icon3) - 1);
	
	strncpy((char *)icon.FnameView, icon1, sizeof(icon.FnameView) - 1);
	strncpy((char *)icon.FnameCopy, icon2, sizeof(icon.FnameCopy) - 1);
	strncpy((char *)icon.FnameDel,  icon3, sizeof(icon.FnameDel) - 1);
	strncpy((char *)icon.Head, head, sizeof(icon.Head));
	
	int32 iconFile = CreateMemCardFileReadWrite(Cards[cardID].port, "icon.sys");
	
	if ( nError != NO_ERR_SUCCESS )
		return RES_FAILED;
	
	WritetoMemCard(iconFile, &icon, sizeof(icon));
	
	if ( nError != NO_ERR_SUCCESS )
		return RES_FAILED;
	
	CloseMemCardFile(iconFile);
	
	if ( nError != NO_ERR_SUCCESS )
		return RES_FAILED;
	
	if ( LoadIconFiles(Cards[cardID].port, icon_one, icon_two, icon_three) == RES_SUCCESS )
	{
		printf("All Icon files Created and loaded. \n");
		return RES_SUCCESS;
	}
	
	printf("Could not load all the icon files \n");
	
	return RES_FAILED;
#else
	return RES_SUCCESS;
#endif
}

int32
CMemoryCard::LoadIconFiles(int32 cardID, char *icon_one, char *icon_two, char *icon_three)
{
#if defined(PS2)
	const uint32 size = 50968;
	uint8 *data = new uint8[size];
	
	char icon1_path[80];
	char icon2_path[80];
	char icon3_path[80];
	char icon1[32];
	char icon2[32];
	char icon3[32];
	
	strncpy(icon1, icon_one, sizeof(icon1) - 1);
	strncpy(icon2, icon_two, sizeof(icon2) - 1);
	strncpy(icon3, icon_three, sizeof(icon3) - 1);
	
	int hostlen = strlen(HostFileLocationOfIcons);

	strncpy(icon1_path, HostFileLocationOfIcons, sizeof(icon1_path) - 1);
	strncpy(icon2_path, HostFileLocationOfIcons, sizeof(icon2_path) - 1);
	strncpy(icon3_path, HostFileLocationOfIcons, sizeof(icon3_path) - 1);
	
	strncpy(icon1_path+hostlen, icon_one,   sizeof(icon1_path) - 1 - hostlen);
	strncpy(icon2_path+hostlen, icon_two,   sizeof(icon2_path) - 1 - hostlen);
	strncpy(icon3_path+hostlen, icon_three, sizeof(icon3_path) - 1 - hostlen);
	
	// ico1 copy
	int32 ico1file = CFileMgr::OpenFile(icon1_path);
	CFileMgr::Read(ico1file, (char *)data, size);
	CFileMgr::CloseFile(ico1file);
	
	int32 ico1mc = CreateMemCardFileReadWrite(Cards[cardID].port, icon1);
	
	if ( nError != NO_ERR_SUCCESS )
	{
		delete [] data;
		return RES_FAILED;
	}
	
	WritetoMemCard(ico1mc, data, size);
	
	if ( nError != NO_ERR_SUCCESS )
	{
		delete [] data;
		return RES_FAILED;
	}
	
	CloseMemCardFile(ico1mc);
	
	if ( nError != NO_ERR_SUCCESS )
	{
		delete [] data;
		return RES_FAILED;
	}
	
	// ico2 copy
	int32 ico2file = CFileMgr::OpenFile(icon2_path);
	CFileMgr::Read(ico2file, (char *)data, size);
	CFileMgr::CloseFile(ico2file);
	
	int32 ico2mc = CreateMemCardFileReadWrite(Cards[cardID].port, icon2);
	
	if ( nError != NO_ERR_SUCCESS )
	{
		delete [] data;
		return RES_FAILED;
	}
	
	WritetoMemCard(ico2mc, data, size);
	
	if ( nError != NO_ERR_SUCCESS )
	{
		delete [] data;
		return RES_FAILED;
	}
	
	CloseMemCardFile(ico2mc);
	
	if ( nError != NO_ERR_SUCCESS )
	{
		delete [] data;
		return RES_FAILED;
	}
	
	// ico3 copy
	int32 ico3file = CFileMgr::OpenFile(icon3_path);
	CFileMgr::Read(ico3file, (char *)data, size);
	CFileMgr::CloseFile(ico3file);
	
	int32 ico3mc = CreateMemCardFileReadWrite(Cards[cardID].port, icon3);
	
	if ( nError != NO_ERR_SUCCESS )
	{
		delete [] data;
		return RES_FAILED;
	}
	
	WritetoMemCard(ico3mc, data, size);
	
	if ( nError != NO_ERR_SUCCESS )
	{
		delete [] data;
		return RES_FAILED;
	}
	
	CloseMemCardFile(ico3mc);
	
	if ( nError != NO_ERR_SUCCESS )
	{
		delete [] data;
		return RES_FAILED;
	}
	
	delete [] data;
	
	return RES_SUCCESS;
#else
	return RES_SUCCESS;
#endif
}

int32
CMemoryCard::CloseMemCardFile(int32 file)
{
	CTimer::Stop();
	
#if defined(PS2)
	int cmd = sceMcFuncNoClose;
	
	while ( sceMcClose(file) != sceMcResSucceed )
		;
	
	int result;
	sceMcSync(0, &cmd, &result);
	
	if ( result == sceMcResSucceed )
	{
		printf("File %i closed\n", file);
	
		nError = NO_ERR_SUCCESS;
		return nError;
	}
	
	if ( result == sceMcResNoFormat )
	{
		printf("Memory Card is Unformatted");
	
		nError = ERR_NOFORMAT;
		return nError;
	}
	
	if ( result == sceMcResNoEntry )
	{
		printf("Memory Card File Handle %i has not been opened", file);
	
		nError = ERR_OPENNOENTRY;
		return nError;
	}
	
	nError = ERR_NONE;
	return nError;
#else
	CFileMgr::CloseFile(file);
	printf("File %i closed\n", file);
	
	nError = NO_ERR_SUCCESS;
	return nError;	
#endif
}

int32
CMemoryCard::CreateMemCardFileReadWrite(int32 cardID, char *filename)
{
	CTimer::Stop();
	
#if defined(PS2)
	int cmd = sceMcFuncNoOpen;
	
	char buff[255];
	
	strncpy(buff, filename, sizeof(buff));
	
	while ( sceMcOpen(Cards[cardID].port, 0, buff, SCE_RDWR|SCE_CREAT) != sceMcResSucceed )
		;
	
	int result;
	sceMcSync(0, &cmd, &result);
	
	if ( result >= sceMcResSucceed )
	{
		printf("%s File Created for MemoryCard. Its File handle is %i. \n", buff, result);
	
		nError = NO_ERR_SUCCESS;
		return result;
	}
	
	if ( result == sceMcResNoFormat )
	{
		nError = ERR_NOFORMAT;
		return nError;
	}
	
	if ( result == sceMcResFullDevice )
	{
		nError = ERR_FILEFULLDEVICE;
		return nError;
	}
	
	if ( result == sceMcResNoEntry )
	{
		nError = ERR_FILENOPATHENTRY;
		return nError;
	}
	
	if ( result == sceMcResDeniedPermit )
	{
		nError = ERR_FILEDENIED;
		return nError;
	}
	
	if ( result == sceMcResUpLimitHandle )
	{
		nError = ERR_FILEUPLIMIT;
		return nError;
	}
	
	printf("File %s not created on memory card.\n", buff);
	
	return ERR_NONE;
#else
	char path[512];
	sprintf(path, "%s\\%s\\%s\\%s", PCCardsPath, PCCardDir[cardID], CardCurDir[cardID], filename);
	int32 file = CFileMgr::OpenFile(path, "wb+");
	if (file == 0)
	{
		sprintf(path, "%s\\%s\\%s", PCCardsPath, PCCardDir[cardID], filename);
		file = CFileMgr::OpenFile(path, "wb+");
	}

	if ( file )
	{
		printf("%s File Created for MemoryCard. Its File handle is %i. \n", filename, file);
		nError = NO_ERR_SUCCESS;
		return file;
	}
	
	printf("File %s not created on memory card.\n", path);
	
	nError = ERR_NONE;
	return 0;
#endif
}

int32
CMemoryCard::OpenMemCardFileForReading(int32 cardID, char *filename)
{
	CTimer::Stop();
	
#if defined(PS2)
	int cmd = sceMcFuncNoOpen;
	
	char buff[255];
	
	strncpy(buff, filename, sizeof(buff));
	
	while ( sceMcOpen(Cards[cardID].port, 0, buff, SCE_RDONLY) != sceMcResSucceed )
		;
	
	int result;
	sceMcSync(0, &cmd, &result);
	
	if ( result >= sceMcResSucceed )
	{
		printf("%s File Created for MemoryCard. Its File handle is %i. \n", buff, result);
	
		nError = NO_ERR_SUCCESS;
		return result;
	}
	
	if ( result == sceMcResNoFormat )
	{
		nError = ERR_NOFORMAT;
		return nError;
	}
	
	if ( result == sceMcResFullDevice )
	{
		nError = ERR_FILEFULLDEVICE;
		return nError;
	}
	
	if ( result == sceMcResNoEntry )
	{
		nError = ERR_FILENOPATHENTRY;
		return nError;
	}
	
	if ( result == sceMcResDeniedPermit )
	{
		nError = ERR_FILEDENIED;
		return nError;
	}
	
	if ( result == sceMcResUpLimitHandle )
	{
		nError = ERR_FILEUPLIMIT;
		return nError;
	}
	
	printf("File %s not created on memory card.\n", buff);
	nError = ERR_NONE;
	return nError;
#else	
	char path[512];
	sprintf(path, "%s\\%s\\%s\\%s", PCCardsPath, PCCardDir[cardID], CardCurDir[cardID], filename);
	int32 file = CFileMgr::OpenFile(path, "rb");
	if (file == 0)
	{
		sprintf(path, "%s\\%s\\%s", PCCardsPath, PCCardDir[cardID], filename);
		file = CFileMgr::OpenFile(path, "rb");
	}

	if ( file )
	{
		printf("%s File Created for MemoryCard. Its File handle is %i. \n", filename, file);
		nError = NO_ERR_SUCCESS;
		return file;
	}
	
	printf("File %s not created on memory card.\n", path);
	nError = ERR_NONE;
	return 0;
#endif
}

int32
CMemoryCard::ReadFromMemCard(int32 file, void *buff, int32 size)
{
	CTimer::Stop();
	
#if defined(PS2)
	int cmd = sceMcFuncNoRead;
	
	while ( sceMcRead(file, buff, size) != sceMcResSucceed )
		;
	
	int result;
	sceMcSync(0, &cmd, &result);
	
	if ( result >= sceMcResSucceed )
	{
		if ( size >= result )
		{
			printf("%i Bytes Read for Filehandle %i \n", result, file);
			
			nError = NO_ERR_SUCCESS;
			return result;
		}
	}
	
	if ( result == sceMcResNoFormat )
	{
		nError = ERR_NOFORMAT;
		return nError;
	}
	
	if ( result == sceMcResNoEntry )
	{
		nError = ERR_READNOENTRY;
		return nError;
	}
	
	if ( result == sceMcResDeniedPermit )
	{
		nError = ERR_READDENIED;
		return nError;
	}
	
	printf("No Bytes Read for Filehandle %i \n", file);
			
	nError = ERR_NONE;
	return result;
#else		
	int32 s = CFileMgr::Read(file, (const char *)buff, size);
	if ( s == size )
	{
		printf("%i Bytes Read for Filehandle %i \n", s, file);
		
		nError = NO_ERR_SUCCESS;
		return s;
	}
	
	printf("No Bytes Read for Filehandle %i \n", file);
	
	nError = ERR_NONE;
	return s;
#endif
}

int32
CMemoryCard::DeleteMemoryCardFile(int32 cardID, char *filename)
{
	CTimer::Stop();
	
#if defined(PS2)
	int cmd = sceMcFuncNoDelete;
	
	while ( sceMcDelete(Cards[cardID].port, 0, filename) != sceMcResSucceed )
		;
	
	int result;
	sceMcSync(0, &cmd, &result);
	
	if ( result == sceMcResSucceed )
	{
		printf("Memory Card %i, %s NO_ERR_SUCCESSfully deleted", cardID, filename);
			
		nError = NO_ERR_SUCCESS;
		return nError;
	}
	
	if ( result == sceMcResNoFormat )
	{
		printf("Memory Card %i, %s not deleted as memory Card is unformatted", cardID, filename);
			
		nError = ERR_NOFORMAT;
		return nError;
	}
	
	if ( result == sceMcResNoEntry )
	{
		printf("Memory Card %i, %s attempt made to delete non existing file", cardID, filename);
			
		nError = ERR_DELETENOENTRY;
		return nError;
	}
	
	if ( result == sceMcResDeniedPermit )
	{
		printf("Memory Card %i, %s not deleted as file is in use or write protected", cardID, filename);
		
		nError = ERR_DELETEDENIED;
		return nError;
	}
	
	if ( result == sceMcResNotEmpty )
	{
		printf("Memory Card %i, %s not deleted. Entries remain in subdirectory", cardID, filename);
		
		nError = ERR_DELETEFAILED;
		return nError;
	}
	
	nError = ERR_NONE;
	return nError;
#else
	char path[512];
	sprintf(path, "%s\\%s\\%s\\%s", PCCardsPath, PCCardDir[cardID], CardCurDir[cardID], filename);
	int32 file = CFileMgr::OpenFile(path, "rb");
	if (file == 0)
	{
		sprintf(path, "%s\\%s\\%s", PCCardsPath, PCCardDir[cardID], filename);
		file = CFileMgr::OpenFile(path, "rb");
	}

	if ( file )
	{
		CFileMgr::CloseFile(file);
		
		DeleteFile(path);
		
		printf("Memory Card %i, %s NO_ERR_SUCCESSfully deleted", cardID, filename);
		
		nError = NO_ERR_SUCCESS;
		return nError;
	}
	
	printf("Memory Card %i, %s attempt made to delete non existing file", cardID, filename);
	nError = ERR_DELETENOENTRY;
		
	//nError = ERR_NONE;
	return nError;
		
#endif
}

void
CMemoryCard::PopulateErrorMessage()
{
	switch ( nError )
	{
		case ERR_WRITEFULLDEVICE:
		case ERR_DIRFULLDEVICE:
			pErrorMsg = TheText.Get("SLONDR"); break; // Insufficient space to save. Please insert a Memory Card (PS2) with at least 500KB of free space available into MEMORY CARD slot 1.
		case ERR_FORMATFAILED:
			pErrorMsg = TheText.Get("SLONFM"); break; // Error formatting Memory Card (PS2) in MEMORY CARD slot 1.
		case ERR_SAVEFAILED:
			pErrorMsg = TheText.Get("SLNSP");  break; // Insufficient space to save. Please insert a Memory Card (PS2) with at least 200KB of free space available into MEMORY CARD slot 1.
		case ERR_DELETEDENIED:
		case ERR_NOFORMAT:
			pErrorMsg = TheText.Get("SLONNF"); break; // Memory Card (PS2) in MEMORY CARD slot 1 is unformatted.
		case ERR_NONE:
			pErrorMsg = TheText.Get("SLONNO"); break; // No Memory Card (PS2) in MEMORY CARD slot 1.
	}
}

int32
CMemoryCard::WritetoMemCard(int32 file, void *buff, int32 size)
{
#if defined(PS2)
	int cmd = sceMcFuncNoWrite;
	int result = sceMcResSucceed;
	int result1 = sceMcResSucceed;
	
	CTimer::Stop();
	
	while ( sceMcWrite(file, buff, size) != sceMcResSucceed )
		;
	
	sceMcSync(0, &cmd, &result);
	
	if ( result == sceMcResNoFormat )
	{
		printf("No Bytes written for Filehandle %i \n", file);
		
		nError = ERR_NOFORMAT;
		return nError;
	}
	
	if ( result == sceMcResFullDevice )
	{
		printf("No Bytes written for Filehandle %i \n", file);
		
		nError = ERR_WRITEFULLDEVICE;
		return nError;
	}
	
	if ( result == sceMcResNoEntry )
	{
		printf("No Bytes written for Filehandle %i \n", file);
		
		nError = ERR_WRITENOENTRY;
		return nError;
	}
	
	if ( result == sceMcResDeniedPermit )
	{
		printf("No Bytes written for Filehandle %i \n", file);
		
		nError = ERR_WRITEDENIED;
		return nError;
	}
	
	if ( result == sceMcResFailReplace )
	{
		printf("No Bytes written for Filehandle %i \n", file);
		
		nError = ERR_WRITEFAILED;
		return nError;
	}
	
	if ( result <= -10 )
	{
		printf("No Bytes written for Filehandle %i \n", file);
		
		nError = ERR_NONE;
		return nError;
	}
	
	cmd = sceMcFuncNoFlush;
	
	while ( sceMcFlush(file) != sceMcResSucceed )
		;
	
	sceMcSync(0, &cmd, &result1);
	
	if ( result1 == sceMcResNoFormat )
	{
		printf("No Bytes written for Filehandle %i \n", file);
		
		nError = ERR_NOFORMAT;
		return nError;
	}
	
	if ( result1 == sceMcResNoEntry )
	{
		printf("No Bytes written for Filehandle %i \n", file);
		
		nError = ERR_FLUSHNOENTRY;
		return nError;
	}
	
	if ( result1 <= -10 )
	{
		printf("No Bytes written for Filehandle %i \n", file);
		
		nError = ERR_NONE;
		return nError;
	}
	
	if ( result > 0 && result1 == sceMcResSucceed )
	{
		printf("%i Bytes written for Filehandle %i \n", result, file);
	}
	else if ( result == sceMcResSucceed && result1 == sceMcResSucceed )
	{
		printf("Filehandle %i was flushed\n", file);
	}
	
	nError = NO_ERR_SUCCESS;
	return nError;
#else
	CTimer::Stop();

	int32 s = CFileMgr::Write(file, (const char *)buff, size);
	if ( s == size )
	{
		printf("%i Bytes written for Filehandle %i \n", s, file);
		nError = NO_ERR_SUCCESS;
		return s;
	}
	
	nError = ERR_NONE;
	return s;
#endif
}

static inline void
MakeSpaceForSizeInBufferPointer(uint8 *&presize, uint8 *&buf, uint8 *&postsize)
{
	presize = buf;
	buf += sizeof(uint32);
	postsize = buf;
}

static inline void
CopySizeAndPreparePointer(uint8 *&buf, uint8 *&postbuf, uint8 *&postbuf2, uint32 &unused, uint32 &size)
{
	memcpy(buf, &size, sizeof(size));
	size = align4bytes(size);
	postbuf2 += size;
	postbuf = postbuf2;
}

bool
CMemoryCard::SaveGame(void)
{
	uint32 saveSize = 0;
	uint32 totalSize = 0;
	
	CurrentCard = CARD_ONE;
	
	CheckSum = 0;
	
	CGameLogic::PassTime(360);
	CPlayerPed *ped = FindPlayerPed();
	ped->m_fCurrentStamina = ped->m_fMaxStamina;
	CGame::TidyUpMemory(true, false);
	
	saveSize = SAVE_FILE_SIZE;
	int32 minfree = 198;
	
	PopulateCardFlags(CurrentCard, false, false, true, false);
	
	if ( nError != NO_ERR_SUCCESS )
		return false;
	
	minfree += GetClusterAmountForFileCreation(CurrentCard);
	
	if ( nError != NO_ERR_SUCCESS )
		return false;
	
	if ( Cards[CurrentCard].free < 200 )
	{
		CTimer::Update();
		uint32 startTime = CTimer::GetTimeInMillisecondsPauseMode();
		
		while ( CTimer::GetTimeInMillisecondsPauseMode()-startTime < 1250 )
		{
			for ( int32 i = 0; i < 1000; i++ )
				powf(3.33f, 3.444f);
			
			CTimer::Update();
		}
			
		nError = ERR_SAVEFAILED;
		return false;
	}
	
	if ( Cards[CurrentCard].free < minfree )
	{
		CTimer::Update();
		uint32 startTime = CTimer::GetTimeInMillisecondsPauseMode();
		
		while ( CTimer::GetTimeInMillisecondsPauseMode()-startTime < 1250 )
		{
			for ( int32 i = 0; i < 1000; i++ )
				powf(3.33f, 3.444f);
			
			CTimer::Update();
		}
			
		nError = ERR_SAVEFAILED;
		return false;
	}
	
	uint32 size;
	uint8 *buf = work_buff;
	uint32 reserved = 0;
	
	int32 file = CreateMemCardFileReadWrite(CurrentCard, ValidSaveName);
	
	if ( nError != NO_ERR_SUCCESS )
	{
		strncpy(SaveFileNameJustSaved, ValidSaveName, sizeof(SaveFileNameJustSaved) - 1);
		return false;
	}
	
	WriteDataToBufferPointer(buf, saveSize);
	WriteDataToBufferPointer(buf, CGame::currLevel);
	WriteDataToBufferPointer(buf, TheCamera.GetPosition().x);
	WriteDataToBufferPointer(buf, TheCamera.GetPosition().y);
	WriteDataToBufferPointer(buf, TheCamera.GetPosition().z);
	WriteDataToBufferPointer(buf, CClock::ms_nMillisecondsPerGameMinute);
	WriteDataToBufferPointer(buf, CClock::ms_nLastClockTick);
	WriteDataToBufferPointer(buf, CClock::ms_nGameClockHours);
	WriteDataToBufferPointer(buf, CClock::ms_nGameClockMinutes);
	WriteDataToBufferPointer(buf, CPad::GetPad(0)->Mode);
	WriteDataToBufferPointer(buf, CTimer::m_snTimeInMilliseconds);
	WriteDataToBufferPointer(buf, CTimer::ms_fTimeScale);
	WriteDataToBufferPointer(buf, CTimer::ms_fTimeStep);
	WriteDataToBufferPointer(buf, CTimer::ms_fTimeStepNonClipped);
	WriteDataToBufferPointer(buf, CTimer::m_FrameCounter);
	WriteDataToBufferPointer(buf, CTimeStep::ms_fTimeStep);
	WriteDataToBufferPointer(buf, CTimeStep::ms_fFramesPerUpdate);
	WriteDataToBufferPointer(buf, CTimeStep::ms_fTimeScale);
	WriteDataToBufferPointer(buf, CWeather::OldWeatherType);
	WriteDataToBufferPointer(buf, CWeather::NewWeatherType);
	WriteDataToBufferPointer(buf, CWeather::ForcedWeatherType);
	WriteDataToBufferPointer(buf, CWeather::InterpolationValue);
	WriteDataToBufferPointer(buf, CMenuManager::m_PrefsMusicVolume);
	WriteDataToBufferPointer(buf, CMenuManager::m_PrefsSfxVolume);
	WriteDataToBufferPointer(buf, CMenuManager::m_PrefsControllerConfig);
	WriteDataToBufferPointer(buf, CMenuManager::m_PrefsUseVibration);
	WriteDataToBufferPointer(buf, CMenuManager::m_PrefsStereoMono);
	WriteDataToBufferPointer(buf, CMenuManager::m_PrefsRadioStation);
	WriteDataToBufferPointer(buf, CMenuManager::m_PrefsBrightness);
	WriteDataToBufferPointer(buf, CMenuManager::m_PrefsShowTrails);
	WriteDataToBufferPointer(buf, CMenuManager::m_PrefsShowSubtitles);
	WriteDataToBufferPointer(buf, CMenuManager::m_PrefsLanguage);
	WriteDataToBufferPointer(buf, CMenuManager::m_PrefsUseWideScreen);
	WriteDataToBufferPointer(buf, CPad::GetPad(0)->Mode);
#ifdef PS2
	WriteDataToBufferPointer(buf, BlurOn);
#else
	WriteDataToBufferPointer(buf, CMBlur::BlurOn);
#endif
	WriteDataToBufferPointer(buf, CompileDateAndTime.m_nSecond);
	WriteDataToBufferPointer(buf, CompileDateAndTime.m_nMinute);
	WriteDataToBufferPointer(buf, CompileDateAndTime.m_nHour);
	WriteDataToBufferPointer(buf, CompileDateAndTime.m_nDay);
	WriteDataToBufferPointer(buf, CompileDateAndTime.m_nMonth);
	WriteDataToBufferPointer(buf, CompileDateAndTime.m_nYear);
	WriteDataToBufferPointer(buf, CWeather::WeatherTypeInList);
	WriteDataToBufferPointer(buf, TheCamera.CarZoomIndicator);
	WriteDataToBufferPointer(buf, TheCamera.PedZoomIndicator);
	
	if ( nError != NO_ERR_SUCCESS )
	{
		strncpy(SaveFileNameJustSaved, ValidSaveName, sizeof(SaveFileNameJustSaved) - 1);
		return false;
	}
	
	uint8 *presize;
	uint8 *postsize;
	
	#define WriteSaveDataBlock(save_func)\
		do {\
			MakeSpaceForSizeInBufferPointer(presize, buf, postsize);\
			save_func(buf, &size);\
			CopySizeAndPreparePointer(presize, buf, postsize, reserved, size);\
		} while (0)
	
	WriteSaveDataBlock(CTheScripts::SaveAllScripts);
	printf("Script Save Size %d, \n", size);
	
	WriteSaveDataBlock(CPools::SavePedPool);
	printf("PedPool Save Size %d, \n", size);
	
	WriteSaveDataBlock(CGarages::Save);
	printf("Garage Save Size %d, \n", size);
	
	WriteSaveDataBlock(CPools::SaveVehiclePool);
	printf("Vehicle Save Size %d, \n", size);
	
	DoClassSaveRoutine(file, work_buff, buf - work_buff);
	totalSize += buf - work_buff;
	
	if ( nError != NO_ERR_SUCCESS )
		return false;
	
	buf = work_buff;
	reserved = 0;
	
	WriteSaveDataBlock(CPools::SaveObjectPool);
	printf("Object Save Size %d, \n", size);
	
	WriteSaveDataBlock(ThePaths.Save);
	printf("The Paths Save Size %d, \n", size);
	
	WriteSaveDataBlock(CCranes::Save);
	printf("Cranes Save Size %d, \n", size);
	
	DoClassSaveRoutine(file, work_buff, buf - work_buff);
	totalSize += buf - work_buff;
	
	if ( nError != NO_ERR_SUCCESS )
		return false;
	
	buf = work_buff;
	reserved = 0;
	
	WriteSaveDataBlock(CPickups::Save);
	printf("Pick Ups Save Size %d, \n", size);
	
	WriteSaveDataBlock(gPhoneInfo.Save);
	printf("Phones Save Size %d, \n", size);
	
	WriteSaveDataBlock(CRestart::SaveAllRestartPoints);
	printf("RestartPoints Save Size %d, \n", size);
	
	WriteSaveDataBlock(CRadar::SaveAllRadarBlips);
	printf("Radar Save Size %d, \n", size);
	
	WriteSaveDataBlock(CTheZones::SaveAllZones);
	printf("Save Size %d, \n", size);
	
	WriteSaveDataBlock(CGangs::SaveAllGangData);
	printf("Gangs Save Size %d, \n", size);
	
	WriteSaveDataBlock(CTheCarGenerators::SaveAllCarGenerators);
	printf("Car Gens Save Size %d, \n", size);
	
	WriteSaveDataBlock(CParticleObject::SaveParticle);
	printf("Particles Save Size %d, \n", size);
	
	WriteSaveDataBlock(cAudioScriptObject::SaveAllAudioScriptObjects);
	printf("Audio Script Save Size %d, \n", size);
	
	WriteSaveDataBlock(CWorld::Players[CWorld::PlayerInFocus].SavePlayerInfo);
	printf("Player Info Save Size %d, \n", size);
	
	WriteSaveDataBlock(CStats::SaveStats);
	printf("Stats Save Size %d, \n", size);
	
	WriteSaveDataBlock(CStreaming::MemoryCardSave);
	printf("Streaming Save Size %d, \n", size);
	
	WriteSaveDataBlock(CPedType::Save);
	printf("PedType Save Size %d, \n", size);
	
	DoClassSaveRoutine(file, work_buff, buf - work_buff);
	totalSize += buf - work_buff;
	
	if ( nError != NO_ERR_SUCCESS )
		return false;
	
	buf = work_buff;
	reserved = 0;
	
	for (int32 i = 0; i < 3; i++)
	{
		size = align4bytes(saveSize - totalSize - 4);
		if (size > sizeof(work_buff))
			size = sizeof(work_buff);
		if (size > 4) {
			DoClassSaveRoutine(file, work_buff, size);
			totalSize += size;
		}
	}

	WritetoMemCard(file, &CheckSum, sizeof(CheckSum));
	
	CloseMemCardFile(file);
	
	#undef WriteSaveDataBlock
	
	if ( nError != NO_ERR_SUCCESS )
	{
		strncpy(SaveFileNameJustSaved, ValidSaveName, sizeof(SaveFileNameJustSaved) - 1);
		DoHackRoundSTUPIDSonyDateTimeStuff(CARD_ONE, ValidSaveName);
		return false;
	}
	
	DoHackRoundSTUPIDSonyDateTimeStuff(CARD_ONE, ValidSaveName);
	strncpy(SaveFileNameJustSaved, ValidSaveName, sizeof(SaveFileNameJustSaved) - 1);
	return true;
}

bool
CMemoryCard::DoHackRoundSTUPIDSonyDateTimeStuff(int32 port, char *filename)
{
#if defined(PS2)
	int cmd = sceMcFuncNoFileInfo;
	int result = sceMcResSucceed;
	
	sceCdCLOCK rtc;
	sceCdReadClock(&rtc);
	
	sceScfGetLocalTimefromRTC(&rtc);
	
	#define ROUNDHACK(a) ( ((a) & 15) + ( ( ( ((a) >> 4) << 2 ) + ((a) >> 4) ) << 1 ) )
	
	sceMcTblGetDir info;
	
	info._Create.Sec   = ROUNDHACK(rtc.second);
	info._Create.Min   = ROUNDHACK(rtc.minute);
	info._Create.Hour  = ROUNDHACK(rtc.hour);
	info._Create.Day   = ROUNDHACK(rtc.day);
	info._Create.Month = ROUNDHACK(rtc.month);
	info._Create.Year  = ROUNDHACK(rtc.year) + 2000;
	
	#undef ROUNDHACK
	
	while ( sceMcSetFileInfo(port, 0, filename, (char *)&info, sceMcFileInfoCreate) != sceMcResSucceed )
		;
	
	sceMcSync(0, &cmd, &result);
	
	return sceMcResSucceed >= result;
#else
	return true;
#endif
}

int32
CMemoryCard::LookForRootDirectory(int32 cardID)
{
	CTimer::Stop();
	
#if defined(PS2)	
	int cmd = sceMcFuncNoGetDir;
	
	while ( sceMcGetDir(Cards[cardID].port, 0, Cards[cardID].dir, 0, ARRAY_SIZE(Cards[cardID].table), Cards[cardID].table) != sceMcResSucceed )
		;
	
	int result;
	sceMcSync(0, &cmd, &result);
	
	if ( result == 0 )
	{
		nError = NO_ERR_SUCCESS;
		return ERR_NOROOTDIR;
	}
	
	if ( result > sceMcResSucceed )
	{
		printf("Memory card %i present PopulateFileTables function NO_ERR_SUCCESSfull \n", cardID);
		
		nError = NO_ERR_SUCCESS;
		return nError;
	}
	
	if ( result == sceMcResNoFormat )
	{
		printf("Memory card %i PopulateFileTables function unNO_ERR_SUCCESSfull. MemoryCard not Formatted  \n", cardID);
		
		nError = ERR_NOFORMAT;
		return nError;
	}
	
	if ( result == sceMcResNoEntry )
	{
		printf("Memory card %i  PopulateFileTables function unNO_ERR_SUCCESSfull. Path does not exist  \n", cardID);
		
		nError = ERR_FILETABLENOENTRY;
		return nError;
	}
	
	printf("Memory card %i not Present\n", cardID);
		
	nError = ERR_NONE;
	return nError;
#else
	char path[512];
	sprintf(path, "%s\\%s\\%s", PCCardsPath, PCCardDir[cardID], Cards[cardID].dir);
	
	memset(Cards[cardID].table, 0, sizeof(Cards[cardID].table));
	WIN32_FIND_DATA fd; HANDLE hFind; int32 num = 0;
	if ( (hFind = FindFirstFile(path, &fd)) == INVALID_HANDLE_VALUE )
	{
		nError = NO_ERR_SUCCESS;
		return ERR_NOROOTDIR;
	}
	do
	{
		SYSTEMTIME st;
		FileTimeToSystemTime(&fd.ftCreationTime, &st);
		Cards[cardID].table[num]._Create.Sec = st.wSecond;Cards[cardID].table[num]._Create.Min = st.wMinute;Cards[cardID].table[num]._Create.Hour = st.wHour;Cards[cardID].table[num]._Create.Day = st.wDay;Cards[cardID].table[num]._Create.Month = st.wMonth;Cards[cardID].table[num]._Create.Year = st.wYear;
		FileTimeToSystemTime(&fd.ftLastWriteTime, &st);
		Cards[cardID].table[num]._Modify.Sec = st.wSecond;Cards[cardID].table[num]._Modify.Min = st.wMinute;Cards[cardID].table[num]._Modify.Hour = st.wHour;Cards[cardID].table[num]._Modify.Day = st.wDay;Cards[cardID].table[num]._Modify.Month = st.wMonth;Cards[cardID].table[num]._Modify.Year = st.wYear;
		Cards[cardID].table[num].FileSizeByte = fd.nFileSizeLow;
		strncpy((char *)Cards[cardID].table[num].EntryName, fd.cFileName, sizeof(Cards[cardID].table[num].EntryName) - 1);
		num++;
	} while( FindNextFile(hFind, &fd) && num < ARRAY_SIZE(Cards[cardID].table) );
	FindClose(hFind);
	
	if ( num == 0 )
	{
		nError = NO_ERR_SUCCESS;
		return ERR_NOROOTDIR;
	}
	
	//todo errors
	
	printf("Memory card %i present PopulateFileTables function NO_ERR_SUCCESSfull \n", cardID);
	
	nError = NO_ERR_SUCCESS;
	return nError;
#endif
}

int32
CMemoryCard::FillFirstFileWithGuff(int32 cardID)
{
	CTimer::Stop();
	
	char buff[80];
	strncpy(buff, Cards[cardID].dir+1, sizeof(buff) - 1);
	
	int32 file = CreateMemCardFileReadWrite(Cards[cardID].port, buff);
	
	if ( nError != NO_ERR_SUCCESS )
		return RES_FAILED;
	
	const int32 kBlockSize = GUFF_FILE_SIZE / 3;
	
	work_buff[kBlockSize-1] = 5;
	WritetoMemCard(file, work_buff, kBlockSize);
	WritetoMemCard(file, work_buff, kBlockSize);
	WritetoMemCard(file, work_buff, kBlockSize);
	
	if ( nError != NO_ERR_SUCCESS )
		return RES_FAILED;
	
	CloseMemCardFile(file);
	
	if ( nError != NO_ERR_SUCCESS )
		return RES_FAILED;
	
	return RES_SUCCESS;
}

bool
CMemoryCard::FindMostRecentFileName(int32 cardID, char *filename)
{
	CDate date1, date2;
	
	CTimer::Stop();
	
#if defined(PS2)
	int cmd = sceMcFuncNoGetDir;
	
	ClearFileTableBuffer(cardID);
	
	while ( sceMcGetDir(Cards[cardID].port, 0, "*", 0, ARRAY_SIZE(Cards[cardID].table), Cards[cardID].table) != sceMcResSucceed )
		;
	
	int result;
	sceMcSync(0, &cmd, &result);
	
	if ( result >= sceMcResSucceed )
	{
		printf("Memory card %i present PopulateFileTables function NO_ERR_SUCCESSfull \n", cardID);
		nError = NO_ERR_SUCCESS;
		
		for ( int32 entry = 7; entry < ARRAY_SIZE(Cards[CARD_ONE].table); entry++ )
		{
			bool found = false;
							
			if (   Cards[CARD_ONE].table[entry]._Modify.Sec   != 0
				|| Cards[CARD_ONE].table[entry]._Modify.Min   != 0
				|| Cards[CARD_ONE].table[entry]._Modify.Hour  != 0
				|| Cards[CARD_ONE].table[entry]._Modify.Day   != 0
				|| Cards[CARD_ONE].table[entry]._Modify.Month != 0
				|| Cards[CARD_ONE].table[entry]._Modify.Year  != 0 )
			{
				date1.m_nSecond = Cards[CARD_ONE].table[entry]._Modify.Sec;
				date1.m_nMinute = Cards[CARD_ONE].table[entry]._Modify.Min;
				date1.m_nHour   = Cards[CARD_ONE].table[entry]._Modify.Hour; 
				date1.m_nDay    = Cards[CARD_ONE].table[entry]._Modify.Day;
				date1.m_nMonth  = Cards[CARD_ONE].table[entry]._Modify.Month;
				date1.m_nYear   = Cards[CARD_ONE].table[entry]._Modify.Year;
				
				if ( Cards[CARD_ONE].table[entry].FileSizeByte != 0
					&& Cards[CARD_ONE].table[entry].AttrFile & sceMcFileAttrClosed
					&& Cards[CARD_ONE].table[entry].FileSizeByte >= SAVE_FILE_SIZE )
				{
					found = true;
				}
			}
			else
			if (   Cards[CARD_ONE].table[entry]._Create.Sec   != 0
				|| Cards[CARD_ONE].table[entry]._Create.Min   != 0
				|| Cards[CARD_ONE].table[entry]._Create.Hour  != 0
				|| Cards[CARD_ONE].table[entry]._Create.Day   != 0
				|| Cards[CARD_ONE].table[entry]._Create.Month != 0
				|| Cards[CARD_ONE].table[entry]._Create.Year  != 0 )
			{
				date1.m_nSecond = Cards[CARD_ONE].table[entry]._Create.Sec;
				date1.m_nMinute = Cards[CARD_ONE].table[entry]._Create.Min;
				date1.m_nHour   = Cards[CARD_ONE].table[entry]._Create.Hour; 
				date1.m_nDay    = Cards[CARD_ONE].table[entry]._Create.Day;
				date1.m_nMonth  = Cards[CARD_ONE].table[entry]._Create.Month;
				date1.m_nYear   = Cards[CARD_ONE].table[entry]._Create.Year;
				
				if ( Cards[CARD_ONE].table[entry].FileSizeByte != 0
					&& Cards[CARD_ONE].table[entry].AttrFile & sceMcFileAttrClosed
					&& Cards[CARD_ONE].table[entry].FileSizeByte >= SAVE_FILE_SIZE )
				{
					found = true;
				}
			}
			
			if ( found )
			{
				int32 d;
				if ( date1 > date2 )      d = 1;
				else if ( date1 < date2 ) d = 2;
				else                      d = 0;
				
				if ( d == 1 )
				{
					char *entryname = (char *)Cards[CARD_ONE].table[entry].EntryName;
					
					date2 = date1;
					strncpy(filename, entryname, 28);
				}
				else
				{
					int32 d;
					if ( date1 > date2 )      d = 1;
					else if ( date1 < date2 ) d = 2;
					else                      d = 0;
					
					if ( d == 0 )
					{
						char *entryname = (char *)Cards[CARD_ONE].table[entry].EntryName;
						date2 = date1;
						strncpy(filename, entryname, 28);
					}
				}
			}
		}
		
		if (   date2.m_nSecond != 0
			|| date2.m_nMinute != 0
			|| date2.m_nHour   != 0
			|| date2.m_nDay    != 0
			|| date2.m_nMonth  != 0
			|| date2.m_nYear   != 0 )
		{
			return true;
		}
		
		return false;
	}
	
	if ( result == sceMcResNoFormat )
	{
		printf("Memory card %i PopulateFileTables function unNO_ERR_SUCCESSfull. MemoryCard not Formatted  \n", cardID);
		nError = ERR_NOFORMAT;
		return false;
	}
	
	if ( result == sceMcResNoEntry )
	{
		printf("Memory card %i  PopulateFileTables function unNO_ERR_SUCCESSfull. Path does not exist  \n", cardID);
		nError = ERR_FILETABLENOENTRY;
		return false;
	}
	
	printf("Memory card %i not Present\n", cardID);
	nError = ERR_NONE;
	return false;
#else
	ClearFileTableBuffer(cardID);
	
	char path[512];
	sprintf(path, "%s\\%s\\%s\\*", PCCardsPath, PCCardDir[cardID], CardCurDir[cardID]);
	
	memset(Cards[cardID].table, 0, sizeof(Cards[cardID].table));
	WIN32_FIND_DATA fd; HANDLE hFind; int32 num = 0;
	if ( (hFind = FindFirstFile(path, &fd)) == INVALID_HANDLE_VALUE )
	{
		printf("Memory card %i not Present\n", cardID);
		nError = ERR_NONE;
		return nError;
	}
	do
	{
		SYSTEMTIME st;
		FileTimeToSystemTime(&fd.ftCreationTime, &st);
		Cards[cardID].table[num]._Create.Sec = st.wSecond;Cards[cardID].table[num]._Create.Min = st.wMinute;Cards[cardID].table[num]._Create.Hour = st.wHour;Cards[cardID].table[num]._Create.Day = st.wDay;Cards[cardID].table[num]._Create.Month = st.wMonth;Cards[cardID].table[num]._Create.Year = st.wYear;
		FileTimeToSystemTime(&fd.ftLastWriteTime, &st);
		Cards[cardID].table[num]._Modify.Sec = st.wSecond;Cards[cardID].table[num]._Modify.Min = st.wMinute;Cards[cardID].table[num]._Modify.Hour = st.wHour;Cards[cardID].table[num]._Modify.Day = st.wDay;Cards[cardID].table[num]._Modify.Month = st.wMonth;Cards[cardID].table[num]._Modify.Year = st.wYear;
		Cards[cardID].table[num].FileSizeByte = fd.nFileSizeLow;
		strncpy((char *)Cards[cardID].table[num].EntryName, fd.cFileName, sizeof(Cards[cardID].table[num].EntryName) - 1);
		num++;
	} while( FindNextFile(hFind, &fd) && num < ARRAY_SIZE(Cards[cardID].table) );
	FindClose(hFind);
	
	if ( num > 0 )
	{
		printf("Memory card %i present PopulateFileTables function NO_ERR_SUCCESSfull \n", cardID);
		nError = NO_ERR_SUCCESS;
		
		for ( int32 entry = 0; entry < ARRAY_SIZE(Cards[CARD_ONE].table); entry++ )
		{
			bool found = false;
							
			if (   Cards[CARD_ONE].table[entry]._Modify.Sec   != 0
				|| Cards[CARD_ONE].table[entry]._Modify.Min   != 0
				|| Cards[CARD_ONE].table[entry]._Modify.Hour  != 0
				|| Cards[CARD_ONE].table[entry]._Modify.Day   != 0
				|| Cards[CARD_ONE].table[entry]._Modify.Month != 0
				|| Cards[CARD_ONE].table[entry]._Modify.Year  != 0 )
			{
				date1.m_nSecond = Cards[CARD_ONE].table[entry]._Modify.Sec;
				date1.m_nMinute = Cards[CARD_ONE].table[entry]._Modify.Min;
				date1.m_nHour   = Cards[CARD_ONE].table[entry]._Modify.Hour; 
				date1.m_nDay    = Cards[CARD_ONE].table[entry]._Modify.Day;
				date1.m_nMonth  = Cards[CARD_ONE].table[entry]._Modify.Month;
				date1.m_nYear   = Cards[CARD_ONE].table[entry]._Modify.Year;
				
				if ( Cards[CARD_ONE].table[entry].FileSizeByte != 0
					&& Cards[CARD_ONE].table[entry].FileSizeByte >= SAVE_FILE_SIZE )
				{
					found = true;
				}
			}
			else
			if (   Cards[CARD_ONE].table[entry]._Create.Sec   != 0
				|| Cards[CARD_ONE].table[entry]._Create.Min   != 0
				|| Cards[CARD_ONE].table[entry]._Create.Hour  != 0
				|| Cards[CARD_ONE].table[entry]._Create.Day   != 0
				|| Cards[CARD_ONE].table[entry]._Create.Month != 0
				|| Cards[CARD_ONE].table[entry]._Create.Year  != 0 )
			{
				date1.m_nSecond = Cards[CARD_ONE].table[entry]._Create.Sec;
				date1.m_nMinute = Cards[CARD_ONE].table[entry]._Create.Min;
				date1.m_nHour   = Cards[CARD_ONE].table[entry]._Create.Hour; 
				date1.m_nDay    = Cards[CARD_ONE].table[entry]._Create.Day;
				date1.m_nMonth  = Cards[CARD_ONE].table[entry]._Create.Month;
				date1.m_nYear   = Cards[CARD_ONE].table[entry]._Create.Year;
				
				if ( Cards[CARD_ONE].table[entry].FileSizeByte != 0
					&& Cards[CARD_ONE].table[entry].FileSizeByte >= SAVE_FILE_SIZE )
				{
					found = true;
				}
			}
			
			if ( found )
			{
				int32 d;
				if ( date1 > date2 )      d = 1;
				else if ( date1 < date2 ) d = 2;
				else                      d = 0;
				
				if ( d == 1 )
				{
					char *entryname = (char *)Cards[CARD_ONE].table[entry].EntryName;
					
					date2 = date1;
					strncpy(filename, entryname, 28);
				}
				else
				{
					int32 d;
					if ( date1 > date2 )      d = 1;
					else if ( date1 < date2 ) d = 2;
					else                      d = 0;
					
					if ( d == 0 )
					{
						char *entryname = (char *)Cards[CARD_ONE].table[entry].EntryName;
						date2 = date1;
						strncpy(filename, entryname, 28);
					}
				}
			}
		}
		
		if (   date2.m_nSecond != 0
			|| date2.m_nMinute != 0
			|| date2.m_nHour   != 0
			|| date2.m_nDay    != 0
			|| date2.m_nMonth  != 0
			|| date2.m_nYear   != 0 )
		{
			return true;
		}
		
		return false;
	}
	
	//todo errors
	
	nError = ERR_NONE;
	return false;
#endif
}

void
CMemoryCard::ClearFileTableBuffer(int32 cardID)
{
	for ( int32 i = 0; i < ARRAY_SIZE(Cards[cardID].table); i++ )
	{
		Cards[cardID].table[i].FileSizeByte = 0;
		strncpy((char *)Cards[cardID].table[i].EntryName, " ", sizeof(Cards[cardID].table[i].EntryName) - 1);
	}
}

int32
CMemoryCard::GetClusterAmountForFileCreation(int32 port)
{
#if defined(PS2)
	int cmd = sceMcFuncNoEntSpace;
	int result = 0;
	
	CTimer::Stop();
	
	while ( sceMcGetEntSpace(port, 0, TheGameRootDirectory) != sceMcResSucceed )
		;
	
	sceMcSync(0, &cmd, &result);
	
	if ( result >= sceMcResSucceed )
	{
		nError = NO_ERR_SUCCESS;
		return result;
	}
	
	if ( result == sceMcResNoFormat )
	{
		nError = ERR_NOFORMAT;
		return nError;
	}
	
	nError = ERR_NONE;
	return nError;
#else
	CTimer::Stop();
	nError = NO_ERR_SUCCESS;
	return 0;
#endif
}

bool
CMemoryCard::DeleteEverythingInGameRoot(int32 cardID)
{
	CTimer::Stop();
	
	ChangeDirectory(CurrentCard, Cards[CurrentCard].dir);
	
	if ( nError != NO_ERR_SUCCESS )
		return false;
	
	PopulateFileTable(cardID);
	
	for ( int32 i = ARRAY_SIZE(Cards[cardID].table) - 1; i >= 0; i--)
		DeleteMemoryCardFile(cardID, (char *)Cards[cardID].table[i].EntryName);
	
	ChangeDirectory(CurrentCard, "/");
	
	DeleteMemoryCardFile(cardID, Cards[CurrentCard].dir);
	
	if ( nError != NO_ERR_SUCCESS )
		return false;
	
	return true;
}

int32
CMemoryCard::CheckDataNotCorrupt(char *filename)
{
	CheckSum = 0;
	
	int32 lang = 0;
	int32 level = 0;
	
	LastBlockSize = 0;
	
	char buf[100*4];
	
	for ( int32 i = 0; i < sizeof(buf); i++ )
		buf[i] = '\0';
	
	strncpy(buf, Cards[CurrentCard].dir, sizeof(buf) - 1);
	strncat(buf, "/", sizeof(buf) - 1);
	strcat (buf, filename);
	
	ChangeDirectory(CurrentCard, Cards[CurrentCard].dir);
	
	if ( nError != NO_ERR_SUCCESS )
		return RES_FAILED;
	
	int32 file = OpenMemCardFileForReading(CurrentCard, buf);
	
	if ( nError != NO_ERR_SUCCESS )
		return RES_FAILED;
	
	int32 bytes_processed = 0;
	int32 blocknum = 0; 	
	int32 lastblocksize;
	
	while ( SAVE_FILE_SIZE - sizeof(int32) > bytes_processed && blocknum < 8 )
	{
		int32 size;
		
		ReadFromMemCard(file, &size, sizeof(size));
		
		if ( nError != NO_ERR_SUCCESS )
			return RES_FAILED;
		
		lastblocksize = ReadFromMemCard(file, work_buff, align4bytes(size));
		
		if ( nError != NO_ERR_SUCCESS )
			return RES_FAILED;
		
		uint8 sizebuff[4];
		memcpy(sizebuff, &size, sizeof(size));
		
		for ( int32 i = 0; i < ARRAY_SIZE(sizebuff); i++ )
			CheckSum += sizebuff[i];
		
		uint8 *pWork_buf = work_buff;
		for ( int32 i = 0; i < lastblocksize; i++ )
		{
			CheckSum += *pWork_buf++;
			bytes_processed++;
		}
		
		if ( blocknum == 0 )
		{
			uint8 *pBuf = work_buff + sizeof(uint32);
			ReadDataFromBufferPointer(pBuf, level);
			pBuf += sizeof(uint32) * 29;
			ReadDataFromBufferPointer(pBuf, lang);
		}
		
		blocknum++;
	}
	
	int32 checkSum;
	ReadFromMemCard(file, &checkSum, sizeof(checkSum));
	CloseMemCardFile(file);
	
	if ( nError != NO_ERR_SUCCESS )
		return RES_FAILED;
	
	if ( CheckSum == checkSum )
	{
		m_LevelToLoad    = level;
		m_LanguageToLoad = lang;
		LastBlockSize    = lastblocksize;
		
		return RES_SUCCESS;
	}
	
	nError = ERR_DATACORRUPTED;
	return RES_FAILED;
}

int32
CMemoryCard::GetLanguageToLoad(void)
{
	return m_LanguageToLoad;
}

int32
CMemoryCard::GetLevelToLoad(void)
{
	return m_LevelToLoad;
}

bool
CMemoryCard::CreateGameDirectoryFromScratch(int32 cardID)
{
	TheMemoryCard.PopulateCardFlags(cardID, false, false, true, true);
	
	int32 err = RES_SUCCESS;
	
	if ( nError != NO_ERR_SUCCESS )
		return false;
	
	if ( Cards[CurrentCard].free < 500 )
	{
		CTimer::Update();
		uint32 startTime = CTimer::GetTimeInMillisecondsPauseMode();
		
		while ( CTimer::GetTimeInMillisecondsPauseMode()-startTime < 1250 )
		{
			for ( int32 i = 0; i < 1000; i++ )
				powf(3.33f, 3.444f);
			
			CTimer::Update();
		}
			
		nError = ERR_DIRFULLDEVICE;
		return false;
	}
	
	TheMemoryCard.ChangeDirectory(CARD_ONE, "/");
	
	if ( nError != NO_ERR_SUCCESS )
		return false;

	int32 r = LookForRootDirectory(CARD_ONE);
	
	if ( nError != NO_ERR_SUCCESS )
		return false;
	
	if ( r == ERR_NOROOTDIR )
	{
		CreateRootDirectory(CARD_ONE);
		
		if ( nError != NO_ERR_SUCCESS )
			DeleteEverythingInGameRoot(CARD_ONE);
	}
	
	ChangeDirectory(CARD_ONE, Cards[CARD_ONE].dir);
	
	if ( nError != NO_ERR_SUCCESS )
		return false;
	
	PopulateFileTable(CARD_ONE);
	
	if ( nError != NO_ERR_SUCCESS )
		return false;
	
#if defined(PS2)
	bool entryExist;
	
	entryExist = false;
	if ( TheMemoryCard.PopulateFileTable(CARD_ONE) == NO_ERR_SUCCESS )
	{
		for ( int32 i = 0; i < ARRAY_SIZE(TheMemoryCard.Cards[CARD_ONE].table); i++ )
		{
			if ( !strcmp("icon.sys", (char *)TheMemoryCard.Cards[CARD_ONE].table[i].EntryName) )
			{
				entryExist = true;
				break;
			}
		}
	}
	
	if ( !entryExist )
		err = RES_FAILED;
	
	if ( nError != NO_ERR_SUCCESS )
		return false;
	
	entryExist = false;
	if ( TheMemoryCard.PopulateFileTable(CARD_ONE) == NO_ERR_SUCCESS )
	{
		for ( int32 i = 0; i < ARRAY_SIZE(TheMemoryCard.Cards[CARD_ONE].table); i++ )
		{
			if ( !strcmp(icon_one, (char *)TheMemoryCard.Cards[CARD_ONE].table[i].EntryName) )
			{
				entryExist = true;
				break;
			}
		}
	}
	
	if ( !entryExist )
		err = RES_FAILED;
	
	if ( nError != NO_ERR_SUCCESS )
		return false;
	
	entryExist = false;
	if ( TheMemoryCard.PopulateFileTable(CARD_ONE) == NO_ERR_SUCCESS )
	{
		for ( int32 i = 0; i < ARRAY_SIZE(TheMemoryCard.Cards[CARD_ONE].table); i++ )
		{
			if ( !strcmp(icon_two, (char *)TheMemoryCard.Cards[CARD_ONE].table[i].EntryName) )
			{
				entryExist = true;
				break;
			}
		}
	}
	
	if ( !entryExist )
		err = RES_FAILED;
	
	if ( nError != NO_ERR_SUCCESS )
		return false;
	
	entryExist = false;
	if ( TheMemoryCard.PopulateFileTable(CARD_ONE) == NO_ERR_SUCCESS )
	{
		for ( int32 i = 0; i < ARRAY_SIZE(TheMemoryCard.Cards[CARD_ONE].table); i++ )
		{
			if ( !strcmp(icon_three, (char *)TheMemoryCard.Cards[CARD_ONE].table[i].EntryName) )
			{
				entryExist = true;
				break;
			}
		}
	}
	
	if ( !entryExist )
		err = RES_FAILED;
	
	if ( nError != NO_ERR_SUCCESS )
		return false;
	
	if ( err != RES_SUCCESS )
	{
		int32 icon = CreateIconFiles(CARD_ONE, icon_one, icon_two, icon_three);
		
		if ( nError != NO_ERR_SUCCESS )
			return false;
		
		if ( icon != RES_SUCCESS )
			DeleteEverythingInGameRoot(CARD_ONE);
	}
#endif
	
	int32 guff = FillFirstFileWithGuff(CARD_ONE);
	
	if ( nError != NO_ERR_SUCCESS )
		return false;
	
	if ( guff == RES_SUCCESS )
	{
		printf("Game Default directory present");
		return true;
	}
	
	DeleteEverythingInGameRoot(CARD_ONE);
	
	return false;
}

bool
CMemoryCard::CheckGameDirectoryThere(int32 cardID)
{
	TheMemoryCard.PopulateCardFlags(cardID, false, false, true, true);
	
	if ( TheMemoryCard.nError != NO_ERR_SUCCESS )
		return false;
	
	TheMemoryCard.ChangeDirectory(cardID, Cards[CARD_ONE].dir);
	
	if ( TheMemoryCard.nError != NO_ERR_SUCCESS )
		return false;
	
	PopulateFileTable(cardID);
	
	if ( TheMemoryCard.nError != NO_ERR_SUCCESS )
		return false;
	

	bool entryExist;
	
#if defined(PS2)
	entryExist = false;
	if ( TheMemoryCard.PopulateFileTable(cardID) == NO_ERR_SUCCESS )
	{
		for ( int32 i = 0; i < ARRAY_SIZE(TheMemoryCard.Cards[cardID].table); i++ )
		{
			if ( !strcmp("icon.sys", (char *)TheMemoryCard.Cards[cardID].table[i].EntryName) )
			{
				entryExist = true;
				break;
			}
		}
	}
	
	if ( !entryExist )
		return false;
	
	entryExist = false;
	if ( TheMemoryCard.PopulateFileTable(cardID) == NO_ERR_SUCCESS )
	{
		for ( int32 i = 0; i < ARRAY_SIZE(TheMemoryCard.Cards[cardID].table); i++ )
		{
			if ( !strcmp(icon_one, (char *)TheMemoryCard.Cards[cardID].table[i].EntryName) )
			{
				entryExist = true;
				break;
			}
		}
	}
	
	if ( !entryExist )
		return false;
	
	entryExist = false;
	if ( TheMemoryCard.PopulateFileTable(cardID) == NO_ERR_SUCCESS )
	{
		for ( int32 i = 0; i < ARRAY_SIZE(TheMemoryCard.Cards[cardID].table); i++ )
		{
			if ( !strcmp(icon_two, (char *)TheMemoryCard.Cards[cardID].table[i].EntryName) )
			{
				entryExist = true;
				break;
			}
		}
	}
	
	if ( !entryExist )
		return false;
	
	entryExist = false;
	if ( TheMemoryCard.PopulateFileTable(cardID) == NO_ERR_SUCCESS )
	{
		for ( int32 i = 0; i < ARRAY_SIZE(TheMemoryCard.Cards[cardID].table); i++ )
		{
			if ( !strcmp(icon_three, (char *)TheMemoryCard.Cards[cardID].table[i].EntryName) )
			{
				entryExist = true;
				break;
			}
		}
	}
	
	if ( !entryExist )
		return false;
#endif
	
	char buff[80];
	
	strncpy(buff, Cards[cardID].dir+1, sizeof(buff) - 1);


	entryExist = false;
	if ( TheMemoryCard.PopulateFileTable(cardID) == NO_ERR_SUCCESS )
	{
		for ( int32 i = 0; i < ARRAY_SIZE(TheMemoryCard.Cards[cardID].table); i++ )
		{
			if ( !strcmp(buff, (char *)TheMemoryCard.Cards[cardID].table[i].EntryName) )
			{
				entryExist = true;
				break;
			}
		}
	}
	
	if ( !entryExist )
		return false;
	
	printf("Game directory present");
	
	return true;
}

void
CMemoryCard::PopulateSlotInfo(int32 cardID)
{
	CTimer::Stop();
	
	for ( int32 i = 0; i < MAX_SLOTS; i++ )
	{
		Slots[i] = SLOT_NOTPRESENT;
		
		for ( int32 j = 0; j < ARRAY_SIZE(SlotFileName[i]); j++ )
			SlotFileName[i][j] = L'\0';
		
		for ( int32 j = 0; j < ARRAY_SIZE(SlotSaveDate[i]); j++ )
			SlotSaveDate[i][j] = L'\0';
		
		UnicodeStrcpy(SlotSaveDate[i], TheText.Get("DEFDT"));
	}
	
	TheMemoryCard.PopulateCardFlags(cardID, false, false, true, true);
	
	if ( nError != NO_ERR_SUCCESS )
		return;
	
	TheMemoryCard.ChangeDirectory(cardID, TheMemoryCard.Cards[CARD_ONE].dir);
	
	if ( nError != NO_ERR_SUCCESS && nError != ERR_DIRNOENTRY )
		return;
	
	PopulateFileTable(cardID);
	
	if ( nError != NO_ERR_SUCCESS && nError != ERR_FILETABLENOENTRY )
		return;
	
	for ( int32 slot = 0; slot < MAX_SLOTS; slot++ )
	{
#if defined(PS2)
		for ( int32 entry = 7; entry < ARRAY_SIZE(Cards[cardID].table); entry++ )
#else
		for ( int32 entry = 0; entry < ARRAY_SIZE(Cards[cardID].table); entry++ )
#endif
		{
			if ( TheMemoryCard.Cards[CARD_ONE].table[entry].FileSizeByte != 0 )
			{
				char slotnum[30];
				char slotname[30];
				char slotdate[30];
						
				if (
#if defined(PS2)
					TheMemoryCard.Cards[CARD_ONE].table[entry].AttrFile & sceMcFileAttrClosed &&
#endif
					TheMemoryCard.Cards[CARD_ONE].table[entry].FileSizeByte >= SAVE_FILE_SIZE )
				{
					char *entryname = (char *)Cards[cardID].table[entry].EntryName;
					
					bool bFound = false;
#if defined(PS2)
					for ( int32 i = 7; i < ARRAY_SIZE(Cards[cardID].table) && !bFound; i++ )
#else
					for ( int32 i = 0; i < ARRAY_SIZE(Cards[cardID].table) && !bFound; i++ )
#endif
					{
						sprintf(slotnum, "%i ", slot+1);

						for ( int32 j = 0; j < sizeof(slotname); j++ )
							slotname[j] = '\0';
							
						strncat(slotname, slotnum, sizeof(slotnum)-1);
						
						if ( !strncmp(slotname, entryname, 1) )
						{
							bFound = true;

							Slots[slot] = SLOT_PRESENT;							
							AsciiToUnicode(entryname, SlotFileName[slot]);
							
							int32 sec   = Cards[CARD_ONE].table[entry]._Create.Sec;
							int32 month = Cards[CARD_ONE].table[entry]._Create.Month;
							int32 year  = Cards[CARD_ONE].table[entry]._Create.Year;
							int32 min   = Cards[CARD_ONE].table[entry]._Create.Min;
							int32 hour  = Cards[CARD_ONE].table[entry]._Create.Hour;
							int32 day   = Cards[CARD_ONE].table[entry]._Create.Day;
							
							for ( int32 j = 0; j < ARRAY_SIZE(SlotSaveDate[slot]); j++ )
								SlotSaveDate[slot][j] = L'\0';
							
							for ( int32 j = 0; j < ARRAY_SIZE(slotdate); j++ )
								slotdate[j] = '\0';
							
							char *monthstr;
							switch ( month )
							{
								case  1: monthstr = UnicodeToAsciiForMemoryCard(TheText.Get("JAN")); break;
								case  2: monthstr = UnicodeToAsciiForMemoryCard(TheText.Get("FEB")); break;
								case  3: monthstr = UnicodeToAsciiForMemoryCard(TheText.Get("MAR")); break;
								case  4: monthstr = UnicodeToAsciiForMemoryCard(TheText.Get("APR")); break;
								case  5: monthstr = UnicodeToAsciiForMemoryCard(TheText.Get("MAY")); break;
								case  6: monthstr = UnicodeToAsciiForMemoryCard(TheText.Get("JUN")); break;
								case  7: monthstr = UnicodeToAsciiForMemoryCard(TheText.Get("JUL")); break;
								case  8: monthstr = UnicodeToAsciiForMemoryCard(TheText.Get("AUG")); break;
								case  9: monthstr = UnicodeToAsciiForMemoryCard(TheText.Get("SEP")); break;
								case 10: monthstr = UnicodeToAsciiForMemoryCard(TheText.Get("OCT")); break;
								case 11: monthstr = UnicodeToAsciiForMemoryCard(TheText.Get("NOV")); break;
								case 12: monthstr = UnicodeToAsciiForMemoryCard(TheText.Get("DEC")); break;
							}
							
							sprintf(slotdate, "%02d %s %04d %02d:%02d:%02d", day, monthstr, year, hour, min, sec);
							AsciiToUnicode(slotdate, SlotSaveDate[slot]);
						}
					}
				}
				else
				{
					char *entryname = (char *)Cards[cardID].table[entry].EntryName;
					
					bool bFound = false;
#if defined(PS2)
					for ( int32 i = 7; i < ARRAY_SIZE(Cards[cardID].table) && !bFound; i++ ) // again ...
#else
					for ( int32 i = 0; i < ARRAY_SIZE(Cards[cardID].table) && !bFound; i++ ) // again ...
#endif
					{
						sprintf(slotnum, "%i ", slot+1);
						
						for ( int32 j = 0; j < sizeof(slotname); j++ )
							slotname[j] = '\0';
						
						strncat(slotname, slotnum, sizeof(slotnum)-1);
						
						if ( !strncmp(slotname, entryname, 1) )
						{							
							bFound = true;

							Slots[slot] = SLOT_CORRUPTED;							
							AsciiToUnicode(entryname, SlotFileName[slot]);
						}
					}
				}
			}
		}
	}
	
	nError = NO_ERR_SUCCESS;
	return;
}

int32
CMemoryCard::GetInfoOnSpecificSlot(int32 slotID)
{
	return Slots[slotID];
}

wchar *
CMemoryCard::GetDateAndTimeOfSavedGame(int32 slotID)
{
	return SlotSaveDate[slotID];
}

int32
CMemoryCard::CheckCardStateAtGameStartUp(int32 cardID)
{
	CheckCardInserted(cardID);
	if ( nError == ERR_NOFORMAT )
		return MCSTATE_OK;
	if ( nError == ERR_NONE )
		return MCSTATE_NOCARD;
	
	if ( !CheckGameDirectoryThere(cardID) )
	{
		if ( nError == ERR_NONE )
			return MCSTATE_NOCARD;
		
		DeleteEverythingInGameRoot(cardID);
		if ( nError == ERR_NONE )
			return MCSTATE_NOCARD;
		
		TheMemoryCard.PopulateCardFlags(cardID, false, false, true, true);
		if ( nError == ERR_NONE )
			return MCSTATE_NOCARD;
		
		if ( Cards[CurrentCard].free < 500 )
			return MCSTATE_NEED_500KB;
		
		return MCSTATE_OK;
	}
	
	TheMemoryCard.CheckCardInserted(CARD_ONE);
	
	if ( nError == NO_ERR_SUCCESS )
	{
		if ( TheMemoryCard.ChangeDirectory(CARD_ONE, Cards[CARD_ONE].dir) != ERR_NONE )
		{
			if ( TheMemoryCard.FindMostRecentFileName(CARD_ONE, MostRecentFile) == true )
			{
				if ( TheMemoryCard.CheckDataNotCorrupt(MostRecentFile) == RES_FAILED )
				{
					TheMemoryCard.PopulateCardFlags(cardID, false, false, true, true);
					if ( Cards[CurrentCard].free < 200 )
						return MCSTATE_NEED_200KB;
				}
			}
			else
			{
				TheMemoryCard.PopulateCardFlags(cardID, false, false, true, true);
				if ( Cards[CurrentCard].free < 200 )
					return MCSTATE_NEED_200KB;
			}
		}
	}
	
	if ( TheMemoryCard.CheckCardInserted(CARD_ONE) != NO_ERR_SUCCESS )
		return MCSTATE_NOCARD;
	
	return MCSTATE_OK;
}

void
CMemoryCard::SaveSlot(int32 slotID)
{
	bool bSave = true;
	
	for ( int32 j = 0; j < sizeof(ValidSaveName); j++ )
		ValidSaveName[j] = '\0';
						
	char buff[100];
	
	sprintf(buff, "%i ", slotID+1);
	strncat(ValidSaveName, buff, sizeof(ValidSaveName) - 1);
	
	if ( CStats::LastMissionPassedName[0] != '\0' )
	{
		char mission[100];
		
		strcpy(mission, UnicodeToAsciiForMemoryCard(TheText.Get(CStats::LastMissionPassedName)));
		
#ifdef FIX_BUGS
		strncat(ValidSaveName, mission, sizeof(ValidSaveName)-1);
#else
		strncat(ValidSaveName, mission, 21);
		strncat(ValidSaveName, "...", strlen("..."));
#endif
	}
	
	if ( !CheckGameDirectoryThere(CARD_ONE) )
	{
		DeleteEverythingInGameRoot(CARD_ONE);
		bSave = CreateGameDirectoryFromScratch(CARD_ONE);
	}
	
	if ( bSave )
	{
		if ( Slots[slotID] == SLOT_PRESENT )
		{
			TheMemoryCard.ChangeDirectory(CARD_ONE, Cards[CurrentCard].dir);
			if ( nError == NO_ERR_SUCCESS )
				TheMemoryCard.DeleteMemoryCardFile(CARD_ONE, UnicodeToAsciiForMemoryCard(SlotFileName[slotID]));
		}
		
		SaveGame();
	}
	
	CTimer::Stop();
	CStreaming::FlushRequestList();
	CStreaming::DeleteRwObjectsAfterDeath(FindPlayerPed()->GetPosition());
	CStreaming::RemoveUnusedModelsInLoadedList();
	CGame::DrasticTidyUpMemory(false);
	CTimer::Update();
}

void
CMemoryCard::DeleteSlot(int32 slotID)
{
	TheMemoryCard.ChangeDirectory(CARD_ONE, Cards[CurrentCard].dir);
	
	if ( nError == NO_ERR_SUCCESS )
		TheMemoryCard.DeleteMemoryCardFile(CARD_ONE, UnicodeToAsciiForMemoryCard(SlotFileName[slotID]));
}

void
CMemoryCard::LoadSlotToBuffer(int32 slotID)
{
	CStreaming::DeleteAllRwObjects();
	
	strcpy(LoadFileName, UnicodeToAsciiForMemoryCard(SlotFileName[slotID]));
	
	TheMemoryCard.ChangeDirectory(CARD_ONE, Cards[CurrentCard].dir);
	
	if ( nError == NO_ERR_SUCCESS )
		TheMemoryCard.CheckDataNotCorrupt(LoadFileName);
}

wchar *
CMemoryCard::GetNameOfSavedGame(int32 slotID)
{
	return SlotFileName[slotID];
}

int32
CMemoryCard::DoClassSaveRoutine(int32 file, uint8 *data, uint32 size)
{
	WritetoMemCard(file, &size, sizeof(size));
	
	if ( nError != NO_ERR_SUCCESS )
	{
		strncpy(SaveFileNameJustSaved, ValidSaveName, sizeof(ValidSaveName) - 1);
		return ERR_NONE;
	}
	
	WritetoMemCard(file, data, align4bytes(size));
	
	uint8 sizebuff[4];
	memcpy(sizebuff, &size, sizeof(size));
	
	for ( int32 i = 0; i < ARRAY_SIZE(sizebuff); i++ )
		CheckSum += sizebuff[i];
	
	for ( int32 i = 0; i < align4bytes(size); i++ )
		CheckSum += *data++;
	
	if ( nError != NO_ERR_SUCCESS )
	{
		strncpy(SaveFileNameJustSaved, ValidSaveName, sizeof(ValidSaveName) - 1);
		return ERR_NONE;
	}
	
	return nError;
}

#endif
