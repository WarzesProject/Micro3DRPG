#include "stdafx.h"
#include "Resource/CompositorNode/Pass/GenerateMipmaps/CompositorResourcePassGenerateMipmaps.h"
#include "Resource/CompositorNode/Loader/CompositorNodeFileFormat.h"


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
namespace RendererRuntime
{


	//[-------------------------------------------------------]
	//[ Public virtual RendererRuntime::ICompositorResourcePass methods ]
	//[-------------------------------------------------------]
	void CompositorResourcePassGenerateMipmaps::deserialize([[maybe_unused]] uint32_t numberOfBytes, const uint8_t* data)
	{
		// Sanity check
		assert(sizeof(v1CompositorNode::PassGenerateMipmaps) == numberOfBytes);

		// Call the base implementation
		ICompositorResourcePass::deserialize(sizeof(v1CompositorNode::Pass), data);

		// Read data
		const v1CompositorNode::PassGenerateMipmaps* passGenerateMipmaps = reinterpret_cast<const v1CompositorNode::PassGenerateMipmaps*>(data);
		mTextureAssetId = passGenerateMipmaps->textureAssetId;
		mMaterialBlueprintAssetId = passGenerateMipmaps->materialBlueprintAssetId;
		mTextureMaterialBlueprintProperty = passGenerateMipmaps->textureMaterialBlueprintProperty;

		// Sanity check
		assert((isInvalid(mMaterialBlueprintAssetId) && isInvalid(mTextureMaterialBlueprintProperty)) || (isValid(mMaterialBlueprintAssetId) && isValid(mTextureMaterialBlueprintProperty)));
	}


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererRuntime
