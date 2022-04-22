#pragma once
#include <d3d11.h>
#include <wrl.h>
#include "VertexBuffer.h"

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
	const std::vector<VertexBuffer::ElementType>& GetVertexElements() const;
private:
	void InitInputLayoutAndBuffers(Microsoft::WRL::ComPtr<ID3DBlob> blob);
private:
	Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader = nullptr;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout = nullptr;
	std::vector<VertexBuffer::ElementType> vertexElements;
	inline static const std::string perObjectBufferName = "PerObject";
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