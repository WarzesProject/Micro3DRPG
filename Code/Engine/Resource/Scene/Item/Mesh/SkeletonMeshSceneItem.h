#pragma once


//[-------------------------------------------------------]
//[ Includes                                              ]
//[-------------------------------------------------------]
#include "Resource/Scene/Item/Mesh/MeshSceneItem.h"


//[-------------------------------------------------------]
//[ Forward declarations                                  ]
//[-------------------------------------------------------]
namespace RendererRuntime
{
	class SkeletonAnimationController;
}


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
namespace RendererRuntime
{


	//[-------------------------------------------------------]
	//[ Global definitions                                    ]
	//[-------------------------------------------------------]
	typedef uint32_t SkeletonResourceId;	///< POD skeleton resource identifier


	//[-------------------------------------------------------]
	//[ Classes                                               ]
	//[-------------------------------------------------------]
	/**
	*  @brief
	*    Skeleton mesh scene item class
	*/
	class SkeletonMeshSceneItem final : public MeshSceneItem
	{


	//[-------------------------------------------------------]
	//[ Friends                                               ]
	//[-------------------------------------------------------]
		friend class SceneFactory;	// Needs to be able to create scene item instances


	//[-------------------------------------------------------]
	//[ Public definitions                                    ]
	//[-------------------------------------------------------]
	public:
		static constexpr uint32_t TYPE_ID = STRING_ID("SkeletonMeshSceneItem");


	//[-------------------------------------------------------]
	//[ Public methods                                        ]
	//[-------------------------------------------------------]
	public:
		[[nodiscard]] inline AssetId getSkeletonAnimationAssetId() const
		{
			return mSkeletonAnimationAssetId;
		}

		[[nodiscard]] RENDERERRUNTIME_API_EXPORT SkeletonResourceId getSkeletonResourceId() const;


	//[-------------------------------------------------------]
	//[ Public RendererRuntime::ISceneItem methods            ]
	//[-------------------------------------------------------]
	public:
		[[nodiscard]] inline virtual SceneItemTypeId getSceneItemTypeId() const override
		{
			return TYPE_ID;
		}

		virtual void deserialize(uint32_t numberOfBytes, const uint8_t* data) override;


	//[-------------------------------------------------------]
	//[ Protected methods                                     ]
	//[-------------------------------------------------------]
	protected:
		inline explicit SkeletonMeshSceneItem(SceneResource& sceneResource) :
			MeshSceneItem(sceneResource),
			mSkeletonAnimationController(nullptr)
		{
			// Nothing here
		}

		virtual ~SkeletonMeshSceneItem() override;
		explicit SkeletonMeshSceneItem(const SkeletonMeshSceneItem&) = delete;
		SkeletonMeshSceneItem& operator=(const SkeletonMeshSceneItem&) = delete;


	//[-------------------------------------------------------]
	//[ Protected virtual RendererRuntime::IResourceListener methods ]
	//[-------------------------------------------------------]
	protected:
		virtual void onLoadingStateChange(const IResource& resource) override;


	//[-------------------------------------------------------]
	//[ Private data                                          ]
	//[-------------------------------------------------------]
	private:
		AssetId						 mSkeletonAnimationAssetId;		///< Skeleton animation asset ID, can be set to invalid value
		SkeletonAnimationController* mSkeletonAnimationController;	///< Skeleton animation controller instance, can be a null pointer, destroy the instance if you no longer need it


	};


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererRuntime
