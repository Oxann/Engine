#pragma once

#include <vector>
#include <type_traits>
#include <memory>
#include <string>
#include <typeindex>
#include <unordered_map>

#include "Component.h"
#include "Transform.h"
#include "RendererBase.h"

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
	static Entity* FindPrefab(std::string_view name);

	static Entity* NewPrefab(std::string_view name);

	static Entity* NewPrefab(Entity* cloneFrom);
public:
	template<class ComponentType>
	ComponentType* GetComponent() const
	{
		static_assert(std::is_base_of<Component, ComponentType>::value,"All components must be derived from Component.");
		static_assert(!std::is_same<Transform, ComponentType>::value, "Use GetTransform instead.");
		
		if constexpr (std::is_base_of<RendererBase, ComponentType>::value)
		{
			return dynamic_cast<ComponentType*>(Renderer_.get());
		}
		else
		{
			auto result = Components.find(typeid(ComponentType));
			if (result == Components.end())
				return nullptr;
			else
			{
				return static_cast<ComponentType*>(result->second.get());
			}
		}
	}
	
	//Returns newly added component.
	template<class ComponentType>
	ComponentType* AddComponent()
	{		
		static_assert(std::is_base_of<Component, ComponentType>::value, "All components must be derived from Component.");
		static_assert(!std::is_same<Transform, ComponentType>::value, "Transform can't be added.");

		if constexpr (std::is_base_of<RendererBase, ComponentType>::value)
		{
			if (!Renderer_)
			{
				Renderer_ = std::make_unique<ComponentType>();

				static_cast<Component*>(Renderer_.get())->entity = this;
				static_cast<Component*>(Renderer_.get())->transform = this->Transform_.get();

				return static_cast<ComponentType*>(Renderer_.get());
			}
			else
				return nullptr;
		}
		else
		{
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
	}

	//Clones the component from given entity.
	//If the given entity doesn't have the component or this entity already has the component,it will return nullptr.
	template <class ComponentType>
	ComponentType* AddComponent(const Entity* cloneFrom)
	{
		static_assert(std::is_base_of<Component, ComponentType>::value, "All components must be derived from Component.");
		static_assert(std::is_same<Transform, ComponentType>::value, "Transform can't be added.");
		
		if constexpr (std::is_base_of<RendererBase, ComponentType>::value)
		{
			if (!Renderer_)
			{
				Renderer_.reset(cloneFrom->GetComponent<ComponentType>()->Clone());

				static_cast<Component*>(Renderer_.get())->entity = this;
				static_cast<Component*>(Renderer_.get())->transform = this->Transform_.get();
			}
			return static_cast<ComponentType*>(Renderer_.get());
		}
		else
		{
			std::type_index componentType = typeid(ComponentType);
			auto cloneComponent = cloneFrom->Components.find(componentType);

			if (cloneComponent == cloneFrom->Components.end() || Components.find(componentType) != Components.end())
				return nullptr;
			else
			{
				auto result = Components.insert({ componentType, std::unique_ptr<Component>(cloneComponent.second.get()->Clone()) });
				result.first->second->entity = this;
				result.first->second->transform = Transform_.get();
				return static_cast<ComponentType*>(result.first->second.get());
			}
		}
	}

	//Returns newly added child.
	Entity* AddChild(std::string_view name);

	//Returns newly added child.
	Entity* AddChild(Entity* entity);

	Entity* GetChild(std::string_view name) const;

	Entity* GetChild(unsigned int index) const;

	Entity* GetDescendant(std::string_view name);

	Entity* GetParent() const;

	size_t GetChildrenCount() const;

	size_t GetDescendantCount() const;

	//If the caller is the root then returns itself.
	Entity* GetRoot() const;

	Transform* GetTransform() const;
public:
	std::string name;
private:
	Entity* parent = nullptr;
	std::vector<std::unique_ptr<Entity>> Children;
	std::unordered_map<std::type_index, std::unique_ptr<Component>> Components;
	std::unique_ptr<Transform> Transform_;
	std::unique_ptr<RendererBase> Renderer_ = nullptr;
private:
	inline static std::unordered_map<std::string, std::unique_ptr<Entity>> Prefabs;
};