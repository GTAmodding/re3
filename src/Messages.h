#pragma once

struct tMessage {
	wchar        *m_pText;
	unsigned short  m_nFlag;
private:
	char _pad6[2];
public:
	unsigned int    m_nTime;
	unsigned int    m_nStartTime;
	int             m_nNumber[6];
	wchar        *m_pString;
};

struct tBigMessage {
	tMessage m_Current;
	tMessage m_Stack[3];
};

struct tPreviousBrief {
	wchar *m_pText;
	int      m_nNumber[6];
	wchar *m_pString;
};

class CMessages {
public:
	static tPreviousBrief *PreviousBriefs;
	static tMessage *BriefMessages;
	static tBigMessage *BIGMessages;

public:
	static void Display(void);
	static void ClearAllMessagesDisplayedByGame(void);
	static int WideStringCopy(wchar* dst, wchar* src, unsigned short size);
	static char WideStringCompare(wchar* str1, wchar* str2, unsigned short size);
	static void InsertNumberInString(wchar* src, int n1, int n2, int n3, int n4, int n5, int n6, wchar* dst);
	static void InsertPlayerControlKeysInString(wchar* src);
};
