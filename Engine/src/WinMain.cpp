#include "Win.h"
#include "Engine.h"

int WINAPI WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR     lpCmdLine,
    int       nShowCmd
)
{
    Engine::Init(L"Game", 1920, 1080);
    return Engine::Start();
}