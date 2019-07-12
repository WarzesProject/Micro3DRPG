#include "stdafx.h"
#include "Resource/CompositorNode/Pass/VrHiddenAreaMesh/CompositorResourcePassVrHiddenAreaMesh.h"
#include "Resource/CompositorNode/Loader/CompositorNodeFileFormat.h"


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
namespace RendererRuntime
{


	//[-------------------------------------------------------]
	//[ Public virtual RendererRuntime::ICompositorResourcePass methods ]
	//[-------------------------------------------------------]
	void CompositorResourcePassVrHiddenAreaMesh::deserialize([[maybe_unused]] uint32_t numberOfBytes, const uint8_t* data)
	{
		// Sanity check
		assert(sizeof(v1CompositorNode::PassVrHiddenAreaMesh) == numberOfBytes);

		// Call the base implementation
		ICompositorResourcePass::deserialize(sizeof(v1CompositorNode::Pass), data);

		// Read data
		const v1CompositorNode::PassVrHiddenAreaMesh* passVrHiddenAreaMesh = reinterpret_cast<const v1CompositorNode::PassVrHiddenAreaMesh*>(data);
		mFlags = passVrHiddenAreaMesh->flags;
		mStencil = passVrHiddenAreaMesh->stencil;

		// Sanity checks
		assert((0 != mFlags) && "The VR hidden area mesh compositor resource pass flags must not be null");
		assert(((mFlags & Renderer::ClearFlag::COLOR) == 0) && "The VR hidden area mesh compositor resource pass doesn't support the color flag");
		assert(((mFlags & Renderer::ClearFlag::STENCIL) == 0) && "TODO(co) The VR hidden area mesh compositor resource pass doesn't support the stencil flag, yet");
	}


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererRuntime
