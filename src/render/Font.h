#pragma once

struct CFontDetails
{
	CRGBA color;
	float scaleX;
	float scaleY;
	float slant;
	float slantRefX;
	float slantRefY;
	bool justify;
	bool centre;
	bool rightJustify;
	bool background;
	bool backgroundOnlyText;
	bool proportional;
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
};

enum {
	ALIGN_LEFT,
	ALIGN_CENTER,
	ALIGN_RIGHT,
};

class CFont
{
	static int16 Size[3][193];
	static int16 &NewLine;
	static CSprite2d *Sprite;	//[3]
public:
	static CFontDetails& Details;

	static void Initialise(void);
	static void Shutdown(void);
	static void InitPerFrame(void);
	static void PrintChar(float x, float y, uint16 c);
	static void PrintString(float x, float y, uint16 *s);
	static int GetNumberLines(float xstart, float ystart, uint16 *s);
	static void GetTextRect(CRect *rect, float xstart, float ystart, uint16 *s);
	static void PrintString(float x, float y, uint16 *start, uint16 *end, float spwidth);
	static float GetCharacterWidth(uint16 c);
	static float GetCharacterSize(uint16 c);
	static float GetStringWidth(uint16 *s, bool spaces = false);
	static uint16 *GetNextSpace(uint16 *s);
	static uint16 *ParseToken(uint16 *s, uint16*);
	static void DrawFonts(void);
	static uint16 character_code(uint8 c);

	static CFontDetails GetDetails() { return Details; }
	static void SetScale(float x, float y) { Details.scaleX = x; Details.scaleY = y; }
	static void SetSlantRefPoint(float x, float y) { Details.slantRefX = x; Details.slantRefY = y; }
	static void SetSlant(float s) { Details.slant = s; }
	static void SetJustifyOn(void) {
		Details.justify = true;
		Details.centre = false;
		Details.rightJustify = false;
	}
	static void SetJustifyOff(void) {
		Details.justify = false;
		Details.rightJustify = false;
	}
	static void SetRightJustifyOn(void) {
		Details.rightJustify = true;
		Details.justify = false;
		Details.centre = false;
	}
	static void SetRightJustifyOff(void) {
		Details.rightJustify = false;
		Details.justify = false;
		Details.centre = false;
	}
	static void SetCentreOn(void) {
		Details.centre = true;
		Details.justify = false;
		Details.rightJustify = false;
	}
	static void SetCentreOff(void) {
		Details.centre = false;
	}
	static void SetAlignment(uint8 alignment) {
		if (alignment == ALIGN_LEFT) {
			CFont::Details.justify = true;
			CFont::Details.centre = false;
			CFont::Details.rightJustify = false;
		}
		else if (alignment == ALIGN_CENTER) {
			CFont::Details.justify = false;
			CFont::Details.centre = true;
			CFont::Details.rightJustify = false;
		}
		else if (alignment == ALIGN_RIGHT) {
			CFont::Details.justify = false;
			CFont::Details.centre = false;
			CFont::Details.rightJustify = true;
		}
	}
	static void SetWrapx(float x) { Details.wrapX = x; }
	static void SetCentreSize(float s) { Details.centreSize = s; }
	static void SetBackgroundOn(void) { Details.background = true; }
	static void SetBackgroundOff(void) { Details.background = false; }
	static void SetBackGroundOnlyTextOn(void) { Details.backgroundOnlyText = true; }
	static void SetBackGroundOnlyTextOff(void) { Details.backgroundOnlyText = false; }
	static void SetPropOn(void) { Details.proportional = true; }
	static void SetPropOff(void) { Details.proportional = false; }
	static void SetFontStyle(int16 style) { Details.style = style; }
	static void SetRightJustifyWrap(float wrap) { Details.rightJustifyWrap = wrap; }
	static void SetAlphaFade(float fade) { Details.alphaFade = fade; }
	static void SetDropShadowPosition(int16 pos) { Details.dropShadowPosition = pos; }

	// TODO: really just CRGBA but that isn't passed correctly
	static void SetBackgroundColor(const CRGBA &col) { Details.backgroundColor = col; }
	static void SetColor(const CRGBA &col) {
		Details.color = col;
		if(Details.alphaFade < 255.0f)
			Details.color.a *= Details.alphaFade/255.0f;
	}
	static void SetDropColor(const CRGBA &col) {
		Details.dropColor = col;
		if(Details.alphaFade < 255.0f)
			Details.dropColor.a *= Details.alphaFade/255.0f;
	}
};
