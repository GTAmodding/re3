#pragma once

#include "Physical.h"

class CPed;
class CAnimBlendAssociation;

enum PhoneState {
	PHONE_STATE_FREE,
	PHONE_STATE_REPORTING_CRIME, // CCivilianPed::ProcessControl sets it but unused
	PHONE_STATE_2,
	PHONE_STATE_MESSAGE_REMOVED,
	PHONE_STATE_ONETIME_MESSAGE_SET,
	PHONE_STATE_REPEATED_MESSAGE_SET,
	PHONE_STATE_REPEATED_MESSAGE_SHOWN_ONCE,
	PHONE_STATE_ONETIME_MESSAGE_STARTED,
	PHONE_STATE_REPEATED_MESSAGE_STARTED,
	PHONE_STATE_9 // just rings, picking being handled via script. most of the time game uses this
};

class CPhone
{
public:
	CVector m_vecPos;
	wchar *m_apMessages[6];
	uint32 m_repeatedMessagePickupStart;
	CEntity *m_pEntity; // stored as building pool index in save files
	PhoneState m_nState;
	bool m_visibleToCam;

	CPhone() { }
	~CPhone() { }
};

VALIDATE_SIZE(CPhone, 0x34);

class CPhoneInfo {
public:
	static bool bDisplayingPhoneMessage;
	static uint32 PhoneEnableControlsTimer;
	static CPhone *pPhoneDisplayingMessages;
	static bool bPickingUpPhone;
	static CPed *pCallBackPed;

	int32 m_nMax;
	int32 m_nScriptPhonesMax;
	CPhone m_aPhones[NUMPHONES];

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
#ifdef PEDS_REPORT_CRIMES_ON_PHONE
	void SwapPhone(float xPos, float yPos, int into);
#endif
};

extern CPhoneInfo gPhoneInfo;

void PhonePutDownCB(CAnimBlendAssociation *assoc, void *arg);
void PhonePickUpCB(CAnimBlendAssociation *assoc, void *arg);

#ifdef PEDS_REPORT_CRIMES_ON_PHONE
extern CPed *crimeReporters[NUMPHONES];
bool isPhoneAvailable(int);
#endif
