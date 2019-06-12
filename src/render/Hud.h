#pragma once
#include "Sprite2d.h"

enum eSprites {
	HUD_RADARDISC = 15,
	HUD_PAGER = 16
};

class CHud {
public:
	static CSprite2d *Sprites;

	static wchar *m_pHelpMessage;
	static wchar *m_pLastHelpMessage;
	static int &m_nHelpMessageState;
	static int &m_nHelpMessageTimer;
	static int &m_nHelpMessageFadeTimer;
	static wchar *m_pHelpMessageToPrint;
	static float &m_fTextBoxNumLines;
	static float &m_fHelpMessageTime;
	static bool	&m_bHelpMessageQuick;
	static int m_ZoneState;
	static int m_ZoneFadeTimer;
	static int m_ZoneNameTimer;
	static wchar* &m_pZoneName;
	static wchar* m_pLastZoneName;
	static wchar* m_ZoneToPrint;
	static wchar* &m_pVehicleName;
	static wchar* m_pLastVehicleName;
	static wchar* m_pVehicleNameToPrint;
	static int m_VehicleState;
	static int m_VehicleFadeTimer;
	static int m_VehicleNameTimer;
	static wchar* m_Message;
	static wchar* m_PagerMessage;
	static bool &m_bRetuneInProgress;
	static bool &m_Wants_To_Draw_Hud;
	static bool &m_Wants_To_Draw_3dMarkers;
	static wchar(*m_BigMessage)[128];
	static float* BigMessageInUse;
	static float* BigMessageAlpha;
	static float* BigMessageX;
	static float &OddJob2OffTimer;
	static char	&CounterOnLastFrame;
	static float &OddJob2XOffset;
	static __int16 &CounterFlashTimer;
	static __int16 &OddJob2Timer;
	static char	&TimerOnLastFrame;
	static __int16 &OddJob2On;
	static __int16 &TimerFlashTimer;
	static __int16 &PagerSoundPlayed;
	static int &SpriteBrightness;
	static float &PagerXOffset;
	static int m_ItemToFlash;
	static __int16 &PagerTimer;
	static __int16 &PagerOn;

public:
	static void Draw();
	static void DrawAfterFade();
	static int GetRidOfAllHudMessages();
	static void Initialise();
	static void ReInitialise();
	static int SetBigMessage(wchar* message, int style);
	static void SetHelpMessage(wchar* message, bool quick);
	static void SetMessage(wchar* message);
	static void SetPagerMessage(wchar *message);
	static int SetVehicleName(wchar* name);
	static int SetZoneName(wchar* name);
	static void Shutdown();

};