#pragma once


//[-------------------------------------------------------]
//[ Includes                                              ]
//[-------------------------------------------------------]
#include "Core/Manager.h"
#include "Resource/MaterialBlueprint/MaterialBlueprintResource.h"
#include <glm/glm.hpp>

//[-------------------------------------------------------]
//[ Forward declarations                                  ]
//[-------------------------------------------------------]
namespace RendererRuntime
{
	class Renderable;
	class IRendererRuntime;
	class MaterialTechnique;
	class PassBufferManager;
}


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
namespace RendererRuntime
{


	//[-------------------------------------------------------]
	//[ Classes                                               ]
	//[-------------------------------------------------------]
	/**
	*  @brief
	*    Texture instance buffer manager; includes a texture buffer to transport more complex per-instance data
	*
	*  @remarks
	*    "RendererRuntime::UniformInstanceBufferManager" is a simplified version of "RendererRuntime::TextureInstanceBufferManager". Shared code is duplicated by intent
	*     (including this comment) to avoid making the implementations too complex due to over-engineering. This is performance critical code and the topic is complex
	*     enough as it is. When changing one implementation don't forget to update the other one as well.
	*/
	class TextureInstanceBufferManager final : private Manager
	{


	//[-------------------------------------------------------]
	//[ Public methods                                        ]
	//[-------------------------------------------------------]
	public:
		/**
		*  @brief
		*    Constructor
		*
		*  @param[in] rendererRuntime
		*    Renderer runtime instance to use
		*/
		explicit TextureInstanceBufferManager(IRendererRuntime& rendererRuntime);

		/**
		*  @brief
		*    Destructor
		*/
		~TextureInstanceBufferManager();

		/**
		*  @brief
		*    Startup instance buffer filling
		*
		*  @param[in] materialBlueprintResource
		*    Material blueprint resource
		*  @param[out] commandBuffer
		*    Command buffer to fill
		*/
		void startupBufferFilling(const MaterialBlueprintResource& materialBlueprintResource, Renderer::CommandBuffer& commandBuffer);

		/**
		*  @brief
		*    Fill the instance buffer
		*
		*  @param[in] worldSpaceCameraPosition
		*    64 bit world space position of the camera for camera relative rendering
		*  @param[in] materialBlueprintResource
		*    Material blueprint resource
		*  @param[in] passBufferManager
		*    Pass buffer manager instance to use, can be a null pointer
		*  @param[in] instanceUniformBuffer
		*    Instance uniform buffer instance to use
		*  @param[in] renderable
		*    Renderable to fill the buffer for
		*  @param[in] materialTechnique
		*    Used material technique
		*  @param[out] commandBuffer
		*    Command buffer to fill
		*
		*  @return
		*    Start instance location, used for draw ID (see "17/11/2012 Surviving without gl_DrawID" - https://www.g-truc.net/post-0518.html)
		*/
		[[nodiscard]] uint32_t fillBuffer(const glm::dvec3& worldSpaceCameraPosition, const MaterialBlueprintResource& materialBlueprintResource, PassBufferManager* passBufferManager, const MaterialBlueprintResource::UniformBuffer& instanceUniformBuffer, const Renderable& renderable, MaterialTechnique& materialTechnique, Renderer::CommandBuffer& commandBuffer);

		/**
		*  @brief
		*    Called pre command buffer execution
		*/
		void onPreCommandBufferExecution();


	//[-------------------------------------------------------]
	//[ Private methods                                       ]
	//[-------------------------------------------------------]
	private:
		explicit TextureInstanceBufferManager(const TextureInstanceBufferManager&) = delete;
		TextureInstanceBufferManager& operator=(const TextureInstanceBufferManager&) = delete;
		void createInstanceBuffer();
		void mapCurrentInstanceBuffer();
		void unmapCurrentInstanceBuffer();


	//[-------------------------------------------------------]
	//[ Private definitions                                   ]
	//[-------------------------------------------------------]
	private:
		struct InstanceBuffer final
		{
			Renderer::IUniformBuffer* uniformBuffer;	///< Uniform buffer instance, always valid
			Renderer::ITextureBuffer* textureBuffer;	///< Texture buffer instance, always valid
			Renderer::IResourceGroup* resourceGroup;	///< Resource group instance, can be a null pointer
			bool					  mapped;
			InstanceBuffer(Renderer::IUniformBuffer& _uniformBuffer, Renderer::ITextureBuffer& _textureBuffer) :
				uniformBuffer(&_uniformBuffer),
				textureBuffer(&_textureBuffer),
				resourceGroup(nullptr),
				mapped(false)
			{
				// Nothing here
			}

		};
		typedef std::vector<InstanceBuffer> InstanceBuffers;


	//[-------------------------------------------------------]
	//[ Private data                                          ]
	//[-------------------------------------------------------]
	private:
		IRendererRuntime& mRendererRuntime;				///< Renderer runtime instance to use
		const uint32_t	  mMaximumUniformBufferSize;	///< Maximum uniform buffer size in bytes
		const uint32_t	  mMaximumTextureBufferSize;	///< Maximum texture buffer size in bytes
		InstanceBuffers	  mInstanceBuffers;				///< Instance buffers
		// Current instance buffer related data
		size_t			mCurrentInstanceBufferIndex;	///< Current instance buffer index, can be invalid if there's currently no current instance buffer
		InstanceBuffer* mCurrentInstanceBuffer;			///< Current instance buffer, can be a null pointer, don't destroy the instance since this is just a reference
		uint8_t*		mStartUniformBufferPointer;
		uint8_t*		mCurrentUniformBufferPointer;
		float*			mStartTextureBufferPointer;
		float*			mCurrentTextureBufferPointer;
		uint32_t		mStartInstanceLocation;			///< Start instance location, used for draw ID (see "17/11/2012 Surviving without gl_DrawID" - https://www.g-truc.net/post-0518.html)


	};


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererRuntime
