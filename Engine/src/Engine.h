#pragma once

#include <string>

class Engine
{
public:
	static void Init(std::wstring mainWindowName, unsigned int mainWindowWidth, unsigned int mainWindowHeight);
	static int Start();

private:
	static void ShutDown();
};
