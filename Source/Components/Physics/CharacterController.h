#pragma once
#include "ECS/Component.h"

#include "btBulletDynamicsCommon.h"
#include <BulletCollision/CollisionDispatch/btGhostObject.h>
#include "BulletDynamics/Dynamics/btDynamicsWorld.h"
#include "World/Scene.h"
#include "LinearMath/btDefaultMotionState.h"
#include "BulletCollision/CollisionShapes/btCollisionShape.h"
#include "BulletDynamics/Dynamics/btRigidBody.h"
#include "LinearMath/btTransform.h"

class CharacterController
	: public Component<CharacterController>
{
	friend class PhysicsCore;
public:
	CharacterController();
	~CharacterController();

	float Deceleration = 0.1f;
	float MaxSpeed = 5.f;
	float JumpForce = 10.f;

	float JumpRechargeTime = 0.5f;

	// Separate init from construction code.
	virtual void Init() final;

	void Initialize(Scene* pScene, btDynamicsWorld* pPhysicsWorld, const Vector3 spawnPos, float radius, float height, float mass, float stepHeight);

	void Walk(const Vector3& direction);
	void Walk(Vector2 direction);

	void Update(float dt);

	void Jump();

	Vector3 GetPosition() const;
	Vector3 GetVelocity() const;

	bool IsOnGround() const;

#if ME_EDITOR
	virtual void OnEditorInspect() override;
#endif
private:
	btDynamicsWorld* m_world = nullptr;
	btCollisionShape* m_shape = nullptr;
	btDefaultMotionState* m_motionState = nullptr;
	btRigidBody* m_rigidbody = nullptr;
	btPairCachingGhostObject* m_ghostObject = nullptr;

	bool m_isGrounded = false;
	bool m_isHittingWall = false;
	float m_bottomYOffset = 0.f;
	float m_bottomRoundedRegionYOffset = 0.f;
	float m_stepHeight = 0.1f;

	btTransform m_motionTransform;

	Vector3 m_manualVelocity;
	std::vector<Vector3> m_surfaceHitNormals;

	btVector3 m_previousPosition;
	float m_jumpTimer = 0.f;

	void ParseGhostContacts();

	void UpdatePosition();
	void UpdateVelocity(float dt);

	virtual void OnSerialize(json& outJson) final;
	virtual void OnDeserialize(const json& inJson) final;
};

ME_REGISTER_COMPONENT_FOLDER(CharacterController, "Physics")