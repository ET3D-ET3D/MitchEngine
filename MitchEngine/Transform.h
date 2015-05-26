#pragma once
#include "Component.h"
#include <glm.hpp>

namespace ma {
	class Transform :
		public Component<Transform> {
	public:
		glm::vec2 Position;
		glm::vec2 Scale;
		float Rotation;

		Transform();
		~Transform();

		// Each core must update each loop
		virtual void Update(float dt) final;

		// Separate init from construction code.
		virtual void Init() final;

		// Can receive messages from the engine.
		virtual void SendMessage(class Message* message) final;
	};
}