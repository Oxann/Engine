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

class Entity;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

class Editor
{
public:
	class Camera
	{
		friend Editor;
	private:
		static void Update();
	public:
		static void Focus(const Entity* entity);
		static void UpdateViewMatrix();
	public:
		inline static DirectX::XMVECTOR position = DirectX::XMVectorZero();
		inline static DirectX::XMMATRIX rotationMatrix = DirectX::XMMatrixIdentity();
		inline static float movementSpeed = 10.0f;
		inline static float rotationSpeed = 1.5f;

		//Angles are in degrees.
		inline static float pitch = 0.0f;
		inline static float yaw = 0.0f;

		inline static bool isChanged = true;
	};
public:
	static void Init(HWND hWnd, ID3D11DeviceContext* pDeviceContext, ID3D11Device* pDevice);
	static void ShutDown();
	static LRESULT EditorInputHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	static void Update();
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
	static void MousePick(Entity* entity, float& minDistance, Entity** pickedEntity, const float viewSpaceX, const float viewSpaceY);
public:
	inline static bool isActive = true;
	inline static ImGuiIO* imguiIO = nullptr;
	inline static std::vector<std::unique_ptr<EditorWindowBase>> windows;
	inline static bool isWireframe = false;
};


#endif