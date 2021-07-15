#include "common.h"
#include "main.h"

#include "Entity.h"
#include "Fluff.h"
#include "Camera.h"
#include "Sprite.h"
#include "Coronas.h"
#include "General.h"
#include "Timer.h"
#include "Clock.h"
#include "Weather.h"
#include "Stats.h"
#include "maths.h"
#include "Frontend.h"

uint8 ScrollCharSet[59][5] = {
	{ 0x00, 0x00, 0x00, 0x00, 0x00 }, // ' '
	{ 0x00, 0x00, 0x1D, 0x00, 0x00 }, // '!'
	{ 0x00, 0x00, 0x00, 0x00, 0x00 }, // '"'
	{ 0x0A, 0x1F, 0x0A, 0x1F, 0x0A }, // '#'
	{ 0x00, 0x09, 0x1F, 0x12, 0x00 }, // '$'
	{ 0x18, 0x18, 0x00, 0x03, 0x03 }, // '%'
	{ 0x00, 0x00, 0x00, 0x00, 0x00 }, // '&'
	{ 0x00, 0x00, 0x00, 0x00, 0x00 }, // '''
	{ 0x01, 0x02, 0x04, 0x08, 0x10 }, // '('
	{ 0x00, 0x00, 0x18, 0x00, 0x00 }, // ')'
	{ 0x15, 0x04, 0x1F, 0x04, 0x15 }, // '*'
	{ 0x00, 0x04, 0x0E, 0x04, 0x00 }, // '+'
	{ 0x00, 0x00, 0x03, 0x00, 0x00 }, // ','
	{ 0x00, 0x04, 0x04, 0x04, 0x00 }, // '-'
	{ 0x00, 0x00, 0x01, 0x00, 0x00 }, // '.'
	{ 0x00, 0x00, 0x00, 0x00, 0x00 }, // '/'
	{ 0x0E, 0x11, 0x11, 0x11, 0x0E }, // '0'
	{ 0x01, 0x09, 0x1F, 0x01, 0x01 }, // '1'
	{ 0x03, 0x15, 0x15, 0x15, 0x09 }, // '2'
	{ 0x11, 0x11, 0x15, 0x15, 0x0A }, // '3'
	{ 0x02, 0x06, 0x0A, 0x1F, 0x02 }, // '4'
	{ 0x1D, 0x15, 0x15, 0x15, 0x12 }, // '5'
	{ 0x0E, 0x15, 0x15, 0x15, 0x12 }, // '6'
	{ 0x18, 0x10, 0x13, 0x14, 0x18 }, // '7'
	{ 0x0A, 0x15, 0x15, 0x15, 0x0A }, // '8'
	{ 0x08, 0x15, 0x15, 0x15, 0x0E }, // '9'
	{ 0x00, 0x00, 0x0A, 0x00, 0x00 }, // ':'
	{ 0x18, 0x18, 0x00, 0x03, 0x03 }, // ';'
	{ 0x04, 0x08, 0x1F, 0x08, 0x04 }, // '<'
	{ 0x00, 0x0A, 0x0A, 0x0A, 0x00 }, // '='
	{ 0x04, 0x02, 0x1F, 0x02, 0x04 }, // '>'
	{ 0x10, 0x10, 0x15, 0x14, 0x1D }, // '?'
	{ 0x00, 0x1C, 0x14, 0x1C, 0x00 }, // '@'
	{ 0x0F, 0x12, 0x12, 0x12, 0x0F }, // 'A'
	{ 0x1F, 0x15, 0x15, 0x15, 0x0A }, // 'B'
	{ 0x0E, 0x11, 0x11, 0x11, 0x0A }, // 'C'
	{ 0x1F, 0x11, 0x11, 0x11, 0x0E }, // 'D'
	{ 0x1F, 0x15, 0x15, 0x11, 0x11 }, // 'E'
	{ 0x1F, 0x14, 0x14, 0x10, 0x10 }, // 'F'
	{ 0x0E, 0x11, 0x15, 0x15, 0x06 }, // 'G'
	{ 0x1F, 0x04, 0x04, 0x04, 0x1F }, // 'H'
	{ 0x11, 0x11, 0x1F, 0x11, 0x11 }, // 'I'
	{ 0x02, 0x01, 0x01, 0x01, 0x1E }, // 'J'
	{ 0x1F, 0x04, 0x0C, 0x12, 0x01 }, // 'K'
	{ 0x1F, 0x01, 0x01, 0x01, 0x01 }, // 'L'
	{ 0x1F, 0x08, 0x06, 0x08, 0x1F }, // 'M'
	{ 0x1F, 0x08, 0x04, 0x02, 0x1F }, // 'N'
	{ 0x0E, 0x11, 0x11, 0x11, 0x0E }, // 'O'
	{ 0x1F, 0x12, 0x12, 0x12, 0x0C }, // 'P'
	{ 0x0C, 0x12, 0x12, 0x13, 0x0D }, // 'Q'
	{ 0x1F, 0x14, 0x14, 0x16, 0x09 }, // 'R'
	{ 0x09, 0x15, 0x15, 0x15, 0x02 }, // 'S'
	{ 0x10, 0x10, 0x1F, 0x10, 0x10 }, // 'T'
	{ 0x1E, 0x01, 0x01, 0x01, 0x1E }, // 'U'
	{ 0x1C, 0x02, 0x01, 0x02, 0x1C }, // 'V'
	{ 0x1E, 0x01, 0x06, 0x01, 0x1E }, // 'W'
	{ 0x11, 0x0A, 0x04, 0x0A, 0x11 }, // 'X'
	{ 0x18, 0x04, 0x03, 0x04, 0x18 }, // 'Y'
	{ 0x11, 0x13, 0x15, 0x19, 0x11 }  // 'Z'
};

// ---------- CMovingThings ----------
enum eScrollBarTypes
{
	SCROLL_BUSINESS,
	SCROLL_TRAFFIC,
	SCROLL_ENTERTAINMENT,
	SCROLL_AIRPORT_DOORS,
	SCROLL_AIRPORT_FRONT,
	SCROLL_STORE,
	SCROLL_USED_CARS
};

CScrollBar    aScrollBars[11];
CTowerClock   aTowerClocks[2];
CDigitalClock aDigitalClocks[3];

CMovingThing CMovingThings::StartCloseList;
CMovingThing CMovingThings::EndCloseList;
int16 CMovingThings::Num;
CMovingThing CMovingThings::aMovingThings[NUMMOVINGTHINGS];
 
void CMovingThings::Init()
{
	StartCloseList.m_pNext = &CMovingThings::EndCloseList;
	StartCloseList.m_pPrev = nil;
	EndCloseList.m_pNext = nil;
	EndCloseList.m_pPrev = &CMovingThings::StartCloseList;
	Num = 0;
	
	// Initialize scroll bars
	 aScrollBars[0].Init(CVector(  228.3f,    -669.0f,     39.0f  ), SCROLL_BUSINESS,       0.0f,     0.5f,     0.5f,   255, 128, 0,   0.3f);
	 aScrollBars[1].Init(CVector(  772.0f,     164.0f,     -9.5f  ), SCROLL_TRAFFIC,        0.0f,     0.5f,     0.25f,  128, 255, 0,   0.3f);
	 aScrollBars[2].Init(CVector(-1089.61f,   -584.224f,   13.246f), SCROLL_AIRPORT_DOORS,  0.0f,    -0.1706f,  0.107f, 255, 0,   0,   0.11f);
	 aScrollBars[3].Init(CVector(-1089.61f,   -602.04602f, 13.246f), SCROLL_AIRPORT_DOORS,  0.0f,    -0.1706f,  0.107f, 0,   255, 0,   0.11f);
	 aScrollBars[4].Init(CVector(-1089.61f,   -619.81702f, 13.246f), SCROLL_AIRPORT_DOORS,  0.0f,    -0.1706f,  0.107f, 255, 128, 0,   0.11f);
	 aScrollBars[5].Init(CVector(-754.578f,   -633.50897f, 18.411f), SCROLL_AIRPORT_FRONT,  0.0f,     0.591f,   0.52f,  100, 100, 255, 0.3f);
	 aScrollBars[6].Init(CVector( -754.578f,  -586.672f,   18.411f), SCROLL_AIRPORT_FRONT,  0.0f,     0.591f,   0.52f,  100, 100, 255, 0.3f);
	 aScrollBars[7].Init(CVector(   85.473f, -1069.512f,   30.5f  ), SCROLL_STORE,          0.625f,  -0.3125f,  0.727f, 100, 100, 255, 0.5f);
	 aScrollBars[8].Init(CVector(   74.823f, -1086.879f,   31.495f), SCROLL_ENTERTAINMENT, -0.2083f,  0.1041f,  0.5f,   255, 255, 128, 0.3f);
	 aScrollBars[9].Init(CVector(  -36.459f, -1031.2371f,  32.534f), SCROLL_ENTERTAINMENT, -0.1442f,  0.0721f,  0.229f, 150, 255, 50,  0.3f);
	aScrollBars[10].Init(CVector( 1208.0f,     -62.208f,   19.157f), SCROLL_USED_CARS,      0.0642f, -0.20365f, 0.229f, 255, 128, 0,   0.3f);

	// Initialize tower clocks
	aTowerClocks[0].Init(CVector(59.4f, -1081.3f, 54.15f), -1.0f,  0.0f, 0, 0, 0, 80.0f, 2.0f);
	aTowerClocks[1].Init(CVector(55.4f, -1083.6f, 54.15f),  0.0f, -1.0f, 0, 0, 0, 80.0f, 2.0f);

	// Initialize digital clocks
	CVector2D sz(3.7f, 2.144f);
	sz.Normalise();
	aDigitalClocks[0].Init(
		CVector(54.485f - sz.x * 0.05f + sz.y * 0.3f, -1081.679f - sz.y * 0.05f - sz.x * 0.3f, 32.803f),
		sz.y, -sz.x, 255, 0, 0, 100.0f, 0.8f
	);
	aDigitalClocks[1].Init(
		CVector(60.564f + sz.x * 0.05f - sz.y * 0.3f, -1083.089f + sz.y * 0.05f + sz.x * 0.3f, 32.803f),
		-sz.y, sz.x, 0, 0, 255, 100.0f, 0.8f
	);
	aDigitalClocks[2].Init(
		CVector(58.145f - sz.y * 0.05f - sz.x * 0.3f, -1079.268f + sz.x * 0.05f - sz.y * 0.3f, 32.803f),
		-sz.x, -sz.y, 0, 255, 0, 100.0f, 0.8f
	);
}

void CMovingThings::Shutdown()
{
	int i;
	for (i = 0; i < ARRAY_SIZE(aScrollBars); ++i)
		aScrollBars[i].SetVisibility(false);
	for (i = 0; i < ARRAY_SIZE(aTowerClocks); ++i)
		aTowerClocks[i].SetVisibility(false);
	for (i = 0; i < ARRAY_SIZE(aDigitalClocks); ++i)
		aDigitalClocks[i].SetVisibility(false);
}

void CMovingThings::Update()
{
	int16 i;
#ifndef SQUEEZE_PERFORMANCE
	const int TIME_SPAN = 64; // frames to process all aMovingThings

	int block = CTimer::GetFrameCounter() % TIME_SPAN;

	for (i = (block * NUMMOVINGTHINGS) / TIME_SPAN; i < ((block + 1) * NUMMOVINGTHINGS) / TIME_SPAN; i++) {
		if (aMovingThings[i].m_nHidden == 1)
			aMovingThings[i].Update();
	}

	for (i = 0; i < CMovingThings::Num; i++) {
		if (aMovingThings[i].m_nHidden == 0)
			aMovingThings[i].Update();
	}
#endif

	for (i = 0; i < ARRAY_SIZE(aScrollBars); ++i)
	{
		if (aScrollBars[i].IsVisible() || (CTimer::GetFrameCounter() + i) % 8 == 0)
			aScrollBars[i].Update();
	}
	for (i = 0; i < ARRAY_SIZE(aTowerClocks); ++i)
	{
		if (aTowerClocks[i].IsVisible() || (CTimer::GetFrameCounter() + i) % 8 == 0)
			aTowerClocks[i].Update();
	}
	for (i = 0; i < ARRAY_SIZE(aDigitalClocks); ++i)
	{
		if (aDigitalClocks[i].IsVisible() || (CTimer::GetFrameCounter() + i) % 8 == 0)
			aDigitalClocks[i].Update();
	}
}

void CMovingThings::Render()
{
	int i;
	PUSH_RENDERGROUP("CMovingThings::Render");
	for (i = 0; i < ARRAY_SIZE(aScrollBars); ++i)
	{
		if (aScrollBars[i].IsVisible())
			aScrollBars[i].Render();
	}
	for (i = 0; i < ARRAY_SIZE(aTowerClocks); ++i)
	{
		if (aTowerClocks[i].IsVisible())
			aTowerClocks[i].Render();
	}
	for (i = 0; i < ARRAY_SIZE(aDigitalClocks); ++i)
	{
		if (aDigitalClocks[i].IsVisible())
			aDigitalClocks[i].Render();
	}
	POP_RENDERGROUP();
}

// ---------- CMovingThing ----------
void CMovingThing::Update()
{
	m_pEntity->GetMatrix().UpdateRW();
	m_pEntity->UpdateRwFrame();
	
	if (SQR(m_pEntity->GetPosition().x - TheCamera.GetPosition().x) + SQR(m_pEntity->GetPosition().y - TheCamera.GetPosition().y) < 40000.0f) {
		if (m_nHidden == 1) {
			AddToList(&CMovingThings::StartCloseList);
			m_nHidden = 0;
		}
	} else {
		if (m_nHidden == 0) {
			RemoveFromList();
			m_nHidden = 1;
		}
	}
}

void CMovingThing::AddToList(CMovingThing *pThing)
{
	m_pNext = pThing->m_pNext;
	m_pPrev = pThing;
	pThing->m_pNext = this;
	m_pNext->m_pPrev = this;
}

void CMovingThing::RemoveFromList()
{
	m_pNext->m_pPrev = m_pPrev;
	m_pPrev->m_pNext = m_pNext;
}

int16 CMovingThing::SizeList()
{
	CMovingThing *next = m_pNext;
	int16 count = 0;

	while (next != nil) {
		next = next->m_pNext;
		count++;
	}

	return count;
}

// ---------- Find message functions ----------
const char* FindTunnelMessage()
{
	if (CStats::CommercialPassed)
		return "LIBERTY TUNNEL HAS BEEN OPENED TO ALL TRAFFIC . . .   ";

	if (CStats::IndustrialPassed)
		return "FIRST PHASE LIBERTY TUNNEL HAS BEEN COMPLETED . . .   ";

	return "FIRST PHASE LIBERTY TUNNEL ABOUT TO BE COMPLETED . . .   ";
}

const char* FindBridgeMessage()
{
	if (CStats::CommercialPassed)
		return "STAUNTON LIFT BRIDGE IS OPERATIONAL AGAIN    ";

	if (CStats::IndustrialPassed)
		return "LONG DELAYS BEHIND US AS CALLAHAN BRIDGE IS FIXED . . .   STAUNTON LIFT BRIDGE STUCK OPEN   ";

	return "CHAOS AS CALLAHAN BRIDGE IS UNDER REPAIR. . .   ";
}

char String_Time[] = "THE TIME IS 12:34    ";
const char* FindTimeMessage()
{
	String_Time[12] = '0' + CClock::GetHours() / 10;
	String_Time[13] = '0' + CClock::GetHours() % 10;
	String_Time[15] = '0' + CClock::GetMinutes() / 10;
	String_Time[16] = '0' + CClock::GetMinutes() % 10;
	return String_Time;
}

char String_DigitalClock[] = "12:34";
const char* FindDigitalClockMessage()
{
	if (((CTimer::GetTimeInMilliseconds() >> 10) & 7) < 6)
	{
		String_DigitalClock[0] = '0' + CClock::GetHours() / 10;
		String_DigitalClock[1] = '0' + CClock::GetHours() % 10;
		String_DigitalClock[2] = CTimer::GetTimeInMilliseconds() & 0x200 ? ':' : ' ';
		String_DigitalClock[3] = '0' + CClock::GetMinutes() / 10;
		String_DigitalClock[4] = '0' + CClock::GetMinutes() % 10;
	}
	else
	{
		// they didn't use rad2deg here because of 3.14
		int temperature = 13.0f - 6.0f * Cos((CClock::GetMinutes() + 60.0f * CClock::GetHours()) / (4.0f * 180.0f / 3.14f) - 1.0f);
		String_DigitalClock[0] = '0' + temperature / 10;
		if (String_DigitalClock[0] == '0')
			String_DigitalClock[0] = ' ';
		String_DigitalClock[1] = '0' + temperature % 10;
		String_DigitalClock[2] = ' ';
		String_DigitalClock[3] = '@';
		String_DigitalClock[4] = 'C';
	}
	return String_DigitalClock;
}

// ---------- CScrollBar ----------
void CScrollBar::Init(CVector position, uint8 type, float sizeX, float sizeY, float sizeZ, uint8 red, uint8 green, uint8 blue, float scale)
{
	for (int i = 0; i < ARRAY_SIZE(m_MessageBar); ++i)
		m_MessageBar[i] = 0;

	m_pMessage = ". ";
	m_MessageCurrentChar = 0;
	m_MessageLength = 2;

	m_Counter  = 0;
	m_bVisible = false;
	m_Position = position;
	m_Type     = type;
	m_Size.x   = sizeX;
	m_Size.y   = sizeY;
	m_Size.z   = sizeZ;
	m_uRed     = red;
	m_uGreen   = green;
	m_uBlue    = blue;
	m_fScale   = scale;
}

void CScrollBar::Update()
{
	float distanceFromCamera = (TheCamera.GetPosition() - m_Position).Magnitude();
	if (distanceFromCamera > 100.0f)
	{
		m_bVisible = false;
		return;
	}

	m_bVisible = true;

	if (distanceFromCamera < 75.0f)
		m_fIntensity = 1.0f;
	else
		m_fIntensity = 1.0f - 4.0f * (distanceFromCamera - 75.0f) / 100.0f;

	m_Counter = (m_Counter + 1) % 8;
		
	// if message is fully printed, load up the next one
	if (m_Counter == 0 && ++m_MessageCurrentChar >= m_MessageLength)
	{
		const char* previousMessage = m_pMessage;
		switch (m_Type)
		{
		case SCROLL_BUSINESS:
			while (previousMessage == m_pMessage)
			{
				switch (CGeneral::GetRandomNumber() % 7)
				{
				case 0:
					m_pMessage = "SHARES  UYE<10% DWD<20% NDWE>22% . . .   ";
					break;
				case 1:
					m_pMessage = "CRIME WAVE HITS LIBERTY CITY . . .   ";
					break;
				case 2:
					m_pMessage = "SHARES  OBR>29% MADD<76% LEZ<11% ADAMSKI>53% AAG>110%. . .   ";
					break;
				case 3:
					m_pMessage = FindTunnelMessage();
					break;
				case 4:
					m_pMessage = FindBridgeMessage();
					break;
				case 5:
					m_pMessage = FindTimeMessage();
					break;
				case 6:
					if (CMenuManager::m_PrefsLanguage == CMenuManager::LANGUAGE_FRENCH || CMenuManager::m_PrefsLanguage == CMenuManager::LANGUAGE_GERMAN)
						m_pMessage = FindTimeMessage();
					else
						m_pMessage = "WWW.GRANDTHEFTAUTO3.COM    ";
					break;
				}
			}
			break;
		case SCROLL_TRAFFIC:
			while (previousMessage == m_pMessage)
			{
				switch (CGeneral::GetRandomNumber() % 8)
				{
				case 0:
					m_pMessage = "DRIVE CAREFULLY . . .   ";
					break;
				case 1:
					m_pMessage = "RECENT WAVE OF CARJACKINGS. KEEP YOUR DOORS LOCKED !!!  ";
					break;
				case 2:
					m_pMessage = "CHECK YOUR SPEED . . .  ";
					break;
				case 3:
					m_pMessage = "KEEP YOUR EYES ON THE ROAD AND NOT ON THIS SIGN  ";
					break;
				case 4:
					if (CWeather::Foggyness > 0.5f)
						m_pMessage = "POOR VISIBILITY !   ";
					else if (CWeather::WetRoads > 0.5f)
						m_pMessage = "ROADS ARE SLIPPERY !   ";
					else
						m_pMessage = "ENJOY YOUR TRIP   ";
					break;
				case 5:
					m_pMessage = FindTunnelMessage();
					break;
				case 6:
					m_pMessage = FindBridgeMessage();
					break;
				case 7:
					m_pMessage = FindTimeMessage();
					break;
				}
			}
			break;
		case SCROLL_ENTERTAINMENT:
			while (previousMessage == m_pMessage)
			{
				switch (CGeneral::GetRandomNumber() % 12)
				{
				case 0:
					m_pMessage = " )69TH STREET) STILL HOLDS TOP POSITION THIS MONTH AT THE BOX-OFFICE WITH )MY FAIR LADYBOY) JUST CREEPING UP BEHIND.   ";
					break;
				case 1:
					m_pMessage = " TALKING OF )FANNIE). THERE IS STILL TIME TO CATCH THIS LOVELY FAMILY MUSICAL, ABOUT THE ORPHAN WHO IS SO EASILY TAKEN IN BY ANY MAN WITH LOADS OF MONEY.  ";
					break;
				case 2:
					m_pMessage = " DO NOT MISS  )GTA3, THE MUSICAL) . . . ";
					break;
				case 3:
					m_pMessage =
						" STILL RUNNING ARE )RATS) AND )GUYS AND DOGS), BETWEEN THEN THEY SHOULD HAVE THE LEGS TO LAST TILL THE AND OF THE YEAR. . . "
						" ALSO FOR FOUR LEGGED FANS, THE STAGE VERSION OF THE GRITTY REALISTIC )SATERDAY NIGHT BEAVER) OPENED LAST WEEKEND,"
						" AND I FOR ONE CERTAINLY ENJOYED THAT.  ";
					break;
				case 4:
					m_pMessage =
						" NOW SHOWING STATE-WIDE, ARNOLD STEELONE, HOLLYWOODS BEST LIVING SPECIAL EFFECT, APPEARS AGAIN AS A HALF_MAN,"
						" HALF ANDROID IN THE HALF-BAKED ROMP, )TOP DOWN CITY). AN HOMAGE TO HIS EARLIER TWO MULTI_MILLION MAKING MOVIES,"
						" IN WHICH HE PLAYED TWO-DEE, AN OUT OF CONTROL MONSTER, INTENT ON CORRUPTING CIVILISATION!   ";
					break;
				case 5:
					m_pMessage =
						" ALSO APPEARING THIS WEEK )HALF-COCKED) SEES CHUCK SCHWARTZ UP TO HIS USUAL NONSENSE AS HE TAKES ON HALF OF LIBERTY CITY"
						" IN AN ATTEMPT TO SAVE HIS CROSS-DRESSING LADY-BOY SIDEKICK, )MISS PING-PONG), FROM A GANG OF RUTHLESS COSMETIC SURGEONS.    ";
					break;
				case 6:
					m_pMessage =
						" STILL SHOWING: )SOLDIERS OF MISFORTUNE), ATTROCIOUS ACTING WHICH SEES BOYZ 2 GIRLZ) TRANSITION FROM THE CHARTS TO THE BIG SCREEN,"
						" AT LEAST THEY ALL DIE AT THE END. . . ";
					break;
				case 7:
					m_pMessage =
						" )BADFELLAS) IS STILL GOING STRONG WITH CROWDS ALMOST BEING PUSHED INTO CINEMAS TO SEE THIS ONE."
						" ANOTHER ONE WORTH LOOKING INTO IS )THE TUNNEL).    ";
					break;
				case 8:
					m_pMessage = FindTunnelMessage();
					break;
				case 9:
					m_pMessage = FindBridgeMessage();
					break;
				case 10:
					m_pMessage = FindTimeMessage();
					break;
				case 11:
					m_pMessage = "WWW.ROCKSTARGAMES.COM    ";
					break;
				}
			}
			break;
		case SCROLL_AIRPORT_DOORS:
			while (previousMessage == m_pMessage)
			{
				switch (CGeneral::GetRandomNumber() % 4)
				{
				case 0:
					m_pMessage = "WELCOME TO LIBERTY CITY . . .   ";
					break;
				case 1:
					m_pMessage = "PLEASE HAVE YOUR PASSPORT READY . . .   ";
					break;
				case 2:
					m_pMessage = "PLACE KEYS, FIREARMS, CHANGE AND OTHER METAL OBJECTS ON THE TRAY PLEASE . . .  ";
					break;
				case 3:
					m_pMessage = FindTimeMessage();
					break;
				}
			}
			break;
		case SCROLL_AIRPORT_FRONT:
			while (previousMessage == m_pMessage)
			{
				switch (CGeneral::GetRandomNumber() % 4)
				{
				case 0:
					m_pMessage = "WELCOME TO FRANCIS INTERNATIONAL AIRPORT . . .  ";
					break;
				case 1:
					m_pMessage = "PLEASE DO NOT LEAVE LUGGAGE UNATTENDED . . .  ";
					break;
				case 2:
					m_pMessage = "FOLLOW 1 FOR LONG AND SHORT TERM PARKING  ";
					break;
				case 3:
					m_pMessage = FindTimeMessage();
					break;
				}
			}
			break;
		case SCROLL_STORE:
			while (previousMessage == m_pMessage)
			{
				switch (CGeneral::GetRandomNumber() % 10)
				{
				case 0:
					m_pMessage = "WWW.ROCKSTARGAMES.COM    ";
					break;
				case 1:
					m_pMessage = "GTA3   OUT NOW . . . ";
					break;
				case 2:
					m_pMessage = "OUR STUFF IS  CHEAP CHEAP CHEAP      ";
					break;
				case 3:
					m_pMessage = "BUY 12 CDS GET ONE FREE . . . ";
					break;
				case 4:
					m_pMessage = "APPEARING IN SHOP SOON, )THE BLOODY CHOPPERS), WITH THEIR NEW ALBUM, )IS THAT MY DAUGHTER?) ";
					break;
				case 5:
					m_pMessage = "THIS MONTH IS OUR CRAZY CLEAROUT MONTH, EVERYTHING MUST GO, CDS, DVDS, STAFF, EVEN OUR CARPETS!    ";
					break;
				case 6:
					m_pMessage =
						"OUT THIS WEEK: THE THEME TUNE TO )BOYS TO GIRLS) FIRST MOVIE )SOLDIERS OF MISFORTUNE), "
						"THE SINGLE )LET ME IN YOU)RE BODY-BAG) IS TAKEN FROM THE SOUNDTRACK ALBUM, )BOOT CAMP BOYS). "
						"ALSO INCLUDES THE SMASH SINGLE, )PRAY IT GOES OK).    ";
					break;
				case 7:
					m_pMessage =
						"ALBUMS OUT THIS WEEK: MARYDANCING, )MUTHA O) CHRIST), FEATURING THE SINGLE )WASH HIM OFF), "
						"ALSO CRAIG GRAYS) DEBUT, )FADE AWAY), INCLUDES THE SINGLE OF THE SAME NAME. . . ";
					break;
				case 8:
					m_pMessage =
						"ON THE FILM FRONT, A NELY COMPILED COMPILATION OF ARNOLD STEELONES GREATEST MOVIES ON DVD. "
						"THE PACK INCLUDES THE EARLY )BY-CEP), THE CULT CLASSIC )FUTURE ANNHILATOR), AND THE HILARIOUS CROSS-DRESSING COMEDY )SISTERS). "
						"ONE FOR ALL THE FAMILY. . . ";
					break;
				case 9:
					m_pMessage = FindTimeMessage();
					break;
				}
			}
			break;
		case SCROLL_USED_CARS:
			while (previousMessage == m_pMessage)
			{
				switch (CGeneral::GetRandomNumber() % 11)
				{
				case 0:
					m_pMessage = "QUICK, TAKE A LOOK AT OUR CURRENT STOCK )CAUSE THESE AUTOS ARE MOVIN) FAST . . .  ";
					break;
				case 1:
					m_pMessage = "THAT)S RIGHT, HERE AT )CAPITAL AUTO SALES) OUR VEHICLES ARE SO GOOD THAT THEY PRACTICALLY DRIVE THEMSELVES OFF OUR LOT . . .   ";
					break;
				case 2:
					m_pMessage = "EASY CREDIT ON ALL CARS . . .   ";
					break;
				case 3:
					m_pMessage = "FEEL LIKE A STUD IN ONE OF OUR STALLIONS OR TEST-DRIVE OUR BANSHEE, IT)S A REAL STEAL!!!   ";
					break;
				case 4:
					m_pMessage = "TRY OUR HARDY PERENNIAL, IT)LL LAST YOU THE WHOLE YEAR. OUR BOBCATS AIN)T NO PUSSIES EITHER!!!   ";
					break;
				case 5:
					m_pMessage = "IF IT)S A GUARANTEE YOU'RE AFTER, GO SOMEWHERE ELSE, )CAPITAL) CARS ARE THAT GOOD THEY DON)T NEED GUARANTEES!!!   ";
					break;
				case 6:
					m_pMessage = "TOP DOLLAR OFFERED FOR YOUR OLD WHEELS, NOT YOUR CAR, JUST IT)S WHEELS. . .   ";
					break;
				case 7:
					m_pMessage = "THAT)S RIGHT WE)RE CAR SILLY. TEST DRIVE ANY CAR, YOU WON)T WANT TO BRING IT BACK!!!   ";
					break;
				case 8:
					m_pMessage = "FREE FLUFFY DICE WITH ALL PURCHASES. . .";
					break;
				case 9:
					if (CMenuManager::m_PrefsLanguage == CMenuManager::LANGUAGE_FRENCH || CMenuManager::m_PrefsLanguage == CMenuManager::LANGUAGE_GERMAN)
						m_pMessage = "QUICK, TAKE A LOOK AT OUR CURRENT STOCK )CAUSE THESE AUTOS ARE MOVIN) FAST . . .  ";
					else
						m_pMessage = "HTTP:((ROCKSTARGAMES.COM(GRANDTHEFTAUTO3(CAPITALAUTOS    ";
					break;
				case 10:
					m_pMessage = FindTimeMessage();
					break;
				}
			}
			break;
		}

		m_MessageLength = (uint32)strlen(m_pMessage);
		m_MessageCurrentChar = 0;
	}

	// Scroll
	for (int i = 0; i < ARRAY_SIZE(m_MessageBar)-1; i++)
		m_MessageBar[i] = m_MessageBar[i + 1];
	m_MessageBar[ARRAY_SIZE(m_MessageBar)-1] = m_Counter < 5 ? ScrollCharSet[m_pMessage[m_MessageCurrentChar] - ' '][m_Counter] : 0;

	// Introduce some random displaying glitches; signs aren't supposed to be perfect :P
	switch (CGeneral::GetRandomNumber() & 0xFF)
	{
	case 0x0D: m_MessageBar[ARRAY_SIZE(m_MessageBar)-1] = 0;                 break;
	case 0xE3: m_MessageBar[ARRAY_SIZE(m_MessageBar)-1] = 0xE3;              break;
	case 0x64: m_MessageBar[ARRAY_SIZE(m_MessageBar)-1] = ~m_MessageBar[ARRAY_SIZE(m_MessageBar)-1]; break;
	}
}

void CScrollBar::Render()
{
	if (!TheCamera.IsSphereVisible(m_Position, 2.0f * 20.0f * (ABS(m_Size.x) + ABS(m_Size.y))))
		return;

	CSprite::InitSpriteBuffer();

	// Calculate intensity of colours
	uint8 r = m_fIntensity * m_uRed;
	uint8 g = m_fIntensity * m_uGreen;
	uint8 b = m_fIntensity * m_uBlue;

	// Set render states
	RwRenderStateSet(rwRENDERSTATEZWRITEENABLE,      (void*)FALSE);
	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)TRUE);
	RwRenderStateSet(rwRENDERSTATESRCBLEND,          (void*)rwBLENDONE);
	RwRenderStateSet(rwRENDERSTATEDESTBLEND,         (void*)rwBLENDONE);
	RwRenderStateSet(rwRENDERSTATETEXTUREFILTER,     (void*)rwFILTERLINEAR);
	RwRenderStateSet(rwRENDERSTATETEXTURERASTER,     RwTextureGetRaster(gpCoronaTexture[0]));
	RwRenderStateSet(rwRENDERSTATEZTESTENABLE,       (void*)TRUE);

	CVector coronaCoord, screenCoord;
	float screenW, screenH;
	for (int i = 1; i < ARRAY_SIZE(m_MessageBar); ++i)
	{
		for (int j = 0; j < 5; ++j)
		{
			coronaCoord.x = m_Position.x + m_Size.x * i;
			coronaCoord.y = m_Position.y + m_Size.y * i;
			coronaCoord.z = m_Position.z + m_Size.z * j;

			// Render main coronas
			if (m_MessageBar[i] & (1 << j))
			{
				if (CSprite::CalcScreenCoors(coronaCoord, &screenCoord, &screenW, &screenH, true))
				{
					CSprite::RenderBufferedOneXLUSprite(
						screenCoord.x, screenCoord.y, screenCoord.z,
						screenW * m_fScale, screenH * m_fScale,
						r, g, b,
						255, 1.0f / screenCoord.z, 255);
				}
			}
			// Render smaller and faded coronas for a trailing effect
			else if (m_MessageBar[i - 1] & (1 << j))
			{
				if (CSprite::CalcScreenCoors(coronaCoord, &screenCoord, &screenW, &screenH, true))
				{
					CSprite::RenderBufferedOneXLUSprite(
						screenCoord.x, screenCoord.y, screenCoord.z,
						screenW * m_fScale * 0.8f,
						screenH * m_fScale * 0.8f,
						r / 2,
						g / 2,
						b / 2,
						255, 1.0f / screenCoord.z, 255);
				}
			}
		}
	}

	CSprite::FlushSpriteBuffer();
}

// ---------- CTowerClock ----------
void CTowerClock::Init(CVector position, float sizeX, float sizeY, uint8 red, uint8 green, uint8 blue, float drawDistance, float scale)
{
	m_bVisible      = false;
	m_Position      = position;
	m_Size.x        = sizeX;
	m_Size.y        = sizeY;
	m_Size.z        = 0.0f;
	m_uRed          = red;
	m_uGreen        = green;
	m_uBlue         = blue;
	m_fDrawDistance = drawDistance;
	m_fScale        = scale;
}

void CTowerClock::Update()
{
	float distanceFromCamera = (TheCamera.GetPosition() - m_Position).Magnitude();
	if (distanceFromCamera < m_fDrawDistance)
	{
		m_bVisible = true;
		if (distanceFromCamera < 0.75f * m_fDrawDistance)
			m_fIntensity = 1.0f;
		else
			m_fIntensity = 1.0f - (distanceFromCamera - 0.75f * m_fDrawDistance) * 4.0f / m_fDrawDistance;
	}
	else
		m_bVisible = false;
}

RwIm3DVertex TempV[4];
void CTowerClock::Render()
{
	if (TheCamera.IsSphereVisible(m_Position, m_fScale))
	{
		// Calculate angle for each clock index
		float angleHour   = 2.0f * (float)PI * (CClock::GetMinutes() + 60.0f * CClock::GetHours())   / 720.0f;
		float angleMinute = 2.0f * (float)PI * (CClock::GetSeconds() + 60.0f * CClock::GetMinutes()) / 3600.0f;

		// Prepare render states
		RwRenderStateSet(rwRENDERSTATEZWRITEENABLE,      (void*)TRUE);
		RwRenderStateSet(rwRENDERSTATEZTESTENABLE,       (void*)TRUE);
		RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)TRUE);
		RwRenderStateSet(rwRENDERSTATESRCBLEND,          (void*)rwBLENDSRCALPHA);
		RwRenderStateSet(rwRENDERSTATEDESTBLEND,         (void*)rwBLENDINVSRCALPHA);
		RwRenderStateSet(rwRENDERSTATETEXTUREFILTER,     (void*)rwFILTERLINEAR);
		RwRenderStateSet(rwRENDERSTATETEXTURERASTER,     nil);

		// Set vertices colors
		RwIm3DVertexSetRGBA(&TempV[0], m_uRed, m_uGreen, m_uBlue, (uint8)(m_fIntensity * 255.0f));
		RwIm3DVertexSetRGBA(&TempV[1], m_uRed, m_uGreen, m_uBlue, (uint8)(m_fIntensity * 255.0f));
		RwIm3DVertexSetRGBA(&TempV[2], m_uRed, m_uGreen, m_uBlue, (uint8)(m_fIntensity * 255.0f));
		RwIm3DVertexSetRGBA(&TempV[3], m_uRed, m_uGreen, m_uBlue, (uint8)(m_fIntensity * 255.0f));

		// Set vertices position
		RwIm3DVertexSetPos(&TempV[0], m_Position.x, m_Position.y, m_Position.z);
		RwIm3DVertexSetPos(
			&TempV[1],
			m_Position.x + Sin(angleMinute) * m_fScale * m_Size.x,
			m_Position.y + Sin(angleMinute) * m_fScale * m_Size.y,
			m_Position.z + Cos(angleMinute) * m_fScale
		);
		RwIm3DVertexSetPos(&TempV[2], m_Position.x, m_Position.y, m_Position.z);
		RwIm3DVertexSetPos(
			&TempV[3],
			m_Position.x + Sin(angleHour) * 0.75f * m_fScale * m_Size.x,
			m_Position.y + Sin(angleHour) * 0.75f * m_fScale * m_Size.y,
			m_Position.z + Cos(angleHour) * 0.75f * m_fScale
		);

		LittleTest();

		// Draw lines
		if (RwIm3DTransform(TempV, 4, nil, 0))
		{
			RwIm3DRenderLine(0, 1);
			RwIm3DRenderLine(2, 3);
			RwIm3DEnd();
		}
	}
}

// ---------- CDigitalClock ----------
void CDigitalClock::Init(CVector position, float sizeX, float sizeY, uint8 red, uint8 green, uint8 blue, float drawDistance, float scale)
{
	m_bVisible      = false;
	m_Position      = position;
	m_Size.x        = sizeX;
	m_Size.y        = sizeY;
	m_Size.z        = 0.0f;
	m_uRed          = red;
	m_uGreen        = green;
	m_uBlue         = blue;
	m_fDrawDistance = drawDistance;
	m_fScale        = scale;
}

void CDigitalClock::Update()
{
	float distanceFromCamera = (TheCamera.GetPosition() - m_Position).Magnitude();
	if (distanceFromCamera < m_fDrawDistance)
	{
		m_bVisible = true;
		if (distanceFromCamera < 0.75f * m_fDrawDistance)
			m_fIntensity = 1.0f;
		else
			m_fIntensity = 1.0f - (distanceFromCamera - 0.75f * m_fDrawDistance) * 4.0f / m_fDrawDistance;
	}
	else
		m_bVisible = false;
}

void CDigitalClock::Render()
{
	if (TheCamera.IsSphereVisible(m_Position, 5.0f * m_fScale))
	{
		CSprite::InitSpriteBuffer();

		// Simulate flicker
		float currentIntensity = m_fIntensity * CGeneral::GetRandomNumberInRange(0x300, 0x400) / 1024.0f;

		uint8 r = currentIntensity * m_uRed;
		uint8 g = currentIntensity * m_uGreen;
		uint8 b = currentIntensity * m_uBlue;

		// Set render states
		RwRenderStateSet(rwRENDERSTATEZWRITEENABLE,      (void*)FALSE);
		RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)TRUE);
		RwRenderStateSet(rwRENDERSTATESRCBLEND,          (void*)rwBLENDONE);
		RwRenderStateSet(rwRENDERSTATEDESTBLEND,         (void*)rwBLENDONE);
		RwRenderStateSet(rwRENDERSTATETEXTUREFILTER,     (void*)rwFILTERLINEAR);
		RwRenderStateSet(rwRENDERSTATETEXTURERASTER,     RwTextureGetRaster(gpCoronaTexture[0]));
		RwRenderStateSet(rwRENDERSTATEZTESTENABLE,       (void*)TRUE);

		const char* clockMessage = FindDigitalClockMessage();

		CVector coronaCoord, screenCoord;
		float screenW, screenH;
		for (int c = 0; c < 5; ++c) // for each char to be displayed
		{
			for (int i = 0; i < 5; ++i) // for each column of coronas
			{
				for (int j = 0; j < 5; ++j) // for each row of coronas
				{
					if (ScrollCharSet[clockMessage[c] - ' '][i] & (1 << j))
					{
						coronaCoord.x = m_Position.x + (8 * c + i) * m_Size.x * m_fScale / 8.0f;
						coronaCoord.y = m_Position.y + (8 * c + i) * m_Size.y * m_fScale / 8.0f;
						coronaCoord.z = m_Position.z + j * m_fScale / 8.0f;

						if (CSprite::CalcScreenCoors(coronaCoord, &screenCoord, &screenW, &screenH, true))
						{
							CSprite::RenderBufferedOneXLUSprite(
								screenCoord.x, screenCoord.y, screenCoord.z,
								screenW * m_fScale * 0.12f,
								screenW * m_fScale * 0.12f,
								r, g, b,
								255,
								1.0f / screenCoord.z,
								255);
						}
					}
				}
			}
		}

		CSprite::FlushSpriteBuffer();
	}
}
