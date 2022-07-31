#define STB_IMAGE_IMPLEMENTATION

#include "../thirdparty/stb_image.h"
#include "Texture.h"
#include <filesystem>
#include "EngineAssert.h"
#include "EngineException.h"
#include "Graphics.h"

using namespace Microsoft::WRL;

Texture::Texture(const std::filesystem::path& file, FilterMode filterMode, int anisotropy, ColorSpace colorSpace)
	:ResourceBase(file)
{
	ENGINEASSERT(anisotropy >= 1 && anisotropy <= 16, "Anisotropy must be between 1 and 16")
	this->anisotropy = anisotropy;
	this->filterMode = filterMode;

	unsigned char* img_data = stbi_load(file.string().c_str() , &width, &height, &nChannels, 4);
	
	if (img_data)
	{
		D3D11_TEXTURE2D_DESC textureDesc = {  };
		textureDesc.Width = width;
		textureDesc.Height = height;
		textureDesc.MipLevels = 0u;
		textureDesc.ArraySize = 1u;
		textureDesc.Format = (DXGI_FORMAT)colorSpace;
		textureDesc.SampleDesc.Count = 1u;
		textureDesc.SampleDesc.Quality = 0u;
		textureDesc.Usage = D3D11_USAGE_DEFAULT;
		textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
		textureDesc.CPUAccessFlags = 0u;
		textureDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

		ComPtr<ID3D11Texture2D> texture2D;
		D3D11_SUBRESOURCE_DATA data;
		data.pSysMem = img_data;
		data.SysMemPitch = sizeof(unsigned char) * 4 * width;
		CHECK_DX_ERROR(GetDevice()->CreateTexture2D(&textureDesc, nullptr, &texture2D));

		Graphics::immediateContextMutex.lock();
		GetDeviceContext()->UpdateSubresource(texture2D.Get(), 0, nullptr, img_data, sizeof(unsigned char) * 4 * width, 0);
		Graphics::immediateContextMutex.unlock();

		D3D11_SHADER_RESOURCE_VIEW_DESC SRV_Desc = {};
		SRV_Desc.Format = textureDesc.Format;
		SRV_Desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		SRV_Desc.Texture2D.MostDetailedMip = 0u;
		SRV_Desc.Texture2D.MipLevels = -1;
		
		GetDevice()->CreateShaderResourceView(texture2D.Get(), &SRV_Desc, &pSRV);

		Graphics::immediateContextMutex.lock();
		GetDeviceContext()->GenerateMips(pSRV.Get());
		Graphics::immediateContextMutex.unlock();

		//Sampler
		D3D11_SAMPLER_DESC samplerDesc = {};
		samplerDesc.Filter = (D3D11_FILTER)filterMode;
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


Texture::Texture(int width, int height, unsigned char* initialData, FilterMode filterMode, D3D11_USAGE usage)
	:ResourceBase("")
{
	ENGINEASSERT(width != 0, "Texture width can't be 0.")
	ENGINEASSERT(height != 0, "Texture height can't be 0.")
	ENGINEASSERT(initialData != nullptr, "Texture data can't be null.")
	
	this->width = width;
	this->height = height;
	this->filterMode = filterMode;

	D3D11_TEXTURE2D_DESC textureDesc = {  };
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.MipLevels = 1u;
	textureDesc.ArraySize = 1u;
	textureDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.SampleDesc.Count = 1u;
	textureDesc.SampleDesc.Quality = 0u;
	textureDesc.Usage = usage;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0u;
	textureDesc.MiscFlags = 0;

	ComPtr<ID3D11Texture2D> texture2D;
	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = initialData;
	data.SysMemPitch = sizeof(unsigned char) * 4 * width;
	data.SysMemSlicePitch = 0;

	CHECK_DX_ERROR(GetDevice()->CreateTexture2D(&textureDesc, &data, &texture2D));

	D3D11_SHADER_RESOURCE_VIEW_DESC SRV_Desc = {};
	SRV_Desc.Format = textureDesc.Format;
	SRV_Desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	SRV_Desc.Texture2D.MostDetailedMip = 0u;
	SRV_Desc.Texture2D.MipLevels = -1;

	CHECK_DX_ERROR(GetDevice()->CreateShaderResourceView(texture2D.Get(), &SRV_Desc, &pSRV));

	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Filter = (D3D11_FILTER)filterMode;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MinLOD = 0.0f;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	samplerDesc.MaxAnisotropy = anisotropy;
	CHECK_DX_ERROR(GetDevice()->CreateSamplerState(&samplerDesc, &sampler));
}

bool Texture::HasAlpha() const
{
	return nChannels == 4;
}

void Texture::BindPipeline(unsigned int slot) const
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
	anisotropy = std::clamp(value, 1u, 16u);

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

Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> Texture::GetResourceView() const
{
	return pSRV;
}

Texture::ColorSpace Texture::GetColorSpace() const
{
	D3D11_SHADER_RESOURCE_VIEW_DESC desc;
	pSRV->GetDesc(&desc);

	return (ColorSpace)desc.Format;
}

int Texture::GetWidth() const
{
	return width;
}

int Texture::GetHeight() const
{
	return height;
}

int Texture::GetChannels() const
{
	return nChannels;
}
