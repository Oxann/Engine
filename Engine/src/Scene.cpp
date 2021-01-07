#include "Scene.h"
#include "EngineAssert.h"
#include "Renderer.h"
#include "Resources.h"
#include "Unlit_Material.h"

std::map<unsigned char,Scene> Scene::Scenes;
Scene* Scene::ActiveScene;

void Scene::Init()
{
	Scene::CreateNewScene(1, "first scene");
	
	Entity* wolf = Scene::GetScene(1)->NewEntity(Entity::FindPrefab("goblin.obj"));

	Entity* light = Scene::GetScene(1)->NewEntity("Light");
	light->AddComponent<DirectionalLight>();
	
	Scene::LoadScene(1);
}

void Scene::CreateNewScene(unsigned char index, std::string name)
{
	auto scene = Scenes.insert(std::pair<unsigned char, Scene>(index, Scene(index, name)));
	ENGINEASSERT(scene.second, "Duplicate scene index.");
}

Scene* Scene::GetScene(unsigned char index)
{
	auto scene = Scenes.find(index);
	ENGINEASSERT(scene != Scenes.end(), "Specified scene index is not correct");
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

void Scene::Update()
{
	size_t entitiySize = Entities.size();

	for (size_t i = 0; i < entitiySize; i++)
	{
		UpdateComponents(Entities[i].get());
	}

	//Engine Component Calls
	for (size_t i = 0; i < entitiySize; i++)
	{
		UpdateEngineComponents(Entities[i].get());
	}
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

void Scene::UpdateComponents(Entity* entity)
{
	/*
	for (size_t j = 0; j < componentSize; j++)
	{
		auto& component = entity->Components[j];
		switch (component->state)
		{
		[[unlikely]] case Component::State::Start:
			component->Start();
			component->state = Component::State::Update;
			break;
		[[likely]] case Component::State::Update:
			component->Update();
			break;
		default:
			break;
		}
	}

	for (int i = 0; i < entity->GetChildrenCount(); i++)
		UpdateComponents(entity->Children[i].get());*/
}

void Scene::UpdateEngineComponents(Entity* entity)
{
	for (auto& component : entity->EngineComponents)
	{
		switch (component.second->state)
		{
		[[unlikely]] case Component::State::Start:
			component.second->Start();
			component.second->state = Component::State::Update;
			break;
		[[likely]] case Component::State::Update:
			component.second->Update();
			break;
		default:
			break;
		}
	}

	for (int i = 0; i < entity->GetChildrenCount(); i++)
		UpdateEngineComponents(entity->Children[i].get());
}
