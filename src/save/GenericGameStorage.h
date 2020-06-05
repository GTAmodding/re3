#pragma once

#include "Game.h"
#include "PCSave.h"

#define SLOT_COUNT (8)

bool GenericSave(int file);
bool GenericLoad();
bool ReadInSizeofSaveFileBuffer(int32 &file, uint32 &size);
bool ReadDataFromFile(int32 file, uint8 *buf, uint32 size);
bool CloseFile(int32 file);
void DoGameSpecificStuffAfterSucessLoad();
bool CheckSlotDataValid(int32 slot);
void MakeSpaceForSizeInBufferPointer(uint8 *&presize, uint8 *&buf, uint8 *&postsize);
void CopySizeAndPreparePointer(uint8 *&buf, uint8 *&postbuf, uint8 *&postbuf2, uint32 &unused, uint32 &size);
void DoGameSpecificStuffBeforeSave();
void MakeValidSaveName(int32 slot);
wchar *GetSavedGameDateAndTime(int32 slot);
wchar *GetNameOfSavedGame(int32 slot);
bool CheckDataNotCorrupt(int32 slot, char *name);
bool RestoreForStartLoad();
int align4bytes(int32 size);

extern class CDate CompileDateAndTime;

extern char DefaultPCSaveFileName[260];
extern char ValidSaveName[260];
extern char LoadFileName[256];
extern wchar SlotFileName[SLOT_COUNT][260];
extern wchar SlotSaveDate[SLOT_COUNT][70];
extern int CheckSum;
extern enum eLevelName m_LevelToLoad;
extern int Slots[SLOT_COUNT+1];

extern bool b_FoundRecentSavedGameWantToLoad;
extern bool JustLoadedDontFadeInYet;
extern bool StillToFadeOut;
extern uint32 TimeStartedCountingForFade;
extern uint32 TimeToStayFadedBeforeFadeOut;

extern char SaveFileNameJustSaved[260]; // 8F2570

const char TopLineEmptyFile[] = "THIS FILE IS NOT VALID YET";

#ifdef MISSION_REPLAY
extern int8 IsQuickSave; // originally int

bool SaveGameForPause(int);
#endif
