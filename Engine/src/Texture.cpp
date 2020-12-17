#define STB_IMAGE_IMPLEMENTATION

#include "../thirdparty/stb_image.h"
#include "Texture.h"
#include <filesystem>
#include "EngineAssert.h"
#include "EngineException.h"

using namespace Microsoft::WRL;

Texture::Texture(std::filesystem::path file)
	:ResourceBase(file)
{
	int width;
	int height;	
	unsigned char* img_data = stbi_load(file.string().c_str() , &width, &height, &nChannels, 4);
	
	if (img_data)
	{
		D3D11_TEXTURE2D_DESC textureDesc = {  };
		textureDesc.Width = width;
		textureDesc.Height = height;
		textureDesc.MipLevels = 1u;
		textureDesc.ArraySize = 1u;
		textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		textureDesc.SampleDesc.Count = 1u;
		textureDesc.SampleDesc.Quality = 0u;
		textureDesc.Usage = D3D11_USAGE_DEFAULT;
		textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		textureDesc.CPUAccessFlags = 0u;
		textureDesc.MiscFlags = 0u;

		D3D11_SUBRESOURCE_DATA data = {};
		data.pSysMem = img_data;
		data.SysMemPitch = width * sizeof(unsigned char) * 4;

		ComPtr<ID3D11Texture2D> texture2D;
		CHECK_DX_ERROR(GetDevice()->CreateTexture2D(&textureDesc, &data, &texture2D));
		stbi_image_free(img_data);

		D3D11_SHADER_RESOURCE_VIEW_DESC textureView = {};
		textureView.Format = textureDesc.Format;
		textureView.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		textureView.Texture2D.MostDetailedMip = 0u;
		textureView.Texture2D.MipLevels = 1u;

		ComPtr<ID3D11ShaderResourceView> srv;
		GetDevice()->CreateShaderResourceView(texture2D.Get(), &textureView, srv.GetAddressOf());
		pSRV = srv;

		//Sampler
		Microsoft::WRL::ComPtr<ID3D11SamplerState> pSamplerState;
		D3D11_SAMPLER_DESC samplerDesc = {};
		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;

		GetDevice()->CreateSamplerState(&samplerDesc, pSamplerState.GetAddressOf());
		sampler = pSamplerState;
	}
	else
	{
		std::stringstream ss;
		ss << file.string() << "is not exist or not a loadable image file.";
		THROW_ENGINE_EXCEPTION(ss.str(), true);
	}
}

bool Texture::HasAlpha() const
{
	return nChannels == 4;
}

void Texture::BindPipeline() const
{
	GetDeviceContext()->PSSetShaderResources(slot, 1u, pSRV.GetAddressOf());
	GetDeviceContext()->PSSetSamplers(slot, 1u, sampler.GetAddressOf());
}

void Texture::SetType(Type type)
{
	slot = static_cast<int>(type);
}

void Texture::BindPipeline(Type slot) const
{
	GetDeviceContext()->PSSetShaderResources(slot, 1u, pSRV.GetAddressOf());
	GetDeviceContext()->PSSetSamplers(slot, 1u, sampler.GetAddressOf());
}
