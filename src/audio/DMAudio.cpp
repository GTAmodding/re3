#include "common.h"
#include "patcher.h"
#include "DMAudio.h"

cDMAudio &DMAudio = *(cDMAudio*)0x95CDBE;

WRAPPER void cDMAudio::Terminate(void) { EAXJMP(0x57C780); }
WRAPPER void cDMAudio::ReleaseDigitalHandle(void) { EAXJMP(0x57CA10); }
WRAPPER void cDMAudio::ReacquireDigitalHandle(void) { EAXJMP(0x57CA30); }

WRAPPER void cDMAudio::Service(void) { EAXJMP(0x57C7A0); }
WRAPPER void cDMAudio::ReportCollision(CEntity *A, CEntity *B, uint8 surfA, uint8 surfB, float impulse, float speed) { EAXJMP(0x57CBE0); }
WRAPPER void cDMAudio::ResetTimers(UInt32 timerval) { EAXJMP(0x57CCD0); }
WRAPPER Bool cDMAudio::IsAudioInitialised() { EAXJMP(0x57CAB0); }
WRAPPER Char cDMAudio::GetCDAudioDriveLetter() { EAXJMP(0x57CA90); }
WRAPPER Bool cDMAudio::CheckForAnAudioFileOnCD() { EAXJMP(0x57CA70); }
WRAPPER void cDMAudio::ChangeMusicMode(UInt8 mode) { EAXJMP(0x57CCF0); }
WRAPPER void cDMAudio::PlayFrontEndSound(int16 m_dwID, float fVolume) { EAXJMP(0x57CC20); }
