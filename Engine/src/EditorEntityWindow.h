#pragma once
#include "Editor.h"
#include "EditorWindowBase.h"
#include "Scene.h"
#include "EditorResourceSelectionWindow.h"
#include "EditorMaterialEditWindow.h"

class EditorEntityWindow : public EditorWindowBase
{
public:
	void Start() override
	{
		name = "Entity";
		editorRSW = Editor::GetWindow<EditorResourceSelectionWindow>();
		editorMEW = Editor::GetWindow<EditorMaterialEditWindow>();
	}

	void Update() override
	{
		//If any entity has been selected in scene hierarchy window, we display it.
		if (displayedEntity)
		{
			DisplayTransform();

			if(displayedRenderer)
				DisplayRenderer();
		}
	}

	void DisplayEntity(Entity* entity)
	{
		displayedEntity = entity;
		displayedTransform = displayedEntity->GetTransform();
		displayedRenderer = displayedEntity->GetComponent<Renderer>();
		strcpy_s(displayedName, displayedEntity->name.c_str());

		//Deactivating resource selection window when displayed entity is changed, because it causes misuse.
		editorRSW->Deactivate();
	}
private:
	void DisplayTransform()
	{	
		if (ImGui::CollapsingHeader("TRANSFORM"))
		{
			//Reset button 
			if (ImGui::Button("RESET", ImVec2(80.0f, 20.0f)))
				displayedTransform->Reset();

			//Name
			ImGui::AlignTextToFramePadding();
			ImGui::Text("NAME:");
			ImGui::SameLine();
			ImGui::InputText("##name", displayedName, 128u);
			if (ImGui::IsItemDeactivatedAfterEdit)
			{
				displayedEntity->name = displayedName;
			}

			//Position
			ImGui::AlignTextToFramePadding();
			ImGui::Text("POSITION:");
			ImGui::SameLine();
			float position[3] = { 
				displayedTransform->GetLocalPosition().x,
				displayedTransform->GetLocalPosition().y ,
				displayedTransform->GetLocalPosition().z };
			if(ImGui::InputFloat3("##position", position))
				displayedTransform->SetLocalPosition(position[0], position[1], position[2]);

			//Scale
			ImGui::AlignTextToFramePadding();
			ImGui::Text("SCALE:");
			ImGui::SameLine();
			float scale[3] = {
				displayedTransform->GetLocalScale().x,
				displayedTransform->GetLocalScale().y ,
				displayedTransform->GetLocalScale().z };

			if (ImGui::InputFloat3("##scale", scale))
				displayedTransform->SetLocalScale(scale[0], scale[1], scale[2]);

			//Rotation
			ImGui::AlignTextToFramePadding();
			ImGui::Text("ROTATION:");
			ImGui::SameLine();
			//Space (Local or World)
			static std::string currentSpace = "WORLD";
			ImGui::SameLine();
			if (ImGui::Button(currentSpace.c_str(), ImVec2(80.0f, 20.0f)))
			{
				if (currentSpace[0] == 'W')
					currentSpace = "LOCAL";
				else
					currentSpace = "WORLD";
			}

			ImGui::AlignTextToFramePadding();
			ImGui::Text("ANGLE:");
			ImGui::SameLine();
			static float angle = 0.0f;
			ImGui::InputFloat("##angle", &angle);
			if (ImGui::SmallButton("ROTATE X"))
			{
				if (currentSpace[0] == 'W')
					displayedTransform->RotateWorld(angle, 0.0f, 0.0f);
				else
					displayedTransform->RotateLocal(angle, 0.0f, 0.0f);
			}
			ImGui::SameLine();
			if (ImGui::SmallButton("ROTATE Y")) 
			{
				if (currentSpace[0] == 'W')
					displayedTransform->RotateWorld(0.0f, angle, 0.0f);
				else
					displayedTransform->RotateLocal(0.0f, angle, 0.0f);
			}
			ImGui::SameLine();
			if (ImGui::SmallButton("ROTATE Z"))
			{
				if (currentSpace[0] == 'W')
					displayedTransform->RotateWorld(0.0f, 0.0f, angle);
				else
					displayedTransform->RotateLocal(0.0f, 0.0f, angle);
			}
		}
	}

	void DisplayRenderer()
	{
		if (ImGui::CollapsingHeader("RENDERER"))
		{
			//Mesh selection
			ImGui::AlignTextToFramePadding();
			ImGui::Text("Mesh:");
			ImGui::SameLine();
			if (ImGui::RadioButton(displayedRenderer->GetMesh()->GetFileName().c_str(), true))
			{
				editorRSW->PopUp(EditorResourceSelectionWindow::Type::Mesh,
					displayedRenderer->mesh,
					[this](const ResourceBase* newResource) {
						displayedRenderer->SetMesh(static_cast<const Mesh*>(newResource));
					});
			}

			//Material selection
			bool isMaterialsExpanded = ImGui::TreeNodeEx("Materials");
			ImGui::SameLine();
			ImGui::Text("Count: %d", displayedRenderer->GetMaterials().size());
			if (isMaterialsExpanded)
			{
				for (int i = 0; i < displayedRenderer->materials.size(); i++)
				{
					if (ImGui::RadioButton(displayedRenderer->GetMaterials()[i]->GetFileName().c_str(), true))
					{
						editorMEW->PopUp(const_cast<Material*>(displayedRenderer->materials[i]));
						editorRSW->PopUp(EditorResourceSelectionWindow::Type::Material,
							displayedRenderer->materials[i],
							[this,i](const ResourceBase* newResource) {
								displayedRenderer->SetMaterial(static_cast<const Material*>(newResource),i);
							});
					}						
				}				
				ImGui::TreePop();
			}
		}
	}
private:
	Entity* displayedEntity = nullptr;
	Transform* displayedTransform = nullptr;
	Renderer* displayedRenderer = nullptr;
	static inline char displayedName[128];
	
	EditorResourceSelectionWindow* editorRSW;
	EditorMaterialEditWindow* editorMEW;
};