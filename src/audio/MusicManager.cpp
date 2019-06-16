#include "common.h"
#include "patcher.h"
#include "MusicManager.h"
#include "Font.h"
#include "Hud.h"
#include "Text.h"
#include "Timer.h"
#include "Camera.h"
#include "World.h"
#include "ModelIndices.h"
#include "SampleManager.h"

cMusicManager &MusicManager = *(cMusicManager*)0x8F3964;
int &gNumRetunePresses = *(int*)0x650B80;
int &gRetuneCounter = *(int*)0x650B84;

//WRAPPER void cMusicManager::DisplayRadioStationName(void) { EAXJMP(0x57E6D0); }

bool cMusicManager::PlayerInCar() {
	if (!FindPlayerVehicle())
		return false;
	else {
		int32 State = CWorld::Players[CWorld::PlayerInFocus].m_pPed->m_nPedState;

		if (State == PED_DRAG_FROM_CAR || State == PED_EXIT_CAR || State == PED_ARRESTED)
			return false;

		int16 Model = FindPlayerVehicle()->m_modelIndex;
		switch (FindPlayerVehicle()->m_modelIndex) {
		case MI_FIRETRUCK:
		case MI_AMBULAN:
		case MI_MRWHOOP:
		case MI_PREDATOR:
		case MI_TRAIN:
		case MI_SPEEDER:
		case MI_REEFER:
		case MI_GHOST:
			return false;
		default:
			return true;
		}
	}
}

void cMusicManager::DisplayRadioStationName() {
	wchar* RadioName = nullptr;
	uint32 RadioStation = gNumRetunePresses + MusicManager.m_nCurrentStreamedSound;

	switch (RadioStation) {
	case HEAD_RADIO:
		RadioName = TheText.Get("FEA_FM0");
		break;
	case DOUBLE_CLEF:
		RadioName = TheText.Get("FEA_FM1");
		break;
	case JAH_RADIO:
		RadioName = TheText.Get("FEA_FM2");
		break;
	case RISE_FM:
		RadioName = TheText.Get("FEA_FM3");
		break;
	case LIPS_106:
		RadioName = TheText.Get("FEA_FM4");
		break;
	case GAME_FM:
		RadioName = TheText.Get("FEA_FM5");
		break;
	case MSX_FM:
		RadioName = TheText.Get("FEA_FM6");
		break;
	case FLASHBACK:
		RadioName = TheText.Get("FEA_FM7");
		break;
	case CHATTERBOX:
		RadioName = TheText.Get("FEA_FM8");
		break;
	case USERTRACK:
		RadioName = TheText.Get("FEA_FM9");
		break;
	default:
		RadioName = nullptr;
		break;
	}

	CFont::SetJustifyOff();
	CFont::SetBackgroundOff();
	CFont::SetScale(SCREEN_STRETCH_X(0.8f), SCREEN_STRETCH_Y(1.35f));
	CFont::SetPropOn();
	CFont::SetFontStyle(FONT_HEADING);
	CFont::SetCentreOn();
	CFont::SetCentreSize(SCREEN_STRETCH_X(640.0f));;

	static int32 nTime = 0;
	if (!CTimer::GetIsPaused() && !TheCamera.m_WideScreenOn && MusicManager.PlayerInCar()) {
		if (gNumRetunePresses || (MusicManager.m_bPlayerInCar && !MusicManager.m_bPreviousPlayerInCar))
			nTime = CTimer::GetTimeInMilliseconds() + 2000;

		if (CTimer::GetTimeInMilliseconds() < nTime) {
			if (RadioStation) {
				if (MusicManager.m_nCurrentStreamedSound == STREAMED_SOUND_WATER_AMBIENT ||
					MusicManager.m_nCurrentStreamedSound == STREAMED_SOUND_RADIO_POLICE ||
					MusicManager.m_nCurrentStreamedSound == STREAMED_SOUND_WATER_AMBIENT ||
					MusicManager.m_nCurrentStreamedSound == STREAMED_SOUND_CITY_AMBIENT)
					RadioStation = RADIO_OFF;
				else {
					if (RadioStation > HEAD_RADIO) {
						if (cSampleManager.IsMP3RadioChannelAvailable()) {
							if (RadioStation > USERTRACK)
								RadioStation = RADIO_OFF;
						}
						else {
							if (RadioStation > CHATTERBOX)
								RadioStation = RADIO_OFF;
						}
					}
					else {
						RadioStation = RADIO_OFF;
					}
				}
			}

			if (RadioName) {
				CFont::SetColor(CRGBA(0, 0, 0, 255));

				CFont::PrintString(SCREEN_WIDTH / 2, SCREEN_STRETCH_Y(23.0f), RadioName);

				if (gNumRetunePresses)
					CFont::SetColor(CRGBA(102, 133, 143, 255));
				else
					CFont::SetColor(CRGBA(147, 196, 211, 255));

				CFont::PrintString(SCREEN_WIDTH / 2, SCREEN_STRETCH_Y(22.0f), RadioName);
				CFont::DrawFonts();
			}
		}
	}
}