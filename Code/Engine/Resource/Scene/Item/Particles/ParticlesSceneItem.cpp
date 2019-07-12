#include "stdafx.h"
#include "Resource/Scene/Item/Particles/ParticlesSceneItem.h"
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
	void ParticlesSceneItem::initialize()
	{
		// Call the base implementation
		if (mMaximumNumberOfParticles > 0)
		{
			MaterialSceneItem::initialize();
		}
	}

	void ParticlesSceneItem::onMaterialResourceCreated()
	{
		// Setup renderable manager: Six vertices per particle, particle index = instance index
		const IRendererRuntime& rendererRuntime = getSceneResource().getRendererRuntime();
		const MaterialResourceManager& materialResourceManager = rendererRuntime.getMaterialResourceManager();
		const MaterialResourceId materialResourceId = getMaterialResourceId();
		mRenderableManager.getRenderables().emplace_back(mRenderableManager, rendererRuntime.getMeshResourceManager().getDrawIdVertexArrayPtr(), materialResourceManager, materialResourceId, getInvalid<SkeletonResourceId>(), false, 0, 6, mMaximumNumberOfParticles);
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
	ParticlesSceneItem::ParticlesSceneItem(SceneResource& sceneResource) :
		MaterialSceneItem(sceneResource, false),	// TODO(co) Set bounding box
		mMaximumNumberOfParticles(8)				// TODO(co) Make this dynamic
	{
		// The renderer backend must support structured buffers
		const IRendererRuntime& rendererRuntime = getSceneResource().getRendererRuntime();
		if (rendererRuntime.getRenderer().getCapabilities().maximumStructuredBufferSize > 0)
		{
			// Create vertex array object (VAO)
			// Create the vertex buffer object (VBO)
			// TODO(co) Make this dynamic
			const ParticleDataStruct particlesData[8] =
			{
				{
					4.88f,  1.4f, -1.44f, 0.5f,
					 1.0f,  1.0f,   1.0f, 0.3f
				},
				{
					-6.2f,  1.4f, -1.44f, 0.5f,
					 1.0f,  1.0f,   1.0f, 0.3f
				},
				{
					4.88f,  1.4f,   2.2f, 0.5f,
					 1.0f,  1.0f,   1.0f, 0.3f
				},
				{
					-6.2f,  1.4f,   2.2f, 0.5f,
					 1.0f,  1.0f,   1.0f, 0.3f
				},
				{
					-12.0f, 1.39f,  -4.0f, 1.0f,
					  1.0f,  0.0f,   0.0f, 1.0f
				},
				{
					11.2f, 1.39f,  -4.0f, 1.0f,
					 0.0f,  1.0f,   0.0f, 1.0f
				},
				{
					-12.0f, 1.39f,   4.5f, 1.0f,
					  0.0f,  0.0f,   1.0f, 1.0f
				},
				{
					11.2f, 1.39f,   4.5f, 1.0f,
					 1.0f,  1.0f,   1.0f, 1.0f
				}
			};

			// Create the structured buffer
			mStructuredBufferPtr = rendererRuntime.getBufferManager().createStructuredBuffer(sizeof(ParticleDataStruct) * mMaximumNumberOfParticles, particlesData, Renderer::BufferFlag::SHADER_RESOURCE, Renderer::BufferUsage::STATIC_DRAW, sizeof(ParticleDataStruct));
			RENDERER_SET_RESOURCE_DEBUG_NAME(mStructuredBufferPtr, "Particles structured buffer")
		}
		else
		{
			mMaximumNumberOfParticles = 0;
			RENDERER_LOG_ONCE(rendererRuntime.getContext(), COMPATIBILITY_WARNING, "The renderer runtime particles scene item needs a renderer backend with structured buffer support")
		}
	}


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererRuntime
