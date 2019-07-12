#include "stdafx.h"
#include "Resource/Scene/Item/Volume/VolumeSceneItem.h"
#include "Resource/Scene/SceneResource.h"
#include "Resource/Scene/SceneNode.h"
#include "IRendererRuntime.h"
#include "Resource/Mesh/MeshResourceManager.h"


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
		static Renderer::IVertexArrayPtr VolumeVertexArrayPtr;	///< Vertex array object (VAO), can be a null pointer, shared between all volume instances


		//[-------------------------------------------------------]
		//[ Global functions                                      ]
		//[-------------------------------------------------------]
		[[nodiscard]] Renderer::IVertexArray* createVolumeVertexArray(const RendererRuntime::IRendererRuntime& rendererRuntime)
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
				},
				{ // Attribute 1, see "17/11/2012 Surviving without gl_DrawID" - https://www.g-truc.net/post-0518.html
					// Data destination
					Renderer::VertexAttributeFormat::UINT_1,	// vertexAttributeFormat (Renderer::VertexAttributeFormat)
					"drawId",									// name[32] (char)
					"DRAWID",									// semanticName[32] (char)
					0,											// semanticIndex (uint32_t)
					// Data source
					1,											// inputSlot (uint32_t)
					0,											// alignedByteOffset (uint32_t)
					sizeof(uint32_t),							// strideInBytes (uint32_t)
					1											// instancesPerElement (uint32_t)
				}
			};
			const Renderer::VertexAttributes vertexAttributes(static_cast<uint32_t>(GLM_COUNTOF(vertexAttributesLayout)), vertexAttributesLayout);

			// Our cube is constructed like this
			/*
					3+------+2  y
					/|     /|   |
				   / |    / |   |
				  / 0+---/--+1  *---x
				7+------+6 /   /
				 | /    | /   z
				 |/     |/
				4+------+5
			*/

			// Create the vertex buffer object (VBO)
			// -> Object space vertex positions
			static constexpr float VERTEX_POSITION[] =
			{
				-0.5f, -0.5f, -0.5f,	// 0
				 0.5f, -0.5f, -0.5f,	// 1
				 0.5f,  0.5f, -0.5f,	// 2
				-0.5f,  0.5f, -0.5f,	// 3
				-0.5f, -0.5f,  0.5f,	// 4
				 0.5f, -0.5f,  0.5f,	// 5
				 0.5f,  0.5f,  0.5f,	// 6
				-0.5f,  0.5f,  0.5f,	// 7
			};
			Renderer::IBufferManager& bufferManager = rendererRuntime.getBufferManager();
			Renderer::IVertexBufferPtr vertexBuffer(bufferManager.createVertexBuffer(sizeof(VERTEX_POSITION), VERTEX_POSITION));
			RENDERER_SET_RESOURCE_DEBUG_NAME(vertexBuffer, "Volume")

			// Create the index buffer object (IBO)
			static constexpr uint16_t INDICES[] =
			{
				// Back		Triangle
				2, 3, 0,	// 0
				0, 1, 2,	// 1
				// Front
				7, 6, 5,	// 0
				5, 4, 7,	// 1
				// Left
				3, 7, 4,	// 0
				4, 0, 3,	// 1
				// Right
				6, 2, 1,	// 0
				1, 5, 6,	// 1
				// Top
				3, 2, 6,	// 0
				6, 7, 3,	// 1
				// Bottom
				0, 4, 5,	// 0
				5, 1, 0		// 1
			};
			Renderer::IIndexBuffer* indexBuffer = bufferManager.createIndexBuffer(sizeof(INDICES), INDICES);
			RENDERER_SET_RESOURCE_DEBUG_NAME(indexBuffer, "Volume")

			// Create vertex array object (VAO)
			const Renderer::VertexArrayVertexBuffer vertexArrayVertexBuffers[] = { vertexBuffer, rendererRuntime.getMeshResourceManager().getDrawIdVertexBufferPtr() };
			Renderer::IVertexArray* vertexArray = bufferManager.createVertexArray(vertexAttributes, static_cast<uint32_t>(GLM_COUNTOF(vertexArrayVertexBuffers)), vertexArrayVertexBuffers, indexBuffer);
			RENDERER_SET_RESOURCE_DEBUG_NAME(vertexArray, "Volume")

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
	//[ Public RendererRuntime::ISceneItem methods            ]
	//[-------------------------------------------------------]
	const RenderableManager* VolumeSceneItem::getRenderableManager() const
	{
		// Sanity check
		RENDERER_ASSERT(getContext(), mRenderableManager.getTransform().scale.x == mRenderableManager.getTransform().scale.y && mRenderableManager.getTransform().scale.y == mRenderableManager.getTransform().scale.z, "Only uniform scale is supported to keep things simple")

		// Call the base implementation
		return MaterialSceneItem::getRenderableManager();
	}


	//[-------------------------------------------------------]
	//[ Protected virtual RendererRuntime::MaterialSceneItem methods ]
	//[-------------------------------------------------------]
	void VolumeSceneItem::onMaterialResourceCreated()
	{
		// Setup renderable manager
		mRenderableManager.getRenderables().emplace_back(mRenderableManager, ::detail::VolumeVertexArrayPtr, getSceneResource().getRendererRuntime().getMaterialResourceManager(), getMaterialResourceId(), getInvalid<SkeletonResourceId>(), true, 0, 36);
		mRenderableManager.updateCachedRenderablesData();
	}


	//[-------------------------------------------------------]
	//[ Private methods                                       ]
	//[-------------------------------------------------------]
	VolumeSceneItem::VolumeSceneItem(SceneResource& sceneResource) :
		MaterialSceneItem(sceneResource)
	{
		// Add reference to vertex array object (VAO) shared between all volume instances
		if (nullptr == ::detail::VolumeVertexArrayPtr)
		{
			::detail::VolumeVertexArrayPtr = ::detail::createVolumeVertexArray(getSceneResource().getRendererRuntime());
			RENDERER_ASSERT(getContext(), nullptr != ::detail::VolumeVertexArrayPtr, "Invalid volume vertex array")
		}
		::detail::VolumeVertexArrayPtr->addReference();
	}

	VolumeSceneItem::~VolumeSceneItem()
	{
		if (isValid(getMaterialResourceId()))
		{
			// Clear the renderable manager right now so we have no more references to the shared vertex array
			mRenderableManager.getRenderables().clear();
		}

		// Release reference to vertex array object (VAO) shared between all volume instances
		if (nullptr != ::detail::VolumeVertexArrayPtr && 1 == ::detail::VolumeVertexArrayPtr->releaseReference())	// +1 for reference to global shared pointer
		{
			::detail::VolumeVertexArrayPtr = nullptr;
		}
	}


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererRuntime
