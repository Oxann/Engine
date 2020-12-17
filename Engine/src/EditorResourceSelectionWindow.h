#pragma once
#include "EditorWindowBase.h"
#include "Resources.h"

#include <functional>

class EditorResourceSelectionWindow : public EditorWindowBase
{
public:
	enum class Type
	{
		Texture,
		Mesh,
		Material
	};
public:
	void Start() override
	{
		Deactivate();
	}

	void Update() override
	{	
		switch (type)
		{
		case EditorResourceSelectionWindow::Type::Texture:
			if (ImGui::Selectable("null###nullTexture",!currentResource))
			{
				onResourceChange(nullptr);
				currentResource = nullptr;
			}
				
			for (const auto& texture : Resources::Textures)
			{
				bool isSelected = texture.second.get() == currentResource;
				if (ImGui::Selectable(texture.second->GetFileName().c_str(), isSelected))
				{
					onResourceChange(texture.second.get());
					currentResource = texture.second.get();
				}					
			}
			break;
		case EditorResourceSelectionWindow::Type::Material:
			for (const auto& model : Resources::Models)
			{
				for (const auto& material : model.second->GetMaterials())
				{
					bool isSelected = material.second.get() == currentResource;
					if (ImGui::Selectable(material.second->GetFileName().c_str(), isSelected))
						onResourceChange(material.second.get());
				}
			}
			break;
		case EditorResourceSelectionWindow::Type::Mesh:
			for (const auto& model : Resources::Models)
			{
				for (const auto& mesh : model.second->GetMeshes())
				{
					bool isSelected = mesh.second.get() == currentResource;			
					if (ImGui::Selectable(mesh.second->GetFileName().c_str(), isSelected))
						onResourceChange(mesh.second.get());
				}
			}
			break;
		default:
			break;
		}
	}

	void PopUp(Type type,const ResourceBase* currentResource, std::function<void(const ResourceBase*)> onResourceChange)
	{
		Activate();

		this->type = type;
		this->currentResource = currentResource;
		this->onResourceChange = onResourceChange;

		switch (type)
		{
		case EditorResourceSelectionWindow::Type::Texture:
			name = "Select Texture###ResourceSelection";
			break;
		case EditorResourceSelectionWindow::Type::Mesh:
			name = "Select Mesh###ResourceSelection";
			break;
		case EditorResourceSelectionWindow::Type::Material:
			name = "Select Material###ResourceSelection";
			break;
		default:
			break;
		}
	}
private:
	Type type;
	const ResourceBase* currentResource;
	std::function<void(const ResourceBase*)> onResourceChange;
};
