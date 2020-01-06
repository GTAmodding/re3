#include "common.h"
#include "patcher.h"
#include "Sprite2d.h"
#include "TxdStore.h"
#include "Font.h"

CFontDetails &CFont::Details = *(CFontDetails*)0x8F317C;
int16 &CFont::NewLine = *(int16*)0x95CC94;
CSprite2d *CFont::Sprite = (CSprite2d*)0x95CC04;

int16 CFont::Size[3][193] = {
	{
	13, 12, 31, 35, 23, 35, 31,  9, 14, 15, 25, 30, 11, 17, 13, 31,
	23, 16, 22, 21, 24, 23, 23, 20, 23, 22, 10, 35, 26, 26, 26, 26,
	30, 26, 24, 23, 24, 22, 21, 24, 26, 10, 20, 26, 22, 29, 26, 25,
	23, 25, 24, 24, 22, 25, 24, 29, 29, 23, 25, 37, 22, 37, 35, 37,
	35, 21, 22, 21, 21, 22, 13, 22, 21, 10, 16, 22, 11, 32, 21, 21,
	23, 22, 16, 20, 14, 21, 20, 30, 25, 21, 21, 33, 33, 33, 33, 35,
	27, 27, 27, 27, 32, 24, 23, 23, 23, 23, 11, 11, 11, 11, 26, 26,
	26, 26, 26, 26, 26, 25, 26, 21, 21, 21, 21, 32, 23, 22, 22, 22,
	22, 11, 11, 11, 11, 22, 22, 22, 22, 22, 22, 22, 22, 26, 21, 24,
	12, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26,
	26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 18, 26, 26,
	26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26,
	20
	},

	{
	13,  9, 21, 35, 23, 35, 35, 11, 35, 35, 25, 35, 11, 17, 13, 33,
	28, 14, 22, 21, 24, 23, 23, 21, 23, 22, 10, 35, 13, 35, 13, 33,
	 5, 25, 22, 23, 24, 21, 21, 24, 24,  9, 20, 24, 21, 27, 25, 25,
	22, 25, 23, 20, 23, 23, 23, 31, 23, 23, 23, 37, 33, 37, 35, 37,
	35, 21, 19, 19, 21, 19, 17, 21, 21,  8, 17, 18, 14, 24, 21, 21,
	20, 22, 19, 20, 20, 19, 20, 26, 21, 20, 21, 33, 33, 33, 33, 35,
	19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19,
	19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19,
	19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19,
	19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19,
	19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19,
	19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19,
	16
	},

	{
	15, 14, 16, 25, 19, 26, 22, 11, 18, 18, 27, 26, 13, 19,  9, 27,
	19, 18, 19, 19, 22, 19, 20, 18, 19, 20, 12, 32, 15, 32, 15, 35,
	15, 19, 19, 19, 19, 19, 16, 19, 20,  9, 19, 20, 14, 29, 19, 20,
	19, 19, 19, 19, 21, 19, 20, 32, 20, 19, 19, 33, 31, 39, 37, 39,
	37, 21, 21, 21, 23, 21, 19, 23, 23, 10, 19, 20, 16, 26, 23, 23,
	20, 20, 20, 22, 21, 22, 22, 26, 22, 22, 23, 35, 35, 35, 35, 37,
	19, 19, 19, 19, 29, 19, 19, 19, 19, 19,  9,  9,  9,  9, 19, 19,
	19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 30, 19, 19, 19, 19,
	19, 10, 10, 10, 10, 19, 19, 19, 19, 19, 19, 19, 19, 19, 23, 35,
	12, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19,
	19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 11, 19, 19,
	19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19,
	19
	}
};

uint16 foreign_table[128] = {
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	  0, 176,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0, 177,   0,   0,   0,   0,   0,  95,   0,   0,   0,   0, 175,
	128, 129, 130,   0, 131,   0, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141,
	  0, 173, 142, 143, 144,   0, 145,   0,   0, 146, 147, 148, 149,   0,   0, 150,
	151, 152, 153,   0, 154,   0, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164,
	  0, 174, 165, 166, 167,   0, 168,   0,   0, 169, 170, 171, 172,   0,   0,   0,
};

void
CFont::Initialise(void)
{
	int slot;

	slot = CTxdStore::AddTxdSlot("fonts");
	CTxdStore::LoadTxd(slot, "MODELS/FONTS.TXD");
	CTxdStore::AddRef(slot);
	CTxdStore::PushCurrentTxd();
	CTxdStore::SetCurrentTxd(slot);
	Sprite[0].SetTexture("font2", "font2_mask");
	Sprite[1].SetTexture("pager", "pager_mask");
	Sprite[2].SetTexture("font1", "font1_mask");
	SetScale(1.0f, 1.0f);
	SetSlantRefPoint(SCREEN_WIDTH, 0.0f);
	SetSlant(0.0);
	SetColor(CRGBA(0xFF, 0xFF, 0xFF, 0));
	SetJustifyOff();
	SetCentreOff();
	SetWrapx(640.0f);
	SetCentreSize(640.0f);
	SetBackgroundOff();
	SetBackgroundColor(CRGBA(0x80, 0x80, 0x80, 0x80));
	SetBackGroundOnlyTextOff();
	SetPropOn();
	SetFontStyle(0);
	SetRightJustifyWrap(0.0);
	SetAlphaFade(255.0f);
	SetDropShadowPosition(0);
	CTxdStore::PopCurrentTxd();
}

void
CFont::Shutdown(void)
{
	Sprite[0].Delete();
	Sprite[1].Delete();
	Sprite[2].Delete();
	CTxdStore::RemoveTxdSlot(CTxdStore::FindTxdSlot("fonts"));
}

void
CFont::InitPerFrame(void)
{
	Details.bank = CSprite2d::GetBank(30, Sprite[0].m_pTexture);
	CSprite2d::GetBank(15, Sprite[1].m_pTexture);
	CSprite2d::GetBank(15, Sprite[2].m_pTexture);
	SetDropShadowPosition(0);
	NewLine = 0;
}

void
CFont::PrintChar(float x, float y, uint16 c)
{
	if(x <= 0.0f || x > SCREEN_WIDTH ||
	   y <= 0.0f || y > SCREEN_HEIGHT)	// BUG: game uses SCREENW again
		return;

	float w = GetCharacterWidth(c) / 32.0f;
	float xoff = c & 0xF;
	float yoff = c >> 4;

	if(Details.style == FONT_BANK || Details.style == FONT_HEADING){
		if(Details.dropShadowPosition != 0){
			CSprite2d::AddSpriteToBank(Details.bank + Details.style,	// BUG: game doesn't add bank
				CRect(x + SCREEN_SCALE_X(Details.dropShadowPosition),
				      y + SCREEN_SCALE_Y(Details.dropShadowPosition),
				      x + SCREEN_SCALE_X(Details.dropShadowPosition) + 32.0f * Details.scaleX * 1.0f,
				      y + SCREEN_SCALE_Y(Details.dropShadowPosition) + 40.0f * Details.scaleY * 0.5f),
				Details.dropColor,
				xoff/16.0f,                 yoff/12.8f,
				(xoff+1.0f)/16.0f - 0.001f, yoff/12.8f,
				xoff/16.0f,                 (yoff+1.0f)/12.8f,
				(xoff+1.0f)/16.0f - 0.001f, (yoff+1.0f)/12.8f - 0.0001f);
		}
		CSprite2d::AddSpriteToBank(Details.bank + Details.style,	// BUG: game doesn't add bank
			CRect(x, y,
			      x + 32.0f * Details.scaleX * 1.0f,
			      y + 40.0f * Details.scaleY * 0.5f),
			Details.color,
			xoff/16.0f,                 yoff/12.8f,
			(xoff+1.0f)/16.0f - 0.001f, yoff/12.8f,
			xoff/16.0f,                 (yoff+1.0f)/12.8f - 0.002f,
			(xoff+1.0f)/16.0f - 0.001f, (yoff+1.0f)/12.8f - 0.002f);
	}else{
		CSprite2d::AddSpriteToBank(Details.bank + Details.style,	// BUG: game doesn't add bank
			CRect(x, y,
			      x + 32.0f * Details.scaleX * w,
			      y + 32.0f * Details.scaleY * 0.5f),
			Details.color,
			xoff/16.0f,               yoff/16.0f,
			(xoff+w)/16.0f,           yoff/16.0f,
			xoff/16.0f,               (yoff+1.0f)/16.0f,
			(xoff+w)/16.0f - 0.0001f, (yoff+1.0f)/16.0f - 0.0001f);
	}
}

void
CFont::PrintString(float xstart, float ystart, uint16 *s)
{
	CRect rect;
	int numSpaces;
	float lineLength;
	float x, y;
	bool first;
	uint16 *start, *t;

	if(*s == '*')
		return;

	if(Details.background){
		GetNumberLines(xstart, ystart, s);	// BUG: result not used
		GetTextRect(&rect, xstart, ystart, s);
		CSprite2d::DrawRect(rect, Details.backgroundColor);
	}

	lineLength = 0.0f;
	numSpaces = 0;
	first = true;
	if(Details.centre || Details.rightJustify)
		x = 0.0f;
	else
		x = xstart;
	y = ystart;
	start = s;

	// This is super ugly, I blame R*
	for(;;){
		for(;;){
			for(;;){
				if(*s == '\0')
					return;
				int xend = Details.centre ? Details.centreSize :
				           Details.rightJustify ? xstart - Details.rightJustifyWrap :
				           Details.wrapX;
				if(x + GetStringWidth(s) > xend && !first){
					// flush line
					float spaceWidth = !Details.justify || Details.centre ? 0.0f :
						(Details.wrapX - lineLength) / numSpaces;
					float xleft = Details.centre ? xstart - x/2 :
					              Details.rightJustify ? xstart - x :
					              xstart;
					PrintString(xleft, y, start, s, spaceWidth);
					// reset things
					lineLength = 0.0f;
					numSpaces = 0;
					first = true;
					if(Details.centre || Details.rightJustify)
						x = 0.0f;
					else
						x = xstart;
					y += 32.0f * CFont::Details.scaleY * 0.5f + 2.0f * CFont::Details.scaleY;
					start = s;
				}else
					break;
			}
			// advance by one word
			t = GetNextSpace(s);
			if(t[0] == '\0' ||
			   t[0] == ' ' && t[1] == '\0')
				break;
			if(!first)
				numSpaces++;
			first = false;
			x += GetStringWidth(s) + GetCharacterSize(*t - ' ');
			lineLength = x;
			s = t+1;
		}
		// print rest
		if(t[0] == ' ' && t[1] == '\0')
			t[0] = '\0';
		x += GetStringWidth(s);
		s = t;
		float xleft = Details.centre ? xstart - x/2 :
		              Details.rightJustify ? xstart - x :
		              xstart;
		PrintString(xleft, y, start, s, 0.0f);
	}
}

int
CFont::GetNumberLines(float xstart, float ystart, uint16 *s)
{
	int n;
	float x, y;
	uint16 *t;

	n = 0;
	if(Details.centre || Details.rightJustify)
		x = 0.0f;
	else
		x = xstart;
	y = ystart;

	while(*s){
		if(x + GetStringWidth(s) > (Details.centre ? Details.centreSize : Details.wrapX)){
			// reached end of line
			if(Details.centre || Details.rightJustify)
				x = 0.0f;
			else
				x = xstart;
			n++;
			// Why even?
			y += 32.0f * CFont::Details.scaleY * 0.5f + 2.0f * CFont::Details.scaleY;
		}else{
			// still space in current line
			t = GetNextSpace(s);
			if(*t == '\0'){
				// end of string
				x += GetStringWidth(s);
				n++;
				s = t;
			}else{
				x += GetStringWidth(s);
				x += GetCharacterSize(*t - ' ');
				s = t+1;
			}
		}
	}

	return n;
}

void
CFont::GetTextRect(CRect *rect, float xstart, float ystart, uint16 *s)
{
	int numLines;
	float x, y;
	int16 maxlength;
	uint16 *t;

	maxlength = 0;
	numLines = 0;
	if(Details.centre || Details.rightJustify)
		x = 0.0f;
	else
		x = xstart;
	y = ystart;

	while(*s){
		if(x + GetStringWidth(s) > (Details.centre ? Details.centreSize : Details.wrapX)){
			// reached end of line
			if(x > maxlength)
				maxlength = x;
			if(Details.centre || Details.rightJustify)
				x = 0.0f;
			else
				x = xstart;
			numLines++;
			y += 32.0f * CFont::Details.scaleY * 0.5f + 2.0f * CFont::Details.scaleY;
		}else{
			// still space in current line
			t = GetNextSpace(s);
			if(*t == '\0'){
				// end of string
				x += GetStringWidth(s);
				if(x > maxlength)
					maxlength = x;
				numLines++;
				s = t;
			}else{
				x += GetStringWidth(s);
				x += GetCharacterSize(*t - ' ');
				s = t+1;
			}
		}
	}

	if(Details.centre){
		if(Details.backgroundOnlyText){
			rect->left = xstart - maxlength/2 - 4.0f;
			rect->right = xstart + maxlength/2 + 4.0f;
			rect->bottom = (32.0f * CFont::Details.scaleY * 0.5f + 2.0f * CFont::Details.scaleY) * numLines +
				ystart + 2.0f;
			rect->top = ystart - 2.0f;
		}else{
			rect->left = xstart - Details.centreSize*0.5f - 4.0f;
			rect->right = xstart + Details.centreSize*0.5f + 4.0f;
			rect->bottom = (32.0f * CFont::Details.scaleY * 0.5f + 2.0f * CFont::Details.scaleY) * numLines +
				ystart + 2.0f;
			rect->top = ystart - 2.0f;
		}
	}else{
		rect->left = xstart - 4.0f;
		rect->right = Details.wrapX;
		// WTF?
		rect->bottom = ystart - 4.0f + 4.0f;
		rect->top = (32.0f * CFont::Details.scaleY * 0.5f + 2.0f * CFont::Details.scaleY) * numLines +
				ystart + 2.0f + 2.0f;
	}
}

void
CFont::PrintString(float x, float y, uint16 *start, uint16 *end, float spwidth)
{
	uint16 *s, c, unused;

	for(s = start; s < end; s++){
		if(*s == '~')
			s = ParseToken(s, &unused);
		c = *s - ' ';
		if(Details.slant != 0.0f)
			y = (Details.slantRefX - x)*Details.slant + Details.slantRefY;
		PrintChar(x, y, c);
		x += GetCharacterSize(c);
		if(c == 0)	// space
			x += spwidth;
	}
}

float
CFont::GetCharacterWidth(uint16 c)
{
	if(Details.proportional)
		return Size[Details.style][c];
	else
		return Size[Details.style][192];
}

float
CFont::GetCharacterSize(uint16 c)
{
	if(Details.proportional)
		return Size[Details.style][c] * Details.scaleX;
	else
		return Size[Details.style][192] * Details.scaleX;
}

float
CFont::GetStringWidth(uint16 *s, bool spaces)
{
	float w;

	w = 0.0f;
	for(; (*s != ' ' || spaces) && *s != '\0'; s++){
		if(*s == '~'){
			s++;
			while(*s != '~') s++;
			s++;
			if(*s == ' ' && !spaces)
				break;
		}
		w += GetCharacterSize(*s - ' ');
	}
	return w;
}

uint16*
CFont::GetNextSpace(uint16 *s)
{
	for(; *s != ' ' && *s != '\0'; s++)
		if(*s == '~'){
			s++;
			while(*s != '~') s++;
			s++;
			if(*s == ' ')
				break;
		}
	return s;
}

uint16*
CFont::ParseToken(uint16 *s, uint16*)
{
	s++;
	if(Details.color.r || Details.color.g || Details.color.b)
		switch(*s){
		case 'N':
		case 'n':
			NewLine = 1;
			break;
		case 'b': SetColor(CRGBA(0x80, 0xA7, 0xF3, 0xFF)); break;
		case 'g': SetColor(CRGBA(0x5F, 0xA0, 0x6A, 0xFF)); break;
		case 'h': SetColor(CRGBA(0xE1, 0xE1, 0xE1, 0xFF)); break;
		case 'l': SetColor(CRGBA(0x00, 0x00, 0x00, 0xFF)); break;
		case 'p': SetColor(CRGBA(0xA8, 0x6E, 0xFC, 0xFF)); break;
		case 'r': SetColor(CRGBA(0x71, 0x2B, 0x49, 0xFF)); break;
		case 'w': SetColor(CRGBA(0xAF, 0xAF, 0xAF, 0xFF)); break;
		case 'y': SetColor(CRGBA(0xD2, 0xC4, 0x6A, 0xFF)); break;
		}
	while(*s != '~') s++;
	return s+1;
}

void
CFont::DrawFonts(void)
{
	CSprite2d::DrawBank(Details.bank);
	CSprite2d::DrawBank(Details.bank+1);
	CSprite2d::DrawBank(Details.bank+2);
}

uint16
CFont::character_code(uint8 c)
{
	if(c < 128)
		return c;
	return foreign_table[c-128];
}

STARTPATCHES

	InjectHook(0x500A40, CFont::Initialise, PATCH_JUMP);
	InjectHook(0x500BA0, CFont::Shutdown, PATCH_JUMP);
	InjectHook(0x500BE0, CFont::InitPerFrame, PATCH_JUMP);
	InjectHook(0x500C30, CFont::PrintChar, PATCH_JUMP);
	InjectHook(0x500F50, (void (*)(float, float, uint16*))CFont::PrintString, PATCH_JUMP);
	InjectHook(0x501260, CFont::GetNumberLines, PATCH_JUMP);
	InjectHook(0x5013B0, CFont::GetTextRect, PATCH_JUMP);
	InjectHook(0x501730, (void (*)(float, float, uint16*, uint16*, float))CFont::PrintString, PATCH_JUMP);
	InjectHook(0x5017E0, CFont::GetCharacterWidth, PATCH_JUMP);
	InjectHook(0x501840, CFont::GetCharacterSize, PATCH_JUMP);
	InjectHook(0x5018A0, CFont::GetStringWidth, PATCH_JUMP);
	InjectHook(0x501960, CFont::GetNextSpace, PATCH_JUMP);
	InjectHook(0x5019A0, CFont::ParseToken, PATCH_JUMP);
	InjectHook(0x501B50, CFont::DrawFonts, PATCH_JUMP);
	InjectHook(0x501E80, CFont::character_code, PATCH_JUMP);

	InjectHook(0x501B80, CFont::SetScale, PATCH_JUMP);
	InjectHook(0x501BA0, CFont::SetSlantRefPoint, PATCH_JUMP);
	InjectHook(0x501BC0, CFont::SetSlant, PATCH_JUMP);
	InjectHook(0x501BD0, CFont::SetColor, PATCH_JUMP);
	InjectHook(0x501C60, CFont::SetJustifyOn, PATCH_JUMP);
	InjectHook(0x501C80, CFont::SetJustifyOff, PATCH_JUMP);
	InjectHook(0x501C90, CFont::SetCentreOn, PATCH_JUMP);
	InjectHook(0x501CB0, CFont::SetCentreOff, PATCH_JUMP);
	InjectHook(0x501CC0, CFont::SetWrapx, PATCH_JUMP);
	InjectHook(0x501CD0, CFont::SetCentreSize, PATCH_JUMP);
	InjectHook(0x501CE0, CFont::SetBackgroundOn, PATCH_JUMP);
	InjectHook(0x501CF0, CFont::SetBackgroundOff, PATCH_JUMP);
	InjectHook(0x501D00, CFont::SetBackgroundColor, PATCH_JUMP);
	InjectHook(0x501D30, CFont::SetBackGroundOnlyTextOn, PATCH_JUMP);
	InjectHook(0x501D40, CFont::SetBackGroundOnlyTextOff, PATCH_JUMP);
	InjectHook(0x501D50, CFont::SetRightJustifyOn, PATCH_JUMP);
	InjectHook(0x501D70, CFont::SetRightJustifyOff, PATCH_JUMP);
	InjectHook(0x501D90, CFont::SetPropOff, PATCH_JUMP);
	InjectHook(0x501DA0, CFont::SetPropOn, PATCH_JUMP);
	InjectHook(0x501DB0, CFont::SetFontStyle, PATCH_JUMP);
	InjectHook(0x501DC0, CFont::SetRightJustifyWrap, PATCH_JUMP);
	InjectHook(0x501DD0, CFont::SetAlphaFade, PATCH_JUMP);
	InjectHook(0x501DE0, CFont::SetDropColor, PATCH_JUMP);
	InjectHook(0x501E70, CFont::SetDropShadowPosition, PATCH_JUMP);

ENDPATCHES
