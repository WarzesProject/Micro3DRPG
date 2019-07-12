#include "stdafx.h"
#include "Resource/Scene/Item/Grass/GrassSceneItem.h"
#include "Resource/Scene/SceneResource.h"
#include "Resource/Scene/SceneNode.h"
#include "IRendererRuntime.h"
#include "Resource/Mesh/MeshResourceManager.h"
#include "Resource/Material/MaterialTechnique.h"
#include "Resource/Material/MaterialResource.h"
#include "Resource/Material/MaterialResourceManager.h"


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
namespace RendererRuntime
{


	//[-------------------------------------------------------]
	//[ Protected virtual RendererRuntime::MaterialSceneItem methods ]
	//[-------------------------------------------------------]
	void GrassSceneItem::initialize()
	{
		// Call the base implementation
		if (mMaximumNumberOfGrass > 0)
		{
			MaterialSceneItem::initialize();
		}
	}

	void GrassSceneItem::onMaterialResourceCreated()
	{
		// Setup renderable manager
		const IRendererRuntime& rendererRuntime = getSceneResource().getRendererRuntime();
		const MaterialResourceManager& materialResourceManager = rendererRuntime.getMaterialResourceManager();
		const MaterialResourceId materialResourceId = getMaterialResourceId();
		mRenderableManager.getRenderables().emplace_back(mRenderableManager, rendererRuntime.getMeshResourceManager().getDrawIdVertexArrayPtr(), materialResourceManager, materialResourceId, getInvalid<SkeletonResourceId>(), false, mIndirectBufferPtr);
		mRenderableManager.updateCachedRenderablesData();

		// Tell the used material resource about our structured buffer
		for (MaterialTechnique* materialTechnique : materialResourceManager.getById(materialResourceId).getSortedMaterialTechniqueVector())
		{
			materialTechnique->setStructuredBufferPtr(mStructuredBufferPtr);
		}
	}


	//[-------------------------------------------------------]
	//[ Protected methods                                     ]
	//[-------------------------------------------------------]
	GrassSceneItem::GrassSceneItem(SceneResource& sceneResource) :
		MaterialSceneItem(sceneResource, false),	// TODO(co) Set bounding box
		mMaximumNumberOfGrass(3)	// TODO(co) Make this dynamic
	{
		// The renderer backend must support structured buffers
		const IRendererRuntime& rendererRuntime = getSceneResource().getRendererRuntime();
		if (rendererRuntime.getRenderer().getCapabilities().maximumStructuredBufferSize > 0)
		{
			// Create vertex array object (VAO)
			// Create the vertex buffer object (VBO)
			// TODO(co) Make this dynamic
			const GrassDataStruct grassData[3] =
			{
				{
					3.0f, -1.781f, 20.0f, 0.5f,
					1.0f,  1.0f,   1.0f, 0.4f
				},
				{
					5.0f, -1.781f, 19.0f, 1.0f,
					1.0f,  1.0f,   1.0f, 0.8f
				},
				{
					4.0f, -1.781f, 21.0f, 1.5f,
					1.0f,  1.0f,   1.0f, 1.2f
				}
			};
			Renderer::IBufferManager& bufferManager = rendererRuntime.getBufferManager();

			// Create the structured buffer
			mStructuredBufferPtr = bufferManager.createStructuredBuffer(sizeof(GrassDataStruct) * mMaximumNumberOfGrass, grassData, Renderer::BufferFlag::SHADER_RESOURCE, Renderer::BufferUsage::STATIC_DRAW, sizeof(GrassDataStruct));
			RENDERER_SET_RESOURCE_DEBUG_NAME(mStructuredBufferPtr, "Grass structured buffer")

			{ // Create the indirect buffer: Twelve vertices per grass (two quads), grass index = instance index
				const Renderer::DrawArguments drawArguments =
				{
					12,						// vertexCountPerInstance (uint32_t)
					mMaximumNumberOfGrass,	// instanceCount (uint32_t)
					0,						// startVertexLocation (uint32_t)
					0						// startInstanceLocation (uint32_t)
				};
				mIndirectBufferPtr = bufferManager.createIndirectBuffer(sizeof(Renderer::DrawArguments), &drawArguments, Renderer::IndirectBufferFlag::UNORDERED_ACCESS | Renderer::IndirectBufferFlag::DRAW_ARGUMENTS);
				RENDERER_SET_RESOURCE_DEBUG_NAME(mIndirectBufferPtr, "Grass indirect buffer")
			}
		}
		else
		{
			mMaximumNumberOfGrass = 0;
			RENDERER_LOG_ONCE(rendererRuntime.getContext(), COMPATIBILITY_WARNING, "The renderer runtime grass scene item needs a renderer backend with structured buffer support")
		}
	}


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererRuntime
