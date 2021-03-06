#include "PCH.h"
#include "Engine.h"
#include "CLog.h"
#include "Config.h"
#include "Window/UWPWindow.h"
#include "Window/Win32Window.h"
#include "Events/EventManager.h"
#include "Cores/PhysicsCore.h"
#include "Cores/Cameras/CameraCore.h"
#include "Cores/SceneGraph.h"
#include "Cores/Rendering/RenderCore.h"
#include "Game.h"
#include "Window/IWindow.h"
#include "Input.h"
#include "Havana.h"
#include "Components/Transform.h"
#include "Dementia.h"
#include "Events/SceneEvents.h"
#include "Components/Camera.h"
#include "Components/Cameras/FlyingCamera.h"
#include "Cores/Cameras/FlyingCameraCore.h"
#include "Cores/AudioCore.h"
#include "Cores/UI/UICore.h"

#if ME_EDITOR
#include "Utils/StringUtils.h"
#include <fileapi.h>
#endif
#include "Resource/ResourceCache.h"
#include "optick.h"
#include "Work/Burst.h"
#include "Profiling/BasicFrameProfile.h"

Engine& GetEngine()
{
	return Engine::GetInstance();
}

Engine::Engine()
	: Running(true)
{
	std::vector<TypeId> events;
	events.push_back(LoadSceneEvent::GetEventId());
	EventManager::GetInstance().RegisterReceiver(this, events);
}

Engine::~Engine()
{
	delete EngineConfig;
}

void Engine::Init(Game* game)
{
	if (m_isInitialized || !game)
	{
		return;
	}

	m_game = game;

	CLog::GetInstance().SetLogFile("Engine.txt");
	CLog::GetInstance().SetLogPriority(CLog::LogType::Info);
	CLog::GetInstance().Log(CLog::LogType::Info, "Starting the MitchEngine.");
	Path engineCfg("Assets\\Config\\Engine.cfg");

#if ME_EDITOR
	if (engineCfg.FullPath.rfind("Engine") != -1)
	{
		Path gameEngineCfgPath("Assets\\Config\\Engine.cfg", true);
		if (!gameEngineCfgPath.Exists)
		{
			CreateDirectory(StringUtils::ToWString(gameEngineCfgPath.Directory).c_str(), NULL);
			File gameEngineCfg = File(engineCfg);
			File newGameConfig(gameEngineCfgPath);
			newGameConfig.Write(gameEngineCfg.Read());
		}
	}
#endif

	EngineConfig = new Config(engineCfg);

	burst.InitializeWorkerThreads();

#if ME_PLATFORM_WIN64
	const json& WindowConfig = EngineConfig->GetObject("Window");
	int WindowWidth = WindowConfig["Width"];
	int WindowHeight = WindowConfig["Height"];
	std::function<void(const Vector2&)> Func = [this](const Vector2& NewSize)
	{
		if (m_renderer)
		{
			m_renderer->WindowResized(NewSize);
		}
		if (UI)
		{
			if (Camera::CurrentCamera)
			{
				UI->OnResize(Camera::CurrentCamera->OutputSize);
			}
		}
	};
	GameWindow = new Win32Window(EngineConfig->GetValue("Title"), Func, 500, 300, Vector2(WindowWidth, WindowHeight));
#endif
#if ME_PLATFORM_UWP
	GameWindow = new UWPWindow("MitchEngine", 1920, 1080);
#endif

	m_renderer = new Moonlight::Renderer();
	m_renderer->WindowResized(GameWindow->GetSize());

	GameWorld = std::make_shared<World>();

	Cameras = new CameraCore();

	SceneNodes = new SceneGraph();

	ModelRenderer = new RenderCore();
	AudioThread = new AudioCore();

	m_renderer->Init();

	UI = new UICore(GameWindow, m_renderer);

	InitGame();

	m_isInitialized = true;
}

void Engine::InitGame()
{
	GameWorld->AddCore<CameraCore>(*Cameras);
	GameWorld->AddCore<SceneGraph>(*SceneNodes);
	GameWorld->AddCore<RenderCore>(*ModelRenderer);
	GameWorld->AddCore<AudioCore>(*AudioThread);
	GameWorld->AddCore<UICore>(*UI);

	m_game->OnInitialize();
}

void Engine::StopGame()
{
	m_game->OnEnd();
}

void Engine::Run()
{
	m_game->OnStart();

	GameClock.Reset();
	float lastTime = GameClock.GetTimeInMilliseconds();

	const float FramesPerSec = FPS;
	const float MaxDeltaTime = (1.f / FramesPerSec);
	// Game loop
	forever
	{
		FrameProfile::GetInstance().Start();
		// Check and call events
		GameWindow->ParseMessageQueue();

		if (GameWindow->ShouldClose())
		{
			StopGame();
			break;
		}

		EventManager::GetInstance().FirePendingEvents();

		GameClock.Update();

		AccumulatedTime += GameClock.GetDeltaSeconds();
		if (AccumulatedTime >= MaxDeltaTime)
		{
			OPTICK_FRAME("MainLoop");
			float deltaTime = DeltaTime = AccumulatedTime;

			FrameProfile::GetInstance().Set("Physics", ProfileCategory::Physics);
			GameWorld->Simulate();

			m_input.Update();

			// Update our engine
			GameWorld->UpdateLoadedCores(deltaTime);
			FrameProfile::GetInstance().Complete("Physics");

			FrameProfile::GetInstance().Set("SceneNodes", ProfileCategory::UI);
			SceneNodes->Update(deltaTime);
			Cameras->Update(0.0f);
			FrameProfile::GetInstance().Complete("SceneNodes");

			{
				FrameProfile::GetInstance().Set("Game", ProfileCategory::Game);
				//ME_PROFILE("Game", ProfileCategory::Game);
				OPTICK_CATEGORY("MainLoop::GameUpdate", Optick::Category::GameLogic);
				m_game->OnUpdate(deltaTime);
				FrameProfile::GetInstance().Complete("Game");
			}
			
			FrameProfile::GetInstance().Set("ModelRenderer", ProfileCategory::Rendering);
			AudioThread->Update(deltaTime);
			ModelRenderer->Update(deltaTime);
			FrameProfile::GetInstance().Complete("ModelRenderer");

			{
				OPTICK_CATEGORY("UICore::Update", Optick::Category::Rendering)
				FrameProfile::GetInstance().Set("UI", ProfileCategory::UI);
				// editor only?
				if (UI)
				{
					if (Camera::CurrentCamera)
					{
						UI->OnResize(Camera::CurrentCamera->OutputSize);
					}
				}
				UI->Update(deltaTime);
				FrameProfile::GetInstance().Complete("UI");
			}
//
//#if !ME_EDITOR
//			Vector2 MainOutputSize = m_renderer->GetDevice().GetOutputSize();
//			MainCamera.Position = Camera::CurrentCamera->Position;
//			MainCamera.Front = Camera::CurrentCamera->Front;
//			MainCamera.OutputSize = MainOutputSize;
//			MainCamera.FOV = Camera::CurrentCamera->GetFOV();
//			MainCamera.Skybox = Camera::CurrentCamera->Skybox;
//			MainCamera.ClearColor = Camera::CurrentCamera->ClearColor;
//			MainCamera.ClearType = Camera::CurrentCamera->ClearType;
//			MainCamera.Projection = Camera::CurrentCamera->Projection;
//			MainCamera.OrthographicSize = Camera::CurrentCamera->OrthographicSize;
//
//			EditorCamera = MainCamera;
//#endif

			FrameProfile::GetInstance().Set("Render", ProfileCategory::Rendering);
			m_renderer->ThreadedRender([this]() {
				m_game->PostRender();
			}, [this]() {
				UI->Render();
			}, EditorCamera);
			FrameProfile::GetInstance().Complete("Render");

			// This makes the profiler overview data to be delayed for a frame, but takes the renderer into account.
			static float fpsTime = 0;
			fpsTime += AccumulatedTime;
			if (fpsTime > 1.f)
			{
				FrameProfile::GetInstance().Dump();
				fpsTime -= 1.f;
			}

			AccumulatedTime = std::fmod(AccumulatedTime, MaxDeltaTime);

			FrameProfile::GetInstance().End(AccumulatedTime);
		}

		ResourceCache::GetInstance().Dump();
		//Sleep(1);
	}
	EngineConfig->Save();
}

bool Engine::OnEvent(const BaseEvent& evt)
{
	if (evt.GetEventId() == LoadSceneEvent::GetEventId())
	{
		const LoadSceneEvent& test = static_cast<const LoadSceneEvent&>(evt);
		//InputEnabled = test.Enabled;
		LoadScene(test.Level);
	}

	return false;
}

Moonlight::Renderer& Engine::GetRenderer() const
{
	return *m_renderer;
}

std::weak_ptr<World> Engine::GetWorld() const
{
	return GameWorld;
}

const bool Engine::IsInitialized() const
{
	return m_isInitialized;
}

bool Engine::IsRunning() const
{
	return true;
}

void Engine::Quit() { Running = false; }

IWindow* Engine::GetWindow()
{
	return GameWindow;
}

Game* Engine::GetGame() const
{
	return m_game;
}

Config& Engine::GetConfig() const
{
	return *EngineConfig;
}

Input& Engine::GetInput()
{
	return m_input;
}

Burst& Engine::GetBurstWorker()
{
	return burst;
}

void Engine::LoadScene(const std::string& SceneFile)
{
	Cameras->Init();
	if (CurrentScene)
	{
		CurrentScene->UnLoad();
		delete CurrentScene;
		CurrentScene = nullptr;
	}

	GameWorld->Unload();
	SceneNodes->Init();
	CurrentScene = new Scene(SceneFile);

	if (!CurrentScene->Load(GameWorld))
	{
	}

	SceneLoadedEvent evt;
	evt.LoadedScene = CurrentScene;
	evt.Fire();
#if !ME_EDITOR
	//GameWorld->Start();
#endif
}
