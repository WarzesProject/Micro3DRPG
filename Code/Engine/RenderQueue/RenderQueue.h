#pragma once


//[-------------------------------------------------------]
//[ Includes                                              ]
//[-------------------------------------------------------]
#include "Resource/ShaderBlueprint/Cache/ShaderProperties.h"
#include "Renderer.h"

// Disable warnings in external headers, we can't fix them
PRAGMA_WARNING_PUSH
	PRAGMA_WARNING_DISABLE_MSVC(4365)	// warning C4365: 'argument': conversion from 'long' to 'unsigned int', signed/unsigned mismatch
	PRAGMA_WARNING_DISABLE_MSVC(5026)	// warning C5026: 'std::_Generic_error_category': move constructor was implicitly defined as deleted
	PRAGMA_WARNING_DISABLE_MSVC(5027)	// warning C5027: 'std::_Generic_error_category': move assignment operator was implicitly defined as deleted
	PRAGMA_WARNING_DISABLE_MSVC(4571)	// warning C4571: Informational: catch(...) semantics changed since Visual C++ 7.1; structured exceptions (SEH) are no longer caught
	PRAGMA_WARNING_DISABLE_MSVC(4625)	// warning C4625: 'std::codecvt_base': copy constructor was implicitly defined as deleted
	PRAGMA_WARNING_DISABLE_MSVC(4626)	// warning C4626: 'std::codecvt<char16_t,char,_Mbstatet>': assignment operator was implicitly defined as deleted
	PRAGMA_WARNING_DISABLE_MSVC(4668)	// warning C4668: '_M_HYBRID_X86_ARM64' is not defined as a preprocessor macro, replacing with '0' for '#if/#elif'
	PRAGMA_WARNING_DISABLE_MSVC(4774)	// warning C4774: 'sprintf_s' : format string expected in argument 3 is not a string literal
	#include <string>
PRAGMA_WARNING_POP


//[-------------------------------------------------------]
//[ Forward declarations                                  ]
//[-------------------------------------------------------]
namespace RendererRuntime
{
	class Renderable;
	class IRendererRuntime;
	class RenderableManager;
	class CompositorContextData;
	class IndirectBufferManager;
}


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
namespace RendererRuntime
{

	class MaterialResource;
	class MaterialTechnique;
	class MaterialBlueprintResource;


	//[-------------------------------------------------------]
	//[ Global definitions                                    ]
	//[-------------------------------------------------------]
	typedef uint32_t MaterialTechniqueId;	///< Material technique identifier, result of hashing the material technique name via "RendererRuntime::StringId"


	//[-------------------------------------------------------]
	//[ Classes                                               ]
	//[-------------------------------------------------------]
	/**
	*  @brief
	*    Render queue
	*
	*  @remarks
	*    Using layered rendering (aka bucketized rendering) concept described in
	*    - "realtimecollisiondetection.net � the blog" - "Order your graphics draw calls around!" by Christer Ericson from October 3, 2008 - http://realtimecollisiondetection.net/blog/?p=86
	*    - "Molecular Musings" - "Stateless, layered, multi-threaded rendering � Part 1" by Stefan Reinalter from November 6, 2014 - https://blog.molecular-matters.com/2014/11/06/stateless-layered-multi-threaded-rendering-part-1/
	*
	*    The sole purpose of the render queue is to fill sorted commands into a given command buffer.
	*/
	class RenderQueue final
	{


	//[-------------------------------------------------------]
	//[ Public methods                                        ]
	//[-------------------------------------------------------]
	public:
		/**
		*  @brief
		*    Constructor
		*
		*  @param[in] indirectBufferManager
		*    Indirect buffer manager to use, must stay valid as long as the indirect buffer manager instance exists
		*  @param[in] minimumRenderQueueIndex
		*    Minimum render queue index (inclusive)
		*  @param[in] maximumRenderQueueIndex
		*    Maximum render queue index (inclusive)
		*  @param[in] transparentPass
		*    "true" if this render queue is used for a transparent render pass, else "false" for opaque render pass (influences the renderables sorting)
		*  @param[in] doSort
		*    Sort renderables?
		*/
		RenderQueue(IndirectBufferManager& indirectBufferManager, uint8_t minimumRenderQueueIndex, uint8_t maximumRenderQueueIndex, bool transparentPass, bool doSort);

		inline ~RenderQueue()
		{
			// Nothing here
		}

		[[nodiscard]] inline uint32_t getNumberOfDrawCalls() const
		{
			return mNumberOfNullDrawCalls + mNumberOfDrawIndexedCalls + mNumberOfDrawCalls;
		}

		[[nodiscard]] inline uint8_t getMinimumRenderQueueIndex() const
		{
			return mMinimumRenderQueueIndex;
		}

		[[nodiscard]] inline uint8_t getMaximumRenderQueueIndex() const
		{
			return mMaximumRenderQueueIndex;
		}

		void clear();
		void addRenderablesFromRenderableManager(const RenderableManager& renderableManager, MaterialTechniqueId materialTechniqueId, const CompositorContextData& compositorContextData, bool castShadows = false);
		void fillGraphicsCommandBuffer(const Renderer::IRenderTarget& renderTarget, const CompositorContextData& compositorContextData, Renderer::CommandBuffer& commandBuffer);
		void fillComputeCommandBuffer(const CompositorContextData& compositorContextData, Renderer::CommandBuffer& commandBuffer);


	//[-------------------------------------------------------]
	//[ Private methods                                       ]
	//[-------------------------------------------------------]
	private:
		explicit RenderQueue(const RenderQueue&) = delete;
		RenderQueue& operator=(const RenderQueue&) = delete;


	//[-------------------------------------------------------]
	//[ Private definitions                                   ]
	//[-------------------------------------------------------]
	private:
		struct QueuedRenderable final
		{
			const Renderable*		   renderable;					///< Always valid, don't destroy the instance
			const MaterialResource*	   materialResource;			///< Always valid, don't destroy the instance
			MaterialTechnique*		   materialTechnique;			///< Always valid, don't destroy the instance
			MaterialBlueprintResource* materialBlueprintResource;	///< Always valid, don't destroy the instance
			Renderer::IPipelineState*  foundPipelineState;			///< Always valid, don't destroy the instance
			uint64_t				   sortingKey;					///< Key used for sorting

			inline QueuedRenderable() :
				renderable(nullptr),
				materialResource(nullptr),
				materialTechnique(nullptr),
				materialBlueprintResource(nullptr),
				foundPipelineState(nullptr),
				sortingKey(0)
			{}
			inline QueuedRenderable(const Renderable& _renderable, const MaterialResource& _materialResource, MaterialTechnique& _materialTechnique, MaterialBlueprintResource& _materialBlueprintResource, Renderer::IPipelineState& _foundPipelineState, uint64_t _sortingKey) :
				renderable(&_renderable),
				materialResource(&_materialResource),
				materialTechnique(&_materialTechnique),
				materialBlueprintResource(&_materialBlueprintResource),
				foundPipelineState(&_foundPipelineState),
				sortingKey(_sortingKey)
			{}
			[[nodiscard]] inline bool operator < (const QueuedRenderable& queuedRenderable) const
			{
				return (sortingKey < queuedRenderable.sortingKey);
			}
		};
		typedef std::vector<QueuedRenderable> QueuedRenderables;

		struct Queue final
		{
			QueuedRenderables queuedRenderables;
			bool			  sorted = false;
		};
		typedef std::vector<Queue> Queues;


	//[-------------------------------------------------------]
	//[ Private data                                          ]
	//[-------------------------------------------------------]
	private:
		const IRendererRuntime&	mRendererRuntime;			///< Renderer runtime instance, we don't own the instance so don't delete it
		IndirectBufferManager&	mIndirectBufferManager;		///< Indirect buffer manager instance, we don't own the instance so don't delete it
		Queues					mQueues;
		uint32_t				mNumberOfNullDrawCalls;
		uint32_t				mNumberOfDrawIndexedCalls;
		uint32_t				mNumberOfDrawCalls;
		uint8_t					mMinimumRenderQueueIndex;	///< Inclusive
		uint8_t					mMaximumRenderQueueIndex;	///< Inclusive
		bool					mTransparentPass;
		bool					mDoSort;
		// Scratch buffers to reduce dynamic memory allocations
		Renderer::CommandBuffer	mScratchCommandBuffer;
		ShaderProperties		mScratchShaderProperties;
		ShaderProperties		mScratchOptimizedShaderProperties;


	};


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererRuntime
