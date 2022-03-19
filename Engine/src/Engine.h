#pragma once

#include <string>
#include <memory>

#include "Entity.h"
#include "ThreadPool.h"


class Engine
{
public:
	static void Init(std::wstring mainWindowName, unsigned int mainWindowWidth, unsigned int mainWindowHeight);
	static int Start();
private:
	static void Update();
private:
	static void ShutDown();
public:
	static inline ThreadPool threadPool;
};

