#include "stdafx.h"
#include "Game.h"
#include "Framework/ApplicationImplSdl2.h"

//-----------------------------------------------------------------------------
extern Renderer::DefaultAllocator g_DefaultAllocator;
extern Renderer::DefaultLog	g_DefaultLog;
extern Renderer::DefaultAssert g_DefaultAssert;

Game *game = nullptr;
Game& Game::Get()
{
	return *game;
}
//-----------------------------------------------------------------------------
Game::Game(const GameConfig &config)
{
	game = this;
}
//-----------------------------------------------------------------------------
int Game::Run()
{
	if (m_windows.Init("Game") && init() && gameInit())
	{
		while (!m_windows.ProcessMessages())
		{
			gameUpdate();

			onDrawRequest();
		}
	}

	gameClose();

	// Delete the renderer instance
	if (nullptr != mMainSwapChain)
	{
		mMainSwapChain->releaseReference();
		mMainSwapChain = nullptr;
	}
	mRenderer = nullptr;
	if (nullptr != mRendererInstance)
		mRendererInstance->destroyRenderer();
	
	// Delete the renderer instance
	delete mRendererInstance;
	mRendererInstance = nullptr;
	delete mRendererContext;
	mRendererContext = nullptr;
#ifdef RENDERER_RUNTIME_GRAPHICS_DEBUGGER
	delete static_cast<RendererRuntime::RenderDocGraphicsDebugger*>(mGraphicsDebugger);
#endif
	
	m_windows.Close();
	return 0;
}
//-----------------------------------------------------------------------------
bool Game::init()
{
	m_windows.onDrawRequest = std::bind(&Game::onDrawRequest, this);
	m_windows.onResize = std::bind(&Game::onResize, this);
	m_windows.onToggleFullscreenState = std::bind(&Game::onToggleFullscreenState, this);

	gameInit = std::bind(&GameApp::Init, m_app);
	gameClose = std::bind(&GameApp::Close, m_app);
	gameUpdate = std::bind(&GameApp::Update, m_app);
	gameDoesCompleteOwnDrawing = std::bind(&GameApp::DoesCompleteOwnDrawing, m_app);
	gameDraw = std::bind(&GameApp::Draw, m_app);


	bool loadRendererApiSharedLibrary = false;
	mRendererContext = new Renderer::Context(g_DefaultLog, g_DefaultAssert, g_DefaultAllocator, m_windows.GetNativeWindowHandle());

#if defined(RENDERER_RUNTIME) && defined(RENDERER_RUNTIME_GRAPHICS_DEBUGGER)
	mGraphicsDebugger = new RendererRuntime::RenderDocGraphicsDebugger(*mRendererContext);
#endif
	mRendererInstance = new Renderer::RendererInstance("Direct3D11", *mRendererContext, loadRendererApiSharedLibrary);
	Renderer::IRenderer* renderer = (nullptr != mRendererInstance) ? mRendererInstance->getRenderer() : nullptr;

	if (nullptr != renderer && !renderer->isInitialized())
	{
		// We are not interested in not properly initialized renderer instances, so get rid of the broken thing
		renderer = nullptr;
		delete mRendererInstance;
		mRendererInstance = nullptr;
		delete mRendererContext;
		mRendererContext = nullptr;
	}

#ifndef RENDERER_DEBUG
	// By using
	//   "Renderer::IRenderer::isDebugEnabled()"
	// in here its possible to check whether or not your application is currently running
	// within a known debug/profile tool like e.g. Direct3D PIX (also works directly within VisualStudio
	// 2017 out-of-the-box). In case you want at least try to protect your asset, you might want to stop
	// the execution of your application when a debug/profile tool is used which can e.g. record your data.
	// Please be aware that this will only make it a little bit harder to debug and e.g. while doing so
	// reading out your asset data. Public articles like
	// "PIX: How to circumvent D3DPERF_SetOptions" at
	//   http://www.gamedev.net/blog/1323/entry-2250952-pix-how-to-circumvent-d3dperf-setoptions/
	// describe how to "hack around" this security measurement, so, don't rely on it. Those debug
	// methods work fine when using a Direct3D renderer implementation. OpenGL on the other hand
	// has no Direct3D PIX like functions or extensions, use for instance "gDEBugger" (http://www.gremedy.com/)
	// instead.
	if (nullptr != renderer && renderer->isDebugEnabled())
	{
		// We don't allow debugging in case debugging is disabled
		RENDERER_LOG(renderer->getContext(), CRITICAL, "Debugging with debug/profile tools like e.g. Direct3D PIX is disabled within this application")
			delete renderer;
		renderer = nullptr;
	}
#endif

	mRenderer = renderer;

	if (nullptr != mRenderer)
	{
		// Create render pass using the preferred swap chain texture format
		const Renderer::Capabilities& capabilities = mRenderer->getCapabilities();
		Renderer::IRenderPass* renderPass = mRenderer->createRenderPass(1, &capabilities.preferredSwapChainColorTextureFormat, capabilities.preferredSwapChainDepthStencilTextureFormat);

		// Create a main swap chain instance
		mMainSwapChain = mRenderer->createSwapChain(*renderPass, Renderer::WindowHandle{ m_windows.GetNativeWindowHandle(), nullptr, nullptr }, mRenderer->getContext().isUsingExternalContext());
		RENDERER_SET_RESOURCE_DEBUG_NAME(mMainSwapChain, "Main swap chain")
			mMainSwapChain->addReference();	// Internal renderer reference
	}

	return true;
}
//-----------------------------------------------------------------------------
void Game::onResize()
{
	// Is there a renderer and main swap chain instance?
	if (nullptr != mRenderer && nullptr != mMainSwapChain)
	{
		// Inform the swap chain that the size of the native window was changed
		// -> Required for Direct3D 9, Direct3D 10, Direct3D 11
		// -> Not required for OpenGL and OpenGL ES 3
		mMainSwapChain->resizeBuffers();
	}
}
//-----------------------------------------------------------------------------
void Game::onToggleFullscreenState()
{
	// Is there a renderer and main swap chain instance?
	if (nullptr != mRenderer && nullptr != mMainSwapChain)
	{
		// Toggle the fullscreen state
		mMainSwapChain->setFullscreenState(!mMainSwapChain->getFullscreenState());
	}
}
//-----------------------------------------------------------------------------
void Game::onDrawRequest()
{
	if (gameDoesCompleteOwnDrawing())
	{
		// The example does the drawing completely on its own
		gameDraw();
	}
	// Is there a renderer and main swap chain instance?
	else if (nullptr != mRenderer && nullptr != mMainSwapChain)
	{
		// Begin scene rendering
		// -> Required for Direct3D 9 and Direct3D 12
		// -> Not required for Direct3D 10, Direct3D 11, OpenGL and OpenGL ES 3
		if (mRenderer->beginScene())
		{
			{ // Scene rendering
				// Scoped debug event
				COMMAND_SCOPED_DEBUG_EVENT_FUNCTION(mCommandBuffer)

				// Make the graphics main swap chain to the current render target
				Renderer::Command::SetGraphicsRenderTarget::create(mCommandBuffer, mMainSwapChain);
				{ // Since Direct3D 12 is command list based, the viewport and scissor rectangle
					// must be set in every draw call to work with all supported renderer APIs
					// Get the window size
					uint32_t width = 1;
					uint32_t height = 1;
					mMainSwapChain->getWidthAndHeight(width, height);

					// Set the graphics viewport and scissor rectangle
					Renderer::Command::SetGraphicsViewportAndScissorRectangle::create(mCommandBuffer, 0, 0, width, height);
				}

				// Submit command buffer to the renderer backend
				mCommandBuffer.submitToRendererAndClear(*mRenderer);

				// Call the draw method
				gameDraw();
			}

			// Submit command buffer to the renderer backend
			mCommandBuffer.submitToRendererAndClear(*mRenderer);

			// End scene rendering
			// -> Required for Direct3D 9 and Direct3D 12
			// -> Not required for Direct3D 10, Direct3D 11, OpenGL and OpenGL ES 3
			mRenderer->endScene();
		}

		// Present the content of the current back buffer
		mMainSwapChain->present();
	}
}
//-----------------------------------------------------------------------------