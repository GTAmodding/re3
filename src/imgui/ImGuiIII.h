#pragma once

RsEventStatus ImGuiEventHandler(RsEvent e, void *param);

class CImGuiIII
{
public:
    static void Initialize();
    static void Shutdown();

    static void Draw();

    static void Process();

private:
    static void NewFrame();
};
