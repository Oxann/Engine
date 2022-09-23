#pragma once

#ifdef EDITOR

#include "Win.h"
#include "../thirdparty/imgui/imgui.h"
#include "../thirdparty/imgui/imgui_impl_dx11.h"
#include "../thirdparty/imgui/imgui_impl_win32.h"
#include "EditorWindowBase.h"
#include <DirectXMath.h>
#include <string>
#include <vector>
#include <memory>
#include <type_traits>
#include "Camera.h"

class Entity;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

class Editor
{
public:
	class EditorCamera
	{
		friend Editor;
	private:
		static void Update();
	public:
		static void Focus(const Entity* entity);
		static void UpdateMatrices();
		static float GetHorizontalFOV() { return horizontalFOV; }
		static float GetVerticalFOV() { return verticalFOV; }
		static void SetProjection(Camera::ProjectionType type, float aspectRatio, float height, float near_z, float far_z);
	public:
		inline static DirectX::XMVECTOR position = DirectX::XMVectorZero();
		inline static DirectX::XMMATRIX rotationMatrix = DirectX::XMMatrixIdentity();
		inline static float movementSpeed = 10.0f;
		inline static float rotationSpeed = 1.5f;

		//Angles are in degrees.
		inline static float pitch = 0.0f;
		inline static float yaw = 0.0f;

		inline static bool isChanged = true;
	
	private:
		inline static float horizontalFOV;
		inline static float verticalFOV;
		inline static DirectX::XMMATRIX projectionMatrix;
		inline static Camera::ProjectionType projectionType;
	};
public:
	static void Init(HWND hWnd, ID3D11DeviceContext* pDeviceContext, ID3D11Device* pDevice);
	static void ShutDown();
	static LRESULT EditorInputHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	static void Update();
	static void Render();
	static bool WantCaptureKeyboard();
	static bool WantCaptureMouse();

	template<class WindowType>
	inline static WindowType* GetWindow()
	{
		for (const auto& window : windows)
		{
			if (dynamic_cast<WindowType*>(window.get()) != nullptr)
				return (WindowType*)window.get();
		}
		return nullptr;
	}
private:
	//static void MousePick(Entity* entity, float& minDistance, Entity** pickedEntity, const float viewSpaceX, const float viewSpaceY);
public:
	inline static bool isActive = true;
	inline static ImGuiIO* imguiIO = nullptr;
	inline static std::vector<std::unique_ptr<EditorWindowBase>> windows;
	inline static bool isWireframeEnabled = false;
};


#endif