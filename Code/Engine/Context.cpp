#include "stdafx.h"
#include "Context.h"
#include "Renderer.h"

namespace RendererRuntime
{


	//[-------------------------------------------------------]
	//[ Public methods                                        ]
	//[-------------------------------------------------------]
	#if defined(RENDERER_RUNTIME_GRAPHICS_DEBUGGER) && defined(RENDERER_RUNTIME_PROFILER)
		Context::Context(Renderer::IRenderer& renderer, IFileManager& fileManager, IGraphicsDebugger& graphicsDebugger, IProfiler& profiler) :
	#elif defined RENDERER_RUNTIME_GRAPHICS_DEBUGGER
		Context::Context(Renderer::IRenderer& renderer, IFileManager& fileManager, IGraphicsDebugger& graphicsDebugger) :
	#elif defined RENDERER_RUNTIME_PROFILER
		Context::Context(Renderer::IRenderer& renderer, IFileManager& fileManager, IProfiler& profiler) :
	#else
		Context::Context(Renderer::IRenderer& renderer, IFileManager& fileManager) :
	#endif
		mLog(renderer.getContext().getLog()),
		mAssert(renderer.getContext().getAssert()),
		mAllocator(renderer.getContext().getAllocator()),
		mRenderer(renderer),
		mFileManager(fileManager)
		#ifdef RENDERER_RUNTIME_GRAPHICS_DEBUGGER
			, mGraphicsDebugger(graphicsDebugger)
		#endif
		#ifdef RENDERER_RUNTIME_PROFILER
			, mProfiler(profiler)
		#endif
	{
		// Nothing here
	}


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererRuntime
