#include "MainWindow.h"
#include "EngineAssert.h"
#include "Input.h"
#include "EngineException.h"
#include "Log.h"
#include "Scene.h"
#include "Mesh.h"
#include "Model.h"
#include "Resources.h"

#ifdef EDITOR
#define EDITOR_CAPTURE_INPUT \
if(Editor::WantCaptureKeyboard() || (Editor::WantCaptureMouse() && !MainWindow::isCursorHidden))\
	break;
#else
#define EDITOR_CAPTURE_INPUT
#endif

#ifdef EDITOR
std::unique_ptr<Editor> MainWindow::editor = nullptr;
#endif



const std::wstring MainWindow::MainWindowClassName = L"MainWindowClass";
bool MainWindow::isCreated = false;
bool MainWindow::isFullScreen = false;
Resolution MainWindow::displayResolution;
HWND MainWindow::hWnd = nullptr;
bool MainWindow::isCursorHidden = false;


MainWindow::MainWindow(std::wstring name,int clientWidth,int clientHeight)
	:MainWindowName(name)
{
	ENGINEASSERT(!isCreated, "Main Window can only be created once!!!");

	//Creating window
	CreateMainWindowClass();
	CreateMainWindow(clientWidth, clientHeight);

	Graphics::Init(hWnd);
	Mesh::Init();
	Resources::Init();
	Scene::Init();

	//Initially setting cursor position to middle of the client region.
	SetCursorToMiddle();

	ShowWindow(hWnd, SW_SHOW);
	CHECK_WIN32_ERROR()

	//Initializing editor if we are in editor mode.
#ifdef EDITOR
	editor = std::make_unique<Editor>(hWnd,Graphics::pDeviceContext.Get(),Graphics::pDevice.Get());
#endif

	isCreated = true;
	ENGINE_LOG(ENGINE_INFO, "Main Window Ready!");
}

HWND MainWindow::GetHWND()
{
	return hWnd;
}

Resolution MainWindow::GetDisplayResolution()
{
	return displayResolution;
}

bool MainWindow::isMinimized()
{
	return !IsIconic(hWnd);
}

void MainWindow::SetFullScreen(bool isFullScreen)
{
	MainWindow::isFullScreen = isFullScreen;
	Graphics::pSwapChain->SetFullscreenState(isFullScreen, nullptr);
	if (isFullScreen)
	{
		
	}
}

void MainWindow::HideCursor()
{
	Input::DeltaX = 0;
	Input::DeltaY = 0;
	SetCursorToMiddle();
	while (::ShowCursor(FALSE) > -1);
	isCursorHidden = true;
}

void MainWindow::ShowCursor()
{
	while (::ShowCursor(TRUE) < 0);
	isCursorHidden = false;
}

void MainWindow::ClipCursor(bool clip)
{
	if (clip)
	{
		RECT clientRect;
		GetClientRect(hWnd, &clientRect);	
		MapWindowPoints(hWnd, nullptr, reinterpret_cast<POINT*>(&clientRect), 2);
		::ClipCursor(&clientRect);
	}
	else
		::ClipCursor(NULL);
}

void MainWindow::SetCursorToMiddle()
{
	POINT clientMidPoint = { displayResolution.width / 2, displayResolution.height / 2 };
	Input::MousePosition.first = clientMidPoint.x;
	Input::MousePosition.second = clientMidPoint.y;
	ClientToScreen(hWnd, &clientMidPoint);
	SetCursorPos(clientMidPoint.x, clientMidPoint.y);
}

MainWindow::~MainWindow()
{
	DestroyWindow(hWnd);
	UnregisterClass(MainWindowClassName.c_str(),GetModuleHandle(nullptr));
}

LRESULT MainWindow::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
#ifdef EDITOR
	if (Editor::EditorInputHandler(hWnd, uMsg, wParam, lParam))
		return true;
#endif

	switch (uMsg)
	{
	case WM_SIZE:
		displayResolution.width = LOWORD(lParam);
		displayResolution.height = HIWORD(lParam);
		return 0;
	case WM_CLOSE:
		PostQuitMessage(0);
		return 0;
	case WM_SETFOCUS:
		Input::IsMainWindowFocused = true;
		break;
	case WM_KILLFOCUS:
		ReleaseCapture();
		ShowCursor();
		ClipCursor(false);
		Input::IsMainWindowFocused = false;
		break;

		//Keyboard Events
	case WM_SYSKEYDOWN:
	case WM_KEYDOWN:
		//If ImGui wants to capture keyboard we let it.
		EDITOR_CAPTURE_INPUT
		//else we process input
		if (!(lParam & 0x40000000))
			Input::SetKeyState(static_cast<unsigned char>(wParam), Input::KeyState::Press);
		return 0;
	case WM_SYSKEYUP:
	case WM_KEYUP:
		//If ImGui wants to capture keyboard we let it.
		EDITOR_CAPTURE_INPUT
		//else we process input
		Input::SetKeyState(static_cast<unsigned char>(wParam), Input::KeyState::Release);
		return 0;
	case WM_SYSCHAR:
	case WM_SYSDEADCHAR:
		//If ImGui wants to capture keyboard we let it.
		EDITOR_CAPTURE_INPUT
		return 0;

		//Mouse Events
	case WM_MOUSEMOVE:
	{
		//If ImGui wants to capture mouse we let it.
		EDITOR_CAPTURE_INPUT

		//else we process input
		const POINTS mousePos = MAKEPOINTS(lParam);
		Input::DeltaX = mousePos.x - Input::MousePosition.first;
		Input::DeltaY = mousePos.y - Input::MousePosition.second;
		Input::MousePosition = { mousePos.x,mousePos.y };

		if (isCursorHidden)
			SetCursorToMiddle();

		return 0;
	}
	case WM_LBUTTONDOWN:
		//If ImGui wants to capture mouse we let it.
		EDITOR_CAPTURE_INPUT
		//else we process input
		Input::SetKeyState(static_cast<unsigned char>(VK_LBUTTON), Input::KeyState::Press);
		SetCapture(hWnd);
		return 0;
	case WM_RBUTTONDOWN:
		//If ImGui wants to capture mouse we let it.
		EDITOR_CAPTURE_INPUT
		//else we process input
		Input::SetKeyState(static_cast<unsigned char>(VK_RBUTTON), Input::KeyState::Press);
		SetCapture(hWnd);
		return 0;
	case WM_LBUTTONUP:
		//If ImGui wants to capture mouse we let it.
		EDITOR_CAPTURE_INPUT
		//else we process input
		Input::SetKeyState(static_cast<unsigned char>(VK_LBUTTON), Input::KeyState::Release);
		ReleaseCapture();
		return 0;
	case WM_RBUTTONUP:
		//If ImGui wants to capture mouse we let it.
		EDITOR_CAPTURE_INPUT
		//else we process input
		Input::SetKeyState(static_cast<unsigned char>(VK_RBUTTON), Input::KeyState::Release);
		ReleaseCapture();
		return 0;
	case WM_MOUSEWHEEL:
		//If ImGui wants to capture mouse we let it.
		EDITOR_CAPTURE_INPUT
		//else we process input
		Input::WheelDelta += GET_WHEEL_DELTA_WPARAM(wParam);
		return 0;
	default:
		break;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

void MainWindow::CreateMainWindow(int clientWidth, int clientHeigth)
{
	DWORD dwStyle = WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;
	Resolution adjustedRes = GetAdjustedResolution(clientWidth, clientHeigth, dwStyle);
	CHECK_WIN32_ERROR()

	hWnd = CreateWindowEx(0,
		MainWindowClassName.c_str(),
		MainWindowName.c_str(),
		dwStyle,
		CW_USEDEFAULT, CW_USEDEFAULT,
		adjustedRes.width, adjustedRes.height,
		nullptr,
		nullptr,
		GetModuleHandle(nullptr),
		nullptr);
	CHECK_WIN32_ERROR()

	RECT clientRect;
	GetClientRect(hWnd, &clientRect);
	displayResolution = {clientRect.right - clientRect.left, clientRect.bottom - clientRect.top};
}

void MainWindow::CreateMainWindowClass()
{
	WNDCLASSEX MainWindowClass = { 0 };
	MainWindowClass.cbSize = sizeof(WNDCLASSEX);
	MainWindowClass.style = CS_OWNDC;
	MainWindowClass.lpfnWndProc = WndProc;
	MainWindowClass.cbClsExtra = 0;
	MainWindowClass.cbWndExtra = 0;
	MainWindowClass.hInstance = GetModuleHandle(NULL);
	MainWindowClass.hIcon = NULL;
	MainWindowClass.hCursor = NULL;
	MainWindowClass.hbrBackground = NULL;
	MainWindowClass.lpszMenuName = NULL;
	MainWindowClass.lpszClassName = MainWindowClassName.c_str();
	MainWindowClass.hIconSm = NULL;
	RegisterClassEx(&MainWindowClass);
	CHECK_WIN32_ERROR()
}

Resolution MainWindow::GetAdjustedResolution(int clientWidth, int clientHeight,DWORD dwStyle)
{	
	RECT windowRect = { 0,0,clientWidth,clientHeight};
	AdjustWindowRect(&windowRect,dwStyle,TRUE);
	CHECK_WIN32_ERROR()
	return { windowRect.right - windowRect.left , windowRect.bottom - windowRect.top };
}
