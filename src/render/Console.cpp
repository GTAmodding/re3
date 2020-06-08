#include "common.h"
#include <stdarg.h>

#include "Console.h"
#include "Font.h"
#include "Timer.h"

#define CONSOLE_X_POS (30.0f)
#define CONSOLE_Y_POS (10.0f)
#define CONSOLE_LINE_HEIGHT (12.0f)

CConsole TheConsole;

void
CConsole::AddLine(char *s, uint8 r, uint8 g, uint8 b)
{
	char tempstr[MAX_STR_LEN+1];

	while (strlen(s) > MAX_STR_LEN) {
		strncpy(tempstr, s, MAX_STR_LEN);
		tempstr[MAX_STR_LEN-1] = '\0';
		s += MAX_STR_LEN - 1;
		AddOneLine(tempstr, r, g, b);
	}
	AddOneLine(s, r, g, b);
}

void
CConsole::AddOneLine(char *s, uint8 r, uint8 g, uint8 b)
{
	int32 StrIndex = (m_nLineCount + m_nCurrentLine) % MAX_LINES;

	for (int32 i = 0; i < MAX_STR_LEN; i++) {
		Buffers[StrIndex][i] = s[i];
		if (s[i] == '\0') break;
	}

	uint8 _strNum1 = m_nLineCount;
	if (_strNum1 < MAX_LINES)
		_strNum1++;

	m_aTimer[StrIndex] = CTimer::GetTimeInMilliseconds();
	Buffers[StrIndex][MAX_STR_LEN-1] = '\0';
	m_aRed[StrIndex] = r;
	m_aGreen[StrIndex] = g;
	m_aBlue[StrIndex] = b;

	if (_strNum1 >= MAX_LINES)
		m_nCurrentLine = (m_nCurrentLine + 1) % MAX_LINES;
	else
		m_nLineCount = _strNum1;

}

void
CConsole::Display()
{
	CFont::SetPropOn();
	CFont::SetBackgroundOff();
	CFont::SetScale(0.6f, 0.6f); 
	CFont::SetCentreOff();
	CFont::SetRightJustifyOff();
	CFont::SetJustifyOn();
	CFont::SetRightJustifyWrap(0.0f);
	CFont::SetBackGroundOnlyTextOff();
	CFont::SetFontStyle(FONT_STANDARD);
#ifndef FIX_BUGS
	CFont::SetPropOff(); // not sure why this is here anyway
#endif
	CFont::SetWrapx(RsGlobal.width);

	while (m_nLineCount != 0 && CTimer::GetTimeInMilliseconds() - m_aTimer[m_nCurrentLine] > 20000) {
		m_nLineCount--;
		m_nCurrentLine = (m_nCurrentLine + 1) % MAX_LINES;
	}

	for (int16 i = 0; i < m_nLineCount; i++) {
		int16 line = (i + m_nCurrentLine) % MAX_LINES;
		CFont::SetColor(CRGBA(0, 0, 0, 200));
		CFont::PrintString(CONSOLE_X_POS + 1.0f, CONSOLE_Y_POS + 1.0f + i * CONSOLE_LINE_HEIGHT, Buffers[line]);
		CFont::SetColor(CRGBA(m_aRed[line], m_aGreen[line], m_aBlue[line], 200));
		CFont::PrintString(CONSOLE_X_POS, CONSOLE_Y_POS + i * CONSOLE_LINE_HEIGHT, Buffers[line]);
	}
}

void
cprintf(char* format, ...)
{
	char s[256];
	va_list vl1, vl2;

	va_start(vl1, format);
	va_copy(vl2, vl1);
	vsprintf(s, format, vl1);
	TheConsole.AddLine(s, 255, 255, 128);
}
