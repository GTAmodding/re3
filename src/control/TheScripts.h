#pragma once
#include "Sprite2d.h"

struct CScriptRectangle {
	char m_bIsUsed;
	char m_bIsAntialiased;
	__int16 m_wTextureId;
	CRect m_sRect;
	CRGBA m_sColor;
};

struct CTextLine {
	float m_fScaleX;
	float m_fScaleY;
	CRGBA m_sColor;
	char m_bJustify;
	char m_bCentered;
	char m_bBackground;
	char m_bBackgroundOnly;
	float m_fWrapX;
	float m_fCenterSize;
	CRGBA m_sBackgroundColor;
	char m_bTextProportional;
	char field_29;
	char m_bRightJustify;
	char field_31;
	int m_nFont;
	float field_36;
	float field_40;
	wchar m_awText[500];
};

class CTheScripts {
public:
	static char* ScriptSpace;
	static CTextLine* IntroTextLines;
	static CScriptRectangle* IntroRectangles;
	static CSprite2d* ScriptSprites;
};
