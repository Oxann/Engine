#include "RendererManager.h"
#include "Editor.h"
#include "Graphics.h"
#include "MainWindow.h"

void RendererManager::Update()
{
	Graphics::pDeviceContext->ClearRenderTargetView(Graphics::pView.Get(), Graphics::clearColor);
	Graphics::pDeviceContext->ClearDepthStencilView(Graphics::pDepthStencil.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0u);

	for (const auto& renderer : renderers)
	{
		renderer->Update();
	}

	//Render queues
	renderQueueOpaque.Render();
	renderQueueTransparent.Render();

#ifdef EDITOR
	renderQueueWireframe.Render();
	renderQueueOutline.Render();
	Editor::Render();
#endif

	//In debug mode present is not working when window is minimized.
#ifndef NDEBUG
	if (MainWindow::isMinimized())
	{
		CHECK_DX_ERROR(Graphics::pSwapChain->Present(Graphics::isVSyncEnabled, 0u));
	}
#else
	Graphics::pSwapChain->Present(Graphics::isVSyncEnabled, 0u);
#endif
}
