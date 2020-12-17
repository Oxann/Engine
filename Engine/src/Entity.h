#pragma once

#include <vector>
#include <type_traits>
#include <memory>
#include <string>
#include <typeindex>

//Components
#include "Transform.h"
#include "Renderer.h"
#include "Light.h"

class Entity
{
	friend class Model;
	friend class Scene;
	friend class EditorSceneHierarchyWindow;
public:
	Entity(std::string name);
	~Entity() = default;
private:
	Entity() = default;
	Entity(const Entity& entity) = delete;
	Entity(Entity&& Entity) = delete;
	Entity& operator= (const Entity& rhs) = delete;
	Entity& operator= (Entity&& rhs) = delete;
	Entity* Clone();
public:
	//DON'T DELETE ENTITY!!!
	//USE DeletePrefab instead.
	static Entity* FindPrefab(std::string name);

	static void MakePrefab(Entity* entity);
public:
	template<class ComponentType>
	inline ComponentType* GetComponent() const
	{
		static_assert(std::is_base_of<Component, ComponentType>::value,"All components must be derived from Component.");
		static_assert(!std::is_same<Transform, ComponentType>::value, "Use GetTransform instead.");
		
		auto result = EngineComponents.find(typeid(ComponentType));
		if (result == EngineComponents.end())
			return nullptr;
		else
		{
			return static_cast<ComponentType*>(result->second.get());
		}
	}
	
	//Returns newly added component.
	template<class ComponentType>
	inline ComponentType* AddComponent()
	{		
		static_assert(std::is_base_of<Component, ComponentType>::value, "All components must be derived from Component.");
		static_assert(!std::is_same<Transform, ComponentType>::value, "Transform can't be added.");

		auto result = EngineComponents.insert({ typeid(ComponentType),std::make_unique<ComponentType>() });
		if (result.second)
		{
			result.first->second->entity = this;
			return static_cast<ComponentType*>(result.first->second.get());
		}
		else
			return nullptr;
	}

	//Clones the component from given entity.
	//If the given entity doesn't have the component or this entity already has the component,it will return nullptr.
	template <class ComponentType>
	inline ComponentType* AddComponent(const Entity* cloneFrom)
	{
		static_assert(std::is_base_of<Component, ComponentType>::value, "All components must be derived from Component.");
		static_assert(std::is_same<Transform, ComponentType>::value, "Transform can't be added.");

		std::type_index componentType = typeid(ComponentType);		
		auto cloneComponent = cloneFrom->EngineComponents.find(componentType);

		if(cloneComponent == cloneFrom->EngineComponents.end() || EngineComponents.find(componentType) != EngineComponents.end())
			return nullptr;
		else
		{
			auto result = EngineComponents.insert({ componentType,std::make_unique<ComponentType>() });
			ComponentType* newComponent = static_cast<ComponentType*>(cloneComponent.second.get())->Clone();
			result.first->second.reset(newComponent);
			return newComponent;
		}
	}

	//Returns newly added child.
	Entity* AddChild(std::string name);

	//Returns newly added child.
	Entity* AddChild(Entity* entity);

	Entity* GetParent() const;

	size_t GetChildrenCount() const;

	size_t GetDescendantCount() const;

	//If the calling entity is the root then returns itself.
	Entity* GetRoot() const;

	Transform* GetTransform() const;

	/*//If the entity is an instance of a prefab returns prefab, otherwise returns nullptr.
	//If the caller entity is a prefab returns itself.
	const Entity* GetPrefab() const;

	//If the entity is an instance of a prefab returns instance count, otherwise returns 0.
	unsigned int GetInstanceCount() const;*/
public:
	std::string name;
private:
	Entity* parent = nullptr;
	std::vector<std::unique_ptr<Entity>> Children;
	std::unordered_map<std::type_index, std::unique_ptr<Component>> Components;
	std::unordered_map<std::type_index, std::unique_ptr<Component>> EngineComponents;
	std::unique_ptr<Transform> transform;
	//Prefab
	/*const Entity* prefab = nullptr;
	unsigned int instanceCount = 0u;*/
private:
	inline static std::unordered_map<std::string, std::unique_ptr<Entity>> Prefabs;
};