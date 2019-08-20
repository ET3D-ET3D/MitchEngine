#include "PCH.h"
#include "UICore.h"
#include "Components/Transform.h"
#include "ECS/ComponentFilter.h"
#include "Logger.h"
#include "Graphics/ShaderCommand.h"
#include "Resource/Resource.h"

#include <iostream>
#include "Resource/ResourceCache.h"
#include "RenderCommands.h"
#include "Engine/Engine.h"
#include "Components/UI/Text.h"
#include "Utils/StringUtils.h"

//#include "UI/FileSystemWin.h"
//#include "UI/d3d11/GPUDriverD3D11.h"
#include <Ultralight/platform/Platform.h>
#include <Ultralight/platform/Config.h>
#include "UI/UIWindow.h"
#include "AppCore/Overlay.h"
#include "UI2/FileSystemWin.h"
#include <Shlwapi.h>
#include "UI2/d3d11/GPUDriverD3D11.h"
#include "UI2/d3d11/GPUContextD3D11.h"
#include "UI/OverlayImpl.h"
#include "File.h"
#include <filesystem>

UICore::UICore(IWindow* window)
	: Base(ComponentFilter().Requires<Transform>().RequiresOneOf<Text>().RequiresOneOf<UIView>())
	, m_uiRenderer(ultralight::Renderer::Create())
{
	IsSerializable = false;
	m_renderer = &GetEngine().GetRenderer();

	m_window = AdoptRef(*new UIWindow(window, GetOverlayManager()));
	ultralight::Platform& platform = ultralight::Platform::instance();
	ultralight::Config config_;
	config_.device_scale_hint = 1.0f;
	config_.face_winding = ultralight::FaceWinding::kFaceWinding_Clockwise;

	HMODULE hModule = GetModuleHandleW(NULL);
	WCHAR path[MAX_PATH];
	GetModuleFileNameW(hModule, path, MAX_PATH);
	PathRemoveFileSpecW(path);

	PathAppendW(path, L"Assets/UI");

	m_fs.reset(new ultralight::FileSystemWin(path));
	m_fontLoader.reset(new ultralight::FontLoaderWin());

	m_context.reset(new ultralight::GPUContextD3D11(m_renderer));

	platform.set_config(config_);


	UIWindow* win = static_cast<UIWindow*>(m_window.get());
	if (!m_context->Initialize(win->hwnd(), win->width(),
		win->height(), win->scale(), win->is_fullscreen(), true, false, 1))
	{
		MessageBoxW(NULL, (LPCWSTR)L"Failed to initialize D3D11 context", (LPCWSTR)L"Notification", MB_OK);
		exit(-1);
	}

	m_driver.reset(new ultralight::GPUDriverD3D11(m_context.get()));

	platform.set_gpu_driver(m_driver.get());
	platform.set_file_system(m_fs.get());
	platform.set_font_loader(m_fontLoader.get());
	//win->set_app_listener(this);

}

UICore::~UICore()
{
	Logger::GetInstance().Log(Logger::LogType::Debug, "UICore Destroyed...");
}

void UICore::Init()
{
	Logger::GetInstance().Log(Logger::LogType::Debug, "UICore Initialized...");
	if (m_renderer)
		m_renderer->ClearUIText();
	if (!IsInitialized)
	{
		IsInitialized = true;
		//ultralight::Ref<ultralight::View> view = m_uiRenderer->CreateView(GetEngine().MainCamera.OutputSize.X(), GetEngine().MainCamera.OutputSize.Y(), true);
		//ultralight::RefPtr<ultralight::Overlay> ref = ultralight::Overlay::Create(*m_window.get(), view, 0, 0);//AdoptRef(*new ultralight::OverlayImpl(*m_window.get(), view, 0, 0));//
		//ultralight::OverlayImpl* impl = static_cast<ultralight::OverlayImpl*>(ref.get());
		//File testFile = File(Path("Assets/UI/Test.html"));
		//impl->view()->LoadHTML(testFile.Read().c_str());
		////impl->view()->LoadURL("https://wobbier.com/");
		//m_overlays.push_back(ref);
		//GetOverlayManager()->Add(m_overlays[0].get());
	}
}

void UICore::OnEntityAdded(Entity & NewEntity)
{
	if (NewEntity.HasComponent<Text>())
	{
		Moonlight::TextCommand command;
		Text& textComponent = NewEntity.GetComponent<Text>();
		command.SourceText = StringUtils::ToWString(textComponent.SourceText);
		textComponent.RenderId = m_renderer->PushUIText(command);
	}

	if (NewEntity.HasComponent<UIView>())
	{
		UIView& view = NewEntity.GetComponent<UIView>();

		InitUIView(view);
	}
}

void UICore::OnEntityRemoved(Entity & InEntity)
{
	if (InEntity.HasComponent<Text>())
	{
		Text& textComponent = InEntity.GetComponent<Text>();
		m_renderer->PopUIText(textComponent.RenderId);
	}
	if (InEntity.HasComponent<UIView>())
	{
		UIView view = InEntity.GetComponent<UIView>();

		GetOverlayManager()->Remove(m_overlays[view.Index].get());

		m_overlays.erase(m_overlays.begin() + view.Index);
	}
}

void UICore::Update(float dt)
{
	// Update internal logic (timers, event callbacks, etc.)
	m_uiRenderer->Update();

	OPTICK_CATEGORY("UICore::Update", Optick::Category::Rendering)

		auto Renderables = GetEntities();
	for (auto& InEntity : Renderables)
	{
		Transform& transform = InEntity.GetComponent<Transform>();

		if (InEntity.HasComponent<Text>())
		{
			Text& textComponent = InEntity.GetComponent<Text>();

			Moonlight::TextCommand command;
			command.SourceText = StringUtils::ToWString(textComponent.SourceText);
			command.ScreenPosition = Vector2(transform.Position.X(), transform.Position.Y());
			command.Anchor = textComponent.Anchor;
			m_renderer->UpdateText(textComponent.RenderId, command);
		}
		if (InEntity.HasComponent<UIView>())
		{
			UIView& view = InEntity.GetComponent<UIView>();
			if (!view.IsInitialized)
			{
				InitUIView(view);
			}
		}
	}
}

void UICore::OnStop()
{
	m_renderer->ClearUIText();
}

void UICore::Render()
{
	m_driver->BeginSynchronize();

	// Render all active views to command lists and dispatch calls to GPUDriver
	m_uiRenderer->Render();

	m_driver->EndSynchronize();

	// Draw any pending commands to screen
	if (m_driver->HasCommandsPending())
	{
		m_context->BeginDrawing();
		m_driver->DrawCommandList();

		// Perform any additional drawing (Overlays) here...
		//DrawOverlays();

		// Flip buffers here.
		if (m_window)
		{
			Draw();
		}
		//m_context->PresentFrame();
		m_context->EndDrawing();
	}
}

ultralight::OverlayManager* UICore::GetOverlayManager()
{
	return this;
}

void UICore::OnResize(const Vector2 & NewSize)
{
	if (m_context)
	{
		m_context->Resize((int)NewSize.X(), (int)NewSize.Y());
		//m_driver->RebindBackbuffer();
		for (auto overlay : overlays_)
		{
			overlay->Resize((int)NewSize.X(), (int)NewSize.Y());
		}
	}
}

void UICore::InitUIView(UIView & view)
{
	ultralight::Ref<ultralight::View> newView = m_uiRenderer->CreateView(GetEngine().MainCamera.OutputSize.X(), GetEngine().MainCamera.OutputSize.Y(), true);
	ultralight::RefPtr<ultralight::Overlay> overlay = ultralight::Overlay::Create(*m_window.get(), newView, 0, 0);
	overlay->view()->LoadHTML(view.SourceFile.Read().c_str());
	//impl->view()->LoadURL("https://wobbier.com/");
	m_overlays.push_back(overlay);
	GetOverlayManager()->Add(overlay.get());
	view.IsInitialized = true;
	view.Index = m_overlays.size() - 1;
}

UIView::UIView()
{

}

void UIView::Serialize(json & outJson)
{
}

void UIView::Deserialize(const json & inJson)
{
}

void UIView::OnEditorInspect()
{
	static std::vector<Path> Textures;
	if (Textures.empty())
	{
		Path path = Path("Assets");
		Textures.push_back(Path(""));
		for (const auto& entry : std::filesystem::recursive_directory_iterator(path.FullPath))
		{
			Path filePath(entry.path().string());
			if (filePath.LocalPath.rfind(".html") != std::string::npos && filePath.LocalPath.rfind(".meta") == std::string::npos)
			{
				Textures.push_back(filePath);
			}
		}
	}

	if (ImGui::BeginCombo("##HTMLSource", ""))
	{
		for (int n = 0; n < Textures.size(); n++)
		{
			if (ImGui::Selectable(Textures[n].LocalPath.c_str(), false))
			{
				FilePath = Textures[n];
				SourceFile = File(FilePath);
				IsInitialized = false;
				Textures.clear();
				break;
			}
		}
		ImGui::EndCombo();
	}
}

void UIView::Init()
{
	SourceFile = File(FilePath);
}
