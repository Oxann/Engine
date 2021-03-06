#include "RenderQueueOpaque.h"
#include "Graphics.h"
#include "Renderer.h"
#include "RendererManager.h"
#include "Scene.h"

RenderQueueOpaque::RenderQueueOpaque(RendererManager* const rendererManager)
	:RenderQueueBase(rendererManager)
{
	queue.reserve(500u);

	//Rasterizer States
	D3D11_RASTERIZER_DESC rasterizerStateDesc;
	ZeroMemory(&rasterizerStateDesc, sizeof(D3D11_RASTERIZER_DESC));
	rasterizerStateDesc.FillMode = D3D11_FILL_MODE::D3D11_FILL_SOLID;
	rasterizerStateDesc.CullMode = D3D11_CULL_BACK;
	rasterizerStateDesc.FrontCounterClockwise = FALSE;
	rasterizerStateDesc.DepthClipEnable = TRUE;
	Graphics::pDevice->CreateRasterizerState(&rasterizerStateDesc, &rasterizerState);

	//Blend State
	D3D11_BLEND_DESC blendStateDesc;
	blendStateDesc.AlphaToCoverageEnable = FALSE;
	blendStateDesc.IndependentBlendEnable = FALSE;
	blendStateDesc.RenderTarget[0].BlendEnable = FALSE;
	blendStateDesc.RenderTarget[0].SrcBlend = D3D11_BLEND::D3D11_BLEND_SRC_ALPHA;
	blendStateDesc.RenderTarget[0].DestBlend = D3D11_BLEND::D3D11_BLEND_INV_SRC_ALPHA;
	blendStateDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP::D3D11_BLEND_OP_ADD;
	blendStateDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND::D3D11_BLEND_ONE;
	blendStateDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND::D3D11_BLEND_ZERO;
	blendStateDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP::D3D11_BLEND_OP_ADD;
	blendStateDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE::D3D11_COLOR_WRITE_ENABLE_ALL;

	Graphics::pDevice->CreateBlendState(&blendStateDesc, &blendState);


	//Depth Stencil State
	D3D11_DEPTH_STENCIL_DESC depthStencilStateDesc = {};
	depthStencilStateDesc.DepthEnable = TRUE;
	depthStencilStateDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilStateDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	depthStencilStateDesc.StencilEnable = FALSE;

	Graphics::pDevice->CreateDepthStencilState(&depthStencilStateDesc, &depthStencilState);
}

void RenderQueueOpaque::Add(Renderer* renderer, unsigned int index)
{
	queue.emplace_back(renderer, index);
}

void RenderQueueOpaque::Render()
{
	Graphics::pDeviceContext->RSSetState(rasterizerState.Get());
	Graphics::pDeviceContext->OMSetBlendState(blendState.Get(), NULL, 0xffffffff);
	Graphics::pDeviceContext->OMSetDepthStencilState(depthStencilState.Get(), 0u);

	//Counting rendered meshes.
	rendererManager->meshCount += queue.size();

	for (const auto& renderItem : queue)
	{
		renderItem.renderer->Render(renderItem.subMeshIndex);

		//Counting rendered vertices.
		const Mesh::SubMesh& subMesh = renderItem.renderer->GetMesh()->GetSubMeshes()[renderItem.subMeshIndex];
		rendererManager->vertexCount += subMesh.GetVertexCount();
		rendererManager->triangleCount += subMesh.GetIndexCount() / 3u;
	}

	queue.clear();
}
