#include "RenderQueueTransparent.h"
#include "Graphics.h"
#include "Renderer.h"
#include "RendererManager.h"
#include <algorithm>


RenderQueueTransparent::RenderQueueTransparent(RendererManager* const rendererManager)
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
	blendStateDesc.RenderTarget[0].BlendEnable = TRUE;
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

void RenderQueueTransparent::Add(Renderer* renderer, unsigned int index)
{
	DirectX::XMVECTOR meshCenterViewSpace = DirectX::XMVector3Transform(DirectX::XMLoadFloat3(&renderer->GetMesh()->GetSubMeshes()[index].AABB.Center), 
												   DirectX::XMMatrixTranspose(renderer->GetWorldViewMatrix()));
	queue.emplace_back(renderer, index, DirectX::XMVectorGetZ(meshCenterViewSpace));
}

void RenderQueueTransparent::Render()
{
	Graphics::pDeviceContext->RSSetState(rasterizerState.Get());
	Graphics::pDeviceContext->OMSetBlendState(blendState.Get(), NULL, 0xffffffff);
	Graphics::pDeviceContext->OMSetDepthStencilState(depthStencilState.Get(), 0u);

	//Counting rendered meshes.
	rendererManager->meshCount += queue.size();

	std::sort(queue.begin(), queue.end());

	for (const auto& renderItem : queue)
	{
		renderItem.renderer->Render(renderItem.subMeshIndex);

		//Counting rendered vertices.
		const Mesh::SubMesh& subMesh = static_cast<Renderer*>(renderItem.renderer)->GetMesh()->GetSubMeshes()[renderItem.subMeshIndex];
		rendererManager->vertexCount += subMesh.GetVertexCount();
		rendererManager->triangleCount += subMesh.GetIndexCount() / 3u;
	}

	queue.clear();
}

RenderQueueTransparent::RenderQueueTransparentItem::RenderQueueTransparentItem(Renderer* renderer, unsigned int subMeshIndex, float Z)
	:RenderQueueBaseItem(renderer, subMeshIndex),
	Z(Z)
{
}

bool RenderQueueTransparent::RenderQueueTransparentItem::operator<(const RenderQueueTransparentItem rhs)
{
	return Z > rhs.Z;
}
