#include "stdafx.h"
#include "RenderQueue/RenderableManager.h"
#include "Core/Math/Transform.h"
#include "Core/GetInvalid.h"


//[-------------------------------------------------------]
//[ Anonymous detail namespace                            ]
//[-------------------------------------------------------]
namespace
{
	namespace detail
	{


		//[-------------------------------------------------------]
		//[ Global variables                                      ]
		//[-------------------------------------------------------]
		const RendererRuntime::Transform IdentityTransform;


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
	RenderableManager::RenderableManager() :
		mTransform(&::detail::IdentityTransform),
		mVisible(true),
		mCachedDistanceToCamera(getInvalid<float>()),
		mMinimumRenderQueueIndex(0),
		mMaximumRenderQueueIndex(0),
		mCastShadows(false)
	{
		// Nothing here
	}

	void RenderableManager::setTransform(const Transform* transform)
	{
		mTransform = (nullptr != transform) ? transform : &::detail::IdentityTransform;
	}

	void RenderableManager::updateCachedRenderablesData()
	{
		if (mRenderables.empty())
		{
			mMinimumRenderQueueIndex = 0;
			mMaximumRenderQueueIndex = 0;
			mCastShadows			 = false;
		}
		else
		{
			{ // Initialize the data using the first renderable
				const Renderable& renderable = mRenderables[0];
				mMinimumRenderQueueIndex = renderable.getRenderQueueIndex();
				mMaximumRenderQueueIndex = mMinimumRenderQueueIndex;
				mCastShadows			 = renderable.getCastShadows();
			}

			// Now incorporate the data from the other renderables
			const size_t numberOfRenderables = mRenderables.size();
			for (size_t i = 1; i < numberOfRenderables; ++i)
			{
				const Renderable& renderable = mRenderables[i];
				const uint8_t renderQueueIndex = renderable.getRenderQueueIndex();
				if (mMinimumRenderQueueIndex > renderQueueIndex)
				{
					mMinimumRenderQueueIndex = renderQueueIndex;
				}
				else if (mMaximumRenderQueueIndex < renderQueueIndex)
				{
					mMaximumRenderQueueIndex = renderQueueIndex;
				}
				if (renderable.getCastShadows())
				{
					mCastShadows = true;
				}
			}
		}
	}


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererRuntime
