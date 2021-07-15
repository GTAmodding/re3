#include "common.h"

#include "Sprite2d.h"
#include "TxdStore.h"
#include "Font.h"
#ifdef BUTTON_ICONS
#include "FileMgr.h"
#endif

void
AsciiToUnicode(const char *src, wchar *dst)
{
	while((*dst++ = (unsigned char)*src++) != '\0');
}

void
UnicodeStrcat(wchar *dst, wchar *append)
{
	UnicodeStrcpy(&dst[UnicodeStrlen(dst)], append);
}

void
UnicodeStrcpy(wchar *dst, const wchar *src)
{
	while((*dst++ = *src++) != '\0');
}

int
UnicodeStrlen(const wchar *str)
{
	int len;
	for(len = 0; *str != '\0'; len++, str++);
	return len;
}

CFontDetails CFont::Details;
bool16 CFont::NewLine;
CSprite2d CFont::Sprite[MAX_FONTS];

#ifdef MORE_LANGUAGES
uint8 CFont::LanguageSet = FONT_LANGSET_EFIGS;
int32 CFont::Slot = -1;
#define JAP_TERMINATION (0x8000 | '~')

int16 CFont::Size[LANGSET_MAX][MAX_FONTS][193] = {
	{
#else
int16 CFont::Size[MAX_FONTS][193] = {
#endif

#if !defined(GTA_PS2) || defined(FIX_BUGS)
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
#else // #if defined(GTA_PS2) && !defined(FIX_BUGS)
		{
		13, 12, 31, 35, 23, 35, 31,  9, 14, 15, 25, 30, 11, 17, 13, 31,
		23, 16, 22, 21, 24, 23, 23, 20, 23, 22, 10, 35, 26, 26, 26, 26,
		30, 26, 24, 23, 24, 22, 21, 24, 26, 10, 20, 26, 22, 29, 26, 25,
		24, 25, 24, 24, 22, 25, 24, 29, 29, 23, 25, 37, 22, 37, 35, 37,
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
		19, 18, 19, 19, 21, 19, 20, 18, 19, 20, 12, 32, 15, 32, 15, 35,
		15, 19, 19, 19, 19, 19, 16, 19, 20,  9, 19, 20, 14, 29, 19, 19,
		19, 19, 19, 19, 21, 19, 20, 32, 20, 19, 19, 33, 31, 39, 37, 39,
		37, 21, 21, 21, 23, 21, 19, 23, 23, 10, 19, 20, 16, 26, 23, 23,
		20, 20, 20, 22, 21, 22, 22, 26, 22, 22, 23, 35, 35, 35, 35, 37,
		19, 19, 19, 19, 29, 19, 19, 19, 19, 19,  9,  9,  9,  9, 19, 19,
		19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 30, 19, 19, 19, 19, 19,
		10, 10, 10, 10, 19, 19, 19, 19, 19, 19, 19, 19, 19, 23, 35, 12,
		19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19,
		19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 11, 19, 19, 19,
		19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19,
		19
		}
#endif

#ifdef MORE_LANGUAGES
	},
	{
		{ 13, 12, 31, 35, 23, 35, 31, 9, 14, 15, 25, 30, 11, 17,
			13, 31, 23, 16, 22, 21, 24, 23, 23, 20, 23, 22, 10,
			35, 26, 26, 26, 26, 30, 26, 24, 23, 24, 22, 21, 24,
			26, 10, 20, 26, 22, 29, 26, 25, 23, 25, 24, 24, 22,
			25, 24, 29, 29, 23, 25, 37, 22, 37, 35, 37, 35, 21,
			22, 21, 21, 22, 13, 22, 21, 10, 16, 22, 11, 32, 21,
			21, 23, 22, 16, 20, 14, 21, 20, 30, 25, 21, 21, 13,
			33, 13, 13, 13, 24, 22, 22, 19, 26, 21, 30, 20, 23,
			23, 21, 24, 26, 23, 22, 23, 21, 22, 20, 20, 26, 25,
			24, 22, 31, 32, 23, 30, 22, 22, 32, 23, 19, 18, 18,
			15, 22, 19, 27, 19, 20, 20, 18, 22, 24, 20, 19, 19,
			20, 19, 16, 19, 28, 20, 20, 18, 26, 27, 19, 26, 18,
			19, 27, 19, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26,
			26, 26, 26, 18, 26, 26, 26, 26, 26, 26, 26, 26, 26,
			26, 26, 26, 26, 26, 26, 26, 26, 26, 20 },
		{ 13, 9, 21, 35, 23, 35, 35, 11, 35, 35, 25, 35, 11,
			17, 13, 33, 28, 14, 22, 21, 24, 23, 23, 21, 23, 22,
			10, 35, 13, 35, 13, 33, 5, 25, 22, 23, 24, 21, 21, 24,
			24, 9, 20, 24, 21, 27, 25, 25, 22, 25, 23, 20, 23, 23,
			23, 31, 23, 23, 23, 37, 33, 37, 35, 37, 35, 21, 19,
			19, 21, 19, 17, 21, 21, 8, 17, 18, 14, 24, 21, 21, 20,
			22, 19, 20, 20, 19, 20, 26, 21, 20, 21, 33, 33, 33,
			33, 35, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19,
			19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19,
			19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19,
			19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19,
			19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19,
			19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19,
			19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19,
			19, 19, 19, 19, 19, 19, 19, 16, },
		{ 15, 14, 16, 25, 19,
			26, 22, 11, 18, 18, 27, 26, 13, 19, 9, 27, 19, 18, 19,
			19, 22, 19, 20, 18, 19, 20, 12, 32, 15, 32, 15, 35,
			15, 19, 19, 19, 19, 19, 16, 19, 20, 9, 19, 20, 14, 29,
			19, 20, 19, 19, 19, 19, 21, 19, 20, 32, 20, 19, 19,
			33, 31, 39, 37, 39, 37, 21, 21, 21, 23, 21, 19, 23, 23, 10, 19, 20, 16, 26, 23,
			21, 21, 20, 20, 22, 21, 22, 22, 26, 22, 22, 23, 35,
			35, 35, 35, 37, 19, 19, 19, 19, 19, 19, 29, 19, 19,
			19, 20, 22, 31, 19, 19, 19, 19, 19, 29, 19, 29, 19,
			21, 19, 30, 31, 21, 29, 19, 19, 29, 19, 21, 23, 32,
			21, 21, 30, 31, 22, 21, 32, 33, 23, 32, 21, 21, 32,
			21, 19, 19, 30, 31, 22, 22, 21, 32, 33, 23, 32, 21,
			21, 32, 21, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19,
			19, 19, 19, 11, 19, 19, 19, 19, 19, 19, 19, 19, 19,
			19, 19, 19, 19, 19, 19, 19, 19, 19, 19 },
	},

	{
		{
			13, 12, 31, 35, 23, 35, 31, 9, 14, 15, 25, 30, 11, 17, 13, 31,
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
	}
#endif
};

#ifdef MORE_LANGUAGES
int16 Size_jp[] = {
	15, 14, 16, 20, 19, 26, 22, 11, 18, 18, 27, 26, 13, //; 0
	19, 20, 27, 19, 15, 19, 19, 21, 19, 20, 18, 19, 15, //; 13
	13, 28, 15, 32, 15, 35, 15, 19, 19, 19, 19, 17, 16, //; 26
	19, 20, 15, 19, 20, 14, 17, 19, 19, 19, 19, 19, 19, //; 39
	19, 19, 20, 25, 20, 19, 19, 33, 31, 39, 37, 39, 37, //; 52
	21, 21, 21, 19, 17, 15, 23, 21, 15, 19, 20, 16, 19, //; 65
	19, 19, 20, 20, 17, 22, 19, 22, 22, 19, 22, 22, 23, //; 78
	35, 35, 35, 35, 37, 19, 19, 19, 19, 29, 19, 19, 19, //; 91
	19, 19, 9, 9, 9, 9, 19, 19, 19, 19, 19, 19, 19, 19, //; 104
	19, 19, 19, 19, 19, 30, 19, 19, 19, 19, 19, 10, 10, //; 118
	10, 10, 19, 19, 19, 19, 19, 19, 19, 19, 19, 23, 35, //; 131
	12, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, //; 144
	19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, //; 157
	19, 19, 19, 11, 19, 19, 19, 19, 19, 19, 19, 19, 19, //; 170
	19, 19, 19, 19, 19, 19, 19, 19, 19, 21
};
#endif

wchar foreign_table[128] = {
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	  0, 176,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0, 177,   0,   0,   0,   0,   0,  95,   0,   0,   0,   0, 175,
	128, 129, 130,   0, 131,   0, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141,
	  0, 173, 142, 143, 144,   0, 145,   0,   0, 146, 147, 148, 149,   0,   0, 150,
	151, 152, 153,   0, 154,   0, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164,
	  0, 174, 165, 166, 167,   0, 168,   0,   0, 169, 170, 171, 172,   0,   0,   0,
};

#ifdef BUTTON_ICONS
CSprite2d CFont::ButtonSprite[MAX_BUTTON_ICONS];
int CFont::PS2Symbol = BUTTON_NONE;
int CFont::ButtonsSlot = -1;
#endif // BUTTON_ICONS

void
CFont::Initialise(void)
{
	int slot;

	slot = CTxdStore::AddTxdSlot("fonts");
#ifdef MORE_LANGUAGES
	Slot = slot;
	switch (LanguageSet)
	{
	case FONT_LANGSET_EFIGS:
	default:
		CTxdStore::LoadTxd(slot, "MODELS/FONTS.TXD");
		break;
	case FONT_LANGSET_POLISH:
		CTxdStore::LoadTxd(slot, "MODELS/FONTS_P.TXD");
		break;
	case FONT_LANGSET_RUSSIAN:
		CTxdStore::LoadTxd(slot, "MODELS/FONTS_R.TXD");
		break;
	case FONT_LANGSET_JAPANESE:
		CTxdStore::LoadTxd(slot, "MODELS/FONTS_J.TXD");
		break;
	}
#else
	CTxdStore::LoadTxd(slot, "MODELS/FONTS.TXD");
#endif
	CTxdStore::AddRef(slot);
	CTxdStore::PushCurrentTxd();
	CTxdStore::SetCurrentTxd(slot);
	Sprite[0].SetTexture("font2", "font2_mask");
#ifdef MORE_LANGUAGES
	if (IsJapanese()) {
		Sprite[1].SetTexture("FONTJAP", "FONTJAP_mask");
		Sprite[3].SetTexture("FONTJAP", "FONTJAP_mask");
	}
	else
#endif // MORE_LANGUAGES
		Sprite[1].SetTexture("pager", "pager_mask");
	Sprite[2].SetTexture("font1", "font1_mask");
	SetScale(1.0f, 1.0f);
	SetSlantRefPoint(SCREEN_WIDTH, 0.0f);
	SetSlant(0.0f);
	SetColor(CRGBA(255, 255, 255, 0));
	SetJustifyOff();
	SetCentreOff();
#ifdef FIX_BUGS
	SetWrapx(SCREEN_STRETCH_X(DEFAULT_SCREEN_WIDTH));
	SetCentreSize(SCREEN_STRETCH_X(DEFAULT_SCREEN_WIDTH));
#else
	SetWrapx(DEFAULT_SCREEN_WIDTH);
	SetCentreSize(DEFAULT_SCREEN_WIDTH);
#endif
	SetBackgroundOff();
	SetBackgroundColor(CRGBA(128, 128, 128, 128));
	SetBackGroundOnlyTextOff();
	SetPropOn();
	SetFontStyle(FONT_BANK);
	SetRightJustifyWrap(0.0f);
	SetAlphaFade(255.0f);
	SetDropShadowPosition(0);
	CTxdStore::PopCurrentTxd();

#if !defined(GAMEPAD_MENU) && defined(BUTTON_ICONS)
	// loaded in CMenuManager with GAMEPAD_MENU defined
	LoadButtons("MODELS/X360BTNS.TXD");
#endif
}

#ifdef BUTTON_ICONS
void
CFont::LoadButtons(const char* txdPath)
{
	if (int file = CFileMgr::OpenFile(txdPath)) {
		CFileMgr::CloseFile(file);
		if (ButtonsSlot == -1)
			ButtonsSlot = CTxdStore::AddTxdSlot("buttons");
		else {
			for (int i = 0; i < MAX_BUTTON_ICONS; i++)
				ButtonSprite[i].Delete();
			CTxdStore::RemoveTxd(ButtonsSlot);
		}
		CTxdStore::LoadTxd(ButtonsSlot, txdPath);
		CTxdStore::AddRef(ButtonsSlot);
		CTxdStore::PushCurrentTxd();
		CTxdStore::SetCurrentTxd(ButtonsSlot);
#if 0  // unused
		ButtonSprite[BUTTON_UP].SetTexture("up");
		ButtonSprite[BUTTON_DOWN].SetTexture("down");
		ButtonSprite[BUTTON_LEFT].SetTexture("left");
		ButtonSprite[BUTTON_RIGHT].SetTexture("right");
#endif
		ButtonSprite[BUTTON_CROSS].SetTexture("cross");
		ButtonSprite[BUTTON_CIRCLE].SetTexture("circle");
		ButtonSprite[BUTTON_SQUARE].SetTexture("square");
		ButtonSprite[BUTTON_TRIANGLE].SetTexture("triangle");
		ButtonSprite[BUTTON_L1].SetTexture("l1");
		ButtonSprite[BUTTON_L2].SetTexture("l2");
		ButtonSprite[BUTTON_L3].SetTexture("l3");
		ButtonSprite[BUTTON_R1].SetTexture("r1");
		ButtonSprite[BUTTON_R2].SetTexture("r2");
		ButtonSprite[BUTTON_R3].SetTexture("r3");
		CTxdStore::PopCurrentTxd();
	}
	else {
		if (ButtonsSlot != -1) {
			for (int i = 0; i < MAX_BUTTON_ICONS; i++)
				ButtonSprite[i].Delete();
			CTxdStore::RemoveTxdSlot(ButtonsSlot);
			ButtonsSlot = -1;
		}
	}
}
#endif // BUTTON_ICONS

#ifdef MORE_LANGUAGES
void
CFont::ReloadFonts(uint8 set)
{
	if (Slot != -1 && LanguageSet != set) {
		Sprite[0].Delete();
		Sprite[1].Delete();
		Sprite[2].Delete();
		if (IsJapanese())
			Sprite[3].Delete();
		CTxdStore::PushCurrentTxd();
		CTxdStore::RemoveTxd(Slot);
		switch (set)
		{
		case FONT_LANGSET_EFIGS:
		default:
			CTxdStore::LoadTxd(Slot, "MODELS/FONTS.TXD");
			break;
		case FONT_LANGSET_POLISH:
			CTxdStore::LoadTxd(Slot, "MODELS/FONTS_P.TXD");
			break;
		case FONT_LANGSET_RUSSIAN:
			CTxdStore::LoadTxd(Slot, "MODELS/FONTS_R.TXD");
			break;
		case FONT_LANGSET_JAPANESE:
			CTxdStore::LoadTxd(Slot, "MODELS/FONTS_J.TXD");
			break;
		}
		CTxdStore::SetCurrentTxd(Slot);
		Sprite[0].SetTexture("font2", "font2_mask");
		if (set == FONT_LANGSET_JAPANESE) {
			Sprite[1].SetTexture("FONTJAP", "FONTJAP_mask");
			Sprite[3].SetTexture("FONTJAP", "FONTJAP_mask");
		}
		else
			Sprite[1].SetTexture("pager", "pager_mask");
		Sprite[2].SetTexture("font1", "font1_mask");
		CTxdStore::PopCurrentTxd();
	}
	LanguageSet = set;
}
#endif

void
CFont::Shutdown(void)
{
#ifdef BUTTON_ICONS
	if (ButtonsSlot != -1) {
		for (int i = 0; i < MAX_BUTTON_ICONS; i++)
			ButtonSprite[i].Delete();
		CTxdStore::RemoveTxdSlot(ButtonsSlot);
		ButtonsSlot = -1;
	}
#endif
	Sprite[0].Delete();
	Sprite[1].Delete();
	Sprite[2].Delete();
#ifdef MORE_LANGUAGES
	if (IsJapanese())
		Sprite[3].Delete();
	CTxdStore::RemoveTxdSlot(Slot);
	Slot = -1;
#else
	CTxdStore::RemoveTxdSlot(CTxdStore::FindTxdSlot("fonts"));
#endif
}

void
CFont::InitPerFrame(void)
{
	Details.bank = CSprite2d::GetBank(30, Sprite[0].m_pTexture);
	CSprite2d::GetBank(15, Sprite[1].m_pTexture);
	CSprite2d::GetBank(15, Sprite[2].m_pTexture);
#ifdef MORE_LANGUAGES
	if (IsJapanese())
		CSprite2d::GetBank(15, Sprite[3].m_pTexture);
#endif
	SetDropShadowPosition(0);
	NewLine = false;
#ifdef BUTTON_ICONS
	PS2Symbol = BUTTON_NONE;
#endif
}

#ifdef BUTTON_ICONS
void
CFont::DrawButton(float x, float y)
{
	if (x <= 0.0f || x > SCREEN_WIDTH || y <= 0.0f || y > SCREEN_HEIGHT)
		return;

	if (PS2Symbol != BUTTON_NONE) {
		CRect rect;
		rect.left = x;
		rect.top = Details.scaleY + Details.scaleY + y;
		rect.right = Details.scaleY * 17.0f + x;
		rect.bottom = Details.scaleY * 19.0f + y;

		int vertexAlphaState;
		RwRenderStateGet(rwRENDERSTATEVERTEXALPHAENABLE, &vertexAlphaState);
		RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void *)TRUE);
		ButtonSprite[PS2Symbol].Draw(rect, CRGBA(255, 255, 255, Details.color.a));
		RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void *)vertexAlphaState);
	}
}
#endif

void
CFont::PrintChar(float x, float y, wchar c)
{
	if(x <= 0.0f || x > SCREEN_WIDTH ||
#ifdef FIX_BUGS
	   y <= 0.0f || y > SCREEN_HEIGHT)
#else
	   y <= 0.0f || y > SCREEN_WIDTH)
#endif
		return;

	float w = GetCharacterWidth(c) / 32.0f;
	float xoff = c % 16;
	float yoff = c / 16;
#ifdef MORE_LANGUAGES
	if (IsJapaneseFont()) {
		w = 21.0f;
		xoff = (float)(c % 48);
		yoff = c / 48;
	}
#endif

	if(Details.style == FONT_BANK || Details.style == FONT_HEADING){
		if(Details.dropShadowPosition != 0){
			CSprite2d::AddSpriteToBank(
#ifdef FIX_BUGS
				Details.bank + Details.style,
#else
				Details.style, // BUG: game doesn't add bank
#endif
#ifdef FIX_BUGS
				CRect(x + SCREEN_SCALE_X(Details.dropShadowPosition),
				      y + SCREEN_SCALE_Y(Details.dropShadowPosition),
				      x + SCREEN_SCALE_X(Details.dropShadowPosition) + 32.0f * Details.scaleX * 1.0f,
				      y + SCREEN_SCALE_Y(Details.dropShadowPosition) + 40.0f * Details.scaleY * 0.5f),
#else
				CRect(x + Details.dropShadowPosition,
				      y + Details.dropShadowPosition,
				      x + Details.dropShadowPosition + 32.0f * Details.scaleX * 1.0f,
				      y + Details.dropShadowPosition + 40.0f * Details.scaleY * 0.5f),
#endif
				Details.dropColor,
				xoff/16.0f,                 yoff/12.8f,
				(xoff+1.0f)/16.0f - 0.001f, yoff/12.8f,
				xoff/16.0f,                 (yoff+1.0f)/12.8f,
				(xoff+1.0f)/16.0f - 0.001f, (yoff+1.0f)/12.8f - 0.0001f);
		}
		CSprite2d::AddSpriteToBank(
#ifdef FIX_BUGS
			Details.bank + Details.style,
#else
			Details.style, // BUG: game doesn't add bank
#endif
			CRect(x, y,
			      x + 32.0f * Details.scaleX * 1.0f,
			      y + 40.0f * Details.scaleY * 0.5f),
			Details.color,
			xoff/16.0f,                 yoff/12.8f,
			(xoff+1.0f)/16.0f - 0.001f, yoff/12.8f,
			xoff/16.0f,                 (yoff+1.0f)/12.8f - 0.002f,
			(xoff+1.0f)/16.0f - 0.001f, (yoff+1.0f)/12.8f - 0.002f);
#ifdef MORE_LANGUAGES
	}else if (IsJapaneseFont()) {
		if (Details.dropShadowPosition != 0) {
			CSprite2d::AddSpriteToBank(
#ifdef FIX_BUGS
				Details.bank + Details.style,
#else
				Details.style, // BUG: game doesn't add bank
#endif
#ifdef FIX_BUGS
				CRect(x + SCREEN_SCALE_X(Details.dropShadowPosition),
					y + SCREEN_SCALE_Y(Details.dropShadowPosition),
					x + SCREEN_SCALE_X(Details.dropShadowPosition) + 32.0f * Details.scaleX * 1.0f,
					y + SCREEN_SCALE_Y(Details.dropShadowPosition) + 40.0f * Details.scaleY / 2.75f),
#else
				CRect(x + Details.dropShadowPosition,
					y + Details.dropShadowPosition,
					x + Details.dropShadowPosition + 32.0f * Details.scaleX * 1.0f,
					y + Details.dropShadowPosition + 40.0f * Details.scaleY / 2.75f),
#endif
				Details.dropColor,
				xoff * w / 1024.0f, yoff / 25.6f,
				xoff * w / 1024.0f + (1.0f / 48.0f) - 0.001f, yoff / 25.6f,
				xoff * w / 1024.0f, (yoff + 1.0f) / 25.6f,
				xoff * w / 1024.0f + (1.0f / 48.0f) - 0.001f, (yoff + 1.0f) / 25.6f - 0.0001f);
		}
		CSprite2d::AddSpriteToBank(Details.bank + Details.style,	// BUG: game doesn't add bank
			CRect(x, y,
				x + 32.0f * Details.scaleX * 1.0f,
				y + 40.0f * Details.scaleY / 2.75f),
			Details.color,
			xoff * w / 1024.0f, yoff / 25.6f,
			xoff * w / 1024.0f + (1.0f / 48.0f) - 0.001f, yoff / 25.6f,
			xoff * w / 1024.0f, (yoff + 1.0f) / 25.6f - 0.002f,
			xoff * w / 1024.0f + (1.0f / 48.0f) - 0.001f, (yoff + 1.0f) / 25.6f - 0.0001f);
#endif
	}else
	{
		CSprite2d::AddSpriteToBank(
#ifdef FIX_BUGS
			Details.bank + Details.style,
#else
			Details.style, // BUG: game doesn't add bank
#endif
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

#ifdef MORE_LANGUAGES
bool CFont::IsJapanesePunctuation(wchar *str)
{
	return (*str == 0xE7 || *str == 0x124 || *str == 0x126 || *str == 0x128 || *str == 0x104 || *str == ',' || *str == '>' || *str == '!' || *str == 0x99 || *str == '?' || *str == ':');
}

bool CFont::IsAnsiCharacter(wchar *s)
{
	if (*s >= 'A' && *s <= 'Z')
		return true;
	if (*s >= 'a' && *s <= 'z')
		return true;
	if (*s >= '0' && *s <= ':')
		return true;
	if (*s == '(' || *s == ')')
		return true;
	if (*s == 'D' || *s == '$')
		return true;
	return false;
}
#endif

void
CFont::PrintString(float xstart, float ystart, wchar *s)
{
	CRect rect;
	int numSpaces;
	float lineLength;
	float x, y;
	bool first;
	wchar *start, *t;

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
				float xend = Details.centre ? Details.centreSize :
				           Details.rightJustify ? xstart - Details.rightJustifyWrap :
				           Details.wrapX;
#ifdef MORE_LANGUAGES
				if (IsJapaneseFont())
					xend -= SCREEN_SCALE_X(21.0f * 2.0f);
#endif
				if(x + GetStringWidth(s) > xend && !first){
#ifdef MORE_LANGUAGES
					if (IsJapanese() && IsJapanesePunctuation(s))
						s--;
#endif
					// flush line
					float spaceWidth = !Details.justify || Details.centre ? 0.0f :
						(Details.wrapX - lineLength) / numSpaces;
					float xleft = Details.centre ? xstart - x/2 :
					              Details.rightJustify ? xstart - x :
					              xstart;
#ifdef MORE_LANGUAGES
					PrintString(xleft, y, start, s, spaceWidth, xstart);
#else
					PrintString(xleft, y, start, s, spaceWidth);
#endif
					// reset things
					lineLength = 0.0f;
					numSpaces = 0;
					first = true;
					if(Details.centre || Details.rightJustify)
						x = 0.0f;
					else
						x = xstart;
#ifdef MORE_LANGUAGES
					if (IsJapaneseFont())
						y += 32.0f * CFont::Details.scaleY / 2.75f + 2.0f * CFont::Details.scaleY;
					else
#endif
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
#ifdef MORE_LANGUAGES
			if (IsJapaneseFont() && IsAnsiCharacter(s))
				x += 21.0f;
#endif
			lineLength = x;
			s = t+1;
#ifdef MORE_LANGUAGES
			if (IsJapaneseFont() && !*s) {
				x += GetStringWidth(s);
				if (IsAnsiCharacter(s))
					x += 21.0f;
				float xleft = Details.centre ? xstart - x / 2 :
					Details.rightJustify ? xstart - x :
					xstart;
				if (PrintString(xleft, y, start, s, 0.0f, xstart))
				{
					start = s;
					if (!Details.centre && !Details.rightJustify)
						x = xstart;
					else
						x = 0.0f;

					y += 32.0f * CFont::Details.scaleY / 2.75f + 2.0f * CFont::Details.scaleY;
					numSpaces = 0;
					first = true;
					lineLength = 0.0f;
				}
			}
#endif
		}
		// print rest
		if(t[0] == ' ' && t[1] == '\0')
			t[0] = '\0';
		x += GetStringWidth(s);
		s = t;
		float xleft = Details.centre ? xstart - x/2 :
		              Details.rightJustify ? xstart - x :
		              xstart;
#ifdef MORE_LANGUAGES
		if (PrintString(xleft, y, start, s, 0.0f, xstart) && IsJapaneseFont()) {
			start = s;
			if (!Details.centre && !Details.rightJustify)
				x = xstart;
			else
				x = 0.0f;
			y += 32.0f * CFont::Details.scaleY / 2.75f + 2.0f * CFont::Details.scaleY;
			numSpaces = 0;
			first = true;
			lineLength = 0.0f;
		}
#else
		PrintString(xleft, y, start, s, 0.0f);
#endif
	}
}

int
CFont::GetNumberLines(float xstart, float ystart, wchar *s)
{
	int n;
	float x, y;
	wchar *t;
	n = 0;

#ifdef MORE_LANGUAGES
	bool bSomeJapBool = false;

	if (IsJapanese()) {
		t = s;
		wchar unused;
		while (*t) {
			if (*t == JAP_TERMINATION || *t == '~')
				t = ParseToken(t, &unused, true);
			if (NewLine) {
				n++;
				NewLine = false;
				bSomeJapBool = true;
			}
			t++;
		}
	}

	if (bSomeJapBool) n--;
#endif

	if(Details.centre || Details.rightJustify)
		x = 0.0f;
	else
		x = xstart;
	y = ystart;

	while(*s){
#ifdef FIX_BUGS
		float f = Details.centre ? Details.centreSize :
		           Details.rightJustify ? xstart - Details.rightJustifyWrap :
		           Details.wrapX;
#else
		float f = (Details.centre ? Details.centreSize : Details.wrapX);
#endif

#ifdef MORE_LANGUAGES
		if (IsJapaneseFont())
			f -= SCREEN_SCALE_X(21.0f * 2.0f);
#endif

		if(x + GetStringWidth(s) > f){
#ifdef MORE_LANGUAGES
			if (IsJapanese())
			{
				if (IsJapanesePunctuation(s))
					s--;
			}
#endif
			// reached end of line
			if(Details.centre || Details.rightJustify)
				x = 0.0f;
			else
				x = xstart;
			n++;
			// Why even?
#ifdef MORE_LANGUAGES
			if (IsJapanese())
				y += 32.0f * CFont::Details.scaleY / 2.75f + 2.0f * CFont::Details.scaleY;
			else
#endif
				y += 32.0f * CFont::Details.scaleY * 0.5f + 2.0f * CFont::Details.scaleY;
		}else{
			// still space in current line
			t = GetNextSpace(s);
			if(*t == '\0'){
				// end of string
				x += GetStringWidth(s);
#ifdef MORE_LANGUAGES
				if (IsJapanese() && IsAnsiCharacter(s))
					x += 21.0f;
#endif
				n++;
				s = t;
			}else{
				x += GetStringWidth(s);
#ifdef MORE_LANGUAGES
				if (IsJapanese() && IsAnsiCharacter(s))
					x += 21.0f;
#endif
				s = t+1;
				x += GetCharacterSize(*t - ' ');
#ifdef MORE_LANGUAGES
				if (IsJapanese() && !*s)
					n++;
#endif
			}
		}
	}

	return n;
}

void
CFont::GetTextRect(CRect *rect, float xstart, float ystart, wchar *s)
{
	int numLines;
	float x, y;
	int16 maxlength;
	wchar *t;

	maxlength = 0;
	numLines = 0;

#ifdef MORE_LANGUAGES
	if (IsJapanese()) {
		numLines = GetNumberLines(xstart, ystart, s);
	}else{
#endif

#ifdef FIX_BUGS
		if(Details.centre || Details.rightJustify)
#else
		if(Details.centre)
#endif
			x = 0.0f;
		else
			x = xstart;
		y = ystart;

#ifdef FIX_BUGS
		float xEnd = Details.centre ? Details.centreSize :
		           Details.rightJustify ? xstart - Details.rightJustifyWrap :
		           Details.wrapX;
#else
		float xEnd = (Details.centre ? Details.centreSize : Details.wrapX);
#endif
		while(*s){
			if(x + GetStringWidth(s) > xEnd){
				// reached end of line
				if(x > maxlength)
					maxlength = x;
#ifdef FIX_BUGS
				if(Details.centre || Details.rightJustify)
#else
				if(Details.centre)
#endif
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
#ifdef MORE_LANGUAGES
	}
#endif

	if(Details.centre){
		if(Details.backgroundOnlyText){
			rect->left = xstart - maxlength/2 - 4.0f;
			rect->right = xstart + maxlength/2 + 4.0f;
#ifdef MORE_LANGUAGES
			if (IsJapaneseFont()) {
				rect->bottom = (32.0f * CFont::Details.scaleY / 2.75f + 2.0f * CFont::Details.scaleY) * numLines + ystart + (4.0f / 2.75f);
				rect->top = ystart - (4.0f / 2.75f);
			} else {
#endif
				rect->bottom = (32.0f * CFont::Details.scaleY * 0.5f + 2.0f * CFont::Details.scaleY) * numLines + ystart + 2.0f;
				rect->top = ystart - 2.0f;
#ifdef MORE_LANGUAGES
			}
#endif
		}else{
			rect->left = xstart - Details.centreSize*0.5f - 4.0f;
			rect->right = xstart + Details.centreSize*0.5f + 4.0f;
#ifdef MORE_LANGUAGES
			if (IsJapaneseFont()) {
				rect->bottom = (32.0f * CFont::Details.scaleY / 2.75f + 2.0f * CFont::Details.scaleY) * numLines + ystart + (4.0f / 2.75f);
				rect->top = ystart - (4.0f / 2.75f);
			} else {
#endif
				rect->bottom = (32.0f * CFont::Details.scaleY * 0.5f + 2.0f * CFont::Details.scaleY) * numLines + ystart + 2.0f;
				rect->top = ystart - 2.0f;
#ifdef MORE_LANGUAGES
			}
#endif
		}
	}else{
		rect->left = xstart - 4.0f;
		rect->right = Details.wrapX;
		// WTF?
		rect->bottom = ystart - 4.0f + 4.0f;
#ifdef MORE_LANGUAGES
		if (IsJapaneseFont())
			rect->top = (32.0f * CFont::Details.scaleY / 2.75f + 2.0f * CFont::Details.scaleY) * numLines + ystart + 2.0f + (4.0f / 2.75f);
		else
#endif
			rect->top = (32.0f * CFont::Details.scaleY * 0.5f + 2.0f * CFont::Details.scaleY) * numLines + ystart + 2.0f + 2.0f;
	}
}

#ifdef MORE_LANGUAGES
bool
CFont::PrintString(float x, float y, wchar *start, wchar *&end, float spwidth, float japX)
{
	wchar *s, c, unused;

	if (IsJapanese()) {
		float jx = 0.0f;
		for (s = start; s < end; s++) {
			if (*s == JAP_TERMINATION || *s == '~')
				s = ParseToken(s, &unused, true);
			if (NewLine) {
				NewLine = false;
				break;
			}
			jx += GetCharacterSize(*s - ' ');
		}
		s = start;
		if (Details.centre)
			x = japX - jx / 2.0f;
		else if (Details.rightJustify)
			x = japX - jx;
	}

	for (s = start; s < end; s++) {
		if (*s == '~' || (IsJapanese() && *s == JAP_TERMINATION))
			s = ParseToken(s, &unused);
		if (NewLine && IsJapanese()) {
			NewLine = false;
			end = s;
			return true;
		}
		c = *s - ' ';
		if (Details.slant != 0.0f && !IsJapanese())
			y = (Details.slantRefX - x) * Details.slant + Details.slantRefY;

#ifdef BUTTON_ICONS
		if (PS2Symbol != BUTTON_NONE) {
			DrawButton(x, y);
			x += Details.scaleY * 17.0f;
			PS2Symbol = BUTTON_NONE;
		}
#endif

		PrintChar(x, y, c);
		x += GetCharacterSize(c);
		if (c == 0 && (!NewLine || !IsJapanese()))	// space
			x += spwidth;
	}
	return false;
}
#else
void
CFont::PrintString(float x, float y, wchar *start, wchar *end, float spwidth)
{
	wchar *s, c, unused;

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
#endif

void
CFont::PrintStringFromBottom(float x, float y, wchar *str)
{
#ifdef MORE_LANGUAGES
	if (IsJapaneseFont())
		y -= (32.0f * CFont::Details.scaleY / 2.75f + 2.0f * CFont::Details.scaleY) * GetNumberLines(x, y, str);
	else
#endif
		y -= (32.0f * CFont::Details.scaleY * 0.5f + 2.0f * CFont::Details.scaleY) * GetNumberLines(x, y, str);
	PrintString(x, y, str);
}

#ifdef XBOX_SUBTITLES
void
CFont::PrintOutlinedString(float x, float y, wchar *str, float outlineStrength, bool fromBottom, CRGBA outlineColor)
{
	CRGBA textColor = Details.color;
	SetColor(outlineColor);
	CVector2D offsets[] = { {1.f, 1.f}, {1.f, -1.f}, {-1.f, 1.f}, {-1.f, -1.f} };
	for(int i = 0; i < ARRAY_SIZE(offsets); i++){
		if (fromBottom)
			PrintStringFromBottom(x + SCREEN_SCALE_X(offsets[i].x * outlineStrength), y + SCREEN_SCALE_Y(offsets[i].y * outlineStrength), str);
		else
			PrintString(x + SCREEN_SCALE_X(offsets[i].x * outlineStrength), y + SCREEN_SCALE_Y(offsets[i].y * outlineStrength), str);
	}
	SetColor(textColor);
	
	if (fromBottom)
		PrintStringFromBottom(x, y, str);
	else
		PrintString(x, y, str);
}
#endif

float
CFont::GetCharacterWidth(wchar c)
{
#ifdef MORE_LANGUAGES
	if (IsJapanese()) {
		if (!Details.proportional)
			return Size[0][Details.style][192];
		if (c <= 94 || Details.style == FONT_HEADING || Details.style == FONT_BANK) {
			switch (Details.style)
			{
			case FONT_JAPANESE:
				return Size_jp[c];
			default:
				return Size[0][Details.style][c];
			}
		}
		if (c < 254 && Details.style == FONT_PAGER)
			return 29.4f;

		switch (Details.style)
		{
		case FONT_JAPANESE:
			return 29.4f;
		case FONT_BANK:
			return 10.0f;
		case FONT_PAGER:
			return 31.5f;
		default:
			return Size[0][Details.style][c];
		}
	}

	else if (Details.proportional)
		return Size[LanguageSet][Details.style][c];
	else
		return Size[LanguageSet][Details.style][192];
#else
	if (Details.proportional)
		return Size[Details.style][c];
	else
		return Size[Details.style][192];
#endif // MORE_LANGUAGES
}

float
CFont::GetCharacterSize(wchar c)
{
#ifdef MORE_LANGUAGES

	if (IsJapanese())
	{
		if (!Details.proportional)
			return Size[0][Details.style][192] * Details.scaleX;
		if (c <= 94 || Details.style == FONT_HEADING || Details.style == FONT_BANK) {
			switch (Details.style)
			{
			case FONT_JAPANESE:
				return Size_jp[c] * Details.scaleX;
			default:
				return Size[0][Details.style][c] * Details.scaleX;
			}
		}
		if (c < 254 && (Details.style == FONT_PAGER))
			return 29.4f * Details.scaleX;

		switch (Details.style)
		{
		case FONT_JAPANESE:
			return 29.4f * Details.scaleX;
		case FONT_BANK:
			return 10.0f * Details.scaleX;
		case FONT_PAGER:
			return 31.5f * Details.scaleX;
		default:
			return Size[0][Details.style][c] * Details.scaleX;
		}
	}
	else if(Details.proportional)
		return Size[LanguageSet][Details.style][c] * Details.scaleX;
	else
		return Size[LanguageSet][Details.style][192] * Details.scaleX;
#else
	if (Details.proportional)
		return Size[Details.style][c] * Details.scaleX;
	else
		return Size[Details.style][192] * Details.scaleX;
#endif // MORE_LANGUAGES
}

float
CFont::GetStringWidth(wchar *s, bool spaces)
{
	float w;

	w = 0.0f;
#ifdef MORE_LANGUAGES
	if (IsJapanese())
	{
		do
		{
			if ((*s != ' ' || spaces) && *s != '\0') {
				do {
					while (*s == '~' || *s == JAP_TERMINATION) {
						s++;
#ifdef BUTTON_ICONS
						switch (*s) {
#if 0 // unused
						case 'U':
						case 'D':
						case '<':
						case '>':
#endif
						case 'X':
						case 'O':
						case 'Q':
						case 'T':
						case 'K':
						case 'M':
						case 'A':
						case 'J':
						case 'V':
						case 'C':
							w += 17.0f * Details.scaleY;
							break;
						default:
							break;
						}
#endif
						while (!(*s == '~' || *s == JAP_TERMINATION)) s++;
						s++;
					}
					w += GetCharacterSize(*s - ' ');
					++s;
				} while (*s == '~' || *s == JAP_TERMINATION);
			}
		} while (IsAnsiCharacter(s));
	} else
#endif
	{
		for (; (*s != ' ' || spaces) && *s != '\0'; s++) {
			if (*s == '~') {
				s++;
#ifdef BUTTON_ICONS
				switch (*s) {
#if 0 // unused
				case 'U':
				case 'D':
				case '<':
				case '>':
#endif
				case 'X':
				case 'O':
				case 'Q':
				case 'T':
				case 'K':
				case 'M':
				case 'A':
				case 'J':
				case 'V':
				case 'C':
					w += 17.0f * Details.scaleY;
					break;
				default:
					break;
				}
#endif
				while (*s != '~') s++;
#ifndef FIX_BUGS
				s++;
				if (*s == ' ' && !spaces)
					break;
			} 
#else
			} else
#endif
				w += GetCharacterSize(*s - ' ');
		}
	}
	return w;
}

#ifdef MORE_LANGUAGES
float
CFont::GetStringWidth_Jap(wchar* s)
{
	float w;

	w = 0.0f;
	for (; *s != '\0';) {
		do {
			while (*s == '~' || *s == JAP_TERMINATION) {
				s++;
				while (!(*s == '~' || *s == JAP_TERMINATION)) s++;
				s++;
			}
			w += GetCharacterSize(*s - ' ');
			++s;
		} while (*s == '~' || *s == JAP_TERMINATION);
	}
	return w;
}
#endif

wchar*
CFont::GetNextSpace(wchar *s)
{
#ifdef MORE_LANGUAGES
	if (IsJapanese()) {
		do
		{
			if (*s != ' ' && *s != '\0') {
				do {
					while (*s == '~' || *s == JAP_TERMINATION) {
						s++;
						while (!(*s == '~' || *s == JAP_TERMINATION)) s++;
						s++;
					}
					++s;
				} while (*s == '~' || *s == JAP_TERMINATION);
			}
		} while (IsAnsiCharacter(s));
	} else
#endif
	{
		for(; *s != ' ' && *s != '\0'; s++)
			if(*s == '~'){
				s++;
				while(*s != '~') s++;
#ifndef FIX_BUGS
				s++;
				if(*s == ' ')
					break;
#endif
			}
	}
	return s;
}

#ifdef MORE_LANGUAGES
wchar*
CFont::ParseToken(wchar *s, wchar* ss, bool japShit)
{
	s++;
	if ((Details.color.r || Details.color.g || Details.color.b) && !japShit) {
		wchar c = *s;
		if (IsJapanese())
			c &= 0x7FFF;
		switch (c) {
		case 'N':
		case 'n':
			NewLine = true;
			break;
		case 'b': SetColor(CRGBA(128, 167, 243, 255)); break;
		case 'g': SetColor(CRGBA(95, 160, 106, 255)); break;
		case 'h': SetColor(CRGBA(225, 225, 225, 255)); break;
		case 'l': SetColor(CRGBA(0, 0, 0, 255)); break;
		case 'p': SetColor(CRGBA(168, 110, 252, 255)); break;
		case 'r': SetColor(CRGBA(113, 43, 73, 255)); break;
		case 'w': SetColor(CRGBA(175, 175, 175, 255)); break;
		case 'y': SetColor(CRGBA(210, 196, 106, 255)); break;
#ifdef BUTTON_ICONS
#if 0 // unused
		case 'U': PS2Symbol = BUTTON_UP; break;
		case 'D': PS2Symbol = BUTTON_DOWN; break;
		case '<': PS2Symbol = BUTTON_LEFT; break;
		case '>': PS2Symbol = BUTTON_RIGHT; break;
#endif
		case 'X': PS2Symbol = BUTTON_CROSS; break;
		case 'O': PS2Symbol = BUTTON_CIRCLE; break;
		case 'Q': PS2Symbol = BUTTON_SQUARE; break;
		case 'T': PS2Symbol = BUTTON_TRIANGLE; break;
		case 'K': PS2Symbol = BUTTON_L1; break;
		case 'M': PS2Symbol = BUTTON_L2; break;
		case 'A': PS2Symbol = BUTTON_L3; break;
		case 'J': PS2Symbol = BUTTON_R1; break;
		case 'V': PS2Symbol = BUTTON_R2; break;
		case 'C': PS2Symbol = BUTTON_R3; break;
#endif
		}
	} else if (IsJapanese()) {
		if ((*s & 0x7FFF) == 'N' || (*s & 0x7FFF) == 'n')
			NewLine = true;
	}
	while ((!IsJapanese() || (*s != JAP_TERMINATION)) && *s != '~') s++;
#ifdef FIX_BUGS
	if (*(++s) == '~')
		s = ParseToken(s, ss, japShit);
	return s;
#else
	return s + 1;
#endif
}
#else
wchar*
CFont::ParseToken(wchar *s, wchar*)
{
	s++;
	if(Details.color.r || Details.color.g || Details.color.b)
		switch(*s){
		case 'N':
		case 'n':
			NewLine = true;
			break;
		case 'b': SetColor(CRGBA(128, 167, 243, 255)); break;
		case 'g': SetColor(CRGBA(95, 160, 106, 255)); break;
		case 'h': SetColor(CRGBA(225, 225, 225, 255)); break;
		case 'l': SetColor(CRGBA(0, 0, 0, 255)); break;
		case 'p': SetColor(CRGBA(168, 110, 252, 255)); break;
		case 'r': SetColor(CRGBA(113, 43, 73, 255)); break;
		case 'w': SetColor(CRGBA(175, 175, 175, 255)); break;
		case 'y': SetColor(CRGBA(210, 196, 106, 255)); break;
#ifdef BUTTON_ICONS
#if 0 // unused
		case 'U': PS2Symbol = BUTTON_UP; break;
		case 'D': PS2Symbol = BUTTON_DOWN; break;
		case '<': PS2Symbol = BUTTON_LEFT; break;
		case '>': PS2Symbol = BUTTON_RIGHT; break;
#endif
		case 'X': PS2Symbol = BUTTON_CROSS; break;
		case 'O': PS2Symbol = BUTTON_CIRCLE; break;
		case 'Q': PS2Symbol = BUTTON_SQUARE; break;
		case 'T': PS2Symbol = BUTTON_TRIANGLE; break;
		case 'K': PS2Symbol = BUTTON_L1; break;
		case 'M': PS2Symbol = BUTTON_L2; break;
		case 'A': PS2Symbol = BUTTON_L3; break;
		case 'J': PS2Symbol = BUTTON_R1; break;
		case 'V': PS2Symbol = BUTTON_R2; break;
		case 'C': PS2Symbol = BUTTON_R3; break;
#endif
		}
	while(*s != '~') s++;
	return s+1;
}
#endif

void
CFont::DrawFonts(void)
{
	CSprite2d::DrawBank(Details.bank);
	CSprite2d::DrawBank(Details.bank+1);
	CSprite2d::DrawBank(Details.bank+2);
#ifdef MORE_LANGUAGES
	if (IsJapanese())
		CSprite2d::DrawBank(Details.bank+3);
#endif
}


void
CFont::SetScale(float x, float y)
{
#ifdef MORE_LANGUAGES
	/*if (IsJapanese()) {
		x *= 1.35f;
		y *= 1.25f;
	}*/
#endif
	Details.scaleX = x;
	Details.scaleY = y;
}

void
CFont::SetSlantRefPoint(float x, float y)
{
	Details.slantRefX = x;
	Details.slantRefY = y;
}

void
CFont::SetSlant(float s)
{
	Details.slant = s;
}

void
CFont::SetColor(CRGBA col)
{
	Details.color = col;
	if (Details.alphaFade < 255.0f)
		Details.color.a *= Details.alphaFade / 255.0f;
}

void
CFont::SetJustifyOn(void)
{
	Details.justify = true;
	Details.centre = false;
	Details.rightJustify = false;
}

void
CFont::SetJustifyOff(void)
{
	Details.justify = false;
	Details.rightJustify = false;
}

void
CFont::SetCentreOn(void)
{
	Details.centre = true;
	Details.justify = false;
	Details.rightJustify = false;
}

void
CFont::SetCentreOff(void)
{
	Details.centre = false;
}

void
CFont::SetWrapx(float x)
{
	Details.wrapX = x;
}

void
CFont::SetCentreSize(float s)
{
	Details.centreSize = s;
}

void
CFont::SetBackgroundOn(void)
{
	Details.background = true;
}

void
CFont::SetBackgroundOff(void)
{
	Details.background = false;
}

void
CFont::SetBackgroundColor(CRGBA col)
{
	Details.backgroundColor = col;
}

void
CFont::SetBackGroundOnlyTextOn(void)
{
	Details.backgroundOnlyText = true;
}

void
CFont::SetBackGroundOnlyTextOff(void)
{
	Details.backgroundOnlyText = false;
}

void
CFont::SetRightJustifyOn(void)
{
	Details.rightJustify = true;
	Details.justify = false;
	Details.centre = false;
}

void
CFont::SetRightJustifyOff(void)
{
	Details.rightJustify = false;
	Details.justify = false;
	Details.centre = false;
}

void
CFont::SetPropOn(void)
{
	Details.proportional = true;
}

void
CFont::SetPropOff(void)
{
	Details.proportional = false;
}

void
CFont::SetFontStyle(int16 style)
{
	Details.style = style;
}

void
CFont::SetRightJustifyWrap(float wrap)
{
	Details.rightJustifyWrap = wrap;
}

void
CFont::SetAlphaFade(float fade)
{
	Details.alphaFade = fade;
}

void
CFont::SetDropColor(CRGBA col)
{
	Details.dropColor = col;
	if (Details.alphaFade < 255.0f)
		Details.dropColor.a *= Details.alphaFade / 255.0f;
}

void
CFont::SetDropShadowPosition(int16 pos)
{
	Details.dropShadowPosition = pos;
}

wchar
CFont::character_code(uint8 c)
{
	if(c < 128)
		return c;
	return foreign_table[c-128];
}