#ifdef EDITOR

#include "Editor.h"
#include "Input.h"
#include "Time.h"
#include "MainWindow.h"

#include "EditorEntityWindow.h"
#include "EditorSceneHierarchyWindow.h"
#include "EditorResourceSelectionWindow.h"
#include "EditorMaterialEditWindow.h"
#include "EditorTextureEditWindow.h"
#include "EditorMetricsWindow.h"

#include <DirectXCollision.h>
#include <limits>

void Editor::Init(HWND hWnd, ID3D11DeviceContext* pDeviceContext, ID3D11Device* pDevice)
{
	ImGui::CreateContext();
	imguiIO = &ImGui::GetIO();
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(hWnd);
	ImGui_ImplDX11_Init(pDevice, pDeviceContext);

	//Windows
	windows.push_back(std::make_unique<EditorEntityWindow>());
	windows.push_back(std::make_unique<EditorSceneHierarchyWindow>());
	windows.push_back(std::make_unique<EditorResourceSelectionWindow>());
	windows.push_back(std::make_unique<EditorMaterialEditWindow>());
	windows.push_back(std::make_unique<EditorTextureEditWindow>());
	windows.push_back(std::make_unique<EditorMetricsWindow>());

	//Windows start calls
	for (const auto& window : windows)
		window->Start();
}

LRESULT Editor::EditorInputHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);
}

void Editor::ShutDown()
{
	ImGui_ImplWin32_Shutdown();
	ImGui_ImplDX11_Shutdown();
	ImGui::DestroyContext();
}

void Editor::Update()
{
	if (isActive)
	{
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		//Main Menu
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("Windows##windowsMenu"))
			{
				if (ImGui::MenuItem("Scene##windowsMenu/Scene", nullptr, nullptr))
					windows[1]->isActive = true;
				
				if (ImGui::MenuItem("Metrics##windowsMenu/Metrics", nullptr, nullptr))
					windows[5]->isActive = true;

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Draw Mode##drawModeMenu"))
			{
				if (ImGui::MenuItem("Solid##drawModeMenu/Solid", nullptr, nullptr))
				{
					isWireframeEnabled = false;
				}
				if (ImGui::MenuItem("Wireframe##drawModeMenu/Wireframe", nullptr, nullptr))
				{
					isWireframeEnabled = true;
				}	

				ImGui::EndMenu();
			}

			ImGui::EndMainMenuBar();
		}

		//Windows update calls	
		for (auto& window : windows)
		{
			if (window->isActive)
			{
				if (ImGui::Begin(window->name.c_str(), &window->isActive))
					window->Update();

				ImGui::End();
			}
		}
	}

	Camera::Update();
	
	if (Input::GetKeyDown(VK_LBUTTON))
	{
		Entity* pickedEntity = nullptr;
		float distance = std::numeric_limits<float>::max();

		DirectX::XMFLOAT4X4 projection;
		DirectX::XMStoreFloat4x4(&projection, Graphics::projectionMatrix);

		float viewSpaceX = (((2.0f * Input::GetMouseX()) / MainWindow::GetDisplayResolution().width) - 1.0f) / projection(0, 0);
		float viewSpaceY = -(((2.0f * Input::GetMouseY()) / MainWindow::GetDisplayResolution().height) - 1.0f) / projection(1, 1);

		for (auto& entity : Scene::GetActiveScene()->Entities)
		{
			MousePick(entity.get(), distance, &pickedEntity, viewSpaceX, viewSpaceY);
		}
		static_cast<EditorEntityWindow*>(windows[0].get())->PopUp(pickedEntity);
	}
}

void Editor::Render()
{
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

bool Editor::WantCaptureKeyboard()
{
	return imguiIO->WantCaptureKeyboard;
}

bool Editor::WantCaptureMouse()
{
	return imguiIO->WantCaptureMouse;
}

void Editor::MousePick(Entity* entity, float& minDistance, Entity** pickedEntity, const float viewSpaceX, const float viewSpaceY)
{
	if (entity->Renderer_)
	{
		DirectX::XMMATRIX inverseMV = DirectX::XMMatrixInverse(nullptr,DirectX::XMMatrixTranspose(entity->Renderer_->GetWorldViewMatrix())); //MV is column order

		DirectX::XMVECTOR rayOrigin = DirectX::XMVector3TransformCoord(DirectX::XMVectorSet(0.0f,0.0f,0.0f,1.0f), inverseMV);
		DirectX::XMVECTOR rayDirection = DirectX::XMVector3TransformNormal(DirectX::XMVectorSet(viewSpaceX, viewSpaceY, 1.0f, 0.0f), inverseMV);

		float scaleFactor = 1.0f / DirectX::XMVectorGetX(DirectX::XMVector3Length(rayDirection));

		rayDirection = DirectX::XMVector3Normalize(rayDirection);

		float distance = 0.0f;

		if (entity->Renderer_->mesh->GetSubMeshCount() == 1 || entity->Renderer_->mesh->AABB.Intersects(rayOrigin, rayDirection, distance))
		{
			for (const auto& subMesh : entity->Renderer_->mesh->GetSubMeshes())
			{
				if (subMesh.AABB.Intersects(rayOrigin, rayDirection, distance))
				{
					for (unsigned int i = 0; i < subMesh.GetIndexCount(); i += 3)
					{
						DirectX::XMVECTOR p1 = DirectX::XMLoadFloat3(&subMesh.positions[subMesh.indices[i]]);
						DirectX::XMVECTOR p2 = DirectX::XMLoadFloat3(&subMesh.positions[subMesh.indices[i + 1]]);
						DirectX::XMVECTOR p3 = DirectX::XMLoadFloat3(&subMesh.positions[subMesh.indices[i + 2]]);

						if (DirectX::TriangleTests::Intersects(rayOrigin, rayDirection, p1, p2, p3, distance))
						{
							if ((distance * scaleFactor) < minDistance)
							{
								minDistance = distance * scaleFactor;
								*pickedEntity = entity;
							}
						}
					}
				}
			}
		}
	}

	for (auto& child : entity->Children)
		MousePick(child.get(), minDistance, pickedEntity, viewSpaceX, viewSpaceY);
}

void Editor::Camera::Update()
{
	if (!(WantCaptureKeyboard() || WantCaptureMouse()))
	{
		if (Input::GetKeyDown(VK_RBUTTON))
			MainWindow::HideCursor();
		else if (Input::GetKeyUp(VK_RBUTTON))
			MainWindow::ShowCursor();


		if (Input::GetKey(VK_RBUTTON))
		{
			isChanged = true;

			//Reset camera transform
			if (Input::GetKeyDown(VK_SPACE))
			{
				Camera::pitch = 0.0f;
				Camera::yaw = 0.0f;
				Camera::position = { 0.0f,0.0f,0.0f };
			}

			//Updating movement speed
			if (Input::OnMouseWheelUp() && movementSpeed < 99.5f)
				movementSpeed += 10.0f;
			else if (Input::OnMouseWheelDown() && movementSpeed > 10.5f)
				movementSpeed -= 10.0f;

			//Update camera rotation
			yaw += rotationSpeed * (float)Input::GetMouseDeltaX() * (Graphics::GetHorizontalFOV() / (float)MainWindow::GetDisplayResolution().width);
			pitch += rotationSpeed * (float)Input::GetMouseDeltaY() * (Graphics::GetVerticalFOV() / (float)MainWindow::GetDisplayResolution().height);
			if (pitch > 360.0f)
				pitch -= 360.0f;
			if (pitch < -360.0f)
				pitch += 360.0f;

			//Updating rotation matrix
			rotationMatrix = DirectX::XMMatrixRotationRollPitchYaw(DirectX::XMConvertToRadians(pitch), DirectX::XMConvertToRadians(yaw), 0.0f);

			//Updating camera position
			DirectX::XMVECTOR movementDirection = DirectX::XMVectorZero();

			if (Input::GetKey('W'))
				movementDirection = DirectX::XMVectorAdd(movementDirection, rotationMatrix.r[2]);
			else if (Input::GetKey('S'))
				movementDirection = DirectX::XMVectorSubtract(movementDirection, rotationMatrix.r[2]);
			if (Input::GetKey('A'))
				movementDirection = DirectX::XMVectorSubtract(movementDirection, rotationMatrix.r[0]);
			else if (Input::GetKey('D'))
				movementDirection = DirectX::XMVectorAdd(movementDirection, rotationMatrix.r[0]);

			position = DirectX::XMVectorAdd(position, DirectX::XMVectorScale(DirectX::XMVector3Normalize(movementDirection), Time::GetDeltaTime() * movementSpeed));
		}
	}

	if (isChanged)
	{
		isChanged = false;
		UpdateViewMatrix();
	}
}

void Editor::Camera::Focus(const Entity* entity)
{
	if (entity->Renderer_)
	{
		DirectX::BoundingBox worldAABB;
		entity->Renderer_->GetMesh()->AABB.Transform(worldAABB, entity->GetTransform()->GetWorldMatrix());

		float distanceMultiplier = std::max({ worldAABB.Extents.x, worldAABB.Extents.y, worldAABB.Extents.z }) * 2.75f;
		
		position = DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&worldAABB.Center), DirectX::XMVectorScale( rotationMatrix.r[2], -distanceMultiplier));
	
		UpdateViewMatrix();
	}
}

void Editor::Camera::UpdateViewMatrix()
{
	Graphics::viewMatrix = DirectX::XMMatrixTranslationFromVector(DirectX::XMVectorNegate(position)) * DirectX::XMMatrixTranspose(rotationMatrix);
}


#endif