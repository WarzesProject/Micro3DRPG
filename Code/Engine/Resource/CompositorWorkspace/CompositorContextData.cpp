#include "stdafx.h"
#include "Resource/CompositorWorkspace/CompositorContextData.h"
#include "Resource/Scene/Item/Camera/CameraSceneItem.h"
#include "Resource/Scene/SceneNode.h"
#include "Core/Math/Math.h"


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
namespace RendererRuntime
{


	//[-------------------------------------------------------]
	//[ Public methods                                        ]
	//[-------------------------------------------------------]
	CompositorContextData::CompositorContextData(const CompositorWorkspaceInstance* compositorWorkspaceInstance, const CameraSceneItem* cameraSceneItem, bool singlePassStereoInstancing, const LightSceneItem* lightSceneItem, const CompositorInstancePassShadowMap* compositorInstancePassShadowMap) :
		mCompositorWorkspaceInstance(compositorWorkspaceInstance),
		mCameraSceneItem(cameraSceneItem),
		mSinglePassStereoInstancing(singlePassStereoInstancing),
		mLightSceneItem(lightSceneItem),
		mCompositorInstancePassShadowMap(compositorInstancePassShadowMap),
		mWorldSpaceCameraPosition((nullptr != cameraSceneItem) ? cameraSceneItem->getWorldSpaceCameraPosition() : Math::DVEC3_ZERO),
		mCurrentlyBoundMaterialBlueprintResource(nullptr),
		mGlobalComputeSize{0, 0, 0}
	{
		// Nothing here
	}


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererRuntime
