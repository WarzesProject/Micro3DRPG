#pragma once

#include "Core/Platform/PlatformTypes.h"

// Disable warnings in external headers, we can't fix them
PRAGMA_WARNING_PUSH
	PRAGMA_WARNING_DISABLE_MSVC(4668)	// warning C4668: '_M_HYBRID_X86_ARM64' is not defined as a preprocessor macro, replacing with '0' for '#if/#elif'
	#include <inttypes.h>	// For uint32_t, uint64_t etc.
PRAGMA_WARNING_POP


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
	*    Abstract file interface
	*
	*  @note
	*    - There's no "seek()" or "tell()" by intent, the renderer toolkit can transform the data so during runtime the most simple file access pattern is sufficient
	*    - It's only called "file interface" because this is usually the name for such a concept, doesn't mean the concrete implementation needs to work with files
	*    - Renderer runtime loaders try to read bigger chunks as often as possible instead of too fine granular byte wise read calls
	*/
	class IFile
	{


	//[-------------------------------------------------------]
	//[ Public virtual RendererRuntime::IFile methods         ]
	//[-------------------------------------------------------]
	public:
		//[-------------------------------------------------------]
		//[ Read                                                  ]
		//[-------------------------------------------------------]
		/**
		*  @brief
		*    Return the number of bytes inside the file
		*
		*  @return
		*    The number of bytes inside the file
		*/
		[[nodiscard]] virtual size_t getNumberOfBytes() = 0;

		/**
		*  @brief
		*    Read a requested number of bytes from the file
		*
		*  @param[out] destinationBuffer
		*    Destination buffer were to write to, must be at least "numberOfBytes" long, never ever a null pointer
		*  @param[in] numberOfBytes
		*    Number of bytes to write into the destination buffer, it's the callers responsibility that this number of byte is correct
		*/
		virtual void read(void* destinationBuffer, size_t numberOfBytes) = 0;

		/**
		*  @brief
		*    Skip a requested number of bytes
		*
		*  @param[in] numberOfBytes
		*    Number of bytes to skip, it's the callers responsibility that this number of byte is correct
		*/
		virtual void skip(size_t numberOfBytes) = 0;

		//[-------------------------------------------------------]
		//[ Write                                                 ]
		//[-------------------------------------------------------]
		/**
		*  @brief
		*    Write a requested number of bytes into the file
		*
		*  @param[in] sourceBuffer
		*    Source buffer were to read from, must be at least "numberOfBytes" long, never ever a null pointer
		*  @param[in] numberOfBytes
		*    Number of bytes to read from the source buffer and write into the file, it's the callers responsibility that this number of byte is correct
		*/
		virtual void write(const void* sourceBuffer, size_t numberOfBytes) = 0;

		//[-------------------------------------------------------]
		//[ Debug                                                 ]
		//[-------------------------------------------------------]
		#ifdef _DEBUG
			/**
			*  @brief
			*    Get debug filename
			*
			*  @return
			*    The debug filename, can be a null pointer, do not free the memory
			*/
			[[nodiscard]] virtual const char* getDebugFilename() const = 0;
		#endif


	//[-------------------------------------------------------]
	//[ Protected methods                                     ]
	//[-------------------------------------------------------]
	protected:
		inline IFile()
		{
			// Nothing here
		}

		inline virtual ~IFile()
		{
			// Nothing here
		}

		explicit IFile(const IFile&) = delete;
		IFile& operator=(const IFile&) = delete;


	};


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererRuntime
