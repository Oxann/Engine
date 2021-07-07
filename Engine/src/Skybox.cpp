#include "Skybox.h"
#include "Resources.h"
#include "../thirdparty/stb_image.h"
#include "Mesh.h"
#include "ConstantBuffer.h"

Skybox::Skybox(	const std::string& front, 
				const std::string& back, 
				const std::string& up, 
				const std::string& down, 
				const std::string& right, 
				const std::string& left )

	:shader(Resources::FindShader("Skybox"))
{
	//Creating texture and srv
	int width;
	int height;
	int nChannels;

	D3D11_SUBRESOURCE_DATA data[6];
	data[0].pSysMem = stbi_load(right.c_str(), &width, &height, &nChannels, 4);
	data[1].pSysMem = stbi_load(left.c_str(), &width, &height, &nChannels, 4);
	data[2].pSysMem = stbi_load(up.c_str(), &width, &height, &nChannels, 4);
	data[3].pSysMem = stbi_load(down.c_str(), &width, &height, &nChannels, 4);
	data[4].pSysMem = stbi_load(front.c_str(), &width, &height, &nChannels, 4);
	data[5].pSysMem = stbi_load(back.c_str(), &width, &height, &nChannels, 4);
	
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
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.SampleDesc.Count = 1u;
	textureDesc.SampleDesc.Quality = 0u;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
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
	rasterizerStateDesc.CullMode = D3D11_CULL_NONE;
	rasterizerStateDesc.FrontCounterClockwise = FALSE;
	rasterizerStateDesc.DepthClipEnable = TRUE;
	CHECK_DX_ERROR(Graphics::pDevice->CreateRasterizerState(&rasterizerStateDesc, &rasterizerState));
}

void Skybox::Draw()
{
	shader.Bind();
	
	Graphics::pDeviceContext->OMSetDepthStencilState(depthState.Get(), 0);
	Graphics::pDeviceContext->RSSetState(rasterizerState.Get());
	Graphics::pDeviceContext->OMSetBlendState(NULL, NULL, 1);
	Graphics::pDeviceContext->PSSetShaderResources(0u, 1u, textureSRV.GetAddressOf());
	Graphics::pDeviceContext->PSSetSamplers(0u, 1u, sampler.GetAddressOf());
	
	Mesh::Cube->GetSubMeshes()[0].GetVertexElement(VertexBuffer::ElementType::Position3D)->BindPipeline();
	Mesh::Cube->GetSubMeshes()[0].GetIndexBuffer()->BindPipeline();

	static VS_ConstantBuffer<DirectX::XMMATRIX> vs_cb(&Graphics::GetViewMatrix(),
		1u,
		0u,
		D3D11_USAGE::D3D11_USAGE_DYNAMIC,
		D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_WRITE);
	
	DirectX::XMMATRIX viewProjection = DirectX::XMMatrixTranspose(Graphics::GetViewMatrix() * Graphics::GetProjectionMatrix());

	vs_cb.ChangeData(&viewProjection);
	vs_cb.BindPipeline();

	Graphics::pDeviceContext->DrawIndexed(Mesh::Cube->GetSubMeshes()[0].GetIndexCount(), 0u, 0u);	
}
