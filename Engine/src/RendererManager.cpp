#include "RendererManager.h"
#include "Editor.h"
#include "Graphics.h"
#include "MainWindow.h"
#include "Entity.h"
#include <DirectXMath.h>
#include "EngineAssert.h"
#include <wrl.h>
#include "Scene.h"

RendererManager::RendererManager()
	: meshCount(0),
	  vertexCount(0),
	  triangleCount(0),
	  renderQueueOpaque(this),
	  renderQueueTransparent(this),
	  renderQueueWireframe(this),
	  renderQueueOutline(this)
{
	using namespace Microsoft::WRL;

	ComPtr<ID3D11Texture2D> finalOutput;
	D3D11_TEXTURE2D_DESC desc = {};
	desc.Width = MainWindow::GetDisplayResolution().width;
	desc.Height = MainWindow::GetDisplayResolution().height;
	desc.MipLevels = 1u;
	desc.ArraySize = 1u;
	desc.Format = DXGI_FORMAT::DXGI_FORMAT_R16G16B16A16_FLOAT;
	desc.SampleDesc.Count = 1u;
	desc.SampleDesc.Quality = 0u;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;
	CHECK_DX_ERROR(Graphics::pDevice->CreateTexture2D(&desc, NULL, &finalOutput));

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R16G16B16A16_FLOAT;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = -1;
	srvDesc.Texture2D.MostDetailedMip = 0;
	CHECK_DX_ERROR(Graphics::pDevice->CreateShaderResourceView(finalOutput.Get(), &srvDesc, &postProcessInputTexture));

	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Filter = D3D11_FILTER::D3D11_FILTER_MIN_MAG_MIP_POINT;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MinLOD = 0.0f;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	samplerDesc.MaxAnisotropy = 0;
	CHECK_DX_ERROR(Graphics::pDevice->CreateSamplerState(&samplerDesc, &postProcessSampler));

	D3D11_RASTERIZER_DESC rasterizerStateDesc;
	ZeroMemory(&rasterizerStateDesc, sizeof(D3D11_RASTERIZER_DESC));
	rasterizerStateDesc.FillMode = D3D11_FILL_MODE::D3D11_FILL_SOLID;
	rasterizerStateDesc.CullMode = D3D11_CULL_BACK;
	rasterizerStateDesc.FrontCounterClockwise = FALSE;
	rasterizerStateDesc.DepthClipEnable = FALSE;
	Graphics::pDevice->CreateRasterizerState(&rasterizerStateDesc, &postProcessRasterizerState);

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
	CHECK_DX_ERROR(Graphics::pDevice->CreateBlendState(&blendStateDesc, &postProcessBlendState));
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
	UpdatePixelShaderPerFrameBuffer();

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

	if(directionalLights.size() > 0)
		directionalLights[0]->shadowMap.BindAsShaderResource();


	UpdatePointLights();

	//Render queues
	renderQueueOpaque.Render();
	renderQueueTransparent.Render();


	if (skybox)
		skybox->Draw();


	Tonemap();

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
	Shader::VertexShaderPerFrameBuffer::buffer.viewProjection = DirectX::XMMatrixTranspose(Graphics::GetViewMatrix() * Graphics::GetProjectionMatrix());
	Shader::GetVertexShaderPerFrameBuffer()->ChangeData(&Shader::VertexShaderPerFrameBuffer::buffer);
}

void RendererManager::UpdatePixelShaderPerFrameBuffer()
{
#ifdef EDITOR
	Shader::PixelShaderPerFrameBuffer::buffer.cameraPosition = Editor::EditorCamera::position;
#else
	DirectX::XMFLOAT3 activeCameraPosition = activeCamera->GetTransform()->GetWorldPosition();
	Shader::PixelShaderPerFrameBuffer::buffer.cameraPosition = DirectX::XMLoadFloat3(&activeCameraPosition);
#endif
	Shader::GetPixelShaderPerFrameBuffer()->ChangeData(&Shader::PixelShaderPerFrameBuffer::buffer);
}

void RendererManager::UpdateDirectionalLights()
{
	directionalLights_TO_GPU.Count = directionalLights.size();

	for (unsigned int i = 0; i < DirectionalLights_TO_GPU::maxCount && i < directionalLights_TO_GPU.Count; i++)
	{
		const DirectionalLight& currentLight = *directionalLights[i];
		directionalLights_TO_GPU.lights[i].depthBias = currentLight.depthBias;

		DirectX::XMMATRIX lightWorld = DirectX::XMMatrixRotationQuaternion(currentLight.GetEntity()->GetTransform()->GetWorldQuaternion());

		DirectX::XMStoreFloat3(&directionalLights_TO_GPU.lights[i].direction, DirectX::XMVector3Normalize(lightWorld.r[2]));

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

	for (unsigned int i = 0; i < PointLight::MaxCount && i < pointLights_TO_GPU.Count; i++)
	{
		const PointLight& currentLight = *pointLights[i];
		DirectX::XMFLOAT3 worldPos = currentLight.GetEntity()->GetTransform()->GetWorldPosition();

		//Light calculations in world space
		DirectX::XMVECTOR pos = DirectX::XMLoadFloat3(&worldPos);
		DirectX::XMStoreFloat3(&pointLights_TO_GPU.lights[i].position, pos);

		pointLights_TO_GPU.lights[i].light = {
			currentLight.color.x * currentLight.intensity,
			currentLight.color.y * currentLight.intensity,
			currentLight.color.z * currentLight.intensity
		};

		pointLights_TO_GPU.lights[i].range = currentLight.range;
	}

	pointLightsBuffer.ChangeData(&pointLights_TO_GPU);
}

void RendererManager::Tonemap()
{
	using namespace Microsoft::WRL;
	ComPtr<ID3D11Resource> texture;
	postProcessInputTexture->GetResource(&texture);

	ComPtr<ID3D11Resource> frameBuffer;
	Graphics::pSwapChain->GetBuffer(0, __uuidof(ID3D11Resource), &frameBuffer);

	Graphics::pDeviceContext->CopyResource(texture.Get(), frameBuffer.Get());

	static Shader* gamma = Resources::FindShader("Tonemap");
	gamma->GetDefaultPixelShaderVariant()->Bind();
	gamma->GetDefaultVertexShaderVariant()->Bind();

	Graphics::pDeviceContext->PSSetShaderResources(0, 1, postProcessInputTexture.GetAddressOf());
	Graphics::pDeviceContext->PSSetSamplers(0, 1, postProcessSampler.GetAddressOf());
	Graphics::pDeviceContext->RSSetState(postProcessRasterizerState.Get());
	Graphics::pDeviceContext->OMSetBlendState(postProcessBlendState.Get(), NULL, 0xffffffff);

	static const IndexBuffer* quadIndexBuffer = Mesh::Quad->GetSubMeshes()[0].GetIndexBuffer();
	static const VertexBuffer* quadPositions = Mesh::Quad->GetSubMeshes()[0].GetVertexElement(VertexBuffer::ElementType::Position3D);
	static const VertexBuffer* quadTexCoords = Mesh::Quad->GetSubMeshes()[0].GetVertexElement(VertexBuffer::ElementType::TexCoord);

	quadIndexBuffer->BindPipeline();
	quadPositions->BindPipeline();
	quadTexCoords->BindPipeline();

	Graphics::pDeviceContext->DrawIndexed(6, 0, 0);
}
