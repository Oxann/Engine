#pragma once
#include <unordered_set>

class PhysicsWorld;



class Physics
{
	friend class Engine;
	friend PhysicsWorld;
public:
	static PhysicsWorld* const GetActiveSceneWorld();
private:
	static void Init();
	static void ShutDown();
	static void Update();
	static bool CheckCollision(class Collider* c1, class Collider* c2);
private:
	inline static class btDefaultCollisionConfiguration* collisionConfiguration;
	inline static class btCollisionDispatcher* dispatcher;
	inline static class btBroadphaseInterface* overlappingPairCache;
	inline static class btSequentialImpulseConstraintSolver* solver;
};