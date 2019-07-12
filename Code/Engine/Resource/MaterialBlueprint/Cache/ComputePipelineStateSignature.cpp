#include "stdafx.h"
#include "Resource/MaterialBlueprint/Cache/ComputePipelineStateSignature.h"
#include "Resource/MaterialBlueprint/Cache/GraphicsPipelineStateSignature.h"
#include "Resource/MaterialBlueprint/MaterialBlueprintResourceManager.h"
#include "Resource/MaterialBlueprint/MaterialBlueprintResource.h"
#include "Resource/ShaderBlueprint/ShaderBlueprintResourceManager.h"
#include "Resource/ShaderBlueprint/ShaderBlueprintResource.h"
#include "Core/Math/Math.h"
#include "IRendererRuntime.h"


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
namespace RendererRuntime
{


	//[-------------------------------------------------------]
	//[ Public methods                                        ]
	//[-------------------------------------------------------]
	ComputePipelineStateSignature::ComputePipelineStateSignature(const ComputePipelineStateSignature& computePipelineStateSignature) :
		mMaterialBlueprintResourceId(computePipelineStateSignature.mMaterialBlueprintResourceId),
		mShaderProperties(computePipelineStateSignature.mShaderProperties),
		mComputePipelineStateSignatureId(computePipelineStateSignature.mComputePipelineStateSignatureId),
		mShaderCombinationId(computePipelineStateSignature.mShaderCombinationId)
	{
		// Nothing here
	}

	ComputePipelineStateSignature& ComputePipelineStateSignature::operator=(const ComputePipelineStateSignature& computePipelineStateSignature)
	{
		mMaterialBlueprintResourceId = computePipelineStateSignature.mMaterialBlueprintResourceId;
		mShaderProperties = computePipelineStateSignature.mShaderProperties;
		mComputePipelineStateSignatureId = computePipelineStateSignature.mComputePipelineStateSignatureId;
		mShaderCombinationId = computePipelineStateSignature.mShaderCombinationId;

		// Done
		return *this;
	}

	void ComputePipelineStateSignature::set(const MaterialBlueprintResource& materialBlueprintResource, const ShaderProperties& shaderProperties)
	{
		mMaterialBlueprintResourceId		= materialBlueprintResource.getId();
		mShaderProperties					= shaderProperties;
		mComputePipelineStateSignatureId	= Math::FNV1a_INITIAL_HASH_32;
		mShaderCombinationId				= getInvalid<ShaderCombinationId>();

		// Incorporate primitive hashes
		mComputePipelineStateSignatureId = Math::calculateFNV1a32(reinterpret_cast<const uint8_t*>(&mMaterialBlueprintResourceId), sizeof(uint32_t), mComputePipelineStateSignatureId);

		// Incorporate shader related hash
		const ShaderBlueprintResource* shaderBlueprintResource = materialBlueprintResource.getResourceManager<MaterialBlueprintResourceManager>().getRendererRuntime().getShaderBlueprintResourceManager().tryGetById(materialBlueprintResource.getComputeShaderBlueprintResourceId());
		if (nullptr != shaderBlueprintResource)
		{
			const uint32_t hash = mShaderCombinationId = GraphicsPipelineStateSignature::generateShaderCombinationId(*shaderBlueprintResource, mShaderProperties);
			mComputePipelineStateSignatureId = Math::calculateFNV1a32(reinterpret_cast<const uint8_t*>(&hash), sizeof(uint32_t), mComputePipelineStateSignatureId);
		}
	}


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererRuntime
