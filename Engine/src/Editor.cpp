#include "Editor.h"
#include "Input.h"
#include "Time.h"
#include "MainWindow.h"

#include "EditorEntityWindow.h"
#include "EditorSceneHierarchyWindow.h"
#include "EditorResourceSelectionWindow.h"
#include "EditorMaterialEditWindow.h"



Editor::Editor(HWND hWnd, ID3D11DeviceContext* pDeviceContext, ID3D11Device* pDevice)
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

	//Windows start calls
	for (const auto& window : windows)
		window->Start();
}

LRESULT Editor::EditorInputHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);
}

Editor::~Editor()
{
	ImGui_ImplWin32_Shutdown();
	ImGui_ImplDX11_Shutdown();
	ImGui::DestroyContext();
}

void Editor::Update()
{
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	
	//Windows update calls	
	for (auto& window : windows)
	{
		if (window->isActive)
		{
			if(ImGui::Begin(window->name.c_str(),&window->isActive))
				window->Update();
			
			ImGui::End();
		}			
	}
	
	Camera::Update();

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

void Editor::Camera::Update()
{
	static DirectX::XMMATRIX rotationMatrix = DirectX::XMMatrixIdentity();

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

			position = DirectX::XMVectorAdd(position, DirectX::XMVectorScale(DirectX::XMVector3Normalize(movementDirection), Time::DeltaTime * movementSpeed));
		}
	}


	if (isChanged)
	{
		isChanged = false;

		//Flipping world up vector according to camera local up vector.
		//If the the y component of the vector is greater than zero world up vector must be (0.0f,1.0f,0.0f,1.0f) else it must be (0.0f,-1.0f,0.0f,1.0f)
		//This enables 360 degrees pitch rotation to camera.
		static DirectX::XMVECTOR worldUp;

		float local_Y = DirectX::XMVectorGetY(rotationMatrix.r[1]);

		if (local_Y >= 0.0f)
			worldUp = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f);
		else
			worldUp = DirectX::XMVectorSet(0.0f, -1.0f, 0.0f, 1.0f);

		//Updating view matrix
		TransformMatrix = DirectX::XMMatrixLookToLH(position, rotationMatrix.r[2], worldUp);
	}
}
