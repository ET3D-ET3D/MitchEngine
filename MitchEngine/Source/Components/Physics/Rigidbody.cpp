#include "PCH.h"
#include "Rigidbody.h"
#include "Cores/PhysicsCore.h"

Rigidbody::Rigidbody()
{
}

Rigidbody::~Rigidbody()
{
}

void Rigidbody::Init()
{

}


bool Rigidbody::IsRigidbodyInitialized()
{
	return IsInitialized;
}

void Rigidbody::ApplyForce(const Vector3& direction, float force)
{
	InternalRigidbody->applyForce(PhysicsCore::ToBulletVector(direction * force), -PhysicsCore::ToBulletVector(direction));
}

void Rigidbody::CreateObject(const Vector3& Position)
{
	btCollisionShape* fallShape = new btBoxShape(btVector3(0.25, 0.25, 0.25));

	btDefaultMotionState* fallMotionState =
		new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(Position.X(), Position.Y(), Position.Z())));
	btScalar mass = 10;
	btVector3 fallInertia(0, 0, 0);
	fallShape->calculateLocalInertia(mass, fallInertia);
	btRigidBody::btRigidBodyConstructionInfo fallRigidBodyCI(mass, fallMotionState, fallShape, fallInertia);
	InternalRigidbody = new btRigidBody(fallRigidBodyCI);
	IsInitialized = true;
}