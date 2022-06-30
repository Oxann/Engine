#pragma once
#include <wrl.h>
#include <d3d11.h>

class ShadowMap
{
public:
	enum class ShadowType
	{
		Soft,
		Hard
	};

public:
	ShadowMap(int width, int height, ShadowType shadowType);
	void BindAsDepthBuffer() const;
	void BindAsShaderResource() const;
	void SetResolution(int width, int height);
	int GetWidth() const;
	int GetHeight() const;
	void SetShadowType(ShadowType shadowType);

private:
	void CreateTextureAndViews(int width, int height);

private:
	D3D11_VIEWPORT viewport;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthMap;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shaderResource;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthStencilState;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizerState;
	ShadowType shadowType;
	static constexpr int shaderResourceSlot = 3;
};
