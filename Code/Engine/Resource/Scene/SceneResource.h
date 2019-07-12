#pragma once


//[-------------------------------------------------------]
//[ Includes                                              ]
//[-------------------------------------------------------]
#include "Resource/IResource.h"
#include "Core/Manager.h"

// Disable warnings in external headers, we can't fix them
PRAGMA_WARNING_PUSH
	PRAGMA_WARNING_DISABLE_MSVC(4365)	// warning C4365: 'argument': conversion from 'long' to 'unsigned int', signed/unsigned mismatch
	PRAGMA_WARNING_DISABLE_MSVC(4571)	// warning C4571: Informational: catch(...) semantics changed since Visual C++ 7.1; structured exceptions (SEH) are no longer caught
	PRAGMA_WARNING_DISABLE_MSVC(4668)	// warning C4668: '_M_HYBRID_X86_ARM64' is not defined as a preprocessor macro, replacing with '0' for '#if/#elif'
	#include <vector>
PRAGMA_WARNING_POP


//[-------------------------------------------------------]
//[ Forward declarations                                  ]
//[-------------------------------------------------------]
namespace RendererRuntime
{
	class Transform;
	class SceneNode;
	class ISceneItem;
	class ISceneFactory;
	class IRendererRuntime;
	class SceneCullingManager;
	class SceneResourceLoader;
	template <class ELEMENT_TYPE, typename ID_TYPE, uint32_t MAXIMUM_NUMBER_OF_ELEMENTS> class PackedElementManager;
	template <class TYPE, class LOADER_TYPE, typename ID_TYPE, uint32_t MAXIMUM_NUMBER_OF_ELEMENTS> class ResourceManagerTemplate;
}


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
namespace RendererRuntime
{


	//[-------------------------------------------------------]
	//[ Global definitions                                    ]
	//[-------------------------------------------------------]
	typedef uint32_t SceneResourceId;	///< POD scene resource identifier
	typedef StringId SceneItemTypeId;	///< Scene item type identifier, internally just a POD "uint32_t"


	//[-------------------------------------------------------]
	//[ Classes                                               ]
	//[-------------------------------------------------------]
	class SceneResource final : public IResource
	{


	//[-------------------------------------------------------]
	//[ Friends                                               ]
	//[-------------------------------------------------------]
		friend class SceneResourceManager;															// Needs to be able to update the scene factory instance
		friend PackedElementManager<SceneResource, SceneResourceId, 16>;							// Type definition of template class
		friend ResourceManagerTemplate<SceneResource, SceneResourceLoader, SceneResourceId, 16>;	// Type definition of template class


	//[-------------------------------------------------------]
	//[ Public definitions                                    ]
	//[-------------------------------------------------------]
	public:
		typedef std::vector<SceneNode*> SceneNodes;
		typedef std::vector<ISceneItem*> SceneItems;


	//[-------------------------------------------------------]
	//[ Public methods                                        ]
	//[-------------------------------------------------------]
	public:
		[[nodiscard]] RENDERERRUNTIME_API_EXPORT IRendererRuntime& getRendererRuntime() const;

		[[nodiscard]] inline SceneCullingManager& getSceneCullingManager() const
		{
			// We know that this pointer is always valid
			assert(nullptr != mSceneCullingManager);
			return *mSceneCullingManager;
		}

		RENDERERRUNTIME_API_EXPORT void destroyAllSceneNodesAndItems();

		//[-------------------------------------------------------]
		//[ Node                                                  ]
		//[-------------------------------------------------------]
		[[nodiscard]] RENDERERRUNTIME_API_EXPORT SceneNode* createSceneNode(const Transform& transform);
		RENDERERRUNTIME_API_EXPORT void destroySceneNode(SceneNode& sceneNode);
		RENDERERRUNTIME_API_EXPORT void destroyAllSceneNodes();

		[[nodiscard]] inline const SceneNodes& getSceneNodes() const
		{
			return mSceneNodes;
		}

		//[-------------------------------------------------------]
		//[ Item                                                  ]
		//[-------------------------------------------------------]
		[[nodiscard]] RENDERERRUNTIME_API_EXPORT ISceneItem* createSceneItem(SceneItemTypeId sceneItemTypeId, SceneNode& sceneNode);

		template <typename T>
		[[nodiscard]] inline T* createSceneItem(SceneNode& sceneNode)
		{
			return static_cast<T*>(createSceneItem(T::TYPE_ID, sceneNode));
		}

		RENDERERRUNTIME_API_EXPORT void destroySceneItem(ISceneItem& sceneItem);
		RENDERERRUNTIME_API_EXPORT void destroyAllSceneItems();

		[[nodiscard]] inline const SceneItems& getSceneItems() const
		{
			return mSceneItems;
		}


	//[-------------------------------------------------------]
	//[ Private methods                                       ]
	//[-------------------------------------------------------]
	private:
		inline SceneResource() :
			mSceneFactory(nullptr),
			mSceneCullingManager(nullptr)
		{
			// Nothing here
		}

		inline virtual ~SceneResource() override
		{
			// Sanity checks
			assert(nullptr == mSceneFactory);
			assert(nullptr == mSceneCullingManager);
			assert(mSceneNodes.empty());
			assert(mSceneItems.empty());
		}

		explicit SceneResource(const SceneResource&) = delete;
		SceneResource& operator=(const SceneResource&) = delete;

		inline SceneResource& operator=(SceneResource&& sceneResource)
		{
			// Call base implementation
			IResource::operator=(std::move(sceneResource));

			// Swap data
			std::swap(mSceneFactory, sceneResource.mSceneFactory);
			std::swap(mSceneCullingManager, sceneResource.mSceneCullingManager);
			std::swap(mSceneNodes, sceneResource.mSceneNodes);
			std::swap(mSceneItems, sceneResource.mSceneItems);

			// Done
			return *this;
		}

		//[-------------------------------------------------------]
		//[ "RendererRuntime::PackedElementManager" management    ]
		//[-------------------------------------------------------]
		void initializeElement(SceneResourceId sceneResourceId);
		void deinitializeElement();


	//[-------------------------------------------------------]
	//[ Private data                                          ]
	//[-------------------------------------------------------]
	private:
		const ISceneFactory* mSceneFactory;			///< Scene factory instance, always valid, do not destroy the instance
		SceneCullingManager* mSceneCullingManager;	///< Scene culling manager, always valid, destroy the instance if you no longer need it
		SceneNodes			 mSceneNodes;
		SceneItems			 mSceneItems;


	};


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererRuntime
