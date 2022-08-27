#pragma comment(lib, "D3DCompiler.lib")
#pragma comment(lib, "dxguid.lib")


#include <filesystem>
#include <sstream>
#include <d3dcompiler.h>
#include <d3d11.h>
#include <wrl.h>

#include "Shader.h"
#include "Log.h"
#include "EngineAssert.h"
#include "VertexBuffer.h"
#include "ConstantBuffer.h"


using namespace Microsoft::WRL;

Shader::VertexShaderPerFrameBuffer Shader::VertexShaderPerFrameBuffer::buffer;
Shader::VertexShaderPerObjectBuffer Shader::VertexShaderPerObjectBuffer::buffer;
Shader::VertexShaderShadowBuffer Shader::VertexShaderShadowBuffer::buffer;
Shader::PixelShaderPerFrameBuffer Shader::PixelShaderPerFrameBuffer::buffer;

Shader::Shader(const std::string& name,const std::filesystem::path& VS_Path, const std::filesystem::path& PS_Path)
	:name(name),
	VS_SourceName(VS_Path.string()),
	PS_SourceName(PS_Path.string())
{
	ENGINEASSERT(std::filesystem::exists(VS_Path) && std::filesystem::exists(PS_Path), "Shader paths are not existing.")

	//Reading vertex shader.
	std::fstream VS_IN(VS_Path.string().c_str(), std::ios_base::in);
	
	std::stringstream VS_tempSource;
	VS_tempSource << VS_IN.rdbuf();
	
	ExtractMacrosFromSource(VS_tempSource, VS_Macros);

	VS_Source = VS_tempSource.str();

	//Reading pixel shader.
	std::fstream PS_IN(PS_Path.string().c_str(), std::ios_base::in);

	std::stringstream PS_tempSource;
	PS_tempSource << PS_IN.rdbuf();

	ExtractMacrosFromSource(PS_tempSource, PS_Macros);

	PS_Source = PS_tempSource.str();

	//Reserving memory for variants.
	VS_Variants.reserve(VS_Macros.size() * (VS_Macros.size() - 1 ? VS_Macros.size() - 1 : 1));
	PS_Variants.reserve(PS_Macros.size() * (PS_Macros.size() - 1 ? PS_Macros.size() - 1 : 1));

	//Compiling default shaders without any macro definiton.
	VS_Default = { CompileVS({}) };

	Microsoft::WRL::ComPtr<ID3DBlob> psBlob = CompilePS({});
	PS_Default = { psBlob };
	InitMaterialBufferAndTextures(psBlob);
}

const std::vector<std::string>& Shader::GetVertexShaderMacros() const
{
	return VS_Macros;
}

const std::vector<std::string>& Shader::GetPixelShaderMacros() const
{
	return PS_Macros;
}

const size_t Shader::GetVertexShaderMacroCount() const
{
	return VS_Macros.size();
}

const size_t Shader::GetPixelShaderMacroCount() const
{
	return PS_Macros.size();
}

VertexShaderVariant* const Shader::GetVertexShaderVariant(const std::set<unsigned char>& macroIndices) const
{
	for (auto& variant : VS_Variants)
	{
		if (macroIndices == variant.first)
			return &variant.second;
	}

	return &VS_Variants.emplace_back(std::piecewise_construct, std::forward_as_tuple(macroIndices), std::forward_as_tuple(CompileVS(macroIndices))).second;
}

PixelShaderVariant* const Shader::GetPixelShaderVariant(const std::set<unsigned char>& macroIndices) const
{
	for (auto& variant : PS_Variants)
	{
		if (macroIndices == variant.first)
			return &variant.second;
	}

	return &PS_Variants.emplace_back(std::piecewise_construct, std::forward_as_tuple(macroIndices), std::forward_as_tuple(CompilePS(macroIndices))).second;
}

VertexShaderVariant* const Shader::GetDefaultVertexShaderVariant() const
{
	return &VS_Default;
}

bool Shader::HasMacro(std::string_view macro) const
{
	return std::ranges::find(VS_Macros, macro) != VS_Macros.end();
}

PixelShaderVariant* const Shader::GetDefaultPixelShaderVariant() const
{
	return &PS_Default;
}

VS_ConstantBuffer<Shader::VertexShaderPerObjectBuffer>* const Shader::GetVertexShaderPerObjectBuffer()
{
	static VS_ConstantBuffer<Shader::VertexShaderPerObjectBuffer> perObjectBuffer(	&Shader::VertexShaderPerObjectBuffer::buffer, 
																					1, 
																					Shader::VertexShaderPerObjectBuffer::slot,
																					D3D11_USAGE::D3D11_USAGE_DYNAMIC,
																					D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_WRITE,
																					true);
	return &perObjectBuffer;
}

VS_ConstantBuffer<Shader::VertexShaderPerFrameBuffer>* const Shader::GetVertexShaderPerFrameBuffer()
{
	static VS_ConstantBuffer<Shader::VertexShaderPerFrameBuffer> perFrameBuffer(	&Shader::VertexShaderPerFrameBuffer::buffer,
																					1,
																					Shader::VertexShaderPerFrameBuffer::slot,
																					D3D11_USAGE::D3D11_USAGE_DYNAMIC,
																					D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_WRITE,
																					true);

	return &perFrameBuffer;
}

VS_ConstantBuffer<Shader::VertexShaderShadowBuffer>* const Shader::GetVertexShaderShadowBuffer()
{
	static VS_ConstantBuffer<Shader::VertexShaderShadowBuffer> shadowBuffer(&Shader::VertexShaderShadowBuffer::buffer,
		1,
		Shader::VertexShaderShadowBuffer::slot,
		D3D11_USAGE::D3D11_USAGE_DYNAMIC,
		D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_WRITE,
		true);

	return &shadowBuffer;
}

void Shader::ExtractMacrosFromSource(std::stringstream& source, std::vector<std::string>& macros)
{
	macros.reserve(10u);

	static const std::string variantsBegin = "// VARIANTS BEGIN";
	static const std::string variantsEnd = "// VARIANTS END";

	//Shader must begin with "// VARIANTS BEGIN"
	std::string line;
	std::getline(source, line);

	ENGINEASSERT(line == variantsBegin, "Shaders must have // VARIANTS BEGIN --- // VARIANTS END section at the beginning.");

	// Extracting variant macros.
	while (std::getline(source, line) && line != variantsEnd)
		macros.emplace_back(line.begin() + 3, line.end());

	ENGINEASSERT(line == variantsEnd, "Shaders must have // VARIANTS BEGIN --- // VARIANTS END section at the beginning.");

	macros.shrink_to_fit();
}

Microsoft::WRL::ComPtr<ID3DBlob> Shader::CompileVS(const std::set<unsigned char>& macroIndices) const
{
	Microsoft::WRL::ComPtr<ID3DBlob> blob;

	std::vector<D3D_SHADER_MACRO> macros;
	macros.reserve(macroIndices.size());

	for (const auto& macro : macroIndices)
	{
		macros.push_back({ VS_Macros[macro].c_str(), NULL });
	}

	//D3D_SHADER_MACRO expects NULL,NULL for the last element.
	macros.push_back({ NULL, NULL });

	//Compiling shader variant in debug mode.
	Microsoft::WRL::ComPtr<ID3DBlob> errorMessages;

	D3DCompile(VS_Source.data(),
		VS_Source.size(),
		VS_SourceName.c_str(),
		macros.data(),
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"main",
		"vs_5_0",
		flags1,
		0u,
		&blob,
		&errorMessages);

	ENGINEASSERT(errorMessages == nullptr, (char*)errorMessages->GetBufferPointer())

	return blob;
}

Microsoft::WRL::ComPtr<ID3DBlob> Shader::CompilePS(const std::set<unsigned char>& macroIndices) const
{
	Microsoft::WRL::ComPtr<ID3DBlob> blob;

	std::vector<D3D_SHADER_MACRO> macros;
	macros.reserve(macroIndices.size());

	for (const auto& macro : macroIndices)
	{
		macros.push_back({ PS_Macros[macro].c_str(), NULL });
	}

	//D3D_SHADER_MACRO expects NULL,NULL for the last element.
	macros.push_back({ NULL, NULL });

	//Compiling shader variant in debug mode.
	Microsoft::WRL::ComPtr<ID3DBlob> errorMessages;

	D3DCompile(PS_Source.data(),
		PS_Source.size(),
		PS_SourceName.c_str(),
		macros.data(),
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"main",
		"ps_5_0",
		flags1,
		0u,
		&blob,
		&errorMessages);

	ENGINEASSERT(errorMessages == nullptr, (char*)errorMessages->GetBufferPointer())

	return blob;
}

bool Shader::HasMaterial() const
{
	return materialBuffer != nullptr;
}

unsigned int Shader::GetMaterialBufferSize() const
{
	return materialBuffer->GetSize();
}

const std::vector<Shader::MaterialBufferVariableDefinition>& Shader::GetFloat1Defs() const
{
	return float1s;
}

const std::vector<Shader::MaterialBufferVariableDefinition>& Shader::GetFloat2Defs() const
{
	return float2s;
}

const std::vector<Shader::MaterialBufferVariableDefinition>& Shader::GetFloat3Defs() const
{
	return float3s;
}

const std::vector<Shader::MaterialBufferVariableDefinition>& Shader::GetFloat4Defs() const
{
	return float4s;
}

void Shader::InitMaterialBufferAndTextures(Microsoft::WRL::ComPtr<ID3DBlob> pixelShaderBlob)
{
	//Reflection
	ComPtr<ID3D11ShaderReflection> pixelShaderReflection;
	CHECK_DX_ERROR(D3DReflect(pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize(), IID_ID3D11ShaderReflection, &pixelShaderReflection));
	
	ID3D11ShaderReflectionConstantBuffer* materialBufferReflection = pixelShaderReflection->GetConstantBufferByName(materialBufferName.c_str());
	D3D11_SHADER_BUFFER_DESC materialBufferDesc;
	HRESULT hasMaterial = materialBufferReflection->GetDesc(&materialBufferDesc);

	if (hasMaterial == E_FAIL)
		return;

	//Allocating the actual buffer.
	materialBuffer = new PS_ConstantBuffer<unsigned char>(nullptr, materialBufferDesc.Size, materialBufferSlot, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_WRITE, false);

	for (int i = 0; i < materialBufferDesc.Variables; i++)
	{
		ID3D11ShaderReflectionVariable* variable = materialBufferReflection->GetVariableByIndex(i);

		D3D11_SHADER_VARIABLE_DESC variableDesc;
		variable->GetDesc(&variableDesc);

		D3D11_SHADER_TYPE_DESC typeDesc;
		variable->GetType()->GetDesc(&typeDesc);

		switch (typeDesc.Type)
		{
		case D3D_SVT_FLOAT:
			if (variableDesc.Size == 4)
				float1s.emplace_back(variableDesc.Name, variableDesc.StartOffset);
			else if (variableDesc.Size == 8)
				float2s.emplace_back(variableDesc.Name, variableDesc.StartOffset);
			else if (variableDesc.Size == 12)
				float3s.emplace_back(variableDesc.Name, variableDesc.StartOffset);
			else if (variableDesc.Size == 16)
				float4s.emplace_back(variableDesc.Name, variableDesc.StartOffset);
			break;
		default:
			break;
		}
	}
}

PS_ConstantBuffer<Shader::PixelShaderPerFrameBuffer>* const Shader::GetPixelShaderPerFrameBuffer()
{
	static PS_ConstantBuffer<Shader::PixelShaderPerFrameBuffer> perFrameBuffer(&Shader::PixelShaderPerFrameBuffer::buffer,
		1,
		Shader::PixelShaderPerFrameBuffer::slot,
		D3D11_USAGE::D3D11_USAGE_DYNAMIC,
		D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_WRITE,
		true);

	return &perFrameBuffer;
}
