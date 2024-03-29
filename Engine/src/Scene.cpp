#include "Scene.h"
#include "Entity.h"
#include "EngineAssert.h"
#include "Renderer.h"
#include "Resources.h"
#include "Light.h"
#include "Graphics.h"
#include "Editor.h"
#include "EnvironmentMap.h"
#include "BoxCollider.h"
#include "Rigidbody.h"
#include "SphereCollider.h"

#include <set>


void Scene::Init()
{
	Scene::CreateNewScene(1, "first scene");

	auto a = Scene::GetScene(1)->NewEntity(Entity::FindPrefab("character.fbx"));
	a->GetTransform()->SetLocalScale(0.01f, 0.01f, 0.01f);

	/*auto x = Scene::GetScene(1)->NewEntity("asd");
	auto r = x->AddRenderer(nullptr);
	r->SetMesh(Mesh::Cube);
	r->GetTransform()->SetLocalPosition(0.0f, 0.5f, 0.0f);*/

	Renderer* renderer = Scene::GetScene(1)->NewEntity("cube")->AddRenderer(nullptr);
	renderer->GetTransform()->SetLocalScale(50.0f,1.0f,50.0f);
	renderer->GetTransform()->SetLocalPosition(0.0f,-0.5f,0.0f);
	renderer->SetMesh(Mesh::Cube);

/* {
		auto x = Scene::GetScene(1)->NewEntity("asd");
		x->AddComponent<SphereCollider>()->SetRadius(1.0f);
		auto r = x->AddRenderer(nullptr);
		r->SetMesh(Mesh::Cube);
		r->GetTransform()->SetLocalPosition({ 0.0f,15.0f,0.0f });
	}

	{
		auto x = Scene::GetScene(1)->NewEntity("asd");
		x->AddComponent<BoxCollider>()->SetSize(1.0f,1.0f,1.0f);
		auto r = x->AddRenderer(nullptr);
		r->SetMesh(Mesh::Cube);
	}*/
	
	
	/*for (int i = 0; i <= 10; i++)
	{
		for (int j = 0; j <= 10; j++)
		{
			Renderer* renderer = Scene::GetScene(1)->NewEntity("cube")->AddRenderer(nullptr);
			renderer->GetTransform()->SetLocalPosition(i, 0.0f, -j);
			renderer->SetMesh(Mesh::Sphere);

			static auto shader = Resources::FindShader("LitPBR");
			Material* pbr = new Material("pbr", shader);
			pbr->SetFloat4("diffuseColor", { 1.0f,0.0f,0.0f,1.0f });
			pbr->SetFloat("metalness", i * 0.1f);
			pbr->SetFloat("smoothness", j * 0.1f);
			renderer->SetMaterial(pbr, 0);
		}
	}*/

	Entity* light5 = Scene::GetScene(1)->NewEntity("Light2");
	light5->AddComponent<DirectionalLight>();
	light5->GetTransform()->SetLocalRotation(45.0f, 0.0f, 0.0f);
	
	//Skybox* skybox = new Skybox(std::string_view("Resources\\newport.hdr"));
	//EnvironmentMap* env = new EnvironmentMap(skybox->GetTexture());
	//Scene::GetScene(1)->rendererManager.SetSkybox(skybox);
	//Scene::GetScene(1)->rendererManager.SetEnvironmentMap(env);


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