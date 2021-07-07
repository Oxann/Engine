#pragma once

class Transform;
class Renderer;
class Entity;

class Component
{
public:
	enum class State
	{
		START,
		UPDATE
	};

	friend Entity;
public:
	Component() = default;
	virtual ~Component() = default;
	Component(const Component& component) = default;
	Component& operator=(const Component&) = default;
	Component(Component&& component) = default;
	Component& operator=(Component&&) = default;
	virtual void Start() {}
	virtual void Update() {}
	Entity* GetEntity() const { return entity; }
	Transform* GetTransform() const { return transform; };
	virtual Component* Clone() = 0;

public:
	State state = State::START;
private:
	Entity* entity = nullptr;
	Transform* transform = nullptr;
};