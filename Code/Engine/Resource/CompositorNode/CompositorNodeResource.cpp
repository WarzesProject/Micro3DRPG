#include "stdafx.h"
#include "Resource/CompositorNode/CompositorNodeResource.h"
#include "Resource/CompositorNode/CompositorNodeResourceManager.h"
#include "Resource/ResourceStreamer.h"
#include "IRendererRuntime.h"


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
namespace RendererRuntime
{


	//[-------------------------------------------------------]
	//[ Public methods                                        ]
	//[-------------------------------------------------------]
	void CompositorNodeResource::enforceFullyLoaded()
	{
		// TODO(co) Implement more efficient solution: We need to extend "Runtime::ResourceStreamer" to request emergency immediate processing of requested resources
		ResourceStreamer& resourceStreamer = getResourceManager<CompositorNodeResourceManager>().getRendererRuntime().getResourceStreamer();
		while (IResource::LoadingState::LOADED != getLoadingState())
		{
			using namespace std::chrono_literals;
			std::this_thread::sleep_for(1ms);
			resourceStreamer.dispatch();
		}
	}


	//[-------------------------------------------------------]
	//[ Private methods                                       ]
	//[-------------------------------------------------------]
	void CompositorNodeResource::deinitializeElement()
	{
		mInputChannels.clear();
		mCompositorRenderTargetTextures.clear();
		mCompositorFramebuffers.clear();
		mCompositorTargets.clear();
		mOutputChannels.clear();

		// Call base implementation
		IResource::deinitializeElement();
	}


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererRuntime
