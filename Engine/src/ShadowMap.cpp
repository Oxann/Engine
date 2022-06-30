#include "ShadowMap.h"
#include "Graphics.h"
#include "EngineAssert.h"

using namespace Microsoft::WRL;

ShadowMap::ShadowMap(int width, int height, ShadowType shadowType)
{
	D3D11_RASTERIZER_DESC rasterizerDesc;
	rasterizerDesc.FillMode = D3D11_FILL_MODE::D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_MODE::D3D11_CULL_BACK;
	rasterizerDesc.FrontCounterClockwise = FALSE;
	rasterizerDesc.DepthBias = 1000;
	rasterizerDesc.DepthBiasClamp = 0.0f;
	rasterizerDesc.SlopeScaledDepthBias = 1.0f;
	rasterizerDesc.DepthClipEnable = TRUE;
	rasterizerDesc.ScissorEnable = FALSE;
	rasterizerDesc.MultisampleEnable = FALSE;
	rasterizerDesc.AntialiasedLineEnable = FALSE;
	CHECK_DX_ERROR(Graphics::pDevice->CreateRasterizerState(&rasterizerDesc, rasterizerState.GetAddressOf()));

	D3D11_DEPTH_STENCIL_DESC depthStencilStateDesc = {};
	depthStencilStateDesc.DepthEnable = TRUE;
	depthStencilStateDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilStateDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	depthStencilStateDesc.StencilEnable = FALSE;
	CHECK_DX_ERROR(Graphics::pDevice->CreateDepthStencilState(&depthStencilStateDesc, &depthStencilState));

	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	viewport.Width = width;
	viewport.Height = height;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	CreateTextureAndViews(width,height);
	SetShadowType(shadowType);
}

void ShadowMap::BindAsDepthBuffer() const
{
	Graphics::pDeviceContext->OMSetDepthStencilState(depthStencilState.Get(), 0);
	Graphics::pDeviceContext->ClearDepthStencilView(depthMap.Get(), D3D11_CLEAR_FLAG::D3D11_CLEAR_DEPTH, 1.0f, 0);
	Graphics::pDeviceContext->OMSetRenderTargets(0, nullptr, depthMap.Get());
	Graphics::pDeviceContext->RSSetViewports(1, &viewport);
	Graphics::pDeviceContext->RSSetState(rasterizerState.Get());
}

void ShadowMap::BindAsShaderResource() const
{
	Graphics::pDeviceContext->PSSetShaderResources(shaderResourceSlot, 1, shaderResource.GetAddressOf());
	Graphics::pDeviceContext->PSSetSamplers(shaderResourceSlot, 1, sampler.GetAddressOf());
}

void ShadowMap::SetResolution(int width, int height)
{
	viewport.Width = width;
	viewport.Height = height;

	CreateTextureAndViews(width, height);
}

int ShadowMap::GetWidth() const
{
	return viewport.Width;
}

int ShadowMap::GetHeight() const
{
	return viewport.Height;
}

void ShadowMap::SetShadowType(ShadowType shadowType)
{
	this->shadowType = shadowType;
	D3D11_SAMPLER_DESC samplerDesc = {};

	if (shadowType == ShadowType::Soft)
	{
		samplerDesc.Filter = D3D11_FILTER::D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
		samplerDesc.MipLODBias = 0.0f;
		samplerDesc.MinLOD = 0.0f;
		samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
		samplerDesc.MaxAnisotropy = 0;
		samplerDesc.BorderColor[0] = 0.0f;
		samplerDesc.BorderColor[1] = 0.0f;
		samplerDesc.BorderColor[2] = 0.0f;
		samplerDesc.BorderColor[3] = 0.0f;
		samplerDesc.ComparisonFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_LESS_EQUAL;
	}
	else
	{
		samplerDesc.Filter = D3D11_FILTER::D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
		samplerDesc.MipLODBias = 0.0f;
		samplerDesc.MinLOD = 0.0f;
		samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
		samplerDesc.MaxAnisotropy = 0;
		samplerDesc.BorderColor[0] = 0.0f;
		samplerDesc.BorderColor[1] = 0.0f;
		samplerDesc.BorderColor[2] = 0.0f;
		samplerDesc.BorderColor[3] = 0.0f;
		samplerDesc.ComparisonFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_NEVER;
	}

	sampler.Reset();
	CHECK_DX_ERROR(Graphics::pDevice->CreateSamplerState(&samplerDesc, sampler.GetAddressOf()));
}

void ShadowMap::CreateTextureAndViews(int width, int height)
{
	ComPtr<ID3D11Texture2D> depthTexture;
	D3D11_TEXTURE2D_DESC textureDesc;
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.MipLevels = 1u;
	textureDesc.ArraySize = 1u;
	textureDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R32_TYPELESS;
	textureDesc.SampleDesc.Count = 1u;
	textureDesc.SampleDesc.Quality = 0u;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;
	CHECK_DX_ERROR(Graphics::pDevice->CreateTexture2D(&textureDesc, nullptr, depthTexture.GetAddressOf()));

	//depthMap.Reset();
	D3D11_DEPTH_STENCIL_VIEW_DESC dsViewDesc = {};
	dsViewDesc.Format = DXGI_FORMAT::DXGI_FORMAT_D32_FLOAT;
	dsViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsViewDesc.Texture2D.MipSlice = 0u;
	CHECK_DX_ERROR(Graphics::pDevice->CreateDepthStencilView(depthTexture.Get(), &dsViewDesc, depthMap.GetAddressOf()));

	//shaderResource.Reset();
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc = {};
	shaderResourceViewDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R32_FLOAT;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION::D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	CHECK_DX_ERROR(Graphics::pDevice->CreateShaderResourceView(depthTexture.Get(), &shaderResourceViewDesc, shaderResource.GetAddressOf()));
}
