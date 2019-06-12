#include "common.h"
#include "patcher.h"
#include "Messages.h"

WRAPPER void CMessages::Display(void) { EAXJMP(0x529800); }
WRAPPER void CMessages::ClearAllMessagesDisplayedByGame(void) { EAXJMP(0x52B670); }
WRAPPER int CMessages::WideStringCopy(wchar* dst, wchar* src, unsigned short size) { EAXJMP(0x5294B0); }
WRAPPER char CMessages::WideStringCompare(wchar* str1, wchar* str2, unsigned short size) { EAXJMP(0x529510); }
WRAPPER void CMessages::InsertNumberInString(wchar* src, int n1, int n2, int n3, int n4, int n5, int n6, wchar* dst) { EAXJMP(0x52A1A0); }
WRAPPER void CMessages::InsertPlayerControlKeysInString(wchar* src) { EAXJMP(0x52A490); }

tPreviousBrief *CMessages::PreviousBriefs = (tPreviousBrief *)0x713C08;
tMessage *CMessages::BriefMessages = (tMessage *)0x8786E0;
tBigMessage *CMessages::BIGMessages = (tBigMessage *)0x773628;
