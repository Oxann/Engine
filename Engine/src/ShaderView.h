#pragma once

#include "Shader.h"


class ShaderView
{
	friend class Material;
public:
	ShaderView(const Shader* shader);

	bool ActivateMacro_VS(const std::string& macro);
	bool ActivateMacro_PS(const std::string& macro);
	bool DeactivateMacro_VS(const std::string& macro);
	bool DeactivateMacro_PS(const std::string& macro);
 
	void Bind() const;
private:
	const Shader* shader;

	VertexShaderVariant* VS_ActiveVariant;
	PixelShaderVariant* PS_ActiveVariant;

	std::set<unsigned char> VS_ActiveMacros;
	std::set<unsigned char> PS_ActiveMacros;
};