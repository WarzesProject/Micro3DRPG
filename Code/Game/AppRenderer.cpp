#include "stdafx.h"
#include "AppRenderer.h"
#include <Engine/RendererInstance.h>
#include "BaseExample.h"
#ifdef SDL2_FOUND
#include "Framework/ApplicationImplSdl2.h"
#elif defined _WIN32
#include "Framework/ApplicationImplWindows.h"
#elif defined __ANDROID__
#warning TODO(co) The Android support is work - in - progress
#elif defined LINUX
#include "Framework/Linux/ApplicationImplLinux.h"
#endif
#if defined(RENDERER_RUNTIME) && defined(RENDERER_RUNTIME_GRAPHICS_DEBUGGER)
#include <Engine/Core/RenderDocGraphicsDebugger.h>
#endif

extern Renderer::DefaultAllocator g_DefaultAllocator;
extern Renderer::DefaultLog	g_DefaultLog;
extern Renderer::DefaultAssert g_DefaultAssert;

AppRenderer::AppRenderer(const char* rendererName, BaseExample& exampleBase) 
	:mApplicationImpl(nullptr)
	, mExit(false)
#if defined(RENDERER_RUNTIME) && defined(RENDERER_RUNTIME_GRAPHICS_DEBUGGER)
	mGraphicsDebugger(nullptr),
#endif
	,mExampleBase(exampleBase),
	mRendererContext(nullptr),
	mRendererInstance(nullptr),
	mRenderer(nullptr),
	mMainSwapChain(nullptr)
{
#ifdef SDL2_FOUND
	mApplicationImpl = new AppWindowSDL(*this, rendererName);
#elif defined _WIN32
	mApplicationImpl = new ApplicationImplWindows(*this, windowTitle);
#elif defined __ANDROID__
	#warning TODO(co) The Android support is work - in - progress
#elif defined LINUX
	mApplicationImpl = new ApplicationImplLinux(*this, windowTitle);
#else
#error "Unsupported platform"
#endif

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
	// Delete the renderer instance
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

	// Call base implementation after renderer was destroyed, needed at least under Linux see comments in private method "RendererInstance::loadRendererApiSharedLibrary()" for more details
	// TODO(co): Try to find another solution which doesn't change the application flow which results in deinitialization been called twice
#ifdef LINUX
	IApplication::onDeinitialization();
#endif

	// Delete the renderer instance
	delete mRendererInstance;
	mRendererInstance = nullptr;
	delete mRendererContext;
	mRendererContext = nullptr;
#ifdef RENDERER_RUNTIME_GRAPHICS_DEBUGGER
	delete static_cast<RendererRuntime::RenderDocGraphicsDebugger*>(mGraphicsDebugger);
#endif
}

//[-------------------------------------------------------]
//[ Private methods                                       ]
//[-------------------------------------------------------]
Renderer::IRenderer* AppRenderer::createRendererInstance(const char* rendererName)
{
	// Is the given pointer valid?
	if (nullptr != rendererName)
	{
		bool loadRendererApiSharedLibrary = false;
#ifdef _WIN32
		mRendererContext = new Renderer::Context(g_DefaultLog, ::g_DefaultAssert, g_DefaultAllocator, getNativeWindowHandle());
#elif LINUX
		// Under Linux the OpenGL library interacts with the library from X11 so we need to load the library ourself instead letting it be loaded by the renderer instance
		// -> See http://dri.sourceforge.net/doc/DRIuserguide.html "11.5 libGL.so and dlopen()"
		loadRendererApiSharedLibrary = true;
		mRendererContext = new Renderer::X11Context(log, ::detail::g_DefaultAssert, g_DefaultAllocator, getX11Display(), getNativeWindowHandle());
#endif
#if defined(RENDERER_RUNTIME) && defined(RENDERER_RUNTIME_GRAPHICS_DEBUGGER)
		mGraphicsDebugger = new RendererRuntime::RenderDocGraphicsDebugger(*mRendererContext);
#endif
		mRendererInstance = new Renderer::RendererInstance(rendererName, *mRendererContext, loadRendererApiSharedLibrary);
	}
	Renderer::IRenderer* renderer = (nullptr != mRendererInstance) ? mRendererInstance->getRenderer() : nullptr;

	// Is the renderer instance is properly initialized?
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

	// Done
	return renderer;
}
