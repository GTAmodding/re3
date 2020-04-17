#include "common.h"
#include "rwcore.h"
#include "Timecycle.h"
#include "WeatherEdit.h"
#include "Timer.h"
#include "Clock.h"
#include "Pad.h"
#include "ControllerConfig.h"
#include "Weather.h"
#include "skeleton.h"
#include "FileMgr.h"
#include "Sprite2d.h"
#include "Font.h"
#include "Coronas.h"

#define JUSTDOWN(b) (pad->NewState.b && !pad->OldState.b)
#define KEYJUSTDOWN(k) ControlsManager.GetIsKeyboardKeyJustDown((RsKeyCodes)k)
#define KEYDOWN(k) ControlsManager.GetIsKeyboardKeyDown((RsKeyCodes)k)
#define CTRLJUSTDOWN(key)                                                      \
	((KEYDOWN(rsLCTRL) || KEYDOWN(rsRCTRL)) && KEYJUSTDOWN((RsKeyCodes)key) || \
	 (KEYJUSTDOWN(rsLCTRL) || KEYJUSTDOWN(rsRCTRL)) && KEYDOWN((RsKeyCodes)key))
#define CTRLDOWN(key) ((KEYDOWN(rsLCTRL) || KEYDOWN(rsRCTRL)) && KEYDOWN((RsKeyCodes)key))

#define NUMHOURS 24
#define NUMWEATHERS 4

static const char *header = "// Amb     Dir          Sky top			Sky bot		SunCore 		SunCorona   SunSz  SprSz	SprBght Shdw LightShd  TreeShd FarClp  FogSt	LightOnGround LowCloudsRGB TopCloudRGB BottomCloudRGB		BlurRGBA\n";
static const char *weatherNames[] = {"SUNNY", "CLOUDY", "RAINY", "FOGGY"};
static const char *timeNames[] = {"Midnight", "1AM", "2AM", "3AM", "4AM", "5AM", "6AM", "7AM", "8AM", "9AM", "10AM", "11AM", "Midday",
								  "1PM", "2PM", "3PM", "4PM", "5PM", "6PM", "7PM", "8PM", "9PM", "10PM", "11PM"};

enum Commands
{
	IGNORED,
	HOUR,
	WEATHER,
	AMB,
	DIRECT,
	SKYTOP,
	SKYBOT,
	SUNCORE,
	SUNCORONA,
	SUNSZ,
	SPRSZ,
	SPRBGHT,
	SHADOW,
	LIGHTSHD,
	TREESHD,
	FARCLIP,
	FOGSTART,
	LIGHTONGROUND,
	LOWCLOUDS,
	TOPCLOUD,
	BOTTOMCLOUD,
	BLUR,
	LOAD,
	SAVE,
	NOTHING,

	NUMCOMMANDS,

	NUMVISIBLE = 9
};

const char *commandStrings[] = {
	"ignored",
	"HOUR",			// int
	"WEATHER",		// int
	"AMBIENT",		// RGB
	"DIRECTIONAL",	// RGB
	"SKY TOP",		// RGB
	"SKY BOTTOM",	// RGB
	"SUN CORE",		// RGB
	"SUN CORONA",	// RGB
	"SUN SIZE",		// float in III, *10 in VC
	"SPRITE SIZE",	// float in III, *10 in VC
	"SPRITE BGHT",	// float in III, *10 in VC
	"SHADOW",		// int
	"LIGHT SHD",	// int
	"TREE SHD",		// int
	"FAR CLIP",		// float in III, short in VC
	"FOG START",	// float in III, short in VC
	"LIGHT GROUND", // float in III, *10 in VC
	"LOW CLOUDS",	// RGB
	"TOP CLOUD",	// RGB
	"BOTTOM CLOUD", // RGB
	"BLUR",			// RGBA in III, RGB in VC
	"LOAD FILE",
	"SAVE FILE",
	"",
};

bool CWeatherEdit::m_bEditOn = false;
int CWeatherEdit::m_currentCommand = 1;
int CWeatherEdit::m_channelSelect = 0;
int CWeatherEdit::m_maxSelect = 3;
char CWeatherEdit::m_text[200];
CRGBA CWeatherEdit::m_color(255, 255, 255, 255);

#define SPEED1DOWN (KEYDOWN(rsLSHIFT) || KEYDOWN(rsRSHIFT))
#define SPEED2DOWN (KEYDOWN(rsLCTRL) || KEYDOWN(rsRCTRL))

void CWeatherEdit::Update(void)
{
	if(m_bEditOn)
		CTimer::SetCodePause(true);
	else
		CTimer::SetCodePause(false);
	
	CPad *pad = CPad::GetPad(1);

	// toggle mode
	if (KEYJUSTDOWN(rsRCTRL) && KEYDOWN(rsTAB))
	{
		m_bEditOn = !m_bEditOn;
		// Init
		if (m_bEditOn)
		{
			CWeather::NewWeatherType = CWeather::OldWeatherType;
			CWeather::InterpolationValue = 0.0f;
			CClock::ms_nGameClockMinutes = 0;
			CClock::ms_nGameClockSeconds = 0;
		}
	}
	if (!m_bEditOn)
		return;

	// select menu entry
	if (KEYJUSTDOWN(rsUP))
	{
		m_currentCommand--;
		if (m_currentCommand <= 0)
			m_currentCommand = NUMCOMMANDS - 1;
	}
	if (KEYJUSTDOWN(rsDOWN))
	{
		m_currentCommand++;
		if (m_currentCommand >= NUMCOMMANDS)
			m_currentCommand = 1;
	}

	bool channels = false;
	int h = CClock::ms_nGameClockHours;
	int w = CWeather::OldWeatherType;
	int proxycolor[4];
	int *pcolor[4];
	float finc, clamp = 100000.0f;
	float *floatp;
	int16 *intp;
	int iinc;

	m_color = CRGBA(0, 0, 0, 0);
	m_text[0] = '\0';
	switch (m_currentCommand)
	{
	case HOUR:
		if (KEYJUSTDOWN(rsLEFT))
		{
			CClock::ms_nGameClockHours--;
			if (CClock::ms_nGameClockHours < 0)
				CClock::ms_nGameClockHours = 23;
		}
		if (KEYJUSTDOWN(rsRIGHT))
		{
			CClock::ms_nGameClockHours++;
			if (CClock::ms_nGameClockHours >= 24)
				CClock::ms_nGameClockHours = 0;
		}
		sprintf(m_text, "%d", CClock::ms_nGameClockHours);
		break;
	case WEATHER:
		if (KEYJUSTDOWN(rsLEFT))
		{
			CWeather::OldWeatherType--;
			if (CWeather::OldWeatherType < 0)
				CWeather::OldWeatherType = NUMWEATHERS - 1;
		}
		if (KEYJUSTDOWN(rsRIGHT))
		{
			CWeather::OldWeatherType++;
			if (CWeather::OldWeatherType >= NUMWEATHERS)
				CWeather::OldWeatherType = 0;
		}
		CWeather::NewWeatherType = CWeather::OldWeatherType;
		sprintf(m_text, "%s", weatherNames[CWeather::OldWeatherType]);
		break;

	// RGB
	case AMB:
		pcolor[0] = &CTimeCycle::m_nAmbientRed[h][w];
		pcolor[1] = &CTimeCycle::m_nAmbientGreen[h][w];
		pcolor[2] = &CTimeCycle::m_nAmbientBlue[h][w];
		goto rgb;
	case DIRECT:
		pcolor[0] = &CTimeCycle::m_nDirectionalRed[h][w];
		pcolor[1] = &CTimeCycle::m_nDirectionalGreen[h][w];
		pcolor[2] = &CTimeCycle::m_nDirectionalBlue[h][w];
		goto rgb;
	case SKYTOP:
		pcolor[0] = &CTimeCycle::m_nSkyTopRed[h][w];
		pcolor[1] = &CTimeCycle::m_nSkyTopGreen[h][w];
		pcolor[2] = &CTimeCycle::m_nSkyTopBlue[h][w];
		goto rgb;
	case SKYBOT:
		pcolor[0] = &CTimeCycle::m_nSkyBottomRed[h][w];
		pcolor[1] = &CTimeCycle::m_nSkyBottomGreen[h][w];
		pcolor[2] = &CTimeCycle::m_nSkyBottomBlue[h][w];
		goto rgb;
	case SUNCORE:
		pcolor[0] = &CTimeCycle::m_nSunCoreRed[h][w];
		pcolor[1] = &CTimeCycle::m_nSunCoreGreen[h][w];
		pcolor[2] = &CTimeCycle::m_nSunCoreBlue[h][w];
		goto rgb;
	case SUNCORONA:
		pcolor[0] = &CTimeCycle::m_nSunCoronaRed[h][w];
		pcolor[1] = &CTimeCycle::m_nSunCoronaGreen[h][w];
		pcolor[2] = &CTimeCycle::m_nSunCoronaBlue[h][w];
		goto rgb;
	case LOWCLOUDS:
		pcolor[0] = &CTimeCycle::m_nLowCloudsRed[h][w];
		pcolor[1] = &CTimeCycle::m_nLowCloudsGreen[h][w];
		pcolor[2] = &CTimeCycle::m_nLowCloudsBlue[h][w];
		goto rgb;
	case TOPCLOUD:
		pcolor[0] = &CTimeCycle::m_nFluffyCloudsTopRed[h][w];
		pcolor[1] = &CTimeCycle::m_nFluffyCloudsTopGreen[h][w];
		pcolor[2] = &CTimeCycle::m_nFluffyCloudsTopBlue[h][w];
		goto rgb;
	case BOTTOMCLOUD:
		pcolor[0] = &CTimeCycle::m_nFluffyCloudsBottomRed[h][w];
		pcolor[1] = &CTimeCycle::m_nFluffyCloudsBottomGreen[h][w];
		pcolor[2] = &CTimeCycle::m_nFluffyCloudsBottomBlue[h][w];
		goto rgb;
	rgb:
		m_maxSelect = 3;
		goto channels;

	// RGBA
	case BLUR:
		// first convert to int, write back later
		proxycolor[0] = (int)CTimeCycle::m_fBlurRed[h][w];
		proxycolor[1] = (int)CTimeCycle::m_fBlurGreen[h][w];
		proxycolor[2] = (int)CTimeCycle::m_fBlurBlue[h][w];
		proxycolor[3] = (int)CTimeCycle::m_fBlurAlpha[h][w];

		pcolor[0] = &proxycolor[0];
		pcolor[1] = &proxycolor[1];
		pcolor[2] = &proxycolor[2];
		pcolor[3] = &proxycolor[3];
		m_maxSelect = 4;
		goto channels;
	channels:
		if (m_channelSelect >= m_maxSelect)
			m_channelSelect = m_maxSelect - 1;
		if (KEYJUSTDOWN(rsLEFT))
		{
			m_channelSelect--;
			if (m_channelSelect < 0)
				m_channelSelect = m_maxSelect - 1;
		}
		if (KEYJUSTDOWN(rsRIGHT))
		{
			m_channelSelect++;
			if (m_channelSelect >= m_maxSelect)
				m_channelSelect = 0;
		}

		iinc = SPEED1DOWN ? 2 : 1;
		// If R1 is being held, only increment once per button press
		if ((pad->NewState.Square && (!pad->NewState.RightShoulder1 || !pad->OldState.Square)) ||
			KEYJUSTDOWN(rsPGUP) || KEYDOWN(rsPGUP) && !KEYDOWN((RsKeyCodes)' '))
		{
			(*pcolor[m_channelSelect]) += iinc;
			if (*pcolor[m_channelSelect] > 255)
				*pcolor[m_channelSelect] = 255;
		}
		if ((pad->NewState.Cross && (!pad->NewState.RightShoulder1 || !pad->OldState.Cross)) ||
			KEYJUSTDOWN(rsPGDN) || KEYDOWN(rsPGDN) && !KEYDOWN((RsKeyCodes)' '))
		{
			(*pcolor[m_channelSelect]) -= iinc;
			if (*pcolor[m_channelSelect] < 0)
				*pcolor[m_channelSelect] = 0;
		}

		// format nicely
		char tmp[10], *p;
		p = m_text;
		for (int i = 0; i < m_maxSelect; i++)
		{
			sprintf(tmp, i == m_channelSelect ? ">%d" : " %d", *pcolor[i]);
			//			sprintf(p, "%s%5s", i ? " " : "", tmp);
			sprintf(p, "%5s", tmp);
			while (*p)
				p++;
		}
		m_color.r = *pcolor[0];
		m_color.g = *pcolor[1];
		m_color.b = *pcolor[2];
		m_color.a = 0xFF;
		break;

	// float
	case SUNSZ:
		floatp = &CTimeCycle::m_fSunSize[h][w];
		goto handlefloat;
	case SPRSZ:
		floatp = &CTimeCycle::m_fSpriteSize[h][w];
		goto handlefloat;
	case SPRBGHT:
		floatp = &CTimeCycle::m_fSpriteBrightness[h][w];
		clamp = 1.0f;
		goto handlefloat;
	case FARCLIP:
		floatp = &CTimeCycle::m_fFarClip[h][w];
		goto handlefloat;
	case FOGSTART:
		floatp = &CTimeCycle::m_fFogStart[h][w];
		goto handlefloat;
	case LIGHTONGROUND:
		floatp = &CTimeCycle::m_fLightsOnGroundBrightness[h][w];
	handlefloat:
		finc = 1.0f;
		if (SPEED1DOWN)
			finc = 0.5f;
		if (SPEED2DOWN)
			finc = 0.1f;
		if (SPEED1DOWN && SPEED2DOWN)
			finc = 10.0f;
		// If R1 is being held, only increment once per button press
		if ((pad->NewState.Square && (!pad->NewState.RightShoulder1 || !pad->OldState.Square)) ||
			KEYJUSTDOWN(rsPGUP) || KEYDOWN(rsPGUP) && !KEYDOWN((RsKeyCodes)' '))
		{
			*floatp += finc;
			if (*floatp > clamp)
				*floatp = clamp;
		}
		if ((pad->NewState.Cross && (!pad->NewState.RightShoulder1 || !pad->OldState.Cross)) ||
			KEYJUSTDOWN(rsPGDN) || KEYDOWN(rsPGDN) && !KEYDOWN((RsKeyCodes)' '))
		{
			*floatp -= finc;
			if (*floatp < 0.0f)
				*floatp = 0.0f;
		}
		sprintf(m_text, "%.4f", *floatp);
		break;

	// integer (short)
	case SHADOW:
		intp = &CTimeCycle::m_nShadowStrength[h][w];
		goto handleint;
	case LIGHTSHD:
		intp = &CTimeCycle::m_nLightShadowStrength[h][w];
		goto handleint;
	case TREESHD:
		intp = &CTimeCycle::m_nTreeShadowStrength[h][w];
		goto handleint;
	handleint:
		iinc = SPEED1DOWN ? 2 : 1;
		// If R1 is being held, only increment once per button press
		if ((pad->NewState.Square && (!pad->NewState.RightShoulder1 || !pad->OldState.Square)) ||
			KEYJUSTDOWN(rsPGUP) || KEYDOWN(rsPGUP) && !KEYDOWN((RsKeyCodes)' '))
		{
			*intp += iinc;
			if (*intp > 255)
				*intp = 255;
		}
		if ((pad->NewState.Cross && (!pad->NewState.RightShoulder1 || !pad->OldState.Cross)) ||
			KEYJUSTDOWN(rsPGDN) || KEYDOWN(rsPGDN) && !KEYDOWN((RsKeyCodes)' '))
		{
			*intp -= iinc;
			if (*intp < 0)
				*intp = 0;
		}
		sprintf(m_text, "%d", *intp);
		break;
	case LOAD:
		if (JUSTDOWN(Square) || KEYJUSTDOWN(rsENTER))
		{
			CTimeCycle::Initialise();
			sprintf(m_text, "TIMECYC.DAT LOADED");
		}
		break;

	case SAVE:
		if (JUSTDOWN(Square) || KEYJUSTDOWN(rsENTER))
			sprintf(m_text, "TIMECYC.DAT %s SAVED", CTimeCycle::Save() ? "" : "NOT");
		break;
	}

	if (m_currentCommand == BLUR)
	{
		m_color.r = (*pcolor[3] * m_color.r) / 255;
		m_color.g = (*pcolor[3] * m_color.g) / 255;
		m_color.b = (*pcolor[3] * m_color.b) / 255;
		// convert back to float
		CTimeCycle::m_fBlurRed[h][w] = (float)proxycolor[0];
		CTimeCycle::m_fBlurGreen[h][w] = (float)proxycolor[1];
		CTimeCycle::m_fBlurBlue[h][w] = (float)proxycolor[2];
		CTimeCycle::m_fBlurAlpha[h][w] = (float)proxycolor[3];
	}

	CTimeCycle::Update();
	CCoronas::DoSunAndMoon();
	CCoronas::Update();
}

static CRGBA activeEntry(0x9C, 0x5B, 0x28, 0xFF);
static CRGBA inactiveEntry(0xC1, 0xA4, 0x78, 0xFF);

// meh
void AsciiToUnicode(const char *src, wchar *dst);

void CWeatherEdit::Draw(void)
{
	char buf[200];
	uint16 unibuf[200];
	float scx = RsGlobal.maximumWidth / 640.0f;
	float scy = RsGlobal.maximumHeight / 448.0f;

	CFont::SetBackgroundOff();
	//	CFont::SetScale(scx*0.8f, scy*1.35f);
	CFont::SetScale(scx * 0.7f, scy * 0.7f);
	CFont::SetRightJustifyWrap(0.0f);
	CFont::SetBackGroundOnlyTextOff();

	AsciiToUnicode(m_text, unibuf);
	CFont::SetJustifyOff();
	CFont::SetCentreOn();
	CFont::SetPropOff();
	CFont::SetFontStyle(FONT_HEADING);

	CFont::SetColor(CRGBA(0, 0, 0, 255));
	CFont::PrintString(scx * (320 + 2), scy * (400 + 2), unibuf);
	CFont::SetColor(CRGBA(255, 255, 255, 255));
	CFont::PrintString(scx * 320, scy * 400, unibuf);

	CFont::SetCentreOff();
	CFont::SetRightJustifyOn();
	CFont::SetFontStyle(FONT_HEADING);
	CFont::SetPropOn();
	CFont::SetScale(scx * 0.7f, scy * 0.7f);
	int linespacing = 16;
	int x = 620;
	int y = 20;
	for (int i = 0; i < NUMVISIBLE; i++)
	{
		int cmd = i + m_currentCommand - NUMVISIBLE / 2;
		if (cmd >= NUMCOMMANDS)
			cmd -= NUMCOMMANDS - 1;
		if (cmd <= 0)
			cmd += NUMCOMMANDS - 1;

		sprintf(buf, "%s", commandStrings[cmd]);
		AsciiToUnicode(buf, unibuf);
		CFont::SetColor(CRGBA(0, 0, 0, 255));
		CFont::PrintString(scx * (x + 2), scy * (y + 2), unibuf);
		CFont::SetColor(cmd == m_currentCommand ? activeEntry : inactiveEntry);
		CFont::PrintString(scx * x, scy * y, unibuf);
		y += linespacing;
	}

	CSprite2d::DrawRect(CRect(scx * 580, scy * 388, scx * 620, scy * 428), m_color);
}