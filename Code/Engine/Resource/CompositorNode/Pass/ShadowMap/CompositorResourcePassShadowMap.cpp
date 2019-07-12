#include "stdafx.h"
#include "Resource/CompositorNode/Pass/ShadowMap/CompositorResourcePassShadowMap.h"
#include "Resource/CompositorNode/Loader/CompositorNodeFileFormat.h"


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
namespace RendererRuntime
{


	//[-------------------------------------------------------]
	//[ Public virtual RendererRuntime::ICompositorResourcePass methods ]
	//[-------------------------------------------------------]
	void CompositorResourcePassShadowMap::deserialize([[maybe_unused]] uint32_t numberOfBytes, const uint8_t* data)
	{
		// Sanity check
		assert(sizeof(v1CompositorNode::PassShadowMap) == numberOfBytes);

		// Call the base implementation
		CompositorResourcePassScene::deserialize(sizeof(v1CompositorNode::PassScene), data);

		// Read data
		const v1CompositorNode::PassShadowMap* passShadowMap = reinterpret_cast<const v1CompositorNode::PassShadowMap*>(data);
		mTextureAssetId = passShadowMap->textureAssetId;
		mDepthToExponentialVarianceMaterialBlueprintAssetId = passShadowMap->depthToExponentialVarianceMaterialBlueprintAssetId;
		mBlurMaterialBlueprintAssetId = passShadowMap->blurMaterialBlueprintAssetId;
	}


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererRuntime
