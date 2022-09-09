#pragma once

#include <map>
#include <string>

#include "Entity.h"
#include "RendererManager.h"
#include "PhysicsWorld.h"

class Camera;

class Scene
{
	friend class Engine;
	friend class Physics;
	friend Entity;

#ifdef EDITOR
	friend class EditorSceneHierarchyWindow;
	friend class Editor;
#endif

public:
	static void Init();
	static void CreateNewScene(unsigned char index, const std::string& name);
	static Scene* GetScene(unsigned char index);
	static Scene* GetScene(const std::string& name);
	static void LoadScene(unsigned char index);
	static void LoadScene(const std::string& name);
	static Scene* GetActiveScene();
public:
	Entity* NewEntity(const std::string& name);
	Entity* NewEntity(Entity* from);
	Entity* GetEntity(const std::string& name);
	size_t GetEntityCount() const;
	size_t GetRootEntityCount() const;

	Scene(unsigned char index, const std::string& name)
		:Index(index),
		Name(name)
	{
		physicsWorld.reset(new PhysicsWorld);
	}
public:
	std::string Name;
	unsigned char Index;
	RendererManager rendererManager{};
	std::unique_ptr<PhysicsWorld> physicsWorld;
private:
	std::vector<std::unique_ptr<Entity>> Entities;
	inline static std::map<unsigned char, std::unique_ptr<Scene>> Scenes;
	inline static Scene* ActiveScene;
};