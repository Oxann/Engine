#pragma comment(lib, "D3DCompiler.lib")
#pragma comment(lib, "dxguid.lib")

#include <d3dcompiler.h>
#include <filesystem>
#include <sstream>

#include "Shader.h"
#include "Log.h"
#include "EngineAssert.h"

using namespace Microsoft::WRL;

VertexShader::VertexShader(std::filesystem::path file)
	:ResourceBase(file)
{
	ComPtr<ID3DBlob> Blob;
	CHECK_DX_ERROR(D3DReadFileToBlob(file.wstring().c_str(), &Blob));
	ComPtr<ID3D11VertexShader> vs;
	CHECK_DX_ERROR(GetDevice()->CreateVertexShader(Blob->GetBufferPointer(), Blob->GetBufferSize(), nullptr, &vs));
	this->vs = vs;
	InitLayout(Blob);
}

void VertexShader::InitLayout(Microsoft::WRL::ComPtr<ID3DBlob> blob)
{
	//Reflection
	ComPtr<ID3D11ShaderReflection> vertexShaderReflection;
	CHECK_DX_ERROR(D3DReflect(blob->GetBufferPointer(), blob->GetBufferSize(), IID_ID3D11ShaderReflection, &vertexShaderReflection));

	//Desc
	D3D11_SHADER_DESC vertexShaderDesc;
	vertexShaderReflection->GetDesc(&vertexShaderDesc);

	//Input layout from desc
	std::vector<D3D11_INPUT_ELEMENT_DESC> ilo;
	for (unsigned int i = 0; i < vertexShaderDesc.InputParameters; i++)
	{
		D3D11_SIGNATURE_PARAMETER_DESC parameterDesc;
		vertexShaderReflection->GetInputParameterDesc(i, &parameterDesc);

		D3D11_INPUT_ELEMENT_DESC temp_ilo;
		temp_ilo.SemanticName = parameterDesc.SemanticName;
		temp_ilo.SemanticIndex = parameterDesc.SemanticIndex;
		temp_ilo.InstanceDataStepRate = 0;
		temp_ilo.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		temp_ilo.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;

		std::string semanticName = temp_ilo.SemanticName;
		if (semanticName == "POSITION")
		{
			temp_ilo.InputSlot = 0u;
			switch (parameterDesc.Mask)
			{
			case 1:
				temp_ilo.Format = DXGI_FORMAT_R32_FLOAT;
				break;
			case 3:
				temp_ilo.Format = DXGI_FORMAT_R32G32_FLOAT;
				break;
			case 7:
				temp_ilo.Format = DXGI_FORMAT_R32G32B32_FLOAT;
				break;
			case 15:
				temp_ilo.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
				break;
			default:
				break;
			}
		}
		else if (semanticName == "TEXCOORD")
		{
			temp_ilo.InputSlot = 1u;
			temp_ilo.Format = DXGI_FORMAT_R32G32_FLOAT;
		}
		else if (semanticName == "NORMAL")
		{
			temp_ilo.InputSlot = 2u;
			temp_ilo.Format = DXGI_FORMAT_R32G32B32_FLOAT;
		}
		else if (semanticName == "TANGENT")
		{
			temp_ilo.InputSlot = 3u;
			temp_ilo.Format = DXGI_FORMAT_R32G32B32_FLOAT;
		}
		else if (semanticName == "BITANGENT")
		{
			temp_ilo.InputSlot = 4u;
			temp_ilo.Format = DXGI_FORMAT_R32G32B32_FLOAT;
		}
		ilo.push_back(temp_ilo);
	}
	ComPtr<ID3D11InputLayout> inputLayout;
	CHECK_DX_ERROR(GetDevice()->CreateInputLayout(ilo.data(), (UINT)ilo.size(), blob->GetBufferPointer(), blob->GetBufferSize(), &inputLayout));
	layout = inputLayout;
}

PixelShader::PixelShader(std::filesystem::path file)
	:ResourceBase(file)
{	
	ComPtr<ID3DBlob> Blob;
	CHECK_DX_ERROR(D3DReadFileToBlob(file.wstring().c_str(), &Blob));
	ComPtr<ID3D11PixelShader> ps;
	CHECK_DX_ERROR(GetDevice()->CreatePixelShader(Blob->GetBufferPointer(), Blob->GetBufferSize(), nullptr, &ps));
	this->ps = ps;
}
