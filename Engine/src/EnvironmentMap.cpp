#include "EnvironmentMap.h"
#include "EngineAssert.h"
#include "Graphics.h"
#include "Resources.h"
#include "Transform.h"

EnvironmentMap::EnvironmentMap(Microsoft::WRL::ComPtr<ID3D11Texture2D> cubemap, unsigned int diffuseIrradianceMapResolution, unsigned int specularIrradianceMapMipLevels)
{
	D3D11_TEXTURE2D_DESC cubemapDesc;
	cubemap->GetDesc(&cubemapDesc);

	D3D11_TEXTURE2D_DESC cubeMapWithMipsDesc = cubemapDesc;
	cubeMapWithMipsDesc.MipLevels = 0u;
	cubeMapWithMipsDesc.ArraySize = 6u;
	cubeMapWithMipsDesc.Usage = D3D11_USAGE_DEFAULT;
	cubeMapWithMipsDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_FLAG::D3D11_BIND_RENDER_TARGET;
	cubeMapWithMipsDesc.CPUAccessFlags = 0;
	cubeMapWithMipsDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE | D3D11_RESOURCE_MISC_GENERATE_MIPS;

	Microsoft::WRL::ComPtr<ID3D11Texture2D> cubemapWithMips;
	CHECK_DX_ERROR(Graphics::pDevice->CreateTexture2D(&cubeMapWithMipsDesc, NULL, &cubemapWithMips));
	Graphics::pDeviceContext->CopySubresourceRegion(cubemapWithMips.Get(), 0, 0, 0, 0, cubemap.Get(), 0, NULL);
	Graphics::pDeviceContext->CopySubresourceRegion(cubemapWithMips.Get(), 10, 0, 0, 0, cubemap.Get(), 1, NULL);
	Graphics::pDeviceContext->CopySubresourceRegion(cubemapWithMips.Get(), 20, 0, 0, 0, cubemap.Get(), 2, NULL);
	Graphics::pDeviceContext->CopySubresourceRegion(cubemapWithMips.Get(), 30, 0, 0, 0, cubemap.Get(), 3, NULL);
	Graphics::pDeviceContext->CopySubresourceRegion(cubemapWithMips.Get(), 40, 0, 0, 0, cubemap.Get(), 4, NULL);
	Graphics::pDeviceContext->CopySubresourceRegion(cubemapWithMips.Get(), 50, 0, 0, 0, cubemap.Get(), 5, NULL);

	D3D11_SHADER_RESOURCE_VIEW_DESC cubemapWithMipsSRVDesc;
	cubemapWithMipsSRVDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R16G16B16A16_FLOAT;
	cubemapWithMipsSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	cubemapWithMipsSRVDesc.TextureCube.MipLevels = -1;
	cubemapWithMipsSRVDesc.TextureCube.MostDetailedMip = 0;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cubemapWithMipsSRV;
	CHECK_DX_ERROR(Graphics::pDevice->CreateShaderResourceView(cubemapWithMips.Get(), &cubemapWithMipsSRVDesc, &cubemapWithMipsSRV));

	Graphics::pDeviceContext->GenerateMips(cubemapWithMipsSRV.Get());
	Graphics::pDeviceContext->PSSetShaderResources(0, 1, cubemapWithMipsSRV.GetAddressOf());


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

	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Filter = D3D11_FILTER::D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MinLOD = 0.0f;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	samplerDesc.MaxAnisotropy = 0;

	CHECK_DX_ERROR(Graphics::pDevice->CreateSamplerState(&samplerDesc, &sampler));
	Graphics::pDeviceContext->PSSetSamplers(0, 1, sampler.GetAddressOf());



	D3D11_TEXTURE2D_DESC renderTargetTextureDesc = {  };
	renderTargetTextureDesc.Width = diffuseIrradianceMapResolution;
	renderTargetTextureDesc.Height = diffuseIrradianceMapResolution;
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

	static const Mesh::SubMesh& cubeMesh = Mesh::Cube->GetSubMeshes()[0];
	static const VertexBuffer* cubeVertices = cubeMesh.GetVertexElement(VertexBuffer::ElementType::Position3D);
	static const IndexBuffer* cubeIndices = cubeMesh.GetIndexBuffer();
	cubeVertices->BindPipeline();
	cubeIndices->BindPipeline();

	static VS_ConstantBuffer<DirectX::XMMATRIX> vsBuffer = {
				nullptr,
				1,
				0,
				D3D11_USAGE::D3D11_USAGE_DYNAMIC,
				D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_WRITE,
				true			
	};
	vsBuffer.BindPipeline();

	static const DirectX::XMMATRIX projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(90.0f), 1.0f, 0.1f, 1.0f);
	static const DirectX::XMMATRIX viewProjPositiveZ = DirectX::XMMatrixTranspose(DirectX::XMMatrixLookToLH(Transform::GetWorldOrigin(), Transform::GetWorldForward(), Transform::GetWorldUp()) * projectionMatrix);
	static const DirectX::XMMATRIX viewProjNegativeZ = DirectX::XMMatrixTranspose(DirectX::XMMatrixLookToLH(Transform::GetWorldOrigin(), Transform::GetWorldBack(), Transform::GetWorldUp()) * projectionMatrix);
	static const DirectX::XMMATRIX viewProjPositiveY = DirectX::XMMatrixTranspose(DirectX::XMMatrixLookToLH(Transform::GetWorldOrigin(), Transform::GetWorldUp(), Transform::GetWorldBack()) * projectionMatrix);
	static const DirectX::XMMATRIX viewProjNegativeY = DirectX::XMMatrixTranspose(DirectX::XMMatrixLookToLH(Transform::GetWorldOrigin(), Transform::GetWorldDown(), Transform::GetWorldForward()) * projectionMatrix);
	static const DirectX::XMMATRIX viewProjPositiveX = DirectX::XMMatrixTranspose(DirectX::XMMatrixLookToLH(Transform::GetWorldOrigin(), Transform::GetWorldRight(), Transform::GetWorldUp()) * projectionMatrix);
	static const DirectX::XMMATRIX viewProjNegativeX = DirectX::XMMatrixTranspose(DirectX::XMMatrixLookToLH(Transform::GetWorldOrigin(), Transform::GetWorldLeft(), Transform::GetWorldUp()) * projectionMatrix);


	
	// --- GENERATING DIFFUSE IRRADIANCE MAP ---

	static Shader* diffuseIrradianceMapShader = Resources::FindShader("GenerateDiffuseIrradianceMap");
	diffuseIrradianceMapShader->GetDefaultVertexShaderVariant()->Bind();
	diffuseIrradianceMapShader->GetDefaultPixelShaderVariant()->Bind();

	D3D11_VIEWPORT viewport;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = diffuseIrradianceMapResolution;
	viewport.Height = diffuseIrradianceMapResolution;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	Graphics::pDeviceContext->RSSetViewports(1, &viewport);


	D3D11_TEXTURE2D_DESC diffuseIrradianceMapDesc = {  };
	diffuseIrradianceMapDesc.Width = diffuseIrradianceMapResolution;
	diffuseIrradianceMapDesc.Height = diffuseIrradianceMapResolution;
	diffuseIrradianceMapDesc.MipLevels = 1u;
	diffuseIrradianceMapDesc.ArraySize = 6u;
	diffuseIrradianceMapDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	diffuseIrradianceMapDesc.SampleDesc.Count = 1u;
	diffuseIrradianceMapDesc.SampleDesc.Quality = 0u;
	diffuseIrradianceMapDesc.Usage = D3D11_USAGE_DEFAULT;
	diffuseIrradianceMapDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_SHADER_RESOURCE;
	diffuseIrradianceMapDesc.CPUAccessFlags = 0;
	diffuseIrradianceMapDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

	CHECK_DX_ERROR(Graphics::pDevice->CreateTexture2D(&diffuseIrradianceMapDesc, NULL, &diffuseIrradianceMap));

	D3D11_SHADER_RESOURCE_VIEW_DESC diffuseIrradianceMapSRVDesc;
	diffuseIrradianceMapSRVDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R16G16B16A16_FLOAT;
	diffuseIrradianceMapSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	diffuseIrradianceMapSRVDesc.TextureCube.MipLevels = 1;
	diffuseIrradianceMapSRVDesc.TextureCube.MostDetailedMip = 0;

	CHECK_DX_ERROR(Graphics::pDevice->CreateShaderResourceView(diffuseIrradianceMap.Get(), &diffuseIrradianceMapSRVDesc, &diffuseIrradianceMapSRV));

	//Render positive Z face
	vsBuffer.ChangeData(&viewProjPositiveZ);
	Graphics::pDeviceContext->DrawIndexed(cubeMesh.GetIndexCount(), 0, 0);
	Graphics::pDeviceContext->CopySubresourceRegion(diffuseIrradianceMap.Get(), 4, 0, 0, 0, renderTargetTexture.Get(), 0, NULL);

	//Render negative Z face
	vsBuffer.ChangeData(&viewProjNegativeZ);
	Graphics::pDeviceContext->DrawIndexed(cubeMesh.GetIndexCount(), 0, 0);
	Graphics::pDeviceContext->CopySubresourceRegion(diffuseIrradianceMap.Get(), 5, 0, 0, 0, renderTargetTexture.Get(), 0, NULL);

	//Render positive Y face
	vsBuffer.ChangeData(&viewProjPositiveY);
	Graphics::pDeviceContext->DrawIndexed(cubeMesh.GetIndexCount(), 0, 0);
	Graphics::pDeviceContext->CopySubresourceRegion(diffuseIrradianceMap.Get(), 2, 0, 0, 0, renderTargetTexture.Get(), 0, NULL);

	//Render negative Y face
	vsBuffer.ChangeData(&viewProjNegativeY);
	Graphics::pDeviceContext->DrawIndexed(cubeMesh.GetIndexCount(), 0, 0);
	Graphics::pDeviceContext->CopySubresourceRegion(diffuseIrradianceMap.Get(), 3, 0, 0, 0, renderTargetTexture.Get(), 0, NULL);

	//Render positive X face
	vsBuffer.ChangeData(&viewProjPositiveX);
	Graphics::pDeviceContext->DrawIndexed(cubeMesh.GetIndexCount(), 0, 0);
	Graphics::pDeviceContext->CopySubresourceRegion(diffuseIrradianceMap.Get(), 0, 0, 0, 0, renderTargetTexture.Get(), 0, NULL);

	//Render negative X face
	vsBuffer.ChangeData(&viewProjNegativeX);
	Graphics::pDeviceContext->DrawIndexed(cubeMesh.GetIndexCount(), 0, 0);
	Graphics::pDeviceContext->CopySubresourceRegion(diffuseIrradianceMap.Get(), 1, 0, 0, 0, renderTargetTexture.Get(), 0, NULL);
	
	////////////////////////////////////////////
		
	// --- GENERATING SPECULAR IRRADIANCE MAP ---
	static Shader* specularIrrandianceMapShader = Resources::FindShader("GenerateSpecularIrradianceMap");
	specularIrrandianceMapShader->GetDefaultVertexShaderVariant()->Bind();
	specularIrrandianceMapShader->GetDefaultPixelShaderVariant()->Bind();

	struct GenerateSpecularIrradianceMapPSBuffer
	{
		alignas(16u) float roughness;
	};
	GenerateSpecularIrradianceMapPSBuffer generateSpecularIrradianceMapPSBuffer;

	PS_ConstantBuffer<GenerateSpecularIrradianceMapPSBuffer> roughnessBuffer(nullptr, 1, 0, D3D11_USAGE::D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_WRITE, true);

	const unsigned int resolution = std::pow(2, specularIrradianceMapMipLevels - 1);

	D3D11_TEXTURE2D_DESC specularIrradianceMapDesc = {  };
	specularIrradianceMapDesc.Width = resolution;
	specularIrradianceMapDesc.Height = resolution;
	specularIrradianceMapDesc.MipLevels = 0u;
	specularIrradianceMapDesc.ArraySize = 6u;
	specularIrradianceMapDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	specularIrradianceMapDesc.SampleDesc.Count = 1u;
	specularIrradianceMapDesc.SampleDesc.Quality = 0u;
	specularIrradianceMapDesc.Usage = D3D11_USAGE_DEFAULT;
	specularIrradianceMapDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_FLAG::D3D11_BIND_RENDER_TARGET;
	specularIrradianceMapDesc.CPUAccessFlags = 0;
	specularIrradianceMapDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE | D3D11_RESOURCE_MISC_GENERATE_MIPS;
	CHECK_DX_ERROR(Graphics::pDevice->CreateTexture2D(&specularIrradianceMapDesc, NULL, &specularIrradianceMap));

	D3D11_SHADER_RESOURCE_VIEW_DESC specularIrradianceMapSRVDesc;
	specularIrradianceMapSRVDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R16G16B16A16_FLOAT;
	specularIrradianceMapSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	specularIrradianceMapSRVDesc.TextureCube.MipLevels = -1;
	specularIrradianceMapSRVDesc.TextureCube.MostDetailedMip = 0;
	CHECK_DX_ERROR(Graphics::pDevice->CreateShaderResourceView(specularIrradianceMap.Get(), &specularIrradianceMapSRVDesc, &specularIrradianceMapSRV));

	Graphics::pDeviceContext->GenerateMips(specularIrradianceMapSRV.Get());

	for (int i = 0; i < specularIrradianceMapMipLevels; i++)
	{
		generateSpecularIrradianceMapPSBuffer.roughness = (float)(i) / (float)(specularIrradianceMapMipLevels - 1);
		roughnessBuffer.ChangeData(&generateSpecularIrradianceMapPSBuffer);

		const int mipRes = pow(2, specularIrradianceMapMipLevels - 1 - i);

		D3D11_VIEWPORT viewport;
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.Width = mipRes;
		viewport.Height = mipRes;
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;
		Graphics::pDeviceContext->RSSetViewports(1, &viewport);

		D3D11_TEXTURE2D_DESC renderTargetTextureDesc = {  };
		renderTargetTextureDesc.Width = mipRes;
		renderTargetTextureDesc.Height = mipRes;
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


		//Render positive Z face
		vsBuffer.ChangeData(&viewProjPositiveZ);
		Graphics::pDeviceContext->DrawIndexed(cubeMesh.GetIndexCount(), 0, 0);
		Graphics::pDeviceContext->CopySubresourceRegion(specularIrradianceMap.Get(), 4 * specularIrradianceMapMipLevels + i, 0, 0, 0, renderTargetTexture.Get(), 0, NULL);

		//Render negative Z face
		vsBuffer.ChangeData(&viewProjNegativeZ);
		Graphics::pDeviceContext->DrawIndexed(cubeMesh.GetIndexCount(), 0, 0);
		Graphics::pDeviceContext->CopySubresourceRegion(specularIrradianceMap.Get(), 5 * specularIrradianceMapMipLevels + i, 0, 0, 0, renderTargetTexture.Get(), 0, NULL);

		//Render positive Y face
		vsBuffer.ChangeData(&viewProjPositiveY);
		Graphics::pDeviceContext->DrawIndexed(cubeMesh.GetIndexCount(), 0, 0);
		Graphics::pDeviceContext->CopySubresourceRegion(specularIrradianceMap.Get(), 2 * specularIrradianceMapMipLevels + i, 0, 0, 0, renderTargetTexture.Get(), 0, NULL);

		//Render negative Y face
		vsBuffer.ChangeData(&viewProjNegativeY);
		Graphics::pDeviceContext->DrawIndexed(cubeMesh.GetIndexCount(), 0, 0);
		Graphics::pDeviceContext->CopySubresourceRegion(specularIrradianceMap.Get(), 3 * specularIrradianceMapMipLevels + i, 0, 0, 0, renderTargetTexture.Get(), 0, NULL);

		//Render positive X face
		vsBuffer.ChangeData(&viewProjPositiveX);
		Graphics::pDeviceContext->DrawIndexed(cubeMesh.GetIndexCount(), 0, 0);
		Graphics::pDeviceContext->CopySubresourceRegion(specularIrradianceMap.Get(), i, 0, 0, 0, renderTargetTexture.Get(), 0, NULL);

		//Render negative X face
		vsBuffer.ChangeData(&viewProjNegativeX);
		Graphics::pDeviceContext->DrawIndexed(cubeMesh.GetIndexCount(), 0, 0);
		Graphics::pDeviceContext->CopySubresourceRegion(specularIrradianceMap.Get(), specularIrradianceMapMipLevels + i, 0, 0, 0, renderTargetTexture.Get(), 0, NULL);
	}

	//Setting screen viewport after we are done.
	Graphics::pDeviceContext->RSSetViewports(1, &Graphics::viewport);
}

Microsoft::WRL::ComPtr<ID3D11Texture2D> EnvironmentMap::GetDiffuseIrradianceMap() const
{
	return diffuseIrradianceMap;
}

void EnvironmentMap::Bind()
{
	Graphics::pDeviceContext->PSSetShaderResources(diffuseSlot, 1, diffuseIrradianceMapSRV.GetAddressOf());
	Graphics::pDeviceContext->PSSetShaderResources(specularSlot, 1, specularIrradianceMapSRV.GetAddressOf());
	Graphics::pDeviceContext->PSSetSamplers(irradianceSamplerSlot, 1, sampler.GetAddressOf());
}

void EnvironmentMap::GenerateBRDFLUT(int resolution)
{
	D3D11_SAMPLER_DESC samplerDesc = {};
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
	Graphics::pDeviceContext->PSSetSamplers(brdfSamplerSlot, 1, sampler.GetAddressOf());

	D3D11_TEXTURE2D_DESC BRDFLUTDesc = {  };
	BRDFLUTDesc.Width = resolution;
	BRDFLUTDesc.Height = resolution;
	BRDFLUTDesc.MipLevels = 1u;
	BRDFLUTDesc.ArraySize = 1u;
	BRDFLUTDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	BRDFLUTDesc.SampleDesc.Count = 1u;
	BRDFLUTDesc.SampleDesc.Quality = 0u;
	BRDFLUTDesc.Usage = D3D11_USAGE_DEFAULT;
	BRDFLUTDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_SHADER_RESOURCE;
	BRDFLUTDesc.CPUAccessFlags = 0;
	BRDFLUTDesc.MiscFlags = 0;
	
	Microsoft::WRL::ComPtr<ID3D11Texture2D> BRDFLUT;
	CHECK_DX_ERROR(Graphics::pDevice->CreateTexture2D(&BRDFLUTDesc, NULL, &BRDFLUT));

	D3D11_SHADER_RESOURCE_VIEW_DESC BRDFLUTSRVDesc;
	BRDFLUTSRVDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R16G16B16A16_FLOAT;
	BRDFLUTSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	BRDFLUTSRVDesc.TextureCube.MipLevels = 1;
	BRDFLUTSRVDesc.TextureCube.MostDetailedMip = 0;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> BRDFLUTSRV;
	CHECK_DX_ERROR(Graphics::pDevice->CreateShaderResourceView(BRDFLUT.Get(), &BRDFLUTSRVDesc, &BRDFLUTSRV));
	Graphics::pDeviceContext->PSSetShaderResources(brdfTextureSlot, 1, BRDFLUTSRV.GetAddressOf());

	
	D3D11_TEXTURE2D_DESC renderTargetTextureDesc = {  };
	renderTargetTextureDesc.Width = resolution;
	renderTargetTextureDesc.Height = resolution;
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


	static Shader* shader = Resources::FindShader("IntegrateBRDF");
	shader->GetDefaultVertexShaderVariant()->Bind();
	shader->GetDefaultPixelShaderVariant()->Bind();

	static const Mesh::SubMesh& quadMesh = Mesh::Quad->GetSubMeshes()[0];
	static const VertexBuffer* quadVertices = quadMesh.GetVertexElement(VertexBuffer::ElementType::Position3D);
	static const VertexBuffer* quadTexCoords = quadMesh.GetVertexElement(VertexBuffer::ElementType::TexCoord);
	static const IndexBuffer* quadIndices = quadMesh.GetIndexBuffer();
	quadVertices->BindPipeline();
	quadTexCoords->BindPipeline();
	quadIndices->BindPipeline();


	D3D11_VIEWPORT viewport;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = resolution;
	viewport.Height = resolution;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	Graphics::pDeviceContext->RSSetViewports(1, &viewport);

	Graphics::pDeviceContext->DrawIndexed(quadMesh.GetIndexCount(), 0, 0);

	Graphics::pDeviceContext->CopyResource(BRDFLUT.Get(), renderTargetTexture.Get());

	//Setting screen viewport after we are done.
	Graphics::pDeviceContext->RSSetViewports(1, &Graphics::viewport);
}
