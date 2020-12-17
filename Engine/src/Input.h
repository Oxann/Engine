#pragma once
#include "Win.h"
#include <array>
#include <utility>

//TODO: keycodes
enum class Keycode
{

};

class Input
{
private:
private:
	enum class KeyState : unsigned char
	{
		None = 0u,
		Press = 1u,
		Release = 2u
	};
public:
	Input() = delete;
	~Input() = delete;
	Input(const Input&) = delete;
	Input(Input&&) = delete;
	Input& operator= (const Input&) = delete;
	Input& operator= (Input&&) = delete;
	static bool GetKey(unsigned char Keycode) noexcept;
	static bool GetKeyDown(unsigned char Keycode) noexcept;
	static bool GetKeyUp(unsigned char Keycode) noexcept;
	static bool OnMouseWheelUp() noexcept;
	static bool OnMouseWheelDown() noexcept;

	//Relative to client space
	static const std::pair<short, short>& GetMousePosition() noexcept;
	static short GetMouseX() noexcept; 
	static short GetMouseY() noexcept;	
	static short GetMouseDeltaX() noexcept;
	static short GetMouseDeltaY() noexcept;
	static void SetMousePos(short x, short y);
private:
	static void SetKeyState(unsigned char KeyCode, KeyState ks) noexcept;
public:
	static void FlushKeys() noexcept;
private:
	friend class MainWindow;
	static constexpr unsigned int KeyCount = 256u;
	static std::array<KeyState,KeyCount> keys;
	//Keyboard
	static bool IsMainWindowFocused;
	//Mouse
	static bool IsMouseInWindow;
	static std::pair<short, short> MousePosition;
	static int WheelDelta;
	static short DeltaX;
	static short DeltaY;
};