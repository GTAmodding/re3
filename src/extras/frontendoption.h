#pragma once
#include "common.h"

#ifdef CUSTOM_FRONTEND_OPTIONS

// ! There are 2 ways to use CFO,
//		1st; by adding a new option to the array in MenuScreensCustom.cpp and passing attributes/CBs to it
//		2nd; by calling the functions listed at the bottom of this file.

// -- Option types
//
// Static/select:	You allocate the variable, pass it to function and game sets it from user input among the strings given to function,
//					optionally you can add post-change event via ChangeFunc(only called on enter if onlyApplyOnEnter set, or set immediately)
//					You can store the option in an INI file if you pass the key(as a char array) to corresponding parameter.
//
// Dynamic:			Passing variable to function is only needed if you want to store it, otherwise you should do
//					all the operations with ButtonPressFunc, this includes allocating the variable.
//					Left-side text is passed while creating and static, but ofc right-side text is dynamic -
//					you should return it in DrawFunc, which is called on every draw.
//
// Built-in action:	As the name suggests, any action that game has built-in. But as an extra you can set the option text,

// -- Returned via ButtonPressFunc() action param.
#define FEOPTION_ACTION_LEFT 0
#define FEOPTION_ACTION_RIGHT 1
#define FEOPTION_ACTION_SELECT 2
#define FEOPTION_ACTION_FOCUSLOSS 3

// -- Passed via FrontendScreenAdd()
#define FESCREEN_CENTER 0
#define FESCREEN_LEFT_ALIGN 1
#define FESCREEN_RIGHT_ALIGN 2

// -- Callbacks

// pretty much in everything I guess, and optional in all of them
typedef void (*ReturnPrevPageFunc)();

// for static options
typedef void (*ChangeFunc)(int8 before, int8 after); // called after updating the value.
												// only called on enter if onlyApplyOnEnter set, otherwise called on every value change

// for dynamic options
typedef wchar* (*DrawFunc)(bool* disabled, bool userHovering); // you must return a pointer for right text.
															// you can also set *disabled if you want to gray it out.
typedef void (*ButtonPressFunc)(int8 action); // see FEOPTION_ACTIONs above

// -- Internal things
void CustomFrontendOptionsPopulate();
extern int lastOgScreen; // for reloading
extern int numCustomFrontendOptions;
extern int numCustomFrontendScreens;

// -- To be used in ButtonPressFunc / ChangeFunc(this one would be weird):
void ChangeScreen(int screen, int option = 0, bool fadeIn = true);
void GoBack(bool fadeIn = true);

uint8 GetNumberOfMenuOptions(int screen);

// !!! We're now moved to MenuScreensCustom.cpp, which houses an array that keeps all original+custom options.
//		But you can still use the APIs below, and manipulate aScreens while in game.

// Limits:
//		The code relies on that you won't use more then NUM_MENUROWS(18) options on one page, and won't exceed the MENUPAGES of pages.
//		Also congrats if you can make 18 options visible at once.

// Texts:
//		All text parameters accept char[8] GXT key.

// Execute direction:
//		All of the calls below eventually manipulate the aScreens array, so keep in mind to add/replace options in order,
//		i.e. don't set cursor to 8 first and then 3.


// -- Placing the cursor to append/overwrite option
//
// Done via FrontendOptionSetCursor(screen, position, overwrite = false), parameters explained below:
// Screen:			as the name suggests. Also accepts the screen IDs returned from FrontendScreenAdd.
// Option:			if positive, next AddOption call will put the option to there and progress the cursor.
//					if negative, cursor will be placed on bottom-(pos+1), so -1 means the very bottom, -2 means before the back button etc.
// Overwrite:		Use to overwrite the options, not appending a new one. AddOption calls will still progress the cursor.

void FrontendOptionSetCursor(int screen, int8 option, bool overwrite = false);

// var is optional in AddDynamic, enables you to save them in an INI file(also needs passing char array to and saveCat saveKey param. obv), otherwise pass nil/0
void FrontendOptionAddBuiltinAction(const char* gxtKey, int action, int targetMenu = MENUPAGE_NONE, int saveSlot = SAVESLOT_NONE);
void FrontendOptionAddSelect(const char* gxtKey, const char** rightTexts, int8 numRightTexts, int8 *var, bool onlyApplyOnEnter, ChangeFunc changeFunc, const char* saveCat = nil, const char* saveKey = nil, bool disableIfGameLoaded = false);
void FrontendOptionAddDynamic(const char* gxtKey, DrawFunc rightTextDrawFunc, int8 *var, ButtonPressFunc buttonPressFunc, const char* saveCat = nil, const char* saveKey = nil);

uint8 FrontendScreenAdd(const char* gxtKey, eMenuSprites sprite, int prevPage, int columnWidth, int headerHeight, int lineHeight, int8 font, float fontScaleX, float fontScaleY, int8 alignment, bool showLeftRightHelper, ReturnPrevPageFunc returnPrevPageFunc = nil);
#endif
