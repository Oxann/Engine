#pragma once
#include "D3DBase.h"
#include "ResourceBase.h"

#include <string>
#include <unordered_map>

class Texture : public ResourceBase, public D3DBase
{
	friend class Material;
public:
	enum FilterMode
	{
		POINT = D3D11_FILTER_MIN_MAG_MIP_POINT,
		LINEAR = D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR,
		BILINEAR = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT,
		TRILINEAR = D3D11_FILTER_MIN_MAG_MIP_LINEAR,
		ANISOTROPIC = D3D11_FILTER_ANISOTROPIC
	};
public:
	Texture(const std::filesystem::path& file);

	bool HasAlpha() const;

	//Also binds sampler.
	void BindPipeline() const override {};

	void BindPipeline(unsigned int slot) const;

	void SetFilterMode(FilterMode fm);
	
	FilterMode GetFilterMode() const;
	
	//[1,16]
	void SetAnisotropy(unsigned int value);

	unsigned int GetAnisotropy() const;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> GetResourceView() const;

public:
	const int width = 0;
	const int height = 0;
	const int nChannels = 0;
	
private:
	Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler = nullptr;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pSRV = nullptr;
	FilterMode filterMode;
	unsigned int anisotropy = 16u;
};