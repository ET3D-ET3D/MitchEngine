#pragma once
#include "ECS/Component.h"
#include "ECS/ComponentDetail.h"
#include <btBulletDynamicsCommon.h>
#include "Math/Vector3.h"
#include <DirectXMath.h>
#include "Math/Matirx4.h"

class Rigidbody
	: public Component<Rigidbody>
{
	friend class PhysicsCore;
public:

	enum class ColliderType : unsigned int
	{
		Box = 0,
		Sphere
	};
	Rigidbody();
	Rigidbody(ColliderType type);
	virtual ~Rigidbody() final;

	// Separate init from construction code.
	virtual void Init() final;

	bool IsRigidbodyInitialized();
	void ApplyForce(const Vector3& direction, float force);
	unsigned int Id = 0;
	void SetScale(Vector3 InScale);

	Matrix4 GetMat()
	{
		btTransform trans;
		InternalRigidbody->getMotionState()->getWorldTransform(trans);

		float m[16];
		trans.getOpenGLMatrix(m);

		DirectX::XMMATRIX transform(m[0], m[4], m[8], m[12],
			m[1], m[5], m[9], m[13],
			m[2], m[6], m[10], m[14],
			m[3], m[7], m[11], m[15]);

		return Matrix4(transform);
	}
private:
	void CreateObject(const Vector3& Position, Vector3& Rotation, class btDiscreteDynamicsWorld* world);
	btRigidBody* InternalRigidbody = nullptr;
	btCollisionShape* fallShape = nullptr;
	Vector3 Scale;
	ColliderType Type = ColliderType::Box;
protected:
	bool IsInitialized = false;
	class btDiscreteDynamicsWorld* m_world;
	int DebugColliderId = 0;
#if ME_EDITOR

	virtual void OnEditorInspect() final;

#endif
};

ME_REGISTER_COMPONENT(Rigidbody)