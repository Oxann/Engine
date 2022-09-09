#include "PhysicsWorld.h"
#include "Physics.h"
#include "Rigidbody.h"
#include "Collider.h"

PhysicsWorld::PhysicsWorld()
{
	world = new btDiscreteDynamicsWorld(Physics::dispatcher, Physics::overlappingPairCache, Physics::solver, Physics::collisionConfiguration);
	SetGravity(0.0f, -1.0f, 0.0f);
}

void PhysicsWorld::SetGravity(float x, float y, float z)
{
	world->setGravity(btVector3(x, y, z));
}

void PhysicsWorld::AddRigidbody(Rigidbody* body)
{
	world->addRigidBody(body->body);
}

void PhysicsWorld::AddCollider(Collider* collider)
{
	world->addCollisionObject(collider->collider, 1 , 1);
}
