#include "stdafx.h"
#include "Resource/CompositorNode/Loader/CompositorNodeResourceLoader.h"
#include "Resource/CompositorNode/Loader/CompositorNodeFileFormat.h"
#include "Resource/CompositorNode/Pass/ICompositorResourcePass.h"
#include "Resource/CompositorNode/CompositorNodeResourceManager.h"
#include "Resource/CompositorNode/CompositorNodeResource.h"


// TODO(co) Error handling


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
		void nodeTargetDeserialization(RendererRuntime::IFile& file, RendererRuntime::CompositorNodeResource& compositorNodeResource, const RendererRuntime::ICompositorPassFactory& compositorPassFactory)
		{
			// Read in the compositor node resource target
			RendererRuntime::v1CompositorNode::Target target;
			file.read(&target, sizeof(RendererRuntime::v1CompositorNode::Target));

			// Create the compositor node resource target instance
			RendererRuntime::CompositorTarget& compositorTarget = compositorNodeResource.addCompositorTarget(target.compositorChannelId, target.compositorFramebufferId);

			// Read in the compositor resource node target passes
			compositorTarget.setNumberOfCompositorResourcePasses(target.numberOfPasses);
			for (uint32_t i = 0; i < target.numberOfPasses; ++i)
			{
				// Read the pass header
				RendererRuntime::v1CompositorNode::PassHeader passHeader;
				file.read(&passHeader, sizeof(RendererRuntime::v1CompositorNode::PassHeader));

				// Create the compositor resource pass
				RendererRuntime::ICompositorResourcePass* compositorResourcePass = compositorTarget.addCompositorResourcePass(compositorPassFactory, passHeader.compositorPassTypeId);

				// Deserialize the compositor resource pass
				if (nullptr != compositorResourcePass && 0 != passHeader.numberOfBytes)
				{
					// Load in the compositor resource pass data
					// TODO(co) Get rid of the new/delete in here
					uint8_t* data = new uint8_t[passHeader.numberOfBytes];
					file.read(data, passHeader.numberOfBytes);

					// Deserialize the compositor resource pass
					compositorResourcePass->deserialize(passHeader.numberOfBytes, data);

					// Cleanup
					delete [] data;
				}
				else
				{
					// TODO(co) Error handling
				}
			}
		}

		void nodeDeserialization(RendererRuntime::IFile& file, const RendererRuntime::v1CompositorNode::CompositorNodeHeader& compositorNodeHeader, RendererRuntime::CompositorNodeResource& compositorNodeResource, const RendererRuntime::ICompositorPassFactory& compositorPassFactory)
		{
			// Read in the compositor resource node input channels
			// TODO(co) Read all input channels in a single burst? (need to introduce a maximum number of input channels for this)
			compositorNodeResource.reserveInputChannels(compositorNodeHeader.numberOfInputChannels);
			for (uint32_t i = 0; i < compositorNodeHeader.numberOfInputChannels; ++i)
			{
				RendererRuntime::CompositorChannelId channelId;
				file.read(&channelId, sizeof(RendererRuntime::CompositorChannelId));
				compositorNodeResource.addInputChannel(channelId);
			}

			// TODO(co) Read all render target textures in a single burst?
			compositorNodeResource.reserveRenderTargetTextures(compositorNodeHeader.numberOfRenderTargetTextures);
			for (uint32_t i = 0; i < compositorNodeHeader.numberOfRenderTargetTextures; ++i)
			{
				RendererRuntime::v1CompositorNode::RenderTargetTexture renderTargetTexture;
				file.read(&renderTargetTexture, sizeof(RendererRuntime::v1CompositorNode::RenderTargetTexture));
				compositorNodeResource.addRenderTargetTexture(renderTargetTexture.assetId, renderTargetTexture.renderTargetTextureSignature);
			}

			// TODO(co) Read all framebuffers in a single burst?
			compositorNodeResource.reserveFramebuffers(compositorNodeHeader.numberOfFramebuffers);
			for (uint32_t i = 0; i < compositorNodeHeader.numberOfFramebuffers; ++i)
			{
				RendererRuntime::v1CompositorNode::Framebuffer framebuffer;
				file.read(&framebuffer, sizeof(RendererRuntime::v1CompositorNode::Framebuffer));
				compositorNodeResource.addFramebuffer(framebuffer.compositorFramebufferId, framebuffer.framebufferSignature);
			}

			// Read in the compositor node resource targets
			compositorNodeResource.reserveCompositorTargets(compositorNodeHeader.numberOfTargets);
			for (uint32_t i = 0; i < compositorNodeHeader.numberOfTargets; ++i)
			{
				nodeTargetDeserialization(file, compositorNodeResource, compositorPassFactory);
			}

			// Read in the compositor resource node output channels
			// TODO(co) Read all output channels in a single burst? (need to introduce a maximum number of output channels for this)
			compositorNodeResource.reserveOutputChannels(compositorNodeHeader.numberOfOutputChannels);
			for (uint32_t i = 0; i < compositorNodeHeader.numberOfOutputChannels; ++i)
			{
				RendererRuntime::CompositorChannelId channelId;
				file.read(&channelId, sizeof(RendererRuntime::CompositorChannelId));
				compositorNodeResource.addOutputChannel(channelId);
			}
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
	//[ Public virtual RendererRuntime::IResourceLoader methods ]
	//[-------------------------------------------------------]
	void CompositorNodeResourceLoader::initialize(const Asset& asset, bool reload, IResource& resource)
	{
		IResourceLoader::initialize(asset, reload);
		mCompositorNodeResource = static_cast<CompositorNodeResource*>(&resource);
	}

	bool CompositorNodeResourceLoader::onDeserialization(IFile& file)
	{
		// Tell the memory mapped file about the LZ4 compressed data
		return mMemoryFile.loadLz4CompressedDataFromFile(v1CompositorNode::FORMAT_TYPE, v1CompositorNode::FORMAT_VERSION, file);
	}

	void CompositorNodeResourceLoader::onProcessing()
	{
		// Decompress LZ4 compressed data
		mMemoryFile.decompress();

		// Read in the compositor node header
		v1CompositorNode::CompositorNodeHeader compositorNodeHeader;
		mMemoryFile.read(&compositorNodeHeader, sizeof(v1CompositorNode::CompositorNodeHeader));

		// Read in the compositor node resource
		::detail::nodeDeserialization(mMemoryFile, compositorNodeHeader, *mCompositorNodeResource, static_cast<CompositorNodeResourceManager&>(getResourceManager()).getCompositorPassFactory());
	}


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererRuntime
