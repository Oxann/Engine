#pragma once
#include <d3d11.h>
#include <wrl.h>

class Renderer;

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
	virtual ~RenderQueueBase() = default;
	virtual void Render() = 0;
};