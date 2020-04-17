#pragma once

class CWeatherEdit
{
    public:
	static bool m_bEditOn;
	static int  m_currentCommand;
	static int  m_channelSelect;
	static int  m_maxSelect;
	static char m_text[200];
	static CRGBA m_color;

	static void Update(void);
	static void Draw(void);
};