#include "common.h"

#include "Script.h"
#include "ScriptCommands.h"

#include "CarCtrl.h"
#include "BulletInfo.h"
#include "General.h"
#include "Lines.h"
#include "Messages.h"
#include "Pad.h"
#include "Pools.h"
#include "Population.h"
#include "RpAnimBlend.h"
#include "Shadows.h"
#include "SpecialFX.h"
#include "World.h"
#include "main.h"

void CRunningScript::UpdateCompareFlag(bool flag)
{
	if (m_bNotFlag)
		flag = !flag;
	if (m_nAndOrState == ANDOR_NONE) {
		m_bCondResult = flag;
		return;
	}
	if (m_nAndOrState >= ANDS_1 && m_nAndOrState <= ANDS_8) {
		m_bCondResult &= flag;
		if (m_nAndOrState == ANDS_1) {
			m_nAndOrState = ANDOR_NONE;
			return;
		}
	}
	else if (m_nAndOrState >= ORS_1 && m_nAndOrState <= ORS_8) {
		m_bCondResult |= flag;
		if (m_nAndOrState == ORS_1) {
			m_nAndOrState = ANDOR_NONE;
			return;
		}
	}
	else {
		return;
	}
	m_nAndOrState--;
}

void CRunningScript::LocatePlayerCommand(int32 command, uint32* pIp)
{
	bool b3D, result, debug, decided = false;
	float X, Y, Z, dX, dY, dZ;
	switch (command) {
	case COMMAND_LOCATE_PLAYER_ANY_MEANS_3D:
	case COMMAND_LOCATE_PLAYER_ON_FOOT_3D:
	case COMMAND_LOCATE_PLAYER_IN_CAR_3D:
	case COMMAND_LOCATE_STOPPED_PLAYER_ANY_MEANS_3D:
	case COMMAND_LOCATE_STOPPED_PLAYER_ON_FOOT_3D:
	case COMMAND_LOCATE_STOPPED_PLAYER_IN_CAR_3D:
		b3D = true;
		break;
	default:
		b3D = false;
		break;
	}
	CollectParameters(pIp, b3D ? 8 : 6);
	CPlayerInfo* pPlayerInfo = &CWorld::Players[ScriptParams[0]];
	switch (command) {
	case COMMAND_LOCATE_STOPPED_PLAYER_ANY_MEANS_2D:
	case COMMAND_LOCATE_STOPPED_PLAYER_ANY_MEANS_3D:
	case COMMAND_LOCATE_STOPPED_PLAYER_IN_CAR_2D:
	case COMMAND_LOCATE_STOPPED_PLAYER_IN_CAR_3D:
	case COMMAND_LOCATE_STOPPED_PLAYER_ON_FOOT_2D:
	case COMMAND_LOCATE_STOPPED_PLAYER_ON_FOOT_3D:
		if (!CTheScripts::IsPlayerStopped(pPlayerInfo)) {
			result = false;
			decided = true;
		}
		break;
	default:
		break;
	}
	X = *(float*)&ScriptParams[1];
	Y = *(float*)&ScriptParams[2];
	if (b3D) {
		Z = *(float*)&ScriptParams[3];
		dX = *(float*)&ScriptParams[4];
		dY = *(float*)&ScriptParams[5];
		dZ = *(float*)&ScriptParams[6];
		debug = ScriptParams[7];
	} else {
		dX = *(float*)&ScriptParams[3];
		dY = *(float*)&ScriptParams[4];
		debug = ScriptParams[5];
	}
	if (!decided) {
		CVector pos = pPlayerInfo->GetPos();
		result = false;
		bool in_area;
		if (b3D) {
			in_area = X - dX <= pos.x &&
				X + dX >= pos.x &&
				Y - dY <= pos.y &&
				Y + dY >= pos.y &&
				Z - dZ <= pos.z &&
				Z + dZ >= pos.z;
		} else {
			in_area = X - dX <= pos.x &&
				X + dX >= pos.x &&
				Y - dY <= pos.y &&
				Y + dY >= pos.y;
		}
		if (in_area) {
			switch (command) {
			case COMMAND_LOCATE_PLAYER_ANY_MEANS_2D:
			case COMMAND_LOCATE_PLAYER_ANY_MEANS_3D:
			case COMMAND_LOCATE_STOPPED_PLAYER_ANY_MEANS_2D:
			case COMMAND_LOCATE_STOPPED_PLAYER_ANY_MEANS_3D:
				result = true;
				break;
			case COMMAND_LOCATE_PLAYER_ON_FOOT_2D:
			case COMMAND_LOCATE_PLAYER_ON_FOOT_3D:
			case COMMAND_LOCATE_STOPPED_PLAYER_ON_FOOT_2D:
			case COMMAND_LOCATE_STOPPED_PLAYER_ON_FOOT_3D:
				result = !pPlayerInfo->m_pPed->bInVehicle;
				break;
			case COMMAND_LOCATE_PLAYER_IN_CAR_2D:
			case COMMAND_LOCATE_PLAYER_IN_CAR_3D:
			case COMMAND_LOCATE_STOPPED_PLAYER_IN_CAR_2D:
			case COMMAND_LOCATE_STOPPED_PLAYER_IN_CAR_3D:
				result = pPlayerInfo->m_pPed->bInVehicle;
				break;
			default:
				script_assert(false);
				break;
			}
		}
	}
	UpdateCompareFlag(result);
	if (debug)
		CTheScripts::HighlightImportantArea((uintptr)this + m_nIp, X - dX, Y - dY, X + dX, Y + dY, b3D ? Z : MAP_Z_LOW_LIMIT);
	if (CTheScripts::DbgFlag) {
		if (b3D)
			CTheScripts::DrawDebugCube(X - dX, Y - dY, Z - dZ, X + dX, Y + dY, Z + dZ);
		else
			CTheScripts::DrawDebugSquare(X - dX, Y - dY, X + dX, Y + dY);
	}
}

void CRunningScript::LocatePlayerCharCommand(int32 command, uint32* pIp)
{
	bool b3D, result, debug;
	float X, Y, Z, dX, dY, dZ;
	switch (command) {
	case COMMAND_LOCATE_PLAYER_ANY_MEANS_CHAR_3D:
	case COMMAND_LOCATE_PLAYER_ON_FOOT_CHAR_3D:
	case COMMAND_LOCATE_PLAYER_IN_CAR_CHAR_3D:
		b3D = true;
		break;
	default:
		b3D = false;
		break;
	}
	CollectParameters(pIp, b3D ? 6 : 5);
	CPlayerInfo* pPlayerInfo = &CWorld::Players[ScriptParams[0]];
	CPed* pTarget = CPools::GetPedPool()->GetAt(ScriptParams[1]);
	script_assert(pTarget);
	CVector pos = pPlayerInfo->GetPos();
	if (pTarget->bInVehicle) {
		X = pTarget->m_pMyVehicle->GetPosition().x;
		Y = pTarget->m_pMyVehicle->GetPosition().y;
		Z = pTarget->m_pMyVehicle->GetPosition().z;
	} else {
		X = pTarget->GetPosition().x;
		Y = pTarget->GetPosition().y;
		Z = pTarget->GetPosition().z;
	}
	dX = *(float*)&ScriptParams[2];
	dY = *(float*)&ScriptParams[3];
	if (b3D) {
		dZ = *(float*)&ScriptParams[4];
		debug = ScriptParams[5];
	}
	else {
		debug = ScriptParams[4];
	}
	result = false;
	bool in_area;
	if (b3D) {
		in_area = X - dX <= pos.x &&
			X + dX >= pos.x &&
			Y - dY <= pos.y &&
			Y + dY >= pos.y &&
			Z - dZ <= pos.z &&
			Z + dZ >= pos.z;
	}
	else {
		in_area = X - dX <= pos.x &&
			X + dX >= pos.x &&
			Y - dY <= pos.y &&
			Y + dY >= pos.y;
	}
	if (in_area) {
		switch (command) {
		case COMMAND_LOCATE_PLAYER_ANY_MEANS_CHAR_2D:
		case COMMAND_LOCATE_PLAYER_ANY_MEANS_CHAR_3D:
			result = true;
			break;
		case COMMAND_LOCATE_PLAYER_ON_FOOT_CHAR_2D:
		case COMMAND_LOCATE_PLAYER_ON_FOOT_CHAR_3D:
			result = !pPlayerInfo->m_pPed->bInVehicle;
			break;
		case COMMAND_LOCATE_PLAYER_IN_CAR_CHAR_2D:
		case COMMAND_LOCATE_PLAYER_IN_CAR_CHAR_3D:
			result = pPlayerInfo->m_pPed->bInVehicle;
			break;
		default:
			script_assert(false);
			break;
		}
	}
	UpdateCompareFlag(result);
	if (debug)
#ifdef FIX_BUGS
		CTheScripts::HighlightImportantArea((uintptr)this + m_nIp, X - dX, Y - dY, X + dX, Y + dY, b3D ? Z : MAP_Z_LOW_LIMIT);
#else
		CTheScripts::HighlightImportantArea((uintptr)this + m_nIp, X - dX, Y - dY, X + dX, Y + dX, b3D ? Z : MAP_Z_LOW_LIMIT);
#endif
	if (CTheScripts::DbgFlag) {
		if (b3D)
			CTheScripts::DrawDebugCube(X - dX, Y - dY, Z - dZ, X + dX, Y + dY, Z + dZ);
		else
			CTheScripts::DrawDebugSquare(X - dX, Y - dY, X + dX, Y + dY);
	}
}

void CRunningScript::LocatePlayerCarCommand(int32 command, uint32* pIp)
{
	bool b3D, result, debug;
	float X, Y, Z, dX, dY, dZ;
	switch (command) {
	case COMMAND_LOCATE_PLAYER_ANY_MEANS_CAR_3D:
	case COMMAND_LOCATE_PLAYER_ON_FOOT_CAR_3D:
	case COMMAND_LOCATE_PLAYER_IN_CAR_CAR_3D:
		b3D = true;
		break;
	default:
		b3D = false;
		break;
	}
	CollectParameters(pIp, b3D ? 6 : 5);
	CPlayerInfo* pPlayerInfo = &CWorld::Players[ScriptParams[0]];
	CVehicle* pTarget = CPools::GetVehiclePool()->GetAt(ScriptParams[1]);
	script_assert(pTarget);
	CVector pos = pPlayerInfo->GetPos();
	X = pTarget->GetPosition().x;
	Y = pTarget->GetPosition().y;
	Z = pTarget->GetPosition().z;
	dX = *(float*)&ScriptParams[2];
	dY = *(float*)&ScriptParams[3];
	if (b3D) {
		dZ = *(float*)&ScriptParams[4];
		debug = ScriptParams[5];
	}
	else {
		debug = ScriptParams[4];
	}
	result = false;
	bool in_area;
	if (b3D) {
		in_area = X - dX <= pos.x &&
			X + dX >= pos.x &&
			Y - dY <= pos.y &&
			Y + dY >= pos.y &&
			Z - dZ <= pos.z &&
			Z + dZ >= pos.z;
	}
	else {
		in_area = X - dX <= pos.x &&
			X + dX >= pos.x &&
			Y - dY <= pos.y &&
			Y + dY >= pos.y;
	}
	if (in_area) {
		switch (command) {
		case COMMAND_LOCATE_PLAYER_ANY_MEANS_CAR_2D:
		case COMMAND_LOCATE_PLAYER_ANY_MEANS_CAR_3D:
			result = true;
			break;
		case COMMAND_LOCATE_PLAYER_ON_FOOT_CAR_2D:
		case COMMAND_LOCATE_PLAYER_ON_FOOT_CAR_3D:
			result = !pPlayerInfo->m_pPed->bInVehicle;
			break;
		case COMMAND_LOCATE_PLAYER_IN_CAR_CAR_2D:
		case COMMAND_LOCATE_PLAYER_IN_CAR_CAR_3D:
			result = pPlayerInfo->m_pPed->bInVehicle;
			break;
		default:
			script_assert(false);
			break;
		}
	}
	UpdateCompareFlag(result);
	if (debug)
		CTheScripts::HighlightImportantArea((uintptr)this + m_nIp, X - dX, Y - dY, X + dX, Y + dY, b3D ? Z : MAP_Z_LOW_LIMIT);
	if (CTheScripts::DbgFlag) {
		if (b3D)
			CTheScripts::DrawDebugCube(X - dX, Y - dY, Z - dZ, X + dX, Y + dY, Z + dZ);
		else
			CTheScripts::DrawDebugSquare(X - dX, Y - dY, X + dX, Y + dY);
	}
}

void CRunningScript::LocateCharCommand(int32 command, uint32* pIp)
{
	bool b3D, result, debug, decided = false;
	float X, Y, Z, dX, dY, dZ;
	switch (command) {
	case COMMAND_LOCATE_CHAR_ANY_MEANS_3D:
	case COMMAND_LOCATE_CHAR_ON_FOOT_3D:
	case COMMAND_LOCATE_CHAR_IN_CAR_3D:
	case COMMAND_LOCATE_STOPPED_CHAR_ANY_MEANS_3D:
	case COMMAND_LOCATE_STOPPED_CHAR_ON_FOOT_3D:
	case COMMAND_LOCATE_STOPPED_CHAR_IN_CAR_3D:
		b3D = true;
		break;
	default:
		b3D = false;
		break;
	}
	CollectParameters(pIp, b3D ? 8 : 6);
	CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
	script_assert(pPed);
	CVector pos = pPed->bInVehicle ? pPed->m_pMyVehicle->GetPosition() : pPed->GetPosition();
	switch (command) {
	case COMMAND_LOCATE_STOPPED_CHAR_ANY_MEANS_2D:
	case COMMAND_LOCATE_STOPPED_CHAR_ANY_MEANS_3D:
	case COMMAND_LOCATE_STOPPED_CHAR_IN_CAR_2D:
	case COMMAND_LOCATE_STOPPED_CHAR_IN_CAR_3D:
	case COMMAND_LOCATE_STOPPED_CHAR_ON_FOOT_2D:
	case COMMAND_LOCATE_STOPPED_CHAR_ON_FOOT_3D:
		if (!CTheScripts::IsPedStopped(pPed)) {
			result = false;
			decided = true;
		}
		break;
	default:
		break;
	}
	X = *(float*)&ScriptParams[1];
	Y = *(float*)&ScriptParams[2];
	if (b3D) {
		Z = *(float*)&ScriptParams[3];
		dX = *(float*)&ScriptParams[4];
		dY = *(float*)&ScriptParams[5];
		dZ = *(float*)&ScriptParams[6];
		debug = ScriptParams[7];
	}
	else {
		dX = *(float*)&ScriptParams[3];
		dY = *(float*)&ScriptParams[4];
		debug = ScriptParams[5];
	}
	if (!decided) {
		result = false;
		bool in_area;
		if (b3D) {
			in_area = X - dX <= pos.x &&
				X + dX >= pos.x &&
				Y - dY <= pos.y &&
				Y + dY >= pos.y &&
				Z - dZ <= pos.z &&
				Z + dZ >= pos.z;
		}
		else {
			in_area = X - dX <= pos.x &&
				X + dX >= pos.x &&
				Y - dY <= pos.y &&
				Y + dY >= pos.y;
		}
		if (in_area) {
			switch (command) {
			case COMMAND_LOCATE_CHAR_ANY_MEANS_2D:
			case COMMAND_LOCATE_CHAR_ANY_MEANS_3D:
			case COMMAND_LOCATE_STOPPED_CHAR_ANY_MEANS_2D:
			case COMMAND_LOCATE_STOPPED_CHAR_ANY_MEANS_3D:
				result = true;
				break;
			case COMMAND_LOCATE_CHAR_ON_FOOT_2D:
			case COMMAND_LOCATE_CHAR_ON_FOOT_3D:
			case COMMAND_LOCATE_STOPPED_CHAR_ON_FOOT_2D:
			case COMMAND_LOCATE_STOPPED_CHAR_ON_FOOT_3D:
				result = !pPed->bInVehicle;
				break;
			case COMMAND_LOCATE_CHAR_IN_CAR_2D:
			case COMMAND_LOCATE_CHAR_IN_CAR_3D:
			case COMMAND_LOCATE_STOPPED_CHAR_IN_CAR_2D:
			case COMMAND_LOCATE_STOPPED_CHAR_IN_CAR_3D:
				result = pPed->bInVehicle;
				break;
			default:
				script_assert(false);
				break;
			}
		}
	}
	UpdateCompareFlag(result);
	if (debug)
		CTheScripts::HighlightImportantArea((uintptr)this + m_nIp, X - dX, Y - dY, X + dX, Y + dY, b3D ? Z : MAP_Z_LOW_LIMIT);
	if (CTheScripts::DbgFlag) {
		if (b3D)
			CTheScripts::DrawDebugCube(X - dX, Y - dY, Z - dZ, X + dX, Y + dY, Z + dZ);
		else
			CTheScripts::DrawDebugSquare(X - dX, Y - dY, X + dX, Y + dY);
	}
}

void CRunningScript::LocateCharCharCommand(int32 command, uint32* pIp)
{
	bool b3D, result, debug;
	float X, Y, Z, dX, dY, dZ;
	switch (command) {
	case COMMAND_LOCATE_CHAR_ANY_MEANS_CHAR_3D:
	case COMMAND_LOCATE_CHAR_ON_FOOT_CHAR_3D:
	case COMMAND_LOCATE_CHAR_IN_CAR_CHAR_3D:
		b3D = true;
		break;
	default:
		b3D = false;
		break;
	}
	CollectParameters(pIp, b3D ? 6 : 5);
	CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
	script_assert(pPed);
	CPed* pTarget = CPools::GetPedPool()->GetAt(ScriptParams[1]);
	script_assert(pTarget);
	CVector pos = pPed->bInVehicle ? pPed->m_pMyVehicle->GetPosition() : pPed->GetPosition();
	if (pTarget->bInVehicle) {
		X = pTarget->m_pMyVehicle->GetPosition().x;
		Y = pTarget->m_pMyVehicle->GetPosition().y;
		Z = pTarget->m_pMyVehicle->GetPosition().z;
	}
	else {
		X = pTarget->GetPosition().x;
		Y = pTarget->GetPosition().y;
		Z = pTarget->GetPosition().z;
	}
	dX = *(float*)&ScriptParams[2];
	dY = *(float*)&ScriptParams[3];
	if (b3D) {
		dZ = *(float*)&ScriptParams[4];
		debug = ScriptParams[5];
	}
	else {
		debug = ScriptParams[4];
	}
	result = false;
	bool in_area;
	if (b3D) {
		in_area = X - dX <= pos.x &&
			X + dX >= pos.x &&
			Y - dY <= pos.y &&
			Y + dY >= pos.y &&
			Z - dZ <= pos.z &&
			Z + dZ >= pos.z;
	}
	else {
		in_area = X - dX <= pos.x &&
			X + dX >= pos.x &&
			Y - dY <= pos.y &&
			Y + dY >= pos.y;
	}
	if (in_area) {
		switch (command) {
		case COMMAND_LOCATE_CHAR_ANY_MEANS_CHAR_2D:
		case COMMAND_LOCATE_CHAR_ANY_MEANS_CHAR_3D:
			result = true;
			break;
		case COMMAND_LOCATE_CHAR_ON_FOOT_CHAR_2D:
		case COMMAND_LOCATE_CHAR_ON_FOOT_CHAR_3D:
			result = !pPed->bInVehicle;
			break;
		case COMMAND_LOCATE_CHAR_IN_CAR_CHAR_2D:
		case COMMAND_LOCATE_CHAR_IN_CAR_CHAR_3D:
			result = pPed->bInVehicle;
			break;
		default:
			script_assert(false);
			break;
		}
	}
	UpdateCompareFlag(result);
	if (debug)
#ifdef FIX_BUGS
		CTheScripts::HighlightImportantArea((uintptr)this + m_nIp, X - dX, Y - dY, X + dX, Y + dY, b3D ? Z : MAP_Z_LOW_LIMIT);
#else
		CTheScripts::HighlightImportantArea((uintptr)this + m_nIp, X - dX, Y - dY, X + dX, Y + dX, b3D ? Z : MAP_Z_LOW_LIMIT);
#endif
	if (CTheScripts::DbgFlag) {
		if (b3D)
			CTheScripts::DrawDebugCube(X - dX, Y - dY, Z - dZ, X + dX, Y + dY, Z + dZ);
		else
			CTheScripts::DrawDebugSquare(X - dX, Y - dY, X + dX, Y + dY);
	}
}

void CRunningScript::LocateCharCarCommand(int32 command, uint32* pIp)
{
	bool b3D, result, debug;
	float X, Y, Z, dX, dY, dZ;
	switch (command) {
	case COMMAND_LOCATE_CHAR_ANY_MEANS_CAR_3D:
	case COMMAND_LOCATE_CHAR_ON_FOOT_CAR_3D:
	case COMMAND_LOCATE_CHAR_IN_CAR_CAR_3D:
		b3D = true;
		break;
	default:
		b3D = false;
		break;
	}
	CollectParameters(pIp, b3D ? 6 : 5);
	CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
	script_assert(pPed);
	CVehicle* pTarget = CPools::GetVehiclePool()->GetAt(ScriptParams[1]);
	script_assert(pTarget);
	CVector pos = pPed->bInVehicle ? pPed->m_pMyVehicle->GetPosition() : pPed->GetPosition();
	X = pTarget->GetPosition().x;
	Y = pTarget->GetPosition().y;
	Z = pTarget->GetPosition().z;
	dX = *(float*)&ScriptParams[2];
	dY = *(float*)&ScriptParams[3];
	if (b3D) {
		dZ = *(float*)&ScriptParams[4];
		debug = ScriptParams[5];
	}
	else {
		debug = ScriptParams[4];
	}
	result = false;
	bool in_area;
	if (b3D) {
		in_area = X - dX <= pos.x &&
			X + dX >= pos.x &&
			Y - dY <= pos.y &&
			Y + dY >= pos.y &&
			Z - dZ <= pos.z &&
			Z + dZ >= pos.z;
	}
	else {
		in_area = X - dX <= pos.x &&
			X + dX >= pos.x &&
			Y - dY <= pos.y &&
			Y + dY >= pos.y;
	}
	if (in_area) {
		switch (command) {
		case COMMAND_LOCATE_CHAR_ANY_MEANS_CAR_2D:
		case COMMAND_LOCATE_CHAR_ANY_MEANS_CAR_3D:
			result = true;
			break;
		case COMMAND_LOCATE_CHAR_ON_FOOT_CAR_2D:
		case COMMAND_LOCATE_CHAR_ON_FOOT_CAR_3D:
			result = !pPed->bInVehicle;
			break;
		case COMMAND_LOCATE_CHAR_IN_CAR_CAR_2D:
		case COMMAND_LOCATE_CHAR_IN_CAR_CAR_3D:
			result = pPed->bInVehicle;
			break;
		default:
			script_assert(false);
			break;
		}
	}
	UpdateCompareFlag(result);
	if (debug)
		CTheScripts::HighlightImportantArea((uintptr)this + m_nIp, X - dX, Y - dY, X + dX, Y + dY, b3D ? Z : MAP_Z_LOW_LIMIT);
	if (CTheScripts::DbgFlag) {
		if (b3D)
			CTheScripts::DrawDebugCube(X - dX, Y - dY, Z - dZ, X + dX, Y + dY, Z + dZ);
		else
			CTheScripts::DrawDebugSquare(X - dX, Y - dY, X + dX, Y + dY);
	}
}

void CRunningScript::LocateCharObjectCommand(int32 command, uint32* pIp)
{
	bool b3D, result, debug;
	float X, Y, Z, dX, dY, dZ;
	switch (command) {
	case COMMAND_LOCATE_CHAR_ANY_MEANS_OBJECT_3D:
	case COMMAND_LOCATE_CHAR_ON_FOOT_OBJECT_3D:
	case COMMAND_LOCATE_CHAR_IN_CAR_OBJECT_3D:
		b3D = true;
		break;
	default:
		b3D = false;
		break;
	}
	CollectParameters(pIp, b3D ? 6 : 5);
	CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
	script_assert(pPed);
	CObject* pTarget = CPools::GetObjectPool()->GetAt(ScriptParams[1]);
	script_assert(pTarget);
	CVector pos = pPed->bInVehicle ? pPed->m_pMyVehicle->GetPosition() : pPed->GetPosition();
	X = pTarget->GetPosition().x;
	Y = pTarget->GetPosition().y;
	Z = pTarget->GetPosition().z;
	dX = *(float*)&ScriptParams[2];
	dY = *(float*)&ScriptParams[3];
	if (b3D) {
		dZ = *(float*)&ScriptParams[4];
		debug = ScriptParams[5];
	}
	else {
		debug = ScriptParams[4];
	}
	result = false;
	bool in_area;
	if (b3D) {
		in_area = X - dX <= pos.x &&
			X + dX >= pos.x &&
			Y - dY <= pos.y &&
			Y + dY >= pos.y &&
			Z - dZ <= pos.z &&
			Z + dZ >= pos.z;
	}
	else {
		in_area = X - dX <= pos.x &&
			X + dX >= pos.x &&
			Y - dY <= pos.y &&
			Y + dY >= pos.y;
	}
	if (in_area) {
		switch (command) {
		case COMMAND_LOCATE_CHAR_ANY_MEANS_OBJECT_2D:
		case COMMAND_LOCATE_CHAR_ANY_MEANS_OBJECT_3D:
			result = true;
			break;
		case COMMAND_LOCATE_CHAR_ON_FOOT_OBJECT_2D:
		case COMMAND_LOCATE_CHAR_ON_FOOT_OBJECT_3D:
			result = !pPed->bInVehicle;
			break;
		case COMMAND_LOCATE_CHAR_IN_CAR_OBJECT_2D:
		case COMMAND_LOCATE_CHAR_IN_CAR_OBJECT_3D:
			result = pPed->bInVehicle;
			break;
		default:
			script_assert(false);
			break;
		}
	}
	UpdateCompareFlag(result);
	if (debug)
		CTheScripts::HighlightImportantArea((uintptr)this + m_nIp, X - dX, Y - dY, X + dX, Y + dY, b3D ? Z : MAP_Z_LOW_LIMIT);
	if (CTheScripts::DbgFlag) {
		if (b3D)
			CTheScripts::DrawDebugCube(X - dX, Y - dY, Z - dZ, X + dX, Y + dY, Z + dZ);
		else
			CTheScripts::DrawDebugSquare(X - dX, Y - dY, X + dX, Y + dY);
	}
}

void CRunningScript::LocateCarCommand(int32 command, uint32* pIp)
{
	bool b3D, result, debug, decided = false;
	float X, Y, Z, dX, dY, dZ;
	switch (command) {
	case COMMAND_LOCATE_CAR_3D:
	case COMMAND_LOCATE_STOPPED_CAR_3D:
		b3D = true;
		break;
	default:
		b3D = false;
		break;
	}
	CollectParameters(pIp, b3D ? 8 : 6);
	CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
	script_assert(pVehicle);
	CVector pos = pVehicle->GetPosition();
	switch (command) {
	case COMMAND_LOCATE_STOPPED_CAR_2D:
	case COMMAND_LOCATE_STOPPED_CAR_3D:
		if (!CTheScripts::IsVehicleStopped(pVehicle)) {
			result = false;
			decided = true;
		}
		break;
	default:
		break;
	}
	X = *(float*)&ScriptParams[1];
	Y = *(float*)&ScriptParams[2];
	if (b3D) {
		Z = *(float*)&ScriptParams[3];
		dX = *(float*)&ScriptParams[4];
		dY = *(float*)&ScriptParams[5];
		dZ = *(float*)&ScriptParams[6];
		debug = ScriptParams[7];
	}
	else {
		dX = *(float*)&ScriptParams[3];
		dY = *(float*)&ScriptParams[4];
		debug = ScriptParams[5];
	}
	if (!decided) {
		result = false;
		bool in_area;
		if (b3D) {
			in_area = X - dX <= pos.x &&
				X + dX >= pos.x &&
				Y - dY <= pos.y &&
				Y + dY >= pos.y &&
				Z - dZ <= pos.z &&
				Z + dZ >= pos.z;
		}
		else {
			in_area = X - dX <= pos.x &&
				X + dX >= pos.x &&
				Y - dY <= pos.y &&
				Y + dY >= pos.y;
		}
		result = in_area;
	}
	UpdateCompareFlag(result);
	if (debug)
		CTheScripts::HighlightImportantArea((uintptr)this + m_nIp, X - dX, Y - dY, X + dX, Y + dY, b3D ? Z : MAP_Z_LOW_LIMIT);
	if (CTheScripts::DbgFlag) {
		if (b3D)
			CTheScripts::DrawDebugCube(X - dX, Y - dY, Z - dZ, X + dX, Y + dY, Z + dZ);
		else
			CTheScripts::DrawDebugSquare(X - dX, Y - dY, X + dX, Y + dY);
	}
}

void CRunningScript::LocateSniperBulletCommand(int32 command, uint32* pIp)
{
	bool b3D, result, debug;
	float X, Y, Z, dX, dY, dZ;
	switch (command) {
	case COMMAND_LOCATE_SNIPER_BULLET_3D:
		b3D = true;
		break;
	default:
		b3D = false;
		break;
	}
	CollectParameters(pIp, b3D ? 7 : 5);
	X = *(float*)&ScriptParams[0];
	Y = *(float*)&ScriptParams[1];
	if (b3D) {
		Z = *(float*)&ScriptParams[2];
		dX = *(float*)&ScriptParams[3];
		dY = *(float*)&ScriptParams[4];
		dZ = *(float*)&ScriptParams[5];
		debug = ScriptParams[6];
	}
	else {
		dX = *(float*)&ScriptParams[2];
		dY = *(float*)&ScriptParams[3];
		debug = ScriptParams[4];
	}
	result = CBulletInfo::TestForSniperBullet(X - dX, X + dX, Y - dY, Y + dY, b3D ? Z - dZ : -1000.0f, b3D ? Z + dZ : 1000.0f);
	UpdateCompareFlag(result);
	if (debug)
		CTheScripts::HighlightImportantArea((uintptr)this + m_nIp, X - dX, Y - dY, X + dX, Y + dY, b3D ? Z : MAP_Z_LOW_LIMIT);
	if (CTheScripts::DbgFlag) {
		if (b3D)
			CTheScripts::DrawDebugCube(X - dX, Y - dY, Z - dZ, X + dX, Y + dY, Z + dZ);
		else
			CTheScripts::DrawDebugSquare(X - dX, Y - dY, X + dX, Y + dY);
	}
}

void CRunningScript::PlayerInAreaCheckCommand(int32 command, uint32* pIp)
{
	bool b3D, result, debug, decided = false;
	float infX, infY, infZ, supX, supY, supZ;
	switch (command) {
	case COMMAND_IS_PLAYER_IN_AREA_3D:
	case COMMAND_IS_PLAYER_IN_AREA_ON_FOOT_3D:
	case COMMAND_IS_PLAYER_IN_AREA_IN_CAR_3D:
	case COMMAND_IS_PLAYER_STOPPED_IN_AREA_3D:
	case COMMAND_IS_PLAYER_STOPPED_IN_AREA_ON_FOOT_3D:
	case COMMAND_IS_PLAYER_STOPPED_IN_AREA_IN_CAR_3D:
		b3D = true;
		break;
	default:
		b3D = false;
		break;
	}
	CollectParameters(pIp, b3D ? 8 : 6);
	CPlayerInfo* pPlayerInfo = &CWorld::Players[ScriptParams[0]];
	switch (command) {
	case COMMAND_IS_PLAYER_STOPPED_IN_AREA_3D:
	case COMMAND_IS_PLAYER_STOPPED_IN_AREA_ON_FOOT_3D:
	case COMMAND_IS_PLAYER_STOPPED_IN_AREA_IN_CAR_3D:
	case COMMAND_IS_PLAYER_STOPPED_IN_AREA_2D:
	case COMMAND_IS_PLAYER_STOPPED_IN_AREA_ON_FOOT_2D:
	case COMMAND_IS_PLAYER_STOPPED_IN_AREA_IN_CAR_2D:
		if (!CTheScripts::IsPlayerStopped(pPlayerInfo)) {
			result = false;
			decided = true;
		}
		break;
	default:
		break;
	}
	infX = *(float*)&ScriptParams[1];
	infY = *(float*)&ScriptParams[2];
	if (b3D) {
		infZ = *(float*)&ScriptParams[3];
		supX = *(float*)&ScriptParams[4];
		supY = *(float*)&ScriptParams[5];
		supZ = *(float*)&ScriptParams[6];
		if (infZ > supZ) {
			infZ = *(float*)&ScriptParams[6];
			supZ = *(float*)&ScriptParams[3];
		}
		debug = ScriptParams[7];
	}
	else {
		supX = *(float*)&ScriptParams[3];
		supY = *(float*)&ScriptParams[4];
		debug = ScriptParams[5];
	}
	if (infX > supX) {
		float tmp = infX;
		infX = supX;
		supX = tmp;
	}
	if (infY > supY) {
		float tmp = infY;
		infY = supY;
		supY = tmp;
	}
	if (!decided) {
		CVector pos = pPlayerInfo->GetPos();
		result = false;
		bool in_area;
		if (b3D) {
			in_area = infX <= pos.x &&
				supX >= pos.x &&
				infY <= pos.y &&
				supY >= pos.y &&
				infZ <= pos.z &&
				supZ >= pos.z;
		}
		else {
			in_area = infX <= pos.x &&
				supX >= pos.x &&
				infY <= pos.y &&
				supY >= pos.y;
		}
		if (in_area) {
			switch (command) {
			case COMMAND_IS_PLAYER_IN_AREA_2D:
			case COMMAND_IS_PLAYER_IN_AREA_3D:
			case COMMAND_IS_PLAYER_STOPPED_IN_AREA_2D:
			case COMMAND_IS_PLAYER_STOPPED_IN_AREA_3D:
				result = true;
				break;
			case COMMAND_IS_PLAYER_IN_AREA_ON_FOOT_2D:
			case COMMAND_IS_PLAYER_IN_AREA_ON_FOOT_3D:
			case COMMAND_IS_PLAYER_STOPPED_IN_AREA_ON_FOOT_2D:
			case COMMAND_IS_PLAYER_STOPPED_IN_AREA_ON_FOOT_3D:
				result = !pPlayerInfo->m_pPed->bInVehicle;
				break;
			case COMMAND_IS_PLAYER_IN_AREA_IN_CAR_2D:
			case COMMAND_IS_PLAYER_IN_AREA_IN_CAR_3D:
			case COMMAND_IS_PLAYER_STOPPED_IN_AREA_IN_CAR_2D:
			case COMMAND_IS_PLAYER_STOPPED_IN_AREA_IN_CAR_3D:
				result = pPlayerInfo->m_pPed->bInVehicle;
				break;
			default:
				script_assert(false);
				break;
			}
		}
	}
	UpdateCompareFlag(result);
	if (debug)
		CTheScripts::HighlightImportantArea((uintptr)this + m_nIp, infX, infY, supX, supY, b3D ? (infZ + supZ) / 2 : MAP_Z_LOW_LIMIT);
	if (CTheScripts::DbgFlag) {
		if (b3D)
			CTheScripts::DrawDebugCube(infX, infY, infZ, supX, supY, supZ);
		else
			CTheScripts::DrawDebugSquare(infX, infY, supX, supY);
	}
}

void CRunningScript::PlayerInAngledAreaCheckCommand(int32 command, uint32* pIp)
{
	bool b3D, result, debug, decided = false;
	float infX, infY, infZ, supX, supY, supZ, side2length;
	switch (command) {
	case COMMAND_IS_PLAYER_IN_ANGLED_AREA_3D:
	case COMMAND_IS_PLAYER_IN_ANGLED_AREA_ON_FOOT_3D:
	case COMMAND_IS_PLAYER_IN_ANGLED_AREA_IN_CAR_3D:
	case COMMAND_IS_PLAYER_STOPPED_IN_ANGLED_AREA_3D:
	case COMMAND_IS_PLAYER_STOPPED_IN_ANGLED_AREA_ON_FOOT_3D:
	case COMMAND_IS_PLAYER_STOPPED_IN_ANGLED_AREA_IN_CAR_3D:
		b3D = true;
		break;
	default:
		b3D = false;
		break;
	}
	CollectParameters(pIp, b3D ? 9 : 7);
	CPlayerInfo* pPlayerInfo = &CWorld::Players[ScriptParams[0]];
	switch (command) {
	case COMMAND_IS_PLAYER_STOPPED_IN_ANGLED_AREA_3D:
	case COMMAND_IS_PLAYER_STOPPED_IN_ANGLED_AREA_ON_FOOT_3D:
	case COMMAND_IS_PLAYER_STOPPED_IN_ANGLED_AREA_IN_CAR_3D:
	case COMMAND_IS_PLAYER_STOPPED_IN_ANGLED_AREA_2D:
	case COMMAND_IS_PLAYER_STOPPED_IN_ANGLED_AREA_ON_FOOT_2D:
	case COMMAND_IS_PLAYER_STOPPED_IN_ANGLED_AREA_IN_CAR_2D:
		if (!CTheScripts::IsPlayerStopped(pPlayerInfo)) {
			result = false;
			decided = true;
		}
		break;
	default:
		break;
	}
	infX = *(float*)&ScriptParams[1];
	infY = *(float*)&ScriptParams[2];
	if (b3D) {
		infZ = *(float*)&ScriptParams[3];
		supX = *(float*)&ScriptParams[4];
		supY = *(float*)&ScriptParams[5];
		supZ = *(float*)&ScriptParams[6];
		if (infZ > supZ) {
			infZ = *(float*)&ScriptParams[6];
			supZ = *(float*)&ScriptParams[3];
		}
		side2length = *(float*)&ScriptParams[7];
		debug = ScriptParams[8];
	}
	else {
		supX = *(float*)&ScriptParams[3];
		supY = *(float*)&ScriptParams[4];
		side2length = *(float*)&ScriptParams[5];
		debug = ScriptParams[6];
	}
	float initAngle = CGeneral::GetRadianAngleBetweenPoints(infX, infY, supX, supY) + HALFPI;
	while (initAngle < 0.0f)
		initAngle += TWOPI;
	while (initAngle > TWOPI)
		initAngle -= TWOPI;
	// it looks like the idea is to use a rectangle using the diagonal of the rectangle as
	// the side of new rectangle, with "length" being the length of second side
	float rotatedSupX = supX + side2length * sin(initAngle);
	float rotatedSupY = supY - side2length * cos(initAngle);
	float rotatedInfX = infX + side2length * sin(initAngle);
	float rotatedInfY = infY - side2length * cos(initAngle);
	float side1X = supX - infX;
	float side1Y = supY - infY;
	float side1Length = CVector2D(side1X, side1Y).Magnitude();
	float side2X = rotatedInfX - infX;
	float side2Y = rotatedInfY - infY;
	float side2Length = CVector2D(side2X, side2Y).Magnitude(); // == side2length?
	if (!decided) {
		CVector pos = pPlayerInfo->GetPos();
		result = false;
		float X = pos.x - infX;
		float Y = pos.y - infY;
		float positionAlongSide1 = X * side1X / side1Length + Y * side1Y / side1Length;
		bool in_area = false;
		if (positionAlongSide1 >= 0.0f && positionAlongSide1 <= side1Length) {
			float positionAlongSide2 = X * side2X / side2Length + Y * side2Y / side2Length;
			if (positionAlongSide2 >= 0.0f && positionAlongSide2 <= side2Length) {
				in_area = !b3D || pos.z >= infZ && pos.z <= supZ;
			}
		}

		if (in_area) {
			switch (command) {
			case COMMAND_IS_PLAYER_IN_ANGLED_AREA_2D:
			case COMMAND_IS_PLAYER_IN_ANGLED_AREA_3D:
			case COMMAND_IS_PLAYER_STOPPED_IN_ANGLED_AREA_2D:
			case COMMAND_IS_PLAYER_STOPPED_IN_ANGLED_AREA_3D:
				result = true;
				break;
			case COMMAND_IS_PLAYER_IN_ANGLED_AREA_ON_FOOT_2D:
			case COMMAND_IS_PLAYER_IN_ANGLED_AREA_ON_FOOT_3D:
			case COMMAND_IS_PLAYER_STOPPED_IN_ANGLED_AREA_ON_FOOT_2D:
			case COMMAND_IS_PLAYER_STOPPED_IN_ANGLED_AREA_ON_FOOT_3D:
				result = !pPlayerInfo->m_pPed->bInVehicle;
				break;
			case COMMAND_IS_PLAYER_IN_ANGLED_AREA_IN_CAR_2D:
			case COMMAND_IS_PLAYER_IN_ANGLED_AREA_IN_CAR_3D:
			case COMMAND_IS_PLAYER_STOPPED_IN_ANGLED_AREA_IN_CAR_2D:
			case COMMAND_IS_PLAYER_STOPPED_IN_ANGLED_AREA_IN_CAR_3D:
				result = pPlayerInfo->m_pPed->bInVehicle;
				break;
			default:
				script_assert(false);
				break;
			}
		}
	}
	UpdateCompareFlag(result);
	if (debug)
		CTheScripts::HighlightImportantAngledArea((uintptr)this + m_nIp, infX, infY, supX, supY,
			rotatedSupX, rotatedSupY, rotatedInfX, rotatedInfY, b3D ? (infZ + supZ) / 2 : MAP_Z_LOW_LIMIT);
	if (CTheScripts::DbgFlag) {
		if (b3D)
			CTheScripts::DrawDebugAngledCube(infX, infY, infZ, supX, supY, supZ,
				rotatedSupX, rotatedSupY, rotatedInfX, rotatedInfY);
		else
			CTheScripts::DrawDebugAngledSquare(infX, infY, supX, supY,
				rotatedSupX, rotatedSupY, rotatedInfX, rotatedInfY);
	}
}

void CRunningScript::CharInAreaCheckCommand(int32 command, uint32* pIp)
{
	bool b3D, result, debug, decided = false;
	float infX, infY, infZ, supX, supY, supZ;
	switch (command) {
	case COMMAND_IS_CHAR_IN_AREA_3D:
	case COMMAND_IS_CHAR_IN_AREA_ON_FOOT_3D:
	case COMMAND_IS_CHAR_IN_AREA_IN_CAR_3D:
	case COMMAND_IS_CHAR_STOPPED_IN_AREA_3D:
	case COMMAND_IS_CHAR_STOPPED_IN_AREA_ON_FOOT_3D:
	case COMMAND_IS_CHAR_STOPPED_IN_AREA_IN_CAR_3D:
		b3D = true;
		break;
	default:
		b3D = false;
		break;
	}
	CollectParameters(pIp, b3D ? 8 : 6);
	CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
	script_assert(pPed);
	CVector pos = pPed->bInVehicle ? pPed->m_pMyVehicle->GetPosition() : pPed->GetPosition();
	switch (command) {
	case COMMAND_IS_CHAR_STOPPED_IN_AREA_3D:
	case COMMAND_IS_CHAR_STOPPED_IN_AREA_ON_FOOT_3D:
	case COMMAND_IS_CHAR_STOPPED_IN_AREA_IN_CAR_3D:
	case COMMAND_IS_CHAR_STOPPED_IN_AREA_2D:
	case COMMAND_IS_CHAR_STOPPED_IN_AREA_ON_FOOT_2D:
	case COMMAND_IS_CHAR_STOPPED_IN_AREA_IN_CAR_2D:
		if (!CTheScripts::IsPedStopped(pPed)) {
			result = false;
			decided = true;
		}
		break;
	default:
		break;
	}
	infX = *(float*)&ScriptParams[1];
	infY = *(float*)&ScriptParams[2];
	if (b3D) {
		infZ = *(float*)&ScriptParams[3];
		supX = *(float*)&ScriptParams[4];
		supY = *(float*)&ScriptParams[5];
		supZ = *(float*)&ScriptParams[6];
		if (infZ > supZ) {
			infZ = *(float*)&ScriptParams[6];
			supZ = *(float*)&ScriptParams[3];
		}
		debug = ScriptParams[7];
	}
	else {
		supX = *(float*)&ScriptParams[3];
		supY = *(float*)&ScriptParams[4];
		debug = ScriptParams[5];
	}
	if (infX > supX) {
		float tmp = infX;
		infX = supX;
		supX = tmp;
	}
	if (infY > supY) {
		float tmp = infY;
		infY = supY;
		supY = tmp;
	}
	if (!decided) {
		result = false;
		bool in_area;
		if (b3D) {
			in_area = infX <= pos.x &&
				supX >= pos.x &&
				infY <= pos.y &&
				supY >= pos.y &&
				infZ <= pos.z &&
				supZ >= pos.z;
		}
		else {
			in_area = infX <= pos.x &&
				supX >= pos.x &&
				infY <= pos.y &&
				supY >= pos.y;
		}
		if (in_area) {
			switch (command) {
			case COMMAND_IS_CHAR_IN_AREA_2D:
			case COMMAND_IS_CHAR_IN_AREA_3D:
			case COMMAND_IS_CHAR_STOPPED_IN_AREA_2D:
			case COMMAND_IS_CHAR_STOPPED_IN_AREA_3D:
				result = true;
				break;
			case COMMAND_IS_CHAR_IN_AREA_ON_FOOT_2D:
			case COMMAND_IS_CHAR_IN_AREA_ON_FOOT_3D:
			case COMMAND_IS_CHAR_STOPPED_IN_AREA_ON_FOOT_2D:
			case COMMAND_IS_CHAR_STOPPED_IN_AREA_ON_FOOT_3D:
				result = !pPed->bInVehicle;
				break;
			case COMMAND_IS_CHAR_IN_AREA_IN_CAR_2D:
			case COMMAND_IS_CHAR_IN_AREA_IN_CAR_3D:
			case COMMAND_IS_CHAR_STOPPED_IN_AREA_IN_CAR_2D:
			case COMMAND_IS_CHAR_STOPPED_IN_AREA_IN_CAR_3D:
				result = pPed->bInVehicle;
				break;
			default:
				script_assert(false);
				break;
			}
		}
	}
	UpdateCompareFlag(result);
	if (debug)
		CTheScripts::HighlightImportantArea((uintptr)this + m_nIp, infX, infY, supX, supY, b3D ? (infZ + supZ) / 2 : MAP_Z_LOW_LIMIT);
	if (CTheScripts::DbgFlag) {
		if (b3D)
			CTheScripts::DrawDebugCube(infX, infY, infZ, supX, supY, supZ);
		else
			CTheScripts::DrawDebugSquare(infX, infY, supX, supY);
	}
}

void CRunningScript::CarInAreaCheckCommand(int32 command, uint32* pIp)
{
	bool b3D, result, debug, decided = false;
	float infX, infY, infZ, supX, supY, supZ;
	switch (command) {
	case COMMAND_IS_CAR_IN_AREA_3D:
	case COMMAND_IS_CAR_STOPPED_IN_AREA_3D:
		b3D = true;
		break;
	default:
		b3D = false;
		break;
	}
	CollectParameters(pIp, b3D ? 8 : 6);
	CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
	script_assert(pVehicle);
	CVector pos = pVehicle->GetPosition();
	switch (command) {
	case COMMAND_IS_CAR_STOPPED_IN_AREA_3D:
	case COMMAND_IS_CAR_STOPPED_IN_AREA_2D:
		if (!CTheScripts::IsVehicleStopped(pVehicle)) {
			result = false;
			decided = true;
		}
		break;
	default:
		break;
	}
	infX = *(float*)&ScriptParams[1];
	infY = *(float*)&ScriptParams[2];
	if (b3D) {
		infZ = *(float*)&ScriptParams[3];
		supX = *(float*)&ScriptParams[4];
		supY = *(float*)&ScriptParams[5];
		supZ = *(float*)&ScriptParams[6];
		if (infZ > supZ) {
			infZ = *(float*)&ScriptParams[6];
			supZ = *(float*)&ScriptParams[3];
		}
		debug = ScriptParams[7];
	}
	else {
		supX = *(float*)&ScriptParams[3];
		supY = *(float*)&ScriptParams[4];
		debug = ScriptParams[5];
	}
	if (infX > supX) {
		float tmp = infX;
		infX = supX;
		supX = tmp;
	}
	if (infY > supY) {
		float tmp = infY;
		infY = supY;
		supY = tmp;
	}
	if (!decided) {
		result = false;
		bool in_area;
		if (b3D) {
			in_area = infX <= pos.x &&
				supX >= pos.x &&
				infY <= pos.y &&
				supY >= pos.y &&
				infZ <= pos.z &&
				supZ >= pos.z;
		}
		else {
			in_area = infX <= pos.x &&
				supX >= pos.x &&
				infY <= pos.y &&
				supY >= pos.y;
		}
		if (in_area) {
			switch (command) {
			case COMMAND_IS_CAR_IN_AREA_2D:
			case COMMAND_IS_CAR_IN_AREA_3D:
			case COMMAND_IS_CAR_STOPPED_IN_AREA_2D:
			case COMMAND_IS_CAR_STOPPED_IN_AREA_3D:
				result = true;
				break;
			default:
				script_assert(false);
				break;
			}
		}
	}
	UpdateCompareFlag(result);
	if (debug)
		CTheScripts::HighlightImportantArea((uintptr)this + m_nIp, infX, infY, supX, supY, b3D ? (infZ + supZ) / 2 : MAP_Z_LOW_LIMIT);
	if (CTheScripts::DbgFlag) {
		if (b3D)
			CTheScripts::DrawDebugCube(infX, infY, infZ, supX, supY, supZ);
		else
			CTheScripts::DrawDebugSquare(infX, infY, supX, supY);
	}
}

void CRunningScript::DoDeatharrestCheck()
{
	if (!m_bDeatharrestEnabled)
		return;
	if (!CTheScripts::IsPlayerOnAMission())
		return;
	CPlayerInfo* pPlayer = &CWorld::Players[CWorld::PlayerInFocus];
	if (!pPlayer->IsRestartingAfterDeath() && !pPlayer->IsRestartingAfterArrest() && !CTheScripts::UpsideDownCars.AreAnyCarsUpsideDown())
		return;
#ifdef MISSION_REPLAY
	if (AllowMissionReplay != 0)
		return;
	if (CanAllowMissionReplay())
		AllowMissionReplay = 1;
#endif
	script_assert(m_nStackPointer > 0);
	while (m_nStackPointer > 1)
		--m_nStackPointer;
	m_nIp = m_anStack[--m_nStackPointer];
	int16 messageId;
	if (pPlayer->IsRestartingAfterDeath())
		messageId = 0;
	else if (pPlayer->IsRestartingAfterArrest())
		messageId = 5;
	else
		messageId = 10;
	messageId += CGeneral::GetRandomNumberInRange(0, 5);
	bool found = false;
	for (int16 contact = 0; !found && contact < MAX_NUM_CONTACTS; contact++) {
		int contactFlagOffset = CTheScripts::OnAMissionForContactFlag[contact];
		if (contactFlagOffset && CTheScripts::ScriptSpace[contactFlagOffset] == 1) {
			messageId += CTheScripts::BaseBriefIdForContact[contact];
			found = true;
		}
	}
	if (!found)
		messageId = 8001;
	char tmp[16];
	sprintf(tmp, "%d", messageId);
	CMessages::ClearSmallMessagesOnly();
	wchar* text = TheText.Get(tmp);
	// ...and do nothing about it
	*(int32*)&CTheScripts::ScriptSpace[CTheScripts::OnAMissionFlag] = 0;
	m_bDeatharrestExecuted = true;
	m_nWakeTime = 0;
}

int16 CRunningScript::GetPadState(uint16 pad, uint16 button)
{
	CPad* pPad = CPad::GetPad(pad);
	switch (button) {
	case 0: return pPad->NewState.LeftStickX;
	case 1: return pPad->NewState.LeftStickY;
	case 2: return pPad->NewState.RightStickX;
	case 3: return pPad->NewState.RightStickY;
	case 4: return pPad->NewState.LeftShoulder1;
	case 5: return pPad->NewState.LeftShoulder2;
	case 6: return pPad->NewState.RightShoulder1;
	case 7: return pPad->NewState.RightShoulder2;
	case 8: return pPad->NewState.DPadUp;
	case 9: return pPad->NewState.DPadDown;
	case 10: return pPad->NewState.DPadLeft;
	case 11: return pPad->NewState.DPadRight;
	case 12: return pPad->NewState.Start;
	case 13: return pPad->NewState.Select;
	case 14: return pPad->NewState.Square;
	case 15: return pPad->NewState.Triangle;
	case 16: return pPad->NewState.Cross;
	case 17: return pPad->NewState.Circle;
	case 18: return pPad->NewState.LeftShock;
	case 19: return pPad->NewState.RightShock;
	default: break;
	}
	return 0;
}

#ifdef GTA_SCRIPT_COLLECTIVE
void CRunningScript::LocateCollectiveCommand(int32 command, uint32* pIp)
{
	bool b3D, result, debug, decided = false;
	float X, Y, Z, dX, dY, dZ;
	switch (command) {
	case COMMAND_LOCATE_COLL_ANY_MEANS_2D:
	case COMMAND_LOCATE_COLL_ON_FOOT_2D:
	case COMMAND_LOCATE_COLL_IN_CAR_2D:
	case COMMAND_LOCATE_STOPPED_COLL_ANY_MEANS_2D:
	case COMMAND_LOCATE_STOPPED_COLL_ON_FOOT_2D:
	case COMMAND_LOCATE_STOPPED_COLL_IN_CAR_2D:
		b3D = false;
		break;
	default:
		b3D = true;
		break;
	}
	CollectParameters(pIp, b3D ? 8 : 6);
	X = *(float*)&ScriptParams[1];
	Y = *(float*)&ScriptParams[2];
	if (b3D) {
		Z = *(float*)&ScriptParams[3];
		dX = *(float*)&ScriptParams[4];
		dY = *(float*)&ScriptParams[5];
		dZ = *(float*)&ScriptParams[6];
		debug = ScriptParams[7];
	}
	else {
		dX = *(float*)&ScriptParams[3];
		dY = *(float*)&ScriptParams[4];
		debug = ScriptParams[5];
	}
	result = true;
	for (int i = 0; i < MAX_NUM_COLLECTIVES && result; i++) {
		if (ScriptParams[0] != CTheScripts::CollectiveArray[i].colIndex)
			continue;
		CPed* pPed = CPools::GetPedPool()->GetAt(CTheScripts::CollectiveArray[i].pedIndex);
		if (!pPed) {
			CTheScripts::CollectiveArray[i].colIndex = -1;
			CTheScripts::CollectiveArray[i].pedIndex = 0;
			continue;
		}
		CVector pos = pPed->bInVehicle ? pPed->m_pMyVehicle->GetPosition() : pPed->GetPosition();
		switch (command) {
		case COMMAND_LOCATE_STOPPED_COLL_ANY_MEANS_2D:
		case COMMAND_LOCATE_STOPPED_COLL_ON_FOOT_2D:
		case COMMAND_LOCATE_STOPPED_COLL_IN_CAR_2D:
			if (!CTheScripts::IsPedStopped(pPed)) {
				result = false;
				decided = true;
			}
			break;
		default:
			break;
		}
		if (!decided) {
			bool in_area;
			if (b3D) {
				in_area = X - dX <= pos.x &&
					X + dX >= pos.x &&
					Y - dY <= pos.y &&
					Y + dY >= pos.y &&
					Z - dZ <= pos.z &&
					Z + dZ >= pos.z;
			}
			else {
				in_area = X - dX <= pos.x &&
					X + dX >= pos.x &&
					Y - dY <= pos.y &&
					Y + dY >= pos.y;
			}
			result = false;
			if (in_area) {
				switch (command) {
				case COMMAND_LOCATE_COLL_ANY_MEANS_2D:
				case COMMAND_LOCATE_STOPPED_COLL_ANY_MEANS_2D:
					result = true;
					break;
				case COMMAND_LOCATE_COLL_ON_FOOT_2D:
				case COMMAND_LOCATE_STOPPED_COLL_ON_FOOT_2D:
					result = !pPed->bInVehicle;
					break;
				case COMMAND_LOCATE_COLL_IN_CAR_2D:
				case COMMAND_LOCATE_STOPPED_COLL_IN_CAR_2D:
					result = pPed->bInVehicle;
					break;
				default:
					script_assert(false);
					break;
				}
			}
		}
	}
	UpdateCompareFlag(result);
	if (debug)
		CTheScripts::HighlightImportantArea((uintptr)this + m_nIp, X - dX, Y - dY, X + dX, Y + dY, b3D ? Z : MAP_Z_LOW_LIMIT);
	if (CTheScripts::DbgFlag) {
		if (b3D)
			CTheScripts::DrawDebugCube(X - dX, Y - dY, Z - dZ, X + dX, Y + dY, Z + dZ);
		else
			CTheScripts::DrawDebugSquare(X - dX, Y - dY, X + dX, Y + dY);
	}
}

void CRunningScript::LocateCollectiveCharCommand(int32 command, uint32* pIp)
{
	bool b3D, result, debug;
	float X, Y, Z, dX, dY, dZ;
	switch (command) {
	case COMMAND_LOCATE_COLL_ANY_MEANS_CHAR_2D:
	case COMMAND_LOCATE_COLL_ON_FOOT_CHAR_2D:
	case COMMAND_LOCATE_COLL_IN_CAR_CHAR_2D:
		b3D = false;
		break;
	default:
		b3D = true;
		break;
	}
	CollectParameters(pIp, b3D ? 6 : 5);
	CPed* pTarget = CPools::GetPedPool()->GetAt(ScriptParams[1]);
	script_assert(pTarget);
	if (pTarget->bInVehicle) {
		X = pTarget->m_pMyVehicle->GetPosition().x;
		Y = pTarget->m_pMyVehicle->GetPosition().y;
		Z = pTarget->m_pMyVehicle->GetPosition().z;
	}
	else {
		X = pTarget->GetPosition().x;
		Y = pTarget->GetPosition().y;
		Z = pTarget->GetPosition().z;
	}
	dX = *(float*)&ScriptParams[2];
	dY = *(float*)&ScriptParams[3];
	if (b3D) {
		dZ = *(float*)&ScriptParams[4];
		debug = ScriptParams[5];
	}
	else {
		debug = ScriptParams[4];
	}
	result = true;
	for (int i = 0; i < MAX_NUM_COLLECTIVES && result; i++) {
		if (ScriptParams[0] != CTheScripts::CollectiveArray[i].colIndex)
			continue;
		CPed* pPed = CPools::GetPedPool()->GetAt(CTheScripts::CollectiveArray[i].pedIndex);
		if (!pPed) {
			CTheScripts::CollectiveArray[i].colIndex = -1;
			CTheScripts::CollectiveArray[i].pedIndex = 0;
			continue;
		}
		CVector pos = pPed->bInVehicle ? pPed->m_pMyVehicle->GetPosition() : pPed->GetPosition();
		bool in_area;
		if (b3D) {
			in_area = X - dX <= pos.x &&
				X + dX >= pos.x &&
				Y - dY <= pos.y &&
				Y + dY >= pos.y &&
				Z - dZ <= pos.z &&
				Z + dZ >= pos.z;
		}
		else {
			in_area = X - dX <= pos.x &&
				X + dX >= pos.x &&
				Y - dY <= pos.y &&
				Y + dY >= pos.y;
		}
		result = false;
		if (in_area) {
			switch (command) {
			case COMMAND_LOCATE_COLL_ANY_MEANS_CHAR_2D:
				result = true;
				break;
			case COMMAND_LOCATE_COLL_ON_FOOT_CHAR_2D:
				result = !pPed->bInVehicle;
				break;
			case COMMAND_LOCATE_COLL_IN_CAR_CHAR_2D:
				result = pPed->bInVehicle;
				break;
			default:
				script_assert(false);
				break;
			}
		}
	}
	UpdateCompareFlag(result);
	if (debug)
		CTheScripts::HighlightImportantArea((uintptr)this + m_nIp, X - dX, Y - dY, X + dX, Y + dY, b3D ? Z : MAP_Z_LOW_LIMIT);
	if (CTheScripts::DbgFlag) {
		if (b3D)
			CTheScripts::DrawDebugCube(X - dX, Y - dY, Z - dZ, X + dX, Y + dY, Z + dZ);
		else
			CTheScripts::DrawDebugSquare(X - dX, Y - dY, X + dX, Y + dY);
	}
}

void CRunningScript::LocateCollectiveCarCommand(int32 command, uint32* pIp)
{
	bool b3D, result, debug;
	float X, Y, Z, dX, dY, dZ;
	switch (command) {
	case COMMAND_LOCATE_COLL_ANY_MEANS_CAR_2D:
	case COMMAND_LOCATE_COLL_ON_FOOT_CAR_2D:
	case COMMAND_LOCATE_COLL_IN_CAR_CAR_2D:
		b3D = false;
		break;
	default:
		b3D = true;
		break;
	}
	CollectParameters(pIp, b3D ? 6 : 5);
	CVehicle* pTarget = CPools::GetVehiclePool()->GetAt(ScriptParams[1]);
	script_assert(pTarget);
	X = pTarget->GetPosition().x;
	Y = pTarget->GetPosition().y;
	Z = pTarget->GetPosition().z;
	dX = *(float*)&ScriptParams[2];
	dY = *(float*)&ScriptParams[3];
	if (b3D) {
		dZ = *(float*)&ScriptParams[4];
		debug = ScriptParams[5];
	}
	else {
		debug = ScriptParams[4];
	}
	result = true;
	for (int i = 0; i < MAX_NUM_COLLECTIVES && result; i++) {
		if (ScriptParams[0] != CTheScripts::CollectiveArray[i].colIndex)
			continue;
		CPed* pPed = CPools::GetPedPool()->GetAt(CTheScripts::CollectiveArray[i].pedIndex);
		if (!pPed) {
			CTheScripts::CollectiveArray[i].colIndex = -1;
			CTheScripts::CollectiveArray[i].pedIndex = 0;
			continue;
		}
		CVector pos = pPed->bInVehicle ? pPed->m_pMyVehicle->GetPosition() : pPed->GetPosition();
		bool in_area;
		if (b3D) {
			in_area = X - dX <= pos.x &&
				X + dX >= pos.x &&
				Y - dY <= pos.y &&
				Y + dY >= pos.y &&
				Z - dZ <= pos.z &&
				Z + dZ >= pos.z;
		}
		else {
			in_area = X - dX <= pos.x &&
				X + dX >= pos.x &&
				Y - dY <= pos.y &&
				Y + dY >= pos.y;
		}
		result = false;
		if (in_area) {
			switch (command) {
			case COMMAND_LOCATE_COLL_ANY_MEANS_CAR_2D:
				result = true;
				break;
			case COMMAND_LOCATE_COLL_ON_FOOT_CAR_2D:
				result = !pPed->bInVehicle;
				break;
			case COMMAND_LOCATE_COLL_IN_CAR_CAR_2D:
				result = pPed->bInVehicle;
				break;
			default:
				script_assert(false);
				break;
			}
		}
	}
	UpdateCompareFlag(result);
	if (debug)
		CTheScripts::HighlightImportantArea((uintptr)this + m_nIp, X - dX, Y - dY, X + dX, Y + dY, b3D ? Z : MAP_Z_LOW_LIMIT);
	if (CTheScripts::DbgFlag) {
		if (b3D)
			CTheScripts::DrawDebugCube(X - dX, Y - dY, Z - dZ, X + dX, Y + dY, Z + dZ);
		else
			CTheScripts::DrawDebugSquare(X - dX, Y - dY, X + dX, Y + dY);
	}
}

void CRunningScript::LocateCollectivePlayerCommand(int32 command, uint32* pIp)
{
	bool b3D, result, debug;
	float X, Y, Z, dX, dY, dZ;
	switch (command) {
	case COMMAND_LOCATE_COLL_ANY_MEANS_PLAYER_2D:
	case COMMAND_LOCATE_COLL_ON_FOOT_PLAYER_2D:
	case COMMAND_LOCATE_COLL_IN_CAR_PLAYER_2D:
		b3D = false;
		break;
	default:
		b3D = true;
		break;
	}
	CollectParameters(pIp, b3D ? 6 : 5);
	CVector pos = CWorld::Players[ScriptParams[1]].GetPos();
	X = pos.x;
	Y = pos.y;
	Z = pos.z;
	dX = *(float*)&ScriptParams[2];
	dY = *(float*)&ScriptParams[3];
	if (b3D) {
		dZ = *(float*)&ScriptParams[4];
		debug = ScriptParams[5];
	}
	else {
		debug = ScriptParams[4];
	}
	result = true;
	for (int i = 0; i < MAX_NUM_COLLECTIVES && result; i++) {
		if (ScriptParams[0] != CTheScripts::CollectiveArray[i].colIndex)
			continue;
		CPed* pPed = CPools::GetPedPool()->GetAt(CTheScripts::CollectiveArray[i].pedIndex);
		if (!pPed) {
			CTheScripts::CollectiveArray[i].colIndex = -1;
			CTheScripts::CollectiveArray[i].pedIndex = 0;
			continue;
		}
		CVector pos = pPed->bInVehicle ? pPed->m_pMyVehicle->GetPosition() : pPed->GetPosition();
		bool in_area;
		if (b3D) {
			in_area = X - dX <= pos.x &&
				X + dX >= pos.x &&
				Y - dY <= pos.y &&
				Y + dY >= pos.y &&
				Z - dZ <= pos.z &&
				Z + dZ >= pos.z;
		}
		else {
			in_area = X - dX <= pos.x &&
				X + dX >= pos.x &&
				Y - dY <= pos.y &&
				Y + dY >= pos.y;
		}
		result = false;
		if (in_area) {
			switch (command) {
			case COMMAND_LOCATE_COLL_ANY_MEANS_PLAYER_2D:
				result = true;
				break;
			case COMMAND_LOCATE_COLL_ON_FOOT_PLAYER_2D:
				result = !pPed->bInVehicle;
				break;
			case COMMAND_LOCATE_COLL_IN_CAR_PLAYER_2D:
				result = pPed->bInVehicle;
				break;
			default:
				script_assert(false);
				break;
			}
		}
	}
	UpdateCompareFlag(result);
	if (debug)
		CTheScripts::HighlightImportantArea((uintptr)this + m_nIp, X - dX, Y - dY, X + dX, Y + dY, b3D ? Z : MAP_Z_LOW_LIMIT);
	if (CTheScripts::DbgFlag) {
		if (b3D)
			CTheScripts::DrawDebugCube(X - dX, Y - dY, Z - dZ, X + dX, Y + dY, Z + dZ);
		else
			CTheScripts::DrawDebugSquare(X - dX, Y - dY, X + dX, Y + dY);
	}
}

void CRunningScript::CollectiveInAreaCheckCommand(int32 command, uint32* pIp)
{
	bool b3D, result, debug, decided = false;
	float infX, infY, infZ, supX, supY, supZ;
	switch (command) {
	case COMMAND_IS_COLL_IN_AREA_2D:
	case COMMAND_IS_COLL_IN_AREA_ON_FOOT_2D:
	case COMMAND_IS_COLL_IN_AREA_IN_CAR_2D:
	case COMMAND_IS_COLL_STOPPED_IN_AREA_2D:
	case COMMAND_IS_COLL_STOPPED_IN_AREA_ON_FOOT_2D:
	case COMMAND_IS_COLL_STOPPED_IN_AREA_IN_CAR_2D:
		b3D = false;
		break;
	default:
		b3D = true;
		break;
	}
	CollectParameters(pIp, b3D ? 8 : 6);
	infX = *(float*)&ScriptParams[1];
	infY = *(float*)&ScriptParams[2];
	if (b3D) {
		infZ = *(float*)&ScriptParams[3];
		supX = *(float*)&ScriptParams[4];
		supY = *(float*)&ScriptParams[5];
		supZ = *(float*)&ScriptParams[6];
		if (infZ > supZ) {
			infZ = *(float*)&ScriptParams[6];
			supZ = *(float*)&ScriptParams[3];
		}
		debug = ScriptParams[7];
	}
	else {
		supX = *(float*)&ScriptParams[3];
		supY = *(float*)&ScriptParams[4];
		debug = ScriptParams[5];
	}
	if (infX > supX) {
		float tmp = infX;
		infX = supX;
		supX = tmp;
	}
	if (infY > supY) {
		float tmp = infY;
		infY = supY;
		supY = tmp;
	}
	result = true;
	for (int i = 0; i < MAX_NUM_COLLECTIVES && result; i++) {
		if (ScriptParams[0] != CTheScripts::CollectiveArray[i].colIndex)
			continue;
		CPed* pPed = CPools::GetPedPool()->GetAt(CTheScripts::CollectiveArray[i].pedIndex);
		if (!pPed) {
			CTheScripts::CollectiveArray[i].colIndex = -1;
			CTheScripts::CollectiveArray[i].pedIndex = 0;
			continue;
		}
		CVector pos = pPed->bInVehicle ? pPed->m_pMyVehicle->GetPosition() : pPed->GetPosition();
		switch (command) {
		case COMMAND_IS_COLL_STOPPED_IN_AREA_2D:
		case COMMAND_IS_COLL_STOPPED_IN_AREA_ON_FOOT_2D:
		case COMMAND_IS_COLL_STOPPED_IN_AREA_IN_CAR_2D:
			if (!CTheScripts::IsPedStopped(pPed)) {
				result = false;
				decided = true;
			}
			break;
		default:
			break;
		}
		if (!decided) {
			bool in_area;
			if (b3D) {
				in_area = infX <= pos.x &&
					supX >= pos.x &&
					infY <= pos.y &&
					supY >= pos.y &&
					infZ <= pos.z &&
					supZ >= pos.z;
			}
			else {
				in_area = infX <= pos.x &&
					supX >= pos.x &&
					infY <= pos.y &&
					supY >= pos.y;
			}
			result = false;
			if (in_area) {
				switch (command) {
				case COMMAND_IS_COLL_IN_AREA_2D:
				case COMMAND_IS_COLL_STOPPED_IN_AREA_2D:
					result = true;
					break;
				case COMMAND_IS_COLL_IN_AREA_ON_FOOT_2D:
				case COMMAND_IS_COLL_STOPPED_IN_AREA_ON_FOOT_2D:
					result = !pPed->bInVehicle;
					break;
				case COMMAND_IS_COLL_IN_AREA_IN_CAR_2D:
				case COMMAND_IS_COLL_STOPPED_IN_AREA_IN_CAR_2D:
					result = pPed->bInVehicle;
					break;
				default:
					script_assert(false);
					break;
				}
			}
		}
	}
	UpdateCompareFlag(result);
	if (debug)
		CTheScripts::HighlightImportantArea((uintptr)this + m_nIp, infX, infY, supX, supY, b3D ? (infZ + supZ) / 2 : MAP_Z_LOW_LIMIT);
	if (CTheScripts::DbgFlag) {
		if (b3D)
			CTheScripts::DrawDebugCube(infX, infY, infZ, supX, supY, supZ);
		else
			CTheScripts::DrawDebugSquare(infX, infY, supX, supY);
	}
}
#endif

void CTheScripts::PrintListSizes()
{
	int active = 0;
	int idle = 0;

	for (CRunningScript* pScript = pActiveScripts; pScript; pScript = pScript->GetNext())
		active++;
	for (CRunningScript* pScript = pIdleScripts; pScript; pScript = pScript->GetNext())
		idle++;

	debug("active: %d, idle: %d", active, idle);
}

uint32 DbgLineColour = 0x0000FFFF; // r = 0, g = 0, b = 255, a = 255

void CTheScripts::DrawDebugSquare(float infX, float infY, float supX, float supY)
{
	CColPoint tmpCP;
	CEntity* tmpEP;
	CVector p1, p2, p3, p4;
	p1 = CVector(infX, infY, -1000.0f);
	CWorld::ProcessVerticalLine(p1, 1000.0f, tmpCP, tmpEP, true, false, false, false, true, false, nil);
	p1.z = 2.0f + tmpCP.point.z;
	p2 = CVector(supX, supY, -1000.0f);
	CWorld::ProcessVerticalLine(p2, 1000.0f, tmpCP, tmpEP, true, false, false, false, true, false, nil);
	p2.z = 2.0f + tmpCP.point.z;
	p3 = CVector(infX, supY, -1000.0f);
	CWorld::ProcessVerticalLine(p3, 1000.0f, tmpCP, tmpEP, true, false, false, false, true, false, nil);
	p3.z = 2.0f + tmpCP.point.z;
	p4 = CVector(supX, infY, -1000.0f);
	CWorld::ProcessVerticalLine(p4, 1000.0f, tmpCP, tmpEP, true, false, false, false, true, false, nil);
	p4.z = 2.0f + tmpCP.point.z;
	CTheScripts::ScriptDebugLine3D(p1.x, p1.y, p1.z, p2.x, p2.y, p2.z, DbgLineColour, DbgLineColour);
	CTheScripts::ScriptDebugLine3D(p2.x, p2.y, p2.z, p3.x, p3.y, p3.z, DbgLineColour, DbgLineColour);
	CTheScripts::ScriptDebugLine3D(p3.x, p3.y, p3.z, p4.x, p4.y, p4.z, DbgLineColour, DbgLineColour);
	CTheScripts::ScriptDebugLine3D(p4.x, p4.y, p4.z, p1.x, p1.y, p1.z, DbgLineColour, DbgLineColour);
}

void CTheScripts::DrawDebugAngledSquare(float infX, float infY, float supX, float supY, float rotSupX, float rotSupY, float rotInfX, float rotInfY)
{
	CColPoint tmpCP;
	CEntity* tmpEP;
	CVector p1, p2, p3, p4;
	p1 = CVector(infX, infY, -1000.0f);
	CWorld::ProcessVerticalLine(p1, 1000.0f, tmpCP, tmpEP, true, false, false, false, true, false, nil);
	p1.z = 2.0f + tmpCP.point.z;
	p2 = CVector(supX, supY, -1000.0f);
	CWorld::ProcessVerticalLine(p2, 1000.0f, tmpCP, tmpEP, true, false, false, false, true, false, nil);
	p2.z = 2.0f + tmpCP.point.z;
	p3 = CVector(rotSupX, rotSupY, -1000.0f);
	CWorld::ProcessVerticalLine(p3, 1000.0f, tmpCP, tmpEP, true, false, false, false, true, false, nil);
	p3.z = 2.0f + tmpCP.point.z;
	p4 = CVector(rotInfX, rotInfY, -1000.0f);
	CWorld::ProcessVerticalLine(p4, 1000.0f, tmpCP, tmpEP, true, false, false, false, true, false, nil);
	p4.z = 2.0f + tmpCP.point.z;
	CTheScripts::ScriptDebugLine3D(p1.x, p1.y, p1.z, p2.x, p2.y, p2.z, DbgLineColour, DbgLineColour);
	CTheScripts::ScriptDebugLine3D(p2.x, p2.y, p2.z, p3.x, p3.y, p3.z, DbgLineColour, DbgLineColour);
	CTheScripts::ScriptDebugLine3D(p3.x, p3.y, p3.z, p4.x, p4.y, p4.z, DbgLineColour, DbgLineColour);
	CTheScripts::ScriptDebugLine3D(p4.x, p4.y, p4.z, p1.x, p1.y, p1.z, DbgLineColour, DbgLineColour);
}

void CTheScripts::DrawDebugCube(float infX, float infY, float infZ, float supX, float supY, float supZ)
{
	CTheScripts::ScriptDebugLine3D(infX, infY, infZ, supX, infY, infZ, DbgLineColour, DbgLineColour);
	CTheScripts::ScriptDebugLine3D(supX, infY, infZ, supX, supY, infZ, DbgLineColour, DbgLineColour);
	CTheScripts::ScriptDebugLine3D(supX, supY, infZ, infX, supY, infZ, DbgLineColour, DbgLineColour);
	CTheScripts::ScriptDebugLine3D(infX, supY, infZ, infX, infY, infZ, DbgLineColour, DbgLineColour);
	CTheScripts::ScriptDebugLine3D(infX, infY, supZ, supX, infY, supZ, DbgLineColour, DbgLineColour);
	CTheScripts::ScriptDebugLine3D(supX, infY, supZ, supX, supY, supZ, DbgLineColour, DbgLineColour);
	CTheScripts::ScriptDebugLine3D(supX, supY, supZ, infX, supY, supZ, DbgLineColour, DbgLineColour);
	CTheScripts::ScriptDebugLine3D(infX, supY, supZ, infX, infY, supZ, DbgLineColour, DbgLineColour);
	CTheScripts::ScriptDebugLine3D(infX, infY, supZ, infX, infY, infZ, DbgLineColour, DbgLineColour);
	CTheScripts::ScriptDebugLine3D(supX, infY, supZ, supX, infY, infZ, DbgLineColour, DbgLineColour);
	CTheScripts::ScriptDebugLine3D(supX, supY, supZ, supX, supY, infZ, DbgLineColour, DbgLineColour);
	CTheScripts::ScriptDebugLine3D(infX, supY, supZ, infX, supY, infZ, DbgLineColour, DbgLineColour);
}

void CTheScripts::DrawDebugAngledCube(float infX, float infY, float infZ, float supX, float supY, float supZ, float rotSupX, float rotSupY, float rotInfX, float rotInfY)
{
	CTheScripts::ScriptDebugLine3D(infX, infY, infZ, supX, infY, infZ, DbgLineColour, DbgLineColour);
	CTheScripts::ScriptDebugLine3D(supX, infY, infZ, rotSupX, rotSupY, infZ, DbgLineColour, DbgLineColour);
	CTheScripts::ScriptDebugLine3D(rotSupX, rotSupY, infZ, rotInfX, rotInfY, infZ, DbgLineColour, DbgLineColour);
	CTheScripts::ScriptDebugLine3D(rotInfX, rotInfY, infZ, infX, infY, infZ, DbgLineColour, DbgLineColour);
	CTheScripts::ScriptDebugLine3D(infX, infY, supZ, supX, infY, supZ, DbgLineColour, DbgLineColour);
	CTheScripts::ScriptDebugLine3D(supX, infY, supZ, rotSupX, rotSupY, supZ, DbgLineColour, DbgLineColour);
	CTheScripts::ScriptDebugLine3D(rotSupX, rotSupY, rotInfX, rotInfY, supY, supZ, DbgLineColour, DbgLineColour);
	CTheScripts::ScriptDebugLine3D(rotInfX, rotInfY, supZ, infX, infY, supZ, DbgLineColour, DbgLineColour);
	CTheScripts::ScriptDebugLine3D(infX, infY, supZ, infX, infY, infZ, DbgLineColour, DbgLineColour);
	CTheScripts::ScriptDebugLine3D(supX, infY, supZ, supX, infY, infZ, DbgLineColour, DbgLineColour);
	CTheScripts::ScriptDebugLine3D(rotSupX, rotSupY, supZ, rotSupX, rotSupY, infZ, DbgLineColour, DbgLineColour);
	CTheScripts::ScriptDebugLine3D(rotInfX, rotInfY, supZ, rotInfX, rotInfY, infZ, DbgLineColour, DbgLineColour);
}

void CTheScripts::ScriptDebugLine3D(float x1, float y1, float z1, float x2, float y2, float z2, uint32 col, uint32 col2)
{
	if (NumScriptDebugLines >= MAX_NUM_STORED_LINES)
		return;
	aStoredLines[NumScriptDebugLines].vecInf = CVector(x1, y1, z1);
	aStoredLines[NumScriptDebugLines].vecSup = CVector(x2, y2, z2);
	aStoredLines[NumScriptDebugLines].color1 = col;
	aStoredLines[NumScriptDebugLines++].color2 = col2;
}

void CTheScripts::RenderTheScriptDebugLines()
{
	RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)1);
	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)1);
	for (int i = 0; i < NumScriptDebugLines; i++) {
		CLines::RenderLineWithClipping(
			aStoredLines[i].vecInf.x,
			aStoredLines[i].vecInf.y,
			aStoredLines[i].vecInf.z,
			aStoredLines[i].vecSup.x,
			aStoredLines[i].vecSup.y,
			aStoredLines[i].vecSup.z,
			aStoredLines[i].color1,
			aStoredLines[i].color2);
	}
	NumScriptDebugLines = 0;
	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)0);
}

#define SCRIPT_DATA_SIZE sizeof(CTheScripts::OnAMissionFlag) + sizeof(CTheScripts::BaseBriefIdForContact) + sizeof(CTheScripts::OnAMissionForContactFlag) +\
	sizeof(CTheScripts::CollectiveArray) + 4 * sizeof(uint32) * MAX_NUM_BUILDING_SWAPS + 2 * sizeof(uint32) * MAX_NUM_INVISIBILITY_SETTINGS + 5 * sizeof(uint32)

void CTheScripts::SaveAllScripts(uint8* buf, uint32* size)
{
INITSAVEBUF
	uint32 varSpace = GetSizeOfVariableSpace();
	uint32 runningScripts = 0;
	for (CRunningScript* pScript = pActiveScripts; pScript; pScript = pScript->GetNext())
		runningScripts++;
	*size = CRunningScript::nSaveStructSize * runningScripts + varSpace + SCRIPT_DATA_SIZE + SAVE_HEADER_SIZE + 3 * sizeof(uint32);
	WriteSaveHeader(buf, 'S', 'C', 'R', '\0', *size - SAVE_HEADER_SIZE);
	WriteSaveBuf(buf, varSpace);
	for (uint32 i = 0; i < varSpace; i++)
		WriteSaveBuf(buf, ScriptSpace[i]);
#ifdef CHECK_STRUCT_SIZES
	static_assert(SCRIPT_DATA_SIZE == 968, "CTheScripts::SaveAllScripts");
#endif
	uint32 script_data_size = SCRIPT_DATA_SIZE;
	WriteSaveBuf(buf, script_data_size);
	WriteSaveBuf(buf, OnAMissionFlag);
	for (uint32 i = 0; i < MAX_NUM_CONTACTS; i++) {
		WriteSaveBuf(buf, OnAMissionForContactFlag[i]);
		WriteSaveBuf(buf, BaseBriefIdForContact[i]);
	}
	for (uint32 i = 0; i < MAX_NUM_COLLECTIVES; i++)
		WriteSaveBuf(buf, CollectiveArray[i]);
	WriteSaveBuf(buf, NextFreeCollectiveIndex);
	for (uint32 i = 0; i < MAX_NUM_BUILDING_SWAPS; i++) {
		CBuilding* pBuilding = BuildingSwapArray[i].m_pBuilding;
		uint32 type, handle;
		if (!pBuilding) {
			type = 0;
			handle = 0;
		} else if (pBuilding->GetIsATreadable()) {
			type = 1;
			handle = CPools::GetTreadablePool()->GetJustIndex_NoFreeAssert((CTreadable*)pBuilding) + 1;
		} else {
			type = 2;
			handle = CPools::GetBuildingPool()->GetJustIndex_NoFreeAssert(pBuilding) + 1;
		}
		WriteSaveBuf(buf, type);
		WriteSaveBuf(buf, handle);
		WriteSaveBuf(buf, BuildingSwapArray[i].m_nNewModel);
		WriteSaveBuf(buf, BuildingSwapArray[i].m_nOldModel);
	}
	for (uint32 i = 0; i < MAX_NUM_INVISIBILITY_SETTINGS; i++) {
		CEntity* pEntity = InvisibilitySettingArray[i];
		uint32 type, handle;
		if (!pEntity) {
			type = 0;
			handle = 0;
		} else {
			switch (pEntity->GetType()) {
			case ENTITY_TYPE_BUILDING:
				if (((CBuilding*)pEntity)->GetIsATreadable()) {
					type = 1;
					handle = CPools::GetTreadablePool()->GetJustIndex_NoFreeAssert((CTreadable*)pEntity) + 1;
				} else {
					type = 2;
					handle = CPools::GetBuildingPool()->GetJustIndex_NoFreeAssert((CBuilding*)pEntity) + 1;
				}
				break;
			case ENTITY_TYPE_OBJECT:
				type = 3;
				handle = CPools::GetObjectPool()->GetJustIndex_NoFreeAssert((CObject*)pEntity) + 1;
				break;
			case ENTITY_TYPE_DUMMY:
				type = 4;
				handle = CPools::GetDummyPool()->GetJustIndex_NoFreeAssert((CDummy*)pEntity) + 1;
			default: break;
			}
		}
		WriteSaveBuf(buf, type);
		WriteSaveBuf(buf, handle);
	}
	WriteSaveBuf(buf, bUsingAMultiScriptFile);
	WriteSaveBuf(buf, (uint8)0);
	WriteSaveBuf(buf, (uint16)0);
	WriteSaveBuf(buf, MainScriptSize);
	WriteSaveBuf(buf, LargestMissionScriptSize);
	WriteSaveBuf(buf, NumberOfMissionScripts);
	WriteSaveBuf(buf, (uint16)0);
	WriteSaveBuf(buf, runningScripts);
	for (CRunningScript* pScript = pActiveScripts; pScript; pScript = pScript->GetNext())
		pScript->Save(buf);
VALIDATESAVEBUF(*size)
}

void CTheScripts::LoadAllScripts(uint8* buf, uint32 size)
{
	Init();
INITSAVEBUF
	CheckSaveHeader(buf, 'S', 'C', 'R', '\0', size - SAVE_HEADER_SIZE);
	uint32 varSpace = ReadSaveBuf<uint32>(buf);
	for (uint32 i = 0; i < varSpace; i++)
		ScriptSpace[i] = ReadSaveBuf<uint8>(buf);
	script_assert(ReadSaveBuf<uint32>(buf) == SCRIPT_DATA_SIZE);
	OnAMissionFlag = ReadSaveBuf<uint32>(buf);
	for (uint32 i = 0; i < MAX_NUM_CONTACTS; i++) {
		OnAMissionForContactFlag[i] = ReadSaveBuf<uint32>(buf);
		BaseBriefIdForContact[i] = ReadSaveBuf<uint32>(buf);
	}
	for (uint32 i = 0; i < MAX_NUM_COLLECTIVES; i++)
		CollectiveArray[i] = ReadSaveBuf<tCollectiveData>(buf);
	NextFreeCollectiveIndex = ReadSaveBuf<uint32>(buf);
	for (uint32 i = 0; i < MAX_NUM_BUILDING_SWAPS; i++) {
		uint32 type = ReadSaveBuf<uint32>(buf);
		uint32 handle = ReadSaveBuf<uint32>(buf);
		switch (type) {
		case 0:
			BuildingSwapArray[i].m_pBuilding = nil;
			break;
		case 1:
			BuildingSwapArray[i].m_pBuilding = CPools::GetTreadablePool()->GetSlot(handle - 1);
			break;
		case 2:
			BuildingSwapArray[i].m_pBuilding = CPools::GetBuildingPool()->GetSlot(handle - 1);
			break;
		default:
			script_assert(false);
		}
		BuildingSwapArray[i].m_nNewModel = ReadSaveBuf<uint32>(buf);
		BuildingSwapArray[i].m_nOldModel = ReadSaveBuf<uint32>(buf);
		if (BuildingSwapArray[i].m_pBuilding)
			BuildingSwapArray[i].m_pBuilding->ReplaceWithNewModel(BuildingSwapArray[i].m_nNewModel);
	}
	for (uint32 i = 0; i < MAX_NUM_INVISIBILITY_SETTINGS; i++) {
		uint32 type = ReadSaveBuf<uint32>(buf);
		uint32 handle = ReadSaveBuf<uint32>(buf);
		switch (type) {
		case 0:
			InvisibilitySettingArray[i] = nil;
			break;
		case 1:
			InvisibilitySettingArray[i] = CPools::GetTreadablePool()->GetSlot(handle - 1);
			break;
		case 2:
			InvisibilitySettingArray[i] = CPools::GetBuildingPool()->GetSlot(handle - 1);
			break;
		case 3:
			InvisibilitySettingArray[i] = CPools::GetObjectPool()->GetSlot(handle - 1);
			break;
		case 4:
			InvisibilitySettingArray[i] = CPools::GetDummyPool()->GetSlot(handle - 1);
			break;
		default:
			script_assert(false);
		}
		if (InvisibilitySettingArray[i])
			InvisibilitySettingArray[i]->bIsVisible = false;
	}
	script_assert(ReadSaveBuf<bool>(buf) == bUsingAMultiScriptFile);
	ReadSaveBuf<uint8>(buf);
	ReadSaveBuf<uint16>(buf);
	script_assert(ReadSaveBuf<uint32>(buf) == MainScriptSize);
	script_assert(ReadSaveBuf<uint32>(buf) == LargestMissionScriptSize);
	script_assert(ReadSaveBuf<uint16>(buf) == NumberOfMissionScripts);
	ReadSaveBuf<uint16>(buf);
	uint32 runningScripts = ReadSaveBuf<uint32>(buf);
	for (uint32 i = 0; i < runningScripts; i++)
		StartNewScript(0)->Load(buf);
VALIDATESAVEBUF(size)
}

#undef SCRIPT_DATA_SIZE

void CRunningScript::Save(uint8*& buf)
{
#ifdef COMPATIBLE_SAVES
	SkipSaveBuf(buf, 8);
	for (int i = 0; i < 8; i++)
		WriteSaveBuf<char>(buf, m_abScriptName[i]);
	WriteSaveBuf<uint32>(buf, m_nIp);
#ifdef CHECK_STRUCT_SIZES
	static_assert(MAX_STACK_DEPTH == 6, "Compatibility loss: MAX_STACK_DEPTH != 6");
#endif
	for (int i = 0; i < MAX_STACK_DEPTH; i++)
		WriteSaveBuf<uint32>(buf, m_anStack[i]);
	WriteSaveBuf<uint16>(buf, m_nStackPointer);
	SkipSaveBuf(buf, 2);
#ifdef CHECK_STRUCT_SIZES
	static_assert(NUM_LOCAL_VARS + NUM_TIMERS == 18, "Compatibility loss: NUM_LOCAL_VARS + NUM_TIMERS != 18");
#endif
	for (int i = 0; i < NUM_LOCAL_VARS + NUM_TIMERS; i++)
		WriteSaveBuf<int32>(buf, m_anLocalVariables[i]);
	WriteSaveBuf<bool>(buf, m_bCondResult);
	WriteSaveBuf<bool>(buf, m_bIsMissionScript);
	WriteSaveBuf<bool>(buf, m_bSkipWakeTime);
	SkipSaveBuf(buf, 1);
	WriteSaveBuf<uint32>(buf, m_nWakeTime);
	WriteSaveBuf<uint16>(buf, m_nAndOrState);
	WriteSaveBuf<bool>(buf, m_bNotFlag);
	WriteSaveBuf<bool>(buf, m_bDeatharrestEnabled);
	WriteSaveBuf<bool>(buf, m_bDeatharrestExecuted);
	WriteSaveBuf<bool>(buf, m_bMissionFlag);
	SkipSaveBuf(buf, 2);
#else
	WriteSaveBuf(buf, *this);
#endif
}

void CRunningScript::Load(uint8*& buf)
{
#ifdef COMPATIBLE_SAVES
	SkipSaveBuf(buf, 8);
	for (int i = 0; i < 8; i++)
		m_abScriptName[i] = ReadSaveBuf<char>(buf);
	m_nIp = ReadSaveBuf<uint32>(buf);
#ifdef CHECK_STRUCT_SIZES
	static_assert(MAX_STACK_DEPTH == 6, "Compatibility loss: MAX_STACK_DEPTH != 6");
#endif
	for (int i = 0; i < MAX_STACK_DEPTH; i++)
		m_anStack[i] = ReadSaveBuf<uint32>(buf);
	m_nStackPointer = ReadSaveBuf<uint16>(buf);
	SkipSaveBuf(buf, 2);
#ifdef CHECK_STRUCT_SIZES
	static_assert(NUM_LOCAL_VARS + NUM_TIMERS == 18, "Compatibility loss: NUM_LOCAL_VARS + NUM_TIMERS != 18");
#endif
	for (int i = 0; i < NUM_LOCAL_VARS + NUM_TIMERS; i++)
		m_anLocalVariables[i] = ReadSaveBuf<int32>(buf);
	m_bCondResult = ReadSaveBuf<bool>(buf);
	m_bIsMissionScript = ReadSaveBuf<bool>(buf);
	m_bSkipWakeTime = ReadSaveBuf<bool>(buf);
	SkipSaveBuf(buf, 1);
	m_nWakeTime = ReadSaveBuf<uint32>(buf);
	m_nAndOrState = ReadSaveBuf<uint16>(buf);
	m_bNotFlag = ReadSaveBuf<bool>(buf);
	m_bDeatharrestEnabled = ReadSaveBuf<bool>(buf);
	m_bDeatharrestExecuted = ReadSaveBuf<bool>(buf);
	m_bMissionFlag = ReadSaveBuf<bool>(buf);
	SkipSaveBuf(buf, 2);
#else
	CRunningScript* n = next;
	CRunningScript* p = prev;
	*this = ReadSaveBuf<CRunningScript>(buf);
	next = n;
	prev = p;
#endif
}

void CTheScripts::ClearSpaceForMissionEntity(const CVector& pos, CEntity* pEntity)
{
	static CColPoint aTempColPoints[MAX_COLLISION_POINTS];
	int16 entities = 0;
	CEntity* aEntities[16];
	CWorld::FindObjectsKindaColliding(pos, pEntity->GetBoundRadius(), false, &entities, 16, aEntities, false, true, true, false, false);
	if (entities <= 0)
		return;
	for (uint16 i = 0; i < entities; i++) {
		if (aEntities[i] != pEntity && aEntities[i]->IsPed() && ((CPed*)aEntities[i])->bInVehicle)
			aEntities[i] = nil;
	}
	for (uint16 i = 0; i < entities; i++) {
		if (aEntities[i] == pEntity || !aEntities[i])
			continue;
		CEntity* pFound = aEntities[i];
		int cols;
		if (pEntity->GetColModel()->numLines <= 0)
			cols = CCollision::ProcessColModels(pEntity->GetMatrix(), *pEntity->GetColModel(),
				pFound->GetMatrix(), *pFound->GetColModel(), aTempColPoints, nil, nil);
		else {
			float lines[4];
			lines[0] = lines[1] = lines[2] = lines[3] = 1.0f;
			CColPoint tmp[4];
			cols = CCollision::ProcessColModels(pEntity->GetMatrix(), *pEntity->GetColModel(),
				pFound->GetMatrix(), *pFound->GetColModel(), aTempColPoints,tmp, lines);
		}
		if (cols <= 0)
			continue;
		switch (pFound->GetType()) {
		case ENTITY_TYPE_VEHICLE:
		{
			printf("Will try to delete a vehicle where a mission entity should be\n");
			CVehicle* pVehicle = (CVehicle*)pFound;
			if (pVehicle->bIsLocked || !pVehicle->CanBeDeleted())
				break;
			if (pVehicle->pDriver) {
				CPopulation::RemovePed(pVehicle->pDriver);
				pVehicle->pDriver = nil;
			}
			for (int i = 0; i < pVehicle->m_nNumMaxPassengers; i++) {
				if (pVehicle->pPassengers[i]) {
					CPopulation::RemovePed(pVehicle->pPassengers[i]);
					pVehicle->pPassengers[i] = 0;
					pVehicle->m_nNumPassengers--;
				}
			}
			CCarCtrl::RemoveFromInterestingVehicleList(pVehicle);
			CWorld::Remove(pVehicle);
			delete pVehicle;
			break;
		}
		case ENTITY_TYPE_PED:
		{
			CPed* pPed = (CPed*)pFound;
			if (pPed->IsPlayer() || !pPed->CanBeDeleted())
				break;
			CPopulation::RemovePed(pPed);
			printf("Deleted a ped where a mission entity should be\n");
			break;
		}
		default: break;
		}
	}
}

void CTheScripts::HighlightImportantArea(uint32 id, float x1, float y1, float x2, float y2, float z)
{
	float infX, infY, supX, supY;
	if (x1 < x2) {
		infX = x1;
		supX = x2;
	} else {
		infX = x2;
		supX = x1;
	}
	if (y1 < y2) {
		infY = y1;
		supY = y2;
	}
	else {
		infY = y2;
		supY = y1;
	}
	CVector center;
	center.x = (infX + supX) / 2;
	center.y = (infY + supY) / 2;
	center.z = (z <= MAP_Z_LOW_LIMIT) ? CWorld::FindGroundZForCoord(center.x, center.y) : z;
	CShadows::RenderIndicatorShadow(id, 2, gpGoalTex, &center, supX - center.x, 0.0f, 0.0f, center.y - supY, 0);
}

void CTheScripts::HighlightImportantAngledArea(uint32 id, float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, float z)
{
	float infX, infY, supX, supY, X, Y;
	X = (x1 + x2) / 2;
	Y = (y1 + y2) / 2;
	supX = infX = X;
	supY = infY = Y;
	X = (x2 + x3) / 2;
	Y = (y2 + y3) / 2;
	infX = Min(infX, X);
	supX = Max(supX, X);
	infY = Min(infY, Y);
	supY = Max(supY, Y);
	X = (x3 + x4) / 2;
	Y = (y3 + y4) / 2;
	infX = Min(infX, X);
	supX = Max(supX, X);
	infY = Min(infY, Y);
	supY = Max(supY, Y);
	X = (x4 + x1) / 2;
	Y = (y4 + y1) / 2;
	infX = Min(infX, X);
	supX = Max(supX, X);
	infY = Min(infY, Y);
	supY = Max(supY, Y);
	CVector center;
	center.x = (infX + supX) / 2;
	center.y = (infY + supY) / 2;
	center.z = (z <= MAP_Z_LOW_LIMIT) ? CWorld::FindGroundZForCoord(center.x, center.y) : z;
	CShadows::RenderIndicatorShadow(id, 2, gpGoalTex, &center, supX - center.x, 0.0f, 0.0f, center.y - supY, 0);
}

#ifdef GTA_SCRIPT_COLLECTIVE
int CTheScripts::AddPedsInVehicleToCollective(int index)
{
	int colIndex = NextFreeCollectiveIndex;
	AdvanceCollectiveIndex();
	CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(index);
	script_assert(pVehicle);
	CPed* pDriver = pVehicle->pDriver;
	if (pDriver && !pDriver->IsPlayer() && pDriver->CharCreatedBy != MISSION_CHAR && pDriver->m_nPedType != PEDTYPE_COP) {
		int index = FindFreeSlotInCollectiveArray();
		if (index > -1) {
			CollectiveArray[index].colIndex = colIndex;
			CollectiveArray[index].pedIndex = CPools::GetPedPool()->GetIndex(pDriver);
		}
	}
	for (int i = 0; i < pVehicle->m_nNumMaxPassengers; i++) {
		CPed* pPassenger = pVehicle->pPassengers[i];
		if (pPassenger && !pPassenger->IsPlayer() && pPassenger->CharCreatedBy != MISSION_CHAR && pPassenger->m_nPedType != PEDTYPE_COP) {
			int index = FindFreeSlotInCollectiveArray();
			if (index > -1) {
				CollectiveArray[index].colIndex = colIndex;
				CollectiveArray[index].pedIndex = CPools::GetPedPool()->GetIndex(pPassenger);
			}
		}
	}
	return colIndex;
}

int CTheScripts::AddPedsInAreaToCollective(float x, float y, float z, float radius)
{
	int16 numFound;
	CEntity* pEntities[64];
	int colIndex = NextFreeCollectiveIndex;
	AdvanceCollectiveIndex();
	CWorld::FindObjectsInRange(CVector(x, y, z), radius, true, &numFound, 64, pEntities, false, true, true, false, false);
	for (int16 i = 0; i < numFound; i++) {
		if (pEntities[i]->GetType() == ENTITY_TYPE_PED) {
			CPed* pPed = (CPed*)pEntities[i];
			if (pPed && !pPed->IsPlayer() && pPed->CharCreatedBy != MISSION_CHAR && pPed->m_nPedType != PEDTYPE_COP) {
				int index = FindFreeSlotInCollectiveArray();
				if (index > -1) {
					CollectiveArray[index].colIndex = colIndex;
					CollectiveArray[index].pedIndex = CPools::GetPedPool()->GetIndex(pPed);
				}
			}
		}
		else if (pEntities[i]->GetType() == ENTITY_TYPE_VEHICLE) {
			CVehicle* pVehicle = (CVehicle*)pEntities[i];
			CPed* pDriver = pVehicle->pDriver;
			if (pDriver && !pDriver->IsPlayer() && pDriver->CharCreatedBy != MISSION_CHAR && pDriver->m_nPedType != PEDTYPE_COP) {
				int index = FindFreeSlotInCollectiveArray();
				if (index > -1) {
					CollectiveArray[index].colIndex = colIndex;
					CollectiveArray[index].pedIndex = CPools::GetPedPool()->GetIndex(pDriver);
				}
			}
			for (int i = 0; i < pVehicle->m_nNumMaxPassengers; i++) {
				CPed* pPassenger = pVehicle->pPassengers[i];
				if (pPassenger && !pPassenger->IsPlayer() && pPassenger->CharCreatedBy != MISSION_CHAR && pPassenger->m_nPedType != PEDTYPE_COP) {
					int index = FindFreeSlotInCollectiveArray();
					if (index > -1) {
						CollectiveArray[index].colIndex = colIndex;
						CollectiveArray[index].pedIndex = CPools::GetPedPool()->GetIndex(pPassenger);
					}
				}
			}
		}
	}
	return colIndex;
}

int CTheScripts::FindFreeSlotInCollectiveArray()
{
	for (int i = 0; i < MAX_NUM_COLLECTIVES; i++) {
		if (CollectiveArray[i].colIndex == -1)
			return i;
	}
	return -1;
}

void CTheScripts::SetObjectiveForAllPedsInCollective(int colIndex, eObjective objective, int16 p1, int16 p2)
{
	for (int i = 0; i < MAX_NUM_COLLECTIVES; i++) {
		if (CollectiveArray[i].colIndex == colIndex) {
			CPed* pPed = CPools::GetPedPool()->GetAt(CollectiveArray[i].pedIndex);
			if (pPed == nil) {
				CollectiveArray[i].colIndex = -1;
				CollectiveArray[i].pedIndex = 0;
			}
			else {
				pPed->bScriptObjectiveCompleted = false;
				pPed->SetObjective(objective, p1, p2);
			}
		}
	}
}

void CTheScripts::SetObjectiveForAllPedsInCollective(int colIndex, eObjective objective, CVector p1, float p2)
{
	for (int i = 0; i < MAX_NUM_COLLECTIVES; i++) {
		if (CollectiveArray[i].colIndex == colIndex) {
			CPed* pPed = CPools::GetPedPool()->GetAt(CollectiveArray[i].pedIndex);
			if (pPed == nil) {
				CollectiveArray[i].colIndex = -1;
				CollectiveArray[i].pedIndex = 0;
			}
			else {
				pPed->bScriptObjectiveCompleted = false;
				pPed->SetObjective(objective, p1, p2);
			}
		}
	}
}

void CTheScripts::SetObjectiveForAllPedsInCollective(int colIndex, eObjective objective, CVector p1)
{
	for (int i = 0; i < MAX_NUM_COLLECTIVES; i++) {
		if (CollectiveArray[i].colIndex == colIndex) {
			CPed* pPed = CPools::GetPedPool()->GetAt(CollectiveArray[i].pedIndex);
			if (pPed == nil) {
				CollectiveArray[i].colIndex = -1;
				CollectiveArray[i].pedIndex = 0;
			}
			else {
				pPed->bScriptObjectiveCompleted = false;
				pPed->SetObjective(objective, p1);
			}
		}
	}
}

void CTheScripts::SetObjectiveForAllPedsInCollective(int colIndex, eObjective objective, void* p1)
{
	for (int i = 0; i < MAX_NUM_COLLECTIVES; i++) {
		if (CollectiveArray[i].colIndex == colIndex) {
			CPed* pPed = CPools::GetPedPool()->GetAt(CollectiveArray[i].pedIndex);
			if (pPed == nil) {
				CollectiveArray[i].colIndex = -1;
				CollectiveArray[i].pedIndex = 0;
			}
			else {
				pPed->bScriptObjectiveCompleted = false;
				pPed->SetObjective(objective, p1);
			}
		}
	}
}

void CTheScripts::SetObjectiveForAllPedsInCollective(int colIndex, eObjective objective)
{
	for (int i = 0; i < MAX_NUM_COLLECTIVES; i++) {
		if (CollectiveArray[i].colIndex == colIndex) {
			CPed* pPed = CPools::GetPedPool()->GetAt(CollectiveArray[i].pedIndex);
			if (pPed == nil) {
				CollectiveArray[i].colIndex = -1;
				CollectiveArray[i].pedIndex = 0;
			}
			else {
				pPed->bScriptObjectiveCompleted = false;
				pPed->SetObjective(objective);
			}
		}
	}
}
#endif //GTA_SCRIPT_COLLECTIVE

bool CTheScripts::IsPedStopped(CPed* pPed)
{
	if (pPed->bInVehicle)
		return IsVehicleStopped(pPed->m_pMyVehicle);
	return pPed->m_nMoveState == PEDMOVE_NONE || pPed->m_nMoveState == PEDMOVE_STILL;
}

bool CTheScripts::IsPlayerStopped(CPlayerInfo* pPlayer)
{
	CPed* pPed = pPlayer->m_pPed;
	if (pPed->bInVehicle)
		return IsVehicleStopped(pPed->m_pMyVehicle);
	if (RpAnimBlendClumpGetAssociation(pPed->GetClump(), ANIM_STD_RUNSTOP1) ||
		RpAnimBlendClumpGetAssociation(pPed->GetClump(), ANIM_STD_RUNSTOP2) ||
		RpAnimBlendClumpGetAssociation(pPed->GetClump(), ANIM_STD_JUMP_LAUNCH) ||
		RpAnimBlendClumpGetAssociation(pPed->GetClump(), ANIM_STD_JUMP_GLIDE))
		return false;
	return pPed->m_nMoveState == PEDMOVE_NONE || pPed->m_nMoveState == PEDMOVE_STILL;
}

bool CTheScripts::IsVehicleStopped(CVehicle* pVehicle)
{
	return 0.01f * CTimer::GetTimeStep() >= pVehicle->m_fDistanceTravelled;
}

void CTheScripts::CleanUpThisPed(CPed* pPed)
{
	if (!pPed)
		return;
	if (pPed->CharCreatedBy != MISSION_CHAR)
		return;
	pPed->CharCreatedBy = RANDOM_CHAR;
	if (pPed->m_nPedType == PEDTYPE_PROSTITUTE)
		pPed->m_objectiveTimer = CTimer::GetTimeInMilliseconds() + 30000;
	if (pPed->bInVehicle) {
		if (pPed->m_pMyVehicle->pDriver == pPed) {
			if (pPed->m_pMyVehicle->m_vehType == VEHICLE_TYPE_CAR) {
				CCarCtrl::JoinCarWithRoadSystem(pPed->m_pMyVehicle);
				pPed->m_pMyVehicle->AutoPilot.m_nCarMission = MISSION_CRUISE;
			}
		}
		else {
			if (pPed->m_pMyVehicle->m_vehType == VEHICLE_TYPE_CAR) {
				pPed->SetObjective(OBJECTIVE_LEAVE_CAR, pPed->m_pMyVehicle);
				pPed->bWanderPathAfterExitingCar = true;
			}
		}
	}
	bool flees = false;
	PedState state;
	eMoveState ms;
	if (pPed->m_nPedState == PED_FLEE_ENTITY || pPed->m_nPedState == PED_FLEE_POS) {
		ms = pPed->m_nMoveState;
		state = pPed->m_nPedState;
		flees = true;
	}
	pPed->ClearObjective();
	pPed->bRespondsToThreats = true;
	pPed->bScriptObjectiveCompleted = false;
	pPed->ClearLeader();
	if (pPed->IsPedInControl())
		pPed->SetWanderPath(CGeneral::GetRandomNumber() & 7);
	if (flees) {
		pPed->SetPedState(state);
		pPed->SetMoveState(ms);
	}
	--CPopulation::ms_nTotalMissionPeds;
}

void CTheScripts::CleanUpThisVehicle(CVehicle* pVehicle)
{
	if (!pVehicle)
		return;
	if (pVehicle->VehicleCreatedBy != MISSION_VEHICLE)
		return;
	pVehicle->bIsLocked = false;
	CCarCtrl::RemoveFromInterestingVehicleList(pVehicle);
	pVehicle->VehicleCreatedBy = RANDOM_VEHICLE;
	++CCarCtrl::NumRandomCars;
	--CCarCtrl::NumMissionCars;
}

void CTheScripts::CleanUpThisObject(CObject* pObject)
{
	if (!pObject)
		return;
	if (pObject->ObjectCreatedBy != MISSION_OBJECT)
		return;
	pObject->ObjectCreatedBy = TEMP_OBJECT;
	pObject->m_nEndOfLifeTime = CTimer::GetTimeInMilliseconds() + 20000;
	pObject->m_nRefModelIndex = -1;
	pObject->bUseVehicleColours = false;
	++CObject::nNoTempObjects;
}

void CTheScripts::ReadObjectNamesFromScript()
{
	int32 varSpace = GetSizeOfVariableSpace();
	uint32 ip = varSpace + 8;
	NumberOfUsedObjects = Read2BytesFromScript(&ip);
	ip += 2;
	for (uint16 i = 0; i < NumberOfUsedObjects; i++) {
		for (int j = 0; j < USED_OBJECT_NAME_LENGTH; j++)
			UsedObjectArray[i].name[j] = ScriptSpace[ip++];
		UsedObjectArray[i].index = 0;
	}
}

void CTheScripts::UpdateObjectIndices()
{
	char name[USED_OBJECT_NAME_LENGTH];
	char error[112];
	for (int i = 1; i < NumberOfUsedObjects; i++) {
		bool found = false;
		for (int j = 0; j < MODELINFOSIZE && !found; j++) {
			CBaseModelInfo* pModel = CModelInfo::GetModelInfo(j);
			if (!pModel)
				continue;
			strcpy(name, pModel->GetModelName());
#ifdef FIX_BUGS
			for (int k = 0; k < USED_OBJECT_NAME_LENGTH && name[k]; k++)
#else
			for (int k = 0; k < USED_OBJECT_NAME_LENGTH; k++)
#endif
				name[k] = toupper(name[k]);
			if (strcmp(name, UsedObjectArray[i].name) == 0) {
				found = true;
				UsedObjectArray[i].index = j;
			}
		}
		if (!found) {
			sprintf(error, "CTheScripts::UpdateObjectIndices - Couldn't find %s", UsedObjectArray[i].name);
			debug("%s\n", error);
		}
	}
}

void CTheScripts::ReadMultiScriptFileOffsetsFromScript()
{
	int32 varSpace = GetSizeOfVariableSpace();
	uint32 ip = varSpace + 3;
	int32 objectSize = Read4BytesFromScript(&ip);
	ip = objectSize + 8;
	MainScriptSize = Read4BytesFromScript(&ip);
	LargestMissionScriptSize = Read4BytesFromScript(&ip);
	NumberOfMissionScripts = Read2BytesFromScript(&ip);
	ip += 2;
	for (int i = 0; i < NumberOfMissionScripts; i++) {
		MultiScriptArray[i] = Read4BytesFromScript(&ip);
	}
}
