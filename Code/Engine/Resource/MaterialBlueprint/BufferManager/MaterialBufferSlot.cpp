#include "stdafx.h"
#include "Resource/MaterialBlueprint/BufferManager/MaterialBufferSlot.h"
#include "Resource/Material/MaterialResourceManager.h"
#include "Resource/Material/MaterialResource.h"


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
namespace RendererRuntime
{


	//[-------------------------------------------------------]
	//[ Public methods                                        ]
	//[-------------------------------------------------------]
	MaterialBufferSlot::MaterialBufferSlot(MaterialResource& materialResource) :
		mMaterialResourceManager(&materialResource.getResourceManager<MaterialResourceManager>()),
		mMaterialResourceId(materialResource.getId()),
		mAssignedMaterialPool(nullptr),
		mAssignedMaterialSlot(getInvalid<uint32_t>()),
		mGlobalIndex(getInvalid<int>()),
		mDirty(false)
	{
		// Nothing here
	}

	const MaterialResource& MaterialBufferSlot::getMaterialResource() const
	{
		return mMaterialResourceManager->getById(mMaterialResourceId);
	}


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererRuntime
