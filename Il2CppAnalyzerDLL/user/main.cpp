#include "pch-il2cpp.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <string>
#include <iostream>
#include <thread>
#include <il2cpp-appdata.h>
#include <helpers.h>
#include <d3d11.h>
#include <dxgi.h>
#pragma comment(lib, "d3d11.lib")
#include <kiero.h>
#include <imgui.h>
#include <backends/imgui_impl_dx11.h>
#include <backends/imgui_impl_win32.h>
#include <MinHook.h>

#include "Output.h"
#include "AnalyzerGUI.h"
#include "Analyzer.h"

#define GAME_WINDOW_TITLE "OriAndTheWilloftheWisps"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

using namespace app;

ID3D11Device* g_pd3dDevice = NULL;
ID3D11DeviceContext* g_pd3dDeviceContext = NULL;
HWND g_gameWindow = NULL;

WNDPROC oWndProc;
LRESULT CALLBACK hkWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam)) {
		return true;
	}
	return CallWindowProc(oWndProc, hWnd, msg, wParam, lParam);
}

typedef HRESULT(*PresentFn)(IDXGISwapChain* __this, UINT SyncInterval, UINT Flags);
static PresentFn oPresent = nullptr;
HRESULT __stdcall hkPresent(IDXGISwapChain* __this, UINT SyncInterval, UINT Flags) {
	static bool init = false;
	if (!init) {
		// Get the dx11 device and context
		if (SUCCEEDED(__this->GetDevice(__uuidof(ID3D11Device), (void**)&g_pd3dDevice)))
		{
			g_pd3dDevice->GetImmediateContext(&g_pd3dDeviceContext);
		}

		ImGui::CreateContext();
		ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);
		ImGui_ImplWin32_Init(g_gameWindow);

		// Hook WndProc
		{
			Output::Log("Hooking WndProc...");
			oWndProc = (WNDPROC)GetWindowLongPtr(g_gameWindow, GWLP_WNDPROC);
			SetWindowLongPtr(g_gameWindow, GWLP_WNDPROC, (LONG_PTR)hkWndProc);
			Output::Log("WndProc hooked successfully!");
		}

		AnalyzerGUI::Initialize();

		init = true;
	}

	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	AnalyzerGUI::Render();

	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	return oPresent(__this, SyncInterval, Flags);
}

typedef void (*UpdateManager_UpdateFn)(UpdateManager*, float, MethodInfo*);
static UpdateManager_UpdateFn oUpdate = nullptr;
void hkUpdate(UpdateManager* __this, float deltaTime, MethodInfo* method) {
	if (Input_GetKeyDown(KeyCode__Enum::F2, nullptr)) {
		AnalyzerGUI::RefreshHierachy();
	}
	return oUpdate(__this, deltaTime, method);
}
typedef void (*Object_1_DontDestroyOnLoadFn)(Object_1*, MethodInfo*);
static Object_1_DontDestroyOnLoadFn oDontDestroyOnLoad = nullptr;
void hkDontDestroyOnLoad(Object_1* __this, MethodInfo* method) {
	Output::Log("DontDestroyOnLoad: %p", __this);
	Analyzer::AddDontDestroyOnLoadObject(__this);
	return oDontDestroyOnLoad(__this, method);
}

void Run() {
    il2cpp_thread_attach(il2cpp_domain_get());
    il2cppi_new_console();

    Output::Log("Initializing Il2CppAnalyzer...");
	// Get game window
	{
		Output::Log("Getting game window...");
		g_gameWindow = FindWindowA(NULL, GAME_WINDOW_TITLE);
		if (g_gameWindow == NULL) {
			Output::Fatal("Failed to get game window!", 1);
		}
		Output::Log("Game window: %p", g_gameWindow);
	}
	// Initialize kiero
	{
		Output::Log("Initializing Kiero...");
		kiero::Status::Enum status = kiero::init(kiero::RenderType::D3D11);
		if (status != kiero::Status::Success) {
			Output::Fatal("Failed to initialize kiero: %d", status);
		}
		Output::Log("Kiero initialized successfully!");
	}
	// Hook present
	{
		Output::Log("Hooking present...");
		kiero::Status::Enum status = kiero::bind(8, (void**)&oPresent, hkPresent);
		if (status != kiero::Status::Success) {
			Output::Fatal("Failed to hook present: %d", status);
		}
		Output::Log("Present hooked successfully!");
	}
	// Hook GameObject_DontDestroyOnLoad()
	{
		Output::Log("Hooking DontDestroyOnLoad...");
		MH_STATUS status = MH_CreateHook((void*)Object_1_DontDestroyOnLoad, hkDontDestroyOnLoad, (void**)&oDontDestroyOnLoad);
		if (status != MH_OK) {
			Output::Fatal("Failed to hook Object.DontDestroyOnLoad: %d", status);
		}
		status = MH_EnableHook((void*)Object_1_DontDestroyOnLoad);
		if (status != MH_OK) {
			Output::Fatal("Failed to enable Object.DontDestroyOnLoad hook: %d", status);
		}
	}
	// Hook UpdateManager_Update
	{
		Output::Log("Hooking Update...");
		MH_STATUS status = MH_CreateHook((void*)UpdateManager_Update, hkUpdate, (void**)&oUpdate);
		if (status != MH_OK) {
			Output::Fatal("Failed to hook UpdateManager.Update: %d", status);
		}
		status = MH_EnableHook((void*)UpdateManager_Update);
		if (status != MH_OK) {
			Output::Fatal("Failed to enable UpdateManager.Update hook: %d", status);
		}
		Output::Log("UpdateManager.Update hooked successfully!");
	}

	Output::Log("Il2CppAnalyzer initialized successfully!");
}