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
	class IIndirectBuffer;
}
namespace RendererRuntime
{
	class IRendererRuntime;
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
	*    Indirect buffer manager
	*/
	class IndirectBufferManager final : private Manager
	{


	//[-------------------------------------------------------]
	//[ Public definitions                                    ]
	//[-------------------------------------------------------]
	public:
		struct IndirectBuffer final
		{
			Renderer::IIndirectBuffer* indirectBuffer;			///< Indirect buffer instance, always valid
			uint32_t				   indirectBufferOffset;	///< Current indirect buffer offset
			uint8_t*				   mappedData;				///< Currently mapped data, don't destroy the data
			explicit IndirectBuffer(Renderer::IIndirectBuffer* _indirectBuffer) :
				indirectBuffer(_indirectBuffer),
				indirectBufferOffset(0),
				mappedData(nullptr)
			{}
		};


	//[-------------------------------------------------------]
	//[ Public methods                                        ]
	//[-------------------------------------------------------]
	public:
		/**
		*  @brief
		*    Constructor
		*
		*  @param[in] rendererRuntime
		*    Renderer runtime instance to use, must stay valid as long as the indirect buffer manager instance exists
		*/
		explicit IndirectBufferManager(const IRendererRuntime& rendererRuntime);

		/**
		*  @brief
		*    Return the renderer runtime instance to use
		*
		*  @return
		*    The renderer runtime instance to use
		*/
		[[nodiscard]] inline const IRendererRuntime& getRendererRuntime() const
		{
			return mRendererRuntime;
		}

		/**
		*  @brief
		*    Destructor
		*/
		~IndirectBufferManager();

		/**
		*  @brief
		*    Return a indirect buffer
		*
		*  @param[in] numberOfBytes
		*    Number of bytes the indirect buffer must be able to hold
		*
		*  @return
		*    The requested indirect buffer, don't destroy the instance, null pointer in case of a horrible nightmare apocalypse scenario
		*/
		[[nodiscard]] IndirectBuffer* getIndirectBuffer(uint32_t numberOfBytes);

		/**
		*  @brief
		*    Called pre command buffer execution
		*/
		void onPreCommandBufferExecution();


	//[-------------------------------------------------------]
	//[ Private methods                                       ]
	//[-------------------------------------------------------]
	private:
		explicit IndirectBufferManager(const IndirectBufferManager&) = delete;
		IndirectBufferManager& operator=(const IndirectBufferManager&) = delete;
		void unmapCurrentIndirectBuffer();


	//[-------------------------------------------------------]
	//[ Private definitions                                   ]
	//[-------------------------------------------------------]
	private:
		typedef std::vector<IndirectBuffer> IndirectBuffers;


	//[-------------------------------------------------------]
	//[ Private data                                          ]
	//[-------------------------------------------------------]
	private:
		const IRendererRuntime& mRendererRuntime;
		uint32_t				mMaximumIndirectBufferSize;	///< Maximum indirect buffer size in bytes
		IndirectBuffers			mFreeIndirectBuffers;
		IndirectBuffers			mUsedIndirectBuffers;
		IndirectBuffer*			mCurrentIndirectBuffer;		///< Currently filled indirect buffer, can be a null pointer, don't destroy the instance since it's just a reference
		uint32_t				mPreviouslyRequestedNumberOfBytes;


	};


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererRuntime
