#pragma once
#include "Win.h"
#include "Graphics.h"
#include "Editor.h"
#include <string>
#include <memory>

class MainWindow
{
private:
	MainWindow() = delete;
	MainWindow(const MainWindow&) = delete;
	MainWindow(MainWindow&&) = delete;
	MainWindow& operator=(const MainWindow&) = delete;
	MainWindow& operator=(MainWindow&&) = delete;
public:
	MainWindow(std::wstring name, int clientWidth, int clientHeight);
	~MainWindow();

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
	void CreateMainWindow(int clientWidth,int clientHeight);
	static void CreateMainWindowClass();
	static Resolution GetAdjustedResolution(int clientWidth, int clientHeight,DWORD dwStyle);
private:
	//MainWindow Class
	static const std::wstring MainWindowClassName;
	static bool isCreated;
	//MainWindow
	const std::wstring MainWindowName;
	static HWND hWnd;
	static Resolution displayResolution;
	static bool isFullScreen;
	static bool isCursorHidden;

	//Initialization and destruction purposes
#ifdef EDITOR
	static std::unique_ptr<Editor> editor;
#endif
};
