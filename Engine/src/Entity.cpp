#include "Entity.h"
#include "EngineException.h"
#include "Transform.h"
#include "Renderer.h"
#include "Scene.h"

Entity::Entity(const std::string& name)
{
	this->name = name;
	Transform_ = std::make_unique<Transform>();
	static_cast<Component*>(Transform_.get())->entity = this;
}

Entity* Entity::Clone()
{
	Entity* newEntity = new Entity;
	newEntity->name = name;

	//Transform
	newEntity->Transform_ = std::unique_ptr<Transform>(this->Transform_->Clone());
	static_cast<Component*>(newEntity->Transform_.get())->entity = newEntity;

	//Renderer
	if (Renderer_)
	{
		newEntity->Renderer_ = std::unique_ptr<RendererBase>(this->Renderer_->Clone());
		static_cast<Component*>(newEntity->Renderer_.get())->entity = newEntity;
		static_cast<Component*>(newEntity->Renderer_.get())->transform = newEntity->Transform_.get();
	}

	for (auto& component : Components)
	{
		auto newComponent = newEntity->Components.insert({ component.first,std::unique_ptr<Component>(component.second->Clone()) });
		newComponent.first->second->entity = newEntity;
		newComponent.first->second->transform = newEntity->Transform_.get();
	}

	for (int i = 0; i < Children.size(); i++)
	{
		newEntity->Children.push_back(std::unique_ptr<Entity>(this->Children[i]->Clone()));
		newEntity->Children.back()->parent = newEntity;
	}

	return newEntity;
}

void Entity::Update()
{
	if (Renderer_ && Renderer_->state == Component::State::START)
	{
		((Component*)Renderer_.get())->Start();
		Renderer_->state = Component::State::UPDATE;
	}

	for (auto& component : Components)
	{
		if (component.second->state == Component::State::START)
		{
			component.second->Start();
			component.second->state = Component::State::UPDATE;
		}

		component.second->Update();
	}

	for (const auto& child : Children)
	{
		child->Update();
	}
}

Entity* Entity::FindPrefab(std::string_view name)
{
	auto result = Prefabs.find(name.data());
	if (result == Prefabs.end())
	{
		std::stringstream msg;
		msg << name << " is not a Prefab.";
		THROW_ENGINE_EXCEPTION(msg.str(), true);
	}
	return result->second.get();
}


Entity* Entity::NewPrefab(std::string_view name)
{
	auto newPrefab = Prefabs.emplace(std::piecewise_construct, std::forward_as_tuple(name), std::forward_as_tuple(new Entity(name.data())));
	
	if (newPrefab.second)
		return newPrefab.first->second.get();
	else
		return nullptr;
}

Entity* Entity::NewPrefab(Entity* cloneFrom)
{
	if (cloneFrom)
	{
		auto newPrefab = Prefabs.emplace(std::piecewise_construct, std::forward_as_tuple(cloneFrom->name), std::forward_as_tuple(cloneFrom->Clone()));
	
		if (newPrefab.second)
			return newPrefab.first->second.get();
		else
			return nullptr;
	}
	else
		return nullptr;
}

Entity* Entity::AddChild(std::string_view name)
{
	Children.push_back(std::make_unique<Entity>(name.data()));
	Entity* newChild = Children.back().get();
	newChild->parent = this;
	return newChild;
}

Entity* Entity::AddChild(Entity* entity)
{
	std::unique_ptr<Entity> newChild(entity->Clone());
	newChild->parent = this;
	Children.push_back(std::move(newChild));

	return Children.back().get();
}

Entity* Entity::GetChild(std::string_view name) const
{
	for (const auto& child : Children)
	{
		if (name == child->name)
			return child.get();
	}
	
	return nullptr;
}

Entity* Entity::GetChild(unsigned int index) const
{
	return Children[index].get();
}

Entity* Entity::GetDescendant(std::string_view name)
{
	for (const auto& child : Children)
	{
		if (child->name == name)
			return child.get();

		Entity* descendant = child->GetDescendant(name);
		
		if (descendant)
			return descendant;
	}

	return nullptr;
}

Entity* Entity::GetParent() const
{
	return parent;
}

size_t Entity::GetChildrenCount() const
{
	return Children.size();
}

size_t Entity::GetDescendantCount() const
{
	size_t count = Children.size();
	for (int i = 0; i < Children.size(); i++)
	{
		count += Children[i]->GetDescendantCount();
	}
	return count;
}

Entity* Entity::GetRoot() const
{
	Entity* temp = const_cast<Entity*>(this);

	while (temp->parent != nullptr)
		temp = temp->parent;

	return temp;
}

Transform* Entity::GetTransform() const
{
	return Transform_.get();
}
