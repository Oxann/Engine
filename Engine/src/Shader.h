#pragma once
#include <unordered_map>
#include <string>
#include <d3d11.h>
#include <wrl.h>
#include "D3DBase.h"
#include "ResourceBase.h"

/*///////////////////////////////////
//	All shader binaries must be inside Resources/Shaders directory. 
//	Vertex shaders must start with VS_
//	Pixel shaders must start with PS_
//
//	--- Default Input Slots For Vertex Elements ---
//  Position = 0
//  Color = 1
//	TexCoord = 2
//  Normal = 3
//
//  
//
//
//
*////////////////////////////////////


class VertexShader final : public D3DBase, public ResourceBase
{
public:
	VertexShader(std::filesystem::path file);
	void BindPipeline() const override
	{
		GetDeviceContext()->VSSetShader(vs.Get(), nullptr, 0u);
		GetDeviceContext()->IASetInputLayout(layout.Get());
	}
private:
	void InitLayout(Microsoft::WRL::ComPtr<ID3DBlob> blob);
private:
	Microsoft::WRL::ComPtr<ID3D11VertexShader> vs;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> layout;
};


class PixelShader final : public D3DBase, public ResourceBase
{
public:
	PixelShader(std::filesystem::path file);
	void BindPipeline() const override
	{
		GetDeviceContext()->PSSetShader(ps.Get(), nullptr, 0u);
	}
private:
	Microsoft::WRL::ComPtr<ID3D11PixelShader> ps;
};