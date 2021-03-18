#pragma once

#include <string>
#include <memory>

#include "Entity.h"


class Engine
{
public:
	static void Init(std::wstring mainWindowName, unsigned int mainWindowWidth, unsigned int mainWindowHeight);
	static int Start();
private:
	static void Update();
private:
	static void ShutDown();
};

