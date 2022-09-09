#pragma once
#include <wrl.h>
#include <d3d11.h>

class ShadowMap
{
	friend class RendererManager;
public:
	enum class ShadowType
	{
		Hard = 0,
		Soft = 1
	};

public:
	ShadowMap(int width, int height, ShadowType shadowType);
	void BindAsDepthBuffer() const;
	void BindAsShaderResource() const;
	void SetResolution(int width, int height);
	int GetWidth() const;
	int GetHeight() const;
	void SetShadowType(ShadowType shadowType);
	ShadowType GetShadowType() const;
	void SetBias(float bias);
	float GetBias() const;
private:
	void CreateTextureAndViews(int width, int height);

private:
	D3D11_VIEWPORT viewport;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthMap;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shaderResource;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthStencilState;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> softShadowSampler;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> hardShadowSampler;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizerState;
	ShadowType shadowType;
	static constexpr int textureSlot = 9;
	static constexpr int hardShadowsSamplerSlot = 9;
	static constexpr int softShadowsSamplerSlot = 10;
	float bias;
};
