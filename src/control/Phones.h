#pragma once

#include "Physical.h"

class CPed;
class CAnimBlendAssociation;

enum {
	PHONE_STATE_FREE,
	PHONE_STATE_1,
	PHONE_STATE_2,
	PHONE_STATE_MESSAGE_REMOVED,
	PHONE_STATE_ONETIME_MESSAGE_SET,
	PHONE_STATE_REPEATED_MESSAGE_SET,
	PHONE_STATE_REPEATED_MESSAGE_SHOWN_ONCE,
	PHONE_STATE_ONETIME_MESSAGE_SHOWN,
	PHONE_STATE_REPEATED_MESSAGE_SHOWN,
	PHONE_STATE_9
};

struct CPhone
{
	CVector m_vecPos;
	wchar *m_apMessages[6];
	uint32 m_lastTimeRepeatedMsgShown;
	CEntity *m_pEntity; // it's building pool index in save files
	int32 m_nState;
	uint8 field_30;
};

static_assert(sizeof(CPhone) == 0x34, "CPhone: error");

class CPhoneInfo {
public:
	static bool &isPhonePickedUp;
	static uint32 &phoneMessagesTimer;
	static CPhone *&pickedUpPhone;
	static bool &isPhoneBeingPickedUp;
	static CPed *&pedWhoPickingUpPhone;

	int32 m_nMax;
	int32 m_nNum;
	CPhone m_aPhones[50];

	CPhoneInfo() { }
	~CPhoneInfo() { }

	int FindNearestFreePhone(CVector*);
	bool PhoneAtThisPosition(CVector);
	bool HasMessageBeenDisplayed(int);
	bool IsMessageBeingDisplayed(int);
	void Load(uint8 *buf, uint32 size);
	void Save(uint8 *buf, uint32 *size);
	void SetPhoneMessage_JustOnce(int phoneId, wchar *msg1, wchar *msg2, wchar *msg3, wchar *msg4, wchar *msg5, wchar *msg6);
	void SetPhoneMessage_Repeatedly(int phoneId, wchar *msg1, wchar *msg2, wchar *msg3, wchar *msg4, wchar *msg5, wchar *msg6);
	int GrabPhone(float, float);
	void Initialise(void);
	void Shutdown(void);
	void Update(void);
};

extern CPhoneInfo &gPhoneInfo;

void PhonePutDownCB(CAnimBlendAssociation *assoc, void *arg);
void PhonePickUpCB(CAnimBlendAssociation *assoc, void *arg);