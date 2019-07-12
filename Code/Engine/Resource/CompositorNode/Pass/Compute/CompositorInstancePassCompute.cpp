#include "stdafx.h"
#include "Resource/CompositorNode/Pass/Compute/CompositorInstancePassCompute.h"
#include "Resource/CompositorNode/Pass/Compute/CompositorResourcePassCompute.h"
#include "Resource/CompositorNode/CompositorNodeInstance.h"
#include "Resource/CompositorWorkspace/CompositorWorkspaceInstance.h"
#include "Resource/MaterialBlueprint/MaterialBlueprintResourceManager.h"
#include "Resource/Material/MaterialResourceManager.h"
#include "Resource/Material/MaterialResource.h"
#include "Core/IProfiler.h"
#include "IRendererRuntime.h"
#include "Renderer.h"
#include "Resource/Material/MaterialTechnique.h"
#include "Resource/MaterialBlueprint/MaterialBlueprintResource.h"


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
namespace RendererRuntime
{


	//[-------------------------------------------------------]
	//[ Public methods                                        ]
	//[-------------------------------------------------------]
	CompositorInstancePassCompute::CompositorInstancePassCompute(const CompositorResourcePassCompute& compositorResourcePassCompute, const CompositorNodeInstance& compositorNodeInstance) :
		ICompositorInstancePass(compositorResourcePassCompute, compositorNodeInstance),
		mComputeMaterialBlueprint(true),
		mRenderQueue(compositorNodeInstance.getCompositorWorkspaceInstance().getRendererRuntime().getMaterialBlueprintResourceManager().getIndirectBufferManager(), 0, 0, false, false),
		mMaterialResourceId(getInvalid<MaterialResourceId>())
	{
		const IRendererRuntime& rendererRuntime = compositorNodeInstance.getCompositorWorkspaceInstance().getRendererRuntime();

		// Sanity checks
		RENDERER_ASSERT(rendererRuntime.getContext(), !compositorResourcePassCompute.isMaterialDefinitionMandatory() || isValid(compositorResourcePassCompute.getMaterialAssetId()) || isValid(compositorResourcePassCompute.getMaterialBlueprintAssetId()), "Invalid compositor resource pass compute configuration")
		RENDERER_ASSERT(rendererRuntime.getContext(), !(isValid(compositorResourcePassCompute.getMaterialAssetId()) && isValid(compositorResourcePassCompute.getMaterialBlueprintAssetId())), "Invalid compositor resource pass compute configuration")

		// Get parent material resource ID and initiate creating the compositor instance pass compute material resource
		MaterialResourceManager& materialResourceManager = rendererRuntime.getMaterialResourceManager();
		if (isValid(compositorResourcePassCompute.getMaterialAssetId()))
		{
			// Get or load material resource
			MaterialResourceId materialResourceId = getInvalid<MaterialResourceId>();
			materialResourceManager.loadMaterialResourceByAssetId(compositorResourcePassCompute.getMaterialAssetId(), materialResourceId, this);
		}
		else
		{
			// Get or load material blueprint resource
			const AssetId materialBlueprintAssetId = compositorResourcePassCompute.getMaterialBlueprintAssetId();
			if (isValid(materialBlueprintAssetId))
			{
				MaterialResourceId parentMaterialResourceId = materialResourceManager.getMaterialResourceIdByAssetId(materialBlueprintAssetId);
				if (isInvalid(parentMaterialResourceId))
				{
					parentMaterialResourceId = materialResourceManager.createMaterialResourceByAssetId(materialBlueprintAssetId, materialBlueprintAssetId, compositorResourcePassCompute.getMaterialTechniqueId());
				}
				createMaterialResource(parentMaterialResourceId);
			}
		}
	}

	CompositorInstancePassCompute::~CompositorInstancePassCompute()
	{
		if (isValid(mMaterialResourceId))
		{
			// Clear the renderable manager
			mRenderableManager.getRenderables().clear();

			// Destroy the material resource the compositor instance pass compute created
			getCompositorNodeInstance().getCompositorWorkspaceInstance().getRendererRuntime().getMaterialResourceManager().destroyMaterialResource(mMaterialResourceId);
		}
	}


	//[-------------------------------------------------------]
	//[ Public virtual RendererRuntime::ICompositorInstancePass methods ]
	//[-------------------------------------------------------]
	void CompositorInstancePassCompute::onFillCommandBuffer(const Renderer::IRenderTarget* renderTarget, const CompositorContextData& compositorContextData, Renderer::CommandBuffer& commandBuffer)
	{
		if (isValid(mMaterialResourceId))
		{
			// Sanity check
			RENDERER_ASSERT(getCompositorNodeInstance().getCompositorWorkspaceInstance().getRendererRuntime().getContext(), !mRenderableManager.getRenderables().empty(), "No renderables")

			// Combined scoped profiler CPU and GPU sample as well as renderer debug event command
			RENDERER_SCOPED_PROFILER_EVENT_DYNAMIC(getCompositorNodeInstance().getCompositorWorkspaceInstance().getRendererRuntime().getContext(), commandBuffer, getCompositorResourcePass().getDebugName())

			// Fill command buffer depending on graphics or compute material blueprint
			mRenderQueue.addRenderablesFromRenderableManager(mRenderableManager, static_cast<const CompositorResourcePassCompute&>(getCompositorResourcePass()).getMaterialTechniqueId(), compositorContextData);
			if (mRenderQueue.getNumberOfDrawCalls() > 0)
			{
				if (mComputeMaterialBlueprint)
				{
					// Sanity check
					RENDERER_ASSERT(getCompositorNodeInstance().getCompositorWorkspaceInstance().getRendererRuntime().getContext(), nullptr == renderTarget, "The compute compositor instance pass needs an invalid render target in case a compute material blueprint is used")

					// Fill command buffer using a compute material blueprint
					mRenderQueue.fillComputeCommandBuffer(compositorContextData, commandBuffer);
				}
				else
				{
					// Sanity check
					RENDERER_ASSERT(getCompositorNodeInstance().getCompositorWorkspaceInstance().getRendererRuntime().getContext(), nullptr != renderTarget, "The compute compositor instance pass needs a valid render target in case a graphics material blueprint is used")

					// Fill command buffer using a graphics material blueprint
					mRenderQueue.fillGraphicsCommandBuffer(*renderTarget, compositorContextData, commandBuffer);
				}
			}
		}
	}


	//[-------------------------------------------------------]
	//[ Protected virtual RendererRuntime::IResourceListener methods ]
	//[-------------------------------------------------------]
	void CompositorInstancePassCompute::onLoadingStateChange(const IResource& resource)
	{
		RENDERER_ASSERT(getCompositorNodeInstance().getCompositorWorkspaceInstance().getRendererRuntime().getContext(), resource.getId() == mMaterialResourceId, "Invalid material resource ID")
		createMaterialResource(resource.getId());
	}


	//[-------------------------------------------------------]
	//[ Protected virtual RendererRuntime::CompositorInstancePassCompute methods ]
	//[-------------------------------------------------------]
	void CompositorInstancePassCompute::createMaterialResource(MaterialResourceId parentMaterialResourceId)
	{
		const IRendererRuntime& rendererRuntime = getCompositorNodeInstance().getCompositorWorkspaceInstance().getRendererRuntime();

		// Sanity checks
		RENDERER_ASSERT(rendererRuntime.getContext(), isInvalid(mMaterialResourceId), "Invalid material resource ID")
		RENDERER_ASSERT(rendererRuntime.getContext(), isValid(parentMaterialResourceId), "Invalid material resource ID")

		// Each compositor instance pass compute must have its own material resource since material property values might vary
		MaterialResourceManager& materialResourceManager = rendererRuntime.getMaterialResourceManager();
		mMaterialResourceId = materialResourceManager.createMaterialResourceByCloning(parentMaterialResourceId);

		// Graphics or compute material blueprint?
		mComputeMaterialBlueprint = true;
		MaterialResource& materialResource = materialResourceManager.getById(mMaterialResourceId);
		{
			const MaterialTechnique* materialTechnique = materialResource.getMaterialTechniqueById(MaterialResourceManager::DEFAULT_MATERIAL_TECHNIQUE_ID);
			RENDERER_ASSERT(rendererRuntime.getContext(), nullptr != materialTechnique, "Invalid material technique")
			MaterialBlueprintResource* materialBlueprintResource = rendererRuntime.getMaterialBlueprintResourceManager().tryGetById(materialTechnique->getMaterialBlueprintResourceId());
			RENDERER_ASSERT(rendererRuntime.getContext(), nullptr != materialBlueprintResource, "Invalid material blueprint resource")
			mComputeMaterialBlueprint = isValid(materialBlueprintResource->getComputeShaderBlueprintResourceId());
		}

		{ // Set compositor resource pass compute material properties
			const MaterialProperties::SortedPropertyVector& sortedPropertyVector = static_cast<const CompositorResourcePassCompute&>(getCompositorResourcePass()).getMaterialProperties().getSortedPropertyVector();
			if (!sortedPropertyVector.empty())
			{
				for (const MaterialProperty& materialProperty : sortedPropertyVector)
				{
					if (materialProperty.isOverwritten())
					{
						materialResource.setPropertyById(materialProperty.getMaterialPropertyId(), materialProperty);
					}
				}
			}
		}

		// Setup renderable manager using attribute-less rendering
		mRenderableManager.getRenderables().emplace_back(mRenderableManager, Renderer::IVertexArrayPtr(), materialResourceManager, mMaterialResourceId, getInvalid<SkeletonResourceId>(), false, 0, 3);
	}


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererRuntime
