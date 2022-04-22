#pragma once
#include <unordered_map>
#include <set>
#include <string>
#include <d3dcompiler.h>

#include "ShaderVariant.h"
#include "ResourceBase.h"
#include "ConstantBuffer.h"

/*///////////////////////////////////
//
//	--- Default Input Slots For Vertex Elements ---
//  Position	=	0
//  TexCoord	=	1
//	Normal		=	2
//  Tangent		=	3
//	Bitanget	=	4
*////////////////////////////////////


class Shader
{
public:
	struct VertexShaderPerObjectBuffer
	{
		DirectX::XMMATRIX model;
		DirectX::XMMATRIX modelView;
		DirectX::XMMATRIX modelViewProjection;
		DirectX::XMMATRIX normal;

		static constexpr int slot = 0;
		static VertexShaderPerObjectBuffer buffer;
	};

	struct VertexShaderPerFrameBuffer
	{
		DirectX::XMMATRIX view;
		DirectX::XMMATRIX projection;

		static constexpr int slot = 1;
		static VertexShaderPerFrameBuffer buffer;
	};

private:
	typedef std::vector<std::pair<std::set<unsigned char>, VertexShaderVariant>> VSMAP;
	typedef std::vector<std::pair<std::set<unsigned char>, PixelShaderVariant>> PSMAP;

	friend class Resources;
	friend class ShaderView;
public:
	Shader(const std::string& name, const std::filesystem::path& VS_Path, const std::filesystem::path& PS_Path);

	const std::vector<std::string>& GetVertexShaderMacros() const;
	const std::vector<std::string>& GetPixelShaderMacros() const;

	const size_t GetVertexShaderMacroCount() const;

	const size_t GetPixelShaderMacroCount() const;

	VertexShaderVariant* const GetVertexShaderVariant(const std::set<unsigned char>& macroIndices) const;
	PixelShaderVariant* const GetPixelShaderVariant(const std::set<unsigned char>& macroIndices) const;

	VertexShaderVariant* const GetDefaultVertexShaderVariant() const;
	PixelShaderVariant* const GetDefaultPixelShaderVariant() const;

	static VS_ConstantBuffer<VertexShaderPerObjectBuffer>* const GetVertexShaderPerObjectBuffer();
	static VS_ConstantBuffer<VertexShaderPerFrameBuffer>* const GetVertexShaderPerFrameBuffer();

private:
	void ExtractMacrosFromSource(std::stringstream& source, std::vector<std::string>& macros);
	Microsoft::WRL::ComPtr<ID3DBlob> CompileVS(const std::set<unsigned char>& macroIndices) const;
	Microsoft::WRL::ComPtr<ID3DBlob> CompilePS(const std::set<unsigned char>& macroIndices) const;

public:
	const std::string name;
	const std::string VS_SourceName;
	const std::string PS_SourceName;

private:
	mutable VertexShaderVariant VS_Default;
	mutable PixelShaderVariant PS_Default;

	mutable VSMAP VS_Variants;
	mutable PSMAP PS_Variants;

	std::vector<std::string> VS_Macros;
	std::vector<std::string> PS_Macros;

	std::string VS_Source;
	std::string PS_Source;

private:
#ifdef NDEBUG
	inline static const UINT flags1 = D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_OPTIMIZATION_LEVEL3;
#else
	inline static const UINT flags1 = D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG;
#endif	
	inline static const std::string VS_Model = "vs_5_0";
	inline static const std::string PS_Model = "ps_5_0";

};

