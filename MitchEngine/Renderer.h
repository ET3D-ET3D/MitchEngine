#pragma once
#include "Core.h"
#include "Sprite.h"

namespace ma {
	class Renderer :
		public Core<Renderer> {
	public:
		Renderer();
		~Renderer();

		// Separate init from construction code.
		virtual void Init() final;

		// Each core must update each loop
		virtual void Update(float dt) final;

		// Can receive messages from the engine.
		virtual void SendMessage(class Message* message) final;

		void AddSprite(Sprite* sprite);
	};
}