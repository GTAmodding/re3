#include "config.h"
#include "common.h"
#include "patcher.h"
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

float &CRadar::m_RadarRange = *(float*)0x8E281C;
CBlip *CRadar::ms_RadarTrace = (CBlip*)0x6ED5E0;

CVector2D &vec2DRadarOrigin = *(CVector2D*)0x6299B8;
int *gRadarTxdIds = (int*)0x6299C0;

CSprite2d *CRadar::AsukaSprite = (CSprite2d*)0x8F1A40;
CSprite2d *CRadar::BombSprite = (CSprite2d*)0x8F5FB4;
CSprite2d *CRadar::CatSprite = (CSprite2d*)0x885B24;
CSprite2d *CRadar::CentreSprite = (CSprite2d*)0x8F6268;
CSprite2d *CRadar::CopcarSprite = (CSprite2d*)0x8F1A2C;
CSprite2d *CRadar::DonSprite = (CSprite2d*)0x8F2BE0;
CSprite2d *CRadar::EightSprite = (CSprite2d*)0x8F2BCC;
CSprite2d *CRadar::ElSprite = (CSprite2d*)0x8F1B80;
CSprite2d *CRadar::IceSprite = (CSprite2d*)0x9415FC;
CSprite2d *CRadar::JoeySprite = (CSprite2d*)0x8F2C00;
CSprite2d *CRadar::KenjiSprite = (CSprite2d*)0x8F2C68;
CSprite2d *CRadar::LizSprite = (CSprite2d*)0x8F5830;
CSprite2d *CRadar::LuigiSprite = (CSprite2d*)0x8F1A3C;
CSprite2d *CRadar::NorthSprite = (CSprite2d*)0x8F6274;
CSprite2d *CRadar::RaySprite = (CSprite2d*)0x8E2A7C;
CSprite2d *CRadar::SalSprite = (CSprite2d*)0x8F29EC;
CSprite2d *CRadar::SaveSprite = (CSprite2d*)0x8F5F74;
CSprite2d *CRadar::SpraySprite = (CSprite2d*)0x94307C;
CSprite2d *CRadar::TonySprite = (CSprite2d*)0x885B58;
CSprite2d *CRadar::WeaponSprite = (CSprite2d*)0x941534;

CSprite2d *CRadar::RadarSprites[RADAR_SPRITE_COUNT] = { 
	nil,
	AsukaSprite,
	BombSprite,
	CatSprite,
	CentreSprite,
	CopcarSprite,
	DonSprite,
	EightSprite,
	ElSprite,
	IceSprite,
	JoeySprite,
	KenjiSprite,
	LizSprite,
	LuigiSprite,
	NorthSprite,
	RaySprite,
	SalSprite,
	SaveSprite,
	SpraySprite,
	TonySprite,
	WeaponSprite
};

#define RADAR_NUM_TILES (8)
#define RADAR_TILE_SIZE (WORLD_SIZE_X / RADAR_NUM_TILES)
static_assert(RADAR_TILE_SIZE == (WORLD_SIZE_Y / RADAR_NUM_TILES), "CRadar: not a square");

#define RADAR_MIN_RANGE (120.0f)
#define RADAR_MAX_RANGE (350.0f)
#define RADAR_MIN_SPEED (0.3f)
#define RADAR_MAX_SPEED (0.9f)

#if 0
WRAPPER void CRadar::CalculateBlipAlpha(float) { EAXJMP(0x4A4F90); }
#else
int CRadar::CalculateBlipAlpha(float dist)
{
	if (dist <= 1.0f)
		return 255;

	if (dist <= 5.0f)
		return (((1.0f - ((dist * 0.25f) - 0.25f)) * 255.0f) + (((dist * 0.25f) - 0.25f) * 128.0f));

	return 128;
}
#endif

#if 1
WRAPPER void CRadar::ChangeBlipBrightness(int32, int32) { EAXJMP(0x4A57A0); }
#else
void CRadar::ChangeBlipBrightness(int32 i, int32 bright)
{

}
#endif

#if 1
WRAPPER void CRadar::ChangeBlipColour(int32) { EAXJMP(0x4A5770); }
#else
void CRadar::ChangeBlipColour(int32 i)
{

}
#endif

#if 1
WRAPPER void CRadar::ChangeBlipDisplay(int32, int16) { EAXJMP(0x4A5810); }
#else
void CRadar::ChangeBlipDisplay(int32 i, int16 flag)
{

}
#endif

#if 1
WRAPPER void CRadar::ChangeBlipScale(int32, int16) { EAXJMP(0x4A57E0); }
#else
void CRadar::ChangeBlipScale(int32 i, int16 scale)
{

}
#endif

#if 1
WRAPPER void CRadar::ClearBlip(int32) { EAXJMP(0x4A5720); }
#else
void CRadar::ClearBlip(int32 i)
{

}
#endif

#if 0
WRAPPER void CRadar::ClearBlipForEntity(int16, int32) { EAXJMP(0x4A56C0); }
#else
void CRadar::ClearBlipForEntity(int16 type, int32 id)
{
	for (int i = 0; i < NUMRADARBLIPS; i++) {
		if (type == ms_RadarTrace[i].m_eBlipType && id == ms_RadarTrace[i].m_nEntityHandle) {
			SetRadarMarkerState(i, 0);
			ms_RadarTrace[i].m_bInUse = 0;
			ms_RadarTrace[i].m_eBlipType = 0;
			ms_RadarTrace[i].m_eBlipDisplay = 0;
			ms_RadarTrace[i].m_IconID = 0;
		}
	};
}
#endif

#if 0
WRAPPER int CRadar::ClipRadarPoly(CVector2D *poly, const CVector2D *in) { EAXJMP(0x4A64A0); }
#else
// Why not a proper clipping algorithm?
int CRadar::ClipRadarPoly(CVector2D *poly, const CVector2D *rect)
{
	CVector2D corners[4] = {
		{  1.0f, -1.0f },	// top right
		{  1.0f,  1.0f },	// bottom right
		{ -1.0f,  1.0f },	// bottom left
		{ -1.0f, -1.0f },	// top left
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
#endif

bool CRadar::DisplayThisBlip(int32 counter)
{
	switch (ms_RadarTrace[counter].m_IconID) {
	case RADAR_SPRITE_BOMB:
	case RADAR_SPRITE_SPRAY:
	case RADAR_SPRITE_WEAPON:
		return true;
	default:
		return false;
	}
}

#if 1
WRAPPER void CRadar::Draw3dMarkers() { EAXJMP(0x4A4C70); }
#else
void CRadar::Draw3dMarkers()
{

}
#endif


#if 0
WRAPPER void CRadar::DrawBlips() { EAXJMP(0x4A42F0); }
#else
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

		float angle;
		if (TheCamera.Cams[TheCamera.ActiveCam].Mode == CCam::MODE_TOPDOWN1)
			angle = PI + FindPlayerHeading();
		else
			angle = FindPlayerHeading() - (PI + TheCamera.GetForward().Heading());

		DrawRotatingRadarSprite(CentreSprite, out.x, out.y, angle, 255);

		CVector2D vec2d;
		vec2d.x = vec2DRadarOrigin.x;
		vec2d.y = M_SQRT2 * m_RadarRange + vec2DRadarOrigin.y;
		TransformRealWorldPointToRadarSpace(in, vec2d);
		LimitRadarPoint(in);
		TransformRadarPointToScreenSpace(out, in);
		DrawRadarSprite(RADAR_SPRITE_NORTH, out.x, out.y, 255);

		/*
			DrawEntityBlip
		*/
		for (int i = 0; i < NUMRADARBLIPS; i++) {
			if (ms_RadarTrace[i].m_bInUse) {
				if (ms_RadarTrace[i].m_eBlipType <= BLIP_OBJECT) {
					CEntity *e = nil;
					switch (ms_RadarTrace[i].m_eBlipType) {
					case BLIP_CAR:
						e = CPools::GetVehiclePool()->GetAt(ms_RadarTrace[i].m_nEntityHandle);
						break;
					case BLIP_CHAR:
						e = CPools::GetPedPool()->GetAt(ms_RadarTrace[i].m_nEntityHandle);
						break;
					case BLIP_OBJECT:
						e = CPools::GetObjectPool()->GetAt(ms_RadarTrace[i].m_nEntityHandle);
						break;
					};

					if (e) {
						if (ms_RadarTrace[i].m_eBlipDisplay == BLIP_DISPLAY_BOTH || ms_RadarTrace[i].m_eBlipDisplay == BLIP_DISPLAY_MARKER_ONLY) {
							if (CTheScripts::DbgFlag) {
								ShowRadarMarker(e->GetPosition(), GetRadarTraceColour(ms_RadarTrace[i].m_nColor, ms_RadarTrace[i].m_bDim), ms_RadarTrace->m_Radius);

								ms_RadarTrace[i].m_Radius = ms_RadarTrace[i].m_Radius - 0.1f;
								if (ms_RadarTrace[i].m_Radius >= 1.0f)
									ms_RadarTrace[i].m_Radius = 5.0;
							}
						}
						if (ms_RadarTrace[i].m_eBlipDisplay == BLIP_DISPLAY_BOTH || ms_RadarTrace[i].m_eBlipDisplay == BLIP_DISPLAY_BLIP_ONLY) {
							vec2d = e->GetPosition();
							TransformRealWorldPointToRadarSpace(in, vec2d);
							float dist = LimitRadarPoint(in);
							int a = CalculateBlipAlpha(dist);
							TransformRadarPointToScreenSpace(out, in);

							int32 col = GetRadarTraceColour(ms_RadarTrace[i].m_nColor, ms_RadarTrace[i].m_bDim);

							if (ms_RadarTrace[i].m_IconID)
								DrawRadarSprite(ms_RadarTrace[i].m_IconID, out.x, out.y, a);
							else
								ShowRadarTrace(out.x, out.y, ms_RadarTrace[i].m_wScale, ((col >> 24)), ((col >> 16) & 0xFF), ((col >> 8)), 255);
						}
					}
				}

				/*
					DrawCoordBlip
				*/
				if (ms_RadarTrace[i].m_eBlipType >= BLIP_COORD) {
					if (ms_RadarTrace[i].m_eBlipType != BLIP_CONTACT_POINT || ms_RadarTrace[i].m_eBlipType == BLIP_CONTACT_POINT && DisplayThisBlip(i) || !CTheScripts::IsPlayerOnAMission()) {
						if (ms_RadarTrace[i].m_eBlipDisplay == BLIP_DISPLAY_BOTH || ms_RadarTrace[i].m_eBlipDisplay == BLIP_DISPLAY_MARKER_ONLY) {
							if (CTheScripts::DbgFlag) {
								ShowRadarMarker(ms_RadarTrace[i].m_vecPos, GetRadarTraceColour(ms_RadarTrace[i].m_nColor, ms_RadarTrace[i].m_bDim), ms_RadarTrace->m_Radius);
								ms_RadarTrace[i].m_Radius = ms_RadarTrace[i].m_Radius - 0.1f;
								if (ms_RadarTrace[i].m_Radius >= 1.0f)
									ms_RadarTrace[i].m_Radius = 5.0f;
							}
						}

						if (ms_RadarTrace[i].m_eBlipDisplay == BLIP_DISPLAY_BOTH || ms_RadarTrace[i].m_eBlipDisplay == BLIP_DISPLAY_BLIP_ONLY) {
							TransformRealWorldPointToRadarSpace(in, ms_RadarTrace[i].m_vec2DPos);
							float dist = LimitRadarPoint(in);
							int a = CalculateBlipAlpha(dist);
							TransformRadarPointToScreenSpace(out, in);

							int32 col = GetRadarTraceColour(ms_RadarTrace[i].m_nColor, ms_RadarTrace[i].m_bDim);

							if (ms_RadarTrace[i].m_IconID)
								DrawRadarSprite(ms_RadarTrace[i].m_IconID, out.x, out.y, a);
							else
								ShowRadarTrace(out.x, out.y, ms_RadarTrace[i].m_wScale, ((col >> 24)), ((col >> 16) & 0xFF), ((col >> 8)), 255);
						}
					}
				}
			};
		}
	}
}
#endif


#if 0
WRAPPER void CRadar::DrawMap () { EAXJMP(0x4A4200); }
#else
void CRadar::DrawMap()
{
	if (!TheCamera.m_WideScreenOn && CHud::m_Wants_To_Draw_Hud) {
		if (FindPlayerVehicle()) {
			float speed = FindPlayerSpeed().Magnitude();
			if (speed < RADAR_MIN_SPEED)
				m_RadarRange = RADAR_MIN_RANGE;
			else if (speed < RADAR_MAX_SPEED)
				m_RadarRange = (speed - RADAR_MIN_SPEED)/(RADAR_MAX_SPEED-RADAR_MIN_SPEED) * (RADAR_MAX_RANGE-RADAR_MIN_RANGE) + RADAR_MIN_RANGE;
			else
				m_RadarRange = RADAR_MAX_RANGE;
		}
		else
			m_RadarRange = RADAR_MIN_RANGE;

		vec2DRadarOrigin = CVector2D(FindPlayerCentreOfWorld_NoSniperShift());
		DrawRadarMap();
	}
}
#endif 

#if 0
WRAPPER void CRadar::DrawRadarMap() { EAXJMP(0x4A6C20); }
#else
void CRadar::DrawRadarMap()
{
	// Game calculates an unused CRect here

	DrawRadarMask();

	// top left ist (0, 0)
	int x = floorf((vec2DRadarOrigin.x - WORLD_MIN_X) / RADAR_TILE_SIZE);
	int y = ceilf((RADAR_NUM_TILES - 1) - (vec2DRadarOrigin.y - WORLD_MIN_Y) / RADAR_TILE_SIZE);
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
#endif

#if 0
WRAPPER void CRadar::DrawRadarMask() { EAXJMP(0x4A69C0); }
#else
void CRadar::DrawRadarMask() 
{ 
	CVector2D corners[4] = {
		CVector2D(1.0f, -1.0f),
		CVector2D(1.0f, 1.0f),
		CVector2D(-1.0f, 1.0f),
		CVector2D(-1.0, -1.0f)
	};

	RwRenderStateSet(rwRENDERSTATETEXTURERASTER, (void*)FALSE);
	RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDSRCALPHA);
	RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDINVSRCALPHA);
	RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void*)FALSE);
	RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void*)rwFILTERLINEAR);
	RwRenderStateSet(rwRENDERSTATESHADEMODE, (void*)rwSHADEMODEFLAT);
	RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*)FALSE);
	RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)TRUE);
	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)TRUE);
	RwD3D8SetRenderState(rwRENDERSTATESTENCILFUNCTION, rwSTENCILFUNCTIONALWAYS);

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
			in.x = corners[i].x * cos(j * (PI / 2.0f / 6.0f));
			in.y = corners[i].y * sin(j * (PI / 2.0f / 6.0f));
			TransformRadarPointToScreenSpace(out[j + 1], in);
		};

		CSprite2d::SetMaskVertices(8, (float *)out);
		RwIm2DRenderPrimitive(rwPRIMTYPETRIFAN, CSprite2d::GetVertices(), 8);
	};

	RwD3D8SetRenderState(rwRENDERSTATESTENCILFUNCTION, rwSTENCILFUNCTIONGREATER);
}
#endif

#if 0
WRAPPER void CRadar::DrawRadarSection(int32, int32) { EAXJMP(0x4A67E0); }
#else
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
#endif

#if 0
WRAPPER void CRadar::DrawRadarSprite(int32 sprite, float x, float y, int32 alpha) { EAXJMP(0x4A5EF0); }
#else
void CRadar::DrawRadarSprite(int32 sprite, float x, float y, int32 alpha)
{
#ifndef SA_STYLE_RADAR
	RadarSprites[sprite]->Draw(CRect(x - SCREEN_SCALE_X(8.0f), y - SCREEN_SCALE_Y(8.0f), x + SCREEN_SCALE_X(8.0f), y + SCREEN_SCALE_Y(8.0f)), CRGBA(255, 255, 255, alpha));
#else
	RadarSprites[sprite]->Draw(CRect(x - SCREEN_SCALE_X(6.0f), y - SCREEN_SCALE_Y(6.0f), x + SCREEN_SCALE_X(6.0f), y + SCREEN_SCALE_Y(6.0f)), CRGBA(255, 255, 255, alpha));
#endif
}
#endif

#if 0
WRAPPER void CRadar::DrawRotatingRadarSprite(CSprite2d* sprite, float x, float y, float angle, int32 alpha) { EAXJMP(0x4A5D10); }
#else
void CRadar::DrawRotatingRadarSprite(CSprite2d* sprite, float x, float y, float angle, int32 alpha)
{
	CVector curPosn[4];
	CVector oldPosn[4];

	float fSize = 0.0f;

#ifndef SA_STYLE_RADAR
	fSize = 5.6f;
#else
	fSize = 4.6f;
#endif

	curPosn[0].x = x - SCREEN_SCALE_X(fSize);
	curPosn[0].y = y + SCREEN_SCALE_Y(fSize);

	curPosn[1].x = x + SCREEN_SCALE_X(fSize);
	curPosn[1].y = y + SCREEN_SCALE_Y(fSize);

	curPosn[2].x = x - SCREEN_SCALE_X(fSize);
	curPosn[2].y = y - SCREEN_SCALE_Y(fSize);

	curPosn[3].x = x + SCREEN_SCALE_X(fSize);
	curPosn[3].y = y - SCREEN_SCALE_Y(fSize);

	for (uint32 i = 0; i < 4; i++) {
		oldPosn[i] = curPosn[i];

		curPosn[i].x = x + (oldPosn[i].x - x) * cosf(angle) + (oldPosn[i].y - y) * sinf(angle);
		curPosn[i].y = y - (oldPosn[i].x - x) * sinf(angle) + (oldPosn[i].y - y) * cosf(angle);
	}

	sprite->Draw(curPosn[2].x, curPosn[2].y, curPosn[3].x, curPosn[3].y, curPosn[0].x, curPosn[0].y, curPosn[1].x, curPosn[1].y, CRGBA(255, 255, 255, alpha));
}
#endif 

#if 1
WRAPPER int32 CRadar::GetActualBlipArray(int32) { EAXJMP(0x4A41C0); }
#else
int32 CRadar::GetActualBlipArray(int32 i)
{
	return int32();
}
#endif

#if 1
WRAPPER int32 CRadar::GetNewUniqueBlipIndex(int32) { EAXJMP(0x4A4180); }
#else
int32 CRadar::GetNewUniqueBlipIndex(int32 i)
{
	return int32();
}
#endif

#if 0
WRAPPER int32 CRadar::GetRadarTraceColour(int32 color, bool bright) { EAXJMP(0x4A5BB0); }
#else
int32 CRadar::GetRadarTraceColour(int32 color, bool bright)
{
	int32 c;
	switch (color) {
	case 0:
		if (bright)
			c = 0x712B49FF;
		else
			c = 0x7F0000FF;
		break;
	case 1:
		if (bright)
			c = 0x5FA06AFF;
		else
			c = 0x7F00FF;
		break;
	case 2:
		if (bright)
			c = 0x80A7F3FF;
		else
			c = 0x007FFF;
		break;
	case 3:
		if (bright)
			c = 0xE1E1E1FF;
		else
			c = 0x7F7F7FFF;
		break;
	case 4:
		if (bright)
			c = 0xFFFF00FF;
		else
			c = 0x7F7F00FF;
		break;
	case 5:
		if (bright)
			c = 0xFF00FFFF;
		else
			c = 0x7F007FFF;
		break;
	case 6:
		if (bright)
			c = 0xFFFFFF;
		else
			c = 0x7F7FFF;
		break;
	default:
		c = color;
		break;
	};
	return c;
}
#endif

#if 1
WRAPPER void CRadar::Initialise() { EAXJMP(0x4A3EF0); }
#else
void CRadar::Initialise()
{

}
#endif

#if 0
WRAPPER float CRadar::LimitRadarPoint(CVector2D &point) { EAXJMP(0x4A4F30); }
#else
float CRadar::LimitRadarPoint(CVector2D &point)
{
	float dist, invdist;

	dist = point.Magnitude();
	if (dist > 1.0f) {
		invdist = 1.0f / dist;
		point.x *= invdist;
		point.y *= invdist;
	}
	return dist;
}
#endif

#if 1
WRAPPER void CRadar::LoadAllRadarBlips(int32) { EAXJMP(0x4A6F30); }
#else
void CRadar::LoadAllRadarBlips(int32)
{

}
#endif

#if 1
WRAPPER void CRadar::LoadTextures() { EAXJMP(0x4A4030); }
#else
void CRadar::LoadTextures()
{

}
#endif

#if 1
WRAPPER void CRadar::RemoveRadarSections() { EAXJMP(0x4A60E0); }
#else
void CRadar::RemoveRadarSections()
{

}
#endif

#if 0
WRAPPER void CRadar::RemoveMapSection(int32, int32) { EAXJMP(0x00); }
#else
void CRadar::RemoveMapSection(int32 x, int32 y)
{
	if (x >= 0 && x <= 7 && y >= 0 && y <= 7)
		CStreaming::RemoveTxd(gRadarTxdIds[x + RADAR_NUM_TILES * y]);
}
#endif

#if 0
WRAPPER void CRadar::RequestMapSection(int32, int32) { EAXJMP(0x00); }
#else
void CRadar::RequestMapSection(int32 x, int32 y)
{
	ClipRadarTileCoords(x, y);
	CStreaming::RequestTxd(gRadarTxdIds[x + RADAR_NUM_TILES * y], STREAMFLAGS_DONT_REMOVE | STREAMFLAGS_DEPENDENCY);
}
#endif

#if 1
WRAPPER void CRadar::SaveAllRadarBlips(int32) { EAXJMP(0x4A6E30); }
#else
void CRadar::SaveAllRadarBlips(int32)
{

}
#endif

#if 1
WRAPPER void CRadar::SetBlipSprite(int32, int32) { EAXJMP(0x4A5840); }
#else
void CRadar::SetBlipSprite(int32 i, int32 icon)
{

}
#endif

#if 1
WRAPPER int CRadar::SetCoordBlip(int32, CVector, int32) { EAXJMP(0x4A5590); }
#else
int CRadar::SetCoordBlip(int32 type, CVector pos, int32 flag)
{
	return 0;
}
#endif

#if 1
WRAPPER int CRadar::SetEntityBlip(int32 type, CVector pos, int32 color, int32 flag) { EAXJMP(0x4A5640); }
#else
int CRadar::SetEntityBlip(int32 type, CVector pos, int32 color, int32 flag)
{
	return 0;
}
#endif

#if 0
WRAPPER void CRadar::SetRadarMarkerState(int32, int32) { EAXJMP(0x4A5C60); }
#else
void CRadar::SetRadarMarkerState(int32 counter, int32 flag)
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
#endif

#if 0
WRAPPER void CRadar::ShowRadarMarker(CVector pos, int16 color, float radius) { EAXJMP(0x4A59C0); }
#else
void CRadar::ShowRadarMarker(CVector pos, int16 color, float radius) {
	float f1 = radius * 0.5f;
	float f2 = radius * 1.4f;
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
#endif

#if 0
WRAPPER void CRadar::ShowRadarTrace(float x, float y, uint32 size, uint32 red, uint32 green, uint32 blue, uint32 alpha) { EAXJMP(0x4A5870); }
#else
void CRadar::ShowRadarTrace(float x, float y, uint32 size, uint32 red, uint32 green, uint32 blue, uint32 alpha) 
{
	CSprite2d::DrawRect(CRect(x - SCREEN_SCALE_X(size + 1.0f), y - SCREEN_SCALE_Y(size + 1.0f), SCREEN_SCALE_X(size + 1.0f) + x, SCREEN_SCALE_Y(size + 1.0f) + y), CRGBA(0, 0, 0, alpha));
	CSprite2d::DrawRect(CRect(x - SCREEN_SCALE_X(size), y - SCREEN_SCALE_Y(size), SCREEN_SCALE_X(size) + x, SCREEN_SCALE_Y(size) + y), CRGBA(red, green, blue, alpha));
}
#endif

#if 1
WRAPPER void CRadar::Shutdown() { EAXJMP(0x4A3F60); }
#else
void CRadar::Shutdown()
{

}
#endif

#if 1
WRAPPER void CRadar::StreamRadarSections(const CVector &posn) { EAXJMP(0x4A6B60); }
#else
void CRadar::StreamRadarSections(const CVector &posn)
{

}
#endif

#if 0
WRAPPER void CRadar::StreamRadarSections(int32 x, int32 y) { EAXJMP(0x4A6100); }
#else
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
#endif

#if 0
WRAPPER void CRadar::TransformRealWorldToTexCoordSpace(CVector2D &out, const CVector2D &in, int32 x, int32 y) { EAXJMP(0x4A5530); }
#else
void CRadar::TransformRealWorldToTexCoordSpace(CVector2D &out, const CVector2D &in, int32 x, int32 y)
{
	out.x = in.x - (x * RADAR_TILE_SIZE + WORLD_MIN_X);
	out.y = -(in.y - ((RADAR_NUM_TILES - y) * RADAR_TILE_SIZE + WORLD_MIN_Y));
	out.x /= RADAR_TILE_SIZE;
	out.y /= RADAR_TILE_SIZE;
}
#endif

#if 0
WRAPPER void CRadar::TransformRadarPointToRealWorldSpace(CVector2D &out, const CVector2D &in) { EAXJMP(0x4A5300); }
#else
void CRadar::TransformRadarPointToRealWorldSpace(CVector2D &out, const CVector2D &in)
{
	float s, c;

	s = -sin(TheCamera.GetForward().Heading());
	c = cos(TheCamera.GetForward().Heading());

	if (TheCamera.Cams[TheCamera.ActiveCam].Mode == CCam::MODE_TOPDOWN1 || TheCamera.Cams[TheCamera.ActiveCam].Mode == CCam::MODE_TOPDOWNPED) {
		s = 0.0f;
		c = 1.0f;
	}
	else if (TheCamera.GetLookDirection() != LOOKING_FORWARD) {
		CVector forward;

		if (TheCamera.Cams[TheCamera.ActiveCam].Mode == CCam::MODE_FIRSTPERSON) {
			forward = TheCamera.Cams[TheCamera.ActiveCam].CamTargetEntity->GetForward();
			forward.Normalise();	// a bit useless...
		}
		else
			forward = TheCamera.Cams[TheCamera.ActiveCam].CamTargetEntity->GetPosition() - TheCamera.Cams[TheCamera.ActiveCam].SourceBeforeLookBehind;

		s = -sin(forward.Heading());
		c = cos(forward.Heading());
	}

	out.x = s * in.y + c * in.x;
	out.y = c * in.y - s * in.x;

	out = out * m_RadarRange + vec2DRadarOrigin;
}
#endif

// Radar space goes from -1.0 to 1.0 in x and y, top right is (1.0, 1.0)
void CRadar::TransformRadarPointToScreenSpace(CVector2D &out, const CVector2D &in)
{
	// FIX: game doesn't scale RADAR_LEFT here
	out.x = (in.x + 1.0f)*0.5f*SCREEN_SCALE_X(RADAR_WIDTH) + SCREEN_SCALE_X(RADAR_LEFT);
	out.y = (1.0f - in.y)*0.5f*SCREEN_SCALE_Y(RADAR_HEIGHT) + SCREEN_SCALE_FROM_BOTTOM(RADAR_BOTTOM + RADAR_HEIGHT);
}

#if 0
WRAPPER void CRadar::TransformRealWorldPointToRadarSpace(CVector2D &out, const CVector2D &in) { EAXJMP(0x4A50D0); }
#else
void CRadar::TransformRealWorldPointToRadarSpace(CVector2D &out, const CVector2D &in)
{
	float s, c;
	if (TheCamera.Cams[TheCamera.ActiveCam].Mode == CCam::MODE_TOPDOWN1 || TheCamera.Cams[TheCamera.ActiveCam].Mode == CCam::MODE_TOPDOWNPED) {
		s = 0.0f;
		c = 1.0f;
	}
	else if (TheCamera.GetLookDirection() == LOOKING_FORWARD) {
		s = sin(TheCamera.GetForward().Heading());
		c = cos(TheCamera.GetForward().Heading());
	}
	else {
		CVector forward;

		if (TheCamera.Cams[TheCamera.ActiveCam].Mode == CCam::MODE_FIRSTPERSON) {
			forward = TheCamera.Cams[TheCamera.ActiveCam].CamTargetEntity->GetForward();
			forward.Normalise();	// a bit useless...
		}
		else
			forward = TheCamera.Cams[TheCamera.ActiveCam].CamTargetEntity->GetPosition() - TheCamera.Cams[TheCamera.ActiveCam].SourceBeforeLookBehind;

		s = sin(forward.Heading());
		c = cos(forward.Heading());
	}

	float x = (in.x - vec2DRadarOrigin.x) * (1.0f / m_RadarRange);
	float y = (in.y - vec2DRadarOrigin.y) * (1.0f / m_RadarRange);

	out.x = s * y + c * x;
	out.y = c * y - s * x;
}
#endif

#if 0
WRAPPER void CRadar::GetTextureCorners(int32 x, int32 y, CVector2D *out) { EAXJMP(0x4A61C0); };
#else
// Transform from section indices to world coordinates
void CRadar::GetTextureCorners(int32 x, int32 y, CVector2D *out)
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
#endif

#if 0
WRAPPER void CRadar::ClipRadarTileCoords(int32 &, int32 &) { EAXJMP(0x00); };
#else
void CRadar::ClipRadarTileCoords(int32 &x, int32 &y)
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
#endif


#if 0
WRAPPER bool CRadar::IsPointInsideRadar(const CVector2D &) { EAXJMP(0x4A6160); }
#else
bool CRadar::IsPointInsideRadar(const CVector2D &point)
{
	if (point.x < -1.0f || point.x > 1.0f) return false;
	if (point.y < -1.0f || point.y > 1.0f) return false;
	return true;
}
#endif

// clip line p1,p2 against (-1.0, 1.0) in x and y, set out to clipped point closest to p1
#if 0
WRAPPER int CRadar::LineRadarBoxCollision(CVector2D &, const CVector2D &, const CVector2D &) { EAXJMP(0x4A6250); }
#else
int CRadar::LineRadarBoxCollision(CVector2D &out, const CVector2D &p1, const CVector2D &p2)
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
#endif

STARTPATCHES
//	InjectHook(0x4A3EF0, CRadar::Initialise, PATCH_JUMP);
//	InjectHook(0x4A3F60, CRadar::Shutdown, PATCH_JUMP);
//	InjectHook(0x4A4030, CRadar::LoadTextures, PATCH_JUMP);
//	InjectHook(0x4A4180, CRadar::GetNewUniqueBlipIndex, PATCH_JUMP);
//	InjectHook(0x4A41C0, CRadar::GetActualBlipArrayIndex, PATCH_JUMP);
	InjectHook(0x4A4200, CRadar::DrawMap, PATCH_JUMP);
	InjectHook(0x4A42F0, CRadar::DrawBlips, PATCH_JUMP);
//	InjectHook(0x4A4C70, CRadar::Draw3dMarkers, PATCH_JUMP);
	InjectHook(0x4A4F30, CRadar::LimitRadarPoint, PATCH_JUMP);
	InjectHook(0x4A4F90, CRadar::CalculateBlipAlpha, PATCH_JUMP);
	InjectHook(0x4A5040, CRadar::TransformRadarPointToScreenSpace, PATCH_JUMP);
	InjectHook(0x4A50D0, CRadar::TransformRealWorldPointToRadarSpace, PATCH_JUMP);
	InjectHook(0x4A5300, CRadar::TransformRadarPointToRealWorldSpace, PATCH_JUMP);
	InjectHook(0x4A5530, CRadar::TransformRealWorldToTexCoordSpace, PATCH_JUMP);
//	InjectHook(0x4A5590, CRadar::SetCoordBlip, PATCH_JUMP);
//	InjectHook(0x4A5640, CRadar::SetEntityBlip, PATCH_JUMP);
	InjectHook(0x4A56C0, CRadar::ClearBlipForEntity, PATCH_JUMP);
//	InjectHook(0x4A5720, CRadar::ClearBlip, PATCH_JUMP);
//	InjectHook(0x4A5770, CRadar::ChangeBlipColour, PATCH_JUMP);
//	InjectHook(0x4A57A0, CRadar::ChangeBlipBrightness, PATCH_JUMP);
//	InjectHook(0x4A57E0, CRadar::ChangeBlipScale, PATCH_JUMP);
//	InjectHook(0x4A5810, CRadar::ChangeBlipDisplay, PATCH_JUMP);
//	InjectHook(0x4A5840, CRadar::SetBlipSprite, PATCH_JUMP);
	InjectHook(0x4A5870, CRadar::ShowRadarTrace, PATCH_JUMP);
	InjectHook(0x4A59C0, CRadar::ShowRadarMarker, PATCH_JUMP);
	//InjectHook(0x4A5BB0, CRadar::GetRadarTraceColour, PATCH_JUMP);
	InjectHook(0x4A5C60, CRadar::SetRadarMarkerState, PATCH_JUMP);
	InjectHook(0x4A5D10, CRadar::DrawRotatingRadarSprite, PATCH_JUMP);
	InjectHook(0x4A5EF0, CRadar::DrawRadarSprite, PATCH_JUMP);
//	InjectHook(0x4A60E0, CRadar::RemoveRadarSections, PATCH_JUMP);
//	InjectHook(0x4A6100, CRadar::StreamRadarSections, PATCH_JUMP);
	InjectHook(0x4A64A0, CRadar::ClipRadarPoly, PATCH_JUMP);
	InjectHook(0x4A67E0, CRadar::DrawRadarSection, PATCH_JUMP);
	InjectHook(0x4A69C0, CRadar::DrawRadarMask, PATCH_JUMP);
//	InjectHook(0x4A6B60, CRadar::StreamRadarSections, PATCH_JUMP);
	InjectHook(0x4A6C20, CRadar::DrawRadarMap, PATCH_JUMP);
//	InjectHook(0x4A6E30, CRadar::SaveAllRadarBlips, PATCH_JUMP);
//	InjectHook(0x4A6F30, CRadar::LoadAllRadarBlips, PATCH_JUMP);

	InjectHook(0x4A61C0, CRadar::GetTextureCorners, PATCH_JUMP);
	InjectHook(0x4A6160, CRadar::IsPointInsideRadar, PATCH_JUMP);
	InjectHook(0x4A6250, CRadar::LineRadarBoxCollision, PATCH_JUMP);
ENDPATCHES
