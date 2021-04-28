#pragma once
#include <DirectXMath.h>
#include "Texture.h"
#include "ConstantBuffer.h"
#include "ShaderView.h"
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
	Material(const std::string& name, const Shader* shader)
		:ResourceBase(name), 
		shaderView(shader)
	{}

	virtual ~Material() = default;

	virtual void Bind(const Mesh::SubMesh* subMesh, Renderer* renderer) const = 0;

public:
	Mode mode = Mode::Opaque;

protected:
	//Shaders
	ShaderView shaderView;
	static constexpr unsigned int PS_MaterialSlot = 3u;	
};