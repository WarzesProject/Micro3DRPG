#pragma once

#include "Export.h"

namespace Renderer
{
	class ILog;
	class IAssert;
	class IRenderer;
	class IAllocator;
}
namespace RendererRuntime
{
	class IFileManager;
	#ifdef RENDERER_RUNTIME_GRAPHICS_DEBUGGER
		class IGraphicsDebugger;
	#endif
	#ifdef RENDERER_RUNTIME_PROFILER
		class IProfiler;
	#endif
}


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
namespace RendererRuntime
{


	//[-------------------------------------------------------]
	//[ Classes                                               ]
	//[-------------------------------------------------------]
	/**
	*  @brief
	*    Context class encapsulating all embedding related wirings
	*/
	class Context final
	{


	//[-------------------------------------------------------]
	//[ Public methods                                        ]
	//[-------------------------------------------------------]
	public:
		/**
		*  @brief
		*    Constructor
		*
		*  @param[in] renderer
		*    Renderer instance to use, the render instance must stay valid as long as the renderer runtime instance exists
		*  @param[in] fileManager
		*    File manager instance to use, the file manager instance must stay valid as long as the renderer runtime instance exists
		*  @param[in] graphicsDebugger
		*    Graphics debugger instance to use, the graphics debugger instance must stay valid as long as the renderer runtime instance exists
		*  @param[in] profiler
		*    Profiler instance to use, the profiler instance must stay valid as long as the renderer runtime instance exists
		*/
		#if defined(RENDERER_RUNTIME_GRAPHICS_DEBUGGER) && defined(RENDERER_RUNTIME_PROFILER)
			RENDERERRUNTIME_API_EXPORT Context(Renderer::IRenderer& renderer, IFileManager& fileManager, IGraphicsDebugger& graphicsDebugger, IProfiler& profiler);
		#elif defined RENDERER_RUNTIME_GRAPHICS_DEBUGGER
			RENDERERRUNTIME_API_EXPORT Context(Renderer::IRenderer& renderer, IFileManager& fileManager, IGraphicsDebugger& graphicsDebugger);
		#elif defined RENDERER_RUNTIME_PROFILER
			RENDERERRUNTIME_API_EXPORT Context(Renderer::IRenderer& renderer, IFileManager& fileManager, IProfiler& profiler);
		#else
			RENDERERRUNTIME_API_EXPORT Context(Renderer::IRenderer& renderer, IFileManager& fileManager);
		#endif

		/**
		*  @brief
		*    Destructor
		*/
		inline ~Context()
		{
			// Nothing here
		}

		/**
		*  @brief
		*    Return the log instance
		*
		*  @return
		*    The log instance
		*/
		[[nodiscard]] inline Renderer::ILog& getLog() const
		{
			return mLog;
		}

		/**
		*  @brief
		*    Return the assert instance
		*
		*  @return
		*    The assert instance
		*/
		[[nodiscard]] inline Renderer::IAssert& getAssert() const
		{
			return mAssert;
		}

		/**
		*  @brief
		*    Return the allocator instance
		*
		*  @return
		*    The allocator instance
		*/
		[[nodiscard]] inline Renderer::IAllocator& getAllocator() const
		{
			return mAllocator;
		}

		/**
		*  @brief
		*    Return the used renderer instance
		*
		*  @return
		*    The used renderer instance
		*/
		[[nodiscard]] inline Renderer::IRenderer& getRenderer() const
		{
			return mRenderer;
		}

		/**
		*  @brief
		*    Return the used file manager instance
		*
		*  @return
		*    The used file manager instance
		*/
		[[nodiscard]] inline IFileManager& getFileManager() const
		{
			return mFileManager;
		}

		#ifdef RENDERER_RUNTIME_GRAPHICS_DEBUGGER
			/**
			*  @brief
			*    Return the used graphics debugger instance
			*
			*  @return
			*    The used graphics debugger instance
			*/
			[[nodiscard]] inline IGraphicsDebugger& getGraphicsDebugger() const
			{
				return mGraphicsDebugger;
			}
		#endif

		#ifdef RENDERER_RUNTIME_PROFILER
			/**
			*  @brief
			*    Return the used profiler instance
			*
			*  @return
			*    The used profiler instance
			*/
			[[nodiscard]] inline IProfiler& getProfiler() const
			{
				return mProfiler;
			}
		#endif


	//[-------------------------------------------------------]
	//[ Private methods                                       ]
	//[-------------------------------------------------------]
	private:
		explicit Context(const Context&) = delete;
		Context& operator=(const Context&) = delete;


	//[-------------------------------------------------------]
	//[ Private data                                          ]
	//[-------------------------------------------------------]
	private:
		Renderer::ILog&		  mLog;
		Renderer::IAssert&	  mAssert;
		Renderer::IAllocator& mAllocator;
		Renderer::IRenderer&  mRenderer;
		IFileManager&		  mFileManager;
		#ifdef RENDERER_RUNTIME_GRAPHICS_DEBUGGER
			IGraphicsDebugger& mGraphicsDebugger;
		#endif
		#ifdef RENDERER_RUNTIME_PROFILER
			IProfiler& mProfiler;
		#endif


	};


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererRuntime
