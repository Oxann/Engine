#pragma once
#include <wrl.h>
#include <d3d11.h>

class ShadowMap
{
public:
	ShadowMap();
	void BindAsDepthBuffer() const;
	void BindAsShaderResource() const;
private:
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthMap;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shaderResource;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthStencilState;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizerState;
	D3D11_VIEWPORT viewport;
	static constexpr int shaderResourceSlot = 3;
};