#pragma once

#include <vector>
#include <type_traits>
#include <memory>
#include <string>
#include <typeindex>
#include <unordered_map>

#include "Component.h"
#include "Transform.h"
#include "Renderer.h"

class Scene;

class Entity
{
	friend class Model;
	friend Scene;
	friend class EditorSceneHierarchyWindow;
	friend class Engine;

#ifdef EDITOR
	friend class Editor;
#endif

public:
	Entity() = default;
	Entity(const std::string& name);
private:
	Entity* Clone();
	void Start();
	void Update();
public:
	//DON'T DELETE ENTITY!!!
	//USE DeletePrefab instead.
	static Entity* FindPrefab(const std::string& name);

	static void MakePrefab(Entity* entity);
public:
	template<class ComponentType>
	ComponentType* GetComponent() const
	{
		static_assert(std::is_base_of<Component, ComponentType>::value,"All components must be derived from Component.");
		static_assert(!std::is_same<Transform, ComponentType>::value, "Use GetTransform instead.");
		
		auto result = Components.find(typeid(ComponentType));
		if (result == Components.end())
			return nullptr;
		else
		{
			return static_cast<ComponentType*>(result->second.get());
		}
	}
	
	//Returns newly added component.
	template<class ComponentType>
	ComponentType* AddComponent()
	{		
		static_assert(std::is_base_of<Component, ComponentType>::value, "All components must be derived from Component.");
		static_assert(!std::is_same<Transform, ComponentType>::value, "Transform can't be added.");

		auto result = Components.insert({ typeid(ComponentType),std::make_unique<ComponentType>() });
		if (result.second)
		{
			result.first->second->entity = this;
			result.first->second->transform = Transform_.get();
			return static_cast<ComponentType*>(result.first->second.get());
		}
		else
			return nullptr;
	}

	//Clones the component from given entity.
	//If the given entity doesn't have the component or this entity already has the component,it will return nullptr.
	template <class ComponentType>
	ComponentType* AddComponent(const Entity* cloneFrom)
	{
		static_assert(std::is_base_of<Component, ComponentType>::value, "All components must be derived from Component.");
		static_assert(std::is_same<Transform, ComponentType>::value, "Transform can't be added.");

		std::type_index componentType = typeid(ComponentType);		
		auto cloneComponent = cloneFrom->Components.find(componentType);

		if(cloneComponent == cloneFrom->Components.end() || Components.find(componentType) != Components.end())
			return nullptr;
		else
		{
			auto result = Components.insert({ componentType, std::unique_ptr<Component>(cloneComponent.second.get()->Clone()) });
			result.first->second->entity = this;
			result.first->second->transform = Transform_.get();
			return static_cast<ComponentType*>(result.first->second.get());
		}
	}

	//Returns newly added child.
	Entity* AddChild(std::string name);

	//Returns newly added child.
	Entity* AddChild(Entity* entity);

	Entity* GetChild(const std::string& name) const;

	Entity* GetChild(unsigned int index) const;

	Entity* GetParent() const;

	size_t GetChildrenCount() const;

	size_t GetDescendantCount() const;

	//If the caller is the root then returns itself.
	Entity* GetRoot() const;

	Transform* GetTransform() const;

	Renderer* GetRenderer() const;
	Renderer* AddRenderer(Entity* cloneFrom);

public:
	std::string name;
private:
	Entity* parent = nullptr;
	std::vector<std::unique_ptr<Entity>> Children;
	std::unordered_map<std::type_index, std::unique_ptr<Component>> Components;
	std::unique_ptr<Transform> Transform_;
	std::unique_ptr<Renderer> Renderer_ = nullptr;
private:
	inline static std::unordered_map<std::string, std::unique_ptr<Entity>> Prefabs;
};