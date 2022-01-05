#pragma once
#include "Renderer.h"
#include "RenderQueueOpaque.h"
#include "RenderQueueTransparent.h"
#include "RenderQueueWireframe.h"
#include "RenderQueueOutline.h"
#include "Camera.h"
#include "Skybox.h"
#include <vector>


class RendererManager
{
	friend class Engine;
	friend class Scene;
	friend Renderer;
	friend class EditorEntityWindow;
	friend class Entity;
	friend PointLight;
	friend DirectionalLight;
private:
	RendererManager();
	void Update();
	void UpdateShadowMaps();
public:
	unsigned long long meshCount;
	unsigned long long vertexCount;
	unsigned long long triangleCount;

	Camera* activeCamera = nullptr;
private:	
	std::vector<Renderer*> renderers;
	std::vector<const DirectionalLight*> directionalLights;
	std::vector<const PointLight*> pointLights;

	//Render Queues
	RenderQueueOpaque renderQueueOpaque;
	RenderQueueTransparent renderQueueTransparent;
	RenderQueueWireframe renderQueueWireframe;
	RenderQueueOutline renderQueueOutline;

	//Frustum
	DirectX::BoundingFrustum frustum;

	//Skybox
	Skybox* skybox = nullptr;
};
