#include "common.h"
#include "patcher.h"
#include "PlayerSkin.h"

WRAPPER void CPlayerSkin::BeginFrontEndSkinEdit() { EAXJMP(0x59BC70); }
