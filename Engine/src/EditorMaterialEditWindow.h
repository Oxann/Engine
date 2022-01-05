#pragma once
#include "Editor.h"
#include "Phong_Material.h"
#include "Unlit_Material.h"
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
		
		materialEdits.insert({typeid(Phong_Material),&EditorMaterialEditWindow::EditPhong});
		materialEdits.insert({typeid(Unlit_Material),&EditorMaterialEditWindow::EditUnlit});
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

		(this->*(this->materialEdit))(material);
	}

	void PopUp(Material* material)
	{
		isActive = true;
		name = material->GetFileName() + "###MaterialEdit";
		this->material = material;
		materialEdit = materialEdits[typeid(*material)];

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
			
			if(phongMaterial->diffuseMap)
				editorTEW->PopUp(const_cast<Texture*>(phongMaterial->diffuseMap));
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
			
			if(phongMaterial->specularMap)
				editorTEW->PopUp(const_cast<Texture*>(phongMaterial->specularMap));
		}

		//Normal map edit
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
			
			if(phongMaterial->normalMap)
				editorTEW->PopUp(const_cast<Texture*>(phongMaterial->normalMap));
		}

		//Shadows
		bool receiveShadows = phongMaterial->receiveShadows;
		ImGui::AlignTextToFramePadding();
		if(ImGui::Checkbox("Receive Shadows##receiveShadows", &receiveShadows))
			phongMaterial->SetReceiveShadows(receiveShadows);
	}

	void EditUnlit(Material* material)
	{
		Unlit_Material* unlitMaterial = static_cast<Unlit_Material*>(material);

		//Diffuse color edit
		ImGui::AlignTextToFramePadding();
		ImGui::Text("Color ");
		ImGui::SameLine();
		ImGui::ColorEdit4("##color", reinterpret_cast<float*>(&unlitMaterial->color));


		//Texture edit
		ImGui::AlignTextToFramePadding();
		ImGui::Text("Texture -> %s", unlitMaterial->texture ? unlitMaterial->texture->GetFileName().c_str() : "No Texture.");
		ImGui::SameLine();
		if (ImGui::RadioButton("##texture", true))
		{
			editorRSW->PopUp(EditorResourceSelectionWindow::Type::Texture,
				unlitMaterial->texture,
				[unlitMaterial](const ResourceBase* newResource) {
					unlitMaterial->SetTexture(static_cast<const Texture*>(newResource));
				});

			if(unlitMaterial->texture)
				editorTEW->PopUp(const_cast<Texture*>(unlitMaterial->texture));
		}
	}
private:

	Material* material;
	EditorResourceSelectionWindow* editorRSW;
	EditorTextureEditWindow* editorTEW;
	std::string opacityName;

	std::unordered_map <std::type_index, void(EditorMaterialEditWindow::*)(Material*)> materialEdits;
	void(EditorMaterialEditWindow::* materialEdit)(Material*) = nullptr;
};

