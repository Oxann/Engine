#pragma comment(lib, "D3DCompiler.lib")
#pragma comment(lib, "dxguid.lib")


#include <filesystem>
#include <sstream>

#include "Shader.h"
#include "Log.h"
#include "EngineAssert.h"


using namespace Microsoft::WRL;

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

	//Reserving memory variants.
	VS_Variants.reserve(VS_Macros.size() * (VS_Macros.size() - 1 ? VS_Macros.size() - 1 : 1));
	PS_Variants.reserve(PS_Macros.size() * (PS_Macros.size() - 1 ? PS_Macros.size() - 1 : 1));

	//Compiling default shaders without any macro definiton.
	VS_Default = { CompileVS({}) };
	PS_Default = { CompilePS({}) };
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

PixelShaderVariant* const Shader::GetDefaultPixelShaderVariant() const
{
	return &PS_Default;
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
