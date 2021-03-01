#pragma once

#include <map>
#include <string>

#include "Entity.h"

class Scene
{
	friend class EditorSceneHierarchyWindow;
	friend class Engine;

#ifdef EDITOR
	friend class Editor;
#endif

public:
	static void Init();
	static void CreateNewScene(unsigned char index, std::string name);
	static Scene* GetScene(unsigned char index);
	static Scene* GetScene(std::string name);
	static void LoadScene(unsigned char index);
	static void LoadScene(std::string name);
	static Scene* GetActiveScene();
public:
	Entity* NewEntity(std::string name);
	Entity* NewEntity(Entity* from);
	Entity* GetEntity(std::string name);
	size_t GetEntityCount() const;
	size_t GetRootEntityCount() const;
private:
	Scene(unsigned char index,std::string name)
		:Index(index),
		Name(name)
	{}
public:
	std::string Name;
	unsigned char Index;
private:
	std::vector<std::unique_ptr<Entity>> Entities;
	static std::map<unsigned char,Scene> Scenes;
	static Scene* ActiveScene;
};