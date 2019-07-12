#include "stdafx.h"
#include "Resource/Scene/SceneResource.h"
#include "Resource/Scene/SceneNode.h"
#include "Resource/Scene/SceneResourceManager.h"
#include "Resource/Scene/Item/ISceneItem.h"
#include "Resource/Scene/Factory/ISceneFactory.h"
#include "Resource/Scene/Culling/SceneCullingManager.h"
#include "IRendererRuntime.h"


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
namespace RendererRuntime
{


	//[-------------------------------------------------------]
	//[ Public methods                                        ]
	//[-------------------------------------------------------]
	IRendererRuntime& SceneResource::getRendererRuntime() const
	{
		return getResourceManager<SceneResourceManager>().getRendererRuntime();
	}

	void SceneResource::destroyAllSceneNodesAndItems()
	{
		destroyAllSceneNodes();
		destroyAllSceneItems();

		// TODO(co) The following is just for culling kickoff and won't stay this way
		// Re-create scene culling manager
		delete mSceneCullingManager;
		mSceneCullingManager = new SceneCullingManager();

		// Update the resource loading state
		setLoadingState(IResource::LoadingState::UNLOADED);
	}

	SceneNode* SceneResource::createSceneNode(const Transform& transform)
	{
		SceneNode* sceneNode = new SceneNode(transform);
		mSceneNodes.push_back(sceneNode);
		return sceneNode;
	}

	void SceneResource::destroySceneNode(SceneNode& sceneNode)
	{
		SceneNodes::iterator iterator = std::find(mSceneNodes.begin(), mSceneNodes.end(), &sceneNode);
		if (iterator != mSceneNodes.end())
		{
			mSceneNodes.erase(iterator);
			delete &sceneNode;
		}
		else
		{
			// TODO(co) Error handling
		}
	}

	void SceneResource::destroyAllSceneNodes()
	{
		const size_t numberOfSceneNodes = mSceneNodes.size();
		for (size_t i = 0; i < numberOfSceneNodes; ++i)
		{
			delete mSceneNodes[i];
		}
		mSceneNodes.clear();
	}

	ISceneItem* SceneResource::createSceneItem(SceneItemTypeId sceneItemTypeId, SceneNode& sceneNode)
	{
		if (nullptr == mSceneFactory)
		{
			mSceneFactory = &getResourceManager<SceneResourceManager>().getSceneFactory();
		}
		assert(nullptr != mSceneFactory);
		ISceneItem* sceneItem = mSceneFactory->createSceneItem(sceneItemTypeId, *this);
		if (nullptr != sceneItem)
		{
			sceneNode.attachSceneItem(*sceneItem);
			mSceneItems.push_back(sceneItem);
		}
		else
		{
			// TODO(co) Error handling
		}
		return sceneItem;
	}

	void SceneResource::destroySceneItem(ISceneItem& sceneItem)
	{
		SceneItems::iterator iterator = std::find(mSceneItems.begin(), mSceneItems.end(), &sceneItem);
		if (iterator != mSceneItems.end())
		{
			mSceneItems.erase(iterator);
			delete &sceneItem;
		}
		else
		{
			// TODO(co) Error handling
		}
	}

	void SceneResource::destroyAllSceneItems()
	{
		const size_t numberOfSceneItems = mSceneItems.size();
		for (size_t i = 0; i < numberOfSceneItems; ++i)
		{
			delete mSceneItems[i];
		}
		mSceneItems.clear();
	}


	//[-------------------------------------------------------]
	//[ Private methods                                       ]
	//[-------------------------------------------------------]
	void SceneResource::initializeElement(SceneResourceId sceneResourceId)
	{
		// Sanity checks
		assert(nullptr == mSceneFactory);
		assert(nullptr == mSceneCullingManager);
		assert(mSceneNodes.empty());
		assert(mSceneItems.empty());

		// Create scene culling manager
		mSceneCullingManager = new SceneCullingManager();

		// Call base implementation
		IResource::initializeElement(sceneResourceId);
	}

	void SceneResource::deinitializeElement()
	{
		// Reset everything
		destroyAllSceneNodesAndItems();
		mSceneFactory = nullptr;
		delete mSceneCullingManager;
		mSceneCullingManager = nullptr;

		// Call base implementation
		IResource::deinitializeElement();
	}


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererRuntime
