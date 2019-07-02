#include "common.h"
#include "patcher.h"
#include "DMAudio.h"

cDMAudio &DMAudio = *(cDMAudio*)0x95CDBE;

WRAPPER void cDMAudio::Terminate(void) { EAXJMP(0x57C780); }
WRAPPER void cDMAudio::ReleaseDigitalHandle(void) { EAXJMP(0x57CA10); }
WRAPPER void cDMAudio::ReacquireDigitalHandle(void) { EAXJMP(0x57CA30); }

WRAPPER void cDMAudio::Service(void) { EAXJMP(0x57C7A0); }
WRAPPER void cDMAudio::ReportCollision(CEntity *A, CEntity *B, uint8 surfA, uint8 surfB, float impulse, float speed) { EAXJMP(0x57CBE0); }
WRAPPER void cDMAudio::ResetTimers(uint32 timerval) { EAXJMP(0x57CCD0); }
WRAPPER bool cDMAudio::IsAudioInitialised() { EAXJMP(0x57CAB0); }
WRAPPER char cDMAudio::GetCDAudioDriveLetter() { EAXJMP(0x57CA90); }
WRAPPER bool cDMAudio::CheckForAnAudioFileOnCD() { EAXJMP(0x57CA70); }
WRAPPER void cDMAudio::ChangeMusicMode(uint8 mode) { EAXJMP(0x57CCF0); }

WRAPPER void cDMAudio::PlayFrontEndSound(uint32, uint32) { EAXJMP(0x57CC20); }
WRAPPER void cDMAudio::PlayFrontEndTrack(uint32, uint32) { EAXJMP(0x57CC80); }
WRAPPER void cDMAudio::StopFrontEndTrack() { EAXJMP(0x57CCB0); }
WRAPPER void cDMAudio::PlayOneShot(int32, uint16, float) { EAXJMP(0x57C840); }
WRAPPER void cDMAudio::SetMusicMasterVolume(int8) { EAXJMP(0x57C8C0); }
WRAPPER void cDMAudio::SetEffectsMasterVolume(int8) { EAXJMP(0x57C890); }
WRAPPER int8 cDMAudio::SetCurrent3DProvider(int8) { EAXJMP(0x57C9B0); }
WRAPPER int32 cDMAudio::SetSpeakerConfig(int32) { EAXJMP(0x57C9D0); }

WRAPPER int32 cDMAudio::GetRadioInCar() { EAXJMP(0x57CE40); }
WRAPPER uint8 cDMAudio::IsMP3RadioChannelAvailable() { EAXJMP(0x57C9F0); }
WRAPPER void cDMAudio::SetEffectsFadeVol(uint8) { EAXJMP(0x57C8F0); }
WRAPPER void cDMAudio::SetMusicFadeVol(uint8) { EAXJMP(0x57C920); }
WRAPPER int32 cDMAudio::CreateEntity(int, void*) { EAXJMP(0x57C7C0); }
WRAPPER void cDMAudio::SetEntityStatus(int32 id, uint8 enable) { EAXJMP(0x57C810); }
WRAPPER void cDMAudio::SetRadioInCar(int32) { EAXJMP(0x57CE60); }
WRAPPER void cDMAudio::DestroyEntity(int32) { EAXJMP(0x57C7F0); }
