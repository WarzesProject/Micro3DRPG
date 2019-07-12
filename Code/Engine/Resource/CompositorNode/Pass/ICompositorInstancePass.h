#pragma once


//[-------------------------------------------------------]
//[ Includes                                              ]
//[-------------------------------------------------------]
#include "Core/Platform/PlatformTypes.h"

// Disable warnings in external headers, we can't fix them
PRAGMA_WARNING_PUSH
	PRAGMA_WARNING_DISABLE_MSVC(4668)	// warning C4668: '_M_HYBRID_X86_ARM64' is not defined as a preprocessor macro, replacing with '0' for '#if/#elif'
	#include <inttypes.h>	// For uint32_t, uint64_t etc.
PRAGMA_WARNING_POP


//[-------------------------------------------------------]
//[ Forward declarations                                  ]
//[-------------------------------------------------------]
namespace Renderer
{
	class IRenderTarget;
	class CommandBuffer;
}
namespace RendererRuntime
{
	class CompositorContextData;
	class CompositorNodeInstance;
	class ICompositorResourcePass;
}


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
namespace RendererRuntime
{


	//[-------------------------------------------------------]
	//[ Classes                                               ]
	//[-------------------------------------------------------]
	class ICompositorInstancePass
	{


	//[-------------------------------------------------------]
	//[ Friends                                               ]
	//[-------------------------------------------------------]
		friend class CompositorNodeInstance;		// Needs to execute compositor node instances
		friend class CompositorWorkspaceInstance;	// Needs to be able to set the render target


	//[-------------------------------------------------------]
	//[ Public methods                                        ]
	//[-------------------------------------------------------]
	public:
		[[nodiscard]] inline const ICompositorResourcePass& getCompositorResourcePass() const
		{
			return mCompositorResourcePass;
		}

		[[nodiscard]] inline const CompositorNodeInstance& getCompositorNodeInstance() const
		{
			return mCompositorNodeInstance;
		}

		[[nodiscard]] inline Renderer::IRenderTarget* getRenderTarget() const
		{
			return mRenderTarget;
		}


	//[-------------------------------------------------------]
	//[ Protected virtual RendererRuntime::ICompositorInstancePass methods ]
	//[-------------------------------------------------------]
	protected:
		/**
		*  @brief
		*    Method is called when the owner compositor workspace instance loading has been finished
		*
		*  @note
		*    - A compositor pass instance can e.g. prefetch a render queue index ranges instance in here to avoid repeating this during runtime
		*    - The default implementation is empty
		*/
		inline virtual void onCompositorWorkspaceInstanceLoadingFinished()
		{
			// Nothing here
		}

		/**
		*  @brief
		*    Fill the compositor pass into the given command buffer
		*
		*  @param[in] renderTarget
		*    Render target to render into, can be a null pointer (e.g. for compute shader or resource copy compositor passes)
		*  @param[in] compositorContextData
		*    Compositor context data
		*  @param[out] commandBuffer
		*    Command buffer to fill
		*/
		virtual void onFillCommandBuffer(const Renderer::IRenderTarget* renderTarget, const CompositorContextData& compositorContextData, Renderer::CommandBuffer& commandBuffer) = 0;

		/**
		*  @brief
		*    Called post command buffer execution
		*
		*  @note
		*    - The default implementation is empty
		*/
		inline virtual void onPostCommandBufferExecution()
		{
			// Nothing here
		}


	//[-------------------------------------------------------]
	//[ Protected methods                                     ]
	//[-------------------------------------------------------]
	protected:
		inline ICompositorInstancePass(const ICompositorResourcePass& compositorResourcePass, const CompositorNodeInstance& compositorNodeInstance) :
			mCompositorResourcePass(compositorResourcePass),
			mCompositorNodeInstance(compositorNodeInstance),
			mRenderTarget(nullptr),
			mNumberOfExecutionRequests(0)
		{
			// Nothing here
		}

		inline virtual ~ICompositorInstancePass()
		{
			// Nothing here
		}

		explicit ICompositorInstancePass(const ICompositorInstancePass&) = delete;
		ICompositorInstancePass& operator=(const ICompositorInstancePass&) = delete;


	//[-------------------------------------------------------]
	//[ Private data                                          ]
	//[-------------------------------------------------------]
	private:
		const ICompositorResourcePass& mCompositorResourcePass;
		const CompositorNodeInstance&  mCompositorNodeInstance;
		Renderer::IRenderTarget*	   mRenderTarget;	/// Render target, can be a null pointer, don't destroy the instance
		uint32_t					   mNumberOfExecutionRequests;


	};


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererRuntime
