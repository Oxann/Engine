#include "Time.h"
#include "MainWindow.h"
#include "Input.h"
#include "EngineException.h"
#include "Scene.h"


int WINAPI WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR     lpCmdLine,
    int       nShowCmd
)
{
    try
    {
        MainWindow mw(L"Game", 1920, 1080);
        while (true)
        {
            auto FrameBegin = std::chrono::steady_clock::now();
            Input::FlushKeys();
            MSG msg;
            while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
            {
                if (msg.message == WM_QUIT)
                {
                    ENGINE_LOG(ENGINE_INFO, "Program terminated successfully.");
                    while (Engine::Log::MessageCount() > 0); // Waiting for all log messages to be pumped.
                    return static_cast<int>(msg.wParam);
                }
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }

            //--------FRAME--------//
            Graphics::BeginFrame();
            Scene::GetActiveScene()->Update();
            #ifdef EDITOR
            Editor::Update();
            #endif
            Graphics::EndFrame();
            /////////////////////////

            //Time 
            auto FrameEnd = std::chrono::steady_clock::now();
            Time::DeltaTime = std::chrono::duration<float>(FrameEnd - FrameBegin).count();
            Time::AppTime += Time::GetDeltaTime();
        }
    }
    catch (const EngineException & e)
    {
        while (Engine::Log::MessageCount() > 0); // Waiting for all log messages to be pumped.
        e.PopUp();
    }
    catch (const std::exception & e)
    {
        while (Engine::Log::MessageCount() > 0); // Waiting for all log messages to be pumped.
        MessageBoxA(nullptr, e.what(), "Standard Exception Caught!", MB_OK | MB_ICONSTOP);
    }
    catch (...)
    {
        while (Engine::Log::MessageCount() > 0); // Waiting for all log messages to be pumped.
        MessageBoxA(nullptr, "Engine Exception should be used.", "Unknown Exception Caught!", MB_OK | MB_ICONSTOP);
    }
    return 0;
}