#pragma once
#include "D3DBase.h"
#include "ResourceBase.h"

#include <string>
#include <unordered_map>

class Texture : public ResourceBase, public D3DBase
{
public:
	enum Type
	{
		DIFFUSE,
		SPECULAR,
		NORMAL
	};

	friend class Material;
public:
	Texture(std::filesystem::path file);

	bool HasAlpha() const;

	void SetType(Type type);

	//Also binds sampler.
	void BindPipeline() const override;

	void BindPipeline(Texture::Type slot) const;
private:
	int nChannels;
	UINT slot = 0u;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler = nullptr;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pSRV = nullptr;
};