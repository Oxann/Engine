#pragma once
#include <d3d11.h>
#include <wrl.h>

class Renderer;
class RendererManager;

class RenderQueueBase
{
public:
	struct RenderQueueBaseItem
	{
		RenderQueueBaseItem(Renderer* renderer, unsigned int subMeshIndex)
			:renderer(renderer), subMeshIndex(subMeshIndex)
		{}
		Renderer* renderer;
		unsigned int subMeshIndex;
	};
public:
	RenderQueueBase(RendererManager* const rendererManager)
		:rendererManager(rendererManager)
	{}

	virtual ~RenderQueueBase() = default;
	virtual void Render() = 0;

protected:
	RendererManager* const rendererManager;
};