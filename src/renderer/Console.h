#pragma once

class CConsole
{
	enum
	{
		MAX_LINES = 8, // BUG? only shows 7
		MAX_STR_LEN = 40,
	};

	uint8 m_nLineCount;
	uint8 m_nCurrentLine;
	wchar Buffers[MAX_LINES][MAX_STR_LEN];
	uint32 m_aTimer[MAX_LINES];
	uint8 m_aRed[MAX_LINES];
	uint8 m_aGreen[MAX_LINES];
	uint8 m_aBlue[MAX_LINES];
public:
	void AddLine(char *s, uint8 r, uint8 g, uint8 b);
	void AddOneLine(char *s, uint8 r, uint8 g, uint8 b);
	void Display();
	void Init() { m_nCurrentLine = 0; m_nLineCount = 0; }
};

extern CConsole TheConsole;

void cprintf(char*, ...);