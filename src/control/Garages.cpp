#include "common.h"
#include "patcher.h"
#include "ModelIndices.h"
#include "Garages.h"
#include "Timer.h"
#include "Font.h"
#include "Messages.h"
#include "Text.h"

//WRAPPER void CGarages::PrintMessages(void) { EAXJMP(0x426310); }

int32 &CGarages::BankVansCollected = *(int32 *)0x8F1B34;
bool &CGarages::BombsAreFree = *(bool *)0x95CD7A;
bool &CGarages::RespraysAreFree = *(bool *)0x95CD1D;
int32 &CGarages::CarsCollected = *(int32 *)0x880E18;
int32 &CGarages::CarTypesCollected = *(int32 *)0x8E286C;
int32 &CGarages::CrushedCarId = *(int32 *)0x943060;
uint32 &CGarages::LastTimeHelpMessage = *(uint32 *)0x8F1B58;
int32 &CGarages::MessageNumberInString = *(int32 *)0x885BA8;
const char *CGarages::MessageIDString = (const char *)0x878358;
int32 &CGarages::MessageNumberInString2 = *(int32 *)0x8E2C14;
uint32 &CGarages::MessageStartTime = *(uint32 *)0x8F2530;
uint32 &CGarages::MessageEndTime = *(uint32 *)0x8F597C;
uint32 &CGarages::NumGarages = *(uint32 *)0x8F29F4;
bool &CGarages::PlayerInGarage = *(bool *)0x95CD83;
int32 &CGarages::PoliceCarsCollected = *(int32 *)0x941444;
uint32 &CGarages::GarageToBeTidied = *(uint32 *)0x623570;

bool
CGarages::IsModelIndexADoor(uint32 id)
{
	return id == MI_GARAGEDOOR1 ||
		id == MI_GARAGEDOOR2 ||
		id == MI_GARAGEDOOR3 ||
		id == MI_GARAGEDOOR4 ||
		id == MI_GARAGEDOOR5 ||
		id == MI_GARAGEDOOR6 ||
		id == MI_GARAGEDOOR7 ||
		id == MI_GARAGEDOOR9 ||
		id == MI_GARAGEDOOR10 ||
		id == MI_GARAGEDOOR11 ||
		id == MI_GARAGEDOOR12 ||
		id == MI_GARAGEDOOR13 ||
		id == MI_GARAGEDOOR14 ||
		id == MI_GARAGEDOOR15 ||
		id == MI_GARAGEDOOR16 ||
		id == MI_GARAGEDOOR17 ||
		id == MI_GARAGEDOOR18 ||
		id == MI_GARAGEDOOR19 ||
		id == MI_GARAGEDOOR20 ||
		id == MI_GARAGEDOOR21 ||
		id == MI_GARAGEDOOR22 ||
		id == MI_GARAGEDOOR23 ||
		id == MI_GARAGEDOOR24 ||
		id == MI_GARAGEDOOR25 ||
		id == MI_GARAGEDOOR26 ||
		id == MI_GARAGEDOOR27 ||
		id == MI_GARAGEDOOR28 ||
		id == MI_GARAGEDOOR29 ||
		id == MI_GARAGEDOOR30 ||
		id == MI_GARAGEDOOR31 ||
		id == MI_GARAGEDOOR32 ||
		id == MI_CRUSHERBODY ||
		id == MI_CRUSHERLID;
}

void CGarages::PrintMessages()
{
	if (CTimer::GetTimeInMilliseconds() > CGarages::MessageStartTime && CTimer::GetTimeInMilliseconds() < CGarages::MessageEndTime) {
		CFont::SetScale(HUD_STRETCH_X(1.2f / 2), HUD_STRETCH_Y(1.5f / 2)); // BUG: game doesn't use macro here.
		CFont::SetPropOn();
		CFont::SetJustifyOff();
		CFont::SetBackgroundOff();
		CFont::SetCentreSize(SCREEN_FROM_RIGHT(50.0f));
		CFont::SetCentreOn();
		CFont::SetFontStyle(FONT_BANK);

		if (CGarages::MessageNumberInString2 < 0) {
			if (CGarages::MessageNumberInString < 0) {
				CFont::SetColor(CRGBA(0, 0, 0, 255));
				CFont::PrintString((SCREEN_WIDTH/ 2) + SCREEN_STRETCH_X(2.0f), (SCREEN_HEIGHT / 2) + HUD_STRETCH_Y(-84.0f), TheText.Get(CGarages::MessageIDString));

				CFont::SetColor(CRGBA(89, 115, 150, 255));
				CFont::PrintString((SCREEN_WIDTH / 2), (SCREEN_HEIGHT / 2) + HUD_STRETCH_Y(-84.0f), TheText.Get(CGarages::MessageIDString));
			}
			else {
				CMessages::InsertNumberInString(TheText.Get(CGarages::MessageIDString), CGarages::MessageNumberInString, -1, -1, -1, -1, -1, gUString);

				CFont::SetColor(CRGBA(0, 0, 0, 255));
				CFont::PrintString((SCREEN_WIDTH / 2) + SCREEN_STRETCH_X(2.0f), (SCREEN_HEIGHT / 2) + HUD_STRETCH_Y(-84.0f + 2.0 - 40.0f), gUString);

				CFont::SetColor(CRGBA(89, 115, 150, 255));
				CFont::PrintString((SCREEN_WIDTH / 2), (SCREEN_HEIGHT / 2) + HUD_STRETCH_Y(-84.0f - 40.0f), gUString);
			}
		}
		else {
			CMessages::InsertNumberInString(TheText.Get(CGarages::MessageIDString), CGarages::MessageNumberInString2, -1, -1, -1, -1, -1, gUString);

			CFont::SetColor(CRGBA(0, 0, 0, 255));
			CFont::PrintString((SCREEN_WIDTH / 2) + SCREEN_STRETCH_X(2.0f), (SCREEN_HEIGHT / 2) + HUD_STRETCH_Y(-84.0f + 2.0 - 40.0f), gUString);

			CFont::SetColor(CRGBA(89, 115, 150, 255));
			CFont::PrintString((SCREEN_WIDTH / 2), (SCREEN_HEIGHT / 2) + HUD_STRETCH_Y(-84.0f - 40.0f), gUString);
		}
	}
}