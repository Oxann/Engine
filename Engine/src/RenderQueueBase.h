#pragma once
#include <d3d11.h>
#include <wrl.h>

class RendererBase;
class RendererManager;

class RenderQueueBase
{
public:
	struct RenderQueueBaseItem
	{
		RenderQueueBaseItem(RendererBase* renderer, unsigned int subMeshIndex)
			:renderer(renderer), subMeshIndex(subMeshIndex)
		{}
		RendererBase* renderer;
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