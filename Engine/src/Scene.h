#pragma once

#include <map>
#include <string>

#include "Entity.h"
#include "RendererManager.h"

class Scene
{
	friend class EditorSceneHierarchyWindow;
	friend class Engine;
	friend Entity;

#ifdef EDITOR
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
	{}
public:
	std::string Name;
	unsigned char Index;
	RendererManager rendererManager{};
private:
	std::vector<std::unique_ptr<Entity>> Entities;
	static std::map<unsigned char, Scene> Scenes;
	static Scene* ActiveScene;
};