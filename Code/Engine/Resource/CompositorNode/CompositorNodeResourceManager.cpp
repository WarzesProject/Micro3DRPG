#include "stdafx.h"
#include "Resource/CompositorNode/CompositorNodeResourceManager.h"
#include "Resource/CompositorNode/CompositorNodeResource.h"
#include "Resource/CompositorNode/Pass/CompositorPassFactory.h"
#include "Resource/CompositorNode/Loader/CompositorNodeResourceLoader.h"
#include "Resource/CompositorWorkspace/CompositorWorkspaceResourceManager.h"
#include "Resource/CompositorWorkspace/CompositorWorkspaceResource.h"
#include "Resource/ResourceManagerTemplate.h"
#include "Core/Renderer/RenderTargetTextureManager.h"
#include "Core/Renderer/FramebufferManager.h"
#include "Core/Renderer/RenderPassManager.h"


//[-------------------------------------------------------]
//[ Anonymous detail namespace                            ]
//[-------------------------------------------------------]
namespace
{
	namespace detail
	{


		//[-------------------------------------------------------]
		//[ Global variables                                      ]
		//[-------------------------------------------------------]
		static const RendererRuntime::CompositorPassFactory defaultCompositorPassFactory;


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
	//[ Public methods                                        ]
	//[-------------------------------------------------------]
	void CompositorNodeResourceManager::loadCompositorNodeResourceByAssetId(AssetId assetId, CompositorNodeResourceId& compositorNodeResourceId, IResourceListener* resourceListener, bool reload, ResourceLoaderTypeId resourceLoaderTypeId)
	{
		mInternalResourceManager->loadResourceByAssetId(assetId, compositorNodeResourceId, resourceListener, reload, resourceLoaderTypeId);
	}

	void CompositorNodeResourceManager::setCompositorPassFactory(const ICompositorPassFactory* compositorPassFactory)
	{
		// There must always be a valid compositor pass factory instance
		mCompositorPassFactory = (nullptr != compositorPassFactory) ? compositorPassFactory : &::detail::defaultCompositorPassFactory;
	}


	//[-------------------------------------------------------]
	//[ Public virtual RendererRuntime::IResourceManager methods ]
	//[-------------------------------------------------------]
	uint32_t CompositorNodeResourceManager::getNumberOfResources() const
	{
		return mInternalResourceManager->getResources().getNumberOfElements();
	}

	IResource& CompositorNodeResourceManager::getResourceByIndex(uint32_t index) const
	{
		return mInternalResourceManager->getResources().getElementByIndex(index);
	}

	IResource& CompositorNodeResourceManager::getResourceByResourceId(ResourceId resourceId) const
	{
		return mInternalResourceManager->getResources().getElementById(resourceId);
	}

	IResource* CompositorNodeResourceManager::tryGetResourceByResourceId(ResourceId resourceId) const
	{
		return mInternalResourceManager->getResources().tryGetElementById(resourceId);
	}

	void CompositorNodeResourceManager::reloadResourceByAssetId(AssetId assetId)
	{
		// TODO(co) Experimental implementation (take care of resource cleanup etc.)
		const uint32_t numberOfElements = mInternalResourceManager->getResources().getNumberOfElements();
		for (uint32_t i = 0; i < numberOfElements; ++i)
		{
			const CompositorNodeResource& compositorNodeResource = mInternalResourceManager->getResources().getElementByIndex(i);
			if (compositorNodeResource.getAssetId() == assetId)
			{
				CompositorNodeResourceId compositorNodeResourceId = getInvalid<CompositorNodeResourceId>();
				loadCompositorNodeResourceByAssetId(assetId, compositorNodeResourceId, nullptr, true, compositorNodeResource.getResourceLoaderTypeId());

				{ // Reload all compositor workspace resources using this compositor node resource
					CompositorWorkspaceResourceManager& compositorWorkspaceResourceManager = mRendererRuntime.getCompositorWorkspaceResourceManager();
					const uint32_t numberOfCompositorWorkspaceResources = compositorWorkspaceResourceManager.getNumberOfResources();
					for (uint32_t compositorWorkspaceResourceIndex = 0; compositorWorkspaceResourceIndex < numberOfCompositorWorkspaceResources; ++compositorWorkspaceResourceIndex)
					{
						const CompositorWorkspaceResource& compositorWorkspaceResource = compositorWorkspaceResourceManager.getByIndex(compositorWorkspaceResourceIndex);
						const CompositorWorkspaceResource::CompositorNodeAssetIds& compositorNodeAssetIds = compositorWorkspaceResource.getCompositorNodeAssetIds();
						for (AssetId currentAssetId : compositorNodeAssetIds)
						{
							if (currentAssetId == assetId)
							{
								compositorWorkspaceResourceManager.reloadResourceByAssetId(compositorWorkspaceResource.getAssetId());
								break;
							}
						}
					}
				}

				break;
			}
		}
	}


	//[-------------------------------------------------------]
	//[ Private virtual RendererRuntime::IResourceManager methods ]
	//[-------------------------------------------------------]
	IResourceLoader* CompositorNodeResourceManager::createResourceLoaderInstance(ResourceLoaderTypeId resourceLoaderTypeId)
	{
		return mInternalResourceManager->createResourceLoaderInstance(resourceLoaderTypeId);
	}


	//[-------------------------------------------------------]
	//[ Private methods                                       ]
	//[-------------------------------------------------------]
	CompositorNodeResourceManager::CompositorNodeResourceManager(IRendererRuntime& rendererRuntime) :
		mRendererRuntime(rendererRuntime),
		mCompositorPassFactory(&::detail::defaultCompositorPassFactory),
		mRenderTargetTextureManager(new RenderTargetTextureManager(mRendererRuntime)),
		mRenderPassManager(new RenderPassManager(mRendererRuntime.getRenderer())),
		mFramebufferManager(new FramebufferManager(*mRenderTargetTextureManager, *mRenderPassManager))
	{
		mInternalResourceManager = new ResourceManagerTemplate<CompositorNodeResource, CompositorNodeResourceLoader, CompositorNodeResourceId, 32>(rendererRuntime, *this);
	}

	CompositorNodeResourceManager::~CompositorNodeResourceManager()
	{
		delete mFramebufferManager;
		delete mRenderPassManager;
		delete mRenderTargetTextureManager;
		delete mInternalResourceManager;
	}


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererRuntime
