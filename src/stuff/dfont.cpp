#include "dcommon.h"
#include "common.h"
#include "rwcore.h"
#include "rwplcore.h"
#include "main.h" // Camera
#include "Font.h"

// Forward declaration
void AsciiToUnicode(const char *src, wchar *dst);

Pt fontPrint(const char *s, float xstart, float ystart, int style)
{
	AsciiToUnicode(s, gUString);

	CFont::SetPropOn();
	CFont::SetBackgroundOff();
	CFont::SetScale(0.65f, 0.65f);
	CFont::SetCentreOff();
	CFont::SetRightJustifyOff();
	CFont::SetJustifyOn();
	CFont::SetRightJustifyWrap(0.0f);
	CFont::SetBackGroundOnlyTextOff();
	CFont::SetFontStyle(FONT_BANK);

	// crashes
	switch (style)
	{
	case dFONT_NORMAL:
		CFont::SetColor(CRGBA(255, 255, 255, 255));
		break;
	case dFONT_SEL_ACTIVE:
		CFont::SetColor(CRGBA(200, 200, 200, 255));
		break;
	case dFONT_SEL_INACTIVE:
		CFont::SetColor(CRGBA(200, 200, 200, 255));
		break;
	case dFONT_MOUSE:
		CFont::SetColor(CRGBA(255, 255, 255, 255));
		break;
	}
	//CFont::SetColor(CRGBA(255, 255, 255, 255));
	CFont::PrintString(xstart, ystart, gUString);

	return fontGetStringSize(s);
}

Pt fontGetStringSize(const char *s)
{
	AsciiToUnicode(s, gUString);

	Pt sz = {0, 0};
	int x;
	sz.y = CFont::GetCharacterSize(gUString[0]);
	x = 0;

	wchar *ptr = gUString;
	while (wchar c = *ptr++)
	{
		if (c == '\n')
		{
			sz.y += CFont::GetCharacterSize(c);
			if (x > sz.x)
				sz.x = x;
			x = 0;
		}
		else
			x += CFont::GetCharacterSize(c);
	}
	if (x > sz.x)
		sz.x = x;
	return sz;
}

int fontGetLen(const char *s)
{
	AsciiToUnicode(s, gUString);
	return CFont::GetStringWidth(gUString);
}
