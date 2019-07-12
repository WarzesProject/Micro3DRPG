#include "stdafx.h"
#include "Resource/Scene/SceneResourceManager.h"
#include "Resource/Scene/SceneResource.h"
#include "Resource/Scene/Factory/SceneFactory.h"
#include "Resource/Scene/Loader/SceneResourceLoader.h"
#include "Resource/ResourceManagerTemplate.h"


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
		static const RendererRuntime::SceneFactory defaultSceneFactory;


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
	void SceneResourceManager::setSceneFactory(const ISceneFactory* sceneFactory)
	{
		// There must always be a valid scene factory instance
		mSceneFactory = (nullptr != sceneFactory) ? sceneFactory : &::detail::defaultSceneFactory;

		// Tell the scene resource instances about the new scene factory in town
		const uint32_t numberOfElements = mInternalResourceManager->getResources().getNumberOfElements();
		for (uint32_t i = 0; i < numberOfElements; ++i)
		{
			mInternalResourceManager->getResources().getElementByIndex(i).mSceneFactory = mSceneFactory;
		}
	}

	SceneResource* SceneResourceManager::getSceneResourceByAssetId(AssetId assetId) const
	{
		return mInternalResourceManager->getResourceByAssetId(assetId);
	}

	SceneResourceId SceneResourceManager::getSceneResourceIdByAssetId(AssetId assetId) const
	{
		const SceneResource* sceneResource = getSceneResourceByAssetId(assetId);
		return (nullptr != sceneResource) ? sceneResource->getId() : getInvalid<SceneResourceId>();
	}

	void SceneResourceManager::loadSceneResourceByAssetId(AssetId assetId, SceneResourceId& sceneResourceId, IResourceListener* resourceListener, bool reload, ResourceLoaderTypeId resourceLoaderTypeId)
	{
		mInternalResourceManager->loadResourceByAssetId(assetId, sceneResourceId, resourceListener, reload, resourceLoaderTypeId);
	}

	void SceneResourceManager::destroySceneResource(SceneResourceId sceneResourceId)
	{
		mInternalResourceManager->getResources().removeElement(sceneResourceId);
	}


	//[-------------------------------------------------------]
	//[ Public virtual RendererRuntime::IResourceManager methods ]
	//[-------------------------------------------------------]
	uint32_t SceneResourceManager::getNumberOfResources() const
	{
		return mInternalResourceManager->getResources().getNumberOfElements();
	}

	IResource& SceneResourceManager::getResourceByIndex(uint32_t index) const
	{
		return mInternalResourceManager->getResources().getElementByIndex(index);
	}

	IResource& SceneResourceManager::getResourceByResourceId(ResourceId resourceId) const
	{
		return mInternalResourceManager->getResources().getElementById(resourceId);
	}

	IResource* SceneResourceManager::tryGetResourceByResourceId(ResourceId resourceId) const
	{
		return mInternalResourceManager->getResources().tryGetElementById(resourceId);
	}

	void SceneResourceManager::reloadResourceByAssetId(AssetId assetId)
	{
		SceneResource* sceneResource = mInternalResourceManager->getResourceByAssetId(assetId);
		if (nullptr != sceneResource)
		{
			sceneResource->destroyAllSceneNodesAndItems();
		}
		return mInternalResourceManager->reloadResourceByAssetId(assetId);
	}


	//[-------------------------------------------------------]
	//[ Private virtual RendererRuntime::IResourceManager methods ]
	//[-------------------------------------------------------]
	IResourceLoader* SceneResourceManager::createResourceLoaderInstance(ResourceLoaderTypeId resourceLoaderTypeId)
	{
		return mInternalResourceManager->createResourceLoaderInstance(resourceLoaderTypeId);
	}


	//[-------------------------------------------------------]
	//[ Private methods                                       ]
	//[-------------------------------------------------------]
	SceneResourceManager::SceneResourceManager(IRendererRuntime& rendererRuntime) :
		mRendererRuntime(rendererRuntime),
		mSceneFactory(&::detail::defaultSceneFactory)
	{
		mInternalResourceManager = new ResourceManagerTemplate<SceneResource, SceneResourceLoader, SceneResourceId, 16>(rendererRuntime, *this);
	}

	SceneResourceManager::~SceneResourceManager()
	{
		delete mInternalResourceManager;
	}


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererRuntime
