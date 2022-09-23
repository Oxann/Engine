#pragma once
#include "RenderQueueBase.h"
#include <vector>
 
class RenderQueueWireframe : public RenderQueueBase
{
public:
	RenderQueueWireframe(RendererManager* const rendererManager);
	void Add(RendererBase* renderer, unsigned int index);
	void Render() override;
private:
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizerState;
	Microsoft::WRL::ComPtr<ID3D11BlendState> blendState;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthStencilState;

	std::vector<RenderQueueBaseItem> queue;
};