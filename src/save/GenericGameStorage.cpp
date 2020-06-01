#define WITHWINDOWS
#include "common.h"
#include "crossplatform.h"
#include "main.h"

#include "DMAudio.h"
#include "AudioScriptObject.h"
#include "Camera.h"
#include "CarGen.h"
#include "Cranes.h"
#include "Clock.h"
#include "Date.h"
#include "FileMgr.h"
#include "Frontend.h"
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
#include "SetPieces.h"
#include "Stats.h"
#include "Streaming.h"
#include "Timer.h"
#include "TimeStep.h"
#include "Weather.h"
#include "World.h"
#include "Zones.h"

#define BLOCK_COUNT 20
#define SIZE_OF_SIMPLEVARS 0xBC

const uint32 SIZE_OF_ONE_GAME_IN_BYTES = 201729;

char DefaultPCSaveFileName[260];
char ValidSaveName[260];
char LoadFileName[256];
wchar SlotFileName[SLOT_COUNT][260];
wchar SlotSaveDate[SLOT_COUNT][70];
int CheckSum;
eLevelName m_LevelToLoad;
char SaveFileNameJustSaved[260];
int Slots[SLOT_COUNT+1];
CDate CompileDateAndTime;

bool b_FoundRecentSavedGameWantToLoad;
bool JustLoadedDontFadeInYet;
bool StillToFadeOut;
uint32 TimeStartedCountingForFade;
uint32 TimeToStayFadedBeforeFadeOut = 1750;

#define ReadDataFromBufferPointer(buf, to) memcpy(&to, buf, sizeof(to)); buf += align4bytes(sizeof(to));
#define WriteDataToBufferPointer(buf, from) memcpy(buf, &from, sizeof(from)); buf += align4bytes(sizeof(from));

#define LoadSaveDataBlock()\
do {\
	if (!ReadDataFromFile(file, (uint8 *) &size, 4))\
		return false;\
	size = align4bytes(size);\
	if (!ReadDataFromFile(file, work_buff, size))\
		return false;\
	buf = work_buff;\
} while (0)

#define ReadDataFromBlock(msg,load_func)\
do {\
	debug(msg);\
	ReadDataFromBufferPointer(buf, size);\
	load_func(buf, size);\
	size = align4bytes(size);\
	buf += size;\
} while (0)

#define WriteSaveDataBlock(save_func)\
do {\
	size = 0;\
	buf = work_buff;\
	reserved = 0;\
	MakeSpaceForSizeInBufferPointer(presize, buf, postsize);\
	save_func(buf, &size);\
	CopySizeAndPreparePointer(presize, buf, postsize, reserved, size);\
	if (!PcSaveHelper.PcClassSaveRoutine(file, work_buff, buf - work_buff))\
		return false;\
	totalSize += buf - work_buff;\
} while (0)

bool
GenericSave(int file)
{
	uint8 *buf, *presize, *postsize;
	uint32 size;
	uint32 reserved;

	uint32 totalSize;
	
	wchar *lastMissionPassed;
	wchar suffix[6];
	wchar saveName[24];
	SYSTEMTIME saveTime;
	CPad *currPad;

	CheckSum = 0;
	buf = work_buff;
	reserved = 0;

	// Save simple vars
	lastMissionPassed = TheText.Get(CStats::LastMissionPassedName);
	if (lastMissionPassed[0] != '\0') {
		AsciiToUnicode("...'", suffix);
#ifdef FIX_BUGS
		// fix buffer overflow
		int len = UnicodeStrlen(lastMissionPassed);
		if (len > ARRAY_SIZE(saveName)-1)
			len = ARRAY_SIZE(saveName)-1;
		memcpy(saveName, lastMissionPassed, sizeof(wchar) * len);
#else
		TextCopy(saveName, lastMissionPassed);
		int len = UnicodeStrlen(saveName);
#endif
		saveName[len] = '\0';
		if (len > ARRAY_SIZE(saveName)-2)
			TextCopy(&saveName[ARRAY_SIZE(saveName)-ARRAY_SIZE(suffix)], suffix);
		saveName[ARRAY_SIZE(saveName)-1] = '\0';
	}
	WriteDataToBufferPointer(buf, saveName);
	GetLocalTime(&saveTime);
	WriteDataToBufferPointer(buf, saveTime);
	WriteDataToBufferPointer(buf, SIZE_OF_ONE_GAME_IN_BYTES);
	WriteDataToBufferPointer(buf, CGame::currLevel);
	WriteDataToBufferPointer(buf, TheCamera.GetPosition().x);
	WriteDataToBufferPointer(buf, TheCamera.GetPosition().y);
	WriteDataToBufferPointer(buf, TheCamera.GetPosition().z);
	WriteDataToBufferPointer(buf, CClock::ms_nMillisecondsPerGameMinute);
	WriteDataToBufferPointer(buf, CClock::ms_nLastClockTick);
	WriteDataToBufferPointer(buf, CClock::ms_nGameClockHours);
	WriteDataToBufferPointer(buf, CClock::ms_nGameClockMinutes);
	currPad = CPad::GetPad(0);
	WriteDataToBufferPointer(buf, currPad->Mode);
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
	WriteDataToBufferPointer(buf, CompileDateAndTime.m_nSecond);
	WriteDataToBufferPointer(buf, CompileDateAndTime.m_nMinute);
	WriteDataToBufferPointer(buf, CompileDateAndTime.m_nHour);
	WriteDataToBufferPointer(buf, CompileDateAndTime.m_nDay);
	WriteDataToBufferPointer(buf, CompileDateAndTime.m_nMonth);
	WriteDataToBufferPointer(buf, CompileDateAndTime.m_nYear);
	WriteDataToBufferPointer(buf, CWeather::WeatherTypeInList);
#ifdef COMPATIBLE_SAVES
	// converted to float for compatibility with original format
	// TODO: maybe remove this? not really gonna break anything vital
	float f = TheCamera.CarZoomIndicator;
	WriteDataToBufferPointer(buf, f);
	f = TheCamera.PedZoomIndicator;
	WriteDataToBufferPointer(buf, f);
#else
	WriteDataToBufferPointer(buf, TheCamera.CarZoomIndicator);
	WriteDataToBufferPointer(buf, TheCamera.PedZoomIndicator);
#endif
	assert(buf - work_buff == SIZE_OF_SIMPLEVARS);

	// Save scripts, block is nested within the same block as simple vars for some reason
	presize = buf;
	buf += 4;
	postsize = buf;
	CTheScripts::SaveAllScripts(buf, &size);
	CopySizeAndPreparePointer(presize, buf, postsize, reserved, size);
	if (!PcSaveHelper.PcClassSaveRoutine(file, work_buff, buf - work_buff))
		return false;

	totalSize = buf - work_buff;

	// Save the rest
	WriteSaveDataBlock(CPools::SavePedPool);
	WriteSaveDataBlock(CGarages::Save);
	WriteSaveDataBlock(CGameLogic::Save);
	WriteSaveDataBlock(CPools::SaveVehiclePool);
	WriteSaveDataBlock(CPools::SaveObjectPool);
	WriteSaveDataBlock(ThePaths.Save);
	WriteSaveDataBlock(CCranes::Save);
	WriteSaveDataBlock(CPickups::Save);
	WriteSaveDataBlock(gPhoneInfo.Save);
	WriteSaveDataBlock(CRestart::SaveAllRestartPoints);
	WriteSaveDataBlock(CRadar::SaveAllRadarBlips);
	WriteSaveDataBlock(CTheZones::SaveAllZones);
	WriteSaveDataBlock(CGangs::SaveAllGangData);
	WriteSaveDataBlock(CTheCarGenerators::SaveAllCarGenerators);
	WriteSaveDataBlock(CParticleObject::SaveParticle);
	WriteSaveDataBlock(cAudioScriptObject::SaveAllAudioScriptObjects);
	WriteSaveDataBlock(CWorld::Players[CWorld::PlayerInFocus].SavePlayerInfo);
	WriteSaveDataBlock(CStats::SaveStats);
	WriteSaveDataBlock(CSetPieces::Save);
	WriteSaveDataBlock(CStreaming::MemoryCardSave);
	WriteSaveDataBlock(CPedType::Save);

	// Write padding
	for (int i = 0; i < 4; i++) {
		size = align4bytes(SIZE_OF_ONE_GAME_IN_BYTES - totalSize - 4);
		if (size > sizeof(work_buff))
			size = sizeof(work_buff);
		if (size > 4) {
			if (!PcSaveHelper.PcClassSaveRoutine(file, work_buff, size))
				return false;
			totalSize += size;
		}
	}
	
	// Write checksum and close
	CFileMgr::Write(file, (const char *) &CheckSum, sizeof(CheckSum));
	if (CFileMgr::GetErrorReadWrite(file)) {
		PcSaveHelper.nErrorCode = SAVESTATUS_ERR_SAVE_WRITE;
		if (!CloseFile(file))
			PcSaveHelper.nErrorCode = SAVESTATUS_ERR_SAVE_CLOSE;

		return false;
	}
	
	return true;
}

bool
GenericLoad()
{
	uint8 *buf;
	int32 file;
	uint32 size;

	int32 saveSize;
	CPad *currPad;

	// Load SimpleVars and Scripts
	CheckSum = 0;
	CDate dummy; // unused
	CPad::ResetCheats();
	if (!ReadInSizeofSaveFileBuffer(file, size))
		return false;
	size = align4bytes(size);
	ReadDataFromFile(file, work_buff, size);
	buf = (work_buff + 0x40);
	ReadDataFromBufferPointer(buf, saveSize);
	ReadDataFromBufferPointer(buf, CGame::currLevel);
	ReadDataFromBufferPointer(buf, TheCamera.GetMatrix().GetPosition().x);
	ReadDataFromBufferPointer(buf, TheCamera.GetMatrix().GetPosition().y);
	ReadDataFromBufferPointer(buf, TheCamera.GetMatrix().GetPosition().z);
	ReadDataFromBufferPointer(buf, CClock::ms_nMillisecondsPerGameMinute);
	ReadDataFromBufferPointer(buf, CClock::ms_nLastClockTick);
	ReadDataFromBufferPointer(buf, CClock::ms_nGameClockHours);
	ReadDataFromBufferPointer(buf, CClock::ms_nGameClockMinutes);
	currPad = CPad::GetPad(0);
	ReadDataFromBufferPointer(buf, currPad->Mode);
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
	ReadDataFromBufferPointer(buf, CompileDateAndTime.m_nSecond);
	ReadDataFromBufferPointer(buf, CompileDateAndTime.m_nMinute);
	ReadDataFromBufferPointer(buf, CompileDateAndTime.m_nHour);
	ReadDataFromBufferPointer(buf, CompileDateAndTime.m_nDay);
	ReadDataFromBufferPointer(buf, CompileDateAndTime.m_nMonth);
	ReadDataFromBufferPointer(buf, CompileDateAndTime.m_nYear);
	ReadDataFromBufferPointer(buf, CWeather::WeatherTypeInList);
#ifdef COMPATIBLE_SAVES
	// converted to float for compatibility with original format
	// TODO: maybe remove this? not really gonna break anything vital
	float f;
	ReadDataFromBufferPointer(buf, f);
	TheCamera.CarZoomIndicator = f;
	ReadDataFromBufferPointer(buf, f);
	TheCamera.PedZoomIndicator = f;
#else
	ReadDataFromBufferPointer(buf, TheCamera.CarZoomIndicator);
	ReadDataFromBufferPointer(buf, TheCamera.PedZoomIndicator);
#endif
	assert(buf - work_buff == SIZE_OF_SIMPLEVARS);
	ReadDataFromBlock("Loading Scripts \n", CTheScripts::LoadAllScripts);

	// Load the rest
	LoadSaveDataBlock();
	ReadDataFromBlock("Loading PedPool \n", CPools::LoadPedPool);
	LoadSaveDataBlock();
	ReadDataFromBlock("Loading Garages \n", CGarages::Load);
	LoadSaveDataBlock();
	ReadDataFromBlock("Loading GameLogic \n", CGameLogic::Load);
	LoadSaveDataBlock();
	ReadDataFromBlock("Loading Vehicles \n", CPools::LoadVehiclePool);
	LoadSaveDataBlock();
	CProjectileInfo::RemoveAllProjectiles();
	CObject::DeleteAllTempObjects();
	ReadDataFromBlock("Loading Objects \n", CPools::LoadObjectPool);
	LoadSaveDataBlock();
	ReadDataFromBlock("Loading Paths \n", ThePaths.Load);
	LoadSaveDataBlock();
	ReadDataFromBlock("Loading Cranes \n", CCranes::Load);
	LoadSaveDataBlock();
	ReadDataFromBlock("Loading Pickups \n", CPickups::Load);
	LoadSaveDataBlock();
	ReadDataFromBlock("Loading Phoneinfo \n", gPhoneInfo.Load);
	LoadSaveDataBlock();
	ReadDataFromBlock("Loading Restart \n", CRestart::LoadAllRestartPoints);
	LoadSaveDataBlock();
	ReadDataFromBlock("Loading Radar Blips \n", CRadar::LoadAllRadarBlips);
	LoadSaveDataBlock();
	ReadDataFromBlock("Loading Zones \n", CTheZones::LoadAllZones);
	LoadSaveDataBlock();
	ReadDataFromBlock("Loading Gang Data \n", CGangs::LoadAllGangData);
	LoadSaveDataBlock();
	ReadDataFromBlock("Loading Car Generators \n", CTheCarGenerators::LoadAllCarGenerators);
	CParticle::ReloadConfig();
	LoadSaveDataBlock();
	ReadDataFromBlock("Loading Particles \n", CParticleObject::LoadParticle);
	LoadSaveDataBlock();
	ReadDataFromBlock("Loading AudioScript Objects \n", cAudioScriptObject::LoadAllAudioScriptObjects);
	LoadSaveDataBlock();
	ReadDataFromBlock("Loading Player Info \n", CWorld::Players[CWorld::PlayerInFocus].LoadPlayerInfo);
	LoadSaveDataBlock();
	ReadDataFromBlock("Loading Stats \n", CStats::LoadStats);
	LoadSaveDataBlock();
	ReadDataFromBlock("Loading Set Pieces \n", CSetPieces::Load);
	LoadSaveDataBlock();
	ReadDataFromBlock("Loading Streaming Stuff \n", CStreaming::MemoryCardLoad);
	LoadSaveDataBlock();
	ReadDataFromBlock("Loading PedType Stuff \n", CPedType::Load);

	DMAudio.SetMusicMasterVolume(FrontEndMenuManager.m_PrefsMusicVolume);
	DMAudio.SetEffectsMasterVolume(FrontEndMenuManager.m_PrefsSfxVolume);
	if (!CloseFile(file)) {
		PcSaveHelper.nErrorCode = SAVESTATUS_ERR_LOAD_CLOSE;
		return false;
	}

	DoGameSpecificStuffAfterSucessLoad();
	debug("Game successfully loaded \n");
	return true;
}

bool
ReadInSizeofSaveFileBuffer(int32 &file, uint32 &size)
{
	file = CFileMgr::OpenFile(LoadFileName, "rb");
	if (file == 0) {
		PcSaveHelper.nErrorCode = SAVESTATUS_ERR_LOAD_OPEN;
		return false;
	}
	CFileMgr::Read(file, (const char*)&size, sizeof(size));
	if (CFileMgr::GetErrorReadWrite(file)) {
		PcSaveHelper.nErrorCode = SAVESTATUS_ERR_LOAD_READ;
		if (!CloseFile(file))
			PcSaveHelper.nErrorCode = SAVESTATUS_ERR_LOAD_CLOSE;
		return false;
	}
	return true;
}

bool
ReadDataFromFile(int32 file, uint8 *buf, uint32 size)
{
	if (file == 0) {
		PcSaveHelper.nErrorCode = SAVESTATUS_ERR_LOAD_OPEN;
		return false;
	}
	size_t read_size = CFileMgr::Read(file, (const char*)buf, size);
	if (CFileMgr::GetErrorReadWrite(file) || read_size != size) {
		PcSaveHelper.nErrorCode = SAVESTATUS_ERR_LOAD_READ;
		if (!CloseFile(file))
			PcSaveHelper.nErrorCode = SAVESTATUS_ERR_LOAD_CLOSE;
		return false;
	}
	return true;
}

bool
CloseFile(int32 file)
{
	return CFileMgr::CloseFile(file) == 0;
}

void
DoGameSpecificStuffAfterSucessLoad()
{
	StillToFadeOut = true;
	JustLoadedDontFadeInYet = true;
	CTheScripts::Process();
}

bool
CheckSlotDataValid(int32 slot)
{
	PcSaveHelper.nErrorCode = SAVESTATUS_SUCCESSFUL;
	if (CheckDataNotCorrupt(slot, LoadFileName)) {
		CStreaming::DeleteAllRwObjects();
		return true;
	}

	PcSaveHelper.nErrorCode = SAVESTATUS_ERR_DATA_INVALID;
	return false;
}

void
MakeSpaceForSizeInBufferPointer(uint8 *&presize, uint8 *&buf, uint8 *&postsize)
{
	presize = buf;
	buf += sizeof(uint32);
	postsize = buf;
}

void
CopySizeAndPreparePointer(uint8 *&buf, uint8 *&postbuf, uint8 *&postbuf2, uint32 &unused, uint32 &size)
{
	memcpy(buf, &size, sizeof(size));
	size = align4bytes(size);
	postbuf2 += size;
	postbuf = postbuf2;
}

void
DoGameSpecificStuffBeforeSave()
{
	CGameLogic::PassTime(360);
	CPlayerPed *ped = FindPlayerPed();
	ped->m_fCurrentStamina = ped->m_fMaxStamina;
	CGame::TidyUpMemory(true, false);
}


void
MakeValidSaveName(int32 slot)
{
	ValidSaveName[0] = '\0';
	sprintf(ValidSaveName, "%s%i", DefaultPCSaveFileName, slot + 1);
	strncat(ValidSaveName, ".b", 5);
}

wchar *
GetSavedGameDateAndTime(int32 slot)
{
	return SlotSaveDate[slot];
}

wchar *
GetNameOfSavedGame(int32 slot)
{
	return SlotFileName[slot];
}

bool
CheckDataNotCorrupt(int32 slot, char *name)
{
	char filename[100];

	int32 blocknum = 0;
	eLevelName level = LEVEL_NONE;
	CheckSum = 0;
	uint32 bytes_processed = 0;
	sprintf(filename, "%s%i%s", DefaultPCSaveFileName, slot + 1, ".b");
	int file = CFileMgr::OpenFile(filename, "rb");
	if (file == 0)
		return false;
	strcpy(name, filename);
	while (SIZE_OF_ONE_GAME_IN_BYTES - sizeof(uint32) > bytes_processed && blocknum < 40) {
		int32 blocksize;
		if (!ReadDataFromFile(file, (uint8*)&blocksize, sizeof(blocksize))) {
			CloseFile(file);
			return false;
		}
		if (blocksize > align4bytes(sizeof(work_buff)))
			blocksize = sizeof(work_buff) - sizeof(uint32);
		if (!ReadDataFromFile(file, work_buff, align4bytes(blocksize))) {
			CloseFile(file);
			return false;
		}

		CheckSum += ((uint8*)&blocksize)[0];
		CheckSum += ((uint8*)&blocksize)[1];
		CheckSum += ((uint8*)&blocksize)[2];
		CheckSum += ((uint8*)&blocksize)[3];
		uint8 *_work_buf = work_buff;
		for (int i = 0; i < align4bytes(blocksize); i++) {
			CheckSum += *_work_buf++;
			bytes_processed++;
		}

		if (blocknum == 0)
			memcpy(&level, work_buff+4, sizeof(level));
		blocknum++;
	}
	int32 _checkSum;
	if (ReadDataFromFile(file, (uint8*)&_checkSum, sizeof(_checkSum))) {
		if (CloseFile(file)) {
			if (CheckSum == _checkSum) {
				m_LevelToLoad = level;
				return true;
			}
			return false;
		}
		return false;
	}

	CloseFile(file);
	return false;
}

bool
RestoreForStartLoad()
{
	uint8 buf[999];

	int file = CFileMgr::OpenFile(LoadFileName, "rb");
	if (file == 0) {
		PcSaveHelper.nErrorCode = SAVESTATUS_ERR_LOAD_OPEN;
		return false;
	}
	ReadDataFromFile(file, buf, sizeof(buf));
	if (CFileMgr::GetErrorReadWrite(file)) {
		PcSaveHelper.nErrorCode = SAVESTATUS_ERR_LOAD_READ;
		if (!CloseFile(file))
			PcSaveHelper.nErrorCode = SAVESTATUS_ERR_LOAD_CLOSE;
		return false;
	} else {
		uint8 *_buf = buf + sizeof(int32) + sizeof(wchar[24]) + sizeof(SYSTEMTIME) + sizeof(SIZE_OF_ONE_GAME_IN_BYTES);
		ReadDataFromBufferPointer(_buf, CGame::currLevel);
		ReadDataFromBufferPointer(_buf, TheCamera.GetMatrix().GetPosition().x);
		ReadDataFromBufferPointer(_buf, TheCamera.GetMatrix().GetPosition().y);
		ReadDataFromBufferPointer(_buf, TheCamera.GetMatrix().GetPosition().z);
		CStreaming::RemoveUnusedBigBuildings(CGame::currLevel);
		CStreaming::RemoveUnusedBuildings(CGame::currLevel);

		if (CloseFile(file)) {
			return true;
		} else {
			PcSaveHelper.nErrorCode = SAVESTATUS_ERR_LOAD_CLOSE;
			return false;
		}
	}
}

int
align4bytes(int32 size)
{
	return (size + 3) & 0xFFFFFFFC;
}
