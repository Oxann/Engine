#include "Skybox.h"
#include "Resources.h"
#include "../thirdparty/stb_image.h"
#include "Mesh.h"
#include "ConstantBuffer.h"
#include "Transform.h"

Skybox::Skybox(	std::string_view front,
				std::string_view back,
				std::string_view up,
				std::string_view down,
				std::string_view right,
				std::string_view left )

{
	//Creating texture and srv
	int width;
	int height;
	int nChannels;

	D3D11_SUBRESOURCE_DATA data[6];
	data[0].pSysMem = stbi_load(right.data(), &width, &height, &nChannels, 4);
	data[1].pSysMem = stbi_load(left.data(), &width, &height, &nChannels, 4);
	data[2].pSysMem = stbi_load(up.data(), &width, &height, &nChannels, 4);
	data[3].pSysMem = stbi_load(down.data(), &width, &height, &nChannels, 4);
	data[4].pSysMem = stbi_load(front.data(), &width, &height, &nChannels, 4);
	data[5].pSysMem = stbi_load(back.data(), &width, &height, &nChannels, 4);
	
	for (int i = 0; i < 6; i++)
	{
		data[i].SysMemPitch = sizeof(unsigned char) * 4 * width;
		data[i].SysMemSlicePitch = 0;
	}

	D3D11_TEXTURE2D_DESC textureDesc = {  };
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.MipLevels = 1u;
	textureDesc.ArraySize = 6u;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	textureDesc.SampleDesc.Count = 1u;
	textureDesc.SampleDesc.Quality = 0u;
	textureDesc.Usage = D3D11_USAGE_IMMUTABLE;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0u;
	textureDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

	Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
	CHECK_DX_ERROR(Graphics::pDevice->CreateTexture2D(&textureDesc, data, &texture));

	D3D11_SHADER_RESOURCE_VIEW_DESC SRV_Desc;
	SRV_Desc.Format = textureDesc.Format;
	SRV_Desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	SRV_Desc.Texture2D.MipLevels = 1;
	SRV_Desc.Texture2D.MostDetailedMip = 0;

	CHECK_DX_ERROR(Graphics::pDevice->CreateShaderResourceView(texture.Get(), &SRV_Desc , &textureSRV));

	InitStates();
}

Skybox::Skybox(std::string_view equirectangularMap, unsigned int outputResolution)
{
	int width;
	int height;
	int nChannels;

	float* image = stbi_loadf(equirectangularMap.data(), &width, &height, &nChannels, 4);

	D3D11_TEXTURE2D_DESC textureDesc = {  };
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.MipLevels = 1u;
	textureDesc.ArraySize = 1u;
	textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	textureDesc.SampleDesc.Count = 1u;
	textureDesc.SampleDesc.Quality = 0u;
	textureDesc.Usage = D3D11_USAGE_IMMUTABLE;
	textureDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = image;
	data.SysMemPitch = sizeof(float) * 4 * width;

	Microsoft::WRL::ComPtr<ID3D11Texture2D> hdrImage;
	CHECK_DX_ERROR(Graphics::pDevice->CreateTexture2D(&textureDesc, &data, &hdrImage));
	stbi_image_free(image);


	D3D11_SHADER_RESOURCE_VIEW_DESC SRV_Desc;
	SRV_Desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	SRV_Desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	SRV_Desc.Texture2D.MipLevels = 1;
	SRV_Desc.Texture2D.MostDetailedMip = 0;

	Microsoft::WRL::ComPtr<ID3D11Texture2D> cubemapTexture = CreateCubemapFromEquirectangularMap(hdrImage, outputResolution);
	CHECK_DX_ERROR(Graphics::pDevice->CreateShaderResourceView(cubemapTexture.Get(), &SRV_Desc, &textureSRV));

	InitStates();
}

Skybox::Skybox(Microsoft::WRL::ComPtr<ID3D11Texture2D> textureCube)
{
	D3D11_TEXTURE2D_DESC textureCubeDesc;
	textureCube->GetDesc(&textureCubeDesc);

	D3D11_SHADER_RESOURCE_VIEW_DESC SRV_Desc;
	SRV_Desc.Format = textureCubeDesc.Format;
	SRV_Desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	SRV_Desc.TextureCube.MipLevels = 1;
	SRV_Desc.TextureCube.MostDetailedMip = 0;

	CHECK_DX_ERROR(Graphics::pDevice->CreateShaderResourceView(textureCube.Get(), &SRV_Desc, &textureSRV));

	InitStates();
}

void Skybox::Draw()
{
	static Shader* shader = Resources::FindShader("Skybox");

	shader->GetDefaultPixelShaderVariant()->Bind();
	shader->GetDefaultVertexShaderVariant()->Bind();
	
	Graphics::pDeviceContext->OMSetDepthStencilState(depthState.Get(), 0);
	Graphics::pDeviceContext->RSSetState(rasterizerState.Get());
	Graphics::pDeviceContext->OMSetBlendState(NULL, NULL, 1);
	Graphics::pDeviceContext->PSSetShaderResources(0u, 1u, textureSRV.GetAddressOf());
	Graphics::pDeviceContext->PSSetSamplers(0u, 1u, sampler.GetAddressOf());
	
	Mesh::Cube->GetSubMeshes()[0].GetVertexElement(VertexBuffer::ElementType::Position3D)->BindPipeline();
	Mesh::Cube->GetSubMeshes()[0].GetIndexBuffer()->BindPipeline();
	
	Graphics::pDeviceContext->DrawIndexed(Mesh::Cube->GetSubMeshes()[0].GetIndexCount(), 0u, 0u);	
}

Microsoft::WRL::ComPtr<ID3D11Texture2D> Skybox::GetTexture() const
{
	Microsoft::WRL::ComPtr<ID3D11Resource> textureResource;
	textureSRV->GetResource(&textureResource);
	
	Microsoft::WRL::ComPtr<ID3D11Texture2D> texture; 
	textureResource.As(&texture);

	return texture;
}

Microsoft::WRL::ComPtr<ID3D11Texture2D>  Skybox::CreateCubemapFromEquirectangularMap(Microsoft::WRL::ComPtr<ID3D11Texture2D> equirectangularMap, unsigned int outputResolution)
{
	D3D11_TEXTURE2D_DESC equirectangularMapDesc;
	equirectangularMap->GetDesc(&equirectangularMapDesc);

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> equirectangularMapSRV;
	D3D11_SHADER_RESOURCE_VIEW_DESC hdrImageSRVDesc;
	hdrImageSRVDesc.Format = equirectangularMapDesc.Format;
	hdrImageSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	hdrImageSRVDesc.Texture2D.MipLevels = -1;
	hdrImageSRVDesc.Texture2D.MostDetailedMip = 0;
	Graphics::pDevice->CreateShaderResourceView(equirectangularMap.Get(), &hdrImageSRVDesc, &equirectangularMapSRV);
	Graphics::pDeviceContext->PSSetShaderResources(0, 1, equirectangularMapSRV.GetAddressOf());

	
	D3D11_SAMPLER_DESC samplerDesc;
	samplerDesc.Filter = D3D11_FILTER::D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MinLOD = 0.0f;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	samplerDesc.MaxAnisotropy = 0;

	Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler;
	CHECK_DX_ERROR(Graphics::pDevice->CreateSamplerState(&samplerDesc, &sampler));
	Graphics::pDeviceContext->PSSetSamplers(0, 1, sampler.GetAddressOf());

	
	D3D11_TEXTURE2D_DESC renderTargetTextureDesc = {  };
	renderTargetTextureDesc.Width = outputResolution;
	renderTargetTextureDesc.Height = outputResolution;
	renderTargetTextureDesc.MipLevels = 1u;
	renderTargetTextureDesc.ArraySize = 1u;
	renderTargetTextureDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	renderTargetTextureDesc.SampleDesc.Count = 1u;
	renderTargetTextureDesc.SampleDesc.Quality = 0u;
	renderTargetTextureDesc.Usage = D3D11_USAGE_DEFAULT;
	renderTargetTextureDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_RENDER_TARGET;
	renderTargetTextureDesc.CPUAccessFlags = 0;
	renderTargetTextureDesc.MiscFlags = 0;

	Microsoft::WRL::ComPtr<ID3D11Texture2D> renderTargetTexture;
	CHECK_DX_ERROR(Graphics::pDevice->CreateTexture2D(&renderTargetTextureDesc, NULL, &renderTargetTexture));


	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
	renderTargetViewDesc.Format = renderTargetTextureDesc.Format;
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetViewDesc.Texture2D.MipSlice = 0;

	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> renderTargetView;
	CHECK_DX_ERROR(Graphics::pDevice->CreateRenderTargetView(renderTargetTexture.Get(), &renderTargetViewDesc, &renderTargetView));

	Graphics::pDeviceContext->OMSetRenderTargets(1, renderTargetView.GetAddressOf(), NULL);

	D3D11_VIEWPORT viewport;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = outputResolution;
	viewport.Height = outputResolution;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	Graphics::pDeviceContext->RSSetViewports(1, &viewport);

	static Shader* shader = Resources::FindShader("GenerateCubemap");
	shader->GetDefaultVertexShaderVariant()->Bind();
	shader->GetDefaultPixelShaderVariant()->Bind();

	static const Mesh::SubMesh& cubeMesh = Mesh::Cube->GetSubMeshes()[0];
	static const VertexBuffer* cubeVertices = cubeMesh.GetVertexElement(VertexBuffer::ElementType::Position3D);
	static const IndexBuffer* cubeIndices = cubeMesh.GetIndexBuffer();
	cubeVertices->BindPipeline();
	cubeIndices->BindPipeline();

	
	D3D11_TEXTURE2D_DESC cubemapDesc = {  };
	cubemapDesc.Width = outputResolution;
	cubemapDesc.Height = outputResolution;
	cubemapDesc.MipLevels = 1u;
	cubemapDesc.ArraySize = 6u;
	cubemapDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	cubemapDesc.SampleDesc.Count = 1u;
	cubemapDesc.SampleDesc.Quality = 0u;
	cubemapDesc.Usage = D3D11_USAGE_DEFAULT;
	cubemapDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_SHADER_RESOURCE;
	cubemapDesc.CPUAccessFlags = 0;
	cubemapDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

	Microsoft::WRL::ComPtr<ID3D11Texture2D> cubemapTexture;
	CHECK_DX_ERROR(Graphics::pDevice->CreateTexture2D(&cubemapDesc, NULL, &cubemapTexture));


	static VS_ConstantBuffer<DirectX::XMMATRIX> vsBuffer = {
		nullptr,
		1,
		0,
		D3D11_USAGE::D3D11_USAGE_DYNAMIC,
		D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_WRITE,
	};
	vsBuffer.BindPipeline();

	D3D11_RASTERIZER_DESC rasterizerStateDesc;
	ZeroMemory(&rasterizerStateDesc, sizeof(D3D11_RASTERIZER_DESC));
	rasterizerStateDesc.FillMode = D3D11_FILL_MODE::D3D11_FILL_SOLID;
	rasterizerStateDesc.CullMode = D3D11_CULL_FRONT;
	rasterizerStateDesc.FrontCounterClockwise = FALSE;
	rasterizerStateDesc.DepthClipEnable = TRUE;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizerState;
	CHECK_DX_ERROR(Graphics::pDevice->CreateRasterizerState(&rasterizerStateDesc, &rasterizerState));
	Graphics::pDeviceContext->RSSetState(rasterizerState.Get());

	D3D11_DEPTH_STENCIL_DESC depthDesc;
	ZeroMemory(&depthDesc, sizeof(depthDesc));
	depthDesc.DepthEnable = FALSE;
	depthDesc.StencilEnable = FALSE;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthStencilState;
	CHECK_DX_ERROR(Graphics::pDevice->CreateDepthStencilState(&depthDesc, &depthStencilState));
	Graphics::pDeviceContext->OMSetDepthStencilState(depthStencilState.Get(), 0u);

	
	static const DirectX::XMMATRIX projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(90.0f), 1.0f, 0.1f, 1.0f);

	//Render positive Z face
	static const DirectX::XMMATRIX viewProjPositiveZ = DirectX::XMMatrixTranspose(DirectX::XMMatrixLookToLH(Transform::GetWorldOrigin(), Transform::GetWorldForward(), Transform::GetWorldUp()) * projectionMatrix);
	vsBuffer.ChangeData(&viewProjPositiveZ);
	Graphics::pDeviceContext->DrawIndexed(cubeMesh.GetIndexCount(), 0, 0);
	Graphics::pDeviceContext->CopySubresourceRegion(cubemapTexture.Get(), 4, 0, 0, 0, renderTargetTexture.Get(), 0, NULL);

	//Render negative Z face
	static const DirectX::XMMATRIX viewProjNegativeZ = DirectX::XMMatrixTranspose(DirectX::XMMatrixLookToLH(Transform::GetWorldOrigin(), Transform::GetWorldBack(), Transform::GetWorldUp()) * projectionMatrix);
	vsBuffer.ChangeData(&viewProjNegativeZ);
	Graphics::pDeviceContext->DrawIndexed(cubeMesh.GetIndexCount(), 0, 0);
	Graphics::pDeviceContext->CopySubresourceRegion(cubemapTexture.Get(), 5, 0, 0, 0, renderTargetTexture.Get(), 0, NULL);

	//Render positive Y face
	static const DirectX::XMMATRIX viewProjPositiveY = DirectX::XMMatrixTranspose(DirectX::XMMatrixLookToLH(Transform::GetWorldOrigin(), Transform::GetWorldUp(), Transform::GetWorldBack()) * projectionMatrix);
	vsBuffer.ChangeData(&viewProjPositiveY);
	Graphics::pDeviceContext->DrawIndexed(cubeMesh.GetIndexCount(), 0, 0);
	Graphics::pDeviceContext->CopySubresourceRegion(cubemapTexture.Get(), 2, 0, 0, 0, renderTargetTexture.Get(), 0, NULL);

	//Render negative Y face
	static const DirectX::XMMATRIX viewProjNegativeY = DirectX::XMMatrixTranspose(DirectX::XMMatrixLookToLH(Transform::GetWorldOrigin(), Transform::GetWorldDown(), Transform::GetWorldForward()) * projectionMatrix);
	vsBuffer.ChangeData(&viewProjNegativeY);
	Graphics::pDeviceContext->DrawIndexed(cubeMesh.GetIndexCount(), 0, 0);
	Graphics::pDeviceContext->CopySubresourceRegion(cubemapTexture.Get(), 3, 0, 0, 0, renderTargetTexture.Get(), 0, NULL);

	//Render positive X face
	static const DirectX::XMMATRIX viewProjPositiveX = DirectX::XMMatrixTranspose(DirectX::XMMatrixLookToLH(Transform::GetWorldOrigin(), Transform::GetWorldRight(), Transform::GetWorldUp()) * projectionMatrix);
	vsBuffer.ChangeData(&viewProjPositiveX);
	Graphics::pDeviceContext->DrawIndexed(cubeMesh.GetIndexCount(), 0, 0);
	Graphics::pDeviceContext->CopySubresourceRegion(cubemapTexture.Get(), 0, 0, 0, 0, renderTargetTexture.Get(), 0, NULL);
	
	//Render negative X face
	static const DirectX::XMMATRIX viewProjNegativeX = DirectX::XMMatrixTranspose(DirectX::XMMatrixLookToLH(Transform::GetWorldOrigin(), Transform::GetWorldLeft(), Transform::GetWorldUp()) * projectionMatrix);
	vsBuffer.ChangeData(&viewProjNegativeX);
	Graphics::pDeviceContext->DrawIndexed(cubeMesh.GetIndexCount(), 0, 0);
	Graphics::pDeviceContext->CopySubresourceRegion(cubemapTexture.Get(), 1, 0, 0, 0, renderTargetTexture.Get(), 0, NULL);

	Graphics::pDeviceContext->RSSetViewports(1,&Graphics::viewport);
	return cubemapTexture;
}

void Skybox::InitStates()
{
	//Creating sampler
	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MinLOD = -FLT_MAX;
	samplerDesc.MaxLOD = FLT_MAX;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	//samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;

	CHECK_DX_ERROR(Graphics::pDevice->CreateSamplerState(&samplerDesc, &sampler));

	//Creaing depth state
	D3D11_DEPTH_STENCIL_DESC depthDesc;
	depthDesc.DepthEnable = TRUE;
	depthDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	depthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	depthDesc.StencilEnable = FALSE;
	CHECK_DX_ERROR(Graphics::pDevice->CreateDepthStencilState(&depthDesc, &depthState));

	//Creating rasterizer state
	D3D11_RASTERIZER_DESC rasterizerStateDesc;
	ZeroMemory(&rasterizerStateDesc, sizeof(D3D11_RASTERIZER_DESC));
	rasterizerStateDesc.FillMode = D3D11_FILL_MODE::D3D11_FILL_SOLID;
	rasterizerStateDesc.CullMode = D3D11_CULL_FRONT;
	rasterizerStateDesc.FrontCounterClockwise = FALSE;
	rasterizerStateDesc.DepthClipEnable = TRUE;
	CHECK_DX_ERROR(Graphics::pDevice->CreateRasterizerState(&rasterizerStateDesc, &rasterizerState));
}

