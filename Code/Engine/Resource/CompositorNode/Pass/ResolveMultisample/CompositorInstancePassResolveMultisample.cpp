#include "stdafx.h"
#include "Resource/CompositorNode/Pass/ResolveMultisample/CompositorInstancePassResolveMultisample.h"
#include "Resource/CompositorNode/Pass/ResolveMultisample/CompositorResourcePassResolveMultisample.h"
#include "Resource/CompositorNode/CompositorNodeInstance.h"
#include "Resource/CompositorWorkspace/CompositorWorkspaceResourceManager.h"
#include "Resource/CompositorWorkspace/CompositorWorkspaceInstance.h"
#include "Core/Renderer/FramebufferManager.h"
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
	void CompositorInstancePassResolveMultisample::onFillCommandBuffer(const Renderer::IRenderTarget* renderTarget, const CompositorContextData&, Renderer::CommandBuffer& commandBuffer)
	{
		const IRendererRuntime& rendererRuntime = getCompositorNodeInstance().getCompositorWorkspaceInstance().getRendererRuntime();

		// Sanity check
		RENDERER_ASSERT(rendererRuntime.getContext(), nullptr != renderTarget, "The resolve multisample compositor instance pass needs a valid render target")

		// Combined scoped profiler CPU and GPU sample as well as renderer debug event command
		RENDERER_SCOPED_PROFILER_EVENT_DYNAMIC(rendererRuntime.getContext(), commandBuffer, getCompositorResourcePass().getDebugName())

		// Resolve
		Renderer::IFramebuffer* framebuffer = rendererRuntime.getCompositorWorkspaceResourceManager().getFramebufferManager().getFramebufferByCompositorFramebufferId(static_cast<const CompositorResourcePassResolveMultisample&>(getCompositorResourcePass()).getSourceMultisampleCompositorFramebufferId());
		if (nullptr != framebuffer)
		{
			// TODO(co) Get rid of the evil const-cast
			Renderer::Command::ResolveMultisampleFramebuffer::create(commandBuffer, const_cast<Renderer::IRenderTarget&>(*renderTarget), *framebuffer);
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
	CompositorInstancePassResolveMultisample::CompositorInstancePassResolveMultisample(const CompositorResourcePassResolveMultisample& compositorResourcePassResolveMultisample, const CompositorNodeInstance& compositorNodeInstance) :
		ICompositorInstancePass(compositorResourcePassResolveMultisample, compositorNodeInstance)
	{
		// Nothing here
	}


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererRuntime
