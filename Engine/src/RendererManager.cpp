#include "RendererManager.h"
#include "Editor.h"
#include "Graphics.h"
#include "MainWindow.h"
#include "Entity.h"
#include <DirectXMath.h>
#include "EngineAssert.h"

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
	activeCamera->UpdateMatrices();
#endif
	vertexCount = 0;
	meshCount = 0;
	triangleCount = 0;

	//Updating per frame constant buffer
	UpdateVertexShaderPerFrameBuffer();

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

	UpdateDirectionalLights();
	if (directionalLights.size() > 0)
		directionalLights[0]->shadowMap.BindAsShaderResource();

	UpdatePointLights();

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
	static VS_ConstantBuffer<VS_CBUFFER_SLOT0> vs_cbuffer_slot0(&vs_cbuffer_slot0_data,1,0,D3D11_USAGE::D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_WRITE,true);

	for (int i = 0; i < directionalLights.size(); i++)
	{
		directionalLights[i]->shadowMap.BindAsDepthBuffer();
		
		DirectX::XMMATRIX lightViewMatrix = DirectX::XMMatrixTranspose(DirectX::XMMatrixRotationQuaternion(directionalLights[i]->GetTransform()->GetWorldQuaternion()));
		
#ifdef EDITOR
		DirectX::XMMATRIX viewToLightSpace = Editor::EditorCamera::rotationMatrix * DirectX::XMMatrixTranslationFromVector(Editor::EditorCamera::position) * lightViewMatrix;
#else
		DirectX::XMMATRIX viewToLightSpace = activeCamera->GetTransform()->GetWorldMatrix()* lightViewMatrix;
#endif
		DirectX::XMFLOAT3 cascadeCenterInViewSpace;
		DirectX::XMVECTOR _cascadeCenterInViewSpace = DirectX::XMVectorSet(0.0f, 0.0f, directionalLights[i]->shadowDistance * 0.5f, 1.0f);
		_cascadeCenterInViewSpace = DirectX::XMVector4Transform(_cascadeCenterInViewSpace, viewToLightSpace);
		DirectX::XMStoreFloat3(&cascadeCenterInViewSpace, _cascadeCenterInViewSpace);

		DirectX::XMFLOAT3 min = {	cascadeCenterInViewSpace.x - directionalLights[i]->shadowDistance * 0.5f, 
									cascadeCenterInViewSpace.y - directionalLights[i]->shadowDistance * 0.5f, 
									cascadeCenterInViewSpace.z - directionalLights[i]->shadowDistance * 0.5f };
		
		DirectX::XMFLOAT3 max = {	cascadeCenterInViewSpace.x + directionalLights[i]->shadowDistance * 0.5f, 
									cascadeCenterInViewSpace.y + directionalLights[i]->shadowDistance * 0.5f, 
									cascadeCenterInViewSpace.z + directionalLights[i]->shadowDistance * 0.5f };

		float step = directionalLights[i]->shadowDistance / directionalLights[i]->GetShadowResolutionWidth();
		min.x = step * std::floor(min.x / step);
		max.x = step * std::floor(max.x / step);
		min.y = step * std::floor(min.y / step);
		max.y = step * std::floor(max.y / step);

		DirectX::XMMATRIX proj = DirectX::XMMatrixOrthographicOffCenterLH(min.x, max.x, min.y, max.y, min.z, max.z);
		
		for (int j = 0; j < renderers.size(); j++)
		{
			Renderer* currentRenderer = renderers[j];
			
			if (currentRenderer->castShadows)
			{
				currentRenderer->lightSpaceMatrix = DirectX::XMMatrixTranspose(DirectX::XMMatrixTranspose(currentRenderer->GetWorldMatrix()) * lightViewMatrix * proj);
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

void RendererManager::UpdateVertexShaderPerFrameBuffer()
{
	Shader::VertexShaderPerFrameBuffer::buffer.view = DirectX::XMMatrixTranspose(Graphics::GetViewMatrix());
	Shader::VertexShaderPerFrameBuffer::buffer.projection = DirectX::XMMatrixTranspose(Graphics::GetProjectionMatrix());
	Shader::GetVertexShaderPerFrameBuffer()->ChangeData(&Shader::VertexShaderPerFrameBuffer::buffer);
}

void RendererManager::UpdateDirectionalLights()
{
	directionalLights_TO_GPU.Count = directionalLights.size();

	for (unsigned int i = 0; i < DirectionalLights_TO_GPU::maxCount && i < directionalLights_TO_GPU.Count; i++)
	{
		const DirectionalLight& currentLight = *directionalLights[i];
		directionalLights_TO_GPU.lights[i].depthBias = currentLight.depthBias;

		DirectX::XMMATRIX lightModelView = DirectX::XMMatrixMultiply(
			DirectX::XMMatrixRotationQuaternion(currentLight.GetEntity()->GetTransform()->GetWorldQuaternion()), Graphics::GetViewMatrix());

		DirectX::XMStoreFloat3(&directionalLights_TO_GPU.lights[i].direction, DirectX::XMVector3Normalize(lightModelView.r[2]));

		directionalLights_TO_GPU.lights[i].light = { currentLight.color.x * currentLight.intensity,
									currentLight.color.y * currentLight.intensity,
									currentLight.color.z * currentLight.intensity };
	
		directionalLights_TO_GPU.lights->shadowType = (int)currentLight.GetShadowType();
	}

	directionalLightsBuffer.ChangeData(&directionalLights_TO_GPU);
}

void RendererManager::UpdatePointLights()
{
	pointLights_TO_GPU.Count = pointLights.size();
	pointLights_TO_GPU.Constant = PointLight::constant;
	pointLights_TO_GPU.Linear = PointLight::linear;
	pointLights_TO_GPU.Quadratic = PointLight::quadratic;

	for (unsigned int i = 0; i < PointLight::MaxCount && i < pointLights_TO_GPU.Count; i++)
	{
		const PointLight& currentLight = *pointLights[i];
		DirectX::XMFLOAT3 worldPos = currentLight.GetEntity()->GetTransform()->GetWorldPosition();

		//Light calculations in view space
		DirectX::XMVECTOR pos = DirectX::XMVector3Transform(DirectX::XMLoadFloat3(&worldPos), Graphics::GetViewMatrix());
		DirectX::XMStoreFloat3(&pointLights_TO_GPU.lights[i].position, pos);

		pointLights_TO_GPU.lights[i].light = {
			currentLight.color.x * currentLight.intensity,
			currentLight.color.y * currentLight.intensity,
			currentLight.color.z * currentLight.intensity
		};
	}

	pointLightsBuffer.ChangeData(&pointLights_TO_GPU);
}
