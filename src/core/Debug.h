#pragma once

class CDebug
{
	enum
	{
		MAX_LINES = 15,
		MAX_STR_LEN = 80,

		MAX_SCREEN_STRS = 100,
	};

	static int16 ms_nCurrentTextLine;
	static char ms_aTextBuffer[MAX_LINES][MAX_STR_LEN];

	// custom
	struct ScreenStr {
		int x, y;
		char str[256];
	};
	static ScreenStr ms_aScreenStrs[MAX_SCREEN_STRS];
	static int ms_nScreenStrs;

public:
	static void DebugInitTextBuffer();
	static void DebugDisplayTextBuffer();
	static void DebugAddText(const char *str);

	// custom
	static void PrintAt(const char *str, int x, int y);
	static void DisplayScreenStrings();
};

extern bool gbDebugStuffInRelease;
