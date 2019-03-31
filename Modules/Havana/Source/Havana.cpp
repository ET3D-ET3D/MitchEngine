#include "Havana.h"
#include "imgui.h"
#include "examples/imgui_impl_win32.h"
#include "examples/imgui_impl_dx11.h"
#include "Engine/World.h"
#include "Components/Transform.h"
#include <stack>
#include "Components/Camera.h"

#if ME_EDITOR

Havana::Havana(Moonlight::Renderer* renderer)
	: Renderer(renderer)
{
	InitUI();
}

void Havana::InitUI()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
	io.MouseDrawCursor = true;

	ImGui::StyleColorsDark();
	ImGuiStyle& style = ImGui::GetStyle();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}
	ImGui_ImplWin32_Init(Renderer->GetDevice().m_window);
	ImGui_ImplDX11_Init(Renderer->GetDevice().GetD3DDevice(), Renderer->GetDevice().GetD3DDeviceContext());
}

bool show_demo_window = true;
void Havana::NewFrame()
{
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	ImGui::BeginMainMenuBar();
	ImGui::MenuItem("File");
	ImGui::MenuItem("Add");
	ImGui::EndMainMenuBar();

	static float f = 0.0f;
	ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->Pos);
	ImGui::SetNextWindowSize(viewport->Size);
	ImGui::SetNextWindowViewport(viewport->ID);
	ImGui::SetNextWindowBgAlpha(0.0f);

	ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
	window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
	window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::Begin("DockSpace Demo", &show_demo_window, window_flags);
	ImGui::PopStyleVar(3);

	ImGuiID dockspace_id = ImGui::GetID("MyDockspace");
	ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;
	ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);

	ImGui::End();

	ImGui::Begin("Debug Info");

	ImGui::Checkbox("Demo Window", &show_demo_window);
	ImGui::ShowDemoWindow(&show_demo_window);

	ImGui::Text("FPS Average: %.3f FPS: (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	ImGui::End();
}

void Havana::UpdateWorld(World* world, Transform* root)
{
	ImGui::Begin("Scene View");
	UpdateWorldRecursive(root);
	ImGui::End();

	if (SelectedTransform != nullptr)
	{
		ImGui::Begin("Properties");
		Entity* entity = world->GetEntity(SelectedTransform->Parent);
		if (entity)
		{
			for (BaseComponent* comp : entity->GetAllComponents())
			{
				comp->OnEditorInspect();
			}
		}

		ImGui::End();
	}
}

void Havana::UpdateWorldRecursive(Transform* root)
{
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
			}
		}
		else
		{
			bool node_open = ImGui::TreeNodeEx((void*)(intptr_t)i, node_flags, var->Name.c_str());
			if (ImGui::IsItemClicked())
			{
				SelectedTransform = var;
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
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	// Update and Render additional Platform Windows
	if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}
}

void Havana::Text(const std::string& Name, const Vector3& Vector)
{
	ImGui::Text(Name.c_str());
	ImGui::Text("X: %f", Vector.X());
	ImGui::SameLine();
	ImGui::Text("Y: %f", Vector.Y());
	ImGui::SameLine();
	ImGui::Text("Z: %f", Vector.Z());
}

#endif