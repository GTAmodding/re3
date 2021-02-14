#pragma once

#include "Sprite2d.h"

void AsciiToUnicode(const char *src, wchar *dst);
void UnicodeStrcpy(wchar *dst, const wchar *src);
void UnicodeStrcat(wchar *dst, wchar *append);
int UnicodeStrlen(const wchar *str);

struct CFontDetails
{
	CRGBA color;
	float scaleX;
	float scaleY;
	float slant;
	float slantRefX;
	float slantRefY;
	bool8 justify;
	bool8 centre;
	bool8 rightJustify;
	bool8 background;
	bool8 backgroundOnlyText;
	bool8 proportional;
	float alphaFade;
	CRGBA backgroundColor;
	float wrapX;
	float centreSize;
	float rightJustifyWrap;
	int16 style;
	int32 bank;
	int16 dropShadowPosition;
	CRGBA dropColor;
};

class CSprite2d;

enum {
	FONT_BANK,
	FONT_PAGER,
	FONT_HEADING,
#ifdef MORE_LANGUAGES
	FONT_JAPANESE,
#endif
	MAX_FONTS
};

enum {
	ALIGN_LEFT,
	ALIGN_CENTER,
	ALIGN_RIGHT,
};

#ifdef MORE_LANGUAGES
enum
{
	FONT_LANGSET_EFIGS,
	FONT_LANGSET_RUSSIAN,
	FONT_LANGSET_POLISH,
	FONT_LANGSET_JAPANESE,
	LANGSET_MAX
};

#define FONT_LOCALE(style) (CFont::IsJapanese() ? FONT_JAPANESE : style)
#else
#define FONT_LOCALE(style) (style)
#endif

#ifdef BUTTON_ICONS
enum
{
	BUTTON_NONE = -1,
#if 0 // unused
	BUTTON_UP,
	BUTTON_DOWN,
	BUTTON_LEFT,
	BUTTON_RIGHT,
#endif
	BUTTON_CROSS,
	BUTTON_CIRCLE,
	BUTTON_SQUARE,
	BUTTON_TRIANGLE,
	BUTTON_L1,
	BUTTON_L2,
	BUTTON_L3,
	BUTTON_R1,
	BUTTON_R2,
	BUTTON_R3,
	MAX_BUTTON_ICONS
};
#endif // BUTTON_ICONS


class CFont
{
#ifdef MORE_LANGUAGES
	static int16 Size[LANGSET_MAX][MAX_FONTS][193];
	static uint8 LanguageSet;
	static int32 Slot;
#else
	static int16 Size[MAX_FONTS][193];
#endif
	static bool16 NewLine;
public:
	static CSprite2d Sprite[MAX_FONTS];
	static CFontDetails Details;

#ifdef BUTTON_ICONS
	static int32 ButtonsSlot;
	static CSprite2d ButtonSprite[MAX_BUTTON_ICONS];
	static int PS2Symbol;

	static void LoadButtons(const char *txdPath);
	static void DrawButton(float x, float y);
#endif // BUTTON_ICONS


	static void Initialise(void);
	static void Shutdown(void);
	static void InitPerFrame(void);
	static void PrintChar(float x, float y, wchar c);
	static void PrintString(float x, float y, wchar *s);
	static void PrintStringFromBottom(float x, float y, wchar *str);
#ifdef XBOX_SUBTITLES
	static void PrintOutlinedString(float x, float y, wchar *str, float outlineStrength, bool fromBottom, CRGBA outlineColor);
#endif
	static int GetNumberLines(float xstart, float ystart, wchar *s);
	static void GetTextRect(CRect *rect, float xstart, float ystart, wchar *s);
#ifdef MORE_LANGUAGES
	static bool PrintString(float x, float y, wchar *start, wchar* &end, float spwidth, float japX);
#else
	static void PrintString(float x, float y, wchar *start, wchar *end, float spwidth);
#endif
	static float GetCharacterWidth(wchar c);
	static float GetCharacterSize(wchar c);
	static float GetStringWidth(wchar *s, bool spaces = false);
#ifdef MORE_LANGUAGES
	static float GetStringWidth_Jap(wchar* s);
#endif
	static uint16 *GetNextSpace(wchar *s);
#ifdef MORE_LANGUAGES
	static uint16 *ParseToken(wchar *s, wchar*, bool japShit = false);
#else
	static uint16 *ParseToken(wchar *s, wchar*);
#endif
	static void DrawFonts(void);
	static uint16 character_code(uint8 c);

	static void SetScale(float x, float y);
	static void SetSlantRefPoint(float x, float y);
	static void SetSlant(float s);
	static void SetJustifyOn(void);
	static void SetJustifyOff(void);
	static void SetRightJustifyOn(void);
	static void SetRightJustifyOff(void);
	static void SetCentreOn(void);
	static void SetCentreOff(void);
	static void SetWrapx(float x);
	static void SetCentreSize(float s);
	static void SetBackgroundOn(void);
	static void SetBackgroundOff(void);
	static void SetBackGroundOnlyTextOn(void);
	static void SetBackGroundOnlyTextOff(void);
	static void SetPropOn(void);
	static void SetPropOff(void);
	static void SetFontStyle(int16 style);
	static void SetRightJustifyWrap(float wrap);
	static void SetAlphaFade(float fade);
	static void SetDropShadowPosition(int16 pos);
	static void SetBackgroundColor(CRGBA col);
	static void SetColor(CRGBA col);
	static void SetDropColor(CRGBA col);

#ifdef MORE_LANGUAGES
	static void ReloadFonts(uint8 set);

	// japanese stuff
	static bool IsAnsiCharacter(wchar* s);
	static bool IsJapanesePunctuation(wchar* str);
	static bool IsJapanese() { return LanguageSet == FONT_LANGSET_JAPANESE; }
	static bool IsJapaneseFont() { return IsJapanese() && (Details.style == FONT_JAPANESE || Details.style == FONT_PAGER);  }
#endif
};
