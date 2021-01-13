#pragma once 

#include "Editor.h"

class EditorMetricsWindow : public EditorWindowBase
{
public:
	void Start() override
	{
		name = "Metrics";
		isActive = false;
	}

	void Update() override
	{
		ImGui::Text("FPS: %f", Editor::imguiIO->Framerate);
	}
private:
};