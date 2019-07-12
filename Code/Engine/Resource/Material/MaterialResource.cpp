#include "stdafx.h"
#include "Resource/Material/MaterialResource.h"
#include "Resource/Material/MaterialTechnique.h"
#include "Resource/Material/MaterialResourceManager.h"
#include "RenderQueue/RenderableManager.h"
#include "IRendererRuntime.h"

#include <algorithm>


//[-------------------------------------------------------]
//[ Anonymous detail namespace                            ]
//[-------------------------------------------------------]
namespace
{
	namespace detail
	{


		//[-------------------------------------------------------]
		//[ Structures                                            ]
		//[-------------------------------------------------------]
		struct OrderByMaterialResourceId final
		{
			[[nodiscard]] inline bool operator()(RendererRuntime::MaterialResourceId left, RendererRuntime::MaterialResourceId right) const
			{
				return (left < right);
			}
		};

		struct OrderByMaterialTechniqueId final
		{
			[[nodiscard]] inline bool operator()(const RendererRuntime::MaterialTechnique* left, RendererRuntime::MaterialTechniqueId right) const
			{
				return (left->getMaterialTechniqueId() < right);
			}

			[[nodiscard]] inline bool operator()(RendererRuntime::MaterialTechniqueId left, const RendererRuntime::MaterialTechnique* right) const
			{
				return (left < right->getMaterialTechniqueId());
			}
		};


//[-------------------------------------------------------]
//[ Anonymous detail namespace                            ]
//[-------------------------------------------------------]
	} // detail
}


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
namespace RendererRuntime
{


	//[-------------------------------------------------------]
	//[ Public methods                                        ]
	//[-------------------------------------------------------]
	Context& MaterialResource::getContext() const
	{
		return getResourceManager<MaterialResourceManager>().getRendererRuntime().getContext();
	}

	void MaterialResource::setParentMaterialResourceId(MaterialResourceId parentMaterialResourceId)
	{
		if (mParentMaterialResourceId != parentMaterialResourceId)
		{
			const MaterialResourceId materialResourceId = getId();

			// Destroy all material techniques
			destroyAllMaterialTechniques();

			// Unregister from previous parent material resource
			const MaterialResourceManager& materialResourceManager = getResourceManager<MaterialResourceManager>();
			if (isValid(mParentMaterialResourceId))
			{
				MaterialResource& parentMaterialResource = materialResourceManager.getById(mParentMaterialResourceId);
				SortedChildMaterialResourceIds::const_iterator iterator = std::lower_bound(parentMaterialResource.mSortedChildMaterialResourceIds.cbegin(), parentMaterialResource.mSortedChildMaterialResourceIds.cend(), materialResourceId, ::detail::OrderByMaterialResourceId());
				RENDERER_ASSERT(getContext(), iterator != parentMaterialResource.mSortedChildMaterialResourceIds.end() && *iterator == materialResourceId, "Invalid material resource ID")
				parentMaterialResource.mSortedChildMaterialResourceIds.erase(iterator);
			}

			// Set new parent material resource ID
			mParentMaterialResourceId = parentMaterialResourceId;
			if (isValid(mParentMaterialResourceId))
			{
				// Register to new parent material resource
				MaterialResource& parentMaterialResource = materialResourceManager.getById(mParentMaterialResourceId);
				RENDERER_ASSERT(getContext(), parentMaterialResource.getLoadingState() == IResource::LoadingState::LOADED, "Invalid parent material resource loading state")
				SortedChildMaterialResourceIds::const_iterator iterator = std::lower_bound(parentMaterialResource.mSortedChildMaterialResourceIds.cbegin(), parentMaterialResource.mSortedChildMaterialResourceIds.cend(), materialResourceId, ::detail::OrderByMaterialResourceId());
				RENDERER_ASSERT(getContext(), iterator == parentMaterialResource.mSortedChildMaterialResourceIds.end() || *iterator != materialResourceId, "Invalid material resource ID")
				parentMaterialResource.mSortedChildMaterialResourceIds.insert(iterator, materialResourceId);

				// Setup material resource
				setAssetId(parentMaterialResource.getAssetId());
				mMaterialProperties = parentMaterialResource.mMaterialProperties;
				for (MaterialTechnique* materialTechnique : parentMaterialResource.mSortedMaterialTechniqueVector)
				{
					mSortedMaterialTechniqueVector.push_back(new MaterialTechnique(materialTechnique->getMaterialTechniqueId(), *this, materialTechnique->getMaterialBlueprintResourceId()));
				}
			}
			else
			{
				// Don't touch the child material resources, but reset everything else
				mMaterialProperties.removeAllProperties();
			}
		}
	}

	MaterialTechnique* MaterialResource::getMaterialTechniqueById(MaterialTechniqueId materialTechniqueId) const
	{
		SortedMaterialTechniqueVector::const_iterator iterator = std::lower_bound(mSortedMaterialTechniqueVector.cbegin(), mSortedMaterialTechniqueVector.cend(), materialTechniqueId, ::detail::OrderByMaterialTechniqueId());
		return (iterator != mSortedMaterialTechniqueVector.end() && (*iterator)->getMaterialTechniqueId() == materialTechniqueId) ? *iterator : nullptr;
	}
	
	void MaterialResource::destroyAllMaterialTechniques()
	{
		for (MaterialTechnique* materialTechnique : mSortedMaterialTechniqueVector)
		{
			delete materialTechnique;
		}
		mSortedMaterialTechniqueVector.clear();
	}

	void MaterialResource::releaseTextures()
	{
		// TODO(co) Cleanup
		for (MaterialTechnique* materialTechnique : mSortedMaterialTechniqueVector)
		{
			materialTechnique->clearTextures();
		}
	}


	//[-------------------------------------------------------]
	//[ Private methods                                       ]
	//[-------------------------------------------------------]
	MaterialResource::~MaterialResource()
	{
		// Sanity checks
		RENDERER_ASSERT(getContext(), isInvalid(mParentMaterialResourceId), "Invalid parent material resource ID")
		RENDERER_ASSERT(getContext(), mSortedChildMaterialResourceIds.empty(), "Invalid sorted child material resource IDs")
		RENDERER_ASSERT(getContext(), mSortedMaterialTechniqueVector.empty(), "Invalid sorted material technique vector")
		RENDERER_ASSERT(getContext(), mMaterialProperties.getSortedPropertyVector().empty(), "Invalid material properties")
		RENDERER_ASSERT(getContext(), mAttachedRenderables.empty(), "Invalid attached renderables")

		// Avoid crash in case of failed sanity check
		while (!mAttachedRenderables.empty())
		{
			mAttachedRenderables[0]->unsetMaterialResourceId();
		}
	}

	MaterialResource& MaterialResource::operator=(MaterialResource&& materialResource)
	{
		// Call base implementation
		IResource::operator=(std::move(materialResource));

		// Swap data
		// -> Lucky us that we're usually not referencing by using raw-pointers, so a simple swap does the trick
		std::swap(mParentMaterialResourceId,	   materialResource.mParentMaterialResourceId);
		std::swap(mSortedChildMaterialResourceIds, materialResource.mSortedChildMaterialResourceIds);
		std::swap(mSortedMaterialTechniqueVector,  materialResource.mSortedMaterialTechniqueVector);
		std::swap(mMaterialProperties,			   materialResource.mMaterialProperties);
		std::swap(mAttachedRenderables,			   materialResource.mAttachedRenderables);

		// Done
		return *this;
	}

	void MaterialResource::deinitializeElement()
	{
		// Sanity check
		RENDERER_ASSERT(getContext(), mAttachedRenderables.empty(), "Invalid attached renderables")

		// Avoid crash in case of failed sanity check
		while (!mAttachedRenderables.empty())
		{
			mAttachedRenderables[0]->unsetMaterialResourceId();
		}

		// Unset parent material resource ID
		setParentMaterialResourceId(getInvalid<MaterialResourceId>());

		// Inform child material resources, if required
		if (!mSortedChildMaterialResourceIds.empty())
		{
			const MaterialResourceManager& materialResourceManager = static_cast<MaterialResourceManager&>(getResourceManager());
			while (!mSortedChildMaterialResourceIds.empty())
			{
				const MaterialResourceId materialResourceId = mSortedChildMaterialResourceIds.front();
				materialResourceManager.getById(materialResourceId).setParentMaterialResourceId(getInvalid<MaterialResourceId>());
			}
			mSortedChildMaterialResourceIds.clear();
		}

		// Cleanup
		destroyAllMaterialTechniques();
		mMaterialProperties.removeAllProperties();

		// Call base implementation
		IResource::deinitializeElement();
	}

	bool MaterialResource::setPropertyByIdInternal(MaterialPropertyId materialPropertyId, const MaterialPropertyValue& materialPropertyValue, MaterialProperty::Usage materialPropertyUsage, bool changeOverwrittenState)
	{
		// Call the base implementation
		MaterialProperty* materialProperty = mMaterialProperties.setPropertyById(materialPropertyId, materialPropertyValue, materialPropertyUsage, changeOverwrittenState);
		if (nullptr != materialProperty)
		{
			// Perform derived work, if required to do so
			switch (materialProperty->getUsage())
			{
				case MaterialProperty::Usage::SHADER_UNIFORM:
					for (MaterialTechnique* materialTechnique : mSortedMaterialTechniqueVector)
					{
						materialTechnique->scheduleForShaderUniformUpdate();
					}
					break;

				case MaterialProperty::Usage::SHADER_COMBINATION:
					// Handled by "RendererRuntime::MaterialProperties::setPropertyById()"
					break;

				case MaterialProperty::Usage::RASTERIZER_STATE:
				case MaterialProperty::Usage::DEPTH_STENCIL_STATE:
				case MaterialProperty::Usage::BLEND_STATE:
					// TODO(co) Optimization: The calculation of the FNV1a hash of "Renderer::SerializedGraphicsPipelineState" is pretty fast, but maybe it makes sense to schedule the calculation in case many material properties are changed in a row?
					for (MaterialTechnique* materialTechnique : mSortedMaterialTechniqueVector)
					{
						materialTechnique->calculateSerializedGraphicsPipelineStateHash();
					}
					break;

				case MaterialProperty::Usage::TEXTURE_REFERENCE:
					for (MaterialTechnique* materialTechnique : mSortedMaterialTechniqueVector)
					{
						materialTechnique->clearTextures();
					}
					break;

				case MaterialProperty::Usage::STATIC:
					// Initial cached material data gathering is performed inside "RendererRuntime::Renderable::setMaterialResourceId()"

					// Optional "RenderQueueIndex" (e.g. compositor materials usually don't need this property)
					if (RENDER_QUEUE_INDEX_PROPERTY_ID == materialPropertyId)
					{
						const int renderQueueIndex = materialProperty->getIntegerValue();

						// Sanity checks
						RENDERER_ASSERT(getContext(), renderQueueIndex >= 0, "Invalid render queue index")
						RENDERER_ASSERT(getContext(), renderQueueIndex <= 255, "Invalid render queue index")

						// Update the cached material data of all attached renderables
						for (Renderable* renderable : mAttachedRenderables)
						{
							renderable->mRenderQueueIndex = static_cast<uint8_t>(renderQueueIndex);

							// In here we don't care about the fact that one and the same renderable manager instance might
							// update cached renderables data. It's not performance critical in here and resolving this will
							// require additional logic which itself has an performance impact. So keep it simple.
							renderable->getRenderableManager().updateCachedRenderablesData();
						}
					}

					// Optional "CastShadows" (e.g. compositor materials usually don't need this property)
					else if (CAST_SHADOWS_PROPERTY_ID == materialPropertyId)
					{
						// Update the cached material data of all attached renderables
						const bool castShadows = materialProperty->getBooleanValue();
						for (Renderable* renderable : mAttachedRenderables)
						{
							renderable->mCastShadows = castShadows;

							// In here we don't care about the fact that one and the same renderable manager instance might
							// update cached renderables data. It's not performance critical in here and resolving this will
							// require additional logic which itself has an performance impact. So keep it simple.
							renderable->getRenderableManager().updateCachedRenderablesData();
						}
					}
					break;

				case MaterialProperty::Usage::UNKNOWN:
				case MaterialProperty::Usage::SAMPLER_STATE:
				case MaterialProperty::Usage::GLOBAL_REFERENCE:
				case MaterialProperty::Usage::UNKNOWN_REFERENCE:
				case MaterialProperty::Usage::PASS_REFERENCE:
				case MaterialProperty::Usage::MATERIAL_REFERENCE:
				case MaterialProperty::Usage::INSTANCE_REFERENCE:
				case MaterialProperty::Usage::GLOBAL_REFERENCE_FALLBACK:
				default:
					// Nothing here
					break;
			}

			// Inform child material resources, if required
			const MaterialResourceManager& materialResourceManager = static_cast<MaterialResourceManager&>(getResourceManager());
			for (MaterialResourceId materialResourceId : mSortedChildMaterialResourceIds)
			{
				materialResourceManager.getById(materialResourceId).setPropertyByIdInternal(materialPropertyId, materialPropertyValue, materialPropertyUsage, false);
			}

			// Material property change detected
			return true;
		}

		// No material property change detected
		return false;
	}


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererRuntime
