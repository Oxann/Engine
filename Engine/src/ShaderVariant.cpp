#include "ShaderVariant.h"
#include "Graphics.h"
#include "EngineAssert.h"
#include <d3dcompiler.h>

using namespace Microsoft::WRL;

VertexShaderVariant::VertexShaderVariant(Microsoft::WRL::ComPtr<ID3DBlob> blob)
{
	CHECK_DX_ERROR(Graphics::pDevice->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &vertexShader));
	InitLayout(blob);
}

void VertexShaderVariant::Bind() const
{
	Graphics::pDeviceContext->VSSetShader(vertexShader.Get(), nullptr, 0u);
	Graphics::pDeviceContext->IASetInputLayout(inputLayout.Get());
}

const std::vector<VertexBuffer::ElementType>& VertexShaderVariant::GetVertexElements() const
{
	return vertexElements;
}

void VertexShaderVariant::InitLayout(Microsoft::WRL::ComPtr<ID3DBlob> blob)
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
				vertexElements.emplace_back(VertexBuffer::ElementType::Position2D);
				break;
			case 7:
				temp_ilo.Format = DXGI_FORMAT_R32G32B32_FLOAT;
				vertexElements.emplace_back(VertexBuffer::ElementType::Position3D);
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
			vertexElements.emplace_back(VertexBuffer::ElementType::TexCoord);
		}
		else if (semanticName == "NORMAL")
		{
			temp_ilo.InputSlot = 2u;
			temp_ilo.Format = DXGI_FORMAT_R32G32B32_FLOAT;
			vertexElements.emplace_back(VertexBuffer::ElementType::Normal);
		}
		else if (semanticName == "TANGENT")
		{
			temp_ilo.InputSlot = 3u;
			temp_ilo.Format = DXGI_FORMAT_R32G32B32_FLOAT;
			vertexElements.emplace_back(VertexBuffer::ElementType::Tangent);
		}
		else if (semanticName == "BITANGENT")
		{
			temp_ilo.InputSlot = 4u;
			temp_ilo.Format = DXGI_FORMAT_R32G32B32_FLOAT;
			vertexElements.emplace_back(VertexBuffer::ElementType::Bitangent);
		}
		ilo.push_back(temp_ilo);
	}

	CHECK_DX_ERROR(Graphics::pDevice->CreateInputLayout(ilo.data(), (UINT)ilo.size(), blob->GetBufferPointer(), blob->GetBufferSize(), &inputLayout));
}

PixelShaderVariant::PixelShaderVariant(Microsoft::WRL::ComPtr<ID3DBlob> blob)
{
	CHECK_DX_ERROR(Graphics::pDevice->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &pixelShader));
}

void PixelShaderVariant::Bind() const
{
	Graphics::pDeviceContext->PSSetShader(pixelShader.Get(), nullptr, 0u);
}
