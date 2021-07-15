#pragma once
#include "Sprite2d.h"

#define HELP_MSG_LENGTH 256

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
	HUD_SITEM16,
	HUD_SITEROCKET,
	NUM_HUD_SPRITES,
};

class CHud
{
public:
	static int16 m_ItemToFlash;
	static CSprite2d Sprites[NUM_HUD_SPRITES];
	static wchar *m_pZoneName;
	static wchar *m_pLastZoneName;
	static wchar *m_ZoneToPrint;
	static wchar m_Message[256];
	static wchar m_BigMessage[6][128];
	static wchar m_PagerMessage[256];
	static uint32 m_ZoneNameTimer;
	static int32 m_ZoneFadeTimer;
	static uint32 m_ZoneState;
	static wchar m_HelpMessage[HELP_MSG_LENGTH];
	static wchar m_LastHelpMessage[HELP_MSG_LENGTH];
	static wchar m_HelpMessageToPrint[HELP_MSG_LENGTH];
	static uint32 m_HelpMessageTimer;
	static int32 m_HelpMessageFadeTimer;
	static uint32 m_HelpMessageState;
	static bool	m_HelpMessageQuick;
	static float m_HelpMessageDisplayTime;
	static int32 SpriteBrightness;
	static bool m_Wants_To_Draw_Hud;
	static bool m_Wants_To_Draw_3dMarkers;
	static wchar *m_pVehicleName;
	static wchar *m_pLastVehicleName;
	static uint32 m_VehicleNameTimer;
	static int32 m_VehicleFadeTimer;
	static uint32 m_VehicleState;
	static wchar *m_pVehicleNameToPrint;
public:
	static void Initialise();
	static void Shutdown();
	static void ReInitialise();
	static void GetRidOfAllHudMessages();
	static void SetZoneName(wchar *name);
	static void SetHelpMessage(wchar *message, bool quick);
	static void SetVehicleName(wchar *name);
	static void Draw();
	static void DrawAfterFade();
	static void SetMessage(wchar *message);
	static void SetBigMessage(wchar *message, uint16 style);
	static void SetPagerMessage(wchar *message);
};
