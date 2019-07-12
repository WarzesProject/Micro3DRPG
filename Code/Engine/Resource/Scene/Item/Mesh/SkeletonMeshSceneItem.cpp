#include "stdafx.h"
#include "Resource/Scene/Item/Mesh/SkeletonMeshSceneItem.h"
#include "Resource/Scene/Loader/SceneFileFormat.h"
#include "Resource/Scene/SceneResource.h"
#include "Resource/SkeletonAnimation/SkeletonAnimationController.h"
#include "Resource/Mesh/MeshResourceManager.h"
#include "Resource/Mesh/MeshResource.h"
#include "IRendererRuntime.h"


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
namespace RendererRuntime
{


	//[-------------------------------------------------------]
	//[ Public methods                                        ]
	//[-------------------------------------------------------]
	SkeletonResourceId SkeletonMeshSceneItem::getSkeletonResourceId() const
	{
		const MeshResource* meshResource = getSceneResource().getRendererRuntime().getMeshResourceManager().tryGetById(getMeshResourceId());
		return (nullptr != meshResource) ? meshResource->getSkeletonResourceId() : getInvalid<SkeletonResourceId>();
	}


	//[-------------------------------------------------------]
	//[ Public RendererRuntime::ISceneItem methods            ]
	//[-------------------------------------------------------]
	void SkeletonMeshSceneItem::deserialize(uint32_t numberOfBytes, const uint8_t* data)
	{
		// Sanity check
		RENDERER_ASSERT(getContext(), sizeof(v1Scene::SkeletonMeshItem) <= numberOfBytes, "Invalid number of bytes")
		const v1Scene::SkeletonMeshItem* skeletonMeshItem = reinterpret_cast<const v1Scene::SkeletonMeshItem*>(data);

		// Read data
		mSkeletonAnimationAssetId = skeletonMeshItem->skeletonAnimationAssetId;

		// Call base implementation
		MeshSceneItem::deserialize(numberOfBytes - sizeof(v1Scene::SkeletonMeshItem), data + sizeof(v1Scene::SkeletonMeshItem));
	}


	//[-------------------------------------------------------]
	//[ Protected methods                                     ]
	//[-------------------------------------------------------]
	SkeletonMeshSceneItem::~SkeletonMeshSceneItem()
	{
		// Destroy the skeleton animation controller instance, if needed
		delete mSkeletonAnimationController;
	}


	//[-------------------------------------------------------]
	//[ Protected virtual RendererRuntime::IResourceListener methods ]
	//[-------------------------------------------------------]
	void SkeletonMeshSceneItem::onLoadingStateChange(const IResource& resource)
	{
		// Create/destroy skeleton animation controller
		if (resource.getId() == getMeshResourceId())
		{
			if (resource.getLoadingState() == IResource::LoadingState::LOADED)
			{
				if (isValid(mSkeletonAnimationAssetId))
				{
					RENDERER_ASSERT(getContext(), nullptr == mSkeletonAnimationController, "Invalid skeleton animation controller")
					mSkeletonAnimationController = new SkeletonAnimationController(getSceneResource().getRendererRuntime(), static_cast<const MeshResource&>(resource).getSkeletonResourceId());
					mSkeletonAnimationController->startSkeletonAnimationByAssetId(mSkeletonAnimationAssetId);
				}
			}
			else if (nullptr != mSkeletonAnimationController)
			{
				delete mSkeletonAnimationController;
				mSkeletonAnimationController = nullptr;
			}
		}

		// Call the base implementation
		MeshSceneItem::onLoadingStateChange(resource);
	}


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererRuntime
