#include "stdafx.h"
#include "Core/Renderer/FramebufferManager.h"
#include "Core/Renderer/RenderPassManager.h"
#include "Core/Renderer/RenderTargetTextureManager.h"
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
		//[ Global functions                                      ]
		//[-------------------------------------------------------]
		[[nodiscard]] inline bool orderFramebufferElementByFramebufferSignatureId(const RendererRuntime::FramebufferManager::FramebufferElement& left, const RendererRuntime::FramebufferManager::FramebufferElement& right)
		{
			return (left.framebufferSignature.getFramebufferSignatureId() < right.framebufferSignature.getFramebufferSignatureId());
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
	//[ Public methods                                        ]
	//[-------------------------------------------------------]
	void FramebufferManager::clear()
	{
		clearRendererResources();
		mSortedFramebufferVector.clear();
		mCompositorFramebufferIdToFramebufferSignatureId.clear();
	}

	void FramebufferManager::clearRendererResources()
	{
		for (FramebufferElement& framebufferElement : mSortedFramebufferVector)
		{
			if (nullptr != framebufferElement.framebuffer)
			{
				framebufferElement.framebuffer->releaseReference();
				framebufferElement.framebuffer = nullptr;
			}
		}
	}

	void FramebufferManager::addFramebuffer(CompositorFramebufferId compositorFramebufferId, const FramebufferSignature& framebufferSignature)
	{
		FramebufferElement framebufferElement(framebufferSignature);
		SortedFramebufferVector::iterator iterator = std::lower_bound(mSortedFramebufferVector.begin(), mSortedFramebufferVector.end(), framebufferElement, ::detail::orderFramebufferElementByFramebufferSignatureId);
		if (iterator == mSortedFramebufferVector.end() || iterator->framebufferSignature.getFramebufferSignatureId() != framebufferElement.framebufferSignature.getFramebufferSignatureId())
		{
			// Add new framebuffer

			// Register the new framebuffer element
			++framebufferElement.numberOfReferences;
			mSortedFramebufferVector.insert(iterator, framebufferElement);
		}
		else
		{
			// Just increase the number of references
			++iterator->numberOfReferences;
		}
		mCompositorFramebufferIdToFramebufferSignatureId.emplace(compositorFramebufferId, framebufferSignature.getFramebufferSignatureId());
	}

	Renderer::IFramebuffer* FramebufferManager::getFramebufferByCompositorFramebufferId(CompositorFramebufferId compositorFramebufferId) const
	{
		Renderer::IFramebuffer* framebuffer = nullptr;

		// Map compositor framebuffer ID to framebuffer signature ID
		CompositorFramebufferIdToFramebufferSignatureId::const_iterator iterator = mCompositorFramebufferIdToFramebufferSignatureId.find(compositorFramebufferId);
		if (mCompositorFramebufferIdToFramebufferSignatureId.cend() != iterator)
		{
			// TODO(co) Is there need for a more efficient search?
			const FramebufferSignatureId framebufferSignatureId = iterator->second;
			for (const FramebufferElement& framebufferElement : mSortedFramebufferVector)
			{
				const FramebufferSignature& framebufferSignature = framebufferElement.framebufferSignature;
				if (framebufferSignature.getFramebufferSignatureId() == framebufferSignatureId)
				{
					framebuffer = framebufferElement.framebuffer;
					break;
				}
			}
			assert(nullptr != framebuffer);
		}
		else
		{
			// Error! Unknown compositor framebuffer ID, this shouldn't have happened.
			assert(false);
		}

		// Done
		return framebuffer;
	}

	Renderer::IFramebuffer* FramebufferManager::getFramebufferByCompositorFramebufferId(CompositorFramebufferId compositorFramebufferId, const Renderer::IRenderTarget& renderTarget, uint8_t numberOfMultisamples, float resolutionScale)
	{
		Renderer::IFramebuffer* framebuffer = nullptr;

		// Map compositor framebuffer ID to framebuffer signature ID
		CompositorFramebufferIdToFramebufferSignatureId::const_iterator iterator = mCompositorFramebufferIdToFramebufferSignatureId.find(compositorFramebufferId);
		if (mCompositorFramebufferIdToFramebufferSignatureId.cend() != iterator)
		{
			// TODO(co) Is there need for a more efficient search?
			const FramebufferSignatureId framebufferSignatureId = iterator->second;
			for (FramebufferElement& framebufferElement : mSortedFramebufferVector)
			{
				const FramebufferSignature& framebufferSignature = framebufferElement.framebufferSignature;
				if (framebufferSignature.getFramebufferSignatureId() == framebufferSignatureId)
				{
					// Do we need to create the renderer framebuffer instance right now?
					if (nullptr == framebufferElement.framebuffer)
					{
						// Get the color texture instances
						Renderer::TextureFormat::Enum colorTextureFormats[8] = { Renderer::TextureFormat::Enum::UNKNOWN, Renderer::TextureFormat::Enum::UNKNOWN, Renderer::TextureFormat::Enum::UNKNOWN, Renderer::TextureFormat::Enum::UNKNOWN, Renderer::TextureFormat::Enum::UNKNOWN, Renderer::TextureFormat::Enum::UNKNOWN, Renderer::TextureFormat::Enum::UNKNOWN, Renderer::TextureFormat::Enum::UNKNOWN };
						const uint8_t numberOfColorAttachments = framebufferSignature.getNumberOfColorAttachments();
						assert(numberOfColorAttachments < 8);
						Renderer::FramebufferAttachment colorFramebufferAttachments[8];
						uint8_t usedNumberOfMultisamples = 0;
						for (uint8_t i = 0; i < numberOfColorAttachments; ++i)
						{
							const FramebufferSignatureAttachment& framebufferSignatureAttachment = framebufferSignature.getColorFramebufferSignatureAttachment(i);
							const AssetId colorTextureAssetId = framebufferSignatureAttachment.textureAssetId;
							const RenderTargetTextureSignature* colorRenderTargetTextureSignature = nullptr;
							Renderer::FramebufferAttachment& framebufferAttachment = colorFramebufferAttachments[i];
							framebufferAttachment.texture = isValid(colorTextureAssetId) ? mRenderTargetTextureManager.getTextureByAssetId(colorTextureAssetId, renderTarget, numberOfMultisamples, resolutionScale, &colorRenderTargetTextureSignature) : nullptr;
							assert(nullptr != framebufferAttachment.texture);
							framebufferAttachment.mipmapIndex = framebufferSignatureAttachment.mipmapIndex;
							framebufferAttachment.layerIndex = framebufferSignatureAttachment.layerIndex;
							assert(nullptr != colorRenderTargetTextureSignature);
							if (0 == usedNumberOfMultisamples)
							{
								usedNumberOfMultisamples = ((colorRenderTargetTextureSignature->getFlags() & RenderTargetTextureSignature::Flag::ALLOW_MULTISAMPLE) != 0) ? numberOfMultisamples : 1u;
							}
							else
							{
								assert(1 == usedNumberOfMultisamples || ((colorRenderTargetTextureSignature->getFlags() & RenderTargetTextureSignature::Flag::ALLOW_MULTISAMPLE) != 0));
							}
							colorTextureFormats[i] = colorRenderTargetTextureSignature->getTextureFormat();
						}

						// Get the depth stencil texture instances
						const FramebufferSignatureAttachment& depthStencilFramebufferSignatureAttachment = framebufferSignature.getDepthStencilFramebufferSignatureAttachment();
						const RenderTargetTextureSignature* depthStencilRenderTargetTextureSignature = nullptr;
						Renderer::FramebufferAttachment depthStencilFramebufferAttachment(isValid(depthStencilFramebufferSignatureAttachment.textureAssetId) ? mRenderTargetTextureManager.getTextureByAssetId(depthStencilFramebufferSignatureAttachment.textureAssetId, renderTarget, numberOfMultisamples, resolutionScale, &depthStencilRenderTargetTextureSignature) : nullptr, depthStencilFramebufferSignatureAttachment.mipmapIndex, depthStencilFramebufferSignatureAttachment.layerIndex);
						if (nullptr != depthStencilRenderTargetTextureSignature)
						{
							if (0 == usedNumberOfMultisamples)
							{
								usedNumberOfMultisamples = ((depthStencilRenderTargetTextureSignature->getFlags() & RenderTargetTextureSignature::Flag::ALLOW_MULTISAMPLE) != 0) ? numberOfMultisamples : 1u;
							}
							else
							{
								assert(1 == usedNumberOfMultisamples || ((depthStencilRenderTargetTextureSignature->getFlags() & RenderTargetTextureSignature::Flag::ALLOW_MULTISAMPLE) != 0));
							}
						}
						const Renderer::TextureFormat::Enum depthStencilTextureFormat = (nullptr != depthStencilRenderTargetTextureSignature) ? depthStencilRenderTargetTextureSignature->getTextureFormat() : Renderer::TextureFormat::Enum::UNKNOWN;

						// Get or create the managed render pass
						Renderer::IRenderPass* renderPass = mRenderPassManager.getOrCreateRenderPass(numberOfColorAttachments, colorTextureFormats, depthStencilTextureFormat, usedNumberOfMultisamples);
						assert(nullptr != renderPass);

						// Create the framebuffer object (FBO) instance
						// -> The framebuffer automatically adds a reference to the provided textures
						framebufferElement.framebuffer = mRenderTargetTextureManager.getRendererRuntime().getRenderer().createFramebuffer(*renderPass, colorFramebufferAttachments, (nullptr != depthStencilFramebufferAttachment.texture) ? &depthStencilFramebufferAttachment : nullptr);
						RENDERER_SET_RESOURCE_DEBUG_NAME(framebufferElement.framebuffer, "Framebuffer manager")
						framebufferElement.framebuffer->addReference();
					}
					framebuffer = framebufferElement.framebuffer;
					break;
				}
			}
			assert(nullptr != framebuffer);
		}
		else
		{
			// Error! Unknown compositor framebuffer ID, this shouldn't have happened.
			assert(false);
		}

		// Done
		return framebuffer;
	}

	void FramebufferManager::releaseFramebufferBySignature(const FramebufferSignature& framebufferSignature)
	{
		const FramebufferElement framebufferElement(framebufferSignature);
		SortedFramebufferVector::iterator iterator = std::lower_bound(mSortedFramebufferVector.begin(), mSortedFramebufferVector.end(), framebufferElement, ::detail::orderFramebufferElementByFramebufferSignatureId);
		if (iterator != mSortedFramebufferVector.end() && iterator->framebufferSignature.getFramebufferSignatureId() == framebufferElement.framebufferSignature.getFramebufferSignatureId())
		{
			// Was this the last reference?
			if (1 == iterator->numberOfReferences)
			{
				if (nullptr != iterator->framebuffer)
				{
					iterator->framebuffer->releaseReference();
				}
				mSortedFramebufferVector.erase(iterator);
			}
			else
			{
				--iterator->numberOfReferences;
			}
		}
		else
		{
			// Error! Framebuffer signature isn't registered.
			assert(false);
		}
	}


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererRuntime
