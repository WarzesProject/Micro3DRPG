#include "stdafx.h"
#include "Resource/CompositorNode/CompositorTarget.h"
#include "Resource/CompositorNode/Pass/ICompositorPassFactory.h"
#include "Resource/CompositorNode/Pass/ICompositorResourcePass.h"


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
namespace RendererRuntime
{


	//[-------------------------------------------------------]
	//[ Public methods                                        ]
	//[-------------------------------------------------------]
	ICompositorResourcePass* CompositorTarget::addCompositorResourcePass(const ICompositorPassFactory& compositorPassFactory, CompositorPassTypeId compositorPassTypeId)
	{
		ICompositorResourcePass* compositorResourcePass = compositorPassFactory.createCompositorResourcePass(*this, compositorPassTypeId);
		mCompositorResourcePasses.push_back(compositorResourcePass);
		return compositorResourcePass;
	}

	void CompositorTarget::removeAllCompositorResourcePasses()
	{
		const size_t numberOfCompositorResourcePasses = mCompositorResourcePasses.size();
		for (size_t i = 0; i < numberOfCompositorResourcePasses; ++i)
		{
			delete mCompositorResourcePasses[i];
		}
		mCompositorResourcePasses.clear();
	}


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererRuntime
