#pragma once


//[-------------------------------------------------------]
//[ Includes                                              ]
//[-------------------------------------------------------]
#include "Core/StringId.h"
#include "Context.h"


//[-------------------------------------------------------]
//[ Forward declarations                                  ]
//[-------------------------------------------------------]
namespace RendererRuntime
{
	class Context;
	class SceneNode;
	struct SceneItemSet;
	class SceneResource;
	class RenderableManager;
}


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
namespace RendererRuntime
{


	//[-------------------------------------------------------]
	//[ Global definitions                                    ]
	//[-------------------------------------------------------]
	typedef StringId SceneItemTypeId;	///< Scene item type identifier, internally just a POD "uint32_t"


	//[-------------------------------------------------------]
	//[ Classes                                               ]
	//[-------------------------------------------------------]
	class ISceneItem
	{


	//[-------------------------------------------------------]
	//[ Friends                                               ]
	//[-------------------------------------------------------]
		friend class SceneNode;		// TODO(co) Remove this
		friend class MeshSceneItem;	// TODO(co) Remove this
		friend class SceneResource;	// Needs to be able to destroy scene items


	//[-------------------------------------------------------]
	//[ Public methods                                        ]
	//[-------------------------------------------------------]
	public:
		RENDERERRUNTIME_API_EXPORT [[nodiscard]] Context& getContext() const;

		[[nodiscard]] inline SceneResource& getSceneResource() const
		{
			return mSceneResource;
		}

		[[nodiscard]] inline bool hasParentSceneNode() const
		{
			return (nullptr != mParentSceneNode);
		}

		[[nodiscard]] inline SceneNode* getParentSceneNode()
		{
			return mParentSceneNode;
		}

		[[nodiscard]] inline const SceneNode* getParentSceneNode() const
		{
			return mParentSceneNode;
		}

		[[nodiscard]] inline SceneNode& getParentSceneNodeSafe()	// Just safe in context known as safe
		{
			assert(nullptr != mParentSceneNode);
			return *mParentSceneNode;
		}

		[[nodiscard]] inline const SceneNode& getParentSceneNodeSafe() const	// Just safe in context known as safe
		{
			assert(nullptr != mParentSceneNode);
			return *mParentSceneNode;
		}


	//[-------------------------------------------------------]
	//[ Public RendererRuntime::ISceneItem methods            ]
	//[-------------------------------------------------------]
	public:
		[[nodiscard]] virtual SceneItemTypeId getSceneItemTypeId() const = 0;
		virtual void deserialize(uint32_t numberOfBytes, const uint8_t* data) = 0;

		inline virtual void onAttachedToSceneNode(SceneNode& sceneNode)
		{
			assert(nullptr == mParentSceneNode);
			mParentSceneNode = &sceneNode;
		}

		inline virtual void onDetachedFromSceneNode([[maybe_unused]] SceneNode& sceneNode)
		{
			assert(nullptr != mParentSceneNode);
			mParentSceneNode = nullptr;
		}

		inline virtual void setVisible([[maybe_unused]] bool visible)
		{
			// Nothing here
		}

		[[nodiscard]] inline virtual const RenderableManager* getRenderableManager() const
		{
			return nullptr;
		}


	//[-------------------------------------------------------]
	//[ Protected methods                                     ]
	//[-------------------------------------------------------]
	protected:
		ISceneItem(SceneResource& sceneResource, bool cullable = true);
		virtual ~ISceneItem();
		explicit ISceneItem(const ISceneItem&) = delete;
		ISceneItem& operator=(const ISceneItem&) = delete;


	//[-------------------------------------------------------]
	//[ Private data                                          ]
	//[-------------------------------------------------------]
	private:
		SceneResource& mSceneResource;
		SceneNode*	   mParentSceneNode;	///< Parent scene node, can be a null pointer, don't destroy the instance
		SceneItemSet*  mSceneItemSet;		///< Scene item set, always valid, don't destroy the instance
		uint32_t	   mSceneItemSetIndex;	///< Index inside the scene item set


	};


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererRuntime
