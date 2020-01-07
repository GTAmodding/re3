#include "common.h"
#include "patcher.h"
#include "Record.h"

uint16 &CRecordDataForGame::RecordingState = *(uint16*)0x95CC24;

uint8 &CRecordDataForChase::Status = *(uint8*)0x95CDCE;

WRAPPER void CRecordDataForGame::SaveOrRetrieveDataForThisFrame(void) { EAXJMP(0x4341F0); }

WRAPPER void CRecordDataForChase::SaveOrRetrieveDataForThisFrame(void) { EAXJMP(0x4347F0); }
WRAPPER void CRecordDataForChase::ProcessControlCars(void) { EAXJMP(0x435540); }
WRAPPER void CRecordDataForChase::SaveOrRetrieveCarPositions(void) { EAXJMP(0x434B20); }
WRAPPER void CRecordDataForChase::StartChaseScene(float) { EAXJMP(0x435690); }
WRAPPER void CRecordDataForChase::CleanUpChaseScene() { EAXJMP(0x4357C0); }
