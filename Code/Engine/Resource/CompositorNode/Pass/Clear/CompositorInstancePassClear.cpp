#include "stdafx.h"
#include "Resource/CompositorNode/Pass/Clear/CompositorInstancePassClear.h"
#include "Resource/CompositorNode/Pass/Clear/CompositorResourcePassClear.h"
#include "Core/IProfiler.h"
#include "Renderer.h"

// Disable warnings in external headers, we can't fix them
PRAGMA_WARNING_PUSH
	PRAGMA_WARNING_DISABLE_MSVC(4201)	// warning C4201: nonstandard extension used: nameless struct/union
	PRAGMA_WARNING_DISABLE_MSVC(4464)	// warning C4464: relative include path contains '..'
	#include <glm/gtc/type_ptr.hpp>
PRAGMA_WARNING_POP


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
namespace RendererRuntime
{


	//[-------------------------------------------------------]
	//[ Protected virtual RendererRuntime::ICompositorInstancePass methods ]
	//[-------------------------------------------------------]
	void CompositorInstancePassClear::onFillCommandBuffer([[maybe_unused]] const Renderer::IRenderTarget* renderTarget, const CompositorContextData&, Renderer::CommandBuffer& commandBuffer)
	{
		// Sanity check
		RENDERER_ASSERT(getCompositorNodeInstance().getCompositorWorkspaceInstance().getRendererRuntime().getContext(), nullptr != renderTarget, "The clear compositor instance pass needs a valid render target")

		// Combined scoped profiler CPU and GPU sample as well as renderer debug event command
		const CompositorResourcePassClear& compositorResourcePassClear = static_cast<const CompositorResourcePassClear&>(getCompositorResourcePass());
		RENDERER_SCOPED_PROFILER_EVENT_DYNAMIC(getCompositorNodeInstance().getCompositorWorkspaceInstance().getRendererRuntime().getContext(), commandBuffer, compositorResourcePassClear.getDebugName())

		// Push the clear graphics command
		Renderer::Command::ClearGraphics::create(commandBuffer, compositorResourcePassClear.getFlags(), glm::value_ptr(compositorResourcePassClear.getClearColor()), compositorResourcePassClear.getZ(), compositorResourcePassClear.getStencil());
	}


	//[-------------------------------------------------------]
	//[ Private methods                                       ]
	//[-------------------------------------------------------]
	CompositorInstancePassClear::CompositorInstancePassClear(const CompositorResourcePassClear& compositorResourcePassClear, const CompositorNodeInstance& compositorNodeInstance) :
		ICompositorInstancePass(compositorResourcePassClear, compositorNodeInstance)
	{
		// Nothing here
	}


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererRuntime
