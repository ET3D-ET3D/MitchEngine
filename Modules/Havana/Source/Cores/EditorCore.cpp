#include <Cores/EditorCore.h>

#include <Components/Camera.h>
#include <Components/Graphics/Mesh.h>
#include <Components/Graphics/Model.h>
#include <Components/Transform.h>
#include <Cores/Rendering/RenderCore.h>
#include <Engine/Engine.h>
#include <Events/SceneEvents.h>
#include <Havana.h>
#include <HavanaEvents.h>
#include <Mathf.h>
#include <optick.h>
#include <Window/IWindow.h>
#include <World/Scene.h>

#if ME_EDITOR

EditorCore::EditorCore(Havana* editor)
	: Base(ComponentFilter().Excludes<Transform>())
	, m_editor(editor)
{
	SetIsSerializable(false);

	EditorCameraTransform = new Transform();
	EditorCamera = new Camera();

	std::vector<TypeId> events;
	events.push_back(SaveSceneEvent::GetEventId());
	events.push_back(NewSceneEvent::GetEventId());
	events.push_back(Moonlight::PickingEvent::GetEventId());
	EventManager::GetInstance().RegisterReceiver(this, events);
}

EditorCore::~EditorCore()
{
}

void EditorCore::Init()
{
	Camera::EditorCamera = EditorCamera;
}

void EditorCore::Update(float dt)
{
	OPTICK_CATEGORY("FlyingCameraCore::Update", Optick::Category::Camera);

	auto Keyboard = m_editor->GetInput().GetKeyboardState();
	auto Mouse = m_editor->GetInput().GetMouseState();

	if (m_editor->IsWorldViewFocused())
	{
		if (Keyboard.F)
		{
			IsFocusingTransform = true;

			// Keep a note of the time the movement started.
			startTime = 0.f;

			// Calculate the journey length.
			TravelDistance = (EditorCameraTransform->GetPosition() - Vector3()).Length();

			totalTime = 0.f;
		}

		totalTime += dt;
		
		if (!IsFocusingTransform)
		{
			if (Mouse.rightButton)
			{
				if (!PreviousMouseDown)
				{
					FirstUpdate = true;
					PreviousMouseDown = true;
				}
			}
			else
			{
				PreviousMouseDown = false;
				return;
			}

			float CameraSpeed = m_flyingSpeed;
			if (Keyboard.LeftShift)
			{
				CameraSpeed += m_speedModifier;
			}
			CameraSpeed *= dt;

			const Vector3& Front = EditorCameraTransform->Front();

			if (Keyboard.W)
			{
				EditorCameraTransform->Translate(Front * CameraSpeed);
			}
			if (Keyboard.S)
			{
				EditorCameraTransform->Translate((Front * CameraSpeed) * -1.f);
			}
			if (Keyboard.A)
			{
				EditorCameraTransform->Translate(Vector3::Up.Cross(Front).Normalized() * CameraSpeed);
			}
			if (Keyboard.D)
			{
				EditorCameraTransform->Translate(Front.Cross(Vector3::Up).Normalized() * CameraSpeed);
			}
			if (Keyboard.Space)
			{
				EditorCameraTransform->Translate(Vector3::Up * CameraSpeed);
			}
			if (Keyboard.E)
			{
				EditorCameraTransform->Translate(EditorCameraTransform->Up() * CameraSpeed);
			}
			if (Keyboard.Q)
			{
				EditorCameraTransform->Translate(Front.Cross(-Vector3::Up).Cross(Front).Normalized() * CameraSpeed);
			}

			Vector2 MousePosition = m_editor->GetInput().GetMousePosition();
			if (MousePosition.IsZero())
			{
				return;
			}

			if (FirstUpdate)
			{
				LastX = MousePosition.X();
				LastY = MousePosition.Y();
				FirstUpdate = false;
			}

			float XOffset = MousePosition.X() - LastX;
			float YOffest = LastY - MousePosition.Y();

			Input& editorInput = GetEditor()->GetInput();
			Vector2 windowPos = GetEngine().GetWindow()->GetPosition();
			Vector2 windowSize = GetEngine().GetWindow()->GetSize();
			Vector2 offset = editorInput.GetMouseOffset();
			if (MousePosition.X() + windowPos.X()<= windowPos.X())
			{
				editorInput.SetMousePosition(Vector2(windowPos.X() + windowSize.X(), MousePosition.Y() + windowPos.Y()));
			}
			else if (MousePosition.X() + windowPos.X() >= windowPos.X() + windowSize.X())
			{
				editorInput.SetMousePosition(Vector2(windowPos.X(), MousePosition.Y() + windowPos.Y()));
			}

			if (MousePosition.Y() + windowPos.Y() <= windowPos.Y())
			{
				editorInput.SetMousePosition(Vector2(windowPos.X() + MousePosition.X(), windowPos.Y() + windowSize.Y()));
			}
			else if (MousePosition.Y() + windowPos.Y() >= windowPos.Y() + windowSize.Y())
			{
				editorInput.SetMousePosition(Vector2(windowPos.X() + MousePosition.X(), windowPos.Y()));
			}

			LastX = MousePosition.X();
			LastY = MousePosition.Y();

			XOffset *= m_lookSensitivity;
			YOffest *= m_lookSensitivity;

			const float Yaw = EditorCamera->Yaw += XOffset;
			float Pitch = EditorCamera->Pitch += YOffest;

			if (Pitch > 89.0f)
			{
				Pitch = 89.0f;
			}
			if (Pitch < -89.0f)
			{
				Pitch = -89.0f;
			}

			Vector3 newFront;
			newFront.x = (cos(Mathf::Radians(Yaw)) * cos(Mathf::Radians(Pitch)));
			newFront.y = (sin(Mathf::Radians(Pitch)));
			newFront.z = (sin(Mathf::Radians(Yaw)) * cos(Mathf::Radians(Pitch)));

			EditorCameraTransform->LookAt(newFront.Normalized());
		}
		else
		{
			// Distance moved = time * speed.
			float distCovered = (totalTime - startTime) * m_focusSpeed;

			// Fraction of journey completed = current distance divided by total distance.
			float fracJourney = distCovered / TravelDistance;

			if (fracJourney <= .8f)
			{
				Vector3 lerp = Mathf::Lerp(EditorCameraTransform->GetPosition(), Vector3(), fracJourney);
				EditorCameraTransform->SetPosition(lerp);
			}
			else
			{
				IsFocusingTransform = false;
			}
		}
	}
}

void EditorCore::Update(float dt, Transform* rootTransform)
{
	OPTICK_EVENT("EditorCore::Update");

	RootTransform = rootTransform;

	Update(dt);

	if (m_isTryingToSaveNewScene)
	{
		ImGui::OpenPopup("SaveNewScenePopup");
		if (ImGui::BeginPopup("SaveNewScenePopup"))
		{
			char output[256] = "";
			if (ImGui::InputText("Destination", output, IM_ARRAYSIZE(output), ImGuiInputTextFlags_EnterReturnsTrue))
			{
				GetEngine().CurrentScene->Save(output, rootTransform);
				GetEngine().GetConfig().SetValue(std::string("CurrentScene"), GetEngine().CurrentScene->FilePath.LocalPath);
				m_isTryingToSaveNewScene = false;
			}
			ImGui::EndPopup();
		}
	}
}

bool EditorCore::OnEvent(const BaseEvent& evt)
{
	if (evt.GetEventId() == SaveSceneEvent::GetEventId())
	{
		if (GetEngine().CurrentScene->IsNewScene())
		{
			m_isTryingToSaveNewScene = true;
		}
		else
		{
			GetEngine().CurrentScene->Save(GetEngine().CurrentScene->FilePath.LocalPath, RootTransform);
			GetEngine().GetConfig().SetValue(std::string("CurrentScene"), GetEngine().CurrentScene->FilePath.LocalPath);
		}
		return true;
	}
	else if (evt.GetEventId() == Moonlight::PickingEvent::GetEventId())
	{
		const Moonlight::PickingEvent& casted = static_cast<const Moonlight::PickingEvent&>(evt);

		auto ents = GetEngine().GetWorld().lock()->GetCore(RenderCore::GetTypeId())->GetEntities();
		for (auto& ent : ents)
		{
			if (!ent.HasComponent<Mesh>())
			{
				continue;
			}
			if (ent.GetComponent<Mesh>().Id == casted.Id)
			{
				Transform* meshTransform = &ent.GetComponent<Transform>();
				std::stack<Transform*> parentEnts;
				parentEnts.push(meshTransform->GetParent());

				static Transform* selectedParentObjec = nullptr;

				while (parentEnts.size() > 0)
				{
					Transform* parent = parentEnts.top();
					parentEnts.pop();
					if (!parent)
					{
						continue;
					}

					if (parent->Parent->HasComponent<Model>())
					{
						Transform* selectedModel = &parent->Parent->GetComponent<Transform>();
						if (m_editor->SelectedTransform == nullptr || selectedParentObjec != selectedModel)
						{
							m_editor->SelectedTransform = selectedModel;
							selectedParentObjec = selectedModel;
							break;
						}

						if (meshTransform != m_editor->SelectedTransform)
						{
							m_editor->SelectedTransform = meshTransform;
						}
						else
						{
							m_editor->SelectedTransform = selectedModel;
						}
						break;
					}
					else
					{
						parentEnts.push(parent->GetParent());
					}
				}
			}
		}
		return true;
	}

	return false;
}

void EditorCore::OnEntityAdded(Entity& NewEntity)
{
	Base::OnEntityAdded(NewEntity);
}

Havana* EditorCore::GetEditor() const
{
	return m_editor;
}

Transform* EditorCore::GetEditorCameraTransform() const
{
	return EditorCameraTransform;
}

void EditorCore::OnEditorInspect()
{
	BaseCore::OnEditorInspect();

	ImGui::Text("Camera Settings");
	ImGui::DragFloat("Flying Speed", &m_flyingSpeed);
	ImGui::DragFloat("Speed Modifier", &m_speedModifier);
	ImGui::DragFloat("Focus Speed", &m_focusSpeed);
	ImGui::DragFloat("Look Sensitivity", &m_lookSensitivity, 0.01f);

	EditorCameraTransform->OnEditorInspect();

	EditorCamera->OnEditorInspect();
}

#endif