#pragma once
#include <DirectXMath.h>
#include <string_view>
#include <array>
#include "Texture.h"
#include "ConstantBuffer.h"
#include "ShaderView.h"
#include "Mesh.h"

class Renderer;

class Material final : public ResourceBase
{
#ifdef EDITOR
	friend class EditorMaterialEditWindow;
#endif

public:
	enum Mode
	{
		Opaque,
		Transparent
	};

public:
	Material(const std::string& name, const Shader* shader);

	void ActivateMacro(std::string_view name);
	void DeactivateMacro(std::string_view name);

	void SetFloat(std::string_view name, float value);
	void SetFloat2(std::string_view name, DirectX::XMFLOAT2 value);
	void SetFloat3(std::string_view name, DirectX::XMFLOAT3 value);
	void SetFloat4(std::string_view name, DirectX::XMFLOAT4 value);

	float GetFloat(std::string_view name) const;
	DirectX::XMFLOAT2 GetFloat2(std::string_view name) const;
	DirectX::XMFLOAT3 GetFloat3(std::string_view name) const;
	DirectX::XMFLOAT4 GetFloat4(std::string_view name) const;

	void SetTexture(std::string_view name, Texture* texture);
	Texture* GetTexture(std::string_view name) const;

	unsigned int GetTextureCount() const;

 	void Bind(const Mesh::SubMesh* subMesh, Renderer* renderer) const;

public:
	Mode mode = Mode::Opaque;
protected:
	//Shaders
	ShaderView shaderView;
	static constexpr unsigned int PS_MaterialSlot = 3u;
	std::vector<unsigned char> materialBuffer;

	std::array<Texture*,8> texture2Ds;
};