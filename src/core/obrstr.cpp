#include "common.h"
#include "Debug.h"
#include "obrstr.h"

char obrstr[128];
char obrstr2[128];

void ObrInt(int32 n1)
{
	IntToStr(n1, obrstr);
	CDebug::DebugAddText(obrstr);
}

void ObrInt2(int32 n1, int32 n2)
{
	IntToStr(n1, obrstr);
	strcat(obrstr, "  ");
	IntToStr(n2, obrstr2);
	strcat(obrstr, obrstr2);
	CDebug::DebugAddText(obrstr);
}

void ObrInt3(int32 n1, int32 n2, int32 n3)
{
	IntToStr(n1, obrstr);
	strcat(obrstr, "  ");
	IntToStr(n2, obrstr2);
	strcat(obrstr, obrstr2);
	strcat(obrstr, "  ");
	IntToStr(n3, obrstr2);
	strcat(obrstr, obrstr2);
	CDebug::DebugAddText(obrstr);
}

void ObrInt4(int32 n1, int32 n2, int32 n3, int32 n4)
{
	IntToStr(n1, obrstr);
	strcat(obrstr, "  ");
	IntToStr(n2, obrstr2);
	strcat(obrstr, obrstr2);
	strcat(obrstr, "  ");
	IntToStr(n3, obrstr2);
	strcat(obrstr, obrstr2);
	strcat(obrstr, "  ");
	IntToStr(n4, obrstr2);
	strcat(obrstr, obrstr2);
	CDebug::DebugAddText(obrstr);
}

void ObrInt5(int32 n1, int32 n2, int32 n3, int32 n4, int32 n5)
{
	IntToStr(n1, obrstr);
	strcat(obrstr, "  ");
	IntToStr(n2, obrstr2);
	strcat(obrstr, obrstr2);
	strcat(obrstr, "  ");
	IntToStr(n3, obrstr2);
	strcat(obrstr, obrstr2);
	strcat(obrstr, "  ");
	IntToStr(n4, obrstr2);
	strcat(obrstr, obrstr2);
	strcat(obrstr, "  ");
	IntToStr(n5, obrstr2);
	strcat(obrstr, obrstr2);
	CDebug::DebugAddText(obrstr);
}

void ObrInt6(int32 n1, int32 n2, int32 n3, int32 n4, int32 n5, int32 n6)
{
	IntToStr(n1, obrstr);
	strcat(obrstr, "  ");
	IntToStr(n2, obrstr2);
	strcat(obrstr, obrstr2);
	strcat(obrstr, "  ");
	IntToStr(n3, obrstr2);
	strcat(obrstr, obrstr2);
	strcat(obrstr, "  ");
	IntToStr(n4, obrstr2);
	strcat(obrstr, obrstr2);
	strcat(obrstr, "  ");
	IntToStr(n5, obrstr2);
	strcat(obrstr, obrstr2);
	strcat(obrstr, "  ");
	IntToStr(n6, obrstr2);
	strcat(obrstr, obrstr2);
	CDebug::DebugAddText(obrstr);
}

void IntToStr(int32 inNum, char *outStr)
{
	bool isNeg = inNum < 0;

	if (isNeg) {
		inNum = -inNum;
		*outStr = '-';
	}

	int16 digits = 1;

	if (inNum > 9) {
		int32 _inNum = inNum;
		do {
			digits++;
			_inNum /= 10;
		} while (_inNum > 9);
	}

	int32 strSize = digits;
	if (isNeg)
		strSize++;

	char *pStr = &outStr[strSize];
	int32 i = 0;
	do {
		*(pStr-- - 1) = (inNum % 10) + '0';
		inNum /= 10;
	} while (++i < strSize);
	outStr[strSize] = '\0';
}