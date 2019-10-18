#include "common.h"
#include "patcher.h"
#include "SceneEdit.h"

WRAPPER void CSceneEdit::Update(void) { EAXJMP(0x585570); }
