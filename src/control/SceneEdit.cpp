#include "common.h"

#include "SceneEdit.h"
#ifdef GTA_SCENE_EDIT
#include "Automobile.h"
#include "Camera.h"
#include "CarCtrl.h"
#include "CivilianPed.h"
#include "FileMgr.h"
#include "Font.h"
#include "ModelIndices.h"
#include "ModelInfo.h"
#include "Pad.h"
#include "Ped.h"
#include "Population.h"
#include "Text.h"
#include "Timecycle.h"
#include "Streaming.h"
#include "Vehicle.h"
#include "WeaponInfo.h"
#include "World.h"

bool CSceneEdit::m_bEditOn;
int32 CSceneEdit::m_bCameraFollowActor;
bool CSceneEdit::m_bRecording;
CVector CSceneEdit::m_vecCurrentPosition;
CVector CSceneEdit::m_vecCamHeading;
CVector CSceneEdit::m_vecGotoPosition;
int32 CSceneEdit::m_nVehicle;
int32 CSceneEdit::m_nVehicle2;
int32 CSceneEdit::m_nActor;
int32 CSceneEdit::m_nActor2;
int32 CSceneEdit::m_nVehiclemodelId;
int32 CSceneEdit::m_nPedmodelId;
int16 CSceneEdit::m_nCurrentMovieCommand;
int16 CSceneEdit::m_nNumActors;
int16 CSceneEdit::m_nNumMovieCommands;
int16 CSceneEdit::m_nCurrentCommand;
int16 CSceneEdit::m_nCurrentVehicle;
int16 CSceneEdit::m_nCurrentActor;
int16 CSceneEdit::m_nWeaponType;
bool CSceneEdit::m_bCommandActive;
bool CSceneEdit::m_bActorSelected;
bool CSceneEdit::m_bActor2Selected;
bool CSceneEdit::m_bVehicleSelected;
int16 CSceneEdit::m_nNumVehicles;
CPed* CSceneEdit::pActors[NUM_ACTORS_IN_MOVIE];
CVehicle* CSceneEdit::pVehicles[NUM_VEHICLES_IN_MOVIE];
bool CSceneEdit::m_bDrawGotoArrow;
CMovieCommand CSceneEdit::Movie[NUM_COMMANDS_IN_MOVIE];

#define SHADOW_OFFSET (2.0f)
#define ACTION_MESSAGE_X_RIGHT (60.0f)
#define ACTION_MESSAGE_Y (8.0f)
#define SELECTED_MESSAGE_X_RIGHT (60.0f)
#define SELECTED_MESSAGE_Y (248.0f)
#define COMMAND_NAME_X_RIGHT (60.0f)
#define COMMAND_NAME_Y (38.0f)
#define COMMAND_NAME_HEIGHT (16.0f)

#define NUM_COMMANDS_TO_DRAW (9)

static const char* pCommandStrings[] = {
	"do-nothing", "New Actor", "Move Actor", "Select Actor", "Delete Actor",
	"New Vehicle", "Move Vehicle", "Select Vehicle", "Delete Vehicle", "Give Weapon",
	"Goto", "Goto (wait)", "Get In Car", "Get Out Car", "Kill",
	"Flee", "Wait", "Position Camera", "Set Camera Target", "Select Camera Mode",
	"Save Movie", "Load Movie", "Play Movie", "END"
};

#ifdef CHECK_STRUCT_SIZES
static_assert(ARRAY_SIZE(pCommandStrings) == CSceneEdit::MOVIE_TOTAL_COMMANDS, "Scene edit: not all commands have names");
#endif

static int32 NextValidModelId(int32 mi, int32 step)
{
	int32 result = -1;
	int32 i = mi;
	while (result == -1) {
		i += step;
		if (i < 0 || i > MODELINFOSIZE) {
			step = -step;
			continue;
		}
		CBaseModelInfo* pInfo = CModelInfo::GetModelInfo(i);
		CVehicleModelInfo* pVehicleInfo = (CVehicleModelInfo*)pInfo;
		if (!pInfo)
			continue;
		if (pInfo->GetModelType() == MITYPE_PED
#ifdef FIX_BUGS
			&& !(i >= MI_SPECIAL01 && i <= MI_SPECIAL04)
#endif
			|| pInfo->GetModelType() == MITYPE_VEHICLE &&
#ifdef FIX_BUGS
			(pVehicleInfo->m_vehicleType == VEHICLE_TYPE_CAR || pVehicleInfo->m_vehicleType == VEHICLE_TYPE_BOAT))
#else // && and || priority failure it seems, also crashes on special models
			pVehicleInfo->m_vehicleType == VEHICLE_TYPE_CAR || pVehicleInfo->m_vehicleType == VEHICLE_TYPE_BOAT)
#endif
			result = i;
	}
	return result;
}

void CSceneEdit::LoadMovie(void)
{
	ReInitialise();
	CFileMgr::SetDir("DATA");
	int fid = CFileMgr::OpenFile("movie.dat", "r");
#ifdef FIX_BUGS
	if (fid >= 0)
#endif
	{
		CFileMgr::Read(fid, (char*)&Movie, sizeof(Movie));
		CFileMgr::Read(fid, (char*)&m_nNumMovieCommands, sizeof(m_nNumMovieCommands));
		CFileMgr::CloseFile(fid);
	}
	CFileMgr::SetDir("");
	m_bCommandActive = false;
}

void CSceneEdit::SaveMovie(void)
{
	CFileMgr::SetDir("DATA");
	int fid = CFileMgr::OpenFileForWriting("movie.dat");
	if (fid >= 0) {
		CFileMgr::Write(fid, (char*)&Movie, sizeof(Movie));
		CFileMgr::Write(fid, (char*)&m_nNumMovieCommands, sizeof(m_nNumMovieCommands));
		CFileMgr::CloseFile(fid);
	}
	CFileMgr::SetDir("");
	m_bCommandActive = false;
}

void CSceneEdit::Initialise(void)
{
	m_nActor = -1;
	m_nActor2 = -1;
	m_nVehicle = -1;
	m_nVehicle2 = -1;
	m_nCurrentCommand = MOVIE_NEW_ACTOR;
	m_nVehiclemodelId = MI_INFERNUS;
	m_nPedmodelId = MI_MALE01;
	m_nNumVehicles = 0;
	m_nNumActors = 0;
	m_nNumMovieCommands = 0;
	m_bCommandActive = false;
	m_bRecording = true;
	m_bEditOn = false;
	for (int i = 0; i < NUM_ACTORS_IN_MOVIE; i++)
		pActors[i] = nil;
	for (int i = 0; i < NUM_VEHICLES_IN_MOVIE; i++)
		pVehicles[i] = nil;
	m_vecCamHeading = TheCamera.Cams[TheCamera.ActiveCam].Front;
	m_vecGotoPosition = CVector(0.0f, 0.0f, 0.0f);
	m_bCameraFollowActor = false;
	TheCamera.Cams[TheCamera.ActiveCam].ResetStatics = true;
	m_bDrawGotoArrow = false;
}

void CSceneEdit::InitPlayback(void)
{
	m_nVehiclemodelId = MI_INFERNUS;
	m_nPedmodelId = MI_MALE01;
	m_bCommandActive = false;
	m_nNumActors = 0;
	m_nNumVehicles = 0;
	m_nActor = -1;
	m_nActor2 = -1;
	m_nVehicle = -1;
	m_nVehicle2 = -1;
	TheCamera.Cams[TheCamera.ActiveCam].ResetStatics = true;
	m_vecCamHeading = TheCamera.Cams[TheCamera.ActiveCam].Front;
	for (int i = 0; i < NUM_ACTORS_IN_MOVIE; i++) {
		if (pActors[i]) {
			CPopulation::RemovePed(pActors[i]);
			pActors[i] = nil;
		}
	}
	m_nCurrentActor = 0;
	for (int i = 0; i < NUM_VEHICLES_IN_MOVIE; i++) {
		if (pVehicles[i]) {
			CWorld::Remove(pVehicles[i]);
			delete pVehicles[i];
			pVehicles[i] = nil;
		}
	}
	m_nCurrentVehicle = 0;
	m_vecGotoPosition = CVector(0.0f, 0.0f, 0.0f);
	m_nCurrentMovieCommand = MOVIE_DO_NOTHING;
	m_bDrawGotoArrow = false;
}

void CSceneEdit::ReInitialise(void)
{
	m_nVehiclemodelId = MI_INFERNUS;
	m_nPedmodelId = MI_MALE01;
	m_nCurrentCommand = MOVIE_NEW_ACTOR;
	m_bEditOn = true;
	m_bRecording = true;
	m_bCommandActive = false;
#ifdef FIX_BUGS
	m_bCameraFollowActor = false;
	TheCamera.Cams[TheCamera.ActiveCam].ResetStatics = true; // not enough...
#endif
	m_nActor = -1;
	m_nActor2 = -1;
	m_nVehicle = -1;
	m_nVehicle2 = -1;
	m_nNumMovieCommands = 0;
	m_nCurrentMovieCommand = MOVIE_DO_NOTHING;
	m_nNumActors = 0;
	m_nNumVehicles = 0;
	for (int i = 0; i < NUM_ACTORS_IN_MOVIE; i++) {
		if (pActors[i]) {
			CPopulation::RemovePed(pActors[i]);
			pActors[i] = nil;
		}
	}
	for (int i = 0; i < NUM_VEHICLES_IN_MOVIE; i++) {
		if (pVehicles[i]) {
			CWorld::Remove(pVehicles[i]);
			delete pVehicles[i];
			pVehicles[i] = nil;
		}
	}
	for (int i = 0; i < NUM_COMMANDS_IN_MOVIE; i++) {
		Movie[i].m_nCommandId = MOVIE_DO_NOTHING;
		Movie[i].m_vecPosition = CVector(0.0f, 0.0f, 0.0f);
		Movie[i].m_vecCamera = CVector(0.0f, 0.0f, 0.0f);
		Movie[i].m_nActorId = -1;
		Movie[i].m_nActor2Id = -1;
		Movie[i].m_nVehicleId = -1;
		Movie[i].m_nModelIndex = 0;
	}
	m_vecGotoPosition = CVector(0.0f, 0.0f, 0.0f);
	m_bDrawGotoArrow = false;
}

void CSceneEdit::Update(void)
{
	if (!m_bEditOn)
		return;
	if (m_bRecording)
		ProcessCommand();
	else {
		if (m_bCameraFollowActor && m_nActor != -1) {
			if (pActors[m_nActor]->bInVehicle)
				TheCamera.TakeControl(pActors[m_nActor]->m_pMyVehicle, CCam::MODE_BEHINDCAR, JUMP_CUT, CAMCONTROL_SCRIPT);
			else
				TheCamera.TakeControl(pActors[m_nActor], CCam::MODE_FOLLOWPED, JUMP_CUT, CAMCONTROL_SCRIPT);
		}
		PlayBack();
	}
}

void CSceneEdit::Draw(void)
{
	char str[200];
	wchar wstr[200];
	if (TheCamera.m_WideScreenOn)
		return;
#ifndef FIX_BUGS
	CFont::SetPropOff();
#endif
	CFont::SetBackgroundOff();
	CFont::SetScale(SCREEN_SCALE_X(0.8f), SCREEN_SCALE_Y(1.35f));
	CFont::SetCentreOn();
	CFont::SetRightJustifyOn();
	CFont::SetRightJustifyWrap(0.0f);
	CFont::SetBackGroundOnlyTextOff();
#ifdef FIX_BUGS
	CFont::SetFontStyle(FONT_BANK);
	CFont::SetPropOn();
	CFont::SetDropColor(CRGBA(0, 0, 0, 255));
	CFont::SetDropShadowPosition(1);
#else
	CFont::SetFontStyle(FONT_HEADING);
	CFont::SetPropOff();
#endif
	sprintf(str, "Action");
	AsciiToUnicode(str, wstr);
	CFont::SetColor(CRGBA(0, 0, 0, 255));
#ifdef FIX_BUGS
	CFont::PrintString(SCREEN_SCALE_FROM_RIGHT(ACTION_MESSAGE_X_RIGHT - SHADOW_OFFSET), SCREEN_SCALE_Y(ACTION_MESSAGE_Y + SHADOW_OFFSET), wstr);
#else
	CFont::PrintString(SCREEN_SCALE_X(DEFAULT_SCREEN_WIDTH-ACTION_MESSAGE_X_RIGHT) + SHADOW_OFFSET, SCREEN_SCALE_FROM_BOTTOM(DEFAULT_SCREEN_HEIGHT-ACTION_MESSAGE_Y) + SHADOW_OFFSET, wstr);
#endif
	CFont::SetColor(CRGBA(193, 164, 120, 255));
#ifdef FIX_BUGS
	CFont::PrintString(SCREEN_SCALE_FROM_RIGHT(ACTION_MESSAGE_X_RIGHT), SCREEN_SCALE_Y(ACTION_MESSAGE_Y), wstr);
#else
	CFont::PrintString(SCREEN_SCALE_X(DEFAULT_SCREEN_WIDTH-ACTION_MESSAGE_X_RIGHT), SCREEN_SCALE_FROM_BOTTOM(DEFAULT_SCREEN_HEIGHT-ACTION_MESSAGE_Y), wstr);
#endif
	sprintf(str, "Selected");
	AsciiToUnicode(str, wstr);
	CFont::SetColor(CRGBA(0, 0, 0, 255));
#ifdef FIX_BUGS
	CFont::PrintString(SCREEN_SCALE_FROM_RIGHT(SELECTED_MESSAGE_X_RIGHT - SHADOW_OFFSET), SCREEN_SCALE_Y(SELECTED_MESSAGE_Y + SHADOW_OFFSET), wstr);
#else
	CFont::PrintString(SCREEN_SCALE_X(DEFAULT_SCREEN_WIDTH-SELECTED_MESSAGE_X_RIGHT) + SHADOW_OFFSET, SCREEN_SCALE_FROM_BOTTOM(DEFAULT_SCREEN_HEIGHT-SELECTED_MESSAGE_Y) + SHADOW_OFFSET, wstr);
#endif
	CFont::SetColor(CRGBA(193, 164, 120, 255));
#ifdef FIX_BUGS
	CFont::PrintString(SCREEN_SCALE_FROM_RIGHT(SELECTED_MESSAGE_X_RIGHT), SCREEN_SCALE_Y(SELECTED_MESSAGE_Y), wstr);
#else
	CFont::PrintString(SCREEN_SCALE_X(DEFAULT_SCREEN_WIDTH-SELECTED_MESSAGE_X_RIGHT), SCREEN_SCALE_FROM_BOTTOM(DEFAULT_SCREEN_HEIGHT-SELECTED_MESSAGE_Y), wstr);
#endif
	CFont::SetCentreOff();
#ifdef FIX_BUGS
	CFont::SetScale(SCREEN_SCALE_X(0.7f), SCREEN_SCALE_Y(0.7f));
#else
	CFont::SetScale(0.7f, 0.7f);
#endif
#ifdef FIX_BUGS
	CFont::SetFontStyle(FONT_BANK);
#else
	CFont::SetFontStyle(FONT_HEADING);
#endif
	CFont::SetColor(CRGBA(0, 0, 0, 255));
	for (int i = 0; i < NUM_COMMANDS_TO_DRAW; i++) {
		int16 nCommandDrawn = m_nCurrentCommand + i - NUM_COMMANDS_TO_DRAW / 2;
		if (nCommandDrawn >= MOVIE_TOTAL_COMMANDS)
			nCommandDrawn -= (MOVIE_TOTAL_COMMANDS - 1);
		if (nCommandDrawn <= MOVIE_DO_NOTHING)
			nCommandDrawn += (MOVIE_TOTAL_COMMANDS - 1);
		sprintf(str, "%s", pCommandStrings[nCommandDrawn]);
		AsciiToUnicode(str, wstr);
		CFont::SetColor(CRGBA(0, 0, 0, 255));
#ifdef FIX_BUGS
		CFont::PrintString(SCREEN_SCALE_FROM_RIGHT(COMMAND_NAME_X_RIGHT - SHADOW_OFFSET), SCREEN_SCALE_Y(COMMAND_NAME_Y + SHADOW_OFFSET + i * COMMAND_NAME_HEIGHT), wstr);
#else
		CFont::PrintString(SCREEN_SCALE_X(DEFAULT_SCREEN_WIDTH-COMMAND_NAME_X_RIGHT) + SHADOW_OFFSET, SCREEN_SCALE_FROM_BOTTOM(DEFAULT_SCREEN_HEIGHT-COMMAND_NAME_Y) + SHADOW_OFFSET + i * COMMAND_NAME_HEIGHT, wstr);
#endif
		if (nCommandDrawn == m_nCurrentCommand)
			CFont::SetColor(CRGBA(156, 91, 40, 255));
		else
			CFont::SetColor(CRGBA(193, 164, 120, 255));
#ifdef FIX_BUGS
		CFont::PrintString(SCREEN_SCALE_FROM_RIGHT(COMMAND_NAME_X_RIGHT), SCREEN_SCALE_Y(COMMAND_NAME_Y + i * COMMAND_NAME_HEIGHT), wstr);
#else
		CFont::PrintString(SCREEN_SCALE_X(DEFAULT_SCREEN_WIDTH-COMMAND_NAME_X_RIGHT), SCREEN_SCALE_FROM_BOTTOM(DEFAULT_SCREEN_HEIGHT-COMMAND_NAME_Y) + i * COMMAND_NAME_HEIGHT, wstr);
#endif
	}
}

void CSceneEdit::ProcessCommand(void)
{
	if (!m_bCommandActive) {
		ClearForNewCommand();
		if (CPad::GetPad(1)->GetDPadUpJustDown()) {
			if (--m_nCurrentCommand == MOVIE_DO_NOTHING)
				m_nCurrentCommand = MOVIE_END;
		}
		if (CPad::GetPad(1)->GetDPadDownJustDown()) {
			if (++m_nCurrentCommand == MOVIE_TOTAL_COMMANDS)
				m_nCurrentCommand = MOVIE_NEW_ACTOR;
		}
		if (CPad::GetPad(1)->GetTriangleJustDown()) {
			if (m_nCurrentCommand != MOVIE_DO_NOTHING)
				m_bCommandActive = true;
		}
		return;
	}
	switch (m_nCurrentCommand) {
	case MOVIE_DO_NOTHING:
		m_bCommandActive = false;
		break;
	case MOVIE_NEW_ACTOR:
		if (m_nActor == -1) {
			if (m_nNumActors == NUM_ACTORS_IN_MOVIE)
				break;
			if (!CStreaming::HasModelLoaded(m_nPedmodelId)) {
				CStreaming::RequestModel(m_nPedmodelId, 0);
#ifdef FIX_BUGS
				CStreaming::LoadAllRequestedModels(false); // otherwise gets stuck :(
#endif
				break;
			}
			CPed* pPed = new CCivilianPed(PEDTYPE_SPECIAL, m_nPedmodelId);
			pPed->CharCreatedBy = MISSION_CHAR;
			pPed->SetPosition(m_vecCurrentPosition);
			pPed->SetOrientation(0.0f, 0.0f, 0.0f);
			CWorld::Add(pPed);
			pPed->bUsesCollision = false;
			pPed->bAffectedByGravity = false;
			for (int i = 0; i < NUM_ACTORS_IN_MOVIE; i++) {
				if (pActors[i] == nil) {
					m_nActor = i;
					pActors[i] = pPed;
					break;
				}
			}
		}
		else {
			pActors[m_nActor]->SetPosition(m_vecCurrentPosition);
			pActors[m_nActor]->SetOrientation(0.0f, 0.0f, 0.0f);
			int32 mi = m_nPedmodelId;
			if (CPad::GetPad(1)->GetLeftShoulder1JustDown())
				mi = NextValidModelId(m_nPedmodelId, -1);
			else if (CPad::GetPad(1)->GetRightShoulder1JustDown())
				mi = NextValidModelId(m_nPedmodelId, 1);
			if (mi == m_nPedmodelId) {
				if (CPad::GetPad(1)->GetTriangleJustDown()) {
					pActors[m_nActor]->bUsesCollision = true;
					pActors[m_nActor]->bAffectedByGravity = true;
					++m_nNumActors;
					Movie[m_nNumMovieCommands].m_nCommandId = MOVIE_NEW_ACTOR;
					Movie[m_nNumMovieCommands].m_vecPosition = m_vecCurrentPosition;
					Movie[m_nNumMovieCommands].m_nModelIndex = m_nPedmodelId;
					Movie[m_nNumMovieCommands++].m_nActorId = m_nActor;
					m_nActor = -1;
					m_bCommandActive = false;
				}
				if (CPad::GetPad(1)->GetCircleJustDown()) {
					CWorld::Remove(pActors[m_nActor]);
					delete pActors[m_nActor];
					pActors[m_nActor] = nil;
					m_nActor = -1;
					m_bCommandActive = false;
				}
			}
			else {
				m_nPedmodelId = mi;
				if (pActors[m_nActor]) {
					CWorld::Remove(pActors[m_nActor]);
					delete pActors[m_nActor];
				}
				pActors[m_nActor] = nil;
				m_nActor = -1;
			}
		}
		break;
	case MOVIE_MOVE_ACTOR:
		SelectActor();
		if (m_bCommandActive)
			break;
		pActors[m_nActor]->SetPosition(m_vecCurrentPosition);
		if (CPad::GetPad(1)->GetTriangleJustDown()) {
			m_bCommandActive = false;
#ifndef FIX_BUGS // why? it crashes, also makes no sense
			pActors[m_nActor] = nil;
#endif
			SelectActor();
		}
		break;
	case MOVIE_SELECT_ACTOR:
		SelectActor();
		break;
	case MOVIE_DELETE_ACTOR:
		SelectActor();
		if (m_bActorSelected) {
			CPopulation::RemovePed(pActors[m_nActor]);
			m_nCurrentActor = 0;
			--m_nNumActors;
#ifdef FIX_BUGS
			pActors[m_nActor] = nil;
			m_nActor = -1;
#else
			m_nActor = -1;
			pActors[m_nActor] = nil;
#endif
			SelectActor();
			m_bCommandActive = false;
		}
		else if (CPad::GetPad(1)->GetCircleJustDown()) {
			m_nActor = -1;
			m_bCommandActive = false;
		}
		break;
	case MOVIE_NEW_VEHICLE:
		if (m_nVehicle == -1) {
			if (m_nNumVehicles == NUM_VEHICLES_IN_MOVIE)
				break;
			if (!CStreaming::HasModelLoaded(m_nVehiclemodelId)) {
				CStreaming::RequestModel(m_nVehiclemodelId, 0);
#ifdef FIX_BUGS
				CStreaming::LoadAllRequestedModels(false); // otherwise gets stuck :(
#endif
				break;
			}
			CVehicle* pVehicle = new CAutomobile(m_nVehiclemodelId, MISSION_VEHICLE);
			pVehicle->SetStatus(STATUS_PHYSICS);
			pVehicle->SetPosition(m_vecCurrentPosition);
			pVehicle->SetOrientation(0.0f, 0.0f, 0.0f);
			CWorld::Add(pVehicle);
			pVehicle->bUsesCollision = false;
			pVehicle->bAffectedByGravity = false;
			for (int i = 0; i < NUM_VEHICLES_IN_MOVIE; i++) {
				if (pVehicles[i] == nil) {
					m_nVehicle = i;
					pVehicles[i] = pVehicle;
					break;
				}
			}
		}
		else {
			pVehicles[m_nVehicle]->SetPosition(m_vecCurrentPosition);
			pVehicles[m_nVehicle]->SetOrientation(0.0f, 0.0f, 0.0f);
			int32 mi = m_nVehiclemodelId;
			if (CPad::GetPad(1)->GetLeftShoulder1JustDown())
				mi = NextValidModelId(m_nVehiclemodelId, -1);
			else if (CPad::GetPad(1)->GetRightShoulder1JustDown())
				mi = NextValidModelId(m_nVehiclemodelId, 1);
			if (mi == m_nVehiclemodelId) {
				if (CPad::GetPad(1)->GetTriangleJustDown()) {
					pVehicles[m_nVehicle]->bUsesCollision = true;
					pVehicles[m_nVehicle]->bAffectedByGravity = true;
					++m_nNumVehicles;
					Movie[m_nNumMovieCommands].m_nCommandId = MOVIE_NEW_VEHICLE;
					Movie[m_nNumMovieCommands].m_vecPosition = m_vecCurrentPosition;
					Movie[m_nNumMovieCommands].m_nModelIndex = m_nVehiclemodelId;
					Movie[m_nNumMovieCommands++].m_nVehicleId = m_nVehicle;
					m_nVehicle = -1;
					m_bCommandActive = false;
				}
				if (CPad::GetPad(1)->GetCircleJustDown()) {
					CWorld::Remove(pVehicles[m_nVehicle]);
					delete pVehicles[m_nVehicle];
					pVehicles[m_nVehicle] = nil;
					m_nVehicle = -1;
					m_bCommandActive = false;
				}
			}
			else {
				m_nVehiclemodelId = mi;
				if (pVehicles[m_nVehicle]) {
					CWorld::Remove(pVehicles[m_nVehicle]);
					delete pVehicles[m_nVehicle];
				}
				pVehicles[m_nVehicle] = nil;
				m_nVehicle = -1;
			}
		}
		break;
	case MOVIE_MOVE_VEHICLE:
		SelectVehicle();
		if (m_bCommandActive)
			break;
		pVehicles[m_nVehicle]->SetPosition(m_vecCurrentPosition);
		if (CPad::GetPad(1)->GetTriangleJustDown()) {
			m_bCommandActive = false;
#ifndef FIX_BUGS // again, why? works wrong
			pVehicles[m_nVehicle] = nil;
#endif
			m_nVehicle = -1;
		}
		break;
	case MOVIE_SELECT_VEHICLE:
		SelectVehicle();
		break;
	case MOVIE_DELETE_VEHICLE:
		SelectVehicle();
		if (m_bVehicleSelected) {
			CWorld::Remove(pVehicles[m_nVehicle]);
			delete pVehicles[m_nVehicle];
			m_nCurrentVehicle = 0;
			--m_nNumVehicles;
			pVehicles[m_nVehicle] = nil;
			m_nVehicle = -1;
			SelectVehicle();
			m_bCommandActive = false;
		}
		else if (CPad::GetPad(1)->GetCircleJustDown()) {
			pVehicles[m_nVehicle] = nil;
			m_nVehicle = -1;
			m_bCommandActive = false;
		}
		break;
	case MOVIE_GIVE_WEAPON:
		if (m_bActorSelected) {
			if (SelectWeapon()) {
				m_bCommandActive = false;
				Movie[m_nNumMovieCommands].m_nCommandId = MOVIE_GIVE_WEAPON;
				Movie[m_nNumMovieCommands].m_nActorId = m_nActor;
				Movie[m_nNumMovieCommands++].m_nModelIndex = m_nWeaponType;
			}
		}
		else {
			SelectActor();
			m_bCommandActive = true;
		}
		break;
	case MOVIE_GOTO:
	case MOVIE_GOTO_WAIT:
		if (!m_bActorSelected) {
			m_bDrawGotoArrow = true;
			SelectActor();
			if (m_nActor == -1)
				m_bCommandActive = true;
		}
		else {
			m_vecGotoPosition = m_vecCurrentPosition;
			if (CPad::GetPad(1)->GetTriangleJustDown()) {
				if (pActors[m_nActor]->bInVehicle) {
					if (CCarCtrl::JoinCarWithRoadSystemGotoCoors(pActors[m_nActor]->m_pMyVehicle, m_vecGotoPosition, false))
						pActors[m_nActor]->m_pMyVehicle->AutoPilot.m_nCarMission = MISSION_GOTOCOORDS_STRAIGHT;
					else
						pActors[m_nActor]->m_pMyVehicle->AutoPilot.m_nCarMission = MISSION_GOTOCOORDS;
					pActors[m_nActor]->m_pMyVehicle->SetStatus(STATUS_PHYSICS);
					pActors[m_nActor]->m_pMyVehicle->bEngineOn = true;
					pActors[m_nActor]->m_pMyVehicle->AutoPilot.m_nCruiseSpeed = Max(16, pActors[m_nActor]->m_pMyVehicle->AutoPilot.m_nCruiseSpeed);
					pActors[m_nActor]->m_pMyVehicle->AutoPilot.m_nAntiReverseTimer = CTimer::GetTimeInMilliseconds();
					TheCamera.TakeControl(pActors[m_nActor]->m_pMyVehicle, CCam::MODE_BEHINDCAR, JUMP_CUT, CAMCONTROL_SCRIPT);
				}
				else {
					pActors[m_nActor]->SetObjective(OBJECTIVE_GOTO_AREA_ON_FOOT, m_vecGotoPosition);
					TheCamera.TakeControl(pActors[m_nActor], CCam::MODE_FOLLOWPED, JUMP_CUT, CAMCONTROL_SCRIPT);
				}
				m_bDrawGotoArrow = false;
				Movie[m_nNumMovieCommands].m_nCommandId = MOVIE_GOTO;
				Movie[m_nNumMovieCommands].m_nActorId = m_nActor;
				Movie[m_nNumMovieCommands++].m_vecPosition = m_vecGotoPosition;
			}
			if (!m_bDrawGotoArrow) {
				if (pActors[m_nActor]->bInVehicle && pActors[m_nActor]->m_pMyVehicle->AutoPilot.m_nCarMission == MISSION_NONE ||
					!pActors[m_nActor]->bInVehicle && pActors[m_nActor]->m_objective == OBJECTIVE_NONE) {
					if (pActors[m_nActor]) // if there is something that requires this check the least, it's this one
						m_vecCamHeading = TheCamera.Cams[TheCamera.ActiveCam].CamTargetEntity->GetPosition() - TheCamera.Cams[TheCamera.ActiveCam].Source;
					m_bCommandActive = false;
					TheCamera.Cams[TheCamera.ActiveCam].Mode = CCam::MODE_FIGHT_CAM_RUNABOUT;
					m_vecCurrentPosition = pActors[m_nActor]->GetPosition();
					pActors[m_nActor]->SetObjective(OBJECTIVE_NONE);
					if (pActors[m_nActor]->bInVehicle)
						pActors[m_nActor]->m_pMyVehicle->AutoPilot.m_nCarMission = MISSION_NONE;
				}
			}
			if (CPad::GetPad(1)->GetCircleJustDown()) {
				pActors[m_nActor] = nil;
				m_nActor = -1;
				m_bCommandActive = false;
			}
		}
		break;
	case MOVIE_GET_IN_CAR:
		if (m_bActorSelected)
			SelectVehicle();
		else {
			SelectActor();
			if (m_nActor != -1)
				m_bCommandActive = true;
		}
		if (m_bVehicleSelected) {
			pActors[m_nActor]->SetObjective(OBJECTIVE_ENTER_CAR_AS_DRIVER, pVehicles[m_nVehicle]);
			Movie[m_nNumMovieCommands].m_nCommandId = MOVIE_GET_IN_CAR;
			Movie[m_nNumMovieCommands].m_nActorId = m_nActor;
			Movie[m_nNumMovieCommands++].m_nVehicleId = m_nVehicle;
			m_nVehicle = -1;
			m_bCommandActive = false;
		}
		if (CPad::GetPad(1)->GetCircleJustDown()) {
			pVehicles[m_nVehicle] = nil;
			m_nVehicle = -1;
			pActors[m_nActor] = nil;
			m_nActor = -1;
			m_bCommandActive = false;
		}
		break;
	case MOVIE_GET_OUT_CAR:
		SelectActor();
		if (m_bActorSelected) {
			if (pActors[m_nActor]->bInVehicle) {
				pActors[m_nActor]->SetObjective(OBJECTIVE_LEAVE_CAR);
				Movie[m_nNumMovieCommands].m_nCommandId = MOVIE_GET_OUT_CAR;
				Movie[m_nNumMovieCommands++].m_nActorId = m_nActor;
			}
			m_nActor = -1;
			m_bCommandActive = false;
		}
		if (CPad::GetPad(1)->GetCircleJustDown()) {
			pVehicles[m_nVehicle] = nil;
			m_nVehicle = -1;
			pActors[m_nActor] = nil;
			m_nActor = -1;
			m_bCommandActive = false;
		}
		break;
	case MOVIE_KILL:
		if (!m_bActorSelected) {
			SelectActor();
			m_bCommandActive = true;
		}
		else if (!m_bActor2Selected) {
			SelectActor2();
			if (m_bActorSelected && m_bActor2Selected && m_nActor != -1 && m_nActor2 != -1 && m_nActor != m_nActor2) {
				pActors[m_nActor]->SetObjective(OBJECTIVE_KILL_CHAR_ON_FOOT, pActors[m_nActor2]);
				Movie[m_nNumMovieCommands].m_nCommandId = MOVIE_KILL;
				Movie[m_nNumMovieCommands].m_nActorId = m_nActor;
				Movie[m_nNumMovieCommands++].m_nActor2Id = m_nActor2;
				m_bCommandActive = false;
			}
		}
		if (CPad::GetPad(1)->GetCircleJustDown()) {
			pActors[m_nActor] = nil;
			m_nActor = -1;
			pActors[m_nActor2] = nil;
			m_nActor2 = -1;
			m_bCommandActive = false;
		}
		break;
	case MOVIE_FLEE:
		if (!m_bActorSelected) {
			SelectActor();
			m_bCommandActive = true;
		}
		else if (!m_bActor2Selected) {
			SelectActor2();
			if (m_bActorSelected && m_bActor2Selected && m_nActor != -1 && m_nActor2 != -1 && m_nActor != m_nActor2) {
				pActors[m_nActor]->SetObjective(OBJECTIVE_FLEE_CHAR_ON_FOOT_ALWAYS, pActors[m_nActor2]);
				Movie[m_nNumMovieCommands].m_nCommandId = MOVIE_FLEE;
				Movie[m_nNumMovieCommands].m_nActorId = m_nActor;
				Movie[m_nNumMovieCommands++].m_nActor2Id = m_nActor2;
				m_bCommandActive = false;
			}
		}
		if (CPad::GetPad(1)->GetCircleJustDown()) {
			pActors[m_nActor] = nil;
			m_nActor = -1;
			pActors[m_nActor2] = nil;
			m_nActor2 = -1;
			m_bCommandActive = false;
		}
		break;
	case MOVIE_WAIT:
		SelectActor();
		if (m_bActorSelected) {
			pActors[m_nActor]->SetObjective(OBJECTIVE_WAIT_ON_FOOT);
			Movie[m_nNumMovieCommands].m_nCommandId = MOVIE_WAIT;
			Movie[m_nNumMovieCommands++].m_nActorId = m_nActor;
		}
		if (CPad::GetPad(1)->GetCircleJustDown()) {
			pActors[m_nActor] = nil;
			m_nActor = -1;
			m_bCommandActive = false;
		}
		break;
	case MOVIE_POSITION_CAMERA:
		if (CPad::GetPad(1)->GetTriangleJustDown()) {
			Movie[m_nNumMovieCommands].m_nCommandId = MOVIE_POSITION_CAMERA;
			Movie[m_nNumMovieCommands].m_vecPosition = TheCamera.Cams[TheCamera.ActiveCam].Source;
			Movie[m_nNumMovieCommands++].m_vecCamera = m_vecCamHeading;
			m_bCommandActive = false;
		}
		if (CPad::GetPad(1)->GetCircleJustDown()) {
			m_bCommandActive = false;
		}
		break;
	case MOVIE_SET_CAMERA_TARGET:
		if (!m_bActorSelected) {
			SelectActor();
			m_bCommandActive = true;
		}
		else {
			TheCamera.Cams[TheCamera.ActiveCam].CamTargetEntity = pActors[m_nActor];
			if (CPad::GetPad(1)->GetTriangleJustDown()) {
				Movie[m_nNumMovieCommands].m_nCommandId = MOVIE_SET_CAMERA_TARGET;
				Movie[m_nNumMovieCommands++].m_nActorId = m_nActor;
				m_bCommandActive = false;
			}
		}
		break;
	case MOVIE_SELECT_CAMERA_MODE:
		m_bCommandActive = false;
		break;
	case MOVIE_SAVE_MOVIE:
		SaveMovie();
		break;
	case MOVIE_LOAD_MOVIE:
		LoadMovie();
		break;
	case MOVIE_PLAY_MOVIE:
		InitPlayback();
		LoadMovie();
		m_bRecording = false;
		break;
	case MOVIE_END:
		m_bRecording = false;
		break;
	default:
		assert(0);
	}
}

void CSceneEdit::PlayBack(void)
{
	m_nCurrentCommand = Movie[m_nCurrentMovieCommand].m_nCommandId;
	if (m_nCurrentMovieCommand >= m_nNumMovieCommands) {
		if (CPad::GetPad(1)->GetTriangleJustDown()) {
			m_nCurrentCommand = MOVIE_DO_NOTHING;
			m_bRecording = true;
			ReInitialise();
		}
		return;
	}
	switch (m_nCurrentCommand) {
	case MOVIE_DO_NOTHING:
	case MOVIE_MOVE_ACTOR:
	case MOVIE_SELECT_ACTOR:
	case MOVIE_DELETE_ACTOR:
	case MOVIE_MOVE_VEHICLE:
	case MOVIE_SELECT_VEHICLE:
	case MOVIE_DELETE_VEHICLE:
		break;
	case MOVIE_NEW_ACTOR:
	{
		m_nPedmodelId = Movie[m_nCurrentMovieCommand].m_nModelIndex;
		m_vecCurrentPosition = Movie[m_nCurrentMovieCommand].m_vecPosition;
		if (!CStreaming::HasModelLoaded(m_nPedmodelId)) {
			CStreaming::RequestModel(m_nPedmodelId, 0);
#ifdef FIX_BUGS
			CStreaming::LoadAllRequestedModels(false); // otherwise gets stuck :(
#endif
			break;
		}
		CPed* pPed = new CCivilianPed(PEDTYPE_SPECIAL, m_nPedmodelId);
		pPed->CharCreatedBy = MISSION_CHAR;
		CWorld::Add(pPed);
		pPed->SetPosition(m_vecCurrentPosition);
		pPed->SetOrientation(0.0f, 0.0f, 0.0f);
		for (int i = 0; i < NUM_ACTORS_IN_MOVIE; i++) {
			if (pActors[i] == nil) {
				m_nActor = i;
				pActors[i] = pPed;
				break;
			}
		}
		m_nNumActors++;
		m_nCurrentMovieCommand++;
		break;
	}
	case MOVIE_NEW_VEHICLE:
	{
		m_nVehiclemodelId = Movie[m_nCurrentMovieCommand].m_nModelIndex;
		m_vecCurrentPosition = Movie[m_nCurrentMovieCommand].m_vecPosition;
		if (!CStreaming::HasModelLoaded(m_nVehiclemodelId)) {
			CStreaming::RequestModel(m_nVehiclemodelId, 0);
#ifdef FIX_BUGS
			CStreaming::LoadAllRequestedModels(false); // otherwise gets stuck :(
#endif
			break;
		}
		CVehicle* pVehicle = new CAutomobile(m_nVehiclemodelId, MISSION_VEHICLE);
		pVehicle->SetStatus(STATUS_PHYSICS);
		pVehicle->SetPosition(m_vecCurrentPosition);
		pVehicle->SetOrientation(0.0f, 0.0f, 0.0f);
		CWorld::Add(pVehicle);
		for (int i = 0; i < NUM_VEHICLES_IN_MOVIE; i++) {
			if (pVehicles[i] == nil) {
				m_nVehicle = i;
				pVehicles[i] = pVehicle;
				break;
			}
		}
		m_nNumVehicles++;
		m_nCurrentMovieCommand++;
		break;
	}
	case MOVIE_GIVE_WEAPON:
		m_nActor = Movie[m_nCurrentMovieCommand].m_nActorId;
		m_nWeaponType = Movie[m_nCurrentMovieCommand].m_nModelIndex;
		pActors[m_nActor]->GiveWeapon((eWeaponType)m_nWeaponType, 1000);
		pActors[m_nActor]->AddWeaponModel(CWeaponInfo::GetWeaponInfo(pActors[m_nActor]->GetWeapon()->m_eWeaponType)->m_nModelId);
		pActors[m_nActor]->SetCurrentWeapon(m_nWeaponType);
		m_nCurrentMovieCommand++;
		break;
	case MOVIE_GOTO:
	case MOVIE_GOTO_WAIT:
		m_nActor = Movie[m_nCurrentMovieCommand].m_nActorId;
		m_vecGotoPosition = Movie[m_nCurrentMovieCommand].m_vecPosition;
		if (pActors[m_nActor]->bInVehicle) {
			if (pActors[m_nActor]->m_pMyVehicle->AutoPilot.m_nCarMission != MISSION_GOTOCOORDS &&
				pActors[m_nActor]->m_pMyVehicle->AutoPilot.m_nCarMission != MISSION_GOTOCOORDS_STRAIGHT) {
				if ((pActors[m_nActor]->m_pMyVehicle->GetPosition() - m_vecGotoPosition).Magnitude() < 5.0f) {
					if (CCarCtrl::JoinCarWithRoadSystemGotoCoors(pActors[m_nActor]->m_pMyVehicle, m_vecGotoPosition, false))
						pActors[m_nActor]->m_pMyVehicle->AutoPilot.m_nCarMission = MISSION_GOTOCOORDS_STRAIGHT;
					else
						pActors[m_nActor]->m_pMyVehicle->AutoPilot.m_nCarMission = MISSION_GOTOCOORDS;
					pActors[m_nActor]->m_pMyVehicle->SetStatus(STATUS_PHYSICS);
					pActors[m_nActor]->m_pMyVehicle->bEngineOn = true;
					pActors[m_nActor]->m_pMyVehicle->AutoPilot.m_nCruiseSpeed = Max(16, pActors[m_nActor]->m_pMyVehicle->AutoPilot.m_nCruiseSpeed);
					pActors[m_nActor]->m_pMyVehicle->AutoPilot.m_nAntiReverseTimer = CTimer::GetTimeInMilliseconds();
					if (m_nCurrentCommand != MOVIE_GOTO_WAIT)
						++m_nCurrentMovieCommand;
				}
				else
					++m_nCurrentMovieCommand;
			}
		}
		else {
			if (pActors[m_nActor]->m_objective != OBJECTIVE_GOTO_AREA_ON_FOOT) {
				pActors[m_nActor]->SetObjective(OBJECTIVE_GOTO_AREA_ON_FOOT, m_vecGotoPosition);
				++m_nCurrentMovieCommand;
			}
		}
		break;
	case MOVIE_GET_IN_CAR:
		m_nActor = Movie[m_nCurrentMovieCommand].m_nActorId;
		if (!pActors[m_nActor]->bInVehicle){
			m_nVehicle = Movie[m_nCurrentMovieCommand].m_nVehicleId;
			pActors[m_nActor]->SetObjective(OBJECTIVE_ENTER_CAR_AS_DRIVER, pVehicles[m_nVehicle]);
		}
		else
			++m_nCurrentMovieCommand;
		break;
	case MOVIE_GET_OUT_CAR:
		m_nActor = Movie[m_nCurrentMovieCommand].m_nActorId;
		if (pActors[m_nActor]->bInVehicle)
			pActors[m_nActor]->SetObjective(OBJECTIVE_LEAVE_CAR);
		else
			++m_nCurrentMovieCommand;
		break;
	case MOVIE_KILL:
		m_nActor = Movie[m_nCurrentMovieCommand].m_nActorId;
		m_nActor2 = Movie[m_nCurrentMovieCommand].m_nActor2Id;
		pActors[m_nActor]->SetObjective(OBJECTIVE_KILL_CHAR_ON_FOOT, pActors[m_nActor2]);
		if (pActors[m_nActor2]->GetPedState() == PED_DEAD)
			++m_nCurrentMovieCommand;
		break;
	case MOVIE_FLEE:
		m_nActor = Movie[m_nCurrentMovieCommand].m_nActorId;
		m_nActor2 = Movie[m_nCurrentMovieCommand].m_nActor2Id;
		pActors[m_nActor]->SetObjective(OBJECTIVE_FLEE_CHAR_ON_FOOT_ALWAYS, pActors[m_nActor2]);
		++m_nCurrentMovieCommand;
		break;
	case MOVIE_WAIT:
		m_nActor = Movie[m_nCurrentMovieCommand].m_nActorId;
		pActors[m_nActor]->SetObjective(OBJECTIVE_WAIT_ON_FOOT);
		++m_nCurrentMovieCommand;
		break;
	case MOVIE_POSITION_CAMERA:
		TheCamera.Cams[TheCamera.ActiveCam].Source = Movie[m_nCurrentMovieCommand].m_vecPosition;
		m_vecCamHeading = Movie[m_nCurrentMovieCommand].m_vecCamera;
		TheCamera.Cams[TheCamera.ActiveCam].Front = m_vecCamHeading;
		++m_nCurrentMovieCommand;
		break;
	case MOVIE_SET_CAMERA_TARGET:
		m_bCameraFollowActor = true;
		TheCamera.Cams[TheCamera.ActiveCam].CamTargetEntity = pActors[Movie[m_nNumMovieCommands].m_nActorId];
		TheCamera.pTargetEntity = pActors[Movie[m_nNumMovieCommands].m_nActorId];
		TheCamera.m_bLookingAtPlayer = false;
		++m_nCurrentMovieCommand;
		break;
	case MOVIE_SELECT_CAMERA_MODE:
		m_bCommandActive = false; // this is wrong
		break;
	}
}

void CSceneEdit::ClearForNewCommand(void)
{
	m_nActor = -1;
	m_nActor2 = -1;
	m_nVehicle = -1;
	m_bActorSelected = false;
	m_bActor2Selected = false;
	m_bVehicleSelected = false;
	m_bDrawGotoArrow = false;
}
void CSceneEdit::SelectActor(void)
{
	m_bActorSelected = false;
	if (m_nActor != -1) {
		if (CPad::GetPad(1)->GetLeftShoulder1JustDown()) {
			CPed* pPed;
			do {
				if (--m_nActor < 0)
					m_nActor = NUM_ACTORS_IN_MOVIE - 1;
				pPed = pActors[m_nActor];
			} while (pPed == nil);
			TheCamera.Cams[TheCamera.ActiveCam].Source = pPed->GetPosition() - m_vecCamHeading;
		}
		else if (CPad::GetPad(1)->GetRightShoulder1JustDown()) {
			CPed* pPed;
			do {
				if (++m_nActor == NUM_ACTORS_IN_MOVIE)
					m_nActor = 0;
				pPed = pActors[m_nActor];
			} while (pPed == nil);
			TheCamera.Cams[TheCamera.ActiveCam].Source = pPed->GetPosition() - m_vecCamHeading;
		}
		m_vecCurrentPosition = pActors[m_nActor]->GetPosition();
		if (CPad::GetPad(1)->GetTriangleJustDown()) {
			m_bActorSelected = true;
			m_bCommandActive = false;
		}
		else if (CPad::GetPad(1)->GetCircleJustDown()) {
			m_nActor = -1;
		}
	}
	else if (m_nNumActors != 0) {
		for (int i = 0; i < NUM_ACTORS_IN_MOVIE; i++) {
			if (pActors[i] != nil) {
				m_nActor = i;
				break;
			}
		}
		TheCamera.Cams[TheCamera.ActiveCam].Source = pActors[m_nActor]->GetPosition() - m_vecCamHeading;
		if (m_nNumActors == 1) {
			m_bActorSelected = true;
			m_bCommandActive = false;
		}
	}
	else {
		m_bCommandActive = false;
	}
}

void CSceneEdit::SelectActor2(void)
{
	m_bActor2Selected = false;
	if (m_nNumActors <= 1) {
		m_bCommandActive = false;
		return;
	}
	if (m_nActor2 != -1) {
		if (CPad::GetPad(1)->GetLeftShoulder1JustDown()) {
			CPed* pPed;
			do {
				if (--m_nActor2 < 0)
					m_nActor2 = NUM_ACTORS_IN_MOVIE - 1;
				pPed = pActors[m_nActor2];
			} while (pPed == nil || pPed == pActors[m_nActor]);
			TheCamera.Cams[TheCamera.ActiveCam].Source = pPed->GetPosition() - m_vecCamHeading;
		}
		else if (CPad::GetPad(1)->GetRightShoulder1JustDown()) {
			CPed* pPed;
			do {
				if (++m_nActor2 == NUM_ACTORS_IN_MOVIE)
					m_nActor2 = 0;
				pPed = pActors[m_nActor2];
			} while (pPed == nil || pPed == pActors[m_nActor]);
			TheCamera.Cams[TheCamera.ActiveCam].Source = pPed->GetPosition() - m_vecCamHeading;
		}
		m_vecCurrentPosition = pActors[m_nActor2]->GetPosition();
		if (CPad::GetPad(1)->GetTriangleJustDown()) {
			m_bActor2Selected = true;
			m_bCommandActive = false;
		}
		else if (CPad::GetPad(1)->GetCircleJustDown()) {
			m_nActor2 = -1;
		}
	}
	else {
		for (int i = 0; i < NUM_ACTORS_IN_MOVIE; i++) {
			if (pActors[i] != nil && pActors[m_nActor] != pActors[i] ) {
				m_nActor2 = i;
				break;
			}
		}
		TheCamera.Cams[TheCamera.ActiveCam].Source = pActors[m_nActor2]->GetPosition() - m_vecCamHeading;
	}
}

void CSceneEdit::SelectVehicle(void)
{
	m_bVehicleSelected = false;
	if (m_nVehicle != -1) {
		if (CPad::GetPad(1)->GetLeftShoulder1JustDown()) {
			CVehicle* pVehicle;
			do {
				if (--m_nVehicle < 0)
					m_nVehicle = NUM_VEHICLES_IN_MOVIE - 1;
				pVehicle = pVehicles[m_nVehicle];
			} while (pVehicle == nil);
		}
		else if (CPad::GetPad(1)->GetRightShoulder1JustDown()) {
			CVehicle* pVehicle;
			do {
				if (++m_nVehicle == NUM_VEHICLES_IN_MOVIE)
					m_nVehicle = 0;
				pVehicle = pVehicles[m_nVehicle];
			} while (pVehicle == nil);
		}
		m_vecCurrentPosition = pVehicles[m_nVehicle]->GetPosition();
		TheCamera.Cams[TheCamera.ActiveCam].Source = pVehicles[m_nVehicle]->GetPosition() - m_vecCamHeading;
		if (CPad::GetPad(1)->GetTriangleJustDown()) {
			m_bVehicleSelected = true;
			m_bCommandActive = false;
		}
		else if (CPad::GetPad(1)->GetCircleJustDown()) {
			m_nVehicle = -1;
		}
	}
	else if (m_nNumVehicles != 0) {
		for (int i = 0; i < NUM_ACTORS_IN_MOVIE; i++) {
			if (pVehicles[i] != nil) {
				m_nVehicle = i;
				break;
			}
		}
	}
}

bool CSceneEdit::SelectWeapon(void)
{
	if (m_nWeaponType == WEAPONTYPE_UNARMED) {
		m_nWeaponType = WEAPONTYPE_COLT45;
		return false;
	}
	if (CPad::GetPad(1)->GetLeftShoulder1JustDown()) {
		if (++m_nWeaponType >= WEAPONTYPE_DETONATOR)
			m_nWeaponType = WEAPONTYPE_BASEBALLBAT;
		pActors[m_nActor]->ClearWeapons();
		pActors[m_nActor]->GiveWeapon((eWeaponType)m_nWeaponType, 1000);
		pActors[m_nActor]->AddWeaponModel(CWeaponInfo::GetWeaponInfo(pActors[m_nActor]->GetWeapon()->m_eWeaponType)->m_nModelId);
		pActors[m_nActor]->SetCurrentWeapon(m_nWeaponType);
	}
	else if (CPad::GetPad(1)->GetRightShoulder1JustDown()){
		if (--m_nWeaponType <= WEAPONTYPE_UNARMED)
			m_nWeaponType = WEAPONTYPE_GRENADE;
		pActors[m_nActor]->ClearWeapons();
		pActors[m_nActor]->GiveWeapon((eWeaponType)m_nWeaponType, 1000);
		pActors[m_nActor]->AddWeaponModel(CWeaponInfo::GetWeaponInfo(pActors[m_nActor]->GetWeapon()->m_eWeaponType)->m_nModelId);
		pActors[m_nActor]->SetCurrentWeapon(m_nWeaponType);
	}
	if (CPad::GetPad(1)->GetTriangleJustDown()) {
		m_bCommandActive = false;
		return true;
	}
	if (CPad::GetPad(1)->GetCircleJustDown()) {
		pActors[m_nActor]->ClearWeapons();
		m_nWeaponType = WEAPONTYPE_UNARMED;
		m_bCommandActive = false;
		return false;
	}
	return false;
}
#endif
