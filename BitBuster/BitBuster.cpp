#include "BitBuster.h"
#include "Renderer.h"
#include "Logger.h"
#include "Sprite.h"
#include "Component.h"
#include "Transform.h"
#include "Entity.h"
#include <string>
#include "Input.h"
#include "Window.h"
#include "Animation.h"
#include "Collider2D.h"

#include <memory>

using namespace ma;

BitBuster::BitBuster() : Game() {
}

BitBuster::~BitBuster() {
	Game::~Game();
}

std::vector<Entity> EntList;

void BitBuster::Initialize() {
	Resources.Push();
	for (int i = 0; i < 3; ++i) {
		Entity& Background = GameWorld->CreateEntity();
		Transform& BGPos = Background.GetComponent<Transform>();
		Sprite& BGSprite = Background.AddComponent<Sprite>();
		BGSprite.SetImage(Resources.Get<Texture>("colored_grass.png"));
		BGPos.Position = glm::vec3(BGSprite.FrameSize.x * i, Window::WINDOW_HEIGHT / 2, 0.0f);
	}

	for (int i = 0; i < 10; ++i) {
		EntList.push_back(GameWorld->CreateEntity());
		auto& TransformComponent = EntList[i].GetComponent<Transform>();
		auto& SpriteComponent = EntList[i].AddComponent<Sprite>();
		auto& AnimationComponent = EntList[i].AddComponent<Animation>();
		auto& ColliderComponent = EntList[i].AddComponent<Collider2D>();

		TransformComponent.Position = glm::vec3((SpriteComponent.FrameSize.x+ 200) * i, SpriteComponent.FrameSize.y * i, 0.0f);

		SpriteComponent.SetImage(Resources.Get<Texture>("Default3.png"));

		AnimationComponent.SetAnimationInfo(SpriteComponent.FrameSize.x, SpriteComponent.FrameSize.y, 7, 4);
		AnimationComponent.FPS = 60.f;
	}

	for (int i = 0; i < 30; ++i) {
		Entity& Ground = GameWorld->CreateEntity();
		auto& TransformComponent = Ground.GetComponent<Transform>();
		auto& SpriteComponent = Ground.AddComponent<Sprite>();
		auto& ColliderComponent = Ground.AddComponent<Collider2D>();

		SpriteComponent.SetImage(Resources.Get<Texture>("Grass.png"));

		TransformComponent.Position = glm::vec3(SpriteComponent.FrameSize.x * i, Window::WINDOW_HEIGHT - (SpriteComponent.FrameSize.y/2), 0.0f);
		TransformComponent.Scale = glm::vec3(1.f);

		ColliderComponent.SetBodyType(b2_staticBody);
	}
}

void BitBuster::Update(float DeltaTime) {
	for (auto E : EntList) {
		Transform& TransformComponent = E.GetComponent<Transform>();
		if (Input::Get().IsKeyDown(GLFW_KEY_W)) {
			E.SetActive(true);
			TransformComponent.Position += glm::vec3(0, -40 * DeltaTime, 0);
		}
		if (Input::Get().IsKeyDown(GLFW_KEY_S)) {
			E.SetActive(false);
			TransformComponent.Position += glm::vec3(0, 40 * DeltaTime, 0);
		}
		if (Input::Get().IsKeyDown(GLFW_KEY_A)) {
			TransformComponent.Position += glm::vec3(-40 * DeltaTime, 0, 0);
		}
		if (Input::Get().IsKeyDown(GLFW_KEY_D)) {
			TransformComponent.Position += glm::vec3(40 * DeltaTime, 0, 0);
		}
	}
}

void BitBuster::End() {
}
