#include "Entity.h"
#include "EngineException.h"
#include "Transform.h"
#include "Renderer.h"
#include "Scene.h"

Entity::Entity(std::string name)
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
		newEntity->Renderer_ = std::unique_ptr<Renderer>(this->Renderer_->Clone());
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

void Entity::Start()
{
	if (Renderer_)
		((Component*)Renderer_.get())->Start();

	for (const auto& component : Components)
	{
		component.second->Start();
	}

	for (const auto& child : Children)
	{
		child->Start();
	}
}

void Entity::Update()
{
	for (auto& component : Components)
	{
		component.second->Update();
	}

	for (const auto& child : Children)
	{
		child->Update();
	}
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
	Prefabs.insert({ entity->name,std::unique_ptr<Entity>(entity->Clone()) });
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
	return Transform_.get();
}

Renderer* Entity::GetRenderer() const
{
	return Renderer_.get();
}

Renderer* Entity::AddRenderer(Entity* cloneFrom)
{
	if (!Renderer_)
	{
		if (cloneFrom)
			Renderer_.reset(cloneFrom->GetRenderer()->Clone());
		else
			Renderer_ = std::make_unique<Renderer>();

		static_cast<Component*>(Renderer_.get())->entity = this;
		static_cast<Component*>(Renderer_.get())->transform = this->Transform_.get();
	}
	return Renderer_.get();
}
