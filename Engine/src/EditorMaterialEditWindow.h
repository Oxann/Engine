#pragma once
#include "Editor.h"
#include "Phong_Material.h"

#include <typeindex>
#include <typeinfo>

class EditorMaterialEditWindow : public EditorWindowBase
{
public:
	void Start()
	{
		Deactivate();
		editorRSW = Editor::GetWindow<EditorResourceSelectionWindow>();
		
		materialEdits.insert({typeid(Phong_Material),&EditorMaterialEditWindow::EditPhong});
	}

	void Update()
	{
		(this->*(this->materialEdit))(material);
	}

	void PopUp(Material* material)
	{
		Activate();
		name = material->GetFileName() + "###MaterialEdit";
		this->material = material;
		materialEdit = materialEdits[typeid(*material)];
	}

private:
	void EditPhong(Material* material)
	{
		Phong_Material* phongMaterial = static_cast<Phong_Material*>(material);

		//Diffuse color edit
		ImGui::AlignTextToFramePadding();
		ImGui::Text("Diffuse Color ");
		ImGui::SameLine();
		ImGui::ColorEdit4("##diffusecolor", reinterpret_cast<float*>(&phongMaterial->diffuseColor));

		//Specular color edit
		ImGui::AlignTextToFramePadding();
		ImGui::Text("Specular Color ");
		ImGui::SameLine();
		ImGui::ColorEdit4("##specularColor", reinterpret_cast<float*>(&phongMaterial->specularColor));

		//Shininess edit
		ImGui::AlignTextToFramePadding();
		ImGui::Text("Shininess ");
		ImGui::SameLine();
		ImGui::SliderFloat("##shiniess", &phongMaterial->Shininess, 0.0f, 1.0f);

		//Shininess strength edit
		ImGui::AlignTextToFramePadding();
		ImGui::Text("Shininess Strength");
		ImGui::SameLine();
		ImGui::SliderFloat("##shiniessStrength", &phongMaterial->ShininessStrength, 0.0f, 1.0f);

		//Diffuse map edit
		ImGui::AlignTextToFramePadding();
		ImGui::Text("Diffuse Map -> %s", phongMaterial->diffuseMap ? phongMaterial->diffuseMap->GetFileName().c_str() : "No Texture.");
		ImGui::SameLine();
		if (ImGui::RadioButton("##diffuseMap", true))
		{
			editorRSW->PopUp(EditorResourceSelectionWindow::Type::Texture,
				phongMaterial->diffuseMap,
				[phongMaterial](const ResourceBase* newResource) {
					phongMaterial->SetDiffuseMap(static_cast<const Texture*>(newResource));
				});
		}

		//Specular map edit
		ImGui::AlignTextToFramePadding();
		ImGui::Text("Specular Map -> %s", phongMaterial->specularMap ? phongMaterial->specularMap->GetFileName().c_str() : "No texture.");
		ImGui::SameLine();
		if (ImGui::RadioButton("##specularMap", true))
		{
			editorRSW->PopUp(EditorResourceSelectionWindow::Type::Texture,
				phongMaterial->specularMap,
				[phongMaterial](const ResourceBase* newResource) {
					phongMaterial->SetSpecularMap(static_cast<const Texture*>(newResource));
				});
		}

		//Diffuse map edit
		ImGui::AlignTextToFramePadding();
		ImGui::Text("Normal Map -> %s", phongMaterial->normalMap ? phongMaterial->normalMap->GetFileName().c_str() : "No texture.");
		ImGui::SameLine();
		if (ImGui::RadioButton("##normalMap", true))
		{
			editorRSW->PopUp(EditorResourceSelectionWindow::Type::Texture,
				phongMaterial->normalMap,
				[phongMaterial](const ResourceBase* newResource) {
					phongMaterial->SetNormalMap(static_cast<const Texture*>(newResource));
				});
		}
	}
private:
	Material* material;
	EditorResourceSelectionWindow* editorRSW;
	
	std::unordered_map <std::type_index, void(EditorMaterialEditWindow::*)(Material*)> materialEdits;
	void(EditorMaterialEditWindow::* materialEdit)(Material*) = nullptr;
};

