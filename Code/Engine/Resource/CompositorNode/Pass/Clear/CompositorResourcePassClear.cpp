#include "stdafx.h"
#include "Resource/CompositorNode/Pass/Clear/CompositorResourcePassClear.h"
#include "Resource/CompositorNode/Loader/CompositorNodeFileFormat.h"

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
	//[ Public virtual RendererRuntime::ICompositorResourcePass methods ]
	//[-------------------------------------------------------]
	void CompositorResourcePassClear::deserialize([[maybe_unused]] uint32_t numberOfBytes, const uint8_t* data)
	{
		// Sanity check
		assert(sizeof(v1CompositorNode::PassClear) == numberOfBytes);

		// Call the base implementation
		ICompositorResourcePass::deserialize(sizeof(v1CompositorNode::Pass), data);

		// Read data
		const v1CompositorNode::PassClear* passClear = reinterpret_cast<const v1CompositorNode::PassClear*>(data);
		mFlags = passClear->flags;
		memcpy(glm::value_ptr(mColor), passClear->color, sizeof(float) * 4);
		mZ = passClear->z;
		mStencil = passClear->stencil;

		// Sanity check
		assert((0 != mFlags) && "The clear compositor resource pass flags must not be null");
	}


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererRuntime
