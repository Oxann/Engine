#pragma once

class Transform;
class Renderer;
class Entity;

class Component
{
	friend Entity;
public:
	enum class State : unsigned char
	{
		Start = 0,
		Update = 1
	};
public:
	Component() = default;
	virtual ~Component() = default;
	Component(const Component& component) = delete;
	Component& operator=(const Component&) = delete;
	Component(Component&& component) = delete;
	Component& operator=(Component&&) = delete;
	virtual void Start() {}
	virtual void Update() {}
	Entity* GetEntity() const { return entity; }
	Transform* GetTransform() const { return transform; };
	virtual Component* Clone() = 0;
public:
	State state = State::Start;
private:
	Entity* entity = nullptr;
	Transform* transform = nullptr;
};