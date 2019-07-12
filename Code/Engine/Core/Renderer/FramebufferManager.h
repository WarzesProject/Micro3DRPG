#pragma once


//[-------------------------------------------------------]
//[ Includes                                              ]
//[-------------------------------------------------------]
#include "Core/Manager.h"
#include "Core/Renderer/FramebufferSignature.h"

// Disable warnings in external headers, we can't fix them
PRAGMA_WARNING_PUSH
	PRAGMA_WARNING_DISABLE_MSVC(4365)	// warning C4365: 'argument': conversion from 'long' to 'unsigned int', signed/unsigned mismatch
	PRAGMA_WARNING_DISABLE_MSVC(4571)	// warning C4571: Informational: catch(...) semantics changed since Visual C++ 7.1; structured exceptions (SEH) are no longer caught
	PRAGMA_WARNING_DISABLE_MSVC(4668)	// warning C4668: '_M_HYBRID_X86_ARM64' is not defined as a preprocessor macro, replacing with '0' for '#if/#elif'
	#include <vector>
PRAGMA_WARNING_POP
#include <unordered_map>


//[-------------------------------------------------------]
//[ Forward declarations                                  ]
//[-------------------------------------------------------]
namespace Renderer
{
	class IFramebuffer;
	class IRenderTarget;
}
namespace RendererRuntime
{
	class RenderPassManager;
	class RenderTargetTextureManager;
}


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
namespace RendererRuntime
{


	//[-------------------------------------------------------]
	//[ Global definitions                                    ]
	//[-------------------------------------------------------]
	typedef StringId CompositorFramebufferId;	///< Compositor framebuffer identifier, internally just a POD "uint32_t"


	//[-------------------------------------------------------]
	//[ Classes                                               ]
	//[-------------------------------------------------------]
	class FramebufferManager final : private Manager
	{


	//[-------------------------------------------------------]
	//[ Public definitions                                    ]
	//[-------------------------------------------------------]
	public:
		struct FramebufferElement final
		{
			FramebufferSignature    framebufferSignature;
			Renderer::IFramebuffer* framebuffer;		///< Can be a null pointer, no "Renderer::IFramebufferPtr" to not have overhead when internally reallocating
			uint32_t				numberOfReferences;	///< Number of framebuffer references (don't misuse the renderer framebuffer reference counter for this)

			inline FramebufferElement() :
				framebuffer(nullptr),
				numberOfReferences(0)
			{
				// Nothing here
			}

			inline explicit FramebufferElement(const FramebufferSignature& _framebufferSignature) :
				framebufferSignature(_framebufferSignature),
				framebuffer(nullptr),
				numberOfReferences(0)
			{
				// Nothing here
			}

			inline FramebufferElement(const FramebufferSignature& _framebufferSignature, Renderer::IFramebuffer& _framebuffer) :
				framebufferSignature(_framebufferSignature),
				framebuffer(&_framebuffer),
				numberOfReferences(0)
			{
				// Nothing here
			}
		};


	//[-------------------------------------------------------]
	//[ Public methods                                        ]
	//[-------------------------------------------------------]
	public:
		inline FramebufferManager(RenderTargetTextureManager& renderTargetTextureManager, RenderPassManager& renderPassManager) :
			mRenderTargetTextureManager(renderTargetTextureManager),
			mRenderPassManager(renderPassManager)
		{
			// Nothing here
		}

		inline ~FramebufferManager()
		{
			// Nothing here
		}

		explicit FramebufferManager(const FramebufferManager&) = delete;
		FramebufferManager& operator=(const FramebufferManager&) = delete;
		void clear();
		void clearRendererResources();
		void addFramebuffer(CompositorFramebufferId compositorFramebufferId, const FramebufferSignature& framebufferSignature);
		[[nodiscard]] Renderer::IFramebuffer* getFramebufferByCompositorFramebufferId(CompositorFramebufferId compositorFramebufferId) const;
		[[nodiscard]] Renderer::IFramebuffer* getFramebufferByCompositorFramebufferId(CompositorFramebufferId compositorFramebufferId, const Renderer::IRenderTarget& mainRenderTarget, uint8_t numberOfMultisamples, float resolutionScale);
		void releaseFramebufferBySignature(const FramebufferSignature& framebufferSignature);


	//[-------------------------------------------------------]
	//[ Private definitions                                   ]
	//[-------------------------------------------------------]
	private:
		typedef std::vector<FramebufferElement> SortedFramebufferVector;
		typedef std::unordered_map<uint32_t, FramebufferSignatureId> CompositorFramebufferIdToFramebufferSignatureId;	///< Key = "RendererRuntime::CompositorFramebufferId"


	//[-------------------------------------------------------]
	//[ Private data                                          ]
	//[-------------------------------------------------------]
	private:
		RenderTargetTextureManager&						mRenderTargetTextureManager;	///< Render target texture manager, just shared so don't destroy the instance
		RenderPassManager&								mRenderPassManager;				///< Render pass manager, just shared so don't destroy the instance
		SortedFramebufferVector							mSortedFramebufferVector;
		CompositorFramebufferIdToFramebufferSignatureId	mCompositorFramebufferIdToFramebufferSignatureId;


	};


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererRuntime
