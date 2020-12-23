#pragma once
#include <DirectXMath.h>
#include "Texture.h"
#include "ConstantBuffer.h"
#include "Shader.h"
#include "Mesh.h"


class Renderer;

class Material : public ResourceBase
{
protected:
	friend Renderer;

public:
	Material(std::string name)
		:ResourceBase(name)
	{}

	virtual ~Material() = default;

private:
	virtual void Bind(const Mesh::SubMesh* subMesh, const Renderer* renderer) const = 0;

protected:
	//Shaders
	VertexShader* vertexShader = nullptr;
	PixelShader* pixelShader = nullptr;
	static constexpr unsigned int PS_MaterialSlot = 3u;
};