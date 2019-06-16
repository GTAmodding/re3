#include "common.h"
#include "patcher.h"
#include "Radar.h"
#include "Camera.h"
#include "Hud.h"
#include "World.h"
#include "Frontend.h"

WRAPPER void CRadar::ClearBlipForEntity(eBlipType type, int32 id) { EAXJMP(0x4A56C0); }
WRAPPER void CRadar::Draw3dMarkers() { EAXJMP(0x4A4C70); }
//WRAPPER void CRadar::DrawMap () { EAXJMP(0x4A4200); }
WRAPPER void CRadar::DrawBlips() { EAXJMP(0x4A42F0); }
WRAPPER void CRadar::DrawRadarMap() { EAXJMP(0x4A6C20); }

Float &CRadar::m_RadarRange = *(Float*)0x8E281C;
CVector2D &CRadar::vec2DRadarOrigin = *(CVector2D*)0x6299B8;

void CRadar::DrawMap() {
	if (!TheCamera.m_WideScreenOn && CHud::m_Wants_To_Draw_Hud) {
		if (FindPlayerVehicle()) {
			if (FindPlayerVehicle()->GetSpeed().Magnitude() > 0.3f) {
				if (FindPlayerVehicle()->GetSpeed().Magnitude() > 0.9f)
					CRadar::m_RadarRange = 350.0f;
				else
					CRadar::m_RadarRange = (FindPlayerVehicle()->GetSpeed().Magnitude() + 0.3f) * 200.0f;
			}
			else {
				CRadar::m_RadarRange = 120.0f;
			}
		}
		else {
			CRadar::m_RadarRange = 120.0f;
		}
		vec2DRadarOrigin.x = FindPlayerCentreOfWorld_NoSniperShift().x;
		vec2DRadarOrigin.y = FindPlayerCentreOfWorld_NoSniperShift().y;
		CRadar::DrawRadarMap();
	}
}

void CRadar::TransformRadarPointToScreenSpace(CVector2D *out, CVector2D *in) {
	out->x = in->x * HUD_STRETCH_X(47.0f) + HUD_STRETCH_X(47.0f + 20.0f);
	out->y =  (HUD_STRETCH_Y(76.0f)) * 0.5f + SCREEN_HEIGHT - (HUD_STRETCH_Y(123.0f)) - in->y * (HUD_STRETCH_Y(76.0f)) * 0.5f;
}

STARTPATCHES
	InjectHook(0x4A5040, CRadar::TransformRadarPointToScreenSpace, PATCH_JUMP);
ENDPATCHES