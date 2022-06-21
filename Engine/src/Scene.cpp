#include "Scene.h"
#include "Entity.h"
#include "EngineAssert.h"
#include "Renderer.h"
#include "Resources.h"
#include "Light.h"
#include "Graphics.h"
#include "Editor.h"

#include <set>


void Scene::Init()
{
	Scene::CreateNewScene(1, "first scene");

	Entity* e2 = Scene::GetScene(1)->NewEntity(Entity::FindPrefab("sponza.obj"));
	e2->GetTransform()->SetLocalScale({ 0.01f,0.01f ,0.01f });

	Entity* light = Scene::GetScene(1)->NewEntity("Light");
	light->AddComponent<DirectionalLight>();
	light->GetTransform()->SetLocalRotation(90.0f, 0.0f, 0.0f);

	Scene::LoadScene(1);
};

void Scene::CreateNewScene(unsigned char index, const std::string& name)
{
	auto scene = Scenes.emplace(std::piecewise_construct, std::forward_as_tuple(index), std::forward_as_tuple(std::make_unique<Scene>(index,name)));
	ENGINEASSERT(scene.second, "Duplicate scene index.");
}

Scene* Scene::GetScene(unsigned char index)
{
	auto scene = Scenes.find(index);
	ENGINEASSERT(scene != Scenes.end(), "Scene index is not correct");
	return scene->second.get();
}

Scene* Scene::GetScene(const std::string& name)
{
	for (auto& s : Scenes)
	{
		if (s.second->Name == name)
			return s.second.get();
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
	ActiveScene = scene->second.get();
}

void Scene::LoadScene(const std::string& name)
{
	Scene* scene = nullptr;
	for (auto& s : Scenes)
	{
		if (s.second->Name == name)
			scene = s.second.get();
	}
	ENGINEASSERT(scene != nullptr, "Specified scene name is not correct");
	ActiveScene = scene;
}

Entity* Scene::NewEntity(const std::string& name)
{
	Entities.push_back(std::make_unique<Entity>(name));
	return Entities.back().get();
}

Entity* Scene::NewEntity(Entity* cloneFrom)
{
	Entities.push_back(std::unique_ptr<Entity>(cloneFrom->Clone()));
	return Entities.back().get();
}

Entity* Scene::GetEntity(const std::string& name)
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