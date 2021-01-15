#include "common.h"

#ifdef CUSTOM_FRONTEND_OPTIONS
#include "Frontend.h"
#include "Text.h"

int lastOgScreen = MENUPAGES; // means no new pages

int numCustomFrontendOptions = 0;
int numCustomFrontendScreens = 0;

int optionCursor = -2;
int currentMenu;
bool optionOverwrite = false;

void ChangeScreen(int screen, int option, bool fadeIn)
{
	FrontEndMenuManager.m_nPrevScreen = FrontEndMenuManager.m_nCurrScreen;
	FrontEndMenuManager.m_nCurrScreen = screen;
	FrontEndMenuManager.m_nCurrOption = option;
	if (fadeIn)
		FrontEndMenuManager.m_nMenuFadeAlpha = 0;
}

void GoBack(bool fadeIn)
{
	int screen = !FrontEndMenuManager.m_bGameNotLoaded ?
		aScreens[FrontEndMenuManager.m_nCurrScreen].m_PreviousPage[1] : aScreens[FrontEndMenuManager.m_nCurrScreen].m_PreviousPage[0];
	int option = FrontEndMenuManager.GetPreviousPageOption();

	FrontEndMenuManager.ThingsToDoBeforeGoingBack();

	ChangeScreen(screen, option, fadeIn);
}

uint8
GetNumberOfMenuOptions(int screen)
{
	uint8 Rows = 0;
	for (int i = 0; i < NUM_MENUROWS; i++) {
		if (aScreens[screen].m_aEntries[i].m_Action == MENUACTION_NOTHING)
			break;

		++Rows;
	}
	return Rows;
}

uint8
GetLastMenuScreen()
{
	int8 page = -1;
	for (int i = 0; i < MENUPAGES; i++) {
		if (strcmp(aScreens[i].m_ScreenName, "") == 0 && aScreens[i].m_PreviousPage[0] == MENUPAGE_NONE)
			break;

		++page;
	}
	return page;
}

int8 RegisterNewScreen(const char *name, int prevPage, ReturnPrevPageFunc returnPrevPageFunc)
{
	if (lastOgScreen == MENUPAGES)
		lastOgScreen = GetLastMenuScreen();

	numCustomFrontendScreens++;
	int id = lastOgScreen + numCustomFrontendScreens;
	assert(id < MENUPAGES && "No room for new custom frontend screens! Increase MENUPAGES");
	strncpy(aScreens[id].m_ScreenName, name, 8);
	aScreens[id].m_PreviousPage[0] = aScreens[id].m_PreviousPage[1] = prevPage;
	aScreens[id].returnPrevPageFunc = returnPrevPageFunc;
	return id;
}

int8 RegisterNewOption()
{
	numCustomFrontendOptions++;
	uint8 numOptions = GetNumberOfMenuOptions(currentMenu);
	uint8 curIdx;
	if (optionCursor < 0) {
		optionCursor = curIdx = numOptions + optionCursor + 1;
	} else 
		curIdx = optionCursor;

	if (!optionOverwrite) {
		if (aScreens[currentMenu].m_aEntries[curIdx].m_Action != MENUACTION_NOTHING) {
			for (int i = numOptions - 1; i >= curIdx; i--) {
				memcpy(&aScreens[currentMenu].m_aEntries[i + 1], &aScreens[currentMenu].m_aEntries[i], sizeof(CMenuScreenCustom::CMenuEntry));
			}
		}
	}
	optionCursor++;
	return curIdx;
}

void FrontendOptionSetCursor(int screen, int8 option, bool overwrite)
{
	currentMenu = screen;
	optionCursor = option;
	optionOverwrite = overwrite;
}

void FrontendOptionAddBuiltinAction(const char* gxtKey, int action, int targetMenu, int saveSlot) {
	int8 screenOptionOrder = RegisterNewOption();

	CMenuScreenCustom::CMenuEntry &option = aScreens[currentMenu].m_aEntries[screenOptionOrder];

	// We can't use custom text on those :shrug:
	switch (action) {
		case MENUACTION_SCREENRES:
			strcpy(option.m_EntryName, "FED_RES");
			break;
		case MENUACTION_AUDIOHW:
			strcpy(option.m_EntryName, "FEA_3DH");
			break;
		default:
			strncpy(option.m_EntryName, gxtKey, 8);
			break;
	}
	option.m_Action = action;
	option.m_SaveSlot = saveSlot;
	option.m_TargetMenu = targetMenu;
}

void FrontendOptionAddSelect(const char* gxtKey, const char** rightTexts, int8 numRightTexts, int8 *var, bool onlyApplyOnEnter, ChangeFunc changeFunc, const char* saveCat, const char* saveKey, bool disableIfGameLoaded)
{	
	int8 screenOptionOrder = RegisterNewOption();

	CMenuScreenCustom::CMenuEntry &option = aScreens[currentMenu].m_aEntries[screenOptionOrder];
	option.m_Action = MENUACTION_CFO_SELECT;
	strncpy(option.m_EntryName, gxtKey, 8);
	option.m_CFOSelect = new CCFOSelect();
	option.m_CFOSelect->rightTexts = (char**)malloc(numRightTexts * sizeof(char*));
	memcpy(option.m_CFOSelect->rightTexts, rightTexts, numRightTexts * sizeof(char*));
	option.m_CFOSelect->numRightTexts = numRightTexts;
	option.m_CFOSelect->value = var;
	if (var) {
		option.m_CFOSelect->displayedValue = *var;
		option.m_CFOSelect->lastSavedValue = *var;
	}
	option.m_CFOSelect->saveCat = saveCat;
	option.m_CFOSelect->save = saveKey;
	option.m_CFOSelect->onlyApplyOnEnter = onlyApplyOnEnter;
	option.m_CFOSelect->changeFunc = changeFunc;
	option.m_CFOSelect->disableIfGameLoaded = disableIfGameLoaded;
}

void FrontendOptionAddDynamic(const char* gxtKey, DrawFunc drawFunc, int8 *var, ButtonPressFunc buttonPressFunc, const char* saveCat, const char* saveKey)
{
	int8 screenOptionOrder = RegisterNewOption();

	CMenuScreenCustom::CMenuEntry &option = aScreens[currentMenu].m_aEntries[screenOptionOrder];
	option.m_Action = MENUACTION_CFO_DYNAMIC;
	strncpy(option.m_EntryName, gxtKey, 8);
	option.m_CFODynamic = new CCFODynamic();
	option.m_CFODynamic->drawFunc = drawFunc;
	option.m_CFODynamic->buttonPressFunc = buttonPressFunc;
	option.m_CFODynamic->value = var;
	option.m_CFODynamic->saveCat = saveCat;
	option.m_CFODynamic->save = saveKey;
}

uint8 FrontendScreenAdd(const char* gxtKey, eMenuSprites sprite, int prevPage, int columnWidth, int headerHeight, int lineHeight,
	int8 font, float fontScaleX, float fontScaleY, int8 alignment, bool showLeftRightHelper, ReturnPrevPageFunc returnPrevPageFunc) {

	uint8 screenOrder = RegisterNewScreen(gxtKey, prevPage, returnPrevPageFunc);

	CCustomScreenLayout *screen = new CCustomScreenLayout();
	aScreens[screenOrder].layout = screen;
	screen->sprite = sprite;
	screen->columnWidth = columnWidth;
	screen->headerHeight = headerHeight;
	screen->lineHeight = lineHeight;
	screen->font = font;
	screen->fontScaleX = fontScaleX;
	screen->fontScaleY = fontScaleY;
	screen->alignment = alignment;

	return screenOrder;
}
#endif
