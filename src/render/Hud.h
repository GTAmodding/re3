#pragma once
#include "Sprite2d.h"

enum eSprites {
	HUD_FIST,
	HUD_BAT,
	HUD_PISTOL,
	HUD_UZI,
	HUD_SHOTGUN,
	HUD_AK47,
	HUD_M16,
	HUD_SNIPER,
	HUD_ROCKET,
	HUD_FLAME,
	HUD_MOLOTOV,
	HUD_GRENADE,
	HUD_DETONATOR,
	HUD_RADARDISC = 15,
	HUD_PAGER = 16,
	HUD_SITESNIPER = 20,
	HUD_SITEM16 = 21
};

class CHud {
public:
	static CSprite2d *Sprites;

	static wchar *m_HelpMessage;
	static wchar *m_LastHelpMessage;
	static int32 &m_HelpMessageState;
	static int32 &m_HelpMessageTimer;
	static int32 &m_HelpMessageFadeTimer;
	static wchar *m_HelpMessageToPrint;
	static Float &m_HelpMessageDisplayTime;
	static Float &m_fTextBoxNumLines;
	static Float &m_fHelpMessageTime;
	static Bool	&m_bHelpMessageQuick;
	static int32 m_ZoneState;
	static int32 m_ZoneFadeTimer;
	static int32 m_ZoneNameTimer;
	static wchar* &m_pZoneName;
	static wchar* m_pLastZoneName;
	static wchar* m_ZoneToPrint;
	static wchar* &m_pVehicleName;
	static wchar* m_pLastVehicleName;
	static wchar* m_pVehicleNameToPrint;
	static int32 m_VehicleState;
	static int32 m_VehicleFadeTimer;
	static int32 m_VehicleNameTimer;
	static wchar* m_Message;
	static wchar* m_PagerMessage;
	static Bool &m_Wants_To_Draw_Hud;
	static Bool &m_Wants_To_Draw_3dMarkers;
	static wchar(*m_BigMessage)[128];
	static Float* BigMessageInUse;
	static Float* BigMessageAlpha;
	static Float* BigMessageX;
	static Float &OddJob2OffTimer;
	static int8 &CounterOnLastFrame;
	static Float &OddJob2XOffset;
	static int16 &CounterFlashTimer;
	static int16 &OddJob2Timer;
	static int8	&TimerOnLastFrame;
	static int16 &OddJob2On;
	static int16 &TimerFlashTimer;
	static int16 &PagerSoundPlayed;
	static int32 &SpriteBrightness;
	static Float &PagerXOffset;
	static int32 m_ItemToFlash;
	static int16 &PagerTimer;
	static int16 &PagerOn;

public:
	static void Initialise();
	static void Shutdown();
	static void ReInitialise();
	static void GetRidOfAllHudMessages();
	static void SetZoneName(wchar* name);
	static void SetHelpMessage(wchar* message, bool quick);
	static void SetVehicleName(wchar* name);
	static void Draw();
	static void DrawAfterFade();
	static void SetMessage(wchar* message);
	static void SetBigMessage(wchar* message, int16 style);
	static void SetPagerMessage(wchar* message);
};