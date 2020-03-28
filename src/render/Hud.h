#pragma once

enum eItems
{
	ITEM_NONE = -1,
	ITEM_ARMOUR = 3,
	ITEM_HEALTH = 4,
	ITEM_RADAR = 8
};

enum eSprites 
{
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

class CHud
{
public:
	static class CSprite2d *Sprites;
	static int32 &SpriteBrightness;
	static wchar *m_HelpMessage;
	static wchar *m_LastHelpMessage;
	static int32 &m_HelpMessageState;
	static int32 &m_HelpMessageTimer;
	static int32 &m_HelpMessageFadeTimer;
	static wchar *m_HelpMessageToPrint;
	static float &m_HelpMessageDisplayTime;
	static float &m_fTextBoxNumLines;
	static float &m_fHelpMessageTime;
	static bool	&m_HelpMessageQuick;
	static int32 m_ZoneState;
	static int32 m_ZoneFadeTimer;
	static int32 m_ZoneNameTimer;
	static wchar *&m_pZoneName;
	static wchar *m_pLastZoneName;
	static wchar *m_ZoneToPrint;
	static wchar *&m_VehicleName;
	static wchar *m_pLastVehicleName;
	static wchar *m_pVehicleNameToPrint;
	static int32 m_VehicleState;
	static int32 m_VehicleFadeTimer;
	static int32 m_VehicleNameTimer;
	static wchar *m_Message;
	static wchar *m_PagerMessage;
	static bool &m_Wants_To_Draw_Hud;
	static bool &m_Wants_To_Draw_3dMarkers;
	static wchar(&m_BigMessage)[6][128];
	static int16 &m_ItemToFlash;

	// These aren't really in CHud
	static float BigMessageInUse[6];
	static float BigMessageAlpha[6];
	static float BigMessageX[6];
	static float &OddJob2OffTimer;
	static int8 &CounterOnLastFrame;
	static float &OddJob2XOffset;
	static int16 &CounterFlashTimer;
	static int16 &OddJob2Timer;
	static int8 &TimerOnLastFrame;
	static int16 &OddJob2On;
	static int16 &TimerFlashTimer;
	static int16 &PagerSoundPlayed;
	static float &PagerXOffset;
	static int16 &PagerTimer;
	static int16 &PagerOn;

public:
	static void Draw();
	static void DrawAfterFade();
	static void GetRidOfAllHudMessages();
	static void Initialise();
	static void ReInitialise();
	static void SetBigMessage(wchar *message, int16 style);
	static void SetHelpMessage(wchar *message, bool quick);
	static void SetMessage(wchar *message);
	static void SetPagerMessage(wchar *message);
	static void SetVehicleName(wchar *name);
	static void SetZoneName(wchar *name);
	static void Shutdown();
};
