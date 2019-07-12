#pragma once
#ifndef RENDERER_RUNTIME_GRAPHICS_DEBUGGER
	#error "Do only include this header if the RENDERER_RUNTIME_GRAPHICS_DEBUGGER preprocessor definition is set"
#endif


//[-------------------------------------------------------]
//[ Includes                                              ]
//[-------------------------------------------------------]
#include "Core/IGraphicsDebugger.h"

#include <RenderDoc/renderdoc_app.h>

#ifdef _WIN32
	// Set Windows version to Windows Vista (0x0600), we don't support Windows XP (0x0501)
	#ifdef WINVER
		#undef WINVER
	#endif
	#define WINVER			0x0600
	#ifdef _WIN32_WINNT
		#undef _WIN32_WINNT
	#endif
	#define _WIN32_WINNT	0x0600

	// Exclude some stuff from "windows.h" to speed up compilation a bit
	#define WIN32_LEAN_AND_MEAN
	#define NOGDICAPMASKS
	#define NOMENUS
	#define NOICONS
	#define NOKEYSTATES
	#define NOSYSCOMMANDS
	#define NORASTEROPS
	#define OEMRESOURCE
	#define NOATOM
	#define NOMEMMGR
	#define NOMETAFILE
	#define NOOPENFILE
	#define NOSCROLL
	#define NOSERVICE
	#define NOSOUND
	#define NOWH
	#define NOCOMM
	#define NOKANJI
	#define NOHELP
	#define NOPROFILER
	#define NODEFERWINDOWPOS
	#define NOMCX
	#define NOCRYPT
	#include <windows.h>

	// Get rid of some nasty OS macros
	#undef min
	#undef max
#elif defined LINUX
	#include <dlfcn.h>
#else
	#error "Unsupported platform"
#endif

#include <stdio.h>
#include <cassert>


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
	*    RenderDoc ( https://renderdoc.org/ ) graphics debugger implementation class one can use
	*
	*  @note
	*    - Designed to be instanced and used inside a single C++ file
	*    - Must be instanced before the renderer gets instantiated
	*    - See https://renderdoc.org/docs/in_application_api.html for RenderDoc integration details
	*/
	class RenderDocGraphicsDebugger final : public IGraphicsDebugger
	{


	//[-------------------------------------------------------]
	//[ Public methods                                        ]
	//[-------------------------------------------------------]
	public:
		inline RenderDocGraphicsDebugger(Renderer::Context& context) :
			mRenderDocSharedLibrary(nullptr),
			mRenderDocApi(nullptr)
		{
			// Sanity check
			#if !defined(_DEBUG) && !defined(SHARED_LIBRARIES)
				RENDERER_LOG(context, PERFORMANCE_WARNING, "Reminder: You might not want to ship products with enabled RenderDoc graphics debugging")
			#endif

			// Load in the RenderDoc shared library
			pRENDERDOC_GetAPI RENDERDOC_GetAPI = nullptr;
			#ifdef _WIN32
			{
				HMODULE hModule = GetModuleHandleA("renderdoc.dll");
				if (nullptr == hModule)
				{
					// RenderDoc isn't already running, so we have to explicitly load the dll
					char renderDocFilename[256] = "C:\\Program Files\\RenderDoc\\renderdoc.dll";	// TODO(co) Get this dynamically by using e.g. the registry key "HKEY_LOCAL_MACHINE\SOFTWARE\Classes\RenderDoc.RDCCapture.1\DefaultIcon\", also handle x86/x64
					mRenderDocSharedLibrary = ::LoadLibraryExA(renderDocFilename, nullptr, LOAD_WITH_ALTERED_SEARCH_PATH);
					if (nullptr != mRenderDocSharedLibrary)
					{
						hModule = static_cast<HMODULE>(mRenderDocSharedLibrary);
					}
					else
					{
						// Error!
						RENDERER_LOG(context, CRITICAL, "Failed to locate the entry point \"RENDERDOC_GetAPI\" within the shared RenderDoc library \"%s\"", renderDocFilename)
					}
				}
				if (nullptr != hModule)
				{
					void* symbol = ::GetProcAddress(hModule, "RENDERDOC_GetAPI");
					if (nullptr != symbol)
					{
						RENDERDOC_GetAPI = static_cast<pRENDERDOC_GetAPI>(symbol);
					}
					else
					{
						// Error!
						RENDERER_LOG(context, CRITICAL, "Failed to locate the entry point \"RENDERDOC_GetAPI\" within the shared RenderDoc library \"renderdoc.dll\"")
					}
				}
			}
			#elif defined LINUX
			{
				#if defined __ANDROID__
					mRenderDocSharedLibrary = ::dlopen("libVkLayer_GLES_RenderDoc.so", RTLD_NOW | RTLD_NOLOAD);
				#else
					mRenderDocSharedLibrary = ::dlopen("librenderdoc.so", RTLD_NOW | RTLD_NOLOAD);
				#endif
				if (nullptr != mRenderDocSharedLibrary)
				{
					void* symbol = ::dlsym(mRenderDocSharedLibrary, "RENDERDOC_GetAPI");
					if (nullptr != symbol)
					{
						RENDERDOC_GetAPI = static_cast<pRENDERDOC_GetAPI>(symbol);
					}
					else
					{
						// Error!
						RENDERER_LOG(context, CRITICAL, "Failed to locate the entry point \"RENDERDOC_GetAPI\" within the shared RenderDoc library \"%s\"", renderDocFilename)
					}
				}
				else
				{
					RENDERER_LOG(context, CRITICAL, "Failed to load in the shared RenderDoc library \"%s\"", renderDocFilename)
				}
			}
			#else
				#error "Unsupported platform"
			#endif

			// Get the RenderDoc API instance
			if (nullptr != RENDERDOC_GetAPI && RENDERDOC_GetAPI(eRENDERDOC_API_Version_1_3_0, reinterpret_cast<void**>(&mRenderDocApi)) == 1)
			{
				// Disable the RenderDoc overlay by default
				mRenderDocApi->MaskOverlayBits(eRENDERDOC_Overlay_None, 0);
			}
			else
			{
				// Error!
				RENDERER_LOG(context, CRITICAL, "Failed to get the RenderDoc API 1.3.0 instance")
			}
		}

		inline virtual ~RenderDocGraphicsDebugger() override
		{
			#ifdef _DEBUG
				assert((0 == mNumberOfCurrentlyStartedFrameCaptures) && "Graphics debugger capturing leak detected, not all started frame capturings were ended");
			#endif

			// Unload the shared library instance
			#ifdef _WIN32
				if (nullptr != mRenderDocSharedLibrary)
				{
					::FreeLibrary(static_cast<HMODULE>(mRenderDocSharedLibrary));
				}
			#elif defined LINUX
				if (nullptr != mRenderDocSharedLibrary)
				{
					::dlclose(mRenderDocSharedLibrary);
				}
			#else
				#error "Unsupported platform"
			#endif
		}


	//[-------------------------------------------------------]
	//[ Public virtual RendererRuntime::IProfiler methods     ]
	//[-------------------------------------------------------]
	public:
		[[nodiscard]] virtual bool isInitialized() const override
		{
			return (nullptr != mRenderDocApi);
		}

		virtual void startFrameCapture(Renderer::handle nativeWindowHandle) override
		{
			#ifdef _DEBUG
				assert(nullptr != mRenderDocApi);
				assert(getCaptureNextFrame());
				++mNumberOfCurrentlyStartedFrameCaptures;
			#endif
			mRenderDocApi->StartFrameCapture(nullptr, reinterpret_cast<RENDERDOC_WindowHandle>(nativeWindowHandle));
		}

		virtual void endFrameCapture(Renderer::handle nativeWindowHandle) override
		{
			#ifdef _DEBUG
				assert(nullptr != mRenderDocApi);
				assert(getCaptureNextFrame());
				--mNumberOfCurrentlyStartedFrameCaptures;
			#endif
			mRenderDocApi->EndFrameCapture(nullptr, reinterpret_cast<RENDERDOC_WindowHandle>(nativeWindowHandle));
			resetCaptureNextFrame();
		}


	//[-------------------------------------------------------]
	//[ Private methods                                       ]
	//[-------------------------------------------------------]
	private:
		explicit RenderDocGraphicsDebugger(const RenderDocGraphicsDebugger&) = delete;
		RenderDocGraphicsDebugger& operator=(const RenderDocGraphicsDebugger&) = delete;

		// Implementation from "08/02/2015 Better array 'countof' implementation with C++ 11 (updated)" - https://www.g-truc.net/post-0708.html
		template<typename T, std::size_t N>
		[[nodiscard]] constexpr std::size_t countof(T const (&)[N])
		{
			return N;
		}


	//[-------------------------------------------------------]
	//[ Private data                                          ]
	//[-------------------------------------------------------]
	private:
		#ifdef _DEBUG
			int mNumberOfCurrentlyStartedFrameCaptures = 0;	///< For leak detection
		#endif
		void*				 mRenderDocSharedLibrary;	///< Shared RenderDoc library, can be a null pointer
		RENDERDOC_API_1_3_0* mRenderDocApi;				///< RenderDoc API instance, can be a null pointer


	};


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererRuntime
