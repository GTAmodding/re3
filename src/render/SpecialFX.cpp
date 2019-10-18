#include "common.h"
#include "patcher.h"
#include "SpecialFX.h"
#include "Timer.h"
#include "Sprite.h"
#include "Font.h"
#include "Text.h"

WRAPPER void CSpecialFX::Render(void) { EAXJMP(0x518DC0); }
WRAPPER void CSpecialFX::Update(void) { EAXJMP(0x518D40); }

WRAPPER void CMotionBlurStreaks::RegisterStreak(int32 id, uint8 r, uint8 g, uint8 b, CVector p1, CVector p2) { EAXJMP(0x519460); }


CBulletTrace (&CBulletTraces::aTraces)[16] = *(CBulletTrace(*)[16])*(uintptr*)0x72B1B8;

WRAPPER void CBulletTraces::Init(void) { EAXJMP(0x518DE0); }

WRAPPER void CBrightLights::RegisterOne(CVector pos, CVector up, CVector right, CVector fwd, uint8 type, uint8 unk1, uint8 unk2, uint8 unk3) { EAXJMP(0x51A410); }

WRAPPER void C3dMarkers::PlaceMarkerSet(uint32 id, uint16 type, CVector& pos, float size, uint8 r, uint8 g, uint8 b, uint8 a, uint16 pulsePeriod, float pulseFraction, int16 rotateRate)  { EAXJMP(0x51BB80); }



#define MONEY_MESSAGE_LIFETIME_MS 2000

CMoneyMessage CMoneyMessages::aMoneyMessages[NUMMONEYMESSAGES];

void
CMoneyMessage::Render()
{
	const float MAX_SCALE = 4.0f;
	uint32 nLifeTime = CTimer::GetTimeInMilliseconds() - m_nTimeRegistered;
	if (nLifeTime >= MONEY_MESSAGE_LIFETIME_MS) m_nTimeRegistered = 0;
	else {
		float fLifeTime = (float)nLifeTime / MONEY_MESSAGE_LIFETIME_MS;
		RwV3d vecOut;
		float fDistX, fDistY;
		if (CSprite::CalcScreenCoors(m_vecPosition + CVector(0.0f, 0.0f, fLifeTime), &vecOut, &fDistX, &fDistY, true)) {
			fDistX *= (0.7 * fLifeTime + 2.0) * m_fSize;
			fDistY *= (0.7 * fLifeTime + 2.0) * m_fSize;
			CFont::SetPropOn();
			CFont::SetBackgroundOff();

			float fScaleY = fDistY / 100.0f;
			if (fScaleY > MAX_SCALE) fScaleY = MAX_SCALE;

			float fScaleX = fDistX / 100.0f;
			if (fScaleX > MAX_SCALE) fScaleX = MAX_SCALE;

			CFont::SetScale(fScaleX, fScaleY); // maybe use SCREEN_SCALE_X and SCREEN_SCALE_Y here?
			CFont::SetCentreOn();
			CFont::SetCentreSize(SCREEN_WIDTH);
			CFont::SetJustifyOff();
			CFont::SetColor(CRGBA(m_Colour.r, m_Colour.g, m_Colour.b, (255.0f - 255.0f * fLifeTime) * m_fOpacity));
			CFont::SetBackGroundOnlyTextOff();
			CFont::SetFontStyle(FONT_BANK);
			CFont::PrintString(vecOut.x, vecOut.y, m_aText);
		}
	}
}

void
CMoneyMessages::Init()
{
	for (int32 i = 0; i < NUMMONEYMESSAGES; i++)
		aMoneyMessages[i].m_nTimeRegistered = 0;
}

void
CMoneyMessages::Render()
{
	for (int32 i = 0; i < NUMMONEYMESSAGES; i++) {
		if (aMoneyMessages[i].m_nTimeRegistered != 0)
			aMoneyMessages[i].Render();
	}
}

void
CMoneyMessages::RegisterOne(CVector vecPos, const char *pText, uint8 bRed, uint8 bGreen, uint8 bBlue, float fSize, float fOpacity)
{
	uint32 nIndex = 0;
	while (aMoneyMessages[nIndex].m_nTimeRegistered != 0) {
		if (++nIndex >= NUMMONEYMESSAGES) return;
	}

	// Add data of this money message to the array
	AsciiToUnicode(pText, aMoneyMessages[nIndex].m_aText);

	aMoneyMessages[nIndex].m_nTimeRegistered = CTimer::GetTimeInMilliseconds();
	aMoneyMessages[nIndex].m_vecPosition = vecPos;
	aMoneyMessages[nIndex].m_Colour.red = bRed;
	aMoneyMessages[nIndex].m_Colour.green = bGreen;
	aMoneyMessages[nIndex].m_Colour.blue = bBlue;
	aMoneyMessages[nIndex].m_fSize = fSize;
	aMoneyMessages[nIndex].m_fOpacity = fOpacity;
}

STARTPATCHES
	InjectHook(0x51AF70, CMoneyMessages::Init, PATCH_JUMP);
	InjectHook(0x51B030, CMoneyMessages::Render, PATCH_JUMP);
ENDPATCHES
