#pragma once
#include "RenderQueueBase.h"
#include <vector>


class RenderQueueOutline : public RenderQueueBase
{
public:
	RenderQueueOutline(RendererManager* const rendererManager);
	void Add(RendererBase* renderer, unsigned int index);
	void Render() override;
private:
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizerState;
	Microsoft::WRL::ComPtr<ID3D11BlendState> blendState;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthStencilStateMaskObject;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthStencilStateDrawOutline;

	std::vector<RenderQueueBaseItem> queue;
};