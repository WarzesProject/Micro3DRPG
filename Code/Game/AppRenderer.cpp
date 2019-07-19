#include "stdafx.h"
#include "AppRenderer.h"
#include "BaseExample.h"
#ifdef SDL2_FOUND
#include "Framework/ApplicationImplSdl2.h"
#endif

extern Renderer::DefaultAllocator g_DefaultAllocator;
extern Renderer::DefaultLog	g_DefaultLog;
extern Renderer::DefaultAssert g_DefaultAssert;

AppRenderer::AppRenderer(const char* rendererName, BaseExample& exampleBase) 
	: mExampleBase(exampleBase)
{
	mApplicationImpl = new AppWindowSDL(*this, rendererName);
	mExampleBase.setApplicationFrontend(this);
	strncpy(mRendererName, rendererName, 32);
}

AppRenderer::~AppRenderer()
{
	delete mApplicationImpl;
}

int AppRenderer::run()
{
	mApplicationImpl->onInitialization();
	if (onInitialization())
	{
		while (!mExit && !mApplicationImpl->processMessages())
		{
			onUpdate();
			redraw();
		}

		onDeinitialization();
		mApplicationImpl->onDeinitialization();
	}
	return 0;
}

bool AppRenderer::onInitialization()
{
	createRenderer();
	mExampleBase.onInitialization();
	return true;
}

void AppRenderer::onDeinitialization()
{
	mExampleBase.onDeinitialization();
	destroyRenderer();
	mApplicationImpl->onDeinitialization();
}

void AppRenderer::onUpdate()
{
	mExampleBase.onUpdate();
}

void AppRenderer::onResize()
{
	if (nullptr != mRenderer && nullptr != mMainSwapChain)
		mMainSwapChain->resizeBuffers();
}

void AppRenderer::onToggleFullscreenState()
{
	if (nullptr != mRenderer && nullptr != mMainSwapChain)
		mMainSwapChain->setFullscreenState(!mMainSwapChain->getFullscreenState());
}

void AppRenderer::onDrawRequest()
{
	if (mExampleBase.doesCompleteOwnDrawing())
		mExampleBase.onDraw();
	else if (nullptr != mRenderer && nullptr != mMainSwapChain)
	{
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
				mExampleBase.onDraw();
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

void AppRenderer::onEscapeKey()
{
	exit();
}

void AppRenderer::createRenderer()
{
	assert(nullptr == mRenderer);

	mRenderer = createRendererInstance(mRendererName);
	if (nullptr != mRenderer)
	{
		// Create render pass using the preferred swap chain texture format
		const Renderer::Capabilities& capabilities = mRenderer->getCapabilities();
		Renderer::IRenderPass* renderPass = mRenderer->createRenderPass(1, &capabilities.preferredSwapChainColorTextureFormat, capabilities.preferredSwapChainDepthStencilTextureFormat);

		// Create a main swap chain instance
		mMainSwapChain = mRenderer->createSwapChain(*renderPass, Renderer::WindowHandle{ getNativeWindowHandle(), nullptr, nullptr }, mRenderer->getContext().isUsingExternalContext());
		RENDERER_SET_RESOURCE_DEBUG_NAME(mMainSwapChain, "Main swap chain")
			mMainSwapChain->addReference();	// Internal renderer reference
	}
}

void AppRenderer::destroyRenderer()
{
	if (nullptr != mMainSwapChain)
	{
		mMainSwapChain->releaseReference();
		mMainSwapChain = nullptr;
	}
	mRenderer = nullptr;
	if (nullptr != mRendererInstance)
	{
		mRendererInstance->destroyRenderer();
	}

	delete mRendererInstance;
	mRendererInstance = nullptr;
	delete mRendererContext;
	mRendererContext = nullptr;
#ifdef RENDERER_RUNTIME_GRAPHICS_DEBUGGER
	delete static_cast<RendererRuntime::RenderDocGraphicsDebugger*>(mGraphicsDebugger);
#endif
}

Renderer::IRenderer* AppRenderer::createRendererInstance(const char* rendererName)
{
	if (nullptr != rendererName)
	{
		bool loadRendererApiSharedLibrary = false;
#ifdef _WIN32
		mRendererContext = new Renderer::Context(g_DefaultLog, ::g_DefaultAssert, g_DefaultAllocator, getNativeWindowHandle());
#endif
#if defined(RENDERER_RUNTIME) && defined(RENDERER_RUNTIME_GRAPHICS_DEBUGGER)
		mGraphicsDebugger = new RendererRuntime::RenderDocGraphicsDebugger(*mRendererContext);
#endif
		mRendererInstance = new Renderer::RendererInstance(rendererName, *mRendererContext, loadRendererApiSharedLibrary);
	}
	Renderer::IRenderer* renderer = (nullptr != mRendererInstance) ? mRendererInstance->getRenderer() : nullptr;

	if (nullptr != renderer && !renderer->isInitialized())
	{
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

	return renderer;
}