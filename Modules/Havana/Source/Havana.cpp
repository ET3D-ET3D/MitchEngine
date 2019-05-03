#include "Havana.h"
#include "imgui.h"
#include "examples/imgui_impl_win32.h"
#include "examples/imgui_impl_dx11.h"
#include "Engine/World.h"
#include "Components/Transform.h"
#include <stack>
#include "Components/Camera.h"
#include "ECS/Core.h"
#include "Engine/Engine.h"
#include "HavanaEvents.h"
#include <string>
#include <iostream>
#include "Graphics/RenderTexture.h"
#if ME_EDITOR
#include <filesystem>
#include "Math/Vector2.h"
#include "Mathf.h"
#include "Logger.h"
#include "Resource/ResourceCache.h"
#include "Graphics/Texture.h"
#include "Window/D3D12Window.h"
#include "Widgets/AssetBrowser.h"
#include <chrono>
#include "Events/EventManager.h"
#include "Engine/Input.h"
namespace fs = std::filesystem;

Havana::Havana(Engine* GameEngine, Moonlight::Renderer* renderer)
	: Renderer(renderer)
	, m_engine(GameEngine)
	, CurrentDirectory("Assets")
	, m_assetBrowser(FilePath("Assets").FullPath, std::chrono::milliseconds(300))
{
	InitUI();
	m_assetBrowser.Start([](const std::string& path_to_watch, FileStatus status) -> void {
		// Process only regular files, all other file types are ignored
		if (!std::filesystem::is_regular_file(std::filesystem::path(path_to_watch)) && status != FileStatus::Deleted)
		{
			return;
		}

		switch (status) {
		case FileStatus::Created:
		{
			Logger::GetInstance().Log(Logger::LogType::Info, "File created: " + path_to_watch);
			TestEditorEvent evt;
			evt.Path = std::move(path_to_watch);
			evt.Queue();
		}
			break;
		case FileStatus::Modified:
			Logger::GetInstance().Log(Logger::LogType::Info, "File modified: " + path_to_watch);
			break;
		case FileStatus::Deleted:
			Logger::GetInstance().Log(Logger::LogType::Info, "File deleted: " + path_to_watch);
			break;
		default:
			Logger::GetInstance().Log(Logger::LogType::Error, "Error! Unknown file status.");
		}
		});

	std::vector<TypeId> events;
	events.push_back(TestEditorEvent::GetEventId());
	EventManager::GetInstance().RegisterReceiver(this, events);
}

void Havana::InitUI()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
	//io.MouseDrawCursor = true;

	ImVec4* colors = ImGui::GetStyle().Colors;
	colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
	colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.06f, 0.06f, 0.94f);
	colors[ImGuiCol_ChildBg] = ImVec4(1.00f, 1.00f, 1.00f, 0.00f);
	colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
	colors[ImGuiCol_Border] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
	colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.21f, 0.22f, 0.54f);
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.40f, 0.40f, 0.40f, 0.40f);
	colors[ImGuiCol_FrameBgActive] = ImVec4(0.18f, 0.18f, 0.18f, 0.67f);
	colors[ImGuiCol_TitleBg] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
	colors[ImGuiCol_TitleBgActive] = ImVec4(0.29f, 0.29f, 0.29f, 1.00f);
	colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
	colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
	colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
	colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
	colors[ImGuiCol_CheckMark] = ImVec4(0.94f, 0.94f, 0.94f, 1.00f);
	colors[ImGuiCol_SliderGrab] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
	colors[ImGuiCol_SliderGrabActive] = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
	colors[ImGuiCol_Button] = ImVec4(0.44f, 0.44f, 0.44f, 0.40f);
	colors[ImGuiCol_ButtonHovered] = ImVec4(0.46f, 0.47f, 0.48f, 1.00f);
	colors[ImGuiCol_ButtonActive] = ImVec4(0.42f, 0.42f, 0.42f, 1.00f);
	colors[ImGuiCol_Header] = ImVec4(0.70f, 0.70f, 0.70f, 0.31f);
	colors[ImGuiCol_HeaderHovered] = ImVec4(0.70f, 0.70f, 0.70f, 0.80f);
	colors[ImGuiCol_HeaderActive] = ImVec4(0.48f, 0.50f, 0.52f, 1.00f);
	colors[ImGuiCol_Separator] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
	colors[ImGuiCol_SeparatorHovered] = ImVec4(0.72f, 0.72f, 0.72f, 0.78f);
	colors[ImGuiCol_SeparatorActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
	colors[ImGuiCol_ResizeGrip] = ImVec4(0.91f, 0.91f, 0.91f, 0.25f);
	colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.81f, 0.81f, 0.81f, 0.67f);
	colors[ImGuiCol_ResizeGripActive] = ImVec4(0.46f, 0.46f, 0.46f, 0.95f);
	colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
	colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
	colors[ImGuiCol_PlotHistogram] = ImVec4(0.73f, 0.60f, 0.15f, 1.00f);
	colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
	colors[ImGuiCol_TextSelectedBg] = ImVec4(0.87f, 0.87f, 0.87f, 0.35f);
	colors[ImGuiCol_ModalWindowDarkening] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
	colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
	colors[ImGuiCol_NavHighlight] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
	colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
	//ImGui::StyleColorsDark();
	ImGuiStyle& style = ImGui::GetStyle();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}
	ImGui_ImplWin32_Init(Renderer->GetDevice().m_window);
	ImGui_ImplDX11_Init(Renderer->GetDevice().GetD3DDevice(), Renderer->GetDevice().GetD3DDeviceContext());

	Icons["Close"] = ResourceCache::GetInstance().Get<Moonlight::Texture>(FilePath("Assets/Havana/UI/Close.png"));
	Icons["BugReport"] = ResourceCache::GetInstance().Get<Moonlight::Texture>(FilePath("Assets/Havana/UI/BugReport.png"));
	Icons["Maximize"] = ResourceCache::GetInstance().Get<Moonlight::Texture>(FilePath("Assets/Havana/UI/Maximize.png"));
	Icons["Play"] = ResourceCache::GetInstance().Get<Moonlight::Texture>(FilePath("Assets/Havana/UI/Play.png"));
	Icons["Pause"] = ResourceCache::GetInstance().Get<Moonlight::Texture>(FilePath("Assets/Havana/UI/Pause.png"));
	Icons["Stop"] = ResourceCache::GetInstance().Get<Moonlight::Texture>(FilePath("Assets/Havana/UI/Stop.png"));

}

bool show_demo_window = false;
bool show_dockspace = true;
void Havana::NewFrame(std::function<void()> StartGameFunc, std::function<void()> PauseGameFunc, std::function<void()> StopGameFunc)
{
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	ImGuiIO& io = ImGui::GetIO();

	// Output size fix?
	RenderSize = Vector2(io.DisplaySize.x, io.DisplaySize.y);
	Renderer->GetDevice().SetOutputSize(RenderSize);

	DrawMainMenuBar(StartGameFunc, PauseGameFunc, StopGameFunc);
	DrawOpenFilePopup();

	static float f = 0.0f;
	ImGuiViewport* viewport = ImGui::GetMainViewport();
	MainMenuSize.x = 0.f;
	MainMenuSize.y = 17.f;
	ImVec2 DockSize = viewport->Size;
	DockSize.y = viewport->Size.y - MainMenuSize.y;
	ImVec2 DockPos = viewport->Pos;
	DockPos.y = viewport->Pos.y + MainMenuSize.y;

	ImGui::SetNextWindowPos(DockPos);
	ImGui::SetNextWindowSize(DockSize);
	ImGui::SetNextWindowViewport(viewport->ID);
	ImGui::SetNextWindowBgAlpha(0.0f);

	ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
	window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
	window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::Begin("DockSpace Demo", &show_dockspace, window_flags);
	ImGui::PopStyleVar(3);

	ImGuiID dockspace_id = ImGui::GetID("MyDockspace");
	ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;
	ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);

	ImGui::End();

	DrawLog();
	m_assetBrowser.Draw();
}

void Havana::DrawOpenFilePopup()
{
	if (OpenScene)
	{
		ImGui::OpenPopup("Open Scene");
	}
	if (ImGui::BeginPopupModal("Open Scene", &OpenScene, ImGuiWindowFlags_MenuBar))
	{
		BrowseDirectory(FilePath("Assets"));

		for (auto& j : AssetDirectory)
		{
			FilePath path(j);
			ImGui::TreeNode(path.LocalPath.c_str());
			if (ImGui::IsItemClicked())
			{
				if (!j.is_string())
				{
					return;
				}
				LoadSceneEvent evt;
				evt.Level = path.LocalPath;
				evt.Fire();
				OpenScene = false;
				ImGui::CloseCurrentPopup();
			}
		}

		if (ImGui::Button("Close"))
		{
			OpenScene = false;
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}

void Havana::DrawMainMenuBar(std::function<void()> StartGameFunc, std::function<void()> PauseGameFunc, std::function<void()> StopGameFunc)
{
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 12.f));
	if (ImGui::BeginMainMenuBar())
	{
		MainMenuSize = ImGui::GetWindowSize();
		//MainMenuSize.y = 25.f;
		ImGui::SetWindowSize(MainMenuSize);
		ImGui::PopStyleVar(1);
		if (ImGui::BeginMenu("File"))
		{
			//ImGui::MenuItem("(dummy menu)", NULL, false, false);
			if (ImGui::MenuItem("New Scene"))
			{
				NewSceneEvent evt;
				evt.Queue();
			}
			if (ImGui::MenuItem("Open Scene", "Ctrl+O"))
			{
				OpenScene = true;
			}

			if (ImGui::BeginMenu("Open Recent"))
			{
				ImGui::MenuItem("fish_hat.c");
				ImGui::MenuItem("fish_hat.inl");
				ImGui::MenuItem("fish_hat.h");
				if (ImGui::BeginMenu("More.."))
				{
					ImGui::MenuItem("Hello");
					ImGui::MenuItem("Sailor");
					if (ImGui::BeginMenu("Recurse.."))
					{
						//ShowExampleMenuFile();
						ImGui::EndMenu();
					}
					ImGui::EndMenu();
				}
				ImGui::EndMenu();
			}
			if (ImGui::MenuItem("Save", "Ctrl+S"))
			{
				SaveSceneEvent evt;
				evt.Fire();
			}
			if (ImGui::MenuItem("Save As..")) {}
			ImGui::Separator();
			if (ImGui::BeginMenu("Options"))
			{
				static bool enabled = true;
				ImGui::MenuItem("Enabled", "", &enabled);
				ImGui::BeginChild("child", ImVec2(0, 60), true);
				for (int i = 0; i < 10; i++)
					ImGui::Text("Scrolling Text %d", i);
				ImGui::EndChild();
				static float f = 0.5f;
				static int n = 0;
				static bool b = true;
				ImGui::SliderFloat("Value", &f, 0.0f, 1.0f);
				ImGui::InputFloat("Input", &f, 0.1f);
				ImGui::Combo("Combo", &n, "Yes\0No\0Maybe\0\0");
				ImGui::Checkbox("Check", &b);
				ImGui::EndMenu();
			}
			if (ImGui::MenuItem("Quit", "Alt+F4")) {}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Edit"))
		{
			if (ImGui::MenuItem("Undo", "CTRL+Z")) {}
			if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {}  // Disabled item
			ImGui::Separator();
			if (ImGui::MenuItem("Cut", "CTRL+X")) {}
			if (ImGui::MenuItem("Copy", "CTRL+C")) {}
			if (ImGui::MenuItem("Paste", "CTRL+V")) {}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Add"))
		{
			if (ImGui::MenuItem("Entity"))
			{
				SelectedEntity = m_engine->GetWorld().lock()->CreateEntity().lock().get();
				SelectedEntity->AddComponent<Transform>("New Entity");
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Help"))
		{
			if (ImGui::MenuItem("Show ImGui Demo"))
			{
				show_demo_window = !show_demo_window;
			}
			ImGui::EndMenu();
		}

		if (ImGui::ImageButton(Icons["Play"]->CubesTexture, ImVec2(30.f, 30.f)) || Input::GetInstance().IsKeyDown(KeyCode::F5))
		{
			StartGameFunc();
		}

		if (m_engine->IsGameRunning())
		{
			if (ImGui::ImageButton(Icons["Pause"]->CubesTexture, ImVec2(30.f, 30.f)) || Input::GetInstance().IsKeyDown(KeyCode::F10))
			{
				PauseGameFunc();
			}

			if (ImGui::ImageButton(Icons["Stop"]->CubesTexture, ImVec2(30.f, 30.f)) || (Input::GetInstance().IsKeyDown(KeyCode::F5) && Input::GetInstance().IsKeyDown(KeyCode::LeftShift)))
			{
				StopGameFunc();
			}
		}

		float endOfMenu = ImGui::GetCursorPosX();
		float buttonWidth = 40.f;
		float pos = (ImGui::GetMousePos().x - m_engine->GetWindow()->GetPosition().X());
		static_cast<D3D12Window*>(m_engine->GetWindow())->CanMoveWindow((pos > endOfMenu && pos < ImGui::GetWindowWidth() - (buttonWidth * 3.f)));

		ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);

		ImGui::SetCursorPosX((ImGui::GetWindowWidth() / 2.f) - (ImGui::CalcTextSize(WindowTitle.c_str()).x / 2.f));
		ImGui::Text(WindowTitle.c_str());

		ImGui::BeginGroup();
		ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.0f, 0.6f, 0.6f, 0.f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.f, 0.8f, 0.8f, 0.f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.35f, 126.f, 43.f, 1.f));
		ImGui::SetCursorPosX(ImGui::GetWindowWidth() - (buttonWidth * 3.f));
		if (ImGui::ImageButton(Icons["BugReport"]->CubesTexture, ImVec2(30.f, 30.f)))
		{
			ShellExecute(0, 0, L"https://github.com/wobbier/MitchEngine/issues", 0, 0, SW_SHOW);
		}
		ImGui::PopStyleColor(1);

		ImGui::SetCursorPosX(ImGui::GetWindowWidth() - (buttonWidth * 2.f));
		if (ImGui::ImageButton(Icons["Maximize"]->CubesTexture, ImVec2(30.f, 30.f)))
		{
			m_engine->GetWindow()->Maximize();
		}

		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(1.f, 42.f, 43.f, 1.f));
		ImGui::SetCursorPosX(ImGui::GetWindowWidth() - buttonWidth);
		//ImGui::SameLine(0.f);
		if (ImGui::ImageButton(Icons["Close"]->CubesTexture, ImVec2(30.f, 30.f)))
		{
			m_engine->StopGame();
		}
		ImGui::PopStyleColor(3);
		bool hoveringButtons = ImGui::IsMouseHoveringWindow();
		ImGui::EndGroup();

		ImGui::EndMainMenuBar();
	}
}

void Havana::DrawLog()
{
	static std::unordered_map<Logger::LogType, bool> DebugFilters;
	ImGuiWindowFlags window_flags = 0;
	window_flags |= ImGuiWindowFlags_MenuBar;
	bool showLog = true;
	ImGui::Begin("Log", &showLog, window_flags);
	// Menu
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("Show"))
		{
			ImGui::Checkbox("Debug", &DebugFilters[Logger::LogType::Debug]);
			ImGui::Checkbox("Error", &DebugFilters[Logger::LogType::Error]);
			ImGui::Checkbox("Info", &DebugFilters[Logger::LogType::Info]);
			ImGui::EndMenu();
		}

		if (ImGui::Button("Clear Log"))
		{
			Logger::Messages.clear();
		}

		ImGui::EndMenuBar();
	}

	bool showMessage = true;
	for (auto& msg : Logger::Messages)
	{
		if (DebugFilters.find(msg.Type) == DebugFilters.end())
		{
			DebugFilters[msg.Type] = true;
		}
		if (DebugFilters[msg.Type])
		{
			ImGui::Text(msg.Message.c_str());
		}
	}

	ImGui::End();
}

void Havana::AddComponentPopup()
{
	// Simple selection popup
	// (If you want to show the current selection inside the Button itself, you may want to build a string using the "###" operator to preserve a constant ID with a variable label)
	ImGui::PushItemWidth(-100);
	if (ImGui::Button("Add Component.."))
	{
		ImGui::OpenPopup("my_select_popup");
	}
	ImGui::PopItemWidth();

	if (ImGui::BeginPopup("my_select_popup"))
	{
		ImGui::Text("Components");
		ImGui::Separator();

		ComponentRegistry& reg = GetComponentRegistry();

		for (auto& thing : reg)
		{
			if (ImGui::Selectable(thing.first.c_str()))
			{
				if (SelectedEntity)
				{
					SelectedEntity->AddComponentByName(thing.first);
				}
				if (SelectedTransform)
				{
					m_engine->GetWorld().lock()->GetEntity(SelectedTransform->Parent)->AddComponentByName(thing.first);
				}
			}
		}
		ImGui::EndPopup();
	}
}

void Havana::UpdateWorld(World * world, Transform * root, const std::vector<Entity> & ents)
{
	ImGui::Begin("World");
	if (ImGui::CollapsingHeader("Scene", ImGuiTreeNodeFlags_DefaultOpen))
	{
		UpdateWorldRecursive(root);
	}
	if (ents.size() > 0)
	{
		if (ImGui::CollapsingHeader("Utility", ImGuiTreeNodeFlags_DefaultOpen))
		{
			int i = 0;
			for (const Entity& ent : ents)
			{
				for (BaseComponent* comp : ent.GetAllComponents())
				{
					ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | (SelectedEntity == &ent ? ImGuiTreeNodeFlags_Selected : 0);
					{
						node_flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen; // ImGuiTreeNodeFlags_Bullet
						ImGui::TreeNodeEx((void*)(intptr_t)i, node_flags, comp->GetName().c_str());
						if (ImGui::IsItemClicked())
						{
							SelectedCore = nullptr;
							SelectedTransform = nullptr;
							SelectedEntity = &const_cast<Entity&>(ent);
						}
					}
				}
			}
		}
	}

	if (ImGui::CollapsingHeader("Entity Cores", ImGuiTreeNodeFlags_DefaultOpen))
	{
		int i = 0;
		for (BaseCore* comp : world->GetAllCores())
		{
			ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | (SelectedCore == comp ? ImGuiTreeNodeFlags_Selected : 0);
			{
				node_flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen; // ImGuiTreeNodeFlags_Bullet
				ImGui::TreeNodeEx((void*)(intptr_t)i, node_flags, comp->GetName().c_str());
				if (ImGui::IsItemClicked())
				{
					SelectedCore = comp;
					SelectedTransform = nullptr;
					SelectedEntity = nullptr;
				}
			}
		}
	}
	ImGui::End();

	ImGui::Begin("Properties");

	Entity* entity = SelectedEntity;
	if (SelectedTransform != nullptr)
	{
		entity = world->GetEntity(SelectedTransform->Parent);
	}

	if (entity)
	{
		for (BaseComponent* comp : entity->GetAllComponents())
		{
			if (ImGui::CollapsingHeader(comp->GetName().c_str(), ImGuiTreeNodeFlags_DefaultOpen))
			{
				comp->OnEditorInspect();
			}
		}
		AddComponentPopup();
	}

	if (SelectedCore != nullptr)
	{
		SelectedCore->OnEditorInspect();
	}
	ImGui::End();
}

void Havana::UpdateWorldRecursive(Transform * root)
{
	if (!root)
		return;
	int i = 0;
	for (Transform* var : root->Children)
	{
		ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | (SelectedTransform == var ? ImGuiTreeNodeFlags_Selected : 0);
		if (var->Children.empty())
		{
			node_flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen; // ImGuiTreeNodeFlags_Bullet
			ImGui::TreeNodeEx((void*)(intptr_t)i, node_flags, var->Name.c_str());
			if (ImGui::IsItemClicked())
			{
				SelectedTransform = var;
				SelectedCore = nullptr;
				SelectedEntity = m_engine->GetWorld().lock()->GetEntity(SelectedTransform->Parent);
			}
		}
		else
		{
			bool node_open = ImGui::TreeNodeEx((void*)(intptr_t)i, node_flags, var->Name.c_str());
			if (ImGui::IsItemClicked())
			{
				SelectedTransform = var;
				SelectedCore = nullptr;
				SelectedEntity = m_engine->GetWorld().lock()->GetEntity(SelectedTransform->Parent);
			}

			if (node_open)
			{
				UpdateWorldRecursive(var);
				ImGui::TreePop();
			}
		}

		i++;
	}
}

void Havana::Render()
{
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::Begin("Game");
	{
		m_isGameFocused = ImGui::IsWindowFocused();


		if (Renderer->RTT->shaderResourceViewMap != nullptr)
		{
			// Get the current cursor position (where your window is)
			ImVec2 pos = ImGui::GetCursorScreenPos();
			ImVec2 maxPos = ImVec2(pos.x + ImGui::GetWindowSize().x, pos.y + ImGui::GetWindowSize().y);
			//GameRenderSize = Vector2(ImGui::GetWindowSize().x, ImGui::GetWindowSize().y);
			GameRenderSize = Vector2(ImGui::GetWindowSize().x, ImGui::GetWindowSize().y);

			// Ask ImGui to draw it as an image:
			// Under OpenGL the ImGUI image type is GLuint
			// So make sure to use "(void *)tex" but not "&tex"
			ImGui::GetWindowDrawList()->AddImage(
				(void*)Renderer->RTT->shaderResourceViewMap,
				ImVec2(pos.x, pos.y),
				ImVec2(maxPos),
				ImVec2(0, 0),
				ImVec2(Mathf::Clamp(0.f, 1.0f, GameRenderSize.X() / Renderer->RTT->Size.X()), Mathf::Clamp(0.f, 1.0f, GameRenderSize.Y() / Renderer->RTT->Size.Y())));
			//ImVec2(WorldViewRenderSize.X() / RenderSize.X(), WorldViewRenderSize.Y() / RenderSize.Y()));
		}
	}
	ImGui::End();

	ImGui::Begin("World View");
	{
		m_isWorldViewFocused = ImGui::IsWindowFocused();


		if (Renderer->RTT2->shaderResourceViewMap != nullptr)
		{
			// Get the current cursor position (where your window is)
			ImVec2 pos = ImGui::GetCursorScreenPos();
			ImVec2 maxPos = ImVec2(pos.x + ImGui::GetWindowSize().x, pos.y + ImGui::GetWindowSize().y);
			WorldViewRenderSize = Vector2(ImGui::GetWindowSize().x, ImGui::GetWindowSize().y);
			WorldViewRenderLocation = Vector2(pos.x, pos.y);
			// Ask ImGui to draw it as an image:
			// Under OpenGL the ImGUI image type is GLuint
			// So make sure to use "(void *)tex" but not "&tex"
			ImGui::GetWindowDrawList()->AddImage(
				(void*)Renderer->RTT2->shaderResourceViewMap,
				ImVec2(pos.x, pos.y),
				ImVec2(maxPos),
				ImVec2(0, 0),
				ImVec2(Mathf::Clamp(0.f, 1.0f, WorldViewRenderSize.X() / Renderer->RTT2->Size.X()), Mathf::Clamp(0.f, 1.0f, WorldViewRenderSize.Y() / Renderer->RTT2->Size.Y())));
			//ImVec2(WorldViewRenderSize.X() / RenderSize.X(), WorldViewRenderSize.Y() / RenderSize.Y()));
		}
	}
	ImGui::End();
	ImGui::PopStyleVar(3);

	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	// Update and Render additional Platform Windows
	if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}
}

const bool Havana::IsGameFocused() const
{
	return m_isGameFocused;
}

const bool Havana::IsWorldViewFocused() const
{
	return m_isWorldViewFocused;
}

void Havana::Text(const std::string & Name, const Vector3 & Vector)
{
	ImGui::Text(Name.c_str());

	ImGui::Text("X: %f", Vector.X());
	ImGui::SameLine();
	ImGui::Text("Y: %f", Vector.Y());
	ImGui::SameLine();
	ImGui::Text("Z: %f", Vector.Z());
}

void Havana::Text(const std::string & Name, const Vector2 & Vector)
{
	ImGui::Text(Name.c_str());

	ImGui::Text("X: %f", Vector.X());
	ImGui::SameLine();
	ImGui::Text("Y: %f", Vector.Y());
}

void Havana::EditableVector3(const std::string & Name, Vector3 & Vector)
{
	ImGui::DragFloat3(Name.c_str(), &Vector[0], 0.005f);
}

void Havana::BrowseDirectory(const FilePath & path)
{
	if (CurrentDirectory.FullPath == path.FullPath && !AssetDirectory.empty())
	{
		return;
	}

	for (const auto& entry : fs::directory_iterator(path.FullPath))
	{
		FilePath filePath(entry.path().string());
		if (filePath.LocalPath.find(".lvl") != std::string::npos)
		{
			AssetDirectory.push_back(filePath.FullPath);
		}

		ImGui::Text(filePath.Directory.c_str());

		ImGui::Text(filePath.LocalPath.c_str());
	}
}

const Vector2& Havana::GetGameOutputSize() const
{
	return GameRenderSize;
}

bool Havana::OnEvent(const BaseEvent& evt)
{

	if (evt.GetEventId() == TestEditorEvent::GetEventId())
	{
		const TestEditorEvent& test = static_cast<const TestEditorEvent&>(evt);
		Logger::GetInstance().Log(Logger::LogType::Info, "We did it fam" + test.Path);
		return true;
	}
	return false;
}

#endif