#include "ShaderView.h"

ShaderView::ShaderView(const Shader* shader)
{
	this->shader = shader;
	VS_ActiveVariant = shader->GetDefaultVertexShaderVariant();
	PS_ActiveVariant = shader->GetDefaultPixelShaderVariant();
}

bool ShaderView::ActivateMacro_VS(const std::string& macro)
{
	for (int i = 0; i < shader->VS_Macros.size(); i++)
	{
		if (macro == shader->VS_Macros[i])
		{
			VS_ActiveMacros.emplace(i);
			VS_ActiveVariant = shader->GetVertexShaderVariant(VS_ActiveMacros);
			return true;
		}
	}

	return false;
}

bool ShaderView::ActivateMacro_PS(const std::string& macro)
{
	for (int i = 0; i < shader->PS_Macros.size(); i++)
	{
		if (macro == shader->PS_Macros[i])
		{
			PS_ActiveMacros.emplace(i);
			PS_ActiveVariant = shader->GetPixelShaderVariant(PS_ActiveMacros);
			return true;
		}
	}

	return false;
}

bool ShaderView::DeactivateMacro_PS(const std::string& macro)
{
	for (int i = 0; i < shader->PS_Macros.size(); i++)
	{
		if (macro == shader->PS_Macros[i])
		{
			PS_ActiveMacros.erase(i);
			PS_ActiveVariant = shader->GetPixelShaderVariant(PS_ActiveMacros);
			return true;
		}
	}

	return false;
}

void ShaderView::Bind() const
{
	VS_ActiveVariant->Bind();
	PS_ActiveVariant->Bind();
}

const VertexShaderVariant& ShaderView::GetActiveVertexShader() const
{
	return *VS_ActiveVariant;
}

const PixelShaderVariant& ShaderView::GetActivePixelShader() const
{
	return *PS_ActiveVariant;
}

bool ShaderView::DeactivateMacro_VS(const std::string& macro)
{
	for (int i = 0; i < shader->VS_Macros.size(); i++)
	{
		if (macro == shader->VS_Macros[i])
		{
			VS_ActiveMacros.erase(i);
			VS_ActiveVariant = shader->GetVertexShaderVariant(VS_ActiveMacros);
			return true;
		}
	}

	return false;
}
