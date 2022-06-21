#pragma once

#include "Shader.h"


class ShaderView
{
	friend class Material;

#ifdef EDITOR
	friend class EditorMaterialEditWindow;
#endif

public:
	ShaderView(const Shader* shader);

	bool ActivateMacro_VS(std::string_view macro);
	bool ActivateMacro_PS(std::string_view macro);
	bool DeactivateMacro_VS(std::string_view macro);
	bool DeactivateMacro_PS(std::string_view macro);
 
	void Bind() const;

	const VertexShaderVariant& GetActiveVertexShader() const;
	const PixelShaderVariant& GetActivePixelShader() const;
private:
	const Shader* shader;

	VertexShaderVariant* VS_ActiveVariant;
	PixelShaderVariant* PS_ActiveVariant;

	std::set<unsigned char> VS_ActiveMacros;
	std::set<unsigned char> PS_ActiveMacros;
};