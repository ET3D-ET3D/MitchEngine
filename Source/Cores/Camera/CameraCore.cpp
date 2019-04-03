#include "PCH.h"
#include "CameraCore.h"
#include "Components/Camera.h"

CameraCore::CameraCore() : Base(ComponentFilter().Requires<Camera>().Requires<Transform>())
{
}

CameraCore::~CameraCore()
{
}

void CameraCore::Init()
{
	Entity CameraEntity = GetWorld().CreateEntity();
	DefaultCamera = &CameraEntity.AddComponent<Camera>();
}

void CameraCore::Update(float dt)
{
	BROFILER_CATEGORY("CameraCore::Update", Brofiler::Color::CornflowerBlue);
	auto Cameras = GetEntities();

	Camera* currentCam = nullptr;

	for (auto& InEntity : Cameras)
	{
		// Get our components that our Core required
		Camera& CameraComponent = InEntity.GetComponent<Camera>();
		Transform& TransformComponent = InEntity.GetComponent<Transform>();

		if (CameraComponent.IsCurrent())
		{
			currentCam = &CameraComponent;
			CameraComponent.UpdateCameraTransform(TransformComponent.GetPosition());
		}
	}

	if (currentCam == nullptr)
	{
		DefaultCamera->SetCurrent();
	}
}