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

            //--------FRAME--------//
            Graphics::BeginFrame();
            Scene::GetActiveScene()->Update();

#ifdef EDITOR
            if (Input::GetKeyDown(VK_TAB))
                Editor::isActive = !Editor::isActive;

            Editor::Update();
#endif
            Graphics::EndFrame();
            /////////////////////////

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

void Engine::ShutDown()
{
    MainWindow::ShutDown();
}
