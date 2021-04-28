#pragma once
#include <d3d11.h>
#include <wrl.h>

class Shader;
class ShaderView;

class ShaderVariantBase
{
public:
	virtual ~ShaderVariantBase() = default;
	virtual void Bind() const = 0;
};

class VertexShaderVariant : public ShaderVariantBase
{
public:
	VertexShaderVariant() = default;
	VertexShaderVariant(Microsoft::WRL::ComPtr<ID3DBlob> blob);
	void Bind() const override;
private:
	void InitLayout(Microsoft::WRL::ComPtr<ID3DBlob> blob);
private:
	Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader = nullptr;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout = nullptr;
};

class PixelShaderVariant : public ShaderVariantBase
{
public:
	PixelShaderVariant() = default;
	PixelShaderVariant(Microsoft::WRL::ComPtr<ID3DBlob> blob);
	void Bind() const override;
private:
	Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader = nullptr;
};