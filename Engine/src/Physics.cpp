#include "Physics.h"
#include "PhysicsWorld.h"
#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"
#include "Scene.h"
#include "Time.h"
#include "Collider.h"
#include "BoxCollider.h"
#include "SphereCollider.h"
#include "Log.h"
#include <DirectXMath.h>

struct InternalCollisionInfo
{
	const btCollisionObject* c1;
	const btCollisionObject* c2;

	bool operator==(const InternalCollisionInfo& right) const noexcept
	{
		return c1 == right.c1 && c2 == right.c2;
	}
};

template<>
struct std::hash<InternalCollisionInfo>
{
	size_t operator()(InternalCollisionInfo info) const noexcept
	{
		std::hash<const btCollisionObject*> hasher;
		size_t hash = 0;
		hash ^= hasher(info.c1) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
		hash ^= hasher(info.c2) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
		return hash;
	}
};

//A container for storing collisions which exist current frame.
inline static std::unordered_map<InternalCollisionInfo, btPersistentManifold*> collisions;


void Physics::Init()
{
	collisionConfiguration = new btDefaultCollisionConfiguration();
	dispatcher = new btCollisionDispatcher(collisionConfiguration);
	overlappingPairCache = new btDbvtBroadphase();
	solver = new btSequentialImpulseConstraintSolver();
}

void Physics::ShutDown()
{
	for (auto& scene : Scene::Scenes)
	{
		scene.second->physicsWorld.reset();
	}

	delete solver;
	delete overlappingPairCache;
	delete dispatcher;
	delete collisionConfiguration;
}

void Physics::Update()
{
	const btCollisionObjectArray& objects = Scene::ActiveScene->physicsWorld->world->getCollisionObjectArray();
	const unsigned int size = objects.size();

	for (int i = 0; i < size; i++)
	{
		objects[i]->activate(true);

		btRigidBody* body = btRigidBody::upcast(objects[i]);
		btTransform _transform;

		if (body)
		{

		}
		else
		{
			Transform* transform = (Transform*)(objects[i]->getUserPointer());
			DirectX::XMFLOAT3 worldPosition = transform->GetWorldPosition();
			DirectX::XMFLOAT4 worldQuaternion;
			DirectX::XMStoreFloat4(&worldQuaternion, transform->GetWorldQuaternion());

			_transform.setOrigin(btVector3(worldPosition.x, worldPosition.y, worldPosition.z));
			_transform.setRotation(btQuaternion(worldQuaternion.x, worldQuaternion.y, worldQuaternion.z, worldQuaternion.w));

			objects[i]->setWorldTransform(_transform);
		}
	}

	Scene::ActiveScene->physicsWorld->world->stepSimulation(Time::GetDeltaTime(), 5);


	for (int i = 0; i < size; i++)
	{
		btRigidBody* body = btRigidBody::upcast(objects[i]);
		btTransform _transform;
		if (body)
		{
			body->getMotionState()->getWorldTransform(_transform);

			Transform* transform = (Transform*)body->getUserPointer();



			btQuaternion rotation = _transform.getRotation();
			transform->SetLocalPosition(_transform.getOrigin().x(), _transform.getOrigin().y(), _transform.getOrigin().z());
			transform->SetLocalRotation(rotation.x(), rotation.y(), rotation.z(), rotation.w());
		}
		else
		{
		}
	}

	collisions.clear();

	for (int i = 0; i < dispatcher->getNumManifolds(); i++)
	{
		auto manifold = dispatcher->getManifoldByIndexInternal(0);
		
		if (manifold->getNumContacts() > 0)
		{
			collisions.emplace(std::piecewise_construct, std::forward_as_tuple(manifold->getBody0(), manifold->getBody1()), std::forward_as_tuple(manifold));
		}
	}
}

bool Physics::CheckCollision(Collider* c1, Collider* c2)
{
	return collisions.find({ c1->collider,c2->collider }) != collisions.end();
}

PhysicsWorld* const Physics::GetActiveSceneWorld()
{
	return Scene::ActiveScene->physicsWorld.get();
}
