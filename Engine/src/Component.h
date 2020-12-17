#pragma once


class Component
{
private:
	friend class Entity;
	friend class Scene;

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
	virtual void Start() = 0;
	virtual void Update() = 0;
	inline Entity* GetEntity() const
	{
		return entity;
	}
	virtual Component* Clone() = 0;
private:
	Entity* entity = nullptr;
	State state = State::Start;
};