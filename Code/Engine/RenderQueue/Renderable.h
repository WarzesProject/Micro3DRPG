#pragma once


//[-------------------------------------------------------]
//[ Includes                                              ]
//[-------------------------------------------------------]
#include "Export.h"
#include "Renderer.h"

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
namespace RendererRuntime
{
	class RenderableManager;
	class MaterialResourceManager;
}


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
namespace RendererRuntime
{


	//[-------------------------------------------------------]
	//[ Global definitions                                    ]
	//[-------------------------------------------------------]
	typedef uint32_t MaterialResourceId;	///< POD material resource identifier
	typedef uint32_t SkeletonResourceId;	///< POD skeleton resource identifier
	typedef uint32_t MaterialTechniqueId;	///< Material technique identifier, result of hashing the material technique name via "RendererRuntime::StringId"


	//[-------------------------------------------------------]
	//[ Classes                                               ]
	//[-------------------------------------------------------]
	/**
	*  @brief
	*    Renderable
	*
	*  @note
	*    - Example: Abstract representation of a sub-mesh which is part of an mesh scene item
	*/
	class Renderable final
	{


	//[-------------------------------------------------------]
	//[ Friends                                               ]
	//[-------------------------------------------------------]
		friend class RenderQueue;		// Must be able to update cached material data
		friend class MaterialResource;	// Must be able to update cached material data


	//[-------------------------------------------------------]
	//[ Public methods                                        ]
	//[-------------------------------------------------------]
	public:
		RENDERERRUNTIME_API_EXPORT Renderable();
		RENDERERRUNTIME_API_EXPORT Renderable(RenderableManager& renderableManager, const Renderer::IVertexArrayPtr& vertexArrayPtr, const MaterialResourceManager& materialResourceManager, MaterialResourceId materialResourceId, SkeletonResourceId skeletonResourceId, bool drawIndexed, uint32_t startIndexLocation, uint32_t numberOfIndices, uint32_t instanceCount = 1);
		RENDERERRUNTIME_API_EXPORT Renderable(RenderableManager& renderableManager, const Renderer::IVertexArrayPtr& vertexArrayPtr, const MaterialResourceManager& materialResourceManager, MaterialResourceId materialResourceId, SkeletonResourceId skeletonResourceId, bool drawIndexed, const Renderer::IIndirectBufferPtr& indirectBufferPtr, uint32_t indirectBufferOffset = 0, uint32_t numberOfDraws = 1);

		inline ~Renderable()
		{
			unsetMaterialResourceIdInternal();
		}

		//[-------------------------------------------------------]
		//[ Data                                                  ]
		//[-------------------------------------------------------]
		[[nodiscard]] inline RenderableManager& getRenderableManager() const
		{
			return mRenderableManager;
		}

		[[nodiscard]] inline const Renderer::IVertexArrayPtr& getVertexArrayPtr() const
		{
			return mVertexArrayPtr;
		}

		inline void setVertexArrayPtr(const Renderer::IVertexArrayPtr& vertexArrayPtr)
		{
			mVertexArrayPtr = vertexArrayPtr;
		}

		[[nodiscard]] inline const Renderer::IIndirectBufferPtr& getIndirectBufferPtr() const
		{
			return mIndirectBufferPtr;
		}

		inline void setIndirectBufferPtr(const Renderer::IIndirectBufferPtr& indirectBufferPtr)
		{
			mIndirectBufferPtr = indirectBufferPtr;
		}

		[[nodiscard]] inline bool getDrawIndexed() const
		{
			return mDrawIndexed;
		}

		inline void setDrawIndexed(bool drawIndexed)
		{
			mDrawIndexed = drawIndexed;
		}

		[[nodiscard]] inline uint32_t getStartIndexLocation() const
		{
			return mStartIndexLocation;
		}

		inline void setStartIndexLocation(uint32_t startIndexLocation)
		{
			mStartIndexLocation = startIndexLocation;
		}

		[[nodiscard]] inline uint32_t getIndirectBufferOffset() const
		{
			return mIndirectBufferOffset;
		}

		inline void setIndirectBufferOffset(uint32_t indirectBufferOffset)
		{
			mIndirectBufferOffset = indirectBufferOffset;
		}

		[[nodiscard]] inline uint32_t getNumberOfIndices() const
		{
			return mNumberOfIndices;
		}

		inline void setNumberOfIndices(uint32_t numberOfIndices)
		{
			mNumberOfIndices = numberOfIndices;
		}

		[[nodiscard]] inline uint32_t getInstanceCount() const
		{
			return mInstanceCount;
		}

		inline void setInstanceCount(uint32_t instanceCount)
		{
			mInstanceCount = instanceCount;
		}

		[[nodiscard]] inline uint32_t getNumberOfDraws() const
		{
			return mNumberOfDraws;
		}

		inline void setNumberOfDraws(uint32_t numberOfDraws)
		{
			mNumberOfDraws = numberOfDraws;
		}

		[[nodiscard]] inline MaterialResourceId getMaterialResourceId() const
		{
			return mMaterialResourceId;
		}

		RENDERERRUNTIME_API_EXPORT void setMaterialResourceId(const MaterialResourceManager& materialResourceManager, MaterialResourceId materialResourceId);

		inline void unsetMaterialResourceId()
		{
			unsetMaterialResourceIdInternal();
		}

		[[nodiscard]] inline SkeletonResourceId getSkeletonResourceId() const
		{
			return mSkeletonResourceId;
		}

		inline void setSkeletonResourceId(SkeletonResourceId skeletonResourceId)
		{
			mSkeletonResourceId = skeletonResourceId;
		}

		//[-------------------------------------------------------]
		//[ Cached material data                                  ]
		//[-------------------------------------------------------]
		[[nodiscard]] inline uint8_t getRenderQueueIndex() const
		{
			return mRenderQueueIndex;
		}

		[[nodiscard]] inline bool getCastShadows() const
		{
			return mCastShadows;
		}


	//[-------------------------------------------------------]
	//[ Private methods                                       ]
	//[-------------------------------------------------------]
	private:
		Renderable& operator=(const Renderable&) = delete;
		RENDERERRUNTIME_API_EXPORT void unsetMaterialResourceIdInternal();


	//[-------------------------------------------------------]
	//[ Public definitions                                    ]
	//[-------------------------------------------------------]
	public:
		struct PipelineStateCache
		{
			MaterialTechniqueId			materialTechniqueId;
			uint32_t					generationCounter;	// Most simple solution to detect e.g. shader combination changes which make the pipeline state cache invalid
			Renderer::IPipelineStatePtr	pipelineStatePtr;
			inline PipelineStateCache(MaterialTechniqueId _materialTechniqueId, uint32_t _generationCounter, const Renderer::IGraphicsPipelineStatePtr& graphicsPipelineStatePtr) :
				materialTechniqueId(_materialTechniqueId),
				generationCounter(_generationCounter),
				pipelineStatePtr(graphicsPipelineStatePtr)
			{};
			inline PipelineStateCache(MaterialTechniqueId _materialTechniqueId, uint32_t _generationCounter, const Renderer::IComputePipelineStatePtr& computePipelineStatePtr) :
				materialTechniqueId(_materialTechniqueId),
				generationCounter(_generationCounter),
				pipelineStatePtr(computePipelineStatePtr)
			{};
		};
		typedef std::vector<PipelineStateCache> PipelineStateCaches;


	//[-------------------------------------------------------]
	//[ Private data                                          ]
	//[-------------------------------------------------------]
	private:
		// Data
		RenderableManager&				mRenderableManager;
		Renderer::IVertexArrayPtr		mVertexArrayPtr;		///< Optional vertex array object (VAO), can be a null pointer
		Renderer::IIndirectBufferPtr	mIndirectBufferPtr;		///< Optional indirect buffer, can be a null pointer
		union
		{
			uint32_t					mStartIndexLocation;	///< Used in case there's no indirect buffer
			uint32_t					mIndirectBufferOffset;	///< Used in case there's an indirect buffer
		};
		uint32_t						mNumberOfIndices;		///< If there's an indirect buffer set, this value is unused
		union
		{
			uint32_t					mInstanceCount;			///< Used in case there's no indirect buffer
			uint32_t					mNumberOfDraws;			///< Used in case there's an indirect buffer
		};
		MaterialResourceId				mMaterialResourceId;
		SkeletonResourceId				mSkeletonResourceId;
		bool							mDrawIndexed;			///< Placed at this location due to padding
		// Cached material data
		uint8_t							mRenderQueueIndex;
		bool							mCastShadows;
		PipelineStateCaches				mPipelineStateCaches;
		// Internal data
		const MaterialResourceManager*	mMaterialResourceManager;
		int								mMaterialResourceAttachmentIndex;


	};


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererRuntime
