#pragma once
#include <d3d11.h>
#include <wrl.h>
#include <vector>
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
	bool HasShadows() const;
	const std::vector<VertexBuffer::ElementType>& GetVertexElements() const;
private:
	void InitInputLayoutAndBuffers(Microsoft::WRL::ComPtr<ID3DBlob> blob);
private:
	Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader = nullptr;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout = nullptr;
	std::vector<VertexBuffer::ElementType> vertexElements;
	inline static const std::string perObjectBufferName = "PerObject";

	enum PerObjectBufferUsageFlags
	{
		None = 0,
		ModelMatrix = 1,
		ModelViewMatrix = 2,
		ModelViewProjectionMatrix = 4,
		NormalMatrix = 8
	};
	unsigned int perObjectBufferUsageFlags = None;
	bool hasShadows;
};

class PixelShaderVariant : public ShaderVariantBase
{
public:
	struct ResourceDefinition
	{
		std::string name;
		unsigned int slot;

		bool operator ==(std::string_view name) const { return this->name == name; }
	};

public:
	PixelShaderVariant() = default;
	PixelShaderVariant(Microsoft::WRL::ComPtr<ID3DBlob> blob);
	void Bind() const override;

	const std::vector<ResourceDefinition>& GetTexture2Ds() const;

private:
	void InitTextureLayout(Microsoft::WRL::ComPtr<ID3DBlob> blob);
private:
	Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader = nullptr;
	std::vector<ResourceDefinition> texture2Ds;
};