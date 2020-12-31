#pragma once
#include "Editor.h"
#include "EditorWindowBase.h"
#include "Scene.h"
#include "EditorEntityWindow.h"

class EditorSceneHierarchyWindow : public EditorWindowBase
{
public:
	void Start() override
	{
		name = "Scene";
		entityWindow = Editor::GetWindow<EditorEntityWindow>();
	}

	void Update() override
	{
		uniqueIndex = 0;
		for (const auto& entity : Scene::GetActiveScene()->Entities)
		{
			ShowEntityHierarchy(entity.get());
		}
	}
private:
	void ShowEntityHierarchy(Entity* entity)
	{
		//TODO: unique index mechanism wont work after removing an entity.

		uniqueIndex++;
		std::string nodeId = entity->name + "##" + std::to_string(uniqueIndex);
		auto flags = ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_OpenOnArrow
			| (!entity->Children.empty() ? 0 : ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_Leaf)
			| (uniqueIndex == selectedIndex ? ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_Selected : 0);

		bool isNodeExpanded = ImGui::TreeNodeEx(nodeId.c_str(), flags);

		if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
		{
			selectedIndex = uniqueIndex;
			entityWindow->PopUp(entity);
		}

		//Zooming selected entity
		if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
		{
			DirectX::XMFLOAT3 entityPosition = entity->GetTransform()->GetWorldPosition();
			Editor::Camera::position = DirectX::XMLoadFloat3(&entityPosition);
			Editor::Camera::isChanged = true;
		}


		if (isNodeExpanded)
		{
			for (int i = 0; i < entity->GetChildrenCount(); i++)
			{
				ShowEntityHierarchy(entity->Children[i].get());
			}
			ImGui::TreePop();
		}
		else
		{
			//If the node is not expanded we increment uniqueIndex by descendant count.
			//So every node will have its unique id whether its expanded or not.
			uniqueIndex += entity->GetDescendantCount(); 
		}
	}
private:
	EditorEntityWindow* entityWindow = nullptr;
	int uniqueIndex = 0;
	int selectedIndex = 0;
};