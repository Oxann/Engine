#pragma once
#include "Editor.h"
#include "Material.h"


class EditorMaterialEditWindow : public EditorWindowBase
{
public:
	void Start()
	{
		Deactivate();
		editorRSW = Editor::GetWindow<EditorResourceSelectionWindow>();
	}

	void Update()
	{
		//Diffuse color edit
		ImGui::AlignTextToFramePadding();
		ImGui::Text("Diffuse Color ");
		ImGui::SameLine();
		ImGui::ColorEdit4("##diffusecolor", reinterpret_cast<float*>(&material->diffuseColor));

		//Specular color edit
		ImGui::AlignTextToFramePadding();
		ImGui::Text("Specular Color ");
		ImGui::SameLine();
		ImGui::ColorEdit4("##specularColor", reinterpret_cast<float*>(&material->specularColor));

		//Shininess edit
		ImGui::AlignTextToFramePadding();
		ImGui::Text("Shininess ");
		ImGui::SameLine();
		ImGui::SliderFloat("##shiniess", &material->Shininess,0.0f,1.0f);

		//Shininess strength edit
		ImGui::AlignTextToFramePadding();
		ImGui::Text("Shininess Strength");
		ImGui::SameLine();
		ImGui::SliderFloat("##shiniessStrength", &material->ShininessStrength, 0.0f, 1.0f);

		//Diffuse map edit
		ImGui::AlignTextToFramePadding();
		ImGui::Text("Diffuse Map -> %s", material->diffuseMap ? material->diffuseMap->GetFileName().c_str() : "No Texture.");
		ImGui::SameLine();
		if (ImGui::RadioButton("##diffuseMap", true))
		{
			editorRSW->PopUp(EditorResourceSelectionWindow::Type::Texture,
				material->diffuseMap,
				[this](const ResourceBase* newResource) {
					material->SetDiffuseMap(static_cast<const Texture*>(newResource));
				});
		}

		//Specular map edit
		ImGui::AlignTextToFramePadding();
		ImGui::Text("Specular Map -> %s", material->specularMap ? material->specularMap->GetFileName().c_str() : "No texture.");
		ImGui::SameLine();
		if (ImGui::RadioButton("##specularMap", true))
		{
			editorRSW->PopUp(EditorResourceSelectionWindow::Type::Texture,
				material->specularMap,
				[this](const ResourceBase* newResource) {
					material->SetSpecularMap(static_cast<const Texture*>(newResource));
				});
		}

		//Diffuse map edit
		ImGui::AlignTextToFramePadding();
		ImGui::Text("Normal Map -> %s", material->normalMap ? material->normalMap->GetFileName().c_str() : "No texture.");
		ImGui::SameLine();
		if (ImGui::RadioButton("##normalMap", true))
		{
			editorRSW->PopUp(EditorResourceSelectionWindow::Type::Texture,
				material->normalMap,
				[this](const ResourceBase* newResource) {
					material->SetNormalMap(static_cast<const Texture*>(newResource));
				});
		}		
	}

	void PopUp(Material* mat)
	{
		Activate();
		name = mat->GetFileName() + "###MaterialEdit";
		material = mat;
	}
private:
	Material* material;
	EditorResourceSelectionWindow* editorRSW;
};

