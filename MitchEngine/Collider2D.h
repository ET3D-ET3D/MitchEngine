#pragma once
#include "Component.h"
#include "Box2D/Box2D.h"

namespace ma {
	class Collider2D : public Component<Collider2D> {
		friend class PhysicsCore;
	public:
		Collider2D();
		~Collider2D();

		// Separate init from construction code.
		virtual void Init() final;

		// Each core must update each loop
		virtual void Update(float dt) final;

		void SetBodyType(b2BodyType InBodyType);
	private:
		class b2PhysicsWorld;
		b2Body* Body;

		b2BodyDef BodyDefinition;
		b2FixtureDef FixtureDefinition;
		b2PolygonShape ShapeDefinition;

		bool IsInitialized;
	};
}