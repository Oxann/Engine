#pragma once
#include "Editor.h"
#include "EditorTextureEditWindow.h"

#include <typeindex>
#include <typeinfo>

class EditorMaterialEditWindow : public EditorWindowBase
{
public:
	void Start() override
	{
		isActive = false;
		editorRSW = Editor::GetWindow<EditorResourceSelectionWindow>();
		editorTEW = Editor::GetWindow<EditorTextureEditWindow>();
		name = "Material";
	}

	void Update() override
	{
		ImGui::AlignTextToFramePadding();
		ImGui::Text("Opacity Mode:");
		ImGui::SameLine();

		const Material::Mode opacity = material->mode;
		if (ImGui::BeginCombo("##opacityMode", opacityName.c_str()))
		{
			if (ImGui::Selectable("Opaque", opacity == Material::Mode::Opaque))
			{
				opacityName = "Opaque";
				material->mode = Material::Mode::Opaque;
			}
			if (ImGui::Selectable("Transparent", opacity == Material::Mode::Transparent))
			{
				opacityName = "Transparent";
				material->mode = Material::Mode::Transparent;
			}
			ImGui::EndCombo();
		}

		char materialVarDefLabel[4] = {0};
		materialVarDefLabel[0] = '#';
		materialVarDefLabel[1] = '#';
		materialVarDefLabel[2] = 1;

		for (const auto& float1Def : material->shaderView.shader->GetFloat1Defs())
		{
			ImGui::AlignTextToFramePadding();
			ImGui::Text(float1Def.name.c_str());
			ImGui::SameLine();

			float currenValue = material->GetFloat(float1Def.name);
			if (ImGui::DragFloat(materialVarDefLabel, &currenValue, 0.001f))
			{
				material->SetFloat(float1Def.name, currenValue);
			}

			materialVarDefLabel[2]++;
		}

		for (const auto& float2Def : material->shaderView.shader->GetFloat2Defs())
		{
			ImGui::AlignTextToFramePadding();
			ImGui::Text(float2Def.name.c_str());
			ImGui::SameLine();

			DirectX::XMFLOAT2 currenValue = material->GetFloat2(float2Def.name);
			if (ImGui::DragFloat2(materialVarDefLabel, reinterpret_cast<float*>(&currenValue), 0.001f))
			{
				material->SetFloat2(float2Def.name, currenValue);
			}

			materialVarDefLabel[2]++;
		}
	
		for (const auto& float3Def : material->shaderView.shader->GetFloat3Defs())
		{
			ImGui::AlignTextToFramePadding();
			ImGui::Text(float3Def.name.c_str());
			ImGui::SameLine();

			DirectX::XMFLOAT3 currenValue = material->GetFloat3(float3Def.name);
			if (ImGui::DragFloat3(materialVarDefLabel, reinterpret_cast<float*>(&currenValue), 0.001f))
			{
				material->SetFloat3(float3Def.name, currenValue);
			}

			materialVarDefLabel[2]++;
		}
	
		for (const auto& float4Def : material->shaderView.shader->GetFloat4Defs())
		{
			ImGui::AlignTextToFramePadding();
			ImGui::Text(float4Def.name.c_str());
			ImGui::SameLine();

			DirectX::XMFLOAT4 currenValue = material->GetFloat4(float4Def.name);
			if (ImGui::ColorEdit4(materialVarDefLabel, reinterpret_cast<float*>(&currenValue)))
			{
				material->SetFloat4(float4Def.name, currenValue);
			}

			materialVarDefLabel[2]++;
		}

		for (const auto& texture2DDef : material->shaderView.GetActivePixelShader().GetTexture2Ds())
		{
			ImGui::AlignTextToFramePadding();
			ImGui::Text(texture2DDef.name.c_str());
			ImGui::SameLine();

			Texture* texture = material->GetTexture(texture2DDef.name);
			
			if (ImGui::RadioButton(materialVarDefLabel, true))
			{
				editorRSW->PopUp(EditorResourceSelectionWindow::Type::Texture,
					texture,
					[this, texture2DDef](ResourceBase* newResource) {
						material->SetTexture(texture2DDef.name, static_cast<Texture*>(newResource));
					});

				if (texture)
					editorTEW->PopUp(texture);
			}

			if (texture)
			{
				ImGui::SameLine();
				ImGui::Text(texture->GetFileName().c_str());
				ImGui::AlignTextToFramePadding();
			}
			else
			{
				ImGui::SameLine();
				ImGui::Text("NONE");
				ImGui::AlignTextToFramePadding();
			}

			materialVarDefLabel[2]++;
		}

		ShowMacros();
	}

	void PopUp(Material* material)
	{
		isActive = true;
		name = material->GetFileName() + "###MaterialEdit";
		this->material = material;

		switch (material->mode)
		{
		case Material::Mode::Opaque:
			opacityName = "Opaque";
			break;
		case Material::Mode::Transparent:
			opacityName = "Transparent";
			break;
		default:
			break;
		}
	}

private:
	void ShowMacros()
	{
		if (ImGui::CollapsingHeader("SHADER MACROS"))
		{
			const auto& macros = material->shaderView.shader->GetPixelShaderMacros();
			for (int i = 0; i < macros.size(); i++)
			{
				bool isMacroActive = material->shaderView.PS_ActiveMacros.find(i) != material->shaderView.PS_ActiveMacros.end();

				if (ImGui::RadioButton(macros[i].c_str(), isMacroActive))
				{
					if (isMacroActive)
						material->DeactivateMacro(macros[i]);
					else
						material->ActivateMacro(macros[i]);
				}
			}
		}
	}

private:

	Material* material;
	EditorResourceSelectionWindow* editorRSW;
	EditorTextureEditWindow* editorTEW;
	std::string opacityName;
};

