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
	unsigned char* img_data = stbi_load(file.string().c_str() , const_cast<int*>(&width), const_cast<int*>(&heigth), const_cast<int*>(&nChannels), 4);
	
	if (img_data)
	{
		D3D11_TEXTURE2D_DESC textureDesc = {  };
		textureDesc.Width = width;
		textureDesc.Height = heigth;
		textureDesc.MipLevels = 0u;
		textureDesc.ArraySize = 1u;
		textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		textureDesc.SampleDesc.Count = 1u;
		textureDesc.SampleDesc.Quality = 0u;
		textureDesc.Usage = D3D11_USAGE_DEFAULT;
		textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
		textureDesc.CPUAccessFlags = 0u;
		textureDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

		ComPtr<ID3D11Texture2D> texture2D;
		CHECK_DX_ERROR(GetDevice()->CreateTexture2D(&textureDesc, nullptr, &texture2D));
		GetDeviceContext()->UpdateSubresource(texture2D.Get(), 0, nullptr, img_data, sizeof(unsigned char) * 4 * width, 0);

		D3D11_SHADER_RESOURCE_VIEW_DESC SRV_Desc = {};
		SRV_Desc.Format = textureDesc.Format;
		SRV_Desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		SRV_Desc.Texture2D.MostDetailedMip = 0u;
		SRV_Desc.Texture2D.MipLevels = -1;
		
		GetDevice()->CreateShaderResourceView(texture2D.Get(), &SRV_Desc, &pSRV);
		GetDeviceContext()->GenerateMips(pSRV.Get());

		//Sampler
		filterMode = FilterMode::ANISOTROPIC;

		D3D11_SAMPLER_DESC samplerDesc = {};
		samplerDesc.Filter = (D3D11_FILTER)FilterMode::ANISOTROPIC;
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.MipLODBias = 0.0f;
		samplerDesc.MinLOD = 0.0f;
		samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
		samplerDesc.MaxAnisotropy = anisotropy;
		GetDevice()->CreateSamplerState(&samplerDesc, &sampler);


		stbi_image_free(img_data);
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

void Texture::BindPipeline(Type slot) const
{
	GetDeviceContext()->PSSetShaderResources(slot, 1u, pSRV.GetAddressOf());
	GetDeviceContext()->PSSetSamplers(slot, 1u, sampler.GetAddressOf());
}

void Texture::SetFilterMode(FilterMode fm)
{
	filterMode = fm;

	D3D11_SAMPLER_DESC samplerDesc = {};
	sampler->GetDesc(&samplerDesc);
	samplerDesc.Filter = (D3D11_FILTER)fm;

	sampler.Reset();	
	GetDevice()->CreateSamplerState(&samplerDesc, &sampler);
}

Texture::FilterMode Texture::GetFilterMode() const
{
	return filterMode;
}

void Texture::SetAnisotropy(unsigned int value)
{
	std::clamp(value, 1u, 16u);
	anisotropy = value;

	D3D11_SAMPLER_DESC samplerDesc = {};
	sampler->GetDesc(&samplerDesc);
	samplerDesc.MaxAnisotropy = anisotropy;

	sampler.Reset();
	GetDevice()->CreateSamplerState(&samplerDesc, &sampler);
}

unsigned int Texture::GetAnisotropy() const
{
	return anisotropy;
}
