#pragma once

#ifdef PS2_MENU
#include "Date.h"

#if defined(PS2)
#include <libcdvd.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

enum
{
	CARD_ONE = 0,
	CARD_TWO,
	MAX_CARDS,
};

class CMemoryCardInfo
{
public:
	int port;
	int slot;
	int type;
	int free;
	int format;
	char dir[40];
#if defined(PS2)
	sceMcTblGetDir table[15];
#else
	struct
	{
		typedef struct {unsigned char Sec,Min,Hour; unsigned char Day,Month; unsigned short Year;} _time;
		_time _Create;
		_time _Modify;
		unsigned int FileSizeByte;
		unsigned short AttrFile;
		unsigned char EntryName[32];
	}table[15];
#endif
	CMemoryCardInfo(void);
};


#define GUFF_FILE_SIZE 147096
#define SAVE_FILE_SIZE 201729

class CMemoryCard
{
public:
	enum
	{
		MAX_SLOTS = 8,
	};
	
	enum MCSTATE
	{
		MCSTATE_OK = 0,
		MCSTATE_NEED_500KB,
		MCSTATE_NEED_200KB,
		MCSTATE_NOCARD,
	};
	
	enum SLOTINFO
	{
		SLOT_PRESENT = 0,
		SLOT_NOTPRESENT,
		SLOT_CORRUPTED,
	};

	int             _unk0;
	int             _unk1;
	bool            m_bWantToLoad;
	bool            JustLoadedDontFadeInYet;
	bool            StillToFadeOut;
	bool            b_FoundRecentSavedGameWantToLoad;
	uint32          TimeStartedCountingForFade;
	uint32          TimeToStayFadedBeforeFadeOut;
	uint32          LastBlockSize;
	bool            _bunk2;
	char            ValidSaveName        [30];
	char            MostRecentFile       [30];
	char            _unkName3            [30];
	char            SaveFileNameJustSaved[30];
	char _pad0[3];
	wchar          *pErrorMsg;
	char            _unk4[32];
	bool            _bunk5;
	bool            _bunk6;
	bool            _bunk7;
	bool            _bunk8;
	int             nError;
	wchar           _unk9[30];
	char            LoadFileName[30];
	char _pad1[2];
	CDate           CompileDateAndTime;
	int             m_LanguageToLoad;
	int             m_LevelToLoad;
	int             CurrentCard;
	CMemoryCardInfo Cards       [MAX_CARDS];
	int             Slots       [MAX_SLOTS];
	wchar           SlotFileName[MAX_SLOTS][30];
	wchar           SlotSaveDate[MAX_SLOTS][30];
	char            _unk10[32];
			
	enum
	{
		ERR_NONE = 0,
		ERR_NOFORMAT = 1,
		ERR_DIRNOENTRY = 2,
		ERR_OPENNOENTRY = 3,
		ERR_DELETENOENTRY = 4,
		ERR_DELETEDENIED = 5,
		ERR_DELETEFAILED = 6,
		ERR_WRITEFULLDEVICE = 7,
		ERR_WRITENOENTRY = 8,
		ERR_WRITEDENIED = 9,
		ERR_FLUSHNOENTRY,
		ERR_WRITEFAILED,
		ERR_FORMATFAILED = 12,
		ERR_FILETABLENOENTRY = 13,
		ERR_DIRFULLDEVICE = 14,
		ERR_DIRBADENTRY = 15,
		ERR_FILEFULLDEVICE = 16,
		ERR_FILENOPATHENTRY = 17,
		ERR_FILEDENIED = 18,
		ERR_FILEUPLIMIT = 19,
		ERR_READNOENTRY = 20,
		ERR_READDENIED = 21,
		ERR_LOADFAILED = 22, // unused
		ERR_SAVEFAILED = 23,
		ERR_DATACORRUPTED = 24,
		ERR_NOROOTDIR = 25,
		NO_ERR_SUCCESS = 26,
	};
	
	enum
	{
		RES_SUCCESS = 1,
		RES_FAILED  = -1,
	};

	int32 GetError()
	{
		return nError;
	}

	wchar *GetErrorMessage()
	{
		return pErrorMsg;
	}
	
	int32 Init(void);
	CMemoryCard(void);
	int32 RestoreForStartLoad(void);
	int32 LoadSavedGame(void);
	int32 CheckCardInserted(int32 cardID);
	int32 PopulateCardFlags(int32 cardID, bool bSlotFlag, bool bTypeFlag, bool bFreeFlag, bool bFormatFlag);
	int32 FormatCard(int32 cardID);
	int32 PopulateFileTable(int32 cardID);
	int32 CreateRootDirectory(int32 cardID);
	int32 ChangeDirectory(int32 cardID, char *dir);
	int32 CreateIconFiles(int32 cardID, char *icon_one, char *icon_two, char *icon_three);
	int32 LoadIconFiles(int32 cardID, char *icon_one, char *icon_two, char *icon_three);
	int32 CloseMemCardFile(int32 file);
	int32 CreateMemCardFileReadWrite(int32 cardID, char *filename);
	int32 OpenMemCardFileForReading(int32 cardID, char *filename);
	int32 ReadFromMemCard(int32 file, void *buff, int32 size);
	int32 DeleteMemoryCardFile(int32 cardID, char *filename);
	void PopulateErrorMessage();
	int32 WritetoMemCard(int32 file, void *buff, int32 size);
	bool SaveGame(void);
	bool DoHackRoundSTUPIDSonyDateTimeStuff(int32 port, char *filename);
	int32 LookForRootDirectory(int32 cardID);
	int32 FillFirstFileWithGuff(int32 cardID);
	bool FindMostRecentFileName(int32 cardID, char *filename);
	void ClearFileTableBuffer(int32 cardID);
	int32 GetClusterAmountForFileCreation(int32 port);
	bool DeleteEverythingInGameRoot(int32 cardID);
	int32 CheckDataNotCorrupt(char *filename);
	int32 GetLanguageToLoad(void);
	int32 GetLevelToLoad(void);
	bool CreateGameDirectoryFromScratch(int32 cardID);
	bool CheckGameDirectoryThere(int32 cardID);
	void PopulateSlotInfo(int32 cardID);
	int32 GetInfoOnSpecificSlot(int32 slotID);
	wchar *GetDateAndTimeOfSavedGame(int32 slotID);
	int32 CheckCardStateAtGameStartUp(int32 cardID);
	void SaveSlot(int32 slotID);
	void DeleteSlot(int32 slotID);
	void LoadSlotToBuffer(int32 slotID);
	wchar *GetNameOfSavedGame(int32 slotID);
	int32 DoClassSaveRoutine(int32 file, uint8 *data, uint32 size);
};

extern CMemoryCard TheMemoryCard;
#endif