#include "common.h"
#include "main.h"
#include "Timer.h"
#include "Sprite2d.h"
#include "Text.h"
#include "Font.h"
#include "FrontEndControls.h"

#define X SCREEN_SCALE_X
#define Y(x) SCREEN_SCALE_Y(float(x)*(float(DEFAULT_SCREEN_HEIGHT)/float(SCREEN_HEIGHT_PAL)))

void
CPlaceableShText::Draw(float x, float y)
{
	if(m_text == nil)
		return;

	if(m_bRightJustify)
		CFont::SetRightJustifyOn();
	if(m_bDropShadow){
		CFont::SetDropShadowPosition(m_shadowOffset.x);
		CFont::SetDropColor(m_shadowColor);
	}
	CFont::SetColor(m_color);
	CFont::PrintString(x+m_position.x, y+m_position.y, m_text);
	if(m_bDropShadow)
		CFont::SetDropShadowPosition(0);
	if(m_bRightJustify)
		CFont::SetRightJustifyOff();
}

void
CPlaceableShText::Draw(const CRGBA &color, float x, float y)
{
	if(m_text == nil)
		return;

	if(m_bRightJustify)
		CFont::SetRightJustifyOn();
	if(m_bDropShadow){
		CFont::SetDropShadowPosition(m_shadowOffset.x);
		CFont::SetDropColor(m_shadowColor);
	}
	CFont::SetColor(color);
	CFont::PrintString(x+m_position.x, y+m_position.y, m_text);
	if(m_bDropShadow)
		CFont::SetDropShadowPosition(0);
	if(m_bRightJustify)
		CFont::SetRightJustifyOff();
}

void
CPlaceableShTextTwoLines::Draw(float x, float y)
{
	if(m_line1.m_text == nil && m_line2.m_text == nil)
		return;

	if(m_bRightJustify)
		CFont::SetRightJustifyOn();
	if(m_bDropShadow){
		CFont::SetDropShadowPosition(m_shadowOffset.x);
		CFont::SetDropColor(m_shadowColor);
	}

	if(m_line1.m_text){
		CFont::SetColor(m_line1.m_color);
		CFont::PrintString(x+m_line1.m_position.x, y+m_line1.m_position.y, m_line1.m_text);
	}
	if(m_line2.m_text){
		CFont::SetColor(m_line2.m_color);
		CFont::PrintString(x+m_line2.m_position.x, y+m_line2.m_position.y, m_line2.m_text);
	}

	if(m_bDropShadow)
		CFont::SetDropShadowPosition(0);
	if(m_bRightJustify)
		CFont::SetRightJustifyOff();
}

void
CPlaceableShTextTwoLines::Draw(const CRGBA &color, float x, float y)
{
	if(m_line1.m_text == nil && m_line2.m_text == nil)
		return;

	if(m_bRightJustify)
		CFont::SetRightJustifyOn();
	if(m_bDropShadow){
		CFont::SetDropShadowPosition(m_shadowOffset.x);
		CFont::SetDropColor(m_shadowColor);
	}

	if(m_line1.m_text){
		CFont::SetColor(color);
		CFont::PrintString(x+m_line1.m_position.x, y+m_line1.m_position.y, m_line1.m_text);
	}
	if(m_line2.m_text){
		CFont::SetColor(color);
		CFont::PrintString(x+m_line2.m_position.x, y+m_line2.m_position.y, m_line2.m_text);
	}

	if(m_bDropShadow)
		CFont::SetDropShadowPosition(0);
	if(m_bRightJustify)
		CFont::SetRightJustifyOff();
}

void
CPlaceableShOption::Draw(const CRGBA &highlightColor, float x, float y, bool bHighlight)
{
	if(bHighlight)
		CPlaceableShText::Draw(highlightColor, x, y);
	else if(m_bSelected)
		CPlaceableShText::Draw(m_selectedColor, x, y);
	else
		CPlaceableShText::Draw(x, y);
}

void
CPlaceableShOptionTwoLines::Draw(const CRGBA &highlightColor, float x, float y, bool bHighlight)
{
	if(bHighlight)
		CPlaceableShTextTwoLines::Draw(highlightColor, x, y);
	else if(m_bSelected)
		CPlaceableShTextTwoLines::Draw(m_selectedColor, x, y);
	else
		CPlaceableShTextTwoLines::Draw(x, y);
}

void
CPlaceableSprite::Draw(float x, float y)
{
	Draw(m_color, x, y);
}

void
CPlaceableSprite::Draw(const CRGBA &color, float x, float y)
{
	if(m_pSprite)
		m_pSprite->Draw(CRect(m_position.x+x, m_position.y+y,
		                      m_position.x+x + m_size.x, m_position.y+y + m_size.y),
		                color);
}

void
CPlaceableShSprite::Draw(float x, float y)
{
	if(m_bDropShadow)
		m_shadow.Draw(m_shadow.m_color, m_sprite.m_position.x+x, m_sprite.m_position.y+y);
	m_sprite.Draw(x, y);
}


/*
 * CMenuPictureAndText
 */

void
CMenuPictureAndText::SetNewOldShadowWrapX(bool bWrapX, float newWrapX, float oldWrapX)
{
	m_bWrap = bWrapX;
	m_wrapX = newWrapX;
	m_oldWrapx = oldWrapX;
}

void
CMenuPictureAndText::SetNewOldTextScale(bool bTextScale, const CVector2D &newScale, const CVector2D &oldScale)
{
	m_bSetTextScale = bTextScale;
	m_textScale = newScale;
	m_oldTextScale = oldScale;
}

void
CMenuPictureAndText::SetTextsColor(CRGBA const &color)
{
	int i;
	for(i = 0; i < m_numTexts; i++)
		m_texts[i].m_color = color;
}

void
CMenuPictureAndText::AddText(wchar *text, float positionX, float positionY, CRGBA const &color, bool bRightJustify)
{
	int i;
	if(m_numTexts >= 20)
		return;
	i = m_numTexts++;
	m_texts[i].m_text = text;
	m_texts[i].m_position.x = positionX;
	m_texts[i].m_position.y = positionY;
	m_texts[i].m_color = color;
	m_texts[i].m_bRightJustify = bRightJustify;
}

void
CMenuPictureAndText::AddPicture(CSprite2d *sprite, CSprite2d *shadow, float positionX, float positionY, float width, float height, CRGBA const &color)
{
	int i;
	if(m_numSprites >= 5)
		return;
	i = m_numSprites++;
	m_sprites[i].m_sprite.m_pSprite = sprite;
	m_sprites[i].m_shadow.m_pSprite = shadow;
	m_sprites[i].m_sprite.m_position.x = positionX;
	m_sprites[i].m_sprite.m_position.y = positionY;
	m_sprites[i].m_sprite.m_size.x = width;
	m_sprites[i].m_sprite.m_size.y = height;
	m_sprites[i].m_shadow.m_size.x = width;
	m_sprites[i].m_shadow.m_size.y = height;
	m_sprites[i].m_sprite.m_color = color;
}

void
CMenuPictureAndText::AddPicture(CSprite2d *sprite, float positionX, float positionY, float width, float height, CRGBA const &color)
{
	int i;
	if(m_numSprites >= 5)
		return;
	i = m_numSprites++;
	m_sprites[i].m_sprite.m_pSprite = sprite;
	m_sprites[i].m_shadow.m_pSprite = nil;
	m_sprites[i].m_sprite.m_position.x = positionX;
	m_sprites[i].m_sprite.m_position.y = positionY;
	m_sprites[i].m_sprite.m_size.x = width;
	m_sprites[i].m_sprite.m_size.y = height;
	m_sprites[i].m_shadow.m_size.x = width;
	m_sprites[i].m_shadow.m_size.y = height;
	m_sprites[i].m_sprite.m_color = color;
}

void
CMenuPictureAndText::Draw(CRGBA const &,CRGBA const &, float x, float y)
{
	int i;

	for(i = 0; i < m_numSprites; i++)
		m_sprites[i].Draw(m_position.x+x, m_position.y+y);

	if(m_bSetTextScale)
		CFont::SetScale(m_textScale.x, m_textScale.y);
	for(i = 0; i < m_numTexts; i++)
		if(m_bWrap)
			m_texts[i].DrawShWrap(m_position.x+x, m_position.y+y, m_wrapX, m_oldWrapx);
		else
			m_texts[i].Draw(m_position.x+x, m_position.y+y);
	if(m_bSetTextScale)
		CFont::SetScale(m_oldTextScale.x, m_oldTextScale.y);
}

void
CMenuPictureAndText::SetAlpha(uint8 alpha)
{
	int i;

	for(i = 0; i < m_numSprites; i++)
		m_sprites[i].SetAlpha(alpha);
	for(i = 0; i < m_numTexts; i++)
		m_texts[i].SetAlpha(alpha);
}

void
CMenuPictureAndText::SetShadows(bool bDropShadows, const CRGBA &shadowColor, const CVector2D &shadowOffset)
{
	int i;

	for(i = 0; i < 5; i++)
		m_sprites[i].SetShadows(bDropShadows, shadowColor, shadowOffset);
	for(i = 0; i < 20; i++)
		m_texts[i].SetShadows(bDropShadows, shadowColor, shadowOffset);
}

/*
 * CMenuMultiChoice
 */

void
CMenuMultiChoice::AddTitle(wchar *text, float positionX, float positionY, bool bRightJustify)
{
	m_title.m_text = text;
	m_title.SetPosition(positionX, positionY, bRightJustify);
}

CPlaceableShOption*
CMenuMultiChoice::AddOption(wchar *text, float positionX, float positionY, bool bSelected, bool bRightJustify)
{
	if(m_numOptions == NUM_MULTICHOICE_OPTIONS)
		return nil;
	m_options[m_numOptions].m_text = text;
	m_options[m_numOptions].SetPosition(positionX, positionY);
	m_options[m_numOptions].m_bSelected = bSelected;
	m_options[m_numOptions].m_bRightJustify = bRightJustify;
	return &m_options[m_numOptions++];
}

void
CMenuMultiChoice::SetColors(const CRGBA &title, const CRGBA &normal, const CRGBA &selected)
{
	int i;
	m_title.SetColor(title);
	for(i = 0; i < NUM_MULTICHOICE_OPTIONS; i++)
		m_options[i].SetColors(normal, selected);
}

void
CMenuMultiChoice::SetNewOldTextScale(bool bTextScale, const CVector2D &newScale, const CVector2D &oldScale, bool bTitleTextScale)
{
	m_bSetTextScale = bTextScale;
	m_textScale = newScale;
	m_oldTextScale = oldScale;
	m_bSetTitleTextScale = bTitleTextScale;
}

void
CMenuMultiChoice::Draw(CRGBA const &optionHighlight ,CRGBA const &titleHighlight, float x, float y)
{
	int i;

	if(m_bSetTextScale && m_bSetTitleTextScale)
		CFont::SetScale(m_textScale.x, m_textScale.y);
	if(m_cursor == -1)
		m_title.Draw(m_position.x+x, m_position.y+y);
	else
		m_title.Draw(titleHighlight, m_position.x+x, m_position.y+y);

	if(m_bSetTextScale)
		CFont::SetScale(m_textScale.x, m_textScale.y);

	if(m_cursor == -1)
		for(i = 0; i < m_numOptions; i++)
			m_options[i].Draw(CRGBA(0,0,0,0), m_position.x+x, m_position.y+y, false);
	else
		for(i = 0; i < m_numOptions; i++){
			if(i == m_cursor)
				m_options[i].Draw(optionHighlight, m_position.x+x, m_position.y+y);
			else
				m_options[i].Draw(CRGBA(0,0,0,0), m_position.x+x, m_position.y+y, false);
		}

	if(m_bSetTextScale){
		CFont::DrawFonts();
		CFont::SetScale(m_oldTextScale.x, m_oldTextScale.y);
	}
}

void
CMenuMultiChoice::DrawNormal(float x, float y)
{
	int i;

	if(m_bSetTextScale && m_bSetTitleTextScale)
		CFont::SetScale(m_textScale.x, m_textScale.y);
	m_title.Draw(m_position.x+x, m_position.y+y);

	if(m_bSetTextScale)
		CFont::SetScale(m_textScale.x, m_textScale.y);

	for(i = 0; i < m_numOptions; i++)
		m_options[i].Draw(CRGBA(0,0,0,0), m_position.x+x, m_position.y+y, false);

	if(m_bSetTextScale){
		CFont::DrawFonts();
		CFont::SetScale(m_oldTextScale.x, m_oldTextScale.y);
	}
}

void
CMenuMultiChoice::DrawHighlighted(CRGBA const &titleHighlight, float x, float y)
{
	int i;

	if(m_bSetTextScale && m_bSetTitleTextScale)
		CFont::SetScale(m_textScale.x, m_textScale.y);
	if(m_cursor == -1)
		m_title.Draw(m_position.x+x, m_position.y+y);
	else
		m_title.Draw(titleHighlight, m_position.x+x, m_position.y+y);

	if(m_bSetTextScale)
		CFont::SetScale(m_textScale.x, m_textScale.y);

	for(i = 0; i < m_numOptions; i++)
		m_options[i].Draw(CRGBA(0,0,0,0), m_position.x+x, m_position.y+y, false);

	if(m_bSetTextScale){
		CFont::DrawFonts();
		CFont::SetScale(m_oldTextScale.x, m_oldTextScale.y);
	}
}

void
CMenuMultiChoice::SetAlpha(uint8 alpha)
{
	int i;
	m_title.SetAlpha(alpha);
	for(i = 0; i < NUM_MULTICHOICE_OPTIONS; i++)
		m_options[i].SetAlpha(alpha);
}

void
CMenuMultiChoice::SetShadows(bool bDropShadows, CRGBA const &shadowColor, CVector2D const &shadowOffset)
{
	int i;
	m_title.SetShadows(bDropShadows, shadowColor, shadowOffset);
	for(i = 0; i < NUM_MULTICHOICE_OPTIONS; i++)
		m_options[i].SetShadows(bDropShadows, shadowColor, shadowOffset);
}


bool
CMenuMultiChoice::GoNext(void)
{
	if(m_cursor == m_numOptions-1){
		m_cursor = -1;
		return false;
	}else{
		m_cursor++;
		return true;
	}
}

bool
CMenuMultiChoice::GoPrev(void)
{
	if(m_cursor == 0){
		m_cursor = -1;
		return false;
	}else{
		m_cursor--;
		return true;
	}
}

void
CMenuMultiChoice::SelectCurrentOptionUnderCursor(void)
{
	int i;
	if(m_cursor == -1)
		return;
	for(i = 0; i < NUM_MULTICHOICE_OPTIONS; i++)
		m_options[i].m_bSelected = false;
	m_options[m_cursor].m_bSelected = true;
}

int
CMenuMultiChoice::GetMenuSelection(void)
{
	int i;
	for(i = 0; i < NUM_MULTICHOICE_OPTIONS; i++)
		if(m_options[i].m_bSelected)
			return i;
	return -1;
}

void
CMenuMultiChoice::SetMenuSelection(int selection)
{
	int i;
	for(i = 0; i < NUM_MULTICHOICE_OPTIONS; i++)
		m_options[i].m_bSelected = false;
	m_options[selection%NUM_MULTICHOICE_OPTIONS].m_bSelected = true;
}

/*
 * CMenuMultiChoiceTriggered
 */

void
CMenuMultiChoiceTriggered::Initialise(void)
{
	int i;
	for(i = 0; i < NUM_MULTICHOICE_OPTIONS; i++)
		m_triggers[i] = nil;
	m_defaultCancel = nil;
}

CPlaceableShOption*
CMenuMultiChoiceTriggered::AddOption(wchar *text, float positionX, float positionY, Trigger trigger, bool bSelected, bool bRightJustify)
{
	CPlaceableShOption *option;
	option = CMenuMultiChoice::AddOption(text, positionX, positionY, bSelected, bRightJustify);
	if(option)
		m_triggers[m_numOptions-1] = trigger;
	return option;
}

void
CMenuMultiChoiceTriggered::SelectCurrentOptionUnderCursor(void)
{
	CMenuMultiChoice::SelectCurrentOptionUnderCursor();
	if(m_cursor != -1 && m_triggers[m_cursor] != nil )
		m_triggers[m_cursor](this);
}

void
CMenuMultiChoiceTriggered::SelectDefaultCancelAction(void)
{
	if(m_defaultCancel)
		m_defaultCancel(this);
}

/*
 * CMenuMultiChoiceTriggeredAlways
 */

void
CMenuMultiChoiceTriggeredAlways::Draw(CRGBA const &optionHighlight, CRGBA const &titleHighlight, float x, float y)
{
	if(m_alwaysTrigger)
		m_alwaysTrigger(this);
	CMenuMultiChoiceTriggered::Draw(optionHighlight, titleHighlight, x, y);
}

void
CMenuMultiChoiceTriggeredAlways::DrawNormal(float x, float y)
{
	if(m_alwaysNormalTrigger)
		m_alwaysNormalTrigger(this);
	CMenuMultiChoiceTriggered::DrawNormal(x, y);
}

void
CMenuMultiChoiceTriggeredAlways::DrawHighlighted(CRGBA const &titleHighlight, float x, float y)
{
	if(m_alwaysHighlightTrigger)
		m_alwaysHighlightTrigger(this);
	CMenuMultiChoiceTriggered::DrawHighlighted(titleHighlight, x, y);
}

/*
 * CMenuMultiChoicePictured
 */

void
CMenuMultiChoicePictured::Initialise(void)
{
	int i;
	for(i = 0; i < NUM_MULTICHOICE_OPTIONS; i++)
		m_bHasSprite[i] = false;
	for(i = 0; i < NUM_MULTICHOICE_OPTIONS; i++)
		m_sprites[i].m_pSprite = nil;
}

CPlaceableShOption*
CMenuMultiChoicePictured::AddOption(CSprite2d *sprite, float positionX, float positionY, const CVector2D &size, bool bSelected)
{
	CPlaceableShOption *option;
	option = CMenuMultiChoice::AddOption(nil, 0.0f, 0.0f, bSelected, false);
	if(option){
		m_sprites[m_numOptions-1].m_pSprite = sprite;
		m_sprites[m_numOptions-1].SetPosition(positionX, positionY);
		m_sprites[m_numOptions-1].m_size = size;
		m_bHasSprite[m_numOptions-1] = true;
	}
	return option;
}

void
CMenuMultiChoicePictured::Draw(const CRGBA &optionHighlight, const CRGBA &titleHighlight, float x, float y)
{
	int i;

	// The title and all the text
	CMenuMultiChoice::Draw(optionHighlight, titleHighlight, x, y);

	CRGBA selectedColor = m_options[0].GetSelectedColor();
	CRGBA color = m_options[0].GetColor();

	// The sprites
	if(m_cursor == -1){
		for(i = 0; i < m_numOptions; i++)
			if(m_bHasSprite[i]){
				if(m_options[i].m_bSelected)
					m_sprites[i].Draw(selectedColor, m_position.x+x, m_position.y+y);
				else
					m_sprites[i].Draw(color, m_position.x+x, m_position.y+y);
			}
	}else{
		for(i = 0; i < m_numOptions; i++)
			if(i == m_cursor){
				if(m_bHasSprite[i])
				{
					uint8 color = Max(Max(optionHighlight.r, optionHighlight.g), optionHighlight.b);
					m_sprites[i].Draw(CRGBA(color, color, color, optionHighlight.a), m_position.x+x, m_position.y+y);
				}
			}else{
				if(m_bHasSprite[i]){
					if(m_options[i].m_bSelected)
						m_sprites[i].Draw(selectedColor, m_position.x+x, m_position.y+y);
					else
						m_sprites[i].Draw(color, m_position.x+x, m_position.y+y);
				}
			}
	}
}

void
CMenuMultiChoicePictured::DrawNormal(float x, float y)
{
	int i;

	// The title and all the text
	CMenuMultiChoice::DrawNormal(x, y);

	CRGBA selectedColor = m_options[0].GetSelectedColor();
	CRGBA color = m_options[0].GetColor();

	// The sprites
	for(i = 0; i < m_numOptions; i++)
		if(m_bHasSprite[i]){
			if(m_options[i].m_bSelected)
				m_sprites[i].Draw(selectedColor, m_position.x+x, m_position.y+y);
			else
				m_sprites[i].Draw(color, m_position.x+x, m_position.y+y);
		}
}

void
CMenuMultiChoicePictured::DrawHighlighted(const CRGBA &titleHighlight, float x, float y)
{
	int i;

	// The title and all the text
	CMenuMultiChoice::DrawHighlighted(titleHighlight, x, y);

	CRGBA selectedColor = m_options[0].GetSelectedColor();
	CRGBA color = m_options[0].GetColor();

	// The sprites
	for(i = 0; i < m_numOptions; i++)
		if(m_bHasSprite[i]){
			if(m_options[i].m_bSelected)
				m_sprites[i].Draw(selectedColor, m_position.x+x, m_position.y+y);
			else
				m_sprites[i].Draw(color, m_position.x+x, m_position.y+y);
		}
}

void
CMenuMultiChoicePictured::SetAlpha(uint8 alpha)
{
	int i;
	CMenuMultiChoice::SetAlpha(alpha);
	for(i = 0; i < NUM_MULTICHOICE_OPTIONS; i++)
		m_sprites[i].SetAlpha(alpha);

}


/*
 * CMenuMultiChoicePicturedTriggered
 */

void
CMenuMultiChoicePicturedTriggered::Initialise(void)
{
	int i;
	for(i = 0; i < NUM_MULTICHOICE_OPTIONS; i++)
		m_triggers[i] = nil;
	m_defaultCancel = nil;	// missing on PS2
}

CPlaceableShOption*
CMenuMultiChoicePicturedTriggered::AddOption(CSprite2d *sprite, float positionX, float positionY, const CVector2D &size, Trigger trigger, bool bSelected)
{
	CPlaceableShOption *option;
	option = CMenuMultiChoicePictured::AddOption(sprite, positionX, positionY, size, bSelected);
	if(option)
		m_triggers[m_numOptions-1] = trigger;
	return option;
}

void
CMenuMultiChoicePicturedTriggered::SelectCurrentOptionUnderCursor(void)
{
	CMenuMultiChoice::SelectCurrentOptionUnderCursor();
	if(m_cursor != -1)
		m_triggers[m_cursor](this);
}

void
CMenuMultiChoicePicturedTriggered::SelectDefaultCancelAction(void)
{
	if(m_defaultCancel)
		m_defaultCancel(this);
}

/*
 * CMenuMultiChoicePicturedTriggeredAnyMove
 */

void
CMenuMultiChoicePicturedTriggeredAnyMove::Initialise(void)
{
	int i;
	CMenuMultiChoicePicturedTriggered::Initialise();
	for(i = 0; i < NUM_MULTICHOICE_OPTIONS; i++){
		m_moveTab[i].right = -1;
		m_moveTab[i].left = -1;
		m_moveTab[i].down = -1;
		m_moveTab[i].up = -1;
	}
}

CPlaceableShOption*
CMenuMultiChoicePicturedTriggeredAnyMove::AddOption(CSprite2d *sprite, FEC_MOVETAB *moveTab, float positionX, float positionY, const CVector2D &size, Trigger trigger, bool bSelected)
{
	CPlaceableShOption *option;
	option = CMenuMultiChoicePicturedTriggered::AddOption(sprite, positionX, positionY, size, trigger, bSelected);
	if(option && moveTab)
		m_moveTab[m_numOptions-1] = *moveTab;
	return option;
}

bool
CMenuMultiChoicePicturedTriggeredAnyMove::GoDown(void)
{
	int move = m_moveTab[m_cursor].down;
	if(move == -1)
		return GoNext();
	m_cursor = move;
	return true;
}

bool
CMenuMultiChoicePicturedTriggeredAnyMove::GoUp(void)
{
	int move = m_moveTab[m_cursor].up;
	if(move == -1)
		return GoPrev();
	m_cursor = move;
	return true;
}

bool
CMenuMultiChoicePicturedTriggeredAnyMove::GoLeft(void)
{
	int move = m_moveTab[m_cursor].left;
	if(move == -1)
		return GoPrev();
	m_cursor = move;
	return true;
}

bool
CMenuMultiChoicePicturedTriggeredAnyMove::GoRight(void)
{
	int move = m_moveTab[m_cursor].right;
	if(move == -1)
		return GoNext();
	m_cursor = move;
	return true;
}


/*
 * CMenuMultiChoiceTwoLines
 */

void
CMenuMultiChoiceTwoLines::AddTitle(wchar *text, float positionX, float positionY, bool bRightJustify)
{
	m_title.m_text = text;
	m_title.SetPosition(positionX, positionY, bRightJustify);
}

CPlaceableShOptionTwoLines*
CMenuMultiChoiceTwoLines::AddOption(wchar *text, float positionX, float positionY, bool bSelected, bool bRightJustify)
{
	return AddOption(text, positionX, positionY, nil, 0.0f, 0.0f, bSelected, bRightJustify);
}

CPlaceableShOptionTwoLines*
CMenuMultiChoiceTwoLines::AddOption(wchar *text1, float positionX1, float positionY1, wchar *text2, float positionX2, float positionY2, bool bSelected, bool bRightJustify)
{
	if(m_numOptions == NUM_MULTICHOICE_OPTIONS)
		return nil;
	m_options[m_numOptions].m_line1.m_text = text1;
	m_options[m_numOptions].m_line2.m_text = text2;
	m_options[m_numOptions].m_line1.SetPosition(positionX1, positionY1);
	m_options[m_numOptions].m_line2.SetPosition(positionX2, positionY2);
	m_options[m_numOptions].m_bSelected = bSelected;
	m_options[m_numOptions].m_bRightJustify = bRightJustify;
	return &m_options[m_numOptions++];
}


void
CMenuMultiChoiceTwoLines::SetColors(const CRGBA &title, const CRGBA &normal, const CRGBA &selected)
{
	int i;
	m_title.SetColor(title);
	for(i = 0; i < NUM_MULTICHOICE_OPTIONS; i++)
		m_options[i].SetColors(normal, selected);
}

void
CMenuMultiChoiceTwoLines::SetNewOldTextScale(bool bTextScale, const CVector2D &newScale, const CVector2D &oldScale, bool bTitleTextScale)
{
	m_bSetTextScale = bTextScale;
	m_textScale = newScale;
	m_oldTextScale = oldScale;
	m_bSetTitleTextScale = bTitleTextScale;
}

void
CMenuMultiChoiceTwoLines::Draw(CRGBA const &optionHighlight ,CRGBA const &titleHighlight, float x, float y)
{
	int i;

	if(m_bSetTextScale && m_bSetTitleTextScale)
		CFont::SetScale(m_textScale.x, m_textScale.y);
	if(m_cursor == -1)
		m_title.Draw(m_position.x+x, m_position.y+y);
	else
		m_title.Draw(titleHighlight, m_position.x+x, m_position.y+y);

	if(m_bSetTextScale)
		CFont::SetScale(m_textScale.x, m_textScale.y);

	if(m_cursor == -1)
		for(i = 0; i < m_numOptions; i++)
			m_options[i].Draw(CRGBA(0,0,0,0), m_position.x+x, m_position.y+y, false);
	else
		for(i = 0; i < m_numOptions; i++){
			if(i == m_cursor)
				m_options[i].Draw(optionHighlight, m_position.x+x, m_position.y+y);
			else
				m_options[i].Draw(CRGBA(0,0,0,0), m_position.x+x, m_position.y+y, false);
		}

	if(m_bSetTextScale){
		CFont::DrawFonts();
		CFont::SetScale(m_oldTextScale.x, m_oldTextScale.y);
	}
}

void
CMenuMultiChoiceTwoLines::DrawNormal(float x, float y)
{
	int i;

	if(m_bSetTextScale && m_bSetTitleTextScale)
		CFont::SetScale(m_textScale.x, m_textScale.y);
	m_title.Draw(m_position.x+x, m_position.y+y);

	if(m_bSetTextScale)
		CFont::SetScale(m_textScale.x, m_textScale.y);

	for(i = 0; i < m_numOptions; i++)
		m_options[i].Draw(CRGBA(0,0,0,0), m_position.x+x, m_position.y+y, false);

	if(m_bSetTextScale){
		CFont::DrawFonts();
		CFont::SetScale(m_oldTextScale.x, m_oldTextScale.y);
	}
}

void
CMenuMultiChoiceTwoLines::DrawHighlighted(CRGBA const &titleHighlight, float x, float y)
{
	int i;

	if(m_bSetTextScale && m_bSetTitleTextScale)
		CFont::SetScale(m_textScale.x, m_textScale.y);
	if(m_cursor == -1)
		m_title.Draw(m_position.x+x, m_position.y+y);
	else
		m_title.Draw(titleHighlight, m_position.x+x, m_position.y+y);

	if(m_bSetTextScale)
		CFont::SetScale(m_textScale.x, m_textScale.y);

	for(i = 0; i < m_numOptions; i++)
		m_options[i].Draw(CRGBA(0,0,0,0), m_position.x+x, m_position.y+y, false);

	if(m_bSetTextScale){
		CFont::DrawFonts();
		CFont::SetScale(m_oldTextScale.x, m_oldTextScale.y);
	}
}

void
CMenuMultiChoiceTwoLines::SetAlpha(uint8 alpha)
{
	int i;
	m_title.SetAlpha(alpha);
	for(i = 0; i < NUM_MULTICHOICE_OPTIONS; i++)
		m_options[i].SetAlpha(alpha);
}

void
CMenuMultiChoiceTwoLines::SetShadows(bool bDropShadows, CRGBA const &shadowColor, CVector2D const &shadowOffset)
{
	int i;
	m_title.SetShadows(bDropShadows, shadowColor, shadowOffset);
	for(i = 0; i < NUM_MULTICHOICE_OPTIONS; i++)
		m_options[i].SetShadows(bDropShadows, shadowColor, shadowOffset);
}


bool
CMenuMultiChoiceTwoLines::GoNext(void)
{
	if(m_cursor == m_numOptions-1){
		m_cursor = -1;
		return false;
	}else{
		m_cursor++;
		return true;
	}
}

bool
CMenuMultiChoiceTwoLines::GoPrev(void)
{
	if(m_cursor == 0){
		m_cursor = -1;
		return false;
	}else{
		m_cursor--;
		return true;
	}
}

void
CMenuMultiChoiceTwoLines::SelectCurrentOptionUnderCursor(void)
{
	int i;
	if(m_cursor == -1)
		return;
	for(i = 0; i < NUM_MULTICHOICE_OPTIONS; i++)
		m_options[i].m_bSelected = false;
	m_options[m_cursor].m_bSelected = true;
}

int
CMenuMultiChoiceTwoLines::GetMenuSelection(void)
{
	int i;
	for(i = 0; i < NUM_MULTICHOICE_OPTIONS; i++)
		if(m_options[i].m_bSelected)
			return i;
	return -1;
}

void
CMenuMultiChoiceTwoLines::SetMenuSelection(int selection)
{
	int i;
	for(i = 0; i < NUM_MULTICHOICE_OPTIONS; i++)
		m_options[i].m_bSelected = false;
	m_options[selection%NUM_MULTICHOICE_OPTIONS].m_bSelected = true;
}

/*
 * CMenuMultiChoiceTwoLinesTriggered
 */

void
CMenuMultiChoiceTwoLinesTriggered::Initialise(void)
{
	int i;
	for(i = 0; i < NUM_MULTICHOICE_OPTIONS; i++)
		m_triggers[i] = nil;
	m_defaultCancel = nil;
}

CPlaceableShOptionTwoLines*
CMenuMultiChoiceTwoLinesTriggered::AddOption(wchar *text, float positionX, float positionY, Trigger trigger, bool bSelected, bool bRightJustify)
{
	CPlaceableShOptionTwoLines *option;
	option = CMenuMultiChoiceTwoLines::AddOption(text, positionX, positionY, bSelected, bRightJustify);
	if(option)
		m_triggers[m_numOptions-1] = trigger;
	return option;
}

CPlaceableShOptionTwoLines*
CMenuMultiChoiceTwoLinesTriggered::AddOption(wchar *text1, float positionX1, float positionY1, wchar *text2, float positionX2, float positionY2, Trigger trigger, bool bSelected, bool bRightJustify)
{
	CPlaceableShOptionTwoLines *option;
	option = CMenuMultiChoiceTwoLines::AddOption(text1, positionX1, positionY1, text2, positionX2, positionY2, bSelected, bRightJustify);
	if(option)
		m_triggers[m_numOptions-1] = trigger;
	return option;
}

void
CMenuMultiChoiceTwoLinesTriggered::SelectCurrentOptionUnderCursor(void)
{
	CMenuMultiChoiceTwoLines::SelectCurrentOptionUnderCursor();
	if(m_cursor != -1)
		m_triggers[m_cursor](this);
}

void
CMenuMultiChoiceTwoLinesTriggered::SelectDefaultCancelAction(void)
{
	if(m_defaultCancel)
		m_defaultCancel(this);
}


/*
 * CMenuOnOff
 */

void
CMenuOnOff::SetColors(const CRGBA &title, const CRGBA &options)
{
	m_title.SetColors(title, title);
	m_options[0].SetColor(options);
	m_options[1].SetColor(options);
}

void
CMenuOnOff::SetNewOldTextScale(bool bTextScale, const CVector2D &newScale, const CVector2D &oldScale, bool bTitleTextScale)
{
	m_bSetTextScale = bTextScale;
	m_textScale = newScale;
	m_oldTextScale = oldScale;
	m_bSetTitleTextScale = bTitleTextScale;
}

void
CMenuOnOff::SetOptionPosition(float x, float y, bool bRightJustify)
{
	m_options[0].SetPosition(x, y, bRightJustify);
	m_options[1].SetPosition(x, y, bRightJustify);
}

void
CMenuOnOff::AddTitle(wchar *text, bool bSelected, float positionX, float positionY, bool bRightJustify)
{
	m_title.m_text = text;
	m_title.m_bSelected = bSelected;
	m_title.SetPosition(positionX, positionY, bRightJustify);
}

void
CMenuOnOff::Draw(CRGBA const &optionHighlight, CRGBA const &titleHighlight, float x, float y)
{
	if(m_type == 1){
		m_options[0].m_text = TheText.Get("FEM_NO");
		m_options[1].m_text = TheText.Get("FEM_YES");
	}else if(m_type == 0){
		m_options[0].m_text = TheText.Get("FEM_OFF");
		m_options[1].m_text = TheText.Get("FEM_ON");
	}

	if(m_bSetTextScale && m_bSetTitleTextScale)
		CFont::SetScale(m_textScale.x, m_textScale.y);
	if(m_bActive)
		m_title.Draw(titleHighlight, m_position.x+x, m_position.y+y);
	else
		m_title.Draw(CRGBA(0,0,0,0), m_position.x+x, m_position.y+y, false);

	if(m_bSetTextScale)
		CFont::SetScale(m_textScale.x, m_textScale.y);

	if(m_bActive){
		if(m_title.m_bSelected)
			m_options[1].Draw(optionHighlight, m_position.x+x, m_position.y+y);
		else
			m_options[0].Draw(optionHighlight, m_position.x+x, m_position.y+y);
	}else{
		if(m_title.m_bSelected)
			m_options[1].Draw(m_position.x+x, m_position.y+y);
		else
			m_options[0].Draw(m_position.x+x, m_position.y+y);
	}

	if(m_bSetTextScale)
		CFont::SetScale(m_oldTextScale.x, m_oldTextScale.y);
}

void
CMenuOnOff::DrawNormal(float x, float y)
{
	if(m_type == 1){
		m_options[0].m_text = TheText.Get("FEM_NO");
		m_options[1].m_text = TheText.Get("FEM_YES");
	}else if(m_type == 0){
		m_options[0].m_text = TheText.Get("FEM_OFF");
		m_options[1].m_text = TheText.Get("FEM_ON");
	}

	if(m_bSetTextScale && m_bSetTitleTextScale)
		CFont::SetScale(m_textScale.x, m_textScale.y);
	m_title.Draw(CRGBA(0,0,0,0), m_position.x+x, m_position.y+y, false);

	if(m_bSetTextScale)
		CFont::SetScale(m_textScale.x, m_textScale.y);

	if(m_title.m_bSelected)
		m_options[1].Draw(m_position.x+x, m_position.y+y);
	else
		m_options[0].Draw(m_position.x+x, m_position.y+y);

	if(m_bSetTextScale)
		CFont::SetScale(m_oldTextScale.x, m_oldTextScale.y);
}

void
CMenuOnOff::DrawHighlighted(CRGBA const &titleHighlight, float x, float y)
{
	if(m_type == 1){
		m_options[0].m_text = TheText.Get("FEM_NO");
		m_options[1].m_text = TheText.Get("FEM_YES");
	}else if(m_type == 0){
		m_options[0].m_text = TheText.Get("FEM_OFF");
		m_options[1].m_text = TheText.Get("FEM_ON");
	}

	if(m_bSetTextScale && m_bSetTitleTextScale)
		CFont::SetScale(m_textScale.x, m_textScale.y);
	if(m_bActive)
		m_title.Draw(titleHighlight, m_position.x+x, m_position.y+y);
	else
		m_title.Draw(CRGBA(0,0,0,0), m_position.x+x, m_position.y+y, false);

	if(m_bSetTextScale)
		CFont::SetScale(m_textScale.x, m_textScale.y);

	if(m_title.m_bSelected)
		m_options[1].Draw(m_position.x+x, m_position.y+y);
	else
		m_options[0].Draw(m_position.x+x, m_position.y+y);

	if(m_bSetTextScale)
		CFont::SetScale(m_oldTextScale.x, m_oldTextScale.y);
}

void
CMenuOnOff::SetAlpha(uint8 alpha)
{
	m_title.SetAlpha(alpha);
	m_options[0].SetAlpha(alpha);
	m_options[1].SetAlpha(alpha);
}

void
CMenuOnOff::SetShadows(bool bDropShadows, CRGBA const &shadowColor, CVector2D const &shadowOffset)
{
	m_title.SetShadows(bDropShadows, shadowColor, shadowOffset);
	m_options[0].SetShadows(bDropShadows, shadowColor, shadowOffset);
	m_options[1].SetShadows(bDropShadows, shadowColor, shadowOffset);
}

/*
 * CMenuOnOffTriggered
 */

void
CMenuOnOffTriggered::SetOptionPosition(float x, float y, Trigger trigger, bool bRightJustify)
{
	CMenuOnOff::SetOptionPosition(x, y, bRightJustify);
	if(trigger)
		m_trigger = trigger;
}

void
CMenuOnOffTriggered::SelectCurrentOptionUnderCursor(void)
{
	CMenuOnOff::SelectCurrentOptionUnderCursor();
	if(m_trigger)
		m_trigger(this);
}



/*
 * CMenuSlider
 */

char CMenuSlider::Buf8[8];
wchar CMenuSlider::Buf16[8];

void
CMenuSlider::SetColors(const CRGBA &title, const CRGBA &percentage, const CRGBA &left, const CRGBA &right)
{
	m_title.SetColor(title);
	m_percentageText.SetColor(percentage);
	m_colors[0] = left;
	m_colors[1] = right;
}


void
CMenuSlider::AddTickBox(float positionX, float positionY, float width, float heightLeft, float heightRight)
{
	m_box.SetPosition(positionX, positionY);
	m_size[0].x = width;
	m_size[0].y = heightLeft;
	m_size[1].x = width;
	m_size[1].y = heightRight;
}

void
CMenuSlider::AddTitle(wchar *text, float positionX, float positionY)
{
	m_title.m_text = text;
	m_title.SetPosition(positionX, positionY);
}

static CRGBA SELECTED_TEXT_COLOR_0(255, 182, 48, 255);

void
CMenuSlider::Draw(const CRGBA &optionHighlight, const CRGBA &titleHighlight, float x, float y)
{
	if(m_bActive){
		CRGBA selectionCol = m_colors[0];
		if(optionHighlight.red == SELECTED_TEXT_COLOR_0.red &&
		   optionHighlight.green == SELECTED_TEXT_COLOR_0.green &&
		   optionHighlight.blue == SELECTED_TEXT_COLOR_0.blue &&
		   optionHighlight.alpha == SELECTED_TEXT_COLOR_0.alpha)
			selectionCol = m_colors[1];

		if(m_style == 1){
			// solid bar
			CRGBA shadowCol = m_box.GetShadowColor();
			float f = m_value/1000.0f;
			CVector2D boxPos = m_box.m_position + m_position + CVector2D(x,y);
			if(m_box.m_bDropShadow)
				CSprite2d::DrawRect(
					CRect(boxPos.x + X(m_box.m_shadowOffset.x),
					      boxPos.y + Y(m_box.m_shadowOffset.y),
					      boxPos.x + X(m_box.m_shadowOffset.x) + m_size[0].x,
					      boxPos.y + Y(m_box.m_shadowOffset.y) + m_size[0].y),
					shadowCol);
			CSprite2d::DrawRect(
				CRect(boxPos.x, boxPos.y,
				      boxPos.x + m_size[0].x, boxPos.y + m_size[0].y),
				m_colors[1]);
			CSprite2d::DrawRect(
				CRect(boxPos.x, boxPos.y,
				      boxPos.x + m_size[0].x*f, boxPos.y + m_size[0].y),
				selectionCol);
		}else if(m_style == 0){
			// ticks...
			CVector2D boxPos = m_box.m_position + m_position + CVector2D(x,y);
			DrawTicks(boxPos, m_size[0], m_size[1].y,
				m_value/1000.0f, m_colors[0], selectionCol, m_colors[1],
				m_box.m_bDropShadow, m_box.m_shadowOffset, m_box.GetShadowColor());
		}

		m_title.Draw(titleHighlight, m_position.x+x, m_position.y+y);

		if(m_bDrawPercentage){
			sprintf(Buf8, "%d%%", m_value/10);
			AsciiToUnicode(Buf8, Buf16);
			m_percentageText.m_text = Buf16;
			m_percentageText.Draw(optionHighlight, m_position.x+x, m_position.y+y);
		}
	}else
		CMenuSlider::DrawNormal(x, y);
}

void
CMenuSlider::DrawNormal(float x, float y)
{
	if(m_style == 1){
		// solid bar
		CRGBA shadowCol = m_box.GetShadowColor();
		float f = m_value/1000.0f;
		CVector2D boxPos = m_box.m_position + m_position + CVector2D(x,y);
		if(m_box.m_bDropShadow)
			CSprite2d::DrawRect(
				CRect(boxPos.x + X(m_box.m_shadowOffset.x),
				      boxPos.y + Y(m_box.m_shadowOffset.y),
				      boxPos.x + X(m_box.m_shadowOffset.x) + m_size[0].x,
				      boxPos.y + Y(m_box.m_shadowOffset.y) + m_size[0].y),
				shadowCol);
		CSprite2d::DrawRect(
			CRect(boxPos.x, boxPos.y,
			      boxPos.x + m_size[0].x, boxPos.y + m_size[0].y),
			m_colors[1]);
		CSprite2d::DrawRect(
			CRect(boxPos.x, boxPos.y,
			      boxPos.x + m_size[0].x*f, boxPos.y + m_size[0].y),
			m_colors[0]);
	}else if(m_style == 0){
		// ticks...
		CVector2D boxPos = m_box.m_position + m_position + CVector2D(x,y);
		DrawTicks(boxPos, m_size[0], m_size[1].y,
			m_value/1000.0f, m_colors[0], m_colors[1],
			m_box.m_bDropShadow, m_box.m_shadowOffset, m_box.GetShadowColor());
	}

	m_title.Draw(m_position.x+x, m_position.y+y);

	if(m_bDrawPercentage){
		sprintf(Buf8, "%d%%", m_value/10);
		AsciiToUnicode(Buf8, Buf16);
		m_percentageText.m_text = Buf16;
		m_percentageText.Draw(m_percentageText.GetColor(), m_position.x+x, m_position.y+y);
	}
}

void
CMenuSlider::DrawHighlighted(const CRGBA &titleHighlight, float x, float y)
{
	if(m_bActive)
		m_title.Draw(titleHighlight, m_position.x+x, m_position.y+y);
	else
		m_title.Draw(m_position.x+x, m_position.y+y);

	if(m_style == 1){
		// solid bar
		CRGBA shadowCol = m_box.GetShadowColor();
		float f = m_value/1000.0f;
		CVector2D boxPos = m_box.m_position + m_position + CVector2D(x,y);
		if(m_box.m_bDropShadow)
			CSprite2d::DrawRect(
				CRect(boxPos.x + X(m_box.m_shadowOffset.x),
				      boxPos.y + Y(m_box.m_shadowOffset.y),
				      boxPos.x + X(m_box.m_shadowOffset.x) + m_size[0].x,
				      boxPos.y + Y(m_box.m_shadowOffset.y) + m_size[0].y),
				shadowCol);
		CSprite2d::DrawRect(
			CRect(boxPos.x, boxPos.y,
			      boxPos.x + m_size[0].x, boxPos.y + m_size[0].y),
			m_colors[1]);
		CSprite2d::DrawRect(
			CRect(boxPos.x, boxPos.y,
			      boxPos.x + m_size[0].x*f, boxPos.y + m_size[0].y),
			m_colors[0]);
	}else if(m_style == 0){
		// ticks...
		CVector2D boxPos = m_box.m_position + m_position + CVector2D(x,y);
		DrawTicks(boxPos, m_size[0], m_size[1].y,
			m_value/1000.0f, m_colors[0], m_colors[1],
			m_box.m_bDropShadow, m_box.m_shadowOffset, m_box.GetShadowColor());
	}

	if(m_bDrawPercentage){
		sprintf(Buf8, "%d%%", m_value/10);
		AsciiToUnicode(Buf8, Buf16);
		m_percentageText.m_text = Buf16;
		m_percentageText.Draw(m_percentageText.GetColor(), m_position.x+x, m_position.y+y);
	}
}

void
CMenuSlider::DrawTicks(const CVector2D &position, const CVector2D &size, float heightRight, float level, const CRGBA &leftCol, const CRGBA &selCol, const CRGBA &rightCol, bool bShadow, const CVector2D &shadowOffset, const CRGBA &shadowColor)
{
	int i;
	int numTicks = size.x / X(8.0f);
	float dy = heightRight - size.y;
	float stepy = dy / numTicks;
	int left = level*numTicks;
	int drewSelection = 0;
	for(i = 0; i < numTicks; i++){
		CRect rect(position.x + X(8.0f)*i, position.y + dy - stepy*i,
		           position.x + X(8.0f)*i + X(4.0f), position.y + dy + size.y);
		if(bShadow){
			CRect shadowRect = rect;
			shadowRect.left += X(shadowOffset.x);
			shadowRect.right += X(shadowOffset.x);
			shadowRect.top += Y(shadowOffset.y);
			shadowRect.bottom += Y(shadowOffset.y);
			CSprite2d::DrawRect(shadowRect, shadowColor);
		}
		if(i < left)
			CSprite2d::DrawRect(rect, leftCol);
		else if(!drewSelection){
			CSprite2d::DrawRect(rect, selCol);
			drewSelection = 1;
		}else
			CSprite2d::DrawRect(rect, rightCol);
	}
}

void
CMenuSlider::DrawTicks(const CVector2D &position, const CVector2D &size, float heightRight, float level, const CRGBA &leftCol, const CRGBA &rightCol, bool bShadow, const CVector2D &shadowOffset, const CRGBA &shadowColor)
{
	int i;
	int numTicks = size.x / X(8.0f);
	float dy = heightRight - size.y;
	float stepy = dy / numTicks;
	int left = level*numTicks;
	for(i = 0; i < numTicks; i++){
		CRect rect(position.x + X(8.0f)*i, position.y + dy - stepy*i,
		           position.x + X(8.0f)*i + X(4.0f), position.y + dy + size.y);
		if(bShadow){
			CRect shadowRect = rect;
			shadowRect.left += X(shadowOffset.x);
			shadowRect.right += X(shadowOffset.x);
			shadowRect.top += Y(shadowOffset.y);
			shadowRect.bottom += Y(shadowOffset.y);
			CSprite2d::DrawRect(shadowRect, shadowColor);
		}
		if(i < left)
			CSprite2d::DrawRect(rect, leftCol);
		else
			CSprite2d::DrawRect(rect, rightCol);
	}
}

void
CMenuSlider::SetAlpha(uint8 alpha)
{
	m_title.SetAlpha(alpha);
	m_box.SetAlpha(alpha);
	m_someAlpha = alpha;
	m_percentageText.SetAlpha(alpha);
	m_colors[0].alpha = alpha;
	m_colors[1].alpha = alpha;
}

void
CMenuSlider::SetShadows(bool bDropShadows, const CRGBA &shadowColor, const CVector2D &shadowOffset)
{
	m_title.SetShadows(bDropShadows, shadowColor, shadowOffset);
	m_box.SetShadows(bDropShadows, shadowColor, shadowOffset);
	m_percentageText.SetShadows(bDropShadows, shadowColor, shadowOffset);
}

/*
 * CMenuSliderTriggered
 */
	
void
CMenuSliderTriggered::AddTickBox(float positionX, float positionY, float width, float heightLeft, float heightRight, Trigger trigger, Trigger alwaysTrigger)
{
	CMenuSlider::AddTickBox(positionX, positionY, width, heightLeft, heightRight);
	m_trigger = trigger;
	m_alwaysTrigger = alwaysTrigger;
}

void
CMenuSliderTriggered::Draw(const CRGBA &optionHighlight, const CRGBA &titleHighlight, float x, float y)
{
	CMenuSlider::Draw(optionHighlight, titleHighlight, x, y);
	if(m_alwaysTrigger)
		m_alwaysTrigger(this);
}

bool
CMenuSliderTriggered::GoLeft(void)
{
	CMenuSlider::GoLeft();
	if(m_trigger)
		m_trigger(this);
	return true;
}

bool
CMenuSliderTriggered::GoRight(void)
{
	CMenuSlider::GoRight();
	if(m_trigger)
		m_trigger(this);
	return true;
}

bool
CMenuSliderTriggered::GoLeftStill(void)
{
	CMenuSlider::GoLeftStill();
	if(m_trigger)
		m_trigger(this);
	return true;
}

bool
CMenuSliderTriggered::GoRightStill(void)
{
	CMenuSlider::GoRightStill();
	if(m_trigger)
		m_trigger(this);
	return true;
}

/*
 * CMenuLineLister
 */

CMenuLineLister::CMenuLineLister(void)
 : m_numLines(0), m_width(0.0f), m_height(0.0f),
   m_scrollPosition(0.0f), m_scrollSpeed(1.0f), m_lineSpacing(15.0f), field_10E8(0)
{
	int i;
	for(i = 0; i < NUM_LINELISTER_LINES_TOTAL; i++){
		m_lineAlphas[i] = 0;
		m_lineFade[i] = 0;
	}
}


void
CMenuLineLister::SetLinesColor(const CRGBA &color)
{
	int i;
	for(i = 0; i < NUM_LINELISTER_LINES_TOTAL; i++){
		m_linesLeft[i].SetColor(color);
		m_linesRight[i].SetColor(color);
	}
}

void
CMenuLineLister::ResetNumberOfTextLines(void)
{
	int i;
	m_numLines = 0;
	for(i = 0; i < NUM_LINELISTER_LINES_TOTAL; i++){
		m_lineAlphas[i] = 0;
		m_lineFade[i] = 0;
	}
	for(i = 0; i < NUM_LINELISTER_LINES_TOTAL; i++){
		// note this doesn't clear lines 0-14, probably an oversight
		GetLeftLine(i)->m_text = nil;
		GetRightLine(i)->m_text = nil;
	}
}

bool
CMenuLineLister::AddTextLine(wchar *left, wchar *right)
{
	CPlaceableShText *leftLine, *rightLine;
	if(m_numLines == NUM_LINELISTER_LINES)
		return false;
	leftLine = GetLeftLine(m_numLines);
	leftLine->m_text = left;
	leftLine->SetPosition(0.0f, m_lineSpacing*(m_numLines+15));
	rightLine = GetRightLine(m_numLines);
	rightLine->m_text = right;
	rightLine->SetPosition(leftLine->m_position.x, leftLine->m_position.y);
	m_numLines++;
	return true;
}

void
CMenuLineLister::Draw(const CRGBA &optionHighlight, const CRGBA &titleHighlight, float x, float y)
{
	int i, n;

	m_scrollPosition += m_scrollSpeed;
	n = m_numLines + 15;
	if(m_scrollSpeed > 0.0f){
		if(m_scrollPosition > n*m_lineSpacing)
			m_scrollPosition = 0.0f;
	}else{
		if(m_scrollPosition < 0.0f)
			m_scrollPosition = n*m_lineSpacing;
	}
	// this is a weird condition....
	for(i = 0;
	    m_scrollPosition < i*m_lineSpacing || m_scrollPosition >= (i+1)*m_lineSpacing;
	    i++);

	float screenPos = 0.0f;
	for(; i < n; i++){
		CVector2D linePos = m_linesLeft[i].m_position;

		if(linePos.y+m_position.y - (m_scrollPosition+m_position.y) < Y(64.0f))
			m_lineFade[i] = -4.0f*Abs(m_scrollSpeed);
		else
			m_lineFade[i] = 4.0f*Abs(m_scrollSpeed);
		int newAlpha = m_lineAlphas[i] + m_lineFade[i];
		if(newAlpha < 0) newAlpha = 0;
		if(newAlpha > 255) newAlpha = 255;
		m_lineAlphas[i] = newAlpha;

		uint8 alpha = m_linesLeft[i].m_shadowColor.alpha;

		// apply alpha
		m_linesLeft[i].SetAlpha((alpha*m_lineAlphas[i])>>8);
		m_linesRight[i].SetAlpha((alpha*m_lineAlphas[i])>>8);

		m_linesLeft[i].Draw(m_position.x+x, m_position.y+y - m_scrollPosition);
		CFont::SetRightJustifyOn();
		m_linesRight[i].Draw(m_position.x+x + m_width, m_position.y+y - m_scrollPosition);
		CFont::SetRightJustifyOff();

		// restore alpha
		m_linesLeft[i].SetAlpha(alpha);
		m_linesRight[i].SetAlpha(alpha);

		screenPos += m_lineSpacing;
		if(screenPos >= m_height)
			break;
	}

	m_scrollSpeed = 1.0f;
}

void
CMenuLineLister::SetAlpha(uint8 alpha)
{
	int i;
	for(i = 0; i < NUM_LINELISTER_LINES_TOTAL; i++){
		m_linesLeft[i].SetAlpha(alpha);
		m_linesRight[i].SetAlpha(alpha);
	}
}

void
CMenuLineLister::SetShadows(bool bDropShadows, const CRGBA &shadowColor, const CVector2D &shadowOffset)
{
	int i;
	for(i = 0; i < NUM_LINELISTER_LINES_TOTAL; i++){
		m_linesLeft[i].SetShadows(bDropShadows, shadowColor, shadowOffset);
		m_linesRight[i].SetShadows(bDropShadows, shadowColor, shadowOffset);
	}
}


/*
 * CMenuPage
 */

void
CMenuPage::Initialise(void)
{
	int i;
	m_numControls = 0;
	m_pCurrentControl = nil;
	m_cursor = 0;
	for(i = 0; i < NUM_PAGE_WIDGETS; i++)
		m_controls[i] = nil;
}

bool
CMenuPage::AddMenu(CMenuBase *widget)
{
	if(m_numControls >= NUM_PAGE_WIDGETS)
		return false;
	m_controls[m_numControls] = widget;
	if(m_numControls == 0){
		m_pCurrentControl = widget;
		m_cursor = 0;
	}
	m_numControls++;
	return true;
}

bool
CMenuPage::IsActiveMenuTwoState(void)
{
	return m_pCurrentControl && m_pCurrentControl->m_bTwoState;
}

void
CMenuPage::ActiveMenuTwoState_SelectNextPosition(void)
{
	int sel;
	if(m_pCurrentControl == nil || !m_pCurrentControl->m_bTwoState)
		return;
	m_pCurrentControl->GoFirst();
	sel = m_pCurrentControl->GetMenuSelection();
	if(sel == 1)
		m_pCurrentControl->SelectCurrentOptionUnderCursor();
	else if(sel == 0){
		if ( m_pCurrentControl )
		{
			if ( !m_pCurrentControl->GoNext() )
				m_pCurrentControl->GoFirst();
		}

		m_pCurrentControl->SelectCurrentOptionUnderCursor();
	}
}

void
CMenuPage::Draw(const CRGBA &optionHighlight, const CRGBA &titleHighlight, float x, float y)
{
	int i;
	for(i = 0; i < m_numControls; i++)
		if(m_controls[i]){
			if(i == m_cursor)
				m_controls[i]->Draw(optionHighlight, titleHighlight, x, y);
			else
				m_controls[i]->DrawNormal(x, y);
		}
}

void
CMenuPage::DrawHighlighted(const CRGBA &titleHighlight, float x, float y)
{
	int i;
	for(i = 0; i< m_numControls; i++)
		if(m_controls[i]){
			if(i == m_cursor)
				m_controls[i]->DrawHighlighted(titleHighlight, x, y);
			else
				m_controls[i]->DrawNormal(x, y);
		}
}

void
CMenuPage::DrawNormal(float x, float y)
{
	int i;
	for(i = 0; i< m_numControls; i++)
		if(m_controls[i])
			m_controls[i]->DrawNormal(x, y);
}

void
CMenuPage::ActivatePage(void)
{
	m_cursor = 0;
	if(m_numControls == 0)
		return;
	for(;;){
		m_pCurrentControl = m_controls[m_cursor];
		if(m_pCurrentControl->GoFirst())
			return;
		if(m_cursor == m_numControls-1)
			m_cursor = 0;
		else
			m_cursor++;
	}
}

void
CMenuPage::SetAlpha(uint8 alpha)
{
	int i;
	for(i = 0; i< m_numControls; i++)
		if(m_controls[i])
			m_controls[i]->SetAlpha(alpha);
}

void
CMenuPage::SetShadows(bool bDropShadows, const CRGBA &shadowColor, const CVector2D &shadowOffset)
{
	int i;
	for(i = 0; i< m_numControls; i++)
		if(m_controls[i])
			m_controls[i]->SetShadows(bDropShadows, shadowColor, shadowOffset);
}

void
CMenuPage::GoUpMenuOnPage(void)
{
	if(m_pCurrentControl == nil)
		return;
	m_pCurrentControl->DeactivateMenu();
	do{
		if(m_cursor == 0)
			m_cursor = m_numControls-1;
		else
			m_cursor--;
		m_pCurrentControl = m_controls[m_cursor];
	}while(!m_pCurrentControl->GoLast());
}

void
CMenuPage::GoDownMenuOnPage(void)
{
	if(m_pCurrentControl == nil)
		return;
	m_pCurrentControl->DeactivateMenu();
	do{
		if(m_cursor == m_numControls-1)
			m_cursor = 0;
		else
			m_cursor++;
		m_pCurrentControl = m_controls[m_cursor];
	}while(!m_pCurrentControl->GoFirst());
}

void
CMenuPage::GoLeftMenuOnPage(void)
{
	// same as up
	if(m_pCurrentControl == nil)
		return;
	m_pCurrentControl->DeactivateMenu();
	do{
		if(m_cursor == 0)
			m_cursor = m_numControls-1;
		else
			m_cursor--;
		m_pCurrentControl = m_controls[m_cursor];
	}while(!m_pCurrentControl->GoLast());
}

void
CMenuPage::GoRightMenuOnPage(void)
{
	// same as right
	if(m_pCurrentControl == nil)
		return;
	m_pCurrentControl->DeactivateMenu();
	do{
		if(m_cursor == m_numControls-1)
			m_cursor = 0;
		else
			m_cursor++;
		m_pCurrentControl = m_controls[m_cursor];
	}while(!m_pCurrentControl->GoFirst());
}

/*
 * CMenuPageAnyMove
 */

void
CMenuPageAnyMove::Initialise(void)
{
	int i;
	CMenuPage::Initialise();
	for(i = 0; i < NUM_PAGE_WIDGETS; i++){
		m_moveTab[i].left = -1;
		m_moveTab[i].right = -1;
		m_moveTab[i].up = -1;
		m_moveTab[i].down = -1;
	}
}

bool
CMenuPageAnyMove::AddMenu(CMenuBase *widget, FEC_MOVETAB *moveTab)
{
	if(AddMenu(widget)){
		m_moveTab[m_numControls-1] = *moveTab;
		return true;
	}
	return false;
}

void
CMenuPageAnyMove::GoUpMenuOnPage(void)
{
	if(m_pCurrentControl == nil)
		return;
	m_pCurrentControl->DeactivateMenu();
	int move = m_moveTab[m_cursor].up;
	if(move == -1)
		CMenuPage::GoUpMenuOnPage();
	else{	// BUG: no else in original code
		m_cursor = move;
		m_pCurrentControl = m_controls[m_cursor];
		m_pCurrentControl->GoLast();
	}
}

void
CMenuPageAnyMove::GoDownMenuOnPage(void)
{
	if(m_pCurrentControl == nil)
		return;
	m_pCurrentControl->DeactivateMenu();
	int move = m_moveTab[m_cursor].down;
	if(move == -1)
		CMenuPage::GoDownMenuOnPage();
	else{	// BUG: no else in original code
		m_cursor = move;
		m_pCurrentControl = m_controls[m_cursor];
		m_pCurrentControl->GoLast();
	}
}

void
CMenuPageAnyMove::GoLeftMenuOnPage(void)
{
	if(m_pCurrentControl == nil)
		return;
	m_pCurrentControl->DeactivateMenu();
	int move = m_moveTab[m_cursor].left;
	if(move == -1)
		CMenuPage::GoLeftMenuOnPage();
	else{	// BUG: no else in original code
		m_cursor = move;
		m_pCurrentControl = m_controls[m_cursor];
		m_pCurrentControl->GoLast();
	}
}

void
CMenuPageAnyMove::GoRightMenuOnPage(void)
{
	if(m_pCurrentControl == nil)
		return;
	m_pCurrentControl->DeactivateMenu();
	int move = m_moveTab[m_cursor].right;
	if(move == -1)
		CMenuPage::GoRightMenuOnPage();
	else{	// BUG: no else in original code
		m_cursor = move;
		m_pCurrentControl = m_controls[m_cursor];
		m_pCurrentControl->GoLast();
	}
}
