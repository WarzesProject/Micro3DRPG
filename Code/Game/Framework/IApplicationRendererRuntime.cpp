#include "stdafx.h"
#include "Framework/IApplicationRendererRuntime.h"
#include "Framework/ExampleBase.h"

#include <Engine/Context.h>
#include <Engine/RendererRuntimeInstance.h>
#include <Engine/Asset/AssetManager.h>
#ifdef RENDERER_RUNTIME_PROFILER
	#include <Engine/Core/RemoteryProfiler.h>
#endif
#ifdef __ANDROID__
	#include <Engine/Core/File/AndroidFileManager.h>

	#include <android_native_app_glue.h>
#else
	#include <Engine/Core/File/PhysicsFSFileManager.h>
#endif

#ifdef RENDERER_TOOLKIT
	#include <Engine/Core/File/DefaultFileManager.h>

	#include <RendererToolkit/Public/RendererToolkitInstance.h>

	#include <exception>
#endif


//[-------------------------------------------------------]
//[ Public virtual IApplicationFrontend methods           ]
//[-------------------------------------------------------]
RendererRuntime::IRendererRuntime* IApplicationRendererRuntime::getRendererRuntime() const
{
	return (nullptr != mRendererRuntimeInstance) ? mRendererRuntimeInstance->getRendererRuntime() : nullptr;
}

RendererToolkit::IRendererToolkit* IApplicationRendererRuntime::getRendererToolkit()
{
	#ifdef RENDERER_TOOLKIT
		// Create the renderer toolkit instance, if required
		if (nullptr == mRendererToolkitInstance)
		{
			assert((nullptr != mRendererRuntimeInstance) && "The renderer runtime instance must be valid");
			const RendererRuntime::IRendererRuntime* rendererRuntime = mRendererRuntimeInstance->getRendererRuntime();
			Renderer::ILog& log = rendererRuntime->getRenderer().getContext().getLog();
			Renderer::IAssert& assert = rendererRuntime->getRenderer().getContext().getAssert();
			Renderer::IAllocator& allocator = rendererRuntime->getRenderer().getContext().getAllocator();
			mRendererToolkitFileManager = new RendererRuntime::DefaultFileManager(log, assert, allocator, mFileManager->getAbsoluteRootDirectory());
			mRendererToolkitContext = new RendererToolkit::Context(log, assert, allocator, *mRendererToolkitFileManager);
			mRendererToolkitInstance = new RendererToolkit::RendererToolkitInstance(*mRendererToolkitContext);
		}
		return (nullptr != mRendererToolkitInstance) ? mRendererToolkitInstance->getRendererToolkit() : nullptr;
	#else
		return nullptr;
	#endif
}


//[-------------------------------------------------------]
//[ Public virtual IApplication methods                   ]
//[-------------------------------------------------------]
bool IApplicationRendererRuntime::onInitialization()
{
	// Create the renderer instance
	createRenderer();

	// Is there a valid renderer instance?
	Renderer::IRenderer* renderer = getRenderer();
	if (nullptr != renderer)
	{
		// Create the renderer runtime instance
		#ifdef __ANDROID__
			struct android_app androidApp;	// TODO(co) Get Android app instance
			assert((nullptr != androidApp.activity->assetManager) && "Invalid Android asset manager instance");
			mFileManager = new RendererRuntime::AndroidFileManager(renderer->getContext().getLog(), renderer->getContext().getAssert(), renderer->getContext().getAllocator(), std_filesystem::canonical(std_filesystem::current_path() / "..").generic_string(), *androidApp.activity->assetManager);
		#else
			mFileManager = new RendererRuntime::PhysicsFSFileManager(renderer->getContext().getLog(), std_filesystem::canonical(std_filesystem::current_path() / "..").generic_string());
		#endif
		#if defined(RENDERER_RUNTIME_GRAPHICS_DEBUGGER) && defined(RENDERER_RUNTIME_PROFILER)
			mProfiler = new RendererRuntime::RemoteryProfiler(*renderer);
			mRendererRuntimeContext = new RendererRuntime::Context(*renderer, *mFileManager, *mGraphicsDebugger, *mProfiler);
		#elif defined RENDERER_RUNTIME_GRAPHICS_DEBUGGER
			mRendererRuntimeContext = new RendererRuntime::Context(*renderer, *mFileManager, *mGraphicsDebugger);
		#elif defined RENDERER_RUNTIME_PROFILER
			mProfiler = new RendererRuntime::RemoteryProfiler(*renderer);
			mRendererRuntimeContext = new RendererRuntime::Context(*renderer, *mFileManager, *mProfiler);
		#else
			mRendererRuntimeContext = new RendererRuntime::Context(*renderer, *mFileManager);
		#endif
		mRendererRuntimeInstance = new RendererRuntime::RendererRuntimeInstance(*mRendererRuntimeContext);

		{
			RendererRuntime::IRendererRuntime* rendererRuntime = getRendererRuntime();
			if (nullptr != rendererRuntime)
			{
				// Mount asset package
				bool mountAssetPackageResult = false;
				RendererRuntime::AssetManager& assetManager = rendererRuntime->getAssetManager();
				bool rendererIsOpenGLES = (renderer->getNameId() == Renderer::NameId::OPENGLES3);
				if (rendererIsOpenGLES)
				{
					// Handy fallback for development: If the mobile data isn't there, use the PC data
					mountAssetPackageResult = (assetManager.mountAssetPackage("../DataMobile/Example/Content", "Example") != nullptr);
					if (!mountAssetPackageResult)
					{
						RENDERER_LOG(rendererRuntime->getContext(), COMPATIBILITY_WARNING, "The examples application failed to find \"../DataMobile/Example/Content\", using \"../DataPc/Example/Content\" as fallback")
						mountAssetPackageResult = (assetManager.mountAssetPackage("../DataPc/Example/Content", "Example") != nullptr);
						rendererIsOpenGLES = false;
					}
				}
				else
				{
					mountAssetPackageResult = (assetManager.mountAssetPackage("../DataPc/Example/Content", "Example") != nullptr);
				}
				if (!mountAssetPackageResult)
				{
					// Error!
					showUrgentMessage("Please start \"ExampleProjectCompiler\" before starting \"Examples\" for the first time");
					deinitialization();
					return false;
				}
				rendererRuntime->loadPipelineStateObjectCache();

				// Load renderer toolkit project to enable hot-reloading in case of asset changes
				#ifdef RENDERER_TOOLKIT
				{
					RendererToolkit::IRendererToolkit* rendererToolkit = getRendererToolkit();
					if (nullptr != rendererToolkit)
					{
						mProject = rendererToolkit->createProject();
						if (nullptr != mProject)
						{
							try
							{
								// Load project: Shippable executable binaries are inside e.g. "unrimp/Binary/Windows_x64_Shared" while development data source is located
								// at "unrimp/Example/DataSource/Example" and the resulting compiled/baked data ends up inside e.g. "unrimp/Binary/DataPc/Example"
								mProject->load("../../Example/DataSource/Example");
								mProject->startupAssetMonitor(*rendererRuntime, rendererIsOpenGLES ? "OpenGLES3_300" : "Direct3D11_50");
							}
							catch (const std::exception& e)
							{
								RENDERER_LOG(renderer->getContext(), CRITICAL, "Failed to load renderer toolkit project: %s", e.what())
							}
						}
					}
				}
				#endif
			}
		}
	}

	// Initialize the example now that the renderer instance should be created successfully
	mExampleBase.onInitialization();

	// Done
	return true;
}

void IApplicationRendererRuntime::onDeinitialization()
{
	mExampleBase.onDeinitialization();
	deinitialization();
}

void IApplicationRendererRuntime::onUpdate()
{
	RendererRuntime::IRendererRuntime* rendererRuntime = getRendererRuntime();
	if (nullptr != rendererRuntime)
	{
		rendererRuntime->update();
	}

	// Call base implementation
	IApplicationRenderer::onUpdate();
}


//[-------------------------------------------------------]
//[ Private methods                                       ]
//[-------------------------------------------------------]
void IApplicationRendererRuntime::deinitialization()
{
	delete mRendererRuntimeInstance;
	mRendererRuntimeInstance = nullptr;
	delete mRendererRuntimeContext;
	mRendererRuntimeContext = nullptr;
	#ifdef RENDERER_RUNTIME_PROFILER
		delete static_cast<RendererRuntime::RemoteryProfiler*>(mProfiler);
	#endif
	#ifdef __ANDROID__
		delete static_cast<RendererRuntime::AndroidFileManager*>(mFileManager);
	#else
		delete static_cast<RendererRuntime::PhysicsFSFileManager*>(mFileManager);
	#endif
	mFileManager = nullptr;
	#ifdef RENDERER_TOOLKIT
		if (nullptr != mProject)
		{
			delete mProject;
			mProject = nullptr;
		}
		if (nullptr != mRendererToolkitInstance)
		{
			delete mRendererToolkitInstance;
			mRendererToolkitInstance = nullptr;
		}
		if (nullptr != mRendererToolkitContext)
		{
			delete mRendererToolkitContext;
			mRendererToolkitContext = nullptr;
		}
		if (nullptr != mRendererToolkitFileManager)
		{
			delete static_cast<RendererRuntime::DefaultFileManager*>(mRendererToolkitFileManager);
			mRendererToolkitFileManager = nullptr;
		}
	#endif
	destroyRenderer();
}
