#include "common.h"

#include "Phones.h"
#include "Pools.h"
#include "ModelIndices.h"
#include "Ped.h"
#include "Pad.h"
#include "Messages.h"
#include "Camera.h"
#include "World.h"
#include "General.h"
#include "AudioScriptObject.h"
#include "RpAnimBlend.h"
#include "AnimBlendAssociation.h"
#include "soundlist.h"
#ifdef FIX_BUGS
#include "Replay.h"
#endif

CPhoneInfo gPhoneInfo;

bool CPhoneInfo::bDisplayingPhoneMessage;  // is phone picked up
uint32 CPhoneInfo::PhoneEnableControlsTimer;
CPhone *CPhoneInfo::pPhoneDisplayingMessages;
bool CPhoneInfo::bPickingUpPhone;
CPed *CPhoneInfo::pCallBackPed; // ped who picking up the phone (reset after pickup cb)

/*
	Entering phonebooth cutscene, showing messages and triggering these things
	by checking coordinates happens in here - blue mission marker is cosmetic.

	Repeated message means after the script set the messages for a particular phone,
	player can pick the phone again with the same messages appearing,
	after 60 seconds of last phone pick-up.
*/

#ifdef PEDS_REPORT_CRIMES_ON_PHONE
CPed* crimeReporters[NUMPHONES] = {};
bool
isPhoneAvailable(int m_phoneId)
{
	return crimeReporters[m_phoneId] == nil || !crimeReporters[m_phoneId]->IsPointerValid() || crimeReporters[m_phoneId]->m_objective > OBJECTIVE_WAIT_ON_FOOT ||
			(crimeReporters[m_phoneId]->m_nPedState != PED_MAKE_CALL && crimeReporters[m_phoneId]->m_nPedState != PED_FACE_PHONE && crimeReporters[m_phoneId]->m_nPedState != PED_SEEK_POS);
}
#endif

void
CPhoneInfo::Update(void)
{
#ifdef FIX_BUGS
	if (CReplay::IsPlayingBack())
		return;
#endif
	CPlayerPed *player = FindPlayerPed();
	CPlayerInfo *playerInfo = &CWorld::Players[CWorld::PlayerInFocus];
	if (bDisplayingPhoneMessage && CTimer::GetTimeInMilliseconds() > PhoneEnableControlsTimer) {
		playerInfo->MakePlayerSafe(false);
		TheCamera.SetWideScreenOff();
		pPhoneDisplayingMessages = nil;
		bDisplayingPhoneMessage = false;
		CAnimBlendAssociation *talkAssoc = RpAnimBlendClumpGetAssociation(player->GetClump(), ANIM_STD_PHONE_TALK);
		if (talkAssoc && talkAssoc->blendAmount > 0.5f) {
			CAnimBlendAssociation *endAssoc = CAnimManager::BlendAnimation(player->GetClump(), ASSOCGRP_STD, ANIM_STD_PHONE_OUT, 8.0f);
			endAssoc->flags &= ~ASSOC_DELETEFADEDOUT;
			endAssoc->SetFinishCallback(PhonePutDownCB, player);
		} else {
			CPad::GetPad(0)->SetEnablePlayerControls(PLAYERCONTROL_PHONE);
			if (player->m_nPedState == PED_MAKE_CALL)
				player->SetPedState(PED_IDLE);
		}
	}
	bool notInCar;
	CVector playerPos;
	if (FindPlayerVehicle()) {
		notInCar = false;
		playerPos = FindPlayerVehicle()->GetPosition();
	} else {
		notInCar = true;
		playerPos = player->GetPosition();
	}
	bool phoneRings = false;
	bool scratchTheCabinet;
	for(int phoneId = 0; phoneId < m_nScriptPhonesMax; phoneId++) {
		if (m_aPhones[phoneId].m_visibleToCam) {
			switch (m_aPhones[phoneId].m_nState) {
				case PHONE_STATE_ONETIME_MESSAGE_SET:
				case PHONE_STATE_REPEATED_MESSAGE_SET:
				case PHONE_STATE_REPEATED_MESSAGE_SHOWN_ONCE:
					if (bPickingUpPhone) {
						scratchTheCabinet = false;
						phoneRings = false;
					} else {
						scratchTheCabinet = (CTimer::GetTimeInMilliseconds() / 1880) % 2 == 1;
						phoneRings = (CTimer::GetPreviousTimeInMilliseconds() / 1880) % 2 == 1;
					}
					if (scratchTheCabinet) {
						m_aPhones[phoneId].m_pEntity->GetUp().z = (CGeneral::GetRandomNumber() % 1024) / 16000.0f + 1.0f;
						if (!phoneRings)
						    PlayOneShotScriptObject(SCRIPT_SOUND_PAYPHONE_RINGING, m_aPhones[phoneId].m_pEntity->GetPosition());
					} else {
						m_aPhones[phoneId].m_pEntity->GetUp().z = 1.0f;
					}
					m_aPhones[phoneId].m_pEntity->GetMatrix().UpdateRW();
					m_aPhones[phoneId].m_pEntity->UpdateRwFrame();
					if (notInCar && !bPickingUpPhone && player->IsPedInControl()) {
						CVector2D distToPhone = playerPos - m_aPhones[phoneId].m_vecPos;
						if (Abs(distToPhone.x) < 1.0f && Abs(distToPhone.y) < 1.0f) {
							if (DotProduct2D(distToPhone, m_aPhones[phoneId].m_pEntity->GetForward()) / distToPhone.Magnitude() < -0.85f) {
								CVector2D distToPhoneObj = playerPos - m_aPhones[phoneId].m_pEntity->GetPosition();
								float angleToFace = CGeneral::GetATanOfXY(distToPhoneObj.x, distToPhoneObj.y) + HALFPI;
								if (angleToFace > TWOPI)
									angleToFace = angleToFace - TWOPI;
								player->m_fRotationCur = angleToFace;
								player->m_fRotationDest = angleToFace;
								player->SetHeading(angleToFace);
								player->SetPedState(PED_MAKE_CALL);
								CPad::GetPad(0)->SetDisablePlayerControls(PLAYERCONTROL_PHONE);
								TheCamera.SetWideScreenOn();
								playerInfo->MakePlayerSafe(true);
								CAnimBlendAssociation *phonePickAssoc = CAnimManager::BlendAnimation(player->GetClump(), ASSOCGRP_STD, ANIM_STD_PHONE_IN, 4.0f);
								phonePickAssoc->SetFinishCallback(PhonePickUpCB, &m_aPhones[phoneId]);
								bPickingUpPhone = true;
								pCallBackPed = player;
							}
						}
					}
					break;
				case PHONE_STATE_REPEATED_MESSAGE_STARTED:
					if (CTimer::GetTimeInMilliseconds() - m_aPhones[phoneId].m_repeatedMessagePickupStart > 60000)
						m_aPhones[phoneId].m_nState = PHONE_STATE_REPEATED_MESSAGE_SHOWN_ONCE;
					break;
				case PHONE_STATE_9:
					scratchTheCabinet = (CTimer::GetTimeInMilliseconds() / 1880) % 2 == 1;
					phoneRings = (CTimer::GetPreviousTimeInMilliseconds() / 1880) % 2 == 1;
					if (scratchTheCabinet) {
						m_aPhones[phoneId].m_pEntity->GetUp().z = (CGeneral::GetRandomNumber() % 1024) / 16000.0f + 1.0f;
						if (!phoneRings)
						    PlayOneShotScriptObject(SCRIPT_SOUND_PAYPHONE_RINGING, m_aPhones[phoneId].m_pEntity->GetPosition());
					} else {
						m_aPhones[phoneId].m_pEntity->GetUp().z = 1.0f;
					}
					m_aPhones[phoneId].m_pEntity->GetMatrix().UpdateRW();
					m_aPhones[phoneId].m_pEntity->UpdateRwFrame();
					break;
				default:
					break;
			}
			if (CVector2D(TheCamera.GetPosition() - m_aPhones[phoneId].m_vecPos).MagnitudeSqr() > sq(100.0f))
				m_aPhones[phoneId].m_visibleToCam = false;
		} else if (!((CTimer::GetFrameCounter() + m_aPhones[phoneId].m_pEntity->m_randomSeed) % 16)) {
			if (CVector2D(TheCamera.GetPosition() - m_aPhones[phoneId].m_vecPos).MagnitudeSqr() < sq(60.0f))
				m_aPhones[phoneId].m_visibleToCam = true;
		}
	}
}

int
CPhoneInfo::FindNearestFreePhone(CVector *pos)
{
	int nearestPhoneId = -1;
	float nearestPhoneDist = 60.0f;

 	for (int phoneId = 0; phoneId < m_nMax; phoneId++) {

#ifdef PEDS_REPORT_CRIMES_ON_PHONE
		if (isPhoneAvailable(phoneId))
#else
		if (gPhoneInfo.m_aPhones[phoneId].m_nState == PHONE_STATE_FREE)
#endif
		{
			float phoneDist = (m_aPhones[phoneId].m_vecPos - *pos).Magnitude2D();

			if (phoneDist < nearestPhoneDist) {
				nearestPhoneDist = phoneDist;
				nearestPhoneId = phoneId;
			}
		}
	}
	return nearestPhoneId;
}

bool
CPhoneInfo::PhoneAtThisPosition(CVector pos)
{
	for (int phoneId = 0; phoneId < m_nMax; phoneId++) {
		if (pos.x == m_aPhones[phoneId].m_vecPos.x && pos.y == m_aPhones[phoneId].m_vecPos.y)
			return true;
	}
	return false;
}

bool
CPhoneInfo::HasMessageBeenDisplayed(int phoneId)
{
	if (bDisplayingPhoneMessage)
		return false;

	int state = m_aPhones[phoneId].m_nState;

	return state == PHONE_STATE_REPEATED_MESSAGE_SHOWN_ONCE ||
		state == PHONE_STATE_ONETIME_MESSAGE_STARTED ||
		state == PHONE_STATE_REPEATED_MESSAGE_STARTED;
}

bool
CPhoneInfo::IsMessageBeingDisplayed(int phoneId)
{
	return pPhoneDisplayingMessages == &m_aPhones[phoneId];
}

void
CPhoneInfo::Load(uint8 *buf, uint32 size)
{
INITSAVEBUF
	int max = ReadSaveBuf<int32>(buf);
	int scriptPhonesMax = ReadSaveBuf<int32>(buf);

#ifdef PEDS_REPORT_CRIMES_ON_PHONE
	m_nMax = Min(NUMPHONES, max);
	m_nScriptPhonesMax = 0;

	bool ignoreOtherPhones = false;

	// We can do it without touching saves. We'll only load script phones, others are already loaded in Initialise
	for (int i = 0; i < 50; i++) {
		CPhone phoneToLoad = ReadSaveBuf<CPhone>(buf);

		if (ignoreOtherPhones)
			continue;

		if (i < scriptPhonesMax) {
			if (i >= m_nMax) {
				assert(0 && "Number of phones used by script exceeds the NUMPHONES or the stored phones in save file. Ignoring some phones");
				ignoreOtherPhones = true;
				continue;
			}
			SwapPhone(phoneToLoad.m_vecPos.x, phoneToLoad.m_vecPos.y, i);

			m_aPhones[i] = phoneToLoad;
			// It's saved as building pool index in save file, convert it to true entity
			if (m_aPhones[i].m_pEntity) {
				m_aPhones[i].m_pEntity = CPools::GetBuildingPool()->GetSlot((uintptr)m_aPhones[i].m_pEntity - 1);
			}
		} else 
			ignoreOtherPhones = true;
	}
#else
	m_nMax = max;
	m_nScriptPhonesMax = scriptPhonesMax;

	for (int i = 0; i < NUMPHONES; i++) {
		m_aPhones[i] = ReadSaveBuf<CPhone>(buf);
		// It's saved as building pool index in save file, convert it to true entity
		if (m_aPhones[i].m_pEntity) {
			m_aPhones[i].m_pEntity = CPools::GetBuildingPool()->GetSlot((uintptr)m_aPhones[i].m_pEntity - 1);
		}
	}
#endif
VALIDATESAVEBUF(size)
}

void
CPhoneInfo::SetPhoneMessage_JustOnce(int phoneId, wchar *msg1, wchar *msg2, wchar *msg3, wchar *msg4, wchar *msg5, wchar *msg6)
{
	// If there is at least one message, it should be msg1.
	if (msg1) {
		m_aPhones[phoneId].m_apMessages[0] = msg1;
		m_aPhones[phoneId].m_apMessages[1] = msg2;
		m_aPhones[phoneId].m_apMessages[2] = msg3;
		m_aPhones[phoneId].m_apMessages[3] = msg4;
		m_aPhones[phoneId].m_apMessages[4] = msg5;
		m_aPhones[phoneId].m_apMessages[5] = msg6;
		m_aPhones[phoneId].m_nState = PHONE_STATE_ONETIME_MESSAGE_SET;
	} else {
		m_aPhones[phoneId].m_nState = PHONE_STATE_MESSAGE_REMOVED;
	}
}

void
CPhoneInfo::SetPhoneMessage_Repeatedly(int phoneId, wchar *msg1, wchar *msg2, wchar *msg3, wchar *msg4, wchar *msg5, wchar *msg6)
{
	// If there is at least one message, it should be msg1.
	if (msg1) {
		m_aPhones[phoneId].m_apMessages[0] = msg1;
		m_aPhones[phoneId].m_apMessages[1] = msg2;
		m_aPhones[phoneId].m_apMessages[2] = msg3;
		m_aPhones[phoneId].m_apMessages[3] = msg4;
		m_aPhones[phoneId].m_apMessages[4] = msg5;
		m_aPhones[phoneId].m_apMessages[5] = msg6;
		m_aPhones[phoneId].m_nState = PHONE_STATE_REPEATED_MESSAGE_SET;
	} else {
		m_aPhones[phoneId].m_nState = PHONE_STATE_MESSAGE_REMOVED;
	}
}

#ifdef PEDS_REPORT_CRIMES_ON_PHONE
void
CPhoneInfo::SwapPhone(float xPos, float yPos, int into)
{
	// "into" should be in 0 - m_nScriptPhonesMax range
	int nearestPhoneId = -1;
	CVector pos(xPos, yPos, 0.0f);
	float nearestPhoneDist = 1.0f;

	for (int phoneId = m_nScriptPhonesMax; phoneId < m_nMax; phoneId++) {
		float phoneDistance = (m_aPhones[phoneId].m_vecPos - pos).Magnitude2D();
		if (phoneDistance < nearestPhoneDist) {
			nearestPhoneDist = phoneDistance;
			nearestPhoneId = phoneId;
		}
	}
	m_aPhones[nearestPhoneId].m_nState = PHONE_STATE_MESSAGE_REMOVED;

	CPhone oldPhone = m_aPhones[into];
	m_aPhones[into] = m_aPhones[nearestPhoneId];
	m_aPhones[nearestPhoneId] = oldPhone;
	m_nScriptPhonesMax++;
}
#endif

int
CPhoneInfo::GrabPhone(float xPos, float yPos)
{
	// "Grab" doesn't mean picking up the phone, it means allocating some particular phone to
	// whoever called the 024A opcode first with the position parameters closest to phone.
	// Same phone won't be available on next run of this function.

	int nearestPhoneId = -1;
	CVector pos(xPos, yPos, 0.0f);
	float nearestPhoneDist = 100.0f;

	for (int phoneId = m_nScriptPhonesMax; phoneId < m_nMax; phoneId++) {
		float phoneDistance = (m_aPhones[phoneId].m_vecPos - pos).Magnitude2D();
		if (phoneDistance < nearestPhoneDist) {
			nearestPhoneDist = phoneDistance;
			nearestPhoneId = phoneId;
		}
	}
	m_aPhones[nearestPhoneId].m_nState = PHONE_STATE_MESSAGE_REMOVED;

	CPhone oldFirstPhone = m_aPhones[m_nScriptPhonesMax];
	m_aPhones[m_nScriptPhonesMax] = m_aPhones[nearestPhoneId];
	m_aPhones[nearestPhoneId] = oldFirstPhone;
	m_nScriptPhonesMax++;
	return m_nScriptPhonesMax - 1;
}

void
CPhoneInfo::Initialise(void)
{
	CBuildingPool *pool = CPools::GetBuildingPool();
	pCallBackPed = nil;
	bDisplayingPhoneMessage = false;
	bPickingUpPhone = false;
	pPhoneDisplayingMessages = nil;
	m_nMax = 0;
	m_nScriptPhonesMax = 0;
	for (int i = pool->GetSize() - 1; i >= 0; i--) {
		CBuilding *building = pool->GetSlot(i);
		if (building) {
			if (building->GetModelIndex() == MI_PHONEBOOTH1) {
				assert(m_nMax < ARRAY_SIZE(m_aPhones) && "NUMPHONES should be increased");
				CPhone *maxPhone = &m_aPhones[m_nMax];
				maxPhone->m_nState = PHONE_STATE_FREE;
				maxPhone->m_vecPos = building->GetPosition();
				maxPhone->m_pEntity = building;
				m_nMax++;
			}
		}
	}
}

void
CPhoneInfo::Save(uint8 *buf, uint32 *size)
{
	*size = sizeof(CPhoneInfo);
INITSAVEBUF
	WriteSaveBuf(buf, m_nMax);
	WriteSaveBuf(buf, m_nScriptPhonesMax);
#ifdef PEDS_REPORT_CRIMES_ON_PHONE
	for (int phoneId = 0; phoneId < 50; phoneId++) { // We can do it without touching saves
#else
	for (int phoneId = 0; phoneId < NUMPHONES; phoneId++) {
#endif
		CPhone* phone = WriteSaveBuf(buf, m_aPhones[phoneId]);

		// Convert entity pointer to building pool index while saving
		if (phone->m_pEntity) {
			phone->m_pEntity = (CEntity*) (CPools::GetBuildingPool()->GetJustIndex_NoFreeAssert((CBuilding*)phone->m_pEntity) + 1);
		}
	}
VALIDATESAVEBUF(*size)
}

void
CPhoneInfo::Shutdown(void)
{
	m_nMax = 0;
	m_nScriptPhonesMax = 0;
}

void
PhonePutDownCB(CAnimBlendAssociation *assoc, void *arg)
{
	assoc->flags |= ASSOC_DELETEFADEDOUT;
	assoc->blendDelta = -1000.0f;
	CPad::GetPad(0)->SetEnablePlayerControls(PLAYERCONTROL_PHONE);
	CPed *ped = (CPed*)arg;

	if (assoc->blendAmount > 0.5f)
		ped->bUpdateAnimHeading = true;

	if (ped->m_nPedState == PED_MAKE_CALL)
		ped->SetPedState(PED_IDLE);
}

void
PhonePickUpCB(CAnimBlendAssociation *assoc, void *arg)
{
	CPhone *phone = (CPhone*)arg;
	int messagesDisplayTime = 0;

	for(int i=0; i < 6; i++) {
		wchar *msg = phone->m_apMessages[i];
		if (msg) {
			CMessages::AddMessage(msg, 3000, 0);
			messagesDisplayTime += 3000;
		}
	}

	CPhoneInfo::bPickingUpPhone = false;
	CPhoneInfo::bDisplayingPhoneMessage = true;
	CPhoneInfo::pPhoneDisplayingMessages = phone;
	CPhoneInfo::PhoneEnableControlsTimer = CTimer::GetTimeInMilliseconds() + messagesDisplayTime;

	if (phone->m_nState == PHONE_STATE_ONETIME_MESSAGE_SET) {
		phone->m_nState = PHONE_STATE_ONETIME_MESSAGE_STARTED;
	} else {
		phone->m_nState = PHONE_STATE_REPEATED_MESSAGE_STARTED;
		phone->m_repeatedMessagePickupStart = CTimer::GetTimeInMilliseconds();
	}

	CPed *ped = CPhoneInfo::pCallBackPed;
	ped->m_nMoveState = PEDMOVE_STILL;
	CAnimManager::BlendAnimation(ped->GetClump(), ASSOCGRP_STD, ANIM_STD_IDLE, 8.0f);

	if (assoc->blendAmount > 0.5f && ped)
		CAnimManager::BlendAnimation(ped->GetClump(), ASSOCGRP_STD, ANIM_STD_PHONE_TALK, 8.0f);

	CPhoneInfo::pCallBackPed = nil;
}
