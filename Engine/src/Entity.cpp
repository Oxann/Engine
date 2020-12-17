#include "Entity.h"
#include "EngineException.h"

Entity::Entity(std::string name)
{
	this->name = name;
	transform = std::make_unique<Transform>();
	Component* temp = transform.get();
	temp->entity = this;
}

Entity* Entity::Clone()
{
	Entity* newEntity = new Entity;
	newEntity->name = name;
	newEntity->transform = std::unique_ptr<Transform>(this->transform->Clone());
	Component* temp = newEntity->transform.get();
	temp->entity = newEntity;


	for (auto& engineComponent : EngineComponents)
	{
		auto newComponent = newEntity->EngineComponents.insert({ engineComponent.first,nullptr });
		Component* clone = engineComponent.second->Clone();
		clone->entity = newEntity;
		newComponent.first->second.reset(clone);
	}

	for (auto& component : Components)
	{
		auto newComponent = newEntity->Components.insert({ component.first,nullptr });
		Component* clone = component.second->Clone();
		clone->entity = newEntity;
		newComponent.first->second.reset(clone);
	}

	for (int i = 0; i < Children.size(); i++)
	{
		newEntity->Children.push_back(nullptr);
		newEntity->Children.back().reset(this->Children[i]->Clone());
		newEntity->Children.back()->parent = newEntity;
	}

	return newEntity;
}

Entity* Entity::FindPrefab(std::string name)
{
	auto result = Prefabs.find(name);
	if (result == Prefabs.end())
	{
		std::stringstream msg;
		msg << name << " is not a Prefab.";
		THROW_ENGINE_EXCEPTION(msg.str(), true);
	}
	return result->second.get();
}

void Entity::MakePrefab(Entity* entity)
{
	Entity* newPrefab = entity->Clone();
	//newPrefab->prefab = newPrefab;
	Prefabs.insert({ entity->name,std::unique_ptr<Entity>(newPrefab) });
}

Entity* Entity::AddChild(std::string name)
{
	Children.push_back(std::make_unique<Entity>(name));
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
	return transform.get();
}

/*const Entity* Entity::GetPrefab() const
{
	return prefab;
}

unsigned int Entity::GetInstanceCount() const
{
	return prefab ? prefab->instanceCount : 1u;
}*/
