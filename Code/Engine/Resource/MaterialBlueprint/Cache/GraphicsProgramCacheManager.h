#pragma once


//[-------------------------------------------------------]
//[ Includes                                              ]
//[-------------------------------------------------------]
#include "Core/Manager.h"
#include "Core/Platform/PlatformTypes.h"

// Disable warnings in external headers, we can't fix them
PRAGMA_WARNING_PUSH
	PRAGMA_WARNING_DISABLE_MSVC(4548)	// warning C4548: expression before comma has no effect; expected expression with side-effect
	PRAGMA_WARNING_DISABLE_MSVC(4623)	// warning C4623: 'std::_UInt_is_zero': default constructor was implicitly defined as deleted
	PRAGMA_WARNING_DISABLE_MSVC(4626)	// warning C4626: 'std::_UInt_is_zero': assignment operator was implicitly defined as deleted
	PRAGMA_WARNING_DISABLE_MSVC(5027)	// warning C5027: 'std::_UInt_is_zero': move assignment operator was implicitly defined as deleted
	PRAGMA_WARNING_DISABLE_MSVC(5039)	// warning C5039: '_Thrd_start': pointer or reference to potentially throwing function passed to extern C function under -EHc. Undefined behavior may occur if this function throws an exception.
	#include <mutex>
	#include <unordered_map>
PRAGMA_WARNING_POP


//[-------------------------------------------------------]
//[ Forward declarations                                  ]
//[-------------------------------------------------------]
namespace RendererRuntime
{
	class GraphicsProgramCache;
	class GraphicsPipelineStateSignature;
	class GraphicsPipelineStateCacheManager;
}


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
namespace RendererRuntime
{


	//[-------------------------------------------------------]
	//[ Global definitions                                    ]
	//[-------------------------------------------------------]
	typedef uint32_t GraphicsProgramCacheId;	///< Graphics program cache identifier, result of hashing the shader combination IDs of the referenced shaders


	//[-------------------------------------------------------]
	//[ Classes                                               ]
	//[-------------------------------------------------------]
	/*
	*  @brief
	*    Graphics program cache manager
	*
	*  @see
	*    - See "RendererRuntime::GraphicsPipelineStateCacheManager" for additional information
	*/
	class GraphicsProgramCacheManager final : private Manager
	{


	//[-------------------------------------------------------]
	//[ Friends                                               ]
	//[-------------------------------------------------------]
		friend class GraphicsPipelineStateCacheManager;	// Is creating and using a graphics program cache manager instance
		friend class GraphicsPipelineStateCompiler;		// Is tightly interacting with the graphics program cache manager


	//[-------------------------------------------------------]
	//[ Public static methods                                 ]
	//[-------------------------------------------------------]
	public:
		/**
		*  @brief
		*    Generate a graphics program cache ID by using a provided graphics pipeline state signature
		*
		*  @param[in] graphicsPipelineStateSignature
		*    Graphics pipeline state signature to use
		*
		*  @return
		*    The graphics program cache ID
		*/
		[[nodiscard]] static GraphicsProgramCacheId generateGraphicsProgramCacheId(const GraphicsPipelineStateSignature& graphicsPipelineStateSignature);


	//[-------------------------------------------------------]
	//[ Public methods                                        ]
	//[-------------------------------------------------------]
	public:
		/**
		*  @brief
		*    Return the owner graphics pipeline state cache manager
		*
		*  @return
		*    The owner graphics pipeline state cache manager
		*/
		[[nodiscard]] inline GraphicsPipelineStateCacheManager& getGraphicsPipelineStateCacheManager() const
		{
			return mGraphicsPipelineStateCacheManager;
		}

		/**
		*  @brief
		*    Get graphics program cache by graphics pipeline state signature; synchronous processing
		*
		*  @param[in] graphicsPipelineStateSignature
		*    Graphics pipeline state signature to use
		*
		*  @return
		*    The graphics program cache, null pointer on error
		*/
		[[nodiscard]] GraphicsProgramCache* getGraphicsProgramCacheByGraphicsPipelineStateSignature(const GraphicsPipelineStateSignature& graphicsPipelineStateSignature);

		/**
		*  @brief
		*    Clear the graphics program cache manager
		*/
		void clearCache();


	//[-------------------------------------------------------]
	//[ Private methods                                       ]
	//[-------------------------------------------------------]
	private:
		inline explicit GraphicsProgramCacheManager(GraphicsPipelineStateCacheManager& graphicsPipelineStateCacheManager) :
			mGraphicsPipelineStateCacheManager(graphicsPipelineStateCacheManager)
		{
			// Nothing here
		}

		inline ~GraphicsProgramCacheManager()
		{
			clearCache();
		}

		explicit GraphicsProgramCacheManager(const GraphicsProgramCacheManager&) = delete;
		GraphicsProgramCacheManager& operator=(const GraphicsProgramCacheManager&) = delete;


	//[-------------------------------------------------------]
	//[ Private definitions                                   ]
	//[-------------------------------------------------------]
	private:
		typedef std::unordered_map<GraphicsProgramCacheId, GraphicsProgramCache*> GraphicsProgramCacheById;


	//[-------------------------------------------------------]
	//[ Private data                                          ]
	//[-------------------------------------------------------]
	private:
		GraphicsPipelineStateCacheManager& mGraphicsPipelineStateCacheManager;	///< Owner graphics pipeline state cache manager
		GraphicsProgramCacheById		   mGraphicsProgramCacheById;
		std::mutex						   mMutex;								///< Mutex due to "RendererRuntime::GraphicsPipelineStateCompiler" interaction, no too fine granular lock/unlock required because usually it's only asynchronous or synchronous processing, not both at one and the same time


	};


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererRuntime
