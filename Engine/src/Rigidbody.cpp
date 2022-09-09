#include "Rigidbody.h"
#include "Transform.h"
#include "Physics.h"
#include "PhysicsWorld.h"
#include "EngineAssert.h"
#include "Entity.h"

#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"

void Rigidbody::Start()
{
	ENGINEASSERT(GetEntity()->GetParent() == nullptr, "Rigidbodies can only be attached to root entities.")

	DirectX::XMFLOAT3 worldPosition = GetTransform()->GetWorldPosition();
	DirectX::XMFLOAT4 worldRotation;
	DirectX::XMStoreFloat4(&worldRotation, GetTransform()->GetWorldQuaternion());

	btTransform transform;
	transform.setOrigin(btVector3(-worldPosition.x, worldPosition.y, worldPosition.z));
	transform.setRotation(btQuaternion(-worldRotation.x, worldRotation.y, worldRotation.z, worldRotation.w));

	static btBoxShape* shape = new btBoxShape(btVector3(0.5f,0.5f,0.5f));

	motionState = new btDefaultMotionState(transform);
	body = new btRigidBody(btScalar(1.0f), motionState, shape, btVector3(1.0f, 1.0f, 1.0f));
	body->setUserPointer(GetTransform());

	Physics::GetActiveSceneWorld()->AddRigidbody(this);
}

Rigidbody* Rigidbody::Clone()
{
	Rigidbody* newBody = new Rigidbody;

	btTransform newTransform;
	motionState->getWorldTransform(newTransform);

	newBody->motionState = new btDefaultMotionState(newTransform);
	newBody->body = new btRigidBody(body->getMass(), newBody->motionState, nullptr, btVector3(1.0f, 1.0f, 1.0f));
	newBody->body->setUserPointer(newBody->GetTransform());

	return newBody;
}

void Rigidbody::SetMass(float mass)
{
}

float Rigidbody::GetMass() const
{
	return body->getMass();
}
