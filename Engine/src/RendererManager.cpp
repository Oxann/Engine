#include "RendererManager.h"
#include "Editor.h"
#include "Graphics.h"
#include "MainWindow.h"
#include "Entity.h"

RendererManager::RendererManager()
	: meshCount(0),
	  vertexCount(0),
	  triangleCount(0),
	  renderQueueOpaque(this),
	  renderQueueTransparent(this),
	  renderQueueWireframe(this),
	  renderQueueOutline(this)
{
}

void RendererManager::Update()
{
#ifndef EDITOR
	activeCamera->UpdateViewMatrix();
#endif
	vertexCount = 0;
	meshCount = 0;
	triangleCount = 0;

	//Creating frustum.
	DirectX::BoundingFrustum::CreateFromMatrix(frustum,Graphics::GetProjectionMatrix());

	for (const auto& renderer : renderers)
	{
		renderer->Update();
	}

	UpdateShadowMaps();

	Graphics::pDeviceContext->ClearRenderTargetView(Graphics::pView.Get(), Graphics::clearColor);
	Graphics::pDeviceContext->ClearDepthStencilView(Graphics::pDepthStencil.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0u);
	Graphics::pDeviceContext->OMSetRenderTargets(1u, Graphics::pView.GetAddressOf(), Graphics::pDepthStencil.Get());
	Graphics::pDeviceContext->RSSetViewports(1, &Graphics::viewport);

	if (directionalLights.size() > 0)
		directionalLights[0]->shadowMap.BindAsShaderResource();

	//Render queues
	renderQueueOpaque.Render();
	renderQueueTransparent.Render();

	if (skybox)
		skybox->Draw();

#ifdef EDITOR
	renderQueueWireframe.Render();
	renderQueueOutline.Render();
	Editor::Render();
#endif

	//In debug mode present is not working when window is minimized.
#ifndef NDEBUG
	if (MainWindow::isMinimized())
	{
		CHECK_DX_ERROR(Graphics::pSwapChain->Present(Graphics::isVSyncEnabled, 0u));
	}
#else
	Graphics::pSwapChain->Present(Graphics::isVSyncEnabled, 0u);
#endif
}

void RendererManager::UpdateShadowMaps()
{
	static ShaderView shadowMapShader(Resources::FindShader("ShadowCast"));
	shadowMapShader.Bind();
	Graphics::pDeviceContext->OMSetRenderTargets(0u, nullptr, nullptr);

	struct VS_CBUFFER_SLOT0
	{
		DirectX::XMMATRIX MVP = DirectX::XMMatrixIdentity();
	};

	static VS_CBUFFER_SLOT0 vs_cbuffer_slot0_data;
	static VS_ConstantBuffer<VS_CBUFFER_SLOT0> vs_cbuffer_slot0(&vs_cbuffer_slot0_data,1,0,D3D11_USAGE::D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_WRITE);
	static DirectX::XMMATRIX projectionMatrix = DirectX::XMMatrixOrthographicLH(64.0f, 64.0f, -50.0f, 50.0f);

	vs_cbuffer_slot0.BindPipeline();

	for (int i = 0; i < directionalLights.size(); i++)
	{
		directionalLights[i]->shadowMap.BindAsDepthBuffer();
		DirectX::XMMATRIX lightViewMatrix = DirectX::XMMatrixTranspose(DirectX::XMMatrixRotationQuaternion(directionalLights[i]->GetTransform()->GetWorldQuaternion()));
		
		for (int j = 0; j < renderers.size(); j++)
		{
			Renderer* currentRenderer = renderers[j];

			if (currentRenderer->castShadows)
			{
				currentRenderer->lightSpaceMatrix = DirectX::XMMatrixTranspose(DirectX::XMMatrixTranspose(currentRenderer->GetWorldMatrix()) * lightViewMatrix * projectionMatrix);
				vs_cbuffer_slot0_data.MVP = currentRenderer->lightSpaceMatrix;
				vs_cbuffer_slot0.ChangeData(&vs_cbuffer_slot0_data);

				const auto& subMeshes = currentRenderer->mesh->GetSubMeshes();
				for (int subMeshIndex = 0; subMeshIndex < subMeshes.size(); subMeshIndex++)
				{
					subMeshes[subMeshIndex].GetVertexElement(VertexBuffer::ElementType::Position3D)->BindPipeline();
					subMeshes[subMeshIndex].GetIndexBuffer()->BindPipeline();
					Graphics::pDeviceContext->DrawIndexed(subMeshes[subMeshIndex].GetIndexCount(), 0, 0);
				}
			}
		}
	}
}
