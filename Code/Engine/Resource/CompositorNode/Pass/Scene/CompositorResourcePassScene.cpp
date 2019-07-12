#include "stdafx.h"
#include "Resource/CompositorNode/Pass/Scene/CompositorResourcePassScene.h"
#include "Resource/CompositorNode/Loader/CompositorNodeFileFormat.h"


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
namespace RendererRuntime
{


	//[-------------------------------------------------------]
	//[ Public virtual RendererRuntime::ICompositorResourcePass methods ]
	//[-------------------------------------------------------]
	void CompositorResourcePassScene::deserialize([[maybe_unused]] uint32_t numberOfBytes, const uint8_t* data)
	{
		// Sanity check
		assert(sizeof(v1CompositorNode::PassScene) == numberOfBytes);

		// Call the base implementation
		ICompositorResourcePass::deserialize(sizeof(v1CompositorNode::Pass), data);

		// Read data
		const v1CompositorNode::PassScene* passScene = reinterpret_cast<const v1CompositorNode::PassScene*>(data);
		mMinimumRenderQueueIndex = passScene->minimumRenderQueueIndex;
		mMaximumRenderQueueIndex = passScene->maximumRenderQueueIndex;
		mTransparentPass		 = passScene->transparentPass;
		mMaterialTechniqueId	 = passScene->materialTechniqueId;

		// Sanity check
		assert(mMaximumRenderQueueIndex >= mMinimumRenderQueueIndex);
	}

	bool CompositorResourcePassScene::getRenderQueueIndexRange(uint8_t& minimumRenderQueueIndex, uint8_t& maximumRenderQueueIndex) const
	{
		// This compositor resource pass has a render queue range defined
		minimumRenderQueueIndex = mMinimumRenderQueueIndex;
		maximumRenderQueueIndex = mMaximumRenderQueueIndex;
		return true;
	}


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererRuntime
