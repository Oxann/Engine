#include "RenderQueueOutline.h"
#include "Graphics.h"
#include "Renderer.h"

RenderQueueOutline::RenderQueueOutline()
{
	queue.reserve(100u);

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

	//Depth Stencil States
	D3D11_DEPTH_STENCIL_DESC depthStencilStateMaskObjectDesc = {};
	depthStencilStateMaskObjectDesc.DepthEnable = TRUE;
	depthStencilStateMaskObjectDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilStateMaskObjectDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	depthStencilStateMaskObjectDesc.StencilEnable = TRUE;
	depthStencilStateMaskObjectDesc.StencilWriteMask = 0xffu;
	depthStencilStateMaskObjectDesc.StencilReadMask = 0xffu;
	depthStencilStateMaskObjectDesc.BackFace.StencilFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_NEVER;
	depthStencilStateMaskObjectDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP::D3D11_STENCIL_OP_REPLACE;
	depthStencilStateMaskObjectDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP::D3D11_STENCIL_OP_REPLACE;
	depthStencilStateMaskObjectDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP::D3D11_STENCIL_OP_REPLACE;
	depthStencilStateMaskObjectDesc.FrontFace.StencilFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_NEVER;
	depthStencilStateMaskObjectDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP::D3D11_STENCIL_OP_REPLACE;
	depthStencilStateMaskObjectDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP::D3D11_STENCIL_OP_REPLACE;
	depthStencilStateMaskObjectDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP::D3D11_STENCIL_OP_REPLACE;
	Graphics::pDevice->CreateDepthStencilState(&depthStencilStateMaskObjectDesc, &depthStencilStateMaskObject);

	D3D11_DEPTH_STENCIL_DESC depthStencilStateDrawOutlineDesc = {};
	depthStencilStateDrawOutlineDesc.DepthEnable = FALSE;
	depthStencilStateDrawOutlineDesc.StencilEnable = TRUE;
	depthStencilStateDrawOutlineDesc.StencilWriteMask = 0xffu;
	depthStencilStateDrawOutlineDesc.StencilReadMask = 0xffu;
	depthStencilStateDrawOutlineDesc.BackFace.StencilFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_NOT_EQUAL;
	depthStencilStateDrawOutlineDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP::D3D11_STENCIL_OP_KEEP;
	depthStencilStateDrawOutlineDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP::D3D11_STENCIL_OP_KEEP;
	depthStencilStateDrawOutlineDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP::D3D11_STENCIL_OP_KEEP;
	depthStencilStateDrawOutlineDesc.FrontFace.StencilFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_NOT_EQUAL;
	depthStencilStateDrawOutlineDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP::D3D11_STENCIL_OP_KEEP;
	depthStencilStateDrawOutlineDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP::D3D11_STENCIL_OP_KEEP;
	depthStencilStateDrawOutlineDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP::D3D11_STENCIL_OP_ZERO;
	Graphics::pDevice->CreateDepthStencilState(&depthStencilStateDrawOutlineDesc, &depthStencilStateDrawOutline);
}

void RenderQueueOutline::Add(Renderer* renderer, unsigned int index)
{
	queue.emplace_back(renderer, index);
}

void RenderQueueOutline::Render()
{
	static Material* outlineMaterial = Resources::FindMaterial("$Default\\outlineMaterial");

	Graphics::pDeviceContext->RSSetState(rasterizerState.Get());
	Graphics::pDeviceContext->OMSetBlendState(blendState.Get(), NULL, 0xffffffff);
	
	for (const auto& renderItem : queue)
	{
		//Masking
		Graphics::pDeviceContext->OMSetDepthStencilState(depthStencilStateMaskObject.Get(), 1u);
		renderItem.renderer->Render(renderItem.subMeshIndex, outlineMaterial);
	
		//Drawing outline
		Graphics::pDeviceContext->OMSetDepthStencilState(depthStencilStateDrawOutline.Get(), 1u);

		DirectX::XMMATRIX MVP = renderItem.renderer->GetWorldViewProjectionMatrix();

		renderItem.renderer->worldViewProjectionMatrix = MVP * DirectX::XMMatrixScaling(1.01f, 1.01f, 1.01f); //Scaling MVP by a factor, !!!MVP is column order!!!
		renderItem.renderer->Render(renderItem.subMeshIndex, outlineMaterial);
		
		renderItem.renderer->worldViewProjectionMatrix = MVP * DirectX::XMMatrixScaling(0.99f, 0.99f, 0.99f); //Scaling MVP by a factor, !!!MVP is column order!!!
		renderItem.renderer->Render(renderItem.subMeshIndex, outlineMaterial);
	}

	queue.clear();
}
