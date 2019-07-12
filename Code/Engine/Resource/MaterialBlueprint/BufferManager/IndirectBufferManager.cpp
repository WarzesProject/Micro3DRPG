#include "stdafx.h"
#include "Resource/MaterialBlueprint/BufferManager/IndirectBufferManager.h"
#include "IRendererRuntime.h"

#include <algorithm>


//[-------------------------------------------------------]
//[ Anonymous detail namespace                            ]
//[-------------------------------------------------------]
namespace
{
	namespace detail
	{


		//[-------------------------------------------------------]
		//[ Global definitions                                    ]
		//[-------------------------------------------------------]
		static uint32_t DEFAULT_INDIRECT_BUFFER_NUMBER_OF_BYTES = 128 * 1024;	// 128 KiB


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
	//[ Public methods                                        ]
	//[-------------------------------------------------------]
	IndirectBufferManager::IndirectBufferManager(const IRendererRuntime& rendererRuntime) :
		mRendererRuntime(rendererRuntime),
		mMaximumIndirectBufferSize(std::min(rendererRuntime.getRenderer().getCapabilities().maximumIndirectBufferSize, ::detail::DEFAULT_INDIRECT_BUFFER_NUMBER_OF_BYTES)),
		mCurrentIndirectBuffer(nullptr),
		mPreviouslyRequestedNumberOfBytes(0)
	{
		// The maximum indirect buffer size must be a multiple of "Renderer::DrawIndexedArguments"
		mMaximumIndirectBufferSize -= (mMaximumIndirectBufferSize % sizeof(Renderer::DrawIndexedArguments));
	}

	IndirectBufferManager::~IndirectBufferManager()
	{
		// At this point in time, no indirect buffers should be in use anymore
		RENDERER_ASSERT(mRendererRuntime.getContext(), mUsedIndirectBuffers.empty(), "Invalid used indirect buffers")
		RENDERER_ASSERT(mRendererRuntime.getContext(), nullptr == mCurrentIndirectBuffer, "Invalid current indirect buffer")
		RENDERER_ASSERT(mRendererRuntime.getContext(), 0 == mPreviouslyRequestedNumberOfBytes, "Invalid previously requested number of bytes")

		// Destroy all indirect buffers
		for (IndirectBuffer& indirectBuffer : mFreeIndirectBuffers)
		{
			indirectBuffer.indirectBuffer->releaseReference();
		}
	}

	IndirectBufferManager::IndirectBuffer* IndirectBufferManager::getIndirectBuffer(uint32_t numberOfBytes)
	{
		// Sanity check
		RENDERER_ASSERT(mRendererRuntime.getContext(), numberOfBytes > 0, "Don't call this method if there's no work to be done")
		RENDERER_ASSERT(mRendererRuntime.getContext(), numberOfBytes <= mMaximumIndirectBufferSize, "Maximum indirect buffer size exceeded")

		// Is there enough space left inside the current indirect buffer?
		if (nullptr != mCurrentIndirectBuffer)
		{
			// Advance indirect buffer offset using the previously requested number of bytes which are consumed now
			mCurrentIndirectBuffer->indirectBufferOffset += mPreviouslyRequestedNumberOfBytes;
			if ((mCurrentIndirectBuffer->indirectBufferOffset + numberOfBytes) > mMaximumIndirectBufferSize)
			{
				// Out of space
				unmapCurrentIndirectBuffer();
			}
		}
		mPreviouslyRequestedNumberOfBytes = numberOfBytes;

		// Create new indirect buffer, if required
		if (nullptr == mCurrentIndirectBuffer)
		{
			if (mFreeIndirectBuffers.empty())
			{
				// Create new indirect buffer instance
				Renderer::IIndirectBuffer* rendererIndirectBuffer = mRendererRuntime.getBufferManager().createIndirectBuffer(mMaximumIndirectBufferSize, nullptr, Renderer::IndirectBufferFlag::DRAW_INDEXED_ARGUMENTS, Renderer::BufferUsage::DYNAMIC_DRAW);
				RENDERER_SET_RESOURCE_DEBUG_NAME(rendererIndirectBuffer, "Indirect buffer manager")
				mUsedIndirectBuffers.emplace_back(rendererIndirectBuffer);
			}
			else
			{
				// Use existing free indirect buffer instance
				mUsedIndirectBuffers.push_back(mFreeIndirectBuffers.back());
				mFreeIndirectBuffers.pop_back();
			}
			mCurrentIndirectBuffer = &mUsedIndirectBuffers.back();

			{ // Map
				// Sanity checks
				RENDERER_ASSERT(mRendererRuntime.getContext(), nullptr != mCurrentIndirectBuffer->indirectBuffer, "Invalid current indirect buffer")
				RENDERER_ASSERT(mRendererRuntime.getContext(), 0 == mCurrentIndirectBuffer->indirectBufferOffset, "Invalid current indirect buffer")
				RENDERER_ASSERT(mRendererRuntime.getContext(), nullptr == mCurrentIndirectBuffer->mappedData, "Invalid current indirect buffer")

				// Map
				Renderer::MappedSubresource mappedSubresource;
				if (mRendererRuntime.getRenderer().map(*mCurrentIndirectBuffer->indirectBuffer, 0, Renderer::MapType::WRITE_DISCARD, 0, mappedSubresource))
				{
					mCurrentIndirectBuffer->mappedData = static_cast<uint8_t*>(mappedSubresource.data);
				}
				RENDERER_ASSERT(mRendererRuntime.getContext(), nullptr != mCurrentIndirectBuffer->mappedData, "Invalid current indirect buffer")
			}
		}

		// Done
		return mCurrentIndirectBuffer;
	}

	void IndirectBufferManager::onPreCommandBufferExecution()
	{
		// Unmap current indirect buffer
		if (nullptr != mCurrentIndirectBuffer)
		{
			unmapCurrentIndirectBuffer();
		}

		// Free all used indirect buffers
		mFreeIndirectBuffers.insert(mFreeIndirectBuffers.end(), mUsedIndirectBuffers.begin(), mUsedIndirectBuffers.end());
		mUsedIndirectBuffers.clear();
	}


	//[-------------------------------------------------------]
	//[ Private methods                                       ]
	//[-------------------------------------------------------]
	void IndirectBufferManager::unmapCurrentIndirectBuffer()
	{
		// Sanity checks
		RENDERER_ASSERT(mRendererRuntime.getContext(), nullptr != mCurrentIndirectBuffer, "Invalid current indirect buffer")
		RENDERER_ASSERT(mRendererRuntime.getContext(), nullptr != mCurrentIndirectBuffer->mappedData, "Invalid current indirect buffer")

		// Unmap
		mRendererRuntime.getRenderer().unmap(*mCurrentIndirectBuffer->indirectBuffer, 0);
		mCurrentIndirectBuffer->indirectBufferOffset = 0;
		mCurrentIndirectBuffer->mappedData = nullptr;
		mCurrentIndirectBuffer = nullptr;
		mPreviouslyRequestedNumberOfBytes = 0;
	}


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererRuntime
