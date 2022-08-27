#pragma once
#include <d3d11.h>
#include <wrl.h>


class EnvironmentMap
{
public:
	EnvironmentMap(Microsoft::WRL::ComPtr<ID3D11Texture2D> cubemap, unsigned int diffuseIrradianceMapResolution = 512u, unsigned int specularIrradianceMapMipLevels = 10u);

	Microsoft::WRL::ComPtr<ID3D11Texture2D> GetDiffuseIrradianceMap() const;

	void Bind();

	static void GenerateBRDFLUT(int resolution = 1024);

private:
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> diffuseIrradianceMapSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> specularIrradianceMapSRV;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> diffuseIrradianceMap;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> specularIrradianceMap;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler;

private:
	inline static unsigned int diffuseSlot = 11;
	inline static unsigned int specularSlot = 12;
	inline static unsigned int irradianceSamplerSlot = 11;
	inline static unsigned int brdfSamplerSlot = 13;
	inline static unsigned int brdfTextureSlot = 13;
};