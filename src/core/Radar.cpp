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
#include "Font.h"
#include "SaveBuf.h"

float CRadar::m_radarRange;
sRadarTrace CRadar::ms_RadarTrace[NUMRADARBLIPS];
CVector2D vec2DRadarOrigin;
int32 gRadarTxdIds[64];

CSprite2d CRadar::CentreSprite;
CSprite2d CRadar::MapHereSprite;
CSprite2d CRadar::NorthSprite;
CSprite2d CRadar::AverySprite;
CSprite2d CRadar::BikerSprite;
CSprite2d CRadar::CortezSprite;
CSprite2d CRadar::DiazSprite;
CSprite2d CRadar::KentSprite;
CSprite2d CRadar::LawyerSprite;
CSprite2d CRadar::PhilSprite;
CSprite2d CRadar::BikersSprite;
CSprite2d CRadar::BoatyardSprite;
CSprite2d CRadar::MalibuClubSprite;
CSprite2d CRadar::CubansSprite;
CSprite2d CRadar::FilmSprite;
CSprite2d CRadar::GunSprite;
CSprite2d CRadar::HaitiansSprite;
CSprite2d CRadar::HardwareSprite;
CSprite2d CRadar::SaveHouseSprite;
CSprite2d CRadar::StripSprite;
CSprite2d CRadar::IceSprite;
CSprite2d CRadar::KCabsSprite;
CSprite2d CRadar::LovefistSprite;
CSprite2d CRadar::PrintworksSprite;
CSprite2d CRadar::PropertySprite;
CSprite2d CRadar::SunYardSprite;
CSprite2d CRadar::SpraySprite;
CSprite2d CRadar::TShirtSprite;
CSprite2d CRadar::TommySprite;
CSprite2d CRadar::PhoneSprite;
CSprite2d CRadar::RadioWildstyleSprite;
CSprite2d CRadar::RadioFlashSprite;
CSprite2d CRadar::RadioKChatSprite;
CSprite2d CRadar::RadioFeverSprite;
CSprite2d CRadar::RadioVRockSprite;
CSprite2d CRadar::RadioVCPRSprite;
CSprite2d CRadar::RadioEspantosoSprite;
CSprite2d CRadar::RadioEmotionSprite;
CSprite2d CRadar::RadioWaveSprite;

CSprite2d *CRadar::RadarSprites[RADAR_SPRITE_COUNT] = { 
	nil,
	&CentreSprite,
	&MapHereSprite,
	&NorthSprite,
	&AverySprite,
	&BikerSprite,
	&CortezSprite,
	&DiazSprite,
	&KentSprite,
	&LawyerSprite,
	&PhilSprite,
	&BikersSprite,
	&BoatyardSprite,
	&MalibuClubSprite,
	&CubansSprite,
	&FilmSprite,
	&GunSprite,
	&HaitiansSprite,
	&HardwareSprite,
	&SaveHouseSprite,
	&StripSprite,
	&IceSprite,
	&KCabsSprite,
	&LovefistSprite,
	&PrintworksSprite,
	&PropertySprite,
	&SunYardSprite,
	&SpraySprite,
	&TShirtSprite,
	&TommySprite,
	&PhoneSprite,
	&RadioWildstyleSprite,
	&RadioFlashSprite,
	&RadioKChatSprite,
	&RadioFeverSprite,
	&RadioVRockSprite,
	&RadioVCPRSprite,
	&RadioEspantosoSprite,
	&RadioEmotionSprite,
	&RadioWaveSprite
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

CRGBA CRadar::ArrowBlipColour1;
CRGBA CRadar::ArrowBlipColour2;
int16 CRadar::MapLegendCounter;
int16 CRadar::MapLegendList[NUM_MAP_LEGENDS];
#ifdef MAP_ENHANCEMENTS
int CRadar::TargetMarkerId = -1;
CVector CRadar::TargetMarkerPos;
#endif

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

uint8 CRadar::CalculateBlipAlpha(float dist)
{
	if (FrontEndMenuManager.m_bMenuMapActive)
		return 255;

	if (dist <= 1.0f)
		return 255;

	if (dist <= 10.0f)
		return (128.0f * ((dist - 1.0f) / 9.0f)) + ((1.0f - (dist - 1.0f) / 9.0f) * 255.0f);

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
		ms_RadarTrace[index].m_eBlipType = BLIP_NONE;
		ms_RadarTrace[index].m_eBlipDisplay = BLIP_DISPLAY_NEITHER;
		ms_RadarTrace[index].m_eRadarSprite = RADAR_SPRITE_NONE;
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
#ifdef THIS_IS_STUPID

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
#else

int
ClipPolyPlane(const CVector2D *in, int nin, CVector2D *out, CVector *plane)
{
	int j;
	int nout;
	int x1, x2;
	float d1, d2, t;

	nout = 0;
	for(j = 0; j < nin; j++){
		x1 = j;
		x2 = (j+1) % nin;

		d1 = plane->x*in[x1].x + plane->y*in[x1].y + plane->z;
		d2 = plane->x*in[x2].x + plane->y*in[x2].y + plane->z;
		if(d1*d2 < 0.0f){
			t = d1/(d1 - d2);
			out[nout++] = in[x1]*(1.0f-t) + in[x2]*t;
		}
		if(d2 >= 0.0f)
			out[nout++] = in[x2];
	}
	return nout;
}

int CRadar::ClipRadarPoly(CVector2D *poly, const CVector2D *rect)
{
	CVector planes[4] = {
		CVector(-1.0f, 0.0f, 1.0f),
		CVector( 1.0f, 0.0f, 1.0f),
		CVector(0.0f, -1.0f, 1.0f),
		CVector(0.0f,  1.0f, 1.0f)
	};
	CVector2D tmp[8];
	int n;
	if(n = ClipPolyPlane(rect, 4, tmp, &planes[0]), n == 0) return 0;
	if(n = ClipPolyPlane(tmp, n, poly, &planes[1]), n == 0) return 0;
	if(n = ClipPolyPlane(poly, n, tmp, &planes[2]), n == 0) return 0;
	if(n = ClipPolyPlane(tmp, n, poly, &planes[3]), n == 0) return 0;
	return n;
}
#endif

bool CRadar::DisplayThisBlip(int32 counter)
{
	switch (ms_RadarTrace[counter].m_eRadarSprite) {
	case RADAR_SPRITE_SPRAY:
	case RADAR_SPRITE_GUN:
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
					C3dMarkers::PlaceMarker(i | (ms_RadarTrace[i].m_BlipIndex << 16), MARKERTYPE_ARROW, pos, 2.5f, CARBLIP_MARKER_COLOR_R, CARBLIP_MARKER_COLOR_G, CARBLIP_MARKER_COLOR_B, CARBLIP_MARKER_COLOR_A, 1024, 0.2f, 5);
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
					C3dMarkers::PlaceMarker(i | (ms_RadarTrace[i].m_BlipIndex << 16), MARKERTYPE_ARROW, pos, 1.5f, CHARBLIP_MARKER_COLOR_R, CHARBLIP_MARKER_COLOR_G, CHARBLIP_MARKER_COLOR_B, CHARBLIP_MARKER_COLOR_A, 1024, 0.2f, 5);
				}
				break;
			}
			case BLIP_OBJECT:
			{
				CEntity *entity = CPools::GetObjectPool()->GetAt(ms_RadarTrace[i].m_nEntityHandle);
				if (ms_RadarTrace[i].m_eBlipDisplay == BLIP_DISPLAY_BOTH || ms_RadarTrace[i].m_eBlipDisplay == BLIP_DISPLAY_MARKER_ONLY) {
					CVector pos = entity->GetPosition();
					pos.z += CModelInfo::GetModelInfo(entity->GetModelIndex())->GetColModel()->boundingBox.max.z + 1.0f + 1.0f;
					C3dMarkers::PlaceMarker(i | (ms_RadarTrace[i].m_BlipIndex << 16), MARKERTYPE_ARROW, pos, 1.0f, OBJECTBLIP_MARKER_COLOR_R, OBJECTBLIP_MARKER_COLOR_G, OBJECTBLIP_MARKER_COLOR_B, OBJECTBLIP_MARKER_COLOR_A, 1024, 0.2f, 5);
				}
				break;
			}
			case BLIP_COORD:
				break;
			case BLIP_CONTACT_POINT:
				if (!CTheScripts::IsPlayerOnAMission()) {
					if (ms_RadarTrace[i].m_eBlipDisplay == BLIP_DISPLAY_BOTH || ms_RadarTrace[i].m_eBlipDisplay == BLIP_DISPLAY_MARKER_ONLY)
						C3dMarkers::PlaceMarkerSet(i | (ms_RadarTrace[i].m_BlipIndex << 16), MARKERTYPE_CYLINDER, ms_RadarTrace[i].m_vecPos, 2.0f, COORDBLIP_MARKER_COLOR_R, COORDBLIP_MARKER_COLOR_G, COORDBLIP_MARKER_COLOR_B, COORDBLIP_MARKER_COLOR_A, 2048, 0.2f, 0);
				}
				break;
			}
		}
	}
}

void CRadar::DrawBlips()
{
	if (!TheCamera.m_WideScreenOn && CHud::m_Wants_To_Draw_Hud) {
#ifdef SECUROM
		extern uint8 roadBlocksPirateCheck;
		if (roadBlocksPirateCheck == 2) return;
#endif
		RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)FALSE);
		RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*)FALSE);
		RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)TRUE);
		RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDSRCALPHA);
		RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDINVSRCALPHA);
		RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void*)FALSE);

		CVector2D out;
		CVector2D in = CVector2D(0.0f, 0.0f);
		TransformRadarPointToScreenSpace(out, in);

		if (!FrontEndMenuManager.m_bMenuMapActive) {
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
		}

		for(int blipId = 0; blipId < NUMRADARBLIPS; blipId++) {
			if (!ms_RadarTrace[blipId].m_bInUse)
				continue;

			switch (ms_RadarTrace[blipId].m_eBlipType) {
				case BLIP_CAR:
				case BLIP_CHAR:
				case BLIP_OBJECT:
					if (ms_RadarTrace[blipId].m_eRadarSprite == RADAR_SPRITE_PROPERTY
						&& (!CTheScripts::bPlayerIsInTheStatium || !FrontEndMenuManager.m_bMenuMapActive))
						DrawEntityBlip(blipId);

					break;
				case BLIP_COORD:
				case BLIP_CONTACT_POINT:
					if (ms_RadarTrace[blipId].m_eRadarSprite == RADAR_SPRITE_PHONE
						&& (!CTheScripts::bPlayerIsInTheStatium || !FrontEndMenuManager.m_bMenuMapActive))
						DrawCoordBlip(blipId);
					
					break;
				default:
					break;
			}
		}

		// New in VC: Always draw Hardware/gun/pay'n spray/save blips
		for(int blipId = 0; blipId < NUMRADARBLIPS; blipId++) {
			if (!ms_RadarTrace[blipId].m_bInUse)
				continue;

			if (ms_RadarTrace[blipId].m_eRadarSprite != RADAR_SPRITE_SAVE && ms_RadarTrace[blipId].m_eRadarSprite != RADAR_SPRITE_HARDWARE
					&& ms_RadarTrace[blipId].m_eRadarSprite != RADAR_SPRITE_SPRAY && ms_RadarTrace[blipId].m_eRadarSprite != RADAR_SPRITE_GUN)
				continue;

			switch (ms_RadarTrace[blipId].m_eBlipType) {
				case BLIP_CAR:
				case BLIP_CHAR:
				case BLIP_OBJECT:
					if (!CTheScripts::bPlayerIsInTheStatium || !FrontEndMenuManager.m_bMenuMapActive)
						DrawEntityBlip(blipId);

					break;
				case BLIP_COORD:
				case BLIP_CONTACT_POINT:
					if (!CTheScripts::bPlayerIsInTheStatium || !FrontEndMenuManager.m_bMenuMapActive)
						DrawCoordBlip(blipId);
					
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
					if (ms_RadarTrace[blipId].m_eRadarSprite != RADAR_SPRITE_SAVE && ms_RadarTrace[blipId].m_eRadarSprite != RADAR_SPRITE_HARDWARE
						&& ms_RadarTrace[blipId].m_eRadarSprite != RADAR_SPRITE_SPRAY && ms_RadarTrace[blipId].m_eRadarSprite != RADAR_SPRITE_PROPERTY
						&& ms_RadarTrace[blipId].m_eRadarSprite != RADAR_SPRITE_GUN
						&& (!CTheScripts::bPlayerIsInTheStatium || !FrontEndMenuManager.m_bMenuMapActive))

						DrawEntityBlip(blipId);
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
					if (ms_RadarTrace[blipId].m_eRadarSprite != RADAR_SPRITE_SAVE && ms_RadarTrace[blipId].m_eRadarSprite != RADAR_SPRITE_HARDWARE
						&& ms_RadarTrace[blipId].m_eRadarSprite != RADAR_SPRITE_SPRAY && ms_RadarTrace[blipId].m_eRadarSprite != RADAR_SPRITE_PROPERTY
						&& ms_RadarTrace[blipId].m_eRadarSprite != RADAR_SPRITE_GUN && ms_RadarTrace[blipId].m_eRadarSprite != RADAR_SPRITE_PHONE
						&& (!CTheScripts::bPlayerIsInTheStatium || !FrontEndMenuManager.m_bMenuMapActive))

						DrawCoordBlip(blipId);
					break;
				default:
					break;
			}
		}
		if (FrontEndMenuManager.m_bMenuMapActive) {
			CVector2D in, out;
			if (!CTheScripts::bPlayerIsInTheStatium)
				TransformRealWorldPointToRadarSpace(in, FindPlayerCentreOfWorld_NoSniperShift());
			else
				TransformRealWorldPointToRadarSpace(in, CVector2D(-1302.5f, 1332.8f));

			LimitRadarPoint(in);
			TransformRadarPointToScreenSpace(out, in);
			DrawYouAreHereSprite(out.x, out.y);
		}
	}
}

void CRadar::DrawMap()
{
	if (!TheCamera.m_WideScreenOn && CHud::m_Wants_To_Draw_Hud) {
		CalculateCachedSinCos();
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
		if (FrontEndMenuManager.m_PrefsRadarMode != 1)
			DrawRadarMap();
	}
}

void CRadar::DrawRadarMap()
{
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

	if (!CTheScripts::bPlayerIsInTheStatium &&
		(!(txd = CTxdStore::GetSlot(gRadarTxdIds[x + RADAR_NUM_TILES * y])->texDict) || !(texture = GetFirstTexture(txd))))
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

	if (CTheScripts::bPlayerIsInTheStatium) {
		RwRenderStateSet(rwRENDERSTATETEXTURERASTER, nil);
		CSprite2d::SetVertices(numVertices, (float*)screenPoly, (float*)texCoords, CRGBA(204, 204, 204, 255));
	} else {
		RwRenderStateSet(rwRENDERSTATETEXTURERASTER, RwTextureGetRaster(texture));
		CSprite2d::SetVertices(numVertices, (float*)screenPoly, (float*)texCoords, CRGBA(255, 255, 255, 255));
	}

	// check done above now
//	if(numVertices > 2)
	RwIm2DRenderPrimitive(rwPRIMTYPETRIFAN, CSprite2d::GetVertices(), numVertices);
}

void CRadar::DrawRadarSprite(uint16 sprite, float x, float y, uint8 alpha)
{
	RadarSprites[sprite]->Draw(CRect(x - SCREEN_SCALE_X(8.0f), y - SCREEN_SCALE_Y(8.0f), x + SCREEN_SCALE_X(8.0f), y + SCREEN_SCALE_Y(8.0f)), CRGBA(255, 255, 255, alpha));

	if (FrontEndMenuManager.m_bMenuMapActive) {
		bool alreadyThere = false;
		for (int i = 0; i < NUM_MAP_LEGENDS; i++) {
			if (MapLegendList[i] == sprite)
				alreadyThere = true;
		}
		if (!alreadyThere) {
			MapLegendList[MapLegendCounter] = sprite;
			MapLegendCounter++;
		}
	}
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
		ms_RadarTrace[i].m_bShortRange = false;
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

	if (FrontEndMenuManager.m_bMenuMapActive)
		return dist;

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

	for (int i = 0; i < NUMRADARBLIPS; i++) {
		ReadSaveBuf(&ms_RadarTrace[i].m_nColor, buf);
		ReadSaveBuf(&ms_RadarTrace[i].m_Radius, buf);
		ReadSaveBuf(&ms_RadarTrace[i].m_eBlipType, buf);
		ReadSaveBuf(&ms_RadarTrace[i].m_nEntityHandle, buf);
		ReadSaveBuf(&ms_RadarTrace[i].m_vec2DPos.x, buf);
		ReadSaveBuf(&ms_RadarTrace[i].m_vec2DPos.y, buf);
		ReadSaveBuf(&ms_RadarTrace[i].m_vecPos, buf);
		ReadSaveBuf(&ms_RadarTrace[i].m_BlipIndex, buf);
		ReadSaveBuf(&ms_RadarTrace[i].m_bDim, buf);
		ReadSaveBuf(&ms_RadarTrace[i].m_bInUse, buf);
		ReadSaveBuf(&ms_RadarTrace[i].m_bShortRange, buf);
		ReadSaveBuf(&ms_RadarTrace[i].m_unused, buf);
		ReadSaveBuf(&ms_RadarTrace[i].m_wScale, buf);
		ReadSaveBuf(&ms_RadarTrace[i].m_eBlipDisplay, buf);
		ReadSaveBuf(&ms_RadarTrace[i].m_eRadarSprite, buf);
	}

VALIDATESAVEBUF(size);
}

void CRadar::SaveAllRadarBlips(uint8 *buf, uint32 *size)
{
	*size = SAVE_HEADER_SIZE + NUMRADARBLIPS * sizeof(sRadarTraceSave);

INITSAVEBUF
	WriteSaveHeader(buf, 'R', 'D', 'R', '\0', *size - SAVE_HEADER_SIZE);

#ifdef MAP_ENHANCEMENTS
	if (TargetMarkerId != -1) {
		ClearBlip(TargetMarkerId);
		TargetMarkerId = -1;
	}
#endif

	for (int i = 0; i < NUMRADARBLIPS; i++) {
		sRadarTraceSave *saveStruct = (sRadarTraceSave*) buf;

		saveStruct->m_nColor = ms_RadarTrace[i].m_nColor;
		saveStruct->m_Radius = ms_RadarTrace[i].m_Radius;
		saveStruct->m_eBlipType = ms_RadarTrace[i].m_eBlipType;
		saveStruct->m_nEntityHandle = ms_RadarTrace[i].m_nEntityHandle;
		saveStruct->m_vec2DPos = ms_RadarTrace[i].m_vec2DPos;
		saveStruct->m_vecPos = ms_RadarTrace[i].m_vecPos;
		saveStruct->m_BlipIndex = ms_RadarTrace[i].m_BlipIndex;
		saveStruct->m_bDim = ms_RadarTrace[i].m_bDim;
		saveStruct->m_bInUse = ms_RadarTrace[i].m_bInUse;
		saveStruct->m_bShortRange = ms_RadarTrace[i].m_bShortRange;
		saveStruct->m_unused = ms_RadarTrace[i].m_unused;
		saveStruct->m_wScale = ms_RadarTrace[i].m_wScale;
		saveStruct->m_eBlipDisplay = ms_RadarTrace[i].m_eBlipDisplay;
		saveStruct->m_eRadarSprite = ms_RadarTrace[i].m_eRadarSprite;

		SkipSaveBuf(buf, sizeof(sRadarTraceSave));
	}

VALIDATESAVEBUF(*size);
}

void
CRadar::LoadTextures()
{
	CTxdStore::PushCurrentTxd();
	CTxdStore::SetCurrentTxd(CTxdStore::FindTxdSlot("hud"));
	CentreSprite.SetTexture("radar_centre");
	MapHereSprite.SetTexture("arrow");
	NorthSprite.SetTexture("radar_north");
	AverySprite.SetTexture("radar_avery");
	BikerSprite.SetTexture("radar_biker");
	CortezSprite.SetTexture("radar_cortez");
	DiazSprite.SetTexture("radar_diaz");
	KentSprite.SetTexture("radar_kent");
	LawyerSprite.SetTexture("radar_lawyer");
	PhilSprite.SetTexture("radar_phil");
	BikersSprite.SetTexture("bikers");
	BoatyardSprite.SetTexture("boatyard");
	MalibuClubSprite.SetTexture("club");
	CubansSprite.SetTexture("cubans");
	FilmSprite.SetTexture("filmstudio");
	GunSprite.SetTexture("gun");
	HaitiansSprite.SetTexture("haitians");
	HardwareSprite.SetTexture("hardware");
	SaveHouseSprite.SetTexture("radar_save");
	StripSprite.SetTexture("radar_strip");
	IceSprite.SetTexture("icecream");
	KCabsSprite.SetTexture("kcabs");
	LovefistSprite.SetTexture("lovefist");
	PrintworksSprite.SetTexture("printworks");
	PropertySprite.SetTexture("property");
	SunYardSprite.SetTexture("SunYard");
	SpraySprite.SetTexture("spray");
	TShirtSprite.SetTexture("tshirt");
	TommySprite.SetTexture("tommy");
	PhoneSprite.SetTexture("phone");
	RadioWildstyleSprite.SetTexture("RWildstyle");
	RadioFlashSprite.SetTexture("RFlash");
	RadioKChatSprite.SetTexture("RKchat");
	RadioFeverSprite.SetTexture("RFever");
	RadioVRockSprite.SetTexture("RVRock");
	RadioVCPRSprite.SetTexture("RVCPR");
	RadioEspantosoSprite.SetTexture("REspantoso");
	RadioEmotionSprite.SetTexture("REmotion");
	RadioWaveSprite.SetTexture("RWave");
	CTxdStore::PopCurrentTxd();
}

void CRadar::RemoveRadarSections()
{
	for (int i = 0; i < 8; i++)
		for (int j = 0; j < 8; j++)
			RemoveMapSection(i, j);
}

void CRadar::SetBlipSprite(int32 i, int32 icon)
{
	int index = CRadar::GetActualBlipArrayIndex(i);
	if (index != -1) {
		ms_RadarTrace[index].m_eRadarSprite = icon;
	}
}

int CRadar::SetCoordBlip(eBlipType type, CVector pos, uint32 color, eBlipDisplay display)
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
	ms_RadarTrace[nextBlip].m_nColor = RADAR_TRACE_MAGENTA;
	ms_RadarTrace[nextBlip].m_bDim = true;
	ms_RadarTrace[nextBlip].m_bInUse = true;
	ms_RadarTrace[nextBlip].m_bShortRange = false;
	ms_RadarTrace[nextBlip].m_Radius = 1.0f;
	ms_RadarTrace[nextBlip].m_vec2DPos = pos;
	ms_RadarTrace[nextBlip].m_vecPos = pos;
	ms_RadarTrace[nextBlip].m_nEntityHandle = 0;
	ms_RadarTrace[nextBlip].m_wScale = 1;
	ms_RadarTrace[nextBlip].m_eBlipDisplay = display;
	ms_RadarTrace[nextBlip].m_eRadarSprite = RADAR_SPRITE_NONE;
	return CRadar::GetNewUniqueBlipIndex(nextBlip);
}

int CRadar::SetShortRangeCoordBlip(eBlipType type, CVector pos, uint32 color, eBlipDisplay display)
{
	int index = SetCoordBlip(type, pos, color, display);
	if (index == -1)
		return -1;
	ms_RadarTrace[GetActualBlipArrayIndex(index)].m_bShortRange = true;
	return index;
}

int CRadar::SetEntityBlip(eBlipType type, int32 handle, uint32 color, eBlipDisplay display)
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
	ms_RadarTrace[nextBlip].m_nColor = RADAR_TRACE_YELLOW;
	ms_RadarTrace[nextBlip].m_bDim = true;
	ms_RadarTrace[nextBlip].m_bInUse = true;
	ms_RadarTrace[nextBlip].m_bShortRange = false;
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
		size++;
		CSprite2d::Draw2DPolygon(x + SCREEN_SCALE_X(size + 3.0f), y + SCREEN_SCALE_Y(size + 2.0f), x - (SCREEN_SCALE_X(size + 3.0f)), y + SCREEN_SCALE_Y(size + 2.0f), x, y - (SCREEN_SCALE_Y(size + 3.0f)), x, y - (SCREEN_SCALE_Y(size + 3.0f)), CRGBA(0, 0, 0, alpha));
		CSprite2d::Draw2DPolygon(x + SCREEN_SCALE_X(size + 1.0f), y + SCREEN_SCALE_Y(size + 1.0f), x - (SCREEN_SCALE_X(size + 1.0f)), y + SCREEN_SCALE_Y(size + 1.0f), x, y - (SCREEN_SCALE_Y(size + 1.0f)), x, y - (SCREEN_SCALE_Y(size + 1.0f)), CRGBA(red, green, blue, alpha));
		break;
	case BLIP_MODE_TRIANGULAR_DOWN:
		size++;
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
	CentreSprite.Delete();
	MapHereSprite.Delete();
	NorthSprite.Delete();
	AverySprite.Delete();
	BikerSprite.Delete();
	CortezSprite.Delete();
	DiazSprite.Delete();
	KentSprite.Delete();
	LawyerSprite.Delete();
	PhilSprite.Delete();
	BikersSprite.Delete();
	BoatyardSprite.Delete();
	MalibuClubSprite.Delete();
	CubansSprite.Delete();
	FilmSprite.Delete();
	GunSprite.Delete();
	HaitiansSprite.Delete();
	HardwareSprite.Delete();
	SaveHouseSprite.Delete();
	StripSprite.Delete();
	IceSprite.Delete();
	KCabsSprite.Delete();
	LovefistSprite.Delete();
	PrintworksSprite.Delete();
	PropertySprite.Delete();
	SunYardSprite.Delete();
	SpraySprite.Delete();
	TShirtSprite.Delete();
	TommySprite.Delete();
	PhoneSprite.Delete();
	RadioWildstyleSprite.Delete();
	RadioFlashSprite.Delete();
	RadioKChatSprite.Delete();
	RadioFeverSprite.Delete();
	RadioVRockSprite.Delete();
	RadioVCPRSprite.Delete();
	RadioEspantosoSprite.Delete();
	RadioEmotionSprite.Delete();
	RadioWaveSprite.Delete();
	RemoveRadarSections();
}

void CRadar::StreamRadarSections(const CVector &posn)
{
	if (!CStreaming::ms_disableStreaming)
		StreamRadarSections(Floor((RADAR_MAX_X + posn.x) / RADAR_TILE_SIZE), Ceil((RADAR_NUM_TILES - 1) - (RADAR_MAX_Y + posn.y) / RADAR_TILE_SIZE));
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
	s = -cachedSin;
	c = cachedCos;

	out.x = s * in.y + c * in.x;
	out.y = c * in.y - s * in.x;

	out = out * m_radarRange + vec2DRadarOrigin;
}

// Radar space goes from -1.0 to 1.0 in x and y, top right is (1.0, 1.0)
void CRadar::TransformRadarPointToScreenSpace(CVector2D &out, const CVector2D &in)
{
	if (FrontEndMenuManager.m_bMenuMapActive) {
		out.x = (FrontEndMenuManager.m_fMapCenterX - FrontEndMenuManager.m_fMapSize) + (MENU_MAP_LENGTH / 2 + MENU_MAP_LEFT_OFFSET + in.x) * FrontEndMenuManager.m_fMapSize * MENU_MAP_WIDTH_SCALE * 2.0f / MENU_MAP_LENGTH;
		out.y = (FrontEndMenuManager.m_fMapCenterY - FrontEndMenuManager.m_fMapSize) + (MENU_MAP_LENGTH / 2 - MENU_MAP_TOP_OFFSET - in.y) * FrontEndMenuManager.m_fMapSize * MENU_MAP_HEIGHT_SCALE * 2.0f / MENU_MAP_LENGTH;
	} else {
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
	s = cachedSin;
	c = cachedCos;

	float x = (in.x - vec2DRadarOrigin.x) * (1.0f / m_radarRange);
	float y = (in.y - vec2DRadarOrigin.y) * (1.0f / m_radarRange);

	out.x = s * y + c * x;
	out.y = c * y - s * x;
}

void
CRadar::CalculateCachedSinCos()
{
	if (/*TheCamera.Cams[TheCamera.ActiveCam].Mode == CCam::MODE_TOPDOWN || TheCamera.Cams[TheCamera.ActiveCam].Mode == CCam::MODE_TOP_DOWN_PED || */
		FrontEndMenuManager.m_bMenuMapActive ) {
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

void
CRadar::InitFrontEndMap()
{
	CalculateCachedSinCos();
	vec2DRadarOrigin.x = 0.0f;
	vec2DRadarOrigin.y = 0.0f;
	m_radarRange = MENU_MAP_LENGTH_UNIT; // just affects the multiplier in TransformRadarPointToScreenSpace
	for (int i = 0; i < NUM_MAP_LEGENDS; i++) {
		MapLegendList[i] = RADAR_SPRITE_NONE;
	}
	MapLegendCounter = 0;
	ArrowBlipColour1 = CRGBA(0, 0, 0, 0);
	ArrowBlipColour2 = CRGBA(0, 0, 0, 0);
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
		const float left = x - SCREEN_SCALE_X(8.0f);
		const float top = y - SCREEN_SCALE_Y(40.0f);
		const float right = x + SCREEN_SCALE_X(40.0);
		const float bottom = y + SCREEN_SCALE_Y(8.0f);
		MapHereSprite.Draw(CRect(left + SCREEN_SCALE_X(2.f), top + SCREEN_SCALE_Y(2.f), right + SCREEN_SCALE_X(2.f), bottom + SCREEN_SCALE_Y(2.f)),
			CRGBA(0, 0, 0, 255));

		MapHereSprite.Draw(CRect(left, top, right, bottom), CRGBA(255, 255, 255, 255));

		CFont::SetWrapx(right + SCREEN_SCALE_X(28.0f));
		CFont::SetRightJustifyWrap(right);
		CFont::SetBackGroundOnlyTextOff();
		CFont::SetColor(CRGBA(255, 150, 225, 255));
		CFont::SetDropShadowPosition(2);
		CFont::SetDropColor(CRGBA(0, 0, 0, 255));
		CFont::SetCentreOff();
		CFont::SetRightJustifyOff();
		CFont::SetFontStyle(FONT_LOCALE(FONT_HEADING));
		CFont::SetScale(SCREEN_SCALE_X(0.65f), SCREEN_SCALE_Y(0.95f));
		CFont::PrintString(right, top, TheText.Get("MAP_YAH"));
		CFont::SetDropShadowPosition(0);
		CFont::DrawFonts();
	}
	MapLegendList[MapLegendCounter++] = RADAR_SPRITE_MAP_HERE;
}

#ifdef MAP_ENHANCEMENTS
void
CRadar::ToggleTargetMarker(float x, float y)
{
	if (TargetMarkerId == -1) {
		int nextBlip;
		for (nextBlip = 0; nextBlip < NUMRADARBLIPS; nextBlip++) {
			if (!ms_RadarTrace[nextBlip].m_bInUse)
				break;
		}
		
		if (nextBlip == NUMRADARBLIPS)
			return;
			
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

void
CRadar::DrawEntityBlip(int32 blipId)
{
	CVector2D out;
	CVector2D in;
	CEntity *blipEntity;
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
			if (!ms_RadarTrace[blipId].m_bShortRange || dist <= 1.0f || FrontEndMenuManager.m_bMenuMapActive) {
				if (ms_RadarTrace[blipId].m_eRadarSprite != RADAR_SPRITE_NONE) {
					DrawRadarSprite(ms_RadarTrace[blipId].m_eRadarSprite, out.x, out.y, CalculateBlipAlpha(dist));
				} else {
					const CVector& pos = FindPlayerCentreOfWorld_NoSniperShift();
					const CVector& blipPos = blipEntity->GetPosition();
					uint8 mode = BLIP_MODE_TRIANGULAR_UP;
					if (blipPos.z - pos.z <= 2.0f) {
						if (blipPos.z - pos.z < -4.0f) mode = BLIP_MODE_TRIANGULAR_DOWN;
						else mode = BLIP_MODE_SQUARE;
					}
					ShowRadarTraceWithHeight(out.x, out.y, ms_RadarTrace[blipId].m_wScale, (uint8)(color >> 24), (uint8)(color >> 16), (uint8)(color >> 8), 255, mode);

					if (FrontEndMenuManager.m_bMenuMapActive) {
						bool alreadyThere = false;
						for (int i = 0; i < NUM_MAP_LEGENDS; i++) {
							if (MapLegendList[i] == -2)
								alreadyThere = true;
						}
						if (!alreadyThere) {
							MapLegendList[MapLegendCounter] = -2;
							MapLegendCounter++;
							ArrowBlipColour2 = CRGBA((uint8)(color >> 24), (uint8)(color >> 16), (uint8)(color >> 8), 255);
						}
					}
				}
			}
		}
	}
}

void
CRadar::DrawCoordBlip(int32 blipId)
{
	CVector2D out;
	CVector2D in;
	if (ms_RadarTrace[blipId].m_eBlipType != BLIP_CONTACT_POINT || !CTheScripts::IsPlayerOnAMission()) {

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
			if (!ms_RadarTrace[blipId].m_bShortRange || dist <= 1.0f || FrontEndMenuManager.m_bMenuMapActive) {
				if (ms_RadarTrace[blipId].m_eRadarSprite != RADAR_SPRITE_NONE) {
					DrawRadarSprite(ms_RadarTrace[blipId].m_eRadarSprite, out.x, out.y, CalculateBlipAlpha(dist));
				} else {
					const CVector& pos = FindPlayerCentreOfWorld_NoSniperShift();
					const CVector& blipPos = ms_RadarTrace[blipId].m_vecPos;
					uint8 mode = BLIP_MODE_TRIANGULAR_UP;
					if (blipPos.z - pos.z <= 2.0f) {
						if (blipPos.z - pos.z < -4.0f) mode = BLIP_MODE_TRIANGULAR_DOWN;
						else mode = BLIP_MODE_SQUARE;
					}
					ShowRadarTraceWithHeight(out.x, out.y, ms_RadarTrace[blipId].m_wScale, (uint8)(color >> 24), (uint8)(color >> 16), (uint8)(color >> 8), 255, mode);

					if (FrontEndMenuManager.m_bMenuMapActive) {
						bool alreadyThere = false;
						for (int i = 0; i < NUM_MAP_LEGENDS; i++) {
							if (MapLegendList[i] == -1)
								alreadyThere = true;
						}
						if (!alreadyThere) {
							MapLegendList[MapLegendCounter] = -1;
							MapLegendCounter++;
							ArrowBlipColour1 = CRGBA((uint8)(color >> 24), (uint8)(color >> 16), (uint8)(color >> 8), 255);
						}
					}
				}
			}
		}
	}
}

void
CRadar::DrawLegend(int32 x, int32 y, int32 sprite)
{
	if (sprite < 0) {
		static uint32 lastChange = 0;
		static int8 blipMode = 0;

		CRGBA color;
		if (sprite == -1) {
			color = ArrowBlipColour1;
		} else {
			color = ArrowBlipColour2;
		}

		if (CTimer::GetTimeInMillisecondsPauseMode() - lastChange > 600) {
			lastChange = CTimer::GetTimeInMillisecondsPauseMode();
			if ( blipMode == 2 )
				blipMode = 0;
			else
				++blipMode;
		}

		switch (blipMode) {
			case BLIP_MODE_TRIANGULAR_UP:
				CSprite2d::Draw2DPolygon(x + SCREEN_SCALE_X(14.0f), y + SCREEN_SCALE_Y(13.0f), x + SCREEN_SCALE_X(2.0f), y + SCREEN_SCALE_Y(13.0f), x + SCREEN_SCALE_X(8.f), y + SCREEN_SCALE_Y(2.0f), x + SCREEN_SCALE_X(8.f), y + SCREEN_SCALE_Y(2.0f), CRGBA(0, 0, 0, 255));
				CSprite2d::Draw2DPolygon(x + SCREEN_SCALE_X(12.0f), y + SCREEN_SCALE_Y(12.0f), x + SCREEN_SCALE_X(4.0f), y + SCREEN_SCALE_Y(12.0f), x + SCREEN_SCALE_X(8.f), y + SCREEN_SCALE_Y(4.0f), x + SCREEN_SCALE_X(8.f), y + SCREEN_SCALE_Y(4.0f), color);
				break;
			case BLIP_MODE_TRIANGULAR_DOWN:
				CSprite2d::Draw2DPolygon(x + SCREEN_SCALE_X(8.0f), y + SCREEN_SCALE_Y(14.0f), x + SCREEN_SCALE_X(8.0f), y + SCREEN_SCALE_Y(14.0f), x + SCREEN_SCALE_X(2.f), y + SCREEN_SCALE_Y(3.0f), x + SCREEN_SCALE_X(2.f), y + SCREEN_SCALE_Y(3.0f), CRGBA(0, 0, 0, 255));
				CSprite2d::Draw2DPolygon(x + SCREEN_SCALE_X(8.0f), y + SCREEN_SCALE_Y(12.0f), x + SCREEN_SCALE_X(8.0f), y + SCREEN_SCALE_Y(12.0f), x + SCREEN_SCALE_X(12.f), y + SCREEN_SCALE_Y(4.0f), x + SCREEN_SCALE_X(4.f), y + SCREEN_SCALE_Y(4.0f), color);
				break;
			case BLIP_MODE_SQUARE:
				CSprite2d::DrawRect(CRect(x + SCREEN_SCALE_X(4.0f), y + SCREEN_SCALE_Y(3.0f), SCREEN_SCALE_X(12.0f) + x, SCREEN_SCALE_Y(12.0f) + y), CRGBA(0, 0, 0, 255));
				CSprite2d::DrawRect(CRect(x + SCREEN_SCALE_X(5.0f), y + SCREEN_SCALE_Y(4.0f), SCREEN_SCALE_X(11.0f) + x, SCREEN_SCALE_Y(11.0f) + y), color);
				break;
		}

	} else {
		RadarSprites[sprite]->Draw(CRect(x, y, x + SCREEN_SCALE_X(16.f), y + SCREEN_SCALE_X(16.f)), CRGBA(255, 255, 255, 255));
	}

	wchar *text;
	switch ( sprite ) {
		case RADAR_SPRITE_ENTITY_BLIP:
			text = TheText.Get("LG_38");
		break;
		case RADAR_SPRITE_COORD_BLIP:
			text = TheText.Get("LG_35");
		break;
		case RADAR_SPRITE_MAP_HERE:
			text = TheText.Get("LG_01");
		break;
		case RADAR_SPRITE_AVERY:
			text = TheText.Get("LG_02");
		break;
		case RADAR_SPRITE_BIKER:
			text = TheText.Get("LG_03");
		break;
		case RADAR_SPRITE_CORTEZ:
			text = TheText.Get("LG_04");
		break;
		case RADAR_SPRITE_DIAZ:
			text = TheText.Get("LG_05");
		break;
		case RADAR_SPRITE_KENT:
			text = TheText.Get("LG_06");
		break;
		case RADAR_SPRITE_LAWYER:
			text = TheText.Get("LG_07");
		break;
		case RADAR_SPRITE_PHIL:
			text = TheText.Get("LG_08");
		break;
		case RADAR_SPRITE_BIKERS:
			text = TheText.Get("LG_03");
		break;
		case RADAR_SPRITE_BOATYARD:
			text = TheText.Get("LG_09");
		break;
		case RADAR_SPRITE_MALIBU_CLUB:
			text = TheText.Get("LG_10");
		break;
		case RADAR_SPRITE_CUBANS:
			text = TheText.Get("LG_11");
		break;
		case RADAR_SPRITE_FILM:
			text = TheText.Get("LG_12");
		break;
		case RADAR_SPRITE_GUN:
			text = TheText.Get("LG_13");
		break;
		case RADAR_SPRITE_HAITIANS:
			text = TheText.Get("LG_14");
		break;
		case RADAR_SPRITE_HARDWARE:
			text = TheText.Get("LG_15");
		break;
		case RADAR_SPRITE_SAVE:
			text = TheText.Get("LG_16");
		break;
		case RADAR_SPRITE_STRIP:
			text = TheText.Get("LG_37");
		break;
		case RADAR_SPRITE_ICE:
			text = TheText.Get("LG_17");
		break;
		case RADAR_SPRITE_KCABS:
			text = TheText.Get("LG_18");
		break;
		case RADAR_SPRITE_LOVEFIST:
			text = TheText.Get("LG_19");
		break;
		case RADAR_SPRITE_PRINTWORKS:
			text = TheText.Get("LG_20");
		break;
		case RADAR_SPRITE_PROPERTY:
			text = TheText.Get("LG_21");
		break;
		case RADAR_SPRITE_SUNYARD:
			text = TheText.Get("LG_36");
		break;
		case RADAR_SPRITE_SPRAY:
			text = TheText.Get("LG_22");
		break;
		case RADAR_SPRITE_TSHIRT:
			text = TheText.Get("LG_23");
		break;
		case RADAR_SPRITE_TOMMY:
			text = TheText.Get("LG_24");
		break;
		case RADAR_SPRITE_PHONE:
			text = TheText.Get("LG_25");
		break;
		case RADAR_SPRITE_RADIO_WILDSTYLE:
			text = TheText.Get("LG_26");
		break;
		case RADAR_SPRITE_RADIO_FLASH:
			text = TheText.Get("LG_27");
		break;
		case RADAR_SPRITE_RADIO_KCHAT:
			text = TheText.Get("LG_28");
		break;
		case RADAR_SPRITE_RADIO_FEVER:
			text = TheText.Get("LG_29");
		break;
		case RADAR_SPRITE_RADIO_VROCK:
			text = TheText.Get("LG_30");
		break;
		case RADAR_SPRITE_RADIO_VCPR:
			text = TheText.Get("LG_31");
		break;
		case RADAR_SPRITE_RADIO_ESPANTOSO:
			text = TheText.Get("LG_32");
		break;
		case RADAR_SPRITE_RADIO_EMOTION:
			text = TheText.Get("LG_33");
		break;
		case RADAR_SPRITE_RADIO_WAVE:
			text = TheText.Get("LG_34");
		break;
		default:
		break;
	}
	CFont::PrintString(SCREEN_SCALE_X(20.f) + x, SCREEN_SCALE_Y(3.0f) + y, text);
}
