#include "stdafx.h"
#include "Core/Renderer/RenderPassManager.h"
#include "Core/Math/Math.h"


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
namespace RendererRuntime
{


	//[-------------------------------------------------------]
	//[ Public methods                                        ]
	//[-------------------------------------------------------]
	RenderPassManager::~RenderPassManager()
	{
		for (auto& renderPassPair : mRenderPasses)
		{
			renderPassPair.second->releaseReference();
		}
	}

	Renderer::IRenderPass* RenderPassManager::getOrCreateRenderPass(uint32_t numberOfColorAttachments, const Renderer::TextureFormat::Enum* colorAttachmentTextureFormats, Renderer::TextureFormat::Enum depthStencilAttachmentTextureFormat, uint8_t numberOfMultisamples)
	{
		// Calculate the render pass signature
		// TODO(co) Tiny performance optimization: It should be possible to pre-calculate a partial render pass signature using "numberOfColorAttachments", "colorAttachmentTextureFormats" and "depthStencilAttachmentTextureFormat" inside the renderer toolkit for the normal use-cases
		uint32_t renderPassSignature = Math::calculateFNV1a32(reinterpret_cast<const uint8_t*>(&numberOfColorAttachments), sizeof(uint32_t), Math::FNV1a_INITIAL_HASH_32);
		for (uint32_t i = 0; i < numberOfColorAttachments; ++i)
		{
			renderPassSignature = Math::calculateFNV1a32(reinterpret_cast<const uint8_t*>(&colorAttachmentTextureFormats[i]), sizeof(Renderer::TextureFormat::Enum), renderPassSignature);
		}
		renderPassSignature = Math::calculateFNV1a32(reinterpret_cast<const uint8_t*>(&depthStencilAttachmentTextureFormat), sizeof(Renderer::TextureFormat::Enum), renderPassSignature);
		renderPassSignature = Math::calculateFNV1a32(&numberOfMultisamples, sizeof(uint8_t), renderPassSignature);

		// Does the render pass instance already exist?
		const RenderPasses::const_iterator iterator = mRenderPasses.find(renderPassSignature);
		if (mRenderPasses.cend() != iterator)
		{
			// The render pass instance already exists
			return iterator->second;
		}
		else
		{
			// Create the render pass instance
			Renderer::IRenderPass* renderPass = mRenderer.createRenderPass(numberOfColorAttachments, colorAttachmentTextureFormats, depthStencilAttachmentTextureFormat, numberOfMultisamples);
			renderPass->addReference();
			mRenderPasses.emplace(renderPassSignature, renderPass);
			return renderPass;
		}
	}


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererRuntime
