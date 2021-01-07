#include "Input.h"
#include "MainWindow.h"


std::array<Input::KeyState,Input::KeyCount> Input::keys;
bool Input::IsMainWindowFocused = false;
std::pair<short, short> Input::MousePosition;
int Input::WheelDelta = 0;
short Input::DeltaX = 0;
short Input::DeltaY = 0;

void Input::FlushKeys() noexcept
{
	//Keyboard
	keys.fill(KeyState::None);

	//Mouse
	if (WheelDelta >= WHEEL_DELTA) 
		WheelDelta -= WHEEL_DELTA;
	else if (WheelDelta <= -WHEEL_DELTA) 
		WheelDelta += WHEEL_DELTA;
	DeltaX = 0;
	DeltaY = 0;
}

bool Input::GetKey(unsigned char Keycode) noexcept
{
	return IsMainWindowFocused && (GetKeyState(Keycode) & 0x8000);
}

bool Input::GetKeyDown(unsigned char Keycode) noexcept
{
	return keys[Keycode] == KeyState::Press;
}

bool Input::GetKeyUp(unsigned char Keycode) noexcept
{
	return keys[Keycode] == KeyState::Release;
}

bool Input::OnMouseWheelUp() noexcept
{
	return WheelDelta >= WHEEL_DELTA;
}

bool Input::OnMouseWheelDown() noexcept
{
	return WheelDelta <= -WHEEL_DELTA;
}

const std::pair<short, short>& Input::GetMousePosition() noexcept
{
	return MousePosition;
}

short Input::GetMouseX() noexcept
{
	return MousePosition.first;
}

short Input::GetMouseY() noexcept
{
	return MousePosition.second;
}

short Input::GetMouseDeltaX() noexcept
{
	return DeltaX;
}

short Input::GetMouseDeltaY() noexcept
{
	return DeltaY;
}

void Input::SetMousePos(short x, short y)
{
	DeltaX = x - MousePosition.first;
	DeltaY = y - MousePosition.second;
	MousePosition.first = x;
	MousePosition.second = y;
	
	POINT newPos = { x,y };
	ClientToScreen(MainWindow::GetHWND(), &newPos);
	SetCursorPos(newPos.x,newPos.y);
}

void Input::SetKeyState(unsigned char KeyCode, KeyState ks) noexcept
{
	keys[KeyCode] = ks;
}