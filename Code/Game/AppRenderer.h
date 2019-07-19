#pragma once

#include "Framework/PlatformTypes.h"
#include "AppWindowSDL.h"

class BaseExample;

class AppRenderer
{
public:
	// "Null", "Vulkan", "OpenGL", "OpenGLES3", "Direct3D11", "Direct3D12"
	AppRenderer(const char* rendererName, BaseExample &exampleBase);
	~AppRenderer();

	[[nodiscard]] Renderer::IRenderer* getRenderer() const
	{
		return mRenderer;
	}

	[[nodiscard]] Renderer::IRenderTarget* getMainRenderTarget() const
	{
		return mMainSwapChain;
	}

	[[nodiscard]] int run();

	inline void exit()
	{
		mExit = true;
	}

	inline void getWindowSize(int& width, int& height) const
	{
		return mApplicationImpl->getWindowSize(width, height);
	}

	[[nodiscard]] inline handle getNativeWindowHandle() const
	{
		return mApplicationImpl->getNativeWindowHandle();
	}

	inline void redraw()
	{
		mApplicationImpl->redraw();
	}
public:
	[[nodiscard]] bool onInitialization();
	void onDeinitialization();
	void onUpdate();
	void onResize();
	void onToggleFullscreenState();
	void onDrawRequest();
	void onEscapeKey();
protected:
	void createRenderer();
	void destroyRenderer();
protected:
#ifdef RENDERER_RUNTIME_GRAPHICS_DEBUGGER
	RendererRuntime::IGraphicsDebugger*	mGraphicsDebugger = nullptr;	///< Graphics debugger instance, can be a null pointer
#endif
	BaseExample& mExampleBase;

private:
	[[nodiscard]] Renderer::IRenderer* createRendererInstance(const char* rendererName);

private:
	char						mRendererName[32];	///< Case sensitive ASCII name of the renderer to instance
	Renderer::Context*			mRendererContext = nullptr;	///< Renderer context, can be a null pointer
	Renderer::RendererInstance* mRendererInstance = nullptr;	///< Renderer instance, can be a null pointer
	Renderer::IRenderer*		mRenderer = nullptr;			///< Renderer instance, can be a null pointer, do not destroy the instance
	Renderer::ISwapChain*		mMainSwapChain = nullptr;		///< Main swap chain instance, can be a null pointer, release the instance if you no longer need it
	Renderer::CommandBuffer		mCommandBuffer;		///< Command buffer

	AppWindowSDL* mApplicationImpl = nullptr;	///< Application implementation instance, always valid
	bool			  mExit = false;	///< If "true", the application has been asked politely to shut down as soon as possible, else "false"
};