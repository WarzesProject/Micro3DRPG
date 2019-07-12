#include "stdafx.h"
#include "Resource/Scene/SceneNode.h"
#include "Resource/Scene/SceneResource.h"
#include "Resource/Scene/Item/ISceneItem.h"
#include "Resource/Scene/Item/Mesh/MeshSceneItem.h"
#include "Resource/Scene/Culling/SceneItemSet.h"
#include "Resource/Mesh/MeshResourceManager.h"
#include "Resource/Mesh/MeshResource.h"
#include "IRendererRuntime.h"

// Disable warnings in external headers, we can't fix them
PRAGMA_WARNING_PUSH
	PRAGMA_WARNING_DISABLE_MSVC(4464)	// warning C4464: relative include path contains '..'
	#include <glm/gtx/component_wise.hpp>
PRAGMA_WARNING_POP


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
namespace RendererRuntime
{


	//[-------------------------------------------------------]
	//[ Public methods                                        ]
	//[-------------------------------------------------------]
	void SceneNode::attachSceneNode(SceneNode& sceneNode)
	{
		// TODO(co) Need to guarantee that one scene node is only attached to one scene node at the same time
		mAttachedSceneNodes.push_back(&sceneNode);
		sceneNode.mParentSceneNode = this;
		sceneNode.updateGlobalTransformRecursive();
		mPreviousGlobalTransform = mGlobalTransform;	// Teleport since we don't have a decent incremental previous global transform
	}

	void SceneNode::detachAllSceneNodes()
	{
		for (SceneNode* sceneNode : mAttachedSceneNodes)
		{
			sceneNode->mParentSceneNode = nullptr;
			sceneNode->updateGlobalTransformRecursive();
			mPreviousGlobalTransform = mGlobalTransform;	// Teleport since we don't have a decent incremental previous global transform
		}
		mAttachedSceneNodes.clear();
	}

	void SceneNode::setVisible(bool visible)
	{
		setSceneItemsVisible(visible);
		for (SceneNode* sceneNode : mAttachedSceneNodes)
		{
			sceneNode->setVisible(visible);
		}
	}

	void SceneNode::attachSceneItem(ISceneItem& sceneItem)
	{
		// TODO(co) Need to guarantee that one scene item is only attached to one scene node at the same time
		mAttachedSceneItems.push_back(&sceneItem);
		updateSceneItemTransform(sceneItem);
		sceneItem.onAttachedToSceneNode(*this);
	}

	void SceneNode::detachAllSceneItems()
	{
		for (ISceneItem* sceneItem : mAttachedSceneItems)
		{
			sceneItem->onDetachedFromSceneNode(*this);
		}
		mAttachedSceneItems.clear();
	}

	void SceneNode::setSceneItemsVisible(bool visible)
	{
		for (ISceneItem* sceneItem : mAttachedSceneItems)
		{
			sceneItem->setVisible(visible);
		}
	}


	//[-------------------------------------------------------]
	//[ Private methods                                       ]
	//[-------------------------------------------------------]
	void SceneNode::updateGlobalTransformRecursive()
	{
		// Backup the previous global transform
		mPreviousGlobalTransform = mGlobalTransform;

		// Update this node
		if (nullptr != mParentSceneNode)
		{
			mGlobalTransform = mParentSceneNode->mGlobalTransform;
			mGlobalTransform += mTransform;
		}
		else
		{
			mGlobalTransform = mTransform;
		}

		// Update scene items
		for (ISceneItem* sceneItem : mAttachedSceneItems)
		{
			updateSceneItemTransform(*sceneItem);
		}

		// Update attached scene nodes
		for (SceneNode* sceneNode : mAttachedSceneNodes)
		{
			sceneNode->mGlobalTransform = mGlobalTransform;
			sceneNode->mGlobalTransform += sceneNode->mTransform;
			sceneNode->updateGlobalTransformRecursive();
		}
	}

	void SceneNode::updateSceneItemTransform(ISceneItem& sceneItem)
	{
		// TODO(co) The following is just for culling kickoff and won't stay this way
		SceneItemSet* sceneItemSet = sceneItem.mSceneItemSet;
		if (nullptr != sceneItemSet)
		{
			const uint32_t sceneItemSetIndex = sceneItem.mSceneItemSetIndex;

			{ // Set object space to world space matrix
				glm::mat4 objectSpaceToWorldSpace;
				mGlobalTransform.getAsMatrix(objectSpaceToWorldSpace);
				sceneItemSet->worldXX[sceneItemSetIndex] = objectSpaceToWorldSpace[0][0];
				sceneItemSet->worldXY[sceneItemSetIndex] = objectSpaceToWorldSpace[1][0];
				sceneItemSet->worldXZ[sceneItemSetIndex] = objectSpaceToWorldSpace[2][0];
				sceneItemSet->worldXW[sceneItemSetIndex] = objectSpaceToWorldSpace[3][0];
				sceneItemSet->worldYX[sceneItemSetIndex] = objectSpaceToWorldSpace[0][1];
				sceneItemSet->worldYY[sceneItemSetIndex] = objectSpaceToWorldSpace[1][1];
				sceneItemSet->worldYZ[sceneItemSetIndex] = objectSpaceToWorldSpace[2][1];
				sceneItemSet->worldYW[sceneItemSetIndex] = objectSpaceToWorldSpace[3][1];
				sceneItemSet->worldZX[sceneItemSetIndex] = objectSpaceToWorldSpace[0][2];
				sceneItemSet->worldZY[sceneItemSetIndex] = objectSpaceToWorldSpace[1][2];
				sceneItemSet->worldZZ[sceneItemSetIndex] = objectSpaceToWorldSpace[2][2];
				sceneItemSet->worldZW[sceneItemSetIndex] = objectSpaceToWorldSpace[3][2];
				sceneItemSet->worldWX[sceneItemSetIndex] = objectSpaceToWorldSpace[0][3];
				sceneItemSet->worldWY[sceneItemSetIndex] = objectSpaceToWorldSpace[1][3];
				sceneItemSet->worldWZ[sceneItemSetIndex] = objectSpaceToWorldSpace[2][3];
				sceneItemSet->worldWW[sceneItemSetIndex] = objectSpaceToWorldSpace[3][3];
			}

			if (sceneItem.getSceneItemTypeId() == MeshSceneItem::TYPE_ID)
			{
				const MeshResource* meshResource = sceneItem.getSceneResource().getRendererRuntime().getMeshResourceManager().tryGetById(static_cast<MeshSceneItem&>(sceneItem).getMeshResourceId());
				if (nullptr != meshResource)
				{
					{ // Set world space center position of bounding sphere
						const glm::vec3& boundingSpherePosition = meshResource->getBoundingSpherePosition();
						const glm::dvec3& position = mGlobalTransform.position;
						const glm::vec3& scale = mGlobalTransform.scale;
						sceneItemSet->spherePositionX[sceneItemSetIndex] = static_cast<float>(boundingSpherePosition.x * scale.x + position.x);
						sceneItemSet->spherePositionY[sceneItemSetIndex] = static_cast<float>(boundingSpherePosition.y * scale.y + position.y);
						sceneItemSet->spherePositionZ[sceneItemSetIndex] = static_cast<float>(boundingSpherePosition.z * scale.z + position.z);
					}

					// Set negative world space radius of bounding sphere
					sceneItemSet->negativeRadius[sceneItemSetIndex] = -meshResource->getBoundingSphereRadius() * glm::compMax(mGlobalTransform.scale);
				}
				else
				{
					// Set world space center position of bounding sphere
					sceneItemSet->spherePositionX[sceneItemSetIndex] = static_cast<float>(mGlobalTransform.position.x);
					sceneItemSet->spherePositionY[sceneItemSetIndex] = static_cast<float>(mGlobalTransform.position.y);
					sceneItemSet->spherePositionZ[sceneItemSetIndex] = static_cast<float>(mGlobalTransform.position.z);
				}
			}
			else
			{
				// Set world space center position of bounding sphere
				sceneItemSet->spherePositionX[sceneItemSetIndex] = static_cast<float>(mGlobalTransform.position.x);
				sceneItemSet->spherePositionY[sceneItemSetIndex] = static_cast<float>(mGlobalTransform.position.y);
				sceneItemSet->spherePositionZ[sceneItemSetIndex] = static_cast<float>(mGlobalTransform.position.z);
			}
		}
	}


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererRuntime
