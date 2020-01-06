#include "common.h"
#include "patcher.h"

#include "DMAudio.h"
#include "Hud.h"
#include "Replay.h"
#include "Timer.h"
#include "Script.h"
#include "User.h"

CPlaceName& CUserDisplay::PlaceName = *(CPlaceName*)0x8F29BC;
COnscreenTimer& CUserDisplay::OnscnTimer = *(COnscreenTimer*)0x862238;
CPager& CUserDisplay::Pager = *(CPager*)0x8F2744;
CCurrentVehicle& CUserDisplay::CurrentVehicle = *(CCurrentVehicle*)0x8F5FE8;

WRAPPER void CUserDisplay::Process(void) { EAXJMP(0x4AD690); }

void COnscreenTimer::Init() {
	m_bDisabled = false;
	for(uint32 i = 0; i < NUMONSCREENTIMERENTRIES; i++) {
		m_sEntries[i].m_nTimerOffset = 0;
		m_sEntries[i].m_nCounterOffset = 0;

		for(uint32 j = 0; j < 10; j++) {
			m_sEntries[i].m_aTimerText[j] = 0;
			m_sEntries[i].m_aCounterText[j] = 0;
		}

		m_sEntries[i].m_nType = 0;
		m_sEntries[i].m_bTimerProcessed = 0;
		m_sEntries[i].m_bCounterProcessed = 0;
	}
}

void COnscreenTimer::Process() {
	if(!CReplay::IsPlayingBack() && !m_bDisabled) {
		for(uint32 i = 0; i < NUMONSCREENTIMERENTRIES; i++) {
			m_sEntries[i].Process();
		}
	}
}

void COnscreenTimer::ProcessForDisplay() {
	if(CHud::m_Wants_To_Draw_Hud) {
		m_bProcessed = false;
		for(uint32 i = 0; i < NUMONSCREENTIMERENTRIES; i++) {
			if(m_sEntries[i].ProcessForDisplay()) {
				m_bProcessed = true;
			}
		}
	}
}

void COnscreenTimer::ClearCounter(uint32 offset) {
	for(uint32 i = 0; i < NUMONSCREENTIMERENTRIES; i++) {
		if(offset == m_sEntries[i].m_nCounterOffset) {
			m_sEntries[i].m_nCounterOffset = 0;
			m_sEntries[i].m_aCounterText[0] = 0;
			m_sEntries[i].m_nType = 0;
			m_sEntries[i].m_bCounterProcessed = 0;
		}
	}
}

void COnscreenTimer::ClearClock(uint32 offset) {
	for(uint32 i = 0; i < NUMONSCREENTIMERENTRIES; i++) {
		if(offset == m_sEntries[i].m_nTimerOffset) {
			m_sEntries[i].m_nTimerOffset = 0;
			m_sEntries[i].m_aTimerText[0] = 0;
			m_sEntries[i].m_bTimerProcessed = 0;
		}
	}
}

void COnscreenTimer::AddCounter(uint32 offset, uint16 type, char* text) {
	uint32 i = 0;
	for(uint32 i = 0; i < NUMONSCREENTIMERENTRIES; i++) {
		if(m_sEntries[i].m_nCounterOffset == 0) {
			break;
		}
		return;
	}

	m_sEntries[i].m_nCounterOffset = offset;
	if(text) {
		strncpy(m_sEntries[i].m_aCounterText, text, 10);
	} else {
		m_sEntries[i].m_aCounterText[0] = 0;
	}

	m_sEntries[i].m_nType = type;
}

void COnscreenTimer::AddClock(uint32 offset, char* text) {
	uint32 i = 0;
	for(uint32 i = 0; i < NUMONSCREENTIMERENTRIES; i++) {
		if(m_sEntries[i].m_nTimerOffset == 0) {
			break;
		}
		return;
	}

	m_sEntries[i].m_nTimerOffset = offset;
	if(text) {
		strncpy(m_sEntries[i].m_aTimerText, text, 10);
	} else {
		m_sEntries[i].m_aTimerText[0] = 0;
	}
}

void COnscreenTimerEntry::Process() {
	if(m_nTimerOffset == 0) {
		return;
	}

	uint32* timerPtr = (uint32*)&CTheScripts::ScriptSpace[m_nTimerOffset];
	uint32 oldTime = *timerPtr;
	int32 newTime = int32(oldTime - uint32(20.0f * CTimer::GetTimeStep()));
	if(newTime < 0) {
		*timerPtr = 0;
		m_bTimerProcessed = 0;
		m_nTimerOffset = 0;
		m_aTimerText[0] = 0;
	} else {
		*timerPtr = (uint32)newTime;
		uint32 oldTimeSeconds = oldTime / 1000;
		if(oldTimeSeconds <= 11 && newTime / 1000 != oldTimeSeconds) {
			DMAudio.PlayFrontEndSound(SOUND_CLOCK_TICK, newTime / 1000);
		}
	}
}

bool COnscreenTimerEntry::ProcessForDisplay() {
	m_bTimerProcessed = false;
	m_bCounterProcessed = false;

	if(m_nTimerOffset == 0 && m_nCounterOffset == 0) {
		return false;
	}

	if(m_nTimerOffset != 0) {
		m_bTimerProcessed = true;
		ProcessForDisplayClock();
	}

	if(m_nCounterOffset != 0) {
		m_bCounterProcessed = true;
		ProcessForDisplayCounter();
	}
	return true;
}

void COnscreenTimerEntry::ProcessForDisplayClock() {
	uint32 time = *(uint32*)&CTheScripts::ScriptSpace[m_nTimerOffset];
	sprintf(m_bTimerBuffer, "%02d:%02d", time / 1000 / 60,
				   time / 1000 % 60);
}

void COnscreenTimerEntry::ProcessForDisplayCounter() {
	uint32 counter = *(uint32*)&CTheScripts::ScriptSpace[m_nCounterOffset];

	assert(!m_nType || counter <= 100);

	sprintf(m_bCounterBuffer, "%d", counter);
}

STARTPATCHES
	InjectHook(0x429160, &COnscreenTimerEntry::Process, PATCH_JUMP);
	InjectHook(0x429110, &COnscreenTimerEntry::ProcessForDisplay, PATCH_JUMP);
	InjectHook(0x429080, &COnscreenTimerEntry::ProcessForDisplayClock, PATCH_JUMP);
	InjectHook(0x4290F0, &COnscreenTimerEntry::ProcessForDisplayCounter, PATCH_JUMP);

	InjectHook(0x429220, &COnscreenTimer::Init, PATCH_JUMP);
	InjectHook(0x429320, &COnscreenTimer::Process, PATCH_JUMP);
	InjectHook(0x4292E0, &COnscreenTimer::ProcessForDisplay, PATCH_JUMP);
	InjectHook(0x429450, &COnscreenTimer::ClearCounter, PATCH_JUMP);
	InjectHook(0x429410, &COnscreenTimer::ClearClock, PATCH_JUMP);
	InjectHook(0x4293B0, &COnscreenTimer::AddCounter, PATCH_JUMP);
	InjectHook(0x429350, &COnscreenTimer::AddClock, PATCH_JUMP);
ENDPATCHES
