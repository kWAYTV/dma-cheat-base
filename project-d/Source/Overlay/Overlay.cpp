#include <Pch.hpp>
#include <SDK.hpp>

#include "Overlay.hpp"
#include "Fonts/IBMPlexMono_Medium.h"

ID3D11Device* Overlay::device = nullptr;

ID3D11DeviceContext* Overlay::device_context = nullptr;

IDXGISwapChain* Overlay::swap_chain = nullptr;

ID3D11RenderTargetView* Overlay::render_targetview = nullptr;

HWND Overlay::overlay = nullptr;
WNDCLASSEX Overlay::wc = { };

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK window_procedure(HWND window, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(window, msg, wParam, lParam))
		return true;

	switch (msg)
	{
	case WM_SYSCOMMAND:
		if ((wParam & 0xfff0) == SC_KEYMENU)
			return 0;
		break;

	case WM_DESTROY:
		Overlay::DestroyDevice();
		Overlay::DestroyOverlay();
		Overlay::DestroyImGui();
		PostQuitMessage(0);
		return 0;

	case WM_CLOSE:
		Overlay::DestroyDevice();
		Overlay::DestroyOverlay();
		Overlay::DestroyImGui();
		return 0;
	}

	return DefWindowProc(window, msg, wParam, lParam);
}

bool Overlay::CreateDevice()
{
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));

	sd.BufferCount = 2;

	sd.BufferDesc.Width = 0;
	sd.BufferDesc.Height = 0;

	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;

	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

	sd.OutputWindow = overlay;

	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;

	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	D3D_FEATURE_LEVEL featureLevel;
	const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };

	HRESULT result = D3D11CreateDeviceAndSwapChain(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		0U,
		featureLevelArray,
		2,
		D3D11_SDK_VERSION,
		&sd,
		&swap_chain,
		&device,
		&featureLevel,
		&device_context);

	if (result == DXGI_ERROR_UNSUPPORTED) {
		result = D3D11CreateDeviceAndSwapChain(
			nullptr,
			D3D_DRIVER_TYPE_WARP,
			nullptr,
			0U,
			featureLevelArray,
			2, D3D11_SDK_VERSION,
			&sd,
			&swap_chain,
			&device,
			&featureLevel,
			&device_context);

		LOG_ERROR("Created with D3D_DRIVER_TYPE_WARP");
	}

	if (result != S_OK) {
		LOG_ERROR("Device not supported");
		return false;
	}

	ID3D11Texture2D* back_buffer{ nullptr };
	swap_chain->GetBuffer(0U, IID_PPV_ARGS(&back_buffer));

	if (back_buffer)
	{
		device->CreateRenderTargetView(back_buffer, nullptr, &render_targetview);
		back_buffer->Release();
		return true;
	}

	LOG_ERROR("Failed to create device");
	return false;
}

void Overlay::DestroyDevice()
{
	if (device)
	{
		device->Release();
		device_context->Release();
		swap_chain->Release();
		render_targetview->Release();
	}
	else
		LOG_ERROR("Device not found when exiting");
}

bool Overlay::CreateOverlay()
{
	wc.cbSize = sizeof(wc);
	wc.style = CS_CLASSDC;
	wc.lpfnWndProc = window_procedure;
	wc.hInstance = GetModuleHandleA(0);
	wc.lpszClassName = L"Awhare";

	RegisterClassEx(&wc);

	overlay = CreateWindowEx(
		WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_TOOLWINDOW,
		wc.lpszClassName,
		L"Awhare",
		WS_POPUP,
		0,
		0,
		GetSystemMetrics(SM_CXSCREEN),
		GetSystemMetrics(SM_CYSCREEN),
		NULL,
		NULL,
		wc.hInstance,
		NULL
	);

	if (overlay == NULL)
	{
		LOG_ERROR("Failed to create overlay");
		return false;
	}

	SetLayeredWindowAttributes(overlay, RGB(0, 0, 0), BYTE(255), LWA_ALPHA);

	{
		RECT client_area{};
		RECT window_area{};

		GetClientRect(overlay, &client_area);
		GetWindowRect(overlay, &window_area);

		POINT diff{};
		ClientToScreen(overlay, &diff);

		const MARGINS margins{
			window_area.left + (diff.x - window_area.left),
			window_area.top + (diff.y - window_area.top),
			client_area.right,
			client_area.bottom
		};

		DwmExtendFrameIntoClientArea(overlay, &margins);
	}

	ShowWindow(overlay, SW_SHOW);
	UpdateWindow(overlay);

	return true;
}

void Overlay::DestroyOverlay()
{
	DestroyWindow(overlay);
	UnregisterClass(wc.lpszClassName, wc.hInstance);
}

bool Overlay::CreateImGui()
{
	ImGui::CreateContext();
	ImGui::StyleColorsDark();

	if (!ImGui_ImplWin32_Init(overlay)) {
		LOG_ERROR("Failed ImGui_ImplWin32_Init");
		return false;
	}

	if (!ImGui_ImplDX11_Init(device, device_context)) {
		LOG_ERROR("Failed ImGui_ImplDX11_Init");
		return false;
	}

	return true;
}

void Overlay::DestroyImGui()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void Overlay::SetForeground(HWND window)
{
	if (!IsWindowInForeground(window))
		BringToForeground(window);
}

void Overlay::StartRender()
{
	MSG msg;
	while (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	if (GetAsyncKeyState(VK_INSERT) & 1)
	{
		shouldRenderMenu = !shouldRenderMenu;

		if (shouldRenderMenu)
		{
			SetWindowLong(overlay, GWL_EXSTYLE, WS_EX_TOOLWINDOW);
		}
		else
		{
			SetWindowLong(overlay, GWL_EXSTYLE, WS_EX_TOOLWINDOW | WS_EX_TRANSPARENT | WS_EX_LAYERED);
		}
	}
}

void Overlay::EndRender()
{
	ImGui::Render();

	float color[4];
	if (config.Visuals.Background) // Black bg
	{
		color[0] = 0; color[1] = 0; color[2] = 0; color[3] = 1;
	}
	else // Transparent bg
	{
		color[0] = 0; color[1] = 0; color[2] = 0; color[3] = 0;
	}

	device_context->OMSetRenderTargets(1, &render_targetview, nullptr);
	device_context->ClearRenderTargetView(render_targetview, color);

	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	swap_chain->Present(config.Visuals.VSync ? 1U : 0U, 0U);
}

void Overlay::StyleMenu(ImGuiIO& IO, ImGuiStyle& style)
{
	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	// Disable Ini file
	IO.IniFilename = nullptr;

    // Custom styles
    style.WindowRounding    = 0;
    style.ChildRounding     = 0;
    style.FrameRounding     = 0;
    style.GrabRounding      = 0;
    style.PopupRounding     = 0;
    style.TabRounding       = 0;
    style.ScrollbarRounding = 0;

    style.ButtonTextAlign   = { 0.5f, 0.5f };
    style.WindowTitleAlign  = { 0.5f, 0.5f };
    style.FramePadding      = { 8.0f, 8.0f };
    style.WindowPadding     = { 10.0f, 10.0f };
    style.ItemSpacing       = ImVec2(style.WindowPadding.x, style.WindowPadding.y * 0.75f);
    style.ItemInnerSpacing  = { 10, 4 };

    style.WindowBorderSize  = 1;
    style.FrameBorderSize   = 1;
    style.PopupBorderSize   = 1;

    style.ScrollbarSize     = 12.f;
    style.GrabMinSize       = style.FrameRounding;
    
    // Colors
    style.Colors[ImGuiCol_WindowBg]             = ImAdd::HexToColorVec4(0x181818, 0.3f);
    style.Colors[ImGuiCol_PopupBg]              = ImAdd::HexToColorVec4(0x181818, 1.0f);
    style.Colors[ImGuiCol_ChildBg]              = ImAdd::HexToColorVec4(0x282828, 1.0f);

    style.Colors[ImGuiCol_Text]                 = ImAdd::HexToColorVec4(0xFFFFFF, 1.0f);
    style.Colors[ImGuiCol_CheckMark]            = style.Colors[ImGuiCol_Text];
    style.Colors[ImGuiCol_TextDisabled]         = ImAdd::HexToColorVec4(0xA3A3A3, 1.0f);

    style.Colors[ImGuiCol_SliderGrab]           = ImAdd::HexToColorVec4(0x545070, 1.0f);
    style.Colors[ImGuiCol_SliderGrabActive]     = ImAdd::HexToColorVec4(0x45425D, 1.0f);
    
    style.Colors[ImGuiCol_ScrollbarGrab]        = ImAdd::HexToColorVec4(0x181818, 1.0f);
    style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImAdd::HexToColorVec4(0x181818, 1.0f);
    style.Colors[ImGuiCol_ScrollbarGrabActive]  = ImAdd::HexToColorVec4(0x181818, 1.0f);

    style.Colors[ImGuiCol_ScrollbarBg]          = ImVec4(0, 0, 0, 0);

    style.Colors[ImGuiCol_Border]               = ImAdd::HexToColorVec4(0x060606, 1.0f);
    style.Colors[ImGuiCol_Separator]            = style.Colors[ImGuiCol_Border];

    style.Colors[ImGuiCol_Button]               = ImAdd::HexToColorVec4(0x181818, 1.0f);
    style.Colors[ImGuiCol_ButtonHovered]        = ImAdd::HexToColorVec4(0x181818, 0.7f);
    style.Colors[ImGuiCol_ButtonActive]         = ImAdd::HexToColorVec4(0x181818, 0.5f);

    style.Colors[ImGuiCol_FrameBg]              = ImAdd::HexToColorVec4(0x181818, 1.0f);
    style.Colors[ImGuiCol_FrameBgHovered]       = ImAdd::HexToColorVec4(0x181818, 0.7f);
    style.Colors[ImGuiCol_FrameBgActive]        = ImAdd::HexToColorVec4(0x181818, 0.5f);

    style.Colors[ImGuiCol_Header]               = ImAdd::HexToColorVec4(0x282828, 1.0f);
    style.Colors[ImGuiCol_HeaderHovered]        = ImAdd::HexToColorVec4(0x282828, 0.7f);
    style.Colors[ImGuiCol_HeaderActive]         = ImAdd::HexToColorVec4(0x282828, 0.5f);

	static bool bInit = false;
	if (!bInit)
	{
		m_iSelectedPage = 0;

		m_Tabs.push_back("Aim");     // MenuPage_Aiming
		m_Tabs.push_back("Visuals");    // MenuPage_Visuals
		m_Tabs.push_back("Config");    // MenuPage_Configs
		m_Tabs.push_back("Info");       // MenuPage_Info

		ImFont* MainFont = IO.Fonts->AddFontFromMemoryCompressedTTF(IBMPlexMono_Medium_compressed_data, IBMPlexMono_Medium_compressed_size, 14, nullptr, IO.Fonts->GetGlyphRangesDefault());

		bInit = true;
	}
}

bool Overlay::Create()
{
	shouldRun = true;
	shouldRenderMenu = false;

	if (!CreateOverlay())
		return false;

	if (!CreateDevice())
		return false;

	if (!CreateImGui())
		return false;

	SetForeground(GetConsoleWindow());
	return true;
}

void Overlay::Destroy()
{
	DestroyImGui();
	DestroyDevice();
	DestroyOverlay();
}

void Overlay::RenderMenu()
{
	ImGuiStyle& style = ImGui::GetStyle();
	ImGuiIO& io = ImGui::GetIO();

	ImGui::SetNextWindowSize(ImVec2(570, 500), ImGuiCond_Always);
	ImGui::Begin(
		"Awhare",
		&shouldRenderMenu,
		ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_AlwaysAutoResize |
		ImGuiWindowFlags_NoDecoration
	);

	StyleMenu(io, style);

	float OverlayFps = ImGui::GetIO().Framerate;

	ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 2 - ImGui::CalcTextSize("Awhare").x / 2);
	ImGui::Text("Awhare");

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	ImGui::PushStyleColor(ImGuiCol_ChildBg, style.Colors[ImGuiCol_FrameBg]);
	ImGui::BeginChild("Main", ImVec2(0, 0), ImGuiCol_Border);
	ImGui::PopStyleColor();
	ImGui::PopStyleVar();
	{
		ImGui::BeginChild("MenuBar", ImVec2(0, ImGui::GetFrameHeight()), 0, ImGuiWindowFlags_NoBackground);
		{
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
			{
				int RadioWidth = ImGui::GetWindowWidth() / m_Tabs.size();

				for (int i = 0; i < m_Tabs.size(); i++)
				{
					ImAdd::RadioFrame(m_Tabs[i], &m_iSelectedPage, i, i % 2 == 0, ImVec2(i == m_Tabs.size() - 1 ? -0.1f : RadioWidth, ImGui::GetWindowHeight()));
					ImGui::SameLine();
				}
			}
			ImGui::PopStyleVar();

			ImGui::GetWindowDrawList()->AddLine(ImGui::GetWindowPos() + ImVec2(style.WindowBorderSize, ImGui::GetWindowHeight() - style.WindowBorderSize), ImGui::GetWindowPos() + ImVec2(ImGui::GetWindowWidth() - style.WindowBorderSize, ImGui::GetWindowHeight() - style.WindowBorderSize), ImGui::GetColorU32(ImGuiCol_Border), style.WindowBorderSize);
		}
		ImGui::EndChild();
		ImGui::SetCursorPosY(ImGui::GetFrameHeight());
		ImGui::BeginChild("Content", ImVec2(0, ImGui::GetWindowHeight() - ImGui::GetFrameHeight() * 2), ImGuiChildFlags_Border, ImGuiWindowFlags_NoBackground);
		{
			float fGroupWidth = (ImGui::GetWindowWidth() - style.WindowPadding.x * 2 - style.ItemSpacing.x) / 2;

			if (m_iSelectedPage == MenuPage_Aim)
			{
				ImGui::BeginGroup(); // Left group
				{
					ImGui::BeginChild("Aimbot", ImVec2(fGroupWidth, 
						ImGui::GetFrameHeight() + // MenuBar
						style.WindowPadding.y * 2 + // child padding
						style.ItemSpacing.x * 15 + // spacing
						ImGui::GetFontSize() * 10 // checkbox + separators
					), ImGuiChildFlags_Border, ImGuiWindowFlags_MenuBar); 
					{
						if (ImGui::BeginMenuBar()) {
							ImGui::SetCursorPos(style.FramePadding);
							ImAdd::CheckBox("Aimbot##Enable", &config.Aim.Aimbot);
							ImGui::EndMenuBar();
						}

						if (config.Aim.Aimbot)
						{
							if (ProcInfo::KmboxInitialized)
							{
								ImAdd::CheckBox("Draw FOV", &config.Aim.DrawFov);
								if (config.Aim.DrawFov)
								{
									ImGui::SameLine(ImGui::GetWindowWidth() - ImGui::GetFontSize() * 2 - style.WindowPadding.x * 2);
									ImAdd::ColorEdit4("##FovColor", (float*)&config.Aim.AimbotFovColor);
								}

								ImAdd::CheckBox("Aim Visible", &config.Aim.AimVisible);
								ImAdd::CheckBox("Aim Teammates", &config.Aim.AimFriendly);

								ImAdd::KeyBindOptions KeyMode = (ImAdd::KeyBindOptions)config.Aim.AimbotKeyMode;
								ImAdd::KeyBind("Aimbot Key", &config.Aim.AimbotKey, 0, &KeyMode);

								ImAdd::SliderFloat("Aimbot Fov", &config.Aim.AimbotFov, 0.0f, 180.0f);
								ImAdd::SliderFloat("Aimbot Smooth", &config.Aim.AimbotSmooth, 0.0f, 100.0f);
							}
							else
							{
								ImGui::SetCursorPos(ImVec2(ImGui::GetWindowWidth(), ImGui::GetWindowHeight() - ImGui::GetFrameHeight()) / 2 - ImGui::CalcTextSize("KMBOX not connected.") / 2 + ImVec2(0, ImGui::GetFrameHeight()));
								ImGui::TextColored(ImVec4(1, 0, 0, 1), "KMBOX not connected.");
							}
						}
					}
					ImGui::EndChild();
				}
				ImGui::EndGroup();
				ImGui::SameLine();
				ImGui::BeginGroup();
				{
					ImGui::BeginChild("Trigger", ImVec2(fGroupWidth,
						ImGui::GetFrameHeight() + // MenuBar
						style.WindowPadding.y * 2 + // child padding
						style.ItemSpacing.x * 15 + // spacing
						ImGui::GetFontSize() * 10 // checkbox + separators
					), ImGuiChildFlags_Border, ImGuiWindowFlags_MenuBar);
					{
						if (ImGui::BeginMenuBar()) {
							ImGui::SetCursorPos(style.FramePadding);
							ImAdd::CheckBox("Trigger##Enable", &config.Aim.Trigger);
							ImGui::EndMenuBar();
						}

						if (config.Aim.Trigger)
						{
							if (ProcInfo::KmboxInitialized)
							{
								ImAdd::CheckBox("Trigger##Enable", &config.Aim.Trigger);

								ImAdd::KeyBindOptions KeyMode = (ImAdd::KeyBindOptions)config.Aim.TriggerKeyMode;
								ImAdd::KeyBind("Trigger Key", &config.Aim.TriggerKey, 0, &KeyMode);

								ImAdd::SliderInt("Trigger Delay (ms)", &config.Aim.TriggerDelay, 0, 250);
							}
							else
							{
								ImGui::SetCursorPos(ImVec2(ImGui::GetWindowWidth(), ImGui::GetWindowHeight() - ImGui::GetFrameHeight()) / 2 - ImGui::CalcTextSize("KMBOX not connected.") / 2 + ImVec2(0, ImGui::GetFrameHeight()));
								ImGui::TextColored(ImVec4(1, 0, 0, 1), "KMBOX not connected.");
							}
						}
					}
					ImGui::EndChild();
				}
				ImGui::EndGroup();
			}

			else if (m_iSelectedPage == MenuPage_Visuals)
			{
				ImGui::BeginChild("Visuals", ImVec2(fGroupWidth,
					ImGui::GetFrameHeight() + // MenuBar
					style.WindowPadding.y * 2 + // child padding
					style.ItemSpacing.x * 15 + // spacing
					ImGui::GetFontSize() * 10 // checkbox + separators
				), ImGuiChildFlags_Border, ImGuiWindowFlags_MenuBar);
				{
					if (ImGui::BeginMenuBar()) {
						ImGui::SetCursorPos(style.FramePadding);
						ImAdd::CheckBox("Visuals##Enable", &config.Visuals.Enabled); 
						ImGui::EndMenuBar();
					}

					if (config.Visuals.Enabled)
					{
						ImAdd::SeparatorText("General");

						ImGui::BeginGroup();
						{
							ImAdd::CheckBox("Watermark", &config.Visuals.Watermark);
							if (config.Visuals.Watermark)
							{
								ImGui::SameLine(ImGui::GetWindowWidth() - ImGui::GetFontSize() * 2 - style.WindowPadding.x * 2);
								ImAdd::ColorEdit4("##WatermarkColor", (float*)&config.Visuals.WatermarkColor);
							}
						}

						ImAdd::CheckBox("Background", &config.Visuals.Background);

						ImAdd::SeparatorText("Visual");
						ImAdd::CheckBox("VSync", &config.Visuals.VSync);
						ImAdd::CheckBox("Team Check", &config.Visuals.TeamCheck);
						ImAdd::CheckBox("Visible Check", &config.Visuals.VisibleCheck);

						ImGui::BeginGroup();
						{
							ImAdd::CheckBox("Hitmarker", &config.Visuals.Hitmarker);
							if (config.Visuals.Hitmarker)
							{
								ImGui::SameLine(ImGui::GetWindowWidth() - ImGui::GetFontSize() * 2 - style.WindowPadding.x * 2);
								ImAdd::ColorEdit4("##HitmarkerColor", (float*)&config.Visuals.HitmarkerColor);
							}
						}

						ImAdd::SeparatorText("Players");

						ImGui::BeginGroup();
						{
							ImAdd::CheckBox("Name", &config.Visuals.Name);
							if (config.Visuals.Name)
							{
								ImGui::SameLine(ImGui::GetWindowWidth() - ImGui::GetFontSize() * 2 - style.WindowPadding.x * 2);
								ImAdd::ColorEdit4("##NameColor", (float*)&config.Visuals.NameColor);
							}
						}

						ImAdd::CheckBox("Health", &config.Visuals.Health);

						ImGui::BeginGroup();
						{
							ImAdd::CheckBox("Box", &config.Visuals.Box);
							if (config.Visuals.Box)
							{
								ImAdd::ColorEdit4("Box Color", (float*)&config.Visuals.BoxColor);
								ImAdd::ColorEdit4("Box Color Visible", (float*)&config.Visuals.BoxColorVisible);
							}
						}

						ImGui::BeginGroup();
						{
							ImAdd::CheckBox("Weapon", &config.Visuals.Weapon);
							if (config.Visuals.Weapon)
							{
								ImGui::SameLine(ImGui::GetWindowWidth() - ImGui::GetFontSize() * 2 - style.WindowPadding.x * 2);
								ImAdd::ColorEdit4("##WeaponColor", (float*)&config.Visuals.WeaponColor);
							}
						}

						ImGui::BeginGroup();
						{
							ImAdd::CheckBox("Bones", &config.Visuals.Bones);
							if (config.Visuals.Bones)
							{
								ImGui::SameLine(ImGui::GetWindowWidth() - ImGui::GetFontSize() * 2 - style.WindowPadding.x * 2);
								ImAdd::ColorEdit4("##BonesColor", (float*)&config.Visuals.BonesColor);
							}
						}
					}
				}
				ImGui::EndChild();
			}

			else if (m_iSelectedPage == MenuPage_Config)
			{
				ImGui::BeginChild("Configs", ImVec2(0, 0), ImGuiChildFlags_Border, ImGuiWindowFlags_MenuBar);
				{
					if (ImGui::BeginMenuBar()) {
						ImGui::Text("Configs");
						ImGui::EndMenuBar();
					}
						
					static char configName[128] = "";
					static std::vector<std::string> configFiles;

					if (ImAdd::Button("Refresh"))
					{
						configFiles = config.ListConfigs("configs/");
						LOG_INFO("Refreshed config list");
					}

					ImGui::Separator();

					// Config List
					if (ImGui::BeginListBox("Config list"))
					{
						if (configFiles.empty())
						{
							ImGui::Selectable("No configs found", false, ImGuiSelectableFlags_Disabled);
						}
						else
						{
							for (const auto& file : configFiles)
							{
								bool isSelected = (file == configName);
								if (ImGui::Selectable(file.c_str(), isSelected))
								{
									strcpy(configName, file.c_str());
									LOG_INFO("Selected config: {}", configName);
								}

								if (isSelected)
								{
									ImGui::SetItemDefaultFocus();
								}
							}
						}
						ImGui::EndListBox();
					}

					// Config Name Input
					ImGui::InputText("Config Name", configName, IM_ARRAYSIZE(configName));

					// Control Buttons
					float buttonWidth = 75.0f;
					float buttonSpacing = 10.0f;
					ImGui::Dummy(ImVec2(0.0f, 5.0f));

					if (ImAdd::Button("Load", ImVec2(buttonWidth, 0)))
					{
						std::string filePath = "configs/" + std::string(configName);
						if (!config.LoadFromFile(filePath))
						{
							LOG_ERROR("Failed to load config: {}", filePath);
						}
						else
						{
							LOG_INFO("Loaded config: {}", filePath);
						}
					}

					ImGui::SameLine(0.0f, buttonSpacing);

					if (ImAdd::Button("Save", ImVec2(buttonWidth, 0)))
					{
						std::string filePath = "configs/" + std::string(configName);
						if (!config.SaveToFile(filePath))
						{
							LOG_ERROR("Failed to save config: {}", filePath);
						}
						else
						{
							LOG_INFO("Saved config: {}", filePath);
						}
					}

					ImGui::SameLine(0.0f, buttonSpacing);

					if (ImAdd::Button("Delete", ImVec2(buttonWidth, 0)))
					{
						std::string filePath = "configs/" + std::string(configName);
						if (!config.DeleteConfigFile(filePath))
						{
							LOG_ERROR("Failed to delete config: {}", filePath);
						}
						else
						{
							LOG_INFO("Deleted config: {}", filePath);
							configFiles = config.ListConfigs("configs/");
						}
					}

					ImGui::SameLine(0.0f, buttonSpacing);

					if (ImAdd::Button("Import", ImVec2(buttonWidth, 0)))
					{
						if (config.LoadFromClipboard())
						{
							LOG_INFO("Config imported from clipboard");
						}
						else
						{
							LOG_ERROR("Failed to import config from clipboard");
						}
					}
				}
				ImGui::EndChild();
			}

			else if (m_iSelectedPage == MenuPage_Info)
			{
				ImGui::BeginChild("Info", ImVec2(0, 0), ImGuiChildFlags_Border, ImGuiWindowFlags_MenuBar);
				{
					if (ImGui::BeginMenuBar()) {
						ImGui::Text("Info");
						ImGui::EndMenuBar();
					}

					ImAdd::SeparatorText("Hardware");

					ImGui::Text("DMA:");
					ImGui::SameLine();
					ImGui::TextColored(ProcInfo::DmaInitialized ? ImVec4(0, 1, 0, 1)/* green */ : ImVec4(1, 0, 0, 1)/* red */, "%s", ProcInfo::DmaInitialized ? "Connected" : "Disconnected");

					ImGui::Text("KMBOX:");
					ImGui::SameLine();
					ImGui::TextColored(ProcInfo::KmboxInitialized ? ImVec4(0, 1, 0, 1)/* green */: ImVec4(1, 0, 0, 1)/* red */, "%s", ProcInfo::KmboxInitialized ? "Connected" : "Disconnected");

					ImAdd::SeparatorText("Game");

					ImGui::Text("Client:");
					ImGui::SameLine();
					ImGui::Text("0x%llx", Globals::ClientBase);

					ImAdd::SeparatorText("Cheat");

					ImGui::Text("Overlay FPS: %.2f", OverlayFps);

					float buttonWidth = 100.0f;
					float buttonSpacing = 20.0f;
					ImGui::SetCursorPosX((ImGui::GetWindowSize().x - 2 * buttonWidth - buttonSpacing) / 2);

					if (ImAdd::Button("Open folder", ImVec2(buttonWidth, 0)))
					{
						ShellExecuteA(nullptr, "open", "explorer.exe", ".\\", nullptr, SW_SHOW);
					}

					ImGui::SameLine();

					if (ImAdd::Button("Unload", ImVec2(buttonWidth, 0)))
					{
						Globals::Running = false;
						shouldRun = false;
					}
				}
				ImGui::EndChild();
			}
		}
		ImGui::EndChild();
		ImGui::SetCursorPosY(ImGui::GetWindowHeight() - ImGui::GetFrameHeight());
		ImGui::BeginChild("Footer", ImVec2(0, 0), 0, ImGuiWindowFlags_NoBackground);
		{
			ImGui::GetWindowDrawList()->AddRectFilled(ImGui::GetWindowPos(), ImGui::GetWindowPos() + ImGui::GetWindowSize(), ImGui::GetColorU32(ImGuiCol_ChildBg), style.WindowRounding, ImDrawFlags_RoundCornersBottom);
			ImGui::GetWindowDrawList()->AddLine(ImGui::GetWindowPos() + ImVec2(style.WindowBorderSize, 0), ImGui::GetWindowPos() + ImVec2(ImGui::GetWindowWidth() - style.WindowBorderSize, 0), ImGui::GetColorU32(ImGuiCol_Border), style.WindowBorderSize);
			ImGui::GetWindowDrawList()->AddText(ImGui::GetWindowPos() + style.FramePadding, ImGui::GetColorU32(ImGuiCol_Text), "Build: Developer");
			ImGui::GetWindowDrawList()->AddText(ImGui::GetWindowPos() + ImVec2(ImGui::GetWindowWidth() - ImGui::CalcTextSize("Expires: Never").x - style.FramePadding.x, style.FramePadding.y), ImGui::GetColorU32(ImGuiCol_TextDisabled), "Expires: Never");
		}
		ImGui::EndChild();
	}

	ImGui::EndChild();
	ImGui::End();
}