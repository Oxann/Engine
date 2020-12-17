#include "EngineException.h"

const char* EngineException::what() const noexcept
{
	return whatBuffer.c_str();
}

void EngineException::PopUp() const
{
	MessageBoxA(nullptr, what(), "Engine Exception Caught!", MB_OK | MB_ICONSTOP);
}

EngineException::EngineException(std::string what, std::string file, int line, std::string function) noexcept
{
	whatBuffer = "File: " + file + "\n" +
		"Function " + function + "\n" +
		"Line: " + std::to_string(line) + "\n"
		"Description: " + what;
}

