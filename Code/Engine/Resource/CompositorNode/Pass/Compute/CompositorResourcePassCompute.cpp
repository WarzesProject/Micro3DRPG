#include "stdafx.h"
#include "Resource/CompositorNode/Pass/Compute/CompositorResourcePassCompute.h"
#include "Resource/CompositorNode/Loader/CompositorNodeFileFormat.h"
#include "Resource/Material/MaterialResourceManager.h"


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
namespace RendererRuntime
{


	//[-------------------------------------------------------]
	//[ Public methods                                        ]
	//[-------------------------------------------------------]
	CompositorResourcePassCompute::CompositorResourcePassCompute(const CompositorTarget& compositorTarget, AssetId materialBlueprintAssetId, const MaterialProperties& materialProperties) :
		ICompositorResourcePass(compositorTarget),
		mMaterialDefinitionMandatory(true),
		mMaterialTechniqueId(MaterialResourceManager::DEFAULT_MATERIAL_TECHNIQUE_ID),
		mMaterialBlueprintAssetId(materialBlueprintAssetId),
		mMaterialProperties(materialProperties)
	{
		// Nothing here
	}


	//[-------------------------------------------------------]
	//[ Public virtual RendererRuntime::ICompositorResourcePass methods ]
	//[-------------------------------------------------------]
	void CompositorResourcePassCompute::deserialize([[maybe_unused]] uint32_t numberOfBytes, const uint8_t* data)
	{
		// Sanity check
		assert(sizeof(v1CompositorNode::PassCompute) <= numberOfBytes);

		// Call the base implementation
		ICompositorResourcePass::deserialize(sizeof(v1CompositorNode::Pass), data);

		// Read data
		const v1CompositorNode::PassCompute* passCompute = reinterpret_cast<const v1CompositorNode::PassCompute*>(data);
		assert(sizeof(v1CompositorNode::PassCompute) + sizeof(MaterialProperty) * passCompute->numberOfMaterialProperties == numberOfBytes);
		mMaterialAssetId = passCompute->materialAssetId;
		mMaterialTechniqueId = passCompute->materialTechniqueId;
		mMaterialBlueprintAssetId = passCompute->materialBlueprintAssetId;

		{ // Read material properties
			// TODO(co) Get rid of the evil const-cast
			MaterialProperties::SortedPropertyVector& sortedPropertyVector = const_cast<MaterialProperties::SortedPropertyVector&>(mMaterialProperties.getSortedPropertyVector());
			sortedPropertyVector.resize(passCompute->numberOfMaterialProperties);
			memcpy(reinterpret_cast<char*>(sortedPropertyVector.data()), data + sizeof(v1CompositorNode::PassCompute), sizeof(MaterialProperty) * passCompute->numberOfMaterialProperties);
		}

		// Sanity checks
		assert(!mMaterialDefinitionMandatory || isValid(mMaterialAssetId) || isValid(mMaterialBlueprintAssetId));
		assert(!(isValid(mMaterialAssetId) && isValid(mMaterialBlueprintAssetId)));
	}


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererRuntime
