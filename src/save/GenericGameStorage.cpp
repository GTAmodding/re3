#include "common.h"
#include "main.h"
#include "patcher.h"
#include "AudioScriptObject.h"
#include "Camera.h"
#include "CarGen.h"
#include "Cranes.h"
#include "Clock.h"
#include "Date.h"
#include "FileMgr.h"
#include "GameLogic.h"
#include "Gangs.h"
#include "Garages.h"
#include "GenericGameStorage.h"
#include "Pad.h"
#include "ParticleObject.h"
#include "PathFind.h"
#include "PCSave.h"
#include "Phones.h"
#include "Pickups.h"
#include "PlayerPed.h"
#include "Pools.h"
#include "Radar.h"
#include "Restart.h"
#include "Script.h"
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

char (&DefaultPCSaveFileName)[260] = *(char(*)[260])*(uintptr*)0x8E28C0;
char (&ValidSaveName)[260] = *(char(*)[260])*(uintptr*)0x8E2CBC;
char (&LoadFileName)[256] = *(char(*)[256])*(uintptr*)0x9403C4;
wchar (&SlotFileName)[SLOT_COUNT][260] = *(wchar(*)[SLOT_COUNT][260])*(uintptr*)0x6F07C8;
wchar (&SlotSaveDate)[SLOT_COUNT][70] = *(wchar(*)[SLOT_COUNT][70])*(uintptr*)0x72B858;
int &CheckSum = *(int*)0x8E2BE0;
eLevelName &m_LevelToLoad = *(eLevelName*)0x8E29CC;
char SaveFileNameJustSaved[260];
int (&Slots)[SLOT_COUNT+1] = *(int(*)[SLOT_COUNT+1])*(uintptr*)0x72803C;
CDate &CompileDateAndTime = *(CDate*)0x72BCB8;

#define ReadDataFromBufferPointer(buf, to) memcpy(&to, buf, sizeof(to)); buf += align4bytes(sizeof(to));
#define WriteDataToBufferPointer(buf, from) memcpy(buf, &from, sizeof(from)); buf += align4bytes(sizeof(from));

//WRAPPER bool GenericSave(int file) { EAXJMP(0x58F8D0); }
WRAPPER bool GenericLoad() { EAXJMP(0x590A00); }


#define WRITE_BLOCK(save_func)\
do {\
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
	if (*lastMissionPassed) {
		AsciiToUnicode("...'", suffix);
		TextCopy(saveName, lastMissionPassed);
		int len = UnicodeStrlen(saveName);
		saveName[len] = '\0';
		if (len > 22)
			TextCopy(saveName + 18, suffix);
		saveName[23] = '\0';
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
	WriteDataToBufferPointer(buf, CTimer::GetTimeInMilliseconds());
	WriteDataToBufferPointer(buf, CTimer::GetTimeScale());
	WriteDataToBufferPointer(buf, CTimer::GetTimeStep());
	WriteDataToBufferPointer(buf, CTimer::GetTimeStepNonClipped());
	WriteDataToBufferPointer(buf, CTimer::GetFrameCounter());
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
	WriteDataToBufferPointer(buf, TheCamera.CarZoomIndicator);
	WriteDataToBufferPointer(buf, TheCamera.PedZoomIndicator);

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
	WRITE_BLOCK(CPools::SavePedPool);
	WRITE_BLOCK(CGarages::Save);
	WRITE_BLOCK(CPools::SaveVehiclePool);
	WRITE_BLOCK(CPools::SaveObjectPool);
	WRITE_BLOCK(ThePaths.Save);
	WRITE_BLOCK(CCranes::Save);
	WRITE_BLOCK(CPickups::Save);
	WRITE_BLOCK(gPhoneInfo.Save);
	WRITE_BLOCK(CRestart::SaveAllRestartPoints);
	WRITE_BLOCK(CRadar::SaveAllRadarBlips);
	WRITE_BLOCK(CTheZones::SaveAllZones);
	WRITE_BLOCK(CGangs::SaveAllGangData);
	WRITE_BLOCK(CTheCarGenerators::SaveAllCarGenerators);
	WRITE_BLOCK(CParticleObject::SaveParticle);
	WRITE_BLOCK(cAudioScriptObject::SaveAllAudioScriptObjects);
	WRITE_BLOCK(CWorld::Players[CWorld::PlayerInFocus].SavePlayerInfo);
	WRITE_BLOCK(CStats::SaveStats);
	WRITE_BLOCK(CStreaming::MemoryCardSave);
	WRITE_BLOCK(CPedType::Save);

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
		ReadDataFromBufferPointer(_buf, TheCamera.GetPosition().x);
		ReadDataFromBufferPointer(_buf, TheCamera.GetPosition().y);
		ReadDataFromBufferPointer(_buf, TheCamera.GetPosition().z);
		CStreaming::RemoveUnusedBigBuildings(CGame::currLevel);
		CStreaming::RemoveUnusedBuildings(CGame::currLevel);
		CCollision::SortOutCollisionAfterLoad();
		CStreaming::RequestBigBuildings(CGame::currLevel);
		CStreaming::LoadAllRequestedModels(false);
		CStreaming::HaveAllBigBuildingsLoaded(CGame::currLevel);
		CGame::TidyUpMemory(true, false);

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

STARTPATCHES
	InjectHook(0x58F8D0, GenericSave, PATCH_JUMP);
	//InjectHook(0x590A00, GenericLoad, PATCH_JUMP);
	InjectHook(0x591910, ReadInSizeofSaveFileBuffer, PATCH_JUMP);
	InjectHook(0x591990, ReadDataFromFile, PATCH_JUMP);
	InjectHook(0x591A00, CloseFile, PATCH_JUMP);
	InjectHook(0x591A20, DoGameSpecificStuffAfterSucessLoad, PATCH_JUMP);
	InjectHook(0x591A40, CheckSlotDataValid, PATCH_JUMP);
	InjectHook(0x591A80, MakeSpaceForSizeInBufferPointer, PATCH_JUMP);
	InjectHook(0x591AA0, CopySizeAndPreparePointer, PATCH_JUMP);
	InjectHook(0x591AE0, DoGameSpecificStuffBeforeSave, PATCH_JUMP);
	InjectHook(0x591B10, MakeValidSaveName, PATCH_JUMP);
	InjectHook(0x591B50, GetSavedGameDateAndTime, PATCH_JUMP);
	InjectHook(0x591B60, GetNameOfSavedGame, PATCH_JUMP);
	InjectHook(0x591B70, CheckDataNotCorrupt, PATCH_JUMP);
	InjectHook(0x591D60, RestoreForStartLoad, PATCH_JUMP);
	InjectHook(0x591E80, align4bytes, PATCH_JUMP);
ENDPATCHES