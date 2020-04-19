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

float CImGuiIII::ms_fMouseX = 0.0f;
float CImGuiIII::ms_fMouseY = 0.0f;
CImGuiClassMenu CImGuiIII::ms_aClassMenuArray[MAX_IMGUI_CLASS_MENU_ENTRIES] = {0};
uint32 CImGuiIII::ms_nClassMenuCount = 0;

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

	rw::Camera *cam = Scene.camera;
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
void ImGuiAddOptions();

void CImGuiIII::Draw()
{
	if (!bDrawImGui)
		return;

	ImGuiIO &io = ImGui::GetIO();
	NewFrame();

	ImGui::NewFrame();

	bool p_open = true;
	ImGuiWindowFlags window_flags = 0;
	ImGui::SetNextWindowSize(ImVec2(550, 680), ImGuiCond_FirstUseEver);
	if (!ImGui::Begin("Debug Menu", &p_open, window_flags))
	{
		ImGui::End();
		return;
	}

	for (int i = 0; i < ms_nClassMenuCount; i++)
	{
		if (ImGui::CollapsingHeader(ms_aClassMenuArray[i].m_acClassName))
		{
			for (int j = 0; j < ms_aClassMenuArray[i].m_nEntriesCount; j++)
			{
				auto pMenu = &ms_aClassMenuArray[i].m_aEntriesArray[j];
				if (pMenu->type == 1)
					ImGui::Checkbox(pMenu->m_acName, pMenu->m_pVar);
				else
				{
					if (ImGui::Button(pMenu->m_acName))
						pMenu->cb();
				}
				ImGui::Separator();
			}
		}
	}
	ImGui::End();

	//ImGui::ShowDemoWindow();

	ImGui::EndFrame();
	ImGui::Render();
}

void CImGuiIII::Process()
{
	static bool bInitializedOptions = false;
	if (!bInitializedOptions)
	{
		ImGuiAddOptions();
		bInitializedOptions = true;
	}

	CPad *pad = CPad::GetPad(0);
	if (CTRLJUSTDOWN('M'))
		bDrawImGui = !bDrawImGui;
	if (!bDrawImGui)
		return;

	pad->DisablePlayerControls |= 1;

	UpdateMouse();
}

void CImGuiIII::UpdateMouse()
{
	CPad *pad = CPad::GetPad(0);

	int dirX = 1;
	int dirY = 1;

	if (MousePointerStateHelper.bInvertHorizontally)
		dirX = -1;
	if (MousePointerStateHelper.bInvertVertically)
		dirY = -1;

	ms_fMouseX += CPad::NewMouseControllerState.x * dirX;
	ms_fMouseY += CPad::NewMouseControllerState.y * dirY;

	if (ms_fMouseX < 0.0f)
		ms_fMouseX = 0.0f;
	if (ms_fMouseY < 0.0f)
		ms_fMouseY = 0.0f;
	if (ms_fMouseX >= (float)RsGlobal.maximumWidth)
		ms_fMouseX = (float)RsGlobal.maximumWidth;
	if (ms_fMouseY >= (float)RsGlobal.maximumHeight)
		ms_fMouseY = (float)RsGlobal.maximumHeight;

	pad->NewMouseControllerState.x = 0.0f;
	pad->NewMouseControllerState.y = 0.0f;
}

void CImGuiIII::NewFrame()
{
	ImGuiIO &io = ImGui::GetIO();

	io.DisplaySize = ImVec2((float)RsGlobal.maximumWidth, (float)RsGlobal.maximumHeight);
	io.DeltaTime = CTimer::GetTimeStep() * 0.02f;
	io.MousePos = ImVec2(ms_fMouseX, ms_fMouseY);
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

void CImGuiIII::AddVarInt8(const char *pszClassName, const char *pszName, bool *pVal)
{
	int nIndex = -1;
	for (int i = 0; i < MAX_IMGUI_CLASS_MENU_ENTRIES; i++)
	{
		if (stricmp(ms_aClassMenuArray[i].m_acClassName, pszClassName) == 0)
		{
			nIndex = i;
			break;
		}
	}

	if (nIndex < 0)
	{
		nIndex = ms_nClassMenuCount;
		strcpy(ms_aClassMenuArray[nIndex].m_acClassName, pszClassName);
		ms_nClassMenuCount++;
	}

	auto pMenu = &ms_aClassMenuArray[nIndex].m_aEntriesArray[ms_aClassMenuArray[nIndex].m_nEntriesCount];
	strcpy(pMenu->m_acName, pszName);
	pMenu->m_pVar = pVal;
	pMenu->type = 1;
	ms_aClassMenuArray[nIndex].m_nEntriesCount++;
}

void CImGuiIII::AddVarCmd(const char *pszClassName, const char *pszName, VoidCB cb)
{
	int nIndex = -1;
	for (int i = 0; i < MAX_IMGUI_CLASS_MENU_ENTRIES; i++)
	{
		if (stricmp(ms_aClassMenuArray[i].m_acClassName, pszClassName) == 0)
		{
			nIndex = i;
			break;
		}
	}

	if (nIndex < 0)
	{
		nIndex = ms_nClassMenuCount;
		strcpy(ms_aClassMenuArray[nIndex].m_acClassName, pszClassName);
		ms_nClassMenuCount++;
	}

	auto pMenu = &ms_aClassMenuArray[nIndex].m_aEntriesArray[ms_aClassMenuArray[nIndex].m_nEntriesCount];
	strcpy(pMenu->m_acName, pszName);
	pMenu->cb = cb;
	pMenu->type = 2;
	ms_aClassMenuArray[nIndex].m_nEntriesCount++;
}

#endif