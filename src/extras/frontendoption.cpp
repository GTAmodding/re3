#include "common.h"

#ifdef CUSTOM_FRONTEND_OPTIONS
#include "frontendoption.h"

int numCustomFrontendOptions = 0;
FrontendOption *customFrontendOptions;

int optionCursor = -1;
eMenuScreen currentMenu;

void ChangeScreen(eMenuScreen screen, int option, bool fadeIn)
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
	int option = !FrontEndMenuManager.m_bGameNotLoaded ?
		aScreens[FrontEndMenuManager.m_nCurrScreen].m_ParentEntry[1] : aScreens[FrontEndMenuManager.m_nCurrScreen].m_ParentEntry[0];

	FrontEndMenuManager.ThingsToDoBeforeGoingBack();

	ChangeScreen((eMenuScreen)screen, option, fadeIn);
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

// Used before reloading in InitialiseChangedLanguageSettings and debugmenu
void
RemoveCustomFrontendOptions()
{
	for (int i = 0; i < MENUPAGES; i++) {
		for (int j = 0; j < NUM_MENUROWS; j++) {
			if (aScreens[i].m_aEntries[j].m_Action == MENUACTION_TRIGGERFUNC) {
				int k;
				for (k = j; k < NUM_MENUROWS-1; k++) {
					memcpy(&aScreens[i].m_aEntries[k], &aScreens[i].m_aEntries[k+1], sizeof(CMenuScreen::CMenuEntry));
				}
				aScreens[i].m_aEntries[k].m_Action = MENUACTION_NOTHING;
				aScreens[i].m_aEntries[k].m_EntryName[0] = '\0';
				j--;
			}
		}
	}
	free(customFrontendOptions);
	numCustomFrontendOptions = 0;
}

int8 RegisterNewOption(int screen)
{
	numCustomFrontendOptions++;
	if (numCustomFrontendOptions == 1)
		customFrontendOptions = (FrontendOption*)malloc(5 * sizeof(FrontendOption));
	else if (numCustomFrontendOptions % 5 == 1)
		customFrontendOptions = (FrontendOption*)realloc(customFrontendOptions, (numCustomFrontendOptions + 4) * sizeof(FrontendOption));

	assert(customFrontendOptions != nil && "Custom frontend options can't be allocated");

	uint8 nth = GetNumberOfMenuOptions(screen);
	if (optionCursor < 0) {
		if (optionCursor == -1) {
			if (!strcmp(aScreens[screen].m_aEntries[nth - 1].m_EntryName, "FEDS_TB") || !strcmp(aScreens[screen].m_aEntries[nth - 1].m_EntryName, "FESZ_CA")) {
				// Move back button one below
				memcpy(&aScreens[screen].m_aEntries[nth], &aScreens[screen].m_aEntries[nth - 1], sizeof(CMenuScreen::CMenuEntry));
				nth--;
			}
		}
	} else {
		if (aScreens[screen].m_aEntries[optionCursor].m_Action != MENUACTION_NOTHING) {
			for (int i = nth - 1; i >= optionCursor; i--) {
				memcpy(&aScreens[screen].m_aEntries[i + 1], &aScreens[screen].m_aEntries[i], sizeof(CMenuScreen::CMenuEntry));
			}
		}
		nth = optionCursor;
		optionCursor++;
	}

	aScreens[screen].m_aEntries[nth].m_Action = MENUACTION_TRIGGERFUNC;
	aScreens[screen].m_aEntries[nth].m_TargetMenu = numCustomFrontendOptions - 1;
	aScreens[screen].m_aEntries[nth].m_EntryName[0] = 1; // just something to fool it
	return nth;
}

void FrontendOptionSetPosition(eMenuScreen screen, int8 option)
{
	currentMenu = screen;
	optionCursor = option;
}

void FrontendOptionAddSelect(const wchar* leftText, const wchar** rightTexts, int8 numRightTexts, int8 *var, bool onlyApplyOnEnter, ChangeFunc changeFunc, ReturnPrevPageFunc returnPrevPageFunc)
{
	int8 screenOptionOrder = RegisterNewOption(currentMenu);

	FrontendOption& option = customFrontendOptions[numCustomFrontendOptions - 1];
	option.screen = currentMenu;
	option.type = FEOPTION_SELECT;
	option.leftText = leftText;
	option.rightTexts = rightTexts;
	option.numRightTexts = numRightTexts;
	option.value = var;
	option.displayedValue = *var;
	option.onlyApplyOnEnter = onlyApplyOnEnter;
	option.changeFunc = changeFunc;
	option.screenOptionOrder = screenOptionOrder;
	option.returnPrevPageFunc = returnPrevPageFunc;
}

void FrontendOptionAddDynamic(const wchar* leftText, DrawFunc drawFunc, ButtonPressFunc buttonPressFunc, ReturnPrevPageFunc returnPrevPageFunc)
{
	int8 screenOptionOrder = RegisterNewOption(currentMenu);

	FrontendOption& option = customFrontendOptions[numCustomFrontendOptions - 1];
	option.screen = currentMenu;
	option.type = FEOPTION_DYNAMIC;
	option.drawFunc = drawFunc;
	option.buttonPressFunc = buttonPressFunc;
	option.leftText = leftText;
	option.onlyApplyOnEnter = false;
	option.screenOptionOrder = screenOptionOrder;
	option.returnPrevPageFunc = returnPrevPageFunc;
}

void FrontendOptionAddRedirect(const wchar* text, eMenuScreen to, int8 selectedOption, bool fadeIn)
{
	int8 screenOptionOrder = RegisterNewOption(currentMenu);

	FrontendOption &option = customFrontendOptions[numCustomFrontendOptions - 1];
	option.screen = currentMenu;
	option.type = FEOPTION_REDIRECT;
	option.to = to;
	option.option = selectedOption;
	option.fadeIn = fadeIn;
	option.leftText = text;
	option.onlyApplyOnEnter = false;
	option.screenOptionOrder = screenOptionOrder;
	option.returnPrevPageFunc = nil;
}

void FrontendOptionAddBackButton(const wchar* text, bool fadeIn)
{
	int8 screenOptionOrder = RegisterNewOption(currentMenu);

	FrontendOption& option = customFrontendOptions[numCustomFrontendOptions - 1];
	option.screen = currentMenu;
	option.type = FEOPTION_GOBACK;
	option.fadeIn = fadeIn;
	option.leftText = text;
	option.onlyApplyOnEnter = false;
	option.screenOptionOrder = screenOptionOrder;
	option.returnPrevPageFunc = nil;
}
#endif