#include "stdafx.h"
#include "Resource/Scene/Item/Sky/SkySceneItem.h"
#include "Resource/Scene/SceneResource.h"
#include "Resource/Scene/SceneNode.h"
#include "IRendererRuntime.h"


//[-------------------------------------------------------]
//[ Anonymous detail namespace                            ]
//[-------------------------------------------------------]
namespace
{
	namespace detail
	{


		//[-------------------------------------------------------]
		//[ Global variables                                      ]
		//[-------------------------------------------------------]
		static Renderer::IVertexArrayPtr SkyVertexArrayPtr;	///< Vertex array object (VAO), can be a null pointer, shared between all sky instances


		//[-------------------------------------------------------]
		//[ Global functions                                      ]
		//[-------------------------------------------------------]
		[[nodiscard]] Renderer::IVertexArray* createSkyVertexArray(Renderer::IBufferManager& bufferManager)
		{
			// Vertex input layout
			static constexpr Renderer::VertexAttribute vertexAttributesLayout[] =
			{
				{ // Attribute 0
					// Data destination
					Renderer::VertexAttributeFormat::FLOAT_3,	// vertexAttributeFormat (Renderer::VertexAttributeFormat)
					"Position",									// name[32] (char)
					"POSITION",									// semanticName[32] (char)
					0,											// semanticIndex (uint32_t)
					// Data source
					0,											// inputSlot (uint32_t)
					0,											// alignedByteOffset (uint32_t)
					sizeof(float) * 3,							// strideInBytes (uint32_t)
					0											// instancesPerElement (uint32_t)
				}
			};
			const Renderer::VertexAttributes vertexAttributes(static_cast<uint32_t>(GLM_COUNTOF(vertexAttributesLayout)), vertexAttributesLayout);

			// Create the vertex buffer object (VBO)
			// -> Clip space vertex positions, left/bottom is (-1,-1) and right/top is (1,1)
			static constexpr float VERTEX_POSITION[] =
			{
				-1.0f,  1.0f,  1.0f,
				 1.0f,  1.0f,  1.0f,
				 1.0f, -1.0f,  1.0f,
				-1.0f, -1.0f,  1.0f,
				 1.0f,  1.0f, -1.0f,
				-1.0f,  1.0f, -1.0f,
				-1.0f, -1.0f, -1.0f,
				 1.0f, -1.0f,- 1.0f
			};
			Renderer::IVertexBufferPtr vertexBuffer(bufferManager.createVertexBuffer(sizeof(VERTEX_POSITION), VERTEX_POSITION));
			RENDERER_SET_RESOURCE_DEBUG_NAME(vertexBuffer, "Sky")

			// Create the index buffer object (IBO)
			static constexpr uint16_t INDICES[] =
			{
				0, 1, 2, 2, 3, 0,	// Front
				1, 4, 7, 7, 2, 1,	// Right
				4, 5, 6, 6, 7, 4,	// Back
				5, 0, 3, 3, 6, 5,	// Left
				5, 4, 1, 1, 0, 5,	// Top
				3, 2, 7, 7, 6, 3	// Bottom
			};
			Renderer::IIndexBuffer* indexBuffer = bufferManager.createIndexBuffer(sizeof(INDICES), INDICES);
			RENDERER_SET_RESOURCE_DEBUG_NAME(indexBuffer, "Sky")

			// Create vertex array object (VAO)
			const Renderer::VertexArrayVertexBuffer vertexArrayVertexBuffers[] = { vertexBuffer };
			Renderer::IVertexArray* vertexArray = bufferManager.createVertexArray(vertexAttributes, static_cast<uint32_t>(GLM_COUNTOF(vertexArrayVertexBuffers)), vertexArrayVertexBuffers, indexBuffer);
			RENDERER_SET_RESOURCE_DEBUG_NAME(vertexArray, "Sky")

			// Done
			return vertexArray;
		}


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
	//[ Protected virtual RendererRuntime::MaterialSceneItem methods ]
	//[-------------------------------------------------------]
	void SkySceneItem::onMaterialResourceCreated()
	{
		// Setup renderable manager
		mRenderableManager.getRenderables().emplace_back(mRenderableManager, ::detail::SkyVertexArrayPtr, getSceneResource().getRendererRuntime().getMaterialResourceManager(), getMaterialResourceId(), getInvalid<SkeletonResourceId>(), true, 0, 36);
		mRenderableManager.updateCachedRenderablesData();
	}


	//[-------------------------------------------------------]
	//[ Private methods                                       ]
	//[-------------------------------------------------------]
	SkySceneItem::SkySceneItem(SceneResource& sceneResource) :
		MaterialSceneItem(sceneResource, false)	///< The sky isn't allowed to be culled
	{
		// Add reference to vertex array object (VAO) shared between all sky instances
		if (nullptr == ::detail::SkyVertexArrayPtr)
		{
			::detail::SkyVertexArrayPtr = ::detail::createSkyVertexArray(getSceneResource().getRendererRuntime().getBufferManager());
			RENDERER_ASSERT(getContext(), nullptr != ::detail::SkyVertexArrayPtr, "Invalid sky vertex array")
		}
		::detail::SkyVertexArrayPtr->addReference();
	}

	SkySceneItem::~SkySceneItem()
	{
		if (isValid(getMaterialResourceId()))
		{
			// Clear the renderable manager right now so we have no more references to the shared vertex array
			mRenderableManager.getRenderables().clear();
		}

		// Release reference to vertex array object (VAO) shared between all sky instances
		if (nullptr != ::detail::SkyVertexArrayPtr && 1 == ::detail::SkyVertexArrayPtr->releaseReference())	// +1 for reference to global shared pointer
		{
			::detail::SkyVertexArrayPtr = nullptr;
		}
	}


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererRuntime
