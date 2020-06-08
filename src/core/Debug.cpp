#include "common.h"
#include "RwHelper.h"
#include "Debug.h"
#include "Lines.h"
#include "Font.h"
#include "main.h"
#include "Text.h"

bool gbDebugStuffInRelease = false;

#define DEBUG_X_POS (300)
#define DEBUG_Y_POS (41)
#define DEBUG_LINE_HEIGHT (22)

int16 CDebug::ms_nCurrentTextLine;
char CDebug::ms_aTextBuffer[MAX_LINES][MAX_STR_LEN];

void
CDebug::DebugInitTextBuffer()
{
	ms_nCurrentTextLine = 0;
}

void
CDebug::DebugAddText(const char *str)
{
	int32 i = 0;
	if (*str != '\0') {
		while (i < MAX_STR_LEN - 1) {
			ms_aTextBuffer[ms_nCurrentTextLine][i++] = *(str++);
			if (*str == '\0')
				break;
		}
	}

	ms_aTextBuffer[ms_nCurrentTextLine++][i] = '\0';
	if (ms_nCurrentTextLine >= MAX_LINES)
		ms_nCurrentTextLine = 0;
}

void
CDebug::DebugDisplayTextBuffer()
{
#ifndef MASTER
	if (gbDebugStuffInRelease)
	{
		int32 i = 0;
		int32 y = DEBUG_Y_POS;
#ifdef FIX_BUGS
		CFont::SetPropOn();
		CFont::SetBackgroundOff();
		CFont::SetScale(1.0f, 1.0f);
		CFont::SetCentreOff();
		CFont::SetRightJustifyOff();
		CFont::SetJustifyOn();
		CFont::SetRightJustifyWrap(0.0f);
		CFont::SetBackGroundOnlyTextOff();
		CFont::SetFontStyle(FONT_STANDARD);
#else
		// this is not even readable
		CFont::SetPropOff();
		CFont::SetBackgroundOff();
		CFont::SetScale(1.0f, 1.0f);
		CFont::SetCentreOff();
		CFont::SetRightJustifyOn();
		CFont::SetRightJustifyWrap(0.0f);
		CFont::SetBackGroundOnlyTextOff();
		CFont::SetFontStyle(FONT_STANDARD);
		CFont::SetPropOff();
#endif
		do {
			char *line;
			while (true) {
				line = ms_aTextBuffer[(ms_nCurrentTextLine + i++) % MAX_LINES];
				if (*line != '\0')
					break;
				y += DEBUG_LINE_HEIGHT;
				if (i == MAX_LINES) {
					CFont::DrawFonts();
					return;
				}
			}
			AsciiToUnicode(line, gUString);
			CFont::SetColor(CRGBA(0, 0, 0, 255));
			CFont::PrintString(DEBUG_X_POS, y-1, gUString);
			CFont::SetColor(CRGBA(255, 128, 128, 255));
			CFont::PrintString(DEBUG_X_POS+1, y, gUString);
			y += DEBUG_LINE_HEIGHT;
		} while (i != MAX_LINES);
		CFont::DrawFonts();
	}
#endif
}


// custom

CDebug::ScreenStr CDebug::ms_aScreenStrs[MAX_SCREEN_STRS];
int CDebug::ms_nScreenStrs;

void
CDebug::DisplayScreenStrings()
{
	int i;
	

	CFont::SetPropOn();
	CFont::SetBackgroundOff();
	CFont::SetScale(1.0f, 1.0f);
	CFont::SetCentreOff();
	CFont::SetRightJustifyOff();
	CFont::SetJustifyOff();
	CFont::SetRightJustifyWrap(0.0f);
	CFont::SetWrapx(9999.0f);
	CFont::SetBackGroundOnlyTextOff();
	CFont::SetFontStyle(FONT_STANDARD);

	for(i = 0; i < ms_nScreenStrs; i++){
/*
		AsciiToUnicode(ms_aScreenStrs[i].str, gUString);
		CFont::SetColor(CRGBA(0, 0, 0, 255));
		CFont::PrintString(ms_aScreenStrs[i].x, ms_aScreenStrs[i].y, gUString);
		CFont::SetColor(CRGBA(255, 255, 255, 255));
		CFont::PrintString(ms_aScreenStrs[i].x+1, ms_aScreenStrs[i].y+1, gUString);
*/
		ObrsPrintfString(ms_aScreenStrs[i].str, ms_aScreenStrs[i].x, ms_aScreenStrs[i].y);
	}
	CFont::DrawFonts();

	ms_nScreenStrs = 0;
}

void
CDebug::PrintAt(const char *str, int x, int y)
{
	if(ms_nScreenStrs >= MAX_SCREEN_STRS)
		return;
	strncpy(ms_aScreenStrs[ms_nScreenStrs].str, str, 256);
	ms_aScreenStrs[ms_nScreenStrs].x = x;//*12;
	ms_aScreenStrs[ms_nScreenStrs].y = y;//*22;
	ms_nScreenStrs++;
}

CDebug::Line CDebug::ms_aLines[MAX_DEBUG_LINES];
int CDebug::ms_nLines;

void
CDebug::AddLine(CVector p1, CVector p2, uint32 c1, uint32 c2)
{
	if(ms_nLines >= MAX_DEBUG_LINES)
		return;
	ms_aLines[ms_nLines].p1 = p1;
	ms_aLines[ms_nLines].p2 = p2;
	ms_aLines[ms_nLines].c1 = c1;
	ms_aLines[ms_nLines].c2 = c2;
	ms_nLines++;
}

void
CDebug::DrawLines(void)
{
	int i;
	RwRenderStateSet(rwRENDERSTATETEXTURERASTER, nil);
	RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*)FALSE);
	for(i = 0; i < ms_nLines; i++){
		Line *l = &ms_aLines[i];
		CLines::RenderLineWithClipping(l->p1.x, l->p1.y, l->p1.z, l->p2.x, l->p2.y, l->p2.z, l->c1, l->c2);
	}
	ms_nLines = 0;
}
