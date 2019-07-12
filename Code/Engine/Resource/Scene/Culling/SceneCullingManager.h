#pragma once


//[-------------------------------------------------------]
//[ Includes                                              ]
//[-------------------------------------------------------]
#include "Core/Manager.h"
#include "Resource/CompositorWorkspace/CompositorWorkspaceInstance.h"


//[-------------------------------------------------------]
//[ Forward declarations                                  ]
//[-------------------------------------------------------]
namespace Renderer
{
	class IRenderTarget;
}
namespace RendererRuntime
{
	class ISceneItem;
	struct SceneItemSet;
	class CompositorContextData;
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
	*    Scene culling manager
	*
	*  @note
	*    - The implementation is basing on "The Implementation of Frustum Culling in Stingray" - http://bitsquid.blogspot.de/2016/10/the-implementation-of-frustum-culling.html
	*/
	class SceneCullingManager final : public Manager
	{


	//[-------------------------------------------------------]
	//[ Public definitions                                    ]
	//[-------------------------------------------------------]
	public:
		typedef std::vector<ISceneItem*> SceneItems;	// TODO(co) No raw-pointers (but no smart pointers either, use handles)


	//[-------------------------------------------------------]
	//[ Public methods                                        ]
	//[-------------------------------------------------------]
	public:
		SceneCullingManager();
		~SceneCullingManager();
		void gatherRenderQueueIndexRangesRenderableManagers(const Renderer::IRenderTarget& renderTarget, const CompositorContextData& compositorContextData, CompositorWorkspaceInstance::RenderQueueIndexRanges& renderQueueIndexRanges);

		[[nodiscard]] inline SceneItemSet& getCullableSceneItemSet() const
		{
			// We know that this pointer is always valid
			assert(nullptr != mCullableSceneItemSet);
			return *mCullableSceneItemSet;
		}

		[[nodiscard]] inline SceneItems& getUncullableSceneItems()
		{
			return mUncullableSceneItems;
		}


	//[-------------------------------------------------------]
	//[ Private methods                                       ]
	//[-------------------------------------------------------]
	private:
		explicit SceneCullingManager(const SceneCullingManager&) = delete;
		SceneCullingManager& operator=(const SceneCullingManager&) = delete;


	//[-------------------------------------------------------]
	//[ Private data                                          ]
	//[-------------------------------------------------------]
	private:
		SceneItemSet*		  mCullableSceneItemSet;				///< Cullable scene item set, always valid, destroy the instance if you no longer need it
		SceneItemSet*		  mCullableShadowCastersSceneItemSet;	///< Cullable shadow casters scene item set, always valid, destroy the instance if you no longer need it	TODO(co) Implement me
		SceneItems			  mUncullableSceneItems;				///< Scene items which can't be culled and hence are always considered to be visible
		std::vector<uint32_t> mIndirection;


	};


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererRuntime
