#pragma once 

#include "Editor.h"
#include "Scene.h"
#include <assert.h>

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
		ImGui::Text("Mesh Count: %llu", Scene::GetActiveScene()->rendererManager.meshCount);
		ImGui::Text("Vertex Count: %llu", Scene::GetActiveScene()->rendererManager.vertexCount);
		ImGui::Text("Triangle Count: %llu", Scene::GetActiveScene()->rendererManager.triangleCount);
	}
private:
};