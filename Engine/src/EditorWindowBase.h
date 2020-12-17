#pragma once
#include <string>

class EditorWindowBase
{
	friend class Editor;
public:
	virtual void Start() = 0;
	virtual void Update() = 0;
	void Activate() { isActive = true; }
	void Deactivate() { isActive = false; }
protected:
	std::string name;
	bool isActive = true;
};