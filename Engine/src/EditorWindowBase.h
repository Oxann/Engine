#pragma once
#include <string>

class EditorWindowBase
{
	friend class Editor;
public:
	virtual void Start() = 0;
	virtual void Update() = 0;
public:
	std::string name;
	bool isActive = true;
};