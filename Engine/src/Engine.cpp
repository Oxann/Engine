#include "Engine.h"
#include "MainWindow.h"
#include "Graphics.h"
#include "Mesh.h"
#include "Resources.h"
#include "Scene.h"
#include "Editor.h"
#include "Input.h"
#include "Log.h"
#include "EngineAssert.h"
#include "EngineException.h"
#include "Time.h"

#include <set>

void Engine::Init(std::wstring mainWindowName, unsigned int mainWindowWidth, unsigned int mainWindowHeight)
{
    auto initStart = std::chrono::steady_clock::now();

	MainWindow::Init(mainWindowName, mainWindowWidth, mainWindowHeight);

	Graphics::Init(MainWindow::GetHWND());

#ifdef EDITOR
	Editor::Init(MainWindow::GetHWND(), Graphics::pDeviceContext.Get(), Graphics::pDevice.Get());
#endif

	Mesh::Init();

	Resources::Init();

	Scene::Init();

    auto initEnd = std::chrono::steady_clock::now();
    Time::AppTime += std::chrono::duration<float>(initStart - initEnd).count();
}


int Engine::Start()
{
    try
    {
        while (true)
        {
            auto FrameBegin = std::chrono::steady_clock::now();
            Input::FlushKeys();
            MSG msg;
            while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
            {
                if (msg.message == WM_QUIT)
                {
                    if (msg.wParam == 0)
                        ENGINE_LOG(ENGINE_INFO, "Program terminated successfully.")

                        while (Log::MessageCount() > 0); // Waiting for all log messages to be pumped.

                    return static_cast<int>(msg.wParam);
                }
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }

            Update();

            //Time 
            auto FrameEnd = std::chrono::steady_clock::now();
            Time::DeltaTime = std::chrono::duration<float>(FrameEnd - FrameBegin).count();
            Time::AppTime += Time::DeltaTime;
        }
    }
    catch (const EngineException& e)
    {
        e.PopUp();
    }
    catch (const std::exception& e)
    {
        MessageBoxA(nullptr, e.what(), "Standard Exception Caught!", MB_OK | MB_ICONSTOP);
    }
    catch (...)
    {
        MessageBoxA(nullptr, "Engine Exception should be used.", "Unknown Exception Caught!", MB_OK | MB_ICONSTOP);
    }

    while (Log::MessageCount() > 0); // Waiting for all log messages to be pumped.

    Engine::ShutDown();

    return -1;
}


struct TransparentObject
{
	std::unique_ptr<Renderer>& renderer;
	unsigned int index;
	float distanceSq;

	bool operator<(const TransparentObject& rhs) const
	{
		return distanceSq < rhs.distanceSq;
	}

	void Render() const
	{
		renderer->GetMaterials()[index]->Bind(&renderer->GetMesh()->GetSubMeshes()[index], renderer.get());
		Graphics::pDeviceContext->DrawIndexed(renderer->GetMesh()->GetSubMeshes()[index].GetIndexCount(), 0u, 0u);
	}
};

static std::multiset<TransparentObject> transparents;


void Engine::Update()
{
	//Updating Components
	for (auto& entity : Scene::ActiveScene->Entities)
	{
		UpdateEntity(entity);
	}

	//Render
	Graphics::BeginFrame();

	Graphics::viewProjectionMatrix = DirectX::XMMatrixMultiply(Graphics::viewMatrix, Graphics::projectionMatrix);

	//Opaque
	Graphics::pDeviceContext->OMSetBlendState(Graphics::BS_Opaque.Get(), NULL, 0xffffffff);
	for (auto& entity : Scene::ActiveScene->Entities)
	{
		static const DirectX::XMMATRIX identity = DirectX::XMMatrixIdentity();
		RenderEntity(entity,identity);
	}

	//Transparent
	Graphics::pDeviceContext->OMSetBlendState(Graphics::BS_Transparent.Get(), NULL, 0xffffffff);
	for (auto& transparent : transparents)
	{
		transparent.Render();
	}
	transparents.clear();

#ifdef EDITOR
	if (Input::GetKeyDown(VK_TAB))
		Editor::isActive = !Editor::isActive;

	Editor::Update();
#endif

	Graphics::EndFrame();
}

void Engine::UpdateEntity(std::unique_ptr<Entity>& entity)
{
	for (auto& component : entity->Components)
	{
		switch (component.second->state)
		{
		[[unlikely]] case Component::State::Start:
			component.second->Start();
			component.second->state = Component::State::Update;
			break;
		[[likely]] case Component::State::Update:
			component.second->Update();
			break;
		default:
			break;
		}
	}

	for (int i = 0; i < entity->GetChildrenCount(); i++)
		UpdateEntity(entity->Children[i]);
}

void Engine::RenderEntity(std::unique_ptr<Entity>& entity, DirectX::XMMATRIX worldMatrix)
{
	worldMatrix = entity->GetTransform()->GetLocalMatrix() * worldMatrix;
	
	if (entity->Renderer_)
	{
		entity->Renderer_->MV_Matrix = worldMatrix * Graphics::GetViewMatrix();

		float distanceSq = DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(entity->Renderer_->MV_Matrix.r[3]));

		entity->Renderer_->MV_Matrix = DirectX::XMMatrixTranspose(entity->Renderer_->MV_Matrix);
		entity->Renderer_->normalMatrix = DirectX::XMMatrixTranspose(DirectX::XMMatrixInverse(nullptr, entity->Renderer_->MV_Matrix));
		entity->Renderer_->MVP_Matrix = DirectX::XMMatrixTranspose(Graphics::GetProjectionMatrix()) * entity->Renderer_->MV_Matrix;

		if (auto mesh = entity->Renderer_->GetMesh())
		{
			auto& materials = entity->Renderer_->GetMaterials();

			for (unsigned int i = 0; i < mesh->GetSubMeshCount(); i++)
			{
#ifdef EDITOR
				if (Editor::isWireframe)
				{
					static Material* wireframeMaterial = Resources::FindMaterial("$Default\\wireframeMaterial");
					wireframeMaterial->Bind(&mesh->GetSubMeshes()[i], entity->Renderer_.get());
					Graphics::pDeviceContext->DrawIndexed(mesh->GetSubMeshes()[i].GetIndexCount(), 0u, 0u);
					continue;
				}
#endif
				if (materials[i]->mode == Material::Mode::Opaque)
				{
					materials[i]->Bind(&mesh->GetSubMeshes()[i], entity->Renderer_.get());
					Graphics::pDeviceContext->DrawIndexed(mesh->GetSubMeshes()[i].GetIndexCount(), 0u, 0u);
				}
				else
				{
					transparents.insert({entity->Renderer_, i, distanceSq});
				}
			}
		}
	}

	for (int i = 0; i < entity->GetChildrenCount(); i++)
		RenderEntity(entity->Children[i], worldMatrix);
}

void Engine::ShutDown()
{
    MainWindow::ShutDown();
}