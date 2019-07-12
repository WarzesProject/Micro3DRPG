#pragma once


//[-------------------------------------------------------]
//[ Includes                                              ]
//[-------------------------------------------------------]
#include "Core/Manager.h"

// Disable warnings in external headers, we can't fix them
PRAGMA_WARNING_PUSH
	PRAGMA_WARNING_DISABLE_MSVC(4365)	// warning C4365: 'argument': conversion from 'long' to 'unsigned int', signed/unsigned mismatch
	PRAGMA_WARNING_DISABLE_MSVC(4571)	// warning C4571: Informational: catch(...) semantics changed since Visual C++ 7.1; structured exceptions (SEH) are no longer caught
	PRAGMA_WARNING_DISABLE_MSVC(4668)	// warning C4668: '_M_HYBRID_X86_ARM64' is not defined as a preprocessor macro, replacing with '0' for '#if/#elif'
	#include <vector>
PRAGMA_WARNING_POP


//[-------------------------------------------------------]
//[ Forward declarations                                  ]
//[-------------------------------------------------------]
namespace Renderer
{
	class CommandBuffer;
	class IUniformBuffer;
	class IBufferManager;
	class IResourceGroup;
}
namespace RendererRuntime
{
	class IRendererRuntime;
	class MaterialBufferSlot;
	class MaterialBlueprintResource;
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
	*    Material buffer manager
	*
	*  @note
	*    - For material batching
	*    - Concept basing on OGRE 2.1 "Ogre::ConstBufferPool", but more generic and simplified thanks to the material blueprint concept
	*/
	class MaterialBufferManager final : private Manager
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
		*  @param[in] materialBlueprintResource
		*    Material blueprint resource
		*/
		MaterialBufferManager(IRendererRuntime& rendererRuntime, const MaterialBlueprintResource& materialBlueprintResource);

		/**
		*  @brief
		*    Destructor
		*/
		~MaterialBufferManager();

		/**
		*  @brief
		*    Request a slot and fill the material slot; automatically schedules for update
		*/
		void requestSlot(MaterialBufferSlot& materialBufferSlot);

		/**
		*  @brief
		*    Release a slot requested with "RendererRuntime::MaterialBufferManager::requestSlot()"
		*/
		void releaseSlot(MaterialBufferSlot& materialBufferSlot);

		/**
		*  @brief
		*    Schedule the slot of the given material slot for update
		*/
		void scheduleForUpdate(MaterialBufferSlot& materialBufferSlot);

		/**
		*  @brief
		*    Reset last graphics bound pool and update the dirty slots
		*/
		void resetLastGraphicsBoundPool();

		/**
		*  @brief
		*    Reset last compute bound pool and update the dirty slots
		*/
		void resetLastComputeBoundPool();

		/**
		*  @brief
		*    Fill slot to graphics command buffer
		*
		*  @param[in] materialBufferSlot
		*    Graphics material buffer slot to bind
		*  @param[out] commandBuffer
		*    Command buffer to fill
		*/
		void fillGraphicsCommandBuffer(MaterialBufferSlot& materialBufferSlot, Renderer::CommandBuffer& commandBuffer);

		/**
		*  @brief
		*    Fill slot to compute command buffer
		*
		*  @param[in] materialBufferSlot
		*    Compute material buffer slot to bind
		*  @param[out] commandBuffer
		*    Command buffer to fill
		*/
		void fillComputeCommandBuffer(MaterialBufferSlot& materialBufferSlot, Renderer::CommandBuffer& commandBuffer);


	//[-------------------------------------------------------]
	//[ Private methods                                       ]
	//[-------------------------------------------------------]
	private:
		explicit MaterialBufferManager(const MaterialBufferManager&) = delete;
		MaterialBufferManager& operator=(const MaterialBufferManager&) = delete;
		void uploadDirtySlots();


	//[-------------------------------------------------------]
	//[ Private definitions                                   ]
	//[-------------------------------------------------------]
	private:
		struct BufferPool final
		{
			std::vector<uint32_t>	  freeSlots;
			Renderer::IUniformBuffer* uniformBuffer;	///< Memory is managed by this buffer pool instance
			Renderer::IResourceGroup* resourceGroup;	///< Memory is managed by this buffer pool instance

			BufferPool(uint32_t bufferSize, uint32_t slotsPerPool, Renderer::IBufferManager& bufferManager, const MaterialBlueprintResource& materialBlueprintResource);
			~BufferPool();
		};

		typedef std::vector<BufferPool*>		 BufferPools;
		typedef std::vector<MaterialBufferSlot*> MaterialBufferSlots;
		typedef std::vector<uint8_t>			 ScratchBuffer;


	//[-------------------------------------------------------]
	//[ Private data                                          ]
	//[-------------------------------------------------------]
	private:
		IRendererRuntime&				 mRendererRuntime;
		const MaterialBlueprintResource& mMaterialBlueprintResource;
		BufferPools						 mBufferPools;
		uint32_t						 mSlotsPerPool;
		uint32_t						 mBufferSize;
		MaterialBufferSlots				 mDirtyMaterialBufferSlots;
		MaterialBufferSlots				 mMaterialBufferSlots;
		const BufferPool*				 mLastGraphicsBoundPool;
		const BufferPool*				 mLastComputeBoundPool;
		ScratchBuffer					 mScratchBuffer;


	};


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererRuntime
