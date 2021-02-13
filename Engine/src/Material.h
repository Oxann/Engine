#pragma once
#include <DirectXMath.h>
#include "Texture.h"
#include "ConstantBuffer.h"
#include "Shader.h"
#include "Mesh.h"


class Renderer;

class Material : public ResourceBase
{
public:
	enum Mode
	{
		Opaque,
		Transparent
	};

public:
	Material(std::string name)
		:ResourceBase(name)
	{}

	virtual ~Material() = default;

	virtual void Bind(const Mesh::SubMesh* subMesh, Renderer* renderer) const = 0;

public:
	Mode mode = Mode::Opaque;

protected:
	//Shaders
	VertexShader* vertexShader = nullptr;
	PixelShader* pixelShader = nullptr;
	static constexpr unsigned int PS_MaterialSlot = 3u;	
};