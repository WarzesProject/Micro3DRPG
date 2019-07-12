#include "stdafx.h"
#include "Resource/CompositorNode/Pass/Copy/CompositorInstancePassCopy.h"
#include "Resource/CompositorNode/Pass/Copy/CompositorResourcePassCopy.h"
#include "Resource/CompositorNode/CompositorNodeInstance.h"
#include "Resource/CompositorWorkspace/CompositorWorkspaceResourceManager.h"
#include "Resource/CompositorWorkspace/CompositorWorkspaceInstance.h"
#include "Resource/Texture/TextureResourceManager.h"
#include "Resource/Texture/TextureResource.h"
#include "Core/IProfiler.h"
#include "IRendererRuntime.h"


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
namespace RendererRuntime
{


	//[-------------------------------------------------------]
	//[ Protected virtual RendererRuntime::ICompositorInstancePass methods ]
	//[-------------------------------------------------------]
	void CompositorInstancePassCopy::onFillCommandBuffer([[maybe_unused]] const Renderer::IRenderTarget* renderTarget, const CompositorContextData&, Renderer::CommandBuffer& commandBuffer)
	{
		const CompositorResourcePassCopy& compositorResourcePassCopy = static_cast<const CompositorResourcePassCopy&>(getCompositorResourcePass());
		const IRendererRuntime& rendererRuntime = getCompositorNodeInstance().getCompositorWorkspaceInstance().getRendererRuntime();

		// Sanity check
		RENDERER_ASSERT(rendererRuntime.getContext(), nullptr == renderTarget, "The copy compositor instance pass needs an invalid render target")

		// Combined scoped profiler CPU and GPU sample as well as renderer debug event command
		RENDERER_SCOPED_PROFILER_EVENT_DYNAMIC(rendererRuntime.getContext(), commandBuffer, compositorResourcePassCopy.getDebugName())

		// Get destination and source texture resources
		// TODO(co) "RendererRuntime::TextureResourceManager::getTextureResourceByAssetId()" is considered to be inefficient, don't use it in here
		const TextureResourceManager& textureResourceManager = rendererRuntime.getTextureResourceManager();
		const TextureResource* destinationTextureResource = textureResourceManager.getTextureResourceByAssetId(compositorResourcePassCopy.getDestinationTextureAssetId());
		const TextureResource* sourceTextureResource = textureResourceManager.getTextureResourceByAssetId(compositorResourcePassCopy.getSourceTextureAssetId());
		if (nullptr != destinationTextureResource && nullptr != sourceTextureResource)
		{
			const Renderer::ITexturePtr& destinationTexturePtr = destinationTextureResource->getTexturePtr();
			const Renderer::ITexturePtr& sourceTexturePtr = sourceTextureResource->getTexturePtr();
			if (nullptr != destinationTexturePtr && nullptr != sourceTexturePtr)
			{
				Renderer::Command::CopyResource::create(commandBuffer, *destinationTexturePtr, *sourceTexturePtr);
			}
			else
			{
				// Error!
				RENDERER_ASSERT(rendererRuntime.getContext(), false, "We should never end up in here")
			}
		}
		else
		{
			// Error!
			RENDERER_ASSERT(rendererRuntime.getContext(), false, "We should never end up in here")
		}
	}


	//[-------------------------------------------------------]
	//[ Private methods                                       ]
	//[-------------------------------------------------------]
	CompositorInstancePassCopy::CompositorInstancePassCopy(const CompositorResourcePassCopy& compositorResourcePassCopy, const CompositorNodeInstance& compositorNodeInstance) :
		ICompositorInstancePass(compositorResourcePassCopy, compositorNodeInstance)
	{
		// Nothing here
	}


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererRuntime
