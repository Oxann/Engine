#pragma once
#include "Renderer.h"
#include "RenderQueueOpaque.h"
#include "RenderQueueTransparent.h"
#include "RenderQueueWireframe.h"
#include "RenderQueueOutline.h"
#include <vector>


class RendererManager
{
	friend class Engine;
	friend class Scene;
	friend Renderer;
	friend class EditorEntityWindow;
	friend class Entity;
private:
	RendererManager();
	void Update();
public:
	unsigned long long meshCount;
	unsigned long long vertexCount;
	unsigned long long triangleCount;

private:
	std::vector<Renderer*> renderers;

	//Render Queues
	RenderQueueOpaque renderQueueOpaque;
	RenderQueueTransparent renderQueueTransparent;
	RenderQueueWireframe renderQueueWireframe;
	RenderQueueOutline renderQueueOutline;
};
