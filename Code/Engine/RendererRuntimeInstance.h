#pragma once

#include "Context.h"
#include "IRendererRuntime.h"

namespace RendererRuntime
{
	class Context;
}

#ifndef SHARED_LIBRARIES
	// Statically linked library create renderer runtime instance signatures
	// This is needed to do here because the methods in the library are also defined in global namespace

	// "createRendererRuntimeInstance()" signature
	[[nodiscard]] extern RendererRuntime::IRendererRuntime* createRendererRuntimeInstance(RendererRuntime::Context& context);
#endif


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
	*    Renderer runtime instance using runtime linking
	*
	*  @note
	*    - Designed to be instanced and used inside a single C++ file
	*/
	class RendererRuntimeInstance final
	{


	//[-------------------------------------------------------]
	//[ Public methods                                        ]
	//[-------------------------------------------------------]
	public:
		/**
		*  @brief
		*    Constructor
		*
		*  @param[in] context
		*    Renderer runtime context, the renderer runtime context instance must stay valid as long as the renderer runtime instance exists
		*/
		explicit RendererRuntimeInstance(Context& context)
		{
			#ifdef SHARED_LIBRARIES
				// Dynamically linked libraries
				#ifdef _WIN32
					// Load in the dll
					static constexpr const char RENDERER_RUNTIME_FILENAME[] = "RendererRuntime.dll";
					mRendererRuntimeSharedLibrary = ::LoadLibraryExA(RENDERER_RUNTIME_FILENAME, nullptr, LOAD_WITH_ALTERED_SEARCH_PATH);
					if (nullptr != mRendererRuntimeSharedLibrary)
					{
						// Get the "createRendererRuntimeInstance()" function pointer
						void* symbol = ::GetProcAddress(static_cast<HMODULE>(mRendererRuntimeSharedLibrary), "createRendererRuntimeInstance");
						if (nullptr != symbol)
						{
							// "createRendererRuntimeInstance()" signature
							typedef IRendererRuntime* (__cdecl *createRendererRuntimeInstance)(Context& context);

							// Create the renderer runtime instance
							mRendererRuntime = static_cast<createRendererRuntimeInstance>(symbol)(context);
						}
						else
						{
							// Error!
							RENDERER_LOG(context, CRITICAL, "Failed to locate the entry point \"createRendererRuntimeInstance\" within the shared renderer runtime library \"%s\"", RENDERER_RUNTIME_FILENAME)
						}
					}
					else
					{
						// Error!
						RENDERER_LOG(context, CRITICAL, "Failed to load in the shared renderer runtime library \"%s\"", RENDERER_RUNTIME_FILENAME)
					}
				#elif defined LINUX
					// Load in the shared library
					static constexpr const char RENDERER_RUNTIME_FILENAME[] = "libRendererRuntime.so";
					mRendererRuntimeSharedLibrary = dlopen(RENDERER_RUNTIME_FILENAME, RTLD_NOW);
					if (nullptr != mRendererRuntimeSharedLibrary)
					{
						// Get the "createRendererRuntimeInstance()" function pointer
						void* symbol = dlsym(mRendererRuntimeSharedLibrary, "createRendererRuntimeInstance");
						if (nullptr != symbol)
						{
							// "createRendererRuntimeInstance()" signature
							typedef IRendererRuntime* (*createRendererRuntimeInstance)(Context& context);

							// Create the renderer runtime instance
							mRendererRuntime = reinterpret_cast<createRendererRuntimeInstance>(symbol)(context);
						}
						else
						{
							// Error!
							RENDERER_LOG(context, CRITICAL, "Failed to locate the entry point \"createRendererRuntimeInstance\" within the shared renderer runtime library \"%s\"", RENDERER_RUNTIME_FILENAME)
						}
					}
					else
					{
						// Error!
						RENDERER_LOG(context, CRITICAL, "Failed to load in the shared renderer runtime library \"%s\"", RENDERER_RUNTIME_FILENAME)
					}
				#else
					#error "Unsupported platform"
				#endif
			#else
				// Statically linked libraries

				// Create the renderer runtime instance
				mRendererRuntime = createRendererRuntimeInstance(context);
			#endif
		}

		/**
		*  @brief
		*    Destructor
		*/
		~RendererRuntimeInstance()
		{
			// Delete the renderer runtime instance
			mRendererRuntime = nullptr;

			// Destroy the shared library instance
			#ifdef SHARED_LIBRARIES
				#ifdef _WIN32
					if (nullptr != mRendererRuntimeSharedLibrary)
					{
						::FreeLibrary(static_cast<HMODULE>(mRendererRuntimeSharedLibrary));
						mRendererRuntimeSharedLibrary = nullptr;
					}
				#elif defined LINUX
					if (nullptr != mRendererRuntimeSharedLibrary)
					{
						::dlclose(mRendererRuntimeSharedLibrary);
						mRendererRuntimeSharedLibrary = nullptr;
					}
				#else
					#error "Unsupported platform"
				#endif
			#endif
		}

		/**
		*  @brief
		*    Return the renderer runtime instance
		*
		*  @remarks
		*    The renderer runtime instance, can be a null pointer
		*/
		[[nodiscard]] inline IRendererRuntime* getRendererRuntime() const
		{
			return mRendererRuntime;
		}


	//[-------------------------------------------------------]
	//[ Private data                                          ]
	//[-------------------------------------------------------]
	private:
		void*				mRendererRuntimeSharedLibrary;	///< Shared renderer runtime library, can be a null pointer
		IRendererRuntimePtr	mRendererRuntime;				///< Renderer runtime instance, can be a null pointer


	};


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererRuntime
