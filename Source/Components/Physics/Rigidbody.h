#pragma once
#include "ECS/Component.h"
#include "ECS/ComponentDetail.h"
#include <btBulletDynamicsCommon.h>
#include "Math/Vector3.h"
#include <DirectXMath.h>
#include "Math/Matirx4.h"
#include "Math/Quaternion.h"

class Rigidbody
	: public Component<Rigidbody>
{
	friend class PhysicsCore;
public:

	enum class ColliderType : unsigned int
	{
		Box = 0,
		Sphere,
		Count
	};
	Rigidbody();
	Rigidbody(ColliderType type);
	virtual ~Rigidbody() final;

	// Separate init from construction code.
	virtual void Init() final;

	bool IsRigidbodyInitialized();
	void ApplyForce(const Vector3& direction, float force);
	void SetScale(Vector3 InScale);
	void SetMass(float InMass);
	void SetVelocity(Vector3 newVelocity);

	const bool IsDynamic() const;

	std::string GetColliderString(ColliderType InType);

	Matrix4 GetMat();

	unsigned int Id = 0;

private:
	void CreateObject(const Vector3& Position, Quaternion& Rotation, Vector3& InScale, class btDiscreteDynamicsWorld* world);
	btRigidBody* InternalRigidbody = nullptr;
	btCollisionShape* fallShape = nullptr;
	Vector3 Scale;
	Vector3 Velocity;
	ColliderType Type = ColliderType::Box;
	float Mass = 10.0f;
	bool m_isDynamic = false;

	virtual void OnSerialize(json& outJson) final;
	virtual void OnDeserialize(const json& inJson) final;
protected:
	bool IsInitialized = false;
	class btDiscreteDynamicsWorld* m_world;
	int DebugColliderId = 0;
#if ME_EDITOR

	virtual void OnEditorInspect() final;

#endif
};

ME_REGISTER_COMPONENT_FOLDER(Rigidbody, "Physics")