#include "stdafx.h"
#include "Resource/Scene/Item/ISceneItem.h"
#include "Resource/Scene/SceneResource.h"
#include "Resource/Scene/Culling/SceneItemSet.h"
#include "Resource/Scene/Culling/SceneCullingManager.h"
#include "IRendererRuntime.h"

//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
namespace RendererRuntime
{


	//[-------------------------------------------------------]
	//[ Public methods                                        ]
	//[-------------------------------------------------------]
	Context& ISceneItem::getContext() const
	{
		return mSceneResource.getRendererRuntime().getContext();
	}


	//[-------------------------------------------------------]
	//[ Protected methods                                     ]
	//[-------------------------------------------------------]
	ISceneItem::ISceneItem(SceneResource& sceneResource, bool cullable) :
		mSceneResource(sceneResource),
		mParentSceneNode(nullptr),
		mSceneItemSet(nullptr),
		mSceneItemSetIndex(getInvalid<uint32_t>())
	{
		// TODO(co) The following is just for culling kickoff and won't stay this way
		if (cullable)
		{
			mSceneItemSet = &mSceneResource.getSceneCullingManager().getCullableSceneItemSet();
			mSceneItemSetIndex = mSceneItemSet->numberOfSceneItems;

			// Set minimum object space bounding box corner position
			mSceneItemSet->minimumX.push_back(-0.5f);
			mSceneItemSet->minimumY.push_back(-0.5f);
			mSceneItemSet->minimumZ.push_back(-0.5f);

			// Set maximum object space bounding box corner position
			mSceneItemSet->maximumX.push_back(0.5f);
			mSceneItemSet->maximumY.push_back(0.5f);
			mSceneItemSet->maximumZ.push_back(0.5f);

			// Set object space to world space matrix
			mSceneItemSet->worldXX.push_back(1.0f);
			mSceneItemSet->worldXY.push_back(0.0f);
			mSceneItemSet->worldXZ.push_back(0.0f);
			mSceneItemSet->worldXW.push_back(0.0f);
			mSceneItemSet->worldYX.push_back(0.0f);
			mSceneItemSet->worldYY.push_back(1.0f);
			mSceneItemSet->worldYZ.push_back(0.0f);
			mSceneItemSet->worldYW.push_back(0.0f);
			mSceneItemSet->worldZX.push_back(0.0f);
			mSceneItemSet->worldZY.push_back(0.0f);
			mSceneItemSet->worldZZ.push_back(1.0f);
			mSceneItemSet->worldZW.push_back(0.0f);
			mSceneItemSet->worldWX.push_back(0.0f);
			mSceneItemSet->worldWY.push_back(0.0f);
			mSceneItemSet->worldWZ.push_back(0.0f);
			mSceneItemSet->worldWW.push_back(1.0f);

			// Set world space center position of bounding sphere
			mSceneItemSet->spherePositionX.push_back(0.0f);
			mSceneItemSet->spherePositionY.push_back(0.0f);
			mSceneItemSet->spherePositionZ.push_back(0.0f);

			// Set negative world space radius of bounding sphere
			mSceneItemSet->negativeRadius.push_back(-1.0f);

			mSceneItemSet->visibilityFlag.push_back(0);
			mSceneItemSet->sceneItemVector.push_back(this);
			++mSceneItemSet->numberOfSceneItems;
		}
		else
		{
			mSceneResource.getSceneCullingManager().getUncullableSceneItems().push_back(this);
		}
	}

	ISceneItem::~ISceneItem()
	{
		// Nothing here
	}


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererRuntime
