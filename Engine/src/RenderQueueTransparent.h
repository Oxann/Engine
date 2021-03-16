#pragma once

#include "RenderQueueBase.h"

#include <vector>

class RenderQueueTransparent : public RenderQueueBase
{
private:
	struct RenderQueueTransparentItem : public RenderQueueBaseItem
	{
		RenderQueueTransparentItem(Renderer* renderer, unsigned int subMeshIndex, float distanceSq);

		bool operator<(const RenderQueueTransparentItem rhs);

		float Z = 0.0f;
	};
public:
	RenderQueueTransparent();
	void Add(Renderer* renderer, unsigned int index);
	virtual void Render() override;
private:
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizerState;
	Microsoft::WRL::ComPtr<ID3D11BlendState> blendState;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthStencilState;

	std::vector<RenderQueueTransparentItem> queue;
};