#include "stdafx.h"
#include "Resource/CompositorNode/Pass/CompositorPassFactory.h"
#include "Resource/CompositorNode/Pass/Clear/CompositorResourcePassClear.h"
#include "Resource/CompositorNode/Pass/Clear/CompositorInstancePassClear.h"
#include "Resource/CompositorNode/Pass/VrHiddenAreaMesh/CompositorResourcePassVrHiddenAreaMesh.h"
#include "Resource/CompositorNode/Pass/VrHiddenAreaMesh/CompositorInstancePassVrHiddenAreaMesh.h"
#include "Resource/CompositorNode/Pass/Compute/CompositorResourcePassCompute.h"
#include "Resource/CompositorNode/Pass/Compute/CompositorInstancePassCompute.h"
#include "Resource/CompositorNode/Pass/Copy/CompositorResourcePassCopy.h"
#include "Resource/CompositorNode/Pass/Copy/CompositorInstancePassCopy.h"
#include "Resource/CompositorNode/Pass/GenerateMipmaps/CompositorResourcePassGenerateMipmaps.h"
#include "Resource/CompositorNode/Pass/GenerateMipmaps/CompositorInstancePassGenerateMipmaps.h"
#include "Resource/CompositorNode/Pass/ShadowMap/CompositorResourcePassShadowMap.h"
#include "Resource/CompositorNode/Pass/ShadowMap/CompositorInstancePassShadowMap.h"
#include "Resource/CompositorNode/Pass/DebugGui/CompositorResourcePassDebugGui.h"
#include "Resource/CompositorNode/Pass/DebugGui/CompositorInstancePassDebugGui.h"
#include "Resource/CompositorNode/Pass/ResolveMultisample/CompositorResourcePassResolveMultisample.h"
#include "Resource/CompositorNode/Pass/ResolveMultisample/CompositorInstancePassResolveMultisample.h"


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
namespace RendererRuntime
{


	//[-------------------------------------------------------]
	//[ Protected virtual RendererRuntime::ICompositorPassFactory methods ]
	//[-------------------------------------------------------]
	ICompositorResourcePass* CompositorPassFactory::createCompositorResourcePass(const CompositorTarget& compositorTarget, CompositorPassTypeId compositorPassTypeId) const
	{
		ICompositorResourcePass* compositorResourcePass = nullptr;

		// Define helper macro
		#define CASE_VALUE(resource) case resource::TYPE_ID: compositorResourcePass = new resource(compositorTarget); break;

		// Evaluate the compositor pass type
		switch (compositorPassTypeId)
		{
			CASE_VALUE(CompositorResourcePassClear)
			CASE_VALUE(CompositorResourcePassVrHiddenAreaMesh)
			CASE_VALUE(CompositorResourcePassScene)
			CASE_VALUE(CompositorResourcePassShadowMap)
			CASE_VALUE(CompositorResourcePassResolveMultisample)
			CASE_VALUE(CompositorResourcePassCopy)
			CASE_VALUE(CompositorResourcePassGenerateMipmaps)
			CASE_VALUE(CompositorResourcePassCompute)
			CASE_VALUE(CompositorResourcePassDebugGui)
		}

		// Undefine helper macro
		#undef CASE_VALUE

		// Done
		return compositorResourcePass;
	}

	ICompositorInstancePass* CompositorPassFactory::createCompositorInstancePass(const ICompositorResourcePass& compositorResourcePass, const CompositorNodeInstance& compositorNodeInstance) const
	{
		ICompositorInstancePass* compositorInstancePass = nullptr;

		// Define helper macro
		#define CASE_VALUE(resource, instance) case resource::TYPE_ID: compositorInstancePass = new instance(static_cast<const resource&>(compositorResourcePass), compositorNodeInstance); break;

		// Evaluate the compositor pass type
		switch (compositorResourcePass.getTypeId())
		{
			CASE_VALUE(CompositorResourcePassClear,				 CompositorInstancePassClear)
			CASE_VALUE(CompositorResourcePassVrHiddenAreaMesh,	 CompositorInstancePassVrHiddenAreaMesh)
			CASE_VALUE(CompositorResourcePassScene,				 CompositorInstancePassScene)
			CASE_VALUE(CompositorResourcePassShadowMap,			 CompositorInstancePassShadowMap)
			CASE_VALUE(CompositorResourcePassResolveMultisample, CompositorInstancePassResolveMultisample)
			CASE_VALUE(CompositorResourcePassCopy,				 CompositorInstancePassCopy)
			CASE_VALUE(CompositorResourcePassGenerateMipmaps,	 CompositorInstancePassGenerateMipmaps)
			CASE_VALUE(CompositorResourcePassCompute,			 CompositorInstancePassCompute)
			CASE_VALUE(CompositorResourcePassDebugGui,			 CompositorInstancePassDebugGui)
		}

		// Undefine helper macro
		#undef CASE_VALUE

		// Done
		return compositorInstancePass;
	}


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererRuntime
