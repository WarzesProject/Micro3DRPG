#include "stdafx.h"
#include "RendererRuntimeImpl.h"
#include "Asset/AssetManager.h"
#include "Core/File/MemoryFile.h"
#include "Core/Time/TimeManager.h"
#include "Core/File/IFileManager.h"
#include "Core/Thread/ThreadPool.h"
#include "Resource/ResourceStreamer.h"
#include "Resource/RendererResourceManager.h"
#include "Resource/Mesh/MeshResourceManager.h"
#include "Resource/Scene/SceneResourceManager.h"
#include "Resource/ShaderPiece/ShaderPieceResourceManager.h"
#include "Resource/ShaderBlueprint/ShaderBlueprintResourceManager.h"
#include "Resource/VertexAttributes/VertexAttributesResourceManager.h"
#include "Resource/Texture/TextureResourceManager.h"
#include "Resource/MaterialBlueprint/MaterialBlueprintResourceManager.h"
#include "Resource/MaterialBlueprint/Cache/GraphicsPipelineStateCompiler.h"
#include "Resource/MaterialBlueprint/Cache/ComputePipelineStateCompiler.h"
#include "Resource/MaterialBlueprint/BufferManager/LightBufferManager.h"
#include "Resource/MaterialBlueprint/Listener/MaterialBlueprintResourceListener.h"
#include "Resource/Material/MaterialResourceManager.h"
#include "Resource/Skeleton/SkeletonResourceManager.h"
#include "Resource/SkeletonAnimation/SkeletonAnimationResourceManager.h"
#include "Resource/CompositorNode/CompositorNodeResourceManager.h"
#include "Resource/CompositorWorkspace/CompositorWorkspaceResourceManager.h"
#ifdef RENDERER_RUNTIME_IMGUI
	#ifdef _WIN32
		#include "DebugGui/Detail/DebugGuiManagerWindows.h"
	#elif LINUX
		#include "DebugGui/Detail/DebugGuiManagerLinux.h"
	#endif
#endif
#include "Context.h"

#include <cstring>


//[-------------------------------------------------------]
//[ Global functions                                      ]
//[-------------------------------------------------------]
// Export the instance creation function
RENDERERRUNTIME_FUNCTION_EXPORT RendererRuntime::IRendererRuntime* createRendererRuntimeInstance(RendererRuntime::Context& context)
{
	return RENDERER_NEW(context, RendererRuntime::RendererRuntimeImpl)(context);
}


//[-------------------------------------------------------]
//[ Anonymous detail namespace                            ]
//[-------------------------------------------------------]
namespace
{
	namespace detail
	{


		//[-------------------------------------------------------]
		//[ Global definitions                                    ]
		//[-------------------------------------------------------]
		namespace PipelineStateCache
		{
			static constexpr uint32_t FORMAT_TYPE	 = STRING_ID("PipelineStateCache");
			static constexpr uint32_t FORMAT_VERSION = 1;
		}


		//[-------------------------------------------------------]
		//[ Global functions                                      ]
		//[-------------------------------------------------------]
		void getPipelineStateObjectCacheFilename(const RendererRuntime::IRendererRuntime& rendererRuntime, std::string& virtualDirectoryName, std::string& virtualFilename)
		{
			virtualDirectoryName = rendererRuntime.getFileManager().getLocalDataMountPoint();
			virtualFilename = virtualDirectoryName + '/' + rendererRuntime.getRenderer().getName() + ".pso_cache";
		}

		[[nodiscard]] bool loadPipelineStateObjectCacheFile(const RendererRuntime::IRendererRuntime& rendererRuntime, RendererRuntime::MemoryFile& memoryFile)
		{
			// Tell the memory mapped file about the LZ4 compressed data and decompress it at once
			std::string virtualDirectoryName;
			std::string virtualFilename;
			getPipelineStateObjectCacheFilename(rendererRuntime, virtualDirectoryName, virtualFilename);
			const RendererRuntime::IFileManager& fileManager = rendererRuntime.getFileManager();
			if (fileManager.doesFileExist(virtualFilename.c_str()) && memoryFile.loadLz4CompressedDataByVirtualFilename(PipelineStateCache::FORMAT_TYPE, PipelineStateCache::FORMAT_VERSION, fileManager, virtualFilename.c_str()))
			{
				memoryFile.decompress();

				// Done
				return true;
			}
			
			// Failed to load the cache
			// -> No error since the cache might just not exist which is a valid situation
			return false;
		}

		void savePipelineStateObjectCacheFile(const RendererRuntime::IRendererRuntime& rendererRuntime, const RendererRuntime::MemoryFile& memoryFile)
		{
			std::string virtualDirectoryName;
			std::string virtualFilename;
			getPipelineStateObjectCacheFilename(rendererRuntime, virtualDirectoryName, virtualFilename);
			RendererRuntime::IFileManager& fileManager = rendererRuntime.getFileManager();
			if (fileManager.createDirectories(virtualDirectoryName.c_str()) && !memoryFile.writeLz4CompressedDataByVirtualFilename(PipelineStateCache::FORMAT_TYPE, PipelineStateCache::FORMAT_VERSION, fileManager, virtualFilename.c_str()))
			{
				RENDERER_LOG(rendererRuntime.getContext(), CRITICAL, "The renderer runtime failed to save the pipeline state object cache to \"%s\"", virtualFilename.c_str())
			}
		}


//[-------------------------------------------------------]
//[ Anonymous detail namespace                            ]
//[-------------------------------------------------------]
	} // detail
}


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
namespace RendererRuntime
{


	//[-------------------------------------------------------]
	//[ Public static methods                                 ]
	//[-------------------------------------------------------]
	void RendererRuntimeImpl::getDefaultTextureAssetIds(AssetIds& assetIds)
	{
		TextureResourceManager::getDefaultTextureAssetIds(assetIds);
		MaterialBlueprintResourceListener::getDefaultTextureAssetIds(assetIds);
		LightBufferManager::getDefaultTextureAssetIds(assetIds);
		#ifdef RENDERER_RUNTIME_IMGUI
			DebugGuiManager::getDefaultTextureAssetIds(assetIds);
		#endif
	}


	//[-------------------------------------------------------]
	//[ Public methods                                        ]
	//[-------------------------------------------------------]
	RendererRuntimeImpl::RendererRuntimeImpl(Context& context) :
		IRendererRuntime(context)
	{
		// Backup the given renderer and add our reference
		mRenderer = &context.getRenderer();
		mRenderer->addReference();

		// Create the buffer and texture manager instance and add our reference
		mBufferManager = mRenderer->createBufferManager();
		mBufferManager->addReference();
		mTextureManager = mRenderer->createTextureManager();
		mTextureManager->addReference();

		// Backup the given file manager instance
		mFileManager = &context.getFileManager();

		// Create the core manager instances
		mDefaultThreadPool = new DefaultThreadPool();
		mAssetManager = new AssetManager(*this);
		mTimeManager = new TimeManager();

		// Create the resource manager instances
		mRendererResourceManager = new RendererResourceManager(*this);
		mResourceStreamer = new ResourceStreamer(*this);
		mVertexAttributesResourceManager = new VertexAttributesResourceManager(*this);
		mTextureResourceManager = new TextureResourceManager(*this);
		mShaderPieceResourceManager = new ShaderPieceResourceManager(*this);
		mShaderBlueprintResourceManager = new ShaderBlueprintResourceManager(*this);
		mMaterialBlueprintResourceManager = new MaterialBlueprintResourceManager(*this);
		mMaterialResourceManager = new MaterialResourceManager(*this);
		mSkeletonResourceManager = new SkeletonResourceManager(*this);
		mSkeletonAnimationResourceManager = new SkeletonAnimationResourceManager(*this);
		mMeshResourceManager = new MeshResourceManager(*this);
		mSceneResourceManager = new SceneResourceManager(*this);
		mCompositorNodeResourceManager = new CompositorNodeResourceManager(*this);
		mCompositorWorkspaceResourceManager = new CompositorWorkspaceResourceManager(*this);

		// Register the resource managers inside the resource managers list
		mResourceManagers.push_back(mVertexAttributesResourceManager);
		mResourceManagers.push_back(mTextureResourceManager);
		mResourceManagers.push_back(mShaderPieceResourceManager);
		mResourceManagers.push_back(mShaderBlueprintResourceManager);
		mResourceManagers.push_back(mMaterialBlueprintResourceManager);
		mResourceManagers.push_back(mMaterialResourceManager);
		mResourceManagers.push_back(mSkeletonResourceManager);
		mResourceManagers.push_back(mSkeletonAnimationResourceManager);
		mResourceManagers.push_back(mMeshResourceManager);
		mResourceManagers.push_back(mSceneResourceManager);
		mResourceManagers.push_back(mCompositorNodeResourceManager);
		mResourceManagers.push_back(mCompositorWorkspaceResourceManager);

		// Misc
		mGraphicsPipelineStateCompiler = new GraphicsPipelineStateCompiler(*this);
		mComputePipelineStateCompiler = new ComputePipelineStateCompiler(*this);

		// Create the optional manager instances
		#ifdef RENDERER_RUNTIME_IMGUI
			#ifdef _WIN32
				mDebugGuiManager = new DebugGuiManagerWindows(*this);
			#elif LINUX
				mDebugGuiManager = new DebugGuiManagerLinux(*this);
			#else
				#error "Unsupported platform"
			#endif
			mDebugGuiManager->initializeImGuiKeyMap();
		#endif

		#ifdef RENDERER_RUNTIME_OPENVR
			mVrManager = new VrManagerOpenVR(*this);
		#endif

		// Don't try to load the pipeline state object cache at this point in time, the asset manager will have no asset packages and hence there will be no material blueprint assets
	}

	RendererRuntimeImpl::~RendererRuntimeImpl()
	{
		// Before doing anything else, ensure the resource streamer has no more work to do
		flushAllQueues();

		// Save pipeline state object cache
		savePipelineStateObjectCache();

		// Destroy the optional manager instances
		#ifdef RENDERER_RUNTIME_OPENVR
			delete mVrManager;
		#endif
		#ifdef RENDERER_RUNTIME_IMGUI
			delete mDebugGuiManager;
		#endif

		// Destroy miscellaneous
		delete mGraphicsPipelineStateCompiler;
		delete mComputePipelineStateCompiler;

		{ // Destroy the resource manager instances in reverse order
			const int numberOfResourceManagers = static_cast<int>(mResourceManagers.size());
			for (int i = numberOfResourceManagers - 1; i >= 0; --i)
			{
				delete mResourceManagers[static_cast<size_t>(i)];
			}
		}
		delete mResourceStreamer;

		// Destroy the core manager instances
		delete mTimeManager;
		delete mAssetManager;
		delete mDefaultThreadPool;

		// Release the texture and buffer manager instance
		mTextureManager->releaseReference();
		mBufferManager->releaseReference();
		delete mRendererResourceManager;

		// Release our renderer reference
		mRenderer->releaseReference();
	}


	//[-------------------------------------------------------]
	//[ Public virtual RendererRuntime::IRendererRuntime methods ]
	//[-------------------------------------------------------]
	void RendererRuntimeImpl::reloadResourceByAssetId(AssetId assetId)
	{
		// TODO(co) Optimization: If required later on, change this method to a "were's one, there are many"-signature (meaning passing multiple asset IDs at once)
		std::unique_lock<std::mutex> assetIdsOfResourcesToReloadMutexLock(mAssetIdsOfResourcesToReloadMutex);
		if (std::find(mAssetIdsOfResourcesToReload.begin(), mAssetIdsOfResourcesToReload.end(), assetId) == mAssetIdsOfResourcesToReload.cend())
		{
			mAssetIdsOfResourcesToReload.push_back(assetId);
		}
	}

	void RendererRuntimeImpl::flushAllQueues()
	{
		mResourceStreamer->flushAllQueues();
		mGraphicsPipelineStateCompiler->flushAllQueues();
		mComputePipelineStateCompiler->flushAllQueues();
	}

	void RendererRuntimeImpl::update()
	{
		// Update the time manager
		mTimeManager->update();

		{ // Handle resource reloading requests
			std::unique_lock<std::mutex> assetIdsOfResourcesToReloadMutexLock(mAssetIdsOfResourcesToReloadMutex);
			if (!mAssetIdsOfResourcesToReload.empty())
			{
				const size_t numberOfResourceManagers = mResourceManagers.size();
				for (uint32_t assetId : mAssetIdsOfResourcesToReload)
				{
					// Inform the individual resource manager instances
					for (size_t i = 0; i < numberOfResourceManagers; ++i)
					{
						mResourceManagers[i]->reloadResourceByAssetId(assetId);
					}
				}
				mAssetIdsOfResourcesToReload.clear();
			}
		}

		// Pipeline state compiler and resource streamer update
		mGraphicsPipelineStateCompiler->dispatch();
		mComputePipelineStateCompiler->dispatch();
		mResourceStreamer->dispatch();

		// Inform the individual resource manager instances
		const size_t numberOfResourceManagers = mResourceManagers.size();
		for (size_t i = 0; i < numberOfResourceManagers; ++i)
		{
			mResourceManagers[i]->update();
		}
		mRendererResourceManager->garbageCollection();
	}

	void RendererRuntimeImpl::clearPipelineStateObjectCache()
	{
		mShaderBlueprintResourceManager->clearPipelineStateObjectCache();
		mMaterialBlueprintResourceManager->clearPipelineStateObjectCache();
	}

	void RendererRuntimeImpl::loadPipelineStateObjectCache()
	{
		if (mRenderer->getCapabilities().shaderBytecode)
		{
			clearPipelineStateObjectCache();

			// Load file
			MemoryFile memoryFile;
			if (::detail::loadPipelineStateObjectCacheFile(*this, memoryFile))
			{
				mShaderBlueprintResourceManager->loadPipelineStateObjectCache(memoryFile);
				mMaterialBlueprintResourceManager->loadPipelineStateObjectCache(memoryFile);
			}
			else
			{
				// TODO(co) As soon as everything is in place, we might want to enable this assert
				// RENDERER_ASSERT(getContext(), false, "Renderer runtime is unable to load the pipeline state object cache. This will possibly result decreased runtime performance up to runtime hiccups. You might want to create the pipeline state object cache via the renderer toolkit.")
			}
		}
	}

	void RendererRuntimeImpl::savePipelineStateObjectCache()
	{
		// Do only save the pipeline state object cache if writing local data is allowed
		// -> We only support saving material blueprint based shader bytecodes, creating shaders without material blueprint is supposed to be only used for debugging and tiny shaders which are compiled at the very beginning of rendering
		if (mRenderer->getCapabilities().shaderBytecode && nullptr != mFileManager->getLocalDataMountPoint() && (mShaderBlueprintResourceManager->doesPipelineStateObjectCacheNeedSaving() || mMaterialBlueprintResourceManager->doesPipelineStateObjectCacheNeedSaving()))
		{
			MemoryFile memoryFile;
			mShaderBlueprintResourceManager->savePipelineStateObjectCache(memoryFile);
			mMaterialBlueprintResourceManager->savePipelineStateObjectCache(memoryFile);
			::detail::savePipelineStateObjectCacheFile(*this, memoryFile);
		}
	}


	//[-------------------------------------------------------]
	//[ Protected virtual Renderer::RefCount methods          ]
	//[-------------------------------------------------------]
	void RendererRuntimeImpl::selfDestruct()
	{
		RENDERER_DELETE(mRenderer->getContext(), RendererRuntimeImpl, this);
	}


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererRuntime
