#include "common.h"
#include "patcher.h"
#include "SceneEdit.h"

int32 &CSceneEdit::m_bCameraFollowActor = *(int*)0x940590;
bool &CSceneEdit::m_bRecording = *(bool*)0x95CD1F;
CVector &CSceneEdit::m_vecCurrentPosition = *(CVector*)0x943064;
CVector &CSceneEdit::m_vecCamHeading = *(CVector*)0x942F8C;

WRAPPER void CSceneEdit::Update(void) { EAXJMP(0x585570); }
WRAPPER void CSceneEdit::Init(void) { EAXJMP(0x585170); }
