#if (!defined(GTA_PS2_STUFF) && defined(RWLIBS)) || defined(__MWERKS__)
#define WITHD3D
#endif
#include "config.h"
#include "common.h"

#include "RwHelper.h"
#include "Radar.h"
#include "Camera.h"
#include "Hud.h"
#include "World.h"
#include "Frontend.h"
#include "General.h"
#include "Vehicle.h"
#include "Pools.h"
#include "Script.h"
#include "TxdStore.h"
#include "World.h"
#include "Streaming.h"
#include "SpecialFX.h"

float CRadar::m_radarRange;
sRadarTrace CRadar::ms_RadarTrace[NUMRADARBLIPS];
CVector2D vec2DRadarOrigin;
int32 gRadarTxdIds[64];

CSprite2d CRadar::AsukaSprite;
CSprite2d CRadar::BombSprite;
CSprite2d CRadar::CatSprite;
CSprite2d CRadar::CentreSprite;
CSprite2d CRadar::CopcarSprite;
CSprite2d CRadar::DonSprite;
CSprite2d CRadar::EightSprite;
CSprite2d CRadar::ElSprite;
CSprite2d CRadar::IceSprite;
CSprite2d CRadar::JoeySprite;
CSprite2d CRadar::KenjiSprite;
CSprite2d CRadar::LizSprite;
CSprite2d CRadar::LuigiSprite;
CSprite2d CRadar::NorthSprite;
CSprite2d CRadar::RaySprite;
CSprite2d CRadar::SalSprite;
CSprite2d CRadar::SaveSprite;
CSprite2d CRadar::SpraySprite;
CSprite2d CRadar::TonySprite;
CSprite2d CRadar::WeaponSprite;

CSprite2d *CRadar::RadarSprites[RADAR_SPRITE_COUNT] = { 
	nil,
	&AsukaSprite,
	&BombSprite,
	&CatSprite,
	&CentreSprite,
	&CopcarSprite,
	&DonSprite,
	&EightSprite,
	&ElSprite,
	&IceSprite,
	&JoeySprite,
	&KenjiSprite,
	&LizSprite,
	&LuigiSprite,
	&NorthSprite,
	&RaySprite,
	&SalSprite,
	&SaveSprite,
	&SpraySprite,
	&TonySprite,
	&WeaponSprite
};

// Why this doesn't coincide with world coordinates i don't know
#define RADAR_MIN_X (-2000.0f)
#define RADAR_MIN_Y (-2000.0f)
#define RADAR_MAX_X (2000.0f)
#define RADAR_MAX_Y (2000.0f)
#define RADAR_SIZE_X (RADAR_MAX_X - RADAR_MIN_X)
#define RADAR_SIZE_Y (RADAR_MAX_Y - RADAR_MIN_Y)

#define RADAR_NUM_TILES (8)
#define RADAR_TILE_SIZE (RADAR_SIZE_X / RADAR_NUM_TILES)
static_assert(RADAR_TILE_SIZE == (RADAR_SIZE_Y / RADAR_NUM_TILES), "CRadar: not a square");

#define RADAR_MIN_RANGE (120.0f)
#define RADAR_MAX_RANGE (350.0f)
#define RADAR_MIN_SPEED (0.3f)
#define RADAR_MAX_SPEED (0.9f)

#ifdef MENU_MAP
int CRadar::TargetMarkerId = -1;
CVector CRadar::TargetMarkerPos;
#endif

// taken from VC
float CRadar::cachedCos;
float CRadar::cachedSin;

void ClipRadarTileCoords(int32 &x, int32 &y)
{
	if (x < 0)
		x = 0;
	if (x > RADAR_NUM_TILES-1)
		x = RADAR_NUM_TILES-1;
	if (y < 0)
		y = 0;
	if (y > RADAR_NUM_TILES-1)
		y = RADAR_NUM_TILES-1;
}

void RequestMapSection(int32 x, int32 y)
{
	ClipRadarTileCoords(x, y);
	CStreaming::RequestTxd(gRadarTxdIds[x + RADAR_NUM_TILES * y], STREAMFLAGS_DONT_REMOVE | STREAMFLAGS_DEPENDENCY);
}

void RemoveMapSection(int32 x, int32 y)
{
	if (x >= 0 && x <= RADAR_NUM_TILES - 1 && y >= 0 && y <= RADAR_NUM_TILES - 1)
		CStreaming::RemoveTxd(gRadarTxdIds[x + RADAR_NUM_TILES * y]);
}

// Transform from section indices to world coordinates
void GetTextureCorners(int32 x, int32 y, CVector2D *out)
{
	x =   x - RADAR_NUM_TILES/2;
	y = -(y - RADAR_NUM_TILES/2);

	// bottom left
	out[0].x = RADAR_TILE_SIZE * (x);
	out[0].y = RADAR_TILE_SIZE * (y - 1);

	// bottom right
	out[1].x = RADAR_TILE_SIZE * (x + 1);
	out[1].y = RADAR_TILE_SIZE * (y - 1);

	// top right
	out[2].x = RADAR_TILE_SIZE * (x + 1);
	out[2].y = RADAR_TILE_SIZE * (y);

	// top left
	out[3].x = RADAR_TILE_SIZE * (x);
	out[3].y = RADAR_TILE_SIZE * (y);
}


bool IsPointInsideRadar(const CVector2D &point)
{
	if (point.x < -1.0f || point.x > 1.0f) return false;
	if (point.y < -1.0f || point.y > 1.0f) return false;
	return true;
}

// clip line p1,p2 against (-1.0, 1.0) in x and y, set out to clipped point closest to p1
int LineRadarBoxCollision(CVector2D &out, const CVector2D &p1, const CVector2D &p2)
{
	float d1, d2;
	float t;
	float x, y;
	float shortest = 1.0f;
	int edge = -1;

	// clip against left edge, x = -1.0
	d1 = -1.0f - p1.x;
	d2 = -1.0f - p2.x;
	if (d1 * d2 < 0.0f) {
		// they are on opposite sides, get point of intersection
		t = d1 / (d1 - d2);
		y = (p2.y - p1.y)*t + p1.y;
		if (y >= -1.0f && y <= 1.0f && t <= shortest) {
			out.x = -1.0f;
			out.y = y;
			edge = 3;
			shortest = t;
		}
	}

	// clip against right edge, x = 1.0
	d1 = p1.x - 1.0f;
	d2 = p2.x - 1.0f;
	if (d1 * d2 < 0.0f) {
		// they are on opposite sides, get point of intersection
		t = d1 / (d1 - d2);
		y = (p2.y - p1.y)*t + p1.y;
		if (y >= -1.0f && y <= 1.0f && t <= shortest) {
			out.x = 1.0f;
			out.y = y;
			edge = 1;
			shortest = t;
		}
	}

	// clip against top edge, y = -1.0
	d1 = -1.0f - p1.y;
	d2 = -1.0f - p2.y;
	if (d1 * d2 < 0.0f) {
		// they are on opposite sides, get point of intersection
		t = d1 / (d1 - d2);
		x = (p2.x - p1.x)*t + p1.x;
		if (x >= -1.0f && x <= 1.0f && t <= shortest) {
			out.y = -1.0f;
			out.x = x;
			edge = 0;
			shortest = t;
		}
	}

	// clip against bottom edge, y = 1.0
	d1 = p1.y - 1.0f;
	d2 = p2.y - 1.0f;
	if (d1 * d2 < 0.0f) {
		// they are on opposite sides, get point of intersection
		t = d1 / (d1 - d2);
		x = (p2.x - p1.x)*t + p1.x;
		if (x >= -1.0f && x <= 1.0f && t <= shortest) {
			out.y = 1.0f;
			out.x = x;
			edge = 2;
			shortest = t;
		}
	}

	return edge;
}


uint8 CRadar::CalculateBlipAlpha(float dist)
{
#ifdef MENU_MAP
	if (CMenuManager::bMenuMapActive)
		return 255;
#endif
	if (dist <= 1.0f)
		return 255;

	if (dist <= 5.0f)
		return (128.0f * ((dist - 1.0f) / 4.0f)) + ((1.0f - (dist - 1.0f) / 4.0f) * 255.0f);

	return 128;
}

void CRadar::ChangeBlipBrightness(int32 i, int32 bright)
{
	int index = GetActualBlipArrayIndex(i);
	if (index != -1)
		ms_RadarTrace[index].m_bDim = bright != 1;
}

void CRadar::ChangeBlipColour(int32 i, int32 color)
{
	int index = GetActualBlipArrayIndex(i);
	if (index != -1)
		ms_RadarTrace[index].m_nColor = color;
}

void CRadar::ChangeBlipDisplay(int32 i, eBlipDisplay display)
{
	int index = GetActualBlipArrayIndex(i);
	if (index != -1)
		ms_RadarTrace[index].m_eBlipDisplay = display;
}

void CRadar::ChangeBlipScale(int32 i, int32 scale)
{
	int index = GetActualBlipArrayIndex(i);
	if (index != -1)
		ms_RadarTrace[index].m_wScale = scale;
}

void CRadar::ClearBlip(int32 i)
{
	int index = GetActualBlipArrayIndex(i);
	if (index != -1) {
		SetRadarMarkerState(index, false);
		ms_RadarTrace[index].m_bInUse = false;
#ifndef MENU_MAP
		// Ssshhh
		ms_RadarTrace[index].m_eBlipType = BLIP_NONE;
		ms_RadarTrace[index].m_eBlipDisplay = BLIP_DISPLAY_NEITHER;
		ms_RadarTrace[index].m_eRadarSprite = RADAR_SPRITE_NONE;
#endif
	}
}

void CRadar::ClearBlipForEntity(eBlipType type, int32 id)
{
	for (int i = 0; i < NUMRADARBLIPS; i++) {
		if (type == ms_RadarTrace[i].m_eBlipType && id == ms_RadarTrace[i].m_nEntityHandle) {
			SetRadarMarkerState(i, false);
			ms_RadarTrace[i].m_bInUse = false;
			ms_RadarTrace[i].m_eBlipType = BLIP_NONE;
			ms_RadarTrace[i].m_eBlipDisplay = BLIP_DISPLAY_NEITHER;
			ms_RadarTrace[i].m_eRadarSprite = RADAR_SPRITE_NONE;
		}
	};
}

// Why not a proper clipping algorithm?
int CRadar::ClipRadarPoly(CVector2D *poly, const CVector2D *rect)
{
	CVector2D corners[4] = {
		CVector2D(  1.0f, -1.0f ),	// top right
		CVector2D(  1.0f,  1.0f ),	// bottom right
		CVector2D( -1.0f,  1.0f ),	// bottom left
		CVector2D( -1.0f, -1.0f ),	// top left
	};
	CVector2D tmp;
	int i, j, n;
	int laste, e, e1, e2;;
	bool inside[4];

	for (i = 0; i < 4; i++)
		inside[i] = IsPointInsideRadar(rect[i]);

	laste = -1;
	n = 0;
	for (i = 0; i < 4; i++)
		if (inside[i]) {
			// point is inside, just add
			poly[n++] = rect[i];
		}
		else {
			// point is outside but line to this point might be clipped
			e1 = LineRadarBoxCollision(poly[n], rect[i], rect[(i + 4 - 1) % 4]);
			if (e1 != -1) {
				laste = e1;
				n++;
			}
			// and line from this point might be clipped as well
			e2 = LineRadarBoxCollision(poly[n], rect[i], rect[(i + 1) % 4]);
			if (e2 != -1) {
				if (e1 == -1) {
					// if other line wasn't clipped, i.e. it was complete outside,
					// we may have to insert another vertex if last clipped line
					// was on a different edge

					// find the last intersection if we haven't seen it yet
					if (laste == -1)
						for (j = 3; j >= i; j--) {
							// game uses an if here for j == 0
							e = LineRadarBoxCollision(tmp, rect[j], rect[(j + 4 - 1) % 4]);
							if (e != -1) {
								laste = e;
								break;
							}
						}
					assert(laste != -1);

					// insert corners that were skipped
					tmp = poly[n];
					for (e = laste; e != e2; e = (e + 1) % 4)
						poly[n++] = corners[e];
					poly[n] = tmp;
				}
				n++;
			}
		}

	if (n == 0) {
		// If no points, either the rectangle is completely outside or completely surrounds the radar
		// no idea what's going on here...
		float m = (rect[0].y - rect[1].y) / (rect[0].x - rect[1].x);
		if ((m*rect[3].x - rect[3].y) * (m*rect[0].x - rect[0].y) < 0.0f) {
			m = (rect[0].y - rect[3].y) / (rect[0].x - rect[3].x);
			if ((m*rect[1].x - rect[1].y) * (m*rect[0].x - rect[0].y) < 0.0f) {
				poly[0] = corners[0];
				poly[1] = corners[1];
				poly[2] = corners[2];
				poly[3] = corners[3];
				n = 4;
			}
		}
	}

	return n;
}

bool CRadar::DisplayThisBlip(int32 counter)
{
	switch (ms_RadarTrace[counter].m_eRadarSprite) {
	case RADAR_SPRITE_BOMB:
	case RADAR_SPRITE_SPRAY:
	case RADAR_SPRITE_WEAPON:
		return true;
	default:
		return false;
	}
}

void CRadar::Draw3dMarkers()
{
	for (int i = 0; i < NUMRADARBLIPS; i++) {
		if (ms_RadarTrace[i].m_bInUse) {
			switch (ms_RadarTrace[i].m_eBlipType) {
			case BLIP_CAR:
			{
				CEntity *entity = CPools::GetVehiclePool()->GetAt(ms_RadarTrace[i].m_nEntityHandle);
				if (ms_RadarTrace[i].m_eBlipDisplay == BLIP_DISPLAY_BOTH || ms_RadarTrace[i].m_eBlipDisplay == BLIP_DISPLAY_MARKER_ONLY) {
					CVector pos = entity->GetPosition();
					pos.z += 1.2f * CModelInfo::GetModelInfo(entity->GetModelIndex())->GetColModel()->boundingBox.max.z + 2.5f;
					C3dMarkers::PlaceMarker(i | (ms_RadarTrace[i].m_BlipIndex << 16), 1, pos, 2.5f, 0, 128, 255, 255, 1024, 0.2f, 5);
				}
				break;
			}
			case BLIP_CHAR:
			{
				CEntity *entity = CPools::GetPedPool()->GetAt(ms_RadarTrace[i].m_nEntityHandle);
				if (entity != nil) {
					if (((CPed*)entity)->InVehicle())
						entity = ((CPed * )entity)->m_pMyVehicle;
				}
				if (ms_RadarTrace[i].m_eBlipDisplay == BLIP_DISPLAY_BOTH || ms_RadarTrace[i].m_eBlipDisplay == BLIP_DISPLAY_MARKER_ONLY) {
					CVector pos = entity->GetPosition();
					pos.z += 3.0f;
					C3dMarkers::PlaceMarker(i | (ms_RadarTrace[i].m_BlipIndex << 16), 1, pos, 1.5f, 0, 128, 255, 255, 1024, 0.2f, 5);
				}
				break;
			}
			case BLIP_OBJECT:
			{
				CEntity *entity = CPools::GetObjectPool()->GetAt(ms_RadarTrace[i].m_nEntityHandle);
				if (ms_RadarTrace[i].m_eBlipDisplay == BLIP_DISPLAY_BOTH || ms_RadarTrace[i].m_eBlipDisplay == BLIP_DISPLAY_MARKER_ONLY) {
					CVector pos = entity->GetPosition();
					pos.z += CModelInfo::GetModelInfo(entity->GetModelIndex())->GetColModel()->boundingBox.max.z + 1.0f + 1.0f;
					C3dMarkers::PlaceMarker(i | (ms_RadarTrace[i].m_BlipIndex << 16), 1, pos, 1.0f, 0, 128, 255, 255, 1024, 0.2f, 5);
				}
				break;
			}
			case BLIP_COORD:
				break;
			case BLIP_CONTACT_POINT:
				if (!CTheScripts::IsPlayerOnAMission()) {
					if (ms_RadarTrace[i].m_eBlipDisplay == BLIP_DISPLAY_BOTH || ms_RadarTrace[i].m_eBlipDisplay == BLIP_DISPLAY_MARKER_ONLY)
						C3dMarkers::PlaceMarkerSet(i | (ms_RadarTrace[i].m_BlipIndex << 16), 4, ms_RadarTrace[i].m_vecPos, 2.0f, 0, 128, 255, 128, 2048, 0.2f, 0);
				}
				break;
			}
		}
	}
}

void CRadar::DrawBlips()
{
	if (!TheCamera.m_WideScreenOn && CHud::m_Wants_To_Draw_Hud) {
		RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)FALSE);
		RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*)FALSE);
		RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)TRUE);
		RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDSRCALPHA);
		RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDINVSRCALPHA);
		RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void*)FALSE);

		CVector2D out;
		CVector2D in = CVector2D(0.0f, 0.0f);
		TransformRadarPointToScreenSpace(out, in);

#ifdef MENU_MAP
		if (!CMenuManager::bMenuMapActive) {
#endif
			float angle;
			if (TheCamera.Cams[TheCamera.ActiveCam].Mode == CCam::MODE_TOPDOWN)
				angle = PI + FindPlayerHeading();
#ifdef FIX_BUGS
			else if (TheCamera.GetLookDirection() != LOOKING_FORWARD)
				angle = FindPlayerHeading() - (PI + (TheCamera.Cams[TheCamera.ActiveCam].CamTargetEntity->GetPosition() - TheCamera.Cams[TheCamera.ActiveCam].SourceBeforeLookBehind).Heading());
#endif
			else
				angle = FindPlayerHeading() - (PI + TheCamera.GetForward().Heading());

			DrawRotatingRadarSprite(&CentreSprite, out.x, out.y, angle, 255);

			CVector2D vec2d;
			vec2d.x = vec2DRadarOrigin.x;
			vec2d.y = M_SQRT2 * m_radarRange + vec2DRadarOrigin.y;
			TransformRealWorldPointToRadarSpace(in, vec2d);
			LimitRadarPoint(in);
			TransformRadarPointToScreenSpace(out, in);
			DrawRadarSprite(RADAR_SPRITE_NORTH, out.x, out.y, 255);
#ifdef MENU_MAP
		}
#endif

		CEntity *blipEntity = nil;
		for(int blipId = 0; blipId < NUMRADARBLIPS; blipId++) {
#ifdef MENU_MAP
			// A little hack to reuse cleared blips in menu map. hehe
			if (!CMenuManager::bMenuMapActive || ms_RadarTrace[blipId].m_eBlipType == BLIP_CAR ||
				ms_RadarTrace[blipId].m_eBlipType == BLIP_CHAR || ms_RadarTrace[blipId].m_eBlipType == BLIP_OBJECT)
#endif
			if (!ms_RadarTrace[blipId].m_bInUse)
				continue;

			switch (ms_RadarTrace[blipId].m_eBlipType) {
				case BLIP_CAR:
				case BLIP_CHAR:
				case BLIP_OBJECT:
					if (ms_RadarTrace[blipId].m_eRadarSprite == RADAR_SPRITE_BOMB || ms_RadarTrace[blipId].m_eRadarSprite == RADAR_SPRITE_SAVE
						|| ms_RadarTrace[blipId].m_eRadarSprite == RADAR_SPRITE_SPRAY || ms_RadarTrace[blipId].m_eRadarSprite == RADAR_SPRITE_WEAPON) {

						switch (ms_RadarTrace[blipId].m_eBlipType) {
							case BLIP_CAR:
								blipEntity = CPools::GetVehiclePool()->GetAt(ms_RadarTrace[blipId].m_nEntityHandle);
								break;
							case BLIP_CHAR:
								blipEntity = CPools::GetPedPool()->GetAt(ms_RadarTrace[blipId].m_nEntityHandle);
								if (blipEntity != nil) {
									if (((CPed*)blipEntity)->InVehicle())
										blipEntity = ((CPed*)blipEntity)->m_pMyVehicle;
								}
								break;
							case BLIP_OBJECT:
								blipEntity = CPools::GetObjectPool()->GetAt(ms_RadarTrace[blipId].m_nEntityHandle);
								break;
							default:
								break;
						}
						if (blipEntity) {
							uint32 color = GetRadarTraceColour(ms_RadarTrace[blipId].m_nColor, ms_RadarTrace[blipId].m_bDim);
							if (ms_RadarTrace[blipId].m_eBlipDisplay == BLIP_DISPLAY_BOTH || ms_RadarTrace[blipId].m_eBlipDisplay == BLIP_DISPLAY_MARKER_ONLY) {
								if (CTheScripts::IsDebugOn()) {
									ShowRadarMarker(blipEntity->GetPosition(), color, ms_RadarTrace[blipId].m_Radius);
									ms_RadarTrace[blipId].m_Radius = ms_RadarTrace[blipId].m_Radius - 0.1f;
									if (ms_RadarTrace[blipId].m_Radius < 1.0f)
										ms_RadarTrace[blipId].m_Radius = 5.0f;
								}
							}
							if (ms_RadarTrace[blipId].m_eBlipDisplay == BLIP_DISPLAY_BOTH || ms_RadarTrace[blipId].m_eBlipDisplay == BLIP_DISPLAY_BLIP_ONLY) {
								TransformRealWorldPointToRadarSpace(in, blipEntity->GetPosition());
								float dist = LimitRadarPoint(in);
								TransformRadarPointToScreenSpace(out, in);
								if (ms_RadarTrace[blipId].m_eRadarSprite != RADAR_SPRITE_NONE) {
									DrawRadarSprite(ms_RadarTrace[blipId].m_eRadarSprite, out.x, out.y, CalculateBlipAlpha(dist));
								} else {
#ifdef TRIANGULAR_BLIPS
								    const CVector &pos = FindPlayerCentreOfWorld_NoSniperShift();
									const CVector &blipPos = blipEntity->GetPosition();
									uint8 mode = BLIP_MODE_TRIANGULAR_UP;
									if (blipPos.z - pos.z <= 2.0f) {
										if (blipPos.z - pos.z < -4.0f) mode = BLIP_MODE_TRIANGULAR_DOWN;
										else mode = BLIP_MODE_SQUARE;
									}
									ShowRadarTraceWithHeight(out.x, out.y, ms_RadarTrace[blipId].m_wScale, (uint8)(color >> 24), (uint8)(color >> 16), (uint8)(color >> 8), 255, mode);
#else
									ShowRadarTrace(out.x, out.y, ms_RadarTrace[blipId].m_wScale, (uint8)(color >> 24), (uint8)(color >> 16), (uint8)(color >> 8), 255);
#endif
								}
							}
						}
					}
					break;
				case BLIP_COORD:
				case BLIP_CONTACT_POINT:
					if ((ms_RadarTrace[blipId].m_eRadarSprite == RADAR_SPRITE_BOMB || ms_RadarTrace[blipId].m_eRadarSprite == RADAR_SPRITE_SAVE
						|| ms_RadarTrace[blipId].m_eRadarSprite == RADAR_SPRITE_SPRAY || ms_RadarTrace[blipId].m_eRadarSprite == RADAR_SPRITE_WEAPON)
						&& (ms_RadarTrace[blipId].m_eBlipType != BLIP_CONTACT_POINT || !CTheScripts::IsPlayerOnAMission())) {

						uint32 color = GetRadarTraceColour(ms_RadarTrace[blipId].m_nColor, ms_RadarTrace[blipId].m_bDim);
						if (ms_RadarTrace[blipId].m_eBlipDisplay == BLIP_DISPLAY_BOTH || ms_RadarTrace[blipId].m_eBlipDisplay == BLIP_DISPLAY_MARKER_ONLY) {
							if (CTheScripts::IsDebugOn()) {
								ShowRadarMarker(ms_RadarTrace[blipId].m_vecPos, color, ms_RadarTrace[blipId].m_Radius);
								ms_RadarTrace[blipId].m_Radius = ms_RadarTrace[blipId].m_Radius - 0.1f;
								if (ms_RadarTrace[blipId].m_Radius < 1.0f)
									ms_RadarTrace[blipId].m_Radius = 5.0f;
							}
						}
						if (ms_RadarTrace[blipId].m_eBlipDisplay == BLIP_DISPLAY_BOTH || ms_RadarTrace[blipId].m_eBlipDisplay == BLIP_DISPLAY_BLIP_ONLY) {
							TransformRealWorldPointToRadarSpace(in, ms_RadarTrace[blipId].m_vec2DPos);
							float dist = LimitRadarPoint(in);
							TransformRadarPointToScreenSpace(out, in);
							if (ms_RadarTrace[blipId].m_eRadarSprite != RADAR_SPRITE_NONE) {
								DrawRadarSprite(ms_RadarTrace[blipId].m_eRadarSprite, out.x, out.y, CalculateBlipAlpha(dist));
							} else {
#ifdef TRIANGULAR_BLIPS
							    const CVector &pos = FindPlayerCentreOfWorld_NoSniperShift();
							    const CVector &blipPos = ms_RadarTrace[blipId].m_vecPos;
								uint8 mode = BLIP_MODE_TRIANGULAR_UP;
								if (blipPos.z - pos.z <= 2.0f) {
									if (blipPos.z - pos.z < -4.0f) mode = BLIP_MODE_TRIANGULAR_DOWN;
									else mode = BLIP_MODE_SQUARE;
								}
								ShowRadarTraceWithHeight(out.x, out.y, ms_RadarTrace[blipId].m_wScale, (uint8)(color >> 24), (uint8)(color >> 16), (uint8)(color >> 8), 255, mode);
#else
								ShowRadarTrace(out.x, out.y, ms_RadarTrace[blipId].m_wScale, (uint8)(color >> 24), (uint8)(color >> 16), (uint8)(color >> 8), 255);
#endif
							}
						}
					}
					break;
				default:
					break;
			}
		}
		for(int blipId = 0; blipId < NUMRADARBLIPS; blipId++) {
			if (!ms_RadarTrace[blipId].m_bInUse)
				continue;

			switch (ms_RadarTrace[blipId].m_eBlipType) {
				case BLIP_CAR:
				case BLIP_CHAR:
				case BLIP_OBJECT:
					if (ms_RadarTrace[blipId].m_eRadarSprite != RADAR_SPRITE_BOMB && ms_RadarTrace[blipId].m_eRadarSprite != RADAR_SPRITE_SAVE
						&& ms_RadarTrace[blipId].m_eRadarSprite != RADAR_SPRITE_SPRAY && ms_RadarTrace[blipId].m_eRadarSprite != RADAR_SPRITE_WEAPON) {

						switch (ms_RadarTrace[blipId].m_eBlipType) {
							case BLIP_CAR:
								blipEntity = CPools::GetVehiclePool()->GetAt(ms_RadarTrace[blipId].m_nEntityHandle);
								break;
							case BLIP_CHAR:
								blipEntity = CPools::GetPedPool()->GetAt(ms_RadarTrace[blipId].m_nEntityHandle);
								if (blipEntity != nil) {
									if (((CPed*)blipEntity)->InVehicle())
										blipEntity = ((CPed*)blipEntity)->m_pMyVehicle;
								}
								break;
							case BLIP_OBJECT:
								blipEntity = CPools::GetObjectPool()->GetAt(ms_RadarTrace[blipId].m_nEntityHandle);
								break;
							default:
								break;
						}

						if (blipEntity) {
							uint32 color = GetRadarTraceColour(ms_RadarTrace[blipId].m_nColor, ms_RadarTrace[blipId].m_bDim);
							if (ms_RadarTrace[blipId].m_eBlipDisplay == BLIP_DISPLAY_BOTH || ms_RadarTrace[blipId].m_eBlipDisplay == BLIP_DISPLAY_MARKER_ONLY) {
								if (CTheScripts::IsDebugOn()) {
									ShowRadarMarker(blipEntity->GetPosition(), color, ms_RadarTrace[blipId].m_Radius);
									ms_RadarTrace[blipId].m_Radius = ms_RadarTrace[blipId].m_Radius - 0.1f;
									if (ms_RadarTrace[blipId].m_Radius < 1.0f)
										ms_RadarTrace[blipId].m_Radius = 5.0f;
								}
							}
							if (ms_RadarTrace[blipId].m_eBlipDisplay == BLIP_DISPLAY_BOTH || ms_RadarTrace[blipId].m_eBlipDisplay == BLIP_DISPLAY_BLIP_ONLY) {
								TransformRealWorldPointToRadarSpace(in, blipEntity->GetPosition());
								float dist = LimitRadarPoint(in);
								TransformRadarPointToScreenSpace(out, in);
								if (ms_RadarTrace[blipId].m_eRadarSprite != RADAR_SPRITE_NONE)
									DrawRadarSprite(ms_RadarTrace[blipId].m_eRadarSprite, out.x, out.y, CalculateBlipAlpha(dist));
								else
#ifdef TRIANGULAR_BLIPS
								{
								    const CVector &pos = FindPlayerCentreOfWorld_NoSniperShift();
									const CVector &blipPos = blipEntity->GetPosition();
									uint8 mode = BLIP_MODE_TRIANGULAR_UP;
									if (blipPos.z - pos.z <= 2.0f) {
										if (blipPos.z - pos.z < -4.0f) mode = BLIP_MODE_TRIANGULAR_DOWN;
										else mode = BLIP_MODE_SQUARE;
									}
									ShowRadarTraceWithHeight(out.x, out.y, ms_RadarTrace[blipId].m_wScale, (uint8)(color >> 24), (uint8)(color >> 16), (uint8)(color >> 8), 255, mode);
								}
#else
									ShowRadarTrace(out.x, out.y, ms_RadarTrace[blipId].m_wScale, (uint8)(color >> 24), (uint8)(color >> 16), (uint8)(color >> 8), 255);
#endif
							}
						}
					}
					break;
				default:
					break;
			}
		}
		for (int blipId = 0; blipId < NUMRADARBLIPS; blipId++) {
			if (!ms_RadarTrace[blipId].m_bInUse)
				continue;

			switch (ms_RadarTrace[blipId].m_eBlipType) {
				case BLIP_COORD:
				case BLIP_CONTACT_POINT:
					if (ms_RadarTrace[blipId].m_eRadarSprite != RADAR_SPRITE_BOMB && ms_RadarTrace[blipId].m_eRadarSprite != RADAR_SPRITE_SAVE
						&& ms_RadarTrace[blipId].m_eRadarSprite != RADAR_SPRITE_SPRAY && ms_RadarTrace[blipId].m_eRadarSprite != RADAR_SPRITE_WEAPON
						&& (ms_RadarTrace[blipId].m_eBlipType != BLIP_CONTACT_POINT || !CTheScripts::IsPlayerOnAMission())) {

						uint32 color = GetRadarTraceColour(ms_RadarTrace[blipId].m_nColor, ms_RadarTrace[blipId].m_bDim);
						if (ms_RadarTrace[blipId].m_eBlipDisplay == BLIP_DISPLAY_BOTH || ms_RadarTrace[blipId].m_eBlipDisplay == BLIP_DISPLAY_MARKER_ONLY) {
							if (CTheScripts::IsDebugOn()) {
								ShowRadarMarker(ms_RadarTrace[blipId].m_vecPos, color, ms_RadarTrace[blipId].m_Radius);
								ms_RadarTrace[blipId].m_Radius = ms_RadarTrace[blipId].m_Radius - 0.1f;
								if (ms_RadarTrace[blipId].m_Radius < 1.0f)
									ms_RadarTrace[blipId].m_Radius = 5.0f;
							}
						}
						if (ms_RadarTrace[blipId].m_eBlipDisplay == BLIP_DISPLAY_BOTH || ms_RadarTrace[blipId].m_eBlipDisplay == BLIP_DISPLAY_BLIP_ONLY) {
							TransformRealWorldPointToRadarSpace(in, ms_RadarTrace[blipId].m_vec2DPos);
							float dist = LimitRadarPoint(in);
							TransformRadarPointToScreenSpace(out, in);
							if (ms_RadarTrace[blipId].m_eRadarSprite != RADAR_SPRITE_NONE)
								DrawRadarSprite(ms_RadarTrace[blipId].m_eRadarSprite, out.x, out.y, CalculateBlipAlpha(dist));
							else
#ifdef TRIANGULAR_BLIPS
							{
							    const CVector &pos = FindPlayerCentreOfWorld_NoSniperShift();
							    const CVector &blipPos = ms_RadarTrace[blipId].m_vecPos;
								uint8 mode = BLIP_MODE_TRIANGULAR_UP;
								if (blipPos.z - pos.z <= 2.0f) {
									if (blipPos.z - pos.z < -4.0f) mode = BLIP_MODE_TRIANGULAR_DOWN;
									else mode = BLIP_MODE_SQUARE;
								}
								ShowRadarTraceWithHeight(out.x, out.y, ms_RadarTrace[blipId].m_wScale, (uint8)(color >> 24), (uint8)(color >> 16), (uint8)(color >> 8), 255, mode);
							}
#else
								ShowRadarTrace(out.x, out.y, ms_RadarTrace[blipId].m_wScale, (uint8)(color >> 24), (uint8)(color >> 16), (uint8)(color >> 8), 255);
#endif
						}
					}
					break;
				default:
					break;
			}
		}
#ifdef MENU_MAP
		if (CMenuManager::bMenuMapActive) {
			CVector2D in, out;
			TransformRealWorldPointToRadarSpace(in, FindPlayerCentreOfWorld_NoSniperShift());
			LimitRadarPoint(in);
			TransformRadarPointToScreenSpace(out, in);
			DrawYouAreHereSprite(out.x, out.y);
		}
#endif
	}
}

void CRadar::DrawMap()
{
	if (!TheCamera.m_WideScreenOn && CHud::m_Wants_To_Draw_Hud) {
#if 1 // from VC
		CalculateCachedSinCos();
#endif
		if (FindPlayerVehicle()) {
			float speed = FindPlayerSpeed().Magnitude();
			if (speed < RADAR_MIN_SPEED)
				m_radarRange = RADAR_MIN_RANGE;
			else if (speed < RADAR_MAX_SPEED)
				m_radarRange = (speed - RADAR_MIN_SPEED)/(RADAR_MAX_SPEED-RADAR_MIN_SPEED) * (RADAR_MAX_RANGE-RADAR_MIN_RANGE) + RADAR_MIN_RANGE;
			else
				m_radarRange = RADAR_MAX_RANGE;
		}
		else
			m_radarRange = RADAR_MIN_RANGE;

		vec2DRadarOrigin = CVector2D(FindPlayerCentreOfWorld_NoSniperShift());
		DrawRadarMap();
	}
}

void CRadar::DrawRadarMap()
{
	// Game calculates an unused CRect here

	DrawRadarMask();

	// top left ist (0, 0)
	int x = Floor((vec2DRadarOrigin.x - RADAR_MIN_X) / RADAR_TILE_SIZE);
	int y = Ceil((RADAR_NUM_TILES - 1) - (vec2DRadarOrigin.y - RADAR_MIN_Y) / RADAR_TILE_SIZE);
	StreamRadarSections(x, y);

	RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void*)FALSE);
	RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDSRCALPHA);
	RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDINVSRCALPHA);
	RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void*)rwFILTERLINEAR);
	RwRenderStateSet(rwRENDERSTATESHADEMODE, (void*)rwSHADEMODEFLAT);
	RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*)TRUE);
	RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)FALSE);
	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)FALSE);
	RwRenderStateSet(rwRENDERSTATETEXTUREADDRESS, (void*)rwTEXTUREADDRESSCLAMP);
	RwRenderStateSet(rwRENDERSTATETEXTUREPERSPECTIVE, (void*)FALSE);

	DrawRadarSection(x - 1, y - 1);
	DrawRadarSection(x, y - 1);
	DrawRadarSection(x + 1, y - 1);
	DrawRadarSection(x - 1, y);
	DrawRadarSection(x, y);
	DrawRadarSection(x + 1, y);
	DrawRadarSection(x - 1, y + 1);
	DrawRadarSection(x, y + 1);
	DrawRadarSection(x + 1, y + 1);
}

void CRadar::DrawRadarMask() 
{ 
	CVector2D corners[4] = {
		CVector2D(1.0f, -1.0f),
		CVector2D(1.0f, 1.0f),
		CVector2D(-1.0f, 1.0f),
		CVector2D(-1.0, -1.0f)
	};

	RwRenderStateSet(rwRENDERSTATETEXTURERASTER, (void*)nil);
	RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void*)FALSE);
	RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void*)rwFILTERLINEAR);
	RwRenderStateSet(rwRENDERSTATESHADEMODE, (void*)rwSHADEMODEFLAT);
	RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*)FALSE);
	RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)TRUE);
	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)TRUE);
#if !defined(GTA_PS2_STUFF) && defined(RWLIBS)
	RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDSRCALPHA);
	RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDINVSRCALPHA);
	RwD3D8SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_ALWAYS);
#else
	RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDZERO);
	RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDONE);
#endif

	CVector2D out[8];
	CVector2D in;

	// Draw the shape we want to mask out from the radar in four segments
	for (int i = 0; i < 4; i++) {
		// First point is always the corner itself
		in.x = corners[i].x;
		in.y = corners[i].y;
		TransformRadarPointToScreenSpace(out[0], in);

		// Then generate a quarter of the circle
		for (int j = 0; j < 7; j++) {
			in.x = corners[i].x * Cos(j * (PI / 2.0f / 6.0f));
			in.y = corners[i].y * Sin(j * (PI / 2.0f / 6.0f));
			TransformRadarPointToScreenSpace(out[j + 1], in);
		};

		CSprite2d::SetMaskVertices(8, (float *)out);
		RwIm2DRenderPrimitive(rwPRIMTYPETRIFAN, CSprite2d::GetVertices(), 8);
	}
#if !defined(GTA_PS2_STUFF) && defined(RWLIBS)
	RwD3D8SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);
#endif
}

void CRadar::DrawRadarSection(int32 x, int32 y)
{
	int i;
	RwTexDictionary *txd;
	CVector2D worldPoly[8];
	CVector2D radarCorners[4];
	CVector2D radarPoly[8];
	CVector2D texCoords[8];
	CVector2D screenPoly[8];
	int numVertices;
	RwTexture *texture = nil;

	GetTextureCorners(x, y, worldPoly);
	ClipRadarTileCoords(x, y);

	assert(CTxdStore::GetSlot(gRadarTxdIds[x + RADAR_NUM_TILES * y]));
	txd = CTxdStore::GetSlot(gRadarTxdIds[x + RADAR_NUM_TILES * y])->texDict;
	if (txd)
		texture = GetFirstTexture(txd);
	if (texture == nil)
		return;

	for (i = 0; i < 4; i++)
		TransformRealWorldPointToRadarSpace(radarCorners[i], worldPoly[i]);

	numVertices = ClipRadarPoly(radarPoly, radarCorners);

	// FIX: can return earlier here
//	if(numVertices == 0)
	if (numVertices < 3)
		return;

	for (i = 0; i < numVertices; i++) {
		TransformRadarPointToRealWorldSpace(worldPoly[i], radarPoly[i]);
		TransformRealWorldToTexCoordSpace(texCoords[i], worldPoly[i], x, y);
		TransformRadarPointToScreenSpace(screenPoly[i], radarPoly[i]);
	}
	RwRenderStateSet(rwRENDERSTATETEXTURERASTER, RwTextureGetRaster(texture));
	CSprite2d::SetVertices(numVertices, (float*)screenPoly, (float*)texCoords, CRGBA(255, 255, 255, 255));
	// check done above now
//	if(numVertices > 2)
	RwIm2DRenderPrimitive(rwPRIMTYPETRIFAN, CSprite2d::GetVertices(), numVertices);
}

void CRadar::DrawRadarSprite(uint16 sprite, float x, float y, uint8 alpha)
{
	RadarSprites[sprite]->Draw(CRect(x - SCREEN_SCALE_X(8.0f), y - SCREEN_SCALE_Y(8.0f), x + SCREEN_SCALE_X(8.0f), y + SCREEN_SCALE_Y(8.0f)), CRGBA(255, 255, 255, alpha));
}

void CRadar::DrawRotatingRadarSprite(CSprite2d* sprite, float x, float y, float angle, int32 alpha)
{
	CVector curPosn[4];
	const float sizeX = SCREEN_SCALE_X(8.0f);
	const float correctedAngle = angle - PI / 4.f;
	const float sizeY = SCREEN_SCALE_Y(8.0f);

	for (uint32 i = 0; i < 4; i++) {
		const float cornerAngle = i * HALFPI + correctedAngle;
		curPosn[i].x = x + (0.0f * Cos(cornerAngle) + 1.0f * Sin(cornerAngle)) * sizeX;
		curPosn[i].y = y - (0.0f * Sin(cornerAngle) - 1.0f * Cos(cornerAngle)) * sizeY;
	}

	sprite->Draw(curPosn[3].x, curPosn[3].y, curPosn[2].x, curPosn[2].y, curPosn[0].x, curPosn[0].y, curPosn[1].x, curPosn[1].y, CRGBA(255, 255, 255, alpha));
}

int32 CRadar::GetActualBlipArrayIndex(int32 i)
{
	if (i == -1)
		return -1;
	else if ((i & 0xFFFF0000) >> 16 != ms_RadarTrace[(uint16)i].m_BlipIndex)
		return -1;
	else
		return (uint16)i;
}

int32 CRadar::GetNewUniqueBlipIndex(int32 i)
{
	if (ms_RadarTrace[i].m_BlipIndex >= UINT16_MAX - 1)
		ms_RadarTrace[i].m_BlipIndex = 1;
	else
		ms_RadarTrace[i].m_BlipIndex++;
	return i | (ms_RadarTrace[i].m_BlipIndex << 16);
}

uint32 CRadar::GetRadarTraceColour(uint32 color, bool bright)
{
	uint32 c;
	switch (color) {
	case RADAR_TRACE_RED:
		if (bright)
			c = 0x712B49FF;
		else
			c = 0x7F0000FF;
		break;
	case RADAR_TRACE_GREEN:
		if (bright)
			c = 0x5FA06AFF;
		else
			c = 0x007F00FF;
		break;
	case RADAR_TRACE_LIGHT_BLUE:
		if (bright)
			c = 0x80A7F3FF;
		else
			c = 0x00007FFF;
		break;
	case RADAR_TRACE_GRAY:
		if (bright)
			c = 0xE1E1E1FF;
		else
			c = 0x7F7F7FFF;
		break;
	case RADAR_TRACE_YELLOW:
		if (bright)
			c = 0xFFFF00FF;
		else
			c = 0x7F7F00FF;
		break;
	case RADAR_TRACE_MAGENTA:
		if (bright)
			c = 0xFF00FFFF;
		else
			c = 0x7F007FFF;
		break;
	case RADAR_TRACE_CYAN:
		if (bright)
			c = 0x00FFFFFF;
		else
			c = 0x007F7FFF;
		break;
	default:
		c = color;
		break;
	};
	return c;
}

const char* gRadarTexNames[] = {
	"radar00", "radar01", "radar02", "radar03", "radar04", "radar05", "radar06", "radar07",
	"radar08", "radar09", "radar10", "radar11", "radar12", "radar13", "radar14", "radar15",
	"radar16", "radar17", "radar18", "radar19", "radar20", "radar21", "radar22", "radar23",
	"radar24", "radar25", "radar26", "radar27", "radar28", "radar29", "radar30", "radar31",
	"radar32", "radar33", "radar34", "radar35", "radar36", "radar37", "radar38", "radar39",
	"radar40", "radar41", "radar42", "radar43", "radar44", "radar45", "radar46", "radar47",
	"radar48", "radar49", "radar50", "radar51", "radar52", "radar53", "radar54", "radar55",
	"radar56", "radar57", "radar58", "radar59", "radar60", "radar61", "radar62", "radar63",
};

void
CRadar::Initialise()
{
	for (int i = 0; i < NUMRADARBLIPS; i++) {
		ms_RadarTrace[i].m_BlipIndex = 1;
		SetRadarMarkerState(i, false);
		ms_RadarTrace[i].m_bInUse = false;
		ms_RadarTrace[i].m_eBlipType = BLIP_NONE;
		ms_RadarTrace[i].m_eBlipDisplay = BLIP_DISPLAY_NEITHER;
		ms_RadarTrace[i].m_eRadarSprite = RADAR_SPRITE_NONE;
	}

	m_radarRange = 350.0f;
	for (int i = 0; i < 64; i++) 
		gRadarTxdIds[i] = CTxdStore::FindTxdSlot(gRadarTexNames[i]);
}

float CRadar::LimitRadarPoint(CVector2D &point)
{
	float dist, invdist;

	dist = point.Magnitude();
#ifdef MENU_MAP
	if (CMenuManager::bMenuMapActive)
		return dist;
#endif
	if (dist > 1.0f) {
		invdist = 1.0f / dist;
		point.x *= invdist;
		point.y *= invdist;
	}
	return dist;
}

void CRadar::LoadAllRadarBlips(uint8 *buf, uint32 size)
{
	Initialise();
INITSAVEBUF
	CheckSaveHeader(buf, 'R', 'D', 'R', '\0', size - SAVE_HEADER_SIZE);

	for (int i = 0; i < NUMRADARBLIPS; i++)
		ms_RadarTrace[i] = ReadSaveBuf<sRadarTrace>(buf);

VALIDATESAVEBUF(size);
}

void
CRadar::LoadTextures()
{
	CTxdStore::PushCurrentTxd();
	CTxdStore::SetCurrentTxd(CTxdStore::FindTxdSlot("hud"));
	AsukaSprite.SetTexture("radar_asuka");
	BombSprite.SetTexture("radar_bomb");
	CatSprite.SetTexture("radar_cat");
	CentreSprite.SetTexture("radar_centre");
	CopcarSprite.SetTexture("radar_copcar");
	DonSprite.SetTexture("radar_don");
	EightSprite.SetTexture("radar_eight");
	ElSprite.SetTexture("radar_el");
	IceSprite.SetTexture("radar_ice");
	JoeySprite.SetTexture("radar_joey");
	KenjiSprite.SetTexture("radar_kenji");
	LizSprite.SetTexture("radar_liz");
	LuigiSprite.SetTexture("radar_luigi");
	NorthSprite.SetTexture("radar_north");
	RaySprite.SetTexture("radar_ray");
	SalSprite.SetTexture("radar_sal");
	SaveSprite.SetTexture("radar_save");
	SpraySprite.SetTexture("radar_spray");
	TonySprite.SetTexture("radar_tony");
	WeaponSprite.SetTexture("radar_weapon");
	CTxdStore::PopCurrentTxd();
}

void CRadar::RemoveRadarSections()
{
	for (int i = 0; i < 8; i++)
		for (int j = 0; j < 8; j++)
			RemoveMapSection(i, j);
}

void CRadar::SaveAllRadarBlips(uint8 *buf, uint32 *size)
{
	*size = SAVE_HEADER_SIZE + sizeof(ms_RadarTrace);
INITSAVEBUF
	WriteSaveHeader(buf, 'R', 'D', 'R', '\0', *size - SAVE_HEADER_SIZE);

#ifdef MENU_MAP
	if (TargetMarkerId != -1) {
		ClearBlip(TargetMarkerId);
		TargetMarkerId = -1;
	}
#endif

	for (int i = 0; i < NUMRADARBLIPS; i++)
		WriteSaveBuf(buf, ms_RadarTrace[i]);

VALIDATESAVEBUF(*size);
}

void CRadar::SetBlipSprite(int32 i, int32 icon)
{
	int index = CRadar::GetActualBlipArrayIndex(i);
	if (index != -1) {
		ms_RadarTrace[index].m_eRadarSprite = icon;
	}
}

int CRadar::SetCoordBlip(eBlipType type, CVector pos, int32 color, eBlipDisplay display)
{
	int nextBlip;
	for (nextBlip = 0; nextBlip < NUMRADARBLIPS; nextBlip++) {
		if (!ms_RadarTrace[nextBlip].m_bInUse)
			break;
	}
#ifdef FIX_BUGS
	if (nextBlip == NUMRADARBLIPS)
		return -1;
#endif
	ms_RadarTrace[nextBlip].m_eBlipType = type;
	ms_RadarTrace[nextBlip].m_nColor = color;
	ms_RadarTrace[nextBlip].m_bDim = 1;
	ms_RadarTrace[nextBlip].m_bInUse = 1;
	ms_RadarTrace[nextBlip].m_Radius = 1.0f;
	ms_RadarTrace[nextBlip].m_vec2DPos = pos;
	ms_RadarTrace[nextBlip].m_vecPos = pos;
	ms_RadarTrace[nextBlip].m_nEntityHandle = 0;
	ms_RadarTrace[nextBlip].m_wScale = 1;
	ms_RadarTrace[nextBlip].m_eBlipDisplay = display;
	ms_RadarTrace[nextBlip].m_eRadarSprite = RADAR_SPRITE_NONE;
	return CRadar::GetNewUniqueBlipIndex(nextBlip);
}

int CRadar::SetEntityBlip(eBlipType type, int32 handle, int32 color, eBlipDisplay display)
{
	int nextBlip;
	for (nextBlip = 0; nextBlip < NUMRADARBLIPS; nextBlip++) {
		if (!ms_RadarTrace[nextBlip].m_bInUse)
			break;
	}
#ifdef FIX_BUGS
	if (nextBlip == NUMRADARBLIPS)
		return -1;
#endif
	ms_RadarTrace[nextBlip].m_eBlipType = type;
	ms_RadarTrace[nextBlip].m_nColor = color;
	ms_RadarTrace[nextBlip].m_bDim = 1;
	ms_RadarTrace[nextBlip].m_bInUse = 1;
	ms_RadarTrace[nextBlip].m_Radius = 1.0f;
	ms_RadarTrace[nextBlip].m_nEntityHandle = handle;
	ms_RadarTrace[nextBlip].m_wScale = 1;
	ms_RadarTrace[nextBlip].m_eBlipDisplay = display;
	ms_RadarTrace[nextBlip].m_eRadarSprite = RADAR_SPRITE_NONE;
	return GetNewUniqueBlipIndex(nextBlip);
}

void CRadar::SetRadarMarkerState(int32 counter, bool flag)
{
	CEntity *e;
	switch (ms_RadarTrace[counter].m_eBlipType) {
	case BLIP_CAR:
		e = CPools::GetVehiclePool()->GetAt(ms_RadarTrace[counter].m_nEntityHandle);
		break;
	case BLIP_CHAR:
		e = CPools::GetPedPool()->GetAt(ms_RadarTrace[counter].m_nEntityHandle);
		break;
	case BLIP_OBJECT:
		e = CPools::GetObjectPool()->GetAt(ms_RadarTrace[counter].m_nEntityHandle);
		break;
	default:
		return;
	}

	if (e)
		e->bHasBlip = flag;
}

void CRadar::ShowRadarMarker(CVector pos, uint32 color, float radius) {
	float f1 = radius * 1.4f;
	float f2 = radius * 0.5f;
	CVector p1, p2;

	p1 = pos + TheCamera.GetUp()*f1;
	p2 = pos + TheCamera.GetUp()*f2;
	CTheScripts::ScriptDebugLine3D(p1.x, p1.y, p1.z, p2.x, p2.y, p2.z, color, color);

	p1 = pos - TheCamera.GetUp()*f1;
	p2 = pos - TheCamera.GetUp()*f2;
	CTheScripts::ScriptDebugLine3D(p1.x, p1.y, p1.z, p2.x, p2.y, p2.z, color, color);

	p1 = pos + TheCamera.GetRight()*f1;
	p2 = pos + TheCamera.GetRight()*f2;
	CTheScripts::ScriptDebugLine3D(p1.x, p1.y, p1.z, p2.x, p2.y, p2.z, color, color);

	p1 = pos - TheCamera.GetRight()*f1;
	p2 = pos - TheCamera.GetRight()*f2;
	CTheScripts::ScriptDebugLine3D(p1.x, p1.y, p1.z, p2.x, p2.y, p2.z, color, color);
}

void CRadar::ShowRadarTrace(float x, float y, uint32 size, uint8 red, uint8 green, uint8 blue, uint8 alpha)
{
	if (!CHud::m_Wants_To_Draw_Hud || TheCamera.m_WideScreenOn)
		return;

	CSprite2d::DrawRect(CRect(x - SCREEN_SCALE_X(size + 1.0f), y - SCREEN_SCALE_Y(size + 1.0f), SCREEN_SCALE_X(size + 1.0f) + x, SCREEN_SCALE_Y(size + 1.0f) + y), CRGBA(0, 0, 0, alpha));
	CSprite2d::DrawRect(CRect(x - SCREEN_SCALE_X(size), y - SCREEN_SCALE_Y(size), SCREEN_SCALE_X(size) + x, SCREEN_SCALE_Y(size) + y), CRGBA(red, green, blue, alpha));
}

void CRadar::ShowRadarTraceWithHeight(float x, float y, uint32 size, uint8 red, uint8 green, uint8 blue, uint8 alpha, uint8 mode)
{
	if (!CHud::m_Wants_To_Draw_Hud || TheCamera.m_WideScreenOn)
		return;

	switch (mode)
	{
	case BLIP_MODE_TRIANGULAR_UP:
		// size++; // VC does size + 1 for triangles
		CSprite2d::Draw2DPolygon(x + SCREEN_SCALE_X(size + 3.0f), y + SCREEN_SCALE_Y(size + 2.0f), x - (SCREEN_SCALE_X(size + 3.0f)), y + SCREEN_SCALE_Y(size + 2.0f), x, y - (SCREEN_SCALE_Y(size + 3.0f)), x, y - (SCREEN_SCALE_Y(size + 3.0f)), CRGBA(0, 0, 0, alpha));
		CSprite2d::Draw2DPolygon(x + SCREEN_SCALE_X(size + 1.0f), y + SCREEN_SCALE_Y(size + 1.0f), x - (SCREEN_SCALE_X(size + 1.0f)), y + SCREEN_SCALE_Y(size + 1.0f), x, y - (SCREEN_SCALE_Y(size + 1.0f)), x, y - (SCREEN_SCALE_Y(size + 1.0f)), CRGBA(red, green, blue, alpha));
		break;
	case BLIP_MODE_TRIANGULAR_DOWN:
		// size++; // VC does size + 1 for triangles
		CSprite2d::Draw2DPolygon(x, y + SCREEN_SCALE_Y(size + 2.0f), x, y + SCREEN_SCALE_Y(size + 3.0f), x + SCREEN_SCALE_X(size + 3.0f), y - (SCREEN_SCALE_Y(size + 2.0f)), x - (SCREEN_SCALE_X(size + 3.0f)), y - (SCREEN_SCALE_Y(size + 2.0f)), CRGBA(0, 0, 0, alpha));
		CSprite2d::Draw2DPolygon(x, y + SCREEN_SCALE_Y(size + 1.0f), x, y + SCREEN_SCALE_Y(size + 1.0f), x + SCREEN_SCALE_X(size + 1.0f), y - (SCREEN_SCALE_Y(size + 1.0f)), x - (SCREEN_SCALE_X(size + 1.0f)), y - (SCREEN_SCALE_Y(size + 1.0f)), CRGBA(red, green, blue, alpha));
		break;
	case BLIP_MODE_SQUARE:
		CSprite2d::DrawRect(CRect(x - SCREEN_SCALE_X(size + 1.0f), y - SCREEN_SCALE_Y(size + 1.0f), SCREEN_SCALE_X(size + 1.0f) + x, SCREEN_SCALE_Y(size + 1.0f) + y), CRGBA(0, 0, 0, alpha));
		CSprite2d::DrawRect(CRect(x - SCREEN_SCALE_X(size), y - SCREEN_SCALE_Y(size), SCREEN_SCALE_X(size) + x, SCREEN_SCALE_Y(size) + y), CRGBA(red, green, blue, alpha));
		break;
	}
}

void CRadar::Shutdown()
{
	AsukaSprite.Delete();
	BombSprite.Delete();
	CatSprite.Delete();
	CentreSprite.Delete();
	CopcarSprite.Delete();
	DonSprite.Delete();
	EightSprite.Delete();
	ElSprite.Delete();
	IceSprite.Delete();
	JoeySprite.Delete();
	KenjiSprite.Delete();
	LizSprite.Delete();
	LuigiSprite.Delete();
	NorthSprite.Delete();
	RaySprite.Delete();
	SalSprite.Delete();
	SaveSprite.Delete();
	SpraySprite.Delete();
	TonySprite.Delete();
	WeaponSprite.Delete();
	RemoveRadarSections();
}

void CRadar::StreamRadarSections(const CVector &posn)
{
	StreamRadarSections(Floor((2000.0f + posn.x) / 500.0f), Ceil(7.0f - (2000.0f + posn.y) / 500.0f));
}

void CRadar::StreamRadarSections(int32 x, int32 y)
{
	for (int i = 0; i < RADAR_NUM_TILES; ++i) {
		for (int j = 0; j < RADAR_NUM_TILES; ++j) {
			if ((i >= x - 1 && i <= x + 1) && (j >= y - 1 && j <= y + 1))
				RequestMapSection(i, j);
			else
				RemoveMapSection(i, j);
		};
	};
}

void CRadar::TransformRealWorldToTexCoordSpace(CVector2D &out, const CVector2D &in, int32 x, int32 y)
{
	out.x = in.x - (x * RADAR_TILE_SIZE + RADAR_MIN_X);
	out.y = -(in.y - ((RADAR_NUM_TILES - y) * RADAR_TILE_SIZE + RADAR_MIN_Y));
	out.x /= RADAR_TILE_SIZE;
	out.y /= RADAR_TILE_SIZE;
}

void CRadar::TransformRadarPointToRealWorldSpace(CVector2D &out, const CVector2D &in)
{
	float s, c;
#if 1
	s = -cachedSin;
	c = cachedCos;
#else
	// Original code

	s = -Sin(TheCamera.GetForward().Heading());
	c = Cos(TheCamera.GetForward().Heading());

	if (TheCamera.Cams[TheCamera.ActiveCam].Mode == CCam::MODE_TOPDOWN || TheCamera.Cams[TheCamera.ActiveCam].Mode == CCam::MODE_TOP_DOWN_PED) {
		s = 0.0f;
		c = 1.0f;
	}
	else if (TheCamera.GetLookDirection() != LOOKING_FORWARD) {
		CVector forward;

		if (TheCamera.Cams[TheCamera.ActiveCam].Mode == CCam::MODE_1STPERSON) {
			forward = TheCamera.Cams[TheCamera.ActiveCam].CamTargetEntity->GetForward();
			forward.Normalise();	// a bit useless...
		}
		else
			forward = TheCamera.Cams[TheCamera.ActiveCam].CamTargetEntity->GetPosition() - TheCamera.Cams[TheCamera.ActiveCam].SourceBeforeLookBehind;

		s = -Sin(forward.Heading());
		c = Cos(forward.Heading());
	}
#endif

	out.x = s * in.y + c * in.x;
	out.y = c * in.y - s * in.x;

	out = out * m_radarRange + vec2DRadarOrigin;
}

// Radar space goes from -1.0 to 1.0 in x and y, top right is (1.0, 1.0)
void CRadar::TransformRadarPointToScreenSpace(CVector2D &out, const CVector2D &in)
{
#ifdef MENU_MAP
	if (CMenuManager::bMenuMapActive) {
		// fMapSize is actually half map size. Radar range is 1000, so if x is -2000, in.x + 2.0f is 0.
		out.x = (CMenuManager::fMapCenterX - CMenuManager::fMapSize) + (in.x + 2.0f) * CMenuManager::fMapSize * 2.0f / 4.0f;
		out.y = (CMenuManager::fMapCenterY - CMenuManager::fMapSize) + (2.0f - in.y) * CMenuManager::fMapSize * 2.0f / 4.0f;
	} else
#endif
	{
#ifdef FIX_BUGS
		out.x = (in.x + 1.0f) * 0.5f * SCREEN_SCALE_X(RADAR_WIDTH) + SCREEN_SCALE_X(RADAR_LEFT);
#else
		out.x = (in.x + 1.0f) * 0.5f * SCREEN_SCALE_X(RADAR_WIDTH) + RADAR_LEFT;
#endif
		out.y = (1.0f - in.y) * 0.5f * SCREEN_SCALE_Y(RADAR_HEIGHT) + SCREEN_SCALE_FROM_BOTTOM(RADAR_BOTTOM + RADAR_HEIGHT);
	}
}

void CRadar::TransformRealWorldPointToRadarSpace(CVector2D &out, const CVector2D &in)
{
	float s, c;
#if 1
	s = cachedSin;
	c = cachedCos;
#else
	// Original code

	float s, c;
	if (TheCamera.Cams[TheCamera.ActiveCam].Mode == CCam::MODE_TOPDOWN || TheCamera.Cams[TheCamera.ActiveCam].Mode == CCam::MODE_TOP_DOWN_PED) {
		s = 0.0f;
		c = 1.0f;
	}
	else if (TheCamera.GetLookDirection() == LOOKING_FORWARD) {
		s = Sin(TheCamera.GetForward().Heading());
		c = Cos(TheCamera.GetForward().Heading());
	}
	else {
		CVector forward;

		if (TheCamera.Cams[TheCamera.ActiveCam].Mode == CCam::MODE_1STPERSON) {
			forward = TheCamera.Cams[TheCamera.ActiveCam].CamTargetEntity->GetForward();
			forward.Normalise();	// a bit useless...
		}
		else
			forward = TheCamera.Cams[TheCamera.ActiveCam].CamTargetEntity->GetPosition() - TheCamera.Cams[TheCamera.ActiveCam].SourceBeforeLookBehind;

		s = Sin(forward.Heading());
		c = Cos(forward.Heading());
	}
#endif

	float x = (in.x - vec2DRadarOrigin.x) * (1.0f / m_radarRange);
	float y = (in.y - vec2DRadarOrigin.y) * (1.0f / m_radarRange);

	out.x = s * y + c * x;
	out.y = c * y - s * x;
}

void
CRadar::CalculateCachedSinCos()
{
	if (TheCamera.Cams[TheCamera.ActiveCam].Mode == CCam::MODE_TOPDOWN || TheCamera.Cams[TheCamera.ActiveCam].Mode == CCam::MODE_TOP_DOWN_PED
#ifdef MENU_MAP
		|| CMenuManager::bMenuMapActive
#endif
		) {
		cachedSin = 0.0f;
		cachedCos = 1.0f;
	} else if (TheCamera.GetLookDirection() == LOOKING_FORWARD) {
		cachedSin = Sin(TheCamera.GetForward().Heading());
		cachedCos = Cos(TheCamera.GetForward().Heading());
	} else {
		CVector forward;

		if (TheCamera.Cams[TheCamera.ActiveCam].Mode == CCam::MODE_1STPERSON) {
			forward = TheCamera.Cams[TheCamera.ActiveCam].CamTargetEntity->GetForward();
			forward.Normalise();	// a bit useless...
		}
		else
			forward = TheCamera.Cams[TheCamera.ActiveCam].CamTargetEntity->GetPosition() - TheCamera.Cams[TheCamera.ActiveCam].SourceBeforeLookBehind;

		cachedSin = Sin(forward.Heading());
		cachedCos = Cos(forward.Heading());
	}
}

#ifdef MENU_MAP
void
CRadar::InitFrontEndMap()
{
	CalculateCachedSinCos();
	vec2DRadarOrigin.x = 0.0f;
	vec2DRadarOrigin.y = 0.0f;
	m_radarRange = 1000.0f; // doesn't mean anything, just affects the calculation in TransformRadarPointToScreenSpace
}

void
CRadar::DrawYouAreHereSprite(float x, float y)
{
	static uint32 lastChange = 0;
	static bool show = true;

	if (show) {
		if (CTimer::GetTimeInMillisecondsPauseMode() - lastChange > 500) {
			lastChange = CTimer::GetTimeInMillisecondsPauseMode();
			show = !show;
		}
	} else {
		if (CTimer::GetTimeInMillisecondsPauseMode() - lastChange > 200) {
			lastChange = CTimer::GetTimeInMillisecondsPauseMode();
			show = !show;
		}
	}

	if (show) {
		float left = x - SCREEN_SCALE_X(12.0f);
		float top = y;
		float right = SCREEN_SCALE_X(12.0) + x;
		float bottom = y - SCREEN_SCALE_Y(24.0f);
		CentreSprite.Draw(CRect(left, top, right, bottom), CRGBA(255, 255, 255, 255));
	}
}

void
CRadar::ToggleTargetMarker(float x, float y)
{
	if (TargetMarkerId == -1) {
		int nextBlip;
		for (nextBlip = 0; nextBlip < NUMRADARBLIPS; nextBlip++) {
			if (!ms_RadarTrace[nextBlip].m_bInUse)
				break;
		}
#ifdef FIX_BUGS
		if (nextBlip == NUMRADARBLIPS)
			return;
#endif
		ms_RadarTrace[nextBlip].m_eBlipType = BLIP_COORD;
		ms_RadarTrace[nextBlip].m_nColor = RADAR_TRACE_GRAY;
		ms_RadarTrace[nextBlip].m_bDim = 0;
		ms_RadarTrace[nextBlip].m_bInUse = 1;
		ms_RadarTrace[nextBlip].m_Radius = 1.0f;
		CVector pos(x, y, CWorld::FindGroundZForCoord(x,y));
		TargetMarkerPos = pos;
		ms_RadarTrace[nextBlip].m_vec2DPos = pos;
		ms_RadarTrace[nextBlip].m_vecPos = pos;
		ms_RadarTrace[nextBlip].m_nEntityHandle = 0;
		ms_RadarTrace[nextBlip].m_wScale = 5;
		ms_RadarTrace[nextBlip].m_eBlipDisplay = BLIP_DISPLAY_BLIP_ONLY;
		ms_RadarTrace[nextBlip].m_eRadarSprite = RADAR_SPRITE_NONE;
		TargetMarkerId = CRadar::GetNewUniqueBlipIndex(nextBlip);
	} else {
		ClearBlip(TargetMarkerId);
		TargetMarkerId = -1;
	}
}
#endif

