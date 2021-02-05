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
	static void UpdateEntity(std::unique_ptr<Entity>& entity);
	static void RenderEntity(std::unique_ptr<Entity>& entity, DirectX::XMMATRIX worldMatrix);
private:
	static void ShutDown();
};

