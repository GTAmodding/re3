#include "common.h"
#include <skeleton.h>
#include <assert.h>
#include "Pad.h"
#include "Timer.h"
#include "Sprite2d.h"
#include "main.h"
#include "ControllerConfig.h"

#include "imgui/imgui.h"
#include "ImGuiIII.h"

#ifdef IMGUI
static RwTexture *g_FontTexture;
static RwIm2DVertex *g_vertbuf;
static int g_vertbufSize;

void RenderDrawLists(ImDrawData *draw_data)
{
	ImGuiIO &io = ImGui::GetIO();

	// minimized
	if (io.DisplaySize.x <= 0.0f || io.DisplaySize.y <= 0.0f)
		return;

	if (g_vertbuf == nil || g_vertbufSize < draw_data->TotalVtxCount)
	{
		if (g_vertbuf)
		{
			rwFree(g_vertbuf);
			g_vertbuf = nil;
		}
		g_vertbufSize = draw_data->TotalVtxCount + 5000;
		g_vertbuf = rwNewT(RwIm2DVertex, g_vertbufSize, 0);
	}

	float xoff = 0.0f;
	float yoff = 0.0f;
#ifdef RWHALFPIXEL
	xoff = -0.5;
	yoff = 0.5;
#endif

	rw::Camera *cam = (rw::Camera *)rw::engine->currentCamera;
	RwIm2DVertex *vtx_dst = g_vertbuf;
	float recipZ = 1.0f / cam->nearPlane;
	for (int n = 0; n < draw_data->CmdListsCount; n++)
	{
		const ImDrawList *cmd_list = draw_data->CmdLists[n];
		const ImDrawVert *vtx_src = cmd_list->VtxBuffer.Data;
		for (int i = 0; i < cmd_list->VtxBuffer.Size; i++)
		{
			vtx_dst[i].setScreenX(vtx_src[i].pos.x + xoff);
			vtx_dst[i].setScreenY(vtx_src[i].pos.y + yoff);
			vtx_dst[i].setScreenZ(rw::im2d::GetNearZ());
			vtx_dst[i].setCameraZ(cam->nearPlane);
			vtx_dst[i].setRecipCameraZ(recipZ);
			vtx_dst[i].setColor(vtx_src[i].col & 0xFF, vtx_src[i].col >> 8 & 0xFF, vtx_src[i].col >> 16 & 0xFF, vtx_src[i].col >> 24 & 0xFF);
			vtx_dst[i].setU(vtx_src[i].uv.x, recipZ);
			vtx_dst[i].setV(vtx_src[i].uv.y, recipZ);
		}
		vtx_dst += cmd_list->VtxBuffer.Size;
	}

	rw::SetRenderState(rw::VERTEXALPHA, 1);
	rw::SetRenderState(rw::SRCBLEND, rw::BLENDSRCALPHA);
	rw::SetRenderState(rw::DESTBLEND, rw::BLENDINVSRCALPHA);
	rw::SetRenderState(rw::ZTESTENABLE, 0);

	int vtx_offset = 0;
	for (int n = 0; n < draw_data->CmdListsCount; n++)
	{
		const ImDrawList *cmd_list = draw_data->CmdLists[n];
		int idx_offset = 0;
		for (int i = 0; i < cmd_list->CmdBuffer.Size; i++)
		{
			const ImDrawCmd *pcmd = &cmd_list->CmdBuffer[i];
			if (pcmd->UserCallback)
				pcmd->UserCallback(cmd_list, pcmd);
			else
			{
				rw::Texture *tex = (rw::Texture *)pcmd->TextureId;
				if (tex && tex->raster)
				{
					rw::SetRenderStatePtr(rw::TEXTURERASTER, tex->raster);
					rw::SetRenderState(rw::TEXTUREADDRESSU, tex->getAddressU());
					rw::SetRenderState(rw::TEXTUREADDRESSV, tex->getAddressV());
					rw::SetRenderState(rw::TEXTUREFILTER, tex->getFilter());
				}
				else
					rw::SetRenderStatePtr(rw::TEXTURERASTER, nil);
				rw::im2d::RenderIndexedPrimitive(rw::PRIMTYPETRILIST,
												 g_vertbuf + vtx_offset, cmd_list->VtxBuffer.Size,
												 cmd_list->IdxBuffer.Data + idx_offset, pcmd->ElemCount);
			}
			idx_offset += pcmd->ElemCount;
		}
		vtx_offset += cmd_list->VtxBuffer.Size;
	}
}

RsEventStatus ImGuiEventHandler(RsEvent e, void *param)
{
	ImGuiIO &io = ImGui::GetIO();
	int c;

	switch (e)
	{
	case rsKEYDOWN:
		c = *(int *)param;
		if (c < 256)
			io.KeysDown[c] = 1;
		return rsEVENTPROCESSED;
	case rsKEYUP:
		c = *(int *)param;
		if (c < 256)
			io.KeysDown[c] = 0;
		return rsEVENTPROCESSED;
	}
	return rsEVENTPROCESSED;
}

void CImGuiIII::Initialize()
{
	ImGuiIO &io = ImGui::GetIO();
	io.IniFilename = nullptr;
	io.MouseDrawCursor = true;

	io.KeyMap[ImGuiKey_Tab] = RsKeyCodes::rsTAB;
	io.KeyMap[ImGuiKey_LeftArrow] = RsKeyCodes::rsLEFT;
	io.KeyMap[ImGuiKey_RightArrow] = RsKeyCodes::rsRIGHT;
	io.KeyMap[ImGuiKey_UpArrow] = RsKeyCodes::rsUP;
	io.KeyMap[ImGuiKey_DownArrow] = RsKeyCodes::rsDOWN;
	io.KeyMap[ImGuiKey_PageUp] = RsKeyCodes::rsPGUP;
	io.KeyMap[ImGuiKey_PageDown] = RsKeyCodes::rsPGDN;
	io.KeyMap[ImGuiKey_Home] = RsKeyCodes::rsHOME;
	io.KeyMap[ImGuiKey_End] = RsKeyCodes::rsEND;
	io.KeyMap[ImGuiKey_Delete] = RsKeyCodes::rsDEL;
	io.KeyMap[ImGuiKey_Backspace] = RsKeyCodes::rsBACKSP;
	io.KeyMap[ImGuiKey_Enter] = RsKeyCodes::rsENTER;
	io.KeyMap[ImGuiKey_Escape] = RsKeyCodes::rsESC;
	io.KeyMap[ImGuiKey_A] = 'A';
	io.KeyMap[ImGuiKey_C] = 'C';
	io.KeyMap[ImGuiKey_V] = 'V';
	io.KeyMap[ImGuiKey_X] = 'X';
	io.KeyMap[ImGuiKey_Y] = 'Y';
	io.KeyMap[ImGuiKey_Z] = 'Z';

	io.RenderDrawListsFn = RenderDrawLists;

	g_vertbufSize = 2000 + 5000;
	g_vertbuf = rwNewT(RwIm2DVertex, g_vertbufSize, 0);

	unsigned char *pixels;
	int width, height;
	io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height, nil);

	RwImage *image;
	image = RwImageCreate(width, height, 32);
	image->allocate();
	for (int y = 0; y < height; y++)
		memcpy(image->pixels + image->stride * y, pixels + width * 4 * y, width * 4);
	g_FontTexture = RwTextureCreate(rw::Raster::createFromImage(image));
	g_FontTexture->setFilter(rw::Texture::LINEAR);
	image->destroy();
	io.Fonts->TexID = (void *)g_FontTexture;
}

void CImGuiIII::Shutdown()
{
	RwTextureDestroy(g_FontTexture);
	if (g_vertbuf)
		rwFree(g_vertbuf);
}

#define KEYJUSTDOWN(k) ControlsManager.GetIsKeyboardKeyJustDown((RsKeyCodes)k)
#define KEYDOWN(k) ControlsManager.GetIsKeyboardKeyDown((RsKeyCodes)k)
#define CTRLJUSTDOWN(key)                                                      \
	((KEYDOWN(rsLCTRL) || KEYDOWN(rsRCTRL)) && KEYJUSTDOWN((RsKeyCodes)key) || \
	 (KEYJUSTDOWN(rsLCTRL) || KEYJUSTDOWN(rsRCTRL)) && KEYDOWN((RsKeyCodes)key))
#define CTRLDOWN(key) ((KEYDOWN(rsLCTRL) || KEYDOWN(rsRCTRL)) && KEYDOWN((RsKeyCodes)key))

static bool bDrawImGui = false;

// forward declaration
void ImGuiOptions();

void CImGuiIII::Draw()
{
	if (!bDrawImGui)
		return;

	ImGuiIO &io = ImGui::GetIO();
	NewFrame();

	ImGui::NewFrame();

	bool p_open = true;
	ImGuiWindowFlags window_flags = 0;
    ImGui::SetNextWindowSize(ImVec2(550,680), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("ImGui Demo", &p_open, window_flags))
    {
        ImGui::End();
        return;
    }

	ImGuiOptions();

	ImGui::End();

	//ImGui::ShowDemoWindow();

	ImGui::EndFrame();
	ImGui::Render();
}

void CImGuiIII::Process()
{
	CPad *pad = CPad::GetPad(0);
	if(CTRLJUSTDOWN('M'))
		bDrawImGui = !bDrawImGui;
	if(!bDrawImGui)
		return;

	pad->DisablePlayerControls = 1;
}

void CImGuiIII::NewFrame()
{
	ImGuiIO &io = ImGui::GetIO();

	io.DisplaySize = ImVec2((float)RsGlobal.maximumWidth, (float)RsGlobal.maximumHeight);
	io.DeltaTime = CTimer::GetTimeStep() * 0.02f;

	int dirX = 1;
	int dirY = 1;

	if (MousePointerStateHelper.bInvertHorizontally)
		dirX = -1;
	if (MousePointerStateHelper.bInvertVertically)
		dirY = -1;

	static float mx, my;

	mx += CPad::NewMouseControllerState.x * dirX;
	my += CPad::NewMouseControllerState.y * dirY;

	if (mx < 0.0f)
		mx = 0.0f;
	if (my < 0.0f)
		my = 0.0f;
	if (mx >= (float)RsGlobal.maximumWidth)
		mx = (float)RsGlobal.maximumWidth;
	if (my >= (float)RsGlobal.maximumHeight)
		my = (float)RsGlobal.maximumHeight;
	io.MousePos = ImVec2(mx, my);
	io.MouseDown[0] = CPad::NewMouseControllerState.LMB ? true : false;
	io.MouseDown[1] = CPad::NewMouseControllerState.RMB ? true : false;
	io.MouseDown[2] = CPad::NewMouseControllerState.MMB ? true : false;

#ifdef DANGEROUS_CODE
	// check this first, may crash
	static BYTE KeyStates[256];

	GetKeyboardState(KeyStates);

	for (int i = 0; i < 256; i++)
	{
		if (KeyStates[i] & 0x80 && !io.KeysDown[i]) //key is down
		{
			io.KeysDown[i] = true;

			char res[2] = {0};
			if (ToAscii(i, MapVirtualKey(i, 0), (const BYTE *)KeyStates, (LPWORD)res, 0) == 1)
			{
				io.AddInputCharactersUTF8(res);
			}
		}
		else if (!(KeyStates[i] & 0x80) && io.KeysDown[i])
			io.KeysDown[i] = false;
	}
#endif
}
#endif