#pragma once

RsEventStatus ImGuiEventHandler(RsEvent e, void *param);

#define MAX_IMGUI_MENU_ENTRIES 50
#define MAX_IMGUI_CLASS_MENU_ENTRIES 10

typedef void (*VoidCB)();

class CImGuiMenu
{
public:
    char m_acName[128];
    union {
        bool *m_pVar;
        VoidCB cb;
    };
    int8 type;
};

class CImGuiClassMenu
{
public:
    char m_acClassName[64];
    CImGuiMenu m_aEntriesArray[MAX_IMGUI_MENU_ENTRIES];
    uint32 m_nEntriesCount;
};

class CImGuiIII
{
public:
    static void Initialize();
    static void Shutdown();

    static void Draw();

    static void Process();

    static void AddVarInt8(const char *pszClassName, const char *pszName, bool *pVal);
    static void AddVarCmd(const char *pszClassName, const char *pszName, VoidCB cb);

private:
    static void NewFrame();
    static void UpdateMouse();

    static float ms_fMouseX, ms_fMouseY;
    static CImGuiClassMenu ms_aClassMenuArray[MAX_IMGUI_CLASS_MENU_ENTRIES];
    static uint32 ms_nClassMenuCount;
};
