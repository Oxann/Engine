#include "Scene.h"
#include "Entity.h"
#include "EngineAssert.h"
#include "Renderer.h"
#include "Resources.h"
#include "Unlit_Material.h"
#include "Light.h"
#include "Graphics.h"
#include "Editor.h"

#include <set>

std::map<unsigned char,Scene> Scene::Scenes;
Scene* Scene::ActiveScene;

void Scene::Init()
{
	Scene::CreateNewScene(1, "first scene");
	Scene::LoadScene(1);
};

void Scene::CreateNewScene(unsigned char index, std::string name)
{
	auto scene = Scenes.emplace(std::piecewise_construct, std::forward_as_tuple(index), std::forward_as_tuple(index,name));
	ENGINEASSERT(scene.second, "Duplicate scene index.");
}

Scene* Scene::GetScene(unsigned char index)
{
	auto scene = Scenes.find(index);
	ENGINEASSERT(scene != Scenes.end(), "Scene index is not correct");
	return &scene->second;
}

Scene* Scene::GetScene(std::string name)
{
	for (auto& s : Scenes)
	{
		if (s.second.Name == name)
			return &s.second;
	}
	ENGINEASSERT(false, "Specified scene name is not correct");
}

Scene* Scene::GetActiveScene()
{
	return ActiveScene;
}

void Scene::LoadScene(unsigned char index)
{
	auto scene = Scenes.find(index);
	ENGINEASSERT(scene != Scenes.end(), "Specified scene index is not correct");
	ActiveScene = &scene->second;

	for (const auto& entity : ActiveScene->Entities)
	{
		entity->Start();
	}
}

void Scene::LoadScene(std::string name)
{
	Scene* scene = nullptr;
	for (auto& s : Scenes)
	{
		if (s.second.Name == name)
			scene = &s.second;
	}
	ENGINEASSERT(scene != nullptr, "Specified scene name is not correct");
	ActiveScene = scene;

	for (const auto& entity : ActiveScene->Entities)
	{
		entity->Start();
	}
}

Entity* Scene::NewEntity(std::string name)
{
	Entities.push_back(std::make_unique<Entity>(name));
	return Entities.back().get();
}

Entity* Scene::NewEntity(Entity* cloneFrom)
{
	Entities.push_back(std::unique_ptr<Entity>(cloneFrom->Clone()));
	return Entities.back().get();
}

Entity* Scene::GetEntity(std::string name)
{
	for (const auto& entity : Entities)
	{
		if (entity->name == name)
			return entity.get();
	}
	return nullptr;
}

size_t Scene::GetRootEntityCount() const
{
	return Entities.size();
}

size_t Scene::GetEntityCount() const
{
	size_t count = Entities.size();
	for (int i = 0; i < Entities.size(); i++)
	{
		count += Entities[i]->GetDescendantCount();
	}
	return count;
}