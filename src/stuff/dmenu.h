#pragma once

typedef void (*TriggerFunc)(void);

enum EntryType
{
	MENUEMPTY = 0,
	MENUSUB,
	MENUVAR,

	MENUVAR_INT,
	MENUVAR_FLOAT,
	MENUVAR_CMD,

	MENUSCROLL // dummy
};

struct Menu;

struct DebugMenuEntry
{
	int type;
	const char *name;
	DebugMenuEntry *next;
	RwRect r;
	Menu *menu;

	DebugMenuEntry(const char *name);
};

struct Menu
{
	Menu *parent;
	RwRect r;
	DebugMenuEntry *entries;
	int numEntries;
	int maxNameWidth, maxValWidth;

	DebugMenuEntry *findEntry(const char *entryname);
	void insertEntrySorted(DebugMenuEntry *entry);
	void appendEntry(DebugMenuEntry *entry);

	bool isScrollingUp, isScrollingDown;
	int scrollStart;
	int numVisible;
	RwRect scrollUpR, scrollDownR;
	void scroll(int off);

	int selection;
	DebugMenuEntry *selectedEntry; // updated by update
	void changeSelection(int newsel);
	void changeSelection(DebugMenuEntry *e);

	void update(void);
	void draw(void);
	Menu(void) { memset(this, 0, sizeof(Menu)); }
};
extern Menu toplevel;

struct DebugMenuEntry_Sub : DebugMenuEntry
{
	Menu *submenu;

	DebugMenuEntry_Sub(const char *name, Menu *menu);
};

struct DebugMenuEntry_Var : DebugMenuEntry
{
	int maxvallen;
	int vartype;
	bool wrapAround;

	virtual void processInput(bool mouseOver, bool selected) = 0;
	int getValWidth(void) { return maxvallen; }
	virtual void getValStr(char *str, int len) = 0;
	DebugMenuEntry_Var(const char *name, int type);
};

struct DebugMenuEntry_Int : DebugMenuEntry_Var
{
	virtual void setStrings(const char **strings) = 0;
	virtual int findStringLen(void) = 0;
	DebugMenuEntry_Int(const char *name);
};

#define MUHINTS                         \
	X(Int8, int8, 4, "%4" PRId8)        \
	X(Int16, int16, 6, "%6" PRId16)     \
	X(Int32, int32, 11, "%11" PRId32)   \
	X(Int64, int64, 21, "%21" PRId64)   \
	X(UInt8, uint8, 4, "%4" PRIu8)      \
	X(UInt16, uint16, 6, "%6" PRIu16)   \
	X(UInt32, uint32, 11, "%11" PRIu32) \
	X(UInt64, uint64, 21, "%21" PRIu64)
#define MUHFLOATS                   \
	X(Float32, float, 11, "%11.3f") \
	X(Float64, double, 11, "%11.3lf")

#define X(NAME, TYPE, MAXLEN, FMT)                                                                                                                      \
	struct DebugMenuEntry_##NAME : DebugMenuEntry_Int                                                                                                   \
	{                                                                                                                                                   \
		TYPE *variable;                                                                                                                                 \
		TYPE lowerBound, upperBound;                                                                                                                    \
		TYPE step;                                                                                                                                      \
		TriggerFunc triggerFunc;                                                                                                                        \
		const char *fmt;                                                                                                                                \
		const char **strings;                                                                                                                           \
                                                                                                                                                        \
		void processInput(bool mouseOver, bool selected);                                                                                               \
		void getValStr(char *str, int len);                                                                                                             \
                                                                                                                                                        \
		void setStrings(const char **strings);                                                                                                          \
		int findStringLen(void);                                                                                                                        \
		DebugMenuEntry_##NAME(const char *name, TYPE *ptr, TriggerFunc triggerFunc, TYPE step, TYPE lowerBound, TYPE upperBound, const char **strings); \
	};
MUHINTS
#undef X

#define X(NAME, TYPE, MAXLEN, FMT)                                                                                                \
	struct DebugMenuEntry_##NAME : DebugMenuEntry_Var                                                                             \
	{                                                                                                                             \
		TYPE *variable;                                                                                                           \
		TYPE lowerBound, upperBound;                                                                                              \
		TYPE step;                                                                                                                \
		TriggerFunc triggerFunc;                                                                                                  \
		const char *fmt;                                                                                                          \
                                                                                                                                  \
		void processInput(bool mouseOver, bool selected);                                                                         \
		void getValStr(char *str, int len);                                                                                       \
                                                                                                                                  \
		DebugMenuEntry_##NAME(const char *name, TYPE *ptr, TriggerFunc triggerFunc, TYPE step, TYPE lowerBound, TYPE upperBound); \
	};
MUHFLOATS
#undef X

typedef void (*TriggerFunc)();

struct DebugMenuEntry_Cmd : DebugMenuEntry_Var
{
	TriggerFunc triggerFunc;

	void processInput(bool mouseOver, bool selected);
	void getValStr(char *str, int len);

	DebugMenuEntry_Cmd(const char *name, TriggerFunc triggerFunc);
};

Menu *findMenu(const char *name);
