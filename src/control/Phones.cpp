#include "common.h"
#include "patcher.h"
#include "Phones.h"
#include "Pools.h"
#include "ModelIndices.h"
#include "Ped.h"
#include "Pad.h"
#include "Messages.h"

CPhoneInfo &gPhoneInfo = *(CPhoneInfo*)0x732A20;

bool &CPhoneInfo::isPhonePickedUp = *(bool*)0x6283AC;
uint32 &CPhoneInfo::phoneMessagesTimer = *(uint32*)0x6283A8;
CPhone *&CPhoneInfo::pickedUpPhone = *(CPhone**)0x6283B0;
bool &CPhoneInfo::isPhoneBeingPickedUp = *(bool*)0x6283B4;
CPed *&CPhoneInfo::pedWhoPickingUpPhone = *(CPed**)0x6283B8;

WRAPPER void CPhoneInfo::Update(void) { EAXJMP(0x42F7A0); }

int
CPhoneInfo::FindNearestFreePhone(CVector *pos)
{
	int nearestPhoneId = -1;
	float nearestPhoneDist = 60.0f;

	for (int phoneId = 0; phoneId < m_nMax; phoneId++) {

		if (gPhoneInfo.m_aPhones[phoneId].m_nState == PHONE_STATE_FREE) {
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
	if (isPhonePickedUp)
		return false;
	
	int state = m_aPhones[phoneId].m_nState;

	return state == PHONE_STATE_REPEATED_MESSAGE_SHOWN_ONCE || 
		state == PHONE_STATE_ONETIME_MESSAGE_SHOWN ||
		state == PHONE_STATE_REPEATED_MESSAGE_SHOWN;
}

bool
CPhoneInfo::IsMessageBeingDisplayed(int phoneId)
{
	return pickedUpPhone == &m_aPhones[phoneId];
}

void
CPhoneInfo::Load(uint8 *buf, uint32 size)
{
INITSAVEBUF
	m_nMax = ReadSaveBuf<int32>(buf);
	m_nNum = ReadSaveBuf<int32>(buf);
	for (int i = 0; i < 50; i++) {
		m_aPhones[i] = ReadSaveBuf<CPhone>(buf);
		// It's saved as building pool index in save file, convert it to true entity
		if (m_aPhones[i].m_pEntity) {
			m_aPhones[i].m_pEntity = CPools::GetBuildingPool()->GetSlot((int)m_aPhones[i].m_pEntity - 1);
		}
	}
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

int
CPhoneInfo::GrabPhone(float xPos, float yPos)
{
	// "Grab" doesn't mean picking up the phone, it means allocating some particular phone to
	// whoever called the 024A opcode first with the position parameters closest to phone.
	// Same phone won't be available on next run of this function.

	int nearestPhoneId = -1;
	CVector pos(xPos, yPos, 0.0f);
	float nearestPhoneDist = 100.0f;

	for (int phoneId = m_nNum; phoneId < m_nMax; phoneId++) {
		float phoneDistance = (m_aPhones[phoneId].m_vecPos - pos).Magnitude2D();
		if (phoneDistance < nearestPhoneDist) {
			nearestPhoneDist = phoneDistance;
			nearestPhoneId = phoneId;
		}
	}
	m_aPhones[nearestPhoneId].m_nState = PHONE_STATE_MESSAGE_REMOVED;

	CPhone oldFirstPhone = m_aPhones[m_nNum];
	m_aPhones[m_nNum] = m_aPhones[nearestPhoneId];
	m_aPhones[nearestPhoneId] = oldFirstPhone;
	m_nNum++;
	return m_nNum - 1;
}

void
CPhoneInfo::Initialise(void)
{
	CBuildingPool *pool = CPools::GetBuildingPool();
	pedWhoPickingUpPhone = nil;
	isPhonePickedUp = false;
	isPhoneBeingPickedUp = false;
	pickedUpPhone = nil;
	m_nMax = 0;
	m_nNum = 0;
	for (int v5 = pool->GetSize() - 1; v5 >= 0; v5--) {
		CBuilding *building = pool->GetSlot(v5);
		if (building) {
			if (building->m_modelIndex == MI_PHONEBOOTH1) {
				CPhone *maxPhone = &m_aPhones[m_nMax];
				maxPhone->m_nState = PHONE_STATE_FREE;
				maxPhone->m_vecPos = *(building->GetPosition());
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
	WriteSaveBuf(buf, m_nNum);
	for(int phoneId = 0; phoneId < 50; phoneId++) {
		CPhone* phone = WriteSaveBuf(buf, m_aPhones[phoneId]);

		// Convert entity pointer to building pool index while saving
		if (phone->m_pEntity) {
			phone->m_pEntity = (CEntity*) (CPools::GetBuildingPool()->GetJustIndex((CBuilding*)phone->m_pEntity) + 1);
		}
	}
VALIDATESAVEBUF(*size)
}

void
CPhoneInfo::Shutdown(void)
{
	m_nMax = 0;
	m_nNum = 0;
}

void
PhonePutDownCB(CAnimBlendAssociation *assoc, void *arg)
{
	assoc->flags |= ASSOC_DELETEFADEDOUT;
	assoc->blendDelta = -1000.0f;
	CPad::GetPad(0)->DisablePlayerControls &= ~PLAYERCONTROL_DISABLED_40;
	CPed *ped = (CPed*)arg;

	if (assoc->blendAmount > 0.5f)
		ped->bUpdateAnimHeading = true;

	if (ped->m_nPedState == PED_MAKE_CALL)
		ped->m_nPedState = PED_IDLE;
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

	CPhoneInfo::isPhoneBeingPickedUp = false;
	CPhoneInfo::isPhonePickedUp = true;
	CPhoneInfo::pickedUpPhone = phone;
	CPhoneInfo::phoneMessagesTimer = CTimer::GetTimeInMilliseconds() + messagesDisplayTime;

	if (phone->m_nState == PHONE_STATE_ONETIME_MESSAGE_SET) {
		phone->m_nState = PHONE_STATE_ONETIME_MESSAGE_SHOWN;
	} else {
		phone->m_nState = PHONE_STATE_REPEATED_MESSAGE_SHOWN;
		phone->m_lastTimeRepeatedMsgShown = CTimer::GetTimeInMilliseconds();
	}

	CPed *ped = CPhoneInfo::pedWhoPickingUpPhone;
	ped->m_nMoveState = PEDMOVE_STILL;
	CAnimManager::BlendAnimation(ped->GetClump(), ASSOCGRP_STD, ANIM_IDLE_STANCE, 8.0f);

	if (assoc->blendAmount > 0.5f && ped)
		CAnimManager::BlendAnimation(ped->GetClump(), ASSOCGRP_STD, ANIM_PHONE_TALK, 8.0f);

	CPhoneInfo::pedWhoPickingUpPhone = nil;
}

STARTPATCHES
	InjectHook(0x42F720, &CPhoneInfo::FindNearestFreePhone, PATCH_JUMP);
	InjectHook(0x42FD50, &CPhoneInfo::PhoneAtThisPosition, PATCH_JUMP);
	InjectHook(0x42FFF0, &CPhoneInfo::HasMessageBeenDisplayed, PATCH_JUMP);
	InjectHook(0x430030, &CPhoneInfo::IsMessageBeingDisplayed, PATCH_JUMP);
	InjectHook(0x430120, &CPhoneInfo::Load, PATCH_JUMP);
	InjectHook(0x42FF90, &CPhoneInfo::SetPhoneMessage_JustOnce, PATCH_JUMP);
	InjectHook(0x42FF30, &CPhoneInfo::SetPhoneMessage_Repeatedly, PATCH_JUMP);
	InjectHook(0x430060, &CPhoneInfo::Save, PATCH_JUMP);
	InjectHook(0x42F710, &CPhoneInfo::Shutdown, PATCH_JUMP);
	InjectHook(0x42F640, &CPhoneInfo::Initialise, PATCH_JUMP);
	InjectHook(0x42FDB0, &CPhoneInfo::GrabPhone, PATCH_JUMP);
	InjectHook(0x42F570, &PhonePutDownCB, PATCH_JUMP);
	InjectHook(0x42F470, &PhonePickUpCB, PATCH_JUMP);
ENDPATCHES