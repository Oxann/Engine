#pragma once
#include "Win.h"
#include <string>
#include "Graphics.h"

class MainWindow
{
public:
	static void Init(std::wstring name, int clientWidth, int clientHeight);
	static void ShutDown();

	static HWND GetHWND();
	static Resolution GetDisplayResolution();
	static bool isMinimized();
	static void SetFullScreen(bool isFullScreen);
	static void HideCursor();
	static void ShowCursor();
	static void ClipCursor(bool clip);
	static void SetCursorToMiddle();
private:
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static void CreateMainWindow(int clientWidth,int clientHeight);
	static void CreateMainWindowClass();
	static Resolution GetAdjustedResolution(int clientWidth, int clientHeight,DWORD dwStyle);

private:
	//MainWindow Class
	inline static const std::wstring MainWindowClassName = L"MainWindowClass";
	inline static bool isCreated = false;

	//MainWindow
	inline static std::wstring MainWindowName;
	inline static HWND hWnd = nullptr;
	inline static Resolution displayResolution;
	inline static bool isFullScreen = false;
	inline static bool isCursorHidden = false;
};
