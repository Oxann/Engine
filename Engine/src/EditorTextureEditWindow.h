#pragma once
#include "Editor.h"
#include "Texture.h"

class EditorTextureEditWindow : public EditorWindowBase
{
public:
	void Start() override
	{
		isActive = false;
	}

	void Update() override
	{
		//Width and height
		ImGui::AlignTextToFramePadding();
		ImGui::Text("Width: %d    height: %d", texture->width, texture->height);

		//Filter mode selection
		ImGui::AlignTextToFramePadding();
		ImGui::Text("Filter Mode:");
		ImGui::SameLine();
		
		const Texture::FilterMode fm = texture->GetFilterMode();

		if (ImGui::BeginCombo("##filterMode", filterMode.c_str()))
		{
			if (ImGui::Selectable("Point", fm == Texture::FilterMode::POINT))
			{
				filterMode = "Point";
				texture->SetFilterMode(Texture::FilterMode::POINT);
			}	
			if (ImGui::Selectable("Linear", fm == Texture::FilterMode::LINEAR))
			{
				filterMode = "Linear";
				texture->SetFilterMode(Texture::FilterMode::LINEAR);
			}
			if (ImGui::Selectable("Bilinear", fm == Texture::FilterMode::BILINEAR))
			{
				filterMode = "Bilinear";
				texture->SetFilterMode(Texture::FilterMode::BILINEAR);
			}
			if (ImGui::Selectable("Trilinear", fm == Texture::FilterMode::TRILINEAR))
			{
				filterMode = "Trilinear";
				texture->SetFilterMode(Texture::FilterMode::TRILINEAR);
			}
			if (ImGui::Selectable("Anisotropic", fm == Texture::FilterMode::ANISOTROPIC))
			{
				filterMode = "Anisotropic";
				texture->SetFilterMode(Texture::FilterMode::ANISOTROPIC);
			}
			ImGui::EndCombo();
		}

		//If filter mode is anisotropic, anisotropy level is adjustable.
		if (fm == Texture::FilterMode::ANISOTROPIC)
		{
			ImGui::AlignTextToFramePadding();
			ImGui::Text("Anisotropy: ");
			ImGui::SameLine();			
			ImGui::SliderInt("##anisotropyLevel", &anisotropy, 1, 16);
			if(ImGui::IsItemDeactivatedAfterEdit())
				texture->SetAnisotropy(anisotropy);
		}		
		const float textureRatio = (float)texture->width / texture->height;
		ImGui::Image((void*)(texture->GetResourceView().Get()), ImVec2(textureRatio * 300.0f, 300));
	}

	void PopUp(Texture* texture)
	{
		isActive = true;
		name = texture->GetFileName() + "###TextureEdit";
		this->texture = texture;
		anisotropy = texture->GetAnisotropy();
		
		switch (texture->GetFilterMode())
		{
		case Texture::FilterMode::POINT:
			filterMode = "Point";
			break;
		case Texture::FilterMode::LINEAR:
			filterMode = "Linear";
			break;
		case Texture::FilterMode::BILINEAR:
			filterMode = "Bilinear";
			break;
		case Texture::FilterMode::TRILINEAR:
			filterMode = "Trilinear";
			break;
		case Texture::FilterMode::ANISOTROPIC:
			filterMode = "Anisotropic";
			break;
		default:
			break;
		}
	}
private:
	Texture* texture;
	std::string filterMode;
	int anisotropy;
};